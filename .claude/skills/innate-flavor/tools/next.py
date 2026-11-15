"""Where the innate flavor sweep stands, and what to judge next.

    python3 -I .claude/skills/innate-flavor/tools/next.py            # next species + next innate
    python3 -I .claude/skills/innate-flavor/tools/next.py VENUSAUR   # one species' remaining innates
    python3 -I .claude/skills/innate-flavor/tools/next.py --status   # every roster species' progress
    python3 -I .claude/skills/innate-flavor/tools/next.py --check    # lock check; exits 1 on a problem

The pool is sImplementedInnates[] (test/fork/innate_abilities.c) minus
fork-docs/innate_flavor/excluded.csv, in alphabetical order of in-game name. Species
come from the extended roster (src/fork/frontier_extended_mons.c) in the order their
.species first appear. The log is fork-docs/innate_flavor.csv.

--check enforces the lock on every species whose sweep is complete:
  * its row in src/fork/innate_abilities.c must hold exactly the innates the log
    marks fit=Yes (an edit to a reviewed row must go through the log first), and
  * it must have a decided row for every innate in the current pool (an innate
    wired after the review shows up here as needing assessment).
"""
import csv, re, sys

LOG = 'fork-docs/innate_flavor.csv'
EXCLUDED = 'fork-docs/innate_flavor/excluded.csv'


def ab(x): return x.replace('ABILITY_', '')


def pool():
    t = open('test/fork/innate_abilities.c').read()
    ids = re.findall(r'ABILITY_\w+', re.search(r'sImplementedInnates\[\]\s*=\s*\{(.*?)\};', t, re.S).group(1))
    a = open('src/data/abilities.h').read()
    names = {m.group(1): m.group(2) for m in re.finditer(r'\[(ABILITY_\w+)\]\s*=\s*\{\s*\.name = _\("([^"]*)"\)', a)}
    excluded = {r['innate'] for r in csv.DictReader(open(EXCLUDED, newline=''))}
    full = sorted(((names[i], i) for i in ids), key=lambda p: p[0].lower())
    return [p for p in full if p[0] not in excluded], {n: i for n, i in full}


def roster():
    out = []
    for s in re.findall(r'\.species = SPECIES_(\w+)', open('src/fork/frontier_extended_mons.c').read()):
        if s not in out:
            out.append(s)
    return out


def innate_rows():
    s = open('src/fork/innate_abilities.c').read()
    rows = {}
    for m in re.finditer(r'\{ // \d+\s*\n\s*SPECIES_(\w+),\s*(INNATES\(.*?\)|ABILITY_\w+)\s*\n\s*\}', s, re.S):
        rows[m.group(1)] = set(re.findall(r'ABILITY_\w+', m.group(2)))
    return rows


def log():
    by = {}
    for r in csv.DictReader(open(LOG, newline='')):
        by.setdefault(r['species'], {})[r['innate']] = r
    return by


def remaining(sp, p, lg):
    done = lg.get(sp, {})
    return [n for n, _ in p if n not in done or done[n]['fit'] == 'Pending']


def main():
    args = sys.argv[1:]
    p, ids = pool()
    lg, ros = log(), roster()
    if '--check' in args:
        rows, bad = innate_rows(), []
        for sp, done in lg.items():
            if remaining(sp, p, lg):
                missing = [n for n, _ in p if n not in done]
                pending = [n for n in done if done[n]['fit'] == 'Pending']
                # a species with every pool innate once decided, minus ones added later, is a locked species
                decided = sum(1 for r in done.values() if r['fit'] != 'Pending')
                if decided and not pending and missing:
                    bad.append('%s: reviewed, but not yet assessed for %s' % (sp, ', '.join(missing)))
                continue
            want = {ids[n] for n, r in done.items() if r['fit'] == 'Yes' and n in ids}
            have = rows.get(sp, set())
            if want != have:
                extra = sorted(ab(x) for x in have - want)
                lost = sorted(ab(x) for x in want - have)
                bad.append('%s: innate row differs from the flavor log (row has extra %s; missing %s)'
                           % (sp, extra or '-', lost or '-'))
        print('\n'.join(bad) if bad else 'innate flavor lock: OK')
        sys.exit(1 if bad else 0)
    if '--status' in args:
        for sp in ros:
            rem = remaining(sp, p, lg)
            state = 'done' if not rem else ('not started' if sp not in lg else '%d left' % len(rem))
            print('%-28s %s' % (sp, state))
        return
    species = [a for a in args if not a.startswith('-')]
    if not species:
        species = [next((s for s in ros if remaining(s, p, lg)), None)]
        if species[0] is None:
            print('every roster species is done')
            return
    for sp in species:
        rem = remaining(sp, p, lg)
        print('%s: %d of %d innates left%s' % (sp, len(rem), len(p), '' if rem else ' (done)'))
        if rem:
            print('next innate: %s (%d of %d)' % (rem[0], len(p) - len(rem) + 1, len(p)))


if __name__ == '__main__':
    main()
