#!/bin/sh
# Uppix disk imajları:
#   uppix-kok.img  64 MiB ext4, etiket "uppix-kok": kök dosya sistemi (kok/ ağacından)
#   usb-fat32.img  64 MiB, MBR bölüm tablosu + FAT32 bölümü "UPPIXUSB" (örnek dosyalarla)
#   usb-exfat.img  32 MiB exFAT "UPPIXEX" (bölümsüz, boş)
# Kullanım: sh disk_yap.sh [çıktı_dizini]   (varsayılan: derleme)
# Gerekenler: mkfs.ext4 (e2fsprogs); FAT/exFAT için mkfs.fat + mtools ve mkfs.exfat
# (dosfstools, mtools, exfatprogs). FS_ARACLAR, paketlerin açıldığı dizini gösterebilir.
set -e
BURA=$(cd "$(dirname "$0")" && pwd)
CIKTI=${1:-derleme}
case "$CIKTI" in /*) ;; *) CIKTI="$(pwd)/$CIKTI" ;; esac
if [ -n "$FS_ARACLAR" ]; then
    PATH="$FS_ARACLAR/usr/sbin:$FS_ARACLAR/usr/bin:$FS_ARACLAR/sbin:$PATH"
fi
PATH="$PATH:/sbin:/usr/sbin"
LC_ALL=C.UTF-8
export PATH MTOOLS_SKIP_CHECK=1 LC_ALL
mkdir -p "$CIKTI"

# ---- ext4 kök ----
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
if [ -f "$BURA/derleme/uppix32.elf" ]; then
    cp "$BURA/derleme/uppix32.elf" "$HAZIR/boot/uppix32.elf"
fi
rm -f "$CIKTI/uppix-kok.img"
if [ "$(id -u)" = "0" ]; then
    mkfs.ext4 -q -F -L uppix-kok -b 4096 -I 256 -d "$HAZIR" "$CIKTI/uppix-kok.img" 64M
else
    # root değilken -d dosya sahipliğini kullanıcıdan alır; sahipleri sonra 0:0 yap.
    mkfs.ext4 -q -F -L uppix-kok -b 4096 -I 256 -E root_owner=0:0 -d "$HAZIR" "$CIKTI/uppix-kok.img" 64M
    ( cd "$HAZIR" && find . -mindepth 1 | sed 's|^\.||' ) | while read -r yol; do
        echo "set_inode_field \"$yol\" uid 0"
        echo "set_inode_field \"$yol\" gid 0"
    done > "$CIKTI/sahip.debugfs"
    debugfs -w -f "$CIKTI/sahip.debugfs" "$CIKTI/uppix-kok.img" >/dev/null 2>&1 || true
    rm -f "$CIKTI/sahip.debugfs"
fi
rm -rf "$HAZIR"
echo "[disk] ext4 kök: $CIKTI/uppix-kok.img"

# ---- FAT32 (MBR, 1 MiB'den başlayan bölüm) ----
rm -f "$CIKTI/usb-fat32.img"
truncate -s 64M "$CIKTI/usb-fat32.img"
if command -v mkfs.fat >/dev/null 2>&1 && command -v mcopy >/dev/null 2>&1 && command -v sfdisk >/dev/null 2>&1; then
    echo 'label: dos
start=2048, type=c' | sfdisk -q "$CIKTI/usb-fat32.img"
    mkfs.fat -F 32 -n UPPIXUSB --offset=2048 "$CIKTI/usb-fat32.img" $(( (65536 * 1024 / 512 - 2048) / 2 )) >/dev/null
    G="$CIKTI/usb_icerik"
    rm -rf "$G"
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

# ---- exFAT (bölümsüz) ----
rm -f "$CIKTI/usb-exfat.img"
truncate -s 32M "$CIKTI/usb-exfat.img"
if command -v mkfs.exfat >/dev/null 2>&1; then
    mkfs.exfat -L UPPIXEX "$CIKTI/usb-exfat.img" >/dev/null
    echo "[disk] exFAT: $CIKTI/usb-exfat.img"
else
    echo "[disk] mkfs.exfat yok: usb-exfat.img biçimlendirilmedi"
fi
