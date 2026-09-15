import * as vscode from "vscode";
import { BicimSaglayici } from "./bicim";
import { registerDerleme } from "./derleme";
import { DurumCubugu } from "./durum";
import { findRepoRoot } from "./kok";
import { indirUrl, motorKur, yolBelirt, type Motor } from "./motor";
import { registerSozluk } from "./sozlukSaglayici";
import { TaniYoneticisi } from "./taniYoneticisi";

let durum: DurumCubugu;
let tani: TaniYoneticisi;
let motor: Motor = { hazir: false, neden: "taranmadı" };
let uyariGosterildi = false;

async function contextYaz(): Promise<void> {
  await vscode.commands.executeCommand("setContext", "upp.uppcHazir", motor.hazir);
}

async function tara(opts?: { sessiz?: boolean }): Promise<void> {
  const ed = vscode.window.activeTextEditor;
  const start = ed?.document.uri.fsPath;
  const repo = findRepoRoot(start) ?? findRepoRoot();
  motor = await motorKur(repo);
  durum.motorYaz(motor);
  tani.motorYaz(motor);
  await contextYaz();
  if (motor.hazir) {
    if (ed && ed.document.languageId === "upp") {
      void tani.calistir(ed.document);
    }
    return;
  }
  if (opts?.sessiz || uyariGosterildi) {
    return;
  }
  uyariGosterildi = true;
  const sec = await vscode.window.showWarningMessage(
    `u++: Temel mod. ${motor.neden} Syntax ve snippet açık; derleme için derleyici gerekli.`,
    "İndir",
    "Yol belirt",
    "Sonra"
  );
  if (sec === "İndir") {
    await vscode.env.openExternal(indirUrl());
  } else if (sec === "Yol belirt") {
    const p = await yolBelirt();
    if (p) {
      uyariGosterildi = false;
      await tara({ sessiz: true });
    }
  }
}

async function durumMenu(): Promise<void> {
  const items: vscode.QuickPickItem[] = [
    { label: "Derleyiciyi tara", description: motor.hazir ? motor.yol : motor.neden },
    { label: "Yol belirt" },
  ];
  if (motor.hazir) {
    items.push({ label: "Sürümü göster", description: motor.surum });
  }
  const sec = await vscode.window.showQuickPick(items, { title: "u++ derleyici" });
  if (!sec) {
    return;
  }
  if (sec.label === "Derleyiciyi tara") {
    uyariGosterildi = false;
    await tara();
  } else if (sec.label === "Yol belirt") {
    const p = await yolBelirt();
    if (p) {
      uyariGosterildi = false;
      await tara({ sessiz: true });
    }
  } else if (sec.label === "Sürümü göster" && motor.hazir) {
    void vscode.window.showInformationMessage(`u++ ${motor.surum} (protokol ${motor.protokol})\n${motor.yol}`);
  }
}

export async function activate(context: vscode.ExtensionContext): Promise<void> {
  durum = new DurumCubugu();
  tani = new TaniYoneticisi(durum, context);
  context.subscriptions.push(durum.statusBarItem);

  registerSozluk(context);
  registerDerleme(context, () => motor);
  context.subscriptions.push(
    vscode.languages.registerDocumentFormattingEditProvider(
      { language: "upp" },
      new BicimSaglayici(() => motor)
    )
  );

  context.subscriptions.push(
    vscode.commands.registerCommand("upp.tara", () => {
      uyariGosterildi = false;
      return tara();
    }),
    vscode.commands.registerCommand("upp.yolBelirt", async () => {
      const p = await yolBelirt();
      if (p) {
        uyariGosterildi = false;
        await tara({ sessiz: true });
      }
    }),
    vscode.commands.registerCommand("upp.durumMenu", () => durumMenu())
  );

  context.subscriptions.push(
    vscode.window.onDidChangeActiveTextEditor((ed) => {
      durum.ciz();
      if (ed && ed.document.languageId === "upp" && motor.hazir) {
        void tani.calistir(ed.document);
      }
    }),
    vscode.workspace.onDidChangeTextDocument((e) => {
      if (e.document.languageId === "upp" && motor.hazir) {
        tani.zamanla(e.document);
      }
    }),
    vscode.workspace.onDidSaveTextDocument((doc) => {
      if (doc.languageId === "upp" && motor.hazir) {
        void tani.calistir(doc);
      }
    }),
    vscode.workspace.onDidChangeConfiguration((e) => {
      if (e.affectsConfiguration("upp")) {
        uyariGosterildi = false;
        void tara({ sessiz: true });
      }
    }),
    vscode.tasks.onDidStartTask((e) => {
      if (e.execution.task.source === "u++") {
        durum.derlemeYaz(true);
      }
    }),
    vscode.tasks.onDidEndTask((e) => {
      if (e.execution.task.source === "u++") {
        durum.derlemeYaz(false);
      }
    }),
    vscode.languages.onDidChangeDiagnostics(() => {
      const ed = vscode.window.activeTextEditor;
      if (!ed || ed.document.languageId !== "upp") {
        return;
      }
      const n = vscode.languages
        .getDiagnostics(ed.document.uri)
        .filter((d) => d.severity === vscode.DiagnosticSeverity.Error).length;
      durum.hataYaz(n);
    })
  );

  durum.ciz();
  await tara();
}

export async function deactivate(): Promise<void> {
  /* yok */
}
