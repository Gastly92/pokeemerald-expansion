"""Show everything the three fork data files hold for a species.

    python3 .claude/skills/line-review/tools/line.py VENUSAUR VENUSAUR_MEGA

Prints the innate row, any ability-override rows, and every frontier set (format,
item, ability, moves, tera, nature, EVs) -- the Step 1/2/3 starting state for a
line in one call. Names are bare constants without the SPECIES_ prefix, and must
be the exact constant the row is keyed on: run tools/forms.py first if a species
comes back with "(no row)", since that is far more often an alias miss than a
real gap.
"""
import re,sys
src=open('src/fork/frontier_extended_mons.c').read()
inn=open('src/fork/innate_abilities.c').read()
ovr=open('src/fork/species_ability_overrides.c').read()
for sp in sys.argv[1:]:
    m=re.search(r'        SPECIES_%s,\n        INNATES\(\n(.*?)\n        \)'%sp, inn, re.S)
    print("== %s innates: %s"%(sp, ', '.join(a.strip().rstrip(',').replace('ABILITY_','') for a in m.group(1).split('\n')) if m else '(no row)'))
    for o in re.findall(r'SPECIES_%s, (\d),\n        (ABILITY_\w+)'%sp, ovr):
        print("   override slot %s -> %s"%(o[0],o[1].replace('ABILITY_','')))
    for h in re.findall(r'\{\n        \.species = SPECIES_%s,\n(.*?)\n    \},'%sp, src, re.S):
        item=re.search(r'\.heldItem = (\w+)',h); mv=re.findall(r'MOVE_[A-Z0-9_]+',h)
        ab=re.search(r'\.ability = (\w+)',h); tera=re.search(r'\.teraType = (\w+)',h); tags=re.search(r'\.tags = (\w+)',h)
        nat=re.search(r'\.nature = ([^\n]+)',h)
        ev=' '.join(re.findall(r'\.\w+ = \d+',h.split('.ev = EVS(')[1].split(')')[0])) if '.ev = EVS(' in h else '?'
        print("   %-9s %-17s %-15s %s | tera=%s | %s %s"%(tags.group(1).replace('FORMAT_',''),item.group(1).replace('ITEM_',''),ab.group(1).replace('ABILITY_',''),','.join(x.replace('MOVE_','') for x in mv),tera.group(1).replace('TYPE_',''),(nat.group(1) if nat else '?').replace('NATURE(','').rstrip(','),ev))
