; SPDX-License-Identifier: GPL-3.0-or-later

strAX:    db  "AX=",0
strDX:    db  "DX=",0
strPS:    db  "PS=",0
strDE:    db  "#DE ",0 ; when this is displayed, it indicates a Divide Error exception
achSize   db  "BW"

;
;   printEOL()
;
;   Uses: None
;
printEOL:
    push    ax
;   mov     al,0x0d
;   call    printChar
    mov     al,0x0a
    call    printChar
    pop     ax
    ret

;
;   printChar(AL)
;
;   Uses: None
;
printChar:
    push    dx
    ; The 286 fixture uses a write-only output port. No UART/LPT polling is
    ; needed; MOV, OUT and PUSH/POP leave FLAGS and AX unchanged.
    %if OUT_PORT
    mov     dx, OUT_PORT
    out     dx, al
    %endif
    pop     dx
    ret

;
;   printStr(si -> zero-terminated string)
;
;   Uses: si, Flags
;
printStr:
    push    ax
.loop:
    cs lodsb
    test    al, al
    jz      .done
    call    printChar
    jmp     .loop
.done:
    pop     ax
    ret

;
;   printVal(ax == value, CL == number of hex digits)
;
;   Uses: ax, cx, Flags
;
printVal:
    shl     cl, 2                       ; CL == number of bits (4 times the number of hex digits)
    jz      .done
.loop:
    sub     cl, 4
    push    ax
    shr     ax, cl
    and     al, 0x0f
    add     al, '0'
    cmp     al, '9'
    jbe     .digit
    add     al, 'A'-'0'-10
.digit:
    call    printChar
    pop     ax
    test    cl, cl
    jnz     .loop
.done:
    mov     al, ' '
    call    printChar
    ret

;
;   printOp(si -> instruction sequence)
;
;   Rewinds si to the start of the mnemonic preceding the instruction sequence and prints the mnemonic
;
;   Uses: None
;
printOp:
    pushf
    pusha
.findSize:
    dec     si
    mov     al, [cs:si-1]
    cmp     al, 32
    jae     .findSize
    call    printStr
    xor     ah, ah
    mov     bx, ax
    mov     al, [cs:achSize+bx]
    call    printChar
    mov     al, ' '
    call    printChar
    popa
    popf
    ret

;
;   printAX()
;
;   Uses: None
;
printAX:
    pushf
    pusha
    mov     si, strAX
    call    printStr
    mov     cl, 4
    call    printVal
    popa
    popf
    ret

;
;   printDX()
;
;   Uses: None
;
printDX:
    pushf
    pusha
    mov     si, strDX
    call    printStr
    mov     cl, 4
    mov     ax, dx
    call    printVal
    popa
    popf
    ret

;
;   printPS(si -> instruction sequence)
;
;   Uses: None
;
printPS:
    pushf
    pusha
    pushf
    pop     dx
.findType:
    dec     si
    mov     al, [cs:si-1]
    cmp     al, 32
    jae     .findType
    xor     bx, bx
    mov     bl, [cs:si-2]
    shl     bx, 1
    and     dx, [cs:typeMasks+bx]
    mov     si, strPS
    call    printStr
    mov     cl, 4
    mov     ax, dx
    call    printVal
    popa
    popf
    ret

;
;   printPS2
;   ax -> mask
;
;   Uses: None
;
printPS2:
    pushf
    pusha
    pushf
    pop     dx
    and     dx, ax
    mov     si, strPS
    call    printStr
    mov     cl, 4
    mov     ax, dx
    call    printVal
    popa
    popf
    ret
