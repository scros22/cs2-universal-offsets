# 为 `vcall_finder` 设计可复用的 IDA 端直接落盘大结果导出方案

## 背景

当前 `ida_vcall_finder.py` 的 `export_object_xref_details_via_mcp()` 采用两段式导出：

1. 先通过 `py_eval` 定位对象及其引用函数列表。
2. 再对每个引用函数通过 `py_eval` 返回完整反汇编与伪代码，由客户端本地写入 detail YAML。

该方案在普通函数上可工作，但在超大函数上会失败。以 `networksystem` 中的 `CNetworkSystem_SendNetworkStats` 为例，函数 dump 结果可能达到二十多万字符。IDA MCP 服务端在响应过大时会截断 `structuredContent`，并注入 `_output_truncated`、`_download_url` 等额外字段；而 MCP Python SDK 会按工具声明的严格 `outputSchema` 做校验，最终在客户端抛出异常，导致 `vcall_finder` 无法继续导出 detail YAML。

当前故障链路的核心问题不是“本地不会写 YAML”，而是“大结果穿过 `py_eval` 返回通道时就已经失败”。

## 目标

- 为当前仓库抽出一个可复用的“IDA 端直接落盘大结果，客户端只接收小 ack”的通用模式。
- 仅修改当前仓库客户端逻辑，继续基于现有 `py_eval` 能力，不修改 `ida-pro-mcp`。
- 将 `vcall_finder` 的函数 detail 导出切换到该新模式，避免超大 `structuredContent` 校验失败。
- 保持下游 `aggregate_vcall_results_for_object()` 的读取契约不变。
- 保持现有缓存语义不变：已有 detail 文件继续跳过，聚合后继续回写 `found_vcall`。

## 非目标

- 不修改 `ida-pro-mcp` 的 `py_eval` 输出 schema。
- 不新增专用 MCP tool。
- 不自动回退到旧的“大 JSON 返回到客户端”模式。
- 不将仓库内所有 YAML 写入逻辑一次性统一重构。
- 不为本次设计增加路径白名单或仓库根目录限制。

## 问题根因

当前 `vcall_finder` 的第二段 `py_eval` 负责返回如下大对象：

- `func_name`
- `func_va`
- `disasm_code`
- `procedure`

其中 `disasm_code` 与 `procedure` 对大函数来说可能极大，超过 IDA MCP 的响应安全阈值。服务端截断响应后向 `structuredContent` 注入额外元数据字段，但这些字段又不在工具的严格 `outputSchema` 中，客户端 SDK 在 schema 校验阶段直接抛异常。于是：

- 客户端拿不到正常结果；
- `_parse_py_eval_json_payload()` 没有机会解析正文；
- detail YAML 根本不会写出。

因此，修复点应当前移到“不要让大正文穿过 `py_eval` 的返回通道”。

## 总体方案

采用“通用远端文本落盘协议 + `vcall_finder` 专用 detail 导出 builder”的最短路径方案。

### 方案概述

1. 在仓库中新增一个可复用的远端落盘模式：
   - 客户端向 `py_eval` 下发一段脚本；
   - 脚本在 IDA 端生成完整文本并直接写入目标绝对路径；
   - 脚本只返回小型 ack 结果。
2. `vcall_finder` 不再让 `py_eval` 返回完整 `disasm_code/procedure`；
   - 改为在 IDA 端直接生成 detail 文件；
   - 客户端只检查 ack 是否成功。
3. 下游聚合逻辑保持不变；
   - 仍从 detail 文件读取 `object_name/module/platform/func_name/func_va/disasm_code/procedure`；
   - 仍在聚合后回写 `found_vcall`。

## 通用远端落盘契约

### 设计落点

通用能力放在 `ida_analyze_util.py` 一侧，与现有 MCP/`py_eval` 辅助逻辑同层。`ida_vcall_finder.py` 仅负责本业务的 payload 构造与流程编排。

### 输入契约

调用方需要提供：

- `output_path`
  - 目标文件绝对路径。
  - 按本次决策，允许任意绝对路径，不做仓库内约束。
