<p align="center">
  <img src="https://raw.githubusercontent.com/ploxdev/upp-lang/main/docs/logo.png" alt="u++ logo" width="120" onerror="this.style.display='none'"/>
</p>

<h1 align="center">u++ Programlama Dili</h1>

<p align="center">
  <strong>Türkçe sözdizimli, C11'e derlenen (transpiled), çalışma zamanını kendi içine gömen (standalone) ve varsayılan bellek güvenliğiyle işletim sistemi çekirdeğinden masaüstü uygulamalarına kadar uzanan modern sistem programlama dili.</strong>
</p>

<p align="center">
  <a href="OGREN.md"><img src="https://img.shields.io/badge/sürüm-v3.1%20beta-007ACC.svg?style=flat-square" alt="Sürüm" /></a>
  <a href="#-lisans"><img src="https://img.shields.io/badge/lisans-Apache%202.0-blue.svg?style=flat-square" alt="Lisans" /></a>
  <img src="https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20x86__64%20Çekirdek-222222.svg?style=flat-square" alt="Platform" />
  <img src="https://img.shields.io/badge/derleyici-Native%20Self--Hosted%20(Standalone)-success.svg?style=flat-square" alt="Derleyici" />
  <img src="https://img.shields.io/badge/testler-tümü%20geçti-brightgreen.svg?style=flat-square" alt="Testler" />
  <a href="eklenti/README.md"><img src="https://img.shields.io/badge/IDE-VS%20Code%20%7C%20Cursor-blueviolet.svg?style=flat-square" alt="IDE Desteği" /></a>
</p>

<p align="center">
  <a href="OGREN.md">📚 Öğrenme Rehberi</a> ·
  <a href="docs/API_tr.md">📖 API Referansı (TR)</a> ·
  <a href="docs/API_en.md">🌐 API Reference (EN)</a> ·
  <a href="derleyici.md">⚙️ Derleyici Mimarisi</a> ·
  <a href="src/os/README.md">🐧 Uppix İşletim Sistemi</a>
</p>

---

## 📌 Proje Nedir?

**u++**, geliştiricilerin kendi anadilinde (Türkçe) sistem seviyesinde yazılım geliştirmesini sağlayan, yorumlayıcı (interpreter) veya sanal makine (VM) katmanını aradan çıkararak **doğrudan makine koduna (native binary)** derlenen açık kaynaklı bir programlama dilidir.

Yüksek seviyeli dillerin okunabilirliğini C'nin çıplak metal (bare-metal) performansıyla birleştirir. Yazdığınız `u++` kaynak kodu önce standart **GNU C11 (`-std=gnu11`)** koduna dönüştürülür, ardından sistemdeki **GCC** derleyicisiyle Windows üzerinde bağımsız `.exe`, Linux üzerinde `.out` ve çekirdek hedefinde önyüklenebilir **x86_64 ELF** çekirdeklerine (Multiboot ya da Limine) bağlanır. C çalışma zamanı kütüphanesi doğrudan derleyici ikilisinin içine gömülüdür (standalone); harici `.c` veya çalışma zamanı dosyaları taşımaya gerek kalmaz.

> [!NOTE]
> **Eğitim, Araştırma ve Hobi Kapsamı:** Bu proje; derleyici tasarımı (compiler design), soyut sözdizim ağaçları (AST), transpile teknikleri, dil sunucusu mimarisi (LSP), düşük seviye bellek yönetimi ve işletim sistemi çekirdeği geliştirme konularını derinlemesine keşfetmek amacıyla geliştirilmiştir. Sıfırdan başlayanlar için hazırlanan **[OGREN.md (Öğrenme Rehberi)](OGREN.md)** belgesinden dili adım adım öğrenebilirsiniz.

---

## 🚀 Öne Çıkan Özellikler

