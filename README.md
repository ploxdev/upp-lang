<p align="center">
  <img src="https://raw.githubusercontent.com/ploxdev/upp-lang/main/docs/logo.png" alt="u++ logo" width="120" onerror="this.style.display='none'"/>
</p>

<h1 align="center">u++ Programlama Dili</h1>

<p align="center">
  <strong>Türkçe sözdizimli, C11 transpile eden, GCC ile doğrudan native makine kodu üreten, C çalışma zamanını içine gömen (standalone) ve bellek güvenliğini önceleyen modern sistem programlama dili.</strong>
</p>

<p align="center">
  <a href="OGREN.md"><img src="https://img.shields.io/badge/sürüm-v3.0%20beta-007ACC.svg?style=flat-square" alt="Sürüm" /></a>
  <a href="#-lisans"><img src="https://img.shields.io/badge/lisans-Apache%202.0-blue.svg?style=flat-square" alt="Lisans" /></a>
  <img src="https://img.shields.io/badge/platform-Windows%20%7C%20Linux-222222.svg?style=flat-square" alt="Platform" />
  <img src="https://img.shields.io/badge/derleyici-Native%20Self--Hosted%20(Standalone)-success.svg?style=flat-square" alt="Derleyici" />
  <img src="https://img.shields.io/badge/testler-45%2F45%20geçti-brightgreen.svg?style=flat-square" alt="Testler" />
  <a href="eklenti/README.md"><img src="https://img.shields.io/badge/IDE-VS%20Code%20%7C%20Cursor-blueviolet.svg?style=flat-square" alt="IDE Desteği" /></a>
</p>

---

## 📌 Proje Nedir?

**u++**, geliştiricilerin kendi anadilinde (Türkçe) sistem seviyesinde yazılım geliştirmesini sağlayan, yorumlayıcı (interpreter) katmanını aradan çıkararak **doğrudan makine koduna (native binary)** derlenen açık kaynak bir programlama dilidir.

Yüksek seviyeli dillerin okunabilirliğini, C'nin çıplak metal performansıyla buluşturur. Yazdığınız u++ kodu önce temiz ve standart **GNU C11 (`-std=gnu11`)** koduna dönüştürülür (transpile), ardından sistemdeki **GCC** derleyicisi kullanılarak Windows üzerinde bağımsız `.exe`, Linux üzerinde `.out` ikili dosyalarına bağlanır. C çalışma zamanı kütüphanesi derleyici ikilisinin içerisine doğrudan gömülüdür (standalone); harici `.c` veya runtime dosyası taşımaya gerek kalmaz.

### 🎯 Projenin Amacı (Eğitim & Hobi)
Bu proje; derleyici mühendisliği (compiler design), soyut sözdizim ağaçları (AST), transpile teknikleri, dil sunucusu mimarisi (LSP) ve düşük seviye bellek yönetimi konularını derinlemesine anlamak ve deneysel olarak keşfetmek amacıyla **tamamen bir eğitim, araştırma ve hobi projesi** olarak geliştirilmiştir. Kendi kendini derleyebilen (self-hosting) ve gerçek native ikililer üretebilen uçtan uca bağımsız bir sistem dili altyapısı sunar.

> 📚 **Dili Detaylı Öğrenmek İçin:** Sıfırdan başlayanlar için bir kitap/rehber niteliğindeki **[OGREN.md (Öğrenme Rehberi)](OGREN.md)** belgesini inceleyebilirsiniz.

---

## 🚀 Temel Özellikler

- **🇹🇷 Doğal Türkçe Sözdizimi:** `fonk`, `eger`, `dongu`, `sinif`, `don`, `kullan` gibi akıcı, okunaklı ve tutarlı anahtar sözcükler.
- **⚡ Gerçek Native Performans:** Yorumlayıcı ya da VM (sanal makine) yükü yoktur. Doğrudan optimize C11 çıktısı ve GCC derlemesiyle en yüksek çalışma hızı.
- **🛡️ Varsayılan Bellek Güvenliği:** Ham işaretçiler (`*`, `&`), doğrudan bellek manipülasyonu ve harici çekirdek bellek erişimleri (`upp.windows.bellek.*`, `upp.linux.sayi_oku/yaz`) yalnızca açıkça belirtilen `guvensiz { ... }` blokları içerisinde serbesttir.
- **🔄 Standalone Native Derleyici (Self-Hosting):** Tamamen saf u++ diliyle yazılmış, C çalışma zamanını kendi içine gömerek tek ikili (`uppc` / `uppc.exe`) halinde çalışan bağımsız yerel derleyici.
- **🧱 Zengin Tip Sistemi:** `sayi` (64-bit int), `ondalik` (double), `bayt` (uint8_t), `metin` (string), `mantik` (bool), `liste[T]` (`sil`, `temizle`), `harita[K,V]` (`sil`, `temizle`), `dizi[T, N]` ve `sinif`.
- **📦 Zengin Standart Kütüphane (`upp.*`):**
  - Dosya & Klasör I/O (`upp.dosya`, `upp.yol`, `upp.dosya.boyut`)
  - Gelişmiş Metin ve UTF-8 İşlemleri (`upp.metin`, `upp.metin.hex_coz`)
  - Sistem & Süreç Yönetimi (`upp.sistem.calistir`, `SurecCikti.basarili`, `upp.sistem.pid`, `upp.platform`)
  - Platforma Özgü API'ler (`upp.windows.*` ve `upp.linux.*`)
  - JSON Ayrıştırma ve Yönetimi (`upp.json`)
  - Milisaniyelik Hassas Zaman ve Matematik (`upp.zaman`, `upp.matematik`)
  - Çoklu İş Parçacığı & Senkronizasyon (`arkaplan { }`, `Kilit`)
