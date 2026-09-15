export type TamamTur = "anahtar" | "tip" | "fonksiyon" | "adalan" | "yontem";

export type TamamItem = {
  etiket: string;
  tur: TamamTur;
  ayrinti: string;
};

export const ANAHTARLAR: Record<string, string> = {
  fonk: "Fonksiyon tanımı. `fonk ad(param) -> tur { ... }`",
  don: "Fonksiyondan dön. `-> bos` ise değer yok.",
  eger: "Koşullu dal. `eger (mantik) { ... } yoksa { ... }`",
  yoksa: "`eger` zincirinin else dalı.",
  iken: "Koşullu döngü. `iken (koşul) { ... }`",
  dongu: "C `for`. `dongu (init; koşul; adım) { ... }`",
  her: "Dizi veya harita üzerinde gezin. `her (oto x in xs)`.",
  in: "`her` döngüsünde kaynak.",
  dur: "Döngüden / `secim` dalından çık.",
  devam: "Döngünün sonraki turuna geç.",
  secim: "C `switch`. `sayi` veya `metin`.",
  durum: "`secim` sabiti. Çalışma anı değişken olamaz.",
  varsayilan: "`secim` varsayılan dalı.",
  arkaplan: "İş parçacığı. `upp.bekle`.",
  guvensiz: "İşaretçi, `c_kod`, süreç belleği kapısı.",
  guvenli: "İç içe güvenli moda dönüş.",
  sinif: "Yapı + metotlar. Alanlarda ham işaretçi yok.",
  secenek: "Adlandırılmış tam sayı kümesi.",
  kendi: "Metot alıcısı.",
  oto: "Tür çıkarımı. `oto x = ...;`",
  c_ekle: "Üst düzey C `#include`.",
  c_kod: "Ham C gövdesi — yalnızca `guvensiz`.",
  kullan: "`kullan \"kutuphane.uph\";` veya `olarak` önek.",
  olarak: "`kullan \"yol.uph\" olarak ad`.",
  sabit: "Derleme anı sabiti. Yeniden atanamaz.",
  ana: "Giriş noktası. Parametre almaz.",
};

export const TIPLER: Record<string, string> = {
  sayi: "64-bit tam sayı (`long long`).",
  bayt: "8-bit işaretsiz (`uint8_t`).",
  ondalik: "64-bit kayan (`double`).",
  metin: "C dizgesi (`char*`).",
  mantik: "Boolean (`dogru` / `yanlis`).",
  bos: "Yok türü (`void`).",
  liste: "`liste[T]`. ekle, al, yaz, sil, temizle, uzunluk, bosalt.",
  harita: "`harita[K,V]`. koy, al, sil, temizle, anahtarlar.",
};

export const UPP_FONK: Record<string, string> = {
  yaz: "Biçimli yaz. `{ad}` interpolasyonu.",
  satir_yaz: "Satır yaz. Argümansız boş satır.",
  mesaj: "`mesaj(baslik, govde)` — bilgi kutusu.",
  hata: "`hata(baslik, govde)` — hata kutusu.",
  hata_yaz: "stderr yaz.",
  hata_satir_yaz: "stderr + satır sonu.",
  ses_cal: "WAV çal. Dönüş `sayi`.",
  uyut: "Milisaniye uyu.",
  rastgele: "`rastgele(min, max)` dahil aralık.",
  dosya_oku: "Metin oku.",
  dosya_yaz: "Metin yaz. `1`/`0`.",
  dosya_ekle: "Dosyanın sonuna yaz.",
  dosya_var_mi: "Dosya var mı.",
  dosya_sil: "Dosya sil.",
  dosya_boyut: "Dosya boyutu (`sayi`).",
  uzunluk: "Metin veya koleksiyon uzunluğu.",
  giris: "Konsol satırı oku.",
  min: "İki `sayi` minimumu.",
  max: "İki `sayi` maksimumu.",
  mutlak: "`sayi` mutlak değer.",
  metin_bosalt: "Yığın metni serbest bırak.",
  bekle: "`ArkaplanIs` bitene kadar bekle.",
  arguman_sayisi: "`argc`.",
  arguman: "`argv[i]`.",
  zaman: "Unix zamanı.",
  metinden_sayiya: "Metni `sayi` yap.",
  sayidan_metin: "`sayi` → metin.",
  ondalikdan_metin: "`ondalik` → metin.",
  metinden_ondalik: "Metni `ondalik` yap.",
  ortam_al: "Ortam değişkeni; yoksa `yok`.",
  ortam_yaz: "Ortam değişkeni yaz (`mantik`).",
  platform: "`windows` veya `linux`.",
};

