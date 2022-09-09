#pragma once

#undef _WITE_PLATFORM_OS_WINDOWS
#undef _WITE_PLATFORM_OS_LINUX  
#undef _WITE_PLATFORM_OS_UNIX   
#undef _WITE_PLATFORM_OS_APPLE  
#undef _WITE_PLATFORM_OS_POSIX  

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define _WITE_PLATFORM_OS_WINDOWS
#elif __APPLE__
#define _WITE_PLATFORM_OS_APPLE
#elif __linux__
#define _WITE_PLATFORM_OS_LINUX
#elif __unix__  // all unices not caught above
#define _WITE_PLATFORM_OS_UNIX
#elif defined(_POSIX_VERSION)
#define _WITE_PLATFORM_OS_POSIX
#endif
