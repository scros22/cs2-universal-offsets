# RegisterEventListener_Abstract 预处理共享 Helper 设计

## 背景

当前 `ida_preprocessor_scripts/find-CLoopModeGame_OnEventMapCallbacks-client.py` 依赖 `llm_decompile` 从 `CLoopModeGame_RegisterEventMapInternal` 的参考样例中提取：

- `RegisterEventListener_Abstract`
- 多个 `CLoopModeGame_OnXXX` 回调函数

当 `CLoopModeGame_OnXXX` 数量较多时，LLM 返回的 `found_call` / `found_funcptr` 结果会漏掉部分符号，导致预处理结果不完整。

仓库中已经存在两类可复用模式：

- `ida_preprocessor_scripts/_registerconcommand.py`
  - 在受限范围内程序化收集调用候选，再做唯一性和输出校验
- `ida_preprocessor_scripts/_igamesystem_dispatch_common.py`
  - 使用共享 helper + 薄配置脚本的结构，复用稳定的枚举逻辑

本设计的目标是将 `RegisterEventListener_Abstract` 类模式改为完全程序化处理，并抽成未来可复用到 server 模块的共享 helper。

## 目标

- 新增一个共享 helper，用于从指定 source function 内程序化识别 `RegisterEventListener_Abstract` 及其批量注册的回调函数
- 将 `find-CLoopModeGame_OnEventMapCallbacks-client.py` 改为薄配置脚本，不再依赖 `llm_decompile`
- 以 `CLoopModeGame_RegisterEventMapInternal.{platform}.yaml` 为入口，而不是直接从字符串全局反查
- 对声明的目标事件采用半严格校验：
  - `target_specs` 中声明的目标必须全部找到
  - 每个声明目标必须唯一
  - 允许存在未声明的额外注册项
- 对 `RegisterEventListener_Abstract` 的定位采用双重确认：
  - 汇编层确认
  - Hex-Rays 伪代码层确认
- 若 `ida_hexrays` 不可用，则直接失败
- 保持现有 YAML 输出字段和下游消费方式不变

## 非目标

- 不修改 `update_gamedata.py` 或下游 gamedata 生成逻辑
- 不统一抽象所有“回调注册”模式，只覆盖 `RegisterEventListener_Abstract` 这一类稳定模式
- 不尝试兼容没有 `CLoopModeGame_RegisterEventMapInternal.{platform}.yaml` 入口的工作流
- 不对未声明的额外回调自动写 YAML

## 方案选择

本次选择的方案是：

- 以 `CLoopModeGame_RegisterEventMapInternal.{platform}.yaml` 提供的 `func_va` 作为唯一入口
- 在该 source function 内，通过锚点事件名定位并双重确认真实的 `RegisterEventListener_Abstract`
- 再在同一 source function 内枚举所有对该 callee 的调用
- 从每个调用恢复事件名和回调函数地址
- 最后按照 `target_specs` 做半严格映射并写出 YAML

未选方案及原因：

1. 字符串逐项全局反查
   - 重复扫描多
   - 不适合同时稳定输出 `RegisterEventListener_Abstract`
   - 不利于未来 server 模块复用

2. 仅依赖临时对象写入模式
   - 对编译器局部布局更敏感
   - 失去对真实注册 callee 的直接确认
   - 更容易误绑无关调用

## 架构

### 新增共享 helper

新增文件：

- `ida_preprocessor_scripts/_register_event_listener_abstract.py`

职责：

- 读取 source YAML 并获取 `func_va`
- 在 source function 内构建平台相关 `py_eval` 并收集候选项
- 双重确认 `RegisterEventListener_Abstract`
- 恢复每个注册调用的 `event_name` 与 `callback_va`
- 按 `target_specs` 做校验、函数信息查询与 YAML 写出
- 对真实函数做 best-effort rename

### 薄配置脚本

将 `ida_preprocessor_scripts/find-CLoopModeGame_OnEventMapCallbacks-client.py` 改为配置层，主要声明：

- `SOURCE_YAML_STEM = "CLoopModeGame_RegisterEventMapInternal"`
- `REGISTER_FUNC_TARGET_NAME = "RegisterEventListener_Abstract"`
- `ANCHOR_EVENT_NAME = "CLoopModeGame::OnClientPollNetworking"`
- `TARGET_SPECS`
- `GENERATE_YAML_DESIRED_FIELDS`

`preprocess_skill(...)` 只调用共享 helper。

未来 server 版本只需要新增一个类似的薄配置脚本，并替换入口 YAML、锚点事件和目标事件列表即可。