export const ADALAN: Record<string, Record<string, string>> = {
  metin: {
    kod: "Kod noktası (`sayi`). İndeks kod noktası.",
    ileri: "Kod noktasının bayt uzunluğu (1–4).",
    kes: "`kes(s, bas, uzunluk)` kod noktası dilimi.",
    bol: "Ayırıcıya göre böl → `liste[metin]`.",
    birlestir: "İki metni birleştir.",
    icinde: "Alt dizge indeksi.",
    ara: "Alt dizge ara.",
    degistir: "Değiştir.",
    kirp: "Kenar boşluklarını kırp.",
    hex_coz: "Hex dizgeyi bayta çevir.",
    baslar: "Önek mi.",
    biter: "Sonek mi.",
  },
  sistem: {
    calistir: "`SurecCikti { cikti, kod, basarili }`.",
    komut_bul: "PATH'te komut ara.",
    exe_dizin: "Çalışan ikilinin dizini.",
    pid: "Süreç kimliği.",
  },
  yol: {
    dizin: "Üst dizin.",
    birlestir: "Yol birleştir.",
    gercek: "Gerçek yol.",
    calisma: "Çalışma dizini.",
    mutlak_mi: "Mutlak yol mu.",
    klasor_olustur: "Klasör oluştur.",
  },
  dosya: {
    listele: "`liste[metin]` dizin içeriği.",
  },
  json: {
    ayristir: "JSON metnini ayrıştır.",
    olustur: "JSON değeri oluştur.",
  },
  matematik: {
    pi: "π.",
    tan: "tanjant (radyan).",
    atan2: "atan2.",
  },
  linux: {
    pid: "Süreç kimliği.",
    sinyal_gonder: "`sinyal_gonder(pid, sig)`.",
    proc_oku: "`/proc` oku.",
    sayi_oku: "Süreç belleği oku — `guvensiz`.",
    sayi_yaz: "Süreç belleği yaz — `guvensiz`.",
  },
  windows: {
    mesaj: "MessageBox.",
    hata: "Hata kutusu.",
    ses_cal: "WAV.",
  },
  bellek: {
    sayioku: "Yabancı süreç `sayi` oku — `guvensiz`.",
    sayiyaz: "Yabancı süreç `sayi` yaz — `guvensiz`.",
  },
};

export const LISTE_YONTEM: Record<string, string> = {
  ekle: "Sona ekle.",
  al: "İndeksten oku.",
  yaz: "İndekse yaz.",
  sil: "İndeksi sil.",
  temizle: "Elemanları sil, kimlik kalır.",
  uzunluk: "Eleman sayısı.",
  bosalt: "Koleksiyonu bırak.",
};

export const HARITA_YONTEM: Record<string, string> = {
  koy: "Anahtar ← değer.",
  al: "Değer oku.",
  var_mi: "Anahtar var mı.",
  sil: "Anahtarı sil (`mantik`).",
  temizle: "Girdileri sil.",
  anahtarlar: "Anahtar listesi.",
  uzunluk: "Girdi sayısı.",
  bosalt: "Koleksiyonu bırak.",
};

function item(etiket: string, tur: TamamTur, ayrinti: string): TamamItem {
  return { etiket, tur, ayrinti };
}

