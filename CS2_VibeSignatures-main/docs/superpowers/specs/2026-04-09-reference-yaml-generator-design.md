# reference YAML 自动生成 CLI 设计

## 背景

当前 `call_llm_decompile` 的接线已经支持从 prompt 模板与 reference YAML 读取参考函数上下文，但 reference YAML 仍需要人工在 IDA 中导出并放到 `ida_preprocessor_scripts/references/...`。

对于 `CNetworkMessages_FindNetworkGroup` 这类场景，人工流程的核心工作其实很固定：

1. 根据参考函数名定位目标函数地址
2. 在 IDA 中导出该函数的反汇编与伪代码
3. 按固定 schema 写成 reference YAML
4. 放入 `ida_preprocessor_scripts/references/<module>/<func>.<platform>.yaml`

因此本次希望新增一个 project-level 的独立 CLI，把这条流程自动化；同时再提供一个轻量 SKILL 作为统一触发入口，让人和 agent 都走同一条实现路径。

## 目标

- 新增一个独立 CLI，用于自动生成 reference YAML。
- CLI 同时支持：
  - 连接已运行的 `ida-pro-mcp` / `idalib-mcp`
  - 自动启动 `idalib-mcp` 后执行导出
- CLI 的主输入是 `func_name`，而不是手工输入地址。
- 默认先从现有 `bin/<gamever>/<module>/<func>.<platform>.yaml` 读取 `func_va`；读不到时，再用 `config.yaml` 中的 symbol name / alias 到 IDA 搜索。
- reference YAML 采用最小 schema，仅包含：
  - `func_name`
  - `func_va`
  - `disasm_code`
  - `procedure`
- `module` 与 `platform` 不写入 YAML，而是体现在输出路径与文件名中，例如：
  - `ida_preprocessor_scripts/references/engine/CNetworkGameClient_RecordEntityBandwidth.windows.yaml`
- 新增一个 project-level SKILL，内部只调用该 CLI，不复制导出逻辑。
- 首版按“通用框架 + `func` 首版实现”落地，后续可扩展到 `vfunc` 或其他 reference 类型。

## 非目标

- 本次不直接把该 CLI 接入 `ida_analyze_bin.py` 主流程。
- 本次不一次性实现 `vfunc`、`vcall_finder detail`、struct/member 等所有 reference 类型。
- 本次不替代已有 `call_llm_decompile` 逻辑，只负责补齐其 reference 产物来源。
- 本次不要求 reference YAML 自动注册到 `config.yaml`。
- 本次不要求生成 reference YAML 后立即执行真实 LLM fallback 验证。

## 方案比较

### 方案 1：最薄 CLI

只做一个小脚本，输入 `func_name/module/platform/gamever`，导出 reference YAML。

优点：

- 实现最快
- 首版最小

缺点：

- 未来扩展到 `vfunc` / 其他 reference 类型时，容易继续堆条件分支
- 与 project-level SKILL 的协作边界不够明确

### 方案 2：通用框架 + `func` 首版实现

新增独立 CLI，但内部拆为“目标解析”“MCP 会话适配”“IDA 导出”“YAML 写入”几个小单元；首版只实现 `func` reference 导出。

优点：

- 满足当前需求的同时，为后续扩展预留清晰边界
- 适合作为 project-level SKILL 的统一后端
- 更容易测试和复用

缺点：

- 首版实现量略高于最薄 CLI

### 方案 3：直接做成 SKILL

把 reference 导出逻辑写入一个 SKILL，由 SKILL 直接连接 IDA 并落盘。

优点：

- 入口简单

缺点：

- 不符合“独立命令行工具”的目标
- 人工与自动化会变成两套入口
- 不利于后续在 shell / CI / 其他 agent 工作流中复用

## 选定方案

采用方案 2：实现一个独立 CLI，并提供一个 project-level SKILL 作为该 CLI 的统一触发器。

这是满足当前需求与未来扩展需求的最小充分方案。

## 详细设计

### 1. CLI 入口

建议新增独立脚本：

- `generate_reference_yaml.py`

建议命令形态：

```bash
uv run generate_reference_yaml.py \
  -gamever 14141 \
  -module engine \
  -platform windows \
  -func_name CNetworkGameClient_RecordEntityBandwidth
```

两种运行模式：

#### 1.1 连接现有 MCP

