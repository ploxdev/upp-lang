# -*- coding: utf-8 -*-
"""u++ dil zekası: tanılama, tamamlama, hover, simge, tanım, imza.

Derleyici yüzü `uppc.py` (in-process) veya native `uppc.exe --analiz`.
"""
from __future__ import annotations

import json
import os
import re
import subprocess
import sys
import tempfile
from typing import Any, Dict, Iterable, List, Optional, Tuple

ROOT_CANDIDATE = os.environ.get("UPP_KOK", "")
if ROOT_CANDIDATE and ROOT_CANDIDATE not in sys.path:
    sys.path.insert(0, ROOT_CANDIDATE)

from uppc import (  # noqa: E402
    BUILTIN_CLASS_NAMES,
    KEYWORDS,
    TYPE_NAMES,
    UPP_ARITY,
    UPP_NAMESPACES,
    UPP_RETURNS,
    ClassDecl,
    EnumDecl,
    FunctionDecl,
    Lexer,
    Program,
    Token,
    UppError,
    VarDecl,
    analiz_kaynak,
    ide_program,
    type_desc,
)

CONTROL_KEYWORDS = KEYWORDS - TYPE_NAMES - {"dogru", "yanlis", "yok"}
CONST_KEYWORDS = {"dogru", "yanlis", "yok"}

HOVER_KW: Dict[str, str] = {
    "fonk": "Fonksiyon tanımı. `fonk ad(param) -> tur { ... }`",
    "don": "Fonksiyondan dön. `-> bos` ise değer yok.",
    "eger": "Koşullu dal. `eger (mantik) { ... } yoksa { ... }`",
    "yoksa": "`eger` zincirinin else dalı.",
    "iken": "Koşullu döngü. `iken (koşul) { ... }`",
    "dongu": "C `for`. `dongu (init; koşul; adım) { ... }`",
    "her": "Dizi veya harita üzerinde gezin. `her (oto x in xs)` değer; `her (oto k in h)` haritada anahtar.",
    "in": "`her` döngüsünde kaynak dizi / harita.",
    "dur": "Döngüden / `secim` dalından çık (`break`).",
    "devam": "Döngünün sonraki turuna geç (`continue`).",
    "secim": "C `switch`. `sayi` veya `metin`. `durum` derleme sabiti. Düşme için `dur;`.",
    "durum": "`secim` sabiti (`\"...\"` veya `sabit metin` / sayı). Çalışma anı değişken olamaz.",
    "varsayilan": "`secim` varsayılan dalı.",
    "arkaplan": "İş parçacığı. `don` değer döndürür; `upp.bekle`.",
    "guvensiz": "İşaretçi, `c_kod`, `upp.bellek` kapısı.",
    "guvenli": "İç içe güvenli moda dönüş: işaretçi, `c_kod` ve `upp.bellek` yeniden yasak.",
    "sinif": "Yapı + metotlar. Alanlarda ham işaretçi yok.",
    "secenek": "Adlandırılmış tam sayı kümesi (C enum).",
    "kendi": "Metot alıcısı (`this`).",
    "oto": "Tür çıkarımı. `oto x = ...;`",
    "c_ekle": "Üst düzey C `#include`.",
    "kullan": "Başka bir `.upp` / `.uph` dosyasının bildirimlerini katar. `kullan \"kutuphane.uph\";` öneksiz. `kullan \"yol.uph\" olarak ad` yalnız o dosyanın kendi bildirimlerini önekler.",
    "olarak": "`kullan \"yol.uph\" olarak ad` — önek yalnız o dosyanın kendi bildirimlerine. Alt öneksiz `kullan` çıplak ad sızdırır.",
    "sabit": "Derleme anı sabiti. `sabit sayi MAX = 10;` veya sınıf içinde `Araba.MAKS`. Yeniden atanamaz.",
    "sayi": "64-bit tam sayı (`long long`).",
    "ondalik": "64-bit kayan (`double`).",
    "metin": "C dizgesi (`char*`).",
    "mantik": "Boolean (`dogru` / `yanlis`).",
    "bos": "Yok türü (`void`).",
    "ana": "Giriş noktası. C `main`. Parametre almaz.",
}

HOVER_BUILTIN: Dict[str, str] = {
    "SurecCikti": "`metin cikti` + `sayi kod`. `upp.sistem.calistir` dönüşü. `cikti` `malloc`; `metin_bosalt`.",
    "Kilit": "`kilitle` / `birak`. Bildirimde tutamak; kopya paylaşır. Açık destroy yok.",
    "JSONDeger": "JSON tutamacı (`id`). `upp.json.*`.",
    "ArkaplanIs": "Arka plan işi. `bekle()`.",
    "Vektor3": "`x` `y` `z` (`ondalik`).",
    "Yonelme": "`pitch` `yaw` (`ondalik`).",
    "EkranNokta": "`x` `y` `gorunur`.",
}

