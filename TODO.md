## Upcoming Changes

### BUFF_SHELL_BELL

- Increase Shell Bell healing from 1/8 of damage dealt to 1/4.

### DETERMINISTIC_ABILITIES

- Stench can only attempt flinching on the first turn the battler is out.
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

### DETERMINISTIC_ACCURACY_EVASION

- Stat stage increases and decreases to accuracy and evasion no longer impact the accuracy of moves.
- Increases to accuracy recover 1 PP per stage when using moves that target an opposing mon.
- Decreases to accuracy increase the cost of moves that target an opposing mon by 1 PP per stage.
- Increases to evasion increase the cost of moves that target the evasive mon by 1 PP per stage.
- Decreases to evasion recover 1 PP per stage when using moves that target the less evasive mon.
- Accuracy and evasion modifiers continue to cancel each other out.
- All moves have 100% accuracy by default.
- Sleep moves that previously did not have 100% accuracy now behave like Yawn.
- Some moves have PP reduced or additional effects removed as they now have 100% accuracy.
- Moves that previously had 50% accuracy now behave like Hyper Beam and require a recharge turn.

### DETERMINISTIC_ADDITIONAL_EFFECTS

- Additional effects of moves always happen conditionally rather than randomly.
- For example, the move Fire Punch only leaves a burn if the move was super effective.
- Moves that raise all stats have been reduced to 1 PP.

### DETERMINISTIC_ATTRACTION

- Attract can effect any opposing mon rather than requiring the user to be the opposite gender of the target.
- Attract only lasts 2 turns rather than indefinitely.
- The volatile attract status disallows using moves that would damage the opponent while infatuated.
- Non-damaging moves used against the opponent or used on the battler function normally.

### DETERMINISTIC_CONFUSION

- Confusion always lasts 2 turns.
- Rather than battlers randomly hurting themselves in confusion, they only hurt themselves if they use an attacking move.
- If a battler uses a non attacking move, they snap out of their confusion immediately.

### DETERMINISTIC_DAMAGE

- Calculated damage is no longer multiplied by a random value 0.85 to 1.
- Instead damage is always multipled by 0.92 plus 0.01 for every turn that has passed in the battle.

### DETERMINISTIC_FLINCH

- Moves with a flinch effect only cause flinching if the target was not flinched on the previous turn.

### DETERMINISTIC_FOCUS_BAND

- Focus band no longer randomly activates 20% of the time, instead it only activates the first turn the holder is in battle, regardless of their health.

### DETERMINISTIC_MULTI_HIT

- Multi hit moves always hit 3 times.
- Holding a loaded dice allows multi hit moves to hit 6 times.

### DETERMINISTIC_OHKO

- OHKO moves no longer deal damage equal to the remaining health of the targeting mon.
- Instead they always deal damage equal to 50% of the target's maximum hp, minimum 1 damage.
- Maximum PP for these moves is reduced to 1.

### DETERMINISTIC_PARALYSIS

- Paralysis no longer causes a battler to randomly miss their turn.
- Paralysis no longer halves speed.
- Paralysis increases the PP usage of all moves used by a battler by 1.
- Paralysis decreases the priority of all moves used by a battler by 1.

### DETERMINISTIC_PROTECT

- Protect always fails if used in succession rather than having a small chance of working on second uses.

### DETERMINISTIC_QUICK_CLAW

- Quick Claw activates immediately and is then consumed rather than only activating 20% of the time.

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

### HIDE_SYNCHRONIZE_POP_UPS

- The Synchronize ability pop up appears even if nothing happens so hide it instead.

### UX_ICONS_LEFT

- Show type icons to the left of the player's mon rather than on the right.