/* =====================================================================
 * u++ v2.8 çalışma zamanı — upp.* stdlib
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
typedef uint8_t bayt;

#if defined(_MSC_VER)
#define UPP_TLS __declspec(thread)
#else
#define UPP_TLS __thread
#endif

static void upp_panik(const char* mesaj) {
    fprintf(stderr, "[u++ HATA] %s\n", mesaj ? mesaj : "iç hata");
    fflush(stderr);
    abort();
}

static long long upp_dizi_idx(long long i, long long n) {
    if (i < 0 || i >= n) {
        fprintf(stderr, "[u++ HATA] indeks %lld, dizi boyutu %lld'yi aşıyor\n", i, n);
        fflush(stderr);
        abort();
    }
    return i;
}

#include <errno.h>
#if !defined(UPP_WIN) || !UPP_WIN
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#if defined(__has_include)
#  if __has_include(<sys/wait.h>)
#    include <sys/wait.h>
#  endif
#else
#  include <sys/wait.h>
#endif
#endif

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
    buf = (char*)malloc((size_t)size + 1u);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    n = fread(buf, 1, (size_t)size, f);
    buf[n] = '\0';
    fclose(f);
    return buf; /* upp.metin_bosalt / bellek_bosalt ile bırakın */
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
}

static long long upp_arguman_sayisi(void) {
    return (long long)_upp_argc;
}

static char* upp_arguman(long long i) {
    if (i < 0 || i >= (long long)_upp_argc || !_upp_argv) {
        return NULL;
    }
    return _upp_argv[(int)i];
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

static long long upp_metinden_sayiya(const char* s) {
    if (!s) {
        return 0;
    }
    return atoll(s);
}

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
    p = (char*)malloc(n + 1u);
    if (!p) {
        return NULL;
    }
    memcpy(p, v, n + 1u);
    return p;
}

static char* upp_sayidan_metin(long long n) {
    char buf[32];
    char* p;
    size_t len;
    snprintf(buf, sizeof(buf), "%lld", n);
    len = strlen(buf);
    p = (char*)malloc(len + 1u);
    if (!p) {
        return NULL;
    }
    memcpy(p, buf, len + 1u);
    return p;
}

static char* upp_ondalikdan_metin(double x) {
    char buf[64];
    char* p;
    size_t len;
    snprintf(buf, sizeof(buf), "%.17g", x);
    len = strlen(buf);
    p = (char*)malloc(len + 1u);
    if (!p) {
        return NULL;
    }
    memcpy(p, buf, len + 1u);
    return p;
}

