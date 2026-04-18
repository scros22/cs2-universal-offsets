# RegisterConCommand 预处理公共 helper 设计

## 背景

当前仓库中的 `preprocess_common_skill` 更擅长处理“通过字符串或其他 xref 条件定位目标函数本身”的场景。

但 `BotAdd_CommandHandler` 这类目标并不直接由 usage/help 字符串所在函数定义，而是作为 `RegisterConCommand(...)` 的 handler 参数被注册。

这类场景有两个特点：

1. 目标函数真实入口藏在注册 API 的参数中，而不是字符串 xref 所在函数本身
2. Linux 下注册逻辑可能出现在一个非常大的 CRT 静态初始化函数中，不能依赖“小而稳定的注册函数”这一假设

因此，本次需要新增一个独立的公共 helper，专门解决“通过 `RegisterConCommand` 调用点提取 handler 函数”的预处理问题，并用 `find-BotAdd_CommandHandler.py` 作为首个调用脚本落地。

## 目标

- 新增 `ida_preprocessor_scripts/_registerconcommand.py`
- 新增高层入口 `preprocess_registerconcommand_skill(...)`
- 支持通过 `command_name` 和 `help_string` 两种条件定位候选注册点
- 在同时提供两者时，要求候选调用点同时满足两个条件
- `command_name` 一旦提供，必须强制完全匹配，禁止子串、前缀或模糊匹配
- `help_string` 按完整字符串匹配处理
- Windows 与 Linux 都以“调用点附近有限窗口回溯取参”为核心，不依赖伪代码文本
- Linux 不依赖所属函数边界稳定，允许注册调用存在于超大初始化函数内
- 调用脚本按 `GENERATE_YAML_DESIRED_FIELDS` 声明输出字段契约
- 成功时写出普通函数 YAML，失败时返回 `False` 交由上层 Agent 流程回退

## 非目标

- 本次不改造 `preprocess_common_skill`
- 本次不把 `RegisterConCommand` 之外的注册 API 一并抽象进统一框架
- 本次不引入 help 文案模糊匹配、正则匹配或版本漂移兼容逻辑
- 本次不新增调试型扩展字段到最终 YAML，例如 `register_callsite`、`matched_help_string`
- 本次不顺带改造 `config.yaml` schema

## 方案比较

### 方案 1：指令级调用点分析 + 双条件过滤

先通过 `command_name` / `help_string` 找字符串地址与 xref，再围绕 xref 附近的 `RegisterConCommand` 调用点做平台相关参数恢复，最终提取 handler。

优点：

- 最符合“确定性预处理脚本”的定位
- 不依赖伪代码文本，跨平台稳定性更高
- 可以自然适配 Linux 超大初始化函数场景
- 公共 helper 复用价值高

缺点：

- 需要写平台相关的调用约定恢复逻辑

### 方案 2：基于反编译伪代码文本匹配 `RegisterConCommand(...)`

在伪代码层搜索目标调用，再从参数文本中提取 handler。

优点：

- 直观

缺点：

- 高度依赖 Hex-Rays 输出形态
- 容易受优化、局部变量恢复、类型恢复影响
- 不适合作为稳定预处理基础设施

### 方案 3：先复用 `func_xrefs` 找注册函数，再二次提取 handler

先让现有 `preprocess_common_skill` 定位包含字符串的注册函数，再由另一个逻辑继续从注册函数里提取 handler。

优点：

- 可复用部分已有字符串定位机制

缺点：

- 逻辑边界不清晰
- Linux 大函数场景下，“先定位注册函数”本身并不稳定
- 会把“定位调用点”和“提取真正目标函数”拆成两套不自然的流程

## 选定方案

采用方案 1：指令级调用点分析 + 双条件过滤。

这是满足当前需求的最小充分方案，同时能够保持脚本输出的确定性与可复用性。

## 详细设计

### 1. 新公共接口

新增模块：

- `ida_preprocessor_scripts/_registerconcommand.py`

新增高层入口：

```python
async def preprocess_registerconcommand_skill(
    session,
    expected_outputs,
    new_binary_dir,
    platform,
    image_base,
    target_name,
    generate_yaml_desired_fields,
    command_name=None,
    help_string=None,
    rename_to=None,
    expected_match_count=1,
    search_window_before_call=48,
    search_window_after_xref=24,
    debug=False,
):
    ...
```

约束：

- `command_name` 与 `help_string` 至少提供一个
- `generate_yaml_desired_fields` 为必传
- `expected_match_count` 默认为 1
- 返回值为 `True` / `False`

