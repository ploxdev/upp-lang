# Uppix

Uppix, tamamen **u++** ile yazılmış, Unix benzeri küçük bir işletim sistemidir. x86_64 makinede Multiboot ile açılır ve libc kullanmaz. **Kök dosya sistemi ext4 bir diskte durur** ve yapılan değişiklikler kalıcıdır. USB bellek gibi **FAT32** ve **exFAT** diskleri tanır, okur ve yazar. Kabuk, dosya sistemi sürücüleri ve 73 komutun hepsi u++ kaynağıdır. Altta yalnızca u++'ın serbest (freestanding) çalışma zamanı vardır: kesmeler, zamanlayıcı, klavye/seri girdi, ATA disk erişimi, bellek ayırıcı ve görev geçişi.

```text
root@uppix:~# lsblk
AD      BOYUT TÜR   FSTÜR  ETİKET           BAĞLAMA
sda       64M disk  ext4   uppix-kok        /
sdb       64M disk
└─sdb1    63M part  vfat   UPPIXUSB         /mnt/sdb1
sdc       32M disk  exfat                   /mnt/sdc
root@uppix:~# cat "/mnt/sdb1/Türkçe uzun dosya adı.txt"
Uzun ve Türkçe bir dosya adı: şğüöçı ŞĞÜÖÇİ
```

## Çalıştırma

