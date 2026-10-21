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

/* (T)x ondalık -> tamsayı: kesilmiş değer [alt, ust) dışında ya da NaN ise panik
   (C'de bu dönüşüm belirsiz davranıştır). */
static long long upp_ondalik_tam(double d, double alt, double ust) {
    if (!((d >= alt || d > alt - 1.0) && d < ust)) {
        upp_panik("tür dönüşümünde taşma (ondalik -> tamsayı)");
    }
    return (long long)d;
}

static unsigned long long upp_ondalik_u64(double d) {
    if (!(d > -1.0 && d < 18446744073709551616.0)) {
        upp_panik("tür dönüşümünde taşma (ondalik -> u64)");
    }
    return (unsigned long long)d;
}

/* beklenen[T, metin].deger() hata taşırken: mesajı yığında birleştirir (ayırma yok). */
static void upp_beklenen_panik(const char* hata) {
    static const char on[] = "beklenen: değer yerine hata var: ";
    char b[512];
    size_t n = sizeof(on) - 1;
    size_t h = hata ? strlen(hata) : 0;
    memcpy(b, on, n);
    if (h > sizeof(b) - n - 1) {
        h = sizeof(b) - n - 1;
    }
    if (h) {
        memcpy(b + n, hata, h);
    }
    b[n + h] = '\0';
    upp_panik(b);
}

static long long upp_bol_s(long long a, long long b) {
    if (b == 0) {
        upp_panik("sıfıra bölme");
    }
    if (b == -1 && a == LLONG_MIN) {
        upp_panik("tamsayı bölmede taşma");
    }
    return a / b;
}

static long long upp_mod_s(long long a, long long b) {
    if (b == 0) {
        upp_panik("sıfıra bölme (kalan)");
    }
    if (b == -1) {
        return 0;
    }
    return a % b;
}

static unsigned long long upp_bol_u(unsigned long long a, unsigned long long b) {
    if (b == 0) {
        upp_panik("sıfıra bölme");
    }
    return a / b;
}

static unsigned long long upp_mod_u(unsigned long long a, unsigned long long b) {
    if (b == 0) {
        upp_panik("sıfıra bölme (kalan)");
    }
    return a % b;
}

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

/* ---- koleksiyon v2: değer türü liste/harita ----
 * Tutamaç bir hücreye işaretçidir (NULL = boş). Kopya atomik referans artışıdır;
 * değiştiren her işlem hücre paylaşılıyorsa önce onu klonlar (copy-on-write).
 * Elemanlar ham bayt olarak saklanır; tür tanımlayıcısı boyutu ve kopya/bırak
 * kancalarını verir (NULL kanca = düz bellek kopyası / bırakılacak bir şey yok). */
typedef struct UppKolTur {
    size_t boyut;
    void (*kopya)(void* hedef, const void* kaynak);
    void (*birak)(void* p);
} UppKolTur;

typedef struct _UppListe _UppListe;
typedef struct _UppHarita _UppHarita;

typedef struct UppKolListe {
    _UppListe* h;
} UppKolListe;

typedef struct UppKolHarita {
    _UppHarita* h;
} UppKolHarita;

/* Düz (kancasız) türler için çağrı yerinde tanımlayıcı. */
#define UPP_KT_DUZ(T) (&(const UppKolTur){ sizeof(T), NULL, NULL })

#define UPP_KOLLEKSIYON_MAX 100000000LL

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

/* =====================================================================
 * koleksiyon v2 — uygulama
 * ===================================================================== */
struct _UppListe {
    long long ref;
    const UppKolTur* tur;
    long long n;
    long long kap;
    unsigned char* veri;
};

#define _UPP_HK_TAM 0
#define _UPP_HK_METIN 1
#define _UPP_H_BOS (-1LL)
#define _UPP_H_MEZAR (-2LL)

struct _UppHarita {
    long long ref;
    int kk;                  /* anahtar türü: _UPP_HK_TAM (long long) / _UPP_HK_METIN (char*) */
    const UppKolTur* vtur;
    long long n;
    long long kap;
    unsigned char* anahtar;  /* n adet: long long ya da char* — eklenme sırası */
    unsigned char* deger;    /* n adet vtur->boyut */
    long long* hash;         /* açık adresleme: indeks ya da _UPP_H_BOS/_UPP_H_MEZAR */
    long long hash_kap;
    long long mezar;
};

/* ---- yerleşik tür tanımlayıcıları ---- */
static void _upp_kt_metin_kopya(void* d, const void* s) {
    *(char**)d = upp_metin_kopya(*(char* const*)s);
}
static void _upp_kt_metin_birak(void* p) {
    upp_metin_release(*(char**)p);
    *(char**)p = NULL;
}
static UppKolListe upp_liste_paylas(UppKolListe L);
static void upp_liste_birak(UppKolListe* L);
static UppKolHarita upp_harita_paylas(UppKolHarita H);
static void upp_harita_birak(UppKolHarita* H);
static void _upp_kt_liste_kopya(void* d, const void* s) {
    *(UppKolListe*)d = upp_liste_paylas(*(const UppKolListe*)s);
}
static void _upp_kt_liste_birak(void* p) {
    upp_liste_birak((UppKolListe*)p);
}
static void _upp_kt_harita_kopya(void* d, const void* s) {
    *(UppKolHarita*)d = upp_harita_paylas(*(const UppKolHarita*)s);
}
static void _upp_kt_harita_birak(void* p) {
    upp_harita_birak((UppKolHarita*)p);
}

