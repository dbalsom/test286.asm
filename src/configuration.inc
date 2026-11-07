; SPDX-License-Identifier: GPL-3.0-or-later

; CONFIGURATION
; POST and ASCII are write-only ports; no external device initialization.

; The diagnostic port used to emit the current test procedure number.
; Possible values: the 16-bit value of the diagnostic port of your system.
%ifndef POST_PORT
%define POST_PORT 0x190
%endif

; Additional port for direct ASCII output.
; Possible values: any 16-bit value, 0=disabled.
%ifndef OUT_PORT
%define OUT_PORT 0x191
%endif

%ifndef TRACE_ARITH
%define TRACE_ARITH 1
%endif

; Intentionally cause a test failure (for harness validation)
%ifndef INJECT_FAILURE
%define INJECT_FAILURE 0
%endif

; Enable only when the system responds to CPU shutdown by asserting RESET
; while retaining RAM. Tests all eight general registers across two RESETs.
%ifndef RESET_ON_TRIPLE_FAULT
%define RESET_ON_TRIPLE_FAULT 0
%endif
