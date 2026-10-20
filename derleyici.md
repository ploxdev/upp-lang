# u++ Native Derleyici Mimarisi (Self-Hosting & Standalone)

Bu belge, **u++** dilinin kendi kaynak koduyla yazılmış, çalışma zamanını kendi içine gömen (**standalone binary**) ve kendi kendini derleyebilen (**self-hosting**) yerel derleyicisinin (`src/uppc/`) mimari yapısını, bellek modelini, çalışma mantığını ve tasarım kararlarını detaylandırmaktadır.

---

## 📌 Genel Bakış ve Tasarım Felsefesi

u++ dili (v3.0+), harici betik dillerine (Python vb.) olan tüm bağımlılıklarını geride bırakmış ve **tamamen bağımsız, yerel (native) bir derleyiciye** kavuşmuştur:

* **Tek ve Standalone İkili (`uppc` / `uppc.exe`):** Derleyici, C çalışma zamanı kütüphanesini (`src/runtime/upp_runtime.c`) kendi içine gömülü (embedded) olarak taşır. Derleme esnasında diskte `.c` çalışma zamanı dosyası aramaz; tek başına taşınabilir ve çalıştırılabilir.
* **Saf u++ Kaynak Kodu:** Native derleyicinin kaynak kodunda (`src/uppc/`) ham işaretçiler (`*`, `&`) veya `guvensiz` blokları kullanılmaz. Derleyici, dilin standart koleksiyonları (`liste`, `harita`), temel türleri ve nesne modelleri üzerinde güvenli olarak koşar.
* **Transpile & GCC Motoru:** u++ kaynak kodunu yüksek başarımlı ve taşınabilir GNU C11 koduna dönüştürür, ardından sistemdeki GCC derleyicisini otomatik olarak invoke ederek doğrudan yerel makine ikilisi üretir.
* **Gelişmiş CLI Deneyimi:** `-o` / `--cikti`, `-c` / `--sadece-c`, `--sadece-derle`, `--runtime`, `-v` / `--surum`, `-h` / `--yardim`, `--analiz`, `--json-hata` bayraklarını destekler.

---

## 🛠️ Derleme Hattı (Pipeline) ve Modül Mimarisi

Native derleyici, kaynak koddan C11 çıktısına kadar klasik ve modüler bir boru hattı izler:

```text
Kaynak Kod (.upp) 
       │
       ▼
[ Lexer ] ────────────> Jetonlar (Tokens) & Kaynak İndeksleri
       │
       ▼
[ Parser ] ───────────> Soyut Sözdizim Ağacı (AST) [SoA Havuz Düğüm ID'leri]
       │
       ▼
[ Tür Denetimi ] ─────> Çok Seviyeli Ad Alanı, Tip Çıkarımı, Sembol Tabloları & Kapsam Doğrulama
       │
       ▼
[ Güvenlik Denetimi ] ─> Bellek Güvenliği, `guvensiz` Kapı Kontrolleri (upp.windows.bellek / upp.linux)
       │
       ▼
[ C Kod Üretimi ] ────> GNU C11 Kod Çıktısı (Gömülü Runtime Enjeksiyonu + Kullanıcı Kodu)
       │
       ▼
[ Sürücü / GCC ] ─────> Platforma Özgü GCC Çağrısı (.exe / .out)
```

### Modüller ve Sorumluluk Dağılımı (`src/uppc/`)

u++ modülleri tek bir derleme birimi halinde birleştirildiğinde her dosya derleme hattının belirli bir katmanından sorumludur:

