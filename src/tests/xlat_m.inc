; SPDX-License-Identifier: GPL-3.0-or-later
;
; Real-mode XLAT test adapted from test386's systembiosexpansionarea.asm.
; Tests all 256 indices against a table of negated byte values.
;
; %1: table offset in DS; the complete table must fit within the segment.
; %2: initial FLAGS, including fixed bit 1 (TF and IF must be clear).
; Uses: AX, BX, CX, DX, SI, Flags and the 256-byte table at DS:%1.
%macro testXlat 2
    mov     cx, 0x100                   ; Length of the lookup table
    mov     al, 0                       ; First entry
    mov     bx, %1                      ; Initialize the table pointer
%%fill:
    neg     al                          ; Store negated entries
    mov     [bx], al
    inc     bx
    neg     al                          ; Restore the original entry number
    inc     al
    loop    %%fill

    mov     cx, 0x100
    mov     bx, %1                      ; XLAT adds AL to this base
%%check:
    mov     ax, 0x100
    sub     ax, cx                      ; Entry number to validate
    mov     dl, al
    neg     dl                          ; Expected translated byte
    mov     ah, 0x5A                    ; AH must not be part of the index
    push    word %2
    popf
    xlat
    pushf                               ; Capture FLAGS before comparisons
    pop     si
    cmp     si, %2
    jne     error
    cmp     al, dl
    jne     error
    cmp     ah, 0x5A
    jne     error
    cmp     bx, %1
    jne     error
    loop    %%check
%endmacro
