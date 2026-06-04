## Upcoming Changes

### DETERMINISTIC_ABILITIES

- Stench can only attempt flinching on the first turn the battler is out. Takes effect before king's rock like items get consumed.
- Static always attempts to set paralysis on contact.
- Poison Point and Poison Touch always attempt to set poison on contact.
- Cute charm always attempts to activate on contact, regardless of gender.
- Flame body always attempts to set burn on contact.
- Effect Spore always attempts to set drowsy on contact.
- Shed Skin always cures status conditions at the end of the turn.
- Cursed Body always attempts to disable the used move.
- Healer always heals the always status condition at the end of the turn.
- Toxic Chain always attempts to set badly poisoned status.
- Quick Draw only activates the first turn the user is out.
- Rivalry activates when battlers are of the same type.
- Harvest always recovers used berries and also heals for 1/16 of max HP in the sun when activating.
- Moody compares raw stat values: raise the lowest +2, lower the highest -1, ties raise/lower all. Always ignores acc/evasion.
- Pickup recovers the first valid item: self, then partner, then opposing foe, then across foe.
- Trace copies the directly-opposing foe's ability in doubles (fallback to the other foe).
- Forewarn tie-breaks by move slot order (1-4) instead of randomly.
- Overworld ability RNG (encounter/pickup bias) deferred until the Battle Pyramid survival rework.

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

### FEATURE_NEW_TYPES

- Some species have brand new type combinations.

### FRONTIER_BOSS_BATTLES

- A boss battle will occur every 10 battles.

### FRONTIER_NO_BANS

- No species are banned from frontier challenges.
