import { YERLESIK } from "./yerlesik_uretilen";

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
  arkaplan: "İş parçacığı: `oto is = arkaplan { ... }; is.bekle();` (barındırılan).",
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
  sayi: "İşaretli 64 bit (`s64`).",
  s64: "İşaretli 64 bit (`sayi`).",
  u64: "İşaretsiz 64 bit (`usayi`).",
  usayi: "İşaretsiz 64 bit (`u64`).",
  s32: "İşaretli 32 bit.",
  u32: "İşaretsiz 32 bit.",
  s16: "İşaretli 16 bit.",
  u16: "İşaretsiz 16 bit.",
  bayt: "İşaretli 8 bit (`s8`).",
  ubayt: "İşaretsiz 8 bit (`u8`).",
  ondalik: "64-bit kayan (`double`).",
  metin: "Yönetilen UTF-8 metin; kopya O(1), otomatik bırakılır.",
  mantik: "Boolean (`dogru` / `yanlis`).",
  bos: "Yok türü (`void`).",
  liste: "`liste[T]`. ekle, al, yaz, sil, temizle, uzunluk, bosalt.",
  harita: "`harita[K,V]`. koy, al, sil, temizle, anahtarlar.",
};

/** Tabloda olmayan özel yerleşikler. */
const OZEL_FONK: Record<string, string> = {
  yaz: "`yaz(x)` — yazdırılabilir her tür; `\"{ifade}\"` interpolasyonu. printf yok.",
  satir_yaz: "`satir_yaz(x)` — satır sonu ile yaz; argümansız boş satır.",
  hata_yaz: "`hata_yaz(x)` — stderr.",
  hata_satir_yaz: "`hata_satir_yaz(x)` — stderr + satır sonu.",
  uzunluk: "`uzunluk(x) -> sayi` — metin, dizi, liste, harita veya görünüm.",
  bekle: "`bekle(is)` — `ArkaplanIs` bitene kadar bekle.",
};

function bayrakNotu(b: number): string {
  const n: string[] = [];
  if (b & 1) n.push("guvensiz");
  if (b & 2) n.push("barındırılan");
  if (b & 4) n.push("yalnızca Windows");
  if (b & 8) n.push("yalnızca Linux");
  if (b & 64) n.push("yalnızca serbest/çekirdek");
  return n.length ? ` · ${n.join(", ")}` : "";
}

export const UPP_FONK: Record<string, string> = { ...OZEL_FONK };
/** Ad alanı → (ad → açıklama). Anahtarlar iç içe olabilir: "windows.bellek". */
export const ADALAN: Record<string, Record<string, string>> = {};
for (const y of YERLESIK) {
  const parca = y.ad.split(".");
  const ad = parca.pop() as string;
  const ns = parca.join(".");
  const param = y.param ? y.param.split(",").join(", ") : "";
  const metin = `\`${ad}(${param}) -> ${y.donus}\`${bayrakNotu(y.bayrak)}`;
  if (!ns) {
    UPP_FONK[ad] = metin;
  } else {
    (ADALAN[ns] ??= {})[ad] = metin;
  }
}
ADALAN["matematik"] = { ...(ADALAN["matematik"] ?? {}), dunya_ekran: "`dunya_ekran(...) -> EkranNokta`" };
ADALAN["windows.bellek"] = { ...(ADALAN["windows.bellek"] ?? {}), zincir_oku: "`zincir_oku(taban, [ofsetler]) -> sayi` · guvensiz" };

/** ns altındaki doğrudan alt ad alanları ("windows" → bellek, girdi, cizim). */
function altAdalanlar(ns: string): string[] {
  const on = ns ? `${ns}.` : "";
  const set = new Set<string>();
  for (const k of Object.keys(ADALAN)) {
    if (k.startsWith(on)) {
      const kalan = k.slice(on.length);
      if (kalan) set.add(kalan.split(".")[0]);
    }
  }
  return [...set].sort();
}

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
    const ns = yol.slice(1).join(".");
    const alt = altAdalanlar(ns).map((k) => item(k, "adalan", "upp ad alanı"));
    if (!ns) {
      return filtrele([...alt, ...kayitlar(UPP_FONK, "fonksiyon")], pref);
    }
    return filtrele([...alt, ...kayitlar(ADALAN[ns] ?? {}, "yontem")], pref);
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
    return altAdalanlar("").includes(kelime) ? "upp ad alanı" : UPP_FONK[kelime];
  }
  if (yol[0] === "upp" && yol.length >= 2) {
    const ns = yol.slice(1).join(".");
    if (altAdalanlar(ns).includes(kelime)) return "upp ad alanı";
    return ADALAN[ns]?.[kelime];
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
