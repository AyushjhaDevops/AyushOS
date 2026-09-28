ENTRY(_start)

SECTIONS
{
    . = 1M;

    /*
     * Multiboot header
     *
     * Keep this as a separate output section so that
     * we can inspect and verify it independently.
     */
    .multiboot ALIGN(4) :
    {
        KEEP(*(.multiboot))
    }

    /*
     * Kernel executable code
     */
    .text ALIGN(4K) :
    {
        *(.text)
    }

    /*
     * Read-only data
     */
    .rodata ALIGN(4K) :
    {
        *(.rodata*)
    }

    /*
     * Initialized data
     */
    .data ALIGN(4K) :
    {
        *(.data)
    }

    /*
     * Uninitialized data / kernel stack
     */
    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss)
    }
}
