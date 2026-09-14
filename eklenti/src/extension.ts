import * as fs from "fs";
import * as path from "path";
import * as vscode from "vscode";
import {
  LanguageClient,
  LanguageClientOptions,
  ServerOptions,
  Trace,
} from "vscode-languageclient/node";
import { registerDerleme } from "./derleme";
import { cozPythonYolu, findRepoRoot, nativeExe, uppAyar } from "./kok";

let client: LanguageClient | undefined;
let status: vscode.StatusBarItem | undefined;

function durumGuncelle(): void {
  if (!status) {
    return;
  }
  const ed = vscode.window.activeTextEditor;
  if (!ed || ed.document.languageId !== "upp") {
    status.hide();
    return;
  }
  const diags = vscode.languages.getDiagnostics(ed.document.uri);
  const n = diags.filter((d) => d.severity === vscode.DiagnosticSeverity.Error).length;
  status.text = n > 0 ? `u++ v3.0 $(error) ${n}` : "u++ v3.0";
  status.tooltip = n > 0 ? `${n} tanı` : "u++ v3.0 — Derle";
  status.show();
}

function initOptions(kok: string): Record<string, unknown> {
  const ayar = uppAyar();
  const uppcYolu = nativeExe(kok);
  return {
    repoRoot: kok,
    derleyici: ayar.derleyici,
    uppcYolu,
    gccTanilari: ayar.gccTanilari,
    settings: {
      derleyici: ayar.derleyici,
      uppcYolu,
      gccTanilari: ayar.gccTanilari,
    },
  };
}

export async function activate(context: vscode.ExtensionContext): Promise<void> {
  const repo = findRepoRoot(context.extensionPath);
  if (!repo || !fs.existsSync(path.join(repo, "uppc.py"))) {
    void vscode.window.showErrorMessage(
      "u++ kökünü açın (uppc.py bu klasörde veya üstünde olmalı)."
    );
  }

  status = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Right, 100);
  status.command = "upp.derle";
  context.subscriptions.push(status);
  durumGuncelle();

  const kok = repo ?? findRepoRoot() ?? context.extensionPath;
  const ayar = uppAyar();
  const python = cozPythonYolu(ayar.pythonYolu);
  registerDerleme(context);

  if (!python) {
    void vscode.window.showErrorMessage(
      "u++ dil sunucusu için Python bulunamadı. Ayar: upp.pythonYolu (ör. C:\\\\Python314\\\\python.exe). Windows'ta Store 'python' kısayolunu kapatın (Ayarlar → Uygulamalar → Gelişmiş → Uygulama yürütme diğer adları)."
    );
    context.subscriptions.push(
      vscode.window.onDidChangeActiveTextEditor(() => durumGuncelle()),
      vscode.languages.onDidChangeDiagnostics(() => durumGuncelle())
    );
    return;
  }

  const serverScript = path.join(context.extensionPath, "server", "upp_lsp.py");
  const env = {
    ...process.env,
    PYTHONPATH: kok,
    UPP_KOK: kok,
    PYTHONIOENCODING: "utf-8",
  };

  const serverOptions: ServerOptions = {
    command: python,
    args: ["-u", serverScript],
    options: { cwd: kok, env },
  };

  const clientOptions: LanguageClientOptions = {
    documentSelector: [
      { scheme: "file", language: "upp" },
      { scheme: "untitled", language: "upp" },
    ],
    initializationOptions: initOptions(kok),
    synchronize: {
      configurationSection: "upp",
    },
    outputChannelName: "u++ Dil Sunucusu",
  };

  client = new LanguageClient("upp", "u++", serverOptions, clientOptions);
  await client.start();
  const izleme = Trace.fromString(ayar.izSunucu);
  client.setTrace(izleme === undefined ? Trace.Off : izleme);

  context.subscriptions.push(
    vscode.window.onDidChangeActiveTextEditor(() => durumGuncelle()),
    vscode.languages.onDidChangeDiagnostics(() => durumGuncelle()),
    vscode.workspace.onDidChangeConfiguration((e) => {
      if (!e.affectsConfiguration("upp") || !client) {
        return;
      }
      const guncel = uppAyar();
      const t = Trace.fromString(guncel.izSunucu);
      client.setTrace(t === undefined ? Trace.Off : t);
    }),
    { dispose: () => void client?.stop() }
  );
}

export async function deactivate(): Promise<void> {
  if (client) {
    await client.stop();
    client = undefined;
  }
}
