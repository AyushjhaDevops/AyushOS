ASM      = nasm
CC       = gcc
LD       = ld

CFLAGS   = -m32 \
           -ffreestanding \
           -fno-pie \
           -fno-stack-protector \
           -nostdlib \
           -nostdinc \
           -g \
           -Wall \
           -Wextra

LDFLAGS  = -m elf_i386 \
           -T scripts/linker.ld

KERNEL   = build/kernel.bin

ASM_SRC  = arch/x86/boot.asm

C_OBJS  = \
          build/kernel.o \
          build/terminal.o \
          build/kprintf.o \
          build/keyboard.o \
          build/gdt.o \
          build/idt.o \
          build/pic.o \
          build/irq.o

ASM_OBJ       = build/boot.o
GDT_ASM_OBJ   = build/gdt_flush.o
IDT_ASM_OBJ   = build/idt_flush.o
ISR_ASM_OBJ   = build/interrupts.o
IRQ_ASM_OBJ   = build/irq_asm.o

OBJS = \
       $(ASM_OBJ) \
       $(GDT_ASM_OBJ) \
       $(IDT_ASM_OBJ) \
       $(ISR_ASM_OBJ) \
       $(IRQ_ASM_OBJ) \
       $(C_OBJS)

ISO_DIR  = iso
ISO_FILE = AyushOS.iso


.PHONY: all clean iso run debug


all: $(KERNEL)


# ---------------------------------------------------------
# Boot assembly
# ---------------------------------------------------------

$(ASM_OBJ): $(ASM_SRC)
	@mkdir -p build
	$(ASM) -f elf32 $< -o $@


# ---------------------------------------------------------
# GDT assembly
# ---------------------------------------------------------

build/gdt_flush.o: arch/x86/gdt_flush.asm
	@mkdir -p build
	$(ASM) -f elf32 $< -o $@


# ---------------------------------------------------------
# IDT assembly
# ---------------------------------------------------------

build/idt_flush.o: arch/x86/idt_flush.asm
	@mkdir -p build
	$(ASM) -f elf32 $< -o $@

build/interrupts.o: arch/x86/interrupts.asm
	@mkdir -p build
	$(ASM) -f elf32 $< -o $@
# ---------------------------------------------------------
# IRQ assembly
# ---------------------------------------------------------

build/irq_asm.o: arch/x86/irq.asm
	@mkdir -p build
	$(ASM) -f elf32 $< -o $@


# ---------------------------------------------------------
# Kernel
# ---------------------------------------------------------

build/kernel.o: kernel/kernel.c
	@mkdir -p build
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@


# ---------------------------------------------------------
# Terminal
# ---------------------------------------------------------

build/terminal.o: kernel/drivers/terminal.c
	@mkdir -p build
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@


# ---------------------------------------------------------
# printf
# ---------------------------------------------------------

build/kprintf.o: kernel/lib/kprintf.c
	@mkdir -p build
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@


# ---------------------------------------------------------
# Keyboard
# ---------------------------------------------------------

build/keyboard.o: kernel/drivers/keyboard.c
	@mkdir -p build
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@


# ---------------------------------------------------------
# GDT
# ---------------------------------------------------------

build/gdt.o: kernel/gdt.c
	@mkdir -p build
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@


# ---------------------------------------------------------
# IDT
# ---------------------------------------------------------

build/idt.o: kernel/idt.c
	@mkdir -p build
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@


# ---------------------------------------------------------
# PIC
# ---------------------------------------------------------

build/pic.o: kernel/pic.c
	@mkdir -p build
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@


# ---------------------------------------------------------
# IRQ
# ---------------------------------------------------------

build/irq.o: kernel/irq.c
	@mkdir -p build
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@


# ---------------------------------------------------------
# Link kernel
# ---------------------------------------------------------

$(KERNEL): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^


# ---------------------------------------------------------
# Create ISO
# ---------------------------------------------------------

iso: $(KERNEL)
	mkdir -p $(ISO_DIR)/boot
	cp $(KERNEL) $(ISO_DIR)/boot/kernel.bin
	grub-mkrescue -o $(ISO_FILE) $(ISO_DIR)


# ---------------------------------------------------------
# Run
# ---------------------------------------------------------

run: iso
	qemu-system-i386 \
		-cdrom $(ISO_FILE)


# ---------------------------------------------------------
# Debug
# ---------------------------------------------------------

debug: iso
	qemu-system-i386 \
		-cdrom $(ISO_FILE) \
		-no-reboot \
		-no-shutdown \
		-s \
		-S


# ---------------------------------------------------------
# Clean
# ---------------------------------------------------------

clean:
	rm -rf build/*
	rm -f $(ISO_FILE)
	rm -f $(ISO_DIR)/boot/kernel.bin
