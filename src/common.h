#ifndef vessel_common_h
#define vessel_common_h

#ifndef OPT_RANDOM
    #define OPT_RANDOM 1
#endif
#ifndef OPT_MATH
    #define OPT_MATH 1
#endif

//#define NAN_BOXING
#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION
#define DEBUG_STRESS_GC
#define DEBUG_LOG_GC
#define UINT8_COUNT (UINT8_MAX + 1)

//#define DEBUG_PRINT_STACK
//#define DEBUG_PRINT_OPCODE

#define MAX_MODULE_VARS 65536

#define MAX_PARAMETERS 16
#define MAX_METHOD_NAME 64
#define MAX_METHOD_SIGNATURE (MAX_METHOD_NAME + (MAX_PARAMETERS * 2) + 6)

// This is used to clearly mark flexible-sized arrays that appear at the end of
// some dynamically-allocated structs, known as the "struct hack".
#if __STDC_VERSION__ >= 199901L
    // In C99, a flexible array member is just "[]".
    #define FLEXIBLE_ARRAY
#else
    // Elsewhere, use a zero-sized array. It's technically undefined behavior,
    // but works reliably in most known compilers.
    #define FLEXIBLE_ARRAY 0
#endif

#ifdef DEBUG

#include <stdlib.h>
#include <stdio.h>

#define ASSERT(condition, message)                                       \
do                                                                       \
{                                                                        \
    if (!(condition))                                                    \
    {                                                                    \
        fprintf(stderr, "[%s:%d] Assert failed in %s(): %s\n",           \
            __FILE__, __LINE__, __func__, message);                      \
        abort();                                                         \
    }                                                                    \
} while (false)

#define UNREACHABLE()                                                    \
do                                                                       \
{                                                                        \
    fprintf(stderr, "[%s:%d] This code should not be reached in %s()\n", \
    __FILE__, __LINE__, __func__);                                       \
    abort();                                                             \
} while (false)

#else

#define ASSERT(condition, message) do { } while (false)

#if defined( _MSC_VER )
#define UNREACHABLE() __assume(0)
#elif (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5))
#define UNREACHABLE() __builtin_unreachable()
#else
#define UNREACHABLE()
#endif

#endif

#endif // vessel_common_h

#undef DEBUG_PRINT_CODE
#undef DEBUG_TRACE_EXECUTION
#undef DEBUG_STRESS_GC
#undef DEBUG_LOG_GC
