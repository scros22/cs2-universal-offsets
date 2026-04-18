# Download Depot 设计文档

## 背景

当前 `.github/workflows/build-on-self-runner.yml` 通过解析 Git tag 提取 `GAMEVER`，并可选从 tag 中拆出 `MANIFESTID`。这种方式把下载规则分散在 tag 命名和 workflow 逻辑之间，维护成本高，也不利于按配置集中管理不同版本的下载清单。

仓库已经新增 `download.yaml`，其中维护了 tag 与 depot manifests 的映射。后续需要把 manifest 下载决策完全收敛到该文件：workflow 不再解析或拆分 tag，也不再从 tag 中推导 manifest id，而是直接用完整 tag 去 `download.yaml` 里做精确匹配。匹配不到则任务失败。

## 目标

- 新增 `download_depot.py`，负责读取 `download.yaml` 并执行 depot 下载。
- 将 `download.yaml` 作为 manifest 的唯一来源。
- 让 `.github/workflows/build-on-self-runner.yml` 不再解析或拆分 tag。
- 让 `GAMEVER` 直接等于 Git tag 名称。
- 让 `download.yaml` 的 `downloads[].tag` 与 Git tag 完全一致。
- 当 tag 在 `download.yaml` 中匹配不到时，下载步骤直接失败。

## 非目标

- 不修改 `copy_depot_bin.py`、`ida_analyze_bin.py`、`update_gamedata.py`、`run_cpp_tests.py` 的 CLI 形式。
- 不限制 tag 的字符模式；tag 是否可用只由 `download.yaml` 是否存在唯一匹配决定。
- 不引入从远程接口动态拉取 manifest 的逻辑。
- 不要求 `manifests` 中固定包含某几个 depot；存在几个就下载几个。

## 输入与规则

### tag 规则

- GitHub tag 不再带固定前缀要求，也不做格式校验。
- tag 可以包含字母、数字、`-`、`_`，也允许其他 GitHub tag 可接受字符；workflow 不对其模式做额外限制。
- `download.yaml` 中的 `downloads[].tag` 必须与 `github.ref_name` 完全一致。
- workflow 不再做 tag 拆分、正则提取或前缀裁剪。

### GAMEVER 规则

- 传给后续脚本的 `GAMEVER` 直接等于 `github.ref_name`。
- 例如：
  - `14141` -> `14141`
  - `14141b` -> `14141b`
  - `release_14141b` -> `release_14141b`
- `GAMEVER` 不再做格式转换。

### download.yaml 规则

期望结构保持如下语义：

```yaml
downloads:
  - tag: 14141b
    name: 1.41.4.1
    manifests:
      "2347771": "2367650111076067440"
      "2347773": "5170166536177825328"
```

约束如下：

- 根节点必须存在 `downloads`
- `downloads` 必须是列表
- 每一项必须有唯一的 `tag`
- 匹配到的项必须包含 `manifests`
- `manifests` 必须是映射；其中有几个条目就下载几个条目

## 总体方案

采用“workflow 负责环境准备与变量透传，Python helper 负责下载决策与执行”的分层方案：

1. workflow 监听 tag push 事件
2. workflow 直接把 `github.ref_name` 写入 `TAG` 与 `GAMEVER`
3. workflow 调用 `uv run download_depot.py -tag <完整tag> -depotdir <目录> -config download.yaml`
4. `download_depot.py` 读取 `download.yaml`
5. 按完整 tag 精确匹配 `downloads[].tag`
6. 匹配成功后，遍历该项的 `manifests`
7. 对每个 `depot -> manifest` 执行一次 `DepotDownloader`
8. 任一步失败即返回非 0，阻止后续工作流继续执行

这样可以把“什么 tag 下载什么 manifest”的规则统一收口到 `download.yaml`，而 workflow 只负责调用。

## 详细设计

### 1. `download_depot.py` 的职责

新增根目录脚本 `download_depot.py`，仅负责：

- 解析命令行参数
- 读取并校验 `download.yaml`
- 依据完整 tag 查找唯一匹配项
- 遍历 manifests 并调用 `DepotDownloader`
- 输出清晰的成功/失败信息

该脚本不负责：

- 写入 `GITHUB_ENV`
- 推导或转换 `GAMEVER`
- 执行二进制复制、分析、打包或发布

保持单一职责后，脚本既可在 GitHub Actions 中调用，也可在本地独立复现下载行为。

### 2. `download_depot.py` CLI 设计

建议参数如下：

- `-tag`：必填，完整 Git tag，例如 `14141b`、`release_14141b`
- `-depotdir`：可选，下载目录，默认 `cs2_depot`
- `-config`：可选，配置文件路径，默认 `download.yaml`
- `-app`：可选，默认 `730`
- `-os`：可选，默认 `all-platform`

示例：

```bash
uv run download_depot.py -tag 14141b -depotdir cs2_depot -config download.yaml
```

### 3. `download_depot.py` 解析与校验逻辑

脚本启动后按以下顺序执行：

1. 校验 `-config` 文件存在
2. 使用 `yaml.safe_load` 读取 YAML
3. 校验顶层 `downloads` 是否为列表
4. 查找 `downloads` 中 `tag == -tag` 的所有项
5. 若匹配数为 0，报错并退出
6. 若匹配数大于 1，报错并退出，避免歧义
7. 取唯一匹配项，校验 `manifests` 是否为映射
8. 遍历 `manifests` 中每个 `depot -> manifest`

