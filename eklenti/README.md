# u++ — VS Code ve Cursor Eklentisi

u++ programlama dili için geliştirilmiş resmi VS Code ve Cursor editör eklentisidir.

Bu eklenti, harici bir Python kurulumuna, LSP betiklerine veya karmaşık arka plan süreçlerine ihtiyaç duymaz. Tamamen **iki katmanlı** ve hafif bir mimari üzerine kuruludur:
- **Katman 1 (Temel Mod):** Sisteminizde `uppc` derleyicisi bulunmasa dahi sözdizimi renklendirmesi, kod parçacıkları (snippets) ve zengin TypeScript sözlüğü anında çalışır.
- **Katman 2 (Gelişmiş Mod):** Yerel (native) `uppc` derleyicisi algılandığında canlı tanı (diagnostics), kod biçimlendirme ve derle/çalıştır özellikleri otomatik olarak devreye girer.

---

## 🚀 Çalışma Modları

### Katman 1 — Temel Mod (`uppc` Gerekmez)
Sistemde derleyici olmasa bile hemen kod yazmaya başlayabilirsiniz:
- `.upp` ve `.uph` dosyaları için TextMate tabanlı sözdizimi vurgulama,
- Yaygın dil yapıları için hazır kod parçacıkları (snippets), otomatik girintileme ve parantez eşleme,
- TypeScript tabanlı semantik sözlük: Anahtar sözcükler, `upp.*` kütüphanesi, `liste` ve `harita` metotları, hover (üzerine gelince bilgi) ve sembol anahatları (outline),
- Durum çubuğunda sarı renkli `u++: Temel mod` göstergesi,
- Derleyici henüz hazır değilse Derle / Çalıştır komutları devre dışı kalır ve kullanıcıyı rahatsız etmeyen nazik bir indirme / yol belirtme bildirimi sunulur.

### Katman 2 — Gelişmiş Mod (Native `uppc` ile)
Eklenti yerel `uppc` derleyicisini algıladığında tüm yetenekler etkinleşir:
- **Canlı Tanı (Diagnostics):** Kod yazılırken veya kaydedildiğinde arka planda `uppc --analiz` koşturulur ve derleyicinin JSON çıktısı anında editörde kırmızı hata çizgileri olarak gösterilir.
- **Kod Biçimlendirme (Formatting):** `Belgeyi Biçimlendir` (`Shift+Alt+F`) komutuyla kaynak kod `uppc --bicim` üzerinden standart 4 boşluk girintili u++ biçimine sokulur.
- **Derleme ve Çalıştırma:** Tek tuşla (`F5`) programınız yerel ikiliye derlenir ve terminalde yürütülür.

---

## 🔍 Derleyici Keşif Sırası

Eklenti, native derleyiciyi şu öncelik sırasına göre otomatik olarak arar:

1. **Özel Kullanıcı Ayarı:** `upp.uppcYolu` ayarında belirtilen yol,
2. **Çalışma Alanı (Repo Kökü):**
   - Windows: `<repo>/derleyici/uppc.exe`
   - Linux: `<repo>/derleyici/uppc` veya `<repo>/derleyici/uppc.out`
3. **Sistem Ortamı:** İşletim sisteminin `PATH` ortam değişkeninde kayıtlı dizinler.

### Sürüm ve Protokol Doğrulaması
Eklenti, derleyici ile güvenli iletişim kurmak için JSON tabanlı sürüm sözleşmesini kullanır:

```bash
uppc --surum --json
```

Örnek çıktı:
```json
{"dil":"u++","surum":"3.0.1","protokol":1}
```

Eklenti `protokol: 1` (`--analiz` JSON hata şeması) uyumluluğunu doğrular. Uyumsuz bir sürüm saptanırsa editör güvenli şekilde Katman 1'de kalır ve durum çubuğunda uyarı verir.

---

## ⌨️ Komutlar ve Kısayollar

