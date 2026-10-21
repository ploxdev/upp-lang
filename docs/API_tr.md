# u++ Standart Kütüphane ve API Referansı (v3.1 beta)

Bu belge `upp.*` yerleşiklerini, türleri, bellek modelini, hedefleri ve derleyici komut satırını tanımlar. Yerleşiklerin tek doğru kaynağı [`src/uppc/yerlesik.upp`](../src/uppc/yerlesik.upp) tablosudur: tür denetimi argüman sayısını, argüman türlerini, dönüş türünü, `guvensiz` zorunluluğunu ve hedef uygunluğunu oradan okur. Bu belge o tabloyla birebir eşleşir.

Öğrenme rehberi: [OGREN.md](../OGREN.md) · Derleyici mimarisi: [derleyici.md](../derleyici.md) · English: [API_en.md](API_en.md)

---

## 1. Bellek Modeli

### Metin sahipliği (otomatik drop)
- Her `metin` yönetilen bir nesnedir: başlığında atomik referans sayısı, bayt uzunluğu, kapasite ve kod noktası bilgisi bulunur. Veri, NUL ile biten ve C uyumlu bir `char*` olarak kalır.
- Kopya O(1)'dir (referans artışı). `+=` tek sahipli metni yerinde büyütür, paylaşılan metinde copy-on-write yapar.
- Derleyici metinleri kapsam sonunda, `dur`/`devam` ile çıkarken ve `don` ile dönerken bırakır. Elle bırakma yoktur.
- `upp.uzunluk(s)` O(1)'dir. ASCII metinde `s[i]` de O(1)'dir. ASCII olmayan metin kendi kod noktası tablosunu taşır.
- Metin sabitleri, başlığı derleme anında doldurulmuş ölümsüz nesnelerdir.

### Sahipli sınıflar
`metin` alanı (ya da sahipli sınıf alanı) taşıyan sınıflar için derleyici `T__birak` ve `T__kopya` üretir:
- Değişkenden kopyalama referans artırır. Geçici değerler (kurucu, çağrı sonucu) taşınır.
- Atama eski değeri bırakır. Parametreler girişte sahiplenilir, `don` taşıyarak döner.
- Yerel değişkenler kapsam sonunda bırakılır.

### Görünümler: `&metin`, `&dilim[T]`
Kopyasız, salt okunur ödünç bakışlardır: `fonk yaz(&metin v)`, `&dilim[sayi] d = dizi[1..4]`. Alanları `v.uzunluk` (metinde UTF-8 bayt sayısı) ve `v.veri` (yalnızca `&metin`). Görünüm döndürülemez, sınıf alanı ya da üst düzey değişken olamaz, `arkaplan` içine yakalanamaz.

### Koleksiyonlar
`liste[T]` ve `harita[K,V]` değer türüdür: kopya O(1)'dir (referans artışı), yazma anında paylaşılan kopya ayrılır (copy-on-write). `M = L` sonrası `M.ekle(x)` `L`'yi değiştirmez; parametreye geçen liste çağıranınkini değiştirmez. Derleyici koleksiyonları da kapsam sonunda bırakır; `.bosalt()` yalnızca belleği erken geri vermek içindir ve koleksiyonu boş bırakır.

Eleman türü işaretçi ve görünüm dışında her değer türü olabilir: sayısal türler, `metin`, `sinif`, `liste[...]`, `harita[...]`, `fonk(...)`. Harita anahtarı `metin` ya da `sayi` olur. `L[i][j] = x`, `K[0].ad += "!"`, `K[1].yaslan()` yerinde çalışır.

`JSONDeger` tutamaçtır, `upp.json.bosalt()` ile kapatılır.

### Fonksiyon değerleri
`fonk(P1, P2) -> R` bir türdür (`-> R` yazılmazsa `bos`). Üst düzey fonksiyonlar adlarıyla değer olur; değişkende, parametrede, dönüş değerinde, sınıf alanında ve koleksiyonda saklanabilir:

