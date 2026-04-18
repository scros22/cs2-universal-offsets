# IDA VCall Finder Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 `ida_analyze_bin.py` 增加 `-vcall_finder` 流程，使脚本能导出对象引用函数的完整反汇编与伪代码，并在所有 IDA 任务结束后调用 OpenAI 聚合虚调用结果、回写 `found_vcall` 缓存并流式生成对象级 `.txt` 汇总。

**Architecture:** 保持 `ida_analyze_bin.py` 作为唯一入口，在其中补充参数解析、模块选择和主流程调度；新增 `ida_vcall_finder.py` 封装路径规划、明细 YAML 写入、IDA `py_eval` 导出和 OpenAI 聚合。模块是否启动 IDA 需要同时考虑待执行 `skills` 与命中的 `vcall_finder` 对象，不能再仅靠既有 `skills` 产物缓存来跳过；聚合阶段则以 detail YAML 顶层 `found_vcall` 作为缓存命中标记，命中时跳过 LLM 并直接回放到对象级 `.txt`。

**Tech Stack:** Python 3.10+、`argparse`、`PyYAML`、`mcp`、IDA `py_eval`、OpenAI Python SDK

---

## File Structure

- Modify: `ida_analyze_bin.py`
  - 增加 `-vcall_finder` / `-llm_model` / `-llm_apikey` / `-llm_baseurl`
  - 解析模块级 `vcall_finder`
  - 修正 IDA 启动判定
  - 在模块/平台级别挂接明细导出
  - 在全局收尾阶段挂接 OpenAI 聚合
- Create: `ida_vcall_finder.py`
  - 维护 `vcall_finder/{gamever}` 路径
  - 写入单函数明细 YAML
  - 生成 IDA `py_eval` 脚本并导出完整函数文本
  - 渲染 Prompt、解析 LLM YAML、回写 detail YAML 中的 `found_vcall`
  - 将 `found_vcall` 条目流式追加到对象级汇总 `.txt`
- Modify: `pyproject.toml`
  - 新增 `openai` 依赖
- Modify: `README_CN.md`
  - 补充 CLI 参数、共享 LLM 参数、输出路径示例
- Modify: `README.md`
  - 同步英文说明，避免双文档行为分叉

说明：仓库当前没有现成的 Python 单元测试目录，本计划使用“先写失败的命令行/内联 Python 验证，再实现，再重跑验证”的方式做最小 TDD，不新增新的测试框架。

### Task 1: 扩展参数解析与模块对象筛选

**Files:**
- Modify: `ida_analyze_bin.py`
- Verify: 本任务 Step 1 / Step 3 中的 `uv run python - <<'PY'`

- [ ] **Step 1: 先写失败的参数/配置探针**

Run:

```bash
uv run python - <<'PY'
import sys
sys.argv = [
    "ida_analyze_bin.py",
    "-gamever=14141",
    "-vcall_finder=g_pNetworkMessages",
    "-llm_model=gpt-4.1",
    "-llm_apikey=test-key",
    "-llm_baseurl=https://api.example.com/v1",
]
from ida_analyze_bin import parse_args
parse_args()
PY
```

Expected: FAIL，错误中包含 `unrecognized arguments: -vcall_finder=g_pNetworkMessages -llm_model=gpt-4.1 -llm_apikey=test-key -llm_baseurl=https://api.example.com/v1`

- [ ] **Step 2: 在 `ida_analyze_bin.py` 增加参数解析和模块筛选辅助函数**

在常量区和 `parse_args()` / `parse_config()` 附近加入以下代码：

```python
DEFAULT_LLM_MODEL = "gpt-4o"


def parse_vcall_finder_filter(raw_value):
    """Parse -vcall_finder into {'all': bool, 'names': set[str]} or None."""
    if raw_value is None:
        return None

    value = raw_value.strip()
    if not value:
        return None

    if value == "*":
        return {"all": True, "names": set()}

    names = {name.strip() for name in value.split(",") if name.strip()}
    if not names:
        raise ValueError("vcall_finder filter cannot be empty")

    return {"all": False, "names": names}


def resolve_module_vcall_targets(module, selector):
    """Return configured object names in this module that match the CLI selector."""
    configured = list(module.get("vcall_finder_objects", []))
    if not selector:
        return []
    if selector["all"]:
        return configured
    return [name for name in configured if name in selector["names"]]
```

在 `parse_args()` 里补参数和归一化逻辑：

