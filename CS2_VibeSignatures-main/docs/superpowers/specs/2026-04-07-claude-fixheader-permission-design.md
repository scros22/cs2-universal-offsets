# 为 `run_cpp_tests.py` 的 `claude.cmd -fixheader` 补充写入权限参数的设计

## 背景

当前仓库支持在 `run_cpp_tests.py` 中使用 `-fixheader` 调用外部 agent 自动修复 C++ header 的 vtable 声明差异。

在使用 `claude.cmd` 作为 agent 时，实际调用链已经支持把以下参数透传给 Claude CLI：

- `claude_allowed_tools`
- `claude_permission_mode`
- `claude_extra_args`

对应拼接位置在：

- `run_cpp_tests.py:338`
- `run_cpp_tests.py:340`

本次问题不是 vtable 差异内容错误，而是 `claude.cmd` 在尝试编辑目标文件时触发了写权限确认，导致输出中出现：

```text
It seems file write permission is being denied. Could you grant permission for writing to ...
```

虽然当前一次运行中最终仍显示 header fix agent completed successfully，但该现象说明 fixheader 自动化链路在非交互场景下存在不稳定因素：如果 Claude 进入人工确认分支，就可能无法稳定完成文件修改。

## 目标

- 通过现有参数能力，避免 `claude.cmd` 在 `-fixheader` 场景下再次弹出文件写入确认。
- 不修改 `run_cpp_tests.py` 的控制流与成功判定逻辑。
- 支持两种使用方式：
  - 命令行临时注入
  - `config.yaml` 测试项内长期固化

## 非目标

- 不修改 `run_cpp_tests.py` 的默认权限策略。
- 不新增文件是否实际改动的二次校验。
- 不调整 `vtable-fixer` prompt 内容。
- 不切换到更激进的全跳过权限方案，除非后续再次确认有必要。

## 方案对比

### 方案 A：只设置 `claude_permission_mode: acceptEdits`

优点：

- 改动最小。
- 与当前问题最直接对应。
- 适合本地自动化执行。

缺点：

- 若 Claude 还会因为工具白名单而拦截，单独设置该项可能仍不够。

### 方案 B：同时设置 `claude_permission_mode` 与 `claude_allowed_tools`

推荐值：

```yaml
claude_permission_mode: acceptEdits
claude_allowed_tools: Read,Edit,MultiEdit,Write
```

优点：

- 同时覆盖“编辑确认”和“工具使用限制”两类阻塞。

缺点：

- 放宽范围略大于当前已确认问题。

### 方案 C：通过 `claude_extra_args` 注入全局跳过权限参数

示例：

```yaml
claude_extra_args: --dangerously-skip-permissions
```

优点：

- 非交互自动化最彻底。

缺点：

- 风险最高。
- 不适合作为默认推荐。

## 选型

本次采用方案 A 作为首选设计：

```yaml
claude_permission_mode: acceptEdits
```

理由如下：

- 已足够对症当前暴露出的写入确认问题。
- 保持最小权限放宽。
- 与现有 `run_cpp_tests.py` 参数模型完全兼容，不需要代码改动。

若实际再次运行后仍出现工具权限阻塞，再升级到方案 B。

## 用户接口

### 命令行临时使用

推荐命令：

```bash
uv run run_cpp_tests.py -gamever %CS2_GAMEVER% -debug -fixheader -agent="claude.cmd" -claude_permission_mode acceptEdits
```

PowerShell 环境中 `%CS2_GAMEVER%` 可替换为对应变量形式或具体版本号。

### `config.yaml` 长期固化

在 `config.yaml:2503` 对应测试项中加入：

```yaml
cpp_tests:
  - name: IGameSystem_MSVC
    symbol: IGameSystem
    cpp: cpp_tests/igamesystem.cpp
    headers:
      - hl2sdk_cs2/game/shared/igamesystem.h
    claude_permission_mode: acceptEdits
```

该字段会在运行时由以下逻辑读取并透传给 Claude CLI：

- 读取覆盖值：`run_cpp_tests.py:805`
- 拼接命令参数：`run_cpp_tests.py:340`

## 执行流程

当 `cpp_tests` 项开启 `-fixheader` 且 `agent` 包含 `claude` 时：

1. `run_cpp_tests.py` 收集 vtable differences。
2. `_build_fix_prompt()` 生成 header 修复提示。
3. `run_fix_header_agent()` 构造 Claude CLI 命令。
4. 若 `claude_permission_mode` 已配置，则附加：

```text
--permission-mode acceptEdits
```

5. Claude 在编辑目标 header 或辅助 cpp 测试文件时不再进入人工确认分支。

## 验证方式

本设计的验收标准如下：

1. 再次运行 `-fixheader` 时，不再出现：

```text
Could you grant permission for writing to ...
```

2. 仍能正常进入：

```text
VTable differences detected; invoking agent ...
```

3. 目标文件能够被实际修改：

- `hl2sdk_cs2/game/shared/igamesystem.h`
- `cpp_tests/igamesystem.cpp`

4. 后续对应 C++ 编译测试仍可继续执行。

## 风险与回退

### 风险

- 某些 Claude CLI 版本除编辑确认外，还可能因为工具权限限制阻塞写入。
- 当前脚本尚未在 agent 返回成功后核验目标文件是否真的发生变更。

### 回退与升级

如果仅设置 `acceptEdits` 仍无法稳定写入，则升级为：

```yaml
claude_permission_mode: acceptEdits
claude_allowed_tools: Read,Edit,MultiEdit,Write
```

如果仍需更强自动化，再单独评估是否允许使用：

```yaml
claude_extra_args: --dangerously-skip-permissions
```

该升级不属于本次默认范围。

## 后续改进候选

本次设计刻意不进入实现，但后续可考虑补两项增强：

- 在 `run_fix_header_agent()` 成功后校验目标文件是否实际发生修改。
- 在 README 中补充 `claude_permission_mode` 与 `claude_allowed_tools` 的推荐示例。
