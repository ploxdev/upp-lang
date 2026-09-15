import assert from "node:assert/strict";
import path from "node:path";
import test from "node:test";
import { kokBul, uppcAdaylari, uppcBul } from "../src/kesif";

test("Windows aday sırası: özel yol, derleyici/uppc.exe, PATH", () => {
  const aday = uppcAdaylari({
    platform: "win32",
    repo: "C:\\proj",
    uppcYolu: "D:\\tools\\uppc.exe",
    pathDirs: ["C:\\msys64\\ucrt64\\bin"],
  });
  assert.equal(aday[0], "D:\\tools\\uppc.exe");
  assert.ok(aday.includes("C:\\proj\\derleyici\\uppc.exe"));
  assert.ok(aday.includes("C:\\msys64\\ucrt64\\bin\\uppc.exe"));
  assert.ok(!aday.some((p) => p.includes("derleme")));
});

test("Linux adayları: uppc sonra uppc.out, exe yok", () => {
  const aday = uppcAdaylari({
    platform: "linux",
    repo: "/home/a/upp",
    pathDirs: ["/usr/local/bin"],
  });
  assert.ok(aday.includes("/home/a/upp/derleyici/uppc"));
  assert.ok(aday.includes("/home/a/upp/derleyici/uppc.out"));
  assert.ok(aday.includes("/usr/local/bin/uppc"));
  assert.ok(!aday.some((p) => p.endsWith("uppc.exe")));
});

test("uppcBul ilk var olanı döner", () => {
  const files = new Set(["/repo/derleyici/uppc"]);
  const yol = uppcBul(
    { platform: "linux", repo: "/repo", pathDirs: ["/usr/bin"] },
    { dosyaVar: (p) => files.has(p) }
  );
  assert.equal(yol, "/repo/derleyici/uppc");
});

test("uppcBul yoksa undefined", () => {
  const yol = uppcBul(
    { platform: "win32", repo: "C:\\x" },
    { dosyaVar: () => false }
  );
  assert.equal(yol, undefined);
});

test("kokBul surucu.upp ile üste çıkar", () => {
  const files = new Set(["/repo/src/uppc/surucu.upp"]);
  const kok = kokBul(["/repo/ornekler/a.upp"], { dosyaVar: (p) => files.has(p) }, path.posix);
  assert.equal(kok, "/repo");
});

test("kokBul Windows derleyici/uppc.exe", () => {
  const files = new Set(["C:\\repo\\derleyici\\uppc.exe"]);
  const kok = kokBul(["C:\\repo\\eklenti"], { dosyaVar: (p) => files.has(p) }, path.win32);
  assert.equal(kok, "C:\\repo");
});

test("upp.uppcYolu dosya varsa herkesten önce", () => {
  const files = new Set(["C:\\ozel\\uppc.exe", "C:\\repo\\derleyici\\uppc.exe"]);
  const yol = uppcBul(
    {
      platform: "win32",
      repo: "C:\\repo",
      uppcYolu: "C:\\ozel\\uppc.exe",
    },
    { dosyaVar: (p) => files.has(p) }
  );
  assert.equal(yol, "C:\\ozel\\uppc.exe");
});
