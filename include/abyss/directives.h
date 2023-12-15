#ifndef ABYSS_DIRECTIVES_H_
#define ABYSS_DIRECTIVES_H_

#if defined(_MSC_VER)
#define ABYSS_INLINE __forceinline
#elif defined(__GNUC__)
#define ABYSS_INLINE __attribute__((always_inline)) inline
#else
#define ABYSS_INLINE inline
#endif

#if defined(_MSC_VER)
#define ABYSS_NOINLINE __declspec(noinline)
#elif defined(__GNUC__)
#define ABYSS_NOINLINE __attribute__((noinline))
#else
#define ABYSS_NOINLINE
#endif

// Prevent the compiler from reordering memory operations.
#define ABYSS_MFENCE asm volatile("# ABYSS_MFENCE" ::: "memory")

// Signal to the compiler that the condition is likely to be true or false.
#define ABYSS_LIKELY(x) __builtin_expect(!!(x), 1L)
#define ABYSS_UNLIKELY(x) __builtin_expect(!!(x), 0L)

#define ABYSS_FALLTHROUGH [[fallthrough]]

#define ABYSS_TODO asm volatile("int $3")

#endif // ABYSS_DIRECTIVES_H_
