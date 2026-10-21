#!/bin/sh
# Örnek betik: konum parametreleri ve koşullar
echo "Merhaba, $1!"
echo "Bu betik $# argüman aldı: $@"
test -d /tmp && echo "/tmp var" || echo "/tmp yok"
echo "Kabuk süreci: $$"