```python
    parser.add_argument(
        "-vcall_finder",
        default=None,
        help="Object selector for vcall_finder. Use '*' for all configured objects or a comma-separated list."
    )
    parser.add_argument(
        "-llm_model",
        default=DEFAULT_LLM_MODEL,
        help=f"OpenAI-compatible model for preprocessing and vcall_finder workflow (default: {DEFAULT_LLM_MODEL})"
    )
    parser.add_argument(
        "-llm_apikey",
        default=None,
        help="OpenAI-compatible API key used by preprocessing and vcall_finder aggregation"
    )
    parser.add_argument(
        "-llm_baseurl",
        default=None,
        help="Optional OpenAI-compatible base URL used by preprocessing and vcall_finder aggregation"
    )
```

```python
    try:
        args.vcall_finder_filter = parse_vcall_finder_filter(args.vcall_finder)
    except ValueError as exc:
        parser.error(str(exc))
```

在 `parse_config()` 的模块字典里加入：

```python
            "vcall_finder_objects": module.get("vcall_finder", []) or [],
```

- [ ] **Step 3: 重跑探针并确认参数与配置解析通过**

Run:

```bash
uv run python - <<'PY'
import sys
import tempfile
import textwrap
from pathlib import Path

sys.argv = [
    "ida_analyze_bin.py",
    "-gamever=14141",
    "-vcall_finder=g_pNetworkMessages,foo",
    "-llm_model=gpt-4.1",
    "-llm_apikey=test-key",
    "-llm_baseurl=https://api.example.com/v1",
]

from ida_analyze_bin import parse_args, parse_config, resolve_module_vcall_targets

args = parse_args()
assert args.vcall_finder_filter == {"all": False, "names": {"g_pNetworkMessages", "foo"}}
assert args.vcall_finder_model == "gpt-4.1"
assert args.vcall_finder_apikey == "test-key"
assert args.vcall_finder_baseurl == "https://api.example.com/v1"

with tempfile.TemporaryDirectory() as tmpdir:
    config_path = Path(tmpdir) / "config.yaml"
    config_path.write_text(textwrap.dedent("""
    modules:
      - name: networksystem
        vcall_finder:
          - g_pNetworkMessages
      - name: engine
        vcall_finder:
          - g_pNetworkMessages
          - g_pGameRules
    """).strip() + "\n", encoding="utf-8")
    modules = parse_config(config_path)
    assert resolve_module_vcall_targets(modules[0], args.vcall_finder_filter) == ["g_pNetworkMessages"]
    assert resolve_module_vcall_targets(modules[1], args.vcall_finder_filter) == ["g_pNetworkMessages"]

print("ok")
PY
```

Expected: PASS，输出 `ok`

- [ ] **Step 4: 提交本任务**

```bash
git add ida_analyze_bin.py
git commit -m "feat(ida): 增加 vcall_finder 参数解析"
```

### Task 2: 新建 `ida_vcall_finder.py` 的纯数据与 YAML 辅助层

**Files:**
- Create: `ida_vcall_finder.py`
- Verify: 本任务 Step 1 / Step 3 中的 `uv run python - <<'PY'`

- [ ] **Step 1: 先写失败的 helper 导入探针**

Run:

```bash
uv run python - <<'PY'
from ida_vcall_finder import (
    build_vcall_detail_path,
    build_vcall_summary_path,
    render_vcall_prompt,
    parse_llm_vcall_response,
)
print(build_vcall_detail_path)
PY
```

Expected: FAIL，错误中包含 `ModuleNotFoundError: No module named 'ida_vcall_finder'`

- [ ] **Step 2: 创建 `ida_vcall_finder.py` 并实现路径、Prompt、YAML 解析、detail 回写与 TXT 汇总辅助函数**

写入以下基础代码：

```python
#!/usr/bin/env python3

import re
from pathlib import Path

import yaml


VCALL_FINDER_DIRNAME = "vcall_finder"

PROMPT_TEMPLATE = """You are a reverse engineering expert. I have disassembly outputs and procedure code of the same function.

**Disassembly**

```c
{disasm_code}
```

**Procedure code**

```c
{procedure}
```

Please collect all virtual function calls for "{object_name}" and output those calls as YAML

Example:

```yaml
found_vcall:
  - insn_va: 0x12345678
    insn_disasm: call    [rax+68h]
    vfunc_offset: 0x68
  - insn_va: 0x12345680
    insn_disasm: call    rax
    vfunc_offset: 0x80
