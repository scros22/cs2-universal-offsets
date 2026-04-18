# DEFINE_INPUTFUNC 预处理共享 Helper 设计

## 背景

当前 `ida_preprocessor_scripts/find-ShowHudHint.py` 只是普通的 `preprocess_common_skill` 调用脚本，依赖旧版本 `func_sig` 复用来定位 `ShowHudHint`。但 `.claude/skills/find-ShowHudHint/SKILL.md` 描述的真实定位方式并不是“沿用旧签名”，而是：

- 先按精确字符串 `"ShowHudHint"` 找到字符串地址
- 再找到 `.data` 中引用该字符串的描述符项
- 读取 `string_ptr_addr + 0x10` 处的函数指针
- 将该函数指针视为真正的 input handler

仓库中已经有两类可借鉴的程序化模式：

- `ida_preprocessor_scripts/_registerconcommand.py`
  - 使用共享 helper + 薄配置脚本模式
  - 完成候选收集、唯一性校验、函数信息查询、`func_sig` 生成和 YAML 写出
- `ida_preprocessor_scripts/_register_event_listener_abstract.py`
  - 将平台相关恢复逻辑封装进 `py_eval`
  - Python 层只消费统一候选结构并执行严格校验

本设计的目标是新增一个针对 `DEFINE_INPUTFUNC` 类模式的共享 helper，并将 `find-ShowHudHint.py` 改造成薄配置脚本，使该 skill 改为完全程序化处理，不再依赖旧 `func_sig`。

## 目标

- 新增 `ida_preprocessor_scripts/_define_inputfunc.py`
- 共享 helper 支持“单个 input 字符串 -> 单个 input handler”的完整预处理流程
- helper 内部直接完成：
  - 字符串定位
  - 数据段描述符候选收集
  - `handler_ptr_offset` 处函数指针读取
  - `.text` 段有效性过滤
  - 函数信息查询
  - `func_sig` 生成
  - YAML 写出
  - best-effort rename
- 将 `ida_preprocessor_scripts/find-ShowHudHint.py` 改为仅声明常量并调用 helper 的薄配置脚本
- 保持 `ShowHudHint.{platform}.yaml` 的字段契约和下游消费方式不变
- 允许 helper 参数化：
  - 数据描述符允许所在段名集合
  - handler 指针偏移 `handler_ptr_offset`

## 非目标

- 不支持“多字符串 -> 多 handler”批量处理
- 不统一抽象所有 entity input / output / event 注册模式
- 不修改 `config.yaml` 中 `find-ShowHudHint` 的输出契约
- 不修改 `update_gamedata.py` 或任何下游 gamedata 生成逻辑
- 不尝试复用旧版本 `func_sig` 作为主定位路径
- 不对非 `.text` 段的 handler 指针做容错接受

## 方案选择

本次选择的方案是：

- 新增单目标共享 helper `preprocess_define_inputfunc_skill(...)`
- 由 helper 完整处理定位、校验、签名和 YAML 写出
- 调用脚本只声明 `target_name`、`input_name`、`handler_ptr_offset`、允许的数据段名集合、输出字段和可选重命名名

未选方案及原因：

1. 继续使用 `preprocess_common_skill`
   - 依赖旧版本 `func_sig`
   - 与 `ShowHudHint` 这类字符串描述符定位模式不匹配
   - 无法覆盖 skill 文档中定义的真实查找路径

2. 在 `find-ShowHudHint.py` 内直接硬编码全部逻辑
   - 能解决当前问题，但不符合仓库近期“共享 helper + 薄脚本”的演进方向
   - 后续若再出现相同模式，将产生重复代码

3. 一开始就支持多字符串多输出
   - 当前明确不需要
   - 会额外引入目标映射、批量失败语义和更多参数校验复杂度

## 架构

### 新增共享 helper

新增文件：

- `ida_preprocessor_scripts/_define_inputfunc.py`

职责：

