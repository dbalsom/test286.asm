; SPDX-License-Identifier: GPL-3.0-or-later
;
;   Tests Call near by displacement and register indirect
;   Stack must be initialized.
;   %1: stack pointer register, sp
;   Uses: AX, BX, Flags
;
%macro testCallNear 1
    %define spcmp ax
    mov     spcmp, %1

%%rel16:
    clc
    call    word %%nearfn16
    jnc     error
    jmp     %%rm16
%%nearfn16:
    sub     spcmp, 2
    cmp     %1, spcmp
    jne     error
    add     spcmp, 2
    stc
    ret
    jmp     error

%%rm16:
    clc
    mov     bx, %%nearfn16
    call    bx
    jnc     error
%endmacro

;
;   Tests Call far by immediate and memory pointers
;   Stack must be initialized
;   %1: code segment
;   Uses: AX, Flags, DS:SI as scratch memory
;
%macro testCallFar 1
    mov     ax, sp

    clc
    call    word %1:%%farfn16
    jnc     error
    jmp     %%m1616
%%farfn16:
    sub     ax, 4
    cmp     sp, ax
    jne     error
    add     ax, 4
    stc
    retf
    jmp     error

%%m1616:
    clc
    mov     [si], word %%farfn16
    mov     [si+2], word %1
    call    far [si]
    jnc     error
%%exit:
%endmacro