### 2. 调用脚本形态

新增：

- `ida_preprocessor_scripts/find-BotAdd_CommandHandler.py`

调用脚本只负责声明常量与字段契约，例如：

```python
TARGET_FUNCTION_NAMES = [
    "BotAdd_CommandHandler",
]

COMMAND_NAME = "bot_add"
HELP_STRING = (
    "bot_add <t|ct> <type> <difficulty> <name> - "
    "Adds a bot matching the given criteria."
)

GENERATE_YAML_DESIRED_FIELDS = [
    (
        "BotAdd_CommandHandler",
        [
            "func_name",
            "func_sig",
            "func_va",
            "func_rva",
            "func_size",
        ],
    ),
]
```

然后在 `preprocess_skill(...)` 中调用 `preprocess_registerconcommand_skill(...)`。

### 3. 定位模型：以调用点为中心，而不是以函数为中心

公共 helper 的核心分析单位不是“某个函数”，而是“某个字符串 xref 附近的一次 `RegisterConCommand` 调用点”。

这样设计的原因是：

- Linux 场景中，注册逻辑可能位于超大的 CRT 静态初始化函数内
- 如果先把整个大函数当作分析边界，模式匹配容易变脆
- 调用点附近的几十条指令通常比整个函数形态稳定得多

因此，helper 应采用以下流程：

1. 解析 `command_name` 与 `help_string` 对应的字符串地址
2. 收集这些字符串的 xref
3. 对每个 xref，仅在局部窗口中尝试收敛到附近的 `RegisterConCommand` 调用
4. 一旦找到候选调用点，再围绕该调用点做参数恢复

### 4. 匹配规则

#### 4.1 `command_name`

`command_name` 一旦提供，必须强制完全匹配：

- 不允许子串匹配
- 不允许前缀匹配
- 不允许模糊匹配

原因是短命令名很容易误命中其他无关字符串，完全匹配能显著降低噪声。

#### 4.2 `help_string`

`help_string` 按完整字符串匹配处理。

本次不引入模糊匹配或文案漂移兼容逻辑。如果未来遇到版本变化导致 usage 文案轻微修改，再单独扩展。

#### 4.3 双条件组合

当 `command_name` 与 `help_string` 同时提供时：

- 候选调用点必须同时满足两项条件
- 只有通过双条件过滤的调用点，才允许进入 handler 提取阶段

当只提供其中一项时：

- 可以用单条件定位候选调用点
- 但最终仍要求唯一 handler

### 5. 平台相关 handler 提取策略

#### 5.1 Linux

围绕 `RegisterConCommand` 调用点向前有限窗口回溯寄存器赋值，重点恢复：

- `rsi`：`command_name`
- `rdx`：handler
- `r8`：`help_string`
- `r9d`：flags

对当前已知样本，`rdx` 可直接指向 `BotAdd_CommandHandler`。

Linux 分析不依赖当前调用点所属函数是否为“小函数”或“业务函数”，只依赖：

- 调用点在窗口内可见
- 关键参数赋值在窗口内可恢复

#### 5.2 Windows

围绕 `RegisterConCommand` 调用点向前有限窗口回溯，重点恢复：

- `rdx`：`command_name`
- `r8`：本地 handler slot 地址
- `r9`：`help_string`

由于 Windows 样本中第三参数不是直接的 handler 地址，而是一个局部结构或栈槽地址，因此还需要：

1. 回溯该本地 slot 的初始化位置
2. 解析其中保存的真实 handler 地址
3. 将其作为最终目标函数入口

#### 5.3 统一原则

- 两个平台都基于真实机器指令，不依赖反编译伪代码文本
- 两个平台都只分析调用点附近有限窗口
- 参数恢复失败即视为当前候选点无效，不做猜测式补救

### 6. 输出契约：统一遵循 `GENERATE_YAML_DESIRED_FIELDS`

本 helper 不引入专用的 `generate_func_sig=True/False` 开关，而是统一遵循调用脚本声明的 `GENERATE_YAML_DESIRED_FIELDS`。

规则：

- `find-BotAdd_CommandHandler.py` 必须声明 `GENERATE_YAML_DESIRED_FIELDS`
- helper 根据字段清单决定需要生成哪些字段
- 若字段清单中包含 `func_sig`，则 helper 再调用现有函数签名生成逻辑
- 若声明了某字段但无法生成，则严格失败

首个调用脚本建议字段集合为：

