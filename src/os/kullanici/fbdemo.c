/* fbdemo: /dev/fb0'ı Linux'taki gibi açar, ioctl ile ekranı öğrenir, mmap ile
 * çerçeve tamponunu adres alanına alır ve doğrudan piksel çizer. */
#include "linux.h"

static unsigned int renk(const struct fb_var_screeninfo* v, unsigned r, unsigned g, unsigned b) {
    return ((r >> (8 - v->red.length)) << v->red.offset) | ((g >> (8 - v->green.length)) << v->green.offset) |
           ((b >> (8 - v->blue.length)) << v->blue.offset);
}

int main(int argc, char** argv, char** envp) {
    struct fb_var_screeninfo v;
    struct fb_fix_screeninfo f;
    struct timespec ts;
    unsigned char* fb;
    unsigned x, y, gen, yuk, x0, y0;
    long fd = sys_open("/dev/fb0", O_RDWR, 0);
    (void)argc;
    (void)argv;
    (void)envp;
    if (fd < 0) {
        hyaz("fbdemo: /dev/fb0 açılamadı\n");
        return 1;
    }
    if (sys_ioctl((int)fd, FBIOGET_VSCREENINFO, &v) < 0 || sys_ioctl((int)fd, FBIOGET_FSCREENINFO, &f) < 0) {
        hyaz("fbdemo: ioctl başarısız\n");
        return 1;
    }
    yaz("fb: ");
    sayi_yaz(v.xres);
    yaz("x");
    sayi_yaz(v.yres);
    yaz(", ");
    sayi_yaz(v.bits_per_pixel);
    yaz(" bpp, satır ");
    sayi_yaz(f.line_length);
    yaz(" bayt, ");
    yaz(f.id);
    yaz("\n");
    if (v.bits_per_pixel != 32) {
        hyaz("fbdemo: yalnızca 32 bpp destekleniyor\n");
        return 1;
    }
    fb = (unsigned char*)sys_mmap(0, f.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, (int)fd, 0);
    if (fb == MAP_FAILED) {
        hyaz("fbdemo: mmap başarısız\n");
        return 1;
    }
    /* sağ üst köşede renk geçişli bir kare */
    gen = v.xres / 3;
    yuk = v.yres / 3;
    x0 = v.xres - gen - 16;
    y0 = 16;
    for (y = 0; y < yuk; y++) {
        unsigned int* satir_p = (unsigned int*)(fb + (y0 + y) * f.line_length) + x0;
        for (x = 0; x < gen; x++) {
            unsigned r = x * 255 / gen, g = y * 255 / yuk, b = 255 - r;
            satir_p[x] = renk(&v, r, g, b);
        }
    }
    yaz("fbdemo: çizildi (sağ üst köşe)\n");
    ts.tv_sec = 1;
    ts.tv_nsec = 0;
    sys_nanosleep(&ts, 0);
    sys_munmap(fb, f.smem_len);
    sys_close((int)fd);
    return 0;
}
