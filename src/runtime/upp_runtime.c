/* OTOMATİK ÜRETİLDİ (uppc kendini-derle) — düzenlemeyin.
 * Kaynak: upp_on_barindirilan.c + upp_cekirdek.c + upp_barindirilan.c */
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
 * u++ çalışma zamanı — BARINDIRILAN katman (işletim sistemi + libc)
 *
 * Dosya, süreç, iş parçacığı, zaman, ortam, JSON, matematik (libm),
 * Windows/Linux API'leri ve çekirdeğin platform sözleşmesi.
 * ===================================================================== */

static void upp_panik(const char* mesaj) {
    fprintf(stderr, "[u++ HATA] %s\n", mesaj ? mesaj : "iç hata");
    fflush(stderr);
    abort();
}

static size_t _upp_ondalik_yaz(char* b, size_t cap, double v, int hassas17) {
    int n = snprintf(b, cap, hassas17 ? "%.17g" : "%g", v);
    if (n < 0) {
        n = 0;
    }
    if ((size_t)n >= cap) {
        n = (int)cap - 1;
    }
    return (size_t)n;
}

static int _upp_rand_hazir = 0;

#if UPP_WIN
static void _upp_rand_baslat(void) {
    unsigned seed;
    LARGE_INTEGER qpc;
    if (_upp_rand_hazir) {
        return;
    }
    /* GetTickCount 10–16 ms; PID + QPC ile aynı tikte iki sürecin aynı tohumu alması zorlaşır. */
    seed = (unsigned)GetTickCount();
    seed ^= (unsigned)GetCurrentProcessId() * 2654435761u;
    if (QueryPerformanceCounter(&qpc)) {
        seed ^= (unsigned)qpc.LowPart;
        seed ^= (unsigned)qpc.HighPart;
    }
    srand(seed ? seed : 1u);
    _upp_rand_hazir = 1;
}
#else
static void _upp_rand_baslat(void) {
    unsigned seed;
    struct timespec ts;
    if (_upp_rand_hazir) {
        return;
    }
    seed = (unsigned)time(NULL);
    seed ^= (unsigned)getpid() * 2654435761u;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        seed ^= (unsigned)ts.tv_nsec;
        seed ^= (unsigned)(ts.tv_sec & 0xffff);
    }
    srand(seed ? seed : 1u);
    _upp_rand_hazir = 1;
}
#endif

/* C standardı RAND_MAX >= 32767 (15 bit). Windows MinGW/MSVC'de tam 32767'dır:
 * tek rand() ile upp.rastgele(0, 100000) asla 32767'nin üstüne çıkamazdı.
 * 15 bitlik parçaları kaydırıp 64 bit dolduruyoruz. */
static unsigned long long _upp_rand_u64(void) {
    unsigned long long r = 0;
    int filled = 0;
    while (filled < 64) {
        unsigned long long chunk = (unsigned long long)rand() & 0x7FFFull;
        r |= chunk << filled;
        filled += 15;
    }
    return r;
}

#if UPP_WIN
static void upp_mesaj(const char* baslik, const char* metin) {
    MessageBoxA(NULL, metin ? metin : "", baslik ? baslik : "", MB_OK | MB_ICONINFORMATION);
}

static void upp_hata(const char* baslik, const char* metin) {
    MessageBoxA(NULL, metin ? metin : "", baslik ? baslik : "", MB_OK | MB_ICONERROR);
}

static int upp_ses_cal(const char* dosya_yolu) {
    if (!dosya_yolu) {
        return 0;
    }
    return PlaySoundA(dosya_yolu, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT) ? 1 : 0;
}

static void upp_uyut(long long ms) {
    /* Sleep(0xFFFFFFFF) = INFINITE; DWORD kesmesi sonsuz beklemeye dönüşmesin. */
    if (ms < 0) {
        ms = 0;
    }
    if ((unsigned long long)ms >= 0xfffffffeull) {
        Sleep(0xfffffffeul);
        return;
    }
    Sleep((DWORD)ms);
}
#else
static void upp_mesaj(const char* baslik, const char* metin) {
    fprintf(stderr, "[u++ MESAJ] %s: %s\n", baslik ? baslik : "", metin ? metin : "");
}

static void upp_hata(const char* baslik, const char* metin) {
    fprintf(stderr, "[u++ HATA] %s: %s\n", baslik ? baslik : "", metin ? metin : "");
}

static int upp_ses_cal(const char* dosya_yolu) {
    /* Linux: ses çalma altyapısı yok; sessizce yut. */
    (void)dosya_yolu;
    return 0;
}

static void upp_uyut(long long ms) {
    struct timespec ts;
    if (ms < 0) {
        ms = 0;
    }
    if (ms > 1000000000LL) {
        ms = 1000000000LL;
    }
    ts.tv_sec = (time_t)(ms / 1000);
    ts.tv_nsec = (long)((ms % 1000) * 1000000L);
    while (nanosleep(&ts, &ts) != 0) {
        if (errno != EINTR) {
            break;
        }
    }
}
#endif

static long long upp_rastgele(long long mini, long long maxi) {
    unsigned long long umin, umax, span, pick, limit;
    _upp_rand_baslat();
    if (maxi < mini) {
        long long t = mini;
        mini = maxi;
        maxi = t;
    }
    /* İmzasız fark: (maxi - mini) signed taşmasını önler. */
    umin = (unsigned long long)mini;
    umax = (unsigned long long)maxi;
    span = umax - umin + 1ull;
    if (span == 0ull) {
        /* [LLONG_MIN, LLONG_MAX] tam 2^64 değer. */
        return (long long)_upp_rand_u64();
    }
    /* 2^64'ü span'e bölünmeyen artık, alt tarafa yığılmasın diye reddet. */
    limit = (0xffffffffffffffffull / span) * span;
    do {
        pick = _upp_rand_u64();
    } while (pick >= limit);
    pick %= span;
    return (long long)(umin + pick);
}

