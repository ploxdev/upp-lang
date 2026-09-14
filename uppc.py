#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
u++ v2.7 Derleyicisi — uppc.py
==============================

Türkçe sözdizimli kaynak kodu C11'e transpile eder, bellek güvenliğini
denetler ve GCC ile native ikili üretir (Windows .exe / Linux .out).

Kullanım:
    python uppc.py ornekler/ornek1.upp
    python uppc.py ornekler/ornek1.upp --cikti ornek1
    python uppc.py ornekler/ornek1.upp --sadece-c
    python uppc.py ornekler/ornek2.upp --sadece-derle
    python uppc.py dosya.upp --linux
    python uppc.py dosya.upp --windows
    python uppc.py dosya.upp --ast
    python uppc.py dosya.upp --analiz
    python uppc.py dosya.upp --json-hata
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass, field
from typing import Any, Dict, List, Optional, Tuple, Union


# ---------------------------------------------------------------------------
# Hata türleri
# ---------------------------------------------------------------------------

class UppError(Exception):
    def __init__(
        self,
        line: int,
        message: str,
        column: Optional[int] = None,
        kaynak: Optional[str] = None,
    ):
        self.line = line
        self.column = column
        self.message = message
        self.kaynak = kaynak or ""
        super().__init__(self._format())

    def _loc(self) -> str:
        loc = f"Satır {self.line}"
        if self.column is not None:
            loc += f":{self.column}"
        if self.kaynak:
            loc += f" ({self.kaynak})"
        return loc

    def _format(self) -> str:
        return f"[u++ HATA] {self._loc()}: {self.message}"

    def __str__(self) -> str:
        return self._format()


class SafetyError(UppError):
    def _format(self) -> str:
        return (
            f"[u++ BELLEK GÜVENLİĞİ İHLALİ] {self._loc()}: {self.message}\n"
            f"  -> İpucu: Bu işlemi 'guvensiz {{ ... }}' bloğu içerisine alın."
        )


# ---------------------------------------------------------------------------
# Lexer
# ---------------------------------------------------------------------------

@dataclass(frozen=True)
class Token:
    type: str
    value: str
    line: int
    column: int


KEYWORDS = {
    "fonk", "don", "eger", "yoksa", "iken", "dongu", "dur", "devam",
    "sayi", "ondalik", "metin", "mantik", "bos", "liste", "harita",
    "dogru", "yanlis", "yok",
    "guvensiz", "guvenli", "arkaplan",
    "sinif", "kendi", "oto", "c_ekle", "kullan", "olarak", "sabit",
    "secim", "durum", "varsayilan", "her", "in", "secenek",
}

TYPE_NAMES = {"sayi", "ondalik", "metin", "mantik", "bos", "liste", "harita"}
KOLEKSIYON_ELEMAN = ("sayi", "ondalik", "mantik", "metin")
HARITA_ANAHTAR = ("sayi", "metin")

_MULTI_OPS = [
    ("=>", "YAG_OK"),
    ("->", "OK"),
    ("==", "ESIT_ESIT"),
    ("!=", "ESIT_DEGIL"),
    ("<=", "KUCUK_ESIT"),
    (">=", "BUYUK_ESIT"),
    ("&&", "VE"),
    ("||", "VEYA"),
    ("+=", "ARTI_ATAMA"),
    ("-=", "EKSI_ATAMA"),
    ("*=", "YILDIZ_ATAMA"),
    ("/=", "BOLU_ATAMA"),
    ("%=", "MOD_ATAMA"),
]

_ASSIGN_OPS = {
    "ATAMA": "=",
    "ARTI_ATAMA": "+=",
    "EKSI_ATAMA": "-=",
    "YILDIZ_ATAMA": "*=",
    "BOLU_ATAMA": "/=",
    "MOD_ATAMA": "%=",
}

_SINGLE_OPS = {
    "+": "ARTI",
    "-": "EKSI",
    "*": "YILDIZ",
    "/": "BOLU",
    "%": "MOD",
    "=": "ATAMA",
    "<": "KUCUK",
    ">": "BUYUK",
    "!": "UNLEM",
    "&": "VE_ISARET",
    "|": "VAY_ISARET",
    "^": "XOR",
    "(": "PARANTEZ_AC",
    ")": "PARANTEZ_KAPA",
    "{": "SLUS_AC",
    "}": "SLUS_KAPA",
    "[": "KOSE_AC",
    "]": "KOSE_KAPA",
    ";": "NOKTALI_VIRGUL",
    ",": "VIRGUL",
    ".": "NOKTA",
    ":": "IKI_NOKTA",
    "?": "SORU",
}

_IDENT_START = set("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_çğıöşüÇĞİÖŞÜ")
_IDENT_CONT = _IDENT_START | set("0123456789")


class Lexer:
    def __init__(self, source: str):
        self.source = source
        self.pos = 0
        self.line = 1
        self.column = 1
        self.length = len(source)
        self.kaynak_ad = ""

    def _err(
        self,
        message: str,
        line: Optional[int] = None,
        column: Optional[int] = None,
    ) -> None:
        raise UppError(
            self.line if line is None else line,
            message,
            self.column if column is None else column,
            self.kaynak_ad or None,
        )

    def tokenize(self) -> List[Token]:
        tokens: List[Token] = []
        while self.pos < self.length:
            ch = self.source[self.pos]
            if ch in " \t\r":
                self._advance()
                continue
            if ch == "\n":
                self._advance()
                continue
            if ch == "/" and self._peek_char(1) == "/":
                self._skip_line_comment()
                continue
            if ch == "/" and self._peek_char(1) == "*":
                self._skip_block_comment()
                continue
            if ch == '"':
                tokens.append(self._read_string())
                continue
            if ch.isdigit():
                tokens.append(self._read_number())
                continue
            multi = self._match_multi_op()
            if multi is not None:
                tokens.append(multi)
                continue
            if ch in _SINGLE_OPS:
                tokens.append(Token(_SINGLE_OPS[ch], ch, self.line, self.column))
                self._advance()
                continue
            if ch in _IDENT_START:
                tokens.extend(self._read_ident_or_embed())
                continue
            self._err(f"Tanımlanamayan simge: '{ch}'")
        tokens.append(Token("EOF", "", self.line, self.column))
        return tokens

    def _peek_char(self, offset: int = 0) -> str:
        i = self.pos + offset
        return self.source[i] if i < self.length else ""

    def _advance(self) -> str:
        ch = self.source[self.pos]
        self.pos += 1
        if ch == "\n":
            self.line += 1
            self.column = 1
        else:
            self.column += 1
        return ch

    def _skip_line_comment(self) -> None:
        self._advance()
        self._advance()
        start = self.pos
        while self.pos < self.length and self.source[self.pos] != "\n":
            self._advance()
        govde = self.source[start:self.pos].lstrip(" \t")
        if govde.startswith("@upp-kaynak"):
            ad = govde[len("@upp-kaynak"):].strip(" \t\r")
            if ad:
                self.kaynak_ad = ad
                self.line = 0
                self.column = 1

    def _skip_block_comment(self) -> None:
        start_line, start_col = self.line, self.column
        self._advance()
        self._advance()
        while self.pos < self.length:
            if self.source[self.pos] == "*" and self._peek_char(1) == "/":
                self._advance()
                self._advance()
                return
            self._advance()
        self._err("Kapatılmamış blok yorumu (/* ... */)", start_line, start_col)

    def _read_string(self) -> Token:
        line, col = self.line, self.column
        if self._peek_char(1) == '"' and self._peek_char(2) == '"':
            return self._read_triple_string()
        buf = [self._advance()]
        while self.pos < self.length:
            ch = self.source[self.pos]
            if ch == "\n":
                self._err("Metin sabitinde satır sonu. Metni kapatın veya \\n kullanın.", line, col)
            buf.append(self._advance())
            if ch == "\\":
                if self.pos >= self.length:
                    self._err("Metin sabiti kaçış karakterinden sonra bitti.", line, col)
                buf.append(self._advance())
                continue
            if ch == '"':
                return Token("METIN", "".join(buf), line, col)
        self._err("Kapatılmamış metin sabiti.", line, col)

    def _read_triple_string(self) -> Token:
        line, col = self.line, self.column
        buf = [self._advance(), self._advance(), self._advance()]
        while self.pos < self.length:
            if (
                self.source[self.pos] == '"'
                and self._peek_char(1) == '"'
                and self._peek_char(2) == '"'
            ):
                buf.append(self._advance())
                buf.append(self._advance())
                buf.append(self._advance())
                return Token("METIN", "".join(buf), line, col)
            buf.append(self._advance())
        self._err('Kapatılmamış çok satırlı metin sabiti ("""...""").', line, col)

    def _read_number(self) -> Token:
        line, col = self.line, self.column
        start = self.pos
        if (
            self.source[self.pos] == "0"
            and self.pos + 1 < self.length
            and self.source[self.pos + 1] in ("x", "X")
        ):
            self._advance()
            self._advance()
            digits = 0
            while self.pos < self.length and self.source[self.pos] in "0123456789abcdefABCDEF":
                self._advance()
                digits += 1
            if digits == 0:
                self._err("'0x' sonrası onaltılık basamak bekleniyordu.", line, col)
            lex = self.source[start:self.pos]
            return Token("SAYI", str(int(lex, 16)), line, col)
        while self.pos < self.length and self.source[self.pos].isdigit():
            self._advance()
        if self.pos < self.length and self.source[self.pos] == "." and self._peek_char(1).isdigit():
            self._advance()
            while self.pos < self.length and self.source[self.pos].isdigit():
                self._advance()
            return Token("ONDALIK", self.source[start:self.pos], line, col)
        return Token("SAYI", self.source[start:self.pos], line, col)

    def _match_multi_op(self) -> Optional[Token]:
        for lexeme, ttype in _MULTI_OPS:
            n = len(lexeme)
            if self.source[self.pos:self.pos + n] == lexeme:
                line, col = self.line, self.column
                for _ in range(n):
                    self._advance()
                return Token(ttype, lexeme, line, col)
        return None

    def _read_ident_or_embed(self) -> List[Token]:
        line, col = self.line, self.column
        start = self.pos
        while self.pos < self.length and self.source[self.pos] in _IDENT_CONT:
            self._advance()
        value = self.source[start:self.pos]
        if value in ("c_kod", "asm_kod") and self._next_nonspace_is("{"):
            self._skip_space_and_comments()
            embed = self._read_raw_brace_block(
                "C_GOMMU" if value == "c_kod" else "ASM_GOMMU",
                line,
                col,
            )
            return [embed]
        ttype = "ANAHTAR" if value in KEYWORDS else "KIMLIK"
        return [Token(ttype, value, line, col)]

    def _next_nonspace_is(self, ch: str) -> bool:
        i = self.pos
        n = self.length
        while i < n:
            c = self.source[i]
            if c in " \t\r\n":
                i += 1
                continue
            if c == "/" and i + 1 < n and self.source[i + 1] == "/":
                i += 2
                while i < n and self.source[i] != "\n":
                    i += 1
                continue
            if c == "/" and i + 1 < n and self.source[i + 1] == "*":
                i += 2
                while i + 1 < n and not (self.source[i] == "*" and self.source[i + 1] == "/"):
                    i += 1
                i = min(i + 2, n)
                continue
            return c == ch
        return False

    def _skip_space_and_comments(self) -> None:
        while self.pos < self.length:
            ch = self.source[self.pos]
            if ch in " \t\r\n":
                self._advance()
                continue
            if ch == "/" and self._peek_char(1) == "/":
                self._skip_line_comment()
                continue
            if ch == "/" and self._peek_char(1) == "*":
                self._skip_block_comment()
                continue
            break

    def _read_raw_brace_block(self, kind: str, line: int, col: int) -> Token:
        if self.pos >= self.length or self.source[self.pos] != "{":
            self._err(f"'{kind}' bloğu '{{' ile başlamalı.", line, col)
        self._advance()
        inner_start = self.pos
        depth = 1
        in_str = False
        in_char = False
        escape = False
        in_line = False
        in_block = False
        while self.pos < self.length and depth > 0:
            ch = self.source[self.pos]
            nxt = self._peek_char(1)
            if in_line:
                self._advance()
                if ch == "\n":
                    in_line = False
                continue
            if in_block:
                self._advance()
                if ch == "*" and nxt == "/":
                    if self.pos < self.length:
                        self._advance()
                    in_block = False
                continue
            if in_str or in_char:
                self._advance()
                if escape:
                    escape = False
                    continue
                if ch == "\\":
                    escape = True
                    continue
                if in_str and ch == '"':
                    in_str = False
                elif in_char and ch == "'":
                    in_char = False
                continue
            if ch == "/" and nxt == "/":
                in_line = True
                self._advance()
                continue
            if ch == "/" and nxt == "*":
                in_block = True
                self._advance()
                continue
            if ch == '"':
                in_str = True
                self._advance()
                continue
            if ch == "'":
                in_char = True
                self._advance()
                continue
            if ch == "{":
                depth += 1
                self._advance()
                continue
            if ch == "}":
                depth -= 1
                self._advance()
                continue
            self._advance()
        if depth != 0:
            self._err("Kapatılmamış gömülü C/asm bloğu.", line, col)
        inner = self.source[inner_start:self.pos - 1]
        return Token(kind, inner, line, col)


# ---------------------------------------------------------------------------
# AST
# ---------------------------------------------------------------------------

@dataclass
class Node:
    line: int
    column: int = 1


@dataclass
class TypeNode(Node):
    name: str = ""
    pointer_depth: int = 0
    array_dims: int = 0
    array_len: int = 0


@dataclass
class Program(Node):
    decls: List[Union["CInclude", "ClassDecl", "FunctionDecl", "EnumDecl", "VarDecl", "KullanDecl"]] = field(default_factory=list)


@dataclass
class EnumVariant(Node):
    name: str = ""
    value: Optional[int] = None


@dataclass
class EnumDecl(Node):
    name: str = ""
    variants: List[EnumVariant] = field(default_factory=list)
    kaynak: str = ""
    mod_alias: str = ""


@dataclass
class CInclude(Node):
    spec: str = ""


@dataclass
class KullanDecl(Node):
    spec: str = ""
    alias: str = ""


@dataclass
class Param(Node):
    type: Optional[TypeNode] = None
    name: str = ""
    default: Optional["Expr"] = None


@dataclass
class FunctionDecl(Node):
    name: str = ""
    params: List[Param] = field(default_factory=list)
    return_type: TypeNode = field(default_factory=lambda: TypeNode(0, 1, "bos", 0))
    body: Optional["Block"] = None
    class_name: Optional[str] = None
    is_arrow: bool = False
    return_explicit: bool = False
    kaynak: str = ""
    mod_alias: str = ""


@dataclass
class FieldDecl(Node):
    type: TypeNode = field(default_factory=lambda: TypeNode(0, 1, "sayi", 0))
    name: str = ""
    array_size: Optional["Expr"] = None
    unsized_array: bool = False


@dataclass
class ClassDecl(Node):
    name: str = ""
    fields: List[FieldDecl] = field(default_factory=list)
    methods: List[FunctionDecl] = field(default_factory=list)
    constants: List["VarDecl"] = field(default_factory=list)
    kaynak: str = ""
    mod_alias: str = ""


@dataclass
class VarDecl(Node):
    type: TypeNode = field(default_factory=lambda: TypeNode(0, 1, "sayi", 0))
    name: str = ""
    array_size: Optional["Expr"] = None
    unsized_array: bool = False
    init: Optional["Expr"] = None
    inferred: bool = False
    is_const: bool = False
    kaynak: str = ""
    mod_alias: str = ""


@dataclass
class Block(Node):
    stmts: List["Stmt"] = field(default_factory=list)


@dataclass
class UnsafeBlock(Node):
    body: Optional[Block] = None


@dataclass
class SafeBlock(Node):
    body: Optional[Block] = None


@dataclass
class BackgroundBlock(Node):
    body: Optional[Block] = None


@dataclass
class BackgroundExpr(Node):
    body: Optional[Block] = None


@dataclass
class SwitchCase(Node):
    value: Optional["Expr"] = None
    stmts: List["Stmt"] = field(default_factory=list)


@dataclass
class SwitchStmt(Node):
    expr: Optional["Expr"] = None
    cases: List[SwitchCase] = field(default_factory=list)


@dataclass
class ForEachStmt(Node):
    var_name: str = ""
    var_type: TypeNode = field(default_factory=lambda: TypeNode(0, 1, "", 0))
    iterable: Optional["Expr"] = None
    body: Optional["Stmt"] = None
    inferred: bool = True


@dataclass
class IfStmt(Node):
    cond: Optional["Expr"] = None
    then_branch: Optional["Stmt"] = None
    else_branch: Optional["Stmt"] = None


@dataclass
class WhileStmt(Node):
    cond: Optional["Expr"] = None
    body: Optional["Stmt"] = None


@dataclass
class ForStmt(Node):
    init: Optional[Union[VarDecl, "ExprStmt"]] = None
    cond: Optional["Expr"] = None
    step: Optional["Expr"] = None
    body: Optional["Stmt"] = None


@dataclass
class ReturnStmt(Node):
    value: Optional["Expr"] = None


@dataclass
class BreakStmt(Node):
    pass


@dataclass
class ContinueStmt(Node):
    pass


@dataclass
class ExprStmt(Node):
    expr: Optional["Expr"] = None


@dataclass
class EmptyStmt(Node):
    pass


@dataclass
class CEmbed(Node):
    code: str = ""


@dataclass
class AsmEmbed(Node):
    code: str = ""


@dataclass
class Assign(Node):
    target: Optional["Expr"] = None
    value: Optional["Expr"] = None
    op: str = "="


@dataclass
class TernaryOp(Node):
    cond: Optional["Expr"] = None
    then_expr: Optional["Expr"] = None
    else_expr: Optional["Expr"] = None


@dataclass
class BinaryOp(Node):
    op: str = ""
    left: Optional["Expr"] = None
    right: Optional["Expr"] = None


@dataclass
class UnaryOp(Node):
    op: str = ""
    operand: Optional["Expr"] = None


@dataclass
class Call(Node):
    callee: Optional["Expr"] = None
    args: List["Expr"] = field(default_factory=list)


@dataclass
class Index(Node):
    array: Optional["Expr"] = None
    index: Optional["Expr"] = None


@dataclass
class Member(Node):
    obj: Optional["Expr"] = None
    name: str = ""


@dataclass
class Ident(Node):
    name: str = ""


@dataclass
class Literal(Node):
    value: Any = None
    kind: str = ""


@dataclass
class ArrayLiteral(Node):
    elements: List["Expr"] = field(default_factory=list)


Expr = Union[Assign, TernaryOp, BinaryOp, UnaryOp, Call, Index, Member, Ident, Literal, ArrayLiteral, BackgroundExpr]
Stmt = Union[
    VarDecl, Block, UnsafeBlock, SafeBlock, BackgroundBlock, IfStmt, WhileStmt, ForStmt,
    SwitchStmt, ForEachStmt,
    ReturnStmt, BreakStmt, ContinueStmt, ExprStmt, EmptyStmt,
    CEmbed, AsmEmbed,
]


# ---------------------------------------------------------------------------
# Parser
# ---------------------------------------------------------------------------

