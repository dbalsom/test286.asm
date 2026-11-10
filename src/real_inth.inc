; SPDX-License-Identifier: GPL-3.0-or-later
;
; Real-mode interrupt handler adapted from test386's protected_tssinth.asm.
; AX holds the pre-INT SP. DI counts entries, so an ignored INT cannot pass.
; The caller sets IF and CF, with the other writable flags clear.
;
realmodeInterrupt:
    push    bp
    mov     bp, sp
    push    bx
    pushf                               ; Inspect live FLAGS before changing them
    pop     bx
    cmp     bx, PS_CF|2                 ; INT cleared IF and retained CF
    jne     error
    mov     bx, ax
    sub     bx, 6+4                     ; Six-byte frame + saved BP and BX
    cmp     sp, bx
    jne     error
    mov     bx, ss
    cmp     bx, S_SEG_REAL
    jne     error
    mov     bx, cs
    cmp     bx, C_SEG_REAL
    jne     error
    cmp     word [ss:bp+2], realmodeInterruptRET
    jne     error                       ; Saved IP points after INT
    cmp     word [ss:bp+4], C_SEG_REAL
    jne     error                       ; Saved CS identifies the caller
    cmp     word [ss:bp+6], PS_IF|PS_CF|2
    jne     error                       ; Saved FLAGS retain the original IF
    inc     di
    pop     bx
    pop     bp
    iret
