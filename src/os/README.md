# Uppix

Uppix, tamamen **u++** ile yazılmış, Unix benzeri küçük bir işletim sistemidir. x86_64 makinede [Limine](onyukleyici/limine) ile diskten **64-bit long mode**'da açılır (BIOS ya da UEFI) ve libc kullanmaz. **Kök dosya sistemi ext4 bir diskte durur** ve yapılan değişiklikler kalıcıdır. USB bellek gibi **FAT32** ve **exFAT** diskleri tanır, okur ve yazar. Kabuk, dosya sistemi sürücüleri ve 74 komutun (bir yılan oyunu dahil) hepsi u++ kaynağıdır. Altta yalnızca u++'ın serbest (freestanding) çalışma zamanı vardır: kesmeler, zamanlayıcı, klavye/seri girdi, ATA disk erişimi, bellek ayırıcı ve görev geçişi.

Uppix **kullanıcı programlarını ring 3'te** çalıştırır. Sistem çağrıları **Linux x86_64 ile birebir uyumludur**: `gcc -static -nostdlib` ile Linux için derlenmiş bir ELF, Uppix'te değiştirilmeden çalışır ve aynı çıktıyı verir.

```text
root@uppix:~# lsblk
AD      BOYUT TÜR   FSTÜR  ETİKET           BAĞLAMA
sda      192M disk
├─sda1    64M part  vfat   UPPIXBOOT        /boot
└─sda2   127M part  ext4   uppix-kok        /
sdb       64M disk
└─sdb1    63M part  vfat   UPPIXUSB         /mnt/sdb1
sdc       32M disk  exfat                   /mnt/sdc
root@uppix:~# cat "/mnt/sdb1/Türkçe uzun dosya adı.txt"
Uzun ve Türkçe bir dosya adı: şğüöçı ŞĞÜÖÇİ
```

## Çalıştırma

