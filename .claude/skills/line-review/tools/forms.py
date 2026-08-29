"""Resolve a species name to every constant that shares its id or prefix.

    python3 .claude/skills/line-review/tools/forms.py AEGISLASH MINIOR

Run this BEFORE concluding a species has no innate row or no frontier set. Rows
and sets are routinely keyed on a form constant a bare name aliases to, and the
reverse -- SPECIES_ALCREMIE is _STRAWBERRY_VANILLA_CREAM, SPECIES_MINIOR chains
two hops to _METEOR_RED. Prints "NO SUCH CONSTANT" for a name that does not
exist, which is how a row written against a made-up constant gets caught before
it reaches the compiler.
"""
import re,sys
# usage: forms.py BASENAME...  -> every SPECIES_ constant beginning with that name, with its id
t=open('include/constants/species.h').read()
ent=re.findall(r'SPECIES_([A-Z0-9_]+)\s*=\s*([A-Z0-9_]+)',t)
val={}
for n,v in ent:
    val[n]=v
for base in sys.argv[1:]:
    hits=[(n,v) for n,v in ent if n==base or n.startswith(base+'_')]
    if not hits: print('%s: NO SUCH CONSTANT'%base); continue
    print('%s:'%base)
    for n,v in hits:
        print('   SPECIES_%-34s = %s'%(n,v))
