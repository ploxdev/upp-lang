import assert from "node:assert/strict";
import test from "node:test";
import { hoverMetin, kelimeAl, tamamla } from "../src/sozluk";
import { outlineSemboller } from "../src/outline";

test("fon öneki fonk verir", () => {
  const etiket = tamamla("fon").map((x) => x.etiket);
  assert.ok(etiket.includes("fonk"));
});

test("upp. ad alanları", () => {
  const etiket = tamamla("upp.").map((x) => x.etiket);
  assert.ok(etiket.includes("metin"));
  assert.ok(etiket.includes("sistem"));
  assert.ok(etiket.includes("yaz"));
});

test("upp.metin. kod/ileri/kes", () => {
  const etiket = tamamla("upp.metin.").map((x) => x.etiket);
  assert.ok(etiket.includes("kod"));
  assert.ok(etiket.includes("ileri"));
  assert.ok(etiket.includes("kes"));
});

test("noktadan sonra liste/harita yöntemleri", () => {
  const etiket = tamamla("xs.").map((x) => x.etiket);
  assert.ok(etiket.includes("ekle"));
  assert.ok(etiket.includes("sil"));
  assert.ok(etiket.includes("temizle"));
  assert.ok(etiket.includes("koy"));
});

test("hover fonk", () => {
  assert.match(hoverMetin("fonk") || "", /Fonksiyon/);
});

test("hover upp.metin.kod", () => {
  const { kelime, yol } = kelimeAl("    sayi x = upp.metin.kod", 28);
  assert.equal(kelime, "kod");
  assert.deepEqual(yol, ["upp", "metin"]);
  assert.match(hoverMetin(kelime, yol) || "", /Kod noktası/);
});

test("outline fonk ve sinif", () => {
  const s = outlineSemboller("fonk ana() -> sayi {\n}\n\nsinif Araba {\n}\n");
  assert.equal(s.length, 2);
  assert.equal(s[0].ad, "ana");
  assert.equal(s[0].tur, "fonk");
  assert.equal(s[1].ad, "Araba");
  assert.equal(s[1].tur, "sinif");
});
