ASM      = nasm
CC       = gcc
LD       = ld

CFLAGS   = -m32 \
           -ffreestanding \
           -fno-pie \
           -fno-stack-protector \
           -nostdlib \
           -nostdinc \
           -Wall \
           -Wextra

LDFLAGS  = -m elf_i386 \
           -T scripts/linker.ld

KERNEL   = build/kernel.bin

ASM_SRC  = arch/x86/boot.asm

C_SRCS   = \
           kernel/kernel.c \
           kernel/drivers/terminal.c \
           kernel/lib/kprintf.c

ASM_OBJ  = build/boot.o

C_OBJS   = \
           build/kernel.o \
           build/terminal.o \
           build/kprintf.o

OBJS     = $(ASM_OBJ) $(C_OBJS)

ISO_DIR  = iso
ISO_FILE = AyushOS.iso


.PHONY: all clean iso run debug

all: $(KERNEL)


$(ASM_OBJ): $(ASM_SRC)
	@mkdir -p build
	$(ASM) -f elf32 $< -o $@


build/kernel.o: kernel/kernel.c
	@mkdir -p build
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@


build/terminal.o: kernel/drivers/terminal.c
	@mkdir -p build
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@


build/kprintf.o: kernel/lib/kprintf.c
	@mkdir -p build
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@


$(KERNEL): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^


iso: $(KERNEL)
	mkdir -p $(ISO_DIR)/boot
	cp $(KERNEL) $(ISO_DIR)/boot/kernel.bin
	grub-mkrescue -o $(ISO_FILE) $(ISO_DIR)


run: iso
	qemu-system-i386 -cdrom $(ISO_FILE)


debug: iso
	qemu-system-i386 \
		-cdrom $(ISO_FILE) \
		-no-reboot \
		-no-shutdown \
		-s \
		-S


clean:
	rm -rf build/*
	rm -f $(ISO_FILE)
	rm -f $(ISO_DIR)/boot/kernel.bin
