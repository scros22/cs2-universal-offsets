# copy_depot_bin `-checkonly` Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 `copy_depot_bin.py` 增加 `-checkonly` 参数，并让自托管 workflow 在目标二进制已齐全时跳过 depot 下载与复制。

**Architecture:** 先用单元测试锁定 CLI 的退出码与缓存判定语义，再把 `copy_depot_bin.py` 重构为“共享目标枚举 + 分支执行”的结构，让复制模式与检查模式使用同一套目标路径规则。随后更新 `.github/workflows/build-on-self-runner.yml`，把脚本的 `0/1/2` 退出码翻译成 `bin_ready=true/false/fail`，只在缓存缺失时执行下载与复制步骤，并补充 README 用法说明。

**Tech Stack:** Python 3, `argparse`, `pathlib`, `yaml.safe_load`, `unittest`, GitHub Actions YAML, PowerShell

---

## File Map

- Modify: `copy_depot_bin.py`
  - 增加 `-checkonly`
  - 抽取共享目标枚举逻辑
  - 让 `main()` 返回显式退出码并由 `sys.exit(main())` 收尾
- Create: `tests/test_copy_depot_bin.py`
  - 覆盖 `-checkonly` 的命中、缺失、异常三种退出码
  - 覆盖普通复制模式仍要求 depot 目录存在的回归场景
- Modify: `.github/workflows/build-on-self-runner.yml`
  - 新增 `Check cached binaries` step
  - 用 `steps.<id>.outputs.bin_ready` 控制下载与复制步骤
- Modify: `README.md`
  - 补充 `-checkonly` 命令行示例与用途说明
- Modify: `README_CN.md`
  - 补充 `-checkonly` 命令行示例与用途说明

### Task 1: 用测试锁定 `copy_depot_bin.py` 的目标行为

**Files:**
- Create: `tests/test_copy_depot_bin.py`
- Modify: `copy_depot_bin.py`

- [ ] **Step 1: 新建失败测试文件**

```python
import argparse
import os
import tempfile
import unittest
from unittest.mock import patch

import copy_depot_bin


class TestCopyDepotBin(unittest.TestCase):
    def _write_config(self, root: str, *, include_linux: bool = False) -> str:
        lines = [
            "modules:",
            "  - name: server",
            "    path_windows: game/bin/win64/server.dll",
        ]
        if include_linux:
            lines.append("    path_linux: game/bin/linuxsteamrt64/libserver.so")

        config_path = os.path.join(root, "config.yaml")
        with open(config_path, "w", encoding="utf-8") as handle:
            handle.write("\n".join(lines) + "\n")
        return config_path

    def _make_args(
        self,
        *,
        bindir: str,
        gamever: str,
        platform: str,
        depotdir: str,
        config: str,
        checkonly: bool,
    ) -> argparse.Namespace:
        return argparse.Namespace(
            bindir=bindir,
            gamever=gamever,
            platform=platform,
            depotdir=depotdir,
            config=config,
            checkonly=checkonly,
        )

    def test_main_checkonly_returns_zero_when_all_expected_targets_exist(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            config_path = self._write_config(temp_dir)
            bindir = os.path.join(temp_dir, "bin")
            target_dir = os.path.join(bindir, "14141", "server")
            os.makedirs(target_dir, exist_ok=True)

            with open(os.path.join(target_dir, "server.dll"), "wb") as handle:
                handle.write(b"ok")

            args = self._make_args(
                bindir=bindir,
                gamever="14141",
                platform="windows",
                depotdir=os.path.join(temp_dir, "missing_depot"),
                config=config_path,
                checkonly=True,
            )

            with patch("copy_depot_bin.parse_args", return_value=args):
                self.assertEqual(0, copy_depot_bin.main())

    def test_main_checkonly_returns_one_when_any_expected_target_is_missing(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            config_path = self._write_config(temp_dir, include_linux=True)
            bindir = os.path.join(temp_dir, "bin")
            target_dir = os.path.join(bindir, "14141", "server")
            os.makedirs(target_dir, exist_ok=True)

            with open(os.path.join(target_dir, "server.dll"), "wb") as handle:
                handle.write(b"ok")

            args = self._make_args(
                bindir=bindir,
                gamever="14141",
                platform="all-platform",
                depotdir=os.path.join(temp_dir, "missing_depot"),
                config=config_path,
                checkonly=True,
            )

            with patch("copy_depot_bin.parse_args", return_value=args):
                self.assertEqual(1, copy_depot_bin.main())

    def test_main_checkonly_returns_two_when_config_is_missing(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            bindir = os.path.join(temp_dir, "bin")
            args = self._make_args(
                bindir=bindir,
                gamever="14141",
                platform="all-platform",
                depotdir=os.path.join(temp_dir, "missing_depot"),
                config=os.path.join(temp_dir, "missing.yaml"),
                checkonly=True,
            )

            with patch("copy_depot_bin.parse_args", return_value=args):
                self.assertEqual(2, copy_depot_bin.main())

    def test_main_copy_mode_still_requires_existing_depot_directory(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            config_path = self._write_config(temp_dir)
            bindir = os.path.join(temp_dir, "bin")
            args = self._make_args(
                bindir=bindir,
                gamever="14141",
                platform="windows",
                depotdir=os.path.join(temp_dir, "missing_depot"),
                config=config_path,
                checkonly=False,
            )

            with patch("copy_depot_bin.parse_args", return_value=args):
                self.assertEqual(1, copy_depot_bin.main())


if __name__ == "__main__":
    unittest.main()
```