设计选择：

- 使用“精确匹配”而不是模糊匹配或前缀匹配
- 不对 tag 的内容做语义判断，只以配置存在性为准
- 对重复 tag 直接失败，避免 silently picking first
- 不强制校验固定 depot 集合，遵循“有几个下几个”

### 4. `download_depot.py` 下载逻辑

对每个 `depot -> manifest` 生成并执行命令：

```cmd
DepotDownloader -app 730 -depot <depot> -os all-platform -dir <depotdir> -manifest <manifest>
```

其中：

- `<depot>` 来自 `manifests` 的 key
- `<manifest>` 来自 `manifests` 的 value
- `<depotdir>` 来自 `-depotdir`
- `730` 与 `all-platform` 提供默认值，但允许通过参数覆盖

脚本行为：

- 下载前打印匹配到的 tag、版本名（如存在）、待下载条目数
- 每执行一个 depot 下载前打印完整参数摘要
- 任意一个 `DepotDownloader` 返回非 0，则脚本直接退出非 0
- 所有下载成功后打印汇总

### 5. workflow 调整

目标文件：`.github/workflows/build-on-self-runner.yml`

调整点如下：

- tag 触发条件改为监听所有 tag push：
  - `push.tags: ['*']`
- 保留环境校验：
  - `PERSISTED_WORKSPACE`
  - `RUNNER_AGENT`
- 删除 `MANIFESTID` 相关解析、导出和使用逻辑
- 删除 tag 格式正则校验
- 将变量导出步骤改为仅做：
  - 读取 `github.ref_name`
  - 导出 `TAG`
  - 导出 `GAMEVER`
  - 导出 `WORKSPACE`
- 其中：
  - `TAG = github.ref_name`
  - `GAMEVER = github.ref_name`
- 将 “Update CS2 depot” 步骤改为调用：

```cmd
uv run download_depot.py -tag %TAG% -depotdir "%GITHUB_WORKSPACE%\cs2_depot" -config download.yaml
```

后续步骤保持：

```cmd
uv run copy_depot_bin.py -gamever %GAMEVER% -platform all-platform
uv run ida_analyze_bin.py -gamever %GAMEVER% -agent=%RUNNER_AGENT% -debug
uv run update_gamedata.py -gamever %GAMEVER% -debug
uv run run_cpp_tests.py -gamever %GAMEVER% -fixheader -agent=%RUNNER_AGENT% -debug
```

### 6. `download.yaml` 调整

现有 `download.yaml` 需要把 `downloads[].tag` 改成与 Git tag 完全一致的值，不再带额外转换逻辑。

例如：

- `14141`
- `14141b`
- `release_14141b`

原则上，只要某个 tag 会被用于触发 workflow，就应在 `download.yaml` 中存在一条同名记录；否则下载步骤将直接失败。

## 错误处理

### 失败场景

以下场景直接失败：

- `download.yaml` 不存在
- `download.yaml` YAML 格式非法
- 顶层 `downloads` 缺失或类型错误
- 指定 tag 没有匹配项
- 指定 tag 有多个匹配项
- 匹配项缺少 `manifests`
- `manifests` 类型不是映射
- 任意一次 `DepotDownloader` 执行失败

### 非失败场景

以下情况不应直接失败：

- tag 包含 `-` 或 `_`
- tag 不符合某种“版本号”样式
- 匹配项中只声明了一个 depot manifest
- 匹配项没有 `name`
- `manifests` 中的 depot 数量与历史版本不同

## 测试与验收

### 定向验证

建议围绕 helper 脚本与 workflow 静态修改做最小验证：

1. 用存在的 tag 调用脚本，验证能正确匹配并生成下载行为
2. 用不存在的 tag 调用脚本，验证脚本直接失败
3. 检查 workflow 中已不存在 `MANIFESTID` 解析与分支逻辑
4. 检查 workflow 已改为调用 `download_depot.py`
5. 检查 workflow 已改为 `push.tags: ['*']`
6. 检查 `download.yaml` 中 tag 已与真实 Git tag 命名一致

### 验收标准

- `download.yaml` 成为 manifest 的唯一来源
- workflow 不再从 tag 中解析 manifest id
- workflow 不再限制 tag 格式
- 完整 tag 可在 `download.yaml` 中精确匹配
- 匹配不到时，任务直接失败
- 匹配成功时，仅下载 `manifests` 中声明的 depot
- 后续步骤收到的 `GAMEVER` 与 tag 完全一致

## 风险与权衡

- 相比把逻辑直接写进 workflow，新增脚本会多一个维护点，但换来了更清晰的职责划分和更好的本地可调试性
- tag 不再做格式限制后，系统灵活性更高，但配置纪律更依赖 `download.yaml` 的准确性
- `download.yaml` 现在成为唯一真相源，配置错误会更集中暴露；这属于可接受风险，因为失败会比 silent fallback 更容易被发现
- 不对 depot 列表做硬编码校验，灵活性更高，但也意味着配置审核需要更谨慎

## 实施摘要

本次改动应以最小边界完成三件事：

1. 新增 `download_depot.py`
2. 修改 `.github/workflows/build-on-self-runner.yml`，移除 tag 解析逻辑并改为调用 helper
3. 修改 `download.yaml`，让 `downloads[].tag` 与真实 Git tag 完全一致
