; SPDX-License-Identifier: GPL-3.0-or-later

; 286 forms of test386's task-validation macros.

; Validate the busy bit of a TSS.
;
; Parameters:
; %1: GDT TSS descriptor selector to validate.
; %2: expected B bit (0 or 1).
;
; The descriptor's access byte is read through the GDT data alias.
; General registers and DS are preserved; Flags are changed.
; The stack must be initialized.
;
%macro validateTSSbusy286 2
    pusha
    mov     ax, %1
    mov     dx, %2
    call    validateTSSbusy
    popa
%endmacro

; Validate the backlink of a TSS.
;
; Parameters:
; %1: data-segment selector mapping the TSS image.
; %2: expected backlink selector at offset 0.
;
; Pass the TSS data alias, not the system TSS selector. General registers
; and DS are preserved; Flags are changed. The stack must be initialized.
;
%macro validateTSSbacklink286 2
    pusha
    mov     ax, %1
    mov     dx, %2
    call    validateTSSbacklink
    popa
%endmacro

; Validate the NT bit of the current task.
;
; Parameters retain the test386 interface:
; %1: TSS data descriptor, or 0 for the current task; this port requires 0.
; %2: TSS size selector; this port requires 0 (16-bit TSS).
; %3: expected NT bit (0 or 1).
;
; Only live FLAGS are inspected. Other %1/%2 combinations are rejected at
; assembly time. AX is preserved; Flags are changed by the check.
; The stack must be initialized.
;
%macro validateTSSNT286 3
    ; Retain test386's signature; this port currently checks live FLAGS.
    ; Saved-image and 32-bit-TSS variants must not silently check this task.
    %if %1 != 0 || %2 != 0
        %error validateTSSNT286 currently requires current-task, 16-bit arguments (0,0)
    %endif
    push    ax
    pushf
    pop     ax
    and     ax, PS_NT
    cmp     ax, (%3)*PS_NT
    jne     error
    pop     ax
%endmacro

; Validate the Task Switched bit, then clear it with CLTS.
;
; %1: expected TS bit in the machine status word (0 or 1).
;
; Run at CPL 0 with an initialized stack. AX is preserved; Flags are changed.
; Checks TS before CLTS, then reads MSW again to verify that TS was cleared.
;
%macro validateTSandClear 1
    push    ax
    smsw    ax
    and     ax, 8
    cmp     ax, (%1)*8
    jne     error
    clts
    smsw    ax
    test    ax, 8
    jnz     error
    pop     ax
%endmacro