static double upp_metinden_ondalik(const char* s) {
    if (!s) {
        return 0.0;
    }
    return atof(s);
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

/* kod/uzunluk/kes her çağrıda baştan tararsa büyük kaynak O(n²) olur. */
typedef struct {
    const char* s;
    size_t nbytes;
    long long ncp;
    size_t* off;
    unsigned char sig0;
    unsigned char sig1;
    unsigned ver;
} UppUtf8Tablo;

static UPP_TLS UppUtf8Tablo _u8t;
static UPP_TLS char* _upp_geo_p = NULL;
static UPP_TLS size_t _upp_geo_cap = 0;
static UPP_TLS unsigned _upp_geo_ver = 0;

static int _u8t_esles(const char* s, size_t n) {
    if (_u8t.s != s || _u8t.nbytes != n || _u8t.ncp < 0 || !_u8t.off) {
        return 0;
    }
    if (s && s == _upp_geo_p && _u8t.ver != _upp_geo_ver) {
        return 0;
    }
    if (n == 0) {
        return 1;
    }
    if (!s) {
        return 0;
    }
    if ((unsigned char)s[0] != _u8t.sig0) {
        return 0;
    }
    if ((unsigned char)s[n - 1] != _u8t.sig1) {
        return 0;
    }
    return 1;
}

static int _u8t_hazir(const char* s) {
    size_t n = s ? strlen(s) : 0;
    size_t cap;
    size_t i;
    long long k;
    size_t* off;
    if (_u8t_esles(s, n)) {
        return 1;
    }
    free(_u8t.off);
    _u8t.off = NULL;
    _u8t.s = s;
    _u8t.nbytes = n;
    _u8t.ncp = 0;
    _u8t.sig0 = (n && s) ? (unsigned char)s[0] : 0;
    _u8t.sig1 = (n && s) ? (unsigned char)s[n - 1] : 0;
    cap = n + 2u;
    if (cap < 8u) {
        cap = 8u;
    }
    off = (size_t*)malloc(cap * sizeof(size_t));
    if (!off) {
        _u8t.ncp = -1;
        _u8t.s = NULL;
        return 0;
    }
    off[0] = 0;
    i = 0;
    k = 0;
    while (i < n) {
        long long a = _upp_utf8_adv(s ? s : "", n, i);
        if (a <= 0) {
            break;
        }
        i += (size_t)a;
        k++;
        if ((size_t)k + 1u >= cap) {
            size_t ncap = cap * 2u;
            size_t* q;
            if (ncap / 2u != cap) {
                ncap = (size_t)k + 2u;
            }
            q = (size_t*)realloc(off, ncap * sizeof(size_t));
            if (!q) {
                free(off);
                _u8t.off = NULL;
                _u8t.ncp = -1;
                _u8t.s = NULL;
                return 0;
            }
            off = q;
            cap = ncap;
        }
        off[k] = i;
    }
    _u8t.off = off;
    _u8t.ncp = k;
    _u8t.ver = (s && s == _upp_geo_p) ? _upp_geo_ver : 0;
    return 1;
}

static size_t _u8t_bayt(const char* s, long long cp) {
    if (cp <= 0) {
        return 0;
    }
    if (!_u8t_hazir(s) || !_u8t.off) {
        size_t n = s ? strlen(s) : 0;
        return _upp_utf8_bayt(s ? s : "", n, cp);
    }
    if (cp >= _u8t.ncp) {
        return _u8t.nbytes;
    }
    return _u8t.off[cp];
}

static long long upp_metin_kod(const char* s, long long i) {
    size_t b;
    size_t n;
    if (i < 0) {
        return 0;
    }
    if (!_u8t_hazir(s) || !_u8t.off) {
        n = s ? strlen(s) : 0;
        b = _upp_utf8_bayt(s ? s : "", n, i);
        if (b >= n) {
            return 0;
        }
        return _upp_utf8_cp_at(s, n, b);
    }
    if (i >= _u8t.ncp) {
        return 0;
    }
    b = _u8t.off[i];
    return _upp_utf8_cp_at(s, _u8t.nbytes, b);
}

static long long upp_metin_ileri(const char* s, long long i) {
    if (i < 0) {
        return 0;
    }
    if (!_u8t_hazir(s) || !_u8t.off) {
        size_t n = s ? strlen(s) : 0;
        size_t b = _upp_utf8_bayt(s ? s : "", n, i);
        if (b >= n) {
            return 0;
        }
        return _upp_utf8_adv(s, n, b);
    }
    if (i >= _u8t.ncp) {
        return 0;
    }
    return (long long)(_u8t.off[i + 1] - _u8t.off[i]);
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

static long long upp_uzunluk(const char* s) {
    if (!_u8t_hazir(s)) {
        size_t n = s ? strlen(s) : 0;
        return _upp_utf8_say(s ? s : "", n);
    }
    return _u8t.ncp;
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
    return p; /* upp.metin_bosalt / bellek_bosalt ile bırakın */
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
    size_t na = a ? strlen(a) : 0u;
    size_t nb = b ? strlen(b) : 0u;
    char* p;
    if (nb > SIZE_MAX - na - 1u) {
        return NULL;
    }
    p = (char*)malloc(na + nb + 1u);
    if (!p) {
        return NULL;
    }
    if (na) {
        memcpy(p, a, na);
    }
    if (nb) {
        memcpy(p + na, b, nb);
    }
    p[na + nb] = '\0';
    return p; /* upp.metin_bosalt / bellek_bosalt ile bırakın */
}

static int upp_metin_esit(const char* a, const char* b) {
    if (a == b) {
        return 1;
    }
    if (!a || !b) {
        return 0;
    }
    return strcmp(a, b) == 0;
}

static void _upp_geo_birak(const char* p) {
    if (p && p == _upp_geo_p) {
        _upp_geo_p = NULL;
        _upp_geo_cap = 0;
        _upp_geo_ver++;
    }
}

static void _u8t_birak_eger(const char* s) {
    if (s && _u8t.s == s) {
        free(_u8t.off);
        _u8t.off = NULL;
        _u8t.s = NULL;
        _u8t.nbytes = 0;
        _u8t.ncp = -1;
        _u8t.sig0 = 0;
        _u8t.sig1 = 0;
        _u8t.ver = 0;
    }
}

static void upp_metin_bosalt(char* s) {
    _upp_geo_birak(s);
    _u8t_birak_eger(s);
    free(s);
}

static char* upp_metin_kopya(const char* s) {
    size_t n;
    char* p;
    if (!s) {
        return NULL;
    }
    n = strlen(s);
    p = (char*)malloc(n + 1u);
    if (!p) {
        return NULL;
    }
    memcpy(p, s, n + 1u);
    return p;
}

static void upp_metin_ekle(char** s, const char* x) {
    char* a;
    const char* b;
    char* kopya;
    char* p;
    size_t na;
    size_t nb;
    size_t need;
    size_t cap;
    if (!s) {
        return;
    }
    a = *s;
    b = x ? x : "";
    kopya = NULL;
    if (a && b && a == b) {
        kopya = upp_metin_kopya(b);
        b = kopya ? kopya : "";
    }
    na = a ? strlen(a) : 0u;
    nb = strlen(b);
    if (nb > SIZE_MAX - na - 1u) {
        if (kopya) {
            free(kopya);
        }
        upp_panik("metin ekleme taşması");
    }
    need = na + nb + 1u;
    if (a && a == _upp_geo_p && _upp_geo_cap >= need) {
        memcpy(a + na, b, nb + 1u);
        _upp_geo_ver++;
        if (kopya) {
            free(kopya);
        }
        return;
    }
    if (a && a == _upp_geo_p && _upp_geo_cap > 0) {
        cap = _upp_geo_cap;
        while (cap < need) {
            if (cap > SIZE_MAX / 2u) {
                cap = need;
                break;
            }
            cap *= 2u;
        }
        p = (char*)realloc(a, cap);
        if (!p) {
            if (kopya) {
                free(kopya);
            }
            upp_panik("metin bellek yetersiz");
        }
        _u8t_birak_eger(a);
        memcpy(p + na, b, nb + 1u);
        *s = p;
        _upp_geo_p = p;
        _upp_geo_cap = cap;
        _upp_geo_ver++;
        if (kopya) {
            free(kopya);
        }
        return;
    }
    cap = need < 32u ? 64u : need * 2u;
    if (cap < need) {
        cap = need;
    }
    p = (char*)malloc(cap);
    if (!p) {
        if (kopya) {
            free(kopya);
        }
        upp_panik("metin bellek yetersiz");
    }
    if (na && a) {
        memcpy(p, a, na);
    }
    memcpy(p + na, b, nb + 1u);
    _upp_geo_birak(a);
    _u8t_birak_eger(a);
    free(a);
    *s = p;
    _upp_geo_p = p;
    _upp_geo_cap = cap;
    _upp_geo_ver++;
    if (kopya) {
        free(kopya);
    }
}

static char* upp_metin_kes(const char* s, long long bas, long long uzunluk) {
    size_t n, start, end, take;
    char* p;
    n = s ? strlen(s) : 0u;
    if (bas < 0) bas = 0;
    if (uzunluk < 0) uzunluk = 0;
    start = _u8t_bayt(s, bas);
    end = _u8t_bayt(s, bas + uzunluk);
    if (end < start) {
        end = start;
    }
    take = end - start;
    p = (char*)malloc(take + 1u);
    if (!p) return NULL;
    if (take) memcpy(p, (s ? s : "") + start, take);
    p[take] = '\0';
    return p;
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
    p = (char*)malloc((j - i) + 1u);
    if (!p) return NULL;
    if (j > i) memcpy(p, s + i, j - i);
    p[j - i] = '\0';
    return p;
}

static char* upp_metin_buyuk(const char* s) {
    size_t n, i;
    char* p;
    if (!s) s = "";
    n = strlen(s);
    p = (char*)malloc(n + 1u);
    if (!p) return NULL;
    for (i = 0; i < n; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c >= 'a' && c <= 'z') c = (unsigned char)(c - 32);
        p[i] = (char)c;
    }
    p[n] = '\0';
    return p;
}

static char* upp_metin_kucuk(const char* s) {
    size_t n, i;
    char* p;
    if (!s) s = "";
    n = strlen(s);
    p = (char*)malloc(n + 1u);
    if (!p) return NULL;
    for (i = 0; i < n; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c >= 'A' && c <= 'Z') c = (unsigned char)(c + 32);
        p[i] = (char)c;
    }
    p[n] = '\0';
    return p;
}

