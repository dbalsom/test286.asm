; SPDX-License-Identifier: GPL-3.0-or-later

; Ring-switching procedures retained from test386. All frames and TSS fields
; are 16-bit; ES replaces FS

; Switches from ring 0 to ring 3. Registers and FLAGS are scratch.
switchToRing3:
    testCPL 0
    pop     dx
    mov     ax, ds
    lds     bx, [cs:ptrTSSprot]
    mov     [bx+0x28], ax               ; saved kernel DS
    mov     ax, es
    mov     [bx+0x22], ax               ; saved kernel ES
    mov     [bx+2], sp                  ; TSS SP0
    mov     ax, ss
    mov     [bx+4], ax                  ; TSS SS0
    cli
    push    word SU_SEG_PROT16|3
    push    word SP_R3_PROT
    pushf
    mov     bp, sp
    or      word [ss:bp], 0x200
    push    word CU_SEG_PROT16|3
    push    dx
    iret

; Switches back through the same writable call-gate mechanism as test386.
switchToRing0:
    testCPL 3
    pop     cx
    les     bx, [cs:ptrGDTUprot]
    mov     ax, RING0_GATE
    mov     si, C_SEG_PROT16
    mov     di, .ring0
    mov     dx, ACC_DPL_3
    call    initCallGate286
    call    RING0_GATE|3:0
.ring0:
    add     sp, 8                       ; IP, CS, old SP, old SS
    lds     bx, [cs:ptrTSSprot]
    mov     ax, [bx+0x22]
    mov     es, ax
    mov     ax, [bx+0x28]
    mov     ds, ax
    push    cx
    ret

; Retained 16-bit half of test386's parameter-copying call-gate test.
testCallGateWithParameters:
    push    word switchToRing0_2
    jmp     switchToRing3

switchToRing0_2:
    testCPL 3
    mov     cx, ring0_2TestEndLocation
    les     bx, [cs:ptrGDTUprot]
    mov     ax, RING0_GATE2
    mov     si, C_SEG_PROT16
    mov     di, .ring0_3
    mov     dx, ACC_DPL_3|0xA
    call    initCallGate286
    push    cx
    push    word 0x1234
    push    word 0x5678
    push    word 0x9ABC
    push    word 0xDEF0
    push    word 0x1122
    push    word 0x3344
    push    word 0x5566
    push    word 0x7788
    push    word 0x99AA
    push    word 0xBBCC
    call    RING0_GATE2|3:0
    pop     cx
    call    RING0_GATE|3:0
.ring0_3:
    push    ds
    push    bp
    push    bx
    lds     bp, [cs:ptrTSSprot]
    mov     bx, [ds:bp+2]
    sub     bx, 8+6+20
    cmp     sp, bx
    jne     error
    mov     bx, [ds:bp+4]
    mov     bp, ss
    cmp     bx, bp
    jne     error
    pop     bx
    pop     bp
    pop     ds
    testCPL 0
    push    bp
    mov     bp, sp
    cmp     word [ss:bp+0x18], 0x1234
    jne     error
    cmp     word [ss:bp+0x16], 0x5678
    jne     error
    cmp     word [ss:bp+0x14], 0x9ABC
    jne     error
    cmp     word [ss:bp+0x12], 0xDEF0
    jne     error
    cmp     word [ss:bp+0x10], 0x1122
    jne     error
    cmp     word [ss:bp+0x0E], 0x3344
    jne     error
    cmp     word [ss:bp+0x0C], 0x5566
    jne     error
    cmp     word [ss:bp+0x0A], 0x7788
    jne     error
    cmp     word [ss:bp+0x08], 0x99AA
    jne     error
    cmp     word [ss:bp+0x06], 0xBBCC
    jne     error
    pop     bp
    retf    20
