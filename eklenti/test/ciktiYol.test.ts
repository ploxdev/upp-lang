import assert from "node:assert/strict";
import path from "node:path";
import test from "node:test";
import { ciktiStemRel } from "../src/ciktiYol";

test("aynı basename farklı klasörde çakışmaz", () => {
  const repo = path.resolve("repo");
  const kok = path.join(repo, "derleme");
  const a = ciktiStemRel(repo, path.join(repo, "src", "main.upp"), kok);
  const b = ciktiStemRel(repo, path.join(repo, "test", "main.upp"), kok);
  assert.notEqual(a, b);
  assert.equal(a, path.join(kok, "src", "main"));
  assert.equal(b, path.join(kok, "test", "main"));
});