```upp
fonk ikiye(sayi x) -> sayi => x * 2;
fonk uygula(fonk(sayi) -> sayi f, sayi x) -> sayi => f(x);

fonk(sayi) -> sayi f = ikiye;
upp.satir_yaz("{uygula(f, 5)}");
harita[metin, fonk(sayi, sayi) -> sayi] komutlar;
komutlar.koy("carp", fonk(sayi a, sayi b) -> sayi => a * b);
upp.satir_yaz("{komutlar.al(\"carp\")(3, 4)}");
```

- Anonim fonksiyon: `fonk(sayi x) -> sayi => x * 2` ya da `fonk(...) -> T { ... }`. Yakalama yoktur; dış yerel değişkene erişmek derleme hatasıdır, değeri parametreyle geçin.
- Atanmamış fonksiyon değeri `yok`'tur. `f == yok` ile denetlenir; `yok` çağrılırsa program açık bir panikle durur.
- İmzalar birebir eşleşmelidir. Fonksiyon değerleriyle yalnızca `==` ve `!=` kullanılır. Metotlar ve `ana` değer olarak alınamaz.
- C karşılığı düz fonksiyon işaretçisidir; serbest ve çekirdek hedeflerinde de çalışır (kesme/komut işleyici tabloları).

### Hata yönetimi: `beklenen`, `?`, `ertele`
`beklenen[T, H]` ya bir `T` değeri ya da bir `H` hatası taşır; `beklenen[T]`, `beklenen[T, metin]` kısaltmasıdır. C'de bir bayrak ve birlikten (`union`) oluşan düz bir yapıdır: değer olarak döner, gizli bellek ayırma yoktur. Hata türü `secenek` ise heap'e hiç dokunulmaz.

```upp
secenek DiskHata { YOK, BOZUK, DOLU }

fonk sektor(sayi no) -> beklenen[sayi, DiskHata] {
    eger (no > 100) { don hata(DiskHata.DOLU); }
    don no * 512;                     // T değeri: başarı
}

fonk iki_sektor(sayi a, sayi b) -> beklenen[sayi, DiskHata] {
    sayi x = sektor(a)?;              // hata ise temizleyip aynı hatayı döndürür
    sayi y = sektor(b)?;
    don x + y;
}
```

- **Kurma:** `beklenen` dönen fonksiyonda `don x;` başarı, `don hata(e);` hata döndürür. Aynısı ilk değer, atama ve argümanda da geçerlidir: `beklenen[sayi] r = 5;`, `r = hata("x");`. İlk değeri olmayan değişken başarı + sıfır değerdir.
- **`beklenen[bos, H]`:** değer taşımaz; `don;` ya da fonksiyonun sonuna gelmek başarıdır.
- **Metotlar:** `tamam_mi()`, `hata_mi()`, `deger()` (hata varsa panik; `H` metinse mesajı yazar), `hata()` (hata yoksa panik), `deger_yoksa(v)`.
- **`e?`:** yalnızca `beklenen` dönen ve aynı `H` türünü kullanan fonksiyonda yazılır. Döngü koşulunda, `&&`/`||` sağ tarafında ve `?:` dallarında kullanılamaz.
- **Yok sayma yasak:** `f();` biçiminde atılan `beklenen` sonucu derleme hatasıdır; `?` ile yayın ya da denetleyin.
- **Kısıtlar:** `beklenen` sınıf alanı olamaz, `arkaplan` içine yakalanamaz, üst düzey değişkeni ilk değer almaz. `hata(...)` `oto` ile kullanılamaz.

`ertele` gövdesi bulunduğu kapsamdan her çıkışta (blok sonu, `dur`, `devam`, `don`, `?`) ters sırayla çalışır. `hata_ertele` yalnızca fonksiyon hata döndürürken çalışır (`don hata(...)`, `?`, ya da çalışma anında hata taşıyan bir `beklenen` döndürülürken). Gövde `ertele ifade;` ya da `ertele { ... }` olabilir; içinden `don`, `?` ya da dışarıya `dur`/`devam` ile çıkılamaz. Dönüş değeri `ertele` çalışmadan önce hesaplanır.

