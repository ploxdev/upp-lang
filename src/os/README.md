# Uppix

Uppix, tamamen **u++** ile yazılmış, Unix benzeri küçük bir işletim sistemidir. x86_64 makinede [Limine](onyukleyici/limine) ile diskten **64-bit long mode**'da açılır (BIOS ya da UEFI) ve libc kullanmaz. **Kök dosya sistemi ext4 bir diskte durur** ve yapılan değişiklikler kalıcıdır. USB bellek gibi **FAT32** ve **exFAT** diskleri tanır, okur ve yazar. Kabuk, dosya sistemi sürücüleri ve 74 komutun (bir yılan oyunu dahil) hepsi u++ kaynağıdır. Altta yalnızca u++'ın serbest (freestanding) çalışma zamanı vardır: kesmeler, zamanlayıcı, klavye/seri girdi, ATA disk erişimi, bellek ayırıcı ve görev geçişi.

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
- **Önyükleme:** [Limine](onyukleyici/limine) v11.4.1 depoya gömülüdür (BIOS aşamaları, `BOOTX64.EFI` ve `bios-install` aracının kaynağı). Çekirdek, `0xffffffff80000000`'da duran bir ELF64'tür. Limine protokolünün istekleriyle bellek haritasını, çerçeve tamponunu, HHDM'yi ve çekirdeğin fiziksel adresini alır. Sonra kendi GDT'sini ve sayfa tablolarını kurar: ilk 4 GiB birebir 2 MiB sayfalarla, çekirdek de kendi yüksek adresinde 4 KiB sayfalarla eşlenir. 8259 PIC kesmeleri doğrudan gelsin diye yerel APIC'i kapatır.
- **Ekran:** Limine ekranı grafik kipte bırakır. Uppix bu çerçeve tamponuna kendi metin konsolunu çizer: [Terminus](../runtime/TERMINUS_LISANS.txt) 8x16 yazı tipi (Türkçe karakterler dahil), ANSI renkleri, kutu çizgileri, kaydırma ve alt çizgi imleç. Multiboot ile açılınca VGA metin ekranı kullanılır.
- **Donanım:** Limine/Multiboot bellek haritası, CMOS saati (dosya zamanları Unix saniyesidir), CPUID, ACPI ile kapatma, 8042 ile yeniden başlatma.

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
| `kaynak/yardimci.upp` | Sıralama, biçimleme, joker eşleme, takvim/Unix zamanı |

Her dosya sistemi sürücüsü fonksiyon değerlerinden oluşan bir `sinif Surucu`'dur: `bagla`, `yukle` (dizini önbelleğe al), `oku`, `yaz`, `olustur`, `sil`, `tasi`, `oznitelik`, `alan`. VFS hangi düğümün hangi bağlamaya ait olduğunu bilir ve doğru sürücüyü çağırır. Dizinler ilk erişimde yüklenir. Komutlar dosya sisteminden habersizdir; `cp /root/a.txt /mnt/sdc/` gibi diskler arası kopyalama aynı kodla çalışır.

İkili veri `upp.tampon.*` ile işlenir. Bunlar sınır denetimli bayt tamponlarıdır: küçük sonlu okuma/yazma, UTF-16, CRC32C. Disk erişimi `upp.cekirdek.disk_oku/disk_yaz` ile yapılır. Ayrıntılar için [API belgesine](../../docs/API_tr.md) bakın.

### Testler

| Test | Ne yapar |
|---|---|
| `tests/cekirdek/08_uppix.upp` | Disksiz açılış (RAM kök); 25 kabuk komutu |
| `tests/cekirdek/09_uppix_disk.upp` | İmajları sıfırdan üretir, üç dosya sistemine yazar, sonra Linux araçlarıyla denetler: `e2fsck -fn`, `debugfs`, `fsck.fat`, `mtype`, `fsck.exfat` |
| `tests/cekirdek/10_uppix_kalici.upp` | Aynı imajlarla ikinci açılış: yazılanlar yerinde mi |
| `tests/cekirdek/12_uppix_snake.upp` | `snake -o -n 200` otopilotu beklemeden oynatır; rekor dosyası, boru/yönlendirmede reddetme ve seçenek hataları |
| `tests/cekirdek/11_uppix_limine.upp` | `uppix.img`'i üretir ve Limine ile 64-bit açar; `/boot` ve kökü yazar, sonra iki bölümü `e2fsck`, `fsck.fat` ve `mtype` ile denetler |

## Sınırlar ve sıradakiler

- ext4 yazımı günlük (journal) kullanmaz. Dosya en çok 4 extent'te tutulur, yani boş alan çok parçalıysa büyük dosya yazılamaz. htree ile dizinlenmiş büyük dizinlere ekleme yapılmaz. Grup bitmap'i başlatılmamış (`*_UNINIT`) gruplar ayırmada atlanır.
- FAT12/FAT16 ve 512 bayt dışı sektör boyutları desteklenmez. Önbellek yoktur: her işlem diske gider.
- Disk sürücüsü ATA PIO'dur; AHCI/NVMe ve virtio-blk sırada.
- Çoklu görev işbirlikçidir. Kullanıcı modu (ring 3), sistem çağrıları ve ELF programları henüz yok; komutlar çekirdek içinde çalışır.
- Çerçeve tamponu 4 GiB'nin altında ve RGB olmalı; değilse ekran çıktısı yalnızca seri porta gider. SMP yok: yalnızca açılış işlemcisi çalışır.
- `uppc` henüz Uppix içinde çalışmıyor, çünkü C çıktısını derleyecek bir derleyici gerekiyor.
