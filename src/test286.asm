;SPDX-License-Identifier: GPL-3.0-or-later
;
;   test286.asm -- 16-bit port of test386.asm.
;
;   Copyright (C)2012-2015 Jeff Parsons <Jeff@pcjs.org>
;   Copyright (C)2017-2026 Marco Bortolin <barotto@gmail.com> and contributors
;   Copyright (C)2026 Daniel Balsom (gloriouscow).
;
;   Adapted from test386.asm, originally developed for IBMulator:
;   https://github.com/dbalsom/test386.asm
;   https://barotto.github.io/IBMulator
;   test386.asm derives from the PCjs CPU test ROM:
;   https://www.pcjs.org/software/pcx86/test/cpu/80386/test386.asm
;
;   test286.asm is free software: you can redistribute it and/or modify it under
;   the terms of the GNU General Public License as published by the Free
;   Software Foundation, either version 3 of the License, or (at your option)
;   any later version.
;
;   test286.asm is distributed in the hope that it will be useful, but WITHOUT ANY
;   WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
;   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
;   details.
;
;   You should have received a copy of the GNU General Public License along with
;   test286.asm.  If not see <http://www.gnu.org/licenses/gpl.html>.
;
;   Overview
;   --------
;   This file is designed to run as a test ROM, loaded in place of the BIOS.
;   Its purpose is to test the CPU, reporting its status to the POST port and
;   its computational results in ASCII form to various configurable ports.
;   A 80286 CPU is required. This ROM is designed to test an emulator but has
;   been validated on a physical Harris 80c286 CPU via ArduinoX86:
;   https://github.com/dbalsom/arduinox86
;
;   The ROM must be installed at physical address 0xf0000 and aliased at 
;   physical address 0xff0000.
;   The jump at resetVector should align with the CPU reset address 0xfffff0,
;   which transfers control to cpuTest immediately after the ROM header.
;   From that point on, all memory accesses should remain within the first 1MB.

    cpu     286
    bits    16
    org     0

%define VERSION_MAJOR 1
%define VERSION_MINOR 0
%define VERSION_PATCH 0
%defstr VERSION_STRING VERSION_MAJOR %+ . %+ VERSION_MINOR %+ . %+ VERSION_PATCH

%substr RELEASE_MONTH __DATE__ 6,2
%substr RELEASE_DAY   __DATE__ 9,2
%substr RELEASE_YEAR  __DATE__ 3,2
%define RELEASE RELEASE_MONTH, '/', RELEASE_DAY, '/', RELEASE_YEAR

%include "configuration.inc"
%include "x86_e.inc"
%include "macros_m.inc"

;
;   Physical memory map (hexadecimal addresses):
;   00000-003FF  Real mode interrupt vector table
;   00400-0057F  Protected mode IDT (vectors 00h-2Fh)
;   00600-007FF  Protected mode GDT allocation
;   00A00-00FFF  Protected mode LDT allocation
;   05000-0502B  Current task's 286 TSS
;   05200-0522B  Second task's 286 TSS
;   10000-1FFFF  Real mode and protected mode stacks
;   20000-9FFFF  Test data area, with segment bases advanced between tests
;   30000-3FFFF  Second task's stack within the test data area
;   F0000-FFFFF  Test ROM; also visible at FF0000-FFFFFF for RESET
;
TEST_BASE equ 0x20000
%assign TEST_BASE1 TEST_BASE
%assign TEST_BASE2 0x60000
%assign D1_SEG_REAL TEST_BASE1 >> 4
%assign D2_SEG_REAL TEST_BASE2 >> 4

;
;   Real mode segments
;
C_SEG_REAL equ 0xF000
S_SEG_REAL equ 0x1000
IDT_SEG_REAL equ 0x0040
GDT_SEG_REAL equ 0x0060
GDT_SEG_LIMIT equ 0x1FF
SP_REAL equ 0xFFFE
SP_R0_PROT equ 0xFFFE
SP_R3_PROT equ 0x7FFE

header:
    db      'test286.asm v', VERSION_STRING, 0

cpuTest:
    cli
    cld


; ==============================================================================
;   Real mode tests
; ==============================================================================

%include "real_m.inc"
;-------------------------------------------------------------------------------
    POST 0
;-------------------------------------------------------------------------------
;
;   Real mode initialisation
;
    initRealModeIDT
    mov     ax, S_SEG_REAL
    mov     ss, ax
    mov     sp, SP_REAL
    mov     dx, D1_SEG_REAL
    mov     ds, dx
    mov     dx, D2_SEG_REAL
    mov     es, dx


;-------------------------------------------------------------------------------
    POST 1
;-------------------------------------------------------------------------------
;
;   Conditional jumps
;
%include "tests/jcc_m.inc"
    testJcc 8

;
;   Loops
;
%include "tests/loop_m.inc"
    testLoop
    testLoopZ
    testLoopNZ


;-------------------------------------------------------------------------------
    POST 2
;-------------------------------------------------------------------------------
;
;   Quick tests of unsigned 16-bit multiplication and division
;   Thorough arithmetical and logical tests are done later
;
    mov     ax, 0x8001
    imul    ax
    mov     ax, 0x2211
    mov     bx, ax
    mov     cx, 0x6655
    mul     cx
    div     cx
    cmp     ax, bx
    jne     error


%if INJECT_FAILURE
    jmp     error
%endif

%include "tests/mov_m.inc"
;-------------------------------------------------------------------------------
    POST 3
