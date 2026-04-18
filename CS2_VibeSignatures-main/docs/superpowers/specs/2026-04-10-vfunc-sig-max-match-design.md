# `vfunc_sig_max_match` 设计

## 背景

当前仓库中，`GENERATE_YAML_DESIRED_FIELDS` 被设计为每个 symbol 的唯一 YAML 输出契约，`preprocess_common_skill(...)` 会严格按该契约校验字段并生成 YAML。

但现有 `vfunc_sig` 生成与复用逻辑都默认要求签名字节模式必须唯一匹配：

- `preprocess_gen_vfunc_sig_via_mcp(...)` 只有在 `find_bytes` 命中数等于 `1` 时才接受签名
- `preprocess_func_sig_via_mcp(...)` 在旧 YAML 走 `vfunc_sig` fallback 时，也要求旧 `vfunc_sig` 在新二进制里唯一匹配

这对某些模板化实例函数并不合适。以 `INetworkMessages_GetLoggingChannel` 为例，LLM fallback 能稳定找到多个语义等价的 `vcall` 指令，它们共享同一模板形态与同一 `vfunc_offset`，但当前实现会因为命中数大于 `1` 而持续扩展签名，最终失败。

用户要求新增：

```python
"vfunc_sig_max_match:10"
```

其语义是：

- 生成 `vfunc_sig` 时，允许最多 `10` 个匹配，不再强求唯一
- 当匹配数已经降到 `<= 10` 时，停止继续扩展 signature bytes
- 最终 YAML 需要持久化：

```yaml
vfunc_sig_max_match: 10
```

- 后续 `preprocess_func_sig_via_mcp(...)` 复用旧 YAML 中的 `vfunc_sig` 时，也要按该上限放宽匹配条件

## 目标

- 支持在 `GENERATE_YAML_DESIRED_FIELDS` 中声明 `"vfunc_sig_max_match:N"`
- 将该声明规范化为最终 YAML 字段 `vfunc_sig_max_match: N`
- 让 slot-only vfunc 的 `vfunc_sig` 生成逻辑支持“最多 N 个匹配”
- 让旧 YAML 复用 `vfunc_sig` 的 fallback 路径也支持“最多 N 个匹配”
- 保持未声明 `vfunc_sig_max_match` 的现有脚本行为完全不变

## 非目标

- 本次不放宽 `func_sig`、`gv_sig`、`patch_sig`、`offset_sig` 的唯一匹配语义
- 本次不改变 `FUNC_VTABLE_RELATIONS`、`FUNC_XREFS`、`LLM_DECOMPILE` 的结构
- 本次不引入新的 YAML 契约入口，仍以 `GENERATE_YAML_DESIRED_FIELDS` 为唯一声明入口
- 本次不修改与 `vfunc_sig` 无关的 writer 顺序与字段语义

## 方案比较

### 方案 A：把 `vfunc_sig_max_match:N` 作为字段契约 directive，并持久化为 YAML 字段

示例：

```python
GENERATE_YAML_DESIRED_FIELDS = [
    (
        "INetworkMessages_GetLoggingChannel",
        [
            "func_name",
            "vfunc_sig",
            "vfunc_sig_max_match:10",
            "vfunc_offset",
            "vfunc_index",
            "vtable_name",
        ],
    ),
]
```

优点：

- 与用户给出的目标写法完全一致
- 继续保持 `GENERATE_YAML_DESIRED_FIELDS` 作为唯一契约入口
- 既能表达生成控制，又能自然落地为最终 YAML 字段

缺点：

- 公共层需要区分“真实输出字段”和“带参数 directive”

### 方案 B：把 `vfunc_sig_max_match` 作为普通字段单独声明

示例：

```python
["func_name", "vfunc_sig", "vfunc_sig_max_match", ...]
```

优点：

- 字段名更接近普通 YAML key

缺点：

- 无法直接表达数值
- 需要在脚本别处额外放参数，语义分散

### 方案 C：增加独立配置入口

示例：

```python
VFUNC_SIG_MATCH_LIMITS = [
    ("INetworkMessages_GetLoggingChannel", 10),
]
```

优点：

- 类型边界最清晰

缺点：

- 打破 `GENERATE_YAML_DESIRED_FIELDS` 作为唯一契约入口的设计
- 增加维护成本，不符合本次最短路径

## 选定方案

采用方案 A。

`"vfunc_sig_max_match:10"` 保留在 `GENERATE_YAML_DESIRED_FIELDS` 中作为带参数 directive。公共层解析后：

- 参与最终 YAML 输出的真实字段为 `vfunc_sig_max_match`
- 参与生成期与复用期控制的选项值为 `10`

这样既满足用户希望的脚本写法，也让 YAML 持久化后的复用链路具备完整语义。

## 详细设计

### 1. 字段契约模型

`_normalize_generate_yaml_desired_fields(...)` 需要从当前“仅返回字段列表”的模型升级为“字段列表 + 生成选项”的模型。

每个 symbol 的规范化结果建议包含两部分：

- `desired_output_fields`
- `generation_options`

