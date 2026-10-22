# u++ Standard Library and API Reference (v3.1 beta)

This document defines the memory model, type system, error-handling mechanisms, `upp.*` standard library builtins, compilation targets, and command-line tooling of the **u++** programming language.

> [!NOTE]
> The single source of truth for all builtin functions inside the compiler is [`src/uppc/yerlesik.upp`](../src/uppc/yerlesik.upp). The type checker reads parameter counts, parameter and return types, `guvensiz` block requirements, and target availability (`hosted`, `freestanding`, `windows`, `linux`) directly from that table. This reference mirrors that table one-to-one.

📚 **Related Documentation:** [Language Tutorial (OGREN.md)](../OGREN.md) · [Compiler Internals (derleyici.md)](../derleyici.md) · [Uppix Operating System](../src/os/README.md) · [Türkçe Referans (API_tr.md)](API_tr.md)

---

## 📑 Table of Contents

1. [Memory Model and Language Mechanisms](#1-memory-model-and-language-mechanisms)
2. [Type System, Casts, and `boyut(T)`](#2-type-system-casts-and-boyutt)
3. [String Interpolation and Standard I/O](#3-string-interpolation-and-standard-io)
4. [General Builtins (`upp.*`)](#4-general-builtins-upp)
5. [String and UTF-8 Operations (`upp.metin`)](#5-string-and-utf-8-operations-uppmetin)
6. [File Operations (`upp.dosya`)](#6-file-operations-uppdosya)
7. [Path and Directory Operations (`upp.yol`)](#7-path-and-directory-operations-uppyol)
8. [System, Process, and Standard I/O (`upp.sistem`, `upp.stdio`)](#8-system-process-and-standard-io-uppsistem-uppstdio)
9. [Platform-Specific APIs (`upp.linux`, `upp.windows`)](#9-platform-specific-apis-upplinux-uppwindows)
10. [Math and Geometry (`upp.matematik`)](#10-math-and-geometry-uppmatematik)
11. [Dynamic Collections (`liste[T]` and `harita[K, V]`)](#11-dynamic-collections-listet-and-haritak-v)
12. [JSON Operations (`upp.json`)](#12-json-operations-uppjson)
13. [Concurrency (`arkaplan` and `Kilit`)](#13-concurrency-arkaplan-and-kilit)
14. [Kernel, Hardware, and Byte Buffers (`upp.cekirdek`, `upp.tampon`)](#14-kernel-hardware-and-byte-buffers-uppcekirdek-upptampon)
15. [Compilation Targets](#15-compilation-targets)
16. [Command Line (`uppc`) and `upp.proje`](#16-command-line-uppc-and-uppproje)

---

## 1. Memory Model and Language Mechanisms

### String Ownership (Managed UTF-8 and Automatic Drop)
- Every `metin` is a managed object: a hidden header (`UppMetinBas`) located immediately before the character buffer stores an atomic reference count, byte length, capacity, code-point count, and a lazy UTF-8 index table. The character pointer itself remains a NUL-terminated (`\0`), C-compatible `char*`.
- **Copying and Appending:** Assignment and copying run in $O(1)$ time by incrementing the atomic reference count. The `+=` operator grows a uniquely owned string in place and performs **Copy-on-Write (COW)** when the string is shared across multiple bindings.
- **Automatic Cleanup:** The compiler automatically injects release calls at block exits, loop jumps (`dur` / `devam`), and function returns (`don`). Manual deallocation is never required.
- **Fast UTF-8 Indexing:** `upp.uzunluk(s)` runs in $O(1)$ time. Indexing (`s[i]`) is $O(1)$ directly on ASCII strings; non-ASCII UTF-8 strings lazily build and cache a per-string code-point offset table.
- **Immortal Literals:** String literals (`"merhaba"`) are emitted into a compile-time pool with pre-populated headers and never trigger runtime `malloc` or `free`.

### Owned Classes
For classes that contain `metin` fields, collections, or nested owned classes, the compiler automatically synthesizes `T__birak` (drop) and `T__kopya` (copy) helpers:
- Copying from a variable increments inner reference counts; temporary values (constructor calls or function return values) are moved without copying.
- Reassignment releases the previous value. Function parameters take ownership on entry, `don` moves the returned value out, and local class instances are released automatically at scope exit.

### Views (`&metin` and `&dilim[T]`)
Zero-allocation, zero-copy, read-only borrows (fat pointers):
- Example: `fonk yaz_gorunum(&metin v)`, `&dilim[sayi] d = arr[1..4];`
- **Fields:** `v.uzunluk` (UTF-8 **byte** length for `&metin`; element count for `&dilim[T]`) and `v.veri` (available on `&metin` only).
- **Safety Rules (Dangling-Reference Prevention):** View types cannot be returned from a function (`don`), stored in `sinif` fields, declared as top-level (global) variables, or captured inside `arkaplan` blocks.

<a id="collections"></a>
<a id="koleksiyonlar"></a>
### Collections (`liste[T]` and `harita[K, V]`)
`liste[T]` and `harita[K, V]` have **value semantics**:
- Copying a collection is $O(1)$ (reference count increment), and mutating a shared collection splits its backing storage on write (**Copy-on-Write**). After `M = L;`, calling `M.ekle(x)` does not modify `L`, and passing a collection to a function never mutates the caller's instance.
- The compiler automatically releases local collections at scope exit. Calling `.bosalt()` only returns memory early before scope end and leaves the collection empty.
- **Supported Element Types:** Any value type except raw pointers and views can be stored in a collection (`sayi`, `ondalik`, `mantik`, `metin`, `sinif`, nested `liste[...]`, `harita[...]`, `fonk(...)`, `beklenen[...]`). Map keys can be `metin` or `sayi`.
- **In-Place Mutation:** Chained indexing and method calls such as `L[i][j] = x`, `K[0].ad += "!"`, and `K[1].yaslan()` mutate elements directly in place.
- `JSONDeger` is an explicit handle; release it with `upp.json.bosalt(root)` when finished.

### Function Values (`fonk(P...) -> R`)
`fonk(P1, P2) -> R` is a first-class type (`-> R` defaults to `bos` if omitted). Top-level functions can be referenced by name as values and stored in variables, parameters, return values, class fields, and collections:

```upp
fonk ikiye(sayi x) -> sayi => x * 2;
fonk uygula(fonk(sayi) -> sayi f, sayi x) -> sayi => f(x);

fonk(sayi) -> sayi f = ikiye;
upp.satir_yaz("{uygula(f, 5)}");

harita[metin, fonk(sayi, sayi) -> sayi] komutlar;
komutlar.koy("carp", fonk(sayi a, sayi b) -> sayi => a * b);
upp.satir_yaz("{komutlar.al(\"carp\")(3, 4)}");
```

- **Anonymous Functions:** Written as `fonk(sayi x) -> sayi => x * 2` (expression body) or `fonk(...) -> T { ... }` (block body). Anonymous functions are captureless: accessing an outer local variable is a compile error (pass needed values as parameters instead).
- **Null Function Value (`yok`):** An uninitialized function variable holds `yok`. Check it with `f == yok`; invoking a `yok` function value halts the program with a clear panic message.
- **Constraints and Target Support:** Signatures must match exactly. Only `==` and `!=` comparisons are supported on function values. Class methods and `ana` cannot be taken as function values. Because function values compile down to plain C function pointers, they work with zero overhead on `--hedef serbest` and `--hedef cekirdek` targets as well.

### Error Handling (`beklenen`, `?`, `ertele`, and `hata_ertele`)
`beklenen[T, H]` holds either a successful value of type `T` or an error of type `H` (`beklenen[T]` is shorthand for `beklenen[T, metin]`). In C, it lowers to a plain tagged `struct` + `union`, returned by value with zero hidden heap allocation. When the error type `H` is a `secenek` enum, the heap is never touched at all.

```upp
secenek DiskHata { YOK, BOZUK, DOLU }

fonk sektor(sayi no) -> beklenen[sayi, DiskHata] {
    eger (no > 100) { don hata(DiskHata.DOLU); }
    don no * 512;                     // Returning a T value directly indicates success
}

fonk iki_sektor(sayi a, sayi b) -> beklenen[sayi, DiskHata] {
    sayi x = sektor(a)?;              // On error: cleans up scope and propagates the error
    sayi y = sektor(b)?;
    don x + y;
}
```

- **Construction and Return:** Inside a function returning `beklenen`, `don x;` returns a success value and `don hata(e);` returns an error. The same syntax applies to variable initializers, assignments, and call arguments (`beklenen[sayi] r = 5;`, `r = hata("x");`). A default-initialized `beklenen` variable starts in the success state with the zero value of `T`.
- **`beklenen[bos, H]`:** Carries no payload on success; `don;` or reaching the end of the function body returns success.
- **Methods:**
  - `r.tamam_mi() -> mantik`: Returns `dogru` if holding a success value.
  - `r.hata_mi() -> mantik`: Returns `dogru` if holding an error.
  - `r.deger() -> T`: Unwraps the success value; panics if holding an error (printing the error message when `H` is `metin`).
  - `r.hata() -> H`: Unwraps the error value; panics if holding a success value.
  - `r.deger_yoksa(fallback) -> T`: Returns the success value or `fallback` if holding an error.
- **`e?` (Error Propagation Operator):** Allowed only inside functions returning `beklenen` with the same error type `H`. Disallowed in loop conditions, on the right-hand side of `&&` / `||`, and inside `?:` ternary branches.
- **No Unused Results:** Discarding a `beklenen` return value via a bare `f();` statement is a compile error; either propagate it with `?` or bind and inspect it.
- **Restrictions:** `beklenen` cannot be used as a `sinif` field, cannot be captured by `arkaplan`, and cannot have a top-level initializer. `hata(...)` cannot be used directly with `oto`.

#### `ertele` (`defer`) and `hata_ertele` (`errdefer`)
- **`ertele`:** Executes in reverse registration order (LIFO) on **every exit** from its enclosing scope (end of block, `dur`, `devam`, `don`, or `?`).
- **`hata_ertele`:** Executes **only when the function exits with an error** (`don hata(...)`, early exit via `?`, or returning a `beklenen` variable that dynamically holds an error at runtime).
- Both accept either a single statement (`ertele expr;`) or a block (`ertele { ... }`). Exiting a deferred block via `don`, `?`, or an outer loop's `dur`/`devam` is a compile error. The function's return value is evaluated before deferred blocks run.

```upp
fonk ac(metin ad) -> beklenen[Dosya] {
    oto d = dosya_ac(ad)?;
    hata_ertele d.kapat();            // Runs only if a subsequent step fails
    baslik_oku(d)?;
    don d;
}
```

### Checked Integer Division
`/` and `%` explicitly panic on division by zero and on `MIN / -1` (`LLONG_MIN / -1`) signed overflow across all targets (including the kernel) instead of invoking C undefined behavior. Division by a compile-time constant `0` is a compile error.

### `guvensiz` (`unsafe`) Blocks
The following operations are permitted only inside explicit `guvensiz { ... }` blocks:
- Raw pointer types and operations (`*`, `&`) and pointer casts (`(u16*)0xB8000`),
- Manual `bellek_ayir` / `bellek_bosalt` allocations and inline `c_kod { ... }` blocks,
- Foreign process memory access (`upp.windows.bellek.*`, `upp.linux.sayi_oku/yaz`),
- Direct hardware port I/O, MMIO, and privileged instructions (`upp.cekirdek.port_*`, `bellek_*`, `dur`, `kesme_ac/kapat`, `yigin_kur`).

#### `c_kod` and `metin`
A `metin` is always a **managed** value: a header (reference count, length, magic) sits right before the data and the contents are shared. Inside `c_kod` a metin variable can be read like any `const char*` (`strlen(s)`, `printf("%s", s)`), but it may only be changed through these calls:

| C call | Effect |
| :--- | :--- |
| `upp_metin_ata_c(&s, c)` | **Copies** a NUL-terminated raw C string (literal, stack buffer, `getenv` result…) into `s` and releases the old value. If `c` is `NULL`, `s` becomes empty |
| `upp_metin_ata(&s, t)` | Shares another managed string (`t` is a u++ metin or `UPP_LIT("...")`) and releases the old value |

```upp
metin s = "old";
guvensiz {
    c_kod {
        char buf[16];
        snprintf(buf, sizeof buf, "%d", 42);
        upp_metin_ata_c(&s, buf);
    }
}
```

The compiler rejects, with a compile error, any `c_kod` text that re-points a `metin` variable in scope (`s = ...`, `s += 1`, `s++`) or writes into its contents (`s[0] = ...`, `*s = ...`). A C local of the same name declared by the `c_kod` block itself (`char* s = ...`) and member accesses (`p->s`) are not affected. The check is lexical, so indirect assignment through a pointer (`char** p = &s; *p = "x";`) cannot be caught. In that case the header-less string stops with a `yönetilmeyen metin` panic on first use instead of silently corrupting memory.

#### `c_harici` and `cmetin`: Type-Safe Calls into C
Most of the time `c_kod` is unnecessary: declare a C function with its signature and call it from u++ like any other function.

```upp
c_harici fonk strlen(cmetin s) -> u64;
c_harici fonk getenv(cmetin ad) -> cmetin;
c_harici "abs" fonk c_mutlak(s32 x) -> s32;   // when the C symbol differs from the u++ name

fonk ana() -> sayi {
    guvensiz {
        u64 n = strlen("merhaba");             // metin → cmetin is implicit (borrowed)
        cmetin yol = getenv("PATH");
        eger (yol != yok) {
            metin kopya = (metin)yol;          // managed copy of the C string
            upp.satir_yaz(kopya);
        }
    }
    don 0;
}
```

- **Declaration:** `c_harici [\"c_symbol\"] fonk name(params) -> type;` appears only at top level and has no body. The compiler emits `extern T _upp_c_name(...) __asm__("c_symbol");`. The GCC asm label binds the call straight to the symbol, so it never conflicts with a prototype from a header brought in by `c_ekle` (e.g. `size_t strlen(const char*)`). If the u++ name is not ASCII, the C symbol must be given explicitly.
- **Allowed types:**
  - Parameters: integers, `ondalik`, `mantik`, `cmetin`, `metin` (the data itself, borrowed), and pointers.
  - Return: the same plus `bos`; `metin` cannot be returned.
  - Lists, maps, classes, views, and `beklenen` cannot cross into C.
- **Safety:** calling a `c_harici` function or taking it as a value (`fonk(cmetin) -> u64 f = strlen;`) is allowed only inside `guvensiz`; the compiler cannot verify the memory safety of C code.
- **`cmetin`:** a header-less raw C string (`const char*`). It is not managed and never freed. Rules:
  - **Assignment:** `metin` → `cmetin` is implicit and lends the data itself. `cmetin` → `metin` happens only via `(metin)c`, which copies; `NULL` becomes an empty string.
  - **Comparison:** only `==` / `!=` against `yok` or another `cmetin` (pointer equality). No arithmetic, indexing, or printing; use `(metin)c` for the contents.
  - **Where it can be used:** variables may be declared only inside `guvensiz`. It cannot be a class field or a collection element.
  - **Temporaries:** a temporary string such as `cmetin c = a + b;` cannot be stored in a `cmetin` variable, because it is freed when the statement ends. Passing one as a function argument is safe.

### Attributes: `@paketli`, `@hizali(N)`, `@bolum("name")`
Written immediately before a top-level declaration and emitted as GCC `__attribute__((...))`. Intended for controlling memory layout in kernel and hardware code.

| Attribute | Applies to | Effect |
| :--- | :--- | :--- |
| `@paketli` | `sinif` | No padding between fields (`packed`): `sinif Baslik { ubayt tur; u32 uzunluk; }` is 5 bytes |
| `@hizali(N)` | `sinif`, top-level `fonk`, global variable | Aligns to `N` bytes (`aligned(N)`); `N` must be a power of two between 1 and 2^30 (e.g. `@hizali(4096)` for page tables) |
| `@bolum("name")` | top-level `fonk`, global variable | Selects the linker section (`section("name")`); global variables also get `used`, so they are kept even if the code never references them. The name may contain only letters, digits, `.`, `_`, `$` (at most 64 characters; short names are recommended for Windows PE) |

```upp
@paketli
sinif GdtGirdisi { u16 sinir; u16 taban_alt; ubayt taban_orta; ubayt erisim; ubayt bayrak; ubayt taban_ust; }

@bolum(".limine_istekler") @hizali(8)
u64 g_istek = 0;
```

An unknown attribute, a repeated attribute, or an attribute applied to an unsupported declaration is a compile error.

---

## 2. Type System, Casts, and `boyut(T)`

| Type | C Representation | Description |
| :--- | :--- | :--- |
| `sayi` / `s64` | `long long` | Signed 64-bit integer (default type of unsuffixed integer literals) |
| `u64` / `usayi` | `unsigned long long` | Unsigned 64-bit integer (e.g. `5u64`) |
| `s32` / `u32`, `s16` / `u16` | `int32_t`, `uint32_t`, `int16_t`, `uint16_t` | Fixed-width narrow integers (e.g. `7u32`, `100u16`) |
| `bayt` / `s8`, `ubayt` / `u8` | `int8_t` / `uint8_t` | Signed (`-128..127`) and unsigned (`0..255`) 8-bit integers |
| `ondalik` | `double` | Double-precision (64-bit IEEE 754) floating-point number |
| `metin` | `char*` (with managed header) | Atomic reference-counted, Copy-on-Write UTF-8 string |
| `cmetin` | `const char*` | Header-less raw C string; only inside `guvensiz` and in `c_harici` signatures (see [`c_harici` and `cmetin`](#c_harici-and-cmetin-type-safe-calls-into-c)) |
| `mantik` | `bool` | Boolean value (`dogru` / `yanlis`) |
| `bos` | `void` | Return type for functions that return no value |
| `dizi[T, N]` | C array (`T[N]`) | Stack-allocated, runtime bounds-checked fixed array (`sayi a[] = [1, 2];`) |
| `liste[T]`, `harita[K, V]` | `UppKolListe`, `UppKolHarita` | Value-type (refcount + COW) dynamic collections |
| `fonk(P...) -> R` | Function pointer | First-class function value (nullable via `yok`) |
| `beklenen[T, H]`, `beklenen[T]` | Tagged `struct` + `union` | Result type holding a value or error (propagated with `?`) |
| `&metin`, `&dilim[T]` | `upp_ref_metin`, `upp_ref_dilim_*` | Zero-copy, zero-allocation read-only views |
| `sinif`, `secenek` | `struct`, `enum` | User-defined structure and enumeration types |

### Explicit Type Casts `(T)x` and `boyut(T)`
Uses standard C-style prefix cast syntax:
```upp
ubayt b = (ubayt)x;
u16 p = (u16)70000;
sayi n = (sayi)3.9;      // 3
Renk r = (Renk)2;
```
- **Numeric / Boolean / Enum Casts:** Freely allowed in safe code between numeric types, `mantik`, and `secenek`. Integer narrowing truncates low bits just like C (`(ubayt)300 == 44`).
- **`ondalik` $\rightarrow$ Integer Safety:** Converting a floating-point number to an integer panics explicitly at runtime if the value is `NaN` or outside the target integer type's range (avoiding C undefined behavior).
- **Pointer Casts (`guvensiz`):** Pointer $\leftrightarrow$ pointer, pointer $\leftrightarrow$ integer, and function value $\leftrightarrow$ pointer/integer casts are permitted only inside `guvensiz` blocks: `u16* vga = (u16*)0xB8000;`.
- `metin`, `sinif`, `liste`, `harita`, `beklenen`, and `dizi` types cannot be cast with `(T)x`. Exception: `(metin)c` copies a `cmetin` into a managed string, and `(cmetin)m` lends a string's data.
- **`boyut(T)` and `boyut(variable)`:** Returns the size in bytes of a type or variable as a `sayi` (equivalent to C `sizeof`; returns the total byte size for fixed arrays). Evaluated as a compile-time constant and usable in top-level `sabit` declarations.

### Character Literals
A character literal such as `'a'` is a single Unicode code point and behaves like an unsuffixed integer literal: its type is `sayi`, and it can also be assigned to types like `ubayt` or `u16` when it fits (`ubayt b = 'z';`). Turkish letters are written directly (`'ş'` == 351, `'İ'` == 304).

| Form | Value |
| :--- | :--- |
| `'a'`, `'ş'`, `'{'` | The character's code point |
| `'\n'` `'\t'` `'\r'` `'\0'` | 10, 9, 13, 0 |
| `'\\'` `'\''` `'\"'` `'\e'` | 92, 39, 34, 27 (ESC) |
| `'\xHH'` | 2 hex digits (`'\x41'` == 65) |
| `'\uHHHH'` | 4 hex digits (`'\u015f'` == `'ş'`); the UTF-16 surrogate range (D800–DFFF) is an error |

Empty (`''`) or multi-character (`'ab'`) literals and unknown escapes are compile errors. Use `"..."` for strings.

```upp
eger (tus == 'q' || tus == '\e') { cik(); }
```

---

## 3. String Interpolation and Standard I/O

- **Expression Interpolation (`"... {expr} ..."`):** Inside any string literal, a `{` immediately followed by a letter, `_`, `(`, or a complete character literal (`'a'`, `'\n'`) begins a full `u++` expression: `"{a[0]}"`, `"{f(x)}"`, `"{a + b * 2}"`, `"{k.ad}"`, `"{(1 + 2)}"`, `"{'}'}"`. Invalid expressions produce a compile-time error.
- **Expression source:** the text inside `{...}` is parsed exactly as written; the outer string's escapes apply only to the literal parts. The single exception is a nested string's quote, written `\"` so it does not close the outer string (`"{f(\"abc\")}"`). This makes `"{'\n'}"` and `"{'\''}"` work as expected. An unclosed quote such as `'{' ile` is not a character literal and stays literal text.
- **Escape Sequences:**
  - Use `{{` and `}}` to write literal braces (`{` followed by a space or digit is treated as a literal character automatically).
  - Standard escapes: `\n`, `\t`, `\r`, `\\`, `\"`, `\'`, `\e` (ESC - `0x1B`), `\xHH` (hex byte), and `\uHHHH` (4-digit Unicode code point, e.g. `\u00e7` $\rightarrow$ `ç`). Unrecognized escape sequences produce a compile error.
- **Printable Types:** All integer types, `ondalik`, `mantik`, `metin`, `&metin`, and `secenek` can be passed directly to `upp.yaz` and `upp.satir_yaz`.

| Function | Signature | Description |
| :--- | :--- | :--- |
| `upp.yaz(x)` | `(printable) -> bos` | Writes the value to standard output (COM1 + screen console on freestanding/kernel targets) without a trailing newline |
| `upp.satir_yaz(x)` | `(printable) -> bos` | Writes the value followed by a newline (`\n`) (`upp.satir_yaz(42)` is valid) |
| `upp.hata_yaz(x)` | `(printable) -> bos` | Writes the value to standard error (`stderr`) without a trailing newline |
| `upp.hata_satir_yaz(x)` | `(printable) -> bos` | Writes the value to standard error (`stderr`) followed by a newline |
| `upp.giris()` | `() -> metin` | Reads a line from standard input (PS/2 keyboard or serial port on the kernel target) |

> [!IMPORTANT]
> Output functions take a single argument (`printf`-style format strings and variadic arguments are not used). Combine multiple values using string interpolation (`"{a} and {b}"`).

---

## 4. General Builtins (`upp.*`)

| Function | Signature | Target | Description |
| :--- | :--- | :---: | :--- |
| `upp.uzunluk(x)` | `(metin \| dizi \| liste \| harita \| view) -> sayi` | All | Returns the UTF-8 code-point count for `metin` and `&metin`, or the element count for arrays, slices, lists, and maps |
| `upp.metinden_sayiya(s)` | `(metin) -> sayi` | All | Parses a string into a signed 64-bit integer |
| `upp.sayidan_metin(n)` | `(sayi) -> metin` | All | Formats a signed 64-bit integer as a string |
| `upp.ondalikdan_metin(d)` | `(ondalik) -> metin` | All | Formats a floating-point number with 17 significant digits |
| `upp.metinden_ondalik(s)` | `(metin) -> ondalik` | Hosted | Parses a string into a floating-point number (`ondalik`) |
| `upp.min(a, b)` | `(sayi, sayi) -> sayi` | All | Returns the smaller of two integers |
| `upp.max(a, b)` | `(sayi, sayi) -> sayi` | All | Returns the larger of two integers |
| `upp.mutlak(x)` | `(sayi) -> sayi` | All | Returns the absolute value of an integer (safe against `LLONG_MIN` overflow) |
| `upp.zaman()` | `() -> sayi` | All | Monotonic clock in milliseconds (driven by the PIT timer in the kernel) |
| `upp.uyut(ms)` | `(sayi) -> bos` | All | Sleeps the current thread or kernel task for `ms` milliseconds |
| `upp.rastgele(min, max)` | `(sayi, sayi) -> sayi` | Hosted | Returns a random integer in the inclusive range `[min, max]` |
| `upp.platform()` | `() -> metin` | Hosted | Returns the host OS identifier (`"windows"` or `"linux"`) |
| `upp.arguman_sayisi()` | `() -> sayi` | Hosted | Returns the number of command-line arguments (`argc`) |
| `upp.arguman(i)` | `(sayi) -> metin` | Hosted | Returns the `i`-th command-line argument (`argv[i]`) |
| `upp.mesaj(title, body)` | `(metin, metin) -> bos` | Hosted | Displays an info dialog (`MessageBox` on Windows, console output on Linux) |
| `upp.hata(title, body)` | `(metin, metin) -> bos` | Hosted | Displays an error dialog/message |
| `upp.ses_cal(path)` | `(metin) -> mantik` | Hosted | Plays a `.wav` audio file |

<details>
<summary><strong>⚠️ Removed Legacy API Names and Replacements</strong></summary>

The following legacy names were removed in `v3.1 beta` and produce a compile-time error pointing to the new name:
- `upp.metin_bosalt` $\rightarrow$ Removed (strings are dropped automatically at scope exit).
- `upp.dosya_oku / yaz / ekle / var_mi / sil / boyut` $\rightarrow$ `upp.dosya.oku`, `upp.dosya.yaz`, ...
- `upp.ortam_al / ortam_yaz` $\rightarrow$ `upp.sistem.ortam_al`, `upp.sistem.ortam_yaz`
- `upp.metin.icinde` $\rightarrow$ `upp.metin.bul` (check `>= 0`)
- `upp.yol.klasor_olustur` $\rightarrow$ `upp.yol.dizin_olustur`
- `upp.matematik.sin / cos / tan` $\rightarrow$ `upp.matematik.sinus / kosinus / tanjant`
- `upp.json.dizi_eleman` $\rightarrow$ `upp.json.indeks_al`
- `upp.bellek.*`, `upp.girdi.*`, `upp.cizim.*` $\rightarrow$ `upp.windows.bellek.*`, `upp.windows.girdi.*`, `upp.windows.cizim.*`
</details>

---

## 5. String and UTF-8 Operations (`upp.metin`)

All `upp.metin.*` builtins are `libc`-independent and work across both hosted and freestanding/kernel targets.

| Function | Signature | Description |
| :--- | :--- | :--- |
| `upp.metin.kes(s, start, count)` | `(metin, sayi, sayi) -> metin` | Extracts a substring of `count` UTF-8 code points starting at code-point index `start` |
| `upp.metin.buyuk(s)` | `(metin) -> metin` | Converts to uppercase using Turkish casing rules (`i` $\rightarrow$ `İ`, `ı` $\rightarrow$ `I`) |
| `upp.metin.kucuk(s)` | `(metin) -> metin` | Converts to lowercase using Turkish casing rules (`İ` $\rightarrow$ `i`, `I` $\rightarrow$ `ı`) |
| `upp.metin.ingbuyuk(s)` | `(metin) -> metin` | English (ASCII) uppercase: only `a-z` change; `i` $\rightarrow$ `I`, Turkish letters (`ı`, `ş`, `ö`…) are left untouched. For locale-independent text such as file names, protocols, and hex |
| `upp.metin.ingkucuk(s)` | `(metin) -> metin` | English (ASCII) lowercase: only `A-Z` change; `I` $\rightarrow$ `i` (not Turkish `ı`), letters like `İ` and `Ş` are kept (`"LIMINE.CONF"` $\rightarrow$ `"limine.conf"`) |
| `upp.metin.kirp(s)` | `(metin) -> metin` | Trims leading and trailing whitespace (` `, `\t`, `\r`, `\n`) |
| `upp.metin.baslar(s, prefix)` | `(metin, metin) -> mantik` | Returns `dogru` if `s` starts with `prefix` |
| `upp.metin.biter(s, suffix)` | `(metin, metin) -> mantik` | Returns `dogru` if `s` ends with `suffix` |
| `upp.metin.bul(s, needle)` | `(metin, metin) -> sayi` | Returns the UTF-8 code-point index of the first match, or `-1` if not found |
| `upp.metin.degistir(s, old, new)` | `(metin, metin, metin) -> metin` | Replaces all occurrences of `old` in `s` with `new` |
| `upp.metin.bol(s, sep)` | `(metin, metin) -> liste[metin]` | Splits `s` by `sep` into a new `liste[metin]` |
| `upp.metin.birlestir(L, sep)` | `(liste[metin], metin) -> metin` | Joins a list of strings with `sep` between elements |
| `upp.metin.kod(s, i)` | `(metin, sayi) -> sayi` | Returns the Unicode code-point value at character index `i` |
| `upp.metin.ileri(s, i)` | `(metin, sayi) -> sayi` | Returns the UTF-8 byte width (`1..4`) of the character at index `i` |
| `upp.metin.karakter(code)` | `(sayi) -> metin` | Creates a single-character UTF-8 string from a Unicode code point (or `""` if invalid) |
| `upp.metin.tekrar(s, n)` | `(metin, sayi) -> metin` | Repeats `s` `n` times |
| `upp.metin.hex_kodla(s)` | `(metin) -> metin` | Encodes raw bytes of `s` into a lowercase hexadecimal string |
| `upp.metin.hex_coz(hex)` | `(metin) -> metin` | Decodes a hexadecimal string into raw bytes |

---

## 6. File Operations (`upp.dosya`)

> *Available on hosted (Windows / Linux) targets only.*

| Function | Signature | Description |
| :--- | :--- | :--- |
| `upp.dosya.oku(path)` | `(metin) -> metin` | Reads the entire file as a string; returns `yok` if the file does not exist or cannot be read |
| `upp.dosya.yaz(path, content)` | `(metin, metin) -> mantik` | Writes `content` to `path` (overwriting if it exists); returns `dogru` on success |
| `upp.dosya.ekle(path, content)` | `(metin, metin) -> mantik` | Appends `content` to the end of `path`; returns `dogru` on success |
| `upp.dosya.var_mi(path)` | `(metin) -> mantik` | Returns `dogru` if a file exists at `path` |
| `upp.dosya.sil(path)` | `(metin) -> mantik` | Deletes the file at `path`; returns `dogru` on success |
| `upp.dosya.boyut(path)` | `(metin) -> sayi` | Returns the file size in bytes (or `-1` if missing) |
| `upp.dosya.listele(dir)` | `(metin) -> liste[metin]` | Lists file and subdirectory names inside `dir` (excluding `"."` and `".."`) |

---

## 7. Path and Directory Operations (`upp.yol`)

> *Available on hosted (Windows / Linux) targets only.*

| Function | Signature | Description |
| :--- | :--- | :--- |
| `upp.yol.birlestir(a, b)` | `(metin, metin) -> metin` | Joins two path segments using the platform's directory separator |
| `upp.yol.dizin(path)` | `(metin) -> metin` | Returns the parent directory portion of `path` (handles both `/` and `\`) |
| `upp.yol.dosya_adi(path)` | `(metin) -> metin` | Returns the final path component (file or directory name) |
| `upp.yol.uzanti(path)` | `(metin) -> metin` | Returns the file extension including the leading dot (e.g. `".upp"`, or `""` if none) |
| `upp.yol.mutlak_mi(path)` | `(metin) -> mantik` | Returns `dogru` if `path` is an absolute path |
| `upp.yol.gercek(path)` | `(metin) -> metin` | Resolves relative segments and symlinks to a canonical absolute path |
| `upp.yol.calisma()` | `() -> metin` | Returns the current working directory (`cwd`) |
| `upp.yol.dizin_var_mi(path)` | `(metin) -> mantik` | Returns `dogru` if `path` exists and is a directory |
| `upp.yol.dizin_olustur(path)` | `(metin) -> mantik` | Creates a directory and any missing parent directories recursively |

---

## 8. System, Process, and Standard I/O (`upp.sistem`, `upp.stdio`)

> *Available on hosted (Windows / Linux) targets only.*

| Function | Signature | Description |
| :--- | :--- | :--- |
| `upp.sistem.calistir(cmd)` | `(metin) -> SurecCikti` | Runs a subprocess with `stdin` closed, capturing both `stdout` and `stderr` into `SurecCikti { sayi kod; metin cikti; mantik basarili; }` |
| `upp.sistem.kos(cmd)` | `(metin) -> sayi` | Runs an interactive subprocess inheriting standard I/O handles and returns its exit code |
| `upp.sistem.komut_bul(name)` | `(metin) -> metin` | Searches the system `PATH` for `name` and returns its full path (or `""` if not found) |
| `upp.sistem.exe_dizin()` | `() -> metin` | Returns the directory containing the currently running executable |
| `upp.sistem.pid()` | `() -> sayi` | Returns the operating system process ID (PID) |
| `upp.sistem.cikis(code)` | `(sayi) -> bos` | Terminates the process immediately with the given exit code |
| `upp.sistem.ortam_al(name)` | `(metin) -> metin` | Returns the value of an environment variable (or `""` if unset) |
| `upp.sistem.ortam_yaz(name, val)` | `(metin, metin) -> mantik` | Sets an environment variable |
| `upp.stdio.gelen_oku()` | `() -> metin` | Reads an LSP `Content-Length` framed JSON-RPC message from `stdin` |
| `upp.stdio.yaz(json)` | `(metin) -> bos` | Writes an LSP `Content-Length` framed JSON-RPC message to `stdout` |

---

## 9. Platform-Specific APIs (`upp.linux`, `upp.windows`)

### `upp.linux.*` *(`--hedef linux` Only)*
| Function | Signature | Safety | Description |
| :--- | :--- | :---: | :--- |
| `upp.linux.pid()` | `() -> sayi` | Safe | Returns the current process ID |
| `upp.linux.sinyal_gonder(pid, sig)` | `(sayi, sayi) -> mantik` | Safe | Sends a POSIX signal (`kill`) to `pid` |
| `upp.linux.proc_oku(pid, file)` | `(sayi, metin) -> metin` | Safe | Reads `/proc/<pid>/<file>` |
| `upp.linux.sayi_oku(pid, addr)` | `(sayi, sayi) -> sayi` | `guvensiz` | Reads an integer from a target process's virtual memory |
| `upp.linux.sayi_yaz(pid, addr, val)` | `(sayi, sayi, sayi) -> mantik` | `guvensiz` | Writes an integer to a target process's virtual memory |

### `upp.windows.*` *(Windows Target Only)*
| Function | Signature | Safety | Description |
| :--- | :--- | :---: | :--- |
| `upp.windows.mesaj(title, body)` | `(metin, metin) -> bos` | Safe | Displays a Win32 information message box |
| `upp.windows.hata(title, body)` | `(metin, metin) -> bos` | Safe | Displays a Win32 error message box |
| `upp.windows.ses_cal(path)` | `(metin) -> mantik` | Safe | Plays a `.wav` file via WinMM |
| `upp.windows.girdi.fare_tasi(x, y)` | `(sayi, sayi) -> bos` | Safe | Moves the mouse cursor to screen coordinates `(x, y)` |
| `upp.windows.girdi.fare_tikla()` | `() -> bos` | Safe | Synthesizes a left mouse button click |
| `upp.windows.girdi.tus_bas(key)` | `(metin) -> bos` | Safe | Synthesizes a virtual key press |
| `upp.windows.cizim.baslat(win)` | `(metin) -> sayi` | Safe | Initializes a GDI overlay on the target window |
| `upp.windows.cizim.temizle()` | `() -> bos` | Safe | Clears the drawing overlay |
| `upp.windows.cizim.kutu(x, y, w, h, col)` | `(sayi, sayi, sayi, sayi, metin) -> bos` | Safe | Draws a rectangle outline in the specified color |
| `upp.windows.cizim.yazi(x, y, text, col)` | `(sayi, sayi, metin, metin) -> bos` | Safe | Draws text at `(x, y)` in the specified color |

### `upp.windows.bellek.*` *(Requires `guvensiz`)*
| Function | Signature | Description |
| :--- | :--- | :--- |
| `baglan(process_name)` | `(metin) -> sayi` | Attaches to a target process by name and returns its PID |
| `modul_bul(dll_name)` | `(metin) -> sayi` | Returns the base address of a loaded module inside the attached process |
| `sayioku(addr)` / `sayiyaz(addr, val)` | `(sayi) -> sayi` / `(sayi, sayi) -> mantik` | Reads / writes an integer at `addr` |
| `oku_ondalik(addr)` / `yaz_ondalik(addr, d)` | `(sayi) -> ondalik` / `(sayi, ondalik) -> mantik` | Reads / writes a `double` at `addr` |
| `oku_metin(addr, len)` | `(sayi, sayi) -> metin` | Reads a string of `len` bytes from `addr` |
| `zincir_oku(base, [offsets])` | `(sayi, dizi) -> sayi` | Follows a multi-level pointer chain and returns the resolved address |
| `yama_yap(addr, hex)` | `(sayi, metin) -> mantik` | Writes a hexadecimal byte patch to `addr` |
| `koruma_degistir(addr, size, mode)` | `(sayi, sayi, metin) -> mantik` | Changes memory page protection flags (`koruma_degistir_sayi` accepts numeric Win32 flags) |
| `son_hata()` / `okundu()` | `() -> sayi` | Returns the last Win32 error code / number of bytes transferred |

---

## 10. Math and Geometry (`upp.matematik`)

| Function | Signature | Target | Description |
| :--- | :--- | :---: | :--- |
| `pi()`, `e()` | `() -> ondalik` | All | Mathematical constants $\pi$ and $e$ |
| `mutlak(x)` | `(ondalik) -> ondalik` | All | Floating-point absolute value |
| `sinirla(x, min, max)` | `(ondalik, ondalik, ondalik) -> ondalik` | All | Clamps `x` to `[min, max]` |
| `yumusat(a, b, t)` | `(ondalik, ondalik, ondalik) -> ondalik` | All | Linear interpolation (`lerp`) |
| `karekok(x)`, `us(base, exp)` | `(ondalik) -> ondalik` / `(ondalik, ondalik) -> ondalik` | Hosted | Square root and exponentiation |
| `sinus(x)`, `kosinus(x)`, `tanjant(x)` | `(ondalik) -> ondalik` | Hosted | Trigonometric functions (in radians) |
| `atan2(y, x)` | `(ondalik, ondalik) -> ondalik` | Hosted | Two-argument arctangent |
| `taban(x)`, `tavan(x)`, `yuvarla(x)` | `(ondalik) -> sayi` | Hosted | Floor, ceiling, and nearest-integer rounding |
| `mesafe(v1, v2)` / `mesafe_3d(...)` | `(Vektor3, Vektor3) -> ondalik` | Hosted | 3D Euclidean distance |
| `aci_hesapla(x1, y1, z1, x2, y2, z2)` | `(ondalik x 6) -> Yonelme` | Hosted | Computes orientation angles (`pitch`, `yaw`) between two 3D points |
| `dunya_ekran(matrix, pt, w, h)` | `(ondalik[16], Vektor3, sayi, sayi) -> EkranNokta` | Hosted | Projects a 3D world coordinate onto 2D screen coordinates |

---

## 11. Dynamic Collections (`liste[T]` and `harita[K, V]`)

See [Collections](#collections) for value semantics (refcount + Copy-on-Write) and supported element types.

### `liste[T]` Methods and Operations
| Usage | Return Type | Description |
| :--- | :--- | :--- |
| `L.ekle(val)` | `bos` | Appends `val` to the end of the list |
| `L[i]` / `L.al(i)` | `T` | Reads element `i` (bounds-checked; panics on out-of-bounds). `L[i] = x` mutates in place |
| `L.yaz(i, val)` | `bos` | Overwrites element at index `i` |
| `L.sil(i)` | `bos` | Removes element at index `i` and shifts subsequent elements |
| `L.uzunluk()` | `sayi` | Returns the number of elements in the list |
| `L.temizle()` | `bos` | Removes all elements (sets length to `0`) |
| `L.bosalt()` | `bos` | Releases backing memory early and leaves the list empty |
| `L[a..b]` | `&dilim[T]` | Creates a half-open slice view `[a, b)` on `liste[sayi]` or `liste[ondalik]` |
| `her (oto x in L)` | — | Iterates through all elements in order |
| `her (sayi i in a..b)` | — | Range: iterates integers from `a` inclusive to `b` exclusive (`0..n` → `0, 1, …, n-1`). Bounds are evaluated once; nothing runs if `a >= b`. The variable is an integer type or `oto` (`sayi`); modifying it in the body does not affect the next iteration |

### `harita[K, V]` Methods and Operations
| Usage | Return Type | Description |
| :--- | :--- | :--- |
| `H.koy(key, val)` | `bos` | Inserts or updates a key-value pair (preserving insertion order) |
| `H.al(key)` | `V` | Returns the value associated with `key`, or the zero/default value of `V` if absent |
| `H.var_mi(key)` | `mantik` | Returns `dogru` if `key` exists in the map |
| `H.sil(key)` | `bos` | Removes `key` and its value from the map |
| `H.uzunluk()` | `sayi` | Returns the number of entries in the map |
| `H.anahtarlar()` | `liste[K]` | Returns all keys as a list in insertion order |
| `H.temizle()` / `H.bosalt()` | `bos` | Clears all entries / releases backing memory early |
| `her (oto k in H)` | — | Iterates over keys in insertion order |

---

## 12. JSON Operations (`upp.json`)

> *Available on hosted (Windows / Linux) targets only.*

| Function | Signature | Description |
| :--- | :--- | :--- |
| `ayristir(str)` / `olustur(node)` | `(metin) -> JSONDeger` / `(JSONDeger) -> metin` | Parses a JSON string / serializes a `JSONDeger` tree to a string |
| `tur(node)` | `(JSONDeger) -> metin` | Returns the node kind: `"nesne"`, `"dizi"`, `"metin"`, `"sayi"`, `"mantik"`, or `"bosluk"` |
| `var_mi(node)` / `uzunluk(node)` | `(JSONDeger) -> mantik` / `sayi` | Checks if a node handle is valid / returns array or object length |
| `anahtar_al(node, key)` | `(JSONDeger, metin) -> JSONDeger` | Retrieves a field from a JSON object by key |
| `anahtarlar(node)` | `(JSONDeger) -> JSONDeger` | Returns a JSON array containing all keys of an object |
| `indeks_al(node, i)` | `(JSONDeger, sayi) -> JSONDeger` | Retrieves the `i`-th element of a JSON array |
| `metin_al(n)`, `sayi_al(n)`, `ondalik_al(n)`, `mantik_al(n)` | `(JSONDeger) -> ...` | Extracts the primitive value stored in a JSON node |
| `nesne()`, `dizi()`, `yok_yap()` | `() -> JSONDeger` | Creates a new empty JSON object, array, or `null` node |
| `metin_yap(s)`, `sayi_yap(n)`, `ondalik_yap(d)`, `mantik_yap(b)` | `(...) -> JSONDeger` | Wraps a primitive value into a new JSON node |
| `koy(obj, key, val)` | `(JSONDeger, metin, JSONDeger) -> bos` | Sets a property on a JSON object |
| `ekle(arr, val)` | `(JSONDeger, JSONDeger) -> bos` | Appends an element to a JSON array |
| `bosalt(root)` | `(JSONDeger) -> bos` | Frees the memory of a JSON tree |

---

## 13. Concurrency (`arkaplan` and `Kilit`)

> *Available on hosted (Windows / Linux) targets only.*

```upp
Kilit k;
oto gorev = arkaplan {
    k.kilitle();
    upp.satir_yaz("Thread-safe background execution");
    k.birak();
};
gorev.bekle();
```

- `arkaplan { ... }` spawns a native OS thread and returns a task handle supporting `.bekle()` (`join`).
- Outer numeric and `metin` variables referenced inside the block are deep-copied into the thread (`Kilit` mutex instances are shared by reference).
- Collections (`liste`, `harita`), `beklenen`, `JSONDeger`, and views (`&metin`, `&dilim`) cannot be captured inside `arkaplan` blocks.

---

## 14. Kernel, Hardware, and Byte Buffers (`upp.cekirdek`, `upp.tampon`)

### `upp.cekirdek.*` *(`--hedef serbest` and `--hedef cekirdek` Only)*

Low-level primitives that directly touch CPU state, I/O ports, or raw physical memory require an explicit **`guvensiz`** block:

| Function (Requires `guvensiz`) | Signature | Description |
| :--- | :--- | :--- |
| `port_yaz8 / 16 / 32(port, val)` | `(sayi, sayi) -> bos` | Writes to an x86 hardware I/O port (`outb`, `outw`, `outl`) |
| `port_oku8 / 16 / 32(port)` | `(sayi) -> sayi` | Reads from an x86 hardware I/O port (`inb`, `inw`, `inl`) |
| `bellek_yaz8 / 16 / 32 / 64(addr, val)` | `(sayi, sayi) -> bos` | Performs a `volatile` MMIO write to physical `addr` (through the HHDM under Limine; a higher-half kernel address is used as is) |
| `bellek_oku8 / 16 / 32 / 64(addr)` | `(sayi) -> sayi` | Performs a `volatile` MMIO read from physical `addr` (through the HHDM under Limine) |
| `dur()`, `kesme_kapat()`, `kesme_ac()` | `() -> bos` | Privileged CPU instructions: `hlt` (halt), `cli` (disable interrupts), `sti` (enable interrupts) |
| `yigin_kur(addr, size)` | `(sayi, sayi) -> bos` | Assigns a memory region to the freestanding heap allocator |

The remaining kernel services can be called directly from **safe code**:

| Function (Safe) | Signature | Description |
| :--- | :--- | :--- |
| `kesmeler_kur()` | `() -> bos` | Initializes the 48-entry IDT, PIC, and 1000 Hz PIT timer (called automatically on demand) |
| `tik()` | `() -> sayi` | Returns elapsed 1 ms PIT timer ticks since boot |
| `tus_oku()` | `() -> sayi` | Non-blocking read of one code point from the PS/2 keyboard (Turkish Q layout) or COM1 serial port (`0` if none). Special keys arrive as ANSI sequences (`ESC [ A`, etc.) |
| `tus_bekle()` | `() -> sayi` | Waits for a key press while yielding CPU time to other ready tasks |
| `bellek_toplam()` | `() -> sayi` | Usable RAM reported by the Limine/Multiboot memory map (in bytes) |
| `yigin_boyut()` / `yigin_kullanilan()` | `() -> sayi` | Total capacity and currently used bytes of the kernel heap |
| `gorev_baslat(fn, arg, name)` | `(fonk(sayi)->bos, sayi, metin) -> sayi` | Spawns a cooperative task with a 64 KiB stack running `fn(arg)` and returns its task ID (`-1` if full) |
| `sira_ver()` | `() -> bos` | Yields execution to the next ready cooperative task |
| `gorev_kimlik()` | `() -> sayi` | Returns the ID of the currently running task (`1` for the main boot task) |
| `gorev_listesi()` | `() -> liste[sayi]` | Returns a list of active task IDs |
| `gorev_ad(id)` / `gorev_durum(id)` | `(sayi) -> metin` | Returns the task name / status (`"çalışıyor"`, `"hazır"`, `"uyuyor"`, or `""`) |
| `gorev_baslama(id)` | `(sayi) -> sayi` | Returns the start time of task `id` in milliseconds |
| `gorev_oldur(id)` | `(sayi) -> mantik` | Terminates task `id` (the main task `1` cannot be killed) |
| `tarih()` | `() -> metin` | Reads the CMOS real-time clock formatted as `"YYYY-MM-DD HH:MM:SS"` |
| `islemci()` | `() -> metin` | Returns the CPUID processor brand string |
| `ekran_temizle()` | `() -> bos` | Clears the screen console (VGA text or framebuffer) and serial terminal |
| `onyukleyici()` | `() -> metin` | The bootloader that started the kernel: name and version such as `"Limine 11.4.1"`, `"Multiboot 1"`, or `""` if unknown |
| `ekran()` | `() -> metin` | Description of the screen console: `"çerçeve tamponu 1280x800x32, 160x50 karakter"` or `"VGA metin 80x25"` |
| `kapat()` / `yeniden_baslat()` | `() -> bos` | Powers off the machine via ACPI (or QEMU `isa-debug-exit`) / resets via the 8042 controller |
| `disk_var(d)` | `(sayi) -> mantik` | Returns `dogru` if ATA IDE drive `d` (`0..3`) is present |
| `disk_sektor(d)` / `disk_model(d)` | `(sayi) -> sayi` / `metin` | Returns the total 512-byte sector count and model string of drive `d` |
| `disk_oku(d, lba, count, buf, ofs)` | `(sayi, sayi, sayi, sayi, sayi) -> mantik` | Reads `count` sectors starting at `lba` into `upp.tampon` handle `buf` at byte offset `ofs` |
| `disk_yaz(d, lba, count, buf, ofs)` | `(sayi, sayi, sayi, sayi, sayi) -> mantik` | Writes `count` sectors from `buf` (`ofs`) to disk `d` and flushes the drive cache (LBA28/LBA48 PIO) |

#### User mode (ring 3) *(`--onyukleyici limine` only)*

The mechanism lives in the kernel (C) and the policy in the operating system (u++): the process table, ELF loading, and the meaning of each system call are written in u++. Uppix uses these with the Linux x86_64 ABI (see [`src/os/README.md`](../src/os/README.md)). `ka` is an address-space handle (the physical address of its PML4). `koruma` uses Linux `PROT_*` bits: 1 read, 2 write, 4 execute.

| Function | Signature | Description |
| :--- | :--- | :--- |
| `kullanici_destekli()` | `() -> mantik` | Whether user mode is set up (Limine/HHDM) |
| `kullanici_alan_yeni()` / `kullanici_alan_sil(ka)` | `() -> sayi` / `(sayi) -> bos` | New address space with an empty lower half (`0` if out of memory) / delete it with all its pages |
| `kullanici_esle(ka, addr, len, prot)` | `(sayi, sayi, sayi, sayi) -> mantik` | Maps the range with fresh zeroed pages; permissions of already mapped pages are merged |
| `kullanici_esleme_kaldir(ka, addr, len)` | `(sayi, sayi, sayi) -> bos` | Unmaps pages and releases the process's frames |
| `kullanici_koruma(ka, addr, len, prot, merge)` | `(sayi, sayi, sayi, sayi, mantik) -> bos` | Sets permissions (the mprotect mechanism); `merge` only adds |
| `kullanici_bos_mu(ka, addr, len)` | `(sayi, sayi, sayi) -> mantik` | `dogru` if no page in the range is mapped |
| `kullanici_esli_mi(ka, addr, len)` | `(sayi, sayi, sayi) -> mantik` | `dogru` if every page in the range is mapped (mprotect/madvise `ENOMEM` check) |
| `kullanici_fiziksel_esle(ka, addr, phys, len, prot)` | `(sayi, sayi, sayi, sayi, sayi) -> mantik` | Maps device memory (e.g. the framebuffer) write-combining. *Requires `guvensiz`* |
| `kullanici_oku(ka, addr, buf, ofs, n)` / `kullanici_yaz(...)` | `(sayi, sayi, sayi, sayi, sayi) -> mantik` | User memory ↔ `upp.tampon`. Every page's permissions are checked; `yanlis` on a bad address (EFAULT) |
| `kullanici_metin(ka, addr, max)` | `(sayi, sayi, sayi) -> metin` | NUL-terminated user string; `yok` on a bad address or at the limit |
| `kullanici_calistir(ka, entry, stack)` | `(sayi, sayi, sayi) -> sayi` | Runs the process in ring 3 on this task; returns its exit code (128 + signal when killed) |
| `kullanici_cikis_iste(code)` | `(sayi) -> bos` | The process exits with `code` when the system call handler returns (exit) |
| `kullanici_tls_ayarla(segment, base)` / `kullanici_tls(segment)` | `(sayi, sayi) -> mantik` / `(sayi) -> sayi` | FS (`0`) or GS (`1`) base of the current process (arch_prctl, TLS); restored on task switch. `yanlis` if the base is not in user space |
| `kullanici_on_plan(v)` | `(mantik) -> bos` | Whether the process is in the foreground (Ctrl-C kills it with SIGINT) |
| `kullanici_son_hata()` | `() -> metin` | Why the last process ended (`"Bölümleme hatası (SIGSEGV): ..."`, `"^C"`; `""` on a normal exit) |
| `sistem_cagrisi_kur(f)` | `(fonk(sayi,sayi,sayi,sayi,sayi,sayi,sayi)->sayi) -> bos` | `SYSCALL` handler: `f(no, a1, ..., a6)`; the result goes to rax (errors: `-errno`) |
| `ekran_bilgi(field)` | `(sayi) -> sayi` | 0 framebuffer physical address, 1 size, 2-3 width/height in pixels, 4 pitch, 5 bpp, 6-11 color fields, 12-13 columns/rows |
| `cerceve_bilgi(field)` | `(sayi) -> sayi` | User frame pool: 0 total, 1 used, 2 free (bytes) |

> [!NOTE]
> **Kernel Runtime Environment:**
> - **Heap Placement:** At boot, the freestanding heap automatically relocates to the largest available RAM region in the memory map (within the first 1 GiB under Multiboot; under Limine three quarters of the largest region, the remaining quarter becoming the user-process frame pool), falling back to a 4 MiB `.bss` arena if no memory map is provided.
> - **Console:** Under Multiboot the VGA text driver maps Turkish characters to CP437; under Limine a text console is drawn onto the framebuffer with the Terminus 8x16 font (Turkish characters included). Both render box-drawing and shade characters (`├ └ │ ─ █ ░ ▒ ▓`) and interpret ANSI escape sequences: color (`\e[1;32m`), clearing (`\e[2J`, `\e[K`), cursor positioning (`\e[H`, `\e[row;colH`, `\e[nC`/`\e[nD`), and hiding/showing the cursor (`\e[?25l` / `\e[?25h`).
> - **Limine:** With `--onyukleyici limine` the kernel is an ELF64 linked at `0xffffffff80000000` (`-mcmodel=kernel`). At boot it uses Limine protocol (base revision 6) requests for the memory map, framebuffer, HHDM and kernel address, then installs its own page tables: the kernel image and all physical memory (HHDM) live in the upper half, leaving the lower half empty for user processes. It sets up the GDT/TSS, `SYSCALL`, SSE, and NX/SMEP/SMAP, and disables the local APIC so 8259 PIC interrupts are delivered.
> - **Exceptions:** CPU exceptions (divide error, page fault, etc.) print a Turkish exception diagnostic with `RIP` and `CR2` and exit QEMU with code `3`. An exception in ring 3 only terminates that process.

### `upp.tampon.*` — Bounds-Checked Byte Buffers *(All Targets)*

Designed for parsing and constructing binary structures (disk sectors, `ext4` / `FAT32` / `exFAT` on-disk structures, network packets) entirely from safe code. Each buffer is referenced by a `sayi` handle, and **every read/write is bounds-checked** (panicking on invalid handles or out-of-bounds offsets). Multi-byte integers use **little-endian** byte order.

| Function | Signature | Description |
| :--- | :--- | :--- |
| `upp.tampon.yeni(size)` | `(sayi) -> sayi` | Allocates a zero-initialized byte buffer of `size` bytes and returns its handle |
| `upp.tampon.birak(t)` | `(sayi) -> bos` | Frees the byte buffer associated with handle `t` |
| `upp.tampon.boyut(t)` | `(sayi) -> sayi` | Returns the size of buffer `t` in bytes |
| `upp.tampon.oku8 / 16 / 32 / 64(t, ofs)` | `(sayi, sayi) -> sayi` | Reads a little-endian integer at byte offset `ofs` (`8/16/32`-bit reads zero-extend; `64`-bit returns the raw bit pattern) |
| `upp.tampon.yaz8 / 16 / 32 / 64(t, ofs, val)` | `(sayi, sayi, sayi) -> bos` | Writes a little-endian integer at byte offset `ofs` |
| `upp.tampon.metin(t, ofs, n)` | `(sayi, sayi, sayi) -> metin` | Copies `n` bytes starting at `ofs` into a managed UTF-8 `metin` |
| `upp.tampon.metin_yaz(t, ofs, s)` | `(sayi, sayi, metin) -> sayi` | Copies the UTF-8 bytes of `s` into `t` at `ofs` and returns the number of bytes written |
| `upp.tampon.utf16_metin(t, ofs, n)` | `(sayi, sayi, sayi) -> metin` | Decodes up to `n` UTF-16LE 16-bit code units (stopping at `0x0000`, surrogate pairs supported) into a UTF-8 `metin` |
| `upp.tampon.utf16_yaz(t, ofs, s, limit)` | `(sayi, sayi, metin, sayi) -> sayi` | Encodes `s` as UTF-16LE (up to `limit` 16-bit code units) and returns the number of code units written |
| `upp.tampon.kopyala(dst, dofs, src, sofs, n)` | `(sayi x 5) -> bos` | Copies `n` bytes between buffers (overlap-safe when `dst == src`) |
| `upp.tampon.doldur(t, ofs, n, byte)` | `(sayi, sayi, sayi, sayi) -> bos` | Fills `[ofs, ofs + n)` with `byte` |
| `upp.tampon.crc32c(t, ofs, n, seed)` | `(sayi, sayi, sayi, sayi) -> sayi` | Computes Castagnoli **CRC32C** over `n` bytes without final XOR (`ext4` convention; XOR with `4294967295` for standard CRC32C) |

---

## 15. Compilation Targets

| Target Flag | Output | Description |
| :--- | :--- | :--- |
| *(default)* / `--hedef yerel` | `.exe` / `.out` | Standalone native executable for the host OS |
| `--hedef windows` / `--hedef linux` | `.exe` / `.out` | Target-specific OS ABI and platform namespace selection |
| `--hedef serbest` | ELF `.o` | Freestanding object file without `libc` (entry point: `upp_cekirdek_giris()`) |
| `--hedef cekirdek` | `<name>.elf` + `<name>32.elf` | Bootable Multiboot 1 x86_64 kernel image (automatically launches QEMU unless `--sadece-derle` is passed) |
| `--hedef cekirdek --onyukleyici limine` | `<name>.elf` | Higher-half ELF64 kernel booted from disk in 64-bit mode by Limine (BIOS and UEFI) |

> [!WARNING]
> On `--hedef serbest` and `--hedef cekirdek` targets, hosted OS modules (`upp.dosya`, `upp.yol`, `upp.sistem`, `upp.json`) as well as `arkaplan` and `Kilit` are unavailable and produce a compile-time error if used.

---

## 16. Command Line (`uppc`) and `upp.proje`

```bash
# Single-file compilation and execution
uppc <file.upp> [-o name] [--sadece-derle | -c] [--hedef target] [--analiz] [--json-hata]
                [--bicim | --bicim-kontrol] [--ast] [--runtime path] [--disk image]...
                [--onyukleyici multiboot|limine] [--disk-hazirla script.sh]
                [--guvenlik-denetimi[=is]]

# Project management and self-hosting
uppc yeni <name> [--hedef cekirdek]   # Scaffold a new project (upp.proje + kaynak/ana.upp)
uppc derle [dir]                      # Build the project according to upp.proje
uppc calistir [dir]                   # Build and run the project (launches QEMU on kernel target)
uppc asistan [dir]                    # Launch the interactive terminal build assistant
uppc kendini-derle [--hizli]          # Rebuild the compiler with itself and update bootstrap/uppc.c
```

### `upp.proje` Configuration Format

```ini
ad = merhaba
giris = kaynak/ana.upp
hedef = yerel                 # yerel | windows | linux | serbest | cekirdek
cikti = derleme/merhaba
bayraklar =                   # Optional extra flags passed to uppc
diskler = a.img, b.img        # Kernel target: raw IDE disk images attached to QEMU (sda, sdb...)
disk_hazirla = disk_yap.sh    # Script executed via sh in the project directory if any disk image is missing
onyukleyici = limine         # Boot the kernel from the first disk with Limine; disk_hazirla then runs after every build
```

- Passing `--disk <image>` (up to 4 times) on the `--hedef cekirdek` target attaches raw disk images to QEMU as `-drive if=ide,format=raw`.
- `--guvenlik-denetimi` builds the program with sanitizers. On Linux this enables ASan + UBSan: out-of-bounds and use-after-free accesses, leaks, and undefined behavior stop at the first error with the source line. `--guvenlik-denetimi=is` instead uses TSan to look for races between `arkaplan` threads. Windows/MinGW has no ASan runtime, so only UBSan trap mode is enabled and undefined behavior stops with an explanatory message. The flag is not available for the serbest and cekirdek targets. The older `UPP_SANITIZE` environment variable (raw gcc flags) still works.
- `--onyukleyici limine` builds the kernel for the Limine protocol. QEMU boots it from the first disk without `-kernel`, so the first `--disk` must be a bootable image.
- `--disk-hazirla <script>` runs `sh <script> <directory of first disk> <kernel.elf>` after the kernel is built (inside WSL on Windows). The script creates the bootable image or updates the kernel inside it; see [`src/os/disk_yap.sh`](../src/os/disk_yap.sh). The `UPP_FS_ARACLAR` environment variable is passed to the script as `FS_ARACLAR`.