- 规范化并校验请求字段与输出路径
- 在 IDA 中程序化定位与 `input_name` 对应的描述符候选
- 按允许的数据段名集合过滤字符串引用点
- 从 `xref_from + handler_ptr_offset` 读取函数指针
- 只接受位于 `.text` 段的 handler 地址
- 对唯一 handler 生成函数元数据和 `func_sig`
- 写出 YAML
- 对真实函数执行 best-effort rename

### 薄配置脚本

将 `ida_preprocessor_scripts/find-ShowHudHint.py` 改为配置层，主要声明：

- `TARGET_NAME = "ShowHudHint"`
- `INPUT_NAME = "ShowHudHint"`
- `HANDLER_PTR_OFFSET = 0x10`
- `ALLOWED_SEGMENT_NAMES = (".data",)`
- `RENAME_TO = "ShowHudHint"`
- `GENERATE_YAML_DESIRED_FIELDS`

`preprocess_skill(...)` 仅负责转调共享 helper。

若未来出现其他“字符串描述符 + 固定偏移函数指针”的 input handler 目标，只需要新增一个新的薄脚本，并传入新的常量即可。

## Helper 接口

建议共享 helper 暴露主入口：

```python
async def preprocess_define_inputfunc_skill(
    session,
    expected_outputs,
    platform,
    image_base,
    target_name,
    input_name,
    generate_yaml_desired_fields,
    handler_ptr_offset=0x10,
    allowed_segment_names=(".data",),
    rename_to=None,
    debug=False,
):
    ...
```

### 参数约束

- `target_name`
  - YAML 中的目标符号名，例如 `ShowHudHint`
- `input_name`
  - 要精确匹配的字符串内容，例如 `ShowHudHint`
- `generate_yaml_desired_fields`
  - 沿用现有函数 YAML 字段契约
- `handler_ptr_offset`
  - 从数据引用点开始读取 handler 指针的偏移，默认 `0x10`
  - 必须是非负整数
- `allowed_segment_names`
  - 允许承载字符串描述符引用点的段名集合
  - 至少包含一个非空字符串
  - `ShowHudHint` 首版调用脚本使用保守值 `(".data",)`
- `rename_to`
  - 可选，若提供则对定位到的真实函数做 best-effort rename

## 候选收集模型

helper 应模仿现有 `_registerconcommand.py` / `_register_event_listener_abstract.py` 风格：

- 由 `_build_define_inputfunc_py_eval(...)` 构造平台无关的 IDA Python 代码
- 通过 `py_eval` 一次性收集候选并返回 JSON
- Python 层通过 `_collect_define_inputfunc_candidates(...)` 执行结构校验和唯一性判断

建议 `py_eval` 返回的统一结构为：

```python
{
    "ok": True,
    "string_eas": ["0x..."],
    "items": [
        {
            "string_ea": "0x...",
            "xref_from": "0x...",
            "xref_seg_name": ".data",
            "handler_ptr_ea": "0x...",
            "handler_va": "0x...",
            "handler_seg_name": ".text",
        }
    ],
}
```

说明：

- `string_eas`
  - 精确等于 `input_name` 的字符串地址列表
  - helper 最终要求唯一
- `xref_from`
  - 指向该字符串的引用地址
  - 在本设计中代表描述符项内保存字符串指针的位置
- `xref_seg_name`
  - `xref_from` 所在段名
  - 仅当其位于 `allowed_segment_names` 中时，才可参与后续处理
- `handler_ptr_ea`
  - `xref_from + handler_ptr_offset`
- `handler_va`
  - 从 `handler_ptr_ea` 读取出的 `u64` 值
- `handler_seg_name`
  - `handler_va` 所在段名
  - 本设计要求必须为 `.text`

## 定位流程

### 第一步：精确字符串定位

在 IDA 中遍历字符串表，只接受与 `input_name` 完全相等的字符串。

要求：

- 精确匹配，不做模糊匹配或大小写宽松匹配
- 过滤后的字符串地址必须唯一

