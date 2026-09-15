import { execFile } from "child_process";
import * as fs from "fs";
import * as os from "os";
import * as path from "path";
import * as vscode from "vscode";
import type { Motor } from "./motor";

export class BicimSaglayici implements vscode.DocumentFormattingEditProvider {
  constructor(private motorAl: () => Motor) {}

  provideDocumentFormattingEdits(doc: vscode.TextDocument): Thenable<vscode.TextEdit[]> {
    const motor = this.motorAl();
    if (!motor.hazir) {
      return Promise.resolve([]);
    }
    const tmp = path.join(os.tmpdir(), `upp-bicim-${process.pid}-${Date.now()}.upp`);
    fs.writeFileSync(tmp, doc.getText(), "utf8");
    const cwd = doc.uri.scheme === "file" ? path.dirname(doc.uri.fsPath) : os.tmpdir();
    return new Promise((resolve) => {
      execFile(
        motor.yol,
        [tmp, "--bicim"],
        { cwd, timeout: 20000, windowsHide: true, encoding: "utf8" },
        (err) => {
          try {
            if (err) {
              resolve([]);
              return;
            }
            const yeni = fs.readFileSync(tmp, "utf8");
            const tam = new vscode.Range(doc.positionAt(0), doc.positionAt(doc.getText().length));
            resolve([vscode.TextEdit.replace(tam, yeni)]);
          } catch {
            resolve([]);
          } finally {
            try {
              fs.unlinkSync(tmp);
            } catch {
              /* yok */
            }
          }
        }
      );
    });
  }
}
