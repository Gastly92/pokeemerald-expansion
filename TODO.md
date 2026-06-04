## Upcoming Changes

### FEATURE_FRONTIER_MONS

- Trainers in the battle frontier will use an expanded list of competitive mons.
- https://raw.githubusercontent.com/Skeli789/Complete-Fire-Red-Upgrade/refs/heads/master/src/Tables/battle_tower_spreads.h

### FEATURE_INNATE_ABILITIES

- Some species have innate abilities that are always active in addition to their single chosen ability.
- For example, Flygon always has the innate ability levitate.

### FEATURE_NEW_TYPES

- Some species have brand new type combinations. Galarian Ponyta/Rapidash are Fire/Fairy.

### FRONTIER_BOSS_BATTLES

- A boss battle will occur every 10 battles and reward BP equal to the current streak. Brain battles take precedence.

### FRONTIER_NO_BANS

- No species are banned from frontier challenges.

## Bugs/Issues

- After a mon hurts themselves in confusion, the volatile should persist and they should snap out of confusion when they make their next move. That way that can't get locked into confusion by a faster foe. AI needs to understand this behavior. Another idea, when a mon is confused, the next time they attack, they hurt themselves before dealing damage. Then stay confused until they snap out next turn to avoid confusion lock. and perhaps this effect only lasts 2 turns.
- King's rock gets used up against a quick claw user even though no flinch occurs. It should only get used if it causes a flinch.
- Moves can pull up a display to show accuracy info by hitting L, this can be replaced with projected PP cost?
- Battle factory help text still hard codes "three" in some places.
- Battle factory opponent preview should reveal upcoming team info.
- Battle info view should show current turn (damage multiplier)
- Battle info should support L to navigate left
- TV in frontier facilities no longer need to show level 50 info. Perhaps it can be replaced with dynamax mode?
