# Cross-Module Artifact + Inherit-VFunc Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Support sibling-module YAML references in `expected_input` and allow `inherit_vfuncs` to reuse `../server/...` slot metadata when generating a full current-module function YAML.

**Architecture:** Keep module iteration unchanged in `ida_analyze_bin.py`, but add a small resolver that normalizes current-module and sibling-module artifact paths under the same `bin/{gamever}` root. Extend `preprocess_index_based_vfunc_via_mcp()` to resolve base YAMLs from either the current module or a sibling module, derive the slot index from `vfunc_index` or `vfunc_offset`, and keep the existing func-sig reuse/generation flow. Add a thin preprocessor script that only declares `INHERIT_VFUNCS` and delegates to `preprocess_common_skill()`.

**Tech Stack:** Python 3, `unittest`, `AsyncMock`, PyYAML, existing IDA MCP helpers

---

## File Map

- Modify: `ida_analyze_bin.py` — add safe artifact path resolution and use it for `expected_output` expansion plus `expected_input` validation.
- Modify: `ida_analyze_util.py` — teach index-based vfunc preprocessing to resolve sibling-module base YAMLs and derive slot index from `vfunc_offset`.
- Create: `ida_preprocessor_scripts/find-CFlattenedSerializers_CreateFieldChangedEventQueue-impl.py` — thin declarative wrapper around `preprocess_common_skill()`.
- Create: `tests/test_ida_analyze_bin.py` — unit tests for artifact path resolution.
- Create: `tests/test_ida_analyze_util.py` — async unit tests for cross-module `inherit_vfuncs` lookup and `vfunc_offset` fallback.
- Create: `tests/test_ida_preprocessor_scripts.py` — dynamic-load test for the new preprocessor script.

## Repository Constraints

- Keep module execution order unchanged; do not add cross-module scheduling.
- Keep all resolved artifact paths under the same `bin/{gamever}` root.
- Do not run `git commit` from this plan unless the user explicitly asks for it in the implementation session.

### Task 1: Add artifact-path resolver coverage in `ida_analyze_bin.py`

**Files:**
- Create: `tests/test_ida_analyze_bin.py`
- Modify: `ida_analyze_bin.py`

- [ ] **Step 1: Write the failing resolver tests**

```python
import unittest
from pathlib import Path

import ida_analyze_bin


class TestResolveArtifactPath(unittest.TestCase):
    def test_resolve_artifact_path_keeps_current_module_artifacts_local(self) -> None:
        binary_dir = str(Path("/tmp/bin/14141/networksystem"))

        resolved = ida_analyze_bin.resolve_artifact_path(
            binary_dir,
            "CNetChan_vtable.{platform}.yaml",
            "linux",
        )

        self.assertEqual(
            str(Path("/tmp/bin/14141/networksystem/CNetChan_vtable.linux.yaml").resolve()),
            resolved,
        )

    def test_resolve_artifact_path_supports_sibling_module_reference(self) -> None:
        binary_dir = str(Path("/tmp/bin/14141/networksystem"))

        resolved = ida_analyze_bin.resolve_artifact_path(
            binary_dir,
            "../server/CFlattenedSerializers_CreateFieldChangedEventQueue.{platform}.yaml",
            "windows",
        )

        self.assertEqual(
            str(
                Path(
                    "/tmp/bin/14141/server/"
                    "CFlattenedSerializers_CreateFieldChangedEventQueue.windows.yaml"
                ).resolve()
            ),
            resolved,
        )

    def test_resolve_artifact_path_rejects_escape_outside_gamever_root(self) -> None:
        binary_dir = str(Path("/tmp/bin/14141/networksystem"))

        with self.assertRaises(ValueError):
            ida_analyze_bin.resolve_artifact_path(
                binary_dir,
                "../../outside/secret.{platform}.yaml",
                "windows",
            )


if __name__ == "__main__":
    unittest.main()
```

- [ ] **Step 2: Run the new tests to verify they fail**

Run: `python -m unittest tests.test_ida_analyze_bin -v`

Expected: FAIL with `AttributeError: module 'ida_analyze_bin' has no attribute 'resolve_artifact_path'`

- [ ] **Step 3: Implement the resolver and wire it into `expand_expected_paths()` / `process_binary()`**

