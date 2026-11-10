; SPDX-License-Identifier: GPL-3.0-or-later

; Protected-mode helpers adapted from test386's macro collection.

; Tests the Current Privilege Level value.
;
; %1: the value (0-3) to compare to.
; %2: error label to jump to if the CPL does not match.
;
; AX is preserved on success; Flags are changed. A usable stack is required.
; The failure branch leaves the saved AX on the stack.
;
%macro testCPL_E 2
    push    ax
    mov     ax, cs
    and     ax, 3
    cmp     ax, %1
    jne     %2
    pop     ax
%endmacro

; Tests the Current Privilege Level value.
;
; %1: the value (0-3) to compare to; jumps to error if not equal.
;
; Uses: AX, Flags.
;
%macro testCPL 1
    mov     ax, cs
    and     ax, 3
    cmp     ax, %1
    jne     error
%endmacro

; Advances the base address of data segments used by tests, D1_SEG_PROT and
; D2_SEG_PROT.
;
; Loads DS with D1_SEG_PROT and ES with D2_SEG_PROT after updating their LDT
; descriptors. Uses: BX, DX, DS, ES, Flags.
;
%macro advTestSegProt 0
    advTestBase
    updLDTDescBase D1_SEG_PROT, TEST_BASE1
    updLDTDescBase D2_SEG_PROT, TEST_BASE2
    mov     dx, D1_SEG_PROT
    mov     ds, dx
    mov     dx, D2_SEG_PROT
    mov     es, dx
%endmacro

%assign GDTSelDesc 0

; Defines a GDT descriptor in RAM, given a name, base, limit and access byte.
;
; %1: name assigned the next GDT selector (GDTSelDesc).
; %2: 24-bit base address (optional, defaults to zero).
; %3: 16-bit byte-granular limit (optional, defaults to zero).
; %4: access attributes as ORed ACC_* equates (optional, defaults to zero).
;
; Called in real mode through ptrGDTreal; advances GDTSelDesc by 8.
; There is no fifth extended-attributes argument on the 286.
; Uses: AX, BX, CL, DX, SI, DI, DS, Flags.
;
%macro defGDTDesc 1-4 0,0,0
    %assign %1 GDTSelDesc
    lds     bx, [cs:ptrGDTreal]
    mov     ax, %1
    mov     si, (%2)&0xFFFF
    mov     cl, ((%2)>>16)&0xFF
    mov     di, %3
    mov     dx, %4
    initDescriptor
    %assign GDTSelDesc GDTSelDesc+8
%endmacro

%assign LDTSelDesc 4

; Defines an LDT descriptor in RAM, given a name, base, limit and access byte.
;
; %1: name assigned the next LDT selector (LDTSelDesc, including TI).
; %2: 24-bit base address (optional, defaults to zero).
; %3: 16-bit byte-granular limit (optional, defaults to zero).
; %4: access attributes as ORed ACC_* equates (optional, defaults to zero).
;
; Called in protected mode through ptrLDTprot; advances LDTSelDesc by 8.
; There is no fifth extended-attributes argument on the 286.
; Uses: AX, BX, CL, DX, SI, DI, DS, Flags.
;
%macro defLDTDesc 1-4 0,0,0
    %assign %1 LDTSelDesc
    lds     bx, [cs:ptrLDTprot]
    mov     ax, %1
    mov     si, (%2)&0xFFFF
    mov     cl, ((%2)>>16)&0xFF
    mov     di, %3
    mov     dx, %4
    initDescriptor
    %assign LDTSelDesc LDTSelDesc+8
%endmacro

; Updates an LDT descriptor, given a selector, base, limit and access byte.
;
; %1: existing LDT selector.
; %2: new 24-bit base (optional, defaults to zero).
; %3: new 16-bit byte-granular limit (optional, defaults to zero).
; %4: new access attributes, ORed ACC_* equates (optional, defaults to zero).
;
; The stack must be initialized. General registers and DS are preserved;
; Flags are changed. Reload the affected segment register to use the new
; descriptor contents.
;
%macro updLDTDesc 1-4 0,0,0
    pusha
    push    ds
    lds     bx, [cs:ptrLDTprot]
    mov     ax, %1
    mov     si, (%2)&0xFFFF
    mov     cl, ((%2)>>16)&0xFF
    mov     di, %3
    mov     dx, %4
    call    initDescriptorProt
    pop     ds
    popa
%endmacro

; Updates the access byte of a descriptor in the LDT.
;
; %1: LDT selector.
; %2: new access attributes (ORed ACC_* equates, access byte in bits 15-8).
;
; Uses: DS. General registers and Flags are preserved; the stack must be
; initialized. Reload the affected segment register after changing its entry.
;
%macro updLDTDescAcc 2
    pusha
    pushf
    lds     bx, [cs:ptrLDTprot]
    add     bx, (%1)&0xFFF8
    mov     byte [bx+5], (%2)>>8
    popf
    popa
%endmacro

; Updates the base of a descriptor in the LDT.
;
; %1: LDT selector.
; %2: new 24-bit base.
;
; Uses: DS, BX, Flags. Reload the affected segment register to use the new base.
;
%macro updLDTDescBase 2
    lds     bx, [cs:ptrLDTprot]
    add     bx, (%1)&0xFFF8
    mov     word [bx+2], (%2)&0xFFFF
    mov     byte [bx+4], ((%2)>>16)&0xFF
