# preprocess_common_skill 的 generate_yaml_desired_fields 强制输出契约设计

## 背景

当前 `preprocess_common_skill` 的 YAML 产出方式，仍然主要由各类 `write_*_yaml` writer 的固定 key 顺序决定。

这带来两个问题：

1. 输出字段并不真正由具体脚本声明控制，而是由公共层“能拿到什么就写什么”
2. 某些特殊 symbol 并不适合输出完整传统 shape，例如某些 vfunc 场景只能稳定产出 `vfunc_sig`，却不一定能稳定产出 `func_sig`、`func_va`

同时，当前 `FUNC_VTABLE_RELATIONS` 仍携带 `generate_vfunc_offset` 这类输出控制语义，导致“关系定义”和“输出契约定义”耦合在一起。

本次希望把输出控制统一收敛到 `generate_yaml_desired_fields`，让 `preprocess_common_skill` 不再自由组装 YAML，而是严格根据脚本声明的字段契约生成 YAML。

## 目标

- 为 `preprocess_common_skill` 增加强制参数 `generate_yaml_desired_fields`
- 将 `generate_yaml_desired_fields` 设计为公共机制，覆盖：
  - `func` / `vfunc`
  - `gv`
  - `vtable`
  - `patch`
  - `struct-member`
- 所有 YAML 最终只输出声明过的字段
- 声明了的字段若无法生成，严格失败
- `FUNC_VTABLE_RELATIONS` 不再承担输出控制职责
- 删除 `generate_vfunc_offset` 语义，是否生成 `vfunc_offset` / `vfunc_index` 完全由 `generate_yaml_desired_fields` 决定
- 执行全量切换，不保留旧兼容分支

## 非目标

- 本次不改造 `preprocess_common_skill` 之外的自定义预处理脚本输出机制
- 本次不顺带重构与本需求无关的 symbol 定位逻辑
- 本次不新增自动推导默认字段集的兼容模式
- 本次不要求修改 `config.yaml` schema

## 方案比较

### 方案 1：显式字段契约 + 统一字段规划器

在 `preprocess_common_skill` 中新增统一字段契约层。每个 symbol 先产出候选字段，再按 `generate_yaml_desired_fields` 组装最终 YAML，并对缺失字段严格失败。

优点：

- 输出语义最清晰
- 与“严格失败”“全量切换”目标完全一致
- 可自然扩展到所有 symbol 类型

缺点：

- 需要批量迁移所有 `preprocess_common_skill` 调用脚本

### 方案 2：显式字段契约 + 旧 writer 默认补齐

要求新参数存在，但公共层对未声明字段回退到旧 writer 的默认字段集合。

优点：

- 迁移成本较低

缺点：

- 仍保留“自由组装 YAML”的旧语义
- 与本次目标不一致

### 方案 3：新增独立 emitter 层

把“定位 symbol”和“输出 YAML”彻底拆为两个公共层，新增专门的 emitter/assembler 模块。

优点：

- 长期架构边界更清晰

缺点：

- 对当前需求属于过重设计
- 改造面明显超出最短路径

## 选定方案

采用方案 1：显式字段契约 + 统一字段规划器。

这是满足当前需求的最小充分方案，同时为后续扩展保留统一模型。

## 详细设计

### 1. 公共接口变更

`preprocess_common_skill(...)` 新增必传参数：

```python
generate_yaml_desired_fields=None
```

该参数为公共强制参数，不再允许省略。

统一声明格式为：

```python
GENERATE_YAML_DESIRED_FIELDS = [
    ("symbol_name", ["field1", "field2", "field3"]),
]
```

规则：

- `symbol_name` 必须是字符串，且必须唯一
- 字段列表必须是非空列表
- 字段名必须是字符串，且不能为空
- 同一个 symbol 不允许重复定义
- 每个实际要产出的 symbol 都必须在此表中出现

### 2. symbol 类型与合法字段集合

公共层为不同 symbol 类型维护合法字段集合与稳定输出顺序。

#### 2.1 `func` / `vfunc`

合法字段：

- `func_name`
- `func_va`
- `func_rva`
- `func_size`
- `func_sig`
- `vtable_name`
- `vfunc_offset`
- `vfunc_index`
- `vfunc_sig`

稳定顺序：

1. `func_name`
2. `func_va`
3. `func_rva`
4. `func_size`
5. `func_sig`
6. `vtable_name`
7. `vfunc_offset`
8. `vfunc_index`
9. `vfunc_sig`

#### 2.2 `gv`

合法字段：

- `gv_name`
- `gv_va`
- `gv_rva`
- `gv_sig`
- `gv_sig_va`
- `gv_inst_offset`
- `gv_inst_length`
- `gv_inst_disp`

#### 2.3 `vtable`

合法字段：

- `vtable_class`
- `vtable_symbol`
- `vtable_va`
- `vtable_rva`
- `vtable_size`
- `vtable_numvfunc`
- `vtable_entries`

#### 2.4 `patch`

合法字段：

- `patch_name`
- `patch_sig`
- `patch_bytes`

#### 2.5 `struct-member`

合法字段：

