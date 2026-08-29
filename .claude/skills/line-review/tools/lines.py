"""List the evolutionary lines a National Dex range covers.

    python3 .claude/skills/line-review/tools/lines.py SPRIGATITO PECHARUNT

Arguments are NATIONAL_DEX_ names without the prefix, inclusive at both ends.
Lines are derived from .natDexNum and .evolutions in species_info, with species
sharing a dex number unioned so a creature's forms collapse into one line, and
whole lines pulled in even where members fall outside the range.

Check the output for holes before trusting it: the Gen 9 batch ran this and still
dropped Ogerpon because nothing verified that every number in the range was
covered by some line.
"""
import re,glob,sys
# Build evolutionary lines for a national-dex range, from the species_info tables.
txt=''.join(open(f,encoding='utf-8',errors='replace').read() for f in glob.glob('src/data/pokemon/species_info/*.h'))
blocks=re.findall(r'\[SPECIES_([A-Z0-9_]+)\] =(.*?)(?=\n    \[SPECIES_|\Z)',txt,re.S)
natdex={}; evos={}
for name,body in blocks:
    m=re.search(r'\.natDexNum\s*=\s*NATIONAL_DEX_([A-Z0-9_]+)',body)
    if m: natdex[name]=m.group(1)
    e=re.findall(r'\.evolutions = EVOLUTION\((.*?)\),\n',body,re.S)
    if e: evos[name]=re.findall(r'SPECIES_([A-Z0-9_]+)',e[0])
# dex order
order=[l.strip().rstrip(',') for l in open('include/constants/pokedex.h') if re.match(r'\s*NATIONAL_DEX_[A-Z0-9_]+,\s*$',l)]
order=[x.replace('NATIONAL_DEX_','') for x in order]
idx={d:i for i,d in enumerate(order)}
# union-find over evolution edges
parent={}
def find(x):
    parent.setdefault(x,x)
    while parent[x]!=x: parent[x]=parent[parent[x]]; x=parent[x]
    return x
def union(a,b):
    ra,rb=find(a),find(b)
    if ra!=rb: parent[ra]=rb
for a,bs in evos.items():
    for b in bs:
        if a in natdex and b in natdex: union(a,b)
# forms of one species share a natDexNum -- same line
bydex={}
for n,d in natdex.items(): bydex.setdefault(d,[]).append(n)
for d,mem in bydex.items():
    for m in mem[1:]: union(mem[0],m)
lo,hi=sys.argv[1],sys.argv[2]
inrange=[n for n,d in natdex.items() if idx.get(d,-1)>=idx[lo] and idx.get(d,-1)<=idx[hi]]
groups={}
for n in inrange:
    groups.setdefault(find(n),set()).add(n)
# add out-of-range line members
for n,d in natdex.items():
    r=find(n)
    if r in groups: groups[r].add(n)
out=[]
for r,mem in groups.items():
    mem=sorted(mem,key=lambda n: idx.get(natdex[n],9999))
    out.append((idx.get(natdex[mem[0]],9999),mem))
out.sort()
print("LINES: %d"%len(out))
for i,(k,mem) in enumerate(out,1):
    dexes=[]
    for m in mem:
        if natdex[m] not in dexes: dexes.append(natdex[m])
    print("%3d. %s"%(i,' / '.join(dexes)))
