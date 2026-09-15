# -*- coding: utf-8 -*-
"""Aşama-0 bootstrap C: hazırla (yerel transpile'dan) ve gcc ile derle."""
from __future__ import annotations

import os
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
KAYNAK_C = os.path.join(ROOT, "derleyici", "uppc_yeni.c")
BOOTSTRAP_C = os.path.join(ROOT, "bootstrap", "uppc.c")

BASLIK = r"""/* u++ aşama-0 bootstrap C.
 * Temiz checkout: gcc bootstrap/uppc.c → derleyici/uppc[.exe]
 * Kaynak gerçeklik: src/uppc/*.upp — bu dosya son başarılı transpile kopyasıdır.
 */
#if defined(_WIN32)
#define UPP_HEDEF_WINDOWS 1
#else
#ifndef UPP_HEDEF_LINUX
#define UPP_HEDEF_LINUX 1
#endif
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <math.h>
#if defined(_WIN32)
#include <windows.h>
#include <mmsystem.h>
#include <tlhelp32.h>
#include <io.h>
#include <sys/stat.h>
#else
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

"""


def hazirla() -> int:
    if not os.path.isfile(KAYNAK_C):
        print("[bootstrap] yerel transpile yok:", KAYNAK_C)
        print("[bootstrap] bootstrap/uppc.c zaten repoda; gcc ile derleyin.")
        return 1
    ham = open(KAYNAK_C, encoding="utf-8", errors="replace").read()
    isaret = "/* ====================================================================="
    i = ham.find(isaret)
    if i < 0:
        print("[bootstrap] runtime işareti bulunamadı")
        return 1
    os.makedirs(os.path.dirname(BOOTSTRAP_C), exist_ok=True)
    with open(BOOTSTRAP_C, "w", encoding="utf-8", newline="\n") as f:
        f.write(BASLIK)
        f.write(ham[i:])
    print("[bootstrap] yazıldı:", BOOTSTRAP_C, "bayt", os.path.getsize(BOOTSTRAP_C))
    return 0


def gcc_bul() -> str | None:
    env = os.environ.get("UPP_GCC")
    if env and os.path.isfile(env):
        return env
    which = shutil.which("gcc")
    if which:
        return which
    aday = [
        r"C:\msys64\ucrt64\bin\gcc.exe",
        r"C:\msys64\mingw64\bin\gcc.exe",
        "/usr/bin/gcc",
    ]
    for p in aday:
        if os.path.isfile(p):
            return p
    return None


def derle() -> int:
    if not os.path.isfile(BOOTSTRAP_C):
        print("[bootstrap] eksik:", BOOTSTRAP_C)
        return 1
    gcc = gcc_bul()
    if not gcc:
        print("[bootstrap] gcc bulunamadı")
        return 1
    out_dir = os.path.join(ROOT, "derleyici")
    os.makedirs(out_dir, exist_ok=True)
    if sys.platform == "win32":
        out = os.path.join(out_dir, "uppc.exe")
        cmd = [gcc, "-std=gnu11", "-O2", BOOTSTRAP_C, "-o", out, "-luser32", "-lwinmm", "-lgdi32"]
    else:
        out = os.path.join(out_dir, "uppc")
        cmd = [gcc, "-std=gnu11", "-O2", BOOTSTRAP_C, "-o", out, "-pthread", "-lm"]
    print("[bootstrap]", " ".join(cmd))
    r = subprocess.run(cmd)
    if r.returncode != 0:
        return r.returncode
    print("[bootstrap] ikili:", out)
    return 0


def main(argv: list[str] | None = None) -> int:
    args = list(sys.argv[1:] if argv is None else argv)
    if not args or args[0] in ("-h", "--yardim"):
        print("kullanım: py araclar/bootstrap.py hazirla|derle")
        return 0
    if args[0] == "hazirla":
        return hazirla()
    if args[0] == "derle":
        return derle()
    print("[bootstrap] bilinmeyen komut:", args[0])
    return 1


if __name__ == "__main__":
    sys.exit(main())
