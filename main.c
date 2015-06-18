/* 
 * File:   main.c
 * Author: donizyo
 *
 * Created on March 20, 2015, 12:50 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "jar.h"

int
main(int argc, char** argv)
{
    char *path;
    struct BufferInput input;
    JarFile jf;
    ClassFile cf;

    if (argc < 2)
    {
usage:
        fprintf(stderr, "Usage:\r\n\t%s [-jar] <path>\r\n", argv[0]);
        return -1;
    }

    if (argc == 2)
    {
        path = argv[1];
        printf("Parsing ClassFile '%s'...\r\n", path);

        input.buffer = (char *) malloc(input.bufsize = 1024);
        if (!input.buffer)
            return -1;

        input.fp = fillBuffer_f;

        input.file = fopen(path, "r");
        if (!input.file)
        {
            free(input.buffer);
            input.buffer = (char *) 0;
            return -1;
        }

        input.more = 1;

        bzero(&cf, sizeof (ClassFile));
        parseClassfile(&input, &cf);

        freeClassfile(&cf);
        free(input.buffer);
        input.buffer = (char *) 0;
    }
    else if (argc == 3)
    {
        if (strncmp(argv[1], "-jar", 4))
            goto usage;
        path = argv[2];
        printf("Parsing JarFile '%s'...\r\n", path);
        parseJarfile(path, &jf);
        freeJarfile(&jf);
    }

    return 0;
}

