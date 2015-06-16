#ifndef JAR_H
#define JAR_H

#ifdef  __cplusplus
extern "C" {
#endif


    typedef struct
    {
        char *mainclass;
        char *classpath;
    } JarFile;

    extern int parseJarfile(const char *, JarFile *);
    extern int freeJarfile(JarFile *);

#ifdef  __cplusplus
}
#endif

#endif /* JAR_H */
