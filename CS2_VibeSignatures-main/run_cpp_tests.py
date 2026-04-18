#!/usr/bin/env python3
"""
Run C++ tests declared in config.yaml and compare clang vtable dumps with YAML references.
"""

import argparse
import json
import shlex
import subprocess
import sys
import tempfile
import uuid
from pathlib import Path
from typing import Any, Dict, List, Sequence

try:
    import yaml
except ImportError as e:
    print(f"Error: Missing required dependency: {e.name}")
    print("Please install required dependencies with: uv sync")
    sys.exit(1)

from cpp_tests_util import (
    compare_compiler_vtable_with_yaml,
    format_compiler_vtable_entries,
    format_reference_vtable_entries,
    format_vtable_compare_differences,
    format_vtable_compare_report,
    format_vtable_differences_for_agent,
    map_target_triple_to_platform,
    pointer_size_from_target_triple,
)


DEFAULT_CONFIG_FILE = "config.yaml"
DEFAULT_BIN_DIR = "bin"
DEFAULT_CLANG = "clang++"
DEFAULT_CPP_STD = "c++20"
DEFAULT_AGENT = "claude"
DEFAULT_MAX_RETRY = 3
DEFAULT_MAX_VERIFY = 3
SKILL_TIMEOUT = 600
VTABLE_FIXER_AGENT_FILE = Path(".claude/agents/vtable-fixer.md")


def parse_args():
    """Parse CLI arguments."""
    parser = argparse.ArgumentParser(
        description="Run configured C++ tests with clang++ and compare vtable metadata"
    )
    parser.add_argument(
        "-configyaml",
        default=DEFAULT_CONFIG_FILE,
        help=f"Path to config.yaml file (default: {DEFAULT_CONFIG_FILE})",
    )
    parser.add_argument(
        "-bindir",
        default=DEFAULT_BIN_DIR,
        help=f"Directory containing YAML outputs (default: {DEFAULT_BIN_DIR})",
    )
    parser.add_argument(
        "-gamever",
        required=True,
        help="Game version subdirectory name under bin (required)",
    )
    parser.add_argument(
        "-clang",
        default=DEFAULT_CLANG,
        help=f"clang++ executable path (default: {DEFAULT_CLANG})",
    )
    parser.add_argument(
        "-std",
        default=DEFAULT_CPP_STD,
        help=f"C++ standard for compilation (default: {DEFAULT_CPP_STD})",
    )
    parser.add_argument(
        "-debug",
        action="store_true",
        help="Enable debug output",
    )
    parser.add_argument(
        "-fixheader",
        action="store_true",
        help="When vtable differences are found, invoke agent to fix configured C++ headers",
    )
    parser.add_argument(
        "-agent",
        default=DEFAULT_AGENT,
        help=f"Agent executable to use for header fixing, e.g. claude/codex (default: {DEFAULT_AGENT})",
    )
    parser.add_argument(
        "-maxretry",
        type=int,
        default=DEFAULT_MAX_RETRY,
        help=f"Maximum retry attempts for header-fix agent runs (default: {DEFAULT_MAX_RETRY})",
    )
    parser.add_argument(
        "-maxverify",
        type=int,
        default=DEFAULT_MAX_VERIFY,
        help=f"Maximum verify-and-retry cycles after agent fix (default: {DEFAULT_MAX_VERIFY})",
    )
    parser.add_argument(
        "-claude_allowed_tools",
        default="",
        help="Pass-through value for Claude '--allowedTools' during -fixheader runs",
    )
    parser.add_argument(
        "-claude_permission_mode",
        default="",
        help="Pass-through value for Claude '--permission-mode' during -fixheader runs",
    )
    parser.add_argument(
        "-claude_extra_args",
        default="",
        help="Additional raw CLI arguments appended to Claude command during -fixheader runs",
    )
    return parser.parse_args()


def _to_list(value: Any) -> List[str]:
    if value is None:
        return []
    if isinstance(value, list):
        return [str(v).strip() for v in value if str(v).strip()]
    if isinstance(value, str):
        text = value.strip()
        return [text] if text else []
    return [str(value).strip()]


def _to_text(value: Any) -> str:
    if value is None:
        return ""
    return str(value).strip()


