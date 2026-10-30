# IS EVERY PAPER THE TEXT LEANS ON ACTUALLY IN THE REFERENCE LIST?
#
# The README's `## References` section opens with a promise -- "every paper the text leans
# on is listed with a link" -- and an earlier pass found two citations the body used and
# the list had not carried. That pass was done by reading. This does it mechanically, so
# the promise is checkable rather than re-asserted.
#
# WHAT IT DOES. Builds an index of (surname, year) from the reference list, then scans the
# prose and the source comments for citation-shaped mentions and reports the ones the index
# does not cover. It also reports list entries that nothing cites, which is not an error --
# a bibliography may carry a paper read and not used -- but is worth seeing.
#
# WHAT IT CANNOT DO, stated because a clean exit here does not mean the citations are
# right: it matches surnames and years, so it cannot tell a correct citation from a
# plausible-looking wrong one, and it cannot find a paper leaned on without being named at
# all. It catches the omission, not the misattribution.
#
#   python3 tools/audit_citations.py
import html
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# Scanned for mentions. The README is also where the list lives; its reference section is
# cut out below so the list does not count as a citation of itself.
CORPUS = ["README.md", "docs/ROADMAP.md", "docs/discoveries.html",
          "host/src/experiments.cpp", "host/src/experiments_milestones.cpp",
          "core/src/brain.cpp", "core/src/network.cpp", "core/include/aibaby/dna.h"]

# Words that take a year after them in this project's prose but are not surnames. Nearly
# all of them are the project's own log idiom ("CLOSED 2026-10-29") or a venue name in a
# reference entry. ALL-CAPS tokens are dropped separately, so only the mixed-case ones
# need listing.
NOT_A_SURNAME = {
    # the project's own dated-log vocabulary
    "Added", "Built", "Checked", "Closed", "Changed", "Corrected", "Down", "Explained",
    "Fixed", "Measured", "Moved", "Open", "Parallelised", "Pinned", "Re", "Removed",
    "Renamed", "Replaced", "Resized", "Restored", "Revised", "Run", "Shipped", "Split",
    "Struck", "The", "Until", "Upward", "Written", "Retaken", "Recommitted", "Repinned",
    "Remeasured", "Retired", "Superseded", "Reopened", "Landed", "Raised", "Since",
    "Dated", "Updated", "Started", "Stopped", "Found", "Read", "Ran", "Now", "By", "In",
    "On", "At", "From", "This", "Also", "And", "But", "See", "Note", "Day", "Night",
    "Morning", "Evening", "Version", "Build", "Commit", "Session", "Today", "Yesterday",
    # venues and publishers, which appear inside reference entries next to their year
    "Biology", "Communications", "Computation", "Conference", "Frontiers", "Journal",
    "Nature", "Neuron", "Neuroscience", "Physiology", "Proceedings", "Reports", "Review",
    "Reviews", "Rxiv", "Science", "Sciences", "Society", "Systems", "Transactions",
    "Advances", "Annals", "Bulletin", "Cortex", "Elife", "Letters", "Magazine",
    "Psychology", "Trends", "Learning", "Research", "Networks", "Computing", "Press",
    "NeurIPS", "ICLR", "ICML", "PNAS", "JMLR", "BioRxiv", "ArXiv", "Elsevier",
    # months, which appear in document datelines
    "January", "February", "March", "April", "May", "June", "July", "August",
    "September", "October", "November", "December",
}

CITE = re.compile(
    r"\b([A-Z][A-Za-z’'\-]{2,})"                 # a surname
    r"(?:(?:,? (?:&|and) [A-Z][A-Za-z’'\-]{2,})"  # & Other
    r"|(?: et al\.?)"                                  # et al.
    r"|(?:, [A-Z][A-Za-z’'\-]{2,}))*"             # , Other
    r"(?:’s|'s)?"                                 # Matsuoka's 1985
    r"[  ]*\(?((?:19|20)[0-9]{2})[a-z]?\)?")


def read(rel):
    path = os.path.join(ROOT, rel)
    if not os.path.exists(path):
        return None
    with open(path, encoding="utf-8", errors="replace") as fh:
        return fh.read()


def ref_section(readme):
    """The `## References` block only: from its heading to the next `### ` that is not part
    of it. The section ends where the chronological log resumes, which that note names."""
    start = readme.find("\n## References\n")
    if start < 0:
        return None
    end = readme.find("### A note on where this project's record actually lives", start)
    if end < 0:                          # the end marker moved; fall back to the next ##
        nxt = readme.find("\n## ", start + 10)
        end = nxt if nxt > 0 else len(readme)
    return readme[start:end]