- **🇹🇷 Doğal ve Tutarlı Türkçe Sözdizimi:** `fonk`, `eger`, `yoksa`, `iken`, `dongu`, `her`, `secim`, `sinif`, `don`, `ertele`, `kullan` gibi akıcı ve okunaklı anahtar sözcükler.
- **⚡ Gerçek Native Performans:** Yorumlayıcı veya çöp toplayıcı (GC) duraksaması yoktur. Doğrudan optimize C11 üretimi ve GCC derlemesiyle C hızında çalışır.
- **🛡️ Varsayılan Bellek Güvenliği:** Dizi ve dilim erişimleri sınır denetimlidir; tamsayı sıfıra bölme ve taşma durumları kontrollü panik üretir. Ham işaretçiler (`*`, `&`), işaretçi tür dönüşümleri, harici süreç belleği erişimleri ve doğrudan donanım G/Ç çağrıları yalnızca açıkça yazılan `guvensiz { ... }` blokları içinde serbesttir.
- **♻️ Otomatik Bellek Yönetimi ve Görünümler:** Metinler (`metin`), listeler (`liste[T]`), haritalar (`harita[K, V]`) ve sahipli sınıflar atomik referans sayımı + yazma anında kopyalama (**Copy-on-Write / COW**) modeliyle çalışır. Derleyici kapsam çıkışlarında, `dur`/`devam` ve `don` noktalarında bırakma kodunu kendisi üretir; kullanıcı elle bellek boşaltmaz. Kopyasız ve sıfır ayırmalı okuma için `&metin` ve `&dilim[T]` (`dizi[1..4]`) görünümleri bulunur.
- **🚦 Sıfır Maliyetli Hata Yönetimi (`beklenen`, `?`, `ertele`):** İstisna (exception) fırlatma yerine değer tabanlı `beklenen[T, H]` (kısaca `beklenen[T]`) kullanılır. `don hata(e)` ile hata döndürülür, `ifade?` ile hatalar temizlik yapılarak yukarı iletilir; kontrol edilmeyen `beklenen` sonuçları derleme hatası verir. `ertele` kapsamdan her çıkışta, `hata_ertele` ise yalnızca hata dönüşlerinde kaynak temizliği yapar.
- **🧱 Zengin Tür Sistemi ve Fonksiyon Değerleri:**
  - Tam genişlikli tamsayı ailesi (`sayi`/`s64`, `u64`/`usayi`, `s32`/`u32`, `s16`/`u16`, `bayt`/`s8`, `ubayt`/`u8`), `ondalik`, `metin`, `mantik`, `dizi[T, N]`, `sinif` ve `secenek`.
  - Birinci sınıf fonksiyon türleri (`fonk(sayi) -> sayi`): fonksiyonlar değişkende, sınıf alanında ve koleksiyonda saklanabilir; yakalamasız anonim fonksiyonlar (`fonk(sayi x) -> sayi => x * 2`) tanımlanabilir.
  - C tarzı `(T)x` açık tür dönüşümü ve derleme zamanı `boyut(T)` (`sizeof`) desteği.
  - Karakter sabitleri (`'a'`, `'ş'`, `'\n'`, `'\u015f'`), aralık döngüsü (`her (sayi i in 0..n)`) ve yerel ayardan bağımsız `upp.metin.ingkucuk` / `ingbuyuk`.
