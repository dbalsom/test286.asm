; SPDX-License-Identifier: GPL-3.0-or-later

; test386 stack tests, retaining the 16-bit operand/stack paths.
; DS and SS must address the same test segment. SP starts at zero to exercise
; wrapping. The neighboring word checks remain, using two 16-bit accesses.

; Tests 16-bit PUSH/POP for general purpose registers.
; 50+rw PUSH r16
; 58+rw POP r16
;
; %1: register to be tested; ax, bx, cx, dx, bp, si, di, or sp.
; %2: stack address size: 16.
;
; Checks the pushed word, the unchanged neighboring word and the updated SP.
; POP SP is checked for its loaded value, rather than an ordinary SP increment.
;
%macro testPushPopR 2
    %ifidni %1,bp
        %define sptr bx
    %else
        %define sptr bp
    %endif
    %define r16 %1
    mov     sp, 0
    mov     r16, 0
    mov     sptr, 0xFFFC
    mov     word [sptr], 0xBEEF
    mov     word [sptr+2], 0xDEAD
    push    r16                         ; 16-bit PUSH
    cmp     word [sptr], 0xBEEF         ; was the neighboring word left unchanged?
    jne     error
    cmp     word [sptr+2], 0            ; was the selected word pushed?
    jne     error
    cmp     sp, 0xFFFE
    jne     error
    mov     word [sptr], 0xBEEF
    mov     word [sptr+2], 0xDEAD
    pop     r16
    cmp     r16, 0xDEAD
    jne     error
    ; POP SP loads its operand into SP; other POPs should restore SP to zero.
    %ifnidni %1,sp
        cmp     sp, 0
        jne     error
    %endif
%endmacro

; Tests 16-bit PUSH/POP for segment registers.
; 0E PUSH CS
; 1E PUSH DS       1F POP DS
; 16 PUSH SS       17 POP SS
; 06 PUSH ES       07 POP ES
;
; %1: register to be tested; cs, ds, ss, or es.
; %2: stack address size: 16.
;
; CS is pushed but not popped. Other segment registers are restored after
; loading DTEST_SEG_PROT from the stack. Checks the neighboring word and SP.
;
%macro testPushPopSR 2
    mov     dx, %1
    mov     sp, 0
    mov     bp, 0xFFFC
    mov     word [bp], 0xBEEF
    mov     word [bp+2], 0xDEAD
    push    %1
    cmp     word [bp], 0xBEEF
    jne     error
    cmp     [bp+2], dx
    jne     error
    cmp     sp, 0xFFFE
    jne     error
    %ifnidni %1,cs
        mov     word [bp+2], DTEST_SEG_PROT
        pop     %1
        mov     ax, %1
        cmp     ax, DTEST_SEG_PROT
        jne     error
        cmp     sp, 0
        jne     error
        mov     %1, dx
    %else
        mov     sp, 0
    %endif
%endmacro

; Tests 16-bit PUSH/POP with a memory operand.
; FF /6 PUSH r/m16
; 8F /0 POP r/m16
;
; %1: stack address size: 16.
;
; DS:SI addresses the memory operand; SS:BP addresses the stack guard words.
; Checks that only the selected word is copied and that SP is updated.
;
%macro testPushPopM 1
    mov     sp, 0
    mov     bp, 0xFFFC
    mov     si, 0xFFF8
    mov     word [si], 0x3344
    mov     word [si+2], 0x1122
    mov     word [bp], 0xBEEF
    mov     word [bp+2], 0xDEAD
    push    word [si]
    cmp     word [bp], 0xBEEF
    jne     error
    cmp     word [bp+2], 0x3344
    jne     error
    cmp     sp, 0xFFFE
    jne     error
    mov     word [si], 0xBEEF
    mov     word [si+2], 0xDEAD
    pop     word [si]
    cmp     word [si], 0x3344
    jne     error
    cmp     word [si+2], 0xDEAD
    jne     error
    cmp     sp, 0
    jne     error
%endmacro

; Tests 16-bit PUSHA/POPA.
; 60 PUSHA
; 61 POPA
;
; %1: stack address size: 16.
;
; Checks the saved order AX, CX, DX, BX, original SP, BP, SI and DI.
; The saved SP slot is replaced with a bogus value before POPA to check
; that POPA skips it rather than loading it into SP.
;
%macro testPushPopAll16 1
    mov     sp, 0
    mov     bp, 0xFFFE
    mov     cx, 8
    mov     di, bp
