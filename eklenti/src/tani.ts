export type AnalizHata = {
  satir: number;
  sutun: number;
  tur: string;
  mesaj: string;
};

export type Tani = {
  satir: number;
  sutun: number;
  mesaj: string;
  guvenlik: boolean;
};

function sonJsonSatir(ham: string): string | undefined {
  const satirlar = ham
    .split(/\r?\n/)
    .map((s) => s.trim())
    .filter((s) => s.length > 0);
  if (satirlar.length === 0) {
    return undefined;
  }
  return satirlar[satirlar.length - 1];
}

export function taniHarita(ham: string): Tani[] {
  const satir = sonJsonSatir(ham || "");
  if (!satir) {
    return [];
  }
  let data: unknown;
  try {
    data = JSON.parse(satir);
  } catch {
    return [
      {
        satir: 1,
        sutun: 1,
        mesaj: `Native JSON okunamadı: ${satir.slice(0, 200)}`,
        guvenlik: false,
      },
    ];
  }
  if (!data || typeof data !== "object") {
    return [];
  }
  const hatalar = (data as { hatalar?: unknown }).hatalar;
  if (!Array.isArray(hatalar)) {
    return [];
  }
  const out: Tani[] = [];
  for (const h of hatalar) {
    if (!h || typeof h !== "object") {
      continue;
    }
    const o = h as Record<string, unknown>;
    const s = typeof o.satir === "number" ? o.satir : 1;
    const c = typeof o.sutun === "number" ? o.sutun : 1;
    const mesaj = typeof o.mesaj === "string" ? o.mesaj : "";
    const tur = typeof o.tur === "string" ? o.tur : "hata";
    out.push({
      satir: s < 1 ? 1 : s,
      sutun: c < 1 ? 1 : c,
      mesaj,
      guvenlik: tur === "guvenlik",
    });
  }
  return out;
}