def index_of(section):
    """{(surname, year)} and {year: [entry first line]} from the list's bullet entries.

    Every surname in an entry is indexed, not just the first, because the body cites
    second authors ("Benna & Fusi") as readily as first ones.
    """
    pairs, entries = set(), []
    for block in re.split(r"\n(?=- )", section):
        if not block.startswith("- "):
            continue
        flat = " ".join(block.split())
        # EVERY year in the bullet, not just the first. A bullet here routinely carries two
        # or three papers -- "Naud & Sprekeler (2018); and Lisman, J. E. (1997)" -- and the
        # first version of this function indexed only up to the first year, so it reported
        # Lisman, Munkres and Kuhn's co-citations as absent from a list that carries them.
        # Three of nine "missing" citations in its first run were this bug.
        spans = list(re.finditer(r"\b((?:19|20)[0-9]{2})[a-z]?\b", flat))
        if not spans:
            entries.append((None, flat[:100]))
            continue
        prev = 0
        for m in spans:
            year = m.group(1)
            # Names are those appearing between the previous citation's year and this one,
            # which is where an author list sits in this bibliography's house style.
            for n in re.findall(r"\b([A-Z][A-Za-z’'\-]{2,})\b", flat[prev:m.start()]):
                n = re.sub(r"[’']s$", "", n)
                # The ALL-CAPS filter the mention scanner applies belongs here too. Without
                # it, an entry whose commentary SHOUTS -- this list's house style -- indexes
                # "THE", "STRONGEST", "REGIME" as surnames, which cannot cause a false PASS
                # but buries the not-cited report in its own prose.
                if n not in NOT_A_SURNAME and not n.isupper():
                    pairs.add((n, year))
            prev = m.end()
        entries.append((spans[0].group(1), flat[:100]))
    return pairs, entries


def mentions(text, label):
    out = []
    for m in CITE.finditer(text):
        name, year = m.group(1), m.group(2)
        # The apostrophe is in the surname character class so that O'Reilly matches, which
        # also swallows the possessive: the first run of this tool reported `Brown's 1911`
        # and `Matsuoka's 1985` as uncited alongside the plain forms of the same two.
        name = re.sub(r"[’']s$", "", name)
        # `Re-measured 2026-08-29` and its family: the project's own dated-log idiom, which
        # the hyphen in the surname class lets through.
        if name.isupper() or name in NOT_A_SURNAME or name.startswith("Re-"):
            continue
        ctx = " ".join(text[max(0, m.start() - 60):m.end() + 20].split())
        out.append((name, year, label, ctx))
    return out


#   Surname & Surname / Surname, Surname & Surname / Surname et al., with NO year after it.
# These cannot be matched against an author-year index at all, and skipping them silently is
# how Zenke & Gerstner (2017) stayed out of the list through a clean run of this tool: the
# document cited it with no year, so the pattern above never saw it. They are reported as
# unverifiable rather than passed over, because a checker that only inspects what it can
# parse certifies the rest by omission.
BARE = re.compile(
    # `(?<!, )` keeps the match from starting in the MIDDLE of an author list: without it,
    # "Fiete, Fee & Seung" also reports "Fee & Seung", and "Toyoizumi, Kaneko, Stryker &
    # Miller" also reports "Stryker & Miller".
    r"(?<!, )\b([A-Z][A-Za-z’'\-]{2,}"
    r"(?:(?:,? (?:&|and) [A-Z][A-Za-z’'\-]{2,})|(?: et al\.))+)"
    # The `\b` is load-bearing. Without it the negative lookahead makes the engine backtrack
    # one character into the final surname so the year falls outside the group, and the
    # report prints "Stryker & Mille" and "Poole & Gangul" for citations that DO carry a
    # year -- noise that looks like a finding.
    r"\b(?![  ]*\(?(?:19|20)[0-9]{2})")


def bare_mentions(text):
    out = {}
    for m in BARE.finditer(text):
        who = " ".join(m.group(1).split())
        if any(w in NOT_A_SURNAME or w.isupper() for w in re.findall(r"[A-Za-z\-]+", who)):
            continue
        out[who] = out.get(who, 0) + 1
    return out


readme = read("README.md")
if readme is None:
    print("VOID: no README.md at %s" % ROOT)
    sys.exit(2)
section = ref_section(readme)
if section is None:
    print("VOID: no `## References` heading in README.md, so there is no list to audit.")
    sys.exit(2)

pairs, entries = index_of(section)
print("reference list      %d entries, %d indexed (surname, year) pairs"
      % (len(entries), len(pairs)))
noyear = [e for y, e in entries if y is None]
if noyear:
    print("                    %d entries carry no year and cannot be matched:" % len(noyear))
    for e in noyear[:6]:
        print("                      %s" % e[:96])

