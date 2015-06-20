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
#include <signal.h>

#include "jar.h"

#define OPTION_JAR                          "-jar"
#define OPTION_CLASSPATH                    "-classpath"

JarFile *jf;
ClassFile *cf;
struct BufferInput input;

static void freeAll();
static void handleInterruption(int);

int
main(int argc, char** argv)
{
    char *path, *name;
    time_t t;

    cf = (ClassFile *) 0;
    jf = (JarFile *) 0;
    path = name = (char *) 0;
    bzero(&input, sizeof (struct BufferInput));
    signal(SIGINT, handleInterruption);
    time(&t);

    if (argc == 2)
    {
        path = argv[1];
        printf("Parsing ClassFile '%s'...\r\n", path);

        input.bufsize = 1024;
        input.buffer = (char *) malloc(input.bufsize);
        if (!input.buffer)
        {
            logError("Fail to allocate memory!\r\n");
            return -1;
        }

        input.fp = fillBuffer_f;

        input.file = fopen(path, "r");
        if (!input.file)
        {
            logError("Fail to open file %s!\r\n", path);
            free(input.buffer);
            input.buffer = (char *) 0;
            return -1;
        }

        input.more = 1;

        cf = (ClassFile *) malloc(sizeof (ClassFile));
        if (!cf)
        {
            logError("Fail to allocate memory!\r\n");
            free(input.buffer);
            input.buffer = (char *) 0;
            fclose(input.file);
            input.file = (FILE *) 0;
            return -1;
        }

        if (parseClassfile(&input, cf) < 0)
        {
            freeClassfile(cf);
            free(cf);
            cf = (ClassFile *) 0;
            free(input.buffer);
            input.buffer = (char *) 0;
            fclose(input.file);
            input.file = (FILE *) 0;
            return -1;
        }

        freeClassfile(cf);
        free(cf);
        cf = (ClassFile *) 0;
        free(input.buffer);
        input.buffer = (char *) 0;
        fclose(input.file);
        input.file = (FILE *) 0;

        goto good_end;
    }
    else if (argc == 3)
    {
        if (!strcmp(argv[1], OPTION_JAR))
        {
            path = argv[2];
            printf("Parsing JarFile '%s'...\r\n", path);

            jf = (JarFile *) malloc(sizeof (JarFile));
            if (!jf)
            {
                logError("Fail to allocate memory!\r\n");
                return -1;
            }
            if (parseJarfile(path, jf) < 0)
            {
                freeJarfile(jf);
                return -1;
            }

            freeJarfile(jf);
            free(jf);
            jf = 0;

            goto good_end;
        }
    }
    else if (argc == 4)
    {
        if (!strcmp(argv[1], OPTION_CLASSPATH))
        {
            path = argv[2];
            name = argv[3];
            printf("Parsing ClassFile '%s' in JAR '%s'...\r\n",
                    name, path);
            cf = (ClassFile *) malloc(sizeof (ClassFile));
            if (!cf)
            {
                logError("Fail to allocate memory!\r\n");
                return -1;
            }

            if (parseClassfileInJar(path, name, cf) < 0)
            {
                freeClassfile(cf);
                free(cf);
                cf = (ClassFile *) 0;
                return -1;
            }

            freeClassfile(cf);
            free(cf);
            cf = (ClassFile *) 0;

            goto good_end;
        }
    }

usage:
    fprintf(stderr,
            "Usage: %s %s <jarfile>\r\n"
            "   or  %s [%s <path>] <classfile>\r\n",
            argv[0], OPTION_JAR, argv[0], OPTION_CLASSPATH);
    return 0;
good_end:
    printf("Time used: %.2f seconds.\r\n",
            difftime(time(0), t));
    return 0;
}

static void
freeAll()
{
    if (cf)
    {
        freeClassfile(cf);
        free(cf);
        cf = (ClassFile *) 0;
        fclose(input.file);
        input.file = (FILE *) 0;
        free(input.buffer);
        input.buffer = (char *) 0;
    }
    if (jf)
    {
        freeJarfile(jf);
        free(jf);
        jf = (JarFile *) 0;
    }
}

static void
handleInterruption(int param)
{
    logError("Interrupted[%i]!\r\n", param);
    freeAll();
    exit(SIGINT);
}
