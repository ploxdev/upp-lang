# u++ Beta Yol Haritası

`beta` dalı geriye uyumluluğu bilinçli olarak kırar. Hedef, tutarlı ve işletim sistemi yazmaya uygun bir sistem dili. Kaldırılan her API, yerine neyin geçtiğini söyleyen net bir derleme hatası verir.

## Kararlar

| Konu | Karar |
|---|---|
| Freestanding ilk hedefi | x86_64 çekirdek: Multiboot ile açılır, QEMU `-kernel` ile çalışır |
| Heap'li türler (metin/liste/harita) freestanding'de | Ayırıcı arayüzü: çekirdek bir yığın bölgesi verir ya da `upp_ayir`/`upp_birak` sağlar. Yoksa açık panik |
| Derleme asistanı | `uppc asistan` (terminal TUI) + `upp.proje` dosyası + `uppc derle` |
| Uyumluluk | Kırılabilir, net hata |

## Faz 0 — Python'sız self-host ✅

- [x] `uppc kendini-derle`: birleştirme, runtime gömme, 3 aşama, sabit nokta (aşama2 == aşama3), duman testi, kurulum, `bootstrap/uppc.c` üretimi. `--hizli` iki aşama yapar.
- [x] `araclar/*.py` silindi. Temiz klon yalnızca `gcc bootstrap/uppc.c` ile başlar.
- [x] Çıktı Windows ve Linux'ta bayt bayt aynı (CI: `git diff --exit-code`). Self-host ~5 dk'dan ~30 sn'ye indi.

## Faz 1 — Metin çekirdeği (metin v2) ✅

- [x] `metin` yönetilen bir tür: başlıkta atomik referans sayısı, bayt uzunluğu, kapasite, kod noktası sayısı ve tembel indeks tablosu var. Sihir verinin hemen önündeki 4 bayttır. Veri C uyumlu bir `char*` olarak kalır.
- [x] Kopya O(1) (referans artışı). `+=` yerinde büyür, paylaşılmış metinde copy-on-write yapar.
- [x] `upp.uzunluk` O(1). ASCII metinde `s[i]` O(1). Küresel tek girişli UTF-8 önbelleği kaldırıldı (kendini transpile: 144 sn → 5 sn).
- [x] Metin sabitleri derleme anında başlığı doldurulmuş ölümsüz nesneler (sabit havuzu). Güvenli kod ASan/UBSan/LSan altında temiz.
- [x] Yönetilmeyen işaretçiler (`c_kod`) asla `free` edilmez; eski heap bozulması kapandı.
- [x] `upp.metin_bosalt` kaldırıldı ve kullanımı net bir hata verir.
- [x] Sahipli sınıflar (`metin` ya da sahipli sınıf alanı taşıyanlar): `T__birak`/`T__kopya` üretilir. Yerel değişken, atama, parametre, `don` ve koşullu ifade sızıntısız; eski use-after-free kapandı.
- [x] `upp.metin.buyuk/kucuk` gerçekten Türkçe kurallı (i↔İ, ı↔I) ve UTF-8 farkında.
- [x] Hata konumu doğru dosyayı gösterir (`kullan` ile katılan dosyalar dahil).

## Faz 2 — Dil yüzeyi ve codegen

