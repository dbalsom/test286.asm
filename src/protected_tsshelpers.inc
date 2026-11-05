; SPDX-License-Identifier: GPL-3.0-or-later

; 16-bit implementations of test386's task-state validation helpers.
; AX selects the GDT descriptor or TSS data alias; DX is the expected value.
validateTSSbusy:
    push    ds
    lds     bx, [cs:ptrGDTprot]
    add     bx, ax
    mov     al, [bx+5]
    and     al, 2
    shl     dl, 1
    cmp     al, dl
    jne     error
    pop     ds
    ret

validateTSSbacklink:
    push    ds
    mov     ds, ax
    cmp     [0], dx
    jne     error
    pop     ds
    ret