;-------------------------------------------------------------------------------
;
;   Move segment registers in real mode
;
    testMovSegR_real ss
    testMovSegR_real ds
    testMovSegR_real es
    testMovSegR_real cs

    advTestSegReal


%include "tests/string_m.inc"
;-------------------------------------------------------------------------------
    POST 4
;-------------------------------------------------------------------------------
;
;   Test store, move, scan, and compare string data
;
    testStringOps b,0
    testStringOps w,0
    testStringOps b,1
    testStringOps w,1
    testStringReps b,0
    testStringReps w,0
    testStringReps b,1
    testStringReps w,1

    advTestSegReal


%include "tests/call_m.inc"
;-------------------------------------------------------------------------------
    POST 5
;-------------------------------------------------------------------------------
;
;   Calls in real mode
;
    mov     si, 0
    testCallNear sp
    testCallFar C_SEG_REAL

    advTestSegReal


%include "tests/load_ptr_m.inc"
;-------------------------------------------------------------------------------
    POST 6
;-------------------------------------------------------------------------------
;
;   Load full pointer in real mode
;
    mov     di, 0
    testLoadPtr ds
    testLoadPtr es

    advTestSegReal

; ==============================================================================
; Protected mode tests
; ==============================================================================

;-------------------------------------------------------------------------------
    POST 8
;-------------------------------------------------------------------------------    
;
;   Set up the GDT and IDT, enter protected mode, then initialize the LDT.
;
%include "protected_m.inc"
    ; MOV-to-CS fault tests reset the real-mode stack; restore it explicitly.
    mov     ax, S_SEG_REAL
    mov     ss, ax
    mov     sp, SP_REAL
    jmp     initGDT

; Real mode 16:16 pointers to the protected mode descriptor tables.
ptrGDTreal: dw 0, GDT_SEG_REAL          ; Offset, segment for the GDT
ptrIDTreal: dw 0, IDT_SEG_REAL          ; Offset, segment for the IDT

initGDT:
    ; The first descriptor in the GDT is reserved for the null selector.
    defGDTDesc NULL
    defGDTDesc LDT_SEG_PROT, 0xA00, 0x5FF, ACC_PRESENT|ACC_TYPE_LDT
    defGDTDesc TSS_DSEG_PROT, 0x5000, 0x2B, ACC_PRESENT|ACC_TYPE_DATA_W
    defGDTDesc TSS_DSEG_PROT16, 0x5200, 0x2B, ACC_PRESENT|ACC_TYPE_DATA_W
    defGDTDesc TSS_PROT, 0x5000, 0x2B, ACC_PRESENT|ACC_TYPE_TSS16|ACC_DPL_3
    defGDTDesc TSS_PROT16, 0x5200, 0x2B, ACC_PRESENT|ACC_TYPE_TSS16|ACC_DPL_3
    defGDTDesc C_SEG_PROT16, 0xF0000, 0xFFFF, ACC_PRESENT|ACC_TYPE_CODE_R
    defGDTDesc CU_SEG_PROT16, 0xF0000, 0xFFFF, ACC_PRESENT|ACC_TYPE_CODE_R|ACC_DPL_3
    defGDTDesc CC_SEG_PROT16, 0xF0000, 0xFFFF, ACC_PRESENT|ACC_TYPE_CODE_R|ACC_TYPE_CONFORMING
    defGDTDesc IDT_SEG_PROT, 0x400, 0x17F, ACC_PRESENT|ACC_TYPE_DATA_W
    defGDTDesc IDTU_SEG_PROT, 0x400, 0x17F, ACC_PRESENT|ACC_TYPE_DATA_W|ACC_DPL_3
    defGDTDesc GDT_DSEG_PROT, 0x600, GDT_SEG_LIMIT, ACC_PRESENT|ACC_TYPE_DATA_W
    defGDTDesc GDTU_DSEG_PROT, 0x600, GDT_SEG_LIMIT, ACC_PRESENT|ACC_TYPE_DATA_W|ACC_DPL_3
    defGDTDesc LDT_DSEG_PROT, 0xA00, 0x5FF, ACC_PRESENT|ACC_TYPE_DATA_W
    defGDTDesc S_SEG_PROT16, 0x10000, 0xFFFF, ACC_PRESENT|ACC_TYPE_DATA_W
    defGDTDesc SU_SEG_PROT16, 0x10000, 0xFFFF, ACC_PRESENT|ACC_TYPE_DATA_W|ACC_DPL_3
    defGDTDesc SU_TASK_PROT16, 0x30000, 0xFFFF, ACC_PRESENT|ACC_TYPE_DATA_W|ACC_DPL_3
    defGDTDesc RING0_GATE               ; Placeholder for the return-to-ring-0 gate
    defGDTDesc RING0_GATE2              ; Placeholder for the parameter-copying gate
    jmp     initIDT

; Protected mode pointers contain a 16-bit offset and a 16-bit selector.
; The data aliases let the code initialize system tables and TSS contents.
; Pointer to the IDT through its kernel data alias.
ptrIDTprot:
    dw 0, IDT_SEG_PROT
; Pointer to the IDT through its user data alias.
ptrIDTUprot:
    dw 0, IDTU_SEG_PROT|3
; Pointer to the GDT through its kernel data alias.
ptrGDTprot: 
    dw 0, GDT_DSEG_PROT
; Pointer to the GDT through its user data alias.
ptrGDTUprot: 
    dw 0, GDTU_DSEG_PROT|3
; Pointer to the LDT through its kernel data alias.
ptrLDTprot: 
    dw 0, LDT_DSEG_PROT
