# JSC LoongArch64 Feature Record

This file tracks the local LoongArch64 JavaScriptCore port status for Bun.
Update it whenever a feature is implemented, deferred, or found to be out of
scope for the current phase.

## Scope

- Current phase: LLInt and JavaScript Baseline JIT non-SIMD completeness.
- Target CPU baseline: LoongArch64 v1.1.
- Explicitly deferred: LSX/LASX SIMD, Wasm BBQ/OMG/B3/Air-specific work unless
  it blocks LLInt or JavaScript Baseline JIT.

## Status Legend

- `done`: implemented and has at least smoke/stress coverage.
- `in-progress`: being implemented in the current phase.
- `missing`: not implemented and relevant to the current phase.
- `defer`: not part of the current phase.
- `audit`: needs reachability or correctness audit before deciding.

## LLInt / OfflineASM

| Feature | Status | Notes |
| --- | --- | --- |
| JavaScript LLInt LoongArch64 backend | done | `offlineasm/loongarch64.rb` is present and used by `LowLevelInterpreter.asm`. |
| Wasm IPInt basic execution | done | Initial Loong64 IPInt support is committed. |
| Wasm IPInt atomic operations | done | LoongArch v1.1 atomics are implemented in offlineasm and tested remotely. |
| OfflineASM scalar load/store/arithmetic/branch lowering | done | Main LLInt/IPInt scalar paths are implemented. |
| OfflineASM `loadpair*` / `storepair*` pseudo ops | done | Lowered as scalar operations for Loong64; do not treat as hardware pair ops. |
| OfflineASM vector load/store pseudo ops | defer | Only stack/value movement support exists; SIMD operations remain out of scope. |
| LLInt callee-save register set | audit | Loong64 currently preserves `csr5..csr8` for metadata/PB/tags. Re-audit against ARM64/RISCV64/x64 call boundaries. |

## JavaScript Baseline JIT

| Feature | Status | Notes |
| --- | --- | --- |
| Baseline JIT entry/prologue/slow-path call plumbing | done | Existing Loong64 baseline commit builds and runs stress subsets. |
| Property access IC / DataIC plumbing | partial | Targeted get/put/delete dynamic property test passed; broader stress still needed. |
| Call frame shuffling | partial | Direct call, `call`, and `apply` targeted tests passed; constructor/varargs stress still needed. |
| RegExp Yarr JIT integration | partial | Targeted `exec`/`test` loop passed with RegExp JIT enabled; full Yarr stress still needed. |
| Baseline `%` fast path | done | Loong64 fast path uses `MOD.W`; targeted `%` test passed under LLInt and Baseline-only. |

## MacroAssembler Non-SIMD

| Feature | Status | Notes |
| --- | --- | --- |
| Integer add/sub/mul/div/shift/compare/test | done | Present in `MacroAssemblerLOONGARCH64.h`; covered by baseline smoke/stress. |
| Floating-point branch/compare against register | done | Existing `branchFP` / `compareFP` templates handle ordered/unordered conditions. |
| Floating-point branch/compare against zero | done | `branch*WithZero` and `compare*WithZero` implemented with Loong64-specific zero compare helpers. |
| Conditional integer move based on FP compare with zero | done | `moveConditionally*WithZero` implemented and covered by FP/zero smoke. |
| Conditional FP move based on FP compare with zero | done | `moveDoubleConditionally*WithZero` implemented; keep DFG/B3 coverage separate from this phase. |
| Rotate right 32/64 | audit | MacroAssembler methods are noops. Likely needed by Wasm BBQ/B3 before JS Baseline, but cheap to implement later. |
| Popcount 32/64 | defer | `supportsCountPopulation()` is false. IPInt uses C slow paths; defer unless Baseline reachability appears. |
| Carry materialization / `ResultCondition::Carry` | audit | Not expected on JS Baseline hot paths; required by later Wasm BBQ address-overflow work. |
| Float16 scalar helpers | defer | `supportsFloat16()` is false; Float16 helpers remain unreachable unless Float16 is enabled. |
| LSX/LASX vector helpers | defer | SIMD phase. |

## Validation Matrix

| Test Area | Status | Notes |
| --- | --- | --- |
| LLInt-only JavaScript smoke | done | Remote `jsc-loong64-phase2-fpzero --useJIT=false` smoke passed. |
| Baseline-only JavaScript stress | partial | Remote Baseline-only smoke and FP/zero targeted test passed; broader stress still pending. |
| Arithmetic targeted stress | partial | FP zero/NaN comparison and `%` tests passed; broader int overflow still pending. |
| Property access targeted stress | partial | Basic get/put/delete dynamic property test passed; enumerator paths still pending. |
| Call-frame targeted stress | partial | Direct call, `call`, and `apply` passed; construct/varargs/tail-call still pending. |
| Yarr/RegExp targeted stress | partial | Basic RegExp JIT `exec`/`test` passed; full Yarr stress still pending. |
| Wasm IPInt non-SIMD regression | pending | Keep previous known SIMD/GC exclusions separate from this phase. |
