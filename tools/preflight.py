#!/usr/bin/env python3
"""Preflight: lay every sheet's rows and columns over each other.

Errors (block the build): unfilled cells, unknown columns, duplicate ids,
references between sheets that do not resolve.
Warnings (do not block): rows still marked verified=false.

Usage: preflight.py [--scope logic|all]
  logic = world, board, inputs, tricks, scoring (enough to generate and unit-test the rider code)
  all   = logic + hooks, runs (needed before building the real plugin)
"""
import json
import pathlib
import sys

SHEETS = pathlib.Path(__file__).resolve().parent.parent / "sheets"
LOGIC = ["world", "board", "surfaces", "inputs", "tricks", "scoring"]
ALL = LOGIC + ["hooks", "runs"]
# (sheet, column) -> sheet whose id column it must match
REFS = {("tricks", "input"): "inputs", ("runs", "board"): "board"}


def load(name):
    return json.loads((SHEETS / f"{name}.json").read_text())


def check(scope):
    names = LOGIC if scope == "logic" else ALL
    data = {n: load(n) for n in names}
    errors, warnings = [], []
    for n, sheet in data.items():
        cols, seen = sheet["columns"], set()
        for row in sheet["rows"]:
            rid = row.get("id", "?")
            if rid in seen:
                errors.append(f"{n}/{rid}: duplicate id")
            seen.add(rid)
            for extra in set(row) - set(cols):
                errors.append(f"{n}/{rid}: column '{extra}' is not declared")
            for c in cols:
                if c not in row or row[c] in (None, ""):
                    errors.append(f"{n}/{rid}: unfilled cell '{c}'")
            if row.get("verified") is False:
                warnings.append(f"{n}/{rid}: not verified yet")
    for (sheet, col), target in REFS.items():
        if sheet not in data or target not in data:
            continue
        ids = {r["id"] for r in data[target]["rows"]}
        for row in data[sheet]["rows"]:
            if col in row and row[col] not in ids:
                errors.append(f"{sheet}/{row.get('id')}: {col}='{row[col]}' does not exist in {target}")
    return errors, warnings


if __name__ == "__main__":
    scope = "all"
    if "--scope" in sys.argv:
        scope = sys.argv[sys.argv.index("--scope") + 1]
    errs, warns = check(scope)
    for e in errs:
        print("ERROR  ", e)
    for w in warns:
        print("warning", w)
    print(f"\nscope={scope}: {len(errs)} errors, {len(warns)} warnings")
    sys.exit(1 if errs else 0)
