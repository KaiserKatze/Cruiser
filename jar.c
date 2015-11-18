#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "jar.h"
#include "log.h"
#include "memory.h"

#define ENTRY_MANIFEST          "META-INF/MANIFEST.MF\0"
#define BUFSIZE                 10240

static int zip_readLine(struct zip_file *, u1 *, u1 **);

extern int
parseJarfile(const char *path, JarFile *jf)
{
    struct zip *z;
    struct zip_stat st;
    struct zip_file *zf;
    int error, rbit, i, j, k;
    zip_uint64_t entries_count, entry_index, manifest_index, class_count, cap;
    zip_uint64_t *classes;
    u1 *buffer, *save;
    char *str;
    const char *entry_name;
    struct BufferIO input;
    int ze, se;
    char strError[1024];
#ifdef DEBUG
    int column;
#endif

    if (!path)
    {
        logError("Parameter 'path' in function %s is NULL!\r\n", __func__);
        return -1;
    }
    if (!jf)
    {
        logError("Parameter 'jf' in function %s is NULL!\r\n", __func__);
        return -1;
    }
    error = 0;
    buffer = (u1 *) 0;
    classes = (zip_uint64_t *) 0;
    z = zip_open(path, 0, &error);
    if (!z)
    {
        logError("Fail to open jar archieve!\r\n");
        zip_error_get(z, &ze, &se);
        if (ze != ZIP_ER_OK && se != ZIP_ER_OK)
        {
            zip_error_to_str(strError, sizeof (strError), ze, se);
            logError("Libzip: %s\r\n", strError);
        }
        goto close;
    }
    buffer = (u1 *) malloc(BUFSIZE);
    if (!buffer)
    {
        logError("Fail to allocate memory!\r\n");
        goto close;
    }
    bzero(buffer, BUFSIZE);
    bzero(jf, sizeof (JarFile));

    // read MANIFEST
    zip_stat_init(&st);
    if (zip_stat(z, ENTRY_MANIFEST, 0, &st))
    {
        logError("Fail to retrieve manifest entry stat!\r\n");
        goto close;
    }
    if (st.valid & ZIP_STAT_INDEX)
    {
        manifest_index = st.index;
        logInfo("Manifest entry index: %lli\r\n", manifest_index);
    }
    zf = zip_fopen(z, ENTRY_MANIFEST, 0);
    if (!zf)
    {
        logError("Fail to open manifest!\r\n");
        goto close;
    }
    save = buffer;
    while ((rbit = zip_readLine(zf, buffer, &save)) > 0)
    {
        str = (char *) allocMemory(16, sizeof (char));
        if (!str)
            return -1;
        strcpy(str, "Main-Class: ");
        j = strlen(str);
        for (i = 0; i < j; i++)
            if (buffer[i] != str[i])
            {
                freeMemory(str);
                str = (char *) 0;
                break;
            }
        if (str)
        {
            str = (char *) &(buffer[j]);
            j = rbit - j;
            jf->mainclass = (u1 *) malloc(j + 1);
            if (!jf->mainclass)
            {
                logError("Fail to allocate memory!\r\n");
                goto close;
            }
            memcpy(jf->mainclass, str, j);
            jf->mainclass[j] = '\0';
            continue;
        }

        str = (char *) allocMemory(16, sizeof (char));
        if (!str)
            return -1;
        strcpy(str, "Class-Path: ");
        j = strlen(str);
        for (i = 0; i < j; i++)
            if (buffer[i] != str[i])
            {
                freeMemory(str);
                str = (char *) 0;
                break;
            }
        if (str)
        {
            str = (char *) &(buffer[j]);
            j = rbit - j;
            jf->classpath = (u1 *) malloc(j + 1);
            if (!jf->classpath)
            {
                logError("Fail to allocate memory!\r\n");
                goto close;
            }
            memcpy(jf->classpath, str, j);
            jf->classpath[j] = '\0';
            continue;
        }
    }
    if (rbit < 0)
    {
        logError("IO exception[%i] in function %s!\r\n", rbit, __func__);
        goto close;
    }
    logInfo("Main-Class: %s\r\nClasspath:%s\r\n", jf->mainclass, jf->classpath);

    // iterate .class files
    entries_count = zip_get_num_entries(z, 0);
    class_count = 0;
    classes = (zip_uint64_t *) malloc(entries_count * sizeof (zip_uint64_t));
    if (entries_count < 0)
    {
        logError("Entry count: %lli\r\n", entries_count);
        goto close;
    }
    logInfo("\r\nEntry count: %lli\r\n\r\n", entries_count);
    for (entry_index = 0; entry_index < entries_count; entry_index++)
    {
        if (entry_index == manifest_index)
            continue;

        zip_stat_init(&st);
        if (zip_stat_index(z, entry_index, 0, &st))
        {
            logError("Fail to retrieve entry[%lli] stat!\r\n", entry_index);
            goto close;
        }
        if (st.valid & ZIP_STAT_NAME)
        {
            entry_name = st.name;
            k = strlen(entry_name);
            str = (char *) allocMemory(16, sizeof (char));
            if (!str)
                return -1;
            strcpy(str, ".class");
            // check file extension
            j = strlen(str);
            for (i = 0; i < j; i++)
                if (str[i] != entry_name[k - j + i])
                {
                    freeMemory(str);
                    str = (char *) 0;
                    break;
                }
            if (str)
            {
                classes[class_count++] = entry_index;
                logInfo("Class '%s' in queue.\r\n", entry_name);
            }
        }
    }

    logInfo("\r\nClass count: %lli\r\n", class_count);
    jf->class_count = class_count;
    cap = class_count * sizeof (ClassFile);
    jf->classes = (ClassFile *) malloc(cap);
    if (!jf->classes)
    {
        logError("Fail to allocate memory!\r\n");
        goto close;
    }
    bzero(jf->classes, cap);

#ifdef DEBUG
    column = (int) ceil(log10(class_count));
#endif
    input.bufsize = BUFSIZE;
    input.buffer = buffer;
    input.fp = fillBuffer_z;
    for (entry_index = 0; entry_index < class_count; entry_index++)
    {
        zip_stat_init(&st);
        zip_stat_index(z, classes[entry_index], 0, &st);
#ifdef DEBUG
        logInfo("\r\n%*lli> Parsing class '%s'...\r\n",
                column, entry_index, st.name);
#endif

        zf = zip_fopen_index(z, classes[entry_index], 0);
        if (!zf)
        {
            logError("Fail to open class file!\r\n");
            goto close;
        }
        zip_file_error_clear(zf);

        input.bufsrc = input.bufdst = 0;
        input.entry = zf;
        input.more = 1;
        if (parseClassfile(&input, &(jf->classes[entry_index])) < 0)
        {
            logError("Fail to parse class file [%i]!\r\n", entry_index);
            zip_file_error_get(zf, &ze, &se);
            if (ze != ZIP_ER_OK && se != ZIP_ER_OK)
            {
                zip_error_to_str(strError, sizeof (strError), ze, se);
                logError("Libzip: %s\r\n", strError);
            }
            zf = (struct zip_file *) 0;
            goto close;
        }
        zip_fclose(zf);
        zf = (struct zip_file *) 0;
        logInfo("\r\n");
    }

close:
    zip_close(z);
    free(buffer);
    free(classes);
    logInfo("---------------------------\r\n");
    return error;
}

