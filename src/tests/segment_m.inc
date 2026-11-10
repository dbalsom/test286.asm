; SPDX-License-Identifier: GPL-3.0-or-later
;
; Protected-mode data-segment limit tests. The stack keeps its normal limit
; so that expected #GP faults can enter the existing exception-test handler.
;

; Checks a byte or word read/write pair that must raise #GP(0).
;
; %1: segment register under test (DS or ES).
; %2: the other data segment, a full-size read-only alias of the same memory.
; %3: offset.
; %4: operand size (byte or word).
;
; The write attempts the complement of the original contents, then checks
; through the alias that the fault did not modify them. At FFFFh the alias
; can check only the first byte of a word; reading a word there would fault.
; Uses: AX, BX, CX, DX, Flags. Requires a valid stack and protected-mode IDT.
%macro testSegmentLimitFault 4
    %ifidni %4, byte
    protModeFaultTest EX_GP, 0, mov al,[%1:%3]
    mov     dl, [%2:%3]
    mov     cl, dl
    not     cl                          ; Make any committed byte detectable
    push    dx
    protModeFaultTest EX_GP, 0, mov byte [%1:%3],cl
    pop     dx
    cmp     [%2:%3], dl
    jne     error
    %else
    protModeFaultTest EX_GP, 0, mov ax,[%1:%3]
    %if %3 == 0xFFFF
    mov     dl, [%2:%3]
    %else
    mov     dx, [%2:%3]
    %endif
    mov     cx, dx
    not     cx                          ; Detect even a partial word write
    push    dx
    protModeFaultTest EX_GP, 0, mov word [%1:%3],cx
    pop     dx
    %if %3 == 0xFFFF
    cmp     [%2:%3], dl
    %else
    cmp     [%2:%3], dx
    %endif
    jne     error
    %endif
%endmacro

; Tests inclusive expand-up limits and exclusive expand-down lower bounds.
;
; %1: segment register under test (DS or ES).
; %2: the other data segment, used to check memory after faulting writes.
;
; DTEST_SEG_PROT and RO_SEG_PROT both map TEST_BASE. The descriptor is
; explicitly reloaded after every change. DS and ES are restored on return,
; and DTEST_SEG_PROT is left writable, expand-up, with a limit of FFFFh.
; Uses: AX, BX, CX, DX, Flags and memory at TEST_BASE+0, +1, +2, +FFFE, +FFFF.
%macro testSegmentLimits 2
    push    ds
    push    es
    mov     ax, RO_SEG_PROT
    mov     %2, ax

    ; Full-size expand-up segment: the last byte and last complete word fit.
    updLDTDesc DTEST_SEG_PROT, TEST_BASE, 0xFFFF, ACC_PRESENT|ACC_TYPE_DATA_W
    mov     ax, DTEST_SEG_PROT
    mov     %1, ax
    mov     word [%1:0xFFFE], 0x1234
    cmp     word [%1:0xFFFE], 0x1234
    jne     error
    mov     byte [%1:0xFFFF], 0xA5
    cmp     byte [%1:0xFFFF], 0xA5
    jne     error
    ; A word at FFFFh must fault instead of wrapping or crossing the segment.
    testSegmentLimitFault %1, %2, 0xFFFF, word

    ; Expand-up limit 0000h describes one byte, not an empty segment.
    updLDTDesc DTEST_SEG_PROT, TEST_BASE, 0x0000, ACC_PRESENT|ACC_TYPE_DATA_W
    mov     ax, DTEST_SEG_PROT
    mov     %1, ax
    mov     byte [%1:0], 0x5A
    cmp     byte [%1:0], 0x5A
    jne     error
    testSegmentLimitFault %1, %2, 0, word
    testSegmentLimitFault %1, %2, 1, byte
    testSegmentLimitFault %1, %2, 1, word

    ; Expand-down limit 0000h excludes offset zero; offsets 1-FFFFh fit.
    updLDTDesc DTEST_SEG_PROT, TEST_BASE, 0x0000, ACC_PRESENT|ACC_TYPE_DATA_W|ACC_TYPE_EXPAND_DOWN
    mov     ax, DTEST_SEG_PROT
    mov     %1, ax
    testSegmentLimitFault %1, %2, 0, byte
    testSegmentLimitFault %1, %2, 0, word
    mov     byte [%1:1], 0x69
    cmp     byte [%1:1], 0x69
    jne     error
    mov     word [%1:1], 0xC35A
    cmp     word [%1:1], 0xC35A
    jne     error
    mov     byte [%1:0xFFFF], 0x96
    cmp     byte [%1:0xFFFF], 0x96
    jne     error
    mov     word [%1:0xFFFE], 0xA55A
    cmp     word [%1:0xFFFE], 0xA55A
    jne     error
    testSegmentLimitFault %1, %2, 0xFFFF, word

    ; Expand-down limit FFFFh is empty. Probe the bottom, middle and top
    ; with byte/word reads and writes; every access must raise #GP(0).
    updLDTDesc DTEST_SEG_PROT, TEST_BASE, 0xFFFF, ACC_PRESENT|ACC_TYPE_DATA_W|ACC_TYPE_EXPAND_DOWN
    mov     ax, DTEST_SEG_PROT
    mov     %1, ax
    testSegmentLimitFault %1, %2, 0x0000, byte
    testSegmentLimitFault %1, %2, 0x0000, word
    testSegmentLimitFault %1, %2, 0x0001, byte
    testSegmentLimitFault %1, %2, 0x0001, word
    testSegmentLimitFault %1, %2, 0x7FFF, byte
    testSegmentLimitFault %1, %2, 0x7FFF, word
    testSegmentLimitFault %1, %2, 0xFFFE, byte
    testSegmentLimitFault %1, %2, 0xFFFE, word
    testSegmentLimitFault %1, %2, 0xFFFF, byte
    testSegmentLimitFault %1, %2, 0xFFFF, word

    updLDTDesc DTEST_SEG_PROT, TEST_BASE, 0xFFFF, ACC_PRESENT|ACC_TYPE_DATA_W
    pop     es
    pop     ds
%endmacro