def _to_bool(value: Any, default: bool = False) -> bool:
    if value is None:
        return default
    if isinstance(value, bool):
        return value
    if isinstance(value, int):
        if value in (0, 1):
            return bool(value)
        raise ValueError(f"Invalid boolean value: {value!r}")
    if isinstance(value, str):
        normalized = value.strip().lower()
        if not normalized:
            return default
        if normalized in {"1", "true", "yes", "y", "on"}:
            return True
        if normalized in {"0", "false", "no", "n", "off"}:
            return False
        raise ValueError(f"Invalid boolean value: {value!r}")
    raise ValueError(f"Invalid boolean value: {value!r}")


def _choose_override(item_value: Any, fallback: str) -> str:
    override = _to_text(item_value)
    if override:
        return override
    return fallback


def _split_cli_args(raw_args: str) -> List[str]:
    text = _to_text(raw_args)
    if not text:
        return []
    try:
        return shlex.split(text, posix=False)
    except ValueError:
        return text.split()


def _normalize_option(option_text: str) -> str:
    option_text = option_text.strip()
    if not option_text:
        return ""
    if option_text.startswith("-"):
        return option_text
    return f"-{option_text}"


def _contains_fdump_vtable_layouts(options: Sequence[str]) -> bool:
    for option in options:
        normalized = _normalize_option(option)
        if normalized and normalized.lstrip("-") == "fdump-vtable-layouts":
            return True
    return False


def _format_command(command: Sequence[str]) -> str:
    return subprocess.list2cmdline(list(command))


def _collect_process_output(result: subprocess.CompletedProcess) -> str:
    stdout_text = result.stdout.strip() if result.stdout else ""
    stderr_text = result.stderr.strip() if result.stderr else ""

    if stdout_text and stderr_text:
        return f"{stdout_text}\n{stderr_text}"
    if stdout_text:
        return stdout_text
    return stderr_text


def parse_config(config_path: Path) -> List[Dict[str, Any]]:
    """Load and validate cpp_tests from config.yaml."""
    try:
        with config_path.open("r", encoding="utf-8") as f:
            config = yaml.safe_load(f) or {}
    except FileNotFoundError:
        print(f"Error: Config file not found: {config_path}")
        sys.exit(1)
    except Exception as exc:
        print(f"Error: Failed to parse config file {config_path}: {exc}")
        sys.exit(1)

    cpp_tests = config.get("cpp_tests", [])
    if not isinstance(cpp_tests, list):
        print("Error: 'cpp_tests' in config.yaml must be a list")
        sys.exit(1)

    return cpp_tests


def _strip_optional_frontmatter(markdown_text: str) -> str:
    """Remove optional YAML frontmatter from an agent markdown file."""
    content = markdown_text.strip()
    if not content.startswith("---"):
        return content
    lines = content.splitlines()
    frontmatter_end = None
    for idx, line in enumerate(lines[1:], start=1):
        if line.strip() == "---":
            frontmatter_end = idx
            break
    if frontmatter_end is None:
        return content
    return "\n".join(lines[frontmatter_end + 1 :]).strip()


def _load_codex_developer_instructions(agent_md_path: Path) -> str:
    """Load and normalize Codex developer_instructions from agent markdown."""
    try:
        raw = agent_md_path.read_text(encoding="utf-8")
    except FileNotFoundError:
        print(f"Error: Codex agent prompt file not found: {agent_md_path}")
        return ""
    except OSError as exc:
        print(f"Error: Failed to read Codex agent prompt file {agent_md_path}: {exc}")
        return ""

    prompt = _strip_optional_frontmatter(raw)
    if not prompt:
        print(f"Error: Codex agent prompt is empty in {agent_md_path}")
        return ""
    return f"developer_instructions={json.dumps(prompt)}"


def _resolve_header_paths(test_item: Dict[str, Any], config_dir: Path) -> List[Path]:
    """Resolve configured header paths to absolute paths."""
    headers = _to_list(test_item.get("headers"))
    resolved: List[Path] = []
    for header in headers:
        path = Path(header)
        if not path.is_absolute():
            path = (config_dir / path).resolve()
        resolved.append(path)
    return resolved


