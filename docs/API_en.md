# u++ Standard Library and API Reference (v3.1 beta)

This document defines the `upp.*` builtins, types, memory model, targets, and the compiler command line. The single source of truth for builtins is the table in [`src/uppc/yerlesik.upp`](../src/uppc/yerlesik.upp). The type checker reads argument counts, argument types, return types, the `guvensiz` requirement, and target availability from it, and this document mirrors it exactly.

Tutorial: [OGREN.md](../OGREN.md) · Compiler internals: [derleyici.md](../derleyici.md) · Türkçe: [API_tr.md](API_tr.md)

---

## 1. Memory Model

### String ownership (automatic drop)
- Every `metin` is a managed object. Its header holds an atomic reference count, the byte length, the capacity, and code point information. The data stays a NUL-terminated, C-compatible `char*`.
- Copies are O(1) (a refcount increment). `+=` grows a uniquely owned string in place and does copy-on-write when the string is shared.
- The compiler releases strings at scope exit, at `dur`/`devam`, and at `don`. There is no manual free.
- `upp.uzunluk(s)` is O(1), and so is `s[i]` on ASCII strings. Non-ASCII strings carry their own code-point index.
- String literals are immortal objects whose headers are filled in at compile time.

### Owned classes
For classes with `metin` fields (or with fields of owned classes), the compiler generates `T__birak` and `T__kopya`:
- Copying from a variable bumps refcounts. Temporaries (constructors, call results) are moved.
- Assignment releases the old value. Parameters are owned on entry, and `don` moves the value out.
- Locals are released at scope exit.

### Views: `&metin`, `&dilim[T]`
Zero-copy, read-only borrows: `fonk yaz(&metin v)`, `&dilim[sayi] d = arr[1..4]`. Their fields are `v.uzunluk` (UTF-8 bytes for strings) and `v.veri` (`&metin` only). A view cannot be returned, stored in a class field or top-level variable, or captured by `arkaplan`.

### Collections
`liste[T]` and `harita[K,V]` are value types: copies are O(1) (a refcount increment) and a shared copy is split on write (copy-on-write). After `M = L`, `M.ekle(x)` does not change `L`, and a list passed as a parameter does not change the caller's list. The compiler releases collections at scope exit too; `.bosalt()` only returns memory early and leaves the collection empty.

The element type can be any value type except pointers and views: numeric types, `metin`, `sinif`, `liste[...]`, `harita[...]`, `fonk(...)`. Map keys are `metin` or `sayi`. `L[i][j] = x`, `K[0].ad += "!"`, and `K[1].yaslan()` work in place.

`JSONDeger` is a handle; close it with `upp.json.bosalt()`.

### Function values
`fonk(P1, P2) -> R` is a type (`-> R` defaults to `bos`). Top-level functions become values by name and can be stored in variables, parameters, return values, class fields, and collections:

```upp
fonk ikiye(sayi x) -> sayi => x * 2;
fonk uygula(fonk(sayi) -> sayi f, sayi x) -> sayi => f(x);

fonk(sayi) -> sayi f = ikiye;
upp.satir_yaz("{uygula(f, 5)}");
harita[metin, fonk(sayi, sayi) -> sayi] komutlar;
komutlar.koy("carp", fonk(sayi a, sayi b) -> sayi => a * b);
upp.satir_yaz("{komutlar.al(\"carp\")(3, 4)}");
```

- Anonymous functions: `fonk(sayi x) -> sayi => x * 2` or `fonk(...) -> T { ... }`. There are no captures; touching an outer local is a compile error, so pass the value as a parameter.
- An unassigned function value is `yok`. Test it with `f == yok`; calling `yok` stops the program with an explicit panic.
- Signatures must match exactly. Only `==` and `!=` work on function values. Methods and `ana` cannot be taken as values.
- In C it is a plain function pointer, so it also works on the freestanding and kernel targets (interrupt/command handler tables).

### Integer division
`/` and `%` panic explicitly on division by zero and on `MIN / -1` overflow (in the kernel too). A constant zero divisor is a compile error.

### `guvensiz`
The following are only allowed inside `guvensiz { }`: pointers (`*`, `&`), `bellek_ayir` / `bellek_bosalt`, `c_kod { }`, process-memory access, and hardware access (`upp.cekirdek.*`). If `c_kod` stores a raw C pointer into a `metin` variable, the runtime never frees it. To store a managed value, use `upp_metin_ata(&s, UPP_LIT("..."))`.