```bash
uv run generate_reference_yaml.py \
  -gamever 14141 \
  -module engine \
  -platform windows \
  -func_name CNetworkGameClient_RecordEntityBandwidth \
  -mcp_host 127.0.0.1 \
  -mcp_port 13337
```

#### 1.2 自动启动 `idalib-mcp`

```bash
uv run generate_reference_yaml.py \
  -gamever 14141 \
  -module engine \
  -platform windows \
  -func_name CNetworkGameClient_RecordEntityBandwidth \
  -binary bin/14141/engine/engine2.dll \
  -auto_start_mcp
```

规则：

- `-auto_start_mcp` 与 `-binary` 成对出现
- 未指定 `-auto_start_mcp` 时，默认连接已存在的 MCP 服务
- 两种模式共享同一套解析与导出逻辑

### 2. 内部结构

建议拆成以下逻辑单元，但首版可先放在一个文件中，后续再提炼：

#### 2.1 `ReferenceTargetResolver`

职责：

- 根据 `func_name/module/platform/gamever` 解析目标函数地址

解析顺序：

1. 先读：
   - `bin/<gamever>/<module>/<func_name>.<platform>.yaml`
2. 若该 YAML 存在且包含 `func_va`，直接使用
3. 若不存在或缺失 `func_va`，读取 `config.yaml`
   - 找到对应 `symbol`
   - 收集 `name` 与 `alias`
4. 使用这些候选名字在 IDA 中搜索函数

失败策略：

- 如果 YAML 与 IDA 搜索都失败，则 CLI 返回非零并输出明确错误

#### 2.2 `McpSessionAdapter`

职责：

- 统一封装“连接已有 MCP”和“自动启动 MCP”

能力：

- 复用现有仓库中的 MCP 连接模式
- 自动启动模式下复用 `ida_analyze_bin.py` 里的 `idalib-mcp` 启动参数习惯
- 失败时统一做 graceful cleanup

#### 2.3 `IdaReferenceExporter`

职责：

- 给定 `func_va`
- 通过 MCP 从 IDA 导出：
  - `disasm_code`
  - `procedure`

首版要求：

- 必须拿到反汇编文本
- 伪代码若 Hex-Rays 不可用，可允许为空字符串，但字段仍保留

建议优先复用仓库中已有“函数导出反汇编/伪代码”的 py_eval 组织方式，避免再造一套不一致的导出格式。

#### 2.4 `ReferenceYamlWriter`

职责：

- 生成最小 reference YAML
- 负责输出路径规范化与落盘

### 3. 输出 schema 与路径

reference YAML 最小 schema：

```yaml
func_name: CNetworkGameClient_RecordEntityBandwidth
func_va: 0x180123450
disasm_code: |
  ...
procedure: |
  ...
```

不额外写入：

- `module`
- `platform`
- `generated_at`
- `generated_by`

原因：

- 这些信息已经体现在路径与文件名中
- 当前 `call_llm_decompile` 实际只需要函数内容与名字
- 保持 reference 产物最小化更利于人工检查

默认输出路径：

```text
ida_preprocessor_scripts/references/<module>/<func_name>.<platform>.yaml
```

例如：

```text
ida_preprocessor_scripts/references/engine/CNetworkGameClient_RecordEntityBandwidth.windows.yaml
```

### 4. 函数名解析规则

CLI 主输入为：

- `func_name`

规则：

- `func_name` 是规范名，不要求用户自己提供 alias
- 解析地址时，优先读现有 YAML
- 现有 YAML 不可用时，再用 `config.yaml` 中该符号的 `name + alias` 到 IDA 搜索

这样可以最大化复用现有仓库资产，同时避免每次都依赖 IDA 名字匹配。

### 5. IDA 搜索策略

当需要 fallback 到 IDA 搜索时：

1. 先按 `func_name`
2. 再按 `alias` 列表逐个尝试
3. 若匹配到多个候选：
   - 输出歧义错误
   - 列出候选地址
   - 返回失败，不自动猜测

不做的事情：

- 不做模糊搜索后自动择优
- 不根据字符串 xref 再做复杂启发式推断

原因：

- CLI 的职责是“导出 reference”，不是重新实现符号定位框架
- 当前项目已有 `config.yaml` 与旧 YAML 作为更稳定的来源

### 6. 与 `call_llm_decompile` 的协作

该 CLI 不直接调用 LLM。

它只负责生成可被 `call_llm_decompile` 使用的 reference YAML。

