/* OTOMATİK ÜRETİLDİ (uppc kendini-derle) — düzenlemeyin.
 * Kaynak: upp_on_serbest.c + upp_cekirdek.c + upp_serbest.c + upp_kullanici.c */
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
 *   - sihir: başlığın son alanı, verinin hemen önündeki 4 bayt.
 *
 * Sözleşme: bir metin değeri (değişken, alan, koleksiyon elemanı, parametre)
 * HER ZAMAN yönetilir; başlığı sorgulamadan önce "yönetiliyor mu" diye
 * tahmin edilmez. Ham C dizgileri (argv, getenv, C tamponları, c_kod) metne
 * yalnızca kopyalanarak girer: _upp_m_c / upp_metin_ata_c. Derleyici, c_kod
 * içinde metin değişkenine doğrudan atamayı reddeder. Sihir bir bütünlük
 * denetimidir: tutmazsa (ham işaretçi, serbest bırakılmış metin) panik.
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

/* Yönetilen metnin başlığı; NULL (boş metin) için NULL. */
static UppMetinBas* _upp_mb(const char* s) {
    UppMetinBas* b;
    if (!s) {
        return NULL;
    }
    b = (UppMetinBas*)(void*)(s - sizeof(UppMetinBas));
    if (b->sihir != UPP_METIN_SIHIR) {
        upp_panik("yönetilmeyen metin: başlık yok ya da metin serbest bırakılmış (c_kod içinde ham C dizgisi için upp_metin_ata_c kullanın)");
    }
    return b;
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
        return NULL;
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
    return b ? b->nbayt : 0u;
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
    UppMetinBas* b = _upp_mb(s);
    if (!b) {
        return NULL;
    }
    if (!(b->bayrak & UPP_MB_SABIT)) {
        __atomic_add_fetch(&b->ref, 1, __ATOMIC_RELAXED);
    }
    return (char*)s;
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
        return 0;
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
    if (cp <= 0 || !b) {
        return 0;
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

/* Sözlük sırası (metin < metin): UTF-8 baytları karşılaştırılır; bu, kod noktası
 * sırasıyla aynıdır. Dönüş -1, 0, 1. */
static int upp_metin_karsilastir(const char* a, const char* b) {
    size_t na = a ? upp_metin_nbayt(a) : 0u;
    size_t nb = b ? upp_metin_nbayt(b) : 0u;
    size_t n = na < nb ? na : nb;
    int r = n ? memcmp(a, b, n) : 0;
    if (r != 0) {
        return r < 0 ? -1 : 1;
    }
    return na < nb ? -1 : (na > nb ? 1 : 0);
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

/* c_kod için: ham C dizgisini (NUL ile biten, başlıksız) metne kopyalayarak
 * atar ve eski değeri bırakır. c, *dst'nin içini gösterebilir: önce kopya. */
static void upp_metin_ata_c(char** dst, const char* c) {
    char* yeni;
    if (!dst) {
        return;
    }
    yeni = _upp_m_c(c);
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

/* Kod noktasından tek karakterlik metin; geçersiz kod noktası boş metin verir. */
static char* upp_metin_karakter(long long cp) {
    char b[4];
    if (cp < 0 || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) {
        return _upp_m_bayttan("", 0u);
    }
    return _upp_m_bayttan(b, (size_t)_upp_utf8_yaz(cp, b));
}

static char* upp_metin_tekrar(const char* s, long long n) {
    size_t nb = upp_metin_nbayt(s);
    char* r = upp_mk_bas();
    long long i;
    for (i = 0; i < n; i++) {
        _upp_m_ekle_n(&r, s, nb, 0);
    }
    return r;
}

/* =====================================================================
 * Bayt tamponları (upp.tampon.*): ikili veri (disk sektörleri, dosya
 * sistemi yapıları) için sınır denetimli, tutamaçla erişilen bellek.
 * Tutamaç 1'den başlayan bir sayıdır; geçersiz tutamaç ya da sınır dışı
 * erişim açık panik verir. Çok baytlı okuma/yazma küçük sonludur (LE).
 * ===================================================================== */
typedef struct {
    unsigned char* veri;
    size_t boyut;
} _UppTampon;

static _UppTampon* _upp_tampon = NULL;
static long long _upp_tampon_n = 0;
static long long _upp_tampon_kap = 0;

static _UppTampon* _upp_tampon_al(long long t, long long ofs, long long n) {
    _UppTampon* b;
    if (t < 1 || t > _upp_tampon_n || !_upp_tampon[t - 1].veri) {
        upp_panik("geçersiz tampon tutamacı");
    }
    b = &_upp_tampon[t - 1];
    if (ofs < 0 || n < 0 || (size_t)ofs > b->boyut || (size_t)n > b->boyut - (size_t)ofs) {
        upp_panik("tampon sınırı aşıldı");
    }
    return b;
}

static long long upp_tampon_yeni(long long boyut) {
    long long i;
    unsigned char* v;
    if (boyut < 1 || boyut > 0x40000000LL) {
        upp_panik("tampon boyutu geçersiz");
    }
    v = (unsigned char*)UPP_AYIR((size_t)boyut);
    if (!v) {
        upp_panik("tampon için bellek yetersiz");
    }
    memset(v, 0, (size_t)boyut);
    for (i = 0; i < _upp_tampon_n; i++) {
        if (!_upp_tampon[i].veri) {
            _upp_tampon[i].veri = v;
            _upp_tampon[i].boyut = (size_t)boyut;
            return i + 1;
        }
    }
    if (_upp_tampon_n == _upp_tampon_kap) {
        long long yk = _upp_tampon_kap ? _upp_tampon_kap * 2 : 16;
        _UppTampon* y = (_UppTampon*)UPP_YENIDEN(_upp_tampon, (size_t)yk * sizeof(_UppTampon));
        if (!y) {
            upp_panik("tampon tablosu için bellek yetersiz");
        }
        _upp_tampon = y;
        _upp_tampon_kap = yk;
    }
    _upp_tampon[_upp_tampon_n].veri = v;
    _upp_tampon[_upp_tampon_n].boyut = (size_t)boyut;
    _upp_tampon_n++;
    return _upp_tampon_n;
}

static void upp_tampon_birak(long long t) {
    if (t < 1 || t > _upp_tampon_n || !_upp_tampon[t - 1].veri) {
        return;
    }
    UPP_BIRAK(_upp_tampon[t - 1].veri);
    _upp_tampon[t - 1].veri = NULL;
    _upp_tampon[t - 1].boyut = 0;
}

static long long upp_tampon_boyut(long long t) {
    return (long long)_upp_tampon_al(t, 0, 0)->boyut;
}

static unsigned char* _upp_tampon_p(long long t, long long ofs, long long n) {
    return _upp_tampon_al(t, ofs, n)->veri + ofs;
}

static long long upp_tampon_oku8(long long t, long long ofs) {
    return *_upp_tampon_p(t, ofs, 1);
}

static long long upp_tampon_oku16(long long t, long long ofs) {
    unsigned char* p = _upp_tampon_p(t, ofs, 2);
    return (long long)p[0] | ((long long)p[1] << 8);
}

static long long upp_tampon_oku32(long long t, long long ofs) {
    unsigned char* p = _upp_tampon_p(t, ofs, 4);
    return (long long)((uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
}

static long long upp_tampon_oku64(long long t, long long ofs) {
    unsigned char* p = _upp_tampon_p(t, ofs, 8);
    uint64_t v = 0;
    int i;
    for (i = 7; i >= 0; i--) {
        v = (v << 8) | p[i];
    }
    return (long long)v;
}

static void upp_tampon_yaz8(long long t, long long ofs, long long v) {
    *_upp_tampon_p(t, ofs, 1) = (unsigned char)v;
}

static void upp_tampon_yaz16(long long t, long long ofs, long long v) {
    unsigned char* p = _upp_tampon_p(t, ofs, 2);
    p[0] = (unsigned char)v;
    p[1] = (unsigned char)(v >> 8);
}

static void upp_tampon_yaz32(long long t, long long ofs, long long v) {
    unsigned char* p = _upp_tampon_p(t, ofs, 4);
    p[0] = (unsigned char)v;
    p[1] = (unsigned char)(v >> 8);
    p[2] = (unsigned char)(v >> 16);
    p[3] = (unsigned char)(v >> 24);
}

static void upp_tampon_yaz64(long long t, long long ofs, long long v) {
    unsigned char* p = _upp_tampon_p(t, ofs, 8);
    uint64_t u = (uint64_t)v;
    int i;
    for (i = 0; i < 8; i++) {
        p[i] = (unsigned char)(u >> (8 * i));
    }
}

/* Ham baytlardan metin (UTF-8 olduğu varsayılır). */
static char* upp_tampon_metin(long long t, long long ofs, long long n) {
    return _upp_m_bayttan((const char*)_upp_tampon_p(t, ofs, n), (size_t)n);
}

/* Metnin UTF-8 baytlarını yazar; yazılan bayt sayısını döner. */
static long long upp_tampon_metin_yaz(long long t, long long ofs, const char* s) {
    size_t n = upp_metin_nbayt(s);
    if (n) {
        memcpy(_upp_tampon_p(t, ofs, (long long)n), s, n);
    }
    return (long long)n;
}

static void upp_tampon_kopyala(long long hedef, long long hofs, long long kaynak, long long kofs, long long n) {
    unsigned char* k = _upp_tampon_p(kaynak, kofs, n);
    unsigned char* h = _upp_tampon_p(hedef, hofs, n);
    if (n > 0) {
        memmove(h, k, (size_t)n);
    }
}

static void upp_tampon_doldur(long long t, long long ofs, long long n, long long v) {
    unsigned char* p = _upp_tampon_p(t, ofs, n);
    if (n > 0) {
        memset(p, (int)(v & 0xFF), (size_t)n);
    }
}

/* CRC32C (Castagnoli, yansıtılmış 0x82F63B78); son XOR yok (ext4 biçimi). */
static uint32_t _upp_crc32c_tablo[256];
static int _upp_crc32c_hazir = 0;

static long long upp_tampon_crc32c(long long t, long long ofs, long long n, long long baslangic) {
    unsigned char* p = _upp_tampon_p(t, ofs, n);
    uint32_t c = (uint32_t)baslangic;
    long long i;
    if (!_upp_crc32c_hazir) {
        uint32_t k;
        int j;
        for (k = 0; k < 256; k++) {
            uint32_t r = k;
            for (j = 0; j < 8; j++) {
                r = (r & 1u) ? (r >> 1) ^ 0x82F63B78u : r >> 1;
            }
            _upp_crc32c_tablo[k] = r;
        }
        _upp_crc32c_hazir = 1;
    }
    for (i = 0; i < n; i++) {
        c = _upp_crc32c_tablo[(c ^ p[i]) & 0xFFu] ^ (c >> 8);
    }
    return (long long)c;
}

/* UTF-16LE → metin; en çok n kod birimi, 0x0000'da durur. */
static char* upp_tampon_utf16_metin(long long t, long long ofs, long long n) {
    unsigned char* p = _upp_tampon_p(t, ofs, n * 2);
    char* s = upp_mk_bas();
    long long i;
    for (i = 0; i < n; i++) {
        uint32_t c = (uint32_t)p[2 * i] | ((uint32_t)p[2 * i + 1] << 8);
        char b[4];
        if (c == 0) {
            break;
        }
        if (c >= 0xD800 && c <= 0xDBFF && i + 1 < n) {
            uint32_t d = (uint32_t)p[2 * i + 2] | ((uint32_t)p[2 * i + 3] << 8);
            if (d >= 0xDC00 && d <= 0xDFFF) {
                c = 0x10000u + ((c - 0xD800u) << 10) + (d - 0xDC00u);
                i++;
            }
        }
        _upp_m_ekle_n(&s, b, (size_t)_upp_utf8_yaz((long long)c, b), c < 128);
    }
    return s;
}

/* Metni UTF-16LE olarak yazar (en çok 'sinir' kod birimi); yazılan kod birimi sayısını döner. */
static long long upp_tampon_utf16_yaz(long long t, long long ofs, const char* s, long long sinir) {
    size_t nb = upp_metin_nbayt(s);
    size_t i = 0;
    long long k = 0;
    while (i < nb) {
        long long cp = _upp_utf8_cp_at(s, nb, i);
        i += (size_t)_upp_utf8_adv(s, nb, i);
        if (cp >= 0x10000) {
            if (k + 2 > sinir) {
                break;
            }
            cp -= 0x10000;
            upp_tampon_yaz16(t, ofs + 2 * k, 0xD800 + (cp >> 10));
            upp_tampon_yaz16(t, ofs + 2 * k + 2, 0xDC00 + (cp & 0x3FF));
            k += 2;
        } else {
            if (k + 1 > sinir) {
                break;
            }
            upp_tampon_yaz16(t, ofs + 2 * k, cp);
            k += 1;
        }
    }
    return k;
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

/* İngilizce (ASCII) harf dönüşümü: yalnızca A-Z / a-z değişir; Türkçe harfler
 * ve diğer UTF-8 dizileri (hepsi >= 0x80 baytlardır) olduğu gibi kalır.
 * Dosya adları, protokoller, onaltılık metin gibi yerel ayardan bağımsız işler için. */
static char* _upp_ascii_cevir(const char* s, int buyuk) {
    size_t n, i;
    char* p;
    if (!s) s = UPP_LIT("");
    n = upp_metin_nbayt(s);
    p = _upp_m_yeni(n);
    for (i = 0; i < n; i++) {
        char c = s[i];
        if (buyuk && c >= 'a' && c <= 'z') {
            c = (char)(c - 32);
        } else if (!buyuk && c >= 'A' && c <= 'Z') {
            c = (char)(c + 32);
        }
        p[i] = c;
    }
    return p;
}

static char* upp_metin_ingbuyuk(const char* s) {
    return _upp_ascii_cevir(s, 1);
}

static char* upp_metin_ingkucuk(const char* s) {
    return _upp_ascii_cevir(s, 0);
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
/* Statik ömürlü: koleksiyon tanımlayıcı işaretçisini saklar; bileşik sabit
 * (compound literal) fonksiyon dönünce geçersiz olurdu. */
#define UPP_KT_DUZ(T) (__extension__ ({ static const UppKolTur _upp_kt_duz = { sizeof(T), NULL, NULL }; &_upp_kt_duz; }))

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
        return upp_metin_esit(x ? x : UPP_LIT(""), y ? y : UPP_LIT(""));
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

/* Limine'de tüm fiziksel bellek bu ofsette (HHDM) eşlenir, alt yarı süreçlerindir;
 * Multiboot'ta 0'dır (ilk 1 GiB birebir). */
static uint64_t _upp_hhdm = 0;
static uint64_t _upp_hhdm_son = 0; /* HHDM'nin kapsadığı fiziksel üst sınır */

/* upp.cekirdek.bellek_*: adres fizikseldir (MMIO, VGA); zaten üst yarıdaki bir
 * çekirdek adresi olduğu gibi kullanılır. */
static inline volatile void* _upp_fiz_sanal(long long a) {
    uint64_t u = (uint64_t)a;
    if (u >= 0xffff800000000000ull) {
        return (volatile void*)(uintptr_t)u;
    }
    return (volatile void*)(uintptr_t)(u + _upp_hhdm);
}

static void upp_cekirdek_bellek_yaz8(long long a, long long v) { *(volatile uint8_t*)_upp_fiz_sanal(a) = (uint8_t)v; }
static void upp_cekirdek_bellek_yaz16(long long a, long long v) { *(volatile uint16_t*)_upp_fiz_sanal(a) = (uint16_t)v; }
static void upp_cekirdek_bellek_yaz32(long long a, long long v) { *(volatile uint32_t*)_upp_fiz_sanal(a) = (uint32_t)v; }
static void upp_cekirdek_bellek_yaz64(long long a, long long v) { *(volatile uint64_t*)_upp_fiz_sanal(a) = (uint64_t)v; }
static long long upp_cekirdek_bellek_oku8(long long a) { return *(volatile uint8_t*)_upp_fiz_sanal(a); }
static long long upp_cekirdek_bellek_oku16(long long a) { return *(volatile uint16_t*)_upp_fiz_sanal(a); }
static long long upp_cekirdek_bellek_oku32(long long a) { return *(volatile uint32_t*)_upp_fiz_sanal(a); }
static long long upp_cekirdek_bellek_oku64(long long a) { return (long long)*(volatile uint64_t*)_upp_fiz_sanal(a); }

/* ---- PCI yapılandırma alanı (yapılandırma mekanizması #1: 0xCF8/0xCFC) ----
 * Okuma güvenlidir; olmayan aygıt 0xFFFFFFFF okur. Yazma (BAR, komut yazmacı)
 * belleği yeniden düzenleyebildiğinden 'guvensiz' ister. */
static uint32_t _upp_pci_adres(long long bus, long long aygit, long long islev, long long ofs) {
    return 0x80000000u | ((uint32_t)bus << 16) | ((uint32_t)aygit << 11) | ((uint32_t)islev << 8) | ((uint32_t)ofs & 0xFCu);
}

static int _upp_pci_gecerli(long long bus, long long aygit, long long islev, long long ofs) {
    return bus >= 0 && bus <= 255 && aygit >= 0 && aygit <= 31 && islev >= 0 && islev <= 7 && ofs >= 0 && ofs <= 255;
}

static long long upp_cekirdek_pci_oku(long long bus, long long aygit, long long islev, long long ofs) {
    if (!_upp_pci_gecerli(bus, aygit, islev, ofs)) {
        return 0xFFFFFFFFLL;
    }
    _upp_outl(0xCF8, _upp_pci_adres(bus, aygit, islev, ofs));
    return (long long)_upp_inl(0xCFC);
}

static void upp_cekirdek_pci_yaz(long long bus, long long aygit, long long islev, long long ofs, long long deger) {
    if (!_upp_pci_gecerli(bus, aygit, islev, ofs)) {
        return;
    }
    _upp_outl(0xCF8, _upp_pci_adres(bus, aygit, islev, ofs));
    _upp_outl(0xCFC, (uint32_t)deger);
}

/* ---- MMIO pencereleri (FreeBSD bus_space benzeri) ----
 * mmio_ac bir fiziksel aralığı ('guvensiz') tutamaçla açar; okuma/yazma güvenli
 * yoldan, sınır ve hizalama denetimiyle yapılır. Erişim HHDM (Limine) ya da
 * ilk 1 GiB'ın birebir eşlemesi (Multiboot) üzerindendir; kapsanmayan aralık 0
 * döndürür. Gerçek donanımda MMIO deliği MTRR ile önbelleksizdir. */
#define _UPP_MMIO_N 64
static uint64_t _upp_mmio_fiz[_UPP_MMIO_N], _upp_mmio_boy[_UPP_MMIO_N];
static int _upp_mmio_n = 0;

static long long upp_cekirdek_mmio_ac(long long fiz, long long boyut) {
    uint64_t f = (uint64_t)fiz, b = (uint64_t)boyut;
    uint64_t ust = _upp_hhdm ? _upp_hhdm_son : 0x40000000ull;
    int i;
    if (fiz <= 0 || boyut <= 0 || b > 0x40000000ull || f + b < f || f + b > ust) {
        return 0;
    }
    for (i = 0; i < _upp_mmio_n; i++) {
        if (_upp_mmio_fiz[i] == f && _upp_mmio_boy[i] == b) {
            return i + 1;
        }
    }
    if (_upp_mmio_n >= _UPP_MMIO_N) {
        return 0;
    }
    _upp_mmio_fiz[_upp_mmio_n] = f;
    _upp_mmio_boy[_upp_mmio_n] = b;
    _upp_mmio_n++;
    return _upp_mmio_n;
}

static volatile unsigned char* _upp_mmio_p(long long h, long long ofs, unsigned n) {
    if (h < 1 || h > _upp_mmio_n) {
        upp_panik("geçersiz MMIO tutamacı");
    }
    if (ofs < 0 || (uint64_t)ofs + n > _upp_mmio_boy[h - 1] || ((uint64_t)ofs & (n - 1u))) {
        upp_panik("MMIO sınırı ya da hizalaması aşıldı");
    }
    return (volatile unsigned char*)(uintptr_t)(_upp_mmio_fiz[h - 1] + (uint64_t)ofs + _upp_hhdm);
}

static long long upp_cekirdek_mmio_oku8(long long h, long long ofs) { return *(volatile uint8_t*)_upp_mmio_p(h, ofs, 1); }
static long long upp_cekirdek_mmio_oku16(long long h, long long ofs) { return *(volatile uint16_t*)_upp_mmio_p(h, ofs, 2); }
static long long upp_cekirdek_mmio_oku32(long long h, long long ofs) { return *(volatile uint32_t*)_upp_mmio_p(h, ofs, 4); }
static void upp_cekirdek_mmio_yaz8(long long h, long long ofs, long long v) { *(volatile uint8_t*)_upp_mmio_p(h, ofs, 1) = (uint8_t)v; }
static void upp_cekirdek_mmio_yaz16(long long h, long long ofs, long long v) { *(volatile uint16_t*)_upp_mmio_p(h, ofs, 2) = (uint16_t)v; }
static void upp_cekirdek_mmio_yaz32(long long h, long long ofs, long long v) { *(volatile uint32_t*)_upp_mmio_p(h, ofs, 4) = (uint32_t)v; }

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

static unsigned char _upp_cp437(long long cp);

/* =====================================================================
 * Ekran konsolu: VGA metin kipi (0xB8000, 80x25) ya da çerçeve tamponu
 * (Limine ile açılınca; 8x16 Terminus yazı tipiyle). İki arka uç da aynı
 * hücre modelini (kod noktası + VGA renk özniteliği) kullanır; ANSI
 * çözümleyici ve imleç ortaktır. Seri terminal baytları kendisi yorumlar.
 * ===================================================================== */
/* VGA metin belleği: Multiboot'ta birebir, Limine'de HHDM üzerinden (bkz. _upp_limine_kur). */
static volatile uint16_t* _upp_vga_taban = (volatile uint16_t*)(uintptr_t)0xB8000;
#define _UPP_VGA _upp_vga_taban
#define _UPP_FB_HUCRE_MAX 32768

/* Terminus Font 8x16 (Lat15: Latin-1/5/9, Türkçe dahil), console-setup Lat15-Terminus16.psf.
 * Copyright (c) 2010 Dimitar Toshkov Zhekov, with Reserved Font Name "Terminus Font".
 * SIL Open Font License 1.1 — tam metin: src/runtime/TERMINUS_LISANS.txt */
static const uint8_t _upp_yazi[256][16] = {
    {0x00, 0x00, 0x00, 0x7e, 0x81, 0x99, 0xa5, 0xa1, 0xa5, 0x99, 0x81, 0x7e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x7e, 0x81, 0xb9, 0xa5, 0xb9, 0xa9, 0xa5, 0x81, 0x7e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3c, 0x43, 0x42, 0x46, 0x4a, 0x52, 0x62, 0x42, 0xc2, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0xfb, 0x55, 0x55, 0x51, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x10, 0x38, 0x7c, 0xfe, 0x7c, 0x38, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x44, 0x38, 0x44, 0x44, 0x44, 0x38, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x3c, 0x3c, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x38, 0x04, 0x18, 0x04, 0x04, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x08, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x20, 0x00},
    {0x00, 0x10, 0x30, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0xe0, 0x10, 0x60, 0x12, 0xe4, 0x08, 0x10, 0x22, 0x46, 0x8a, 0x1e, 0x02, 0x02, 0x00, 0x00},
    {0x10, 0x08, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x42, 0x42, 0x00, 0x00, 0x00, 0x00},
    {0x08, 0x10, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x42, 0x42, 0x00, 0x00, 0x00, 0x00},
    {0x18, 0x24, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x42, 0x42, 0x00, 0x00, 0x00, 0x00},
    {0x32, 0x4c, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x42, 0x42, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x00, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x7e, 0x92, 0x92, 0x92, 0x92, 0x72, 0x12, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x38, 0x44, 0x40, 0x30, 0x48, 0x44, 0x44, 0x24, 0x18, 0x04, 0x44, 0x38, 0x00, 0x00, 0x00},
    {0x10, 0x08, 0x00, 0x7e, 0x40, 0x40, 0x40, 0x78, 0x40, 0x40, 0x40, 0x7e, 0x00, 0x00, 0x00, 0x00},
    {0x18, 0x24, 0x00, 0x7e, 0x40, 0x40, 0x40, 0x78, 0x40, 0x40, 0x40, 0x7e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x10, 0x38, 0x54, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x54, 0x38, 0x10, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x04, 0xfe, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x40, 0xfe, 0x40, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x24, 0x24, 0x00, 0x7e, 0x40, 0x40, 0x40, 0x78, 0x40, 0x40, 0x40, 0x7e, 0x00, 0x00, 0x00, 0x00},
    {0x20, 0x10, 0x00, 0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x00},
    {0x08, 0x10, 0x00, 0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x00},
    {0x18, 0x24, 0x00, 0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x24, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x24, 0x24, 0x24, 0x7e, 0x24, 0x24, 0x7e, 0x24, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x10, 0x10, 0x7c, 0x92, 0x90, 0x90, 0x7c, 0x12, 0x12, 0x92, 0x7c, 0x10, 0x10, 0x00, 0x00},
    {0x00, 0x00, 0x64, 0x94, 0x68, 0x08, 0x10, 0x10, 0x20, 0x2c, 0x52, 0x4c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x18, 0x24, 0x24, 0x18, 0x30, 0x4a, 0x44, 0x44, 0x44, 0x3a, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x08, 0x10, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x20, 0x10, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x18, 0x7e, 0x18, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x7c, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x20, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3c, 0x42, 0x42, 0x46, 0x4a, 0x52, 0x62, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x08, 0x18, 0x28, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x3e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3c, 0x42, 0x42, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x7e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3c, 0x42, 0x42, 0x02, 0x1c, 0x02, 0x02, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x02, 0x06, 0x0a, 0x12, 0x22, 0x42, 0x7e, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x7e, 0x40, 0x40, 0x40, 0x7c, 0x02, 0x02, 0x02, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x1c, 0x20, 0x40, 0x40, 0x7c, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x7e, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x3e, 0x02, 0x02, 0x04, 0x38, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00, 0x10, 0x10, 0x20, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x04, 0x08, 0x10, 0x20, 0x40, 0x20, 0x10, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x40, 0x20, 0x10, 0x08, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x04, 0x08, 0x08, 0x00, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x7c, 0x82, 0x9e, 0xa2, 0xa2, 0xa2, 0xa6, 0x9a, 0x80, 0x7e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x42, 0x42, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x7c, 0x42, 0x42, 0x42, 0x7c, 0x42, 0x42, 0x42, 0x42, 0x7c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3c, 0x42, 0x42, 0x40, 0x40, 0x40, 0x40, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x78, 0x44, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x44, 0x78, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x7e, 0x40, 0x40, 0x40, 0x78, 0x40, 0x40, 0x40, 0x40, 0x7e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x7e, 0x40, 0x40, 0x40, 0x78, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3c, 0x42, 0x42, 0x40, 0x40, 0x4e, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x42, 0x42, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x42, 0x42, 0x42, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x0e, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x42, 0x44, 0x48, 0x50, 0x60, 0x60, 0x50, 0x48, 0x44, 0x42, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x82, 0xc6, 0xaa, 0x92, 0x92, 0x82, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x42, 0x42, 0x42, 0x62, 0x52, 0x4a, 0x46, 0x42, 0x42, 0x42, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x7c, 0x42, 0x42, 0x42, 0x42, 0x7c, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x4a, 0x3c, 0x02, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x7c, 0x42, 0x42, 0x42, 0x42, 0x7c, 0x50, 0x48, 0x44, 0x42, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3c, 0x42, 0x40, 0x40, 0x3c, 0x02, 0x02, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0xfe, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x42, 0x42, 0x42, 0x42, 0x42, 0x24, 0x24, 0x24, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x82, 0x82, 0x82, 0x82, 0x82, 0x92, 0x92, 0xaa, 0xc6, 0x82, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x42, 0x42, 0x24, 0x24, 0x18, 0x18, 0x24, 0x24, 0x42, 0x42, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x82, 0x82, 0x44, 0x44, 0x28, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x7e, 0x02, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x40, 0x7e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x38, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x38, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x40, 0x40, 0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x38, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x10, 0x28, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00},
    {0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x02, 0x3e, 0x42, 0x42, 0x42, 0x3e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x40, 0x40, 0x40, 0x7c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x7c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x42, 0x40, 0x40, 0x40, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x02, 0x02, 0x02, 0x3e, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x42, 0x42, 0x7e, 0x40, 0x40, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x0e, 0x10, 0x10, 0x7c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3e, 0x02, 0x02, 0x3c, 0x00},
    {0x00, 0x00, 0x40, 0x40, 0x40, 0x7c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x10, 0x10, 0x00, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x04, 0x04, 0x00, 0x0c, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x44, 0x44, 0x38, 0x00},
    {0x00, 0x00, 0x40, 0x40, 0x40, 0x42, 0x44, 0x48, 0x70, 0x48, 0x44, 0x42, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x92, 0x92, 0x92, 0x92, 0x92, 0x92, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x7c, 0x40, 0x40, 0x40, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3e, 0x02, 0x02, 0x02, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x5e, 0x60, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x40, 0x40, 0x3c, 0x02, 0x02, 0x7c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x10, 0x10, 0x10, 0x7c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x42, 0x42, 0x24, 0x24, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x82, 0x92, 0x92, 0x92, 0x92, 0x7c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x42, 0x24, 0x18, 0x24, 0x42, 0x42, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3e, 0x02, 0x02, 0x3c, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x04, 0x08, 0x10, 0x20, 0x40, 0x7e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x0c, 0x10, 0x10, 0x10, 0x20, 0x10, 0x10, 0x10, 0x10, 0x0c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x30, 0x08, 0x08, 0x08, 0x04, 0x08, 0x08, 0x08, 0x08, 0x30, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x62, 0x92, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x44, 0x44, 0x00, 0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3c, 0x42, 0x42, 0x40, 0x40, 0x40, 0x40, 0x42, 0x42, 0x3c, 0x10, 0x10, 0x20, 0x00},
    {0x00, 0x00, 0x24, 0x24, 0x00, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x08, 0x10, 0x00, 0x3c, 0x42, 0x42, 0x7e, 0x40, 0x40, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x18, 0x24, 0x00, 0x3c, 0x02, 0x3e, 0x42, 0x42, 0x42, 0x3e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x24, 0x24, 0x00, 0x3c, 0x02, 0x3e, 0x42, 0x42, 0x42, 0x3e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x10, 0x08, 0x00, 0x3c, 0x02, 0x3e, 0x42, 0x42, 0x42, 0x3e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x18, 0x24, 0x18, 0x3c, 0x02, 0x3e, 0x42, 0x42, 0x42, 0x3e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x42, 0x40, 0x40, 0x40, 0x42, 0x3c, 0x10, 0x10, 0x20, 0x00},
    {0x00, 0x00, 0x18, 0x24, 0x00, 0x3c, 0x42, 0x42, 0x7e, 0x40, 0x40, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x24, 0x24, 0x00, 0x3c, 0x42, 0x42, 0x7e, 0x40, 0x40, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x10, 0x08, 0x00, 0x3c, 0x42, 0x42, 0x7e, 0x40, 0x40, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x48, 0x48, 0x00, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x30, 0x48, 0x00, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x20, 0x10, 0x00, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x00},
    {0x24, 0x24, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x42, 0x42, 0x00, 0x00, 0x00, 0x00},
    {0x18, 0x24, 0x18, 0x3c, 0x42, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x42, 0x42, 0x00, 0x00, 0x00, 0x00},
    {0x08, 0x10, 0x00, 0x7e, 0x40, 0x40, 0x40, 0x78, 0x40, 0x40, 0x40, 0x7e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x6c, 0x12, 0x72, 0x9e, 0x90, 0x90, 0x6c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x7e, 0x90, 0x90, 0x90, 0xfc, 0x90, 0x90, 0x90, 0x90, 0x9e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x18, 0x24, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x24, 0x24, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x10, 0x08, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x18, 0x24, 0x00, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x10, 0x08, 0x00, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x24, 0x24, 0x00, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3e, 0x02, 0x02, 0x3c, 0x00},
    {0x24, 0x24, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x24, 0x24, 0x00, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x10, 0x10, 0x7c, 0x92, 0x90, 0x90, 0x90, 0x92, 0x7c, 0x10, 0x10, 0x00, 0x00},
    {0x00, 0x00, 0x18, 0x24, 0x20, 0x20, 0x78, 0x20, 0x20, 0x20, 0x22, 0x7e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x82, 0x82, 0x44, 0x28, 0x10, 0x7c, 0x10, 0x7c, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0xf0, 0x88, 0x88, 0x88, 0xf4, 0x84, 0x8e, 0x84, 0x84, 0x82, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x0c, 0x12, 0x10, 0x10, 0x7c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x90, 0x60, 0x00},
    {0x00, 0x00, 0x08, 0x10, 0x00, 0x3c, 0x02, 0x3e, 0x42, 0x42, 0x42, 0x3e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x08, 0x10, 0x00, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x08, 0x10, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x08, 0x10, 0x00, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x32, 0x4c, 0x00, 0x7c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x00, 0x00, 0x00, 0x00},
    {0x32, 0x4c, 0x00, 0x42, 0x42, 0x62, 0x52, 0x4a, 0x46, 0x42, 0x42, 0x42, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x38, 0x04, 0x3c, 0x44, 0x3c, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x38, 0x44, 0x44, 0x44, 0x38, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x10, 0x10, 0x00, 0x10, 0x10, 0x20, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x78, 0x44, 0x42, 0x42, 0xf2, 0x42, 0x42, 0x42, 0x44, 0x78, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x20, 0x60, 0x20, 0x22, 0x24, 0x08, 0x10, 0x20, 0x4c, 0x92, 0x04, 0x08, 0x1e, 0x00, 0x00},
    {0x00, 0x20, 0x60, 0x20, 0x22, 0x24, 0x08, 0x10, 0x22, 0x46, 0x8a, 0x1e, 0x02, 0x02, 0x00, 0x00},
    {0x00, 0x00, 0x10, 0x10, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x24, 0x48, 0x90, 0x48, 0x24, 0x12, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x48, 0x24, 0x12, 0x24, 0x48, 0x90, 0x00, 0x00, 0x00, 0x00},
    {0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22},
    {0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55},
    {0x10, 0x08, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0xf0, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
    {0x08, 0x10, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x18, 0x24, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x32, 0x4c, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x10, 0x08, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x08, 0x10, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x18, 0x24, 0x00, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x08, 0x10, 0x82, 0x82, 0x44, 0x44, 0x28, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x40, 0x40, 0x7c, 0x42, 0x42, 0x42, 0x42, 0x7c, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x32, 0x4c, 0x00, 0x3c, 0x02, 0x3e, 0x42, 0x42, 0x42, 0x3e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0xff, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
    {0x00, 0x00, 0x28, 0x10, 0x28, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x32, 0x4c, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x02, 0x3c, 0x46, 0x4a, 0x52, 0x62, 0x42, 0xbc, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x08, 0x10, 0x00, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3e, 0x02, 0x02, 0x3c, 0x00},
    {0x00, 0x00, 0x40, 0x40, 0x40, 0x7c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x7c, 0x40, 0x40, 0x40, 0x00},
    {0x24, 0x18, 0x00, 0x3c, 0x42, 0x40, 0x40, 0x4e, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x24, 0x18, 0x00, 0x3e, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3e, 0x02, 0x02, 0x3c, 0x00},
    {0x10, 0x10, 0x00, 0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x7e, 0x90, 0x90, 0x90, 0x9c, 0x90, 0x90, 0x90, 0x90, 0x7e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x92, 0x92, 0x9e, 0x90, 0x90, 0x7c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3c, 0x42, 0x40, 0x40, 0x3c, 0x02, 0x02, 0x42, 0x42, 0x3c, 0x10, 0x10, 0x20, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x40, 0x40, 0x3c, 0x02, 0x02, 0x7c, 0x10, 0x10, 0x20, 0x00},
    {0x24, 0x18, 0x00, 0x3c, 0x42, 0x40, 0x40, 0x3c, 0x02, 0x02, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x24, 0x18, 0x00, 0x3e, 0x40, 0x40, 0x3c, 0x02, 0x02, 0x7c, 0x00, 0x00, 0x00, 0x00},
    {0x44, 0x44, 0x00, 0x82, 0x82, 0x44, 0x44, 0x28, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00},
    {0x24, 0x18, 0x00, 0x7e, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x40, 0x7e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x24, 0x18, 0x00, 0x7e, 0x04, 0x08, 0x10, 0x20, 0x40, 0x7e, 0x00, 0x00, 0x00, 0x00},
    {0x18, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    {0x32, 0x4c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x08, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x08, 0x08, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x10, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x38, 0x44, 0x44, 0x48, 0x7c, 0x42, 0x42, 0x42, 0x62, 0x5c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x12, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x12, 0x12, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x24, 0x48, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x42, 0x42, 0x42, 0x42, 0x46, 0x7a, 0x40, 0x40, 0x40, 0x00},
    {0x00, 0x00, 0x10, 0x10, 0x7c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x10, 0x10, 0x7c, 0x10, 0x10, 0x10, 0x10, 0x7c, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x92, 0x92, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x48, 0xa8, 0x50, 0x10, 0x20, 0x20, 0x40, 0x54, 0xaa, 0x94, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x08, 0x10, 0x20, 0x10, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x92, 0x92, 0x92, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x10, 0x08, 0x04, 0x08, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x1c, 0x22, 0x40, 0xf8, 0x40, 0xf8, 0x40, 0x22, 0x1c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x7c, 0x10, 0x10, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x20, 0x10, 0x08, 0x04, 0x08, 0x10, 0x20, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x04, 0x08, 0x10, 0x20, 0x10, 0x08, 0x04, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x02, 0x7e, 0x08, 0x10, 0x7e, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x96, 0x96, 0x96, 0xd0, 0xf0, 0xf0, 0xb0, 0x96, 0x90, 0x96, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x00, 0x7c, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x4c, 0x00, 0x32, 0x4c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x18, 0x24, 0x24, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0xfe, 0x10, 0x10, 0x1c, 0x70, 0x1c, 0x70, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x1e, 0x20, 0x40, 0x40, 0x7e, 0x40, 0x40, 0x20, 0x1e, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x38, 0x24, 0x24, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x18, 0x24, 0x04, 0x08, 0x10, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x02, 0x04, 0x7c, 0x8a, 0x92, 0x92, 0xa2, 0x7c, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00},
};
static const uint16_t _upp_yazi_kod[528] = {
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b,
    0x002c, 0x002d, 0x002e, 0x002f, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f, 0x0040, 0x0041, 0x0042, 0x0043,
    0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b,
    0x005c, 0x005d, 0x005e, 0x005f, 0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
    0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f, 0x0070, 0x0071, 0x0072, 0x0073,
    0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x00a0,
    0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac,
    0x00ad, 0x00ae, 0x00af, 0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8,
    0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf, 0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4,
    0x00c5, 0x00c6, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf, 0x00d0,
    0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc,
    0x00dd, 0x00de, 0x00df, 0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8,
    0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef, 0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4,
    0x00f5, 0x00f6, 0x00f7, 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff, 0x0110,
    0x011e, 0x011f, 0x0130, 0x0131, 0x0152, 0x0153, 0x015e, 0x015f, 0x0160, 0x0161, 0x0178, 0x017d,
    0x017e, 0x0192, 0x02c6, 0x02c9, 0x02dc, 0x0391, 0x0392, 0x0395, 0x0396, 0x0397, 0x0399, 0x039a,
    0x039c, 0x039d, 0x039f, 0x03a1, 0x03a4, 0x03a7, 0x03bc, 0x03c0, 0x0401, 0x0405, 0x0406, 0x0407,
    0x0408, 0x0410, 0x0412, 0x0415, 0x041a, 0x041c, 0x041d, 0x041e, 0x0420, 0x0421, 0x0422, 0x0425,
    0x0430, 0x0435, 0x043e, 0x0440, 0x0441, 0x0443, 0x0445, 0x0451, 0x0455, 0x0456, 0x0457, 0x0458,
    0x04ae, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008, 0x2009, 0x200a,
    0x2010, 0x2011, 0x2012, 0x2013, 0x2014, 0x2015, 0x2018, 0x2019, 0x201a, 0x201c, 0x201d, 0x201e,
    0x2020, 0x2021, 0x2022, 0x2026, 0x202f, 0x2030, 0x2039, 0x203a, 0x203c, 0x207f, 0x20a7, 0x20ac,
    0x20ae, 0x2116, 0x2122, 0x212a, 0x212b, 0x2190, 0x2191, 0x2192, 0x2193, 0x2205, 0x2208, 0x2212,
    0x221e, 0x2229, 0x2248, 0x2260, 0x2261, 0x2264, 0x2265, 0x226a, 0x226b, 0x2295, 0x2296, 0x2298,
    0x2299, 0x229b, 0x229c, 0x2460, 0x2461, 0x2462, 0x2463, 0x2464, 0x2465, 0x2466, 0x2467, 0x2468,
    0x24b6, 0x24b7, 0x24b8, 0x24b9, 0x24ba, 0x24bb, 0x24bc, 0x24bd, 0x24be, 0x24bf, 0x24c0, 0x24c1,
    0x24c2, 0x24c3, 0x24c4, 0x24c5, 0x24c6, 0x24c7, 0x24c8, 0x24c9, 0x24ca, 0x24cb, 0x24cc, 0x24cd,
    0x24ce, 0x24cf, 0x24d0, 0x24d1, 0x24d2, 0x24d3, 0x24d4, 0x24d5, 0x24d6, 0x24d7, 0x24d8, 0x24d9,
    0x24da, 0x24db, 0x24dc, 0x24dd, 0x24de, 0x24df, 0x24e0, 0x24e1, 0x24e2, 0x24e3, 0x24e4, 0x24e5,
    0x24e6, 0x24e7, 0x24e8, 0x24e9, 0x24ea, 0x2500, 0x2501, 0x2502, 0x2503, 0x2504, 0x2505, 0x2506,
    0x2507, 0x2508, 0x2509, 0x250a, 0x250b, 0x250c, 0x250d, 0x250e, 0x250f, 0x2510, 0x2511, 0x2512,
    0x2513, 0x2514, 0x2515, 0x2516, 0x2517, 0x2518, 0x2519, 0x251a, 0x251b, 0x251c, 0x251d, 0x251e,
    0x251f, 0x2520, 0x2521, 0x2522, 0x2523, 0x2524, 0x2525, 0x2526, 0x2527, 0x2528, 0x2529, 0x252a,
    0x252b, 0x252c, 0x252d, 0x252e, 0x252f, 0x2530, 0x2531, 0x2532, 0x2533, 0x2534, 0x2535, 0x2536,
    0x2537, 0x2538, 0x2539, 0x253a, 0x253b, 0x253c, 0x253d, 0x253e, 0x253f, 0x2540, 0x2541, 0x2542,
    0x2543, 0x2544, 0x2545, 0x2546, 0x2547, 0x2548, 0x2549, 0x254a, 0x254b, 0x254c, 0x254d, 0x254e,
    0x254f, 0x2550, 0x2551, 0x2552, 0x2553, 0x2554, 0x2555, 0x2556, 0x2557, 0x2558, 0x2559, 0x255a,
    0x255b, 0x255c, 0x255d, 0x255e, 0x255f, 0x2560, 0x2561, 0x2562, 0x2563, 0x2564, 0x2565, 0x2566,
    0x2567, 0x2568, 0x2569, 0x256a, 0x256b, 0x256c, 0x256d, 0x256e, 0x256f, 0x2570, 0x2571, 0x2572,
    0x2573, 0x2574, 0x2575, 0x2576, 0x2577, 0x2578, 0x2579, 0x257a, 0x257b, 0x257c, 0x257d, 0x257e,
    0x257f, 0x2588, 0x2591, 0x2592, 0x25a0, 0x25ae, 0x25b2, 0x25b4, 0x25b6, 0x25b8, 0x25bc, 0x25be,
    0x25c0, 0x25c2, 0x25c8, 0x25cf, 0x2610, 0x2611, 0x2612, 0x2666, 0x2717, 0x2718, 0x2bbd, 0xfffd,
};
static const uint8_t _upp_yazi_glif[528] = {
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
    96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 32,
    173, 155, 156, 5, 157, 6, 21, 8, 0, 166, 174, 170, 45, 1, 9, 248,
    241, 253, 10, 11, 230, 20, 250, 12, 13, 167, 175, 172, 171, 14, 168, 15,
    16, 17, 18, 142, 143, 146, 128, 22, 144, 23, 28, 29, 30, 31, 127, 169,
    165, 178, 181, 182, 183, 153, 184, 2, 185, 186, 187, 154, 188, 189, 225, 133,
    160, 131, 190, 132, 134, 145, 135, 138, 130, 136, 137, 141, 161, 140, 139, 198,
    164, 149, 162, 147, 199, 148, 246, 200, 151, 163, 150, 129, 201, 202, 152, 169,
    203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 159, 216, 9,
    220, 65, 66, 69, 90, 72, 73, 75, 77, 78, 79, 80, 84, 88, 230, 227,
    28, 83, 73, 127, 74, 65, 66, 69, 75, 77, 72, 79, 80, 67, 84, 88,
    97, 101, 111, 112, 99, 121, 120, 137, 115, 105, 139, 106, 89, 32, 32, 32,
    32, 32, 32, 32, 32, 32, 32, 32, 45, 45, 45, 45, 221, 221, 222, 223,
    224, 226, 228, 229, 231, 232, 7, 233, 32, 234, 235, 237, 19, 252, 158, 238,
    249, 245, 3, 75, 143, 27, 24, 26, 25, 255, 251, 45, 236, 239, 247, 244,
    240, 243, 242, 174, 175, 43, 45, 47, 46, 42, 61, 49, 50, 51, 52, 53,
    54, 55, 56, 57, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76,
    77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 97, 98,
    99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114,
    115, 116, 117, 118, 119, 120, 121, 122, 48, 196, 196, 179, 179, 196, 196, 179,
    179, 196, 196, 179, 179, 218, 218, 218, 218, 191, 191, 191, 191, 192, 192, 192,
    192, 217, 217, 217, 217, 195, 195, 195, 195, 195, 195, 195, 195, 180, 180, 180,
    180, 180, 180, 180, 180, 194, 194, 194, 194, 194, 194, 194, 194, 193, 193, 193,
    193, 193, 193, 193, 193, 197, 197, 197, 197, 197, 197, 197, 197, 197, 197, 197,
    197, 197, 197, 197, 197, 196, 196, 179, 179, 196, 179, 218, 218, 218, 191, 191,
    191, 192, 192, 192, 217, 217, 217, 195, 195, 195, 180, 180, 180, 194, 194, 194,
    193, 193, 193, 197, 197, 197, 218, 191, 217, 192, 47, 92, 88, 196, 179, 196,
    179, 196, 179, 196, 179, 196, 179, 196, 179, 219, 176, 177, 254, 254, 24, 24,
    26, 26, 25, 25, 27, 27, 4, 7, 46, 118, 120, 4, 120, 120, 120, 4,
};
#define _UPP_YAZI_KOD_N 528u

static int _upp_ekr_g = 80;
static int _upp_ekr_y = 25;
static int _upp_ekr_x = 0;
static int _upp_ekr_s = 0;
/* Ertelenmiş satır sarma (VT100): son sütuna yazınca imleç orada kalır; satır
 * ancak sonraki yazdırılabilir karakterde sarılır. Böylece sağ alt hücreye
 * yazmak ekranı kaydırmaz ve tam genişlikte satırın ardından gelen \n boş satır
 * bırakmaz. İmleci taşıyan her işlem bekleyen sarmayı iptal eder. */
static int _upp_ekr_sarma = 0;

/* çerçeve tamponu */
static int _upp_fb_aktif = 0;
static volatile uint8_t* _upp_fb = NULL;
static uint32_t _upp_fb_gen = 0, _upp_fb_yuk = 0, _upp_fb_adim = 0, _upp_fb_bpp = 0;
static uint8_t _upp_fb_rk = 8, _upp_fb_rs = 16, _upp_fb_gk = 8, _upp_fb_gs = 8, _upp_fb_bk = 8, _upp_fb_bs = 0;
static uint32_t _upp_fb_hucreler[_UPP_FB_HUCRE_MAX];
static int _upp_fb_imlec_x = -1, _upp_fb_imlec_y = -1;

static const uint32_t _upp_vga_rgb[16] = {
    0x000000, 0x0000AA, 0x00AA00, 0x00AAAA, 0xAA0000, 0xAA00AA, 0xAA5500, 0xAAAAAA,
    0x555555, 0x5555FF, 0x55FF55, 0x55FFFF, 0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF,
};

static uint32_t _upp_fb_piksel(uint32_t rgb) {
    uint32_t r = (rgb >> 16) & 0xFF, g = (rgb >> 8) & 0xFF, b = rgb & 0xFF;
    return ((r >> (8 - _upp_fb_rk)) << _upp_fb_rs) | ((g >> (8 - _upp_fb_gk)) << _upp_fb_gs) | ((b >> (8 - _upp_fb_bk)) << _upp_fb_bs);
}

static int _upp_yazi_bul(long long cp) {
    int alt = 0, ust = (int)_UPP_YAZI_KOD_N - 1;
    if (cp < 0 || cp > 0xFFFF) {
        return -1;
    }
    while (alt <= ust) {
        int orta = (alt + ust) / 2;
        if (_upp_yazi_kod[orta] == cp) {
            return _upp_yazi_glif[orta];
        }
        if (_upp_yazi_kod[orta] < cp) {
            alt = orta + 1;
        } else {
            ust = orta - 1;
        }
    }
    return -1;
}

static void _upp_fb_ciz(int x, int y, uint32_t hucre, int imlec) {
    long long cp = (long long)(hucre & 0xFFFFFFu);
    uint8_t renk = (uint8_t)(hucre >> 24);
    uint32_t on = _upp_fb_piksel(_upp_vga_rgb[renk & 15]);
    uint32_t arka = _upp_fb_piksel(_upp_vga_rgb[(renk >> 4) & 15]);
    int g = _upp_yazi_bul(cp);
    int sx, sy;
    uint32_t bpp = _upp_fb_bpp / 8;
    /* Yazı tipinde olmayan gölge blokları (░ ▒ ▓) CP437'deki desenleriyle çizilir. */
    int golge = g < 0 && cp >= 0x2591 && cp <= 0x2593 ? (int)(cp - 0x2590) : 0;
    if (g < 0) {
        g = _upp_yazi_bul('?');
    }
    if (g < 0) {
        g = 0;
    }
    for (sy = 0; sy < 16; sy++) {
        uint8_t bit = _upp_yazi[g][sy];
        if (golge == 1) {
            bit = (sy & 1) ? 0x22 : 0x88;
        } else if (golge == 2) {
            bit = (sy & 1) ? 0x55 : 0xAA;
        } else if (golge == 3) {
            bit = (sy & 1) ? 0xDD : 0x77;
        }
        volatile uint8_t* satir = _upp_fb + (size_t)(y * 16 + sy) * _upp_fb_adim + (size_t)x * 8u * bpp;
        if (imlec && sy >= 14) {
            bit = 0xFF;
        }
        for (sx = 0; sx < 8; sx++) {
            uint32_t p = (bit & (0x80u >> sx)) ? on : arka;
            volatile uint8_t* q = satir + (size_t)sx * bpp;
            if (bpp == 4) {
                *(volatile uint32_t*)q = p;
            } else if (bpp == 3) {
                q[0] = (uint8_t)p;
                q[1] = (uint8_t)(p >> 8);
                q[2] = (uint8_t)(p >> 16);
            } else {
                *(volatile uint16_t*)q = (uint16_t)p;
            }
        }
    }
}

static void _upp_ekr_hucre(int x, int y, long long cp, uint8_t renk) {
    if (x < 0 || y < 0 || x >= _upp_ekr_g || y >= _upp_ekr_y) {
        return;
    }
    if (_upp_fb_aktif) {
        uint32_t h = (uint32_t)(cp & 0xFFFFFF) | ((uint32_t)renk << 24);
        _upp_fb_hucreler[y * _upp_ekr_g + x] = h;
        _upp_fb_ciz(x, y, h, 0);
        return;
    }
    _UPP_VGA[y * _upp_ekr_g + x] = (uint16_t)(((uint16_t)renk << 8) | _upp_cp437(cp));
}

static void _upp_ekr_kaydir(void) {
    int i;
    if (_upp_fb_aktif) {
        /* Çerçeve tamponundan asla okunmaz: aygıt belleğinden okumak (önbelleksiz ya
         * da write-combining) çok yavaştır ve her satırda tüm ekranı taşımak
         * megabaytlarca okuma demektir. Hücre tablosu kaydırılır, yalnızca içeriği
         * değişen hücreler yeniden çizilir (boş alanlar ve tekrarlar atlanır). */
        int x, y, g = _upp_ekr_g;
        uint32_t bos = (uint32_t)' ' | (0x07u << 24);
        if (_upp_fb_imlec_x >= 0 && _upp_fb_imlec_y >= 0 && _upp_fb_imlec_x < g && _upp_fb_imlec_y < _upp_ekr_y) {
            _upp_fb_ciz(_upp_fb_imlec_x, _upp_fb_imlec_y, _upp_fb_hucreler[_upp_fb_imlec_y * g + _upp_fb_imlec_x], 0);
        }
        _upp_fb_imlec_x = -1;
        _upp_fb_imlec_y = -1;
        for (y = 0; y < _upp_ekr_y - 1; y++) {
            for (x = 0; x < g; x++) {
                uint32_t yeni = _upp_fb_hucreler[(y + 1) * g + x];
                if (_upp_fb_hucreler[y * g + x] != yeni) {
                    _upp_fb_hucreler[y * g + x] = yeni;
                    _upp_fb_ciz(x, y, yeni, 0);
                }
            }
        }
        for (x = 0; x < g; x++) {
            if (_upp_fb_hucreler[(_upp_ekr_y - 1) * g + x] != bos) {
                _upp_fb_hucreler[(_upp_ekr_y - 1) * g + x] = bos;
                _upp_fb_ciz(x, _upp_ekr_y - 1, bos, 0);
            }
        }
        (void)i;
    } else {
        for (i = 0; i < _upp_ekr_g * (_upp_ekr_y - 1); i++) {
            _UPP_VGA[i] = _UPP_VGA[i + _upp_ekr_g];
        }
        for (i = _upp_ekr_g * (_upp_ekr_y - 1); i < _upp_ekr_g * _upp_ekr_y; i++) {
            _UPP_VGA[i] = (uint16_t)(0x0700 | ' ');
        }
    }
    _upp_ekr_s = _upp_ekr_y - 1;
}

static void _upp_ekr_temizle(void) {
    int x, y;
    for (y = 0; y < _upp_ekr_y; y++) {
        for (x = 0; x < _upp_ekr_g; x++) {
            _upp_ekr_hucre(x, y, ' ', 0x07);
        }
    }
    _upp_ekr_x = 0;
    _upp_ekr_s = 0;
    _upp_ekr_sarma = 0;
    _upp_fb_imlec_x = -1;
}

static void _upp_ekr_karakter(long long cp, uint8_t renk) {
    if (cp != '\n' && cp != '\r' && cp != 8 && _upp_ekr_sarma) {
        _upp_ekr_x = 0;
        if (++_upp_ekr_s >= _upp_ekr_y) {
            _upp_ekr_kaydir();
        }
    }
    _upp_ekr_sarma = 0;
    if (cp == '\n') {
        _upp_ekr_x = 0;
        if (++_upp_ekr_s >= _upp_ekr_y) {
            _upp_ekr_kaydir();
        }
        return;
    }
    if (cp == '\r') {
        _upp_ekr_x = 0;
        return;
    }
    if (cp == 8) {
        if (_upp_ekr_x > 0) {
            _upp_ekr_x--;
        }
        return;
    }
    if (cp == '\t') {
        do {
            _upp_ekr_karakter(' ', renk);
        } while (_upp_ekr_x % 8 != 0);
        return;
    }
    _upp_ekr_hucre(_upp_ekr_x, _upp_ekr_s, cp, renk);
    if (_upp_ekr_x + 1 >= _upp_ekr_g) {
        _upp_ekr_sarma = 1;
    } else {
        _upp_ekr_x++;
    }
}

/* İmleç: VGA'da donanım imleci, çerçeve tamponunda alt çizgi. */
/* \e[?25l / \e[?25h: imleç gizli mi (tam ekran programlar, ör. oyunlar). */
static int _upp_ekr_imlec_gizli = 0;

static void _upp_ekr_imlec(void) {
    if (_upp_fb_aktif) {
        if (_upp_fb_imlec_x >= 0 && _upp_fb_imlec_y >= 0 && _upp_fb_imlec_x < _upp_ekr_g && _upp_fb_imlec_y < _upp_ekr_y) {
            _upp_fb_ciz(_upp_fb_imlec_x, _upp_fb_imlec_y, _upp_fb_hucreler[_upp_fb_imlec_y * _upp_ekr_g + _upp_fb_imlec_x], 0);
        }
        if (_upp_ekr_imlec_gizli) {
            _upp_fb_imlec_x = -1;
            _upp_fb_imlec_y = -1;
            return;
        }
        _upp_fb_imlec_x = _upp_ekr_x;
        _upp_fb_imlec_y = _upp_ekr_s;
        _upp_fb_ciz(_upp_ekr_x, _upp_ekr_s, _upp_fb_hucreler[_upp_ekr_s * _upp_ekr_g + _upp_ekr_x], 1);
        return;
    }
    {
        /* Ekran dışındaki bir konum VGA donanım imlecini gizler. */
        uint16_t yer = _upp_ekr_imlec_gizli ? (uint16_t)0xFFFF : (uint16_t)(_upp_ekr_s * _upp_ekr_g + _upp_ekr_x);
        _upp_outb(0x3D4, 0x0F);
        _upp_outb(0x3D5, (uint8_t)(yer & 0xFF));
        _upp_outb(0x3D4, 0x0E);
        _upp_outb(0x3D5, (uint8_t)(yer >> 8));
    }
}

/* Limine çerçeve tamponuna geçer (sayfalama kurulduktan sonra, fiziksel adresle). */
static void _upp_fb_kur(uint64_t adres, uint32_t gen, uint32_t yuk, uint32_t adim, uint32_t bpp,
                        uint8_t rk, uint8_t rs, uint8_t gk, uint8_t gs, uint8_t bk, uint8_t bs) {
    int g, y;
    if (bpp != 32 && bpp != 24 && bpp != 16) {
        return;
    }
    g = (int)(gen / 8u);
    y = (int)(yuk / 16u);
    if (g < 1 || y < 1) {
        return;
    }
    while ((long long)g * y > _UPP_FB_HUCRE_MAX) {
        y--;
    }
    _upp_fb = (volatile uint8_t*)(uintptr_t)adres;
    _upp_fb_gen = gen;
    _upp_fb_yuk = yuk;
    _upp_fb_adim = adim;
    _upp_fb_bpp = bpp;
    _upp_fb_rk = rk;
    _upp_fb_rs = rs;
    _upp_fb_gk = gk;
    _upp_fb_gs = gs;
    _upp_fb_bk = bk;
    _upp_fb_bs = bs;
    _upp_ekr_g = g;
    _upp_ekr_y = y;
    _upp_fb_aktif = 1;
    _upp_ekr_temizle();
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
    case 0xB0: return 0xF8; /* ° */
    case 0x2500: return 0xC4; /* ─ */
    case 0x2502: return 0xB3; /* │ */
    case 0x250C: return 0xDA; /* ┌ */
    case 0x2510: return 0xBF; /* ┐ */
    case 0x2514: return 0xC0; /* └ */
    case 0x2518: return 0xD9; /* ┘ */
    case 0x251C: return 0xC3; /* ├ */
    case 0x2524: return 0xB4; /* ┤ */
    case 0x252C: return 0xC2; /* ┬ */
    case 0x2534: return 0xC1; /* ┴ */
    case 0x253C: return 0xC5; /* ┼ */
    case 0x2588: return 0xDB; /* █ */
    case 0x2591: return 0xB0; /* ░ */
    case 0x2592: return 0xB1; /* ▒ */
    case 0x2593: return 0xB2; /* ▓ */
    default: return '?';
    }
}

/* Küçük ANSI çözümleyici (ekran için): ESC[...m (renk, 7 ters, 40-47/100-107 arka plan),
 * ESC[2J, ESC[H, ESC[K, ESC[nA/B/C/D.
 * Seri terminal aynı baytları kendisi yorumlar. */
static int _upp_ansi_durum = 0;
static int _upp_ansi_p[4];
static int _upp_ansi_n = 0;
static int _upp_ansi_ozel = 0; /* '?' ile başlayan özel dizi (\e[?25l) */
static int _upp_vga_renk = -1;
static int _upp_ansi_ters = 0; /* SGR 7: ön ve arka plan yer değiştirir */
static const uint8_t _upp_ansi_vga[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };

static void _upp_vga_temizle(void) {
    _upp_ekr_temizle();
}

static void _upp_ansi_uygula(long long son) {
    int i;
    if (son == 'm') {
        if (_upp_ansi_n == 0) {
            _upp_vga_renk = -1;
        }
        if (_upp_ansi_n == 0) {
            _upp_ansi_ters = 0;
        }
        for (i = 0; i < _upp_ansi_n; i++) {
            int v = _upp_ansi_p[i];
            int r = _upp_vga_renk < 0 ? 0x07 : _upp_vga_renk;
            if (v == 0) {
                _upp_vga_renk = -1;
                _upp_ansi_ters = 0;
            } else if (v == 1) {
                _upp_vga_renk = r | 0x08;
            } else if (v == 22) {
                _upp_vga_renk = r & ~0x08;
            } else if (v == 7) {
                _upp_ansi_ters = 1;
            } else if (v == 27) {
                _upp_ansi_ters = 0;
            } else if (v >= 30 && v <= 37) {
                _upp_vga_renk = (r & 0xF8) | _upp_ansi_vga[v - 30];
            } else if (v == 39) {
                _upp_vga_renk = (r & 0xF0) | 0x07;
            } else if (v >= 40 && v <= 47) {
                _upp_vga_renk = (r & 0x0F) | (_upp_ansi_vga[v - 40] << 4);
            } else if (v == 49) {
                _upp_vga_renk = r & 0x0F;
            } else if (v >= 90 && v <= 97) {
                _upp_vga_renk = (r & 0xF0) | 0x08 | _upp_ansi_vga[v - 90];
            } else if (v >= 100 && v <= 107) {
                _upp_vga_renk = (r & 0x0F) | ((0x08 | _upp_ansi_vga[v - 100]) << 4);
            }
        }
    } else if (_upp_ansi_ozel) {
        /* \e[?25l / \e[?25h */
        if ((son == 'l' || son == 'h') && _upp_ansi_n > 0 && _upp_ansi_p[0] == 25) {
            _upp_ekr_imlec_gizli = son == 'l';
        }
    } else if (son == 'J') {
        _upp_ekr_temizle();
    } else if (son == 'A' || son == 'B') {
        int n = _upp_ansi_n > 0 && _upp_ansi_p[0] > 0 ? _upp_ansi_p[0] : 1;
        _upp_ekr_s += son == 'B' ? n : -n;
        if (_upp_ekr_s < 0) _upp_ekr_s = 0;
        if (_upp_ekr_s >= _upp_ekr_y) _upp_ekr_s = _upp_ekr_y - 1;
        _upp_ekr_sarma = 0;
    } else if (son == 'H' || son == 'f') {
        /* \e[satır;sütunH, 1 tabanlı; eksik parametre 1 sayılır. */
        int s = _upp_ansi_n > 0 && _upp_ansi_p[0] > 0 ? _upp_ansi_p[0] : 1;
        int x = _upp_ansi_n > 1 && _upp_ansi_p[1] > 0 ? _upp_ansi_p[1] : 1;
        _upp_ekr_s = s > _upp_ekr_y ? _upp_ekr_y - 1 : s - 1;
        _upp_ekr_x = x > _upp_ekr_g ? _upp_ekr_g - 1 : x - 1;
        _upp_ekr_sarma = 0;
    } else if (son == 'C' || son == 'D') {
        int n = _upp_ansi_n > 0 && _upp_ansi_p[0] > 0 ? _upp_ansi_p[0] : 1;
        _upp_ekr_x += son == 'C' ? n : -n;
        if (_upp_ekr_x < 0) _upp_ekr_x = 0;
        if (_upp_ekr_x >= _upp_ekr_g) _upp_ekr_x = _upp_ekr_g - 1;
        _upp_ekr_sarma = 0;
    } else if (son == 'K') {
        for (i = _upp_ekr_x; i < _upp_ekr_g; i++) {
            _upp_ekr_hucre(i, _upp_ekr_s, ' ', 0x07);
        }
    }
}

/* 1: kod noktası kaçış dizisine ait (VGA'ya yazılmaz). */
static int _upp_ansi_isle(long long cp) {
    if (_upp_ansi_durum == 0) {
        if (cp == 27) {
            _upp_ansi_durum = 1;
            return 1;
        }
        return 0;
    }
    if (_upp_ansi_durum == 1) {
        if (cp == '[') {
            _upp_ansi_durum = 2;
            _upp_ansi_n = 0;
            _upp_ansi_p[0] = 0;
            _upp_ansi_ozel = 0;
            return 1;
        }
        _upp_ansi_durum = 0;
        return 1;
    }
    if (cp == '?' && _upp_ansi_n == 0 && !_upp_ansi_ozel) {
        _upp_ansi_ozel = 1;
        return 1;
    }
    if (cp >= '0' && cp <= '9') {
        if (_upp_ansi_n == 0) {
            _upp_ansi_n = 1;
            _upp_ansi_p[0] = 0;
        }
        _upp_ansi_p[_upp_ansi_n - 1] = _upp_ansi_p[_upp_ansi_n - 1] * 10 + (int)(cp - '0');
        return 1;
    }
    if (cp == ';') {
        if (_upp_ansi_n == 0) {
            _upp_ansi_n = 1;
            _upp_ansi_p[0] = 0;
        }
        if (_upp_ansi_n < 4) {
            _upp_ansi_p[_upp_ansi_n++] = 0;
        }
        return 1;
    }
    _upp_ansi_durum = 0;
    _upp_ansi_uygula(cp);
    return 1;
}

static void _upp_akis_bayt(int akis, const char* p, size_t n, int nl) {
    uint8_t renk = akis == 2 ? 0x0C : 0x07;
    size_t i = 0;
    while (p && i < n) {
        size_t a = (size_t)_upp_utf8_adv(p, n, i);
        size_t k;
        long long cp = _upp_utf8_cp_at(p, n, i);
        for (k = 0; k < a; k++) {
            _upp_seri_bayt((unsigned char)p[i + k]);
        }
        if (!_upp_ansi_isle(cp)) {
            uint8_t r = renk;
            if (akis != 2) {
                if (_upp_vga_renk >= 0) {
                    r = (uint8_t)_upp_vga_renk;
                }
                if (_upp_ansi_ters) {
                    r = (uint8_t)(((r & 0x0F) << 4) | ((r >> 4) & 0x0F));
                }
            }
            _upp_ekr_karakter(cp, r);
        }
        i += a;
    }
    if (nl) {
        _upp_seri_bayt('\r');
        _upp_seri_bayt('\n');
        _upp_ekr_karakter('\n', renk);
    }
    _upp_ekr_imlec();
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
    /* Kesilen kodun (ör. ring 3) MXCSR/x87 ayarı işleyiciye sızmasın: varsayılan. */
    "    sub $16, %rsp\n"
    "    movl $0x1F80, (%rsp)\n"
    "    ldmxcsr (%rsp)\n"
    "    add $16, %rsp\n"
    "    fninit\n"
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
static int _upp_tus_ctrl = 0;
static int _upp_tus_altgr = 0;
static int _upp_tus_e0 = 0;

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

/* Genişletilmiş tuşlar (ok, Home/End, Delete) seri terminaldeki gibi ESC [ X dizisine çevrilir. */
static void _upp_tus_dizi(uint32_t son) {
    _upp_tus_koy(27);
    _upp_tus_koy('[');
    _upp_tus_koy(son);
}

static void _upp_klavye(void) {
    uint8_t sc = _upp_inb(0x60);
    if (sc == 0xE0) {
        _upp_tus_e0 = 1;
        return;
    }
    if (_upp_tus_e0) {
        _upp_tus_e0 = 0;
        if (sc == 0x1D) { _upp_tus_ctrl = 1; return; }
        if (sc == 0x9D) { _upp_tus_ctrl = 0; return; }
        if (sc == 0x38) { _upp_tus_altgr = 1; return; }
        if (sc == 0xB8) { _upp_tus_altgr = 0; return; }
        switch (sc) {
        case 0x48: _upp_tus_dizi('A'); break;
        case 0x50: _upp_tus_dizi('B'); break;
        case 0x4D: _upp_tus_dizi('C'); break;
        case 0x4B: _upp_tus_dizi('D'); break;
        case 0x47: _upp_tus_dizi('H'); break;
        case 0x4F: _upp_tus_dizi('F'); break;
        case 0x53: _upp_tus_dizi('3'); _upp_tus_koy('~'); break;
        case 0x52: _upp_tus_dizi('2'); _upp_tus_koy('~'); break; /* Insert */
        case 0x49: _upp_tus_dizi('5'); _upp_tus_koy('~'); break; /* Page Up */
        case 0x51: _upp_tus_dizi('6'); _upp_tus_koy('~'); break; /* Page Down */
        case 0x1C: _upp_tus_koy('\n'); break;
        default: break;
        }
        return;
    }
    if (sc == 0x1D) {
        _upp_tus_ctrl = 1;
        return;
    }
    if (sc == 0x9D) {
        _upp_tus_ctrl = 0;
        return;
    }
    if (sc == 0x2A || sc == 0x36) {
        _upp_tus_shift = 1;
        return;
    }
    if (sc == 0xAA || sc == 0xB6) {
        _upp_tus_shift = 0;
        return;
    }
    /* ISO 102. tuş (küme 1: 0x56): Türkçe Q'da < > |, sol Shift'in sağı.
       Tablo 0x3A ile bittiği için bu tarama kodu yok sayılıyordu. */
    if (sc == 0x56) {
        uint32_t iso = _upp_tus_altgr ? (uint32_t)'|' : (_upp_tus_shift ? (uint32_t)'>' : (uint32_t)'<');
        _upp_tus_koy(iso);
        return;
    }
    if ((sc & 0x80) || sc >= 0x3A) {
        return;
    }
    {
        uint32_t c = _upp_tus_shift ? _upp_trq_shift[sc] : _upp_trq_normal[sc];
        if (_upp_tus_ctrl && ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
            c &= 0x1Fu;
        }
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

/* upp_kullanici.c: ring 3'ten gelen kesme ve istisnalar */
static void _upp_kul_istisna(UppKesmeCerceve* c);
static void _upp_kul_kesme(UppKesmeCerceve* c);

__attribute__((used)) void _upp_kesme_isle(UppKesmeCerceve* c) {
    if (c->vektor < 32 && (c->cs & 3u) == 3u) {
        _upp_kul_istisna(c); /* yalnızca süreci sonlandırır; dönmez */
    }
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
    if (c->vektor >= 32 && (c->cs & 3u) == 3u) {
        _upp_kul_kesme(c); /* Ctrl-C, seri girdi, zaman dilimi (EOI'dan sonra: sıra verebilir) */
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

static int _upp_limine_bellek(void);

/* =====================================================================
 * Bellek haritası: Multiboot 1 bilgisinden RAM'i bulur ve yığını
 * çekirdeğin bitişinden sonraki en büyük boş bölgeye (ilk 1 GiB'lik
 * birebir eşleme içinde) taşır. Önyükleyici yoksa (serbest .o) atlanır.
 * ===================================================================== */
extern uint32_t upp_multiboot_bilgi __attribute__((weak));
extern unsigned char _upp_cekirdek_son[] __attribute__((weak));
static uint64_t _upp_ram_toplam = 0;

static void _upp_bellek_kur(void) {
    const uint32_t* mb;
    uint64_t taban, en_bas = 0, en_son = 0, sinir = 0x40000000ull;
    if (_upp_limine_bellek()) {
        return;
    }
    if (&upp_multiboot_bilgi == NULL || _upp_cekirdek_son == NULL || upp_multiboot_bilgi == 0) {
        return;
    }
    mb = (const uint32_t*)(uintptr_t)upp_multiboot_bilgi;
    taban = ((uint64_t)(uintptr_t)_upp_cekirdek_son + 0xFFFull) & ~0xFFFull;
    if (mb[0] & 64u) {
        uintptr_t p = (uintptr_t)mb[12];
        uintptr_t z = p + mb[11];
        while (p < z) {
            uint32_t boy = *(const uint32_t*)p;
            uint64_t bas = *(const uint64_t*)(p + 4);
            uint64_t uz = *(const uint64_t*)(p + 12);
            uint32_t tur = *(const uint32_t*)(p + 20);
            if (tur == 1) {
                uint64_t s = bas, e = bas + uz;
                _upp_ram_toplam += uz;
                if (e > sinir) e = sinir;
                if (s < taban) s = taban;
                if (e > s && e - s > en_son - en_bas) {
                    en_bas = s;
                    en_son = e;
                }
            }
            p += (uintptr_t)boy + 4u;
        }
    } else if (mb[0] & 1u) {
        uint64_t e = 0x100000ull + (uint64_t)mb[2] * 1024u;
        _upp_ram_toplam = e;
        if (e > sinir) e = sinir;
        if (e > taban) {
            en_bas = taban;
            en_son = e;
        }
    }
    if (!_upp_yigin_bas && en_son - en_bas >= 0x100000ull) {
        _upp_yigin_bolge((void*)(uintptr_t)en_bas, (size_t)(en_son - en_bas));
    }
}

/* =====================================================================
 * Limine önyükleme protokolü (--onyukleyici limine).
 * Limine çekirdeği 64-bit long mode'da, higher-half (0xffffffff80000000)
 * adreste başlatır. İstekler önyükleyici asm'sinde (.limine_istekler);
 * _upp_limine_kur() yanıtları HHDM üzerinden okuyup kopyalar, sonra kendi
 * sayfa tablolarımıza geçer. Çekirdek tamamen üst yarıdadır:
 *   - görüntü kendi sanal adresinde (0xffffffff80000000, 4 KiB sayfalar);
 *   - tüm fiziksel bellek HHDM'de (Limine'in ofseti, 2 MiB sayfalar;
 *     çerçeve tamponunun sayfaları önbelleksiz).
 * Alt yarı (0..0x00007fffffffffff) boştur: kullanıcı süreçlerinindir
 * (bkz. upp_kullanici.c). Limine yerel APIC'i açık ama LINT0'ı maskeli
 * bırakır; APIC kapatılınca 8259 PIC kesmeleri yeniden gelir.
 * ===================================================================== */
extern volatile uint64_t _upp_limine_surum[3] __attribute__((weak));
extern volatile uint64_t _upp_limine_hhdm[6] __attribute__((weak));
extern volatile uint64_t _upp_limine_adres[6] __attribute__((weak));
extern volatile uint64_t _upp_limine_harita[6] __attribute__((weak));
extern volatile uint64_t _upp_limine_bilgi[6] __attribute__((weak));
extern volatile uint64_t _upp_limine_fb[6] __attribute__((weak));
extern unsigned char _upp_cekirdek_bas[] __attribute__((weak));

#define _UPP_LM_MAX 128
static int _upp_limine = 0;
static int _upp_lm_n = 0;
static uint64_t _upp_lm_bas[_UPP_LM_MAX], _upp_lm_uz[_UPP_LM_MAX], _upp_lm_tur[_UPP_LM_MAX];
static char _upp_lm_ad[96];
static uint64_t _upp_lm_surum = 0;
static char _upp_ekran_bilgi[64];

#define _UPP_ST_HAVUZ 96
static uint64_t _upp_st_havuz[_UPP_ST_HAVUZ][512] __attribute__((aligned(4096)));
static int _upp_st_n = 0;
static uint64_t _upp_st_fiz = 0, _upp_st_sanal = 0;
/* Çekirdeğin PML4'ü: süreçlerin PML4'leri üst yarı girdilerini buradan kopyalar. */
static uint64_t* _upp_cekirdek_pml4 = NULL;
static uint64_t _upp_cekirdek_cr3 = 0;
/* Çerçeve tamponunun fiziksel adresi/boyu (kullanıcıya mmap için). */
static uint64_t _upp_fb_fiz = 0, _upp_fb_boy = 0;

static uint64_t _upp_st_fizik(const void* p) {
    return (uint64_t)(uintptr_t)p - _upp_st_sanal + _upp_st_fiz;
}

/* DMA için bir tampon baytının fiziksel adresi. Çekirdek yığını fiziksel
 * olarak bitişik tek bir bölgededir (HHDM'de ya da Multiboot'un birebir
 * eşlemesinde), çekirdek görüntüsü de doğrusal eşlenir; bu yüzden bir tamponun
 * tamamı bitişiktir ve adres + n ile ilerlenebilir. Bilinmeyen bölge -1. */
static long long upp_cekirdek_tampon_fiziksel(long long t, long long ofs) {
    _UppTampon* b = _upp_tampon_al(t, ofs, 0);
    uint64_t v = (uint64_t)(uintptr_t)(b->veri + ofs);
    if (_upp_st_sanal && v >= _upp_st_sanal) {
        return (long long)(v - _upp_st_sanal + _upp_st_fiz);
    }
    if (_upp_hhdm) {
        if (v >= _upp_hhdm && v - _upp_hhdm < _upp_hhdm_son) {
            return (long long)(v - _upp_hhdm);
        }
        return -1;
    }
    return v < 0x40000000ull ? (long long)v : -1;
}

static uint64_t* _upp_st_yeni(void) {
    uint64_t* t;
    if (_upp_st_n >= _UPP_ST_HAVUZ) {
        for (;;) {
            __asm__ volatile("cli; hlt");
        }
    }
    t = _upp_st_havuz[_upp_st_n++];
    memset(t, 0, 4096);
    return t;
}

/* Sanal adresin girdisinin bir alt tablosunu (yoksa oluşturarak) döndürür. */
static uint64_t* _upp_st_alt(uint64_t* t, unsigned i) {
    if (!(t[i] & 1u)) {
        uint64_t* y = _upp_st_yeni();
        t[i] = _upp_st_fizik(y) | 3u;
        return y;
    }
    return (uint64_t*)(uintptr_t)((t[i] & 0x000FFFFFFFFFF000ull) - _upp_st_fiz + _upp_st_sanal);
}

static void _upp_kopyala_ad(char* h, size_t kap, const char* k) {
    size_t i = 0;
    if (!k) {
        h[0] = 0;
        return;
    }
    while (k[i] && i + 1 < kap) {
        h[i] = k[i];
        i++;
    }
    h[i] = 0;
}

static void _upp_ekle_ad(char* h, size_t kap, const char* k) {
    size_t n = strlen(h);
    _upp_kopyala_ad(h + n, kap - n, k);
}

static void _upp_sayi_ad(char* h, size_t kap, uint64_t v) {
    char b[24];
    int n = 0;
    do {
        b[n++] = (char)('0' + v % 10u);
        v /= 10u;
    } while (v && n < 23);
    while (n > 0 && strlen(h) + 1 < kap) {
        size_t m = strlen(h);
        h[m] = b[--n];
        h[m + 1] = 0;
    }
}

__attribute__((used)) void _upp_limine_kur(void) {
    uint64_t hhdm = 0;
    uint64_t fb_fiz = 0, fb_boy = 0;
    uint32_t fb_gen = 0, fb_yuk = 0, fb_adim = 0, fb_bpp = 0;
    uint8_t rk = 0, rs = 0, gk = 0, gs = 0, bk = 0, bs = 0;
    uint64_t* pml4;
    uint64_t v, son, ust;
    unsigned i, j;
    if (&_upp_limine_surum == NULL) {
        return;
    }
    _upp_limine = 1;
    _upp_lm_surum = _upp_limine_surum[1] != 0x6a7b384944536bdcull ? _upp_limine_surum[1] : 0;
    if (_upp_limine_hhdm[5]) {
        hhdm = ((volatile uint64_t*)(uintptr_t)_upp_limine_hhdm[5])[1];
    }
    if (_upp_limine_adres[5]) {
        volatile uint64_t* r = (volatile uint64_t*)(uintptr_t)_upp_limine_adres[5];
        _upp_st_fiz = r[1];
        _upp_st_sanal = r[2];
    }
    if (_upp_limine_bilgi[5]) {
        volatile uint64_t* r = (volatile uint64_t*)(uintptr_t)_upp_limine_bilgi[5];
        _upp_kopyala_ad(_upp_lm_ad, sizeof(_upp_lm_ad), (const char*)(uintptr_t)r[1]);
        _upp_ekle_ad(_upp_lm_ad, sizeof(_upp_lm_ad), " ");
        _upp_ekle_ad(_upp_lm_ad, sizeof(_upp_lm_ad), (const char*)(uintptr_t)r[2]);
    } else {
        _upp_kopyala_ad(_upp_lm_ad, sizeof(_upp_lm_ad), "Limine");
    }
    if (_upp_limine_harita[5]) {
        volatile uint64_t* r = (volatile uint64_t*)(uintptr_t)_upp_limine_harita[5];
        uint64_t n = r[1];
        volatile uint64_t* const* g = (volatile uint64_t* const*)(uintptr_t)r[2];
        for (i = 0; i < n && i < _UPP_LM_MAX; i++) {
            _upp_lm_bas[i] = g[i][0];
            _upp_lm_uz[i] = g[i][1];
            _upp_lm_tur[i] = g[i][2];
        }
        _upp_lm_n = (int)i;
    }
    if (_upp_limine_fb[5]) {
        volatile uint64_t* r = (volatile uint64_t*)(uintptr_t)_upp_limine_fb[5];
        if (r[1] > 0) {
            volatile uint8_t* f = *(volatile uint8_t* const*)(uintptr_t)r[2];
            uint64_t adres = *(volatile uint64_t*)(f + 0);
            fb_gen = (uint32_t)*(volatile uint64_t*)(f + 8);
            fb_yuk = (uint32_t)*(volatile uint64_t*)(f + 16);
            fb_adim = (uint32_t)*(volatile uint64_t*)(f + 24);
            fb_bpp = *(volatile uint16_t*)(f + 32);
            rk = f[35];
            rs = f[36];
            gk = f[37];
            gs = f[38];
            bk = f[39];
            bs = f[40];
            fb_fiz = adres - hhdm;
            fb_boy = (uint64_t)fb_adim * fb_yuk;
            if (f[34] != 1) {
                fb_bpp = 0; /* yalnızca RGB bellek modeli */
            }
        }
    }
    if (!_upp_st_sanal || !hhdm || (hhdm & 0x3FFFFFFFull) != 0) {
        return;
    }
    /* HHDM'nin kapsayacağı fiziksel üst sınır: RAM, ACPI ve çerçeve tamponu
     * dahil haritadaki en yüksek adres (en az 4 GiB: MMIO; en çok 64 GiB). */
    ust = 0x100000000ull;
    for (i = 0; i < (unsigned)_upp_lm_n; i++) {
        uint64_t t = _upp_lm_tur[i];
        uint64_t e = _upp_lm_bas[i] + _upp_lm_uz[i];
        if ((t == 0 || t == 2 || t == 3 || t == 5 || t == 6 || t == 7) && e > ust) {
            ust = e;
        }
    }
    if (fb_boy && fb_fiz + fb_boy > ust) {
        ust = fb_fiz + fb_boy;
    }
    ust = (ust + 0x3FFFFFFFull) & ~0x3FFFFFFFull;
    if (ust > (64ull << 30)) {
        ust = 64ull << 30;
    }
    /* PAT: PA1 (PWT=1, PCD=0) write-combining olur; çerçeve tamponu bununla
     * eşlenir. PAT yoksa PWT tek başına write-through'dur. */
    {
        uint32_t a, b, c, d;
        __asm__ volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(1u), "c"(0u));
        if (d & (1u << 16)) {
            uint64_t pat = 0x0007040600070106ull;
            __asm__ volatile("wbinvd" ::: "memory");
            __asm__ volatile("wrmsr" : : "a"((uint32_t)pat), "d"((uint32_t)(pat >> 32)), "c"(0x277u));
        }
    }
    /* Kendi sayfa tablolarımız */
    pml4 = _upp_st_yeni();
    {
        uint64_t* pdpt = _upp_st_alt(pml4, (unsigned)((hhdm >> 39) & 511u));
        unsigned pdpt_ilk = (unsigned)((hhdm >> 30) & 511u);
        for (i = 0; (uint64_t)i < (ust >> 30) && pdpt_ilk + i < 512u; i++) {
            uint64_t* pd = _upp_st_alt(pdpt, pdpt_ilk + i);
            for (j = 0; j < 512; j++) {
                uint64_t fiz = ((uint64_t)i << 30) | ((uint64_t)j << 21);
                uint64_t bayrak = 0x83; /* P | RW | 2 MiB; kullanıcıya kapalı */
                if (fb_boy && fiz + 0x200000ull > fb_fiz && fiz < fb_fiz + fb_boy) {
                    bayrak |= 0x08; /* PWT → PAT PA1: write-combining */
                }
                pd[j] = fiz | bayrak;
            }
        }
    }
    v = _upp_st_sanal & ~0xFFFull;
    son = ((uint64_t)(uintptr_t)_upp_cekirdek_son + 0xFFFull) & ~0xFFFull;
    for (; v < son; v += 4096u) {
        uint64_t* pdpt = _upp_st_alt(pml4, (unsigned)((v >> 39) & 511u));
        uint64_t* pd = _upp_st_alt(pdpt, (unsigned)((v >> 30) & 511u));
        uint64_t* pt = _upp_st_alt(pd, (unsigned)((v >> 21) & 511u));
        pt[(v >> 12) & 511u] = (v - _upp_st_sanal + _upp_st_fiz) | 3u;
    }
    __asm__ volatile("mov %0, %%cr3" : : "r"(_upp_st_fizik(pml4)) : "memory");
    _upp_cekirdek_pml4 = pml4;
    _upp_cekirdek_cr3 = _upp_st_fizik(pml4);
    _upp_hhdm = hhdm;
    _upp_hhdm_son = ust;
    _upp_vga_taban = (volatile uint16_t*)(uintptr_t)(hhdm + 0xB8000ull);
    /* Yerel APIC'i kapat: 8259 PIC kesmeleri doğrudan INTR'ye gelir. */
    {
        uint32_t lo, hi;
        __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(0x1Bu));
        lo &= ~0x800u;
        __asm__ volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(0x1Bu));
    }
    if (fb_bpp && fb_fiz + fb_boy <= ust) {
        _upp_fb_fiz = fb_fiz;
        _upp_fb_boy = fb_boy;
        _upp_fb_kur(hhdm + fb_fiz, fb_gen, fb_yuk, fb_adim, fb_bpp, rk, rs, gk, gs, bk, bs);
        _upp_kopyala_ad(_upp_ekran_bilgi, sizeof(_upp_ekran_bilgi), "çerçeve tamponu ");
        _upp_sayi_ad(_upp_ekran_bilgi, sizeof(_upp_ekran_bilgi), fb_gen);
        _upp_ekle_ad(_upp_ekran_bilgi, sizeof(_upp_ekran_bilgi), "x");
        _upp_sayi_ad(_upp_ekran_bilgi, sizeof(_upp_ekran_bilgi), fb_yuk);
        _upp_ekle_ad(_upp_ekran_bilgi, sizeof(_upp_ekran_bilgi), "x");
        _upp_sayi_ad(_upp_ekran_bilgi, sizeof(_upp_ekran_bilgi), fb_bpp);
        _upp_ekle_ad(_upp_ekran_bilgi, sizeof(_upp_ekran_bilgi), ", ");
        _upp_sayi_ad(_upp_ekran_bilgi, sizeof(_upp_ekran_bilgi), (uint64_t)_upp_ekr_g);
        _upp_ekle_ad(_upp_ekran_bilgi, sizeof(_upp_ekran_bilgi), "x");
        _upp_sayi_ad(_upp_ekran_bilgi, sizeof(_upp_ekran_bilgi), (uint64_t)_upp_ekr_y);
        _upp_ekle_ad(_upp_ekran_bilgi, sizeof(_upp_ekran_bilgi), " karakter");
    }
}

/* =====================================================================
 * Fiziksel çerçeve (4 KiB sayfa) ayırıcı — yalnızca Limine (HHDM) kipinde.
 * Kullanıcı süreçlerinin sayfaları ve sayfa tabloları buradan gelir; çekirdek
 * yığını (heap) ayrı bir bölgededir, böylece kullanıcı belleği çekirdeği
 * aç bırakamaz. Bölgeler sırayla tüketilir; bırakılan çerçeveler, ilk 8
 * baytında bir sonrakinin fiziksel adresini tutan tek bağlı listeye girer.
 * ===================================================================== */
#define _UPP_CER_BOLGE_N 32
static uint64_t _upp_cer_bas[_UPP_CER_BOLGE_N], _upp_cer_son[_UPP_CER_BOLGE_N];
static int _upp_cer_bolge_n = 0;
static uint64_t _upp_cer_serbest = 0;
static uint64_t _upp_cer_toplam = 0, _upp_cer_kullanilan = 0;

static void _upp_cer_bolge_ekle(uint64_t bas, uint64_t son) {
    bas = (bas + 0xFFFull) & ~0xFFFull;
    son &= ~0xFFFull;
    if (bas < 0x100000ull) {
        bas = 0x100000ull;
    }
    if (son > _upp_hhdm_son) {
        son = _upp_hhdm_son;
    }
    if (son <= bas || _upp_cer_bolge_n >= _UPP_CER_BOLGE_N) {
        return;
    }
    _upp_cer_bas[_upp_cer_bolge_n] = bas;
    _upp_cer_son[_upp_cer_bolge_n] = son;
    _upp_cer_bolge_n++;
    _upp_cer_toplam += son - bas;
}

/* Sıfırlanmış bir çerçevenin fiziksel adresi; bellek bittiyse 0. */
static uint64_t _upp_cer_ayir(void) {
    uint64_t f = 0;
    int i;
    if (_upp_cer_serbest) {
        f = _upp_cer_serbest;
        _upp_cer_serbest = *(volatile uint64_t*)(uintptr_t)(f + _upp_hhdm);
    } else {
        for (i = 0; i < _upp_cer_bolge_n; i++) {
            if (_upp_cer_bas[i] < _upp_cer_son[i]) {
                f = _upp_cer_bas[i];
                _upp_cer_bas[i] += 4096u;
                break;
            }
        }
    }
    if (!f) {
        return 0;
    }
    memset((void*)(uintptr_t)(f + _upp_hhdm), 0, 4096);
    _upp_cer_kullanilan += 4096u;
    return f;
}

static void _upp_cer_birak(uint64_t f) {
    if (!f) {
        return;
    }
    *(volatile uint64_t*)(uintptr_t)(f + _upp_hhdm) = _upp_cer_serbest;
    _upp_cer_serbest = f;
    _upp_cer_kullanilan -= 4096u;
}

/* Limine bellek haritası: en büyük kullanılabilir bölgenin dörtte üçü çekirdek
 * yığını, dörtte biri ve diğer kullanılabilir/geri kazanılabilir bölgeler
 * kullanıcı çerçeveleri olur. İkisi de HHDM üzerinden erişilir. */
static int _upp_limine_bellek(void) {
    uint64_t en_bas = 0, en_son = 0, cer;
    int i, en = -1;
    if (!_upp_limine || _upp_lm_n == 0) {
        return 0;
    }
    for (i = 0; i < _upp_lm_n; i++) {
        uint64_t s = _upp_lm_bas[i], e = _upp_lm_bas[i] + _upp_lm_uz[i];
        uint64_t t = _upp_lm_tur[i];
        if (t == 0 || t == 5 || t == 6) {
            _upp_ram_toplam += _upp_lm_uz[i];
        }
        if (t != 0) {
            continue;
        }
        if (s < 0x100000ull) s = 0x100000ull;
        if (e > _upp_hhdm_son) e = _upp_hhdm_son;
        if (e > s && e - s > en_son - en_bas) {
            en_bas = s;
            en_son = e;
            en = i;
        }
    }
    if (_upp_yigin_bas || en_son - en_bas < 0x100000ull) {
        return 1;
    }
    cer = ((en_son - en_bas) / 4u) & ~0xFFFull;
    if (en_son - en_bas < (16ull << 20)) {
        cer = 0;
    }
    _upp_yigin_bolge((void*)(uintptr_t)(en_bas + _upp_hhdm), (size_t)(en_son - en_bas - cer));
    _upp_cer_bolge_ekle(en_son - cer, en_son);
    for (i = 0; i < _upp_lm_n; i++) {
        if (i != en && (_upp_lm_tur[i] == 0 || _upp_lm_tur[i] == 5)) {
            _upp_cer_bolge_ekle(_upp_lm_bas[i], _upp_lm_bas[i] + _upp_lm_uz[i]);
        }
    }
    return 1;
}

static char* upp_cekirdek_onyukleyici(void) {
    if (_upp_limine) {
        return _upp_m_c(_upp_lm_ad);
    }
    if (&upp_multiboot_bilgi != NULL && upp_multiboot_bilgi != 0) {
        return _upp_m_c("Multiboot 1");
    }
    return _upp_m_c("");
}

static char* upp_cekirdek_ekran(void) {
    if (_upp_fb_aktif) {
        return _upp_m_c(_upp_ekran_bilgi);
    }
    return _upp_m_c("VGA metin 80x25");
}

static long long upp_cekirdek_bellek_toplam(void) {
    return (long long)_upp_ram_toplam;
}

static long long upp_cekirdek_yigin_boyut(void) {
    _upp_yigin_hazir();
    return (long long)(_upp_yigin_son - _upp_yigin_bas);
}

static long long upp_cekirdek_yigin_kullanilan(void) {
    return (long long)_upp_yigin_dolu;
}

static void _upp_kul_kur(void); /* upp_kullanici.c */

/* Çekirdek girişinde, upp_ana()'dan önce. Limine kipinde kullanıcı kipi
 * (GDT/TSS, SYSCALL) de kurulur. */
static void _upp_cekirdek_hazirla(void) {
    _upp_bellek_kur();
    upp_cekirdek_kesmeler_kur();
    _upp_kul_kur();
}

/* =====================================================================
 * Görevler: işbirlikçi çoklu görev. Çekirdek kodu yalnızca upp.uyut,
 * upp.cekirdek.sira_ver ve girdi beklerken sıra verir; runtime'ın paylaşılan
 * durumu bu yüzden kilitsiz güvendedir. Görev 1 çekirdeğin ana akışıdır.
 * Bir görev kullanıcı süreci (ring 3) çalıştırıyorsa ka != 0'dır: geçiş
 * CR3'ü ve TSS.rsp0'ı ona göre ayarlar; ring 3 kodu zamanlayıcıyla kesilip
 * sırasını verebilir (bkz. upp_kullanici.c). Geçiş RFLAGS'ı da taşır:
 * kesme içinden sıra veren görev IF=0 ile, diğerleri IF=1 ile sürer.
 * ===================================================================== */
#define _UPP_GOREV_N 32
#define _UPP_GOREV_YIGIN (64u * 1024u)
#define _UPP_G_BOS 0
#define _UPP_G_HAZIR 1
#define _UPP_G_UYUYOR 2
#define _UPP_G_BITTI 3

typedef struct {
    uint64_t rsp;
    unsigned char* yigin;
    int durum;
    long long kimlik;
    char* ad;
    uint64_t uyan;
    uint64_t baslama;
    void (*f)(long long);
    long long arg;
    /* kullanıcı süreci (ring 3) */
    uint64_t ka;          /* adres alanı: PML4'ün fiziksel adresi; 0 = çekirdek görevi */
    uint64_t kul_kayit;   /* ring 3'e girerken kaydedilen çekirdek yığını (çıkış, TSS.rsp0) */
    uint64_t kul_dilim;   /* zaman diliminin başladığı tik */
    uint64_t kul_fs;      /* FS/GS tabanı (arch_prctl; TLS). Çekirdek FS/GS kullanmaz. */
    uint64_t kul_gs;
    int kul_on_plan;      /* Ctrl-C bu süreci sonlandırır */
    int kul_cikis;        /* sistem çağrısı dönerken süreci bitir */
    long long kul_kod;
    char kul_hata[128];   /* son sonlandırma nedeni ("" = normal çıkış) */
    void* kul_sc;         /* işlenmekte olan sistem çağrısının yazmaç çerçevesi (fork) */
    int kul_yurut;        /* sistem çağrısı dönerken yeni görüntüye geç (execve) */
    uint64_t kul_y_ka, kul_y_giris, kul_y_yigin;
} UppGorev;

static UppGorev _upp_gorev[_UPP_GOREV_N];
static int _upp_gorev_cur = 0;
static long long _upp_gorev_sonraki = 2;
static int _upp_gorev_kuruldu = 0;

__asm__(
    ".text\n"
    ".global _upp_gorev_gec\n"
    "_upp_gorev_gec:\n"
    "    pushfq\n"
    "    push %rbp\n    push %rbx\n    push %r12\n    push %r13\n    push %r14\n    push %r15\n"
    "    mov %rsp, (%rdi)\n"
    "    mov %rsi, %rsp\n"
    "    pop %r15\n    pop %r14\n    pop %r13\n    pop %r12\n    pop %rbx\n    pop %rbp\n"
    "    popfq\n"
    "    ret\n");
extern void _upp_gorev_gec(uint64_t* eski, uint64_t yeni);

/* upp_kullanici.c */
static void _upp_kul_gecis_hazirla(int k);
static void _upp_kul_alan_sil(uint64_t ka);

static void _upp_gorev_ilk(void) {
    if (_upp_gorev_kuruldu) {
        return;
    }
    _upp_gorev_kuruldu = 1;
    upp_cekirdek_kesmeler_kur();
    _upp_gorev[0].durum = _UPP_G_HAZIR;
    _upp_gorev[0].kimlik = 1;
    _upp_gorev[0].ad = _upp_m_c("ana");
    _upp_gorev[0].baslama = 0;
}

static void _upp_gorev_temizle(void) {
    int k;
    for (k = 1; k < _UPP_GOREV_N; k++) {
        UppGorev* g = &_upp_gorev[k];
        if (k != _upp_gorev_cur && g->durum == _UPP_G_BITTI) {
            if (g->ka) {
                _upp_kul_alan_sil(g->ka);
                g->ka = 0;
            }
            UPP_BIRAK(g->yigin);
            upp_metin_release(g->ad);
            g->yigin = NULL;
            g->ad = NULL;
            g->durum = _UPP_G_BOS;
        }
    }
}

/* Sıradaki hazır görev (şimdiki görev en sona bakılır); yoksa -1. */
static int _upp_gorev_sec(void) {
    int i;
    for (i = 1; i <= _UPP_GOREV_N; i++) {
        int k = (_upp_gorev_cur + i) % _UPP_GOREV_N;
        UppGorev* g = &_upp_gorev[k];
        if (g->durum == _UPP_G_UYUYOR && _upp_tik >= g->uyan) {
            g->durum = _UPP_G_HAZIR;
        }
        if (g->durum == _UPP_G_HAZIR) {
            return k;
        }
    }
    return -1;
}

static void _upp_gecis(void) {
    for (;;) {
        int k = _upp_gorev_sec();
        if (k >= 0) {
            if (k != _upp_gorev_cur) {
                int eski = _upp_gorev_cur;
                _upp_kul_gecis_hazirla(k);
                _upp_gorev_cur = k;
                _upp_gorev_gec(&_upp_gorev[eski].rsp, _upp_gorev[k].rsp);
                _upp_gorev_temizle();
            }
            return;
        }
        __asm__ volatile("sti; hlt");
    }
}

static int _upp_baska_hazir(void) {
    int k;
    if (!_upp_gorev_kuruldu) {
        return 0;
    }
    k = _upp_gorev_sec();
    return k >= 0 && k != _upp_gorev_cur;
}

static void _upp_gorev_giris(void) {
    UppGorev* g = &_upp_gorev[_upp_gorev_cur];
    g->f(g->arg);
    _upp_gorev[_upp_gorev_cur].durum = _UPP_G_BITTI;
    _upp_gecis();
    _upp_sonsuz_dur();
}

static long long upp_cekirdek_gorev_baslat(void (*f)(long long), long long arg, const char* ad) {
    int k, i;
    UppGorev* g;
    uint64_t* sp;
    _upp_gorev_ilk();
    if (!f) {
        upp_panik("görev başlatılamadı: fonksiyon değeri 'yok'");
    }
    _upp_gorev_temizle();
    for (k = 1; k < _UPP_GOREV_N; k++) {
        if (_upp_gorev[k].durum == _UPP_G_BOS) {
            break;
        }
    }
    if (k == _UPP_GOREV_N) {
        return -1;
    }
    g = &_upp_gorev[k];
    g->yigin = (unsigned char*)UPP_AYIR(_UPP_GOREV_YIGIN);
    if (!g->yigin) {
        return -1;
    }
    sp = (uint64_t*)(((uintptr_t)(g->yigin + _UPP_GOREV_YIGIN)) & ~(uintptr_t)15u);
    *--sp = 0;
    *--sp = (uint64_t)(uintptr_t)_upp_gorev_giris;
    *--sp = 0x202; /* RFLAGS: IF=1 */
    for (i = 0; i < 6; i++) {
        *--sp = 0;
    }
    g->rsp = (uint64_t)(uintptr_t)sp;
    g->f = f;
    g->arg = arg;
    g->ad = ad ? upp_metin_kopya(ad) : _upp_m_c("");
    g->kimlik = _upp_gorev_sonraki++;
    g->baslama = _upp_tik;
    g->uyan = 0;
    g->ka = 0;
    g->kul_kayit = 0;
    g->kul_on_plan = 0;
    g->kul_cikis = 0;
    g->kul_fs = 0;
    g->kul_gs = 0;
    g->kul_hata[0] = 0;
    g->kul_sc = NULL;
    g->kul_yurut = 0;
    g->durum = _UPP_G_HAZIR;
    return g->kimlik;
}

static void upp_cekirdek_sira_ver(void) {
    _upp_gorev_ilk();
    _upp_gecis();
}

static int _upp_gorev_bul(long long kimlik) {
    int k;
    _upp_gorev_ilk();
    for (k = 0; k < _UPP_GOREV_N; k++) {
        int d = _upp_gorev[k].durum;
        if ((d == _UPP_G_HAZIR || d == _UPP_G_UYUYOR) && _upp_gorev[k].kimlik == kimlik) {
            return k;
        }
    }
    return -1;
}

static long long upp_cekirdek_gorev_kimlik(void) {
    _upp_gorev_ilk();
    return _upp_gorev[_upp_gorev_cur].kimlik;
}

static UppKolListe upp_cekirdek_gorev_listesi(void) {
    UppKolListe L = {0};
    int k;
    _upp_gorev_ilk();
    for (k = 0; k < _UPP_GOREV_N; k++) {
        int d = _upp_gorev[k].durum;
        if (d == _UPP_G_HAZIR || d == _UPP_G_UYUYOR) {
            *(long long*)upp_liste_ekle_ptr(&L, &upp_kt_sayi) = _upp_gorev[k].kimlik;
        }
    }
    return L;
}

static char* upp_cekirdek_gorev_ad(long long kimlik) {
    int k = _upp_gorev_bul(kimlik);
    if (k < 0) {
        return _upp_m_c("");
    }
    return upp_metin_kopya(_upp_gorev[k].ad);
}

static char* upp_cekirdek_gorev_durum(long long kimlik) {
    int k = _upp_gorev_bul(kimlik);
    if (k < 0) {
        return _upp_m_c("");
    }
    if (k == _upp_gorev_cur) {
        return _upp_m_c("çalışıyor");
    }
    return _upp_m_c(_upp_gorev[k].durum == _UPP_G_UYUYOR ? "uyuyor" : "hazır");
}

static long long upp_cekirdek_gorev_baslama(long long kimlik) {
    int k = _upp_gorev_bul(kimlik);
    if (k < 0) {
        return -1;
    }
    return (long long)(_upp_gorev[k].baslama * 1000u / _UPP_TIK_HZ);
}

static bool upp_cekirdek_gorev_oldur(long long kimlik) {
    int k = _upp_gorev_bul(kimlik);
    if (k <= 0) {
        return false;
    }
    _upp_gorev[k].durum = _UPP_G_BITTI;
    if (k == _upp_gorev_cur) {
        _upp_gecis();
    }
    _upp_gorev_temizle();
    return true;
}

/* =====================================================================
 * Gerçek zamanlı saat (CMOS), güç, ekran, işlemci adı
 * ===================================================================== */
static uint8_t _upp_cmos(uint8_t r) {
    _upp_outb(0x70, r);
    return _upp_inb(0x71);
}

static int _upp_bcd(int v) {
    return (v & 0x0F) + (v >> 4) * 10;
}

static void _upp_mk_iki(char** s, int v) {
    char b[2];
    b[0] = (char)('0' + (v / 10) % 10);
    b[1] = (char)('0' + v % 10);
    _upp_m_ekle_n(s, b, 2u, 1);
}

/* "YYYY-AA-GG SS:DD:ss" (CMOS saati; QEMU'da UTC) */
static char* upp_cekirdek_tarih(void) {
    int i, sn, dk, sa, gun, ay, yil, b;
    char* s;
    for (i = 0; i < 100000 && (_upp_cmos(0x0A) & 0x80); i++) {
    }
    sn = _upp_cmos(0x00);
    dk = _upp_cmos(0x02);
    sa = _upp_cmos(0x04);
    gun = _upp_cmos(0x07);
    ay = _upp_cmos(0x08);
    yil = _upp_cmos(0x09);
    b = _upp_cmos(0x0B);
    if (!(b & 4)) {
        int pm = sa & 0x80;
        sn = _upp_bcd(sn);
        dk = _upp_bcd(dk);
        sa = _upp_bcd(sa & 0x7F) | pm;
        gun = _upp_bcd(gun);
        ay = _upp_bcd(ay);
        yil = _upp_bcd(yil);
    }
    if (!(b & 2) && (sa & 0x80)) {
        sa = ((sa & 0x7F) + 12) % 24;
    }
    sa &= 0x7F;
    s = upp_mk_bas();
    upp_mk_sayi(&s, 2000 + yil);
    _upp_m_ekle_n(&s, "-", 1u, 1);
    _upp_mk_iki(&s, ay);
    _upp_m_ekle_n(&s, "-", 1u, 1);
    _upp_mk_iki(&s, gun);
    _upp_m_ekle_n(&s, " ", 1u, 1);
    _upp_mk_iki(&s, sa);
    _upp_m_ekle_n(&s, ":", 1u, 1);
    _upp_mk_iki(&s, dk);
    _upp_m_ekle_n(&s, ":", 1u, 1);
    _upp_mk_iki(&s, sn);
    return s;
}

/* ACPI kapatma (QEMU/Bochs/VirtualBox), olmazsa QEMU isa-debug-exit. */
static void upp_cekirdek_kapat(void) {
    __asm__ volatile("cli");
    _upp_outw(0x604, 0x2000);
    _upp_outw(0xB004, 0x2000);
    _upp_outw(0x4004, 0x3400);
    _upp_outl(0xf4, 0);
    _upp_sonsuz_dur();
}

/* 8042 klavye denetleyicisiyle sıfırlama; olmazsa üçlü hata. */
static void upp_cekirdek_yeniden_baslat(void) {
    int i;
    UppIdtGosterge bos;
    __asm__ volatile("cli");
    for (i = 0; i < 100000 && (_upp_inb(0x64) & 2u); i++) {
    }
    _upp_outb(0x64, 0xFE);
    bos.sinir = 0;
    bos.taban = 0;
    __asm__ volatile("lidt %0\n int3" : : "m"(bos));
    _upp_sonsuz_dur();
}

static void upp_cekirdek_ekran_temizle(void) {
    const char* e = "\x1b[2J\x1b[H";
    size_t i;
    for (i = 0; e[i]; i++) {
        _upp_seri_bayt((unsigned char)e[i]);
    }
    _upp_vga_temizle();
}

static char* upp_cekirdek_islemci(void) {
    uint32_t r[12];
    uint32_t a, b, c, d;
    unsigned i;
    char* s;
    __asm__ volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(0x80000000u));
    if (a < 0x80000004u) {
        uint32_t v[3];
        __asm__ volatile("cpuid" : "=a"(a), "=b"(v[0]), "=c"(v[2]), "=d"(v[1]) : "a"(0u));
        s = upp_mk_bas();
        _upp_m_ekle_n(&s, (const char*)v, 12u, 1);
        return s;
    }
    for (i = 0; i < 3u; i++) {
        __asm__ volatile("cpuid" : "=a"(r[i * 4]), "=b"(r[i * 4 + 1]), "=c"(r[i * 4 + 2]), "=d"(r[i * 4 + 3]) : "a"(0x80000002u + i));
    }
    {
        const char* p = (const char*)r;
        size_t bas = 0, son = 48;
        while (bas < son && p[bas] == ' ') bas++;
        while (son > bas && (p[son - 1] == 0 || p[son - 1] == ' ')) son--;
        s = upp_mk_bas();
        _upp_m_ekle_n(&s, p + bas, son - bas, 1);
    }
    return s;
}

/* =====================================================================
 * ATA PIO disk sürücüsü: birincil/ikincil kanal, ana/yardımcı sürücü
 * (QEMU -drive if=ide,index=0..3 → disk 0..3). Kesmesiz (nIEN), yoklamalı.
 * LBA28, gerektiğinde LBA48. Sektör 512 bayt.
 * ===================================================================== */
typedef struct {
    int var;
    int lba48;
    uint64_t sektor;
    char model[41];
} _UppDisk;

static _UppDisk _upp_disk[4];
static int _upp_disk_tarandi = 0;

static uint16_t _upp_ata_taban(int d) { return d < 2 ? 0x1F0 : 0x170; }
static uint16_t _upp_ata_denetim(int d) { return d < 2 ? 0x3F6 : 0x376; }

static void _upp_ata_400ns(int d) {
    int i;
    for (i = 0; i < 4; i++) {
        (void)_upp_inb(_upp_ata_denetim(d));
    }
}

/* BSY bitinin düşmesini bekler; 0 = tamam, -1 = zaman aşımı/hata. */
static int _upp_ata_bekle(int d, int drq) {
    uint16_t b = _upp_ata_taban(d);
    long i;
    for (i = 0; i < 5000000L; i++) {
        uint8_t s = _upp_inb((uint16_t)(b + 7));
        if (s == 0xFF) {
            return -1;
        }
        if (s & 0x80) {
            continue;
        }
        if (s & 0x21) {
            return -1;
        }
        if (!drq || (s & 0x08)) {
            return 0;
        }
    }
    return -1;
}

static void _upp_disk_tara(void) {
    int d;
    if (_upp_disk_tarandi) {
        return;
    }
    _upp_disk_tarandi = 1;
    for (d = 0; d < 4; d++) {
        uint16_t b = _upp_ata_taban(d);
        uint16_t w[256];
        uint8_t s;
        int i;
        _upp_disk[d].var = 0;
        _upp_outb(_upp_ata_denetim(d), 0x02);
        _upp_outb((uint16_t)(b + 6), (uint8_t)(0xA0 | ((d & 1) << 4)));
        _upp_ata_400ns(d);
        _upp_outb((uint16_t)(b + 2), 0);
        _upp_outb((uint16_t)(b + 3), 0);
        _upp_outb((uint16_t)(b + 4), 0);
        _upp_outb((uint16_t)(b + 5), 0);
        _upp_outb((uint16_t)(b + 7), 0xEC);
        s = _upp_inb((uint16_t)(b + 7));
        if (s == 0 || s == 0xFF) {
            continue;
        }
        for (i = 0; i < 1000000 && (_upp_inb((uint16_t)(b + 7)) & 0x80); i++) {
        }
        if (_upp_inb((uint16_t)(b + 4)) != 0 || _upp_inb((uint16_t)(b + 5)) != 0) {
            continue; /* ATAPI ya da SATA: desteklenmiyor */
        }
        if (_upp_ata_bekle(d, 1) != 0) {
            continue;
        }
        for (i = 0; i < 256; i++) {
            w[i] = _upp_inw(b);
        }
        _upp_disk[d].var = 1;
        _upp_disk[d].lba48 = (w[83] & (1u << 10)) != 0;
        if (_upp_disk[d].lba48) {
            _upp_disk[d].sektor = (uint64_t)w[100] | ((uint64_t)w[101] << 16) | ((uint64_t)w[102] << 32) | ((uint64_t)w[103] << 48);
        } else {
            _upp_disk[d].sektor = (uint64_t)w[60] | ((uint64_t)w[61] << 16);
        }
        for (i = 0; i < 20; i++) {
            _upp_disk[d].model[2 * i] = (char)(w[27 + i] >> 8);
            _upp_disk[d].model[2 * i + 1] = (char)(w[27 + i] & 0xFF);
        }
        _upp_disk[d].model[40] = 0;
        for (i = 39; i >= 0 && _upp_disk[d].model[i] == ' '; i--) {
            _upp_disk[d].model[i] = 0;
        }
    }
}

static bool upp_cekirdek_disk_var(long long d) {
    _upp_disk_tara();
    return d >= 0 && d < 4 && _upp_disk[d].var;
}

static long long upp_cekirdek_disk_sektor(long long d) {
    if (!upp_cekirdek_disk_var(d)) {
        return 0;
    }
    return (long long)_upp_disk[d].sektor;
}

static char* upp_cekirdek_disk_model(long long d) {
    if (!upp_cekirdek_disk_var(d)) {
        return _upp_m_c("");
    }
    return _upp_m_c(_upp_disk[d].model);
}

/* adet sektörü (≤ 256) tek komutla aktarır. */
static int _upp_ata_aktar(int d, uint64_t lba, int adet, unsigned char* p, int yaz) {
    uint16_t b = _upp_ata_taban(d);
    int i, k;
    if (_upp_ata_bekle(d, 0) != 0) {
        return 0;
    }
    if (_upp_disk[d].lba48) {
        _upp_outb((uint16_t)(b + 6), (uint8_t)(0x40 | ((d & 1) << 4)));
        _upp_ata_400ns(d);
        _upp_outb((uint16_t)(b + 2), (uint8_t)((adet >> 8) & 0xFF));
        _upp_outb((uint16_t)(b + 3), (uint8_t)(lba >> 24));
        _upp_outb((uint16_t)(b + 4), (uint8_t)(lba >> 32));
        _upp_outb((uint16_t)(b + 5), (uint8_t)(lba >> 40));
        _upp_outb((uint16_t)(b + 2), (uint8_t)(adet & 0xFF));
        _upp_outb((uint16_t)(b + 3), (uint8_t)lba);
        _upp_outb((uint16_t)(b + 4), (uint8_t)(lba >> 8));
        _upp_outb((uint16_t)(b + 5), (uint8_t)(lba >> 16));
        _upp_outb((uint16_t)(b + 7), yaz ? 0x34 : 0x24);
    } else {
        _upp_outb((uint16_t)(b + 6), (uint8_t)(0xE0 | ((d & 1) << 4) | ((lba >> 24) & 0x0F)));
        _upp_ata_400ns(d);
        _upp_outb((uint16_t)(b + 2), (uint8_t)(adet & 0xFF));
        _upp_outb((uint16_t)(b + 3), (uint8_t)lba);
        _upp_outb((uint16_t)(b + 4), (uint8_t)(lba >> 8));
        _upp_outb((uint16_t)(b + 5), (uint8_t)(lba >> 16));
        _upp_outb((uint16_t)(b + 7), yaz ? 0x30 : 0x20);
    }
    for (k = 0; k < adet; k++) {
        _upp_ata_400ns(d);
        if (_upp_ata_bekle(d, 1) != 0) {
            return 0;
        }
        for (i = 0; i < 256; i++) {
            unsigned char* q = p + (size_t)k * 512u + (size_t)i * 2u;
            if (yaz) {
                _upp_outw(b, (uint16_t)(q[0] | (q[1] << 8)));
            } else {
                uint16_t w = _upp_inw(b);
                q[0] = (unsigned char)w;
                q[1] = (unsigned char)(w >> 8);
            }
        }
    }
    if (yaz) {
        _upp_outb((uint16_t)(b + 7), _upp_disk[d].lba48 ? 0xEA : 0xE7);
        if (_upp_ata_bekle(d, 0) != 0) {
            return 0;
        }
    }
    return 1;
}

static bool _upp_disk_gc(long long d, long long lba, long long adet, long long t, long long ofs, int yaz) {
    unsigned char* p;
    if (!upp_cekirdek_disk_var(d) || lba < 0 || adet < 0 || (uint64_t)(lba + adet) > _upp_disk[d].sektor) {
        return false;
    }
    p = _upp_tampon_p(t, ofs, adet * 512);
    while (adet > 0) {
        int n = adet > 128 ? 128 : (int)adet;
        if (!_upp_ata_aktar((int)d, (uint64_t)lba, n, p, yaz)) {
            return false;
        }
        adet -= n;
        lba += n;
        p += (size_t)n * 512u;
    }
    return true;
}

static bool upp_cekirdek_disk_oku(long long d, long long lba, long long adet, long long t, long long ofs) {
    return _upp_disk_gc(d, lba, adet, t, ofs, 0);
}

static bool upp_cekirdek_disk_yaz(long long d, long long lba, long long adet, long long t, long long ofs) {
    return _upp_disk_gc(d, lba, adet, t, ofs, 1);
}

/* ---- girdi: PS/2 kuyruğu + COM1 (UTF-8) ---- */
static int _upp_seri_var(void) {
    _upp_seri_kur();
    return (_upp_inb(_UPP_COM1 + 5) & 1u) != 0;
}

static int _upp_seri_son_cr = 0;

/* Seri girdiden bir kod noktası; CR → LF, CR LF çifti tek satır sonudur (0 = tuş yok). */
static uint32_t _upp_seri_cp(void) {
    uint32_t c = _upp_inb(_UPP_COM1);
    if (c == '\n' && _upp_seri_son_cr) {
        _upp_seri_son_cr = 0;
        return 0;
    }
    _upp_seri_son_cr = c == '\r';
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

/* Tuş gelene kadar bekler; bu sırada hazır görevler çalışır. */
static long long upp_cekirdek_tus_bekle(void) {
    upp_cekirdek_kesmeler_kur();
    for (;;) {
        long long c = upp_cekirdek_tus_oku();
        if (c) {
            return c;
        }
        if (_upp_baska_hazir()) {
            _upp_gecis();
        } else {
            __asm__ volatile("hlt");
        }
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
    _upp_gorev_ilk();
    _upp_gorev[_upp_gorev_cur].uyan = hedef;
    _upp_gorev[_upp_gorev_cur].durum = _UPP_G_UYUYOR;
    _upp_gecis();
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
/* =====================================================================
 * Kullanıcı kipi (ring 3) ve Linux uyumlu sistem çağrıları — mekanizma.
 *
 * Mimari
 *  - Çekirdek tamamen üst yarıdadır: görüntü 0xffffffff80000000'da, tüm
 *    fiziksel bellek HHDM'de (bkz. _upp_limine_kur). Alt yarı
 *    (0 .. 0x00007fffffffffff) süreçlerindir. Her sürecin kendi PML4'ü
 *    vardır; üst yarı girdileri çekirdeğin PML4'ünden paylaşılır, kullanıcı
 *    biti (U/S) yalnızca alt yarıdaki eşlemelerde açıktır.
 *  - GDT: 0x08 çekirdek kod, 0x10 çekirdek veri, 0x23 kullanıcı veri,
 *    0x2B kullanıcı kod (64-bit), 0x30 TSS. Sıralama SYSRET'in istediği
 *    gibidir (STAR[63:48] = 0x18).
 *  - Sistem çağrıları Linux x86_64 ABI'siyle SYSCALL/SYSRET üzerinden gelir:
 *    rax numara; rdi, rsi, rdx, r10, r8, r9 argüman; dönüş rax (hata -errno).
 *    rcx ve r11 dışındaki yazmaçlar (SSE dahil) korunur.
 *  - Her süreç bir çekirdek görevinde çalışır. Görev geçişi CR3'ü ve
 *    TSS.rsp0'ı değiştirir. Çekirdek kodu işbirlikçidir; ring 3 kodu ise
 *    zamanlayıcıyla kesilir ve 10 ms'lik dilimi dolunca sırasını verir.
 *  - Ring 3'teki bir istisna yalnızca o süreci sonlandırır (Linux'taki
 *    sinyaller gibi, çıkış kodu 128 + sinyal); çekirdek panik vermez.
 *  - Mekanizma burada (C), politika Uppix'tedir (u++): sistem çağrılarının
 *    anlamı, VFS, dosya tanımlayıcıları, ELF yükleme, mmap yerleşimi.
 *    Çekirdek kullanıcı belleğine hiçbir zaman kullanıcı adresiyle değil,
 *    sayfa tablosunu yürüyüp izinleri denetleyerek HHDM üzerinden erişir;
 *    işlemci destekliyorsa SMEP ve SMAP da açılır.
 *
 * Yalnızca Limine kipinde (HHDM) kullanılabilir; Multiboot'ta çekirdek alt
 * yarıdaki ilk 1 GiB'ı birebir kullandığından kullanıcı alanı yoktur.
 * ===================================================================== */

#define _UPP_KUL_UST 0x0000800000000000ull /* kullanıcı alanı üst sınırı (hariç) */
#define _UPP_KUL_ALT 0x10000ull            /* altı eşlenmez: boş gösterici hata versin */

#define _UPP_SY_P 0x1ull
#define _UPP_SY_RW 0x2ull
#define _UPP_SY_US 0x4ull
#define _UPP_SY_PWT 0x8ull
#define _UPP_SY_PCD 0x10ull
#define _UPP_SY_SAHIP 0x200ull /* yazılım biti: çerçeve bu eşlemenindir, kaldırılınca bırakılır */
#define _UPP_SY_NX 0x8000000000000000ull
#define _UPP_SY_ADRES 0x000FFFFFFFFFF000ull

typedef struct __attribute__((packed)) {
    uint32_t ayrilmis0;
    uint64_t rsp0, rsp1, rsp2;
    uint64_t ayrilmis1;
    uint64_t ist[7];
    uint64_t ayrilmis2;
    uint16_t ayrilmis3;
    uint16_t iomap;
} UppTss;

/* SYSCALL girişinin yığın çerçevesi (bkz. _upp_sc_giris). */
typedef struct {
    uint64_t no, a1, a2, a3, a4, a5, a6; /* rax, rdi, rsi, rdx, r10, r8, r9 */
    uint64_t r15, r14, r13, r12, rbx, rbp;
    uint64_t rip, rflags, rsp;
} UppSistemCagrisi;

static uint64_t _upp_gdt[8] __attribute__((aligned(16)));
__attribute__((used)) static UppTss _upp_tss __attribute__((aligned(16)));
static unsigned char _upp_cift_hata_yigini[16384] __attribute__((aligned(16)));
__attribute__((used)) static uint64_t _upp_sc_kul_rsp = 0; /* SYSCALL anında kullanıcı yığını */
__attribute__((used)) static uint64_t _upp_sc_cek_rsp = 0; /* şimdiki sürecin çekirdek yığını */
static int _upp_kul_hazir = 0;
static int _upp_nx_var = 0;
static long long (*_upp_sc_isleyici)(long long, long long, long long, long long, long long, long long, long long) = NULL;

__asm__(
    ".text\n"
    /* long long _upp_kul_gir(uint64_t* kayit, uint64_t giris, uint64_t yigin):
     * çekirdek bağlamını kaydeder ve ring 3'e geçer. Süreç bitince
     * _upp_kul_don buraya döner (rax = çıkış kodu). */
    ".global _upp_kul_gir\n"
    "_upp_kul_gir:\n"
    "    push %rbp\n    push %rbx\n    push %r12\n    push %r13\n    push %r14\n    push %r15\n"
    "    cli\n"
    "    mov %rsp, (%rdi)\n"
    "    mov %rsp, _upp_tss+4(%rip)\n"
    "    mov %rsp, _upp_sc_cek_rsp(%rip)\n"
    "    pushq $0x23\n"
    "    push %rdx\n"
    "    pushq $0x202\n"
    "    pushq $0x2B\n"
    "    push %rsi\n"
    /* Çekirdeğin yazmaç ve SSE içeriği kullanıcıya sızmasın. */
    "    xor %eax, %eax\n    xor %ebx, %ebx\n    xor %ecx, %ecx\n    xor %edx, %edx\n"
    "    xor %esi, %esi\n    xor %edi, %edi\n    xor %ebp, %ebp\n"
    "    xor %r8d, %r8d\n    xor %r9d, %r9d\n    xor %r10d, %r10d\n    xor %r11d, %r11d\n"
    "    xor %r12d, %r12d\n    xor %r13d, %r13d\n    xor %r14d, %r14d\n    xor %r15d, %r15d\n"
    "    fninit\n"
    "    sub $16, %rsp\n    movl $0x1F80, (%rsp)\n    ldmxcsr (%rsp)\n    add $16, %rsp\n"
    "    pxor %xmm0, %xmm0\n    pxor %xmm1, %xmm1\n    pxor %xmm2, %xmm2\n    pxor %xmm3, %xmm3\n"
    "    pxor %xmm4, %xmm4\n    pxor %xmm5, %xmm5\n    pxor %xmm6, %xmm6\n    pxor %xmm7, %xmm7\n"
    "    pxor %xmm8, %xmm8\n    pxor %xmm9, %xmm9\n    pxor %xmm10, %xmm10\n    pxor %xmm11, %xmm11\n"
    "    pxor %xmm12, %xmm12\n    pxor %xmm13, %xmm13\n    pxor %xmm14, %xmm14\n    pxor %xmm15, %xmm15\n"
    "    iretq\n"
    /* void _upp_kul_don(uint64_t kayit, long long kod): _upp_kul_gir'in
     * kaydettiği bağlama döner; aradaki çekirdek çerçeveleri atılır. */
    ".global _upp_kul_don\n"
    "_upp_kul_don:\n"
    "    mov %rdi, %rsp\n"
    "    mov %rsi, %rax\n"
    "    pop %r15\n    pop %r14\n    pop %r13\n    pop %r12\n    pop %rbx\n    pop %rbp\n"
    "    ret\n"
    /* SYSCALL girişi (LSTAR). IF=0 gelir (FMASK); tek işlemci olduğu için
     * kullanıcı yığını genel bir değişkende kısa süre tutulabilir. */
    ".global _upp_sc_giris\n"
    "_upp_sc_giris:\n"
    "    mov %rsp, _upp_sc_kul_rsp(%rip)\n"
    "    mov _upp_sc_cek_rsp(%rip), %rsp\n"
    "    and $-16, %rsp\n"
    "    pushq _upp_sc_kul_rsp(%rip)\n"
    "    push %r11\n"
    "    push %rcx\n"
    "    push %rbp\n    push %rbx\n    push %r12\n    push %r13\n    push %r14\n    push %r15\n"
    "    push %r9\n    push %r8\n    push %r10\n    push %rdx\n    push %rsi\n    push %rdi\n    push %rax\n"
    "    mov %rsp, %rbx\n"
    "    sub $512, %rsp\n"
    "    fxsave (%rsp)\n"
    "    sub $16, %rsp\n    movl $0x1F80, (%rsp)\n    ldmxcsr (%rsp)\n    add $16, %rsp\n"
    "    fninit\n"
    "    sti\n"
    "    mov %rbx, %rdi\n"
    "    call _upp_sc_isle\n"
    "    cli\n"
    "    fxrstor (%rsp)\n"
    "    mov %rbx, %rsp\n"
    "    add $8, %rsp\n"
    "    pop %rdi\n    pop %rsi\n    pop %rdx\n    pop %r10\n    pop %r8\n    pop %r9\n"
    "    pop %r15\n    pop %r14\n    pop %r13\n    pop %r12\n    pop %rbx\n    pop %rbp\n"
    "    pop %rcx\n"
    "    pop %r11\n"
    "    pop %rsp\n"
    "    sysretq\n"
    /* long long _upp_kul_gir_tam(uint64_t* kayit, const uint64_t* y, const void* fx):
     * _upp_kul_gir gibi, ama ring 3'e kaydedilmiş bir yazmaç kümesiyle girer
     * (fork ile doğan süreç). y: _UPP_CER_* sırasıyla 16 yazmaç; fx: 16 bayta
     * hizalı 512 baytlık FXSAVE alanı. rcx/r11 SYSCALL dönüşündeki gibi
     * rip/rflags olur. */
    ".global _upp_kul_gir_tam\n"
    "_upp_kul_gir_tam:\n"
    "    push %rbp\n    push %rbx\n    push %r12\n    push %r13\n    push %r14\n    push %r15\n"
    "    cli\n"
    "    mov %rsp, (%rdi)\n"
    "    mov %rsp, _upp_tss+4(%rip)\n"
    "    mov %rsp, _upp_sc_cek_rsp(%rip)\n"
    "    fxrstor (%rdx)\n"
    "    pushq $0x23\n"
    "    pushq 120(%rsi)\n"
    "    pushq 112(%rsi)\n"
    "    pushq $0x2B\n"
    "    pushq 104(%rsi)\n"
    "    mov 0(%rsi), %rax\n"
    "    mov 8(%rsi), %rdi\n"
    "    mov 24(%rsi), %rdx\n"
    "    mov 32(%rsi), %r10\n"
    "    mov 40(%rsi), %r8\n"
    "    mov 48(%rsi), %r9\n"
    "    mov 56(%rsi), %r15\n"
    "    mov 64(%rsi), %r14\n"
    "    mov 72(%rsi), %r13\n"
    "    mov 80(%rsi), %r12\n"
    "    mov 88(%rsi), %rbx\n"
    "    mov 96(%rsi), %rbp\n"
    "    mov 104(%rsi), %rcx\n"
    "    mov 112(%rsi), %r11\n"
    "    mov 16(%rsi), %rsi\n"
    "    iretq\n"
    /* void _upp_kul_atla(uint64_t kayit, uint64_t giris, uint64_t yigin):
     * sürecin çekirdek yığınını kayıt noktasına geri sarar ve yeni görüntünün
     * girişine temiz yazmaçlarla geçer (execve). */
    ".global _upp_kul_atla\n"
    "_upp_kul_atla:\n"
    "    cli\n"
    "    mov %rdi, %rsp\n"
    "    pushq $0x23\n"
    "    push %rdx\n"
    "    pushq $0x202\n"
    "    pushq $0x2B\n"
    "    push %rsi\n"
    "    xor %eax, %eax\n    xor %ebx, %ebx\n    xor %ecx, %ecx\n    xor %edx, %edx\n"
    "    xor %esi, %esi\n    xor %edi, %edi\n    xor %ebp, %ebp\n"
    "    xor %r8d, %r8d\n    xor %r9d, %r9d\n    xor %r10d, %r10d\n    xor %r11d, %r11d\n"
    "    xor %r12d, %r12d\n    xor %r13d, %r13d\n    xor %r14d, %r14d\n    xor %r15d, %r15d\n"
    "    fninit\n"
    "    sub $16, %rsp\n    movl $0x1F80, (%rsp)\n    ldmxcsr (%rsp)\n    add $16, %rsp\n"
    "    pxor %xmm0, %xmm0\n    pxor %xmm1, %xmm1\n    pxor %xmm2, %xmm2\n    pxor %xmm3, %xmm3\n"
    "    pxor %xmm4, %xmm4\n    pxor %xmm5, %xmm5\n    pxor %xmm6, %xmm6\n    pxor %xmm7, %xmm7\n"
    "    pxor %xmm8, %xmm8\n    pxor %xmm9, %xmm9\n    pxor %xmm10, %xmm10\n    pxor %xmm11, %xmm11\n"
    "    pxor %xmm12, %xmm12\n    pxor %xmm13, %xmm13\n    pxor %xmm14, %xmm14\n    pxor %xmm15, %xmm15\n"
    "    iretq\n");
extern long long _upp_kul_gir(uint64_t* kayit, uint64_t giris, uint64_t yigin);
extern long long _upp_kul_gir_tam(uint64_t* kayit, const uint64_t* y, const void* fx);
extern void _upp_kul_atla(uint64_t kayit, uint64_t giris, uint64_t yigin) __attribute__((noreturn));
extern void _upp_kul_don(uint64_t kayit, long long kod) __attribute__((noreturn));
extern void _upp_sc_giris(void);

static uint64_t _upp_msr_oku(uint32_t m) {
    uint32_t lo, hi;
    __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(m));
    return ((uint64_t)hi << 32) | lo;
}

static void _upp_msr_yaz(uint32_t m, uint64_t v) {
    __asm__ volatile("wrmsr" : : "a"((uint32_t)v), "d"((uint32_t)(v >> 32)), "c"(m));
}

static uint64_t _upp_cr3_oku(void) {
    uint64_t v;
    __asm__ volatile("mov %%cr3, %0" : "=r"(v));
    return v & _UPP_SY_ADRES;
}

static void _upp_cr3_yaz(uint64_t v) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(v) : "memory");
}

static inline uint64_t* _upp_fv(uint64_t fiz) {
    return (uint64_t*)(uintptr_t)(fiz + _upp_hhdm);
}

/* GDT + TSS, SYSCALL MSR'ları, NX/SMEP/SMAP. Limine kipinde açılışta bir kez. */
static void _upp_kul_kur(void) {
    struct __attribute__((packed)) {
        uint16_t sinir;
        uint64_t taban;
    } g;
    uint64_t tb = (uint64_t)(uintptr_t)&_upp_tss;
    uint32_t a, b, c, d;
    uint64_t cr4;
    if (_upp_kul_hazir || !_upp_hhdm || !_upp_cekirdek_pml4) {
        return;
    }
    upp_cekirdek_kesmeler_kur();
    memset(&_upp_tss, 0, sizeof(_upp_tss));
    _upp_tss.iomap = (uint16_t)sizeof(UppTss); /* G/Ç izin haritası yok: ring 3 port kullanamaz */
    _upp_tss.ist[0] = (uint64_t)(uintptr_t)(_upp_cift_hata_yigini + sizeof(_upp_cift_hata_yigini));
    _upp_gdt[0] = 0;
    _upp_gdt[1] = 0x00AF9B000000FFFFull; /* 0x08 çekirdek kod (64-bit) */
    _upp_gdt[2] = 0x00CF93000000FFFFull; /* 0x10 çekirdek veri */
    _upp_gdt[3] = 0x00CFFB000000FFFFull; /* 0x18 kullanıcı kod 32-bit: SYSRET düzeni için yer tutucu */
    _upp_gdt[4] = 0x00CFF3000000FFFFull; /* 0x20 kullanıcı veri (DPL 3) */
    _upp_gdt[5] = 0x00AFFB000000FFFFull; /* 0x28 kullanıcı kod (64-bit, DPL 3) */
    _upp_gdt[6] = (uint64_t)(sizeof(UppTss) - 1u) | ((tb & 0xFFFFFFull) << 16) | (0x89ull << 40) | (((tb >> 24) & 0xFFull) << 56);
    _upp_gdt[7] = tb >> 32;
    g.sinir = (uint16_t)(sizeof(_upp_gdt) - 1u);
    g.taban = (uint64_t)(uintptr_t)_upp_gdt;
    __asm__ volatile(
        "lgdt %0\n"
        "pushq $0x08\n"
        "lea 1f(%%rip), %%rax\n"
        "pushq %%rax\n"
        "lretq\n"
        "1:\n"
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%ss\n"
        "xor %%eax, %%eax\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov $0x30, %%ax\n"
        "ltr %%ax\n"
        : : "m"(g) : "rax", "memory");
    /* Çift hata ayrı yığında (IST1): çekirdek yığını taşsa bile tanılanabilsin. */
    _upp_idt[8].ist = 1;
    __asm__ volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(0x80000001u), "c"(0u));
    _upp_nx_var = (int)((d >> 20) & 1u);
    _upp_msr_yaz(0xC0000080u, _upp_msr_oku(0xC0000080u) | 1u | (_upp_nx_var ? 0x800u : 0u)); /* EFER: SCE, NXE */
    _upp_msr_yaz(0xC0000081u, (0x18ull << 48) | (0x08ull << 32));                           /* STAR */
    _upp_msr_yaz(0xC0000082u, (uint64_t)(uintptr_t)_upp_sc_giris);                          /* LSTAR */
    _upp_msr_yaz(0xC0000084u, 0x47700u); /* FMASK: TF, IF, DF, IOPL, AC, NT temizlenir */
    /* SMEP/SMAP: çekirdek kullanıcı sayfasını yürütemez / doğrudan okuyamaz. */
    __asm__ volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(0u), "c"(0u));
    if (a >= 7u) {
        __asm__ volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(7u), "c"(0u));
        __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
        if (b & (1u << 7)) cr4 |= 1ull << 20;
        if (b & (1u << 20)) cr4 |= 1ull << 21;
        __asm__ volatile("mov %0, %%cr4" : : "r"(cr4));
    }
    _upp_kul_hazir = 1;
}

/* ---- adres alanları ---- */

static int _upp_kul_aralik(long long adres, long long uzunluk, uint64_t* bas, uint64_t* son) {
    uint64_t a = (uint64_t)adres, n = (uint64_t)uzunluk;
    if (adres < 0 || uzunluk <= 0 || a < _UPP_KUL_ALT || a >= _UPP_KUL_UST || n > _UPP_KUL_UST - a) {
        return 0;
    }
    *bas = a & ~0xFFFull;
    *son = (a + n + 0xFFFull) & ~0xFFFull;
    return 1;
}

/* va'nın sayfa girdisi; olustur ise eksik ara tablolar kurulur (kullanıcıya açık). */
static uint64_t* _upp_kul_pte(uint64_t ka, uint64_t va, int olustur) {
    uint64_t* t = _upp_fv(ka);
    int seviye;
    for (seviye = 3; seviye > 0; seviye--) {
        unsigned i = (unsigned)((va >> (12 + 9 * seviye)) & 511u);
        if (!(t[i] & _UPP_SY_P)) {
            uint64_t y;
            if (!olustur) {
                return NULL;
            }
            y = _upp_cer_ayir();
            if (!y) {
                return NULL;
            }
            t[i] = y | _UPP_SY_P | _UPP_SY_RW | _UPP_SY_US;
        }
        t = _upp_fv(t[i] & _UPP_SY_ADRES);
    }
    return &t[(va >> 12) & 511u];
}

static void _upp_kul_tlb(uint64_t ka, uint64_t va) {
    if (_upp_cr3_oku() == ka) {
        __asm__ volatile("invlpg (%0)" : : "r"(va) : "memory");
    }
}

/* Linux PROT_* (1 okuma, 2 yazma, 4 yürütme) → sayfa bayrakları. */
static uint64_t _upp_kul_bayrak(long long koruma) {
    uint64_t b = _UPP_SY_P | _UPP_SY_US;
    if (koruma & 2) b |= _UPP_SY_RW;
    if (!(koruma & 4) && _upp_nx_var) b |= _UPP_SY_NX;
    return b;
}

static void _upp_kul_alan_sil(uint64_t ka) {
    uint64_t* l4;
    unsigned a, b, c, d;
    if (!ka) {
        return;
    }
    if (_upp_cr3_oku() == ka) {
        _upp_cr3_yaz(_upp_cekirdek_cr3);
    }
    l4 = _upp_fv(ka);
    for (a = 0; a < 256u; a++) {
        uint64_t* l3;
        if (!(l4[a] & _UPP_SY_P)) continue;
        l3 = _upp_fv(l4[a] & _UPP_SY_ADRES);
        for (b = 0; b < 512u; b++) {
            uint64_t* l2;
            if (!(l3[b] & _UPP_SY_P)) continue;
            l2 = _upp_fv(l3[b] & _UPP_SY_ADRES);
            for (c = 0; c < 512u; c++) {
                uint64_t* l1;
                if (!(l2[c] & _UPP_SY_P)) continue;
                l1 = _upp_fv(l2[c] & _UPP_SY_ADRES);
                for (d = 0; d < 512u; d++) {
                    if ((l1[d] & (_UPP_SY_P | _UPP_SY_SAHIP)) == (_UPP_SY_P | _UPP_SY_SAHIP)) {
                        _upp_cer_birak(l1[d] & _UPP_SY_ADRES);
                    }
                }
                _upp_cer_birak(l2[c] & _UPP_SY_ADRES);
            }
            _upp_cer_birak(l3[b] & _UPP_SY_ADRES);
        }
        _upp_cer_birak(l4[a] & _UPP_SY_ADRES);
    }
    _upp_cer_birak(ka);
}

/* Kullanıcı belleğiyle kopyalama: her sayfanın P|U(|W) izni denetlenir, erişim
 * HHDM üzerindendir. Eşlenmemiş ya da izinsiz bir sayfada 0 döner (EFAULT). */
static int _upp_kul_kopya(uint64_t ka, uint64_t va, unsigned char* p, size_t n, int yaz) {
    uint64_t gerek = _UPP_SY_P | _UPP_SY_US | (yaz ? _UPP_SY_RW : 0u);
    if (!ka || va >= _UPP_KUL_UST || n > _UPP_KUL_UST - va) {
        return 0;
    }
    while (n) {
        uint64_t* pte = _upp_kul_pte(ka, va, 0);
        size_t ofs = (size_t)(va & 0xFFFu);
        size_t parca = 4096u - ofs;
        unsigned char* k;
        if (!pte || (*pte & gerek) != gerek) {
            return 0;
        }
        if (parca > n) {
            parca = n;
        }
        k = (unsigned char*)_upp_fv(*pte & _UPP_SY_ADRES) + ofs;
        if (yaz) {
            memcpy(k, p, parca);
        } else {
            memcpy(p, k, parca);
        }
        va += parca;
        p += parca;
        n -= parca;
    }
    return 1;
}

/* ---- görev geçişi, istisna ve kesme kancaları (upp_serbest.c çağırır) ---- */

/* FS_BASE / GS_BASE (MSR 0xC0000100 / 0xC0000101): sürecin TLS tabanı. */
static void _upp_kul_tls_yukle(const UppGorev* g) {
    _upp_msr_yaz(0xC0000100u, g->kul_fs);
    _upp_msr_yaz(0xC0000101u, g->kul_gs);
}

static void _upp_kul_gecis_hazirla(int k) {
    UppGorev* g = &_upp_gorev[k];
    if (!g->ka) {
        return;
    }
    if (_upp_cr3_oku() != g->ka) {
        _upp_cr3_yaz(g->ka);
    }
    _upp_kul_tls_yukle(g);
    _upp_tss.rsp0 = g->kul_kayit;
    _upp_sc_cek_rsp = g->kul_kayit;
    g->kul_dilim = _upp_tik;
}

static void _upp_hex_ad(char* h, size_t kap, uint64_t v) {
    char b[19];
    int i;
    b[0] = '0';
    b[1] = 'x';
    for (i = 0; i < 16; i++) {
        unsigned r = (unsigned)((v >> (60 - 4 * i)) & 15u);
        b[2 + i] = (char)(r < 10 ? '0' + r : 'a' + r - 10);
    }
    b[18] = 0;
    _upp_ekle_ad(h, kap, b);
}

/* Süreci sinyalle sonlandırır: kayıtlı çekirdek bağlamına döner (128 + sinyal). */
static void _upp_kul_sonlandir(UppGorev* g, int sinyal, const char* neden) __attribute__((noreturn));
static void _upp_kul_sonlandir(UppGorev* g, int sinyal, const char* neden) {
    (void)neden;
    __asm__ volatile("cli");
    _upp_kul_don(g->kul_kayit, 128 + sinyal);
}

static void _upp_kul_istisna(UppKesmeCerceve* c) {
    UppGorev* g = &_upp_gorev[_upp_gorev_cur];
    int sinyal = 11;
    const char* ad = "Bölümleme hatası (SIGSEGV)";
    if (!g->ka) {
        upp_panik("ring 3 istisnası ama görevin kullanıcı süreci yok");
    }
    switch (c->vektor) {
    case 0: case 16: case 19:
        sinyal = 8; ad = "Aritmetik hata (SIGFPE)"; break;
    case 6:
        sinyal = 4; ad = "Geçersiz komut (SIGILL)"; break;
    case 1: case 3:
        sinyal = 5; ad = "İz tuzağı (SIGTRAP)"; break;
    case 17:
        sinyal = 7; ad = "Veri yolu hatası (SIGBUS)"; break;
    default:
        break;
    }
    _upp_kopyala_ad(g->kul_hata, sizeof(g->kul_hata), ad);
    _upp_ekle_ad(g->kul_hata, sizeof(g->kul_hata), ": komut ");
    _upp_hex_ad(g->kul_hata, sizeof(g->kul_hata), c->rip);
    if (c->vektor == 14) {
        uint64_t cr2;
        __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
        _upp_ekle_ad(g->kul_hata, sizeof(g->kul_hata), ", adres ");
        _upp_hex_ad(g->kul_hata, sizeof(g->kul_hata), cr2);
    }
    _upp_kul_sonlandir(g, sinyal, ad);
}

/* Kuyruktaki ilk c'yi çıkarır; bulunduysa 1. */
static int _upp_tus_cikar(uint32_t c) {
    unsigned i = _upp_tus_bas, j;
    while (i != _upp_tus_son) {
        if (_upp_tus_kuyruk[i] == c) {
            j = i;
            while (j != _upp_tus_bas) {
                unsigned o = (j + _UPP_TUS_N - 1u) % _UPP_TUS_N;
                _upp_tus_kuyruk[j] = _upp_tus_kuyruk[o];
                j = o;
            }
            _upp_tus_bas = (_upp_tus_bas + 1u) % _UPP_TUS_N;
            return 1;
        }
        i = (i + 1u) % _UPP_TUS_N;
    }
    return 0;
}

/* Ring 3 kesildiğinde (EOI gönderilmiş): seri girdi kuyruğa alınır (ring 3
 * dönerken kimse yoklamaz), ön plandaki süreç Ctrl-C ile SIGINT alır, dilimi
 * dolan süreç sırasını başka göreve verir. */
static void _upp_kul_kesme(UppKesmeCerceve* c) {
    UppGorev* g = &_upp_gorev[_upp_gorev_cur];
    if (!g->ka) {
        return;
    }
    while ((_upp_tus_son + 1u) % _UPP_TUS_N != _upp_tus_bas && _upp_seri_var()) {
        uint32_t cp = _upp_seri_cp();
        if (cp) {
            _upp_tus_koy(cp);
        }
    }
    if (g->kul_on_plan && _upp_tus_cikar(3u)) {
        _upp_kopyala_ad(g->kul_hata, sizeof(g->kul_hata), "^C");
        _upp_kul_sonlandir(g, 2, "SIGINT");
    }
    if (c->vektor == 32 && _upp_tik - g->kul_dilim >= 10u && _upp_baska_hazir()) {
        _upp_gecis();
        _upp_gorev[_upp_gorev_cur].kul_dilim = _upp_tik;
    }
}

/* SYSCALL girişinin C tarafı: Uppix'in kaydettiği işleyiciye iletir. */
__attribute__((used)) long long _upp_sc_isle(UppSistemCagrisi* f) {
    UppGorev* g = &_upp_gorev[_upp_gorev_cur];
    long long r = -38; /* ENOSYS */
    if (f->rip >= _UPP_KUL_UST) {
        /* SYSRET kanonik olmayan RIP'te çekirdekte #GP üretirdi. */
        _upp_kopyala_ad(g->kul_hata, sizeof(g->kul_hata), "Bölümleme hatası (SIGSEGV): geçersiz dönüş adresi");
        _upp_kul_sonlandir(g, 11, "SIGSEGV");
    }
    g->kul_sc = f;
    if (_upp_sc_isleyici) {
        r = _upp_sc_isleyici((long long)f->no, (long long)f->a1, (long long)f->a2, (long long)f->a3,
                             (long long)f->a4, (long long)f->a5, (long long)f->a6);
    }
    g = &_upp_gorev[_upp_gorev_cur];
    g->kul_sc = NULL;
    if (g->kul_cikis) {
        g->kul_cikis = 0;
        g->kul_yurut = 0;
        __asm__ volatile("cli");
        _upp_kul_don(g->kul_kayit, g->kul_kod);
    }
    if (g->kul_yurut) {
        /* execve: eski adres alanı bırakılır, süreç aynı görevde ve aynı
         * çekirdek yığınında yeni görüntünün girişinden sürer. */
        uint64_t eski = g->ka;
        g->kul_yurut = 0;
        __asm__ volatile("cli");
        g->ka = g->kul_y_ka;
        g->kul_fs = 0;
        g->kul_gs = 0;
        g->kul_dilim = _upp_tik;
        _upp_cr3_yaz(g->ka);
        _upp_kul_tls_yukle(g);
        if (eski && eski != g->ka) {
            _upp_kul_alan_sil(eski);
        }
        _upp_kul_atla(g->kul_kayit, g->kul_y_giris, g->kul_y_yigin);
    }
    return r;
}

/* ---- upp.cekirdek.* yerleşikleri (Uppix'in süreç ve sistem çağrısı katmanı) ---- */

static bool upp_cekirdek_kullanici_destekli(void) {
    _upp_kul_kur();
    return _upp_kul_hazir != 0;
}

/* Yeni adres alanı: alt yarı boş, üst yarı çekirdeğinki. Bellek yoksa 0. */
static long long upp_cekirdek_kullanici_alan_yeni(void) {
    uint64_t p;
    _upp_kul_kur();
    if (!_upp_kul_hazir) {
        return 0;
    }
    p = _upp_cer_ayir();
    if (!p) {
        return 0;
    }
    memcpy(_upp_fv(p) + 256, _upp_cekirdek_pml4 + 256, 256u * sizeof(uint64_t));
    return (long long)p;
}

static void upp_cekirdek_kullanici_alan_sil(long long ka) {
    _upp_kul_alan_sil((uint64_t)ka);
}

/* [adres, adres+uzunluk) sayfalarını sıfırlanmış yeni çerçevelerle eşler.
 * Zaten eşli sayfaların izinleri birleşir (ör. iki ELF bölümünün ortak sayfası).
 * koruma 0 (PROT_NONE) ise sayfa ayrılmaz. Bellek biterse yanlis. */
static bool upp_cekirdek_kullanici_esle(long long ka, long long adres, long long uzunluk, long long koruma) {
    uint64_t va, son;
    if (!ka || !_upp_kul_aralik(adres, uzunluk, &va, &son)) {
        return false;
    }
    if ((koruma & 7) == 0) {
        return true;
    }
    for (; va < son; va += 4096u) {
        uint64_t* pte = _upp_kul_pte((uint64_t)ka, va, 1);
        if (!pte) {
            return false;
        }
        if (*pte & _UPP_SY_P) {
            if (koruma & 2) *pte |= _UPP_SY_RW;
            if (koruma & 4) *pte &= ~_UPP_SY_NX;
        } else {
            uint64_t f = _upp_cer_ayir();
            if (!f) {
                return false;
            }
            *pte = f | _upp_kul_bayrak(koruma) | _UPP_SY_SAHIP;
        }
        _upp_kul_tlb((uint64_t)ka, va);
    }
    return true;
}

static void upp_cekirdek_kullanici_esleme_kaldir(long long ka, long long adres, long long uzunluk) {
    uint64_t va, son;
    if (!ka || !_upp_kul_aralik(adres, uzunluk, &va, &son)) {
        return;
    }
    for (; va < son; va += 4096u) {
        uint64_t* pte = _upp_kul_pte((uint64_t)ka, va, 0);
        if (pte && (*pte & _UPP_SY_P)) {
            if (*pte & _UPP_SY_SAHIP) {
                _upp_cer_birak(*pte & _UPP_SY_ADRES);
            }
            *pte = 0;
            _upp_kul_tlb((uint64_t)ka, va);
        }
    }
}

/* Eşli sayfaların iznini koruma'ya ayarlar (mprotect mekanizması). birlestir
 * ise yalnızca izin ekler (ELF bölümlerinin ortak sayfaları için). koruma 0
 * (PROT_NONE) sayfayı kullanıcıya kapatır; çerçeve süreçte kalır. */
static void upp_cekirdek_kullanici_koruma(long long ka, long long adres, long long uzunluk, long long koruma, bool birlestir) {
    uint64_t va, son;
    if (!ka || !_upp_kul_aralik(adres, uzunluk, &va, &son)) {
        return;
    }
    for (; va < son; va += 4096u) {
        uint64_t* pte = _upp_kul_pte((uint64_t)ka, va, 0);
        uint64_t yeni;
        if (!pte || !(*pte & _UPP_SY_P)) {
            continue;
        }
        yeni = *pte & (_UPP_SY_ADRES | _UPP_SY_SAHIP | _UPP_SY_PCD | _UPP_SY_PWT);
        if (birlestir) {
            yeni = *pte;
            if (koruma & 7) yeni |= _UPP_SY_US;
            if (koruma & 2) yeni |= _UPP_SY_RW;
            if (koruma & 4) yeni &= ~_UPP_SY_NX;
        } else if ((koruma & 7) == 0) {
            yeni |= _UPP_SY_P | (_upp_nx_var ? _UPP_SY_NX : 0u);
        } else {
            yeni |= _upp_kul_bayrak(koruma);
        }
        *pte = yeni;
        _upp_kul_tlb((uint64_t)ka, va);
    }
}

/* Aralıktaki hiçbir sayfa eşli değilse dogru (mmap yerleşimi için). */
static bool upp_cekirdek_kullanici_bos_mu(long long ka, long long adres, long long uzunluk) {
    uint64_t va, son;
    if (!ka || !_upp_kul_aralik(adres, uzunluk, &va, &son)) {
        return false;
    }
    for (; va < son; va += 4096u) {
        uint64_t* pte = _upp_kul_pte((uint64_t)ka, va, 0);
        if (pte && (*pte & _UPP_SY_P)) {
            return false;
        }
    }
    return true;
}

/* Aralıktaki her sayfa eşliyse dogru (mprotect/madvise ENOMEM denetimi). */
static bool upp_cekirdek_kullanici_esli_mi(long long ka, long long adres, long long uzunluk) {
    uint64_t va, son;
    if (!ka || !_upp_kul_aralik(adres, uzunluk, &va, &son)) {
        return false;
    }
    for (; va < son; va += 4096u) {
        uint64_t* pte = _upp_kul_pte((uint64_t)ka, va, 0);
        if (!pte || !(*pte & _UPP_SY_P)) {
            return false;
        }
    }
    return true;
}

/* Aygıt belleğini (çerçeve tamponu) kullanıcıya write-combining (PAT PA1) eşler. Çerçeveler
 * sürecin değildir: kaldırılınca bırakılmaz. Hedef aralık boş olmalı. */
static bool upp_cekirdek_kullanici_fiziksel_esle(long long ka, long long adres, long long fiziksel, long long uzunluk, long long koruma) {
    uint64_t va, son, fz = (uint64_t)fiziksel & ~0xFFFull;
    if (!ka || !_upp_kul_aralik(adres, uzunluk, &va, &son) || fiziksel < 0 || (koruma & 7) == 0) {
        return false;
    }
    for (; va < son; va += 4096u, fz += 4096u) {
        uint64_t* pte = _upp_kul_pte((uint64_t)ka, va, 1);
        if (!pte || (*pte & _UPP_SY_P)) {
            return false;
        }
        *pte = fz | _upp_kul_bayrak(koruma) | _UPP_SY_PWT;
        _upp_kul_tlb((uint64_t)ka, va);
    }
    return true;
}

static bool upp_cekirdek_kullanici_oku(long long ka, long long adres, long long t, long long ofs, long long n) {
    _UppTampon* b = _upp_tampon_al(t, ofs, n);
    return n == 0 || _upp_kul_kopya((uint64_t)ka, (uint64_t)adres, b->veri + ofs, (size_t)n, 0) != 0;
}

static bool upp_cekirdek_kullanici_yaz(long long ka, long long adres, long long t, long long ofs, long long n) {
    _UppTampon* b = _upp_tampon_al(t, ofs, n);
    return n == 0 || _upp_kul_kopya((uint64_t)ka, (uint64_t)adres, b->veri + ofs, (size_t)n, 1) != 0;
}

/* NUL ile biten kullanıcı dizgisi (yol, argv). Hatalı adres ya da en_cok
 * baytta NUL yoksa yok. */
static char* upp_cekirdek_kullanici_metin(long long ka, long long adres, long long en_cok) {
    char* tampon;
    char* m;
    long long i;
    if (en_cok <= 0 || en_cok > 65536) {
        return NULL;
    }
    tampon = (char*)UPP_AYIR((size_t)en_cok);
    if (!tampon) {
        return NULL;
    }
    for (i = 0; i < en_cok; i++) {
        unsigned char c;
        if (!_upp_kul_kopya((uint64_t)ka, (uint64_t)adres + (uint64_t)i, &c, 1u, 0)) {
            break;
        }
        if (c == 0) {
            m = _upp_m_bayttan(tampon, (size_t)i);
            UPP_BIRAK(tampon);
            return m;
        }
        tampon[i] = (char)c;
    }
    UPP_BIRAK(tampon);
    return NULL;
}

/* Süreci bu görevde çalıştırır; süreç bitince (exit, sinyal) çıkış kodunu
 * döndürür. Adres alanını çağıran siler (upp.cekirdek.kullanici_alan_sil). */
static long long upp_cekirdek_kullanici_calistir(long long ka, long long giris, long long yigin) {
    UppGorev* g;
    long long kod;
    _upp_kul_kur();
    if (!_upp_kul_hazir || !ka || (uint64_t)giris >= _UPP_KUL_UST || (uint64_t)yigin >= _UPP_KUL_UST) {
        return -1;
    }
    _upp_gorev_ilk();
    g = &_upp_gorev[_upp_gorev_cur];
    if (g->ka) {
        return -1;
    }
    g->ka = (uint64_t)ka;
    g->kul_cikis = 0;
    g->kul_hata[0] = 0;
    g->kul_dilim = _upp_tik;
    g->kul_fs = 0;
    g->kul_gs = 0;
    _upp_kul_tls_yukle(g);
    _upp_cr3_yaz((uint64_t)ka);
    kod = _upp_kul_gir(&g->kul_kayit, (uint64_t)giris, (uint64_t)yigin);
    g = &_upp_gorev[_upp_gorev_cur];
    g->ka = 0;
    g->kul_kayit = 0;
    g->kul_on_plan = 0;
    _upp_cr3_yaz(_upp_cekirdek_cr3);
    __asm__ volatile("sti");
    return kod;
}

/* ---- fork ve execve mekanizması ----
 * Politika (süreç tablosu, tanımlayıcıların kopyası, PID'ler, wait4) Uppix'te.
 * Buradaki yerleşikler yalnızca adres alanını kopyalar, sistem çağrısı anındaki
 * yazmaçları bir tampona yazar ve bir süreci o yazmaçlarla ya da yeni bir
 * görüntünün girişinden ring 3'e sokar. */

/* Tampondaki yazmaç çerçevesi (8'er bayt): rax, rdi, rsi, rdx, r10, r8, r9,
 * r15, r14, r13, r12, rbx, rbp, rip, rflags, rsp, fs tabanı, gs tabanı; 160.
 * bayttan sonra 512 baytlık FXSAVE alanı. */
#define _UPP_CER_YAZMAC 18
#define _UPP_CER_FX 160
#define _UPP_CER_BOYUT (_UPP_CER_FX + 512)

/* Alt yarının kopyası: sahipli her sayfa yeni bir çerçeveye kopyalanır, sahipsiz
 * eşlemeler (çerçeve tamponu) paylaşılır. Bellek yetmezse 0. */
static long long upp_cekirdek_kullanici_alan_kopyala(long long ka) {
    uint64_t yeni, *l4;
    unsigned a, b, c, d;
    if (!ka || !_upp_kul_hazir) {
        return 0;
    }
    yeni = (uint64_t)upp_cekirdek_kullanici_alan_yeni();
    if (!yeni) {
        return 0;
    }
    l4 = _upp_fv((uint64_t)ka);
    for (a = 0; a < 256u; a++) {
        uint64_t* l3;
        if (!(l4[a] & _UPP_SY_P)) continue;
        l3 = _upp_fv(l4[a] & _UPP_SY_ADRES);
        for (b = 0; b < 512u; b++) {
            uint64_t* l2;
            if (!(l3[b] & _UPP_SY_P)) continue;
            l2 = _upp_fv(l3[b] & _UPP_SY_ADRES);
            for (c = 0; c < 512u; c++) {
                uint64_t* l1;
                if (!(l2[c] & _UPP_SY_P)) continue;
                l1 = _upp_fv(l2[c] & _UPP_SY_ADRES);
                for (d = 0; d < 512u; d++) {
                    uint64_t e = l1[d], va, *pte;
                    if (!(e & _UPP_SY_P)) continue;
                    va = ((uint64_t)a << 39) | ((uint64_t)b << 30) | ((uint64_t)c << 21) | ((uint64_t)d << 12);
                    pte = _upp_kul_pte(yeni, va, 1);
                    if (!pte) {
                        _upp_kul_alan_sil(yeni);
                        return 0;
                    }
                    if (e & _UPP_SY_SAHIP) {
                        uint64_t f = _upp_cer_ayir();
                        if (!f) {
                            _upp_kul_alan_sil(yeni);
                            return 0;
                        }
                        memcpy(_upp_fv(f), _upp_fv(e & _UPP_SY_ADRES), 4096u);
                        *pte = f | (e & ~_UPP_SY_ADRES);
                    } else {
                        *pte = e;
                    }
                }
            }
        }
    }
    return (long long)yeni;
}

/* İşlenmekte olan sistem çağrısının yazmaçlarını t'ye yazar (fork). Sistem
 * çağrısı dışında ya da tampon küçükse yanlis. */
static bool upp_cekirdek_kullanici_cerceve_kaydet(long long t) {
    UppGorev* g = &_upp_gorev[_upp_gorev_cur];
    const UppSistemCagrisi* f = (const UppSistemCagrisi*)g->kul_sc;
    _UppTampon* b;
    uint64_t y[_UPP_CER_YAZMAC];
    if (!f || !g->ka) {
        return false;
    }
    b = _upp_tampon_al(t, 0, _UPP_CER_BOYUT);
    y[0] = f->no; y[1] = f->a1; y[2] = f->a2; y[3] = f->a3; y[4] = f->a4; y[5] = f->a5; y[6] = f->a6;
    y[7] = f->r15; y[8] = f->r14; y[9] = f->r13; y[10] = f->r12; y[11] = f->rbx; y[12] = f->rbp;
    y[13] = f->rip; y[14] = f->rflags; y[15] = f->rsp; y[16] = g->kul_fs; y[17] = g->kul_gs;
    memcpy(b->veri, y, sizeof(y));
    /* _upp_sc_giris FXSAVE alanını çerçevenin hemen altına koyar. */
    memcpy(b->veri + _UPP_CER_FX, (const unsigned char*)f - 512, 512u);
    return true;
}

/* Süreci bu görevde t'deki yazmaçlarla çalıştırır (fork'un çocuğu); dönüşü
 * upp.cekirdek.kullanici_calistir ile aynıdır. Yazmaçlar kullanıcıya aittir:
 * rip/rsp/TLS tabanı kullanıcı alanında olmalı, RFLAGS yalnızca kullanıcının
 * değiştirebileceği bitleri taşır, MXCSR'ın ayrılmış bitleri temizlenir. */
static long long upp_cekirdek_kullanici_cerceve_calistir(long long ka, long long t) {
    UppGorev* g;
    _UppTampon* b;
    uint64_t y[_UPP_CER_YAZMAC];
    unsigned char fx[512] __attribute__((aligned(16)));
    uint32_t mxcsr;
    long long kod;
    _upp_kul_kur();
    if (!_upp_kul_hazir || !ka) {
        return -1;
    }
    b = _upp_tampon_al(t, 0, _UPP_CER_BOYUT);
    memcpy(y, b->veri, sizeof(y));
    memcpy(fx, b->veri + _UPP_CER_FX, sizeof(fx));
    if (y[13] >= _UPP_KUL_UST || y[15] >= _UPP_KUL_UST || y[16] >= _UPP_KUL_UST || y[17] >= _UPP_KUL_UST) {
        return -1;
    }
    y[14] = (y[14] & 0xCD5ull) | 0x202ull; /* CF PF AF ZF SF DF OF + IF; IOPL 0 */
    memcpy(&mxcsr, fx + 24, 4);
    mxcsr &= 0xFFBFu;
    memcpy(fx + 24, &mxcsr, 4);
    _upp_gorev_ilk();
    g = &_upp_gorev[_upp_gorev_cur];
    if (g->ka) {
        return -1;
    }
    g->ka = (uint64_t)ka;
    g->kul_cikis = 0;
    g->kul_yurut = 0;
    g->kul_hata[0] = 0;
    g->kul_dilim = _upp_tik;
    g->kul_fs = y[16];
    g->kul_gs = y[17];
    _upp_kul_tls_yukle(g);
    _upp_cr3_yaz((uint64_t)ka);
    kod = _upp_kul_gir_tam(&g->kul_kayit, y, fx);
    g = &_upp_gorev[_upp_gorev_cur];
    g->ka = 0;
    g->kul_kayit = 0;
    g->kul_on_plan = 0;
    _upp_cr3_yaz(_upp_cekirdek_cr3);
    __asm__ volatile("sti");
    return kod;
}

/* execve mekanizması: sistem çağrısı dönerken süreç ka'ya geçer ve giris'ten
 * yigin ile başlar; eski adres alanı bırakılır. Sistem çağrısı dışında yanlis. */
static bool upp_cekirdek_kullanici_yurut(long long ka, long long giris, long long yigin) {
    UppGorev* g = &_upp_gorev[_upp_gorev_cur];
    if (!g->kul_sc || !g->ka || !ka || (uint64_t)ka == g->ka || giris < 0 || yigin < 0 ||
        (uint64_t)giris >= _UPP_KUL_UST || (uint64_t)yigin >= _UPP_KUL_UST) {
        return false;
    }
    g->kul_yurut = 1;
    g->kul_y_ka = (uint64_t)ka;
    g->kul_y_giris = (uint64_t)giris;
    g->kul_y_yigin = (uint64_t)yigin;
    return true;
}

/* arch_prctl mekanizması: segment 0 = FS, 1 = GS. Taban kullanıcı alanında
 * olmalı (kanonik olmayan bir taban çekirdekte #GP üretirdi). */
static bool upp_cekirdek_kullanici_tls_ayarla(long long segment, long long taban) {
    UppGorev* g = &_upp_gorev[_upp_gorev_cur];
    if (!g->ka || taban < 0 || (uint64_t)taban >= _UPP_KUL_UST || (segment != 0 && segment != 1)) {
        return false;
    }
    if (segment == 0) {
        g->kul_fs = (uint64_t)taban;
    } else {
        g->kul_gs = (uint64_t)taban;
    }
    _upp_kul_tls_yukle(g);
    return true;
}

static long long upp_cekirdek_kullanici_tls(long long segment) {
    UppGorev* g = &_upp_gorev[_upp_gorev_cur];
    return (long long)(segment == 0 ? g->kul_fs : g->kul_gs);
}

/* Sistem çağrısı işleyicisi dönünce süreç kod ile biter (exit, exit_group). */
static void upp_cekirdek_kullanici_cikis_iste(long long kod) {
    UppGorev* g = &_upp_gorev[_upp_gorev_cur];
    if (g->ka) {
        g->kul_cikis = 1;
        g->kul_kod = kod;
    }
}

/* Bu görevin süreci ön planda mı (Ctrl-C SIGINT ile sonlandırır). */
static void upp_cekirdek_kullanici_on_plan(bool v) {
    _upp_gorev_ilk();
    _upp_gorev[_upp_gorev_cur].kul_on_plan = v ? 1 : 0;
}

/* Bu görevdeki son sürecin sonlanma nedeni ("" = kendi çıktı). */
static char* upp_cekirdek_kullanici_son_hata(void) {
    _upp_gorev_ilk();
    return _upp_m_c(_upp_gorev[_upp_gorev_cur].kul_hata);
}

static void upp_cekirdek_sistem_cagrisi_kur(long long (*f)(long long, long long, long long, long long, long long, long long, long long)) {
    _upp_sc_isleyici = f;
}

/* Kullanıcı çerçeve havuzu: 0 toplam, 1 kullanılan, 2 boş (bayt). */
static long long upp_cekirdek_cerceve_bilgi(long long alan) {
    if (alan == 0) return (long long)_upp_cer_toplam;
    if (alan == 1) return (long long)_upp_cer_kullanilan;
    if (alan == 2) return (long long)(_upp_cer_toplam - _upp_cer_kullanilan);
    return 0;
}

/* Ekran: 0 çerçeve tamponu fiziksel, 1 boyu, 2 genişlik, 3 yükseklik (piksel),
 * 4 satır adımı (bayt), 5 bpp, 6-11 kırmızı/yeşil/mavi boy ve kaydırma,
 * 12 sütun, 13 satır (karakter). Çerçeve tamponu yoksa 0-11 sıfırdır. */
static long long upp_cekirdek_ekran_bilgi(long long alan) {
    if (alan == 12) return _upp_ekr_g;
    if (alan == 13) return _upp_ekr_y;
    if (!_upp_fb_aktif || !_upp_fb_fiz) return 0;
    switch (alan) {
    case 0: return (long long)_upp_fb_fiz;
    case 1: return (long long)_upp_fb_boy;
    case 2: return _upp_fb_gen;
    case 3: return _upp_fb_yuk;
    case 4: return _upp_fb_adim;
    case 5: return _upp_fb_bpp;
    case 6: return _upp_fb_rk;
    case 7: return _upp_fb_rs;
    case 8: return _upp_fb_gk;
    case 9: return _upp_fb_gs;
    case 10: return _upp_fb_bk;
    case 11: return _upp_fb_bs;
    default: return 0;
    }
}
