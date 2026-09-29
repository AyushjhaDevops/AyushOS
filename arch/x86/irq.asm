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


; =========================================================
; IRQ 0 - Timer - IDT vector 32
; =========================================================

irq0:
    push dword 0
    push dword 32
    jmp irq_common


; =========================================================
; IRQ 1 - Keyboard - IDT vector 33
; =========================================================

irq1:
    push dword 0
    push dword 33
    jmp irq_common


; =========================================================
; IRQ 2 - Cascade - IDT vector 34
; =========================================================

irq2:
    push dword 0
    push dword 34
    jmp irq_common


; =========================================================
; IRQ 3 - COM2 - IDT vector 35
; =========================================================

irq3:
    push dword 0
    push dword 35
    jmp irq_common


; =========================================================
; IRQ 4 - COM1 - IDT vector 36
; =========================================================

irq4:
    push dword 0
    push dword 36
    jmp irq_common


; =========================================================
; IRQ 5 - LPT2 - IDT vector 37
; =========================================================

irq5:
    push dword 0
    push dword 37
    jmp irq_common


; =========================================================
; IRQ 6 - Floppy - IDT vector 38
; =========================================================

irq6:
    push dword 0
    push dword 38
    jmp irq_common


; =========================================================
; IRQ 7 - LPT1 / Spurious - IDT vector 39
; =========================================================

irq7:
    push dword 0
    push dword 39
    jmp irq_common


; =========================================================
; IRQ 8 - RTC - IDT vector 40
; =========================================================

irq8:
    push dword 0
    push dword 40
    jmp irq_common


; =========================================================
; IRQ 9 - Redirected IRQ2 - IDT vector 41
; =========================================================

irq9:
    push dword 0
    push dword 41
    jmp irq_common


; =========================================================
; IRQ 10 - IDT vector 42
; =========================================================

irq10:
    push dword 0
    push dword 42
    jmp irq_common


; =========================================================
; IRQ 11 - IDT vector 43
; =========================================================

irq11:
    push dword 0
    push dword 43
    jmp irq_common


; =========================================================
; IRQ 12 - PS/2 Mouse - IDT vector 44
; =========================================================

irq12:
    push dword 0
    push dword 44
    jmp irq_common


; =========================================================
; IRQ 13 - FPU - IDT vector 45
; =========================================================

irq13:
    push dword 0
    push dword 45
    jmp irq_common


; =========================================================
; IRQ 14 - Primary ATA - IDT vector 46
; =========================================================

irq14:
    push dword 0
    push dword 46
    jmp irq_common


; =========================================================
; IRQ 15 - Secondary ATA - IDT vector 47
; =========================================================

irq15:
    push dword 0
    push dword 47
    jmp irq_common


; =========================================================
; Common IRQ handler
; =========================================================

irq_common:

    ; Save general purpose registers.
    pusha

    ; Save current data segment.
    ; Clear upper 16 bits first.
    xor eax, eax
    mov ax, ds
    push eax

    ; Switch to kernel data segment.
    mov ax, 0x10

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Pass pointer to irq_frame_t.
    ;
    ; Stack layout:
    ;
    ; DS
    ; EDI
    ; ESI
    ; EBP
    ; saved ESP
    ; EBX
    ; EDX
    ; ECX
    ; EAX
    ; INT_NO
    ; ERROR_CODE
    ; EIP
    ; CS
    ; EFLAGS
    ; USERESP
    ; SS

    push esp

    call irq_handler

    ; Remove irq_frame_t pointer.
    add esp, 4

    ; Restore original data segment.
    pop eax

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Restore general purpose registers.
    popa

    ; Remove INT_NO and ERROR_CODE.
    add esp, 8

    ; Return from interrupt.
    iretd
