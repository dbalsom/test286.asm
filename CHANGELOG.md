# CHANGELOG

## 1.2.0 - 2026/11/09

 - (POST `1D`, `1E`, `1F` and `23`): Added real-mode flag, descriptor-table 
   register, `XLAT` and software-interrupt tests adapted from [test386 PR #23](https://github.com/barotto/test386.asm/pull/23)
 - (POST `20`): Added checks for saved/live `IF` and privilege-dependent `IRET`
   flag restoration in protected mode
 - (POST `12`): Extended limit checks with `DS`/`ES` expand-up and expand-down
   segment-limit tests, including limits `0000h` and `FFFFh`, word accesses at
   `FFFFh`, and memory preservation after faulting writes
 - (POST `04` and `10`): Added zero-count `REP` and `REPE`/`REPNE` 
   early-termination checks for byte/word strings in both directions, in real
   and protected mode
 - (POST `1C`): Added register/memory `LAR`/`LSL` checks, including null and
   out-of-table selectors, descriptor types, privilege checks, not-present
   descriptors, limits `0000h`/`FFFFh`, and unchanged destinations on selector
   check failure
 - (POST `08`): Added LMSW checks that `PE` remains set
 - (POST `22`): Added busy-TSS LTR `#GP(selector)` checks and explicit `CLTS`
   readback to verify that `TS` clears

## 1.1.0 - 2026/11/07

 - (POST `07`): Add optional `RESET_ON_TRIPLE_FAULT` tests for return to real 
   mode and preservation of general registers.

## 1.0.0 - 2026/11/05
 - Initial 286 real/protected-mode port

