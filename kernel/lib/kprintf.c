#include <stdint.h>
#include <kernel/kprintf.h>
#include <kernel/terminal.h>

typedef __builtin_va_list va_list;

#define va_start(ap, last) __builtin_va_start(ap, last)
#define va_arg(ap, type)   __builtin_va_arg(ap, type)
#define va_end(ap)         __builtin_va_end(ap)

static void print_unsigned_decimal(uint32_t value)
{
    char buffer[11];
    uint32_t i = 0;

    if (value == 0)
    {
        terminal_putchar('0');
        return;
    }

    while (value > 0)
    {
        buffer[i++] =
            (char)('0' + (value % 10));

        value /= 10;
    }

    while (i > 0)
    {
        terminal_putchar(buffer[--i]);
    }
}

static void print_signed_decimal(int32_t value)
{
    if (value < 0)
    {
        terminal_putchar('-');

        /*
         * Avoid undefined behavior from directly
         * negating INT_MIN.
         */
        uint32_t magnitude =
            (uint32_t)(-(value + 1)) + 1;

        print_unsigned_decimal(magnitude);
        return;
    }

    print_unsigned_decimal((uint32_t)value);
}

static void print_hex(uint32_t value)
{
    const char *digits =
        "0123456789ABCDEF";

    terminal_write("0x");

    if (value == 0)
    {
        terminal_putchar('0');
        return;
    }

    char buffer[8];
    uint32_t i = 0;

    while (value > 0)
    {
        buffer[i++] =
            digits[value & 0xF];

        value >>= 4;
    }

    while (i > 0)
    {
        terminal_putchar(buffer[--i]);
    }
}

void kprintf(const char *format, ...)
{
    if (format == 0)
        return;

    va_list args;

    va_start(args, format);

    for (uint32_t i = 0; format[i] != '\0'; i++)
    {
        if (format[i] != '%')
        {
            terminal_putchar(format[i]);
            continue;
        }

        i++;

        switch (format[i])
        {
            case '%':
                terminal_putchar('%');
                break;

            case 'c':
            {
                int value = va_arg(args, int);
                terminal_putchar((char)value);
                break;
            }

            case 's':
            {
                const char *value =
                    va_arg(args, const char *);

                if (value == 0)
                    terminal_write("(null)");
                else
                    terminal_write(value);

                break;
            }

            case 'd':
            {
                int32_t value =
                    va_arg(args, int32_t);

                print_signed_decimal(value);
                break;
            }

            case 'u':
            {
                uint32_t value =
                    va_arg(args, uint32_t);

                print_unsigned_decimal(value);
                break;
            }

            case 'x':
            case 'X':
            {
                uint32_t value =
                    va_arg(args, uint32_t);

                print_hex(value);
                break;
            }

            default:
                terminal_putchar('%');
                terminal_putchar(format[i]);
                break;
        }
    }

    va_end(args);
}

void klog_info(const char *message)
{
    terminal_write("[INFO] ");
    terminal_write(message);
    terminal_putchar('\n');
}

void klog_ok(const char *message)
{
    terminal_write("[ OK ] ");
    terminal_write(message);
    terminal_putchar('\n');
}

void klog_warn(const char *message)
{
    terminal_write("[WARN] ");
    terminal_write(message);
    terminal_putchar('\n');
}

void klog_error(const char *message)
{
    terminal_write("[ERROR] ");
    terminal_write(message);
    terminal_putchar('\n');
}
