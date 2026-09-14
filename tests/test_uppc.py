#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""u++ tip sistemi / C üretimi regresyon testleri."""

from __future__ import annotations

import io
import json
import os
import subprocess
import sys
import tempfile
import unittest
from contextlib import redirect_stderr, redirect_stdout

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if ROOT not in sys.path:
    sys.path.insert(0, ROOT)

from uppc import (
    SafetyError,
    UppError,
    cli_hedef,
    compile_file,
    compile_source,
    find_gcc,
    hedef_bu_makinede_calisir,
    host_hedef,
    invoke_gcc,
    output_stems,
)


def wrap(body: str, extra: str = "") -> str:
    return extra + "fonk ana() -> sayi {\n" + body + "\n    don 0;\n}\n"


def compile_ok(src: str) -> str:
    buf = io.StringIO()
    with redirect_stderr(buf):
        return compile_source(src)


def _denetim_engeli(err: BaseException) -> bool:
    kod = getattr(err, "winerror", None)
    metin = str(err)
    return kod == 4551 or "Uygulama Denetimi" in metin or "Application Control" in metin


def native_run(cmd, **kwargs):
    kwargs.setdefault("capture_output", True)
    kwargs.setdefault("text", True)
    kwargs.setdefault("encoding", "utf-8")
    kwargs.setdefault("errors", "replace")
    kwargs.setdefault("cwd", ROOT)
    try:
        return subprocess.run(cmd, **kwargs)
    except OSError as err:
        if _denetim_engeli(err):
            raise unittest.SkipTest(f"Uygulama Denetimi bu ikiliyi engelledi: {cmd[0]}") from err
        raise


def compile_err(src: str) -> str:
    buf = io.StringIO()
    with redirect_stderr(buf):
        try:
            compile_source(src)
        except (UppError, SafetyError) as err:
            return str(err)
    raise AssertionError("u++ hata bekleniyordu, derleme başarılı oldu")


