#ifndef __COMMON_EXPORT_H__
#define __COMMON_EXPORT_H__

#if defined(COMMON_STATIC)
  #define COMMON_API
#elif defined(COMMON_DYNAMIC)
  #ifdef COMMON_BUILD
    #if defined(_WIN32) || defined(_WIN64)
      #define COMMON_API        __declspec(dllexport)
    #else
      #define COMMON_API        __attribute__((visibility("default")))
    #endif
  #else
    #if defined(_WIN32) || defined(_WIN64)
      #define COMMON_API        __declspec(dllimport)
    #else
      #define COMMON_API
    #endif
  #endif
#else
  #define COMMON_API
#endif

#endif /* __COMMON_EXPORT_H__ */