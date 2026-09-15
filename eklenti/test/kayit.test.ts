import assert from "node:assert/strict";
import test from "node:test";
import { kayitSonrasiDevam } from "../src/kayit";

test("save false derlemeyi keser", () => {
  assert.equal(kayitSonrasiDevam(false), false);
});

test("save true derlemeye izin verir", () => {
  assert.equal(kayitSonrasiDevam(true), true);
});