def _build_fix_prompt(
    *,
    symbol: str,
    header_paths: Sequence[Path],
    diff_reports: Sequence[Dict[str, Any]],
) -> str:
    """Build English prompt for fixing C++ headers based on vtable differences."""
    lines: List[str] = []
    lines.append(
        f"Please update the C++ header declarations for interface/class '{symbol}' according to the YAML reference vtable entries."
    )
    lines.append(
        "Follow the existing code style, formatting, and naming conventions in the header."
    )
    lines.append("Do not make unrelated edits.")
    lines.append("")
    lines.append("Header file paths to edit:")
    for path in header_paths:
        lines.append(f"- {path.as_posix()}")
    lines.append("")
    lines.append("VTable Information:")
    for report in diff_reports:
        # reference modules are unrelated and should not be populated in prompt.
        #module_name = report.get("reference_module")
        #if not module_name:
        #    requested = report.get("requested_modules", [])
        #    module_name = ", ".join(requested) if requested else "unknown"
        #lines.append(f"Reference module: {module_name}")
        lines.append("  Current vtable entries in c++ header:")
        for entry_line in format_compiler_vtable_entries(report):
            lines.append(f"    {entry_line}")
        lines.append("  YAML reference vtable entries:")
        for entry_line in format_reference_vtable_entries(report):
            lines.append(f"    {entry_line}")
        lines.append("  VTable Differences:")
        for diff_line in format_vtable_differences_for_agent(report):
            lines.append(f"    {diff_line}")
    lines.append("")
    lines.append(
        "Apply the header updates now and keep the resulting declarations consistent with the latest vtable layout."
    )
    return "\n".join(lines)


def run_fix_header_agent(
    *,
    fix_prompt: str,
    agent: str,
    debug: bool,
    max_retries: int,
    session_id: str = "",
    is_continuation: bool = False,
    claude_allowed_tools: str = "",
    claude_permission_mode: str = "",
    claude_extra_args: str = "",
) -> bool:
    """Invoke claude/codex agent to apply header fixes."""
    max_retries = max(1, int(max_retries))
    claude_session_id = session_id if session_id else str(uuid.uuid4())

    codex_developer_instructions = None
    if "codex" in agent.lower():
        codex_developer_instructions = _load_codex_developer_instructions(
            VTABLE_FIXER_AGENT_FILE
        )
        if not codex_developer_instructions:
            return False

    for attempt in range(max_retries):
        is_retry = (attempt > 0) or is_continuation
        is_claude_agent = "claude" in agent.lower()
        is_codex_agent = "codex" in agent.lower()
        agent_input = None

        if is_claude_agent:
            agent_input = fix_prompt
            cmd = [
                agent,
                "-p",
                "-",
                "--agent",
                "vtable-fixer",
                "--settings",
                '{"alwaysThinkingEnabled": false}',
            ]
            if _to_text(claude_allowed_tools):
                cmd.extend(["--allowedTools", _to_text(claude_allowed_tools)])
            if _to_text(claude_permission_mode):
                cmd.extend(["--permission-mode", _to_text(claude_permission_mode)])
            extra_args = _split_cli_args(claude_extra_args)
            if extra_args:
                cmd.extend(extra_args)
            if is_retry:
                cmd.extend(["--resume", claude_session_id])
            else:
                cmd.extend(["--session-id", claude_session_id])
            retry_target_desc = f"session {claude_session_id}"
        elif is_codex_agent:
            agent_input = fix_prompt
            if is_retry:
                cmd = [
                    agent,
                    "-c",
                    codex_developer_instructions,
                    "-c",
                    "model_reasoning_effort=high",
                    "-c",
                    "model_reasoning_summary=none",
                    "-c",
                    "model_verbosity=low",
                    "exec",
                    "resume",
                    "--last",
                    "-",
                ]
            else:
                cmd = [
                    agent,
                    "-c",
                    codex_developer_instructions,
                    "-c",
                    "model_reasoning_effort=high",
                    "-c",
                    "model_reasoning_summary=none",
                    "-c",
                    "model_verbosity=low",
                    "exec",
                    "-",
                ]
            retry_target_desc = "the latest codex session (--last)"
        else:
            print(
                f"    Error: Unknown agent type '{agent}'. Agent name must contain 'claude' or 'codex'."
            )
            return False

        retry_tag = "[RETRY] " if is_retry else ""
        attempt_str = f"(attempt {attempt + 1}/{max_retries})" if max_retries > 1 else ""
        prompt_transport = " via stdin" if agent_input is not None else ""
        print(
            f"    {retry_tag}Running {attempt_str}: {agent} <vtable-fixer-prompt{prompt_transport}>"
        )

        try:
            run_kwargs = {
                "timeout": SKILL_TIMEOUT,
                "check": False,
            }
            if agent_input is not None:
                run_kwargs["input"] = agent_input
                run_kwargs["text"] = True

            if debug:
                result = subprocess.run(cmd, **run_kwargs)
            else:
                run_kwargs["capture_output"] = True
                run_kwargs.setdefault("text", True)
                result = subprocess.run(cmd, **run_kwargs)

            if result.returncode == 0:
                return True

            print(f"    Agent failed with return code: {result.returncode}")
            if not debug and result.stderr:
                print(f"    stderr: {result.stderr[:500]}")
            if attempt < max_retries - 1:
                print(f"    Retrying with {retry_target_desc}...")
        except subprocess.TimeoutExpired:
            print(f"    Error: Agent execution timeout ({SKILL_TIMEOUT} seconds)")
            if attempt < max_retries - 1:
                print(f"    Retrying with {retry_target_desc}...")
        except FileNotFoundError:
            print(f"    Error: Agent '{agent}' not found. Please ensure it is installed and in PATH.")
            return False
        except Exception as exc:
            print(f"    Error executing fix-header agent: {exc}")
            if attempt < max_retries - 1:
                print(f"    Retrying with {retry_target_desc}...")

    print(f"    Failed after {max_retries} attempts")
    return False