; Initial SP and SS for the protected mode kernel stack.
ptrSSprot: 
    dw SP_R0_PROT, S_SEG_PROT16
; Data pointer to the current task's 286 TSS.
ptrTSSprot: 
    dw 0, TSS_DSEG_PROT
; Data pointer to the second task's 286 TSS.
ptrTSSprot16: 
    dw 0, TSS_DSEG_PROT16
; LIDT/LGDT operands: 16-bit limit followed by the physical base.
; The 286 uses 24 base bits; the unused high byte of each DD is zero.
addrProtIDT: 
    dw 0x17F                            ; IDT limit, inclusive
    dd      0x400                       ; IDT physical base
addrGDT: dw GDT_SEG_LIMIT               ; GDT limit, inclusive
    dd      0x600                       ; GDT physical base

; Initialize a 286 interrupt gate in RAM while still in real mode.
initIntGateReal286:
    pusha
    initIntGate286
    popa
    ret
; Initialize a task gate in RAM while still in real mode.
initIntTaskGateReal:
    pusha
    initIntTaskGate
    popa
    ret

initIDT:
    ; Initially send every configured vector to the default exception handler.
    lds     bx, [cs:ptrIDTreal]
    xor     ax, ax
    mov     si, C_SEG_PROT16
    mov     di, DefaultExcHandler
    xor     dx, dx
.loop:
    call    initIntGateReal286
    inc     ax
    cmp     ax, 0x30
    jb      .loop
    ; Install the interrupt handlers used by the privilege and task tests.
    ; INT 21h: conforming kernel code, callable from user mode.
    mov     dx, ACC_DPL_3
    mov     ax, 0x21
    mov     si, CC_SEG_PROT16
    mov     di, kernelConformingInterrupt
    call    initIntGateReal286
    ; INT 22h: user code interrupt handler, entered at CPL 3.
    mov     ax, 0x22
    mov     si, CU_SEG_PROT16
    mov     di, userModeInterrupt
    call    initIntGateReal286
    ; INT 23h: kernel code interrupt handler, callable only from CPL 0.
    xor     dx, dx
    mov     ax, 0x23
    mov     si, C_SEG_PROT16
    mov     di, kernelOnlyInterrupt
    call    initIntGateReal286
    ; INT 24h: conforming kernel code, gate callable only from CPL 0.
    mov     ax, 0x24
    mov     si, CC_SEG_PROT16
    mov     di, kernelOnlyConformingInterrupt
    call    initIntGateReal286
    ; INT 27h: non-conforming kernel code through a 286 interrupt gate,
    ; callable from user mode and switching to the kernel stack.
    mov     dx, ACC_DPL_3
    mov     ax, 0x27
    mov     si, C_SEG_PROT16
    mov     di, kernelInterrupt286
    call    initIntGateReal286
    ; INT 28h: task switch to the second 286 TSS, callable from user mode.
    mov     ax, 0x28
    mov     si, TSS_PROT16
    call    initIntTaskGateReal
    ; 286 harness addition: a ring-3 assertion can report failure through this
    ; gate without attempting a privileged OUT, CLI or HLT in user mode.
    mov     ax, 0x2F
    mov     si, C_SEG_PROT16
    mov     di, error
    call    initIntGateReal286

switchToProtMode:
    cli                                 ; Keep interrupts off during the mode switch
    lidt    [cs:addrProtIDT]
    lgdt    [cs:addrGDT]
    smsw    ax
    or      ax, CR0_MSW_PE              ; Set PE in the machine status word
    lmsw    ax
    jmp     C_SEG_PROT16:toProt16       ; Reload CS with a protected mode selector
toProt16:
    jmp     initLDT
%include "protected_p.inc"

initLDT:
    ; Define the data segments and fault-test descriptors used below.
    defLDTDesc D_SEG_PROT16, TEST_BASE, 0xFFFF, ACC_PRESENT|ACC_TYPE_DATA_W
    defLDTDesc DU_SEG_PROT, TEST_BASE, 0xFFFF, ACC_PRESENT|ACC_TYPE_DATA_W|ACC_DPL_3
    defLDTDesc D1_SEG_PROT, TEST_BASE1, 0xFFFF, ACC_PRESENT|ACC_TYPE_DATA_W
    defLDTDesc D2_SEG_PROT, TEST_BASE2, 0xFFFF, ACC_PRESENT|ACC_TYPE_DATA_W
    defLDTDesc RO_SEG_PROT, TEST_BASE, 0xFFFF, ACC_PRESENT|ACC_TYPE_DATA_R
    defLDTDesc ROU_SEG_PROT, TEST_BASE, 0xFFFF, ACC_PRESENT|ACC_TYPE_DATA_R|ACC_DPL_3
    defLDTDesc DTEST_SEG_PROT, TEST_BASE, 0xFFFF, ACC_PRESENT|ACC_TYPE_DATA_W
    defLDTDesc DPL1_SEG_PROT, TEST_BASE, 0xFFFF, ACC_PRESENT|ACC_TYPE_DATA_W|ACC_DPL_1
    defLDTDesc NP_SEG_PROT, TEST_BASE, 0xFFFF, ACC_TYPE_DATA_W
    defLDTDesc SYS_SEG_PROT, TEST_BASE, 0xFFFF, ACC_PRESENT|ACC_TYPE_LDT
    mov     ax, LDT_SEG_PROT
    lldt    ax                          ; Load the task's local descriptor table
    mov     ax, TSS_PROT
    ltr     ax                          ; Select the current task's TSS
    loadProtModeStack
    jmp     protTests