---

## 2. Types

| Type | C | Notes |
|---|---|---|
| `sayi` / `s64` | `long long` | Signed 64-bit; type of unsuffixed integer literals |
| `u64` / `usayi` | `unsigned long long` | `5u64` |
| `s32` / `u32`, `s16` / `u16` | `int32_t`… | `7u32` |
| `bayt` / `s8`, `ubayt` / `u8` | `int8_t` / `uint8_t` | Constant overflow is a compile error |
| `ondalik` | `double` | IEEE 754 |
| `metin` | managed `char*` | UTF-8 |
| `mantik` | `bool` | `dogru` / `yanlis` |
| `bos` | `void` | |
| `dizi[T, N]` | C array | Bounds-checked |
| `liste[T]`, `harita[K,V]` | value (refcount + COW) | |
| `fonk(P...) -> R` | function pointer | Function value, may be `yok` |
| `&metin`, `&dilim[T]` | view | |
| `sinif`, `secenek` | `struct`, `enum` | |

---

## 3. Interpolation and Output

- `"... {expr} ..."`: a full expression starts when `{` is immediately followed by a letter, `_`, or `(`, for example `"{a[0]}"`, `"{f(x)}"`, `"{a + b * 2}"`, `"{k.ad}"`. An expression that fails to parse is a compile error.
- `{{` and `}}` mean `{` and `}` in every string. A `{` followed by a space or a digit stays literal.
- String escapes: `\n \t \r \\ \" \e` (ESC) and `\xHH`.
- `upp.yaz(x)` / `upp.satir_yaz(x)` (stdout; serial + VGA on freestanding targets) and `upp.hata_yaz(x)` / `upp.hata_satir_yaz(x)` (stderr) accept any printable type: integers, `ondalik`, `mantik`, `metin`, `&metin`, and enums. They take at most one argument. There are no printf format strings.
- `upp.giris() -> metin` reads one line from stdin.

## 4. General (`upp.*`)

`uzunluk`, `metinden_sayiya`, `sayidan_metin`, `ondalikdan_metin`, `metinden_ondalik`, `min`, `max`, `mutlak`, `zaman`, `uyut`, `rastgele(a, b)`, `platform`, `arguman_sayisi`, `arguman(i)`, `mesaj`, `hata`, `ses_cal`.

**Removed names** now produce a compile error that names the replacement:
- `upp.metin_bosalt`
- `upp.dosya_*` → `upp.dosya.*`
- `upp.ortam_al/yaz` → `upp.sistem.*`
- `upp.metin.icinde` → `upp.metin.bul`
- `upp.yol.klasor_olustur` → `upp.yol.dizin_olustur`
- `upp.matematik.sin/cos/tan` → `sinus/kosinus/tanjant`
- `upp.json.dizi_eleman` → `upp.json.indeks_al`
- the `upp.bellek/girdi/cizim` shortcuts → `upp.windows.*`

## 5. `upp.metin`

- `kes(s, start, count)` (counted in code points), `buyuk`, `kucuk` (Turkish casing: i↔İ, ı↔I), `kirp`
- `baslar`, `biter`
- `bul(s, needle) -> sayi` (returns -1 if absent), `degistir`
- `bol(s, sep) -> liste[metin]`, `birlestir(list, sep)`
- `kod(s, i)`, `ileri(s, i)`
- `hex_coz`, `hex_kodla`

## 6–8. `upp.dosya`, `upp.yol`, `upp.sistem`, `upp.stdio` (hosted)

- **`dosya`**: `oku`, `yaz`, `ekle`, `var_mi`, `sil`, `boyut` (-1 if missing), `listele`
- **`yol`**: `birlestir`, `dizin`, `dosya_adi`, `uzanti`, `mutlak_mi`, `gercek`, `calisma`, `dizin_var_mi`, `dizin_olustur` (recursive)
- **`sistem`**:
  - `calistir(cmd) -> SurecCikti { kod, cikti, basarili }` captures stdout and stderr.
  - `kos(cmd) -> sayi` runs the command with the inherited console (interactive) and returns its exit code.
  - `komut_bul`, `exe_dizin`, `pid`, `ortam_al`, `ortam_yaz`, `cikis(kod)`
- **`stdio`**: `gelen_oku()`, `yaz(json)` (LSP framing)

## 9. Platform APIs

