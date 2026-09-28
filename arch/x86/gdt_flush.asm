BITS 32

section .text

global gdt_flush

gdt_flush:
    ; C passes the address of the GDT pointer
    ; as the first argument on the stack.
    ;
    ; [esp + 4] = gdt_ptr address

    mov eax, [esp + 4]

    ; Load GDTR
    lgdt [eax]

    ; Kernel data segment selector = 0x10
    mov ax, 0x10

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Reload CS with kernel code selector = 0x08
    jmp 0x08:.flush

.flush:
    ret
