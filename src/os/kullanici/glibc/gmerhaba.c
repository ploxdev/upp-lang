/* gmerhaba: glibc ile statik derlenmiş sıradan bir Linux programı (cc -static).
 * Uppix'in Linux ABI'sinin sınaması: glibc açılışı (arch_prctl TLS, brk, mmap,
 * mprotect, prlimit64, getrandom...), stdio, malloc, dizin okuma, errno.
 * Çıktısı Linux'takiyle aynıdır; çıkış kodu 7. */
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>

static __thread int tls_sayac = 41;

int main(int argc, char** argv) {
    struct utsname u;
    struct timespec ts;
    char satir[100];
    char* p;
    FILE* f;
    DIR* d;
    int n = 0;
    (void)argv;
    printf("glibc merhaba! argc=%d\n", argc);
    tls_sayac++;
    printf("tls: %d\n", tls_sayac);
    uname(&u);
    printf("uname: %s %s\n", u.sysname, u.machine);
    printf("clock_gettime: %d\n", clock_gettime(CLOCK_MONOTONIC, &ts) == 0 && ts.tv_nsec < 1000000000L);
    p = malloc(1 << 20);
    memset(p, 'a', 1 << 20);
    printf("malloc 1 MiB: %c\n", p[12345]);
    free(p);
    f = fopen("/tmp/gmerhaba.txt", "w");
    fprintf(f, "satır %d\nsatır %d\n", 1, 2);
    fclose(f);
    f = fopen("/tmp/gmerhaba.txt", "r");
    while (fgets(satir, sizeof(satir), f)) printf("okundu: %s", satir);
    fclose(f);
    unlink("/tmp/gmerhaba.txt");
    d = opendir("/");
    while (readdir(d)) n++;
    closedir(d);
    printf("opendir /: %d\n", n > 3);
    printf("open: %s\n", open("/yok/dosya", O_RDONLY) < 0 ? strerror(errno) : "?");
    printf("printf: %.3f %x %s\n", 3.14159, 48879, "son");
    return 7;
}
