; SPDX-License-Identifier: GPL-3.0-or-later
;
; Tests GDTR/IDTR loading and storing, adapted from test386's dtr_m.asm.
; The 286 has a 16-bit limit and 24-bit base. Operand byte 5 is unused;
; we vary it on input, but do not impose a value on the byte stored there.
; Requires real mode, a valid stack and writable DS:0000-0015.
;

; Tests one descriptor-table-register pattern.
; %1: load instruction (LGDT or LIDT).
; %2: store instruction (SGDT or SIDT).
; %3: base pattern, including the ignored upper byte.
; %4: limit.
; %5: error label.
; DS:0010 holds the original register, saved by testDTR.
%macro testDTRpattern 5
    ; Load input: a word limit followed by a three-byte base and unused byte.
    mov     word [0], %4
    mov     word [2], %3 & 0xFFFF
    mov     word [4], (%3 >> 16) & 0xFFFF
    ; Initialize output storage and its neighboring guards.
    mov     word [6], 0xBEEF
    mov     word [8], 0xDEAD
    mov     word [0xA], 0xDEAD
    mov     word [0xC], 0xDEAD
    mov     word [0xE], 0xBEEF
    %1      [0]
    %2      [8]
    ; Restore the original table before any assertion can report failure.
    %1      [0x10]
    cmp     word [8], %4                ; Only 16 limit bits are stored
    jne     %5
    cmp     word [0xA], %3 & 0xFFFF      ; Base bits 15-0
    jne     %5
    cmp     byte [0xC], (%3 >> 16) & 0xFF ; Base bits 23-16
    jne     %5
    cmp     word [6], 0xBEEF
    jne     %5
    cmp     word [0xE], 0xBEEF
    jne     %5
%endmacro

; Tests four patterns and restores the original table register and FLAGS.
; %1: load instruction.
; %2: store instruction.
; Uses DS:0000-0015 as scratch; leaves general registers unchanged.
%macro testDTR 2
    pushf
    cli                                 ; IDTR temporarily names invalid tables
    %2      [0x10]
    testDTRpattern %1, %2, 0, 0, error
    testDTRpattern %1, %2, 0x12345678, 0xAABB, error
    testDTRpattern %1, %2, 0xCCDDEEFF, 0x1122, error
    testDTRpattern %1, %2, 0xFFFFFFFF, 0xEEEE, error
    popf
%endmacro
