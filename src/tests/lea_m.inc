; SPDX-License-Identifier: GPL-3.0-or-later

; Executes LEA with 16-bit addressing and compares the result with a given value.
; %1: address expression to calculate.
; %2: expected effective offset.
;
; Uses: Flags. AX is preserved; the stack must be initialized.
;
%macro testLEA16 2
    push    ax
    lea     ax, %1
    cmp     ax, %2
    jne     error
    pop     ax
%endmacro