HOVER_UPP: Dict[str, str] = {
    "yaz": "Biçimli yaz (`printf`). `{ad}` interpolasyonu.",
    "satir_yaz": "Satır yaz. Argümansız boş satır.",
    "mesaj": "`mesaj(baslik, govde)` — MessageBox bilgi.",
    "hata": "`hata(baslik, govde)` — Windows kutu; stderr değil.",
    "hata_yaz": "`hata_yaz(s)` — stderr (`fprintf`). `{ad}` interpolasyonu.",
    "hata_satir_yaz": "`hata_satir_yaz(s)` — stderr + satır sonu. Argümansız boş satır.",
    "ses_cal": "WAV çal. Dönüş `sayi`.",
    "uyut": "Milisaniye uyu.",
    "rastgele": "`rastgele(min, max)` dahil aralık.",
    "dosya_oku": "Metin oku. `metin_bosalt` ile bırak.",
    "dosya_yaz": "Metin yaz. `1`/`0`.",
    "uzunluk": "Metin veya dizi uzunluğu.",
    "giris": "Konsol satırı oku.",
    "min": "İki `sayi` minimumu.",
    "max": "İki `sayi` maksimumu.",
    "mutlak": "`sayi` mutlak değer.",
    "metin_bosalt": "Yığın metni serbest bırak.",
    "bekle": "`ArkaplanIs` bitene kadar bekle.",
    "arguman_sayisi": "`argc` (`sayi`).",
    "arguman": "`argv[i]` (`metin`).",
    "dosya_var_mi": "Dosya var mı (`mantik`).",
    "dosya_sil": "Dosya sil (`mantik`).",
    "zaman": "Unix zamanı (`sayi`).",
    "metinden_sayiya": "Metni `sayi` yap.",
    "sayidan_metin": "`sayi` → metin (`malloc`; `metin_bosalt`).",
    "ondalikdan_metin": "`ondalik` → metin (`malloc`; `metin_bosalt`).",
    "metinden_ondalik": "Metni `ondalik` yap.",
    "dosya_ekle": "Dosyanın sonuna yaz. `1`/`0`.",
    "ortam_al": "Ortam değişkeni; yoksa `yok`.",
}

HOVER_NS: Dict[str, str] = {
    "girdi": "Fare / tuş simülasyonu.",
    "bellek": "Yabancı süreç belleği — yalnızca `guvensiz`.",
    "cizim": "Overlay çizim.",
    "matematik": "Ondalık matematik (radyan). tan, atan2, taban, tavan, yuvarla, pi().",
    "metin": "Dizge: kod / ileri / bol / birlestir / kes / ara / değiştir / kirp.",
    "json": "JSON ayrıştır / kur.",
    "stdio": "LSP için Content-Length stdio.",
    "sistem": "`calistir` → `SurecCikti { cikti, kod }`. `komut_bul`, `exe_dizin`.",
    "yol": "Yol: dizin / birlestir / gercek / calisma / mutlak_mi / klasor_olustur.",
    "dosya": "`listele(yol) -> liste[metin]`.",
}


def _create_no_window() -> int:
    if sys.platform == "win32":
        return getattr(subprocess, "CREATE_NO_WINDOW", 0x08000000)
    return 0


def tani_python(source: str, dosya: str = "<girdi>") -> List[Dict[str, Any]]:
    return analiz_kaynak(source, dosya)


def _kaynak_ad(dosya: str, varsayilan: str = "tampon.upp") -> str:
    d = str(dosya)
    k = d.replace("\\", "/").lower()
    if k.endswith(".upp") or k.endswith(".uph"):
        return os.path.basename(dosya)
    return varsayilan


def _yol_uri(path: str) -> str:
    abs_path = os.path.abspath(path)
    if sys.platform == "win32":
        return "file:///" + abs_path.replace("\\", "/")
    return "file://" + abs_path


def _ayni_kaynak(decl: Any, dosya: str) -> bool:
    kaynak = getattr(decl, "kaynak", "") or ""
    if not kaynak or not dosya or dosya.startswith("<"):
        return True
    try:
        return os.path.normcase(os.path.abspath(kaynak)) == os.path.normcase(os.path.abspath(dosya))
    except OSError:
        return os.path.normcase(kaynak) == os.path.normcase(dosya)


