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
- [ ] Sonraki: ext4 günlüğü ve büyük dosyalar için extent ağacı yazımı, AHCI/virtio-blk, kesintili zamanlama (runtime kilitleri), kullanıcı modu + sistem çağrıları + ELF yükleyici, `uppc`'nin Uppix içinde çalışması (doğrudan makine kodu arka ucu).

## Faz 4 — Derleme asistanı ✅

- [x] `upp.proje`, `uppc yeni`, `uppc derle`, `uppc calistir`, `uppc asistan` (menü, renkli, araç zinciri durumu).
- [ ] Ok tuşlu tam ekran TUI (ham terminal girdisi gerektirir).

## Faz 5 — Belgeler ve testler

- [x] README, OGREN, API (TR/EN), derleyici.md güncel. 121 test (QEMU çekirdek testleri, Uppix ve disk/dosya sistemi testleri dahil); Linux'ta ASan ve UBSan temiz.
- [x] Eklenti sözlüğü `yerlesik.upp`'den üretilir (`eklenti/src/yerlesik_uretilen.ts`); iç içe ad alanı tamamlaması; sözdizimi `upp.*`'yi listesiz tanır.