```python
def resolve_artifact_path(binary_dir, artifact_path, platform):
    """Resolve an artifact path under the current gamever root."""
    if not artifact_path:
        raise ValueError("artifact path is empty")

    expanded = artifact_path.replace("{platform}", platform)
    module_dir = Path(binary_dir).resolve()
    gamever_dir = module_dir.parent.resolve()
    candidate = (module_dir / expanded).resolve()

    if os.path.commonpath([str(candidate), str(gamever_dir)]) != str(gamever_dir):
        raise ValueError(
            f"artifact path escapes gamever root: {artifact_path}"
        )

    return str(candidate)


def expand_expected_paths(binary_dir, paths, platform):
    """Expand {platform} placeholders and resolve artifact paths safely."""
    return [
        resolve_artifact_path(binary_dir, path, platform)
        for path in paths
    ]
```

```python
            try:
                expected_inputs = [
                    resolve_artifact_path(binary_dir, artifact, platform)
                    for artifact in skill.get("expected_input", [])
                ]
            except ValueError as exc:
                fail_count += 1
                print(f"  Failed: {skill_name} ({exc})")
                continue

            missing_inputs = [path for path in expected_inputs if not os.path.exists(path)]
```

- [ ] **Step 4: Re-run the resolver tests**

Run: `python -m unittest tests.test_ida_analyze_bin -v`

Expected: PASS with 3 tests

### Task 2: Cover and implement cross-module `inherit_vfuncs` lookup

**Files:**
- Create: `tests/test_ida_analyze_util.py`
- Modify: `ida_analyze_util.py`

- [ ] **Step 1: Write the failing async tests for sibling-module lookup and offset fallback**

```python
import json
import tempfile
import unittest
from pathlib import Path
from unittest.mock import AsyncMock, patch

import yaml

import ida_analyze_util


class _FakeTextContent:
    def __init__(self, text: str) -> None:
        self.text = text


class _FakeCallToolResult:
    def __init__(self, payload: dict[str, object]) -> None:
        self.content = [_FakeTextContent(json.dumps(payload))]


def _py_eval_payload(payload: object) -> _FakeCallToolResult:
    return _FakeCallToolResult(
        {
            "result": json.dumps(payload),
            "stdout": "",
            "stderr": "",
        }
    )


class TestPreprocessIndexBasedVfuncViaMcp(unittest.IsolatedAsyncioTestCase):
    async def test_reads_sibling_module_yaml_and_derives_index_from_offset(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir) / "bin" / "14141"
            network_dir = root / "networksystem"
            server_dir = root / "server"
            network_dir.mkdir(parents=True)
            server_dir.mkdir(parents=True)

            (network_dir / "CFlattenedSerializers_vtable.windows.yaml").write_text(
                yaml.safe_dump(
                    {"vtable_entries": {35: "0x140010000"}},
                    sort_keys=False,
                ),
                encoding="utf-8",
            )
            (server_dir / "CFlattenedSerializers_CreateFieldChangedEventQueue.windows.yaml").write_text(
                yaml.safe_dump(
                    {
                        "func_name": "CFlattenedSerializers_CreateFieldChangedEventQueue",
                        "vtable_name": "CFlattenedSerializers",
                        "vfunc_offset": "0x118",
                    },
                    sort_keys=False,
                ),
                encoding="utf-8",
            )

            session = AsyncMock()
            session.call_tool.return_value = _py_eval_payload(
                {"func_va": "0x140010000", "func_size": "0x40"}
            )

            with patch(
                "ida_analyze_util.preprocess_gen_func_sig_via_mcp",
                new=AsyncMock(return_value={"func_sig": "AA BB CC DD"}),
            ):
                payload = await ida_analyze_util.preprocess_index_based_vfunc_via_mcp(
                    session=session,
                    target_func_name="CFlattenedSerializers_CreateFieldChangedEventQueue",
                    target_output=str(
                        network_dir
                        / "CFlattenedSerializers_CreateFieldChangedEventQueue.windows.yaml"
                    ),
                    old_yaml_map={},
                    new_binary_dir=str(network_dir),
                    platform="windows",
                    image_base=0x140000000,
                    base_vfunc_name="../server/CFlattenedSerializers_CreateFieldChangedEventQueue",
                    inherit_vtable_class="CFlattenedSerializers",
                    generate_func_sig=True,
                    debug=False,
                )

            self.assertEqual(35, payload["vfunc_index"])
            self.assertEqual("0x118", payload["vfunc_offset"])
            self.assertEqual("AA BB CC DD", payload["func_sig"])

    async def test_returns_none_for_misaligned_vfunc_offset(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir) / "bin" / "14141"
            network_dir = root / "networksystem"
            server_dir = root / "server"
            network_dir.mkdir(parents=True)
            server_dir.mkdir(parents=True)

            (network_dir / "CFlattenedSerializers_vtable.windows.yaml").write_text(
                yaml.safe_dump({"vtable_entries": {35: "0x140010000"}}, sort_keys=False),
                encoding="utf-8",
            )
            (server_dir / "CFlattenedSerializers_CreateFieldChangedEventQueue.windows.yaml").write_text(
                yaml.safe_dump(
                    {
                        "func_name": "CFlattenedSerializers_CreateFieldChangedEventQueue",
                        "vtable_name": "CFlattenedSerializers",
                        "vfunc_offset": "0x11A",
                    },
                    sort_keys=False,
                ),
                encoding="utf-8",
            )

            payload = await ida_analyze_util.preprocess_index_based_vfunc_via_mcp(
                session=AsyncMock(),
                target_func_name="CFlattenedSerializers_CreateFieldChangedEventQueue",
                target_output=str(
                    network_dir
                    / "CFlattenedSerializers_CreateFieldChangedEventQueue.windows.yaml"
                ),
                old_yaml_map={},
                new_binary_dir=str(network_dir),
                platform="windows",
                image_base=0x140000000,
                base_vfunc_name="../server/CFlattenedSerializers_CreateFieldChangedEventQueue",
                inherit_vtable_class="CFlattenedSerializers",
                generate_func_sig=False,
                debug=False,
            )

            self.assertIsNone(payload)


if __name__ == "__main__":
    unittest.main()
```

