# llm_decompile 同组请求合并设计

## 背景

`ida_preprocessor_scripts/find-INetworkMessages_GetFieldChangeCallbackOrderCount-AND-INetworkMessages_GetFieldChangeCallbackPriorities.py` 中的 `LLM_DECOMPILE` 包含两个目标：

- `INetworkMessages_GetFieldChangeCallbackOrderCount`
- `INetworkMessages_GetFieldChangeCallbackPriorities`

这两个目标使用相同的 `path_to_prompt` 与 `path_to_reference`。当前 `ida_analyze_util.py` 的 `preprocess_common_skill` 会在逐个 `func_name` 处理失败后分别调用 `call_llm_decompile`，导致两个目标发起两次独立 LLM 请求。

`call_llm_decompile` 本身已经接受 `symbol_name_list`，因此问题不在 LLM 请求函数，而在调用侧没有对同组 unresolved 目标做批量合并。

## 目标

- 当多个 `LLM_DECOMPILE` 条目拥有相同 `path_to_prompt` 与 `path_to_reference`，并且这些目标都仍需要 LLM fallback 时，只发起一次 LLM 请求。
- 单次请求中的 `symbol_name_list` 包含该组仍待追踪的所有目标名称。
- 保持后续结果处理语义不变：仍按 `entry["func_name"] == func_name` 为每个目标提取自己的 `found_call` 或 `found_vcall`。
- 不改变已成功走 `func_sig` 或 `func_xrefs` fast path 的目标行为。

## 非目标

- 不修改 prompt 模板 schema。
- 不修改 `parse_llm_decompile_response` 的 YAML 解析结构。
- 不改变 `call_llm_decompile` 的公开参数与返回值。
- 不合并不同 prompt、不同 reference 或不同 platform 下的请求。
- 不把 LLM fallback 提前到 fast path 之前执行。

## 方案比较

### 方案 A：在 `preprocess_common_skill` 内按需合并同组 LLM fallback

当某个 `func_name` 进入 LLM fallback 时，查找后续仍未处理、同样配置了 `LLM_DECOMPILE`、且 `path_to_prompt` 与 `path_to_reference` 相同的目标，一次调用 `call_llm_decompile`。

优点：

- 改动集中在现有 LLM fallback 调用侧。
- 保留当前主循环结构与 fast path 顺序。
- 可以复用现有 `call_llm_decompile`、解析和 per-symbol 过滤逻辑。

缺点：

- 需要在主循环中维护一份 LLM 结果缓存，避免同组后续目标再次发起请求。

### 方案 B：先跑完所有 fast path，再统一批量处理所有 LLM fallback

先遍历所有函数并完成 `func_sig`、`func_xrefs` 尝试，再把剩余 unresolved 目标按 prompt/reference 分组统一调用 LLM。

优点：

- 批处理模型清晰。
- 对所有 LLM fallback 目标的整体视图更完整。

缺点：

- 会重排 `preprocess_common_skill` 当前逐目标写 YAML 的主流程。
- 改动面更大，回归风险更高。

### 方案 C：只做请求结果 cache

保留当前逐目标调用结构，仅缓存同一 prompt/reference 的结果。

优点：

- 表面改动最小。

缺点：

- 首次请求仍只能包含当前单个 `func_name`。
- 无法满足“同一个请求里在 `symbol_name_list` 里带上两个需要追踪的条目”的核心目标。

## 选定方案

采用方案 A。

该方案是最小充分改动：不改变 `call_llm_decompile`，不重排整个 `preprocess_common_skill`，只在调用侧把“同 prompt/reference 的 unresolved LLM fallback 目标”合并为一次请求。

## 详细设计

### 1. 分组键

LLM fallback 合并键使用原始规格中的两项：

- `prompt_path`
- `reference_yaml_path`

这两项来自 `_build_llm_decompile_specs_map` 的结果。由于 `_prepare_llm_decompile_request` 内部会按 `platform` 渲染路径，因此同一次 `preprocess_common_skill` 调用内的 platform 一致，不需要额外把 platform 放入分组键。

### 2. unresolved 目标范围

只合并当前仍需要 LLM fallback 的目标：

1. 目标位于 `all_func_names`。
2. 目标存在于 `llm_decompile_specs_map`。
3. 目标尚未成功通过 `preprocess_func_sig_via_mcp` 定位。
4. 如果目标配置了 `func_xrefs`，也尚未成功通过 `preprocess_func_xrefs_via_mcp` 定位。
5. 目标尚未拥有缓存的同组 LLM 结果。