- **`upp.linux.*`** (`--hedef linux` only): `pid`, `sinyal_gonder`, `proc_oku`. Inside `guvensiz`: `sayi_oku`, `sayi_yaz`.
- **`upp.windows.*`** (Windows only): `mesaj`, `hata`, `ses_cal`, `girdi.*`, `cizim.*`.
- **`upp.windows.bellek.*`** (`guvensiz`): `baglan`, `modul_bul`, `sayioku`, `sayiyaz`, `oku_ondalik`, `yaz_ondalik`, `oku_metin`, `zincir_oku`, `yama_yap`, `koruma_degistir`, `son_hata`, `okundu`.

## 10. `upp.matematik`

- `karekok`, `us`, `sinus`, `kosinus`, `tanjant`, `atan2`
- `taban`, `tavan`, `yuvarla` → `sayi`
- `mutlak`, `sinirla`, `yumusat`, `pi()`, `e()` (also available on freestanding targets)
- `mesafe`, `mesafe_3d`, `aci_hesapla`, `dunya_ekran`

## 11–13. Collections, JSON, threads

- **`liste`**: `ekle`, `al`, `yaz`, `sil`, `temizle`, `uzunluk`, `bosalt`; `L[i]` reads and writes (bounds-checked), slicing with `L[a..b]`, `her (oto x in L)` iterates.
- **`harita`**: `koy`, `al` (zero value of the type when the key is missing), `var_mi`, `sil`, `temizle`, `uzunluk`, `anahtarlar` (insertion order), `bosalt`; `her (oto k in h)` iterates keys in insertion order.
- **`upp.json`** (hosted): `ayristir`, `olustur`, `tur`, `anahtar_al`, `indeks_al`, `anahtarlar`, `metin_al` / `sayi_al` / `ondalik_al` / `mantik_al`, `uzunluk`, `var_mi`, `nesne`, `dizi`, `*_yap`, `koy`, `ekle`, `bosalt`.
- **`arkaplan { }` / `.bekle()` and `Kilit`** (hosted). Strings are deep-copied into the thread.

## 14. `upp.cekirdek` — hardware (`--hedef serbest|cekirdek` only, `guvensiz`)

- `port_yaz8/16/32`, `port_oku8/16/32`: x86 I/O ports
- `bellek_yaz8..64`, `bellek_oku8..64`: volatile MMIO
- `dur` (`hlt`), `kesme_kapat` (`cli`), `kesme_ac` (`sti`)
- `yigin_kur(adres, boyut)`: hands a memory region to the string/collection allocator (the default is a 4 MiB `.bss` arena)
- `kesmeler_kur()`: sets up the IDT, PIC, and a 1000 Hz PIT (called automatically at boot)
- `tik()`: timer ticks since boot (1 ms)
- `tus_oku()` / `tus_bekle()`: read a code point from the PS/2 keyboard (Turkish Q layout) or the serial port

On the kernel target, `upp.giris()` (line input from keyboard/serial with echo and backspace), `upp.zaman()`, and `upp.uyut(ms)` also work. CPU exceptions panic with the exception name and RIP, and a panic exits QEMU with code 3.

## 15. Targets

| Target | Output | Notes |
|---|---|---|
| (default) | `.exe` / `.out` | Host OS |
| `--hedef windows` / `linux` | | Cross ABI |
| `--hedef serbest` | ELF `.o` | No libc; entry point is `upp_cekirdek_giris()` |
| `--hedef cekirdek` | `name.elf` + `name32.elf` | Multiboot x86_64 kernel; output goes to COM1 + VGA; runs QEMU if it is available |

On freestanding targets, hosted builtins, `arkaplan`, and `Kilit` are compile errors. Toolchain: `gcc` on Linux, and `x86_64-elf-gcc` or `gcc` inside WSL on Windows. `qemu-system-x86_64` runs the kernel.

## 16. Command Line

```text
uppc <file.upp> [-o name] [--sadece-derle | -c] [--hedef t] [--analiz] [--json-hata] [--bicim | --bicim-kontrol] [--ast] [--runtime path]
uppc yeni <name> [--hedef cekirdek]
uppc derle [dir] | uppc calistir [dir]
uppc asistan [dir]
uppc kendini-derle [--hizli]
```

`upp.proje` holds one `key = value` per line, with the keys `ad`, `giris`, `hedef` (`yerel|windows|linux|serbest|cekirdek`), `cikti`, and `bayraklar`.
