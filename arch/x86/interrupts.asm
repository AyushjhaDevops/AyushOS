BITS 32

section .text

extern exception_handler

global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31

; ------------------------------------------------
; Exceptions WITHOUT CPU error code
; ------------------------------------------------

isr0:
    push dword 0
    push dword 0
    jmp isr_common

isr1:
    push dword 0
    push dword 1
    jmp isr_common

isr2:
    push dword 0
    push dword 2
    jmp isr_common

isr3:
    push dword 0
    push dword 3
    jmp isr_common

isr4:
    push dword 0
    push dword 4
    jmp isr_common

isr5:
    push dword 0
    push dword 5
    jmp isr_common

isr6:
    push dword 0
    push dword 6
    jmp isr_common

isr7:
    push dword 0
    push dword 7
    jmp isr_common

; ------------------------------------------------
; Exceptions WITH CPU error code
; ------------------------------------------------

isr8:
    push dword 8
    jmp isr_common

; ISR 9 is historically unusual/reserved on modern CPUs,
; but we provide a normalized stub for completeness.
isr9:
    push dword 0
    push dword 9
    jmp isr_common

isr10:
    push dword 10
    jmp isr_common

isr11:
    push dword 11
    jmp isr_common

isr12:
    push dword 12
    jmp isr_common

isr13:
    push dword 13
    jmp isr_common

isr14:
    push dword 14
    jmp isr_common

; ------------------------------------------------
; Exceptions WITHOUT CPU error code
; ------------------------------------------------

isr15:
    push dword 0
    push dword 15
    jmp isr_common

isr16:
    push dword 0
    push dword 16
    jmp isr_common

isr17:
    push dword 17
    jmp isr_common

isr18:
    push dword 0
    push dword 18
    jmp isr_common

isr19:
    push dword 0
    push dword 19
    jmp isr_common

isr20:
    push dword 0
    push dword 20
    jmp isr_common

isr21:
    push dword 0
    push dword 21
    jmp isr_common

isr22:
    push dword 0
    push dword 22
    jmp isr_common

isr23:
    push dword 0
    push dword 23
    jmp isr_common

isr24:
    push dword 0
    push dword 24
    jmp isr_common

isr25:
    push dword 0
    push dword 25
    jmp isr_common

isr26:
    push dword 0
    push dword 26
    jmp isr_common

isr27:
    push dword 0
    push dword 27
    jmp isr_common

isr28:
    push dword 0
    push dword 28
    jmp isr_common

isr29:
    push dword 29
    jmp isr_common

isr30:
    push dword 30
    jmp isr_common

isr31:
    push dword 0
    push dword 31
    jmp isr_common


; ------------------------------------------------
; Common ISR entry
; ------------------------------------------------

isr_common:

    ; Save general purpose registers.
    pusha

    ; Save current data segment.
    mov ax, ds
    push eax

    ; Load kernel data segment.
    mov ax, 0x10

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; ESP points to interrupt_frame_t.
    push esp

    call exception_handler

    add esp, 4

    ; Restore original data segment.
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Restore registers.
    popa

    ; Remove:
    ;
    ; int_no
    ; err_code
    ;
    add esp, 8

    ; Return from interrupt.
    iretd