class Parser:
    def __init__(self, tokens: List[Token]):
        self.tokens = tokens
        self.index = 0
        self.loop_depth = 0
        self.switch_depth = 0

    def parse(self, require_entry: bool = True) -> Program:
        decls: List[Union[CInclude, ClassDecl, FunctionDecl, EnumDecl, VarDecl, KullanDecl]] = []
        first = self._peek()
        while not self._check("EOF"):
            if self._check_kw("c_ekle"):
                decls.append(self._parse_include())
            elif self._check_kw("kullan"):
                decls.append(self._parse_kullan())
            elif self._check_kw("sabit"):
                decls.append(self._parse_const_decl())
            elif self._check_kw("sinif"):
                decls.append(self._parse_class())
            elif self._check_kw("secenek"):
                decls.append(self._parse_enum())
            elif self._check_kw("fonk"):
                decls.append(self._parse_function())
            elif self._check_kw("oto") or self._is_type_start():
                decls.append(self._parse_oto_decl() if self._check_kw("oto") else self._parse_var_decl())
            else:
                tok = self._peek()
                got = tok.value if tok.value else tok.type
                raise UppError(
                    tok.line,
                    f"Üst düzeyde 'c_ekle', 'kullan', 'sabit', değişken, 'sinif', 'secenek' veya 'fonk' bekleniyordu, '{got}' bulundu.",
                    tok.column,
                )
        ana_decls = [d for d in decls if isinstance(d, FunctionDecl) and d.name == "ana"]
        if require_entry:
            if not ana_decls:
                raise UppError(first.line, "Giriş noktası bulunamadı. Bir 'fonk ana()' tanımlayın.")
            if len(ana_decls) > 1:
                extra = ana_decls[1]
                raise UppError(
                    extra.line,
                    "Birden fazla 'fonk ana()' tanımlanamaz; giriş noktası tek olmalıdır.",
                    extra.column,
                )
        return Program(first.line, first.column, decls)

    def _peek(self, offset: int = 0) -> Token:
        i = self.index + offset
        if i >= len(self.tokens):
            return self.tokens[-1]
        return self.tokens[i]

    def _advance(self) -> Token:
        tok = self._peek()
        if tok.type != "EOF":
            self.index += 1
        return tok

    def _check(self, ttype: str, value: Optional[str] = None) -> bool:
        tok = self._peek()
        if tok.type != ttype:
            return False
        if value is not None and tok.value != value:
            return False
        return True

    def _check_kw(self, word: str) -> bool:
        return self._check("ANAHTAR", word)

    def _match(self, ttype: str) -> Optional[Token]:
        if self._check(ttype):
            return self._advance()
        return None

    def _match_kw(self, word: str) -> Optional[Token]:
        if self._check_kw(word):
            return self._advance()
        return None

    def _expect(self, ttype: str, display: Optional[str] = None) -> Token:
        tok = self._peek()
        if tok.type != ttype:
            shown = display or ttype
            got = tok.value if tok.value else tok.type
            raise UppError(tok.line, f"'{shown}' bekleniyordu, '{got}' bulundu.", tok.column)
        return self._advance()

    def _expect_kw(self, word: str) -> Token:
        tok = self._peek()
        if not self._check_kw(word):
            got = tok.value if tok.value else tok.type
            raise UppError(tok.line, f"'{word}' anahtar sözcüğü bekleniyordu, '{got}' bulundu.", tok.column)
        return self._advance()

    def _expect_ident(self, allow_type_kw: bool = False) -> Token:
        tok = self._peek()
        if tok.type == "KIMLIK":
            return self._advance()
        if tok.type == "ANAHTAR":
            if allow_type_kw and tok.value in TYPE_NAMES:
                return self._advance()
            raise UppError(tok.line, f"'{tok.value}' bir anahtar sözcüktür; tanımlayıcı olarak kullanılamaz.", tok.column)
        raise UppError(tok.line, f"Tanımlayıcı bekleniyordu, '{tok.value}' bulundu.", tok.column)

    def _is_type_start(self) -> bool:
        tok = self._peek()
        if tok.type == "ANAHTAR" and tok.value in TYPE_NAMES:
            return True
        if tok.type == "KIMLIK":
            i = 1
            if self._peek(i).type == "NOKTA" and self._peek(i + 1).type == "KIMLIK":
                i = 3
            while self._peek(i).type == "YILDIZ":
                i += 1
            return self._peek(i).type == "KIMLIK"
        return False

    def _parse_include(self) -> CInclude:
        kw = self._expect_kw("c_ekle")
        spec_tok = self._expect("METIN", "başlık dizgesi")
        self._expect("NOKTALI_VIRGUL", ";")
        inner = decode_string_literal(spec_tok.value)
        if not inner.strip():
            raise UppError(kw.line, "c_ekle boş bir başlık alamaz.", kw.column)
        return CInclude(kw.line, kw.column, inner.strip())

    def _parse_kullan(self) -> KullanDecl:
        kw = self._expect_kw("kullan")
        spec_tok = self._expect("METIN", "kütüphane yolu")
        alias = ""
        if self._check_kw("olarak"):
            self._advance()
            ad = self._expect_ident()
            alias = ad.value
            if alias in TYPE_NAMES or alias in ("upp", "ana", "main"):
                raise UppError(ad.line, f"'{alias}' 'kullan ... olarak' adı olamaz.", ad.column)
        self._expect("NOKTALI_VIRGUL", ";")
        inner = decode_string_literal(spec_tok.value).strip()
        if not inner:
            raise UppError(kw.line, "'kullan' boş bir yol alamaz.", kw.column)
        return KullanDecl(kw.line, kw.column, inner, alias)

    def _parse_const_decl(self) -> VarDecl:
        kw = self._expect_kw("sabit")
        typ = self._parse_type()
        name = self._expect_ident()
        if not self._match("ATAMA"):
            raise UppError(
                name.line,
                f"'sabit {name.value}' bir başlatıcı gerektirir: 'sabit {typ.name} {name.value} = ...;'.",
                name.column,
            )
        init = self._parse_expression()
        self._expect("NOKTALI_VIRGUL", ";")
        return VarDecl(name.line, name.column, typ, name.value, None, False, init, False, True)

    def _parse_class(self) -> ClassDecl:
        kw = self._expect_kw("sinif")
        name_tok = self._expect_ident()
        if name_tok.value == "upp":
            raise UppError(name_tok.line, "'upp' yerleşik ad alanıdır; sınıf adı olamaz.", name_tok.column)
        self._expect("SLUS_AC", "{")
        fields: List[FieldDecl] = []
        methods: List[FunctionDecl] = []
        constants: List[VarDecl] = []
        seen: set = set()
        while not self._check("SLUS_KAPA") and not self._check("EOF"):
            if self._check_kw("fonk"):
                method = self._parse_function(class_name=name_tok.value)
                if method.name in seen:
                    raise UppError(method.line, f"'{name_tok.value}' içinde '{method.name}' birden fazla kez tanımlandı.", method.column)
                seen.add(method.name)
                methods.append(method)
            elif self._check_kw("sabit"):
                c = self._parse_const_decl()
                if c.name in seen:
                    raise UppError(c.line, f"'{name_tok.value}' içinde '{c.name}' birden fazla kez tanımlandı.", c.column)
                seen.add(c.name)
                constants.append(c)
            elif self._is_type_start():
                field = self._parse_field()
                if field.name in seen:
                    raise UppError(field.line, f"'{name_tok.value}' içinde '{field.name}' birden fazla kez tanımlandı.", field.column)
                if field.name == "kendi":
                    raise UppError(field.line, "'kendi' alan adı olarak kullanılamaz.", field.column)
                seen.add(field.name)
                fields.append(field)
            else:
                tok = self._peek()
                got = tok.value if tok.value else tok.type
                raise UppError(tok.line, f"Sınıf gövdesinde alan, 'sabit' veya 'fonk' bekleniyordu, '{got}' bulundu.", tok.column)
        self._expect("SLUS_KAPA", "}")
        return ClassDecl(kw.line, kw.column, name_tok.value, fields, methods, constants)

    def _parse_enum(self) -> EnumDecl:
        kw = self._expect_kw("secenek")
        name_tok = self._expect_ident()
        if name_tok.value in TYPE_NAMES or name_tok.value in ("upp", "ana", "main"):
            raise UppError(name_tok.line, f"'{name_tok.value}' seçenek adı olarak kullanılamaz.", name_tok.column)
        self._expect("SLUS_AC", "{")
        variants: List[EnumVariant] = []
        seen: set = set()
        next_val = 0
        while not self._check("SLUS_KAPA") and not self._check("EOF"):
            ident = self._expect_ident()
            if ident.value in seen:
                raise UppError(ident.line, f"'{name_tok.value}' içinde '{ident.value}' birden fazla kez tanımlandı.", ident.column)
            seen.add(ident.value)
            value = next_val
            if self._match("ATAMA"):
                num = self._expect("SAYI", "tam sayı")
                value = int(num.value)
            variants.append(EnumVariant(ident.line, ident.column, ident.value, value))
            next_val = value + 1
            if not self._match("VIRGUL"):
                break
        self._expect("SLUS_KAPA", "}")
        if not variants:
            raise UppError(name_tok.line, f"'{name_tok.value}' en az bir seçenek değeri içermelidir.", name_tok.column)
        return EnumDecl(kw.line, kw.column, name_tok.value, variants)

    def _parse_field(self) -> FieldDecl:
        typ = self._parse_type()
        name = self._expect_ident()
        array_size: Optional[Expr] = None
        unsized = False
        if self._match("KOSE_AC"):
            if self._check("KOSE_KAPA"):
                unsized = True
            else:
                array_size = self._parse_expression()
            self._expect("KOSE_KAPA", "]")
        if self._match("ATAMA"):
            raise UppError(name.line, "Sınıf alanları başlatıcı alamaz. Değer atamasını 'olustur' metodunda yapın.", name.column)
        self._expect("NOKTALI_VIRGUL", ";")
        return FieldDecl(name.line, name.column, typ, name.value, array_size, unsized)

    def _parse_function(self, class_name: Optional[str] = None) -> FunctionDecl:
        kw = self._expect_kw("fonk")
        name_tok = self._expect_ident()
        if name_tok.value == "upp" and class_name is None:
            raise UppError(name_tok.line, "'upp' yerleşik ad alanıdır; fonksiyon adı olamaz.", name_tok.column)
        self._expect("PARANTEZ_AC", "(")
        params: List[Param] = []
        if not self._check("PARANTEZ_KAPA"):
            params.append(self._parse_param())
            while self._match("VIRGUL"):
                params.append(self._parse_param())
        self._expect("PARANTEZ_KAPA", ")")
        seen_def = False
        for p in params:
            if p.default is not None:
                seen_def = True
            elif seen_def:
                raise UppError(p.line, "Varsayılan parametreler sonda art arda olmalıdır.", p.column)
        ret = TypeNode(kw.line, kw.column, "bos", 0)
        return_explicit = False
        if self._match("OK"):
            ret = self._parse_type()
            return_explicit = True
        is_arrow = False
        if self._match("YAG_OK"):
            expr = self._parse_expression()
            self._expect("NOKTALI_VIRGUL", ";")
            body = Block(expr.line, expr.column, [ReturnStmt(expr.line, expr.column, expr)])
            is_arrow = True
        else:
            body = self._parse_block()
        return FunctionDecl(
            kw.line, kw.column, name_tok.value, params, ret, body, class_name, is_arrow, return_explicit
        )

    def _parse_param(self) -> Param:
        if self._is_type_start():
            typ = self._parse_type()
            name = self._expect_ident()
            default = None
            if self._match("ATAMA"):
                default = self._parse_expression()
            return Param(name.line, name.column, typ, name.value, default)
        name = self._expect_ident()
        default = None
        if self._match("ATAMA"):
            default = self._parse_expression()
        return Param(name.line, name.column, None, name.value, default)

    def _parse_type(self) -> TypeNode:
        tok = self._peek()
        if tok.type == "ANAHTAR" and tok.value in TYPE_NAMES:
            self._advance()
            name = tok.value
            if name == "liste":
                name = self._parse_liste_tur(tok)
            elif name == "harita":
                name = self._parse_harita_tur(tok)
        elif tok.type == "KIMLIK":
            self._advance()
            name = tok.value
            if self._match("NOKTA"):
                part = self._expect_ident()
                name = f"{name}.{part.value}"
        else:
            got = tok.value if tok.value else tok.type
            raise UppError(
                tok.line,
                f"Tür bekleniyordu ('sayi', 'ondalik', 'metin', 'mantik', 'bos', 'liste[T]', 'harita[K,V]' veya sınıf adı), '{got}' bulundu.",
                tok.column,
            )
        depth = 0
        while self._match("YILDIZ"):
            depth += 1
        return TypeNode(tok.line, tok.column, name, depth)

    def _parse_koleksiyon_elem(self, ne: str) -> str:
        tok = self._peek()
        if tok.type == "ANAHTAR" and tok.value in KOLEKSIYON_ELEMAN:
            self._advance()
            return tok.value
        got = tok.value if tok.value else tok.type
        raise UppError(
            tok.line,
            f"'{ne}' eleman türü 'sayi', 'ondalik', 'mantik' veya 'metin' olmalı, '{got}' bulundu.",
            tok.column,
        )

    def _parse_liste_tur(self, kw: Token) -> str:
        self._expect("KOSE_AC", "[")
        elem = self._parse_koleksiyon_elem("liste")
        self._expect("KOSE_KAPA", "]")
        return f"liste[{elem}]"

    def _parse_harita_tur(self, kw: Token) -> str:
        self._expect("KOSE_AC", "[")
        key = self._parse_koleksiyon_elem("harita")
        self._expect("VIRGUL", ",")
        val = self._parse_koleksiyon_elem("harita")
        self._expect("KOSE_KAPA", "]")
        if key not in HARITA_ANAHTAR:
            raise UppError(
                kw.line,
                f"harita anahtarı 'metin' veya 'sayi' olmalı, '{key}' bulundu.",
                kw.column,
            )
        return f"harita[{key},{val}]"

    def _parse_oto_decl(self) -> VarDecl:
        kw = self._expect_kw("oto")
        name = self._expect_ident()
        if not self._match("ATAMA"):
            raise UppError(name.line, f"'oto {name.value}' bir başlatıcı gerektirir: 'oto {name.value} = ...;'.", name.column)
        init = self._parse_initializer()
        self._expect("NOKTALI_VIRGUL", ";")
        dummy = TypeNode(kw.line, kw.column, "", 0)
        return VarDecl(name.line, name.column, dummy, name.value, None, False, init, True)

    def _parse_var_decl(self) -> VarDecl:
        typ = self._parse_type()
        name = self._expect_ident()
        array_size: Optional[Expr] = None
        unsized = False
        if self._match("KOSE_AC"):
            if self._check("KOSE_KAPA"):
                unsized = True
            else:
                array_size = self._parse_expression()
            self._expect("KOSE_KAPA", "]")
        init: Optional[Expr] = None
        if self._match("ATAMA"):
            init = self._parse_initializer()
        if unsized and init is None:
            raise UppError(name.line, f"Boyutsuz dizi '{name.value}' bir başlatıcı gerektirir: '{name.value}[] = [..]'.", name.column)
        self._expect("NOKTALI_VIRGUL", ";")
        return VarDecl(name.line, name.column, typ, name.value, array_size, unsized, init, False)

    def _parse_initializer(self) -> Expr:
        if self._check("KOSE_AC"):
            return self._parse_array_literal()
        return self._parse_expression()

    def _parse_array_literal(self) -> ArrayLiteral:
        tok = self._expect("KOSE_AC", "[")
        elements: List[Expr] = []
        if not self._check("KOSE_KAPA"):
            elements.append(self._parse_expression())
            while self._match("VIRGUL"):
                if self._check("KOSE_KAPA"):
                    break
                elements.append(self._parse_expression())
        self._expect("KOSE_KAPA", "]")
        return ArrayLiteral(tok.line, tok.column, elements)

    def _parse_block(self) -> Block:
        tok = self._expect("SLUS_AC", "{")
        stmts: List[Stmt] = []
        while not self._check("SLUS_KAPA") and not self._check("EOF"):
            stmts.append(self._parse_statement())
        self._expect("SLUS_KAPA", "}")
        return Block(tok.line, tok.column, stmts)

    def _parse_statement(self) -> Stmt:
        if self._check_kw("c_ekle"):
            tok = self._peek()
            raise UppError(
                tok.line,
                "'c_ekle' yalnızca dosya başında (üst düzeyde) kullanılır. Fonksiyon içine #include koymayın.",
                tok.column,
            )
        if self._check_kw("kullan"):
            tok = self._peek()
            raise UppError(
                tok.line,
                "'kullan' yalnızca dosya başında (üst düzeyde) kullanılır.",
                tok.column,
            )
        if self._check_kw("sabit"):
            return self._parse_const_decl()
        if self._check_kw("eger"):
            return self._parse_if()
        if self._check_kw("secim"):
            return self._parse_switch()
        if self._check_kw("her"):
            return self._parse_foreach()
        if self._check_kw("iken"):
            return self._parse_while()
        if self._check_kw("dongu"):
            return self._parse_for()
        if self._check_kw("don"):
            return self._parse_return()
        if self._check_kw("dur"):
            return self._parse_break()
        if self._check_kw("devam"):
            return self._parse_continue()
        if self._check_kw("guvensiz"):
            return self._parse_unsafe()
        if self._check_kw("arkaplan"):
            return self._parse_background()
        if self._check_kw("guvenli"):
            return self._parse_safe()
        if self._check_kw("oto"):
            return self._parse_oto_decl()
        if self._check("SLUS_AC"):
            return self._parse_block()
        if self._check("C_GOMMU"):
            tok = self._advance()
            return CEmbed(tok.line, tok.column, tok.value)
        if self._check("ASM_GOMMU"):
            tok = self._advance()
            return AsmEmbed(tok.line, tok.column, tok.value)
        if self._is_type_start():
            return self._parse_var_decl()
        if self._match("NOKTALI_VIRGUL"):
            tok = self.tokens[self.index - 1]
            return EmptyStmt(tok.line, tok.column)
        return self._parse_expr_stmt()

    def _parse_if(self) -> IfStmt:
        kw = self._expect_kw("eger")
        self._expect("PARANTEZ_AC", "(")
        cond = self._parse_expression()
        self._expect("PARANTEZ_KAPA", ")")
        then_branch = self._parse_statement()
        else_branch: Optional[Stmt] = None
        if self._match_kw("yoksa"):
            else_branch = self._parse_statement()
        return IfStmt(kw.line, kw.column, cond, then_branch, else_branch)

    def _parse_while(self) -> WhileStmt:
        kw = self._expect_kw("iken")
        self._expect("PARANTEZ_AC", "(")
        cond = self._parse_expression()
        self._expect("PARANTEZ_KAPA", ")")
        self.loop_depth += 1
        body = self._parse_statement()
        self.loop_depth -= 1
        return WhileStmt(kw.line, kw.column, cond, body)

    def _parse_for(self) -> ForStmt:
        kw = self._expect_kw("dongu")
        self._expect("PARANTEZ_AC", "(")
        init: Optional[Union[VarDecl, ExprStmt]] = None
        if self._match("NOKTALI_VIRGUL"):
            init = None
        elif self._check_kw("oto"):
            init = self._parse_oto_decl()
        elif self._is_type_start():
            init = self._parse_var_decl()
        else:
            expr = self._parse_expression()
            self._expect("NOKTALI_VIRGUL", ";")
            init = ExprStmt(expr.line, expr.column, expr)
        cond: Optional[Expr] = None
        if not self._check("NOKTALI_VIRGUL"):
            cond = self._parse_expression()
        self._expect("NOKTALI_VIRGUL", ";")
        step: Optional[Expr] = None
        if not self._check("PARANTEZ_KAPA"):
            step = self._parse_expression()
        self._expect("PARANTEZ_KAPA", ")")
        self.loop_depth += 1
        body = self._parse_statement()
        self.loop_depth -= 1
        return ForStmt(kw.line, kw.column, init, cond, step, body)

    def _parse_switch(self) -> SwitchStmt:
        kw = self._expect_kw("secim")
        self._expect("PARANTEZ_AC", "(")
        expr = self._parse_expression()
        self._expect("PARANTEZ_KAPA", ")")
        self._expect("SLUS_AC", "{")
        cases: List[SwitchCase] = []
        saw_default = False
        self.switch_depth += 1
        while not self._check("SLUS_KAPA") and not self._check("EOF"):
            if self._check_kw("durum"):
                et = self._advance()
                value = self._parse_expression()
                self._expect_colon(et)
                cases.append(SwitchCase(et.line, et.column, value, self._parse_switch_arm()))
            elif self._check_kw("varsayilan"):
                et = self._advance()
                if saw_default:
                    raise UppError(et.line, "'varsayilan' bir 'secim' içinde bir kez yazılabilir.", et.column)
                saw_default = True
                self._expect_colon(et)
                cases.append(SwitchCase(et.line, et.column, None, self._parse_switch_arm()))
            else:
                tok = self._peek()
                got = tok.value if tok.value else tok.type
                raise UppError(tok.line, f"'durum' veya 'varsayilan' bekleniyordu, '{got}' bulundu.", tok.column)
        self.switch_depth -= 1
        self._expect("SLUS_KAPA", "}")
        return SwitchStmt(kw.line, kw.column, expr, cases)

    def _expect_colon(self, near: Token) -> None:
        tok = self._peek()
        if tok.type == "IKI_NOKTA":
            self._advance()
            return
        got = tok.value if tok.value else tok.type
        raise UppError(near.line, f"':' bekleniyordu, '{got}' bulundu.", tok.column)

    def _parse_switch_arm(self) -> List[Stmt]:
        stmts: List[Stmt] = []
        while (
            not self._check("SLUS_KAPA")
            and not self._check("EOF")
            and not self._check_kw("durum")
            and not self._check_kw("varsayilan")
        ):
            stmts.append(self._parse_statement())
        return stmts

    def _parse_foreach(self) -> ForEachStmt:
        kw = self._expect_kw("her")
        self._expect("PARANTEZ_AC", "(")
        inferred = False
        if self._check_kw("oto"):
            self._advance()
            inferred = True
            typ = TypeNode(kw.line, kw.column, "", 0)
            name = self._expect_ident()
        else:
            typ = self._parse_type()
            name = self._expect_ident()
        self._expect_kw("in")
        iterable = self._parse_expression()
        self._expect("PARANTEZ_KAPA", ")")
        self.loop_depth += 1
        body = self._parse_statement()
        self.loop_depth -= 1
        return ForEachStmt(kw.line, kw.column, name.value, typ, iterable, body, inferred)

    def _parse_return(self) -> ReturnStmt:
        kw = self._expect_kw("don")
        value: Optional[Expr] = None
        if not self._check("NOKTALI_VIRGUL"):
            value = self._parse_expression()
        self._expect("NOKTALI_VIRGUL", ";")
        return ReturnStmt(kw.line, kw.column, value)

    def _parse_break(self) -> BreakStmt:
        kw = self._expect_kw("dur")
        if self.loop_depth <= 0 and self.switch_depth <= 0:
            raise UppError(
                kw.line,
                "'dur' yalnızca 'iken', 'dongu', 'her' veya 'secim' içinde kullanılabilir.",
                kw.column,
            )
        self._expect("NOKTALI_VIRGUL", ";")
        return BreakStmt(kw.line, kw.column)

    def _parse_continue(self) -> ContinueStmt:
        kw = self._expect_kw("devam")
        if self.loop_depth <= 0:
            raise UppError(kw.line, "'devam' yalnızca 'iken', 'dongu' veya 'her' içinde kullanılabilir.", kw.column)
        self._expect("NOKTALI_VIRGUL", ";")
        return ContinueStmt(kw.line, kw.column)

    def _parse_unsafe(self) -> UnsafeBlock:
        kw = self._expect_kw("guvensiz")
        body = self._parse_block()
        return UnsafeBlock(kw.line, kw.column, body)

    def _parse_safe(self) -> SafeBlock:
        kw = self._expect_kw("guvenli")
        body = self._parse_block()
        return SafeBlock(kw.line, kw.column, body)

    def _parse_background(self) -> BackgroundBlock:
        kw = self._expect_kw("arkaplan")
        saved_loop = self.loop_depth
        saved_sw = self.switch_depth
        self.loop_depth = 0
        self.switch_depth = 0
        body = self._parse_block()
        self.loop_depth = saved_loop
        self.switch_depth = saved_sw
        return BackgroundBlock(kw.line, kw.column, body)

    def _parse_background_expr(self) -> BackgroundExpr:
        kw = self._expect_kw("arkaplan")
        saved_loop = self.loop_depth
        saved_sw = self.switch_depth
        self.loop_depth = 0
        self.switch_depth = 0
        body = self._parse_block()
        self.loop_depth = saved_loop
        self.switch_depth = saved_sw
        return BackgroundExpr(kw.line, kw.column, body)

    def _parse_expr_stmt(self) -> ExprStmt:
        expr = self._parse_expression()
        self._expect("NOKTALI_VIRGUL", ";")
        return ExprStmt(expr.line, expr.column, expr)

    def _parse_expression(self) -> Expr:
        return self._parse_assignment()

    def _parse_assignment(self) -> Expr:
        expr = self._parse_ternary()
        tok = self._peek()
        if tok.type in _ASSIGN_OPS:
            op = _ASSIGN_OPS[tok.type]
            self._advance()
            value = self._parse_assignment()
            return Assign(expr.line, expr.column, expr, value, op)
        return expr

    def _parse_ternary(self) -> Expr:
        expr = self._parse_or()
        if self._match("SORU"):
            then_expr = self._parse_assignment()
            self._expect("IKI_NOKTA", ":")
            else_expr = self._parse_ternary()
            return TernaryOp(expr.line, expr.column, expr, then_expr, else_expr)
        return expr

    def _parse_or(self) -> Expr:
        left = self._parse_and()
        while True:
            op = self._match("VEYA")
            if not op:
                break
            right = self._parse_and()
            left = BinaryOp(op.line, op.column, "||", left, right)
        return left

    def _parse_and(self) -> Expr:
        left = self._parse_bit_or()
        while True:
            op = self._match("VE")
            if not op:
                break
            right = self._parse_bit_or()
            left = BinaryOp(op.line, op.column, "&&", left, right)
        return left

    def _parse_bit_or(self) -> Expr:
        left = self._parse_xor()
        while True:
            op = self._match("VAY_ISARET")
            if not op:
                break
            right = self._parse_xor()
            left = BinaryOp(op.line, op.column, "|", left, right)
        return left

    def _parse_xor(self) -> Expr:
        left = self._parse_bit_and()
        while True:
            op = self._match("XOR")
            if not op:
                break
            right = self._parse_bit_and()
            left = BinaryOp(op.line, op.column, "^", left, right)
        return left

    def _parse_bit_and(self) -> Expr:
        left = self._parse_equality()
        while True:
            op = self._match("VE_ISARET")
            if not op:
                break
            right = self._parse_equality()
            left = BinaryOp(op.line, op.column, "&", left, right)
        return left

    def _parse_equality(self) -> Expr:
        left = self._parse_comparison()
        while True:
            if self._match("ESIT_ESIT"):
                op = self.tokens[self.index - 1]
                right = self._parse_comparison()
                left = BinaryOp(op.line, op.column, "==", left, right)
            elif self._match("ESIT_DEGIL"):
                op = self.tokens[self.index - 1]
                right = self._parse_comparison()
                left = BinaryOp(op.line, op.column, "!=", left, right)
            else:
                break
        return left

    def _parse_comparison(self) -> Expr:
        left = self._parse_term()
        mapping = {
            "KUCUK": "<",
            "BUYUK": ">",
            "KUCUK_ESIT": "<=",
            "BUYUK_ESIT": ">=",
        }
        while self._peek().type in mapping:
            op = self._advance()
            right = self._parse_term()
            left = BinaryOp(op.line, op.column, mapping[op.type], left, right)
        return left

    def _parse_term(self) -> Expr:
        left = self._parse_factor()
        while True:
            if self._match("ARTI"):
                op = self.tokens[self.index - 1]
                right = self._parse_factor()
                left = BinaryOp(op.line, op.column, "+", left, right)
            elif self._match("EKSI"):
                op = self.tokens[self.index - 1]
                right = self._parse_factor()
                left = BinaryOp(op.line, op.column, "-", left, right)
            else:
                break
        return left

    def _parse_factor(self) -> Expr:
        left = self._parse_unary()
        while True:
            if self._match("YILDIZ"):
                op = self.tokens[self.index - 1]
                right = self._parse_unary()
                left = BinaryOp(op.line, op.column, "*", left, right)
            elif self._match("BOLU"):
                op = self.tokens[self.index - 1]
                right = self._parse_unary()
                left = BinaryOp(op.line, op.column, "/", left, right)
            elif self._match("MOD"):
                op = self.tokens[self.index - 1]
                right = self._parse_unary()
                left = BinaryOp(op.line, op.column, "%", left, right)
            else:
                break
        return left

    def _parse_unary(self) -> Expr:
        tok = self._peek()
        if tok.type in ("UNLEM", "EKSI", "ARTI", "YILDIZ", "VE_ISARET"):
            self._advance()
            operand = self._parse_unary()
            op = {"UNLEM": "!", "EKSI": "-", "ARTI": "+", "YILDIZ": "*", "VE_ISARET": "&"}[tok.type]
            return UnaryOp(tok.line, tok.column, op, operand)
        return self._parse_postfix()

    def _parse_postfix(self) -> Expr:
        expr = self._parse_primary()
        while True:
            if self._match("PARANTEZ_AC"):
                args: List[Expr] = []
                if not self._check("PARANTEZ_KAPA"):
                    args.append(self._parse_expression())
                    while self._match("VIRGUL"):
                        args.append(self._parse_expression())
                self._expect("PARANTEZ_KAPA", ")")
                expr = Call(expr.line, expr.column, expr, args)
            elif self._match("KOSE_AC"):
                index = self._parse_expression()
                self._expect("KOSE_KAPA", "]")
                expr = Index(expr.line, expr.column, expr, index)
            elif self._match("NOKTA"):
                name = self._expect_ident(allow_type_kw=True)
                expr = Member(expr.line, expr.column, expr, name.value)
            else:
                break
        return expr

    def _parse_primary(self) -> Expr:
        tok = self._peek()
        if tok.type == "SAYI":
            self._advance()
            return Literal(tok.line, tok.column, int(tok.value), "sayi")
        if tok.type == "ONDALIK":
            self._advance()
            return Literal(tok.line, tok.column, tok.value, "ondalik")
        if tok.type == "METIN":
            self._advance()
            return Literal(tok.line, tok.column, tok.value, "metin")
        if tok.type == "ANAHTAR":
            if tok.value == "dogru":
                self._advance()
                return Literal(tok.line, tok.column, True, "mantik")
            if tok.value == "yanlis":
                self._advance()
                return Literal(tok.line, tok.column, False, "mantik")
            if tok.value == "yok":
                self._advance()
                return Literal(tok.line, tok.column, None, "yok")
            if tok.value == "kendi":
                self._advance()
                return Ident(tok.line, tok.column, "kendi")
            if tok.value == "arkaplan":
                return self._parse_background_expr()
            raise UppError(tok.line, f"'{tok.value}' burada ifade olarak kullanılamaz.", tok.column)
        if tok.type == "KIMLIK":
            self._advance()
            return Ident(tok.line, tok.column, tok.value)
        if self._check("KOSE_AC"):
            return self._parse_array_literal()
        if self._match("PARANTEZ_AC"):
            expr = self._parse_expression()
            self._expect("PARANTEZ_KAPA", ")")
            return expr
        got = tok.value if tok.value else tok.type
        raise UppError(tok.line, f"İfade bekleniyordu, '{got}' bulundu.", tok.column)


# ---------------------------------------------------------------------------
# Yardımcılar: dizge, interpolasyon, eski API
# ---------------------------------------------------------------------------

REMOVED_APIS = {
    "yazdir": "upp.yaz / upp.satir_yaz",
    "mesaj_kutusu": "upp.mesaj",
    "hata_kutusu": "upp.hata",
    "ses_cal": "upp.ses_cal",
    "dosya_oku": "upp.dosya_oku",
    "dosya_yaz": "upp.dosya_yaz",
}

UPP_RETURNS = {
    "yaz": "bos",
    "satir_yaz": "bos",
    "mesaj": "bos",
    "hata": "bos",
    "ses_cal": "sayi",
    "uyut": "bos",
    "rastgele": "sayi",
    "dosya_oku": "metin",
    "dosya_yaz": "sayi",
    "uzunluk": "sayi",
    "giris": "metin",
    "min": "sayi",
    "max": "sayi",
    "mutlak": "sayi",
    "metin_bosalt": "bos",
    "bekle": "bos",
    "arguman_sayisi": "sayi",
    "arguman": "metin",
    "dosya_var_mi": "mantik",
    "dosya_sil": "mantik",
    "zaman": "sayi",
    "metinden_sayiya": "sayi",
    "sayidan_metin": "metin",
    "ondalikdan_metin": "metin",
    "metinden_ondalik": "ondalik",
    "dosya_ekle": "sayi",
    "ortam_al": "metin",
    "hata_yaz": "bos",
    "hata_satir_yaz": "bos",
}

UPP_C_NAMES = {
    "mesaj": "upp_mesaj",
    "hata": "upp_hata",
    "ses_cal": "upp_ses_cal",
    "uyut": "upp_uyut",
    "rastgele": "upp_rastgele",
    "dosya_oku": "upp_dosya_oku",
    "dosya_yaz": "upp_dosya_yaz",
    "uzunluk": "upp_uzunluk",
    "giris": "upp_giris",
    "min": "upp_min",
    "max": "upp_max",
    "mutlak": "upp_mutlak",
    "metin_bosalt": "upp_metin_bosalt",
    "bekle": "upp_arkaplan_bekle_deger",
    "arguman_sayisi": "upp_arguman_sayisi",
    "arguman": "upp_arguman",
    "dosya_var_mi": "upp_dosya_var_mi",
    "dosya_sil": "upp_dosya_sil",
    "zaman": "upp_zaman",
    "metinden_sayiya": "upp_metinden_sayiya",
    "sayidan_metin": "upp_sayidan_metin",
    "ondalikdan_metin": "upp_ondalikdan_metin",
    "metinden_ondalik": "upp_metinden_ondalik",
    "dosya_ekle": "upp_dosya_ekle",
    "ortam_al": "upp_ortam_al",
    "hata_yaz": "upp_hata_yaz",
    "hata_satir_yaz": "upp_hata_satir_yaz",
}

# (min_arg, max_arg); max_arg None = sınırsız
UPP_ARITY: Dict[str, Tuple[int, Optional[int]]] = {
    "yaz": (1, None),
    "satir_yaz": (0, None),
    "mesaj": (2, 2),
    "hata": (2, 2),
    "ses_cal": (1, 1),
    "uyut": (1, 1),
    "rastgele": (2, 2),
    "dosya_oku": (1, 1),
    "dosya_yaz": (2, 2),
    "uzunluk": (1, 1),
    "giris": (0, 0),
    "min": (2, 2),
    "max": (2, 2),
    "mutlak": (1, 1),
    "metin_bosalt": (1, 1),
    "bekle": (1, 1),
    "arguman_sayisi": (0, 0),
    "arguman": (1, 1),
    "dosya_var_mi": (1, 1),
    "dosya_sil": (1, 1),
    "zaman": (0, 0),
    "metinden_sayiya": (1, 1),
    "sayidan_metin": (1, 1),
    "ondalikdan_metin": (1, 1),
    "metinden_ondalik": (1, 1),
    "dosya_ekle": (2, 2),
    "ortam_al": (1, 1),
    "hata_yaz": (1, 1),
    "hata_satir_yaz": (0, 1),
}

INTERP_FUNCS = {"yaz", "satir_yaz", "mesaj", "hata", "hata_yaz", "hata_satir_yaz"}

# ns -> { adı: (dönüş_türü, (min,max), C_adı) }
UPP_NAMESPACES: Dict[str, Dict[str, Tuple[str, Tuple[int, Optional[int]], str]]] = {
    "girdi": {
        "fare_tasi": ("bos", (2, 2), "upp_girdi_fare_tasi"),
        "fare_tikla": ("bos", (0, 0), "upp_girdi_fare_tikla"),
        "tus_bas": ("bos", (1, 1), "upp_girdi_tus_bas"),
    },
    "bellek": {
        "baglan": ("sayi", (1, 1), "upp_bellek_baglan"),
        "modul_bul": ("sayi", (1, 1), "upp_bellek_modul_bul"),
        "sayioku": ("sayi", (1, 1), "upp_bellek_sayioku"),
        "oku_ondalik": ("ondalik", (1, 1), "upp_bellek_oku_ondalik"),
        "oku_metin": ("metin", (2, 2), "upp_bellek_oku_metin"),
        "sayiyaz": ("sayi", (2, 2), "upp_bellek_sayiyaz"),
        "yaz_ondalik": ("sayi", (2, 2), "upp_bellek_yaz_ondalik"),
        "zincir_oku": ("sayi", (2, 2), "upp_bellek_zincir_oku"),
        "yama_yap": ("sayi", (2, 2), "upp_bellek_yama_yap"),
        "koruma_degistir": ("sayi", (3, 3), "upp_bellek_koruma_degistir"),
        "son_hata": ("sayi", (0, 0), "upp_bellek_son_hata"),
        "okundu": ("sayi", (0, 0), "upp_bellek_okundu"),
    },
    "cizim": {
        "baslat": ("sayi", (1, 1), "upp_cizim_baslat"),
        "kutu": ("bos", (5, 5), "upp_cizim_kutu"),
        "yazi": ("bos", (4, 4), "upp_cizim_yazi"),
        "temizle": ("bos", (0, 0), "upp_cizim_temizle"),
    },
    "matematik": {
        "mutlak": ("ondalik", (1, 1), "upp_mat_mutlak"),
        "karekok": ("ondalik", (1, 1), "upp_mat_karekok"),
        "us": ("ondalik", (2, 2), "upp_mat_us"),
        "sin": ("ondalik", (1, 1), "upp_mat_sin"),
        "cos": ("ondalik", (1, 1), "upp_mat_cos"),
        "sinirla": ("ondalik", (3, 3), "upp_mat_sinirla"),
        "mesafe_3d": ("ondalik", (6, 6), "upp_mat_mesafe_3d"),
        "mesafe": ("ondalik", (2, 2), "upp_mat_mesafe_v3"),
        "aci_hesapla": ("Yonelme", (6, 6), "upp_mat_aci_hesapla"),
        "dunya_ekran": ("EkranNokta", (6, 6), "upp_mat_dunya_ekran"),
        "yumusat": ("ondalik", (3, 3), "upp_mat_yumusat"),
        "tan": ("ondalik", (1, 1), "upp_mat_tan"),
        "atan2": ("ondalik", (2, 2), "upp_mat_atan2"),
        "taban": ("ondalik", (1, 1), "upp_mat_taban"),
        "tavan": ("ondalik", (1, 1), "upp_mat_tavan"),
        "yuvarla": ("ondalik", (1, 1), "upp_mat_yuvarla"),
        "pi": ("ondalik", (0, 0), "upp_mat_pi"),
    },
    "metin": {
        "kes": ("metin", (3, 3), "upp_metin_kes"),
        "icinde": ("sayi", (2, 2), "upp_metin_icinde"),
        "degistir": ("metin", (3, 3), "upp_metin_degistir"),
        "baslar": ("mantik", (2, 2), "upp_metin_baslar"),
        "biter": ("mantik", (2, 2), "upp_metin_biter"),
        "kirp": ("metin", (1, 1), "upp_metin_kirp"),
        "buyuk": ("metin", (1, 1), "upp_metin_buyuk"),
        "kucuk": ("metin", (1, 1), "upp_metin_kucuk"),
        "kod": ("sayi", (2, 2), "upp_metin_kod"),
        "ileri": ("sayi", (2, 2), "upp_metin_ileri"),
        "bol": ("liste[metin]", (2, 2), "upp_metin_bol"),
        "birlestir": ("metin", (2, 2), "upp_metin_liste_birlestir"),
    },
    "json": {
        "ayristir": ("JSONDeger", (1, 1), "upp_json_ayristir"),
        "olustur": ("metin", (1, 1), "upp_json_olustur"),
        "tur": ("metin", (1, 1), "upp_json_tur"),
        "anahtarlar": ("JSONDeger", (1, 1), "upp_json_anahtarlar"),
        "anahtar_al": ("JSONDeger", (2, 2), "upp_json_anahtar_al"),
        "dizi_eleman": ("JSONDeger", (2, 2), "upp_json_dizi_eleman"),
        "metin_al": ("metin", (1, 1), "upp_json_metin_al"),
        "sayi_al": ("sayi", (1, 1), "upp_json_sayi_al"),
        "ondalik_al": ("ondalik", (1, 1), "upp_json_ondalik_al"),
        "mantik_al": ("mantik", (1, 1), "upp_json_mantik_al"),
        "uzunluk": ("sayi", (1, 1), "upp_json_uzunluk"),
        "var_mi": ("mantik", (1, 1), "upp_json_var_mi"),
        "nesne": ("JSONDeger", (0, 0), "upp_json_nesne"),
        "dizi": ("JSONDeger", (0, 0), "upp_json_dizi"),
        "metin_yap": ("JSONDeger", (1, 1), "upp_json_metin_yap"),
        "sayi_yap": ("JSONDeger", (1, 1), "upp_json_sayi_yap"),
        "ondalik_yap": ("JSONDeger", (1, 1), "upp_json_ondalik_yap"),
        "mantik_yap": ("JSONDeger", (1, 1), "upp_json_mantik_yap"),
        "yok_yap": ("JSONDeger", (0, 0), "upp_json_yok_yap"),
        "koy": ("bos", (3, 3), "upp_json_koy"),
        "ekle": ("bos", (2, 2), "upp_json_ekle"),
        "bosalt": ("bos", (1, 1), "upp_json_bosalt"),
    },
    "stdio": {
        "gelen_oku": ("metin", (0, 0), "upp_stdio_gelen_oku"),
        "yaz": ("bos", (1, 1), "upp_stdio_yaz"),
    },
    "sistem": {
        "calistir": ("SurecCikti", (1, 1), "upp_sistem_calistir"),
        "komut_bul": ("metin", (1, 1), "upp_sistem_komut_bul"),
        "exe_dizin": ("metin", (0, 0), "upp_sistem_exe_dizin"),
    },
    "yol": {
        "dizin": ("metin", (1, 1), "upp_yol_dizin"),
        "birlestir": ("metin", (2, 2), "upp_yol_birlestir"),
        "gercek": ("metin", (1, 1), "upp_yol_gercek"),
        "calisma": ("metin", (0, 0), "upp_yol_calisma"),
        "mutlak_mi": ("mantik", (1, 1), "upp_yol_mutlak_mi"),
        "klasor_olustur": ("mantik", (1, 1), "upp_yol_klasor_olustur"),
    },
    "dosya": {
        "listele": ("liste[metin]", (1, 1), "upp_dosya_listele"),
    },
}

