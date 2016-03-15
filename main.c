#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include "java.h"
#include "memory.h"
#include "log.h"

int
main(int argc, char** argv)
{
    ClassFile cf;
    char *path;
    struct BufferIO input;
    time_t t;
    int result;

    path = (char *) 0;
    memset((void *) &input, 0, sizeof (struct BufferIO));
    time(&t);

    // <exec> <classfile_absolute_path>
    if (argc == 2)
    {
        path = argv[1];
        logInfo("Parsing ClassFile '%s'...\r\n", path);

        if (initWithFile(&input, path) < 0)
            goto bad_end;

        memset(&cf, 0, sizeof (ClassFile));
        result = parseClassfile(&input, &cf);

        freeClassfile(&cf);
        free(input.buffer);
        fclose(input.file);

        if (result < 0) goto bad_end;
        else            goto good_end;
    }
    else
    {
        logError("Usage: %s <classfile_absolute_path>\r\n", argv[0]);
        return 0;
    }
good_end:
    logInfo("Succeed! Time used: %.2f seconds.\r\n",
            difftime(time(0), t));
    return 0;
bad_end:
    logInfo("Fail! Time used: %.2f seconds.\r\n",
            difftime(time(0), t));
    return -1;
}
