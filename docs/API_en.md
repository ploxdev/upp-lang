# u++ Standard Library and Public API Reference (v3.0)

This document is the official technical reference defining the standard library (`upp.*`), built-in types, memory ownership semantics, and compiler interfaces for **u++ v3.0**.

For a step-by-step programming tutorial designed for beginners, please refer to [OGREN.md](../OGREN.md). For compiler internal architecture details, see [derleyici.md](../derleyici.md). The Turkish equivalent of this document is available at [API_tr.md](API_tr.md).

---

## Table of Contents

1. [General Contract and Memory Management Rules](#1-general-contract-and-memory-management-rules)
2. [Console, Input, and User Interaction](#2-console-input-and-user-interaction)
3. [Conversions and Utility Functions](#3-conversions-and-utility-functions)
4. [String Operations (`upp.metin`)](#4-string-operations-uppmetin)
5. [File System (`upp.dosya`)](#5-file-system-uppdosya)
6. [Path and Directory Operations (`upp.yol`)](#6-path-and-directory-operations-uppyol)
7. [System and Process Management (`upp.sistem`)](#7-system-and-process-management-uppsistem)
8. [Math Library (`upp.matematik`)](#8-math-library-uppmatematik)
9. [Data Structures: `liste` and `harita`](#9-data-structures-liste-and-harita)
10. [JSON Library (`upp.json`)](#10-json-library-uppjson)
11. [Concurrency and Synchronization: `arkaplan` and `Kilit`](#11-concurrency-and-synchronization-arkaplan-and-kilit)
12. [Low-Level Utilities (`guvensiz` Area)](#12-low-level-utilities-guvensiz-area)
13. [LSP and Stdio Framing (`upp.stdio`)](#13-lsp-and-stdio-framing-uppstdio)
14. [Compiler Command-Line Interface (CLI) and JSON Diagnostic Schema](#14-compiler-command-line-interface-cli-and-json-diagnostic-schema)
15. [C Type and Symbol Mapping Table](#15-c-type-and-symbol-mapping-table)

---

## 1. General Contract and Memory Management Rules

* **Text Format:** All strings are encoded in UTF-8. Expressions such as `s[i]`, `upp.uzunluk(s)`, and `upp.metin.kod(s, i)` operate on Unicode **character code points** rather than raw C bytes. Out-of-bounds indices yield `0`.
* **Memory Ownership (`upp.metin_bosalt`):**
  - Any string dynamically allocated on the heap via `malloc` (e.g., results of `upp.giris()`, `upp.dosya_oku()`, `upp.sistem.calistir()`, runtime string concatenations with `+`, and evaluated string interpolations) must be released with `upp.metin_bosalt(s)` once no longer needed.
  - String literals (`"hello"`), compile-time concatenated constants, `upp.arguman(i)` (which lives for the entire process lifetime), and numeric returns (`kod`, `ileri`) **must not** be deallocated.
* **Safety Boundaries:**
  - In standard (safe) u++ code, raw pointers (`*`, `&`), direct manual memory allocation (`bellek_ayir`, `bellek_bosalt`), inline C code blocks (`c_kod { }`), and foreign process memory access (`upp.bellek.*`) are strictly prohibited.
  - These operations are exclusively permitted inside **`guvensiz { ... }`** (unsafe) blocks.
* **Error Handling:**
  - A full compilation stops at the first fatal error and exits with a non-zero exit code.
  - Live editor diagnostics (`--analiz`) accumulate up to 50 independent syntax, type, and safety errors without invoking GCC, emitting structured JSON.

---

## 2. Console, Input, and User Interaction

### Function Signatures and Descriptions

| Function | Parameters | Return Type | Description |
|---|---|---|---|
| `upp.yaz` | `metin s` | `bos` | Writes text to standard output (`stdout`). Supports string interpolation. |
| `upp.satir_yaz` | `metin s` | `bos` | Writes text to `stdout` followed by a newline (`\n`). |
| `upp.hata_yaz` | `metin s` | `bos` | Writes text to standard error (`stderr`). |
| `upp.hata_satir_yaz` | `metin s` | `bos` | Writes text to `stderr` followed by a newline. |
| `upp.giris` | — | `metin` | Reads a line from console input (`stdin`). Requires `metin_bosalt`. |
| `upp.mesaj` | `metin baslik, metin govde` | `bos` | Displays an informational message box (Windows `MessageBox`; no-op on Linux). |
| `upp.hata` | `metin baslik, metin govde` | `bos` | Displays an error message box (Windows `MessageBox` with Error icon). |
| `upp.ses_cal` | `metin wav_yolu` | `bos` | Plays a WAV audio file (Windows; no-op on Linux). |

### Example Usage

```text
fonk ana() -> sayi {
    upp.satir_yaz("Please enter your name: ");
    oto name = upp.giris();

    eger (upp.uzunluk(name) == 0) {
        upp.hata_satir_yaz("Error: Name cannot be empty!");
    } yoksa {
        upp.satir_yaz("Welcome, {name}!");
    }

    upp.metin_bosalt(name);
    don 0;
}
```

---

## 3. Conversions and Utility Functions

| Function | Parameters | Return Type | Description |
|---|---|---|---|
| `upp.uzunluk` | `metin` or `array[]` | `sayi` | Returns character count for UTF-8 strings or element count for arrays. |
| `upp.metinden_sayiya` | `metin s` | `sayi` | Parses a string into a 64-bit integer (`atoll`). |
| `upp.sayidan_metin` | `sayi n` | `metin` | Converts an integer into a string (requires `metin_bosalt`). |
| `upp.ondalikdan_metin` | `ondalik d` | `metin` | Converts a floating-point number into a string (requires `metin_bosalt`). |
| `upp.metinden_ondalik` | `metin s` | `ondalik` | Parses a string into a double-precision float (`atof`). |
| `upp.min` | `sayi a, sayi b` | `sayi` | Returns the minimum of two integers. |
| `upp.max` | `sayi a, sayi b` | `sayi` | Returns the maximum of two integers. |
| `upp.mutlak` | `sayi n` | `sayi` | Returns the absolute value of an integer. |
| `upp.zaman` | — | `sayi` | Monotonic elapsed time in milliseconds since process start. |
| `upp.uyut` | `sayi milisaniye` | `bos` | Pauses execution for the specified number of milliseconds. |
| `upp.rastgele` | `sayi min, sayi max` | `sayi` | Generates a random 64-bit integer in the closed interval `[min, max]`. |
| `upp.metin_bosalt` | `metin s` | `bos` | Frees a dynamically allocated heap string. |

---

## 4. String Operations (`upp.metin`)

A comprehensive suite of built-in functions for string inspection and manipulation.

| Function | Parameters | Return Type | Memory / Notes |
|---|---|---|---|
| `upp.metin.kod` | `metin s, sayi i` | `sayi` | Unicode code point at index `i`. Identical to `s[i]`. |
| `upp.metin.ileri` | `metin s, sayi i` | `sayi` | UTF-8 byte width of character at index `i` (1 to 4). |
| `upp.metin.kes` | `metin s, sayi basla, sayi uz` | `metin` | Substring of `uz` characters starting at index `basla` (requires `metin_bosalt`). |
| `upp.metin.icinde` | `metin kaynak, metin aranan` | `mantik` | Checks if substring `aranan` is contained within `kaynak`. |
| `upp.metin.degistir` | `metin s, metin eski, metin yeni` | `metin` | Replaces occurrences of `eski` with `yeni` (requires `metin_bosalt`). |
| `upp.metin.kirp` | `metin s` | `metin` | Trims leading and trailing whitespace (requires `metin_bosalt`). |
| `upp.metin.buyuk` | `metin s` | `metin` | Converts ASCII/UTF-8 characters to uppercase (requires `metin_bosalt`). |
| `upp.metin.kucuk` | `metin s` | `metin` | Converts ASCII/UTF-8 characters to lowercase (requires `metin_bosalt`). |
| `upp.metin.bol` | `metin s, metin ayirac` | `liste[metin]` | Splits string by delimiter. An empty delimiter splits into individual characters. |
| `upp.metin.birlestir` | `liste[metin] l, metin ayirac` | `metin` | Joins a list of strings using `ayirac` (requires `metin_bosalt`). |

### Example Usage

```text
metin raw = "  apple,banana,orange  ";
metin trimmed = upp.metin.kirp(raw);
liste[metin] fruits = upp.metin.bol(trimmed, ",");

her (oto fruit in fruits) {
    upp.satir_yaz("Fruit: {fruit}");
}

metin joined = upp.metin.birlestir(fruits, " - ");
upp.satir_yaz("Result: {joined}");

upp.metin_bosalt(trimmed);
upp.metin_bosalt(joined);
fruits.bosalt();
```

---

## 5. File System (`upp.dosya`)

| Function | Parameters | Return Type | Description |
|---|---|---|---|
| `upp.dosya_oku` | `metin yol` | `metin` | Reads entire file contents. Returns `yok` (`NULL`) on failure (requires `metin_bosalt`). |
| `upp.dosya_yaz` | `metin yol, metin icerik` | `mantik` | Creates or overwrites a file with the given content. Returns success status. |
| `upp.dosya_ekle` | `metin yol, metin icerik` | `mantik` | Appends content to the end of a file. |
| `upp.dosya_var_mi` | `metin yol` | `mantik` | Tests whether a file exists at the given path. |
| `upp.dosya_sil` | `metin yol` | `mantik` | Deletes the specified file. |
| `upp.dosya.listele` | `metin yol` | `liste[metin]` | Returns a list of file and directory names in the specified path. |
| `upp.ortam_al` | `metin degisken_adi` | `metin` | Reads an environment variable (returns `yok` if unset). |

---

## 6. Path and Directory Operations (`upp.yol`)

Cross-platform filesystem path utilities and folder creation.

| Function | Parameters | Return Type | Description |
|---|---|---|---|
| `upp.yol.dizin` | `metin yol` | `metin` | Returns the directory name / parent directory (requires `metin_bosalt`). |
| `upp.yol.birlestir` | `metin yol1, metin yol2` | `metin` | Joins path components using the system separator (`/` or `\`). |
| `upp.yol.gercek` | `metin yol` | `metin` | Resolves relative paths into a canonical absolute path (`realpath`). |
| `upp.yol.calisma` | — | `metin` | Returns the current working directory (CWD) of the process. |
| `upp.yol.mutlak_mi` | `metin yol` | `mantik` | Checks if a path is absolute. |
| `upp.yol.klasor_olustur` | `metin yol` | `mantik` | Recursively creates missing directories (`mkdir -p`). |

---

## 7. System and Process Management (`upp.sistem`)

### Command-Line Arguments

| Function | Return Type | Description |
|---|---|---|
| `upp.arguman_sayisi()` | `sayi` | Number of command-line arguments passed to the program (`argc`). |
| `upp.arguman(sayi i)` | `metin` | Returns the `i`-th argument (`argv[i]`; index 0 is executable path). Deallocation is **not required**. |

### `SurecCikti` Class and Process Execution

```text
sinif SurecCikti {
    metin cikti; // Combined stdout and stderr output (malloc)
    sayi kod;    // Process exit code (0 = success)
}
```

| Function | Parameters | Return Type | Description |
|---|---|---|---|
| `upp.sistem.calistir` | `metin komut` | `SurecCikti` | Executes a shell command. `cikti` requires `metin_bosalt`. |
| `upp.sistem.komut_bul` | `metin komut_adi` | `metin` | Resolves an executable on the system `PATH` (`which`/`where`). |
| `upp.sistem.exe_dizin` | — | `metin` | Returns the directory path containing the running binary. |

> **Security Notice:** `upp.sistem.calistir` invokes commands via the system shell (`cmd.exe /C` on Windows, `popen` on Linux). Never concatenate untrusted or unsanitized user input directly into command strings.

---

## 8. Math Library (`upp.matematik`)

All trigonometric functions operate in **radians**.

### Standard Math Functions
* Trigonometry: `sin(r)`, `cos(r)`, `tan(r)`, `asin(v)`, `acos(v)`, `atan(v)`, `atan2(y, x)` (`ondalik -> ondalik`).
* Powers and Roots: `karekok(x)` (sqrt), `us(taban, us)` (pow).
* Rounding: `yuvarla(x)` (round), `taban(x)` (floor), `tavan(x)` (ceil).
* Angle Conversions: `radyan(degrees)`, `derece(radians)`.

### Geometric and Vector Types

```text
sinif Vektor3 { ondalik x; ondalik y; ondalik z; }
sinif Yonelme { ondalik pitch; ondalik yaw; }
sinif EkranNokta { ondalik x; ondalik y; mantik gorunur; }
```

| Function | Signature | Description |
|---|---|---|
| `mesafe` | `(Vektor3 a, Vektor3 b) -> ondalik` | Computes 3D Euclidean distance between two vectors. |
| `mesafe_3d` | `(x1, y1, z1, x2, y2, z2) -> ondalik` | Computes 3D Euclidean distance from raw coordinates. |
| `aci_hesapla` | `(x1, y1, z1, x2, y2, z2) -> Yonelme` | Computes aim angles (`pitch` and `yaw`) from source to target. |
| `dunya_ekran` | `(...) -> EkranNokta` | Projects 3D world coordinates onto 2D screen space via view-projection matrix. |

---

## 9. Data Structures: `liste` and `harita`

### Dynamic List: `liste[T]`
Supported element types (`T`): `sayi`, `ondalik`, `mantik`, `metin`.

| Method | Signature | Description |
|---|---|---|
| `.ekle(element)` | `(T) -> bos` | Appends an element to the end of the list. |
| `.al(index)` | `(sayi) -> T` | Returns element at `index`. Yields an owned copy for `metin`. |
| `.yaz(index, v)` | `(sayi, T) -> bos` | Overwrites element at `index`. `liste[i] = v` syntax is also valid. |
| `.uzunluk()` | `() -> sayi` | Returns the number of elements in the list. |
| `.bosalt()` | `() -> bos` | Frees all internal memory used by the list. |

### Key-Value Map: `harita[K, V]`
* Key types (`K`): `metin`, `sayi`.
* Value types (`V`): `sayi`, `ondalik`, `mantik`, `metin`.

| Method | Signature | Description |
|---|---|---|
| `.koy(key, value)` | `(K, V) -> bos` | Inserts a key-value pair or updates the existing value. |
| `.al(key)` | `(K) -> V` | Retrieves value associated with `key`. |
| `.var_mi(key)` | `(K) -> mantik` | Checks whether the given key exists in the map. |
| `.uzunluk()` | `() -> sayi` | Returns total number of key-value pairs. |
| `.anahtarlar()` | `() -> liste[K]` | Returns all keys in insertion order as a `liste[K]`. |
| `.bosalt()` | `() -> bos` | Releases internal map memory. |

---

## 10. JSON Library (`upp.json`)

JSON trees are managed in a C-level node pool and accessed in u++ through a lightweight handle (`JSONDeger`, wrapping `sayi id`; `id == 0` represents an invalid or null node).

| Function | Parameters | Return Type | Description |
|---|---|---|---|
| `upp.json.ayristir` | `metin json_metni` | `JSONDeger` | Parses a JSON string. Returns a handle with `id == 0` on failure. |
| `upp.json.anahtar_al` | `JSONDeger d, metin anahtar` | `JSONDeger` | Retrieves child node handle for the given object key. |
| `upp.json.indeks_al` | `JSONDeger d, sayi indeks` | `JSONDeger` | Retrieves child node handle at array index. |
| `upp.json.metin_al` | `JSONDeger d` | `metin` | Retrieves string value of node (requires `metin_bosalt`). |
| `upp.json.sayi_al` | `JSONDeger d` | `sayi` | Retrieves integer value of node. |
| `upp.json.ondalik_al` | `JSONDeger d` | `ondalik` | Retrieves floating-point value of node. |
| `upp.json.mantik_al` | `JSONDeger d` | `mantik` | Retrieves boolean value of node. |
| `upp.json.uzunluk` | `JSONDeger d` | `sayi` | Returns element count of JSON object or array. |
| `upp.json.tur` | `JSONDeger d` | `metin` | Returns node type: `"nesne"`, `"dizi"`, `"metin"`, `"sayi"`, `"mantik"`, `"bosluk"`. |
| `upp.json.var_mi` | `JSONDeger d` | `mantik` | Checks if the handle points to a valid JSON node. |
| `upp.json.bosalt` | `JSONDeger kok` | `bos` | Releases the entire parsed JSON tree. |

---

## 11. Concurrency and Synchronization: `arkaplan` and `Kilit`

### `arkaplan` and `ArkaplanIs`
Spawns a new native OS thread (POSIX pthread or Windows thread):

```text
oto task = arkaplan {
    upp.satir_yaz("Running in background thread");
};
task.bekle(); // or: upp.bekle(task);
```

* **Capture Semantics:** Enclosed variables of type `sayi` and `metin` are **copied by value** into the worker thread.
* **Restrictions:** Dynamic collections (`liste`, `harita`) and `JSONDeger` handles cannot be captured across thread boundaries to prevent data races.

### `Kilit` (Mutex)
Ensures mutual exclusion across multiple threads. Automatically initializes a system mutex handle upon declaration.

| Method | Signature | Description |
|---|---|---|
| `.kilitle()` | `() -> bos` | Acquires the mutex; blocks until the lock becomes available. |
| `.birak()` | `() -> bos` | Releases the mutex. |

---

## 12. Low-Level Utilities (`guvensiz` Area)

These operations are exclusively permitted within `guvensiz { ... }` blocks.

### Process Memory Inspection (`upp.bellek`)
* `surec_ac(process_name) -> sayi`: Opens a handle to the target process.
* `surec_kapat(handle) -> bos`: Closes the process handle.
* `modul_temel_adresi(handle, module) -> sayi`: Resolves the base address of a module/DLL.
* `sayi_oku(handle, address) -> sayi`: Reads a 64-bit integer from target process memory.
* `sayi_yaz(handle, address, value) -> mantik`: Writes a 64-bit integer into target process memory.
* `ondalik_oku` / `ondalik_yaz`: Floating-point process memory operations.
* `metin_oku(handle, address, length) -> metin`: Reads raw string bytes from target memory.
* `zincir_oku(handle, base, offsets[]) -> sayi`: Traverses a multi-level pointer offset chain.
* `desen_tara(handle, module, pattern, mask) -> sayi`: Scans target memory for an array of bytes (AOB signature).

### Synthetic Input and Drawing Overlay
* `upp.girdi`: `fare_tasi(x, y)`, `fare_tikla(btn)`, `tus_bas(code)`, `tus_birak(code)`, `tus_durumu(code) -> mantik` (Windows; no-op on Linux).
* `upp.cizim`: `baslat(window)`, `cizgi(...)`, `kutu(...)`, `kutu_dolu(...)`, `metin(...)`, `temizle()`, `bitir()`.

---

## 13. LSP and Stdio Framing (`upp.stdio`)

Switches standard input/output into framed binary mode for JSON-RPC / Language Server Protocol (`Content-Length: N\r\n\r\n`).

* `upp.stdio.gelen_oku() -> metin`: Reads framed message from `stdin` and extracts the JSON body (requires `metin_bosalt`).
* `upp.stdio.yaz(json_string) -> bos`: Formats and emits the JSON string with correct `Content-Length` headers to `stdout`.

---

## 14. Compiler Command-Line Interface (CLI) and JSON Diagnostic Schema

Both the Python compiler (`uppc.py`) and the self-hosted native compiler (`uppc.exe`) accept identical command-line flags:

```bash
python uppc.py <source.upp> [flags]
# or natively:
derleme/uppc.exe <source.upp> [flags]
```

### Compiler Flags

| Flag | Description |
|---|---|
| `--cikti <NAME>` | Sets base output name. Produces `<NAME>.c` and `<NAME>.exe` / `<NAME>.out`. |
| `--sadece-derle` | Compiles into a native binary without executing it. |
| `--sadece-c` | Emits intermediate GNU C11 source code only; does not invoke GCC. |
| `--linux` | Targets the Linux (POSIX) ABI. |
| `--windows` | Targets the Windows (Win32) ABI. |
| `--analiz` | Fast editor diagnostic mode (Lex/Parse/Type/Safety checks). Emits JSON up to 50 errors; no GCC invoked. |
| `--json-hata` | Runs full compile pipeline (C + GCC) and outputs the final result as single-line JSON. |
| `--bicim` | Formats source code in-place using standard 4-space indentation. |
| `--bicim-kontrol` | Checks if source code is properly formatted; exits with code `1` if not (for CI/CD). |
| `--ast` | Pretty-prints the parsed abstract syntax tree (AST). |

### Editor Diagnostic JSON Schema (`--analiz`)

```json
{
  "ok": false,
  "hatalar": [
    {
      "satir": 12,
      "sutun": 8,
      "tur": "hata",
      "mesaj": "Tanimlanmamis degisken: 'skor'"
    }
  ]
}
```

* `tur` values: `"hata"` (syntax or type error), `"guvenlik"` (memory safety violation), `"gcc"` (C compilation or linker error).

---

## 15. C Type and Symbol Mapping Table

| u++ Construct | C11 Equivalent | Notes |
|---|---|---|
| `sayi` | `long long` | 64-bit signed integer |
| `ondalik` | `double` | 64-bit IEEE 754 floating-point |
| `metin` | `char*` | UTF-8 encoded string |
| `mantik` | `bool` / `int` | `1` (`dogru`) or `0` (`yanlis`) |
| `bos` | `void` | Empty return type |
| `yok` | `NULL` | Null pointer |
| `liste[T]` | `UppKolListe` | Dynamic collection struct |
| `harita[K, V]` | `UppKolHarita` | Hash map struct |
| `SurecCikti` | `struct UppSurecCikti` | `{ char* cikti; long long kod; }` |
| `Kilit` | `struct UppKilit` | Platform mutex handle struct |
| `s[i]` | `upp_metin_kod(s, i)` | UTF-8 Unicode code point lookup |
| `s += t` | `upp_metin_ekle(&s, t)` | Reallocates and concatenates string |
| `upp.sistem.calistir` | `upp_sistem_calistir(...)` | Shell execution via `_popen` / `popen` |
