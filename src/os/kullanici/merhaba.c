/* merhaba: ring 3'ten ilk program. Argümanları ve ortamı yazar. */
#include "linux.h"

int main(int argc, char** argv, char** envp) {
    int i, n = 0;
    yaz("Merhaba, ring 3! Linux sistem çağrılarıyla konuşuyorum.\n");
    satir("argc", argc);
    for (i = 0; i < argc; i++) {
        yaz("argv[");
        sayi_yaz(i);
        yaz("] = ");
        yaz(argv[i]);
        yaz("\n");
    }
    while (envp[n]) n++;
    satir("ortam değişkeni", n);
    for (i = 1; i < argc; i++) {
        if (esit(argv[i], "--cik")) return 7;
    }
    return 0;
}