def tani_native(source: str, exe: str, dosya: str = "tampon.upp") -> List[Dict[str, Any]]:
    if not exe or not os.path.isfile(exe):
        return [{"satir": 1, "sutun": 1, "tur": "hata", "mesaj": f"Native derleyici bulunamadı: {exe}"}]
    dizin = os.path.dirname(os.path.abspath(dosya)) if os.path.isfile(dosya) else None
    with tempfile.TemporaryDirectory(prefix="upp_analiz_", dir=dizin if dizin and os.path.isdir(dizin) else None) as tmp:
        yol = os.path.join(tmp, _kaynak_ad(dosya))
        with open(yol, "w", encoding="utf-8", newline="\n") as handle:
            handle.write(source)
        try:
            sonuc = subprocess.run(
                [exe, yol, "--analiz"],
                capture_output=True,
                text=True,
                encoding="utf-8",
                errors="replace",
                cwd=dizin if dizin and os.path.isdir(dizin) else None,
                creationflags=_create_no_window(),
            )
        except OSError as err:
            return [{"satir": 1, "sutun": 1, "tur": "hata", "mesaj": f"Native derleyici çalışmadı: {err}"}]
    ham = (sonuc.stdout or "").strip()
    if not ham:
        mesaj = (sonuc.stderr or "Native --analiz çıktı vermedi.").strip()
        return [{"satir": 1, "sutun": 1, "tur": "hata", "mesaj": mesaj}]
    satir = ham.splitlines()[-1]
    try:
        data = json.loads(satir)
    except json.JSONDecodeError:
        return [{"satir": 1, "sutun": 1, "tur": "hata", "mesaj": f"Native JSON okunamadı: {satir[:200]}"}]
    return list(data.get("hatalar") or [])


def tani_gcc(
    source: str,
    dosya: str = "tampon.upp",
    derleyici: str = "python",
    uppc_yolu: str = "",
    kok: str = "",
) -> List[Dict[str, Any]]:
    """Kayıt sonrası tam derleme tanısı (`--json-hata`, geçici çıktı)."""
    kok = kok or os.environ.get("UPP_KOK") or ""
    with tempfile.TemporaryDirectory(prefix="upp_gcc_") as tmp:
        ad = _kaynak_ad(dosya)
        yol = os.path.join(tmp, ad)
        with open(yol, "w", encoding="utf-8", newline="\n") as handle:
            handle.write(source)
        cikti = os.path.join(tmp, "out")
        native = derleyici == "native" and uppc_yolu and os.path.isfile(uppc_yolu)
        if native:
            cmd = [uppc_yolu, yol, "--json-hata", "--sadece-derle", "--cikti", cikti]
        else:
            py = os.path.join(kok, "uppc.py") if kok else ""
            if not py or not os.path.isfile(py):
                import uppc as _uppc_mod

                py = getattr(_uppc_mod, "__file__", "") or ""
            if not py:
                return [{"satir": 1, "sutun": 1, "tur": "hata", "mesaj": "uppc.py bulunamadı (gcc tanısı)"}]
            cmd = [sys.executable, py, yol, "--json-hata", "--sadece-derle", "--cikti", cikti]
        dizin = os.path.dirname(os.path.abspath(dosya)) if os.path.isfile(dosya) else None
        try:
            sonuc = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                encoding="utf-8",
                errors="replace",
                cwd=dizin if dizin and os.path.isdir(dizin) else None,
                creationflags=_create_no_window(),
                timeout=90,
            )
        except subprocess.TimeoutExpired:
            return [{"satir": 1, "sutun": 1, "tur": "gcc", "mesaj": "GCC tanısı zaman aşımı"}]
        except OSError as err:
            return [{"satir": 1, "sutun": 1, "tur": "hata", "mesaj": f"gcc tanısı çalışmadı: {err}"}]
    ham = (sonuc.stdout or "").strip()
    if not ham:
        mesaj = (sonuc.stderr or "gcc tanısı çıktı vermedi").strip()
        return [{"satir": 1, "sutun": 1, "tur": "gcc", "mesaj": mesaj}]
    satir = ham.splitlines()[-1]
    try:
        data = json.loads(satir)
    except json.JSONDecodeError:
        return [{"satir": 1, "sutun": 1, "tur": "gcc", "mesaj": f"gcc JSON okunamadı: {satir[:200]}"}]
    return list(data.get("hatalar") or [])


def tani(
    source: str,
    dosya: str = "<girdi>",
    derleyici: str = "python",
    uppc_yolu: str = "",
) -> List[Dict[str, Any]]:
    if derleyici == "native":
        return tani_native(source, uppc_yolu, dosya)
    return tani_python(source, dosya)