%include "tss_p.inc"
%include "protected_rings_p.inc"
%include "protected_tssh.inc"
%include "protected_tsshelpers.inc"
%include "protected_tss_m.inc"
protTests:

%include "tests/stack_m.inc"

;-------------------------------------------------------------------------------
    POST 9
;-------------------------------------------------------------------------------
;
;   Stack functionality
;
;
;   For the next tests, with a 16-bit data segment in SS, we
;   expect all pushes/pops to address the stack through SP.
;
    mov     ax, D_SEG_PROT16
    mov     ds, ax
    mov     ss, ax
    mov     es, ax

    ;
    ; general purpose registers
    ;
    testPushPopR ax,16
    testPushPopR bx,16
    testPushPopR cx,16
    testPushPopR dx,16
    testPushPopR sp,16
    testPushPopR bp,16
    testPushPopR si,16
    testPushPopR di,16

    testPushPopAll16 16

    ;
    ; segment registers
    ;
    testPushPopSR cs,16
    testPushPopSR ds,16
    testPushPopSR ss,16
    testPushPopSR es,16

    ;
    ; memory
    ;
    testPushPopM 16
    testPushImm 16

    ;
    ; flags
    ;
    testPushPopF 16

    ; the stack works
    ; initialize it for the next tests
    loadProtModeStack

    advTestSegProt

;-------------------------------------------------------------------------------
    POST 20
;-------------------------------------------------------------------------------
;
;   Test user mode (ring 3) switching, interrupt frames and call gates.
;
    call    clearTSS
    mov     ax, D_SEG_PROT16
    mov     ds, ax
    mov     es, ax
    ; Test I/O port access permissions: IOPL 3 permits I/O from user mode.
    pushf
    pop     ax
    or      ax, 0x3000                  ; Set IOPL to 3
    push    ax
    popf
    call    switchToRing3
    ; Same IOPL=3 permission test as test386, but write a diagnostic byte.
    ; There is no device supplying data for an IN on ArduinoX86
    mov     dx, OUT_PORT
    mov     al, '.'
%if OUT_PORT
    out     dx, al
%endif
    call    switchToRing0
    ; CS must be C_SEG_PROT16 (CPL 0) after returning to the kernel.
    mov     ax, cs
    cmp     ax, C_SEG_PROT16
    jne     error
    pushf
    pop     ax
    and     ax, 0x0FFF                  ; Restore IOPL 0 before re-entering user mode
    push    ax
    popf
    call    switchToRing3
    ; CS must be CU_SEG_PROT16|3 (CPL 3).
    mov     ax, cs
    cmp     ax, CU_SEG_PROT16|3
    jne     error
    ; The kernel data selectors in DS and ES must have been cleared on return
    ; to user mode, since their DPL does not permit access from CPL 3.
    mov     ax, ds
    cmp     ax, 0
    jne     error
    mov     ax, es
    cmp     ax, 0
    jne     error
    ; Test privileged instructions and I/O at CPL 3 with IOPL 0.
    protModeFaultTest EX_GP, 0, cli
    protModeFaultTest EX_GP, 0, hlt
    protModeFaultTest EX_GP, 0, in al,0x64
    ; A user INT through the DPL-0 gate must fault; the error code identifies
    ; IDT entry 23h (23h * 8 = 118h, with the IDT bit set).
    protModeFaultTest EX_GP, 0x118|2, int 0x23
    ; The handler checks assume the initial user stack, SU_SEG_PROT16:SP_R3_PROT.
    ; Interrupt from user mode to kernel mode through a 16-bit gate.
    int     0x27
kernelModeInterruptReturn286:
    ; Interrupt into conforming kernel code; execution stays at CPL 3.
    int     0x21
kernelConformingInterruptReturn:
    ; Interrupt from user mode to user mode without a stack switch.
    int     0x22
userInterruptReturn:
    call    CU_SEG_PROT16|3:userFarFunc   ; User mode far call test
    jmp     CU_SEG_PROT16|3:userJmpFunc   ; User mode far jump test
userFarFunc:
    retf                                ; Return at the same privilege level
;
; Interrupt handlers installed during POST 08h.
;
%include "protected_inth.inc"
userJmpFunc:
    ; Direct user-to-kernel transfers must fault rather than change privilege.
    protModeFaultTest EX_GP, C_SEG_PROT16, jmp C_SEG_PROT16|3:0
    protModeFaultTest EX_GP, C_SEG_PROT16, call C_SEG_PROT16|3:0
    push    word C_SEG_PROT16
    push    word error
    protModeFaultTest EX_GP, C_SEG_PROT16, retf
    add     sp, 4                       ; Discard the rejected far-return address
    call    switchToRing0
    ; Verify that the call gate returned us to C_SEG_PROT16 at CPL 0.
    mov     ax, cs
    cmp     ax, C_SEG_PROT16
    jne     error
    ; Test copying parameters from the user stack through a call gate.
    jmp     testCallGateWithParameters
ring0_2TestEndLocation:
    ; Test the kernel-only interrupt without changing privilege.
    mov     ax, sp                      ; Save the pre-INT SP for the handler's check
    int     0x23
kernelModeOnlyInterruptReturn:
    ; An interrupt cannot transfer from kernel code to less-privileged user code.
    protModeFaultTest EX_GP, CU_SEG_PROT16, int 0x22
    ; Interrupt from kernel mode to conforming kernel code, still at CPL 0.
    mov     ax, sp                      ; Save the pre-INT SP for the handler's check
    int     0x24
kernelOnlyConformingInterruptReturn:

;-------------------------------------------------------------------------------
    POST 22
