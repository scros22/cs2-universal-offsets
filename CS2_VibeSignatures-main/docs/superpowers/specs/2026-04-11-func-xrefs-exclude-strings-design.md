# func_xrefs `exclude_strings_list` 统一通用能力设计

日期：2026-04-11

## 背景

当前 `ida_analyze_util.py` 的统一 `func_xrefs` 管线只支持以下 5 元组协议：

`(func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list)`

但 `ida_preprocessor_scripts/find-CBaseEntity_SetStateChanged.py` 在 Linux 场景下已经出现新的需求：当 `FUNC_XREFS_LINUX` 通过 `xref_funcs` 收集到多个候选函数时，需要进一步排除那些“并非真实目标函数，而是因为 gcc 把虚调用优化为 `if (vcall_target == target) { inline body } else { indirect call }` 而产生的外围调用函数”。

这类外围函数的共同特征是：

- 会继续引用 `CNetworkTransmitComponent_StateChanged`，因此落入正向候选集
- 会包含仅出现在内联展开路径中的调试字符串，例如
  `CNetworkTransmitComponent::StateChanged(%s) @%s:%d`

因此需要为统一 `func_xrefs` 管线增加新的负向约束：`exclude_strings_list`，用于通过“字符串引用所在函数集合”排除误命中的候选函数。

## 目标

- 为统一 `func_xrefs` 管线增加通用的 `exclude_strings_list` 能力
- 让该能力可被后续任意脚本复用，而不是只服务单个脚本特判
- 在 Linux 下支持 `CBaseEntity_SetStateChanged` 通过 `xref_funcs + vtable_class + exclude_strings_list` 唯一收敛到真实函数
- 保持 `exclude_strings_list` 与现有 `xref_strings_list` 的字符串匹配语义一致，统一采用子串匹配
- 将 `func_xrefs` tuple schema 统一升级为固定 6 元组，避免 5/6 元组双轨维护

## 非目标

- 不引入新的 dict 风格 `func_xrefs` schema
- 不对 `xref_strings` 的既有匹配逻辑做精确匹配改造
- 不在本次设计中加入正则字符串匹配、权重打分或更复杂的候选排序逻辑
- 不为个别脚本增加硬编码特判

## 设计概览

统一后的 `func_xrefs` 协议固定为：

`(func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list, exclude_strings_list)`

其中：

- `xref_strings_list`：正向字符串约束
- `xref_signatures_list`：正向字节签名约束
- `xref_funcs_list`：正向依赖函数 xref 约束
- `exclude_funcs_list`：负向函数地址排除约束
- `exclude_strings_list`：负向字符串引用函数排除约束

执行模型保持为“先正向求交，再统一负向做差，最后要求唯一”。

## 接口与契约

### `preprocess_common_skill`

`preprocess_common_skill(..., func_xrefs=...)` 中的 `func_xrefs` 条目必须全部满足 6 元组格式：

`(func_name, xref_strings, xref_signatures, xref_funcs, exclude_funcs, exclude_strings)`

约束如下：

- 只接受 6 元组，不再兼容旧 5 元组
- `func_name` 必须为非空字符串
- 5 个列表字段都必须是 `list` 或 `tuple`
- 5 个列表字段中的每个元素都必须是非空字符串
- `xref_strings`、`xref_signatures`、`xref_funcs` 三者不能同时为空
- `exclude_funcs` 与 `exclude_strings` 可为空列表

内部标准化后的 `func_xrefs_map` 结构统一包含：

- `xref_strings`
- `xref_signatures`
- `xref_funcs`
- `exclude_funcs`
- `exclude_strings`

### `preprocess_func_xrefs_via_mcp`

接口扩展为接收：

- `exclude_strings`

语义为：

- 对 `exclude_strings` 中每个字符串，按与 `xref_strings` 相同的子串匹配逻辑收集“引用该字符串的所属函数起始地址集合”
- 将这些集合做并集，形成 `excluded_string_func_addrs`
- 在正向候选集求交完成后，从 `common_funcs` 中减去该集合

## 数据流与算法

### 正向候选集

保持现有逻辑不变，候选集来源包括：

1. `vtable_class` 对应 vtable 的全部 entries
2. `xref_strings`
3. `xref_signatures`
4. `xref_funcs`

### 负向过滤集

新增统一负向过滤阶段，来源包括：

1. `exclude_funcs`
2. `exclude_strings`

其中：

- `exclude_funcs`：从当前版本 YAML 读取 `func_va`，形成地址集合
- `exclude_strings`：对每个字符串收集“引用该字符串的所属函数集合”，最后取并集

### 执行顺序

1. 校验输入 schema
2. 收集所有正向候选集
3. 对正向候选集做交集，得到 `common_funcs`
4. 收集 `exclude_funcs` 地址集合
5. 收集 `exclude_strings` 对应的排除函数集合并集
6. 从 `common_funcs` 中减去两类排除集合
7. 对最终结果执行“必须恰好 1 个函数”的唯一性校验
8. 生成 `func_sig` 或基础函数元数据