def tokenize_guvenli(source: str) -> List[Token]:
    try:
        return Lexer(source).tokenize()
    except UppError:
        return []
    except Exception:
        return []


def _satirlar(source: str) -> List[str]:
    return source.replace("\r\n", "\n").replace("\r", "\n").split("\n")


def metin_oncesi(source: str, line: int, character: int) -> str:
    satirlar = _satirlar(source)
    if line < 0 or line >= len(satirlar):
        return ""
    return satirlar[line][: max(0, character)]


def token_aralik(tok: Token) -> Tuple[int, int, int, int]:
    uzun = max(len(tok.value), 1)
    return tok.line - 1, tok.column - 1, tok.line - 1, tok.column - 1 + uzun


def token_konumda(tokens: List[Token], line: int, character: int) -> Optional[Token]:
    hedef_satir = line + 1
    hedef_sutun = character + 1
    for tok in tokens:
        if tok.type == "EOF":
            continue
        bitis = tok.column + max(len(tok.value), 1)
        if tok.line == hedef_satir and tok.column <= hedef_sutun < bitis:
            return tok
    return None


def kimlik_konumda(tokens: List[Token], line: int, character: int) -> Optional[Token]:
    tok = token_konumda(tokens, line, character)
    if tok is None:
        return None
    if tok.type in ("KIMLIK", "ANAHTAR"):
        return tok
    return None


def _item(label: str, kind: int, detail: str = "", insert: Optional[str] = None) -> Dict[str, Any]:
    item: Dict[str, Any] = {"label": label, "kind": kind, "detail": detail}
    if insert is not None:
        item["insertText"] = insert
    return item


def _arity_yazi(spec: Tuple[int, Optional[int]]) -> str:
    lo, hi = spec
    if hi is None:
        return f"{lo}+"
    if lo == hi:
        return str(lo)
    return f"{lo}–{hi}"


def _fonk_imza(decl: FunctionDecl) -> str:
    params = []
    for p in decl.params:
        tur = type_desc(p.type) if p.type else "oto"
        params.append(f"{tur} {p.name}".strip())
    ret = type_desc(decl.return_type) if decl.return_type else "bos"
    prefix = f"{decl.class_name}." if decl.class_name else ""
    return f"fonk {prefix}{decl.name}({', '.join(params)}) -> {ret}"


def _program_fonksiyonlar(program: Optional[Program]) -> List[FunctionDecl]:
    if program is None:
        return []
    out: List[FunctionDecl] = []
    for d in program.decls:
        if isinstance(d, FunctionDecl):
            out.append(d)
        elif isinstance(d, ClassDecl):
            out.extend(d.methods)
    return out


def _program_siniflar(program: Optional[Program]) -> List[ClassDecl]:
    if program is None:
        return []
    return [d for d in program.decls if isinstance(d, ClassDecl)]


def _program_secenek(program: Optional[Program]) -> List[EnumDecl]:
    if program is None:
        return []
    return [d for d in program.decls if isinstance(d, EnumDecl)]


def _yuruyen_var(node: Any, acc: List[VarDecl]) -> None:
    if node is None:
        return
    if isinstance(node, VarDecl):
        acc.append(node)
    if isinstance(node, list):
        for x in node:
            _yuruyen_var(x, acc)
        return
    if hasattr(node, "__dataclass_fields__"):
        for ad in node.__dataclass_fields__:  # type: ignore[attr-defined]
            _yuruyen_var(getattr(node, ad), acc)


_KIMLIK_YOL = re.compile(
    r"([A-Za-z_çğıöşüÇĞİÖŞÜ][\wçğıöşüÇĞİÖŞÜ]*(?:\.[A-Za-z_çğıöşüÇĞİÖŞÜ][\wçğıöşüÇĞİÖŞÜ]*)*)$"
)


def _uye_yolu(source_line: str) -> Tuple[List[str], bool]:
    """Satır öneki: `upp.matematik.` → (['upp','matematik'], True). Nokta yoksa üye tamamlama yok."""
    s = source_line.rstrip()
    nokta = s.endswith(".")
    if nokta:
        s = s[:-1].rstrip()
    eslesme = _KIMLIK_YOL.search(s)
    if not eslesme:
        return [], nokta
    return eslesme.group(1).split("."), nokta


def _token_yol(tokens: List[Token], tok: Token) -> List[str]:
    try:
        i = tokens.index(tok)
    except ValueError:
        return [tok.value]
    parcalar = [tok.value]
    while i >= 2 and tokens[i - 1].type == "NOKTA" and tokens[i - 2].type in ("KIMLIK", "ANAHTAR"):
        parcalar.append(tokens[i - 2].value)
        i -= 2
    parcalar.reverse()
    return parcalar