BUILTIN_CLASS_NAMES = {"Yonelme", "EkranNokta", "Vektor3", "ArkaplanIs", "JSONDeger", "SurecCikti", "Kilit"}
UNSAFE_UPP_NS = {"bellek"}

KNOWN_TUS_NAMES = {
    "ENTER", "RETURN", "ESC", "ESCAPE", "SPACE", "BOSLUK", "TAB", "SHIFT",
    "CTRL", "CONTROL", "ALT", "BACKSPACE", "BACK", "DELETE", "DEL",
    "INSERT", "INS", "HOME", "END", "LEFT", "SOL", "RIGHT", "SAG",
    "UP", "YUKARI", "DOWN", "ASAGI", "PAGEUP", "PGUP", "PAGEDOWN", "PGDN",
}


def known_tus_name(raw: str) -> bool:
    s = raw.strip().upper()
    if not s:
        return False
    if re.fullmatch(r"F([1-9]|1[0-9]|2[0-4])", s):
        return True
    if len(s) == 1 and (("A" <= s <= "Z") or ("0" <= s <= "9")):
        return True
    return s in KNOWN_TUS_NAMES


def make_builtin_classes() -> Dict[str, ClassDecl]:
    def f(typ: str, name: str) -> FieldDecl:
        return FieldDecl(0, 1, TypeNode(0, 1, typ, 0, 0), name)

    return {
        "Yonelme": ClassDecl(
            0, 1, "Yonelme",
            [f("ondalik", "pitch"), f("ondalik", "yaw")],
            [],
        ),
        "EkranNokta": ClassDecl(
            0, 1, "EkranNokta",
            [f("ondalik", "x"), f("ondalik", "y"), f("sayi", "gorunur")],
            [],
        ),
        "Vektor3": ClassDecl(
            0, 1, "Vektor3",
            [f("ondalik", "x"), f("ondalik", "y"), f("ondalik", "z")],
            [],
        ),
        "ArkaplanIs": ClassDecl(
            0, 1, "ArkaplanIs",
            [f("sayi", "handle")],
            [],
        ),
        "JSONDeger": ClassDecl(
            0, 1, "JSONDeger",
            [f("sayi", "id")],
            [],
        ),
        "SurecCikti": ClassDecl(
            0, 1, "SurecCikti",
            [f("metin", "cikti"), f("sayi", "kod")],
            [],
        ),
        "Kilit": ClassDecl(
            0, 1, "Kilit",
            [f("sayi", "id")],
            [],
        ),
    }


def c_symbol(name: str, alias: str = "") -> str:
    if alias:
        return f"{alias}_{name}"
    return name


def type_c_name(name: str) -> str:
    return name.replace(".", "_")


def upp_member_path(expr: Optional[Expr]) -> Optional[List[str]]:
    parts: List[str] = []
    cur: Optional[Expr] = expr
    while isinstance(cur, Member):
        parts.append(cur.name)
        cur = cur.obj
    if isinstance(cur, Ident) and cur.name == "upp":
        parts.reverse()
        return parts
    return None


def upp_ns_type(obj_t: TypeNode, name: str, line: int, column: int) -> Optional[TypeNode]:
    if obj_t.name == "upp":
        if name in UPP_NAMESPACES:
            return TypeNode(line, column, f"upp.{name}", 0, 0)
        if name in UPP_RETURNS:
            return TypeNode(line, column, UPP_RETURNS[name], 0, 0)
        return None
    if obj_t.name.startswith("upp."):
        ns = obj_t.name[4:]
        table = UPP_NAMESPACES.get(ns)
        if table and name in table:
            return TypeNode(line, column, table[name][0], 0, 0)
        return None
    return None


def decode_string_literal(raw: str) -> str:
    if len(raw) >= 6 and raw.startswith('"""') and raw.endswith('"""'):
        return raw[3:-3].replace("\r\n", "\n").replace("\r", "\n")
    if len(raw) < 2 or raw[0] != '"' or raw[-1] != '"':
        return raw
    inner = raw[1:-1]
    out = []
    i = 0
    while i < len(inner):
        if inner[i] == "\\" and i + 1 < len(inner):
            nxt = inner[i + 1]
            mapping = {"n": "\n", "t": "\t", "r": "\r", "\\": "\\", '"': '"'}
            out.append(mapping.get(nxt, nxt))
            i += 2
            continue
        out.append(inner[i])
        i += 1
    return "".join(out)


def encode_c_string(text: str) -> str:
    return '"' + (
        text.replace("\\", "\\\\")
        .replace('"', '\\"')
        .replace("\n", "\\n")
        .replace("\r", "\\r")
        .replace("\t", "\\t")
    ) + '"'


_INTERP_AD = re.compile(
    r"^[A-Za-z_çğıöşüÇĞİÖŞÜ][A-Za-z0-9_çğıöşüÇĞİÖŞÜ]*(\.[A-Za-z_çğıöşüÇĞİÖŞÜ][A-Za-z0-9_çğıöşüÇĞİÖŞÜ]*)*$"
)


def parse_interpolation(raw_literal: str, line: int, column: int) -> Optional[Tuple[str, List[Expr]]]:
    """Dönen None: süslü interpolasyon yok. Aksi halde (ham_metin_parçaları_placeholder'sız tarama için) format şablonu + ifadeler.
    Format içindeki % henüz kaçışlanmamıştır; çağıran %% yapar ve {n} yerlerine spec koyar.
    Burada şablon, parçalar listesi olarak kurulur: ('lit', str) | ('expr', Expr).
    """
    text = decode_string_literal(raw_literal)
    if "{" not in text:
        return None
    parts: List[Tuple[str, Any]] = []
    buf = []
    i = 0
    found = False
    while i < len(text):
        if text[i] == "{" and i + 1 < len(text) and text[i + 1] == "{":
            buf.append("{")
            i += 2
            continue
        if text[i] == "}" and i + 1 < len(text) and text[i + 1] == "}":
            buf.append("}")
            i += 2
            continue
        if text[i] == "{":
            end = text.find("}", i + 1)
            if end < 0:
                buf.append(text[i])
                i += 1
                continue
            name = text[i + 1:end].strip()
            if not name:
                raise UppError(line, "Boş interpolasyon '{ }' geçersiz.", column)
            if not _INTERP_AD.match(name):
                buf.append(text[i:end + 1])
                i = end + 1
                continue
            if buf:
                parts.append(("lit", "".join(buf)))
                buf = []
            ident_parts = name.split(".")
            expr: Expr = Ident(line, column, ident_parts[0])
            for part in ident_parts[1:]:
                expr = Member(line, column, expr, part)
            parts.append(("expr", expr))
            found = True
            i = end + 1
            continue
        buf.append(text[i])
        i += 1
    if buf:
        parts.append(("lit", "".join(buf)))
    if not found:
        return None
    return ("parts", parts)  # type: ignore


def flatten_interp_parts(result: Tuple) -> List[Tuple[str, Any]]:
    return result[1]


def c_include_line(spec: str) -> str:
    spec = spec.strip()
    if spec.startswith("<") and spec.endswith(">"):
        return f"#include {spec}"
    if spec.startswith('"') and spec.endswith('"'):
        return f"#include {spec}"
    return f'#include "{spec}"'


def c_embed_has_preprocessor(code: str) -> bool:
    for raw in code.splitlines():
        stripped = raw.strip()
        if stripped.startswith("#include") or stripped.startswith("#define") or stripped.startswith("#pragma") or stripped.startswith("#if"):
            return True
    return False


def check_removed_api(expr: Optional[Expr]) -> None:
    if isinstance(expr, Call) and isinstance(expr.callee, Ident) and expr.callee.name in REMOVED_APIS:
        raise UppError(
            expr.line,
            f"'{expr.callee.name}' v2.0'da kaldırıldı. Yerine {REMOVED_APIS[expr.callee.name]} kullanın.",
            expr.column,
        )


def type_desc(t: TypeNode) -> str:
    return t.name + ("*" * t.pointer_depth) + ("[]" * t.array_dims)


def type_is_yok(t: TypeNode) -> bool:
    return t.name == "yok"


def type_is_metin(t: TypeNode) -> bool:
    return t.name == "metin" and t.pointer_depth == 0


def type_is_numeric(t: TypeNode) -> bool:
    return t.name in ("sayi", "ondalik", "mantik") and t.pointer_depth == 0 and t.array_dims == 0


def type_is_class_value(t: TypeNode, classes: Dict[str, ClassDecl]) -> bool:
    return t.name in classes and t.pointer_depth == 0 and t.array_dims == 0


def type_is_void(t: TypeNode) -> bool:
    return t.name == "bos" and t.pointer_depth == 0 and t.array_dims == 0


def parse_liste_elem(name: str) -> Optional[str]:
    if not name.startswith("liste[") or not name.endswith("]"):
        return None
    inner = name[6:-1]
    if inner in KOLEKSIYON_ELEMAN:
        return inner
    return None


def parse_harita_kv(name: str) -> Optional[Tuple[str, str]]:
    if not name.startswith("harita[") or not name.endswith("]"):
        return None
    inner = name[7:-1]
    comma = inner.find(",")
    if comma < 0:
        return None
    key, val = inner[:comma], inner[comma + 1:]
    if key not in HARITA_ANAHTAR or val not in KOLEKSIYON_ELEMAN:
        return None
    return key, val


def is_liste_type_name(name: str) -> bool:
    return name.startswith("liste[") and name.endswith("]")


def is_harita_type_name(name: str) -> bool:
    return name.startswith("harita[") and name.endswith("]")


def type_is_liste(t: TypeNode) -> bool:
    return parse_liste_elem(t.name) is not None and t.pointer_depth == 0 and t.array_dims == 0


def type_is_harita(t: TypeNode) -> bool:
    return parse_harita_kv(t.name) is not None and t.pointer_depth == 0 and t.array_dims == 0


def koleksiyon_tag(elem: str) -> int:
    return {"sayi": 0, "ondalik": 1, "mantik": 2, "metin": 3}[elem]


def is_lvalue(expr: Optional[Expr]) -> bool:
    if isinstance(expr, Ident):
        return expr.name not in ("upp",) and expr.name not in TYPE_NAMES
    if isinstance(expr, (Member, Index)):
        return True
    if isinstance(expr, UnaryOp) and expr.op == "*":
        return True
    return False


def types_assignable(dst: TypeNode, src: TypeNode, classes: Dict[str, ClassDecl], enums: Optional[Dict[str, EnumDecl]] = None) -> bool:
    enums = enums or {}
    if type_is_void(src) or type_is_void(dst):
        return False
    if type_is_yok(src):
        return type_is_metin(dst) or dst.pointer_depth > 0
    if dst.pointer_depth > 0 and src.pointer_depth > 0:
        return src.name == dst.name or src.name == "bos" or dst.name == "bos"
    if dst.pointer_depth != src.pointer_depth:
        return False
    if dst.array_dims != src.array_dims:
        return False
    if dst.name == src.name:
        return True
    if dst.name in enums and src.name in (dst.name, "sayi", "mantik") and src.array_dims == 0:
        return True
    if src.name in enums and dst.name == "sayi" and dst.array_dims == 0:
        return True
    if dst.name == "ondalik" and src.name in ("sayi", "mantik"):
        return True
    if dst.name == "sayi" and src.name == "mantik":
        return True
    if dst.name == "mantik" and src.name == "sayi":
        return True
    return False


def _check_arity(name: str, n: int, line: int, column: int, spec: Tuple[int, Optional[int]]) -> None:
    lo, hi = spec
    if n < lo or (hi is not None and n > hi):
        if hi is None:
            expected = f"en az {lo}"
        elif lo == hi:
            expected = str(lo)
        else:
            expected = f"{lo}–{hi}"
        raise UppError(line, f"'{name}' {expected} argüman bekliyor, {n} verildi.", column)


# ---------------------------------------------------------------------------
# Tip çıkarımı
# ---------------------------------------------------------------------------