function filtrele(items: TamamItem[], pref: string): TamamItem[] {
  if (!pref) {
    return items;
  }
  return items.filter((it) => it.etiket.startsWith(pref));
}

function kayitlar(rec: Record<string, string>, tur: TamamTur): TamamItem[] {
  return Object.keys(rec)
    .sort()
    .map((k) => item(k, tur, rec[k]));
}

/** Satır/önceki metinden noktalı yol. `upp.metin.` → { yol: ["upp","metin"], pref: "" } */
export function noktaYol(oncesi: string): { yol: string[]; pref: string; nokta: boolean } {
  const m = oncesi.match(
    /(?:^|[^A-Za-z0-9_])([A-Za-z_][A-Za-z0-9_]*(?:\.[A-Za-z_][A-Za-z0-9_]*)*)(\.)?$/
  );
  if (!m) {
    const tek = oncesi.match(/([A-Za-z_][A-Za-z0-9_]*)$/);
    return { yol: [], pref: tek ? tek[1] : "", nokta: false };
  }
  const parca = m[1].split(".");
  const nokta = Boolean(m[2]);
  if (nokta) {
    return { yol: parca, pref: "", nokta: true };
  }
  if (parca.length === 1) {
    return { yol: [], pref: parca[0], nokta: false };
  }
  const pref = parca[parca.length - 1];
  return { yol: parca.slice(0, -1), pref, nokta: true };
}

export function tamamla(oncesi: string): TamamItem[] {
  const { yol, pref, nokta } = noktaYol(oncesi);
  if (nokta && yol[0] === "upp") {
    if (yol.length === 1) {
      const ns = Object.keys(ADALAN)
        .sort()
        .map((k) => item(k, "adalan", "upp ad alanı"));
      return filtrele([...ns, ...kayitlar(UPP_FONK, "fonksiyon")], pref);
    }
    if (yol.length === 2 && ADALAN[yol[1]]) {
      return filtrele(kayitlar(ADALAN[yol[1]], "yontem"), pref);
    }
    if (yol.length === 3 && yol[1] === "windows" && yol[2] === "bellek") {
      return filtrele(kayitlar(ADALAN.bellek, "yontem"), pref);
    }
    return [];
  }
  if (nokta) {
    const birlesik = { ...LISTE_YONTEM, ...HARITA_YONTEM };
    return filtrele(kayitlar(birlesik, "yontem"), pref);
  }
  return filtrele(
    [...kayitlar(ANAHTARLAR, "anahtar"), ...kayitlar(TIPLER, "tip"), ...kayitlar(UPP_FONK, "fonksiyon")],
    pref
  );
}

export function hoverMetin(kelime: string, yol: string[] = []): string | undefined {
  if (yol[0] === "upp" && yol.length === 1) {
    return ADALAN[kelime] ? "upp ad alanı" : UPP_FONK[kelime];
  }
  if (yol[0] === "upp" && yol.length === 2) {
    const ns = ADALAN[yol[1]];
    return ns ? ns[kelime] : undefined;
  }
  if (LISTE_YONTEM[kelime]) {
    return LISTE_YONTEM[kelime];
  }
  if (HARITA_YONTEM[kelime]) {
    return HARITA_YONTEM[kelime];
  }
  return ANAHTARLAR[kelime] || TIPLER[kelime] || UPP_FONK[kelime] || (ADALAN[kelime] ? "upp ad alanı" : undefined);
}

export function kelimeAl(satir: string, karakter: number): { kelime: string; yol: string[] } {
  const i = Math.max(0, karakter);
  const sol = satir.slice(0, i);
  const sag = satir.slice(i);
  const solM = sol.match(/[A-Za-z_][A-Za-z0-9_]*$/);
  const sagM = sag.match(/^[A-Za-z0-9_]*/);
  const kelime = `${solM ? solM[0] : ""}${sagM ? sagM[0] : ""}`;
  const parsed = noktaYol(sol);
  return { kelime, yol: parsed.nokta ? parsed.yol : [] };
}
