/* Uppix kullanıcı programları için libc'siz, Linux x86_64 uyumlu ortam.
 * Yalnızca Linux sistem çağrıları kullanılır: aynı statik ikili hem gerçek
 * Linux'ta hem Uppix'te değiştirilmeden çalışır.
 *   gcc -static -nostdlib -fno-pie -no-pie -ffreestanding -O2 program.c */
#ifndef UPPIX_LINUX_H
#define UPPIX_LINUX_H

typedef unsigned long size_t;
typedef long ssize_t;

#define SYS_read 0
#define SYS_write 1
#define SYS_open 2
#define SYS_close 3
#define SYS_stat 4
#define SYS_fstat 5
#define SYS_lseek 8
#define SYS_mmap 9
#define SYS_munmap 11
#define SYS_brk 12
#define SYS_ioctl 16
#define SYS_nanosleep 35
#define SYS_exit 60
#define SYS_rt_sigaction 13
#define SYS_rt_sigprocmask 14
#define SYS_pread64 17
#define SYS_pwrite64 18
#define SYS_readv 19
#define SYS_writev 20
#define SYS_access 21
#define SYS_madvise 28
#define SYS_dup 32
#define SYS_dup2 33
#define SYS_getpid 39
#define SYS_kill 62
#define SYS_uname 63
#define SYS_fcntl 72
#define SYS_getcwd 79
#define SYS_chdir 80
#define SYS_fchdir 81
#define SYS_rename 82
#define SYS_mkdir 83
#define SYS_rmdir 84
#define SYS_unlink 87
#define SYS_readlink 89
#define SYS_umask 95
#define SYS_gettimeofday 96
#define SYS_getrlimit 97
#define SYS_mprotect 10
#define SYS_sigaltstack 131
#define SYS_arch_prctl 158
#define SYS_time 201
#define SYS_futex 202
#define SYS_getdents64 217
#define SYS_clock_gettime 228
#define SYS_clock_getres 229
#define SYS_clock_nanosleep 230
#define SYS_exit_group 231
#define SYS_openat 257
#define SYS_mkdirat 258
#define SYS_newfstatat 262
#define SYS_unlinkat 263
#define SYS_renameat 264
#define SYS_faccessat 269
#define SYS_dup3 292
#define SYS_prlimit64 302
#define SYS_getrandom 318

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define O_CREAT 0100
#define O_EXCL 0200
#define O_TRUNC 01000
#define O_APPEND 02000
#define O_DIRECTORY 0200000

#define O_CLOEXEC 02000000
#define AT_FDCWD (-100)
#define AT_REMOVEDIR 0x200
#define AT_EMPTY_PATH 0x1000

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define PROT_READ 1
#define PROT_WRITE 2
#define MAP_SHARED 1
#define MAP_PRIVATE 2
#define MAP_ANONYMOUS 0x20
#define MAP_FAILED ((void*)-1)

#define TCGETS 0x5401
#define TIOCGWINSZ 0x5413
#define FBIOGET_VSCREENINFO 0x4600
#define FBIOGET_FSCREENINFO 0x4602

#define S_IFMT 0170000
#define S_IFDIR 0040000
#define S_IFREG 0100000
#define S_IFCHR 0020000

struct stat {
    unsigned long st_dev, st_ino, st_nlink;
    unsigned int st_mode, st_uid, st_gid, __pad0;
    unsigned long st_rdev;
    long st_size, st_blksize, st_blocks;
    unsigned long st_atime, st_atime_nsec, st_mtime, st_mtime_nsec, st_ctime, st_ctime_nsec;
    long __unused[3];
};

struct timespec {
    long tv_sec, tv_nsec;
};

struct iovec {
    void* iov_base;
    size_t iov_len;
};

struct utsname {
    char sysname[65], nodename[65], release[65], version[65], machine[65], domainname[65];
};

struct rlimit {
    unsigned long rlim_cur, rlim_max;
};

struct timeval {
    long tv_sec, tv_usec;
};

struct winsize {
    unsigned short ws_row, ws_col, ws_xpixel, ws_ypixel;
};

struct fb_bitfield {
    unsigned int offset, length, msb_right;
};

struct fb_var_screeninfo {
    unsigned int xres, yres, xres_virtual, yres_virtual, xoffset, yoffset, bits_per_pixel, grayscale;
    struct fb_bitfield red, green, blue, transp;
    unsigned int nonstd, activate, height, width, accel_flags, pixclock, left_margin, right_margin;
    unsigned int upper_margin, lower_margin, hsync_len, vsync_len, sync, vmode, rotate, colorspace;
    unsigned int reserved[4];
};