class TypeInferencer:
    def __init__(self) -> None:
        self.classes: Dict[str, ClassDecl] = {}
        self.functions: Dict[str, FunctionDecl] = {}
        self.enums: Dict[str, EnumDecl] = {}
        self.scopes: List[Dict[str, TypeNode]] = [{}]
        self.const_names: List[set] = [set()]
        self.globals: Dict[str, TypeNode] = {}
        self.global_consts: set = set()
        self.modules: Dict[str, Dict[str, Any]] = {}
        self.current_class: Optional[str] = None
        self.current_fn: Optional[FunctionDecl] = None
        self.background_depth = 0
        self._collect = False
        self._errors: List[UppError] = []

    def _assignable(self, dst: TypeNode, src: TypeNode) -> bool:
        return types_assignable(dst, src, self.classes, self.enums)

    def _is_numeric(self, t: TypeNode) -> bool:
        if type_is_numeric(t):
            return True
        return t.name in self.enums and t.pointer_depth == 0 and t.array_dims == 0

    def _is_switchable(self, t: TypeNode) -> bool:
        if t.pointer_depth != 0 or t.array_dims != 0:
            return False
        return t.name in ("sayi", "mantik", "metin") or t.name in self.enums

    def _enum_ns(self, name: str) -> str:
        return f"secenek.{name}"

    def _enum_from_ns(self, t: TypeNode) -> Optional[str]:
        if t.name.startswith("secenek."):
            return t.name[8:]
        return None

    def _is_countable_array(self, expr: Optional[Expr]) -> bool:
        if expr is None:
            return False
        if isinstance(expr, ArrayLiteral):
            return True
        if isinstance(expr, Ident):
            try:
                t = self._lookup(expr.name, expr)
            except UppError:
                return False
            return t.array_dims > 0
        if isinstance(expr, Member):
            t = self._expr_type(expr)
            return t.array_dims > 0
        return False

    def _is_switch_const(self, expr: Optional[Expr]) -> bool:
        if expr is None:
            return False
        if isinstance(expr, Literal) and expr.kind in ("sayi", "mantik", "metin"):
            return True
        if isinstance(expr, UnaryOp) and expr.op == "-" and self._is_switch_const(expr.operand):
            return True
        if isinstance(expr, Ident) and self._name_is_const(expr.name):
            try:
                t = self._lookup(expr.name, expr)
            except UppError:
                return False
            return type_is_metin(t) or t.name in ("sayi", "mantik") or t.name in self.enums
        if isinstance(expr, Member):
            obj_t = self._expr_type(expr.obj)
            return self._enum_from_ns(obj_t) is not None
        return False

    def run(self, program: Program, collect: bool = False) -> List[UppError]:
        self._collect = collect
        self._errors = []
        self.classes = make_builtin_classes()
        self.enums = {}
        self.modules = {}
        self.functions = {}
        try:
            self._register_decls(program)
        except UppError as err:
            if not self._collect:
                raise
            self._errors.append(err)
        self.globals = {}
        self.global_consts = set()
        self.scopes = [{}]
        self.const_names = [set()]
        for decl in program.decls:
            if isinstance(decl, VarDecl):
                try:
                    self._check_global_var(decl)
                    self._walk_stmt(decl)
                    if decl.mod_alias:
                        self._mod(decl.mod_alias)["var"][decl.name] = decl
                        self.scopes[0].pop(decl.name, None)
                        self.const_names[0].discard(decl.name)
                    else:
                        self.globals[decl.name] = self.scopes[0][decl.name]
                        if decl.is_const:
                            self.global_consts.add(decl.name)
                except UppError as err:
                    if not self._collect:
                        raise
                    self._errors.append(err)
                    if len(self._errors) >= 50:
                        return self._errors
        fns = self._all_functions(program)
        for fn in fns:
            try:
                self._fill_params(fn)
                self._check_param_defaults(fn)
            except UppError as err:
                if not self._collect:
                    raise
                self._errors.append(err)
        for _ in range(len(fns) + 1):
            for fn in fns:
                try:
                    self._infer_arrow_return(fn, finalize=False)
                except UppError:
                    if not self._collect:
                        raise
        for fn in fns:
            try:
                self._infer_arrow_return(fn, finalize=True)
            except UppError as err:
                if not self._collect:
                    raise
                self._errors.append(err)
        for fn in fns:
            try:
                self._infer_body(fn)
            except UppError as err:
                if not self._collect:
                    raise
                self._errors.append(err)
                if len(self._errors) >= 50:
                    break
        try:
            self._validate_ana(program)
        except UppError as err:
            if not self._collect:
                raise
            self._errors.append(err)
        return self._errors

    def _mod(self, alias: str) -> Dict[str, Any]:
        if alias not in self.modules:
            self.modules[alias] = {"fn": {}, "class": {}, "enum": {}, "var": {}}
        return self.modules[alias]

    def _register_decls(self, program: Program) -> None:
        aliases_seen: Dict[str, Node] = {}
        for decl in program.decls:
            alias = getattr(decl, "mod_alias", "") or ""
            if isinstance(decl, EnumDecl):
                key = f"{alias}.{decl.name}" if alias else decl.name
                if key in self.enums or key in self.classes or decl.name in TYPE_NAMES:
                    raise UppError(decl.line, f"'{key}' zaten tanımlı; seçenek olarak kullanılamaz.", decl.column)
                self.enums[key] = decl
                if alias:
                    self._mod(alias)["enum"][decl.name] = decl
            elif isinstance(decl, ClassDecl):
                key = f"{alias}.{decl.name}" if alias else decl.name
                if decl.name in BUILTIN_CLASS_NAMES and not alias:
                    raise UppError(
                        decl.line,
                        f"'{decl.name}' yerleşik bir türdür; yeniden tanımlanamaz.",
                        decl.column,
                    )
                if key in self.enums:
                    raise UppError(decl.line, f"'{key}' seçenek adı olarak tanımlı; sınıf olamaz.", decl.column)
                if key in self.classes:
                    self._cift_tanim_hata(self.classes[key], decl, key)
                self.classes[key] = decl
                if alias:
                    self._mod(alias)["class"][decl.name] = decl
                    for method in decl.methods:
                        method.class_name = key
                for method in decl.methods:
                    mkey = f"{key}_{method.name}"
                    if mkey in self.functions:
                        self._cift_tanim_hata(self.functions[mkey], method, mkey)
                    self.functions[mkey] = method
            elif isinstance(decl, FunctionDecl):
                if alias:
                    bag = self._mod(alias)["fn"]
                    if decl.name in bag:
                        self._cift_tanim_hata(bag[decl.name], decl, f"{alias}.{decl.name}")
                    bag[decl.name] = decl
                else:
                    if decl.name in self.functions:
                        self._cift_tanim_hata(self.functions[decl.name], decl, decl.name)
                    self.functions[decl.name] = decl
            elif isinstance(decl, KullanDecl) and decl.alias:
                if decl.alias in aliases_seen:
                    raise UppError(decl.line, f"'{decl.alias}' birden fazla kez 'olarak' adı olarak kullanıldı.", decl.column)
                aliases_seen[decl.alias] = decl
                self._mod(decl.alias)

    def _check_global_var(self, decl: VarDecl) -> None:
        if decl.array_size is not None or decl.unsized_array or decl.type.array_dims:
            raise UppError(decl.line, "Üst düzeyde dizi yok; 'liste[T]' kullanın.", decl.column)
        if decl.type.pointer_depth > 0:
            raise UppError(decl.line, "Üst düzeyde işaretçi değişken yok.", decl.column)
        display = f"{decl.mod_alias}.{decl.name}" if decl.mod_alias else decl.name
        if decl.mod_alias:
            bag = self._mod(decl.mod_alias)["var"]
            if decl.name in bag:
                self._cift_tanim_hata(bag[decl.name], decl, display)
        else:
            if decl.name in self.globals or (decl.name in self.scopes[0] if self.scopes else False):
                # clash against previously bound globals happens in _bind
                pass
        if not decl.is_const and decl.init is not None and not self._is_const_expr(decl.init):
            raise UppError(
                decl.line,
                f"Üst düzey '{display}' derleme sabiti veya sıfır ile başlatılmalıdır.",
                decl.column,
            )

    def _check_param_defaults(self, fn: FunctionDecl) -> None:
        names = {p.name for p in fn.params}

        def mentions_param(e: Optional[Expr]) -> bool:
            if e is None:
                return False
            if isinstance(e, Ident) and e.name in names:
                return True
            if isinstance(e, (BinaryOp,)):
                return mentions_param(e.left) or mentions_param(e.right)
            if isinstance(e, UnaryOp):
                return mentions_param(e.operand)
            if isinstance(e, TernaryOp):
                return mentions_param(e.cond) or mentions_param(e.then_expr) or mentions_param(e.else_expr)
            if isinstance(e, Call):
                return mentions_param(e.callee) or any(mentions_param(a) for a in e.args)
            if isinstance(e, Member):
                return mentions_param(e.obj)
            if isinstance(e, Index):
                return mentions_param(e.array) or mentions_param(e.index)
            return False

        for p in fn.params:
            if p.default is None:
                continue
            if mentions_param(p.default) or not self._is_const_expr(p.default):
                raise UppError(
                    p.line,
                    f"Varsayılan '{p.name}' derleme sabiti olmalı ve parametreye bakamaz.",
                    p.column,
                )

    def _all_functions(self, program: Program) -> List[FunctionDecl]:
        out: List[FunctionDecl] = []
        for decl in program.decls:
            if isinstance(decl, ClassDecl):
                out.extend(decl.methods)
            elif isinstance(decl, FunctionDecl):
                out.append(decl)
        return out

    def _cift_tanim_hata(self, eski: Node, yeni: Node, ad: str) -> None:
        extra = ""
        eski_yol = getattr(eski, "kaynak", "") or ""
        yeni_yol = getattr(yeni, "kaynak", "") or ""
        if eski_yol or yeni_yol:
            extra = f" ({eski_yol or '?'} ve {yeni_yol or '?'})"
        raise UppError(yeni.line, f"'{ad}' birden fazla kez tanımlandı.{extra}", yeni.column)

    def _fill_params(self, fn: FunctionDecl) -> None:
        for param in fn.params:
            if param.type is None or param.type.name == "":
                param.type = TypeNode(fn.line, fn.column, "sayi", 0, 0)

    def _bind_fn_scope(self, fn: FunctionDecl) -> None:
        self.scopes = [dict(self.globals), {}]
        self.const_names = [set(self.global_consts), set()]
        self.current_class = fn.class_name
        self.current_fn = fn
        if fn.class_name:
            self._bind("kendi", TypeNode(fn.line, fn.column, fn.class_name, 1, 0))
        for param in fn.params:
            assert param.type is not None
            self._bind(param.name, param.type)

    def _infer_arrow_return(self, fn: FunctionDecl, finalize: bool) -> None:
        if not fn.is_arrow or not fn.body or not fn.body.stmts:
            return
        stmt = fn.body.stmts[0]
        expr: Optional[Expr] = None
        if isinstance(stmt, ReturnStmt):
            expr = stmt.value
        elif isinstance(stmt, ExprStmt):
            expr = stmt.expr
        self._bind_fn_scope(fn)
        inferred = self._expr_type(expr)
        if not fn.return_explicit:
            fn.return_type = inferred
        if not finalize:
            return
        if fn.return_explicit and not type_is_void(fn.return_type):
            if type_is_void(inferred) or not self._assignable(fn.return_type, inferred):
                raise UppError(
                    fn.line,
                    f"Ok fonksiyonunun dönüş türü uyumsuz: beklenen '{type_desc(fn.return_type)}', bulunan '{type_desc(inferred)}'.",
                    fn.column,
                )
        if type_is_void(fn.return_type) and isinstance(stmt, ReturnStmt) and stmt.value is not None:
            fn.body.stmts[0] = ExprStmt(stmt.line, stmt.column, stmt.value)

    def _infer_body(self, fn: FunctionDecl) -> None:
        self._bind_fn_scope(fn)
        if fn.body:
            self._walk_stmt(fn.body)
            if not type_is_void(fn.return_type):
                if not self._always_returns(fn.body):
                    raise UppError(
                        fn.line,
                        f"'{fn.name}' '{type_desc(fn.return_type)}' dönmeli; bazı yollarda 'don' yok.",
                        fn.column,
                    )
        self.current_class = None
        self.current_fn = None

    def _literal_true(self, expr: Optional[Expr]) -> bool:
        return isinstance(expr, Literal) and expr.kind == "mantik" and bool(expr.value)

    def _always_returns(self, stmt: Optional[Stmt]) -> bool:
        if stmt is None:
            return False
        if isinstance(stmt, ReturnStmt):
            return True
        if isinstance(stmt, Block):
            return any(self._always_returns(s) for s in stmt.stmts)
        if isinstance(stmt, (UnsafeBlock, SafeBlock)):
            return self._always_returns(stmt.body)
        if isinstance(stmt, IfStmt):
            return self._always_returns(stmt.then_branch) and self._always_returns(stmt.else_branch)
        if isinstance(stmt, SwitchStmt):
            return self._switch_always_returns(stmt)
        if isinstance(stmt, WhileStmt):
            if self._literal_true(stmt.cond):
                return self._always_returns(stmt.body)
            return False
        if isinstance(stmt, ForStmt):
            if stmt.cond is None or self._literal_true(stmt.cond):
                return self._always_returns(stmt.body)
            return False
        return False

    def _switch_always_returns(self, stmt: SwitchStmt) -> bool:
        if not stmt.cases:
            return False
        if not any(c.value is None for c in stmt.cases):
            return False
        for i in range(len(stmt.cases)):
            if not self._case_fall_returns(stmt.cases, i):
                return False
        return True

    def _case_fall_returns(self, cases: List[SwitchCase], start: int) -> bool:
        k = start
        while k < len(cases):
            stmts = [s for s in cases[k].stmts if not isinstance(s, EmptyStmt)]
            if any(self._always_returns(s) for s in stmts):
                return True
            if stmts:
                return False
            k += 1
        return False

    def _validate_ana(self, program: Program) -> None:
        for decl in program.decls:
            if not isinstance(decl, FunctionDecl) or decl.name != "ana":
                continue
            rt = decl.return_type
            if rt.pointer_depth != 0 or rt.array_dims != 0 or rt.name not in ("sayi", "bos"):
                raise UppError(
                    decl.line,
                    f"'ana' C'de 'int main' üretir; dönüş türü 'sayi' veya 'bos' olmalı, '{type_desc(rt)}' verildi.",
                    decl.column,
                )
            if decl.params:
                raise UppError(
                    decl.line,
                    "'ana' parametre alamaz. Komut satırı için 'upp.arguman_sayisi()' ve 'upp.arguman(i)' kullanın.",
                    decl.column,
                )

    def _bind(self, name: str, typ: TypeNode, is_const: bool = False) -> None:
        if name in self.scopes[-1]:
            raise UppError(
                getattr(typ, "line", 1) or 1,
                f"'{name}' bu kapsamda birden fazla kez tanımlandı.",
                getattr(typ, "column", 1) or 1,
            )
        self.scopes[-1][name] = typ
        if is_const:
            self.const_names[-1].add(name)

    def _push(self) -> None:
        self.scopes.append({})
        self.const_names.append(set())

    def _pop(self) -> None:
        if len(self.scopes) > 1:
            self.scopes.pop()
            self.const_names.pop()

    def _name_is_const(self, name: str) -> bool:
        for names in reversed(self.const_names):
            if name in names:
                return True
        return False

    def _is_const_expr(self, expr: Optional[Expr]) -> bool:
        if expr is None:
            return False
        if isinstance(expr, Literal) and expr.kind in ("sayi", "ondalik", "metin", "mantik"):
            return True
        if isinstance(expr, UnaryOp) and expr.op in ("+", "-") and self._is_const_expr(expr.operand):
            return True
        if isinstance(expr, BinaryOp) and expr.op in ("+", "-", "*", "/", "%", "==", "!=", "<", ">", "<=", ">=", "&&", "||"):
            return self._is_const_expr(expr.left) and self._is_const_expr(expr.right)
        if isinstance(expr, Ident) and self._name_is_const(expr.name):
            return True
        if isinstance(expr, Member):
            obj_t = self._expr_type(expr.obj)
            return self._enum_from_ns(obj_t) is not None
        if isinstance(expr, TernaryOp):
            return (
                self._is_const_expr(expr.cond)
                and self._is_const_expr(expr.then_expr)
                and self._is_const_expr(expr.else_expr)
            )
        return False

    def _lookup(self, name: str, node: Node) -> TypeNode:
        for scope in reversed(self.scopes):
            if name in scope:
                return scope[name]
        raise UppError(node.line, f"Tanımsız ad: '{name}'.", node.column)

    def _decl_array_len(self, stmt: VarDecl) -> int:
        if isinstance(stmt.init, ArrayLiteral):
            return len(stmt.init.elements)
        sz = stmt.array_size
        if isinstance(sz, Literal) and sz.kind == "sayi":
            try:
                n = int(sz.value)
            except (TypeError, ValueError):
                return 0
            return n if n > 0 else 0
        return 0

    def _effective_var_type(self, stmt: VarDecl) -> TypeNode:
        dims = stmt.type.array_dims
        if stmt.array_size is not None or stmt.unsized_array:
            dims = 1
        return TypeNode(
            stmt.line,
            stmt.column,
            stmt.type.name,
            stmt.type.pointer_depth,
            dims,
            self._decl_array_len(stmt),
        )

    def _walk_stmt(self, stmt: Optional[Stmt]) -> None:
        if stmt is None:
            return
        if isinstance(stmt, Block):
            self._push()
            for s in stmt.stmts:
                if self._collect:
                    try:
                        self._walk_stmt(s)
                    except UppError as err:
                        self._errors.append(err)
                        if len(self._errors) >= 50:
                            self._pop()
                            return
                else:
                    self._walk_stmt(s)
            self._pop()
            return
        if isinstance(stmt, (UnsafeBlock, SafeBlock)):
            self._walk_stmt(stmt.body)
            return
        if isinstance(stmt, BackgroundBlock):
            self.background_depth += 1
            self._walk_stmt(stmt.body)
            self.background_depth -= 1
            return
        if isinstance(stmt, VarDecl):
            if stmt.array_size is not None:
                sz = self._expr_type(stmt.array_size)
                if not type_is_numeric(sz) or sz.name == "ondalik":
                    raise UppError(stmt.line, f"Dizi boyutu 'sayi' olmalıdır, '{type_desc(sz)}' bulundu.", stmt.column)
            if stmt.is_const:
                if stmt.init is None:
                    raise UppError(stmt.line, f"'sabit {stmt.name}' bir başlatıcı gerektirir.", stmt.column)
                if stmt.unsized_array or stmt.array_size is not None:
                    raise UppError(stmt.line, "'sabit' dizi olamaz; skaler bir tür kullanın.", stmt.column)
            if stmt.init is not None:
                init_t = self._expr_type(stmt.init)
                if stmt.inferred:
                    if type_is_void(init_t) or init_t.name == "":
                        raise UppError(stmt.line, f"'{stmt.name}' için tür çıkarılamadı (bos ifade).", stmt.column)
                    if isinstance(stmt.init, ArrayLiteral):
                        init_t.array_dims = max(init_t.array_dims, 1)
                    stmt.type = init_t
                else:
                    dest = self._effective_var_type(stmt)
                    if type_is_void(init_t) or not self._assignable(dest, init_t):
                        raise UppError(
                            stmt.line,
                            f"'{stmt.name}' başlatıcısı uyumsuz: beklenen '{type_desc(dest)}', bulunan '{type_desc(init_t)}'.",
                            stmt.column,
                        )
                if stmt.is_const and not self._is_const_expr(stmt.init):
                    raise UppError(
                        stmt.line,
                        f"'{stmt.name}' derleme zamanı sabit ifadesi ile başlatılmalıdır.",
                        stmt.column,
                    )
            elif stmt.inferred:
                raise UppError(stmt.line, f"'{stmt.name}' için tür çıkarılamadı.", stmt.column)
            bound = self._effective_var_type(stmt)
            self._bind(stmt.name, bound, is_const=stmt.is_const)
            return
        if isinstance(stmt, IfStmt):
            self._expr_type(stmt.cond)
            self._walk_stmt(stmt.then_branch)
            self._walk_stmt(stmt.else_branch)
            return
        if isinstance(stmt, WhileStmt):
            self._expr_type(stmt.cond)
            self._walk_stmt(stmt.body)
            return
        if isinstance(stmt, ForStmt):
            self._push()
            self._walk_stmt(stmt.init if isinstance(stmt.init, VarDecl) else None)
            if isinstance(stmt.init, ExprStmt):
                self._expr_type(stmt.init.expr)
            self._expr_type(stmt.cond)
            self._expr_type(stmt.step)
            self._walk_stmt(stmt.body)
            self._pop()
            return
        if isinstance(stmt, SwitchStmt):
            sw_t = self._expr_type(stmt.expr)
            if not self._is_switchable(sw_t):
                raise UppError(
                    stmt.line,
                    f"'secim' tam sayı, metin veya seçenek bekler, '{type_desc(sw_t)}' bulundu.",
                    stmt.column,
                )
            for case in stmt.cases:
                if case.value is not None:
                    ct = self._expr_type(case.value)
                    if not self._is_switch_const(case.value):
                        raise UppError(
                            case.line,
                            "'durum' değeri derleme zamanı sabiti (sayı, metin veya seçenek üyesi) olmalıdır.",
                            case.column,
                        )
                    if not self._assignable(sw_t, ct) and not self._assignable(ct, sw_t):
                        raise UppError(
                            case.line,
                            f"'durum' türü uyumsuz: beklenen '{type_desc(sw_t)}', bulunan '{type_desc(ct)}'.",
                            case.column,
                        )
                for s in case.stmts:
                    self._walk_stmt(s)
            return
        if isinstance(stmt, ForEachStmt):
            it_t = self._expr_type(stmt.iterable)
            if type_is_liste(it_t):
                elem_name = parse_liste_elem(it_t.name) or "sayi"
                elem = TypeNode(stmt.line, stmt.column, elem_name, 0, 0)
            elif type_is_harita(it_t):
                kv = parse_harita_kv(it_t.name)
                kname = kv[0] if kv else "metin"
                elem = TypeNode(stmt.line, stmt.column, kname, 0, 0)
            else:
                if it_t.array_dims < 1 and not isinstance(stmt.iterable, ArrayLiteral):
                    raise UppError(
                        stmt.line,
                        f"'her' gerçek bir dizi, liste veya harita bekler, '{type_desc(it_t)}' bulundu.",
                        stmt.column,
                    )
                if it_t.array_dims > 1:
                    raise UppError(
                        stmt.line,
                        "'her' tek boyutlu dizi bekler; iç içe '[...]' desteklenmez.",
                        stmt.column,
                    )
                if not self._is_countable_array(stmt.iterable):
                    raise UppError(
                        stmt.line,
                        "'her' yalnızca dizi değişkeni, liste veya '[...]' sabiti üzerinde çalışır.",
                        stmt.column,
                    )
                elem = TypeNode(stmt.line, stmt.column, it_t.name, it_t.pointer_depth, max(0, it_t.array_dims - 1))
            if stmt.inferred:
                stmt.var_type = elem
            elif not self._assignable(stmt.var_type, elem):
                raise UppError(
                    stmt.line,
                    f"'her' değişkeni '{stmt.var_name}' türü uyumsuz: beklenen '{type_desc(stmt.var_type)}', bulunan '{type_desc(elem)}'.",
                    stmt.column,
                )
            self._push()
            self._bind(stmt.var_name, stmt.var_type if not stmt.inferred else elem)
            self._walk_stmt(stmt.body)
            self._pop()
            return
        if isinstance(stmt, ReturnStmt):
            got = self._expr_type(stmt.value)
            self._check_return(stmt, got)
            return
        if isinstance(stmt, ExprStmt):
            check_removed_api(stmt.expr)
            self._expr_type(stmt.expr)
            return

    def _check_return(self, stmt: ReturnStmt, got: TypeNode) -> None:
        if self.background_depth > 0:
            if stmt.value is not None and not type_is_void(got):
                raise UppError(
                    stmt.line,
                    "'arkaplan' bloğunda 'don' değer alamaz; iş parçacığını bitirmek için 'don;' yazın.",
                    stmt.column,
                )
            return
        if self.current_fn is None:
            return
        expect = self.current_fn.return_type
        if type_is_void(expect):
            if stmt.value is not None and not type_is_void(got):
                raise UppError(
                    stmt.line,
                    f"'bos' dönen fonksiyonda değer döndürülemez (ifade türü '{type_desc(got)}').",
                    stmt.column,
                )
            return
        if stmt.value is None or type_is_void(got):
            raise UppError(
                stmt.line,
                f"'{type_desc(expect)}' dönen fonksiyonda 'don' bir değer gerektirir.",
                stmt.column,
            )
        if not self._assignable(expect, got):
            raise UppError(
                stmt.line,
                f"Dönüş türü uyumsuz: beklenen '{type_desc(expect)}', bulunan '{type_desc(got)}'.",
                stmt.column,
            )

    def _check_args(self, fn: FunctionDecl, args: List[Expr], line: int, column: int, display: str) -> None:
        nneed = len(fn.params)
        ngot = len(args)
        if ngot > nneed:
            raise UppError(line, f"'{display}' en fazla {nneed} argüman bekliyor, {ngot} verildi.", column)
        for i, param in enumerate(fn.params):
            if i < ngot:
                arg = args[i]
                got = self._expr_type(arg)
                if param.type is None:
                    continue
                if type_is_void(got) or not self._assignable(param.type, got):
                    raise UppError(
                        arg.line,
                        f"'{display}' argümanı '{param.name}': beklenen '{type_desc(param.type)}', bulunan '{type_desc(got)}'.",
                        arg.column,
                    )
            elif param.default is None:
                raise UppError(line, f"'{display}' {nneed} argüman bekliyor, {ngot} verildi.", column)

    def _ctor_type(self, cls: ClassDecl, expr: Call) -> TypeNode:
        olustur = [m for m in cls.methods if m.name == "olustur"]
        if len(olustur) != 1:
            raise UppError(
                expr.line,
                f"'{cls.name}(...)' için tek 'olustur' metodu gerekir.",
                expr.column,
            )
        m = olustur[0]
        if m.return_type.name != "bos" or m.return_type.pointer_depth:
            raise UppError(expr.line, f"'{cls.name}.olustur' dönüşü 'bos' olmalı.", expr.column)
        key = f"{cls.mod_alias}.{cls.name}" if cls.mod_alias else cls.name
        self._check_args(m, expr.args, expr.line, expr.column, f"{cls.name}.olustur")
        return TypeNode(expr.line, expr.column, key, 0, 0)

    def _expr_type(self, expr: Optional[Expr]) -> TypeNode:
        if expr is None:
            return TypeNode(0, 1, "bos", 0, 0)
        check_removed_api(expr)
        if isinstance(expr, Literal):
            if expr.kind == "yok":
                return TypeNode(expr.line, expr.column, "yok", 1, 0)
            if expr.kind == "metin":
                self._type_interp(expr)
            return TypeNode(expr.line, expr.column, expr.kind, 0, 0)
        if isinstance(expr, Ident):
            if expr.name == "kendi":
                if not self.current_class:
                    raise UppError(expr.line, "'kendi' yalnızca sınıf metodu içinde kullanılabilir.", expr.column)
                return TypeNode(expr.line, expr.column, self.current_class, 1, 0)
            if expr.name == "upp":
                return TypeNode(expr.line, expr.column, "upp", 0, 0)
            if expr.name in self.modules:
                return TypeNode(expr.line, expr.column, f"modul.{expr.name}", 0, 0)
            if expr.name in self.classes:
                return TypeNode(expr.line, expr.column, f"sinif.{expr.name}", 0, 0)
            if expr.name in self.enums:
                return TypeNode(expr.line, expr.column, self._enum_ns(expr.name), 0, 0)
            return self._lookup(expr.name, expr)
        if isinstance(expr, Member):
            return self._member_type(expr, as_call=False)
        if isinstance(expr, Index):
            inner = self._expr_type(expr.array)
            idx_t = self._expr_type(expr.index)
            if type_is_liste(inner):
                elem = parse_liste_elem(inner.name)
                if elem is None:
                    raise UppError(expr.line, f"'{type_desc(inner)}' dizinlenemez.", expr.column)
                if not type_is_numeric(idx_t) or idx_t.name == "ondalik":
                    raise UppError(expr.line, "Liste indeksi 'sayi' olmalı.", expr.column)
                return TypeNode(expr.line, expr.column, elem, 0, 0)
            if inner.array_dims > 0:
                return TypeNode(inner.line, inner.column, inner.name, inner.pointer_depth, inner.array_dims - 1)
            if inner.pointer_depth > 0:
                return TypeNode(inner.line, inner.column, inner.name, inner.pointer_depth - 1, 0)
            if type_is_metin(inner):
                if not type_is_numeric(idx_t) or idx_t.name == "ondalik":
                    raise UppError(expr.line, "Metin indeksi 'sayi' olmalı.", expr.column)
                return TypeNode(inner.line, inner.column, "sayi", 0, 0)
            raise UppError(expr.line, f"'{type_desc(inner)}' dizinlenemez.", expr.column)
        if isinstance(expr, Call):
            return self._call_type(expr)
        if isinstance(expr, Assign):
            if not is_lvalue(expr.target):
                raise UppError(
                    expr.line,
                    "Atama hedefi bir lvalue olmalı (değişken, alan, dizi elemanı veya *işaretçi).",
                    expr.column,
                )
            if isinstance(expr.target, Ident) and expr.target.name == "kendi":
                raise UppError(expr.line, "'kendi' yeniden atanamaz.", expr.column)
            if isinstance(expr.target, Ident) and self._name_is_const(expr.target.name):
                raise UppError(
                    expr.line,
                    f"'{expr.target.name}' sabittir; yeniden atanamaz.",
                    expr.column,
                )
            if isinstance(expr.target, Member):
                if upp_member_path(expr.target) is not None:
                    raise UppError(expr.line, "Yerleşik 'upp' üyesine atama yapılamaz.", expr.column)
                obj_t = self._expr_type(expr.target.obj)
                cls = self.classes.get(obj_t.name)
                if cls is not None and any(m.name == expr.target.name for m in cls.methods):
                    raise UppError(expr.line, f"'{expr.target.name}' bir metottur; atama yapılamaz.", expr.column)
            target_t = self._expr_type(expr.target)
            value_t = self._expr_type(expr.value)
            if expr.op != "=":
                if expr.op == "+=" and type_is_metin(target_t) and (type_is_metin(value_t) or type_is_yok(value_t)):
                    pass
                elif type_is_metin(target_t) or type_is_metin(value_t):
                    raise UppError(expr.line, f"'{expr.op}' metin üzerinde kullanılamaz.", expr.column)
                elif not self._is_numeric(target_t) or not self._is_numeric(value_t):
                    raise UppError(
                        expr.line,
                        f"'{expr.op}' sayısal tür bekler, '{type_desc(target_t)}' ve '{type_desc(value_t)}' bulundu.",
                        expr.column,
                    )
            if type_is_void(value_t) or not self._assignable(target_t, value_t):
                raise UppError(
                    expr.line,
                    f"Atama türü uyumsuz: beklenen '{type_desc(target_t)}', bulunan '{type_desc(value_t)}'.",
                    expr.column,
                )
            return target_t
        if isinstance(expr, TernaryOp):
            self._expr_type(expr.cond)
            then_t = self._expr_type(expr.then_expr)
            else_t = self._expr_type(expr.else_expr)
            if type_is_void(then_t) or type_is_void(else_t):
                raise UppError(expr.line, "'?:' dalları 'bos' olamaz.", expr.column)
            if self._assignable(then_t, else_t):
                return then_t
            if self._assignable(else_t, then_t):
                return else_t
            raise UppError(
                expr.line,
                f"'?:' dal türleri uyumsuz: '{type_desc(then_t)}' ve '{type_desc(else_t)}'.",
                expr.column,
            )
        if isinstance(expr, UnaryOp):
            inner = self._expr_type(expr.operand)
            if expr.op == "&":
                if not is_lvalue(expr.operand):
                    raise UppError(expr.line, "Adres alma ('&') bir lvalue gerektirir.", expr.column)
                return TypeNode(inner.line, inner.column, inner.name, inner.pointer_depth + 1, inner.array_dims)
            if expr.op == "*":
                if inner.pointer_depth <= 0:
                    raise UppError(expr.line, f"'{type_desc(inner)}' işaretçi değil; '*' uygulanamaz.", expr.column)
                return TypeNode(inner.line, inner.column, inner.name, inner.pointer_depth - 1, 0)
            if expr.op in ("+", "-"):
                if not self._is_numeric(inner):
                    raise UppError(expr.line, f"Tekil '{expr.op}' sayısal tür bekler, '{type_desc(inner)}' bulundu.", expr.column)
                if inner.name == "mantik":
                    return TypeNode(expr.line, expr.column, "sayi", 0, 0)
                return inner
            if expr.op == "!":
                if type_is_class_value(inner, self.classes):
                    raise UppError(expr.line, "Sınıf değerine '!' uygulanamaz.", expr.column)
                return TypeNode(expr.line, expr.column, "mantik", 0, 0)
            return inner
        if isinstance(expr, BinaryOp):
            return self._binary_type(expr)
        if isinstance(expr, ArrayLiteral):
            if not expr.elements:
                return TypeNode(expr.line, expr.column, "sayi", 0, 1)
            inner = self._expr_type(expr.elements[0])
            if inner.array_dims > 0:
                raise UppError(
                    expr.line,
                    "İç içe dizi sabiti yok; tek boyut veya 'liste[T]' kullanın.",
                    expr.column,
                )
            for el in expr.elements[1:]:
                t = self._expr_type(el)
                if t.name != inner.name or t.pointer_depth != inner.pointer_depth or t.array_dims != inner.array_dims:
                    raise UppError(el.line, "Dizi elemanlarının türleri tutarsız.", el.column)
            return TypeNode(inner.line, inner.column, inner.name, inner.pointer_depth, inner.array_dims + 1)
        if isinstance(expr, BackgroundExpr):
            self.background_depth += 1
            if expr.body:
                self._walk_stmt(expr.body)
            self.background_depth -= 1
            return TypeNode(expr.line, expr.column, "ArkaplanIs", 0, 0)
        return TypeNode(getattr(expr, "line", 0), getattr(expr, "column", 1), "bos", 0, 0)

    def _beklenen_arg(self, got: TypeNode, expect_name: str, display: str, arg: Expr) -> None:
        expect = TypeNode(arg.line, arg.column, expect_name, 0, 0)
        if not self._assignable(expect, got):
            raise UppError(
                arg.line,
                f"'{display}' '{expect_name}' bekler, '{type_desc(got)}' bulundu.",
                arg.column,
            )

    def _liste_member_type(self, obj_t: TypeNode, expr: Member) -> Optional[TypeNode]:
        elem = parse_liste_elem(obj_t.name)
        if elem is None or obj_t.pointer_depth != 0:
            return None
        name = expr.name
        if name in ("ekle", "yaz", "bosalt"):
            return TypeNode(expr.line, expr.column, "bos", 0, 0)
        if name == "al":
            return TypeNode(expr.line, expr.column, elem, 0, 0)
        if name == "uzunluk":
            return TypeNode(expr.line, expr.column, "sayi", 0, 0)
        raise UppError(
            expr.line,
            f"'liste' üzerinde '{name}' yok. ekle, al, yaz, uzunluk, bosalt.",
            expr.column,
        )

    def _harita_member_type(self, obj_t: TypeNode, expr: Member) -> Optional[TypeNode]:
        kv = parse_harita_kv(obj_t.name)
        if kv is None or obj_t.pointer_depth != 0:
            return None
        _key, val = kv
        name = expr.name
        if name in ("koy", "bosalt"):
            return TypeNode(expr.line, expr.column, "bos", 0, 0)
        if name == "al":
            return TypeNode(expr.line, expr.column, val, 0, 0)
        if name == "var_mi":
            return TypeNode(expr.line, expr.column, "mantik", 0, 0)
        if name == "uzunluk":
            return TypeNode(expr.line, expr.column, "sayi", 0, 0)
        if name == "anahtarlar":
            return TypeNode(expr.line, expr.column, f"liste[{_key}]", 0, 0)
        raise UppError(
            expr.line,
            f"'harita' üzerinde '{name}' yok. koy, al, var_mi, uzunluk, bosalt, anahtarlar.",
            expr.column,
        )

    def _liste_call_type(self, expr: Call, obj_t: TypeNode) -> TypeNode:
        elem = parse_liste_elem(obj_t.name) or "sayi"
        name = expr.callee.name
        args = expr.args
        line, col = expr.line, expr.column
        if name == "ekle":
            _check_arity("liste.ekle", len(args), line, col, (1, 1))
            self._beklenen_arg(self._expr_type(args[0]), elem, "liste.ekle", args[0])
            return TypeNode(line, col, "bos", 0, 0)
        if name == "al":
            _check_arity("liste.al", len(args), line, col, (1, 1))
            self._beklenen_arg(self._expr_type(args[0]), "sayi", "liste.al", args[0])
            return TypeNode(line, col, elem, 0, 0)
        if name == "yaz":
            _check_arity("liste.yaz", len(args), line, col, (2, 2))
            self._beklenen_arg(self._expr_type(args[0]), "sayi", "liste.yaz", args[0])
            self._beklenen_arg(self._expr_type(args[1]), elem, "liste.yaz", args[1])
            return TypeNode(line, col, "bos", 0, 0)
        if name == "uzunluk":
            _check_arity("liste.uzunluk", len(args), line, col, (0, 0))
            return TypeNode(line, col, "sayi", 0, 0)
        if name == "bosalt":
            _check_arity("liste.bosalt", len(args), line, col, (0, 0))
            return TypeNode(line, col, "bos", 0, 0)
        raise UppError(line, f"'liste' üzerinde '{name}' yok.", col)

    def _harita_call_type(self, expr: Call, obj_t: TypeNode) -> TypeNode:
        kv = parse_harita_kv(obj_t.name)
        key, val = kv if kv else ("metin", "sayi")
        name = expr.callee.name
        args = expr.args
        line, col = expr.line, expr.column
        if name == "koy":
            _check_arity("harita.koy", len(args), line, col, (2, 2))
            self._beklenen_arg(self._expr_type(args[0]), key, "harita.koy", args[0])
            self._beklenen_arg(self._expr_type(args[1]), val, "harita.koy", args[1])
            return TypeNode(line, col, "bos", 0, 0)
        if name == "al":
            _check_arity("harita.al", len(args), line, col, (1, 1))
            self._beklenen_arg(self._expr_type(args[0]), key, "harita.al", args[0])
            return TypeNode(line, col, val, 0, 0)
        if name == "var_mi":
            _check_arity("harita.var_mi", len(args), line, col, (1, 1))
            self._beklenen_arg(self._expr_type(args[0]), key, "harita.var_mi", args[0])
            return TypeNode(line, col, "mantik", 0, 0)
        if name == "uzunluk":
            _check_arity("harita.uzunluk", len(args), line, col, (0, 0))
            return TypeNode(line, col, "sayi", 0, 0)
        if name == "bosalt":
            _check_arity("harita.bosalt", len(args), line, col, (0, 0))
            return TypeNode(line, col, "bos", 0, 0)
        if name == "anahtarlar":
            _check_arity("harita.anahtarlar", len(args), line, col, (0, 0))
            return TypeNode(line, col, f"liste[{key}]", 0, 0)
        raise UppError(line, f"'harita' üzerinde '{name}' yok.", col)

    def _member_type(self, expr: Member, as_call: bool) -> TypeNode:
        obj_t = self._expr_type(expr.obj)
        enum_name = self._enum_from_ns(obj_t)
        if enum_name:
            ed = self.enums[enum_name]
            for var in ed.variants:
                if var.name == expr.name:
                    if as_call:
                        raise UppError(expr.line, f"'{expr.name}' bir seçenek değeridir; çağrılamaz.", expr.column)
                    return TypeNode(expr.line, expr.column, enum_name, 0, 0)
            raise UppError(expr.line, f"'{enum_name}' seçeneğinde '{expr.name}' yok.", expr.column)
        ns_t = upp_ns_type(obj_t, expr.name, expr.line, expr.column)
        if obj_t.name == "upp" or obj_t.name.startswith("upp."):
            if ns_t is None:
                prefix = obj_t.name if obj_t.name != "upp" else "upp"
                raise UppError(expr.line, f"'{prefix}.{expr.name}' yerleşik bir üye değil.", expr.column)
            return ns_t
        if obj_t.name.startswith("modul."):
            alias = obj_t.name[6:]
            bag = self.modules.get(alias) or {}
            if expr.name in bag.get("fn", {}):
                fn = bag["fn"][expr.name]
                return TypeNode(fn.line, fn.column, fn.return_type.name, fn.return_type.pointer_depth, 0)
            if expr.name in bag.get("class", {}):
                key = f"{alias}.{expr.name}"
                return TypeNode(expr.line, expr.column, f"sinif.{key}", 0, 0)
            if expr.name in bag.get("enum", {}):
                key = f"{alias}.{expr.name}"
                return TypeNode(expr.line, expr.column, self._enum_ns(key), 0, 0)
            if expr.name in bag.get("var", {}):
                vd = bag["var"][expr.name]
                return TypeNode(vd.line, vd.column, vd.type.name, vd.type.pointer_depth, vd.type.array_dims)
            raise UppError(expr.line, f"'{alias}' modülünde '{expr.name}' yok.", expr.column)
        if obj_t.name.startswith("sinif."):
            key = obj_t.name[6:]
            cls = self.classes.get(key)
            if cls is None:
                raise UppError(expr.line, f"'{key}' bir sınıf değil.", expr.column)
            for c in cls.constants:
                if c.name == expr.name:
                    if as_call:
                        raise UppError(expr.line, f"'{expr.name}' sınıf sabitidir; çağrılamaz.", expr.column)
                    return TypeNode(c.line, c.column, c.type.name, 0, 0)
            raise UppError(expr.line, f"'{cls.name}' sınıfında '{expr.name}' sabiti yok. Örnek üzerinden değil '{key}.{expr.name}' yazın.", expr.column)
        liste_t = self._liste_member_type(obj_t, expr)
        if liste_t is not None:
            return liste_t
        harita_t = self._harita_member_type(obj_t, expr)
        if harita_t is not None:
            return harita_t
        cls = self.classes.get(obj_t.name)
        if cls is None:
            raise UppError(expr.line, f"'{obj_t.name}' bir sınıf değil; '.' üye erişimi yok.", expr.column)
        for field in cls.fields:
            if field.name == expr.name:
                if as_call:
                    raise UppError(expr.line, f"'{expr.name}' bir alandır; fonksiyon gibi çağrılamaz.", expr.column)
                dims = 1 if (field.array_size is not None or field.unsized_array) else 0
                return TypeNode(field.line, field.column, field.type.name, field.type.pointer_depth, dims)
        for method in cls.methods:
            if method.name == expr.name:
                return TypeNode(method.line, method.column, method.return_type.name, method.return_type.pointer_depth, 0)
        if cls.name == "ArkaplanIs" and expr.name == "bekle":
            return TypeNode(expr.line, expr.column, "bos", 0, 0)
        if cls.name == "Kilit" and expr.name in ("kilitle", "birak"):
            return TypeNode(expr.line, expr.column, "bos", 0, 0)
        for c in cls.constants:
            if c.name == expr.name:
                raise UppError(
                    expr.line,
                    f"'{c.name}' sınıf sabitidir; '{cls.name}.{c.name}' yazın (örnek üzerinden değil).",
                    expr.column,
                )
        raise UppError(expr.line, f"'{cls.name}' sınıfında '{expr.name}' üyesi yok.", expr.column)

    def _call_type(self, expr: Call) -> TypeNode:
        if isinstance(expr.callee, Member):
            obj_t = self._expr_type(expr.callee.obj)
            if obj_t.name == "upp":
                name = expr.callee.name
                if name in UPP_NAMESPACES:
                    raise UppError(
                        expr.line,
                        f"'upp.{name}' bir ad alanıdır; 'upp.{name}.fonksiyon(...)' yazın.",
                        expr.column,
                    )
                if name not in UPP_RETURNS:
                    raise UppError(expr.line, f"'upp.{name}' yerleşik bir fonksiyon değil.", expr.column)
                _check_arity(f"upp.{name}", len(expr.args), expr.line, expr.column, UPP_ARITY[name])
                for arg in expr.args:
                    at = self._expr_type(arg)
                    self._type_interp(arg)
                if name == "bekle":
                    if len(expr.args) != 1 or self._expr_type(expr.args[0]).name != "ArkaplanIs":
                        raise UppError(
                            expr.line,
                            "'upp.bekle' bir ArkaplanIs değeri bekler.",
                            expr.column,
                        )
                elif name == "arguman":
                    t = self._expr_type(expr.args[0])
                    if not type_is_numeric(t) or t.name == "ondalik":
                        raise UppError(expr.line, "'upp.arguman' sayi indeks bekler.", expr.column)
                elif name in ("dosya_var_mi", "dosya_sil", "metinden_sayiya", "metinden_ondalik", "ortam_al"):
                    t = self._expr_type(expr.args[0])
                    if not type_is_metin(t) and not type_is_yok(t):
                        raise UppError(expr.line, f"'upp.{name}' metin bekler, '{type_desc(t)}' bulundu.", expr.column)
                elif name == "uzunluk":
                    t = self._expr_type(expr.args[0])
                    if t.array_dims > 0:
                        if not self._is_countable_array(expr.args[0]):
                            raise UppError(
                                expr.args[0].line,
                                "'upp.uzunluk' yalnızca gerçek dizi değişkeni veya '[...]' sabiti alır; işaretçi verilemez.",
                                expr.args[0].column,
                            )
                    elif not type_is_metin(t) and not type_is_yok(t) and not type_is_liste(t) and not type_is_harita(t):
                        raise UppError(
                            expr.line,
                            f"'upp.uzunluk' metin, dizi, liste veya harita bekler, '{type_desc(t)}' bulundu.",
                            expr.column,
                        )
                elif name in ("sayidan_metin",):
                    t = self._expr_type(expr.args[0])
                    if not type_is_numeric(t) or t.name == "ondalik":
                        raise UppError(expr.line, "'upp.sayidan_metin' sayi bekler.", expr.column)
                elif name == "ondalikdan_metin":
                    t = self._expr_type(expr.args[0])
                    if not type_is_numeric(t):
                        raise UppError(expr.line, "'upp.ondalikdan_metin' sayısal argüman bekler.", expr.column)
                elif name == "dosya_ekle":
                    for arg in expr.args:
                        at = self._expr_type(arg)
                        if not type_is_metin(at) and not type_is_yok(at):
                            raise UppError(arg.line, f"'upp.dosya_ekle' metin bekler, '{type_desc(at)}' bulundu.", arg.column)
                ret = UPP_RETURNS[name]
                return TypeNode(expr.line, expr.column, ret, 0, 0)
            if obj_t.name.startswith("upp."):
                ns = obj_t.name[4:]
                table = UPP_NAMESPACES.get(ns)
                name = expr.callee.name
                if not table or name not in table:
                    raise UppError(expr.line, f"'{obj_t.name}.{name}' yerleşik bir fonksiyon değil.", expr.column)
                ret, arity, _cname = table[name]
                _check_arity(f"{obj_t.name}.{name}", len(expr.args), expr.line, expr.column, arity)
                self._check_ns_args(ns, name, expr.args)
                return TypeNode(expr.line, expr.column, ret, 0, 0)
            if obj_t.name.startswith("modul."):
                alias = obj_t.name[6:]
                bag = self.modules.get(alias) or {}
                name = expr.callee.name
                if name in bag.get("fn", {}):
                    fn = bag["fn"][name]
                    self._check_args(fn, expr.args, expr.line, expr.column, f"{alias}.{name}")
                    return TypeNode(fn.line, fn.column, fn.return_type.name, fn.return_type.pointer_depth, 0)
                if name in bag.get("class", {}):
                    return self._ctor_type(bag["class"][name], expr)
                raise UppError(expr.line, f"'{alias}' modülünde '{name}' yok.", expr.column)
            if obj_t.name.startswith("sinif."):
                key = obj_t.name[6:]
                cls_ctor = self.classes.get(key)
                if cls_ctor is None:
                    raise UppError(expr.line, f"'{key}' bir sınıf değil.", expr.column)
                return self._ctor_type(cls_ctor, expr)
            if type_is_liste(obj_t):
                return self._liste_call_type(expr, obj_t)
            if type_is_harita(obj_t):
                return self._harita_call_type(expr, obj_t)
            cls = self.classes.get(obj_t.name)
            if cls is None:
                raise UppError(expr.line, f"'{obj_t.name}' bir sınıf değil; metot çağrılamaz.", expr.column)
            if cls.name == "ArkaplanIs" and expr.callee.name == "bekle":
                _check_arity(f"{cls.name}.bekle", len(expr.args), expr.line, expr.column, (0, 0))
                return TypeNode(expr.line, expr.column, "bos", 0, 0)
            if cls.name == "Kilit" and expr.callee.name in ("kilitle", "birak"):
                _check_arity(f"{cls.name}.{expr.callee.name}", len(expr.args), expr.line, expr.column, (0, 0))
                return TypeNode(expr.line, expr.column, "bos", 0, 0)
            method = next((m for m in cls.methods if m.name == expr.callee.name), None)
            if method is None:
                raise UppError(expr.line, f"'{cls.name}' sınıfında '{expr.callee.name}' metodu yok.", expr.column)
            self._check_args(method, expr.args, expr.line, expr.column, f"{cls.name}.{method.name}")
            return TypeNode(method.line, method.column, method.return_type.name, method.return_type.pointer_depth, 0)
        if isinstance(expr.callee, Ident):
            name = expr.callee.name
            if name == "bellek_ayir":
                _check_arity(name, len(expr.args), expr.line, expr.column, (1, 1))
                for arg in expr.args:
                    self._expr_type(arg)
                return TypeNode(expr.line, expr.column, "bos", 1, 0)
            if name == "bellek_bosalt":
                _check_arity(name, len(expr.args), expr.line, expr.column, (1, 1))
                for arg in expr.args:
                    self._expr_type(arg)
                return TypeNode(expr.line, expr.column, "bos", 0, 0)
            if name in self.classes:
                return self._ctor_type(self.classes[name], expr)
            fn = self.functions.get(name)
            if fn is None:
                raise UppError(expr.line, f"Tanımsız fonksiyon: '{name}'.", expr.column)
            self._check_args(fn, expr.args, expr.line, expr.column, name)
            return TypeNode(fn.line, fn.column, fn.return_type.name, fn.return_type.pointer_depth, 0)
        raise UppError(expr.line, "Yalnızca ad veya 'nesne.metot' çağrılabilir.", expr.column)

    def _check_ns_args(self, ns: str, name: str, args: List[Expr]) -> None:
        display = f"upp.{ns}.{name}"
        kinds: List[str]
        if ns == "girdi":
            kinds = {
                "fare_tasi": ["sayi", "sayi"],
                "fare_tikla": [],
                "tus_bas": ["metin"],
            }[name]
        elif ns == "bellek":
            kinds = {
                "baglan": ["metin"],
                "modul_bul": ["metin"],
                "sayioku": ["sayi"],
                "oku_ondalik": ["sayi"],
                "oku_metin": ["sayi", "sayi"],
                "sayiyaz": ["sayi", "sayi"],
                "yaz_ondalik": ["sayi", "ondalik"],
                "zincir_oku": ["sayi", "sayi[]"],
                "yama_yap": ["sayi", "metin"],
                "koruma_degistir": ["sayi", "sayi", "izin"],
                "son_hata": [],
                "okundu": [],
            }[name]
        elif ns == "cizim":
            kinds = {
                "baslat": ["metin"],
                "kutu": ["sayi", "sayi", "sayi", "sayi", "metin"],
                "yazi": ["sayi", "sayi", "metin", "metin"],
                "temizle": [],
            }[name]
        elif ns == "metin":
            kinds = {
                "kes": ["metin", "sayi", "sayi"],
                "icinde": ["metin", "metin"],
                "degistir": ["metin", "metin", "metin"],
                "baslar": ["metin", "metin"],
                "biter": ["metin", "metin"],
                "kirp": ["metin"],
                "buyuk": ["metin"],
                "kucuk": ["metin"],
                "kod": ["metin", "sayi"],
                "ileri": ["metin", "sayi"],
                "bol": ["metin", "metin"],
                "birlestir": ["liste[metin]", "metin"],
            }[name]
        elif ns == "json":
            kinds = {
                "ayristir": ["metin"],
                "olustur": ["JSONDeger"],
                "tur": ["JSONDeger"],
                "anahtarlar": ["JSONDeger"],
                "anahtar_al": ["JSONDeger", "metin"],
                "dizi_eleman": ["JSONDeger", "sayi"],
                "metin_al": ["JSONDeger"],
                "sayi_al": ["JSONDeger"],
                "ondalik_al": ["JSONDeger"],
                "mantik_al": ["JSONDeger"],
                "uzunluk": ["JSONDeger"],
                "var_mi": ["JSONDeger"],
                "nesne": [],
                "dizi": [],
                "metin_yap": ["metin"],
                "sayi_yap": ["sayi"],
                "ondalik_yap": ["ondalik"],
                "mantik_yap": ["mantik"],
                "yok_yap": [],
                "koy": ["JSONDeger", "metin", "JSONDeger"],
                "ekle": ["JSONDeger", "JSONDeger"],
                "bosalt": ["JSONDeger"],
            }[name]
        elif ns == "stdio":
            kinds = {
                "gelen_oku": [],
                "yaz": ["metin"],
            }[name]
        elif ns == "sistem":
            kinds = {
                "calistir": ["metin"],
                "komut_bul": ["metin"],
                "exe_dizin": [],
            }[name]
        elif ns == "yol":
            kinds = {
                "dizin": ["metin"],
                "birlestir": ["metin", "metin"],
                "gercek": ["metin"],
                "calisma": [],
                "mutlak_mi": ["metin"],
                "klasor_olustur": ["metin"],
            }[name]
        elif ns == "dosya":
            kinds = {
                "listele": ["metin"],
            }[name]
        elif ns == "matematik":
            kinds = {
                "mutlak": ["ondalik"],
                "karekok": ["ondalik"],
                "us": ["ondalik", "ondalik"],
                "sin": ["ondalik"],
                "cos": ["ondalik"],
                "sinirla": ["ondalik", "ondalik", "ondalik"],
                "mesafe_3d": ["ondalik"] * 6,
                "mesafe": ["Vektor3", "Vektor3"],
                "aci_hesapla": ["ondalik"] * 6,
                "dunya_ekran": ["ondalik", "ondalik", "ondalik", "ondalik[]", "ondalik", "ondalik"],
                "yumusat": ["ondalik", "ondalik", "ondalik"],
                "tan": ["ondalik"],
                "atan2": ["ondalik", "ondalik"],
                "taban": ["ondalik"],
                "tavan": ["ondalik"],
                "yuvarla": ["ondalik"],
                "pi": [],
            }[name]
        else:
            raise UppError(args[0].line if args else 1, f"'upp.{ns}.{name}' yerleşik bir fonksiyon değil.")
        if ns == "girdi" and name == "tus_bas" and args:
            first = args[0]
            if isinstance(first, Literal) and first.kind == "metin":
                key = decode_string_literal(first.value)
                if not known_tus_name(key):
                    raise UppError(
                        first.line,
                        f"'upp.girdi.tus_bas' bilinmeyen tuş: '{key}'.",
                        first.column,
                    )
        for arg, kind in zip(args, kinds):
            t = self._expr_type(arg)
            if kind == "metin":
                if not type_is_metin(t) and not type_is_yok(t):
                    raise UppError(arg.line, f"'{display}' metin argümanı bekler, '{type_desc(t)}' bulundu.", arg.column)
            elif kind.startswith("liste["):
                if t.name != kind or t.pointer_depth != 0:
                    raise UppError(arg.line, f"'{display}' '{kind}' bekler, '{type_desc(t)}' bulundu.", arg.column)
            elif kind == "mantik":
                if t.name != "mantik" or t.pointer_depth != 0 or t.array_dims != 0:
                    raise UppError(arg.line, f"'{display}' mantik argümanı bekler, '{type_desc(t)}' bulundu.", arg.column)
            elif kind == "izin":
                if not type_is_metin(t) and not (type_is_numeric(t) and t.name != "ondalik"):
                    raise UppError(arg.line, f"'{display}' izin türü metin veya sayi olmalı.", arg.column)
            elif kind.endswith("[]"):
                inner = kind[:-2]
                ok_name = t.name in (("sayi", "ondalik") if inner == "ondalik" else ("sayi",))
                if t.array_dims < 1 or not ok_name:
                    raise UppError(arg.line, f"'{display}' {inner} dizisi bekler, '{type_desc(t)}' bulundu.", arg.column)
                if not self._is_countable_array(arg):
                    raise UppError(
                        arg.line,
                        f"'{display}' yalnızca gerçek dizi değişkeni veya '[...]' sabiti alır; işaretçi verilemez.",
                        arg.column,
                    )
            elif kind == "sayi":
                if t.name != "sayi" or t.pointer_depth != 0 or t.array_dims != 0:
                    raise UppError(arg.line, f"'{display}' sayi bekler, '{type_desc(t)}' bulundu.", arg.column)
            elif kind == "ondalik":
                if not type_is_numeric(t):
                    raise UppError(arg.line, f"'{display}' sayısal argüman bekler, '{type_desc(t)}' bulundu.", arg.column)
            elif kind in self.classes or kind in self.enums:
                if t.name != kind or t.pointer_depth != 0 or t.array_dims != 0:
                    raise UppError(arg.line, f"'{display}' '{kind}' bekler, '{type_desc(t)}' bulundu.", arg.column                    )
            else:
                if not type_is_numeric(t):
                    raise UppError(arg.line, f"'{display}' sayısal argüman bekler, '{type_desc(t)}' bulundu.", arg.column)
        if ns == "matematik" and name == "dunya_ekran" and len(args) >= 4:
            mat = args[3]
            n: Optional[int] = None
            if isinstance(mat, ArrayLiteral):
                n = len(mat.elements)
            elif isinstance(mat, Ident):
                t = self._lookup(mat.name, mat)
                if t.array_len > 0:
                    n = t.array_len
            if n is not None and n < 16:
                raise UppError(
                    mat.line,
                    "'upp.matematik.dunya_ekran' 16 elemanlı (4x4) ondalik dizisi bekler.",
                    mat.column,
                )

    def _type_interp(self, expr: Expr) -> None:
        if not isinstance(expr, Literal) or expr.kind != "metin":
            return
        parsed = parse_interpolation(expr.value, expr.line, expr.column)
        if parsed is None:
            return
        for kind, payload in flatten_interp_parts(parsed):
            if kind == "expr":
                self._expr_type(payload)

    def _binary_type(self, expr: BinaryOp) -> TypeNode:
        left = self._expr_type(expr.left)
        right = self._expr_type(expr.right)
        op = expr.op
        line, col = expr.line, expr.column
        mantik = TypeNode(line, col, "mantik", 0, 0)

        if type_is_class_value(left, self.classes) or type_is_class_value(right, self.classes):
            raise UppError(
                line,
                f"Sınıf değerleri '{op}' ile kullanılamaz. C'de yapı (struct) karşılaştırması veya aritmetiği yoktur.",
                col,
            )

        if op == "+":
            if type_is_metin(left) and type_is_metin(right):
                return TypeNode(line, col, "metin", 0, 0)
            if type_is_metin(left) or type_is_metin(right):
                raise UppError(line, "Metin birleştirme yalnızca 'metin + metin' biçimindedir.", col)

        if op in ("==", "!="):
            if type_is_yok(left) or type_is_yok(right):
                other = right if type_is_yok(left) else left
                if type_is_metin(other) or other.pointer_depth > 0 or type_is_yok(other):
                    return mantik
                raise UppError(line, "'yok' yalnızca metin veya işaretçi ile karşılaştırılabilir.", col)
            if type_is_metin(left) and type_is_metin(right):
                return mantik
            if type_is_metin(left) or type_is_metin(right):
                raise UppError(line, "Metin yalnızca başka bir metin veya 'yok' ile karşılaştırılabilir.", col)
            if left.pointer_depth > 0 or right.pointer_depth > 0:
                if left.pointer_depth > 0 and right.pointer_depth > 0:
                    return mantik
                raise UppError(line, "İşaretçi yalnızca işaretçi veya 'yok' ile karşılaştırılabilir.", col)
            if type_is_numeric(left) and type_is_numeric(right):
                return mantik
            if self._is_numeric(left) and self._is_numeric(right):
                return mantik
            raise UppError(line, f"'{type_desc(left)}' ve '{type_desc(right)}' karşılaştırılamaz.", col)

        if op in ("<", ">", "<=", ">="):
            if type_is_metin(left) or type_is_metin(right) or left.pointer_depth or right.pointer_depth:
                raise UppError(line, f"'{op}' yalnızca sayısal türlerde kullanılabilir.", col)
            if not self._is_numeric(left) or not self._is_numeric(right):
                raise UppError(line, f"'{op}' '{type_desc(left)}' ve '{type_desc(right)}' üzerinde geçersiz.", col)
            return mantik

        if op in ("&&", "||"):
            if type_is_void(left) or type_is_void(right):
                raise UppError(line, f"'{op}' boş (bos) ifadede kullanılamaz.", col)
            return mantik

        if left.pointer_depth or right.pointer_depth:
            raise UppError(line, f"İşaretçi aritmetiği ('{op}') u++'ta yoktur.", col)
        if type_is_metin(left) or type_is_metin(right):
            raise UppError(line, f"'{op}' metin üzerinde kullanılamaz; birleştirme için '+'.", col)
        if not self._is_numeric(left) or not self._is_numeric(right):
            raise UppError(line, f"'{op}' '{type_desc(left)}' ve '{type_desc(right)}' üzerinde geçersiz.", col)
        if op in ("%", "&", "|", "^") and (left.name == "ondalik" or right.name == "ondalik"):
            raise UppError(line, f"'{op}' ondalik türünde kullanılamaz.", col)
        if left.name == "ondalik" or right.name == "ondalik":
            return TypeNode(line, col, "ondalik", 0, 0)
        if left.name == "sayi" or right.name == "sayi" or left.name in self.enums or right.name in self.enums:
            return TypeNode(line, col, "sayi", 0, 0)
        return left


