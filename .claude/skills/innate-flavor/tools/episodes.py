"""Print what a species actually does in its major anime episodes and movies.

    python3 -I .claude/skills/innate-flavor/tools/episodes.py VENUSAUR [--max 20]

bulbapedia.py gives a one-line summary per appearance; this follows each episode
linked under "Major appearances" in the species page's "In animation" section, fetches
that episode's (or movie's) Bulbapedia page, and prints only the plot paragraphs
that name the species. Minor appearances are skipped: they are one-liners, and a
species like Pikachu has hundreds. --max caps how many episodes are fetched (default 20,
in page order) and the rest are listed by title so nothing is silently dropped.

Episode pages are cached alongside the species pages (~/.cache/line-review-bulbapedia).
Cite what this prints as Bulbapedia with the episode code and title.
"""
import json, os, re, sys, urllib.parse, urllib.request

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                '..', '..', 'line-review', 'tools'))
import bulbapedia as bp  # noqa: E402

PLOT = ('Plot', 'Synopsis', 'Summary')
LINK = re.compile(r'\[\[((?:EP|AG|DP|BW|XY|SM|JN|HZ|PH|M)\d+)\|([^\]]+)\]\]')


def load_page(title):
    os.makedirs(bp.CACHE, exist_ok=True)
    path = os.path.join(bp.CACHE, 'page_' + re.sub(r'[^\w-]', '_', title) + '.wiki')
    if not os.path.exists(path):
        try:
            req = urllib.request.Request(bp.API % urllib.parse.quote(title),
                                         headers={'User-Agent': 'line-review/1.0'})
            with urllib.request.urlopen(req, timeout=60) as r:
                text = json.load(r)['parse']['wikitext']['*']
        except Exception as e:
            print('%s: fetch failed (%s)' % (title, e), file=sys.stderr)
            return ''
        with open(path, 'w', encoding='utf-8') as f:
            f.write(text)
    with open(path, encoding='utf-8') as f:
        return f.read()


def major_episodes(wiki):
    anim = bp.section(wiki, 'In animation')
    s = bp.span(anim, 'Major appearances')
    text = anim[s[0]:s[1]] if s else ''
    seen, out = set(), []
    for code, title in LINK.findall(text):
        if code not in seen:
            seen.add(code)
            out.append((code, title))
    return out


def main():
    args = sys.argv[1:]
    cap = 20
    if '--max' in args:
        i = args.index('--max')
        cap = int(args[i + 1])
        del args[i:i + 2]
    for arg in args:
        name = bp.species_name(arg)
        print('== %s (Bulbapedia: %s) — major anime appearances' % (arg, name))
        eps = major_episodes(bp.load(name))
        if not eps:
            print('  (no episodes linked under Major appearances)')
        for code, title in eps[:cap]:
            page = load_page(code)
            plot = ''
            for t in PLOT:
                plot = bp.section(page, t)
                if plot:
                    break
            paras = [p for p in bp.clean(plot).split('\n') if name.lower() in p.lower()]
            print('--- %s %s' % (code, title))
            print('\n'.join(paras) if paras else '  (plot does not name %s)' % name)
        if len(eps) > cap:
            print('--- not fetched (over --max %d): %s' % (
                cap, '; '.join('%s %s' % e for e in eps[cap:])))


if __name__ == '__main__':
    main()
