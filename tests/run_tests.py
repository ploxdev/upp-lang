#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""u++ dosya tabanlı test runner (Native derleyici).

Kategoriler (tests/ altında otomatik taranır):
  positive/  derle + çalıştır; stdout @stdout ile kıyaslanır
  negative/  derleme [u++ HATA] vermeli
  safety/    derleme [u++ BELLEK GÜVENLİĞİ İHLALİ] vermeli
  runtime/   derle + çalıştır; sıfır olmayan çıkış + stderr @icerir

Kaynak yorumları:
  // @stdout          sonraki // satırlar beklenen çıktı
  // @icerir: METİN   derleyici/çalıştırma çıktısında bulunması gereken alt dize
  // @cikis: N        runtime kategorisinde beklenen süreç çıkış kodu (varsayılan 1)
  // @bayrak: ...      derleyiciye eklenecek bayraklar (ör. --hedef serbest)
  derleme/  yalnızca C üretimi başarılı olmalı (serbest/çekirdek hedefleri için)
  cekirdek/ --hedef cekirdek ile derlenip QEMU'da açılır; QEMU yoksa atlanır
  // @girdi: METİN    cekirdek/ testlerinde seri porta gönderilecek satır
  // @girdi_kacis: M  aynısı; \\e (ESC), \\xNN, \\r, \\t kaçışları çözülür
  // @gerek: araç...  cekirdek/: bu komutlar POSIX kabukta yoksa test atlanır
  // @hazirla: KOMUT  cekirdek/: derlemeden önce POSIX kabukta çalışır (ör. disk imajı)
  // @denetle: KOMUT  cekirdek/: QEMU kapandıktan sonra çalışır; 0 dışı çıkış = başarısız
  POSIX kabuk: Linux'ta sh, Windows'ta WSL. UPP_FS_ARACLAR ortam değişkeni
  (dosfstools/mtools/exfatprogs paketlerinin açıldığı dizin) FS_ARACLAR olarak geçer.

Kullanım:
  python tests/run_tests.py
  python tests/run_tests.py --kategori positive
  python tests/run_tests.py --kategori safety
  python tests/run_tests.py --kategori runtime
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