# ---------------------------------------------------------------------------
# Güvenlik
# ---------------------------------------------------------------------------

UNSAFE_CALLS = {"bellek_ayir", "bellek_bosalt"}


class SafetyChecker:
    def __init__(self, collect: bool = False) -> None:
        self.collect = collect
        self.errors: List[SafetyError] = []

    def check(self, program: Program) -> List[SafetyError]:
        for decl in program.decls:
            if isinstance(decl, FunctionDecl):
                self._walk(decl.body, unsafe=False)
            elif isinstance(decl, ClassDecl):
                for field in decl.fields:
                    if field.type.pointer_depth > 0:
                        stars = "*" * field.type.pointer_depth
                        self._err(SafetyError(
                            field.line,
                            f"İşaretçi türü '{field.type.name}{stars} {field.name}' sınıf alanı olarak "
                            f"yalnızca güvenli modelin dışında kalır; ham işaretçi alanı kullanılamaz. "
                            f"'metin' (yerleşik) veya 'guvensiz' bloktaki yerel işaretçi kullanın.",
                            field.column,
                        ))
                for method in decl.methods:
                    self._walk(method.body, unsafe=False)
        return self.errors

    def _err(self, err: SafetyError) -> None:
        if self.collect:
            if len(self.errors) < 50:
                self.errors.append(err)
            return
        raise err

    def _init_ptr_islem(self, node: VarDecl) -> bool:
        return isinstance(node.init, UnaryOp) and node.init.op in ("*", "&")

    def _walk(self, node: Optional[Node], unsafe: bool) -> None:
        if node is None:
            return
        if isinstance(node, UnsafeBlock):
            self._walk(node.body, True)
            return
        if isinstance(node, SafeBlock):
            self._walk(node.body, False)
            return
        if isinstance(node, BackgroundBlock):
            self._walk(node.body, unsafe)
            return
        if isinstance(node, BackgroundExpr):
            self._walk(node.body, unsafe)
            return
        if isinstance(node, CEmbed):
            if not unsafe:
                self._err(SafetyError(node.line, "Ham C gömme ('c_kod') yalnızca 'guvensiz' blokta kullanılabilir.", node.column))
                return
            if c_embed_has_preprocessor(node.code):
                raise UppError(
                    node.line,
                    "c_kod içinde '#include' / '#define' kullanılamaz. Başlık eklemek için dosya başında c_ekle \"<math.h>\"; yazın.",
                    node.column,
                )
            return
        if isinstance(node, AsmEmbed) and not unsafe:
            self._err(SafetyError(node.line, "Ham asm gömme ('asm_kod') yalnızca 'guvensiz' blokta kullanılabilir.", node.column))
            return
        if isinstance(node, UnaryOp) and node.op in ("*", "&") and not unsafe:
            kind = "işaretçi çözümlemesi ('*')" if node.op == "*" else "adres alma ('&')"
            self._err(SafetyError(node.line, f"Doğrudan {kind} 'guvenli' alanda yasaktır.", node.column))
        if isinstance(node, Call):
            name = None
            if isinstance(node.callee, Ident):
                name = node.callee.name
            path = upp_member_path(node.callee)
            if path and len(path) >= 1 and path[0] in UNSAFE_UPP_NS and not unsafe:
                self._err(SafetyError(
                    node.line,
                    f"'upp.{'.'.join(path)}(...)' süreç belleğine erişir ve yalnızca 'guvensiz' blokta çağrılabilir.",
                    node.column,
                ))
            if name in UNSAFE_CALLS and not unsafe:
                self._err(SafetyError(
                    node.line,
                    f"'{name}(...)' doğrudan bellek manipülasyonudur ve yalnızca 'guvensiz' blokta çağrılabilir.",
                    node.column,
                ))
        if isinstance(node, VarDecl) and node.type.pointer_depth > 0 and not unsafe and not self._init_ptr_islem(node):
            stars = "*" * node.type.pointer_depth
            self._err(SafetyError(
                node.line,
                f"İşaretçi türü '{node.type.name}{stars} {node.name}' yalnızca 'guvensiz' blokta bildirilebilir.",
                node.column,
            ))
        for child in self._children(node):
            self._walk(child, unsafe)

    def _children(self, node: Node) -> List[Node]:
        out: List[Node] = []
        for value in getattr(node, "__dict__", {}).values():
            if isinstance(value, Node):
                out.append(value)
            elif isinstance(value, list):
                for item in value:
                    if isinstance(item, Node):
                        out.append(item)
        return out


# ---------------------------------------------------------------------------
# C runtime
# ---------------------------------------------------------------------------

C_TYPE = {
    "sayi": "long long",
    "ondalik": "double",
    "metin": "char*",
    "mantik": "bool",
    "bos": "void",
}

RUNTIME_C = ""  # asıl kaynak: src/runtime/upp_runtime.c


# ---------------------------------------------------------------------------
# Kod üreteci
# ---------------------------------------------------------------------------

def load_runtime_c() -> str:
    here = os.path.dirname(os.path.abspath(__file__))
    candidates = [
        os.path.join(here, "src", "runtime", "upp_runtime.c"),
        os.path.join(here, "runtime", "upp_runtime.c"),
    ]
    for path in candidates:
        if os.path.isfile(path):
            with open(path, encoding="utf-8") as handle:
                text = handle.read()
            if text.strip():
                return text
    if RUNTIME_C.strip():
        return RUNTIME_C
    raise UppError(1, f"Çalışma zamanı dosyası bulunamadı: {candidates[0]}")

