#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <zip.h>

#include "sys.h"
#include "memory.h"
#include "input.h"
#include "log.h"

#ifdef LINUX

#include <endian.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#elif defined WINDOWS

#include <WinSock2.h>

#define __func__ __FUNCTION__

#if LITTLEENDIAN
#define htobe16(x) htons(x)
#define htobe32(x) htonl(x)
#else
#define htobe16(x) (x)
#define htobe32(x) (x)
#endif

void bzero(void *ptr, size_t s)
{
	memset(ptr, 0, s);
}

#endif

#define PATH_SEPARATOR '/'
/*
extern char *
getParentPath(const char *path)
{
    char *pp;
    int i, j, k;

    i = j = k = 0;
    for (; path[i] != '\0'; i++)
        if (path[i] == PATH_SEPARATOR) { j = i; k = j; }
    // path ends with path separator
    if (j + 1 == i)
    {
    }

    return pp;
}
*/
#ifdef LINUX
extern char *
getName(const char *entry_path)
{
    struct stat st;
    char *name;
    int i, j, k;

    if (lstat(entry_path, &st) < 0) { perror("getName"); return (char *) 0; }
    j = k = 0;
    for (i = 0; entry_path[i] != '\0'; i++)
    {
        if (entry_path[i] == PATH_SEPARATOR)
        {
            j = i;
            k = j;
        }
    }
    if (S_ISDIR(st.st_mode))
    {
        i = j - k; // length between last two separators
        name = (char *) malloc(i + 1);
        memcpy(name, entry_path + k + 1, i);
        name[i] = 0;
    }
    else
    {
        i -= j; // length between end and last separator
        name = (char *) malloc(i + 1);
        memcpy(name, entry_path + j + 1, i);
        name[i] = 0;
    }
    return name;
}

extern char *
getWorkingDirectory()
{
    char *path, *ptr;

    path = ptr = (char *) 0;

    path = (char *) allocMemory(PATH_MAX + 1, sizeof (char));
    if (!path)
        return (char *) 0;

    ptr = getcwd(path, PATH_MAX + 1);
    if (!ptr)
    {
        freeMemory(path);
        return (char *) 0;
    }

    return ptr;
}

static int
scanDir(char *parent, int len_parent,
        int *nFiles, struct Deque *deque)
{
    DIR *dir;
    struct dirent *entry;
    struct stat entry_stat;
    char *entry_name, *buffer;
    int len_entry_name, len_path;

    dir = opendir(parent);
    if (!dir)
    {
        logError("Fail to open dir '%s'!\r\n", parent);
        return -1;
    }
    while (entry = readdir(dir))
    {
        entry_name = entry->d_name;
        // remove current dir entry and parent dir entry
        len_entry_name = strlen(entry_name);
        if (entry_name[0] == '.'
                && (len_entry_name == 1
                || len_entry_name == 2
                && entry_name[1] == '.'))
            continue;
        // get absolute path of entry
        bzero((void *) &(parent[len_parent]), PATH_MAX + 1 - len_parent);
        memcpy((void *) &(parent[len_parent]), entry_name, len_entry_name);
        len_path = len_parent + len_entry_name;
        // get stat of entry
        if (lstat(parent, &entry_stat) < 0)
        {
            logError("Fail to retrieve stat of '%s'!\r\n", parent);
            goto close;
        }
        if (S_ISDIR(entry_stat.st_mode))
        {
            if (parent[len_path - 1] != '/')
                parent[len_path++] = '/';
            if (scanDir(parent, (const int) len_path, nFiles, deque) < 0)
                goto close;
        }
        else if (S_ISREG(entry_stat.st_mode))
        {
            if (strcmp(&(parent[len_path - 6]), ".class"))
                continue;
            buffer = (char *) allocMemory(len_path, sizeof (char));
            if (!buffer)
                goto close;
            memcpy(buffer, parent, len_path);
            deque_push(deque, len_path, buffer);
            ++*nFiles;
        }
    }

    return 0;
close:
    closedir(dir);
    return -1;
}

