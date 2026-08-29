"""Print a species' types, base stats, canon abilities and dex description.

    python3 .claude/skills/line-review/tools/dex.py NINETALES

The .description field is the source of truth for flavor evidence and the thing a
proposal quotes verbatim -- never recalled flavor, which frequently differs from
what this repo actually ships.
"""
import re,sys,glob
files=sorted(glob.glob('src/data/pokemon/species_info/*.h'))
txt={}
for f in files:
    txt[f]=open(f,encoding='utf-8',errors='replace').read()
def show(sp):
    key='[SPECIES_%s] ='%sp
    for f,t in txt.items():
        i=t.find(key)
        if i<0: continue
        # find the end of this block: next "\n    [SPECIES_" at same indent
        j=t.find('\n    [SPECIES_',i+1)
        blk=t[i:j if j>0 else i+9000]
        d=re.search(r'\.description = COMPOUND_STRING\((.*?)\),\n',blk,re.S)
        desc=' '.join(re.findall(r'"([^"]*)"',d.group(1))).replace('\\n',' ') if d else '??'
        st=re.search(r'\.baseHP\s*=\s*(\d+).*?\.baseAttack\s*=\s*(\d+).*?\.baseDefense\s*=\s*(\d+).*?\.baseSpeed\s*=\s*(\d+).*?\.baseSpAttack\s*=\s*(\d+).*?\.baseSpDefense\s*=\s*(\d+)',blk,re.S)
        ty=re.search(r'\.types = MON_TYPES\(([^)]*)\)',blk)
        ab=re.search(r'\.abilities = \{([^}]*)\}',blk,re.S)
        print('### %s  types=%s'%(sp, ty.group(1).replace('TYPE_','') if ty else '?'))
        if st: print('  HP%s Atk%s Def%s Spe%s SpA%s SpD%s'%st.groups())
        print('  abilities: %s'%(' '.join(ab.group(1).split()).replace('ABILITY_','') if ab else '?'))
        print('  dex: %s'%desc.strip())
        return
    print('### %s NOT FOUND'%sp)
for sp in sys.argv[1:]:
    show(sp)