```

If there are no virtual function calls for "{object_name}" found, output an empty YAML.
"""


class LiteralDumper(yaml.SafeDumper):
    pass


def _literal_str_representer(dumper, value):
    style = "|" if "\n" in value else None
    return dumper.represent_scalar("tag:yaml.org,2002:str", value, style=style)


LiteralDumper.add_representer(str, _literal_str_representer)


def build_vcall_root(base_dir=VCALL_FINDER_DIRNAME):
    return Path(base_dir)


def build_vcall_detail_path(base_dir, gamever, object_name, module_name, platform, func_name):
    return Path(base_dir) / gamever / object_name / module_name / platform / f"{func_name}.yaml"


def build_vcall_summary_path(base_dir, gamever, object_name):
    return Path(base_dir) / gamever / f"{object_name}.txt"


def write_vcall_detail_yaml(path, detail):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    payload = {
        "object_name": detail["object_name"],
        "module": detail["module"],
        "platform": detail["platform"],
        "func_name": detail["func_name"],
        "func_va": detail["func_va"],
        "disasm_code": detail.get("disasm_code", ""),
        "procedure": detail.get("procedure", ""),
    }
    with path.open("w", encoding="utf-8") as f:
        yaml.dump(payload, f, Dumper=LiteralDumper, sort_keys=False, allow_unicode=True)


def load_yaml_file(path):
    path = Path(path)
    if not path.exists():
        return {}
    with path.open("r", encoding="utf-8") as f:
        return yaml.safe_load(f) or {}


def render_vcall_prompt(detail):
    return PROMPT_TEMPLATE.format(
        object_name=detail["object_name"],
        disasm_code=detail.get("disasm_code", ""),
        procedure=detail.get("procedure", ""),
    )


def normalize_found_vcalls(entries):
    normalized = []
    for entry in entries or []:
        if not isinstance(entry, dict):
            continue
        normalized.append({
            "insn_va": str(entry.get("insn_va", "")),
            "insn_disasm": str(entry.get("insn_disasm", "")),
            "vfunc_offset": str(entry.get("vfunc_offset", "")),
        })
    return normalized


def parse_llm_vcall_response(response_text):
    match = re.search(r"```(?:yaml)?\\s*(.*?)\\s*```", response_text or "", re.DOTALL)
    yaml_text = match.group(1).strip() if match else (response_text or "").strip()
    if not yaml_text:
        return {"found_vcall": []}

    parsed = yaml.safe_load(yaml_text) or {}
    if not isinstance(parsed, dict):
        return {"found_vcall": []}

    return {"found_vcall": normalize_found_vcalls(parsed.get("found_vcall", []))}


def update_vcall_detail_with_found_vcalls(path, detail, found_vcall):
    payload = dict(detail)
    payload["found_vcall"] = normalize_found_vcalls(found_vcall)
    with Path(path).open("w", encoding="utf-8") as f:
        yaml.dump(payload, f, Dumper=LiteralDumper, sort_keys=False, allow_unicode=True)


def build_vcall_summary_entries(detail, found_vcall):
    base_entry = {
        "object_name": detail["object_name"],
        "module": detail["module"],
        "platform": detail["platform"],
        "func_name": detail["func_name"],
        "func_va": detail["func_va"],
    }
    entries = []
    for item in normalize_found_vcalls(found_vcall):
        entry = dict(base_entry)
        entry.update(item)
        entries.append(entry)
    return entries


def append_vcall_summary_entries(path, detail, found_vcall):
    entries = build_vcall_summary_entries(detail, found_vcall)
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("a", encoding="utf-8") as f:
        for entry in entries:
            yaml.dump(entry, f, Dumper=LiteralDumper, sort_keys=False, allow_unicode=True, explicit_start=True)
    return len(entries)
```

- [ ] **Step 3: 运行 helper 自检，确认路径、Prompt、解析、detail 回写和 TXT 追加正常**

Run:

```bash
uv run python - <<'PY'
import tempfile
from pathlib import Path

from ida_vcall_finder import (
    append_vcall_summary_entries,
    build_vcall_detail_path,
    build_vcall_summary_path,
    parse_llm_vcall_response,
    render_vcall_prompt,
    load_yaml_file,
    update_vcall_detail_with_found_vcalls,
    write_vcall_detail_yaml,
)

detail = {
    "object_name": "g_pNetworkMessages",
    "module": "networksystem",
    "platform": "windows",
    "func_name": "sub_140123450",
    "func_va": "0x140123450",
    "disasm_code": "mov rax, [rcx]\\ncall qword ptr [rax+68h]",
    "procedure": "return (*(*this + 0x68))(this);",
}

with tempfile.TemporaryDirectory() as tmpdir:
    detail_path = build_vcall_detail_path(tmpdir, "14141", "g_pNetworkMessages", "networksystem", "windows", "sub_140123450")
    write_vcall_detail_yaml(detail_path, detail)
    loaded_detail = load_yaml_file(detail_path)
    assert loaded_detail["module"] == "networksystem"
    prompt = render_vcall_prompt(loaded_detail)
    assert 'Please collect all virtual function calls for "g_pNetworkMessages"' in prompt

    parsed = parse_llm_vcall_response("""```yaml
