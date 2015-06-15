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

/*
 * /home/donizyo/NetBeansProjects/Servimg/build/classes/net/donizyo/servimg/desktop/DesktopServimg.class
 */
int
main(int argc, char** argv)
{
    char *path;
    ClassFile cf;

    if (argc > 1)
    {
        path = argv[1];
        decompile((const char *) path, &cf);
        return 0;
    }

    fprintf(stderr,
            "Usage:\r\n"
            "%s <path>\r\n\r\n",
            argv[0]);
    return -1;
}

