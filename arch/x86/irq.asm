BITS 32

section .text

extern irq_handler

global irq0
global irq1
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15

; ------------------------------------------------------------
; IRQ 0 - PIT timer
; ------------------------------------------------------------

irq0:
    push dword 0
    push dword 32
    jmp irq_common

; ------------------------------------------------------------
; IRQ 1 - Keyboard
; ------------------------------------------------------------

irq1:
    push dword 0
    push dword 33
    jmp irq_common

; ------------------------------------------------------------
; IRQ 2
; ------------------------------------------------------------

irq2:
    push dword 0
    push dword 34
    jmp irq_common

; ------------------------------------------------------------
; IRQ 3
; ------------------------------------------------------------

irq3:
    push dword 0
    push dword 35
    jmp irq_common

; ------------------------------------------------------------
; IRQ 4
; ------------------------------------------------------------

irq4:
    push dword 0
    push dword 36
    jmp irq_common

; ------------------------------------------------------------
; IRQ 5
; ------------------------------------------------------------

irq5:
    push dword 0
    push dword 37
    jmp irq_common

; ------------------------------------------------------------
; IRQ 6
; ------------------------------------------------------------

irq6:
    push dword 0
    push dword 38
    jmp irq_common

; ------------------------------------------------------------
; IRQ 7
; ------------------------------------------------------------

irq7:
    push dword 0
    push dword 39
    jmp irq_common

; ------------------------------------------------------------
; IRQ 8
; ------------------------------------------------------------

irq8:
    push dword 0
    push dword 40
    jmp irq_common

; ------------------------------------------------------------
; IRQ 9
; ------------------------------------------------------------

irq9:
    push dword 0
    push dword 41
    jmp irq_common

; ------------------------------------------------------------
; IRQ 10
; ------------------------------------------------------------

irq10:
    push dword 0
    push dword 42
    jmp irq_common

; ------------------------------------------------------------
; IRQ 11
; ------------------------------------------------------------

irq11:
    push dword 0
    push dword 43
    jmp irq_common

; ------------------------------------------------------------
; IRQ 12
; ------------------------------------------------------------

irq12:
    push dword 0
    push dword 44
    jmp irq_common

; ------------------------------------------------------------
; IRQ 13
; ------------------------------------------------------------

irq13:
    push dword 0
    push dword 45
    jmp irq_common

; ------------------------------------------------------------
; IRQ 14
; ------------------------------------------------------------

irq14:
    push dword 0
    push dword 46
    jmp irq_common

; ------------------------------------------------------------
; IRQ 15
; ------------------------------------------------------------

irq15:
    push dword 0
    push dword 47
    jmp irq_common


; ============================================================
; Common IRQ entry
; ============================================================

irq_common:

    ;
    ; Save all general-purpose registers.
    ;
    pusha

    ;
    ; Save current DS.
    ;
    xor eax, eax
    mov ax, ds
    push eax

    ;
    ; Switch kernel data segments.
    ;
    mov ax, 0x10

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ;
    ; ESP now points to irq_frame_t.
    ;
    push esp

    ;
    ; irq_handler(frame)
    ;
    call irq_handler

    ;
    ; Remove function argument.
    ;
    add esp, 4

    ;
    ; IMPORTANT:
    ;
    ; irq_handler returns:
    ;
    ;   same frame = continue current process
    ;
    ; OR
    ;
    ;   different frame = resume another process
    ;
    ; EAX contains the selected frame pointer.
    ;
    mov esp, eax

    ;
    ; Restore DS.
    ;
    pop eax

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ;
    ; Restore general-purpose registers.
    ;
    popa

    ;
    ; Remove:
    ;
    ;   int_no
    ;   err_code
    ;
    add esp, 8

    ;
    ; Restore:
    ;
    ;   EIP
    ;   CS
    ;   EFLAGS
    ;
    iretd