found_vcall:
  - insn_va: 0x140123478
    insn_disasm: call    qword ptr [rax+68h]
    vfunc_offset: 0x68
```""")
    update_vcall_detail_with_found_vcalls(detail_path, loaded_detail, parsed["found_vcall"])
    cached_detail = load_yaml_file(detail_path)
    assert cached_detail["found_vcall"][0]["vfunc_offset"] == "0x68"
    summary_path = build_vcall_summary_path(tmpdir, "14141", "g_pNetworkMessages")
    append_vcall_summary_entries(summary_path, cached_detail, cached_detail["found_vcall"])
    assert summary_path.name == "g_pNetworkMessages.txt"
    summary_text = Path(summary_path).read_text(encoding="utf-8")
    assert "vfunc_offset: 0x68" in summary_text

print("ok")
PY
```

Expected: PASS，输出 `ok`

- [ ] **Step 4: 提交本任务**

```bash
git add ida_vcall_finder.py
git commit -m "feat(ida): 新增 vcall_finder 辅助模块"
```

### Task 3: 在 helper 中实现 IDA `py_eval` 导出

**Files:**
- Modify: `ida_vcall_finder.py`
- Verify: 本任务 Step 1 / Step 3 中的 `uv run python - <<'PY'`

- [ ] **Step 1: 先写失败的导出接口探针**

Run:

```bash
uv run python - <<'PY'
from ida_vcall_finder import (
    build_object_xref_py_eval,
    build_function_dump_py_eval,
    export_object_xref_details_via_mcp,
)
print(build_object_xref_py_eval)
print(build_function_dump_py_eval)
print(export_object_xref_details_via_mcp)
PY
```

Expected: FAIL，错误中包含 `cannot import name 'build_object_xref_py_eval'`

- [ ] **Step 2: 在 `ida_vcall_finder.py` 增加对象 xref 扫描和完整函数导出**

在现有 helper 后追加以下接口：

```python
import json

from ida_analyze_util import parse_mcp_result


def build_object_xref_py_eval(object_name):
    return f'''
import ida_funcs, ida_name, idaapi, idautils, json

object_name = {object_name!r}
object_ea = ida_name.get_name_ea(idaapi.BADADDR, object_name)
if object_ea == idaapi.BADADDR:
    result = json.dumps({{"object_ea": None, "functions": []}})
else:
    seen = set()
    functions = []
    for xref in idautils.XrefsTo(object_ea, 0):
        func = ida_funcs.get_func(xref.frm)
        if func is None or func.start_ea in seen:
            continue
        seen.add(func.start_ea)
        functions.append({{
            "func_name": ida_funcs.get_func_name(func.start_ea),
            "func_va": hex(func.start_ea),
        }})
    functions.sort(key=lambda item: int(item["func_va"], 16))
    result = json.dumps({{"object_ea": hex(object_ea), "functions": functions}})
'''


def build_function_dump_py_eval(func_va):
    return f'''
import ida_funcs, ida_hexrays, ida_idaapi, ida_lines, ida_segment, idc, json

func_ea = {int(func_va)}

def format_address(ea):
    seg = ida_segment.getseg(ea)
    seg_name = ida_segment.get_segm_name(seg) if seg else ""
    return f"{{seg_name}}:{{ea:016X}}" if seg_name else f"{{ea:016X}}"

def get_disasm(start_ea):
    func = ida_funcs.get_func(start_ea)
    if func is None:
        return ""
    lines = []
    ea = func.start_ea
    while ea < func.end_ea:
        disasm_line = idc.generate_disasm_line(ea, 0) or ""
        lines.append(f"{{format_address(ea)}}                 {{ida_lines.tag_remove(disasm_line)}}")
        ea = idc.next_head(ea, func.end_ea)
        if ea == ida_idaapi.BADADDR:
            break
    return "\\n".join(lines)

def get_pseudocode(start_ea):
    if not ida_hexrays.init_hexrays_plugin():
        return ""
    cfunc = ida_hexrays.decompile(start_ea)
    if not cfunc:
        return ""
    return "\\n".join(ida_lines.tag_remove(line.line) for line in cfunc.get_pseudocode())

func = ida_funcs.get_func(func_ea)
if func is None:
    result = json.dumps(None)
else:
    result = json.dumps({{
        "func_name": ida_funcs.get_func_name(func.start_ea),
        "func_va": hex(func.start_ea),
        "disasm_code": get_disasm(func.start_ea),
        "procedure": get_pseudocode(func.start_ea),
    }})
'''


async def export_object_xref_details_via_mcp(
    session,
    *,
    output_root,
    gamever,
    module_name,
    platform,
    object_name,
    debug=False,
):
    query_result = await session.call_tool(
        name="py_eval",
        arguments={"code": build_object_xref_py_eval(object_name)},
    )
    parsed = parse_mcp_result(query_result) or {}
    if not isinstance(parsed, dict) or not parsed.get("object_ea"):
        return {"success": 0, "failed": 0, "skipped": 1}

    success = 0
    failed = 0
    skipped = 0

    for function in parsed.get("functions", []):
        detail_path = build_vcall_detail_path(
            output_root,
            gamever,
            object_name,
            module_name,
            platform,
            function["func_name"],
        )
        if detail_path.exists():
            skipped += 1
            continue

        dump_result = await session.call_tool(
            name="py_eval",
            arguments={"code": build_function_dump_py_eval(int(function["func_va"], 16))},
        )
        dump_data = parse_mcp_result(dump_result)
        if not isinstance(dump_data, dict):
            failed += 1
            continue

        write_vcall_detail_yaml(detail_path, {
            "object_name": object_name,
            "module": module_name,
            "platform": platform,
            "func_name": dump_data["func_name"],
            "func_va": dump_data["func_va"],
            "disasm_code": dump_data.get("disasm_code", ""),
            "procedure": dump_data.get("procedure", ""),
        })
        success += 1

    return {"success": success, "failed": failed, "skipped": skipped}
```