extern int
freeJarfile(JarFile *jf)
{
    zip_uint64_t i;

    logInfo("Releasing JarFile memory...\r\n");
    free(jf->mainclass);
    jf->mainclass = (u1 *) 0;
    free(jf->classpath);
    jf->classpath = (u1 *) 0;
    if (jf->classes)
    {
        for (i = 0; i < jf->class_count; i++)
        {
            logInfo("%4lli> ", i);
            freeClassfile(&(jf->classes[i]));
        }
        free(jf->classes);
        jf->classes = (ClassFile *) 0;
    }
    jf->class_count = 0;
    
    return 0;
}

static int
zip_readLine(struct zip_file *zf, u1 *buffer, u1 **save)
{
    int i, rbit, len, res;

    if (!zf)
    {
        logError("Parameter 'zf' in function %s is NULL!\r\n", __func__);
        return -1;
    }
    if (!save)
    {
        logError("Parameter 'save' in function %s is NULL!\r\n", __func__);
        return -2;
    }
    // first read
    if (*save == buffer)
    {
        rbit = zip_fread(zf, buffer, BUFSIZE);
        if (rbit < 0)
        {
            logError("Fail to read from zip file!\r\n");
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
            logError("Fail to move memory!\r\n");
            return -4;
        }
        // fill in empty buffer
        i = zip_fread(zf, buffer + rbit, len);
        if (i < 0)
        {
            logError("IO exception in function %s!\r\n", __func__);
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
        logError("Runtime error in function %s!\r\n", __func__);
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