KATEGORILER = ("positive", "negative", "safety", "runtime", "derleme", "cekirdek", "ozel")
KATEGORI_BASLIK = {
    "positive": "Pozitif / E2E",
    "negative": "Negatif / derleyici hataları",
    "safety": "Güvenlik ihlalleri",
    "runtime": "Çalışma zamanı / güvenli hata",
    "derleme": "Derleme / serbest hedef",
    "cekirdek": "Çekirdek / QEMU",
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
    cikis: Optional[int] = None
    bayrak: List[str] = field(default_factory=list)
    girdi: List[str] = field(default_factory=list)
    gerek: List[str] = field(default_factory=list)
    hazirla: List[str] = field(default_factory=list)
    denetle: List[str] = field(default_factory=list)


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
        if s.startswith("// @girdi_kacis:"):
            # \e ESC, \xNN, \r, \t kaçışları çözülür (düzenleyici tuşları için)
            ham = s.split(":", 1)[1].strip()
            ham = ham.replace("\\e", "\\x1b")
            meta.girdi.append(ham.encode("latin1", "backslashreplace").decode("unicode_escape"))
            continue
        if s.startswith("// @girdi:"):
            stdout_mod = False
            meta.girdi.append(s.split(":", 1)[1].strip())
            continue
        if s.startswith("// @bayrak:"):
            stdout_mod = False
            meta.bayrak += s.split(":", 1)[1].split()
            continue
        if s.startswith("// @gerek:"):
            stdout_mod = False
            meta.gerek += s.split(":", 1)[1].split()
            continue
        if s.startswith("// @hazirla:"):
            stdout_mod = False
            meta.hazirla.append(s.split(":", 1)[1].strip())
            continue
        if s.startswith("// @denetle:"):
            stdout_mod = False
            meta.denetle.append(s.split(":", 1)[1].strip())
            continue
        if s.startswith("// @cikis:"):
            stdout_mod = False
            parca = s.split(":", 1)[1].strip()
            try:
                meta.cikis = int(parca, 10)
            except ValueError:
                meta.cikis = 1
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
            [uppc, yol, "--sadece-derle", "--cikti", stem] + meta.bayrak,
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
    cikti_dir: str = "",
    basari_bekle: bool = False,
) -> Sonuc:
    ad = os.path.relpath(yol, TESTS)
    aranan = list(meta.icerir) if meta.icerir else ([] if basari_bekle else [varsayilan_icerir])
    try:
        der = subprocess.run(
            [uppc, yol, "--sadece-c", "--cikti", os.path.join(cikti_dir, os.path.splitext(os.path.basename(yol))[0])] + meta.bayrak,
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
    if basari_bekle and der.returncode != 0:
        return Sonuc(kategori, ad, False, "derleme başarısız:\n" + metin)
    if not basari_bekle and der.returncode == 0:
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


def calistir_runtime(uppc: str, yol: str, kaynak: str, meta: Meta, cikti_dir: str, timeout: float) -> Sonuc:
    ad = os.path.relpath(yol, TESTS)
    bek_cikis = 1 if meta.cikis is None else meta.cikis
    os.makedirs(cikti_dir, exist_ok=True)
    kok = os.path.splitext(os.path.basename(yol))[0]
    stem = os.path.join(cikti_dir, kok)
    try:
        der = subprocess.run(
            [uppc, yol, "--sadece-derle", "--cikti", stem] + meta.bayrak,
            cwd=ROOT,
            capture_output=True,
            timeout=max(timeout, 60.0),
            encoding="utf-8",
            errors="replace",
        )
    except subprocess.TimeoutExpired:
        return Sonuc("runtime", ad, False, f"derleme zaman aşımı ({timeout:.0f}s)")
    except OSError as err:
        return Sonuc("runtime", ad, False, str(err))
    if der.returncode != 0:
        msg = (der.stderr or der.stdout or "").strip()
        return Sonuc("runtime", ad, False, f"derleme başarısız (runtime bekleniyordu):\n{msg}")
    exe_yol = stem + (".exe" if sys.platform == "win32" else ".out")
    if not os.path.isfile(exe_yol):
        alt = stem if os.path.isfile(stem) else None
        if alt is None:
            return Sonuc("runtime", ad, False, f"üretilen ikili yok: {exe_yol}")
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
        return Sonuc("runtime", ad, False, f"çalıştırma zaman aşımı ({timeout:.0f}s)")
    except OSError as err:
        return Sonuc("runtime", ad, False, str(err))
    if cal.returncode == 0:
        return Sonuc("runtime", ad, False, "çıkış kodu 0; çalışma zamanı hatası bekleniyordu")
    if bek_cikis not in (0, 1) and cal.returncode != bek_cikis:
        return Sonuc(
            "runtime",
            ad,
            False,
            f"çıkış kodu {cal.returncode}, beklenen {bek_cikis}\nstderr:\n{cal.stderr}",
        )
    metin = (cal.stderr or "") + "\n" + (cal.stdout or "")
    aranan = list(meta.icerir) if meta.icerir else ["[u++ HATA]"]
    eksik = [p for p in aranan if p not in metin]
    if eksik:
        return Sonuc(
            "runtime",
            ad,
            False,
            "çıktıda yok: " + ", ".join(eksik) + f"\n  bulunan:\n{metin}",
        )
    return Sonuc("runtime", ad, True)


def posix_kabuk(komut: str, timeout: float = 300.0) -> Tuple[int, str]:
    """Komutu depo kökünde POSIX kabukta çalıştırır (Windows'ta WSL)."""
    araclar = os.environ.get("UPP_FS_ARACLAR", "")
    on = "export FS_ARACLAR='%s'; " % araclar if araclar else ""
    on += 'if [ -n "$FS_ARACLAR" ]; then PATH="$FS_ARACLAR/usr/sbin:$FS_ARACLAR/usr/bin:$PATH"; fi; '
    on += 'PATH="$PATH:/sbin:/usr/sbin"; export PATH MTOOLS_SKIP_CHECK=1 LC_ALL=C.UTF-8; '
    if os.name == "nt":
        argv = ["wsl", "-e", "bash", "-c", on + komut]
    else:
        argv = ["bash", "-c", on + komut]
    try:
        r = subprocess.run(argv, cwd=ROOT, stdin=subprocess.DEVNULL, stdout=subprocess.PIPE,
                           stderr=subprocess.STDOUT, timeout=timeout)
    except (OSError, subprocess.TimeoutExpired) as e:
        return 1, str(e)
    return r.returncode, r.stdout.decode("utf-8", errors="replace")


def calistir_cekirdek(uppc: str, yol: str, meta: Meta, cikti_dir: str, timeout: float) -> Sonuc:
    ad = os.path.relpath(yol, TESTS)
    if meta.gerek:
        kod, _ = posix_kabuk(" && ".join("command -v %s >/dev/null" % g for g in meta.gerek), 60.0)
        if kod != 0:
            return Sonuc("cekirdek", ad, True, "atlandı (araç yok: %s)" % " ".join(meta.gerek))
    for komut in meta.hazirla:
        kod, cik = posix_kabuk(komut)
        if kod != 0:
            return Sonuc("cekirdek", ad, False, "@hazirla başarısız: " + komut + "\n" + cik)
    stem = os.path.join(cikti_dir, "ck_" + os.path.splitext(os.path.basename(yol))[0])
    girdi = "".join(g + "\n" for g in meta.girdi)
    # stdin çekirdek kapanana kadar açık kalmalı: EOF'ta wsl/QEMU hemen sonlanır.
    cikti_yol = stem + ".konsol"
    with open(cikti_yol, "wb") as cf:
        proc = subprocess.Popen(
            [uppc, yol, "--hedef", "cekirdek", "-o", stem] + meta.bayrak,
            cwd=ROOT,
            stdin=subprocess.PIPE,
            stdout=cf,
            stderr=subprocess.STDOUT,
        )
        try:
            if girdi:
                proc.stdin.write(girdi.encode("utf-8"))
                proc.stdin.flush()
            proc.wait(timeout=max(timeout, 120.0))
        except subprocess.TimeoutExpired:
            proc.kill()
            proc.wait()
            return Sonuc("cekirdek", ad, False, "zaman aşımı (çekirdek kapanmadı mı? port 0xf4 ile çıkılmalı)")
        finally:
            try:
                proc.stdin.close()
            except OSError:
                pass
    with open(cikti_yol, encoding="utf-8", errors="replace") as cf:
        tum = cf.read()
    if "QEMU bulunamadı" in tum or "araç zinciri bulunamadı" in tum:
        return Sonuc("cekirdek", ad, True, "atlandı (QEMU/araç zinciri yok)")
    ayrac = "=================================================="
    cikti = tum.split(ayrac, 1)[1] if ayrac in tum else tum
    cikti = cikti.replace("\r\n", "\n").replace("\r", "").lstrip("\n")
    if meta.stdout is not None:
        if norm_cikti(cikti) != norm_cikti(meta.stdout):
            return Sonuc("cekirdek", ad, False, "çıktı uyuşmadı\n  beklenen:\n" + meta.stdout + "\n  bulunan:\n" + cikti)
    eksik = [x for x in meta.icerir if x not in cikti]
    if eksik:
        return Sonuc("cekirdek", ad, False, "çıktıda yok: " + ", ".join(eksik) + "\n  bulunan:\n" + tum)
    for komut in meta.denetle:
        kod, cik = posix_kabuk(komut)
        if kod != 0:
            return Sonuc("cekirdek", ad, False, "@denetle başarısız: " + komut + "\n" + cik)
    return Sonuc("cekirdek", ad, True)


def dosya_calistir(uppc: str, yol: str, kategori: str, cikti_dir: str, timeout: float) -> Sonuc:
    with open(yol, encoding="utf-8") as handle:
        kaynak = handle.read()
    if kaynak.startswith("\ufeff"):
        kaynak = kaynak[1:]
    meta = meta_oku(kaynak, yol)
    if kategori in ("positive", "ozel"):
        return calistir_pozitif(uppc, yol, kaynak, meta, cikti_dir, timeout)
    if kategori == "negative":
        return calistir_hata(uppc, yol, kaynak, meta, kategori, "[u++ HATA]", timeout, cikti_dir)
    if kategori == "cekirdek":
        return calistir_cekirdek(uppc, yol, meta, cikti_dir, timeout)
    if kategori == "derleme":
        return calistir_hata(uppc, yol, kaynak, meta, kategori, "", timeout, cikti_dir, basari_bekle=True)
    if kategori == "safety":
        return calistir_hata(uppc, yol, kaynak, meta, kategori, "BELLEK GÜVENLİĞİ", timeout, cikti_dir)
    if kategori == "runtime":
        return calistir_runtime(uppc, yol, kaynak, meta, cikti_dir, timeout)
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
        choices=["positive", "negative", "safety", "runtime", "derleme", "cekirdek", "ozel", "hepsi"],
        default="hepsi",
        help="yalnız bu kategoriyi çalıştır (varsayılan: hepsi)",
    )
    parser.add_argument("--uppc", default="", help="Özel native uppc yolu")
    parser.add_argument("--timeout", type=float, default=30.0, help="pozitif exe zaman aşımı (s)")
    parser.add_argument("--dosya", default="", help="yalnız bu .upp dosyasını çalıştır")
    args = parser.parse_args(argv)

    uppc = args.uppc or native_uppc_yol()
    if not uppc or not os.path.isfile(uppc):
        print(kirmizi(f"[HATA] Native uppc derleyicisi bulunamadı: {uppc or 'derleyici/uppc.exe'}"))
        print(soluk("Lütfen önce derleyiciyi derleyin."))
        return 1

    cikti_dir = os.path.join(TESTS, "_cikti")
    if args.dosya:
        yol = args.dosya
        if not os.path.isabs(yol):
            yol = os.path.join(ROOT, yol)
        if not os.path.isfile(yol):
            print(kirmizi(f"[HATA] dosya yok: {yol}"))
            return 1
        rel = os.path.relpath(yol, TESTS).replace("\\", "/")
        kat = rel.split("/", 1)[0]
        if kat not in KATEGORILER:
            kat = "positive"
        return raporla([dosya_calistir(uppc, yol, kat, cikti_dir, args.timeout)])

    secilen = (
        ["positive", "negative", "safety", "runtime", "derleme", "cekirdek", "ozel"]
        if args.kategori == "hepsi"
        else [args.kategori]
    )
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
            if kat == "runtime":
                continue
            sonuclar.append(Sonuc(kat, f"{kat}/ (dosya yok)", False, "hiç .upp bulunamadı"))
            continue
        for yol in dosyalar:
            sonuclar.append(dosya_calistir(uppc, yol, kat, cikti_dir, args.timeout))

    return raporla(sonuclar)


if __name__ == "__main__":
    sys.exit(main())
