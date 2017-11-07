#ifndef __AURORA_COMMON_STRING_H
#define __AURORA_COMMON_STRING_H

#include <ctype.h>

#include <functional>
#include <algorithm>
#include <string>
#include <vector>

#include "common_base.h"

#if AURORA_OS_WINDOWS == aurora_OS
#include <tchar.h>

#if _MSC_VER < 1300
#define    strnlen(sz)                _strncnt(sz)
#endif    /* _MSC_VER */

#define strupr(sz)                _strupr(sz)
#define strlwr(sz)                _strlwr(sz)

#define strcasecmp(s1, s2)        stricmp(s1, s2)
#define strncasecmp(s1, s2, n)    strnicmp(s1, s2, n)

#define bzero(p,n)                memset(p, 0, n)

#elif AURORA_OS_LINUX == aurora_OS || AURORA_OS_ANDROID == aurora_OS

#define stricmp(s1, s2)            strcasecmp(s1, s2)
#define strnicmp(s1, s2, n)        strncasecmp(s1, s2, n)

#define strupr(p){                                      \
            int i=0;                                    \
                                    while(p[i])                                 \
                                                                        {                                           \
                p[i] = (char)toupper(p[i]);             \
                i++;                                    \
                                                                        }                                           \
                                }

#define strlwr(p) {                                     \
            int i=0;                                    \
                                    while(p[i])                                 \
                                                                        {                                           \
                p[i] = (char)tolower(p[i]);             \
                i++;                                    \
                                                                        }                                           \
                                }

#endif

#define STRNCPY(pszDst, pszSrc, iLen)                   \
    do                                                  \
            {                                                   \
        strncpy(pszDst, pszSrc, (iLen)-1);              \
        pszDst[(iLen)-1] = 0;                           \
            }                                                   \
                        while(0)

#define MEMCPY(d, s, size, min) {                       \
    int i;                                              \
    if( (size) <= (min) )                               \
            {                                                   \
        for(i=0; i<(size); i++)                         \
                        {                                               \
            (d)[0]  = (s)[0];                           \
            (d)++; (s)++;                               \
                        }                                               \
            }                                                   \
                else                                                \
                {                                                   \
        memcpy(d, s, size);                             \
        (s) +=    (size);                               \
        (d) +=    (size);                               \
                }                                                   \
                                    }
#endif