- **🔌 Düşük Seviye ve C ile Birlikte Çalışma:** `c_harici fonk strlen(cmetin s) -> u64;` ile C fonksiyonları tür güvenli bildirilir (ham C dizgisi için `cmetin`); `@paketli`, `@hizali(N)`, `@bolum("ad")` nitelikleri bellek düzenini denetler. `uppc --guvenlik-denetimi` programı ASan + UBSan ile derler.
- **🖥️ Çıplak Metal Çekirdek Hedefi (`--hedef cekirdek`):** Tek komutla `u++` programını x86_64 işletim sistemi çekirdeğine derler: Multiboot ile QEMU `-kernel`, ya da `--onyukleyici limine` ile diskten 64-bit higher-half ELF64 (BIOS ve UEFI). `libc` kullanılmaz; derleyiciye gömülü önyükleyici (Long Mode, sayfalama, SSE), 48 vektörlük IDT, 1000 Hz PIT zamanlayıcı, Türkçe Q PS/2 klavye, COM1 seri port, VGA metin ya da çerçeve tamponu konsolu, ATA PIO disk sürücüsü ve işbirlikçi görev zamanlayıcı ile gelir.
- **🐧 Uppix İşletim Sistemi:** Tamamen `u++` ile yazılmış Unix benzeri işletim sistemi ([`src/os/`](src/os/README.md)). Depoya gömülü **Limine** ile diskten 64-bit açılır. Kullanıcı programları **ring 3'te**, kendi adres alanlarında çalışır ve sistem çağrıları **Linux x86_64 ile birebir uyumludur**: Linux için `gcc -static -nostdlib` ile derlenmiş bir ELF değiştirilmeden çalışır. Kalıcı **ext4** kök dosya sistemi, **FAT32** ve **exFAT** okuma/yazma sürücüleri, MBR/GPT bölüm desteği, sanal dosya sistemi (VFS), borulu ve yönlendirmeli `uppsh` kabuğu, `/proc` ve `/dev` ağaçları ve **74 komut** (yılan oyunu `snake` dahil) içerir.
- **🔄 Python'sız Self-Hosting Derleyici:** Derleyicinin tamamı (`src/uppc/*.upp`) güvenli `u++` ile yazılmıştır. `uppc kendini-derle` komutu yaklaşık 30 saniyede 3 aşamalı derleme yaparak sabit noktayı (`aşama2 == aşama3`) doğrular ve Windows ile Linux'ta bayt bayt aynı [`bootstrap/uppc.c`](bootstrap/uppc.c) dosyasını üretir.
- **🧰 Proje Yöneticisi, Asistan ve IDE Eklentisi:** `uppc yeni`, `uppc derle`, `uppc calistir`, etkileşimli `uppc asistan` menüsü ve VS Code / Cursor için canlı tanı (`--analiz`) ile otomatik tamamlama sunan resmi eklenti.

---

## 🛠️ Derleme Hattı ve Mimari

`u++` derleme hattı, kaynak koddan çalıştırılabilir ikiliye kadar modüler bir boru hattı izler:

```text
┌─────────────────┐     ┌──────────────────┐     ┌──────────────────────┐
│  Kaynak (.upp)  │ ──> │ Lexer & Parser   │ ──> │ Tür & Güvenlik Den.  │
└─────────────────┘     └──────────────────┘     └──────────┬───────────┘
                                                            │
┌─────────────────┐     ┌──────────────────┐                ▼
│ Native (.exe)   │ <── │ GCC Bağlama      │ <── ┌──────────────────────┐
│ Linux / .elf    │     │ (-std=gnu11)     │     │ C11 (Gömülü Runtime) │
└─────────────────┘     └──────────────────┘     └──────────────────────┘
```

1. **Ayrıştırma (Frontend):** UTF-8 kod noktası tabanlı sözcük tarayıcı (Lexer) ve özyinelemeli inişli ayrıştırıcı (Parser), kaynak kodu Structure-of-Arrays (SoA) tabanlı AST düğüm havuzuna dönüştürür.
2. **Tür ve Güvenlik Denetimi:** Çok seviyeli ad alanları, tür uyumluluğu, görünüm ömür kuralları (`&metin` / `&dilim[T]`), `beklenen` sonuçlarının yok sayılmaması ve `guvensiz` blok sınırları denetlenir.
3. **C11 Kod Üretimi (Backend):** AST; otomatik kapsam temizliği (`drop` ve `ertele` yığınları), COW koleksiyon çağrıları ve hedef platforma uygun gömülü çalışma zamanı (`upp_runtime.c` veya `upp_runtime_serbest.c`) ile birleştirilerek tek bir C11 dosyasına çevrilir.
4. **Bağlama (Linker):** Hedefe göre (`yerel`, `windows`, `linux`, `serbest`, `cekirdek`) GCC ve bağlayıcı tetiklenerek bağımsız makine ikilisi üretilir.

Detaylı iç mimari için **[derleyici.md](derleyici.md)** belgesini inceleyebilirsiniz.

---

## 📦 Kurulum ve Gereksinimler

