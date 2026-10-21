# Limine (gömülü)

Uppix'i 64-bit long mode'da açan önyükleyici. Dosyalar değiştirilmeden alınmıştır:

| Dosya | Kaynak |
|---|---|
| `limine-bios.sys`, `BOOTX64.EFI`, `limine.c`, `limine-bios-hdd.h`, `LICENSE` | [limine-bootloader/limine](https://github.com/limine-bootloader/limine), `v11.x-binary` dalı, **v11.4.1** (`5be26a73d7b7b4d4477d18be94e1d16e615adf56`), BSD-2-Clause |
| `limine.h`, `LICENSE.protocol` | [limine-bootloader/limine-protocol](https://github.com/limine-bootloader/limine-protocol) (`3a0526b700e356f0eac1b71a77697b3fd1c707a3`), 0BSD |

- `limine-bios.sys`: BIOS'ta 3. aşama; disk imajında `/limine/` altında durur.
- `BOOTX64.EFI`: UEFI önyükleyicisi; `/EFI/BOOT/` altında durur.
- `limine.c` + `limine-bios-hdd.h`: `limine bios-install` aracı. Bu araç MBR'ye ve bölüm boşluğuna 1. ve 2. aşamayı yazar. [`disk_yap.sh`](../../disk_yap.sh) onu `cc` ile derler.
- `limine.h`: protokol başvurusu. Uppix istekleri bu başlığı içermez: kimlikler, uppc'nin ürettiği önyükleme asm'sindedir (`src/uppc/cekirdek.upp`, `ck_limine_s`). Bu dosya yalnızca karşılaştırma için durur.

Güncellemek için: dosyaları yeni bir `vX.x-binary` sürümünden kopyalayın ve bu tabloyu güncelleyin.
