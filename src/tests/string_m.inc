; SPDX-License-Identifier: GPL-3.0-or-later
;
;   Tests store, compare, scan, move, and load string operands
;   %1 data size b=byte, w=word
;   %2 direction 0=increment, 1=decrement
;   DS test segment 1
;   ES test segment 2
;
%macro testStringOps 2

    %assign value 0x5678
    %ifidni %1,b
        %assign val_size 1
        %define val_mask 0x00ff
        %define sized_ax al
    %endif
    %ifidni %1,w
        %assign val_size 2
        %define val_mask 0xffff
        %define sized_ax ax
    %endif

    %if %2 == 0
        cld
        %assign off_value 0xffff-(val_size-1)
            ; 16-bit addressing
            %assign off_cmp 0x0000

    %else
        std
        %assign off_value 0x0000
            ; 16-bit addressing
            %assign off_cmp 0xffff-(val_size-1)

    %endif

        %assign off_mask 0xffff


    ; VERIFY string operands

    mov     di, off_value
    mov     bx, off_value & off_mask
    mov     sized_ax, 0
    mov     [es:bx], sized_ax
    mov     sized_ax, value & val_mask
    stos%1                              ; STORE AL/AX in ES:DI
    cmp     [es:bx], sized_ax
    jne     error
    cmp     di, off_cmp
    jne     error

    mov     si, off_value
    mov     di, off_value
    mov     bx, off_value & off_mask
    mov     [ds:bx], sized_ax
    mov     [es:bx], sized_ax
    cmp     sized_ax, 0
    je      error
    cmps%1                              ; COMPARE ES:di with DS:si
    jne     error
    cmp     di, off_cmp
    jne     error
    cmp     si, off_cmp
    jne     error

    mov     di, off_value
    mov     sized_ax, value & val_mask
    mov     [es:bx], sized_ax
    cmp     sized_ax, 0
    scas%1                              ; SCAN/COMPARE ES:DI with AL/AX
    jne     error
    cmp     di, off_cmp
    jne     error

    mov     si, off_value
    mov     di, off_value
    mov     sized_ax, value & val_mask
    mov     [ds:bx], sized_ax
    mov     sized_ax, 0
    mov     [es:bx], sized_ax
    movs%1                              ; MOVE data from DS:si to ES:di
    mov     sized_ax, value & val_mask
    cmp     [es:bx], sized_ax
    jne     error
    cmp     di, off_cmp
    jne     error
    cmp     si, off_cmp
    jne     error

    mov     si, off_value
    mov     sized_ax, value & val_mask
    mov     [es:bx], sized_ax
    xor     ax, ax
    lods%1                              ; LOAD data from DS:SI into AL/AX
    cmp     sized_ax, value & val_mask
    jne     error
    cmp     si, off_cmp
    jne     error

%endmacro

; Zero-count REP must leave AX, CX, SI, DI, FLAGS and the memory fixtures
; unchanged. Both operands are valid; this checks effects, not bus activity.
; %1: DF (0/1), %2: complete repeat-prefixed instruction.
; Run once with arithmetic flags clear and once with them set.
; Uses: AX, BX, CX, DX, BP, SI, DI, Flags and DS/ES:02FE-0303.
%macro testStringZeroCount 2+
    mov     dx, (%1*PS_DF)|2
    mov     bp, 2
%%again:
    mov     word [ds:0x2FE], 0x1357
    mov     word [ds:0x300], 0xA55A
    mov     word [ds:0x302], 0x2468
    mov     word [es:0x2FE], 0x3579
    mov     word [es:0x300], 0x5AA5
    mov     word [es:0x302], 0x468A
    mov     ax, 0x1234
    mov     si, 0x300
    mov     di, 0x300
    mov     cx, 0
    push    dx
    popf
    %2
    pushf                               ; Capture flags before any comparisons
    pop     bx
    cmp     bx, dx
    jne     error
    cmp     ax, 0x1234
    jne     error
    cmp     cx, 0
    jne     error
    cmp     si, 0x300
    jne     error
    cmp     di, 0x300
    jne     error
    cmp     word [ds:0x2FE], 0x1357
    jne     error
    cmp     word [ds:0x300], 0xA55A
    jne     error
    cmp     word [ds:0x302], 0x2468
    jne     error
    cmp     word [es:0x2FE], 0x3579
    jne     error
    cmp     word [es:0x300], 0x5AA5
    jne     error
    cmp     word [es:0x302], 0x468A
    jne     error
    xor     dx, PS_ARITH
    dec     bp
    jnz     %%again
%endmacro