- [ ] **Step 3: 运行生成器探针并做语法检查**

Run:

```bash
uv run python - <<'PY'
from ida_vcall_finder import build_object_xref_py_eval, build_function_dump_py_eval

code1 = build_object_xref_py_eval("g_pNetworkMessages")
code2 = build_function_dump_py_eval(int("0x140123450", 16))

assert "idautils.XrefsTo" in code1
assert '"g_pNetworkMessages"' in code1
assert "idc.generate_disasm_line" in code2
assert "ida_hexrays.decompile" in code2

print("ok")
PY

uv run python -m py_compile ida_vcall_finder.py
```

Expected: PASS，第一条命令输出 `ok`，第二条命令无输出并返回 0

- [ ] **Step 4: 提交本任务**

```bash
git add ida_vcall_finder.py
git commit -m "feat(ida): 增加对象引用函数导出"
```

### Task 4: 将 `vcall_finder` 导出接入主流程并修正 IDA 跳过条件

**Files:**
- Modify: `ida_analyze_bin.py`
- Verify: 本任务 Step 1 / Step 3 中的 `uv run python - <<'PY'`
- Verify: 本任务 Step 3 中的 `uv run ida_analyze_bin.py -gamever=14141 -modules=networksystem -platform=windows -vcall_finder=g_pNetworkMessages -debug`

- [ ] **Step 1: 先写失败的启动判定探针**

Run:

```bash
uv run python - <<'PY'
from ida_analyze_bin import should_start_binary_processing
assert should_start_binary_processing([], ["g_pNetworkMessages"]) is True
print("ok")
PY
```

Expected: FAIL，错误中包含 `cannot import name 'should_start_binary_processing'`

- [ ] **Step 2: 在 `ida_analyze_bin.py` 增加启动判定与导出挂接**

先在 imports 中加入：

```python
from ida_vcall_finder import export_object_xref_details_via_mcp
```

再增加辅助函数：

```python
def should_start_binary_processing(skills_to_process, vcall_targets):
    """Start IDA when either skills or vcall_finder still has work to do."""
    return bool(skills_to_process or vcall_targets)
```

把 `process_binary()` 的签名改成：

```python
def process_binary(
    binary_path,
    skills,
    agent,
    host,
    port,
    ida_args,
    platform,
    debug=False,
    max_retries=3,
    old_binary_dir=None,
    gamever=None,
    module_name=None,
    vcall_targets=None,
    vcall_output_dir="vcall_finder",
):
```

把原来的早退逻辑：

```python
    if not skills_to_process:
        print(f"  All skills already have yaml files, skipping IDA startup")
        return success_count, fail_count, skip_count
```

改成：

```python
    vcall_targets = list(vcall_targets or [])

    if not should_start_binary_processing(skills_to_process, vcall_targets):
        print("  All skills already have yaml files and no vcall_finder targets remain, skipping IDA startup")
        return success_count, fail_count, skip_count
```

在 `try:` 中、`for skill_name, expected_outputs, skill_max_retries in skills_to_process:` 循环之后追加：