static const UppKolTur upp_kt_sayi = { sizeof(long long), NULL, NULL };
static const UppKolTur upp_kt_ondalik = { sizeof(double), NULL, NULL };
static const UppKolTur upp_kt_mantik = { sizeof(bool), NULL, NULL };
static const UppKolTur upp_kt_metin = { sizeof(char*), _upp_kt_metin_kopya, _upp_kt_metin_birak };
static const UppKolTur upp_kt_liste = { sizeof(UppKolListe), _upp_kt_liste_kopya, _upp_kt_liste_birak };
static const UppKolTur upp_kt_harita = { sizeof(UppKolHarita), _upp_kt_harita_kopya, _upp_kt_harita_birak };

static void _upp_kol_eleman_kopya(const UppKolTur* t, void* d, const void* s) {
    if (t->kopya) {
        t->kopya(d, s);
    } else {
        memcpy(d, s, t->boyut);
    }
}

static void _upp_kol_eleman_birak(const UppKolTur* t, void* p) {
    if (t->birak) {
        t->birak(p);
    }
}

/* ---- liste ---- */
static _UppListe* _upp_liste_yeni(const UppKolTur* t, long long kap) {
    _UppListe* h = (_UppListe*)UPP_AYIR(sizeof(_UppListe));
    if (!h) {
        upp_panik("liste bellek yetersiz");
    }
    h->ref = 1;
    h->tur = t;
    h->n = 0;
    h->kap = kap > 0 ? kap : 0;
    h->veri = NULL;
    if (h->kap > 0) {
        h->veri = (unsigned char*)UPP_AYIR((size_t)h->kap * t->boyut);
        if (!h->veri) {
            upp_panik("liste bellek yetersiz");
        }
    }
    return h;
}

static void _upp_liste_hucre_birak(_UppListe* h) {
    long long i;
    if (h->tur->birak) {
        for (i = 0; i < h->n; i++) {
            h->tur->birak(h->veri + (size_t)i * h->tur->boyut);
        }
    }
    UPP_BIRAK(h->veri);
    UPP_BIRAK(h);
}

static UppKolListe upp_liste_paylas(UppKolListe L) {
    if (L.h) {
        __atomic_add_fetch(&L.h->ref, 1, __ATOMIC_RELAXED);
    }
    return L;
}

static void upp_liste_birak(UppKolListe* L) {
    if (!L || !L->h) {
        return;
    }
    if (__atomic_sub_fetch(&L->h->ref, 1, __ATOMIC_ACQ_REL) == 0) {
        _upp_liste_hucre_birak(L->h);
    }
    L->h = NULL;
}

/* Yazmadan önce: hücre yoksa oluştur, paylaşılıyorsa klonla. */
static _UppListe* _upp_liste_yazilabilir(UppKolListe* L, const UppKolTur* t) {
    _UppListe* h = L->h;
    if (!h) {
        if (!t) {
            return NULL;
        }
        L->h = _upp_liste_yeni(t, 0);
        return L->h;
    }
    if (__atomic_load_n(&h->ref, __ATOMIC_ACQUIRE) > 1) {
        long long i;
        _UppListe* y = _upp_liste_yeni(h->tur, h->n);
        for (i = 0; i < h->n; i++) {
            size_t o = (size_t)i * h->tur->boyut;
            _upp_kol_eleman_kopya(h->tur, y->veri + o, h->veri + o);
        }
        y->n = h->n;
        upp_liste_birak(L);
        L->h = y;
        return y;
    }
    return h;
}

static void _upp_liste_sinir(_UppListe* h, long long i) {
    if (!h || i < 0 || i >= h->n) {
        char* m = upp_mk_bas();
        _upp_m_ekle_n(&m, "liste indeks ", 13u, 1);
        upp_mk_sayi(&m, i);
        _upp_m_ekle_n(&m, ", uzunluk ", 10u, 1);
        upp_mk_sayi(&m, h ? h->n : 0);
        _upp_m_ekle_n(&m, " sınır dışı", strlen(" sınır dışı"), 0);
        upp_panik(m);
    }
}

static long long upp_liste_uzunluk(UppKolListe L) {
    return L.h ? L.h->n : 0;
}

/* Okuma için eleman işaretçisi (sınır denetimli). */
static void* upp_liste_ptr(UppKolListe L, long long i) {
    _upp_liste_sinir(L.h, i);
    return L.h->veri + (size_t)i * L.h->tur->boyut;
}

/* Yazma için eleman işaretçisi (COW + sınır denetimi). */
static void* upp_liste_yaz_ptr(UppKolListe* L, long long i) {
    _UppListe* h = _upp_liste_yazilabilir(L, NULL);
    _upp_liste_sinir(h, i);
    return h->veri + (size_t)i * h->tur->boyut;
}

/* Sona sıfırlanmış yeni bir yer ekler ve işaretçisini döner. */
static void* upp_liste_ekle_ptr(UppKolListe* L, const UppKolTur* t) {
    _UppListe* h = _upp_liste_yazilabilir(L, t);
    unsigned char* p;
    if (h->n >= h->kap) {
        long long yk = h->kap < 8 ? 8 : h->kap * 2;
        unsigned char* yv;
        if (yk > UPP_KOLLEKSIYON_MAX) {
            upp_panik("liste en fazla boyuta ulaştı");
        }
        yv = (unsigned char*)UPP_YENIDEN(h->veri, (size_t)yk * h->tur->boyut);
        if (!yv) {
            upp_panik("liste bellek yetersiz");
        }
        h->veri = yv;
        h->kap = yk;
    }
    p = h->veri + (size_t)h->n * h->tur->boyut;
    memset(p, 0, h->tur->boyut);
    h->n++;
    return p;
}

static void upp_liste_sil(UppKolListe* L, long long i) {
    _UppListe* h = _upp_liste_yazilabilir(L, NULL);
    size_t b;
    _upp_liste_sinir(h, i);
    b = h->tur->boyut;
    _upp_kol_eleman_birak(h->tur, h->veri + (size_t)i * b);
    memmove(h->veri + (size_t)i * b, h->veri + (size_t)(i + 1) * b, (size_t)(h->n - i - 1) * b);
    h->n--;
}