def run_fix_header_with_verification(
    *,
    symbol: str,
    header_paths: List[Path],
    diff_reports: List[Dict[str, Any]],
    test_item: Dict[str, Any],
    args: argparse.Namespace,
    config_dir: Path,
    bindir: Path,
    claude_allowed_tools: str,
    claude_permission_mode: str,
    claude_extra_args: str,
    debug: bool,
) -> bool:
    """Run fix-header agent, then verify by recompiling.

    If vtable diffs persist after the agent edits, re-run the agent with the
    updated diffs.  Repeats up to ``args.maxverify`` cycles.

    Returns True when all diffs are resolved, False otherwise.
    """
    max_verify = max(1, args.maxverify)
    session_id = str(uuid.uuid4())
    current_diff_reports = list(diff_reports)

    for verify_attempt in range(max_verify):
        is_continuation = verify_attempt > 0

        fix_prompt = _build_fix_prompt(
            symbol=symbol,
            header_paths=header_paths,
            diff_reports=current_diff_reports,
        )

        if debug:
            print(fix_prompt)

        if max_verify > 1:
            print(
                f"  [INFO] Verify cycle {verify_attempt + 1}/{max_verify}: "
                f"invoking agent '{args.agent}' to fix headers..."
            )

        agent_success = run_fix_header_agent(
            fix_prompt=fix_prompt,
            agent=args.agent,
            debug=debug,
            max_retries=args.maxretry,
            session_id=session_id,
            is_continuation=is_continuation,
            claude_allowed_tools=claude_allowed_tools,
            claude_permission_mode=claude_permission_mode,
            claude_extra_args=claude_extra_args,
        )

        if not agent_success:
            print(f"  [FAIL] Agent failed during verify cycle {verify_attempt + 1}.")
            return False

        # Agent claimed success -- verify by recompiling
        print("  [INFO] Agent completed; re-compiling to verify fix...")
        recompile_result = compile_and_compare(
            test_item=test_item,
            args=args,
            config_dir=config_dir,
            bindir=bindir,
        )

        if recompile_result["status"] == "compile_failed":
            print("  [FAIL] Re-compilation failed after agent edit.")
            if recompile_result.get("output"):
                print(recompile_result["output"])
            return False

        if recompile_result["status"] == "invalid":
            print(f"  [FAIL] Invalid test item during verification: {recompile_result.get('message', '')}")
            return False

        new_compare_reports = recompile_result.get("compare_reports") or []
        new_reports_with_diff = [
            r for r in new_compare_reports if r.get("differences")
        ]

        if not new_reports_with_diff:
            print("  [PASS] Verification succeeded -- all vtable differences resolved.")
            return True

        remaining_diffs = sum(
            len(r.get("differences", [])) for r in new_reports_with_diff
        )
        print(
            f"  [INFO] {remaining_diffs} vtable difference(s) remain after agent edit."
        )
        if debug:
            for report in new_reports_with_diff:
                for diff in report.get("differences", []):
                    print(f"    - {diff['message']}")

        current_diff_reports = new_reports_with_diff

    print(
        f"  [FAIL] Vtable differences persist after {max_verify} verify-and-retry cycle(s)."
    )
    return False


