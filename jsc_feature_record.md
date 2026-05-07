# JSC LoongArch64 Feature Record

This file tracks the local LoongArch64 JavaScriptCore port status for Bun.
Update it whenever a feature is implemented, deferred, or found to be out of
scope for the current phase.

## Scope

- Current phase: Wasm BBQ SIMD/relaxed SIMD bring-up after LLInt, JavaScript
  Baseline JIT, IPInt atomic, Wasm BBQ scalar, and Wasm IPInt SIMD validation.
- Target CPU baseline: LoongArch64 v1.1.
- Explicitly deferred: Wasm OMG/B3/Air-specific work unless it blocks the
  current IPInt/BBQ baseline. LASX remains deferred; current SIMD work targets
  LSX-width 128-bit Wasm SIMD.

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
| Wasm IPInt atomic operations | done | LoongArch v1.1 atomics are implemented in offlineasm. Remote tests passed for load/store, RMW, xchg, cmpxchg, wait/notify, and fence. LLInt audit also found and fixed the IPInt-internal branching CAS pseudo ops (`batomicweakcas*`) so byte/half/int/quad CAS loops no longer fall into the generic LL/SC placeholder path. |
| OfflineASM scalar load/store/arithmetic/branch lowering | done | Main LLInt/IPInt scalar paths are implemented. |
| OfflineASM `loadpair*` / `storepair*` pseudo ops | done | Lowered as scalar operations for Loong64; do not treat as hardware pair ops. |
| OfflineASM vector load/store pseudo ops | done | `loadv`/`storev` and IPInt SIMD stack/local movement use LSX vector loads/stores. |
| LLInt/IPInt opcode table audit | done | SIMD placeholder table has 256/256 implemented handlers; atomic placeholder table has 67/67 implemented handlers. Generic `unimplementedInstruction(...)` rows are dispatch-table placeholders, not Loong64-specific missing handlers. |
| LLInt callee-save register set | done | Loong64 preserves the required LLInt/IPInt state registers and separately reserves macro scratch FPRs; previous IPInt/BBQ SIMD validation covered the Wasm call/slow-path boundaries. |

## JavaScript Baseline JIT

| Feature | Status | Notes |
| --- | --- | --- |
| Baseline JIT entry/prologue/slow-path call plumbing | done | Existing Loong64 baseline commit builds and runs stress subsets. |
| Property access IC / DataIC plumbing | partial | Targeted get/put/delete dynamic property test passed; selected and broad Baseline-only microbenchmarks also passed. |
| Call frame shuffling | partial | Direct call, `call`, `apply`, construct, arguments, varargs, and tail-call selected and broad microbenchmarks passed. |
| RegExp Yarr JIT integration | partial | Targeted `exec`/`test` loop passed with RegExp JIT enabled; selected and broad RegExp microbenchmarks passed. |
| Baseline `%` fast path | done | Loong64 fast path uses `MOD.W`; targeted `%` test passed under LLInt and Baseline-only. |

## MacroAssembler Non-SIMD

| Feature | Status | Notes |
| --- | --- | --- |
| Integer add/sub/mul/div/shift/compare/test | done | Present in `MacroAssemblerLOONGARCH64.h`; covered by baseline smoke/stress. |
| Floating-point branch/compare against register | done | Existing `branchFP` / `compareFP` templates handle ordered/unordered conditions. |
| Floating-point branch/compare against zero | done | `branch*WithZero` and `compare*WithZero` implemented with Loong64-specific zero compare helpers. |
| Conditional integer move based on FP compare with zero | done | `moveConditionally*WithZero` implemented and covered by FP/zero smoke. |
| Conditional FP move based on FP compare with zero | done | `moveDoubleConditionally*WithZero` implemented; keep DFG/B3 coverage separate from this phase. |
| Rotate right 32/64 | done | Implemented with LoongArch scalar `rotr.w/d` and `rotri.w/d`; 32-bit results are zero-extended for MacroAssembler semantics. |
| CLZ/CTZ 32/64 | done | Uses LoongArch scalar `clz.w/d` and `ctz.w/d`; replaced previous loop lowering. |
| Popcount 32/64 | defer | `supportsCountPopulation()` remains false. LoongArch has LSX `vpcnt`, but popcount is deferred to the SIMD/LSX phase. |
| Carry materialization / `ResultCondition::Carry` | missing | Not expected on JS Baseline hot paths; required by later Wasm BBQ address-overflow and B3/Air work. |
| Float16 scalar helpers | defer | `supportsFloat16()` is false; Float16 helpers remain unreachable unless Float16 is enabled. |
| LSX/LASX vector helpers | defer | SIMD phase. |

