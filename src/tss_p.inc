; SPDX-License-Identifier: GPL-3.0-or-later

; TSS initialization procedures, retaining the test386 16-bit image layout.

clearTSS:
    les     di, [cs:ptrTSSprot]
    xor     ax, ax
    mov     cx, 22
    cld
    rep stosw
    ; LDTR is loaded from, but not saved to, a 286 TSS on a task switch.
    ; The root task must have its LDT selector before its first task return.
    mov     word [es:0x2A], LDT_SEG_PROT
    ret

initTSS16:
    push    ax
    push    bp
    push    ds
    push    di
    les     di, [cs:ptrTSSprot16]
    xor     ax, ax
    mov     cx, 22
    cld
    rep stosw
    les     bp, [cs:ptrTSSprot16]
    mov     word [es:bp+0], 0
    mov     word [es:bp+2], SP_R0_PROT  ; SP0 precedes SS0 in a 286 TSS
    mov     word [es:bp+4], S_SEG_PROT16
    mov     word [es:bp+0x0E], TSS286entrypoint
    mov     word [es:bp+0x10], 2
    mov     word [es:bp+0x12], 0x1234
    mov     word [es:bp+0x14], 0x5678
    mov     word [es:bp+0x16], 0x9ABC
    mov     word [es:bp+0x18], 0xDEF0
    mov     word [es:bp+0x1A], 0x1122
    mov     word [es:bp+0x1C], 0x3344
    mov     word [es:bp+0x1E], 0x5566
    mov     word [es:bp+0x20], 0x7788
    mov     word [es:bp+0x22], DU_SEG_PROT|3
    mov     word [es:bp+0x24], CU_SEG_PROT16|3
    mov     word [es:bp+0x26], SU_TASK_PROT16|3
    mov     word [es:bp+0x28], DU_SEG_PROT|3
    mov     word [es:bp+0x2A], LDT_SEG_PROT
    pop     di
    pop     ds
    pop     bp
    pop     ax
    ret