def get_default_target_triple(clang: str) -> str:
    """Run clang++ -print-target-triple and return the result."""
    command = [clang, "-print-target-triple"]
    result = subprocess.run(
        command,
        capture_output=True,
        text=True,
        check=False,
    )
    if result.returncode != 0:
        output = _collect_process_output(result)
        print(f"Error: Failed to run `{_format_command(command)}`")
        if output:
            print(output)
        sys.exit(1)

    triple = (result.stdout or "").strip()
    if not triple:
        triple = (result.stderr or "").strip()
    if not triple:
        print("Error: clang++ -print-target-triple returned empty output")
        sys.exit(1)
    return triple


def probe_target_support(clang: str, target: str, cpp_std: str) -> Dict[str, Any]:
    """Probe whether clang can compile a minimal source with the given target triple."""
    with tempfile.TemporaryDirectory(prefix="cpp_target_probe_") as temp_dir:
        temp_dir_path = Path(temp_dir)
        source_file = temp_dir_path / "probe.cpp"
        object_file = temp_dir_path / "probe.o"
        source_file.write_text("int main() { return 0; }\n", encoding="utf-8")

        command = [
            clang,
            f"--target={target}",
            f"-std={cpp_std}",
            "-c",
            str(source_file),
            "-o",
            str(object_file),
        ]
        result = subprocess.run(
            command,
            capture_output=True,
            text=True,
            check=False,
        )

    return {
        "target": target,
        "supported": result.returncode == 0,
        "command": command,
        "output": _collect_process_output(result),
    }


def build_compile_command(
    *,
    clang: str,
    cpp_std: str,
    target: str,
    cpp_file: Path,
    object_file: Path,
    include_directories: Sequence[Path],
    defines: Sequence[str],
    additional_options: Sequence[str],
) -> List[str]:
    """Construct clang++ compile command for one cpp test item."""
    command = [
        clang,
        f"--target={target}",
        f"-std={cpp_std}",
        "-c",
        str(cpp_file),
        "-o",
        str(object_file),
    ]

    for include_dir in include_directories:
        command.extend(["-I", str(include_dir)])

    for define in defines:
        command.append(f"-D{define}")

    for option in additional_options:
        normalized_option = _normalize_option(option)
        if normalized_option:
            command.append(normalized_option)

    return command


