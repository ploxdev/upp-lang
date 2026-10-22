/* dongu: sistem çağrısı yapmadan sonsuz döngü. Zaman dilimi kesmesiyle diğer
 * görevler (kabuk) çalışmaya devam etmeli; ön plandaysa Ctrl-C sonlandırır. */
#include "linux.h"

int main(int argc, char** argv, char** envp) {
    volatile unsigned long n = 0;
    (void)argc;
    (void)argv;
    (void)envp;
    yaz("dongu: başladı\n");
    for (;;) {
        n++;
    }
    return 0;
}
