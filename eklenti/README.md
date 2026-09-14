# u++ — VS Code / Cursor Dil Eklentisi

> **Türkçe programlama dili u++** için resmi düzenleyici eklentisi. Sözdizimi vurgulama, canlı hata tanısı (LSP), tamamlama, hover, derleme ve çalıştırma — tek pakette.

[![u++ sürüm](https://img.shields.io/badge/u%2B%2B-v3.0-blue)](../README.md)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey)](../README.md)
[![Dil](https://img.shields.io/badge/dil-Türkçe-red)](../OGREN.md)
[![Derleyici](https://img.shields.io/badge/derleyici-Python%20%7C%20native-green)](../README.md)

---

## Özellikler

| Özellik | Açıklama |
|---|---|
| **Sözdizimi vurgulama** | `.upp` ve `.uph` için Türkçe anahtar sözcükler, `upp.*` ad alanı |
| **Canlı tanı (LSP)** | Yazarken hata — `Lexer → Parser → tür → güvenlik` (`--analiz`; GCC yok) |
| **Tamamlama** | `upp.`, `upp.metin.`, `upp.yol.`, sınıf üyeleri, snippet’ler |
| **Hover & imza** | Yerleşik API, kullanıcı fonksiyonları, parametre ipuçları |
| **Gezinti** | Tanıma git, referanslar, belge simgeleri |
| **Code action** | `guvensiz { }` ile sar, `fonk ana()` iskeleti ekle |
| **Derleme görevleri** | **Derle**, **Çalıştır**, **Yalnızca C üret** |
| **Kısayollar** | **F5** = derle + çalıştır · **Ctrl+Shift+B** = derle |

Desteklenen dil yüzeyleri (v3.0): `kullan` / `olarak`, `sabit`, `secim (metin)`, `SurecCikti`, `Kilit`, `metin +=`, `upp.yol`, `upp.dosya`, `hata_yaz` / `hata_satir_yaz`, çoklu `--analiz` hatası.

---

## Kimler için?

- **u++ öğrenenler** — Türkçe sözdizimi, anında geri bildirim
- **u++ proje geliştiricileri** — tek tık derleme, native veya Python derleyici seçimi
- **VS Code / Cursor kullanıcıları** — Marketplace dışı yerel `.vsix` kurulumu

İlgili depo belgeleri:

- [OGREN.md](../OGREN.md) — dil sözleşmesi
- [docs/API_tr.md](../docs/API_tr.md) — kamu API
- [README.md](../README.md) — kurulum ve `uppc.py` / `uppc.exe`

---

## Gereksinimler

| Bileşen | Sürüm | Not |
|---|---|---|
| **VS Code** veya **Cursor** | 1.80+ | |
| **Python** | 3.8+ | Dil sunucusu; ek pip paketi yok |
| **Node.js** | 18+ | Yalnızca eklenti derlemesi (`npm`) |
| **u++ kökü** | — | Workspace’te `uppc.py` bulunmalı |
| **GCC** | — | Derleme görevleri için (MinGW-w64 / `build-essential`) |

> Windows’ta Store `python` kısayolu (çıkış 9009) sorun çıkarır. Ayarlar → Uygulamalar → Gelişmiş → **Uygulama yürütme diğer adları**’ndan kapatın veya `upp.pythonYolu` ile gerçek `python.exe` yolunu verin.

---

## Kurulum

### 1. Önkoşul

Depoyu klonlayın veya indirin. VS Code / Cursor’da **u++ kök klasörünü** (`uppc.py`’nin olduğu dizin) açın.

### 2. Eklentiyi derle ve paketle

```powershell
cd eklenti
npm install
npm run compile
npm run package
```

`u-plus-plus-3.0.0.vsix` oluşur.

### 3. IDE’ye yükle

```powershell
# Cursor
cursor --install-extension u-plus-plus-3.0.0.vsix

# VS Code
code --install-extension u-plus-plus-3.0.0.vsix
```

Alternatif: **Uzantılar → … → Install from VSIX** → `.vsix` dosyasını seçin.

### 4. Python yolunu ayarla (gerekirse)

`settings.json`:

```json
{
  "upp.pythonYolu": "C:\\Python314\\python.exe",
  "upp.derleyici": "python"
}
```

---

## Kullanım

### Günlük iş akışı

1. `ornekler/ornek1.upp` gibi bir `.upp` dosyası açın.
2. Yazarken canlı tanılar görünür (durum çubuğu: `u++ v3.0`).
3. **F5** — derle ve çalıştır.
4. **Ctrl+Shift+B** — yalnızca derle.

`.uph` başlık dosyaları aynı dil kimliğiyle açılır; **derlenmez** — bir `.upp` programından `kullan` ile katılır.

### Komut paleti

| Komut | İşlev |
|---|---|
| `u++: Derle` | `--sadece-derle` |
| `u++: Çalıştır` | Derle + ikiliyi çalıştır |
| `u++: Yalnızca C üret` | `--sadece-c` |

---

## Ayarlar

| Ayar | Varsayılan | Açıklama |
|---|---|---|
| `upp.pythonYolu` | `python` | LSP ve Python derleyici için `python.exe` |
| `upp.derleyici` | `python` | `python` veya `native` |
| `upp.uppcYolu` | *(boş)* | Native `uppc.exe` yolu; boşsa `derleme/uppc.exe` |
| `upp.ciktiKoku` | `derleme` | `--cikti` çıktı kökü |
| `upp.gccTanilari` | `false` | Kayıtta `--json-hata` ile GCC tanısı (yavaş) |
| `upp.iz.sunucu` | `off` | LSP izleme: `off` / `messages` / `verbose` |

### Python vs native derleyici

| | `python` | `native` |
|---|---|---|
| Canlı tanı | `uppc.py` in-process | `uppc.exe --analiz` |
| Derleme görevi | `python uppc.py …` | `uppc.exe …` |
| Hız | Yeterli | Daha hızlı |
| Kurulum | Her zaman hazır | `araclar/birlestir.py` + derleme gerekir |

Native derleyici üretimi:

```bash
python araclar/birlestir.py
python uppc.py derleme/uppc_birlesik.upp --sadece-derle --cikti derleme/uppc
```

---

## Mimari

```text
┌─────────────────────────────────────┐
│  VS Code / Cursor (TypeScript)      │
│  extension.ts · derleme.ts          │
└──────────────┬──────────────────────┘
               │ vscode-languageclient
               ▼
┌─────────────────────────────────────┐
│  Python LSP (upp_lsp.py)            │
│  analiz.py — tanı, tamamlama, hover │
└──────────────┬──────────────────────┘
               │ import / subprocess
               ▼
┌─────────────────────────────────────┐
│  uppc.py  veya  uppc.exe --analiz   │
└─────────────────────────────────────┘
```

- **Canlı tanı:** GCC çağrılmaz; `--analiz` JSON şeması.
- **Tam derleme:** Görevler `uppc.py` veya native ikili ile GCC’ye kadar gider.
- **Tamamlama / hover:** Python AST (`ide_program`); native yalnızca tanı için.

---

## F5 ve geliştirici notları

### Kullanıcı F5’i

Açık `.upp` dosyasında **F5** = **u++: Çalıştır** (kullanıcı programını derler ve çalıştırır). `when: editorLangId == upp` koşuluyla tanımlıdır.

### Eklenti geliştirmesi (isteğe bağlı)

`eklenti/.vscode/launch.json` içinde **kalıcı** `extensionHost` yapılandırması **olmasın** — aksi halde F5 eklenti kaynağını (`tsc`) derler, kullanıcı kodunu değil.

Eklentinin kendisini debug etmek için geçici olarak:

```json
{
  "name": "Eklentiyi başlat",
  "type": "extensionHost",
  "request": "launch",
  "args": ["--extensionDevelopmentPath=${workspaceFolder}"],
  "outFiles": ["${workspaceFolder}/out/**/*.js"],
  "preLaunchTask": "npm: compile"
}
```

---

## Sorun giderme

| Belirti | Çözüm |
|---|---|
| `Python bulunamadı` | `upp.pythonYolu` ayarla; Store alias’ını kapat |
| `u++ kökünü açın` | Workspace kökünde `uppc.py` olmalı |
| Tanı gelmiyor | Çıktı kanalı: **u++ Dil Sunucusu**; `upp.iz.sunucu: verbose` |
| Native derleyici yok | `upp.derleyici: python` veya `derleme/uppc.exe` üret |
| GCC hatası | MinGW-w64 PATH’te mi? `gcc --version` |
| WinError 4551 (exe) | Windows Uygulama Denetimi; Python derleyici kullanın |

---

## Anahtar sözcükler (GitHub / arama)

`u++` · `uplusplus` · `Türkçe programlama dili` · `Turkish programming language` · `transpiler` · `GCC` · `MinGW` · `VS Code extension` · `Cursor extension` · `Language Server Protocol` · `LSP` · `syntax highlighting` · `code completion` · `C11` · `self-hosting compiler` · `uppc` · `upp` · `programlama dili`

---

## Lisans ve katkı

Bu eklenti [u++](../README.md) deposunun parçasıdır. Sorun ve öneriler için depo issue’larını kullanın.

**İlgili dosyalar:** `src/extension.ts` · `src/derleme.ts` · `server/upp_lsp.py` · `server/analiz.py` · `syntaxes/upp.tmLanguage.json` · `snippets/upp.code-snippets`
