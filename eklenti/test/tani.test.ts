import assert from "node:assert/strict";
import test from "node:test";
import { taniHarita, taniKaynakUppMi } from "../src/tani";

test("ok true boş tanı", () => {
  assert.deepEqual(taniHarita('{"ok":true,"hatalar":[]}\n'), []);
});

test("hata satır/sütun/mesaj", () => {
  const t = taniHarita(
    '{"ok":false,"hatalar":[{"satir":4,"sutun":2,"tur":"hata","mesaj":"x"}]}\n'
  );
  assert.equal(t.length, 1);
  assert.equal(t[0].satir, 4);
  assert.equal(t[0].sutun, 2);
  assert.equal(t[0].mesaj, "x");
  assert.equal(t[0].guvenlik, false);
});

test("güvenlik turu", () => {
  const t = taniHarita(
    '{"ok":false,"hatalar":[{"satir":1,"sutun":1,"tur":"guvenlik","mesaj":"BELLEK"}]}\n'
  );
  assert.equal(t[0].guvenlik, true);
});

test("bozuk JSON tek tanı", () => {
  const t = taniHarita("not json");
  assert.equal(t.length, 1);
  assert.match(t[0].mesaj, /JSON/);
});

test("ok false hatalar yoksa sentetik tanı", () => {
  const t = taniHarita('{"ok":false}\n');
  assert.equal(t.length, 1);
  assert.match(t[0].mesaj, /ok:false/);
});

test("durum cubuğu yalnızca u++ kaynağı", () => {
  assert.equal(taniKaynakUppMi("u++"), true);
  assert.equal(taniKaynakUppMi("u++ güvenlik"), true);
  assert.equal(taniKaynakUppMi("eslint"), false);
});
