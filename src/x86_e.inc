; SPDX-License-Identifier: GPL-3.0-or-later

PS_CF       equ 0x0001
PS_PF       equ 0x0004
PS_AF       equ 0x0010
PS_ZF       equ 0x0040
PS_SF       equ 0x0080
PS_TF       equ 0x0100
PS_IF       equ 0x0200
PS_DF       equ 0x0400
PS_OF       equ 0x0800
PS_NT       equ 0x4000
PS_ARITH    equ (PS_CF | PS_PF | PS_AF | PS_ZF | PS_SF | PS_OF)
PS_LOGIC    equ (PS_CF | PS_PF | PS_ZF | PS_SF | PS_OF)
PS_MULTIPLY equ (PS_CF | PS_OF) ; only CF and OF are "defined" following MUL or IMUL
PS_DIVIDE   equ 0 ; none of the Processor Status flags are "defined" following DIV or IDIV
PS_SHIFTS_1 equ (PS_CF | PS_SF | PS_ZF | PS_PF | PS_OF)
PS_SHIFTS_R equ (PS_CF | PS_SF | PS_ZF | PS_PF)

CR0_MSW_PE  equ 0x0001

;Flags available for load/store tests in user mode
FLAGS_AVL equ (PS_CF|PS_PF|PS_AF|PS_ZF|PS_SF|PS_DF|PS_PF)
;Flags that are always set.
FLAGS_FORCEDSET equ 2
;Flags that are all set
FLAGS_SET equ (FLAGS_AVL|FLAGS_FORCEDSET)
;Flags that are all cleared
FLAGS_CLEARED equ FLAGS_FORCEDSET

ACC_TYPE_GATE286_INT  equ 0x0600
ACC_TYPE_GATE286_CALL equ 0x0400
ACC_TYPE_GATE_TSS    equ 0x0500
ACC_TYPE_SEG         equ 0x1000
ACC_PRESENT          equ 0x8000
ACC_TYPE_CODE_R      equ 0x1a00
ACC_TYPE_CONFORMING  equ 0x0400
ACC_TYPE_DATA_R      equ 0x1000
ACC_TYPE_DATA_W      equ 0x1200
ACC_TYPE_LDT         equ 0x0200
ACC_TYPE_TSS16       equ 0x0100

ACC_DPL_0 equ 0x0000
ACC_DPL_1 equ 0x2000
ACC_DPL_2 equ 0x4000
ACC_DPL_3 equ 0x6000

EX_DE equ 0
EX_DB equ 1
EX_BP equ 3
EX_OF equ 4
EX_BR equ 5
EX_UD equ 6
EX_NM equ 7
EX_DF equ 8
EX_MP equ 9
EX_TS equ 10
EX_NP equ 11
EX_SS equ 12
EX_GP equ 13
EX_MF equ 15

