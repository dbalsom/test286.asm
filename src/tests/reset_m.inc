; SPDX-License-Identifier: GPL-3.0-or-later
;
; Optional system-assisted return from protected mode through shutdown/RESET.
; The system must assert RESET on triple fault and preserve RAM. Reserve the
; six bytes immediately above the IDT; clear them before a new, independent run.
;
; Harris RESET microcode u000-u014 has no decoded write to general registers.
; Its explicit writes initialize flags, segment/system state and temporaries.
; This test checks that hypothesis for AX,CX,DX,BX,SP,BP,SI,DI with two
; complementary patterns. It does not assume any power-on register contents.

RESET_RECORD equ 0x0580
RESET_MAGIC0 equ 0x286A
RESET_MAGIC1 equ 0xD795

; Called at the reset-vector destination before any GPR or stack use.
; RESET supplies DS=0. CMP, conditional branches and memory-immediate stores
; preserve every general register. The record is consumed by testReset286.
; Uses: Flags, RAM at RESET_RECORD; preserves all general registers.
%macro resetResumeDispatch286 0
    cmp     word [RESET_RECORD], RESET_MAGIC0
    jne     %%cold
    cmp     word [RESET_RECORD+2], RESET_MAGIC1
    jne     %%cold
    cmp     word [RESET_RECORD+4], 1
    je      resetResumeFirst286
    cmp     word [RESET_RECORD+4], 2
    je      resetResumeSecond286
    jmp     error
%%cold:
    mov     word [RESET_RECORD], 0
    mov     word [RESET_RECORD+2], 0
    mov     word [RESET_RECORD+4], 0
%endmacro

; Load a distinct pattern into each general register.
; %1: XOR mask, 0000h for the first run and FFFFh for its complement.
; No instruction after this macro uses the stack or changes a GPR before
; the failed interrupt delivery. In particular, POST must precede these MOVs.
%macro loadResetRegisters286 1
    mov     ax, 0x1357 ^ %1
    mov     cx, 0x2468 ^ %1
    mov     dx, 0x369C ^ %1
    mov     bx, 0x48AD ^ %1
    mov     sp, 0x5ABE ^ %1
    mov     bp, 0x6BCF ^ %1
    mov     si, 0x7CD0 ^ %1
    mov     di, 0x8DE2 ^ %1
%endmacro

; Compare the reset register file before changing a GPR or using the stack.
; %1: the same XOR mask passed to loadResetRegisters286.
; After all eight comparisons, SMSW verifies that RESET cleared PE.
; Uses: AX and Flags, but only after every preservation check has passed.
%macro checkResetRegisters286 1
    cmp     ax, 0x1357 ^ %1
    jne     error
    cmp     cx, 0x2468 ^ %1
    jne     error
    cmp     dx, 0x369C ^ %1
    jne     error
    cmp     bx, 0x48AD ^ %1
    jne     error
    cmp     sp, 0x5ABE ^ %1
    jne     error
    cmp     bp, 0x6BCF ^ %1
    jne     error
    cmp     si, 0x7CD0 ^ %1
    jne     error
    cmp     di, 0x8DE2 ^ %1
    jne     error
    smsw    ax
    test    ax, CR0_MSW_PE
    jnz     error
%endmacro

; Enter protected mode with a private three-entry GDT, then provoke shutdown
; using an IDT whose limit excludes INT 3, #GP and #DF. Gate lookup fails
; before an interrupt frame is pushed, so the tested SP is not consumed.
;
; RESET returns through resetResumeDispatch286. Two runs exercise both values
; of every GPR bit. After the second, consume the continuation record and
; continue in real mode. The caller must initialize its stack before using it.
; Uses: GPRs, Flags, segment registers, GDTR/IDTR/MSW, RAM 0580h-0585h and
; 0600h-0617h. Requires inactive external interrupt inputs and CPL 0 on entry.
%macro testReset286 0
    xor     ax, ax
    mov     ds, ax
    mov     word [RESET_RECORD], RESET_MAGIC0
    mov     word [RESET_RECORD+2], RESET_MAGIC1
    mov     word [RESET_RECORD+4], 1
    jmp     %%prepare

resetResumeFirst286:
    checkResetRegisters286 0
    mov     word [RESET_RECORD+4], 2
    jmp     %%prepare

resetResumeSecond286:
    checkResetRegisters286 0xFFFF
    mov     word [RESET_RECORD], 0
    mov     word [RESET_RECORD+2], 0
    mov     word [RESET_RECORD+4], 0
    jmp     %%done

%%prepare:
    cli
    cld
    mov     ax, cs
    mov     ds, ax
    xor     ax, ax
    mov     es, ax
    mov     si, %%gdt
    mov     di, 0x0600
    mov     cx, 12
    rep movsw
    lgdt    [cs:%%gdtr]
    lidt    [cs:%%emptyIDT]
    mov     ax, CR0_MSW_PE
    lmsw    ax
    jmp     0x0008:%%protected

%%protected:
    mov     ax, 0x0010
    mov     ds, ax
    mov     ss, ax
    smsw    ax
    test    ax, CR0_MSW_PE
    jz      error
    cmp     word [RESET_RECORD+4], 1
    je      %%first
    cmp     word [RESET_RECORD+4], 2
    je      %%second
    jmp     error
%%first:
    loadResetRegisters286 0
    jmp     %%fault
%%second:
    loadResetRegisters286 0xFFFF
%%fault:
    int3
    jmp     error                       ; Fault delivery must not return here

%%gdt:
    dq      0                           ; Null descriptor
    dw      0xFFFF, 0x0000              ; Code: F0000h base, FFFFh limit
    db      0x0F, 0x9B                  ; Present/readable, accessed
    dw      0                           ; Reserved on the 286
    dw      0xFFFF, 0x0000              ; Data/stack: base zero, FFFFh limit
    db      0x00, 0x93                  ; Present/writable, accessed
    dw      0
%%gdtr:
    dw      23                          ; Three descriptors, inclusive limit
    dd      0x0600
%%emptyIDT:
    dw      0                           ; Even vector 0 cannot fit
    dd      0
%%done:
%endmacro
