# u++ Native Derleyici Mimarisi (Self-Hosting)

Bu belge, **u++** dilinin kendi kaynak koduyla yazılmış ve kendi kendini derleyebilen (**self-hosting**) yerel derleyicisinin (`src/uppc/`) mimari yapısını, bellek modelini, bootstrap aşamalarını ve tasarım kararlarını detaylandırmaktadır.

---

## 📌 Genel Bakış ve Tasarım Felsefesi

u++ ekosisteminde iki ayrı derleyici uygulaması bulunmaktadır:
1. **Python Referans Derleyicisi (`uppc.py`):** Dilin ilk bootstrap motorudur. Her ortamda harici bir kütüphaneye ihtiyaç duymadan çalışır ve native derleyici için değişmez bir güvenlik ağı sağlar.
2. **Native Derleyici (`derleme/uppc.exe`):** Tamamen saf u++ diliyle yazılmış, C11 transpile adımlarını kendi içinde yürüten ve GCC ile doğrudan native ikili üreten yerel derleyicidir.

### Temel Tasarım İlkeleri:
* **Saf u++ Kaynak Kodu:** Native derleyicinin kaynak kodunda (`src/uppc/`) kesinlikle `c_kod`, `c_ekle` veya doğrudan bellek manipülasyonu yapan `guvensiz` blokları kullanılmaz. Derleyici, dilin standart koleksiyonları (`liste`, `harita`), temel türleri ve nesne modelleri üzerinde saf olarak koşar.
* **Ortak Çalışma Zamanı (Runtime):** Hem Python derleyicisi hem de native derleyici tarafından üretilen kodlar, aynı çalışma zamanı kütüphanesini (`src/runtime/upp_runtime.c`) kullanır.
* **Tam CLI ve Davranış Paritesi:** Native derleyici, Python derleyicisinin sunduğu tüm komut satırı parametrelerini (`--cikti`, `--sadece-derle`, `--analiz`, `--json-hata` vb.) ve hata raporlama formatlarını birebir destekler.

---

## 🛠️ Derleme Hattı (Pipeline) ve Modül Mimarisi

Native derleyici, kaynak koddan C11 çıktısına kadar klasik ve modüler bir boru hattı izler:

```text
Kaynak Kod (.upp) 
       │
       ▼
[ Lexer ] ───> Jetonlar (Tokens) & Kaynak İndeksleri
       │
       ▼
[ Parser ] ───> Soyut Sözdizim Ağacı (AST) [SoA Havuz Düğüm ID'leri]
       │
       ▼
[ Tür Denetimi ] ───> Tip Çıkarımı, Sembol Tabloları & Kapsam Doğrulama
       │
       ▼
[ Güvenlik Denetimi ] ───> Bellek Güvenliği ve `guvensiz` Kapı Kontrolleri
       │
       ▼
[ C Kod Üretimi ] ───> GNU C11 Kod Çıktısı (Tamponlama ve Birleştirme)
       │
       ▼
[ Sürücü / GCC ] ───> Platforma Özgü GCC Çağrısı (.exe / .out)
```

### Modüller ve Sorumluluk Dağılımı (`src/uppc/`)

u++ dilinde modüller derleme öncesinde tek bir kaynakta birleştirildiğinden, her dosya derleme hattının belirli bir katmanından sorumludur:

| Modül | Görev ve Sorumluluk |
|---|---|
| `havuz.upp` | **Veri Mimarisi & SoA Havuz:** Düğüm, jeton, kapsam, dize havuzu (interning) ve tampon depoları. |
| `hata.upp` | **Hata Yönetimi:** İlk hatada durma (fatal error) ve `--analiz` modu için çoklu hata biriktirme tamponu. |
| `ast.upp` | **AST Tanımları:** Jeton türleri (`JetonTur`) ve düğüm türleri (`DugumTur`) sabitleri. |
| `lexer.upp` | **Sözcük Analizi:** Kod noktası (codepoint) tabanlı UTF-8 tarayıcı ve kaynak konumu takibi. |
| `parser.upp` | **Sözdizim Analizi:** Özyinelemeli inişli (recursive-descent) ayrıştırıcı ve AST inşası. |
| `tur.upp` | **Tip Çıkarımı:** Değişken/fonksiyon tip denetimi, sınıf çözümleme ve sembol tabloları. |
| `guvenlik.upp` | **Güvenlik Analizi:** Güvenli/güvensiz blok sınırlarının ve bellek ihlallerinin denetimi. |
| `codegen.upp` | **C11 Üretimi:** AST'den optimize GNU C11 kod üretimi ve çalışma zamanı başlıklarının gömülmesi. |
| `surucu.upp` | **CLI & Sürücü:** Komut satırı argümanlarının ayrıştırılması, GCC çağrısı ve süreç yönetimi. |
| `birlestir.upp` | **Kaynak Birleştirici:** Python'a ihtiyaç duymadan modülleri tek `.upp` haline getiren bağımsız yardımcı araç. |

---

## 🧠 Veri Modeli: Structure of Arrays (SoA) Havuzu

Native derleyicide C benzeri ham işaretçiler (`*`, `&`) yerine, veri odaklı ve bellek dostu **Structure of Arrays (SoA)** mimarisi tercih edilmiştir:

1. **ID Tabanlı Düğüm Mimarisi:**
   - Sözdizim ağacındaki her düğüm bir tam sayıdır (`sayi id`). `id == 0` değeri boş (yok) düğümü temsil eder.
   - Herhangi bir geçersiz ID erişiminde program çökmek (segfault) yerine derhal kontrollü bir `[u++ HATA]` mesajı üretir.