;-------------------------------------------------------------------------------
;
;   Test TSS task switching.
;
; 286 task tests use the test386 16-bit TSS initializer and register checks.
; The nested INT/task-gate path returns with IRET. A non-nested JMP pair is
; added for the 286, checking busy bits, backlink, NT and TS explicitly.
    call    initTSS16
    validateTSSbusy286 TSS_PROT, 1
    validateTSSbusy286 TSS_PROT16, 0
    ; Enter the second task through an IDT task gate; its IRET returns here.
    int     0x28
    mov     ax, DU_SEG_PROT|3
    mov     ds, ax
    cmp     word [0x100], 0x2861        ; The task completed its register checks
    jne     error
    validateTSSbusy286 TSS_PROT, 1
    validateTSSbusy286 TSS_PROT16, 0
    validateTSSbacklink286 TSS_DSEG_PROT16, TSS_PROT
    validateTSSNT286 0,0,0
    validateTSandClear 1
    ; Reuse the second TSS for a JMP task switch, which does not nest tasks.
    lds     bx, [cs:ptrTSSprot16]
    mov     word [bx+0xE], TSS286jumpEntry  ; Saved IP in the 286 TSS
    mov     word [bx+0x10], 2           ; Initial FLAGS with NT clear
    jmp     TSS_PROT16:0
taskJumpReturn:
    mov     ax, DU_SEG_PROT|3
    mov     ds, ax
    cmp     word [0x100], 0x2862        ; The second task took the JMP path
    jne     error
    validateTSSbusy286 TSS_PROT, 1
    validateTSSbusy286 TSS_PROT16, 0
    validateTSSNT286 0,0,0
    validateTSandClear 1
    ; Restore the normal test data segments after task switching.
    mov     ax, D1_SEG_PROT
    mov     ds, ax
    mov     ax, D2_SEG_PROT
    mov     es, ax
;-------------------------------------------------------------------------------
    POST B
;-------------------------------------------------------------------------------
;
;   Test of moving segment registers in protected mode
;
    testMovSegR_prot ds
    testMovSegR_prot es
    testMovSegR_prot cs
    testMovSegR_prot ss

    loadProtModeStack
    advTestSegProt

;-------------------------------------------------------------------------------
    POST C
;-------------------------------------------------------------------------------
;
;   Zero and sign-extension tests.
;
; Retain the test386 immediate sign-extension check; add CBW/CWD in place
; of the unavailable MOVSX/MOVZX instructions.
    push    byte -128                   ; Explicit imm8 form: opcode 6Ah
    pop     bx                          ; Sign-extended result must be FF80h
    cmp     bx, 0xFF80
    jne     error
    and     bx, 0xFF                    ; Keep the byte and zero its upper half
    cmp     bx, 0x80
    jne     error
    ; Sign-extend a negative byte through AX into DX:AX.
    mov     al, 0x80
    cbw
    cmp     ax, 0xFF80
    jne     error
    cwd
    cmp     dx, 0xFFFF
    jne     error
    ; Repeat with a positive byte; AH and DX must remain zero.
    mov     al, 0x7F
    cbw
    cmp     ax, 0x7F
    jne     error
    cwd
    cmp     dx, 0
    jne     error
%include "tests/lea_m.inc"
postD:
;-------------------------------------------------------------------------------
    POST D
;-------------------------------------------------------------------------------
;
;   16-bit addressing modes (LEA)
;
    mov     ax, 0x0001
    mov     bx, 0x0002
    mov     cx, 0x0004
    mov     dx, 0x0008
    mov     si, 0x0010
    mov     di, 0x0020
    testLEA16 [0x4000],0x4000
    testLEA16 [bx], 0x0002
    testLEA16 [si], 0x0010
    testLEA16 [di], 0x0020
    testLEA16 [bx + 0x40], 0x0042
    testLEA16 [si + 0x40], 0x0050
    testLEA16 [di + 0x40], 0x0060
    testLEA16 [bx + 0x4000], 0x4002
    testLEA16 [si + 0x4000], 0x4010
    testLEA16 [bx + si], 0x0012
    testLEA16 [bx + di], 0x0022
    testLEA16 [bx + 0x40 + si], 0x0052
    testLEA16 [bx + 0x40 + di], 0x0062
    testLEA16 [bx + 0x4000 + si], 0x4012
    testLEA16 [bx + 0x4000 + di], 0x4022

;-------------------------------------------------------------------------------
    POST F
;-------------------------------------------------------------------------------
; 286 addition replacing the 386 SIB tests: base/index addressing and the
; BP default-SS rule, with explicit DS and ES overrides.
    mov     bx, 0x200
    mov     si, 0x10
    mov     di, 0x20
    mov     bp, 0x200
    ; Store known words through base/index forms and check their effective offsets.
    mov     word [bx+si], 0x1234
    cmp     word [0x210], 0x1234
    jne     error
    mov     word [bx+di+4], 0x5678
    cmp     word [0x224], 0x5678
    jne     error
    ; A BP-based address uses SS by default.
    mov     word [bp+si], 0xABCD
    cmp     word [ss:0x210], 0xABCD
    jne     error
    ; Explicit segment overrides select DS for BP and ES for BX/DI.
    cmp     word [ds:bp+si], 0x1234
    jne     error
    mov     word [es:bx+di], 0x4321
    cmp     word [es:0x220], 0x4321
    jne     error

;-------------------------------------------------------------------------------
    POST 10
