; SPDX-License-Identifier: GPL-3.0-or-later

TYPE_ARITH    equ  0 ; multiple values for AX and DX
TYPE_ARITH1   equ  1 ; multiple values for AX, 1 value for DX
TYPE_ARITH1D  equ  2 ; 1 value for AX, multiple values for DX
TYPE_LOGIC    equ  3 ; multiple values for AX and DX
TYPE_LOGIC1   equ  4 ; multiple values for AX, 1 value for DX
TYPE_LOGIC1D  equ  5 ; 1 value for AX, multiple values for DX
TYPE_MULTIPLY equ  6
TYPE_DIVIDE   equ  7
TYPE_SHIFTS_1 equ  8
TYPE_SHIFTS_R equ  9

SIZE_BYTE     equ  0
SIZE_SHORT    equ  1

tableOps:
    defOp0   "98 CBW",cbw,b,TYPE_ARITH1
    defOp0   "99 CWD",cwd,w,TYPE_ARITH1
    defOp    "00 ADD",add,al,dl,none,TYPE_ARITH
    defOp    "01 ADD",add,ax,dx,none,TYPE_ARITH
    defOp    "04 ADD",add,al,0xFF,none,TYPE_ARITH1
    defOp    "05 ADD",add,ax,0x8002,none,TYPE_ARITH1
    defOp    "83 ADD",add,ax,byte -1,none,TYPE_ARITH1
    defOp    "80 ADD",add,dl,0xFF,none,TYPE_ARITH1D
    defOp    "81 ADD",add,dx,0x8002,none,TYPE_ARITH1D
    defOp    "02 ADD",add,al,mem,none,TYPE_ARITH
    defOp    "03 ADD",add,ax,mem,none,TYPE_ARITH
    defOp    "08 OR",or,al,dl,none,TYPE_LOGIC
    defOp    "09 OR",or,ax,dx,none,TYPE_LOGIC
    defOp    "0C OR",or,al,0xAA,none,TYPE_LOGIC1
    defOp    "0D OR",or,ax,0xAAAA,none,TYPE_LOGIC1
    defOp    "83 OR",or,ax,byte -86,none,TYPE_LOGIC1
    defOp    "80 OR",or,dl,0xAA,none,TYPE_LOGIC1D
    defOp    "81 OR",or,dx,0xAAAA,none,TYPE_LOGIC1D
    defOp    "0A OR",or,al,mem,none,TYPE_LOGIC
    defOp    "0B OR",or,ax,mem,none,TYPE_LOGIC
    defOp    "10 ADC",adc,al,dl,none,TYPE_ARITH
    defOp    "11 ADC",adc,ax,dx,none,TYPE_ARITH
    defOp    "14 ADC",adc,al,0xFF,none,TYPE_ARITH1
    defOp    "15 ADC",adc,ax,0x8002,none,TYPE_ARITH1
    defOp    "83 ADC",adc,ax,byte -1,none,TYPE_ARITH1
    defOp    "80 ADC",adc,dl,0xFF,none,TYPE_ARITH1D
    defOp    "81 ADC",adc,dx,0x8002,none,TYPE_ARITH1D
    defOp    "12 ADC",adc,al,mem,none,TYPE_ARITH
    defOp    "13 ADC",adc,ax,mem,none,TYPE_ARITH
    defOp    "18 SBB",sbb,al,dl,none,TYPE_ARITH
    defOp    "19 SBB",sbb,ax,dx,none,TYPE_ARITH
    defOp    "1C SBB",sbb,al,0xFF,none,TYPE_ARITH1
    defOp    "1D SBB",sbb,ax,0x8000,none,TYPE_ARITH1
    defOp    "83 SBB",sbb,ax,byte -1,none,TYPE_ARITH1
    defOp    "80 SBB",sbb,dl,0xFF,none,TYPE_ARITH1D
    defOp    "81 SBB",sbb,dx,0x8000,none,TYPE_ARITH1D
    defOp    "1A SBB",sbb,al,mem,none,TYPE_ARITH
    defOp    "1B SBB",sbb,ax,mem,none,TYPE_ARITH
    defOp    "20 AND",and,al,dl,none,TYPE_LOGIC
    defOp    "21 AND",and,ax,dx,none,TYPE_LOGIC
    defOp    "24 AND",and,al,0xAA,none,TYPE_LOGIC1
    defOp    "25 AND",and,ax,0xAAAA,none,TYPE_LOGIC1
    defOp    "83 AND",and,ax,byte -86,none,TYPE_LOGIC1
    defOp    "80 AND",and,dl,0xAA,none,TYPE_LOGIC1D
    defOp    "81 AND",and,dx,0xAAAA,none,TYPE_LOGIC1D
    defOp    "22 AND",and,al,mem,none,TYPE_LOGIC
    defOp    "23 AND",and,ax,mem,none,TYPE_LOGIC
    defOp    "28 SUB",sub,al,dl,none,TYPE_ARITH
    defOp    "29 SUB",sub,ax,dx,none,TYPE_ARITH
    defOp    "2C SUB",sub,al,0xFF,none,TYPE_ARITH1
    defOp    "2D SUB",sub,ax,0x8000,none,TYPE_ARITH1
    defOp    "83 SUB",sub,ax,byte -1,none,TYPE_ARITH1
    defOp    "80 SUB",sub,dl,0xFF,none,TYPE_ARITH1D
    defOp    "81 SUB",sub,dx,0x8000,none,TYPE_ARITH1D
    defOp    "2A SUB",sub,al,mem,none,TYPE_ARITH
    defOp    "2B SUB",sub,ax,mem,none,TYPE_ARITH
    defOp    "30 XOR",xor,al,dl,none,TYPE_LOGIC
    defOp    "31 XOR",xor,ax,dx,none,TYPE_LOGIC
    defOp    "34 XOR",xor,al,0xAA,none,TYPE_LOGIC1
    defOp    "35 XOR",xor,ax,0xAAAA,none,TYPE_LOGIC1
    defOp    "83 XOR",xor,ax,byte -86,none,TYPE_LOGIC1
    defOp    "80 XOR",xor,dl,0xAA,none,TYPE_LOGIC1D
    defOp    "81 XOR",xor,dx,0xAAAA,none,TYPE_LOGIC1D
    defOp    "32 XOR",xor,al,mem,none,TYPE_LOGIC
    defOp    "33 XOR",xor,ax,mem,none,TYPE_LOGIC
    defOp    "38 CMP",cmp,al,dl,none,TYPE_LOGIC
    defOp    "39 CMP",cmp,ax,dx,none,TYPE_LOGIC
    defOp    "3C CMP",cmp,al,0xAA,none,TYPE_LOGIC1
    defOp    "3D CMP",cmp,ax,0xAAAA,none,TYPE_LOGIC1
    defOp    "83 CMP",cmp,ax,byte -86,none,TYPE_LOGIC1
    defOp    "80 CMP",cmp,dl,0xAA,none,TYPE_LOGIC1D
    defOp    "81 CMP",cmp,dx,0xAAAA,none,TYPE_LOGIC1D
    defOp    "3A CMP",cmp,al,mem,none,TYPE_LOGIC
    defOp    "3B CMP",cmp,ax,mem,none,TYPE_LOGIC
    defOp    "84 TEST",test,al,dl,none,TYPE_LOGIC
    defOp    "85 TEST",test,ax,dx,none,TYPE_LOGIC
    defOp    "A8 TEST",test,al,0xAA,none,TYPE_LOGIC1
    defOp    "A9 TEST",test,ax,0xAAAA,none,TYPE_LOGIC1
    defOp    "F6 TEST",test,dl,0xAA,none,TYPE_LOGIC1D
    defOp    "F7 TEST",test,dx,0xAAAA,none,TYPE_LOGIC1D
    defOpInc "40 INC",inc,ax,word
    defOpInc "41 INC",inc,cx,word
    defOpInc "42 INC",inc,dx,word
    defOpInc "43 INC",inc,bx,word
    defOpInc "44 INC",inc,sp,word
    defOpInc "45 INC",inc,bp,word
    defOpInc "46 INC",inc,si,word
    defOpInc "47 INC",inc,di,word
    defOpInc "FE INC",inc,mem,byte
    defOpInc "FF INC",inc,mem,word
    defOpInc "48 DEC",dec,ax,word
    defOpInc "49 DEC",dec,cx,word
    defOpInc "4A DEC",dec,dx,word
    defOpInc "4B DEC",dec,bx,word
    defOpInc "4C DEC",dec,sp,word
    defOpInc "4D DEC",dec,bp,word
    defOpInc "4E DEC",dec,si,word
    defOpInc "4F DEC",dec,di,word
    defOpInc "FE DEC",dec,mem,byte
    defOpInc "FF DEC",dec,mem,word
    defOp    "F6 NEG",neg,al,none,none,TYPE_ARITH1
    defOp    "F7 NEG",neg,ax,none,none,TYPE_ARITH1
    defOp    "F6 NOT",not,al,none,none,TYPE_LOGIC1
    defOp    "F7 NOT",not,ax,none,none,TYPE_LOGIC1
    defOp    "F6 MUL",mul,dl,none,none,TYPE_MULTIPLY
    defOp    "F7 MUL",mul,dx,none,none,TYPE_MULTIPLY
    defOp    "F6 IMUL",imul,dl,none,none,TYPE_MULTIPLY
    defOp    "F7 IMUL",imul,dx,none,none,TYPE_MULTIPLY
    defOp    "6B IMUL",imul,ax,dx,0x77,TYPE_MULTIPLY
    defOp    "6B IMUL",imul,ax,dx,-0x77,TYPE_MULTIPLY
    defOp    "69 IMUL",imul,ax,0x777,none,TYPE_MULTIPLY
    defOp    "F6 DIV",div,dl,none,none,TYPE_DIVIDE
    defOp    "F7 DIV",div,dx,none,none,TYPE_DIVIDE
    defOp    "F6 DIV",div,al,none,none,TYPE_DIVIDE
    defOp    "F7 DIV",div,ax,none,none,TYPE_DIVIDE
    defOp    "F6 IDIV",idiv,dl,none,none,TYPE_DIVIDE
    defOp    "F7 IDIV",idiv,dx,none,none,TYPE_DIVIDE
    defOp    "F6 IDIV",idiv,al,none,none,TYPE_DIVIDE
    defOp    "F7 IDIV",idiv,ax,none,none,TYPE_DIVIDE
    defOpSh  "D0 SAL",sal,al,1,TYPE_SHIFTS_1
    defOpSh  "D1 SAL",sal,ax,1,TYPE_SHIFTS_1
    defOpSh  "C0 SAL",sal,al,7,TYPE_SHIFTS_R
    defOpSh  "C1 SAL",sal,ax,7,TYPE_SHIFTS_R
    defOpSh  "D2 SAL",sal,al,cl,TYPE_SHIFTS_R
    defOpSh  "D3 SAL",sal,ax,cl,TYPE_SHIFTS_R
    defOpSh  "D0 SAR",sar,al,1,TYPE_SHIFTS_1
    defOpSh  "D1 SAR",sar,ax,1,TYPE_SHIFTS_1
    defOpSh  "C0 SAR",sar,al,7,TYPE_SHIFTS_R
    defOpSh  "C1 SAR",sar,ax,7,TYPE_SHIFTS_R
    defOpSh  "D2 SAR",sar,al,cl,TYPE_SHIFTS_R
    defOpSh  "D3 SAR",sar,ax,cl,TYPE_SHIFTS_R
    defOpSh  "D0 SHR",shr,al,1,TYPE_SHIFTS_1
    defOpSh  "D1 SHR",shr,ax,1,TYPE_SHIFTS_1
    defOpSh  "C0 SHR",shr,al,7,TYPE_SHIFTS_R
    defOpSh  "C1 SHR",shr,ax,7,TYPE_SHIFTS_R
    defOpSh  "D2 SHR",shr,al,cl,TYPE_SHIFTS_R
    defOpSh  "D3 SHR",shr,ax,cl,TYPE_SHIFTS_R
    defOpSh  "D0 ROL",rol,al,1,TYPE_SHIFTS_1
    defOpSh  "D1 ROL",rol,ax,1,TYPE_SHIFTS_1
    defOpSh  "C0 ROL",rol,al,7,TYPE_SHIFTS_1
    defOpSh  "C1 ROL",rol,ax,7,TYPE_SHIFTS_1
    defOpSh  "D2 ROL",rol,al,cl,TYPE_SHIFTS_R
    defOpSh  "D3 ROL",rol,ax,cl,TYPE_SHIFTS_R
    defOpSh  "D0 ROR",ror,al,1,TYPE_SHIFTS_1
    defOpSh  "D1 ROR",ror,ax,1,TYPE_SHIFTS_1
    defOpSh  "C0 ROR",ror,al,7,TYPE_SHIFTS_1
    defOpSh  "C1 ROR",ror,ax,7,TYPE_SHIFTS_1
    defOpSh  "D2 ROR",ror,al,cl,TYPE_SHIFTS_R
    defOpSh  "D3 ROR",ror,ax,cl,TYPE_SHIFTS_R
    defOpSh  "D0 RCL",rcl,al,1,TYPE_SHIFTS_1
    defOpSh  "D1 RCL",rcl,ax,1,TYPE_SHIFTS_1
    defOpSh  "C0 RCL",rcl,al,7,TYPE_SHIFTS_1
    defOpSh  "C1 RCL",rcl,ax,7,TYPE_SHIFTS_1
    defOpSh  "D2 RCL",rcl,al,cl,TYPE_SHIFTS_R
    defOpSh  "D3 RCL",rcl,ax,cl,TYPE_SHIFTS_R
    defOpSh  "D0 RCR",rcr,al,1,TYPE_SHIFTS_1
    defOpSh  "D1 RCR",rcr,ax,1,TYPE_SHIFTS_1
    defOpSh  "C0 RCR",rcr,al,7,TYPE_SHIFTS_1
    defOpSh  "C1 RCR",rcr,ax,7,TYPE_SHIFTS_1
    defOpSh  "D2 RCR",rcr,al,cl,TYPE_SHIFTS_R
    defOpSh  "D3 RCR",rcr,ax,cl,TYPE_SHIFTS_R
    db      0

