; SPDX-License-Identifier: GPL-3.0-or-later

; 16-bit task execution path. Register-load checks retain the corresponding
; TSS286entrypoint checks in test386; 386 upper-half checks are removed.

TSS286entrypoint:
    cmp     sp, 0x1122
    jne     error
    cmp     ax, 0x1234
    jne     error
    cmp     cx, 0x5678
    jne     error
    cmp     dx, 0x9ABC
    jne     error
    cmp     bx, 0xDEF0
    jne     error
    cmp     bp, 0x3344
    jne     error
    cmp     si, 0x5566
    jne     error
    cmp     di, 0x7788
    jne     error
    mov     ax, cs
    cmp     ax, CU_SEG_PROT16|3
    jne     error
    mov     ax, ss
    cmp     ax, SU_TASK_PROT16|3
    jne     error
    mov     ax, ds
    cmp     ax, DU_SEG_PROT|3
    jne     error
    mov     ax, es
    cmp     ax, DU_SEG_PROT|3
    jne     error
    str     ax
    cmp     ax, TSS_PROT16
    jne     error
    sldt    ax
    cmp     ax, LDT_SEG_PROT
    jne     error
    pushf
    pop     ax
    test    ax, PS_NT
    jz      error
    smsw    ax
    test    ax, 8
    jz      error
    mov     word [0x100], 0x2861
    iret

; 286 addition: a non-nested JMP switch must leave NT clear.
TSS286jumpEntry:
    pushf
    pop     ax
    test    ax, PS_NT
    jnz     error
    mov     word [0x100], 0x2862
    jmp     TSS_PROT:0