```python
        for object_name in vcall_targets:
            print(f"  Processing vcall_finder: {object_name}")
            try:
                export_stats = asyncio.run(
                    preprocess_single_vcall_object_via_mcp(
                        host=host,
                        port=port,
                        output_root=vcall_output_dir,
                        gamever=gamever,
                        module_name=module_name,
                        platform=platform,
                        object_name=object_name,
                        debug=debug,
                    )
                )
            except Exception as exc:
                fail_count += 1
                print(f"    Failed to export vcall_finder for {object_name}: {exc}")
                continue

            success_count += export_stats["success"]
            fail_count += export_stats["failed"]
            skip_count += export_stats["skipped"]
```

再在 `ida_analyze_bin.py` 中新增会话包装器：

```python
async def preprocess_single_vcall_object_via_mcp(
    host,
    port,
    output_root,
    gamever,
    module_name,
    platform,
    object_name,
    debug=False,
):
    server_url = f"http://{host}:{port}/mcp"
    async with httpx.AsyncClient(
        follow_redirects=True,
        timeout=httpx.Timeout(30.0, read=300.0),
        trust_env=False,
    ) as http_client:
        async with streamable_http_client(server_url, http_client=http_client) as (read_stream, write_stream, _):
            async with ClientSession(read_stream, write_stream) as session:
                await session.initialize()
                return await export_object_xref_details_via_mcp(
                    session,
                    output_root=output_root,
                    gamever=gamever,
                    module_name=module_name,
                    platform=platform,
                    object_name=object_name,
                    debug=debug,
                )
```

最后在 `main()` 调用 `process_binary()` 前先求出：

```python
            vcall_targets = resolve_module_vcall_targets(module, args.vcall_finder_filter)
```

并把 `gamever=gamever`、`module_name=module_name`、`vcall_targets=vcall_targets` 传进去。

- [ ] **Step 3: 先跑纯逻辑探针，再跑一次真实 smoke**

Run:

```bash
uv run python - <<'PY'
from ida_analyze_bin import should_start_binary_processing

assert should_start_binary_processing([], []) is False
assert should_start_binary_processing(["skill"], []) is True
assert should_start_binary_processing([], ["g_pNetworkMessages"]) is True

print("ok")
PY
```

Expected: PASS，输出 `ok`

再运行：

```bash
uv run ida_analyze_bin.py \
  -gamever=14141 \
  -modules=networksystem \
  -platform=windows \
  -vcall_finder=g_pNetworkMessages \
  -debug
```

Expected:

- 不会在 `networksystem/windows` 上打印 `All skills already have yaml files, skipping IDA startup`
- 会打印 `Processing vcall_finder: g_pNetworkMessages`
- 若对象存在，会在 `vcall_finder/14141/g_pNetworkMessages/networksystem/windows/` 下生成或跳过单函数 YAML

- [ ] **Step 4: 提交本任务**

```bash
git add ida_analyze_bin.py ida_vcall_finder.py
git commit -m "fix(ida): 修正 vcall_finder 的 IDA 跳过逻辑"
```

### Task 5: 实现 OpenAI 聚合、detail 缓存回写、TXT 汇总与依赖接线

**Files:**
- Modify: `ida_vcall_finder.py`
- Modify: `ida_analyze_bin.py`
- Modify: `pyproject.toml`
- Verify: 本任务 Step 1 / Step 3 中的 `uv run python - <<'PY'`
- Verify: `uv run python -m py_compile ida_analyze_bin.py ida_vcall_finder.py`

- [ ] **Step 1: 先写失败的聚合接口探针**

Run:

```bash
uv run python - <<'PY'
from ida_vcall_finder import aggregate_vcall_results_for_object
print(aggregate_vcall_results_for_object)
PY
```

Expected: FAIL，错误中包含 `cannot import name 'aggregate_vcall_results_for_object'`

- [ ] **Step 2: 增加 OpenAI 聚合函数、主流程收尾 hook 和依赖**

先在 `pyproject.toml` 的依赖列表加入：

```toml
    "openai",
```

再在 `ida_vcall_finder.py` 里加入：