def tamamla(source: str, line: int, character: int, dosya: str = "<girdi>") -> List[Dict[str, Any]]:
    oncesi = metin_oncesi(source, line, character)
    yol, nokta = _uye_yolu(oncesi)
    program = ide_program(source, dosya)
    items: List[Dict[str, Any]] = []
    seen = set()
    filtre = ""
    uye = list(yol)
    if yol and not nokta and len(yol) >= 2:
        filtre = yol[-1]
        uye = yol[:-1]
        nokta = True

    def ekle(it: Dict[str, Any]) -> None:
        if filtre and not str(it["label"]).startswith(filtre):
            return
        key = it["label"]
        if key in seen:
            return
        seen.add(key)
        items.append(it)

    if nokta and uye and uye[0] == "upp":
        if len(uye) == 1:
            for ns in sorted(UPP_NAMESPACES):
                ekle(_item(ns, 9, HOVER_NS.get(ns, "upp ad alanı")))
            for ad, ret in sorted(UPP_RETURNS.items()):
                ar = _arity_yazi(UPP_ARITY.get(ad, (0, 0)))
                ekle(_item(ad, 3, f"{ret}  ({ar} arg)", ad))
            return items
        if len(uye) == 2 and uye[1] in UPP_NAMESPACES:
            ns = uye[1]
            table = UPP_NAMESPACES[ns]
            for ad, (ret, arity, _c) in sorted(table.items()):
                notu = HOVER_NS.get(ns, "")
                if ns == "bellek":
                    notu = "yalnızca guvensiz · " + notu
                ekle(_item(ad, 3, f"{ret}  ({_arity_yazi(arity)} arg)  {notu}".strip(), ad))
            return items

    if nokta and uye:
        ad = uye[-1]
        for cls in _program_siniflar(program):
            if cls.name == ad:
                for f in cls.fields:
                    ekle(_item(f.name, 5, type_desc(f.type) if f.type else "", f.name))
                for m in cls.methods:
                    ekle(_item(m.name, 2, _fonk_imza(m), m.name))
                return items
        for builtin in BUILTIN_CLASS_NAMES:
            if builtin == ad:
                from uppc import make_builtin_classes

                cls = make_builtin_classes().get(builtin)
                if cls:
                    for f in cls.fields:
                        ekle(_item(f.name, 5, type_desc(f.type) if f.type else "", f.name))
                    if builtin == "Kilit":
                        ekle(_item("kilitle", 2, "fonk Kilit.kilitle() -> bos", "kilitle"))
                        ekle(_item("birak", 2, "fonk Kilit.birak() -> bos", "birak"))
                    if builtin == "ArkaplanIs":
                        ekle(_item("bekle", 2, "fonk ArkaplanIs.bekle() -> bos", "bekle"))
                return items
        vars_: List[VarDecl] = []
        _yuruyen_var(program, vars_)
        for v in vars_:
            if v.name == ad and v.type and v.type.name:
                for cls in _program_siniflar(program):
                    if cls.name == v.type.name:
                        for f in cls.fields:
                            ekle(_item(f.name, 5, type_desc(f.type) if f.type else "", f.name))
                        for m in cls.methods:
                            ekle(_item(m.name, 2, _fonk_imza(m), m.name))
                        return items
                if v.type.name in BUILTIN_CLASS_NAMES:
                    from uppc import make_builtin_classes

                    bcls = make_builtin_classes().get(v.type.name)
                    if bcls:
                        for f in bcls.fields:
                            ekle(_item(f.name, 5, type_desc(f.type) if f.type else "", f.name))
                    if v.type.name == "Kilit":
                        ekle(_item("kilitle", 2, "fonk Kilit.kilitle() -> bos", "kilitle"))
                        ekle(_item("birak", 2, "fonk Kilit.birak() -> bos", "birak"))
                    if v.type.name == "ArkaplanIs":
                        ekle(_item("bekle", 2, "fonk ArkaplanIs.bekle() -> bos", "bekle"))
                    return items

    for kw in sorted(CONTROL_KEYWORDS):
        ekle(_item(kw, 14, HOVER_KW.get(kw, "anahtar sözcük")))
    for kw in sorted(TYPE_NAMES):
        ekle(_item(kw, 25, HOVER_KW.get(kw, "tür")))
    for kw in sorted(CONST_KEYWORDS):
        ekle(_item(kw, 21, "sabit"))
    for ad in sorted(BUILTIN_CLASS_NAMES):
        ekle(_item(ad, 7, "yerleşik sınıf"))
    ekle(_item("upp", 9, "yerleşik ad alanı"))
    if program:
        for fn in _program_fonksiyonlar(program):
            ekle(_item(fn.name, 3, _fonk_imza(fn)))
        for cls in _program_siniflar(program):
            ekle(_item(cls.name, 7, "sinif"))
        for en in _program_secenek(program):
            ekle(_item(en.name, 13, "secenek"))
            for v in en.variants:
                ekle(_item(v.name, 20, en.name))
        vars_: List[VarDecl] = []
        _yuruyen_var(program, vars_)
        for v in vars_:
            tur = type_desc(v.type) if v.type else "oto"
            ekle(_item(v.name, 6, tur))
    else:
        for tok in tokenize_guvenli(source):
            if tok.type == "KIMLIK":
                ekle(_item(tok.value, 6, "kimlik"))
    return items