已成功生成 `func_data` 的目标不进入本次 `symbol_name_list`。

### 3. LLM 请求结果缓存

在 `preprocess_common_skill` 的函数处理阶段维护一个局部缓存，例如：

- key：`(prompt_path, reference_yaml_path)`
- value：一次 `call_llm_decompile` 的 parsed result

当某个目标进入 LLM fallback：

1. 根据当前目标找到分组键。
2. 若缓存已有结果，直接复用。
3. 若缓存没有结果，收集同组仍需 LLM fallback 的目标名称。
4. 调用一次 `call_llm_decompile(symbol_name_list=group_func_names, ...)`。
5. 将 parsed result 写入缓存。
6. 当前目标继续按现有逻辑过滤并生成 `func_data`。

后续同组目标进入 LLM fallback 时直接复用缓存，不再发起 LLM 请求。

### 4. 目标函数上下文

同组请求沿用 `_prepare_llm_decompile_request` 中 reference YAML 指向的参考函数，并通过 `_load_llm_decompile_target_detail_via_mcp` 加载同一个目标函数的反汇编与伪代码。

对于当前触发场景，两个 symbol 都是在同一个目标函数中追踪引用，因此共享同一份：

- `disasm_code`
- `procedure`
- `disasm_for_reference`
- `procedure_for_reference`

若将来出现同 prompt/reference 但实际 target reference function 不同的配置，应通过不同 `reference_yaml_path` 表达，不在本次设计中额外支持。

### 5. 结果消费

LLM 返回值仍使用现有结构：

- `found_call`
- `found_vcall`
- `found_gv`
- `found_struct_offset`

本次目标是函数或虚函数定位，因此继续使用现有 per-symbol 过滤：

- `entry.get("func_name") != func_name` 时跳过
- `found_call` 走 `_resolve_direct_call_target_via_mcp`
- `found_vcall` 走 vtable relation 与 slot-only fallback

这样一次 LLM 响应可以包含多个目标，后续每个目标仍只消费自己的条目。

### 6. 错误处理

- `_prepare_llm_decompile_request` 返回 `None` 时，当前分组缓存为空结果，避免同组目标重复准备失败。
- `_load_llm_decompile_target_detail_via_mcp` 返回 `None` 时，当前分组缓存为空结果。
- `call_llm_decompile` 抛异常时，当前分组缓存为空结果。
- 空结果不直接中断流程；目标后续无法生成 `func_data` 时，沿用现有 `failed to locate` 路径返回 `False`。

### 7. 调试输出

保留现有 `call_llm_decompile` debug 输出。由于 `symbol_name_list` 会包含多个目标，debug 日志中应能看到逗号分隔的 symbol 列表。

可选增加一条调用侧 debug 输出，说明同组 LLM fallback 合并了哪些目标，但不是必须条件。

## 验收标准

- 对 `INetworkMessages_GetFieldChangeCallbackOrderCount` 与 `INetworkMessages_GetFieldChangeCallbackPriorities` 这种同 prompt/reference 的 unresolved 目标，只调用一次 `call_llm_decompile`。
- 该次调用的 `symbol_name_list` 同时包含两个目标名称。
- 两个目标仍分别生成自己的 YAML，并按 `func_name` 过滤 LLM 返回条目。
- 如果其中一个目标已被 fast path 成功定位，则 LLM 请求只包含另一个 unresolved 目标。
- 不影响没有配置 `LLM_DECOMPILE` 的函数目标。
- 不影响 prompt/reference 不同的多个 LLM fallback 目标，它们仍分别请求。

## 验证计划

- 增加或使用定向 mock 验证：mock `call_llm_decompile`，构造两个同 prompt/reference 的 LLM_DECOMPILE 目标，断言只调用一次且 `symbol_name_list` 包含两个名称。
- 验证 fast path 成功的目标不会被加入 LLM fallback 分组。
- 验证同 prompt/reference 组的后续目标复用缓存结果，不再次调用 LLM。
- 如需真实环境验证，再运行对应预处理脚本或 `ida_analyze_bin.py` 的目标模块流程。

## 自审结论

- 无未决占位符。
- 设计范围聚焦在 `ida_analyze_util.py` 的 LLM fallback 调用侧。
- 方案不改变公开接口和 prompt schema。
- 错误处理沿用现有“LLM 空结果后定位失败返回 False”的语义。
