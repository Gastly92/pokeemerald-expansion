## Ability reference for the extended frontier roster

**This doc has been retired as a progress tracker** — the innate-abilities feature
is complete (every ability below is resolved), so there is no more "pending" state
to track. It is **repurposed** as the ability reference for filling out the
extended frontier roster (`src/fork/frontier_extended_mons.c`) and its companion
overrides (`src/fork/species_ability_overrides.c`). When authoring a set or a
line review, use it to place each ability into one of two buckets:

Legend:
- :white_check_mark: **implemented innate** — the ability is wired and can be
  handed to a species as an always-on innate in `src/fork/innate_abilities.c`
  (the allowlist / source of truth is `sImplementedInnates[]` in
  `test/fork/innate_abilities.c`). It is *not* a stable override pick: a set's
  chosen `.ability` must never duplicate one of the species' own innates.
- :x: **rejected as an innate** — deliberately never wired as an innate, so it is
  a **stable species-ability-override pick**: a set can select it as its chosen
  ability without it ever colliding with a future innate. Prefer these for
  `species_ability_overrides.c` rows (see that file's "pick a stable chosen
  ability" note and [`LINE_REVIEW.md`](LINE_REVIEW.md) Step 2).

Every ability is resolved (no pending rows remain). See
[`INNATE_ABILITIES.md`](INNATE_ABILITIES.md) for the per-ability wiring reference
and the rationale for the `:x:` set.

| Use | Ability | Description |
| :---: | :---: | :--- |
| :white_check_mark: | Stench | When the Pokémon deals damage with its moves, there is a 10% chance that targets will flinch. |
| :x: | Drizzle | Summons rain for 5 turns when the Pokémon enters a battle. |
| :white_check_mark: | Speed Boost | Boosts the Pokémon's Speed stat by 1 stage at the end of every turn. |
| :white_check_mark: | Battle Armor | Attacks landed on the Pokémon will never be critical hits. |
| :white_check_mark: | Sturdy | If the Pokémon has full HP and takes damage from a move that would knock it out in one hit, it will endure the hit with 1 HP. The Pokémon is also immune to one-hit KO moves. |
| :x: | Damp | All Pokémon become unable to use explosive moves. Explosive Abilities also fail to trigger. |
| :white_check_mark: | Limber | The Pokémon cannot be paralyzed. |
| :white_check_mark: | Sand Veil | Boosts the Pokémon's evasiveness by 25% in a sandstorm. |
| :x: | Static | When the Pokémon is hit by a contact move, the attacker has a 30% chance of being paralyzed. |
| :x: | Volt Absorb | Electric-type moves do not work on the Pokémon. Instead, they restore 1/4 of its max HP. |
| :x: | Water Absorb | Water-type moves do not work on the Pokémon. Instead, they restore 1/4 of its max HP. |
| :white_check_mark: | Oblivious | The Pokémon cannot gain the Infatuated or Taunted statuses and is unaffected by Intimidate. |
| :x: | Cloud Nine | Eliminates the effects of weather. |
| :white_check_mark: | Compound Eyes | Boosts the accuracy of the Pokémon's moves by 30%. |
| :white_check_mark: | Insomnia | The Pokémon cannot become drowsy or be put to sleep. |
| :x: | Color Change | The Pokémon's type becomes the type of the move used on it. |
| :white_check_mark: | Immunity | The Pokémon cannot be poisoned or badly poisoned. |
| :x: | Flash Fire | Fire-type moves do not work on the Pokémon. Instead, they give the Pokémon the Flash Fire status. |
| :white_check_mark: | Shield Dust | The Pokémon is immune to additional effects from attacks. |
| :white_check_mark: | Own Tempo | The Pokémon cannot become confused and is unaffected by Intimidate. |
| :white_check_mark: | Suction Cups | The Pokémon is unaffected by the moves and held items of other Pokémon that would force it to switch out of battle. |
| :white_check_mark: | Intimidate | When the Pokémon enters a battle, it lowers the Attack stats of opponents by 1 stage. |
| :white_check_mark: | Shadow Tag | Opponents cannot be switched out of battle. |
| :white_check_mark: | Rough Skin | When the Pokémon is hit by a contact move, the attacker takes damage equal to 1/8 of its max HP. |
| :x: | Wonder Guard | Its mysterious power only lets supereffective moves hit the Pokémon. |
| :white_check_mark: | Levitate | The Pokémon floats off the ground, making it immune to Ground-type moves, as well as the Spikes, Toxic Spikes, and Sticky Web statuses. |
| :x: | Effect Spore | Contact with the Pokémon may inflict poison, sleep, or paralysis on the attacker. |
| :x: | Synchronize | If the Pokémon is burned, paralyzed, poisoned, or badly poisoned by another Pokémon's move or Ability, that Pokémon will also be inflicted with the same status condition. |
| :white_check_mark: | Clear Body | The Pokémon's stats cannot be lowered by other Pokémon's moves or Abilities. |
| :white_check_mark: | Natural Cure | The Pokémon's status conditions are cured when it switches out of battle. |
| :x: | Lightning Rod | The Pokémon draws in all Electric-type moves. These moves do not work on the Pokémon. Instead, they boost its Sp. Atk stat by 1 stage. |
| :white_check_mark: | Serene Grace | Raises the likelihood of additional effects occurring when the Pokémon uses its moves. |
| :white_check_mark: | Swift Swim | Doubles the Pokémon's Speed stat in rain. |
| :white_check_mark: | Chlorophyll | Doubles the Pokémon's Speed stat in harsh sunlight. |
| :white_check_mark: | Illuminate | The Pokémon ignores changes to targets' evasiveness and its accuracy cannot be lowered. |
| :x: | Trace | When the Pokémon enters a battle, it changes its Ability to match that of an opponent. |
| :white_check_mark: | Huge Power | Doubles the power of the Pokémon's physical moves. |
| :x: | Poison Point | When the Pokémon is hit by a contact move, the attacker has a 30% chance of being poisoned. |
| :white_check_mark: | Inner Focus | The Pokémon never flinches when attacked and is unaffected by Intimidate. |
| :white_check_mark: | Magma Armor | The Pokémon cannot be frozen. |
| :white_check_mark: | Water Veil | The Pokémon's water veil prevents it from being burned. |
| :white_check_mark: | Magnet Pull | Prevents Steel-type Pokémon from fleeing by pulling them in with magnetism. |
| :x: | Soundproof | The Pokémon is immune to sound-based moves. |
| :white_check_mark: | Rain Dish | The Pokémon has 1/16 of its max HP restored at the end of every turn in rain. |
| :x: | Sand Stream | Summons a sandstorm for 5 turns when the Pokémon enters a battle. |
| :white_check_mark: | Pressure | Causes opponents to expend 1 more PP when using moves against the Pokémon. |
| :white_check_mark: | Thick Fat | Halves the damage the Pokémon takes from Fire- and Ice-type moves. |
| :white_check_mark: | Early Bird | The Pokémon awakens from sleep twice as fast as other Pokémon. |
| :x: | Flame Body | When the Pokémon is hit by a contact move, the attacker has a 30% chance of being burned. |
| :x: | Run Away | Enables a sure getaway from wild Pokémon. |
| :white_check_mark: | Keen Eye | The Pokémon ignores changes to targets' evasiveness and its accuracy cannot be lowered. |
| :white_check_mark: | Hyper Cutter | The Pokémon's Attack stat cannot be lowered by other Pokémon's moves or Abilities. |
| :white_check_mark: | Pickup | If the Pokémon is not already holding an item, at the end of the turn it will pick up an item that was consumed by another Pokémon. |
| :x: | Truant | Each time the Pokémon uses a move, it spends the next turn loafing around. |
| :x: | Hustle | When the Pokémon uses physical moves, its Attack stat is boosted by 50%, but its accuracy is lowered by 20%. |
| :white_check_mark: | Cute Charm | When the Pokémon is hit by a contact move, the attacker has a 30% chance of gaining the Infatuated status if the attacker and the Pokémon are of opposite genders. |
| :x: | Plus | Boosts the Pokémon's Sp. Atk stat by 50% if an ally with the Plus or Minus Ability is also in battle. |
| :x: | Minus | Boosts the Pokémon's Sp. Atk stat by 50% if an ally with the Plus or Minus Ability is also in battle. |
| :x: | Forecast | The Pokémon transforms with the weather to change its type to Water, Fire, or Ice. |
| :white_check_mark: | Sticky Hold | The Pokémon's held item cannot be stolen or removed by other Pokémon. |
| :white_check_mark: | Shed Skin | The Pokémon has a 30% chance of curing its own status conditions at the end of every turn. |
| :white_check_mark: | Guts | When the Pokémon has a status condition, its Attack stat is boosted by 50%. Being burned does not halve the damage dealt by the Pokémon's physical moves. |
| :white_check_mark: | Marvel Scale | When the Pokémon has a status condition, its Defense stat is boosted by 50%. |
| :white_check_mark: | Liquid Ooze | The strong stench of the Pokémon's oozed liquid damages attackers that use HP-draining moves. |
| :white_check_mark: | Overgrow | Boosts the power of the Pokémon's Grass-type moves by 50% when its HP drops to 1/3 or less of its max. |
| :white_check_mark: | Blaze | Boosts the power of the Pokémon's Fire-type moves by 50% when its HP drops to 1/3 or less of its max. |
| :white_check_mark: | Torrent | Boosts the power of the Pokémon's Water-type moves by 50% when its HP drops to 1/3 or less of its max. |
| :white_check_mark: | Swarm | Boosts the power of the Pokémon's Bug-type moves by 50% when its HP drops to 1/3 or less of its max. |
| :white_check_mark: | Rock Head | The Pokémon will not lose HP due to recoil damage from its moves. |
| :x: | Drought | Summons harsh sunlight for 5 turns when the Pokémon enters a battle. |
| :white_check_mark: | Arena Trap | Prevents opposing Pokémon from fleeing from battle. |
| :white_check_mark: | Vital Spirit | The Pokémon cannot become drowsy or be put to sleep. |
| :white_check_mark: | White Smoke | The Pokémon's stats cannot be lowered by other Pokémon's moves or Abilities. |
| :white_check_mark: | Pure Power | Doubles the power of the Pokémon's physical moves. |
| :white_check_mark: | Shell Armor | Attacks landed on the Pokémon will never be critical hits. |
| :x: | Air Lock | Eliminates the effects of weather. |
| :white_check_mark: | Tangled Feet | Doubles the Pokémon's evasiveness if it is confused. |
| :x: | Motor Drive | Electric-type moves do not work on the Pokémon. Instead, they boost its Speed stat by 1 stage. |
| :x: | Rivalry | Boosts the power of the Pokémon's moves by 25% against targets of the same gender, and lowers it by 25% against targets of the opposite gender. The power remains the same as usual if either Pokémon's gender is unknown. |
| :white_check_mark: | Steadfast | When the Pokémon flinches, its Speed stat is boosted by 1 stage. |
| :white_check_mark: | Snow Cloak | Boosts the Pokémon's evasiveness by 25% in snow. |
| :white_check_mark: | Gluttony | If the Pokémon is holding a Berry to be eaten when its HP drops to 1/4 or less of its max, it will instead eat the Berry when its HP drops to 1/2 or less of its max. |
| :white_check_mark: | Anger Point | Boosts the Pokémon's Attack stat to its sixth stage when the Pokémon takes a critical hit. |
| :white_check_mark: | Unburden | Doubles the Pokémon's Speed stat when its held item is consumed or lost. |
| :white_check_mark: | Heatproof | Halves the damage the Pokémon takes from Fire-type moves and from being burned. |
| :x: | Simple | Doubles the Pokémon's stat changes. |
| :x: | Dry Skin | Water-type moves do not work on the Pokémon. Instead, they restore 1/4 of its max HP. However, the Pokémon takes 25% more damage from Fire-type moves. The Pokémon has 1/8 of its max HP restored at the end of every turn in rain, but it loses 1/8 of its max HP at the end of every turn in harsh sunlight. |
| :white_check_mark: | Download | The Pokémon compares an opposing Pokémon's Defense and Sp. Def stats before raising its own Attack or Sp. Atk stat — whichever will be more effective. |
| :white_check_mark: | Iron Fist | Boosts the power of the Pokémon's punching moves by 20%. |
| :white_check_mark: | Poison Heal | If poisoned or badly poisoned, the Pokémon has 1/8 of its max HP restored at the end of every turn instead of losing HP. |
| :white_check_mark: | Adaptability | Boosts the power of moves of the same type as the Pokémon by 100% instead of 50%. |
| :white_check_mark: | Skill Link | The Pokémon's multistrike moves always hit the maximum number of times. |
| :white_check_mark: | Hydration | Cures the Pokémon's status conditions at the end of every turn in rain. |
| :x: | Solar Power | In harsh sunlight, the Pokémon's Sp. Atk stat is boosted by 50%, but it loses 1/8 of its max HP at the end of every turn. |
| :white_check_mark: | Quick Feet | When the Pokémon has a status condition, its Speed stat is boosted by 50%. Being paralyzed does not lower the Pokémon's Speed stat. |
| :x: | Normalize | All the Pokémon's moves become Normal type. The power of those moves is boosted a little. |
| :white_check_mark: | Sniper | Boosts the power of the Pokémon's critical hits by 125% instead of 50%. |
| :white_check_mark: | Magic Guard | The Pokémon takes damage only from attacks. |
| :x: | No Guard | The accuracy of moves used both by and against the Pokémon becomes 100%. |
| :x: | Stall | The Pokémon's moves go last among moves of the same priority. |
| :white_check_mark: | Technician | Boosts the power of the Pokémon's moves by 50% if their power is 60 or less. |
| :white_check_mark: | Leaf Guard | The Pokémon is immune to status conditions in harsh sunlight. |
| :x: | Klutz | Items do not work when held by the Pokémon. |
| :white_check_mark: | Mold Breaker | The Pokémon's moves are unaffected by the Ability of the target (with certain exceptions). |
| :white_check_mark: | Super Luck | The Pokémon has a 1-stage Critical-Hit Ratio Boost. |
| :white_check_mark: | Aftermath | Attackers that knock out the Pokémon with a contact move take damage equal to 1/4 of their max HP. |
| :white_check_mark: | Anticipation | When the Pokémon enters a battle, it senses whether the opponents know any one-hit KO moves or moves that are super effective against it. |
| :white_check_mark: | Forewarn | When it enters a battle, the Pokémon can tell one of the moves an opposing Pokémon has. |
| :white_check_mark: | Unaware | The Pokémon ignores the target's stat changes when attacking, and ignores the attacker's stat changes when being attacked. It cannot ignore changes to the Speed stat, however. |
| :white_check_mark: | Tinted Lens | The Pokémon can use "not very effective" moves to deal regular damage. |
| :white_check_mark: | Filter | Reduces the damage the Pokémon takes from supereffective moves by 25%. |
| :x: | Slow Start | For five turns, the Pokémon's Attack and Speed stats are halved. |
| :white_check_mark: | Scrappy | The Pokémon can hit Ghost types with Normal- and Fighting-type moves. It is also unaffected by Intimidate. |
| :x: | Storm Drain | The Pokémon draws in all Water-type moves. Instead of taking damage from them, its Sp. Atk stat is boosted. |
| :white_check_mark: | Ice Body | The Pokémon has 1/16 of its max HP restored at the end of every turn in snow. |
| :white_check_mark: | Solid Rock | Reduces the damage the Pokémon takes from supereffective moves by 25%. |
| :x: | Snow Warning | Summons snow for 5 turns when the Pokémon enters a battle. |
| :x: | Honey Gather | The Pokémon may gather Honey after a battle. |
| :white_check_mark: | Frisk | When the Pokémon enters a battle, it identifies opponents' held items. |
| :white_check_mark: | Reckless | Boosts the power of the Pokémon's moves by 20% if they have recoil or crash damage. |
| :x: | Multitype | Changes the Pokémon's type to match the plate it holds. |
| :x: | Flower Gift | Boosts the Attack and Sp. Def stats of the Pokémon and its allies in harsh sunlight. **Form-tied** (`cantBeCopied`), like every transformation ability: the effect gates on Cherrim's Sunshine form, so it is inert on any other species — rejected as an innate, and *not* a usable override pick either. |
| :white_check_mark: | Bad Dreams | Damages opposing Pokémon that are asleep. |
| :white_check_mark: | Pickpocket | When the Pokémon is hit by a contact move, it will steal the held item of the attacker if it is not already holding an item. |
| :x: | Sheer Force | The Pokémon's moves lose their additional effects, but the power of those moves will be boosted by 30%. |
| :x: | Contrary | Reverses any stat changes affecting the Pokémon so that attempts to boost its stats instead lower them—and attempts to lower its stats will boost them. |
| :white_check_mark: | Unnerve | Makes opponents unable to eat Berries. |
| :white_check_mark: | Defiant | When the Pokémon has any of its stats lowered by an opponent, its Attack stat is boosted by 2 stages. |
| :x: | Defeatist | Halves the Pokémon's Attack and Sp. Atk stats when its HP drops to half or less. |
| :white_check_mark: | Cursed Body | When the Pokémon takes damage from a move, the attacker has a 30% chance of gaining the Move Disabled status for 4 turns. |
| :white_check_mark: | Healer | The Pokémon has a 50% chance of curing the status conditions of its allies at the end of every turn. |
| :white_check_mark: | Friend Guard | Reduces the damage allies take by 25%. |
| :x: | Weak Armor | When the Pokémon takes damage from a physical move, its Defense stat is lowered by 1 stage, but its Speed stat is boosted by 2 stages. |
| :white_check_mark: | Heavy Metal | Doubles the Pokémon's weight. |
| :white_check_mark: | Light Metal | Halves the Pokémon's weight. |
| :white_check_mark: | Multiscale | Halves the damage the Pokémon takes while its HP is full. |
| :white_check_mark: | Toxic Boost | Powers up physical moves when the Pokémon is poisoned. |
| :white_check_mark: | Flare Boost | Powers up special moves when the Pokémon is burned. |
| :white_check_mark: | Harvest | If the Pokémon has used a Berry, it has a 50% chance of creating another one at the end of every turn. In harsh sunlight, the Pokémon will definitely create a Berry. |
| :white_check_mark: | Telepathy | The Pokémon dodges attacks from its allies. |
| :x: | Moody | At the end of every turn, one of the Pokémon's stats will be boosted by 2 stages, but another will be lowered by 1 stage. |
| :white_check_mark: | Overcoat | The Pokémon takes no damage from sandstorms and is immune to moves and Abilities involving powder. |
| :x: | Poison Touch | When the Pokémon hits a target with a contact move, the target has a 30% chance of being poisoned. |
| :white_check_mark: | Regenerator | The Pokémon has 1/3 of its max HP restored when it switches out of battle. |
| :white_check_mark: | Big Pecks | The Pokémon's Defense stat cannot be lowered by other Pokémon's moves or Abilities. |
| :white_check_mark: | Sand Rush | Doubles the Pokémon's Speed stat in a sandstorm. |
| :white_check_mark: | Wonder Skin | Makes status moves more likely to miss the Pokémon. |
| :white_check_mark: | Analytic | Boosts the power of the Pokémon's moves by 30% when the Pokémon is the last to move that turn. |
| :x: | Illusion | The Pokémon enters battle disguised as the last Pokémon in its party. It reverts to its usual appearance when it takes damage from a move. |
| :x: | Imposter | The Pokémon transforms into the Pokémon in front of it. It also copies all of that Pokémon's stats apart from its HP. |
| :white_check_mark: | Infiltrator | When using its moves, the Pokémon ignores the effects of targets' Light Screen, Reflect, Aurora Veil, Safeguard, and substitutes. |
| :x: | Mummy | When the Pokémon is hit by a contact move, the attacker has its Ability changed to Mummy. |
| :white_check_mark: | Moxie | When the Pokémon knocks out a target with an attack, its Attack stat is boosted by 1 stage. |
| :white_check_mark: | Justified | When the Pokémon takes damage from a Dark-type move, its Attack stat is boosted by 1 stage. |
| :white_check_mark: | Rattled | The Pokémon gets scared when hit by a Dark-, Ghost-, or Bug-type attack or if intimidated, which boosts its Speed stat. |
| :white_check_mark: | Magic Bounce | Instead of being affected by other Pokémon's status moves, the Pokémon bounces them back at the user. |
| :x: | Sap Sipper | Grass-type moves do not work on the Pokémon. Instead, they boost its Attack stat by 1 stage. |
| :white_check_mark: | Prankster | Increases the priority of the Pokémon's status moves by 1 stage. |
| :white_check_mark: | Sand Force | Boosts the power of the Pokémon's Rock-, Ground-, and Steel-type moves by 30% in a sandstorm. |
| :white_check_mark: | Iron Barbs | The Pokémon's iron barbs damage the attacker if it makes direct contact. |
| :x: | Zen Mode | Changes the Pokémon's shape when its HP drops to half or less. |
| :x: | Victory Star | Boosts the accuracy of the Pokémon and its allies. |
| :white_check_mark: | Turboblaze | The Pokémon's moves are unimpeded by the Ability of the target. |
| :white_check_mark: | Teravolt | The Pokémon's moves are unimpeded by the Ability of the target. |
| :white_check_mark: | Aroma Veil | The Pokémon and its allies cannot gain the Infatuated, Taunted, Unable to Repeat, Move Disabled, Healing Prevented, or Encore statuses. |
| :white_check_mark: | Flower Veil | Grass-type allies are immune to status conditions and cannot have their stats lowered. |
| :white_check_mark: | Cheek Pouch | The Pokémon has 1/3 of its max HP restored when it eats a Berry, in addition to the Berry's usual effect. |
| :x: | Protean | Changes the Pokémon's type to the type of the move it's about to use. This works only once per time the Pokémon enters battle. |
| :white_check_mark: | Fur Coat | Halves the damage the Pokémon takes from physical moves. |
| :white_check_mark: | Magician | If the Pokémon is not already holding an item, it will steal the held item from targets it deals damage to with its moves. |
| :x: | Bulletproof | The Pokémon is immune to ball and bomb moves. |
| :white_check_mark: | Competitive | When the Pokémon has any of its stats lowered by an opponent, its Sp. Atk stat is boosted by 2 stages. |
| :white_check_mark: | Strong Jaw | Boosts the power of the Pokémon's biting moves by 50%. |
| :x: | Refrigerate | The Pokémon's Normal-type moves become Ice-type moves and their power is boosted by 20%. |
| :white_check_mark: | Sweet Veil | The Pokémon and its allies cannot become drowsy or be put to sleep. |
| :x: | Stance Change | The Pokémon changes into its Blade Forme when it attacks and changes into its Shield Forme when it uses the move King's Shield. |
| :white_check_mark: | Gale Wings | Increases the priority of the Pokémon's Flying-type moves by 1 stage while its HP is full. |
| :white_check_mark: | Mega Launcher | Boosts the power of the Pokémon's pulse moves by 50%. |
| :white_check_mark: | Grass Pelt | Boosts the Pokémon's Defense stat on Grassy Terrain. |
| :x: | Symbiosis | When an ally consumes an item, the Pokémon gives its own held item to that ally. |
| :white_check_mark: | Tough Claws | Boosts the power of the Pokémon's contact moves by 30%. |
| :x: | Pixilate | The Pokémon's Normal-type moves become Fairy-type moves and their power is boosted by 20%. |
| :white_check_mark: | Gooey | When the Pokémon is hit by a contact move, the attacker's Speed stat is lowered by 1 stage. |
| :x: | Aerilate | The Pokémon's Normal-type moves become Flying-type moves and their power is boosted by 20%. |
| :x: | Parental Bond | The parent and child attack one after the other. The power of the child's attacks is 1/4 of those of the parent. |
| :x: | Dark Aura | Powers up the Dark-type moves of all Pokémon on the field. |
| :x: | Fairy Aura | Boosts the power of the Fairy-type moves of all Pokémon on the field by 33%. |
| :x: | Aura Break | The effects of "Aura" Abilities are reversed to lower the power of affected moves. (Won't wire: its counterparts Dark Aura / Fairy Aura are `:x:`, so an innate clause would be dead code. See INNATE_ABILITIES.md.) |
| :x: | Primordial Sea | The Pokémon changes the weather to nullify Fire-type attacks. |
| :x: | Desolate Land | The Pokémon changes the weather to nullify Water-type attacks. |
| :x: | Delta Stream | The Pokémon changes the weather so that no moves are supereffective against the Flying type. |
| :white_check_mark: | Stamina | When the Pokémon takes damage from a move, its Defense stat is boosted by 1 stage. |
| :x: | Wimp Out | The Pokémon cowardly switches out when its HP drops to half or less. |
| :x: | Emergency Exit | The Pokémon, sensing danger, switches out when its HP drops to half or less. |
| :white_check_mark: | Water Compaction | Boosts the Defense stat sharply when the Pokémon is hit by a Water-type move. |
| :white_check_mark: | Merciless | The Pokémon's attacks become critical hits if the target is poisoned or badly poisoned. |
| :x: | Shields Down | When its HP drops to half or less, the Pokémon's shell breaks and it becomes aggressive. |
| :white_check_mark: | Stakeout | Doubles the damage dealt to a target that has just switched into battle. |
| :white_check_mark: | Water Bubble | Halves the damage the Pokémon takes from Fire-type moves and doubles the power of its Water-type moves. The Pokémon cannot be burned. |
| :white_check_mark: | Steelworker | Powers up Steel-type moves. |
| :white_check_mark: | Berserk | Boosts the Pokémon's Sp. Atk stat by 1 stage when an attack causes its HP to drop to 1/2 or less of its max. |
| :white_check_mark: | Slush Rush | Doubles the Pokémon's Speed stat in snow. |
| :white_check_mark: | Long Reach | None of the moves used by the Pokémon are considered contact moves. |
| :x: | Liquid Voice | The Pokémon's sound-based moves become Water-type moves. |
| :white_check_mark: | Triage | Gives priority to the Pokémon's healing moves. |
| :x: | Galvanize | Normal-type moves become Electric-type moves. The power of those moves is boosted a little. |
| :white_check_mark: | Surge Surfer | Doubles the Pokémon's Speed stat on Electric Terrain. |
| :x: | Schooling | When it has a lot of HP, the Pokémon forms a powerful school. It stops schooling when its HP is low. |
| :x: | Disguise | When the Pokémon is in its Disguised Form and would take damage from a move, it loses 1/8 of its max HP instead of taking the damage, then changes into its Busted Form. |
| :x: | Battle Bond | When the Pokémon knocks out a target, its bond with its Trainer is strengthened, and its Attack, Sp. Atk, and Speed stats are boosted. |
| :x: | Power Construct | Cells gather to aid the Pokémon when its HP drops to half or less, causing it to change into its Complete Forme. |
| :white_check_mark: | Corrosion | The Pokémon can poison or badly poison targets even if they're Steel or Poison types. |
| :white_check_mark: | Comatose | The Pokémon is always drowsing and will never wake up. It can attack while in its sleeping state. |
| :white_check_mark: | Queenly Majesty | Opponents are unable to use priority moves against the Pokémon or its allies. |
| :white_check_mark: | Innards Out | When the Pokémon takes damage from a move that knocks it out, it deals the same amount of damage to the attacker. |
| :white_check_mark: | Dancer | Whenever a dance move is used in battle, the Pokémon will copy the user to immediately perform that dance move itself. |
| :white_check_mark: | Battery | Powers up ally Pokémon's special moves. |
| :x: | Fluffy | Halves the damage taken from moves that make direct contact, but doubles that of Fire-type moves. |
| :white_check_mark: | Dazzling | The Pokémon dazzles its opponents, making them unable to use priority moves against the Pokémon or its allies. |
| :white_check_mark: | Soul-Heart | Boosts the Pokémon's Sp. Atk stat every time another Pokémon faints. |
| :white_check_mark: | Tangling Hair | Contact with the Pokémon lowers the attacker's Speed stat. |
| :x: | Receiver | The Pokémon changes its Ability to match that of a defeated ally. |
| :x: | Power of Alchemy | The Pokémon copies the Ability of a defeated ally. |
| :white_check_mark: | Beast Boost | Boosts the Pokémon's most proficient stat every time it knocks out a target. |
| :x: | RKS System | Changes the Pokémon's type to match the memory disc it holds. |
| :x: | Electric Surge | Turns the ground into Electric Terrain when the Pokémon enters a battle. |
| :x: | Psychic Surge | Turns the ground into Psychic Terrain when the Pokémon enters a battle. |
| :x: | Misty Surge | Turns the ground into Misty Terrain when the Pokémon enters a battle. |
| :x: | Grassy Surge | Turns the ground into Grassy Terrain when the Pokémon enters a battle. |
| :white_check_mark: | Full Metal Body | Prevents other Pokémon's moves or Abilities from lowering the Pokémon's stats. |
| :white_check_mark: | Shadow Shield | Reduces the amount of damage the Pokémon takes while its HP is full. |
| :white_check_mark: | Prism Armor | Reduces the power of supereffective attacks that hit the Pokémon. |
| :white_check_mark: | Neuroforce | Powers up the Pokémon's supereffective attacks even further. |
| :white_check_mark: | Intrepid Sword | Boosts the Pokémon's Attack stat the first time the Pokémon enters a battle. |
| :white_check_mark: | Dauntless Shield | Boosts the Pokémon's Defense stat the first time the Pokémon enters a battle. |
| :x: | Libero | Changes the Pokémon's type to the type of the move it's about to use. This works only once each time the Pokémon enters battle. |
| :x: | Ball Fetch | If the Pokémon is not holding an item, it will fetch the Poké Ball from the first failed throw of the battle. |
| :x: | Cotton Down | When the Pokémon is hit by an attack, it scatters cotton fluff around and lowers the Speed stats of all Pokémon except itself. |
| :white_check_mark: | Propeller Tail | Ignores the effects of opposing Pokémon's Abilities and moves that draw in moves. |
| :white_check_mark: | Mirror Armor | Instead of being affected by stat-lowering effects, the Pokémon bounces them back at whichever Pokémon caused them. |
| :x: | Gulp Missile | When the Pokémon uses Surf or Dive, it will come back with prey. When it takes damage, it will spit out the prey to attack. |
| :white_check_mark: | Stalwart | The Pokémon ignores the effects of Abilities and moves that draw in moves. |
| :white_check_mark: | Steam Engine | Boosts the Speed stat drastically when the Pokémon is hit by a Fire- or Water-type move. |
| :white_check_mark: | Punk Rock | Boosts the power of sound-based moves. The Pokémon also takes half the damage from these kinds of moves. |
| :x: | Sand Spit | Summons a sandstorm for 5 turns when the Pokémon takes damage from moves. |
| :white_check_mark: | Ice Scales | The Pokémon is protected by ice scales, which halve the damage taken from special moves. |
| :white_check_mark: | Ripen | Doubles the effects of Berries eaten by the Pokémon. |
| :x: | Ice Face | The Pokémon's ice head can take a physical attack as a substitute, but the attack also changes the Pokémon's appearance. The ice will be restored when it snows. |
| :white_check_mark: | Power Spot | Just being next to the Pokémon powers up moves. |
| :x: | Mimicry | The Pokémon's type changes depending on the terrain. |
| :x: | Screen Cleaner | When the Pokémon enters a battle, it removes the Light Screen, Reflect, and Aurora Veil statuses. |
| :white_check_mark: | Steely Spirit | Powers up the Steel-type moves of the Pokémon and its allies. |
| :x: | Perish Body | When hit by a move that makes direct contact, the Pokémon and the attacker will faint after three turns unless they switch out of battle. |
| :x: | Wandering Spirit | When the Pokémon is hit by a contact move, it swaps Abilities with the attacker. |
| :x: | Gorilla Tactics | Boosts the Pokémon's Attack stat, but only allows the use of the first selected move. |
| :x: | Neutralizing Gas | While the Pokémon is in the battle, the effects of all other Pokémon's Abilities will be nullified or will not be triggered. |
| :white_check_mark: | Pastel Veil | Prevents the Pokémon and its allies from being poisoned. |
| :x: | Hunger Switch | The Pokémon changes its form, alternating between its Full Belly Mode and Hangry Mode at the end of every turn. |
| :white_check_mark: | Quick Draw | The Pokémon's moves have a 30% chance of going first among moves of the same priority. |
| :white_check_mark: | Unseen Fist | When the Pokémon uses contact moves, it can hit even targets that are protecting themselves, dealing 1/4 of the damage that the move would otherwise deal. Everything aside from the target's protective effects is still triggered. |
| :x: | Curious Medicine | When the Pokémon enters a battle, it removes all stat changes from its allies. |
| :white_check_mark: | Transistor | Powers up Electric-type moves. |
| :white_check_mark: | Dragon's Maw | Powers up Dragon-type moves. |
| :white_check_mark: | Chilling Neigh | When the Pokémon knocks out a target, it utters a chilling neigh, which boosts its Attack stat. |
| :white_check_mark: | Grim Neigh | When the Pokémon knocks out a target, it utters a terrifying neigh, which boosts its Sp. Atk stat. |
| :x: | As One | This Ability combines the effects of both Calyrex's Unnerve Ability and Glastrier's Chilling Neigh Ability. |
| :x: | As One | This Ability combines the effects of both Calyrex's Unnerve Ability and Spectrier's Grim Neigh Ability. |
| :x: | Lingering Aroma | Contact with the Pokémon changes the attacker's Ability to Lingering Aroma. |
| :x: | Seed Sower | Turns the ground into Grassy Terrain when the Pokémon is hit by an attack. |
| :white_check_mark: | Thermal Exchange | Boosts the Attack stat when the Pokémon is hit by a Fire-type move. The Pokémon also cannot be burned. |
| :x: | Anger Shell | When an attack causes its HP to drop to half or less, the Pokémon gets angry. This lowers its Defense and Sp. Def stats but boosts its Attack, Sp. Atk, and Speed stats. |
| :white_check_mark: | Purifying Salt | Halves the damage the Pokémon takes from Ghost-type moves. The Pokémon is immune to status conditions. |
| :x: | Well-Baked Body | The Pokémon takes no damage when hit by Fire-type moves. Instead, its Defense stat is sharply boosted. |
| :x: | Wind Rider | Boosts the Pokémon's Attack stat if Tailwind takes effect or if the Pokémon is hit by a wind move. The Pokémon also takes no damage from wind moves. |
| :white_check_mark: | Guard Dog | Boosts the Pokémon's Attack stat if intimidated. Moves and items that would force the Pokémon to switch out also fail to work. |
| :white_check_mark: | Rocky Payload | Powers up Rock-type moves. |
| :white_check_mark: | Wind Power | The Pokémon becomes charged when it is hit by a wind move, boosting the power of the next Electric-type move the Pokémon uses. |
| :x: | Zero to Hero | The Pokémon changes into its Hero Form when it switches out of battle. |
| :x: | Commander | When the Pokémon enters a battle, it goes inside the mouth of an ally Dondozo if one is on the field. The Pokémon then issues commands from there. |
| :white_check_mark: | Electromorphosis | When the Pokémon takes damage from a move, it gains the Electric Boost status. |
| :x: | Protosynthesis | Boosts the Pokémon's most proficient stat in harsh sunlight or if the Pokémon is holding Booster Energy. |
| :x: | Quark Drive | Boosts the Pokémon's most proficient stat on Electric Terrain or if the Pokémon is holding Booster Energy. |
| :white_check_mark: | Good as Gold | A body of pure, solid gold gives the Pokémon full immunity to other Pokémon's status moves. |
| :x: | Vessel of Ruin | The power of the Pokémon's ruinous vessel lowers the Sp. Atk stats of all Pokémon except itself. |
| :x: | Sword of Ruin | The power of the Pokémon's ruinous sword lowers the Defense stats of all Pokémon except itself. |
| :x: | Tablets of Ruin | The power of the Pokémon's ruinous wooden tablets lowers the Attack stats of all Pokémon except itself. |
| :x: | Beads of Ruin | The power of the Pokémon's ruinous beads lowers the Sp. Def stats of all Pokémon except itself. |
| :x: | Orichalcum Pulse | Turns the sunlight harsh when the Pokémon enters a battle. The ancient pulse thrumming through the Pokémon also boosts its Attack stat in harsh sunlight. |
| :x: | Hadron Engine | Turns the ground into Electric Terrain when the Pokémon enters a battle. The futuristic engine within the Pokémon also boosts its Sp. Atk stat on Electric Terrain. |
| :white_check_mark: | Opportunist | When an opponent's stats are boosted, the Pokémon boosts its own stats in the exact same way. |
| :white_check_mark: | Cud Chew | If the Pokémon eats a Berry, it will eat that same Berry once more at the end of the next turn. |
| :white_check_mark: | Sharpness | Boosts the power of the Pokémon's slicing moves by 50%. |
| :white_check_mark: | Supreme Overlord | When the Pokémon enters a battle, the power of its moves is boosted by 10% for each Pokémon in its party that has been defeated in the battle already. The maximum boost is 50%. |
| :x: | Costar | When the Pokémon enters a battle, it copies an ally's stat changes. |
| :x: | Toxic Debris | When the Pokémon takes damage from a physical move, it gives the opponent's side the Toxic Spikes status. |
| :white_check_mark: | Armor Tail | Opponents are unable to use priority moves against the Pokémon or its allies. |
| :x: | Earth Eater | Ground-type moves do not work on the Pokémon. Instead, they restore 1/4 of its max HP. |
| :x: | Mycelium Might | The Pokémon will always act more slowly when using status moves, but these moves will be unimpeded by the Ability of the target. |
| :white_check_mark: | Hospitality | When the Pokémon enters a battle, it restores 1/4 of its ally's max HP. |
| :white_check_mark: | Mind's Eye | The Pokémon ignores changes to opponents' evasiveness, its accuracy can't be lowered, and it can hit Ghost types with Normal- and Fighting-type moves. |
| :x: | Embody Aspect | The Pokémon's heart fills with memories, causing the Teal Mask to shine and the Pokémon's Speed stat to be boosted. |
| :x: | Embody Aspect | The Pokémon's heart fills with memories, causing the Hearthflame Mask to shine and the Pokémon's Attack stat to be boosted. |
| :x: | Embody Aspect | The Pokémon's heart fills with memories, causing the Wellspring Mask to shine and the Pokémon's Sp. Def stat to be boosted. |
| :x: | Embody Aspect | The Pokémon's heart fills with memories, causing the Cornerstone Mask to shine and the Pokémon's Defense stat to be boosted. |
| :x: | Toxic Chain | The power of the Pokémon's toxic chain may badly poison any target the Pokémon hits with a move. |
| :white_check_mark: | Supersweet Syrup | When the Pokémon enters a battle, opponents' evasiveness is lowered by 1 stage. This Ability is triggered only once per battle. |
| :x: | Tera Shift | When the Pokémon enters a battle, it absorbs the energy around itself and transforms into its Terastal Form. |
| :x: | Tera Shell | The Pokémon's shell contains the powers of each type. All damage-dealing moves that hit the Pokémon when its HP is full will not be very effective. |
| :x: | Teraform Zero | When Terapagos changes into its Stellar Form, it uses its hidden powers to eliminate all effects of weather and terrain, reducing them to zero. |
| :x: | Poison Puppeteer | Pokémon poisoned by Pecharunt's moves will also become confused. |
| :white_check_mark: | Piercing Drill | When the Pokémon uses contact moves, it can hit even targets that are protecting themselves, dealing 1/4 of the damage that the move would otherwise deal. Everything aside from the target's protective effects is still triggered. |
| :x: | Dragonize | The Pokémon's Normal-type moves become Dragon-type moves and their power is boosted by 20%. |
| :white_check_mark: | Mega Sol | Even when the sunlight has not turned harsh, the Pokémon can use its moves as if the weather were harsh sunlight. |
| :x: | Spicy Spray | When the Pokémon takes damage from a move, it burns the attacker. |
