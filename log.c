#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

extern int
logInfo(const char *format, ...)
{
#ifdef DEBUG
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
#ifdef DEBUG
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

int main(int argc, char **argv)
{
    logInfo("Integer: %i\r\n", 5);
    logError("Float  : %f\r\n", 3.8f);

    return 0;
}