- [ ] **Step 2: 运行新测试，确认当前实现尚未满足**

Run:

```bash
uv run python -m unittest tests.test_copy_depot_bin -v
```

Expected:

```text
FAILED (errors=4)
```

失败原因应包括：

- `copy_depot_bin.main()` 仍会直接 `sys.exit(...)`
- 当前实现还不会根据 `checkonly=True` 走检查分支
- 当前没有 `0/1/2` 的显式退出码语义

### Task 2: 重构 `copy_depot_bin.py` 以支持 `-checkonly`

**Files:**
- Modify: `copy_depot_bin.py`
- Test: `tests/test_copy_depot_bin.py`

- [ ] **Step 1: 在 CLI 中加入 `-checkonly` 参数**

把 `parse_args()` 的参数定义扩展为：

```python
    parser.add_argument(
        "-checkonly",
        action="store_true",
        help=(
            "Only check whether expected target binaries already exist. "
            "Return 0 when all expected targets exist, 1 when any target is missing, "
            "and 2 for configuration or argument errors."
        ),
    )
```

- [ ] **Step 2: 抽取共享目标枚举逻辑**

在 `copy_depot_bin.py` 中新增一个专门生成目标条目的 helper，供复制模式与检查模式共用：

```python
def iter_module_entries(module, bin_dir, gamever, platform_filter, depot_dir):
    """Yield expected source/target entries for the selected module/platforms."""
    name = module["name"]
    flat = platform_filter == "all-platform"

    if platform_filter and not flat:
        platforms = [platform_filter]
    else:
        platforms = ["windows", "linux"]

    entries = []
    for platform in platforms:
        path = module.get(f"path_{platform}")
        if not path:
            print(f"  Skipping {name} ({platform}): no path defined")
            continue

        filename = Path(path).name
        entries.append(
            {
                "name": name,
                "platform": platform,
                "source_path": build_source_path(depot_dir, platform, path, flat=flat),
                "target_path": os.path.join(bin_dir, gamever, name, filename),
            }
        )

    return entries
```

- [ ] **Step 3: 新增 `-checkonly` 执行分支与显式退出码**

把 `main()` 改成返回整数，并增加检查逻辑：

