/* sctest2: dosya sistemi, tanımlayıcı, zaman, TLS ve sinyal sistem çağrılarını
 * sınar. sctest gibi çıktısı ortamdan bağımsızdır: gerçek Linux'taki ve
 * Uppix'teki çıktı satır satır aynı olmalıdır. Çalışma dizininde sct2_d
 * dizinini kurar ve sonunda siler. */
#include "linux.h"

#define D "sct2_d"

static char tampon[512];
static long tls_blok[8] = {0x1122334455667788L, 7};

static int sonu(const char* s, const char* son) {
    size_t a = uzunluk(s), b = uzunluk(son);
    return a >= b && esit(s + a - b, son);
}

static long fs_oku(void) {
    long v;
    __asm__ volatile("mov %%fs:0, %0" : "=r"(v));
    return v;
}

static long monotonik_ms(void) {
    struct timespec t;
    sc2(SYS_clock_gettime, 1, &t);
    return t.tv_sec * 1000 + t.tv_nsec / 1000000;
}

/* getdents64 ile dizindeki adları sayar; bulunanlar bit olarak döner */
static long dizin_tara(int fd, int* adet) {
    long bit = 0, n, o;
    *adet = 0;
    while ((n = sc3(SYS_getdents64, fd, tampon, sizeof(tampon))) > 0) {
        for (o = 0; o < n;) {
            unsigned short boy = *(unsigned short*)(tampon + o + 16);
            unsigned char tur = (unsigned char)tampon[o + 18];
            const char* ad = tampon + o + 19;
            (*adet)++;
            if (esit(ad, ".") && tur == 4) bit |= 1;
            if (esit(ad, "..") && tur == 4) bit |= 2;
            if (esit(ad, "a.txt") && tur == 8) bit |= 4;
            if (esit(ad, "b") && tur == 8) bit |= 8;
            if (esit(ad, "c") && tur == 4) bit |= 16;
            if (boy % 8 != 0) bit |= 1024;
            o += boy;
        }
    }
    if (n < 0) return n;
    return bit;
}

