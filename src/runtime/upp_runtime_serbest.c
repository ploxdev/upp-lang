/* OTOMATİK ÜRETİLDİ (uppc kendini-derle) — düzenlemeyin.
 * Kaynak: upp_on_serbest.c + upp_cekirdek.c + upp_serbest.c */
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
/* =====================================================================
 * u++ çalışma zamanı — ÇEKİRDEK (libc'siz, freestanding uyumlu)
 *
 * Bu dosya işletim sistemi varsaymaz: yalnızca <stdint.h>, <stdbool.h>,
 * <stddef.h>, <limits.h> ve memcpy/strlen gibi yardımcılar (barındırılan hedefte
 * libc, serbest hedefte upp_on_serbest.c) kullanır. Bellek UPP_AYIR /
 * UPP_YENIDEN / UPP_BIRAK üzerinden gelir.
 * Önünde bir "on" dosyası (upp_on_barindirilan.c / upp_on_serbest.c),
 * arkasında platform katmanı (upp_barindirilan.c / upp_serbest.c) durur.
 * ===================================================================== */

/* ---- platform sözleşmesi: barındırılan ya da serbest katman sağlar ---- */
static void upp_panik(const char* mesaj);
static void _upp_akis_bayt(int akis, const char* p, size_t n, int nl);
/* v'yi b'ye yazar ("%g" ya da hassas17 ise "%.17g" benzeri); yazılan bayt sayısı. */
static size_t _upp_ondalik_yaz(char* b, size_t cap, double v, int hassas17);
/* çekirdek içi ileri bildirimler */
static char* upp_mk_bas(void);
static void upp_mk_sayi(char** s, long long v);
static void _upp_m_ekle_n(char** s, const char* x, size_t nb, int x_ascii);

static long long upp_dizi_idx(long long i, long long n) {
    if (i < 0 || i >= n) {
        char* m = upp_mk_bas();
        _upp_m_ekle_n(&m, "indeks ", 7u, 1);
        upp_mk_sayi(&m, i);
        _upp_m_ekle_n(&m, ", dizi boyutu ", 14u, 1);
        upp_mk_sayi(&m, n);
        _upp_m_ekle_n(&m, " sınırını aşıyor", strlen(" sınırını aşıyor"), 0);
        upp_panik(m);
    }
    return i;
}

/* =====================================================================
 * metin v2 — yönetilen UTF-8 metin
 *
 * Her u++ metni, verinin hemen önünde duran bir başlık taşır. Veri yine
 * NUL ile biten sıradan bir char* olarak kalır; C ile birlikte çalışır.
 *   - ref: atomik referans sayısı. Kopya O(1), son bırakan belleği verir.
 *   - nbayt/kap: uzunluk O(1), += yerinde büyür (ref == 1 iken).
 *   - ncp/off: kod noktası sayısı ve cp→bayt tablosu tembel ve metne özel;
 *     küresel önbellek yok, metinler arasında gidip gelmek ucuz.
 *   - sihir: başlığın son alanı, verinin hemen önündeki 4 bayt. Bu yüzden
 *     yönetilmeyen bir işaretçinin (C sabiti, c_kod) yalnızca 4 baytı okunur
 *     ve böyle bir işaretçi asla serbest bırakılmaz.
 * ===================================================================== */
#define UPP_METIN_SIHIR 0xF5C3A71Du
#define UPP_MB_ASCII 1u
#define UPP_MB_SABIT 2u
#define UPP_MB_ASCII_BILINIYOR 4u

typedef struct UppMetinBas {
    long long ref;
    size_t nbayt;
    size_t kap;
    long long ncp;
    size_t* off;
    uint32_t bayrak;
    uint32_t sihir;
} UppMetinBas;

static void upp_panik(const char* mesaj);

static UppMetinBas* _upp_mb(const char* s) {
    uint32_t sh;
    if (!s) {
        return NULL;
    }
    memcpy(&sh, s - sizeof(uint32_t), sizeof(uint32_t));
    if (sh != UPP_METIN_SIHIR) {
        return NULL;
    }
    return (UppMetinBas*)(void*)(s - sizeof(UppMetinBas));
}

/* Runtime içindeki C sabitleri için ölümsüz yönetilen metin. */
#define UPP_LIT(s) (__extension__ ({ \
    static struct { UppMetinBas b; char d[sizeof(s)]; } _upp_lit = \
        { { 0, sizeof(s) - 1u, sizeof(s), -1, NULL, UPP_MB_SABIT, UPP_METIN_SIHIR }, s }; \
    _upp_lit.d; }))

static char* _upp_m_yeni_kap(size_t n, size_t kap) {
    UppMetinBas* b;
    char* s;
    if (kap < n + 1u) {
        kap = n + 1u;
    }
    if (kap > SIZE_MAX - sizeof(UppMetinBas)) {
        upp_panik("metin bellek yetersiz");
    }
    b = (UppMetinBas*)UPP_AYIR(sizeof(UppMetinBas) + kap);
    if (!b) {
        upp_panik("metin bellek yetersiz");
    }
    b->ref = 1;
    b->nbayt = n;
    b->kap = kap;
    b->ncp = -1;
    b->off = NULL;
    b->bayrak = 0;
    b->sihir = UPP_METIN_SIHIR;
    s = (char*)(b + 1);
    s[n] = '\0';
    return s;
}

/* n bayt + NUL; içeriği çağıran doldurur. */
static char* _upp_m_yeni(size_t n) {
    return _upp_m_yeni_kap(n, n + 1u);
}

/* Doldurma sonrası gerçek uzunluğu kaydet (n <= kap - 1). */
static char* _upp_m_boy(char* s, size_t n) {
    UppMetinBas* b = _upp_mb(s);
    if (!b) {
        return s;
    }
    if (n + 1u > b->kap) {
        upp_panik("metin boyu kapasiteyi aşıyor");
    }
    b->nbayt = n;
    b->ncp = -1;
    b->bayrak &= ~(UPP_MB_ASCII | UPP_MB_ASCII_BILINIYOR);
    if (b->off) {
        UPP_BIRAK(b->off);
        b->off = NULL;
    }
    s[n] = '\0';
    return s;
}

static char* _upp_m_bayttan(const char* p, size_t n) {
    char* s = _upp_m_yeni(n);
    if (n && p) {
        memcpy(s, p, n);
    }
    return s;
}

/* Ham C tamponundan (yığın dizisi, getenv, argv) yönetilen kopya; önünü okumaz. */
static char* _upp_m_c(const char* s) {
    if (!s) {
        return NULL;
    }
    return _upp_m_bayttan(s, strlen(s));
}

static size_t upp_metin_nbayt(const char* s) {
    UppMetinBas* b = _upp_mb(s);
    if (b) {
        return b->nbayt;
    }
    return s ? strlen(s) : 0u;
}

/* malloc ile üretilmiş ham tamponu yönetilen metne çevirip ham tamponu bırakır. */
static char* _upp_m_sahiplen(char* ham) {
    char* s;
    if (!ham) {
        return NULL;
    }
    s = _upp_m_bayttan(ham, strlen(ham));
    UPP_BIRAK(ham);
    return s;
}

static char* upp_metin_kopya(const char* s) {
    UppMetinBas* b;
    if (!s) {
        return NULL;
    }
    b = _upp_mb(s);
    if (b) {
        if (!(b->bayrak & UPP_MB_SABIT)) {
            __atomic_add_fetch(&b->ref, 1, __ATOMIC_RELAXED);
        }
        return (char*)s;
    }
    return _upp_m_bayttan(s, strlen(s));
}

/* Paylaşımsız, bağımsız kopya (iş parçacığına aktarım için). */
static char* upp_metin_derin(const char* s) {
    if (!s) {
        return NULL;
    }
    return _upp_m_bayttan(s, upp_metin_nbayt(s));
}

static void upp_metin_release(char* s) {
    UppMetinBas* b = _upp_mb(s);
    if (!b || (b->bayrak & UPP_MB_SABIT)) {
        return;
    }
    if (__atomic_sub_fetch(&b->ref, 1, __ATOMIC_ACQ_REL) == 0) {
        b->sihir = 0;
        if (b->off) {
            UPP_BIRAK(b->off);
        }
        UPP_BIRAK(b);
    }
}

static long long _upp_utf8_adv(const char* s, size_t n, size_t i) {
    unsigned char c;
    if (i >= n) {
        return 0;
    }
    c = (unsigned char)s[i];
    if (c < 0x80) {
        return 1;
    }
    if ((c & 0xE0) == 0xC0 && i + 1 < n) {
        return 2;
    }
    if ((c & 0xF0) == 0xE0 && i + 2 < n) {
        return 3;
    }
    if ((c & 0xF8) == 0xF0 && i + 3 < n) {
        return 4;
    }
    return 1;
}

static long long _upp_utf8_say(const char* s, size_t n) {
    size_t i = 0;
    long long k = 0;
    while (i < n) {
        long long a = _upp_utf8_adv(s, n, i);
        if (a <= 0) {
            break;
        }
        i += (size_t)a;
        k++;
    }
    return k;
}

static size_t _upp_utf8_bayt(const char* s, size_t n, long long kac) {
    size_t i = 0;
    long long k = 0;
    if (kac <= 0) {
        return 0;
    }
    while (i < n && k < kac) {
        long long a = _upp_utf8_adv(s, n, i);
        if (a <= 0) {
            break;
        }
        i += (size_t)a;
        k++;
    }
    return i;
}