```python
CHECKONLY_MISSING_EXIT = 1
CHECKONLY_ERROR_EXIT = 2


def check_module_targets(module, bin_dir, gamever, platform_filter, depot_dir):
    ready_count = 0
    missing_count = 0

    for entry in iter_module_entries(module, bin_dir, gamever, platform_filter, depot_dir):
        print(f"\nChecking: {entry['name']} ({entry['platform']})")
        if os.path.exists(entry["target_path"]):
            print(f"  [READY] Target already exists: {entry['target_path']}")
            ready_count += 1
        else:
            print(f"  [MISSING] Target not found: {entry['target_path']}")
            missing_count += 1

    return ready_count, missing_count


def main() -> int:
    args = parse_args()
    config_path = args.config
    bin_dir = args.bindir
    gamever = args.gamever
    platform_filter = args.platform
    depot_dir = args.depotdir
    error_exit = CHECKONLY_ERROR_EXIT if args.checkonly else 1

    if not os.path.exists(config_path):
        print(f"Error: Config file not found: {config_path}")
        return error_exit

    if not args.checkonly and not os.path.isdir(depot_dir):
        print(f"Error: Depot directory not found: {depot_dir}")
        return 1

    if not args.checkonly:
        os.makedirs(bin_dir, exist_ok=True)

    try:
        modules = parse_config(config_path)
    except yaml.YAMLError as exc:
        print(f"Error: Failed to parse config file: {exc}")
        return error_exit

    if not modules:
        print("No modules found in config.")
        return 0

    if args.checkonly:
        total_ready = 0
        total_missing = 0

        for module in modules:
            ready, missing = check_module_targets(
                module, bin_dir, gamever, platform_filter, depot_dir
            )
            total_ready += ready
            total_missing += missing

        print(f"\n{'=' * 50}")
        print(f"Check-only summary: {total_ready} ready, {total_missing} missing")

        if total_missing > 0:
            print("CHECKONLY_RESULT=missing")
            return CHECKONLY_MISSING_EXIT

        print("CHECKONLY_RESULT=ready")
        return 0

    total_success = 0
    total_fail = 0
    for module in modules:
        success, fail = process_module(module, bin_dir, gamever, platform_filter, depot_dir)
        total_success += success
        total_fail += fail

    print(f"\n{'=' * 50}")
    print(f"Completed: {total_success} successful, {total_fail} failed")
    return 1 if total_fail > 0 else 0


if __name__ == "__main__":
    sys.exit(main())
```

- [ ] **Step 4: 让复制模式复用共享目标枚举逻辑**

把 `process_module()` 改成只负责“按条目复制”，不再自己拼装路径：

```python
def process_module(module, bin_dir, gamever, platform_filter, depot_dir):
    success_count = 0
    fail_count = 0

    for entry in iter_module_entries(module, bin_dir, gamever, platform_filter, depot_dir):
        print(f"\nProcessing: {entry['name']} ({entry['platform']})")

        if os.path.exists(entry["target_path"]):
            print(f"  [SKIP] File already exists, skipping copy: {entry['target_path']}")
            success_count += 1
            continue

        if not os.path.exists(entry["source_path"]):
            print(f"  [ERROR] Source file not found in depot: {entry['source_path']}")
            fail_count += 1
            continue

        if copy_file(entry["source_path"], entry["target_path"]):
            success_count += 1
        else:
            fail_count += 1

    return success_count, fail_count
```

- [ ] **Step 5: 运行定向测试，确认新行为与回归语义都已生效**

Run:

```bash
uv run python -m unittest tests.test_copy_depot_bin -v
```

Expected:

```text
OK
```

### Task 3: 在自托管 workflow 中接入缓存预检查

**Files:**
- Modify: `.github/workflows/build-on-self-runner.yml`
- Test: `tests/test_copy_depot_bin.py`

- [ ] **Step 1: 在下载前加入缓存检查 step**

把以下 step 插入到 `Create persisted workspace links` 之后：

```yaml
      - name: Check cached binaries
        id: check_cached_binaries
        shell: pwsh
        run: |
          uv run copy_depot_bin.py -gamever "$env:GAMEVER" -platform all-platform -checkonly
          $exitCode = $LASTEXITCODE

          if ($exitCode -eq 0) {
            "bin_ready=true" | Out-File -FilePath $env:GITHUB_OUTPUT -Encoding utf8 -Append
            Write-Host "All expected binaries already exist; skipping depot download."
            exit 0
          }

          if ($exitCode -eq 1) {
            "bin_ready=false" | Out-File -FilePath $env:GITHUB_OUTPUT -Encoding utf8 -Append
            Write-Host "At least one expected binary is missing; depot download is required."
            exit 0
          }

          throw "copy_depot_bin.py -checkonly failed with exit code $exitCode"
```

- [ ] **Step 2: 只在缓存未命中时下载 depot**

给 `Update CS2 depot` step 增加条件：

```yaml
      - name: Update CS2 depot
        if: steps.check_cached_binaries.outputs.bin_ready != 'true'
        shell: pwsh
        run: |
          $depotDir = Join-Path $env:GITHUB_WORKSPACE "cs2_depot"
          uv run download_depot.py -tag "$env:TAG" -depotdir "$depotDir" -config download.yaml
```