因此正向链路为：

1. 用户或 SKILL 运行 CLI
2. CLI 生成：
   - `ida_preprocessor_scripts/references/<module>/<func_name>.<platform>.yaml`
3. `LLM_DECOMPILE` spec 引用该 YAML
4. `preprocess_common_skill(...)` 在运行 LLM fallback 时读取该 reference YAML

### 7. project-level SKILL 设计

建议新增一个 project-level SKILL，例如：

- `generate-reference-yaml`

其职责非常单一：

- 接收用户给定的 `gamever/module/platform/func_name`
- 调用 `uv run generate_reference_yaml.py ...`
- 不直接与 IDA API 或 MCP 对话

这样可保证：

- 命令行与 SKILL 共用一条后端逻辑
- 避免 shell 路径与 prompt 路径分叉

### 8. 首个目标场景

首版应优先支持：

- `CNetworkGameClient_RecordEntityBandwidth`

因为它正是当前 `CNetworkMessages_FindNetworkGroup` 的 reference 来源。

示例输出目标：

```text
ida_preprocessor_scripts/references/engine/CNetworkGameClient_RecordEntityBandwidth.windows.yaml
ida_preprocessor_scripts/references/engine/CNetworkGameClient_RecordEntityBandwidth.linux.yaml
```

### 9. reference YAML 准备步骤

面向最终用户，推荐准备步骤如下：

1. 先确认目标函数已有当前版本 YAML，或可通过 `config.yaml` alias 在 IDA 中搜索到
2. 运行独立 CLI，输入：
   - `gamever`
   - `module`
   - `platform`
   - `func_name`
3. CLI 导出 reference YAML 到：
   - `ida_preprocessor_scripts/references/<module>/<func_name>.<platform>.yaml`
4. 在对应 `find-*.py` 脚本中，把该路径写入 `LLM_DECOMPILE`
5. 人工检查该 YAML：
   - `func_name` 是否正确
   - `func_va` 是否可信
   - `disasm_code` 是否非空
   - `procedure` 是否符合预期
6. 再执行实际的 LLM fallback 流程

### 10. 失败与降级策略

#### 10.1 现有 YAML 不存在

- 不直接失败
- 转入 `config.yaml + IDA 搜索`

#### 10.2 `config.yaml` 找不到对应 symbol

- 失败
- 输出明确错误

#### 10.3 IDA 搜索结果为空

- 失败
- 输出“无法定位函数地址”

#### 10.4 IDA 搜索结果多于一个

- 失败
- 输出候选列表

#### 10.5 反汇编导出成功、伪代码导出失败

- reference YAML 仍可写出
- `procedure` 写空字符串

#### 10.6 自动启动 MCP 失败

- 返回非零
- 不写出半成品 YAML

### 11. 测试与验证

首版建议覆盖以下层次：

#### 单元测试

- YAML 路径生成
- 现有 YAML 优先解析 `func_va`
- `config.yaml` alias 收集
- 歧义 / 缺失错误处理

#### 集成级测试

- mock MCP 会话，验证导出 payload
- 验证最小 schema 落盘
- 验证 auto-start 与 attach 两种模式的参数分流

#### 手动验证

以 `CNetworkGameClient_RecordEntityBandwidth` 为例：

1. 连接实际 IDA 会话
2. 运行 CLI
3. 检查生成文件是否位于：
   - `ida_preprocessor_scripts/references/engine/CNetworkGameClient_RecordEntityBandwidth.windows.yaml`
4. 打开 YAML，检查反汇编与伪代码是否可信

## 建议落地文件

首版建议至少涉及：

- Create: `generate_reference_yaml.py`
- Create: `.claude/skills/generate-reference-yaml/SKILL.md`
- Modify: `README.md`
- Modify: `README_CN.md`
- Modify: `tests/...`（按最终实现拆分）

如为降低首版复杂度，也可先不新增 SKILL，先完成 CLI 与测试，再在第二步补 SKILL；但从用户目标看，CLI 与 SKILL 最终都应具备。

## 结论

本需求适合做成一个独立 CLI，并由 project-level SKILL 统一触发。

首版采用“通用框架 + `func` 首版实现”，既能尽快覆盖 `CNetworkGameClient_RecordEntityBandwidth` 的 reference YAML 自动化生成，又不会把后续 `vfunc` / 其他 reference 类型的扩展路径堵死。