- [ ] **Step 2: Run the new async tests to verify they fail**

Run: `python -m unittest tests.test_ida_analyze_util -v`

Expected: FAIL because the current code only reads `vfunc_index` and does not derive the slot from `vfunc_offset`

- [ ] **Step 3: Extend `preprocess_index_based_vfunc_via_mcp()` to resolve sibling-module YAMLs and derive the slot index**

```python
from pathlib import Path
```

```python
    def _resolve_related_yaml_path(binary_dir, artifact_stem, platform_name):
        expanded = f"{artifact_stem}.{platform_name}.yaml"
        module_dir = Path(binary_dir).resolve()
        gamever_dir = module_dir.parent.resolve()
        candidate = (module_dir / expanded).resolve()
        if os.path.commonpath([str(candidate), str(gamever_dir)]) != str(gamever_dir):
            raise ValueError(f"artifact path escapes gamever root: {artifact_stem}")
        return str(candidate)

    def _extract_vfunc_index(data):
        raw_index = data.get("vfunc_index")
        raw_offset = data.get("vfunc_offset")

        if raw_index is None and raw_offset is None:
            raise ValueError("missing vfunc_index/vfunc_offset")

        parsed_index = _parse_int(raw_index) if raw_index is not None else None
        parsed_offset = _parse_int(raw_offset) if raw_offset is not None else None

        if parsed_offset is not None:
            if parsed_offset % 8 != 0:
                raise ValueError("vfunc_offset is not 8-byte aligned")
            offset_index = parsed_offset // 8
            if parsed_index is None:
                parsed_index = offset_index
            elif parsed_index != offset_index:
                raise ValueError("vfunc_index/vfunc_offset mismatch")

        return parsed_index
```

```python
    try:
        base_vfunc_path = _resolve_related_yaml_path(
            new_binary_dir,
            base_vfunc_name,
            platform,
        )
    except ValueError:
        if debug:
            print(f"    Preprocess: invalid base vfunc path: {base_vfunc_name}")
        return None

    base_vfunc_data = _read_yaml(base_vfunc_path)
    if not isinstance(base_vfunc_data, dict):
        ...

    try:
        base_index = _extract_vfunc_index(base_vfunc_data)
    except Exception:
        if debug:
            print(
                "    Preprocess: invalid vfunc slot metadata in "
                f"{os.path.basename(base_vfunc_path)}"
            )
        return None
```

- [ ] **Step 4: Re-run the async tests**

Run: `python -m unittest tests.test_ida_analyze_util -v`

Expected: PASS with 2 tests

### Task 3: Add the thin preprocessor script for `find-CFlattenedSerializers_CreateFieldChangedEventQueue-impl`

**Files:**
- Create: `ida_preprocessor_scripts/find-CFlattenedSerializers_CreateFieldChangedEventQueue-impl.py`
- Create: `tests/test_ida_preprocessor_scripts.py`

- [ ] **Step 1: Write the failing script-loader test**

