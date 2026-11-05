; SPDX-License-Identifier: GPL-3.0-or-later
;
; Advances the base address of data segments used by tests, D1_SEG_REAL and
; D2_SEG_REAL.
;
; Loads DS with D1_SEG_REAL and ES with D2_SEG_REAL.
;
%macro advTestSegReal 0
    advTestBase
    %assign D1_SEG_REAL TEST_BASE1 >> 4
    %assign D2_SEG_REAL TEST_BASE2 >> 4
    mov     dx, D1_SEG_REAL
    mov     ds, dx
    mov     dx, D2_SEG_REAL
    mov     es, dx
%endmacro

; Initialises the real-mode IDT with C_SEG_REAL:error.
;
; Installs vectors 0 through 16, each as a 16-bit offset and segment.
; Uses: AX, BX, CX, DS, Flags; leaves DS zero.
;
%macro initRealModeIDT 0
    xor     ax, ax
    mov     ds, ax
    xor     bx, bx
    mov     cx, 17
%%loop:
    mov     [bx], word error
    mov     [bx+2], word C_SEG_REAL
    add     bx, 4
    loop    %%loop
%endmacro


;
; Exception handling testing in real mode
;

; Initialises an exc handler
; %1: vector
; %2: handler IP
; Trashes AX,DS
%macro realModeExcInit 2
    mov     ax, 0
    mov     ds, ax
    mov     [%1*4], word %2
    mov     [%1*4+2], word C_SEG_REAL
%endmacro

; Checks an exception result and restores the default handler.
;
; %1: vector.
; %2: expected pushed value of IP.
;
; Checks SP_REAL-6 and the saved CS:IP at SS:SP_REAL-6, then restores
; C_SEG_REAL:error in the IVT. The six-byte interrupt frame remains on the
; stack; this macro neither removes it nor executes IRET.
; Trashes AX, DS, Flags.
;
%macro realModeExcCheck 2
    cmp     sp, SP_REAL-6
    jne     error
    cmp     [ss:SP_REAL-4], word C_SEG_REAL
    jne     error
    cmp     [ss:SP_REAL-6], word %2
    jne     error
    mov     ax, 0
    mov     ds, ax
    mov     [%1*4], word error
    mov     [%1*4+2], word C_SEG_REAL
%endmacro

; Tests a fault.
;
; %1: vector.
; %2: instruction to execute that causes a fault. The fault must identify
;     this instruction, rather than an instruction inside a called procedure.
;
; Installs an inline handler, loads S_SEG_REAL:SP_REAL, and verifies that the
; saved IP points to the faulting instruction. Normal fall-through jumps to
; error. The default handler is restored; the exception frame is left on the
; stack, as documented by realModeExcCheck.
; Trashes AX, DS, SS, SP, Flags, plus state used by the supplied instruction.
;
%macro realModeFaultTest 2+
    realModeExcInit %1, %%continue
    mov     ax, S_SEG_REAL
    mov     ss, ax
    mov     sp, SP_REAL
%%test:
    %2
    jmp     error
%%continue:
    realModeExcCheck %1, %%test
%endmacro