struct fb_fix_screeninfo {
    char id[16];
    unsigned long smem_start;
    unsigned int smem_len, type, type_aux, visual;
    unsigned short xpanstep, ypanstep, ywrapstep;
    unsigned int line_length;
    unsigned long mmio_start;
    unsigned int mmio_len, accel;
    unsigned short capabilities, reserved[2];
};

static inline long sc6(long n, long a, long b, long c, long d, long e, long f) {
    long r;
    register long r10 __asm__("r10") = d;
    register long r8 __asm__("r8") = e;
    register long r9 __asm__("r9") = f;
    __asm__ volatile("syscall"
                     : "=a"(r)
                     : "a"(n), "D"(a), "S"(b), "d"(c), "r"(r10), "r"(r8), "r"(r9)
                     : "rcx", "r11", "memory");
    return r;
}

#define sc0(n) sc6((n), 0, 0, 0, 0, 0, 0)
#define sc1(n, a) sc6((n), (long)(a), 0, 0, 0, 0, 0)
#define sc2(n, a, b) sc6((n), (long)(a), (long)(b), 0, 0, 0, 0)
#define sc3(n, a, b, c) sc6((n), (long)(a), (long)(b), (long)(c), 0, 0, 0)

static inline long sys_read(int fd, void* b, size_t n) { return sc3(SYS_read, fd, b, n); }
static inline long sys_write(int fd, const void* b, size_t n) { return sc3(SYS_write, fd, b, n); }
static inline long sys_open(const char* p, int f, int m) { return sc3(SYS_open, p, f, m); }
static inline long sys_close(int fd) { return sc1(SYS_close, fd); }
static inline long sys_stat(const char* p, struct stat* s) { return sc2(SYS_stat, p, s); }
static inline long sys_fstat(int fd, struct stat* s) { return sc2(SYS_fstat, fd, s); }
static inline long sys_lseek(int fd, long o, int w) { return sc3(SYS_lseek, fd, o, w); }
static inline void* sys_mmap(void* a, size_t n, int p, int f, int fd, long o) {
    return (void*)sc6(SYS_mmap, (long)a, (long)n, p, f, fd, o);
}
static inline long sys_munmap(void* a, size_t n) { return sc2(SYS_munmap, a, n); }
static inline long sys_brk(void* a) { return sc1(SYS_brk, a); }
static inline long sys_ioctl(int fd, unsigned long r, void* a) { return sc3(SYS_ioctl, fd, r, a); }
static inline long sys_nanosleep(const struct timespec* t, struct timespec* k) { return sc2(SYS_nanosleep, t, k); }
static inline void sys_exit(int k) {
    for (;;) sc1(SYS_exit, k);
}

/* gcc yapı ilklendirmesi için örtük çağırabilir */
void* memset(void* d, int c, size_t n) {
    unsigned char* p = (unsigned char*)d;
    while (n--) *p++ = (unsigned char)c;
    return d;
}

void* memcpy(void* d, const void* s, size_t n) {
    unsigned char* p = (unsigned char*)d;
    const unsigned char* q = (const unsigned char*)s;
    while (n--) *p++ = *q++;
    return d;
}

__attribute__((unused)) static size_t uzunluk(const char* s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

__attribute__((unused)) static int esit(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return *a == *b;
}

__attribute__((unused)) static void yaz(const char* s) { sys_write(1, s, uzunluk(s)); }
__attribute__((unused)) static void hyaz(const char* s) { sys_write(2, s, uzunluk(s)); }

__attribute__((unused)) static void sayi_yaz(long v) {
    char b[24];
    int i = 23;
    int eksi = v < 0;
    unsigned long u = eksi ? (unsigned long)(-v) : (unsigned long)v;
    b[i] = 0;
    do {
        b[--i] = (char)('0' + u % 10);
        u /= 10;
    } while (u);
    if (eksi) b[--i] = '-';
    yaz(b + i);
}

/* "ad: değer\n" */
__attribute__((unused)) static void satir(const char* ad, long v) {
    yaz(ad);
    yaz(": ");
    sayi_yaz(v);
    yaz("\n");
}

int main(int argc, char** argv, char** envp);

__attribute__((used)) static int _giris(long* sp) {
    int argc = (int)sp[0];
    char** argv = (char**)(sp + 1);
    return main(argc, argv, argv + argc + 1);
}

__asm__(".global _start\n"
        "_start:\n"
        "    xor %rbp, %rbp\n"
        "    mov %rsp, %rdi\n"
        "    and $-16, %rsp\n"
        "    call _giris\n"
        "    mov %eax, %edi\n"
        "    mov $60, %eax\n"
        "    syscall\n"
        "    hlt\n");

#endif