2. **Sütun Tabanlı Depolama:**
   - Düğüm alanları (tür, sol çocuk, sağ çocuk, satır, sütun vb.), `liste[sayi]` ve `liste[metin]` gibi düz dizilerde paralel sütunlar halinde tutulur.
   - Bu sayede önbellek (cache locality) verimliliği artırılır ve dinamik nesne ayırma yükü ortadan kaldırılır.
3. **Dize Havuzu (String Interning):**
   - Tanımlayıcılar ve metin sabitleri için iki yönlü bir haritalama kullanılır:
     - `harita[metin, sayi]` (metin $\rightarrow$ ID)
     - `liste[metin]` (ID $\rightarrow$ metin)
   - Karşılaştırmalar metin yerine doğrudan tamsayı ID'ler üzerinden hızlıca yürütülür.
4. **Sembol Tablosu ve Kapsam:**
   - Kapsam yönetimi dinamik `harita` yapılarıyla modellenmiştir (sabit kapasite sınırlamaları yoktur). Blok derinliklerine göre ad, tür ve kapsam düzeyi sütunlarında tutulur.
5. **Kod Üretim Tamponu:**
   - C kodu üretilirken `tampon_*` metin alanları `metin +=` işlemiyle verimli şekilde büyütülür ve son aşamada diske yazılır.

---

## 🔤 Sözcük Analizi (Lexer) ve UTF-8 Yönetimi

u++, Türkçe sözdizimine ve karakter setine (ç, ğ, ı, ö, ş, ü) tam destek verir:

- **Kod Noktası (Codepoint) İmleci:**
  - Lexer imleci standart C baytları yerine UTF-8 kod noktaları üzerinden ilerler (`lex_pos = cp_i`).
  - Dize dilimleme işlemleri `upp.metin.kes` ile Unicode sınırlarına uygun yapılır.
  - Kod noktası ilerlemeleri için `upp.metin.kod` ve `upp.metin.ileri` yerleşik fonksiyonları kullanılır.
- **Kaynak Haritalama (`@upp-kaynak`):**
  - Modüller `araclar/birlestir.py` ile birleştirildiğinde her dosyanın başına `// @upp-kaynak <modul_adi>` etiketi yerleştirilir.
  - Lexer bu etiketi gördüğünde aktif dosya adını günceller ve satır sayacını sıfırlar; böylece hata bildirimlerinde birleşik dosya değil, orijinal kaynak modülü gösterilir.

---

## 🔁 Bootstrap ve Self-Hosting Döngüsü

Native derleyicinin kendini sıfırdan üretebilmesi ve doğrulanabilmesi için şu aşamalar izlenir:

```powershell
# 1. Aşama: Modülleri tek bir u++ kaynağında birleştir
python araclar/birlestir.py

# 2. Aşama (Aşama 0 -> Aşama 1): Python derleyicisi ile ilk native derleyiciyi (uppc.exe) üret
python uppc.py derleme/uppc_birlesik.upp --sadece-derle --cikti derleme/uppc

# 3. Aşama: Dahili havuz testini ve temel örnekleri doğrula
.\derleme\uppc.exe --havuz-test
.\derleme\uppc.exe ornekler\ornek1.upp --sadece-derle --cikti derleme\ornek1
.\derleme\uppc.exe ornekler\ornek2.upp --sadece-derle --cikti derleme\ornek2

# 4. Aşama (Aşama 1 -> Aşama 2): Üretilen native derleyici ile kendini tekrar derle (uppc1.exe)
.\derleme\uppc.exe derleme\uppc_birlesik.upp --sadece-derle --cikti derleme\uppc1

# 5. Aşama (Aşama 2 -> Aşama 3): İkinci nesil native derleyici ile kendini tekrar derle (uppc2.exe)
.\derleme\uppc1.exe derleme\uppc_birlesik.upp --sadece-derle --cikti derleme\uppc2
```

> 🎯 **Doğrulama (Idempotency):** `uppc1.exe` ile üretilen C kodu ile `uppc2.exe` ile üretilen C kodu birebir özdeş olduğunda, self-hosting sürecinin tam deterministik ve kararlı olduğu kanıtlanmış olur.

---

## 💻 Komut Satırı Arayüzü (CLI) ve IDE Entegrasyonu

Native derleyici, Python derleyicisi ile aynı CLI arabirimine sahiptir:

| Bayrak | Açıklama |
|---|---|
| `--cikti AD` | Üretilecek C ve ikili dosya adı (`AD.c`, `AD.exe` / `AD.out`). |
| `--sadece-derle` | Yalnızca derleme yapar; hedef ikiliyi çalıştırmaz. |
| `--sadece-c` | GCC'yi tetiklemeden yalnızca C11 ara kodunu üretir. |
| `--analiz` | Düzenleyici için canlı tanı modu. Hataları JSON şemasıyla döner. |
| `--json-hata` | Tam derleme (C + GCC) sonucunu tek satır JSON olarak raporlar. |
| `--linux` / `--windows` | Hedef platform ABI seçimi. Verilmezse çalışılan işletim sistemi (host) esas alınır. |
| `--havuz-test` | Dahili bellek ve veri havuzu doğrulama testlerini koşar. |

### IDE Entegrasyonu (LSP & Tanı)
VS Code ve Cursor eklentisinde `upp.derleyici: "native"` ayarlandığında, eklenti canlı tanılar için doğrudan `derleme/uppc.exe --analiz` komutunu çalıştırır. Kod yazılırken GCC çalıştırılmadığı için tip ve sözdizimi denetimi milisaniyeler içinde gerçekleşir.
