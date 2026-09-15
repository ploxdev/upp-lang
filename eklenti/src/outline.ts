export type SembolTur = "fonk" | "sinif" | "secenek";

export type Sembol = {
  ad: string;
  tur: SembolTur;
  satir: number;
  sutun: number;
};

const SATIR = /^(?:fonk|sinif|secenek)\b/;

export function outlineSemboller(kaynak: string): Sembol[] {
  const out: Sembol[] = [];
  const satirlar = kaynak.split(/\r?\n/);
  for (let i = 0; i < satirlar.length; i++) {
    const ham = satirlar[i];
    const s = ham.trimStart();
    if (!SATIR.test(s)) {
      continue;
    }
    const m = s.match(/^(fonk|sinif|secenek)\s+([A-Za-z_][A-Za-z0-9_]*)/);
    if (!m) {
      continue;
    }
    const sutun = ham.length - ham.trimStart().length + 1;
    out.push({
      ad: m[2],
      tur: m[1] as SembolTur,
      satir: i + 1,
      sutun,
    });
  }
  return out;
}