typeMasks:
    dw      PS_ARITH
    dw      PS_ARITH
    dw      PS_ARITH
    dw      PS_LOGIC
    dw      PS_LOGIC
    dw      PS_LOGIC
    dw      PS_MULTIPLY
    dw      PS_DIVIDE
    dw      PS_SHIFTS_1
    dw      PS_SHIFTS_R

arithValues:
.bvals: dw  0x00,0x01,0x02,0x7E,0x7F,0x80,0x81,0xFE,0xFF
    ARITH_BYTES equ ($-.bvals)/2

.wvals: dw  0x0000,0x0001,0x0002,0x7FFE,0x7FFF,0x8000,0x8001,0xFFFE,0xFFFF
    ARITH_WORDS equ ($-.wvals)/2


logicValues:
.bvals: dw  0x00,0x01,0x55,0xAA,0x5A,0xA5,0xFF
    LOGIC_BYTES equ ($-.bvals)/2

.wvals: dw  0x0000,0x0001,0x5555,0xAAAA,0x5A5A,0xA5A5,0xFFFF
    LOGIC_WORDS equ ($-.wvals)/2


muldivValues:
.bvals: dw  0x00,0x01,0x02,0x3F,0x40,0x41,0x7E,0x7F,0x80,0x81,0xFE,0xFF
    MULDIV_BYTES equ ($-.bvals)/2