static char* upp_metin_hex_coz(const char* hex) {
    size_t len = hex ? strlen(hex) : 0;
    size_t i, j = 0;
    char* buf;
    if (len == 0) return upp_metin_kopya("");
    buf = (char*)malloc(len / 2 + 1);
    if (!buf) return upp_metin_kopya("");
    for (i = 0; i + 1 < len; i += 2) {
        char c1 = hex[i], c2 = hex[i + 1];
        int v1 = (c1 >= '0' && c1 <= '9') ? (c1 - '0') : ((c1 >= 'a' && c1 <= 'f') ? (c1 - 'a' + 10) : ((c1 >= 'A' && c1 <= 'F') ? (c1 - 'A' + 10) : 0));
        int v2 = (c2 >= '0' && c2 <= '9') ? (c2 - '0') : ((c2 >= 'a' && c2 <= 'f') ? (c2 - 'a' + 10) : ((c2 >= 'A' && c2 <= 'F') ? (c2 - 'A' + 10) : 0));
        buf[j++] = (char)((v1 << 4) | v2);
    }
    buf[j] = 0;
    return buf;
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
        out = (char*)malloc(ns + 1u);
        if (!out) return NULL;
        memcpy(out, s, ns + 1u);
        return out;
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
    out = (char*)malloc(cap);
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
    return out;
}

