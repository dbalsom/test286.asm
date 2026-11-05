; SPDX-License-Identifier: GPL-3.0-or-later

; LEAVE with 16-bit stack addressing and operand size.
;
; SP <- BP;
; BP <- Pop16();
;
; %1: stack address size, 16.
;
; Loads SS with D_SEG_PROT16 and builds a frame whose saved BP is 5678h.
; Checks that LEAVE restores BP and leaves SP at 6. The caller must restore
; its normal stack afterward.
;
%macro testLEAVE 1
    mov     ax, D_SEG_PROT16
    %assign spResult 6
    %assign bpResult 0x5678
    mov     ss, ax
    mov     sp, 8
    push    word 0x1234
    push    word 0x5678
    mov     bp, sp
    mov     sp, 0
    leave
    cmp     sp, spResult
    jne     error
    cmp     bp, bpResult
    jne     error
%endmacro