## Wasm BBQ Scalar

| Feature | Status | Notes |
| --- | --- | --- |
| BBQ build/runtime enablement without B3/FTL | done | CMake default and runtime option gating allow `ENABLE_WEBASSEMBLY_BBQJIT=ON` on LoongArch64 while OMG/B3 remain disabled. |
| B3 utility leakage into BBQ | done | Pure helpers in `B3Common.h` are available to BBQ without enabling B3; B3/Air-only declarations remain guarded. |
| Scalar CLZ/CTZ | done | Uses LoongArch scalar `clz.w/d` and `ctz.w/d`; encoding was cross-checked against LLVM/binutils after fixing opcode-field packing. |
| Scalar rotate right | done | Uses LoongArch scalar `rotr.w/d` and `rotri.w/d`; 32-bit results are zero-extended. |
| 32-bit and 64-bit div/rem helpers | done | 32-bit operands are sign/zero-extended before `DIV.W[U]`/`MOD.W[U]` to satisfy LoongArch operand validity requirements. |
| Scalar memory load/store extensions | done | Added BBQ-needed signed 8/16/32 load-to-64 and transfer helpers. |
| Indirect call/table access | done | Fixed LoongArch MacroAssembler immediate compare/branch-test temp-register clobber that corrupted refs in `call_indirect`. |
| Tail calls | done | Direct and indirect tail calls use the LoongArch link-register frame layout: restore caller FP/RA from the frame header, shuffle arguments, then repair SP/FP before the tail jump. Remote BBQ-only tail-call direct/indirect/stack/stress tests passed. |
| SIMD in BBQ | done | LSX-backed BBQ SIMD is implemented for the current standard/relaxed SIMD surface. Remote BBQ-only SIMD spec passed 57/57, and filtered SIMD stress passed 60/60. |
| Popcount lowering | defer | LoongArch LSX has `vpcnt`, but scalar BBQ keeps popcount deferred until SIMD/LSX phase. |

## Wasm IPInt SIMD

