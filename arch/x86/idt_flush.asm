BITS 32

section .text

global idt_flush

idt_flush:
    ; [esp + 4] = address of idt_ptr

    mov eax, [esp + 4]

    ; Load IDTR
    lidt [eax]

    ret