| Komut | Kısayol | Gereksinim | Açıklama |
|---|---|---|---|
| `u++: Çalıştır` | `F5` | `uppc` hazır | Programı derler ve terminalde anında çalıştırır. |
| `u++: Derle` | `Ctrl+Shift+B` | `uppc` hazır | Yalnızca çalıştırılabilir ikiliyi üretir (`derleme/`). |
| `u++: Yalnızca C üret` | — | `uppc` hazır | Yalnızca C11 ara kodunu üretir (`--sadece-c`). |
| `Belgeyi Biçimlendir` | `Shift+Alt+F` | `uppc` hazır | Kaynak kodu standart u++ biçimine sokar (`--bicim`). |
| `u++: Derleyiciyi tara` | — | Her zaman | Derleyiciyi yeniden tarar ve durumu günceller. |
| `u++: Derleyici yolu belirt` | — | Her zaman | Özel bir `uppc` çalıştırılabilir dosya yolu seçmenizi sağlar. |

---

## 📦 Sistem Gereksinimleri

| Bileşen | Asgari Sürüm | Rolü |
|---|---|---|
| **VS Code / Cursor** | 1.85+ | Eklentinin çalışacağı editör |
| **Node.js** | 18+ | Eklenti geliştirme ve VSIX paketleme için |
| **Native `uppc`** | v3.0+ | Tanı, biçimlendirme ve derleme özellikleri için |
| **GCC (MinGW / build-essential)** | Güncel | C kodunu yerel makine ikilisine bağlamak için |

---

## 🛠️ Kurulum ve Geliştirme

### Kaynak Koddan Paketleme ve Kurulum

```powershell
# Eklenti dizinine geçin
cd eklenti

# Bağımlılıkları yükleyin ve testleri çalıştırın
npm install
npm test

# TypeScript kodlarını derleyin ve VSIX paketini oluşturun
npm run compile
npm run package

# Üretilen VSIX paketini editöre kurun
code --install-extension u-plus-plus-3.0.1.vsix
```

Hazır bir `.vsix` dosyanız varsa, VS Code / Cursor içerisinden **Uzantılar (Extensions) → ... menüsü → Install from VSIX...** seçeneğini kullanarak da doğrudan yükleyebilirsiniz.

---

## ⚙️ Eklenti Ayarları

| Ayar | Varsayılan | Açıklama |
|---|---|---|
| `upp.uppcYolu` | `""` | Özel yerel derleyici yolu. Boş bırakılırsa önce repo içi `derleyici/` klasörüne, ardından `PATH`'e bakılır. |
| `upp.ciktiKoku` | `"derleme"` | Derlenen program ikilileri ve ara C kodlarının kaydedileceği klasör (repo köküne göre göreli veya mutlak). |

---

## 🏗️ Mimari Şema

```text
┌────────────────────────────────────────────────────────┐
│ Katman 1: Temel Mod (Her zaman etkin)                  │
│ TextMate Sözdizimi + Snippets + sozluk.ts (Hover/İmza) │
└───────────────────────────┬────────────────────────────┘
                            │ uppc --surum --json doğrulandı
                            ▼
┌────────────────────────────────────────────────────────┐
│ Katman 2: Native Araçlar                               │
│ - Canlı Tanı:   uppc <dosya> --analiz (JSON Hata Listesi) │
│ - Biçimlendir:  uppc <dosya> --bicim                   │
│ - Derleme:      uppc <dosya> --sadece-derle / --sadece-c│
└────────────────────────────────────────────────────────┘
```

> **Not:** `derleyici/` klasörü dilin derleyicisini (`uppc`), `derleme/` klasörü ise derlediğiniz kullanıcı programlarının çıktılarını barındırır.

---

## 🧪 Testler

Eklenti mantığı, derleyiciye bağımlı olmadan kapsamlı birim testleriyle doğrulanır:

```powershell
cd eklenti
npm test          # Sözlük, keşif, sürüm ve tanı testleri
npm run compile   # TypeScript tip denetimi
```

Testler hem Windows hem de Linux üzerinde uyumlu çalışacak şekilde tasarlanmıştır.
