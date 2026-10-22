/* cokme: boş göstericiye yazar. Çekirdek yalnızca süreci sonlandırmalı
 * (SIGSEGV, çıkış 139); kabuk ve sistem çalışmaya devam eder. */
#include "linux.h"

int main(int argc, char** argv, char** envp) {
    volatile int* p = (volatile int*)0;
    (void)argc;
    (void)argv;
    (void)envp;
    yaz("cokme: boş göstericiye yazılıyor...\n");
    *p = 42;
    yaz("cokme: buraya gelinmemeli\n");
    return 0;
}