class TipVeUretimTestleri(unittest.TestCase):
    def test_cikti_adlari(self) -> None:
        c, exe = output_stems("ornek", "windows")
        self.assertTrue(c.endswith("ornek.c"))
        self.assertTrue(exe.endswith("ornek.exe"))
        c2, exe2 = output_stems(None, "windows")
        self.assertTrue(c2.endswith("cikti.c"))
        self.assertTrue(exe2.endswith("program.exe"))
        c3, exe3 = output_stems("ornek.c", "windows")
        self.assertTrue(c3.endswith("ornek.c"))
        self.assertTrue(exe3.endswith("ornek.exe"))
        c4, exe4 = output_stems("ornek.Exe", "windows")
        self.assertTrue(c4.endswith("ornek.c"))
        self.assertTrue(exe4.endswith("ornek.exe"))
        c5, exe5 = output_stems("mod.UpP", "windows")
        self.assertTrue(c5.endswith("mod.c"))
        self.assertTrue(exe5.endswith("mod.exe"))
        c6, exe6 = output_stems("lib.uph", "windows")
        self.assertTrue(c6.endswith("lib.c"))
        self.assertTrue(exe6.endswith("lib.exe"))

    def test_cikti_adlari_linux(self) -> None:
        c, exe = output_stems("ornek", "linux")
        self.assertTrue(c.endswith("ornek.c"))
        self.assertTrue(exe.endswith("ornek.out"))
        c2, exe2 = output_stems(None, "linux")
        self.assertTrue(c2.endswith("cikti.c"))
        self.assertTrue(exe2.endswith("program.out"))
        c3, exe3 = output_stems("mod.out", "linux")
        self.assertTrue(c3.endswith("mod.c"))
        self.assertTrue(exe3.endswith("mod.out"))

    def test_linux_hedef_posix_c(self) -> None:
        src = wrap(
            "    arkaplan {\n"
            '        upp.satir_yaz("arka");\n'
            "    }\n"
        )
        c = compile_source(src, "<girdi>", "linux")
        self.assertIn("#define UPP_HEDEF_LINUX 1", c)
        self.assertNotIn("#define UPP_HEDEF_WINDOWS 1", c)
        self.assertIn("#include <pthread.h>", c)
        self.assertNotIn("#include <windows.h>", c)
        self.assertIn("pthread_create", c)
        self.assertIn("pthread_detach", c)
        self.assertNotIn("sys/random.h", c)

    def test_linux_arkaplan_bekle_pthread(self) -> None:
        src = wrap(
            "    oto t = arkaplan {\n"
            '        upp.satir_yaz("is");\n'
            "    };\n"
            "    t.bekle();\n"
        )
        c = compile_source(src, "<girdi>", "linux")
        self.assertIn("pthread_join", c)
        self.assertIn("(long long)(uintptr_t)_th", c)
        self.assertIn("UPP_HEDEF_LINUX", c)

    def test_windows_hedef_makro(self) -> None:
        c = compile_source("fonk ana() -> sayi { don 0; }\n", "<girdi>", "windows")
        self.assertIn("#define UPP_HEDEF_WINDOWS 1", c)
        self.assertIn("#include <windows.h>", c)
        self.assertNotIn("#define UPP_HEDEF_LINUX 1", c)

    def test_ornek_dosyalar_linux(self) -> None:
        ornek_dir = os.path.join(ROOT, "ornekler")
        for name in ("ornek1.upp", "ornek2.upp", "ornek_json.upp", "ornek3.upp"):
            path = os.path.join(ornek_dir, name)
            with open(path, encoding="utf-8") as handle:
                src = handle.read()
            c = compile_source(src, path, "linux")
            self.assertIn("int main(int argc, char **argv)", c)
            self.assertIn("#define UPP_HEDEF_LINUX 1", c)
            self.assertNotIn("#include <windows.h>", c)

    def test_cikti_yazilamaz_hata(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            blocker = os.path.join(tmp, "dosya_klasor_degil")
            with open(blocker, "w", encoding="utf-8") as handle:
                handle.write("x")
            src_path = os.path.join(tmp, "a.upp")
            with open(src_path, "w", encoding="utf-8") as handle:
                handle.write("fonk ana() -> sayi { don 0; }\n")
            buf = io.StringIO()
            with redirect_stdout(buf):
                code = compile_file(
                    src_path,
                    emit_c_only=True,
                    cikti=os.path.join(blocker, "cikti"),
                )
            self.assertEqual(code, 1)
            self.assertIn("Çıktı yazılamadı", buf.getvalue())

    def test_struct_karsilastirma_red(self) -> None:
        src = wrap(
            "    A a;\n    A b;\n    eger (a == b) { don 0; }\n",
            extra="sinif A { sayi x; }\n",
        )
        msg = compile_err(src)
        self.assertIn("Sınıf değerleri", msg)

    def test_metin_birlestirme_sabit(self) -> None:
        src = wrap('    oto s = "Merhaba" + " Dunya";\n    upp.satir_yaz(s);\n')
        c = compile_ok(src)
        self.assertIn('"Merhaba Dunya"', c)
        self.assertNotIn("Merhaba\" + \"", c)
        self.assertNotIn('("Merhaba"', c)

    def test_metin_birlestirme_calisma_ani(self) -> None:
        src = wrap(
            '    oto a = "Merhaba";\n'
            '    oto b = " Dunya";\n'
            "    oto s = a + b;\n"
            "    upp.metin_bosalt(s);\n"
        )
        c = compile_ok(src)
        self.assertIn("upp_metin_birlestir", c)

    def test_bos_ok_fonksiyon_return_yok(self) -> None:
        src = (
            'fonk f() -> bos => upp.mesaj("Baslik", "Metin");\n'
            "fonk ana() -> sayi {\n    f();\n    don 0;\n}\n"
        )
        c = compile_ok(src)
        self.assertIn("upp_mesaj", c)
        self.assertNotIn("return upp_mesaj", c)
        self.assertIn("void f(void)", c)

    def test_tanimsiz_fonksiyon_bildirim(self) -> None:
        src = wrap("    sayi x = tanimsiz_fonksiyon();\n")
        msg = compile_err(src)
        self.assertIn("Tanımsız fonksiyon", msg)

    def test_lvalue_atama(self) -> None:
        src = wrap("    sayi x = 1;\n    5 = x;\n")
        msg = compile_err(src)
        self.assertIn("lvalue", msg)

    def test_ana_metin_donus_red(self) -> None:
        src = 'fonk ana() -> metin {\n    don "hello";\n}\n'
        msg = compile_err(src)
        self.assertIn("int main", msg)

    def test_don_tur_uyumsuz(self) -> None:
        src = wrap("    don \"hello\";\n")
        msg = compile_err(src)
        self.assertIn("Dönüş türü uyumsuz", msg)

    def test_sinif_isaretci_alani(self) -> None:
        src = wrap("    don 0;\n", extra="sinif A { sayi* p; }\n")
        msg = compile_err(src)
        self.assertIn("BELLEK GÜVENLİĞİ", msg)

    def test_ana_int_main_sayi(self) -> None:
        c = compile_source("fonk ana() -> sayi { don 0; }\n", "<girdi>", "windows")
        self.assertIn("int main(int argc, char **argv)", c)
        self.assertIn("return (int)(", c)
        self.assertIn("upp_argv_ayarla", c)
        self.assertIn("SetConsoleOutputCP(CP_UTF8)", c)
        self.assertIn("SetConsoleCP(CP_UTF8)", c)
        c_lin = compile_source("fonk ana() -> sayi { don 0; }\n", "<girdi>", "linux")
        self.assertNotIn("SetConsoleOutputCP", c_lin)
        self.assertNotIn("SetConsoleCP", c_lin)

    def test_upp_kaynak_marker_satir(self) -> None:
        pad = "".join(f"// pad {i}\n" for i in range(40))
        src = pad + "// @upp-kaynak lexer.upp\nfonk ana() -> sayi {\n    #bad;\n    don 0;\n}\n"
        msg = compile_err(src)
        self.assertIn("Satır 2:", msg)
        self.assertIn("lexer.upp", msg)
        self.assertNotIn("Satır 43:", msg)

    def test_ana_bos_return_sifir(self) -> None:
        c = compile_ok("fonk ana() -> bos { don; }\n")
        self.assertIn("int main(int argc, char **argv)", c)
        self.assertIn("return 0;", c)

    def test_sinif_sifir_baslatma(self) -> None:
        src = wrap("    A a;\n", extra="sinif A { sayi x; metin ad; }\n")
        c = compile_ok(src)
        self.assertIn("A a = {0};", c)

    def test_runtime_ub_korumasi(self) -> None:
        c = compile_ok("fonk ana() -> sayi { don 0; }\n")
        self.assertIn("#include <limits.h>", c)
        self.assertIn("LLONG_MAX", c)
        self.assertIn("umax - umin", c)
        self.assertIn("_upp_rand_u64", c)
        self.assertIn("0x7FFFull", c)
        self.assertNotIn("(unsigned long long)rand() % span", c)
        self.assertIn("_upp_utf8_bayt", c)
        self.assertIn("upp_metin_bosalt", c)
        self.assertIn("upp_metin_esit", c)

    def test_metin_esitlik(self) -> None:
        src = wrap('    oto a = "x";\n    eger (a == "x") { don 0; }\n')
        c = compile_ok(src)
        self.assertIn("upp_metin_esit", c)

    def test_bos_fonksiyonda_deger_don(self) -> None:
        src = "fonk f() { don 1; }\nfonk ana() -> sayi { don 0; }\n"
        msg = compile_err(src)
        self.assertIn("bos", msg)

    def test_ana_ok_metin_red(self) -> None:
        msg = compile_err('fonk ana() => "hello";\n')
        self.assertIn("int main", msg)

    def test_atama_tur_uyumsuz(self) -> None:
        src = wrap('    sayi x = 1;\n    x = "a";\n')
        msg = compile_err(src)
        self.assertIn("Atama türü uyumsuz", msg)

    def test_metin_arti_sayi_red(self) -> None:
        src = wrap('    oto s = "a" + 1;\n')
        msg = compile_err(src)
        self.assertIn("Metin birleştirme", msg)

    def test_ornek_dosyalar(self) -> None:
        ornek_dir = os.path.join(ROOT, "ornekler")
        for name in ("ornek1.upp", "ornek2.upp", "ornek_json.upp", "ornek3.upp"):
            path = os.path.join(ornek_dir, name)
            with open(path, encoding="utf-8") as handle:
                src = handle.read()
            c = compile_source(src, path)
            self.assertIn("int main(int argc, char **argv)", c)

    def test_arkaplan_createthread(self) -> None:
        src = wrap(
            "    arkaplan {\n"
            '        upp.satir_yaz("arka");\n'
            "    }\n"
        )
        c = compile_source(src, "<girdi>", "windows")
        self.assertIn("CreateThread", c)
        self.assertIn("_upp_bg_1", c)

    def test_matematik_ve_yonelme(self) -> None:
        src = wrap(
            "    Yonelme y = upp.matematik.aci_hesapla(0.0, 0.0, 0.0, 1.0, 0.0, 0.0);\n"
            "    oto d = upp.matematik.mesafe_3d(0.0, 0.0, 0.0, 3.0, 4.0, 0.0);\n"
            '    upp.satir_yaz("{y.pitch} {d}");\n'
        )
        c = compile_ok(src)
        self.assertIn("upp_mat_aci_hesapla", c)
        self.assertIn("upp_mat_mesafe_3d", c)
        self.assertIn("Yonelme y", c)

    def test_girdi_cizim_uret(self) -> None:
        src = wrap(
            "    upp.girdi.fare_tasi(10, 20);\n"
            "    upp.girdi.fare_tikla();\n"
            '    upp.girdi.tus_bas("F1");\n'
            '    upp.cizim.baslat("notepad.exe");\n'
            '    upp.cizim.kutu(1, 2, 3, 4, "KIRMIZI");\n'
            '    upp.cizim.yazi(0, 0, "hi", "YESIL");\n'
        )
        c = compile_ok(src)
        self.assertIn("upp_girdi_fare_tasi", c)
        self.assertIn("upp_girdi_tus_bas", c)
        self.assertIn("upp_cizim_kutu", c)

    def test_bellek_guvensiz_zorunlu(self) -> None:
        src = wrap('    upp.bellek.baglan("x.exe");\n')
        msg = compile_err(src)
        self.assertIn("BELLEK GÜVENLİĞİ", msg)

    def test_bellek_guvensiz_ok(self) -> None:
        src = wrap(
            "    guvensiz {\n"
            '        upp.bellek.baglan("notepad.exe");\n'
            '        oto taban = upp.bellek.modul_bul("notepad.exe");\n'
            "        oto son = upp.bellek.zincir_oku(taban, [16, 32]);\n"
            '        upp.bellek.yama_yap(son, "90 90");\n'
            '        upp.bellek.koruma_degistir(son, 8, "okuyazcalistir");\n'
            "    }\n"
        )
        c = compile_ok(src)
        self.assertIn("upp_bellek_zincir_oku", c)
        self.assertIn("upp_bellek_yama_yap", c)

    def test_bos_blok_don_void_ifade(self) -> None:
        src = (
            'fonk f() -> bos {\n'
            '    don upp.mesaj("A", "B");\n'
            "}\n"
            "fonk ana() -> sayi {\n    f();\n    don 0;\n}\n"
        )
        c = compile_ok(src)
        self.assertIn("upp_mesaj", c)
        self.assertNotIn("return upp_mesaj", c)

    def test_cift_ana_red(self) -> None:
        src = "fonk ana() -> sayi { don 0; }\nfonk ana() -> sayi { don 1; }\n"
        msg = compile_err(src)
        self.assertIn("ana", msg)

    def test_eksik_don_red(self) -> None:
        src = "fonk f() -> sayi { }\nfonk ana() -> sayi { don 0; }\n"
        msg = compile_err(src)
        self.assertIn("don", msg)

    def test_zincir_oku_isaretci_red(self) -> None:
        src = wrap(
            "    guvensiz {\n"
            "        sayi* p = yok;\n"
            "        upp.bellek.zincir_oku(0, p);\n"
            "    }\n"
        )
        msg = compile_err(src)
        self.assertTrue("dizi" in msg or "işaretçi" in msg or "isaretci" in msg)

    def test_arkaplan_dizi_yakalama_red(self) -> None:
        src = wrap(
            "    sayi xs[] = [1, 2, 3];\n"
            "    arkaplan {\n"
            "        sayi n = xs[0];\n"
            "    }\n"
        )
        msg = compile_err(src)
        self.assertIn("dizi", msg)

    def test_arkaplan_liste_yakalama_red(self) -> None:
        src = wrap(
            "    liste[sayi] xs;\n"
            "    xs.ekle(1);\n"
            "    arkaplan {\n"
            "        sayi n = xs.al(0);\n"
            "    }\n"
        )
        msg = compile_err(src)
        self.assertIn("liste", msg)

    def test_arkaplan_harita_yakalama_red(self) -> None:
        src = wrap(
            '    harita[metin,sayi] h;\n'
            '    h.koy("a", 1);\n'
            "    arkaplan {\n"
            '        sayi n = h.al("a");\n'
            "    }\n"
        )
        msg = compile_err(src)
        self.assertIn("harita", msg)

    def test_arkaplan_yakalama_kopya(self) -> None:
        src = wrap(
            "    sayi n = 7;\n"
            "    arkaplan {\n"
            '        upp.satir_yaz("{n}");\n'
            "    }\n"
        )
        c = compile_source(src, "<girdi>", "windows")
        self.assertIn("_upp_bgctx_", c)
        self.assertIn("malloc", c)
        self.assertIn("_ctx->n = n", c)
        self.assertIn("CreateThread", c)

    def test_arkaplan_metin_kopya(self) -> None:
        src = wrap(
            '    metin s = "hi";\n'
            "    arkaplan {\n"
            '        upp.satir_yaz("{s}");\n'
            "    }\n"
        )
        c = compile_ok(src)
        self.assertIn("upp_metin_kopya", c)
        self.assertIn("(s && !_ctx->s)", c)

    def test_arkaplan_don_deger_red(self) -> None:
        src = wrap("    arkaplan { don 1; }\n")
        msg = compile_err(src)
        self.assertIn("arkaplan", msg)

    def test_ana_sayi_don_zorunlu(self) -> None:
        msg = compile_err("fonk ana() -> sayi { }\n")
        self.assertIn("don", msg)

    def test_iken_dogru_don(self) -> None:
        src = (
            "fonk f() -> sayi {\n"
            "    iken (dogru) {\n"
            "        don 1;\n"
            "    }\n"
            "}\n"
            "fonk ana() -> sayi { don f(); }\n"
        )
        c = compile_ok(src)
        self.assertIn("while", c)

    def test_dongu_dogru_don(self) -> None:
        src = (
            "fonk f() -> sayi {\n"
            "    dongu (sayi i = 0; dogru; i = i + 1) {\n"
            "        don 1;\n"
            "    }\n"
            "}\n"
            "fonk ana() -> sayi { don f(); }\n"
        )
        c = compile_ok(src)
        self.assertIn("for (", c)

    def test_dongu_sonsuz_don(self) -> None:
        src = (
            "fonk f() -> sayi {\n"
            "    dongu (;;) {\n"
            "        don 1;\n"
            "    }\n"
            "}\n"
            "fonk ana() -> sayi { don f(); }\n"
        )
        c = compile_ok(src)
        self.assertIn("for (", c)

    def test_dongu_sonlu_don_yok(self) -> None:
        src = (
            "fonk f() -> sayi {\n"
            "    dongu (sayi i = 0; i < 3; i = i + 1) {\n"
            "        don 1;\n"
            "    }\n"
            "}\n"
            "fonk ana() -> sayi { don f(); }\n"
        )
        msg = compile_err(src)
        self.assertIn("don", msg)

    def test_mesaj_interpolasyon_yigin(self) -> None:
        src = wrap('    sayi n = 7;\n    upp.mesaj("t", "x {n}");\n')
        c = compile_ok(src)
        self.assertIn("snprintf(NULL, 0", c)
        self.assertNotRegex(c, r"char _upp_t\d+\[512\]")
        self.assertIn("upp_mesaj", c)

    def test_giris_satir_buyur(self) -> None:
        c = compile_ok(wrap("    oto s = upp.giris();\n    don 0;\n"))
        self.assertIn("fgetc(stdin)", c)
        start = c.find("static char* upp_giris(")
        self.assertNotEqual(start, -1)
        end = c.find("\nstatic ", start + 1)
        govde = c[start:end if end != -1 else None]
        self.assertNotIn("fgets(", govde)

    def test_dunya_ekran_kisa_dizi(self) -> None:
        src = wrap("    oto p = upp.matematik.dunya_ekran(0.0, 0.0, 0.0, [1.0], 10.0, 10.0);\n")
        msg = compile_err(src)
        self.assertIn("16", msg)

    def test_dunya_ekran_kisa_degisken(self) -> None:
        src = wrap(
            "    ondalik m[] = [1.0, 2.0];\n"
            "    oto p = upp.matematik.dunya_ekran(0.0, 0.0, 0.0, m, 10.0, 10.0);\n"
        )
        msg = compile_err(src)
        self.assertIn("16", msg)

    def test_dunya_ekran_kisa_sabit_boyut(self) -> None:
        src = wrap(
            "    ondalik m[4];\n"
            "    oto p = upp.matematik.dunya_ekran(0.0, 0.0, 0.0, m, 10.0, 10.0);\n"
        )
        msg = compile_err(src)
        self.assertIn("16", msg)

    def test_cli_hedef_secimi(self) -> None:
        self.assertEqual(cli_hedef(linux=True), "linux")
        self.assertEqual(cli_hedef(windows=True), "windows")
        self.assertEqual(cli_hedef(), host_hedef())
        with self.assertRaises(UppError):
            cli_hedef(linux=True, windows=True)
        self.assertTrue(hedef_bu_makinede_calisir(host_hedef()))
        diger = "linux" if host_hedef() == "windows" else "windows"
        self.assertFalse(hedef_bu_makinede_calisir(diger))

    def test_cli_linux_windows_cakisma(self) -> None:
        from uppc import main as uppc_main

        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit):
            uppc_main(["ornekler/ornek1.upp", "--linux", "--windows"])
        self.assertIn("--linux ve --windows", buf.getvalue())

    def test_cli_linux_sadece_c(self) -> None:
        from uppc import main as uppc_main

        with tempfile.TemporaryDirectory() as tmp:
            src_path = os.path.join(tmp, "a.upp")
            with open(src_path, "w", encoding="utf-8") as handle:
                handle.write("fonk ana() -> sayi { don 0; }\n")
            cikti = os.path.join(tmp, "hedef")
            buf = io.StringIO()
            with redirect_stdout(buf):
                code = uppc_main([src_path, "--linux", "--sadece-c", "--cikti", cikti])
            self.assertEqual(code, 0)
            with open(cikti + ".c", encoding="utf-8") as handle:
                c = handle.read()
            self.assertIn("#define UPP_HEDEF_LINUX 1", c)
            self.assertNotIn("#include <windows.h>", c)
            self.assertFalse(os.path.isfile(cikti + ".out"))

    def test_dunya_ekran_sayac(self) -> None:
        src = wrap(
            "    ondalik m[] = [1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0];\n"
            "    oto p = upp.matematik.dunya_ekran(0.0, 0.0, 0.0, m, 1920.0, 1080.0);\n"
        )
        c = compile_ok(src)
        self.assertIn("upp_mat_dunya_ekran", c)
        self.assertIn("sizeof", c)

    def test_hex_sayi(self) -> None:
        src = wrap("    oto x = 0x90;\n    don x;\n")
        c = compile_ok(src)
        self.assertIn("144LL", c)

    def test_secim_ve_her(self) -> None:
        src = wrap(
            "    sayi xs[] = [1, 2, 3];\n"
            "    oto t = 0;\n"
            "    her (oto v in xs) { t = t + v; }\n"
            "    secim (t) {\n"
            "        durum 6:\n"
            "            t = t + 1;\n"
            "            dur;\n"
            "        varsayilan:\n"
            "            t = 0;\n"
            "    }\n"
        )
        c = compile_ok(src)
        self.assertIn("switch", c)
        self.assertIn("for (", c)

    def test_secim_degisken_durum_red(self) -> None:
        src = wrap(
            "    sayi k = 2;\n"
            "    secim (k) {\n"
            "        durum k:\n"
            "            dur;\n"
            "        varsayilan:\n"
            "            dur;\n"
            "    }\n"
        )
        msg = compile_err(src)
        self.assertIn("durum", msg)
        self.assertIn("sabit", msg)

    def test_secenek_enum(self) -> None:
        src = wrap(
            "    Renk r = Renk.Yesil;\n"
            "    secim (r) {\n"
            "        durum Renk.Kirmizi:\n"
            "            dur;\n"
            "        durum Renk.Yesil:\n"
            "            dur;\n"
            "        varsayilan:\n"
            "            dur;\n"
            "    }\n",
            extra="secenek Renk { Kirmizi, Yesil = 2, Mavi }\n",
        )
        c = compile_ok(src)
        self.assertIn("typedef enum Renk", c)
        self.assertIn("Renk_Yesil", c)

    def test_arkaplan_bekle(self) -> None:
        src = wrap(
            "    oto t = arkaplan {\n"
            '        upp.satir_yaz("is");\n'
            "    };\n"
            "    t.bekle();\n"
        )
        c = compile_ok(src)
        self.assertIn("WaitForSingleObject", c)
        self.assertIn("upp_arkaplan_bekle", c)
        self.assertIn("_upp_spawn_", c)

    def test_vektor3_mesafe(self) -> None:
        src = wrap(
            "    Vektor3 a;\n"
            "    Vektor3 b;\n"
            "    a.x = 0.0; a.y = 0.0; a.z = 0.0;\n"
            "    b.x = 3.0; b.y = 4.0; b.z = 0.0;\n"
            "    oto d = upp.matematik.mesafe(a, b);\n"
            "    oto eski = upp.matematik.mesafe_3d(0.0, 0.0, 0.0, 3.0, 4.0, 0.0);\n"
        )
        c = compile_ok(src)
        self.assertIn("upp_mat_mesafe_v3", c)
        self.assertIn("upp_mat_mesafe_3d", c)

    def test_tus_bas_bilinmeyen_red(self) -> None:
        src = wrap('    upp.girdi.tus_bas("XYZ");\n')
        msg = compile_err(src)
        self.assertIn("tus", msg.lower())

    def test_arkaplan_dur_red(self) -> None:
        src = wrap("    arkaplan { dur; }\n")
        msg = compile_err(src)
        self.assertIn("dur", msg)

    def test_koruma_degistir_sayi(self) -> None:
        src = wrap(
            "    guvensiz {\n"
            "        upp.bellek.koruma_degistir(0, 8, 64);\n"
            "        oto h = upp.bellek.son_hata();\n"
            "        oto ok = upp.bellek.okundu();\n"
            "    }\n"
        )
        c = compile_ok(src)
        self.assertIn("upp_bellek_koruma_degistir_sayi", c)
        self.assertIn("upp_bellek_son_hata", c)

    def test_c_kod_guvenli_red(self) -> None:
        src = wrap("    c_kod { int x = 0; }\n")
        msg = compile_err(src)
        self.assertIn("BELLEK GÜVENLİĞİ", msg)

    def test_guvenli_icinde_adres_red(self) -> None:
        src = wrap(
            "    guvensiz {\n"
            "        guvenli {\n"
            "            sayi x = 1;\n"
            "            oto p = &x;\n"
            "        }\n"
            "    }\n"
        )
        msg = compile_err(src)
        self.assertIn("BELLEK GÜVENLİĞİ", msg)

    def test_guvenli_sonra_guvensiz_adres_ok(self) -> None:
        src = wrap(
            "    sayi x = 1;\n"
            "    guvensiz {\n"
            "        guvenli {\n"
            "            sayi y = 2;\n"
            "        }\n"
            "        oto p = &x;\n"
            "        sayi z = *p;\n"
            "    }\n"
        )
        c = compile_ok(src)
        self.assertIn("/* guvenli */", c)
        self.assertIn("&x", c)

    def test_kilit_hazir_ve_kopya(self) -> None:
        src = wrap(
            "    Kilit a;\n"
            "    Kilit b = a;\n"
            "    a.kilitle();\n"
            "    a.birak();\n"
        )
        c = compile_ok(src)
        self.assertIn("upp_kilit_hazir(&a);", c)
        self.assertNotIn("upp_kilit_hazir(&b);", c)
        self.assertIn("Kilit b = a;", c)

    def test_interpolasyon(self) -> None:
        src = wrap(
            '    oto ad = "Arda";\n'
            "    oto n = 7;\n"
            '    upp.satir_yaz("merhaba {ad} {n}");\n'
        )
        c = compile_ok(src)
        self.assertIn("%s", c)
        self.assertIn("%lld", c)

    def test_metin_yardimcilari(self) -> None:
        src = wrap(
            '    oto s = "merhaba dunya";\n'
            "    oto a = upp.metin.kes(s, 0, 3);\n"
            '    oto i = upp.metin.icinde(s, "dunya");\n'
            '    oto d = upp.metin.degistir(s, "dunya", "upp");\n'
            "    upp.metin_bosalt(a);\n"
            "    upp.metin_bosalt(d);\n"
        )
        c = compile_ok(src)
        self.assertIn("upp_metin_kes", c)
        self.assertIn("upp_metin_icinde", c)
        self.assertIn("upp_metin_degistir", c)

    def test_arguman_dosya_zaman_parse(self) -> None:
        src = wrap(
            "    oto n = upp.arguman_sayisi();\n"
            "    oto a = upp.arguman(0);\n"
            '    oto var = upp.dosya_var_mi("selam.txt");\n'
            '    oto sil = upp.dosya_sil("yok.txt");\n'
            "    oto t = upp.zaman();\n"
            '    oto x = upp.metinden_sayiya("42");\n'
        )
        c = compile_ok(src)
        self.assertIn("upp_arguman_sayisi", c)
        self.assertIn("upp_arguman", c)
        self.assertIn("upp_dosya_var_mi", c)
        self.assertIn("upp_dosya_sil", c)
        self.assertIn("GetTickCount64", c)
        self.assertIn("atoll", c)
        self.assertIn("int main(int argc, char **argv)", c)

    def test_json_api_c_adlari(self) -> None:
        src = wrap(
            '    JSONDeger k = upp.json.ayristir("{\\"a\\":1}");\n'
            "    oto t = upp.json.tur(k);\n"
            "    oto n = upp.json.uzunluk(k);\n"
            '    JSONDeger a = upp.json.anahtar_al(k, "a");\n'
            "    oto s = upp.json.sayi_al(a);\n"
            "    JSONDeger d = upp.json.dizi();\n"
            "    upp.json.ekle(d, upp.json.sayi_yap(2));\n"
            "    JSONDeger o = upp.json.nesne();\n"
            '    upp.json.koy(o, "x", upp.json.metin_yap("y"));\n'
            "    oto yaz = upp.json.olustur(o);\n"
            "    upp.metin_bosalt(yaz);\n"
            "    upp.json.bosalt(k);\n"
            "    upp.json.bosalt(d);\n"
            "    upp.json.bosalt(o);\n"
        )
        c = compile_ok(src)
        self.assertIn("upp_json_ayristir", c)
        self.assertIn("upp_json_tur", c)
        self.assertIn("upp_json_anahtar_al", c)
        self.assertIn("upp_json_olustur", c)
        self.assertIn("JSONDeger", c)

    def test_json_yanlis_arguman(self) -> None:
        src = wrap('    JSONDeger k = upp.json.ayristir(1);\n')
        msg = compile_err(src)
        self.assertIn("metin", msg)

    def test_json_bilinmeyen_fonksiyon(self) -> None:
        src = wrap("    upp.json.yok_boyle_bir_sey();\n")
        msg = compile_err(src)
        self.assertIn("yerleşik", msg)

    def test_stdio_sistem_c_adlari(self) -> None:
        src = wrap(
            '    upp.stdio.yaz("{\\"ok\\":true}");\n'
            '    SurecCikti c = upp.sistem.calistir("echo ok");\n'
            "    eger (c.cikti != yok) { upp.metin_bosalt(c.cikti); }\n"
        )
        c = compile_ok(src)
        self.assertIn("upp_stdio_yaz", c)
        self.assertIn("upp_sistem_calistir", c)

    def test_json_hata_bayrak(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            src_path = os.path.join(tmp, "a.upp")
            with open(src_path, "w", encoding="utf-8") as handle:
                handle.write('fonk ana() -> sayi {\n    don "x";\n}\n')
            buf = io.StringIO()
            with redirect_stdout(buf):
                code = compile_file(
                    src_path,
                    emit_c_only=True,
                    json_hata=True,
                    cikti=os.path.join(tmp, "out"),
                )
            self.assertEqual(code, 1)
            raw = buf.getvalue().strip()
            self.assertNotIn("[u++", raw)
            data = json.loads(raw)
            self.assertFalse(data["ok"])
            self.assertEqual(data["hatalar"][0]["tur"], "hata")
            self.assertIn("Dönüş türü", data["hatalar"][0]["mesaj"])

    def test_json_hata_ok(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            src_path = os.path.join(tmp, "ok.upp")
            with open(src_path, "w", encoding="utf-8") as handle:
                handle.write("fonk ana() -> sayi { don 0; }\n")
            buf = io.StringIO()
            with redirect_stdout(buf):
                code = compile_file(
                    src_path,
                    emit_c_only=True,
                    json_hata=True,
                    cikti=os.path.join(tmp, "out"),
                )
            self.assertEqual(code, 0)
            data = json.loads(buf.getvalue().strip())
            self.assertTrue(data["ok"])
            self.assertEqual(data["hatalar"], [])

    def test_analiz_kaynak_tip_hatasi(self) -> None:
        from uppc import analiz_kaynak
        from unittest.mock import patch

        hatalar = analiz_kaynak('fonk ana() -> sayi {\n    don "x";\n}\n')
        self.assertEqual(len(hatalar), 1)
        self.assertEqual(hatalar[0]["tur"], "hata")
        self.assertIn("Dönüş türü", hatalar[0]["mesaj"])

    def test_analiz_kaynak_guvenlik(self) -> None:
        from uppc import analiz_kaynak

        src = "fonk ana() -> sayi {\n    sayi x = 10;\n    oto p = &x;\n    don 0;\n}\n"
        hatalar = analiz_kaynak(src)
        self.assertEqual(len(hatalar), 1)
        self.assertEqual(hatalar[0]["tur"], "guvenlik")

    def test_analiz_kaynak_coklu_guvenlik(self) -> None:
        from uppc import analiz_kaynak

        src = (
            "fonk ana() -> sayi {\n"
            "    sayi x = 10;\n"
            "    sayi y = 20;\n"
            "    oto p = &x;\n"
            "    oto q = &y;\n"
            "    don 0;\n"
            "}\n"
        )
        hatalar = analiz_kaynak(src)
        guv = [h for h in hatalar if h["tur"] == "guvenlik"]
        self.assertEqual(len(guv), 2)

    def test_tani_kayit_satir_sifir(self) -> None:
        from uppc import UppError, _tani_kayit

        kayit = _tani_kayit(UppError(0, "marker", 1), "hata")
        self.assertEqual(kayit["satir"], 0)
        self.assertEqual(kayit["sutun"], 1)

    def test_analiz_kaynak_temiz(self) -> None:
        from uppc import analiz_kaynak

        self.assertEqual(analiz_kaynak("fonk ana() -> sayi { don 0; }\n"), [])

    def test_analiz_gcc_ve_codegen_yok(self) -> None:
        from uppc import analiz_kaynak
        from unittest.mock import patch

        with patch("uppc.invoke_gcc") as gcc, patch("uppc.CodeGenerator.generate") as gen:
            analiz_kaynak("fonk ana() -> sayi { don 0; }\n")
            gcc.assert_not_called()
            gen.assert_not_called()

    def test_analiz_cli_json(self) -> None:
        from uppc import main as uppc_main

        with tempfile.TemporaryDirectory() as tmp:
            src_path = os.path.join(tmp, "a.upp")
            with open(src_path, "w", encoding="utf-8") as handle:
                handle.write("fonk ana() -> sayi { don 0; }\n")
            buf = io.StringIO()
            with redirect_stdout(buf):
                code = uppc_main([src_path, "--analiz"])
            self.assertEqual(code, 0)
            data = json.loads(buf.getvalue().strip())
            self.assertTrue(data["ok"])
            self.assertEqual(data["hatalar"], [])
            self.assertFalse(os.path.isfile(os.path.join(tmp, "cikti.c")))

    def test_ide_program_anasiz(self) -> None:
        from uppc import ide_program, FunctionDecl

        program = ide_program("fonk yardim() -> sayi { don 1; }\n")
        self.assertIsNotNone(program)
        adlar = [d.name for d in program.decls if isinstance(d, FunctionDecl)]
        self.assertEqual(adlar, ["yardim"])

    def test_lsp_tamamla_upp_nokta(self) -> None:
        sys.path.insert(0, os.path.join(ROOT, "eklenti", "server"))
        from analiz import tamamla, hover_metni

        src = "fonk ana() -> sayi {\n    upp.\n    don 0;\n}\n"
        labels = {it["label"] for it in tamamla(src, 1, 8)}
        self.assertIn("matematik", labels)
        self.assertIn("satir_yaz", labels)
        hover = hover_metni("fonk ana() -> sayi {\n    upp.satir_yaz();\n    don 0;\n}\n", 1, 8)
        self.assertIsNotNone(hover)
        self.assertIn("satir_yaz", hover["contents"]["value"])

    def test_lsp_tamamla_upp_kismi(self) -> None:
        sys.path.insert(0, os.path.join(ROOT, "eklenti", "server"))
        from analiz import tamamla

        src = "fonk ana() -> sayi {\n    upp.sa\n    don 0;\n}\n"
        labels = {it["label"] for it in tamamla(src, 1, len("    upp.sa"))}
        self.assertIn("satir_yaz", labels)
        self.assertNotIn("ses_cal", labels)
        self.assertNotIn("matematik", labels)

    def test_lsp_guvensiz_satir_sar(self) -> None:
        sys.path.insert(0, os.path.join(ROOT, "eklenti", "server"))
        from analiz import kod_eylemleri

        src = "fonk ana() -> sayi {\n    oto p = &x;\n    don 0;\n}\n"
        acts = kod_eylemleri(
            "file:///a.upp",
            src,
            {},
            [{
                "range": {
                    "start": {"line": 1, "character": 12},
                    "end": {"line": 1, "character": 13},
                },
                "message": "BELLEK GÜVENLİĞİ",
                "data": {"tur": "guvenlik"},
            }],
        )
        self.assertTrue(acts)
        duzelt = acts[0]["edit"]["changes"]["file:///a.upp"][0]
        self.assertEqual(duzelt["range"]["start"]["character"], 0)
        self.assertIn("guvensiz", duzelt["newText"])
        self.assertIn("oto p = &x;", duzelt["newText"])

    def test_lsp_imza_yaz_variadic(self) -> None:
        sys.path.insert(0, os.path.join(ROOT, "eklenti", "server"))
        from analiz import imza_yardim

        src = 'fonk ana() -> sayi {\n    upp.yaz("a", x,\n}\n'
        yardim = imza_yardim(src, 1, len('    upp.yaz("a", x,'))
        self.assertIsNotNone(yardim)
        etiket = [p["label"] for p in yardim["signatures"][0]["parameters"]]
        self.assertIn("...", etiket)
        self.assertGreaterEqual(yardim["activeParameter"], 1)

    def test_problem_matcher_native_parantez(self) -> None:
        import re

        py_satir = "[u++ HATA] Satır 7:5: ';' bekleniyordu, 'don' bulundu."
        nat_satir = "[u++ HATA] Satır 7:5 (tests/negative/01.upp): ';' bekleniyordu, 'don' bulundu."
        guven = "[u++ BELLEK GÜVENLİĞİ İHLALİ] Satır 6:9 (a.upp): işaretçi"
        rx = re.compile(r"^\[u\+\+ HATA\] Satır (\d+)(?::(\d+))?(?:\s*\(([^)]*)\))?: (.*)$")
        m1 = rx.match(py_satir)
        m2 = rx.match(nat_satir)
        self.assertIsNotNone(m1)
        self.assertIsNotNone(m2)
        self.assertEqual(m1.group(4), "';' bekleniyordu, 'don' bulundu.")
        self.assertEqual(m2.group(3), "tests/negative/01.upp")
        self.assertEqual(m2.group(4), "';' bekleniyordu, 'don' bulundu.")
        gx = re.compile(r"^\[u\+\+ BELLEK GÜVENLİĞİ İHLALİ\] Satır (\d+)(?::(\d+))?(?:\s*\(([^)]*)\))?: (.*)$")
        self.assertIsNotNone(gx.match(guven))

    def test_native_analiz_json(self) -> None:
        exe = os.path.join(ROOT, "derleme", "uppc.exe")
        if not os.path.isfile(exe):
            self.skipTest("derleme/uppc.exe yok")
        src = os.path.join(ROOT, "tests", "negative", "01_noktali_virgul.upp")
        ran = native_run(
            [exe, src, "--analiz"],
        )
        ham = (ran.stdout or "").strip()
        if not ham or not ham.lstrip().startswith("{"):
            self.skipTest("native --analiz henüz yok (yeniden derleyin)")
        data = json.loads(ham.splitlines()[-1])
        self.assertFalse(data["ok"])
        self.assertEqual(data["hatalar"][0]["tur"], "hata")
        self.assertNotEqual(ran.returncode, 0)

    def test_native_ast_yazdirir(self) -> None:
        exe = os.path.join(ROOT, "derleme", "uppc.exe")
        if not os.path.isfile(exe):
            self.skipTest("derleme/uppc.exe yok")
        src = os.path.join(ROOT, "ornekler", "ornek1.upp")
        if not os.path.isfile(src):
            self.skipTest("ornekler/ornek1.upp yok")
        ran = native_run(
            [exe, src, "--ast"],
        )
        out = ran.stdout or ""
        if "(AST üretildi)" in out and "Dugum tur=" not in out:
            self.skipTest("native --ast henüz yok (yeniden derleyin)")
        self.assertEqual(ran.returncode, 0)
        self.assertIn("Dugum tur=", out)
        self.assertNotIn("(AST üretildi)", out)

    def test_native_linux_sadece_c(self) -> None:
        exe = os.path.join(ROOT, "derleme", "uppc.exe")
        if not os.path.isfile(exe):
            self.skipTest("derleme/uppc.exe yok")
        src = os.path.join(ROOT, "ornekler", "ornek1.upp")
        if not os.path.isfile(src):
            self.skipTest("ornekler/ornek1.upp yok")
        with tempfile.TemporaryDirectory() as tmp:
            cikti = os.path.join(tmp, "nlinux")
            ran = native_run(
                [exe, src, "--linux", "--sadece-c", "--cikti", cikti],
            )
            c_path = cikti + ".c"
            if ran.returncode != 0 or not os.path.isfile(c_path):
                self.skipTest("native --linux henüz yok (yeniden derleyin)")
            with open(c_path, encoding="utf-8") as handle:
                c = handle.read()
            if "#define UPP_HEDEF_LINUX 1" not in c:
                self.skipTest("native --linux henüz yok (yeniden derleyin)")
            self.assertNotIn("#include <windows.h>", c)
            self.assertIn("#include <pthread.h>", c)
            self.assertIn("UPP_HEDEF_LINUX", c)

    def test_native_ornek3_analiz(self) -> None:
        exe = os.path.join(ROOT, "derleme", "uppc.exe")
        if not os.path.isfile(exe):
            self.skipTest("derleme/uppc.exe yok")
        src = os.path.join(ROOT, "ornekler", "ornek3.upp")
        ran = native_run(
            [exe, src, "--analiz"],
        )
        ham = (ran.stdout or "").strip()
        if not ham or not ham.lstrip().startswith("{"):
            self.skipTest("native --analiz henüz yok (yeniden derleyin)")
        data = json.loads(ham.splitlines()[-1])
        self.assertTrue(data.get("ok"), ham)
        self.assertEqual(ran.returncode, 0)

    def test_runtime_dosyadan(self) -> None:
        c = compile_ok("fonk ana() -> sayi { don 0; }\n")
        self.assertIn("u++ v2.8 çalışma zamanı", c)
        self.assertIn("Vektor3", c)
        self.assertIn("_upp_cizim_ekle", c)


class V26DilTestleri(unittest.TestCase):
    def test_bilesik_atama(self) -> None:
        c = compile_ok(wrap("    sayi n = 1;\n    n += 2;\n    n *= 3;\n    n -= 1;\n    n /= 2;\n    n %= 4;\n"))
        self.assertIn("+=", c)
        self.assertIn("*=", c)
        self.assertIn("-=", c)
        self.assertIn("/=", c)
        self.assertIn("%=", c)

    def test_metin_arti_atama(self) -> None:
        c = compile_ok(wrap('    oto s = "a";\n    s += "b";\n    upp.metin_bosalt(s);\n'))
        self.assertIn("upp_metin_ekle", c)

    def test_uclu_secim(self) -> None:
        c = compile_ok(wrap("    sayi n = 3;\n    oto x = n > 1 ? n : 0;\n"))
        self.assertIn("?", c)
        self.assertIn(":", c)

    def test_uclu_tur_uyumsuz(self) -> None:
        msg = compile_err(wrap('    oto x = 1 ? 2 : "a";\n'))
        self.assertIn("?:", msg)

    def test_sabit_c_const(self) -> None:
        c = compile_ok("sabit sayi MAX = 10;\nfonk ana() -> sayi {\n    don MAX;\n}\n")
        self.assertIn("const", c)
        self.assertIn("MAX", c)

    def test_sabit_yeniden_atama_red(self) -> None:
        msg = compile_err("sabit sayi MAX = 10;\nfonk ana() -> sayi {\n    MAX = 11;\n    don 0;\n}\n")
        self.assertIn("sabittir", msg)

    def test_uzunluk_isaretci_red(self) -> None:
        msg = compile_err(
            wrap(
                "    guvensiz {\n"
                "        sayi xs[] = [1, 2];\n"
                "        sayi* p = &xs[0];\n"
                "        oto n = upp.uzunluk(p);\n"
                "    }\n"
            )
        )
        self.assertIn("uzunluk", msg)

    def test_uzunluk_dizi(self) -> None:
        c = compile_ok(wrap("    sayi xs[] = [1, 2, 3];\n    oto n = upp.uzunluk(xs);\n"))
        self.assertIn("sizeof", c)

    def test_metin_baslar_kirp(self) -> None:
        c = compile_ok(
            wrap(
                '    oto a = upp.metin.baslar("abc", "a");\n'
                '    oto b = upp.metin.kirp("  x  ");\n'
                "    upp.metin_bosalt(b);\n"
            )
        )
        self.assertIn("upp_metin_baslar", c)
        self.assertIn("upp_metin_kirp", c)

    def test_matematik_pi_tan(self) -> None:
        c = compile_ok(wrap("    oto p = upp.matematik.pi();\n    oto t = upp.matematik.tan(p);\n"))
        self.assertIn("upp_mat_pi", c)
        self.assertIn("upp_mat_tan", c)

    def test_kullan_goreli(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            lib = os.path.join(tmp, "lib.upp")
            ana = os.path.join(tmp, "ana.upp")
            with open(lib, "w", encoding="utf-8") as handle:
                handle.write("fonk iki(sayi x) -> sayi { don x * 2; }\n")
            with open(ana, "w", encoding="utf-8") as handle:
                handle.write('kullan "lib.upp";\nfonk ana() -> sayi { don iki(21); }\n')
            with open(ana, encoding="utf-8") as handle:
                src = handle.read()
            c = compile_source(src, ana)
            self.assertIn("iki", c)
            self.assertIn("int main", c)

    def test_kullan_dongu(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            a = os.path.join(tmp, "a.upp")
            b = os.path.join(tmp, "b.upp")
            with open(a, "w", encoding="utf-8") as handle:
                handle.write('kullan "b.upp";\nfonk ana() -> sayi { don 0; }\n')
            with open(b, "w", encoding="utf-8") as handle:
                handle.write('kullan "a.upp";\nfonk yardim() -> bos { }\n')
            with open(a, encoding="utf-8") as handle:
                src = handle.read()
            buf = io.StringIO()
            with redirect_stderr(buf):
                try:
                    compile_source(src, a)
                    self.fail("döngü hatası bekleniyordu")
                except UppError as err:
                    self.assertIn("döngüsü", str(err))

    def test_kullan_elmas_tek_kez(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            d = os.path.join(tmp, "d.upp")
            b = os.path.join(tmp, "b.upp")
            c = os.path.join(tmp, "c.upp")
            a = os.path.join(tmp, "a.upp")
            with open(d, "w", encoding="utf-8") as handle:
                handle.write("fonk foo() -> sayi { don 1; }\n")
            with open(b, "w", encoding="utf-8") as handle:
                handle.write('kullan "d.upp";\n')
            with open(c, "w", encoding="utf-8") as handle:
                handle.write('kullan "d.upp";\n')
            with open(a, "w", encoding="utf-8") as handle:
                handle.write(
                    'kullan "b.upp";\nkullan "c.upp";\nfonk ana() -> sayi { don foo(); }\n'
                )
            with open(a, encoding="utf-8") as handle:
                src = handle.read()
            out = compile_source(src, a)
            self.assertIn("int main", out)
            self.assertEqual(out.count("long long foo("), 2)

    def test_kutuphane_ana_yasak(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            lib = os.path.join(tmp, "lib.upp")
            ana = os.path.join(tmp, "ana.upp")
            with open(lib, "w", encoding="utf-8") as handle:
                handle.write("fonk ana() -> sayi { don 0; }\n")
            with open(ana, "w", encoding="utf-8") as handle:
                handle.write('kullan "lib.upp";\nfonk ana() -> sayi { don 0; }\n')
            with open(ana, encoding="utf-8") as handle:
                src = handle.read()
            buf = io.StringIO()
            with redirect_stderr(buf):
                try:
                    compile_source(src, ana)
                    self.fail("kütüphane ana hatası bekleniyordu")
                except UppError as err:
                    self.assertIn("Kütüphane", str(err))

    def test_kullan_cift_fonk(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            lib = os.path.join(tmp, "lib.upp")
            ana = os.path.join(tmp, "ana.upp")
            with open(lib, "w", encoding="utf-8") as handle:
                handle.write("fonk foo() -> sayi { don 1; }\n")
            with open(ana, "w", encoding="utf-8") as handle:
                handle.write(
                    'kullan "lib.upp";\nfonk foo() -> sayi { don 2; }\nfonk ana() -> sayi { don foo(); }\n'
                )
            with open(ana, encoding="utf-8") as handle:
                src = handle.read()
            buf = io.StringIO()
            with redirect_stderr(buf):
                try:
                    compile_source(src, ana)
                    self.fail("çift tanım hatası bekleniyordu")
                except UppError as err:
                    self.assertIn("birden fazla", str(err))
                    self.assertTrue("lib.upp" in str(err) or "ana.upp" in str(err))

    def test_kullan_uzanti(self) -> None:
        src = 'kullan "lib.h";\nfonk ana() -> sayi { don 0; }\n'
        msg = compile_err(src)
        self.assertIn(".upp", msg)
        self.assertIn(".uph", msg)

    def test_kullan_uph(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            lib = os.path.join(tmp, "lib.uph")
            ana = os.path.join(tmp, "ana.upp")
            with open(lib, "w", encoding="utf-8") as handle:
                handle.write("fonk iki(sayi x) -> sayi { don x * 2; }\n")
            with open(ana, "w", encoding="utf-8") as handle:
                handle.write('kullan "lib.uph";\nfonk ana() -> sayi { don iki(21); }\n')
            with open(ana, encoding="utf-8") as handle:
                src = handle.read()
            c = compile_source(src, ana)
            self.assertIn("iki", c)

    def test_uph_kok_derleme_red(self) -> None:
        src = "fonk yardim() -> sayi { don 1; }\n"
        buf = io.StringIO()
        with tempfile.TemporaryDirectory() as tmp:
            yol = os.path.join(tmp, "lib.uph")
            with open(yol, "w", encoding="utf-8") as handle:
                handle.write(src)
            with redirect_stderr(buf):
                try:
                    compile_source(src, yol)
                    self.fail(".uph kök derleme hatası bekleniyordu")
                except UppError as err:
                    self.assertIn(".uph", str(err))

    def test_uph_analiz_anasiz(self) -> None:
        from uppc import analiz_kaynak

        with tempfile.TemporaryDirectory() as tmp:
            yol = os.path.join(tmp, "lib.uph")
            src = "fonk yardim() -> sayi { don 1; }\n"
            with open(yol, "w", encoding="utf-8") as handle:
                handle.write(src)
            self.assertEqual(analiz_kaynak(src, yol), [])

    def test_cift_sinif(self) -> None:
        src = (
            "sinif A { sayi x; }\n"
            "sinif A { sayi y; }\n"
            "fonk ana() -> sayi { don 0; }\n"
        )
        msg = compile_err(src)
        self.assertIn("birden fazla", msg)

    def test_cift_sabit(self) -> None:
        src = (
            "sabit sayi MAX = 1;\n"
            "sabit sayi MAX = 2;\n"
            "fonk ana() -> sayi { don 0; }\n"
        )
        msg = compile_err(src)
        self.assertIn("birden fazla", msg)

    def test_ayni_kapsam_yeniden(self) -> None:
        msg = compile_err(wrap("    sayi n = 1;\n    sayi n = 2;\n"))
        self.assertIn("birden fazla", msg)

    def test_yerlesik_sinif_yeniden(self) -> None:
        msg = compile_err("sinif Vektor3 { sayi x; }\nfonk ana() -> sayi { don 0; }\n")
        self.assertIn("yerleşik", msg)

    def test_secenek_sinif_cakis(self) -> None:
        msg = compile_err(
            "secenek Renk { Kirmizi }\n"
            "sinif Renk { sayi x; }\n"
            "fonk ana() -> sayi { don 0; }\n"
        )
        self.assertIn("seçenek", msg)

    def test_sinif_cift_metot(self) -> None:
        msg = compile_err(
            "sinif A {\n"
            "    fonk f() -> bos { }\n"
            "    fonk f() -> bos { }\n"
            "}\n"
            "fonk ana() -> sayi { don 0; }\n"
        )
        self.assertIn("birden fazla", msg)

    def test_ornek3_analiz_python(self) -> None:
        from uppc import analiz_kaynak

        path = os.path.join(ROOT, "ornekler", "ornek3.upp")
        with open(path, encoding="utf-8") as handle:
            src = handle.read()
        self.assertEqual(analiz_kaynak(src, path), [])

    def test_kutuphane_uph_analiz(self) -> None:
        from uppc import analiz_kaynak

        path = os.path.join(ROOT, "ornekler", "kutuphane.uph")
        with open(path, encoding="utf-8") as handle:
            src = handle.read()
        self.assertEqual(analiz_kaynak(src, path), [])

    def test_lsp_tanim_uph(self) -> None:
        sys.path.insert(0, os.path.join(ROOT, "eklenti", "server"))
        from analiz import tanim

        with tempfile.TemporaryDirectory() as tmp:
            lib = os.path.join(tmp, "lib.uph")
            ana = os.path.join(tmp, "ana.upp")
            with open(lib, "w", encoding="utf-8") as handle:
                handle.write("fonk iki(sayi x) -> sayi { don x * 2; }\n")
            src = 'kullan "lib.uph";\nfonk ana() -> sayi { don iki(21); }\n'
            with open(ana, "w", encoding="utf-8") as handle:
                handle.write(src)
            satir = src.splitlines()[1]
            kolon = satir.index("iki")
            locs = tanim(src, "file:///ana.upp", 1, kolon, ana)
            self.assertTrue(locs)
            self.assertTrue(
                any("lib.uph" in (loc.get("uri") or "").replace("\\", "/") for loc in locs),
                locs,
            )


class GccSozdizimi(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.gcc = find_gcc()

    def test_uretilen_c_derlenir(self) -> None:
        if not self.gcc:
            self.skipTest("gcc yok")
        src = wrap(
            '    oto s = "Merhaba" + " Dunya";\n'
            '    oto t = "a";\n'
            "    eger (t == \"a\") { }\n"
            "    A a;\n"
            "    a.x = 1;\n"
            "    sayi n = 3;\n"
            '    upp.mesaj("t", "x {n}");\n',
            extra="sinif A { sayi x; }\n"
            "fonk f() -> bos => upp.satir_yaz(\"ok\");\n",
        )
        c = compile_ok(src)
        fd, c_path = tempfile.mkstemp(suffix=".c", prefix="upp_test_")
        os.close(fd)
        exe_path = c_path[:-2] + ".exe"
        try:
            with open(c_path, "w", encoding="utf-8", newline="\n") as handle:
                handle.write(c)
            result = invoke_gcc(c_path, exe_path)
            self.assertEqual(result.returncode, 0, result.stderr)
        finally:
            for path in (c_path, exe_path):
                if os.path.isfile(path):
                    os.remove(path)

    def test_rastgele_buyuk_aralik(self) -> None:
        if not self.gcc:
            self.skipTest("gcc yok")
        src = (
            "fonk ana() -> sayi {\n"
            "    dongu (oto i = 0; i < 80; i = i + 1) {\n"
            "        oto x = upp.rastgele(0, 100000);\n"
            "        eger (x > 32767) {\n"
            "            don 0;\n"
            "        }\n"
            "    }\n"
            "    don 1;\n"
            "}\n"
        )
        c = compile_ok(src)
        self.assertIn("_upp_rand_u64", c)
        fd, c_path = tempfile.mkstemp(suffix=".c", prefix="upp_rand_")
        os.close(fd)
        exe_path = c_path[:-2] + ".exe"
        try:
            with open(c_path, "w", encoding="utf-8", newline="\n") as handle:
                handle.write(c)
            built = invoke_gcc(c_path, exe_path)
            self.assertEqual(built.returncode, 0, built.stderr)
            try:
                ran = subprocess.run([exe_path], cwd=os.getcwd())
            except OSError as err:
                if _denetim_engeli(err):
                    self.skipTest("Uygulama Denetimi üretilen exe'yi engelledi")
                raise
            self.assertEqual(
                ran.returncode,
                0,
                "upp.rastgele(0, 100000) 80 denemede 32767 tavanına takıldı",
            )
        finally:
            for path in (c_path, exe_path):
                if os.path.isfile(path):
                    os.remove(path)

    def test_ornekler_derlenir(self) -> None:
        if not self.gcc:
            self.skipTest("gcc yok")
        ornek_dir = os.path.join(ROOT, "ornekler")
        for name in ("ornek1.upp", "ornek2.upp", "ornek_json.upp", "ornek3.upp"):
            path = os.path.join(ornek_dir, name)
            with open(path, encoding="utf-8") as handle:
                src = handle.read()
            c = compile_source(src, path)
            fd, c_path = tempfile.mkstemp(suffix=".c", prefix="upp_demo_")
            os.close(fd)
            exe_path = c_path[:-2] + ".exe"
            try:
                with open(c_path, "w", encoding="utf-8", newline="\n") as handle:
                    handle.write(c)
                result = invoke_gcc(c_path, exe_path)
                self.assertEqual(result.returncode, 0, f"{name}: {result.stderr}")
            finally:
                for out in (c_path, exe_path):
                    if os.path.isfile(out):
                        os.remove(out)

    def test_linux_ornekler_c_derlenir(self) -> None:
        if not self.gcc:
            self.skipTest("gcc yok")
        ornek_dir = os.path.join(ROOT, "ornekler")
        for name in ("ornek1.upp", "ornek2.upp", "ornek3.upp"):
            path = os.path.join(ornek_dir, name)
            with open(path, encoding="utf-8") as handle:
                src = handle.read()
            c = compile_source(src, path, "linux")
            fd, c_path = tempfile.mkstemp(suffix=".c", prefix="upp_linux_")
            os.close(fd)
            exe_path = c_path[:-2] + (".out" if sys.platform != "win32" else ".exe")
            try:
                with open(c_path, "w", encoding="utf-8", newline="\n") as handle:
                    handle.write(c)
                result = invoke_gcc(c_path, exe_path, "linux")
                self.assertEqual(result.returncode, 0, f"{name} --linux: {result.stderr}")
            finally:
                for out in (c_path, exe_path):
                    if os.path.isfile(out):
                        os.remove(out)

    def test_json_ayristir_calisir(self) -> None:
        if not self.gcc:
            self.skipTest("gcc yok")
        src = (
            "fonk ana() -> sayi {\n"
            '    JSONDeger k = upp.json.ayristir("{\\"a\\":[1,2],\\"b\\":\\"x\\",\\"c\\":true,\\"d\\":null,\\"e\\":3.5}");\n'
            "    eger (!upp.json.var_mi(k)) { don 1; }\n"
            '    eger (upp.json.tur(k) != "nesne") { don 2; }\n'
            "    eger (upp.json.uzunluk(k) != 5) { don 3; }\n"
            '    JSONDeger a = upp.json.anahtar_al(k, "a");\n'
            "    eger (upp.json.sayi_al(upp.json.dizi_eleman(a, 0)) != 1) { don 4; }\n"
            '    oto b = upp.json.metin_al(upp.json.anahtar_al(k, "b"));\n'
            '    eger (b != "x") { don 5; }\n'
            '    eger (!upp.json.mantik_al(upp.json.anahtar_al(k, "c"))) { don 6; }\n'
            '    eger (upp.json.tur(upp.json.anahtar_al(k, "d")) != "yok") { don 7; }\n'
            '    eger (!upp.json.var_mi(upp.json.anahtar_al(k, "d"))) { don 8; }\n'
            '    eger (upp.json.var_mi(upp.json.anahtar_al(k, "yoktur"))) { don 9; }\n'
            "    JSONDeger n = upp.json.nesne();\n"
            '    upp.json.koy(n, "id", upp.json.sayi_yap(1));\n'
            "    oto s = upp.json.olustur(n);\n"
            '    eger (upp.metin.icinde(s, "id") < 0) { don 10; }\n'
            '    JSONDeger u = upp.json.ayristir("\\"\\\\u0041\\"");\n'
            "    oto us = upp.json.metin_al(u);\n"
            '    eger (us != "A") { don 11; }\n'
            '    JSONDeger bozuk = upp.json.ayristir("{");\n'
            "    eger (upp.json.var_mi(bozuk)) { don 12; }\n"
            "    sayi bi = 0;\n"
            "    iken (bi < 2000) {\n"
            '        JSONDeger b2 = upp.json.ayristir("{\\"a\\": [1,2,");\n'
            "        eger (upp.json.var_mi(b2)) { don 13; }\n"
            "        bi = bi + 1;\n"
            "    }\n"
            '    JSONDeger okj = upp.json.ayristir("{\\"z\\":1}");\n'
            "    eger (!upp.json.var_mi(okj)) { don 14; }\n"
            "    eger (upp.json.uzunluk(okj) != 1) { don 15; }\n"
            "    upp.json.bosalt(okj);\n"
            "    upp.metin_bosalt(s);\n"
            "    upp.metin_bosalt(b);\n"
            "    upp.metin_bosalt(us);\n"
            "    upp.json.bosalt(n);\n"
            "    upp.json.bosalt(k);\n"
            "    upp.json.bosalt(u);\n"
            "    don 0;\n"
            "}\n"
        )
        c = compile_ok(src)
        self.assertIn("upp_json_ayristir", c)
        fd, c_path = tempfile.mkstemp(suffix=".c", prefix="upp_json_")
        os.close(fd)
        exe_path = c_path[:-2] + ".exe"
        try:
            with open(c_path, "w", encoding="utf-8", newline="\n") as handle:
                handle.write(c)
            built = invoke_gcc(c_path, exe_path)
            self.assertEqual(built.returncode, 0, built.stderr)
            try:
                ran = subprocess.run([exe_path], cwd=os.getcwd())
            except OSError as err:
                if _denetim_engeli(err):
                    self.skipTest("Uygulama Denetimi üretilen exe'yi engelledi")
                raise
            self.assertEqual(ran.returncode, 0, f"json testi çıkış kodu {ran.returncode}")
        finally:
            for path in (c_path, exe_path):
                if os.path.isfile(path):
                    os.remove(path)

    def test_utf8_ve_liste_metin_kopya(self) -> None:
        if not self.gcc:
            self.skipTest("gcc yok")
        src = (
            "fonk ana() -> sayi {\n"
            '    eger (upp.uzunluk("ç") != 1) { don 1; }\n'
            '    eger (upp.metin.icinde("ağaç", "ç") != 3) { don 2; }\n'
            '    oto k = upp.metin.kes("ağaç", 2, 2);\n'
            '    eger (k != "aç") { don 3; }\n'
            "    upp.metin_bosalt(k);\n"
            "    liste[metin] xs;\n"
            '    xs.ekle("merhaba");\n'
            "    oto s = xs.al(0);\n"
            '    eger (s != "merhaba") { don 4; }\n'
            "    upp.metin_bosalt(s);\n"
            "    xs.bosalt();\n"
            "    don 0;\n"
            "}\n"
        )
        c = compile_ok(src)
        fd, c_path = tempfile.mkstemp(suffix=".c", prefix="upp_utf8_")
        os.close(fd)
        exe_path = c_path[:-2] + ".exe"
        try:
            with open(c_path, "w", encoding="utf-8", newline="\n") as handle:
                handle.write(c)
            built = invoke_gcc(c_path, exe_path)
            self.assertEqual(built.returncode, 0, built.stderr)
            try:
                ran = subprocess.run([exe_path], cwd=os.getcwd())
            except OSError as err:
                if _denetim_engeli(err):
                    self.skipTest("Uygulama Denetimi üretilen exe'yi engelledi")
                raise
            self.assertEqual(ran.returncode, 0, f"utf8/liste testi çıkış kodu {ran.returncode}")
        finally:
            for path in (c_path, exe_path):
                if os.path.isfile(path):
                    os.remove(path)


class V27DilTestleri(unittest.TestCase):
    def test_interpol_ifade(self) -> None:
        c = compile_ok(wrap('    metin ad = "Arda";\n    metin s = "merhaba {ad}";\n    upp.satir_yaz(s);\n    upp.metin_bosalt(s);\n'))
        self.assertIn("snprintf", c)
        self.assertIn("_upp_sn", c)

    def test_yaz_interpol_malloc_yok(self) -> None:
        c = compile_ok(wrap('    sayi n = 3;\n    upp.yaz("n={n}");\n'))
        self.assertIn("printf(", c)
        self.assertNotIn("_upp_sn", c)

    def test_uc_tirnak(self) -> None:
        src = "fonk ana() -> sayi {\n    metin s = \"\"\"merhaba\"\"\";\n    don 0;\n}\n"
        c = compile_ok(src)
        self.assertIn('"merhaba"', c)
        self.assertNotIn('"""', c)
        src2 = "fonk ana() -> sayi {\n    metin s = \"\"\"a\nb\"\"\";\n    don 0;\n}\n"
        c2 = compile_ok(src2)
        self.assertIn("\\n", c2)
        src3 = 'fonk ana() -> sayi {\n    metin s = """a\r\nb""";\n    don 0;\n}\n'
        c3 = compile_ok(src3)
        self.assertIn('"a\\nb"', c3)
        self.assertNotIn('"a\\r\\nb"', c3)

    def test_liste_harita_c(self) -> None:
        src = wrap(
            "    liste[sayi] xs;\n"
            "    xs.ekle(10);\n"
            "    xs[0] = 11;\n"
            "    oto n = xs.uzunluk();\n"
            '    harita[metin,sayi] h;\n'
            '    h.koy("a", 1);\n'
            "    oto v = h.al(\"a\");\n"
        )
        c = compile_ok(src)
        self.assertIn("UppKolListe", c)
        self.assertIn("upp_liste_ekle", c)
        self.assertIn("upp_liste_yaz", c)
        self.assertIn("UppKolHarita", c)
        self.assertIn("upp_harita_koy", c)

    def test_liste_sinif_alani(self) -> None:
        src = (
            "sinif Kutu {\n"
            "    liste[sayi] xs;\n"
            "    harita[metin,sayi] h;\n"
            "}\n"
            "fonk ana() -> sayi {\n"
            "    Kutu k;\n"
            "    k.xs.ekle(3);\n"
            "    don 0;\n"
            "}\n"
        )
        c = compile_ok(src)
        self.assertIn("UppKolListe xs", c)
        self.assertIn("UppKolHarita h", c)

    def test_liste_koseli_yok(self) -> None:
        err = compile_err(wrap("    liste xs;\n"))
        self.assertIn("[u++ HATA]", err)

    def test_harita_anahtar_ondalik(self) -> None:
        err = compile_err(wrap("    harita[ondalik,sayi] h;\n"))
        self.assertIn("[u++ HATA]", err)

    def test_secim_bos_durum_doner(self) -> None:
        src = (
            "fonk f(sayi x) -> sayi {\n"
            "    secim (x) {\n"
            "        durum 1:\n"
            "        durum 2:\n"
            "            don x;\n"
            "        varsayilan:\n"
            "            don 0;\n"
            "    }\n"
            "}\n"
            "fonk ana() -> sayi {\n"
            "    don f(2);\n"
            "}\n"
        )
        c = compile_ok(src)
        self.assertIn("case 1LL:", c)
        self.assertIn("return", c)

    def test_ic_ice_dizi_red(self) -> None:
        err = compile_err(wrap("    her (oto v in [[1, 2], [3, 4]]) { }\n"))
        self.assertIn("[u++ HATA]", err)
        self.assertIn("İç içe", err)

    def test_bellek_ondalik_adres_red(self) -> None:
        err = compile_err(wrap("    guvensiz { upp.bellek.sayioku(1.5); }\n"))
        self.assertIn("[u++ HATA]", err)
        self.assertIn("sayi", err)

    def test_v28_metin_kod_ve_ekle(self) -> None:
        c = compile_ok(wrap(
            '    metin s = "ab";\n'
            "    sayi k = s[0];\n"
            '    s += "c";\n'
            "    upp.metin_bosalt(s);\n"
            "    don k;\n"
        ))
        self.assertIn("upp_metin_kod", c)
        self.assertIn("upp_metin_ekle", c)

    def test_v28_hata_yaz_stderr(self) -> None:
        c = compile_ok(wrap('    upp.hata_yaz("x");\n    upp.hata_satir_yaz("y");\n'))
        self.assertIn("fprintf(stderr", c)

    def test_v28_surec_cikti(self) -> None:
        c = compile_ok(wrap(
            '    SurecCikti r = upp.sistem.calistir("echo ok");\n'
            "    don r.kod;\n"
        ))
        self.assertIn("upp_sistem_calistir", c)
        self.assertIn("SurecCikti", c)

    def test_v29_kurucu(self) -> None:
        src = (
            "sinif A {\n"
            "    sayi x;\n"
            "    fonk olustur(sayi n) { kendi.x = n; }\n"
            "}\n"
            "fonk ana() -> sayi {\n"
            "    A a = A(3);\n"
            "    don a.x;\n"
            "}\n"
        )
        c = compile_ok(src)
        self.assertIn("A_olustur", c)
        self.assertIn("_upp_ct", c)

    def test_v29_varsayilan_param(self) -> None:
        src = (
            "fonk top(sayi a, sayi b = 3) -> sayi { don a + b; }\n"
            "fonk ana() -> sayi { don top(2); }\n"
        )
        c = compile_ok(src)
        self.assertIn("top(2LL, 3LL)", c)

    def test_v29_varsayilan_param_red(self) -> None:
        src = (
            "fonk f(sayi a, sayi b = a) -> sayi { don b; }\n"
            "fonk ana() -> sayi { don f(1); }\n"
        )
        err = compile_err(src)
        self.assertIn("Varsayılan", err)

    def test_analiz_cok_hata(self) -> None:
        from uppc import analiz_kaynak

        src = (
            "fonk ana() -> sayi {\n"
            '    sayi a = "x";\n'
            '    sayi b = "y";\n'
            "    don 0;\n"
            "}\n"
        )
        hatalar = analiz_kaynak(src)
        self.assertGreaterEqual(len(hatalar), 2)


class NativeKaynakTestleri(unittest.TestCase):
    def test_uppc_kaynagi_c_kod_kullanmaz(self) -> None:
        src_dir = os.path.join(ROOT, "src", "uppc")
        for ad in os.listdir(src_dir):
            if not ad.endswith(".upp"):
                continue
            yol = os.path.join(src_dir, ad)
            with open(yol, encoding="utf-8") as handle:
                for i, satir in enumerate(handle, 1):
                    stripped = satir.strip()
                    self.assertFalse(
                        stripped.startswith("c_ekle "),
                        f"{ad}:{i} c_ekle",
                    )
                    self.assertNotIn("c_kod {", satir, f"{ad}:{i} c_kod")
                    self.assertFalse(
                        stripped.startswith("c_kod"),
                        f"{ad}:{i} c_kod deyimi",
                    )


if __name__ == "__main__":
    if sys.platform == "win32":
        for stream in (sys.stdout, sys.stderr):
            try:
                stream.reconfigure(encoding="utf-8")
            except Exception:
                pass
    unittest.main()