```python
from openai import OpenAI


def create_openai_client(api_key, base_url=None):
    if not api_key:
        raise RuntimeError("-llm_apikey is required when -vcall_finder is enabled")

    client_kwargs = {"api_key": api_key}
    if base_url:
        client_kwargs["base_url"] = base_url

    return OpenAI(**client_kwargs)


def call_openai_for_vcalls(client, detail, model):
    response = client.chat.completions.create(
        model=model,
        messages=[
            {"role": "system", "content": "You are a reverse engineering expert."},
            {"role": "user", "content": render_vcall_prompt(detail)},
        ],
        temperature=0.1,
    )
    content = response.choices[0].message.content or ""
    return parse_llm_vcall_response(content)["found_vcall"]


def update_vcall_detail_with_found_vcalls(path, detail, found_vcall):
    payload = dict(detail)
    payload["found_vcall"] = normalize_found_vcalls(found_vcall)
    with Path(path).open("w", encoding="utf-8") as f:
        yaml.dump(payload, f, Dumper=LiteralDumper, sort_keys=False, allow_unicode=True)


def append_vcall_summary_entries(path, detail, found_vcall):
    entries = build_vcall_summary_entries(detail, found_vcall)
    with Path(path).open("a", encoding="utf-8") as f:
        for entry in entries:
            yaml.dump(entry, f, Dumper=LiteralDumper, sort_keys=False, allow_unicode=True, explicit_start=True)
    return len(entries)


def aggregate_vcall_results_for_object(
    *,
    base_dir,
    gamever,
    object_name,
    model,
    api_key=None,
    base_url=None,
    client=None,
    debug=False,
):
    detail_root = Path(base_dir) / gamever / object_name
    summary_path = build_vcall_summary_path(base_dir, gamever, object_name)
    llm_client = client or create_openai_client(api_key=api_key, base_url=base_url)
    summary_path.write_text("", encoding="utf-8")

    processed = 0
    failed = 0

    for detail_path in sorted(detail_root.glob("*/*/*.yaml")):
        detail = load_yaml_file(detail_path)
        if not detail:
            failed += 1
            continue
        try:
            if "found_vcall" in detail:
                found_vcall = normalize_found_vcalls(detail.get("found_vcall"))
            else:
                found_vcall = call_openai_for_vcalls(llm_client, detail, model)
                update_vcall_detail_with_found_vcalls(detail_path, detail, found_vcall)
        except Exception:
            failed += 1
            continue
        append_vcall_summary_entries(summary_path, detail, found_vcall)
        processed += 1

    return {"processed": processed, "failed": failed}
```

在 `ida_analyze_bin.py` imports 中加入：

```python
from ida_vcall_finder import aggregate_vcall_results_for_object
```

在 `main()` 的模块循环之前增加：

```python
    all_vcall_objects = set()
```

在每次算出 `vcall_targets` 后追加：

```python
            all_vcall_objects.update(vcall_targets)
```

在全部模块/平台处理完毕、总结输出前增加：

```python
    if args.vcall_finder_filter and all_vcall_objects:
        print("\nRunning vcall_finder OpenAI aggregation")
        for object_name in sorted(all_vcall_objects):
            try:
                stats = aggregate_vcall_results_for_object(
                    base_dir="vcall_finder",
                    gamever=gamever,
                    object_name=object_name,
                    model=args.vcall_finder_model,
                    api_key=args.vcall_finder_apikey,
                    base_url=args.vcall_finder_baseurl,
                    debug=debug,
                )
                total_success += stats["processed"]
                total_fail += stats["failed"]
            except Exception as exc:
                total_fail += 1
                print(f"  Failed to aggregate {object_name}: {exc}")
```

- [ ] **Step 3: 用 fake client 验证聚合逻辑，再做语法检查**

Run:

```bash
uv run python - <<'PY'
import tempfile

from ida_vcall_finder import (
    aggregate_vcall_results_for_object,
    build_vcall_detail_path,
    load_yaml_file,
    write_vcall_detail_yaml,
)


class FakeResponse:
    def __init__(self, content):
        self.choices = [type("Choice", (), {"message": type("Message", (), {"content": content})()})()]


class FakeCompletions:
    def create(self, **kwargs):
        return FakeResponse("""```yaml
found_vcall:
  - insn_va: 0x140123478
    insn_disasm: call    qword ptr [rax+68h]
    vfunc_offset: 0x68
```""")


class FakeChat:
    def __init__(self):
        self.completions = FakeCompletions()


class FakeClient:
    def __init__(self):
        self.chat = FakeChat()


with tempfile.TemporaryDirectory() as tmpdir:
    detail_path = build_vcall_detail_path(tmpdir, "14141", "g_pNetworkMessages", "networksystem", "windows", "sub_140123450")
    write_vcall_detail_yaml(detail_path, {
        "object_name": "g_pNetworkMessages",
        "module": "networksystem",
        "platform": "windows",
        "func_name": "sub_140123450",
        "func_va": "0x140123450",
        "disasm_code": "mov rax, [rcx]\\ncall qword ptr [rax+68h]",
        "procedure": "return (*(*this + 0x68))(this);",
    })

    stats = aggregate_vcall_results_for_object(
        base_dir=tmpdir,
        gamever="14141",
        object_name="g_pNetworkMessages",
        model="gpt-4o",
        client=FakeClient(),
    )
    assert stats == {"processed": 1, "failed": 0}

    detail = load_yaml_file(detail_path)
    assert detail["found_vcall"][0]["vfunc_offset"] == "0x68"

    summary_text = open(f"{tmpdir}/14141/g_pNetworkMessages.txt", "r", encoding="utf-8").read()
    assert "vfunc_offset: 0x68" in summary_text

    stats = aggregate_vcall_results_for_object(
        base_dir=tmpdir,
        gamever="14141",
        object_name="g_pNetworkMessages",
        model="gpt-4o",
        client=FakeClient(),
    )
    assert stats == {"processed": 1, "failed": 0}

print("ok")
PY

uv run python -m py_compile ida_analyze_bin.py ida_vcall_finder.py
```

