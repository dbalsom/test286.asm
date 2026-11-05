; SPDX-License-Identifier: GPL-3.0-or-later

; Output a byte to the POST port, destroys AL and DX.
;
; %1: POST byte written as hexadecimal digits, without an 0x prefix.
;     For example, POST 20 writes 20h.
;
%macro POST 1
    mov     al, 0x%1
    mov     dx, POST_PORT
    out     dx, al
%endmacro

; Initializes a 286 interrupt gate in system memory.
; This is the body of procedures used in real and protected-mode code.
;
;     7                             0 7                             0
;    ╔═══════════════════════════════╤═══════════════════════════════╗
;  +7║                   RESERVED (written as zero)                  ║+6
;    ╟───┬───────┬───┬───────────────┬───────────────────────────────╢
;  +5║ P │  DPL  │ 0 │ 0   1   1   0 │        UNUSED (zero)          ║+4
;    ╟───┴───────┴───┴───────────────┴───────────────────────────────╢
;  +3║                            SELECTOR                           ║+2
;    ╟───────────────────────────────────────────────────────────────╢
;  +1║                          OFFSET 15-0                          ║ 0
;    ╚═══════════════════════════════════════════════════════════════╝
;     15                                                            0
;
; DS:BX pointer to the IDT
; AX vector
; SI destination code-segment selector
; DI 16-bit offset
; DX DPL (use ACC_DPL_* equates; other bits zero)
;
; Uses: AX, BX, DX, Flags. Bytes 6-7 are zero, not an upper offset.
;
%macro initIntGate286 0
    shl     ax, 3
    add     bx, ax
    mov     [bx], di
    mov     [bx+2], si
    or      dx, ACC_TYPE_GATE286_INT | ACC_PRESENT
    mov     [bx+4], dx
    mov     word [bx+6], 0
%endmacro

; Initializes a task gate in system memory.
; This is the body of procedures used in real and protected-mode code.
;
;     7                             0 7                             0
;    ╔═══════════════════════════════╤═══════════════════════════════╗
;  +7║                         UNUSED (zero)                         ║+6
;    ╟───┬───────┬───┬───────────────┬───────────────────────────────╢
;  +5║ P │  DPL  │ 0 │ 0   1   0   1 │        UNUSED (zero)          ║+4
;    ╟───┴───────┴───┴───────────────┴───────────────────────────────╢
;  +3║                          TSS SELECTOR                         ║+2
;    ╟───────────────────────────────────────────────────────────────╢
;  +1║                         UNUSED (zero)                         ║ 0
;    ╚═══════════════════════════════════════════════════════════════╝
;     15                                                            0
;
; DS:BX pointer to the IDT
; AX vector
; SI TSS selector
; DX DPL (use ACC_DPL_* equates; other bits zero)
;
; Uses: AX, BX, DX, Flags.
;
%macro initIntTaskGate 0
    shl     ax, 3
    add     bx, ax
    mov     word [bx], 0
    mov     [bx+2], si
    or      dx, ACC_TYPE_GATE_TSS | ACC_PRESENT
    mov     [bx+4], dx
    mov     word [bx+6], 0
%endmacro

; Set a descriptor in system memory.
; This is the body of procedures used in real and protected-mode code.
;
;     7                             0 7                             0
;    ╔═══════════════════════════════╤═══════════════════════════════╗
;  +7║                   RESERVED (written as zero)                  ║+6
;    ╟───┬───────┬───┬───────────────┬───────────────────────────────╢
;  +5║ P │  DPL  │ S │     TYPE      │          BASE 23-16           ║+4
;    ╟───┴───────┴───┴───────────────┴───────────────────────────────╢
;  +3║                           BASE 15-0                           ║+2
;    ╟───────────────────────────────────────────────────────────────╢
;  +1║                           LIMIT 15-0                          ║ 0
;    ╚═══════════════════════════════════════════════════════════════╝
;     15                                                            0
;
; DS:BX pointer to the descriptor table
; AX segment selector (RPL/TI bits are masked off to obtain the table offset)
; SI base bits 15-0
; CL base bits 23-16
; DI byte-granular limit, bits 15-0
; DH access byte (P|DPL|S|TYPE; TYPE includes A for code/data segments)
;
; Uses: AX, BX, Flags. The 286 has no extended limit or G/B/D/AVL nibble;
; bytes 6-7 are written as zero.
;
%macro initDescriptor 0
    and     ax, 0xFFF8
    add     bx, ax
    mov     [bx], di
    mov     [bx+2], si
    mov     [bx+4], cl
    mov     [bx+5], dh
    mov     word [bx+6], 0
%endmacro

; Advances TEST_BASE1 and TEST_BASE2 by 1000h for the next tests.
%macro advTestBase 0
    %assign TEST_BASE1 TEST_BASE1+0x1000
    %assign TEST_BASE2 TEST_BASE2+0x1000
%endmacro