```upp
fonk ac(metin ad) -> beklenen[Dosya] {
    oto d = dosya_ac(ad)?;
    hata_ertele d.kapat();            // yalnızca aşağıdaki adımlardan biri başarısız olursa
    baslik_oku(d)?;
    don d;
}
```

### Tamsayı bölme
`/` ve `%` sıfıra bölmede ve `EN_KUCUK / -1` taşmasında açık panik verir (çekirdekte de). Sabit bölen sıfırsa derleme hatasıdır.

### `guvensiz`
Şunlar yalnızca `guvensiz { }` içinde serbesttir: işaretçiler (`*`, `&`), `bellek_ayir` / `bellek_bosalt`, `c_kod { }`, süreç belleği erişimi ve donanım erişimi (`upp.cekirdek.*`). `c_kod` ile bir `metin` değişkenine ham C işaretçisi yazılırsa runtime onu asla serbest bırakmaz. Yönetilen değer yazmak için `upp_metin_ata(&s, UPP_LIT("..."))` kullanın.

---

## 2. Türler

| Tür | C karşılığı | Açıklama |
|---|---|---|
| `sayi` / `s64` | `long long` | İşaretli 64 bit; soneksiz tamsayı sabitinin türü |
| `u64` / `usayi` | `unsigned long long` | İşaretsiz 64 bit (`5u64`) |
| `s32` / `u32`, `s16` / `u16` | `int32_t`… | Dar tamsayılar (`7u32`) |
| `bayt` / `s8`, `ubayt` / `u8` | `int8_t` / `uint8_t` | 8 bit; sabit taşması derleme hatası |
| `ondalik` | `double` | IEEE 754 |
| `metin` | `char*` (yönetilen) | UTF-8 |
| `mantik` | `bool` | `dogru` / `yanlis` |
| `bos` | `void` | Değer döndürmeyen fonksiyon |
| `dizi[T, N]` | C dizisi | `sayi a[] = [1, 2]`, sınır denetimli |
| `liste[T]`, `harita[K,V]` | değer (refcount + COW) | Dinamik koleksiyon |
| `fonk(P...) -> R` | fonksiyon işaretçisi | Fonksiyon değeri, `yok` olabilir |
| `beklenen[T, H]`, `beklenen[T]` | yapı (bayrak + union) | Değer ya da hata; `?` ile yayılır |
| `&metin`, `&dilim[T]` | görünüm | Kopyasız bakış |
| `sinif`, `secenek` | `struct`, `enum` | Kullanıcı türleri |

### Tür dönüşümü `(T)x` ve `boyut(T)`
C ile aynı yazım: `ubayt b = (ubayt)x;`, `u16 p = (u16)x;`, `sayi n = (sayi)3.9;` (3), `Renk r = (Renk)2;`.

