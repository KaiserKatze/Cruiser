/* 
 * File:   sys.h
 * Author: donizyo
 *
 * Created on October 31, 2015, 3:13 AM
 */

#ifndef SYS_H
#define	SYS_H

#ifdef	__cplusplus
extern "C" {
#endif
    
/*
 * Pre-defined macros:
 * https://gcc.gnu.org/onlinedocs/cpp/Predefined-Macros.html
 * https://msdn.microsoft.com/en-us/library/b0084kay.aspx
 */
#if defined _WIN32    
#define WINDOWS
#elif defined unix
#define LINUX
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* SYS_H */