static void upp_liste_temizle(UppKolListe* L) {
    _UppListe* h;
    long long i;
    if (!L->h) {
        return;
    }
    if (__atomic_load_n(&L->h->ref, __ATOMIC_ACQUIRE) > 1) {
        upp_liste_birak(L);
        return;
    }
    h = L->h;
    for (i = 0; i < h->n; i++) {
        _upp_kol_eleman_birak(h->tur, h->veri + (size_t)i * h->tur->boyut);
    }
    h->n = 0;
}

/* Boşalt: bu değişkenin içeriğini bırakır; liste boş kalır. */
static void upp_liste_bosalt(UppKolListe* L) {
    upp_liste_birak(L);
}

static void _upp_liste_ekle_metin(UppKolListe* L, const char* s) {
    upp_metin_ata((char**)upp_liste_ekle_ptr(L, &upp_kt_metin), s);
}

/* Ham C tamponunu (yığın dizisi, dirent adı) önünü okumadan listeye ekler. */
static void _upp_liste_ekle_c(UppKolListe* L, const char* ham) {
    char* m = _upp_m_c(ham);
    *(char**)upp_liste_ekle_ptr(L, &upp_kt_metin) = m;
}

static void _upp_liste_tur_dene(_UppListe* h, const UppKolTur* t, const char* ne) {
    if (h && h->tur->boyut != t->boyut) {
        upp_panik(ne);
    }
}

static upp_ref_dilim_sayi upp_liste_dilim_sayi(UppKolListe L, long long a, long long b) {
    _upp_liste_tur_dene(L.h, &upp_kt_sayi, "liste dilim türü uyumsuz");
    return upp_dilim_sayi(L.h ? (const long long*)L.h->veri : NULL, upp_liste_uzunluk(L), a, b);
}

static upp_ref_dilim_ondalik upp_liste_dilim_ondalik(UppKolListe L, long long a, long long b) {
    _upp_liste_tur_dene(L.h, &upp_kt_ondalik, "liste dilim türü uyumsuz");
    return upp_dilim_ondalik(L.h ? (const double*)L.h->veri : NULL, upp_liste_uzunluk(L), a, b);
}

static upp_ref_dilim_sayi upp_liste_gorunum_sayi(UppKolListe L) {
    return upp_liste_dilim_sayi(L, 0, upp_liste_uzunluk(L));
}

static upp_ref_dilim_ondalik upp_liste_gorunum_ondalik(UppKolListe L) {
    return upp_liste_dilim_ondalik(L, 0, upp_liste_uzunluk(L));
}

/* ---- harita ---- */
static unsigned long long _upp_h_karistir(unsigned long long x) {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
}

static unsigned long long _upp_h_metin(const char* s) {
    unsigned long long h = 1469598103934665603ULL;
    size_t n = s ? upp_metin_nbayt(s) : 0u;
    size_t i;
    for (i = 0; i < n; i++) {
        h ^= (unsigned char)s[i];
        h *= 1099511628211ULL;
    }
    return h;
}

static size_t _upp_h_kboy(int kk) {
    return kk == _UPP_HK_METIN ? sizeof(char*) : sizeof(long long);
}

static unsigned long long _upp_h_anahtar_hash(int kk, const void* kp) {
    if (kk == _UPP_HK_METIN) {
        return _upp_h_metin(*(const char* const*)kp);
    }
    return _upp_h_karistir((unsigned long long)*(const long long*)kp);
}

static int _upp_h_esit(int kk, const void* a, const void* b) {
    if (kk == _UPP_HK_METIN) {
        const char* x = *(const char* const*)a;
        const char* y = *(const char* const*)b;
        return upp_metin_esit(x ? x : "", y ? y : "");
    }
    return *(const long long*)a == *(const long long*)b;
}

static _UppHarita* _upp_harita_yeni(int kk, const UppKolTur* vt) {
    _UppHarita* h = (_UppHarita*)UPP_AYIR(sizeof(_UppHarita));
    if (!h) {
        upp_panik("harita bellek yetersiz");
    }
    memset(h, 0, sizeof(*h));
    h->ref = 1;
    h->kk = kk;
    h->vtur = vt;
    return h;
}

static void _upp_harita_hucre_birak(_UppHarita* h) {
    long long i;
    for (i = 0; i < h->n; i++) {
        if (h->kk == _UPP_HK_METIN) {
            upp_metin_release(((char**)h->anahtar)[i]);
        }
        _upp_kol_eleman_birak(h->vtur, h->deger + (size_t)i * h->vtur->boyut);
    }
    UPP_BIRAK(h->anahtar);
    UPP_BIRAK(h->deger);
    UPP_BIRAK(h->hash);
    UPP_BIRAK(h);
}

static UppKolHarita upp_harita_paylas(UppKolHarita H) {
    if (H.h) {
        __atomic_add_fetch(&H.h->ref, 1, __ATOMIC_RELAXED);
    }
    return H;
}

static void upp_harita_birak(UppKolHarita* H) {
    if (!H || !H->h) {
        return;
    }
    if (__atomic_sub_fetch(&H->h->ref, 1, __ATOMIC_ACQ_REL) == 0) {
        _upp_harita_hucre_birak(H->h);
    }
    H->h = NULL;
}

