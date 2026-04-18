# Run Skill Output Error Detection Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 让 `ida_analyze_bin.py` 的 `run_skill(...)` 在两种 debug 模式下都能完整收集 agent 输出，并在输出命中独立 `error` 标记时即使返回码为 `0` 也判定失败。

**Architecture:** 先用 `unittest` 锁定四类行为：独立 `error` 标记匹配、`debug=True` 时转发 `stdout/stderr`、`debug=False` 时不转发 agent 输出、返回码为 `0` 但输出命中错误标记时进入重试。随后把 `run_skill(...)` 从 `subprocess.run(...)` 重构为 `subprocess.Popen(...) + threading` 的双流采集 helper，保留现有 Codex prompt 经 `stdin` 传递、超时处理和 expected yaml 校验逻辑。

**Tech Stack:** Python 3.10+、`unittest`、`unittest.mock`、`subprocess`、`threading`、`re`

---

## File Structure

- Modify: `ida_analyze_bin.py`
  - 新增 `re` / `threading` 导入
  - 新增独立错误标记正则和 `_output_contains_error_marker(...)`
  - 新增双流采集 helper，负责缓存 `stdout/stderr`，并在 `debug=True` 时实时写回父进程终端
  - 将 `run_skill(...)` 从 `subprocess.run(...)` 改为统一走新 helper
- Modify: `tests/test_ida_analyze_bin.py`
  - 新增 fake pipe / fake popen 测试夹具
  - 新增错误标记匹配测试
  - 新增 `debug=True` 转发、`debug=False` 静默、返回码为 `0` 但输出命中错误标记时重试的测试
  - 将原 Codex prompt 通过 `stdin` 传递的测试改为适配 `subprocess.Popen(...)`

**仓库约束：**

- 当前会话默认不执行 `git commit`
- 默认只做定向验证，不自动运行 build
- 实施阶段如需验证，优先执行 `tests/test_ida_analyze_bin.py` 的定向 `unittest`

### Task 1: 先补 `run_skill(...)` 的 failing tests

**Files:**
- Modify: `tests/test_ida_analyze_bin.py`

- [ ] **Step 1: 扩展测试文件导入，加入 fake process 夹具依赖**

把文件开头导入改成下面这样：

```python
import io
import unittest
from pathlib import Path
from unittest.mock import call, patch

import ida_analyze_bin
```

然后在 `TestRunSkillCodexPromptTransport` 之前加入这组测试夹具：

```python
class _FakePipe:
    def __init__(self, chunks: list[str]) -> None:
        self._chunks = list(chunks)

    def readline(self) -> str:
        return self._chunks.pop(0) if self._chunks else ""

    def close(self) -> None:
        return None


class _FakeStdin:
    def __init__(self) -> None:
        self.writes: list[str] = []
        self.closed = False

    def write(self, data: str) -> int:
        self.writes.append(data)
        return len(data)

    def flush(self) -> None:
        return None

    def close(self) -> None:
        self.closed = True


class _FakePopen:
    def __init__(
        self,
        *,
        stdout_chunks: list[str] | None = None,
        stderr_chunks: list[str] | None = None,
        returncode: int = 0,
    ) -> None:
        self.stdout = _FakePipe(stdout_chunks or [])
        self.stderr = _FakePipe(stderr_chunks or [])
        self.stdin = _FakeStdin()
        self.returncode = returncode
        self.killed = False

    def wait(self, timeout: int | None = None) -> int:
        return self.returncode

    def kill(self) -> None:
        self.killed = True
```

- [ ] **Step 2: 新增独立错误标记匹配测试**

在 `tests/test_ida_analyze_bin.py` 里新增下面的测试类，先锁定正则语义：

```python
class TestRunSkillOutputDetection(unittest.TestCase):
    def test_output_contains_error_marker_only_matches_standalone_tokens(self) -> None:
        self.assertTrue(ida_analyze_bin._output_contains_error_marker("Error"))
        self.assertTrue(ida_analyze_bin._output_contains_error_marker("prefix [ERROR] suffix"))
        self.assertTrue(ida_analyze_bin._output_contains_error_marker("before **ERROR** after"))
        self.assertTrue(ida_analyze_bin._output_contains_error_marker("line one\nerror\nline three"))

        self.assertFalse(ida_analyze_bin._output_contains_error_marker("myErrorCode"))
        self.assertFalse(ida_analyze_bin._output_contains_error_marker("error123"))
        self.assertFalse(ida_analyze_bin._output_contains_error_marker("XerrorY"))
        self.assertFalse(ida_analyze_bin._output_contains_error_marker("all good"))
```

