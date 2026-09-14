#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""u++ dil sunucusu — stdio JSON-RPC (LSP). stdout yalnızca protokol; günlük stderr."""
from __future__ import annotations

import json
import os
import sys
import threading
import traceback
from typing import Any, Dict, Optional
from urllib.parse import unquote, urlparse
from urllib.request import url2pathname

SERVER_DIR = os.path.dirname(os.path.abspath(__file__))
if SERVER_DIR not in sys.path:
    sys.path.insert(0, SERVER_DIR)


def _kok_ekle(kok: str) -> None:
    if kok and os.path.isdir(kok) and kok not in sys.path:
        sys.path.insert(0, kok)
    os.environ["UPP_KOK"] = kok or os.environ.get("UPP_KOK", "")


_kok_ekle(os.environ.get("UPP_KOK") or os.environ.get("PYTHONPATH", "").split(os.pathsep)[0])

from analiz import (  # noqa: E402
    belge_simgeleri,
    hover_metni,
    imza_yardim,
    kod_eylemleri,
    referanslar,
    tamamla,
    tani,
    tani_gcc,
    tanim,
)


def _log(msg: str) -> None:
    sys.stderr.write(msg + "\n")
    sys.stderr.flush()


def uri_yol(uri: str) -> str:
    parsed = urlparse(uri)
    path = unquote(parsed.path or "")
    if sys.platform == "win32" and path.startswith("/") and len(path) >= 3 and path[2] == ":":
        path = path[1:]
    if parsed.scheme == "file":
        try:
            return url2pathname(unquote(parsed.path))
        except Exception:
            return path
    return path


def yol_uri(path: str) -> str:
    abs_path = os.path.abspath(path)
    if sys.platform == "win32":
        return "file:///" + abs_path.replace("\\", "/")
    return "file://" + abs_path


class Sunucu:
    def __init__(self) -> None:
        self.belgeler: Dict[str, str] = {}
        self.zamanlayicilar: Dict[str, threading.Timer] = {}
        self.nesil: Dict[str, int] = {}
        self.kilit = threading.Lock()
        self.kok = os.environ.get("UPP_KOK", "")
        self.derleyici = "python"
        self.uppc_yolu = ""
        self.gcc_tanilari = False
        self.kapan = False

    def ayarla(self, opts: Dict[str, Any], config: Optional[Dict[str, Any]] = None) -> None:
        if opts.get("repoRoot"):
            self.kok = str(opts["repoRoot"])
            _kok_ekle(self.kok)
        cfg = config or {}
        upp = cfg.get("upp") if isinstance(cfg.get("upp"), dict) else cfg
        if not isinstance(upp, dict):
            upp = {}
        if upp.get("derleyici"):
            self.derleyici = str(upp["derleyici"])
        if "uppcYolu" in upp:
            self.uppc_yolu = str(upp.get("uppcYolu") or "")
        if "gccTanilari" in upp:
            self.gcc_tanilari = bool(upp.get("gccTanilari"))
        if not self.uppc_yolu and self.kok:
            aday = os.path.join(self.kok, "derleme", "uppc.exe")
            if os.path.isfile(aday):
                self.uppc_yolu = aday

    def kaynak(self, uri: str) -> str:
        return self.belgeler.get(uri, "")

    def belge_kapat(self, uri: str, rpc: "Rpc") -> None:
        if not uri:
            return
        with self.kilit:
            eski = self.zamanlayicilar.pop(uri, None)
            if eski:
                eski.cancel()
            self.nesil[uri] = self.nesil.get(uri, 0) + 1
            self.belgeler.pop(uri, None)
            rpc.notify("textDocument/publishDiagnostics", {"uri": uri, "diagnostics": []})

    def tani_yayinla(self, uri: str, rpc: "Rpc", gcc: bool = False) -> None:
        with self.kilit:
            if uri not in self.belgeler:
                return
            nesil = self.nesil.get(uri, 0) + 1
            self.nesil[uri] = nesil
            kaynak = self.belgeler[uri]
            derleyici = self.derleyici
            uppc = self.uppc_yolu
            gcc_iste = bool(gcc and self.gcc_tanilari)
            kok = self.kok
        dosya = uri_yol(uri) or "tampon.upp"
        try:
            hatalar = tani(kaynak, dosya, derleyici, uppc)
            if gcc_iste and not hatalar:
                hatalar = tani_gcc(kaynak, dosya, derleyici, uppc, kok)
        except Exception as err:
            _log("analiz hata: " + traceback.format_exc())
            hatalar = [{"satir": 1, "sutun": 1, "tur": "hata", "mesaj": str(err)}]
        with self.kilit:
            if self.nesil.get(uri) != nesil or uri not in self.belgeler:
                return
            diags = [self._lsp_diag(h) for h in hatalar]
            rpc.notify("textDocument/publishDiagnostics", {"uri": uri, "diagnostics": diags})

    def tani_gecikmeli(self, uri: str, rpc: "Rpc", sn: float = 0.3) -> None:
        with self.kilit:
            eski = self.zamanlayicilar.pop(uri, None)
            if eski:
                eski.cancel()

            def calis(hedef: str = uri) -> None:
                with self.kilit:
                    self.zamanlayicilar.pop(hedef, None)
                self.tani_yayinla(hedef, rpc)

            timer = threading.Timer(sn, calis)
            self.zamanlayicilar[uri] = timer
            timer.daemon = True
            timer.start()

    @staticmethod
    def _lsp_diag(h: Dict[str, Any]) -> Dict[str, Any]:
        satir = max(1, int(h.get("satir") or 1))
        sutun = max(1, int(h.get("sutun") or 1))
        son_satir = max(satir, int(h.get("son_satir") or satir))
        son_sutun = int(h.get("son_sutun") or (sutun + 1))
        tur = str(h.get("tur") or "hata")
        return {
            "range": {
                "start": {"line": satir - 1, "character": sutun - 1},
                "end": {"line": son_satir - 1, "character": max(son_sutun - 1, sutun)},
            },
            "severity": 1,
            "source": "gcc" if tur == "gcc" else "u++",
            "message": str(h.get("mesaj") or ""),
            "data": {"tur": tur},
        }


