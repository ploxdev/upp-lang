/** Eklentinin konuştuğu IPC protokol aralığı. --analiz JSON şekli = 1. */
export const PROTOKOL_MIN = 1;
export const PROTOKOL_MAX = 1;

export type SurumNeden =
  | "json_yok"
  | "dil"
  | "eksik"
  | "eski_derleyici"
  | "eski_eklenti";

export type SurumSonuc =
  | { ok: true; dil: "u++"; surum: string; protokol: number }
  | { ok: false; neden: SurumNeden };

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

export function surumCoz(stdout: string): SurumSonuc {
  const satir = sonJsonSatir(stdout || "");
  if (!satir) {
    return { ok: false, neden: "json_yok" };
  }
  let data: unknown;
  try {
    data = JSON.parse(satir);
  } catch {
    return { ok: false, neden: "json_yok" };
  }
  if (!data || typeof data !== "object") {
    return { ok: false, neden: "json_yok" };
  }
  const o = data as Record<string, unknown>;
  if (o.dil !== "u++") {
    return { ok: false, neden: "dil" };
  }
  if (typeof o.surum !== "string" || !o.surum.trim()) {
    return { ok: false, neden: "eksik" };
  }
  if (typeof o.protokol !== "number" || !Number.isFinite(o.protokol)) {
    return { ok: false, neden: "eksik" };
  }
  const protokol = o.protokol;
  if (protokol < PROTOKOL_MIN) {
    return { ok: false, neden: "eski_derleyici" };
  }
  if (protokol > PROTOKOL_MAX) {
    return { ok: false, neden: "eski_eklenti" };
  }
  return { ok: true, dil: "u++", surum: o.surum.trim(), protokol };
}

export function surumNedenMetin(neden: SurumNeden): string {
  switch (neden) {
    case "json_yok":
      return "Derleyici --surum --json konuşmuyor. u++ 3.0.1+ gerekli.";
    case "dil":
      return "Bu ikili u++ derleyicisi değil.";
    case "eksik":
      return "Sürüm JSON eksik alan içeriyor (dil, surum, protokol).";
    case "eski_derleyici":
      return "Derleyici protokolü eski. Derleyiciyi güncelleyin.";
    case "eski_eklenti":
      return "Eklenti protokolü eski. Eklentiyi güncelleyin.";
  }
}