其中：

- 普通字段如 `"func_name"`、`"vfunc_sig"` 进入 `desired_output_fields`
- directive `"vfunc_sig_max_match:10"` 被解析为：
  - 输出字段 `vfunc_sig_max_match`
  - 生成选项 `{"vfunc_sig_max_match": 10}`

建议规范化后的内部结构为：

```python
{
    "INetworkMessages_GetLoggingChannel": {
        "desired_output_fields": [
            "func_name",
            "vfunc_sig",
            "vfunc_sig_max_match",
            "vfunc_offset",
            "vfunc_index",
            "vtable_name",
        ],
        "generation_options": {
            "vfunc_sig_max_match": 10,
        },
    }
}
```

### 2. 合法字段与稳定顺序

`func/vfunc` 类型的合法字段集合新增：

- `vfunc_sig_max_match`

稳定顺序建议更新为：

1. `func_name`
2. `func_va`
3. `func_rva`
4. `func_size`
5. `func_sig`
6. `vtable_name`
7. `vfunc_offset`
8. `vfunc_index`
9. `vfunc_sig`
10. `vfunc_sig_max_match`

这样可保持现有顺序基本不变，并把 `vfunc_sig_max_match` 作为 `vfunc_sig` 的附属元数据放在其后。

### 3. 契约校验规则

新增以下校验：

- `vfunc_sig_max_match:N` 中的 `N` 必须是正整数
- 同一 symbol 不允许声明多个 `vfunc_sig_max_match:*`
- 如果声明了 `vfunc_sig_max_match:*`，同一 symbol 必须同时声明 `vfunc_sig`
- `vfunc_sig_max_match` 只允许出现在 `func/vfunc` 类型 symbol 中

以下情况直接失败：

- `"vfunc_sig_max_match:abc"`
- `"vfunc_sig_max_match:0"`
- `"vfunc_sig_max_match:-1"`
- 同一个 symbol 同时出现 `"vfunc_sig_max_match:10"` 与 `"vfunc_sig_max_match:20"`
- 存在 `"vfunc_sig_max_match:10"` 但没有 `vfunc_sig`

未声明 `vfunc_sig_max_match` 时，默认值为 `1`，完全保持现有行为。

### 4. payload 组装

`_assemble_symbol_payload(...)` 不应再直接迭代原始字段字符串列表，而应只处理规范化后的 `desired_output_fields`。

当某个 symbol 声明了 `vfunc_sig_max_match:N` 时，`candidate_data` 中必须出现：

```python
"vfunc_sig_max_match": N
```

最终写出的 YAML 形态应为：

```yaml
func_name: INetworkMessages_GetLoggingChannel
vtable_name: INetworkMessages
vfunc_offset: '0x120'
vfunc_index: 36
vfunc_sig: FF 90 20 01 00 00 ...
vfunc_sig_max_match: 10
```

### 5. `vfunc_sig` 生成路径

slot-only vfunc LLM fallback 路径如下：

```python
preprocess_common_skill(...)
-> _build_enriched_slot_only_vfunc_payload_via_mcp(...)
-> preprocess_gen_vfunc_sig_via_mcp(...)
```

这条链路需要支持把 `vfunc_sig_max_match` 逐层透传。

建议在 `_build_enriched_slot_only_vfunc_payload_via_mcp(...)` 中新增参数：

```python
vfunc_sig_max_match=1
```

在 `preprocess_gen_vfunc_sig_via_mcp(...)` 中新增参数：

```python
max_match_count=1
```

成功条件从当前的：

- `match_count == 1`

改为：

- `1 <= match_count <= max_match_count`
- 且当前目标 `inst_va` 必须位于返回的 matches 中

一旦满足该条件，就停止继续扩展 signature bytes。

### 6. `find_bytes` 的调用方式

为了区分“命中数已经不超过 N”与“实际命中数大于 N 但被截断”，`preprocess_gen_vfunc_sig_via_mcp(...)` 在测试候选签名时，不应继续固定使用：

```python
limit = 2
```

而应改为：

```python
limit = max_match_count + 1
```

例如：

- `N = 1` 时，`limit = 2`，行为与当前完全一致
- `N = 10` 时，`limit = 11`

判定逻辑：

- 若 `match_count > N`，继续扩展
- 若 `1 <= match_count <= N` 且命中列表包含目标 `inst_va`，则接受该签名

### 7. 旧 YAML 复用路径

`preprocess_func_sig_via_mcp(...)` 当前内部 helper `_find_unique_match(...)` 被同时用于：

- `func_sig`
- `vfunc_sig`

本次不建议直接放宽 `_find_unique_match(...)`，否则会把其它签名类型的“必须唯一匹配”语义一起改松。

建议新增一个仅供 `vfunc_sig` 使用的受限匹配 helper，例如：

```python
_find_match_with_limit(signature, label, max_match_count)
```

语义：

- `func_sig` 仍走 `_find_unique_match(...)`
- `vfunc_sig` fallback 读取旧 YAML 中的 `vfunc_sig_max_match`
- 若不存在该字段，则默认按 `1` 处理
- 若存在，则允许 `match_count <= vfunc_sig_max_match`

