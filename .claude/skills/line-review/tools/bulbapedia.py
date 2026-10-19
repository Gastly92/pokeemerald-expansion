"""Print a species' Biology section from Bulbapedia, and optionally its media sections.

    python3 -I .claude/skills/line-review/tools/bulbapedia.py NINETALES VENUSAUR_MEGA
    python3 -I .claude/skills/line-review/tools/bulbapedia.py --media NINETALES
    python3 -I .claude/skills/line-review/tools/bulbapedia.py --media --full PIKACHU

Names are the bare species constants line.py takes; a form constant resolves to
its base species' page through the repo's .speciesName, which matches Bulbapedia's
titles (Mr. Mime, Nidoran♀, Type: Null). Biology includes its Forms subsections.
--media adds "In animation" and "In the manga", capped at 8000 characters each
unless --full is given. Pages are fetched once as wikitext through the MediaWiki
API and cached in ~/.cache/line-review-bulbapedia (delete that directory to
refresh).

This is the wider-media check the rubric requires (SKILL.md Step 4), read instead
of recalled. Cite it as Bulbapedia, never as the repo's .description, which stays
the dex text a proposal quotes. Biology prose routinely mixes the games with the
anime, so name the medium a cited action comes from, not just the wiki.
"""
import glob, html, json, os, re, sys, urllib.parse, urllib.request

CACHE = os.path.expanduser('~/.cache/line-review-bulbapedia')
API = ('https://bulbapedia.bulbagarden.net/w/api.php?action=parse&format=json'
       '&redirects=1&prop=wikitext&page=%s')
MEDIA = ('In animation', 'In the manga')
# boilerplate subsections dropped from a section, at whatever depth they sit
DROP = {'Biology': ('Evolution',), 'In animation': ('Pokédex entries',),
        'In the manga': ('Pokédex entries',)}
CAP = 8000

names = {}
for f in glob.glob('src/data/pokemon/species_info/*.h'):
    t = open(f, encoding='utf-8', errors='replace').read()
    for m in re.finditer(r'\[SPECIES_(\w+)\]\s*=\s*\{.*?\.speciesName = _\("([^"]*)"\)', t, re.S):
        names.setdefault(m.group(1), m.group(2))


def species_name(arg):
    if arg in names:
        return names[arg]
    print('%s: not in species_info, guessing the page title' % arg, file=sys.stderr)
    return arg.replace('_', ' ').title()


def load(name):
    os.makedirs(CACHE, exist_ok=True)
    path = os.path.join(CACHE, re.sub(r'[^\w-]', '_', name) + '.wiki')
    if not os.path.exists(path):
        url = API % urllib.parse.quote(name + ' (Pokémon)')
        try:
            req = urllib.request.Request(url, headers={'User-Agent': 'line-review/1.0'})
            with urllib.request.urlopen(req, timeout=60) as r:
                data = json.load(r)
            text = data['parse']['wikitext']['*']
        except Exception as e:
            print('%s: fetch failed (%s)' % (name, e), file=sys.stderr)
            return ''
        with open(path, 'w', encoding='utf-8') as f:
            f.write(text)
    with open(path, encoding='utf-8') as f:
        return f.read()


def span(wiki, title, level=None):
    """(start, end) of section `title`, heading and subsections included; any level if None."""
    eq = '=' * level if level else '=+'
    m = re.search(r'^(%s)\s*%s\s*\1\s*$' % (eq, re.escape(title)), wiki, re.M)
    if not m:
        return None
    end = re.search(r'^={2,%d}[^=]' % len(m.group(1)), wiki[m.end():], re.M)
    return m.start(), m.end() + end.start() if end else len(wiki)


def section(wiki, title):
    """The body of level-2 section `title`."""
    s = span(wiki, title, 2)
    return wiki[s[0]:s[1]].split('\n', 1)[-1] if s else ''


def drop(text, title):
    """`text` without every subsection called `title`."""
    while True:
        s = span(text, title)
        if not s:
            return text
        text = text[:s[0]] + text[s[1]:]


def template(m):
    parts = [p for p in m.group(1).split('|') if '=' not in p]
    if len(parts) < 2:
        # {{Ash}}, {{TRT}} name a character; {{left clear}} and its kin are layout
        return parts[0] if parts and parts[0][:1].isupper() else ''
    # {{OBP|Pokémon|species}} and {{tt|text|tooltip}} show their first argument;
    # {{p|..}}, {{m|..}}, {{wp|..}}, {{DL|page|anchor|text}} show their last.
    return parts[1] if parts[0].strip().lower() in ('obp', 'tt') else parts[-1]


def clean(w):
    w = re.sub(r'<!--.*?-->', '', w, flags=re.S)
    w = re.sub(r'<ref[^>]*/>|<ref[^>]*>.*?</ref>', '', w, flags=re.S)
    w = re.sub(r'^\{\|.*?^\|\}', '', w, flags=re.S | re.M)
    while True:
        n = re.sub(r'\[\[(?:File|Image):(?:[^\[\]]|\[\[[^\[\]]*\]\])*\]\]', '', w)
        n = re.sub(r'\{\{([^{}]*)\}\}', template, n)
        if n == w:
            break
        w = n
    w = re.sub(r'\[\[(?:[^|\]]*\|)?([^\]]*)\]\]', r'\1', w)
    w = re.sub(r'\[https?://\S+ ([^\]]*)\]', r'\1', w)
    w = re.sub(r"'{2,}", '', w)
    w = re.sub(r'<[^>]+>', '', w)
    # table rows, and the closers of tables whose opener sat inside a template
    w = '\n'.join(l for l in w.split('\n') if not re.match(r'\s*(\{\||\||!)', l))
    w = html.unescape(w)
    w = re.sub(r'\n{3,}', '\n\n', w)
    return w.strip()


args = sys.argv[1:]
media = '--media' in args
full = '--full' in args
for arg in [a for a in args if not a.startswith('--')]:
    name = species_name(arg)
    print('== %s (Bulbapedia: %s)' % (arg, name))
    wiki = load(name)
    if not wiki:
        continue
    for title in ('Biology',) + (MEDIA if media else ()):
        text = section(wiki, title)
        for sub in DROP.get(title, ()):
            text = drop(text, sub)
        text = clean(text)
        if not full and len(text) > CAP:
            text = text[:CAP] + '\n  [... truncated; rerun with --full]'
        print('--- %s' % title)
        print(text or '  (no %s section)' % title)
