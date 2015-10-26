#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "input.h"
#include "java.h"

int main(int argc, char **argv)
{
    const char *path;
    char *name;
    char *parent;
    int i;
    
    printf("%x %x\r\n",
            ACC_FIELD,
            ~ACC_FIELD);
    printf("%x %x\r\n",
            ACC_FIELD & ~ACC_FIELD,
            0x72 & ~ACC_FIELD);
    
    /*
    path = "/home/donizyo/";
    name = getName(path);
    if (!name)
        return -1;
    parent = getParentPath(path);
    if (!parent)
    {
        free(name);
        return -1;
    }
    printf("Path  : %s\r\n"
           "Name  : %s\r\n"
           "Parent: %s\r\n",
           path, name, parent);
    free(name);
    free(parent);
    */

    return 0;
}
