/* =====================================================================
 * u++ çalışma zamanı — barındırılan hedef ön dosyası (yapılandırma + libc)
 *
 * UPP_HEDEF_LINUX / UPP_HEDEF_WINDOWS derleyici (uppc) tarafından basılır.
 * Host makro _WIN32 hedefi yansıtmaz: MinGW ile --linux derlenince
 * Windows API'leri windows.h olmadan çağrılıyordu.
 * ===================================================================== */

#if defined(UPP_HEDEF_LINUX)
#define UPP_WIN 0
#elif defined(UPP_HEDEF_WINDOWS)
#define UPP_WIN 1
#elif defined(_WIN32)
#define UPP_WIN 1
#else
#define UPP_WIN 0
#endif

#if !UPP_WIN
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif
#endif

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
typedef int8_t bayt;
typedef uint8_t ubayt;
typedef int16_t upp_s16;
typedef uint16_t upp_u16;
typedef int32_t upp_s32;
typedef uint32_t upp_u32;
typedef unsigned long long upp_u64;

#if defined(_MSC_VER)
#define UPP_TLS __declspec(thread)
#else
#define UPP_TLS __thread
#endif

#include <errno.h>
#include <string.h>
#include <math.h>
#if UPP_WIN
#include <windows.h>
#include <mmsystem.h>
#include <tlhelp32.h>
#include <io.h>
#include <sys/stat.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>
#if defined(__has_include)
#  if __has_include(<sys/wait.h>)
#    include <sys/wait.h>
#  endif
#else
#  include <sys/wait.h>
#endif
#endif


#define UPP_SERBEST 0
#define UPP_AYIR(n) malloc(n)
#define UPP_YENIDEN(p, n) realloc((p), (n))
#define UPP_BIRAK(p) free(p)