- **🧩 Tam IDE & LSP Entegrasyonu:** VS Code ve Cursor için geliştirilmiş resmi eklenti; GCC çağırmadan milisaniyeler içinde çalışan canlı sözdizimi analizi (`--analiz`), akıllı kod tamamlama, hover ipuçları ve `F5` ile doğrudan derle/çalıştır desteği sunar.

---

## 🛠️ Mimari ve Çalışma Mantığı

u++ derleme hattı, kaynak koddan çalıştırılabilir ikiliye kadar modüler bir hiyerarşiyle işler:

```text
┌─────────────────┐     ┌──────────────────┐     ┌─────────────────────┐
│  Kaynak (.upp)  │ ──> │ Lexer & Parser   │ ──> │ Tip & Güvenlik Den. │
└─────────────────┘     └──────────────────┘     └──────────┬──────────┘
                                                            │
┌─────────────────┐     ┌──────────────────┐                ▼
│ Native (.exe)   │ <── │ GCC Bağlama      │ <── ┌─────────────────────┐
│ Linux (.out)    │     │ (-std=gnu11)     │     │ C11 (Gömülü Runtime)│
└─────────────────┘     └──────────────────┘     └─────────────────────┘
```

1. **Ayrıştırma (Frontend):** Kaynak kod taranır, AST (Soyut Sözdizim Ağacı) inşa edilir.
2. **Denetim Katmanı:** Çok seviyeli ad alanı ve tür kontrolü yapılır, ardından bellek güvenlik analizcisi yetkisiz işaretçi veya sistem çağrılarını denetler.
3. **C Kod Üretimi (Backend):** AST, optimize C11 eşdeğerine dönüştürülür ve gömülü çalışma zamanı (embedded runtime) modülü ile birleştirilir.
4. **Bağlama (Linker):** Platforma uygun bayraklarla GCC tetiklenir ve hedef makine ikilisi üretilir.

---

## 📦 Gereksinimler ve Kurulum

