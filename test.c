#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "input.h"

int main(int argc, char **argv)
{
    const char *path;
    char *name;
    char *parent;
    int i;

    for (i = 0; i < 3; i++)
    {
        printf("%i\r\n", i);
    }
    printf("%i\r\n", i);

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
