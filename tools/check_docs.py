#!/usr/bin/env python3
#
# check_docs.py    does the documentation still describe THIS library?
# -----------------------------------------------------------------------
#
# ★ a doc drifts silently.  a function is renamed and the old name lives
# on in prose; a type gains an `s`; something documented as public never
# had a declaration.  none of it breaks a build, so none of it is found
# until somebody follows the document and it lies to them.
#
# ⓘ THE SAME SHAPE AS bme's check_faces.py AND test/slots.py - a
# mechanical check for a discipline nobody can hold in their head.
#
#   tools/check_docs.py            report drift
#   tools/check_docs.py --list     also list what IS documented
#
# ⚠ IT CANNOT TELL YOU THE PROSE IS RIGHT.  it checks that every name the
# docs USE exists, and that every API function is at least MENTIONED.
# a paragraph describing the wrong behaviour passes this and always will.
#
# ⚠⚠ AND "EXISTS" MEANS ANYWHERE IN h/ OR src/, not just in a header.
# the document has an INTERNALS section that describes `static` functions
# deliberately, and a checker that only knew about headers called those
# ghosts.  ★ a false alarm in a drift tool is worse than none: it trains
# you to skim the output.

import glob
import os
import re
import sys

DOCS = ("uCurses.md", "USING-uCurses.md", "GETTING_STARTED.md")

# -----------------------------------------------------------------------

def headers():
    return sorted(glob.glob("h/*.h"))

def read(path):
    return open(path, encoding="utf-8", errors="replace").read()

# -----------------------------------------------------------------------
# every function carrying the API macro - the library's public surface

def api_functions():
    out = {}

    for h in headers():
        for line in read(h).splitlines():
            m = re.match(
                r"\s*API\s+[A-Za-z_0-9]+[ \t*]+\**([A-Za-z_0-9]+)\s*\(", line)

            if m:
                out[m.group(1)] = os.path.basename(h)

    return out

# -----------------------------------------------------------------------
# ⓘ source and header BASENAMES are legitimately named in prose - "see
# uC_widget_keys.c" - so they are not ghosts.

def file_names():
    names = set()

    for p in glob.glob("src/**/*.c", recursive=True) + headers():
        names.add(os.path.splitext(os.path.basename(p))[0])

    return names

# -----------------------------------------------------------------------

def where_used(docs, token):
    for d, txt in docs.items():
        for n, line in enumerate(txt.splitlines(), 1):
            if token in line:
                return "%s:%d" % (d, n)

    return "?"

# -----------------------------------------------------------------------

def main():
    docs = {d: read(d) for d in DOCS if os.path.exists(d)}

    if not docs:
        print("no documentation found - run me from the repo root")
        return 1

    prose = "\n".join(docs.values())
    api = api_functions()
    # ⚠⚠ SOURCES TOO, NOT JUST HEADERS.  uCurses.md has a deliberate
    # INTERNALS section - it documents `static` functions like
    # widget_key_f10() and widget_input_active() on purpose - so a name
    # that exists in src/ is documented correctly even though no header
    # declares it.  ★ scanning headers alone reported
    # uC_widget_reset_state() as a ghost when the doc was right and the
    # checker was wrong.
    hdr = "\n".join(read(h) for h in headers())
    hdr += "\n".join(read(c) for c in glob.glob("src/**/*.c", recursive=True))
    files = file_names()

    undocumented = [f for f in sorted(api) if f not in prose]

    # ⚠ a uC_ name the docs USE that is neither a symbol in a header nor
    # a file in the tree.  ★ this is the half that actively misleads a
    # reader, so it is reported first.
    ghosts = sorted({t for t in re.findall(r"\buC_[A-Za-z_0-9]+\b", prose)
                     if t not in hdr and t not in files})

    print("%d API functions, %d documented, %d not mentioned"
          % (len(api), len(api) - len(undocumented), len(undocumented)))

    if ghosts:
        print("\n⚠⚠ %d NAME(S) THE DOCS USE THAT DO NOT EXIST:" % len(ghosts))

        for t in ghosts:
            print("   %-30s %s" % (t, where_used(docs, t)))

    if undocumented:
        print("\n⚠ %d API FUNCTION(S) NOT MENTIONED IN ANY DOC:"
              % len(undocumented))

        for f in undocumented:
            print("   %-40s %s" % (f, api[f]))

    if "--list" in sys.argv:
        print("\ndocumented:")

        for f in sorted(api):
            if f in prose:
                print("   %-40s %s" % (f, api[f]))

    return 1 if (ghosts or undocumented) else 0

# -----------------------------------------------------------------------

if __name__ == "__main__":
    sys.exit(main())

# =======================================================================
