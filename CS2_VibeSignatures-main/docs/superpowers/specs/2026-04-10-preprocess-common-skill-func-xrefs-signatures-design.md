# `preprocess_common_skill` 的 `FUNC_XREFS` `xref_signatures_list` 与 `LoggingChannel_Init` 设计

## 背景

当前 `preprocess_common_skill` 的 `func_xrefs` 只支持 4 元组：

```python
(func_name, xref_strings_list, xref_funcs_list, exclude_funcs_list)
```

这意味着函数定位时只能依赖：

- 字符串交叉引用
- 已有函数 YAML 地址的交叉引用
- 排除函数集合
- 可选 vtable entry 约束

对于 `LoggingChannel_Init` 这类 regular non-virtual function，仅靠字符串 `"Networking"` 约束不够稳定，还需要额外要求：

- Windows 上命中指令特征 `C7 44 24 40 64 FF FF FF`
- Linux 上命中指令特征 `41 B8 64 FF FF FF`

并且这类“由指令签名命中后回溯到所属函数”的能力，不应只为 `LoggingChannel_Init` 特判，而应成为 `FUNC_XREFS` 的通用候选集来源。

用户进一步要求：仓库中所有包含 `FUNC_XREFS` 的已有预处理脚本，都要统一升级到新格式。

## 目标

- 将 `func_xrefs` 条目格式从 4 元组升级为 5 元组：

```python
(func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list)
```

- 将 `xref_signatures_list` 设计为与 `xref_strings_list`、`xref_funcs_list` 同级的通用候选集来源
- 对 `LoggingChannel_Init` 提供 Windows/Linux 双平台预处理脚本
- 在 `config.yaml` 中注册 `find-LoggingChannel_Init`
- 升级所有已有包含 `FUNC_XREFS` 的预处理脚本到 5 元组格式
- 为公共层和新脚本补齐定向测试

## 非目标

- 本次不引入 `func_xrefs` 的 dict 化重构
- 本次不调整 `func_vtable_relations` 语义
- 本次不改动与 `FUNC_XREFS` 无关的预处理脚本
- 本次不新增与当前需求无关的 MCP 能力

## 方案比较

### 方案 1：将 `FUNC_XREFS` 扩展为 5 元组

示例：

```python
(
    "LoggingChannel_Init",
    ["Networking"],
    ["C7 44 24 40 64 FF FF FF"],
    [],
    [],
)
```

优点：

- 与现有脚本风格保持一致
- 与用户给出的目标写法完全一致
- 改动路径最短，适合对现有几十个脚本做机械升级

缺点：

- 位置参数继续增加，可读性一般

### 方案 2：把 `FUNC_XREFS` 改成 dict

优点：

- 可扩展性最好
- 字段语义更显式

缺点：

- 改动面过大
- 需要全量修改现有脚本读取模型，不符合本次最短路径

### 方案 3：单独新增 `func_signature_xrefs`

优点：

- 不需要改动旧 `FUNC_XREFS` 结构

缺点：

- 候选集逻辑被拆成两套接口
- 调用层语义更分散，不利于长期维护

## 选定方案

采用方案 1：将 `FUNC_XREFS` 统一升级为 5 元组。

这是满足当前需求的最小充分方案，既能为 `LoggingChannel_Init` 提供稳定定位能力，也能保持 `FUNC_XREFS` 的统一候选集模型。

## 详细设计

### 1. 公共接口变更

`preprocess_common_skill(...)` 中 `func_xrefs` 的条目格式升级为：

```python
(func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list)
```

`preprocess_func_xrefs_via_mcp(...)` 新增参数：

```python
xref_signatures
```

### 2. 语义定义

- `xref_strings_list`
  - 通过字符串交叉引用生成候选函数集合
- `xref_signatures_list`
  - 通过字节签名定位指令命中地址，再将命中地址映射到所属函数起始地址，生成候选函数集合
- `xref_funcs_list`
  - 通过依赖函数的 `func_va` 交叉引用生成候选函数集合
- `exclude_funcs_list`
  - 在正向候选集求交后，从结果中移除这些函数地址

统一流程为：

1. 每种来源独立生成候选集
2. 对所有正向候选集做求交
3. 应用 `exclude_funcs_list` 差集过滤
4. 结果必须唯一

### 3. 参数校验

`preprocess_common_skill(...)` 在解析 `func_xrefs` 时新增以下约束：

- 每个条目必须是 5 元组
- `func_name` 必须是非空字符串
- `xref_strings_list`、`xref_signatures_list`、`xref_funcs_list`、`exclude_funcs_list` 都必须是列表或元组
- 四个列表中的每个元素都必须是非空字符串
- `xref_strings_list`、`xref_signatures_list`、`xref_funcs_list` 三者不能同时为空
- 同一 `func_name` 不允许重复定义

本次不保留 4 元组兼容分支，直接执行全量切换。

### 4. `xref_signatures_list` 候选集生成逻辑

对 `xref_signatures_list` 中的每个签名：

1. 通过 `find_bytes` 搜索签名
2. 将每个命中地址视为“特征指令地址”
3. 找到包含该地址的函数，并取其函数起始地址
4. 汇总为该签名的候选函数集合

约束：

- 某个签名若没有命中任何有效函数，则该函数的 xref 预处理失败
- 某个签名若命中多个函数，不直接失败；继续参与与其他候选集的求交
- 若最终求交后仍不是唯一函数，则失败

### 5. `LoggingChannel_Init` 的表达方式

