## Upcoming Changes

### DETERMINISTIC_ATTRACTION

- Attract can effect any opposing mon rather than requiring the user to be the opposite gender of the target.
- Attract only lasts 2 turns rather than indefinitely.
- The volatile attract status disallows using moves that would damage the opponent while infatuated.
- Non-damaging moves used against the opponent or used on the battler function normally.

### DETERMINISTIC_CONFUSION

- Confusion always lasts 2 turns.
- Rather than battlers randomly hurting themselves in confusion, they only hurt themselves if they use an attacking move.
- If a battler uses a non attacking move, they snap out of their confusion immediately.

### DETERMINISTIC_MULTI_HIT

- Multi hit moves always hit 3 times.
- Holding a loaded dice allows multi hit moves to hit 6 times.

### DETERMINISTIC_PROTECT

- Protect always fails if used in succession rather than having a small chance of working on second uses.

### DETERMINISTIC_RAMPAGE

- Rampage moves always last 2 turns.

### DETERMINISTIC_SLEEP

- Sleep always lasts 2 turns rather than a randomly choosing 2, 3, 4, or 5 turns.
- Sleep moves cause drowsiness rather than immediately causing sleep.

### DETERMINISTIC_SPEED_TIES

- Speed ties are no longer determined randomly when battles have equal speeds.
- Instead a check on raw base speed occurs first, whichever battler has the greater raw speed goes first.
- If raw speeds are the same, then a freshness check occurs, whichever battler has the greatest percentage of health remaining goes first.
- If health percentages are the same, then a weight check occurs, whichever battle weighs less goes first.
- If no checks pass, then the game reverts to randomly determining the speed tie.

### DETERMINISTIC_WRAP

- Wrap moves always last 4 turns, and 7 if holding a Grip Claw.

### FEATURE_DEBUG

- Allow picking gender when creating a mon from the debug menu.
- Created mons are generated with maximum PP for all of their moves.

### FEATURE_FRONTIER_MONS

- Trainers in the battle frontier will use an expanded list of competitive mons.
- https://raw.githubusercontent.com/Skeli789/Complete-Fire-Red-Upgrade/refs/heads/master/src/Tables/battle_tower_spreads.h

### FEATURE_INNATE_ABILITIES

- Some species have innate abilities that are always active in addition to their single chosen ability.
- For example, Flygon always has the innate ability levitate.
- Implemented (trait layer): `FEATURE_INNATE_ABILITIES` flag, fork-owned species->innate
  table (`src/innate_abilities.c`), and the `BattlerHasAbility()` predicate wired
  into the field/side queries, the grounding spine, trapping, and the discrete
  `GetBattlerAbility(...)==` trait checks. See `FORK.md` for the full status.
- Remaining follow-up: drive **active** on-switch-in innates (Intimidate, weather,
  etc.) by iterating innates in the ability-activation dispatch (not just the trait
  predicate), and convert the cached-local `if (ability == X)` checks.

### FEATURE_NEW_TYPES

- Some species have brand new type combinations.

### FRONTIER_BOSS_BATTLES

- A boss battle will occur every 10 battles.

### FRONTIER_NO_BANS

- No species are banned from frontier challenges.

## Bugs/Issues

- Moves can pull up a display to show accuracy info, this can be replaced with PP cost (projected?)
- Battle factory help text still hard codes "three" in some places.
- Battle factory opponent preview should reveal upcoming team info.
- Battle info view should show current turn (damage multiplier)
- Battle info should support L to navigate left

