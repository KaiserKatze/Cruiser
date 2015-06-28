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
    int res;
    va_list vl;

    va_start(vl, format);
    res = vfprintf(stderr, format, vl);
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
