"""Insert a new frontier set after a species' last existing set.

    python3 .claude/skills/line-review/tools/addset.py GRENINJA <<'EOF'
    { ...the new set block, indented as the file is... },
    EOF

Appending after the last set for that species keeps dex order without hand-
counting braces, and avoids the anchor-matching that breaks whenever a set
carries an optional field (.ball, .iv) the pattern did not expect.
"""
import sys,re
# usage: addset.py SPECIES <<< "new set block text"
p='src/fork/frontier_extended_mons.c'
t=open(p).read()
sp=sys.argv[1]
block=sys.stdin.read().rstrip('\n')
key='        .species = SPECIES_%s,\n'%sp
starts=[m.start() for m in re.finditer(re.escape(key),t)]
assert starts, 'no set for '+sp
last=starts[-1]
# closing of this set: the next "\n    },\n" at that indent
end=t.index('\n    },\n',last)+len('\n    },\n')
t=t[:end]+block+'\n'+t[end:]
open(p,'w').write(t)
print('inserted after last %s set'%sp)
