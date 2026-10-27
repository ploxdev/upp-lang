# u++ Standart Kütüphane ve API Referansı (v3.1 beta)

Bu belge; **u++** dilinin bellek modelini, tür sistemini, hata yönetimi mekanizmalarını, `upp.*` standart kütüphane yerleşiklerini, derleme hedeflerini ve komut satırı araçlarını tanımlar.

> [!NOTE]
> Yerleşik fonksiyonların derleyici içindeki tek doğru kaynağı [`src/uppc/yerlesik.upp`](../src/uppc/yerlesik.upp) tablosudur. Tür denetleyici; argüman sayısını, parametre ve dönüş türlerini, `guvensiz` bloğu zorunluluğunu ve hedef uyumluluğunu (`barındırılan` / `serbest` / `windows` / `linux`) doğrudan bu tablodan okur. Bu referans belgesi söz konusu tabloyla birebir eşleşir.

📚 **İlgili Belgeler:** [Öğrenme Rehberi (OGREN.md)](../OGREN.md) · [Derleyici Mimarisi (derleyici.md)](../derleyici.md) · [Uppix İşletim Sistemi](https://github.com/ploxdev/uppix) · [English Reference (API_en.md)](API_en.md)

---

## 📑 İçindekiler

1. [Bellek Modeli ve Dil Mekanizmaları](#1-bellek-modeli-ve-dil-mekanizmaları)
2. [Tür Sistemi, Dönüşümler ve `boyut(T)`](#2-tür-sistemi-dönüşümler-ve-boyutt)
3. [Metin İnterpolasyonu ve Standart Çıktı/Girdi](#3-metin-i̇nterpolasyonu-ve-standart-çıktıgirdi)
4. [Genel Yerleşikler (`upp.*`)](#4-genel-yerleşikler-upp)
5. [Metin ve UTF-8 İşlemleri (`upp.metin`)](#5-metin-ve-utf-8-i̇şlemleri-uppmetin)
6. [Dosya İşlemleri (`upp.dosya`)](#6-dosya-i̇şlemleri-uppdosya)
7. [Yol ve Dizin İşlemleri (`upp.yol`)](#7-yol-ve-dizin-i̇şlemleri-uppyol)
8. [Sistem, Süreç ve Standart G/Ç (`upp.sistem`, `upp.stdio`)](#8-sistem-süreç-ve-standart-gç-uppsistem-uppstdio)
9. [Platforma Özgü API'ler (`upp.linux`, `upp.windows`)](#9-platforma-özgü-apiler-upplinux-uppwindows)
10. [Matematik ve Geometri (`upp.matematik`)](#10-matematik-ve-geometri-uppmatematik)
11. [Dinamik Koleksiyonlar (`liste[T]` ve `harita[K, V]`)](#11-dinamik-koleksiyonlar-listet-ve-haritak-v)
12. [JSON İşlemleri (`upp.json`)](#12-json-i̇şlemleri-uppjson)
13. [Eşzamanlılık (`arkaplan` ve `Kilit`)](#13-eşzamanlılık-arkaplan-ve-kilit)
14. [Çekirdek, Donanım ve İkili Tamponlar (`upp.cekirdek`, `upp.tampon`)](#14-çekirdek-donanım-ve-i̇kili-tamponlar-uppcekirdek-upptampon)
15. [Derleme Hedefleri](#15-derleme-hedefleri)
16. [Komut Satırı (`uppc`) ve `upp.proje`](#16-komut-satırı-uppc-ve-uppproje)

---

## 1. Bellek Modeli ve Dil Mekanizmaları

### Metin Sahipliği (Yönetilen UTF-8 ve Otomatik Drop)
- Her `metin` yönetilen bir nesnedir: verinin hemen önünde yer alan gizli başlıkta (`UppMetinBas`) atomik referans sayacı, bayt uzunluğu, kapasite, kod noktası sayısı ve tembel UTF-8 indeks tablosu bulunur. Metin verisinin kendisi NUL (`\0`) ile biten, C uyumlu bir `char*` işaretçisi olarak kalır.
- **Kopyalama ve Büyütme:** Atama ve kopyalama işlemleri $O(1)$ sürede çalışır (yalnızca referans sayacı artırılır). `+=` operatörü tek sahipli metni yerinde büyütür; birden fazla değişken tarafından paylaşılan metinlerde ise yazma anında kopyalama (**Copy-on-Write / COW**) yapar.
- **Otomatik Temizlik:** Derleyici; blok sonlarında, `dur` / `devam` ile döngüden çıkarken ve `don` ile fonksiyondan dönerken metin bırakma (`release`) kodunu otomatik olarak enjekte eder. Kullanıcı elle bellek boşaltmaz.
- **Hızlı UTF-8 Erişimi:** `upp.uzunluk(s)` $O(1)$ sürede çalışır. ASCII metinlerde `s[i]` indekslemesi doğrudan $O(1)$'dir; ASCII dışı UTF-8 metinler ise kendi kod noktası tablosunu önbelleğe alır.
- **Ölümsüz Sabitler:** Kaynak koddaki metin sabitleri (`"merhaba"`), başlığı derleme anında doldurulmuş ölümsüz nesnelerdir; çalışma anında `malloc` veya `free` tetiklemezler.

### Sahipli Sınıflar
İçinde `metin`, koleksiyon veya başka bir sahipli sınıf alanı barındıran sınıflar için derleyici otomatik olarak `T__birak` ve `T__kopya` yardımcıları üretir:
- Bir değişkenden kopyalama yapıldığında iç alanların referans sayıları artırılır; geçici değerler (kurucu veya fonksiyon çağrısı sonuçları) kopyalanmadan taşınır (move).
- Üzerine atama yapıldığında eski değer güvenle bırakılır. Fonksiyon parametreleri girişte sahiplenilir, `don` edilen değerler taşınarak dışarı aktarılır ve yerel sınıf değişkenleri kapsam sonunda otomatik olarak temizlenir.

### Veri Taşıyan Numaralandırmalar (`sirali`)
`sirali`, Rust ve Swift'teki `enum` gibi tek bir anahtar sözcükle hem düz numaralandırmayı hem de **etiketli birleşimi** (tagged union) tanımlar. Parantez açılan varyant içinde veri taşır:

```upp
sirali Renk { Kirmizi, Yesil, Mavi }                 // düz: secenek gibi davranır

sirali Durum {
    Bekliyor,
    Yukleniyor(sayi yuzde),
    Hata(metin mesaj, sayi kod)
}

Durum d = Durum.Yukleniyor(40);     // veri taşıyan varyant
Durum b = Durum.Bekliyor;           // veri taşımayan varyant (parantezsiz)

secim (d) {
    durum Bekliyor: upp.satir_yaz("bekliyor");
    durum Yukleniyor(y): upp.satir_yaz("%{y}");
    durum Hata(m, _): upp.satir_yaz("hata: {m}");   // '_' alanı atlar
}
```

- **Eşleştirme:** Değer yalnızca `secim` ile açılır. Kalıp `Varyant`, `Varyant(a, b)` ya da `Tur.Varyant(...)` olabilir; alanlar kolun başında yeni değişkenlere **kopyalanır** (değer anlamı). Veri taşıyan bir varyant bağlama yapılmadan da yazılabilir (`durum Hata:`).
- **Tamlık:** `varsayilan` yoksa tüm varyantlar kapsanmalıdır; eksikler derleme hatasıdır (`eksik: Hata, Bitti`). Aynı varyant iki kez eşleştirilemez.
- **Düşme yok:** `sirali` üzerindeki `secim` kolları birbirine düşmez (`dur` gerekmez); `dur` yine `secim`'den çıkar, `devam` döngüyü sürdürür.
- **Güvenlik:** Veri taşıyan bir değerin alanlarına `d.alan` ile erişilemez; yanlış varyantın verisini okumak bu yüzden imkânsızdır. Kurucu çağrısında değer sayısı ve türleri denetlenir.
- **Değer anlamı:** Atama ve parametre geçişi sahipli sınıflar gibidir (metin alanları referans sayımlı kopyalanır, geçiciler taşınır). `liste[Durum]`, fonksiyon dönüşü, iç içe `sirali` desteklenir. Atamasız bildirimin değeri ilk varyanttır (alanları sıfır/boş).
- **Düz `sirali`:** Hiçbir varyant veri taşımıyorsa `secenek`'tir (`==`, tamsayı dönüşümü, `upp.satir_yaz`); varyantlara `= N` değeri verilebilir. Yine de `secim`'de tamlık denetlenir ve kollar düşmez.
- **C karşılığı:** Veri taşıyan `sirali`, etiket alanlı bir `struct`'a ve varyant başına bir kurucu fonksiyona indirgenir; `secim` etiket üzerinde `switch` olur.

### Görünümler (`&metin` ve `&dilim[T]`)
Bellek ayırmayan ve kopyalama yapmayan salt okunur ödünç bakışlardır (Fat Pointer / View):
- Örnek: `fonk yaz_gorunum(&metin v)`, `&dilim[sayi] d = dizi[1..4];`
- **Alanlar:** `v.uzunluk` (`&metin` için UTF-8 **bayt** sayısı; `&dilim[T]` için eleman sayısı) ve `v.veri` (yalnızca `&metin` üzerinde mevcuttur).
- **Güvenlik Kuralları (Ömür Sarkması Koruması):** Görünüm türleri fonksiyondan `don` ile döndürülemez, `sinif` alanı olamaz, üst düzey (küresel) değişken olarak tanımlanamaz ve `arkaplan` blokları içine yakalanamaz.

<a id="koleksiyonlar"></a>
### Koleksiyonlar (`liste[T]` ve `harita[K, V]`)
`liste[T]` ve `harita[K, V]` **değer türüdür**:
- Kopyalama işlemi $O(1)$'dir (referans artışı); koleksiyon üzerinde değişiklik yapıldığında paylaşılan gövde otomatik olarak ayrılır (**Copy-on-Write**). Örneğin `M = L;` atamasından sonra `M.ekle(x)` çağrılması `L`'yi etkilemez; bir fonksiyona parametre olarak verilen liste çağıranın listesini değiştirmez.
- Derleyici yerel koleksiyonları kapsam sonunda otomatik olarak bırakır. `.bosalt()` metodu yalnızca belleği kapsam bitmeden erken geri vermek içindir ve koleksiyonu boş durumda bırakır.
- **Desteklenen Eleman Türleri:** İşaretçi ve görünüm türleri dışında her değer türü koleksiyon elemanı olabilir (`sayi`, `ondalik`, `mantik`, `metin`, `sinif`, iç içe `liste[...]`, `harita[...]`, `fonk(...)`, `beklenen[...]`). Harita anahtarları `metin` veya `sayi` olabilir.
- **Yerinde Değiştirme:** `L[i][j] = x`, `K[0].ad += "!"` ve `K[1].yaslan()` gibi zincirleme indeks ve metot çağrıları doğrudan yerinde (in-place) çalışır.
- `JSONDeger` bir tutamaçtır (handle); işi bittiğinde `upp.json.bosalt(kok)` ile kapatılır.

### Fonksiyon Değerleri (`fonk(P...) -> R`)
`fonk(P1, P2) -> R` birinci sınıf bir türdür (`-> R` kısmı yazılmazsa dönüş türü `bos` kabul edilir). Üst düzey fonksiyonlar adlarıyla değer olarak alınabilir; değişkende, parametrede, dönüş değerinde, sınıf alanında ve koleksiyon içinde saklanabilir:

```upp
fonk ikiye(sayi x) -> sayi => x * 2;
fonk uygula(fonk(sayi) -> sayi f, sayi x) -> sayi => f(x);

fonk(sayi) -> sayi f = ikiye;
upp.satir_yaz("{uygula(f, 5)}");

harita[metin, fonk(sayi, sayi) -> sayi] komutlar;
komutlar.koy("carp", fonk(sayi a, sayi b) -> sayi => a * b);
upp.satir_yaz("{komutlar.al(\"carp\")(3, 4)}");
```

- **Anonim Fonksiyonlar:** `fonk(sayi x) -> sayi => x * 2` (kısa ifade gövdeli) veya `fonk(...) -> T { ... }` (blok gövdeli) olarak yazılır. Dış yerel değişkenleri yakalama (closure capture) yoktur; dış kapsamdaki bir yerel değişkene erişmek derleme hatası verir (değerleri parametre olarak geçirin).
- **Boş Değer (`yok`):** İlk değer atanmamış fonksiyon değişkeni `yok` değerindedir. `f == yok` ile denetlenebilir; değeri `yok` olan bir fonksiyon çağrılırsa program açık bir panik mesajıyla durur.
- **Kısıtlar ve Hedef Uyumu:** İmzalar birebir eşleşmelidir. Fonksiyon değerleriyle yalnızca `==` ve `!=` karşılaştırmaları yapılabilir. Sınıf metotları ve `ana` fonksiyonu değer olarak alınamaz. C karşılığı düz fonksiyon işaretçisi olduğundan `--hedef serbest` ve `--hedef cekirdek` hedeflerinde de sıfır maliyetle çalışır.

### Hata Yönetimi (`beklenen`, `?`, `ertele` ve `hata_ertele`)
`beklenen[T, H]` türü ya başarılı bir `T` değeri ya da `H` türünde bir hata taşır (`beklenen[T]` yazımı `beklenen[T, metin]` için kısaltmadır). C tarafında bir etiket bayrağı ve `union` yapısına dönüşür; değer olarak döndürülür ve gizli bellek ayırması (`malloc`) yapmaz. Hata türü bir `secenek` olduğunda yığın belleğe (heap) hiç dokunulmaz.

```upp
secenek DiskHata { YOK, BOZUK, DOLU }

fonk sektor(sayi no) -> beklenen[sayi, DiskHata] {
    eger (no > 100) { don hata(DiskHata.DOLU); }
    don no * 512;                     // Doğrudan T değeri döndürmek başarıdır
}

fonk iki_sektor(sayi a, sayi b) -> beklenen[sayi, DiskHata] {
    sayi x = sektor(a)?;              // Hata varsa yerel kapsamı temizler ve hatayı yukarı döndürür
    sayi y = sektor(b)?;
    don x + y;
}
```

- **Oluşturma ve Dönüş:** `beklenen` döndüren bir fonksiyonda `don x;` başarılı değer, `don hata(e);` ise hata döndürür. Aynı kural değişken başlatmada, atamada ve fonksiyon argümanlarında da geçerlidir (`beklenen[sayi] r = 5;`, `r = hata("x");`). İlk değer verilmeyen `beklenen` değişkeni başarılı sıfır değerle başlar.
- **`beklenen[bos, H]`:** Başarı durumunda veri taşımaz; `don;` yazmak veya fonksiyonun sonuna ulaşmak başarı sayılır.
- **Metotlar:**
  - `r.tamam_mi() -> mantik`: Başarılı değer taşıyorsa `dogru`.
  - `r.hata_mi() -> mantik`: Hata taşıyorsa `dogru`.
  - `r.deger() -> T`: Başarılı değeri döndürür; hata varsa panik verir (`H` türü `metin` ise hata mesajını da yazar).
  - `r.hata() -> H`: Hata değerini döndürür; hata yoksa panik verir.
  - `r.deger_yoksa(varsayilan) -> T`: Başarılıysa değeri, hatalıysa `varsayilan` değerini döndürür.
- **`e?` (Hata Yayma Operatörü):** Yalnızca `beklenen` döndüren ve aynı `H` hata türünü kullanan fonksiyonların içinde kullanılabilir. Döngü koşulunda, `&&` / `||` operatörlerinin sağ tarafında ve `?:` üçlü ifadelerinin dallarında kullanılamaz.
- **Sonucu Yok Sayma Yasağı:** `beklenen` döndüren bir çağrının sonucunu `f();` şeklinde kullanmadan atmak derleme hatasıdır; sonucu `?` ile yayın veya bir değişkene alıp denetleyin.
- **Kısıtlar:** `beklenen` sınıf alanı olamaz, `arkaplan` içine yakalanamaz ve üst düzey (küresel) değişkende başlatıcı alamaz. `hata(...)` ifadesi doğrudan `oto` ile kullanılamaz (başarı türü belli olmadığı için).

#### `ertele` ve `hata_ertele`
- **`ertele`:** Bulunduğu kapsamdan **nasıl çıkılırsa çıkılsın** (normal blok sonu, `dur`, `devam`, `don`, `?`) kayıt sırasının tersiyle (LIFO) çalıştırılır.
- **`hata_ertele`:** Yalnızca fonksiyondan **hata ile çıkılırken** (`don hata(...)`, `?` ile erken çıkış veya çalışma anında hata taşıyan bir `beklenen` değişkeni `don r;` ile döndürülürken) çalıştırılır.
- Her ikisi de `ertele ifade;` veya `ertele { ... }` biçiminde yazılabilir. İçlerinden `don`, `?` veya dış döngüye `dur`/`devam` ile çıkılamaz. Fonksiyonun dönüş değeri, `ertele` blokları çalışmadan önce hesaplanır.

```upp
fonk ac(metin ad) -> beklenen[Dosya] {
    oto d = dosya_ac(ad)?;
    hata_ertele d.kapat();            // Yalnızca aşağıdaki adımlardan biri hata verirse çalışır
    baslik_oku(d)?;
    don d;
}
```

### `secim` Kuralları
- **Düşme yok:** Hiçbir `secim` kolu bir sonrakine düşmez; sayı, `secenek`, `sirali` ve `metin` için kural aynıdır. `dur` yine `secim`'den erken çıkar, `devam` çevreleyen döngüyü sürdürür.
- **Boş kol birleşir:** Gövdesi boş bir kol (`durum 1:` ardından `durum 2: ...`) bir sonraki dolu kolla aynı işi yapar. Veri taşıyan `sirali`'de her kol kendi alanlarını bağladığından boş kol birleşemez; derleme hatasıdır.
- **Ok fonksiyonlarında dönüş türü:** `fonk kare(sayi x) => x * x;` gibi `-> T` yazılmayan ok gövdeli fonksiyonların dönüş türü ifadeden çıkarılır; özyinelemeli olanlarda tür yazılmalıdır.

### Güvenli Tamsayı Bölme
`/` ve `%` işlemleri sıfıra bölmede ve `EN_KUCUK / -1` (`LLONG_MIN / -1`) taşmasında tanımsız davranışa (UB) girmek yerine açık panik verir (çekirdek hedefi dahil). Bölen derleme anında sabit `0` ise derleme hatası üretilir.

### `guvensiz` Blokları
Aşağıdaki işlemler yalnızca açıkça `guvensiz { ... }` bloğu içinde yapılabilir:
- Ham işaretçi türleri ve işlemleri (`*`, `&`) ile işaretçi tür dönüşümleri (`(u16*)0xB8000`),
- `bellek_ayir` / `bellek_bosalt` ve gömülü `c_kod { ... }` blokları,
- Harici süreç belleği erişimleri (`upp.windows.bellek.*`, `upp.linux.sayi_oku/yaz`),
- Doğrudan donanım port ve MMIO erişimleri (`upp.cekirdek.port_*`, `bellek_*`, `dur`, `kesme_ac/kapat`, `yigin_kur`).

#### `c_kod` ve `metin`
`metin` her zaman **yönetilen** bir değerdir: verinin hemen önünde başlık (başvuru sayısı, uzunluk, sihirli sayı) durur ve içerik paylaşımlıdır. `c_kod` içinde bir metin değişkeni sıradan bir `const char*` gibi okunabilir (`strlen(s)`, `printf("%s", s)`), ama yalnızca şu çağrılarla değiştirilir:

| C çağrısı | Ne yapar |
| :--- | :--- |
| `upp_metin_ata_c(&s, c)` | NUL ile biten ham C dizgisini (sabit, yığın tamponu, `getenv` sonucu…) **kopyalayarak** atar, eski değeri bırakır. `c` `NULL` ise `s` boş metin olur |
| `upp_metin_ata(&s, t)` | Başka bir yönetilen metni (`t` bir u++ metni ya da `UPP_LIT("...")`) paylaşarak atar, eski değeri bırakır |

```upp
metin s = "eski";
guvensiz {
    c_kod {
        char tampon[16];
        snprintf(tampon, sizeof tampon, "%d", 42);
        upp_metin_ata_c(&s, tampon);
    }
}
```

Derleyici, `c_kod` metninde kapsamdaki bir `metin` değişkenini yeniden yönlendiren (`s = ...`, `s += 1`, `s++`) ya da içeriğine yazan (`s[0] = ...`, `*s = ...`) işlemleri derleme hatasıyla reddeder. `c_kod`'un kendi tanımladığı aynı adlı C yereli (`char* s = ...`) ve üye erişimleri (`p->s`) etkilenmez. Denetim sözcükseldir; işaretçi üzerinden dolaylı atama (`char** p = &s; *p = "x";`) yakalanamaz. Bu durumda başlıksız metin ilk kullanımda `yönetilmeyen metin` paniğiyle durur, sessiz bellek bozulmasına yol açmaz.

#### `c_harici` ve `cmetin`: C fonksiyonlarını tür güvenli çağırma
Çoğu durumda `c_kod` gerekmez: bir C fonksiyonu imzasıyla bildirilir ve u++'tan normal bir fonksiyon gibi çağrılır.

```upp
c_harici fonk strlen(cmetin s) -> u64;
c_harici fonk getenv(cmetin ad) -> cmetin;
c_harici "abs" fonk c_mutlak(s32 x) -> s32;   // C sembolü u++ adından farklıysa

fonk ana() -> sayi {
    guvensiz {
        u64 n = strlen("merhaba");             // metin → cmetin örtük (ödünç)
        cmetin yol = getenv("PATH");
        eger (yol != yok) {
            metin kopya = (metin)yol;          // C dizgisinin yönetilen kopyası
            upp.satir_yaz(kopya);
        }
    }
    don 0;
}
```

- **Bildirim:** `c_harici [\"c_sembolü\"] fonk ad(parametreler) -> tür;` yalnızca üst düzeyde yazılır ve gövdesi olmaz. Derleyici C tarafında `extern T _upp_c_ad(...) __asm__("c_sembolü");` üretir. GCC asm etiketi çağrıyı doğrudan sembole bağlar, bu yüzden `c_ekle` ile gelen başlıktaki prototiple (ör. `size_t strlen(const char*)`) tür çakışması olmaz. u++ adı ASCII değilse C sembolü açıkça yazılmalıdır.
- **İzin verilen türler:**
  - Parametre: tamsayılar, `ondalik`, `mantik`, `cmetin`, `metin` (verinin kendisi, ödünç) ve işaretçiler.
  - Dönüş: bunlara ek olarak `bos`; `metin` döndürülemez.
  - Liste, harita, sınıf, görünüm ve `beklenen` C'ye geçirilemez.
- **Güvenlik:** `c_harici` fonksiyonu çağırmak ya da değer olarak almak (`fonk(cmetin) -> u64 f = strlen;`) yalnızca `guvensiz` blokta yapılabilir; C kodunun bellek güvenliğini derleyici denetleyemez.
- **`cmetin`:** Başlıksız ham C dizgisidir (`const char*`). Yönetilmez ve serbest bırakılmaz. Kurallar:
  - **Atama:** `metin` → `cmetin` örtüktür, verinin kendisini ödünç verir. `cmetin` → `metin` yalnızca `(metin)c` ile ve kopyalanarak yapılır; `NULL` boş metin olur.
  - **Karşılaştırma:** Yalnızca `yok` ya da başka bir `cmetin` ile `==` / `!=` (işaretçi eşitliği). Aritmetik, indeksleme ve yazdırma yoktur; içerik için `(metin)c` kullanın.
  - **Nerede kullanılır:** Değişkeni yalnızca `guvensiz` blokta bildirilebilir. Sınıf alanı ya da koleksiyon elemanı olamaz.
  - **Geçici metin:** `cmetin c = a + b;` gibi geçici bir metin `cmetin` değişkenine konamaz, çünkü deyim bitince serbest kalır. Fonksiyon argümanı olarak geçmek güvenlidir.

### Nitelikler: `@paketli`, `@hizali(N)`, `@bolum("ad")`
Üst düzey bir bildirimden hemen önce yazılır ve C'ye GCC `__attribute__((...))` olarak geçer. Çekirdek ve donanım kodunda bellek düzenini denetlemek içindir.

| Nitelik | Uygulandığı yer | Etkisi |
| :--- | :--- | :--- |
| `@paketli` | `sinif` | Alanlar arasında dolgu olmaz (`packed`): `sinif Baslik { ubayt tur; u32 uzunluk; }` 5 bayt olur |
| `@hizali(N)` | `sinif`, üst düzey `fonk`, global değişken | `N` bayta hizalar (`aligned(N)`); `N` 1 ile 2^30 arasında 2'nin kuvveti olmalı (ör. sayfa tablosu için `@hizali(4096)`) |
| `@bolum("ad")` | üst düzey `fonk`, global değişken | Bağlayıcı bölümünü seçer (`section("ad")`); global değişken ayrıca `used` alır, kodda kullanılmasa da atılmaz. Ad yalnızca harf, rakam, `.`, `_`, `$` içerir (en çok 64 karakter; Windows PE için kısa adlar önerilir) |

```upp
@paketli
sinif GdtGirdisi { u16 sinir; u16 taban_alt; ubayt taban_orta; ubayt erisim; ubayt bayrak; ubayt taban_ust; }

@bolum(".limine_istekler") @hizali(8)
u64 g_istek = 0;
```

Bilinmeyen nitelik, aynı niteliğin iki kez yazılması ve uygun olmayan bildirime uygulanması derleme hatasıdır.

---

## 2. Tür Sistemi, Dönüşümler ve `boyut(T)`

| Tür | C Karşılığı | Açıklama |
| :--- | :--- | :--- |
| `sayi` / `s64` | `long long` | İşaretli 64 bit tamsayı (soneksiz tamsayı sabitlerinin varsayılan türü) |
| `u64` / `usayi` | `unsigned long long` | İşaretsiz 64 bit tamsayı (ör. `5u64`) |
| `s32` / `u32`, `s16` / `u16` | `int32_t`, `uint32_t`, `int16_t`, `uint16_t` | Sabit genişlikli dar tamsayılar (ör. `7u32`, `100u16`) |
| `bayt` / `s8`, `ubayt` / `u8` | `int8_t` / `uint8_t` | İşaretli (`-128..127`) ve işaretsiz (`0..255`) 8 bit tamsayı |
| `ondalik` | `double` | Çift duyarlıklı (64 bit IEEE 754) kayan noktalı sayı |
| `metin` | `char*` (yönetilen başlık) | Atomik referans sayılı, COW destekli UTF-8 metin |
| `cmetin` | `const char*` | Başlıksız ham C dizgisi; yalnızca `guvensiz` blokta ve `c_harici` imzalarında (bkz. [`c_harici` ve `cmetin`](#c_harici-ve-cmetin-c-fonksiyonlarını-tür-güvenli-çağırma)) |
| `mantik` | `bool` | Mantıksal değer (`dogru` / `yanlis`) |
| `bos` | `void` | Değer döndürmeyen fonksiyon dönüş türü |
| `T ad[N]`, `T ad[] = [...]` | C dizisi (`T[N]`) | Yığında (stack) yaşayan, çalışma anı sınır denetimli sabit dizi: `sayi b[4];`, `sayi a[] = [1, 2];` (hata iletilerinde `dizi` diye geçer; `dizi[T, N]` diye bir tür yazımı yoktur) |
| `liste[T]`, `harita[K, V]` | `UppKolListe`, `UppKolHarita` | Değer semantikli (refcount + COW) dinamik koleksiyonlar |
| `fonk(P...) -> R` | Fonksiyon işaretçisi | Birinci sınıf fonksiyon değeri (`yok` olabilir) |
| `beklenen[T, H]`, `beklenen[T]` | Etiketli `struct` + `union` | Değer veya hata taşıyan sonuç türü (`?` ile yayılır) |
| `&metin`, `&dilim[T]` | `upp_ref_metin`, `upp_ref_dilim_*` | Kopyasız ve sıfır ayırmalı salt okunur görünümler |
| `sinif`, `secenek` | `struct`, `enum` | Kullanıcı tanımlı yapı ve numaralandırma türleri |
| `sirali` | `enum` ya da etiketli `struct` | Veri taşıyabilen numaralandırma (etiketli birleşim) |

**Tür ve ad kuralları:**
- **Metin karşılaştırma:** `==` / `!=` içerik eşitliğidir; `<`, `>`, `<=`, `>=` sözlük sırasıdır ve Unicode kod noktası sırasını izler (`"armut" < "elma"`; `ş` (U+015F), `z`'den sonra gelir). Metin yalnızca metinle karşılaştırılır; metinlerle `+` dışında aritmetik yoktur.
- **Seçenek değerleri:** `secenek Kod { A = 10, B = 0xFF, C = -3, D }` — açık değerler onluk, onaltılık ya da eksi olabilir; değersiz üye bir öncekinin bir fazlasıdır. Aynı üye adı iki kez yazılamaz.
- **Sınıf sırası:** Bir sınıf, kendisinden sonra tanımlanan bir sınıfı alan olarak içerebilir; derleyici yapıları bağımlılık sırasıyla üretir. Sınıflar birbirini değer olarak döngüsel içeremez (sonsuz boyut); döngü `liste[T]` ile kırılır.
- **C'de ayrılmış adlar:** Adlar C'ye olduğu gibi geçtiğinden C anahtar sözcükleri ve makroları (`int`, `char`, `errno`, `stdout`, `min`, `max`…) hiçbir yerde ad olamaz; libc/POSIX fonksiyon adları (`abs`, `exit`, `log`, `read`…) üst düzey fonksiyon ya da küresel değişken adı olamaz. Dosyayı `kullan ... olarak m` ile katmak adları `m_` önekiyle ayırır.
- **Gölgeleme:** Yerel değişkenler ve parametreler aynı adlı modül takma adını, sınıf ve seçenek adlarını gölgeler. İç kapsamda aynı adla yeni bir değişken bildirilebilir; dıştaki sahipli değer erken `don`/`dur`/`devam`'da da doğru bırakılır.
- Derleyici geçersiz C üretirse (bir derleyici hatası) `[GCC DERLEME HATASI]` ile birlikte GCC'nin hata satırları da gösterilir.

### Açık Tür Dönüşümü `(T)x` ve `boyut(T)`
C dilindeki dönüşüm sözdizimiyle yazılır:
```upp
ubayt b = (ubayt)x;
u16 p = (u16)70000;
sayi n = (sayi)3.9;      // 3
Renk r = (Renk)2;
```
- **Sayısal / Mantıksal / Seçenek Dönüşümleri:** Sayısal türler, `mantik` ve `secenek` arasında güvenli kodda serbesttir. Tamsayı daraltmaları C gibi alt bitleri koruyarak keser (`(ubayt)300 == 44`).
- **`ondalik` $\rightarrow$ Tamsayı Güvenliği:** Kayan noktalı sayı hedef tamsayı türünün aralığına sığmıyorsa veya `NaN` ise (C'deki tanımsız davranış yerine) çalışma zamanında açık panik verir.
- **İşaretçi Dönüşümleri (`guvensiz`):** İşaretçi $\leftrightarrow$ işaretçi, işaretçi $\leftrightarrow$ tamsayı ve fonksiyon değeri $\leftrightarrow$ işaretçi/tamsayı dönüşümleri yalnızca `guvensiz` bloğu içinde yapılabilir: `u16* vga = (u16*)0xB8000;`.
- `metin`, `sinif`, `liste`, `harita`, `beklenen` ve `dizi` türleri `(T)x` ile dönüştürülemez. İstisna: `(metin)c` bir `cmetin`'i yönetilen metne kopyalar, `(cmetin)m` metnin verisini ödünç verir.
- **`boyut(T)` ve `boyut(degisken)`:** Bir türün veya değişkenin bellekte kapladığı bayt sayısını `sayi` olarak döndürür (C `sizeof` karşılığı; sabit dizilerde tüm dizinin bayt boyutunu verir). Derleme anı sabitidir ve üst düzey `sabit` ifadelerinde kullanılabilir.

### Karakter Sabitleri
`'a'` biçimindeki karakter sabiti tek bir Unicode kod noktasıdır ve soneksiz bir tamsayı sabiti olarak davranır: türü `sayi`'dır, aralığa sığıyorsa `ubayt`, `u16` gibi türlere de atanır (`ubayt b = 'z';`). Türkçe harfler doğrudan yazılır (`'ş'` == 351, `'İ'` == 304).

| Yazım | Değer |
| :--- | :--- |
| `'a'`, `'ş'`, `'{'` | Karakterin kod noktası |
| `'\n'` `'\t'` `'\r'` `'\0'` | 10, 9, 13, 0 |
| `'\\'` `'\''` `'\"'` `'\e'` | 92, 39, 34, 27 (ESC) |
| `'\xHH'` | 2 onaltılık hane (`'\x41'` == 65) |
| `'\uHHHH'` | 4 onaltılık hane (`'\u015f'` == `'ş'`); UTF-16 vekil aralığı (D800–DFFF) hatadır |

Boş (`''`) ya da birden fazla karakter içeren (`'ab'`) sabitler ve bilinmeyen kaçışlar derleme hatasıdır. Metin için `"..."` kullanılır.

```upp
eger (tus == 'q' || tus == '\e') { cik(); }
```


---

## 3. Metin İnterpolasyonu ve Standart Çıktı/Girdi

- **İfade İnterpolasyonu (`"... {ifade} ..."`):** Metin sabiti içinde `{` karakterinden hemen sonra bir harf, `_`, `(` ya da eksiksiz bir karakter sabiti (`'a'`, `'\n'`) gelirse derleyici içeriği tam bir `u++` ifadesi olarak ayrıştırır: `"{a[0]}"`, `"{f(x)}"`, `"{a + b * 2}"`, `"{k.ad}"`, `"{(1 + 2)}"`, `"{'}'}"`. Hatalı ifadeler derleme anında yakalanır.
- **İfadenin kaynağı:** `{...}` içi kaynakta yazıldığı gibi ayrıştırılır; dış metnin kaçışları yalnızca düz metin parçalarına uygulanır. Tek istisna, iç içe metnin tırnağıdır: dış metni kapatmamak için `\"` yazılır (`"{f(\"abc\")}"`). Böylece `"{'\n'}"` ya da `"{'\''}"` beklendiği gibi çalışır. `'{' ile` gibi kapanmayan tırnaklar karakter sabiti sayılmaz, düz metin kalır.
- **Kaçış Dizileri:**
  - Metin içinde düz süslü parantez yazmak için `{{` ve `}}` kullanılır (`{` sonrasında boşluk veya rakam varsa kaçışa gerek olmadan düz metin sayılır).
  - Standart kaçışlar: `\n`, `\t`, `\r`, `\\`, `\"`, `\'`, `\e` (ESC - `0x1B`), `\xHH` (onaltılık bayt) ve `\uHHHH` (4 basamaklı Unicode kod noktası, ör. `\u00e7` $\rightarrow$ `ç`). Tanımsız kaçış dizileri derleme hatası verir.
- **Yazdırılabilir Türler:** Tüm tamsayı türleri, `ondalik`, `mantik`, `metin`, `&metin` ve `secenek` doğrudan `upp.yaz` / `upp.satir_yaz` fonksiyonlarına verilebilir.

| Fonksiyon | İmza | Açıklama |
| :--- | :--- | :--- |
| `upp.yaz(x)` | `(yazdırılabilir) -> bos` | Değeri satır sonu eklemeden standart çıktıya (çekirdekte COM1 + ekran konsoluna) yazar |
| `upp.satir_yaz(x)` | `(yazdırılabilir) -> bos` | Değeri yazıp sonuna yeni satır (`\n`) ekler (`upp.satir_yaz(42)` geçerlidir) |
| `upp.hata_yaz(x)` | `(yazdırılabilir) -> bos` | Değeri satır sonu eklemeden standart hata akışına (`stderr`) yazar |
| `upp.hata_satir_yaz(x)` | `(yazdırılabilir) -> bos` | Değeri standart hata akışına (`stderr`) satır sonuyla birlikte yazar |
| `upp.giris()` | `() -> metin` | Standart girdiden (çekirdekte PS/2 klavye veya seri porttan) bir satır okur |

> [!IMPORTANT]
> Çıktı fonksiyonları tek bir argüman alır (`printf` tarzı çoklu argüman veya `%d` biçim dizgeleri kullanılmaz). Birden fazla değeri birleştirmek için metin interpolasyonu (`"{a} ve {b}"`) kullanılır.

---

## 4. Genel Yerleşikler (`upp.*`)

| Fonksiyon | İmza | Hedef | Açıklama |
| :--- | :--- | :---: | :--- |
| `upp.uzunluk(x)` | `(metin \| dizi \| liste \| harita \| görünüm) -> sayi` | Tümü | Metinlerde UTF-8 karakter (kod noktası) sayısını, koleksiyon ve dizilerde eleman sayısını, `&metin`'de kod noktası sayısını döndürür |
| `upp.metinden_sayiya(s)` | `(metin) -> sayi` | Tümü | Metni işaretli 64 bit tamsayıya çevirir |
| `upp.sayidan_metin(n)` | `(sayi) -> metin` | Tümü | Tamsayıyı metne çevirir |
| `upp.ondalikdan_metin(d)` | `(ondalik) -> metin` | Tümü | Kayan noktalı sayıyı 17 anlamlı basamak hassasiyetiyle metne çevirir |
| `upp.metinden_ondalik(s)` | `(metin) -> ondalik` | Barındırılan | Metni kayan noktalı sayıya (`ondalik`) çevirir |
| `upp.min(a, b)` | `(sayi, sayi) -> sayi` | Tümü | İki tamsayıdan küçük olanı döndürür |
| `upp.max(a, b)` | `(sayi, sayi) -> sayi` | Tümü | İki tamsayıdan büyük olanı döndürür |
| `upp.mutlak(x)` | `(sayi) -> sayi` | Tümü | Tamsayının mutlak değerini döndürür (`LLONG_MIN` taşmasına karşı korumalıdır) |
| `upp.zaman()` | `() -> sayi` | Tümü | Milisaniye cinsinden monotonik zaman sayacı (çekirdekte PIT üzerinden çalışır) |
| `upp.uyut(ms)` | `(sayi) -> bos` | Tümü | Geçerli iş parçacığını veya çekirdek görevini `ms` milisaniye bekletir |
| `upp.rastgele(alt, ust)` | `(sayi, sayi) -> sayi` | Barındırılan | `[alt, ust]` kapalı aralığında rastgele tamsayı üretir |
| `upp.platform()` | `() -> metin` | Barındırılan | Çalışılan işletim sistemini (`"windows"` veya `"linux"`) döndürür |
| `upp.arguman_sayisi()` | `() -> sayi` | Barındırılan | Komut satırı argümanlarının sayısını (`argc`) döndürür |
| `upp.arguman(i)` | `(sayi) -> metin` | Barındırılan | `i`. komut satırı argümanını (`argv[i]`) döndürür |
| `upp.mesaj(baslik, icerik)` | `(metin, metin) -> bos` | Barındırılan | Bilgi penceresi gösterir (Windows'ta `MessageBox`, Linux'ta konsol çıktısı) |
| `upp.hata(baslik, icerik)` | `(metin, metin) -> bos` | Barındırılan | Hata penceresi/mesajı gösterir |
| `upp.ses_cal(yol)` | `(metin) -> mantik` | Barındırılan | `.wav` ses dosyasını çalar |

<details>
<summary><strong>⚠️ Kaldırılan Eski API Adları ve Yeni Karşılıkları</strong></summary>

Aşağıdaki eski adlar `v3.1 beta` ile kaldırılmıştır; kullanıldıklarında derleyici doğrudan yeni adını öneren bir hata mesajı üretir:
- `upp.metin_bosalt` $\rightarrow$ Kaldırıldı (metinler kapsam sonunda otomatik bırakılır).
- `upp.dosya_oku / yaz / ekle / var_mi / sil / boyut` $\rightarrow$ `upp.dosya.oku`, `upp.dosya.yaz`, ...
- `upp.ortam_al / ortam_yaz` $\rightarrow$ `upp.sistem.ortam_al`, `upp.sistem.ortam_yaz`
- `upp.metin.icinde` $\rightarrow$ `upp.metin.bul` (`>= 0` kontrolü ile)
- `upp.yol.klasor_olustur` $\rightarrow$ `upp.yol.dizin_olustur`
- `upp.matematik.sin / cos / tan` $\rightarrow$ `upp.matematik.sinus / kosinus / tanjant`
- `upp.json.dizi_eleman` $\rightarrow$ `upp.json.indeks_al`
- `upp.bellek.*`, `upp.girdi.*`, `upp.cizim.*` $\rightarrow$ `upp.windows.bellek.*`, `upp.windows.girdi.*`, `upp.windows.cizim.*`
</details>

---

## 5. Metin ve UTF-8 İşlemleri (`upp.metin`)

Tüm `upp.metin.*` fonksiyonları `libc`'den bağımsızdır; hem barındırılan hedeflerde hem de `--hedef serbest` / `--hedef cekirdek` altında çalışır.

| Fonksiyon | İmza | Açıklama |
| :--- | :--- | :--- |
| `upp.metin.kes(s, bas, adet)` | `(metin, sayi, sayi) -> metin` | `s` metninden UTF-8 kod noktası (karakter) cinsinden `bas` indeksinden başlayarak `adet` karakter kesip döndürür |
| `upp.metin.buyuk(s)` | `(metin) -> metin` | Türkçe kurallarına duyarlı (`i` $\rightarrow$ `İ`, `ı` $\rightarrow$ `I`) büyük harfe çevirir |
| `upp.metin.kucuk(s)` | `(metin) -> metin` | Türkçe kurallarına duyarlı (`İ` $\rightarrow$ `i`, `I` $\rightarrow$ `ı`) küçük harfe çevirir |
| `upp.metin.ingbuyuk(s)` | `(metin) -> metin` | İngilizce (ASCII) büyük harf: yalnızca `a-z` değişir; `i` $\rightarrow$ `I`, Türkçe harfler (`ı`, `ş`, `ö`…) olduğu gibi kalır. Dosya adı, protokol, onaltılık metin gibi yerel ayardan bağımsız işler için |
| `upp.metin.ingkucuk(s)` | `(metin) -> metin` | İngilizce (ASCII) küçük harf: yalnızca `A-Z` değişir; `I` $\rightarrow$ `i` (Türkçedeki `ı` değil), `İ`, `Ş` gibi harfler korunur (`"LIMINE.CONF"` $\rightarrow$ `"limine.conf"`) |
| `upp.metin.kirp(s)` | `(metin) -> metin` | Baştaki ve sondaki boşluk karakterlerini (` `, `\t`, `\r`, `\n`) temizler |
| `upp.metin.baslar(s, on)` | `(metin, metin) -> mantik` | `s` metni `on` ile başlıyorsa `dogru` döndürür |
| `upp.metin.biter(s, son)` | `(metin, metin) -> mantik` | `s` metni `son` ile bitiyorsa `dogru` döndürür |
| `upp.metin.bul(s, aranan)` | `(metin, metin) -> sayi` | İlk eşleşmenin UTF-8 karakter (kod noktası) indeksini döndürür; bulunamazsa `-1` döner |
| `upp.metin.degistir(s, eski, yeni)` | `(metin, metin, metin) -> metin` | `s` içindeki tüm `eski` parçalarını `yeni` ile değiştirir |
| `upp.metin.bol(s, ayirac)` | `(metin, metin) -> liste[metin]` | Metni `ayirac` dizgesine göre parçalayıp yeni bir `liste[metin]` döndürür |
| `upp.metin.birlestir(L, ayirac)` | `(liste[metin], metin) -> metin` | Listedeki metinleri aralarına `ayirac` koyarak tek metinde birleştirir |
| `upp.metin.kod(s, i)` | `(metin, sayi) -> sayi` | `i`. UTF-8 karakterinin Unicode kod noktası değerini döndürür |
| `upp.metin.ileri(s, i)` | `(metin, sayi) -> sayi` | `i`. UTF-8 karakterinin bellekte kaç bayt (1–4) kapladığını döndürür |
| `upp.metin.karakter(kod)` | `(sayi) -> metin` | Verilen Unicode kod noktasından tek karakterlik UTF-8 metin üretir (geçersizse `""`) |
| `upp.metin.tekrar(s, n)` | `(metin, sayi) -> metin` | `s` metnini `n` kez yan yana tekrarlayarak yeni bir metin üretir |
| `upp.metin.hex_kodla(s)` | `(metin) -> metin` | Metnin ham baytlarını küçük harfli onaltılık (hex) dizgeye çevirir |
| `upp.metin.hex_coz(hex)` | `(metin) -> metin` | Onaltılık (hex) dizgeyi çözerek ham baytlarını içeren metni döndürür |

---

## 6. Dosya İşlemleri (`upp.dosya`)

> *Yalnızca barındırılan (Windows / Linux) hedeflerde kullanılabilir.*

| Fonksiyon | İmza | Açıklama |
| :--- | :--- | :--- |
| `upp.dosya.oku(yol)` | `(metin) -> metin` | Dosyanın tüm içeriğini okur; dosya yoksa veya okunamazsa `yok` döndürür |
| `upp.dosya.yaz(yol, icerik)` | `(metin, metin) -> mantik` | İçeriği dosyaya yazar (varsa üzerine yazar); başarılıysa `dogru` döner |
| `upp.dosya.ekle(yol, icerik)` | `(metin, metin) -> mantik` | İçeriği dosyanın sonuna ekler; başarılıysa `dogru` döner |
| `upp.dosya.var_mi(yol)` | `(metin) -> mantik` | Belirtilen yolda bir dosya varsa `dogru` döndürür |
| `upp.dosya.sil(yol)` | `(metin) -> mantik` | Dosyayı siler; başarılıysa `dogru` döndürür |
| `upp.dosya.boyut(yol)` | `(metin) -> sayi` | Dosyanın bayt cinsinden boyutunu döndürür (dosya yoksa `-1`) |
| `upp.dosya.listele(dizin)` | `(metin) -> liste[metin]` | Dizindeki dosya ve alt klasör adlarını (`"."` ve `".."` hariç) listeler |

---

## 7. Yol ve Dizin İşlemleri (`upp.yol`)

> *Yalnızca barındırılan (Windows / Linux) hedeflerde kullanılabilir.*

| Fonksiyon | İmza | Açıklama |
| :--- | :--- | :--- |
| `upp.yol.birlestir(a, b)` | `(metin, metin) -> metin` | İki yol parçasını platformun dizin ayracıyla birleştirir |
| `upp.yol.dizin(yol)` | `(metin) -> metin` | Yolun üst dizin kısmını döndürür (`/` ve `\` ayraçlarını birlikte tanır) |
| `upp.yol.dosya_adi(yol)` | `(metin) -> metin` | Yoldaki son bileşeni (dosya veya dizin adını) döndürür |
| `upp.yol.uzanti(yol)` | `(metin) -> metin` | Dosya uzantısını nokta dahil döndürür (ör. `".upp"`, uzantı yoksa `""`) |
| `upp.yol.mutlak_mi(yol)` | `(metin) -> mantik` | Yol mutlak bir yol ise `dogru` döndürür |
| `upp.yol.gercek(yol)` | `(metin) -> metin` | Sembolik bağları ve göreli parçaları çözerek tam (canonical) yolu döndürür |
| `upp.yol.calisma()` | `() -> metin` | Geçerli çalışma dizinini (`cwd`) döndürür |
| `upp.yol.dizin_var_mi(yol)` | `(metin) -> mantik` | Belirtilen yolda bir dizin varsa `dogru` döndürür |
| `upp.yol.dizin_olustur(yol)` | `(metin) -> mantik` | Dizini (gerekirse eksik üst dizinleriyle birlikte) oluşturur |

---

## 8. Sistem, Süreç ve Standart G/Ç (`upp.sistem`, `upp.stdio`)

> *Yalnızca barındırılan (Windows / Linux) hedeflerde kullanılabilir.*

| Fonksiyon | İmza | Açıklama |
| :--- | :--- | :--- |
| `upp.sistem.calistir(komut)` | `(metin) -> SurecCikti` | Alt süreci çalıştırır (`stdin` kapalıdır), hem `stdout` hem `stderr` çıktısını yakalar ve `SurecCikti { sayi kod; metin cikti; mantik basarili; }` döndürür |
| `upp.sistem.kos(komut)` | `(metin) -> sayi` | Alt süreci mevcut terminalin standart girdi/çıktı tutamaçlarını devralarak etkileşimli çalıştırır ve çıkış kodunu döndürür |
| `upp.sistem.komut_bul(ad)` | `(metin) -> metin` | Komutu sistem `PATH` dizinlerinde arar; bulursa tam yolunu, bulamazsa `""` döndürür |
| `upp.sistem.exe_dizin()` | `() -> metin` | Çalışmakta olan ikili dosyanın bulunduğu dizini döndürür |
| `upp.sistem.pid()` | `() -> sayi` | Geçerli sürecin işletim sistemi kimliğini (PID) döndürür |
| `upp.sistem.cikis(kod)` | `(sayi) -> bos` | Süreci belirtilen çıkış koduyla derhal sonlandırır |
| `upp.sistem.ortam_al(ad)` | `(metin) -> metin` | Ortam değişkeninin değerini döndürür (tanımsızsa `""`) |
| `upp.sistem.ortam_yaz(ad, deger)` | `(metin, metin) -> mantik` | Ortam değişkenini ayarlar |
| `upp.stdio.gelen_oku()` | `() -> metin` | Standart girdiden LSP `Content-Length` çerçeveli JSON-RPC mesajı okur |
| `upp.stdio.yaz(json)` | `(metin) -> bos` | Standart çıktıya LSP `Content-Length` başlıklı JSON-RPC mesajı yazar |

---

## 9. Platforma Özgü API'ler (`upp.linux`, `upp.windows`)

### `upp.linux.*` *(Yalnızca `--hedef linux`)*
| Fonksiyon | İmza | Güvenlik | Açıklama |
| :--- | :--- | :---: | :--- |
| `upp.linux.pid()` | `() -> sayi` | Güvenli | Geçerli süreç kimliğini döndürür |
| `upp.linux.sinyal_gonder(pid, sinyal)` | `(sayi, sayi) -> mantik` | Güvenli | Hedef sürece POSIX sinyali (`kill`) gönderir |
| `upp.linux.proc_oku(pid, dosya)` | `(sayi, metin) -> metin` | Güvenli | `/proc/<pid>/<dosya>` içeriğini okur |
| `upp.linux.sayi_oku(pid, adres)` | `(sayi, sayi) -> sayi` | `guvensiz` | Hedef sürecin belleğinden tamsayı okur |
| `upp.linux.sayi_yaz(pid, adres, deger)` | `(sayi, sayi, sayi) -> mantik` | `guvensiz` | Hedef sürecin belleğine tamsayı yazar |

### `upp.windows.*` *(Yalnızca Windows Hedefi)*
| Fonksiyon | İmza | Güvenlik | Açıklama |
| :--- | :--- | :---: | :--- |
| `upp.windows.mesaj(baslik, icerik)` | `(metin, metin) -> bos` | Güvenli | Windows bilgi kutusu gösterir |
| `upp.windows.hata(baslik, icerik)` | `(metin, metin) -> bos` | Güvenli | Windows hata kutusu gösterir |
| `upp.windows.ses_cal(yol)` | `(metin) -> mantik` | Güvenli | `.wav` ses dosyasını çalar |
| `upp.windows.girdi.fare_tasi(x, y)` | `(sayi, sayi) -> bos` | Güvenli | Fare imlecini ekran koordinatlarına taşır |
| `upp.windows.girdi.fare_tikla()` | `() -> bos` | Güvenli | Sol fare tıklaması gönderir |
| `upp.windows.girdi.tus_bas(tus)` | `(metin) -> bos` | Güvenli | Sanal tuş vuruşu gönderir |
| `upp.windows.cizim.baslat(pencere)` | `(metin) -> sayi` | Güvenli | Hedef pencere üzerinde GDI çizim katmanı başlatır |
| `upp.windows.cizim.temizle()` | `() -> bos` | Güvenli | Çizim katmanını temizler |
| `upp.windows.cizim.kutu(x, y, g, y, renk)` | `(sayi, sayi, sayi, sayi, metin) -> bos` | Güvenli | Belirtilen renkte dikdörtgen kutu çizer |
| `upp.windows.cizim.yazi(x, y, yazi, renk)` | `(sayi, sayi, metin, metin) -> bos` | Güvenli | Belirtilen koordinata metin çizer |

### `upp.windows.bellek.*` *(Yalnızca `guvensiz` Blokta)*
| Fonksiyon | İmza | Açıklama |
| :--- | :--- | :--- |
| `baglan(surec_adi)` | `(metin) -> sayi` | Sürece bağlanır ve süreç kimliğini döndürür |
| `modul_bul(dll_adi)` | `(metin) -> sayi` | Bağlı süreçteki modülün taban adresini döndürür |
| `sayioku(adres)` / `sayiyaz(adres, deger)` | `(sayi) -> sayi` / `(sayi, sayi) -> mantik` | Adresten tamsayı okur / adrese tamsayı yazar |
| `oku_ondalik(adres)` / `yaz_ondalik(adres, d)` | `(sayi) -> ondalik` / `(sayi, ondalik) -> mantik` | Adresten ondalık okur / adrese ondalık yazar |
| `oku_metin(adres, uzunluk)` | `(sayi, sayi) -> metin` | Adresten belirtilen uzunlukta metin okur |
| `zincir_oku(taban, [ofsetler])` | `(sayi, dizi) -> sayi` | Çok seviyeli işaretçi zincirini takip ederek son adresi çözer |
| `yama_yap(adres, hex)` | `(sayi, metin) -> mantik` | Adrese onaltılık bayt dizisi yazar |
| `koruma_degistir(adres, boyut, kip)` | `(sayi, sayi, metin) -> mantik` | Bellek sayfası koruma bayraklarını değiştirir (`koruma_degistir_sayi` sayısal bayrak alır) |
| `son_hata()` / `okundu()` | `() -> sayi` | Son Win32 hata kodunu / okunan bayt sayısını döndürür |

---

## 10. Matematik ve Geometri (`upp.matematik`)

| Fonksiyon | İmza | Hedef | Açıklama |
| :--- | :--- | :---: | :--- |
| `pi()`, `e()` | `() -> ondalik` | Tümü | $\pi$ ve $e$ matematiksel sabitleri |
| `mutlak(x)` | `(ondalik) -> ondalik` | Tümü | Ondalık sayının mutlak değeri |
| `sinirla(x, alt, ust)` | `(ondalik, ondalik, ondalik) -> ondalik` | Tümü | Değeri `[alt, ust]` aralığına kıstırır (`clamp`) |
| `yumusat(a, b, t)` | `(ondalik, ondalik, ondalik) -> ondalik` | Tümü | Doğrusal ara değerleme (`lerp`) |
| `karekok(x)`, `us(taban, us)` | `(ondalik) -> ondalik` / `(ondalik, ondalik) -> ondalik` | Barındırılan | Karekök ve üs alma |
| `sinus(x)`, `kosinus(x)`, `tanjant(x)` | `(ondalik) -> ondalik` | Barındırılan | Radyan cinsinden trigonometrik fonksiyonlar |
| `atan2(y, x)` | `(ondalik, ondalik) -> ondalik` | Barındırılan | İki argümanlı arktanjant |
| `taban(x)`, `tavan(x)`, `yuvarla(x)` | `(ondalik) -> sayi` | Barındırılan | Aşağı, yukarı ve en yakın tamsayıya yuvarlama |
| `mesafe(v1, v2)` / `mesafe_3d(...)` | `(Vektor3, Vektor3) -> ondalik` | Barındırılan | 3 boyutlu Öklid mesafesi |
| `aci_hesapla(x1, y1, z1, x2, y2, z2)` | `(ondalik x 6) -> Yonelme` | Barındırılan | İki 3B nokta arasındaki yönelim açılarını (`pitch`, `yaw`) hesaplar |
| `dunya_ekran(matris, nokta, g, y)` | `(ondalik[16], Vektor3, sayi, sayi) -> EkranNokta` | Barındırılan | 3B dünya koordinatını 2B ekran koordinatına izdüşürür |

---

## 11. Dinamik Koleksiyonlar (`liste[T]` ve `harita[K, V]`)

Koleksiyonların değer semantiği (refcount + COW) ve desteklenen eleman türleri için [Koleksiyonlar](#koleksiyonlar) bölümüne bakın.

### `liste[T]` Metotları ve İşlemleri
| Kullanım | Dönüş Türü | Açıklama |
| :--- | :--- | :--- |
| `L.ekle(deger)` | `bos` | Listenin sonuna eleman ekler |
| `L[i]` / `L.al(i)` | `T` | `i`. elemanı okur (sınır denetimlidir; taşmada panik verir). `L[i] = x` yerinde yazar |
| `L.yaz(i, deger)` | `bos` | `i`. indeksteki elemanı günceller |
| `L.sil(i)` | `bos` | `i`. indeksteki elemanı siler ve sonraki elemanları kaydırır |
| `L.uzunluk()` | `sayi` | Listedeki eleman sayısını döndürür |
| `L.temizle()` | `bos` | Tüm elemanları siler (uzunluk `0` olur) |
| `L.bosalt()` | `bos` | Belleği erken serbest bırakır ve listeyi boşaltır |
| `L[a..b]` | `&dilim[T]` | `liste[sayi]` ve `liste[ondalik]` üzerinde `[a, b)` yarı açık dilim görünümü verir |
| `her (oto x in L)` | — | Elemanları sırasıyla gezer |
| `her (sayi i in a..b)` | — | Aralık: `a` dahil, `b` hariç tamsayıları gezer (`0..n` → `0, 1, …, n-1`). Sınırlar bir kez hesaplanır, `a >= b` ise hiç dönmez. Değişken tamsayı türünde ya da `oto` (`sayi`) olur; gövdede değiştirilse de sonraki tur etkilenmez |

### `harita[K, V]` Metotları ve İşlemleri
| Kullanım | Dönüş Türü | Açıklama |
| :--- | :--- | :--- |
| `H.koy(anahtar, deger)` | `bos` | Anahtar-değer çiftini ekler veya günceller (eklenme sırası korunur) |
| `H.al(anahtar)` | `V` | Anahtara karşılık gelen değeri döndürür; anahtar yoksa `V` türünün sıfır/varsayılan değerini döner |
| `H.var_mi(anahtar)` | `mantik` | Anahtar haritada mevcutsa `dogru` döndürür |
| `H.sil(anahtar)` | `bos` | Anahtarı ve değerini haritadan siler |
| `H.uzunluk()` | `sayi` | Kayıtlı anahtar-değer çifti sayısını döndürür |
| `H.anahtarlar()` | `liste[K]` | Tüm anahtarları eklenme sırasıyla bir liste olarak döndürür |
| `H.temizle()` / `H.bosalt()` | `bos` | Haritayı temizler / belleğini erken serbest bırakır |
| `her (oto k in H)` | — | Anahtarları eklenme sırasıyla gezer |

---

## 12. JSON İşlemleri (`upp.json`)

> *Yalnızca barındırılan (Windows / Linux) hedeflerde kullanılabilir.*

| Fonksiyon | İmza | Açıklama |
| :--- | :--- | :--- |
| `ayristir(metin)` / `olustur(d)` | `(metin) -> JSONDeger` / `(JSONDeger) -> metin` | JSON metnini ayrıştırır / `JSONDeger` ağacını metne dönüştürür |
| `tur(d)` | `(JSONDeger) -> metin` | Düğüm türünü döndürür: `"nesne"`, `"dizi"`, `"metin"`, `"sayi"`, `"mantik"`, `"bosluk"` |
| `var_mi(d)` / `uzunluk(d)` | `(JSONDeger) -> mantik` / `sayi` | Düğümün geçerli olup olmadığını / dizi veya nesne eleman sayısını döndürür |
| `anahtar_al(d, anahtar)` | `(JSONDeger, metin) -> JSONDeger` | Nesneden belirtilen anahtarın alt düğümünü alır |
| `anahtarlar(d)` | `(JSONDeger) -> JSONDeger` | Nesnenin anahtarlarını bir JSON dizisi olarak döndürür |
| `indeks_al(d, i)` | `(JSONDeger, sayi) -> JSONDeger` | JSON dizisinden `i`. elemanı alır |
| `metin_al(d)`, `sayi_al(d)`, `ondalik_al(d)`, `mantik_al(d)` | `(JSONDeger) -> ...` | Düğümün içindeki temel değeri okur |
| `nesne()`, `dizi()`, `yok_yap()` | `() -> JSONDeger` | Yeni boş JSON nesnesi, dizisi veya `null` düğümü oluşturur |
| `metin_yap(s)`, `sayi_yap(n)`, `ondalik_yap(d)`, `mantik_yap(b)` | `(...) -> JSONDeger` | Temel değerden yeni JSON düğümü oluşturur |
| `koy(nesne, anahtar, deger)` | `(JSONDeger, metin, JSONDeger) -> bos` | JSON nesnesine alan ekler |
| `ekle(dizi, deger)` | `(JSONDeger, JSONDeger) -> bos` | JSON dizisine eleman ekler |
| `bosalt(kok)` | `(JSONDeger) -> bos` | JSON ağacının belleğini serbest bırakır |

---

## 13. Eşzamanlılık (`arkaplan` ve `Kilit`)

> *Yalnızca barındırılan (Windows / Linux) hedeflerde kullanılabilir.*

```upp
Kilit k;
oto gorev = arkaplan {
    k.kilitle();
    upp.satir_yaz("Arka planda güvenli çalışma");
    k.birak();
};
gorev.bekle();
```

- `arkaplan { ... }` bloğu yeni bir işletim sistemi iş parçacığı (thread) başlatır ve `bekle()` metoduna sahip bir görev nesnesi döndürür.
- Dış kapsamdaki sayısal değişkenler ve `metin` değişkenleri iş parçacığına derin kopya olarak aktarılır (`Kilit` nesneleri paylaşımlı tutulur).
- Koleksiyonlar (`liste`, `harita`), `beklenen`, `JSONDeger` ve görünümler (`&metin`, `&dilim`) `arkaplan` bloğu içine yakalanamaz.

---

## 14. Çekirdek, Donanım ve İkili Tamponlar (`upp.cekirdek`, `upp.tampon`)

### `upp.cekirdek.*` *(Yalnızca `--hedef serbest` ve `--hedef cekirdek`)*

Doğrudan işlemci kayıtçılarına, G/Ç portlarına veya fiziksel adreslere dokunan düşük seviye fonksiyonlar **`guvensiz`** bloğu gerektirir:

| Fonksiyon (`guvensiz` Zorunlu) | İmza | Açıklama |
| :--- | :--- | :--- |
| `port_yaz8 / 16 / 32(port, deger)` | `(sayi, sayi) -> bos` | x86 donanım G/Ç portuna (`outb`, `outw`, `outl`) veri yazar |
| `port_oku8 / 16 / 32(port)` | `(sayi) -> sayi` | x86 donanım G/Ç portundan (`inb`, `inw`, `inl`) veri okur |
| `bellek_yaz8 / 16 / 32 / 64(adres, deger)` | `(sayi, sayi) -> bos` | Fiziksel adrese `volatile` MMIO yazması yapar (Limine'de HHDM üzerinden; üst yarıdaki bir çekirdek adresi olduğu gibi kullanılır) |
| `bellek_oku8 / 16 / 32 / 64(adres)` | `(sayi) -> sayi` | Fiziksel adresten `volatile` MMIO okuması yapar (Limine'de HHDM üzerinden) |
| `dur()`, `kesme_kapat()`, `kesme_ac()` | `() -> bos` | İşlemci yönergeleri: `hlt` (bekle), `cli` (kesmeleri kapat), `sti` (kesmeleri aç) |
| `yigin_kur(adres, boyut)` | `(sayi, sayi) -> bos` | Çalışma zamanı yığın ayırıcısına (heap) yeni bellek bölgesi tanımlar |

Aşağıdaki çekirdek servisleri ise **güvenli koddan** doğrudan çağrılabilir:

| Fonksiyon (Güvenli) | İmza | Açıklama |
| :--- | :--- | :--- |
| `kesmeler_kur()` | `() -> bos` | 48 vektörlük IDT, PIC ve 1000 Hz PIT zamanlayıcıyı kurar (açılışta otomatik çağrılır) |
| `tik()` | `() -> sayi` | Açılıştan bu yana geçen 1 ms'lik PIT zamanlayıcı tık sayısı |
| `tus_oku()` | `() -> sayi` | PS/2 klavye (Türkçe Q) veya seri porttan bir kod noktası okur; tuş yoksa `0` döner. Özel tuşlar ANSI dizisi (`ESC [ A` vb.) olarak gelir |
| `tus_bekle()` | `() -> sayi` | Bir tuş gelene kadar bekler; beklerken diğer hazır görevlere sıra verir |
| `bellek_toplam()` | `() -> sayi` | Limine/Multiboot bellek haritasındaki toplam kullanılabilir RAM miktarı (bayt) |
| `yigin_boyut()` / `yigin_kullanilan()` | `() -> sayi` | Çekirdek yığın ayırıcısının toplam kapasitesi ve kullanımda olan bayt miktarı |
| `gorev_baslat(fn, arg, ad)` | `(fonk(sayi)->bos, sayi, metin) -> sayi` | 64 KiB yığınla yeni bir işbirlikçi görev başlatır ve görev kimliğini (PID) döndürür (doluysa `-1`) |
| `sira_ver()` | `() -> bos` | İşlemci sırasını bekleyen bir sonraki hazır göreve devreder (`yield`) |
| `gorev_kimlik()` | `() -> sayi` | O anda çalışan görevin kimliğini döndürür (ana çekirdek akışı `1`'dir) |
| `gorev_listesi()` | `() -> liste[sayi]` | Yaşayan tüm görevlerin kimlik numaralarını döndürür |
| `gorev_ad(id)` / `gorev_durum(id)` | `(sayi) -> metin` | Görevin adını ve durumunu (`"çalışıyor"`, `"hazır"`, `"uyuyor"`, yoksa `""`) döndürür |
| `gorev_baslama(id)` | `(sayi) -> sayi` | Görevin başladığı zamanı (ms) döndürür |
| `gorev_oldur(id)` | `(sayi) -> mantik` | Belirtilen görevi sonlandırır (ana akış `1` sonlandırılamaz) |
| `tarih()` | `() -> metin` | CMOS gerçek zaman saatini `"YYYY-AA-GG SS:DD:ss"` biçiminde döndürür |
| `islemci()` | `() -> metin` | CPUID üzerinden işlemci marka dizgesini döndürür |
| `ekran_temizle()` | `() -> bos` | Ekran konsolunu (VGA metin ya da çerçeve tamponu) ve seri terminali temizler |
| `onyukleyici()` | `() -> metin` | Çekirdeği açan önyükleyici: `"Limine 11.4.1"` gibi ad ve sürüm, `"Multiboot 1"` ya da bilinmiyorsa `""` |
| `ekran()` | `() -> metin` | Ekran konsolunun tanımı: `"çerçeve tamponu 1280x800x32, 160x50 karakter"` ya da `"VGA metin 80x25"` |
| `kapat()` / `yeniden_baslat()` | `() -> bos` | ACPI (veya QEMU `isa-debug-exit`) ile makineyi kapatır / 8042 üzerinden yeniden başlatır |
| `disk_var(d)` | `(sayi) -> mantik` | `d` numaralı (`0..3`) ATA IDE diskinin takılı olup olmadığını döndürür |
| `disk_sektor(d)` / `disk_model(d)` | `(sayi) -> sayi` / `metin` | Diskin toplam sektör sayısını (512 baytlık) ve model adını döndürür |
| `disk_oku(d, lba, adet, tampon, ofs)` | `(sayi, sayi, sayi, sayi, sayi) -> mantik` | Diskten `lba` sektöründen başlayarak `adet` sektörü `upp.tampon` içindeki `ofs` ofsetine okur |
| `disk_yaz(d, lba, adet, tampon, ofs)` | `(sayi, sayi, sayi, sayi, sayi) -> mantik` | Tampondaki veriyi diske yazar ve donanım önbelleğini boşaltır (LBA28/LBA48 PIO) |
| `pci_oku(bus, aygit, islev, ofs)` | `(sayi, sayi, sayi, sayi) -> sayi` | PCI yapılandırma alanından `ofs`'teki (4'e yuvarlanır) 32 bitlik sözcüğü `0xCF8`/`0xCFC` portlarıyla okur; olmayan aygıt ya da geçersiz argüman `0xFFFFFFFF` okur |
| `pci_yaz(bus, aygit, islev, ofs, deger)` | `(sayi, sayi, sayi, sayi, sayi) -> bos` | PCI yapılandırma sözcüğü yazar (komut yazmacı, BAR'lar). *`guvensiz` gerekir* |
| `mmio_ac(fiz, boyut)` | `(sayi, sayi) -> sayi` | Fiziksel bir aralığa MMIO penceresi açar ve tutamacını döndürür (aralık HHDM'de / Multiboot'ta ilk 1 GiB'da değilse `0`). Aynı aralık yeniden açılınca aynı tutamaç döner. *`guvensiz` gerekir* |
| `mmio_oku8 / 16 / 32(h, ofs)` | `(sayi, sayi) -> sayi` | `h` penceresinde `volatile` okuma; sınır dışı ya da hizasız ofset panik verir (FreeBSD `bus_space_read_*` gibi) |
| `mmio_yaz8 / 16 / 32(h, ofs, deger)` | `(sayi, sayi, sayi) -> bos` | `h` penceresinde sınır ve hizalama denetimli `volatile` yazma |
| `tampon_fiziksel(tampon, ofs)` | `(sayi, sayi) -> sayi` | Bir `upp.tampon` tamponunun `ofs`'teki baytının fiziksel adresi (DMA tanımlayıcıları için). Çekirdek yığını fiziksel olarak bitişik tek bölgedir, yani tamponun tamamı bitişiktir; bilinmiyorsa `-1` |

#### Kullanıcı kipi (ring 3) *(Yalnızca `--onyukleyici limine`)*

Mekanizma çekirdekte (C), politika işletim sisteminde (u++) durur: süreç tablosu, ELF yükleme ve sistem çağrılarının anlamı u++'ta yazılır. Uppix bunları Linux x86_64 ABI'siyle kullanır (bkz. [Uppix README](https://github.com/ploxdev/uppix)). `ka`, bir adres alanının tutamacıdır (PML4'ün fiziksel adresi). `koruma` Linux `PROT_*` bitleridir: 1 okuma, 2 yazma, 4 yürütme.

| Fonksiyon | İmza | Açıklama |
| :--- | :--- | :--- |
| `kullanici_destekli()` | `() -> mantik` | Kullanıcı kipi kurulu mu (Limine/HHDM) |
| `kullanici_alan_yeni()` / `kullanici_alan_sil(ka)` | `() -> sayi` / `(sayi) -> bos` | Boş alt yarılı yeni adres alanı (bellek yoksa `0`) / tüm sayfalarıyla silme |
| `kullanici_esle(ka, adres, uzunluk, koruma)` | `(sayi, sayi, sayi, sayi) -> mantik` | Aralığı sıfırlanmış yeni sayfalarla eşler; zaten eşli sayfaların izinleri birleşir |
| `kullanici_esleme_kaldir(ka, adres, uzunluk)` | `(sayi, sayi, sayi) -> bos` | Sayfaları kaldırır ve süreçin çerçevelerini bırakır |
| `kullanici_koruma(ka, adres, uzunluk, koruma, birlestir)` | `(sayi, sayi, sayi, sayi, mantik) -> bos` | İzinleri ayarlar (mprotect mekanizması); `birlestir` yalnızca ekler |
| `kullanici_bos_mu(ka, adres, uzunluk)` | `(sayi, sayi, sayi) -> mantik` | Aralıkta eşli sayfa yoksa `dogru` |
| `kullanici_esli_mi(ka, adres, uzunluk)` | `(sayi, sayi, sayi) -> mantik` | Aralıktaki her sayfa eşliyse `dogru` (mprotect/madvise `ENOMEM` denetimi) |
| `kullanici_fiziksel_esle(ka, adres, fiziksel, uzunluk, koruma)` | `(sayi, sayi, sayi, sayi, sayi) -> mantik` | Aygıt belleğini (ör. çerçeve tamponu) write-combining eşler. *`guvensiz` gerektirir* |
| `kullanici_oku(ka, adres, tampon, ofs, n)` / `kullanici_yaz(...)` | `(sayi, sayi, sayi, sayi, sayi) -> mantik` | Kullanıcı belleği ↔ `upp.tampon`. Her sayfanın izni denetlenir; hatalı adreste `yanlis` (EFAULT) |
| `kullanici_metin(ka, adres, en_cok)` | `(sayi, sayi, sayi) -> metin` | NUL ile biten kullanıcı dizgisi; hatalı adreste ya da sınırda `yok` |
| `kullanici_calistir(ka, giris, yigin)` | `(sayi, sayi, sayi) -> sayi` | Süreci bu görevde ring 3'te çalıştırır; bitince çıkış kodunu döndürür (sinyalde 128 + sinyal) |
| `kullanici_cikis_iste(kod)` | `(sayi) -> bos` | Sistem çağrısı işleyicisi dönünce süreç `kod` ile biter (exit) |
| `kullanici_tls_ayarla(segment, taban)` / `kullanici_tls(segment)` | `(sayi, sayi) -> mantik` / `(sayi) -> sayi` | Şimdiki sürecin FS (`0`) ya da GS (`1`) tabanı (arch_prctl, TLS); görev geçişinde geri yüklenir. Taban kullanıcı alanında değilse `yanlis` |
| `kullanici_on_plan(v)` | `(mantik) -> bos` | Süreç ön planda mı (Ctrl-C SIGINT ile sonlandırır) |
| `kullanici_son_hata()` | `() -> metin` | Son sonlanmanın nedeni (`"Bölümleme hatası (SIGSEGV): ..."`, `"^C"`; normal çıkışta `""`) |
| `sistem_cagrisi_kur(f)` | `(fonk(sayi,sayi,sayi,sayi,sayi,sayi,sayi)->sayi) -> bos` | `SYSCALL` işleyicisi: `f(no, a1, ..., a6)`; dönüş rax'a yazılır (hata: `-errno`) |
| `kullanici_alan_kopyala(ka)` | `(sayi) -> sayi` | Adres alanının kopyası (fork): sahipli her sayfa yeni bir çerçeveye kopyalanır, sahipsiz eşlemeler (çerçeve tamponu) paylaşılır; bellek yetmezse `0` |
| `kullanici_cerceve_kaydet(tampon)` | `(sayi) -> mantik` | Sistem çağrısı içinde, çağıranın kullanıcı yazmaçlarını tampona (en az 672 bayt) yazar: küçük sonlu 18 adet 64 bitlik yuva `rax, rdi, rsi, rdx, r10, r8, r9, r15, r14, r13, r12, rbx, rbp, rip, rflags, rsp, fs tabanı, gs tabanı`, ardından 160. bayttan itibaren 512 baytlık FXSAVE alanı. Sistem çağrısı dışında `yanlis` |
| `kullanici_cerceve_calistir(ka, tampon)` | `(sayi, sayi) -> sayi` | Süreci bu görevde tampondaki yazmaçlardan başlatır (fork'un çocuğu; politika önce `rax` yuvasına `0` yazar); dönüşü `kullanici_calistir` ile aynıdır. `rip`/`rsp`/TLS tabanları kullanıcı adresi olmalı (değilse `-1`); RFLAGS'ın yalnızca kullanıcının değiştirebileceği bitleri korunur |
| `kullanici_yurut(ka, giris, yigin)` | `(sayi, sayi, sayi) -> mantik` | execve mekanizması, yalnızca sistem çağrısı içinde: işleyici dönünce süreç `ka` adres alanında, temiz yazmaçlarla `giris`'ten sürer; eski adres alanı bırakılır |
| `ekran_bilgi(alan)` | `(sayi) -> sayi` | 0 çerçeve tamponu fiziksel adresi, 1 boyu, 2-3 piksel genişlik/yükseklik, 4 satır adımı, 5 bpp, 6-11 renk alanları, 12-13 sütun/satır |
| `cerceve_bilgi(alan)` | `(sayi) -> sayi` | Kullanıcı çerçeve havuzu: 0 toplam, 1 kullanılan, 2 boş (bayt) |

> [!NOTE]
> **Çekirdek Ortamı Notları:**
> - **Bellek:** Açılışta yığın ayırıcı (heap), bellek haritasındaki en büyük boş RAM bölgesine otomatik olarak kurulur (Multiboot'ta ilk 1 GiB içinde; Limine'de en büyük bölgenin dörtte üçü, dörtte biri kullanıcı süreçlerinin çerçeve havuzu olur).
> - **Konsol:** Multiboot ile VGA metin ekranı (CP437 Türkçe eşleme), Limine ile çerçeve tamponuna çizilen metin konsolu (Terminus 8x16 yazı tipi, Türkçe karakterler dahil) kullanılır. İkisi de kutu çizim ve gölge karakterlerini (`├ └ │ ─ █ ░ ▒ ▓`) ve ANSI kaçış dizilerini destekler: renk (`\e[1;32m`), temizleme (`\e[2J`, `\e[K`), imleç konumu (`\e[H`, `\e[satır;sütunH`, `\e[nC`/`\e[nD`) ve imleci gizleme/gösterme (`\e[?25l` / `\e[?25h`).
> - **Limine:** `--onyukleyici limine` ile çekirdek `0xffffffff80000000`'da duran bir ELF64 olur (`-mcmodel=kernel`). Açılışta Limine protokolü (taban revizyon 6) istekleriyle bellek haritası, çerçeve tamponu, HHDM ve çekirdek adresi alınır; çekirdek kendi sayfa tablolarını kurar: çekirdek görüntüsü ve tüm fiziksel bellek (HHDM) üst yarıda, alt yarı kullanıcı süreçleri için boş. GDT/TSS, `SYSCALL`, SSE, NX/SMEP/SMAP açılır ve 8259 PIC kesmeleri için yerel APIC kapatılır.
> - **İstisnalar:** Çekirdekteki işlemci istisnaları (sıfıra bölme, sayfa hatası vb.) Türkçe istisna adı, `RIP` ve `CR2` adresiyle panik verir. Ring 3'teki bir istisna ise yalnızca o süreci sonlandırır.

### `upp.tampon.*` — Güvenli İkili Bayt Tamponları *(Tüm Hedeflerde)*

Disk sektörleri, dosya sistemi yapıları (`ext4`, `FAT32`, `exFAT`) ve ikili protokolleri `guvensiz` bloğuna ihtiyaç duymadan işlemek için kullanılır. Her tampon bir `sayi` tutamacıyla temsil edilir ve **tüm okuma/yazma işlemleri sınır denetimlidir** (taşma veya geçersiz tutamaçta açık panik verir). Çok baytlı tamsayılar küçük sonlu (**little-endian**) düzende okunur ve yazılır.

| Fonksiyon | İmza | Açıklama |
| :--- | :--- | :--- |
| `upp.tampon.yeni(boyut)` | `(sayi) -> sayi` | Sıfırlanmış yeni bir bayt tamponu ayırır ve tutamacını döndürür |
| `upp.tampon.birak(t)` | `(sayi) -> bos` | Tampon belleğini serbest bırakır |
| `upp.tampon.boyut(t)` | `(sayi) -> sayi` | Tamponun bayt cinsinden boyutunu döndürür |
| `upp.tampon.oku8 / 16 / 32 / 64(t, ofs)` | `(sayi, sayi) -> sayi` | `ofs` bayt ofsetinden küçük sonlu tamsayı okur (`8/16/32` işaretsiz genişler, `64` ham bit desenidir) |
| `upp.tampon.yaz8 / 16 / 32 / 64(t, ofs, deger)` | `(sayi, sayi, sayi) -> bos` | `ofs` bayt ofsetine küçük sonlu tamsayı yazar |
| `upp.tampon.metin(t, ofs, n)` | `(sayi, sayi, sayi) -> metin` | Tampondaki `n` baytı UTF-8 metin olarak kopyalar |
| `upp.tampon.metin_yaz(t, ofs, s)` | `(sayi, sayi, metin) -> sayi` | `s` metninin UTF-8 baytlarını tampona yazar ve yazılan bayt sayısını döndürür |
| `upp.tampon.utf16_metin(t, ofs, n)` | `(sayi, sayi, sayi) -> metin` | `n` adet 16 bitlik UTF-16LE kod birimini (`0x0000`'da durarak, vekil çiftler dahil) UTF-8 metne çevirir |
| `upp.tampon.utf16_yaz(t, ofs, s, sinir)` | `(sayi, sayi, metin, sayi) -> sayi` | Metni en çok `sinir` adet UTF-16LE kod birimi olarak yazar ve yazılan birim sayısını döndürür |
| `upp.tampon.kopyala(hedef, hofs, kaynak, kofs, n)` | `(sayi x 5) -> bos` | İki tampon (veya aynı tampon) arasında `n` baytı örtüşmeye dayanıklı (`memmove`) kopyalar |
| `upp.tampon.doldur(t, ofs, n, deger)` | `(sayi, sayi, sayi, sayi) -> bos` | Tamponun `[ofs, ofs + n)` aralığını `deger` baytıyla doldurur |
| `upp.tampon.crc32c(t, ofs, n, baslangic)` | `(sayi, sayi, sayi, sayi) -> sayi` | `n` bayt üzerinde Castagnoli **CRC32C** hesaplar (son XOR uygulanmamış `ext4` biçimi; standart CRC32C için sonucu `4294967295` ile XOR'layın) |

---

## 15. Derleme Hedefleri

| Hedef Bayrağı | Üretilen Çıktı | Açıklama |
| :--- | :--- | :--- |
| *(varsayılan; `upp.proje` içinde `hedef = yerel`)* | `.exe` / `.out` | Derleyicinin çalıştığı işletim sistemi için bağımsız uygulama |
| `--hedef windows` / `--hedef linux` (kısa: `--windows` / `--linux`) | `.exe` / `.out` | Hedef işletim sistemi ABI'si ve platform API'leri |
| `--hedef serbest` | ELF `.o` | `libc` bağımlılığı olmayan nesne dosyası (giriş noktası: `upp_cekirdek_giris()`) |
| `--hedef cekirdek` | `<ad>.elf` + `<ad>32.elf` | Multiboot 1 ile önyüklenen x86_64 işletim sistemi çekirdeği (QEMU kuruluysa otomatik çalıştırır) |
| `--hedef cekirdek --onyukleyici limine` | `<ad>.elf` | Limine ile diskten 64-bit açılan higher-half ELF64 çekirdek (BIOS ve UEFI) |

> [!WARNING]
> `--hedef serbest` ve `--hedef cekirdek` modlarında işletim sistemi gerektiren ("barındırılan") `upp.dosya`, `upp.yol`, `upp.sistem`, `upp.json` modülleri ile `arkaplan` ve `Kilit` yapıları kullanılamaz; kullanıldığında derleme hatası verilir.

---

## 16. Komut Satırı (`uppc`) ve `upp.proje`

```bash
# Tekil dosya derleme ve çalıştırma
uppc <dosya.upp> [-o ad] [--sadece-derle] [-c | --sadece-c] [--hedef h] [--analiz] [--json-hata]
                 [--bicim | --bicim-kontrol] [--ast] [--runtime yol] [--disk imaj]...
                 [--disk-arayuz ide|ahci] [--onyukleyici multiboot|limine] [--disk-hazirla betik.sh]
                 [--guvenlik-denetimi[=is]]

# Proje yönetimi ve araçlar
uppc yeni <ad> [--hedef cekirdek]     # Yeni proje iskeleti (upp.proje + kaynak/ana.upp) oluşturur
uppc derle [dizin]                    # Projeyi upp.proje ayarlarına göre derler
uppc calistir [dizin]                 # Projeyi derler ve çalıştırır (çekirdek hedefinde QEMU başlatır)
uppc asistan [dizin]                  # Etkileşimli terminal derleme asistanını açar
uppc kendini-derle [--hizli] [--kok <dizin>]  # Derleyiciyi kendisiyle yeniden derler ve bootstrap/uppc.c'yi günceller
uppc -v [--json]                      # Derleyici sürümünü yazar
```

### `upp.proje` Yapılandırma Biçimi

```ini
ad = merhaba
giris = kaynak/ana.upp
hedef = yerel                 # yerel | windows | linux | serbest | cekirdek
cikti = derleme/merhaba
bayraklar =                   # İsteğe bağlı ek uppc bayrakları
diskler = a.img, b.img        # Çekirdek hedefinde QEMU'ya bağlanacak ham IDE disk imajları (sda, sdb...)
disk_arayuz = ahci            # İsteğe bağlı: diskleri IDE yerine AHCI (SATA) denetleyicisine bağla
disk_hazirla = disk_yap.sh    # Disk imajlarından biri eksikse proje dizininde sh ile çalıştırılacak betik
onyukleyici = limine         # Çekirdek ilk diskten Limine ile açılır; disk_hazirla her derlemeden sonra çalışır
```

- `--sadece-derle` ikiliyi üretir ama çalıştırmaz; `-c` / `--sadece-c` yalnızca üretilen C dosyasını yazar, GCC'yi çağırmaz. `--hedef` şu değerleri alır: `windows`, `linux`, `serbest`, `cekirdek`; verilmezse derleyicinin çalıştığı sistem hedeflenir.
- `--disk <imaj>` bayrağı (en çok 4 adet), `--hedef cekirdek` ile çalışırken QEMU'ya `-drive if=ide,format=raw` olarak disk imajı bağlar.
- `--disk-arayuz ahci` (`upp.proje` içinde `disk_arayuz = ahci`) diskleri bunun yerine bir AHCI denetleyicisinin SATA kapılarına bağlar (`-device ahci`, en çok 6 disk, ilki önyükleme diskidir); varsayılan `ide`'dir.
- `--guvenlik-denetimi` programı sanitizer'larla derler. Linux'ta ASan + UBSan açılır: sınır dışı ve serbest bırakıldıktan sonra erişim, sızıntı ve tanımsız davranış ilk hatada kaynak satırıyla durur. `--guvenlik-denetimi=is` bunun yerine TSan ile `arkaplan` iş parçacıklarındaki yarışları arar. Windows/MinGW'de ASan çalışma zamanı olmadığı için yalnızca UBSan tuzak kipi açılır ve tanımsız davranış açıklayıcı bir mesajla durdurulur. Bayrak serbest ve çekirdek hedeflerinde kullanılamaz. Eski `UPP_SANITIZE` ortam değişkeni (ham gcc bayrakları) hâlâ geçerlidir.
- `--onyukleyici limine` çekirdeği Limine protokolüyle üretir. QEMU `-kernel` olmadan, ilk diskten açılır; bu yüzden ilk `--disk` önyüklenebilir bir imaj olmalıdır.
- `--disk-hazirla <betik>` çekirdek derlendikten sonra `sh <betik> <ilk diskin dizini> <çekirdek.elf>` komutunu çalıştırır (Windows'ta WSL içinde). Betik önyüklenebilir imajı üretir ya da içindeki çekirdeği günceller; örnek için [Uppix `disk_yap.sh`](https://github.com/ploxdev/uppix/blob/main/disk_yap.sh). `UPP_FS_ARACLAR` ortam değişkeni betiğe `FS_ARACLAR` olarak geçer.
