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
    "    sysretq\n");
extern long long _upp_kul_gir(uint64_t* kayit, uint64_t giris, uint64_t yigin);
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
    if (_upp_sc_isleyici) {
        r = _upp_sc_isleyici((long long)f->no, (long long)f->a1, (long long)f->a2, (long long)f->a3,
                             (long long)f->a4, (long long)f->a5, (long long)f->a6);
    }
    if (g->kul_cikis) {
        g->kul_cikis = 0;
        __asm__ volatile("cli");
        _upp_kul_don(g->kul_kayit, g->kul_kod);
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
