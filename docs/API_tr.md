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
`liste[T]`, `harita[K,V]` ve `JSONDeger` tutamaçtır. `.bosalt()` / `upp.json.bosalt()` ile kapatılır. İçlerindeki metinler refcount ile paylaşılır.

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
| `liste[T]`, `harita[K,V]` | tutamaç | Dinamik koleksiyon |
| `&metin`, `&dilim[T]` | görünüm | Kopyasız bakış |
| `sinif`, `secenek` | `struct`, `enum` | Kullanıcı türleri |

---

## 3. İnterpolasyon ve Çıktı

- `"... {ifade} ..."`: `{` hemen ardından harf, `_` ya da `(` gelirse tam ifade başlar: `"{a[0]}"`, `"{f(x)}"`, `"{a + b * 2}"`, `"{k.ad}"`, `"{(1 + 2)}"`. Ayrıştırılamayan ifade hata verir.
- `{{` ve `}}` her metinde `{` ve `}` anlamına gelir. `{` ardından boşluk ya da rakam gelirse olduğu gibi kalır.
- Metin kaçışları: `\n \t \r \\ \" \e` (ESC) ve `\xHH`.
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

- **`liste`**: `ekle(x)`, `al(i)`, `yaz(i, x)`, `sil(i)`, `temizle()`, `uzunluk()`, `bosalt()`. `L[a..b]` (sayi/ondalik) dilim verir.
- **`harita`**: `koy(k, v)`, `al(k)`, `var_mi(k)`, `sil(k)`, `temizle()`, `uzunluk()`, `anahtarlar()` (eklenme sırasıyla), `bosalt()`. `her (oto k in h)` anahtarları gezer.

## 12. `upp.json` (barındırılan)

- Ayrıştırma ve yazma: `ayristir(metin) -> JSONDeger`, `olustur(d) -> metin`
- Sorgu: `tur(d)` (`"nesne"`, `"dizi"`, `"metin"`, `"sayi"`, `"mantik"`, `"bosluk"`), `anahtar_al(d, k)`, `indeks_al(d, i)`, `anahtarlar(d)`, `uzunluk(d)`, `var_mi(d)`
- Değer alma: `metin_al`, `sayi_al`, `ondalik_al`, `mantik_al`
- Kurma: `nesne()`, `dizi()`, `metin_yap`, `sayi_yap`, `ondalik_yap`, `mantik_yap`, `yok_yap`, `koy(nesne, k, d)`, `ekle(dizi, d)`
- `bosalt(kok)`

## 13. `arkaplan` ve `Kilit` (barındırılan)

`oto is = arkaplan { ... }; is.bekle();` biçiminde kullanılır. `metin` ve sayısal değerler iş parçacığına derin kopya olarak aktarılır. Koleksiyonlar, `JSONDeger` ve görünümler yakalanamaz. Kilit kullanımı: `Kilit k; k.kilitle(); ... k.birak();`

## 14. `upp.cekirdek` — donanım (yalnızca `--hedef serbest|cekirdek`, `guvensiz`)

| Fonksiyon | Açıklama |
|---|---|
| `port_yaz8/16/32(port, deger)`, `port_oku8/16/32(port) -> sayi` | x86 G/Ç portları |
| `bellek_yaz8/16/32/64(adres, deger)`, `bellek_oku8/16/32/64(adres) -> sayi` | volatile MMIO |
| `dur()`, `kesme_kapat()`, `kesme_ac()` | `hlt`, `cli`, `sti` |
| `yigin_kur(adres, boyut)` | Metin/koleksiyon ayırıcısına bellek bölgesi verir (varsayılan 4 MiB `.bss`) |
| `kesmeler_kur()` | IDT, PIC ve 1000 Hz PIT'i kurar (açılışta kendiliğinden çağrılır) |
| `tik() -> sayi` | Açılıştan beri zamanlayıcı tıkı (1 ms) |
| `tus_oku() -> sayi` | Klavye (PS/2, Türkçe Q) ya da seri porttan bir kod noktası; yoksa 0 |
| `tus_bekle() -> sayi` | Bir tuş gelene kadar `hlt` ile bekler |

`kesmeler_kur` dışındaki satırlar `guvensiz` gerektirmez.

Çekirdek hedefinde `upp.giris()` (klavye ve seri porttan satır okur, yankılar, geri silmeyi destekler), `upp.zaman()` ve `upp.uyut(ms)` da çalışır. İşlemci istisnaları (sıfıra bölme, sayfa hatası vb.) Türkçe adı ve RIP ile panik verir. QEMU'da panik 3 koduyla çıkar.

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
                 [--bicim | --bicim-kontrol] [--ast] [--runtime yol]
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
```
