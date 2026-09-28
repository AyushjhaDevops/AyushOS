; ============================================================
; AyushOS - x86 Boot Entry
; Multiboot-compliant entry point
; ============================================================

BITS 32

section .multiboot
align 4

MULTIBOOT_MAGIC    equ 0x1BADB002
MULTIBOOT_FLAGS    equ 0x00000003
MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

dd MULTIBOOT_MAGIC
dd MULTIBOOT_FLAGS
dd MULTIBOOT_CHECKSUM


section .text

global _start
extern kernel_main

_start:

    ; Disable interrupts during early kernel initialization
    cli

    ; Set up our own stack
    mov esp, stack_top

    ; Clear direction flag
    cld

    ; Call the C kernel
    call kernel_main


.hang:

    ; Kernel should never return
    cli
    hlt
    jmp .hang


section .bss

align 16

stack_bottom:
    resb 16384

stack_top:
