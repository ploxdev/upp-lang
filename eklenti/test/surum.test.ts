import assert from "node:assert/strict";
import test from "node:test";
import { surumCoz } from "../src/surum";

test("geçerli --surum --json", () => {
  const s = surumCoz('{"dil":"u++","surum":"3.0.1","protokol":1}\n');
  assert.equal(s.ok, true);
  if (s.ok) {
    assert.equal(s.surum, "3.0.1");
    assert.equal(s.protokol, 1);
  }
});

test("düz metin JSON değil", () => {
  const s = surumCoz("u++ 3.0.1\n");
  assert.equal(s.ok, false);
  if (!s.ok) {
    assert.equal(s.neden, "json_yok");
  }
});

test("yanlış dil", () => {
  const s = surumCoz('{"dil":"c","surum":"3.0.1","protokol":1}');
  assert.equal(s.ok, false);
  if (!s.ok) {
    assert.equal(s.neden, "dil");
  }
});

test("eksik protokol", () => {
  const s = surumCoz('{"dil":"u++","surum":"3.0.1"}');
  assert.equal(s.ok, false);
  if (!s.ok) {
    assert.equal(s.neden, "eksik");
  }
});

test("protokol 0 eski derleyici", () => {
  const s = surumCoz('{"dil":"u++","surum":"1.0.0","protokol":0}');
  assert.equal(s.ok, false);
  if (!s.ok) {
    assert.equal(s.neden, "eski_derleyici");
  }
});

test("protokol 99 eski eklenti", () => {
  const s = surumCoz('{"dil":"u++","surum":"9.0.0","protokol":99}');
  assert.equal(s.ok, false);
  if (!s.ok) {
    assert.equal(s.neden, "eski_eklenti");
  }
});
