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

### Error handling: `beklenen`, `?`, `ertele`
`beklenen[T, H]` holds either a `T` value or an `H` error; `beklenen[T]` is short for `beklenen[T, metin]`. In C it is a plain struct of a flag and a `union`: it is returned by value and never allocates behind your back. When the error type is a `secenek`, the heap is never touched.

```upp
secenek DiskHata { YOK, BOZUK, DOLU }

fonk sektor(sayi no) -> beklenen[sayi, DiskHata] {
    eger (no > 100) { don hata(DiskHata.DOLU); }
    don no * 512;                     // a T value: success
}

fonk iki_sektor(sayi a, sayi b) -> beklenen[sayi, DiskHata] {
    sayi x = sektor(a)?;              // on error: clean up and return the same error
    sayi y = sektor(b)?;
    don x + y;
}
```

- **Construction:** in a function returning `beklenen`, `don x;` returns success and `don hata(e);` returns an error. The same works for initializers, assignments, and arguments: `beklenen[sayi] r = 5;`, `r = hata("x");`. A variable without an initializer is success with the zero value.
- **`beklenen[bos, H]`:** carries no value; `don;` or reaching the end of the function is success.
- **Methods:** `tamam_mi()`, `hata_mi()`, `deger()` (panics on error and prints the message when `H` is `metin`), `hata()` (panics on success), `deger_yoksa(v)`.
- **`e?`:** only inside a function that returns `beklenen` with the same `H`. Not allowed in loop conditions, on the right side of `&&`/`||`, or in `?:` branches.
- **No silent discard:** a `beklenen` result thrown away as `f();` is a compile error; propagate it with `?` or check it.
- **Restrictions:** `beklenen` cannot be a class field, cannot be captured by `arkaplan`, and a top-level variable of this type takes no initializer. `hata(...)` cannot be used with `oto`.

An `ertele` body runs in reverse order on every exit from its scope (end of block, `dur`, `devam`, `don`, `?`). `hata_ertele` only runs when the function returns an error (`don hata(...)`, `?`, or returning a `beklenen` that holds an error at run time). The body is `ertele stmt;` or `ertele { ... }`; it cannot leave through `don` or `?`, nor through `dur`/`devam` to an outer loop. The return value is computed before deferred code runs.

```upp
fonk ac(metin ad) -> beklenen[Dosya] {
    oto d = dosya_ac(ad)?;
    hata_ertele d.kapat();            // only if one of the steps below fails
    baslik_oku(d)?;
    don d;
}
```

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
| `beklenen[T, H]`, `beklenen[T]` | struct (flag + union) | Value or error; propagated with `?` |
| `&metin`, `&dilim[T]` | view | |
| `sinif`, `secenek` | `struct`, `enum` | |

### Casts `(T)x` and `boyut(T)`
Same syntax as C: `ubayt b = (ubayt)x;`, `u16 p = (u16)x;`, `sayi n = (sayi)3.9;` (3), `Renk r = (Renk)2;`.

- Free between numeric types, `mantik`, and enums. Integer narrowing truncates like C (`(ubayt)300` is 44).
- `ondalik` → integer panics explicitly when the result does not fit the target range or the value is NaN (undefined behavior in C).
- Pointer ↔ pointer, pointer ↔ integer, and function value ↔ pointer/integer casts are only allowed inside `guvensiz`: `u16* vga = (u16*)0xB8000;`.
- `metin`, classes, collections, and arrays cannot be cast.

`boyut(T)` gives the size of a type in bytes as a `sayi` (C `sizeof`); `boyut(variable)` gives the variable's size (the whole array for arrays). It is a compile-time constant and can be used in `sabit` expressions.

---

## 3. Interpolation and Output

- `"... {expr} ..."`: a full expression starts when `{` is immediately followed by a letter, `_`, or `(`, for example `"{a[0]}"`, `"{f(x)}"`, `"{a + b * 2}"`, `"{k.ad}"`. An expression that fails to parse is a compile error.
- `{{` and `}}` mean `{` and `}` in every string. A `{` followed by a space or a digit stays literal.
- String escapes: `\n \t \r \\ \" \e` (ESC), `\xHH` (a byte), and `\uHHHH` (a Unicode code point, e.g. `\u00e7` = ç). An unknown escape (e.g. `\q`) is a compile error.
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
- `karakter(code) -> metin` (a one-character string from a code point; empty if invalid), `tekrar(s, n) -> metin` (repeat)

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

## 14. `upp.cekirdek` — hardware (`--hedef serbest|cekirdek` only)

These touch hardware directly and require `guvensiz`:

- `port_yaz8/16/32`, `port_oku8/16/32`: x86 I/O ports
- `bellek_yaz8..64`, `bellek_oku8..64`: volatile MMIO
- `dur` (`hlt`), `kesme_kapat` (`cli`), `kesme_ac` (`sti`)
- `yigin_kur(adres, boyut)`: hands a memory region to the string/collection allocator

The rest are callable from safe code:

