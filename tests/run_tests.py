#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""u++ dosya tabanlı test runner (Native derleyici).

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
  python tests/run_tests.py --kategori safety
"""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
from dataclasses import dataclass, field
from typing import List, Optional, Tuple

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TESTS = os.path.dirname(os.path.abspath(__file__))

KATEGORILER = ("positive", "negative", "safety", "ozel")
KATEGORI_BASLIK = {
    "positive": "Pozitif / E2E",
    "negative": "Negatif / derleyici hataları",
    "safety": "Güvenlik ihlalleri",
    "ozel": "Özel / Beta Özellikleri",
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


def native_uppc_yol() -> Optional[str]:
    for klasor in ("derleyici", "derleme"):
        for ad in ("uppc.exe", "uppc", "uppc.out"):
            p = os.path.join(ROOT, klasor, ad)
            if os.path.isfile(p):
                return p
    return None


def calistir_pozitif(uppc: str, yol: str, kaynak: str, meta: Meta, cikti_dir: str, timeout: float) -> Sonuc:
    ad = os.path.relpath(yol, TESTS)
    if meta.stdout is None:
        return Sonuc("positive", ad, False, "beklenen stdout yok (// @stdout veya .stdout)")
    os.makedirs(cikti_dir, exist_ok=True)
    kok = os.path.splitext(os.path.basename(yol))[0]
    stem = os.path.join(cikti_dir, kok)
    try:
        der = subprocess.run(
            [uppc, yol, "--sadece-derle", "--cikti", stem],
            cwd=ROOT,
            capture_output=True,
            timeout=max(timeout, 60.0),
            encoding="utf-8",
            errors="replace",
        )
    except subprocess.TimeoutExpired:
        return Sonuc("positive", ad, False, f"derleme zaman aşımı ({timeout:.0f}s)")
    except OSError as err:
        return Sonuc("positive", ad, False, str(err))

    if der.returncode != 0:
        msg = (der.stderr or der.stdout or "").strip()
        return Sonuc("positive", ad, False, f"derleme başarısız:\n{msg}")

    exe_yol = stem + (".exe" if sys.platform == "win32" else ".out")
    if not os.path.isfile(exe_yol):
        alt = stem if os.path.isfile(stem) else None
        if alt is None:
            return Sonuc("positive", ad, False, f"üretilen ikili yok: {exe_yol}")
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
        return Sonuc("positive", ad, False, f"çalıştırma zaman aşımı ({timeout:.0f}s)")
    except OSError as err:
        kod = getattr(err, "winerror", None)
        if kod == 4551 or "Uygulama Denetimi" in str(err) or "Application Control" in str(err):
            return Sonuc("positive", ad, True, "exe atlandı (Uygulama Denetimi)", atlandi=True)
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


def calistir_hata(
    uppc: str,
    yol: str,
    kaynak: str,
    meta: Meta,
    kategori: str,
    varsayilan_icerir: str,
    timeout: float = 30.0,
) -> Sonuc:
    ad = os.path.relpath(yol, TESTS)
    aranan = list(meta.icerir) if meta.icerir else [varsayilan_icerir]
    try:
        der = subprocess.run(
            [uppc, yol, "--sadece-c"],
            cwd=ROOT,
            capture_output=True,
            timeout=timeout,
            encoding="utf-8",
            errors="replace",
        )
    except subprocess.TimeoutExpired:
        return Sonuc(kategori, ad, False, f"derleme zaman aşımı ({timeout:.0f}s)")
    except OSError as err:
        return Sonuc(kategori, ad, False, str(err))

    metin = (der.stderr or "") + "\n" + (der.stdout or "")
    if der.returncode == 0:
        return Sonuc(kategori, ad, False, "derleme başarılı oldu; hata bekleniyordu")

    eksik = [p for p in aranan if p not in metin]
    if eksik:
        return Sonuc(
            kategori,
            ad,
            False,
            "çıktıda yok: " + ", ".join(eksik) + f"\n  bulunan:\n{metin}",
        )
    return Sonuc(kategori, ad, True)


def dosya_calistir(uppc: str, yol: str, kategori: str, cikti_dir: str, timeout: float) -> Sonuc:
    with open(yol, encoding="utf-8") as handle:
        kaynak = handle.read()
    if kaynak.startswith("\ufeff"):
        kaynak = kaynak[1:]
    meta = meta_oku(kaynak, yol)
    if kategori in ("positive", "ozel"):
        return calistir_pozitif(uppc, yol, kaynak, meta, cikti_dir, timeout)
    if kategori == "negative":
        return calistir_hata(uppc, yol, kaynak, meta, kategori, "[u++ HATA]", timeout)
    if kategori == "safety":
        return calistir_hata(uppc, yol, kaynak, meta, kategori, "BELLEK GÜVENLİĞİ", timeout)
    return Sonuc(kategori, yol, False, f"bilinmeyen kategori: {kategori}")


def raporla(sonuclar: List[Sonuc]) -> int:
    print()
    print(kalin("u++ Native Test Raporu"))
    print(kalin("=" * 50))
    grup: dict = {}
    for s in sonuclar:
        grup.setdefault(s.kategori, []).append(s)
    for kat in list(KATEGORILER):
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
    parser = argparse.ArgumentParser(description="u++ tests/ native test runner")
    parser.add_argument(
        "--kategori",
        choices=["positive", "negative", "safety", "ozel", "hepsi"],
        default="hepsi",
        help="yalnız bu kategoriyi çalıştır (varsayılan: hepsi)",
    )
    parser.add_argument("--uppc", default="", help="Özel native uppc yolu")
    parser.add_argument("--timeout", type=float, default=30.0, help="pozitif exe zaman aşımı (s)")
    args = parser.parse_args(argv)

    uppc = args.uppc or native_uppc_yol()
    if not uppc or not os.path.isfile(uppc):
        print(kirmizi(f"[HATA] Native uppc derleyicisi bulunamadı: {uppc or 'derleyici/uppc.exe'}"))
        print(soluk("Lütfen önce derleyiciyi derleyin."))
        return 1

    cikti_dir = os.path.join(TESTS, "_cikti")
    secilen = ["positive", "negative", "safety", "ozel"] if args.kategori == "hepsi" else [args.kategori]
    sonuclar: List[Sonuc] = []

    for kat in secilen:
        if kat == "ozel":
            beta_test = os.path.join(TESTS, "test_beta_ozellikleri.upp")
            if os.path.isfile(beta_test):
                sonuclar.append(dosya_calistir(uppc, beta_test, "ozel", cikti_dir, args.timeout))
            continue
        klasor = os.path.join(TESTS, kat)
        uzantilar: Tuple[str, ...] = (".upp", ".uph") if kat == "negative" else (".upp",)
        dosyalar = upp_dosyalari(klasor, uzantilar)
        if not dosyalar:
            sonuclar.append(Sonuc(kat, f"{kat}/ (dosya yok)", False, "hiç .upp bulunamadı"))
            continue
        for yol in dosyalar:
            sonuclar.append(dosya_calistir(uppc, yol, kat, cikti_dir, args.timeout))

    return raporla(sonuclar)


if __name__ == "__main__":
    sys.exit(main())