static void* bellek_ayir(long long boyut) {
    if (boyut <= 0) {
        return NULL;
    }
    return malloc((size_t)boyut);
}

static void bellek_bosalt(void* ptr) {
    free(ptr);
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
} _UppKolHaritaH;

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

static double upp_mat_mutlak(double x) { return fabs(x); }
static double upp_mat_karekok(double x) { return x < 0.0 ? 0.0 : sqrt(x); }
static double upp_mat_us(double a, double b) { return pow(a, b); }
static double upp_mat_sin(double x) { return sin(x); }
static double upp_mat_cos(double x) { return cos(x); }
static double upp_mat_tan(double x) { return tan(x); }
static double upp_mat_atan2(double y, double x) { return atan2(y, x); }
static double upp_mat_taban(double x) { return floor(x); }
static double upp_mat_tavan(double x) { return ceil(x); }
static double upp_mat_yuvarla(double x) { return round(x); }
static double upp_mat_pi(void) { return 3.14159265358979323846; }

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

static double upp_mat_yumusat(double a, double b, double t) {
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;
    return a + (b - a) * t;
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
    return p;
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
    return p;
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

/* ---- liste[T] / harita[K,V] (v2.7) ---- */

static _UppKolListeH* _upp_listeler = NULL;
static long long _upp_liste_n = 1;
static long long _upp_liste_cap = 0;
static _UppKolHaritaH* _upp_haritalar = NULL;
static long long _upp_harita_n = 1;
static long long _upp_harita_cap = 0;

static char* _upp_kol_kopya(const char* s) {
    size_t n;
    char* p;
    if (!s) {
        return NULL;
    }
    n = strlen(s);
    p = (char*)malloc(n + 1u);
    if (!p) {
        return NULL;
    }
    memcpy(p, s, n + 1u);
    return p;
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
        np = (_UppKolListeH*)realloc(_upp_listeler, (size_t)nc * sizeof(_UppKolListeH));
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
        double* p = (double*)realloc(h->d, (size_t)nc * sizeof(double));
        if (!p) {
            return 0;
        }
        h->d = p;
    } else if (h->tag == UPP_KT_METIN) {
        char** p = (char**)realloc(h->s, (size_t)nc * sizeof(char*));
        if (!p) {
            return 0;
        }
        h->s = p;
    } else {
        long long* p = (long long*)realloc(h->i, (size_t)nc * sizeof(long long));
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
            free(h->s[i]);
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
            free(h->s[i]);
        }
    }
    free(h->i);
    free(h->d);
    free(h->s);
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
        if (h->s[idx]) free(h->s[idx]);
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
            if (h->s[j]) free(h->s[j]);
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
        np = (_UppKolHaritaH*)realloc(_upp_haritalar, (size_t)nc * sizeof(_UppKolHaritaH));
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
        char** p = (char**)realloc(h->k_s, (size_t)nc * sizeof(char*));
        if (!p) {
            return 0;
        }
        h->k_s = p;
    } else {
        long long* p = (long long*)realloc(h->k_i, (size_t)nc * sizeof(long long));
        if (!p) {
            return 0;
        }
        h->k_i = p;
    }
    if (h->vtag == UPP_KT_ONDALIK) {
        double* p = (double*)realloc(h->v_d, (size_t)nc * sizeof(double));
        if (!p) {
            return 0;
        }
        h->v_d = p;
    } else if (h->vtag == UPP_KT_METIN) {
        char** p = (char**)realloc(h->v_s, (size_t)nc * sizeof(char*));
        if (!p) {
            return 0;
        }
        h->v_s = p;
    } else {
        long long* p = (long long*)realloc(h->v_i, (size_t)nc * sizeof(long long));
        if (!p) {
            return 0;
        }
        h->v_i = p;
    }
    h->cap = nc;
    return 1;
}