static long long _upp_m_ncp(const char* s) {
    UppMetinBas* b = _upp_mb(s);
    long long k;
    if (!b) {
        return s ? _upp_utf8_say(s, strlen(s)) : 0;
    }
    k = __atomic_load_n(&b->ncp, __ATOMIC_RELAXED);
    if (k >= 0) {
        return k;
    }
    k = _upp_utf8_say(s, b->nbayt);
    if ((size_t)k == b->nbayt) {
        __atomic_or_fetch(&b->bayrak, UPP_MB_ASCII | UPP_MB_ASCII_BILINIYOR, __ATOMIC_RELAXED);
    } else {
        __atomic_or_fetch(&b->bayrak, UPP_MB_ASCII_BILINIYOR, __ATOMIC_RELAXED);
    }
    __atomic_store_n(&b->ncp, k, __ATOMIC_RELAXED);
    return k;
}

/* cp indeksinin bayt ofseti; aralık dışı değerler [0, nbayt]'a sıkıştırılır. */
static size_t _upp_m_cpbayt(const char* s, long long cp) {
    UppMetinBas* b = _upp_mb(s);
    long long ncp;
    size_t* off;
    if (cp <= 0 || !s) {
        return 0;
    }
    if (!b) {
        return _upp_utf8_bayt(s, strlen(s), cp);
    }
    ncp = _upp_m_ncp(s);
    if (cp >= ncp) {
        return b->nbayt;
    }
    if ((size_t)ncp == b->nbayt) {
        return (size_t)cp;
    }
    off = __atomic_load_n(&b->off, __ATOMIC_ACQUIRE);
    if (!off) {
        size_t i = 0;
        long long k = 0;
        size_t* yeni = (size_t*)UPP_AYIR(((size_t)ncp + 1u) * sizeof(size_t));
        size_t* bekl = NULL;
        if (!yeni) {
            return _upp_utf8_bayt(s, b->nbayt, cp);
        }
        while (k < ncp) {
            yeni[k] = i;
            i += (size_t)_upp_utf8_adv(s, b->nbayt, i);
            k++;
        }
        yeni[ncp] = b->nbayt;
        if (__atomic_compare_exchange_n(&b->off, &bekl, yeni, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
            off = yeni;
        } else {
            UPP_BIRAK(yeni);
            off = bekl;
        }
    }
    return off[cp];
}

static int _upp_m_ascii_mi(const char* s) {
    UppMetinBas* b = _upp_mb(s);
    if (!b) {
        return 0;
    }
    if (!(__atomic_load_n(&b->bayrak, __ATOMIC_RELAXED) & UPP_MB_ASCII_BILINIYOR)) {
        (void)_upp_m_ncp(s);
    }
    return (b->bayrak & UPP_MB_ASCII) != 0;
}

static long long upp_metinden_sayiya(const char* s) {
    unsigned long long v = 0;
    int eksi = 0;
    if (!s) {
        return 0;
    }
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') {
        s++;
    }
    if (*s == '-' || *s == '+') {
        eksi = *s == '-';
        s++;
    }
    while (*s >= '0' && *s <= '9') {
        v = v * 10u + (unsigned long long)(*s - '0');
        s++;
    }
    return eksi ? (long long)(0ull - v) : (long long)v;
}
static char* upp_sayidan_metin(long long n) {
    char* s = _upp_m_yeni_kap(0, 24);
    upp_mk_sayi(&s, n);
    return s;
}
static long long _upp_utf8_cp_at(const char* s, size_t n, size_t i) {
    unsigned char c;
    if (!s || i >= n) {
        return 0;
    }
    c = (unsigned char)s[i];
    if (c < 0x80) {
        return (long long)c;
    }
    if ((c & 0xE0) == 0xC0 && i + 1 < n) {
        return ((long long)(c & 0x1F) << 6) | ((unsigned char)s[i + 1] & 0x3F);
    }
    if ((c & 0xF0) == 0xE0 && i + 2 < n) {
        return ((long long)(c & 0x0F) << 12)
            | ((long long)((unsigned char)s[i + 1] & 0x3F) << 6)
            | ((unsigned char)s[i + 2] & 0x3F);
    }
    if ((c & 0xF8) == 0xF0 && i + 3 < n) {
        return ((long long)(c & 0x07) << 18)
            | ((long long)((unsigned char)s[i + 1] & 0x3F) << 12)
            | ((long long)((unsigned char)s[i + 2] & 0x3F) << 6)
            | ((unsigned char)s[i + 3] & 0x3F);
    }
    return (long long)c;
}

static long long upp_metin_kod(const char* s, long long i) {
    size_t b;
    size_t n;
    if (i < 0 || !s) {
        return 0;
    }
    n = upp_metin_nbayt(s);
    if (_upp_m_ascii_mi(s)) {
        return (size_t)i < n ? (long long)(unsigned char)s[i] : 0;
    }
    if (i >= _upp_m_ncp(s)) {
        return 0;
    }
    b = _upp_m_cpbayt(s, i);
    return _upp_utf8_cp_at(s, n, b);
}

static long long upp_metin_ileri(const char* s, long long i) {
    size_t n;
    if (i < 0 || !s) {
        return 0;
    }
    n = upp_metin_nbayt(s);
    if (i >= _upp_m_ncp(s)) {
        return 0;
    }
    return _upp_utf8_adv(s, n, _upp_m_cpbayt(s, i));
}

static long long upp_uzunluk(const char* s) {
    return _upp_m_ncp(s);
}

static long long upp_min(long long a, long long b) {
    return a < b ? a : b;
}

static long long upp_max(long long a, long long b) {
    return a > b ? a : b;
}

static long long upp_mutlak(long long a) {
    unsigned long long u;
    if (a >= 0) {
        return a;
    }
    /* LLONG_MIN için -a signed taşması (UB) olmasın diye imzasız yol. */
    u = 0ull - (unsigned long long)a;
    if (u > (unsigned long long)LLONG_MAX) {
        return LLONG_MAX;
    }
    return (long long)u;
}

static char* upp_metin_birlestir(const char* a, const char* b) {
    size_t na = a ? upp_metin_nbayt(a) : 0u;
    size_t nb = b ? upp_metin_nbayt(b) : 0u;
    char* p;
    if (nb > SIZE_MAX - na - 1u) {
        upp_panik("metin birleştirme taşması");
    }
    p = _upp_m_yeni(na + nb);
    if (na) {
        memcpy(p, a, na);
    }
    if (nb) {
        memcpy(p + na, b, nb);
    }
    return p;
}

static int upp_metin_esit(const char* a, const char* b) {
    size_t na;
    if (a == b) {
        return 1;
    }
    if (!a || !b) {
        return 0;
    }
    na = upp_metin_nbayt(a);
    if (na != upp_metin_nbayt(b)) {
        return 0;
    }
    return memcmp(a, b, na) == 0;
}

static void upp_metin_ata(char** dst, const char* src) {
    char* yeni;
    if (!dst) {
        return;
    }
    if (*dst == src) {
        return;
    }
    yeni = src ? upp_metin_kopya(src) : NULL;
    upp_metin_release(*dst);
    *dst = yeni;
}

static void upp_metin_tasi(char** dst, char** src) {
    char* yeni;
    if (!dst) {
        return;
    }
    if (!src) {
        upp_metin_release(*dst);
        *dst = NULL;
        return;
    }
    if (dst == src) {
        return;
    }
    yeni = *src;
    *src = NULL;
    if (yeni && !_upp_mb(yeni)) {
        yeni = _upp_m_bayttan(yeni, strlen(yeni));
    }
    if (*dst == yeni) {
        upp_metin_release(yeni);
        return;
    }
    upp_metin_release(*dst);
    *dst = yeni;
}

typedef struct {
    const char* veri;
    long long uzunluk; /* UTF-8 bayt sayısı, kod noktası değil */
} upp_ref_metin;

typedef struct {
    const long long* veri;
    long long uzunluk;
} upp_ref_dilim_sayi;

typedef struct {
    const int8_t* veri;
    long long uzunluk;
} upp_ref_dilim_bayt;

typedef struct {
    const uint8_t* veri;
    long long uzunluk;
} upp_ref_dilim_ubayt;

typedef struct {
    const double* veri;
    long long uzunluk;
} upp_ref_dilim_ondalik;

typedef struct {
    const bool* veri;
    long long uzunluk;
} upp_ref_dilim_mantik;

static upp_ref_metin upp_metin_gorunum(const char* p) {
    upp_ref_metin v;
    v.veri = p ? p : "";
    v.uzunluk = p ? (long long)upp_metin_nbayt(p) : 0;
    return v;
}

#define UPP_AKIS_CIKTI 1
#define UPP_AKIS_HATA 2

static void upp_yaz_metin(const char* s, int nl) {
    _upp_akis_bayt(UPP_AKIS_CIKTI, s, s ? upp_metin_nbayt(s) : 0u, nl);
}

static void upp_hata_yaz_metin(const char* s, int nl) {
    _upp_akis_bayt(UPP_AKIS_HATA, s, s ? upp_metin_nbayt(s) : 0u, nl);
}

static void upp_yaz_ref(upp_ref_metin v, int nl, int akis) {
    _upp_akis_bayt(akis, v.veri, v.uzunluk > 0 ? (size_t)v.uzunluk : 0u, nl);
}

