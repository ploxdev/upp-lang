import { execFile } from "child_process";
import * as fs from "fs";
import * as os from "os";
import * as path from "path";
import * as vscode from "vscode";
import { nesilUygula } from "./nesil";
import { taniHarita } from "./tani";
import type { DurumCubugu } from "./durum";
import type { Motor } from "./motor";

export class TaniYoneticisi {
  private koleksiyon: vscode.DiagnosticCollection;
  private timer: NodeJS.Timeout | undefined;
  private motor: Motor = { hazir: false, neden: "" };
  private nesil = 0;
  private iptal: AbortController | undefined;

  constructor(
    private durum: DurumCubugu,
    context: vscode.ExtensionContext
  ) {
    this.koleksiyon = vscode.languages.createDiagnosticCollection("u++");
    context.subscriptions.push(this.koleksiyon);
  }

  motorYaz(m: Motor): void {
    this.motor = m;
    if (!m.hazir) {
      this.nesil += 1;
      this.iptal?.abort();
      for (const d of vscode.workspace.textDocuments) {
        if (d.languageId === "upp") {
          this.koleksiyon.set(d.uri, []);
        }
      }
      this.durum.hataYaz(0);
    }
  }

  zamanla(doc: vscode.TextDocument): void {
    if (this.timer) {
      clearTimeout(this.timer);
    }
    this.timer = setTimeout(() => {
      void this.calistir(doc);
    }, 400);
  }

  async calistir(doc: vscode.TextDocument): Promise<void> {
    if (doc.languageId !== "upp" || doc.uri.scheme !== "file") {
      return;
    }
    if (!this.motor.hazir) {
      this.koleksiyon.set(doc.uri, []);
      return;
    }
    this.nesil += 1;
    const ben = this.nesil;
    this.iptal?.abort();
    this.iptal = new AbortController();
    const signal = this.iptal.signal;
    const exe = this.motor.yol;
    const dizin = path.dirname(doc.uri.fsPath);
    const tmpKok = fs.existsSync(dizin) ? dizin : os.tmpdir();
    const tmp = fs.mkdtempSync(path.join(tmpKok, ".upp-analiz-"));
    const yol = path.join(tmp, path.basename(doc.uri.fsPath) || "tampon.upp");
    try {
      fs.writeFileSync(yol, doc.getText(), "utf8");
      const stdout = await new Promise<string>((resolve, reject) => {
        execFile(
          exe,
          [yol, "--analiz"],
          {
            cwd: dizin,
            timeout: 20000,
            windowsHide: true,
            encoding: "utf8",
            signal,
          },
          (err, out, errOut) => {
            if (signal.aborted) {
              reject(new Error("aborted"));
              return;
            }
            const ham = String(out || "");
            if (!ham && err) {
              reject(new Error(String(errOut || err.message)));
              return;
            }
            resolve(ham);
          }
        );
      });
      if (!nesilUygula(ben, this.nesil)) {
        return;
      }
      const tani = taniHarita(stdout);
      const diags = tani.map((t) => {
        const line = Math.max(0, t.satir - 1);
        const col = Math.max(0, t.sutun - 1);
        const d = new vscode.Diagnostic(
          new vscode.Range(line, col, line, col + 1),
          t.mesaj,
          vscode.DiagnosticSeverity.Error
        );
        d.source = t.guvenlik ? "u++ güvenlik" : "u++";
        return d;
      });
      this.koleksiyon.set(doc.uri, diags);
      if (vscode.window.activeTextEditor?.document.uri.toString() === doc.uri.toString()) {
        this.durum.hataYaz(diags.length);
      }
    } catch (e) {
      if (signal.aborted || (e instanceof Error && e.message === "aborted")) {
        return;
      }
      if (!nesilUygula(ben, this.nesil)) {
        return;
      }
      const d = new vscode.Diagnostic(
        new vscode.Range(0, 0, 0, 1),
        e instanceof Error ? e.message : String(e),
        vscode.DiagnosticSeverity.Error
      );
      d.source = "u++";
      this.koleksiyon.set(doc.uri, [d]);
    } finally {
      try {
        fs.rmSync(tmp, { recursive: true, force: true });
      } catch {
        /* yok */
      }
    }
  }
}