class CodeGenerator:
    def __init__(self) -> None:
        self.lines: List[str] = []
        self.indent = 0
        self.classes: Dict[str, ClassDecl] = {}
        self.functions: Dict[str, FunctionDecl] = {}
        self.scopes: List[Dict[str, TypeNode]] = [{}]
        self.current_class: Optional[str] = None
        self.current_fn: Optional[FunctionDecl] = None
        self._tmp = 0
        self.bg_funcs: List[List[str]] = []
        self._bg_id = 0
        self.background_depth = 0
        self.enums: Dict[str, EnumDecl] = {}
        self.target: str = host_hedef()
        self.modules: Dict[str, Dict[str, Any]] = {}
        self._kuresel_satir: List[str] = []

    def _cls_key(self, cls: ClassDecl) -> str:
        return f"{cls.mod_alias}.{cls.name}" if cls.mod_alias else cls.name

    def _cls_c(self, cls: ClassDecl) -> str:
        return c_symbol(cls.name, cls.mod_alias)

    def _enum_c(self, ed: EnumDecl) -> str:
        return c_symbol(ed.name, ed.mod_alias)

    def _fn_c(self, fn: FunctionDecl) -> str:
        if fn.class_name:
            simple = fn.class_name.split(".")[-1]
            return f"{c_symbol(simple, fn.mod_alias)}_{fn.name}"
        if fn.name == "ana" and not fn.mod_alias:
            return "main"
        return c_symbol(fn.name, fn.mod_alias)

    def _cls_type_name(self, fn: FunctionDecl) -> str:
        if not fn.class_name:
            return ""
        if "." in fn.class_name:
            return fn.class_name
        if fn.mod_alias:
            return f"{fn.mod_alias}.{fn.class_name}"
        return fn.class_name

    def generate(self, program: Program) -> str:
        self.lines = []
        self.indent = 0
        self.classes = make_builtin_classes()
        self.functions = {}
        self.scopes = [{}]
        self.current_class = None
        self.current_fn = None
        self.bg_funcs = []
        self._bg_id = 0
        self.background_depth = 0
        self.enums = {}
        self.modules = {}
        self._kuresel_satir = []
        includes: List[CInclude] = []
        class_decls: List[ClassDecl] = []
        func_decls: List[FunctionDecl] = []
        enum_decls: List[EnumDecl] = []
        global_decls: List[VarDecl] = []
        for decl in program.decls:
            if isinstance(decl, CInclude):
                includes.append(decl)
            elif isinstance(decl, EnumDecl):
                key = f"{decl.mod_alias}.{decl.name}" if decl.mod_alias else decl.name
                self.enums[key] = decl
                if decl.mod_alias:
                    self.modules.setdefault(decl.mod_alias, {"fn": {}, "class": {}, "enum": {}, "var": {}})["enum"][decl.name] = decl
                enum_decls.append(decl)
            elif isinstance(decl, ClassDecl):
                if decl.name in TYPE_NAMES or decl.name in ("upp", "ana", "main") or (decl.name in BUILTIN_CLASS_NAMES and not decl.mod_alias):
                    raise UppError(decl.line, f"'{decl.name}' sınıf adı olarak kullanılamaz.", decl.column)
                key = self._cls_key(decl)
                self.classes[key] = decl
                if decl.mod_alias:
                    self.modules.setdefault(decl.mod_alias, {"fn": {}, "class": {}, "enum": {}, "var": {}})["class"][decl.name] = decl
                class_decls.append(decl)
            elif isinstance(decl, FunctionDecl):
                cname = self._fn_c(decl)
                if cname in self.functions:
                    extra = ""
                    other = self.functions[cname]
                    if decl.kaynak or other.kaynak:
                        extra = f" ({other.kaynak or '?'} ve {decl.kaynak or '?'})"
                    raise UppError(decl.line, f"'{decl.name}' fonksiyonu birden fazla kez tanımlandı.{extra}", decl.column)
                self.functions[cname] = decl
                if not decl.mod_alias:
                    self.functions[decl.name] = decl
                else:
                    self.modules.setdefault(decl.mod_alias, {"fn": {}, "class": {}, "enum": {}, "var": {}})["fn"][decl.name] = decl
                func_decls.append(decl)
            elif isinstance(decl, VarDecl):
                if decl.mod_alias:
                    self.modules.setdefault(decl.mod_alias, {"fn": {}, "class": {}, "enum": {}, "var": {}})["var"][decl.name] = decl
                global_decls.append(decl)

        for cls in class_decls:
            for method in cls.methods:
                cname = f"{self._cls_c(cls)}_{method.name}"
                if cname in self.functions and self.functions[cname].class_name is None:
                    raise UppError(method.line, f"'{cname}' hem serbest fonksiyon hem metot olarak üretilemez.", method.column)

        if self.target == "linux":
            self._emit("#define UPP_HEDEF_LINUX 1")
            self._emit("#ifndef _DEFAULT_SOURCE")
            self._emit("#define _DEFAULT_SOURCE 1")
            self._emit("#endif")
            self._emit("#ifndef _POSIX_C_SOURCE")
            self._emit("#define _POSIX_C_SOURCE 200809L")
            self._emit("#endif")
        else:
            self._emit("#define UPP_HEDEF_WINDOWS 1")
        self._emit("#include <stdio.h>")
        self._emit("#include <stdlib.h>")
        self._emit("#include <stdbool.h>")
        self._emit("#include <string.h>")
        self._emit("#include <limits.h>")
        self._emit("#include <stdint.h>")
        self._emit("#include <math.h>")
        if self.target == "linux":
            self._emit("#include <pthread.h>")
            self._emit("#include <unistd.h>")
            self._emit("#include <time.h>")
            self._emit("#include <fcntl.h>")
            self._emit("#include <errno.h>")
            self._emit("#include <sys/types.h>")
        else:
            self._emit("#include <windows.h>")
            self._emit("#include <mmsystem.h>")
            self._emit("#include <tlhelp32.h>")
        for inc in includes:
            self._emit(c_include_line(inc.spec))
        self._emit("")
        for raw in load_runtime_c().strip("\n").splitlines():
            self._emit(raw)
        self._emit("")
        if enum_decls:
            self._emit("/* ---- u++ seçenekler (enum) ---- */")
            for ed in enum_decls:
                cn = self._enum_c(ed)
                self._emit(f"typedef enum {cn} {{")
                self.indent += 1
                for i, var in enumerate(ed.variants):
                    comma = "," if i < len(ed.variants) - 1 else ""
                    self._emit(f"{cn}_{var.name} = {var.value}{comma}")
                self.indent -= 1
                self._emit(f"}} {cn};")
                self._emit("")

        if class_decls:
            self._emit("/* ---- u++ sınıfları (struct) ---- */")
            for cls in class_decls:
                cn = self._cls_c(cls)
                self._emit(f"typedef struct {cn} {cn};")
            self._emit("")
            for cls in class_decls:
                cn = self._cls_c(cls)
                self._emit(f"struct {cn} {{")
                self.indent += 1
                if not cls.fields:
                    self._emit("char _u_bos;")
                else:
                    for field in cls.fields:
                        self._emit(self._gen_field(field) + ";")
                self.indent -= 1
                self._emit("};")
                self._emit("")
            self._emit("/* ---- u++ sınıf sabitleri ---- */")
            for cls in class_decls:
                for c in cls.constants:
                    self._emit("static " + self._gen_var_decl(c, as_const=True, c_name=f"{self._cls_c(cls)}_{c.name}") + ";")
            self._emit("")
            self._emit("/* ---- u++ metot prototipleri ---- */")
            for cls in class_decls:
                for method in cls.methods:
                    self._emit(self._prototype(method) + ";")
            self._emit("")

        self._emit("/* ---- u++ fonksiyon prototipleri ---- */")
        for decl in func_decls:
            self._emit(self._prototype(decl) + ";")
        self._emit("")
        if global_decls:
            self._emit("/* ---- u++ üst düzey değişkenler ---- */")
            for decl in global_decls:
                bind_name = c_symbol(decl.name, decl.mod_alias)
                self._bind(bind_name, self._decl_type(decl))
                if not decl.mod_alias:
                    self._bind(decl.name, self._decl_type(decl))
                self._emit("static " + self._gen_var_decl(decl, as_const=decl.is_const, c_name=bind_name, kuresel=True) + ";")
                if self._kilit_hazir_gerekli(decl):
                    self._kuresel_satir.append(f"upp_kilit_hazir(&{bind_name});")
            self._emit("static void _upp_kuresel_hazir(void) {")
            self.indent += 1
            for ln in self._kuresel_satir:
                self._emit(ln)
            if not self._kuresel_satir:
                self._emit("(void)0;")
            self.indent -= 1
            self._emit("}")
            self._emit("")
        else:
            self._emit("static void _upp_kuresel_hazir(void) {}")
            self._emit("")
        func_at = len(self.lines)
        for cls in class_decls:
            for method in cls.methods:
                self._gen_function(method)
        for decl in func_decls:
            self._gen_function(decl)
        if self.bg_funcs:
            user_fn = self.lines[func_at:]
            self.lines = self.lines[:func_at]
            self._emit("/* ---- u++ arkaplan iş parçacıkları ---- */")
            for chunk in self.bg_funcs:
                self.lines.extend(chunk)
                self.lines.append("")
            self.lines.extend(user_fn)
        return "\n".join(self.lines) + "\n"

    def _emit(self, text: str) -> None:
        if text == "":
            self.lines.append("")
            return
        self.lines.append(("    " * self.indent) + text)

    def _push_scope(self) -> None:
        self.scopes.append({})

    def _pop_scope(self) -> None:
        if len(self.scopes) > 1:
            self.scopes.pop()

    def _bind(self, name: str, typ: TypeNode) -> None:
        self.scopes[-1][name] = typ

    def _lookup(self, name: str, node: Node) -> TypeNode:
        for scope in reversed(self.scopes):
            if name in scope:
                return scope[name]
        raise UppError(node.line, f"Tanımsız ad: '{name}'.", node.column)

    def _decl_type(self, decl: VarDecl) -> TypeNode:
        dims = decl.type.array_dims
        if decl.array_size is not None or decl.unsized_array:
            dims = 1
        return TypeNode(decl.line, decl.column, decl.type.name, decl.type.pointer_depth, dims)

    def _ctype(self, t: TypeNode) -> str:
        if t.name.startswith("secenek."):
            raise UppError(t.line, f"'{t.name[8:]}' bir seçenek türüdür; değer için '{t.name[8:]}.üye' yazın.", t.column)
        if is_liste_type_name(t.name):
            base = "UppKolListe"
        elif is_harita_type_name(t.name):
            base = "UppKolHarita"
        elif t.name in C_TYPE:
            base = C_TYPE[t.name]
        elif t.name in self.enums:
            base = self._enum_c(self.enums[t.name])
        elif t.name in self.classes:
            base = self._cls_c(self.classes[t.name])
        elif "." in t.name:
            base = type_c_name(t.name)
        else:
            raise UppError(t.line, f"Bilinmeyen tür: '{t.name}'.", t.column)
        if t.pointer_depth:
            return base + ("*" * t.pointer_depth)
        return base

    def _c_ident(self, name: str) -> str:
        return "main" if name == "ana" else name

    def _prototype(self, decl: FunctionDecl) -> str:
        if decl.class_name:
            ret = self._ctype(decl.return_type)
            simple = decl.class_name.split(".")[-1]
            cls_c = c_symbol(simple, decl.mod_alias)
            params = [f"{cls_c}* kendi"]
            params.extend(f"{self._ctype(p.type)} {p.name}" for p in decl.params if p.type)
            return f"{ret} {cls_c}_{decl.name}({', '.join(params)})"
        if decl.name == "ana" and not decl.mod_alias:
            return "int main(int argc, char **argv)"
        ret = self._ctype(decl.return_type)
        params = ", ".join(f"{self._ctype(p.type)} {p.name}" for p in decl.params if p.type)
        if not params:
            params = "void"
        return f"{ret} {self._fn_c(decl)}({params})"

    def _gen_field(self, field: FieldDecl) -> str:
        ctype = self._ctype(field.type)
        name = field.name
        if field.array_size is not None:
            name = f"{name}[{self._gen_expr(field.array_size)}]"
        elif field.unsized_array:
            raise UppError(field.line, f"Sınıf alanı '{field.name}' boyutsuz dizi olamaz.", field.column)
        return f"{ctype} {name}"

    def _gen_function(self, decl: FunctionDecl) -> None:
        reserved = {"upp", "printf", "main"}
        if decl.class_name is None and decl.name in reserved and decl.name != "ana":
            raise UppError(decl.line, f"'{decl.name}' yerleşik bir addır; yeniden tanımlanamaz.", decl.column)
        self.current_class = self._cls_type_name(decl) if decl.class_name else None
        self.current_fn = decl
        self._push_scope()
        if decl.class_name:
            self._bind("kendi", TypeNode(decl.line, decl.column, self.current_class, 1, 0))
        for param in decl.params:
            if param.type:
                self._bind(param.name, TypeNode(param.line, param.column, param.type.name, param.type.pointer_depth, 0))
        self._emit(self._prototype(decl) + " {")
        self.indent += 1
        if decl.name == "ana" and decl.class_name is None:
            self._emit("upp_argv_ayarla(argc, argv);")
            self._emit("_upp_kuresel_hazir();")
            if self.target != "linux":
                self._emit("SetConsoleOutputCP(CP_UTF8);")
                self._emit("SetConsoleCP(CP_UTF8);")
        if decl.body:
            for stmt in decl.body.stmts:
                self._gen_stmt(stmt)
        self.indent -= 1
        self._emit("}")
        self._emit("")
        self._pop_scope()
        self.current_class = None
        self.current_fn = None

    def _gen_stmt(self, stmt: Optional[Stmt]) -> None:
        if stmt is None or isinstance(stmt, EmptyStmt):
            self._emit(";")
            return
        if isinstance(stmt, Block):
            self._push_scope()
            self._emit("{")
            self.indent += 1
            for s in stmt.stmts:
                self._gen_stmt(s)
            self.indent -= 1
            self._emit("}")
            self._pop_scope()
            return
        if isinstance(stmt, UnsafeBlock):
            self._push_scope()
            self._emit("/* guvensiz */ {")
            self.indent += 1
            if stmt.body:
                for s in stmt.body.stmts:
                    self._gen_stmt(s)
            self.indent -= 1
            self._emit("}")
            self._pop_scope()
            return
        if isinstance(stmt, SafeBlock):
            self._push_scope()
            self._emit("/* guvenli */ {")
            self.indent += 1
            if stmt.body:
                for s in stmt.body.stmts:
                    self._gen_stmt(s)
            self.indent -= 1
            self._emit("}")
            self._pop_scope()
            return
        if isinstance(stmt, BackgroundBlock):
            self._gen_background(stmt)
            return
        if isinstance(stmt, CEmbed):
            self._gen_c_embed(stmt)
            return
        if isinstance(stmt, AsmEmbed):
            self._gen_asm_embed(stmt)
            return
        if isinstance(stmt, VarDecl):
            self._bind(stmt.name, self._decl_type(stmt))
            self._emit(self._gen_var_decl(stmt, as_const=stmt.is_const) + ";")
            if self._kilit_hazir_gerekli(stmt):
                self._emit(f"upp_kilit_hazir(&{stmt.name});")
            return
        if isinstance(stmt, IfStmt):
            self._emit(f"if ({self._gen_expr(stmt.cond)})")
            self._gen_stmt_body(stmt.then_branch)
            if stmt.else_branch is not None:
                self._emit("else")
                self._gen_stmt_body(stmt.else_branch)
            return
        if isinstance(stmt, WhileStmt):
            self._emit(f"while ({self._gen_expr(stmt.cond)})")
            self._gen_stmt_body(stmt.body)
            return
        if isinstance(stmt, ForStmt):
            self._push_scope()
            init = ""
            if isinstance(stmt.init, VarDecl):
                self._bind(stmt.init.name, self._decl_type(stmt.init))
                init = self._gen_var_decl(stmt.init)
            elif isinstance(stmt.init, ExprStmt) and stmt.init.expr is not None:
                init = self._gen_expr(stmt.init.expr)
            cond = self._gen_expr(stmt.cond) if stmt.cond is not None else ""
            step = self._gen_expr(stmt.step) if stmt.step is not None else ""
            self._emit(f"for ({init}; {cond}; {step})")
            self._gen_stmt_body(stmt.body)
            self._pop_scope()
            return
        if isinstance(stmt, SwitchStmt):
            sw_t = self._expr_type(stmt.expr)
            if type_is_metin(sw_t):
                self._tmp += 1
                swv = f"_upp_sw{self._tmp}"
                self._emit("{")
                self.indent += 1
                self._emit(f"char* {swv} = {self._gen_expr(stmt.expr)};")
                first = True
                for case in stmt.cases:
                    if case.value is None:
                        self._emit("else")
                    else:
                        cond = f"upp_metin_esit({swv}, {self._gen_expr(case.value)})"
                        self._emit(("if " if first else "else if ") + f"({cond})")
                        first = False
                    self._emit("{")
                    self.indent += 1
                    if not case.stmts:
                        self._emit(";")
                    for s in case.stmts:
                        if isinstance(s, BreakStmt):
                            continue
                        self._gen_stmt(s)
                    self.indent -= 1
                    self._emit("}")
                self.indent -= 1
                self._emit("}")
                return
            self._emit(f"switch ({self._gen_expr(stmt.expr)}) {{")
            self.indent += 1
            for case in stmt.cases:
                if case.value is None:
                    self._emit("default:")
                else:
                    self._emit(f"case {self._gen_expr(case.value)}:")
                self.indent += 1
                if not case.stmts:
                    self._emit(";")
                for s in case.stmts:
                    self._gen_stmt(s)
                self.indent -= 1
            self.indent -= 1
            self._emit("}")
            return
        if isinstance(stmt, ForEachStmt):
            self._tmp += 1
            idx = f"_upp_i{self._tmp}"
            elem_t = stmt.var_type
            it_t = self._expr_type(stmt.iterable)
            self._push_scope()
            self._bind(stmt.var_name, elem_t)
            self._emit("{")
            self.indent += 1
            self._emit(f"long long {idx};")
            keys = None
            if type_is_liste(it_t):
                arr = self._gen_expr(stmt.iterable)
                elem_name = parse_liste_elem(it_t.name) or "sayi"
                self._emit(f"for ({idx} = 0; {idx} < upp_liste_uzunluk({arr}); {idx}++) {{")
                self.indent += 1
                self._emit(f"{self._ctype(elem_t)} {stmt.var_name} = {self._liste_al_c(elem_name, arr, idx)};")
            elif type_is_harita(it_t):
                arr = self._gen_expr(stmt.iterable)
                kv = parse_harita_kv(it_t.name)
                ktur = kv[0] if kv else "metin"
                ktag = koleksiyon_tag(ktur)
                self._tmp += 1
                keys = f"_upp_hk{self._tmp}"
                self._emit(f"UppKolListe {keys} = upp_harita_anahtarlar({arr}, {ktag});")
                self._emit(f"for ({idx} = 0; {idx} < upp_liste_uzunluk({keys}); {idx}++) {{")
                self.indent += 1
                self._emit(f"{self._ctype(elem_t)} {stmt.var_name} = {self._liste_al_c(ktur, keys, idx)};")
            elif isinstance(stmt.iterable, ArrayLiteral):
                self._tmp += 1
                tmpa = f"_upp_arr{self._tmp}"
                self._emit(f"{self._ctype(TypeNode(stmt.line, stmt.column, elem_t.name, elem_t.pointer_depth, 0))} {tmpa}[] = {{{', '.join(self._gen_expr(e) for e in stmt.iterable.elements)}}};")
                self._emit(f"for ({idx} = 0; {idx} < (long long)(sizeof({tmpa}) / sizeof(({tmpa})[0])); {idx}++) {{")
                self.indent += 1
                self._emit(f"{self._ctype(elem_t)} {stmt.var_name} = {tmpa}[{idx}];")
            else:
                arr = self._gen_expr(stmt.iterable)
                self._emit(f"for ({idx} = 0; {idx} < (long long)(sizeof({arr}) / sizeof(({arr})[0])); {idx}++) {{")
                self.indent += 1
                self._emit(f"{self._ctype(elem_t)} {stmt.var_name} = {arr}[{idx}];")
            self._gen_stmt_body(stmt.body)
            self.indent -= 1
            self._emit("}")
            if keys is not None:
                self._emit(f"upp_liste_bosalt(&({keys}));")
            self.indent -= 1
            self._emit("}")
            self._pop_scope()
            return
        if isinstance(stmt, ReturnStmt):
            if self.background_depth > 0:
                self._emit("return 0;")
                return
            in_ana = self.current_fn is not None and self.current_fn.name == "ana" and self.current_fn.class_name is None
            if stmt.value is None:
                self._emit("return 0;" if in_ana else "return;")
            elif in_ana:
                self._emit(f"return (int)({self._gen_expr(stmt.value)});")
            elif self.current_fn is not None and type_is_void(self.current_fn.return_type):
                self._emit(self._gen_expr(stmt.value) + ";")
            else:
                self._emit(f"return {self._gen_expr(stmt.value)};")
            return
        if isinstance(stmt, BreakStmt):
            self._emit("break;")
            return
        if isinstance(stmt, ContinueStmt):
            self._emit("continue;")
            return
        if isinstance(stmt, ExprStmt):
            self._emit(self._gen_expr(stmt.expr) + ";")
            return
        raise UppError(getattr(stmt, "line", 1), f"Üretilemeyen deyim: {type(stmt).__name__}")

    def _collect_idents(self, node: Optional[Node], declared: set, used: List[str]) -> None:
        if node is None:
            return
        if isinstance(node, VarDecl):
            self._collect_idents(node.init, declared, used)
            self._collect_idents(node.array_size, declared, used)
            declared.add(node.name)
            return
        if isinstance(node, Ident):
            if node.name in TYPE_NAMES or node.name == "upp" or node.name in self.enums:
                return
            if node.name in declared:
                return
            if node.name not in used:
                used.append(node.name)
            return
        if isinstance(node, Literal) and node.kind == "metin":
            parsed = parse_interpolation(node.value, node.line, node.column)
            if parsed is not None:
                for kind, payload in flatten_interp_parts(parsed):
                    if kind == "expr":
                        self._collect_idents(payload, declared, used)
            return
        if isinstance(node, BackgroundBlock) or isinstance(node, BackgroundExpr):
            inner_decl = set(declared)
            self._collect_idents(node.body, inner_decl, used)
            return
        for child in getattr(node, "__dict__", {}).values():
            if isinstance(child, Node):
                self._collect_idents(child, declared, used)
            elif isinstance(child, list):
                for item in child:
                    if isinstance(item, Node):
                        self._collect_idents(item, declared, used)

    def _capture_ctype(self, t: TypeNode) -> str:
        if t.array_dims > 0:
            inner = TypeNode(t.line, t.column, t.name, t.pointer_depth, 0)
            return self._ctype(inner) + "*"
        return self._ctype(t)

    def _bg_captures(self, body: Optional[Block], line: int, column: int) -> List[Tuple[str, TypeNode]]:
        used: List[str] = []
        self._collect_idents(body, set(), used)
        caps: List[Tuple[str, TypeNode]] = []
        dummy = Ident(line, column, "")
        for name in used:
            dummy.name = name
            try:
                if name == "kendi":
                    if not self.current_class:
                        continue
                    typ = TypeNode(line, column, self.current_class, 1, 0)
                else:
                    typ = self._lookup(name, dummy)
            except UppError:
                continue
            if typ.array_dims > 0:
                raise UppError(
                    line,
                    f"'arkaplan' dizi '{name}' yakalayamaz. Elemanı kopyalayın veya diziyi iş parçacığı içinde oluşturun.",
                    column,
                )
            if is_liste_type_name(typ.name) or is_harita_type_name(typ.name):
                raise UppError(
                    line,
                    f"'arkaplan' '{typ.name}' '{name}' yakalayamaz. Koleksiyonu iş parçacığı içinde oluşturun.",
                    column,
                )
            if typ.name == "JSONDeger":
                raise UppError(
                    line,
                    f"'arkaplan' JSONDeger '{name}' yakalayamaz. Değeri iş parçacığı içinde okuyun.",
                    column,
                )
            caps.append((name, typ))
        return caps

    def _bg_capture_zero(self, typ: TypeNode) -> str:
        if typ.pointer_depth > 0 or type_is_metin(typ):
            return "NULL"
        if typ.name == "mantik":
            return "false"
        if typ.name == "ondalik":
            return "0.0"
        if is_liste_type_name(typ.name) or is_harita_type_name(typ.name):
            return "{0}"
        if typ.name in self.classes or typ.name in BUILTIN_CLASS_NAMES:
            return "{0}"
        return "0"

    def _bg_copy_field(self, dest: str, name: str, typ: TypeNode) -> str:
        if type_is_metin(typ):
            return f"{dest}{name} = upp_metin_kopya({name});"
        return f"{dest}{name} = {name};"

    def _bg_metin_kopya_hata_kosul(self, dest: str, caps: List[Tuple[str, TypeNode]]) -> str:
        parts = [f"({name} && !{dest}{name})" for name, typ in caps if type_is_metin(typ)]
        return " || ".join(parts)

    def _bg_free_metin_ctx(self, dest: str, caps: List[Tuple[str, TypeNode]]) -> List[str]:
        lines: List[str] = []
        for name, typ in caps:
            if type_is_metin(typ):
                lines.append(f"if ({dest}{name}) free({dest}{name});")
        return lines

    def _build_bg_worker(self, body: Optional[Block], caps: List[Tuple[str, TypeNode]]) -> Tuple[str, str, List[str]]:
        self._bg_id += 1
        nid = self._bg_id
        fn = f"_upp_bg_{nid}"
        ctx = f"_upp_bgctx_{nid}"
        chunk: List[str] = []
        if caps:
            chunk.append(f"struct {ctx} {{")
            for name, typ in caps:
                chunk.append(f"    {self._capture_ctype(typ)} {name};")
            chunk.append("};")
        if self.target == "linux":
            chunk.append(f"static void* {fn}(void* _p) {{")
        else:
            chunk.append(f"static DWORD WINAPI {fn}(LPVOID _p) {{")
        if caps:
            chunk.append(f"    struct {ctx} _c;")
            for name, typ in caps:
                chunk.append(f"    {self._capture_ctype(typ)} {name} = {self._bg_capture_zero(typ)};")
            chunk.append("    if (_p) {")
            chunk.append(f"        _c = *(struct {ctx}*)_p;")
            chunk.append("        free(_p);")
            for name, _typ in caps:
                chunk.append(f"        {name} = _c.{name};")
            chunk.append("    }")
        saved_lines = self.lines
        saved_indent = self.indent
        self.lines = []
        self.indent = 1
        self._push_scope()
        for name, typ in caps:
            self._bind(name, typ)
        self.background_depth += 1
        if body:
            for s in body.stmts:
                self._gen_stmt(s)
        self.background_depth -= 1
        self._pop_scope()
        body_lines = self.lines
        self.lines = saved_lines
        self.indent = saved_indent
        chunk.append("    {")
        for ln in body_lines:
            chunk.append(ln if ln == "" else "    " + ln)
        chunk.append("    }")
        for name, typ in caps:
            if type_is_metin(typ):
                chunk.append(f"    if ({name}) {{ free({name}); {name} = NULL; }}")
        chunk.append("    return 0;")
        chunk.append("}")
        return fn, ctx, chunk

    def _gen_background(self, stmt: BackgroundBlock) -> None:
        caps = self._bg_captures(stmt.body, stmt.line, stmt.column)
        fn, ctx, chunk = self._build_bg_worker(stmt.body, caps)
        self.bg_funcs.append(chunk)
        self._emit("{")
        self.indent += 1
        if caps:
            self._emit(f"struct {ctx}* _ctx = (struct {ctx}*)malloc(sizeof(struct {ctx}));")
            self._emit("if (_ctx) {")
            self.indent += 1
            for name, typ in caps:
                self._emit(self._bg_copy_field("_ctx->", name, typ))
            kopya_hata = self._bg_metin_kopya_hata_kosul("_ctx->", caps)
            if kopya_hata:
                self._emit(f"if ({kopya_hata}) {{")
                self.indent += 1
                self._emit('fprintf(stderr, "[u++] arkaplan: bellek yetersiz.\\n");')
                for ln in self._bg_free_metin_ctx("_ctx->", caps):
                    self._emit(ln)
                self._emit("free(_ctx);")
                self.indent -= 1
                self._emit("} else {")
                self.indent += 1
            if self.target == "linux":
                self._emit(f"pthread_t _th; if (pthread_create(&_th, NULL, {fn}, _ctx) == 0) {{ pthread_detach(_th); }} else {{")
            else:
                self._emit(f"HANDLE _th = CreateThread(NULL, 0, {fn}, _ctx, 0, NULL);")
                self._emit("if (_th) { CloseHandle(_th); } else {")
            self.indent += 1
            self._emit('fprintf(stderr, "[u++] arkaplan: is parcacigi baslatilamadi.\\n");')
            for ln in self._bg_free_metin_ctx("_ctx->", caps):
                self._emit(ln)
            self._emit("free(_ctx);")
            self.indent -= 1
            self._emit("}")
            if kopya_hata:
                self.indent -= 1
                self._emit("}")
            self.indent -= 1
            self._emit("} else {")
            self.indent += 1
            self._emit('fprintf(stderr, "[u++] arkaplan: bellek yetersiz.\\n");')
            self.indent -= 1
            self._emit("}")
        else:
            if self.target == "linux":
                self._emit(f"pthread_t _th; if (pthread_create(&_th, NULL, {fn}, NULL) == 0) {{ pthread_detach(_th); }} else {{")
            else:
                self._emit(f"HANDLE _th = CreateThread(NULL, 0, {fn}, NULL, 0, NULL);")
                self._emit("if (_th) { CloseHandle(_th); } else {")
            self.indent += 1
            self._emit('fprintf(stderr, "[u++] arkaplan: is parcacigi baslatilamadi.\\n");')
            self.indent -= 1
            self._emit("}")
        self.indent -= 1
        self._emit("}")

    def _gen_background_expr(self, expr: BackgroundExpr) -> str:
        caps = self._bg_captures(expr.body, expr.line, expr.column)
        fn, ctx, chunk = self._build_bg_worker(expr.body, caps)
        spawn = f"_upp_spawn_{self._bg_id}"
        if caps:
            params = ", ".join(f"{self._capture_ctype(typ)} {name}" for name, typ in caps)
            call_args = ", ".join(name for name, _typ in caps)
        else:
            params = "void"
            call_args = ""
        spawn_lines = [
            f"static ArkaplanIs {spawn}({params}) {{",
            "    ArkaplanIs _j;",
            "    _j.handle = 0;",
        ]
        if caps:
            spawn_lines.append(f"    struct {ctx}* _ctx = (struct {ctx}*)malloc(sizeof(struct {ctx}));")
            spawn_lines.append("    if (!_ctx) {")
            spawn_lines.append('        fprintf(stderr, "[u++] arkaplan: bellek yetersiz.\\n");')
            spawn_lines.append("        return _j;")
            spawn_lines.append("    }")
            for name, typ in caps:
                spawn_lines.append(f"    {self._bg_copy_field('_ctx->', name, typ)}")
            kopya_hata = self._bg_metin_kopya_hata_kosul("_ctx->", caps)
            if kopya_hata:
                spawn_lines.append(f"    if ({kopya_hata}) {{")
                spawn_lines.append('        fprintf(stderr, "[u++] arkaplan: bellek yetersiz.\\n");')
                for ln in self._bg_free_metin_ctx("_ctx->", caps):
                    spawn_lines.append(f"        {ln}")
                spawn_lines.append("        free(_ctx);")
                spawn_lines.append("        return _j;")
                spawn_lines.append("    }")
            if self.target == "linux":
                spawn_lines.append(f"    {{ pthread_t _th; if (pthread_create(&_th, NULL, {fn}, _ctx) == 0) {{ _j.handle = (long long)(uintptr_t)_th; }} else {{")
            else:
                spawn_lines.append(f"    {{ HANDLE _th = CreateThread(NULL, 0, {fn}, _ctx, 0, NULL);")
                spawn_lines.append("      if (_th) { _j.handle = (long long)(intptr_t)_th; } else {")
            spawn_lines.append('        fprintf(stderr, "[u++] arkaplan: is parcacigi baslatilamadi.\\n");')
            for ln in self._bg_free_metin_ctx("_ctx->", caps):
                spawn_lines.append(f"        {ln}")
            spawn_lines.append("        free(_ctx); } }")
        else:
            if self.target == "linux":
                spawn_lines.append(f"    {{ pthread_t _th; if (pthread_create(&_th, NULL, {fn}, NULL) == 0) {{ _j.handle = (long long)(uintptr_t)_th; }} else {{")
            else:
                spawn_lines.append(f"    {{ HANDLE _th = CreateThread(NULL, 0, {fn}, NULL, 0, NULL);")
                spawn_lines.append("      if (_th) { _j.handle = (long long)(intptr_t)_th; } else {")
            spawn_lines.append('        fprintf(stderr, "[u++] arkaplan: is parcacigi baslatilamadi.\\n"); } }')
        spawn_lines.append("    return _j;")
        spawn_lines.append("}")
        chunk.extend(spawn_lines)
        self.bg_funcs.append(chunk)
        return f"{spawn}({call_args})"

    def _gen_stmt_body(self, stmt: Optional[Stmt]) -> None:
        if isinstance(stmt, (Block, UnsafeBlock, SafeBlock, BackgroundBlock, SwitchStmt, ForEachStmt)):
            self._gen_stmt(stmt)
            return
        self.indent += 1
        self._gen_stmt(stmt)
        self.indent -= 1

    def _gen_var_decl(self, decl: VarDecl, as_const: bool = False, c_name: Optional[str] = None, kuresel: bool = False) -> str:
        ctype = self._ctype(decl.type)
        if as_const or decl.is_const:
            ctype = f"const {ctype}"
        name = c_name or decl.name
        if decl.array_size is not None:
            name = f"{name}[{self._gen_expr(decl.array_size)}]"
        elif decl.unsized_array or (decl.inferred and decl.type.array_dims):
            name = f"{name}[]"
        text = f"{ctype} {name}"
        init = decl.init
        metin_kopya = (
            decl.type.name == "metin"
            and decl.type.pointer_depth == 0
            and not decl.is_const
            and init is not None
        )
        if kuresel and metin_kopya:
            text += " = NULL"
            self._kuresel_satir.append(f"{c_name or decl.name} = upp_metin_kopya({self._gen_expr(init)});")
        elif init is not None:
            if isinstance(init, ArrayLiteral):
                elems = ", ".join(self._gen_expr(e) for e in init.elements)
                text += f" = {{{elems}}}"
            elif metin_kopya and isinstance(init, Literal) and init.kind == "metin":
                text += f" = upp_metin_kopya({self._gen_expr(init)})"
            else:
                text += f" = {self._gen_expr(init)}"
        elif decl.array_size is not None or decl.unsized_array or (decl.inferred and decl.type.array_dims):
            text += " = {0}"
        elif decl.type.pointer_depth > 0 or decl.type.name == "metin":
            text += " = NULL"
        elif is_liste_type_name(decl.type.name) or is_harita_type_name(decl.type.name):
            text += " = {0}"
        elif decl.type.name in self.classes and decl.type.pointer_depth == 0:
            text += " = {0}"
        elif "." in decl.type.name and decl.type.pointer_depth == 0:
            text += " = {0}"
        elif decl.type.name in self.enums:
            text += " = 0"
        return text

    def _kilit_hazir_gerekli(self, decl: VarDecl) -> bool:
        return (
            decl.type.name == "Kilit"
            and decl.type.pointer_depth == 0
            and decl.init is None
            and decl.array_size is None
            and not decl.unsized_array
            and not decl.is_const
        )

    def _dedent_embed(self, code: str) -> str:
        lines = code.replace("\r\n", "\n").replace("\r", "\n").split("\n")
        while lines and not lines[0].strip():
            lines.pop(0)
        while lines and not lines[-1].strip():
            lines.pop()
        indents = [len(ln) - len(ln.lstrip(" \t")) for ln in lines if ln.strip()]
        pad = min(indents) if indents else 0
        return "\n".join(ln[pad:] if len(ln) >= pad else ln for ln in lines)

    def _gen_c_embed(self, stmt: CEmbed) -> None:
        body = self._dedent_embed(stmt.code)
        self._emit("/* c_kod */")
        if not body.strip():
            return
        for line in body.split("\n"):
            stripped = line.lstrip()
            if stripped.startswith("#"):
                raise UppError(
                    stmt.line,
                    "c_kod içinde önişlemci yönergesi yok. Dosya başına c_ekle yazın.",
                    stmt.column,
                )
            self._emit(line)

    def _c_escape(self, text: str) -> str:
        return text.replace("\\", "\\\\").replace('"', '\\"')

    def _gen_asm_embed(self, stmt: AsmEmbed) -> None:
        body = self._dedent_embed(stmt.code)
        self._emit("/* asm_kod */")
        if not body.strip():
            self._emit('__asm__ __volatile__("");')
            return
        if '"' in body or ":" in body:
            self._emit(f"__asm__ __volatile__({body.strip()});")
            return
        instrs = []
        for raw_line in body.split("\n"):
            line = raw_line.strip()
            if not line or line.startswith("//") or line.startswith("#"):
                continue
            if line.endswith(";"):
                line = line[:-1].strip()
            instrs.append(line)
        if not instrs:
            self._emit('__asm__ __volatile__("");')
            return
        pieces = ", ".join(f'"{self._c_escape(ins)}\\n"' for ins in instrs)
        self._emit(f"__asm__ __volatile__({pieces});")

    def _class_field(self, cls: ClassDecl, name: str) -> Optional[FieldDecl]:
        for field in cls.fields:
            if field.name == name:
                return field
        return None

    def _class_method(self, cls: ClassDecl, name: str) -> Optional[FunctionDecl]:
        for method in cls.methods:
            if method.name == name:
                return method
        return None

    def _cg_member_type(self, expr: Member) -> TypeNode:
        obj_t = self._expr_type(expr.obj)
        if obj_t.name.startswith("secenek."):
            return TypeNode(expr.line, expr.column, obj_t.name[8:], 0, 0)
        ns_t = upp_ns_type(obj_t, expr.name, expr.line, expr.column)
        if obj_t.name == "upp" or obj_t.name.startswith("upp."):
            if ns_t is None:
                prefix = obj_t.name if obj_t.name != "upp" else "upp"
                raise UppError(expr.line, f"'{prefix}.{expr.name}' yerleşik bir üye değil.", expr.column)
            return ns_t
        if obj_t.name.startswith("modul."):
            alias = obj_t.name[6:]
            bag = self.modules.get(alias) or {}
            if expr.name in bag.get("fn", {}):
                fn = bag["fn"][expr.name]
                return TypeNode(fn.line, fn.column, fn.return_type.name, fn.return_type.pointer_depth, 0)
            if expr.name in bag.get("class", {}):
                key = f"{alias}.{expr.name}"
                return TypeNode(expr.line, expr.column, f"sinif.{key}", 0, 0)
            if expr.name in bag.get("enum", {}):
                key = f"{alias}.{expr.name}"
                return TypeNode(expr.line, expr.column, f"secenek.{key}", 0, 0)
            if expr.name in bag.get("var", {}):
                vd = bag["var"][expr.name]
                return TypeNode(vd.line, vd.column, vd.type.name, vd.type.pointer_depth, vd.type.array_dims)
            raise UppError(expr.line, f"'{alias}' modülünde '{expr.name}' yok.", expr.column)
        if obj_t.name.startswith("sinif."):
            key = obj_t.name[6:]
            cls = self.classes.get(key)
            if cls is None:
                raise UppError(expr.line, f"'{key}' bir sınıf değil.", expr.column)
            for c in cls.constants:
                if c.name == expr.name:
                    return TypeNode(c.line, c.column, c.type.name, 0, 0)
            raise UppError(expr.line, f"'{cls.name}' sınıfında '{expr.name}' sabiti yok.", expr.column)
        if type_is_liste(obj_t):
            elem = parse_liste_elem(obj_t.name) or "sayi"
            if expr.name == "al":
                return TypeNode(expr.line, expr.column, elem, 0, 0)
            if expr.name == "uzunluk":
                return TypeNode(expr.line, expr.column, "sayi", 0, 0)
            return TypeNode(expr.line, expr.column, "bos", 0, 0)
        if type_is_harita(obj_t):
            kv = parse_harita_kv(obj_t.name)
            val = kv[1] if kv else "sayi"
            key = kv[0] if kv else "metin"
            if expr.name == "al":
                return TypeNode(expr.line, expr.column, val, 0, 0)
            if expr.name == "var_mi":
                return TypeNode(expr.line, expr.column, "mantik", 0, 0)
            if expr.name == "uzunluk":
                return TypeNode(expr.line, expr.column, "sayi", 0, 0)
            if expr.name == "anahtarlar":
                return TypeNode(expr.line, expr.column, f"liste[{key}]", 0, 0)
            return TypeNode(expr.line, expr.column, "bos", 0, 0)
        cls = self.classes.get(obj_t.name)
        if cls is None:
            raise UppError(expr.line, f"'{obj_t.name}' bir sınıf değil; '.' üye erişimi yok.", expr.column)
        field = self._class_field(cls, expr.name)
        if field is not None:
            dims = 1 if (field.array_size is not None or field.unsized_array) else 0
            return TypeNode(field.line, field.column, field.type.name, field.type.pointer_depth, dims)
        method = self._class_method(cls, expr.name)
        if method is not None:
            return TypeNode(method.line, method.column, method.return_type.name, method.return_type.pointer_depth, 0)
        if cls.name == "ArkaplanIs" and expr.name == "bekle":
            return TypeNode(expr.line, expr.column, "bos", 0, 0)
        if cls.name == "Kilit" and expr.name in ("kilitle", "birak"):
            return TypeNode(expr.line, expr.column, "bos", 0, 0)
        raise UppError(expr.line, f"'{cls.name}' sınıfında '{expr.name}' üyesi yok.", expr.column)

    def _expr_type(self, expr: Optional[Expr]) -> TypeNode:
        if expr is None:
            return TypeNode(0, 1, "bos", 0, 0)
        if isinstance(expr, Literal):
            if expr.kind == "yok":
                return TypeNode(expr.line, expr.column, "yok", 1, 0)
            return TypeNode(expr.line, expr.column, expr.kind, 0, 0)
        if isinstance(expr, Ident):
            if expr.name == "kendi":
                if not self.current_class:
                    raise UppError(expr.line, "'kendi' yalnızca sınıf metodu içinde kullanılabilir.", expr.column)
                return TypeNode(expr.line, expr.column, self.current_class, 1, 0)
            if expr.name == "upp":
                return TypeNode(expr.line, expr.column, "upp", 0, 0)
            if expr.name in self.modules:
                return TypeNode(expr.line, expr.column, f"modul.{expr.name}", 0, 0)
            if expr.name in self.classes:
                return TypeNode(expr.line, expr.column, f"sinif.{expr.name}", 0, 0)
            if expr.name in self.enums:
                return TypeNode(expr.line, expr.column, f"secenek.{expr.name}", 0, 0)
            return self._lookup(expr.name, expr)
        if isinstance(expr, Member):
            return self._cg_member_type(expr)
        if isinstance(expr, Index):
            inner = self._expr_type(expr.array)
            if type_is_liste(inner):
                elem = parse_liste_elem(inner.name) or "sayi"
                return TypeNode(expr.line, expr.column, elem, 0, 0)
            if inner.array_dims > 0:
                return TypeNode(inner.line, inner.column, inner.name, inner.pointer_depth, inner.array_dims - 1)
            if inner.pointer_depth > 0:
                return TypeNode(inner.line, inner.column, inner.name, inner.pointer_depth - 1, 0)
            if type_is_metin(inner):
                return TypeNode(expr.line, expr.column, "sayi", 0, 0)
            return TypeNode(inner.line, inner.column, inner.name, 0, 0)
        if isinstance(expr, Call):
            if isinstance(expr.callee, Member):
                return self._cg_member_type(expr.callee)
            if isinstance(expr.callee, Ident):
                if expr.callee.name in self.classes:
                    cls = self.classes[expr.callee.name]
                    key = self._cls_key(cls)
                    return TypeNode(expr.line, expr.column, key, 0, 0)
                fn = self.functions.get(expr.callee.name)
                if fn is not None:
                    return TypeNode(fn.line, fn.column, fn.return_type.name, fn.return_type.pointer_depth, 0)
            return TypeNode(expr.line, expr.column, "bos", 0, 0)
        if isinstance(expr, Assign):
            return self._expr_type(expr.value)
        if isinstance(expr, TernaryOp):
            then_t = self._expr_type(expr.then_expr)
            else_t = self._expr_type(expr.else_expr)
            if then_t.name == "ondalik" or else_t.name == "ondalik":
                return TypeNode(expr.line, expr.column, "ondalik", 0, 0)
            return then_t
        if isinstance(expr, UnaryOp):
            inner = self._expr_type(expr.operand)
            if expr.op == "&":
                return TypeNode(inner.line, inner.column, inner.name, inner.pointer_depth + 1, inner.array_dims)
            if expr.op == "*":
                depth = inner.pointer_depth - 1 if inner.pointer_depth > 0 else 0
                return TypeNode(inner.line, inner.column, inner.name, depth, 0)
            return inner
        if isinstance(expr, BinaryOp):
            left = self._expr_type(expr.left)
            right = self._expr_type(expr.right)
            if expr.op in ("==", "!=", "<", ">", "<=", ">=", "&&", "||"):
                return TypeNode(expr.line, expr.column, "mantik", 0, 0)
            if expr.op == "+" and type_is_metin(left) and type_is_metin(right):
                return TypeNode(expr.line, expr.column, "metin", 0, 0)
            if left.name == "ondalik" or right.name == "ondalik":
                return TypeNode(expr.line, expr.column, "ondalik", 0, 0)
            return left
        if isinstance(expr, ArrayLiteral):
            if expr.elements:
                inner = self._expr_type(expr.elements[0])
                return TypeNode(inner.line, inner.column, inner.name, inner.pointer_depth, inner.array_dims + 1)
            return TypeNode(expr.line, expr.column, "sayi", 0, 1)
        if isinstance(expr, BackgroundExpr):
            return TypeNode(expr.line, expr.column, "ArkaplanIs", 0, 0)
        return TypeNode(getattr(expr, "line", 0), getattr(expr, "column", 1), "bos", 0, 0)

    def _printf_spec(self, typ: TypeNode) -> str:
        if typ.name == "sayi":
            return "%lld"
        if typ.name == "ondalik":
            return "%g"
        if typ.name == "metin":
            return "%s"
        if typ.name == "mantik":
            return "%s"
        if typ.name in self.enums:
            return "%lld"
        return "%s"

    def _interp_printf(self, fmt_expr: Literal, extra_newline: bool, extra_args: List[Expr], line: int, col: int) -> str:
        parsed = parse_interpolation(fmt_expr.value, line, col)
        if parsed is None:
            args = [self._gen_expr(fmt_expr)] + [self._gen_expr(a) for a in extra_args]
            if extra_newline and not extra_args:
                return f'printf("%s\\n", {self._gen_expr(fmt_expr)})'
            if extra_newline:
                raise UppError(line, "upp.satir_yaz interpolasyonsuz ek argümanla '\\n' ekleyemez; format dizesine \\n koyun veya interpolasyon kullanın.", col)
            return f"printf({', '.join(args)})"
        if extra_args:
            raise UppError(line, "Interpolasyonlu dizede ek printf argümanı kullanılamaz.", col)
        parts = flatten_interp_parts(parsed)
        fmt_bits = []
        c_args = []
        for kind, payload in parts:
            if kind == "lit":
                fmt_bits.append(payload.replace("%", "%%"))
            else:
                typ = self._expr_type(payload)
                fmt_bits.append(self._printf_spec(typ))
                if typ.name == "mantik":
                    c_args.append(f"(({self._gen_expr(payload)}) ? \"dogru\" : \"yanlis\")")
                elif typ.name == "metin":
                    c_args.append(self._c_metin(payload))
                elif typ.name in self.enums:
                    c_args.append(f"(long long)({self._gen_expr(payload)})")
                else:
                    c_args.append(self._gen_expr(payload))
        fmt = "".join(fmt_bits)
        if extra_newline:
            fmt += "\n"
        call_args = [encode_c_string(fmt)] + c_args
        return f"printf({', '.join(call_args)})"

    def _interp_fprintf(self, stream: str, fmt_expr: Literal, extra_newline: bool, extra_args: List[Expr], line: int, col: int) -> str:
        parsed = parse_interpolation(fmt_expr.value, line, col)
        if parsed is None:
            args = [self._gen_expr(fmt_expr)] + [self._gen_expr(a) for a in extra_args]
            if extra_newline and not extra_args:
                return f'fprintf({stream}, "%s\\n", {self._gen_expr(fmt_expr)})'
            if extra_newline:
                raise UppError(line, "upp.hata_satir_yaz interpolasyonsuz ek argümanla '\\n' ekleyemez.", col)
            return f"fprintf({stream}, {', '.join(args)})"
        if extra_args:
            raise UppError(line, "Interpolasyonlu dizede ek printf argümanı kullanılamaz.", col)
        parts = flatten_interp_parts(parsed)
        fmt_bits = []
        c_args = []
        for kind, payload in parts:
            if kind == "lit":
                fmt_bits.append(payload.replace("%", "%%"))
            else:
                typ = self._expr_type(payload)
                fmt_bits.append(self._printf_spec(typ))
                if typ.name == "mantik":
                    c_args.append(f"(({self._gen_expr(payload)}) ? \"dogru\" : \"yanlis\")")
                elif typ.name == "metin":
                    c_args.append(self._c_metin(payload))
                elif typ.name in self.enums:
                    c_args.append(f"(long long)({self._gen_expr(payload)})")
                else:
                    c_args.append(self._gen_expr(payload))
        fmt = "".join(fmt_bits)
        if extra_newline:
            fmt += "\n"
        call_args = [encode_c_string(fmt)] + c_args
        return f"fprintf({stream}, {', '.join(call_args)})"

    def _interp_snprintf_box(self, fn: str, args: List[Expr], line: int, col: int) -> str:
        """upp.mesaj / upp.hata: başlık ve metin interpolasyonu."""
        if len(args) < 2:
            cname = UPP_C_NAMES[fn]
            return f"{cname}({', '.join(self._gen_expr(a) for a in args)})"
        title, body = args[0], args[1]
        rest = args[2:]
        if rest:
            raise UppError(line, f"upp.{fn} yalnızca (baslik, metin) alır.", col)

        def one(arg: Expr, var: str) -> Tuple[str, str]:
            if isinstance(arg, Literal) and arg.kind == "metin":
                parsed = parse_interpolation(arg.value, line, col)
                if parsed is None:
                    return "", self._gen_expr(arg)
                parts = flatten_interp_parts(parsed)
                fmt_bits = []
                c_args = []
                for kind, payload in parts:
                    if kind == "lit":
                        fmt_bits.append(payload.replace("%", "%%"))
                    else:
                        typ = self._expr_type(payload)
                        fmt_bits.append(self._printf_spec(typ))
                        if typ.name == "mantik":
                            c_args.append(f"(({self._gen_expr(payload)}) ? \"dogru\" : \"yanlis\")")
                        else:
                            c_args.append(self._gen_expr(payload))
                fmt = encode_c_string("".join(fmt_bits))
                extra = "".join(", " + a for a in c_args)
                nvar = f"{var}_n"
                prep = (
                    f"int {nvar} = snprintf(NULL, 0, {fmt}{extra}); "
                    f"char* {var} = NULL; "
                    f"if ({nvar} >= 0) {{ {var} = (char*)malloc((size_t){nvar} + 1u); "
                    f"if ({var}) snprintf({var}, (size_t){nvar} + 1u, {fmt}{extra}); }}"
                )
                return prep, var
            return "", self._gen_expr(arg)

        self._tmp += 1
        tbuf = f"_upp_t{self._tmp}"
        self._tmp += 1
        bbuf = f"_upp_t{self._tmp}"
        tprep, tref = one(title, tbuf)
        bprep, bref = one(body, bbuf)
        if not tprep and not bprep:
            return f"{UPP_C_NAMES[fn]}({tref}, {bref})"
        stmts = ["({"]
        if tprep:
            stmts.append(f"{tprep}")
        if bprep:
            stmts.append(f"{bprep}")
        stmts.append(f"{UPP_C_NAMES[fn]}({tref}, {bref});")
        if tprep:
            stmts.append(f"free({tbuf});")
        if bprep:
            stmts.append(f"free({bbuf});")
        stmts.append("})")
        return " ".join(stmts)

    def _gen_upp_call(self, name: str, args: List[Expr], line: int, col: int) -> str:
        if name not in UPP_RETURNS:
            raise UppError(line, f"'upp.{name}' yerleşik bir fonksiyon değil.", col)
        if name == "yaz":
            if not args:
                raise UppError(line, "upp.yaz en az bir argüman ister.", col)
            first = args[0]
            extra = args[1:]
            if isinstance(first, Literal) and first.kind == "metin":
                return self._interp_printf(first, False, extra, line, col)
            if extra:
                return f"printf({', '.join(self._gen_expr(a) for a in args)})"
            return f'printf("%s", {self._c_metin(first)})'
        if name == "satir_yaz":
            if not args:
                return 'printf("\\n")'
            first = args[0]
            extra = args[1:]
            if isinstance(first, Literal) and first.kind == "metin":
                return self._interp_printf(first, True, extra, line, col)
            if extra:
                return f"printf({', '.join(self._gen_expr(a) for a in args)})"
            return f'printf("%s\\n", {self._c_metin(first)})'
        if name in ("mesaj", "hata"):
            return self._interp_snprintf_box(name, args, line, col)
        if name in ("hata_yaz", "hata_satir_yaz"):
            extra_nl = name == "hata_satir_yaz"
            if not args:
                return 'fprintf(stderr, "\\n")' if extra_nl else '(void)0'
            first = args[0]
            extra = args[1:]
            if isinstance(first, Literal) and first.kind == "metin":
                return self._interp_fprintf("stderr", first, extra_nl, extra, line, col)
            if extra:
                return f"fprintf(stderr, {', '.join(self._gen_expr(a) for a in args)})"
            if extra_nl:
                return f'fprintf(stderr, "%s\\n", {self._c_metin(first)})'
            return f'fprintf(stderr, "%s", {self._c_metin(first)})'
        if name == "bekle":
            return f"upp_arkaplan_bekle_deger({self._gen_expr(args[0])})"
        if name == "uzunluk" and args:
            arg = args[0]
            at = self._expr_type(arg)
            if at.array_dims > 0:
                ptr, n = self._gen_array_ptr_count(arg, self._ctype(TypeNode(arg.line, arg.column, at.name, at.pointer_depth, 0)))
                return n
            if type_is_liste(at):
                return f"upp_liste_uzunluk({self._gen_expr(arg)})"
            if type_is_harita(at):
                return f"upp_harita_uzunluk({self._gen_expr(arg)})"
        cname = UPP_C_NAMES[name]
        return f"{cname}({', '.join(self._gen_expr(a) for a in args)})"

    def _gen_array_ptr_count(self, expr: Expr, elem_c: str) -> Tuple[str, str]:
        if isinstance(expr, ArrayLiteral):
            elems = ", ".join(self._gen_expr(e) for e in expr.elements)
            n = str(len(expr.elements))
            if not elems:
                return f"(({elem_c}*)0)", "0"
            return f"({elem_c}[]){{{elems}}}", n
        if isinstance(expr, Ident):
            t = self._lookup(expr.name, expr)
            if t.array_dims < 1:
                raise UppError(
                    expr.line,
                    "Dizi uzunluğu yalnızca gerçek dizi değişkeni veya '[...]' sabiti için hesaplanır; işaretçi verilemez.",
                    expr.column,
                )
        elif isinstance(expr, Member):
            t = self._expr_type(expr)
            if t.array_dims < 1:
                raise UppError(
                    expr.line,
                    "Dizi uzunluğu yalnızca gerçek dizi değişkeni veya '[...]' sabiti için hesaplanır; işaretçi verilemez.",
                    expr.column,
                )
        else:
            raise UppError(
                expr.line,
                "Dizi uzunluğu yalnızca gerçek dizi değişkeni veya '[...]' sabiti için hesaplanır; işaretçi verilemez.",
                expr.column,
            )
        c = self._gen_expr(expr)
        return c, f"(long long)(sizeof({c}) / sizeof(({c})[0]))"

    def _gen_upp_ns_call(self, ns: str, name: str, args: List[Expr], line: int, col: int) -> str:
        table = UPP_NAMESPACES.get(ns)
        if not table or name not in table:
            raise UppError(line, f"'upp.{ns}.{name}' yerleşik bir fonksiyon değil.", col)
        _ret, _arity, cname = table[name]
        if ns == "bellek" and name == "zincir_oku":
            base = self._gen_expr(args[0])
            ptr, n = self._gen_array_ptr_count(args[1], "long long")
            return f"{cname}({base}, {ptr}, {n})"
        if ns == "matematik" and name == "dunya_ekran":
            hx, hy, hz = (self._gen_expr(a) for a in args[:3])
            ptr, n = self._gen_array_ptr_count(args[3], "double")
            w, h = self._gen_expr(args[4]), self._gen_expr(args[5])
            return f"{cname}({hx}, {hy}, {hz}, {ptr}, {n}, {w}, {h})"
        if ns == "bellek" and name == "koruma_degistir":
            addr = self._gen_expr(args[0])
            size = self._gen_expr(args[1])
            izin_t = self._expr_type(args[2])
            if type_is_metin(izin_t):
                return f"{cname}({addr}, {size}, {self._gen_expr(args[2])})"
            return f"upp_bellek_koruma_degistir_sayi({addr}, {size}, {self._gen_expr(args[2])})"
        return f"{cname}({', '.join(self._gen_expr(a) for a in args)})"

    def _call_args_c(self, fn: FunctionDecl, args: List[Expr]) -> List[str]:
        out = [self._gen_expr(a) for a in args]
        for i in range(len(args), len(fn.params)):
            d = fn.params[i].default
            if d is None:
                break
            out.append(self._gen_expr(d))
        return out

    def _gen_ctor(self, cls: ClassDecl, args: List[Expr]) -> str:
        cn = self._cls_c(cls)
        olustur = [m for m in cls.methods if m.name == "olustur"]
        if len(olustur) != 1:
            raise UppError(cls.line, f"'{cls.name}(...)' için tek 'olustur' metodu gerekir.")
        cargs = self._call_args_c(olustur[0], args)
        self._tmp += 1
        tn = f"_upp_ct{self._tmp}"
        inner = ", ".join([f"&{tn}"] + cargs)
        return f"({{ {cn} {tn} = {{0}}; {cn}_olustur({inner}); {tn}; }})"

    def _gen_member_access(self, expr: Member) -> str:
        obj_t = self._expr_type(expr.obj)
        if obj_t.name.startswith("secenek."):
            ename = obj_t.name[8:]
            ed = self.enums.get(ename)
            cn = self._enum_c(ed) if ed is not None else type_c_name(ename)
            return f"{cn}_{expr.name}"
        if obj_t.name.startswith("modul."):
            alias = obj_t.name[6:]
            bag = self.modules.get(alias) or {}
            if expr.name in bag.get("var", {}):
                return c_symbol(expr.name, alias)
            raise UppError(
                expr.line,
                f"'{alias}.{expr.name}' bir değer değil; çağrı veya tür olarak yazın.",
                expr.column,
            )
        if obj_t.name.startswith("sinif."):
            key = obj_t.name[6:]
            cls = self.classes.get(key)
            if cls is None:
                raise UppError(expr.line, f"'{key}' bir sınıf değil.", expr.column)
            for c in cls.constants:
                if c.name == expr.name:
                    return f"{self._cls_c(cls)}_{c.name}"
            raise UppError(expr.line, f"'{cls.name}' sınıfında '{expr.name}' sabiti yok.", expr.column)
        if obj_t.name == "upp" or obj_t.name.startswith("upp."):
            raise UppError(
                expr.line,
                f"'{obj_t.name}.{expr.name}' bir fonksiyondur veya ad alanıdır; çağrı '...' ile yazın.",
                expr.column,
            )
        if type_is_liste(obj_t) or type_is_harita(obj_t):
            raise UppError(expr.line, f"'{expr.name}' bir metottur; '{expr.name}(...)' olarak çağırın.", expr.column)
        cls = self.classes.get(obj_t.name)
        if cls is None:
            raise UppError(expr.line, f"'{obj_t.name}' bir sınıf değil; '.' üye erişimi yok.", expr.column)
        field = self._class_field(cls, expr.name)
        if field is None:
            if self._class_method(cls, expr.name) is not None:
                raise UppError(expr.line, f"'{expr.name}' bir metottur; '{expr.name}(...)' olarak çağırın.", expr.column)
            raise UppError(expr.line, f"'{cls.name}' sınıfında '{expr.name}' üyesi yok.", expr.column)
        obj_c = self._gen_expr(expr.obj)
        if obj_t.pointer_depth > 0:
            return f"{obj_c}->{expr.name}"
        return f"{obj_c}.{expr.name}"

    def _gen_method_call(self, expr: Call) -> str:
        member = expr.callee
        assert isinstance(member, Member)
        obj_t = self._expr_type(member.obj)
        if obj_t.name == "upp":
            return self._gen_upp_call(member.name, expr.args, expr.line, expr.column)
        if obj_t.name.startswith("upp."):
            return self._gen_upp_ns_call(obj_t.name[4:], member.name, expr.args, expr.line, expr.column)
        if obj_t.name.startswith("modul."):
            alias = obj_t.name[6:]
            bag = self.modules.get(alias) or {}
            if member.name in bag.get("fn", {}):
                fn = bag["fn"][member.name]
                args = self._call_args_c(fn, expr.args)
                return f"{self._fn_c(fn)}({', '.join(args)})"
            if member.name in bag.get("class", {}):
                return self._gen_ctor(bag["class"][member.name], expr.args)
            raise UppError(expr.line, f"'{alias}' modülünde '{member.name}' yok.", expr.column)
        if obj_t.name.startswith("sinif."):
            key = obj_t.name[6:]
            cls = self.classes.get(key)
            if cls is None:
                raise UppError(expr.line, f"'{key}' bir sınıf değil.", expr.column)
            return self._gen_ctor(cls, expr.args)
        if obj_t.name == "ArkaplanIs" and member.name == "bekle":
            recv = self._gen_expr(member.obj)
            if obj_t.pointer_depth == 0:
                recv = f"&({recv})"
            return f"upp_arkaplan_bekle({recv})"
        if obj_t.name == "Kilit" and member.name in ("kilitle", "birak"):
            recv = self._gen_expr(member.obj)
            if obj_t.pointer_depth == 0:
                recv = f"&({recv})"
            return f"upp_kilit_{member.name}({recv})"
        if type_is_liste(obj_t):
            return self._gen_liste_call(expr, obj_t)
        if type_is_harita(obj_t):
            return self._gen_harita_call(expr, obj_t)
        cls = self.classes.get(obj_t.name)
        if cls is None:
            raise UppError(expr.line, f"'{obj_t.name}' bir sınıf değil; metot çağrılamaz.", expr.column)
        method = self._class_method(cls, member.name)
        if method is None:
            raise UppError(expr.line, f"'{cls.name}' sınıfında '{member.name}' metodu yok.", expr.column)
        recv = self._gen_expr(member.obj)
        if obj_t.pointer_depth == 0:
            recv = f"&({recv})"
        args = [recv] + self._call_args_c(method, expr.args)
        return f"{self._cls_c(cls)}_{member.name}({', '.join(args)})"

    def _gen_expr(self, expr: Optional[Expr]) -> str:
        if expr is None:
            return ""
        check_removed_api(expr)
        if isinstance(expr, Literal):
            return self._gen_literal(expr)
        if isinstance(expr, Ident):
            if expr.name == "kendi" and not self.current_class:
                raise UppError(expr.line, "'kendi' yalnızca sınıf metodu içinde kullanılabilir.", expr.column)
            return expr.name
        if isinstance(expr, Assign):
            if isinstance(expr.target, Index):
                arr_t = self._expr_type(expr.target.array)
                if type_is_liste(arr_t):
                    elem = parse_liste_elem(arr_t.name) or "sayi"
                    return self._gen_liste_index_assign(expr, elem)
            if expr.op == "+=":
                target_t = self._expr_type(expr.target)
                if type_is_metin(target_t) and not isinstance(expr.target, Index):
                    return f"upp_metin_ekle(&({self._gen_expr(expr.target)}), {self._gen_expr(expr.value)})"
            return f"({self._gen_expr(expr.target)} {expr.op} {self._gen_expr(expr.value)})"
        if isinstance(expr, TernaryOp):
            return f"({self._gen_expr(expr.cond)} ? {self._gen_expr(expr.then_expr)} : {self._gen_expr(expr.else_expr)})"
        if isinstance(expr, BinaryOp):
            return self._gen_binary(expr)
        if isinstance(expr, UnaryOp):
            return f"({expr.op}{self._gen_expr(expr.operand)})"
        if isinstance(expr, Call):
            if isinstance(expr.callee, Member):
                return self._gen_method_call(expr)
            if isinstance(expr.callee, Ident):
                if expr.callee.name in self.classes:
                    return self._gen_ctor(self.classes[expr.callee.name], expr.args)
                fn = self.functions.get(expr.callee.name)
                if fn is not None:
                    args = self._call_args_c(fn, expr.args)
                    return f"{self._fn_c(fn)}({', '.join(args)})"
            name = self._gen_expr(expr.callee)
            args = ", ".join(self._gen_expr(a) for a in expr.args)
            return f"{name}({args})"
        if isinstance(expr, Member):
            return self._gen_member_access(expr)
        if isinstance(expr, Index):
            arr_t = self._expr_type(expr.array)
            if type_is_liste(arr_t):
                elem = parse_liste_elem(arr_t.name) or "sayi"
                return self._liste_al_c(elem, self._gen_expr(expr.array), self._gen_expr(expr.index))
            if type_is_metin(arr_t):
                return f"upp_metin_kod({self._gen_expr(expr.array)}, {self._gen_expr(expr.index)})"
            return f"{self._gen_expr(expr.array)}[{self._gen_expr(expr.index)}]"
        if isinstance(expr, ArrayLiteral):
            elems = ", ".join(self._gen_expr(e) for e in expr.elements)
            return f"{{{elems}}}"
        if isinstance(expr, BackgroundExpr):
            return self._gen_background_expr(expr)
        raise UppError(getattr(expr, "line", 1), f"Üretilemeyen ifade: {type(expr).__name__}")

    def _c_metin(self, expr: Expr) -> str:
        c = self._gen_expr(expr)
        return f"(({c}) ? ({c}) : \"\")"

    def _fold_metin_add(self, expr: Expr) -> Optional[str]:
        if isinstance(expr, Literal) and expr.kind == "metin":
            if parse_interpolation(expr.value, expr.line, expr.column) is not None:
                return None
            return decode_string_literal(expr.value)
        if isinstance(expr, BinaryOp) and expr.op == "+":
            left = self._fold_metin_add(expr.left) if expr.left else None
            right = self._fold_metin_add(expr.right) if expr.right else None
            if left is not None and right is not None:
                return left + right
        return None

    def _gen_binary(self, expr: BinaryOp) -> str:
        left_t = self._expr_type(expr.left)
        right_t = self._expr_type(expr.right)
        if expr.op == "+" and type_is_metin(left_t) and type_is_metin(right_t):
            folded = self._fold_metin_add(expr)
            if folded is not None:
                return encode_c_string(folded)
            return f"upp_metin_birlestir({self._gen_expr(expr.left)}, {self._gen_expr(expr.right)})"
        if expr.op in ("==", "!=") and not type_is_yok(left_t) and not type_is_yok(right_t):
            if type_is_metin(left_t) and type_is_metin(right_t):
                eq = f"upp_metin_esit({self._gen_expr(expr.left)}, {self._gen_expr(expr.right)})"
                return eq if expr.op == "==" else f"(!{eq})"
        return f"({self._gen_expr(expr.left)} {expr.op} {self._gen_expr(expr.right)})"

    def _gen_literal(self, lit: Literal) -> str:
        if lit.kind == "sayi":
            return str(lit.value) + "LL"
        if lit.kind == "ondalik":
            return str(lit.value)
        if lit.kind == "metin":
            parsed = parse_interpolation(lit.value, lit.line, lit.column)
            if parsed is not None:
                return self._gen_interp_heap(lit)
            return encode_c_string(decode_string_literal(lit.value))
        if lit.kind == "mantik":
            return "true" if lit.value else "false"
        if lit.kind == "yok":
            return "NULL"
        return str(lit.value)

    def _koleksiyon_pack(self, elem: str, cval: str) -> str:
        if elem == "ondalik":
            return f"0LL, {cval}, NULL"
        if elem == "metin":
            return f"0LL, 0.0, {cval}"
        return f"(long long)({cval}), 0.0, NULL"

    def _liste_al_c(self, elem: str, recv: str, idx: str) -> str:
        if elem == "ondalik":
            return f"upp_liste_al_d({recv}, {idx})"
        if elem == "metin":
            return f"upp_liste_al_s({recv}, {idx})"
        if elem == "mantik":
            return f"(bool)upp_liste_al_i({recv}, {idx})"
        return f"upp_liste_al_i({recv}, {idx})"

    def _gen_liste_index_assign(self, expr: Assign, elem: str) -> str:
        arr = self._gen_expr(expr.target.array)
        idx = self._gen_expr(expr.target.index)
        tag = koleksiyon_tag(elem)
        if expr.op == "=":
            rhs = self._gen_expr(expr.value)
        else:
            op = expr.op[0]
            al = self._liste_al_c(elem, arr, idx)
            rhs = f"({al} {op} {self._gen_expr(expr.value)})"
        pack = self._koleksiyon_pack(elem, rhs)
        return f"upp_liste_yaz(&({arr}), {tag}, {idx}, {pack})"

    def _gen_liste_call(self, expr: Call, obj_t: TypeNode) -> str:
        member = expr.callee
        assert isinstance(member, Member)
        recv = self._gen_expr(member.obj)
        elem = parse_liste_elem(obj_t.name) or "sayi"
        tag = koleksiyon_tag(elem)
        name = member.name
        if name == "ekle":
            pack = self._koleksiyon_pack(elem, self._gen_expr(expr.args[0]))
            return f"upp_liste_ekle(&({recv}), {tag}, {pack})"
        if name == "al":
            return self._liste_al_c(elem, recv, self._gen_expr(expr.args[0]))
        if name == "yaz":
            pack = self._koleksiyon_pack(elem, self._gen_expr(expr.args[1]))
            return f"upp_liste_yaz(&({recv}), {tag}, {self._gen_expr(expr.args[0])}, {pack})"
        if name == "uzunluk":
            return f"upp_liste_uzunluk({recv})"
        if name == "bosalt":
            return f"upp_liste_bosalt(&({recv}))"
        raise UppError(expr.line, f"'liste' üzerinde '{name}' yok.", expr.column)

    def _harita_key_pack(self, ktur: str, expr: Expr) -> str:
        c = self._gen_expr(expr)
        if ktur == "metin":
            return f"0LL, {c}"
        return f"(long long)({c}), NULL"

    def _harita_al_c(self, vtur: str, recv: str, kpack: str) -> str:
        if vtur == "ondalik":
            return f"upp_harita_al_d({recv}, {kpack})"
        if vtur == "metin":
            return f"upp_harita_al_s({recv}, {kpack})"
        if vtur == "mantik":
            return f"(bool)upp_harita_al_i({recv}, {kpack})"
        return f"upp_harita_al_i({recv}, {kpack})"

    def _gen_harita_call(self, expr: Call, obj_t: TypeNode) -> str:
        member = expr.callee
        assert isinstance(member, Member)
        recv = self._gen_expr(member.obj)
        kv = parse_harita_kv(obj_t.name)
        ktur, vtur = kv if kv else ("metin", "sayi")
        ktag = koleksiyon_tag(ktur)
        vtag = koleksiyon_tag(vtur)
        name = member.name
        if name == "koy":
            kpack = self._harita_key_pack(ktur, expr.args[0])
            vpack = self._koleksiyon_pack(vtur, self._gen_expr(expr.args[1]))
            return f"upp_harita_koy(&({recv}), {ktag}, {vtag}, {kpack}, {vpack})"
        if name == "al":
            kpack = self._harita_key_pack(ktur, expr.args[0])
            return self._harita_al_c(vtur, recv, f"{ktag}, {kpack}")
        if name == "var_mi":
            kpack = self._harita_key_pack(ktur, expr.args[0])
            return f"(bool)upp_harita_var_mi({recv}, {ktag}, {kpack})"
        if name == "uzunluk":
            return f"upp_harita_uzunluk({recv})"
        if name == "bosalt":
            return f"upp_harita_bosalt(&({recv}))"
        if name == "anahtarlar":
            return f"upp_harita_anahtarlar({recv}, {ktag})"
        raise UppError(expr.line, f"'harita' üzerinde '{name}' yok.", expr.column)

    def _gen_interp_heap(self, lit: Literal) -> str:
        parsed = parse_interpolation(lit.value, lit.line, lit.column)
        if parsed is None:
            return encode_c_string(decode_string_literal(lit.value))
        parts = flatten_interp_parts(parsed)
        fmt_bits = []
        c_args = []
        for kind, payload in parts:
            if kind == "lit":
                fmt_bits.append(payload.replace("%", "%%"))
            else:
                typ = self._expr_type(payload)
                fmt_bits.append(self._printf_spec(typ))
                if typ.name == "mantik":
                    c_args.append(f"(({self._gen_expr(payload)}) ? \"dogru\" : \"yanlis\")")
                elif typ.name == "metin":
                    c_args.append(self._c_metin(payload))
                elif typ.name in self.enums:
                    c_args.append(f"(long long)({self._gen_expr(payload)})")
                else:
                    c_args.append(self._gen_expr(payload))
        fmt = encode_c_string("".join(fmt_bits))
        extra = "".join(", " + a for a in c_args)
        self._tmp += 1
        var = f"_upp_s{self._tmp}"
        nvar = f"_upp_sn{self._tmp}"
        return (
            f"({{ int {nvar} = snprintf(NULL, 0, {fmt}{extra}); "
            f"char* {var} = NULL; "
            f"if ({nvar} >= 0) {{ {var} = (char*)malloc((size_t){nvar} + 1u); "
            f"if ({var}) snprintf({var}, (size_t){nvar} + 1u, {fmt}{extra}); }} "
            f"{var}; }})"
        )