;-------------------------------------------------------------------------------
;
;   Store, move, scan, compare and load string data in protected mode.
;
; String instructions in protected mode, preserving the test386 cases.
    pusha
    pushf
    testStringOps b,0
    testStringOps w,0
    testStringOps b,1
    testStringOps w,1
    testStringReps b,0
    testStringReps w,0
    testStringReps b,1
    testStringReps w,1
    popf
    popa
    advTestSegProt

;-------------------------------------------------------------------------------
    POST 12
;-------------------------------------------------------------------------------
;
;   Verify memory access faults.
;
; Segment permissions and byte-granular limits (no paging on the 286).
    ; #GP(0) when the destination operand is in a non-writable segment.
    mov     ax, RO_SEG_PROT
    mov     ds, ax
    protModeFaultTest EX_GP, 0, mov word [0],1
    ; Use a DS limit of FFh to check the last byte and accesses crossing it.
    updLDTDesc DTEST_SEG_PROT, TEST_BASE, 0xFF, ACC_PRESENT|ACC_TYPE_DATA_W
    mov     ax, DTEST_SEG_PROT
    mov     ds, ax
    ; Reading and writing the byte at the inclusive limit must succeed.
    mov     byte [0xFF], 0x5A
    cmp     byte [0xFF], 0x5A
    jne     error
    ; #GP(0) on a word read or write that extends past the DS limit,
    ; and on a byte read whose starting offset is already out of bounds.
    protModeFaultTest EX_GP, 0, mov ax,[0xFF]
    protModeFaultTest EX_GP, 0, mov word [0xFF],0x1234
    protModeFaultTest EX_GP, 0, mov al,[0x100]
    ; #SS(0) when the word read extends past the SS limit of FFFFh.
    protModeFaultTest EX_SS, 0, mov ax,[ss:0xFFFF]
    ; Restore the descriptor and reload the normal test data selector.
    updLDTDesc DTEST_SEG_PROT, TEST_BASE, 0xFFFF, ACC_PRESENT|ACC_TYPE_DATA_W
    mov     ax, D1_SEG_PROT
    mov     ds, ax
    advTestSegProt
;-------------------------------------------------------------------------------
    POST 16
;-------------------------------------------------------------------------------
;
;   Calls in protected mode
;
    mov     si, 0
    testCallNear sp
    testCallFar C_SEG_PROT16

    advTestSegProt


;-------------------------------------------------------------------------------
    POST 17
;-------------------------------------------------------------------------------
;
;   Adjust RPL Field of Selector (ARPL)
;
    ; test on register destination
    xor     ax, ax                      ; ZF = 0
    mov     ax, 0xfff0
    mov     bx, 0x0002
    arpl    ax, bx                      ; RPL ax < RPL bx
    jnz     error                       ; must be ZF = 1
    cmp     ax, 0xfff2
    jne     error
    ; test on memory destination
    xor     ax, ax                      ; ZF = 0
    mov     word [0x200], 0xfff0
    arpl    [0x200], bx
    jnz     error
    cmp     word [0x200], 0xfff2
    jne     error
    %if 1
    ; test unexpected memory write
    ;
    ; This test fails with Bochs, which does not write to memory (correctly),
    ; but throws a #GP fault before that, during the reading of the memory
    ; operand. Bochs checks that the destination segment is writeable before the
    ; execution of ARPL.
    ;
    ; make DS read only
    updLDTDescAcc D1_SEG_PROT,ACC_TYPE_DATA_R|ACC_PRESENT
    mov     ax, D1_SEG_PROT
    mov     ds, ax
    xor     ax, ax
    arpl    [0x200], bx                 ; value has not changed, arpl should not write to memory
    ; make DS writeable again
    updLDTDescAcc D1_SEG_PROT,ACC_TYPE_DATA_W|ACC_PRESENT
    mov     ax, D1_SEG_PROT
    mov     ds, ax
    %endif
    ; test with RPL dest > RPL src
    xor     ax, ax                      ; ZF = 0
    mov     ax, 0xfff3
    arpl    ax, bx
    jz      error
    cmp     ax, 0xfff3
    jne     error

    advTestSegProt
;-------------------------------------------------------------------------------
    POST 18
;-------------------------------------------------------------------------------
;
;   Check Array Index Against Bounds (BOUND)
;
    setProtModeIntGate EX_BR, BoundExcHandler
    xor     ax, ax
    mov     bx, 0x100
    mov     word [0x200], 0x0010
    mov     word [0x202], 0x0102
    ; BX is within the inclusive bounds; the exception handler must not run.
    bound   bx, [0x200]
    cmp     ax, BOUND_HANDLER_SIG
    je      error
    ; Lower the upper bound below BX to require #BR.
    mov     word [0x202], 0x00FF
    bound   bx, [0x200]
    cmp     ax, BOUND_HANDLER_SIG
    jne     error
    setProtModeIntGate EX_BR, DefaultExcHandler

    advTestSegProt
    jmp     post19

BoundExcHandler:
    ; Raise the upper bound to BX, then retry BOUND through IRET.
    ; AX records that the handler ran; the retried instruction must succeed.
    mov     word [0x202], 0x0100
    BOUND_HANDLER_SIG equ 0x626F
    mov     ax, BOUND_HANDLER_SIG
    iret
%include "tests/xchg_m.inc"
post19:
;-------------------------------------------------------------------------------
    POST 19
