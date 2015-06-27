#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <endian.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "memory.h"
#include "input.h"

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
