; SPDX-License-Identifier: GPL-3.0-or-later

; Tests LOOP, LOOPZ and LOOPNZ, retaining test386 16-bit cases.

; Tests LOOP with 16-bit addressing.
;
; Starts CX at zero, so decrementing it wraps and the body runs 65536 times.
; Both the AX iteration counter and CX must finish at zero.
; Uses: AX, CX, Flags.
;
%macro testLoop 0

    mov     cx, 0
    mov     ax, 0
%%loop16:
    inc     ax
    loop    %%loop16
    cmp     ax, 0
    jne     error
    cmp     cx, 0
    jne     error

%endmacro

; Tests LOOPZ/LOOPE with 16-bit addressing.
;
; Checks both termination when ZF becomes zero and termination when CX reaches
; zero while ZF remains set. Uses: AX, CX, Flags.
;
%macro testLoopZ 0

    mov     cx, 0xFFFF
    mov     ax, 0
%%loop16a:
    inc     ax
    cmp     ah, 0
    loopz   %%loop16a
    cmp     ax, 0x0100
    jne     error
    cmp     cx, 0xFEFF
    jne     error

    mov     cx, 0x00FF
    mov     ax, 0
%%loop16b:
    inc     ax
    cmp     ah, 0
    loopz   %%loop16b
    cmp     ax, 0x00FF
    jne     error
    cmp     cx, 0
    jne     error

%endmacro

; Tests LOOPNZ/LOOPNE with 16-bit addressing.
;
; Checks both termination when ZF becomes one and termination when CX reaches
; zero while ZF remains clear. Uses: AX, CX, Flags.
;
%macro testLoopNZ 0

    mov     cx, 0xFFFF
    mov     ax, 0
%%loop16a:
    inc     ax
    test    al, 0xFF
    loopnz  %%loop16a
    cmp     ax, 0x0100
    jne     error
    cmp     cx, 0xFEFF
    jne     error

    mov     cx, 0x00FF
    mov     ax, 0
%%loop16b:
    inc     ax
    test    al, 0xFF
    loopnz  %%loop16b
    cmp     ax, 0x00FF
    jne     error
    cmp     cx, 0
    jne     error

%endmacro
