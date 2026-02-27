#ifndef UTILS_MACRO_H
#define UTILS_MACRO_H

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #define RC_BIG_ENDIAN
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define RC_LITTLE_ENDIAN
#else
    #define RC_LITTLE_ENDIAN
#endif

#if defined(_MSC_VER)
    #define FORCE_INLINE __forceinline static inline
#elif defined(__clang__) || defined(__GNUC__)
    #define FORCE_INLINE __attribute__((always_inline, used)) static inline
#else
    #define FORCE_INLINE static inline
#endif

#define LOCAL static

#ifdef RC_DEBUG
    #include <stdio.h>
    #define ASSERT(code, str) \
        puts(str); \
        exit(code)
    #define ASSERTF(code, str, ...) \
        printf(str "\n", ##__VA_ARGS__); \
        exit(code)
#endif

#endif /* UTILS_MACRO_H */ 
