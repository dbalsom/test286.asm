; SPDX-License-Identifier: GPL-3.0-or-later

; ENTER imm16, imm8
;
; OperandSize = 16, StackAddressSize = 16
; ************************************
;
; AllocSize <- imm16;
; NestingLevel <- imm8 MOD 32;
; Push16(BP);
; FrameTemp <- SP;
; IF NestingLevel > 0 THEN
;     IF NestingLevel > 1 THEN
;         FOR i <- 1 to (NestingLevel - 1) DO
;             BP <- BP - 2;
;             Push16([BP]);
;         OD;
;     FI;
;     Push16(FrameTemp);
; FI;
; BP <- FrameTemp;
; SP <- SP - AllocSize;
;
; %1: allocation size in bytes.
; %2: nesting level (the instruction uses its low five bits).
; %3: stack address size: 16.
;
; Requires DF=0 for clearing the stack fixture with REP STOSW. Loads SS and ES
; with D_SEG_PROT16, starts SP at zero, and checks SP, BP, the saved BP and
; the copied nesting chain. The caller must restore its normal stack.
;
%macro testENTER16 3
    %assign NestingLevel (%2 % 32)
    %assign stackWords 1 ; saved BP
    %if NestingLevel>0
        %if NestingLevel>1
            %assign stackWords stackWords+(NestingLevel-1) ; copied outer-frame words
        %endif
        %assign stackWords stackWords+1 ; saved FrameTemp
    %endif
    %assign stackBytes stackWords*2
    mov     ax, D_SEG_PROT16
    %assign bpVal    (0xfffe-(stackBytes-2))
    %assign bpResult  0xfffe
    %assign spResult (0xfffe-%1-(stackBytes-2))

    mov     ss, ax
    mov     es, ax

    ; clear stack memory
    mov     cx, stackWords
    xor     di, di
    sub     di, stackBytes
    xor     ax, ax
    rep stosw

    mov     bp, bpVal
    %if NestingLevel>1
        mov     cx, (NestingLevel-1)
        %%initbp:
        sub     bp, 2
        mov     [bp], cx

        loop    %%initbp
        mov     bp, bpVal
    %endif

    mov     sp, 0
    enter   %1, %2
    cmp     sp, spResult
    jne     error
    cmp     bp, bpResult
    jne     error
    cmp     [bp], word (bpVal&0xffff)
    jne     error
    %if NestingLevel>0
        add     sp, %1
        mov     bx, sp
        cmp     [es:bx], bp
        jne     error
        %if NestingLevel>1
            mov     cx, (NestingLevel-1)
            add     bx, 2+(NestingLevel-1)*2
            %%testNesting:
            sub     bx, 2
            cmp     [es:bx], cx
            jne     error
            loop    %%testNesting
        %endif
    %endif
%endmacro


