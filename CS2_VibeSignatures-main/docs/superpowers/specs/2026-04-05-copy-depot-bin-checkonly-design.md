# copy_depot_bin `-checkonly` 与自托管构建预检查设计

## 背景

当前自托管工作流会在进入 IDA 分析前固定执行以下步骤：

1. 更新 CS2 depot
2. 从 depot 拷贝二进制到 `bin/<gamever>/...`
3. 执行 `ida_analyze_bin.py`

由于 `bin/` 已持久化到 runner 外部目录，某些 tag 构建时目标二进制可能已经存在。此时继续下载 depot 会增加构建耗时，也会引入不必要的外部依赖。

本设计的目标是在不改变现有默认复制行为的前提下，为 `copy_depot_bin.py` 增加一个只检查目标文件是否齐全的模式，并让 GitHub Actions 在缓存完整时直接跳过下载阶段，进入 IDA 分析阶段。

## 目标

- 为 `copy_depot_bin.py` 增加 `-checkonly` 参数。
- 当 `-checkonly` 存在时，只检查目标 `bin/` 目录中的期望文件是否已存在。
- 仅当本次配置与平台下的所有期望目标文件都存在时，脚本返回 `0`。
- 若任一目标文件缺失，脚本返回 `1`。
- 若发生真实异常，如参数错误、配置文件不存在或无法解析，脚本返回非 `0/1` 的错误码，工作流直接失败。
- 更新 `.github/workflows/build-on-self-runner.yml`，在下载 depot 前先执行检查；若缓存完整，则跳过下载与复制步骤。

## 非目标

- 不改变 `copy_depot_bin.py` 的默认复制行为。
- 不修改 `config.yaml` 的结构与语义。
- 不新增独立检查脚本。
- 不在本次设计中扩展到更复杂的缓存命中策略，例如部分命中后按缺失模块增量下载。

## 方案对比

### 方案一：在 `copy_depot_bin.py` 中新增 `-checkonly`

在现有脚本中复用模块解析与目标路径计算逻辑，增加只检查目标文件存在性的分支。工作流只消费退出码与日志。

优点：

- 目标路径规则只维护一处。
- 检查逻辑与复制逻辑天然保持一致。
- 工作流层不需要重新实现 `config.yaml` 解释逻辑。

缺点：

- 脚本内部需要显式区分“检查模式”和“复制模式”。

### 方案二：在 workflow 中使用 PowerShell 直接检查 `bin/`

在 `.github/workflows/build-on-self-runner.yml` 中直接根据配置或固定路径检查 DLL 是否存在，再决定是否下载。

优点：

- Python 脚本无需变更。

缺点：

- 路径规则会在 workflow 中重复实现。
- 后续 `config.yaml` 或目录规则变化时容易漂移。

### 方案三：新增独立预检查脚本

新增专门的 preflight 脚本供 workflow 调用。

优点：

- 表面职责单一。

缺点：

- 与 `copy_depot_bin.py` 重复解析配置与计算目标路径。
- 增加额外维护点。

### 结论

采用方案一。在 `copy_depot_bin.py` 中新增 `-checkonly`，并让 workflow 通过检查结果决定是否跳过下载。

## 脚本设计

### 参数行为

新增布尔参数：

- `-checkonly`

语义：

- 未指定时，保持当前复制行为。
- 指定时，只执行检查，不访问 depot 源文件，不创建目录，不复制文件。

### 期望目标文件的判定

脚本继续使用 `config.yaml` 中 `modules` 的定义。

对每个 module：

- 读取 `name`
- 根据 `-platform` 选择 `path_windows`、`path_linux`，或在 `all-platform` 下按现有规则枚举对应路径
- 对于当前平台未定义路径的 module，直接跳过，不算失败
- 从配置路径中提取目标文件名
- 计算目标路径为 `bin/<gamever>/<module_name>/<filename>`

只有这些“本次应处理”的目标文件才参与 `-checkonly` 判断。

### 复用逻辑

为避免“检查模式”和“复制模式”各自维护一套路径拼装逻辑，脚本应抽出共用的目标枚举逻辑，例如：

- 基于 module 与 platform 生成期望源路径与目标路径
- 复制模式消费源路径与目标路径
- 检查模式只消费目标路径

这样可以保证未来若 `config.yaml` 路径规则变化，只需维护一处。

### 退出码与日志

`-checkonly` 模式下：

- 所有期望目标文件都存在：退出 `0`
- 任一目标文件缺失：退出 `1`
- 参数错误、配置文件缺失、YAML 解析失败等真实异常：退出 `2`

日志要求：

- 对已存在的目标打印已命中信息
- 对缺失目标打印缺失信息
- 最终打印汇总，例如已存在数量与缺失数量
- 明确打印最终结果，区分“缓存完整”和“目标缺失”

真实异常仍沿用错误输出并终止执行，不应伪装成普通缓存未命中。

### 前置校验

在 `-checkonly` 模式下：

- 仍要求 `-config` 存在且可读取
- 不要求 `-depotdir` 存在，因为不访问 depot
- 不需要提前创建 `-bindir`

在普通复制模式下，保持当前前置校验与创建目录行为。

## Workflow 设计

