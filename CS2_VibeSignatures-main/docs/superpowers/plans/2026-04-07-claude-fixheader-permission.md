# Claude Fixheader Permission Mode Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 `IGameSystem_MSVC` 持久化 `claude_permission_mode: acceptEdits`，让 `claude.cmd` 在 `-fixheader` 场景下不再请求文件写入确认。

**Architecture:** 复用 `run_cpp_tests.py` 已经存在的 Claude CLI 参数透传能力，不修改 Python 代码。实现只落在 `config.yaml` 的单个 `cpp_tests` 测试项，然后用一次结构化配置校验和一次端到端回归验证确认配置已经生效。

**Tech Stack:** Python CLI (`uv run`), YAML (`config.yaml`), Claude Code CLI (`claude.cmd`)

---

## 文件结构

- Modify: `config.yaml`
  - 为 `cpp_tests -> IGameSystem_MSVC` 持久化 `claude_permission_mode: acceptEdits`
- Read-only reference: `run_cpp_tests.py`
  - 确认 `parse_config()` 会读取测试项字段
  - 确认 `run_fix_header_agent()` 会拼接 `--permission-mode`
- Verification target outputs:
  - `hl2sdk_cs2/game/shared/igamesystem.h`
  - `cpp_tests/igamesystem.cpp`

本计划不修改 `README.md`、`README_CN.md` 或 `run_cpp_tests.py`，保持最小改动范围。

### Task 1: 持久化 `IGameSystem_MSVC` 的 Claude 写权限模式

**Files:**
- Modify: `config.yaml`
- Verify: `run_cpp_tests.py`

- [ ] **Step 1: 先确认当前代码路径已经支持该配置字段**

Run:

```bash
rg -n "claude_permission_mode|--permission-mode" run_cpp_tests.py -n -S
```

Expected:

```text
run_cpp_tests.py:... claude_permission_mode
run_cpp_tests.py:... --permission-mode
```

- [ ] **Step 2: 修改 `config.yaml` 的 `IGameSystem_MSVC` 测试项**

在 `config.yaml` 的 `cpp_tests` 区块中，把 `IGameSystem_MSVC` 改成下面这个片段：

```yaml
cpp_tests:
  - name: IGameSystem_MSVC
    symbol: IGameSystem
    cpp: cpp_tests/igamesystem.cpp
    headers:
      - hl2sdk_cs2/game/shared/igamesystem.h # This is for LLM agent
    claude_permission_mode: acceptEdits
    target: x86_64-pc-windows-msvc
    include_directories:
      - hl2sdk_cs2/game/shared
      - hl2sdk_cs2/public
      - hl2sdk_cs2/public/tier0
      - hl2sdk_cs2/public/tier1
    defines:
      - COMPILER_MSVC=1
      - COMPILER_MSVC64=1
      - _MSVC_STL_USE_ABORT_AS_DOOM_FUNCTION
    additional_compiler_options:
      - fms-extensions
      - fms-compatibility
      - Xclang
      - fdump-vtable-layouts
    reference_modules:
      - server # bin/{gamever}/server/IGameSystem_*.{platform}.yaml
      - client # bin/{gamever}/client/IGameSystem_*.{platform}.yaml
```

- [ ] **Step 3: 用 `parse_config()` 做一次结构化校验**

Run:

```bash
uv run python - <<'PY'
from pathlib import Path
from run_cpp_tests import parse_config

items = parse_config(Path("config.yaml"))
item = next(test for test in items if test.get("name") == "IGameSystem_MSVC")
assert item.get("claude_permission_mode") == "acceptEdits", item
print(item["name"], item["claude_permission_mode"])
PY
```

Expected:

```text
IGameSystem_MSVC acceptEdits
```

- [ ] **Step 4: 检查工作区只出现预期配置改动**

Run:

```bash
git diff -- config.yaml
```

Expected:

```diff
+    claude_permission_mode: acceptEdits
```

- [ ] **Step 5: 提交这一步配置改动**

Run:

```bash
git add config.yaml
git commit -m "fix(cpp_tests): 为 fixheader 配置写权限模式"
git log -1 --pretty=%s
```

Expected:

```text
fix(cpp_tests): 为 fixheader 配置写权限模式
```

### Task 2: 做一次端到端 fixheader 回归验证

**Files:**
- Verify: `run_cpp_tests.py`
- Verify: `config.yaml`
- Observe: `hl2sdk_cs2/game/shared/igamesystem.h`
- Observe: `cpp_tests/igamesystem.cpp`

- [ ] **Step 1: 先用命令构造校验确认 `acceptEdits` 会被真正透传给 Claude CLI**

Run:

```bash
uv run python - <<'PY'
import run_cpp_tests

captured = {}

class DummyResult:
    returncode = 0
    stderr = ""

def fake_run(cmd, *args, **kwargs):
    captured["cmd"] = cmd
    return DummyResult()

original_run = run_cpp_tests.subprocess.run
run_cpp_tests.subprocess.run = fake_run
try:
    ok = run_cpp_tests.run_fix_header_agent(
        fix_prompt="permission-smoke-test",
        agent="claude.cmd",
        debug=False,
        max_retries=1,
        claude_permission_mode="acceptEdits",
    )
    assert ok is True
    cmd = captured["cmd"]
    assert "--permission-mode" in cmd, cmd
    index = cmd.index("--permission-mode")
    assert cmd[index + 1] == "acceptEdits", cmd
    print("PERMISSION_MODE_WIRED")
finally:
    run_cpp_tests.subprocess.run = original_run
PY
```

Expected:

```text
PERMISSION_MODE_WIRED
```

- [ ] **Step 2: 自动选择本地最新的游戏版本目录用于 live 验证**

Run:

```bash
export CS2_GAMEVER="$(find bin -mindepth 1 -maxdepth 1 -type d -printf '%f\n' | sort | tail -n 1)"
test -n "$CS2_GAMEVER"
echo "GAMEVER_SELECTED"
```

Expected:

```text
GAMEVER_SELECTED
```

- [ ] **Step 3: 在不传额外 CLI 权限参数的前提下直接跑 fixheader**

Run:

```bash
uv run run_cpp_tests.py -gamever "$CS2_GAMEVER" -debug -fixheader -agent="claude.cmd" 2>&1 | tee /tmp/claude-fixheader-permission.log
test -f /tmp/claude-fixheader-permission.log
echo "LIVE_RUN_RECORDED"
```

Expected:

```text
LIVE_RUN_RECORDED
```

- [ ] **Step 4: 确认日志里不再出现写入权限请求**

Run:

```bash
if rg -n "Could you grant permission for writing to|file write permission is being denied" /tmp/claude-fixheader-permission.log; then
  echo "UNEXPECTED_PERMISSION_PROMPT"
  exit 1
fi
echo "NO_PERMISSION_PROMPT"
```

Expected:

```text
NO_PERMISSION_PROMPT
```

- [ ] **Step 5: 确认 agent 阶段没有直接失败**

Run:

```bash
if rg -n "Agent failed with return code|Failed after" /tmp/claude-fixheader-permission.log; then
  echo "AGENT_FAILED"
  exit 1
fi
if rg -n "\\[PASS\\] Header fix agent completed successfully" /tmp/claude-fixheader-permission.log; then
  echo "AGENT_PASS_RECORDED"
else
  echo "NO_AGENT_RUN_RECORDED"
fi
```

Expected:

```text
Either:
- AGENT_PASS_RECORDED
Or:
- NO_AGENT_RUN_RECORDED
```

- [ ] **Step 6: 检查目标文件是否产生新的 header 变更**

Run:

```bash
if git diff --quiet -- hl2sdk_cs2/game/shared/igamesystem.h cpp_tests/igamesystem.cpp; then
  echo "NO_HEADER_DIFF"
else
  echo "HEADER_DIFF_PRESENT"
  git diff -- hl2sdk_cs2/game/shared/igamesystem.h cpp_tests/igamesystem.cpp
fi
```

Expected:

```text
Either:
- NO_HEADER_DIFF
Or:
- HEADER_DIFF_PRESENT
```

- [ ] **Step 7: 仅在 header 确实变化时追加提交**

Run:

```bash
if git diff --quiet -- hl2sdk_cs2/game/shared/igamesystem.h cpp_tests/igamesystem.cpp; then
  echo "NO_HEADER_COMMIT_NEEDED"
else
  git add hl2sdk_cs2/game/shared/igamesystem.h cpp_tests/igamesystem.cpp
  git commit -m "fix(cpp_tests): 同步 IGameSystem vtable 声明"
  git log -1 --pretty=%s
fi
```

Expected:

```text
Either:
- NO_HEADER_COMMIT_NEEDED
Or:
- fix(cpp_tests): 同步 IGameSystem vtable 声明
```

## 自检

### Spec coverage

- “通过现有参数能力避免弹写入确认” 对应 Task 1 Step 2。
- “命令行与 config 两种使用方式” 中，命令行已由现有脚本支持；本计划实现 config 固化，并在 Task 2 直接验证无需 CLI 额外参数也能运行。
- “不修改 `run_cpp_tests.py` 控制流” 通过文件结构与 Task 1 的范围限制体现。
- “出现问题时升级到 `claude_allowed_tools`” 未纳入本次实现，符合 spec 的非目标与回退策略。

### Placeholder scan

- 无 `TODO`、`TBD`、`implement later`。
- 所有修改步骤均给出精确文件路径、命令和代码片段。

### Type consistency

- 统一使用 `claude_permission_mode: acceptEdits`。
- 测试项名称统一为 `IGameSystem_MSVC`。
- CLI 名称统一为 `claude.cmd`。