def hover_metni(source: str, line: int, character: int, dosya: str = "<girdi>") -> Optional[Dict[str, Any]]:
    tokens = tokenize_guvenli(source)
    tok = kimlik_konumda(tokens, line, character)
    if tok is None:
        return None
    ad = tok.value
    yol = _token_yol(tokens, tok)
    pieces: List[str] = []
    if yol and yol[0] == "upp":
        if len(yol) == 1:
            pieces.append("**upp** — yerleşik ad alanı")
        elif len(yol) == 2:
            if yol[1] in UPP_NAMESPACES:
                pieces.append(f"**upp.{yol[1]}** — {HOVER_NS.get(yol[1], '')}".rstrip(" —"))
            elif yol[1] in UPP_RETURNS:
                ret = UPP_RETURNS[yol[1]]
                ar = _arity_yazi(UPP_ARITY.get(yol[1], (0, 0)))
                pieces.append(f"**upp.{yol[1]}** → `{ret}`  ({ar} argüman)")
                if yol[1] in HOVER_UPP:
                    pieces.append(HOVER_UPP[yol[1]])
        elif len(yol) >= 3 and yol[1] in UPP_NAMESPACES:
            table = UPP_NAMESPACES[yol[1]]
            if yol[2] in table:
                ret, arity, cname = table[yol[2]]
                pieces.append(f"**upp.{yol[1]}.{yol[2]}** → `{ret}`  ({_arity_yazi(arity)} argüman)")
                pieces.append(f"`{cname}`")
                if yol[1] == "bellek":
                    pieces.append("Yalnızca `guvensiz` bloğunda.")
    if not pieces and ad in HOVER_KW:
        pieces.append(f"**{ad}**")
        pieces.append(HOVER_KW[ad])
    if not pieces and ad in HOVER_BUILTIN:
        pieces.append(f"**{ad}**")
        pieces.append(HOVER_BUILTIN[ad])
    program = ide_program(source, dosya)
    if program:
        for fn in _program_fonksiyonlar(program):
            if fn.name == ad:
                pieces.append(f"**{_fonk_imza(fn)}**")
                if fn.name == "ana":
                    pieces.append("Giriş noktası (C `main`).")
        for cls in _program_siniflar(program):
            if cls.name == ad:
                alan = ", ".join(f"{type_desc(f.type)} {f.name}" for f in cls.fields)
                pieces.append(f"**sinif {cls.name}**")
                if alan:
                    pieces.append(alan)
        for en in _program_secenek(program):
            if en.name == ad:
                pieces.append(f"**secenek {en.name}**")
                pieces.append(", ".join(v.name for v in en.variants))
            for v in en.variants:
                if v.name == ad:
                    pieces.append(f"**{en.name}.{v.name}**")
        vars_: List[VarDecl] = []
        _yuruyen_var(program, vars_)
        for v in vars_:
            if v.name == ad and v.type:
                pieces.append(f"**{ad}**: `{type_desc(v.type)}`")
                break
        for fn in _program_fonksiyonlar(program):
            for p in fn.params:
                if p.name == ad and p.type:
                    pieces.append(f"**{ad}**: `{type_desc(p.type)}` (parametre)")
                    break
    if not pieces:
        return None
    # tekilleştir
    uniq: List[str] = []
    for p in pieces:
        if p not in uniq:
            uniq.append(p)
    sl, sc, el, ec = token_aralik(tok)
    return {
        "contents": {"kind": "markdown", "value": "\n\n".join(uniq)},
        "range": {
            "start": {"line": sl, "character": sc},
            "end": {"line": el, "character": ec},
        },
    }


def _aralik_node(node: Any, uzun: int = 1) -> Dict[str, Any]:
    line = max(0, int(getattr(node, "line", 1) or 1) - 1)
    col = max(0, int(getattr(node, "column", 1) or 1) - 1)
    ad = getattr(node, "name", "") or ""
    bitis = col + max(len(ad), uzun)
    return {
        "start": {"line": line, "character": col},
        "end": {"line": line, "character": bitis},
    }