static long long _upp_harita_bul(_UppKolHaritaH* h, long long ki, const char* ks) {
    long long i;
    for (i = 0; i < h->n; i++) {
        if (h->ktag == UPP_KT_METIN) {
            const char* a = h->k_s[i] ? h->k_s[i] : "";
            const char* b = ks ? ks : "";
            if (strcmp(a, b) == 0) {
                return i;
            }
        } else if (h->k_i[i] == ki) {
            return i;
        }
    }
    return -1;
}

static void _upp_harita_yaz_deger(_UppKolHaritaH* h, long long i, long long vi, double vd, const char* vs) {
    if (h->vtag == UPP_KT_ONDALIK) {
        h->v_d[i] = vd;
    } else if (h->vtag == UPP_KT_METIN) {
        if (h->v_s[i]) {
            free(h->v_s[i]);
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
            free(h->k_s[i]);
        }
    }
    if (h->vtag == UPP_KT_METIN && h->v_s) {
        for (i = 0; i < h->n; i++) {
            free(h->v_s[i]);
        }
    }
    free(h->k_i);
    free(h->k_s);
    free(h->v_i);
    free(h->v_d);
    free(h->v_s);
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
        free(h->k_s[idx]);
    }
    if (h->vtag == UPP_KT_METIN && h->v_s[idx]) {
        free(h->v_s[idx]);
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
            if (h->k_s[j]) free(h->k_s[j]);
            h->k_s[j] = NULL;
        }
    }
    if (h->vtag == UPP_KT_METIN && h->v_s) {
        for (j = 0; j < h->n; j++) {
            if (h->v_s[j]) free(h->v_s[j]);
            h->v_s[j] = NULL;
        }
    }
    h->n = 0;
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
            upp_liste_ekle(&L, UPP_KT_METIN, 0, 0.0, tmp);
            i += (size_t)adv;
        }
        return L;
    }
    {
        const char* p = s;
        const char* found;
        while ((found = strstr(p, ayirac)) != NULL) {
            size_t take = (size_t)(found - p);
            char* parca = (char*)malloc(take + 1u);
            if (!parca) {
                return L;
            }
            if (take) {
                memcpy(parca, p, take);
            }
            parca[take] = 0;
            upp_liste_ekle(&L, UPP_KT_METIN, 0, 0.0, parca);
            free(parca);
            p = found + na;
        }
        upp_liste_ekle(&L, UPP_KT_METIN, 0, 0.0, p);
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
        out = (char*)malloc(1u);
        if (!out) {
            return NULL;
        }
        out[0] = 0;
        return out;
    }
    cap = 1u;
    for (i = 0; i < h->n; i++) {
        cap += h->s[i] ? strlen(h->s[i]) : 0u;
        if (i + 1 < h->n) {
            cap += na;
        }
    }
    out = (char*)malloc(cap);
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
    return out;
}

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
    return y.p ? y.p : _upp_json_kopya("", 0);
}

