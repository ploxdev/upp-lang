/* ucat: stdin'i (ya da dosyaları) stdout'a kopyalar; read/write/open/close. */
#include "linux.h"

static int kopyala(int fd) {
    char b[512];
    long n;
    while ((n = sys_read(fd, b, sizeof(b))) > 0) {
        if (sys_write(1, b, (size_t)n) != n) return 1;
    }
    return n < 0;
}

int main(int argc, char** argv, char** envp) {
    int i, kod = 0;
    (void)envp;
    if (argc < 2) return kopyala(0);
    for (i = 1; i < argc; i++) {
        long fd = sys_open(argv[i], O_RDONLY, 0);
        if (fd < 0) {
            hyaz("ucat: ");
            hyaz(argv[i]);
            hyaz(": açılamadı\n");
            kod = 1;
            continue;
        }
        kod |= kopyala((int)fd);
        sys_close((int)fd);
    }
    return kod;
}
