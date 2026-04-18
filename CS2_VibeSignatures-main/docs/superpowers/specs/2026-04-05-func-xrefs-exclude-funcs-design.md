# 为 `func_xrefs` 增加 `exclude_funcs_list` 过滤能力设计

## 背景

当前 `ida_analyze_util.py` 中的 `preprocess_common_skill()` 已支持统一的 `func_xrefs` 入口，用于在 `func_sig` 失效时，通过字符串引用与函数引用联合求交来定位目标函数。

这一能力已经覆盖了大部分“多约束交集唯一定位”的场景，但仍有一个缺口：

- 某些函数会与目标函数共享同一组字符串引用特征。
- 某些函数也会共享同一组函数引用特征。
- 仅靠正向约束求交后，候选结果可能仍有 2 个或更多。
- 这些多余候选中，往往包含一个已知且稳定的“应排除函数”。

以 `CNetworkGameClient_RecordEntityBandwidth` 为例，目标函数可能与另一函数同时满足：

- 引用 `"Local Player"`
- 引用 `"Other Players"`

如果其中一个候选已知为 `CNetworkServerService_Init`，则希望在交集完成后显式将其排除，留下唯一正确结果 `CNetworkGameClient_RecordEntityBandwidth`。

## 目标

- 为 `func_xrefs` 增加 `exclude_funcs_list` 能力。
- 将 `func_xrefs` 契约统一升级为强制四元组：
  ```python
  (func_name, xref_strings_list, xref_funcs_list, exclude_funcs_list)
  ```
- 在 `preprocess_func_xrefs_via_mcp()` 中支持“正向约束求交后，再按排除函数地址做差集”。
- 所有 `exclude_funcs_list` 依赖函数地址都只从当前版本 YAML 读取。
- 批量迁移现有所有 `FUNC_XREFS` 调用点到四元组格式。
- 新增 `CNetworkGameClient_RecordEntityBandwidth` 的 skill 与 `config.yaml` 接入。

## 非目标

- 不保留旧三元组 `func_xrefs` 的兼容层。
- 不引入 `exclude_strings_list`、正则匹配或模糊匹配。
- 不改变 `preprocess_func_sig_via_mcp()` 作为常规函数首选快路径的既有策略。
- 不修改 `config.yaml` 的结构定义；`exclude_funcs_list` 仅存在于预处理脚本中。

## 用户接口

### `func_xrefs` 新契约

`preprocess_common_skill()` 中的 `func_xrefs` 改为固定四元组列表：

```python
FUNC_XREFS = [
    (
        "TargetFunction",
        ["string a", "string b"],
        ["DependencyFunctionA"],
        ["FunctionToExcludeA", "FunctionToExcludeB"],
    ),
]
```

字段含义如下：

- `func_name`
  - 目标函数名，也是输出 YAML 文件名 stem 与 IDA 中的重命名目标名。
- `xref_strings_list`
  - 目标函数必须引用的字符串列表，可为空。
- `xref_funcs_list`
  - 目标函数必须引用的已知函数列表，可为空。
- `exclude_funcs_list`
  - 在所有正向约束完成求交后，需要从候选集合中剔除的函数列表，可为空。

约束规则：

- `func_name` 必须为非空字符串。
- 三个列表都必须为 `list` 或 `tuple`，且成员必须为非空字符串。
- `xref_strings_list` 与 `xref_funcs_list` 不能同时为空。
- `exclude_funcs_list` 可以为空。
- 旧三元组格式一律视为非法配置。

### 调用示例

目标场景示例：

```python
FUNC_XREFS = [
    (
        "CNetworkGameClient_RecordEntityBandwidth",
        [
            "Local Player",
            "Other Players",
        ],
        [],
        [
            "CNetworkServerService_Init",
        ],
    ),
]
```

语义为：

1. 先找出同时引用 `"Local Player"` 与 `"Other Players"` 的所有函数。
2. 再将 `CNetworkServerService_Init` 对应地址从候选中排除。
3. 只允许剩余候选恰好为 1 个函数。

## 总体方案

采用“强制升级四元组 + 内部规范化 + 交集后差集过滤”的最短路径方案。

### 入口层：`preprocess_common_skill()`

`preprocess_common_skill()` 的 `func_xrefs` 入口统一做四类工作：

1. 校验每个条目必须是长度为 4 的 tuple/list。
2. 校验 `func_name`、`xref_strings_list`、`xref_funcs_list`、`exclude_funcs_list` 的类型和值。
3. 将四元组规范化为内部字典：
   - `xref_strings`
   - `xref_funcs`
   - `exclude_funcs`
4. 在 fallback 调用 `preprocess_func_xrefs_via_mcp()` 时透传三类列表。

### 底层实现：`preprocess_func_xrefs_via_mcp()`

函数签名新增参数：

```python
async def preprocess_func_xrefs_via_mcp(
    session,
    func_name,
    xref_strings,
    xref_funcs,
    exclude_funcs,
    new_binary_dir,
    platform,
    image_base,
    debug=False,
):
```

内部流程保持现有结构，只新增一段“交集后排除”逻辑：

1. 收集所有字符串候选集合。
2. 收集所有 `xref_funcs` 候选集合。
3. 对全部候选集合求交，得到 `common_funcs`。
4. 解析 `exclude_funcs` 对应的函数地址集合。
5. 执行差集：
   ```python
   common_funcs = common_funcs - excluded_func_addrs
   ```
6. 再检查 `common_funcs` 是否恰好只剩 1 个函数。
7. 若唯一，则继续生成 `func_sig` 或 basic info。

## 当前版本 YAML 地址解析

`xref_funcs_list` 与 `exclude_funcs_list` 的地址解析规则完全一致：