found, missing, seen, bare = [], {}, set(), {}
for rel in CORPUS:
    text = read(rel)
    if text is None:
        print("skipped (absent)    %s" % rel)
        continue
    if rel == "README.md":
        text = text.replace(section, "\n")       # the list is not a citation of itself
    if rel.endswith(".html"):
        # Entities must be unescaped BEFORE matching, not after. Without this, `Zenke &amp;
        # Gerstner` does not match a pattern expecting `& `, and that is the exact citation
        # the year-less report exists to surface -- so the first version of this block
        # printed six harmless author-list tails and silently dropped the one real gap.
        flat = html.unescape(re.sub(r"<[^>]+>", " ", text))
        for who, n in bare_mentions(flat).items():
            bare[who] = bare.get(who, 0) + n
    for name, year, label, ctx in mentions(text, rel):
        key = (name, year)
        if key in pairs:
            seen.add(key)
            found.append(key)
        else:
            missing.setdefault(key, []).append((label, ctx))

print("citations in prose  %d matched the list, %d distinct pairs did not"
      % (len(found), len(missing)))

if missing:
    print("\nNOT IN THE LIST. Each is either a paper the list should carry, or a false")
    print("positive the pattern should not have matched -- both need a human read:")
    for (name, year), where in sorted(missing.items()):
        files = sorted({w[0] for w in where})
        print("\n  %s %s   (%d mention%s in %s)"
              % (name, year, len(where), "" if len(where) == 1 else "s", ", ".join(files)))
        print("    %s" % where[0][1][:150])

if bare:
    print("\nCITED WITHOUT A YEAR, so UNVERIFIABLE by this tool. Read each against the list")
    print("by hand; one of these (Zenke & Gerstner) was genuinely absent while this tool")
    print("exited clean:")
    for who in sorted(bare):
        print("  %-52s x%d" % (who, bare[who]))

# THE SECOND CHECK: does `discoveries.html` resolve its OWN citations? It is distributed as
# a standalone PDF, so a reader of it does not have the README. It named about twenty papers
# and carried no bibliography at all until Appendix B was added, which is a different defect
# from the one above -- the citation existed in the README and was simply unreachable from
# the document that leaned on it.
disc = read("docs/discoveries.html")
orphans = []
if disc is not None:
    # Whitespace is collapsed before matching, because a reference entry wraps across
    # source lines: an author-to-year matcher written with `[^\n]` reported Assran (2023)
    # and Gadagkar (2016) as unresolved while both were sitting in Appendix B.
    flat = " ".join(html.unescape(re.sub(r"<[^>]+>", " ", disc)).split())
    cut = flat.find("Appendix B. References")
    body, bib = (flat[:cut], flat[cut:]) if cut > 0 else (flat, "")
    if not bib:
        print("\nDISCOVERIES HAS NO APPENDIX B, so none of its citations resolve from the")
        print("document itself. A standalone PDF needs its own bibliography.")
        orphans.append(("(no bibliography)", ""))
    else:
        want = set()
        for m in CITE.finditer(body):
            nm = re.sub(r"[’']s$", "", m.group(1))
            if not (nm.isupper() or nm in NOT_A_SURNAME or nm.startswith("Re-")):
                want.add((nm, m.group(2)))
        for nm, yr in sorted(want):
            if not re.search(r"\b%s\b.{0,400}?\(%s\)" % (re.escape(nm), yr), bib) \
               and not re.search(r"\(%s\).{0,400}?\b%s\b" % (yr, re.escape(nm)), bib):
                orphans.append((nm, yr))
    print("\nDOES `discoveries.html` RESOLVE ITS OWN CITATIONS?  %d cited, %d unresolved"
          % (len(want) if bib else 0, len(orphans)))
    for nm, yr in orphans:
        print("  unresolved in Appendix B   %s %s" % (nm, yr))

uncited = sorted({p for p in pairs if p not in seen})
if uncited:
    byyear = {}
    for n, y in uncited:
        byyear.setdefault(y, []).append(n)
    print("\nIN THE LIST, NOT CITED IN THE SCANNED TEXT. Not an error -- a bibliography may")
    print("carry a paper read and not leaned on, and co-author surnames land here when the")
    print("body cites only the first author:")
    for y in sorted(byyear):
        print("  %s  %s" % (y, ", ".join(sorted(byyear[y]))))

print()
if missing or orphans:
    if missing:
        print("THE README LIST IS INCOMPLETE, on %d distinct (surname, year) pairs."
              % len(missing))
    if orphans:
        print("`discoveries.html` DOES NOT RESOLVE %d of its own citations." % len(orphans))
    sys.exit(1)
print("EVERY CITATION-SHAPED MENTION IN THE SCANNED TEXT RESOLVES to a list entry. That")
print("is the omission check only: it cannot tell a correct citation from a plausible")
print("wrong one, nor find a paper leaned on without being named.")
