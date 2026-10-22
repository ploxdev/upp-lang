/* ekran: terminal boyutu (TIOCGWINSZ) ve terminal mi (TCGETS). */
#include "linux.h"

int main(int argc, char** argv, char** envp) {
    struct winsize ws;
    char termios[60];
    (void)argc;
    (void)argv;
    (void)envp;
    if (sys_ioctl(1, TIOCGWINSZ, &ws) == 0) {
        yaz("terminal: ");
        sayi_yaz(ws.ws_col);
        yaz(" sütun x ");
        sayi_yaz(ws.ws_row);
        yaz(" satır");
        if (ws.ws_xpixel) {
            yaz(" (");
            sayi_yaz(ws.ws_xpixel);
            yaz("x");
            sayi_yaz(ws.ws_ypixel);
            yaz(" piksel)");
        }
        yaz("\n");
    } else {
        yaz("terminal: yok (stdout terminal değil)\n");
    }
    yaz(sys_ioctl(0, TCGETS, termios) == 0 ? "stdin: terminal\n" : "stdin: terminal değil\n");
    return 0;
}