def belge_simgeleri(source: str, dosya: str = "<girdi>") -> List[Dict[str, Any]]:
    program = ide_program(source, dosya)
    if program is None:
        return []
    out: List[Dict[str, Any]] = []
    for d in program.decls:
        if not _ayni_kaynak(d, dosya):
            continue
        if isinstance(d, FunctionDecl):
            detay = "main" if d.name == "ana" else type_desc(d.return_type)
            out.append({
                "name": d.name,
                "detail": detay,
                "kind": 12,
                "range": _aralik_node(d, len(d.name)),
                "selectionRange": _aralik_node(d, len(d.name)),
            })
        elif isinstance(d, ClassDecl):
            children = []
            for f in d.fields:
                children.append({
                    "name": f.name,
                    "detail": type_desc(f.type) if f.type else "",
                    "kind": 8,
                    "range": _aralik_node(f, len(f.name)),
                    "selectionRange": _aralik_node(f, len(f.name)),
                })
            for m in d.methods:
                children.append({
                    "name": m.name,
                    "detail": _fonk_imza(m),
                    "kind": 6,
                    "range": _aralik_node(m, len(m.name)),
                    "selectionRange": _aralik_node(m, len(m.name)),
                })
            out.append({
                "name": d.name,
                "detail": "sinif",
                "kind": 5,
                "range": _aralik_node(d, len(d.name)),
                "selectionRange": _aralik_node(d, len(d.name)),
                "children": children,
            })
        elif isinstance(d, EnumDecl):
            children = [{
                "name": v.name,
                "kind": 22,
                "range": _aralik_node(v, len(v.name)),
                "selectionRange": _aralik_node(v, len(v.name)),
            } for v in d.variants]
            out.append({
                "name": d.name,
                "detail": "secenek",
                "kind": 10,
                "range": _aralik_node(d, len(d.name)),
                "selectionRange": _aralik_node(d, len(d.name)),
                "children": children,
            })
    return out


def tanim(source: str, uri: str, line: int, character: int, dosya: str = "<girdi>") -> List[Dict[str, Any]]:
    tokens = tokenize_guvenli(source)
    tok = kimlik_konumda(tokens, line, character)
    if tok is None:
        return []
    ad = tok.value
    program = ide_program(source, dosya)
    if program is None:
        return []
    hedefler: List[Any] = []
    for fn in _program_fonksiyonlar(program):
        if fn.name == ad:
            hedefler.append(fn)
        for p in fn.params:
            if p.name == ad:
                hedefler.append(p)
    for cls in _program_siniflar(program):
        if cls.name == ad:
            hedefler.append(cls)
        for f in cls.fields:
            if f.name == ad:
                hedefler.append(f)
    for en in _program_secenek(program):
        if en.name == ad:
            hedefler.append(en)
        for v in en.variants:
            if v.name == ad:
                hedefler.append(v)
    vars_: List[VarDecl] = []
    _yuruyen_var(program, vars_)
    for v in vars_:
        if v.name == ad:
            hedefler.append(v)
    locs = []
    for h in hedefler:
        rng = _aralik_node(h)
        hedef_uri = uri
        kaynak = getattr(h, "kaynak", "") or ""
        if kaynak and os.path.isfile(kaynak):
            kay_abs = os.path.abspath(kaynak)
            if not os.path.isfile(dosya) or os.path.normcase(kay_abs) != os.path.normcase(os.path.abspath(dosya)):
                hedef_uri = _yol_uri(kay_abs)
        locs.append({"uri": hedef_uri, "range": rng})
    return locs


def referanslar(source: str, uri: str, line: int, character: int) -> List[Dict[str, Any]]:
    tokens = tokenize_guvenli(source)
    tok = kimlik_konumda(tokens, line, character)
    if tok is None:
        return []
    ad = tok.value
    locs = []
    for t in tokens:
        if t.type == "KIMLIK" and t.value == ad:
            sl, sc, el, ec = token_aralik(t)
            locs.append({
                "uri": uri,
                "range": {
                    "start": {"line": sl, "character": sc},
                    "end": {"line": el, "character": ec},
                },
            })
    return locs


