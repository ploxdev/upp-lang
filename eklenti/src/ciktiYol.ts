import * as path from "path";

/** Repo-göreli çıktı gövdesi: src/a.upp ve test/a.upp çakışmaz. */
export function ciktiStemRel(repo: string, file: string, ciktiKokAbs: string): string {
  let rel = path.relative(repo, file);
  if (!rel || rel.startsWith("..") || path.isAbsolute(rel)) {
    rel = path.basename(file);
  }
  const noExt = rel.replace(/\.[^.\\/]+$/, "");
  return path.join(ciktiKokAbs, noExt);
}
