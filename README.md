<p align="center">
  <img src="https://raw.githubusercontent.com/ploxdev/upp-lang/main/docs/logo.png" alt="u++ logo" width="120" onerror="this.style.display='none'"/>
</p>

<h1 align="center">u++ Programlama Dili</h1>

<p align="center">
  <strong>Türkçe sözdizimli, C11 transpile eden, GCC ile doğrudan native makine kodu üreten ve bellek güvenliğini önceleyen modern sistem programlama dili.</strong>
</p>

<p align="center">
  <a href="OGREN.md"><img src="https://img.shields.io/badge/sürüm-v3.0-007ACC.svg?style=flat-square" alt="Sürüm" /></a>
  <a href="#-lisans"><img src="https://img.shields.io/badge/lisans-Apache%202.0-blue.svg?style=flat-square" alt="Lisans" /></a>
  <img src="https://img.shields.io/badge/platform-Windows%20%7C%20Linux-222222.svg?style=flat-square" alt="Platform" />
  <img src="https://img.shields.io/badge/derleyici-Python%20%2B%20Native%20(Self--Hosting)-success.svg?style=flat-square" alt="Derleyici" />
  <img src="https://img.shields.io/badge/testler-200%2B%20geçti-brightgreen.svg?style=flat-square" alt="Testler" />
  <a href="eklenti/README.md"><img src="https://img.shields.io/badge/IDE-VS%20Code%20%7C%20Cursor-blueviolet.svg?style=flat-square" alt="IDE Desteği" /></a>
</p>

---

## 📌 Proje Nedir?

**u++**, geliştiricilerin Türkçe olarak sistem seviyesinde yazılım geliştirmesini sağlayan, yorumlayıcı (interpreter) katmanı OLMADAN **doğrudan makine koduna (native binary)** derlenen açık kaynak bir programlama dilidir.

Yüksek seviyeli dillerin okunabilirliğini, C'nin çıplak metal performansıyla buluşturur. Yazdığınız u++ kodu önce temiz ve standart **GNU C11 (`-std=gnu11`)** koduna dönüştürülür (transpile), ardından sistemdeki **GCC** derleyicisi kullanılarak Windows üzerinde bağımsız `.exe`, Linux üzerinde `.out` ikili dosyalarına bağlanır. C çalışma zamanı kütüphanesi derleme sırasında doğrudan ikiliye gömülür; harici `.dll` veya runtime bağımlılığı gerektirmez.

### 🎯 Projenin Amacı (Eğitim & Hobi)
Bu proje; derleyici mühendisliği (compiler design), soyut sözdizim ağaçları (AST), transpile teknikleri, dil sunucusu mimarisi (LSP) ve düşük seviye bellek yönetimi konularını derinlemesine anlamak ve deneysel olarak keşfetmek amacıyla **tamamen bir eğitim, araştırma ve hobi projesi** olarak geliştirilmiştir. Ticari bir iddia taşımamakla birlikte, kendi kendini derleyebilen (self-hosting) ve gerçek native ikililer üretebilen uçtan uca eksiksiz bir sistem dili altyapısı sunar.

> 📚 **Dili Detaylı Öğrenmek İçin:** Sözdizimi kuralları, tüm anahtar sözcükler, tür kuralları ve standart kitaplık sözleşmesinin yer aldığı kapsamlı rehber için **[OGREN.md (Dil Sözleşmesi)](OGREN.md)** belgesini inceleyebilirsiniz.

---

## 🚀 Temel Özellikler

- **🇹🇷 Doğal Türkçe Sözdizimi:** `fonk`, `eger`, `dongu`, `sinif`, `don`, `kullan` gibi akıcı, okunaklı ve tutarlı anahtar sözcükler.
- **⚡ Gerçek Native Performans:** Yorumlayıcı ya da VM (sanal makine) yükü yoktur. Doğrudan optimize C11 çıktısı ve GCC derlemesiyle en yüksek çalışma hızı.
- **🛡️ Varsayılan Bellek Güvenliği:** Ham işaretçiler (`*`, `&`), doğrudan bellek manipülasyonu ve harici C blokları yalnızca açıkça belirtilen `guvensiz { ... }` blokları içerisinde serbesttir. Güvenli kodda bellek ihlallerine derleme anında geçit verilmez.
- **🔄 İki Derleyici & Self-Hosting:**
  - **`uppc.py`:** Sıfır dış bağımlılıkla her ortamda çalışan Python bootstrap derleyicisi.
  - **`derleme/uppc.exe`:** Tamamen saf u++ diliyle yazılmış, kendi kendini derleyebilen (self-hosting) native derleyici.
