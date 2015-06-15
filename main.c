/* 
 * File:   main.c
 * Author: donizyo
 *
 * Created on March 20, 2015, 12:50 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "java.h"

int
main(int argc, char** argv)
{
    char *path;
    ClassFile cf;

    if (argc > 1)
    {
        path = argv[1];
        parseClassfile((const char *) path, &cf);
        freeClassfile(&cf);
        return 0;
    }

    fprintf(stderr,
            "Usage:\r\n"
            "%s <path>\r\n\r\n",
            argv[0]);
    return -1;
}