### Sistem Gereksinimleri
- **GCC Derleyicisi:**
  - **Windows:** [MinGW-w64](https://www.msys2.org/) (MSYS2 `ucrt64` veya `mingw64` ortamı `PATH` içinde olmalıdır)
  - **Linux:** `build-essential` (`sudo apt install build-essential`)
- **Çekirdek Hedefi ve Uppix İçin (İsteğe Bağlı):**
  - Linux'ta yerel `gcc`; Windows'ta `x86_64-elf-gcc` veya **WSL** içinde `gcc`, `ld`, `objcopy`.
  - Önyükleme ve simülasyon için `qemu-system-x86_64`.
- **Python 3.8+ (İsteğe Bağlı):** Yalnızca `tests/run_tests.py` test koşucusu için kullanılır; derleyici veya `kendini-derle` Python kullanmaz.

### Depoyu Klonlama ve Derleyiciyi Kurma

Temiz bir klondan doğrudan **bootstrap C** dosyasıyla native derleyiciyi (`uppc`) saniyeler içinde derleyebilirsiniz:

```bash
git clone https://github.com/ploxdev/upp-lang.git
cd upp-lang

# Windows (MinGW)
gcc -std=gnu11 -O2 bootstrap/uppc.c -o derleyici/uppc.exe -luser32 -lwinmm -lgdi32

# Linux
gcc -std=gnu11 -O2 bootstrap/uppc.c -o derleyici/uppc -pthread -lm
```

---

## 💻 Hızlı Başlangıç

### 1. Merhaba Dünya

`merhaba.upp` adında bir dosya oluşturun:

```upp
fonk ana() -> sayi {
    upp.satir_yaz("Merhaba, u++ dünyası!");
    don 0;
}
```

### 2. Derleme ve Çalıştırma

```bash
# Doğrudan derle ve anında çalıştır:
./derleyici/uppc.exe merhaba.upp

# Yalnızca çalıştırılabilir ikiliyi (.exe / .out) üret:
./derleyici/uppc.exe merhaba.upp -o merhaba --sadece-derle

# Yalnızca üretilen C11 kodunu incele (GCC çağrılmaz):
./derleyici/uppc.exe merhaba.upp -c -o merhaba.c

# Sürüm ve yardım bilgisi:
./derleyici/uppc.exe -v
./derleyici/uppc.exe -h
```

---

## 🔍 Dil Sözdiziminden Örnekler

### Değişkenler, Türler ve İnterpolasyon
```upp
fonk ana() -> sayi {
    oto isim = "Arda";
    sayi yas = 24;
    ubayt seviye = 255;
    ondalik pi = 3.1415;
    mantik aktif = dogru;

    // Süslü parantez içinde her türlü ifade doğrudan metne gömülebilir:
    upp.satir_yaz("Kullanıcı: {isim}, Yaş: {yas}, Seviye: {seviye}, Aktif: {aktif}");
    don 0;
}
```

### Değer Türü Koleksiyonlar (`liste` ve `harita`)
```upp
fonk ana() -> sayi {
    liste[sayi] sayilar;
    sayilar.ekle(10);
    sayilar.ekle(20);
    sayilar.ekle(30);
    sayilar.sil(1); // 20 silinir
    upp.satir_yaz("Eleman sayısı: {sayilar.uzunluk()}");

    harita[metin, sayi] puanlar;
    puanlar.koy("Ahmet", 95);
    puanlar.koy("Mehmet", 80);
    her (oto kisi in puanlar) {
        upp.satir_yaz("{kisi} -> {puanlar.al(kisi)}");
    }
    don 0;
}
```

### Birinci Sınıf Fonksiyon Değerleri ve Anonim Fonksiyonlar
```upp
fonk ikiye(sayi x) -> sayi => x * 2;

fonk esle(liste[sayi] L, fonk(sayi) -> sayi f) -> liste[sayi] {
    liste[sayi] R;
    her (oto x in L) { R.ekle(f(x)); }
    don R;
}

fonk ana() -> sayi {
    liste[sayi] L;
    L.ekle(1);
    L.ekle(2);
    oto a = esle(L, ikiye);
    oto b = esle(L, fonk(sayi x) -> sayi => x * x + 1);
    upp.satir_yaz("{a[1]} {b[1]}"); // 4 5
    don 0;
}
```

### Hata Yönetimi (`beklenen`, `?`, `ertele` ve `hata_ertele`)
```upp
fonk bol(sayi a, sayi b) -> beklenen[sayi] {
    eger (b == 0) { don hata("sıfıra bölme"); }
    don a / b;
}

fonk hesapla() -> beklenen[sayi] {
    ertele upp.satir_yaz("hesaplama kapsamından çıkıldı");
    hata_ertele upp.satir_yaz("hata oluştu, geri alınıyor");

    sayi x = bol(10, 2)?;
    don x + bol(x, 0)?; // İkinci bölme hata verir; hata_ertele ve ertele çalışır, hata yukarı döner
}

fonk ana() -> sayi {
    oto r = hesapla();
    eger (r.hata_mi()) {
        upp.satir_yaz("Yakalanan hata: {r.hata()}");
    }
    don 0;
}
```

### Sınıflar ve Otomatik Sahiplik
```upp
sinif Motor {
    metin model;
    sayi beygir;

    fonk olustur(metin m, sayi bg) {
        kendi.model = m;
        kendi.beygir = bg;
    }

    fonk bilgi_ver() {
        upp.satir_yaz("Model: {kendi.model} ({kendi.beygir} HP)");
    }
}

fonk ana() -> sayi {
    Motor m = Motor("V8 Twin-Turbo", 650);
    m.bilgi_ver();
    don 0;
}
```

### Çoklu İş Parçacığı (`arkaplan` ve `Kilit`)
```upp
fonk ana() -> sayi {
    Kilit k;

    oto gorev = arkaplan {
        k.kilitle();
        upp.satir_yaz("Arka plan iş parçacığı güvenli bölgede çalışıyor.");
        k.birak();
    };

    gorev.bekle();
    upp.satir_yaz("Görev tamamlandı.");
    don 0;
}
```

### Bellek Güvenliği Kapısı (`guvensiz`)
```upp
fonk ana() -> sayi {
    sayi x = 100;

    // Güvenli alanda işaretçi ve adres operatörleri derleme hatası verir:
    guvensiz {
        sayi* p = &x;
        *p = 200;
        upp.satir_yaz("İşaretçi üzerinden yeni değer: {x}");
    }
    don 0;
}
```

---

## 🧰 Proje Sistemi ve Derleme Asistanı

`uppc`, tekil dosyaların yanı sıra `upp.proje` yapılandırmasına dayalı proje yönetimini ve menülü derleme asistanını destekler:

```bash
uppc yeni oyunum                       # oyunum/upp.proje + oyunum/kaynak/ana.upp oluşturur
cd oyunum && uppc calistir             # projeyi derler ve çalıştırır
uppc yeni cekirdegim --hedef cekirdek  # önyüklenebilir çekirdek projesi oluşturur
uppc asistan                           # etkileşimli menü: hedef seçimi, araç zinciri denetimi, derleme
```

---

## 🖥️ İşletim Sistemi Geliştirme (`--hedef cekirdek`) ve Uppix

`--hedef cekirdek` bayrağı, yazdığınız `u++` programını hiçbir işletim sistemi veya `libc` bağımlılığı olmadan doğrudan donanım/QEMU üzerinde açılan bir **x86_64 çekirdeğine** dönüştürür:

```upp
// ornekler/cekirdek/merhaba.upp
fonk qemu_cik(sayi kod) -> bos {
    guvensiz { upp.cekirdek.port_yaz32(0xf4, kod); }
}

fonk ana() -> sayi {
    upp.satir_yaz("Merhaba, u++ çekirdeği! İşletim sistemi yok, libc yok.");
    liste[sayi] L;
    dongu (sayi i = 0; i < 5; i += 1) { L.ekle(i * i); }
    upp.satir_yaz("Kareler: {L[1]} {L[2]} {L[3]}");
    qemu_cik(0);
    don 0;
}
```

```bash
uppc ornekler/cekirdek/merhaba.upp --hedef cekirdek
```

Dilin tamamı çekirdek hedefinde de eksiksiz çalışır: sınıflar, `liste`/`harita`, metin interpolasyonu, `beklenen`/`ertele` ve otomatik bellek yönetimi. Derleyiciye gömülü serbest çalışma zamanı (`src/runtime/upp_on_serbest.c` + `upp_cekirdek.c` + `upp_serbest.c`) şu donanım olanaklarını hazır sunar:

- **Multiboot 1 Önyükleyici:** 32 bit korumalı moddan 64 bit Long Mode'a geçiş, ilk 1 GiB birebir sayfalama (2 MiB sayfalar) ve SSE etkinleştirme.
- **Limine Protokolü (`--onyukleyici limine`):** `0xffffffff80000000`'da duran ELF64 çekirdek; bellek haritası, çerçeve tamponu ve HHDM Limine isteklerinden alınır, kendi GDT'si ve sayfa tabloları kurulur. `--disk-hazirla` ile derlemeden sonra önyüklenebilir disk imajı hazırlanır.
- **Kesmeler ve Zamanlayıcı:** 48 vektörlük IDT, Türkçe işlemci istisna panikleri (`RIP` ve `CR2` dökümü), PIC yeniden eşleme ve 1000 Hz PIT zamanlayıcı (`upp.zaman`, `upp.uyut`).
- **Konsol ve Girdi:** COM1 seri port + VGA metin ekranı (CP437 Türkçe karakter) ya da çerçeve tamponu konsolu (Terminus 8x16 yazı tipi), ikisinde de ANSI renkleri, Türkçe Q düzeninde PS/2 klavye (`upp.giris`, `upp.cekirdek.tus_oku`).
- **Donanım, Disk ve Görevler:** Limine/Multiboot bellek haritası üzerinden yığın ayırıcı, ATA PIO disk okuma/yazma, CMOS gerçek zaman saati, CPUID, ACPI kapatma ve 32 göreve kadar işbirlikçi çoklu görev (`upp.cekirdek.gorev_baslat`).

### 🐧 Uppix: u++ ile Yazılmış Unix Benzeri İşletim Sistemi

[`src/os/`](src/os/README.md), `--hedef cekirdek` altyapısı üzerinde tamamen `u++` ile geliştirilmiş kapsamlı bir işletim sistemidir:

- **Diskten 64-bit Açılış:** Depoya gömülü [Limine](src/os/onyukleyici/limine) v11.4.1, `uppix.img`'in MBR'sinden (BIOS) ya da `EFI/BOOT/BOOTX64.EFI`'den (UEFI) çekirdeği yükler. İmajın 1. bölümü FAT32 `/boot`, 2. bölümü ext4 köktür. Ekran, Türkçe karakterli çerçeve tamponu konsoludur.

- **Kalıcı Disk Dosya Sistemleri:**
  - **ext4 Kök (`/`):** Extent ağaçları ve `metadata_csum` (CRC32C) sağlamalarıyla tam okuma/yazma. Yazılan disk imajları Linux `e2fsck -fn` denetiminden hatasız geçer.
  - **FAT32 ve exFAT:** UTF-16 uzun Türkçe dosya adlarıyla (VFAT LFN) okuma/yazma, MBR ve GPT bölüm tablosu tanıma, `/mnt` altına otomatik veya `mount`/`umount` ile elle bağlama.
  - **Sanal Dosya Sistemleri:** `/proc`, `/dev`, `/tmp` (`tmpfs`) ve `/bin`.
- **`uppsh` Kabuğu ve 74 Komut:** Boru (`|`), yönlendirme (`>`, `>>`, `<`, `2>`), `&&`/`||`, arka plan işleri (`&`, `jobs`, `kill %1`), ortam değişkenleri, joker (`*`, `?`), takma adlar (`alias`), `#!/bin/sh` betikleri, Tab tamamlama ve komut geçmişi.

```text
root@uppix:~# lsblk
AD      BOYUT TÜR   FSTÜR  ETİKET           BAĞLAMA
sda      192M disk
├─sda1    64M part  vfat   UPPIXBOOT        /boot
└─sda2   127M part  ext4   uppix-kok        /
sdb       64M disk
└─sdb1    63M part  vfat   UPPIXUSB         /mnt/sdb1
sdc       32M disk  exfat                   /mnt/sdc
root@uppix:~# echo merhaba > ~/not.txt && cp ~/not.txt /mnt/sdc/
root@uppix:~# seq 15 | grep 1 | wc -l
7
```

Uppix'i disk imajlarıyla birlikte derleyip QEMU üzerinde hemen başlatmak için:

```bash
uppc calistir src/os
```

---

## ⚙️ Kendi Kendini Derleme (Self-Hosting)

Derleyici kaynaklarında (`src/uppc/*.upp`) veya çalışma zamanı katmanlarında (`src/runtime/*.c`) değişiklik yaptıktan sonra derleyiciyi kendisiyle yeniden inşa edebilirsiniz:

```bash
# Tam self-host: 3 aşama derler, aşama2 == aşama3 sabit noktasını doğrular ve bootstrap/uppc.c'yi günceller
./derleyici/uppc kendini-derle

# Hızlı derleme: 2 aşama derler (geliştirme sırasında hızlı çevrim için)
./derleyici/uppc kendini-derle --hizli
```

---

## 🧪 Test Paketi ve Kalite Güvencesi

`u++`, pozitif dil testleri, bellek güvenliği kapısı testleri, negatif derleme testleri, çalışma zamanı panik testleri ve QEMU çekirdek/disk testlerinden oluşan kapsamlı bir test paketiyle doğrulanır:

```bash
python tests/run_tests.py

# Kategori bazlı çalıştırma:
python tests/run_tests.py --kategori positive
python tests/run_tests.py --kategori safety
python tests/run_tests.py --kategori negative
python tests/run_tests.py --kategori runtime
python tests/run_tests.py --kategori cekirdek
```

---

## 🧩 Editör Desteği (VS Code & Cursor)

Depodaki [`eklenti/`](eklenti/README.md) dizini, VS Code ve Cursor için geliştirilmiş resmi dil eklentisini içerir:

- **Canlı Tanı (Diagnostics):** GCC çağırmadan milisaniyeler içinde çalışan `--analiz` motoruyla anlık hata gösterimi.
- **Otomatik Tamamlama ve İpuçları:** Doğrudan `yerlesik.upp` tablosundan üretilen sözlükle iç içe `upp.*` ad alanı tamamlaması ve tür imzaları.
- **Kısayollar:** **`F5`** ile aktif dosyayı derleyip çalıştırma, **`Ctrl+Shift+B`** ile derleme.

---

## 📂 Proje Dizin Yapısı

```text
upp-lang/
├── bootstrap/              # Aşama-0 C kaynağı (temiz klonda gcc ile derlenir)
├── derleyici/              # Yerel derleyici ikilisi (uppc / uppc.exe)
├── OGREN.md                # Sıfırdan başlayanlar için kapsamlı u++ ders kitabı
├── derleyici.md            # Native derleyici ve çalışma zamanı iç mimarisi
├── YOL_HARITASI.md         # Beta yol haritası ve tamamlanan fazlar
├── docs/                   # Standart kütüphane ve API referansları
│   ├── API_tr.md           # Türkçe API Referansı
│   └── API_en.md           # English API Reference
├── src/
│   ├── runtime/            # C çalışma zamanı katmanları
│   │   ├── upp_cekirdek.c        # libc'siz çekirdek: metin v2, koleksiyonlar, tamponlar
│   │   ├── upp_barindirilan.c    # İşletim sistemi katmanı (dosya, süreç, iş parçacığı, JSON)
│   │   ├── upp_serbest.c         # Çekirdek katmanı (seri/VGA, IDT/PIT, klavye, ATA disk, görevler)
│   │   ├── upp_on_*.c            # Hedef ön dosyaları (yapılandırma ve ayırıcılar)
│   │   └── upp_runtime*.c        # OTOMATİK: kendini-derle tarafından üretilen paketler
│   └── uppc/               # Saf u++ ile yazılmış self-hosted derleyici modülleri
│       ├── yerlesik.upp          # Tüm upp.* yerleşiklerinin tek doğru kaynak tablosu
│       ├── cekirdek.upp          # --hedef cekirdek: gömülü önyükleyici ve araç zinciri
│       ├── asistan.upp           # upp.proje yönetimi ve etkileşimli derleme asistanı
│       └── kendini.upp           # Python'sız 3 aşamalı self-host sistemi
├── src/os/                  # Uppix: u++ ile yazılmış Unix benzeri işletim sistemi (Limine, ext4/FAT32/exFAT)
├── eklenti/                # VS Code & Cursor IDE eklentisi
├── ornekler/               # Örnek u++ programları ve çekirdek örnekleri
└── tests/                  # Pozitif, negatif, güvenlik, runtime ve QEMU çekirdek testleri
```

---

## 📄 Lisans

Bu proje **[Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0)** kapsamında lisanslanmıştır.

---

<br/>

## 🌐 English Summary

### About the Project
**u++** is an open-source, statically-typed systems programming language with **Turkish syntax**. Instead of relying on an interpreter or virtual machine, `u++` transpiles source code into clean **GNU C11 (`-std=gnu11`)** and invokes **GCC** to produce standalone native binaries (`.exe`, Linux ELF, or freestanding x86_64 kernels booted via Multiboot or Limine). Both the hosted and freestanding C runtimes are embedded directly inside the compiler binary.

> 🎓 **Educational & Hobby Scope:** Developed as an in-depth exploration into compiler architecture, Structure-of-Arrays (SoA) AST design, C11 transpilation, reference-counted copy-on-write memory models, and bare-metal OS construction. See **[OGREN.md](OGREN.md)** and **[docs/API_en.md](docs/API_en.md)** for full documentation.

### Key Highlights
- **Memory Safety by Default:** Bounds-checked arrays/slices, checked integer division, and automatic scope-based cleanup. Raw pointers (`*`, `&`), pointer casts, and hardware I/O are gated behind explicit `guvensiz { ... }` blocks.
- **Managed Strings & COW Collections:** `metin` (UTF-8), `liste[T]`, and `harita[K, V]` use atomic reference counting with Copy-on-Write semantics and automatic compiler-injected drops. `&metin` and `&dilim[T]` provide zero-copy views.
- **Value-Based Error Handling & Defer:** `beklenen[T, H]` (flagged union, zero hidden allocation), `?` error propagation, unused-result compile errors, `ertele` (`defer`), and `hata_ertele` (`errdefer`).
- **First-Class Function Values:** `fonk(P...) -> R` types, captureless lambdas (`fonk(sayi x) -> sayi => x * 2`), `(T)x` casts, and compile-time `boyut(T)` (`sizeof`).
- **Literals, Ranges & Text:** character literals (`'a'`, `'ş'`, `'\n'`, `'\u015f'`), range loops (`her (sayi i in 0..n)`), and locale-independent `upp.metin.ingkucuk` / `ingbuyuk`.
- **Low-Level & C Interop:** type-safe C declarations with `c_harici fonk strlen(cmetin s) -> u64;` (`cmetin` for raw C strings), `@paketli` / `@hizali(N)` / `@bolum("name")` layout attributes, and `uppc --guvenlik-denetimi` for ASan + UBSan builds.
- **Freestanding & Kernel Target (`--hedef cekirdek`):** Compiles `u++` programs directly into bootable x86_64 kernels (Multiboot for QEMU `-kernel`, or higher-half ELF64 booted from disk by Limine with `--onyukleyici limine`) with an embedded bootloader, IDT/exceptions, 1000 Hz PIT, PS/2 Turkish-Q keyboard, VGA text or framebuffer console plus COM1, ATA PIO driver, and cooperative multitasking.
- **Uppix Operating System ([`src/os/`](src/os/README.md)):** A Unix-like OS written in pure `u++` that boots from disk in 64-bit mode via the vendored **Limine** bootloader (BIOS and UEFI) and runs user programs in **ring 3** with **Linux x86_64-compatible system calls** (a static `-nostdlib` Linux ELF runs unmodified), featuring read/write **ext4** (with `metadata_csum` CRC32C, verified by `e2fsck`), **FAT32** (VFAT UTF-16 LFN), and **exFAT** filesystems, MBR/GPT partitioning, VFS, the `uppsh` shell with pipes/redirection/jobs, and 74 commands (including a `snake` game).
- **Python-Free Self-Hosting:** `uppc kendini-derle` rebuilds the compiler in ~30 seconds, verifies the 3-stage fixed point (`stage2 == stage3`), and emits a deterministic `bootstrap/uppc.c` across Windows and Linux.

### Quick Start

```bash
git clone https://github.com/ploxdev/upp-lang.git
cd upp-lang

# Bootstrap the native compiler (Windows MinGW / Linux GCC)
gcc -std=gnu11 -O2 bootstrap/uppc.c -o derleyici/uppc.exe -luser32 -lwinmm -lgdi32
# Linux: gcc -std=gnu11 -O2 bootstrap/uppc.c -o derleyici/uppc -pthread -lm

# Compile and run an example
./derleyici/uppc.exe ornekler/ornek1.upp

# Boot the Uppix operating system in QEMU
./derleyici/uppc.exe calistir src/os

# Run the test suite
python tests/run_tests.py
```

Licensed under the **Apache License 2.0**.