- **📦 Zengin Standart Kütüphane (`upp.*`):**
  - Dosya & Klasör I/O (`upp.dosya`, `upp.yol`)
  - Gelişmiş Metin ve UTF-8 İşlemleri (`upp.metin`)
  - Süreç Yönetimi (`upp.sistem.calistir` & `SurecCikti`)
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
│ Linux (.out)    │     │ (-std=gnu11)     │     │ C11 Kod Üretimi     │
└─────────────────┘     └──────────────────┘     └─────────────────────┘
```

1. **Ayrıştırma (Frontend):** Kaynak kod taranır, AST (Soyut Sözdizim Ağacı) inşa edilir.
2. **Denetim Katmanı:** Tür kontrolü yapılır, ardından bellek güvenlik analizcisi yetkisiz işaretçi veya sistem çağrılarını denetler.
3. **C Kod Üretimi (Backend):** AST, optimize C11 eşdeğerine dönüştürülür ve yerleşik `src/runtime/upp_runtime.c` modülü ile birleştirilir.
4. **Bağlama (Linker):** Platforma uygun bayraklarla GCC tetiklenir ve hedef makine ikilisi üretilir.

---

## 📦 Gereksinimler ve Kurulum

### Sistem Gereksinimleri
- **Python 3.8+** (Bootstrap derleyici ve LSP sunucusu için; ek bir pip paketi gerekmez)
- **GCC Derleyicisi:**
  - **Windows:** [MinGW-w64](https://www.msys2.org/) (Örn: MSYS2 `ucrt64` veya `mingw64` ortamı, PATH'e ekli olmalıdır)
  - **Linux:** `build-essential` (`sudo apt install build-essential`)

### Kurulum

Depoyu doğrudan klonlayın, ek bir paket yükleme adımı yoktur:

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
python uppc.py merhaba.upp

# Yalnızca çalıştırılabilir ikiliyi (.exe / .out) üret:
python uppc.py merhaba.upp --cikti merhaba --sadece-derle

# Yalnızca üretilen C11 transpile kodunu incele (GCC çağrılmaz):
python uppc.py merhaba.upp --sadece-c

# IDE / Canlı Tanı modu (AST, tür ve güvenlik kontrolü yapar, JSON döner):
python uppc.py merhaba.upp --analiz
```

---

## 🔍 Dil Sözdiziminden Örnekler

### Değişkenler, Türler ve İnterpolasyon
```text
fonk ana() -> sayi {
    oto isim = "Arda";
    sayi yas = 24;
    ondalik pi = 3.1415;
    mantik aktif = dogru;

    // Doğrudan süslü parantez ile metin interpolasyonu:
    upp.satir_yaz("Kullanıcı: {isim}, Yaş: {yas}, Aktif: {aktif}");
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

## ⚙️ İki Derleyici: Native Self-Hosting

u++, bootstrap döngüsünü tamamlamış self-hosting bir mimariye sahiptir:

| Derleyici | Konum | Altyapı | Rol |
|---|---|---|---|
| **Python Derleyicisi** | `uppc.py` | Python 3 | Referans derleyici, geliştirme ve bootstrap |
| **Native Derleyici** | `derleme/uppc.exe` | Saf u++ (`src/uppc/`) | Yüksek hızlı, bağımsız yerel derleyici |

Native derleyiciyi üretmek için:

```bash
# 1. Modülleri tek bir u++ kaynağında birleştir:
python araclar/birlestir.py

# 2. Python derleyicisi ile native binary'yi üret:
python uppc.py derleme/uppc_birlesik.upp --sadece-derle --cikti derleme/uppc

