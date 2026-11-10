# Multi-hit moves pay per strike (`NERF_MULTI_HIT_PP_PER_HIT`)

Every strike of a multi-hit move costs PP, not just the first. The flag lives in
`include/config/nerf.h`; the code is `src/fork/multi_hit_pp.c`, and the tests are
`test/fork/multi_hit_pp.c`.

## Why

Skill Link turns a 3-hit move (the `DETERMINISTIC_MOVE_RESULTS` count) into a 5-hit one for
free, and several species carry Skill Link and Technician together as innates (Cinccino,
Ambipom, Maushold, Mega Beedrill...). The coverage gate requires every species to carry its own
innate-capable vanilla abilities, so dropping one of the two was not an option. Charging per
strike makes Skill Link a trade instead of a free boost: the same damage per PP, delivered
faster, with the move running dry sooner.

## What pays

- **Moves:** every multi-hit move. That means the 2-5 hitters, fixed multi-strike moves
  (Double Kick, Triple Axel, Dragon Darts, Surging Strikes...), Population Bomb and Beat Up.
  Parental Bond is an ability, not a multi-hit move, so its second hit stays free.
- **Per strike:** each strike after the first pays the move's full per-use cost again, so
  Pressure, the paralysis tax, Halo's upkeep and the `DETERMINISTIC_ACCURACY_EVASION` stage
  economy and item taxes all apply per strike. A net accuracy refund is also repeated per
  strike. The one-shot Micle Berry refund is not.
- **Called and copied moves:** a move called by Metronome, Sleep Talk, Assist, Copycat, Me
  First, Mirror Move or Nature Power, or copied by Dancer, spends only the caller's one PP, and
  its strikes are free.

## When it pays

Each strike is paid for when it is thrown, never up front:

- A KO on strike 3 of 5 pays for 3 strikes.
- A first strike that Protect blocks (or an immunity stops) ends the move, so it pays 1.
- A strike the move cannot pay for in full is not thrown. A 5-hit move with 2 PP left hits
  twice, and "The Pokémon was hit 2 times!" says so. The first strike always goes out, and
  spends whatever PP is left if it cannot pay in full, as every move does.

## What the menu, the preview and the AI see

- **Move menu:** the power field shows the strikes the move can afford (`PWR: 25×5`, or
  `25×2` on 2 PP), and under `DETERMINISTIC_ACCURACY_EVASION` the PP field shows the total
  for all of them. A random 2-5 roll projects its maximum. An armed Z-Move or Max Move is a
  single strike and shows no count.
- **Damage preview and AI:** the AI's damage simulation (`CalcDynamicMoveDamage`, and the
  Triple Kick loop in `AI_CalcDamage`) prices only the strikes that can be paid for. The
  damage preview runs on that simulation, so it shrinks too, and the AI stops treating a
  1 PP Skill Link move as a 5-hit nuke.

## Where it hooks

| Hook | File | What it does |
| --- | --- | --- |
| `CancelerPPDeduction` | `src/battle_move_resolution.c` | Resets the state on entry, and records the use's cost after paying for the first strike |
| `MoveEndMultihitMove` | `src/battle_move_resolution.c` | Pays for each further strike as its last condition, and resets the state once the move ends |
| `CalcDynamicMoveDamage`, `AI_CalcDamage` | `src/battle_ai_util.c` | Caps the simulated damage at the strikes it can afford |
| `MoveSelectionDisplayMoveDescription` | `src/battle_controller_player.c` | `×N` beside the power, and the total PP |

The strike counter is not trimmed up front, because Triple Kick's escalating power and Dragon
Darts' retargeting both read `gMultiHitCounter`. The loop just stops early instead. The
projection (`GetNominalStrikeCount`) mirrors `CancelerMultihitMoves`, so keep them in sync.