/* *s sonuna x'in nb baytını ekler. x_ascii: x'in ASCII olduğu biliniyorsa 1. */
static void _upp_m_ekle_n(char** s, const char* x, size_t nb, int x_ascii) {
    char* a;
    UppMetinBas* ba;
    size_t na;
    size_t need;
    char* p;
    if (!s) {
        return;
    }
    a = *s;
    if (!x) {
        x = "";
        nb = 0;
    }
    na = a ? upp_metin_nbayt(a) : 0u;
    if (nb > SIZE_MAX - na - 1u) {
        upp_panik("metin ekleme taşması");
    }
    need = na + nb + 1u;
    ba = _upp_mb(a);
    if (ba && !(ba->bayrak & UPP_MB_SABIT) && __atomic_load_n(&ba->ref, __ATOMIC_ACQUIRE) == 1 && ba->kap >= need) {
        long long ex_ncp = ba->ncp;
        uint32_t ex_bayrak = ba->bayrak;
        memmove(a + na, x, nb);
        _upp_m_boy(a, na + nb);
        if (ex_ncp >= 0 && (ex_bayrak & UPP_MB_ASCII) && x_ascii) {
            ba->ncp = (long long)(na + nb);
            ba->bayrak |= UPP_MB_ASCII | UPP_MB_ASCII_BILINIYOR;
        }
        return;
    }
    p = _upp_m_yeni_kap(na + nb, need < 32u ? 64u : need * 2u);
    if (na) {
        memcpy(p, a, na);
    }
    if (nb) {
        memcpy(p + na, x, nb);
    }
    p[na + nb] = '\0';
    *s = p;
    upp_metin_release(a);
}

static void upp_metin_ekle(char** s, const char* x) {
    if (!x) {
        return;
    }
    _upp_m_ekle_n(s, x, upp_metin_nbayt(x), _upp_m_ascii_mi(x));
}

/* ---- metin kurucu: interpolasyon libc biçimlendirmesi olmadan buradan kurulur ---- */
static char* upp_mk_bas(void) {
    return _upp_m_yeni_kap(0, 64);
}

static void upp_mk_metin(char** s, const char* x) {
    upp_metin_ekle(s, x);
}

static void upp_mk_ref(char** s, upp_ref_metin v) {
    _upp_m_ekle_n(s, v.veri, v.uzunluk > 0 ? (size_t)v.uzunluk : 0u, 0);
}

static void upp_mk_usayi(char** s, unsigned long long v) {
    char b[24];
    int i = 24;
    do {
        b[--i] = (char)('0' + (int)(v % 10u));
        v /= 10u;
    } while (v);
    _upp_m_ekle_n(s, b + i, (size_t)(24 - i), 1);
}

static void upp_mk_sayi(char** s, long long v) {
    unsigned long long u;
    if (v < 0) {
        _upp_m_ekle_n(s, "-", 1u, 1);
        u = 0ull - (unsigned long long)v;
    } else {
        u = (unsigned long long)v;
    }
    upp_mk_usayi(s, u);
}

static void upp_mk_ondalik(char** s, double v) {
    char b[64];
    size_t n = _upp_ondalik_yaz(b, sizeof(b), v, 0);
    _upp_m_ekle_n(s, b, n, 1);
}
static void upp_mk_mantik(char** s, int v) {
    if (v) {
        _upp_m_ekle_n(s, "dogru", 5u, 1);
    } else {
        _upp_m_ekle_n(s, "yanlis", 6u, 1);
    }
}

static char* upp_metin_kes(const char* s, long long bas, long long uzunluk) {
    size_t start, end;
    if (!s) {
        return _upp_m_yeni(0);
    }
    if (bas < 0) bas = 0;
    if (uzunluk < 0) uzunluk = 0;
    start = _upp_m_cpbayt(s, bas);
    end = _upp_m_cpbayt(s, bas + uzunluk);
    if (end < start) {
        end = start;
    }
    return _upp_m_bayttan(s + start, end - start);
}

static int upp_metin_baslar(const char* s, const char* pref) {
    size_t ns, np;
    if (!s) s = "";
    if (!pref) pref = "";
    ns = strlen(s);
    np = strlen(pref);
    if (np > ns) return 0;
    return memcmp(s, pref, np) == 0 ? 1 : 0;
}

static int upp_metin_biter(const char* s, const char* suf) {
    size_t ns, np;
    if (!s) s = "";
    if (!suf) suf = "";
    ns = strlen(s);
    np = strlen(suf);
    if (np > ns) return 0;
    return memcmp(s + (ns - np), suf, np) == 0 ? 1 : 0;
}

