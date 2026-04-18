# llm_decompile 支持 found_funcptr 通用修复设计

## 背景

`ida_preprocessor_scripts/find-CLoopModeGame_OnEventMapCallbacks-client.py` 依赖 `LLM_DECOMPILE` 从 `CLoopModeGame_RegisterEventMapInternal` 中定位一组事件回调函数。

其中一类典型模式不是直接调用：

```c
v40 = sub_15BC910;
CLoopModeGame_RegisterEventMapInternal(..., "CLoopModeGame::OnClientPollNetworking");
```

对应反汇编通常表现为：

```asm
lea     rax, CLoopModeGame_OnClientPollNetworking
```

这类语句本质上是“函数指针装载”，不是直接 `call`，也不是全局变量访问。当前公共 `llm_decompile` 链路虽然在 prompt 示例中已经出现 `found_funcptr`，但下游并没有把它作为正式协议处理，因此无法从这类引用恢复目标函数地址并生成 `func_sig`。

## 问题定义

当前问题不在单个脚本，而在公共链路存在协议断点：

1. prompt 已经示例化 `found_funcptr`
2. `parse_llm_decompile_response` 没有解析 `found_funcptr`
3. `_empty_llm_decompile_result` 没有提供 `found_funcptr`
4. `preprocess_common_skill` 在函数目标的 LLM fallback 中只消费 `found_call` 和 `found_vcall`

结果是：

- LLM 即使正确输出 `found_funcptr`，结果也会在公共解析层丢失
- 回调函数场景无法复用现有 `_preprocess_direct_func_sig_via_mcp` 流程
- 相关 skill 只能依赖 `found_call` 命中，无法覆盖 `lea reg, sub_xxx` / `mov reg, offset sub_xxx` 这类常见反编译模式

## 目标

- 将 `found_funcptr` 正式纳入 `llm_decompile` 公共协议
- 为函数目标提供基于 `insn_va` 的函数指针地址恢复流程
- 在不改变现有 `found_call` / `found_vcall` / `found_gv` / `found_struct_offset` 语义的前提下，补齐函数指针场景
- 让所有复用 `llm_decompile` 的 skill 自动获得该能力，而无需逐脚本定制

## 非目标

- 不修改 `LLM_DECOMPILE` 的配置格式
- 不修改 `call_llm_decompile` 的公开参数
- 不将 `found_call` 与 `found_funcptr` 合并成新的统一 schema
- 不新增从事件字符串自动推导 `funcptr_name` 的兜底规则
- 不改变 fast path 优先于 LLM fallback 的主流程
- 不扩展新的结果类型，例如 patch、enum、switch-case 等

## 方案比较

### 方案 A：局部补丁

只在 `find-CLoopModeGame_OnEventMapCallbacks-client.py` 之类的脚本中增加特殊分支，识别 `found_funcptr` 并自行回填地址。

优点：

- 改动最小
- 对单一场景见效快

缺点：

- 根因仍在公共层
- 其他脚本无法复用
- prompt、解析、消费、测试和文档继续失配

### 方案 B：协议级通用修复

把 `found_funcptr` 正式纳入公共 schema、公共解析、公共消费、测试与文档，函数目标在 fast path 失败后统一尝试 `found_call -> found_funcptr -> found_vcall`。

优点：

- 直接修复根因
- 与现有 `found_call` / `found_gv` 的处理模式一致
- 任何 skill 只要输出 `found_funcptr` 即可自动复用

缺点：

- 改动面比局部补丁略大
- 需要同步测试与文档

### 方案 C：协议重构

把 `found_call` 与 `found_funcptr` 合并为统一的 `found_func`，再用 `kind=call|funcptr` 区分来源。

优点：

- 长期看协议更整齐

缺点：

- 需要同步修改 prompt、解析、消费与已有测试
- 对本次“补齐现有链路”的目标过重

## 选定方案

采用方案 B。

原因如下：

- 问题根因位于公共协议层，不是单个 skill 的 prompt
- 现有 prompt 已经出现 `found_funcptr`，说明上游表达能力已具备
- 下游只需要补齐解析与消费，就能保持兼容地获得新能力
- 该方案不要求重构公开接口，属于最小充分修复

## 详细设计

### 1. 公共协议

`found_funcptr` 作为 `llm_decompile` 的正式顶级字段，和下列字段并列：

- `found_vcall`
- `found_call`
- `found_funcptr`
- `found_gv`
- `found_struct_offset`

字段结构固定为：

```yaml
found_funcptr:
  - insn_va: "0x180666600"
    insn_disasm: "lea     rdx, sub_15BC910"
    funcptr_name: "CLoopModeGame_OnClientPollNetworking"
```

语义定义：

- `insn_va`：装载目标函数地址的那条指令地址
- `insn_disasm`：对应指令的反汇编文本，仅用于调试与核对
- `funcptr_name`：该条函数指针引用所对应的目标函数名，用于 per-symbol 过滤

### 2. 结果解析

在 `parse_llm_decompile_response` 中新增 `found_funcptr` 解析分支，解析逻辑对齐 `found_call`：

- 使用统一的 `_normalize_llm_entries(...)`
- 要求字段完整包含 `insn_va`、`insn_disasm`、`funcptr_name`
- 非法条目按现有容错策略丢弃，不抛异常

同时在 `_empty_llm_decompile_result()` 中补充：

```python
"found_funcptr": []
```

这样所有调用方都可以按固定 schema 读取返回值，而不必区分旧结果和新结果。

### 3. 地址恢复

为 `found_funcptr` 新增公共 resolver：