extern struct Deque *
findClassfile(const char *dir)
{
    struct Deque *deque;
    int nFiles, i;
    char *parent, *ptr;
    int len_dir, len_parent;
    struct stat dir_stat;

    if (lstat(dir, &dir_stat) < 0 || !S_ISDIR(dir_stat.st_mode))
    {
        logError("Parameter 'dir' is not a directory path!\r\n");
        return (struct Deque *) 0;
    }
    parent = (char *) 0;
    len_dir = strlen(dir);
    if (dir[0] == '.' && len_dir == 1)
    {
        parent = getWorkingDirectory();
        if (parent[len_parent - 1] != '/')
            parent[len_parent++] = '/';
    }
    else if (len_dir == 2 && dir[0] == '.' && dir[1] == '.')
    {
        parent = getWorkingDirectory();
        len_parent = strlen(parent);
        ptr = strrchr(parent, '/');
        if (!ptr)
        {
            logError("The current working directory has no parent directory!\r\n");
            return (struct Deque *) 0;
        }
        if (parent[len_parent - 1] == '/')
        {
            ptr = strrchr(ptr, '/');
            if (!ptr)
            {
                logError("The current working directory has no parent directory!\r\n");
                return (struct Deque *) 0;
            }
        }
        bzero(ptr + 1, PATH_MAX - (ptr - parent));
    }
    else
    {
        parent = allocMemory(PATH_MAX + 1, sizeof (char));
        if (!parent)
            return (struct Deque *) 0;
        len_parent = strlen(dir);
        memcpy(parent, dir, len_parent);
        if (parent[len_parent - 1] != '/')
            parent[len_parent++] = '/';
    }

    nFiles = 0;
    deque = (struct Deque *) deque_createDeque();
    if (scanDir(parent, len_parent, &nFiles, deque) < 0)
    {
        logError("Error when scan dir!\r\n");
        freeMemory(parent);
        return (struct Deque *) 0;
    }

    freeMemory(parent);
    return deque;
}

extern FILE *
openFile(const char *path, const char *mode)
{
    FILE *file;
    struct stat entry_stat;
    char *prev, *mask;

    file = (FILE *) 0;
    prev = (char *) path;
    if (prev[0] == '/')
        mask = strchr(prev + 1, '/');
    else
        mask = strchr(prev, '/');
    do
    {
        *mask = 0;
        if (mkdir(prev, S_IRWXU) < 0)
        {
            if (errno != EEXIST)
            {
                perror("Fail to create new directory");
                return (FILE *) 0;
            }
            if (stat(prev, &entry_stat) < 0)
            {
                perror("Fail to get entry stat");
                return (FILE *) 0;
            }
            if (!S_ISDIR(entry_stat.st_mode))
            {
                logError("Existing entry '%s' is not directory!\r\n", prev);
                return (FILE *) 0;
            }
        }
        if (chdir(prev) < 0)
        {
            perror("Fail to change current working directory");
            return (FILE *) 0;
        }
        prev = mask + 1;
    }
    while (mask = strchr(prev, '/'));
    file = fopen(prev, mode);

    return file;
}
#endif


static inline void
initBufferIO(struct BufferIO *io)
{
    if (!io->buffer)
    {
        io->bufsize = 16384;
        io->buffer = (char *) allocMemory(io->bufsize, sizeof (char));
        if (!io->buffer)
            return;
    }
    io->bufsrc = 0;
    io->bufdst = 0;
    io->more = 1;
}

