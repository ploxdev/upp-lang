# u++ Standart Kütüphane ve Kamu API Referansı (v3.0)

Bu belge, **u++ v3.0** dilinin standart kütüphanesini (`upp.*`), yerleşik türlerini, bellek sahipliği kurallarını ve derleyici arayüzünü tanımlayan resmi Türkçe teknik referanstır.

Sıfırdan dil eğitimi için [OGREN.md](../OGREN.md) kılavuzunu, derleyici iç mimarisi için [derleyici.md](../derleyici.md) belgesini inceleyebilirsiniz. İngilizce eşleniği: [API_en.md](API_en.md).

---

## İçindekiler

1. [Genel Sözleşme ve Bellek Kuralları](#1-genel-sözleşme-ve-bellek-kuralları)
2. [Konsol, Giriş ve Kullanıcı Etkileşimi](#2-konsol-giriş-ve-kullanıcı-etkileşimi)
3. [Dönüşümler ve Yardımcı İşlevler](#3-dönüşümler-ve-yardımcı-işlevler)
4. [Metin İşlemleri (`upp.metin`)](#4-metin-işlemleri-uppmetin)
5. [Dosya Sistemi (`upp.dosya`)](#5-dosya-sistemi-uppdosya)
6. [Yol ve Dizin İşlemleri (`upp.yol`)](#6-yol-ve-dizin-işlemleri-uppyol)
7. [Sistem ve Süreç Yönetimi (`upp.sistem`)](#7-sistem-ve-süreç-yönetimi-uppsistem)
8. [Matematik Kütüphanesi (`upp.matematik`)](#8-matematik-kütüphanesi-uppmatematik)
9. [Veri Yapıları: `liste` ve `harita`](#9-veri-yapıları-liste-ve-harita)
10. [JSON Kütüphanesi (`upp.json`)](#10-json-kütüphanesi-uppjson)
11. [Eşzamanlılık ve Senkronizasyon: `arkaplan` ve `Kilit`](#11-eşzamanlılık-ve-senkronizasyon-arkaplan-ve-kilit)
12. [Düşük Seviyeli Araçlar (`guvensiz` Alanı)](#12-düşük-seviyeli-araçlar-guvensiz-alanı)
13. [LSP ve Stdio Çerçevesi (`upp.stdio`)](#13-lsp-ve-stdio-çerçevesi-uppstdio)
14. [Derleyici Komut Satırı Arayüzü (CLI) ve JSON Tanı Şeması](#14-derleyici-komut-satırı-arayüzü-cli-ve-json-tanı-şeması)
15. [C Tipi ve Sembol Eşleme Tablosu](#15-c-tipi-ve-sembol-eşleme-tablosu)

---

## 1. Genel Sözleşme ve Bellek Kuralları

* **Metin Formatı:** Tüm metinler UTF-8 kodlamasındadır. `s[i]`, `upp.uzunluk(s)` ve `upp.metin.kod(s, i)` çağrıları ham C baytlarını değil, doğrudan Unicode **karakter kod noktasını (code point)** döndürür. Geçersiz indeksler `0` verir.
* **Bellek Sahipliği (`upp.metin_bosalt`):**
  - Dinamik olarak öbekte (`malloc`) oluşturulan metinlerin (`upp.giris()`, `upp.dosya_oku()`, `upp.sistem.calistir()` çıktısı, çalışma anındaki metin birleştirmeleri `+` ve dize interpolasyonları) işi bittiğinde `upp.metin_bosalt(s)` ile serbest bırakılması gerekir.
  - Sabit dizeler (`"merhaba"`), derleme anında birleştirilen sabitler, `upp.arguman(i)` (tüm süreç boyu yaşar) ve sayısal dönüşler (`kod`, `ileri`) için boşaltma çağrısı **yapılmaz**.
* **Güvenlik Sınırları:**
  - Varsayılan u++ kodunda ham işaretçiler (`*`, `&`), doğrudan bellek adresleme (`bellek_ayir`, `bellek_bosalt`), satır içi C kodları (`c_kod { }`) ve harici süreç belleği erişimi (`upp.bellek.*`) yasaktır.
  - Bu işlemlere sadece **`guvensiz { ... }`** blokları içerisinde izin verilir.
* **Hata Yönetimi:**
  - Tam derleme ilk ölümcül hatada durur ve çıkış kodu verir.
  - Canlı düzenleyici analizi (`--analiz`), derleme öncesi tür ve güvenlik hatalarını 50 adede kadar biriktirerek yapılandırılmış JSON formatında raporlar.

---

## 2. Konsol, Giriş ve Kullanıcı Etkileşimi

### Fonksiyon İmzaları ve Açıklamaları

| Fonksiyon | Parametreler | Dönüş Türü | Açıklama |
|---|---|---|---|
| `upp.yaz` | `metin s` | `bos` | Standart çıktıya (`stdout`) metin yazar. Dize interpolasyonu destekler. |
| `upp.satir_yaz` | `metin s` | `bos` | Metni yazar ve sonuna yeni satır karakteri (`\n`) ekler. |
| `upp.hata_yaz` | `metin s` | `bos` | Standart hata akışına (`stderr`) metin yazar. |
| `upp.hata_satir_yaz` | `metin s` | `bos` | Standart hata akışına (`stderr`) satır sonu ile yazar. |
| `upp.giris` | — | `metin` | Kullanıcıdan konsoldan bir satır okur. Dönen metin `metin_bosalt` gerektirir. |
| `upp.mesaj` | `metin baslik, metin govde` | `bos` | Bilgi iletişim kutusu gösterir (Windows `MessageBox`; Linux'ta no-op). |
| `upp.hata` | `metin baslik, metin govde` | `bos` | Hata iletişim kutusu gösterir (Windows `MessageBox` Hata simgeli). |
| `upp.ses_cal` | `metin wav_yolu` | `bos` | WAV ses dosyasını çalar (Windows; Linux'ta no-op). |

### Örnek Kullanım

```text
fonk ana() -> sayi {
    upp.satir_yaz("Lutfen adinizi girin: ");
    oto isim = upp.giris();

    eger (upp.uzunluk(isim) == 0) {
        upp.hata_satir_yaz("Hata: Isim alani bos birakilamaz!");
    } yoksa {
        upp.satir_yaz("Merhaba, {isim}!");
    }

    upp.metin_bosalt(isim);
    don 0;
}
```

---

## 3. Dönüşümler ve Yardımcı İşlevler

| Fonksiyon | Parametreler | Dönüş Türü | Açıklama |
|---|---|---|---|
| `upp.uzunluk` | `metin` veya `dizi[]` | `sayi` | Metin için UTF-8 karakter sayısı, dizi için eleman sayısı döner. |
| `upp.metinden_sayiya` | `metin s` | `sayi` | Metni 64-bit tam sayıya çevirir (`atoll`). |
| `upp.sayidan_metin` | `sayi n` | `metin` | Sayıyı metne çevirir (`metin_bosalt` gerekir). |
| `upp.ondalikdan_metin` | `ondalik d` | `metin` | Ondalıklı sayıyı metne çevirir (`metin_bosalt` gerekir). |
| `upp.metinden_ondalik` | `metin s` | `ondalik` | Metni ondalıklı sayıya çevirir (`atof`). |
| `upp.min` | `sayi a, sayi b` | `sayi` | İki sayıdan küçük olanı döner. |
| `upp.max` | `sayi a, sayi b` | `sayi` | İki sayıdan büyük olanı döner. |
| `upp.mutlak` | `sayi n` | `sayi` | Sayının mutlak değerini döner. |
| `upp.zaman` | — | `sayi` | Sürecin başlangıcından bu yana geçen milisaniye (monotonic). |
| `upp.uyut` | `sayi milisaniye` | `bos` | Belirtilen milisaniye kadar akışı duraklatır. |
| `upp.rastgele` | `sayi min, sayi max` | `sayi` | Verilen kapalı aralıkta (`[min, max]`) rastgele 64-bit sayı üretir. |
| `upp.metin_bosalt` | `metin s` | `bos` | Öbekte ayrılmış dinamik dizeyi serbest bırakır. |

---

## 4. Metin İşlemleri (`upp.metin`)

Dize manipülasyonu için zengin yerleşik işlevler kümesidir.

| Fonksiyon | Parametreler | Dönüş Türü | Bellek / Not |
|---|---|---|---|
| `upp.metin.kod` | `metin s, sayi i` | `sayi` | `i`. karakterin Unicode kod noktası. `s[i]` ile eşdeğerdir. |
| `upp.metin.ileri` | `metin s, sayi i` | `sayi` | `i`. karakterin UTF-8 bayt uzunluğu (1 ile 4 arası). |
| `upp.metin.kes` | `metin s, sayi basla, sayi uz` | `metin` | `basla` indeksinden `uz` karakter kadar kesit alır. (`metin_bosalt` ister). |
| `upp.metin.icinde` | `metin kaynak, metin aranan` | `mantik` | `aranan` alt metni kaynakta geçiyor mu? |
| `upp.metin.degistir` | `metin s, metin eski, metin yeni` | `metin` | Tüm eşleşmeleri değiştirip yeni metin döner (`metin_bosalt` ister). |
| `upp.metin.kirp` | `metin s` | `metin` | Baştaki ve sondaki boşlukları temizler (`metin_bosalt` ister). |
| `upp.metin.buyuk` | `metin s` | `metin` | Karakterleri büyük harfe çevirir (`metin_bosalt` ister). |
| `upp.metin.kucuk` | `metin s` | `metin` | Karakterleri küçük harfe çevirir (`metin_bosalt` ister). |
| `upp.metin.bol` | `metin s, metin ayirac` | `liste[metin]` | Metni ayıraca göre böler. Boş ayıraç karakter karakter böler. |
| `upp.metin.birlestir` | `liste[metin] l, metin ayirac` | `metin` | Liste elemanlarını ayraçla birleştirir (`metin_bosalt` ister). |

### Örnek Kullanım

```text
metin ham = "  elma,armut,muz  ";
metin temiz = upp.metin.kirp(ham);
liste[metin] meyveler = upp.metin.bol(temiz, ",");

her (oto meyve in meyveler) {
    upp.satir_yaz("Meyve: {meyve}");
}

metin birlesik = upp.metin.birlestir(meyveler, " - ");
upp.satir_yaz("Sonuc: {birlesik}");

upp.metin_bosalt(temiz);
upp.metin_bosalt(birlesik);
meyveler.bosalt();
```

---

## 5. Dosya Sistemi (`upp.dosya`)

| Fonksiyon | Parametreler | Dönüş Türü | Açıklama |
|---|---|---|---|
| `upp.dosya_oku` | `metin yol` | `metin` | Dosyanın tüm içeriğini okur. Bulunamazsa `yok` döner. (`metin_bosalt` ister). |
| `upp.dosya_yaz` | `metin yol, metin icerik` | `mantik` | Dosyayı baştan oluşturur veya üzerine yazar. Başarı durumu döner. |
| `upp.dosya_ekle` | `metin yol, metin icerik` | `mantik` | Dosyanın sonuna ekleme yapar (append). |
| `upp.dosya_var_mi` | `metin yol` | `mantik` | Dosyanın varlığını denetler. |
| `upp.dosya_sil` | `metin yol` | `mantik` | Dosyayı siler. |
| `upp.dosya.listele` | `metin yol` | `liste[metin]` | Belirtilen dizin altındaki dosya ve klasör isimlerini listeler. |
| `upp.ortam_al` | `metin degisken_adi` | `metin` | İşletim sistemi ortam değişkenini okur (yoksa `yok`). |

---

## 6. Yol ve Dizin İşlemleri (`upp.yol`)

Platform bağımsız dosya yolu manipülasyonu ve klasör yönetimi sağlar.

| Fonksiyon | Parametreler | Dönüş Türü | Açıklama |
|---|---|---|---|
| `upp.yol.dizin` | `metin yol` | `metin` | Dosya yolunun üst dizinini döner (`metin_bosalt` ister). |
| `upp.yol.birlestir` | `metin yol1, metin yol2` | `metin` | İki yolu işletim sistemi ayracına (`/` veya `\`) göre birleştirir. |
| `upp.yol.gercek` | `metin yol` | `metin` | Göreli yolu mutlak kanonik yola çevirir (`realpath`). |
| `upp.yol.calisma` | — | `metin` | Sürecin o anki çalışma dizinini (CWD) döner. |
| `upp.yol.mutlak_mi` | `metin yol` | `mantik` | Yolun mutlak olup olmadığını denetler. |
| `upp.yol.klasor_olustur` | `metin yol` | `mantik` | Yol üzerindeki tüm eksik dizinleri oluşturur (`mkdir -p`). |

---

## 7. Sistem ve Süreç Yönetimi (`upp.sistem`)

### Komut Satırı Argümanları

| Fonksiyon | Dönüş Türü | Açıklama |
|---|---|---|
| `upp.arguman_sayisi()` | `sayi` | Çalıştırılan programın aldığı argüman sayısı (`argc`). |
| `upp.arguman(sayi i)` | `metin` | `i`. sıradaki argümanı döner (0 ikilinin kendi yoludur). Boşaltma **gerekmez**. |

### `SurecCikti` Sınıfı ve Sistem Çağrıları

```text
sinif SurecCikti {
    metin cikti; // stdout ve stderr birleşik çıktısı (malloc)
    sayi kod;    // Sürecin çıkış kodu (0 = başarı)
}
```

| Fonksiyon | Parametreler | Dönüş Türü | Açıklama |
|---|---|---|---|
| `upp.sistem.calistir` | `metin komut` | `SurecCikti` | Terminal komutunu çalıştırır. Çıktı `metin_bosalt` ister. |
| `upp.sistem.komut_bul` | `metin komut_adi` | `metin` | Komutun `PATH` üzerindeki tam yolunu arar (`which`/`where`). |
| `upp.sistem.exe_dizin` | — | `metin` | Çalışan ikili dosyanın bulunduğu klasörü döner. |

> **Güvenlik Uyarısı:** `upp.sistem.calistir` kabuk üzerinden yürütülür (Windows `cmd.exe /C`, Linux `popen`). Dışarıdan veya kullanıcıdan alınan kontrolsüz girdileri doğrudan komut dizesine eklemeyiniz.

---

## 8. Matematik Kütüphanesi (`upp.matematik`)

Tüm trigonometrik fonksiyonlar **radyan** cinsinden çalışır.

### Temel Fonksiyonlar
* Trigonometri: `sin(r)`, `cos(r)`, `tan(r)`, `asin(v)`, `acos(v)`, `atan(v)`, `atan2(y, x)` (`ondalik -> ondalik`).
* Üs ve Kök: `karekok(x)`, `us(taban, us)`.
* Yuvarlama: `yuvarla(x)`, `taban(x)` (floor), `tavan(x)` (ceil).
* Açı Dönüşümü: `radyan(derece)`, `derece(radyan)`.

### Geometri ve Vektör Yapıları

```text
sinif Vektor3 { ondalik x; ondalik y; ondalik z; }
sinif Yonelme { ondalik pitch; ondalik yaw; }
sinif EkranNokta { ondalik x; ondalik y; mantik gorunur; }
```

| Fonksiyon | İmzası | Açıklama |
|---|---|---|
| `mesafe` | `(Vektor3 a, Vektor3 b) -> ondalik` | İki nokta arasındaki 3B Öklid mesafesini hesaplar. |
| `mesafe_3d` | `(x1, y1, z1, x2, y2, z2) -> ondalik` | Ham koordinatlarla 3B mesafe hesaplar. |
| `aci_hesapla` | `(x1, y1, z1, x2, y2, z2) -> Yonelme` | İki nokta arasındaki hedef açısını (`pitch` ve `yaw`) verir. |
| `dunya_ekran` | `(...) -> EkranNokta` | 3B dünya koordinatını projeksiyon matrisiyle 2B ekrana yansıtır. |

---

## 9. Veri Yapıları: `liste` ve `harita`

### Dinamik Liste: `liste[T]`
Desteklenen eleman türleri (`T`): `sayi`, `ondalik`, `mantik`, `metin`.

| Metot | İmzası | Açıklama |
|---|---|---|
| `.ekle(eleman)` | `(T) -> bos` | Listenin sonuna yeni eleman ekler. |
| `.al(indeks)` | `(sayi) -> T` | Belirtilen indeksteki elemanı döner. `metin` durumunda kopya verir. |
| `.yaz(indeks, v)` | `(sayi, T) -> bos` | Belirtilen indeksteki elemanı günceller. `liste[i] = v` de geçerlidir. |
| `.uzunluk()` | `() -> sayi` | Listedeki eleman sayısını verir. |
| `.bosalt()` | `() -> bos` | Listenin kullandığı tüm hafızayı serbest bırakır. |

### Anahtar-Değer Haritası: `harita[K, V]`
* Anahtar türleri (`K`): `metin`, `sayi`.
* Değer türleri (`V`): `sayi`, `ondalik`, `mantik`, `metin`.

| Metot | İmzası | Açıklama |
|---|---|---|
| `.koy(anahtar, deger)` | `(K, V) -> bos` | Çifte anahtar ekler veya varsa değerini günceller. |
| `.al(anahtar)` | `(K) -> V` | Anahtarın karşılığı olan değeri döner. |
| `.var_mi(anahtar)` | `(K) -> mantik` | Anahtarın varlığını sorgular. |
| `.uzunluk()` | `() -> sayi` | Haritadaki kayıt sayısını döner. |
| `.anahtarlar()` | `() -> liste[K]` | Eklenme sırasına göre tüm anahtarları liste olarak döner. |
| `.bosalt()` | `() -> bos` | Harita hafızasını temizler. |

---

## 10. JSON Kütüphanesi (`upp.json`)

JSON ağaçları C seviyesinde bir havuzda yönetilir. u++ tarafında `JSONDeger` tutamacı (`sayi id`) üzerinden erişilir (`id == 0` geçersiz düğümdür).

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

## 11. Eşzamanlılık ve Senkronizasyon: `arkaplan` ve `Kilit`

### `arkaplan` ve `ArkaplanIs`
Yeni bir işletim sistemi iş parçacığı (POSIX thread veya Windows thread) açar:

```text
oto is = arkaplan {
    upp.satir_yaz("Arka planda calisiyor");
};
is.bekle(); // veya: upp.bekle(is);
```

* **Yakalama:** Kapsamdan kullanılan `sayi` ve `metin` değerleri **kopyalanarak** iş parçacığına aktarılır.
* **Kısıtlar:** Koleksiyonlar (`liste`, `harita`) ve `JSONDeger` tutamaçları veri yarışı riskine karşı iş parçacığı içine yakalanamaz.

### `Kilit` (Mutex)
Ortak kaynaklara sıralı erişimi garanti altına alır. Tanımlandığı anda işletim sistemi mutex nesnesi bağlanır.

| Metot | İmzası | Açıklama |
|---|---|---|
| `.kilitle()` | `() -> bos` | Kilidi alır; kilit başkasındaysa serbest kalana dek bekler. |
| `.birak()` | `() -> bos` | Kilidi serbest bırakır. |

---

## 12. Düşük Seviyeli Araçlar (`guvensiz` Alanı)

Bu araçlar sadece `guvensiz { ... }` blokları içerisinde kullanılabilir.

### Süreç Belleği Erişimi (`upp.bellek`)
* `surec_ac(surec_adi) -> sayi`: Hedef sürecin tanıtıcısını (handle) açar.
* `surec_kapat(tutamak) -> bos`: Süreç tanıtıcısını kapatır.
* `modul_temel_adresi(tutamak, modul) -> sayi`: Süreç içindeki DLL veya modülün temel adresini bulur.
* `sayi_oku(tutamak, adres) -> sayi`: 64-bit bellek okur.
* `sayi_yaz(tutamak, adres, deger) -> mantik`: 64-bit bellek yazar.
* `ondalik_oku` / `ondalik_yaz`: Ondalıklı bellek işlemleri.
* `metin_oku(tutamak, adres, uzunluk) -> metin`: Ham bellekten dize okur.
* `zincir_oku(tutamak, temel, ofsetler[]) -> sayi`: Pointer zincirini takip ederek nihai adresi hesaplar.
* `desen_tara(tutamak, modul, desen, maske) -> sayi`: Bellekte bayt deseni (AOB signature) arar.

### Sentetik Girdi ve Overlay Çizim
* `upp.girdi`: `fare_tasi(x, y)`, `fare_tikla(tus)`, `tus_bas(kod)`, `tus_birak(kod)`, `tus_durumu(kod) -> mantik`. (Windows; Linux'ta no-op).
* `upp.cizim`: `baslat(surec_pencere)`, `cizgi(...)`, `kutu(...)`, `kutu_dolu(...)`, `metin(...)`, `temizle()`, `bitir()`.

---

## 13. LSP ve Stdio Çerçevesi (`upp.stdio`)

JSON-RPC ve Language Server Protocol (LSP) protokolü için standart giriş/çıkış akışını ikili çerçeveleme moduna alır (`Content-Length: N\r\n\r\n`).

* `upp.stdio.gelen_oku() -> metin`: LSP başlığını ve gövdesini okuyup JSON metnini döner (`metin_bosalt` gerekir).
* `upp.stdio.yaz(json_metin) -> bos`: Verilen JSON metnini uygun `Content-Length` başlığı ile `stdout`'a gönderir.

---

## 14. Derleyici Komut Satırı Arayüzü (CLI) ve JSON Tanı Şeması

Hem Python derleyicisi (`uppc.py`) hem de native derleyici (`uppc.exe`) tamamen aynı bayrakları kabul eder:

```bash
python uppc.py <kaynak.upp> [bayraklar]
```

### Bayraklar

| Bayrak | Açıklama |
|---|---|
| `--cikti <AD>` | Çıktı dosya adı tabanı. `<AD>.c` ve `<AD>.exe` / `<AD>.out` üretir. |
| `--sadece-derle` | İkili dosyayı derler ancak otomatik olarak çalıştırmaz. |
| `--sadece-c` | Yalnızca C kodunu üretir; GCC derleyicisini çağırmaz. |
| `--linux` | Hedef ABI'yi Linux (POSIX) olarak belirler. |
| `--windows` | Hedef ABI'yi Windows (Win32) olarak belirler. |
| `--analiz` | Canlı düzenleyici tanısı. C üretmez ve GCC çağırmaz. 50 hataya kadar JSON döner. |
| `--json-hata` | Tam derleme yolunu (C + GCC) dener; sonucu tek satır JSON olarak basar. |
| `--bicim` | Kaynak dosyayı yerinde standart 4 boşluk girintili u++ biçimine sokar. |
| `--bicim-kontrol` | Dosya biçimlendirilmemişse hata kodu `1` ile çıkar (CI ortamları için). |
| `--ast` | Ayrıştırılan sözdizim ağacını (AST) ekrana döker. |

### Düzenleyici JSON Tanı Şeması (`--analiz`)

```json
{
  "ok": false,
  "hatalar": [
    {
      "satir": 12,
      "sutun": 8,
      "tur": "hata",
      "mesaj": "Tanimlanmamis degisken: 'skor'"
    }
  ]
}
```

* `tur` değerleri: `"hata"` (sözdizimi veya tür hatası), `"guvenlik"` (bellek güvenliği ihlali), `"gcc"` (bağlayıcı veya C hatası).

---

## 15. C Tipi ve Sembol Eşleme Tablosu

| u++ Tipi / İfadesi | C11 Karşılığı | Not |
|---|---|---|
| `sayi` | `long long` | 64-bit işaretli tam sayı |
| `ondalik` | `double` | 64-bit IEEE 754 kayan noktalı |
| `metin` | `char*` | UTF-8 dize |
| `mantik` | `bool` / `int` | `1` veya `0` |
| `bos` | `void` | |
| `yok` | `NULL` | |
| `liste[T]` | `UppKolListe` | Dinamik dizi yapısı |
| `harita[K, V]` | `UppKolHarita` | Hash tablosu |
| `SurecCikti` | `struct UppSurecCikti` | `{ char* cikti; long long kod; }` |
| `Kilit` | `struct UppKilit` | İşletim sistemi mutex tanıtıcısı |
| `s[i]` | `upp_metin_kod(s, i)` | Unicode kod noktası dönüşü |
| `s += t` | `upp_metin_ekle(&s, t)` | Eski belleği serbest bırakıp yeniden boyutlandırır |
| `upp.sistem.calistir` | `upp_sistem_calistir(...)` | `_popen` / `popen` |