- [ ] **Step 3: 只在缓存未命中时复制 depot 二进制**

给 `Copy depot binaries` step 增加条件：

```yaml
      - name: Copy depot binaries
        if: steps.check_cached_binaries.outputs.bin_ready != 'true'
        shell: pwsh
        run: |
          uv run copy_depot_bin.py -gamever "$env:GAMEVER" -platform all-platform
```

- [ ] **Step 4: 用静态检查确认 workflow 已接好输出变量**

Run:

```bash
rg -n "check_cached_binaries|bin_ready|Check cached binaries" .github/workflows/build-on-self-runner.yml
```

Expected:

```text
<至少 3 处匹配>
```

匹配中应包含：

- `id: check_cached_binaries`
- 两处 `if: steps.check_cached_binaries.outputs.bin_ready != 'true'`

### Task 4: 补充中英文 README 的 `-checkonly` 用法

**Files:**
- Modify: `README.md`
- Modify: `README_CN.md`

- [ ] **Step 1: 更新英文 README 示例**

把 `README.md` 中“Download CS2 depot and copy binaries to workspace”段落补成：

````markdown
```bash
DepotDownloader -app 730 -depot 2347771 -os all-platform -dir cs2_depot [-branch animgraph_2_beta]
DepotDownloader -app 730 -depot 2347773 -os all-platform -dir cs2_depot [-branch animgraph_2_beta]

uv run copy_depot_bin.py -gamever 14141 -platform all-platform
uv run copy_depot_bin.py -gamever 14141 -platform all-platform -checkonly
```

Use `-checkonly` in CI or preflight scripts when you only need to know whether all expected target binaries already exist under `bin/<gamever>/...`.
````

- [ ] **Step 2: 更新中文 README 示例**

把 `README_CN.md` 中对应段落补成：

````markdown
```bash
DepotDownloader -app 730 -depot 2347771 -os all-platform -dir cs2_depot [-branch animgraph_2_beta]
DepotDownloader -app 730 -depot 2347773 -os all-platform -dir cs2_depot [-branch animgraph_2_beta]

uv run copy_depot_bin.py -gamever 14141 -platform all-platform
uv run copy_depot_bin.py -gamever 14141 -platform all-platform -checkonly
```

当只需要确认 `bin/<gamever>/...` 下的目标二进制是否已经齐全时，可在 CI 或预检查脚本中使用 `-checkonly`。
````

- [ ] **Step 3: 确认文档中已出现新参数**

Run:

```bash
rg -n "checkonly" README.md README_CN.md
```

Expected:

```text
README.md:<line>:uv run copy_depot_bin.py -gamever 14141 -platform all-platform -checkonly
README_CN.md:<line>:uv run copy_depot_bin.py -gamever 14141 -platform all-platform -checkonly
```

### Task 5: 做一次收尾验证，确保局部改动闭环

**Files:**
- Modify: `copy_depot_bin.py`
- Modify: `.github/workflows/build-on-self-runner.yml`
- Modify: `README.md`
- Modify: `README_CN.md`
- Create: `tests/test_copy_depot_bin.py`

- [ ] **Step 1: 运行 Python 定向回归测试**

Run:

```bash
uv run python -m unittest tests.test_copy_depot_bin tests.test_download_depot -v
```

Expected:

```text
OK
```

- [ ] **Step 2: 复核最终 diff 只覆盖本次需求**

Run:

```bash
git diff -- copy_depot_bin.py tests/test_copy_depot_bin.py .github/workflows/build-on-self-runner.yml README.md README_CN.md
```

Expected:

```text
diff --git a/copy_depot_bin.py b/copy_depot_bin.py
diff --git a/tests/test_copy_depot_bin.py b/tests/test_copy_depot_bin.py
diff --git a/.github/workflows/build-on-self-runner.yml b/.github/workflows/build-on-self-runner.yml
diff --git a/README.md b/README.md
diff --git a/README_CN.md b/README_CN.md
```

- [ ] **Step 3: 记录完成口径**

收尾说明应明确写出：

```text
- `copy_depot_bin.py -checkonly` 已支持 0/1/2 三态退出码
- self-runner workflow 已在缓存完整时跳过 depot 下载与复制
- 普通复制模式仍保持原有行为
- 已完成 `tests.test_copy_depot_bin` 与 `tests.test_download_depot` 定向验证
```