extern void
initWithFile(struct BufferIO *io, const char *file_path)
{
#ifdef LINUX
    struct stat st;
    char *path_pdir;
    char *log_name;

    initBufferIO(io);
    if (stat(file_path, &st) < 0)
    {
        perror("initWithFile:file_path");
        return;
    }
    if (!S_ISREG(st.st_mode))
    {
        logError("Parameter 'file_path' is not regular file!\r\n");
        return;
    }
#endif
#if defined WINDOWS
	fopen_s(&(io->file), file_path, "r");
#else
    io->file = fopen(file_path, "r");
#endif
    if (!io->file)
    {
        logError("Parameter file is NULL!\r\n");
        return;
    }
    io->fp = fillBuffer_f;
    /*
#ifdef LINUX
    // analyze parent path
    path_pdir = getParentPath(file_path);
    if (lstat(path_pdir, &st) < 0)
    {
        perror("initWithFile:path_pdir");
        return;
    }
    if (!S_ISDIR(st.st_mode))
    {
        logError("Fail to analyze parent path of file '%s'!\r\n", file_path);
        return;
    }
    // analyze file name
#endif
    */
}

extern void
initWithZipEntry(struct BufferIO *io, struct zip_file *entry)
{
    initBufferIO(io);
    io->entry = entry;
    io->fp = fillBuffer_z;
}

extern int
ru1(u1 *dst, struct BufferIO * input)
{
    char *ptr;

    ptr = (*input->fp)(input, sizeof (u1));
    if (!ptr)
    {
        logError("IO exception in function %s!\r\n", __func__);
        return -1;
    }

    memcpy(dst, ptr, sizeof (u1));
    input->bufsrc += sizeof (u1);

    return 0;
}

extern int
ru2(u2 *dst, struct BufferIO * input)
{
    char *ptr;

    ptr = (*input->fp)(input, sizeof (u2));
    if (!ptr)
    {
        logError("IO exception in function %s!\r\n", __func__);
        return -1;
    }

    memcpy(dst, ptr, sizeof (u2));
    *dst = htobe16(*dst);
    input->bufsrc += sizeof (u2);

    return 0;
}

extern int
ru4(u4 *dst, struct BufferIO * input)
{
    char *ptr;

    ptr = (*input->fp)(input, sizeof (u4));
    if (!ptr)
    {
        logError("IO exception in function %s!\r\n", __func__);
        return -1;
    }
    /*
    logInfo("$$ PTR: %p.\r\n", ptr);
    logInfo("$$ TRS: %i.\r\n", * ((u4 *) ptr));
    logInfo("$$ DST: %p.\r\n", dst);
    */
    memcpy(dst, ptr, sizeof (u4));
    *dst = htobe32(*dst);
    input->bufsrc += sizeof (u4);

    return 0;
}

extern int
checkInput(struct BufferIO * input)
{
    if (!input)
    {
        logError("Member 'input' is NULL!\r\n");
        return -1;
    }
    if (!input->buffer)
    {
        logError("Member 'buffer' is NULL!\r\n");
        return -1;
    }
    if (input->bufsrc > input->bufdst)
    {
        logError("Assertion error in function %s: "
                "bufsrc[%i] > bufdst[%i]\r\n",
                __func__, input->bufsrc, input->bufdst);
        return -1;
    }

    return 0;
}

extern char *
fillBuffer_f(struct BufferIO * input, int nbits)
{
    FILE *file;
    int bufsize, buflen, cap, rbit;

    if (checkInput(input))
        return (char *) 0;
    file = input->file;
    bufsize = input->bufsize;
    if (!file)
    {
        logError("Member 'file' is NULL!\r\n");
    }
    if (nbits < 0)
    {
        logError("Parameter 'nbits' in function %s is negative!\r\n", __func__);
        return (char *) 0;
    }

    // calculate the length of remaining data
    buflen = input->bufdst - input->bufsrc;
    // the remaining data is not enough
    if (buflen < nbits)
    {
        // move memory
        if (input->bufsrc != 0)
        {
            memmove(input->buffer, &(input->buffer[input->bufsrc]), buflen);
            input->bufsrc = 0;
            input->bufdst = buflen;
        }
        // fill in more data if possible
        if (input->more)
        {
            cap = bufsize - buflen;
            rbit = fread(&(input->buffer[buflen]), sizeof (u1), cap, file);
            if (rbit < 0)
            {
                logError("IO exception in function %s!\r\n", __func__);
                return (char *) 0;
            }
            else if (rbit < cap)
                input->more = 0;

            input->bufdst += rbit;
            if (input->bufdst < bufsize)
                bzero(&(input->buffer[input->bufdst]), bufsize - input->bufdst);
        }
    }

    return &(input->buffer[input->bufsrc]);
}

