#!/usr/bin/env python3
"""Find the evaluation-order bug the §48 pre-flight exposed.

The bug: a printf whose argument list contains BOTH a call that writes an
out-parameter and a bare read of that same variable, e.g.

    std::printf("%7.2f +- %5.2f", ctx_mean_se(v, &e1), e1);

C++ does not order printf's arguments, so `e1` may be read before the call
writes it -- it printed 0.00 for every row of a 36-seed table.

Approach: join each `printf(` statement onto one line (up to the matching
close paren + `;`), then look for `&NAME` and a separate bare `NAME` token in
the same statement.
"""
import glob
import re
import sys

PATHS = sys.argv[1:] or sorted(glob.glob("host/src/*.cpp") + glob.glob("host/src/*.h"))


def statements(text):
    """Yield (line_number, one_line_text) for each printf statement."""
    for m in re.finditer(r"printf\s*\(", text):
        depth = 0
        i = m.end() - 1
        while i < len(text):
            if text[i] == "(":
                depth += 1
            elif text[i] == ")":
                depth -= 1
                if depth == 0:
                    break
            i += 1
        stmt = text[m.start():i + 1]
        yield text.count("\n", 0, m.start()) + 1, " ".join(stmt.split())


COMMENT_OR_STRING = re.compile(
    r'("(?:[^"\\\n]|\\.)*")'      # group 1: a string literal, which we KEEP
    r"|//[^\n]*"                  # a line comment
    r"|/\*.*?\*/",                # a block comment
    re.S,
)


def strip_comments(text):
    """Blank out // and /* */ comments, keeping string literals and line numbers.

    Comments must go because the fix for this bug is DOCUMENTED with a comment
    showing the broken call, and the scanner flagged its own explanation.

    String literals must STAY, matched first in the alternation: a naive
    `re.sub(r"//[^\n]*", "", text)` eats the rest of any line holding a `http://`
    inside a string, which merged ~600 characters of following code into one
    pseudo-statement and produced a false positive on `nanosleep(&nap, ...)`.
    """

    def repl(m):
        if m.group(1) is not None:
            return m.group(1)
        return "\n" * m.group(0).count("\n")

    return COMMENT_OR_STRING.sub(repl, text)


def scan(path):
    text = strip_comments(open(path).read())
    hits = 0
    for line, stmt in statements(text):
        # strip the format string so "%s" etc. cannot match a name
        body = re.sub(r'"(?:[^"\\]|\\.)*"', '""', stmt)
        out_params = set(re.findall(r"&([A-Za-z_]\w*)", body))
        if not out_params:
            continue
        for name in sorted(out_params):
            # a bare read of the same name, NOT preceded by '&' and not part
            # of a longer identifier or a member access
            bare = re.search(r"(?<![&\w.>])\b" + re.escape(name) + r"\b(?!\s*\[)", body)
            # count occurrences: one is the &name itself, so we need a second
            n_amp = len(re.findall(r"&" + re.escape(name) + r"\b", body))
            n_all = len(re.findall(r"\b" + re.escape(name) + r"\b", body))
            if n_all > n_amp and bare:
                hits += 1
                print(f"{path}:{line}")
                print(f"    out-param '{name}' also read bare in the same printf")
                print(f"    {stmt[:300]}")
                print()
    return hits


def main():
    total = 0
    for path in PATHS:
        total += scan(path)
    print(f"{total} suspect out-param read(s) across {len(PATHS)} file(s)")
    # Nonzero on a finding, so this can gate. Note the project rule that a
    # PIPELINE EATS A FAILING EXIT: run it as its own command, not mid-pipe.
    return 1 if total else 0


if __name__ == "__main__":
    sys.exit(main())
