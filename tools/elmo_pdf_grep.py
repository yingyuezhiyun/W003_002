#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""Search keywords in a (text-based) PDF and print context snippets.

This is intended for extracting command definitions from vendor manuals.
If the PDF is scanned (image-only), pypdf will likely extract no text.
"""

from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass
from pathlib import Path

from pypdf import PdfReader


@dataclass(frozen=True)
class Hit:
    page_index: int
    keyword: str
    start: int
    end: int
    snippet: str


def _configure_stdio() -> None:
    # Windows consoles may default to GBK and crash on PDF special glyphs.
    # Use UTF-8 + backslashreplace so printing never fails.
    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="backslashreplace")  # type: ignore[attr-defined]
    except Exception:
        pass
    try:
        sys.stderr.reconfigure(encoding="utf-8", errors="backslashreplace")  # type: ignore[attr-defined]
    except Exception:
        pass


def _normalize_text(text: str) -> str:
    # Keep things grep-friendly: collapse whitespace but keep readable.
    return re.sub(r"\s+", " ", text).strip()


def extract_page_texts(pdf_path: Path) -> list[str]:
    reader = PdfReader(str(pdf_path))
    page_texts: list[str] = []
    for page in reader.pages:
        raw = page.extract_text() or ""
        page_texts.append(_normalize_text(raw))
    return page_texts


def find_hits(page_texts: list[str], keywords: list[str], context: int) -> list[Hit]:
    hits: list[Hit] = []
    for page_index, text in enumerate(page_texts):
        if not text:
            continue
        lower = text.lower()
        for kw in keywords:
            needle = kw.lower()
            start = 0
            while True:
                pos = lower.find(needle, start)
                if pos < 0:
                    break
                s = max(0, pos - context)
                e = min(len(text), pos + len(needle) + context)
                snippet = text[s:e]
                hits.append(Hit(page_index=page_index, keyword=kw, start=s, end=e, snippet=snippet))
                start = pos + len(needle)
    return hits


def main(argv: list[str]) -> int:
    _configure_stdio()
    parser = argparse.ArgumentParser(
        description="Search keywords in a PDF and print context snippets (for Elmo SimplIQ manuals, etc.)."
    )
    parser.add_argument("pdf", type=Path, help="Path to PDF file")
    parser.add_argument(
        "--keywords",
        nargs="+",
        default=[
            "brake",
            "quick stop",
            "emergency",
            "fault",
            "reset",
            "stop",
            "abort",
            "halt",
            "enable",
        ],
        help="Keywords to search (case-insensitive)",
    )
    parser.add_argument(
        "--context",
        type=int,
        default=140,
        help="Context size in characters before/after match",
    )
    parser.add_argument(
        "--max-hits",
        type=int,
        default=200,
        help="Maximum number of hits to print",
    )
    args = parser.parse_args(argv)

    if not args.pdf.exists():
        print(f"ERROR: file not found: {args.pdf}", file=sys.stderr)
        return 2

    try:
        page_texts = extract_page_texts(args.pdf)
    except Exception as exc:
        print(f"ERROR: failed to read PDF: {exc}", file=sys.stderr)
        return 2

    total_chars = sum(len(t) for t in page_texts)
    nonempty_pages = sum(1 for t in page_texts if t)

    if nonempty_pages == 0 or total_chars < 2000:
        print("NOTE: Extracted almost no text. This PDF may be scanned (image-only).")
        print("      Suggestion: export text via OCR, or provide a text-based PDF/manual.")
        print(f"      nonempty_pages={nonempty_pages}, total_chars={total_chars}")

    hits = find_hits(page_texts, args.keywords, args.context)

    if not hits:
        print("No hits.")
        return 1

    print(f"Hits: {len(hits)} (showing up to {args.max_hits})")
    printed = 0
    for h in hits:
        printed += 1
        if printed > args.max_hits:
            break
        page_no = h.page_index + 1
        print("-" * 80)
        print(f"Page {page_no}: keyword='{h.keyword}'")
        print(h.snippet)

    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
