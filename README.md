# u++

> **Türkçe kaynak kodlu, C11 transpile eden, GCC ile native ikili üreten açık kaynak programlama dili.**

u++ ile `fonk ana()` yazarsınız; derleyici **GNU C11** (`-std=gnu11`) üretir, **GCC** bağlar. Windows’ta `.exe`, Linux’ta `.out` çıkar. Bellek güvenliği varsayılan; `guvensiz { }` ile işaretçi ve süreç belleği açılır.

[![Sürüm](https://img.shields.io/badge/sürüm-v3.0-blue)](OGREN.md)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey)](#gereksinimler)
[![Derleyici](https://img.shields.io/badge/derleyici-Python%20%2B%20native-green)](#iki-derleyici-tek-dil)
[![Testler](https://img.shields.io/badge/testler-62%2B139%20geçti-brightgreen)](#testler)

---

## Neden u++?

| Özellik | Açıklama |
|---|---|
| **Türkçe sözdizimi** | `fonk`, `eger`, `dongu`, `sinif`, `don` — öğrenmesi kolay, okunaklı kaynak |
| **Gerçek native çıktı** | Ara C + GCC; yorumlayıcı değil, `.exe` / `.out` ikili |
| **Bellek güvenliği** | Güvenli modda işaretçi, `c_kod` ve `upp.bellek` yasak; `guvensiz` kapısı |
| **Zengin standart kitaplık** | `upp.*` — dosya, yol, metin, JSON, iş parçacığı, matematik |
| **İki derleyici** | `uppc.py` (bootstrap) ve `uppc.exe` (native, kendi dilinde yazılmış) |
| **VS Code / Cursor** | LSP tanısı, tamamlama, F5 derle/çalıştır — [eklenti](eklenti/README.md) |

---

## Hızlı başlangıç

### Gereksinimler

- [Python 3.8+](https://www.python.org/) (ek paket yok)
- [GCC](https://gcc.gnu.org/) — Windows: [MinGW-w64](https://www.msys2.org/) (`C:\msys64\ucrt64\bin`)

### Kurulum

Depoyu klonlayın; ek kurulum adımı yoktur:

```bash
git clone <repo-url> uplusplus
cd uplusplus
```

### İlk derleme

```bash
# Derle ve çalıştır
python uppc.py ornekler/ornek1.upp

# Derle, çalıştırma — çıktı: ornek1.c + ornek1.exe
python uppc.py ornekler/ornek1.upp --cikti ornek1 --sadece-derle
```

### İlk program

```text
fonk ana() -> sayi {
    upp.satir_yaz("Merhaba, u++!");
    don 0;
}
```

---

## Örnekler

| Dosya | Konu |
|---|---|
| [`ornekler/ornek1.upp`](ornekler/ornek1.upp) | Temel sözdizimi, `upp.*` |
| [`ornekler/ornek2.upp`](ornekler/ornek2.upp) | Sınıf, `secenek`, `arkaplan` |
| [`ornekler/ornek3.upp`](ornekler/ornek3.upp) | `kullan`, `sabit`, `+=` |
| [`ornekler/kutuphane.uph`](ornekler/kutuphane.uph) | Başlık dosyası (`.uph`) |
| [`ornekler/ornek_json.upp`](ornekler/ornek_json.upp) | JSON, `upp.stdio` |

```bash
python uppc.py ornekler/ornek2.upp --cikti ornek2 --sadece-derle
python uppc.py ornekler/ornek_json.upp --cikti ornek_json --sadece-derle
```

---

## İki derleyici, tek dil

| | **Python** (`uppc.py`) | **Native** (`derleme/uppc.exe`) |
|---|---|---|
| Konum | Kök dizin | `derleme/` |
| Kaynak | Python | `src/uppc/` (saf u++; `c_kod` yok) |
| Rol | Bootstrap; her zaman çalışır | Aynı bayraklar, daha hızlı |
| Kullanım | Varsayılan | Üretildikten sonra; eklentide `upp.derleyici: native` |

Native derleyiciyi üretmek için (bir kez):

```bash
python araclar/birlestir.py
python uppc.py derleme/uppc_birlesik.upp --sadece-derle --cikti derleme/uppc
```

Derleyici kaynağı `c_kod` kullanmaz; dilin `guvensiz` / `c_kod` kaçışı kullanıcı programlarında durur.

Sonra:

```bash
derleme/uppc.exe ornekler/ornek1.upp --sadece-derle --cikti derleme/ornek1
derleme/uppc.exe ornekler/ornek3.upp --analiz
```

> **Windows notu:** Taze üretilen `.exe` bazen Uygulama Denetimi tarafından kesilir (`WinError 4551`). Bu derleyici hatası değil; Python derleyici etkilenmez.

---

## Komut satırı

Python ve native **aynı bayrakları** konuşur.

| Bayrak | Açıklama |
|---|---|
| `--cikti AD` | `AD.c` + `AD.exe` / `AD.out`. Yoksa `cikti.c` + `program.exe` |
| `--sadece-derle` | İkili üretir, çalıştırmaz |
| `--sadece-c` | Yalnızca C üretir; GCC çağrılmaz |
| `--linux` / `--windows` | Hedef ABI. İkisi birden verilemez; yoksa host |
| `--analiz` | Lex / parse / tür / güvenlik → JSON. C ve GCC yok (düzenleyici) |
| `--json-hata` | Tam derleme (C + GCC) → tek satır JSON |
| `--bicim` | Kaynağı yerinde biçimler (girinti 4) |
| `--bicim-kontrol` | Biçimsizse çıkış kodu 1 (CI) |
| `--ast` | Ayrıştırılmış ağacı basar |

```bash
# Canlı tanı (eklenti ile aynı JSON şeması)
python uppc.py ornekler/ornek3.upp --analiz

# Linux hedefi (Windows’ta .out üretir, çalıştırılmaz)
python uppc.py ornekler/ornek1.upp --linux --sadece-derle --cikti ornek1
```

---

## Dil özeti (v3.0)

```text
// Giriş noktası
fonk ana() -> sayi { don 0; }

// Modüller
kullan "kutuphane.uph";
kullan "yardim.uph" olarak y;

// Koleksiyonlar
liste[sayi] xs;
harita[metin, sayi] skor;

// İş parçacığı
oto is = arkaplan { upp.satir_yaz("arka plan"); };
is.bekle();

// Süreç
SurecCikti r = upp.sistem.calistir("echo tamam");
```

| Alan | Öne çıkanlar |
|---|---|
| **Türler** | `sayi`, `ondalik`, `mantik`, `metin`, `liste[T]`, `harita[K,V]`, sınıf, `secenek` |
| **Metin** | UTF-8, `{ad}` interpolasyon, `s[i]` kod noktası, `metin +=` |
| **Kontrol** | `eger` / `iken` / `dongu` / `her` / `secim` (sayı ve metin) |
| **Modüller** | `kullan` / `.uph` başlık / `olarak` önek |
| **Güvenlik** | `guvensiz { }` — işaretçi, `c_kod`, `upp.bellek` |
| **v3.0 yenilikleri** | `Kilit`, `--analiz` çok hata, `--bicim`, `SurecCikti`, `upp.yol` |

Tam sözleşme: **[OGREN.md](OGREN.md)** · Kamu API: **[docs/API_tr.md](docs/API_tr.md)** · English: **[docs/API_en.md](docs/API_en.md)**

---

## VS Code / Cursor eklentisi

Yerel `.vsix` ile kurulur; Marketplace gerekmez.

- Sözdizimi vurgulama (`.upp` + `.uph`)
- Canlı LSP tanısı (`--analiz`)
- Tamamlama, hover, tanıma git
- **F5** derle + çalıştır · **Ctrl+Shift+B** derle

Kurulum ve ayarlar: **[eklenti/README.md](eklenti/README.md)**

---

## Testler

```bash
python tests/run_tests.py          # 62 entegrasyon (positive / negative / safety / parite)
python tests/test_uppc.py          # 139 birim testi
python tests/run_tests.py --kategori positive
```

Kategoriler: `positive/` (derle + stdout), `negative/` (derleme hatası), `safety/` (bellek güvenliği ihlali).

---

## Proje yapısı

```text
u++/
├── uppc.py                 # Python derleyici (bootstrap)
├── OGREN.md                # Dil sözleşmesi (TR)
├── docs/
│   ├── API_tr.md           # Kamu API (TR)
│   └── API_en.md           # Public API (EN)
├── derleyici.md            # Native derleyici mimarisi
├── eklenti/                # VS Code / Cursor eklentisi
├── src/
│   ├── runtime/            # C çalışma zamanı (upp.*)
│   └── uppc/               # Native derleyici kaynağı (u++)
├── araclar/birlestir.py    # Native birleştirme
├── derleme/                # uppc.exe, ara C dosyaları
├── ornekler/               # Örnek programlar
└── tests/                  # Test paketi
```

---

## Gereksinimler

| Bileşen | Windows | Linux |
|---|---|---|
| Python | 3.8+ | 3.8+ |
| GCC | MinGW-w64 (MSYS2) | `build-essential` |
| Bağlama | `-luser32 -lwinmm -lgdi32` | `-pthread -lm` |

**Platform farkları:** Linux’ta pencere üzerine çizim, sentetik girdi ve yabancı süreç belleği yoktur. Çağrılar patlamaz; `0` döner veya no-op olur.

---

## Belgeler

| Belge | Ne için? |
|---|---|
| [OGREN.md](OGREN.md) | Dil öğrenmek, sözleşmeyi anlamak |
| [docs/API_tr.md](docs/API_tr.md) | `upp.*` referansı (TR) |
| [docs/API_en.md](docs/API_en.md) | `upp.*` reference (EN) |
| [derleyici.md](derleyici.md) | Native derleyici iç yapısı |
| [eklenti/README.md](eklenti/README.md) | Düzenleyici kurulumu |

---

## Sürüm geçmişi (kısa)

| Sürüm | Öne çıkanlar |
|---|---|
| **v3.0** | `Kilit`, `--analiz` çok hata, `--bicim`, `SurecCikti`, `upp.yol` |
| **v2.9** | `secim(metin)`, kurucu şekeri, `kullan … olarak`, varsayılan parametre |
| **v2.8** | UTF-8 `s[i]`, `metin +=`, üst düzey değişken, `bol`/`birlestir`, `her(harita)` |

Semver kırıkları (v2.8): `s[i]` artık kod noktası; `upp.sistem.calistir` → `SurecCikti`.

---

## Anahtar kelimeler

`u++` · `uplusplus` · `Türkçe programlama dili` · `Turkish programming language` · `transpiler` · `C11` · `GCC` · `MinGW` · `self-hosting compiler` · `memory safety` · `uppc` · `sistem programlama`

---

*Son güncelleme: u++ v3.0*
