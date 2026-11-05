; SPDX-License-Identifier: GPL-3.0-or-later

; Tests LDS and LES with 16:16 pointers in real mode.
; %1: segment register name, ds or es.
; ES:DI: writable memory address to use for the four-byte pointer.
;
; The fixture contains ABCDh:1234h. Checks the loaded segment and BX offset,
; then restores ES and the segment register under test.
; Uses: AX, BX, CX, DX, Flags; overwrites four bytes at the original ES:DI.
;
%macro testLoadPtr 1
    mov     cx, %1
    mov     dx, es

    mov     [es:di], word 0x1234
    mov     [es:di + 2], word 0xabcd
    l%1     bx, [es:di]
    mov     ax, %1
    cmp     ax, 0xabcd
    jne     error
    cmp     bx, 0x1234
    jne     error

    mov     es, dx

    mov     %1, cx
%endmacro
