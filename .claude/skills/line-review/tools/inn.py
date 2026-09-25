"""Add innates to existing rows, keeping each row alphabetical by display name.

    python3 .claude/skills/line-review/tools/inn.py SPECIES1,SPECIES2 ABILITY1,ABILITY2

Names are bare constants without the SPECIES_/ABILITY_ prefixes. Every named
species must already have a row -- this edits, it does not create -- and it
asserts rather than guessing if one is missing, which is the alias trap failing
loudly instead of silently writing a duplicate row.
"""
import re,sys
# usage: inn.py SPECIES1,SPECIES2 ABILITY1,ABILITY2   (bare names)
p='src/fork/innate_abilities.c'
s=open(p).read()
# Sort by the in-game name, not the constant: ROCKY_PAYLOAD < ROCK_HEAD, but "Rock Head" < "Rocky Payload".
# The "Innate abilities: each species' innates are in alphabetical order" test enforces the same order.
names=dict(re.findall(r'\[(ABILITY_\w+)\]\s*=\s*\{\s*\.name\s*=\s*_\("([^"]*)"\)',open('src/data/abilities.h').read()))
sps=sys.argv[1].split(','); abs_=['ABILITY_'+a for a in sys.argv[2].split(',')]
for sp in sps:
    pat=re.compile(r'(        SPECIES_%s,\n        INNATES\(\n)(.*?)(\n        \))'%sp, re.S)
    m=pat.search(s); assert m, sp
    cur=[x.strip().rstrip(',') for x in m.group(2).split('\n') if x.strip()]
    merged=sorted(set(cur)|set(abs_), key=lambda a: names[a])
    s=s[:m.start()]+m.group(1)+',\n'.join('            '+a for a in merged)+m.group(3)+s[m.end():]
    print('%-22s %s'%(sp, ', '.join(a.replace('ABILITY_','') for a in merged)))
open(p,'w').write(s)
