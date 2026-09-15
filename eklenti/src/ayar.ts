import * as vscode from "vscode";

export function uppAyar(): { uppcYolu: string; ciktiKoku: string } {
  const c = vscode.workspace.getConfiguration("upp");
  return {
    uppcYolu: (c.get<string>("uppcYolu") || "").trim(),
    ciktiKoku: c.get<string>("ciktiKoku") || "derleme",
  };
}
