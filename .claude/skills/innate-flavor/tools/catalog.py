"""List every implemented innate a species does NOT already have, for the flavor sweep.

    python3 -I .claude/skills/innate-flavor/tools/catalog.py VENUSAUR

Prints the species' current innate row and canon .abilities, then one line per
remaining implemented innate (sImplementedInnates[] in test/fork/innate_abilities.c):
its in-game description, how many species have it canonically, and how many carry
it as an innate today. Names are bare constants without the SPECIES_ prefix.
"""
import glob, re, sys

def ab(name): return name.replace('ABILITY_', '')

test = open('test/fork/innate_abilities.c').read()
pool = re.findall(r'ABILITY_\w+', re.search(r'sImplementedInnates\[\]\s*=\s*\{(.*?)\};', test, re.S).group(1))

desc = {}
for m in re.finditer(r'\[(ABILITY_\w+)\]\s*=\s*\{(.*?)\n    \},', open('src/data/abilities.h').read(), re.S):
    d = re.search(r'\.description = COMPOUND_STRING\(\s*"(.*?)"\s*\)', m.group(2), re.S)
    desc[m.group(1)] = re.sub(r'"\s*"', '', d.group(1)).replace('\\n', ' ') if d else ''

canon = {}
for f in glob.glob('src/data/pokemon/species_info/*.h'):
    t = open(f, encoding='utf-8', errors='replace').read()
    for m in re.finditer(r'\[SPECIES_(\w+)\] =(.*?)(?=\n    \[SPECIES_|\Z)', t, re.S):
        a = re.search(r'\.abilities = \{([^}]*)\}', m.group(2), re.S)
        if a: canon[m.group(1)] = [x.strip() for x in a.group(1).split(',') if x.strip() and x.strip() != 'ABILITY_NONE']

inn = open('src/fork/innate_abilities.c').read()
rows = {}
for m in re.finditer(r'\{ // \d+\s*\n\s*SPECIES_(\w+),\s*(INNATES\(.*?\)|ABILITY_\w+)\s*\n\s*\}', inn, re.S):
    rows[m.group(1)] = re.findall(r'ABILITY_\w+', m.group(2))

canonUsers = {a: sum(a in l for l in canon.values()) for a in pool}
innUsers = {a: sum(a in l for l in rows.values()) for a in pool}

for sp in sys.argv[1:]:
    have = rows.get(sp)
    print('== %s' % sp)
    print('   canon abilities: %s' % ', '.join(ab(a) for a in canon.get(sp, [])))
    print('   innates now:     %s' % (', '.join(ab(a) for a in have) if have else '(no row -- check forms.py)'))
    rest = [a for a in pool if a not in (have or [])]
    print('   %d implemented innates to judge (canon users / innate holders):' % len(rest))
    for a in sorted(rest):
        print('   %-18s %3d/%-3d %s' % (ab(a), canonUsers[a], innUsers[a], desc.get(a, '')))
