# Build On Self Runner 设计文档

## 背景

仓库目前没有 GitHub Actions workflow，需要新增一个运行在 self-hosted Windows runner 上的自动化流程，用于在推送特定格式的版本 tag 后，完成 CS2 depot 更新、二进制复制、符号分析、gamedata 生成、C++ 头文件校验/修复、产物打包和 GitHub Release 发布。

该流程必须只允许主仓库触发，且依赖 GitHub Actions 中名为 `win64` 的 environment 提供 `RUNNER_AGENT` variable 与 `PERSISTED_WORKSPACE` secret。

## 目标

- 在 [`.github/workflows/build-on-self-runner.yml`](d:/CS2_VibeSignatures/.github/workflows/build-on-self-runner.yml) 新增 workflow。
- 仅在推送以 `v` 开头且满足约定格式的 tag 时触发。
- 仅允许 `HLND2T/CS2_VibeSignatures` 或 `hzqst/CS2_VibeSignatures` 触发执行。
- 在 self-hosted Windows x64 runner 上串行执行既定命令链路。
- 使用 runner 上的持久目录保存 `cs2_depot` 与 `bin`，避免工作区回收后丢失。
- 将 `bin/{GAMEVER}`、`dist`、`hl2sdk_cs2` 打包为 `gamedata-{GAMEVER}.7z` 并发布到 Release。

## 非目标

- 不在本次设计中引入多 job 并行或 artifact 中转。
- 不增加 fork 仓库运行权限。
- 不自动清理 runner 上的已有真实目录或做破坏性删除。
- 不改变现有 Python 脚本的 CLI 形式。

## 输入与触发规则

### 事件触发

- 仅监听 `push.tags: ['v*']`
- 不使用 `pull_request` 触发

### tag 格式

workflow 仅接受以下形式：

- `v14141`
- `v14141a`
- `v14141a-7617088375292372759`

解析规则：

- 正则：`^v(?<gamever>\d+[a-z]?)(?:-(?<manifest>\d+))?$`
- `GAMEVER`：
  - `v14141` -> `14141`
  - `v14141a` -> `14141a`
  - `v14141a-7617088375292372759` -> `14141a`
- `MANIFESTID`：
  - 无横杠后缀时为空
  - 有横杠后缀时取其数字部分，如 `7617088375292372759`

不匹配上述格式时，workflow 直接失败退出。

### 仓库限制

job 级别增加仓库限制：

```yaml
if: github.repository == 'HLND2T/CS2_VibeSignatures' || github.repository == 'hzqst/CS2_VibeSignatures'
```

### GitHub Environment 约束

- job 设置 `environment: win64`
- `runs-on: [self-hosted, windows, x64]`
- `win64` environment 必须提供：
  - variable：`RUNNER_AGENT`
  - secret：`PERSISTED_WORKSPACE`

导出规则：

- `RUNNER_AGENT` 通过 `${{ vars.RUNNER_AGENT }}` 注入 job `env`
- `PERSISTED_WORKSPACE` 通过 `${{ secrets.PERSISTED_WORKSPACE }}` 注入 job `env`

若上述任一配置缺失，workflow 直接失败退出。

## 总体方案

采用单 job 串行执行方案，并将“解析/校验”和“命令执行”分离：

1. `checkout`
2. 通过 `environment: win64` 注入 `RUNNER_AGENT` 与 `PERSISTED_WORKSPACE`
3. 解析 tag，导出 `GAMEVER`、`MANIFESTID`、`WORKSPACE`
4. 校验 `PERSISTED_WORKSPACE` 与 `RUNNER_AGENT`
5. 将工作区 `cs2_depot`、`bin` 链接到持久目录
6. 更新 CS2 depot
7. 复制 Windows / Linux 二进制到 `bin/{GAMEVER}`
8. 运行 IDA 分析
9. 更新 gamedata
10. 运行 C++ 测试并按需修复头文件
11. 打包产物
12. 创建 GitHub Release 并上传压缩包

## 详细设计

### 1. Environment 注入与变量解析

job 级别先声明：

```yaml
environment: win64
env:
  RUNNER_AGENT: ${{ vars.RUNNER_AGENT }}
  PERSISTED_WORKSPACE: ${{ secrets.PERSISTED_WORKSPACE }}
```

随后使用 `pwsh` step 处理：

- 从 `github.ref_name` 提取 tag 名
- 用正则匹配并验证格式
- 校验 `RUNNER_AGENT` 与 `PERSISTED_WORKSPACE` 已通过 job `env` 注入
- 向 `GITHUB_ENV` 写入：
  - `GAMEVER`
  - `MANIFESTID`
  - `WORKSPACE`

其中：

- `RUNNER_AGENT` 直接复用 job `env` 中的 `${{ vars.RUNNER_AGENT }}`
- `PERSISTED_WORKSPACE` 直接复用 job `env` 中的 `${{ secrets.PERSISTED_WORKSPACE }}`
- `WORKSPACE` 统一设置为 `${{ github.workspace }}`
- `MANIFESTID` 未提供时写入空字符串

选择 PowerShell 的原因是：

- 正则解析更清晰
- 向 `GITHUB_ENV` 导出 tag 解析结果更直接
- 失败时可通过 `throw` 明确终止 job

### 2. 持久目录链接

使用 `shell: cmd` 执行 runner 本地目录准备逻辑，并使用 directory junction 而不是 directory symlink。

目标行为：

- 若 `%PERSISTED_WORKSPACE%\\cs2_depot` 不存在，则创建
- 若 `%PERSISTED_WORKSPACE%\\bin` 不存在，则创建
- 将工作区中的：
  - `%WORKSPACE%cs2_depot`
  - `%WORKSPACE%bin`
  创建为指向持久目录的 directory junction

