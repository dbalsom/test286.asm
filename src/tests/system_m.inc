; SPDX-License-Identifier: GPL-3.0-or-later
;
; System-register checks at CPL 0 with a valid protected-mode stack.
;

; LMSW cannot clear an already-set PE bit. Check both register and memory
; operands while retaining the other MSW bits read at entry.
; Uses: AX, DX, Flags. BP and the stack pointer are preserved.
%macro testLMSWStickyPE 0
    smsw    dx
    test    dx, CR0_MSW_PE
    jz      error                       ; This test must start in protected mode
    mov     ax, dx
    and     ax, ~CR0_MSW_PE
    lmsw    ax
    smsw    ax
    xor     ax, dx
    and     ax, 0x000F                  ; Compare the four defined MSW bits
    jnz     error
    push    bp
    mov     bp, sp
    push    dx
    and     word [ss:bp-2], ~CR0_MSW_PE
    lmsw    [ss:bp-2]
    smsw    ax
    xor     ax, dx
    and     ax, 0x000F
    jnz     error
    mov     sp, bp
    pop     bp
%endmacro

; Loading the current busy TSS must raise #GP(selector), without changing TR
; or clearing its busy bit. The existing fault helper also checks saved CS:IP.
; Uses: AX, BX, Flags.
%macro testLTRBusy286 0
    str     ax
    cmp     ax, TSS_PROT
    jne     error
    mov     ax, TSS_PROT
    protModeFaultTest EX_GP, TSS_PROT, ltr ax
    str     ax
    cmp     ax, TSS_PROT
    jne     error
    validateTSSbusy286 TSS_PROT, 1
%endmacro