extern char *
fillBuffer_z(struct BufferIO * input, int nbits)
{
    struct zip_file *zf;
    int bufsize, buflen, cap, rbit;

    if (checkInput(input))
        return (char *) 0;
    zf = input->entry;
    bufsize = input->bufsize;
    if (!zf)
    {
        logError("Member 'zf' is NULL!\r\n");
        return (char *) 0;
    }
    if (nbits < 0)
    {
        logError("Parameter 'nbits' in function %s is negative!\r\n", __func__);
        return (char *) 0;
    }

    // calculate the length of remaining data
    buflen = input->bufdst - input->bufsrc;
    // the remaining data is not enough
    if (buflen < nbits)
    {
        // move memory
        if (input->bufsrc != 0)
        {
            memmove(input->buffer, &(input->buffer[input->bufsrc]), buflen);
            input->bufsrc = 0;
            input->bufdst = buflen;
        }
        // fill in more data if possible
        if (input->more)
        {
            cap = bufsize - buflen;
            rbit = zip_fread(zf,
                    (void *) &(input->buffer[buflen]),
                    (zip_uint64_t) cap);
            if (rbit < 0)
            {
                logError("IO exception in function %s!\r\n", __func__);
                return (char *) 0;
            }
            else if (rbit < cap)
                input->more = 0;

            input->bufdst += rbit;
            if (input->bufdst < bufsize)
                bzero(&(input->buffer[input->bufdst]), bufsize - input->bufdst);
        }
    }

    return &(input->buffer[input->bufsrc]);
}

extern int
rbs(char *out, struct BufferIO * input, int nbits)
{
    char *buf;
    int bufsize, rbits;

    if (!out)
    {
        logError("Parameter 'out' in function %s is NULL!\r\n", __func__);
        return -1;
    }

    bufsize = input->bufsize;
    rbits = nbits;

    while (rbits > bufsize)
    {
        buf = (*input->fp)(input, bufsize);
        if (buf < 0)
        {
            logError("IO exception in function %s!\r\n", __func__);
            return -1;
        }
        memcpy(out, buf, bufsize);
        rbits -= bufsize;
        input->bufsrc = bufsize;
    }
    buf = (*input->fp)(input, rbits);
    if (buf < 0)
    {
        logError("IO exception in function %s!\r\n", __func__);
        return -1;
    }
    memcpy(out, buf, rbits);
    input->bufsrc += rbits;

    return nbits;
}

extern int
skp(struct BufferIO *input, int nbits)
{
    int bufsize, rbits;
    
    /*
     * buggy
    if (input->fp == fillBuffer_f)
    {
        bufsize = input->bufdst - input->bufsrc;
        if (nbits > bufsize)
        {
            fseek(input->file, nbits - bufsize, SEEK_CUR);
        }
        if ((*input->fp)(input, input->bufsize) < 0)
        {
            logError("IO exception in function %s!\r\n", __func__);
            return -1;
        }
    }
    else
    */
    {
        bufsize = input->bufsize;
        rbits = nbits;
        
        while (rbits > bufsize)
        {
            if ((*input->fp)(input, bufsize) < 0)
            {
                logError("IO exception in function %s!\r\n", __func__);
                return -1;
            }
            rbits -= bufsize;
            input->bufsrc = bufsize;
        }
        if ((*input->fp)(input, rbits) < 0)
        {
            logError("IO exception in function %s!\r\n", __func__);
            return -1;
        }
        input->bufsrc += rbits;
    }

    return nbits;
}