目标文件：`.github/workflows/build-on-self-runner.yml`

### 调整后的步骤顺序

1. Prepare variables and validate environment configuration
2. Checkout repository
3. Create persisted workspace links
4. Check cached binaries
5. 条件执行 Update CS2 depot
6. 条件执行 Copy depot binaries
7. Analyze binaries
8. Update gamedata
9. Run C++ tests
10. Archive release payload
11. Create release

### 检查步骤

新增 `Check cached binaries` step：

- 执行 `uv run copy_depot_bin.py -gamever "$env:GAMEVER" -platform all-platform -checkonly`
- 显式捕获退出码
- 若退出码为 `0`，设置 step output，例如 `bin_ready=true`
- 若退出码为 `1`，设置 `bin_ready=false`
- 若退出码不是 `0` 或 `1`，应让 step 失败，从而终止 workflow

为实现这一点，workflow 中需要在 PowerShell 脚本里显式区分：

- 预期返回：`0` 表示缓存完整，`1` 表示缓存缺失
- 非预期异常：非 `0/1` 返回码

最终判断标准应保持简单：只有 `bin_ready=true` 才跳过下载。

### 条件步骤

将以下两个 step 改为条件执行：

- `Update CS2 depot`
- `Copy depot binaries`

条件为：

- 当 `Check cached binaries` 的输出 `bin_ready` 不为 `true` 时执行

当 `bin_ready=true` 时，直接进入 `Analyze binaries`。

## 数据流

### 缓存命中路径

1. workflow 调用 `copy_depot_bin.py -checkonly`
2. 脚本根据 `config.yaml` 与 `-platform all-platform` 计算所有期望目标
3. 所有目标都存在，脚本返回 `0`
4. workflow 设置 `bin_ready=true`
5. 跳过 depot 下载与复制
6. 直接执行 `ida_analyze_bin.py`

### 缓存未命中路径

1. workflow 调用 `copy_depot_bin.py -checkonly`
2. 存在至少一个目标缺失，脚本返回 `1`
3. workflow 设置 `bin_ready=false`
4. 执行 `download_depot.py`
5. 执行普通模式的 `copy_depot_bin.py`
6. 执行 `ida_analyze_bin.py`

### 异常路径

1. `copy_depot_bin.py -checkonly` 遇到配置错误、参数错误或执行异常
2. 检查 step 直接失败
3. workflow 终止，不进入下载与后续分析

## 错误处理

### 脚本侧

- `config.yaml` 不存在：报错并退出 `2`
- `config.yaml` 无法解析：报错并退出 `2`
- module 缺少 `name`：沿用当前跳过策略
- `-checkonly` 下目标缺失：作为预期分支返回 `1`
- 普通复制模式下源文件缺失：沿用当前失败统计与退出语义

### workflow 侧

- 仅当缓存完整时跳过下载
- 不将真实异常解释为“未命中缓存”
- 下载与复制步骤的失败语义保持不变

## 测试与验证

本次改动属于局部行为增强，采用 Level 0 定向验证。

建议验证场景：

1. 目标文件全部存在
   - 执行 `copy_depot_bin.py -checkonly`
   - 预期退出码为 `0`
2. 目标文件部分缺失
   - 执行 `copy_depot_bin.py -checkonly`
   - 预期退出码为 `1`
3. 配置文件不存在或参数错误
   - 执行 `copy_depot_bin.py -checkonly`
   - 预期脚本报错并退出 `2`
4. workflow 条件流转静态检查
   - 确认 `Check cached binaries` 的 output 被后续两个 step 正确引用
   - 确认 `Analyze binaries` 无条件继续执行

除非用户另行要求，本设计不要求在本阶段主动运行完整构建或完整 CI。

## 兼容性与影响

- 对现有命令行调用兼容：未传 `-checkonly` 时行为不变
- 对 `all-platform` 现有工作流兼容：继续使用当前路径规则
- 对持久化 `bin` 目录友好：可显著减少重复下载
- 若未来增加新的 module 或路径映射，只要配置与原复制逻辑一致，检查模式会自动继承

## 实施清单

1. 在 `copy_depot_bin.py` 增加 `-checkonly` 参数
2. 抽取目标文件枚举逻辑，供复制与检查共用
3. 在 `-checkonly` 分支实现目标存在性检查与退出码语义
4. 更新 `.github/workflows/build-on-self-runner.yml`，新增缓存检查 step 与条件执行
5. 视需要补充 README 中 `-checkonly` 用法

## 风险与权衡

- 若 workflow 对退出码处理不严谨，容易把真实异常误判为缓存未命中，因此必须保持异常直接失败
- 若检查逻辑与复制逻辑未共用路径规则，未来容易漂移，因此需要优先抽共用逻辑
- 若只检查部分核心 DLL，会降低实现复杂度，但会增加后续 IDA 阶段缺文件风险，因此本设计坚持检查所有期望目标文件

## 结论

本设计通过在 `copy_depot_bin.py` 中增加 `-checkonly`，把“目标二进制是否已齐全”的判定集中到现有脚本内部，再由 GitHub Actions 基于该结果决定是否跳过 depot 下载。这样能够在不改变默认复制行为的情况下，减少重复下载时间，并保持检查与复制规则的一致性。
