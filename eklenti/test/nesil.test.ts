import assert from "node:assert/strict";
import test from "node:test";
import { nesilUygula } from "../src/nesil";

test("eski nesil uygulanmaz", () => {
  assert.equal(nesilUygula(3, 4), false);
});

test("güncel nesil uygulanır", () => {
  assert.equal(nesilUygula(4, 4), true);
});