- `struct_name`
- `member_name`
- `offset`
- `size`
- `offset_sig`
- `offset_sig_disp`

### 3. `FUNC_VTABLE_RELATIONS` 语义收敛

`FUNC_VTABLE_RELATIONS` 从三元组改为二元组：

```python
FUNC_VTABLE_RELATIONS = [
    ("func_name", "vtable_class"),
]
```

新语义仅表达“函数属于哪个 vtable 语义域”。

不再允许：

```python
("func_name", "vtable_class", generate_vfunc_offset)
```

`vfunc_offset` / `vfunc_index` 是否需要生成，不再由 `FUNC_VTABLE_RELATIONS` 决定，而只由 `generate_yaml_desired_fields` 决定。

### 4. 内部数据流改造

`preprocess_common_skill` 改为两阶段流程：

#### 第一阶段：定位并收集候选字段

按现有逻辑定位目标 symbol，但不直接写 YAML，而是先生成候选字段集。

候选字段集表示“当前解析路径实际拿到了哪些字段”。例如函数路径可能得到：

- `func_name`
- `func_va`
- `func_rva`
- `func_size`
- `func_sig`
- `vtable_name`
- `vfunc_offset`
- `vfunc_index`
- `vfunc_sig`

候选字段集不是最终输出，只是供第二阶段组装使用。

#### 第二阶段：按字段契约组装最终 payload

对每个 symbol：

1. 查找其 `generate_yaml_desired_fields`
2. 逐个字段尝试从候选字段集中取值
3. 若字段需要派生计算，则在该阶段统一计算
4. 任一声明字段缺失或无法派生，立即失败
5. 按该 symbol 类型的稳定字段顺序写入 YAML

最终 payload 只包含声明字段，不包含任何未声明字段。

### 5. 严格失败规则

以下情况任一发生，`preprocess_common_skill` 直接返回 `False`：

- 未传 `generate_yaml_desired_fields`
- 某个实际要处理的 symbol 没有字段契约
- 某个 symbol 的字段契约重复定义
- 字段列表为空
- 字段名不属于该 symbol 类型的合法字段集合
- 声明了某字段，但最终无法生成或派生

### 6. 函数类 symbol 的字段派生规则

#### 6.1 `vtable_name`

优先级：

1. 若 `FUNC_VTABLE_RELATIONS` 已声明，直接使用对应 `vtable_class`
2. 否则若候选字段集中已有 `vtable_name`，使用该值
3. 否则失败

#### 6.2 `vfunc_offset` / `vfunc_index`

仅当字段契约声明需要时，才执行解析。

优先级：

1. 若候选字段集中已有，直接使用
2. 否则若已有 `vtable_name` 且可得到 `func_va`，则通过 vtable entries 反查 slot
3. 仍无法得到时失败

#### 6.3 `func_sig` / `vfunc_sig`

只要字段契约声明了，就必须由当前路径真实生成。不能因为旧 writer 不写某字段而视为成功。

#### 6.4 纯虚函数或 slot-only vfunc

允许存在仅输出部分字段的函数 YAML，只要字段契约与可生成能力一致即可。

例如以下字段契约是合法的：

```python
("INetworkMessages_FindNetworkGroup", [
    "func_name",
    "vfunc_sig",
    "vfunc_offset",
    "vfunc_index",
    "vtable_name",
])
```

此时不会强制要求 `func_va`、`func_sig`。

### 7. 非函数类 symbol 的组装规则

所有非函数类 symbol 统一适用“候选字段集 + 契约组装 + 严格失败”模型。

#### 7.1 `gv`

只输出声明字段，例如：

- `gv_name`
- `gv_sig`
- `gv_inst_offset`

声明了但拿不到即失败。

#### 7.2 `vtable`

只输出声明字段，例如：

- `vtable_class`
- `vtable_numvfunc`
- `vtable_entries`

声明了但拿不到即失败。

#### 7.3 `patch`

只输出声明字段，例如：

- `patch_name`
- `patch_sig`
- `patch_bytes`

#### 7.4 `struct-member`

只输出声明字段，例如：

- `struct_name`
- `member_name`
- `offset`
- `size`
- `offset_sig`

### 8. Writer 行为调整

现有 `write_func_yaml`、`write_gv_yaml`、`write_vtable_yaml`、`write_patch_yaml`、`write_struct_offset_yaml` 不再作为“最终字段集合定义来源”。

新的职责分工应为：

- 公共层负责确定最终 payload 中有哪些字段
- writer 只负责按传入 payload 落盘，并保持稳定顺序

因此 writer 层需要支持：

- 只写入公共层已选定的 payload 字段
- 不偷偷补齐未声明字段
- 继续保持 `yaml.safe_dump(sort_keys=False)` 的稳定输出行为

### 9. 迁移方案

本次采用全量切换，不保留旧兼容路径。

迁移范围：

1. 修改 `ida_analyze_util.py`
2. 修改全部 `ida_preprocessor_scripts/*.py` 中调用 `preprocess_common_skill` 的脚本
3. 修改全部使用 `FUNC_VTABLE_RELATIONS` 的脚本

迁移要求：

