import * as vscode from "vscode";
import type { Motor } from "./motor";

export type DurumMod = "temel" | "hazir" | "derleniyor" | "hata";

export class DurumCubugu {
  private item: vscode.StatusBarItem;
  private motor: Motor = { hazir: false, neden: "taranmadı" };
  private derleniyor = false;
  private hataSayisi = 0;

  constructor() {
    this.item = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Right, 100);
    this.item.command = "upp.durumMenu";
  }

  get statusBarItem(): vscode.StatusBarItem {
    return this.item;
  }

  motorYaz(m: Motor): void {
    this.motor = m;
    this.ciz();
  }

  derlemeYaz(v: boolean): void {
    this.derleniyor = v;
    this.ciz();
  }

  hataYaz(n: number): void {
    this.hataSayisi = n;
    this.ciz();
  }

  motorAl(): Motor {
    return this.motor;
  }

  ciz(): void {
    const ed = vscode.window.activeTextEditor;
    if (!ed || ed.document.languageId !== "upp") {
      this.item.hide();
      return;
    }
    const mod = this.mod();
    this.item.backgroundColor = undefined;
    if (mod === "temel") {
      this.item.text = "u++: Temel mod";
      this.item.tooltip = this.motor.hazir ? "Temel mod" : this.motor.neden;
      this.item.backgroundColor = new vscode.ThemeColor("statusBarItem.warningBackground");
    } else if (mod === "derleniyor") {
      this.item.text = "u++: Derleniyor...";
      this.item.tooltip = "Derleme sürüyor";
    } else if (mod === "hata") {
      this.item.text = `u++: ${this.hataSayisi} hata`;
      this.item.tooltip = `${this.hataSayisi} tanı`;
      this.item.backgroundColor = new vscode.ThemeColor("statusBarItem.errorBackground");
    } else {
      const v = this.motor.hazir ? this.motor.surum : "";
      this.item.text = v ? `u++: Hazır (${v})` : "u++: Hazır";
      this.item.tooltip = this.motor.hazir ? this.motor.yol : "";
    }
    this.item.show();
  }

  mod(): DurumMod {
    if (!this.motor.hazir) {
      return "temel";
    }
    if (this.derleniyor) {
      return "derleniyor";
    }
    if (this.hataSayisi > 0) {
      return "hata";
    }
    return "hazir";
  }
}