返回值仍然可以是一个代表性匹配地址，用于日志输出；真正定位函数地址的逻辑仍然依赖：

- `vtable_name`
- `vfunc_index`

也就是说，多匹配只意味着“该 `vfunc_sig` 仍然有效”，并不意味着用它直接反推出唯一函数。

### 8. slot-only YAML 的复用语义

对没有 `func_va` 的 slot-only vfunc YAML：

- 复用 `vfunc_sig` 时允许最多 `N` 个匹配
- 但最终仍然只沿用：
  - `vtable_name`
  - `vfunc_offset`
  - `vfunc_index`
  - `vfunc_sig`
  - `vfunc_sig_max_match`

不会因为 `vfunc_sig` 存在多个命中而尝试从这些命中地址中选具体函数地址。

### 9. 目标脚本形态

`ida_preprocessor_scripts/find-INetworkMessages_GetLoggingChannel-windows.py` 的目标形态保持为：

```python
GENERATE_YAML_DESIRED_FIELDS = [
    (
        "INetworkMessages_GetLoggingChannel",
        [
            "func_name",
            "vfunc_sig",
            "vfunc_sig_max_match:10",
            "vfunc_offset",
            "vfunc_index",
            "vtable_name",
        ],
    ),
]
```

无需再引入新的独立配置入口。

## 测试设计

### 1. 契约解析测试

在 `tests/test_ida_analyze_util.py` 中新增或调整以下测试：

- 解析 `"vfunc_sig_max_match:10"` 成功
- 输出字段中包含 `vfunc_sig_max_match`
- 生成选项中记录整数值 `10`
- 非法值、重复声明、缺少 `vfunc_sig` 时失败

### 2. `vfunc_sig` 生成测试

为 `preprocess_gen_vfunc_sig_via_mcp(...)` 增加测试：

- `max_match_count=1` 时保持唯一匹配逻辑
- `max_match_count=10` 且 `match_count=2` 时成功
- `max_match_count=10` 且 `match_count=10` 时成功
- `max_match_count=10` 且 `match_count=11` 时继续扩展，不应提前成功
- 命中结果若不包含目标 `inst_va`，即使 `match_count <= N` 也失败

### 3. slot-only fallback 测试

扩展现有 slot-only vfunc 测试，验证：

- `preprocess_common_skill(...)` 会把 `vfunc_sig_max_match` 透传到 `preprocess_gen_vfunc_sig_via_mcp(...)`
- 最终写出的 payload 包含：
  - `func_name`
  - `vfunc_sig`
  - `vfunc_sig_max_match`
  - `vtable_name`
  - `vfunc_offset`
  - `vfunc_index`

### 4. 旧 YAML 复用测试

为 `preprocess_func_sig_via_mcp(...)` 增加测试：

- 旧 YAML 带 `vfunc_sig_max_match: 10` 时，`vfunc_sig` 多匹配仍可成功进入 fallback
- 旧 YAML 不带该字段时，仍要求唯一匹配
- 旧 YAML 带非法 `vfunc_sig_max_match` 时失败

### 5. 目标脚本测试

在 `tests/test_ida_preprocessor_scripts.py` 中，为 `find-INetworkMessages_GetLoggingChannel-windows.py` 增加断言：

- 转发给 `preprocess_common_skill(...)` 的 `generate_yaml_desired_fields` 中包含：

```python
"vfunc_sig_max_match:10"
```

## 风险与权衡

### 风险 1：directive 与真实字段混用后增加解析复杂度

应对：

- 只为当前确有需求的 `vfunc_sig_max_match` 增加一类 directive
- 不把任意 `name:value` 都当作通用机制无限扩张

### 风险 2：误把多匹配语义扩散到其它签名类型

应对：

- 不修改 `_find_unique_match(...)` 的语义
- 只在 `vfunc_sig` 路径引入受限匹配 helper

### 风险 3：匹配计数被 `find_bytes limit` 截断导致误判

应对：

- 统一使用 `limit = N + 1`
- 只有在确认 `match_count <= N` 时才接受

## 验收标准

- `GENERATE_YAML_DESIRED_FIELDS` 中的 `"vfunc_sig_max_match:10"` 不再被视为非法字段
- 最终 YAML 会持久化 `vfunc_sig_max_match: 10`
- `preprocess_gen_vfunc_sig_via_mcp(...)` 在匹配数降到 `<= N` 时停止扩展
- `preprocess_func_sig_via_mcp(...)` 读取旧 YAML 后，会按 `vfunc_sig_max_match` 放宽 `vfunc_sig` 复用匹配条件
- 未声明 `vfunc_sig_max_match` 的所有现有脚本行为保持不变

## 实施边界

- 本次只覆盖 `vfunc_sig_max_match`
- 若未来需要类似 `func_sig_max_match`、`gv_sig_max_match` 等能力，应重新单独设计，而不是在本次实现中顺手泛化