static void _upp_harita_yeniden_hashle(_UppHarita* h, long long kap) {
    long long i;
    long long* yeni = (long long*)UPP_AYIR((size_t)kap * sizeof(long long));
    size_t kb = _upp_h_kboy(h->kk);
    if (!yeni) {
        upp_panik("harita bellek yetersiz");
    }
    for (i = 0; i < kap; i++) {
        yeni[i] = _UPP_H_BOS;
    }
    for (i = 0; i < h->n; i++) {
        unsigned long long s = _upp_h_anahtar_hash(h->kk, h->anahtar + (size_t)i * kb) & (unsigned long long)(kap - 1);
        while (yeni[s] != _UPP_H_BOS) {
            s = (s + 1u) & (unsigned long long)(kap - 1);
        }
        yeni[s] = i;
    }
    UPP_BIRAK(h->hash);
    h->hash = yeni;
    h->hash_kap = kap;
    h->mezar = 0;
}

/* Anahtarın girdi indeksi; yoksa -1. */
static long long _upp_harita_ara(const _UppHarita* h, const void* kp) {
    unsigned long long s;
    size_t kb;
    if (!h || h->hash_kap == 0) {
        return -1;
    }
    kb = _upp_h_kboy(h->kk);
    s = _upp_h_anahtar_hash(h->kk, kp) & (unsigned long long)(h->hash_kap - 1);
    for (;;) {
        long long g = h->hash[s];
        if (g == _UPP_H_BOS) {
            return -1;
        }
        if (g >= 0 && _upp_h_esit(h->kk, h->anahtar + (size_t)g * kb, kp)) {
            return g;
        }
        s = (s + 1u) & (unsigned long long)(h->hash_kap - 1);
    }
}

static _UppHarita* _upp_harita_yazilabilir(UppKolHarita* H, int kk, const UppKolTur* vt) {
    _UppHarita* h = H->h;
    if (!h) {
        if (!vt) {
            return NULL;
        }
        H->h = _upp_harita_yeni(kk, vt);
        return H->h;
    }
    if (__atomic_load_n(&h->ref, __ATOMIC_ACQUIRE) > 1) {
        long long i;
        size_t kb = _upp_h_kboy(h->kk);
        _UppHarita* y = _upp_harita_yeni(h->kk, h->vtur);
        y->kap = h->n;
        if (h->n > 0) {
            y->anahtar = (unsigned char*)UPP_AYIR((size_t)h->n * kb);
            y->deger = (unsigned char*)UPP_AYIR((size_t)h->n * h->vtur->boyut);
            if (!y->anahtar || !y->deger) {
                upp_panik("harita bellek yetersiz");
            }
        }
        for (i = 0; i < h->n; i++) {
            if (h->kk == _UPP_HK_METIN) {
                ((char**)y->anahtar)[i] = upp_metin_kopya(((char**)h->anahtar)[i]);
            } else {
                ((long long*)y->anahtar)[i] = ((long long*)h->anahtar)[i];
            }
            _upp_kol_eleman_kopya(h->vtur, y->deger + (size_t)i * h->vtur->boyut, h->deger + (size_t)i * h->vtur->boyut);
        }
        y->n = h->n;
        if (h->hash_kap > 0) {
            _upp_harita_yeniden_hashle(y, h->hash_kap);
        }
        upp_harita_birak(H);
        H->h = y;
        return y;
    }
    return h;
}

static long long upp_harita_uzunluk(UppKolHarita H) {
    return H.h ? H.h->n : 0;
}

/* Değer işaretçisi; anahtar yoksa NULL. */
static void* upp_harita_bul(UppKolHarita H, const void* kp) {
    long long g = _upp_harita_ara(H.h, kp);
    if (g < 0) {
        return NULL;
    }
    return H.h->deger + (size_t)g * H.h->vtur->boyut;
}

static bool upp_harita_var(UppKolHarita H, const void* kp) {
    return _upp_harita_ara(H.h, kp) >= 0;
}

/* Anahtarın değer yeri (yoksa sıfırlanmış yeni girdi); COW sonrası. */
static void* upp_harita_koy_ptr(UppKolHarita* H, int kk, const UppKolTur* vt, const void* kp) {
    _UppHarita* h = _upp_harita_yazilabilir(H, kk, vt);
    long long g = _upp_harita_ara(h, kp);
    size_t kb = _upp_h_kboy(h->kk);
    unsigned long long s;
    if (g >= 0) {
        return h->deger + (size_t)g * h->vtur->boyut;
    }
    if (h->n >= h->kap) {
        long long yk = h->kap < 8 ? 8 : h->kap * 2;
        unsigned char* ya;
        unsigned char* yd;
        if (yk > UPP_KOLLEKSIYON_MAX) {
            upp_panik("harita en fazla boyuta ulaştı");
        }
        ya = (unsigned char*)UPP_YENIDEN(h->anahtar, (size_t)yk * kb);
        if (!ya) {
            upp_panik("harita bellek yetersiz");
        }
        h->anahtar = ya;
        yd = (unsigned char*)UPP_YENIDEN(h->deger, (size_t)yk * h->vtur->boyut);
        if (!yd) {
            upp_panik("harita bellek yetersiz");
        }
        h->deger = yd;
        h->kap = yk;
    }
    if ((h->n + h->mezar + 1) * 4 >= h->hash_kap * 3) {
        long long hk = h->hash_kap < 16 ? 16 : h->hash_kap;
        while ((h->n + 1) * 4 >= hk * 3) {
            hk *= 2;
        }
        _upp_harita_yeniden_hashle(h, hk);
    }
    g = h->n;
    if (h->kk == _UPP_HK_METIN) {
        ((char**)h->anahtar)[g] = upp_metin_kopya(*(const char* const*)kp);
    } else {
        ((long long*)h->anahtar)[g] = *(const long long*)kp;
    }
    memset(h->deger + (size_t)g * h->vtur->boyut, 0, h->vtur->boyut);
    h->n++;
    s = _upp_h_anahtar_hash(h->kk, kp) & (unsigned long long)(h->hash_kap - 1);
    while (h->hash[s] >= 0) {
        s = (s + 1u) & (unsigned long long)(h->hash_kap - 1);
    }
    if (h->hash[s] == _UPP_H_MEZAR) {
        h->mezar--;
    }
    h->hash[s] = g;
    return h->deger + (size_t)g * h->vtur->boyut;
}

