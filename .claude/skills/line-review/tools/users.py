"""Count and list the canon users of an ability.

    python3 .claude/skills/line-review/tools/users.py MOXIE INTIMIDATE

This is the Step 1 gate: ONE user means the ability is a signature welded to that
creature and is not available to borrow; many users mean you read the family and
ask whether this species belongs in it. Counting is over .abilities in
species_info, so it is the repo's answer rather than a remembered one.
"""
import re,sys,glob
pat=re.compile(r'\[SPECIES_([A-Z0-9_]+)\] =(.*?)(?=\n    \[SPECIES_|\Z)',re.S)
data=[]
for f in glob.glob('src/data/pokemon/species_info/*.h'):
    t=open(f,encoding='utf-8',errors='replace').read()
    for m in pat.finditer(t):
        ab=re.search(r'\.abilities = \{([^}]*)\}',m.group(2),re.S)
        if ab: data.append((m.group(1),[x.strip() for x in ab.group(1).split(',') if x.strip()]))
for a in sys.argv[1:]:
    key='ABILITY_'+a
    us=[s for s,l in data if key in l]
    print('%s: %d users'%(a,len(us)))
    print('   '+', '.join(us[:60]))
