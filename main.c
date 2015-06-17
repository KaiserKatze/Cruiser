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

#include "jar.h"

int
main(int argc, char** argv)
{
    char *path;
    JarFile jf;

    if (argc < 2)
    {
        fprintf(stderr, "Usage:\r\n\t%s <path>\r\n", argv[0]);
        return -1;
    }

    path = argv[1];
    parseJarfile(path, &jf);
    freeJarfile(jf);

    return 0;
}