## Linux `CBaseEntity_SetStateChanged` 场景说明

Linux 下，`CBaseEntity_SetStateChanged` 的虚调用在部分调用点会被 gcc 优化为：

- 若虚函数目标等于 `CBaseEntity_SetStateChanged`，则直接内联展开函数体
- 否则保留间接调用

这会导致多个外围函数同时满足：

- xref 到 `CNetworkTransmitComponent_StateChanged`
- 属于 `CBaseEntity` vtable entry 候选

从而让正向求交得到多个候选函数。

但这些外围函数又会携带只属于内联分支的字符串：

`CNetworkTransmitComponent::StateChanged(%s) @%s:%d`

因此将该字符串加入 `exclude_strings_list` 后，可以把这些外围函数从 `common_funcs` 中排除，只保留真实的 `CBaseEntity_SetStateChanged`。

## 错误处理

### 硬失败

以下情况直接失败并返回 `None` 或 `False`：

- `func_xrefs` 条目不是 6 元组
- 必填字段类型非法
- `xref_strings`、`xref_signatures`、`xref_funcs` 同时为空
- `xref_funcs` 或 `exclude_funcs` 依赖的 YAML 缺失或无效
- `vtable_class` 对应 vtable YAML 缺失或无效
- 最终唯一性校验失败

### 非硬失败

`exclude_strings` 中某个字符串如果没有收集到任何引用函数，不应直接失败。

理由：

- `exclude_strings` 的角色是负向收敛器，而不是定位目标所必需的正向约束
- 某个排除字符串在新版本中消失，应该表现为“没有额外排除效果”，而不是让整个预处理立即失败

因此该情况应被视为“空排除集”，流程继续执行。

## 调试输出

为便于排查候选收敛过程，建议在 `debug=True` 时增加以下输出：

- 当前 `exclude_strings` 配置内容
- 每个 `exclude_string` 收集到的函数数量
- 合并后的 `excluded_string_func_addrs`
- 负向过滤前的 `common_funcs`
- 负向过滤后的 `common_funcs`
- 若唯一化成功，打印是由哪类排除条件帮助完成收敛

## 迁移方案

本次为全仓 schema 升级，不保留旧协议兼容层。

迁移动作包括：

1. 更新 `preprocess_common_skill` 中 `func_xrefs` 的校验与文档注释
2. 更新 `preprocess_func_xrefs_via_mcp` 的函数签名与内部实现
3. 更新 `_try_preprocess_func_without_llm` 等调用链，将 `exclude_strings` 传递到底层
4. 全量扫描 `ida_preprocessor_scripts/` 中所有 `FUNC_XREFS`、`FUNC_XREFS_WINDOWS`、`FUNC_XREFS_LINUX`
5. 将所有现有 5 元组统一补齐为 6 元组，默认第 6 项为 `[]`
6. 更新相关 spec / memory 中对旧 5 元组协议的描述

## 测试与验证

本次不默认运行测试或构建，但实现后至少需要做定向验证：

1. 语义验证
   - 确认所有 `func_xrefs` 定义均为 6 元组
   - 确认无脚本遗漏迁移

2. 行为验证
   - 以 `find-CBaseEntity_SetStateChanged.py` 的 Linux 路径为主验证样例
   - 确认 `exclude_strings_list` 生效后，原本 5 个候选能收敛为唯一目标

3. 回归验证
   - 至少抽查若干不使用 `exclude_strings_list` 的既有 `func_xrefs` 脚本
   - 确认它们在补空列表后仍能通过公共校验逻辑

## 风险与权衡

- 风险一：全仓强制升级后，任何遗漏的 5 元组都会立即报错
  - 缓解：实现时用搜索统一迁移，并做一次静态扫描确认

- 风险二：某些排除字符串过于宽泛，可能误排除真实目标
  - 缓解：沿用当前子串匹配语义，但要求脚本编写者优先使用足够稳定且低歧义的字符串

- 风险三：`exclude_strings` 在新版本中缺失，可能导致候选无法唯一化
  - 缓解：这属于数据变化导致的正常定位失败，应保留“唯一性失败”的显式结果，而不是静默猜测

## 实施边界

本设计只覆盖统一 `func_xrefs` 管线与脚本 schema 升级，不包含：

- 新的字符串匹配模式配置
- 候选函数排序或打分机制
- 与 LLM decompile fallback 的额外联动改造

## 决策摘要

- `exclude_strings_list` 作为统一 `func_xrefs` 管线的通用能力引入
- 匹配语义与 `xref_strings_list` 保持一致，采用子串匹配
- 作为负向过滤条件，在正向求交之后统一做差集
- `func_xrefs` schema 全仓统一升级为固定 6 元组
- 不保留旧 5 元组兼容层
