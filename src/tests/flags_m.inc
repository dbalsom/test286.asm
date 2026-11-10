; SPDX-License-Identifier: GPL-3.0-or-later
;
; Flag-instruction tests adapted from test386's tests/flags_m.asm.
; A valid stack is required. Uses: AX; restores the caller's FLAGS.
;

; Checks the complete real-mode FLAGS word without changing the test flags.
; %1: expected FLAGS, including fixed bit 1.
%macro checkTestFlags 1
    pushf
    pushf
    pop     ax
    cmp     ax, %1
    jne     error
    popf
%endmacro

; Tests the carry flag instructions, including both CMC transitions.
; Other arithmetic flags start set so accidental clearing is detected.
%macro testCarryFlagInstructions 0
    pushf
    push    word (PS_ARITH & ~PS_CF)|2
    popf
    checkTestFlags (PS_ARITH & ~PS_CF)|2
    cmc                                 ; Set carry by complementing it
    checkTestFlags PS_ARITH|2
    cmc                                 ; Clear carry by complementing it
    checkTestFlags (PS_ARITH & ~PS_CF)|2
    stc
    checkTestFlags PS_ARITH|2
    clc
    checkTestFlags (PS_ARITH & ~PS_CF)|2
    popf
%endmacro

; Tests the direction flag instructions and preservation of other flags.
%macro testDirectionFlagInstructions 0
    pushf
    push    word PS_ARITH|2
    popf
    checkTestFlags PS_ARITH|2
    std                                 ; Set the direction flag
    checkTestFlags PS_ARITH|PS_DF|2
    cld                                 ; Clear the direction flag
    checkTestFlags PS_ARITH|2
    popf
%endmacro

; Tests the interrupt flag instructions and preservation of other flags.
; No external interrupt source is required.
%macro testInterruptFlagInstructions 0
    pushf
    push    word PS_ARITH|2
    popf
    checkTestFlags PS_ARITH|2
    sti                                 ; Set the interrupt flag
    checkTestFlags PS_ARITH|PS_IF|2
    cli                                 ; Clear the interrupt flag
    checkTestFlags PS_ARITH|2
    popf
%endmacro
