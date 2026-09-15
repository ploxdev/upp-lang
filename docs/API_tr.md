# u++ Standart Kütüphane ve Kamu API Referansı (v3.0)

Bu belge, **u++ v3.0** dilinin standart kütüphanesini (`upp.*`), yerleşik türlerini, bellek sahipliği kurallarını ve native derleyici arayüzünü tanımlayan resmi Türkçe teknik referanstır.

Sıfırdan dil eğitimi için [OGREN.md](../OGREN.md) kılavuzunu, derleyici iç mimarisi için [derleyici.md](../derleyici.md) belgesini inceleyebilirsiniz. İngilizce eşleniği: [API_en.md](API_en.md).

---

## İçindekiler

1. [Genel Sözleşme ve Bellek Kuralları](#1-genel-sözleşme-ve-bellek-kuralları)
2. [Veri Tipleri ve Tip Sistemi](#2-veri-tipleri-ve-tip-sistemi)
3. [Konsol, Giriş ve Kullanıcı Etkileşimi](#3-konsol-giriş-ve-kullanıcı-etkileşimi)
4. [Dönüşümler ve Yardımcı İşlevler](#4-dönüşümler-ve-yardımcı-işlevler)
5. [Metin İşlemleri (`upp.metin`)](#5-metin-işlemleri-uppmetin)
6. [Dosya Sistemi (`upp.dosya`)](#6-dosya-sistemi-uppdosya)
7. [Yol ve Dizin İşlemleri (`upp.yol`)](#7-yol-ve-dizin-işlemleri-uppyol)
8. [Sistem ve Süreç Yönetimi (`upp.sistem`)](#8-sistem-ve-süreç-yönetimi-uppsistem)
9. [Platforma Özgü API'ler (`upp.windows` ve `upp.linux`)](#9-platforma-özgü-apiler-uppwindows-ve-upplinux)
10. [Matematik Kütüphanesi (`upp.matematik`)](#10-matematik-kütüphanesi-uppmatematik)
11. [Veri Yapıları: `liste` ve `harita`](#11-veri-yapıları-liste-ve-harita)
12. [JSON Kütüphanesi (`upp.json`)](#12-json-kütüphanesi-uppjson)
13. [Eşzamanlılık ve Senkronizasyon: `arkaplan` ve `Kilit`](#13-eşzamanlılık-ve-senkronizasyon-arkaplan-ve-kilit)
14. [Düşük Seviyeli Araçlar (`guvensiz` Alanı)](#14-düşük-seviyeli-araçlar-guvensiz-alanı)
15. [LSP ve Stdio Çerçevesi (`upp.stdio`)](#15-lsp-ve-stdio-çerçevesi-uppstdio)
16. [Derleyici Komut Satırı Arayüzü (CLI)](#16-derleyici-komut-satırı-arayüzü-cli)
17. [C Tipi ve Sembol Eşleme Tablosu](#17-c-tipi-ve-sembol-eşleme-tablosu)

---

## 1. Genel Sözleşme ve Bellek Kuralları

* **Metin Formatı:** Tüm metinler UTF-8 kodlamasındadır. `s[i]`, `upp.uzunluk(s)` ve `upp.metin.kod(s, i)` çağrıları ham C baytlarını değil, doğrudan Unicode **karakter kod noktasını (code point)** döndürür. Geçersiz indeksler `0` verir.
* **Bellek Sahipliği (`upp.metin_bosalt`):**
  - Dinamik olarak öbekte (`malloc`) oluşturulan metinlerin (`upp.giris()`, `upp.dosya.oku()`, `upp.sistem.calistir()` çıktısı, çalışma anındaki metin birleştirmeleri `+` ve dize interpolasyonları) işi bittiğinde `upp.metin_bosalt(s)` ile serbest bırakılması gerekir.
  - Sabit dizeler (`"merhaba"`), derleme anında birleştirilen sabitler, `upp.arguman(i)` (tüm süreç boyu yaşar) ve sayısal dönüşler (`kod`, `ileri`) için boşaltma çağrısı **yapılmaz**.
* **Güvenlik Sınırları:**
  - Varsayılan u++ kodunda ham işaretçiler (`*`, `&`), doğrudan bellek adresleme (`bellek_ayir`, `bellek_bosalt`), satır içi C kodları (`c_kod { }`) ve çekirdek süreç belleği erişimi (`upp.windows.bellek.*`, `upp.linux.sayi_oku/yaz`) yasaktır.
  - Bu işlemlere sadece **`guvensiz { ... }`** blokları içerisinde izin verilir.
* **Hata Yönetimi:**
  - Tam derleme ilk ölümcül hatada durur ve çıkış kodu verir.
  - Canlı düzenleyici analizi (`--analiz`), derleme öncesi tür ve güvenlik hatalarını 50 adede kadar biriktirerek yapılandırılmış JSON formatında raporlar.

---

## 2. Veri Tipleri ve Tip Sistemi

| Tür Adı | C Karşılığı | Bellek Boyutu | Açıklama |
|---|---|---|---|
| `sayi` | `int64_t` | 8 bayt | İşaretli 64-bit tam sayı. Varsayılan sayı türüdür. |
| `ondalik` | `double` | 8 bayt | Çift duyarlıklı (IEEE 754) kayan noktalı sayı. |
| `bayt` | `uint8_t` | 1 bayt | İşaretsiz 8-bit tam sayı (0 ile 255 arası). Düşük seviyeli veri ve ikili akışlar içindir. |
| `metin` | `char*` | 8 bayt | UTF-8 kodlanmış sıfır sonlandırmalı dinamik veya sabit dize. |
| `mantik` | `bool` | 1 bayt | Mantıksal değer: `dogru` (`true`) veya `yanlis` (`false`). |
| `bos` | `void` | 0 bayt | Değer döndürmeyen fonksiyonların dönüş tipi. |
| `dizi[T, N]` | Düz C dizisi | `N * sizeof(T)` | Sabit boyutlu yığın dizisi (`sayi puanlar[10];`). |
| `liste[T]` | `upp_liste_t*` | Dinamik | Dinamik boyutlu ve tür güvenli sıralı koleksiyon. |
| `harita[K, V]` | `upp_harita_t*` | Dinamik | Anahtar-değer eşlemesi sunan tür güvenli sözlük yapısı. |
| `sinif` | `struct` | Değişken | Kullanıcı tanımlı nesne yönelimli veri modeli. |

---

## 3. Konsol, Giriş ve Kullanıcı Etkileşimi

| Fonksiyon | Parametreler | Dönüş Türü | Açıklama |
|---|---|---|---|
| `upp.yaz` | `metin s` | `bos` | Standart çıktıya (`stdout`) metin yazar. Dize interpolasyonu destekler. |
| `upp.satir_yaz` | `metin s` | `bos` | Metni yazar ve sonuna yeni satır karakteri (`\n`) ekler. |
| `upp.hata_yaz` | `metin s` | `bos` | Standart hata akışına (`stderr`) metin yazar. |
| `upp.hata_satir_yaz` | `metin s` | `bos` | Standart hata akışına (`stderr`) satır sonu ile yazar. |
| `upp.giris` | — | `metin` | Kullanıcıdan konsoldan bir satır okur. Dönen metin `metin_bosalt` gerektirir. |

---

## 4. Dönüşümler ve Yardımcı İşlevler

| Fonksiyon | Parametreler | Dönüş Türü | Açıklama |
|---|---|---|---|
| `upp.uzunluk` | `metin` veya `dizi[]` | `sayi` | Metin için UTF-8 karakter sayısı, dizi için eleman sayısı döner. |
| `upp.metinden_sayiya` | `metin s` | `sayi` | Metni 64-bit tam sayıya çevirir (`atoll`). |
| `upp.sayidan_metin` | `sayi n` / `bayt b` | `metin` | Sayı veya bayt değerini metne çevirir (`metin_bosalt` gerekir). |
| `upp.ondalikdan_metin` | `ondalik d` | `metin` | Ondalıklı sayıyı metne çevirir (`metin_bosalt` gerekir). |
| `upp.metinden_ondalik` | `metin s` | `ondalik` | Metni ondalıklı sayıya çevirir (`atof`). |
| `upp.min` | `sayi a, sayi b` | `sayi` | İki sayıdan küçük olanı döner. |
| `upp.max` | `sayi a, sayi b` | `sayi` | İki sayıdan büyük olanı döner. |
| `upp.mutlak` | `sayi n` | `sayi` | Sayının mutlak değerini döner. |
| `upp.zaman` | — | `sayi` | Sürecin başlangıcından bu yana geçen milisaniye (monotonic). |
| `upp.uyut` | `sayi milisaniye` | `bos` | Belirtilen milisaniye kadar akışı duraklatır. |
| `upp.rastgele` | `sayi min, sayi max` | `sayi` | Verilen kapalı aralıkta (`[min, max]`) rastgele 64-bit sayı üretir. |
| `upp.platform` | — | `metin` | Çalışma ortamını döner: `"windows"` veya `"linux"`. |
| `upp.metin_bosalt` | `metin s` | `bos` | Öbekte ayrılmış dinamik dizeyi serbest bırakır. |

---

## 5. Metin İşlemleri (`upp.metin`)

| Fonksiyon | Parametreler | Dönüş Türü | Açıklama |
|---|---|---|---|
| `upp.metin.kes` | `metin s, sayi bas, sayi son` | `metin` | Unicode karakter indekslerine göre dizeyi dilimler. |
| `upp.metin.buyuk` | `metin s` | `metin` | Metni büyük harfe dönüştürür (Türkçe İ/I desteği). |
| `upp.metin.kucuk` | `metin s` | `metin` | Metni küçük harfe dönüştürür (Türkçe ı/i desteği). |
| `upp.metin.kirp` | `metin s` | `metin` | Baş ve sondaki boşluk karakterlerini temizler. |
| `upp.metin.baslangic_mi` | `metin s, metin on` | `mantik` | Metin belirtilen ön ekle başlıyorsa `dogru`. |
| `upp.metin.bitis_mi` | `metin s, metin son` | `mantik` | Metin belirtilen son ekle bitiyorsa `dogru`. |
| `upp.metin.bul` | `metin s, metin aranan` | `sayi` | Alt dizenin ilk karakter indeksini döner; yoksa `-1`. |
| `upp.metin.degistir` | `metin s, metin eski, metin yeni` | `metin` | Alt dizelerin tümünü yenisiyle değiştirir. |
| `upp.metin.bol` | `metin s, metin ayirici` | `liste[metin]` | Metni ayırıcıya göre bölüp liste olarak döner. |
| `upp.metin.kod` | `metin s, sayi i` | `sayi` | Karakter indeksindeki Unicode kod noktasını döner. |
| `upp.metin.ileri` | `metin s, sayi cp_i` | `sayi` | Verilen kod noktası indeksinden bir sonraki kod noktası indeksini döner. |
| `upp.metin.hex_coz` | `metin hex_str` | `metin` | Onaltılık (hex) dizeyi çözüp ikili/metin tamponuna çevirir. |

---

## 6. Dosya Sistemi (`upp.dosya`)

| Fonksiyon | Parametreler | Dönüş Türü | Açıklama |
|---|---|---|---|
| `upp.dosya.oku` | `metin yol` | `metin` | Dosyanın tüm içeriğini metin olarak okur (`metin_bosalt` gerekir). |
| `upp.dosya.yaz` | `metin yol, metin icerik` | `mantik` | Dosyaya yazar (varsa üzerine yazar). Başarılıysa `dogru`. |
| `upp.dosya.ekle` | `metin yol, metin icerik` | `mantik` | Dosyanın sonuna ekleme yapar. |
| `upp.dosya.var_mi` | `metin yol` | `mantik` | Dosyanın diskte bulunup bulunmadığını kontrol eder. |
| `upp.dosya.sil` | `metin yol` | `mantik` | Dosyayı diskten siler. |
| `upp.dosya.boyut` | `metin yol` | `sayi` | Dosyanın bayt cinsinden boyutunu döner (yoksa `-1`). |

---

## 7. Yol ve Dizin İşlemleri (`upp.yol`)

| Fonksiyon | Parametreler | Dönüş Türü | Açıklama |
|---|---|---|---|
| `upp.yol.birlestir` | `metin a, metin b` | `metin` | İki dosya yolunu platform ayracına göre güvenle birleştirir. |
| `upp.yol.dizin` | `metin yol` | `metin` | Yolun ebeveyn dizin adını döner. |
| `upp.yol.dosya_adi` | `metin yol` | `metin` | Yolun dosya adını (uzantısıyla birlikte) döner. |
| `upp.yol.uzanti` | `metin yol` | `metin` | Dosya uzantısını (`.txt`, `.upp`) döner. |
| `upp.yol.dizin_var_mi` | `metin yol` | `mantik` | Klasörün varlığını sorgular. |
| `upp.yol.dizin_olustur` | `metin yol` | `mantik` | Dizin açar (başarılıysa `dogru`). |
| `upp.yol.listele` | `metin dizin` | `liste[metin]` | Dizindeki dosya ve klasör isimlerini döner. |

---

## 8. Sistem ve Süreç Yönetimi (`upp.sistem`)

```text
sinif SurecCikti {
    sayi kod;
    metin cikti;
    mantik basarili;
}
```

| Fonksiyon | Parametreler | Dönüş Türü | Açıklama |
|---|---|---|---|
| `upp.sistem.calistir` | `metin komut` | `SurecCikti` | Sistem kabuğunda komut çalıştırır; çıkış kodu, çıktı ve başarı durumunu döner. |
| `upp.sistem.ortam_al` | `metin anahtar` | `metin` | Ortam değişkeninin değerini döner (bulunamazsa `""`). |
| `upp.sistem.ortam_yaz` | `metin anahtar, metin deger`| `mantik` | Ortam değişkenini belirler/yazar. Başarılıysa `dogru`. |
| `upp.sistem.pid` | — | `sayi` | Çalışan sürecin işlem kimlik numarasını (PID) döner. |
| `upp.sistem.cikis` | `sayi kod` | `bos` | Süreci verilen çıkış koduyla derhal sonlandırır. |
| `upp.arguman_sayisi` | — | `sayi` | Komut satırından programa geçilen argüman adedi. |
| `upp.arguman` | `sayi i` | `metin` | Belirtilen indeksteki komut satırı argümanı (`0` program yoludur). |

---

## 9. Platforma Özgü API'ler (`upp.windows` ve `upp.linux`)

u++ v3.0 ile platforma özgü işlevler modüler ad alanlarına taşınmıştır.

### Windows API (`upp.windows.*`)

Yalnızca Windows ortamında çalışan sistem ve GUI araçlarıdır.

#### Çekirdek Bellek Erişimi (`upp.windows.bellek.*`)
> ⚠️ **Güvenlik Uyarısı:** `upp.windows.bellek.*` fonksiyonları çekirdek süreç belleğini manipüle ettiğinden **yalnızca `guvensiz { ... }` blokları içerisinde çağrılabilir.**

* `baglan(metin surec_adi) -> sayi`: Hedef sürecin tanıtıcısını (handle) açar.
* `kapat(sayi tutamak) -> bos`: Süreç tanıtıcısını kapatır.
* `modul_bul(metin modul_adi) -> sayi`: Süreç içerisindeki modül veya DLL'in taban adresini bulur.
* `sayioku(sayi adres) -> sayi`: 64-bit bellek alanını okur.
* `sayiyaz(sayi adres, sayi deger) -> mantik`: 64-bit bellek alanına yazar.
* `ondalikoku(sayi adres) -> ondalik`: Kayan noktalı bellek alanını okur.
* `ondalikyaz(sayi adres, ondalik deger) -> mantik`: Kayan noktalı bellek alanına yazar.
* `metinoku(sayi adres, sayi uzunluk) -> metin`: Ham bellekten dize okur.
* `zincir_oku(sayi taban, liste[sayi] ofsetler) -> sayi`: İşaretçi zincirini takip ederek nihai adrese ulaşır.
* `desen_tara(metin modul, metin desen, metin maske) -> sayi`: Bellekte bayt imzası (AOB pattern) arar.
* `yama_yap(sayi adres, metin hex_baytlar) -> mantik`: Bellek adresine doğrudan makine kod baytları yamar.
* `koruma_degistir(sayi adres, sayi boyut, metin bayrak) -> mantik`: Sayfa koruma izinlerini değiştirir.
* `son_hata() -> sayi`: `GetLastError()` kodunu döner.
* `okundu() -> sayi`: Son bellek işleminde başarıyla okunan/yazılan bayt miktarını döner.

#### Kullanıcı Arayüzü ve Girdi (`upp.windows.*`)
* `upp.windows.mesaj(metin baslik, metin govde) -> bos`: Standart Win32 bilgi iletişim kutusu açar.
* `upp.windows.hata(metin baslik, metin govde) -> bos`: Win32 hata uyarı kutusu açar.
* `upp.windows.ses_cal(metin wav_yolu) -> bos`: WAV ses dosyasını asenkron çalar.
* `upp.windows.girdi`: `fare_tasi(x, y)`, `fare_tikla(tus)`, `tus_bas(kod)`, `tus_birak(kod)`, `tus_durumu(kod) -> mantik`.
* `upp.windows.cizim`: `baslat(pencere)`, `cizgi(...)`, `kutu(...)`, `kutu_dolu(...)`, `metin(...)`, `temizle()`, `bitir()`.

---

### Linux API (`upp.linux.*`)

Yalnızca Linux (POSIX) ortamında çalışan sistem ve süreç yönetimi işlevleridir.

* `upp.linux.pid() -> sayi`: Mevcut sürecin PID değerini döner (`getpid`).
* `upp.linux.sinyal_gonder(sayi pid, sayi sinyal) -> sayi`: Belirtilen sürece POSIX sinyali (`SIGTERM`, `SIGKILL` vb.) gönderir.
* `upp.linux.proc_oku(sayi pid, metin dosya) -> metin`: Hedef sürecin `/proc/<pid>/<dosya>` bilgisini okur (`status`, `cmdline` vb.).
* `upp.linux.sayi_oku(sayi pid, sayi adres) -> sayi`: `process_vm_readv` ile hedef süreç belleğinden sayı okur. *(Yalnızca `guvensiz` blokta çağrılabilir)*
* `upp.linux.sayi_yaz(sayi pid, sayi adres, sayi deger) -> mantik`: `process_vm_writev` ile hedef süreç belleğine yazar. *(Yalnızca `guvensiz` blokta çağrılabilir)*

---

## 10. Matematik Kütüphanesi (`upp.matematik`)

| Fonksiyon | Parametreler | Dönüş Türü | Açıklama |
|---|---|---|---|
| `upp.matematik.karekok` | `ondalik x` | `ondalik` | Karekök hesabı (`sqrt`). |
| `upp.matematik.us` | `ondalik taban, ondalik us` | `ondalik` | Üs alma (`pow`). |
| `upp.matematik.sinus` | `ondalik radyan` | `ondalik` | Sinüs açısı (`sin`). |
| `upp.matematik.kosinus` | `ondalik radyan` | `ondalik` | Kosinüs açısı (`cos`). |
| `upp.matematik.tanjant` | `ondalik radyan` | `ondalik` | Tanjant açısı (`tan`). |
| `upp.matematik.taban` | `ondalik x` | `sayi` | Aşağı yuvarlar (`floor`). |
| `upp.matematik.tavan` | `ondalik x` | `sayi` | Yukarı yuvarlar (`ceil`). |
| `upp.matematik.yuvarla` | `ondalik x` | `sayi` | En yakın tam sayıya yuvarlar (`round`). |
| `upp.matematik.pi` | — | `ondalik` | Pi sayısı sabiti ($3.141592653589793$). |
| `upp.matematik.e` | — | `ondalik` | Euler sayısı sabiti ($2.718281828459045$). |

---

## 11. Veri Yapıları: `liste` ve `harita`

### `liste[T]` (Dinamik Dizi)

| Metot | İmzası | Açıklama |
|---|---|---|
| `.ekle(eleman)` | `(T) -> bos` | Listenin sonuna yeni eleman ekler. |
| `.al(indeks)` | `(sayi) -> T` | Belirtilen indisteki elemanı döner. Hatalı indekste program durur. |
| `.yaz(indeks, deger)` | `(sayi, T) -> bos` | Belirtilen indeksteki elemanın değerini günceller. |
| `.sil(indeks)` | `(sayi) -> bos` | Belirtilen indeksteki elemanı listeden kaldırır ve diziyi kaydırır. |
| `.temizle()` | `() -> bos` | Listenin tüm elemanlarını siler ve uzunluğu `0` yapar. |
| `.uzunluk()` | `() -> sayi` | Listenin mevcut eleman sayısını döner. |
| `.bosalt()` | `() -> bos` | Listenin tüm hafızasını serbest bırakır. |

### `harita[K, V]` (Sözlük / Hash Map)

| Metot | İmzası | Açıklama |
|---|---|---|
| `.koy(anahtar, deger)` | `(K, V) -> bos` | Anahtar-değer çiftini ekler veya günceller. |
| `.al(anahtar)` | `(K) -> V` | Anahtarın karşılığı olan değeri döner. |
| `.var_mi(anahtar)` | `(K) -> mantik` | Anahtarın haritada mevcut olup olmadığını sorgular. |
| `.sil(anahtar)` | `(K) -> bos` | Anahtarı ve ilişkili değerini haritadan kaldırır. |
| `.temizle()` | `() -> bos` | Haritadaki tüm kayıtları siler. |
| `.uzunluk()` | `() -> sayi` | Haritadaki kayıt sayısını döner. |
| `.anahtarlar()` | `() -> liste[K]` | Eklenme sırasına göre tüm anahtarları liste olarak döner. |
| `.bosalt()` | `() -> bos` | Harita hafızasını temizler. |

---

## 12. JSON Kütüphanesi (`upp.json`)

| Fonksiyon | Parametreler | Dönüş Türü | Açıklama |
|---|---|---|---|
| `upp.json.ayristir` | `metin json_metni` | `JSONDeger` | JSON metnini ayrıştırır. Başarısız olursa `id = 0` olur. |
| `upp.json.anahtar_al` | `JSONDeger d, metin anahtar` | `JSONDeger` | Nesne içindeki belirtilen alanın tutamacını döner. |
| `upp.json.indeks_al` | `JSONDeger d, sayi indeks` | `JSONDeger` | Dizi içindeki elemanın tutamacını döner. |
| `upp.json.metin_al` | `JSONDeger d` | `metin` | Düğümün dize değerini döner (`metin_bosalt` gerekir). |
| `upp.json.sayi_al` | `JSONDeger d` | `sayi` | Düğümün tam sayı değerini döner. |
| `upp.json.ondalik_al` | `JSONDeger d` | `ondalik` | Düğümün ondalık değerini döner. |
| `upp.json.mantik_al` | `JSONDeger d` | `mantik` | Düğümün mantıksal değerini döner. |
| `upp.json.uzunluk` | `JSONDeger d` | `sayi` | JSON nesnesi veya dizisindeki eleman sayısını döner. |
| `upp.json.tur` | `JSONDeger d` | `metin` | Düğüm türünü verir: `"nesne"`, `"dizi"`, `"metin"`, `"sayi"`, `"mantik"`, `"bosluk"`. |
| `upp.json.var_mi` | `JSONDeger d` | `mantik` | Tutamacın geçerli olup olmadığını doğrular. |
| `upp.json.bosalt` | `JSONDeger kok` | `bos` | Ayrıştırılan tüm JSON ağacını hafızadan siler. |

---

## 13. Eşzamanlılık ve Senkronizasyon: `arkaplan` ve `Kilit`

### `arkaplan` ve `ArkaplanIs`

```text
oto is = arkaplan {
    upp.satir_yaz("Arka planda calisiyor");
};
is.bekle(); // İş parçacığının tamamlanmasını bekler
```

* **Yakalama:** Kapsamdan kullanılan `sayi`, `bayt` ve `metin` değerleri **kopyalanarak** iş parçacığına aktarılır.
* **Kısıtlar:** Koleksiyonlar (`liste`, `harita`) ve `JSONDeger` tutamaçları veri yarışı riskine karşı iş parçacığı içine yakalanamaz.

### `Kilit` (Mutex)

| Metot | İmzası | Açıklama |
|---|---|---|
| `.kilitle()` | `() -> bos` | Kilidi alır; kilit başkasındaysa serbest kalana dek bekler. |
| `.birak()` | `() -> bos` | Kilidi serbest bırakır. |

---

## 14. Düşük Seviyeli Araçlar (`guvensiz` Alanı)

Yalnızca `guvensiz { ... }` bloğu içerisinde izin verilen işlemler:
1. Ham işaretçi tanımlama ve yönlendirme: `sayi* p`, `&x`.
2. Ham bellek tahsisi: `bellek_ayir(boyut)`, `bellek_bosalt(adres)`.
3. Satır içi GNU C kodu enjeksiyonu: `c_kod { printf("Ham C kodu\n"); }`.
4. Harici süreç belleği erişimi: `upp.windows.bellek.*` ve `upp.linux.sayi_oku / sayi_yaz`.

---

## 15. LSP ve Stdio Çerçevesi (`upp.stdio`)

JSON-RPC ve Language Server Protocol (LSP) protokolü için standart giriş/çıkış akışını ikili çerçeveleme moduna alır (`Content-Length: N\r\n\r\n`).

* `upp.stdio.gelen_oku() -> metin`: LSP başlığını ve gövdesini okuyup JSON metnini döner (`metin_bosalt` gerekir).
* `upp.stdio.yaz(metin json_metin) -> bos`: Verilen JSON metnini uygun `Content-Length` başlığı ile `stdout`'a gönderir.

---

## 16. Derleyici Komut Satırı Arayüzü (CLI)

u++ derleyicisi (`uppc` / `uppc.exe`), C çalışma zamanı kütüphanesini kendi içine gömülü olarak taşıyan tek ve bağımsız (standalone) bir çalıştırılabilir dosyadır.

```bash
uppc <kaynak.upp> [bayraklar]
```

### Bayraklar

| Bayrak | Alternatif | Açıklama |
|---|---|---|
| `-o <AD>` | `--cikti <AD>` | Çıktı dosya adı tabanını belirler (`<AD>.c` ve `<AD>.exe` / `<AD>.out`). |
| `-c` | `--sadece-c` | Yalnızca C kodunu üretir; GCC derleyicisini çağırmaz. |
| `--sadece-derle` | — | İkili dosyayı derler ancak otomatik olarak çalıştırmaz. |
| `--runtime <YOL>` | — | Gömülü çalışma zamanı yerine özel bir `upp_runtime.c` dosyası kullanır. |
| `-v` | `--surum` | Derleyici sürüm bilgisini basar (`u++ v3.0 (beta)`). |
| `-h` | `--yardim` | Kullanım ve seçenekler yardım mesajını gösterir. |
| `--linux` | — | Hedef ABI'yi Linux (POSIX) olarak belirler. |
| `--windows` | — | Hedef ABI'yi Windows (Win32) olarak belirler. |
| `--analiz` | — | Canlı düzenleyici tanısı. C üretmez ve GCC çağırmaz. 50 hataya kadar JSON döner. |
| `--json-hata` | — | Tam derleme yolunu (C + GCC) dener; sonucu tek satır JSON olarak basar. |
| `--bicim` | — | Kaynak dosyayı yerinde standart 4 boşluk girintili u++ biçimine sokar. |
| `--bicim-kontrol` | — | Dosya biçimlendirilmemişse hata kodu `1` ile çıkar (CI ortamları için). |
| `--ast` | — | Ayrıştırılan sözdizim ağacını (AST) ekrana döker. |

---

## 17. C Tipi ve Sembol Eşleme Tablosu

| u++ Tanımı | Üretilen C Karşılığı |
|---|---|
| `sayi` | `int64_t` |
| `ondalik` | `double` |
| `bayt` | `uint8_t` |
| `metin` | `char*` |
| `mantik` | `bool` |
| `bos` | `void` |
| `liste[T]` | `upp_liste_t*` |
| `harita[K, V]` | `upp_harita_t*` |
| `upp.yaz(...)` | `upp_yaz(...)` |
| `upp.windows.bellek.sayioku(...)` | `upp_windows_bellek_sayioku(...)` |
| `upp.linux.sayi_oku(...)` | `upp_linux_sayi_oku(...)` |
| `upp.sistem.pid()` | `upp_sistem_pid()` |