;-------------------------------------------------------------------------------
;
;   Exchange Register/Memory with Register (XCHG)
;
    testXchg ax,cx                      ; 91
    testXchg ax,dx                      ; 92
    testXchg ax,bx                      ; 93
    mov     bp,sp
    testXchg ax,sp                      ; 94
    mov     sp,bp
    testXchg ax,bp                      ; 95
    testXchg ax,si                      ; 96
    testXchg ax,di                      ; 97
    testXchg bl,cl                      ; 86 D9
    testXchg byte [0],cl                ; 86 0E 0000
    testXchg bx,cx                      ; 87 D9
    testXchg word [0],cx                ; 87 0E 0000
    advTestSegProt


%include "tests/enter_m.inc"
post1A:
;-------------------------------------------------------------------------------
    POST 1A
;-------------------------------------------------------------------------------
;
;   Make Stack Frame for Procedure Parameters (ENTER)
;
    testENTER16 1, 0,16
    testENTER16 2, 1,16
    testENTER16 3, 4,16
    testENTER16 4,36,16
    loadProtModeStack
%include "tests/leave_m.inc"
post1B:
;-------------------------------------------------------------------------------
    POST 1B
;-------------------------------------------------------------------------------
;
;   High Level Procedure Exit (LEAVE)
;
    testLEAVE 16
    loadProtModeStack
    jmp     post1C


%include "tests/ver_p.inc"
post1C:
;-------------------------------------------------------------------------------
    POST 1C
;-------------------------------------------------------------------------------
;
;   Verify a Segment for Reading or Writing (VERR/VERW)
;
    ; null segment
    testVERR NULL,0
    testVERW NULL,0
    ; out of bounds descriptor
    testVERR GDT_SEG_LIMIT+1,0
    testVERW GDT_SEG_LIMIT+1,0
    ; system segment
    testVERR LDT_SEG_PROT,0
    testVERW LDT_SEG_PROT,0

    ; CPL 0
    ; code segment
    testVERR C_SEG_PROT16,1
    testVERW C_SEG_PROT16,0
    ; data segment
    testVERR S_SEG_PROT16,1
    testVERW S_SEG_PROT16,1

    ; CPL 3
    call    switchToRing3
    ; non-conforming code segment
    testVERR C_SEG_PROT16,0             ; DPL0 code segment
    testVERW C_SEG_PROT16,0
    testVERR CU_SEG_PROT16,1            ; DPL3 code segment
    testVERW CU_SEG_PROT16,0
    ; conforming code segment
    testVERR CC_SEG_PROT16,1
    testVERW CC_SEG_PROT16,0
    ; data segment
    testVERR S_SEG_PROT16,0             ; DPL0 stack segment
    testVERW S_SEG_PROT16,0
    testVERR SU_SEG_PROT16,1            ; DPL3 stack segment
    testVERW SU_SEG_PROT16,1
    testVERR ROU_SEG_PROT,1             ; DPL3 read-only data segment
    testVERW ROU_SEG_PROT,0
    call    switchToRing0

    jmp     arithLogicTests
%include "print_p.inc"
%include "tests/bcd_m.inc"
%include "tests/arith-logic_m.inc"

arithLogicTests:
;-------------------------------------------------------------------------------
    POST EE
;-------------------------------------------------------------------------------
;
;   Run arithmetic and logical instruction tests and print their results.
;   Compare the output with a reference captured from the physical 286.
;
    cld
%if TRACE_ARITH
; As test386, these observations are compared with a hardware reference;
; they are not self-checking assertions.
bcdTests:
    testBCD   daa, 0x0503, PS_AF,         PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   daa, 0x0506, PS_AF,         PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   daa, 0x0507, PS_AF,         PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   daa, 0x0559, PS_AF,         PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   daa, 0x0560, PS_AF,         PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   daa, 0x059f, PS_AF,         PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   daa, 0x05a0, PS_AF,         PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   daa, 0x0503, 0,             PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   daa, 0x0506, 0,             PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   daa, 0x0503, PS_CF,         PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   daa, 0x0506, PS_CF,         PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   daa, 0x0503, PS_CF | PS_AF, PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   daa, 0x0506, PS_CF | PS_AF, PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   das, 0x0503, PS_AF,         PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   das, 0x0506, PS_AF,         PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   das, 0x0507, PS_AF,         PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   das, 0x0559, PS_AF,         PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   das, 0x0560, PS_AF,         PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   das, 0x059f, PS_AF,         PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   das, 0x05a0, PS_AF,         PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   das, 0x0503, 0,             PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   das, 0x0506, 0,             PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   das, 0x0503, PS_CF,         PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   das, 0x0506, PS_CF,         PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   das, 0x0503, PS_CF | PS_AF, PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   das, 0x0506, PS_CF | PS_AF, PS_CF | PS_PF | PS_ZF | PS_SF | PS_AF
    testBCD   aaa, 0x0205, PS_AF,         PS_CF | PS_AF
    testBCD   aaa, 0x0306, PS_AF,         PS_CF | PS_AF
    testBCD   aaa, 0x040a, PS_AF,         PS_CF | PS_AF
    testBCD   aaa, 0x05fa, PS_AF,         PS_CF | PS_AF
    testBCD   aaa, 0x0205, 0,             PS_CF | PS_AF
    testBCD   aaa, 0x0306, 0,             PS_CF | PS_AF
    testBCD   aaa, 0x040a, 0,             PS_CF | PS_AF
    testBCD   aaa, 0x05fa, 0,             PS_CF | PS_AF
    testBCD   aas, 0x0205, PS_AF,         PS_CF | PS_AF
    testBCD   aas, 0x0306, PS_AF,         PS_CF | PS_AF
    testBCD   aas, 0x040a, PS_AF,         PS_CF | PS_AF
    testBCD   aas, 0x05fa, PS_AF,         PS_CF | PS_AF
    testBCD   aas, 0x0205, 0,             PS_CF | PS_AF
    testBCD   aas, 0x0306, 0,             PS_CF | PS_AF
    testBCD   aas, 0x040a, 0,             PS_CF | PS_AF
    testBCD   aas, 0x05fa, 0,             PS_CF | PS_AF
    testBCD   aam, 0x0547, PS_AF,         PS_PF | PS_ZF | PS_SF
    testBCD   aad, 0x0407, PS_AF,         PS_PF | PS_ZF | PS_SF

    setProtModeIntGate EX_DE, DivExcHandler
    cld
    ; Arithmetic and logical tests are a sequence of procedures in tableOps
    ; (see tests/arith-logic_d.inc). Each entry starts with length, TYPE and
    ; SIZE bytes, followed by a zero-terminated name and an instruction sequence.
    mov     si, tableOps                ; SI -> tableOps entry
