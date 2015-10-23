#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include "jar.h"
#include "memory.h"

#define OPTION_JAR                          "-jar"
#define OPTION_CLASSPATH                    "-cp"

JarFile *jf;
ClassFile *cf;
struct BufferIO input;

static void freeAll();
static void handleInterruption(int);
static void loadJar(char *, char *);

int
main(int argc, char** argv)
{
    char *path, *name;
    time_t t;

    cf = (ClassFile *) 0;
    jf = (JarFile *) 0;
    path = name = (char *) 0;
    signal(SIGINT, handleInterruption);
    time(&t);

    if (argc == 2)
    {
        // absolute path of a class file
        path = argv[1];
        printf("Parsing ClassFile '%s'...\r\n", path);

        input.bufsize = 1024;
        input.buffer = (char *) allocMemory(input.bufsize, sizeof (char));
        if (!input.buffer)
            goto bad_end;
        input.bufsrc = input.bufdst = 0;
        input.fp = fillBuffer_f;
        input.file = fopen(path, "r");
        if (!input.file)
        {
            logError("Fail to open file %s!\r\n", path);
            free(input.buffer);
            goto bad_end;
        }
        input.more = 1;
        input.f_out = (FILE *) 0;
        input.f_err = (FILE *) 0;

        cf = (ClassFile *) allocMemory(1, sizeof (ClassFile));
        if (!cf)
        {
            free(input.buffer);
            fclose(input.file);
            goto bad_end;
        }

        if (parseClassfile(&input, cf) < 0)
        {
            freeClassfile(cf);
            free(cf);
            free(input.buffer);
            fclose(input.file);
            goto bad_end;
        }

        freeClassfile(cf);
        free(cf);
        free(input.buffer);
        fclose(input.file);
        goto good_end;
    }
    /*
    else if (argc == 3)
    {
        if (!strcmp(argv[1], OPTION_JAR))
        {
            path = argv[2];
            printf("Parsing JarFile '%s'...\r\n", path);

            jf = (JarFile *) allocMemory(1, sizeof (JarFile));
            if (!jf)
            {
                logError("Fail to allocate memory!\r\n");
                goto bad_end;
            }
            if (parseJarfile(path, jf) < 0)
            {
                freeJarfile(jf);
                goto bad_end;
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
            printf("Parsing ClassFile '%s' in JAR '%s'...\r\n", name, path);
            cf = (ClassFile *) allocMemory(1, sizeof (ClassFile));
            if (!cf)
                goto bad_end;

            if (parseClassfileInJar(path, name, cf) < 0)
            {
                freeClassfile(cf);
                free(cf);
                goto bad_end;
            }

            freeClassfile(cf);
            free(cf);
            goto good_end;
        }
    }
    */
usage:
    /*
    fprintf(stderr,
            "Usage: %s %s <jarfile>\r\n"
            "   or  %s [%s <path>] <classfile>\r\n",
            argv[0], OPTION_JAR, argv[0], OPTION_CLASSPATH);
    */
    fprintf(stderr, "Usage: %s <classfile>\r\n", argv[0]);
    return 0;
good_end:
    printf("Time used: %.2f seconds.\r\n",
            difftime(time(0), t));
    return 0;
bad_end:
    printf("Time used: %.2f seconds.\r\n",
            difftime(time(0), t));
    return -1;
}

static void
loadJar(char *path_jar, char *name_pattern)
{
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
