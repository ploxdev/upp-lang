# u++ — VS Code / Cursor IDE Eklentisi (Language Support & LSP)

<p align="center">
  <strong>Türkçe sistem programlama dili u++ için resmi VS Code & Cursor IDE eklentisi.</strong>
</p>

<p align="center">
  <a href="../README.md"><img src="https://img.shields.io/badge/u%2B%2B-v3.0-blue.svg?style=flat-square" alt="u++ sürüm" /></a>
  <a href="../README.md"><img src="https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg?style=flat-square" alt="Platform" /></a>
  <a href="../OGREN.md"><img src="https://img.shields.io/badge/dil-T%C3%BCrk%C3%A7e-red.svg?style=flat-square" alt="Dil" /></a>
</p>

---

Bu eklenti, **u++** dili (`.upp` ve `.uph`) için tam entegre modern bir geliştirme deneyimi sağlar. Arka planda çalışan Language Server Protocol (LSP) mimarisi sayesinde anlık hata yakalama, akıllı kod tamamlama, dokümantasyon ipuçları ve tek tuşla derleme/çalıştırma desteği sunar.

## ✨ Öne Çıkan Özellikler

- **🎨 Zengin Sözdizimi Vurgulama:** `.upp` ve `.uph` dosyaları için Türkçe anahtar sözcükler, yerleşik `upp.*` modülleri ve C-interop blokları.
- **⚡ Anlık LSP Tanısı (Diagnostics):** Kod yazılırken `Lexer → Parser → Tür Denetimi → Bellek Güvenliği` katmanlarında gerçek zamanlı hata denetimi (`--analiz` modu ile GCC beklemeden milisaniyeler içinde).
- **💡 Akıllı Kod Tamamlama (IntelliSense):** Standart kütüphane (`upp.*`), koleksiyonlar (`liste`, `harita`), sınıf metotları ve dil şablonları (snippets).
- **📖 Hover & Fonksiyon İmzaları:** Yerleşik standart kütüphane fonksiyonları ve kullanıcı tanımlı fonksiyonlar için parametre/dönüş türü ipuçları.
- **🚀 Tek Tuşla Derleme ve Çalıştırma:**
  - **`F5`**: Doğrudan aktif programı derler ve çalıştırır.
  - **`Ctrl+Shift+B`**: Sadece ikili (executable) dosyayı derler.
- **🛡️ Hızlı Düzeltmeler (Code Actions):** Bellek ihlallerinde kodu otomatik olarak `guvensiz { ... }` içine alma veya `fonk ana()` iskeleti ekleme önerileri.

---

## 📋 Sistem Gereksinimleri

| Bileşen | Minimum Sürüm | Açıklama |
|---|---|---|
| **VS Code / Cursor** | 1.80+ | Düzenleyici ortamı |
| **Python** | 3.8+ | Dil sunucusu ve bootstrap derleyici için (harici pip paketi gerekmez) |
| **Node.js & npm** | 18+ | Sadece eklentiyi yerel olarak paketlemek (`.vsix`) için |
| **GCC (MinGW / build-essential)** | Güncel | C11 native binary bağlama için |

> **Windows İpucu:** Windows Store Python kısayolu yerine doğrudan kurulan Python sürümünü kullanmanız önerilir. Gerekirse eklenti ayarlarından (`upp.pythonYolu`) doğrudan Python yolunu belirtebilirsiniz.

---

## 📦 Kurulum

### Hazır Paketten (.vsix) Yükleme

Proje kök dizininde hazır `.vsix` dosyası bulunuyorsa doğrudan kurabilirsiniz:

```powershell
# Cursor için:
cursor --install-extension eklenti/u-plus-plus-3.0.0.vsix

# VS Code için:
code --install-extension eklenti/u-plus-plus-3.0.0.vsix
```
*Veya VS Code/Cursor arayüzünde: **Extensions (Ctrl+Shift+X) → ... Menüsü → Install from VSIX...***

---

### Kaynaktan Derleyip Yükleme

```powershell
# 1. Eklenti dizinine gidin
cd eklenti

# 2. Bağımlılıkları kurun ve paketleyin
npm install
npm run compile
npm run package

# 3. Üretilen vsix paketini yükleyin
code --install-extension u-plus-plus-3.0.0.vsix
```

---

## 🛠️ Yapılandırma Seçenekleri

VS Code / Cursor `settings.json` üzerinden özelleştirebileceğiniz ayarlar:

| Ayar | Varsayılan | Açıklama |
|---|---|---|
| `upp.pythonYolu` | `"python"` | Python çalıştırıcı yolu (`python.exe` adresi) |
| `upp.derleyici` | `"python"` | Dil motoru seçimi: `"python"` (bootstrap) veya `"native"` (uppc.exe) |
| `upp.uppcYolu` | `""` | Native derleyici yolu (boşsa `derleme/uppc.exe` kullanılır) |
| `upp.ciktiKoku` | `"derleme"` | Üretilen `.c` ve `.exe` dosyalarının yazılacağı hedef klasör |
| `upp.gccTanilari` | `false` | Kayıt anında tam GCC derleme tanılarını da JSON olarak al |
| `upp.iz.sunucu` | `"off"` | Dil sunucusu log düzeyi (`"off"` / `"messages"` / `"verbose"`) |

---

## ⌨️ Klavye Kısayolları & Komutlar

Komut Paleti (`Ctrl+Shift+P`) üzerinden kullanılabilir komutlar:

| Kısayol | Komut | Açıklama |
|---|---|---|
| **`F5`** | `u++: Çalıştır` | Programı derler ve terminalde çalıştırır |
| **`Ctrl+Shift+B`** | `u++: Derle` | Yalnızca derleme yapar (`--sadece-derle`) |
| — | `u++: Yalnızca C üret` | Sadece C11 transpile kodunu üretir (`--sadece-c`) |

---

## 🏗️ Mimari ve Çalışma Mantığı

```text
┌───────────────────────────────────────────────┐
│       VS Code / Cursor (TypeScript Client)    │
│            extension.ts · derleme.ts          │
└───────────────────────┬───────────────────────┘
                        │ Language Server Protocol (stdio)
                        ▼
┌───────────────────────────────────────────────┐
│           Python LSP Sunucusu (LSP Server)    │
│           upp_lsp.py · analiz.py (AST)        │
└───────────────────────┬───────────────────────┘
                        │ --analiz JSON IPC
                        ▼
┌───────────────────────────────────────────────┐
│     u++ Derleyicisi (uppc.py veya uppc.exe)   │
│   Lexer → Parser → Tip Denetimi → Güvenlik    │
└───────────────────────────────────────────────┘
```

1. **Hafif ve Hızlı Tanı:** Siz kod yazarken GCC tetiklenmez. Eklenti derleyicinin `--analiz` modunu çağırarak bellek güvenliği ve tip denetimlerini bellek üzerinde yapar, sonuçları JSON olarak editöre iletir.
2. **Çift Motor Uyumu:** İster Python bootstrap derleyiciyi, isterseniz `derleme/uppc.exe` native derleyicisini LSP arkasında çalıştırabilirsiniz.

---

## Katkı ve Lisans

Bu eklenti, ana **u++** projesinin bir parçasıdır. Sorun bildirimleri ve katkılar için ana depo issue/PR mekanizmasını kullanabilirsiniz.
