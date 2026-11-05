; SPDX-License-Identifier: GPL-3.0-or-later
;
; Tests conditional relative jumps.
; %1: relative displacement size, 8 (the 286 has only short Jcc).
; Uses: AX, CX, Flags
;
; Opcodes tested, with positive and negative offsets:
;
; rel8          mnemonic condition
; 70             JO       OF=1
; 71             JNO      OF=0
; 72             JC       CF=1
; 73             JNC      CF=0
; 74             JZ       ZF=1
; 75             JNZ      ZF=0
; 76             JBE      CF=1 || ZF=1
; 77             JA       CF=0 && ZF=0
; 78             JS       SF=1
; 79             JNS      SF=0
; 7A             JP       PF=1
; 7B             JNP      PF=0
; 7C             JL       SF!=OF
; 7D             JNL      SF=OF
; 7E             JLE      ZF=1 || SF!=OF
; 7F             JNLE     ZF=0 && SF=OF
; E3             JCXZ     CX=0
;
%macro testJcc 1
    mov     ah, PS_CF
    sahf                                ; don't use the stack (pushf/popf)
    jnc     %%err1                      ; 73 /        JNC  CF=0
    jc      %%jcok                      ; 72 /        JC   CF=1
    hlt
%%jz:
    mov     ah, PS_ZF
    sahf
    jnz     %%err1                      ; 75 /        JNZ  ZF=0
    jz      %%jzok                      ; 74 /        JZ   ZF=1
    hlt
%%jp:
    mov     ah, PS_PF
    sahf
    jnp     %%err1                      ; 7B /        JNP  PF=0
    jp      %%jpok                      ; 7A /        JP   PF=1
    hlt
%%js:
    mov     ah, PS_SF
    sahf
    jns     %%err1                      ; 79 /        JNS  SF=0
    js      %%jsok                      ; 78 /        JS   SF=1
    hlt
%%jna:
    mov     ah, PS_ZF|PS_CF
    sahf
    ja      %%err1                      ; 77 /        JA   CF=0 && ZF=0
    jna     %%jnaok                     ; 76 /        JBE  CF=1 || ZF=1
%%next1:
    jmp     %%jnc


%%err1:
    hlt

; test negative offsets
%%jcok:   jc   %%jz
%%jzok:   jz   %%jp
%%jpok:   jp   %%js
%%jsok:   js   %%jna
%%jnaok:  jna  %%next1


%%jnc:
    mov     ah, PS_SF|PS_ZF|PS_AF|PS_PF
    sahf
    mov     ax, 0
    sahf
    jnc     %%jncok                     ; 73 /        JNC  CF=0
    hlt
%%jnz:
    mov     ah, PS_SF|PS_AF|PS_PF|PS_CF
    sahf
    jnz     %%jnzok                     ; 75 /        JNZ  ZF=0
    hlt
%%jnp:
    mov     ah, PS_SF|PS_ZF|PS_AF|PS_CF
    sahf
    jnp     %%jnpok                     ; 7B /        JNP  PF=0
    hlt
%%jns:
    mov     ah, PS_ZF|PS_AF|PS_PF|PS_CF
    sahf
    jns     %%jnsok                     ; 79 /        JNS  SF=0
    hlt
%%ja:
    mov     ah, PS_SF|PS_AF|PS_PF
    sahf
    ja      %%jaok                      ; 77 /        JA   CF=0 && ZF=0
    hlt
%%next2:
    jmp     %%jo


; test negative offsets
%%jncok:  jnc  %%jnz
%%jnzok:  jnz  %%jnp
%%jnpok:  jnp  %%jns
%%jnsok:  jns  %%ja
%%jaok:   ja   %%next2

%%jo:
    mov     ah, 0
    sahf
    mov     al, 1000000b
    shl     al, 1                       ; OF = high-order bit of AL <> (CF), ZF=0,SF=1,OF=1
    jno     %%err2
    jo      %%jook
    hlt
%%jnl:
    jl      %%err2                      ; 7C /        JL   SF!=OF
    jnl     %%jnlok                     ; 7D /        JNL  SF=OF
    hlt
%%jnle:
    jle     %%err2                      ; 7E /        JLE  ZF=1 || SF!=OF
    jnle    %%jnleok                    ; 7F /        JNLE ZF=0 && SF=OF
    hlt
%%jl:
    mov     ah, PS_ZF
    sahf                                ; ZF=1,SF=0,OF=1
    jl      %%jlok                      ; 7C /        JL   SF!=OF
    hlt
%%jle:
    jle     %%jleok                     ; 7E /        JLE  ZF=1 || SF!=OF
    hlt
%%jcxz:
%if %1==8
    mov     cx, 1
    jcxz    %%err2                      ; E3   JCXZ  CX=0
    mov     cx, 0
    jcxz    %%jcxzok
%%jcxze:
%endif
    jmp     %%exit


%%err2:
    hlt

; test negative offsets
%%jook:   jo   %%jnl
%%jnlok:  jnl  %%jnle
%%jnleok: jnle %%jl
%%jlok:   jl   %%jle
%%jleok:  jle  %%jcxz
%if %1==8
%%jcxzok:  jcxz  %%jcxze
%endif

%%exit:
%endmacro

