# u++ Standard Library and Public API Reference (v3.0)

This document is the official technical reference defining the standard library (`upp.*`), built-in types, memory ownership semantics, platform-specific APIs, and the native compiler command-line interface for **u++ v3.0**.

For a step-by-step programming tutorial designed for beginners, please refer to [OGREN.md](../OGREN.md). For compiler internal architecture details, see [derleyici.md](../derleyici.md). The Turkish equivalent of this document is available at [API_tr.md](API_tr.md).

---

## Table of Contents

1. [General Contract and Memory Management Rules](#1-general-contract-and-memory-management-rules)
2. [Data Types and Type System](#2-data-types-and-type-system)
3. [Console, Input, and User Interaction](#3-console-input-and-user-interaction)
4. [Conversions and Utility Functions](#4-conversions-and-utility-functions)
5. [String Operations (`upp.metin`)](#5-string-operations-uppmetin)
6. [File System (`upp.dosya`)](#6-file-system-uppdosya)
7. [Path and Directory Operations (`upp.yol`)](#7-path-and-directory-operations-uppyol)
8. [System and Process Management (`upp.sistem`)](#8-system-and-process-management-uppsistem)
9. [Platform-Specific APIs (`upp.windows` and `upp.linux`)](#9-platform-specific-apis-uppwindows-and-upplinux)
10. [Math Library (`upp.matematik`)](#10-math-library-uppmatematik)
11. [Data Structures: `liste` and `harita`](#11-data-structures-liste-and-harita)
12. [JSON Library (`upp.json`)](#12-json-library-uppjson)
13. [Concurrency and Synchronization: `arkaplan` and `Kilit`](#13-concurrency-and-synchronization-arkaplan-and-kilit)
14. [Low-Level Utilities (`guvensiz` Area)](#14-low-level-utilities-guvensiz-area)
15. [LSP and Stdio Framing (`upp.stdio`)](#15-lsp-and-stdio-framing-uppstdio)
16. [Compiler Command-Line Interface (CLI)](#16-compiler-command-line-interface-cli)
17. [C Type and Symbol Mapping Table](#17-c-type-and-symbol-mapping-table)

---

## 1. General Contract and Memory Management Rules

* **Text Format:** All strings are encoded in UTF-8. Expressions such as `s[i]`, `upp.uzunluk(s)`, and `upp.metin.kod(s, i)` operate on Unicode **character code points** rather than raw C bytes. Out-of-bounds indices yield `0`.
* **Memory Ownership (`upp.metin_bosalt`):**
  - Any string dynamically allocated on the heap via `malloc` (e.g., results of `upp.giris()`, `upp.dosya.oku()`, `upp.sistem.calistir()`, runtime string concatenations with `+`, and evaluated string interpolations) must be released with `upp.metin_bosalt(s)` once no longer needed.
  - String literals (`"hello"`), compile-time concatenated constants, `upp.arguman(i)` (which lives for the entire process lifetime), and numeric returns (`kod`, `ileri`) **must not** be deallocated.
* **Safety Boundaries:**
  - In standard (safe) u++ code, raw pointers (`*`, `&`), direct manual memory allocation (`bellek_ayir`, `bellek_bosalt`), inline C code blocks (`c_kod { }`), and kernel process memory access (`upp.windows.bellek.*`, `upp.linux.sayi_oku/yaz`) are strictly prohibited.
  - These operations are exclusively permitted inside **`guvensiz { ... }`** (unsafe) blocks.
* **Error Handling:**
  - A full compilation stops at the first fatal error and exits with a non-zero exit code.
  - Live editor diagnostics (`--analiz`) accumulate up to 50 independent syntax, type, and safety errors without invoking GCC, emitting structured JSON.

---

## 2. Data Types and Type System

| Type Name | C Equivalent | Memory Size | Description |
|---|---|---|---|
| `sayi` | `int64_t` | 8 bytes | Signed 64-bit integer. Default integer type. |
| `ondalik` | `double` | 8 bytes | Double-precision (IEEE 754) 64-bit floating point number. |
| `bayt` | `uint8_t` | 1 byte | Unsigned 8-bit integer (0 to 255). Ideal for raw binary buffers and network streams. |
| `metin` | `char*` | 8 bytes | UTF-8 encoded null-terminated string pointer. |
| `mantik` | `bool` | 1 byte | Boolean value: `dogru` (`true`) or `yanlis` (`false`). |
| `bos` | `void` | 0 bytes | Void return type for functions producing no value. |
| `dizi[T, N]` | Flat C array | `N * sizeof(T)` | Fixed-capacity stack-allocated array (`sayi scores[10];`). |
| `liste[T]` | `upp_liste_t*` | Dynamic | Type-safe dynamically resizable list. |
| `harita[K, V]` | `upp_harita_t*` | Dynamic | Type-safe key-value dictionary (hash map). |
| `sinif` | `struct` | Variable | User-defined object structure. |

---

## 3. Console, Input, and User Interaction

| Function | Parameters | Return Type | Description |
|---|---|---|---|
| `upp.yaz` | `metin s` | `bos` | Writes text to standard output (`stdout`). Supports string interpolation. |
| `upp.satir_yaz` | `metin s` | `bos` | Writes text to `stdout` followed by a newline (`\n`). |
| `upp.hata_yaz` | `metin s` | `bos` | Writes text to standard error (`stderr`). |
| `upp.hata_satir_yaz` | `metin s` | `bos` | Writes text to `stderr` followed by a newline. |
| `upp.giris` | — | `metin` | Reads a line from console input (`stdin`). Requires `metin_bosalt`. |

---

## 4. Conversions and Utility Functions

| Function | Parameters | Return Type | Description |
|---|---|---|---|
| `upp.uzunluk` | `metin` or `dizi[]` | `sayi` | Returns character count for strings or element count for arrays. |
| `upp.metinden_sayiya` | `metin s` | `sayi` | Converts string to 64-bit integer (`atoll`). |
| `upp.sayidan_metin` | `sayi n` / `bayt b` | `metin` | Converts integer/byte to string (requires `metin_bosalt`). |
| `upp.ondalikdan_metin` | `ondalik d` | `metin` | Converts float to string (requires `metin_bosalt`). |
| `upp.metinden_ondalik` | `metin s` | `ondalik` | Converts string to float (`atof`). |
| `upp.min` | `sayi a, sayi b` | `sayi` | Returns the smaller of two numbers. |
| `upp.max` | `sayi a, sayi b` | `sayi` | Returns the larger of two numbers. |
| `upp.mutlak` | `sayi n` | `sayi` | Returns absolute value of the number. |
| `upp.zaman` | — | `sayi` | Monotonic time in milliseconds elapsed since process startup. |
| `upp.uyut` | `sayi milisaniye` | `bos` | Suspends execution for the specified milliseconds. |
| `upp.rastgele` | `sayi min, sayi max` | `sayi` | Generates a random 64-bit integer in the inclusive range `[min, max]`. |
| `upp.platform` | — | `metin` | Returns host OS name: `"windows"` or `"linux"`. |
| `upp.metin_bosalt` | `metin s` | `bos` | Frees a dynamically allocated string from the heap. |

---

## 5. String Operations (`upp.metin`)

| Function | Parameters | Return Type | Description |
|---|---|---|---|
| `upp.metin.kes` | `metin s, sayi bas, sayi son` | `metin` | Slices string by Unicode character indices `[bas, son)`. |
| `upp.metin.buyuk` | `metin s` | `metin` | Converts string to uppercase (supports Turkish İ/I). |
| `upp.metin.kucuk` | `metin s` | `metin` | Converts string to lowercase (supports Turkish ı/i). |
| `upp.metin.kirp` | `metin s` | `metin` | Trims leading and trailing whitespace. |
| `upp.metin.baslangic_mi` | `metin s, metin on` | `mantik` | Returns `dogru` if string starts with prefix. |
| `upp.metin.bitis_mi` | `metin s, metin son` | `mantik` | Returns `dogru` if string ends with suffix. |
| `upp.metin.bul` | `metin s, metin aranan` | `sayi` | Returns first index of substring; `-1` if not found. |
| `upp.metin.degistir` | `metin s, metin eski, metin yeni` | `metin` | Replaces all occurrences of substring. |
| `upp.metin.bol` | `metin s, metin ayirici` | `liste[metin]` | Splits string by delimiter and returns list of tokens. |
| `upp.metin.kod` | `metin s, sayi i` | `sayi` | Returns Unicode codepoint at character index `i`. |
| `upp.metin.ileri` | `metin s, sayi cp_i` | `sayi` | Advances codepoint iterator to next character position. |
| `upp.metin.hex_coz` | `metin hex_str` | `metin` | Decodes hex string into binary/text buffer. |

---

## 6. File System (`upp.dosya`)

| Function | Parameters | Return Type | Description |
|---|---|---|---|
| `upp.dosya.oku` | `metin yol` | `metin` | Reads entire file content as string (requires `metin_bosalt`). |
| `upp.dosya.yaz` | `metin yol, metin icerik` | `mantik` | Writes text to file (overwrites if exists). Returns `dogru` on success. |
| `upp.dosya.ekle` | `metin yol, metin icerik` | `mantik` | Appends text to end of file. |
| `upp.dosya.var_mi` | `metin yol` | `mantik` | Returns `dogru` if file exists on disk. |
| `upp.dosya.sil` | `metin yol` | `mantik` | Deletes file from disk. |
| `upp.dosya.boyut` | `metin yol` | `sayi` | Returns file size in bytes (`-1` if not found). |

---

## 7. Path and Directory Operations (`upp.yol`)

| Function | Parameters | Return Type | Description |
|---|---|---|---|
| `upp.yol.birlestir` | `metin a, metin b` | `metin` | Joins two path segments using host platform separator. |
| `upp.yol.dizin` | `metin yol` | `metin` | Returns parent directory of the path. |
| `upp.yol.dosya_adi` | `metin yol` | `metin` | Returns file name including extension. |
| `upp.yol.uzanti` | `metin yol` | `metin` | Returns file extension (`.txt`, `.upp`). |
| `upp.yol.dizin_var_mi` | `metin yol` | `mantik` | Returns `dogru` if directory exists. |
| `upp.yol.dizin_olustur` | `metin yol` | `mantik` | Creates directory. |
| `upp.yol.listele` | `metin dizin` | `liste[metin]` | Lists entries inside directory. |

---

## 8. System and Process Management (`upp.sistem`)

```text
sinif SurecCikti {
    sayi kod;
    metin cikti;
    mantik basarili;
}
```

| Function | Parameters | Return Type | Description |
|---|---|---|---|
| `upp.sistem.calistir` | `metin komut` | `SurecCikti` | Executes shell command; returns exit code, stdout/stderr, and success status. |
| `upp.sistem.ortam_al` | `metin anahtar` | `metin` | Retrieves environment variable value (`""` if unset). |
| `upp.sistem.ortam_yaz` | `metin anahtar, metin deger`| `mantik` | Sets environment variable. Returns `dogru` on success. |
| `upp.sistem.pid` | — | `sayi` | Returns process identifier (PID) of current process. |
| `upp.sistem.cikis` | `sayi kod` | `bos` | Exits process immediately with given return code. |
| `upp.arguman_sayisi` | — | `sayi` | Returns total CLI arguments passed to process. |
| `upp.arguman` | `sayi i` | `metin` | Returns CLI argument at index `i` (`0` is executable path). |

---

## 9. Platform-Specific APIs (`upp.windows` and `upp.linux`)

Platform-specific functionalities are isolated into dedicated namespaces.

### Windows API (`upp.windows.*`)

Windows-specific operating system, memory manipulation, and GUI utilities.

#### Process Memory Access (`upp.windows.bellek.*`)
> ⚠️ **Safety Enforcement:** All `upp.windows.bellek.*` functions perform raw virtual memory operations on processes and **strictly require being enclosed in a `guvensiz { ... }` block.**

* `baglan(metin surec_adi) -> sayi`: Opens handle to target process by name.
* `kapat(sayi tutamak) -> bos`: Closes target process handle.
* `modul_bul(metin modul_adi) -> sayi`: Finds base address of module or DLL in target process.
* `sayioku(sayi adres) -> sayi`: Reads 64-bit integer from memory.
* `sayiyaz(sayi adres, sayi deger) -> mantik`: Writes 64-bit integer to memory.
* `ondalikoku(sayi adres) -> ondalik`: Reads double float from memory.
* `ondalikyaz(sayi adres, ondalik deger) -> mantik`: Writes double float to memory.
* `metinoku(sayi adres, sayi uzunluk) -> metin`: Reads raw byte string from memory.
* `zincir_oku(sayi taban, liste[sayi] ofsetler) -> sayi`: Follows multilevel pointer chain.
* `desen_tara(metin modul, metin desen, metin maske) -> sayi`: Scans memory for byte pattern (AOB pattern).
* `yama_yap(sayi adres, metin hex_baytlar) -> mantik`: Patches target memory with hex machine code.
* `koruma_degistir(sayi adres, sayi boyut, metin bayrak) -> mantik`: Modifies memory page protection.
* `son_hata() -> sayi`: Returns `GetLastError()` code.
* `okundu() -> sayi`: Returns bytes successfully read/written in last memory call.

#### User Interface and Input (`upp.windows.*`)
* `upp.windows.mesaj(metin baslik, metin govde) -> bos`: Shows Win32 information message dialog.
* `upp.windows.hata(metin baslik, metin govde) -> bos`: Shows Win32 error message dialog.
* `upp.windows.ses_cal(metin wav_yolu) -> bos`: Plays WAV audio asynchronously.
* `upp.windows.girdi`: `fare_tasi(x, y)`, `fare_tikla(tus)`, `tus_bas(kod)`, `tus_birak(kod)`, `tus_durumu(kod) -> mantik`.
* `upp.windows.cizim`: `baslat(pencere)`, `cizgi(...)`, `kutu(...)`, `kutu_dolu(...)`, `metin(...)`, `temizle()`, `bitir()`.

---

### Linux API (`upp.linux.*`)

Linux (POSIX) system utilities and low-level process inspection.

* `upp.linux.pid() -> sayi`: Returns PID of current process (`getpid`).
* `upp.linux.sinyal_gonder(sayi pid, sayi sinyal) -> sayi`: Sends POSIX signal (`SIGTERM`, `SIGKILL`, etc.) to target process.
* `upp.linux.proc_oku(sayi pid, metin dosya) -> metin`: Reads `/proc/<pid>/<file>` entry (`status`, `cmdline`, etc.).
* `upp.linux.sayi_oku(sayi pid, sayi adres) -> sayi`: Reads 64-bit integer via `process_vm_readv`. *(Strictly requires `guvensiz` block)*
* `upp.linux.sayi_yaz(sayi pid, sayi adres, sayi deger) -> mantik`: Writes 64-bit integer via `process_vm_writev`. *(Strictly requires `guvensiz` block)*

---

## 10. Math Library (`upp.matematik`)

| Function | Parameters | Return Type | Description |
|---|---|---|---|
| `upp.matematik.karekok` | `ondalik x` | `ondalik` | Square root (`sqrt`). |
| `upp.matematik.us` | `ondalik taban, ondalik us` | `ondalik` | Exponentiation (`pow`). |
| `upp.matematik.sinus` | `ondalik radyan` | `ondalik` | Sine function (`sin`). |
| `upp.matematik.kosinus` | `ondalik radyan` | `ondalik` | Cosine function (`cos`). |
| `upp.matematik.tanjant` | `ondalik radyan` | `ondalik` | Tangent function (`tan`). |
| `upp.matematik.taban` | `ondalik x` | `sayi` | Floor rounding (`floor`). |
| `upp.matematik.tavan` | `ondalik x` | `sayi` | Ceil rounding (`ceil`). |
| `upp.matematik.yuvarla` | `ondalik x` | `sayi` | Nearest integer rounding (`round`). |
| `upp.matematik.pi` | — | `ondalik` | Pi constant ($3.141592653589793$). |
| `upp.matematik.e` | — | `ondalik` | Euler constant ($2.718281828459045$). |

---

## 11. Data Structures: `liste` and `harita`

### `liste[T]` (Dynamic List)

| Method | Signature | Description |
|---|---|---|
| `.ekle(eleman)` | `(T) -> bos` | Appends item to the end of the list. |
| `.al(indeks)` | `(sayi) -> T` | Returns element at index. Panics on bounds error. |
| `.yaz(indeks, deger)` | `(sayi, T) -> bos` | Overwrites element at index. |
| `.sil(indeks)` | `(sayi) -> bos` | Removes element at index and shifts subsequent elements. |
| `.temizle()` | `() -> bos` | Clears all elements, resetting length to `0`. |
| `.uzunluk()` | `() -> sayi` | Returns number of elements currently stored. |
| `.bosalt()` | `() -> bos` | Deallocates list memory. |

### `harita[K, V]` (Hash Map)

| Method | Signature | Description |
|---|---|---|
| `.koy(anahtar, deger)` | `(K, V) -> bos` | Inserts or updates key-value pair. |
| `.al(anahtar)` | `(K) -> V` | Retrieves value associated with key. |
| `.var_mi(anahtar)` | `(K) -> mantik` | Checks if key exists in map. |
| `.sil(anahtar)` | `(K) -> bos` | Removes key and its associated value from map. |
| `.temizle()` | `() -> bos` | Clears all entries from map. |
| `.uzunluk()` | `() -> sayi` | Returns number of key-value pairs stored. |
| `.anahtarlar()` | `() -> liste[K]` | Returns list of all keys in insertion order. |
| `.bosalt()` | `() -> bos` | Deallocates map memory. |

---

## 12. JSON Library (`upp.json`)

| Function | Parameters | Return Type | Description |
|---|---|---|---|
| `upp.json.ayristir` | `metin json_metni` | `JSONDeger` | Parses JSON string. Returns handle with `id = 0` on failure. |
| `upp.json.anahtar_al` | `JSONDeger d, metin anahtar` | `JSONDeger` | Returns node handle for object property. |
| `upp.json.indeks_al` | `JSONDeger d, sayi indeks` | `JSONDeger` | Returns node handle for array element. |
| `upp.json.metin_al` | `JSONDeger d` | `metin` | Retrieves string value from node (requires `metin_bosalt`). |
| `upp.json.sayi_al` | `JSONDeger d` | `sayi` | Retrieves integer value from node. |
| `upp.json.ondalik_al` | `JSONDeger d` | `ondalik` | Retrieves float value from node. |
| `upp.json.mantik_al` | `JSONDeger d` | `mantik` | Retrieves boolean value from node. |
| `upp.json.uzunluk` | `JSONDeger d` | `sayi` | Returns count of items in JSON object or array. |
| `upp.json.tur` | `JSONDeger d` | `metin` | Returns node type: `"nesne"`, `"dizi"`, `"metin"`, `"sayi"`, `"mantik"`, `"bosluk"`. |
| `upp.json.var_mi` | `JSONDeger d` | `mantik` | Validates handle validity. |
| `upp.json.bosalt` | `JSONDeger kok` | `bos` | Releases entire parsed JSON tree. |

---

## 13. Concurrency and Synchronization: `arkaplan` and `Kilit`

### `arkaplan` and `ArkaplanIs`

```text
oto is = arkaplan {
    upp.satir_yaz("Running in background thread");
};
is.bekle(); // Awaits thread termination
```

* **Captures:** Primitive values (`sayi`, `bayt`, `metin`) are copied into the thread.
* **Restrictions:** Collections (`liste`, `harita`) and `JSONDeger` handles cannot be captured due to data-race safety enforcement.

### `Kilit` (Mutex)

| Method | Signature | Description |
|---|---|---|
| `.kilitle()` | `() -> bos` | Acquires mutex lock, blocking until available. |
| `.birak()` | `() -> bos` | Releases mutex lock. |

---

## 14. Low-Level Utilities (`guvensiz` Area)

Operations strictly gated behind `guvensiz { ... }` blocks:
1. Raw pointer declarations and address-of operators: `sayi* p`, `&x`.
2. Raw heap allocations: `bellek_ayir(size)`, `bellek_bosalt(address)`.
3. Inline GNU C code injection: `c_kod { printf("Raw C\n"); }`.
4. Remote process memory access: `upp.windows.bellek.*` and `upp.linux.sayi_oku / sayi_yaz`.

---

## 15. LSP and Stdio Framing (`upp.stdio`)

Framing layer for Language Server Protocol (LSP) and JSON-RPC (`Content-Length: N\r\n\r\n`).

* `upp.stdio.gelen_oku() -> metin`: Reads header and payload, returning JSON body (requires `metin_bosalt`).
* `upp.stdio.yaz(metin json_metin) -> bos`: Emits JSON text preceded by standard `Content-Length` header to `stdout`.

---

## 16. Compiler Command-Line Interface (CLI)

The u++ compiler (`uppc` / `uppc.exe`) is a standalone native binary with the C runtime library embedded inside it.

```bash
uppc <source.upp> [flags]
```

### Flags

| Flag | Alternative | Description |
|---|---|---|
| `-o <NAME>` | `--cikti <NAME>` | Sets output filename stem (`<NAME>.c` and `<NAME>.exe` / `<NAME>.out`). |
| `-c` | `--sadece-c` | Emits C code only; skips GCC binary compilation. |
| `--sadece-derle` | — | Compiles binary without executing it. |
| `--runtime <PATH>` | — | Overrides embedded runtime with custom `upp_runtime.c` file. |
| `-v` | `--surum` | Displays compiler version (`u++ v3.0 (beta)`). |
| `-h` | `--yardim` | Displays CLI usage and help message. |
| `--linux` | — | Sets target ABI to Linux (POSIX). |
| `--windows` | — | Sets target ABI to Windows (Win32). |
| `--analiz` | — | Real-time editor analysis mode. Emits up to 50 errors in JSON. |
| `--json-hata` | — | Full compile pass emitting results in single-line JSON. |
| `--bicim` | — | Formats source file in-place with standard 4-space indentation. |
| `--bicim-kontrol` | — | Checks formatting; exits with code `1` if unformatted (for CI). |
| `--ast` | — | Dumps AST hierarchy to console. |

---

## 17. C Type and Symbol Mapping Table

| u++ Symbol | Generated C Symbol |
|---|---|
| `sayi` | `int64_t` |
| `ondalik` | `double` |
| `bayt` | `uint8_t` |
| `metin` | `char*` |
| `mantik` | `bool` |
| `bos` | `void` |
| `liste[T]` | `upp_liste_t*` |
| `harita[K, V]` | `upp_harita_t*` |
| `upp.yaz(...)` | `upp_yaz(...)` |
| `upp.windows.bellek.sayioku(...)` | `upp_windows_bellek_sayioku(...)` |
| `upp.linux.sayi_oku(...)` | `upp_linux_sayi_oku(...)` |
| `upp.sistem.pid()` | `upp_sistem_pid()` |
