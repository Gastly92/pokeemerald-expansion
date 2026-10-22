# Innate abilities — quick reference

Every ability a species may carry as an innate (`FEATURE_INNATE_ABILITIES`), with a
one-line summary of what it does in that role. This is a lookup table, not the spec:
follow the **→** link for the exact semantics, wiring and species picks in
[`INNATE_ABILITIES.md`](INNATE_ABILITIES.md#per-ability-wiring-reference).

- **Source of truth:** `sImplementedInnates[]` in `test/fork/innate_abilities.c`. CI
  fails any species row that names an ability not on it. This page lists 193.
- **Most innates are a 1:1 copy** of the real ability. **◆** marks a *pure-boon
  divergence*: the innate keeps the upside and drops a cost the real ability has
  (see [the design principle](INNATE_ABILITIES.md#the-design-principle-an-innate-is-a-pure-boon)).
- **Reserved** abilities may only be declared by their canon carriers
  ([details](INNATE_ABILITIES.md#reserved-abilities-welded-to-one-line-banned-everywhere-else)).
- **Fork abilities** (Rekindle, Willpower, Pacify) are defined in
  [`NEW_ABILITIES.md`](NEW_ABILITIES.md).
- The fork ships with `DETERMINISTIC_ABILITIES` on, so chance-based effects below
  note their deterministic behaviour where it differs.

When you wire a new innate, add its row here in the same PR (Step 5 of
[the recipe](INNATE_ABILITIES.md#step-5--update-the-indexes)).

## Immunities and passive defence

| Ability | As an innate | |
| --- | --- | --- |
| Armor Tail | Foes can't use priority moves against its side. | [→](INNATE_ABILITIES.md#ability_dazzling--ability_queenly_majesty--ability_armor_tail) |
| Battle Armor | Can't be hit by a critical hit. | [→](INNATE_ABILITIES.md#ability_battle_armor--ability_shell_armor) |
| Big Pecks | Its Defense can't be lowered. | [→](INNATE_ABILITIES.md#ability_clear_body--ability_white_smoke--ability_hyper_cutter--ability_big_pecks) |
| Clear Body | Its stats can't be lowered by other Pokémon. | [→](INNATE_ABILITIES.md#ability_clear_body--ability_white_smoke--ability_hyper_cutter--ability_big_pecks) |
| Comatose ◆ | Immune to every non-volatile status and counts as asleep for its own Snore/Sleep Talk, without the costs (Hex, Nightmare, Bad Dreams, Rest block). | [→](INNATE_ABILITIES.md#ability_comatose) |
| Dazzling | Foes can't use priority moves against its side. | [→](INNATE_ABILITIES.md#ability_dazzling--ability_queenly_majesty--ability_armor_tail) |
| Full Metal Body | Its stats can't be lowered by other Pokémon (unbreakable). | [→](INNATE_ABILITIES.md#ability_full_metal_body--ability_minds_eye) |
| Good As Gold | Immune to opponents' status moves. | [→](INNATE_ABILITIES.md#ability_purifying_salt--ability_good_as_gold) |
| Guard Dog | Can't be forced out; Intimidate raises its Attack instead. | [→](INNATE_ABILITIES.md#ability_suction_cups--ability_guard_dog) |
| Hyper Cutter | Its Attack can't be lowered. | [→](INNATE_ABILITIES.md#ability_clear_body--ability_white_smoke--ability_hyper_cutter--ability_big_pecks) |
| Immunity | Can't be poisoned. | [→](INNATE_ABILITIES.md#ability_immunity--ability_pastel_veil) |
| Inner Focus | Can't flinch; ignores Intimidate. | [→](INNATE_ABILITIES.md#ability_magma_armor--ability_water_veil--ability_own_tempo--ability_inner_focus--ability_leaf_guard--ability_overcoat) |
| Insomnia ◆ | Can't be put to sleep or made drowsy, but can still use Rest. | [→](INNATE_ABILITIES.md#ability_insomnia--ability_vital_spirit--ability_sweet_veil) |
| Leaf Guard | Can't be given a non-volatile status in harsh sunlight. | [→](INNATE_ABILITIES.md#ability_magma_armor--ability_water_veil--ability_own_tempo--ability_inner_focus--ability_leaf_guard--ability_overcoat) |
| Levitate ◆ | Immune to Ground moves and hazards, but still counts as grounded for terrain and for absorbing Toxic Spikes. | [→](INNATE_ABILITIES.md#ability_levitate) |
| Limber | Can't be paralyzed. | [→](INNATE_ABILITIES.md#ability_limber) |
| Magic Bounce | Reflects bounceable status moves back at the user. | [→](INNATE_ABILITIES.md#ability_magic_bounce) |
| Magic Guard | Only takes damage from direct attacks. | [→](INNATE_ABILITIES.md#ability_magic_guard) |
| Magma Armor | Can't be frozen or frostbitten. | [→](INNATE_ABILITIES.md#ability_magma_armor--ability_water_veil--ability_own_tempo--ability_inner_focus--ability_leaf_guard--ability_overcoat) |
| Mirror Armor | Bounces stat drops back at whoever caused them. | [→](INNATE_ABILITIES.md#ability_mirror_armor) |
| Oblivious | Can't be infatuated or Taunted; ignores Intimidate. | [→](INNATE_ABILITIES.md#ability_oblivious) |
| Overcoat | Immune to powder moves and sandstorm/hail damage. | [→](INNATE_ABILITIES.md#ability_magma_armor--ability_water_veil--ability_own_tempo--ability_inner_focus--ability_leaf_guard--ability_overcoat) |
| Own Tempo | Can't be confused; ignores Intimidate. | [→](INNATE_ABILITIES.md#ability_magma_armor--ability_water_veil--ability_own_tempo--ability_inner_focus--ability_leaf_guard--ability_overcoat) |
| Pastel Veil | Its whole side can't be poisoned. | [→](INNATE_ABILITIES.md#ability_immunity--ability_pastel_veil) |
| Purifying Salt ◆ | Immune to every non-volatile status and takes half damage from Ghost moves; can still use Rest. | [→](INNATE_ABILITIES.md#ability_purifying_salt--ability_good_as_gold) |
| Queenly Majesty | Foes can't use priority moves against its side. | [→](INNATE_ABILITIES.md#ability_dazzling--ability_queenly_majesty--ability_armor_tail) |
| Shell Armor | Can't be hit by a critical hit. | [→](INNATE_ABILITIES.md#ability_battle_armor--ability_shell_armor) |
| Shield Dust | Blocks the additional effects of moves used against it. | [→](INNATE_ABILITIES.md#ability_shield_dust--ability_tinted_lens--ability_scrappy--ability_wonder_skin--ability_tangled_feet) |
| Sticky Hold | Its held item can't be stolen or removed. | [→](INNATE_ABILITIES.md#ability_sticky_hold) |
| Sturdy | Survives a lethal hit from full HP; immune to OHKO moves. | [→](INNATE_ABILITIES.md#ability_sturdy) |
| Suction Cups | Can't be forced out (Roar, Whirlwind, Dragon Tail, Red Card). | [→](INNATE_ABILITIES.md#ability_suction_cups--ability_guard_dog) |
| Sweet Veil ◆ | Its whole side can't be put to sleep or made drowsy; the holder can still use Rest. | [→](INNATE_ABILITIES.md#ability_insomnia--ability_vital_spirit--ability_sweet_veil) |
| Vital Spirit ◆ | Can't be put to sleep or made drowsy, but can still use Rest. | [→](INNATE_ABILITIES.md#ability_insomnia--ability_vital_spirit--ability_sweet_veil) |
| Water Veil | Can't be burned. | [→](INNATE_ABILITIES.md#ability_magma_armor--ability_water_veil--ability_own_tempo--ability_inner_focus--ability_leaf_guard--ability_overcoat) |
| White Smoke | Its stats can't be lowered by other Pokémon. | [→](INNATE_ABILITIES.md#ability_clear_body--ability_white_smoke--ability_hyper_cutter--ability_big_pecks) |
| Wonder Skin | Incoming status moves are capped at 50% accuracy. | [→](INNATE_ABILITIES.md#ability_shield_dust--ability_tinted_lens--ability_scrappy--ability_wonder_skin--ability_tangled_feet) |

## Damage reduction

| Ability | As an innate | |
| --- | --- | --- |
| Filter | Takes 25% less damage from super-effective moves; stacks with Solid Rock / Prism Armor. | [→](INNATE_ABILITIES.md#ability_filter) |
| Friend Guard | Its allies take 25% less damage. | [→](INNATE_ABILITIES.md#ability_multiscale--ability_solid_rock--ability_fur_coat--ability_ice_scales--ability_heatproof--ability_friend_guard--ability_water_bubble) |
| Fur Coat | Doubled Defense against physical moves. | [→](INNATE_ABILITIES.md#ability_multiscale--ability_solid_rock--ability_fur_coat--ability_ice_scales--ability_heatproof--ability_friend_guard--ability_water_bubble) |
| Grass Pelt | Defense x1.5 on Grassy Terrain. | [→](INNATE_ABILITIES.md#ability_surge_surfer--ability_grass_pelt) |
| Heatproof | Takes half damage from Fire moves and burn. | [→](INNATE_ABILITIES.md#ability_multiscale--ability_solid_rock--ability_fur_coat--ability_ice_scales--ability_heatproof--ability_friend_guard--ability_water_bubble) |
| Heavy Metal | Doubles its weight. | [→](INNATE_ABILITIES.md#ability_heavy_metal--ability_light_metal) |
| Ice Scales | Takes half damage from special moves. | [→](INNATE_ABILITIES.md#ability_multiscale--ability_solid_rock--ability_fur_coat--ability_ice_scales--ability_heatproof--ability_friend_guard--ability_water_bubble) |
| Light Metal | Halves its weight. | [→](INNATE_ABILITIES.md#ability_heavy_metal--ability_light_metal) |
| Marvel Scale | Defense x1.5 while statused. | [→](INNATE_ABILITIES.md#ability_guts--ability_marvel_scale--ability_quick_feet--ability_toxic_boost--ability_flare_boost) |
| Multiscale | Takes half damage at full HP. | [→](INNATE_ABILITIES.md#ability_multiscale--ability_solid_rock--ability_fur_coat--ability_ice_scales--ability_heatproof--ability_friend_guard--ability_water_bubble) |
| Prism Armor | Takes 25% less damage from super-effective moves (unbreakable); stacks with Filter / Solid Rock. | [→](INNATE_ABILITIES.md#ability_prism_armor--ability_shadow_shield--ability_neuroforce--ability_supreme_overlord) |
| Sand Veil | +25% evasion in sandstorm; immune to sandstorm damage. | [→](INNATE_ABILITIES.md#ability_sand_veil--ability_snow_cloak) |
| Shadow Shield | Takes half damage at full HP (unbreakable). | [→](INNATE_ABILITIES.md#ability_prism_armor--ability_shadow_shield--ability_neuroforce--ability_supreme_overlord) |
| Snow Cloak | +25% evasion in hail/snow; immune to hail damage. | [→](INNATE_ABILITIES.md#ability_sand_veil--ability_snow_cloak) |
| Solid Rock | Takes 25% less damage from super-effective moves; stacks with Filter / Prism Armor. | [→](INNATE_ABILITIES.md#ability_multiscale--ability_solid_rock--ability_fur_coat--ability_ice_scales--ability_heatproof--ability_friend_guard--ability_water_bubble) |
| Tangled Feet | Doubled evasion while confused. | [→](INNATE_ABILITIES.md#ability_shield_dust--ability_tinted_lens--ability_scrappy--ability_wonder_skin--ability_tangled_feet) |
| Thick Fat | Takes half damage from Fire and Ice moves. | [→](INNATE_ABILITIES.md#ability_thick_fat) |
| Water Bubble | Takes half damage from Fire moves, can't be burned, and its Water moves are doubled. | [→](INNATE_ABILITIES.md#ability_multiscale--ability_solid_rock--ability_fur_coat--ability_ice_scales--ability_heatproof--ability_friend_guard--ability_water_bubble) |

## Offence: power and damage boosts

| Ability | As an innate | |
| --- | --- | --- |
| Adaptability | STAB becomes x2 instead of x1.5. | [→](INNATE_ABILITIES.md#ability_iron_fist--ability_reckless--ability_strong_jaw--ability_tough_claws--ability_sharpness--ability_mega_launcher--ability_steelworker--ability_steely_spirit--ability_rocky_payload--ability_sand_force--ability_analytic--ability_adaptability--ability_punk_rock--ability_stakeout) |
| Analytic | +30% power when moving last. | [→](INNATE_ABILITIES.md#ability_iron_fist--ability_reckless--ability_strong_jaw--ability_tough_claws--ability_sharpness--ability_mega_launcher--ability_steelworker--ability_steely_spirit--ability_rocky_payload--ability_sand_force--ability_analytic--ability_adaptability--ability_punk_rock--ability_stakeout) |
| Battery | Boosts allies' special moves x1.3. | [→](INNATE_ABILITIES.md#ability_battery--ability_power_spot--ability_telepathy--ability_aroma_veil--ability_flower_veil) |
| Blaze ◆ | +50% Fire moves at low HP; latches on once triggered. | [→](INNATE_ABILITIES.md#ability_overgrow--ability_blaze--ability_torrent--ability_swarm) |
| Dragon's Maw | Boosts Dragon moves. | [→](INNATE_ABILITIES.md#ability_transistor--ability_dragons_maw) |
| Flare Boost | Special power x1.5 while burned. | [→](INNATE_ABILITIES.md#ability_guts--ability_marvel_scale--ability_quick_feet--ability_toxic_boost--ability_flare_boost) |
| Guts | Attack x1.5 while statused. | [→](INNATE_ABILITIES.md#ability_guts--ability_marvel_scale--ability_quick_feet--ability_toxic_boost--ability_flare_boost) |
| Huge Power | Doubles Attack. | [→](INNATE_ABILITIES.md#ability_huge_power--ability_pure_power) |
| Iron Fist | +20% punching moves. | [→](INNATE_ABILITIES.md#ability_iron_fist--ability_reckless--ability_strong_jaw--ability_tough_claws--ability_sharpness--ability_mega_launcher--ability_steelworker--ability_steely_spirit--ability_rocky_payload--ability_sand_force--ability_analytic--ability_adaptability--ability_punk_rock--ability_stakeout) |
| Mega Launcher | +50% pulse moves (and stronger Heal Pulse). | [→](INNATE_ABILITIES.md#ability_iron_fist--ability_reckless--ability_strong_jaw--ability_tough_claws--ability_sharpness--ability_mega_launcher--ability_steelworker--ability_steely_spirit--ability_rocky_payload--ability_sand_force--ability_analytic--ability_adaptability--ability_punk_rock--ability_stakeout) |
| Mega Sol | Its own moves act as if the weather were harsh sun. | [→](INNATE_ABILITIES.md#ability_mega_sol) |
| Merciless | Always crits a poisoned target. | [→](INNATE_ABILITIES.md#ability_super_luck--ability_sniper--ability_merciless) |
| Neuroforce | Boosts super-effective moves. | [→](INNATE_ABILITIES.md#ability_prism_armor--ability_shadow_shield--ability_neuroforce--ability_supreme_overlord) |
| Overgrow ◆ | +50% Grass moves at low HP; latches on once triggered, so healing doesn't remove it. | [→](INNATE_ABILITIES.md#ability_overgrow--ability_blaze--ability_torrent--ability_swarm) |
| Power Spot | Boosts allies' moves x1.3. | [→](INNATE_ABILITIES.md#ability_battery--ability_power_spot--ability_telepathy--ability_aroma_veil--ability_flower_veil) |
| Punk Rock | +30% sound moves. | [→](INNATE_ABILITIES.md#ability_iron_fist--ability_reckless--ability_strong_jaw--ability_tough_claws--ability_sharpness--ability_mega_launcher--ability_steelworker--ability_steely_spirit--ability_rocky_payload--ability_sand_force--ability_analytic--ability_adaptability--ability_punk_rock--ability_stakeout) |
| Pure Power | Doubles Attack. | [→](INNATE_ABILITIES.md#ability_huge_power--ability_pure_power) |
| Reckless | +20% recoil and crash moves. | [→](INNATE_ABILITIES.md#ability_iron_fist--ability_reckless--ability_strong_jaw--ability_tough_claws--ability_sharpness--ability_mega_launcher--ability_steelworker--ability_steely_spirit--ability_rocky_payload--ability_sand_force--ability_analytic--ability_adaptability--ability_punk_rock--ability_stakeout) |
| Rocky Payload | +50% Rock moves. | [→](INNATE_ABILITIES.md#ability_iron_fist--ability_reckless--ability_strong_jaw--ability_tough_claws--ability_sharpness--ability_mega_launcher--ability_steelworker--ability_steely_spirit--ability_rocky_payload--ability_sand_force--ability_analytic--ability_adaptability--ability_punk_rock--ability_stakeout) |
| Sand Force | +30% Ground/Rock/Steel moves in sandstorm; immune to sandstorm damage. | [→](INNATE_ABILITIES.md#ability_iron_fist--ability_reckless--ability_strong_jaw--ability_tough_claws--ability_sharpness--ability_mega_launcher--ability_steelworker--ability_steely_spirit--ability_rocky_payload--ability_sand_force--ability_analytic--ability_adaptability--ability_punk_rock--ability_stakeout) |
| Scrappy | Normal and Fighting moves hit Ghosts; ignores Intimidate. | [→](INNATE_ABILITIES.md#ability_shield_dust--ability_tinted_lens--ability_scrappy--ability_wonder_skin--ability_tangled_feet) |
| Serene Grace | Doubles its moves' additional-effect chances. | [→](INNATE_ABILITIES.md#ability_serene_grace) |
| Sharpness | +50% slicing moves. | [→](INNATE_ABILITIES.md#ability_iron_fist--ability_reckless--ability_strong_jaw--ability_tough_claws--ability_sharpness--ability_mega_launcher--ability_steelworker--ability_steely_spirit--ability_rocky_payload--ability_sand_force--ability_analytic--ability_adaptability--ability_punk_rock--ability_stakeout) |
| Sniper | Critical hits deal x2.25 instead of x1.5. | [→](INNATE_ABILITIES.md#ability_super_luck--ability_sniper--ability_merciless) |
| Stakeout | Double damage against a target that just switched in. | [→](INNATE_ABILITIES.md#ability_iron_fist--ability_reckless--ability_strong_jaw--ability_tough_claws--ability_sharpness--ability_mega_launcher--ability_steelworker--ability_steely_spirit--ability_rocky_payload--ability_sand_force--ability_analytic--ability_adaptability--ability_punk_rock--ability_stakeout) |
| Steelworker | +50% Steel moves. | [→](INNATE_ABILITIES.md#ability_iron_fist--ability_reckless--ability_strong_jaw--ability_tough_claws--ability_sharpness--ability_mega_launcher--ability_steelworker--ability_steely_spirit--ability_rocky_payload--ability_sand_force--ability_analytic--ability_adaptability--ability_punk_rock--ability_stakeout) |
| Steely Spirit | +50% Steel moves for itself and its allies. | [→](INNATE_ABILITIES.md#ability_iron_fist--ability_reckless--ability_strong_jaw--ability_tough_claws--ability_sharpness--ability_mega_launcher--ability_steelworker--ability_steely_spirit--ability_rocky_payload--ability_sand_force--ability_analytic--ability_adaptability--ability_punk_rock--ability_stakeout) |
| Strong Jaw | +50% biting moves. | [→](INNATE_ABILITIES.md#ability_iron_fist--ability_reckless--ability_strong_jaw--ability_tough_claws--ability_sharpness--ability_mega_launcher--ability_steelworker--ability_steely_spirit--ability_rocky_payload--ability_sand_force--ability_analytic--ability_adaptability--ability_punk_rock--ability_stakeout) |
| Super Luck | +1 critical-hit stage. | [→](INNATE_ABILITIES.md#ability_super_luck--ability_sniper--ability_merciless) |
| Supreme Overlord | Power rises for each fainted party member. | [→](INNATE_ABILITIES.md#ability_prism_armor--ability_shadow_shield--ability_neuroforce--ability_supreme_overlord) |
| Swarm ◆ | +50% Bug moves at low HP; latches on once triggered. | [→](INNATE_ABILITIES.md#ability_overgrow--ability_blaze--ability_torrent--ability_swarm) |
| Technician | +50% power on moves of 60 BP or less. | [→](INNATE_ABILITIES.md#ability_technician) |
| Tinted Lens | Doubles damage of not-very-effective moves. | [→](INNATE_ABILITIES.md#ability_shield_dust--ability_tinted_lens--ability_scrappy--ability_wonder_skin--ability_tangled_feet) |
| Torrent ◆ | +50% Water moves at low HP; latches on once triggered. | [→](INNATE_ABILITIES.md#ability_overgrow--ability_blaze--ability_torrent--ability_swarm) |
| Tough Claws | +30% contact moves. | [→](INNATE_ABILITIES.md#ability_iron_fist--ability_reckless--ability_strong_jaw--ability_tough_claws--ability_sharpness--ability_mega_launcher--ability_steelworker--ability_steely_spirit--ability_rocky_payload--ability_sand_force--ability_analytic--ability_adaptability--ability_punk_rock--ability_stakeout) |
| Toxic Boost | Physical power x1.5 while poisoned. | [→](INNATE_ABILITIES.md#ability_guts--ability_marvel_scale--ability_quick_feet--ability_toxic_boost--ability_flare_boost) |
| Transistor | Boosts Electric moves. | [→](INNATE_ABILITIES.md#ability_transistor--ability_dragons_maw) |

## Accuracy, contact and move traits

| Ability | As an innate | |
| --- | --- | --- |
| Compound Eyes | Ignores the target's evasion boosts (the fork's deterministic-accuracy version). | [→](INNATE_ABILITIES.md#ability_compound_eyes--ability_keen_eye--ability_illuminate) |
| Corrosion | Can poison Poison- and Steel-types. | [→](INNATE_ABILITIES.md#ability_corrosion) |
| Illuminate | Ignores the target's evasion boosts; its accuracy can't be lowered. | [→](INNATE_ABILITIES.md#ability_compound_eyes--ability_keen_eye--ability_illuminate) |
| Infiltrator | Ignores screens, Safeguard, Mist and Substitute. | [→](INNATE_ABILITIES.md#ability_infiltrator) |
| Keen Eye | Ignores the target's evasion boosts; its accuracy can't be lowered. | [→](INNATE_ABILITIES.md#ability_compound_eyes--ability_keen_eye--ability_illuminate) |
| Liquid Ooze | HP-draining moves used on it damage the attacker instead. | [→](INNATE_ABILITIES.md#ability_pickpocket--ability_magician--ability_liquid_ooze) |
| Long Reach | Its moves don't make contact. | [→](INNATE_ABILITIES.md#ability_long_reach) |
| Mind's Eye | Keen Eye + Scrappy: ignores evasion, accuracy can't drop, Normal/Fighting hit Ghosts. | [→](INNATE_ABILITIES.md#ability_full_metal_body--ability_minds_eye) |
| Mold Breaker | Its moves ignore the target's breakable ability. | [→](INNATE_ABILITIES.md#ability_mold_breaker) |
| Piercing Drill | Contact moves hit through Protect. | [→](INNATE_ABILITIES.md#ability_unseen_fist--ability_piercing_drill) |
| Propeller Tail | Its moves ignore redirection. | [→](INNATE_ABILITIES.md#ability_propeller_tail--ability_stalwart) |
| Rock Head | No recoil from its own moves. | [→](INNATE_ABILITIES.md#ability_rock_head) |
| Skill Link | Multi-hit moves always hit the maximum number of times. | [→](INNATE_ABILITIES.md#ability_skill_link) |
| Stalwart | Its moves ignore redirection. | [→](INNATE_ABILITIES.md#ability_propeller_tail--ability_stalwart) |
| Teravolt | Its moves ignore the target's breakable ability. | [→](INNATE_ABILITIES.md#ability_teravolt--ability_turboblaze) |
| Turboblaze | Its moves ignore the target's breakable ability. | [→](INNATE_ABILITIES.md#ability_teravolt--ability_turboblaze) |
| Unaware ◆ | Ignores the foe's stat boosts, but still benefits from the foe's stat drops. | [→](INNATE_ABILITIES.md#ability_unaware) |
| Unseen Fist | Contact moves hit through Protect. | [→](INNATE_ABILITIES.md#ability_unseen_fist--ability_piercing_drill) |
| Victory Star | Boosts its own and its partner's accuracy x1.1; both ignore evasion boosts in the PP economy. | [→](INNATE_ABILITIES.md#ability_victory_star) |

## Speed and priority

| Ability | As an innate | |
| --- | --- | --- |
| Chlorophyll | Speed x2 in harsh sun. | [→](INNATE_ABILITIES.md#ability_swift_swim--ability_chlorophyll--ability_sand_rush--ability_slush_rush) |
| Gale Wings | +1 priority on Flying moves at full HP. | [→](INNATE_ABILITIES.md#ability_gale_wings--ability_triage) |
| Prankster ◆ | +1 priority on status moves, and they still work on Dark-types. | [→](INNATE_ABILITIES.md#ability_prankster) |
| Quick Draw | Always moves first in its priority bracket on its entry turn (30% chance without deterministic abilities). | [→](INNATE_ABILITIES.md#ability_quick_draw) |
| Quick Feet | Speed x1.5 while statused. | [→](INNATE_ABILITIES.md#ability_guts--ability_marvel_scale--ability_quick_feet--ability_toxic_boost--ability_flare_boost) |
| Sand Rush | Speed x2 in sandstorm; immune to sandstorm damage. | [→](INNATE_ABILITIES.md#ability_swift_swim--ability_chlorophyll--ability_sand_rush--ability_slush_rush) |
| Slush Rush | Speed x2 in snow. | [→](INNATE_ABILITIES.md#ability_swift_swim--ability_chlorophyll--ability_sand_rush--ability_slush_rush) |
| Speed Boost | +1 Speed at the end of every turn. | [→](INNATE_ABILITIES.md#ability_speed_boost) |
| Surge Surfer | Speed x2 on Electric Terrain. | [→](INNATE_ABILITIES.md#ability_surge_surfer--ability_grass_pelt) |
| Swift Swim | Speed x2 in rain. | [→](INNATE_ABILITIES.md#ability_swift_swim--ability_chlorophyll--ability_sand_rush--ability_slush_rush) |
| Triage | +3 priority on healing moves. | [→](INNATE_ABILITIES.md#ability_gale_wings--ability_triage) |
| Unburden | Speed x2 after its held item is used or lost. | [→](INNATE_ABILITIES.md#ability_gluttony--ability_ripen--ability_cheek_pouch--ability_unburden) |

## Switch-in effects

| Ability | As an innate | |
| --- | --- | --- |
| Anticipation | Warns on entry if a foe has a super-effective or OHKO move. | [→](INNATE_ABILITIES.md#ability_anticipation--ability_forewarn--ability_frisk) |
| Dauntless Shield | +1 Defense the first time it enters battle. | [→](INNATE_ABILITIES.md#ability_intrepid_sword--ability_dauntless_shield) |
| Download | Raises Attack or Sp. Atk on entry, whichever hits the foe's weaker defence. | [→](INNATE_ABILITIES.md#ability_download--ability_supersweet_syrup) |
| Forewarn | Reveals one of a foe's strongest moves on entry. | [→](INNATE_ABILITIES.md#ability_anticipation--ability_forewarn--ability_frisk) |
| Frisk | Reveals the foes' held items on entry. | [→](INNATE_ABILITIES.md#ability_anticipation--ability_forewarn--ability_frisk) |
| Hospitality | Heals an ally 1/4 max HP on entry. | [→](INNATE_ABILITIES.md#ability_unnerve--ability_hospitality) |
| Intimidate | Lowers every foe's Attack by 1 on entry. | [→](INNATE_ABILITIES.md#ability_intimidate) |
| Intrepid Sword | +1 Attack the first time it enters battle. | [→](INNATE_ABILITIES.md#ability_intrepid_sword--ability_dauntless_shield) |
| Pacify | Fork ability: lowers every foe's Sp. Atk by 1 on entry. | [→](INNATE_ABILITIES.md#ability_pacify) |
| Pressure | Foes use 1 extra PP on moves aimed at it. | [→](INNATE_ABILITIES.md#ability_pressure) |
| Protosynthesis | Reserved. Boosts its highest stat in sun or with Booster Energy. | [→](INNATE_ABILITIES.md#ability_protosynthesis--ability_quark_drive) |
| Quark Drive | Reserved. Boosts its highest stat on Electric Terrain or with Booster Energy. | [→](INNATE_ABILITIES.md#ability_protosynthesis--ability_quark_drive) |
| Supersweet Syrup | Lowers every foe's evasion by 1 on entry (once per battle). | [→](INNATE_ABILITIES.md#ability_download--ability_supersweet_syrup) |
| Unnerve | Foes can't eat Berries. | [→](INNATE_ABILITIES.md#ability_unnerve--ability_hospitality) |

## Switch-out and end-of-turn effects

| Ability | As an innate | |
| --- | --- | --- |
| Bad Dreams | Sleeping foes lose 1/8 max HP each turn. | [→](INNATE_ABILITIES.md#ability_rain_dish--ability_ice_body--ability_shed_skin--ability_hydration--ability_healer--ability_harvest--ability_cud_chew--ability_pickup--ability_bad_dreams) |
| Cheek Pouch | Heals 1/3 max HP whenever it eats a Berry. | [→](INNATE_ABILITIES.md#ability_gluttony--ability_ripen--ability_cheek_pouch--ability_unburden) |
| Cud Chew | Eats a used Berry again the next turn. | [→](INNATE_ABILITIES.md#ability_rain_dish--ability_ice_body--ability_shed_skin--ability_hydration--ability_healer--ability_harvest--ability_cud_chew--ability_pickup--ability_bad_dreams) |
| Early Bird | Wakes from sleep twice as fast. | [→](INNATE_ABILITIES.md#ability_early_bird) |
| Gluttony | Eats pinch Berries at 1/2 HP instead of 1/4. | [→](INNATE_ABILITIES.md#ability_gluttony--ability_ripen--ability_cheek_pouch--ability_unburden) |
| Harvest | May restore a used Berry at end of turn. | [→](INNATE_ABILITIES.md#ability_rain_dish--ability_ice_body--ability_shed_skin--ability_hydration--ability_healer--ability_harvest--ability_cud_chew--ability_pickup--ability_bad_dreams) |
| Healer | Cures an ally's status at end of turn (30%, always with deterministic abilities). | [→](INNATE_ABILITIES.md#ability_rain_dish--ability_ice_body--ability_shed_skin--ability_hydration--ability_healer--ability_harvest--ability_cud_chew--ability_pickup--ability_bad_dreams) |
| Hydration | Cures its status at end of turn in rain. | [→](INNATE_ABILITIES.md#ability_rain_dish--ability_ice_body--ability_shed_skin--ability_hydration--ability_healer--ability_harvest--ability_cud_chew--ability_pickup--ability_bad_dreams) |
| Ice Body | Heals 1/16 max HP each turn in snow/hail. | [→](INNATE_ABILITIES.md#ability_rain_dish--ability_ice_body--ability_shed_skin--ability_hydration--ability_healer--ability_harvest--ability_cud_chew--ability_pickup--ability_bad_dreams) |
| Natural Cure | Cures its status on switch-out. | [→](INNATE_ABILITIES.md#ability_natural_cure) |
| Pickup | Picks up an item that was used this turn. | [→](INNATE_ABILITIES.md#ability_rain_dish--ability_ice_body--ability_shed_skin--ability_hydration--ability_healer--ability_harvest--ability_cud_chew--ability_pickup--ability_bad_dreams) |
| Poison Heal | Heals 1/8 max HP each turn while poisoned instead of taking damage. | [→](INNATE_ABILITIES.md#ability_poison_heal) |
| Rain Dish | Heals 1/16 max HP each turn in rain. | [→](INNATE_ABILITIES.md#ability_rain_dish--ability_ice_body--ability_shed_skin--ability_hydration--ability_healer--ability_harvest--ability_cud_chew--ability_pickup--ability_bad_dreams) |
| Regenerator | Heals 1/3 max HP on switch-out. | [→](INNATE_ABILITIES.md#ability_regenerator) |
| Ripen | Doubles Berry effects. | [→](INNATE_ABILITIES.md#ability_gluttony--ability_ripen--ability_cheek_pouch--ability_unburden) |
| Shed Skin | Cures its own status at end of turn (30%, always with deterministic abilities). | [→](INNATE_ABILITIES.md#ability_rain_dish--ability_ice_body--ability_shed_skin--ability_hydration--ability_healer--ability_harvest--ability_cud_chew--ability_pickup--ability_bad_dreams) |

## Reactions: when hit or when a stat drops

| Ability | As an innate | |
| --- | --- | --- |
| Aftermath | A contact move that KOs it costs the attacker 1/4 max HP. | [→](INNATE_ABILITIES.md#ability_aftermath--ability_innards_out) |
| Anger Point | Maxes Attack when it takes a critical hit. | [→](INNATE_ABILITIES.md#ability_justified--ability_stamina--ability_water_compaction--ability_anger_point) |
| Berserk | +1 Sp. Atk when a hit drops it to 1/2 HP or below. | [→](INNATE_ABILITIES.md#ability_moxie--ability_berserk--ability_soul_heart) |
| Competitive | +2 Sp. Atk when a foe lowers its stats. | [→](INNATE_ABILITIES.md#ability_defiant--ability_competitive) |
| Cursed Body | Disables the move that hit it (30%, always with deterministic abilities). | [→](INNATE_ABILITIES.md#ability_cursed_body) |
| Cute Charm | May infatuate a contact attacker (30%, always with deterministic abilities). | [→](INNATE_ABILITIES.md#ability_cute_charm) |
| Dancer | Copies any dance move used by any battler. | [→](INNATE_ABILITIES.md#ability_dancer) |
| Defiant | +2 Attack when a foe lowers its stats. | [→](INNATE_ABILITIES.md#ability_defiant--ability_competitive) |
| Effect Spore | Lowers a contact attacker's accuracy by 1 (the fork's version of Effect Spore with deterministic abilities on). | [→](INNATE_ABILITIES.md#ability_effect_spore) |
| Electromorphosis | Charges its next Electric move when hit by any damaging move. | [→](INNATE_ABILITIES.md#ability_chilling_neigh--ability_grim_neigh--ability_electromorphosis) |
| Gooey | Lowers a contact attacker's Speed by 1. | [→](INNATE_ABILITIES.md#ability_rough_skin--ability_iron_barbs--ability_gooey--ability_tangling_hair) |
| Innards Out | A move that KOs it deals the attacker the HP it lost. | [→](INNATE_ABILITIES.md#ability_aftermath--ability_innards_out) |
| Iron Barbs | A contact attacker loses 1/8 max HP. | [→](INNATE_ABILITIES.md#ability_rough_skin--ability_iron_barbs--ability_gooey--ability_tangling_hair) |
| Justified | +1 Attack when hit by a Dark move. | [→](INNATE_ABILITIES.md#ability_justified--ability_stamina--ability_water_compaction--ability_anger_point) |
| Magician | Steals the item of a target it damages if it has none. | [→](INNATE_ABILITIES.md#ability_pickpocket--ability_magician--ability_liquid_ooze) |
| Opportunist | Copies any stat boost a foe gets. | [→](INNATE_ABILITIES.md#ability_opportunist) |
| Pickpocket | Steals a contact attacker's item if it has none. | [→](INNATE_ABILITIES.md#ability_pickpocket--ability_magician--ability_liquid_ooze) |
| Rattled | +1 Speed when hit by a Dark, Ghost or Bug move, or Intimidated. | [→](INNATE_ABILITIES.md#ability_rattled--ability_steadfast) |
| Rough Skin | A contact attacker loses 1/8 max HP. | [→](INNATE_ABILITIES.md#ability_rough_skin--ability_iron_barbs--ability_gooey--ability_tangling_hair) |
| Stamina | +1 Defense when hit by any move. | [→](INNATE_ABILITIES.md#ability_justified--ability_stamina--ability_water_compaction--ability_anger_point) |
| Steadfast | +1 Speed when it flinches. | [→](INNATE_ABILITIES.md#ability_rattled--ability_steadfast) |
| Steam Engine | +6 Speed when hit by a Fire or Water move. | [→](INNATE_ABILITIES.md#ability_steam_engine--ability_thermal_exchange--ability_wind_power) |
| Stench | 10% flinch chance on its hits (with deterministic abilities, a guaranteed flinch on its first turn out). | [→](INNATE_ABILITIES.md#ability_stench) |
| Tangling Hair | Lowers a contact attacker's Speed by 1. | [→](INNATE_ABILITIES.md#ability_rough_skin--ability_iron_barbs--ability_gooey--ability_tangling_hair) |
| Toxic Chain | Badly poisons the target of any damaging hit (30%, always with deterministic abilities). Not a pure boon. | [→](INNATE_ABILITIES.md#ability_toxic_chain) |
| Thermal Exchange | +1 Attack when hit by a Fire move; can't be burned. | [→](INNATE_ABILITIES.md#ability_steam_engine--ability_thermal_exchange--ability_wind_power) |
| Water Compaction | +2 Defense when hit by a Water move. | [→](INNATE_ABILITIES.md#ability_justified--ability_stamina--ability_water_compaction--ability_anger_point) |
| Willpower | Fork ability: +1 Sp. Def when hit by any move. | [→](INNATE_ABILITIES.md#ability_willpower) |
| Wind Power | Charges its next Electric move when hit by a wind move or when Tailwind is set. | [→](INNATE_ABILITIES.md#ability_steam_engine--ability_thermal_exchange--ability_wind_power) |

## Knock-out and faint triggers

| Ability | As an innate | |
| --- | --- | --- |
| Beast Boost | Its highest stat rises by 1 for each foe it KOs. | [→](INNATE_ABILITIES.md#ability_beast_boost) |
| Chilling Neigh | +1 Attack for each foe it KOs. | [→](INNATE_ABILITIES.md#ability_chilling_neigh--ability_grim_neigh--ability_electromorphosis) |
| Grim Neigh | +1 Sp. Atk for each foe it KOs. | [→](INNATE_ABILITIES.md#ability_chilling_neigh--ability_grim_neigh--ability_electromorphosis) |
| Moxie | +1 Attack for each foe it KOs. | [→](INNATE_ABILITIES.md#ability_moxie--ability_berserk--ability_soul_heart) |
| Rekindle | Fork ability: heals 1/4 max HP for each battler it KOs. | [→](INNATE_ABILITIES.md#ability_rekindle) |
| Soul-Heart | +1 Sp. Atk whenever any battler faints. | [→](INNATE_ABILITIES.md#ability_moxie--ability_berserk--ability_soul_heart) |

## Trapping and doubles support

| Ability | As an innate | |
| --- | --- | --- |
| Arena Trap | Grounded foes can't switch out. | [→](INNATE_ABILITIES.md#ability_shadow_tag--ability_arena_trap--ability_magnet_pull) |
| Aroma Veil | Its side is protected from infatuation, Taunt, Disable, Encore and similar. | [→](INNATE_ABILITIES.md#ability_battery--ability_power_spot--ability_telepathy--ability_aroma_veil--ability_flower_veil) |
| Flower Veil | Grass-type allies are protected from status and stat drops. | [→](INNATE_ABILITIES.md#ability_battery--ability_power_spot--ability_telepathy--ability_aroma_veil--ability_flower_veil) |
| Magnet Pull | Steel-type foes can't switch out. | [→](INNATE_ABILITIES.md#ability_shadow_tag--ability_arena_trap--ability_magnet_pull) |
| Shadow Tag | Foes can't switch out (except another Shadow Tag). | [→](INNATE_ABILITIES.md#ability_shadow_tag--ability_arena_trap--ability_magnet_pull) |
| Telepathy | Dodges its ally's damaging moves. | [→](INNATE_ABILITIES.md#ability_battery--ability_power_spot--ability_telepathy--ability_aroma_veil--ability_flower_veil) |