- 建议函数名：`_resolve_direct_funcptr_target_via_mcp(session, insn_va, debug=False)`

设计原则：

- 只信任 `insn_va`
- 不直接信任 LLM 返回的 `sub_xxx` 文本
- 真实地址恢复仍由 IDA MCP 完成

恢复流程：

1. 将 `insn_va` 解析为整数地址
2. 在 IDA 中以该指令为起点获取数据引用目标
3. 对每个引用目标调用 `ida_funcs.get_func(target_ea)`，归一化到函数起始地址
4. 去重后要求唯一命中 1 个函数地址
5. 唯一命中时返回 `func_va`；否则返回 `None`

唯一性约束与现有 `_resolve_direct_call_target_via_mcp`、`_resolve_direct_gv_target_via_mcp` 保持一致：

- 0 个命中失败
- 多个命中失败
- py_eval 异常失败
- 返回值非法失败

### 4. 函数目标消费顺序

函数目标在 `preprocess_common_skill` 的 LLM fallback 中采用以下顺序：

1. `found_call`
2. `found_funcptr`
3. `found_vcall`
4. slot-only vfunc fallback

选择该顺序的原因：

- `found_call` 最直接，现有链路已稳定
- `found_funcptr` 仍可直接恢复到具体 `func_va`
- `found_vcall` 依赖 vtable class 和 slot，语义比前两者更间接

### 5. per-symbol 过滤

`found_funcptr` 的消费方式与 `found_call` 一致，按当前目标函数名精确过滤：

```python
entry.get("funcptr_name") == func_name
```

过滤后的处理流程：

1. 用 `insn_va` 调用 `_resolve_direct_funcptr_target_via_mcp(...)`
2. 若成功得到唯一 `direct_func_va`
3. 继续复用现有 `_preprocess_direct_func_sig_via_mcp(...)`
4. 由它统一生成 `func_name`、`func_va`、`func_rva`、`func_size`、`func_sig`

这意味着：

- `found_funcptr` 只是新增一种“恢复 `direct_func_va` 的方式”
- 不新增独立 YAML 生成分支
- 不改变已有函数 YAML 的字段顺序和生成逻辑

### 6. 失败语义

`found_funcptr` 的任何失败都只表示该 fallback 条目失败，不改变整体控制流：

- resolver 返回 `None`：跳过当前条目
- `_preprocess_direct_func_sig_via_mcp(...)` 返回 `None`：继续尝试后续条目或后续 fallback
- 所有相关条目都失败：沿用现有 `failed to locate` 语义返回 `False`

这保证了新逻辑只增加能力，不改变原有失败模型。

### 7. 测试设计

测试分为三类。

#### 7.1 解析测试

补充 `parse_llm_decompile_response` 的用例：

- 能正确解析 `found_funcptr`
- 缺字段或非法字段时不会抛异常
- 空输入时返回标准空结构

#### 7.2 消费测试

补充 `preprocess_common_skill` 的函数 fallback 用例：

- fast path 失败
- `call_llm_decompile` 只返回 `found_funcptr`
- `_resolve_direct_funcptr_target_via_mcp` 成功返回唯一函数地址
- `_preprocess_direct_func_sig_via_mcp` 被调用并成功写出函数 YAML

#### 7.3 优先级与失败语义测试

补充以下行为验证：

- 同时存在 `found_call` 与 `found_funcptr` 时优先使用 `found_call`
- `funcptr_name` 与当前 `func_name` 不匹配时跳过
- `found_funcptr` 恢复出 0 个目标时不误生成 YAML
- `found_funcptr` 恢复出多个目标时不误生成 YAML

### 8. 文档同步

文档只做最小必要同步：

- 更新 `docs/call_llm_decompile_prompt.md`
- 保持 `ida_preprocessor_scripts/prompt/call_llm_decompile.md` 与公共解析语义一致
- 如已有 llm_decompile 设计文档明确写死旧 schema，则补充 `found_funcptr` 的公共消费说明

## 验收标准

- `lea reg, sub_xxx`、`mov reg, offset sub_xxx` 这类函数指针引用可以通过 `found_funcptr` 生成目标函数 YAML
- `found_call`、`found_vcall`、`found_gv`、`found_struct_offset` 的现有行为保持不变
- 未使用 `found_funcptr` 的旧 skill 无需改配置即可继续工作
- LLM 即使只返回 `found_funcptr`，函数目标仍可完成 `func_sig` 生成
- 解析器、空结果结构、消费链路、测试与文档保持一致

## 风险与权衡

- 风险一：某些指令的数据引用可能不唯一，导致 resolver 返回多个目标
  - 处理方式：严格要求唯一命中，不做猜测性选择
- 风险二：不同平台下函数指针装载指令形式不同
  - 处理方式：协议只要求给出 `insn_va`，实际恢复由 IDA 数据引用与函数归属决定，尽量避免依赖反汇编文本模式
- 风险三：LLM 误把普通地址装载标成 `found_funcptr`
  - 处理方式：仍以 IDA 恢复结果为准，无法归属到唯一函数即失败

## 验证计划

- 增加定向单元测试覆盖解析、消费、优先级与失败语义
- 实际实现阶段优先运行与 `llm_decompile` 相关的定向测试
- 若后续需要真实链路验证，再使用对应 skill 或预处理脚本验证 `CLoopModeGame_OnEventMapCallbacks` 场景

## 自审结论

- 无 `TODO`、`TBD` 或未定项
- 设计聚焦公共 `llm_decompile` 协议补全，没有引入无关重构
- 新能力通过公共入口复用，不要求逐脚本定制
- 失败语义、主流程与公开接口保持兼容
