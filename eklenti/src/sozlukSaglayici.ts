import * as vscode from "vscode";
import { hoverMetin, kelimeAl, tamamla, type TamamTur } from "./sozluk";
import { outlineSemboller } from "./outline";

function kind(tur: TamamTur): vscode.CompletionItemKind {
  switch (tur) {
    case "anahtar":
      return vscode.CompletionItemKind.Keyword;
    case "tip":
      return vscode.CompletionItemKind.TypeParameter;
    case "fonksiyon":
      return vscode.CompletionItemKind.Function;
    case "adalan":
      return vscode.CompletionItemKind.Module;
    case "yontem":
      return vscode.CompletionItemKind.Method;
  }
}

export class SozlukTamamlama implements vscode.CompletionItemProvider {
  provideCompletionItems(doc: vscode.TextDocument, pos: vscode.Position): vscode.CompletionItem[] {
    const satir = doc.lineAt(pos.line).text.slice(0, pos.character);
    return tamamla(satir).map((it) => {
      const c = new vscode.CompletionItem(it.etiket, kind(it.tur));
      c.detail = it.ayrinti;
      c.documentation = new vscode.MarkdownString(it.ayrinti);
      return c;
    });
  }
}

export class SozlukHover implements vscode.HoverProvider {
  provideHover(doc: vscode.TextDocument, pos: vscode.Position): vscode.Hover | undefined {
    const satir = doc.lineAt(pos.line).text;
    const { kelime, yol } = kelimeAl(satir, pos.character);
    if (!kelime) {
      return undefined;
    }
    const md = hoverMetin(kelime, yol);
    if (!md) {
      return undefined;
    }
    return new vscode.Hover(new vscode.MarkdownString(md));
  }
}

export class SozlukOutline implements vscode.DocumentSymbolProvider {
  provideDocumentSymbols(doc: vscode.TextDocument): vscode.DocumentSymbol[] {
    return outlineSemboller(doc.getText()).map((s) => {
      const kind =
        s.tur === "fonk"
          ? vscode.SymbolKind.Function
          : s.tur === "sinif"
            ? vscode.SymbolKind.Class
            : vscode.SymbolKind.Enum;
      const range = doc.lineAt(s.satir - 1).range;
      const sel = new vscode.Range(s.satir - 1, s.sutun - 1, s.satir - 1, s.sutun - 1 + s.ad.length);
      return new vscode.DocumentSymbol(s.ad, s.tur, kind, range, sel);
    });
  }
}

export function registerSozluk(context: vscode.ExtensionContext): void {
  const sel = { language: "upp" };
  context.subscriptions.push(
    vscode.languages.registerCompletionItemProvider(sel, new SozlukTamamlama(), "."),
    vscode.languages.registerHoverProvider(sel, new SozlukHover()),
    vscode.languages.registerDocumentSymbolProvider(sel, new SozlukOutline())
  );
}