`LoggingChannel_Init` 必须满足：

- 签名命中的指令属于 `LoggingChannel_Init`
- `LoggingChannel_Init` 同时引用字符串 `"Networking"`

这两个条件通过统一求交自然表达，无需在公共层写特判。

Windows 脚本的 `FUNC_XREFS`：

```python
(
    "LoggingChannel_Init",
    ["Networking"],
    ["C7 44 24 40 64 FF FF FF"],
    [],
    [],
)
```

Linux 脚本的 `FUNC_XREFS`：

```python
(
    "LoggingChannel_Init",
    ["Networking"],
    ["41 B8 64 FF FF FF"],
    [],
    [],
)
```

### 6. 脚本组织方式

新增两个分平台预处理脚本：

- `ida_preprocessor_scripts/find-LoggingChannel_Init-windows.py`
- `ida_preprocessor_scripts/find-LoggingChannel_Init-linux.py`

两个脚本都直接复用：

```python
from ida_analyze_util import preprocess_common_skill
```

并保持 regular function 脚本的标准结构：

- `TARGET_FUNCTION_NAMES`
- `FUNC_XREFS`
- `GENERATE_YAML_DESIRED_FIELDS`
- `preprocess_skill(...)`

`LoggingChannel_Init` 是 regular non-virtual function，因此：

- 不配置 `FUNC_VTABLE_RELATIONS`
- 不输出 `vtable_name` / `vfunc_offset` / `vfunc_index`
- symbol `category` 设为 `func`

### 7. `config.yaml` 变更

在 `networksystem` 模块的 `skills` 段新增：

- `find-LoggingChannel_Init`
- `expected_output: LoggingChannel_Init.{platform}.yaml`

在 `symbols` 段新增：

- `LoggingChannel_Init`
- `category: func`

`LoggingChannel_Init` 不依赖其他 YAML，可不配置 `expected_input`。

### 8. 全量脚本迁移策略

仓库中所有已有包含 `FUNC_XREFS` 的预处理脚本，都统一从 4 元组升级到 5 元组。

迁移规则：

- 原格式：

```python
(func_name, xref_strings_list, xref_funcs_list, exclude_funcs_list)
```

- 新格式：

```python
(func_name, xref_strings_list, [], xref_funcs_list, exclude_funcs_list)
```

这保证：

- 旧语义不变
- 新能力统一落在公共层
- 后续新增基于签名的 regular function 时，不再需要再次改接口

### 9. 错误处理

失败条件包括但不限于：

- `func_xrefs` 条目格式非法
- `xref_signatures_list` 含非法值
- 三类正向约束全空
- 某个签名没有命中任何有效函数
- 任一候选集为空
- 求交结果不唯一
- 目标函数基础信息或 `func_sig` 生成失败

调试输出应沿用现有 `debug=True` 风格，明确指出：

- 是哪个 `func_name`
- 哪个签名或哪个候选来源失败
- 最终交集数量

## 测试设计

### 1. `tests/test_ida_analyze_util.py`

新增或扩展以下覆盖：

- `preprocess_func_xrefs_via_mcp` 支持 `xref_signatures`
- 字符串候选集与签名候选集求交成功
- 签名无命中时失败
- 签名命中多个函数，但与字符串求交后唯一时成功
- `preprocess_common_skill` 对 5 元组格式的校验与透传
- 三类正向约束全空时失败

### 2. `tests/test_ida_preprocessor_scripts.py`

新增：

- `find-LoggingChannel_Init-windows.py` 的脚本透传测试
- `find-LoggingChannel_Init-linux.py` 的脚本透传测试

并根据当前测试风格，对至少一个已存在 `FUNC_XREFS` 脚本的断言更新为 5 元组格式，确保批量迁移后的调用契约被测试覆盖。

### 3. 验收层级

本次采用 Level 0 到 Level 1 的定向验证：

- 定向单测验证公共能力
- 定向单测验证新脚本和批量脚本升级后的参数结构

除非用户额外要求，本次不主动扩大全量测试或构建范围。

## 验收标准

- `preprocess_common_skill` 能正确解析 5 元组 `func_xrefs`
- `preprocess_func_xrefs_via_mcp` 能处理 `xref_signatures`
- 所有包含 `FUNC_XREFS` 的已有预处理脚本均完成格式升级
- `find-LoggingChannel_Init-windows.py` 与 `find-LoggingChannel_Init-linux.py` 已新增
- `config.yaml` 完成 skill 与 symbol 注册
- `LoggingChannel_Init` 的预处理同时满足字符串与指令签名双重约束
- 相关定向测试通过

## 风险与权衡

- 最大风险是全量脚本迁移面较大，但改动内容机械且语义简单，适合一次性完成
- 不保留 4 元组兼容分支可以减少公共层长期复杂度，但要求本次迁移必须完整
- `xref_signatures_list` 依赖“命中地址映射到所属函数”的 MCP 辅助逻辑；若底层能力不足，需要在实现阶段优先补一个稳定的公共辅助函数

## 实施范围

预计变更文件包括：

- `ida_analyze_util.py`
- `config.yaml`
- `ida_preprocessor_scripts/find-LoggingChannel_Init-windows.py`
- `ida_preprocessor_scripts/find-LoggingChannel_Init-linux.py`
- 所有包含 `FUNC_XREFS` 的现有预处理脚本
- `tests/test_ida_analyze_util.py`
- `tests/test_ida_preprocessor_scripts.py`