Gerekenler: `gcc` ve `qemu-system-x86_64` (Windows'ta WSL içinde de olur). Disk imajları için `mkfs.ext4` (e2fsprogs) gerekir. FAT32 ve exFAT örnek diskleri için ayrıca `mkfs.fat`, `mtools` ve `mkfs.exfat` gerekir (`sudo apt install e2fsprogs dosfstools mtools exfatprogs fdisk`).

```bash
uppc calistir uppix
```

Bu komut eksik disk imajlarını [`disk_yap.sh`](disk_yap.sh) ile üretir (`upp.proje` içindeki `disk_hazirla`), çekirdeği derler ve QEMU'yu üç diskle ve seri konsolla başlatır. Komutları terminale yazarsınız. İmajlar `derleme/` altında kalır, bu yüzden bir sonraki açılışta yaptığınız değişiklikler yerindedir. Sıfırdan başlamak için `derleme/*.img` dosyalarını silin.

| İmaj | İçerik |
|---|---|
| `uppix-kok.img` | 64 MiB ext4, etiket `uppix-kok`: kök (`/`). İçeriği [`kok/`](kok) ağacından gelir; çekirdek `/boot/uppix32.elf` olarak buradadır |
| `usb-fat32.img` | 64 MiB, MBR bölüm tablosu ve 1. bölümde FAT32 (`UPPIXUSB`), örnek dosyalar uzun Türkçe adlarla |
| `usb-exfat.img` | 32 MiB exFAT (bölümsüz) |

Klavye ve VGA ekranıyla bir pencerede açmak için:

```bash
uppc derle uppix
qemu-system-x86_64 -kernel uppix/derleme/uppix32.elf -serial stdio -drive file=uppix/derleme/uppix-kok.img,format=raw,if=ide,index=0
```

Disk verilmezse Uppix RAM'deki bir kök dosya sistemiyle (rootfs) açılır. `poweroff` dosya sistemlerini boşaltıp makineyi kapatır, `reboot` yeniden başlatır.

## Neler var

- **Dosya sistemleri:**
  - **ext4:** okuma ve yazma. Extent ağaçları, eski ext2/3 blok haritaları, htree dizinleri (okuma) ve `metadata_csum` desteklenir: inode, dizin bloğu, bitmap, grup tanımlayıcı ve süperblok sağlamaları CRC32C ile güncellenir. Desteklenmeyen bir özellik varsa disk salt okunur bağlanır.
  - **FAT32:** okuma ve yazma. VFAT uzun dosya adları (UTF-16) okunur ve yazılır, `AD~1.UZN` kısa adları üretilir, tüm FAT kopyaları birlikte güncellenir.
  - **exFAT:** okuma ve yazma. Ayırma bitmap'i, FAT zinciri ve bitişik (NoFatChain) kümeler, girdi kümesi sağlaması, büyük harf tablosuna göre ad karması ve birim etiketi desteklenir.
  - **Bellekte:** `/proc` (`cpuinfo`, `meminfo`, `mounts`, `partitions`, `uptime`, `version`, `dmesg`), `/dev` (`null`, `console`, `tty`, `random`, `sda`…), `/tmp` ve `/mnt` (tmpfs), `/bin` (yerleşik komutlar).
- **Diskler:** ATA PIO ile en çok 4 disk (`sda`…`sdd`); MBR ve GPT bölüm tabloları (`sdb1`…). Açılışta `uppix-kok` etiketli (yoksa ilk) ext4 kök olarak bağlanır, diğer dosya sistemleri `/mnt/<aygıt>` altına bağlanır.
- **Kabuk (`uppsh`):** boru (`|`), yönlendirme (`>`, `>>`, `<`, `2>`), `;`, `&&`, `||` ve arka plan işleri (`&`, `jobs`, `kill %1`). Ayrıca `$AD`, `${AD}`, `$?`, `$$`, `$1…$9`, `$#`, `$@`, `~`, joker (`*`, `?`), tek/çift tırnak, takma adlar, betikler (`sh dosya`, `source`, `PATH` üzerinden çalıştırılabilir dosyalar) ve `/etc/profile`.
- **Satır düzenleyici:** ↑/↓ geçmiş, ←/→, Home/End, Delete, Tab ile komut ve dosya adı tamamlama, Ctrl-A/E/U/K/W/L/C/D.
- **Görevler:** işbirlikçi çoklu görev (en çok 32 görev, her biri 64 KiB yığınla). Görevler `sleep` sırasında ve girdi beklenirken sıra verir. `ps`, `kill` ve `jobs` ile yönetilirler.
- **Donanım:** Multiboot bellek haritası, CMOS saati (dosya zamanları Unix saniyesidir), CPUID, ACPI ile kapatma, 8042 ile yeniden başlatma. VGA metin ekranında ANSI renkleri ve kutu çizgi karakterleri gösterilir.

### Komutlar

| Alan | Komutlar |
|---|---|
| Dosya | `ls` `cd` `pwd` `cat` `mkdir` `rmdir` `touch` `rm` `cp` `mv` `find` `tree` `stat` `du` `chmod` `basename` `dirname` |
| Disk | `lsblk` `blkid` `mount` (`-t`, `-o ro`) `umount` `df` `sync` |
| Metin | `echo` `wc` `head` `tail` `grep` `sort` `uniq` `rev` `cut` `tr` `seq` `tee` |
| Sistem | `ps` `jobs` `kill` `sleep` `uptime` `uname` `date` `free` `dmesg` `nproc` `uppfetch` `clear` `reboot` `poweroff` (`halt`, `shutdown`) |
| Kabuk | `sh` `source` (`.`) `exit` `env` `printenv` `export` `unset` `alias` `unalias` `which` `history` `time` `test` (`[`) `expr` `true` `false` `whoami` `id` `hostname` `help` |

```text
root@uppix:~# mount /dev/sdb1 /mnt/usb -o ro      # elle bağlama (nokta önce mkdir ile)
root@uppix:~# cp ~/not.txt /mnt/sdc/ && sync      # exFAT diske kopyala
root@uppix:~# umount /mnt/sdb1
```

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
| `kaynak/komut_*.upp` | Komutlar; her biri `fonk(liste[metin] a, metin girdi) -> sayi` |
| `kaynak/yardimci.upp` | Sıralama, biçimleme, joker eşleme, takvim/Unix zamanı |

Her dosya sistemi sürücüsü fonksiyon değerlerinden oluşan bir `sinif Surucu`'dur: `bagla`, `yukle` (dizini önbelleğe al), `oku`, `yaz`, `olustur`, `sil`, `tasi`, `oznitelik`, `alan`. VFS hangi düğümün hangi bağlamaya ait olduğunu bilir ve doğru sürücüyü çağırır. Dizinler ilk erişimde yüklenir. Komutlar dosya sisteminden habersizdir; `cp /root/a.txt /mnt/sdc/` gibi diskler arası kopyalama aynı kodla çalışır.

İkili veri `upp.tampon.*` ile işlenir. Bunlar sınır denetimli bayt tamponlarıdır: küçük sonlu okuma/yazma, UTF-16, CRC32C. Disk erişimi `upp.cekirdek.disk_oku/disk_yaz` ile yapılır. Ayrıntılar için [API belgesine](../docs/API_tr.md) bakın.

### Testler

| Test | Ne yapar |
|---|---|
| `tests/cekirdek/08_uppix.upp` | Disksiz açılış (RAM kök); 25 kabuk komutu |
| `tests/cekirdek/09_uppix_disk.upp` | İmajları sıfırdan üretir, üç dosya sistemine yazar, sonra Linux araçlarıyla denetler: `e2fsck -fn`, `debugfs`, `fsck.fat`, `mtype`, `fsck.exfat` |
| `tests/cekirdek/10_uppix_kalici.upp` | Aynı imajlarla ikinci açılış: yazılanlar yerinde mi |

## Sınırlar ve sıradakiler

- ext4 yazımı günlük (journal) kullanmaz. Dosya en çok 4 extent'te tutulur, yani boş alan çok parçalıysa büyük dosya yazılamaz. htree ile dizinlenmiş büyük dizinlere ekleme yapılmaz. Grup bitmap'i başlatılmamış (`*_UNINIT`) gruplar ayırmada atlanır.
- FAT12/FAT16 ve 512 bayt dışı sektör boyutları desteklenmez. Önbellek yoktur: her işlem diske gider.
- Disk sürücüsü ATA PIO'dur; AHCI/NVMe ve virtio-blk sırada.
- Çoklu görev işbirlikçidir. Kullanıcı modu (ring 3), sistem çağrıları ve ELF programları henüz yok; komutlar çekirdek içinde çalışır.
- Önyükleme hâlâ QEMU `-kernel` (Multiboot) ile yapılıyor. Çekirdek ext4'te `/boot` altında duruyor, ama diskten açılış için GRUB ya da kendi önyükleyicimiz sıradaki adım.
- `uppc` henüz Uppix içinde çalışmıyor, çünkü C çıktısını derleyecek bir derleyici gerekiyor.
