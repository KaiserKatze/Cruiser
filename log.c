#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

extern int
logInfo(const char *format, ...)
{
#if (defined DEBUG && defined LOG_INFO)
    int res;
    va_list vl;

    va_start(vl, format);
    res = vfprintf(stdout, format, vl);
    va_end(vl);

    return res;
#else
    return 0;
#endif
}

extern int
logError(const char *format, ...)
{
#if (defined DEBUG && defined LOG_ERROR)
    int res, len;
    char *fmt;
    va_list vl;

    va_start(vl, format);
    len = strlen(format);
    fmt = (char *) malloc(11 + len);
    memcpy(fmt, "[Error > ", 9);
    memcpy(fmt + 9, format, len);
    memcpy(fmt + 9 + len, "\r\n", 2);
    fmt[10 + len] = '\0';
    res = vfprintf(stderr, fmt, vl);
    va_end(vl);

    return res;
#else
    return 0;
#endif
}

extern int
flogError(FILE *file, const char *format, ...)
{
#if (defined DEBUG && defined LOG_ERROR)
    int res;
    va_list vl;

    va_start(vl, format);
    res = vfprintf(file, format, vl);
    va_end(vl);

    return res;
#else
    return 0;
#endif
}
