#ifndef __NETWORK_COMMON_DATETIME_H__
#define __NETWORK_COMMON_DATETIME_H__
#include "common_base.h"
namespace NetWork
{
#if AURORA_OS_WINDOWS == aurora_OS
int gettimeofday(struct timeval *a_pstTv, struct timezone *a_pstTz)
{
    if (a_pstTv)
    {
        LARGE_INTEGER liTime, liFreq;
        QueryPerformanceFrequency(&liFreq);
        QueryPerformanceCounter(&liTime);
        a_pstTv->tv_sec = (long)(liTime.QuadPart / liFreq.QuadPart);
        a_pstTv->tv_usec = (long)(liTime.QuadPart * 1000000.0 / liFreq.QuadPart - a_pstTv->tv_sec * 1000000.0);
    }
    return 0;
}
#endif
}
#endif