def compile_and_compare(
    *,
    test_item: Dict[str, Any],
    args: argparse.Namespace,
    config_dir: Path,
    bindir: Path,
) -> Dict[str, Any]:
    """Compile a C++ test file and compare vtable layout against YAML references.

    Returns a dict with keys: status, command, output, compare_reports, and
    optionally message (when status is 'invalid').
    """
    test_name = str(test_item.get("name", "unnamed_test"))
    symbol = str(test_item.get("symbol", "")).strip()
    cpp_rel_path = str(test_item.get("cpp", "")).strip()
    target = str(test_item.get("target", "")).strip()

    if not symbol or not cpp_rel_path or not target:
        return {
            "status": "invalid",
            "message": "Missing required fields: symbol/cpp/target",
        }

    cpp_file = Path(cpp_rel_path)
    if not cpp_file.is_absolute():
        cpp_file = (config_dir / cpp_file).resolve()

    if not cpp_file.is_file():
        return {
            "status": "invalid",
            "message": f"CPP file not found: {cpp_file}",
        }

    include_directories: List[Path] = []
    for include_rel in _to_list(test_item.get("include_directories")):
        include_path = Path(include_rel)
        if not include_path.is_absolute():
            include_path = (config_dir / include_path).resolve()
        include_directories.append(include_path)

    defines = _to_list(test_item.get("defines"))

    additional_options = _to_list(test_item.get("additional_compiler_options"))
    if not additional_options:
        # Keep compatibility with alternate field naming.
        additional_options = _to_list(test_item.get("additional_compile_options"))

    should_parse_vtable = _contains_fdump_vtable_layouts(additional_options)

    with tempfile.TemporaryDirectory(prefix=f"cpp_test_{test_name}_") as temp_dir:
        temp_dir_path = Path(temp_dir)
        object_file = temp_dir_path / f"{test_name}.o"
        command = build_compile_command(
            clang=args.clang,
            cpp_std=args.std,
            target=target,
            cpp_file=cpp_file,
            object_file=object_file,
            include_directories=include_directories,
            defines=defines,
            additional_options=additional_options,
        )
        result = subprocess.run(
            command,
            capture_output=True,
            text=True,
            check=False,
        )

    compile_output = _collect_process_output(result)
    if result.returncode != 0:
        return {
            "status": "compile_failed",
            "command": command,
            "output": compile_output,
        }

    compare_reports = None
    if should_parse_vtable:
        platform = map_target_triple_to_platform(target)
        if platform is None:
            compare_reports = [
                {
                "class_name": symbol,
                "platform": "unknown",
                "requested_modules": _to_list(test_item.get("reference_modules")),
                "compiler_found": False,
                "reference_found": False,
                "differences": [],
                "notes": [
                    f"Cannot map target triple '{target}' to yaml platform; vtable compare skipped."
                ],
                }
            ]
        else:
            reference_modules = _to_list(test_item.get("reference_modules"))
            alias_symbols = _to_list(test_item.get("alias_symbols"))
            try:
                merge_reference_modules = _to_bool(
                    test_item.get("merge_reference_modules"), default=True
                )
            except ValueError as exc:
                return {
                    "status": "invalid",
                    "message": (
                        "Invalid 'merge_reference_modules' value: "
                        f"{test_item.get('merge_reference_modules')!r}. {exc}"
                    ),
                }
            compare_reports = []
            if not reference_modules or merge_reference_modules:
                compare_reports.append(
                    compare_compiler_vtable_with_yaml(
                        class_name=symbol,
                        compiler_output=compile_output,
                        bindir=bindir,
                        gamever=args.gamever,
                        platform=platform,
                        reference_modules=reference_modules,
                        merge_reference_modules=merge_reference_modules,
                        pointer_size=pointer_size_from_target_triple(target),
                        alias_class_names=alias_symbols,
                    )
                )
            else:
                for module_name in reference_modules:
                    compare_reports.append(
                        compare_compiler_vtable_with_yaml(
                            class_name=symbol,
                            compiler_output=compile_output,
                            bindir=bindir,
                            gamever=args.gamever,
                            platform=platform,
                            reference_modules=[module_name],
                            merge_reference_modules=False,
                            pointer_size=pointer_size_from_target_triple(target),
                            alias_class_names=alias_symbols,
                        )
                    )

    return {
        "status": "ok",
        "command": command,
        "output": compile_output,
        "compare_reports": compare_reports,
    }


def run_one_test(
    *,
    test_item: Dict[str, Any],
    args: argparse.Namespace,
    config_dir: Path,
    bindir: Path,
) -> Dict[str, Any]:
    """Compile and (optionally) compare one cpp test item."""
    test_name = str(test_item.get("name", "unnamed_test"))
    result = compile_and_compare(
        test_item=test_item,
        args=args,
        config_dir=config_dir,
        bindir=bindir,
    )
    result["name"] = test_name
    return result


