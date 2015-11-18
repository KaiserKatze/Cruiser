#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
extern "C" {
#endif

    extern int logInfo(const char *, ...);
    extern int logError(const char *, ...);
    extern int flogError(FILE *, const u1 *, ...);

#ifdef __cplusplus
}
#endif

#endif /* LOG_H */