def _cagri_geri(source: str, line: int, character: int) -> Tuple[Optional[str], int]:
    oncesi = metin_oncesi(source, line, character)
    derinlik = 0
    virgul = 0
    i = len(oncesi) - 1
    while i >= 0:
        ch = oncesi[i]
        if ch == ")":
            derinlik += 1
        elif ch == "(":
            if derinlik == 0:
                j = i
                while j > 0 and oncesi[j - 1] in " \t":
                    j -= 1
                k = j
                parcalar: List[str] = []
                while k > 0:
                    while k > 0 and oncesi[k - 1] in " \t":
                        k -= 1
                    m = k
                    while m > 0 and (oncesi[m - 1].isalnum() or oncesi[m - 1] in "._çğıöşüÇĞİÖŞÜ"):
                        m -= 1
                    if m == k:
                        break
                    parcalar.append(oncesi[m:k])
                    k = m
                    break
                parcalar.reverse()
                yol, _nokta = _uye_yolu(oncesi[:i] + ".")
                if yol:
                    ad = ".".join(yol)
                else:
                    ad = "".join(parcalar) if parcalar else ""
                return ad, virgul
            derinlik -= 1
        elif ch == "," and derinlik == 0:
            virgul += 1
        i -= 1
    return None, 0


def imza_yardim(source: str, line: int, character: int, dosya: str = "<girdi>") -> Optional[Dict[str, Any]]:
    ad, aktif = _cagri_geri(source, line, character)
    if not ad:
        return None
    label = ""
    params: List[Dict[str, str]] = []
    if ad.startswith("upp."):
        parca = ad.split(".")
        if len(parca) == 2 and parca[1] in UPP_ARITY:
            lo, hi = UPP_ARITY[parca[1]]
            ret = UPP_RETURNS.get(parca[1], "bos")
            if hi is None:
                params = [{"label": f"arg{i + 1}"} for i in range(max(lo, 0))] + [{"label": "..."}]
            else:
                n = hi if hi is not None else lo
                params = [{"label": f"arg{i + 1}"} for i in range(max(n, 1))]
            label = f"{ad}(...) -> {ret}"
        elif len(parca) == 3 and parca[1] in UPP_NAMESPACES and parca[2] in UPP_NAMESPACES[parca[1]]:
            ret, arity, _c = UPP_NAMESPACES[parca[1]][parca[2]]
            lo, hi = arity
            n = lo if hi is None else (hi or lo)
            params = [{"label": f"arg{i + 1}"} for i in range(max(n, 0))]
            label = f"{ad}({', '.join(p['label'] for p in params)}) -> {ret}"
    if not label:
        program = ide_program(source, dosya)
        kisa = ad.split(".")[-1]
        for fn in _program_fonksiyonlar(program):
            if fn.name == kisa:
                params = [{"label": f"{type_desc(p.type) if p.type else 'oto'} {p.name}"} for p in fn.params]
                label = _fonk_imza(fn)
                break
    if not label:
        return None
    return {
        "signatures": [{"label": label, "parameters": params}],
        "activeSignature": 0,
        "activeParameter": min(aktif, max(len(params) - 1, 0)) if params else 0,
    }


def kod_eylemleri(
    uri: str,
    source: str,
    rng: Dict[str, Any],
    diagnostics: Iterable[Dict[str, Any]],
) -> List[Dict[str, Any]]:
    actions: List[Dict[str, Any]] = []
    for d in diagnostics:
        data = d.get("data") or {}
        mesaj = str(d.get("message") or "")
        tur = data.get("tur") if isinstance(data, dict) else ""
        dr = d.get("range") or rng
        if tur == "guvenlik" or "GÜVENLİ" in mesaj or "guvensiz" in mesaj.lower():
            satirlar = _satirlar(source)
            sl = int((dr.get("start") or {}).get("line") or 0)
            if sl < len(satirlar):
                satir = satirlar[sl]
                govde = satir.strip()
                if govde and not govde.startswith("guvensiz"):
                    girinti = satir[: len(satir) - len(satir.lstrip(" \t"))]
                    yeni = f"{girinti}guvensiz {{\n{girinti}    {govde}\n{girinti}}}"
                    satir_aralik = {
                        "start": {"line": sl, "character": 0},
                        "end": {"line": sl, "character": len(satir)},
                    }
                    actions.append({
                        "title": "guvensiz { } ile sar",
                        "kind": "quickfix",
                        "edit": {"changes": {uri: [{"range": satir_aralik, "newText": yeni}]}},
                    })
            break
        if "fonk ana()" in mesaj:
            son = len(_satirlar(source))
            iskelet = "\nfonk ana() -> sayi {\n    don 0;\n}\n"
            actions.append({
                "title": "fonk ana() iskeleti ekle",
                "kind": "quickfix",
                "edit": {
                    "changes": {
                        uri: [{
                            "range": {
                                "start": {"line": son, "character": 0},
                                "end": {"line": son, "character": 0},
                            },
                            "newText": iskelet,
                        }]
                    }
                },
            })
    return actions
