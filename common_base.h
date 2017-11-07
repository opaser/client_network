#ifndef __AURORA_COMMON_BASE_H
#define __AURORA_COMMON_BASE_H
#include "config.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <math.h>
#include <iostream>
typedef signed   char int8_t;
typedef unsigned char uint8_t;
typedef signed   short int16_t;
typedef unsigned short uint16_t;
typedef signed   int int32_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;
typedef long long int int64_t;
#if AURORA_OS_ANDROID == aurora_OS
#include <signal.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <dirent.h>
#include <dlfcn.h>
#include <netinet/tcp.h>
#include <netdb.h>
#elif AURORA_OS_WINDOWS == aurora_OS
#include <WinSock2.h>
#include <WS2tcpip.h>
#elif AURORA_OS_LINUX == aurora_OS

#include <signal.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <dirent.h>
#include <dlfcn.h>
#include <netinet/tcp.h>
#include <netdb.h>

#elif AURORA_OS_IOS == aurora_OS
#include <signal.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
//#include <linux/sockios.h>
#include <net/if.h>
#include <dirent.h>
#include <dlfcn.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <string>

#endif
AURORA_BEGIN_NAMESPACE
#if aurora_OS == AURORA_OS_WINDOWS && defined(_MSC_VER)

#if !defined(snprintf)
#define snprintf sprintf_s
//int snprintf(char *buffer, size_t count, const char *format , ...);
#endif

#define THREAD_LOCAL  __declspec(thread)

//size_t strncpy(char* pszDesBuffer, const char* pszSourceBuffer, size_t iMaxSize);

#else
#if aurora_OS == AURORA_OS_IOS
#define THREAD_LOCAL
#else
#define THREAD_LOCAL  thread_local
#endif

#define LOWORD(l)           ((uint16_t)(((uint32_t)(l)) & 0xffff))
#define HIWORD(l)           ((uint16_t)((((uint32_t)(l)) >> 16) & 0xffff))
#define LOBYTE(w)           ((uint8_t)(((uint32_t)(w)) & 0xff))
#define HIBYTE(w)           ((uint8_t)((((uint32_t)(w)) >> 8) & 0xff))

#endif

#define SafeDelete(p) if (p) {delete (p); p = NULL;}
AURORA_END_NAMESPACE
#endif