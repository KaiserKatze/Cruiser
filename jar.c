#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
    int error, rbit, i, j, k;
    zip_uint64_t entries_count, entry_index, manifest_index, class_count, cap;
    zip_uint64_t *classes;
    char *buffer, *save, *str;
    const char *entry_name;
    struct BufferInput input;

    error = 0;
    buffer = (char *) 0;
    classes = (zip_uint64_t *) 0;
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

    // read MANIFEST
    zip_stat_init(&st);
    if (zip_stat(z, ENTRY_MANIFEST, 0, &st))
    {
        fprintf(stderr, "Fail to retrieve manifest entry stat!\r\n");
        goto close;
    }
    if (st.valid & ZIP_STAT_INDEX)
    {
        manifest_index = st.index;
        printf("Manifest entry index: %lli\r\n", manifest_index);
    }
    zf = zip_fopen(z, ENTRY_MANIFEST, 0);
    if (!zf)
    {
        fprintf(stderr, "Fail to open manifest!\r\n");
        goto close;
    }
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
    printf("Main-Class: %s\r\nClasspath:%s\r\n", jf->mainclass, jf->classpath);

    // iterate .class files
    entries_count = zip_get_num_entries(z, 0);
    class_count = 0;
    classes = (zip_uint64_t *) malloc(entries_count * sizeof (zip_uint64_t));
    if (entries_count < 0)
    {
        fprintf(stderr, "Entry count: %lli\r\n", entries_count);
        goto close;
    }
    printf("\r\nEntry count: %lli\r\n\r\n", entries_count);
    for (entry_index = 0; entry_index < entries_count; entry_index++)
    {
        if (entry_index == manifest_index)
            continue;

        zip_stat_init(&st);
        if (zip_stat_index(z, entry_index, 0, &st))
        {
            fprintf(stderr, "Fail to retrieve entry[%lli] stat!\r\n", entry_index);
            goto close;
        }
        if (st.valid & ZIP_STAT_NAME)
        {
            entry_name = st.name;
            k = strlen(entry_name);
            str = ".class\0";
            j = strlen(str);
            for (i = 0; i < j; i++)
                if (str[i] != entry_name[k - j + i])
                {
                    str = (char *) 0;
                    break;
                }
            if (str)
            {
                classes[class_count++] = entry_index;
                printf("Class '%s' in queue.\r\n", entry_name);
            }
        }
    }

    printf("\r\nClass count: %lli\r\n", class_count);
    jf->class_count = class_count;
    cap = class_count * sizeof (ClassFile);
    jf->classes = (ClassFile *) malloc(cap);
    if (!jf->classes)
    {
        fprintf(stderr, "Fail to allocate memory!\r\n");
        goto close;
    }
    bzero(jf->classes, cap);

    input.bufsize = BUFSIZE;
    input.buffer = buffer;
    input.fp = fillBuffer_z;
    for (entry_index = 0; entry_index < class_count; entry_index++)
    {
        zip_stat_init(&st);
        zip_stat_index(z, classes[entry_index], 0, &st);
        printf("\r\n%4lli> Parsing class '%s'...\r\n",
                entry_index, st.name);

        zf = zip_fopen_index(z, classes[entry_index], 0);
        if (!zf)
        {
            fprintf(stderr, "Fail to open manifest!\r\n");
            goto close;
        }

        input.entry = zf;
        input.more = 1;
        printf("zip_file: %p\r\n"
                "buffer  : %p\r\n"
                "bufsize : %i\r\n", zf, buffer, BUFSIZE);
        parseClassfile(&input, &(jf->classes[entry_index]));

        printf("\r\n");
    }

close:
    zip_close(z);
    free(buffer);
    free(classes);
    printf("---------------------------\r\n");
    return error;
}

extern int
freeJarfile(JarFile *jf)
{
    zip_uint64_t i;

    printf("Releasing JarFile memory...\r\n");
    free(jf->mainclass);
    jf->mainclass = (char *) 0;
    free(jf->classpath);
    jf->classpath = (char *) 0;
    if (jf->classes)
    {
        for (i = 0; i < jf->class_count; i++)
        {
            printf("%4lli> ", i);
            freeClassfile(&(jf->classes[i]));
        }
        free(jf->classes);
        jf->classes = (ClassFile *) 0;
    }
    jf->class_count = 0;
    
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