```python
import importlib.util
import unittest
from pathlib import Path
from unittest.mock import AsyncMock, patch


SCRIPT_PATH = Path(
    "ida_preprocessor_scripts/"
    "find-CFlattenedSerializers_CreateFieldChangedEventQueue-impl.py"
)


def _load_module():
    spec = importlib.util.spec_from_file_location(
        "find_CFlattenedSerializers_CreateFieldChangedEventQueue_impl",
        SCRIPT_PATH,
    )
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


class TestFindCFlattenedSerializersCreateFieldChangedEventQueueImpl(
    unittest.IsolatedAsyncioTestCase
):
    async def test_preprocess_skill_forwards_expected_inherit_vfuncs(self) -> None:
        module = _load_module()
        session = AsyncMock()

        with patch.object(
            module,
            "preprocess_common_skill",
            new=AsyncMock(return_value=True),
        ) as mock_common:
            result = await module.preprocess_skill(
                session=session,
                skill_name="find-CFlattenedSerializers_CreateFieldChangedEventQueue-impl",
                expected_outputs=["out.yaml"],
                old_yaml_map={},
                new_binary_dir="/tmp/bin/14141/networksystem",
                platform="windows",
                image_base=0x140000000,
                debug=False,
            )

        self.assertTrue(result)
        _, kwargs = mock_common.await_args
        self.assertEqual(
            [
                (
                    "CFlattenedSerializers_CreateFieldChangedEventQueue",
                    "CFlattenedSerializers",
                    "../server/CFlattenedSerializers_CreateFieldChangedEventQueue",
                    True,
                )
            ],
            kwargs["inherit_vfuncs"],
        )


if __name__ == "__main__":
    unittest.main()
```

- [ ] **Step 2: Run the script-loader test to verify it fails**

Run: `python -m unittest tests.test_ida_preprocessor_scripts -v`

Expected: FAIL because the target preprocessor script does not exist yet

- [ ] **Step 3: Create the thin preprocessor script**

```python
#!/usr/bin/env python3
"""Preprocess script for find-CFlattenedSerializers_CreateFieldChangedEventQueue-impl skill."""

from ida_analyze_util import preprocess_common_skill

INHERIT_VFUNCS = [
    (
        "CFlattenedSerializers_CreateFieldChangedEventQueue",
        "CFlattenedSerializers",
        "../server/CFlattenedSerializers_CreateFieldChangedEventQueue",
        True,
    ),
]


async def preprocess_skill(
    session,
    skill_name,
    expected_outputs,
    old_yaml_map,
    new_binary_dir,
    platform,
    image_base,
    debug=False,
):
    """Resolve the implementation slot from the sibling-module YAML."""
    _ = skill_name

    return await preprocess_common_skill(
        session=session,
        expected_outputs=expected_outputs,
        old_yaml_map=old_yaml_map,
        new_binary_dir=new_binary_dir,
        platform=platform,
        image_base=image_base,
        inherit_vfuncs=INHERIT_VFUNCS,
        debug=debug,
    )
```

- [ ] **Step 4: Re-run the script-loader test**

Run: `python -m unittest tests.test_ida_preprocessor_scripts -v`

Expected: PASS with 1 test

### Task 4: Run focused regression checks and hand off

**Files:**
- Modify: `ida_analyze_bin.py`
- Modify: `ida_analyze_util.py`
- Create: `ida_preprocessor_scripts/find-CFlattenedSerializers_CreateFieldChangedEventQueue-impl.py`
- Create: `tests/test_ida_analyze_bin.py`
- Create: `tests/test_ida_analyze_util.py`
- Create: `tests/test_ida_preprocessor_scripts.py`

- [ ] **Step 1: Run the full focused unit-test set**

Run: `python -m unittest tests.test_ida_analyze_bin tests.test_ida_analyze_util tests.test_ida_preprocessor_scripts -v`

Expected: PASS with all targeted tests green

- [ ] **Step 2: Run a syntax-only sanity check on the touched runtime files**

Run: `python -m py_compile ida_analyze_bin.py ida_analyze_util.py ida_preprocessor_scripts/find-CFlattenedSerializers_CreateFieldChangedEventQueue-impl.py`

Expected: no output

- [ ] **Step 3: Record the implementation delta for handoff**

```text
- `ida_analyze_bin.py`: new safe artifact resolver + expected_input wiring
- `ida_analyze_util.py`: sibling-module base YAML resolution + vfunc_offset fallback
- `ida_preprocessor_scripts/find-CFlattenedSerializers_CreateFieldChangedEventQueue-impl.py`: declarative inherit_vfunc wrapper
- `tests/test_ida_analyze_bin.py`: resolver coverage
- `tests/test_ida_analyze_util.py`: cross-module inherit_vfunc coverage
- `tests/test_ida_preprocessor_scripts.py`: script wiring coverage
```

- [ ] **Step 4: Stop before any commit unless the user explicitly asks**

Run: `git status --short`

Expected: only the six planned files above appear as modified or new