- `producer_code`
  - 在 IDA Python 环境中生成文件正文的代码。
- `context`
  - 用于 debug 日志和错误信息定位。

### 远端模板职责

通用远端模板负责以下固定行为：

1. 校验 `output_path` 必须为绝对路径。
2. 创建父目录。
3. 以 UTF-8 写临时文件。
4. 用 `os.replace()` 原子替换目标文件。
5. 捕获异常并返回小型 JSON ack。

### 返回契约

`py_eval` 仅返回一个很小的 mapping，例如：

```json
{
  "ok": true,
  "output_path": "/abs/path/to/file.yaml",
  "bytes_written": 12345,
  "format": "text"
}
```

失败时返回：

```json
{
  "ok": false,
  "output_path": "/abs/path/to/file.yaml",
  "error": "..."
}
```

可选返回精简后的 `traceback` 文本，但必须截短，避免再次变成大响应。

### 通用层职责边界

- 通用层只负责“写文本文件并返回 ack”。
- 通用层不负责业务 payload 的语义正确性。
- 通用层不承担完整 YAML 专用序列化器角色。

## `vcall_finder` 具体落地

### 改造范围

仅改造 `ida_vcall_finder.py` 中 `export_object_xref_details_via_mcp()` 的第二跳函数 dump 导出流程。

第一跳 object xref 查询仍保留现状，因为其返回 payload 很小，风险低。

### 新的 detail 导出流程

对每个引用函数：

1. 客户端继续用 `build_vcall_detail_path()` 计算 detail 文件路径。
2. 若 detail 文件已存在，沿用当前缓存语义直接跳过。
3. 否则调用新的 `build_function_dump_export_py_eval(func_va, output_path, ...)`。
4. 该 `py_eval` 脚本在 IDA 端：
   - 定位函数；
   - 生成完整 `disasm_code`；
   - 生成完整 `procedure`；
   - 组装 detail mapping；
   - 直接写入目标 detail 文件；
   - 返回小 ack。
5. 客户端只解析 ack：
   - 成功则记为 `exported_functions += 1`
   - 失败则记为 `failed_functions += 1`

### detail 文件格式

首次导出时，detail 文件直接由 IDA 端写为纯 YAML，格式与当前仓库本地写入风格保持一致：

```yaml
object_name: g_pNetworkMessages
module: networksystem
platform: linux
func_name: CNetworkSystem_SendNetworkStats
func_va: 0x3ea720
disasm_code: |-
  ...
procedure: |-
  ...
```

本次设计直接假定用户环境中的 IDA Python 侧已安装 `PyYAML`，因此远端脚本可以：

- 直接构造 detail mapping；
- 使用 `yaml.dump()` 或等价安全写法生成纯 YAML；
- 继续使用 literal block 形式写出 `disasm_code` 与 `procedure`，保证可读性。

这样做的目的在于：

- 保持 detail 文件首写结果就具备较好的人工可读性；
- 与当前仓库既有 YAML 读写风格保持一致；
- 避免“首次导出是 JSON，后续聚合回写后才变成 YAML”这种阶段性表示差异。

### 保持不变的下游契约

以下行为保持不变：

- `aggregate_vcall_results_for_object()` 仍从 detail 文件读取顶层 mapping。
- 若 detail 顶层存在 `found_vcall`，仍视为缓存命中。
- `found_vcall` 缺失时仍调用 LLM 聚合并回写。
- 对象级 summary 追加逻辑不变。

## 错误处理与边界行为

### 失败策略

新模式失败时不回退旧模式。原因如下：

- 旧模式本身就是当前已知爆点；
- 自动回退只会重新触发同类 schema 校验异常；
- 增加双路径回退会显著提高复杂度，却不能带来稳定收益。

因此，远端写 detail 失败时：

- debug 模式打印函数范围与错误信息；
- `failed_functions += 1`；
- 继续处理下一个函数。

### 原子写入

为避免产生半截 detail 文件，远端模板必须使用：

1. `tmp_path = output_path + ".tmp"`
2. 写入临时文件
3. `os.replace(tmp_path, output_path)`

