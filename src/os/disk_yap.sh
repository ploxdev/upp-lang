#!/bin/sh
# Uppix disk imajları.
#
#   sh disk_yap.sh [ÇIKTI]              ayrı imajlar (QEMU -kernel / Multiboot ile):
#       uppix-kok.img  64 MiB ext4 "uppix-kok": kök dosya sistemi (kok/ ağacından)
#       usb-fat32.img  64 MiB, MBR + FAT32 bölümü "UPPIXUSB" (örnek dosyalarla)
#       usb-exfat.img  32 MiB exFAT "UPPIXEX" (bölümsüz)
#
#   sh disk_yap.sh ÇIKTI ÇEKİRDEK.elf   önyüklenebilir imaj (Limine, 64-bit):
#       uppix.img      192 MiB, MBR:
#                        1: 64 MiB FAT32 "UPPIXBOOT" (ESP): /limine, /EFI/BOOT, /uppix.elf → Uppix'te /boot
#                        2: 127 MiB ext4 "uppix-kok": kök (/)
#                      Limine BIOS aşamaları MBR'ye yazılır; UEFI için BOOTX64.EFI da vardır.
#                      İmaj zaten varsa yalnızca çekirdek ve /usr/bin'deki kullanıcı
#                      programları güncellenir (kök içeriği korunur).
#
#   Kullanıcı programları (kullanici/*.c) libc'siz statik Linux ELF olarak
#   ÇIKTI/kullanici/ altına derlenir ve kökte /usr/bin'e konur.
#       usb-fat32.img, usb-exfat.img yoksa oluşturulur.
#
# Gerekenler: mkfs.ext4 (e2fsprogs), sfdisk, mkfs.fat + mtools (dosfstools, mtools),
# mkfs.exfat (exfatprogs), cc. FS_ARACLAR, paketlerin açıldığı dizini gösterebilir.
set -e
BURA=$(cd "$(dirname "$0")" && pwd)
CIKTI=${1:-derleme}
CEKIRDEK=$2
case "$CIKTI" in /*) ;; *) CIKTI="$(pwd)/$CIKTI" ;; esac
if [ -n "$CEKIRDEK" ]; then
    case "$CEKIRDEK" in /*) ;; *) CEKIRDEK="$(pwd)/$CEKIRDEK" ;; esac
fi
if [ -n "$FS_ARACLAR" ]; then
    PATH="$FS_ARACLAR/usr/sbin:$FS_ARACLAR/usr/bin:$FS_ARACLAR/sbin:$PATH"
fi
PATH="$PATH:/sbin:/usr/sbin"
LC_ALL=C.UTF-8
export PATH MTOOLS_SKIP_CHECK=1 LC_ALL
mkdir -p "$CIKTI"

gerek() {
    for a in "$@"; do
        if ! command -v "$a" >/dev/null 2>&1; then
            echo "[disk] HATA: '$a' bulunamadı (sudo apt install e2fsprogs dosfstools mtools exfatprogs fdisk)" >&2
            exit 1
        fi
    done
}

# Kullanıcı programları: yalnızca Linux sistem çağrılarıyla konuşan statik ELF'ler
# (aynı ikili gerçek Linux'ta da çalışır).
kullanici_derle() {
    KD="$CIKTI/kullanici"
    mkdir -p "$KD"
    if ! command -v cc >/dev/null 2>&1; then
        echo "[disk] cc yok: kullanıcı programları derlenmedi"
        return 0
    fi
    for c in "$BURA"/kullanici/*.c; do
        [ -f "$c" ] || continue
        cc -static -nostdlib -fno-pie -no-pie -ffreestanding -fno-builtin -fno-stack-protector \
            -fno-asynchronous-unwind-tables -O2 -Wall -Wl,--build-id=none -Wl,-z,noexecstack \
            -o "$KD/$(basename "$c" .c)" "$c"
    done
}

# Var olan imajın ext4 kökünde /usr/bin'deki programları yeniler. $1 imaj, $2 bölüm ofseti (bayt).
kullanici_guncelle() {
    [ -d "$CIKTI/kullanici" ] || return 0
    K=$(mktemp)
    echo "cd /usr/bin" > "$K"
    for p in "$CIKTI/kullanici"/*; do
        [ -f "$p" ] || continue
        ad=$(basename "$p")
        echo "rm $ad" >> "$K"
        echo "write $p $ad" >> "$K"
        echo "set_inode_field $ad mode 0100755" >> "$K"
        echo "set_inode_field $ad uid 0" >> "$K"
        echo "set_inode_field $ad gid 0" >> "$K"
    done
    debugfs -w -f "$K" "$1?offset=$2" >/dev/null 2>&1 || echo "[disk] uyarı: /usr/bin güncellenemedi"
    rm -f "$K"
}

# ext4 kök: $1 = imaj dosyası, $2 = boyut
ext4_kok() {
    # Hazırlık Linux dosya sisteminde yapılır: Windows diskinde (/mnt/c) izinler korunmaz.
    HAZIR=$(mktemp -d)
    cp -R "$BURA/kok/." "$HAZIR/"
    for d in bin boot dev etc home mnt proc root tmp usr/bin usr/share var/log; do
        mkdir -p "$HAZIR/$d"
    done
    find "$HAZIR" -type d -exec chmod 755 {} +
    find "$HAZIR" -type f -exec chmod 644 {} +
    chmod 1777 "$HAZIR/tmp"
    chmod 700 "$HAZIR/root"
    chmod 755 "$HAZIR"/usr/share/ornekler/*.sh
    if [ -d "$CIKTI/kullanici" ]; then
        for p in "$CIKTI/kullanici"/*; do
            [ -f "$p" ] || continue
            cp "$p" "$HAZIR/usr/bin/"
            chmod 755 "$HAZIR/usr/bin/$(basename "$p")"
        done
    fi
    if [ -n "$3" ]; then
        cp "$3" "$HAZIR/boot/"
    fi
    rm -f "$1"
    if [ "$(id -u)" = "0" ]; then
        mkfs.ext4 -q -F -L uppix-kok -b 4096 -I 256 -d "$HAZIR" "$1" "$2"
    else
        # root değilken -d dosya sahipliğini kullanıcıdan alır; sahipleri sonra 0:0 yap.
        mkfs.ext4 -q -F -L uppix-kok -b 4096 -I 256 -E root_owner=0:0 -d "$HAZIR" "$1" "$2"
        ( cd "$HAZIR" && find . -mindepth 1 | sed 's|^\.||' ) | while read -r yol; do
            echo "set_inode_field \"$yol\" uid 0"
            echo "set_inode_field \"$yol\" gid 0"
        done > "$1.debugfs"
        debugfs -w -f "$1.debugfs" "$1" >/dev/null 2>&1 || true
        rm -f "$1.debugfs"
    fi
    rm -rf "$HAZIR"
}

usb_fat32() {
    rm -f "$CIKTI/usb-fat32.img"
    truncate -s 64M "$CIKTI/usb-fat32.img"
    if command -v mkfs.fat >/dev/null 2>&1 && command -v mcopy >/dev/null 2>&1 && command -v sfdisk >/dev/null 2>&1; then
        echo 'label: dos
start=2048, type=c' | sfdisk -q "$CIKTI/usb-fat32.img"
        mkfs.fat -F 32 -n UPPIXUSB --offset=2048 "$CIKTI/usb-fat32.img" $(( (65536 * 1024 / 512 - 2048) / 2 )) >/dev/null
        G=$(mktemp -d)
        mkdir -p "$G/Belgeler"
        printf 'Bu dosya Linux'"'"'ta mtools ile FAT32 diske yazildi.\nUppix onu okuyabilir.\n' > "$G/BENIOKU.TXT"
        printf 'Uzun ve Türkçe bir dosya adı: şğüöçı ŞĞÜÖÇİ\n' > "$G/Türkçe uzun dosya adı.txt"
        printf 'alışveriş: ekmek, süt\n' > "$G/Belgeler/notlar.txt"
        mcopy -i "$CIKTI/usb-fat32.img@@1M" -s "$G/BENIOKU.TXT" "$G/Türkçe uzun dosya adı.txt" "$G/Belgeler" ::/
        rm -rf "$G"
        echo "[disk] FAT32 (MBR bölüm 1): $CIKTI/usb-fat32.img"
    else
        echo "[disk] mkfs.fat/mtools/sfdisk yok: usb-fat32.img biçimlendirilmedi"
    fi
}

usb_exfat() {
    rm -f "$CIKTI/usb-exfat.img"
    truncate -s 32M "$CIKTI/usb-exfat.img"
    if command -v mkfs.exfat >/dev/null 2>&1; then
        mkfs.exfat -L UPPIXEX "$CIKTI/usb-exfat.img" >/dev/null
        echo "[disk] exFAT: $CIKTI/usb-exfat.img"
    else
        echo "[disk] mkfs.exfat yok: usb-exfat.img biçimlendirilmedi"
    fi
}

# ---------------------------------------------------------------------------
if [ -z "$CEKIRDEK" ]; then
    gerek mkfs.ext4 debugfs
    kullanici_derle
    MB=""
    [ -f "$BURA/derleme/uppix32.elf" ] && MB="$BURA/derleme/uppix32.elf"
    ext4_kok "$CIKTI/uppix-kok.img" 64M "$MB"
    echo "[disk] ext4 kök: $CIKTI/uppix-kok.img"
    usb_fat32
    usb_exfat
    exit 0
fi

# ---- Limine ile önyüklenebilir imaj ----
[ -f "$CEKIRDEK" ] || { echo "[disk] HATA: çekirdek yok: $CEKIRDEK" >&2; exit 1; }
LIMINE="$BURA/onyukleyici/limine"
IMAJ="$CIKTI/uppix.img"
gerek mcopy mmd
kullanici_derle
if [ -f "$IMAJ" ]; then
    mcopy -o -i "$IMAJ@@1M" "$CEKIRDEK" ::/uppix.elf
    kullanici_guncelle "$IMAJ" $((133120 * 512))
    echo "[disk] çekirdek ve kullanıcı programları güncellendi: $IMAJ (FAT32 /uppix.elf, ext4 /usr/bin)"
else
    gerek mkfs.ext4 debugfs sfdisk mkfs.fat cc
    # İmaj Linux geçici dizininde kurulur ve sonunda tek seferde kopyalanır:
    # Windows diskine (/mnt/c) küçük bloklarla yazmak çok yavaştır.
    IS=$(mktemp -d)
    YIMAJ="$IS/uppix.img"
    # Limine bios-install aracı (gömülü kaynaktan)
    ARAC="$IS/limine-arac"
    cc -O2 -std=c99 -w -I"$LIMINE" -o "$ARAC" "$LIMINE/limine.c"
    truncate -s 192M "$YIMAJ"
    echo 'label: dos
start=2048, size=131072, type=ef, bootable
start=133120, type=83' | sfdisk -q "$YIMAJ"
    # 1: FAT32 /boot (ayrı dosyada kurulur, sonra bölüme yazılır)
    FATB="$IS/boot.fat"
    truncate -s 64M "$FATB"
    mkfs.fat -F 32 -s 1 -n UPPIXBOOT -h 2048 "$FATB" >/dev/null
    B=$(mktemp -d)
    mkdir -p "$B/limine" "$B/EFI/BOOT"
    cp "$LIMINE/limine-bios.sys" "$B/limine/"
    cp "$LIMINE/BOOTX64.EFI" "$B/EFI/BOOT/"
    cat > "$B/limine/limine.conf" <<'KONF'
# Uppix önyükleme menüsü (Limine). Bu dosya Uppix'te /boot/limine/limine.conf olarak görünür.
timeout: 3
interface_branding: Uppix - u++ ile yazilmis isletim sistemi

/Uppix
    comment: Uppix (64-bit, Limine protokolü)
    protocol: limine
    path: boot():/uppix.elf
KONF
    cp "$CEKIRDEK" "$B/uppix.elf"
    mcopy -i "$FATB" -s "$B/limine" "$B/EFI" "$B/uppix.elf" ::/
    rm -rf "$B"
    # Bölüm 1 sektör 2048'de (1 MiB), bölüm 2 sektör 133120'de (65 MiB) başlar.
    dd if="$FATB" of="$YIMAJ" bs=1M seek=1 conv=notrunc status=none
    # 2: ext4 kök
    ext4_kok "$IS/kok.ext4" 127M ""
    dd if="$IS/kok.ext4" of="$YIMAJ" bs=1M seek=65 conv=notrunc status=none
    # Limine BIOS aşamaları (MBR + bölüm boşluğu)
    "$ARAC" bios-install "$YIMAJ" >/dev/null 2>&1 || "$ARAC" bios-install "$YIMAJ"
    rm -f "$IMAJ"
    cp "$YIMAJ" "$IMAJ"
    rm -rf "$IS"
    echo "[disk] önyüklenebilir imaj: $IMAJ (Limine; 1: FAT32 /boot, 2: ext4 kök)"
fi
[ -f "$CIKTI/usb-fat32.img" ] || usb_fat32
[ -f "$CIKTI/usb-exfat.img" ] || usb_exfat
exit 0
