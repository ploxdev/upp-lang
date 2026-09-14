#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""u++ dosya tabanlı test runner.

Kategoriler (tests/ altında otomatik taranır):
  positive/  derle + çalıştır; stdout @stdout ile kıyaslanır
  negative/  derleme [u++ HATA] vermeli
  safety/    derleme [u++ BELLEK GÜVENLİĞİ İHLALİ] vermeli

Kaynak yorumları:
  // @stdout          sonraki // satırlar beklenen çıktı
  // @icerir: METİN   derleyici çıktısında bulunması gereken alt dize

Kullanım:
  python tests/run_tests.py
  python tests/run_tests.py --kategori positive
  python tests/run_tests.py --no-birim
"""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
import tempfile
import traceback
import unittest
from dataclasses import dataclass, field
from typing import List, Optional, Tuple

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TESTS = os.path.dirname(os.path.abspath(__file__))
if ROOT not in sys.path:
    sys.path.insert(0, ROOT)

from uppc import (  # noqa: E402
    SafetyError,
    UppError,
    compile_source,
    find_gcc,
    host_hedef,
    invoke_gcc,
)


KATEGORILER = ("positive", "negative", "safety")
KATEGORI_BASLIK = {
    "positive": "Pozitif / E2E",
    "negative": "Negatif / derleyici hataları",
    "safety": "Güvenlik ihlalleri",
    "parite": "Parite (Python ve native aynı stdout)",
    "birim": "Birim (tests/test_uppc.py)",
}


def _ansi_hazir() -> bool:
    if os.environ.get("NO_COLOR"):
        return False
    if not sys.stdout.isatty():
        return False
    if sys.platform == "win32":
        try:
            import ctypes

            kernel32 = ctypes.windll.kernel32
            handle = kernel32.GetStdHandle(-11)
            mode = ctypes.c_uint()
            if kernel32.GetConsoleMode(handle, ctypes.byref(mode)):
                kernel32.SetConsoleMode(handle, mode.value | 0x0004)
        except Exception:
            return True
    return True


_RENK = _ansi_hazir()


def _r(kod: str, metin: str) -> str:
    if not _RENK:
        return metin
    return f"\033[{kod}m{metin}\033[0m"


def yesil(m: str) -> str:
    return _r("32;1", m)


def kirmizi(m: str) -> str:
    return _r("31;1", m)


def cyan(m: str) -> str:
    return _r("36;1", m)


def soluk(m: str) -> str:
    return _r("2", m)


def kalin(m: str) -> str:
    return _r("1", m)


def norm_cikti(s: str) -> str:
    s = (s or "").replace("\r\n", "\n").replace("\r", "\n")
    satirlar = [ln.rstrip() for ln in s.split("\n")]
    while satirlar and satirlar[-1] == "":
        satirlar.pop()
    return "\n".join(satirlar)


@dataclass
class Meta:
    stdout: Optional[str] = None
    icerir: List[str] = field(default_factory=list)


def meta_oku(kaynak: str, yol: str) -> Meta:
    meta = Meta()
    stdout_satir: List[str] = []
    stdout_mod = False
    yan_stdout = os.path.splitext(yol)[0] + ".stdout"
    for ham in kaynak.splitlines():
        s = ham.strip()
        if s.startswith("// @stdout"):
            stdout_mod = True
            kalan = s[len("// @stdout") :].lstrip()
            if kalan.startswith(":"):
                kalan = kalan[1:].lstrip()
            if kalan:
                stdout_satir.append(kalan)
            continue
        if s.startswith("// @icerir:"):
            stdout_mod = False
            parca = s.split(":", 1)[1].strip()
            if parca:
                meta.icerir.append(parca)
            continue
        if stdout_mod:
            if s.startswith("//"):
                govde = s[2:]
                if govde.startswith(" "):
                    govde = govde[1:]
                if govde.startswith("@"):
                    stdout_mod = False
                    continue
                stdout_satir.append(govde)
                continue
            if s == "":
                continue
            stdout_mod = False
    if os.path.isfile(yan_stdout):
        with open(yan_stdout, encoding="utf-8") as handle:
            meta.stdout = handle.read()
    elif stdout_satir:
        meta.stdout = "\n".join(stdout_satir) + "\n"
    return meta


@dataclass
class Sonuc:
    kategori: str
    ad: str
    ok: bool
    ayrinti: str = ""
    atlandi: bool = False


def upp_dosyalari(klasor: str, uzantilar: Tuple[str, ...] = (".upp",)) -> List[str]:
    if not os.path.isdir(klasor):
        return []
    out: List[str] = []
    for kok, _alts, dosyalar in os.walk(klasor):
        for ad in dosyalar:
            if ad.startswith("_"):
                continue
            k = ad.lower()
            if any(k.endswith(u) for u in uzantilar):
                out.append(os.path.join(kok, ad))
    out.sort()
    return out


def derle_kaynak(yol: str, kaynak: str) -> Tuple[Optional[str], Optional[str]]:
    """(c_kodu, hata_metni). Başarılıysa hata None."""
    try:
        c_kodu = compile_source(kaynak, yol, host_hedef())
        return c_kodu, None
    except (UppError, SafetyError) as err:
        return None, str(err)
    except Exception:
        return None, traceback.format_exc()


def calistir_pozitif(yol: str, kaynak: str, meta: Meta, cikti_dir: str, timeout: float) -> Sonuc:
    ad = os.path.relpath(yol, TESTS)
    if meta.stdout is None:
        return Sonuc("positive", ad, False, "beklenen stdout yok (// @stdout veya .stdout)")
    gcc = find_gcc()
    if gcc is None:
        return Sonuc("positive", ad, False, "GCC bulunamadı (MinGW-w64 PATH)")
    c_kodu, hata = derle_kaynak(yol, kaynak)
    if hata:
        return Sonuc("positive", ad, False, f"derleme beklenmedik hata:\n{hata}")
    assert c_kodu is not None
    os.makedirs(cikti_dir, exist_ok=True)
    kok = os.path.splitext(os.path.basename(yol))[0]
    c_yol = os.path.join(cikti_dir, kok + ".c")
    exe_yol = os.path.join(cikti_dir, kok + (".exe" if sys.platform == "win32" else ".out"))
    with open(c_yol, "w", encoding="utf-8", newline="\n") as handle:
        handle.write(c_kodu)
    try:
        gcc_son = invoke_gcc(c_yol, exe_yol, host_hedef())
    except UppError as err:
        return Sonuc("positive", ad, False, str(err))
    if gcc_son.returncode != 0:
        msg = (gcc_son.stderr or gcc_son.stdout or "").strip()
        return Sonuc("positive", ad, False, f"GCC başarısız:\n{msg}")
    try:
        cal = subprocess.run(
            [exe_yol],
            cwd=ROOT,
            capture_output=True,
            timeout=timeout,
            encoding="utf-8",
            errors="replace",
        )
    except subprocess.TimeoutExpired:
        return Sonuc("positive", ad, False, f"zaman aşımı ({timeout:.0f}s)")
    except OSError as err:
        kod = getattr(err, "winerror", None)
        if kod == 4551 or "Uygulama Denetimi" in str(err) or "Application Control" in str(err):
            return Sonuc(
                "positive",
                ad,
                True,
                f"exe atlandı (Uygulama Denetimi); C/GCC geçti",
                atlandi=True,
            )
        return Sonuc("positive", ad, False, str(err))
    if cal.returncode != 0:
        return Sonuc(
            "positive",
            ad,
            False,
            f"çıkış kodu {cal.returncode}\nstderr:\n{cal.stderr}",
        )
    bek = norm_cikti(meta.stdout)
    bul = norm_cikti(cal.stdout)
    if bek != bul:
        return Sonuc(
            "positive",
            ad,
            False,
            f"stdout uyuşmadı\n  beklenen:\n{bek}\n  bulunan:\n{bul}",
        )
    return Sonuc("positive", ad, True)


def native_uppc_yol() -> Optional[str]:
    for ad in ("uppc.exe", "uppc", "uppc.out"):
        p = os.path.join(ROOT, "derleme", ad)
        if os.path.isfile(p):
            return p
    return None


def calistir_parite(yol: str, kaynak: str, meta: Meta, native: str, cikti_dir: str, timeout: float) -> Sonuc:
    ad = os.path.relpath(yol, TESTS)
    if meta.stdout is None:
        return Sonuc("parite", ad, False, "beklenen stdout yok")
    os.makedirs(cikti_dir, exist_ok=True)
    kok = os.path.splitext(os.path.basename(yol))[0]
    stem = os.path.join(cikti_dir, kok)
    try:
        der = subprocess.run(
            [native, yol, "--sadece-derle", "--cikti", stem],
            cwd=ROOT,
            capture_output=True,
            timeout=max(timeout, 60.0),
            encoding="utf-8",
            errors="replace",
        )
    except subprocess.TimeoutExpired:
        return Sonuc("parite", ad, False, f"native derleme zaman aşımı ({timeout:.0f}s)")
    except OSError as err:
        kod = getattr(err, "winerror", None)
        if kod == 4551 or "Uygulama Denetimi" in str(err) or "Application Control" in str(err):
            return Sonuc("parite", ad, True, "native atlandı (Uygulama Denetimi)", atlandi=True)
        return Sonuc("parite", ad, False, str(err))
    if der.returncode != 0:
        msg = (der.stderr or der.stdout or "").strip()
        return Sonuc("parite", ad, False, f"native derleme başarısız:\n{msg}")
    exe_yol = stem + (".exe" if sys.platform == "win32" else ".out")
    if not os.path.isfile(exe_yol):
        alt = stem if os.path.isfile(stem) else None
        if alt is None:
            return Sonuc("parite", ad, False, f"native ikili yok: {exe_yol}")
        exe_yol = alt
    try:
        cal = subprocess.run(
            [exe_yol],
            cwd=ROOT,
            capture_output=True,
            timeout=timeout,
            encoding="utf-8",
            errors="replace",
        )
    except subprocess.TimeoutExpired:
        return Sonuc("parite", ad, False, f"zaman aşımı ({timeout:.0f}s)")
    except OSError as err:
        kod = getattr(err, "winerror", None)
        if kod == 4551 or "Uygulama Denetimi" in str(err) or "Application Control" in str(err):
            return Sonuc("parite", ad, True, "exe atlandı (Uygulama Denetimi)", atlandi=True)
        return Sonuc("parite", ad, False, str(err))
    if cal.returncode != 0:
        return Sonuc(
            "parite",
            ad,
            False,
            f"çıkış kodu {cal.returncode}\nstderr:\n{cal.stderr}",
        )
    bek = norm_cikti(meta.stdout)
    bul = norm_cikti(cal.stdout)
    if bek != bul:
        return Sonuc(
            "parite",
            ad,
            False,
            f"stdout uyuşmadı\n  beklenen:\n{bek}\n  bulunan:\n{bul}",
        )
    return Sonuc("parite", ad, True)


def calistir_hata(
    yol: str,
    kaynak: str,
    meta: Meta,
    kategori: str,
    varsayilan_icerir: str,
) -> Sonuc:
    ad = os.path.relpath(yol, TESTS)
    aranan = list(meta.icerir) if meta.icerir else [varsayilan_icerir]
    c_kodu, hata = derle_kaynak(yol, kaynak)
    if c_kodu is not None and hata is None:
        return Sonuc(kategori, ad, False, "derleme başarılı oldu; hata bekleniyordu")
    metin = hata or ""
    eksik = [p for p in aranan if p not in metin]
    if eksik:
        return Sonuc(
            kategori,
            ad,
            False,
            "çıktıda yok: " + ", ".join(eksik) + f"\n  bulunan:\n{metin}",
        )
    return Sonuc(kategori, ad, True)


def calistir_birim() -> Sonuc:
    buf_out = os.path.join(tempfile.gettempdir(), "upp_birim_out.txt")
    try:
        loader = unittest.TestLoader()
        suite = loader.discover(TESTS, pattern="test_*.py")
        stream = open(buf_out, "w", encoding="utf-8")
        try:
            runner = unittest.TextTestRunner(stream=stream, verbosity=1)
            sonuc = runner.run(suite)
        finally:
            stream.close()
        with open(buf_out, encoding="utf-8") as handle:
            log = handle.read()
        n_ok = sonuc.testsRun - len(sonuc.failures) - len(sonuc.errors) - len(sonuc.skipped)
        ozet = f"{n_ok}/{sonuc.testsRun} geçti"
        if sonuc.wasSuccessful():
            return Sonuc("birim", "tests/test_uppc.py", True, ozet)
        ayr = ozet + "\n" + log[-4000:]
        return Sonuc("birim", "tests/test_uppc.py", False, ayr)
    except Exception as err:
        return Sonuc("birim", "tests/test_uppc.py", False, str(err))
    finally:
        try:
            os.remove(buf_out)
        except OSError:
            pass


def dosya_calistir(yol: str, kategori: str, cikti_dir: str, timeout: float) -> Sonuc:
    with open(yol, encoding="utf-8") as handle:
        kaynak = handle.read()
    if kaynak.startswith("\ufeff"):
        kaynak = kaynak[1:]
    meta = meta_oku(kaynak, yol)
    if kategori == "positive":
        return calistir_pozitif(yol, kaynak, meta, cikti_dir, timeout)
    if kategori == "negative":
        return calistir_hata(yol, kaynak, meta, kategori, "[u++ HATA]")
    if kategori == "safety":
        return calistir_hata(yol, kaynak, meta, kategori, "BELLEK GÜVENLİĞİ")
    return Sonuc(kategori, yol, False, f"bilinmeyen kategori: {kategori}")


def raporla(sonuclar: List[Sonuc]) -> int:
    print()
    print(kalin("u++ test raporu"))
    print(kalin("=" * 50))
    grup: dict = {}
    for s in sonuclar:
        grup.setdefault(s.kategori, []).append(s)
    for kat in list(KATEGORILER) + ["parite", "birim"]:
        if kat not in grup:
            continue
        print()
        print(cyan(KATEGORI_BASLIK.get(kat, kat)))
        for s in grup[kat]:
            if s.atlandi:
                damga = cyan("Atlandı")
            elif s.ok:
                damga = yesil("Başarılı")
            else:
                damga = kirmizi("Başarısız")
            ekstra = f"  {soluk(s.ayrinti)}" if (s.ok or s.atlandi) and s.ayrinti else ""
            print(f"  {damga}  {s.ad}{ekstra}")
            if not s.ok and not s.atlandi and s.ayrinti:
                for ln in s.ayrinti.splitlines():
                    print(soluk(f"           {ln}"))
    n_ok = sum(1 for s in sonuclar if s.ok and not s.atlandi)
    n_skip = sum(1 for s in sonuclar if s.atlandi)
    n_fail = sum(1 for s in sonuclar if not s.ok and not s.atlandi)
    print()
    print(kalin("-" * 50))
    print(f"  Toplam     : {len(sonuclar)}")
    print(f"  {yesil('Başarılı')}  : {n_ok}")
    if n_skip:
        print(f"  Atlandı    : {n_skip}")
    print(f"  {kirmizi('Başarısız') if n_fail else 'Başarısız'} : {n_fail}")
    print()
    return 0 if n_fail == 0 else 1


def main(argv: Optional[List[str]] = None) -> int:
    if sys.platform == "win32":
        for stream in (sys.stdout, sys.stderr):
            try:
                stream.reconfigure(encoding="utf-8")
            except Exception:
                pass
    parser = argparse.ArgumentParser(description="u++ tests/ tarayıcısı ve runner")
    parser.add_argument(
        "--kategori",
        choices=list(KATEGORILER) + ["birim", "parite", "hepsi"],
        default="hepsi",
        help="yalnız bu kategoriyi çalıştır (varsayılan: hepsi)",
    )
    parser.add_argument("--no-birim", action="store_true", help="tests/test_uppc.py çalıştırma")
    parser.add_argument("--timeout", type=float, default=20.0, help="pozitif exe zaman aşımı (s)")
    args = parser.parse_args(argv)

    cikti_dir = os.path.join(TESTS, "_cikti")
    if args.kategori == "hepsi":
        secilen = list(KATEGORILER)
    elif args.kategori in ("birim", "parite"):
        secilen = []
    else:
        secilen = [args.kategori]
    sonuclar: List[Sonuc] = []

    for kat in secilen:
        if kat == "birim":
            continue
        klasor = os.path.join(TESTS, kat)
        uzantilar: Tuple[str, ...] = (".upp", ".uph") if kat == "negative" else (".upp",)
        dosyalar = upp_dosyalari(klasor, uzantilar)
        if not dosyalar:
            sonuclar.append(Sonuc(kat, f"{kat}/ (dosya yok)", False, "hiç .upp bulunamadı"))
            continue
        for yol in dosyalar:
            sonuclar.append(dosya_calistir(yol, kat, cikti_dir, args.timeout))

    if args.kategori in ("hepsi", "parite"):
        native = native_uppc_yol()
        if native is None:
            sonuclar.append(
                Sonuc("parite", "derleme/uppc", True, "native ikili yok; atlandı", atlandi=True)
            )
        else:
            parite_dir = os.path.join(cikti_dir, "parite")
            pozitifler = upp_dosyalari(os.path.join(TESTS, "positive"), (".upp",))
            for yol in pozitifler:
                with open(yol, encoding="utf-8") as handle:
                    kaynak = handle.read()
                meta = meta_oku(kaynak, yol)
                sonuclar.append(calistir_parite(yol, kaynak, meta, native, parite_dir, args.timeout))

    birim_iste = (args.kategori in ("hepsi", "birim")) and not args.no_birim
    if birim_iste:
        sonuclar.append(calistir_birim())

    if not sonuclar:
        print(kirmizi("Çalıştırılacak test yok."))
        return 1
    return raporla(sonuclar)


if __name__ == "__main__":
    sys.exit(main())