- 所有 `preprocess_common_skill` 调用脚本都必须显式声明 `GENERATE_YAML_DESIRED_FIELDS`
- 所有 `FUNC_VTABLE_RELATIONS` 定义都必须改为二元组
- 所有脚本都必须显式把 `generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS` 传入 `preprocess_common_skill`

### 10. 目标脚本落地示例

`ida_preprocessor_scripts/find-INetworkMessages_FindNetworkGroup.py` 迁移后的关键形态应为：

```python
FUNC_VTABLE_RELATIONS = [
    ("INetworkMessages_FindNetworkGroup", "INetworkMessages"),
]

GENERATE_YAML_DESIRED_FIELDS = [
    (
        "INetworkMessages_FindNetworkGroup",
        ["func_name", "vfunc_sig", "vfunc_offset", "vfunc_index", "vtable_name"],
    ),
]
```

并在 `preprocess_skill(...)` 中显式传入：

```python
func_vtable_relations=FUNC_VTABLE_RELATIONS,
generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
```

### 11. 推荐字段模板

为方便全量迁移，可在设计层面给出推荐模板，但这些模板仅用于迁移参考，不在运行时自动补齐。

#### 11.1 普通 `func`

- `func_name`
- `func_va`
- `func_rva`
- `func_size`
- `func_sig`

#### 11.2 常规 `vfunc`

- `func_name`
- `func_va`
- `func_rva`
- `func_size`
- `func_sig`
- `vtable_name`
- `vfunc_offset`
- `vfunc_index`

#### 11.3 纯签名 `vfunc`

- `func_name`
- `vtable_name`
- `vfunc_sig`

必要时再增加：

- `vfunc_offset`
- `vfunc_index`

#### 11.4 `gv`

- `gv_name`
- `gv_va`
- `gv_rva`
- `gv_sig`
- `gv_sig_va`
- `gv_inst_offset`
- `gv_inst_length`
- `gv_inst_disp`

#### 11.5 `vtable`

- `vtable_class`
- `vtable_symbol`
- `vtable_va`
- `vtable_rva`
- `vtable_size`
- `vtable_numvfunc`
- `vtable_entries`

#### 11.6 `patch`

- `patch_name`
- `patch_sig`
- `patch_bytes`

#### 11.7 `struct-member`

- `struct_name`
- `member_name`
- `offset`
- `size`
- `offset_sig`
- `offset_sig_disp`

## 风险与应对

### 风险 1：全量切换导致批量脚本迁移量大

应对：

- 在公共层提供明确报错，指出缺少哪个 symbol 的字段契约
- 批量迁移时按 symbol 类型分批修改，减少回归面

### 风险 2：历史脚本隐式依赖旧 writer 默认字段

应对：

- 显式补齐 `GENERATE_YAML_DESIRED_FIELDS`
- 不保留旧兼容路径，强制把隐式行为显式化

### 风险 3：函数路径来源多样

应对：

- 用“候选字段集”隔离定位逻辑与输出逻辑
- 无论是 `func_sig`、`xref`、LLM fallback 还是 slot-only vfunc，都统一走同一套输出契约

### 风险 4：`vfunc_offset` / `vfunc_index` 依赖 vtable 反查

应对：

- 仅在字段契约声明需要时才求解
- 声明了但求不出则严格失败，避免输出半正确 YAML

## 验证方案

### Level 0：定向验证

覆盖目标脚本 `ida_preprocessor_scripts/find-INetworkMessages_FindNetworkGroup.py`：

- 已传入 `generate_yaml_desired_fields`
- `FUNC_VTABLE_RELATIONS` 已去掉第三个布尔位
- 最终 YAML 仅按声明字段输出

### Level 1：回归抽查

至少抽查以下类型各一个脚本：

- 普通 `func`
- 带 `FUNC_VTABLE_RELATIONS` 的 `vfunc`
- `gv`
- `vtable`
- `patch` 或 `struct-member`

### 验收标准

- 任一 `preprocess_common_skill` 脚本若未声明 `GENERATE_YAML_DESIRED_FIELDS`，直接失败
- 任一声明字段缺失，直接失败
- 最终 YAML 仅包含声明字段，且 key 顺序稳定
- 仓库内不再存在 `generate_vfunc_offset` 语义

## 实施边界

本设计覆盖：

- `preprocess_common_skill` 的公共接口与内部输出模型
- `FUNC_VTABLE_RELATIONS` 的语义收敛
- 所有 `preprocess_common_skill` 脚本的字段契约迁移

本设计不覆盖：

- 与本需求无关的自定义预处理脚本内部逻辑重写
- 运行时自动迁移旧脚本或自动补全字段契约

## 结论

`generate_yaml_desired_fields` 将成为 `preprocess_common_skill` 的唯一 YAML 输出契约来源。

公共层不再自由组装 YAML，而是先收集候选字段，再严格根据字段契约组装最终 payload。`FUNC_VTABLE_RELATIONS` 只保留 vtable 关系语义，不再控制 `vfunc_offset` 输出。

本次采用全量切换与严格失败策略，以换取统一、显式、可验证的预处理输出模型。