若字符串结果不是唯一，helper 直接失败。

### 第二步：收集字符串的数据段引用

对唯一字符串地址执行 `XrefsTo`，并按以下规则过滤：

- 只保留 `xref.frm` 位于 `allowed_segment_names` 的候选
- 不接受代码段内的普通加载引用
- 不接受段名缺失或不可识别的候选

设计意图是把范围收缩到 skill 文档所描述的“数据段描述符项”，而不是全程序所有对该字符串的使用点。

### 第三步：读取 handler 指针

对每个数据段候选：

- 计算 `handler_ptr_ea = xref_from + handler_ptr_offset`
- 读取 `u64 handler_va`
- 查询 `handler_va` 所在段名
- 仅当 `handler_seg_name == ".text"` 时，视为有效候选

过滤后，将 `handler_va` 去重。

要求：

- 有效 `.text` handler 必须恰好唯一
- 若无有效 `.text` handler，则失败
- 若有效 `.text` handler 超过一个，则失败

这是本设计中最关键的稳健性约束，用于避免误把其他数据、虚表项、常量池项或无关函数指针视为目标函数。

## Python 层校验与输出流程

在 `_collect_define_inputfunc_candidates(...)` 返回候选后，主 helper 执行以下步骤：

1. 调用 `_normalize_requested_fields(...)` 校验 `generate_yaml_desired_fields`
2. 调用 `_resolve_output_path(...)` 定位 `target_name` 的输出 YAML 路径
3. 校验候选结构字段完整性
4. 要求唯一 `string_ea`
5. 仅保留 `xref_seg_name` 属于 `allowed_segment_names` 的项
6. 仅保留 `handler_seg_name == ".text"` 的项
7. 对 `handler_va` 去重后要求唯一
8. 使用 `_query_func_info(...)` 获取函数名、地址、大小等元信息
9. 若请求中包含 `func_sig`，调用 `preprocess_gen_func_sig_via_mcp(...)`
10. 使用 `_build_func_payload(...)` 组装 payload
11. 调用 `write_func_yaml(...)` 输出 YAML
12. 若提供 `rename_to`，调用 `_rename_func_best_effort(...)`

最终输出仍为标准函数 YAML，不引入任何新字段。

## 建议复用的内部函数

为保持风格一致，`_define_inputfunc.py` 建议直接复用或平移现有 helper 的通用小函数模式：

- `_normalize_requested_fields(...)`
- `_resolve_output_path(...)`
- `_query_func_info(...)`
- `_build_func_payload(...)`
- `_rename_func_best_effort(...)`
- `_call_py_eval_json(...)`

专属于本 helper 的新增函数建议为：

- `_build_define_inputfunc_py_eval(...)`
- `_collect_define_inputfunc_candidates(...)`

这样能保持新 helper 与 `_registerconcommand.py`、`_register_event_listener_abstract.py` 的结构一致，同时避免把 `ShowHudHint` 的特例写死在调用脚本之外的多个地方。

## YAML 契约

`find-ShowHudHint.py` 继续输出：

- `ShowHudHint.{platform}.yaml`

字段维持当前函数型 YAML 契约：

- `func_name`
- `func_va`
- `func_rva`
- `func_size`
- `func_sig`

不新增任何“input_name”或“descriptor_addr”辅助字段，避免影响下游消费逻辑。

## 失败语义

以下情况任一发生时，helper 必须返回 `False`：

- `target_name` 或 `input_name` 非法
- `handler_ptr_offset` 非法
- `allowed_segment_names` 非法或为空
- `expected_outputs` 中无法解析 `target_name` 的输出路径
- `generate_yaml_desired_fields` 中无法解析 `target_name` 的字段契约
- 精确字符串匹配结果不是唯一
- 数据段 xref 过滤后没有候选
- `xref_from + handler_ptr_offset` 读取失败
- 读取出的地址不在 `.text` 段，导致没有有效 handler
- 有效 `.text` handler 去重后不是唯一
- 函数信息查询失败
- `func_sig` 生成失败
- YAML 写出失败

