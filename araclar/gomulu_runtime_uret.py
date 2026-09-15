# -*- coding: utf-8 -*-
"""
u++ runtime dosyasını (src/runtime/upp_runtime.c) hex dizgeleri halinde
src/uppc/gomulu_runtime.upp içerisine gömer.
Böylece interpolasyon çakışması, kaçış hatası veya derleyici sınırı olmadan
tek başına (standalone) çalışır.
"""

import os
import sys

def runtime_gom():
    kok = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    c_yol = os.path.join(kok, "src", "runtime", "upp_runtime.c")
    hedef_upp = os.path.join(kok, "src", "uppc", "gomulu_runtime.upp")

    if not os.path.exists(c_yol):
        print(f"HATA: {c_yol} bulunamadı!")
        sys.exit(1)

    with open(c_yol, "rb") as f:
        raw_bytes = f.read()

    chunk_size = 1024  # 1 KB = 2048 hex chars per line
    chunks = []
    for i in range(0, len(raw_bytes), chunk_size):
        chunk = raw_bytes[i:i + chunk_size]
        chunks.append(chunk.hex())

    with open(hedef_upp, "w", encoding="utf-8") as f:
        f.write("// Otomatik üretilmiştir — src/runtime/upp_runtime.c gömülü kopyası\n")
        f.write("fonk os_runtime_gomulu_yay() -> bos {\n")
        for hex_str in chunks:
            f.write(f'    tampon_yaz(upp.metin.hex_coz("{hex_str}"));\n')
        f.write("}\n")

    print(f"[u++] Gömülü runtime üretildi: {hedef_upp} ({len(raw_bytes)} bayt, {len(chunks)} parça)")

if __name__ == "__main__":
    runtime_gom()
