/* Minimal config.h for OpenAL Soft - Windows x64 static build */
#ifndef AL_CONFIG_H
#define AL_CONFIG_H

/* Define to the library version */
#define ALSOFT_VERSION "1.24.3"

/* Git version information */
#define ALSOFT_GIT_COMMIT_HASH "unknown"
#define ALSOFT_GIT_BRANCH "unknown"

/* Force alignment attribute (empty on MSVC/Windows) */
#define FORCE_ALIGN

/* Define if HRTF data is embedded in the library */
/* #undef ALSOFT_EMBED_HRTF_DATA */

/* Define if we have windows.h header */
#define HAVE_WINDOWS_H 1

/* Define if we have stat function */
#define HAVE_STAT 1

/* Define if we have C11 _Generic support */
#define HAVE_C11_GENERIC 1

/* Define if we have C11 atomic functions */
#define HAVE_C11_ATOMIC 1

/* Define if we have C11 _Alignas support */
#define HAVE_C11_ALIGNAS 1

/* Define if we have GCC's __COUNTER__ macro */
#define HAVE___COUNTER__ 1

/* Define if we have SSE CPU extensions */
#define HAVE_SSE 1
#define HAVE_SSE2 1
#define HAVE_SSE3 1
#define HAVE_SSE4_1 1

/* Define if we have cpuid.h header */
/* #undef HAVE_CPUID_H */

/* Define if we have intrin.h header (MSVC) */
#define HAVE_INTRIN_H 1

/* Define if we have CPUID intrinsic (MSVC) */
#define HAVE_CPUID_INTRINSIC 1

/* Define if we have malloc.h */
#define HAVE_MALLOC_H 1

/* Define if we have guiddef.h */
#define HAVE_GUIDDEF_H 1

/* Define if we have initguid.h */
#define HAVE_INITGUID_H 1

/* Define if we have GCC's __builtin_clz */
/* #undef HAVE_BUILTIN_CLZ */

/* Define if we have BitScanForward64 (MSVC) */
#define HAVE_BITSCANFORWARD64_INTRINSIC 1

/* Define if we have BitScanForward (MSVC) */
#define HAVE_BITSCANFORWARD_INTRINSIC 1

/* Define if we have _controlfp() */
#define HAVE__CONTROLFP 1

/* Define if we have __control87_2() */
/* #undef HAVE___CONTROL87_2 */

/* Define if we have pthread_setschedparam() */
/* #undef HAVE_PTHREAD_SETSCHEDPARAM */

/* Define if we have pthread_setname_np() */
/* #undef HAVE_PTHREAD_SETNAME_NP */

/* Define if we have pthread_set_name_np() */
/* #undef HAVE_PTHREAD_SET_NAME_NP */

/* Define SDL3 backend */
#define HAVE_SDL3 1

/* Backend priorities */
#define DEF_PLAYBACK_BACKEND "sdl3"
#define DEF_CAPTURE_BACKEND ""

/* Size of long type */
#define SIZEOF_LONG 4

/* Restrict keyword */
#define RESTRICT __restrict

/* Windows-specific definitions */
#define WIN32_LEAN_AND_MEAN 1
#define NOMINMAX 1

/* C++ standard library features */
#define HAVE_STD_FILESYSTEM 1

/* Inline keyword */
#ifndef __cplusplus
#define inline __inline
#endif

#endif /* AL_CONFIG_H */