static char* upp_metin_kirp(const char* s) {
    size_t n, i, j;
    char* p;
    if (!s) s = "";
    n = strlen(s);
    i = 0;
    while (i < n && (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n')) {
        i++;
    }
    j = n;
    while (j > i && (s[j - 1] == ' ' || s[j - 1] == '\t' || s[j - 1] == '\r' || s[j - 1] == '\n')) {
        j--;
    }
    (void)p;
    return _upp_m_bayttan(s + i, j - i);
}

static int _upp_utf8_yaz(long long cp, char* o) {
    if (cp < 0x80) { o[0] = (char)cp; return 1; }
    if (cp < 0x800) { o[0] = (char)(0xC0 | (cp >> 6)); o[1] = (char)(0x80 | (cp & 0x3F)); return 2; }
    if (cp < 0x10000) {
        o[0] = (char)(0xE0 | (cp >> 12)); o[1] = (char)(0x80 | ((cp >> 6) & 0x3F)); o[2] = (char)(0x80 | (cp & 0x3F));
        return 3;
    }
    o[0] = (char)(0xF0 | (cp >> 18)); o[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
    o[2] = (char)(0x80 | ((cp >> 6) & 0x3F)); o[3] = (char)(0x80 | (cp & 0x3F));
    return 4;
}

/* Türkçe kurallı harf dönüşümü: i↔İ, ı↔I, ç ğ ö ş ü ve Latin-1 harfleri. */
static long long _upp_tr_buyuk(long long c) {
    if (c == 'i') return 0x130;
    if (c == 0x131) return 'I';
    if (c >= 'a' && c <= 'z') return c - 32;
    if (c == 0x11F) return 0x11E;
    if (c == 0x15F) return 0x15E;
    if (c >= 0xE0 && c <= 0xFE && c != 0xF7) return c - 32;
    return c;
}

static long long _upp_tr_kucuk(long long c) {
    if (c == 'I') return 0x131;
    if (c == 0x130) return 'i';
    if (c >= 'A' && c <= 'Z') return c + 32;
    if (c == 0x11E) return 0x11F;
    if (c == 0x15E) return 0x15F;
    if (c >= 0xC0 && c <= 0xDE && c != 0xD7) return c + 32;
    return c;
}

static char* _upp_harf_cevir(const char* s, int buyuk) {
    size_t n, i, w;
    char* p;
    if (!s) s = UPP_LIT("");
    n = upp_metin_nbayt(s);
    p = _upp_m_yeni(n * 2u + 4u);
    i = 0;
    w = 0;
    while (i < n) {
        size_t a = (size_t)_upp_utf8_adv(s, n, i);
        long long c = _upp_utf8_cp_at(s, n, i);
        c = buyuk ? _upp_tr_buyuk(c) : _upp_tr_kucuk(c);
        w += (size_t)_upp_utf8_yaz(c, p + w);
        i += a;
    }
    return _upp_m_boy(p, w);
}

static char* upp_metin_buyuk(const char* s) {
    return _upp_harf_cevir(s, 1);
}

static char* upp_metin_kucuk(const char* s) {
    return _upp_harf_cevir(s, 0);
}

static char* upp_metin_hex_coz(const char* hex) {
    size_t len = hex ? strlen(hex) : 0;
    size_t i, j = 0;
    char* buf;
    if (len == 0) return _upp_m_c("");
    buf = (char*)UPP_AYIR(len / 2 + 1);
    if (!buf) return _upp_m_c("");
    for (i = 0; i + 1 < len; i += 2) {
        char c1 = hex[i], c2 = hex[i + 1];
        int v1 = (c1 >= '0' && c1 <= '9') ? (c1 - '0') : ((c1 >= 'a' && c1 <= 'f') ? (c1 - 'a' + 10) : ((c1 >= 'A' && c1 <= 'F') ? (c1 - 'A' + 10) : 0));
        int v2 = (c2 >= '0' && c2 <= '9') ? (c2 - '0') : ((c2 >= 'a' && c2 <= 'f') ? (c2 - 'a' + 10) : ((c2 >= 'A' && c2 <= 'F') ? (c2 - 'A' + 10) : 0));
        buf[j++] = (char)((v1 << 4) | v2);
    }
    buf[j] = 0;
    return _upp_m_sahiplen(buf);
}

static long long upp_metin_icinde(const char* s, const char* ara) {
    const char* p;
    size_t n;
    if (!s || !ara) return -1;
    p = strstr(s, ara);
    if (!p) return -1;
    n = (size_t)(p - s);
    return _upp_utf8_say(s, n);
}

/* Alt metnin ilk kod noktası indeksi; yoksa -1. */
static long long upp_metin_bul(const char* s, const char* ara) {
    return upp_metin_icinde(s, ara);
}

/* Baytları küçük harf onaltılığa çevirir ("Aç" → "41c3a7"). */
static char* upp_metin_hex_kodla(const char* s) {
    static const char h[] = "0123456789abcdef";
    size_t n = s ? upp_metin_nbayt(s) : 0u;
    size_t i;
    char* o = _upp_m_yeni(n * 2u);
    for (i = 0; i < n; i++) {
        unsigned char c = (unsigned char)s[i];
        o[2u * i] = h[c >> 4];
        o[2u * i + 1u] = h[c & 15u];
    }
    return o;
}

static char* upp_metin_degistir(const char* s, const char* eski, const char* yeni) {
    size_t ns, ne, nn, count, extra, cap;
    const char* p;
    char* out;
    char* w;
    if (!s) s = "";
    if (!eski) eski = "";
    if (!yeni) yeni = "";
    ns = strlen(s);
    ne = strlen(eski);
    nn = strlen(yeni);
    if (ne == 0) {
        out = (char*)UPP_AYIR(ns + 1u);
        if (!out) return NULL;
        memcpy(out, s, ns + 1u);
        return _upp_m_sahiplen(out);
    }
    count = 0;
    p = s;
    while ((p = strstr(p, eski)) != NULL) {
        count++;
        p += ne;
    }
    extra = (nn > ne) ? (nn - ne) : 0u;
    if (extra > 0u && count > (SIZE_MAX - ns - 1u) / extra) {
        return NULL;
    }
    cap = ns + count * extra + 1u;
    out = (char*)UPP_AYIR(cap);
    if (!out) return NULL;
    w = out;
    p = s;
    while (*p) {
        if (strncmp(p, eski, ne) == 0) {
            memcpy(w, yeni, nn);
            w += nn;
            p += ne;
        } else {
            *w++ = *p++;
        }
    }
    *w = '\0';
    return _upp_m_sahiplen(out);
}

static void* bellek_ayir(long long boyut) {
    if (boyut <= 0) {
        return NULL;
    }
    return UPP_AYIR((size_t)boyut);
}
static void bellek_bosalt(void* ptr) {
    UPP_BIRAK(ptr);
}
/* ---- yerleşik yapılar ---- */

typedef struct Yonelme {
    double pitch;
    double yaw;
} Yonelme;

typedef struct EkranNokta {
    double x;
    double y;
    long long gorunur;
} EkranNokta;

typedef struct Vektor3 {
    double x;
    double y;
    double z;
} Vektor3;

typedef struct ArkaplanIs {
    long long handle;
} ArkaplanIs;

typedef struct JSONDeger {
    long long id;
} JSONDeger;

typedef struct SurecCikti {
    char* cikti;
    long long kod;
    int basarili;
} SurecCikti;

typedef struct Kilit {
    long long id;
} Kilit;

typedef struct UppKolListe {
    long long id;
} UppKolListe;

typedef struct UppKolHarita {
    long long id;
} UppKolHarita;

#define UPP_KT_SAYI 0
#define UPP_KT_ONDALIK 1
#define UPP_KT_MANTIK 2
#define UPP_KT_METIN 3
#define UPP_KT_BAYT 4
#define UPP_KOLLEKSIYON_MAX 1000000LL

typedef struct {
    int canli;
    int tag;
    long long n;
    long long cap;
    long long* i;
    double* d;
    char** s;
} _UppKolListeH;

typedef struct {
    int canli;
    int ktag;
    int vtag;
    long long n;
    long long cap;
    long long* k_i;
    char** k_s;
    long long* v_i;
    double* v_d;
    char** v_s;
    long long* hash;
    long long hash_cap;
    long long hash_tomb;
} _UppKolHaritaH;

static double upp_mat_mutlak(double x) { return x < 0.0 ? -x : x; }
static double upp_mat_pi(void) { return 3.14159265358979323846; }
static double upp_mat_e(void) { return 2.71828182845904523536; }
static double upp_mat_sinirla(double x, double lo, double hi) {
    if (lo > hi) {
        double t = lo;
        lo = hi;
        hi = t;
    }
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static double upp_mat_yumusat(double a, double b, double t) {
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;
    return a + (b - a) * t;
}

/* ---- liste[T] / harita[K,V] (v2.7) ---- */

static _UppKolListeH* _upp_listeler = NULL;
static long long _upp_liste_n = 1;
static long long _upp_liste_cap = 0;
static _UppKolHaritaH* _upp_haritalar = NULL;
static long long _upp_harita_n = 1;
static long long _upp_harita_cap = 0;

static char* _upp_kol_kopya(const char* s) {
    return upp_metin_kopya(s);
}

static _UppKolListeH* _upp_liste_hucre(long long id) {
    if (id <= 0 || id >= _upp_liste_n || !_upp_listeler) {
        return NULL;
    }
    if (!_upp_listeler[id].canli) {
        return NULL;
    }
    return &_upp_listeler[id];
}

static long long _upp_liste_yeni(int tag) {
    long long i;
    long long nc;
    _UppKolListeH* np;
    for (i = 1; i < _upp_liste_n; i++) {
        if (!_upp_listeler[i].canli) {
            memset(&_upp_listeler[i], 0, sizeof(_UppKolListeH));
            _upp_listeler[i].canli = 1;
            _upp_listeler[i].tag = tag;
            return i;
        }
    }
    if (_upp_liste_n >= UPP_KOLLEKSIYON_MAX) {
        return 0;
    }
    if (_upp_liste_n + 1 > _upp_liste_cap) {
        nc = _upp_liste_cap ? _upp_liste_cap * 2 : 16;
        if (nc < _upp_liste_n + 1) {
            nc = _upp_liste_n + 1;
        }
        np = (_UppKolListeH*)UPP_YENIDEN(_upp_listeler, (size_t)nc * sizeof(_UppKolListeH));
        if (!np) {
            return 0;
        }
        memset(np + _upp_liste_cap, 0, (size_t)(nc - _upp_liste_cap) * sizeof(_UppKolListeH));
        _upp_listeler = np;
        _upp_liste_cap = nc;
    }
    i = _upp_liste_n++;
    memset(&_upp_listeler[i], 0, sizeof(_UppKolListeH));
    _upp_listeler[i].canli = 1;
    _upp_listeler[i].tag = tag;
    return i;
}

static int _upp_liste_hazir(UppKolListe* L, int tag) {
    if (!L) {
        return 0;
    }
    if (L->id == 0) {
        L->id = _upp_liste_yeni(tag);
        if (L->id == 0) {
            return 0;
        }
    }
    {
        _UppKolListeH* h = _upp_liste_hucre(L->id);
        if (!h || h->tag != tag) {
            return 0;
        }
        return 1;
    }
}

static int _upp_liste_buyut(_UppKolListeH* h) {
    long long nc;
    if (!h) {
        return 0;
    }
    if (h->cap > 0 && h->cap > (LLONG_MAX / 2)) {
        upp_panik("liste kapasite taşması");
    }
    nc = h->cap ? h->cap * 2 : 8;
    if (h->tag == UPP_KT_ONDALIK) {
        double* p = (double*)UPP_YENIDEN(h->d, (size_t)nc * sizeof(double));
        if (!p) {
            return 0;
        }
        h->d = p;
    } else if (h->tag == UPP_KT_METIN) {
        char** p = (char**)UPP_YENIDEN(h->s, (size_t)nc * sizeof(char*));
        if (!p) {
            return 0;
        }
        h->s = p;
    } else {
        long long* p = (long long*)UPP_YENIDEN(h->i, (size_t)nc * sizeof(long long));
        if (!p) {
            return 0;
        }
        h->i = p;
    }
    h->cap = nc;
    return 1;
}

static void upp_liste_ekle(UppKolListe* L, int tag, long long iv, double dv, const char* sv) {
    _UppKolListeH* h;
    if (!_upp_liste_hazir(L, tag)) {
        upp_panik("liste bellek yetersiz");
    }
    h = _upp_liste_hucre(L->id);
    if (!h) {
        return;
    }
    if (h->n >= h->cap && !_upp_liste_buyut(h)) {
        upp_panik("liste bellek yetersiz");
    }
    if (h->n >= h->cap) {
        upp_panik("liste bellek yetersiz");
    }
    if (tag == UPP_KT_ONDALIK) {
        h->d[h->n++] = dv;
    } else if (tag == UPP_KT_METIN) {
        h->s[h->n++] = _upp_kol_kopya(sv);
    } else if (tag == UPP_KT_BAYT) {
        h->i[h->n++] = (long long)((unsigned char)iv);
    } else {
        h->i[h->n++] = iv;
    }
}

/* Ham C tamponunu (yığın dizisi, dirent adı) önünü okumadan listeye ekler. */
static void _upp_liste_ekle_c(UppKolListe* L, const char* ham) {
    char* m = _upp_m_c(ham);
    upp_liste_ekle(L, UPP_KT_METIN, 0, 0.0, m);
    upp_metin_release(m);
}

static void upp_liste_yaz(UppKolListe* L, int tag, long long i, long long iv, double dv, const char* sv) {
    _UppKolListeH* h;
    if (!L || L->id == 0) {
        return;
    }
    h = _upp_liste_hucre(L->id);
    if (!h || h->tag != tag || i < 0 || i >= h->n) {
        upp_panik("liste indeks sınır dışı");
    }
    if (tag == UPP_KT_ONDALIK) {
        h->d[i] = dv;
    } else if (tag == UPP_KT_METIN) {
        if (h->s[i]) {
            upp_metin_release(h->s[i]);
        }
        h->s[i] = _upp_kol_kopya(sv);
    } else if (tag == UPP_KT_BAYT) {
        h->i[i] = (long long)((unsigned char)iv);
    } else {
        h->i[i] = iv;
    }
}

static long long upp_liste_al_i(UppKolListe L, long long i) {
    _UppKolListeH* h = _upp_liste_hucre(L.id);
    if (!h || i < 0 || i >= h->n) {
        upp_panik("liste indeks sınır dışı");
    }
    if (h->tag == UPP_KT_ONDALIK || h->tag == UPP_KT_METIN) {
        return 0;
    }
    if (h->tag == UPP_KT_BAYT) {
        return (long long)((unsigned char)h->i[i]);
    }
    return h->i[i];
}

static double upp_liste_al_d(UppKolListe L, long long i) {
    _UppKolListeH* h = _upp_liste_hucre(L.id);
    if (!h || h->tag != UPP_KT_ONDALIK || i < 0 || i >= h->n) {
        upp_panik("liste indeks sınır dışı");
    }
    return h->d[i];
}

static char* upp_liste_al_s(UppKolListe L, long long i) {
    _UppKolListeH* h = _upp_liste_hucre(L.id);
    if (!h || h->tag != UPP_KT_METIN || i < 0 || i >= h->n) {
        upp_panik("liste indeks sınır dışı");
    }
    return h->s[i] ? _upp_kol_kopya(h->s[i]) : NULL;
}

static long long upp_liste_uzunluk(UppKolListe L) {
    _UppKolListeH* h = _upp_liste_hucre(L.id);
    return h ? h->n : 0;
}

static void upp_dilim_sinir(long long a, long long b, long long n) {
    if (a < 0 || b < a || b > n) {
        char* m = upp_mk_bas();
        _upp_m_ekle_n(&m, "dilim [", 7u, 1);
        upp_mk_sayi(&m, a);
        _upp_m_ekle_n(&m, "..", 2u, 1);
        upp_mk_sayi(&m, b);
        _upp_m_ekle_n(&m, "], uzunluk ", 11u, 1);
        upp_mk_sayi(&m, n);
        _upp_m_ekle_n(&m, " sınır dışı", strlen(" sınır dışı"), 0);
        upp_panik(m);
    }
}

static upp_ref_dilim_sayi upp_dilim_sayi(const long long* p, long long n, long long a, long long b) {
    upp_ref_dilim_sayi r;
    upp_dilim_sinir(a, b, n);
    r.veri = p ? (p + a) : NULL;
    r.uzunluk = b - a;
    return r;
}

static upp_ref_dilim_bayt upp_dilim_bayt(const int8_t* p, long long n, long long a, long long b) {
    upp_ref_dilim_bayt r;
    upp_dilim_sinir(a, b, n);
    r.veri = p ? (p + a) : NULL;
    r.uzunluk = b - a;
    return r;
}

static upp_ref_dilim_ubayt upp_dilim_ubayt(const uint8_t* p, long long n, long long a, long long b) {
    upp_ref_dilim_ubayt r;
    upp_dilim_sinir(a, b, n);
    r.veri = p ? (p + a) : NULL;
    r.uzunluk = b - a;
    return r;
}

static upp_ref_dilim_ondalik upp_dilim_ondalik(const double* p, long long n, long long a, long long b) {
    upp_ref_dilim_ondalik r;
    upp_dilim_sinir(a, b, n);
    r.veri = p ? (p + a) : NULL;
    r.uzunluk = b - a;
    return r;
}

static upp_ref_dilim_mantik upp_dilim_mantik(const bool* p, long long n, long long a, long long b) {
    upp_ref_dilim_mantik r;
    upp_dilim_sinir(a, b, n);
    r.veri = p ? (p + a) : NULL;
    r.uzunluk = b - a;
    return r;
}

static upp_ref_dilim_sayi upp_liste_dilim_sayi(UppKolListe L, long long a, long long b) {
    _UppKolListeH* h = _upp_liste_hucre(L.id);
    if (!h) {
        return upp_dilim_sayi(NULL, 0, a, b);
    }
    if (h->tag != UPP_KT_SAYI) {
        upp_panik("liste dilim türü uyumsuz");
    }
    return upp_dilim_sayi(h->i, h->n, a, b);
}

static upp_ref_dilim_ondalik upp_liste_dilim_ondalik(UppKolListe L, long long a, long long b) {
    _UppKolListeH* h = _upp_liste_hucre(L.id);
    if (!h) {
        return upp_dilim_ondalik(NULL, 0, a, b);
    }
    if (h->tag != UPP_KT_ONDALIK) {
        upp_panik("liste dilim türü uyumsuz");
    }
    return upp_dilim_ondalik(h->d, h->n, a, b);
}

static upp_ref_dilim_sayi upp_liste_gorunum_sayi(UppKolListe L) {
    _UppKolListeH* h = _upp_liste_hucre(L.id);
    if (!h) {
        return upp_dilim_sayi(NULL, 0, 0, 0);
    }
    if (h->tag != UPP_KT_SAYI) {
        upp_panik("liste görünüm türü uyumsuz");
    }
    return upp_dilim_sayi(h->i, h->n, 0, h->n);
}

static upp_ref_dilim_ondalik upp_liste_gorunum_ondalik(UppKolListe L) {
    _UppKolListeH* h = _upp_liste_hucre(L.id);
    if (!h) {
        return upp_dilim_ondalik(NULL, 0, 0, 0);
    }
    if (h->tag != UPP_KT_ONDALIK) {
        upp_panik("liste görünüm türü uyumsuz");
    }
    return upp_dilim_ondalik(h->d, h->n, 0, h->n);
}

static void upp_liste_bosalt(UppKolListe* L) {
    _UppKolListeH* h;
    long long i;
    if (!L || L->id == 0) {
        return;
    }
    h = _upp_liste_hucre(L->id);
    if (!h) {
        L->id = 0;
        return;
    }
    if (h->tag == UPP_KT_METIN && h->s) {
        for (i = 0; i < h->n; i++) {
            upp_metin_release(h->s[i]);
        }
    }
    UPP_BIRAK(h->i);
    UPP_BIRAK(h->d);
    UPP_BIRAK(h->s);
    memset(h, 0, sizeof(_UppKolListeH));
    L->id = 0;
}

static void upp_liste_sil(UppKolListe* L, long long idx) {
    _UppKolListeH* h;
    long long j;
    if (!L || L->id == 0) return;
    h = _upp_liste_hucre(L->id);
    if (!h || idx < 0 || idx >= h->n) return;
    if (h->tag == UPP_KT_METIN) {
        if (h->s[idx]) upp_metin_release(h->s[idx]);
        for (j = idx; j < h->n - 1; j++) {
            h->s[j] = h->s[j + 1];
        }
        h->s[h->n - 1] = NULL;
    } else if (h->tag == UPP_KT_ONDALIK) {
        for (j = idx; j < h->n - 1; j++) {
            h->d[j] = h->d[j + 1];
        }
    } else {
        for (j = idx; j < h->n - 1; j++) {
            h->i[j] = h->i[j + 1];
        }
    }
    h->n--;
}

static void upp_liste_temizle(UppKolListe* L) {
    _UppKolListeH* h;
    long long j;
    if (!L || L->id == 0) return;
    h = _upp_liste_hucre(L->id);
    if (!h) return;
    if (h->tag == UPP_KT_METIN && h->s) {
        for (j = 0; j < h->n; j++) {
            if (h->s[j]) upp_metin_release(h->s[j]);
            h->s[j] = NULL;
        }
    }
    h->n = 0;
}

static _UppKolHaritaH* _upp_harita_hucre(long long id) {
    if (id <= 0 || id >= _upp_harita_n || !_upp_haritalar) {
        return NULL;
    }
    if (!_upp_haritalar[id].canli) {
        return NULL;
    }
    return &_upp_haritalar[id];
}

static long long _upp_harita_yeni(int ktag, int vtag) {
    long long i;
    long long nc;
    _UppKolHaritaH* np;
    for (i = 1; i < _upp_harita_n; i++) {
        if (!_upp_haritalar[i].canli) {
            memset(&_upp_haritalar[i], 0, sizeof(_UppKolHaritaH));
            _upp_haritalar[i].canli = 1;
            _upp_haritalar[i].ktag = ktag;
            _upp_haritalar[i].vtag = vtag;
            return i;
        }
    }
    if (_upp_harita_n >= UPP_KOLLEKSIYON_MAX) {
        return 0;
    }
    if (_upp_harita_n + 1 > _upp_harita_cap) {
        nc = _upp_harita_cap ? _upp_harita_cap * 2 : 16;
        if (nc < _upp_harita_n + 1) {
            nc = _upp_harita_n + 1;
        }
        np = (_UppKolHaritaH*)UPP_YENIDEN(_upp_haritalar, (size_t)nc * sizeof(_UppKolHaritaH));
        if (!np) {
            return 0;
        }
        memset(np + _upp_harita_cap, 0, (size_t)(nc - _upp_harita_cap) * sizeof(_UppKolHaritaH));
        _upp_haritalar = np;
        _upp_harita_cap = nc;
    }
    i = _upp_harita_n++;
    memset(&_upp_haritalar[i], 0, sizeof(_UppKolHaritaH));
    _upp_haritalar[i].canli = 1;
    _upp_haritalar[i].ktag = ktag;
    _upp_haritalar[i].vtag = vtag;
    return i;
}

static int _upp_harita_hazir(UppKolHarita* H, int ktag, int vtag) {
    if (!H) {
        return 0;
    }
    if (H->id == 0) {
        H->id = _upp_harita_yeni(ktag, vtag);
        if (H->id == 0) {
            return 0;
        }
    }
    {
        _UppKolHaritaH* h = _upp_harita_hucre(H->id);
        if (!h || h->ktag != ktag || h->vtag != vtag) {
            return 0;
        }
        return 1;
    }
}

#define UPP_H_EMPTY (-1LL)
#define UPP_H_TOMB  (-2LL)

static unsigned _upp_fnv(const char* s) {
    unsigned h = 2166136261u;
    const unsigned char* p = (const unsigned char*)(s ? s : "");
    while (*p) {
        h ^= *p++;
        h *= 16777619u;
    }
    return h ? h : 1u;
}

static unsigned _upp_mix_i(long long k) {
    unsigned long long x = (unsigned long long)k * 0x9E3779B97F4A7C15ull;
    x ^= x >> 32;
    return (unsigned)x ? (unsigned)x : 1u;
}

static unsigned _upp_hkey(_UppKolHaritaH* h, long long ki, const char* ks) {
    if (h->ktag == UPP_KT_METIN) {
        return _upp_fnv(ks);
    }
    return _upp_mix_i(ki);
}

static int _upp_hesit(_UppKolHaritaH* h, long long i, long long ki, const char* ks) {
    if (h->ktag == UPP_KT_METIN) {
        const char* a = h->k_s[i] ? h->k_s[i] : "";
        const char* b = ks ? ks : "";
        return strcmp(a, b) == 0;
    }
    return h->k_i[i] == ki;
}

static long long _upp_hprobe(_UppKolHaritaH* h, unsigned hv, long long step) {
    long long cap = h->hash_cap;
    if (cap <= 0) {
        return 0;
    }
    if ((cap & (cap - 1)) == 0) {
        return (long long)((hv + (unsigned)step) & (unsigned)(cap - 1));
    }
    return (long long)((hv + (unsigned)step) % (unsigned)cap);
}

static int _upp_harita_rehash(_UppKolHaritaH* h, long long cap) {
    long long* nh;
    long long i;
    long long n = 8;
    if (!h) {
        return 0;
    }
    if (cap < 8) {
        cap = 8;
    }
    while (n < cap) {
        if (n > (LLONG_MAX / 2)) {
            n = cap;
            break;
        }
        n *= 2;
    }
    cap = n;
    nh = (long long*)UPP_AYIR((size_t)cap * sizeof(long long));
    if (!nh) {
        return 0;
    }
    for (i = 0; i < cap; i++) {
        nh[i] = UPP_H_EMPTY;
    }
    UPP_BIRAK(h->hash);
    h->hash = nh;
    h->hash_cap = cap;
    h->hash_tomb = 0;
    for (i = 0; i < h->n; i++) {
        unsigned hv = _upp_hkey(h, h->ktag == UPP_KT_METIN ? 0 : h->k_i[i],
                               h->ktag == UPP_KT_METIN ? h->k_s[i] : NULL);
        long long step = 0;
        for (;;) {
            long long p = _upp_hprobe(h, hv, step);
            if (h->hash[p] == UPP_H_EMPTY) {
                h->hash[p] = i;
                break;
            }
            step++;
            if (step >= cap) {
                UPP_BIRAK(nh);
                h->hash = NULL;
                h->hash_cap = 0;
                return 0;
            }
        }
    }
    return 1;
}

static void _upp_harita_hash_put(_UppKolHaritaH* h, long long idx) {
    unsigned hv;
    long long step = 0;
    long long tomb = -1;
    long long cap;
    if (!h || !h->hash || h->hash_cap < 8) {
        return;
    }
    cap = h->hash_cap;
    hv = _upp_hkey(h, h->ktag == UPP_KT_METIN ? 0 : h->k_i[idx],
                   h->ktag == UPP_KT_METIN ? h->k_s[idx] : NULL);
    for (;;) {
        long long p = _upp_hprobe(h, hv, step);
        long long sl = h->hash[p];
        if (sl == UPP_H_EMPTY) {
            if (tomb >= 0) {
                h->hash[tomb] = idx;
                if (h->hash_tomb > 0) {
                    h->hash_tomb--;
                }
            } else {
                h->hash[p] = idx;
            }
            return;
        }
        if (sl == UPP_H_TOMB && tomb < 0) {
            tomb = p;
        }
        step++;
        if (step >= cap) {
            return;
        }
    }
}

static void _upp_harita_indeks_kaydet(_UppKolHaritaH* h, long long idx) {
    if (!h) {
        return;
    }
    if (!h->hash || h->hash_cap < 8 || (h->n + h->hash_tomb) * 2LL >= h->hash_cap) {
        long long nc = h->hash_cap ? h->hash_cap * 2 : 8;
        _upp_harita_rehash(h, nc);
        return;
    }
    _upp_harita_hash_put(h, idx);
}

static int _upp_harita_buyut(_UppKolHaritaH* h) {
    long long nc;
    if (!h) {
        return 0;
    }
    if (h->cap > 0 && h->cap > (LLONG_MAX / 2)) {
        upp_panik("harita kapasite taşması");
    }
    nc = h->cap ? h->cap * 2 : 8;
    if (h->ktag == UPP_KT_METIN) {
        char** p = (char**)UPP_YENIDEN(h->k_s, (size_t)nc * sizeof(char*));
        if (!p) {
            return 0;
        }
        h->k_s = p;
    } else {
        long long* p = (long long*)UPP_YENIDEN(h->k_i, (size_t)nc * sizeof(long long));
        if (!p) {
            return 0;
        }
        h->k_i = p;
    }
    if (h->vtag == UPP_KT_ONDALIK) {
        double* p = (double*)UPP_YENIDEN(h->v_d, (size_t)nc * sizeof(double));
        if (!p) {
            return 0;
        }
        h->v_d = p;
    } else if (h->vtag == UPP_KT_METIN) {
        char** p = (char**)UPP_YENIDEN(h->v_s, (size_t)nc * sizeof(char*));
        if (!p) {
            return 0;
        }
        h->v_s = p;
    } else {
        long long* p = (long long*)UPP_YENIDEN(h->v_i, (size_t)nc * sizeof(long long));
        if (!p) {
            return 0;
        }
        h->v_i = p;
    }
    h->cap = nc;
    return 1;
}

static long long _upp_harita_bul(_UppKolHaritaH* h, long long ki, const char* ks) {
    unsigned hv;
    long long step;
    long long cap;
    if (!h || h->n <= 0) {
        return -1;
    }
    if (!h->hash || h->hash_cap < 8) {
        long long i;
        for (i = 0; i < h->n; i++) {
            if (_upp_hesit(h, i, ki, ks)) {
                return i;
            }
        }
        return -1;
    }
    cap = h->hash_cap;
    hv = _upp_hkey(h, ki, ks);
    for (step = 0; step < cap; step++) {
        long long p = _upp_hprobe(h, hv, step);
        long long sl = h->hash[p];
        if (sl == UPP_H_EMPTY) {
            return -1;
        }
        if (sl != UPP_H_TOMB && sl >= 0 && sl < h->n && _upp_hesit(h, sl, ki, ks)) {
            return sl;
        }
    }
    return -1;
}

static void _upp_harita_yaz_deger(_UppKolHaritaH* h, long long i, long long vi, double vd, const char* vs) {
    if (h->vtag == UPP_KT_ONDALIK) {
        h->v_d[i] = vd;
    } else if (h->vtag == UPP_KT_METIN) {
        if (h->v_s[i]) {
            upp_metin_release(h->v_s[i]);
        }
        h->v_s[i] = _upp_kol_kopya(vs);
    } else {
        h->v_i[i] = vi;
    }
}

static void upp_harita_koy(UppKolHarita* H, int ktag, int vtag, long long ki, const char* ks, long long vi, double vd, const char* vs) {
    _UppKolHaritaH* h;
    long long f;
    if (!_upp_harita_hazir(H, ktag, vtag)) {
        return;
    }
    h = _upp_harita_hucre(H->id);
    if (!h) {
        return;
    }
    f = _upp_harita_bul(h, ki, ks);
    if (f >= 0) {
        _upp_harita_yaz_deger(h, f, vi, vd, vs);
        return;
    }
    if (h->n >= h->cap && !_upp_harita_buyut(h)) {
        return;
    }
    if (h->n >= h->cap) {
        return;
    }
    if (ktag == UPP_KT_METIN) {
        h->k_s[h->n] = _upp_kol_kopya(ks);
    } else {
        h->k_i[h->n] = ki;
    }
    if (vtag == UPP_KT_ONDALIK) {
        h->v_d[h->n] = vd;
    } else if (vtag == UPP_KT_METIN) {
        h->v_s[h->n] = _upp_kol_kopya(vs);
    } else {
        h->v_i[h->n] = vi;
    }
    h->n++;
    _upp_harita_indeks_kaydet(h, h->n - 1);
}

static long long upp_harita_al_i(UppKolHarita H, int ktag, long long ki, const char* ks) {
    _UppKolHaritaH* h = _upp_harita_hucre(H.id);
    long long f;
    if (!h || h->ktag != ktag || h->vtag == UPP_KT_ONDALIK || h->vtag == UPP_KT_METIN) {
        return 0;
    }
    f = _upp_harita_bul(h, ki, ks);
    return f < 0 ? 0 : h->v_i[f];
}

static double upp_harita_al_d(UppKolHarita H, int ktag, long long ki, const char* ks) {
    _UppKolHaritaH* h = _upp_harita_hucre(H.id);
    long long f;
    if (!h || h->ktag != ktag || h->vtag != UPP_KT_ONDALIK) {
        return 0.0;
    }
    f = _upp_harita_bul(h, ki, ks);
    return f < 0 ? 0.0 : h->v_d[f];
}

static char* upp_harita_al_s(UppKolHarita H, int ktag, long long ki, const char* ks) {
    _UppKolHaritaH* h = _upp_harita_hucre(H.id);
    long long f;
    if (!h || h->ktag != ktag || h->vtag != UPP_KT_METIN) {
        return NULL;
    }
    f = _upp_harita_bul(h, ki, ks);
    return f < 0 ? NULL : (h->v_s[f] ? _upp_kol_kopya(h->v_s[f]) : NULL);
}

static int upp_harita_var_mi(UppKolHarita H, int ktag, long long ki, const char* ks) {
    _UppKolHaritaH* h = _upp_harita_hucre(H.id);
    if (!h || h->ktag != ktag) {
        return 0;
    }
    return _upp_harita_bul(h, ki, ks) >= 0;
}

static long long upp_harita_uzunluk(UppKolHarita H) {
    _UppKolHaritaH* h = _upp_harita_hucre(H.id);
    return h ? h->n : 0;
}

static void upp_harita_bosalt(UppKolHarita* H) {
    _UppKolHaritaH* h;
    long long i;
    if (!H || H->id == 0) {
        return;
    }
    h = _upp_harita_hucre(H->id);
    if (!h) {
        H->id = 0;
        return;
    }
    if (h->ktag == UPP_KT_METIN && h->k_s) {
        for (i = 0; i < h->n; i++) {
            upp_metin_release(h->k_s[i]);
        }
    }
    if (h->vtag == UPP_KT_METIN && h->v_s) {
        for (i = 0; i < h->n; i++) {
            upp_metin_release(h->v_s[i]);
        }
    }
    UPP_BIRAK(h->k_i);
    UPP_BIRAK(h->k_s);
    UPP_BIRAK(h->v_i);
    UPP_BIRAK(h->v_d);
    UPP_BIRAK(h->v_s);
    UPP_BIRAK(h->hash);
    memset(h, 0, sizeof(_UppKolHaritaH));
    H->id = 0;
}

static long long upp_harita_sil(UppKolHarita* H, int ktag, long long ki, const char* ks) {
    _UppKolHaritaH* h;
    long long idx, j;
    if (!H || H->id == 0) return 0;
    h = _upp_harita_hucre(H->id);
    if (!h || h->ktag != ktag) return 0;
    idx = _upp_harita_bul(h, ki, ks);
    if (idx < 0) return 0;
    if (h->ktag == UPP_KT_METIN && h->k_s[idx]) {
        upp_metin_release(h->k_s[idx]);
    }
    if (h->vtag == UPP_KT_METIN && h->v_s[idx]) {
        upp_metin_release(h->v_s[idx]);
    }
    for (j = idx; j < h->n - 1; j++) {
        if (h->ktag == UPP_KT_METIN) h->k_s[j] = h->k_s[j + 1];
        else h->k_i[j] = h->k_i[j + 1];

        if (h->vtag == UPP_KT_METIN) h->v_s[j] = h->v_s[j + 1];
        else if (h->vtag == UPP_KT_ONDALIK) h->v_d[j] = h->v_d[j + 1];
        else h->v_i[j] = h->v_i[j + 1];
    }
    if (h->ktag == UPP_KT_METIN) h->k_s[h->n - 1] = NULL;
    if (h->vtag == UPP_KT_METIN) h->v_s[h->n - 1] = NULL;
    h->n--;
    if (h->hash && h->hash_cap >= 8) {
        _upp_harita_rehash(h, h->hash_cap);
    }
    return 1;
}

static void upp_harita_temizle(UppKolHarita* H) {
    _UppKolHaritaH* h;
    long long j;
    if (!H || H->id == 0) return;
    h = _upp_harita_hucre(H->id);
    if (!h) return;
    if (h->ktag == UPP_KT_METIN && h->k_s) {
        for (j = 0; j < h->n; j++) {
            if (h->k_s[j]) upp_metin_release(h->k_s[j]);
            h->k_s[j] = NULL;
        }
    }
    if (h->vtag == UPP_KT_METIN && h->v_s) {
        for (j = 0; j < h->n; j++) {
            if (h->v_s[j]) upp_metin_release(h->v_s[j]);
            h->v_s[j] = NULL;
        }
    }
    h->n = 0;
    h->hash_tomb = 0;
    if (h->hash && h->hash_cap > 0) {
        for (j = 0; j < h->hash_cap; j++) {
            h->hash[j] = UPP_H_EMPTY;
        }
    }
}

static long long upp_harita_anahtar_i(UppKolHarita H, long long i) {
    _UppKolHaritaH* h = _upp_harita_hucre(H.id);
    if (!h || h->ktag == UPP_KT_METIN || i < 0 || i >= h->n) {
        return 0;
    }
    return h->k_i[i];
}

static char* upp_harita_anahtar_s(UppKolHarita H, long long i) {
    _UppKolHaritaH* h = _upp_harita_hucre(H.id);
    if (!h || h->ktag != UPP_KT_METIN || i < 0 || i >= h->n) {
        return NULL;
    }
    return h->k_s[i] ? _upp_kol_kopya(h->k_s[i]) : NULL;
}

static UppKolListe upp_harita_anahtarlar(UppKolHarita H, int ktag) {
    UppKolListe L;
    _UppKolHaritaH* h;
    long long i;
    L.id = 0;
    h = _upp_harita_hucre(H.id);
    if (!h || h->ktag != ktag) {
        return L;
    }
    for (i = 0; i < h->n; i++) {
        if (ktag == UPP_KT_METIN) {
            upp_liste_ekle(&L, UPP_KT_METIN, 0, 0.0, h->k_s[i]);
        } else {
            upp_liste_ekle(&L, UPP_KT_SAYI, h->k_i[i], 0.0, NULL);
        }
    }
    return L;
}

static UppKolListe upp_metin_bol(const char* s, const char* ayirac) {
    UppKolListe L;
    size_t n, na;
    L.id = 0;
    if (!s) {
        s = "";
    }
    if (!ayirac) {
        ayirac = "";
    }
    n = strlen(s);
    na = strlen(ayirac);
    if (na == 0) {
        size_t i = 0;
        while (i < n) {
            long long adv = _upp_utf8_adv(s, n, i);
            char tmp[8];
            if (adv <= 0) {
                break;
            }
            if ((size_t)adv > sizeof(tmp) - 1u) {
                adv = 1;
            }
            memcpy(tmp, s + i, (size_t)adv);
            tmp[adv] = 0;
            _upp_liste_ekle_c(&L, tmp);
            i += (size_t)adv;
        }
        return L;
    }
    {
        const char* p = s;
        const char* found;
        while ((found = strstr(p, ayirac)) != NULL) {
            size_t take = (size_t)(found - p);
            char* parca = (char*)UPP_AYIR(take + 1u);
            if (!parca) {
                return L;
            }
            if (take) {
                memcpy(parca, p, take);
            }
            parca[take] = 0;
            _upp_liste_ekle_c(&L, parca);
            UPP_BIRAK(parca);
            p = found + na;
        }
        _upp_liste_ekle_c(&L, p);
    }
    return L;
}

static char* upp_metin_liste_birlestir(UppKolListe L, const char* ayirac) {
    _UppKolListeH* h;
    long long i;
    size_t na, cap, used;
    char* out;
    if (!ayirac) {
        ayirac = "";
    }
    na = strlen(ayirac);
    h = _upp_liste_hucre(L.id);
    if (!h || h->tag != UPP_KT_METIN || h->n <= 0) {
        out = (char*)UPP_AYIR(1u);
        if (!out) {
            return NULL;
        }
        out[0] = 0;
        return _upp_m_sahiplen(out);
    }
    cap = 1u;
    for (i = 0; i < h->n; i++) {
        cap += h->s[i] ? strlen(h->s[i]) : 0u;
        if (i + 1 < h->n) {
            cap += na;
        }
    }
    out = (char*)UPP_AYIR(cap);
    if (!out) {
        return NULL;
    }
    used = 0;
    for (i = 0; i < h->n; i++) {
        const char* parca = h->s[i] ? h->s[i] : "";
        size_t np = strlen(parca);
        memcpy(out + used, parca, np);
        used += np;
        if (i + 1 < h->n && na) {
            memcpy(out + used, ayirac, na);
            used += na;
        }
    }
    out[used] = 0;
    return _upp_m_sahiplen(out);
}

/* =====================================================================
 * u++ çalışma zamanı — SERBEST katman (x86_64 çekirdek)
 *
 * Çekirdeğin platform sözleşmesini donanım üzerinden karşılar:
 *   - çıktı: COM1 seri port (UTF-8 olduğu gibi) + VGA metin ekranı
 *     (0xB8000, Türkçe harfler CP437'ye eşlenir)
 *   - panik: mesajı yazar, kesmeleri kapatır, işlemciyi durdurur
 *   - ondalık biçimlendirme: libc'siz "%g" benzeri
 *   - upp.cekirdek.*: port G/Ç, MMIO, hlt/cli/sti, yığın bölgesi
 * Önyükleyici (boot.S) ilk 1 GiB'ı birebir eşler, SSE'yi açar ve
 * upp_cekirdek_giris()'i çağırır.
 * ===================================================================== */

/* ---- port G/Ç ve ayrıcalıklı yönergeler ---- */
static inline void _upp_outb(uint16_t p, uint8_t v) { __asm__ volatile("outb %0, %1" : : "a"(v), "Nd"(p)); }
static inline void _upp_outw(uint16_t p, uint16_t v) { __asm__ volatile("outw %0, %1" : : "a"(v), "Nd"(p)); }
static inline void _upp_outl(uint16_t p, uint32_t v) { __asm__ volatile("outl %0, %1" : : "a"(v), "Nd"(p)); }
static inline uint8_t _upp_inb(uint16_t p) { uint8_t v; __asm__ volatile("inb %1, %0" : "=a"(v) : "Nd"(p)); return v; }
static inline uint16_t _upp_inw(uint16_t p) { uint16_t v; __asm__ volatile("inw %1, %0" : "=a"(v) : "Nd"(p)); return v; }
static inline uint32_t _upp_inl(uint16_t p) { uint32_t v; __asm__ volatile("inl %1, %0" : "=a"(v) : "Nd"(p)); return v; }

static void upp_cekirdek_port_yaz8(long long p, long long v) { _upp_outb((uint16_t)p, (uint8_t)v); }
static void upp_cekirdek_port_yaz16(long long p, long long v) { _upp_outw((uint16_t)p, (uint16_t)v); }
static void upp_cekirdek_port_yaz32(long long p, long long v) { _upp_outl((uint16_t)p, (uint32_t)v); }
static long long upp_cekirdek_port_oku8(long long p) { return _upp_inb((uint16_t)p); }
static long long upp_cekirdek_port_oku16(long long p) { return _upp_inw((uint16_t)p); }
static long long upp_cekirdek_port_oku32(long long p) { return _upp_inl((uint16_t)p); }

static void upp_cekirdek_bellek_yaz8(long long a, long long v) { *(volatile uint8_t*)(uintptr_t)a = (uint8_t)v; }
static void upp_cekirdek_bellek_yaz16(long long a, long long v) { *(volatile uint16_t*)(uintptr_t)a = (uint16_t)v; }
static void upp_cekirdek_bellek_yaz32(long long a, long long v) { *(volatile uint32_t*)(uintptr_t)a = (uint32_t)v; }
static void upp_cekirdek_bellek_yaz64(long long a, long long v) { *(volatile uint64_t*)(uintptr_t)a = (uint64_t)v; }
static long long upp_cekirdek_bellek_oku8(long long a) { return *(volatile uint8_t*)(uintptr_t)a; }
static long long upp_cekirdek_bellek_oku16(long long a) { return *(volatile uint16_t*)(uintptr_t)a; }
static long long upp_cekirdek_bellek_oku32(long long a) { return *(volatile uint32_t*)(uintptr_t)a; }
static long long upp_cekirdek_bellek_oku64(long long a) { return (long long)*(volatile uint64_t*)(uintptr_t)a; }

static void upp_cekirdek_dur(void) { __asm__ volatile("hlt"); }
static void upp_cekirdek_kesme_kapat(void) { __asm__ volatile("cli"); }
static void upp_cekirdek_kesme_ac(void) { __asm__ volatile("sti"); }

static void upp_cekirdek_yigin_kur(long long adres, long long boyut) {
    if (boyut > 0) {
        _upp_yigin_bolge((void*)(uintptr_t)adres, (size_t)boyut);
    }
}

static void _upp_sonsuz_dur(void) {
    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}

/* ---- COM1 seri port ---- */
#define _UPP_COM1 0x3F8
static int _upp_seri_hazir = 0;

static void _upp_seri_kur(void) {
    if (_upp_seri_hazir) {
        return;
    }
    _upp_seri_hazir = 1;
    _upp_outb(_UPP_COM1 + 1, 0x00); /* kesmeler kapalı */
    _upp_outb(_UPP_COM1 + 3, 0x80); /* DLAB */
    _upp_outb(_UPP_COM1 + 0, 0x01); /* 115200 baud */
    _upp_outb(_UPP_COM1 + 1, 0x00);
    _upp_outb(_UPP_COM1 + 3, 0x03); /* 8N1 */
    _upp_outb(_UPP_COM1 + 2, 0xC7); /* FIFO */
    _upp_outb(_UPP_COM1 + 4, 0x0B);
}

static void _upp_seri_bayt(unsigned char c) {
    int bekle = 100000;
    _upp_seri_kur();
    while (bekle-- > 0 && (_upp_inb(_UPP_COM1 + 5) & 0x20) == 0) {
    }
    _upp_outb(_UPP_COM1, c);
}

/* ---- VGA metin ekranı ---- */
#define _UPP_VGA ((volatile uint16_t*)(uintptr_t)0xB8000)
#define _UPP_VGA_G 80
#define _UPP_VGA_Y 25
static int _upp_vga_x = 0;
static int _upp_vga_y = 0;

static void _upp_vga_kaydir(void) {
    int i;
    for (i = 0; i < _UPP_VGA_G * (_UPP_VGA_Y - 1); i++) {
        _UPP_VGA[i] = _UPP_VGA[i + _UPP_VGA_G];
    }
    for (i = _UPP_VGA_G * (_UPP_VGA_Y - 1); i < _UPP_VGA_G * _UPP_VGA_Y; i++) {
        _UPP_VGA[i] = (uint16_t)(0x0700 | ' ');
    }
    _upp_vga_y = _UPP_VGA_Y - 1;
}

static void _upp_vga_karakter(unsigned char c, uint8_t renk) {
    if (c == '\n') {
        _upp_vga_x = 0;
        if (++_upp_vga_y >= _UPP_VGA_Y) {
            _upp_vga_kaydir();
        }
        return;
    }
    if (c == '\r') {
        _upp_vga_x = 0;
        return;
    }
    _UPP_VGA[_upp_vga_y * _UPP_VGA_G + _upp_vga_x] = (uint16_t)(((uint16_t)renk << 8) | c);
    if (++_upp_vga_x >= _UPP_VGA_G) {
        _upp_vga_x = 0;
        if (++_upp_vga_y >= _UPP_VGA_Y) {
            _upp_vga_kaydir();
        }
    }
}

/* Unicode kod noktası → CP437 (VGA yazı tipi); Türkçe harfler dahil. */
static unsigned char _upp_cp437(long long cp) {
    if (cp >= 32 && cp < 127) return (unsigned char)cp;
    if (cp == '\n' || cp == '\r') return (unsigned char)cp;
    switch (cp) {
    case 0xE7: return 0x87; /* ç */
    case 0xC7: return 0x80; /* Ç */
    case 0xFC: return 0x81; /* ü */
    case 0xDC: return 0x9A; /* Ü */
    case 0xF6: return 0x94; /* ö */
    case 0xD6: return 0x99; /* Ö */
    case 0x11F: return 'g'; /* ğ */
    case 0x11E: return 'G'; /* Ğ */
    case 0x131: return 'i'; /* ı */
    case 0x130: return 'I'; /* İ */
    case 0x15F: return 's'; /* ş */
    case 0x15E: return 'S'; /* Ş */
    case 0xE2: return 0x83; /* â */
    case 0xEE: return 0x8C; /* î */
    case 0xFB: return 0x96; /* û */
    default: return '?';
    }
}

static void _upp_akis_bayt(int akis, const char* p, size_t n, int nl) {
    uint8_t renk = akis == 2 ? 0x0C : 0x07;
    size_t i = 0;
    while (p && i < n) {
        size_t a = (size_t)_upp_utf8_adv(p, n, i);
        size_t k;
        for (k = 0; k < a; k++) {
            _upp_seri_bayt((unsigned char)p[i + k]);
        }
        _upp_vga_karakter(_upp_cp437(_upp_utf8_cp_at(p, n, i)), renk);
        i += a;
    }
    if (nl) {
        _upp_seri_bayt('\r');
        _upp_seri_bayt('\n');
        _upp_vga_karakter('\n', renk);
    }
}

static void upp_panik(const char* mesaj) {
    const char* on = "[u++ PANİK] ";
    _upp_akis_bayt(2, on, strlen(on), 0);
    if (!mesaj) {
        mesaj = "iç hata";
    }
    _upp_akis_bayt(2, mesaj, strlen(mesaj), 1);
    _upp_sonsuz_dur();
}

/* ---- libc'siz "%g" benzeri ondalık yazımı ---- */
static size_t _upp_ondalik_yaz(char* b, size_t cap, double v, int hassas17) {
    char t[48];
    size_t n = 0;
    int basamak = hassas17 ? 17 : 6;
    int us = 0;
    int i;
    unsigned long long tam;
    double olcek = 1.0;
    if (cap == 0) {
        return 0;
    }
    if (v != v) {
        const char* s = "nan";
        for (i = 0; s[i] && n + 1 < cap; i++) b[n++] = s[i];
        b[n] = 0;
        return n;
    }
    if (v < 0.0) {
        t[n++] = '-';
        v = -v;
    }
    if (v > 1.7976931348623157e308) {
        const char* s = "inf";
        for (i = 0; s[i]; i++) t[n++] = s[i];
        goto bitir;
    }
    if (v == 0.0) {
        t[n++] = '0';
        goto bitir;
    }
    while (v >= 10.0) { v /= 10.0; us++; }
    while (v < 1.0) { v *= 10.0; us--; }
    for (i = 1; i < basamak; i++) olcek *= 10.0;
    tam = (unsigned long long)(v * olcek + 0.5);
    if (tam >= (unsigned long long)(olcek * 10.0)) {
        tam /= 10u;
        us++;
    }
    {
        char d[20];
        int nd = 0;
        int son;
        while (nd < basamak) {
            d[basamak - 1 - nd] = (char)('0' + (int)(tam % 10u));
            tam /= 10u;
            nd++;
        }
        son = basamak;
        while (son > 1 && d[son - 1] == '0') son--;
        if (us >= -4 && us < basamak) {
            if (us >= 0) {
                for (i = 0; i <= us; i++) t[n++] = i < son ? d[i] : '0';
                if (son > us + 1) {
                    t[n++] = '.';
                    for (i = us + 1; i < son; i++) t[n++] = d[i];
                }
            } else {
                t[n++] = '0';
                t[n++] = '.';
                for (i = 0; i < -us - 1; i++) t[n++] = '0';
                for (i = 0; i < son; i++) t[n++] = d[i];
            }
        } else {
            int u = us < 0 ? -us : us;
            t[n++] = d[0];
            if (son > 1) {
                t[n++] = '.';
                for (i = 1; i < son; i++) t[n++] = d[i];
            }
            t[n++] = 'e';
            t[n++] = us < 0 ? '-' : '+';
            if (u >= 100) t[n++] = (char)('0' + u / 100);
            t[n++] = (char)('0' + (u / 10) % 10);
            t[n++] = (char)('0' + u % 10);
        }
    }
bitir:
    if (n >= cap) n = cap - 1;
    for (i = 0; i < (int)n; i++) b[i] = t[i];
    b[n] = 0;
    return n;
}

/* ---- serbest hedefte de var olan yerleşikler ---- */
static char* upp_ondalikdan_metin(double x) {
    char b[48];
    size_t n = _upp_ondalik_yaz(b, sizeof(b), x, 1);
    return _upp_m_bayttan(b, n);
}
