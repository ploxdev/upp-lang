import * as fs from "fs";
import * as path from "path";
import * as vscode from "vscode";
import { kokBul } from "./kesif";

const gercekFs = {
  dosyaVar(yol: string): boolean {
    try {
      return fs.existsSync(yol);
    } catch {
      return false;
    }
  },
};

export function findRepoRoot(start?: string): string | undefined {
  const seeds: string[] = [];
  if (start) {
    seeds.push(start);
  }
  for (const folder of vscode.workspace.workspaceFolders ?? []) {
    seeds.push(folder.uri.fsPath);
  }
  return kokBul(seeds, gercekFs, path);
}

export function ikiliUzanti(): string {
  return process.platform === "win32" ? ".exe" : ".out";
}

export function hedefBayrak(): string[] {
  return process.platform === "win32" ? [] : ["--linux"];
}

export function ciktiKok(repo: string): string {
  const rel = vscode.workspace.getConfiguration("upp").get<string>("ciktiKoku") || "derleme";
  return path.isAbsolute(rel) ? rel : path.join(repo, rel);
}

export function quote(s: string): string {
  if (/[\s"]/.test(s)) {
    return `"${s.replace(/"/g, '\\"')}"`;
  }
  return s;
}

export function calistirSatiri(exePath: string): string {
  return quote(exePath);
}