int main(int argc, char** argv, char** envp) {
    struct stat st;
    struct timespec ts, ts2;
    long fd, dfd, r, t0;
    int adet;
    (void)argc;
    (void)argv;
    (void)envp;
    yaz("=== sctest2 başla ===\n");

    /* dizinler ve *at çağrıları */
    satir("mkdir", sc2(SYS_mkdir, D, 0755));
    satir("mkdir var", sc2(SYS_mkdir, D, 0755));
    dfd = sc3(SYS_open, D, O_RDONLY | O_DIRECTORY, 0);
    satir("open dizin", dfd >= 3);
    fd = sc6(SYS_openat, dfd, (long)"a.txt", O_CREAT | O_RDWR, 0644, 0, 0);
    satir("openat göreli", fd >= 3);
    satir("write", sc3(SYS_write, fd, "0123456789", 10));
    satir("openat creat 2", sc1(SYS_close, sc6(SYS_openat, AT_FDCWD, (long)D "/b", O_CREAT | O_WRONLY, 0600, 0, 0)));
    satir("mkdirat", sc3(SYS_mkdirat, dfd, "c", 0700));
    satir("openat kötü dirfd", sc6(SYS_openat, 99, (long)"a.txt", O_RDONLY, 0, 0, 0));
    satir("openat dosya dirfd", sc6(SYS_openat, fd, (long)"a.txt", O_RDONLY, 0, 0, 0));
    r = sc6(SYS_openat, 99, (long)"/tmp", O_RDONLY | O_DIRECTORY, 0, 0, 0);
    satir("openat mutlak", r >= 3);
    sc1(SYS_close, r);
    satir("newfstatat", sc6(SYS_newfstatat, dfd, (long)"a.txt", (long)&st, 0, 0, 0));
    satir("newfstatat boyut", st.st_size);
    satir("newfstatat boş yol", sc6(SYS_newfstatat, fd, (long)"", (long)&st, AT_EMPTY_PATH, 0, 0));
    satir("newfstatat boş yol boyut", st.st_size);
    satir("newfstatat boş bayraksız", sc6(SYS_newfstatat, fd, (long)"", (long)&st, 0, 0, 0));
    satir("stat izin umask", sc2(SYS_stat, D "/c", &st) == 0 ? (long)(st.st_mode & 0777) : -1L);

    /* getdents64 */
    r = dizin_tara((int)dfd, &adet);
    satir("getdents64 bulunan", r);
    satir("getdents64 adet", adet);
    satir("getdents64 sonda", sc3(SYS_getdents64, dfd, tampon, sizeof(tampon)));
    satir("getdents64 küçük tampon", (sc3(SYS_lseek, dfd, 0, SEEK_SET), sc3(SYS_getdents64, dfd, tampon, 8)));
    satir("getdents64 dosya", sc3(SYS_getdents64, fd, tampon, sizeof(tampon)));

    /* pread / pwrite / readv / writev */
    satir("pwrite", sc6(SYS_pwrite64, fd, (long)"XY", 2, 3, 0, 0));
    satir("lseek değişmedi", sc3(SYS_lseek, fd, 0, SEEK_CUR));
    r = sc6(SYS_pread64, fd, (long)tampon, 8, 1, 0, 0);
    tampon[r > 0 ? r : 0] = 0;
    yaz("pread: ");
    yaz(tampon);
    yaz("\n");
    satir("pread negatif", sc6(SYS_pread64, fd, (long)tampon, 8, -1, 0, 0));
    satir("pread dizin", sc6(SYS_pread64, dfd, (long)tampon, 8, 0, 0, 0));
    {
        struct iovec v[3];
        char a[4], b[8];
        v[0].iov_base = "wr";
        v[0].iov_len = 2;
        v[1].iov_base = "itev\n";
        v[1].iov_len = 5;
        satir("writev", sc3(SYS_writev, 1, v, 2));
        sc3(SYS_lseek, fd, 0, SEEK_SET);
        v[0].iov_base = a;
        v[0].iov_len = 3;
        v[1].iov_base = 0;
        v[1].iov_len = 0;
        v[2].iov_base = b;
        v[2].iov_len = 4;
        satir("readv", sc3(SYS_readv, fd, v, 3));
        satir("readv içerik", a[0] == '0' && a[2] == '2' && b[0] == 'X' && b[1] == 'Y' && b[3] == '6');
        satir("readv geçersiz sayı", sc3(SYS_readv, fd, v, -1));
    }

    /* dup / dup2 / dup3 / fcntl */
    r = sc1(SYS_dup, fd);
    satir("dup", r > fd);
    satir("dup ofset", sc3(SYS_lseek, r, 0, SEEK_CUR));
    sc1(SYS_close, r);
    satir("dup2", sc2(SYS_dup2, fd, 20));
    satir("dup2 aynı", sc2(SYS_dup2, fd, fd));
    satir("dup2 kötü", sc2(SYS_dup2, 99, 21));
    satir("dup3 aynı", sc3(SYS_dup3, fd, fd, 0));
    satir("dup3 kötü bayrak", sc3(SYS_dup3, fd, 21, 1));
    satir("dup3 cloexec", sc3(SYS_dup3, fd, 21, O_CLOEXEC));
    satir("F_GETFD cloexec", sc3(SYS_fcntl, 21, 1, 0));
    satir("F_GETFD dup2", sc3(SYS_fcntl, 20, 1, 0));
    satir("F_SETFD", sc3(SYS_fcntl, 20, 2, 1));
    satir("F_GETFD ayarlı", sc3(SYS_fcntl, 20, 1, 0));
    satir("F_GETFL", sc3(SYS_fcntl, fd, 3, 0));
    satir("F_SETFL", sc3(SYS_fcntl, fd, 4, O_APPEND | 04000));
    satir("F_GETFL append", sc3(SYS_fcntl, fd, 3, 0));
    satir("F_DUPFD", sc3(SYS_fcntl, fd, 0, 25));
    satir("F_DUPFD_CLOEXEC", sc3(SYS_fcntl, fd, 1030, 25));
    satir("F_GETFD dupfd_cloexec", sc3(SYS_fcntl, 26, 1, 0));
    satir("fcntl kötü fd", sc3(SYS_fcntl, 99, 1, 0));
    sc1(SYS_close, 20);
    sc1(SYS_close, 21);
    sc1(SYS_close, 25);
    sc1(SYS_close, 26);
    r = sc3(SYS_open, D "/b", O_RDONLY, 0);
    satir("F_GETFL salt okunur", sc3(SYS_fcntl, r, 3, 0));
    sc1(SYS_close, r);

    /* access */
    satir("access var", sc2(SYS_access, D "/a.txt", 4 | 2));
    satir("access yürüt", sc2(SYS_access, D "/a.txt", 1));
    satir("access yok", sc2(SYS_access, D "/yok", 0));
    satir("access geçersiz kip", sc2(SYS_access, D "/a.txt", 8));
    satir("faccessat", sc6(SYS_faccessat, dfd, (long)"c", 1, 0, 0, 0));

    /* getcwd / chdir / fchdir */
    {
        char cwd[256];
        char eski[256];
        long n0 = sc2(SYS_getcwd, eski, sizeof(eski));
        satir("getcwd", n0 == (long)uzunluk(eski) + 1);
        satir("getcwd küçük", sc2(SYS_getcwd, cwd, 1));
        satir("chdir", sc1(SYS_chdir, D));
        sc2(SYS_getcwd, cwd, sizeof(cwd));
        satir("getcwd yeni", sonu(cwd, "/" D));
        satir("stat göreli", sc2(SYS_stat, "a.txt", &st));
        satir("chdir dosya", sc1(SYS_chdir, "a.txt"));
        satir("chdir yok", sc1(SYS_chdir, "yok"));
        satir("chdir ..", sc1(SYS_chdir, ".."));
        sc2(SYS_getcwd, cwd, sizeof(cwd));
        satir("getcwd geri", esit(cwd, eski));
        satir("fchdir", sc1(SYS_fchdir, dfd));
        sc2(SYS_getcwd, cwd, sizeof(cwd));
        satir("getcwd fchdir", sonu(cwd, "/" D));
        satir("fchdir dosya", sc1(SYS_fchdir, fd));
        sc1(SYS_chdir, eski);
    }

    /* readlink */
    r = sc3(SYS_readlink, "/proc/self/exe", tampon, sizeof(tampon) - 1);
    tampon[r > 0 ? r : 0] = 0;
    satir("readlink exe", r > 0 && sonu(tampon, "/sctest2"));
    satir("readlink dosya", sc3(SYS_readlink, D "/a.txt", tampon, 64));
    satir("readlink yok", sc3(SYS_readlink, D "/yok", tampon, 64));

    /* rename / unlink / rmdir */
    satir("rename", sc2(SYS_rename, D "/a.txt", D "/d.txt"));
    satir("rename eski yok", sc2(SYS_stat, D "/a.txt", &st));
    satir("rename yeni var", sc2(SYS_stat, D "/d.txt", &st));
    satir("rename üzerine", sc2(SYS_rename, D "/b", D "/d.txt"));
    satir("rename dosya dizine", sc2(SYS_rename, D "/d.txt", D "/c"));
    satir("renameat", sc6(SYS_renameat, dfd, (long)"d.txt", AT_FDCWD, (long)D "/e.txt", 0, 0));
    satir("rename yok", sc2(SYS_rename, D "/yok", D "/z"));
    satir("rmdir dolu", sc1(SYS_rmdir, D));
    satir("rmdir dosya", sc1(SYS_rmdir, D "/e.txt"));
    satir("unlink dizin", sc1(SYS_unlink, D "/c"));
    satir("unlink", sc1(SYS_unlink, D "/e.txt"));
    satir("unlink yok", sc1(SYS_unlink, D "/e.txt"));
    satir("unlinkat dizin bayraksız", sc3(SYS_unlinkat, dfd, "c", 0));
    satir("unlinkat removedir", sc3(SYS_unlinkat, dfd, "c", AT_REMOVEDIR));
    sc1(SYS_close, fd);
    sc1(SYS_close, dfd);
    satir("rmdir", sc1(SYS_rmdir, D));
    satir("rmdir yok", sc1(SYS_rmdir, D));

    /* uname */
    {
        struct utsname u;
        satir("uname", sc1(SYS_uname, &u));
        yaz("uname sysname: ");
        yaz(u.sysname);
        yaz("\nuname machine: ");
        yaz(u.machine);
        yaz("\n");
        satir("uname kötü adres", sc1(SYS_uname, 8));
    }

    /* saatler */
    satir("clock_gettime realtime", sc2(SYS_clock_gettime, 0, &ts) == 0 && ts.tv_sec > 1700000000L && ts.tv_nsec < 1000000000L);
    satir("time", sc1(SYS_time, 0) - ts.tv_sec <= 1);
    {
        struct timeval tv;
        long tt = 0;
        satir("gettimeofday", sc2(SYS_gettimeofday, &tv, 0) == 0 && tv.tv_usec < 1000000 && tv.tv_sec - ts.tv_sec <= 1);
        satir("time tloc", sc1(SYS_time, &tt) == tt);
    }
    satir("clock_getres", sc2(SYS_clock_getres, 1, &ts) == 0 && ts.tv_sec == 0 && ts.tv_nsec > 0);
    satir("clock_getres null", sc2(SYS_clock_getres, 1, 0));
    satir("clock_gettime geçersiz", sc2(SYS_clock_gettime, 99, &ts));
    satir("clock_gettime kötü adres", sc2(SYS_clock_gettime, 1, 8));
    t0 = monotonik_ms();
    ts.tv_sec = 0;
    ts.tv_nsec = 30 * 1000 * 1000;
    satir("clock_nanosleep", sc6(SYS_clock_nanosleep, 1, 0, (long)&ts, 0, 0, 0));
    satir("clock_nanosleep süre", monotonik_ms() - t0 >= 29);
    sc2(SYS_clock_gettime, 1, &ts2);
    ts2.tv_nsec += 20 * 1000 * 1000;
    if (ts2.tv_nsec >= 1000000000L) {
        ts2.tv_sec++;
        ts2.tv_nsec -= 1000000000L;
    }
    satir("clock_nanosleep mutlak", sc6(SYS_clock_nanosleep, 1, 1, (long)&ts2, 0, 0, 0));
    sc2(SYS_clock_gettime, 1, &ts);
    satir("clock_nanosleep mutlak geçti", ts.tv_sec > ts2.tv_sec || (ts.tv_sec == ts2.tv_sec && ts.tv_nsec >= ts2.tv_nsec - 1000000));
    satir("clock_nanosleep thread", sc6(SYS_clock_nanosleep, 3, 0, (long)&ts, 0, 0, 0));

    /* getrandom */
    satir("getrandom", sc3(SYS_getrandom, tampon, 16, 0));
    satir("getrandom bayrak", sc3(SYS_getrandom, tampon, 16, 0x100));
    satir("getrandom kötü adres", sc3(SYS_getrandom, 8, 16, 0));

    /* arch_prctl: FS tabanı */
    satir("arch_prctl set_fs", sc2(SYS_arch_prctl, 0x1002, tls_blok));
    satir("fs:0", fs_oku() == tls_blok[0]);
    {
        long taban = 0;
        satir("arch_prctl get_fs", sc2(SYS_arch_prctl, 0x1003, &taban));
        satir("get_fs eşit", taban == (long)tls_blok);
    }
    satir("arch_prctl çekirdek adresi", sc2(SYS_arch_prctl, 0x1002, 0xffff800000000000UL));
    satir("arch_prctl geçersiz", sc2(SYS_arch_prctl, 0x9999, 0));
    satir("arch_prctl get kötü adres", sc2(SYS_arch_prctl, 0x1003, 8));

    /* mprotect / madvise */
    {
        char* p = (char*)sys_mmap(0, 2 * 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        p[0] = 9;
        p[4096] = 8;
        satir("mprotect", sc3(SYS_mprotect, p, 4096, PROT_READ));
        satir("mprotect okunur", p[0] == 9);
        satir("mprotect hizasız", sc3(SYS_mprotect, p + 1, 4096, PROT_READ));
        satir("mprotect geçersiz koruma", sc3(SYS_mprotect, p, 4096, 0x100));
        satir("mprotect sıfır", sc3(SYS_mprotect, p, 0, PROT_READ));
        satir("madvise dontneed", sc3(SYS_madvise, p + 4096, 4096, 4));
        satir("madvise sıfırlandı", p[4096] == 0);
        sys_munmap(p, 2 * 4096);
        satir("mprotect eşlenmemiş", sc3(SYS_mprotect, p, 4096, PROT_READ));
        satir("madvise eşlenmemiş", sc3(SYS_madvise, p, 4096, 0));
    }

    /* sinyaller */
    {
        long yeni[4] = {1, 0x04000000, 0x1234, 1L << 8};
        long eski[4] = {5, 5, 5, 5};
        unsigned long m = 1UL << 11, om = 99;
        struct {
            void* sp;
            int bayrak;
            size_t boy;
        } ss;
        satir("rt_sigaction", sc6(SYS_rt_sigaction, 10, (long)yeni, 0, 8, 0, 0));
        satir("rt_sigaction eski", sc6(SYS_rt_sigaction, 10, 0, (long)eski, 8, 0, 0));
        satir("rt_sigaction geri", eski[0] == 1 && eski[1] == 0x04000000 && eski[2] == 0x1234 && eski[3] == 0);
        satir("rt_sigaction varsayılan", (sc6(SYS_rt_sigaction, 12, 0, (long)eski, 8, 0, 0), eski[0]));
        satir("rt_sigaction sigkill", sc6(SYS_rt_sigaction, 9, (long)yeni, 0, 8, 0, 0));
        satir("rt_sigaction boy", sc6(SYS_rt_sigaction, 10, 0, (long)eski, 4, 0, 0));
        satir("rt_sigaction 65", sc6(SYS_rt_sigaction, 65, 0, (long)eski, 8, 0, 0));
        satir("kill 0", sc2(SYS_kill, sc0(SYS_getpid), 0));
        satir("kill yok sayılan", sc2(SYS_kill, sc0(SYS_getpid), 10));
        satir("kill olmayan", sc2(SYS_kill, 2147483000L, 0));
        satir("rt_sigprocmask", sc6(SYS_rt_sigprocmask, 0, (long)&m, (long)&om, 8, 0, 0));
        satir("rt_sigprocmask eski", (long)om);
        m = ~0UL;
        sc6(SYS_rt_sigprocmask, 2, (long)&m, 0, 8, 0, 0);
        sc6(SYS_rt_sigprocmask, 0, 0, (long)&om, 8, 0, 0);
        satir("sigkill engellenemez", (om & (1UL << 8)) == 0 && (om & (1UL << 18)) == 0 && (om & (1UL << 11)) != 0);
        m = 0;
        sc6(SYS_rt_sigprocmask, 2, (long)&m, 0, 8, 0, 0);
        satir("rt_sigprocmask nasıl", sc6(SYS_rt_sigprocmask, 7, (long)&m, 0, 8, 0, 0));
        satir("sigaltstack", sc2(SYS_sigaltstack, 0, &ss));
        satir("sigaltstack bayrak", ss.bayrak);
    }

    /* kaynak sınırları, umask */
    {
        struct rlimit rl, rl2;
        satir("getrlimit", sc2(SYS_getrlimit, 7, &rl));
        rl2.rlim_cur = 16;
        rl2.rlim_max = rl.rlim_max;
        satir("prlimit64 set", sc6(SYS_prlimit64, 0, 7, (long)&rl2, 0, 0, 0));
        sc2(SYS_getrlimit, 7, &rl);
        satir("prlimit64 geri", (long)rl.rlim_cur);
        rl2.rlim_cur = rl2.rlim_max + 1;
        satir("prlimit64 cur>max", sc6(SYS_prlimit64, 0, 7, (long)&rl2, 0, 0, 0));
        satir("prlimit64 geçersiz", sc6(SYS_prlimit64, 0, 99, 0, (long)&rl, 0, 0));
        satir("prlimit64 başka süreç", sc6(SYS_prlimit64, 2147483000L, 7, 0, (long)&rl, 0, 0));
        sc1(SYS_umask, 077);
        satir("umask", sc1(SYS_umask, 022));
    }

    /* futex */
    {
        int f = 5;
        satir("futex wait farklı", sc6(SYS_futex, (long)&f, 128, 6, 0, 0, 0));
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        satir("futex wait zaman aşımı", sc6(SYS_futex, (long)&f, 128, 5, (long)&ts, 0, 0));
        satir("futex wake", sc6(SYS_futex, (long)&f, 129, 1, 0, 0, 0));
    }

    yaz("=== sctest2 bitti ===\n");
    sc1(SYS_exit_group, 0);
    return 1;
}