| Feature | Status | Notes |
| --- | --- | --- |
| Runtime gate: `useWasmSIMD` | done | Loong64 is allowed through the Wasm SIMD runtime gate after IPInt SIMD spec coverage passed. |
| Runtime gate: `useWasmIPIntSIMD` | done | Loong64 participates in IPInt SIMD validation and dispatch base setup with ARM64/X86_64. `WasmIPIntPlan` does not force SIMD functions away from IPInt when IPInt SIMD is enabled. |
| Runtime gate: `useWasmRelaxedSIMD` | done | Default remains false, but Loong64 relaxed SIMD handlers are present and pass targeted relaxed SIMD stress when explicitly enabled. |
| IPInt SIMD dispatch/validation | done | `ipint_simd_dispatch_base` is initialized and all standard/relaxed SIMD opcode slots used by the parser are validated for Loong64. `alignIPInt` is widened to 512 bytes on Loong64 to keep large handlers inside slots. |
| V128 stack/local representation | done | IPInt uses 16-byte `StackValueSize`/`LocalSize`. Loong64 `pushVec`/`popVec`/`loadv`/`storev` use LSX vector loads/stores, not two GPRs. Scalar GPR use is only valid at scalar boundaries such as splat input, lane extract, bitmask, `any_true`, and `all_true`. |
| FPR/V128 argument preservation | done | Loong64 follows the X86_64-style IPInt save/restore loop but uses 128-bit `storev`/`loadv` for Wasm FPR argument registers, so V128 arguments survive slow-path/materialization calls. |
| JS/Wasm boundary for V128 | done | Fast JS-to-Wasm IC and Wasm-to-JS imports reject signatures containing V128, matching the spec-visible TypeError behavior. Internal Wasm calls still use Wasm calling convention registers/stack. |
| V128 constants, direct load/store, lane load/store, extract/replace lane | done | Implemented through LSX vector stack slots plus scalar lane memory accesses where appropriate; full SIMD spec coverage passes. |
| V128 globals | done | Global slots are 16 bytes in IPInt and `global_get/set` use `loadv/storev`; `simd_const.wast.js` now passes. |
| Integer and FP comparisons | done | Loong64 LSX compare handlers for i8/i16/i32/i64 and f32/f64 pass the corresponding remote spec tests. FP `ne` uses unordered-not-equal (`vfcmp.cune.*`) as required by Wasm NaN semantics. |
| Boolean vector ops | done | `not`, `and`, `andnot`, `or`, `xor`, `bitselect`, `any_true`, and lane-width `all_true` are implemented and `simd_boolean.wast.js` passes. |
| Splat operations | done | Integer splats use `vreplgr2vr.*`; `f32x4.splat`/`f64x2.splat` use the offlineasm FPR temp mapping correctly and pass `simd_splat.wast.js`. |
| Memory extend/splat loads | done | `simdLoad8x8*`, `simdLoad16x4*`, `simdLoad32x2*`, and `simdLoadSplat*` have Loong64 LSX implementations. |
| Shuffle/swizzle | done | `i8x16.shuffle` and standard/relaxed swizzle are implemented with LSX table operations plus explicit mask handling for standard Wasm out-of-range zeroing. |
| Integer arithmetic/shift/minmax/saturating/narrow/extend/extmul/dot | done | Standard integer SIMD groups are implemented with LSX lowering and pass the SIMD spec suite. |
| FP arithmetic/rounding/conversion | done | `abs/neg/sqrt/add/sub/mul/div/min/max/pmin/pmax`, rounding, promote/demote, and trunc/convert handlers are implemented and pass the SIMD spec suite. |
| Bitmask and all-true reductions | done | Bitmask and lane-width `all_true` reductions are implemented and covered by SIMD spec/stress. |
| Relaxed SIMD handlers | done | Relaxed swizzle, relaxed trunc, relaxed madd/nmadd, lane select, min/max, q15mulr, and relaxed dot handlers are implemented and pass targeted relaxed SIMD stress. |
| IPInt exception/catch with V128 | done | Loong64 is enabled in IPInt catch/catch_all entries and preserves V128 state across exception paths; selected SIMD exception stress now passes. |

## Validation Matrix