Gerekenler: `gcc`, `ld` ve `qemu-system-x86_64` (Windows'ta WSL içinde de olur). Disk imajı için `mkfs.ext4` (e2fsprogs), `sfdisk` (fdisk), `mkfs.fat` ve `mtools` gerekir; exFAT örnek diski için `mkfs.exfat` (`sudo apt install e2fsprogs dosfstools mtools exfatprogs fdisk`).

```bash
uppc calistir src/os
```

Bu komut çekirdeği derler, [`disk_yap.sh`](disk_yap.sh) ile önyüklenebilir `uppix.img` imajını hazırlar (`upp.proje` içindeki `onyukleyici = limine` ve `disk_hazirla`) ve QEMU'yu üç diskle ve seri konsolla başlatır. QEMU, Limine'i diskin MBR'sinden yükler; Limine de çekirdeği FAT32 bölümünden okuyup 64-bit kipte başlatır. Komutları terminale yazarsınız. İmajlar `derleme/` altında kalır: sonraki derlemelerde yalnızca `/boot/uppix.elf` güncellenir, kökte yaptığınız değişiklikler yerinde kalır. Sıfırdan başlamak için `derleme/*.img` dosyalarını silin.

| İmaj | İçerik |
|---|---|
| `uppix.img` | 192 MiB, MBR. **1. bölüm** 64 MiB FAT32 `UPPIXBOOT` (ESP): `/limine/limine-bios.sys`, `/limine/limine.conf`, `/EFI/BOOT/BOOTX64.EFI`, `/uppix.elf`; Uppix'te `/boot` olarak bağlanır. **2. bölüm** 127 MiB ext4 `uppix-kok`: kök (`/`), içeriği [`kok/`](kok) ağacından gelir |
| `usb-fat32.img` | 64 MiB, MBR bölüm tablosu ve 1. bölümde FAT32 (`UPPIXUSB`), örnek dosyalar uzun Türkçe adlarla |
| `usb-exfat.img` | 32 MiB exFAT (bölümsüz) |

Klavye ve ekranla bir pencerede açmak için (Limine menüsü, ardından çerçeve tamponu konsolu):

```bash
uppc derle src/os
qemu-system-x86_64 -m 256 -serial stdio -drive file=src/os/derleme/uppix.img,format=raw
```

UEFI ile açmak için QEMU'ya OVMF verin, örneğin `-bios /usr/share/qemu/OVMF.fd`. Aynı imaj gerçek bir USB belleğe de yazılabilir (`dd if=uppix.img of=/dev/sdX`).

Eski Multiboot yolu da durur: `--onyukleyici multiboot` (varsayılan) ile çekirdek 32-bit `uppix32.elf` olarak üretilir ve QEMU `-kernel` ile açılır; `sh disk_yap.sh` (çekirdek vermeden) bu yol için ayrı `uppix-kok.img` ext4 imajını üretir. Disk verilmezse Uppix RAM'deki bir kök dosya sistemiyle (rootfs) açılır. `poweroff` dosya sistemlerini boşaltıp makineyi kapatır, `reboot` yeniden başlatır.

## Neler var

- **Dosya sistemleri:**
  - **ext4:** okuma ve yazma. Extent ağaçları, eski ext2/3 blok haritaları, htree dizinleri (okuma) ve `metadata_csum` desteklenir: inode, dizin bloğu, bitmap, grup tanımlayıcı ve süperblok sağlamaları CRC32C ile güncellenir. Desteklenmeyen bir özellik varsa disk salt okunur bağlanır.
  - **FAT32:** okuma ve yazma. VFAT uzun dosya adları (UTF-16) okunur ve yazılır, `AD~1.UZN` kısa adları üretilir, tüm FAT kopyaları birlikte güncellenir.
  - **exFAT:** okuma ve yazma. Ayırma bitmap'i, FAT zinciri ve bitişik (NoFatChain) kümeler, girdi kümesi sağlaması, büyük harf tablosuna göre ad karması ve birim etiketi desteklenir.
  - **Bellekte:** `/proc` (`cpuinfo`, `meminfo`, `mounts`, `partitions`, `uptime`, `version`, `dmesg`), `/dev` (`null`, `console`, `tty`, `random`, `sda`…), `/tmp` ve `/mnt` (tmpfs), `/bin` (yerleşik komutlar).
- **Diskler:** ATA PIO ile en çok 4 disk (`sda`…`sdd`); MBR ve GPT bölüm tabloları (`sdb1`…). Açılışta `uppix-kok` etiketli (yoksa ilk) ext4 kök olarak bağlanır, `UPPIXBOOT` etiketli FAT32 bölümü `/boot`'a, diğer dosya sistemleri `/mnt/<aygıt>` altına bağlanır.
- **Kabuk (`uppsh`):** boru (`|`), yönlendirme (`>`, `>>`, `<`, `2>`), `;`, `&&`, `||` ve arka plan işleri (`&`, `jobs`, `kill %1`). Ayrıca `$AD`, `${AD}`, `$?`, `$$`, `$1…$9`, `$#`, `$@`, `~`, joker (`*`, `?`), tek/çift tırnak, takma adlar, betikler (`sh dosya`, `source`, `PATH` üzerinden çalıştırılabilir dosyalar) ve `/etc/profile`.
- **Satır düzenleyici:** ↑/↓ geçmiş, ←/→, Home/End, Delete, Tab ile komut ve dosya adı tamamlama, Ctrl-A/E/U/K/W/L/C/D.
- **Görevler:** işbirlikçi çoklu görev (en çok 32 görev, her biri 64 KiB yığınla). Görevler `sleep` sırasında ve girdi beklenirken sıra verir. `ps`, `kill` ve `jobs` ile yönetilirler.
- **Önyükleme:** [Limine](onyukleyici/limine) v11.4.1 depoya gömülüdür (BIOS aşamaları, `BOOTX64.EFI` ve `bios-install` aracının kaynağı). Çekirdek, `0xffffffff80000000`'da duran bir ELF64'tür. Limine protokolünün istekleriyle bellek haritasını, çerçeve tamponunu, HHDM'yi ve çekirdeğin fiziksel adresini alır. Sonra kendi sayfa tablolarını kurar: çekirdek kendi yüksek adresinde 4 KiB sayfalarla, tüm fiziksel bellek de HHDM'de 2 MiB sayfalarla eşlenir (çerçeve tamponu PAT ile write-combining). Alt yarı boş kalır, kullanıcı süreçlerinindir. GDT/TSS, `SYSCALL` ve NX/SMEP/SMAP kurulur. 8259 PIC kesmeleri doğrudan gelsin diye yerel APIC kapatılır.
- **Ekran:** Limine ekranı grafik kipte bırakır. Uppix bu çerçeve tamponuna kendi metin konsolunu çizer: [Terminus](../runtime/TERMINUS_LISANS.txt) 8x16 yazı tipi (Türkçe karakterler dahil), ANSI renkleri, kutu çizgileri, kaydırma ve alt çizgi imleç. Multiboot ile açılınca VGA metin ekranı kullanılır.
- **Donanım:** Limine/Multiboot bellek haritası, CMOS saati (dosya zamanları Unix saniyesidir), CPUID, ACPI ile kapatma, 8042 ile yeniden başlatma.

## Kullanıcı kipi (ring 3) ve Linux sistem çağrıları

```text
root@uppix:~# merhaba bir iki
Merhaba, ring 3! Linux sistem çağrılarıyla konuşuyorum.
argc: 3
...
root@uppix:~# cokme; echo $?
cokme: boş göstericiye yazılıyor...
Bölümleme hatası (SIGSEGV): komut 0x0000000000401161, adres 0x0000000000000000
139
root@uppix:~# fbdemo
fb: 1280x800, 32 bpp, satır 5120 bayt, uppixfb
fbdemo: çizildi (sağ üst köşe)
```

Kabuk, çalıştırılabilir bir ELF dosyasını (`/usr/bin`'de ya da yol vererek) ayrı bir adres alanında, ring 3'te çalıştırır. Program yalnızca sistem çağrılarıyla konuşur; çekirdeği bozamaz. Bir istisna (boş gösterici, geçersiz komut, sıfıra bölme) yalnızca o süreci Linux'taki sinyaller gibi sonlandırır: çıkış kodu 128 + sinyaldir, kabuk çalışmaya devam eder.

**Sistem çağrıları:** Numaralar, bayraklar, errno değerleri ve yapı düzenleri Linux x86_64'le aynıdır. Çağrı `syscall` komutuyla yapılır: rax numara; rdi, rsi, rdx, r10, r8, r9 argüman; hata dönüşü `-errno`.

| No | Çağrı | Uppix'te |
|---|---|---|
| 0 | `read` | Dosya (VFS), terminal (kanonik satır: yankı, geri silme, Ctrl-D ile EOF, Ctrl-C ile SIGINT) ya da kabuktan gelen girdi (boru, `<`) |
| 1 | `write` | Dosya (ortaya yazma ve sonun ötesine sıfır doldurma dahil), stdout (borulara ve `>`'a yakalanır), stderr (doğrudan terminal) |
| 2 | `open` | `O_RDONLY/WRONLY/RDWR`, `O_CREAT`, `O_EXCL`, `O_TRUNC`, `O_APPEND`, `O_DIRECTORY`; `/dev/null`, `/dev/tty`, `/dev/fb0` |
| 3 | `close` | |
| 4, 5 (ve 6) | `stat`, `fstat` (`lstat`) | 144 baytlık `struct stat`: tür ve izin bitleri, boyut, blok sayısı, zamanlar |
| 8 | `lseek` | `SEEK_SET/CUR/END`; terminalde ve boruda `ESPIPE` |
| 9 | `mmap` | Anonim, dosyadan (`MAP_PRIVATE` kopyası), `MAP_FIXED`; `/dev/fb0` `MAP_SHARED` ile çerçeve tamponunu doğrudan adres alanına eşler |
| 11 | `munmap` | Sayfaları ve çerçevelerini bırakır |
| 12 | `brk` | Yığını büyütür/küçültür; başarısızsa eski sınırı döndürür |
| 16 | `ioctl` | `TIOCGWINSZ` (terminal boyutu), `TCGETS` (`isatty`), `FBIOGET_VSCREENINFO`, `FBIOGET_FSCREENINFO` |
| 35 | `nanosleep` | Diğer görevlere sıra verir; ön planda Ctrl-C ile kesilir |
| 60 (ve 231) | `exit` (`exit_group`) | |
| 257 | `openat` | Yalnızca `AT_FDCWD` (libc'lerin `open()` karşılığı) |

Desteklenmeyen bir çağrı `-ENOSYS` döndürür ve `dmesg`'e bir kez yazılır.

**Uyumluluk testi:** [`kullanici/sctest.c`](kullanici/sctest.c) bu çağrıların hepsini ve hata yollarını sınar; çıktısında adres, inode ya da zaman yoktur. QEMU testi aynı ikiliyi Uppix'te ve gerçek Linux'ta çalıştırıp çıktıları `diff` ile karşılaştırır. 58 satırın hepsi aynıdır.

**Programlar** ([`kullanici/`](kullanici)): [`linux.h`](kullanici/linux.h) libc'siz ortamdır (`_start`, sistem çağrısı sarmalayıcıları, Linux yapıları). [`disk_yap.sh`](disk_yap.sh) her C dosyasını derleyip kökte `/usr/bin`'e koyar.

| Program | Ne yapar |
|---|---|
| `merhaba` | argv ve ortamı yazar; `--cik` ile 7 döner |
| `sctest` | 12 sistem çağrısının uyumluluk testi |
| `ekran` | Terminal boyutu (`TIOCGWINSZ`) ve stdin terminal mi (`TCGETS`) |
| `ucat` | stdin'i ya da dosyaları kopyalar (`cat` gibi) |
| `fbdemo` | `/dev/fb0`'ı açar, ioctl ile ekranı öğrenir, `mmap` ile eşleyip doğrudan piksel çizer |
| `cokme` | Boş göstericiye yazar: yalnızca süreç sonlanır (SIGSEGV, 139) |
| `dongu` | Sistem çağrısı yapmayan sonsuz döngü: zaman dilimiyle kesilir, Ctrl-C ya da `kill` ile durur |

Kendi programınızı eklemek için `kullanici/` altına bir `.c` dosyası koyun (`#include "linux.h"`, `int main(int argc, char** argv, char** envp)`). İmaj zaten varsa sonraki derleme `/usr/bin`'deki programları günceller. Aynı ikili Linux'ta da çalışır:

```bash
gcc -static -nostdlib -fno-pie -no-pie -ffreestanding -O2 -o merhaba src/os/kullanici/merhaba.c
```

**Mimari:**
- **Bellek düzeni:** Çekirdek tamamen üst yarıdadır: görüntü `0xffffffff80000000`'da, tüm fiziksel bellek HHDM'de. Alt yarı süreçlerindir ve düzen Linux'takiyle aynıdır:
  - ELF bölümleri `p_vaddr`'da (yorumlayıcısız `ET_DYN` için `0x555555554000` + `p_vaddr`);
  - `brk` yığını son bölümün ardından;
  - `mmap` bölgeleri `0x7f0000000000`'dan aşağı doğru;
  - 1 MiB yığın `0x7ffffffff000`'a kadar;
  - `0x10000` altı hiç eşlenmez.
- **Adres alanları:** Her sürecin kendi PML4'ü vardır; çekirdek yarısı paylaşılır ve kullanıcıya kapalıdır. Kod sayfaları salt okunurdur, veri ve yığın NX'tir. İşlemci destekliyorsa SMEP ve SMAP açılır. Kullanıcı sayfaları çekirdek yığınından ayrı bir fiziksel çerçeve havuzundan gelir.
- **İlk yığın:** SysV ABI'ye göre `argc`, `argv`, `envp` ve auxv (`AT_PHDR`, `AT_ENTRY`, `AT_PAGESZ`, `AT_RANDOM`, `AT_EXECFN`…) kurulur, yani libc başlangıç kodları da çalışabilir.
- **Giriş ve geçiş:** Sistem çağrıları `SYSCALL`/`SYSRET` ile gelir. GDT'de çekirdek ve kullanıcı segmentleri ve bir TSS vardır. Görev geçişi CR3'ü ve `TSS.rsp0`'ı değiştirir.
- **Zamanlama:** Çekirdek kodu işbirlikçidir; ring 3 kodu ise zamanlayıcıyla kesilir ve 10 ms'lik dilimi dolunca sırasını verir. Bu yüzden `dongu &` çalışırken kabuk yanıt vermeye devam eder.
- **Mekanizma ve politika:** Mekanizma C'dedir ([`upp_kullanici.c`](../runtime/upp_kullanici.c)): GDT/TSS, SYSCALL girişi, sayfa tabloları, kullanıcı belleğine izin denetimli kopyalama. Politika u++'tadır ([`surec.upp`](kaynak/surec.upp), [`sistem_cagrisi.upp`](kaynak/sistem_cagrisi.upp)): süreç tablosu, dosya tanımlayıcıları, ELF yükleme, mmap yerleşimi ve çağrıların anlamı.

Kullanıcı kipi yalnızca Limine ile açılışta vardır. Multiboot'ta çekirdek alt yarıyı (ilk 1 GiB) birebir kullandığından ELF çalıştırmak açık bir hatayla reddedilir.

### Komutlar

| Alan | Komutlar |
|---|---|
| Dosya | `ls` `cd` `pwd` `cat` `mkdir` `rmdir` `touch` `rm` `cp` `mv` `find` `tree` `stat` `du` `chmod` `basename` `dirname` |
| Disk | `lsblk` `blkid` `mount` (`-t`, `-o ro`) `umount` `df` `sync` |
| Metin | `echo` `wc` `head` `tail` `grep` `sort` `uniq` `rev` `cut` `tr` `seq` `tee` |
| Sistem | `ps` `jobs` `kill` `sleep` `uptime` `uname` `date` `free` `dmesg` `nproc` `uppfetch` `clear` `reboot` `poweroff` (`halt`, `shutdown`) |
| Kabuk | `sh` `source` (`.`) `exit` `env` `printenv` `export` `unset` `alias` `unalias` `which` `history` `time` `test` (`[`) `expr` `true` `false` `whoami` `id` `hostname` `help` |
| Oyun | `snake` |

```text
root@uppix:~# mount /dev/sdb1 /mnt/usb -o ro      # elle bağlama (nokta önce mkdir ile)
root@uppix:~# cp ~/not.txt /mnt/sdc/ && sync      # exFAT diske kopyala
root@uppix:~# umount /mnt/sdb1
```

### Yılan (`snake`)

`snake` tam ekran bir yılan oyunudur. Oklar ya da WASD ile yön verilir; `p` duraklatır, `q` ya da Ctrl-C çıkar, çarpınca `r` yeniden başlatır. Her yem 10 puandır ve skor arttıkça yılan hızlanır. Rekor `/root/.yilan_rekor` dosyasında tutulur, ext4 kökte olduğu için açılışlar arasında kalır. `snake -o` otopilottur: yılan yeme doğru giderken taşkın doldurmayla sıkışacağı yollardan kaçınır; `p` ile duraklatılır, `q` ile çıkılır, yönü yılan seçtiği için ok tuşları yok sayılır. Ekran ANSI dizileriyle (`\e[satır;sütunH`, renkler, `\e[?25l`) ve yalnızca değişen hücreler çizilerek güncellenir. Bu yüzden oyun çerçeve tamponu konsolunda, VGA metin ekranında ve seri terminalde aynı şekilde çalışır.

## Mimari

| Dosya | İçerik |
|---|---|
| `kaynak/uppix.upp` | Açılış: çekirdek günlüğü, disk tarama, kök bağlama, sanal dosya sistemleri, `/etc`, komut tablosu, oturum |
| `kaynak/vfs.upp` | VFS: düğüm önbelleği (`sinif Dugum`), bağlama tablosu, sürücü arayüzü (`sinif Surucu`), yol çözme, `/proc` |
| `kaynak/blok.upp` | Blok aygıtları: ATA diskleri, MBR/GPT bölümleri, dosya sistemi tanıma |
| `kaynak/ext4.upp` | ext4 sürücüsü |
| `kaynak/fat.upp` | FAT32 sürücüsü |
| `kaynak/exfat.upp` | exFAT sürücüsü |
| `kaynak/kabuk.upp` | `uppsh`: satır bölme, jetonlar ve açılımlar, boru ve yönlendirme, ve-veya listeleri, arka plan işleri, betikler |
| `kaynak/satir.upp` | Satır düzenleyici, geçmiş, Tab tamamlama, Ctrl-C ile kesme |
| `kaynak/cikti.upp` | Görev başına çıktı yakalama (boru ve yönlendirme için) |
| `kaynak/komut_*.upp` | Komutlar; her biri `fonk(liste[metin] a, metin girdi) -> sayi` (`komut_oyun.upp`: `snake`) |
| `kaynak/surec.upp` | Kullanıcı süreçleri: süreç tablosu, dosya tanımlayıcıları, ELF64 yükleyici, SysV ilk yığını, kabuktan çalıştırma |
| `kaynak/sistem_cagrisi.upp` | Linux x86_64 sistem çağrıları (read, write, open, close, stat, fstat, lseek, mmap, munmap, brk, ioctl, nanosleep, exit) |
| `kullanici/*.c` | Ring 3 programları (libc'siz statik Linux ELF); `linux.h` ortak başlık |
| `kaynak/yardimci.upp` | Sıralama, biçimleme, joker eşleme, takvim/Unix zamanı |

Her dosya sistemi sürücüsü fonksiyon değerlerinden oluşan bir `sinif Surucu`'dur: `bagla`, `yukle` (dizini önbelleğe al), `oku`, `yaz`, `olustur`, `sil`, `tasi`, `oznitelik`, `alan`. VFS hangi düğümün hangi bağlamaya ait olduğunu bilir ve doğru sürücüyü çağırır. Dizinler ilk erişimde yüklenir. Komutlar dosya sisteminden habersizdir; `cp /root/a.txt /mnt/sdc/` gibi diskler arası kopyalama aynı kodla çalışır.

İkili veri `upp.tampon.*` ile işlenir. Bunlar sınır denetimli bayt tamponlarıdır: küçük sonlu okuma/yazma, UTF-16, CRC32C. Disk erişimi `upp.cekirdek.disk_oku/disk_yaz` ile yapılır. Ayrıntılar için [API belgesine](../../docs/API_tr.md) bakın.

### Testler

| Test | Ne yapar |
|---|---|
| `tests/cekirdek/08_uppix.upp` | Disksiz açılış (RAM kök); 25 kabuk komutu |
| `tests/cekirdek/09_uppix_disk.upp` | İmajları sıfırdan üretir, üç dosya sistemine yazar, sonra Linux araçlarıyla denetler: `e2fsck -fn`, `debugfs`, `fsck.fat`, `mtype`, `fsck.exfat` |
| `tests/cekirdek/10_uppix_kalici.upp` | Aynı imajlarla ikinci açılış: yazılanlar yerinde mi |
| `tests/cekirdek/13_uppix_kullanici.upp` | Ring 3: argv/çıkış kodu, `sctest`, terminal ioctl'u, boru, SIGSEGV, zaman dilimiyle kesilen döngü ve `kill`; `sctest`'in çıktısı gerçek Linux'takiyle `diff` ile karşılaştırılır |
| `tests/cekirdek/12_uppix_snake.upp` | `snake -o -n 200` otopilotu beklemeden oynatır; rekor dosyası, boru/yönlendirmede reddetme ve seçenek hataları |
| `tests/cekirdek/11_uppix_limine.upp` | `uppix.img`'i üretir ve Limine ile 64-bit açar; `/boot` ve kökü yazar, sonra iki bölümü `e2fsck`, `fsck.fat` ve `mtype` ile denetler |

## Sınırlar ve sıradakiler

- ext4 yazımı günlük (journal) kullanmaz. Dosya en çok 4 extent'te tutulur, yani boş alan çok parçalıysa büyük dosya yazılamaz. htree ile dizinlenmiş büyük dizinlere ekleme yapılmaz. Grup bitmap'i başlatılmamış (`*_UNINIT`) gruplar ayırmada atlanır.
- FAT12/FAT16 ve 512 bayt dışı sektör boyutları desteklenmez. Önbellek yoktur: her işlem diske gider.
- Disk sürücüsü ATA PIO'dur; AHCI/NVMe ve virtio-blk sırada.
- Çekirdek içindeki kod işbirlikçidir; yalnızca ring 3 kodu kesilir. Yerleşik komutlar çekirdek içinde çalışır.
- Kullanıcı süreçleri tek iş parçacıklıdır. `fork`/`execve`, sinyal işleyicileri, dinamik bağlama (`PT_INTERP`) ve iş parçacığı yerel alanı (`arch_prctl`) henüz yok. Bu yüzden glibc/musl ile derlenmiş programlar sıradaki adımdır; şimdilik `-nostdlib` programlar çalışır. Yığın 1 MiB'dır ve sayfa hatasıyla büyümez.
- Çerçeve tamponu 4 GiB'nin altında ve RGB olmalı; değilse ekran çıktısı yalnızca seri porta gider. SMP yok: yalnızca açılış işlemcisi çalışır.
- `uppc` henüz Uppix içinde çalışmıyor, çünkü C çıktısını derleyecek bir derleyici gerekiyor.
