#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
    typedef uint8_t u1;
    typedef uint16_t u2;
    typedef uint32_t u4;
    typedef uint64_t u8;

    struct String
    {
        int lnk; // true if this is a link, false if this is a copy
        int len;
        char *str;
    };

    struct File
    {
        struct String path;
        struct String parent;
        struct String name;
    };

    extern void File_initFile(const char *, struct File *);
    extern struct String *File_getParentPath();

    // get path of parent directory
    extern char *getParentPath(const char *);
    // get name of regular file or directory
    extern char *getName(const char *);
    // @see bash command `pwd`
    extern char *getWorkingDirectory();
    extern struct Deque *findClassfile(const char *);
    extern FILE *openFile(const char *, const char *);

    struct BufferIO;
    typedef char *(*func_fillBuffer)(struct BufferIO *, int);

    struct BufferIO
    {
        union {
            struct zip_file *entry;
            FILE *file;
        };
        int bufsize;
        char *buffer;
        int bufsrc;
        int bufdst;
        func_fillBuffer fp;
        char more;
        FILE *f_out;
        FILE *f_err;
    };

    extern void initWithFile(struct BufferIO *, const char *);
    extern void initWithZipEntry(struct BufferIO *, struct zip_file *);
    extern int ru1(u1 *, struct BufferIO *);
    extern int ru2(u2 *, struct BufferIO *);
    extern int ru4(u4 *, struct BufferIO *);
    extern int rbs(char *, struct BufferIO *, int);
    extern int skp(struct BufferIO *, int);
    extern char *fillBuffer_f(struct BufferIO *, int);
    extern char *fillBuffer_z(struct BufferIO *, int);
    extern int checkInput(struct BufferIO *);
#ifdef __cplusplus
}
#endif

#endif /* INPUT_H */