## Helper 接口

建议共享 helper 暴露主入口：

```python
async def preprocess_register_event_listener_abstract_skill(
    session,
    expected_outputs,
    new_binary_dir,
    platform,
    image_base,
    source_yaml_stem,
    register_func_target_name,
    anchor_event_name,
    target_specs,
    generate_yaml_desired_fields,
    register_func_rename_to=None,
    allow_extra_events=True,
    search_window_after_anchor=24,
    search_window_before_call=64,
    debug=False,
):
    ...
```

### 参数约束

- `source_yaml_stem`
  - 例如 `CLoopModeGame_RegisterEventMapInternal`
- `register_func_target_name`
  - 例如 `RegisterEventListener_Abstract`
- `anchor_event_name`
  - 用于锁定真实注册函数，client 侧为 `CLoopModeGame::OnClientPollNetworking`
- `target_specs`
  - 非空列表
  - 每项至少包含：
    - `target_name`
    - `event_name`
    - `rename_to` 可选
- `generate_yaml_desired_fields`
  - 与现有脚本保持一致
- `allow_extra_events`
  - 本设计固定按半严格模式使用，默认 `True`

## 中间候选结构

helper 在 Python 层只消费统一候选结构：

```python
{
    "register_func_va": "0x...",
    "items": [
        {
            "event_name": "CLoopModeGame::OnClientPollNetworking",
            "callback_va": "0x...",
            "call_ea": "0x...",
            "temp_base": "0x...",
            "temp_callback_slot": "0x...",
        }
    ],
}
```

说明：

- `register_func_va`
  - 已通过汇编层和 Hex-Rays 层双重确认
- `event_name`
  - 注册调用最后一个字符串参数
- `callback_va`
  - 临时对象 `+8` 槽位最终写入的真实回调函数地址
- `call_ea`
  - 该次注册调用的地址，便于调试
- `temp_base`
  - 第二个参数传入的 16-byte 临时对象基址
- `temp_callback_slot`
  - `temp_base + 8` 的地址，便于调试和后续校验

## RegisterEventListener_Abstract 定位策略

### 第一步：汇编层确认

仅在 `source_yaml_stem.{platform}.yaml` 对应的 source function 内扫描：

1. 查找 `anchor_event_name` 的精确字符串引用
2. 从每个字符串引用点向后在小窗口内寻找最近的 `call` / `jmp`
3. 恢复这些调用点的 callee
4. 要求所有锚点最终收敛到同一个唯一 callee

若失败则终止。

### 第二步：Hex-Rays 伪代码层确认

使用 `ida_hexrays` 反编译同一个 source function，并执行交叉确认：

1. 在伪代码调用表达式中查找最后一个参数等于 `anchor_event_name` 的调用
2. 解析该调用表达式的 callee
3. 要求其与汇编层恢复出的唯一 callee 完全一致

若 `ida_hexrays` 不可用，直接失败。

若 callee 不一致，也直接失败。

### 设计理由

仅依赖“字符串引用附近的调用”容易误绑无关函数。增加 Hex-Rays 层交叉确认后，helper 接受的候选必须同时满足：

- 汇编层能证明 source function 内确有该字符串触发的调用
- 伪代码层也能证明这个调用的真实 callee 就是目标函数

## 批量枚举策略

在唯一确认 `register_func_va` 后，仅在同一 source function 内枚举对该 callee 的所有调用点。

对每个调用点恢复：

- `event_name`
- 第二个参数对应的 16-byte 临时对象基址
- 临时对象 `+8` 槽位的回调函数地址 `callback_va`

helper 不扫描全程序，只扫描 source function，减少误报并保持运行成本可控。

## 平台恢复逻辑

平台差异只放在 `py_eval` 内部的参数恢复层，Python 层只消费统一结构。

### Windows

对每个调用点：

- 从 `rdx` 回溯第二个参数，优先识别 `lea rdx, [frame_var]`
- 从 call 前的 stack store 回溯最后一个字符串参数，例如：
  - `[rsp+disp] = reg`
  - `[rsp+disp] = imm`
- 以 `temp_base + 8` 为目标槽位，向前回溯其写入来源
- 若写入来源是寄存器，再继续回溯该寄存器的值
- 典型模式包括：
  - `lea rax, CLoopModeGame_OnClientPollNetworking`
  - `mov [rbp+var_8], rax`
  - `lea rdx, [rbp+var_10]`

### Linux

对每个调用点：

