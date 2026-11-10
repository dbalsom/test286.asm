; SPDX-License-Identifier: GPL-3.0-or-later

; Interrupt-entry/frame checks retained from test386, adapted to 286 gates,
; TSS offsets and 16-bit stack addressing.

; Saving BP before inspecting a frame places IP at SS:BP+2, CS at SS:BP+4,
; and FLAGS at SS:BP+6. A privilege change also saves SP and SS at +8 and +10.

; Kernel mode interrupt handler called from user mode through a 286 gate.
; Checks the change from CPL 3 to CPL 0, the stack loaded from TSS SS0:SP0,
; and the ten-byte return frame, including the saved user SS:SP.
;
kernelInterrupt286:
    testCPL 0                           ; Verify entry at CPL 0
    push    bx
    push    cx
    push    ds
    lds     bx, [cs:ptrTSSprot]         ; Access the 286 TSS data alias
    mov     cx, [bx+2]                  ; Get SP0 at TSS offset 2
    sub     cx, 0x0A+6                  ; Ten-byte frame + saved BX, CX and DS
    cmp     sp, cx                      ; Did the stack decrease correctly?
    jne     error
    mov     cx, ss
    cmp     cx, word [bx+4]             ; Does SS match SS0 at TSS offset 4?
    jne     error
    pop     ds
    pop     cx
    mov     bx, cs
    cmp     bx, C_SEG_PROT16            ; Did we enter the kernel code segment?
    jne     error
    pop     bx

    push    bp
    mov     bp, sp
    cmp     word [ss:bp+2], kernelModeInterruptReturn286
    jne     error                       ; Invalid return IP
    cmp     word [ss:bp+4], CU_SEG_PROT16|3
    jne     error                       ; Invalid return CS
    test    word [ss:bp+6], PS_IF
    jz      error                       ; Caller entered with IF set
    push    ax
    pushf
    pop     ax
    test    ax, PS_IF
    jnz     error                       ; Interrupt gate cleared live IF
    pop     ax
    cmp     word [ss:bp+8], SP_R3_PROT
    jne     error                       ; Invalid return SP
    cmp     word [ss:bp+10], SU_SEG_PROT16|3
    jne     error                       ; Invalid return SS
    pop     bp
    iret                                ; Return to user mode at CPL 3

; Kernel mode interrupt handler called from kernel mode.
; AX holds the caller's SP before INT. Checks that CPL remains 0 and that
; the six-byte return frame is on the existing kernel stack.
;
kernelOnlyInterrupt:
    push    ax
    testCPL 0                           ; Verify CPL remains 0
    pop     ax
    push    bx
    push    ds
    push    cx
    lds     bx, [cs:ptrTSSprot]         ; Access the 286 TSS data alias
    mov     cx, ax                      ; Caller's SP before INT
    sub     cx, 6+6                     ; Six-byte frame + saved BX, DS and CX
    cmp     sp, cx                      ; Did the stack decrease correctly?
    jne     error
    mov     cx, ss
    mov     bx, word [bx+4]             ; Expected kernel SS: TSS SS0
    cmp     cx, bx                      ; Did we keep the kernel stack segment?
    jne     error
    pop     cx
    mov     bx, cs
    cmp     bx, C_SEG_PROT16            ; Did we enter the kernel code segment?
    jne     error
    pop     ds
    pop     bx

    push    bp
    mov     bp, sp
    cmp     word [ss:bp+2], kernelModeOnlyInterruptReturn
    jne     error                       ; Invalid return IP
    cmp     word [ss:bp+4],C_SEG_PROT16
    jne     error                       ; Invalid return CS
    test    word [ss:bp+6], PS_IF
    jz      error                       ; Caller entered with IF set
    push    ax
    pushf
    pop     ax
    test    ax, PS_IF
    jnz     error                       ; Interrupt gate cleared live IF
    pop     ax
    pop     bp
    iret                                ; Return to the caller at CPL 0