- [ ] **Step 3: 新增 `debug=True` 时转发终端输出的测试**

继续在同一个测试类里加入：

```python
    @patch("ida_analyze_bin.os.path.exists", return_value=True)
    @patch("ida_analyze_bin.subprocess.Popen")
    def test_run_skill_debug_true_forwards_stdout_and_stderr(
        self,
        mock_popen,
        _mock_exists,
    ) -> None:
        mock_popen.return_value = _FakePopen(
            stdout_chunks=["agent stdout line\n"],
            stderr_chunks=["agent stderr line\n"],
            returncode=0,
        )

        with patch("sys.stdout", new_callable=io.StringIO) as fake_stdout, patch(
            "sys.stderr", new_callable=io.StringIO
        ) as fake_stderr:
            result = ida_analyze_bin.run_skill(
                skill_name="find-IGameSystem_vtable",
                agent="claude",
                debug=True,
                max_retries=1,
            )

        self.assertTrue(result)
        self.assertIn("agent stdout line\n", fake_stdout.getvalue())
        self.assertIn("agent stderr line\n", fake_stderr.getvalue())
```

- [ ] **Step 4: 新增 `debug=False` 静默收集与错误标记触发重试的测试**

继续追加：

```python
    @patch.object(Path, "read_text", return_value="sig finder prompt")
    @patch("ida_analyze_bin.os.path.exists", return_value=True)
    @patch("ida_analyze_bin.subprocess.Popen")
    def test_run_skill_retries_when_output_contains_error_marker(
        self,
        mock_popen,
        _mock_exists,
        _mock_read_text,
    ) -> None:
        first_process = _FakePopen(
            stdout_chunks=["starting\n", "[ERROR] lookup failed\n"],
            stderr_chunks=[],
            returncode=0,
        )
        second_process = _FakePopen(
            stdout_chunks=["done\n"],
            stderr_chunks=[],
            returncode=0,
        )
        mock_popen.side_effect = [first_process, second_process]

        with patch("sys.stdout", new_callable=io.StringIO) as fake_stdout, patch(
            "sys.stderr", new_callable=io.StringIO
        ) as fake_stderr:
            result = ida_analyze_bin.run_skill(
                skill_name="find-IGameSystem_vtable",
                agent="codex",
                debug=False,
                max_retries=2,
            )

        self.assertTrue(result)
        self.assertEqual(2, mock_popen.call_count)
        self.assertNotIn("[ERROR] lookup failed\n", fake_stdout.getvalue())
        self.assertEqual("", fake_stderr.getvalue())
        expected_prompt = "Run SKILL: .claude/skills/find-IGameSystem_vtable/SKILL.md"
        self.assertEqual([expected_prompt], first_process.stdin.writes)
        self.assertEqual([expected_prompt], second_process.stdin.writes)
        self.assertTrue(first_process.stdin.closed)
        self.assertTrue(second_process.stdin.closed)
```

- [ ] **Step 5: 运行新增测试，确认先失败**

Run:

```bash
uv run python -m unittest tests.test_ida_analyze_bin.TestRunSkillOutputDetection -v
```

Expected:

```text
FAIL: test_output_contains_error_marker_only_matches_standalone_tokens
AttributeError: module 'ida_analyze_bin' has no attribute '_output_contains_error_marker'
```

### Task 2: 在 `ida_analyze_bin.py` 实现输出采集与错误标记检测

**Files:**
- Modify: `ida_analyze_bin.py`

- [ ] **Step 1: 增加正则与独立错误标记 helper**

在导入区加入：

```python
import re
import threading
```

在常量区 `SKILL_TIMEOUT = 1200` 下方加入：

```python
ERROR_MARKER_RE = re.compile(
    r"(?<![A-Za-z0-9])error(?![A-Za-z0-9])",
    re.IGNORECASE,
)


def _output_contains_error_marker(*texts: str) -> bool:
    merged_output = "\n".join(text for text in texts if text)
    return bool(ERROR_MARKER_RE.search(merged_output))
```

- [ ] **Step 2: 增加双流采集 helper，支持 debug 转发**

在 `run_skill(...)` 之前加入下面两个 helper：