static char* upp_json_tur(JSONDeger v) {
    UppJsonHucre* h = _upp_json_hucre(v.id);
    if (!h) {
        return (char*)"yok";
    }
    if (h->tur == UPP_JT_NULL) {
        return (char*)"yok";
    }
    if (h->tur == UPP_JT_MANTIK) {
        return (char*)"mantik";
    }
    if (h->tur == UPP_JT_SAYI) {
        return (char*)"sayi";
    }
    if (h->tur == UPP_JT_ONDALIK) {
        return (char*)"ondalik";
    }
    if (h->tur == UPP_JT_METIN) {
        return (char*)"metin";
    }
    if (h->tur == UPP_JT_DIZI) {
        return (char*)"dizi";
    }
    if (h->tur == UPP_JT_NESNE) {
        return (char*)"nesne";
    }
    return (char*)"yok";
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
    return _upp_json_kopya(h->metin, strlen(h->metin));
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
    return govde;
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
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
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
        return r;
    }
    free(satir);
    CloseHandle(wr);
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
    r.cikti = cikti;
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
    f = popen(komut, "r");
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
    r.cikti = cikti;
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
        return upp_metin_kopya(buf);
    }
#else
    {
        const char* path;
        char* kopya;
        char* tok;
        char deneme[4096];
        if (strchr(ad, '/')) {
            if (access(ad, X_OK) == 0) {
                return upp_metin_kopya(ad);
            }
            return NULL;
        }
        path = getenv("PATH");
        if (!path) {
            return NULL;
        }
        kopya = upp_metin_kopya(path);
        if (!kopya) {
            return NULL;
        }
        tok = strtok(kopya, ":");
        while (tok) {
            snprintf(deneme, sizeof(deneme), "%s/%s", tok, ad);
            if (access(deneme, X_OK) == 0) {
                char* out = upp_metin_kopya(deneme);
                free(kopya);
                return out;
            }
            tok = strtok(NULL, ":");
        }
        free(kopya);
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
            return upp_metin_kopya(".");
        }
        return upp_metin_kopya(buf);
    }
    slash = strrchr(buf, '\\');
    if (!slash) {
        slash = strrchr(buf, '/');
    }
    if (slash) {
        *slash = 0;
    }
    return upp_metin_kopya(buf[0] ? buf : ".");