Expected: PASS，第一条命令输出 `ok`，第二条命令无输出并返回 0

- [ ] **Step 4: 提交本任务**

```bash
git add pyproject.toml ida_analyze_bin.py ida_vcall_finder.py
git commit -m "feat(ida): 增加 vcall_finder 聚合流程"
```

### Task 6: 更新中英文文档并给出最终使用示例

**Files:**
- Modify: `README_CN.md`
- Modify: `README.md`
- Verify: `rg -n "vcall_finder|vcall_finder_model|vcall_finder_apikey|vcall_finder_baseurl|OPENAI_API_MODEL|OPENAI_API_KEY|OPENAI_API_BASE" README_CN.md README.md`

- [ ] **Step 1: 先写失败的文档探针**

Run:

```bash
rg -n "vcall_finder|vcall_finder_model|vcall_finder_apikey|vcall_finder_baseurl|OPENAI_API_MODEL|OPENAI_API_KEY|OPENAI_API_BASE" README_CN.md README.md
```

Expected: FAIL，返回码非 0，因为当前 README 还没有这些说明

- [ ] **Step 2: 在 `README_CN.md` 和 `README.md` 补 usage、专用 CLI 参数和输出示例**

在 `README_CN.md` 的 `ida_analyze_bin.py` 示例后补充：

````md
#### 2.1 可选：导出对象引用函数并聚合虚调用

```bash
uv run ida_analyze_bin.py -gamever 14141 -modules=networksystem -platform=windows -vcall_finder=g_pNetworkMessages -llm_model=gpt-4o -llm_apikey=your-key
uv run ida_analyze_bin.py -gamever 14141 -platform=windows,linux -vcall_finder=* -llm_model=gpt-4o -llm_apikey=your-key -llm_baseurl=https://api.example.com/v1
```

输出目录：

- `vcall_finder/14141/g_pNetworkMessages/networksystem/windows/sub_140123450.yaml`
- `vcall_finder/14141/g_pNetworkMessages.txt`

共享 LLM CLI 参数：

- `-llm_apikey`：启用基于 LLM 的流程时必需，包括 `vcall_finder` 聚合
- `-llm_baseurl`：可选，自定义兼容 base URL
- `-llm_model`：可选，默认 `gpt-4o`
- 不读取 `OPENAI_API_KEY` / `OPENAI_API_BASE` / `OPENAI_API_MODEL`
````

在 `README.md` 对应位置同步英文版本：

````md
#### 2.1 Optional: export object xref functions and aggregate virtual calls

```bash
uv run ida_analyze_bin.py -gamever 14141 -modules=networksystem -platform=windows -vcall_finder=g_pNetworkMessages -llm_model=gpt-4o -llm_apikey=your-key
uv run ida_analyze_bin.py -gamever 14141 -platform=windows,linux -vcall_finder=* -llm_model=gpt-4o -llm_apikey=your-key -llm_baseurl=https://api.example.com/v1
```

Output paths:

- `vcall_finder/14141/g_pNetworkMessages/networksystem/windows/sub_140123450.yaml`
- `vcall_finder/14141/g_pNetworkMessages.txt`

Shared LLM CLI parameters:

- `-llm_apikey`: required when an LLM-backed workflow is enabled, including `vcall_finder` aggregation
- `-llm_baseurl`: optional custom compatible base URL
- `-llm_model`: optional, defaults to `gpt-4o`
- `OPENAI_API_KEY` / `OPENAI_API_BASE` / `OPENAI_API_MODEL` are not read by `vcall_finder`
````

- [ ] **Step 3: 重跑文档探针**

Run:

```bash
rg -n "vcall_finder|vcall_finder_model|vcall_finder_apikey|vcall_finder_baseurl|OPENAI_API_MODEL|OPENAI_API_KEY|OPENAI_API_BASE" README_CN.md README.md
```

Expected: PASS，能同时匹配到中文和英文 README 中新增的说明

- [ ] **Step 4: 提交本任务**

```bash
git add README_CN.md README.md
git commit -m "docs(ida): 补充 vcall_finder 使用说明"
```
