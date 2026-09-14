# u++ native derleyici (self-hosting)

Python [`uppc.py`](uppc.py) **silinmez**; native derleyici bitip kendini derleyene kadar birincil bootstrap ve güvenlik ağıdır.

Kaynak: [`src/uppc/`](src/uppc/). Çalışma zamanı hâlâ [`src/runtime/upp_runtime.c`](src/runtime/upp_runtime.c).

Derleyici kaynağı `c_kod` / `c_ekle` kullanmaz (SoA `liste`/`harita` + `sayi` id). Dilin kullanıcı kaçışı (`guvensiz` / `c_kod`) durur.

## Mimari

Lexer → Parser → Tip çıkarımı → Güvenlik → C üreteci → GCC.

Modüller birleştirilerek tek `fonk ana` programı olur (u++ modül sistemi yoktur).

| Dosya | Görev |
|---|---|
| `havuz.upp` | SoA havuz: düğüm, liste arenası, jeton, intern, kapsam, tablo, emit, OS |
| `hata.upp` | ilk-hata-ölümcül + `--analiz` toplama (metin tamponu) |
| `ast.upp` | `JetonTur` / `DugumTur` |
| `lexer.upp` | tarayıcı (kod noktası imleci; `c_kod` gömme kullanıcı dili) |
| `parser.upp` | özyinelemeli iniş |
| `tur.upp` | tip çıkarımı |
| `guvenlik.upp` | `guvensiz` / `guvenli` kapısı |
| `codegen.upp` | C11 + `emit` tamponu |
| `surucu.upp` | CLI |
| `birlestir.upp` | Python'sız birleştirici (ayrı program) |

AST: düğüm id `0` = yok. Geçersiz id `[u++ HATA]` basar, segfault yok.

Havuz sütunları üst düzey `liste[sayi]` / `liste[metin]` / `harita[metin,sayi]`. İşaretçi yok; API yalnızca `sayi` id ve `metin` döner. `guvensiz` içinde liste/düğüm deposuna işaretçi alıp sonra `ekle` / `dugum_yeni` deyip aynı işaretçiyi kullanmak tanımsızdır.

Codegen: `emit` / `tampon_*` `metin +=` ile büyür (bootstrap hız hedefi değil).

Intern: `harita` içerik → id + `liste[metin]` id → metin. Sembol tabloları dört `harita` (4096 tavanı yok). Kapsam: ad/tip/seviye sütunları.

Lexer imleci kod noktası indeksidir (`lex_pos` = `cp_i`). Dilim `upp.metin.kes` (kod noktası aralığı). `upp.metin.kod` / `ileri` yorum ve `@upp-kaynak` taramasında kullanılır.

## Bootstrap

```powershell
python araclar/birlestir.py
python uppc.py derleme/uppc_birlesik.upp --sadece-derle --cikti derleme/uppc

.\derleme\uppc.exe --havuz-test
.\derleme\uppc.exe ornekler\ornek1.upp --sadece-derle --cikti derleme\ornek1
.\derleme\uppc.exe ornekler\ornek2.upp --sadece-derle --cikti derleme\ornek2

.\derleme\uppc.exe derleme\uppc_birlesik.upp --sadece-derle --cikti derleme\uppc1
.\derleme\uppc1.exe derleme\uppc_birlesik.upp --sadece-derle --cikti derleme\uppc2
```

Birleşik dosyadaki `// @upp-kaynak <modül>` işaretleri lexer'ın dosya adını günceller ve satır sayacını o modülün başına sıfırlar. Native derleyici hem Windows hem Linux host'ta derlenir; bayrak yoksa hedef host ABI'sidir.

## CLI

Python ile aynı: kaynak, `--cikti`, `--sadece-c`, `--sadece-derle`, `--ast`, `--analiz`, `--json-hata`, `--linux`, `--windows`. Ek: `--havuz-test`. Bayrak yoksa hedef, derleyicinin çalıştığı işletim sistemidir. `--linux` POSIX C ve `.out` üretir; `--windows` Win32 C ve `.exe`. `--sadece-derle` yoksa yalnızca hedef host ile aynıysa ikili çalıştırılır.

Native `--analiz` / `--json-hata` JSON şekli Python ile aynıdır (`ok`, `hatalar[].satir|sutun|tur|mesaj`). `.uph` başlık `kullan` ile katılır; kök derleme girişi olamaz. `--analiz` bir `.uph` için `fonk ana` istemez.

VS Code eklentisi (`eklenti/`) `upp.derleyici` = `native` iken `--analiz` bayrağını kullanır. Tam LSP (tamamlama/hover) hâlâ Python `uppc.py` AST’sinden gelir.

Native JSON-RPC dil sunucusu (`upp.stdio`) ayrı iş; bu derleyicinin v1’i `--analiz` sözleşmesidir.