- `func_name`
- `func_sig`
- `func_va`
- `func_rva`
- `func_size`

最终 YAML 保持与普通函数技能一致，不额外写入调试或注册语义字段。

### 7. 内部数据流

helper 内部建议拆成以下阶段：

#### 7.1 字符串解析阶段

- 查找 `command_name` 完全匹配的字符串地址
- 查找 `help_string` 完全匹配的字符串地址
- 若两个条件都存在，则分别维护地址集合

#### 7.2 候选调用点收集阶段

- 枚举相关字符串的 xref
- 在 xref 附近窗口内搜索 `RegisterConCommand` 调用
- 将“调用点 + 已恢复出的局部参数”作为候选记录

#### 7.3 候选过滤阶段

- 按 `command_name` / `help_string` 条件严格过滤
- 对同一调用点的重复来源做去重
- 对恢复出的 handler 地址做汇总

#### 7.4 目标函数解析阶段

- 查询 handler 是否落在函数头
- 读取 `func_va`、`func_size`
- 根据字段契约按需生成 `func_rva`、`func_sig`

#### 7.5 YAML 输出阶段

- 构造 payload
- 使用现有 `write_func_yaml(...)` 写目标文件
- 若配置了 `rename_to`，执行 best-effort rename

### 8. 失败策略

以下任一情况直接返回 `False`：

- `command_name` 与 `help_string` 都未提供
- 找不到匹配字符串
- 找到字符串，但无法收敛到附近的 `RegisterConCommand` 调用
- 找到调用点，但无法恢复关键参数
- 同一技能最终得到多个不同 handler
- handler 无法确认函数边界
- `GENERATE_YAML_DESIRED_FIELDS` 中声明的字段无法完整生成
- 输出文件匹配不到 `expected_outputs`

helper 作为预处理加速路径，不负责推断式修复；一旦唯一性或确定性不成立，就应回退到上层 Agent 流程。

### 9. 调试输出

在 `debug=True` 时，建议打印：

- 命中的 `command_name` / `help_string` 字符串地址数量
- 候选 xref 数量
- 成功收敛的 `RegisterConCommand` 调用点数量
- 每个平台恢复出的 handler 地址
- 最终输出文件路径

调试信息只用于控制台，不进入 YAML。

## 风险与权衡

### 风险 1：窗口过小导致漏掉参数恢复

如果调用点之前的关键赋值距离较远，有限窗口可能无法恢复完整参数。

应对方式：

- 提供保守但可调的窗口参数
- 首版保持小而稳，失败时返回 `False`

### 风险 2：Windows 局部 slot 形态存在编译器差异

若第三参数的局部结构布局变化，slot 解引用逻辑可能失效。

应对方式：

- 先针对当前已知样本设计最小充分逻辑
- 仅在确有新样本时再扩展

### 风险 3：未来 usage 文案微调

若仅依赖 `help_string`，版本间轻微改字可能导致定位失败。

应对方式：

- 首版支持同时声明 `command_name`
- 当两者都存在时优先走双条件强过滤

## 验证方式

本次设计对应的验证目标为：

1. `find-BotAdd_CommandHandler.py` 能在 Windows 上通过 `command_name + help_string` 成功提取唯一 handler
2. `find-BotAdd_CommandHandler.py` 能在 Linux 上即使面对超大 CRT 初始化函数，也通过调用点局部分析成功提取唯一 handler
3. 输出 YAML 字段严格符合 `GENERATE_YAML_DESIRED_FIELDS`
4. 任一条件不满足时，helper 严格返回 `False`

本次仅定义设计，不在设计阶段声称验证已通过。

## 实施范围

本设计落地时预期修改文件为：

- `ida_preprocessor_scripts/_registerconcommand.py`
- `ida_preprocessor_scripts/find-BotAdd_CommandHandler.py`

若后续需要接入 `config.yaml` 或新增其他 `find-*.py` 调用脚本，应在实现计划中单独列出。

## 结论

本次采用新增 `RegisterConCommand` 专用公共 helper 的方式，绕开 `preprocess_common_skill` 在此类问题上的边界限制。

核心思想是：

- 以调用点为中心，而不是以函数为中心
- 以严格完全匹配为前提，而不是模糊字符串定位
- 以 `GENERATE_YAML_DESIRED_FIELDS` 为输出契约，而不是 helper 自行决定输出形状

该方案对 `BotAdd_CommandHandler` 是最小充分实现，同时也为未来其他控制台命令 handler 提供统一预处理模板。
