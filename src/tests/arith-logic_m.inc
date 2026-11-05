; SPDX-License-Identifier: GPL-3.0-or-later

; Operation record macros retained from test386 tests/arith-logic_d.asm.
; Separated from the data file at the project owner's request.

; Defines a logic/arithmetic operation with 1 to 3 operands.
; %1: name string.
; %2: mnemonic.
; %3: "al", "ax", "dl", or "dx".
; %4: source operand / immediate / "mem" / "none".
; %5: third operand / "none".
; %6: TYPE_* operation category.
;
; Emits the length/type/size record, zero-terminated name and instruction
; sequence ending in RET. For "mem", AL/AX selects the width and DL/DX is
; first stored at DS:0. The record is an observation case, not an assertion.
;
%macro defOp 6
    %ifidni %3,al
    %assign size SIZE_BYTE
    %define msrc dl
    %elifidni %3,dl
    %assign size SIZE_BYTE
    %elifidni %3,ax
    %assign size SIZE_SHORT
    %define msrc dx
    %elifidni %3,dx
    %assign size SIZE_SHORT
    %endif
    db      %%end-%%beg,%6,size
%%name:
    db      %1,' ',0
%%beg:
    %ifidni %4,none
    %2      %3
    %elifidni %4,mem
    mov     [0], msrc
    %2      %3,[0]
    %elifidni %5,none
    %2      %3,%4
    %else
    %2      %3,%4,%5
    %endif
    ret
%%end:
%endmacro

; Defines a logic/arithmetic operation with 0 operands.
; %1: name string.
; %2: mnemonic.
; %3: operand-domain size as "b" or "w".
; %4: TYPE_* operation category.
;
; Emits an operation record and a zero-operand instruction followed by RET.
;
%macro defOp0 4
    %ifidni %3,b
    %assign size SIZE_BYTE
    %elifidni %3,w
    %assign size SIZE_SHORT
    %endif
    db      %%end-%%beg,%4,size
%%name:
    db      %1,' ',0
%%beg:
    %2
    ret
%%end:
%endmacro

; Defines a shift or rotate operation.
; %1: name string.
; %2: mnemonic.
; %3: register operand, "al" or "ax".
; %4: "cl" or immediate count.
; %5: TYPE_* operation category.
;
; The generated sequence sets CF before execution. For a variable count,
; DL is exchanged with CL before and after the operation, preserving the
; observation loop's CX counter.
;
%macro defOpSh 5
    %ifidni %3,al
    %assign size SIZE_BYTE
    %elifidni %3,ax
    %assign size SIZE_SHORT
    %endif
    db      %%end-%%beg,%5,size
%%name:
    db      %1,' ',0
%%beg:
    stc
    %ifidni %4,cl
    xchg    cl,dl
    %2      %3,cl
    xchg    cl,dl
    %else
    %2      %3,%4
    %endif
    ret
%%end:
%endmacro

; Defines an INC or DEC operation.
; %1: name string.
; %2: mnemonic.
; %3: register or "mem".
; %4: size as "byte" or "word".
;
; The case uses TYPE_ARITH1. AL/AX supplies the input and receives the result;
; register cases exchange it with the tested register, and memory cases use
; DS:0. Other tested registers are restored by the second exchange.
;
%macro  defOpInc 4
    %ifidni %4,byte
    %assign size SIZE_BYTE
    %define accumulator al
    %elifidni %4,word
    %assign size SIZE_SHORT
    %define accumulator ax
    %endif
    db      %%end-%%beg,TYPE_ARITH1,size
%%name:
    db      %1,' ',0
%%beg:
    %ifidni %3,mem
        mov     [0], accumulator
        %2      %4 [0]
        mov     accumulator, [0]
    %else
        xchg    accumulator, %3
        %2      %3
        xchg    accumulator, %3
    %endif
    ret
%%end:
%endmacro
