import * as fs from "fs";
import * as path from "path";
import * as vscode from "vscode";
import { ciktiKok, cozPythonYolu, findRepoRoot, nativeExe, quote, uppAyar, ikiliUzanti, hedefBayrak, calistirSatiri } from "./kok";

export interface Derleme {
  cwd: string;
  compileLine: string;
  exePath: string;
}

export function derlemeHazir(doc: vscode.TextDocument): Derleme | undefined {
  const repo = findRepoRoot(path.dirname(doc.uri.fsPath));
  if (!repo) {
    void vscode.window.showErrorMessage("u++ kökü bulunamadı (uppc.py). Repoyu açın.");
    return undefined;
  }
  const file = doc.uri.fsPath;
  if (path.extname(file).toLowerCase() === ".uph") {
    void vscode.window.showErrorMessage(
      "Başlık dosyası (.uph) derlenmez; bir .upp programından 'kullan' ile katın."
    );
    return undefined;
  }
  const stem = path.join(ciktiKok(repo), path.basename(file, path.extname(file)));
  const exePath = `${stem}${ikiliUzanti()}`;
  const ayar = uppAyar();
  const native = nativeExe(repo);
  const hedef = hedefBayrak();
  let compileLine: string;
  if (ayar.derleyici === "native" && fs.existsSync(native)) {
    compileLine = [quote(native), quote(file), "--sadece-derle", "--cikti", quote(stem), ...hedef].join(" ");
  } else {
    if (ayar.derleyici === "native") {
      void vscode.window.showWarningMessage(
        `Native uppc yok (${native}); python uppc.py kullanılacak.`
      );
    }
    const py = cozPythonYolu(ayar.pythonYolu);
    if (!py) {
      void vscode.window.showErrorMessage(
        "Python bulunamadı. Ayar: upp.pythonYolu (ör. C:\\\\Python314\\\\python.exe)."
      );
      return undefined;
    }
    compileLine = [
      quote(py),
      quote(path.join(repo, "uppc.py")),
      quote(file),
      "--sadece-derle",
      "--cikti",
      quote(stem),
      ...hedef,
    ].join(" ");
  }
  return { cwd: repo, compileLine, exePath };
}

export async function gorevCalistir(ad: string, satir: string, cwd: string): Promise<void> {
  const def: vscode.TaskDefinition = { type: "upp", task: ad };
  const exec = new vscode.ShellExecution(satir, { cwd });
  const task = new vscode.Task(def, vscode.TaskScope.Workspace, ad, "u++", exec, [
    "$upp-hata",
    "$upp-guvenlik",
  ]);
  task.presentationOptions = {
    reveal: vscode.TaskRevealKind.Always,
    panel: vscode.TaskPanelKind.Dedicated,
  };
  await vscode.tasks.executeTask(task);
}

export async function aktifUpp(): Promise<vscode.TextDocument | undefined> {
  const ed = vscode.window.activeTextEditor;
  if (!ed || ed.document.languageId !== "upp") {
    void vscode.window.showInformationMessage("Açık bir .upp veya .uph dosyası seçin.");
    return undefined;
  }
  if (ed.document.isDirty) {
    await ed.document.save();
  }
  return ed.document;
}

export class UppTaskProvider implements vscode.TaskProvider {
  provideTasks(): vscode.Task[] {
    const ed = vscode.window.activeTextEditor;
    if (!ed || ed.document.languageId !== "upp") {
      return [];
    }
    const d = derlemeHazir(ed.document);
    if (!d) {
      return [];
    }
    const make = (ad: string, satir: string, grup?: vscode.TaskGroup): vscode.Task => {
      const t = new vscode.Task(
        { type: "upp", task: ad },
        vscode.TaskScope.Workspace,
        ad,
        "u++",
        new vscode.ShellExecution(satir, { cwd: d.cwd }),
        ["$upp-hata", "$upp-guvenlik"]
      );
      if (grup) {
        t.group = grup;
      }
      t.presentationOptions = {
        reveal: vscode.TaskRevealKind.Always,
        panel: vscode.TaskPanelKind.Dedicated,
      };
      return t;
    };
    return [
      make("Derle", d.compileLine, vscode.TaskGroup.Build),
      make("Çalıştır", calistirSatiri(d.compileLine, d.exePath)),
      make("Yalnızca C", d.compileLine.replace("--sadece-derle", "--sadece-c")),
    ];
  }

  resolveTask(task: vscode.Task): vscode.Task {
    return task;
  }
}

export function registerDerleme(context: vscode.ExtensionContext): void {
  context.subscriptions.push(
    vscode.commands.registerCommand("upp.derle", async () => {
      const doc = await aktifUpp();
      if (!doc) {
        return;
      }
      const d = derlemeHazir(doc);
      if (d) {
        await gorevCalistir("u++: Derle", d.compileLine, d.cwd);
      }
    }),
    vscode.commands.registerCommand("upp.calistir", async () => {
      const doc = await aktifUpp();
      if (!doc) {
        return;
      }
      const d = derlemeHazir(doc);
      if (d) {
        const satir = calistirSatiri(d.compileLine, d.exePath);
        await gorevCalistir("u++: Çalıştır", satir, d.cwd);
      }
    }),
    vscode.commands.registerCommand("upp.sadeceC", async () => {
      const doc = await aktifUpp();
      if (!doc) {
        return;
      }
      const d = derlemeHazir(doc);
      if (d) {
        await gorevCalistir(
          "u++: C üret",
          d.compileLine.replace("--sadece-derle", "--sadece-c"),
          d.cwd
        );
      }
    }),
    vscode.tasks.registerTaskProvider("upp", new UppTaskProvider())
  );
}
