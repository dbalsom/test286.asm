; SPDX-License-Identifier: GPL-3.0-or-later
;
; LAR/LSL descriptor queries. No queried selector is loaded as a data segment.
;

; Checks both register and SS-relative memory source forms of LAR or LSL.
; %1: instruction, %2: selector, %3: expected ZF (0/1), %4: expected result.
; Seed ZF opposite to the expected result. A rejected selector must leave AX
; unchanged; a successful LAR must also clear the low byte of AX.
; Uses: AX, BX, DX, Flags. BP and SP are preserved; DS/ES are not used.
%macro testDescriptorQuery 4
    push    bp
    mov     bp, sp
    push    word %2
    mov     dx, %2
    %assign %%memory 0
    %rep 2
    mov     ah, (1-%3)*PS_ZF
    sahf
    mov     ax, 0xA55A
    %if %%memory
    %1      ax, [ss:bp-2]
    %else
    %1      ax, dx
    %endif
    %if %3
    jnz     error
    cmp     ax, %4
    %else
    jz      error
    cmp     ax, 0xA55A                  ; Failure must not publish a result
    %endif
    jne     error
    cmp     dx, %2
    jne     error
    cmp     word [ss:bp-2], %2
    jne     error
        %assign %%memory 1
    %endrep
    mov     sp, bp
    pop     bp
%endmacro

; Check both query instructions for a selector.
; %1: selector, %2: expected ZF, %3: access rights, %4: limit.
%macro testDescriptorPair 4
    testDescriptorQuery lar, %1, %2, %3
    testDescriptorQuery lsl, %1, %2, %4
%endmacro

; Null/out-of-table, data, code, system and privilege cases at CPL 0 and 3.
; Explicitly set A=1 on fixture code/data descriptors so prior segment loads
; cannot change the expected access byte. The 286 LSL result is the literal
; 16-bit limit, including zero and FFFFh; there is no granularity bit.
; Requires the suite's GDT/LDT and ring-switch helpers. Registers are scratch.
%macro testDescriptorQueries 0
    testDescriptorPair NULL, 0, 0, 0
    testDescriptorPair NULL|3, 0, 0, 0
    testDescriptorPair GDT_SEG_LIMIT+1, 0, 0, 0
    testDescriptorPair 0x604, 0, 0, 0    ; TI=1, offset 600h exceeds LDT limit 5FFh

    updLDTDesc DTEST_SEG_PROT, TEST_BASE, 0x1234, ACC_PRESENT|ACC_TYPE_DATA_W|ACC_ACCESSED
    testDescriptorPair DTEST_SEG_PROT, 1, 0x9300, 0x1234
    testDescriptorPair DTEST_SEG_PROT|3, 0, 0, 0 ; RPL alone prevents access
    updLDTDesc DTEST_SEG_PROT, TEST_BASE, 0, ACC_PRESENT|ACC_TYPE_DATA_W|ACC_ACCESSED
    testDescriptorPair DTEST_SEG_PROT, 1, 0x9300, 0
    updLDTDesc DTEST_SEG_PROT, TEST_BASE, 0xFFFF, ACC_PRESENT|ACC_TYPE_DATA_W|ACC_ACCESSED|ACC_DPL_3
    testDescriptorPair DTEST_SEG_PROT|3, 1, 0xF300, 0xFFFF
    ; Querying a not-present descriptor succeeds; it does not load a segment.
    updLDTDesc DTEST_SEG_PROT, TEST_BASE, 0x3456, ACC_TYPE_DATA_W|ACC_ACCESSED
    testDescriptorPair DTEST_SEG_PROT, 1, 0x1300, 0x3456
    testDescriptorPair LDT_SEG_PROT, 1, ACC_PRESENT|ACC_TYPE_LDT, 0x5FF
    ; A call gate has access rights, but no segment limit for LSL to return.
    testDescriptorQuery lar, RING0_GATE, 1, ACC_PRESENT|ACC_DPL_3|ACC_TYPE_GATE286_CALL
    testDescriptorQuery lsl, RING0_GATE, 0, 0

    updLDTDesc DTEST_SEG_PROT, TEST_BASE, 0x2468, ACC_PRESENT|ACC_TYPE_DATA_W|ACC_ACCESSED
    call    switchToRing3
    testDescriptorPair DTEST_SEG_PROT, 0, 0, 0 ; CPL prevents access even with RPL=0
    testDescriptorPair DTEST_SEG_PROT|3, 0, 0, 0
    testDescriptorPair LDT_SEG_PROT, 0, 0, 0
    testDescriptorPair NULL, 0, 0, 0
    testDescriptorPair CU_SEG_PROT16|3, 1, 0xFB00, 0xFFFF
    testDescriptorPair SU_SEG_PROT16|3, 1, 0xF300, 0xFFFF
    call    switchToRing0

    ; Conforming code is visible to CPL 3 even though its descriptor has DPL 0.
    updLDTDesc DTEST_SEG_PROT, TEST_BASE, 0x1357, ACC_PRESENT|ACC_TYPE_CODE_R|ACC_TYPE_CONFORMING|ACC_ACCESSED
    call    switchToRing3
    testDescriptorPair DTEST_SEG_PROT|3, 1, 0x9F00, 0x1357
    call    switchToRing0

    updLDTDesc DTEST_SEG_PROT, TEST_BASE, 0x2468, ACC_PRESENT|ACC_TYPE_DATA_W|ACC_ACCESSED|ACC_DPL_3
    call    switchToRing3
    testDescriptorPair DTEST_SEG_PROT|3, 1, 0xF300, 0x2468
    testDescriptorPair DTEST_SEG_PROT, 1, 0xF300, 0x2468
    call    switchToRing0
    updLDTDesc DTEST_SEG_PROT, TEST_BASE, 0xFFFF, ACC_PRESENT|ACC_TYPE_DATA_W
%endmacro