class Rpc:
    def __init__(self, sunucu: Sunucu) -> None:
        self.sunucu = sunucu
        self.stdin = sys.stdin.buffer
        self.stdout = sys.stdout.buffer
        self._yaz_kilit = threading.Lock()

    def notify(self, method: str, params: Any) -> None:
        self._send({"jsonrpc": "2.0", "method": method, "params": params})

    def _send(self, obj: Dict[str, Any]) -> None:
        ham = json.dumps(obj, ensure_ascii=False, separators=(",", ":")).encode("utf-8")
        with self._yaz_kilit:
            self.stdout.write(f"Content-Length: {len(ham)}\r\n\r\n".encode("ascii") + ham)
            self.stdout.flush()

    def _yanit(self, req_id: Any, result: Any) -> None:
        self._send({"jsonrpc": "2.0", "id": req_id, "result": result})

    def _hata(self, req_id: Any, kod: int, mesaj: str) -> None:
        self._send({"jsonrpc": "2.0", "id": req_id, "error": {"code": kod, "message": mesaj}})

    def oku(self) -> Optional[Dict[str, Any]]:
        baslik: Dict[str, str] = {}
        while True:
            satir = self.stdin.readline()
            if not satir:
                return None
            if satir in (b"\r\n", b"\n"):
                break
            try:
                line = satir.decode("ascii")
            except UnicodeDecodeError:
                line = satir.decode("utf-8", errors="replace")
            if ":" in line:
                k, v = line.split(":", 1)
                baslik[k.strip().lower()] = v.strip()
        n = int(baslik.get("content-length", "0") or 0)
        govde = self.stdin.read(n) if n else b"{}"
        try:
            return json.loads(govde.decode("utf-8"))
        except json.JSONDecodeError:
            _log("bozuk JSON-RPC")
            return {}

    def isle(self, msg: Dict[str, Any]) -> None:
        method = msg.get("method")
        req_id = msg.get("id")
        params = msg.get("params") or {}
        s = self.sunucu
        try:
            if method == "initialize":
                init_opts = params.get("initializationOptions") or {}
                if not isinstance(init_opts, dict):
                    init_opts = {}
                cfg = {"upp": init_opts.get("settings") or init_opts}
                s.ayarla(init_opts, cfg)
                self._yanit(req_id, {
                    "capabilities": {
                        "textDocumentSync": 1,
                        "completionProvider": {"triggerCharacters": ["."]},
                        "hoverProvider": True,
                        "documentSymbolProvider": True,
                        "definitionProvider": True,
                        "referencesProvider": True,
                        "signatureHelpProvider": {"triggerCharacters": ["(", ","]},
                        "codeActionProvider": True,
                    },
                    "serverInfo": {"name": "u++", "version": "3.0"},
                })
                return
            if method == "initialized":
                return
            if method == "shutdown":
                self._yanit(req_id, None)
                return
            if method == "exit":
                s.kapan = True
                return
            if method == "workspace/didChangeConfiguration":
                ayar = params.get("settings") or {}
                s.ayarla({}, ayar if isinstance(ayar, dict) else {})
                return
            if method == "textDocument/didOpen":
                doc = params.get("textDocument") or {}
                uri = doc.get("uri")
                s.belgeler[uri] = doc.get("text") or ""
                s.tani_yayinla(uri, self)
                return
            if method == "textDocument/didChange":
                doc = params.get("textDocument") or {}
                uri = doc.get("uri")
                deg = params.get("contentChanges") or []
                if deg:
                    s.belgeler[uri] = deg[-1].get("text") or ""
                s.tani_gecikmeli(uri, self)
                return
            if method == "textDocument/didSave":
                doc = params.get("textDocument") or {}
                uri = doc.get("uri")
                if uri:
                    s.tani_yayinla(uri, self, gcc=s.gcc_tanilari)
                return
            if method == "textDocument/didClose":
                doc = params.get("textDocument") or {}
                uri = doc.get("uri")
                s.belge_kapat(uri, self)
                return
            if method == "textDocument/completion":
                doc = params.get("textDocument") or {}
                pos = params.get("position") or {}
                uri = doc.get("uri")
                yol = uri_yol(uri) if uri else "<girdi>"
                items = tamamla(s.kaynak(uri), int(pos.get("line") or 0), int(pos.get("character") or 0), yol)
                self._yanit(req_id, {"isIncomplete": False, "items": items})
                return
            if method == "textDocument/hover":
                doc = params.get("textDocument") or {}
                pos = params.get("position") or {}
                uri = doc.get("uri")
                yol = uri_yol(uri) if uri else "<girdi>"
                hover = hover_metni(s.kaynak(uri), int(pos.get("line") or 0), int(pos.get("character") or 0), yol)
                self._yanit(req_id, hover)
                return
            if method == "textDocument/documentSymbol":
                doc = params.get("textDocument") or {}
                uri = doc.get("uri")
                yol = uri_yol(uri) if uri else "<girdi>"
                self._yanit(req_id, belge_simgeleri(s.kaynak(uri), yol))
                return
            if method == "textDocument/definition":
                doc = params.get("textDocument") or {}
                pos = params.get("position") or {}
                uri = doc.get("uri")
                yol = uri_yol(uri) if uri else "<girdi>"
                self._yanit(req_id, tanim(s.kaynak(uri), uri, int(pos.get("line") or 0), int(pos.get("character") or 0), yol))
                return
            if method == "textDocument/references":
                doc = params.get("textDocument") or {}
                pos = params.get("position") or {}
                uri = doc.get("uri")
                self._yanit(req_id, referanslar(s.kaynak(uri), uri, int(pos.get("line") or 0), int(pos.get("character") or 0)))
                return
            if method == "textDocument/signatureHelp":
                doc = params.get("textDocument") or {}
                pos = params.get("position") or {}
                uri = doc.get("uri")
                yol = uri_yol(uri) if uri else "<girdi>"
                self._yanit(req_id, imza_yardim(s.kaynak(uri), int(pos.get("line") or 0), int(pos.get("character") or 0), yol))
                return
            if method == "textDocument/codeAction":
                doc = params.get("textDocument") or {}
                uri = doc.get("uri")
                rng = params.get("range") or {}
                ctx = params.get("context") or {}
                actions = kod_eylemleri(uri, s.kaynak(uri), rng, ctx.get("diagnostics") or [])
                self._yanit(req_id, actions)
                return
            if req_id is not None:
                self._hata(req_id, -32601, f"Bilinmeyen yöntem: {method}")
        except Exception:
            _log(traceback.format_exc())
            if req_id is not None:
                self._hata(req_id, -32603, "İç sunucu hatası")


def main() -> int:
    if sys.platform == "win32":
        try:
            sys.stderr.reconfigure(encoding="utf-8")
        except Exception:
            pass
    sunucu = Sunucu()
    rpc = Rpc(sunucu)
    _log("u++ LSP hazır")
    while not sunucu.kapan:
        msg = rpc.oku()
        if msg is None:
            break
        if not msg:
            continue
        rpc.isle(msg)
    return 0


if __name__ == "__main__":
    sys.exit(main())