# 3. Artık derleme işlemlerinde doğrudan native derleyiciyi kullanabilirsiniz:
./derleme/uppc.exe ornekler/ornek1.upp --cikti ornek1
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

u++, kapsamlı bir test paketiyle sürekli doğrulanır:

```bash
# Tüm entegrasyon ve parite testlerini çalıştır:
python tests/run_tests.py

# Yalnızca birim (unit) testlerini çalıştır (139 test):
python tests/test_uppc.py

# Kategoriye göre test koşumu:
python tests/run_tests.py --kategori positive   # Başarılı E2E senaryoları
python tests/run_tests.py --kategori safety     # Bellek güvenliği ihlal testleri
python tests/run_tests.py --kategori negative   # Derleme hatası yakalama testleri
```

---

## 📂 Proje Dizin Yapısı

```text
upp-lang/
├── uppc.py                 # Python bootstrap derleyicisi (CLI & transpile motoru)
├── OGREN.md                # Kapsamlı dil spesifikasyonu ve başvuru kılavuzu
├── derleyici.md            # Native derleyici mimari dokümantasyonu
├── docs/                   # Ayrıntılı API referansları (TR / EN)
│   ├── API_tr.md
│   └── API_en.md
├── src/
│   ├── runtime/            # C çalışma zamanı kütüphanesi (upp_runtime.c)
│   └── uppc/               # Native derleyici kaynak kodları (saf u++)
├── eklenti/                # VS Code & Cursor IDE eklentisi (TypeScript + Python LSP)
├── araclar/                # Derleyici derleme ve birleştirme yardımcı araçları
├── ornekler/               # Dil yeteneklerini sergileyen örnek kodlar (.upp, .uph)
└── tests/                  # Entegrasyon, güvenlik, parite ve birim test paketi
```

---

## 📄 Lisans

Bu proje **[Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0)** kapsamında lisanslanmıştır. Açık kaynak standartlarına uygun olarak serbestçe incelenebilir, kullanılabilir ve geliştirilebilir.

---

<br/>

## 🌐 English Summary

### About the Project
**u++** is an open-source, statically-typed systems programming language with **Turkish syntax**. It is designed to combine expressive, readable syntax with the raw performance of native binaries. 

Instead of relying on an interpreter or virtual machine, u++ transpiles source code into clean, standard **GNU C11 (`-std=gnu11`)** and uses **GCC** (MinGW-w64 on Windows, native GCC on Linux) to produce standalone executables (`.exe` / `.out`).

> 🎓 **Educational & Hobby Scope:** This project was developed purely as an educational research and hobby exploration into compiler design, AST construction, transpilation, Language Server Protocols (LSP), and native runtime systems. For full language specifications and semantics, please refer to **[OGREN.md](OGREN.md)**.

### Key Features
- **Native Performance:** Zero interpreter overhead; direct compilation to machine code via C11 and GCC.
- **Memory Safety by Default:** Raw pointers, manual memory management, inline C code, and foreign process access are strictly prohibited unless wrapped in an explicit `guvensiz { ... }` (unsafe) block.
- **Self-Hosting Dual Compiler:**
  - `uppc.py`: Zero-dependency Python 3 bootstrap compiler.
  - `derleme/uppc.exe`: Fast native compiler written purely in u++.
- **Comprehensive Standard Library (`upp.*`):** Built-in modules for high-performance I/O, UTF-8 strings, JSON, processes, threading (`arkaplan` / `Kilit`), and math.
- **Modern Tooling & IDE Support:** Official VS Code and Cursor extension featuring a full Language Server Protocol (LSP) implementation, real-time diagnostics (`--analiz`), autocompletion, and one-click build & run (`F5`).

### Quick Start

```bash
# Clone the repository
git clone https://github.com/ploxdev/upp-lang.git
cd upp-lang

# Compile and run an example
python uppc.py ornekler/ornek1.upp

# Compile to standalone binary without running
python uppc.py ornekler/ornek1.upp --cikti ornek1 --sadece-derle

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

Licensed under the **Apache License 2.0**. Contributions and feedback are welcome!