#else
    char buf[4096];
#if defined(_WIN32)
    /* MinGW --linux: readlink yok; çalışma dizini yeter (gerçek Linux gcc POSIX dalı). */
    if (!getcwd(buf, sizeof(buf))) {
        return upp_metin_kopya(".");
    }
    return upp_metin_kopya(buf);
#else
    {
        ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
        char* slash;
        if (n <= 0) {
            if (!getcwd(buf, sizeof(buf))) {
                return upp_metin_kopya(".");
            }
            return upp_metin_kopya(buf);
        }
        buf[n] = 0;
        slash = strrchr(buf, '/');
        if (slash) {
            *slash = 0;
        }
        return upp_metin_kopya(buf[0] ? buf : ".");
    }
#endif
#endif
}

static const char* upp_platform(void) {
#if UPP_WIN
    return "windows";
#else
    return "linux";
#endif
}

static long long upp_sistem_pid(void) {
#if UPP_WIN
    return (long long)GetCurrentProcessId();
#else
    return (long long)getpid();
#endif
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
    return buf;
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
        return upp_metin_kopya(".");
    }
    k = upp_metin_kopya(yol);
    if (!k) {
        return NULL;
    }
    n = strlen(k);
    while (n > 1 && _upp_yol_sep(k[n - 1])) {
        k[--n] = 0;
    }
    slash = strrchr(k, '\\');
    if (!slash) {
        slash = strrchr(k, '/');
    }
    if (!slash) {
        free(k);
        return upp_metin_kopya(".");
    }
    if (slash == k) {
        slash[1] = 0;
        return k;
    }
    *slash = 0;
    if (!k[0]) {
        free(k);
        return upp_metin_kopya(".");
    }
    return k;
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
        return upp_metin_kopya(b);
    }
    if (!b[0]) {
        return upp_metin_kopya(a);
    }
    if (_upp_yol_sep(b[0])
#if UPP_WIN
        || (((b[0] >= 'A' && b[0] <= 'Z') || (b[0] >= 'a' && b[0] <= 'z')) && b[1] == ':')
#endif
    ) {
        return upp_metin_kopya(b);
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
    return p;
}

static char* upp_yol_gercek(const char* yol) {
    if (!yol || !yol[0]) {
        return upp_metin_kopya("");
    }
#if UPP_WIN
    {
        char buf[MAX_PATH * 2];
        DWORD n = GetFullPathNameA(yol, (DWORD)sizeof(buf), buf, NULL);
        if (n == 0 || n >= sizeof(buf)) {
            return upp_metin_kopya(yol);
        }
        return upp_metin_kopya(buf);
    }
#else
    {
        char buf[4096];
#if defined(_WIN32)
        if (!_fullpath(buf, yol, sizeof(buf))) {
            return upp_metin_kopya(yol);
        }
#else
        if (!realpath(yol, buf)) {
            return upp_metin_kopya(yol);
        }
#endif
        return upp_metin_kopya(buf);
    }
#endif
}

static char* upp_yol_calisma(void) {
#if UPP_WIN
    char buf[MAX_PATH];
    if (!GetCurrentDirectoryA(MAX_PATH, buf)) {
        return upp_metin_kopya(".");
    }
    return upp_metin_kopya(buf);
#else
    char buf[PATH_MAX];
    if (!getcwd(buf, sizeof(buf))) {
        return upp_metin_kopya(".");
    }
    return upp_metin_kopya(buf);
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
    L.id = 0;
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
            upp_liste_ekle(&L, UPP_KT_METIN, 0, 0.0, fd.cFileName);
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
            upp_liste_ekle(&L, UPP_KT_METIN, 0, 0.0, e->d_name);
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
