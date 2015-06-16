#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zip.h>

#include "java.h"
#include "jar.h"

#define ENTRY_MANIFEST          "META-INF/MANIFEST.MF\0"
#define BUFSIZE                 10240

static int zip_readLine(struct zip_file *, char *, char **);

extern int
parseJarfile(const char *path, JarFile *jf)
{
    struct zip *z;
    struct zip_stat st;
    struct zip_file *zf;
    int error, rbit, i, j;
    char *buffer, *save, *str;

    error = 0;
    z = zip_open(path, 0, &error);
    if (!z)
    {
        fprintf(stderr, "Fail to open jar archieve!\r\n");
        goto close;
    }
    buffer = (char *) malloc(BUFSIZE);
    if (!buffer)
    {
        fprintf(stderr, "Fail to allocate memory!\r\n");
        goto close;
    }
    bzero(buffer, BUFSIZE);
    bzero(jf, sizeof (JarFile));

    zip_stat_init(&st);
    if (zip_stat(z, ENTRY_MANIFEST, 0, &st))
    {
        fprintf(stderr, "Fail to retrieve manifest entry stat!\r\n");
        goto close;
    }
    zf = zip_fopen(z, ENTRY_MANIFEST, 0);
    //while ((rbit = zip_fread(zf, buffer, BUFSIZE)) > 0)
    //    printf("%s", buffer);
    save = buffer;
    while ((rbit = zip_readLine(zf, buffer, &save)) > 0)
    {
        str = "Main-Class: \0";
        j = strlen(str);
        for (i = 0; i < j; i++)
            if (buffer[i] != str[i])
            {
                str = (char *) 0;
                break;
            }
        if (str)
        {
            str = &(buffer[j]);
            j = rbit - j;
            jf->mainclass = (char *) malloc(j + 1);
            if (!jf->mainclass)
            {
                fprintf(stderr, "Fail to allocate memory!\r\n");
                goto close;
            }
            memcpy(jf->mainclass, str, j);
            jf->mainclass[j] = '\0';
            continue;
        }

        str = "Class-Path: \0";
        j = strlen(str);
        for (i = 0; i < j; i++)
            if (buffer[i] != str[i])
            {
                str = (char *) 0;
                break;
            }
        if (str)
        {
            str = &(buffer[j]);
            j = rbit - j;
            jf->classpath = (char *) malloc(j + 1);
            if (!jf->classpath)
            {
                fprintf(stderr, "Fail to allocate memory!\r\n");
                goto close;
            }
            memcpy(jf->classpath, str, j);
            jf->classpath[j] = '\0';
            continue;
        }
    }
    if (rbit < 0)
    {
        fprintf(stderr, "IO exception[%i] in function %s!\r\n", rbit, __func__);
        goto close;
    }

close:
    zip_close(z);
    free(buffer);
    return error;
}

extern int
freeJarfile(JarFile *jf)
{
    if (!jf)
        return -1;

    free(jf->mainclass);
    free(jf->classpath);
    
    return 0;
}

static int
zip_readLine(struct zip_file *zf, char *buffer, char **save)
{
    int i, rbit, len, res;

    if (!zf)
    {
        fprintf(stderr, "Parameter 'zf' in function %s is NULL!\r\n", __func__);
        return -1;
    }
    if (!save)
    {
        fprintf(stderr, "Parameter 'save' in function %s is NULL!\r\n", __func__);
        return -2;
    }
    // first read
    if (*save == buffer)
    {
        rbit = zip_fread(zf, buffer, BUFSIZE);
        if (rbit < 0)
        {
            fprintf(stderr, "Fail to read from zip file!\r\n");
            return -3;
        }
    }
    // more read
    else if (*save > buffer)
    {
        len = *save - buffer;
        rbit = BUFSIZE - len;
        if (memmove(buffer, *save, rbit) != buffer)
        {
            fprintf(stderr, "Fail to move memory!\r\n");
            return -4;
        }
        // fill in empty buffer
        i = zip_fread(zf, buffer + rbit, len);
        if (i < 0)
        {
            fprintf(stderr, "IO exception in function %s!\r\n", __func__);
            return -5;
        }
        else if (i > 0)
        {
            rbit += i;
            len -= i;
        }
        bzero(buffer + rbit, len);
    }
    else
    {
        fprintf(stderr, "Runtime error in function %s!\r\n", __func__);
        return -6;
    }

    res = BUFSIZE;
    for (i = 0; i < rbit; i++)
    {
        switch (buffer[i])
        {
            case '\r':
                buffer[i] = '\0';
                res = i;
                *save = &(buffer[++i]);
                if (buffer[i] == '\n')
                {
                    buffer[i] = '\0';
                    *save = &(buffer[++i]);
                }
                goto end;
            case '\n':
                buffer[i] = '\0';
                res = i;
                *save = &(buffer[++i]);
                goto end;
            case '\0':
                res = i;
                *save = &(buffer[i]);
                goto end;
        }
    }

end:
    return res;
}

int main(int nargs, char **args)
{
    char *path;
    JarFile jf;

    if (nargs < 2)
    {
        fprintf(stderr, "Usage:\r\n"
                "%s <path>\r\n",
                args[0]);
        return -1;
    }
    path = args[1];

    parseJarfile(path, &jf);
    printf("Main class: %s\r\nClasspath: %s\r\n",
            jf.mainclass, jf.classpath);
    freeJarfile(&jf);
    return 0;
}
