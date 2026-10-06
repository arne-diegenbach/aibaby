# docs

`discoveries.pdf` — the project's original findings, written as a research document:
results with no external antecedent. Author: Arne Diegenbach.

Regenerate from source (the HTML is the source of truth, the PDF is a build product):

    soffice --headless --norestore --convert-to pdf:writer_pdf_Export \
        docs/discoveries.html --outdir docs

**Inclusion criterion.** A finding belongs in `discoveries.pdf` only if this project
discovered it. Anything a published result says or frames first is excluded and listed
in Appendix A instead, so the exclusion is auditable rather than taken on trust — the
line is drawn at the *claim*, not the tools, so quantities measured with Cochran's ratio
estimator and the Quenouille–Tukey jackknife are included while those methods are not.

When adding a finding, give the number with its uncertainty and the n, say what would
have refused it, and put the external antecedent in Appendix A if there is one. The full
reasoning for every entry lives in the root `README.md`; this document is the summary,
not the record.