static char* upp_dosya_oku(const char* yol) {
    FILE* f;
    long long size;
    char* buf;
    size_t n;
    if (!yol) {
        return NULL;
    }
    f = fopen(yol, "rb");
    if (!f) {
        return NULL;
    }
#if UPP_WIN
    if (_fseeki64(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    size = _ftelli64(f);
#else
    if (fseeko(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    size = (long long)ftello(f);
#endif
    if (size < 0) {
        fclose(f);
        return NULL;
    }
    rewind(f);
    buf = _upp_m_yeni((size_t)size);
    n = fread(buf, 1, (size_t)size, f);
    fclose(f);
    return _upp_m_boy(buf, n);
}

#if UPP_WIN
static bool upp_dosya_var_mi(const char* yol) {
    DWORD attr;
    if (!yol || !yol[0]) {
        return false;
    }
    attr = GetFileAttributesA(yol);
    return attr != INVALID_FILE_ATTRIBUTES;
}
#else
static bool upp_dosya_var_mi(const char* yol) {
    if (!yol || !yol[0]) {
        return false;
    }
    return access(yol, F_OK) == 0;
}
#endif

static bool upp_dosya_sil(const char* yol) {
    if (!yol || !yol[0]) {
        return false;
    }
    return remove(yol) == 0;
}

static int _upp_argc = 0;
static char** _upp_argv = NULL;

static void upp_argv_ayarla(int argc, char** argv) {
    _upp_argc = argc;
    _upp_argv = argv;
#if UPP_WIN
    {
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD kip = 0;
        if (h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &kip)) {
            SetConsoleMode(h, kip | 0x0004);
        }
    }
#endif
}

static long long upp_arguman_sayisi(void) {
    return (long long)_upp_argc;
}

static char* upp_arguman(long long i) {
    if (i < 0 || i >= (long long)_upp_argc || !_upp_argv) {
        return NULL;
    }
    return _upp_m_c(_upp_argv[(int)i]);
}

#if UPP_WIN
static long long upp_zaman(void) {
    return (long long)GetTickCount64();
}
#else
static long long upp_zaman(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0;
    }
    return (long long)ts.tv_sec * 1000LL + (long long)(ts.tv_nsec / 1000000L);
}
#endif

static int upp_dosya_yaz(const char* yol, const char* icerik) {
    FILE* f;
    if (!yol) {
        return 0;
    }
    f = fopen(yol, "wb");
    if (!f) {
        return 0;
    }
    if (icerik) {
        fputs(icerik, f);
    }
    fclose(f);
    return 1;
}

static int upp_dosya_ekle(const char* yol, const char* icerik) {
    FILE* f;
    if (!yol) {
        return 0;
    }
    f = fopen(yol, "ab");
    if (!f) {
        return 0;
    }
    if (icerik) {
        fputs(icerik, f);
    }
    fclose(f);
    return 1;
}

static char* upp_ortam_al(const char* ad) {
    const char* v;
    char* p;
    size_t n;
    if (!ad) {
        return NULL;
    }
    v = getenv(ad);
    if (!v) {
        return NULL;
    }
    n = strlen(v);
    p = _upp_m_bayttan(v, n);
    return p;
}

static char* upp_ondalikdan_metin(double x) {
    char buf[64];
    char* p;
    size_t len;
    snprintf(buf, sizeof(buf), "%.17g", x);
    len = strlen(buf);
    p = _upp_m_bayttan(buf, len);
    return p;
}

static double upp_metinden_ondalik(const char* s) {
    if (!s) {
        return 0.0;
    }
    return atof(s);
}

static void upp_hata_yaz(const char* s) {
    fputs(s ? s : "", stderr);
    fflush(stderr);
}

static void upp_hata_satir_yaz(const char* s) {
    fputs(s ? s : "", stderr);
    fputc('\n', stderr);
    fflush(stderr);
}

static char* upp_giris(void) {
    size_t cap = 256;
    size_t n = 0;
    int c = 0;
    int saw = 0;
    char* p = (char*)malloc(cap);
    if (!p) {
        return NULL;
    }
    while ((c = fgetc(stdin)) != EOF) {
        saw = 1;
        if (c == '\n') {
            break;
        }
        if (c == '\r') {
            int nxt = fgetc(stdin);
            if (nxt != '\n' && nxt != EOF) {
                ungetc(nxt, stdin);
            }
            break;
        }
        if (n + 1 >= cap) {
            size_t nc = cap * 2u;
            char* q = (char*)realloc(p, nc);
            if (!q) {
                free(p);
                return NULL;
            }
            p = q;
            cap = nc;
        }
        p[n++] = (char)c;
    }
    if (!saw && c == EOF) {
        free(p);
        return NULL;
    }
    p[n] = '\0';
    return _upp_m_sahiplen(p);
}

/* ---- çıktı: codegen printf üretmez, her yazım buradan geçer ---- */
static void upp_arkaplan_hata(int tur) {
    fputs(tur == 0 ? "[u++] arkaplan: bellek yetersiz.\n" : "[u++] arkaplan: iş parçacığı başlatılamadı.\n", stderr);
    fflush(stderr);
}

static void _upp_akis_bayt(int akis, const char* p, size_t n, int nl) {
    FILE* f = akis == UPP_AKIS_HATA ? stderr : stdout;
    if (p && n) {
        fwrite(p, 1, n, f);
    }
    if (nl) {
        fputc('\n', f);
    }
    if (akis == UPP_AKIS_HATA) {
        fflush(f);
    }
}

#define UPP_ARK_MAX 256
static long long _upp_ark_h[UPP_ARK_MAX];
static int _upp_ark_n = 0;
static int _upp_ark_atexit_ok = 0;
#if UPP_WIN
static CRITICAL_SECTION _upp_ark_cs;
static int _upp_ark_cs_ok = 0;
static void upp_ark_kilit_hazir(void) {
    if (!_upp_ark_cs_ok) {
        InitializeCriticalSection(&_upp_ark_cs);
        _upp_ark_cs_ok = 1;
    }
}
static void upp_ark_kilitle(void) {
    upp_ark_kilit_hazir();
    EnterCriticalSection(&_upp_ark_cs);
}
static void upp_ark_birak(void) {
    LeaveCriticalSection(&_upp_ark_cs);
}
#else
static pthread_mutex_t _upp_ark_mu = PTHREAD_MUTEX_INITIALIZER;
static void upp_ark_kilitle(void) { pthread_mutex_lock(&_upp_ark_mu); }
static void upp_ark_birak(void) { pthread_mutex_unlock(&_upp_ark_mu); }
#endif

static void upp_arkaplan_atexit(void) {
    int i;
    upp_ark_kilitle();
    for (i = 0; i < _upp_ark_n; i++) {
        if (!_upp_ark_h[i]) {
            continue;
        }
#if UPP_WIN
        CloseHandle((HANDLE)(intptr_t)_upp_ark_h[i]);
#else
        pthread_detach((pthread_t)(uintptr_t)_upp_ark_h[i]);
#endif
        _upp_ark_h[i] = 0;
    }
    _upp_ark_n = 0;
    upp_ark_birak();
}

static void upp_arkaplan_kaydet(long long h) {
    if (!h) {
        return;
    }
    upp_ark_kilitle();
    if (!_upp_ark_atexit_ok) {
        atexit(upp_arkaplan_atexit);
        _upp_ark_atexit_ok = 1;
    }
    if (_upp_ark_n < UPP_ARK_MAX) {
        _upp_ark_h[_upp_ark_n++] = h;
    }
    upp_ark_birak();
}

static void upp_arkaplan_cikar(long long h) {
    int i;
    if (!h) {
        return;
    }
    upp_ark_kilitle();
    for (i = 0; i < _upp_ark_n; i++) {
        if (_upp_ark_h[i] == h) {
            _upp_ark_h[i] = _upp_ark_h[_upp_ark_n - 1];
            _upp_ark_n--;
            break;
        }
    }
    upp_ark_birak();
}

#if UPP_WIN
static void upp_arkaplan_bekle(ArkaplanIs* j) {
    HANDLE h;
    if (!j || !j->handle) return;
    h = (HANDLE)(intptr_t)j->handle;
    upp_arkaplan_cikar(j->handle);
    WaitForSingleObject(h, INFINITE);
    CloseHandle(h);
    j->handle = 0;
}
#else
static void upp_arkaplan_bekle(ArkaplanIs* j) {
    pthread_t th;
    if (!j || !j->handle) return;
    th = (pthread_t)(uintptr_t)j->handle;
    upp_arkaplan_cikar(j->handle);
    pthread_join(th, NULL);
    j->handle = 0;
}
#endif

static void upp_arkaplan_bekle_deger(ArkaplanIs j) {
    upp_arkaplan_bekle(&j);
}

/* ---- upp.matematik ---- */

static double upp_mat_karekok(double x) { return x < 0.0 ? 0.0 : sqrt(x); }
static double upp_mat_us(double a, double b) { return pow(a, b); }
static double upp_mat_sin(double x) { return sin(x); }
static double upp_mat_cos(double x) { return cos(x); }
static double upp_mat_tan(double x) { return tan(x); }
static double upp_mat_atan2(double y, double x) { return atan2(y, x); }
static double upp_mat_taban(double x) { return floor(x); }
static double upp_mat_tavan(double x) { return ceil(x); }
static double upp_mat_yuvarla(double x) { return round(x); }
static double upp_mat_mesafe_3d(double x1, double y1, double z1, double x2, double y2, double z2) {
    double dx = x2 - x1, dy = y2 - y1, dz = z2 - z1;
    return sqrt(dx * dx + dy * dy + dz * dz);
}

static double upp_mat_mesafe_v3(Vektor3 a, Vektor3 b) {
    return upp_mat_mesafe_3d(a.x, a.y, a.z, b.x, b.y, b.z);
}

static Yonelme upp_mat_aci_hesapla(double x1, double y1, double z1, double x2, double y2, double z2) {
    Yonelme y;
    double dx = x2 - x1, dy = y2 - y1, dz = z2 - z1;
    double hyp = sqrt(dx * dx + dy * dy);
    y.yaw = atan2(dy, dx);
    y.pitch = atan2(dz, hyp);
    return y;
}

static EkranNokta upp_mat_dunya_ekran(double hx, double hy, double hz, const double* m, long long n, double genislik, double yukseklik) {
    EkranNokta e;
    double cx, cy, cz, cw, ndx, ndy;
    e.x = 0.0;
    e.y = 0.0;
    e.gorunur = 0;
    if (!m || n < 16) {
        return e;
    }
    /* sütun-major 4x4 View-Projection */
    cx = m[0] * hx + m[4] * hy + m[8] * hz + m[12];
    cy = m[1] * hx + m[5] * hy + m[9] * hz + m[13];
    cz = m[2] * hx + m[6] * hy + m[10] * hz + m[14];
    cw = m[3] * hx + m[7] * hy + m[11] * hz + m[15];
    (void)cz;
    if (cw > 1e-6 || cw < -1e-6) {
        ndx = cx / cw;
        ndy = cy / cw;
        e.x = (ndx + 1.0) * 0.5 * genislik;
        e.y = (1.0 - ndy) * 0.5 * yukseklik;
        e.gorunur = (cw > 0.01 && ndx >= -1.2 && ndx <= 1.2 && ndy >= -1.2 && ndy <= 1.2) ? 1 : 0;
    }
    return e;
}

/* ---- upp.girdi ---- */

#if UPP_WIN
static void upp_girdi_fare_tasi(long long x, long long y) {
    SetCursorPos((int)x, (int)y);
}

static void upp_girdi_fare_tikla(void) {
    INPUT in[2];
    memset(in, 0, sizeof(in));
    in[0].type = INPUT_MOUSE;
    in[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    in[1].type = INPUT_MOUSE;
    in[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(2, in, sizeof(INPUT));
}

static int _upp_vk(const char* s) {
    char buf[32];
    size_t i, n;
    if (!s || !s[0]) return 0;
    n = strlen(s);
    if (n >= sizeof(buf)) n = sizeof(buf) - 1;
    for (i = 0; i < n; i++) {
        char c = s[i];
        if (c >= 'a' && c <= 'z') c = (char)(c - 32);
        buf[i] = c;
    }
    buf[n] = '\0';
    if (buf[0] == 'F' && buf[1] >= '1' && buf[1] <= '9') {
        int fn = atoi(buf + 1);
        if (fn >= 1 && fn <= 24) return VK_F1 + (fn - 1);
    }
    if (n == 1) {
        if (buf[0] >= 'A' && buf[0] <= 'Z') return buf[0];
        if (buf[0] >= '0' && buf[0] <= '9') return buf[0];
    }
    if (!strcmp(buf, "ENTER") || !strcmp(buf, "RETURN")) return VK_RETURN;
    if (!strcmp(buf, "ESC") || !strcmp(buf, "ESCAPE")) return VK_ESCAPE;
    if (!strcmp(buf, "SPACE") || !strcmp(buf, "BOSLUK")) return VK_SPACE;
    if (!strcmp(buf, "TAB")) return VK_TAB;
    if (!strcmp(buf, "SHIFT")) return VK_SHIFT;
    if (!strcmp(buf, "CTRL") || !strcmp(buf, "CONTROL")) return VK_CONTROL;
    if (!strcmp(buf, "ALT")) return VK_MENU;
    if (!strcmp(buf, "BACKSPACE") || !strcmp(buf, "BACK")) return VK_BACK;
    if (!strcmp(buf, "DELETE") || !strcmp(buf, "DEL")) return VK_DELETE;
    if (!strcmp(buf, "INSERT") || !strcmp(buf, "INS")) return VK_INSERT;
    if (!strcmp(buf, "HOME")) return VK_HOME;
    if (!strcmp(buf, "END")) return VK_END;
    if (!strcmp(buf, "LEFT") || !strcmp(buf, "SOL")) return VK_LEFT;
    if (!strcmp(buf, "RIGHT") || !strcmp(buf, "SAG")) return VK_RIGHT;
    if (!strcmp(buf, "UP") || !strcmp(buf, "YUKARI")) return VK_UP;
    if (!strcmp(buf, "DOWN") || !strcmp(buf, "ASAGI")) return VK_DOWN;
    if (!strcmp(buf, "PAGEUP") || !strcmp(buf, "PGUP")) return VK_PRIOR;
    if (!strcmp(buf, "PAGEDOWN") || !strcmp(buf, "PGDN")) return VK_NEXT;
    return 0;
}

static void upp_girdi_tus_bas(const char* tus) {
    INPUT in[2];
    WORD vk = (WORD)_upp_vk(tus);
    if (!vk) {
        fprintf(stderr, "[u++] bilinmeyen tus: %s\n", tus ? tus : "");
        return;
    }
    memset(in, 0, sizeof(in));
    in[0].type = INPUT_KEYBOARD;
    in[0].ki.wVk = vk;
    in[1].type = INPUT_KEYBOARD;
    in[1].ki.wVk = vk;
    in[1].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(2, in, sizeof(INPUT));
}
#else
static void upp_girdi_fare_tasi(long long x, long long y) {
    (void)x; (void)y;
}

static void upp_girdi_fare_tikla(void) {
}

static int _upp_vk(const char* s) {
    char buf[32];
    size_t i, n;
    if (!s || !s[0]) return 0;
    n = strlen(s);
    if (n >= sizeof(buf)) n = sizeof(buf) - 1;
    for (i = 0; i < n; i++) {
        char c = s[i];
        if (c >= 'a' && c <= 'z') c = (char)(c - 32);
        buf[i] = c;
    }
    buf[n] = '\0';
    if (buf[0] == 'F' && buf[1] >= '1' && buf[1] <= '9') {
        int fn = atoi(buf + 1);
        if (fn >= 1 && fn <= 24) return 1;
    }
    if (n == 1) {
        if (buf[0] >= 'A' && buf[0] <= 'Z') return 1;
        if (buf[0] >= '0' && buf[0] <= '9') return 1;
    }
    if (!strcmp(buf, "ENTER") || !strcmp(buf, "RETURN")) return 1;
    if (!strcmp(buf, "ESC") || !strcmp(buf, "ESCAPE")) return 1;
    if (!strcmp(buf, "SPACE") || !strcmp(buf, "BOSLUK")) return 1;
    if (!strcmp(buf, "TAB")) return 1;
    if (!strcmp(buf, "SHIFT")) return 1;
    if (!strcmp(buf, "CTRL") || !strcmp(buf, "CONTROL")) return 1;
    if (!strcmp(buf, "ALT")) return 1;
    if (!strcmp(buf, "BACKSPACE") || !strcmp(buf, "BACK")) return 1;
    if (!strcmp(buf, "DELETE") || !strcmp(buf, "DEL")) return 1;
    if (!strcmp(buf, "INSERT") || !strcmp(buf, "INS")) return 1;
    if (!strcmp(buf, "HOME")) return 1;
    if (!strcmp(buf, "END")) return 1;
    if (!strcmp(buf, "LEFT") || !strcmp(buf, "SOL")) return 1;
    if (!strcmp(buf, "RIGHT") || !strcmp(buf, "SAG")) return 1;
    if (!strcmp(buf, "UP") || !strcmp(buf, "YUKARI")) return 1;
    if (!strcmp(buf, "DOWN") || !strcmp(buf, "ASAGI")) return 1;
    if (!strcmp(buf, "PAGEUP") || !strcmp(buf, "PGUP")) return 1;
    if (!strcmp(buf, "PAGEDOWN") || !strcmp(buf, "PGDN")) return 1;
    return 0;
}

static void upp_girdi_tus_bas(const char* tus) {
    if (!_upp_vk(tus)) {
        fprintf(stderr, "[u++] bilinmeyen tus: %s\n", tus ? tus : "");
    }
}
#endif

/* ---- upp.bellek ---- */

#if UPP_WIN
static HANDLE _upp_bellek_h = NULL;
static DWORD _upp_bellek_pid = 0;
static long long _upp_bellek_hata = 0;
static CRITICAL_SECTION _upp_bellek_cs;
static int _upp_bellek_cs_ok = 0;

static void _upp_bellek_cs_hazir(void) {
    if (!_upp_bellek_cs_ok) {
        InitializeCriticalSection(&_upp_bellek_cs);
        _upp_bellek_cs_ok = 1;
    }
}

static void _upp_bellek_kilitle(void) {
    _upp_bellek_cs_hazir();
    EnterCriticalSection(&_upp_bellek_cs);
}

static void _upp_bellek_birak(void) {
    LeaveCriticalSection(&_upp_bellek_cs);
}

static void _upp_bellek_hata_ayarla(int ok) {
    if (ok) {
        _upp_bellek_hata = 0;
        return;
    }
    _upp_bellek_hata = (long long)GetLastError();
    if (_upp_bellek_hata == 0) {
        _upp_bellek_hata = 1;
    }
}

static long long upp_bellek_son_hata(void) {
    long long v;
    _upp_bellek_kilitle();
    v = _upp_bellek_hata;
    _upp_bellek_birak();
    return v;
}

static long long upp_bellek_okundu(void) {
    long long v;
    _upp_bellek_kilitle();
    v = _upp_bellek_hata == 0 ? 1 : 0;
    _upp_bellek_birak();
    return v;
}

static int _upp_ad_esit(const char* a, const char* b) {
    if (!a || !b) return 0;
    while (*a && *b) {
        char ca = *a, cb = *b;
        if (ca >= 'A' && ca <= 'Z') ca = (char)(ca - 'A' + 'a');
        if (cb >= 'A' && cb <= 'Z') cb = (char)(cb - 'A' + 'a');
        if (ca != cb) return 0;
        a++; b++;
    }
    return *a == *b;
}

static int _upp_ad_exe(const char* want, const char* have) {
    char buf[MAX_PATH];
    if (_upp_ad_esit(want, have)) return 1;
    if (!want) return 0;
    snprintf(buf, sizeof(buf), "%s.exe", want);
    return _upp_ad_esit(buf, have);
}

static DWORD _upp_pid_bul(const char* ad) {
    HANDLE snap;
    PROCESSENTRY32 pe;
    DWORD pid = 0;
    if (!ad) return 0;
    snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    pe.dwSize = sizeof(pe);
    if (Process32First(snap, &pe)) {
        do {
            if (_upp_ad_exe(ad, pe.szExeFile)) {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(snap, &pe));
    }
    CloseHandle(snap);
    return pid;
}

static long long upp_bellek_baglan(const char* islem_adi) {
    DWORD pid = _upp_pid_bul(islem_adi);
    long long ok = 0;
    _upp_bellek_kilitle();
    if (_upp_bellek_h) {
        CloseHandle(_upp_bellek_h);
        _upp_bellek_h = NULL;
        _upp_bellek_pid = 0;
    }
    if (pid) {
        _upp_bellek_h = OpenProcess(
            PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION,
            FALSE, pid);
        if (_upp_bellek_h) {
            _upp_bellek_pid = pid;
            ok = 1;
        }
    }
    _upp_bellek_birak();
    return ok;
}

static long long upp_bellek_modul_bul(const char* dll_adi) {
    HANDLE snap;
    MODULEENTRY32 me;
    long long base = 0;
    DWORD pid;
    _upp_bellek_kilitle();
    pid = _upp_bellek_pid;
    if (!pid || !dll_adi) {
        _upp_bellek_birak();
        return 0;
    }
    snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap == INVALID_HANDLE_VALUE) {
        _upp_bellek_birak();
        return 0;
    }
    me.dwSize = sizeof(me);
    if (Module32First(snap, &me)) {
        do {
            if (_upp_ad_exe(dll_adi, me.szModule)) {
                base = (long long)(uintptr_t)me.modBaseAddr;
                break;
            }
        } while (Module32Next(snap, &me));
    }
    CloseHandle(snap);
    _upp_bellek_birak();
    return base;
}

static int _upp_rpm(uintptr_t addr, void* buf, size_t n) {
    SIZE_T got = 0;
    int ok;
    _upp_bellek_kilitle();
    if (!_upp_bellek_h || !buf) {
        _upp_bellek_hata_ayarla(0);
        _upp_bellek_birak();
        return 0;
    }
    ok = ReadProcessMemory(_upp_bellek_h, (LPCVOID)addr, buf, n, &got) && got == n;
    _upp_bellek_hata_ayarla(ok);
    _upp_bellek_birak();
    return ok;
}

static int _upp_wpm(uintptr_t addr, const void* buf, size_t n) {
    SIZE_T got = 0;
    int ok;
    _upp_bellek_kilitle();
    if (!_upp_bellek_h || !buf) {
        _upp_bellek_hata_ayarla(0);
        _upp_bellek_birak();
        return 0;
    }
    ok = WriteProcessMemory(_upp_bellek_h, (LPVOID)addr, buf, n, &got) && got == n;
    _upp_bellek_hata_ayarla(ok);
    _upp_bellek_birak();
    return ok;
}

static long long upp_bellek_sayioku(long long adres) {
    long long v = 0;
    if (!_upp_rpm((uintptr_t)adres, &v, sizeof(v))) return 0;
    return v;
}

static double upp_bellek_oku_ondalik(long long adres) {
    double v = 0.0;
    if (!_upp_rpm((uintptr_t)adres, &v, sizeof(v))) return 0.0;
    return v;
}

static char* upp_bellek_oku_metin(long long adres, long long uzunluk) {
    char* p;
    if (uzunluk <= 0) return NULL;
    if (uzunluk > 1048576) uzunluk = 1048576;
    p = (char*)malloc((size_t)uzunluk + 1u);
    if (!p) return NULL;
    memset(p, 0, (size_t)uzunluk + 1u);
    if (!_upp_rpm((uintptr_t)adres, p, (size_t)uzunluk)) {
        free(p);
        return NULL;
    }
    p[uzunluk] = '\0';
    return _upp_m_sahiplen(p);
}

static long long upp_bellek_sayiyaz(long long adres, long long deger) {
    return _upp_wpm((uintptr_t)adres, &deger, sizeof(deger)) ? 1 : 0;
}

static long long upp_bellek_yaz_ondalik(long long adres, double deger) {
    return _upp_wpm((uintptr_t)adres, &deger, sizeof(deger)) ? 1 : 0;
}

static long long upp_bellek_zincir_oku(long long taban, const long long* offs, long long n) {
    unsigned long long addr = (unsigned long long)taban;
    long long i;
    if (!offs || n <= 0) return 0;
    for (i = 0; i < n; i++) {
        unsigned long long ptr = 0;
        if (!_upp_rpm((uintptr_t)addr, &ptr, sizeof(ptr))) return 0;
        addr = ptr + (unsigned long long)offs[i];
    }
    return (long long)addr;
}

static long long upp_bellek_yama_yap(long long adres, const char* hex) {
    unsigned char buf[256];
    size_t nb = 0;
    const char* p;
    if (!hex) return 0;
    p = hex;
    while (*p && nb < sizeof(buf)) {
        unsigned int b = 0;
        while (*p == ' ' || *p == '\t' || *p == ',') p++;
        if (!*p) break;
        if (sscanf(p, "%2x", &b) != 1) return 0;
        buf[nb++] = (unsigned char)b;
        if (p[0] && p[1] && ((p[0] >= '0' && p[0] <= '9') || (p[0] >= 'A' && p[0] <= 'F') || (p[0] >= 'a' && p[0] <= 'f'))
            && ((p[1] >= '0' && p[1] <= '9') || (p[1] >= 'A' && p[1] <= 'F') || (p[1] >= 'a' && p[1] <= 'f'))) {
            p += 2;
        } else {
            return 0;
        }
    }
    if (!nb) return 0;
    return _upp_wpm((uintptr_t)adres, buf, nb) ? 1 : 0;
}

static DWORD _upp_izin_kod(const char* izin, long long sayi) {
    if (izin) {
        if (!strcmp(izin, "okuma") || !strcmp(izin, "PAGE_READONLY")) return PAGE_READONLY;
        if (!strcmp(izin, "okuyaz") || !strcmp(izin, "yazma") || !strcmp(izin, "PAGE_READWRITE")) return PAGE_READWRITE;
        if (!strcmp(izin, "calistir") || !strcmp(izin, "PAGE_EXECUTE_READ")) return PAGE_EXECUTE_READ;
        if (!strcmp(izin, "okuyazcalistir") || !strcmp(izin, "rwx") || !strcmp(izin, "PAGE_EXECUTE_READWRITE"))
            return PAGE_EXECUTE_READWRITE;
        if (!strcmp(izin, "yok") || !strcmp(izin, "PAGE_NOACCESS")) return PAGE_NOACCESS;
        if (!strcmp(izin, "calistiryaz") || !strcmp(izin, "PAGE_EXECUTE_READWRITE")) return PAGE_EXECUTE_READWRITE;
        if (!strcmp(izin, "PAGE_EXECUTE")) return PAGE_EXECUTE;
    }
    if (sayi != 0) return (DWORD)sayi;
    return PAGE_EXECUTE_READWRITE;
}

static long long upp_bellek_koruma_degistir(long long adres, long long boyut, const char* izin) {
    DWORD eski = 0;
    DWORD yeni;
    long long r = 0;
    _upp_bellek_kilitle();
    if (_upp_bellek_h && boyut > 0) {
        yeni = _upp_izin_kod(izin, 0);
        if (VirtualProtectEx(_upp_bellek_h, (LPVOID)(uintptr_t)adres, (SIZE_T)boyut, yeni, &eski)) {
            r = (long long)eski;
        }
    }
    _upp_bellek_birak();
    return r;
}

static long long upp_bellek_koruma_degistir_sayi(long long adres, long long boyut, long long izin) {
    DWORD eski = 0;
    long long r = 0;
    _upp_bellek_kilitle();
    if (_upp_bellek_h && boyut > 0) {
        if (VirtualProtectEx(_upp_bellek_h, (LPVOID)(uintptr_t)adres, (SIZE_T)boyut, (DWORD)izin, &eski)) {
            r = (long long)eski;
        }
    }
    _upp_bellek_birak();
    return r;
}
#else
/* Linux: bellek modülü yalnızca GÜNCEL sürecin (self) belleğinde çalışır.
 * Başka süreçlere erişim ptrace gerektirir; baglan kendi sürecimizle
 * eşleşirse başarılı olur. */
static int _upp_bellek_self = 0;
static long long _upp_bellek_hata = 0;
static pthread_mutex_t _upp_bellek_mx = PTHREAD_MUTEX_INITIALIZER;

static void _upp_bellek_kilitle(void) {
    pthread_mutex_lock(&_upp_bellek_mx);
}

static void _upp_bellek_birak(void) {
    pthread_mutex_unlock(&_upp_bellek_mx);
}

static int _upp_ad_esit(const char* a, const char* b) {
    if (!a || !b) return 0;
    while (*a && *b) {
        char ca = *a, cb = *b;
        if (ca >= 'A' && ca <= 'Z') ca = (char)(ca - 'A' + 'a');
        if (cb >= 'A' && cb <= 'Z') cb = (char)(cb - 'A' + 'a');
        if (ca != cb) return 0;
        a++; b++;
    }
    return *a == *b;
}

static const char* _upp_yol_ad(const char* p) {
    const char* last = p;
    if (!p) {
        return "";
    }
    while (*p) {
        if (*p == '/' || *p == '\\') {
            last = p + 1;
        }
        p++;
    }
    return last;
}

static void _upp_ad_kirp_exe(char* s) {
    size_t n;
    if (!s) {
        return;
    }
    n = strlen(s);
    if (n > 4) {
        char* e = s + n - 4;
        if (e[0] == '.' && (e[1] == 'e' || e[1] == 'E') && (e[2] == 'x' || e[2] == 'X') && (e[3] == 'e' || e[3] == 'E')) {
            *e = '\0';
        }
    }
}

static int _upp_ad_exe(const char* want, const char* have) {
    char w[256], h[256];
    const char *wb, *hb;
    size_t i;
    if (!want || !have) {
        return 0;
    }
    wb = _upp_yol_ad(want);
    hb = _upp_yol_ad(have);
    for (i = 0; wb[i] && i + 1 < sizeof(w); i++) {
        w[i] = wb[i];
    }
    w[i] = '\0';
    for (i = 0; hb[i] && hb[i] != '\r' && hb[i] != '\n' && i + 1 < sizeof(h); i++) {
        h[i] = hb[i];
    }
    h[i] = '\0';
    if (_upp_ad_esit(w, h)) {
        return 1;
    }
    _upp_ad_kirp_exe(w);
    _upp_ad_kirp_exe(h);
    return w[0] && h[0] && _upp_ad_esit(w, h);
}

static int _upp_kendi_ad(char* out, size_t cap) {
    size_t i;
    if (!out || cap == 0) {
        return 0;
    }
#if defined(__linux__)
    {
        char path[4096];
        ssize_t n;
        const char* ad;
        FILE* f;
        int c;
        n = readlink("/proc/self/exe", path, sizeof(path) - 1);
        if (n > 0) {
            path[n] = '\0';
            ad = _upp_yol_ad(path);
            i = 0;
            while (ad[i] && i + 1 < cap) {
                out[i] = ad[i];
                i++;
            }
            out[i] = '\0';
            if (i > 0) {
                return 1;
            }
        }
        f = fopen("/proc/self/comm", "r");
        if (!f) {
            return 0;
        }
        i = 0;
        while ((c = fgetc(f)) != EOF && i + 1 < cap) {
            if (c == '\n' || c == '\r') {
                break;
            }
            out[i++] = (char)c;
        }
        out[i] = '\0';
        fclose(f);
        return i > 0;
    }
#else
    (void)i;
    return 0;
#endif
}

static long long upp_bellek_baglan(const char* islem_adi) {
    char self[256];
    _upp_bellek_kilitle();
    _upp_bellek_self = 0;
    if (!islem_adi || !islem_adi[0]) {
        _upp_bellek_hata = 1;
        _upp_bellek_birak();
        return 0;
    }
    if (_upp_kendi_ad(self, sizeof(self)) && _upp_ad_exe(islem_adi, self)) {
        _upp_bellek_self = 1;
        _upp_bellek_hata = 0;
        _upp_bellek_birak();
        return 1;
    }
    if (_upp_argc > 0 && _upp_argv && _upp_argv[0] && _upp_ad_exe(islem_adi, _upp_argv[0])) {
        _upp_bellek_self = 1;
        _upp_bellek_hata = 0;
        _upp_bellek_birak();
        return 1;
    }
    _upp_bellek_hata = 1;
    _upp_bellek_birak();
    return 0;
}

static long long upp_bellek_modul_bul(const char* dll_adi) {
    FILE* f;
    char satir[512];
    long long base = 0;
    _upp_bellek_kilitle();
    if (!_upp_bellek_self || !dll_adi) {
        _upp_bellek_birak();
        return 0;
    }
    f = fopen("/proc/self/maps", "r");
    if (!f) {
        _upp_bellek_birak();
        return 0;
    }
    while (fgets(satir, sizeof(satir), f)) {
        char* nl;
        char* p;
        unsigned long long bas, son;
        size_t ln;
        nl = strchr(satir, '\n');
        if (nl) {
            *nl = '\0';
        }
        ln = strlen(satir);
        if (ln > 0 && satir[ln - 1] == '\r') {
            satir[ln - 1] = '\0';
        }
        if (sscanf(satir, "%llx-%llx", &bas, &son) != 2) {
            continue;
        }
        (void)son;
        p = strchr(satir, '/');
        if (!p) {
            continue;
        }
        if (_upp_ad_exe(dll_adi, p)) {
            base = (long long)bas;
            break;
        }
    }
    fclose(f);
    _upp_bellek_birak();
    return base;
}

static int _upp_rpm(uintptr_t addr, void* buf, size_t n) {
    if (!_upp_bellek_self || !buf) {
        _upp_bellek_hata = 1;
        return 0;
    }
    memcpy(buf, (void*)addr, n);
    _upp_bellek_hata = 0;
    return 1;
}

static int _upp_wpm(uintptr_t addr, const void* buf, size_t n) {
    if (!_upp_bellek_self || !buf) {
        _upp_bellek_hata = 1;
        return 0;
    }
    memcpy((void*)addr, buf, n);
    _upp_bellek_hata = 0;
    return 1;
}

static long long upp_bellek_sayioku(long long adres) {
    long long v = 0;
    _upp_bellek_kilitle();
    if (!_upp_rpm((uintptr_t)adres, &v, sizeof(v))) {
        _upp_bellek_birak();
        return 0;
    }
    _upp_bellek_birak();
    return v;
}

static double upp_bellek_oku_ondalik(long long adres) {
    double v = 0.0;
    _upp_bellek_kilitle();
    if (!_upp_rpm((uintptr_t)adres, &v, sizeof(v))) {
        _upp_bellek_birak();
        return 0.0;
    }
    _upp_bellek_birak();
    return v;
}

static char* upp_bellek_oku_metin(long long adres, long long uzunluk) {
    char* p;
    int ok;
    if (uzunluk <= 0) {
        return NULL;
    }
    if (uzunluk > 1048576) {
        uzunluk = 1048576;
    }
    p = (char*)malloc((size_t)uzunluk + 1u);
    if (!p) {
        return NULL;
    }
    memset(p, 0, (size_t)uzunluk + 1u);
    _upp_bellek_kilitle();
    ok = _upp_rpm((uintptr_t)adres, p, (size_t)uzunluk);
    _upp_bellek_birak();
    if (!ok) {
        free(p);
        return NULL;
    }
    p[uzunluk] = '\0';
    return _upp_m_sahiplen(p);
}

static long long upp_bellek_sayiyaz(long long adres, long long deger) {
    long long r;
    _upp_bellek_kilitle();
    r = _upp_wpm((uintptr_t)adres, &deger, sizeof(deger)) ? 1 : 0;
    _upp_bellek_birak();
    return r;
}

static long long upp_bellek_yaz_ondalik(long long adres, double deger) {
    long long r;
    _upp_bellek_kilitle();
    r = _upp_wpm((uintptr_t)adres, &deger, sizeof(deger)) ? 1 : 0;
    _upp_bellek_birak();
    return r;
}

static long long upp_bellek_zincir_oku(long long taban, const long long* offs, long long n) {
    unsigned long long addr = (unsigned long long)taban;
    long long i;
    _upp_bellek_kilitle();
    if (!offs || n <= 0) {
        _upp_bellek_birak();
        return 0;
    }
    for (i = 0; i < n; i++) {
        unsigned long long ptr = 0;
        if (!_upp_rpm((uintptr_t)addr, &ptr, sizeof(ptr))) {
            _upp_bellek_birak();
            return 0;
        }
        addr = ptr + (unsigned long long)offs[i];
    }
    _upp_bellek_birak();
    return (long long)addr;
}

static long long upp_bellek_yama_yap(long long adres, const char* hex) {
    unsigned char buf[256];
    size_t nb = 0;
    const char* p;
    long long r;
    if (!hex) {
        return 0;
    }
    p = hex;
    while (*p && nb < sizeof(buf)) {
        unsigned int b = 0;
        while (*p == ' ' || *p == '\t' || *p == ',') {
            p++;
        }
        if (!*p) {
            break;
        }
        if (sscanf(p, "%2x", &b) != 1) {
            return 0;
        }
        buf[nb++] = (unsigned char)b;
        if (p[0] && p[1] && ((p[0] >= '0' && p[0] <= '9') || (p[0] >= 'A' && p[0] <= 'F') || (p[0] >= 'a' && p[0] <= 'f'))
            && ((p[1] >= '0' && p[1] <= '9') || (p[1] >= 'A' && p[1] <= 'F') || (p[1] >= 'a' && p[1] <= 'f'))) {
            p += 2;
        } else {
            return 0;
        }
    }
    if (!nb) {
        return 0;
    }
    _upp_bellek_kilitle();
    r = _upp_wpm((uintptr_t)adres, buf, nb) ? 1 : 0;
    _upp_bellek_birak();
    return r;
}

static long long upp_bellek_koruma_degistir(long long adres, long long boyut, const char* izin) {
    (void)adres;
    (void)boyut;
    (void)izin;
    return 0;
}

static long long upp_bellek_koruma_degistir_sayi(long long adres, long long boyut, long long izin) {
    (void)adres;
    (void)boyut;
    (void)izin;
    return 0;
}

static long long upp_bellek_son_hata(void) {
    long long v;
    _upp_bellek_kilitle();
    v = _upp_bellek_hata;
    _upp_bellek_birak();
    return v;
}

static long long upp_bellek_okundu(void) {
    long long v;
    _upp_bellek_kilitle();
    v = _upp_bellek_hata == 0 ? 1 : 0;
    _upp_bellek_birak();
    return v;
}
#endif

/* ---- upp.cizim (katmanlı overlay) ---- */

#if UPP_WIN
typedef struct {
    int kind; /* 0 kutu, 1 yazi */
    int x, y, w, h;
    COLORREF renk;
    char yazi[192];
} _UppCizimKomut;

static _UppCizimKomut* _upp_cizim_k = NULL;
static int _upp_cizim_cap = 0;
static int _upp_cizim_n = 0;
static HWND _upp_cizim_hwnd = NULL;
static HWND _upp_cizim_hedef = NULL;
static HANDLE _upp_cizim_th = NULL;
static CRITICAL_SECTION _upp_cizim_cs;
static int _upp_cizim_cs_ok = 0;
static const COLORREF _upp_cizim_anahtar = RGB(255, 0, 255);

static COLORREF _upp_renk(const char* ad) {
    char buf[32];
    size_t i, n;
    if (!ad) return RGB(255, 0, 0);
    n = strlen(ad);
    if (n >= sizeof(buf)) n = sizeof(buf) - 1;
    for (i = 0; i < n; i++) {
        char c = ad[i];
        if (c >= 'a' && c <= 'z') c = (char)(c - 32);
        buf[i] = c;
    }
    buf[n] = '\0';
    if (!strcmp(buf, "KIRMIZI") || !strcmp(buf, "RED")) return RGB(255, 40, 40);
    if (!strcmp(buf, "YESIL") || !strcmp(buf, "GREEN")) return RGB(40, 220, 40);
    if (!strcmp(buf, "MAVI") || !strcmp(buf, "BLUE")) return RGB(40, 120, 255);
    if (!strcmp(buf, "SARI") || !strcmp(buf, "YELLOW")) return RGB(255, 220, 0);
    if (!strcmp(buf, "BEYAZ") || !strcmp(buf, "WHITE")) return RGB(255, 255, 255);
    if (!strcmp(buf, "SIYAH") || !strcmp(buf, "BLACK")) return RGB(20, 20, 20);
    if (!strcmp(buf, "TURUNCU") || !strcmp(buf, "ORANGE")) return RGB(255, 140, 0);
    if (!strcmp(buf, "PEMBE") || !strcmp(buf, "PINK")) return RGB(255, 105, 180);
    if (!strcmp(buf, "GRI") || !strcmp(buf, "GRAY") || !strcmp(buf, "GREY")) return RGB(180, 180, 180);
    if (!strcmp(buf, "CYAN") || !strcmp(buf, "CAMGOBEGI")) return RGB(0, 220, 220);
    return RGB(255, 40, 40);
}

static void _upp_cizim_cs_hazir(void) {
    if (!_upp_cizim_cs_ok) {
        InitializeCriticalSection(&_upp_cizim_cs);
        _upp_cizim_cs_ok = 1;
    }
}

static int _upp_cizim_ekle(const _UppCizimKomut* k) {
    if (_upp_cizim_n >= _upp_cizim_cap) {
        int ncap = _upp_cizim_cap ? _upp_cizim_cap * 2 : 128;
        _UppCizimKomut* p = (_UppCizimKomut*)realloc(_upp_cizim_k, (size_t)ncap * sizeof(*p));
        if (!p) {
            fprintf(stderr, "[u++] cizim kuyrugu dolu (bellek yetersiz)\n");
            return 0;
        }
        _upp_cizim_k = p;
        _upp_cizim_cap = ncap;
    }
    _upp_cizim_k[_upp_cizim_n++] = *k;
    return 1;
}

typedef struct { DWORD pid; HWND hwnd; int alan; } _UppHwndAra;

static BOOL CALLBACK _upp_hwnd_cb(HWND hwnd, LPARAM lp) {
    _UppHwndAra* a = (_UppHwndAra*)lp;
    DWORD pid = 0;
    RECT r;
    int alan;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid != a->pid) return TRUE;
    if (!IsWindowVisible(hwnd)) return TRUE;
    if (!GetWindowRect(hwnd, &r)) return TRUE;
    alan = (r.right - r.left) * (r.bottom - r.top);
    if (alan > a->alan) {
        a->alan = alan;
        a->hwnd = hwnd;
    }
    return TRUE;
}

static HWND _upp_pencere_bul(DWORD pid) {
    _UppHwndAra a;
    a.pid = pid;
    a.hwnd = NULL;
    a.alan = 0;
    EnumWindows(_upp_hwnd_cb, (LPARAM)&a);
    return a.hwnd;
}

static void _upp_cizim_boya(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    RECT rc;
    HBRUSH brush;
    int i;
    GetClientRect(hwnd, &rc);
    brush = CreateSolidBrush(_upp_cizim_anahtar);
    FillRect(hdc, &rc, brush);
    DeleteObject(brush);
    SetBkMode(hdc, TRANSPARENT);
    EnterCriticalSection(&_upp_cizim_cs);
    for (i = 0; i < _upp_cizim_n; i++) {
        _UppCizimKomut* k = &_upp_cizim_k[i];
        if (k->kind == 0) {
            HPEN pen = CreatePen(PS_SOLID, 2, k->renk);
            HGDIOBJ old = SelectObject(hdc, pen);
            HGDIOBJ oldb = SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Rectangle(hdc, k->x, k->y, k->x + k->w, k->y + k->h);
            SelectObject(hdc, oldb);
            SelectObject(hdc, old);
            DeleteObject(pen);
        } else {
            wchar_t wbuf[192];
            int nw = MultiByteToWideChar(CP_UTF8, 0, k->yazi, -1, wbuf, 192);
            SetTextColor(hdc, k->renk);
            if (nw > 0) TextOutW(hdc, k->x, k->y, wbuf, nw - 1);
            else TextOutA(hdc, k->x, k->y, k->yazi, (int)strlen(k->yazi));
        }
    }
    LeaveCriticalSection(&_upp_cizim_cs);
    EndPaint(hwnd, &ps);
}

static LRESULT CALLBACK _upp_cizim_wnd(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_PAINT) {
        _upp_cizim_boya(hwnd);
        return 0;
    }
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

static DWORD WINAPI _upp_cizim_dongu(LPVOID param) {
    WNDCLASSA wc;
    HWND hedef = (HWND)param;
    RECT r;
    MSG msg;
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = _upp_cizim_wnd;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "UppCizimOverlay";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassA(&wc);
    GetWindowRect(hedef, &r);
    _upp_cizim_hwnd = CreateWindowExA(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        "UppCizimOverlay", "", WS_POPUP,
        r.left, r.top, r.right - r.left, r.bottom - r.top,
        NULL, NULL, wc.hInstance, NULL);
    if (_upp_cizim_hwnd) {
        SetLayeredWindowAttributes(_upp_cizim_hwnd, _upp_cizim_anahtar, 0, LWA_COLORKEY);
        ShowWindow(_upp_cizim_hwnd, SW_SHOWNOACTIVATE);
        SetTimer(_upp_cizim_hwnd, 1, 16, NULL);
    }
    while (GetMessageA(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_TIMER && _upp_cizim_hedef && IsWindow(_upp_cizim_hedef) && _upp_cizim_hwnd) {
            RECT nr;
            if (GetWindowRect(_upp_cizim_hedef, &nr)) {
                MoveWindow(_upp_cizim_hwnd, nr.left, nr.top, nr.right - nr.left, nr.bottom - nr.top, TRUE);
            }
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    _upp_cizim_hwnd = NULL;
    return 0;
}

static long long upp_cizim_baslat(const char* surec) {
    DWORD pid;
    HWND hedef;
    _upp_cizim_cs_hazir();
    pid = _upp_pid_bul(surec);
    if (!pid) return 0;
    hedef = _upp_pencere_bul(pid);
    if (!hedef) return 0;
    _upp_cizim_hedef = hedef;
    /* overlay iş parçacığı bittiyse (hwnd yok) yeniden başlat; yaşıyorsa hedefi güncelle */
    if (_upp_cizim_th && !_upp_cizim_hwnd) {
        CloseHandle(_upp_cizim_th);
        _upp_cizim_th = NULL;
    }
    if (!_upp_cizim_th) {
        _upp_cizim_th = CreateThread(NULL, 0, _upp_cizim_dongu, hedef, 0, NULL);
        if (!_upp_cizim_th) return 0;
        /* pencere oluşana kadar kısa bekle */
        {
            int i;
            for (i = 0; i < 50 && !_upp_cizim_hwnd; i++) Sleep(10);
        }
    }
    return _upp_cizim_hwnd ? 1 : 0;
}

static void upp_cizim_temizle(void) {
    _upp_cizim_cs_hazir();
    EnterCriticalSection(&_upp_cizim_cs);
    _upp_cizim_n = 0;
    LeaveCriticalSection(&_upp_cizim_cs);
    if (_upp_cizim_hwnd) InvalidateRect(_upp_cizim_hwnd, NULL, TRUE);
}

static void upp_cizim_kutu(long long x, long long y, long long w, long long h, const char* renk) {
    _UppCizimKomut k;
    _upp_cizim_cs_hazir();
    memset(&k, 0, sizeof(k));
    k.kind = 0;
    k.x = (int)x; k.y = (int)y; k.w = (int)w; k.h = (int)h;
    k.renk = _upp_renk(renk);
    EnterCriticalSection(&_upp_cizim_cs);
    _upp_cizim_ekle(&k);
    LeaveCriticalSection(&_upp_cizim_cs);
    if (_upp_cizim_hwnd) InvalidateRect(_upp_cizim_hwnd, NULL, TRUE);
}

static void upp_cizim_yazi(long long x, long long y, const char* metin, const char* renk) {
    _UppCizimKomut k;
    _upp_cizim_cs_hazir();
    memset(&k, 0, sizeof(k));
    k.kind = 1;
    k.x = (int)x; k.y = (int)y;
    k.renk = _upp_renk(renk);
    if (metin) {
        strncpy(k.yazi, metin, sizeof(k.yazi) - 1);
    }
    EnterCriticalSection(&_upp_cizim_cs);
    _upp_cizim_ekle(&k);
    LeaveCriticalSection(&_upp_cizim_cs);
    if (_upp_cizim_hwnd) InvalidateRect(_upp_cizim_hwnd, NULL, TRUE);
}
#else
/* Linux: cizim (overlay) X11 gerektirir; desteklenmiyor. */
static long long upp_cizim_baslat(const char* surec) {
    (void)surec;
    return 0;
}
static void upp_cizim_temizle(void) {
}
static void upp_cizim_kutu(long long x, long long y, long long w, long long h, const char* renk) {
    (void)x; (void)y; (void)w; (void)h; (void)renk;
}
static void upp_cizim_yazi(long long x, long long y, const char* metin, const char* renk) {
    (void)x; (void)y; (void)metin; (void)renk;
}
#endif

/* ---- upp.json / upp.stdio / upp.sistem (v2.5) ---- */

#include <fcntl.h>
#if UPP_WIN
#include <io.h>
#endif
#include <errno.h>

#define UPP_JT_BOS 0
#define UPP_JT_NULL 1
#define UPP_JT_MANTIK 2
#define UPP_JT_SAYI 3
#define UPP_JT_ONDALIK 4
#define UPP_JT_METIN 5
#define UPP_JT_DIZI 6
#define UPP_JT_NESNE 7

#define UPP_JSON_MAX_DERINLIK 64
#define UPP_JSON_MAX_DUGUM 1000000LL

typedef struct {
    int tur;
    int canli;
    long long sayi;
    double ondalik;
    char* metin;
    char** anahtarlar;
    long long* ogeler;
    long long n;
    long long cap;
} UppJsonHucre;

typedef struct {
    const char* s;
    size_t len;
    size_t i;
    int hata;
    int derin;
} UppJsonOkuyucu;

typedef struct {
    char* p;
    size_t n;
    size_t cap;
} UppJsonYazi;

static UppJsonHucre* _upp_json_h = NULL;
static long long _upp_json_n = 1;
static long long _upp_json_cap = 0;
static int _upp_stdio_ikili_hazir = 0;

static JSONDeger _upp_json_id(long long id) {
    JSONDeger v;
    v.id = id;
    return v;
}

static UppJsonHucre* _upp_json_hucre(long long id) {
    if (id <= 0 || id >= _upp_json_n) {
        return NULL;
    }
    if (!_upp_json_h[id].canli) {
        return NULL;
    }
    return &_upp_json_h[id];
}

static long long _upp_json_yeni(int tur) {
    long long i;
    UppJsonHucre* np;
    long long nc;
    for (i = 1; i < _upp_json_n; i++) {
        if (!_upp_json_h[i].canli) {
            memset(&_upp_json_h[i], 0, sizeof(UppJsonHucre));
            _upp_json_h[i].tur = tur;
            _upp_json_h[i].canli = 1;
            return i;
        }
    }
    if (_upp_json_n >= UPP_JSON_MAX_DUGUM) {
        return 0;
    }
    if (_upp_json_n + 1 > _upp_json_cap) {
        nc = _upp_json_cap ? _upp_json_cap * 2 : 16;
        if (nc < _upp_json_n + 1) {
            nc = _upp_json_n + 1;
        }
        np = (UppJsonHucre*)realloc(_upp_json_h, (size_t)nc * sizeof(UppJsonHucre));
        if (!np) {
            return 0;
        }
        memset(np + _upp_json_cap, 0, (size_t)(nc - _upp_json_cap) * sizeof(UppJsonHucre));
        _upp_json_h = np;
        _upp_json_cap = nc;
    }
    i = _upp_json_n++;
    memset(&_upp_json_h[i], 0, sizeof(UppJsonHucre));
    _upp_json_h[i].tur = tur;
    _upp_json_h[i].canli = 1;
    return i;
}

static int _upp_json_oge_ekle(UppJsonHucre* h, long long id) {
    long long nc;
    if (!h) {
        return 0;
    }
    if (h->n + 1 > h->cap) {
        long long* ogeler2;
        char** anahtarlar2 = NULL;
        nc = h->cap ? h->cap * 2 : 4;
        ogeler2 = (long long*)malloc((size_t)nc * sizeof(long long));
        if (!ogeler2) {
            return 0;
        }
        if (h->n > 0 && h->ogeler) {
            memcpy(ogeler2, h->ogeler, (size_t)h->n * sizeof(long long));
        }
        memset(ogeler2 + h->n, 0, (size_t)(nc - h->n) * sizeof(long long));
        if (h->tur == UPP_JT_NESNE) {
            anahtarlar2 = (char**)malloc((size_t)nc * sizeof(char*));
            if (!anahtarlar2) {
                free(ogeler2);
                return 0;
            }
            if (h->n > 0 && h->anahtarlar) {
                memcpy(anahtarlar2, h->anahtarlar, (size_t)h->n * sizeof(char*));
            }
            memset(anahtarlar2 + h->n, 0, (size_t)(nc - h->n) * sizeof(char*));
        }
        free(h->ogeler);
        h->ogeler = ogeler2;
        if (h->tur == UPP_JT_NESNE) {
            free(h->anahtarlar);
            h->anahtarlar = anahtarlar2;
        }
        h->cap = nc;
    }
    h->ogeler[h->n] = id;
    h->n++;
    return 1;
}

static char* _upp_json_kopya(const char* s, size_t n) {
    char* p;
    if (!s) {
        s = "";
        n = 0;
    }
    p = (char*)malloc(n + 1);
    if (!p) {
        return NULL;
    }
    memcpy(p, s, n);
    p[n] = 0;
    return p;
}

static void _upp_json_bosalt_id(long long id) {
    UppJsonHucre* h;
    long long i;
    h = _upp_json_hucre(id);
    if (!h) {
        return;
    }
    h->canli = 0;
    if (h->tur == UPP_JT_DIZI || h->tur == UPP_JT_NESNE) {
        for (i = 0; i < h->n; i++) {
            _upp_json_bosalt_id(h->ogeler[i]);
        }
    }
    if (h->tur == UPP_JT_NESNE && h->anahtarlar) {
        for (i = 0; i < h->n; i++) {
            free(h->anahtarlar[i]);
        }
    }
    free(h->metin);
    free(h->anahtarlar);
    free(h->ogeler);
    memset(h, 0, sizeof(UppJsonHucre));
}

static void _upp_json_atla(UppJsonOkuyucu* o) {
    while (o->i < o->len) {
        char c = o->s[o->i];
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            break;
        }
        o->i++;
    }
}

static int _upp_json_beklenen(UppJsonOkuyucu* o, char c) {
    _upp_json_atla(o);
    if (o->i >= o->len || o->s[o->i] != c) {
        o->hata = 1;
        return 0;
    }
    o->i++;
    return 1;
}

static int _upp_utf8_kod(unsigned long cp, char* out) {
    if (cp <= 0x7Fu) {
        out[0] = (char)cp;
        return 1;
    }
    if (cp <= 0x7FFu) {
        out[0] = (char)(0xC0u | (cp >> 6));
        out[1] = (char)(0x80u | (cp & 0x3Fu));
        return 2;
    }
    if (cp <= 0xFFFFu) {
        out[0] = (char)(0xE0u | (cp >> 12));
        out[1] = (char)(0x80u | ((cp >> 6) & 0x3Fu));
        out[2] = (char)(0x80u | (cp & 0x3Fu));
        return 3;
    }
    if (cp <= 0x10FFFFu) {
        out[0] = (char)(0xF0u | (cp >> 18));
        out[1] = (char)(0x80u | ((cp >> 12) & 0x3Fu));
        out[2] = (char)(0x80u | ((cp >> 6) & 0x3Fu));
        out[3] = (char)(0x80u | (cp & 0x3Fu));
        return 4;
    }
    return 0;
}

static int _upp_hex4(const char* s, unsigned long* out) {
    unsigned long v = 0;
    int i;
    for (i = 0; i < 4; i++) {
        unsigned char c = (unsigned char)s[i];
        v <<= 4;
        if (c >= '0' && c <= '9') {
            v += (unsigned long)(c - '0');
        } else if (c >= 'a' && c <= 'f') {
            v += (unsigned long)(c - 'a' + 10);
        } else if (c >= 'A' && c <= 'F') {
            v += (unsigned long)(c - 'A' + 10);
        } else {
            return 0;
        }
    }
    *out = v;
    return 1;
}

static long long _upp_json_oku_deger(UppJsonOkuyucu* o);

static long long _upp_json_oku_metin(UppJsonOkuyucu* o) {
    char* buf;
    size_t n = 0;
    size_t cap = 32;
    long long id;
    if (!_upp_json_beklenen(o, '"')) {
        return 0;
    }
    buf = (char*)malloc(cap);
    if (!buf) {
        o->hata = 1;
        return 0;
    }
    while (o->i < o->len) {
        unsigned char c = (unsigned char)o->s[o->i];
        char tmp[8];
        int tn = 1;
        if (c == '"') {
            o->i++;
            buf[n] = 0;
            id = _upp_json_yeni(UPP_JT_METIN);
            if (!id) {
                free(buf);
                o->hata = 1;
                return 0;
            }
            _upp_json_h[id].metin = buf;
            return id;
        }
        o->i++;
        if (c == '\\') {
            if (o->i >= o->len) {
                free(buf);
                o->hata = 1;
                return 0;
            }
            c = (unsigned char)o->s[o->i++];
            if (c == '"' || c == '\\' || c == '/') {
                tmp[0] = (char)c;
            } else if (c == 'b') {
                tmp[0] = '\b';
            } else if (c == 'f') {
                tmp[0] = '\f';
            } else if (c == 'n') {
                tmp[0] = '\n';
            } else if (c == 'r') {
                tmp[0] = '\r';
            } else if (c == 't') {
                tmp[0] = '\t';
            } else if (c == 'u') {
                unsigned long cp;
                if (o->i + 4 > o->len || !_upp_hex4(o->s + o->i, &cp)) {
                    free(buf);
                    o->hata = 1;
                    return 0;
                }
                o->i += 4;
                if (cp >= 0xD800u && cp <= 0xDBFFu && o->i + 6 <= o->len && o->s[o->i] == '\\' && o->s[o->i + 1] == 'u') {
                    unsigned long lo;
                    if (_upp_hex4(o->s + o->i + 2, &lo) && lo >= 0xDC00u && lo <= 0xDFFFu) {
                        cp = 0x10000u + (((cp - 0xD800u) << 10) | (lo - 0xDC00u));
                        o->i += 6;
                    }
                }
                tn = _upp_utf8_kod(cp, tmp);
                if (tn <= 0) {
                    free(buf);
                    o->hata = 1;
                    return 0;
                }
            } else {
                free(buf);
                o->hata = 1;
                return 0;
            }
        } else {
            if (c < 0x20) {
                free(buf);
                o->hata = 1;
                return 0;
            }
            tmp[0] = (char)c;
        }
        if (n + (size_t)tn + 1 > cap) {
            char* nb;
            cap *= 2;
            if (cap < n + (size_t)tn + 1) {
                cap = n + (size_t)tn + 1;
            }
            nb = (char*)realloc(buf, cap);
            if (!nb) {
                free(buf);
                o->hata = 1;
                return 0;
            }
            buf = nb;
        }
        memcpy(buf + n, tmp, (size_t)tn);
        n += (size_t)tn;
    }
    free(buf);
    o->hata = 1;
    return 0;
}

static long long _upp_json_oku_sayi(UppJsonOkuyucu* o) {
    size_t bas;
    int ondalik = 0;
    char tmp[128];
    size_t n;
    long long id;
    _upp_json_atla(o);
    bas = o->i;
    if (o->i < o->len && o->s[o->i] == '-') {
        o->i++;
    }
    if (o->i >= o->len || o->s[o->i] < '0' || o->s[o->i] > '9') {
        o->hata = 1;
        return 0;
    }
    if (o->s[o->i] == '0') {
        o->i++;
    } else {
        while (o->i < o->len && o->s[o->i] >= '0' && o->s[o->i] <= '9') {
            o->i++;
        }
    }
    if (o->i < o->len && o->s[o->i] == '.') {
        ondalik = 1;
        o->i++;
        if (o->i >= o->len || o->s[o->i] < '0' || o->s[o->i] > '9') {
            o->hata = 1;
            return 0;
        }
        while (o->i < o->len && o->s[o->i] >= '0' && o->s[o->i] <= '9') {
            o->i++;
        }
    }
    if (o->i < o->len && (o->s[o->i] == 'e' || o->s[o->i] == 'E')) {
        ondalik = 1;
        o->i++;
        if (o->i < o->len && (o->s[o->i] == '+' || o->s[o->i] == '-')) {
            o->i++;
        }
        if (o->i >= o->len || o->s[o->i] < '0' || o->s[o->i] > '9') {
            o->hata = 1;
            return 0;
        }
        while (o->i < o->len && o->s[o->i] >= '0' && o->s[o->i] <= '9') {
            o->i++;
        }
    }
    n = o->i - bas;
    if (n >= sizeof(tmp)) {
        o->hata = 1;
        return 0;
    }
    memcpy(tmp, o->s + bas, n);
    tmp[n] = 0;
    if (!ondalik) {
        char* end = NULL;
        long long v;
        errno = 0;
        v = strtoll(tmp, &end, 10);
        if (errno != ERANGE) {
            id = _upp_json_yeni(UPP_JT_SAYI);
            if (!id) {
                o->hata = 1;
                return 0;
            }
            _upp_json_h[id].sayi = v;
            return id;
        }
        ondalik = 1;
    }
    id = _upp_json_yeni(UPP_JT_ONDALIK);
    if (!id) {
        o->hata = 1;
        return 0;
    }
    _upp_json_h[id].ondalik = strtod(tmp, NULL);
    return id;
}

static int _upp_json_kelime(UppJsonOkuyucu* o, const char* w) {
    size_t n = strlen(w);
    _upp_json_atla(o);
    if (o->i + n > o->len || memcmp(o->s + o->i, w, n) != 0) {
        return 0;
    }
    o->i += n;
    return 1;
}

static long long _upp_json_oku_dizi(UppJsonOkuyucu* o) {
    long long id;
    if (!_upp_json_beklenen(o, '[')) {
        return 0;
    }
    id = _upp_json_yeni(UPP_JT_DIZI);
    if (!id) {
        o->hata = 1;
        return 0;
    }
    _upp_json_atla(o);
    if (o->i < o->len && o->s[o->i] == ']') {
        o->i++;
        return id;
    }
    for (;;) {
        long long el = _upp_json_oku_deger(o);
        if (o->hata || !el || !_upp_json_oge_ekle(&_upp_json_h[id], el)) {
            o->hata = 1;
            if (el) {
                _upp_json_bosalt_id(el);
            }
            _upp_json_bosalt_id(id);
            return 0;
        }
        _upp_json_atla(o);
        if (o->i < o->len && o->s[o->i] == ',') {
            o->i++;
            continue;
        }
        if (!_upp_json_beklenen(o, ']')) {
            _upp_json_bosalt_id(id);
            return 0;
        }
        return id;
    }
}

static long long _upp_json_oku_nesne(UppJsonOkuyucu* o) {
    long long id;
    if (!_upp_json_beklenen(o, '{')) {
        return 0;
    }
    id = _upp_json_yeni(UPP_JT_NESNE);
    if (!id) {
        o->hata = 1;
        return 0;
    }
    _upp_json_atla(o);
    if (o->i < o->len && o->s[o->i] == '}') {
        o->i++;
        return id;
    }
    for (;;) {
        long long kid;
        long long vid;
        char* key;
        _upp_json_atla(o);
        kid = _upp_json_oku_metin(o);
        if (o->hata || !kid) {
            o->hata = 1;
            _upp_json_bosalt_id(id);
            return 0;
        }
        key = _upp_json_h[kid].metin;
        _upp_json_h[kid].metin = NULL;
        _upp_json_bosalt_id(kid);
        if (!_upp_json_beklenen(o, ':')) {
            free(key);
            _upp_json_bosalt_id(id);
            return 0;
        }
        vid = _upp_json_oku_deger(o);
        if (o->hata || !vid) {
            free(key);
            if (vid) {
                _upp_json_bosalt_id(vid);
            }
            o->hata = 1;
            _upp_json_bosalt_id(id);
            return 0;
        }
        if (!_upp_json_oge_ekle(&_upp_json_h[id], vid)) {
            free(key);
            _upp_json_bosalt_id(vid);
            o->hata = 1;
            _upp_json_bosalt_id(id);
            return 0;
        }
        _upp_json_h[id].anahtarlar[_upp_json_h[id].n - 1] = key;
        _upp_json_atla(o);
        if (o->i < o->len && o->s[o->i] == ',') {
            o->i++;
            continue;
        }
        if (!_upp_json_beklenen(o, '}')) {
            _upp_json_bosalt_id(id);
            return 0;
        }
        return id;
    }
}

static long long _upp_json_oku_deger(UppJsonOkuyucu* o) {
    long long id;
    _upp_json_atla(o);
    if (o->hata || o->i >= o->len) {
        o->hata = 1;
        return 0;
    }
    if (o->derin >= UPP_JSON_MAX_DERINLIK) {
        o->hata = 1;
        return 0;
    }
    o->derin++;
    if (o->s[o->i] == '"') {
        id = _upp_json_oku_metin(o);
    } else if (o->s[o->i] == '{') {
        id = _upp_json_oku_nesne(o);
    } else if (o->s[o->i] == '[') {
        id = _upp_json_oku_dizi(o);
    } else if (o->s[o->i] == '-' || (o->s[o->i] >= '0' && o->s[o->i] <= '9')) {
        id = _upp_json_oku_sayi(o);
    } else if (_upp_json_kelime(o, "true")) {
        id = _upp_json_yeni(UPP_JT_MANTIK);
        if (id) {
            _upp_json_h[id].sayi = 1;
        }
    } else if (_upp_json_kelime(o, "false")) {
        id = _upp_json_yeni(UPP_JT_MANTIK);
        if (id) {
            _upp_json_h[id].sayi = 0;
        }
    } else if (_upp_json_kelime(o, "null")) {
        id = _upp_json_yeni(UPP_JT_NULL);
    } else {
        o->hata = 1;
        id = 0;
    }
    o->derin--;
    if (!id) {
        o->hata = 1;
    }
    return id;
}

static int _upp_json_yazi_ekle(UppJsonYazi* y, const char* s, size_t ln) {
    char* np;
    size_t nc;
    if (!s) {
        return 1;
    }
    if (y->n + ln + 1 > y->cap) {
        nc = y->cap ? y->cap * 2 : 256;
        while (nc < y->n + ln + 1) {
            nc *= 2;
        }
        np = (char*)realloc(y->p, nc);
        if (!np) {
            return 0;
        }
        y->p = np;
        y->cap = nc;
    }
    memcpy(y->p + y->n, s, ln);
    y->n += ln;
    y->p[y->n] = 0;
    return 1;
}

static int _upp_json_yazi_s(UppJsonYazi* y, const char* s) {
    return _upp_json_yazi_ekle(y, s, s ? strlen(s) : 0);
}

static int _upp_json_yazi_metin(UppJsonYazi* y, const char* s) {
    size_t i;
    size_t n;
    if (!_upp_json_yazi_s(y, "\"")) {
        return 0;
    }
    n = s ? strlen(s) : 0;
    for (i = 0; i < n; i++) {
        unsigned char c = (unsigned char)s[i];
        char buf[8];
        if (c == '"' || c == '\\') {
            buf[0] = '\\';
            buf[1] = (char)c;
            if (!_upp_json_yazi_ekle(y, buf, 2)) {
                return 0;
            }
        } else if (c == '\n') {
            if (!_upp_json_yazi_s(y, "\\n")) {
                return 0;
            }
        } else if (c == '\r') {
            if (!_upp_json_yazi_s(y, "\\r")) {
                return 0;
            }
        } else if (c == '\t') {
            if (!_upp_json_yazi_s(y, "\\t")) {
                return 0;
            }
        } else if (c < 0x20) {
            snprintf(buf, sizeof(buf), "\\u%04x", c);
            if (!_upp_json_yazi_s(y, buf)) {
                return 0;
            }
        } else {
            buf[0] = (char)c;
            if (!_upp_json_yazi_ekle(y, buf, 1)) {
                return 0;
            }
        }
    }
    return _upp_json_yazi_s(y, "\"");
}

static int _upp_json_yaz_id(UppJsonYazi* y, long long id, int derin) {
    UppJsonHucre* h = _upp_json_hucre(id);
    char b[64];
    long long i;
    if (!h || derin > UPP_JSON_MAX_DERINLIK) {
        return _upp_json_yazi_s(y, "null");
    }
    if (h->tur == UPP_JT_NULL) {
        return _upp_json_yazi_s(y, "null");
    }
    if (h->tur == UPP_JT_MANTIK) {
        return _upp_json_yazi_s(y, h->sayi ? "true" : "false");
    }
    if (h->tur == UPP_JT_SAYI) {
        snprintf(b, sizeof(b), "%lld", h->sayi);
        return _upp_json_yazi_s(y, b);
    }
    if (h->tur == UPP_JT_ONDALIK) {
        snprintf(b, sizeof(b), "%.17g", h->ondalik);
        return _upp_json_yazi_s(y, b);
    }
    if (h->tur == UPP_JT_METIN) {
        return _upp_json_yazi_metin(y, h->metin ? h->metin : "");
    }
    if (h->tur == UPP_JT_DIZI) {
        if (!_upp_json_yazi_s(y, "[")) {
            return 0;
        }
        for (i = 0; i < h->n; i++) {
            if (i && !_upp_json_yazi_s(y, ",")) {
                return 0;
            }
            if (!_upp_json_yaz_id(y, h->ogeler[i], derin + 1)) {
                return 0;
            }
        }
        return _upp_json_yazi_s(y, "]");
    }
    if (h->tur == UPP_JT_NESNE) {
        if (!_upp_json_yazi_s(y, "{")) {
            return 0;
        }
        for (i = 0; i < h->n; i++) {
            if (i && !_upp_json_yazi_s(y, ",")) {
                return 0;
            }
            if (!_upp_json_yazi_metin(y, h->anahtarlar[i] ? h->anahtarlar[i] : "")) {
                return 0;
            }
            if (!_upp_json_yazi_s(y, ":")) {
                return 0;
            }
            if (!_upp_json_yaz_id(y, h->ogeler[i], derin + 1)) {
                return 0;
            }
        }
        return _upp_json_yazi_s(y, "}");
    }
    return _upp_json_yazi_s(y, "null");
}

static JSONDeger upp_json_ayristir(const char* s) {
    UppJsonOkuyucu o;
    long long id;
    memset(&o, 0, sizeof(o));
    o.s = s ? s : "";
    o.len = strlen(o.s);
    if (o.len >= 3 && (unsigned char)o.s[0] == 0xEF && (unsigned char)o.s[1] == 0xBB && (unsigned char)o.s[2] == 0xBF) {
        o.i = 3;
    }
    id = _upp_json_oku_deger(&o);
    _upp_json_atla(&o);
    if (o.hata || !id || o.i != o.len) {
        if (id) {
            _upp_json_bosalt_id(id);
        }
        return _upp_json_id(0);
    }
    return _upp_json_id(id);
}

static char* upp_json_olustur(JSONDeger v) {
    UppJsonYazi y;
    memset(&y, 0, sizeof(y));
    if (!_upp_json_yaz_id(&y, v.id, 0)) {
        free(y.p);
        return NULL;
    }
    return _upp_m_sahiplen(y.p ? y.p : _upp_json_kopya("", 0));
}

static char* upp_json_tur(JSONDeger v) {
    UppJsonHucre* h = _upp_json_hucre(v.id);
    const char* ad = "yok";
    if (h) {
        if (h->tur == UPP_JT_MANTIK) {
            ad = "mantik";
        } else if (h->tur == UPP_JT_SAYI) {
            ad = "sayi";
        } else if (h->tur == UPP_JT_ONDALIK) {
            ad = "ondalik";
        } else if (h->tur == UPP_JT_METIN) {
            ad = "metin";
        } else if (h->tur == UPP_JT_DIZI) {
            ad = "dizi";
        } else if (h->tur == UPP_JT_NESNE) {
            ad = "nesne";
        }
    }
    return _upp_m_c(ad);
}

static bool upp_json_var_mi(JSONDeger v) {
    return _upp_json_hucre(v.id) != NULL;
}

static long long upp_json_uzunluk(JSONDeger v) {
    UppJsonHucre* h = _upp_json_hucre(v.id);
    if (!h) {
        return 0;
    }
    if (h->tur == UPP_JT_DIZI || h->tur == UPP_JT_NESNE) {
        return h->n;
    }
    if (h->tur == UPP_JT_METIN && h->metin) {
        return _upp_utf8_say(h->metin, strlen(h->metin));
    }
    return 0;
}

static JSONDeger upp_json_anahtar_al(JSONDeger v, const char* anahtar) {
    UppJsonHucre* h = _upp_json_hucre(v.id);
    long long i;
    if (!h || h->tur != UPP_JT_NESNE || !anahtar) {
        return _upp_json_id(0);
    }
    for (i = 0; i < h->n; i++) {
        if (h->anahtarlar[i] && strcmp(h->anahtarlar[i], anahtar) == 0) {
            return _upp_json_id(h->ogeler[i]);
        }
    }
    return _upp_json_id(0);
}

static JSONDeger upp_json_dizi_eleman(JSONDeger v, long long i) {
    UppJsonHucre* h = _upp_json_hucre(v.id);
    if (!h || h->tur != UPP_JT_DIZI || i < 0 || i >= h->n) {
        return _upp_json_id(0);
    }
    return _upp_json_id(h->ogeler[i]);
}

static JSONDeger upp_json_anahtarlar(JSONDeger v) {
    UppJsonHucre* h = _upp_json_hucre(v.id);
    long long arr;
    long long i;
    if (!h || h->tur != UPP_JT_NESNE) {
        return _upp_json_id(0);
    }
    arr = _upp_json_yeni(UPP_JT_DIZI);
    if (!arr) {
        return _upp_json_id(0);
    }
    for (i = 0; i < h->n; i++) {
        long long sid = _upp_json_yeni(UPP_JT_METIN);
        const char* k = h->anahtarlar[i] ? h->anahtarlar[i] : "";
        if (!sid) {
            _upp_json_bosalt_id(arr);
            return _upp_json_id(0);
        }
        _upp_json_h[sid].metin = _upp_json_kopya(k, strlen(k));
        if (!_upp_json_h[sid].metin) {
            _upp_json_bosalt_id(sid);
            _upp_json_bosalt_id(arr);
            return _upp_json_id(0);
        }
        if (!_upp_json_oge_ekle(&_upp_json_h[arr], sid)) {
            _upp_json_bosalt_id(sid);
            _upp_json_bosalt_id(arr);
            return _upp_json_id(0);
        }
    }
    return _upp_json_id(arr);
}

static char* upp_json_metin_al(JSONDeger v) {
    UppJsonHucre* h = _upp_json_hucre(v.id);
    if (!h || h->tur != UPP_JT_METIN || !h->metin) {
        return NULL;
    }
    return _upp_m_sahiplen(_upp_json_kopya(h->metin, strlen(h->metin)));
}

static long long upp_json_sayi_al(JSONDeger v) {
    UppJsonHucre* h = _upp_json_hucre(v.id);
    if (!h) {
        return 0;
    }
    if (h->tur == UPP_JT_SAYI) {
        return h->sayi;
    }
    if (h->tur == UPP_JT_ONDALIK) {
        return (long long)h->ondalik;
    }
    if (h->tur == UPP_JT_MANTIK) {
        return h->sayi ? 1 : 0;
    }
    return 0;
}

static double upp_json_ondalik_al(JSONDeger v) {
    UppJsonHucre* h = _upp_json_hucre(v.id);
    if (!h) {
        return 0.0;
    }
    if (h->tur == UPP_JT_ONDALIK) {
        return h->ondalik;
    }
    if (h->tur == UPP_JT_SAYI) {
        return (double)h->sayi;
    }
    return 0.0;
}

static bool upp_json_mantik_al(JSONDeger v) {
    UppJsonHucre* h = _upp_json_hucre(v.id);
    if (!h) {
        return 0;
    }
    if (h->tur == UPP_JT_MANTIK) {
        return h->sayi != 0;
    }
    if (h->tur == UPP_JT_SAYI) {
        return h->sayi != 0;
    }
    return 0;
}

static JSONDeger upp_json_nesne(void) {
    return _upp_json_id(_upp_json_yeni(UPP_JT_NESNE));
}

static JSONDeger upp_json_dizi(void) {
    return _upp_json_id(_upp_json_yeni(UPP_JT_DIZI));
}

static JSONDeger upp_json_metin_yap(const char* s) {
    long long id = _upp_json_yeni(UPP_JT_METIN);
    if (!id) {
        return _upp_json_id(0);
    }
    _upp_json_h[id].metin = _upp_json_kopya(s ? s : "", s ? strlen(s) : 0);
    if (!_upp_json_h[id].metin) {
        _upp_json_bosalt_id(id);
        return _upp_json_id(0);
    }
    return _upp_json_id(id);
}

static JSONDeger upp_json_sayi_yap(long long n) {
    long long id = _upp_json_yeni(UPP_JT_SAYI);
    if (!id) {
        return _upp_json_id(0);
    }
    _upp_json_h[id].sayi = n;
    return _upp_json_id(id);
}

static JSONDeger upp_json_ondalik_yap(double n) {
    long long id = _upp_json_yeni(UPP_JT_ONDALIK);
    if (!id) {
        return _upp_json_id(0);
    }
    _upp_json_h[id].ondalik = n;
    return _upp_json_id(id);
}

static JSONDeger upp_json_mantik_yap(bool b) {
    long long id = _upp_json_yeni(UPP_JT_MANTIK);
    if (!id) {
        return _upp_json_id(0);
    }
    _upp_json_h[id].sayi = b ? 1 : 0;
    return _upp_json_id(id);
}

static JSONDeger upp_json_yok_yap(void) {
    return _upp_json_id(_upp_json_yeni(UPP_JT_NULL));
}

static void upp_json_koy(JSONDeger nesne, const char* anahtar, JSONDeger deger) {
    UppJsonHucre* h = _upp_json_hucre(nesne.id);
    long long i;
    char* key;
    if (!h || h->tur != UPP_JT_NESNE || !anahtar) {
        return;
    }
    for (i = 0; i < h->n; i++) {
        if (h->anahtarlar[i] && strcmp(h->anahtarlar[i], anahtar) == 0) {
            if (h->ogeler[i] != deger.id) {
                _upp_json_bosalt_id(h->ogeler[i]);
                h->ogeler[i] = deger.id;
            }
            return;
        }
    }
    key = _upp_json_kopya(anahtar, strlen(anahtar));
    if (!key || !_upp_json_oge_ekle(h, deger.id)) {
        free(key);
        return;
    }
    h->anahtarlar[h->n - 1] = key;
}

static void upp_json_ekle(JSONDeger dizi, JSONDeger deger) {
    UppJsonHucre* h = _upp_json_hucre(dizi.id);
    if (!h || h->tur != UPP_JT_DIZI) {
        return;
    }
    _upp_json_oge_ekle(h, deger.id);
}

static void upp_json_bosalt(JSONDeger v) {
    _upp_json_bosalt_id(v.id);
}

static int _upp_ascii_nicmp(const char* a, const char* b, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        unsigned char ca = (unsigned char)a[i];
        unsigned char cb = (unsigned char)b[i];
        if (ca >= 'A' && ca <= 'Z') {
            ca = (unsigned char)(ca - 'A' + 'a');
        }
        if (cb >= 'A' && cb <= 'Z') {
            cb = (unsigned char)(cb - 'A' + 'a');
        }
        if (ca != cb) {
            return (int)ca - (int)cb;
        }
        if (ca == 0) {
            return 0;
        }
    }
    return 0;
}

#if UPP_WIN
static void _upp_stdio_ikili(void) {
    if (_upp_stdio_ikili_hazir) {
        return;
    }
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    _upp_stdio_ikili_hazir = 1;
}
#else
static void _upp_stdio_ikili(void) {
    /* Linux: CRLF çevirisi yok; ikili mod gerekmez. */
    _upp_stdio_ikili_hazir = 1;
}
#endif

static char* upp_stdio_gelen_oku(void) {
    char baslik[8192];
    size_t bn = 0;
    int c;
    long long uzunluk = -1;
    char* govde;
    size_t okunan = 0;
    _upp_stdio_ikili();
    for (;;) {
        c = fgetc(stdin);
        if (c == EOF) {
            return NULL;
        }
        if (bn + 1 >= sizeof(baslik)) {
            return NULL;
        }
        baslik[bn++] = (char)c;
        if (bn >= 4 && memcmp(baslik + bn - 4, "\r\n\r\n", 4) == 0) {
            break;
        }
        if (bn >= 2 && memcmp(baslik + bn - 2, "\n\n", 2) == 0) {
            break;
        }
    }
    baslik[bn] = 0;
    {
        char* p = baslik;
        while (*p) {
            char* nl;
            if (_upp_ascii_nicmp(p, "Content-Length:", 15) == 0) {
                p += 15;
                while (*p == ' ' || *p == '\t') {
                    p++;
                }
                uzunluk = strtoll(p, NULL, 10);
            }
            nl = strstr(p, "\n");
            if (!nl) {
                break;
            }
            p = nl + 1;
        }
    }
    if (uzunluk < 0 || uzunluk > 64LL * 1024LL * 1024LL) {
        return NULL;
    }
    govde = (char*)malloc((size_t)uzunluk + 1);
    if (!govde) {
        return NULL;
    }
    while (okunan < (size_t)uzunluk) {
        size_t r = fread(govde + okunan, 1, (size_t)uzunluk - okunan, stdin);
        if (r == 0) {
            free(govde);
            return NULL;
        }
        okunan += r;
    }
    govde[uzunluk] = 0;
    return _upp_m_sahiplen(govde);
}

static void upp_stdio_yaz(const char* metin) {
    size_t n = metin ? strlen(metin) : 0;
    _upp_stdio_ikili();
    fprintf(stdout, "Content-Length: %zu\r\n\r\n", n);
    if (n && metin) {
        fwrite(metin, 1, n, stdout);
    }
    fflush(stdout);
}

#if UPP_WIN
static SurecCikti upp_sistem_calistir(const char* komut) {
    SECURITY_ATTRIBUTES sa;
    HANDLE rd = NULL;
    HANDLE wr = NULL;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    char* satir;
    char* cikti;
    size_t n = 0;
    size_t cap = 4096;
    DWORD okunan;
    DWORD exit_kod = 1;
    char buf[4096];
    SurecCikti r;
    r.cikti = NULL;
    r.kod = 1;
    r.basarili = 0;
    if (!komut || !komut[0]) {
        return r;
    }
    memset(&sa, 0, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    if (!CreatePipe(&rd, &wr, &sa, 0)) {
        return r;
    }
    SetHandleInformation(rd, HANDLE_FLAG_INHERIT, 0);
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = wr;
    si.hStdError = wr;
    /* Yakalama kipi etkileşimsizdir: alt süreç üst sürecin stdin'ini tüketmesin. */
    si.hStdInput = CreateFileA("NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, 0, NULL);
    memset(&pi, 0, sizeof(pi));
    satir = (char*)malloc(strlen(komut) + 16);
    if (!satir) {
        CloseHandle(rd);
        CloseHandle(wr);
        return r;
    }
    snprintf(satir, strlen(komut) + 16, "cmd.exe /C %s", komut);
    if (!CreateProcessA(NULL, satir, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        free(satir);
        CloseHandle(rd);
        CloseHandle(wr);
        if (si.hStdInput != INVALID_HANDLE_VALUE) CloseHandle(si.hStdInput);
        return r;
    }
    free(satir);
    CloseHandle(wr);
    if (si.hStdInput != INVALID_HANDLE_VALUE) CloseHandle(si.hStdInput);
    cikti = (char*)malloc(cap);
    if (!cikti) {
        CloseHandle(rd);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return r;
    }
    cikti[0] = 0;
    while (ReadFile(rd, buf, sizeof(buf), &okunan, NULL) && okunan) {
        if (n + okunan + 1 > cap) {
            char* np;
            cap *= 2;
            if (cap < n + okunan + 1) {
                cap = n + okunan + 1;
            }
            np = (char*)realloc(cikti, cap);
            if (!np) {
                free(cikti);
                CloseHandle(rd);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                return r;
            }
            cikti = np;
        }
        memcpy(cikti + n, buf, okunan);
        n += okunan;
        cikti[n] = 0;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, &exit_kod);
    CloseHandle(rd);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    r.cikti = _upp_m_sahiplen(cikti);
    r.kod = (long long)exit_kod;
    r.basarili = (exit_kod == 0);
    return r;
}
#else
static SurecCikti upp_sistem_calistir(const char* komut) {
    FILE* f;
    char* cikti;
    size_t n = 0;
    size_t cap = 4096;
    char buf[4096];
    size_t rd;
    int st;
    SurecCikti r;
    r.cikti = NULL;
    r.kod = 1;
    r.basarili = 0;
    if (!komut || !komut[0]) {
        return r;
    }
    {
        /* Windows sürümü gibi stderr de yakalanır. */
        size_t kn = strlen(komut);
        char* tam = (char*)malloc(kn + 20u);
        if (!tam) {
            return r;
        }
        memcpy(tam, komut, kn);
        memcpy(tam + kn, " 2>&1 </dev/null", 17u);
        f = popen(tam, "r");
        free(tam);
    }
    if (!f) {
        return r;
    }
    cikti = (char*)malloc(cap);
    if (!cikti) {
        pclose(f);
        return r;
    }
    cikti[0] = 0;
    while ((rd = fread(buf, 1, sizeof(buf), f)) > 0) {
        if (n + rd + 1 > cap) {
            char* np;
            cap *= 2;
            if (cap < n + rd + 1) {
                cap = n + rd + 1;
            }
            np = (char*)realloc(cikti, cap);
            if (!np) {
                free(cikti);
                pclose(f);
                return r;
            }
            cikti = np;
        }
        memcpy(cikti + n, buf, rd);
        n += rd;
        cikti[n] = 0;
    }
    st = pclose(f);
    r.cikti = _upp_m_sahiplen(cikti);
    if (st < 0) {
        r.kod = 1;
    } else {
#ifdef WIFEXITED
        r.kod = WIFEXITED(st) ? WEXITSTATUS(st) : 1;
#else
        r.kod = (long long)st;
#endif
    }
    r.basarili = (r.kod == 0);
    return r;
}
#endif

/* Komutu konsolu (stdin/stdout/stderr) devralarak çalıştırır; çıkış kodunu döner. */
static long long upp_sistem_kos(const char* komut) {
#if UPP_WIN
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    DWORD kod = 1;
    char* satir;
    size_t n;
    if (!komut) {
        return 1;
    }
    fflush(stdout);
    fflush(stderr);
    n = strlen(komut) + 16u;
    satir = (char*)malloc(n);
    if (!satir) {
        return 1;
    }
    snprintf(satir, n, "cmd.exe /C \"%s\"", komut);
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    /* stdin/stdout/stderr boru da olsa alt sürece açıkça aktarılır. */
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    if (si.hStdInput && si.hStdInput != INVALID_HANDLE_VALUE) {
        SetHandleInformation(si.hStdInput, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    }
    if (si.hStdOutput && si.hStdOutput != INVALID_HANDLE_VALUE) {
        SetHandleInformation(si.hStdOutput, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    }
    if (si.hStdError && si.hStdError != INVALID_HANDLE_VALUE) {
        SetHandleInformation(si.hStdError, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    }
    memset(&pi, 0, sizeof(pi));
    if (!CreateProcessA(NULL, satir, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        free(satir);
        return 1;
    }
    free(satir);
    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, &kod);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return (long long)kod;
#else
    int st;
    if (!komut) {
        return 1;
    }
    fflush(stdout);
    fflush(stderr);
    st = system(komut);
    if (st == -1) {
        return 1;
    }
    if (WIFEXITED(st)) {
        return WEXITSTATUS(st);
    }
    return 128;
#endif
}

static char* upp_sistem_komut_bul(const char* ad) {
    if (!ad || !ad[0]) {
        return NULL;
    }
#if UPP_WIN
    {
        char buf[MAX_PATH];
        DWORD n = SearchPathA(NULL, ad, ".exe", MAX_PATH, buf, NULL);
        if (n == 0 || n >= MAX_PATH) {
            n = SearchPathA(NULL, ad, NULL, MAX_PATH, buf, NULL);
        }
        if (n == 0 || n >= MAX_PATH) {
            return NULL;
        }
        return _upp_m_c(buf);
    }
#else
    {
        const char* path;
        char* kopya;
        char* tok;
        char deneme[4096];
        if (strchr(ad, '/')) {
            if (access(ad, X_OK) == 0) {
                return _upp_m_c(ad);
            }
            return NULL;
        }
        path = getenv("PATH");
        if (!path) {
            return NULL;
        }
        kopya = upp_metin_derin(path);
        if (!kopya) {
            return NULL;
        }
        tok = strtok(kopya, ":");
        while (tok) {
            snprintf(deneme, sizeof(deneme), "%s/%s", tok, ad);
            if (access(deneme, X_OK) == 0) {
                char* out = _upp_m_c(deneme);
                upp_metin_release(kopya);
                return out;
            }
            tok = strtok(NULL, ":");
        }
        upp_metin_release(kopya);
        return NULL;
    }
#endif
}

static char* upp_sistem_exe_dizin(void) {
#if UPP_WIN
    char buf[MAX_PATH];
    char* slash;
    if (!GetModuleFileNameA(NULL, buf, MAX_PATH)) {
        if (!GetCurrentDirectoryA(MAX_PATH, buf)) {
            return _upp_m_c(".");
        }
        return _upp_m_c(buf);
    }
    slash = strrchr(buf, '\\');
    if (!slash) {
        slash = strrchr(buf, '/');
    }
    if (slash) {
        *slash = 0;
    }
    return _upp_m_c(buf[0] ? buf : ".");
#else
    char buf[4096];
#if defined(_WIN32)
    /* MinGW --linux: readlink yok; çalışma dizini yeter (gerçek Linux gcc POSIX dalı). */
    if (!getcwd(buf, sizeof(buf))) {
        return _upp_m_c(".");
    }
    return _upp_m_c(buf);
#else
    {
        ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
        char* slash;
        if (n <= 0) {
            if (!getcwd(buf, sizeof(buf))) {
                return _upp_m_c(".");
            }
            return _upp_m_c(buf);
        }
        buf[n] = 0;
        slash = strrchr(buf, '/');
        if (slash) {
            *slash = 0;
        }
        return _upp_m_c(buf[0] ? buf : ".");
    }
#endif
#endif
}

static char* upp_platform(void) {
#if UPP_WIN
    return _upp_m_c("windows");
#else
    return _upp_m_c("linux");
#endif
}

static long long upp_sistem_pid(void) {
#if UPP_WIN
    return (long long)GetCurrentProcessId();
#else
    return (long long)getpid();
#endif
}

static void upp_sistem_cikis(long long kod) {
    fflush(stdout);
    fflush(stderr);
    exit((int)kod);
}

static long long upp_ortam_yaz(const char* ad, const char* deger) {
    if (!ad || !ad[0]) return 0;
#if UPP_WIN
    return _putenv_s(ad, deger ? deger : "") == 0 ? 1 : 0;
#else
    if (!deger) return unsetenv(ad) == 0 ? 1 : 0;
    return setenv(ad, deger, 1) == 0 ? 1 : 0;
#endif
}

static long long upp_dosya_boyut(const char* yol) {
    if (!yol) return -1;
#if UPP_WIN
    WIN32_FILE_ATTRIBUTE_DATA fad;
    LARGE_INTEGER size;
    if (!GetFileAttributesExA(yol, GetFileExInfoStandard, &fad)) return -1;
    size.LowPart = fad.nFileSizeLow;
    size.HighPart = fad.nFileSizeHigh;
    return (long long)size.QuadPart;
#else
    struct stat st;
    if (stat(yol, &st) != 0) return -1;
    return (long long)st.st_size;
#endif
}

/* Linux-specific APIs (upp.linux.*) */
#if !UPP_WIN && !defined(_WIN32)
static long long upp_linux_pid(void) {
    return (long long)getpid();
}
static long long upp_linux_sinyal_gonder(long long pid, long long sig) {
    return kill((pid_t)pid, (int)sig) == 0 ? 1 : 0;
}
static char* upp_linux_proc_oku(long long pid, const char* dosya) {
    char path[256];
    FILE* f;
    char* buf;
    size_t n = 0, cap = 4096;
    char tmp[1024];
    size_t rd;
    if (!dosya) return NULL;
    snprintf(path, sizeof(path), "/proc/%lld/%s", pid, dosya);
    f = fopen(path, "r");
    if (!f) return NULL;
    buf = (char*)malloc(cap);
    if (!buf) { fclose(f); return NULL; }
    while ((rd = fread(tmp, 1, sizeof(tmp), f)) > 0) {
        if (n + rd + 1 > cap) {
            cap = (cap * 2 > n + rd + 1) ? cap * 2 : n + rd + 1;
            char* nb = (char*)realloc(buf, cap);
            if (!nb) { free(buf); fclose(f); return NULL; }
            buf = nb;
        }
        memcpy(buf + n, tmp, rd);
        n += rd;
    }
    buf[n] = 0;
    fclose(f);
    return _upp_m_sahiplen(buf);
}
static long long upp_linux_sayi_oku(long long pid, long long adres) {
#if defined(__linux__) && defined(_GNU_SOURCE)
    long long val = 0;
    struct iovec local[1];
    struct iovec remote[1];
    local[0].iov_base = &val;
    local[0].iov_len = sizeof(val);
    remote[0].iov_base = (void*)(uintptr_t)adres;
    remote[0].iov_len = sizeof(val);
    if (process_vm_readv((pid_t)pid, local, 1, remote, 1, 0) == sizeof(val)) return val;
    return 0;
#else
    (void)pid; (void)adres;
    return 0;
#endif
}
static long long upp_linux_sayi_yaz(long long pid, long long adres, long long deger) {
#if defined(__linux__) && defined(_GNU_SOURCE)
    struct iovec local[1];
    struct iovec remote[1];
    local[0].iov_base = &deger;
    local[0].iov_len = sizeof(deger);
    remote[0].iov_base = (void*)(uintptr_t)adres;
    remote[0].iov_len = sizeof(deger);
    return process_vm_writev((pid_t)pid, local, 1, remote, 1, 0) == sizeof(deger) ? 1 : 0;
#else
    (void)pid; (void)adres; (void)deger;
    return 0;
#endif
}
#else
static long long upp_linux_pid(void) { return 0; }
static long long upp_linux_sinyal_gonder(long long pid, long long sig) { (void)pid; (void)sig; return 0; }
static char* upp_linux_proc_oku(long long pid, const char* dosya) { (void)pid; (void)dosya; return NULL; }
static long long upp_linux_sayi_oku(long long pid, long long adres) { (void)pid; (void)adres; return 0; }
static long long upp_linux_sayi_yaz(long long pid, long long adres, long long deger) { (void)pid; (void)adres; (void)deger; return 0; }
#endif

/* Windows-specific aliases (upp.windows.*) */
#define upp_windows_bellek_baglan upp_bellek_baglan
#define upp_windows_bellek_modul_bul upp_bellek_modul_bul
#define upp_windows_bellek_sayioku upp_bellek_sayioku
#define upp_windows_bellek_sayiyaz upp_bellek_sayiyaz
#define upp_windows_bellek_oku_ondalik upp_bellek_oku_ondalik
#define upp_windows_bellek_yaz_ondalik upp_bellek_yaz_ondalik
#define upp_windows_bellek_oku_metin upp_bellek_oku_metin
#define upp_windows_bellek_zincir_oku upp_bellek_zincir_oku
#define upp_windows_bellek_yama_yap upp_bellek_yama_yap
#define upp_windows_bellek_koruma_degistir upp_bellek_koruma_degistir
#define upp_windows_bellek_koruma_degistir_sayi upp_bellek_koruma_degistir_sayi
#define upp_windows_bellek_son_hata upp_bellek_son_hata
#define upp_windows_bellek_okundu upp_bellek_okundu

#define upp_windows_girdi_fare_tasi upp_girdi_fare_tasi
#define upp_windows_girdi_fare_tikla upp_girdi_fare_tikla
#define upp_windows_girdi_tus_bas upp_girdi_tus_bas

#define upp_windows_cizim_baslat upp_cizim_baslat
#define upp_windows_cizim_temizle upp_cizim_temizle
#define upp_windows_cizim_kutu upp_cizim_kutu
#define upp_windows_cizim_yazi upp_cizim_yazi

#define upp_windows_mesaj upp_mesaj
#define upp_windows_hata upp_hata
#define upp_windows_ses_cal upp_ses_cal

static int _upp_yol_sep(char c) {
    return c == '/' || c == '\\';
}

static char _upp_yol_ayirac(void) {
#if UPP_WIN
    return '\\';
#else
    return '/';
#endif
}

static char* upp_yol_dizin(const char* yol) {
    char* k;
    char* slash;
    size_t n;
    if (!yol || !yol[0]) {
        return _upp_m_c(".");
    }
    k = upp_metin_derin(yol);
    if (!k) {
        return NULL;
    }
    n = strlen(k);
    while (n > 1 && _upp_yol_sep(k[n - 1])) {
        k[--n] = 0;
    }
    slash = strrchr(k, '\\');
    {
        char* ileri = strrchr(k, '/');
        if (ileri && (!slash || ileri > slash)) {
            slash = ileri;
        }
    }
    if (!slash) {
        upp_metin_release(k);
        return _upp_m_c(".");
    }
    if (slash == k) {
        slash[1] = 0;
        return _upp_m_boy(k, strlen(k));
    }
    *slash = 0;
    if (!k[0]) {
        upp_metin_release(k);
        return _upp_m_c(".");
    }
    return _upp_m_boy(k, strlen(k));
}

static char* upp_yol_birlestir(const char* a, const char* b) {
    size_t na, nb;
    char* p;
    char sep;
    if (!a) {
        a = "";
    }
    if (!b) {
        b = "";
    }
    if (!a[0]) {
        return _upp_m_c(b);
    }
    if (!b[0]) {
        return _upp_m_c(a);
    }
    if (_upp_yol_sep(b[0])
#if UPP_WIN
        || (((b[0] >= 'A' && b[0] <= 'Z') || (b[0] >= 'a' && b[0] <= 'z')) && b[1] == ':')
#endif
    ) {
        return _upp_m_c(b);
    }
    na = strlen(a);
    nb = strlen(b);
    sep = _upp_yol_ayirac();
    p = (char*)malloc(na + nb + 3u);
    if (!p) {
        return NULL;
    }
    memcpy(p, a, na);
    if (!_upp_yol_sep(a[na - 1])) {
        p[na++] = sep;
    }
    memcpy(p + na, b, nb + 1u);
    return _upp_m_sahiplen(p);
}

static char* upp_yol_gercek(const char* yol) {
    if (!yol || !yol[0]) {
        return _upp_m_c("");
    }
#if UPP_WIN
    {
        char buf[MAX_PATH * 2];
        DWORD n = GetFullPathNameA(yol, (DWORD)sizeof(buf), buf, NULL);
        if (n == 0 || n >= sizeof(buf)) {
            return _upp_m_c(yol);
        }
        return _upp_m_c(buf);
    }
#else
    {
        char buf[4096];
#if defined(_WIN32)
        if (!_fullpath(buf, yol, sizeof(buf))) {
            return _upp_m_c(yol);
        }
#else
        if (!realpath(yol, buf)) {
            return _upp_m_c(yol);
        }
#endif
        return _upp_m_c(buf);
    }
#endif
}

static char* upp_yol_calisma(void) {
#if UPP_WIN
    char buf[MAX_PATH];
    if (!GetCurrentDirectoryA(MAX_PATH, buf)) {
        return _upp_m_c(".");
    }
    return _upp_m_c(buf);
#else
    char buf[PATH_MAX];
    if (!getcwd(buf, sizeof(buf))) {
        return _upp_m_c(".");
    }
    return _upp_m_c(buf);
#endif
}

static int upp_yol_mutlak_mi(const char* yol) {
    if (!yol || !yol[0]) {
        return 0;
    }
    if (yol[0] == '/' || yol[0] == '\\') {
        return 1;
    }
#if UPP_WIN
    if (((yol[0] >= 'A' && yol[0] <= 'Z') || (yol[0] >= 'a' && yol[0] <= 'z')) && yol[1] == ':') {
        return 1;
    }
#endif
    return 0;
}

/* Yolun son bileşeni (uzantı dahil). */
static char* upp_yol_dosya_adi(const char* yol) {
    size_t n;
    size_t i;
    if (!yol) {
        return _upp_m_yeni(0);
    }
    n = upp_metin_nbayt(yol);
    while (n > 1 && _upp_yol_sep(yol[n - 1])) {
        n--;
    }
    i = n;
    while (i > 0 && !_upp_yol_sep(yol[i - 1])) {
        i--;
    }
    return _upp_m_bayttan(yol + i, n - i);
}

/* Son bileşenin uzantısı noktasıyla birlikte (".txt"); yoksa "". */
static char* upp_yol_uzanti(const char* yol) {
    size_t n;
    size_t i;
    if (!yol) {
        return _upp_m_yeni(0);
    }
    n = upp_metin_nbayt(yol);
    i = n;
    while (i > 0 && !_upp_yol_sep(yol[i - 1])) {
        if (yol[i - 1] == '.') {
            if (i - 1 == 0 || _upp_yol_sep(yol[i - 2])) {
                break;
            }
            return _upp_m_bayttan(yol + i - 1, n - (i - 1));
        }
        i--;
    }
    return _upp_m_yeni(0);
}

static int upp_yol_dizin_var_mi(const char* yol) {
#if UPP_WIN
    DWORD a;
    if (!yol || !yol[0]) {
        return 0;
    }
    a = GetFileAttributesA(yol);
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
    struct stat st;
    if (!yol || !yol[0]) {
        return 0;
    }
    return stat(yol, &st) == 0 && S_ISDIR(st.st_mode);
#endif
}

static int upp_yol_klasor_olustur(const char* yol) {
    char* buf;
    char* p;
    size_t n;
    if (!yol || !yol[0]) {
        return 1;
    }
    n = strlen(yol);
    buf = (char*)malloc(n + 1u);
    if (!buf) {
        return 0;
    }
    memcpy(buf, yol, n + 1u);
#if UPP_WIN
    for (p = buf; *p; p++) {
        if (*p == '/') {
            *p = '\\';
        }
    }
    p = buf;
    if (p[0] && p[1] == ':') {
        p += 2;
        if (*p == '\\') {
            p++;
        }
    }
    for (; *p; p++) {
        if (*p == '\\') {
            *p = 0;
            if (buf[0]) {
                CreateDirectoryA(buf, NULL);
            }
            *p = '\\';
        }
    }
    if (buf[0]) {
        CreateDirectoryA(buf, NULL);
    }
#else
    p = buf;
    if (*p == '/') {
        p++;
    }
    for (; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (buf[0]) {
#if defined(_WIN32)
                mkdir(buf);
#else
                mkdir(buf, 0755);
#endif
            }
            *p = '/';
        }
    }
    if (buf[0]) {
#if defined(_WIN32)
        mkdir(buf);
#else
        mkdir(buf, 0755);
#endif
    }
#endif
    free(buf);
    return 1;
}

static UppKolListe upp_dosya_listele(const char* yol) {
    UppKolListe L;
    L.h = NULL;
    if (!yol || !yol[0]) {
        yol = ".";
    }
#if UPP_WIN
    {
        char desen[MAX_PATH * 2];
        WIN32_FIND_DATAA fd;
        HANDLE h;
        size_t n = strlen(yol);
        if (n + 3 >= sizeof(desen)) {
            return L;
        }
        memcpy(desen, yol, n + 1u);
        if (n > 0 && desen[n - 1] != '\\' && desen[n - 1] != '/') {
            desen[n++] = '\\';
            desen[n] = 0;
        }
        memcpy(desen + n, "*", 2u);
        h = FindFirstFileA(desen, &fd);
        if (h == INVALID_HANDLE_VALUE) {
            return L;
        }
        do {
            if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) {
                continue;
            }
            _upp_liste_ekle_c(&L, fd.cFileName);
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }
#else
    {
        DIR* d = opendir(yol);
        struct dirent* e;
        if (!d) {
            return L;
        }
        while ((e = readdir(d)) != NULL) {
            if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) {
                continue;
            }
            _upp_liste_ekle_c(&L, e->d_name);
        }
        closedir(d);
    }
#endif
    return L;
}

#define UPP_KILIT_MAX 1000000LL

typedef struct {
    int canli;
    int init;
#if UPP_WIN
    CRITICAL_SECTION cs;
#else
    pthread_mutex_t mu;
#endif
} _UppKilitH;

static _UppKilitH* _upp_kilitler = NULL;
static long long _upp_kilit_n = 1;
static long long _upp_kilit_cap = 0;

static long long _upp_kilit_yeni(void) {
    long long i;
    long long nc;
    _UppKilitH* np;
    for (i = 1; i < _upp_kilit_n; i++) {
        if (!_upp_kilitler[i].canli) {
            memset(&_upp_kilitler[i], 0, sizeof(_UppKilitH));
            _upp_kilitler[i].canli = 1;
            return i;
        }
    }
    if (_upp_kilit_n >= UPP_KILIT_MAX) {
        return 0;
    }
    if (_upp_kilit_n + 1 > _upp_kilit_cap) {
        nc = _upp_kilit_cap ? _upp_kilit_cap * 2 : 8;
        if (nc < _upp_kilit_n + 1) {
            nc = _upp_kilit_n + 1;
        }
        np = (_UppKilitH*)realloc(_upp_kilitler, (size_t)nc * sizeof(_UppKilitH));
        if (!np) {
            return 0;
        }
        memset(np + _upp_kilit_cap, 0, (size_t)(nc - _upp_kilit_cap) * sizeof(_UppKilitH));
        _upp_kilitler = np;
        _upp_kilit_cap = nc;
    }
    i = _upp_kilit_n++;
    memset(&_upp_kilitler[i], 0, sizeof(_UppKilitH));
    _upp_kilitler[i].canli = 1;
    return i;
}

static _UppKilitH* _upp_kilit_hucre(Kilit* k) {
    if (!k) {
        return NULL;
    }
    if (k->id == 0) {
        k->id = _upp_kilit_yeni();
        if (k->id == 0) {
            return NULL;
        }
    }
    if (k->id <= 0 || k->id >= _upp_kilit_n || !_upp_kilitler) {
        return NULL;
    }
    if (!_upp_kilitler[k->id].canli) {
        return NULL;
    }
    return &_upp_kilitler[k->id];
}

static void _upp_kilit_mutex_kur(_UppKilitH* h) {
    if (!h || h->init) {
        return;
    }
#if UPP_WIN
    InitializeCriticalSection(&h->cs);
#else
    pthread_mutex_init(&h->mu, NULL);
#endif
    h->init = 1;
}

static void upp_kilit_hazir(Kilit* k) {
    _upp_kilit_mutex_kur(_upp_kilit_hucre(k));
}

static void upp_kilit_kilitle(Kilit* k) {
    _UppKilitH* h = _upp_kilit_hucre(k);
    if (!h) {
        return;
    }
    _upp_kilit_mutex_kur(h);
#if UPP_WIN
    EnterCriticalSection(&h->cs);
#else
    pthread_mutex_lock(&h->mu);
#endif
}

static void upp_kilit_birak(Kilit* k) {
    _UppKilitH* h;
    if (!k || k->id == 0) {
        return;
    }
    if (k->id <= 0 || k->id >= _upp_kilit_n || !_upp_kilitler) {
        return;
    }
    h = &_upp_kilitler[k->id];
    if (!h->canli || !h->init) {
        return;
    }
#if UPP_WIN
    LeaveCriticalSection(&h->cs);
#else
    pthread_mutex_unlock(&h->mu);
#endif
}

