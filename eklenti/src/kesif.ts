import * as path from "path";

export type Platform = "win32" | "linux" | "darwin" | string;

export interface DosyaSistemi {
  dosyaVar(yol: string): boolean;
}

export interface KesifGirdi {
  platform: Platform;
  repo?: string;
  uppcYolu?: string;
  pathDirs?: string[];
}

function yolApi(platform: Platform): path.PlatformPath {
  return platform === "win32" ? path.win32 : path.posix;
}

export function uppcAdlari(platform: Platform): string[] {
  if (platform === "win32") {
    return ["uppc.exe"];
  }
  return ["uppc", "uppc.out"];
}

export function uppcAdaylari(girdi: KesifGirdi): string[] {
  const P = yolApi(girdi.platform);
  const adlar = uppcAdlari(girdi.platform);
  const aday: string[] = [];
  const ekle = (yol: string) => {
    if (yol && !aday.includes(yol)) {
      aday.push(yol);
    }
  };
  const ozel = (girdi.uppcYolu || "").trim();
  if (ozel) {
    ekle(ozel);
  }
  if (girdi.repo) {
    for (const ad of adlar) {
      ekle(P.join(girdi.repo, "derleyici", ad));
    }
  }
  for (const dizin of girdi.pathDirs || []) {
    if (!dizin) {
      continue;
    }
    for (const ad of adlar) {
      ekle(P.join(dizin, ad));
    }
  }
  return aday;
}

export function uppcBul(girdi: KesifGirdi, fs: DosyaSistemi): string | undefined {
  for (const yol of uppcAdaylari(girdi)) {
    if (fs.dosyaVar(yol)) {
      return yol;
    }
  }
  return undefined;
}

export function pathDirsAl(ham: string | undefined, platform: Platform): string[] {
  if (!ham) {
    return [];
  }
  const ayir = platform === "win32" ? ";" : ":";
  return ham.split(ayir).map((s) => s.trim()).filter((s) => s.length > 0);
}

export function kokMu(dir: string, fs: DosyaSistemi, P: path.PlatformPath = path): boolean {
  return (
    fs.dosyaVar(P.join(dir, "src", "uppc", "surucu.upp")) ||
    fs.dosyaVar(P.join(dir, "src", "uppc")) ||
    fs.dosyaVar(P.join(dir, "derleyici", "uppc.exe")) ||
    fs.dosyaVar(P.join(dir, "derleyici", "uppc")) ||
    fs.dosyaVar(P.join(dir, "derleyici", "uppc.out")) ||
    fs.dosyaVar(P.join(dir, "derleyici", "uppc_birlesik.upp"))
  );
}

export function kokBul(baslangic: string[], fs: DosyaSistemi, P: path.PlatformPath = path): string | undefined {
  for (const seed of baslangic) {
    let dir = seed;
    for (let i = 0; i < 12; i++) {
      if (kokMu(dir, fs, P)) {
        return dir;
      }
      const ust = P.dirname(dir);
      if (ust === dir) {
        break;
      }
      dir = ust;
    }
  }
  return undefined;
}