- 从 `rsi` 回溯第二个参数，优先识别 `lea rsi, [frame_var]`
- 最后一个字符串参数优先从最近的 `push reg/imm` 回溯
- 必要时兼容 call 前的 stack store
- 以 `temp_base + 8` 为目标槽位回溯回调函数地址
- 典型模式包括：
  - `lea rdx, CLoopModeGame_OnClientPollNetworking`
  - `mov [rbp+var_48], rdx`
  - `lea rsi, [rbp+var_50]`
  - `push aCloopmodegameO`

### 稳健性要求

对于任意命中的注册调用，只要被视为 `register_func_va` 的调用，就必须稳定恢复出：

- 唯一 `event_name`
- 唯一 `callback_va`

否则直接失败，而不是吞掉异常候选继续写结果。

## 匹配与校验

### 目标匹配规则

`target_specs` 的主键使用 `event_name`。

每个声明目标必须满足：

- 恰好命中 1 个候选
- 命中 0 个则失败
- 命中多个则失败

### 半严格模式

允许存在未声明的额外注册项：

- 额外项不导致失败
- 额外项不写 YAML
- `debug=True` 时可打印额外项，便于未来扩展 `target_specs`

### 空函数处理

某些额外回调可能是空函数，此时：

- 额外项允许存在，不导致失败
- 只要声明目标全部命中即可

如果某个声明目标本身是空函数，也按普通函数处理并输出 YAML。

## YAML 输出策略

### RegisterEventListener_Abstract

单独写出其 YAML，字段沿用现有模式：

- `func_name`
- `func_sig`
- `func_va`
- `func_rva`
- `func_size`

### CLoopModeGame_OnXXX

每个声明目标：

1. 通过 `callback_va` 查询函数边界
2. 如请求 `func_sig`，调用 `preprocess_gen_func_sig_via_mcp(...)`
3. 输出当前脚本已有字段：
   - `func_name`
   - `func_sig`
   - `func_va`
   - `func_rva`
   - `func_size`

这样下游消费不需要改动，只改变发现方式。

## 失败条件

以下情况直接返回失败：

- `source_yaml_stem.{platform}.yaml` 缺失或没有 `func_va`
- `target_specs` 非法或为空
- `generate_yaml_desired_fields` 缺失必需字段配置
- 无法唯一确认 `RegisterEventListener_Abstract`
- `ida_hexrays` 不可用
- Hex-Rays 层确认到的 callee 与汇编层不一致
- 某个声明目标缺失或歧义
- 某个声明目标无法恢复唯一 `event_name` 或唯一 `callback_va`
- 某个声明目标的 `callback_va` 不是函数起点，且无法查询到函数信息
- 请求 `func_sig` 时签名生成失败

## 与现有代码的关系

### 对 `_registerconcommand.py` 的借鉴

- 共享 helper + 薄配置脚本
- 在受限范围内收集候选，再进行严格校验
- 统一做输出路径解析、函数信息查询和 YAML 写出

### 对 `_igamesystem_dispatch_common.py` 的借鉴

- 平台相关扫描逻辑放在 `py_eval`
- Python 层只处理归一化后的结构化结果
- 未来新增调用脚本时只新增配置，不复制核心逻辑

## 预期落地方式

### 新增

- `ida_preprocessor_scripts/_register_event_listener_abstract.py`

### 修改

- `ida_preprocessor_scripts/find-CLoopModeGame_OnEventMapCallbacks-client.py`

### 未来新增

- server 模块对应的薄配置脚本

## 风险与权衡

1. Hex-Rays 依赖更强
   - 这是有意选择，用更高置信度换更低误绑概率

2. 平台恢复逻辑较细
   - helper 会比 `_registerconcommand.py` 更复杂
   - 但复杂度集中在共享 helper 内，比散落到多个脚本更可控

3. 半严格模式可能掩盖“出现了新的合法回调但尚未纳入配置”
   - 通过 `debug` 输出额外项来缓解
   - 这是为了兼容空函数和非目标回调而做的有意权衡

## 验收标准

- `find-CLoopModeGame_OnEventMapCallbacks-client.py` 不再依赖 `llm_decompile_specs`
- 新 helper 能从 `CLoopModeGame_RegisterEventMapInternal.{platform}.yaml` 出发稳定定位 `RegisterEventListener_Abstract`
- 新 helper 能稳定恢复 `target_specs` 中所有声明的 `CLoopModeGame_OnXXX`
- 允许存在未声明额外回调而不失败
- 产出的 YAML 字段与当前下游消费者兼容
- 结构允许未来在 server 模块新增薄配置脚本复用
