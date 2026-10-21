/* =====================================================================
 * u++ çalışma zamanı — SERBEST (freestanding) hedef ön dosyası
 *
 * İşletim sistemi ve libc yoktur. Bu dosya:
 *   - yapılandırmayı (UPP_SERBEST=1) ve temel türleri,
 *   - gcc'nin freestanding modda bile çağırabileceği mem* fonksiyonlarını,
 *   - çekirdeğin kullandığı str* yardımcılarını,
 *   - metin/liste/harita için yığın ayırıcıyı
 * sağlar. Ayırıcı, varsayılan olarak .bss içindeki bir alanı kullanır;
 * çekirdek daha büyük bir bölgeyi upp.cekirdek.yigin_kur(adres, boyut)
 * ile verebilir.
 * ===================================================================== */

#define UPP_WIN 0
#define UPP_SERBEST 1

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <limits.h>

typedef int8_t bayt;
typedef uint8_t ubayt;
typedef int16_t upp_s16;
typedef uint16_t upp_u16;
typedef int32_t upp_s32;
typedef uint32_t upp_u32;
typedef unsigned long long upp_u64;

/* Erken çekirdekte iş parçacığı yerel depolama yok. */
#define UPP_TLS

/* ---- mem* (dışa açık, zayıf: çekirdek kendi sürümünü verebilir) ---- */
__attribute__((weak)) void* memcpy(void* d, const void* s, size_t n) {
    unsigned char* dp = (unsigned char*)d;
    const unsigned char* sp = (const unsigned char*)s;
    while (n--) {
        *dp++ = *sp++;
    }
    return d;
}

__attribute__((weak)) void* memmove(void* d, const void* s, size_t n) {
    unsigned char* dp = (unsigned char*)d;
    const unsigned char* sp = (const unsigned char*)s;
    if (dp == sp || n == 0) {
        return d;
    }
    if (dp < sp) {
        while (n--) {
            *dp++ = *sp++;
        }
    } else {
        dp += n;
        sp += n;
        while (n--) {
            *--dp = *--sp;
        }
    }
    return d;
}

__attribute__((weak)) void* memset(void* d, int c, size_t n) {
    unsigned char* dp = (unsigned char*)d;
    while (n--) {
        *dp++ = (unsigned char)c;
    }
    return d;
}

__attribute__((weak)) int memcmp(const void* a, const void* b, size_t n) {
    const unsigned char* x = (const unsigned char*)a;
    const unsigned char* y = (const unsigned char*)b;
    for (; n; n--, x++, y++) {
        if (*x != *y) {
            return *x < *y ? -1 : 1;
        }
    }
    return 0;
}

/* ---- str* (çekirdek içi) ---- */
static size_t strlen(const char* s) {
    const char* p = s;
    while (*p) {
        p++;
    }
    return (size_t)(p - s);
}

