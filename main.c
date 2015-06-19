/* 
 * File:   main.c
 * Author: donizyo
 *
 * Created on March 20, 2015, 12:50 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "jar.h"

#define OPTION_JAR                          "-jar"
#define OPTION_CLASSPATH                    "-classpath"

int
main(int argc, char** argv)
{
    char *path, *name;
    struct BufferInput input;
    JarFile jf;
    ClassFile cf;
    time_t t;

    if (argc == 2)
    {
        time(&t);
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

        goto good_end;
    }
    else if (argc == 3)
    {
        if (!strcmp(argv[1], OPTION_JAR))
        {
            time(&t);
            path = argv[2];
            printf("Parsing JarFile '%s'...\r\n", path);
            parseJarfile(path, &jf);
            freeJarfile(&jf);

            goto good_end;
        }
    }
    else if (argc == 4)
    {
        if (!strcmp(argv[1], OPTION_CLASSPATH))
        {
            time(&t);
            path = argv[2];
            name = argv[3];
            printf("Parsing ClassFile '%s' in JAR '%s'...\r\n",
                    name, path);
            parseClassfileInJar(path, name, &cf);
            freeClassfile(&cf);

            goto good_end;
        }
    }

    fprintf(stderr,
        "Usage: %s %s <jarfile>\r\n"
        "   or  %s [%s <path>] <classfile>\r\n",
        argv[0], OPTION_JAR, argv[0], OPTION_CLASSPATH);
    return -1;
good_end:
    printf("Time used: %.2f seconds.\r\n",
        difftime(time(0), t));
    return 0;
}