- [x] İnterpolasyon ayrıştırma anında AST'ye çözülür. `{a[0]}`, `{f(x)}`, `{a + b}` desteklenir. Hatalı ifade sessiz kalmaz, hata verir. `{{`/`}}` her metinde kaçıştır.
- [x] `upp.yaz/satir_yaz/hata_*` printf üretmez, runtime metin kurucusunu kullanır. Her yazdırılabilir türü alır; çok argüman (printf kalıntısı) hata verir.
- [x] Codegen libc çağrısı ya da `#include` üretmez; `arkaplan` dahil her şey runtime üzerinden geçer.
- [x] Yerleşik imza tablosu (`yerlesik.upp`): argüman sayısı ve türleri, dönüş türü, C adı, `guvensiz`, barındırılan, serbest ve platform bayrakları. Bilinmeyen adda mevcut adları önerir.
- [x] Düz eski adlar kaldırıldı. Belgedeki ama var olmayan adlar (`metin.bul`, `matematik.sinus`, `json.indeks_al`, `yol.dosya_adi`, `sistem.cikis`…) artık gerçek.
- [x] `upp.sistem.calistir` iki platformda da stdout ve stderr'i yakalar. `upp.sistem.kos` konsolu devralır.
- [x] Koleksiyonlar v2: `liste`/`harita` değer türü (refcount + copy-on-write), otomatik bırakma, her değer türünü eleman olarak alır (sınıf, iç içe liste/harita, fonksiyon). `L[i][j] = x`, `K[0].ad += ...`, `K[1].metot()` yerinde çalışır. Harita anahtarı `metin`/`sayi`, eklenme sırası korunur.
- [x] Fonksiyon değerleri: `fonk(P...) -> R` türü, adla değer alma, değişken/alan/indis/çağrı sonucu üzerinden çağrı, koleksiyonda saklama, yakalamasız anonim fonksiyon (`fonk(sayi x) -> sayi => x * 2`). `yok` çağrısı açık panik. Çekirdek hedefinde de çalışır.
- [x] `secim` içindeki `dur` doğru döngüyü kırar ve doğru kapsamları bırakır. Tamsayı `/` ve `%` sıfıra bölmede ve `MIN / -1` taşmasında açık panik verir; sabit sıfır bölen derleme hatasıdır.
- [x] Hata yönetimi: `beklenen[T, H]` / `beklenen[T]` (bayrak + union yapısı, gizli ayırma yok), `hata(e)`, `?` ile yayma, `tamam_mi/hata_mi/deger/hata/deger_yoksa`, atılan sonuç derleme hatası. `ertele` ve `hata_ertele` (Zig `defer`/`errdefer`).
- [x] C tarzı `(T)x` dönüşümü (işaretçi dönüşümü yalnızca `guvensiz`; ondalık → tamsayı taşması panik) ve `boyut(T)` / `boyut(degisken)`.
- [x] Üst düzey `sabit metin` değerini taşır (önceden C'de `NULL` kalıyordu). `bayt`, `u32`, `secenek`, `fonk` gibi düz türlü koleksiyonların tanımlayıcısı statik ömürlü oldu; önceden başka fonksiyonda kurulan koleksiyonda sarkıyordu. Ayrıca yeni `upp.metin.karakter` ve `upp.metin.tekrar`. Metinlerde `\uHHHH` kaçışı; bilinmeyen kaçış (eskiden sessizce harfe dönüşüyordu) artık derleme hatası.
- [ ] Sonraki: veri taşıyan `secenek` + desen eşleme, genel (generic) fonksiyonlar, yazılabilir görünümler, yakalamalı kapanışlar.

## Faz 3 — Freestanding / çekirdek ✅

- [x] Runtime katmanları: `upp_cekirdek.c` (libc'siz) + `upp_barindirilan.c` + `upp_serbest.c`, `upp_on_*.c` ön dosyaları; paketleri `kendini-derle` üretir ve gömer.
- [x] `--hedef serbest` (libc'siz `.o`) ve `--hedef cekirdek` (Multiboot x86_64 ELF; QEMU varsa çalıştırır). Araç zinciri: gcc / x86_64-elf-gcc / WSL.
- [x] Serbest runtime: sınır etiketli yığın ayırıcı, COM1 + VGA konsolu (CP437 Türkçe eşleme), panik, libc'siz `%g`.
- [x] `upp.cekirdek.*`: port G/Ç, MMIO, `hlt`/`cli`/`sti`, `yigin_kur`. Barındırılan API'ler, `arkaplan` ve `Kilit` serbest hedefte derleme hatası verir.
- [x] `ornekler/cekirdek/merhaba.upp`. Yapısal doğrulama yapıldı (ELF32, Multiboot başlığı ofset 0x1000, giriş `_start`).
- [x] QEMU ile önyükleme: `tests/cekirdek/` kategorisi (çıktı, istisna, seri girdi, zamanlayıcı) QEMU'da açılır. CI Linux işi QEMU kurar.
- [x] IDT + 32 istisna (Türkçe ad, RIP, CR2), PIC yeniden eşleme, 1000 Hz PIT, PS/2 klavye (Türkçe Q), COM1 UTF-8 girdi.
- [x] Çekirdekte `upp.giris`, `upp.zaman`, `upp.uyut`; `upp.cekirdek.tik/tus_oku/tus_bekle/kesmeler_kur`. Örnek: `ornekler/cekirdek/kabuk.upp`.
- [x] Multiboot bellek haritası: yığın kullanılabilir RAM'e yayılır (`bellek_toplam`, `yigin_boyut`, `yigin_kullanilan`).
- [x] İşbirlikçi görevler: `gorev_baslat(fonk(sayi) -> bos, arg, ad)`, `sira_ver`, `gorev_listesi/ad/durum/baslama/oldur`. `uyut` ve girdi beklemesi sırayı verir.
- [x] CMOS saati (`tarih`), ACPI kapatma, 8042 yeniden başlatma, CPUID (`islemci`), `ekran_temizle`. VGA'da ANSI renk/imleç dizileri ve kutu çizgi karakterleri; klavyede Ctrl ve ok tuşları.
- [x] **Uppix** (`src/os/`): u++ ile yazılmış Unix benzeri işletim sistemi. İçinde `uppsh` kabuğu (boru, yönlendirme, `&&`/`||`, `&`, değişkenler, joker, takma ad, betik), `/proc` ve `/dev` içeren ramfs, satır düzenleyici (geçmiş, Tab) ve 70 komut var. QEMU testi: `tests/cekirdek/08_uppix.upp`.
- [x] Diskler: ATA PIO sürücüsü (`upp.cekirdek.disk_*`), güvenli bayt tamponları (`upp.tampon.*`: LE okuma/yazma, UTF-16, CRC32C), `uppc --disk` ve `upp.proje` içinde `diskler` / `disk_hazirla`.
- [x] Uppix 0.2: VFS bağlama tablosu ve fonksiyon değerli sürücü arayüzü. **ext4 kök** (okuma/yazma, extent, metadata_csum), **FAT32** (LFN) ve **exFAT** okuma/yazma, MBR/GPT bölümleri. `mount`, `umount`, `lsblk`, `blkid`, gerçek `df`. Yazılan imajlar `e2fsck`, `fsck.fat` ve `fsck.exfat`'tan temiz geçer (`tests/cekirdek/09_uppix_disk.upp`, `10_uppix_kalici.upp`).
- [x] Uppix 0.3: **Limine ile diskten 64-bit açılış** (BIOS ve UEFI). Limine v11.4.1 depoya gömülü (`src/os/onyukleyici/limine`). `uppc --onyukleyici limine` higher-half ELF64 üretir; runtime Limine isteklerinden bellek haritası, çerçeve tamponu ve HHDM alır, kendi GDT'sini ve sayfa tablolarını kurar, yerel APIC'i kapatır. Çerçeve tamponu konsolu (Terminus 8x16, Türkçe). `--disk-hazirla` ve `upp.proje` `onyukleyici = limine`. `uppix.img`: FAT32 `/boot` + ext4 kök (`tests/cekirdek/11_uppix_limine.upp`).
- [x] Dil: `c_kod` metin sözleşmesi. `metin` her zaman yönetilir; runtime başlığı tahmin etmez (ASan/UBSan temiz). Ham C dizgisi `upp_metin_ata_c` ile kopyalanarak girer; `c_kod` içinde metin değişkenini yeniden yönlendirmek ya da içeriğine yazmak derleme hatasıdır.
- [x] Uppix: `snake` yılan oyunu (otopilot `-o`, kalıcı rekor). Konsol: `\e[satır;sütunH`, `\e[?25l/h`, çerçeve tamponunda `░▒▓` (`tests/cekirdek/12_uppix_snake.upp`).
- [x] Dil: karakter sabitleri (`'a'`, kaçışlar, Türkçe), `her (sayi i in a..b)` aralık döngüsü, `upp.metin.ingkucuk` / `ingbuyuk` (ASCII). İnterpolasyon `{...}` içi ham kaynaktan ayrıştırılır (`"{'\n'}"`).
- [x] Dil: `c_harici` C fonksiyonu bildirimleri (asm etiketiyle; başlık prototipiyle çakışmaz) ve `cmetin` türü; `@paketli`, `@hizali(N)`, `@bolum("ad")` nitelikleri; `uppc --guvenlik-denetimi[=is]` (ASan + UBSan / TSan; Windows'ta UBSan tuzağı).
- [x] Dil: `sirali` — veri taşıyabilen numaralandırma (etiketli birleşim). Düz `sirali` bir `secenek`'tir; veri taşıyan bir değer sınıfına ve varyant başına kurucuya indirgenir. `secim` ile kalıp eşleştirme (`durum Hata(m, _)`), tamlık denetimi, düşmeyen kollar, alanlara doğrudan erişim yasağı (`tests/positive/43_sirali.upp`, negatif 62-69). Takma adla katılan modüller (`kullan ... olarak m`) kendi sınıf, `secenek`, `sirali`, fonksiyon, küresel ve sabit adlarını nitelemesiz kullanabilir; dışarıdan `m.f` fonksiyon değeridir; yerel değişkenler modül, sınıf ve seçenek adlarını gölgeler; geçici değer üzerinde metot çağrısı (`kutu(3).iki_kat()`) geçerli C üretir (`44`-`47`).
- [x] Derleyici kararlılığı: seçenek açık değerleri onluk okunur (eksi değer, tekrar eden üye hatası); `dongu` adım ifadesi her turda (interpolasyon dahil) çalışır; sınıflar bağımlılık sırasıyla üretilir, değer içerme döngüsü u++ hatasıdır; C anahtar sözcükleri/makroları ve libc adları u++ hatasıdır; dizi sabiti üzerinde `her`; metin sözlük sırası (`<`), metin aritmetiği ve metin–sayı karşılaştırması hatası; `her` eleman türü, liste indeksi ve harita anahtarı denetimi; işaretçi derinliği; `oto x = bos_fonk()` hatası; `ertele` içinde anonim fonksiyon; `arkaplan`'da interpolasyon ifadelerinin yakalanması; gölgelenen sahipli değişkenin erken dönüşte sızmaması; takma adlı modülde `boyut(T)`/`(T)x`; bozuk tür adlı hata iletileri; GCC hata satırlarının gösterilmesi (`48_dil_duzeltmeleri`, negatif 70-80).
- [x] Düzeltmeler: `arkaplan` sahipli sınıf ve veri taşıyan `sirali` yakalamaz (sığ kopya serbest belleğe erişiyordu); gölgeleyen değişkenin ilk değeri dıştakini okur (`sayi x = x + 1`); `sirali` kalıbında konuyla aynı adlı bağlama; metin `secim`'inde herhangi bir yerde `varsayilan`; düz `sirali`'ye geçersiz değer panikler. Uppix: `MAP_SHARED` dosya eşlemeleri `msync`/`munmap`/süreç sonunda geri yazılır (`msync` eklendi); ext4 yazılmamış kapsamların blokları silmede bırakılır; FAT32 sektör tamponu ayırmada bırakılır (`49_golge_secim`, negatif 81-82, runtime 07, çekirdek 13 ve 15).
- [x] Uppix 0.4: **ring 3 ve Linux x86_64 uyumlu sistem çağrıları**. Çekirdek tamamen üst yarıda (HHDM), süreç başına PML4, NX/SMEP/SMAP, GDT/TSS, `SYSCALL`/`SYSRET`, ayrı fiziksel çerçeve havuzu. ELF64 yükleyici ve SysV ilk yığını (auxv dahil). Sistem çağrıları: read, write, open, close, stat, fstat, lseek, mmap (`/dev/fb0` dahil), munmap, brk, ioctl (`TIOCGWINSZ`, `TCGETS`, `FBIOGET_*`), nanosleep, exit. Ring 3 istisnası yalnızca süreci sonlandırır; ring 3 kodu zaman dilimiyle kesilir; Ctrl-C ile SIGINT. Mekanizma C'de (`upp_kullanici.c`), politika u++'ta (`surec.upp`, `sistem_cagrisi.upp`). `sctest`'in çıktısı gerçek Linux'takiyle birebir aynı (`tests/cekirdek/13_uppix_kullanici.upp`). Çerçeve tamponu konsolu artık kaydırırken aygıt belleğinden okumuyor (yalnızca değişen hücreler, PAT write-combining).
- [x] Uppix 0.4: ~60 Linux sistem çağrısı daha: `openat`/`newfstatat`/`*at` ailesi, `pread`/`pwrite`, `readv`/`writev`, `dup`/`dup2`/`dup3`, `fcntl`, `getdents64`, `access`, `getcwd`/`chdir`, `mkdir`, `unlink`/`rmdir`, `rename`, `readlink` (`/proc/self/exe`), `mprotect`, `madvise`, `arch_prctl` (süreç başına FS/GS, TLS), `uname`, `clock_gettime`/`getres`/`nanosleep`, `gettimeofday`, `time`, `getrandom`, `rt_sigaction`/`rt_sigprocmask`, `prlimit64`, `umask`, `futex`, `kill`, `sysinfo`. `sctest2`'nin 151 satırı Linux'takiyle aynı; **glibc ile `gcc -static` derlenmiş program** (`gmerhaba`) değiştirilmeden çalışıyor. Komutlar: `nano`, `vi`/`vim` (ortak düzenleyici çekirdeği), `hexdump`/`hd` (util-linux'la aynı çıktı). Konsol: ters video, arka plan renkleri, `\e[nA/B`, ertelenmiş satır sarma (`tests/cekirdek/14_uppix_araclar.upp`). Derleyici: `bos` fonksiyonda `=> f()` / `don f()` artık geçerli C üretiyor.
- [ ] Sonraki: `fork`/`execve`/`wait`, sinyal işleyicilerinin çağrılması, `clone` (iş parçacıkları), dinamik bağlama, sayfa hatasıyla büyüyen yığın, ext4 günlüğü ve büyük dosyalar için extent ağacı yazımı, AHCI/virtio-blk, `uppc`'nin Uppix içinde çalışması (doğrudan makine kodu arka ucu).

## Faz 4 — Derleme asistanı ✅

- [x] `upp.proje`, `uppc yeni`, `uppc derle`, `uppc calistir`, `uppc asistan` (menü, renkli, araç zinciri durumu).
- [ ] Ok tuşlu tam ekran TUI (ham terminal girdisi gerektirir).

## Faz 5 — Belgeler ve testler

- [x] README, OGREN, API (TR/EN), derleyici.md güncel. 169 test (QEMU çekirdek testleri, Uppix ve disk/dosya sistemi testleri dahil); Linux'ta ASan ve UBSan temiz.
- [x] Eklenti sözlüğü `yerlesik.upp`'den üretilir (`eklenti/src/yerlesik_uretilen.ts`); iç içe ad alanı tamamlaması; sözdizimi `upp.*`'yi listesiz tanır.
