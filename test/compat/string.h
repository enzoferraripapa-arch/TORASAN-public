/**
 * @file   string.h
 * @brief  Minimal freestanding string.h replacement for test build
 * @detail Provides memset, memcpy, strlen without C runtime dependency.
 */

#ifndef COMPAT_STRING_H
#define COMPAT_STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline void *memset(void *s, int c, size_t n)
{
    unsigned char *p = (unsigned char *)s;
    size_t i;
    for (i = 0U; i < n; i++)
    {
        p[i] = (unsigned char)c;
    }
    return s;
}

static inline void *memcpy(void *dest, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    size_t i;
    for (i = 0U; i < n; i++)
    {
        d[i] = s[i];
    }
    return dest;
}

static inline size_t strlen(const char *s)
{
    size_t len = 0U;
    while (s[len] != '\0')
    {
        len++;
    }
    return len;
}

#ifdef __cplusplus
}
#endif

#endif /* COMPAT_STRING_H */
