---
name: innate-flavor
description: Continue the innate flavor sweep — a maintainer-driven review of one Battle Factory extended-roster species at a time, one implemented innate per turn, with a blind Claude verdict and every decision logged in fork-docs/innate_flavor.csv. Triggers when the user asks to continue or resume the flavor sweep, start the next species in it, or run it on a named species. Separate from line-review: innates only, and the maintainer makes every call.
---

# Innate flavor sweep

The full procedure, the log format and the lock are in
`fork-docs/innate_flavor/README.md`. Read it before the first turn, and follow it.

To resume:

1. `python3 -I .claude/skills/innate-flavor/tools/next.py` prints the next species and
   the next innate. If the log has a `Pending` row, that innate is still waiting for the
   maintainer: re-present it rather than starting a new one.
2. Gather the species' evidence once (dex text, `bulbapedia.py`, `episodes.py`), then
   present innates one per turn, as the README describes.
3. Do not present more than one innate per turn, do not show the proposed verdict, and
   do not change `src/fork/innate_abilities.c` until the species is finished and the
   maintainer has decided what gets written.