; Conforming kernel code interrupt handler called from user mode.
; Checks that CPL remains 3 and the six-byte return frame stays on the user
; stack, even though the destination code segment has DPL 0.
;
kernelConformingInterrupt:
    testCPL 3                           ; Conforming code preserves CPL 3
    push    bx
    push    cx
    mov     cx, SP_R3_PROT              ; Caller's SP before INT
    sub     cx, 6+4                     ; Six-byte frame + saved BX and CX
    cmp     sp, cx                      ; Did the user stack decrease correctly?
    jne     error
    mov     cx, ss
    mov     bx, SU_SEG_PROT16|3         ; Expected user stack selector
    cmp     cx, bx                      ; Did we keep the user stack segment?
    jne     error
    pop     cx
    mov     bx, cs
    cmp     bx, CC_SEG_PROT16|3         ; Conforming code selector with RPL 3
    jne     error
    pop     bx

    push    bp
    mov     bp, sp
    cmp     word [ss:bp+2], kernelConformingInterruptReturn
    jne     error                       ; Invalid return IP
    cmp     word [ss:bp+4], CU_SEG_PROT16|3
    jne     error                       ; Invalid return CS
    test    word [ss:bp+6], PS_IF
    jz      error                       ; Caller entered with IF set
    push    ax
    pushf
    pop     ax
    test    ax, PS_IF
    jnz     error                       ; Interrupt gate cleared live IF
    pop     ax
    pop     bp
    iret                                ; Return to the caller at CPL 3

; Conforming kernel code interrupt handler called from kernel mode.
; AX holds the caller's SP before INT. Checks that CPL remains 0 and that
; the six-byte return frame is on the existing kernel stack.
;
kernelOnlyConformingInterrupt:
    push    ax
    testCPL 0                           ; Verify CPL remains 0
    pop     ax
    push    bx
    push    ds
    push    cx
    lds     bx, [cs:ptrTSSprot]         ; Access the 286 TSS data alias
    mov     cx, ax                      ; Caller's SP before INT
    sub     cx, 6+6                     ; Six-byte frame + saved BX, DS and CX
    cmp     sp, cx                      ; Did the stack decrease correctly?
    jne     error
    mov     cx, ss
    mov     bx, word [bx+4]             ; Expected kernel SS: TSS SS0
    cmp     cx, bx                      ; Did we keep the kernel stack segment?
    jne     error
    pop     cx
    mov     bx, cs
    cmp     bx, CC_SEG_PROT16           ; Conforming code selector with RPL 0
    jne     error
    pop     ds
    pop     bx

    push    bp
    mov     bp, sp
    cmp     word [ss:bp+2], kernelOnlyConformingInterruptReturn
    jne     error                       ; Invalid return IP
    cmp     word [ss:bp+4],C_SEG_PROT16
    jne     error                       ; Invalid return CS
    test    word [ss:bp+6], PS_IF
    jz      error                       ; Caller entered with IF set
    push    ax
    pushf
    pop     ax
    test    ax, PS_IF
    jnz     error                       ; Interrupt gate cleared live IF
    pop     ax
    pop     bp
    iret                                ; Return to the caller at CPL 0

; User mode interrupt handler called from user mode.
; Checks that CPL remains 3, the user code and stack selectors are retained,
; and the six-byte return frame identifies the caller after INT.
;
userModeInterrupt:
    testCPL 3                           ; Verify CPL remains 3
    push    bx
    push    cx
    mov     cx, SP_R3_PROT              ; Caller's SP before INT
    sub     cx, 6+4                     ; Six-byte frame + saved BX and CX
    cmp     sp, cx                      ; Did the user stack decrease correctly?
    jne     error
    mov     cx, ss
    mov     bx, SU_SEG_PROT16|3         ; Expected user stack selector
    cmp     cx, bx                      ; Did we keep the user stack segment?
    jne     error
    pop     cx
    mov     bx, cs
    cmp     bx, CU_SEG_PROT16|3         ; Did we enter the user code segment?
    jne     error
    pop     bx

    push    bp
    mov     bp, sp
    cmp     word [ss:bp+2], userInterruptReturn
    jne     error                       ; Invalid return IP
    cmp     word [ss:bp+4],CU_SEG_PROT16|3
    jne     error                       ; Invalid return CS
    test    word [ss:bp+6], PS_IF
    jz      error                       ; Caller entered with IF set
    push    ax
    pushf
    pop     ax
    test    ax, PS_IF
    jnz     error                       ; Interrupt gate cleared live IF
    pop     ax
    pop     bp
    iret                                ; Return to the caller at CPL 3