```python
os.path.join(new_binary_dir, f"{func_name}.{platform}.yaml")
```

读取约束如下：

- 只接受字典型 YAML 顶层。
- 只读取 `func_va` 字段。
- `func_va` 必须能解析为合法整数地址。
- 不读取 `old_yaml_map`。
- 不依赖 IDA 当前名字。

这意味着：

- `xref_funcs_list` 与 `exclude_funcs_list` 中提到的函数，必须已经在当前版本输出目录中存在对应 YAML。
- skill 执行顺序必须由 `config.yaml` 的 `expected_input` 或技能排序保证。

## 算法细节

### 正向候选收集

正向候选收集逻辑不变：

- 对每个 `xref_string` 调用现有字符串 xref 收集逻辑，得到函数起始地址集合。
- 对每个 `xref_func`，先从当前版本 YAML 取到 `func_va`，再调用现有地址 xref 收集逻辑，得到函数起始地址集合。
- 任一候选集合为空，直接失败。

### 交集后排除

新增规则：

- `exclude_funcs_list` 不参与正向求交。
- 它只在 `common_funcs` 生成后参与过滤。
- 每个 `exclude_func` 先解析到当前版本 YAML 的 `func_va`，再加入排除地址集合。
- 执行 `common_funcs - excluded_func_addrs` 后，再判断唯一性。

这样可以保持语义清晰：

- `xref_strings_list` / `xref_funcs_list` 定义“必须满足什么”
- `exclude_funcs_list` 定义“即便满足上述约束，也不能是哪些函数”

### 失败语义

以下情况直接失败：

- `func_xrefs` 条目不是四元组。
- `xref_strings_list` 与 `xref_funcs_list` 同时为空。
- `xref_funcs_list` 中任一依赖 YAML 缺失、不可读或 `func_va` 非法。
- `exclude_funcs_list` 中任一依赖 YAML 缺失、不可读或 `func_va` 非法。
- 差集前或差集后，最终候选不是恰好 1 个。

以下情况不单独报错，视为正常：

- `exclude_funcs_list` 中某个函数地址不在 `common_funcs` 里；差集结果不变即可。

## 迁移策略

### 核心逻辑

修改 `ida_analyze_util.py` 中两个入口：

- `ida_analyze_util.py:2362`
- `ida_analyze_util.py:2525`

需要同步更新：

- `preprocess_func_xrefs_via_mcp()` 参数与 docstring
- `preprocess_common_skill()` docstring
- `func_xrefs` 条目校验逻辑
- `func_xrefs_map` 内部结构
- fallback 调用透传逻辑

### 预处理脚本

仓库内所有使用 `FUNC_XREFS`、`FUNC_XREFS_WINDOWS`、`FUNC_XREFS_LINUX` 的脚本统一迁移为四元组：

- 旧格式：
  ```python
  (func_name, xref_strings_list, xref_funcs_list)
  ```
- 新格式：
  ```python
  (func_name, xref_strings_list, xref_funcs_list, [])
  ```

同时统一注释文案为：

```python
# (func_name, xref_strings_list, xref_funcs_list, exclude_funcs_list)
```

### 新增目标 skill

新增 `find-CNetworkGameClient_RecordEntityBandwidth.py`，其中：

- `TARGET_FUNCTION_NAMES = ["CNetworkGameClient_RecordEntityBandwidth"]`
- `FUNC_XREFS` 使用四元组
- `exclude_funcs_list = ["CNetworkServerService_Init"]`
- 调用 `preprocess_common_skill()` 走常规函数流水线

## `config.yaml` 接入

`config.yaml` 需要增加两类声明：

### skill 声明

为 `CNetworkGameClient_RecordEntityBandwidth` 增加对应 skill：

- `expected_output` 至少包含
  - `CNetworkGameClient_RecordEntityBandwidth.{platform}.yaml`

如果 `exclude_funcs_list` 中的 `CNetworkServerService_Init` 依赖当前版本 YAML，则该 skill 需要补充：

- `expected_input`
  - `CNetworkServerService_Init.{platform}.yaml`

这样可显式保证执行顺序，避免运行时因排除依赖尚未生成而失败。

### symbol 声明

把 `CNetworkGameClient_RecordEntityBandwidth` 加入模块 `symbols`，类别为常规 `func`。

## 验证策略

本次只定义最小充分验证，不默认执行全量分析或全量测试。

建议验证分为 4 项：

1. 检查仓库内所有 `FUNC_XREFS*` 是否都已升级为四元组，避免旧格式残留。
2. 检查 `preprocess_common_skill()` docstring、参数校验与实际调用是否一致。
3. 检查新脚本与 `config.yaml` 的 `expected_output`、`expected_input`、`symbols` 是否闭合。
4. 若后续允许执行命令验证，再优先做目标 skill 的定向运行，而不是全仓全量分析。

## 风险与权衡

- 强制四元组会带来一次性迁移成本，但能避免长期双格式维护。
- `exclude_funcs_list` 依赖当前版本 YAML，因此 skill 排序错误会导致运行失败；这是有意保留的严格约束。
- “交集后差集”只解决“候选中过滤已知误命中”的问题，不解决“排除后仍有多个候选”的场景；这种情况仍应严格失败，而不是引入启发式选择。

## 结论

本次设计选择最小行为扩展：

- 对外统一强制四元组 `func_xrefs`
- 对内复用现有 YAML 地址解析与 xref 收集逻辑
- 仅在 `common_funcs` 求交完成后增加一次差集过滤

这样既能满足 `CNetworkGameClient_RecordEntityBandwidth` 这类“正向约束仍不唯一”的场景，也不会扩大 `config.yaml` 或主流程协议的复杂度。