/* Sil: eklenme sırası korunur (kaydırma + yeniden hash). */
static bool upp_harita_kaldir(UppKolHarita* H, const void* kp) {
    _UppHarita* h;
    long long g;
    size_t kb;
    if (!H->h || _upp_harita_ara(H->h, kp) < 0) {
        return false;
    }
    h = _upp_harita_yazilabilir(H, 0, NULL);
    g = _upp_harita_ara(h, kp);
    kb = _upp_h_kboy(h->kk);
    if (h->kk == _UPP_HK_METIN) {
        upp_metin_release(((char**)h->anahtar)[g]);
    }
    _upp_kol_eleman_birak(h->vtur, h->deger + (size_t)g * h->vtur->boyut);
    memmove(h->anahtar + (size_t)g * kb, h->anahtar + (size_t)(g + 1) * kb, (size_t)(h->n - g - 1) * kb);
    memmove(h->deger + (size_t)g * h->vtur->boyut, h->deger + (size_t)(g + 1) * h->vtur->boyut, (size_t)(h->n - g - 1) * h->vtur->boyut);
    h->n--;
    _upp_harita_yeniden_hashle(h, h->hash_kap);
    return true;
}

static void upp_harita_temizle(UppKolHarita* H) {
    upp_harita_birak(H);
}

static void upp_harita_bosalt(UppKolHarita* H) {
    upp_harita_birak(H);
}

/* Anahtarların listesi (eklenme sırası). */
static UppKolListe upp_harita_anahtar_listesi(UppKolHarita H) {
    UppKolListe L;
    long long i;
    L.h = NULL;
    if (!H.h) {
        return L;
    }
    for (i = 0; i < H.h->n; i++) {
        if (H.h->kk == _UPP_HK_METIN) {
            _upp_liste_ekle_metin(&L, ((char**)H.h->anahtar)[i]);
        } else {
            *(long long*)upp_liste_ekle_ptr(&L, &upp_kt_sayi) = ((long long*)H.h->anahtar)[i];
        }
    }
    return L;
}

/* ---- metin.bol / metin.birlestir ---- */
static UppKolListe upp_metin_bol(const char* s, const char* ayirac) {
    UppKolListe L;
    size_t n, na;
    L.h = NULL;
    if (!s) {
        s = UPP_LIT("");
    }
    if (!ayirac) {
        ayirac = UPP_LIT("");
    }
    n = upp_metin_nbayt(s);
    na = upp_metin_nbayt(ayirac);
    if (na == 0) {
        size_t i = 0;
        while (i < n) {
            long long adv = _upp_utf8_adv(s, n, i);
            if (adv <= 0) {
                break;
            }
            *(char**)upp_liste_ekle_ptr(&L, &upp_kt_metin) = _upp_m_bayttan(s + i, (size_t)adv);
            i += (size_t)adv;
        }
        return L;
    }
    {
        size_t bas = 0;
        size_t i = 0;
        while (i + na <= n) {
            if (memcmp(s + i, ayirac, na) == 0) {
                *(char**)upp_liste_ekle_ptr(&L, &upp_kt_metin) = _upp_m_bayttan(s + bas, i - bas);
                i += na;
                bas = i;
            } else {
                i++;
            }
        }
        *(char**)upp_liste_ekle_ptr(&L, &upp_kt_metin) = _upp_m_bayttan(s + bas, n - bas);
    }
    return L;
}