# ---------------------------------------------------------------------------
# AST yazdırma
# ---------------------------------------------------------------------------

def dump_ast(node: Any, indent: int = 0) -> str:
    pad = "  " * indent
    if node is None:
        return pad + "None"
    if isinstance(node, list):
        if not node:
            return pad + "[]"
        parts = [pad + "["]
        for item in node:
            parts.append(dump_ast(item, indent + 1))
        parts.append(pad + "]")
        return "\n".join(parts)
    if not isinstance(node, Node):
        return pad + repr(node)
    fields = []
    for key, value in node.__dict__.items():
        if key in ("line", "column"):
            continue
        if isinstance(value, (Node, list)):
            inner = dump_ast(value, indent + 1)
            fields.append(f"{pad}  {key}=\n{inner}")
        else:
            fields.append(f"{pad}  {key}={value!r}")
    loc = f" @{node.line}:{node.column}"
    if not fields:
        return f"{pad}{type(node).__name__}{loc}"
    return f"{pad}{type(node).__name__}{loc}\n" + "\n".join(fields)


# ---------------------------------------------------------------------------
# Derleme boru hattı
# ---------------------------------------------------------------------------

def host_hedef() -> str:
    """Derleyicinin çalıştığı işletim sistemi: Win32 → windows, aksi → linux (POSIX)."""
    return "windows" if sys.platform == "win32" else "linux"