| Modül | Görev ve Sorumluluk |
|---|---|
| `havuz.upp` | **Veri Mimarisi & SoA Havuz:** Düğüm, jeton, kapsam, dize havuzu (interning), tampon depoları ve gömülü çalışma zamanı yönlendirmesi. |
| `hata.upp` | **Hata Yönetimi:** İlk hatada durma (fatal error) ve `--analiz` modu için çoklu hata biriktirme tamponu. |
| `ast.upp` | **AST Tanımları:** Jeton türleri (`JetonTur`), düğüm türleri (`DugumTur`) ve `bayt` vb. tip sabitleri. |
| `lexer.upp` | **Sözcük Analizi:** Kod noktası (codepoint) tabanlı UTF-8 tarayıcı ve kaynak konumu takibi. |
| `parser.upp` | **Sözdizim Analizi:** Özyinelemeli inişli (recursive-descent) ayrıştırıcı ve AST inşası. |
| `tur.upp` | **Tip Çıkarımı & Ad Alanı:** Çok seviyeli ad alanları (`upp.windows.*`, `upp.linux.*`), değişken/fonksiyon tip denetimi, sınıf çözümleme. |
| `guvenlik.upp` | **Güvenlik Analizi:** Güvenli/güvensiz blok sınırları ve çekirdek bellek erişimlerinin denetimi. |
| `yerlesik.upp` | **Yerleşik Tablosu:** her `upp.*` için parametre/dönüş türü, C adı, `guvensiz`/barındırılan/serbest bayrakları. |
| `cekirdek.upp` | **Çekirdek Hedefi:** gömülü önyükleyici (Multiboot → long mode), bağlayıcı betiği, araç zinciri (gcc / x86_64-elf / WSL) ve QEMU. |
| `asistan.upp` | **Proje Sistemi:** `upp.proje`, `uppc yeni/derle/calistir/asistan`. |
| `codegen.upp` | **C11 Üretimi:** AST'den optimize GNU C11 kod üretimi ve gömülü çalışma zamanının (`os_runtime_gomulu_yay`) koda eklenmesi. |
| `gomulu_runtime.upp` | **Gömülü Çalışma Zamanı:** `src/runtime/upp_runtime.c` içeriğini onaltılık (hex) bayt dizisi olarak saklayan ve ihtiyaç duyulduğunda C koduna enjekte eden modül. |
| `surucu.upp` | **CLI & Sürücü:** Komut satırı argümanlarının ayrıştırılması, GCC çağrısı ve süreç yönetimi. |
| `kendini.upp` | **Self-host:** `uppc kendini-derle` — birleştirme, runtime gömme, aşamalı derleme, sabit nokta ve bootstrap üretimi. |

---

## 🧱 Çalışma Zamanı Katmanları

| Dosya | Görev |
|---|---|
| `upp_on_barindirilan.c` / `upp_on_serbest.c` | Hedef yapılandırması, libc başlıkları ya da libc ikameleri, `UPP_AYIR/UPP_YENIDEN/UPP_BIRAK` ayırıcısı |
| `upp_cekirdek.c` | libc'siz çekirdek: metin v2 (refcount, COW, UTF-8 indeksi, sabit havuzu), metin kurucu, liste/harita, dilimler |
| `upp_barindirilan.c` | İşletim sistemi katmanı ve platform sözleşmesi (`upp_panik`, `_upp_akis_bayt`, `_upp_ondalik_yaz`) |
| `upp_serbest.c` | Çekirdek katmanı: COM1 + VGA konsolu, panik, libc'siz `%g`, `upp.cekirdek.*` |

`uppc kendini-derle` iki paket üretir: `upp_runtime.c` (barındırılan) ve `upp_runtime_serbest.c`. İkisini de derleyiciye gömer: `gomulu_runtime.upp` ve `gomulu_serbest.upp`. Codegen hiçbir libc çağrısı ya da `#include` üretmez, her şey runtime üzerinden geçer.

## 📦 Gömülü Çalışma Zamanı (Embedded Runtime) Mimarisi

Önceki sürümlerde derleyicinin çalışabilmesi için `src/runtime/upp_runtime.c` dosyasının belirli bir göreli yolda bulunması zorunluydu. u++ v3.0 ile bu kısıtlama tamamen ortadan kaldırılmıştır:

1. **Onaltılık Kodlama (Hex Encoding):**
   - `uppc kendini-derle`, `src/runtime/upp_runtime.c` dosyasını okur ve baytlarını onaltılık dizilere dönüştürür (`src/uppc/kendini.upp`).
   - Bu diziler `src/uppc/gomulu_runtime.upp` içerisindeki `os_runtime_gomulu_yay()` fonksiyonuna `tampon_yaz(upp.metin.hex_coz("..."))` çağrıları olarak işlenir.
2. **Sıfır Bağımlılık (Standalone):**
   - Derleyici çalışırken diskte `upp_runtime.c` dosyası aramak zorunda değildir; C kod üretimi sırasında gömülü çalışma zamanını doğrudan bellekten C kaynak dosyasına yayar.