%%initstack:
    mov     word [di], 0xBEEF
    sub     di, 2
    loop    %%initstack
    mov     ax, 0x1111
    mov     cx, 0x2222
    mov     dx, 0x3333
    mov     bx, 0x4444
    mov     si, 0x7777
    mov     di, 0x8888
    ; Push order: AX, CX, DX, BX, original SP, BP, SI, DI.
    pusha
    cmp     [bp], ax
    jne     error
    cmp     [bp-2], cx
    jne     error
    cmp     [bp-4], dx
    jne     error
    cmp     [bp-6], bx
    jne     error
    cmp     word [bp-8], 0
    jne     error
    cmp     [bp-10], bp
    jne     error
    cmp     [bp-12], si
    jne     error
    cmp     [bp-14], di
    jne     error
    cmp     sp, 0xFFF0
    jne     error
    ; Put a bogus value in the saved-SP slot to detect an incorrect POPA load.
    mov     word [bp-8], 0xBEEF
    mov     ax, 0xBEEF
    mov     cx, 0xBEEF
    mov     dx, 0xBEEF
    mov     bx, 0xBEEF
    mov     bp, 0xBEEF
    mov     si, 0xBEEF
    mov     di, 0xBEEF
    popa
    cmp     ax, 0x1111
    jne     error
    cmp     cx, 0x2222
    jne     error
    cmp     dx, 0x3333
    jne     error
    cmp     bx, 0x4444
    jne     error
    cmp     sp, 0
    jne     error
    cmp     bp, 0xFFFE
    jne     error
    cmp     si, 0x7777
    jne     error
    cmp     di, 0x8888
    jne     error
%endmacro

; Tests PUSH of a sign-extended immediate byte.
; 6A ib PUSH imm8
;
; %1: byte test value; use signed notation for negative imm8 values.
; %2: expected word at SS:BP+2.
; %3: expected SP after the push.
;
; The caller sets BP to FFFCh. The word at SS:BP must remain BEEFh.
;
%macro testBytePush 3
    mov     sp, 0
    mov     word [bp], 0xBEEF
    mov     word [bp+2], 0xDEAD
    push    byte %1                     ; sign-extended byte push
    cmp     word [bp], 0xBEEF
    jne     error
    cmp     word [bp+2], %2
    jne     error
    cmp     sp, %3
    jne     error
%endmacro

; Tests PUSH of immediate word and sign-extended byte values.
; 68 iw PUSH imm16
; 6A ib PUSH imm8
;
; %1: stack address size: 16.
;
; Checks the pushed word, the neighboring BEEFh guard word, and SP.
;
%macro testPushImm 1
    mov     sp, 0
    mov     bp, 0xFFFC
    mov     word [bp], 0xBEEF
    mov     word [bp+2], 0xDEAD
    push    word 0x1122
    cmp     word [bp], 0xBEEF
    jne     error
    cmp     word [bp+2], 0x1122
    jne     error
    cmp     sp, 0xFFFE
    jne     error
    testBytePush 0x11, 0x0011, 0xFFFE
    testBytePush -127, 0xFF81, 0xFFFE
%endmacro

; Tests POPF stack use and the selected arithmetic flag bits.
; 9D POPF
;
; %1: operand size: 16.
;
; The caller sets BP to FFFCh and provides a usable stack. The test writes
; 08D7h (OF,SF,ZF,AF,PF,CF and the fixed bit 1), then checks OF, LAHF and SP.
; This is not a privilege/IOPL test of POPF.
;
%macro testPopF 1
    push    word 0
    popf
    mov     word [bp+2], 0x08D7         ; OF,SF,ZF,AF,PF,CF and fixed bit 1
    mov     sp, 0xFFFE
    popf
    jno     error
    lahf
    cmp     ah, 0xD7                    ; SF:ZF:0:AF:0:PF:1:CF
    jne     error
    cmp     sp, 0
    jne     error
%endmacro

; Tests PUSHF stack use and the selected flag bits.
; 9C PUSHF
;
; %1: operand size: 16.
;
; The caller sets BP to FFFCh and runs with permission to change IF.
; After clearing writable flags, sets CF, DF and IF and checks the pushed
; 0603h word, the neighboring BEEFh guard word, and SP.
; This is not a privilege/IOPL test of PUSHF.
;
%macro testPushF 1
    push    word 0
    popf
    jo      error
    jc      error
    jz      error
    js      error
    jpe     error
    mov     sp, 0
    mov     word [bp], 0xBEEF
    mov     word [bp+2], 0xDEAD
    stc
    std
    sti
    pushf
    cmp     word [bp], 0xBEEF
    jne     error
    cmp     word [bp+2], 0x0603
    jne     error
    cmp     sp, 0xFFFE
    jne     error
%endmacro

; Tests PUSHF/POPF with a 16-bit stack.
;
; %1: stack address size: 16.
;
; Sets up the stack guard and invokes testPushF and testPopF. These checks
; cover stack use and selected flag values, not privilege/IOPL behavior.
;
%macro testPushPopF 1
    mov     sp, 0
    mov     bp, 0xFFFC
    testPushF 16
    testPopF 16
%endmacro
