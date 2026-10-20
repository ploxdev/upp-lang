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