testOps:
    xor     cx, cx
    mov     cl, [cs:si]                 ; CX == instruction-sequence length
    test    cx, cx
    jz      testDone                    ; Zero length marks the end of the table
    xor     bx, bx
    mov     bl, [cs:si+1]               ; BX == TYPE
    shl     bx, 4                       ; TYPE * 16 (two 8-byte width records)
    xor     dx, dx
    mov     dl, [cs:si+2]               ; DX == SIZE (byte or word)
    shl     dx, 3                       ; SIZE * 8
    add     bx, dx
    add     bx, typeValues              ; BX -> operand values for this TYPE/SIZE
    add     si, 3                       ; SI -> instruction name
.skip:
    ; Skip the name so SI points at the executable instruction sequence.
    cs lodsb
    test    al, al
    jnz     .skip
    push    cx                          ; Preserve the sequence length
    mov     cx, [cs:bx]                 ; CX == destination-value count
    mov     ax, [cs:bx+2]               ; AX -> destination values
    mov     bp, [cs:bx+4]               ; BP == source-value count
    mov     di, [cs:bx+6]               ; DI -> source values
    xchg    bx, ax                      ; BX -> destination values
    sub     ax, ax                      ; Initialize arithmetic flags before this test
testDst:
    ; For each destination value, run through the complete source-value set.
    push    bp
    push    di
    pushf
testSrc:
    mov     ax, [cs:bx]                 ; AX == destination input
    mov     dx, [cs:di]                 ; DX == source input
    popf                                ; Restore flags before printing/execution
    call    printOp
    call    printAX
    call    printDX
    call    printPS
    call    si                          ; Execute the instruction sequence
    call    printAX
    call    printDX
    call    printPS
    call    printEOL
    pushf                               ; Carry result flags into the next case
    add     di, 2                       ; DI -> next source value (stored as a word)
    dec     bp                          ; Decrement the source-value count
    jnz     testSrc
    popf
    pop     di                          ; Restore the first source-value pointer
    pop     bp                          ; Restore the source-value count
    lea     bx, [bx+2]                  ; Next destination without changing flags
    loop    testDst
    pop     cx                          ; Restore the instruction-sequence length
    add     si, cx                      ; SI -> next tableOps entry
    jmp     testOps
testDone:
    jmp     postFF

DivExcHandler:
    push    si
    mov     si, strDE
    call    printStr
    pop     si
    ; The 286 reports #DE as a fault: saved CS:IP points at the divide, not
    ; the RET placed after it. Instead of decoding the faulting instruction's
    ; length, redirect the saved IP to our own RET, as test386 does.
    ; Saving BP puts the fault IP at SS:BP+2 in the 16-bit interrupt frame.
    push    bp
    mov     bp, sp
    mov     word [ss:bp+2], DivExcHandlerRet
    pop     bp
    iret
DivExcHandlerRet:
    ret                                 ; Return to the arithmetic observation loop
%else
    jmp     postFF
%endif
%include "tests/arith-logic_d.inc"

postFF:
;
;   Testing finished. Report success and stop.
;
    ; Harness addition: explicit terminal text, followed by the test386 POST.
    cld
    mov     si, passText
    call    printStr
    POST FF
    cli
    hlt
    jmp     postFF
passText: db 'PASS',13,10,0

;
; Default exception handler and error routine.
;
DefaultExcHandler:
error:
    ; CLI, OUT and HLT are privileged at CPL 3 with IOPL 0. Enter the kernel
    ; through the failure gate before reporting and halting. Real mode and
    ; CPL-0 callers can use the kernel path directly.
    mov     ax, cs
    test    ax, 3
    jz      .kernel
    int     0x2F
.kernel:
    cli
%if OUT_PORT
    ; Stack-free output remains usable if a stack test itself failed.
    mov     dx, OUT_PORT
    mov     al, 'F'
    out     dx, al
    mov     al, 'A'
    out     dx, al
    mov     al, 'I'
    out     dx, al
    mov     al, 'L'
    out     dx, al
    mov     al, 13
    out     dx, al
    mov     al, 10
    out     dx, al
%endif
    hlt
    jmp     error

signedWord: db 0x80
signedByte: db 0x80
    ; Fill to ROM offset FFF0h with FFh for sparse upload to the ArduinoX86.
    times   0xFFF0-($-$$) db 0xFF
resetVector:
    jmp     C_SEG_REAL:cpuTest          ; FFF0h: RESET entry point

release:
    db      RELEASE,0                   ; FFF5h: MM/DD/YY release date and NUL
    db      0xFC,0                      ; FFFEh: model ID; FFFFh: unused checksum byte