3. **Özel Çalışma Zamanı Desteği (`--runtime`):**
   - Geliştiriciler isterlerse `--runtime <ozel_runtime.c>` bayrağını vererek gömülü runtime yerine kendi özel çalışma zamanı kütüphanelerini derleyiciye geçirebilirler.

---

## 🧠 Veri Modeli: Structure of Arrays (SoA) Havuzu

Native derleyicide C benzeri ham işaretçiler (`*`, `&`) yerine, veri odaklı ve bellek dostu **Structure of Arrays (SoA)** mimarisi tercih edilmiştir:

1. **ID Tabanlı Düğüm Mimarisi:**
   - Sözdizim ağacındaki her düğüm bir tam sayıdır (`sayi id`). `id == 0` değeri boş (`yok`) düğümü temsil eder.
   - Herhangi bir geçersiz ID erişiminde program çökmek (segfault) yerine derhal kontrollü bir `[u++ HATA]` mesajı üretir.
2. **Sütun Tabanlı Depolama:**
   - Düğüm alanları (tür, sol çocuk, sağ çocuk, satır, sütun vb.), `liste[sayi]` ve `liste[metin]` gibi düz dizilerde paralel sütunlar halinde tutulur.
   - Bu sayede bellek yerelliği (cache locality) verimliliği artırılır ve dinamik nesne ayırma yükü ortadan kaldırılır.
3. **Dize Havuzu (String Interning):**
   - Tanımlayıcılar ve metin sabitleri için iki yönlü bir haritalama kullanılır:
     - `harita[metin, sayi]` (metin $\rightarrow$ ID)
     - `liste[metin]` (ID $\rightarrow$ metin)
   - Karşılaştırmalar metin yerine doğrudan tamsayı ID'ler üzerinden yürütülür.
4. **Sembol Tablosu ve Kapsam:**
   - Kapsam yönetimi dinamik `harita` yapılarıyla modellenmiştir. Blok derinliklerine göre ad, tür ve kapsam düzeyi sütunlarında tutulur.
5. **Kod Üretim Tamponu:**
   - C kodu üretilirken `tampon_*` metin alanları `metin +=` işlemiyle verimli şekilde büyütülür ve son aşamada hedef C dosyasına yazılır.

---

## 🔤 Sözcük Analizi (Lexer) ve UTF-8 Yönetimi

u++, Türkçe sözdizimine ve karakter setine (ç, ğ, ı, ö, ş, ü) tam destek verir:

- **Kod Noktası (Codepoint) İmleci:**
  - Lexer imleci standart C baytları yerine UTF-8 kod noktaları üzerinden ilerler (`lex_pos = cp_i`).
  - Dize dilimleme işlemleri `upp.metin.kes` ile Unicode sınırlarına uygun yapılır.
  - Kod noktası ilerlemeleri için `upp.metin.kod` ve `upp.metin.ileri` yerleşik fonksiyonları kullanılır.
- **Kaynak Haritalama (`@upp-kaynak`):**
  - Modüller birleştirildiğinde her dosyanın başına `// @upp-kaynak <modul_adi>` etiketi yerleştirilir.
  - Lexer bu etiketi gördüğünde aktif dosya adını günceller ve satır sayacını sıfırlar; böylece hata bildirimlerinde birleşik dosya değil, orijinal kaynak modülü gösterilir.

---

## 🔁 Derleme ve Self-Hosting

Temiz klon (aşama-0, Python yok):

```bash
gcc -std=gnu11 -O2 bootstrap/uppc.c -o derleyici/uppc.exe -luser32 -lwinmm -lgdi32
# Linux: gcc -std=gnu11 -O2 bootstrap/uppc.c -o derleyici/uppc -pthread -lm
```

Kaynakları değiştirdikten sonra derleyici kendini yeniden derler. Modülleri birleştirir, runtime'ı gömer, üç aşamayı derleyip sabit noktayı doğrular, yeni `uppc`'yi kurar ve `bootstrap/uppc.c`'yi günceller:

```bash
./derleyici/uppc kendini-derle            # tam: aşama2 == aşama3 doğrulanır
./derleyici/uppc kendini-derle --hizli    # iki aşama, sabit nokta kontrolü yok
```

`havuz_impl.h` SoA native derleyicide yoktur; düğüm/intern havuzu `src/uppc/havuz.upp`, koleksiyon büyütme `src/runtime/upp_runtime.c` içindedir.
