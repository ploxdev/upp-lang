/* sctest: Linux x86_64 sistem çağrılarını sınar. Çıktı ortamdan bağımsızdır
 * (adres, inode, zaman basılmaz): aynı ikilinin gerçek Linux'taki ve Uppix'teki
 * çıktısı satır satır aynı olmalıdır (tests/cekirdek/13_uppix_kullanici.upp). */
#include "linux.h"

#define DOSYA "sctest_dosya.txt"
#define METIN "merhaba dünya!\n" /* 16 bayt (ü iki bayt) */

static char tampon[64];

static void metin_satir(const char* ad, const char* b, long n) {
    yaz(ad);
    yaz(": \"");
    if (n > 0) sys_write(1, b, (size_t)n);
    yaz("\"\n");
}

int main(int argc, char** argv, char** envp) {
    struct stat st;
    struct winsize ws;
    struct timespec ts;
    long fd, r;
    (void)argc;
    (void)argv;
    (void)envp;
    yaz("=== sctest başla ===\n");

    /* open / write / lseek / read */
    satir("open olmayan", sys_open("yok/olmayan.txt", O_RDONLY, 0));
    fd = sys_open(DOSYA, O_CREAT | O_TRUNC | O_RDWR, 0644);
    satir("open creat (fd>=3)", fd >= 3);
    satir("write", sys_write(fd, METIN, 16));
    satir("lseek set", sys_lseek(fd, 0, SEEK_SET));
    r = sys_read(fd, tampon, 7);
    metin_satir("read 7", tampon, r);
    satir("lseek cur", sys_lseek(fd, 1, SEEK_CUR));
    r = sys_read(fd, tampon, sizeof(tampon));
    metin_satir("read kalan", tampon, r);
    satir("read eof", sys_read(fd, tampon, 4));
    satir("lseek end", sys_lseek(fd, -2, SEEK_END));
    satir("lseek negatif", sys_lseek(fd, -100, SEEK_SET));
    satir("lseek geçersiz", sys_lseek(fd, 0, 99));

    /* dosya sonunun ötesine yazma: aradaki baytlar sıfır */
    satir("lseek ötesi", sys_lseek(fd, 20, SEEK_SET));
    satir("write ötesi", sys_write(fd, "Z", 1));
    sys_lseek(fd, 16, SEEK_SET);
    r = sys_read(fd, tampon, 8);
    satir("boşluk okunan", r);
    satir("boşluk sıfır", tampon[0] == 0 && tampon[3] == 0 && tampon[4] == 'Z');

    /* fstat / stat */
    satir("fstat", sys_fstat(fd, &st));
    satir("fstat boyut", st.st_size);
    satir("fstat düzenli", (st.st_mode & S_IFMT) == S_IFREG);
    satir("fstat izin", st.st_mode & 0777);
    satir("stat", sys_stat(DOSYA, &st));
    satir("stat boyut", st.st_size);
    satir("stat dizin", sys_stat("/tmp", &st) == 0 && (st.st_mode & S_IFMT) == S_IFDIR);
    satir("stat olmayan", sys_stat("yok/olmayan.txt", &st));

    /* ioctl: dosyada terminal isteği */
    satir("ioctl dosya", sys_ioctl(fd, TIOCGWINSZ, &ws));
    satir("close", sys_close(fd));
    satir("close tekrar", sys_close(fd));
    satir("read kapalı", sys_read(fd, tampon, 1));

    /* erişim kipleri ve bayraklar */
    fd = sys_open(DOSYA, O_RDONLY, 0);
    satir("write salt okunur", sys_write(fd, "x", 1));
    sys_close(fd);
    fd = sys_open(DOSYA, O_WRONLY | O_APPEND, 0);
    satir("read salt yazılır", sys_read(fd, tampon, 1));
    satir("append", sys_write(fd, "++", 2));
    sys_close(fd);
    sys_stat(DOSYA, &st);
    satir("append boyut", st.st_size);
    satir("open excl", sys_open(DOSYA, O_CREAT | O_EXCL | O_WRONLY, 0644));
    satir("open directory dosyada", sys_open(DOSYA, O_RDONLY | O_DIRECTORY, 0));
    satir("open dizin yazma", sys_open("/tmp", O_WRONLY, 0));
    fd = sys_open(DOSYA, O_WRONLY | O_TRUNC, 0);
    sys_close(fd);
    sys_stat(DOSYA, &st);
    satir("trunc boyut", st.st_size);

    /* brk */
    {
        long b0 = sys_brk(0);
        long b1 = sys_brk((void*)(b0 + 65536));
        char* p = (char*)b0;
        int ok = 1, i;
        satir("brk büyüt", b1 == b0 + 65536);
        for (i = 0; i < 65536; i += 4096) p[i] = (char)i;
        for (i = 0; i < 65536; i += 4096) ok &= p[i] == (char)i;
        satir("brk bellek", ok);
        satir("brk küçült", sys_brk((void*)b0) == b0);
    }

    /* mmap / munmap */
    {
        char* p = (char*)sys_mmap(0, 3 * 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        int ok = p != MAP_FAILED && ((long)p & 4095) == 0;
        satir("mmap anonim", ok);
        if (ok) {
            satir("mmap sıfır", p[0] == 0 && p[3 * 4096 - 1] == 0);
            p[5000] = 42;
            satir("mmap yaz", p[5000] == 42);
            satir("munmap", sys_munmap(p, 3 * 4096));
        }
        satir("mmap bayraksız", (long)sys_mmap(0, 4096, PROT_READ, 0, -1, 0));
        satir("mmap sıfır uzunluk", (long)sys_mmap(0, 0, PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
        satir("munmap hizasız", sys_munmap((void*)4097, 4096));
        fd = sys_open(DOSYA, O_RDWR, 0);
        sys_write(fd, METIN, 16);
        p = (char*)sys_mmap(0, 4096, PROT_READ, MAP_PRIVATE, (int)fd, 0);
        satir("mmap dosya", p != MAP_FAILED);
        if (p != MAP_FAILED) {
            metin_satir("mmap içerik", p, 7);
            sys_munmap(p, 4096);
        }
        satir("mmap kötü fd", (long)sys_mmap(0, 4096, PROT_READ, MAP_PRIVATE, 99, 0));
        sys_close(fd);
    }

    /* nanosleep */
    ts.tv_sec = 0;
    ts.tv_nsec = 20 * 1000 * 1000;
    satir("nanosleep", sys_nanosleep(&ts, 0));
    ts.tv_nsec = 1000000000;
    satir("nanosleep geçersiz", sys_nanosleep(&ts, 0));
    satir("nanosleep kötü adres", sys_nanosleep((struct timespec*)8, 0));

    /* geçersiz adresler ve tanımlayıcılar */
    satir("write kötü adres", sys_write(1, (void*)8, 4));
    satir("open kötü adres", sys_open((const char*)8, O_RDONLY, 0));
    satir("write kötü fd", sys_write(99, "x", 1));
    satir("bilinmeyen çağrı", sc0(9999));

    yaz("=== sctest bitti ===\n");
    return 0;
}