best-effort rename 不应影响成功与否；即 rename 失败只记录调试信息，不改变主流程结果。

## `find-ShowHudHint.py` 改造方式

改造后脚本应具备以下特点：

- 删除对 `preprocess_common_skill` 的依赖
- 改为导入 `preprocess_define_inputfunc_skill`
- 只保留常量声明和 `preprocess_skill(...)` 包装
- 不再依赖 `old_yaml_map`

建议脚本形态如下：

```python
from ida_preprocessor_scripts._define_inputfunc import preprocess_define_inputfunc_skill

TARGET_NAME = "ShowHudHint"
INPUT_NAME = "ShowHudHint"
HANDLER_PTR_OFFSET = 0x10
ALLOWED_SEGMENT_NAMES = (".data",)
RENAME_TO = "ShowHudHint"

GENERATE_YAML_DESIRED_FIELDS = [
    (
        "ShowHudHint",
        ["func_name", "func_va", "func_rva", "func_size", "func_sig"],
    ),
]

async def preprocess_skill(
    session, skill_name, expected_outputs, old_yaml_map,
    new_binary_dir, platform, image_base, debug=False,
):
    return await preprocess_define_inputfunc_skill(
        session=session,
        expected_outputs=expected_outputs,
        platform=platform,
        image_base=image_base,
        target_name=TARGET_NAME,
        input_name=INPUT_NAME,
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        handler_ptr_offset=HANDLER_PTR_OFFSET,
        allowed_segment_names=ALLOWED_SEGMENT_NAMES,
        rename_to=RENAME_TO,
        debug=debug,
    )
```

说明：

- `old_yaml_map`、`new_binary_dir`、`skill_name` 在此模式下不再参与主逻辑，但保留参数以维持 `preprocess_skill(...)` 统一签名
- 若未来新增其他同模式目标，可以复制此薄脚本并替换常量

## 验证策略

本设计对应的是 Level 0 定向验证，不引入完整测试或构建要求。

完成实现后，最小验证应覆盖：

- `ida_preprocessor_scripts/_define_inputfunc.py` 导出 `preprocess_define_inputfunc_skill`
- `ida_preprocessor_scripts/find-ShowHudHint.py` 导出 `preprocess_skill`
- `find-ShowHudHint.py` 的输出文件名仍与 `config.yaml` 中 `find-ShowHudHint` 的配置一致
- helper 的失败语义符合本设计，尤其是：
  - 字符串唯一性
  - 数据段过滤
  - `.text` 段 handler 过滤
  - handler 唯一性
- YAML 输出字段保持不变

若需要更进一步的静态验证，可增加一次非常定向的 Python 级导入检查，但不作为本设计的必选项。

## 风险与权衡

- 风险：某些目标可能在未来版本不再把字符串描述符放在 `.data`
  - 应对：helper 已参数化 `allowed_segment_names`，但 `ShowHudHint` 首版仍采用保守值，避免误报
- 风险：某些描述符项可能出现多处相同字符串引用
  - 应对：本设计坚持“唯一字符串 + 唯一有效 `.text` handler”双重约束，宁可失败也不猜测
- 风险：函数指针读取点并非总是 `+0x10`
  - 应对：helper 将 `handler_ptr_offset` 参数化，但 `ShowHudHint` 调用脚本固定为 `0x10`
- 权衡：本设计没有抽象多目标批处理
  - 原因：当前明确不需要，优先保持 helper 简洁和失败语义清晰

## 实施范围

本设计只覆盖以下文件：

- 新增 `ida_preprocessor_scripts/_define_inputfunc.py`
- 修改 `ida_preprocessor_scripts/find-ShowHudHint.py`
- 如实现中需要，可同步补充与该 helper 直接相关的局部文档，但不扩展到其他 skill