### Sistem Gereksinimleri
- **GCC Derleyicisi:**
  - **Windows:** [MinGW-w64](https://www.msys2.org/) (MSYS2 `ucrt64` veya `mingw64` ortamı, PATH'e ekli olmalıdır)
  - **Linux:** `build-essential` (`sudo apt install build-essential`)
- **Python 3.8+** (Opsiyonel: Yalnızca VS Code LSP dil sunucusu ve test runner için; derleyici bağımsız çalışır)

### Kurulum

Depoyu klonlayın:

```bash
git clone https://github.com/ploxdev/upp-lang.git
cd upp-lang
```

---

## 💻 Hızlı Başlangıç & Kullanım

### 1. Merhaba Dünya

Bir `merhaba.upp` dosyası oluşturun:

```text
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

# Yalnızca üretilen C11 transpile kodunu incele (GCC çağrılmaz):
./derleyici/uppc.exe merhaba.upp -c -o merhaba.c

# Sürüm ve Yardım:
./derleyici/uppc.exe -v
./derleyici/uppc.exe -h
```

---

## 🔍 Dil Sözdiziminden Örnekler

### Değişkenler, Türler ve İnterpolasyon
```text
fonk ana() -> sayi {
    oto isim = "Arda";
    sayi yas = 24;
    bayt seviye = 255;
    ondalik pi = 3.1415;
    mantik aktif = dogru;

    // Doğrudan süslü parantez ile metin interpolasyonu:
    upp.satir_yaz("Kullanıcı: {isim}, Yaş: {yas}, Seviye: {seviye}, Aktif: {aktif}");
    don 0;
}
```

### Dinamik Koleksiyonlar (`liste` ve `harita`)
```text
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
    puanlar.sil("Mehmet");
    upp.satir_yaz("Ahmet'in puanı: {puanlar.al(\"Ahmet\")}");
    don 0;
}
```

### Nesne Yönelim (Sınıflar)
```text
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

### Çoklu İş Parçacığı (Eşzamanlılık) & Senkronizasyon
```text
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
```text
fonk ana() -> sayi {
    sayi x = 100;

    // Güvenli alanda işaretçi kullanımı derleme hatasıdır!
    guvensiz {
        sayi* p = &x;
        *p = 200;
        upp.satir_yaz("İşaretçi üzerinden yeni değer: {x}");
    }

    don 0;
}
```

---

## ⚙️ Native Self-Hosted Derleyici

u++, tamamen kendi kaynak koduyla derlenebilen bağımsız bir native derleyiciye sahiptir:

```bash
# 1. Gömülü çalışma zamanını güncelle ve modülleri birleştir:
py araclar/birlestir.py

# 2. Mevcut native derleyici ile yeni C kodunu transpile et ve GCC ile derle:
./derleyici/uppc.exe derleyici/uppc_birlesik.upp -c -o derleyici/uppc_yeni.c
gcc -std=gnu11 -O2 derleyici/uppc_yeni.c -o derleyici/uppc.exe -luser32 -lwinmm -lgdi32
```

---

## 🧩 Editör Desteği (VS Code & Cursor)

Proje, geliştirme deneyimini üst düzeye taşımak için özel bir IDE eklentisi (`eklenti/`) içerir:

- **Canlı Tanı (Diagnostics):** Kod yazıldığı anda hata tespiti (`--analiz` motoru).
- **IntelliSense & Snippets:** Sözdizimi renklendirme, `upp.*` fonksiyonları ve döngü şablonları.
- **Kısayollar:** **`F5`** ile doğrudan derleyip çalıştırma, **`Ctrl+Shift+B`** ile derleme.

Kurulum detayları için [eklenti/README.md](eklenti/README.md) dosyasını inceleyebilirsiniz.

---

## 🧪 Testler ve Kalite Güvencesi

u++, native derleyici üzerinden çalışan kapsamlı bir test paketiyle sürekli doğrulanır:

```bash
# Tüm testleri yerel derleyiciyle koştur:
python tests/run_tests.py

# Kategoriye göre test koşumu:
python tests/run_tests.py --kategori positive   # Başarılı E2E senaryoları
python tests/run_tests.py --kategori safety     # Bellek güvenliği ihlal testleri
python tests/run_tests.py --kategori negative   # Derleme hatası yakalama testleri
```

---

## 📂 Proje Dizin Yapısı

```text
upp-lang/
├── derleyici/              # Native derleyici (uppc.exe) ve birleşik kaynak
├── derleme/                # Kullanıcı programlarının .c / .exe çıktıları
├── OGREN.md                # Sıfırdan başlayanlar için kapsamlı programlama kitabı
├── derleyici.md            # Native derleyici mimari dokümantasyonu
├── docs/                   # Ayrıntılı API referansları (TR / EN)
│   ├── API_tr.md
│   └── API_en.md
├── src/
│   ├── runtime/            # C çalışma zamanı kütüphanesi (upp_runtime.c)
│   └── uppc/               # Native derleyici kaynak kodları (saf u++)
│       └── gomulu_runtime.upp # Derleyiciye gömülü C çalışma zamanı
├── eklenti/                # VS Code & Cursor IDE eklentisi (TypeScript + Python LSP)
├── araclar/                # Derleyici derleme ve birleştirme yardımcı araçları
├── ornekler/               # Dil yeteneklerini sergileyen örnek kodlar (.upp, .uph)
└── tests/                  # Entegrasyon, güvenlik ve negatif test paketi
```

---

## 📄 Lisans

Bu proje **[Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0)** kapsamında lisanslanmıştır.

---

<br/>

## 🌐 English Summary

### About the Project
**u++** is an open-source, statically-typed systems programming language with **Turkish syntax**. It combines expressive, readable syntax with the raw performance of native binaries. 

Instead of relying on an interpreter or virtual machine, u++ transpiles source code into clean, standard **GNU C11 (`-std=gnu11`)** and uses **GCC** to produce standalone executables (`.exe` / `.out`). The C runtime is directly embedded into the native compiler binary, removing the need for external runtime files.

> 🎓 **Educational & Hobby Scope:** This project was developed purely as an educational research and hobby exploration into compiler design, AST construction, transpilation, Language Server Protocols (LSP), and native runtime systems. For a full tutorial, please refer to **[OGREN.md](OGREN.md)**.

### Key Features
- **Native Performance:** Zero interpreter overhead; direct compilation to machine code via C11 and GCC.
- **Embedded Runtime:** The native compiler embeds its C runtime directly; zero external runtime file dependencies.
- **Memory Safety by Default:** Raw pointers, manual memory allocations, and foreign process memory accesses are strictly prohibited outside explicit `guvensiz { ... }` blocks.
- **Self-Hosting Native Compiler:** The compiler (`uppc` / `uppc.exe`) is written purely in u++ and compiles itself.
- **Rich Standard Library (`upp.*`):** Modules for high-performance I/O, UTF-8 strings, JSON, process execution (`upp.sistem.calistir`), threading (`arkaplan` / `Kilit`), and platform-specific namespaces (`upp.windows.*` and `upp.linux.*`).

### Quick Start

```bash
# Clone the repository
git clone https://github.com/ploxdev/upp-lang.git
cd upp-lang

# Compile and run an example using the native compiler
./derleyici/uppc.exe ornekler/ornek1.upp

# Compile to standalone binary without running
./derleyici/uppc.exe ornekler/ornek1.upp -o ornek1 --sadece-derle

# Run the test suite
python tests/run_tests.py
```

### Hello World in u++
```text
fonk ana() -> sayi {
    upp.satir_yaz("Hello, World from u++!");
    don 0;
}
```

Licensed under the **Apache License 2.0**.
