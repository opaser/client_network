#ifndef __AURORA_CONFIG_H
#define __AURORA_CONFIG_H
#define aurora_version 1.0
#define AURORA_NAMESPACE NetWork
#define AURORA_BEGIN_NAMESPACE namespace NetWork {
#define AURORA_END_NAMESPACE }
#define USING_AURORA_NAMESPACE using namespace AURORA_NAMESPACE

// define supported target platform macro which CC uses.
#define AURORA_OS_UNKNOWN            0
#define AURORA_OS_IOS                1
#define AURORA_OS_ANDROID            2
#define AURORA_OS_WINDOWS            3
#define AURORA_OS_MARMALADE          4
#define AURORA_OS_LINUX              5
#define AURORA_OS_BADA               6
#define AURORA_OS_BLACKBERRY         7
#define AURORA_OS_MAC                8
#define AURORA_OS_NACL               9
#define AURORA_OS_EMSCRIPTEN        10
#define AURORA_OS_TIZEN             11
#define AURORA_OS_WINRT             12
#define AURORA_OS_WP8               13
// Determine target platform by compile environment macro.
#define aurora_OS             AURORA_OS_UNKNOWN

// mac
#if defined(CC_TARGET_OS_MAC)
#undef  aurora_OS
#define aurora_OS         AURORA_OS_MAC
#endif

// iphone
#if defined(CC_TARGET_OS_IPHONE)
#undef  aurora_OS
#define aurora_OS         AURORA_OS_IOS
#endif

// android
#if defined(ANDROID)
#undef  aurora_OS
#define aurora_OS         AURORA_OS_ANDROID
#endif

// WinRT (Windows Store App)
#if defined(WINRT) && defined(_WINRT)
#undef  aurora_OS
#define aurora_OS          AURORA_OS_WINRT
#endif

// WP8 (Windows Phone 8 App)
#if defined(WP8) && defined(_WP8)
#undef  aurora_OS
#define aurora_OS          AURORA_OS_WP8
#endif

// win32
#if defined(_MSC_VER) || defined(WIN32)  || defined(_WIN32) || defined(__WIN32__) \
    || defined(WIN64)    || defined(_WIN64) || defined(__WIN64__)
#undef  aurora_OS
#define aurora_OS         AURORA_OS_WINDOWS
#endif

// linux
#if defined(LINUX)
#undef  aurora_OS
#define aurora_OS         AURORA_OS_LINUX
#endif

// marmalade
#if defined(MARMALADE)
#undef  aurora_OS
#define aurora_OS         AURORA_OS_MARMALADE
#endif

// bada
#if defined(SHP)
#undef  aurora_OS
#define aurora_OS         AURORA_OS_BADA
#endif

// qnx
#if defined(__QNX__)
#undef  aurora_OS
#define aurora_OS     AURORA_OS_BLACKBERRY
#endif

// native client
#if defined(__native_client__)
#undef  aurora_OS
#define aurora_OS     AURORA_OS_NACL
#endif

// Emscripten
#if defined(EMSCRIPTEN)
#undef  aurora_OS
#define aurora_OS     AURORA_OS_EMSCRIPTEN
#endif

// tizen
#if defined(TIZEN)
#undef  aurora_OS
#define aurora_OS     AURORA_OS_TIZEN
#endif

//////////////////////////////////////////////////////////////////////////
// post configure
//////////////////////////////////////////////////////////////////////////

// check user set platform
#if ! aurora_OS
#error  "Cannot recognize the target platform; are you targeting an unsupported platform?"
#endif

#if (aurora_OS == AURORA_OS_WINDOWS)
#pragma warning (disable:4127)
#endif  // AURORA_OS_WINDOWS


#endif //__AURORA_CO