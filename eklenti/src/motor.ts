import { execFile } from "child_process";
import * as fs from "fs";
import * as vscode from "vscode";
import { uppAyar } from "./ayar";
import { pathDirsAl, uppcBul, type Platform } from "./kesif";
import { surumCoz, surumNedenMetin, type SurumSonuc } from "./surum";

export type Motor =
  | { hazir: true; yol: string; surum: string; protokol: number }
  | { hazir: false; neden: string; yol?: string };

const gercekFs = {
  dosyaVar(yol: string): boolean {
    try {
      return fs.existsSync(yol);
    } catch {
      return false;
    }
  },
};

export function uppcKesfet(repo: string | undefined): string | undefined {
  const ayar = uppAyar();
  return uppcBul(
    {
      platform: process.platform as Platform,
      repo,
      uppcYolu: ayar.uppcYolu,
      pathDirs: pathDirsAl(process.env.PATH, process.platform as Platform),
    },
    gercekFs
  );
}

export function surumOku(yol: string): Promise<SurumSonuc> {
  return new Promise((resolve) => {
    execFile(
      yol,
      ["--surum", "--json"],
      {
        timeout: 8000,
        windowsHide: true,
        encoding: "utf8",
        windowsVerbatimArguments: false,
      },
      (err, stdout) => {
        if (err && !stdout) {
          resolve({ ok: false, neden: "json_yok" });
          return;
        }
        resolve(surumCoz(String(stdout || "")));
      }
    );
  });
}

export async function motorKur(repo: string | undefined): Promise<Motor> {
  const yol = uppcKesfet(repo);
  if (!yol) {
    return { hazir: false, neden: "uppc bulunamadı (derleyici/uppc)." };
  }
  const s = await surumOku(yol);
  if (!s.ok) {
    return { hazir: false, neden: surumNedenMetin(s.neden), yol };
  }
  return { hazir: true, yol, surum: s.surum, protokol: s.protokol };
}

export function indirUrl(): vscode.Uri {
  return vscode.Uri.parse("https://github.com/ploxdev/upp-lang");
}

export async function yolBelirt(): Promise<string | undefined> {
  const filters =
    process.platform === "win32"
      ? { Derleyici: ["exe"] }
      : undefined;
  const sec = await vscode.window.showOpenDialog({
    canSelectFiles: true,
    canSelectFolders: false,
    canSelectMany: false,
    filters,
    title: "u++ derleyicisini seç (uppc)",
  });
  if (!sec || sec.length === 0) {
    return undefined;
  }
  const p = sec[0].fsPath;
  await vscode.workspace.getConfiguration("upp").update("uppcYolu", p, vscode.ConfigurationTarget.Workspace);
  return p;
}
