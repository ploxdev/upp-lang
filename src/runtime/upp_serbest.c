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

/* VGA için küçük ANSI çözümleyici: ESC[...m (renk), ESC[2J (temizle), ESC[H, ESC[K.
 * Seri terminal aynı baytları kendisi yorumlar. */
static int _upp_ansi_durum = 0;
static int _upp_ansi_p[4];
static int _upp_ansi_n = 0;
static int _upp_vga_renk = -1;
static const uint8_t _upp_ansi_vga[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };

static void _upp_vga_temizle(void) {
    int i;
    for (i = 0; i < _UPP_VGA_G * _UPP_VGA_Y; i++) {
        _UPP_VGA[i] = (uint16_t)(0x0700 | ' ');
    }
    _upp_vga_x = 0;
    _upp_vga_y = 0;
}

static void _upp_ansi_uygula(long long son) {
    int i;
    if (son == 'm') {
        if (_upp_ansi_n == 0) {
            _upp_vga_renk = -1;
        }
        for (i = 0; i < _upp_ansi_n; i++) {
            int v = _upp_ansi_p[i];
            int r = _upp_vga_renk < 0 ? 0x07 : _upp_vga_renk;
            if (v == 0) {
                _upp_vga_renk = -1;
            } else if (v == 1) {
                _upp_vga_renk = r | 0x08;
            } else if (v >= 30 && v <= 37) {
                _upp_vga_renk = (r & 0xF8) | _upp_ansi_vga[v - 30];
            } else if (v >= 90 && v <= 97) {
                _upp_vga_renk = (r & 0xF0) | 0x08 | _upp_ansi_vga[v - 90];
            }
        }
    } else if (son == 'J') {
        _upp_vga_temizle();
    } else if (son == 'H') {
        _upp_vga_x = 0;
        _upp_vga_y = 0;
    } else if (son == 'C' || son == 'D') {
        int n = _upp_ansi_n > 0 && _upp_ansi_p[0] > 0 ? _upp_ansi_p[0] : 1;
        _upp_vga_x += son == 'C' ? n : -n;
        if (_upp_vga_x < 0) _upp_vga_x = 0;
        if (_upp_vga_x >= _UPP_VGA_G) _upp_vga_x = _UPP_VGA_G - 1;
    } else if (son == 'K') {
        for (i = _upp_vga_x; i < _UPP_VGA_G; i++) {
            _UPP_VGA[_upp_vga_y * _UPP_VGA_G + i] = (uint16_t)(0x0700 | ' ');
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
            return 1;
        }
        _upp_ansi_durum = 0;
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
            _upp_vga_karakter(_upp_cp437(cp), _upp_vga_renk >= 0 && akis != 2 ? (uint8_t)_upp_vga_renk : renk);
        }
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
static int _upp_tus_ctrl = 0;
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
        switch (sc) {
        case 0x48: _upp_tus_dizi('A'); break;
        case 0x50: _upp_tus_dizi('B'); break;
        case 0x4D: _upp_tus_dizi('C'); break;
        case 0x4B: _upp_tus_dizi('D'); break;
        case 0x47: _upp_tus_dizi('H'); break;
        case 0x4F: _upp_tus_dizi('F'); break;
        case 0x53: _upp_tus_dizi('3'); _upp_tus_koy('~'); break;
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

/* Çekirdek girişinde, upp_ana()'dan önce. */
static void _upp_cekirdek_hazirla(void) {
    _upp_bellek_kur();
    upp_cekirdek_kesmeler_kur();
}

/* =====================================================================
 * Görevler: işbirlikçi çoklu görev. Geçiş yalnızca upp.uyut,
 * upp.cekirdek.sira_ver ve girdi beklerken olur; runtime'ın paylaşılan
 * durumu bu yüzden kilitsiz güvendedir. Görev 1 çekirdeğin ana akışıdır.
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
} UppGorev;

static UppGorev _upp_gorev[_UPP_GOREV_N];
static int _upp_gorev_cur = 0;
static long long _upp_gorev_sonraki = 2;
static int _upp_gorev_kuruldu = 0;

__asm__(
    ".text\n"
    ".global _upp_gorev_gec\n"
    "_upp_gorev_gec:\n"
    "    push %rbp\n    push %rbx\n    push %r12\n    push %r13\n    push %r14\n    push %r15\n"
    "    mov %rsp, (%rdi)\n"
    "    mov %rsi, %rsp\n"
    "    pop %r15\n    pop %r14\n    pop %r13\n    pop %r12\n    pop %rbx\n    pop %rbp\n"
    "    ret\n");
extern void _upp_gorev_gec(uint64_t* eski, uint64_t yeni);

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
    for (i = 0; i < 6; i++) {
        *--sp = 0;
    }
    g->rsp = (uint64_t)(uintptr_t)sp;
    g->f = f;
    g->arg = arg;
    g->ad = upp_metin_kopya(ad ? ad : "");
    g->kimlik = _upp_gorev_sonraki++;
    g->baslama = _upp_tik;
    g->uyan = 0;
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