- `kesmeler_kur()`: sets up the IDT, PIC, and a 1000 Hz PIT (called automatically at boot)
- `tik()`: timer ticks since boot (1 ms)
- `tus_oku()` / `tus_bekle()`: read a code point from the PS/2 keyboard (Turkish Q layout) or the serial port. Arrow/Home/End/Delete keys arrive as `ESC [ A`-style sequences, and Ctrl+letter gives the control code. On serial, CR and CR LF are both one line end. `tus_bekle` lets ready tasks run while it waits.
- `bellek_toplam()`: usable RAM from the Multiboot memory map (bytes); `yigin_boyut()`, `yigin_kullanilan()`: allocator region size and bytes in use
- `gorev_baslat(f, arg, ad) -> sayi`: runs `f(arg)` (`f: fonk(sayi) -> bos`) in a new task and returns its id (PID), or -1 if the table is full
- `sira_ver()`: yield to another ready task; `gorev_kimlik()`: current task id (the main flow is 1)
- `gorev_listesi() -> liste[sayi]`, `gorev_ad(id)`, `gorev_durum(id)` (`"çalışıyor"` / `"hazır"` / `"uyuyor"`, or `""`), `gorev_baslama(id)` (ms), `gorev_oldur(id) -> mantik` (task 1 cannot be killed)
- `tarih() -> metin`: CMOS clock as `"YYYY-MM-DD HH:MM:SS"`
- `kapat()`: ACPI power-off (QEMU, Bochs, VirtualBox; falls back to isa-debug-exit); `yeniden_baslat()`: 8042 reset
- `ekran_temizle()`: clears the VGA screen and the serial terminal; `islemci() -> metin`: CPUID brand string
- `disk_var(d)`, `disk_sektor(d)`, `disk_model(d)`: ATA disk `d` (0–3: primary/secondary channel, master/slave; QEMU `-drive if=ide,index=d`), its sector count (512 bytes), and its model name
- `disk_oku(d, lba, adet, tampon, ofs) -> mantik` / `disk_yaz(...)`: read/write `adet` sectors into/from a buffer (PIO, LBA28/LBA48, cache flush after writes)

**Memory:** at boot, the heap moves to the largest free region in the Multiboot memory map (within the first 1 GiB; about 122 MiB on QEMU's default 128 MiB). Without boot information, a 4 MiB `.bss` arena is used.

**Tasks** are cooperative. Switches happen only in `upp.uyut`, `sira_ver`, and while waiting for input (`tus_bekle`, `upp.giris`), so the runtime's shared state is safe without locks, but a task doing long computations should call `sira_ver()` now and then. Each task has a 64 KiB stack, and there can be at most 32 tasks.

On the kernel target, `upp.giris()` (line input from keyboard/serial with echo and backspace), `upp.zaman()`, and `upp.uyut(ms)` also work. The VGA screen understands ANSI colors (`\e[1;32m`), `\e[2J`, `\e[H`, `\e[K`, `\e[nC`, and `\e[nD`, and it renders box-drawing characters. CPU exceptions panic with the exception name and RIP, and a panic exits QEMU with code 3.

A complete example built on these builtins: [Uppix](../uppix/README.md), a Unix-like operating system written in u++ (ext4 root, FAT32/exFAT).

### `upp.tampon` — byte buffers (all targets)

For handling binary data (disk sectors, filesystem structures, network packets) from safe code. A buffer is referred to by a `sayi` handle, and every access is bounds-checked: an invalid handle or an overflow panics. Multi-byte values are little-endian. Buffers are released by hand (`birak`).

- `yeni(boyut) -> sayi` (zero-filled), `birak(t)`, `boyut(t)`
- `oku8/16/32/64(t, ofs)`, `yaz8/16/32/64(t, ofs, deger)` (32-bit reads are unsigned; 64-bit values are the raw bit pattern)
- `metin(t, ofs, n) -> metin`, `metin_yaz(t, ofs, s) -> sayi`: raw bytes ↔ string (UTF-8 bytes; returns the byte count)
- `utf16_metin(t, ofs, n)`, `utf16_yaz(t, ofs, s, sinir) -> sayi`: UTF-16LE ↔ string (surrogate pairs included; stops at 0x0000; returns code units written)
- `kopyala(hedef, hofs, kaynak, kofs, n)` (overlap-safe), `doldur(t, ofs, n, deger)`
- `crc32c(t, ofs, n, baslangic) -> sayi`: CRC32C (Castagnoli) without the final XOR (the ext4 convention); `^ 4294967295` gives the standard value

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
uppc <file.upp> [-o name] [--sadece-derle | -c] [--hedef t] [--analiz] [--json-hata] [--bicim | --bicim-kontrol] [--ast] [--runtime path] [--disk image]...
uppc yeni <name> [--hedef cekirdek]
uppc derle [dir] | uppc calistir [dir]
uppc asistan [dir]
uppc kendini-derle [--hizli]
```

`upp.proje` holds one `key = value` per line, with the keys `ad`, `giris`, `hedef` (`yerel|windows|linux|serbest|cekirdek`), `cikti`, `bayraklar`, `diskler` (comma-separated disk images for the kernel target, attached as sda, sdb, ...), and `disk_hazirla` (a script run with `sh` in the project directory when a disk image is missing).

`--disk image` (up to 4) attaches a raw disk image to QEMU as `-drive if=ide,format=raw` on the kernel target.