def cli_hedef(*, linux: bool = False, windows: bool = False) -> str:
    if linux and windows:
        raise UppError(1, "--linux ve --windows birlikte kullanılamaz.")
    if linux:
        return "linux"
    if windows:
        return "windows"
    return host_hedef()


def hedef_bu_makinede_calisir(target: str) -> bool:
    return target == host_hedef()


def _gercek_yol_anahtar(yol: str) -> str:
    return os.path.normcase(os.path.abspath(yol))


def _yol_kucuk(yol: str) -> str:
    return yol.replace("\\", "/").lower()


def kutup_uzanti_mi(yol: str) -> bool:
    k = _yol_kucuk(yol)
    return k.endswith(".upp") or k.endswith(".uph")


def baslik_dosyasi_mi(yol: str) -> bool:
    return _yol_kucuk(yol).endswith(".uph")


def _coz_kullan_yol(spec: str, dahil_yol: str, satir: int, sutun: int) -> str:
    ham = spec.strip().replace("/", os.sep).replace("\\", os.sep)
    if not kutup_uzanti_mi(ham):
        raise UppError(satir, f"'kullan' yolu '.upp' veya '.uph' ile bitmeli, '{spec}' bulundu.", sutun)
    adaylar: List[str] = []
    if os.path.isabs(ham):
        adaylar.append(ham)
    else:
        if dahil_yol and os.path.isfile(dahil_yol):
            adaylar.append(os.path.join(os.path.dirname(os.path.abspath(dahil_yol)), ham))
        adaylar.append(os.path.join(os.getcwd(), ham))
    gorulen = set()
    for aday in adaylar:
        gercek = os.path.abspath(aday)
        if gercek in gorulen:
            continue
        gorulen.add(gercek)
        if os.path.isfile(gercek):
            return gercek
    raise UppError(satir, f"'kullan' dosyası bulunamadı: {spec}", sutun)


def yukle_program_kaynak(source: str, filename: str = "<girdi>", require_entry: bool = True) -> Program:
    """Kök kaynak + `kullan` ile gelen kütüphaneleri tek Program AST'sinde birleştirir."""
    if source.startswith("\ufeff"):
        source = source[1:]
    seen: set = set()
    stack: List[str] = []
    merged: List[Union[CInclude, ClassDecl, FunctionDecl, EnumDecl, VarDecl]] = []

    def parse_birim(src: str) -> Program:
        return Parser(Lexer(src).tokenize()).parse(require_entry=False)

    def flatten(src: str, yol: str, kutuphane: bool, alias: str = "") -> None:
        gercek = _gercek_yol_anahtar(yol) if os.path.isfile(yol) else os.path.normcase(yol)
        if gercek in stack:
            zincir = " -> ".join(stack + [gercek])
            raise UppError(1, f"kullan döngüsü: {zincir}")
        if gercek in seen:
            return
        seen.add(gercek)
        stack.append(gercek)
        program = parse_birim(src)
        for d in program.decls:
            if isinstance(d, KullanDecl):
                hedef = _coz_kullan_yol(d.spec, yol if os.path.isfile(yol) else "", d.line, d.column)
                with open(hedef, "r", encoding="utf-8") as handle:
                    alt = handle.read()
                if alt.startswith("\ufeff"):
                    alt = alt[1:]
                flatten(alt, hedef, True, d.alias)
            else:
                if kutuphane and isinstance(d, FunctionDecl) and d.name == "ana":
                    raise UppError(
                        d.line,
                        "Kütüphane dosyasında 'fonk ana' yasaktır; giriş noktası yalnızca kök dosyada olur.",
                        d.column,
                    )
                if isinstance(d, (FunctionDecl, ClassDecl, EnumDecl, VarDecl)):
                    d.kaynak = yol
                    d.mod_alias = alias
                    if isinstance(d, ClassDecl):
                        for method in d.methods:
                            method.kaynak = yol
                            method.mod_alias = alias
                        for c in d.constants:
                            c.kaynak = yol
                            c.mod_alias = alias
                merged.append(d)
        stack.pop()

    kok_yol = os.path.abspath(filename) if os.path.isfile(filename) else filename
    if require_entry and baslik_dosyasi_mi(kok_yol):
        raise UppError(
            1,
            "Başlık dosyası (.uph) derleme girişi olamaz; bir .upp programından 'kullan' ile katın.",
        )
    flatten(source, kok_yol, baslik_dosyasi_mi(kok_yol))
    first = 1
    first_col = 1
    ana_decls = [d for d in merged if isinstance(d, FunctionDecl) and d.name == "ana"]
    if require_entry:
        if not ana_decls:
            raise UppError(first, "Giriş noktası bulunamadı. Bir 'fonk ana()' tanımlayın.")
        if len(ana_decls) > 1:
            extra = ana_decls[1]
            raise UppError(
                extra.line,
                "Birden fazla 'fonk ana()' tanımlanamaz; giriş noktası tek olmalıdır.",
                extra.column,
            )
    return Program(first, first_col, merged)


def compile_source(source: str, filename: str = "<girdi>", target: Optional[str] = None) -> str:
    if target is None:
        target = host_hedef()
    if target not in ("windows", "linux"):
        raise UppError(1, f"Bilinmeyen derleme hedefi: {target}")
    try:
        program = yukle_program_kaynak(source, filename, require_entry=True)
    except UppError:
        raise
    except Exception as extra:
        raise UppError(1, f"{filename} ayrıştırılırken iç hata: {extra}") from extra
    TypeInferencer().run(program)
    SafetyChecker().check(program)
    gen = CodeGenerator()
    gen.target = target
    return gen.generate(program)


def parse_source(source: str, require_entry: bool = True, filename: str = "<girdi>") -> Program:
    program = yukle_program_kaynak(source, filename, require_entry=require_entry)
    TypeInferencer().run(program)
    return program


def _tani_kayit(err: BaseException, tur: str = "hata") -> Dict[str, Any]:
    satir_ham = getattr(err, "line", None)
    sutun_ham = getattr(err, "column", None)
    satir = 1 if satir_ham is None else int(satir_ham)
    sutun = 1 if sutun_ham is None else int(sutun_ham)
    mesaj = getattr(err, "message", str(err))
    return {
        "satir": satir,
        "sutun": sutun,
        "son_satir": satir,
        "son_sutun": sutun + 1,
        "tur": tur,
        "mesaj": mesaj,
    }


def analiz_kaynak(source: str, filename: str = "<girdi>") -> List[Dict[str, Any]]:
    """Lexer → Parser → TypeInferencer → SafetyChecker. Codegen ve GCC yok.

    Dönüş, `--json-hata` / `--analiz` ile aynı tanı nesneleridir (`tur`: hata|guvenlik).
    Boş liste = kaynak bu aşamada temiz.
    """
    if source.startswith("\ufeff"):
        source = source[1:]
    try:
        program = yukle_program_kaynak(source, filename, require_entry=not baslik_dosyasi_mi(filename))
        errs = TypeInferencer().run(program, collect=True)
        sc = SafetyChecker(collect=True)
        sc.check(program)
        errs.extend(sc.errors)
        out = []
        for e in errs:
            tur = "guvenlik" if isinstance(e, SafetyError) else "hata"
            out.append(_tani_kayit(e, tur))
            if len(out) >= 50:
                break
        return out
    except UppError as err:
        return [_tani_kayit(err, "hata")]


def ide_program(source: str, filename: str = "<girdi>") -> Optional[Program]:
    """Kısmi AST: `ana` zorunlu değil; tip hatasında yine de ağaç döner."""
    if source.startswith("\ufeff"):
        source = source[1:]
    try:
        program = yukle_program_kaynak(source, filename, require_entry=False)
    except UppError:
        return None
    except Exception:
        return None
    try:
        TypeInferencer().run(program)
    except (UppError, SafetyError):
        pass
    return program


def bicim_kaynak(source: str) -> str:
    """Girinti 4. Dizge ve yorum içinde boşluklara dokunulmaz."""
    if source.startswith("\ufeff"):
        source = source[1:]
    src = source.replace("\r\n", "\n").replace("\r", "\n")
    lines = src.split("\n")
    out: List[str] = []
    depth = 0
    in_block = False
    in_triple = False
    for raw in lines:
        s = raw
        if in_block or in_triple:
            out.append(s)
            if in_block and "*/" in s:
                in_block = False
            if in_triple:
                # kapanış """ bu satırda (açılış üçlüsü değilse)
                idx = 0
                while True:
                    p = s.find('"""', idx)
                    if p < 0:
                        break
                    in_triple = not in_triple
                    idx = p + 3
            continue
        stripped = s.strip()
        if stripped == "":
            out.append("")
            continue
        # satır başı kapanış
        lead = depth
        tmp = stripped
        while tmp.startswith("}"):
            lead = max(0, lead - 1)
            tmp = tmp[1:].lstrip()
        out.append(("    " * lead) + stripped)
        # derinlik: satırdaki süslü (dizge/yorum dışında kabaca)
        i = 0
        in_str = False
        esc = False
        while i < len(s):
            ch = s[i]
            nxt = s[i + 1] if i + 1 < len(s) else ""
            if in_str:
                if esc:
                    esc = False
                elif ch == "\\":
                    esc = True
                elif ch == '"':
                    in_str = False
                i += 1
                continue
            if s[i:i + 3] == '"""':
                in_triple = True
                i += 3
                # aynı satırda kapanış
                p = s.find('"""', i)
                if p >= 0:
                    in_triple = False
                    i = p + 3
                    continue
                break
            if ch == '"' :
                in_str = True
                i += 1
                continue
            if ch == "/" and nxt == "/":
                break
            if ch == "/" and nxt == "*":
                in_block = True
                i += 2
                if "*/" in s[i:]:
                    in_block = False
                break
            if ch == "{":
                depth += 1
            elif ch == "}":
                depth = max(0, depth - 1)
            i += 1
    text = "\n".join(out)
    if src.endswith("\n"):
        if not text.endswith("\n"):
            text += "\n"
    return text


def bicim_dosya(yol: str, kontrol: bool) -> int:
    if not os.path.isfile(yol):
        sys.stderr.write(f"[u++ HATA] Dosya bulunamadı: {yol}\n")
        return 1
    with open(yol, "r", encoding="utf-8") as handle:
        ham = handle.read()
    yeni = bicim_kaynak(ham)
    a = ham.replace("\r\n", "\n").replace("\r", "\n")
    if a.startswith("\ufeff"):
        a = a[1:]
    if kontrol:
        if yeni != a:
            sys.stderr.write(f"[u++ HATA] --bicim-kontrol: '{yol}' biçimsiz.\n")
            return 1
        return 0
    if yeni != ham:
        with open(yol, "w", encoding="utf-8", newline="\n") as handle:
            handle.write(yeni)
    return 0


_GCC_CANDIDATES = [
    r"C:\msys64\ucrt64\bin\gcc.exe",
    r"C:\msys64\mingw64\bin\gcc.exe",
    r"C:\msys64\clang64\bin\gcc.exe",
    r"C:\mingw64\bin\gcc.exe",
    r"C:\MinGW\bin\gcc.exe",
]


def find_gcc() -> Optional[str]:
    found = shutil.which("gcc")
    if found:
        return found
    for path in _GCC_CANDIDATES:
        if os.path.isfile(path):
            return path
    return None


def invoke_gcc(c_path: str, exe_path: str, target: Optional[str] = None) -> subprocess.CompletedProcess:
    if target is None:
        target = host_hedef()
    gcc = find_gcc()
    if gcc is None:
        if target == "linux":
            raise UppError(
                1,
                "GCC bulunamadı. 'gcc' paketini kurun (Debian/Ubuntu: build-essential) ve PATH'te olduğundan emin olun.",
            )
        raise UppError(
            1,
            "GCC bulunamadı. MinGW-w64 kurun ve 'gcc' komutunun PATH içinde olduğundan emin olun.",
        )
    env = os.environ.copy()
    gcc_dir = os.path.dirname(os.path.abspath(gcc))
    env["PATH"] = gcc_dir + os.pathsep + env.get("PATH", "")
    if target == "linux":
        cmd = [gcc, "-std=gnu11", "-pthread", c_path, "-o", exe_path, "-lm"]
    else:
        cmd = [gcc, "-std=gnu11", c_path, "-o", exe_path, "-luser32", "-lwinmm", "-lgdi32"]
    return subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8", errors="replace", env=env)


def output_stems(cikti: Optional[str], target: Optional[str] = None) -> Tuple[str, str]:
    """`--cikti ornek` → ornek.c / ornek.exe (Windows) veya ornek.out (Linux)."""
    if target is None:
        target = host_hedef()
    cwd = os.getcwd()
    exe_ext = ".out" if target == "linux" else ".exe"
    if not cikti or not str(cikti).strip():
        return os.path.join(cwd, "cikti.c"), os.path.join(cwd, "program" + exe_ext)
    base = str(cikti).strip()
    lower = base.lower()
    for ext in (".exe", ".out", ".c", ".upp", ".uph"):
        if lower.endswith(ext):
            base = base[: -len(ext)]
            break
    name = os.path.basename(base)
    if not name or name in (".", ".."):
        raise UppError(1, "'--cikti' geçerli bir dosya kökü bekler (ör. ornek).")
    if os.path.isabs(base):
        root = base
    else:
        root = os.path.join(cwd, base)
    return root + ".c", root + exe_ext


def _json_hata_yaz(ok: bool, hatalar: List[Dict[str, Any]]) -> None:
    sys.stdout.write(json.dumps({"ok": ok, "hatalar": hatalar}, ensure_ascii=False, separators=(",", ":")))
    sys.stdout.write("\n")
    sys.stdout.flush()


def _json_hata_kayit(err: BaseException, tur: str = "hata") -> Dict[str, Any]:
    kayit = _tani_kayit(err, tur)
    return {"satir": kayit["satir"], "sutun": kayit["sutun"], "tur": kayit["tur"], "mesaj": kayit["mesaj"]}


def analiz_dosya(upp_path: str) -> int:
    """GCC ve C üretimi olmadan JSON tanı basar (`--analiz`)."""
    if not os.path.isfile(upp_path):
        _json_hata_yaz(False, [{"satir": 1, "sutun": 1, "tur": "hata", "mesaj": f"Dosya bulunamadı: {upp_path}"}])
        return 1
    with open(upp_path, "r", encoding="utf-8") as handle:
        source = handle.read()
    hatalar = analiz_kaynak(source, os.path.abspath(upp_path))
    _json_hata_yaz(len(hatalar) == 0, [
        {"satir": h["satir"], "sutun": h["sutun"], "tur": h["tur"], "mesaj": h["mesaj"]}
        for h in hatalar
    ])
    return 0 if not hatalar else 1


def compile_file(
    upp_path: str,
    emit_c_only: bool = False,
    dump_tree: bool = False,
    run_exe: bool = True,
    cikti: Optional[str] = None,
    json_hata: bool = False,
    target: Optional[str] = None,
) -> int:
    if target is None:
        target = host_hedef()
    if json_hata:
        run_exe = False
        dump_tree = False

    if not os.path.isfile(upp_path):
        msg = f"Dosya bulunamadı: {upp_path}"
        if json_hata:
            _json_hata_yaz(False, [{"satir": 1, "sutun": 1, "tur": "hata", "mesaj": msg}])
        else:
            print(f"[u++ HATA] {msg}")
        return 1

    if not json_hata:
        print(f"[u++] '{upp_path}' ayrıştırılıyor...")
    with open(upp_path, "r", encoding="utf-8") as handle:
        source = handle.read()
    if source.startswith("\ufeff"):
        source = source[1:]

    try:
        if dump_tree:
            program = parse_source(source, filename=os.path.abspath(upp_path))
            SafetyChecker().check(program)
            print(dump_ast(program))
            return 0
        c_code = compile_source(source, os.path.abspath(upp_path), target)
        c_path, exe_path = output_stems(cikti, target)
    except SafetyError as err:
        if json_hata:
            _json_hata_yaz(False, [_json_hata_kayit(err, "guvenlik")])
        else:
            print(err)
        return 1
    except UppError as err:
        if json_hata:
            _json_hata_yaz(False, [_json_hata_kayit(err, "hata")])
        else:
            print(err)
        return 1

    try:
        parent = os.path.dirname(c_path)
        if parent:
            os.makedirs(parent, exist_ok=True)
        with open(c_path, "w", encoding="utf-8", newline="\n") as handle:
            handle.write(c_code)
    except OSError as err:
        msg = f"Çıktı yazılamadı ({c_path}): {err}"
        if json_hata:
            _json_hata_yaz(False, [{"satir": 1, "sutun": 1, "tur": "hata", "mesaj": msg}])
        else:
            print(f"[u++ HATA] {msg}")
        return 1

    if not json_hata:
        print(f"[u++] Ara C kodu üretildi: {c_path}")
    if emit_c_only:
        if json_hata:
            _json_hata_yaz(True, [])
        return 0

    if not json_hata:
        if target == "linux":
            print("[u++] GCC ile native ikili derleniyor (gcc -std=gnu11 -pthread -lm)...")
        else:
            print("[u++] GCC ile native ikili derleniyor (gcc -std=gnu11 -luser32 -lwinmm -lgdi32)...")
    try:
        result = invoke_gcc(c_path, exe_path, target)
    except UppError as err:
        if json_hata:
            _json_hata_yaz(False, [_json_hata_kayit(err, "hata")])
        else:
            print(err)
        return 1

    if result.returncode != 0:
        gcc_msg = (result.stderr or result.stdout or "GCC derleme başarısız").strip()
        if json_hata:
            _json_hata_yaz(False, [{"satir": 1, "sutun": 1, "tur": "gcc", "mesaj": gcc_msg}])
        else:
            print("[GCC DERLEME HATASI]:")
            print(result.stderr or result.stdout)
        return 1

    if json_hata:
        _json_hata_yaz(True, [])
        return 0

    print(f"[u++] DERLEME BAŞARILI: {exe_path}")
    if not run_exe:
        return 0
    if not hedef_bu_makinede_calisir(target):
        print(f"[u++] Hedef '{target}' bu makinede çalıştırılmıyor; ikili: {exe_path}")
        return 0

    print("=" * 50)
    completed = subprocess.run([exe_path], cwd=os.getcwd())
    return completed.returncode


def _configure_stdio() -> None:
    if sys.platform == "win32":
        for stream in (sys.stdout, sys.stderr):
            try:
                stream.reconfigure(encoding="utf-8")
            except Exception:
                pass


def main(argv: Optional[List[str]] = None) -> int:
    _configure_stdio()
    parser = argparse.ArgumentParser(
        prog="uppc",
        description="u++ v3.0 derleyicisi — Türkçe kaynak kodunu C11'e çevirir ve GCC ile derler. Bayrak yoksa hedef, derleyicinin çalıştığı işletim sistemidir.",
    )
    parser.add_argument("kaynak", nargs="?", default=os.path.join("ornekler", "ornek1.upp"), help="u++ kaynak (.upp program; --analiz ile .uph)")
    parser.add_argument(
        "--cikti",
        metavar="AD",
        default=None,
        help="Çıktı kökü: '--cikti ornek' → ornek.c ve ornek.exe veya ornek.out (yoksa cikti.c / program.exe|.out)",
    )
    parser.add_argument("--sadece-c", action="store_true", help="Yalnızca .c üret; GCC çağırma")
    parser.add_argument("--sadece-derle", action="store_true", help="GCC ile ikili üret ama çalıştırma")
    parser.add_argument("--ast", action="store_true", help="Ayrıştırılmış AST ağacını yazdır ve çık")
    parser.add_argument(
        "--analiz",
        action="store_true",
        help="Lexer/parser/tip/güvenlik tanısını JSON bas; C ve GCC yok (canlı LSP)",
    )
    parser.add_argument(
        "--json-hata",
        action="store_true",
        help="Tam derleme tanısını tek JSON nesnesi olarak yaz (C + GCC; canlı LSP için --analiz)",
    )
    parser.add_argument(
        "--linux",
        action="store_true",
        help="Linux/POSIX hedefi: pthread çalışma zamanı, çıktı .out",
    )
    parser.add_argument(
        "--windows",
        action="store_true",
        help="Windows hedefi: Win32 çalışma zamanı, çıktı .exe",
    )
    parser.add_argument(
        "--bicim",
        action="store_true",
        help="Kaynağı yerinde biçimle (girinti 4); C/GCC yok",
    )
    parser.add_argument(
        "--bicim-kontrol",
        action="store_true",
        dest="bicim_kontrol",
        help="Biçim farkı varsa çıkış kodu 1 (CI)",
    )
    args = parser.parse_args(argv)
    if args.linux and args.windows:
        parser.error("--linux ve --windows birlikte kullanılamaz.")
    if args.bicim and args.bicim_kontrol:
        parser.error("--bicim ve --bicim-kontrol birlikte kullanılamaz.")
    if args.bicim or args.bicim_kontrol:
        return bicim_dosya(args.kaynak, kontrol=args.bicim_kontrol)
    if args.analiz:
        return analiz_dosya(args.kaynak)
    return compile_file(
        args.kaynak,
        emit_c_only=args.sadece_c,
        dump_tree=args.ast,
        run_exe=not args.sadece_derle,
        cikti=args.cikti,
        json_hata=args.json_hata,
        target=cli_hedef(linux=args.linux, windows=args.windows),
    )


if __name__ == "__main__":
    sys.exit(main())