参考逻辑：

```cmd
if not exist "%PERSISTED_WORKSPACE%/cs2_depot" mkdir "%PERSISTED_WORKSPACE%/cs2_depot"
mklink /j "%WORKSPACE%cs2_depot" "%PERSISTED_WORKSPACE%/cs2_depot"

if not exist "%PERSISTED_WORKSPACE%/bin" mkdir "%PERSISTED_WORKSPACE%/bin"
mklink /j "%WORKSPACE%bin" "%PERSISTED_WORKSPACE%/bin"
```

为避免重复运行时报错，实际实现会加入保护：

- 若工作区目标不存在，则创建 junction
- 若工作区目标已存在且是 junction / symlink 且目标正确，则跳过
- 若工作区目标已存在但不是 junction / symlink，则直接失败并提示人工处理

选择 junction 的原因是：

- `mklink /d` 在不少 self-hosted runner 账号下需要额外符号链接权限
- `mklink /j` 更适合本场景的本地持久目录映射，可规避 `You do not have sufficient privilege to perform this operation.` 这一类权限错误
- 本设计不主动删除已有目录，以避免误删 self-hosted runner 上的真实数据。

### 3. Depot 更新

命令基线：

```cmd
DepotDownloader -app 730 -os all-platform -dir "%GITHUB_WORKSPACE%\\cs2_depot"
```

若 `MANIFESTID` 非空，则追加：

```cmd
-manifest %MANIFESTID%
```

设计理由：

- 与 README 中现有用法一致
- `all-platform` 满足同时下载 Windows 与 Linux 版本需求
- 下载路径固定在工作区链接目录，底层实际落到持久盘

### 4. 二进制复制

执行：

```cmd
uv run copy_depot_bin.py -gamever %GAMEVER% -platform all-platform
```

输出目标为 `bin/{GAMEVER}`，供后续分析与打包复用。

### 5. 分析与生成链路

按以下顺序执行：

```cmd
uv run ida_analyze_bin.py -gamever %GAMEVER% -agent=%RUNNER_AGENT% -debug
uv run update_gamedata.py -gamever %GAMEVER% -debug
uv run run_cpp_tests.py -gamever %GAMEVER% -fixheader -agent=%RUNNER_AGENT% -debug
```

说明：

- `RUNNER_AGENT` 来自 `win64` environment 的 `vars.RUNNER_AGENT`
- 不额外强制传 `-platform` 给 `ida_analyze_bin.py`，保持与仓库现有默认行为一致
- 任一步失败时，job 立即终止，不再继续打包与发布

### 6. 打包策略

打包文件名：

- `gamedata-{GAMEVER}.7z`

打包内容：

- `bin/{GAMEVER}`
- `dist`
- `hl2sdk_cs2`

排除项：

- `*.dll`
- `*.so`
- `*.i64`
- `*.id0`
- `*.id1`
- `*.id2`
- `*.nam`
- `*.til`
- `.git`
- `.git-blame-ignore-revs`
- `.gitmodules`

7z 参考形式：

```cmd
7z a gamedata-%GAMEVER%.7z "bin\\%GAMEVER%\\*" "dist\\*" "hl2sdk_cs2\\*" -r ^
-x!*.dll ^
-x!*.so ^
-x!*.i64 ^
-x!*.id0 ^
-x!*.id1 ^
-x!*.id2 ^
-x!*.nam ^
-x!*.til ^
-x!.git ^
-x!.git-blame-ignore-revs ^
-x!.gitmodules
```

### 7. Release 发布

使用：

```yaml
uses: softprops/action-gh-release@v1
```

配置建议：

- `name: gamedata-${{ github.ref_name }}`
- `files: gamedata-${{ env.GAMEVER }}.7z`

设计理由：

- Release 名保留完整 tag，有利于追溯 manifest 变体
- 压缩包名聚焦版本号，有利于消费方使用

### 8. 权限

workflow 显式声明：

```yaml
permissions:
  contents: write
```

不额外申请其他权限，保持最小授权。

## Shell 约定

- `pwsh`：只用于 tag 解析、环境变量校验和导出
- `cmd`：用于 `mklink`、`DepotDownloader`、`uv run ...`、`7z` 等命令执行

这样做的原因：

- PowerShell 适合正则与环境导出
- `cmd` 更贴近仓库 README、你给出的命令格式以及 Windows runner 上的常见工具调用方式

## 错误处理

以下情况必须立即失败：

- tag 不匹配约定格式
- 当前仓库不是主仓库
- `PERSISTED_WORKSPACE` secret 缺失
- `RUNNER_AGENT` variable 缺失
- 工作区 `bin` 或 `cs2_depot` 路径已存在但不是 junction / symlink
- `DepotDownloader` 执行失败
- 任一 `uv run ...` 步骤失败
- 7z 打包失败
- Release 上传失败

## 验收标准

当推送以下 tag 时，workflow 应能正确解析并进入构建：

- `v14141` -> `GAMEVER=14141`, `MANIFESTID=`
- `v14141a` -> `GAMEVER=14141a`, `MANIFESTID=`
- `v14141a-7617088375292372759` -> `GAMEVER=14141a`, `MANIFESTID=7617088375292372759`

当 tag 格式非法时，workflow 应在前置解析阶段直接失败。

当运行成功时，应满足：

- depot 更新完成
- `bin/{GAMEVER}` 已生成
- `dist` 已更新
- C++ 测试链路已执行
- 输出 `gamedata-{GAMEVER}.7z`
- Release 中包含该压缩包

## 实现备注

- 本次实现只新增 workflow，不修改现有 Python 脚本。
- 若未来需要把 Release 与 Build 拆分，可在此设计基础上演进为双 job。
