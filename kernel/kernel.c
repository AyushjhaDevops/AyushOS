#include <stdint.h>

void kernel_main(void)
{
    volatile uint16_t *video_memory =
        (volatile uint16_t *)0xB8000;

    const char *message = "Welcome to AyushOS";

    for (uint32_t i = 0; message[i] != '\0'; i++)
    {
        video_memory[i] =
            ((uint16_t)0x07 << 8) |
            (uint8_t)message[i];
    }

    while (1)
    {
        __asm__ volatile ("hlt");
    }
}