| Test Area | Status | Notes |
| --- | --- | --- |
| LLInt-only JavaScript smoke | done | Remote `jsc-loong64-phase2-fpzero --useJIT=false` smoke passed. |
| Baseline-only JavaScript stress | done | Remote Baseline-only smoke, FP/zero targeted test, `%` targeted test, 100 selected microbenchmarks, and 300 broad microbenchmarks passed. |
| Arithmetic targeted stress | done | FP zero/NaN comparison, `%`, selected overflow/mod microbenchmarks, and broad bit/shift microbenchmarks passed. |
| Property access targeted stress | done | Basic get/put/delete dynamic property and selected/broad get/put/delete/for-in microbenchmarks passed. |
| Call-frame targeted stress | done | Direct call, `call`, `apply`, construct, arguments, varargs, and tail-call selected/broad tests passed. |
| Yarr/RegExp targeted stress | done | Basic RegExp JIT `exec`/`test` and selected/broad RegExp microbenchmarks passed. |
| Wasm IPInt non-SIMD regression | done | Remote IPInt non-SIMD suite passed 101/101 with SIMD and known GC starter excluded. |
| Wasm IPInt SIMD spec | done | Remote IPInt-only run passed all 57 `JSTests/wasm/simd-spec-tests/*.wast.js` files with `--useWasmIPInt=true --useWasmIPIntSIMD=true --useBBQJIT=false --useOMGJIT=false --useWasmSIMD=true`. |
| Wasm IPInt relaxed SIMD stress | done | Remote targeted relaxed SIMD stress passed 9/9 `stress/simd-const-relaxed-*.js` files with `--useWasmRelaxedSIMD=true`. |
| Wasm IPInt SIMD stress | done | Remote large IPInt-only SIMD run passed 63/63 non-v8 SIMD/gc/ipint/stress files after excluding non-target OMG, skip/timeout, missing-resource, and unsupported multimemory cases. The 7 `wasm/v8/*simd*.js` tests also pass when run like WebKit's `runV8WebAssemblySuite(:no_module, "mjsunit.js")`: copy `wasm/v8/resources` helpers plus same-directory dependencies into the run directory and invoke `jsc mjsunit.js test.js` without `-m`. |
| Wasm BBQ scalar smoke | done | Remote BBQ-only smoke passed with `--useWasmIPInt=false --useBBQJIT=true --useOMGJIT=false --useWasmSIMD=false`. |
| Wasm BBQ selected non-SIMD tests | done | Remote BBQ-only non-SIMD suite passed 113/113 with `--useWasmIPInt=false --useBBQJIT=true --useOMGJIT=false --useWasmSIMD=false`, excluding SIMD and the known invalid/empty-buffer `ipint-test-gc-starter.js`. Coverage includes direct calls, indirect calls/table access, multi-return, memory, globals, GC struct, LEB decode, and direct/indirect tail calls. |
| Wasm BBQ SIMD spec | done | Remote BBQ-only run passed all 57 `JSTests/wasm/simd-spec-tests/*.wast.js` files with `--useWasmIPInt=false --useBBQJIT=true --useOMGJIT=false --useWasmSIMD=true --useWasmRelaxedSIMD=true`. |
| Wasm BBQ SIMD stress | done | Remote BBQ-only filtered SIMD stress passed 60/60. Excluded cases were non-target/invalid for this run: `simd-tiny-loop.js` is marked skip/currently broken, `omg-simd-stress.js` is an OMG-specific test with missing local `.wat`, two tests require generated/copied `.wasm` resources, and `simd-multimemory.js` requires unsupported multimemory enablement. |
| FTL/B3/Air scalar and Float16 smoke | partial | Remote `jsc-loong64-ftl-f16review3` passes targeted FTL Float16Array/DataView Float16, arithmetic, and select/branch smoke with low FTL thresholds. Loong64 Float16 scalar helpers are enabled through LSX `vfcvt.h.s`, `vfcvtl.s.h`, `vldrepl.h`, and `vstelm.h`. |
| Wasm OMG SIMD shuffle smoke | partial | Constant `simd.shuffle` now uses the Loong64 3-child `VectorSwizzle`/`VectorSwizzle2` path instead of the previous ARM64-only gate; remote OMG shuffle smoke passes. `VectorTranspose*` and `VectorMulByElement` remain ARM64-only because Loong64 does not currently lower them with equivalent single-instruction semantics and Loong64 strength-reduction does not generate them. |
| Wasm OMG tail-call smoke | partial | Loong64 OMG tail-call setup now restores return PC into the link register like ARM/RISCV. Remote `omg-tail-call-clobber-scratch-register.js`, `omg-tail-call-clobber-scratch-register-2.js` with file header options, `omg-tail-call-clobber-pinned-registers.js`, `tail-call-simple.js` as module, and `simd-tail-call-simple.js` as module pass. |
| Wasm OMG/IPInt stack conversion regression | done | Fixed Loong64 offlineasm float/double-to-unsigned lowering to reserve 8-byte temp slots before `st.d`; this removed the `omg-osr-stack-slot-positioning.js` stack corruption where a stale f32 bit pattern was later consumed as a ref. Remote BBQ-only and OMG+OSR repros now pass. |
| JSPI Pinball continuation | done | Fixed Loong64 JSPI `PinballHandlerContext::arguments` save/restore to use 64-bit FPR slots (`stored`/`loadd`) instead of 128-bit vector slots. Remote `jspi-basic.js`, `jspi-exceptions-from-js.js`, `jspi-resuspension.js`, and `jspi-stack-traces.js` now pass. |
| Wasm OMG broad list | partial | Remote `tools/wasm-omg.txt` with OMG+SIMD passed 530/578 before the JSPI fix. Remaining observed failures were harness/feature/resource/skip categories: missing `.wasm`/`.bin`, `$vm`/`promise_test` harness, unsupported Memory64/multimemory/wide arithmetic gates, two `//@ skip` timeout tests, and JSPI crashes fixed afterwards. |