- Sayısal türler, `mantik` ve `secenek` arasında serbesttir. Tamsayı daraltma C gibi keser (`(ubayt)300` = 44).
- `ondalik` → tamsayı: sonuç hedef aralığa sığmıyorsa ya da değer NaN ise açık panik verir (C'de belirsiz davranıştır).
- İşaretçi ↔ işaretçi, işaretçi ↔ tamsayı ve fonksiyon değeri ↔ işaretçi/tamsayı dönüşümleri yalnızca `guvensiz` içinde: `u16* vga = (u16*)0xB8000;`.
- `metin`, sınıf, koleksiyon ve dizi dönüştürülemez.

`boyut(T)` türün bayt boyutunu `sayi` olarak verir (C `sizeof`); `boyut(degisken)` değişkenin boyutunu verir (dizide tüm dizi). Derleme anı sabitidir, `sabit` ifadelerinde kullanılabilir.

---

## 3. İnterpolasyon ve Çıktı

- `"... {ifade} ..."`: `{` hemen ardından harf, `_` ya da `(` gelirse tam ifade başlar: `"{a[0]}"`, `"{f(x)}"`, `"{a + b * 2}"`, `"{k.ad}"`, `"{(1 + 2)}"`. Ayrıştırılamayan ifade hata verir.
- `{{` ve `}}` her metinde `{` ve `}` anlamına gelir. `{` ardından boşluk ya da rakam gelirse olduğu gibi kalır.
- Metin kaçışları: `\n \t \r \\ \" \e` (ESC), `\xHH` (bayt) ve `\uHHHH` (Unicode kod noktası, ör. `\u00e7` = ç). Bilinmeyen bir kaçış (ör. `\q`) derleme hatasıdır.
- Yazdırılabilir türler: tamsayılar, `ondalik`, `mantik`, `metin`, `&metin`, `secenek`.

| Fonksiyon | İmza | Açıklama |
|---|---|---|
| `upp.yaz(x)`, `upp.satir_yaz(x)` | `(yazdırılabilir) -> bos` | stdout (serbest hedefte seri port + VGA); `upp.satir_yaz(42)` geçerli |
| `upp.hata_yaz(x)`, `upp.hata_satir_yaz(x)` | `(yazdırılabilir) -> bos` | stderr |
| `upp.giris()` | `() -> metin` | stdin'den bir satır (çekirdekte klavye/seri port) |

Bu fonksiyonlar en fazla bir argüman alır, birden çok değer için interpolasyon kullanılır. printf biçim dizgeleri yoktur.

---

## 4. Genel (`upp.*`)

| Fonksiyon | İmza | Not |
|---|---|---|
| `upp.uzunluk(x)` | `metin / dizi / liste / harita / görünüm -> sayi` | |
| `upp.metinden_sayiya(s)` | `(metin) -> sayi` | |
| `upp.sayidan_metin(n)` | `(sayi) -> metin` | |
| `upp.ondalikdan_metin(d)` | `(ondalik) -> metin` | 17 anlamlı basamak |
| `upp.metinden_ondalik(s)` | `(metin) -> ondalik` | barındırılan |
| `upp.min`, `upp.max`, `upp.mutlak` | `sayi` | |
| `upp.zaman()` | `() -> sayi` | ms, monotonik (çekirdekte de var) |
| `upp.uyut(ms)` | `(sayi) -> bos` | çekirdekte de var |
| `upp.rastgele(a, b)` | `(sayi, sayi) -> sayi` | `[a, b]` |
| `upp.platform()` | `() -> metin` | `"windows"` / `"linux"` |
| `upp.arguman_sayisi()`, `upp.arguman(i)` | | Komut satırı |
| `upp.mesaj`, `upp.hata`, `upp.ses_cal` | | Windows'ta iletişim kutusu/ses |

Bu bölümdeki her şey, `uzunluk`/`min`/`max`/`mutlak`/`sayidan_metin`/`metinden_sayiya`/`ondalikdan_metin` dışında, barındırılandır.

**Kaldırılanlar** (yerine ne yazılacağını söyleyen derleme hatası verir):
- `upp.metin_bosalt`.
- Düz dosya adları `upp.dosya_oku/yaz/ekle/var_mi/sil/boyut` → `upp.dosya.*`.
- `upp.ortam_al/yaz` → `upp.sistem.*`.
- `upp.metin.icinde` → `upp.metin.bul`.
- `upp.yol.klasor_olustur` → `upp.yol.dizin_olustur`.
- `upp.matematik.sin/cos/tan` → `sinus/kosinus/tanjant`.
- `upp.json.dizi_eleman` → `upp.json.indeks_al`.
- `upp.bellek.*`, `upp.girdi.*`, `upp.cizim.*` kısayolları → `upp.windows.*`.

---

## 5. `upp.metin`

| Fonksiyon | İmza |
|---|---|
| `kes(s, bas, adet)` | `(metin, sayi, sayi) -> metin`; kod noktası cinsinden başlangıç ve adet |
| `buyuk(s)`, `kucuk(s)` | Türkçe kurallı: i↔İ, ı↔I |
| `kirp(s)` | Baştaki ve sondaki boşluk |
| `baslar(s, on)`, `biter(s, son)` | `-> mantik` |
| `bul(s, aranan)` | `-> sayi`; kod noktası indeksi, yoksa -1 |
| `degistir(s, eski, yeni)` | `-> metin` |
| `bol(s, ayirac)` | `-> liste[metin]` |
| `birlestir(liste, ayirac)` | `(liste[metin], metin) -> metin` |
| `kod(s, i)`, `ileri(s, i)` | Kod noktası değeri / bayt uzunluğu |
| `hex_coz(h)`, `hex_kodla(s)` | Onaltılık ↔ bayt |
| `karakter(kod)` | `sayi -> metin`: kod noktasından tek karakterlik metin (geçersizse boş) |
| `tekrar(s, n)` | `(metin, sayi) -> metin`: `s`'yi `n` kez yan yana yazar |

## 6. `upp.dosya` (barındırılan)

- `oku(yol) -> metin`
- `yaz(yol, icerik) -> mantik`, `ekle(yol, icerik) -> mantik`
- `var_mi(yol) -> mantik`, `sil(yol) -> mantik`
- `boyut(yol) -> sayi` (yoksa -1)
- `listele(dizin) -> liste[metin]`

## 7. `upp.yol` (barındırılan)

- `birlestir(a, b)`, `dizin(yol)`, `dosya_adi(yol)`, `uzanti(yol)` (`".txt"`)
- `mutlak_mi(yol)`, `gercek(yol)`, `calisma()`
- `dizin_var_mi(yol)`, `dizin_olustur(yol)` (iç içe oluşturur)

## 8. `upp.sistem` ve `upp.stdio` (barındırılan)

| Fonksiyon | İmza | Açıklama |
|---|---|---|
| `sistem.calistir(komut)` | `-> SurecCikti { kod, cikti, basarili }` | stdout ve stderr'i yakalar |
| `sistem.kos(komut)` | `-> sayi` | Konsolu devralır (etkileşimli); çıkış kodunu döner |
| `sistem.komut_bul(ad)` | `-> metin` | PATH'te arar |
| `sistem.exe_dizin()`, `sistem.pid()` | | |
| `sistem.ortam_al(ad)`, `sistem.ortam_yaz(ad, deger)` | | Ortam değişkeni |
| `sistem.cikis(kod)` | `-> bos` | Süreci sonlandırır |
| `stdio.gelen_oku()`, `stdio.yaz(json)` | | LSP `Content-Length` çerçeveleme |

## 9. Platform API'leri

- **`upp.linux.*`** (yalnızca `--hedef linux`): `pid()`, `sinyal_gonder(pid, sinyal)`, `proc_oku(pid, dosya)`. `guvensiz` içinde: `sayi_oku(pid, adres)`, `sayi_yaz(pid, adres, deger)`.
- **`upp.windows.*`** (yalnızca Windows):
  - `mesaj`, `hata`, `ses_cal`
  - `girdi.fare_tasi(x, y)`, `girdi.fare_tikla()`, `girdi.tus_bas(tus)`
  - `cizim.baslat(pencere)`, `cizim.temizle()`, `cizim.kutu(x, y, g, y, renk)`, `cizim.yazi(x, y, metin, renk)`
- **`upp.windows.bellek.*`** (`guvensiz`):
  - `baglan(surec) -> sayi`, `modul_bul(dll) -> sayi`
  - `sayioku(adres)`, `sayiyaz(adres, deger)`, `oku_ondalik`, `yaz_ondalik`, `oku_metin(adres, uzunluk)`
  - `zincir_oku(taban, [ofsetler])`, `yama_yap(adres, hex)`, `koruma_degistir(adres, boyut, izin)`
  - `son_hata()`, `okundu()`

## 10. `upp.matematik`

- `karekok`, `us(a, b)`, `sinus`, `kosinus`, `tanjant`, `atan2(y, x)` → `ondalik`
- `taban`, `tavan`, `yuvarla` → `sayi`
- `mutlak`, `sinirla(x, a, b)`, `yumusat(a, b, t)`, `pi()`, `e()` (serbest hedefte de var)
- `mesafe(Vektor3, Vektor3)`, `mesafe_3d(...)`, `aci_hesapla(...) -> Yonelme`, `dunya_ekran(...) -> EkranNokta`

## 11. `liste[T]` ve `harita[K,V]`

- **`liste`**: `ekle(x)`, `al(i)`, `yaz(i, x)`, `sil(i)`, `temizle()`, `uzunluk()`, `bosalt()`. `L[i]` okur ve yazar (sınır denetimli), `L[a..b]` (sayi/ondalik) dilim verir. `her (oto x in L)` elemanları gezer.
- **`harita`**: `koy(k, v)`, `al(k)` (anahtar yoksa türün sıfır değeri), `var_mi(k)`, `sil(k)`, `temizle()`, `uzunluk()`, `anahtarlar()` (eklenme sırasıyla), `bosalt()`. `her (oto k in h)` anahtarları eklenme sırasıyla gezer.
- Eleman ve anahtar türleri için bkz. [Koleksiyonlar](#koleksiyonlar).

## 12. `upp.json` (barındırılan)

- Ayrıştırma ve yazma: `ayristir(metin) -> JSONDeger`, `olustur(d) -> metin`
- Sorgu: `tur(d)` (`"nesne"`, `"dizi"`, `"metin"`, `"sayi"`, `"mantik"`, `"bosluk"`), `anahtar_al(d, k)`, `indeks_al(d, i)`, `anahtarlar(d)`, `uzunluk(d)`, `var_mi(d)`
- Değer alma: `metin_al`, `sayi_al`, `ondalik_al`, `mantik_al`
- Kurma: `nesne()`, `dizi()`, `metin_yap`, `sayi_yap`, `ondalik_yap`, `mantik_yap`, `yok_yap`, `koy(nesne, k, d)`, `ekle(dizi, d)`
- `bosalt(kok)`

## 13. `arkaplan` ve `Kilit` (barındırılan)

`oto is = arkaplan { ... }; is.bekle();` biçiminde kullanılır. `metin` ve sayısal değerler iş parçacığına derin kopya olarak aktarılır. Koleksiyonlar, `JSONDeger` ve görünümler yakalanamaz. Kilit kullanımı: `Kilit k; k.kilitle(); ... k.birak();`

## 14. `upp.cekirdek` — donanım (yalnızca `--hedef serbest|cekirdek`)

Donanıma doğrudan dokunan satırlar `guvensiz` ister:

| Fonksiyon (`guvensiz`) | Açıklama |
|---|---|
| `port_yaz8/16/32(port, deger)`, `port_oku8/16/32(port) -> sayi` | x86 G/Ç portları |
| `bellek_yaz8/16/32/64(adres, deger)`, `bellek_oku8/16/32/64(adres) -> sayi` | volatile MMIO |
| `dur()`, `kesme_kapat()`, `kesme_ac()` | `hlt`, `cli`, `sti` |
| `yigin_kur(adres, boyut)` | Metin/koleksiyon ayırıcısına bellek bölgesi verir |

Geri kalanı güvenli koddan çağrılır:

| Fonksiyon | Açıklama |
|---|---|
| `kesmeler_kur()` | IDT, PIC ve 1000 Hz PIT'i kurar (açılışta kendiliğinden çağrılır) |
| `tik() -> sayi` | Açılıştan beri zamanlayıcı tıkı (1 ms) |
| `tus_oku() -> sayi` | Klavye (PS/2, Türkçe Q) ya da seri porttan bir kod noktası; yoksa 0. Ok/Home/End/Delete tuşları `ESC [ A` biçiminde gelir, Ctrl+harf kontrol kodudur. Seride CR ve CR LF tek satır sonudur |
| `tus_bekle() -> sayi` | Bir tuş gelene kadar bekler; bu sırada hazır görevler çalışır |
| `bellek_toplam() -> sayi` | Multiboot bellek haritasındaki kullanılabilir RAM (bayt) |
| `yigin_boyut() -> sayi`, `yigin_kullanilan() -> sayi` | Ayırıcı bölgesinin boyutu ve kullanılan bayt |
| `gorev_baslat(f, arg, ad) -> sayi` | `fonk(sayi) -> bos` türündeki `f`'yi yeni görevde `f(arg)` olarak başlatır; kimliği (PID) döner, tablo doluysa -1 |
| `sira_ver()` | Sırayı başka bir hazır göreve verir |
| `gorev_kimlik() -> sayi` | Çalışan görevin kimliği (ana akış 1) |
| `gorev_listesi() -> liste[sayi]` | Yaşayan görevlerin kimlikleri |
| `gorev_ad(id) -> metin`, `gorev_durum(id) -> metin`, `gorev_baslama(id) -> sayi` | Ad; `"çalışıyor"` / `"hazır"` / `"uyuyor"` (yoksa `""`); başlama zamanı (ms) |
| `gorev_oldur(id) -> mantik` | Görevi sonlandırır (ana akış 1 sonlandırılamaz) |
| `tarih() -> metin` | CMOS saati: `"YYYY-AA-GG SS:DD:ss"` |
| `kapat()`, `yeniden_baslat()` | ACPI ile kapatma (QEMU, Bochs, VirtualBox; olmazsa isa-debug-exit); 8042 ile sıfırlama |
| `ekran_temizle()` | VGA ekranını ve seri terminali temizler |
| `islemci() -> metin` | CPUID marka adı |
| `disk_var(d) -> mantik`, `disk_sektor(d) -> sayi`, `disk_model(d) -> metin` | ATA diski `d` (0–3: birincil/ikincil kanal, ana/yardımcı; QEMU `-drive if=ide,index=d`); sektör sayısı (512 bayt) ve model adı |
| `disk_oku(d, lba, adet, tampon, ofs) -> mantik`, `disk_yaz(...)` | `adet` sektörü tampona okur / tampondan yazar (PIO, LBA28/LBA48, yazmadan sonra önbellek boşaltma) |

**Bellek:** Açılışta yığın, Multiboot bellek haritasındaki en büyük boş bölgeye taşınır (ilk 1 GiB içinde; QEMU'nun varsayılan 128 MiB'ında yaklaşık 122 MiB). Önyükleyici bilgisi yoksa 4 MiB'lık `.bss` alanı kullanılır.

**Görevler** işbirlikçidir. Geçiş yalnızca `upp.uyut`, `sira_ver` ve girdi beklerken (`tus_bekle`, `upp.giris`) olur. Bu yüzden runtime'ın paylaşılan durumu kilitsiz güvendedir, ama uzun hesap yapan bir görev arada `sira_ver()` çağırmalıdır. Her görevin 64 KiB yığını vardır, en çok 32 görev olabilir. Biten görevin yığını bir sonraki geçişte geri verilir.

Çekirdek hedefinde `upp.giris()` (klavye ve seri porttan satır okur, yankılar, geri silmeyi destekler), `upp.zaman()` ve `upp.uyut(ms)` da çalışır. VGA ekranı ANSI renklerini (`\e[1;32m`), `\e[2J`, `\e[H`, `\e[K`, `\e[nC`, `\e[nD` dizilerini ve kutu çizgi karakterlerini (`├ └ │ ─`) gösterir. İşlemci istisnaları (sıfıra bölme, sayfa hatası vb.) Türkçe adı ve RIP ile panik verir. QEMU'da panik 3 koduyla çıkar.

Bu yerleşiklerle yazılmış eksiksiz bir örnek: [Uppix](../uppix/README.md), u++ ile yazılmış Unix benzeri işletim sistemi (ext4 kök, FAT32/exFAT).

### `upp.tampon` — bayt tamponları (her hedefte)

İkili veriyi (disk sektörleri, dosya sistemi yapıları, ağ paketleri) güvenli kodda işlemek içindir. Tampon, `sayi` bir tutamaçla gösterilir. Her erişim sınır denetimlidir: geçersiz tutamaç ya da taşma açık panik verir. Çok baytlı değerler küçük sonludur (little-endian). Tamponlar elle bırakılır (`birak`).

| Fonksiyon | Açıklama |
|---|---|
| `yeni(boyut) -> sayi`, `birak(t)`, `boyut(t) -> sayi` | Sıfırlanmış tampon; bırakma; boyut |
| `oku8/16/32/64(t, ofs) -> sayi`, `yaz8/16/32/64(t, ofs, deger)` | Küçük sonlu tamsayı (32 bit işaretsiz okunur; 64 bit ham bit deseni) |
| `metin(t, ofs, n) -> metin`, `metin_yaz(t, ofs, s) -> sayi` | Ham bayt ↔ metin (UTF-8 baytları; yazılan bayt sayısı döner) |
| `utf16_metin(t, ofs, n) -> metin`, `utf16_yaz(t, ofs, s, sinir) -> sayi` | UTF-16LE ↔ metin (vekil çiftler dahil; 0x0000'da durur; yazılan kod birimi sayısı) |
| `kopyala(hedef, hofs, kaynak, kofs, n)`, `doldur(t, ofs, n, deger)` | Örtüşmeye dayanıklı kopya; doldurma |
| `crc32c(t, ofs, n, baslangic) -> sayi` | CRC32C (Castagnoli), son XOR'suz (ext4 biçimi); standart değer için `^ 4294967295` |

---

## 15. Hedefler

| Hedef | Çıktı | Açıklama |
|---|---|---|
| (varsayılan) | `.exe` / `.out` | Bu işletim sistemi |
| `--hedef windows`, `--hedef linux` | | Çapraz ABI |
| `--hedef serbest` | ELF `.o` | libc'siz; giriş noktası `upp_cekirdek_giris()` |
| `--hedef cekirdek` | `ad.elf` + `ad32.elf` | Multiboot ile açılan x86_64 çekirdeği; çıktı COM1 + VGA'ya gider, QEMU varsa çalıştırır |

Serbest ve çekirdek hedeflerinde işletim sistemi isteyen ("barındırılan") yerleşikler, `arkaplan` ve `Kilit` derleme hatası verir. Araç zinciri: Linux'ta `gcc`, Windows'ta `x86_64-elf-gcc` ya da WSL içinde `gcc`. Çalıştırmak için `qemu-system-x86_64` gerekir.

## 16. Komut Satırı

```text
uppc <dosya.upp> [-o ad] [--sadece-derle | -c] [--hedef h] [--analiz] [--json-hata]
                 [--bicim | --bicim-kontrol] [--ast] [--runtime yol] [--disk imaj]...
uppc yeni <ad> [--hedef cekirdek]     # upp.proje + kaynak/ana.upp
uppc derle [dizin]                    # upp.proje'yi derle
uppc calistir [dizin]                 # derle ve çalıştır (çekirdekte QEMU)
uppc asistan [dizin]                  # menülü derleme asistanı
uppc kendini-derle [--hizli]          # self-host: 3 aşama + sabit nokta + bootstrap/uppc.c
```

`upp.proje` biçimi:

```text
ad = merhaba
giris = kaynak/ana.upp
hedef = yerel        # yerel | windows | linux | serbest | cekirdek
cikti = derleme/merhaba
bayraklar =          # isteğe bağlı ek uppc bayrakları
diskler = a.img, b.img   # çekirdek: QEMU'ya IDE disk olarak (sırayla sda, sdb...)
disk_hazirla = disk_yap.sh  # bir disk imajı eksikse proje dizininde sh ile çalışır
```

`--disk imaj` (en çok 4) ham disk imajını çekirdek hedefinde QEMU'ya `-drive if=ide,format=raw` olarak ekler.
