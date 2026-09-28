#include <stdint.h>
#include <kernel/gdt.h>

typedef struct __attribute__((packed))
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} gdt_entry_t;

typedef struct __attribute__((packed))
{
    uint16_t limit;
    uint32_t base;
} gdt_ptr_t;

/*
 * GDT layout:
 *
 * Entry 0 -> Null descriptor
 * Entry 1 -> Kernel code segment
 * Entry 2 -> Kernel data segment
 */
gdt_entry_t gdt[3];
gdt_ptr_t gp;

/*
 * Assembly function implemented in:
 *
 * arch/x86/gdt_flush.asm
 */
extern void gdt_flush(uint32_t gdt_ptr_address);

static void gdt_set_gate(
    uint32_t index,
    uint32_t base,
    uint32_t limit,
    uint8_t access,
    uint8_t granularity)
{
    gdt[index].base_low =
        (uint16_t)(base & 0xFFFF);

    gdt[index].base_middle =
        (uint8_t)((base >> 16) & 0xFF);

    gdt[index].base_high =
        (uint8_t)((base >> 24) & 0xFF);

    gdt[index].limit_low =
        (uint16_t)(limit & 0xFFFF);

    gdt[index].granularity =
        (uint8_t)((limit >> 16) & 0x0F);

    gdt[index].granularity |=
        granularity & 0xF0;

    gdt[index].access = access;
}

void gdt_initialize(void)
{
    /*
     * GDTR limit is the size of the GDT minus one.
     */
    gp.limit = sizeof(gdt) - 1;

    /*
     * GDTR base points to our GDT.
     */
    gp.base = (uint32_t)&gdt;

    /*
     * Entry 0:
     * Null descriptor.
     */
    gdt_set_gate(
        0,
        0,
        0,
        0,
        0
    );

    /*
     * Entry 1:
     * Kernel code segment.
     *
     * Access:
     * 0x9A
     *
     * Granularity:
     * 0xCF
     */
    gdt_set_gate(
        1,
        0,
        0xFFFFFFFF,
        0x9A,
        0xCF
    );

    /*
     * Entry 2:
     * Kernel data segment.
     *
     * Access:
     * 0x92
     *
     * Granularity:
     * 0xCF
     */
    gdt_set_gate(
        2,
        0,
        0xFFFFFFFF,
        0x92,
        0xCF
    );

    /*
     * Load our GDT and reload segment registers.
     */
    gdt_flush((uint32_t)&gp);
}