; Stop CMPS/SCAS on the second of four elements, leaving CX=2.
; The initial ZF opposes the repeat condition: the first comparison must
; execute regardless. Compare 2 against 1 for a mismatch, or 2 for equality.
; %1: element size (b/w), %2: DF (0/1), %3: REPE/REPNE, %4: CMPS/SCAS.
; Uses: AX, BX, CX, SI, DI, Flags and DS/ES:02FE-0309.
%macro testStringCompareStop 4
    %ifidni %1, b
        %assign %%size 1
        %define %%operand byte
    %else
        %assign %%size 2
        %define %%operand word
    %endif
    %assign %%step (1-2*%2)*%%size
    %assign %%start 0x300+3*%2*%%size
    %ifidni %3, repe
        %assign %%first 2
        %assign %%second 1
        %assign %%initial 0
        %assign %%final 0
    %else
        %assign %%first 1
        %assign %%second 2
        %assign %%initial PS_ZF
        %assign %%final PS_ZF|PS_PF
    %endif
    mov     word [ds:0x2FE], 0x1357
    mov     word [es:0x2FE], 0x3579
    mov     word [ds:0x300+4*%%size], 0x2468
    mov     word [es:0x300+4*%%size], 0x468A
    %assign %%index 0
    %rep 4
    mov     %%operand [ds:%%start+%%index*%%step], 2
        %assign %%index %%index+1
    %endrep
    mov     %%operand [es:%%start], %%first
    mov     %%operand [es:%%start+%%step], %%second
    mov     %%operand [es:%%start+2*%%step], 3
    mov     %%operand [es:%%start+3*%%step], 3
    mov     ax, 2
    mov     si, %%start
    mov     di, %%start
    mov     cx, 4
    push    word %%initial|(%2*PS_DF)|2
    popf
    %3 %4%1
    pushf
    pop     bx
    cmp     bx, %%final|(%2*PS_DF)|2
    jne     error
    cmp     ax, 2
    jne     error
    cmp     cx, 2
    jne     error
    %ifidni %4, cmps
    cmp     si, %%start+2*%%step
    %else
    cmp     si, %%start                  ; SCAS must not change SI
    %endif
    jne     error
    cmp     di, %%start+2*%%step
    jne     error
    ; All source/destination elements and surrounding guards are read-only.
    %assign %%index 0
    %rep 4
    cmp     %%operand [ds:%%start+%%index*%%step], 2
    jne     error
        %assign %%index %%index+1
    %endrep
    cmp     %%operand [es:%%start], %%first
    jne     error
    cmp     %%operand [es:%%start+%%step], %%second
    jne     error
    cmp     %%operand [es:%%start+2*%%step], 3
    jne     error
    cmp     %%operand [es:%%start+3*%%step], 3
    jne     error
    cmp     word [ds:0x2FE], 0x1357
    jne     error
    cmp     word [es:0x2FE], 0x3579
    jne     error
    cmp     word [ds:0x300+4*%%size], 0x2468
    jne     error
    cmp     word [es:0x300+4*%%size], 0x468A
    jne     error
%endmacro

; Zero-count and early-termination cases for one size/direction combination.
; %1: element size (b/w), %2: DF (0/1).
%macro testStringEdges 2
    testStringZeroCount %2, rep movs%1
    testStringZeroCount %2, rep stos%1
    testStringZeroCount %2, rep lods%1
    testStringZeroCount %2, repe cmps%1
    testStringZeroCount %2, repne cmps%1
    testStringZeroCount %2, repe scas%1
    testStringZeroCount %2, repne scas%1
    testStringCompareStop %1, %2, repe, cmps
    testStringCompareStop %1, %2, repne, cmps
    testStringCompareStop %1, %2, repe, scas
    testStringCompareStop %1, %2, repne, scas
%endmacro

;
;   Tests store, compare, scan, and move string operands with repetitions
;   %1 element size b=byte, w=word
;   %2 direction 0=increment, 1=decrement
;   DS test segment 1
;   ES test segment 2
;
%macro testStringReps 2

    %assign bytes 0x100

    %ifidni %1,b
        %assign items bytes
    %endif
    %ifidni %1,w
        %assign items bytes/2
    %endif

    %if %2 == 0
        cld
        %assign off_value 0xff00
            ; 16-bit addressing
            %assign off_cmp 0x0000

    %else
        std
        %assign off_value 0x00ff
            ; 16-bit addressing
            %assign off_cmp 0xffff

    %endif

    mov     ax, 0x5678
    mov     si, off_value
    mov     di, off_value

    ; VERIFY REPs on memory buffers

    ; STORE buffers with pattern in ax
    mov     ax, 0x5678
    mov     si, off_value
    mov     di, off_value
    mov     cx, items
    rep stos%1                          ; store cx items at ES:di with the value in ax
    cmp     cx, 0
    jnz     error                       ; cx must be 0
    cmp     di, off_cmp
    jnz     error
    mov     di, off_value               ; reset di
    ; now switch ES:di with DS:si
    mov     dx, es
    mov     cx, ds
    xchg    dx, cx
    mov     es, dx
    mov     ds, cx
    xchg    di, si
    ; store again ES:di with pattern in ax
    mov     cx, items                   ; reset cx
    rep stos%1
    mov     di, off_value               ; reset di

    ; COMPARE two buffers
    mov     cx, items                   ; reset cx
    repe cmps%1                         ; find nonmatching items in ES:di and DS:si
    cmp     cx, 0
    jnz     error                       ; cx must be 0
    cmp     si, off_cmp
    jne     error
    cmp     di, off_cmp
    jne     error
    mov     di, off_value               ; reset di
    mov     si, off_value               ; reset si

    ; SCAN buffer for pattern
    mov     cx, items                   ; reset cx
    repe scas%1                         ; SCAN for the first byte/word not equal to AL/AX
    cmp     cx, 0
    jne     error                       ; cx must be 0
    cmp     di, off_cmp
    jne     error
    mov     di, off_value               ; rewind di

    ; MOVE and COMPARE data between buffers
    ; first zero-fill ES:di so that we can compare the moved data later
    mov     ax, 0
    mov     cx, items                   ; reset cx
    rep stos%1                          ; zero fill ES:di
    mov     di, off_value               ; reset di
    mov     cx, items                   ; reset cx
    rep movs%1                          ; MOVE data from DS:si to ES:di
    cmp     cx, 0
    jne     error                       ; cx must be 0
    cmp     si, off_cmp
    jne     error
    cmp     di, off_cmp
    jne     error
    mov     cx, items                   ; reset cx
    mov     di, off_value               ; reset di
    mov     si, off_value               ; reset si
    repe cmps%1                         ; COMPARE moved data in ES:di with DS:si
    cmp     cx, 0
    jne     error                       ; cx must be 0
    cmp     si, off_cmp
    jne     error
    cmp     di, off_cmp
    jne     error
%endmacro