.wvals: dw  0x0000,0x0001,0x0002,0x3FFF,0x4000,0x4001,0x7FFE,0x7FFF,0x8000,0x8001,0xFFFE,0xFFFF
    MULDIV_WORDS equ ($-.wvals)/2


shiftsValues:
.bvals: dw  0x00,0x01,0x02,0x7E,0x7F,0x80,0x81,0xFE,0xFF
    SHIFTS_BYTES equ ($-.bvals)/2

.wvals: dw  0x0000,0x0001,0x0181,0x7FFE,0x7FFF,0x8000,0x8001,0xFFFE,0xFFFF
    SHIFTS_WORDS equ ($-.wvals)/2


shiftsValuesR:
.bvals: dw  0x00,0x01,0x02,0x08
    SHIFTS_BYTES_R equ ($-.bvals)/2

.wvals: dw  0x8000,0x8001,0x8002,0x8010
    SHIFTS_WORDS_R equ ($-.wvals)/2



typeValues:
    ;
    ; Values for TYPE_ARITH
    ;
    dw      ARITH_BYTES,arithValues,ARITH_BYTES,arithValues
    dw      ARITH_BYTES+ARITH_WORDS,arithValues,ARITH_BYTES+ARITH_WORDS,arithValues
    ;
    ; Values for TYPE_ARITH1
    ;
    dw      ARITH_BYTES,arithValues,1,arithValues
    dw      ARITH_BYTES+ARITH_WORDS,arithValues,1,arithValues
    ;
    ; Values for TYPE_ARITH1D
    ;
    dw      1,arithValues,ARITH_BYTES,arithValues
    dw      1,arithValues,ARITH_BYTES+ARITH_WORDS,arithValues
    ;
    ; Values for TYPE_LOGIC
    ;
    dw      LOGIC_BYTES,logicValues,LOGIC_BYTES,logicValues
    dw      LOGIC_BYTES+LOGIC_WORDS,logicValues,LOGIC_BYTES+LOGIC_WORDS,logicValues
    ;
    ; Values for TYPE_LOGIC1
    ;
    dw      LOGIC_BYTES,logicValues,1,logicValues
    dw      LOGIC_BYTES+LOGIC_WORDS,logicValues,1,logicValues
    ;
    ; Values for TYPE_LOGIC1D
    ;
    dw      1,logicValues,LOGIC_BYTES,logicValues
    dw      1,logicValues,LOGIC_BYTES+LOGIC_WORDS,logicValues
    ;
    ; Values for TYPE_MULTIPLY (a superset of ARITH values)
    ;
    dw      MULDIV_BYTES,muldivValues,MULDIV_BYTES,muldivValues
    dw      MULDIV_BYTES+MULDIV_WORDS,muldivValues,MULDIV_BYTES+MULDIV_WORDS,muldivValues
    ;
    ; Values for TYPE_DIVIDE
    ;
    dw      MULDIV_BYTES,muldivValues,MULDIV_BYTES,muldivValues
    dw      MULDIV_BYTES+MULDIV_WORDS,muldivValues,MULDIV_BYTES+MULDIV_WORDS,muldivValues
    ;
    ; Values for TYPE_SHIFTS_1
    ;
    dw      SHIFTS_BYTES,shiftsValues,1,shiftsValues
    dw      SHIFTS_BYTES+SHIFTS_WORDS,shiftsValues,1,shiftsValues
    ;
    ; Values for TYPE_SHIFTS_R
    ;
    dw      SHIFTS_BYTES,shiftsValues,SHIFTS_BYTES_R,shiftsValuesR
    dw      SHIFTS_BYTES+SHIFTS_WORDS,shiftsValues,SHIFTS_BYTES_R+SHIFTS_WORDS_R,shiftsValuesR
