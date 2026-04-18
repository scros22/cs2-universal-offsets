# `ida_analyze_bin.py` `run_skill` 输出错误标记检测设计

## 背景

当前 `ida_analyze_bin.py` 的 `run_skill(...)` 通过 `subprocess.run(...)` 执行 `claude` 或 `codex` 子进程。

现状存在两个限制：

- `debug=False` 时使用 `capture_output=True`，父进程能拿到完整输出，但不会实时打印子进程输出。
- `debug=True` 时不捕获输出，子进程可直接向终端实时打印，但父进程无法完整检查输出文本内容。

本次希望新增一条独立于返回码的失败判定：

- 若子进程 `returncode == 0`
- 但其 `stdout` 或 `stderr` 文本中出现独立的错误标记
- 则 `run_skill(...)` 仍应判定失败并进入现有重试逻辑

## 目标

- 在不破坏现有重试、超时、stdin 注入与 expected yaml 校验逻辑的前提下，增加基于输出文本的失败判定。
- `-debug` 模式下，保留子进程原本的实时终端输出体验，同时父进程仍能完整收集输出文本。
- 非 `-debug` 模式下，不向终端打印 agent 输出，但父进程仍能完整收集输出文本。
- 将 `stdout` 与 `stderr` 都纳入检测范围。
- 匹配以下独立错误标记时判定失败：
  - `Error`
  - `error`
  - `[ERROR]`
  - `**ERROR**`

## 非目标

- 不修改 skill prompt、agent 参数或重试策略本身。
- 不对错误文本做复杂分类、去重或严重级别判断。
- 不引入日志文件落盘。
- 不改变 expected yaml 缺失时的失败逻辑。

## 选定方案

采用“统一使用 `subprocess.Popen`，父进程边读取边缓存；`debug` 时额外实时转发到终端”的方案。

这是满足需求的最小充分方案：

- `debug=True` 时既能实时输出，又能保留完整文本；
- `debug=False` 时仍能静默收集完整文本；
- 可在子进程结束后统一执行文本扫描与失败判定。

## 详细设计

### 1. 进程执行模型

将 `run_skill(...)` 中实际执行子进程的逻辑从 `subprocess.run(...)` 改为 `subprocess.Popen(...)`。

统一设置：

- `stdout=subprocess.PIPE`
- `stderr=subprocess.PIPE`
- `stdin=subprocess.PIPE`（仅在需要 `agent_input` 时写入）
- `text=True`

父进程负责持续消费两条输出流，避免因为管道未及时读取导致阻塞。

### 2. 输出采集与终端转发

父进程维护两份缓冲：

- `stdout_chunks`
- `stderr_chunks`

行为规则：

- `debug=True`
  - 读取到 `stdout` 内容时，立即写回 `sys.stdout`
  - 读取到 `stderr` 内容时，立即写回 `sys.stderr`
  - 同时把文本追加到对应缓冲
- `debug=False`
  - 不向终端转发
  - 仅追加到对应缓冲

这样可以保证：

- `debug` 模式下用户仍能看到接近当前行为的实时输出
- 两种模式下父进程都能获得完整文本用于后处理

### 3. 错误标记匹配规则

对合并后的输出文本执行大小写不敏感的独立词匹配。

推荐正则：

```python
r"(?<![A-Za-z0-9])error(?![A-Za-z0-9])"
```

并使用：

```python
re.search(pattern, merged_output, flags=re.IGNORECASE)
```

该规则的语义：

- `Error`、`error`、`ERROR` 会命中
- `[ERROR]`、`**ERROR**` 会命中
- `myErrorCode`、`error123`、`XerrorY` 不命中

选择 `A-Za-z0-9` 作为边界字符集合，是为了满足“前后不能跟随其他字母或数字”的要求，而不把 `[`、`*`、空白、换行、标点视为阻断匹配的问题。

### 4. 检测范围

检测范围为：

```python
merged_output = stdout_text + "\n" + stderr_text
```

原因：

- agent 可能把错误文本打印到任意一个流
- 仅检查 `stderr` 会漏报
- 分别检查与合并检查在本需求下等价，但合并后实现更简单

### 5. 失败判定顺序

建议保持如下顺序：

1. 子进程超时：直接失败
2. 子进程返回码非零：直接失败
3. 输出文本命中错误标记：失败
4. `expected_yaml_paths` 缺失：失败
5. 以上均未触发：成功

这样可以维持现有失败优先级，并把“输出含错误标记”作为新增的独立失败原因。

### 6. 错误报告方式

当命中错误标记时，输出简洁提示，例如：

```text
    Error: Skill output contains error marker
```

可选地再补充一小段摘要，便于调试，例如首个命中片段的上下文截断文本，但不应一次性打印过长内容，以免污染日志。

### 7. 与现有重试逻辑的兼容性

新增失败原因应与当前失败分支一致：

- 命中错误标记后，不直接 `return False`
- 若还有剩余重试次数，继续打印 `Retrying with ...`
- 用现有 retry session / `codex exec resume --last` 机制重试

因此这次变更只扩展失败判定来源，不改变重试框架。

## 实现边界

本次实现只建议修改：

- `ida_analyze_bin.py` 中的 `run_skill(...)`

如需控制复杂度，可增加一个局部 helper，例如：

- `_run_process_capture_output(...)`
- `_output_contains_error_marker(...)`

但不建议把该逻辑拆到新模块，避免为单点需求引入额外抽象层。

## 风险与权衡

### 1. 误报风险

某些正常文本可能包含 `error` 单词，例如文档说明、示例代码或 prompt 提示语。

本次仍接受该风险，原因是：

- 用户已明确要求只要出现独立错误标记就算失败
- 该规则简单、可预测、易解释
- 若后续误报较多，再考虑加入 allowlist 或更严格的消息格式判断

### 2. 实时输出颗粒度

改为父进程转发后，实时性取决于子进程刷新行为与父进程读取方式；通常可接近原始体验，但并不保证逐字符完全一致。

本次可接受“近实时、按行或按块转发”的行为，不追求终端显示的字节级完全一致。

### 3. 双流读取复杂度

`Popen` + 双流消费比 `subprocess.run` 更复杂。

但这是同时满足“实时显示”和“完整采集”的必要成本，且改动范围仍局限在单个函数内。

## 验证方案

建议采用定向验证，不默认运行完整测试：

1. 构造一个返回码为 `0`、`stdout` 含 `Error` 的子进程，确认判定失败
2. 构造一个返回码为 `0`、`stderr` 含 `[ERROR]` 的子进程，确认判定失败
3. 构造一个返回码为 `0`、输出仅含 `myErrorCode` 或 `error123` 的子进程，确认不因正则误判失败
4. `debug=True` 时确认输出仍实时可见
5. `debug=False` 时确认输出不出现在终端
6. 确认 expected yaml 缺失与 returncode 非零逻辑仍保持原行为

## 结论

本次需求可以实现，推荐在 `run_skill(...)` 内以最小范围改为 `Popen` 驱动的“采集 + 可选转发 + 结束后正则判错”方案。

该方案能同时满足：

- `debug` 模式实时显示
- 两种模式都能完整收集输出
- 输出出现独立错误标记即失败
- 与现有超时、重试、产物校验逻辑兼容
