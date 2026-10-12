"""Print Smogon's published sets for a species, every generation and format.

    python3 -I .claude/skills/line-review/tools/smogon.py VENUSAUR VENUSAUR_MEGA

Names are the bare species constants line.py takes; each also matches its formes
(VENUSAUR also prints Venusaur-Mega and Venusaur-Gmax). The data is Smogon's own
set dex as JSON from data.pkmn.cc, downloaded once per generation and cached in
~/.cache/line-review-smogon (delete that directory to refresh).

Use it AFTER drafting a set from the creature, never before: Smogon sets are
learnset-bound and assume vanilla mechanics, so they are for evaluating and
sparking ideas, not for bounding the move pool (see the Smogon rule in
fork-docs/LINE_REVIEW.md Step 3). Cite what it prints as Smogon, not as recall.
"""
import json, os, re, sys, urllib.request

CACHE = os.path.expanduser('~/.cache/line-review-smogon')
URL = 'https://data.pkmn.cc/sets/gen%d.json'


def norm(s):
    return re.sub(r'[^a-z0-9]', '', s.lower())


def load(gen):
    os.makedirs(CACHE, exist_ok=True)
    path = os.path.join(CACHE, 'gen%d.json' % gen)
    if not os.path.exists(path):
        try:
            req = urllib.request.Request(URL % gen, headers={'User-Agent': 'line-review/1.0'})
            with urllib.request.urlopen(req, timeout=60) as r:
                data = r.read()
        except Exception as e:
            print('gen%d: fetch failed (%s)' % (gen, e), file=sys.stderr)
            return {}
        with open(path, 'wb') as f:
            f.write(data)
    with open(path, encoding='utf-8') as f:
        return json.load(f)


def join(v):
    if v is None:
        return '-'
    return '/'.join(v) if isinstance(v, list) else str(v)


def evs(s):
    e = s.get('evs')
    if isinstance(e, list):
        e = e[0] if e else None
    if not e:
        return '-'
    return ' '.join('%s %s' % (v, k) for k, v in e.items())


gens = [load(g) for g in range(1, 10)]
for arg in sys.argv[1:]:
    want = norm(arg)
    hits = 0
    print('== %s' % arg)
    for g, data in enumerate(gens, 1):
        for name, formats in data.items():
            n = norm(name)
            if n != want and not norm(name.split('-')[0]) == want:
                continue
            for fmt, sets in formats.items():
                for title, s in sets.items():
                    hits += 1
                    moves = ', '.join(join(m) for m in s.get('moves', []))
                    print('  gen%d %-12s %-16s %s' % (g, fmt, name, title))
                    print('      %s | %s | %s | %s | %s' % (
                        join(s.get('item')), join(s.get('ability')),
                        join(s.get('nature')), evs(s), moves))
    if not hits:
        print('  (no Smogon sets)')
