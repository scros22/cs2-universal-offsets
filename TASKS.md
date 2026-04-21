# Roadmap

Tracking list for `cs2-universal-offsets`. Items are intentionally
small + safe — anything bigger gets discussed in an issue first.

Legend: `[ ]` planned · `[~]` in-progress · `[x]` done.

## Recently shipped

- [x] One-binary merger (offsets + signatures)
- [x] Dated session folder layout (`DD-MM-YY-CS2-SDK/`)
- [x] PE/section-aware IDA-style scanner with `Rel32` / `RipRel` / `StringRef`
- [x] Hand-formatted `signatures.json` (one entry per line, columns aligned, with `pattern`)
- [x] Multi-language signature output: `signatures.{cs,hpp,rs}` + `SIGNATURES.md`
- [x] Warm-cache from previous session's `signatures.json` (auto-detected + `--cache <path>` override)
- [x] Auto-diff vs previous session → `diff.json` (added / removed / shifted / pattern_changed)
- [x] Multi-match / ambiguity detection (per-hit `matches` count + run-end warning)
- [x] Removed noisy dead-code paths (`signature_research`, `signature_verification`, `advanced_signatures`)
- [x] GitHub Actions CI: build + clippy + fmt + auto-release on `v*` tags

## Scanner

- [ ] Multi-pattern single-pass scan (group sigs per module, walk `.text` once)
- [ ] SIMD pattern matcher (AVX2 BMH with wildcard mask) — most useful when sig count grows >1k
- [ ] `iced-x86` based string-ref walk-back instead of byte heuristics
- [ ] Auto-pattern generator: take a known VA, walk forward N bytes, mask RIP/imm operands, emit a stable IDA pattern
- [ ] Per-sig confidence score (uniqueness in section)
- [ ] Optional `--strict` flag that fails the run on any ambiguous (`matches > 1`) hit

## Output

- [ ] `signatures.zig` emitter to match the offset pass coverage
- [ ] `signatures.proto` / `.fbs` for cross-language consumers
- [ ] HTML report (sortable table, missing in red, diff highlights)
- [ ] `latest/` symlink/junction pointing at the most recent session

## Schema / offsets

- [ ] VTable index dumper per class (with method names where RTTI is present)
- [ ] Netvar / `CNetworkVarChainer` dumper alongside schemas
- [ ] ConVar dumper (name → address → default / min / max / flags)
- [ ] Interface-version drift detector (flag any `XXX_VERSIONXXX` change)

## Memory / backend

- [ ] First-class CLI presets for DMA backends (`--backend dma|kvm|native`)
- [ ] Read-once full-module dump → `--from-dump <path>` for fully offline replay
- [ ] Parallel module scans (`rayon`) once the multi-pattern path lands

## Distribution / repo

- [ ] Auto-publish a fresh `dumps/<DATE>-CS2-SDK/` to the repo on every CS2 update
- [ ] Auto-tag releases as `vYYYY.MM.DD` with the matching dump zipped as a release asset
- [ ] WinResource version info on the exe (FileVersion, ProductVersion, company)
- [ ] Code-signed exe (kills SmartScreen prompt)

## Code quality

- [ ] Replace pelite legacy scanner entirely with the IDA-style one (single source of truth)
- [ ] `thiserror` typed errors at the module boundaries
- [ ] Snapshot tests against a tiny pinned cs2 module dump in `tests/fixtures/`
- [ ] `tracing` + JSON log mode behind a flag
- [ ] Resolve the remaining `dead_code` warnings in `source2/` (legacy tier1 helpers)

## Nice-to-haves

- [ ] `ratatui` TUI with per-module / per-stage live progress
- [ ] Discord / webhook notifier on completion (with diff summary)
- [ ] Optional auto-upload of `signatures.json` to a paste service when run with `--share`
