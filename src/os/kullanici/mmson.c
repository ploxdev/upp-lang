/* mmson: MAP_SHARED eşlemesinin süreç sonunda (munmap ve msync olmadan) dosyaya
 * yazıldığını sınar. "yaz": dosyayı eşler, bir baytı değiştirir ve çıkar;
 * "oku": dosyayı yazar ve siler. Linux'taki çıktıyla aynı olmalıdır. */
#include "linux.h"

int main(int argc, char** argv, char** envp) {
    char b[16];
    long fd, n;
    (void)envp;
    if (argc > 1 && esit(argv[1], "yaz")) {
        fd = sys_open("mmson.bin", O_CREAT | O_TRUNC | O_RDWR, 0644);
        sys_write(fd, "abcdefgh", 8);
        char* p = (char*)sys_mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, (int)fd, 0);
        if (p == MAP_FAILED) {
            yaz("mmson: mmap başarısız\n");
            return 1;
        }
        p[7] = 'Z';
        sys_close(fd);
        return 0;
    }
    fd = sys_open("mmson.bin", O_RDONLY, 0);
    if (fd < 0) {
        yaz("mmson: dosya yok\n");
        return 1;
    }
    n = sys_read(fd, b, sizeof(b));
    sys_close(fd);
    yaz("mmson: \"");
    if (n > 0) sys_write(1, b, (size_t)n);
    yaz("\" ");
    sayi_yaz(n);
    yaz("\n");
    sc1(SYS_unlink, "mmson.bin");
    return 0;
}