def main():
    args = parse_args()
    config_path = Path(args.configyaml).resolve()
    config_dir = config_path.parent
    bindir = Path(args.bindir).resolve()

    cpp_tests = parse_config(config_path)
    if not cpp_tests:
        print("No cpp_tests defined in config.yaml")
        return 0

    print("=== clang++ target triple detection ===")
    default_target_triple = get_default_target_triple(args.clang)
    print(f"clang++ -print-target-triple => {default_target_triple}")

    configured_targets = sorted(
        {
            str(item.get("target", "")).strip()
            for item in cpp_tests
            if str(item.get("target", "")).strip()
        }
    )

    if not configured_targets:
        print("No target triples found in cpp_tests config")
        return 1

    print("=== target support probe (from configured targets) ===")
    target_support: Dict[str, bool] = {}
    for target in configured_targets:
        probe = probe_target_support(args.clang, target, args.std)
        target_support[target] = bool(probe["supported"])
        status_text = "SUPPORTED" if probe["supported"] else "UNSUPPORTED"
        print(f"[{status_text}] {target}")
        if args.debug and probe["output"]:
            print(probe["output"])

    runnable_tests = []
    skipped_tests = []
    for test_item in cpp_tests:
        target = str(test_item.get("target", "")).strip()
        if target and target_support.get(target):
            runnable_tests.append(test_item)
        else:
            skipped_tests.append(test_item)

    print("=== test selection summary ===")
    print(f"Total tests in config: {len(cpp_tests)}")
    print(f"Runnable tests: {len(runnable_tests)}")
    print(f"Skipped tests (unsupported target): {len(skipped_tests)}")
    for skipped in skipped_tests:
        print(
            f"- skip: {skipped.get('name', 'unnamed_test')} "
            f"(target={skipped.get('target', '')})"
        )

    if not runnable_tests:
        print("No runnable tests for current clang++ environment.")
        return 0

    print("=== running cpp_tests ===")
    compile_failed_count = 0
    invalid_count = 0
    compare_diff_count = 0
    compare_run_count = 0
    header_fix_run_count = 0
    header_fix_fail_count = 0

    for test_item in runnable_tests:
        test_name = str(test_item.get("name", "unnamed_test"))
        symbol = str(test_item.get("symbol", "")).strip()
        print(f"[RUN ] {test_name}")

        result = run_one_test(
            test_item=test_item,
            args=args,
            config_dir=config_dir,
            bindir=bindir,
        )

        if result["status"] == "invalid":
            invalid_count += 1
            print(f"[FAIL] {test_name}: {result['message']}")
            continue

        if result["status"] == "compile_failed":
            compile_failed_count += 1
            print(f"[FAIL] {test_name}: compile failed")
            if args.debug:
                print(f"Command: {_format_command(result['command'])}")
            if result.get("output"):
                print(result["output"])
            continue

        print(f"[PASS] {test_name}: compile succeeded")
        if args.debug:
            print(f"Command: {_format_command(result['command'])}")

        compare_reports = result.get("compare_reports")
        if compare_reports:
            reports_with_diff: List[Dict[str, Any]] = []
            for compare_report in compare_reports:
                compare_run_count += 1
                lines = format_vtable_compare_report(
                    compare_report, include_differences=not args.debug
                )
                for line in lines:
                    print(f"  {line}")
                if compare_report.get("differences"):
                    compare_diff_count += 1
                    reports_with_diff.append(compare_report)
                if args.debug:
                    compiler_debug_lines = format_compiler_vtable_entries(compare_report)
                    print("  [DEBUG] Compiler vtable entries:")
                    for debug_line in compiler_debug_lines:
                        print(f"    {debug_line}")
                    debug_lines = format_reference_vtable_entries(compare_report)
                    print("  [DEBUG] YAML reference vtable entries:")
                    for debug_line in debug_lines:
                        print(f"    {debug_line}")
                    diff_lines = format_vtable_compare_differences(compare_report)
                    for diff_line in diff_lines:
                        print(f"  {diff_line}")

            if args.fixheader and reports_with_diff:
                header_paths = _resolve_header_paths(test_item, config_dir)
                if not header_paths:
                    header_fix_fail_count += 1
                    print(
                        f"  [FAIL] fixheader requested but no headers configured for test '{test_name}'."
                    )
                else:
                    claude_allowed_tools = _choose_override(
                        test_item.get("claude_allowed_tools"),
                        args.claude_allowed_tools,
                    )
                    claude_permission_mode = _choose_override(
                        test_item.get("claude_permission_mode"),
                        args.claude_permission_mode,
                    )
                    claude_extra_args = _choose_override(
                        test_item.get("claude_extra_args"),
                        args.claude_extra_args,
                    )
                    print(
                        f"  [INFO] VTable differences detected; invoking agent '{args.agent}' to fix headers..."
                    )
                    header_fix_run_count += 1
                    if not run_fix_header_with_verification(
                        symbol=symbol,
                        header_paths=header_paths,
                        diff_reports=reports_with_diff,
                        test_item=test_item,
                        args=args,
                        config_dir=config_dir,
                        bindir=bindir,
                        claude_allowed_tools=claude_allowed_tools,
                        claude_permission_mode=claude_permission_mode,
                        claude_extra_args=claude_extra_args,
                        debug=args.debug,
                    ):
                        header_fix_fail_count += 1
        elif args.debug and result.get("output"):
            print("  (Compiler output)")
            print(result["output"])

    print("=== done ===")
    print(f"Compile failures: {compile_failed_count}")
    print(f"Invalid test items: {invalid_count}")
    print(f"VTable compares run: {compare_run_count}")
    print(f"VTable compares with differences: {compare_diff_count}")
    if args.fixheader:
        print(f"Header fix agent runs: {header_fix_run_count}")
        print(f"Header fix agent failures: {header_fix_fail_count}")

    if compile_failed_count > 0 or invalid_count > 0 or header_fix_fail_count > 0:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