```python
def _drain_text_stream(stream, chunks, forward_stream=None):
    try:
        for chunk in iter(stream.readline, ""):
            chunks.append(chunk)
            if forward_stream is not None:
                forward_stream.write(chunk)
                forward_stream.flush()
    finally:
        try:
            stream.close()
        except Exception:
            pass


def _run_process_with_stream_capture(cmd, *, agent_input=None, debug=False, timeout=SKILL_TIMEOUT):
    process = subprocess.Popen(
        cmd,
        stdin=subprocess.PIPE if agent_input is not None else None,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )

    if agent_input is not None and process.stdin is not None:
        process.stdin.write(agent_input)
        process.stdin.flush()
        process.stdin.close()

    stdout_chunks = []
    stderr_chunks = []
    stdout_thread = threading.Thread(
        target=_drain_text_stream,
        args=(process.stdout, stdout_chunks, sys.stdout if debug else None),
    )
    stderr_thread = threading.Thread(
        target=_drain_text_stream,
        args=(process.stderr, stderr_chunks, sys.stderr if debug else None),
    )
    stdout_thread.start()
    stderr_thread.start()

    try:
        process.wait(timeout=timeout)
    except subprocess.TimeoutExpired:
        process.kill()
        stdout_thread.join(timeout=1)
        stderr_thread.join(timeout=1)
        raise

    stdout_thread.join()
    stderr_thread.join()
    return subprocess.CompletedProcess(
        args=cmd,
        returncode=process.returncode,
        stdout="".join(stdout_chunks),
        stderr="".join(stderr_chunks),
    )
```

- [ ] **Step 3: 把 `run_skill(...)` 改为统一走新 helper**

把当前这段：

```python
        try:
            run_kwargs = {"timeout": SKILL_TIMEOUT}
            if agent_input is not None:
                run_kwargs["input"] = agent_input
                run_kwargs["text"] = True

            if debug:
                result = subprocess.run(cmd, **run_kwargs)
            else:
                run_kwargs["capture_output"] = True
                run_kwargs.setdefault("text", True)
                result = subprocess.run(cmd, **run_kwargs)
```

替换成：

```python
        try:
            result = _run_process_with_stream_capture(
                cmd,
                agent_input=agent_input,
                debug=debug,
                timeout=SKILL_TIMEOUT,
            )
```

紧接着在 `returncode` 检查之后、expected yaml 检查之前插入：

```python
            if _output_contains_error_marker(result.stdout, result.stderr):
                print("    Error: Skill output contains error marker")
                if attempt < max_retries - 1:
                    print(f"    Retrying with {retry_target_desc}...")
                continue
```

保留原有：

- `returncode != 0` 时输出返回码与 `stderr[:500]`
- `expected_yaml_paths` 缺失时失败
- `TimeoutExpired` / `FileNotFoundError` / 其他异常时的现有错误分支

- [ ] **Step 4: 运行针对实现的完整定向测试**

Run:

```bash
uv run python -m unittest tests.test_ida_analyze_bin -v
```

Expected:

```text
OK
```

### Task 3: 做一次行为回归核对并检查改动边界

**Files:**
- Modify: `ida_analyze_bin.py`
- Modify: `tests/test_ida_analyze_bin.py`

- [ ] **Step 1: 用单独命令复核 `debug=False` 静默收集路径**

Run:

```bash
uv run python -m unittest \
  tests.test_ida_analyze_bin.TestRunSkillOutputDetection.test_run_skill_retries_when_output_contains_error_marker \
  -v
```

Expected:

```text
ok
```

- [ ] **Step 2: 用单独命令复核 `debug=True` 输出转发路径**

Run:

```bash
uv run python -m unittest \
  tests.test_ida_analyze_bin.TestRunSkillOutputDetection.test_run_skill_debug_true_forwards_stdout_and_stderr \
  -v
```

Expected:

```text
ok
```

- [ ] **Step 3: 检查最终 diff，只包含目标文件**

Run:

```bash
git diff --stat -- ida_analyze_bin.py tests/test_ida_analyze_bin.py
```

Expected:

```text
只出现 `ida_analyze_bin.py` 和 `tests/test_ida_analyze_bin.py` 两个目标文件
```

- [ ] **Step 4: 记录未执行项，不宣称 build 或全量测试通过**

在交付说明中明确写出：

```text
已完成 run_skill 输出采集与 error 标记失败判定的定向单测验证。
未运行 build、未运行与本次改动无关的全量测试。
```
