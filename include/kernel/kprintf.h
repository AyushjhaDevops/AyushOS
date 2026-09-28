#ifndef AYUSHOS_KPRINTF_H
#define AYUSHOS_KPRINTF_H

void kprintf(const char *format, ...);

void klog_info(const char *message);

void klog_ok(const char *message);

void klog_warn(const char *message);

void klog_error(const char *message);

#endif
