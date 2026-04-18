# CS2 VibeSignatures

[English README](README.md)

这是一个主要用于为 CS2 生成 signatures/offsets，并通过 Agent SKILLS 与 MCP Calls 更新 HL2SDK_CS2 C++ 头文件的项目。

该项目的设计目标是在**完全无需人工参与**的情况下更新 signatures/offsets/cpp headers。

目前，本项目已可自动更新 **CounterStrikeSharp** 和 **CS2Fixes** 的全部 signatures/offsets。

* 欢迎通过 PR 贡献你的 SKILL！[如何创建SKILL? 见这里](https://github.com/hzqst/CS2_VibeSignatures/issues?q=is%3Aissue%20state%3Aclosed) - `TODO: Add skill support for XXXXXXX`

## 依赖要求

1. 安装 [uv](https://docs.astral.sh/uv/getting-started/installation/)

2. [depotdownloader](https://github.com/steamre/depotdownloader)

3. `uv sync`

4. claude / codex

5. IDA Pro 9.0+

6. [ida-pro-mcp](https://github.com/mrexodia/ida-pro-mcp)

7. [idalib](https://docs.hex-rays.com/user-guide/idalib)（运行 `ida_analyze_bin.py` 的必需项）

8. Clang-LLVM（运行 `run_cpp_tests.py` 的必需项）

## 整体工作流

#### 1. 下载 CS2 二进制文件并复制dll/so到工作目录

```bash
DepotDownloader -app 730 -depot 2347771 -os all-platform -dir cs2_depot [-branch animgraph_2_beta]
DepotDownloader -app 730 -depot 2347773 -os all-platform -dir cs2_depot [-branch animgraph_2_beta]

uv run copy_depot_bin.py -gamever 14141 -platform all-platform
uv run copy_depot_bin.py -gamever 14141 -platform all-platform -checkonly
```

当只需要确认 `bin/<gamever>/...` 下的目标二进制是否已经齐全时，可在 CI 或预检查脚本中使用 `-checkonly`。该模式只检查目标路径，不要求 `cs2_depot` 已准备完成；当所有目标文件都已就绪时返回 `0`，缺少任一目标文件时返回 `1`，配置或参数错误时返回 `2`。


#### 2. 为 `config.yaml` 的符号生成对应的 signatures

 ```bash
 uv run ida_analyze_bin.py -gamever=14141 [-oldgamever=14140] [-configyaml=path/to/config.yaml] [-modules=server] [-platform=windows] [-agent=claude/codex/"claude.cmd"/"codex.cmd"] [-maxretry=3] [-vcall_finder=g_pNetworkMessages|*] [-llm_model=gpt-4o] [-llm_apikey=your-key] [-llm_baseurl=https://api.example.com/v1] [-debug]
 ```

* 在真正运行 Agent SKILL(s) 前，会先通过 mcp call 直接使用 `bin/{previous_gamever}/{module}/{symbol}.{platform}.yaml` 中的旧 signature 查找当前版本游戏二进制中的符号。不会消耗 token。

* `-agent="claude.cmd"` 用于Windows上使用npm安装的claude cli

* `-vcall_finder=g_pNetworkMessages` 会在模块级 `vcall_finder` 配置中筛选同名对象；`-vcall_finder=*` 会处理 `config.yaml` 中已声明的全部对象。

* 当启用 `-vcall_finder` 时，脚本会在每个模块/平台完成 IDA 任务后导出对象引用函数的完整反汇编与伪代码到 `vcall_finder/{gamever}/{object_name}/{module}/{platform}/`，并在全部模块/平台结束后调用 OpenAI SDK；若某个 detail YAML 已存在顶层 `found_vcall`，则会跳过该次 LLM 调用，直接复用缓存结果。

* LLM 成功返回后，会立刻将 `found_vcall: [...]` 或 `found_vcall: []` 回写到对应的 detail YAML，后续重跑可直接跳过该函数的 LLM 调用。

* `vcall_finder/{gamever}/{object_name}.txt` 现在是按 YAML document stream 追加的扁平记录；每条记录直接包含 `insn_va`、`insn_disasm`、`vfunc_offset`，不再嵌套 `found_vcall:`。

* 共享 LLM CLI 参数：
  - `-llm_apikey`：启用基于 LLM 的流程时必需，包括 `vcall_finder` 聚合与 `LLM_DECOMPILE`
  - `-llm_baseurl`：可选，自定义兼容 base URL
  - `-llm_model`：可选，默认 `gpt-4o`
  - LLM 流程不会读取 `OPENAI_API_KEY`、`OPENAI_API_BASE`、`OPENAI_API_MODEL`

```bash
uv run ida_analyze_bin.py -gamever=14141 -modules=networksystem -platform=windows -vcall_finder=g_pNetworkMessages -llm_model=gpt-4o -llm_apikey=your-key
uv run ida_analyze_bin.py -gamever=14141 -platform=windows,linux -vcall_finder=* -llm_model=gpt-4o -llm_apikey=your-key -llm_baseurl=https://api.example.com/v1
```

输出示例：

- `vcall_finder/14141/g_pNetworkMessages/networksystem/windows/sub_140123450.yaml`
- `vcall_finder/14141/g_pNetworkMessages.txt`

#### 2.5 准备 `LLM_DECOMPILE` 的 reference YAML

reference YAML 存放路径：

- `ida_preprocessor_scripts/references/<module>/<func_name>.<platform>.yaml`

准备步骤：

1. 确认目标函数已有当前版本 YAML 且包含 `func_va`，或可通过 `config.yaml` 的 symbol name/alias 在 IDA 中定位。
2. 运行独立 CLI：

```bash
uv run generate_reference_yaml.py -gamever 14141 CNetworkGameClient_RecordEntityBandwidth -mcp_host 127.0.0.1 -mcp_port 13337
```

自动启动 `idalib-mcp` 示例：

```bash
uv run generate_reference_yaml.py -gamever 14141 -module engine -platform windows -func_name CNetworkGameClient_RecordEntityBandwidth -auto_start_mcp -binary "bin/14141/engine/engine2.dll"
```

3. 检查生成文件：
   - `func_va` 可信
   - `disasm_code` 非空，且与目标函数语义匹配
   - `procedure` 在可用时应与预期语义一致（Hex-Rays 不可用时允许为空字符串）
   - `func_name` 仅用于确认输出文件对应你请求的规范名，不能单独证明地址解析正确
4. 在目标 `find-*.py` 脚本里接入 `LLM_DECOMPILE`：
   - 生成文件在仓库中的路径：
     - `ida_preprocessor_scripts/references/<module>/<func_name>.<platform>.yaml`
   - 若 `LLM_DECOMPILE` 使用相对路径，应写成：
     - `references/<module>/<func_name>.<platform>.yaml`
   - tuple 示例：
     - `("CNetworkMessages_FindNetworkGroup", "prompt/call_llm_decompile.md", "references/engine/CNetworkGameClient_RecordEntityBandwidth.windows.yaml")`
   - `LLM_DECOMPILE` 复用 `ida_analyze_bin.py` 的共享 LLM 参数：`-llm_model`、`-llm_apikey`、`-llm_baseurl`

#### 3. 将 yaml(s) 转换为 gamedata json / txt

```bash
uv run update_gamedata.py -gamever 14141 [-debug]
```

#### 4. 运行 C++ 测试并检查 cpp headers 是否与 yaml(s) 匹配

```bash
uv run run_cpp_tests.py -gamever 14141 [-debug] [-fixheader] [-agent=claude/codex/"claude.cmd"/"codex.cmd"] 
```

* 使用 `-fixheader` 时，会启动一个 agent 来修复 cpp headers 中的不匹配项（会消耗少量token）

### 当前支持的 gamedata

[CounterStrikeSharp](https://github.com/roflmuffin/CounterStrikeSharp)

`dist/CounterStrikeSharp/config/addons/counterstrikesharp/gamedata/gamedata.json`

 - 已跳过 2 个符号。

 - `GameEventManager`：在CSS中已废弃。
 - `CEntityResourceManifest_AddResource`：游戏更新时基本不会改动。

[CS2Fixes](https://github.com/Source2ZE/CS2Fixes)

`dist/CS2Fixes/gamedata/cs2fixes.games.txt`

 - 已跳过 1 个符号。

 - `CCSPlayerPawn_GetMaxSpeed`，因为它并不存在于 `server.dll` 中。

[swiftlys2](https://github.com/swiftly-solution/swiftlys2)

`dist/swiftlys2/plugin_files/gamedata/cs2/core/offsets.jsonc`

`dist/swiftlys2/plugin_files/gamedata/cs2/core/signatures.jsonc`

 - 已跳过 44 个符号。

[plugify](https://github.com/untrustedmodders/plugify-plugin-s2sdk)

`dist/plugify-plugin-s2sdk/assets/gamedata.jsonc`

 - 已跳过 14 个符号。

[cs2kz-metamod](https://github.com/Source2ZE/CS2Fixes)

`dist/cs2kz-metamod/gamedata/cs2kz-core.games.txt`

 - 已跳过 42 个符号。

[modsharp](https://github.com/Kxnrl/modsharp-public)

`dist/modsharp-public/.asset/gamedata/core.games.jsonc`

`dist/modsharp-public/.asset/gamedata/engine.games.jsonc`

`dist/modsharp-public/.asset/gamedata/EntityEnhancement.games.jsonc`

`dist/modsharp-public/.asset/gamedata/log.games.jsonc`

`dist/modsharp-public/.asset/gamedata/server.games.jsonc`

`dist/modsharp-public/.asset/gamedata/tier0.games.jsonc`

 - 已跳过 230 个符号。

[CS2Surf/Timer](https://github.com/CS2Surf-CN/Timer)

`dist/cs2surf/gamedata/cs2surf-core.games.jsonc`

 - 已跳过 26 个符号。

## 如何为 vtable 创建 SKILL

以 `CCSPlayerPawn` 为例。

#### 1. 创建预处理脚本

 - 创建 `ida_preprocessor_scripts/find-CCSPlayerPawn_vtable.py`

 - **务必**检查已有包含 `TARGET_CLASS_NAMES` 的预处理脚本作为参考。

 - 查找 vtable 不需要 LLM，全部逻辑都应在预处理脚本中完成。

#### 2. 在 `config.yaml` 的 `skills` 下添加新 SKILL

 * 显式声明 `expected_output` 和 `expected_input`（可选）。

```yaml
      - name: find-CCSPlayerPawn_vtable
        expected_output:
          - CCSPlayerPawn_vtable.{platform}.yaml
```

#### 3. 在 `config.yaml` 的 `symbols` 下添加新符号

```yaml
      - name: CCSPlayerPawn_vtable
        category: vtable
```

## 如何为普通函数创建 SKILL

* 务必确保 ida-pro-mcp server 正在运行。

* 对于人类贡献者：当你查找新符号时，应编写新的初始提示词，**不要**从 README 直接复制粘贴！

以 `CBaseModelEntity_SetModel` 为例

#### 1. 在 IDA 中查找目标符号

  - 在 IDA 中搜索字符串 `"weapons/models/defuser/defuser.vmdl"`，在其 xrefs 里找如下模式的代码片段：

```c
    v2 = a2;
    v3 = (__int64)a1;
    sub_180XXXXXX(a1, (__int64)"weapons/models/defuser/defuser.vmdl"); //This is CBaseModelEntity_SetModel, rename it to CBaseModelEntity_SetModel
    sub_180YYYYYY(v3, v2);
    v4 = (_DWORD *)sub_180ZZZZZZ(&unk_181AAAAAA, 0xFFFFFFFFi64);
    if ( !v4 )
      v4 = *(_DWORD **)(qword_181BBBBBB + 8);
    if ( *v4 == 1 )
    {
      v5 = (__int64 *)(*(__int64 (__fastcall **)(__int64, const char *, _QWORD, _QWORD))(*(_QWORD *)qword_181CCCCCC + 48i64))(
                        qword_181CCCCCC,
                        "defuser_dropped",
                        0i64,
                        0i64);
```

#### 2. 创建 SKILL

  - 根据你在 IDA 里的分析，创建项目级 skill `find-CBaseModelEntity_SetModel`（**使用英文编写**）。

  - 该 SKILL 应生成 `CBaseModelEntity_SetModel.{platform}.yaml`，并包含 `func_sig`。

  - 该 SKILL 需要同时支持 `server.dll` 和 `libserver.so`。

  - 不要打包 skill。

  - **务必**检查已有使用 `/write-func-as-yaml` 调用的 SKILL 作为参考。

#### 3. 创建预处理脚本

  - 创建 `ida_preprocessor_scripts/find-CBaseModelEntity_SetModel.py`

  - **务必**检查已有包含 `TARGET_FUNCTION_NAMES` 的预处理脚本作为参考。

#### 4. 在 `config.yaml` 的 `skills` 下添加新 SKILL

 * 显式声明 `expected_output` 和 `expected_input`（可选）。

```yaml
      - name: find-CBaseModelEntity_SetModel
        expected_output:
          - CBaseModelEntity_SetModel.{platform}.yaml
```

5. 在 `config.yaml` 的 `symbols` 下添加新符号。

```yaml
      - name: CBaseModelEntity_SetModel
        catagoty: func
        alias:
          - CBaseModelEntity::SetModel
```

## 如何为虚函数创建 SKILL

* 务必确保 ida-pro-mcp server 正在运行。

* 对于人类贡献者：当你查找新符号时，应编写新的初始提示词，**不要**从 README 直接复制粘贴！

以 `CBasePlayerController_Respawn` 为例

#### 1. 在 IDA 中查找目标符号

  - 在 IDA 中搜索字符串 `"GMR_BeginRound"`，找到引用它的函数，反编译该函数并查找如下模式：

```c
      do
      {
        //.......
        if ( v31 )
        {
          if ( (*(unsigned __int8 (__fastcall **)(__int64))(*(_QWORD *)v31 + 3352LL))(v31) )
          {
            (*(void (__fastcall **)(__int64))(*(_QWORD *)v33 + 3368LL))(v33);
            if ( v36 )
            {
              sub_1801C86D0(v36);
              sub_18039EA00(v36, 32LL);
            }
          }
          else if ( v36 && *(_BYTE *)(v30 + 836) == 3 || *(_BYTE *)(v30 + 836) == 2 )
          {
            sub_1809F9670(v36);
            (*(void (__fastcall **)(__int64))(*(_QWORD *)v30 + 2176LL))(v30); // 2176LL is vfunc_offset for CBasePlayerController_Respawn
          }
        }
        ++v28;
      }
      while ( v28 != v29 );
```

#### 2. 创建 SKILL

  - 创建项目级 skill `find-CBasePlayerController_Respawn`（**使用英文编写**）。

  - 该 SKILL 应生成 `CBasePlayerController_Respawn.{platform}.yaml`，并包含 `func_sig`。（如果 `CBasePlayerController_Respawn` 太短或过于通用，可改用 `vfunc_sig`）。

  - 该 SKILL 需要同时支持 `server.dll` 和 `libserver.so`。

  - 不要打包 skill。

  - **务必**检查已有使用 `/write-vfunc-as-yaml` 调用的 SKILL 作为参考。

#### 3. 创建预处理脚本

  - 创建 `ida_preprocessor_scripts/find-CCSPlayerController_Respawn.py`

  - **务必**检查已有包含 `TARGET_FUNCTION_NAMES` 的预处理脚本作为参考。

#### 4. 在 `config.yaml` 的 `skills` 下添加新 SKILL

 * 显式声明 `expected_output` 和 `expected_input`（可选）。

```yaml
      - name: find-CBasePlayerController_Respawn
        expected_output:
          - CBasePlayerController_Respawn.{platform}.yaml
        expected_input:
          - CBasePlayerController_vtable.{platform}.yaml
```

#### 5. 在 `config.yaml` 的 `symbols` 下添加新符号

```yaml
      - name: CBasePlayerController_Respawn
        category: vfunc
        alias:
          - CBasePlayerController::Respawn
```

## 如何为全局变量创建 SKILL

* 务必确保 ida-pro-mcp server 正在运行。

* 对于人类贡献者：当你查找新符号时，应编写新的初始提示词，**不要**从 README 直接复制粘贴！

以 `IGameSystem_InitAllSystems` 和 `IGameSystem_InitAllSystems_pFirst` 为例

#### 1. 在 IDA 中查找目标符号

  - 在 IDA 中搜索字符串 `"IGameSystem::InitAllSystems"`，查找该字符串的 xrefs。引用该字符串的函数就是 `IGameSystem_InitAllSystems`。

  - 如果还没改名，请将其重命名为 `IGameSystem_InitAllSystems`。

  - 查看 `IGameSystem_InitAllSystems` 开头附近的模式：`( i = qword_XXXXXX; i; i = *(_QWORD *)(i + 8) )`

  - 如果还没改名，将前一步发现的 `qword_XXXXXX` 重命名为 `IGameSystem_InitAllSystems_pFirst`。

#### 2. 创建 SKILL

  - 创建项目级 skill `find-IGameSystem_InitAllSystems-AND-IGameSystem_InitAllSystems_pFirst`（**使用英文编写**）。

  - 该 SKILL 应生成 `IGameSystem_InitAllSystems.{platform}.yaml`，并包含 `func_sig`。

  - 该 SKILL 应生成 `IGameSystem_InitAllSystems_pFirst.{platform}.yaml`，并包含 `gv_sig`。

  - 不要打包 skill。

  - 该 SKILL 需要同时支持 `server.dll` 和 `libserver.so`。

  - **务必**检查已有使用 `/write-func-as-yaml` 与 `/write-globalvar-as-yaml` 调用的 SKILL 作为参考。

#### 3. 创建预处理脚本

  - 创建 `ida_preprocessor_scripts/find-IGameSystem_InitAllSystems-AND-IGameSystem_InitAllSystems_pFirst.py`

  - **务必**检查已有包含 `TARGET_FUNCTION_NAMES` 和 `TARGET_GLOBALVAR_NAMES` 的预处理脚本作为参考。

#### 4. 在 `config.yaml` 的 `skills` 下添加新 SKILL

 * 显式声明 `expected_output` 和 `expected_input`（可选）。

```yaml
      - name: find-IGameSystem_InitAllSystems-AND-IGameSystem_InitAllSystems_pFirst
        expected_output:
          - IGameSystem_InitAllSystems.{platform}.yaml
          - IGameSystem_InitAllSystems_pFirst.{platform}.yaml
```

#### 5. 在 `config.yaml` 的 `symbols` 下添加新符号

```yaml
      - name: IGameSystem_InitAllSystems
        category: func
        alias:
          - IGameSystem::InitAllSystems

      - name: IGameSystem_InitAllSystems_pFirst
        category: gv
        alias:
          - IGameSystem::InitAllSystems::pFirst
```

## 如何为结构体偏移创建 SKILL

* 务必确保 ida-pro-mcp server 正在运行。

* 对于人类贡献者：当你查找新符号时，应编写新的初始提示词，**不要**从 README 直接复制粘贴！

以 `CGameResourceService_BuildResourceManifest` 和 `CGameResourceService_m_pEntitySystem` 为例。

#### 1. 在 IDA 中查找目标符号

  - 在 IDA 中搜索字符串 `"CGameResourceService::BuildResourceManifest(start)"`，并查找其 xrefs。

  - xref 应指向一个函数——这就是 `CGameResourceService_BuildResourceManifest`。如果尚未改名，请将其重命名。

#### 2. 创建 SKILL

  - 创建项目级 skill `find-CGameResourceService_BuildResourceManifest-AND-CGameResourceService_m_pEntitySystem`（**使用英文编写**）。

  - 该 SKILL 应生成 `CGameResourceService_BuildResourceManifest.{platform}.yaml`，并包含 `func_sig`。

  - 该 SKILL 应生成 `CGameResourceService_m_pEntitySystem.{platform}.yaml`，并包含 `offset` 和 `offset_sig`。

  - 不要打包 skill。

  - 该 SKILL 需要同时支持 `server.dll` 和 `libserver.so`。

  - **务必**检查已有使用 `/write-structoffset-as-yaml` 调用的 SKILL 作为参考。

#### 3. 创建预处理脚本

 - 创建 `ida_preprocessor_scripts/find-CGameResourceService_BuildResourceManifest-AND-CGameResourceService_m_pEntitySystem.py`

  - **务必**检查已有包含 `TARGET_FUNCTION_NAMES` 和 `TARGET_STRUCT_MEMBER_NAMES` 的预处理脚本作为参考。

#### 4. 在 `config.yaml` 的 `skills` 下添加新 SKILL

 * 显式声明 `expected_output` 和 `expected_input`（可选）。

```yaml
      - name: find-CGameResourceService_BuildResourceManifest-AND-CGameResourceService_m_pEntitySystem
        expected_output:
          - CGameResourceService_BuildResourceManifest.{platform}.yaml
          - CGameResourceService_m_pEntitySystem.{platform}.yaml
```

#### 5. 在 `config.yaml` 的 `symbols` 下添加新符号

```yaml
      - name: CGameResourceService_BuildResourceManifest
        category: func
        alias:
          - CGameResourceService::BuildResourceManifest
          - BuildResourceManifest

      - name: CGameResourceService
        category: struct

      - name: CGameResourceService_m_pEntitySystem
        category: structmember
        struct: CGameResourceService
        member: m_pEntitySystem
        alias:
          - GameEntitySystem

```

## 如何为补丁创建 SKILL

* 补丁 SKILL 会在一个已知函数里定位特定指令，并生成替换字节来修改其运行时行为（例如强制/跳过某分支、NOP 掉某次调用）。目标函数通常应已有对应的 find-SKILL 输出（一般通过 `expected_input` 提供）。

* 务必确保 ida-pro-mcp server 正在运行。

* 对于人类贡献者：当你查找新符号时，应编写新的初始提示词，**不要**从 README 直接复制粘贴！

以 `CCSPlayer_MovementServices_FullWalkMove_SpeedClamp` 为例 —— 在 `CCSPlayer_MovementServices_FullWalkMove` 内把速度限制逻辑对应的 `jbe` 补丁为无条件 `jmp`。

#### 1. 在 IDA 中查找目标符号

  - 反编译 `CCSPlayer_MovementServices_FullWalkMove`，查找类似“某 float > 某 float 平方”的代码模式：

```c
  v20 = (float)((float)(v16 * v16) + (float)(v19 * v19)) + (float)(v17 * v17);
  if ( v20 > (float)(v18 * v18) )
  {
    ...velocity clamping logic...
  }
```

  - 在比较附近反汇编，找到确切的条件跳转指令。

  - 在比较地址附近反汇编，定位 `comiss + jbe` 指令对。

```
  期望的汇编模式：
    addss   xmm2, xmm1          ; v20 = sum of squares
    comiss  xmm2, xmm0          ; compare v20 vs v18*v18
    jbe     loc_XXXXXXXX         ; skip clamp block if v20 <= v18*v18
```

  - 根据指令编码确定补丁字节。

```
  * Near `jbe` (`0F 86 rel32`，6 字节) → `E9 <new_rel32> 90`（无条件 `jmp` + `nop`）
  * Short `jbe` (`76 rel8`，2 字节) → `EB rel8`（无条件 `jmp short`）
```

#### 2. 创建 SKILL

 - 创建项目级 skill `find-CCSPlayer_MovementServices_FullWalkMove_SpeedClamp`（**使用英文编写**）。

 - 该 SKILL 应生成 `CCSPlayer_MovementServices_FullWalkMove.{platform}.yaml`，并包含 `patch_sig` 与 `patch_bytes`。

 - 不要打包 skill。

 - 该 SKILL 需要同时支持 `server.dll` 和 `libserver.so`。

 - **务必**检查已有使用 `/write-patch-as-yaml` 调用的 SKILL 作为参考。

#### 3. 创建预处理脚本

 - 创建 `ida_preprocessor_scripts/find-CCSPlayer_MovementServices_FullWalkMove_SpeedClamp.py`

  - **务必**检查已有包含 `TARGET_PATCH_NAMES` 的预处理脚本作为参考。

#### 4. 在 `config.yaml` 的 `skills` 下添加新 SKILL

 * 显式声明 `expected_output` 和 `expected_input`（可选）。

```yaml
      - name: find-CCSPlayer_MovementServices_FullWalkMove_SpeedClamp
        expected_output:
          - CCSPlayer_MovementServices_FullWalkMove_SpeedClamp.{platform}.yaml
        expected_input:
          - CCSPlayer_MovementServices_FullWalkMove.{platform}.yaml
```

#### 5. 在 `config.yaml` 的 `symbols` 下添加新符号

```yaml
      - name: CCSPlayer_MovementServices_FullWalkMove_SpeedClamp
        category: patch
        alias:
          - ServerMovementUnlock
```

## 故障排查

### Cannot load IDA library file {name}, Please make sure you are using IDA

这是因为官方 idapro 包与 IDA 9.0 不兼容。

处理方式：将 `Python3**/Lib/site-packages/idapro/__init__.py` 替换为 `CS2_VibeSignatures/patched-py/Lib/site-packages/idapro/__init__.py`。

### error: could not create 'ida.egg-info': access denied

处理方式：在 `C:\Program Files\IDA Professional 9.0\idalib\python` 目录下，以**管理员权限**运行 `python py-activate-idalib.py`。

### Could not find idalib64.dll in .........

处理方式：尝试 `set IDADIR=C:\Program Files\IDA Professional 9.0`，或将 `IDADIR=C:\Program Files\IDA Professional 9.0` 添加到系统环境变量。

## Jenkins 工作流参考

```bash
@echo Download latest game binaries

uv run download_bin.py -gamever %CS2_GAMEVER%
```

```bash
@echo Analyze game binaries

uv run ida_analyze_bin.py -gamever %CS2_GAMEVER% -agent="claude.cmd" -platform %CS2_PLATFORM% -debug
```

```bash
@echo Update gamedata with generated yamls

uv run update_gamedata.py -gamever %CS2_GAMEVER% -debug
```

```bash
@echo Find mismatches in CS2SDK headers and fix them

uv run run_cpp_tests.py -gamever %CS2_GAMEVER% -debug -fixheader -agent="claude.cmd"
```