static char* upp_metin_liste_birlestir(UppKolListe L, const char* ayirac) {
    char* o = upp_mk_bas();
    long long i;
    long long n = upp_liste_uzunluk(L);
    if (n > 0) {
        _upp_liste_tur_dene(L.h, &upp_kt_metin, "metin.birlestir liste[metin] bekler");
    }
    for (i = 0; i < n; i++) {
        if (i > 0 && ayirac) {
            upp_metin_ekle(&o, ayirac);
        }
        upp_metin_ekle(&o, ((char**)L.h->veri)[i]);
    }
    return o;
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
    /* FCR'ye yazılmaz: FIFO'yu açmak (QEMU, 16550) gelmiş baytları siler. */
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
    if (c == 8) {
        if (_upp_vga_x > 0) {
            _upp_vga_x--;
        }
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
    if (cp == '\n' || cp == '\r' || cp == 8) return (unsigned char)cp;
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
    /* QEMU isa-debug-exit (0xf4) varsa çıkış kodu 3 ile kapanır; gerçek donanımda etkisizdir. */
    _upp_outl(0xf4, 1);
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

/* =====================================================================
 * Kesmeler: IDT, işlemci istisnaları, PIC, PIT zamanlayıcı, PS/2 klavye,
 * COM1 girdisi. İlk upp.zaman/uyut/giris çağrısında ya da
 * upp.cekirdek.kesmeler_kur() ile kendiliğinden kurulur.
 * ===================================================================== */
typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t vektor, hata;
    uint64_t rip, cs, rflags, rsp, ss;
} UppKesmeCerceve;

typedef struct __attribute__((packed)) {
    uint16_t ofs_dusuk;
    uint16_t secici;
    uint8_t ist;
    uint8_t tur;
    uint16_t ofs_orta;
    uint32_t ofs_yuksek;
    uint32_t sifir;
} UppIdtGiris;

typedef struct __attribute__((packed)) {
    uint16_t sinir;
    uint64_t taban;
} UppIdtGosterge;

#define _UPP_ISR_SIFIR(n) ".global _upp_isr" #n "\n_upp_isr" #n ":\n    push $0\n    push $" #n "\n    jmp _upp_isr_ortak\n"
#define _UPP_ISR_HATA(n) ".global _upp_isr" #n "\n_upp_isr" #n ":\n    push $" #n "\n    jmp _upp_isr_ortak\n"

__asm__(
    ".text\n"
    _UPP_ISR_SIFIR(0) _UPP_ISR_SIFIR(1) _UPP_ISR_SIFIR(2) _UPP_ISR_SIFIR(3)
    _UPP_ISR_SIFIR(4) _UPP_ISR_SIFIR(5) _UPP_ISR_SIFIR(6) _UPP_ISR_SIFIR(7)
    _UPP_ISR_HATA(8) _UPP_ISR_SIFIR(9) _UPP_ISR_HATA(10) _UPP_ISR_HATA(11)
    _UPP_ISR_HATA(12) _UPP_ISR_HATA(13) _UPP_ISR_HATA(14) _UPP_ISR_SIFIR(15)
    _UPP_ISR_SIFIR(16) _UPP_ISR_HATA(17) _UPP_ISR_SIFIR(18) _UPP_ISR_SIFIR(19)
    _UPP_ISR_SIFIR(20) _UPP_ISR_HATA(21) _UPP_ISR_SIFIR(22) _UPP_ISR_SIFIR(23)
    _UPP_ISR_SIFIR(24) _UPP_ISR_SIFIR(25) _UPP_ISR_SIFIR(26) _UPP_ISR_SIFIR(27)
    _UPP_ISR_SIFIR(28) _UPP_ISR_HATA(29) _UPP_ISR_HATA(30) _UPP_ISR_SIFIR(31)
    _UPP_ISR_SIFIR(32) _UPP_ISR_SIFIR(33) _UPP_ISR_SIFIR(34) _UPP_ISR_SIFIR(35)
    _UPP_ISR_SIFIR(36) _UPP_ISR_SIFIR(37) _UPP_ISR_SIFIR(38) _UPP_ISR_SIFIR(39)
    _UPP_ISR_SIFIR(40) _UPP_ISR_SIFIR(41) _UPP_ISR_SIFIR(42) _UPP_ISR_SIFIR(43)
    _UPP_ISR_SIFIR(44) _UPP_ISR_SIFIR(45) _UPP_ISR_SIFIR(46) _UPP_ISR_SIFIR(47)
    "_upp_isr_ortak:\n"
    "    push %rax\n    push %rbx\n    push %rcx\n    push %rdx\n"
    "    push %rsi\n    push %rdi\n    push %rbp\n"
    "    push %r8\n    push %r9\n    push %r10\n    push %r11\n"
    "    push %r12\n    push %r13\n    push %r14\n    push %r15\n"
    "    mov %rsp, %rdi\n"
    "    mov %rsp, %rbx\n"
    "    and $-16, %rsp\n"
    "    sub $512, %rsp\n"
    "    fxsave (%rsp)\n"
    "    call _upp_kesme_isle\n"
    "    fxrstor (%rsp)\n"
    "    mov %rbx, %rsp\n"
    "    pop %r15\n    pop %r14\n    pop %r13\n    pop %r12\n"
    "    pop %r11\n    pop %r10\n    pop %r9\n    pop %r8\n"
    "    pop %rbp\n    pop %rdi\n    pop %rsi\n"
    "    pop %rdx\n    pop %rcx\n    pop %rbx\n    pop %rax\n"
    "    add $16, %rsp\n"
    "    iretq\n");

#define _UPP_ISR_BILDIR(n) extern void _upp_isr##n(void);
_UPP_ISR_BILDIR(0) _UPP_ISR_BILDIR(1) _UPP_ISR_BILDIR(2) _UPP_ISR_BILDIR(3)
_UPP_ISR_BILDIR(4) _UPP_ISR_BILDIR(5) _UPP_ISR_BILDIR(6) _UPP_ISR_BILDIR(7)
_UPP_ISR_BILDIR(8) _UPP_ISR_BILDIR(9) _UPP_ISR_BILDIR(10) _UPP_ISR_BILDIR(11)
_UPP_ISR_BILDIR(12) _UPP_ISR_BILDIR(13) _UPP_ISR_BILDIR(14) _UPP_ISR_BILDIR(15)
_UPP_ISR_BILDIR(16) _UPP_ISR_BILDIR(17) _UPP_ISR_BILDIR(18) _UPP_ISR_BILDIR(19)
_UPP_ISR_BILDIR(20) _UPP_ISR_BILDIR(21) _UPP_ISR_BILDIR(22) _UPP_ISR_BILDIR(23)
_UPP_ISR_BILDIR(24) _UPP_ISR_BILDIR(25) _UPP_ISR_BILDIR(26) _UPP_ISR_BILDIR(27)
_UPP_ISR_BILDIR(28) _UPP_ISR_BILDIR(29) _UPP_ISR_BILDIR(30) _UPP_ISR_BILDIR(31)
_UPP_ISR_BILDIR(32) _UPP_ISR_BILDIR(33) _UPP_ISR_BILDIR(34) _UPP_ISR_BILDIR(35)
_UPP_ISR_BILDIR(36) _UPP_ISR_BILDIR(37) _UPP_ISR_BILDIR(38) _UPP_ISR_BILDIR(39)
_UPP_ISR_BILDIR(40) _UPP_ISR_BILDIR(41) _UPP_ISR_BILDIR(42) _UPP_ISR_BILDIR(43)
_UPP_ISR_BILDIR(44) _UPP_ISR_BILDIR(45) _UPP_ISR_BILDIR(46) _UPP_ISR_BILDIR(47)

static void (*const _upp_isr_tablo[48])(void) = {
    _upp_isr0, _upp_isr1, _upp_isr2, _upp_isr3, _upp_isr4, _upp_isr5, _upp_isr6, _upp_isr7,
    _upp_isr8, _upp_isr9, _upp_isr10, _upp_isr11, _upp_isr12, _upp_isr13, _upp_isr14, _upp_isr15,
    _upp_isr16, _upp_isr17, _upp_isr18, _upp_isr19, _upp_isr20, _upp_isr21, _upp_isr22, _upp_isr23,
    _upp_isr24, _upp_isr25, _upp_isr26, _upp_isr27, _upp_isr28, _upp_isr29, _upp_isr30, _upp_isr31,
    _upp_isr32, _upp_isr33, _upp_isr34, _upp_isr35, _upp_isr36, _upp_isr37, _upp_isr38, _upp_isr39,
    _upp_isr40, _upp_isr41, _upp_isr42, _upp_isr43, _upp_isr44, _upp_isr45, _upp_isr46, _upp_isr47,
};

static UppIdtGiris _upp_idt[48] __attribute__((aligned(16)));
static int _upp_kesme_hazir = 0;
static volatile uint64_t _upp_tik = 0;
#define _UPP_TIK_HZ 1000u

#define _UPP_TUS_N 128u
static volatile uint32_t _upp_tus_kuyruk[_UPP_TUS_N];
static volatile unsigned _upp_tus_bas = 0;
static volatile unsigned _upp_tus_son = 0;
static int _upp_tus_shift = 0;

static void _upp_tus_koy(uint32_t c) {
    unsigned s = (_upp_tus_son + 1u) % _UPP_TUS_N;
    if (s != _upp_tus_bas) {
        _upp_tus_kuyruk[_upp_tus_son] = c;
        _upp_tus_son = s;
    }
}

/* PS/2 küme 1 tarama kodları → Türkçe Q düzeni (kod noktası). */
static const uint16_t _upp_trq_normal[0x3A] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '*', '-', 8, '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 0x131, 'o', 'p', 0x11F, 0xFC, '\n', 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', 0x15F, 'i', '"', 0, ',', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', 0xF6, 0xE7, '.', 0, '*', 0, ' ',
};
static const uint16_t _upp_trq_shift[0x3A] = {
    0, 27, '!', '\'', '^', '+', '%', '&', '/', '(', ')', '=', '?', '_', 8, '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 0x11E, 0xDC, '\n', 0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', 0x15E, 0x130, 0xE9, 0, ';', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', 0xD6, 0xC7, ':', 0, '*', 0, ' ',
};

