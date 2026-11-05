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
