# u++ ile Programlamaya Giriş ve Dil Rehberi

Bu rehber, **u++ (u-plus-plus)** programlama dilini sıfırdan öğrenmeniz için hazırlanmış kapsamlı bir kitaptır.

Bu rehber en temelden ileri seviyeye kadar hazırlandı. Bu dokümanda değişkenlerden döngülere, nesne yönelimli programlamadan çoklu iş parçacıklarına (multithreading) ve düşük seviyeli sistem güvenliğine kadar tüm kavramları adım adım, bol örnek ve açıklamalarla öğreneceksiniz.

---

## İçindekiler

1. [Programlama ve u++ Dünyasına Giriş](#1-programlama-ve-u-dünyasına-giriş)
2. [Kurulum ve İlk Derleme](#2-kurulum-ve-ilk-derleme)
3. [İlk Programımız: "Merhaba Dünya!"](#3-ilk-programımız-merhaba-dünya)
4. [Değişkenler ve Temel Veri Tipleri](#4-değişkenler-ve-temel-veri-tipleri)
5. [Metinler (Strings) ve Dize İnterpolasyonu](#5-metinler-strings-ve-dize-interpolasyonu)
6. [Operatörler ve Matematiksel Hesaplamalar](#6-operatörler-ve-matematiksel-hesaplamalar)
7. [Karar Yapıları: `eger`, `yoksa` ve `secim`](#7-karar-yapıları-eger-yoksa-ve-secim)
8. [Döngüler: `dongu`, `iken` ve `her`](#8-döngüler-dongu-iken-ve-her)
9. [Koleksiyonlar: Sabit Diziler, `liste` ve `harita`](#9-koleksiyonlar-sabit-diziler-liste-ve-harita)
10. [Fonksiyonlar (Alt Programlar)](#10-fonksiyonlar-alt-programlar)
11. [Nesne Yönelimli Programlama: Sınıflar (`sinif`)](#11-nesne-yönelimli-programlama-sınıflar-sinif)
12. [Modüler Kod Mimarisi: `kullan` ve `.uph` Başlıkları](#12-modüler-kod-mimarisi-kullan-ve-uph-başlıkları)
13. [Eşzamanlılık: `arkaplan` ve `Kilit`](#13-eşzamanlılık-arkaplan-ve-kilit)
14. [Standart Kütüphane (`upp.*`) ile Pratik İşler](#14-standart-kütüphane-upp-ile-pratik-işler)
15. [Bellek Yönetimi ve `guvensiz` Kapısı](#15-bellek-yönetimi-ve-guvensiz-kapısı)
16. [Geliştirici Araçları ve VS Code / Cursor Entegrasyonu](#16-geliştirici-araçları-ve-vs-code--cursor-entegrasyonu)
17. [Kapsamlı Bir Proje: Konsol Not Defteri](#17-kapsamlı-bir-proje-konsol-not-defteri)

---

## 1. Programlama ve u++ Dünyasına Giriş

### Programlama Nedir?
Bilgisayarlar temelde milyarlarca minik elektrik anahtarından (transistörlerden) oluşan, son derece hızlı ama kendi başına düşünemeyen makinelerdir. Bir bilgisayara ne yapması gerektiğini söyleyen adım adım talimatlar bütününe **program (veya yazılım)**, bu talimatları yazma sürecine ise **programlama** diyoruz.

### u++ Nedir?
**u++**, kaynak kodları **Türkçe** kelimelerle (`fonk`, `eger`, `dongu`, `sinif` gibi) yazılan, modern bir **sistem programlama dilidir**. 

Pek çok dilden farklı olarak u++ bir "yorumlayıcı" (interpreter) kullanmaz. Yani Python veya JavaScript gibi kod satırlarını çalışırken tek tek okumak yerine:
1. Yazdığınız Türkçe u++ kodunu yüksek verimli **GNU C11** koduna dönüştürür (transpile eder).
2. Ardından sektör standardı **GCC** derleyicisi ile doğrudan makinenizin anladığı saf makine koduna (Windows'ta `.exe`, Linux'ta `.out`) bağlar.
3. Sonuç olarak C ve C++ dilleri kadar hızlı, doğrudan işlemci üzerinde çalışan bağımsız programlar elde edersiniz.

---

## 2. Kurulum ve İki Derleyicili Mimari

u++ ile çalışabilmek için bilgisayarınızda iki temel araca ihtiyaç vardır:
1. **Python 3.8 veya üzeri:** u++ derleyicisini çalıştırmak için kullanılır. (Ekstra hiçbir kütüphane/paket yüklemeniz gerekmez).
2. **GCC Derleyicisi:** Üretilen C kodlarını çalıştırılabilir ikili dosyalara dönüştürmek için gereklidir.
   - **Windows:** [MSYS2](https://www.msys2.org/) kurup MinGW-w64 (`C:\msys64\ucrt64\bin`) yolunu sistem ortam değişkenlerindeki `PATH`'e eklemeniz yeterlidir.
   - **Linux (Ubuntu/Debian):** Terminalden `sudo apt install build-essential` komutu ile kurulabilir.

Kurulumları doğrulamak için terminalinizi (PowerShell veya Bash) açıp şu komutları deneyin:
```bash
python --version
gcc --version
```
Her iki komut da sürüm numarası veriyorsa hazırsınız!

---

### İki Derleyici, Tek Dil: Python Bootstrap ve Native u++ Derleyicisi

u++ ekosisteminde **birbirinin tamamen aynısı olan iki farklı derleyici** bulunur:

| Derleyici | Konum | Tanım ve Rolü |
|---|---|---|
| **Python Derleyicisi (`uppc.py`)** | Kök dizin | **Bootstrap (başlangıç) derleyicisidir.** Hiçbir ön kurulum gerektirmeden doğrudan Python ile çalışır; güvenilir bir referanstır. |
| **Native Derleyici (`uppc.exe` / `uppc`)** | `derleme/` | **Bizzat u++ diliyle yazılmış (`src/uppc/`), kendi kendini derleyebilen (self-hosting) native derleyicidir.** Python'a ihtiyaç duymadan doğrudan makine hızında çalışır. |

#### u++ ile Yazılmış Native Derleyiciyi Üretmek (Build)
Native derleyicinin kaynak kodları `src/uppc/` klasöründedir. Onu ilk kez ikili hale getirmek için:

1. Önce modülleri tek bir kaynak dosyada birleştiririz:
   ```bash
   python araclar/birlestir.py
   ```
   *(Bu işlem `derleme/uppc_birlesik.upp` dosyasını oluşturur).*

2. Ardından Python derleyicisi ile bu dosyayı derleyip native `uppc.exe` (Linux'ta `uppc`) ikilisini üretiriz:
   ```bash
   python uppc.py derleme/uppc_birlesik.upp --sadece-derle --cikti derleme/uppc
   ```

Artık `derleme/` klasörünüzde tamamen u++ ile yazılmış native bir derleyiciniz var!

#### Native Derleyiciyi Kullanmak
Native derleyici, Python derleyicisiyle **birebir aynı komutları ve bayrakları** kabul eder, ancak çok daha hızlı çalışır:

```bash
# Windows üzerinde çalıştırma:
derleme\uppc.exe program.upp

# Sadece derlemek (çalıştırmadan .exe üretmek):
derleme\uppc.exe program.upp --sadece-derle --cikti program

# Linux üzerinde çalıştırma:
./derleme/uppc program.upp
```

#### Kendi Kendini Derleme (Self-Hosting) Testi
Bir programlama dilinin olgunluk seviyesi, kendi derleyicisini kendi derleyicisiyle hatasız inşa edebilmesiyle ölçülür. u++ bunu rahatlıkla başarır:
```bash
# 1. Native derleyici (uppc.exe), kendi kaynak kodunu derleyip uppc1.exe üretir:
derleme\uppc.exe derleme/uppc_birlesik.upp --sadece-derle --cikti derleme/uppc1

# 2. Üretilen yeni derleyici (uppc1.exe) de tekrar aynı kaynak kodu derleyip uppc2.exe üretir:
derleme\uppc1.exe derleme/uppc_birlesik.upp --sadece-derle --cikti derleme/uppc2
```
`uppc1.exe` ve `uppc2.exe` birebir aynı ikiliyi üretir. Geliştirme sürecinizde ister pratikliği için `uppc.py`'yi, ister yüksek performansı için `derleme/uppc.exe`'yi kullanabilirsiniz.

---

---

## 3. İlk Programımız: "Merhaba Dünya!"

Geleneksel olarak her programlama diline ekrana "Merhaba Dünya" yazdırılarak başlanır. Biz de projenizin ana dizininde `merhaba.upp` adında bir dosya oluşturalım ve içine şu satırları yazalım:

```text
fonk ana() -> sayi {
    upp.satir_yaz("Merhaba, u++ dunyasi!");
    don 0;
}
```

Bu programı çalıştırmak için terminalinizde şu komutu yürütün:
```bash
python uppc.py merhaba.upp
```

Tebrikler! Terminalinizde `Merhaba, u++ dunyasi!` yazısını gördünüz.

### Kodun Anatomisi

Gelin bu birkaç satırın ne anlama geldiğini tek tek inceleyelim:

* **`fonk ana() -> sayi { ... }`**:
  - `fonk`: Bir fonksiyon (alt program) tanımladığımızı belirtir.
  - `ana`: Programın **giriş noktasıdır**. Bilgisayar programı çalıştırmaya başladığında ilk olarak `ana` fonksiyonunu arar ve oradan başlar.
  - `() `: Fonksiyonun dışarıdan herhangi bir ek parametre almadığını gösterir.
  - `-> sayi`: Bu fonksiyonun işi bittiğinde geriye bir tam sayı döndüreceğini söyler.
  - `{ ... }`: Fonksiyonun gövdesini (nerede başlayıp nerede bittiğini) belirten süslü parantezlerdir.
* **`upp.satir_yaz("...");`**:
  - `upp`: u++ standart kütüphanesinin kısaltmasıdır.
  - `satir_yaz`: Kendisine verilen metni ekrana basar ve ardından imleci bir alt satıra geçirir.
  - `""`: Çift tırnak içerisine yazılan her şey bir metindir.
  - `;` (Noktalı Virgül): u++ dilinde **her komut satırının sonuna noktalı virgül koymak zorunludur**. Bilgisayara "bu talimat burada bitti" demiş olursunuz.
* **`don 0;`**:
  - Programın başarıyla tamamlandığını işletim sistemine bildirir. Genel kural olarak `0` dönüşü "her şey yolunda, sorunsuz bitti" anlamına gelir.

### Yorum Satırları: Koda Not Bırakmak
Kod yazarken kendinize veya ekip arkadaşlarınıza açıklamalar bırakmak istersiniz. Derleyici bu notları tamamen görmezden gelir:

```text
// Bu tek satırlık bir açıklamadır. Derleyici bunu çalıştırmaz.

/*
   Bu ise çok satırlı
   bir yorum bloğudur.
   İstediğiniz kadar uzatabilirsiniz.
*/
```

---

## 4. Değişkenler ve Temel Veri Tipleri

### Değişken Nedir?
Değişkenleri, üzerine etiket yapıştırılmış birer saklama kutusu gibi düşünebilirsiniz. Bilgisayarın geçici hafızasında (RAM) bir yer ayırır, oraya bir isim verir ve içine dilediğimiz bilgiyi koyarız.

### Temel Veri Tipleri
u++ dilinde her bilginin bir türü (tipi) vardır:

| u++ Tipi | Açıklama | Örnek |
|---|---|---|
| `sayi` | 64-bit tam sayılar (negatif veya pozitif) | `10`, `-42`, `0`, `0xFF` (onaltılık) |
| `ondalik` | Küsurlu / ondalıklı sayılar (çift duyarlıklı - double) | `3.14`, `-0.5`, `100.0` |
| `metin` | Yazılar, karakter dizileri | `"Arda"`, `"u++ ile gelecege"` |
| `mantik` | Mantıksal doğruluk (Boolean) | `dogru` veya `yanlis` |
| `bos` | Değersizlik, hiçbir şey döndürmeme (`void`) | Genellikle fonksiyon dönüşlerinde kullanılır |

### Değişken Tanımlama ve Değer Atama

Bir değişken oluştururken önce türünü yazarız, sonra adını belirler ve `=` işareti ile başlangıç değerini veririz:

```text
fonk ana() -> sayi {
    sayi yas = 25;
    ondalik boy = 1.82;
    metin ad = "Ahmet";
    mantik ogrenci_mi = dogru;

    upp.satir_yaz("Kullanici kaydedildi.");
    don 0;
}
```

Değişkenin değerini daha sonra değiştirebilirsiniz:
```text
sayi puan = 10;
puan = 25; // Artık puan kutusunun içinde 25 var.
```

### Otomatik Tür Çıkarımı: `oto`
Her seferinde değişkenin türünü uzun uzun yazmak istemeyebilirsiniz. u++ verdiğiniz ilk değerden değişkenin türünü otomatik olarak anlar. Bunun için `oto` sözcüğünü kullanırız:

```text
oto mesafe = 150;        // Derleyici otomatik olarak 'sayi' yapar
oto pi = 3.1415;         // Derleyici 'ondalik' olarak anlar
oto sehir = "Istanbul";  // 'metin' türündedir
oto aktif = dogru;       // 'mantik' türündedir
```

> **Önemli Not:** `oto` kullandığınızda mutlaka bir başlangıç değeri vermelisiniz. Örneğin `oto x;` şeklinde bırakamazsınız, çünkü derleyici x'in ne tür bir veri tutacağını bilemez.

### Sabit Değerler: `sabit`
Programınız boyunca değerinin asla değişmesini istemediğiniz durumlar olur (örneğin vergi oranı, maksimum hız sınırı vb.). Bu değişkenlerin önüne `sabit` yazarsanız, sonradan yanlışlıkla değiştirilmelerini engellemiş olursunuz:

```text
sabit sayi MAKSIMUM_CAN = 100;
sabit ondalik KDV = 0.20;

// MAKSIMUM_CAN = 120; // HATA! Derleyici buna izin vermez.
```

---

## 5. Metinler (Strings) ve Dize İnterpolasyonu

u++ dilinde metinlerle çalışmak hem çok kolay hem de çok güçlüdür.

### Metin İçine Değişken Gömme (İnterpolasyon)
Ekrana bir mesaj basarken metnin içine değişkenleri yerleştirmek için süslü parantez `{degisken}` sözdizimini kullanırız. Bu işleme **dize interpolasyonu** denir:

```text
fonk ana() -> sayi {
    oto ad = "Deniz";
    oto yas = 21;
    oto bakiye = 145.50;

    upp.satir_yaz("Merhaba {ad}, su an {yas} yasindasin. Bakiyen: {bakiye} TL.");
    don 0;
}
```

Eğer ekrana gerçekten süslü parantez `{` yazdırmak isterseniz çift parantez `{{` veya `}}` kullanmanız yeterlidir.

### Çok Satırlı Metinler
Uzun bir metin veya SQL sorgusu yazmak isterseniz üç tırnak `"""` imdadınıza yetişir:

```text
metin siir = """
Gokyuzunun baska rengi de varmis!
Gec farkettim tasin sert oldugunu.
Su insani bogar, ates yakarmis!
""";
upp.satir_yaz(siir);
```

### Metinleri Birleştirme (`+` ve `+=`)
Metinleri `+` operatörü ile uç uca ekleyebilirsiniz:

```text
metin ad = "Ali";
metin soyad = "Yilmaz";
metin tam_ad = ad + " " + soyad; // "Ali Yilmaz"

tam_ad += " (Yonetici)";
upp.satir_yaz(tam_ad);
```

### Karakterlere Kod Noktası ile Erişim (`s[i]`)
u++ dili uluslararası karakterleri (Türkçe karakterler `ç, ğ, ı, ö, ş, ü` dahil) tam uyumlu **UTF-8** olarak işler.
Bir metnin `i`'nci karakterini `s[i]` ile aldığınızda, size o karakterin uluslararası Unicode kod numarasını (`sayi`) döndürür:

```text
metin kelime = "Uçak";
sayi ilk_kod = kelime[0]; // 'U' harfinin Unicode kodu (85)
sayi ikinci_kod = kelime[1]; // 'ç' harfinin Unicode kodu (231)
```

---

## 6. Operatörler ve Matematiksel Hesaplamalar

Bilgisayar programları sayıları işlemek ve mantıksal kararlar vermek için operatörleri kullanır.

### Matematiksel (Aritmetik) Operatörler
* `+` : Toplama (`10 + 5` -> `15`)
* `-` : Çıkarma (`10 - 4` -> `6`)
* `*` : Çarpma (`3 * 4` -> `12`)
* `/` : Bölme (`20 / 4` -> `5`)
* `%` : Mod alma / Kalan bulma (`10 % 3` -> `1`)

Kısaltılmış bileşik atama operatörleri de desteklenir:
```text
sayi a = 10;
a += 5; // a = a + 5 ile aynıdır (a şimdi 15)
a -= 2; // a şimdi 13
a *= 2; // a şimdi 26
a /= 2; // a şimdi 13
```

### Karşılaştırma Operatörleri
İki değeri kıyaslamak için kullanılır ve sonuç daima `dogru` veya `yanlis` çıkar:
* `==` : Eşit mi? (`5 == 5` -> `dogru`)
* `!=` : Eşit değil mi? (`5 != 3` -> `dogru`)
* `<`  : Küçük mü?
* `>`  : Büyük mü?
* `<=` : Küçük veya eşit mi?
* `>=` : Büyük veya eşit mi?

### Mantıksal Operatörler
Birden fazla şartı birbirine bağlamak için kullanılır:
* `&&` : **VE** (Her iki taraf da doğruysa sonuç doğrudur)
* `||` : **VEYA** (En az bir taraf doğruysa sonuç doğrudur)
* `!`  : **DEĞİL** (Doğruyu yanlışa, yanlışı doğruya çevirir)

```text
oto yas = 19;
oto ehliyet_var = dogru;

// Hem 18 yaşından büyük VE hem de ehliyeti var mı?
oto araba_kullanabilir = (yas >= 18) && ehliyet_var;
```

### Kısa Koşul (Ternary Operatörü: `? :`)
Küçük koşulları tek satırda ifade etmek için idealdir:
```text
sayi puan = 75;
metin sonuc = (puan >= 50) ? "Gecti" : "Kaldi";
```

---

## 7. Karar Yapıları: `eger`, `yoksa` ve `secim`

Hayatta olduğu gibi programlamada da belirli şartlara göre farklı yollar izlememiz gerekir.

### `eger` ve `yoksa` Blokları
Belirli bir koşul sağlandığında bir kod parçasını, sağlanmadığında başka bir kod parçasını çalıştırmak için `eger` (if) ve `yoksa` (else) kullanılır:

```text
fonk ana() -> sayi {
    sayi hava_sicakligi = 24;

    eger (hava_sicakligi > 30) {
        upp.satir_yaz("Hava cok sicak, bol su icin.");
    } yoksa eger (hava_sicakligi >= 20) {
        upp.satir_yaz("Hava gayet guzel, yuruyuse cikabilirsiniz.");
    } yoksa {
        upp.satir_yaz("Hava serin, montunuzu unutmayin.");
    }

    don 0;
}
```

### Çoklu Seçim: `secim` (Switch-Case)
Bir değişkenin alabileceği birçok sabit değer varsa, art arda onlarca `eger / yoksa eger` yazmak yerine `secim` yapısı kullanılır.

u++ dilinde `secim` hem **sayılar**, hem **seçenekler (enum)** hem de **metinler** ile çalışabilir:

```text
fonk ana() -> sayi {
    metin komut = "baslat";

    secim (komut) {
        durum "baslat":
            upp.satir_yaz("Sistem baslatiliyor...");
            dur; // Seçim bloğundan çık
        durum "durdur":
            upp.satir_yaz("Sistem durduruldu.");
            dur;
        durum "yenile":
            upp.satir_yaz("Sistem yeniden baslatiliyor.");
            dur;
        varsayilan:
            upp.satir_yaz("Bilinmeyen komut!");
    }

    don 0;
}
```

> **Dikkat:** `dur;` yazmazsanız bilgisayar bir sonraki `durum` kodunu da çalıştırmaya devam eder. Bu yüzden her durumun işi bittiğinde `dur;` ile bloğu kapatırız.

---

## 8. Döngüler: `dongu`, `iken` ve `her`

Bilgisayarlar aynı işi milyonlarca kez yorulmadan ve hatasız tekrar edebilir. Bu tekrarları **döngüler** ile kurarız.

### `iken` Döngüsü (While)
Bir koşul doğru (`dogru`) olduğu sürece çalışmaya devam eder:

```text
sayi sayac = 1;
iken (sayac <= 5) {
    upp.satir_yaz("Sayac degeri: {sayac}");
    sayac += 1; // Sayacı artırmayı unutursak sonsuz döngüye girer!
}
```

### `dongu` Döngüsü (For)
Genellikle belirli bir başlangıç değerinden bitişe kadar adım adım saymak için kullanılır:

```text
// 0'dan 4'e kadar 5 kere calisir
dongu (oto i = 0; i < 5; i += 1) {
    upp.satir_yaz("Dongu adimi: {i}");
}
```

### Döngü Kontrolü: `dur` ve `devam`
* `dur;` : Döngüyü anında sonlandırır ve dışarı çıkar.
* `devam;` : O anki adımı yarıda keser ve hemen bir sonraki döngü adımına atlar.

```text
dongu (oto i = 1; i <= 10; i += 1) {
    eger (i == 3) {
        devam; // 3'ü atla, yazdırma
    }
    eger (i == 7) {
        dur;   // 7'ye gelince döngüyü tamamen bitir
    }
    upp.satir_yaz("Sayi: {i}");
}
```

---

## 9. Koleksiyonlar: Sabit Diziler, `liste` ve `harita`

Şimdiye kadar tek bir değişkende tek bir değer tuttuk. Peki 100 öğrencinin notunu ya da binlerce kelimelik bir sözlüğü nasıl saklayacağız?

### 1. Sabit Boyutlu Diziler
Eleman sayısı baştan belli olan koleksiyonlardır:

```text
sayi puanlar[] = [85, 90, 78, 92, 100];

// Elemanlara 0'dan başlayan indekslerle erişiriz:
upp.satir_yaz("Ilk puan: {puanlar[0]}"); // 85
puanlar[1] = 95; // 2. elemanı güncelledik
```

### 2. Dinamik Listeler (`liste[T]`)
Gerçek hayatta bir listeye sürekli yeni elemanlar eklenip çıkarılabilir. u++ dilinde `liste` otomatik olarak hafızasını büyüten dinamik bir yapıdır:

```text
liste[metin] alisveris;
alisveris.ekle("Ekmek");
alisveris.ekle("Sut");
alisveris.ekle("Yumurta");

upp.satir_yaz("Toplam alinacak: {alisveris.uzunluk()}"); // 3
upp.satir_yaz("Ilk urun: {alisveris.al(0)}"); // Ekmek

alisveris.bosalt(); // Liste hafızasını temizler
```

### 3. Haritalar / Sözlükler (`harita[K, V]`)
Bir anahtar (Key) verip karşılığında bir değer (Value) aldığınız yapılardır. Tıpkı telefon rehberi gibidir; ismi verirsiniz, numarayı alırsınız:

```text
harita[metin, sayi] yas_tablosu;
yas_tablosu.koy("Ahmet", 24);
yas_tablosu.koy("Zeynep", 28);

eger (yas_tablosu.var_mi("Ahmet")) {
    oto ahmet_yas = yas_tablosu.al("Ahmet");
    upp.satir_yaz("Ahmet'in yasi: {ahmet_yas}");
}

yas_tablosu.bosalt();
```

### `her` Döngüsü (Foreach)
Koleksiyonların içindeki elemanları tek tek gezmek için `her` döngüsü biçilmiş kaftandır:

```text
// Dizi veya liste üzerinde:
sayi sayilar[] = [10, 20, 30];
her (oto s in sayilar) {
    upp.satir_yaz("Sayi: {s}");
}

// Harita üzerinde (anahtarları gezer):
her (oto isim in yas_tablosu) {
    oto yas = yas_tablosu.al(isim);
    upp.satir_yaz("{isim} -> {yas}");
}
```

---

## 10. Fonksiyonlar (Alt Programlar)

Bir program büyüdükçe aynı kodları defalarca yazmak istemeyiz. Kod parçalarını paketleyip onlara bir isim verir ve istediğimiz zaman o isimle çağırırız. Buna **fonksiyon** denir.

### Fonksiyon Tanımlama
Bir fonksiyon şu bileşenlerden oluşur:
1. `fonk` anahtar kelimesi
2. Fonksiyonun adı
3. Parantez içinde alacağı girdiler (parametreler)
4. `->` sonrasında döndüreceği verinin tipi
5. Süslü parantezler içindeki kodlar ve `don` (return) ifadesi.

```text
// İki sayıyı toplayıp sonucunu döndüren bir fonksiyon:
fonk topla(sayi a, sayi b) -> sayi {
    sayi sonuc = a + b;
    don sonuc; // Hesaplanan degeri geri gonder
}

fonk ana() -> sayi {
    sayi x = topla(15, 25);
    upp.satir_yaz("Toplam: {x}"); // 40
    don 0;
}
```

### Geriye Değer Döndürmeyen Fonksiyonlar (`-> bos`)
Eğer bir fonksiyon sadece ekrana bir şeyler yazdıracaksa veya bir iş yapıp geriye değer vermeyecekse dönüş tipine `bos` (void) yazarız:

```text
fonk karsila(metin isim) -> bos {
    upp.satir_yaz("Hos geldin, {isim}!");
}
```

### Kısa Ok Gövdesi (`=>`)
Tek satırlık basit fonksiyonlar için süslü parantez ve `don` yazmak yerine ok işaretini `=>` kullanabilirsiniz:

```text
fonk kare_al(sayi x) => x * x;
fonk selam_ver() -> bos => upp.satir_yaz("Selam!");
```

### Varsayılan Parametreler
Bir parametreye başlangıç değeri atayarak, çağrılırken verilmesini isteğe bağlı hale getirebilirsiniz:

```text
fonk vergi_hesapla(sayi tutar, ondalik oran = 0.20) -> ondalik {
    don tutar * oran;
}

// Kullanımı:
oto v1 = vergi_hesapla(100);       // oran otomatik 0.20 alinir
oto v2 = vergi_hesapla(100, 0.10); // oran 0.10 olarak ezilir
```

---

## 11. Nesne Yönelimli Programlama: Sınıflar (`sinif`)

Gerçek dünyadaki kavramları (örneğin bir arabayı, bir banka hesabını veya bir oyun karakterini) bilgisayara anlatmak için nesneleri kullanırız. 
* **Sınıf (Class):** Bir nesnenin şablonudur (örneğin "Araba" fikri).
* **Nesne (Object):** O şablondan üretilmiş gerçek bir örnektir (örneğin garajınızdaki 34 ABC 123 plakalı beyaz araba).

### Sınıf Tanımlama ve `kendi` Sözcüğü
Bir sınıf, verileri (alanlar) ve bu verilerle işlem yapan fonksiyonları (metotlar) bir arada tutar:

```text
sinif Oyuncu {
    metin isim;
    sayi can;
    sayi seviye;

    // Kurucu fonksiyon (Nesne ilk yaratıldığında çalışır)
    fonk olustur(metin o_isim) {
        kendi.isim = o_isim;
        kendi.can = 100;
        kendi.seviye = 1;
    }

    // Oyuncunun canını azaltan metot
    fonk hasar_al(sayi miktar) -> bos {
        kendi.can -= miktar;
        eger (kendi.can < 0) {
            kendi.can = 0;
        }
        upp.satir_yaz("{kendi.isim} hasar aldi! Kalan can: {kendi.can}");
    }
}
```

* **`kendi`:** Metotların içerisinde, o an üzerinde çalıştığımız nesnenin kendi alanlarına erişmek için kullanılır (diğer dillerdeki `this` veya `self` gibi).

### Nesne Oluşturma ve Kullanma

```text
fonk ana() -> sayi {
    // Kurucu fonksiyonu doğrudan sınıf adıyla çağırabiliriz:
    Oyuncu p1 = Oyuncu("Savasci");
    
    upp.satir_yaz("Oyuncu olusturuldu: {p1.isim}");
    p1.hasar_al(35); // 65 can kalır
    p1.hasar_al(80); // 0 can kalır

    don 0;
}
```

### Seçenekler: `secenek` (Enum)
Birbiriyle ilişkili sabit durumları adlandırmak için kullanılır:
```text
secenek Yon {
    Kuzey,
    Guney,
    Dogu,
    Bati
}

Yon git = Yon.Kuzey;
```

---

## 12. Modüler Kod Mimarisi: `kullan` ve `.uph` Başlıkları

Büyük projelerde bütün kodları tek bir dosyaya doldurmak projeyi yönetilemez hale getirir. Bu yüzden kodlarımızı farklı dosyalara ayırırız.

### Başlık Dosyaları (`.uph`)
İçerisinde `fonk ana()` bulunmayan, sadece yardımcı fonksiyonları veya sınıfları barındıran dosyalara başlık dosyası denir ve uzantısı `.uph` (u++ header) olur.

Örneğin `matematik_yardim.uph` adında bir dosya yapalım:
```text
// matematik_yardim.uph
fonk kup_al(sayi n) => n * n * n;
sabit ondalik PI = 3.14159;
```

### Dosyayı Dahil Etme: `kullan`
Ana programımızda bu kütüphaneyi içeri aktarmak için `kullan` yazarız:

```text
// ana_program.upp
kullan "matematik_yardim.uph";

fonk ana() -> sayi {
    oto k = kup_al(3);
    upp.satir_yaz("3'un kupu: {k}"); // 27
    don 0;
}
```

### İsim Çakışmasını Önleme: `olarak` Takısı (Takma Ad / Namespace)
İki farklı dosyada aynı isimde fonksiyon varsa veya kodun hangi kütüphaneden geldiğini netleştirmek istiyorsanız `olarak` takısını kullanabilirsiniz:

```text
kullan "matematik_yardim.uph" olarak mat;

fonk ana() -> sayi {
    oto k = mat.kup_al(4);
    upp.satir_yaz("Sonuc: {k}");
    don 0;
}
```

---

## 13. Eşzamanlılık: `arkaplan` ve `Kilit`

Modern bilgisayarların birden çok çekirdeği vardır ve aynı anda birden fazla görevi yürütebilirler.

### Arka Planda İş Çalıştırma: `arkaplan`
Uzun sürecek bir işlemi (örneğin internetten büyük bir dosya indirmeyi veya ağır bir hesaplamayı) arka planda başlatıp ana programın donmasını engelleyebilirsiniz:

```text
fonk ana() -> sayi {
    upp.satir_yaz("Ana program basladi.");

    // Arka plan iş parçacığı başlatılıyor
    oto gorev = arkaplan {
        upp.satir_yaz("Arka plan isi basladi...");
        upp.uyut(2000); // 2 saniye bekle
        upp.satir_yaz("Arka plan isi bitti!");
    };

    upp.satir_yaz("Ana program baska isler yapiyor...");

    // Arka plandaki iş bitene kadar burada bekle:
    gorev.bekle();

    upp.satir_yaz("Tum surec tamamlandi.");
    don 0;
}
```

### Veri Yarışı ve Senkronizasyon: `Kilit`
İki farklı iş parçacığı aynı anda aynı değişkene yazmaya çalışırsa bellek bozulabilir. Buna **veri yarışı** (race condition) denir. Ortak bir kaynağa erişirken sırayla girmelerini sağlamak için `Kilit` kullanırız:

```text
sayi ORTAK_SAYAC = 0;
Kilit kapi;

fonk artir() -> bos {
    kapi.kilitle(); // Başkası giremez, sırasını bekler
    ORTAK_SAYAC += 1;
    kapi.birak();   // Kapıyı aç, sıradaki girebilir
}
```

---

## 14. Standart Kütüphane (`upp.*`) ile Pratik İşler

u++ size günlük hayatta ihtiyacınız olacak birçok hazır işlevi `upp.*` altında sunar.

### Kullanıcıdan Bilgi Alma
Kullanıcının klavyeden yazdığı bir satırı okumak için `upp.giris()` kullanılır:

```text
upp.yaz("Adinizi girin: ");
oto kullanici_adi = upp.giris();
upp.satir_yaz("Hos geldiniz, {kullanici_adi}!");
upp.metin_bosalt(kullanici_adi); // Kullanılan belleği serbest bırak
```

### Dosya Okuma ve Yazma
```text
// Dosyaya metin yazma:
upp.dosya_yaz("notlar.txt", "1. u++ ogren\n2. Harika projeler yap\n");

// Dosya var mı kontrolü:
eger (upp.dosya_var_mi("notlar.txt")) {
    oto icerik = upp.dosya_oku("notlar.txt");
    upp.satir_yaz("Dosya icerigi:\n{icerik}");
    upp.metin_bosalt(icerik);
}
```

### Klasör ve Yol İşlemleri (`upp.yol`)
```text
oto calisma_yeri = upp.yol.calisma();
oto tam_yol = upp.yol.birlestir(calisma_yeri, "veriler");

upp.yol.klasor_olustur(tam_yol); // Klasör yoksa oluşturur
upp.metin_bosalt(calisma_yeri);
upp.metin_bosalt(tam_yol);
```

### Rastgele Sayı Üretme ve Zaman
```text
// 1 ile 100 arasında rastgele bir sayı:
sayi sansli_sayi = upp.rastgele(1, 100);

// Milisaniye cinsinden gecen sure ölçümü:
oto baslangic = upp.zaman();
upp.uyut(500); // 500 milisaniye (yarım saniye) uyu
oto gecen_sure = upp.zaman() - baslangic;
upp.satir_yaz("Gecen sure: {gecen_sure} ms");
```

### Dış Komut Çalıştırma (`upp.sistem.calistir`)
İşletim sistemi terminal komutlarını çalıştırıp sonucunu alabilirsiniz:

```text
SurecCikti sonuc = upp.sistem.calistir("echo Merhaba");
eger (sonuc.cikti != yok) {
    upp.satir_yaz("Komut ciktisi: {sonuc.cikti}");
    upp.metin_bosalt(sonuc.cikti);
}
```

---

## 15. Bellek Yönetimi ve `guvensiz` Kapısı

### Bellek Sahipliği ve `upp.metin_bosalt`
u++, C diline derlendiği için dinamik olarak işletim sisteminden ayrılan (heap) belleklerin işi bittiğinde sisteme geri verilmesi gerekir.
* `upp.giris()`, `upp.dosya_oku()`, `upp.sistem.calistir` veya `+` ile oluşturulan dinamik metinler hafıza kaplar.
* İşi bittiğinde `upp.metin_bosalt(degisken);` çağırmak, programınızın arka planda şişmesini (bellek sızıntısı - memory leak) önler.

### u++ Bellek Güvenliği Felsefesi
C ve C++ dillerinde en çok karşılaşılan kabus, yanlış bellek adreslerine erişerek programın çökmesi (Segmentation Fault) veya güvenlik açığı oluşturmasıdır.

u++ dilinde **varsayılan olarak işaretçiler (pointers `*`, `&`), doğrudan bellek ayırma ve harici C kodları tamamen yasaktır**. 

Eğer bu tür düşük seviyeli işler yapmanız gerekiyorsa, bunu bilinçli olarak **`guvensiz { ... }`** bloğu içinde yapmanız şarttır:

```text
// Güvenli alan: Burada pointer kullanırsanız derleme hatası alırsınız!

guvensiz {
    // Burası düşük seviyeli sistem alanıdır
    sayi deger = 42;
    oto adres = &deger; // Adres alma işlemi serbesttir
    
    // Satır içi C kodu yazabilirsiniz:
    c_kod {
        printf("C dunyasindan selamlar!\n");
    }
}
```

Eğer `guvensiz` bloğunun içinde belirli bir kısmı tekrar güvenli kurallara döndürmek isterseniz iç içe `guvenli { ... }` açabilirsiniz.

---

## 16. Geliştirici Araçları ve VS Code / Cursor Entegrasyonu

### Komut Satırı Bayrakları
Hem `python uppc.py` hem de native `derleme/uppc.exe` tamamen aynı bayrakları destekler:
* `python uppc.py program.upp` (veya `derleme\uppc.exe program.upp`): Derler ve hemen çalıştırır.
* `--sadece-derle` : Çalıştırmaz, sadece `.exe` / `.out` dosyasını üretir.
* `--cikti <AD>` : Üretilecek `.c` ve `.exe` dosyasının adını belirler (örn: `--cikti hesapla`).
* `--sadece-c` : GCC'yi çağırmaz, sadece C11 kodunu üretip incelenmek üzere bırakır.
* `--bicim` : Kaynak kodunuzun girintilerini ve biçimini otomatik düzeltir (Code Formatter).
* `--analiz` : C derleyicisi çalıştırmadan anında sözdizimi ve tip hatalarını denetler (Editörler için JSON döner).

### VS Code ve Cursor Eklentisi
`eklenti/` klasöründeki resmi u++ eklentisini kurduğunuzda:
* Kod renklendirme (`.upp` ve `.uph` desteği),
* Siz kodu yazarken anında altını çizen hata tespitleri (LSP),
* Kod tamamlama ve üzerine gelince doküman görme (hover),
* **F5** tuşu ile tek tuşla derleyip çalıştırma kolaylığı elde edersiniz.
* **Derleyici Seçimi:** Eklenti ayarlarından (`upp.derleyici`) ister `python` ister `native` derleyiciyi seçebilirsiniz. Native seçildiğinde düzenleyici analizleri doğrudan `derleme/uppc.exe` hızıyla gerçekleştirilir.

---

## 17. Kapsamlı Bir Proje: Konsol Not Defteri

Öğrendiğimiz her şeyi (fonksiyonlar, döngüler, koşullar, dosya işlemleri, koleksiyonlar ve kullanıcı girdisi) birleştiren eksiksiz bir konsol uygulaması yazalım:

```text
// notluk.upp — Basit Bir Konsol Not Defteri

fonk menuyu_goster() -> bos {
    upp.satir_yaz("\n=== U++ NOT DEFTERI ===");
    upp.satir_yaz("1. Notlari Oku");
    upp.satir_yaz("2. Yeni Not Ekle");
    upp.satir_yaz("3. Tum Notlari Temizle");
    upp.satir_yaz("4. Cikis");
    upp.yaz("Seciminiz (1-4): ");
}

fonk ana() -> sayi {
    sabit metin DOSYA_ADI = "notlarim.txt";
    mantik calisiyor = dogru;

    iken (calisiyor) {
        menuyu_goster();
        oto secim = upp.giris();

        secim (secim) {
            durum "1":
                eger (upp.dosya_var_mi(DOSYA_ADI)) {
                    oto icerik = upp.dosya_oku(DOSYA_ADI);
                    upp.satir_yaz("\n--- NOTLARINIZ ---");
                    upp.satir_yaz(icerik);
                    upp.satir_yaz("------------------");
                    upp.metin_bosalt(icerik);
                } yoksa {
                    upp.satir_yaz("\nHenuz kaydedilmis bir not yok.");
                }
                dur;

            durum "2":
                upp.yaz("Eklemek istediginiz notu yazin: ");
                oto yeni_not = upp.giris();
                upp.dosya_ekle(DOSYA_ADI, yeni_not + "\n");
                upp.satir_yaz("Notunuz basariyla kaydedildi!");
                upp.metin_bosalt(yeni_not);
                dur;

            durum "3":
                eger (upp.dosya_var_mi(DOSYA_ADI)) {
                    upp.dosya_sil(DOSYA_ADI);
                    upp.satir_yaz("Tum notlar silindi.");
                } yoksa {
                    upp.satir_yaz("Silinecek dosya bulunamadi.");
                }
                dur;

            durum "4":
                upp.satir_yaz("Gorusmek uzere!");
                calisiyor = yanlis;
                dur;

            varsayilan:
                upp.satir_yaz("Gecersiz secim, lutfen 1-4 arasinda bir sayi girin.");
        }

        upp.metin_bosalt(secim);
    }

    don 0;
}
```

Bu kodu `notluk.upp` dosyasına kaydedip `python uppc.py notluk.upp` komutuyla çalıştırabilirsiniz.

---

## Tebrikler!

Artık u++ dilinin temel ve ileri seviye tüm dinamiklerini, programlama mantığını ve sistem mimarisini biliyorsunuz.

Daha fazla fonksiyonel detay ve standart kütüphanenin tam fonksiyon listesi için [docs/API_tr.md](docs/API_tr.md) belgesini inceleyebilirsiniz. İyi kodlamalar!