static void _upp_klavye(void) {
    uint8_t sc = _upp_inb(0x60);
    if (sc == 0x2A || sc == 0x36) {
        _upp_tus_shift = 1;
        return;
    }
    if (sc == 0xAA || sc == 0xB6) {
        _upp_tus_shift = 0;
        return;
    }
    if ((sc & 0x80) || sc >= 0x3A) {
        return;
    }
    {
        uint32_t c = _upp_tus_shift ? _upp_trq_shift[sc] : _upp_trq_normal[sc];
        if (c) {
            _upp_tus_koy(c);
        }
    }
}

static const char* _upp_istisna_adi(uint64_t v) {
    switch (v) {
    case 0: return "bölme hatası";
    case 1: return "hata ayıklama";
    case 2: return "NMI";
    case 3: return "kesme noktası";
    case 4: return "taşma";
    case 5: return "sınır aşımı";
    case 6: return "geçersiz işlem kodu";
    case 7: return "FPU yok";
    case 8: return "çift hata";
    case 10: return "geçersiz TSS";
    case 11: return "segment yok";
    case 12: return "yığın segment hatası";
    case 13: return "genel koruma hatası";
    case 14: return "sayfa hatası";
    case 16: return "x87 FPU hatası";
    case 17: return "hizalama denetimi";
    case 18: return "makine denetimi";
    case 19: return "SIMD hatası";
    default: return "ayrılmış istisna";
    }
}

static void _upp_mk_hex(char** s, uint64_t v) {
    char b[18];
    int i;
    b[0] = '0';
    b[1] = 'x';
    for (i = 0; i < 16; i++) {
        unsigned d = (unsigned)((v >> (60 - 4 * i)) & 15u);
        b[2 + i] = (char)(d < 10 ? '0' + d : 'a' + d - 10);
    }
    _upp_m_ekle_n(s, b, 18u, 1);
}

__attribute__((used)) void _upp_kesme_isle(UppKesmeCerceve* c) {
    if (c->vektor < 32) {
        char* m = upp_mk_bas();
        const char* ad = _upp_istisna_adi(c->vektor);
        _upp_m_ekle_n(&m, "işlemci istisnası #", strlen("işlemci istisnası #"), 0);
        upp_mk_sayi(&m, (long long)c->vektor);
        _upp_m_ekle_n(&m, " (", 2u, 1);
        _upp_m_ekle_n(&m, ad, strlen(ad), 0);
        _upp_m_ekle_n(&m, "), RIP=", 7u, 1);
        _upp_mk_hex(&m, c->rip);
        _upp_m_ekle_n(&m, ", hata=", 7u, 1);
        upp_mk_sayi(&m, (long long)c->hata);
        if (c->vektor == 14) {
            uint64_t cr2;
            __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
            _upp_m_ekle_n(&m, ", adres=", 8u, 1);
            _upp_mk_hex(&m, cr2);
        }
        upp_panik(m);
    }
    if (c->vektor == 32) {
        _upp_tik++;
    } else if (c->vektor == 33) {
        _upp_klavye();
    }
    if (c->vektor >= 40) {
        _upp_outb(0xA0, 0x20);
    }
    if (c->vektor >= 32) {
        _upp_outb(0x20, 0x20);
    }
}