static int strcmp(const char* a, const char* b) {
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

static int strncmp(const char* a, const char* b, size_t n) {
    for (; n; n--, a++, b++) {
        if (*a != *b) {
            return (unsigned char)*a - (unsigned char)*b;
        }
        if (!*a) {
            return 0;
        }
    }
    return 0;
}

static char* strchr(const char* s, int c) {
    for (;; s++) {
        if (*s == (char)c) {
            return (char*)s;
        }
        if (!*s) {
            return NULL;
        }
    }
}

static char* strrchr(const char* s, int c) {
    const char* son = NULL;
    for (;; s++) {
        if (*s == (char)c) {
            son = s;
        }
        if (!*s) {
            return (char*)son;
        }
    }
}

static char* strstr(const char* s, const char* a) {
    size_t n = strlen(a);
    if (n == 0) {
        return (char*)s;
    }
    for (; *s; s++) {
        if (*s == *a && strncmp(s, a, n) == 0) {
            return (char*)s;
        }
    }
    return NULL;
}

/* =====================================================================
 * Yığın ayırıcı: sınır etiketli (boundary tag) first-fit, birleştirmeli.
 * Blok: [16 bayt başlık: boyut|kullanımda][veri ...][16 bayt alt etiket: boyut]
 * Boyutlar 16'nın katı; verinin hizası 16 bayttır.
 * ===================================================================== */
#ifndef UPP_SERBEST_YIGIN
#define UPP_SERBEST_YIGIN (4u * 1024u * 1024u)
#endif

static unsigned char _upp_yigin_varsayilan[UPP_SERBEST_YIGIN] __attribute__((aligned(16)));
static unsigned char* _upp_yigin_bas = NULL;
static unsigned char* _upp_yigin_son = NULL;
static unsigned char* _upp_yigin_gezgin = NULL;
static size_t _upp_yigin_dolu = 0;

#define _UPP_YB_ETIKET 16u
#define _UPP_YB_MIN (2u * _UPP_YB_ETIKET + 16u)

static size_t _upp_yb_boy(unsigned char* b) {
    return *(size_t*)b & ~(size_t)15u;
}

static int _upp_yb_dolu(unsigned char* b) {
    return (int)(*(size_t*)b & 1u);
}

static void _upp_yb_yaz(unsigned char* b, size_t boy, int dolu) {
    *(size_t*)b = boy | (size_t)(dolu ? 1u : 0u);
    *(size_t*)(b + boy - _UPP_YB_ETIKET) = boy | (size_t)(dolu ? 1u : 0u);
}

static void _upp_yigin_bolge(void* adres, size_t boyut) {
    uintptr_t a = ((uintptr_t)adres + 15u) & ~(uintptr_t)15u;
    uintptr_t z = ((uintptr_t)adres + boyut) & ~(uintptr_t)15u;
    if (z <= a || z - a < _UPP_YB_MIN) {
        return;
    }
    _upp_yigin_bas = (unsigned char*)a;
    _upp_yigin_son = (unsigned char*)z;
    _upp_yigin_gezgin = _upp_yigin_bas;
    _upp_yb_yaz(_upp_yigin_bas, (size_t)(z - a), 0);
}

static void _upp_yigin_hazir(void) {
    if (!_upp_yigin_bas) {
        _upp_yigin_bolge(_upp_yigin_varsayilan, sizeof(_upp_yigin_varsayilan));
    }
}

static void* upp_serbest_ayir(size_t n) {
    size_t gerek;
    unsigned char* b;
    int tur;
    _upp_yigin_hazir();
    if (n > (size_t)(_upp_yigin_son - _upp_yigin_bas)) {
        return NULL;
    }
    gerek = (n + 15u) & ~(size_t)15u;
    gerek += 2u * _UPP_YB_ETIKET;
    if (gerek < _UPP_YB_MIN) {
        gerek = _UPP_YB_MIN;
    }
    b = _upp_yigin_gezgin;
    for (tur = 0; tur < 2; tur++) {
        unsigned char* bitis = tur == 0 ? _upp_yigin_son : _upp_yigin_gezgin;
        if (tur == 1) {
            b = _upp_yigin_bas;
        }
        while (b < bitis) {
            size_t boy = _upp_yb_boy(b);
            if (!_upp_yb_dolu(b) && boy >= gerek) {
                if (boy - gerek >= _UPP_YB_MIN) {
                    _upp_yb_yaz(b + gerek, boy - gerek, 0);
                    boy = gerek;
                }
                _upp_yb_yaz(b, boy, 1);
                _upp_yigin_dolu += boy;
                _upp_yigin_gezgin = b + boy < _upp_yigin_son ? b + boy : _upp_yigin_bas;
                return b + _UPP_YB_ETIKET;
            }
            b += boy;
        }
    }
    return NULL;
}

static void upp_serbest_birak(void* p) {
    unsigned char* b;
    size_t boy;
    if (!p) {
        return;
    }
    b = (unsigned char*)p - _UPP_YB_ETIKET;
    boy = _upp_yb_boy(b);
    _upp_yigin_dolu -= boy;
    /* sonraki boşsa birleştir */
    if (b + boy < _upp_yigin_son && !_upp_yb_dolu(b + boy)) {
        if (_upp_yigin_gezgin == b + boy) {
            _upp_yigin_gezgin = b;
        }
        boy += _upp_yb_boy(b + boy);
    }
    /* önceki boşsa birleştir */
    if (b > _upp_yigin_bas) {
        size_t onceki = *(size_t*)(b - _UPP_YB_ETIKET);
        if (!(onceki & 1u)) {
            onceki &= ~(size_t)15u;
            if (_upp_yigin_gezgin == b) {
                _upp_yigin_gezgin = b - onceki;
            }
            b -= onceki;
            boy += onceki;
        }
    }
    _upp_yb_yaz(b, boy, 0);
}

static void* upp_serbest_yeniden(void* p, size_t n) {
    size_t eski;
    void* q;
    if (!p) {
        return upp_serbest_ayir(n);
    }
    eski = _upp_yb_boy((unsigned char*)p - _UPP_YB_ETIKET) - 2u * _UPP_YB_ETIKET;
    if (n <= eski) {
        return p;
    }
    q = upp_serbest_ayir(n);
    if (!q) {
        return NULL;
    }
    memcpy(q, p, eski);
    upp_serbest_birak(p);
    return q;
}

#define UPP_AYIR(n) upp_serbest_ayir(n)
#define UPP_YENIDEN(p, n) upp_serbest_yeniden((p), (n))
#define UPP_BIRAK(p) upp_serbest_birak(p)
