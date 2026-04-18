# LLVM/MSVC `yvals.h` `__builtin_verbose_trap` Issue

## Summary

On one self-hosted Windows runner, the C++ test step fails while compiling `cpp_tests/igamesystem.cpp`:

```powershell
uv run run_cpp_tests.py -gamever "$env:GAMEVER" -fixheader -agent="$env:RUNNER_AGENT" -debug
```

The same repository state does not reproduce locally. The failure occurs inside the MSVC STL headers rather than in repository source code.

## Manifestation

### Observed behavior

- `run_cpp_tests.py` reports the target triple as supported:

  ```text
  === clang++ target triple detection ===
  clang++ -print-target-triple => x86_64-pc-windows-msvc
  === target support probe (from configured targets) ===
  [SUPPORTED] x86_64-pc-windows-msvc
  ```

- The real compile then fails for `IGameSystem_MSVC`:

  ```text
  [FAIL] IGameSystem_MSVC: compile failed
  Command: clang++ --target=x86_64-pc-windows-msvc -std=c++20 -c ...\cpp_tests\igamesystem.cpp ...
  ```

- The error is emitted from the MSVC STL headers:

  ```text
  C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.50.35717\include\xmemory:223:5:
  error: use of undeclared identifier '__builtin_verbose_trap'
  ```

- Macro expansion in the failing log shows:

  ```text
  #define _MSVC_STL_DOOM_FUNCTION(mesg) __builtin_verbose_trap("MSVC STL error", mesg)
  ```

### Include chain

The failure is triggered through the normal repository include chain:

`cpp_tests/igamesystem.cpp`
-> `tier1/convar.h`
-> `tier1/utlcommon.h`
-> `tier1/utlstring.h`
-> `tier1/utlmemory.h`
-> `mathlib/mathlib.h`
-> `mathlib/vector.h`
-> `tier0/threadtools.h`
-> MSVC STL headers such as `<memory>`, `xmemory`, `yvals.h`

### Why the probe passes but the test still fails

`run_cpp_tests.py` first calls `probe_target_support()`, which compiles only a trivial source:

```cpp
int main() { return 0; }
```

That probe does not include STL headers, so it can succeed even when later real-world test files fail after pulling in the MSVC STL.

## Root Cause

### High-confidence root cause

The failing runner resolves `clang++` and the MSVC STL headers to an incompatible combination:

1. The runner uses a newer MSVC STL toolset, visible in the failing log as:

   ```text
   C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.50.35717\include\...
   ```

2. That newer STL version uses `__builtin_verbose_trap(...)` in its error path.

3. The actual `clang++` selected on that runner, reported in logs as `llvm-msvc(v777.2.8)`, can target `x86_64-pc-windows-msvc`, but does not provide the builtin expected by that STL header set.

4. Once a translation unit pulls in STL-heavy headers, compilation fails before repository code becomes the real issue.

In short, this is an environment compatibility problem:

**newer MSVC STL headers + incompatible `clang++` frontend**

not a logic problem in `cpp_tests/igamesystem.cpp` or `hl2sdk_cs2`.

## Why It Reproduces Only On One Runner

Two environment-dependent factors explain the runner-only reproduction:

### 1. The workflow does not pin `clang++`

`.github/workflows/build-on-self-runner.yml` invokes:

```powershell
uv run run_cpp_tests.py -gamever "$env:GAMEVER" -fixheader -agent="$env:RUNNER_AGENT" -debug
```

and `run_cpp_tests.py` defaults to plain `clang++`.

This means different self-hosted runners can pick different `clang++` binaries from `PATH`.

### 2. The MSVC toolset version differs across machines

Local inspection shows a different MSVC toolset version:

```text
.../VC/Tools/MSVC/14.44.35207/include/yvals.h
```

That older local STL uses a different doom function path (`::_invoke_watson(...)`) rather than `__builtin_verbose_trap(...)`.

As a result, the local machine can avoid this exact failure even when compiling the same repository code.

## Evidence

### Evidence from repository behavior

- `run_cpp_tests.py` target probing proves only target-triple support, not STL compatibility.
- `cpp_tests/igamesystem.cpp` includes headers that eventually pull in the MSVC STL.
- The failure happens in system headers, not in repository-owned code.

### Evidence from environment differences

- Failing runner log points to `MSVC 14.50.35717`.
- Local inspection shows `MSVC 14.44.35207`.
- The workflow does not pin a specific `clang++` path or version, so runner PATH differences directly affect behavior.

## Recommended Fix Direction

### Preferred fixes

1. Pin the `clang++` path or version in CI so all self-hosted runners use the same frontend.
2. Align the MSVC toolset version across all self-hosted runners.
3. Add environment diagnostics before running `run_cpp_tests.py`, for example:

   ```powershell
   where clang++
   clang++ --version
   where cl.exe
   ```

### Optional temporary workaround

If a short-term unblock is needed, a workaround may be possible by forcing the STL doom path away from `__builtin_verbose_trap`, for example with `_MSVC_STL_USE_ABORT_AS_DOOM_FUNCTION`.

This is only a mitigation, not the real fix. The root issue remains the toolchain mismatch.

## Minimal Verification Commands

To confirm the diagnosis on the failing runner, run:

```powershell
where clang++
clang++ --version
```

and then a minimal STL probe:

```powershell
@'
#include <memory>
int main() { return 0; }
'@ | Set-Content .\probe.cpp

clang++ --target=x86_64-pc-windows-msvc -std=c++20 -c .\probe.cpp
```

If this minimal probe fails with the same `__builtin_verbose_trap` error, the problem is confirmed to be environment/toolchain compatibility rather than repository code.