static void _upp_io_bekle(void) {
    _upp_outb(0x80, 0);
}

static void upp_cekirdek_kesmeler_kur(void) {
    UppIdtGosterge g;
    unsigned i;
    unsigned bolen;
    if (_upp_kesme_hazir) {
        return;
    }
    _upp_kesme_hazir = 1;
    for (i = 0; i < 48u; i++) {
        uint64_t a = (uint64_t)(uintptr_t)_upp_isr_tablo[i];
        _upp_idt[i].ofs_dusuk = (uint16_t)(a & 0xFFFFu);
        _upp_idt[i].secici = 0x08;
        _upp_idt[i].ist = 0;
        _upp_idt[i].tur = 0x8E;
        _upp_idt[i].ofs_orta = (uint16_t)((a >> 16) & 0xFFFFu);
        _upp_idt[i].ofs_yuksek = (uint32_t)(a >> 32);
        _upp_idt[i].sifir = 0;
    }
    g.sinir = (uint16_t)(sizeof(_upp_idt) - 1u);
    g.taban = (uint64_t)(uintptr_t)_upp_idt;
    __asm__ volatile("lidt %0" : : "m"(g));
    /* PIC: IRQ0-15 → vektör 32-47; yalnızca zamanlayıcı ve klavye açık */
    _upp_outb(0x20, 0x11); _upp_io_bekle();
    _upp_outb(0xA0, 0x11); _upp_io_bekle();
    _upp_outb(0x21, 0x20); _upp_io_bekle();
    _upp_outb(0xA1, 0x28); _upp_io_bekle();
    _upp_outb(0x21, 0x04); _upp_io_bekle();
    _upp_outb(0xA1, 0x02); _upp_io_bekle();
    _upp_outb(0x21, 0x01); _upp_io_bekle();
    _upp_outb(0xA1, 0x01); _upp_io_bekle();
    _upp_outb(0x21, 0xFC);
    _upp_outb(0xA1, 0xFF);
    /* PIT kanal 0, kare dalga, 1000 Hz */
    bolen = 1193182u / _UPP_TIK_HZ;
    _upp_outb(0x43, 0x36);
    _upp_outb(0x40, (uint8_t)(bolen & 0xFFu));
    _upp_outb(0x40, (uint8_t)(bolen >> 8));
    __asm__ volatile("sti");
}

static long long upp_cekirdek_tik(void) {
    return (long long)_upp_tik;
}

/* ---- girdi: PS/2 kuyruğu + COM1 (UTF-8) ---- */
static int _upp_seri_var(void) {
    _upp_seri_kur();
    return (_upp_inb(_UPP_COM1 + 5) & 1u) != 0;
}

static uint32_t _upp_seri_cp(void) {
    uint32_t c = _upp_inb(_UPP_COM1);
    int ek = 0;
    if (c >= 0xF0) { ek = 3; c &= 0x07u; }
    else if (c >= 0xE0) { ek = 2; c &= 0x0Fu; }
    else if (c >= 0xC0) { ek = 1; c &= 0x1Fu; }
    while (ek-- > 0) {
        int bekle = 1000000;
        while (!_upp_seri_var() && bekle-- > 0) {
        }
        c = (c << 6) | (_upp_inb(_UPP_COM1) & 0x3Fu);
    }
    if (c == '\r') {
        c = '\n';
    }
    return c;
}

static long long upp_cekirdek_tus_oku(void) {
    if (_upp_tus_bas != _upp_tus_son) {
        uint32_t c = _upp_tus_kuyruk[_upp_tus_bas];
        _upp_tus_bas = (_upp_tus_bas + 1u) % _UPP_TUS_N;
        return (long long)c;
    }
    if (_upp_seri_var()) {
        return (long long)_upp_seri_cp();
    }
    return 0;
}

static long long upp_cekirdek_tus_bekle(void) {
    upp_cekirdek_kesmeler_kur();
    for (;;) {
        long long c = upp_cekirdek_tus_oku();
        if (c) {
            return c;
        }
        __asm__ volatile("hlt");
    }
}

/* ---- serbest hedefte de var olan yerleşikler: zaman, uyut, giris ---- */
static long long upp_zaman(void) {
    upp_cekirdek_kesmeler_kur();
    return (long long)(_upp_tik * 1000u / _UPP_TIK_HZ);
}

static void upp_uyut(long long ms) {
    uint64_t hedef;
    upp_cekirdek_kesmeler_kur();
    if (ms <= 0) {
        return;
    }
    hedef = _upp_tik + (uint64_t)ms * _UPP_TIK_HZ / 1000u;
    while (_upp_tik < hedef) {
        __asm__ volatile("hlt");
    }
}

/* Satır okur: klavye ya da seri girdi; yankılar, geri silme destekli. */
static char* upp_giris(void) {
    char* s = upp_mk_bas();
    for (;;) {
        long long c = upp_cekirdek_tus_bekle();
        char b[4];
        int n;
        if (c == '\n') {
            _upp_akis_bayt(1, NULL, 0, 1);
            return s;
        }
        if (c == 8 || c == 127) {
            size_t k = upp_metin_nbayt(s);
            if (k > 0) {
                k--;
                while (k > 0 && ((unsigned char)s[k] & 0xC0u) == 0x80u) {
                    k--;
                }
                _upp_m_boy(s, k);
                _upp_akis_bayt(1, "\b \b", 3u, 0);
            }
            continue;
        }
        if (c < 32 && c != '\t') {
            continue;
        }
        n = _upp_utf8_yaz(c, b);
        _upp_m_ekle_n(&s, b, (size_t)n, c < 128);
        _upp_akis_bayt(1, b, (size_t)n, 0);
    }
}