若异常发生，应尽量清理临时文件，但清理失败不应覆盖主错误。

### 路径规则

本次明确允许任意绝对路径，因此：

- 客户端发送前应先将 `detail_path` 解析为绝对路径字符串。
- 远端模板若收到相对路径，直接返回失败。
- 不做仓库根目录、前缀白名单或路径映射。

同时需要明确一个运行时前提：

- 该路径必须对运行 IDA MCP 的进程环境真实可写。
- 若客户端与 IDA 所在环境对路径视图不一致，则失败是预期行为，本次不做自动映射或纠偏。

### ACK 大小控制

远端模板返回值应尽可能小：

- 成功时只返回布尔、路径、字节数、格式等小字段。
- 失败时 `error` 和 `traceback` 需要截短。

目标是从协议层保证 ack 永远不会成为新的超大响应。

## 兼容性分析

### 与现有读取逻辑兼容

`load_yaml_file()` 依赖 `yaml.safe_load()` 将文件读为顶层 dict。IDA 端直接写出的纯 YAML detail 文件天然满足这一契约，不会破坏聚合流程。

### 与现有缓存逻辑兼容

`detail_path.exists()` 的短路逻辑保持不变。已存在 detail 文件时，无论其内容是历史上本地写出的 YAML，还是新方案下由 IDA 端直写的 YAML，都直接复用。

### 与现有回写逻辑兼容

聚合后写回 `found_vcall` 时，文件仍会被当前本地 YAML dumper 重写，但由于首写阶段已经是纯 YAML，因此不存在阶段性格式切换问题。首写与回写都维持在同一种文件语义下。

## 实施顺序

1. 在 `ida_analyze_util.py` 增加通用远端文本落盘辅助逻辑。
2. 在 `ida_vcall_finder.py` 新增 `vcall_finder` 专用的函数 dump 导出 builder。
3. 修改 `export_object_xref_details_via_mcp()`，将第二段函数 dump 切到新模式。
4. 更新说明文档，明确推荐修复方案已从“客户端截断”调整为“IDA 端直接落盘”。

## 验证策略

### Level 0：定向验证

- 确认 `export_object_xref_details_via_mcp()` 不再依赖 `py_eval` 返回完整 `disasm_code/procedure`。
- 确认远端返回 ack 为小 mapping。
- 确认 detail 文件由 IDA 端首写为纯 YAML 后，`load_yaml_file()` 仍可正常读取。

### Level 1：场景回归

- 用一个普通规模函数验证 detail 文件成功导出并可被聚合阶段消费。
- 用一个超大函数复现原故障场景，确认不再因 `structuredContent` 附加字段导致客户端 schema 校验异常。

### 文档校验

- 更新 `docs/too_large_content_break_structuredContent.md`，使其与最终实现方向一致。
- 如有必要，补充 `README.md` 中 `vcall_finder` 的实现说明，但不改变 CLI 接口。

## 风险与权衡

### 风险

- 客户端与 IDA MCP 运行环境的路径视图可能不一致，导致绝对路径不可写。
- 用户环境若缺失 `PyYAML`，则远端纯 YAML 导出会直接失败。
- 远端脚本复杂度会高于纯返回字符串的做法。

### 权衡结论

- 路径可写性问题是此次“允许任意绝对路径”决策的自然代价，接受该约束。
- 本次明确接受 `PyYAML` 作为运行前提，以换取首写即为纯 YAML 的可读性与一致性。
- 相比客户端截断或协议回退，该方案更直接、稳定且可复用。

## 最终结论

本次采用“通用远端文本落盘协议 + `vcall_finder` 专用 detail 导出 builder”方案：

- 只改当前仓库客户端；
- 不让大函数正文穿过 `py_eval` 返回通道；
- 由 IDA 端直接写出 detail 文件；
- 客户端只接收小 ack；
- 保持下游聚合、缓存与 summary 逻辑不变。

该方案以最小行为改动解决当前超大函数导出失败问题，并为未来其他“大结果不适合走 `structuredContent`”的场景提供可复用模式。
