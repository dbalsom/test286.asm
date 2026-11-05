; SPDX-License-Identifier: GPL-3.0-or-later
;
; Tests for VERR and VERW instructions
;
; DX = segment selector
; BL = expected ZF value on bit 6, other bits to 0
;
testVERRp:
    mov     ah, bl
    not     ah
    sahf
    verr    dx
    jmp     testVERresult
testVERWp:
    mov     ah, bl
    not     ah
    sahf
    verw    dx
testVERresult:
    lahf
    and     ah, 0x40
    cmp     bl, ah
    jne     error
    ret

; Tests whether a segment is readable with VERR.
;
; %1: segment selector.
; %2: expected ZF result, 0 or 1.
;
; The helper initializes ZF opposite to the expected result before VERR,
; then checks the flag through LAHF. Uses: AH, BL, DX, Flags.
; The stack must be initialized.
;
%macro testVERR 2
    mov     dx, %1
    mov     bl, ((%2 & 1) << 6)
    call    testVERRp
%endmacro

; Tests whether a segment is writable with VERW.
;
; %1: segment selector.
; %2: expected ZF result, 0 or 1.
;
; The helper initializes ZF opposite to the expected result before VERW,
; then checks the flag through LAHF. Uses: AH, BX, DX, Flags.
; The stack must be initialized.
;
%macro testVERW 2
    mov     dx, %1
    mov     bx, ((%2 & 1) << 6)
    call    testVERWp
%endmacro
