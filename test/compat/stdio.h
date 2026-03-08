/**
 * @file   stdio.h
 * @brief  Minimal freestanding stdio.h replacement for test build
 * @detail Provides printf and snprintf using Windows WriteFile API.
 *         Supports format specifiers: %s, %d, %u, %lu, %02X, %x, %%
 */

#ifndef COMPAT_STDIO_H
#define COMPAT_STDIO_H

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

/* Windows kernel32.dll imports */
__declspec(dllimport) void * __stdcall GetStdHandle(unsigned long nStdHandle);
__declspec(dllimport) int __stdcall WriteFile(
    void *hFile,
    const void *lpBuffer,
    unsigned long nNumberOfBytesToWrite,
    unsigned long *lpNumberOfBytesWritten,
    void *lpOverlapped
);

#define STD_OUTPUT_HANDLE ((unsigned long)-11)

/* Internal: write raw bytes to stdout */
static inline void compat_write(const char *buf, unsigned long len)
{
    void *h;
    unsigned long written;
    if (len == 0U) { return; }
    h = GetStdHandle(STD_OUTPUT_HANDLE);
    WriteFile(h, buf, len, &written, NULL);
}

/* Internal: unsigned to decimal string, returns length */
static inline int compat_utoa(char *buf, unsigned int val)
{
    char tmp[12];
    int i = 0;
    int len;
    if (val == 0U) { buf[0] = '0'; return 1; }
    while (val > 0U) { tmp[i] = (char)('0' + (val % 10U)); val /= 10U; i++; }
    len = i;
    for (i = 0; i < len; i++) { buf[i] = tmp[len - 1 - i]; }
    return len;
}

/* Internal: unsigned long to decimal string, returns length */
static inline int compat_ultoa(char *buf, unsigned long val)
{
    char tmp[22];
    int i = 0;
    int len;
    if (val == 0UL) { buf[0] = '0'; return 1; }
    while (val > 0UL) { tmp[i] = (char)('0' + (char)(val % 10UL)); val /= 10UL; i++; }
    len = i;
    for (i = 0; i < len; i++) { buf[i] = tmp[len - 1 - i]; }
    return len;
}

/* Internal: signed int to decimal string, returns length */
static inline int compat_itoa(char *buf, int val)
{
    int off = 0;
    unsigned int uv;
    if (val < 0) { buf[0] = '-'; off = 1; uv = (unsigned int)(-(val + 1)) + 1U; }
    else { uv = (unsigned int)val; }
    return off + compat_utoa(buf + off, uv);
}

/* Internal: unsigned to hex string (uppercase), returns length */
static inline int compat_xtoa(char *buf, unsigned int val, int width, int upper)
{
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    char tmp[12];
    int i = 0;
    int len;
    if (val == 0U) { tmp[i++] = '0'; }
    else { while (val > 0U) { tmp[i] = digits[val & 0xFU]; val >>= 4; i++; } }
    len = i;
    /* Zero-pad to width */
    while (len < width) { tmp[len++] = '0'; }
    for (i = 0; i < len; i++) { buf[i] = tmp[len - 1 - i]; }
    return len;
}

/*
 * vsnprintf: minimal implementation
 * Supports: %s %d %u %lu %x %X %02X %c %%
 */
static inline int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap)
{
    size_t pos = 0;
    const char *p = fmt;
    char numbuf[24];
    int nlen;

    if (size == 0U) { return 0; }

    while (*p != '\0')
    {
        if (*p != '%')
        {
            if (pos < size - 1U) { buf[pos] = *p; }
            pos++;
            p++;
            continue;
        }

        p++; /* skip '%' */

        /* Parse optional width for zero-padding */
        {
            int zeropad = 0;
            int width = 0;
            int is_long = 0;

            if (*p == '0') { zeropad = 1; p++; }
            while (*p >= '0' && *p <= '9') { width = width * 10 + (*p - '0'); p++; }
            if (*p == 'l') { is_long = 1; p++; }

            switch (*p)
            {
                case 's':
                {
                    const char *s = va_arg(ap, const char *);
                    if (s == NULL) { s = "(null)"; }
                    while (*s != '\0')
                    {
                        if (pos < size - 1U) { buf[pos] = *s; }
                        pos++;
                        s++;
                    }
                    break;
                }
                case 'd':
                {
                    int v;
                    if (is_long) { v = (int)va_arg(ap, long); }
                    else { v = va_arg(ap, int); }
                    nlen = compat_itoa(numbuf, v);
                    { int k; for (k = 0; k < nlen; k++) { if (pos < size - 1U) { buf[pos] = numbuf[k]; } pos++; } }
                    break;
                }
                case 'u':
                {
                    if (is_long)
                    {
                        unsigned long v = va_arg(ap, unsigned long);
                        nlen = compat_ultoa(numbuf, v);
                    }
                    else
                    {
                        unsigned int v = va_arg(ap, unsigned int);
                        nlen = compat_utoa(numbuf, v);
                    }
                    { int k; for (k = 0; k < nlen; k++) { if (pos < size - 1U) { buf[pos] = numbuf[k]; } pos++; } }
                    break;
                }
                case 'X':
                case 'x':
                {
                    unsigned int v = va_arg(ap, unsigned int);
                    int w = (width > 0) ? width : 0;
                    int up = (*p == 'X') ? 1 : 0;
                    (void)zeropad; /* always zero-pad hex if width given */
                    nlen = compat_xtoa(numbuf, v, w, up);
                    { int k; for (k = 0; k < nlen; k++) { if (pos < size - 1U) { buf[pos] = numbuf[k]; } pos++; } }
                    break;
                }
                case 'c':
                {
                    char c = (char)va_arg(ap, int);
                    if (pos < size - 1U) { buf[pos] = c; }
                    pos++;
                    break;
                }
                case '%':
                {
                    if (pos < size - 1U) { buf[pos] = '%'; }
                    pos++;
                    break;
                }
                case '\0':
                {
                    /* Premature end of format string */
                    goto done;
                }
                default:
                {
                    /* Unknown specifier - output as-is */
                    if (pos < size - 1U) { buf[pos] = '%'; }
                    pos++;
                    if (pos < size - 1U) { buf[pos] = *p; }
                    pos++;
                    break;
                }
            }
        }
        p++;
    }

done:
    if (pos < size) { buf[pos] = '\0'; }
    else if (size > 0U) { buf[size - 1U] = '\0'; }

    return (int)pos;
}

static inline int snprintf(char *buf, size_t size, const char *fmt, ...)
{
    va_list ap;
    int ret;
    va_start(ap, fmt);
    ret = vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    return ret;
}

static inline int printf(const char *fmt, ...)
{
    char buf[512];
    va_list ap;
    int len;
    va_start(ap, fmt);
    len = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (len > 0)
    {
        unsigned long wlen = (unsigned long)len;
        if (wlen > sizeof(buf) - 1U) { wlen = sizeof(buf) - 1U; }
        compat_write(buf, wlen);
    }
    return len;
}

static inline int puts(const char *s)
{
    size_t len = 0;
    const char *p = s;
    while (*p != '\0') { len++; p++; }
    compat_write(s, (unsigned long)len);
    compat_write("\n", 1);
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* COMPAT_STDIO_H */
