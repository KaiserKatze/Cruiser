#ifndef JAR_H
#define JAR_H

#include <zip.h>

#include "java.h"

#ifdef  __cplusplus
extern "C" {
#endif


    typedef struct
    {
        char *mainclass;
        char *classpath;
        zip_uint64_t class_count;
        ClassFile *classes;
    } JarFile;

    extern int parseJarfile(const char *, JarFile *);
    extern int freeJarfile(JarFile *);

#ifdef  __cplusplus
}
#endif

#endif /* JAR_H */
