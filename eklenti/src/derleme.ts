import * as fs from "fs";
import * as path from "path";
import * as vscode from "vscode";
import { ciktiStemRel } from "./ciktiYol";
import { kayitSonrasiDevam } from "./kayit";
import { ciktiKok, findRepoRoot, hedefBayrak, ikiliUzanti, quote } from "./kok";
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
  const stem = ciktiStemRel(repo, file, kok);
  fs.mkdirSync(path.dirname(stem), { recursive: true });
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
  derleMi: boolean,
  kimlik?: string
): vscode.Task {
  const def: vscode.TaskDefinition = { type: "upp", task: taskAd, kimlik: kimlik ?? taskAd };
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
    const derle = gorevNesne("derle", "u++: Derle", d.compileLine, d.cwd, true);
    const calistir = gorevNesne("calistir", "u++: Çalıştır", quote(d.exePath), d.cwd, false);
    const calistirBag = calistir as vscode.Task & { dependsOn?: vscode.Task; dependsOrder?: number };
    calistirBag.dependsOn = derle;
    calistirBag.dependsOrder = 1;
    return [
      derle,
      calistir,
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

async function gorevCalistirVeBekle(task: vscode.Task): Promise<number> {
  const kimlik = String(task.definition.kimlik ?? task.name);
  return new Promise((resolve, reject) => {
    const sub = vscode.tasks.onDidEndTaskProcess((e) => {
      if (String(e.execution.task.definition.kimlik ?? "") !== kimlik) {
        return;
      }
      sub.dispose();
      resolve(e.exitCode ?? 1);
    });
    void vscode.tasks.executeTask(task).then(undefined, (err) => {
      sub.dispose();
      reject(err);
    });
  });
}

async function aktifUpp(): Promise<vscode.TextDocument | undefined> {
  const ed = vscode.window.activeTextEditor;
  if (!ed || ed.document.languageId !== "upp") {
    void vscode.window.showInformationMessage("Açık bir .upp dosyası seçin.");
    return undefined;
  }
  if (ed.document.isDirty) {
    const kaydedildi = await ed.document.save();
    if (!kayitSonrasiDevam(kaydedildi)) {
      void vscode.window.showWarningMessage("Dosya kaydedilmedi; derleme iptal.");
      return undefined;
    }
  }
  return ed.document;
}

export function registerDerleme(context: vscode.ExtensionContext, motorAl: () => Motor): void {
  const calis = async (hangisi: "derle" | "calistir" | "c") => {
    try {
      const doc = await aktifUpp();
      if (!doc) {
        return;
      }
      const d = derlemeHazir(doc, motorAl());
      if (!d) {
        return;
      }
      if (hangisi === "derle") {
        const kimlik = `derle-${Date.now()}`;
        await gorevCalistirVeBekle(gorevNesne("u++: Derle", "u++: Derle", d.compileLine, d.cwd, true, kimlik));
      } else if (hangisi === "calistir") {
        const derKimlik = `derle-${Date.now()}`;
        const derKod = await gorevCalistirVeBekle(
          gorevNesne("u++: Derle", "u++: Derle", d.compileLine, d.cwd, true, derKimlik)
        );
        if (derKod !== 0) {
          return;
        }
        const calKimlik = `calistir-${Date.now()}`;
        await gorevCalistirVeBekle(
          gorevNesne("u++: Çalıştır", "u++: Çalıştır", quote(d.exePath), d.cwd, false, calKimlik)
        );
      } else {
        const kimlik = `c-${Date.now()}`;
        await gorevCalistirVeBekle(
          gorevNesne(
            "u++: C üret",
            "u++: C üret",
            d.compileLine.replace("--sadece-derle", "--sadece-c"),
            d.cwd,
            false,
            kimlik
          )
        );
      }
    } catch (e) {
      const msg = e instanceof Error ? e.message : String(e);
      void vscode.window.showErrorMessage(`u++ çalıştırma hatası: ${msg}`);
    }
  };

  context.subscriptions.push(
    vscode.tasks.registerTaskProvider("upp", new UppGorevSaglayici(motorAl)),
    vscode.commands.registerCommand("upp.derle", () => calis("derle")),
    vscode.commands.registerCommand("upp.calistir", () => calis("calistir")),
    vscode.commands.registerCommand("upp.sadeceC", () => calis("c"))
  );
}
