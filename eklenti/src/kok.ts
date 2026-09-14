import { execFileSync } from "child_process";
import * as fs from "fs";
import * as path from "path";
import * as vscode from "vscode";

export type Derleyici = "python" | "native";

export interface UppAyar {
  pythonYolu: string;
  derleyici: Derleyici;
  uppcYolu: string;
  ciktiKoku: string;
  gccTanilari: boolean;
  izSunucu: "off" | "messages" | "verbose";
}

export function uppAyar(): UppAyar {
  const c = vscode.workspace.getConfiguration("upp");
  const derleyici = c.get<string>("derleyici") === "native" ? "native" : "python";
  const iz = c.get<string>("iz.sunucu");
  const izSunucu: UppAyar["izSunucu"] =
    iz === "verbose" || iz === "messages" ? iz : "off";
  return {
    pythonYolu: c.get<string>("pythonYolu") || "python",
    derleyici,
    uppcYolu: (c.get<string>("uppcYolu") || "").trim(),
    ciktiKoku: c.get<string>("ciktiKoku") || "derleme",
    gccTanilari: Boolean(c.get<boolean>("gccTanilari")),
    izSunucu,
  };
}

export function findRepoRoot(start?: string): string | undefined {
  const seeds: string[] = [];
  if (start) {
    seeds.push(start);
  }
  for (const folder of vscode.workspace.workspaceFolders ?? []) {
    seeds.push(folder.uri.fsPath);
  }
  for (const seed of seeds) {
    let dir = seed;
    for (let i = 0; i < 10; i++) {
      if (fs.existsSync(path.join(dir, "uppc.py"))) {
        return dir;
      }
      const parent = path.dirname(dir);
      if (parent === dir) {
        break;
      }
      dir = parent;
    }
  }
  return undefined;
}

export function nativeExe(repo: string): string {
  const custom = uppAyar().uppcYolu;
  if (custom) {
    return custom;
  }
  const win = path.join(repo, "derleme", "uppc.exe");
  const posix = path.join(repo, "derleme", "uppc");
  const outp = path.join(repo, "derleme", "uppc.out");
  if (process.platform === "win32") {
    return win;
  }
  if (fs.existsSync(posix)) {
    return posix;
  }
  if (fs.existsSync(outp)) {
    return outp;
  }
  return posix;
}

export function ikiliUzanti(): string {
  return process.platform === "win32" ? ".exe" : ".out";
}

export function hedefBayrak(): string[] {
  return process.platform === "win32" ? [] : ["--linux"];
}

export function calistirSatiri(compileLine: string, exePath: string): string {
  if (process.platform === "win32") {
    return `${compileLine}; if ($LASTEXITCODE -eq 0) { & ${quote(exePath)} }`;
  }
  return `${compileLine} && ${quote(exePath)}`;
}

export function ciktiKok(repo: string): string {
  const rel = uppAyar().ciktiKoku;
  return path.isAbsolute(rel) ? rel : path.join(repo, rel);
}

export function quote(s: string): string {
  if (/[\s"]/.test(s)) {
    return `"${s.replace(/"/g, '\\"')}"`;
  }
  return s;
}

function magazaPythonMi(yol: string): boolean {
  const n = yol.replace(/\//g, "\\").toLowerCase();
  return n.includes("\\windowsapps\\");
}

function pythonExeYazdir(cmd: string, args: string[]): string | undefined {
  try {
    const ham = execFileSync(cmd, args, {
      encoding: "utf8",
      timeout: 8000,
      windowsHide: true,
      stdio: ["ignore", "pipe", "ignore"],
    }).trim();
    const satir = ham.split(/\r?\n/).map((s) => s.trim()).find((s) => s.length > 0);
    if (satir && fs.existsSync(satir) && !magazaPythonMi(satir)) {
      return satir;
    }
  } catch {
    return undefined;
  }
  return undefined;
}

function windowsPythonAdaylari(): string[] {
  const aday: string[] = [];
  const ekle = (p: string) => {
    if (p && fs.existsSync(p) && !magazaPythonMi(p)) {
      aday.push(p);
    }
  };
  try {
    for (const ent of fs.readdirSync("C:\\")) {
      if (/^Python\d+/i.test(ent)) {
        ekle(path.join("C:\\", ent, "python.exe"));
      }
    }
  } catch {
    /* yok */
  }
  const local = process.env.LOCALAPPDATA;
  if (local) {
    const pyRoot = path.join(local, "Programs", "Python");
    try {
      for (const ent of fs.readdirSync(pyRoot)) {
        ekle(path.join(pyRoot, ent, "python.exe"));
      }
    } catch {
      /* yok */
    }
  }
  aday.sort().reverse();
  return aday;
}

/** Gerçek python.exe. Windows Store `python` kısayolu (çıkış 9009) atlanır. */
export function cozPythonYolu(istenen?: string): string | undefined {
  const ham = (istenen ?? uppAyar().pythonYolu).trim() || "python";
  const genel = ham === "python" || ham === "python3" || ham === "py";

  if (path.isAbsolute(ham) && fs.existsSync(ham) && !magazaPythonMi(ham)) {
    return ham;
  }
  if (!genel) {
    if (fs.existsSync(ham) && !magazaPythonMi(ham)) {
      return ham;
    }
    const acik = pythonExeYazdir(ham, ["-c", "import sys; print(sys.executable)"]);
    if (acik) {
      return acik;
    }
  }

  if (process.platform === "win32") {
    const py3 = pythonExeYazdir("py", ["-3", "-c", "import sys; print(sys.executable)"]);
    if (py3) {
      return py3;
    }
    const kurulum = windowsPythonAdaylari();
    if (kurulum.length > 0) {
      return kurulum[0];
    }
    try {
      const where = execFileSync("where.exe", ["python"], {
        encoding: "utf8",
        timeout: 5000,
        windowsHide: true,
        stdio: ["ignore", "pipe", "ignore"],
      });
      for (const line of where.split(/\r?\n/)) {
        const p = line.trim();
        if (p && fs.existsSync(p) && !magazaPythonMi(p)) {
          return p;
        }
      }
    } catch {
      /* yok */
    }
  }

  for (const cmd of process.platform === "win32" ? ["python3"] : ["python3", "python"]) {
    const acik = pythonExeYazdir(cmd, ["-c", "import sys; print(sys.executable)"]);
    if (acik) {
      return acik;
    }
  }
  return undefined;
}
