# -*- coding: utf-8 -*-
"""u++ native derleyici modüllerini tek .upp dosyasında birleştirir."""
from __future__ import annotations

import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MODULLER = [
    "havuz.upp",
    "hata.upp",
    "ast.upp",
    "lexer.upp",
    "parser.upp",
    "tur.upp",
    "guvenlik.upp",
    "codegen.upp",
    "gomulu_runtime.upp",
    "surucu.upp",
]


def main() -> int:
    try:
        from gomulu_runtime_uret import runtime_gom
        runtime_gom()
    except Exception as e:
        print("[birlestir] gomulu runtime uretilemedi:", e)

    src_dir = os.path.join(ROOT, "src", "uppc")
    out_dir = os.path.join(ROOT, "derleyici")
    os.makedirs(out_dir, exist_ok=True)
    out_upp = os.path.join(out_dir, "uppc_birlesik.upp")
    lines_out: list[str] = []
    for name in MODULLER:
        path = os.path.join(src_dir, name)
        if not os.path.isfile(path):
            print("[birlestir] eksik modül:", path)
            return 1
        with open(path, encoding="utf-8") as handle:
            raw = handle.read()
        if raw.startswith("\ufeff"):
            raw = raw[1:]
        src_lines = raw.splitlines()
        lines_out.append(f"// @upp-kaynak {name}")
        lines_out.extend(src_lines)
        lines_out.append("")
    with open(out_upp, "w", encoding="utf-8", newline="\n") as handle:
        handle.write("\n".join(lines_out) + "\n")
    print(f"[birlestir] {out_upp}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
