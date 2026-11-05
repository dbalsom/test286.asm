; SPDX-License-Identifier: GPL-3.0-or-later

; Procedures for the protected-mode code segment.

initIntGateProt:
    initIntGate286
    ret

initIntTaskGateProt:
    initIntTaskGate
    ret

initDescriptorProt:
    initDescriptor
    ret

; Defines a 286 call gate: ES:BX table, AX selector, SI destination selector,
; DI offset, DL word count, DH access privilege. ES replaces test386's FS.
initCallGate286:
    and     ax, 0xFFF8
    add     bx, ax
    mov     [es:bx], di
    mov     [es:bx+2], si
    or      dx, ACC_TYPE_GATE286_CALL | ACC_PRESENT
    mov     [es:bx+4], dx
    mov     word [es:bx+6], 0
    ret
