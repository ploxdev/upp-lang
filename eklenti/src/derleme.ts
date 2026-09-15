import * as fs from "fs";
import * as path from "path";
import * as vscode from "vscode";
import { ciktiKok, calistirSatiri, findRepoRoot, hedefBayrak, ikiliUzanti, quote } from "./kok";
import type { Motor } from "./motor";

export interface Derleme {
  cwd: string;
  compileLine: string;
  exePath: string;
}

export function derlemeHazir(
  doc: vscode.TextDocument,
  motor: Motor,
  opts?: { sessiz?: boolean }
): Derleme | undefined {
  if (!motor.hazir) {
    return undefined;
  }
  const repo = findRepoRoot(path.dirname(doc.uri.fsPath));
  if (!repo) {
    if (!opts?.sessiz) {
      void vscode.window.showWarningMessage("u++ kökü bulunamadı. Repoyu açın.");
    }
    return undefined;
  }
  const file = doc.uri.fsPath;
  if (path.extname(file).toLowerCase() === ".uph") {
    if (!opts?.sessiz) {
      void vscode.window.showWarningMessage(
        "Başlık dosyası (.uph) derlenmez; bir .upp programından 'kullan' ile katın."
      );
    }
    return undefined;
  }
  const kok = ciktiKok(repo);
  fs.mkdirSync(kok, { recursive: true });
  const stem = path.join(kok, path.basename(file, path.extname(file)));
  const exePath = `${stem}${ikiliUzanti()}`;
  const hedef = hedefBayrak();
  const compileLine = [quote(motor.yol), quote(file), "--sadece-derle", "--cikti", quote(stem), ...hedef].join(
    " "
  );
  return { cwd: repo, compileLine, exePath };
}

function gorevNesne(
  taskAd: string,
  baslik: string,
  satir: string,
  cwd: string,
  derleMi: boolean
): vscode.Task {
  const def: vscode.TaskDefinition = { type: "upp", task: taskAd };
  const task = new vscode.Task(
    def,
    vscode.TaskScope.Workspace,
    baslik,
    "u++",
    new vscode.ShellExecution(satir, { cwd }),
    ["$upp-hata", "$upp-guvenlik"]
  );
  if (derleMi) {
    task.group = vscode.TaskGroup.Build;
  }
  task.presentationOptions = {
    reveal: vscode.TaskRevealKind.Always,
    panel: vscode.TaskPanelKind.Dedicated,
  };
  return task;
}

class UppGorevSaglayici implements vscode.TaskProvider {
  constructor(private motorAl: () => Motor) {}

  provideTasks(): vscode.Task[] {
    const ed = vscode.window.activeTextEditor;
    const motor = this.motorAl();
    if (!ed || ed.document.languageId !== "upp" || !motor.hazir) {
      return [];
    }
    const d = derlemeHazir(ed.document, motor, { sessiz: true });
    if (!d) {
      return [];
    }
    return [
      gorevNesne("derle", "u++: Derle", d.compileLine, d.cwd, true),
      gorevNesne(
        "calistir",
        "u++: Çalıştır",
        calistirSatiri(d.compileLine, d.exePath),
        d.cwd,
        false
      ),
      gorevNesne(
        "sadece-c",
        "u++: C üret",
        d.compileLine.replace("--sadece-derle", "--sadece-c"),
        d.cwd,
        false
      ),
    ];
  }

  resolveTask(task: vscode.Task): vscode.Task | undefined {
    return task;
  }
}

async function gorevCalistir(ad: string, satir: string, cwd: string, derleMi: boolean): Promise<void> {
  await vscode.tasks.executeTask(gorevNesne(ad, ad, satir, cwd, derleMi));
}

async function aktifUpp(): Promise<vscode.TextDocument | undefined> {
  const ed = vscode.window.activeTextEditor;
  if (!ed || ed.document.languageId !== "upp") {
    void vscode.window.showInformationMessage("Açık bir .upp dosyası seçin.");
    return undefined;
  }
  if (ed.document.isDirty) {
    await ed.document.save();
  }
  return ed.document;
}

export function registerDerleme(context: vscode.ExtensionContext, motorAl: () => Motor): void {
  const calis = async (hangisi: "derle" | "calistir" | "c") => {
    const doc = await aktifUpp();
    if (!doc) {
      return;
    }
    const d = derlemeHazir(doc, motorAl());
    if (!d) {
      return;
    }
    if (hangisi === "derle") {
      await gorevCalistir("u++: Derle", d.compileLine, d.cwd, true);
    } else if (hangisi === "calistir") {
      await gorevCalistir("u++: Çalıştır", calistirSatiri(d.compileLine, d.exePath), d.cwd, false);
    } else {
      await gorevCalistir(
        "u++: C üret",
        d.compileLine.replace("--sadece-derle", "--sadece-c"),
        d.cwd,
        false
      );
    }
  };

  context.subscriptions.push(
    vscode.tasks.registerTaskProvider("upp", new UppGorevSaglayici(motorAl)),
    vscode.commands.registerCommand("upp.derle", () => calis("derle")),
    vscode.commands.registerCommand("upp.calistir", () => calis("calistir")),
    vscode.commands.registerCommand("upp.sadeceC", () => calis("c"))
  );
}