%endmacro

; Updates a GDT entry with a 286 call gate.
;
; %1: GDT selector of the gate.
; %2: destination code-segment selector (optional, defaults to zero).
; %3: 16-bit destination offset (optional, defaults to zero).
; %4: number of words copied on a privilege change (optional, defaults to zero).
; %5: DPL as an ACC_DPL_* equate (optional, defaults to zero).
;
; The stack must be initialized. Uses: AX, BX, DX, SI, DI, ES, Flags.
; The writable GDT alias is addressed through ES instead of test386's FS.
;
%macro updCallGate 1-5 0,0,0,0
    les     bx, [cs:ptrGDTprot]
    mov     ax, %1
    mov     si, %2
    mov     di, %3
    mov     dx, %4|%5
    call    initCallGate286
%endmacro

; Loads SS:SP with the pointer to the protected-mode stack in ptrSSprot.
;
; LSS is unavailable on the 286. MOV SS is followed immediately by MOV SP.
; Uses: AX; replaces SS and SP.
;
%macro loadProtModeStack 0
    mov     ax, [cs:ptrSSprot+2]
    mov     ss, ax
    mov     sp, [cs:ptrSSprot]
%endmacro

; Sets an interrupt gate in the IDT in protected mode.
;
; %1: vector.
; %2: 16-bit handler offset.
; %3: DPL, using ACC_DPL_* equates (optional; -1 selects the caller's CPL).
;
; The stack must be initialized. The suite uses ring 0 and ring 3: DPL 0
; selects C_SEG_PROT16, and DPL 3 selects CU_SEG_PROT16. The IDT data alias
; is chosen for the caller's privilege. General registers, DS and Flags
; are preserved.
;
%macro setProtModeIntGate 2-3 -1
    pusha
    pushf
    push    ds
    mov     ax, %1
    mov     di, %2
    %if %3 != -1
        mov     dx, %3
    %else
        mov     dx, cs
        and     dx, 3
        shl     dx, 13
    %endif
    cmp     dx, ACC_DPL_0
    jne     %%dpl3
%%dpl0:
    mov     si, C_SEG_PROT16
    jmp     %%cont
%%dpl3:
    mov     si, CU_SEG_PROT16
%%cont:
    mov     cx, cs
    test    cx, 3
    jnz     %%ring3
%%ring0:
    lds     bx, [cs:ptrIDTprot]
    jmp     %%call
%%ring3:
    lds     bx, [cs:ptrIDTUprot]
%%call:
    call    initIntGateProt
    pop     ds
    popf
    popa
%endmacro

; Tests a fault.
;
; %1: vector.
; %2: expected error code (only checked for vectors that push one).
; %3: fault-causing instruction; cannot be a CALL unless the CALL itself
;     is the instruction expected to fault.
;
; The stack must be initialized. An inline handler at the caller's privilege
; checks the saved CS:IP and error code. Normal fall-through jumps to error.
; After checking and discarding the same-ring frame, the macro installs
; DefaultExcHandler at DPL 0. Saved FLAGS are not restored with IRET.
; Uses: AX, BX, Flags, and any state changed by the instruction under test.
;
%macro protModeFaultTest 3+
    setProtModeIntGate %1, %%continue
%%test:
    %3
    jmp     error
%%continue:
    protModeExcCheck %1, %2, %%test
    setProtModeIntGate %1, DefaultExcHandler, ACC_DPL_0
%endmacro

; Checks IF immediately after IRET. Preserves general registers.
; %1: expected IF (0 or 1). IRET at CPL > IOPL cannot change IF.
%macro checkInterruptReturnIF 1
    push    ax
    pushf
    pop     ax
    test    ax, PS_IF
    %if %1
        jz      error
    %else
        jnz     error
    %endif
    pop     ax
%endmacro

; Checks an exception result and discards the same-ring exception frame.
;
; %1: vector.
; %2: expected error code (for #DF and vectors #TS through #GP).
; %3: expected pushed value of IP.
; %4: expected pushed value of CS (optional; -1 compares with current CS).
;
; The frame is at SS:SP. BX supplies an addressable 16-bit stack cursor;
; SP cannot be used as an effective-address base. Successful validation
; discards IP, CS, FLAGS and any error-code word, without restoring FLAGS.
; This macro does not reinstall an IDT handler; protModeFaultTest does that.
; Uses: AX (when CS is inferred), BX, SP, Flags.
;
%macro protModeExcCheck 3-4 -1
    mov     bx, sp
    %if %1 == 8 || (%1 >= 10 && %1 <= 13)
        %assign exc_errcode 2
        cmp     word [ss:bx], %2
        jne     error
    %else
        %assign exc_errcode 0
    %endif
    %if %4 != -1
        cmp     word [ss:bx+exc_errcode+2], %4
        jne     error
    %else
        mov     ax, cs
        cmp     [ss:bx+exc_errcode+2], ax
        jne     error
    %endif
    cmp     word [ss:bx+exc_errcode], %3
    jne     error
    add     sp, 6+exc_errcode
%endmacro
