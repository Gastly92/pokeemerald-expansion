#include "global.h"
#include "test/battle.h"
#include "fork/innate_abilities.h"

// FORK: coverage for FEATURE_INNATE_ABILITIES (config/feature.h). Feature flags
// default off in the test baseline (see TestInitConfigData), so each test that
// wants innates opts in with WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE). The
// supported innate set is LEVITATE (a passive Ground immunity), REGENERATOR
// (a silent 1/3-HP switch-out heal), UNAWARE (a passive calc modifier), STURDY
// (full-HP endure + OHKO-move immunity), NATURAL_CURE (a silent status cure on
// switch-out), PRANKSTER (+1 priority on status moves), the pinch and weather-speed
// abilities, FILTER (−25% supereffective damage taken), THICK_FAT (halves Fire/Ice
// damage taken), TECHNICIAN (+50% to moves of base power 60 or less), PRESSURE (the holder's foes
// spend 1 extra PP per move used against it), SPEED_BOOST (+1 Speed at the end of
// every turn — the first active, scripted end-turn innate), and the accuracy abilities
// COMPOUND_EYES / KEEN_EYE / ILLUMINATE (which all ignore the target's evasion — under
// DETERMINISTIC_ACCURACY_EVASION a PP-economy boon — and, for KEEN_EYE/ILLUMINATE, keep
// the holder's accuracy from being lowered); plus the Batch A offensive move-power boosters
// (IRON_FIST / RECKLESS / STRONG_JAW / TOUGH_CLAWS / SHARPNESS / MEGA_LAUNCHER / STEELWORKER /
// STEELY_SPIRIT / ROCKY_PAYLOAD / SAND_FORCE / ANALYTIC / ADAPTABILITY / PUNK_ROCK / STAKEOUT);
// plus SERENE_GRACE (doubles the chance of the holder's moves' additional effects).
// see src/innate_abilities.c.

// Flavor-floater coverage: a species with no native Levitate that floats by design
// (Magnemite hovers magnetically; primary is Magnet Pull/Sturdy/Analytic, and it's
// Electric/Steel so Ground is super effective) gains Ground immunity from the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: a flavor floater (Magnemite) gets innate Levitate")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MAGNEMITE, ABILITY_LEVITATE));
        ASSUME(gSpeciesInfo[SPECIES_MAGNEMITE].abilities[0] != ABILITY_LEVITATE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_MAGNEMITE);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_MUD_SLAP); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_LEVITATE);
        MESSAGE("It doesn't affect Magnemite…");
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: with the feature off, no innate Levitate (stock behavior)")
{
    GIVEN {
        // Feature is off by default in the test baseline; assert stock behavior.
        PLAYER(SPECIES_MAGNEZONE);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_MUD_SLAP); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(player, ABILITY_LEVITATE); }
        HP_BAR(player); // the super effective Ground hit connects
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker pierces an innate Levitate")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_MAGNEZONE);
        OPPONENT(SPECIES_TINKATON) { Ability(ABILITY_MOLD_BREAKER); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_MUD_SLAP); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(player, ABILITY_LEVITATE); }
        HP_BAR(player); // grounded through Mold Breaker, the hit connects
    }
}

// Comprehensive-table coverage: every species that natively carries Levitate is also
// listed as an innate (forward-looking — a later flag re-homes their primary). Today
// that's redundant, but it becomes observable the moment the primary is overwritten:
// the innate must keep the mon airborne. Worry Seed strips Gastly's primary Levitate
// (-> Insomnia); the feature-off leg's HP_BAR doubles as proof the primary really was
// removed (otherwise it would still be immune), so the feature-on leg's immunity can
// only come from the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Levitate persists after the primary Levitate is overwritten")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(GetMoveType(MOVE_MUD_SLAP) == TYPE_GROUND);
        ASSUME(SpeciesHasInnate(SPECIES_GASTLY, ABILITY_LEVITATE));
        ASSUME(gSpeciesInfo[SPECIES_GASTLY].abilities[0] == ABILITY_LEVITATE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_GASTLY); // primary Levitate AND (now) innate Levitate
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_WORRY_SEED, MOVE_MUD_SLAP); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_WORRY_SEED); } // primary Levitate -> Insomnia (pops the old Levitate)
        TURN { MOVE(opponent, MOVE_MUD_SLAP); }
    } SCENE {
        // Worry Seed pops the *old* ability (Levitate) as it overwrites, so we can't key
        // on the pop-up; the turn-2 outcome is the discriminator. Both legs run the same
        // Worry Seed, so the feature-off HP_BAR proves the primary Levitate really was
        // removed — making the feature-on immunity attributable only to the innate.
        if (enabled)
            MESSAGE("It doesn't affect Gastly…"); // innate Levitate still blocks the Ground hit
        else
            HP_BAR(player); // grounded: Insomnia replaced Levitate and there's no innate
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Levitate")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_MAGNEZONE);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_GASTRO_ACID); }
        TURN { MOVE(opponent, MOVE_MUD_SLAP); }
    } SCENE {
        MESSAGE("Magnezone's Ability was suppressed!"); // player side: no "opposing" prefix
        HP_BAR(player); // suppressed, so the Ground hit connects
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Trace copies only the primary ability, never an innate")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_GARDEVOIR) { Ability(ABILITY_TRACE); }
        OPPONENT(SPECIES_MAGNEZONE); // primary Magnet Pull, innate Levitate
    } WHEN {
        TURN {}
    } SCENE {
        // Trace resolves deterministically to the single primary ability slot,
        // never an innate.
        ABILITY_POPUP(player, ABILITY_TRACE);
        MESSAGE("It traced the opposing Magnezone's Magnet Pull!");
    }
}

// FORK divergence: an innate Levitate is a *pure boon*, NOT a 1:1 real Levitate. It still floats
// above Ground moves and entry hazards, but the fork keeps the mon grounded for the *beneficial*
// ground interactions (field terrain, Toxic Spikes absorption) via IsBattlerGroundedForBenefit.
// Tapu Bulu shows both halves at once: its Grassy Surge sets Grassy Terrain on entry, its innate
// Levitate blocks the Ground move, and it still heals 1/16 HP from that terrain at end of turn —
// healing that only happens because IsBattlerGroundedForBenefit counts the innate floater as
// grounded (IsBattlerGrounded alone returns FALSE here, so a broken boon would skip the heal).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Levitate is a pure boon — Tapu Bulu floats yet reaps its own Grassy Terrain")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_TAPU_BULU, ABILITY_LEVITATE));
        ASSUME(gSpeciesInfo[SPECIES_TAPU_BULU].abilities[0] == ABILITY_GRASSY_SURGE);
        ASSUME(GetMoveType(MOVE_MUD_SLAP) == TYPE_GROUND);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_TAPU_BULU) { MaxHP(100); HP(1); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_MUD_SLAP); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_MUD_SLAP); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_GRASSY_SURGE);           // sets its own terrain on entry
        MESSAGE("Grass grew to cover the battlefield!");
        ABILITY_POPUP(player, ABILITY_LEVITATE);               // still floats above the Ground move...
        MESSAGE("It doesn't affect Tapu Bulu…");
        s32 maxHPPlayer = GetMonData(&PLAYER_PARTY[0], MON_DATA_MAX_HP);
        MESSAGE("Tapu Bulu is healed by the grassy terrain!"); // ...yet reaps the terrain (the boon)
        HP_BAR(player, damage: -maxHPPlayer / 16);
    }
}

// Toxic Spikes: a Poison-type with an *innate* Levitate clears them (boon), whereas a *real*
// Levitate stays exempt (canonical). Weezing's primary ability is real Levitate AND it carries an
// innate Levitate, so toggling the feature isolates the divergence: feature on -> the innate is
// active -> it absorbs; feature off -> only the real Levitate remains -> it floats over the spikes
// without clearing them. (On the opponent side so the message reads "the opposing team".)
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: an innate Levitate Poison-type clears Toxic Spikes (real Levitate does not)")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(GetSpeciesType(SPECIES_WEEZING, 0) == TYPE_POISON);
        ASSUME(SpeciesHasInnate(SPECIES_WEEZING, ABILITY_LEVITATE));
        ASSUME(gSpeciesInfo[SPECIES_WEEZING].abilities[0] == ABILITY_LEVITATE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TOXIC_SPIKES, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WEEZING) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TOXIC_SPIKES); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_CELEBRATE); SWITCH(opponent, 1); }
    } SCENE {
        if (enabled)
            MESSAGE("The poison spikes disappeared from the ground around the opposing team!"); // innate boon absorbs
        else
            NONE_OF { MESSAGE("The poison spikes disappeared from the ground around the opposing team!"); } // real Levitate floats over them
    }
}

// The boon must not ground a *non*-Poison innate floater for the harmful half: it still dodges
// Toxic Spikes poison entirely. Mew (Psychic, flavor floater, primary Synchronize) is poisoned by
// Toxic Spikes only when grounded — feature off -> grounded -> poisoned; feature on -> innate
// Levitate floats it -> not poisoned (and, not being Poison-type, it does not clear them either).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: a non-Poison innate floater still dodges Toxic Spikes poison")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MEW, ABILITY_LEVITATE));
        ASSUME(gSpeciesInfo[SPECIES_MEW].abilities[0] != ABILITY_LEVITATE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TOXIC_SPIKES, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_MEW) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TOXIC_SPIKES); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_CELEBRATE); SWITCH(opponent, 1); }
    } SCENE {
        if (enabled)
            NONE_OF { STATUS_ICON(opponent, poison: TRUE); } // floats via innate -> not poisoned
        else
            STATUS_ICON(opponent, poison: TRUE);             // grounded -> poisoned
    }
}

AI_DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: AI will spread-Earthquake when its ally's innate Levitate makes it immune")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        // Earthquake hits the ally too (TARGET_FOES_AND_ALLY). The AI normally avoids it
        // when it would damage its own ally for no benefit (the Magnezone ally is
        // Steel/Electric, so Ground is super effective). With the feature on, the ally's
        // innate Levitate makes it immune, so the AI is willing to use Earthquake — proving
        // the partner-ability scoring honors the innate (battle_ai_main.c).
        ASSUME(GetMoveTarget(MOVE_EARTHQUAKE) == TARGET_FOES_AND_ALLY);
        ASSUME(GetMoveType(MOVE_EARTHQUAKE) == TYPE_GROUND);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_PHANPY) { Moves(MOVE_EARTHQUAKE, MOVE_SCRATCH); }
        OPPONENT(SPECIES_MAGNEZONE) { Moves(MOVE_CELEBRATE); } // innate Levitate when the feature is on
    } WHEN {
        if (enabled)
            TURN { EXPECT_MOVE(opponentLeft, MOVE_EARTHQUAKE); }                  // ally immune via innate Levitate
        else
            TURN { EXPECT_MOVE(opponentLeft, MOVE_SCRATCH, target: playerLeft); } // would hit the ally, so avoided
    }
}

AI_SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: AI treats an innate Levitate as Ground immunity")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        // The AI's damage prediction runs through the shared type calc, where the
        // innate immunity is applied off the battler's real species — so it sees the
        // immunity with no AI-specific plumbing. With the feature off, Earthquake is
        // 4x super effective on Steel/Electric Magnezone and the AI prefers it.
        ASSUME(GetMoveType(MOVE_EARTHQUAKE) == TYPE_GROUND);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY);
        PLAYER(SPECIES_MAGNEZONE); // innate Levitate when the feature is on
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_EARTHQUAKE, MOVE_SURF); }
    } WHEN {
        if (enabled)
            TURN { EXPECT_MOVE(opponent, MOVE_SURF); }       // Earthquake would do nothing
        else
            TURN { EXPECT_MOVE(opponent, MOVE_EARTHQUAKE); } // 4x super effective on Steel/Electric
    }
}

// ─── Innate Regenerator ──────────────────────────────────────────────────────
// A silent 1/3-max-HP heal on switch-out, wired additively at the single switch-out
// site (Cmd_switchoutabilities, src/battle_script_commands.c). Staryu is a flavor
// regenerator: it has no native Regenerator (Illuminate/Natural Cure/Analytic), so the
// heal is attributable solely to the innate, and toggling the feature isolates it.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Regenerator heals 1/3 max HP on switch-out")
{
    u32 currHP;
    PARAMETRIZE { currHP = 1; }
    PARAMETRIZE { currHP = 3; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_STARYU, ABILITY_REGENERATOR));
        ASSUME(gSpeciesInfo[SPECIES_STARYU].abilities[0] != ABILITY_REGENERATOR);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_STARYU) { HP(currHP); }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); }
        TURN { SWITCH(player, 0); }
    } SCENE {
        SWITCH_OUT_MESSAGE("Staryu");
        SEND_IN_MESSAGE("Wobbuffet");
        SWITCH_OUT_MESSAGE("Wobbuffet");
        SEND_IN_MESSAGE("Staryu");
    } THEN {
        EXPECT_EQ(player->hp, player->maxHP / 3 + currHP);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: with the feature off, no innate Regenerator heal (stock behavior)")
{
    GIVEN {
        // Feature off by default in the baseline; Staryu carries no native Regenerator.
        ASSUME(gSpeciesInfo[SPECIES_STARYU].abilities[0] != ABILITY_REGENERATOR);
        PLAYER(SPECIES_STARYU) { HP(1); }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); }
        TURN { SWITCH(player, 0); }
    } SCENE {
        SWITCH_OUT_MESSAGE("Staryu");
        SEND_IN_MESSAGE("Wobbuffet");
        SWITCH_OUT_MESSAGE("Wobbuffet");
        SEND_IN_MESSAGE("Staryu");
    } THEN {
        EXPECT_EQ(player->hp, 1); // switched out and back with no heal
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Regenerator's switch-out heal")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_STARYU) { HP(1); Moves(MOVE_CELEBRATE); }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_GASTRO_ACID); } // innate suppressed before it can switch
        TURN { SWITCH(player, 1); }
        TURN { SWITCH(player, 0); }
    } THEN {
        EXPECT_EQ(player->hp, 1); // suppressed -> no heal on switch-out
    }
}

// A canon Regenerator user (Corsola: Hustle/Natural Cure/Regenerator) whose *chosen*
// ability is Natural Cure ALSO carries Regenerator as an innate. Both must fire on
// switch-out: Natural Cure clears status AND the innate heals. This guards the wiring's
// direct party-HP write, which exists precisely so the heal doesn't clobber the single
// switch-out controller emit that Natural Cure already queued.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: chosen Natural Cure and innate Regenerator both fire on switch-out")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CORSOLA, ABILITY_REGENERATOR));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_CORSOLA) { Ability(ABILITY_NATURAL_CURE); HP(1); Status1(STATUS1_POISON); }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); } // switches first (start of turn), before any poison tick
        TURN { SWITCH(player, 0); }
    } THEN {
        EXPECT_EQ(player->hp, player->maxHP / 3 + 1); // innate Regenerator healed
        EXPECT_EQ(player->status1, STATUS1_NONE);     // chosen Natural Cure cleared the poison
    }
}

// ─── Innate Unaware ──────────────────────────────────────────────────────────
// A pure calc-modifier passive: the holder ignores the foe's stat-stage changes in
// the damage and accuracy calcs (src/battle_util.c). No script / pop-up / driver.
// Snorlax is a flavor pick (primary Immunity/Thick Fat/Gluttony, no native Unaware),
// so the stat-ignore is attributable solely to the innate; toggling the feature
// isolates it. Defensive half: the attacker's Attack boosts are ignored while the
// innate Unaware holder takes damage.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Unaware ignores the attacker's Attack boosts when taking damage")
{
    s16 damage[2];
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SNORLAX, ABILITY_UNAWARE));
        ASSUME(gSpeciesInfo[SPECIES_SNORLAX].abilities[0] != ABILITY_UNAWARE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_SNORLAX);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE, MOVE_SWORDS_DANCE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_TACKLE); }        // unboosted
        TURN { MOVE(opponent, MOVE_SWORDS_DANCE); }  // +2 Atk
        TURN { MOVE(opponent, MOVE_TACKLE); }        // boosted
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, opponent);
        HP_BAR(player, captureDamage: &damage[0]);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SWORDS_DANCE, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, opponent);
        HP_BAR(player, captureDamage: &damage[1]);
    } THEN {
        if (enabled)
            EXPECT_EQ(damage[1], damage[0]); // innate Unaware ignores the +2 Atk
        else
            EXPECT_GT(damage[1], damage[0]); // stock: the boost applies
    }
}

// Offensive half: the innate Unaware holder ignores the target's Defense boosts while
// dealing damage.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Unaware ignores the target's Defense boosts when dealing damage")
{
    s16 damage[2];
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SNORLAX, ABILITY_UNAWARE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_SNORLAX) { Moves(MOVE_TACKLE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_IRON_DEFENSE, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }       // target Def +0
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_IRON_DEFENSE); } // target Def +2
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }       // target Def +2
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, player);
        HP_BAR(opponent, captureDamage: &damage[0]);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_IRON_DEFENSE, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, player);
        HP_BAR(opponent, captureDamage: &damage[1]);
    } THEN {
        if (enabled)
            EXPECT_EQ(damage[1], damage[0]); // innate Unaware ignores the +2 Def
        else
            EXPECT_LT(damage[1], damage[0]); // stock: the boost reduces the hit
    }
}

// Suppression parity: Unaware is breakable, so an attacker's Mold Breaker ignores an
// innate Unaware on the defender exactly as it would the real ability.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker pierces an innate Unaware")
{
    s16 damage[2];
    GIVEN {
        ASSUME(gAbilitiesInfo[ABILITY_UNAWARE].breakable);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_SNORLAX); // innate Unaware
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MOLD_BREAKER); Moves(MOVE_TACKLE, MOVE_SWORDS_DANCE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_TACKLE); }
        TURN { MOVE(opponent, MOVE_SWORDS_DANCE); }
        TURN { MOVE(opponent, MOVE_TACKLE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, opponent);
        HP_BAR(player, captureDamage: &damage[0]);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SWORDS_DANCE, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, opponent);
        HP_BAR(player, captureDamage: &damage[1]);
    } THEN {
        EXPECT_GT(damage[1], damage[0]); // Mold Breaker ignores the innate -> the boost applies
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Unaware")
{
    s16 damage[2];
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_SNORLAX); // innate Unaware
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE, MOVE_SWORDS_DANCE, MOVE_GASTRO_ACID); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_TACKLE); }
        TURN { MOVE(opponent, MOVE_GASTRO_ACID); } // suppresses the innate
        TURN { MOVE(opponent, MOVE_SWORDS_DANCE); }
        TURN { MOVE(opponent, MOVE_TACKLE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, opponent);
        HP_BAR(player, captureDamage: &damage[0]);
        MESSAGE("Snorlax's Ability was suppressed!"); // player side: no "opposing" prefix
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SWORDS_DANCE, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, opponent);
        HP_BAR(player, captureDamage: &damage[1]);
    } THEN {
        EXPECT_GT(damage[1], damage[0]); // suppressed -> the +2 Atk now applies
    }
}

// A canon Unaware user (Clefable: Cute Charm/Magic Guard/Unaware) whose *chosen* ability
// is something else still carries Unaware as an innate, so it keeps the stat-ignore no
// matter which slot the build picks.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: a canon Unaware user keeps it via innate when the chosen ability differs")
{
    s16 damage[2];
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CLEFABLE, ABILITY_UNAWARE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_CLEFABLE) { Ability(ABILITY_MAGIC_GUARD); } // chosen ability is NOT Unaware
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE, MOVE_SWORDS_DANCE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_TACKLE); }
        TURN { MOVE(opponent, MOVE_SWORDS_DANCE); }
        TURN { MOVE(opponent, MOVE_TACKLE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, opponent);
        HP_BAR(player, captureDamage: &damage[0]);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SWORDS_DANCE, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, opponent);
        HP_BAR(player, captureDamage: &damage[1]);
    } THEN {
        EXPECT_EQ(damage[1], damage[0]); // innate Unaware ignores the +2 Atk despite chosen Magic Guard
    }
}

// FORK divergence: an innate Unaware is a *pure boon*, NOT a 1:1 real Unaware. A real
// Unaware blanks the foe's stat stage both ways — so it ignores a foe's *drop* too and
// takes more damage for it — whereas the innate ignores only the foe's *boosts* and keeps
// its drops (via InnateUnawareBoonStage). Defensive half: with the attacker's Attack
// lowered, an innate-only Unaware respects the drop (less damage), while a real Unaware
// ignores it (full damage). Clefable can run either: chosen Magic Guard -> Unaware is
// innate-only; chosen Unaware -> the real ability.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Unaware keeps the attacker's Attack drop (pure boon); real Unaware ignores it")
{
    s16 damage[2];
    enum Ability chosen;
    PARAMETRIZE { chosen = ABILITY_MAGIC_GUARD; } // Unaware innate-only -> boon (respects the drop)
    PARAMETRIZE { chosen = ABILITY_UNAWARE; }     // real Unaware -> ignores the drop (canon)
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CLEFABLE, ABILITY_UNAWARE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_CLEFABLE) { Ability(chosen); Moves(MOVE_GROWL, MOVE_CELEBRATE); } // outspeeds Wobbuffet
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_TACKLE); } // neutral Atk
        TURN { MOVE(player, MOVE_GROWL);     MOVE(opponent, MOVE_TACKLE); } // Growl resolves first: -1 Atk, then Tackle
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, opponent);
        HP_BAR(player, captureDamage: &damage[0]);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_GROWL, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, opponent);
        HP_BAR(player, captureDamage: &damage[1]);
    } THEN {
        if (chosen == ABILITY_MAGIC_GUARD)
            EXPECT_LT(damage[1], damage[0]); // boon: the -1 Atk drop is respected -> less damage
        else
            EXPECT_EQ(damage[1], damage[0]); // real Unaware ignores the drop -> same damage
    }
}

// Offensive half of the same divergence: against a target that lowered its own Sp. Def,
// an innate-only Unaware keeps the drop (more damage), while a real Unaware ignores it.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Unaware keeps the target's Defense drop (pure boon); real Unaware ignores it")
{
    s16 damage[2];
    enum Ability chosen;
    PARAMETRIZE { chosen = ABILITY_MAGIC_GUARD; } // Unaware innate-only -> boon (respects the drop)
    PARAMETRIZE { chosen = ABILITY_UNAWARE; }     // real Unaware -> ignores the drop (canon)
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CLEFABLE, ABILITY_UNAWARE));
        ASSUME(GetMoveCategory(MOVE_SWIFT) == DAMAGE_CATEGORY_SPECIAL); // reads the target's Sp. Def
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_CLEFABLE) { Ability(chosen); Moves(MOVE_SWIFT, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SHELL_SMASH, MOVE_CELEBRATE); } // self -1 Sp. Def
    } WHEN {
        TURN { MOVE(player, MOVE_SWIFT);    MOVE(opponent, MOVE_CELEBRATE); }     // neutral Sp. Def
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_SHELL_SMASH); } // target Sp. Def -1
        TURN { MOVE(player, MOVE_SWIFT);    MOVE(opponent, MOVE_CELEBRATE); }     // target Sp. Def -1
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SWIFT, player);
        HP_BAR(opponent, captureDamage: &damage[0]);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SHELL_SMASH, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SWIFT, player);
        HP_BAR(opponent, captureDamage: &damage[1]);
    } THEN {
        if (chosen == ABILITY_MAGIC_GUARD)
            EXPECT_GT(damage[1], damage[0]); // boon: the target's -1 Sp. Def is respected -> more damage
        else
            EXPECT_EQ(damage[1], damage[0]); // real Unaware ignores the target's Sp. Def drop
    }
}

// ─── Innate Sturdy ───────────────────────────────────────────────────────────
// A pure boon with no downside, so a 1:1 copy of the real ability: the holder endures a
// lethal hit at full HP (B_STURDY >= GEN_5) and is immune to OHKO moves. Wired at the two
// effect sites in src/battle_util.c; the "endured"/Sturdy pop-up & message flow from the
// existing MOVE_RESULT_STURDIED / MOVE_RESULT_ONE_HIT_KO_STURDY flags. Cloyster is a flavor
// pick (primary Shell Armor, no native Sturdy), so the endure is attributable solely to the
// innate; toggling the feature isolates it. Seismic Toss deals fixed damage equal to the
// user's level (100), so a MaxHP-100 Cloyster at full HP takes exactly a lethal hit.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Sturdy endures a lethal hit at full HP")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CLOYSTER, ABILITY_STURDY));
        ASSUME(gSpeciesInfo[SPECIES_CLOYSTER].abilities[0] != ABILITY_STURDY);
        ASSUME(B_STURDY >= GEN_5);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_CLOYSTER) { MaxHP(100); HP(100); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SEISMIC_TOSS); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SEISMIC_TOSS, opponent);
        if (enabled) {
            HP_BAR(player, hp: 1);                 // endured at 1 HP via the innate
            ABILITY_POPUP(player, ABILITY_STURDY); // pop-up shows Sturdy, not the chosen Shell Armor
            MESSAGE("Cloyster endured the hit!");
        } else {
            HP_BAR(player, hp: 0);                 // no innate -> the lethal hit KOs
        }
    }
}

// OHKO-move immunity is not gen-gated and the Sturdy check precedes the hit roll, so a single
// feature-on leg is a clean, RNG-free proof: a flavor Cloyster (no native Sturdy) is immune to
// Fissure only via the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Sturdy is immune to OHKO moves")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FISSURE) == EFFECT_OHKO);
        ASSUME(SpeciesHasInnate(SPECIES_CLOYSTER, ABILITY_STURDY));
        ASSUME(gSpeciesInfo[SPECIES_CLOYSTER].abilities[0] != ABILITY_STURDY);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_CLOYSTER);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_FISSURE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_STURDY);
        MESSAGE("It doesn't affect Cloyster…");
    } THEN {
        EXPECT_EQ(player->hp, player->maxHP);
    }
}

// Suppression parity: Sturdy is breakable, so an attacker's Mold Breaker pierces an innate
// Sturdy exactly as it would the real ability — the full-HP endure no longer triggers.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker pierces an innate Sturdy")
{
    GIVEN {
        ASSUME(gAbilitiesInfo[ABILITY_STURDY].breakable);
        ASSUME(B_STURDY >= GEN_5);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_CLOYSTER) { MaxHP(100); HP(100); } // innate Sturdy
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MOLD_BREAKER); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SEISMIC_TOSS); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SEISMIC_TOSS, opponent);
        HP_BAR(player, hp: 0); // Mold Breaker ignores the innate -> no endure -> KO
        NONE_OF { ABILITY_POPUP(player, ABILITY_STURDY); }
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Sturdy")
{
    GIVEN {
        ASSUME(B_STURDY >= GEN_5);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_CLOYSTER) { MaxHP(100); HP(100); Moves(MOVE_CELEBRATE); } // innate Sturdy
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID, MOVE_SEISMIC_TOSS); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_GASTRO_ACID); } // suppresses the innate
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_SEISMIC_TOSS); }
    } SCENE {
        MESSAGE("Cloyster's Ability was suppressed!"); // player side: no "opposing" prefix
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SEISMIC_TOSS, opponent);
        HP_BAR(player, hp: 0); // suppressed -> no endure -> KO
    } THEN {
        EXPECT_EQ(player->hp, 0);
    }
}

// A canon Sturdy user (Skarmory: Keen Eye/Sturdy/Weak Armor) whose *chosen* ability is Keen Eye
// still carries Sturdy as an innate, so it keeps the full-HP endure no matter which slot the
// build picks.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: a canon Sturdy user keeps it via innate when the chosen ability differs")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SKARMORY, ABILITY_STURDY));
        ASSUME(B_STURDY >= GEN_5);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_SKARMORY) { Ability(ABILITY_KEEN_EYE); MaxHP(100); HP(100); } // chosen ability is NOT Sturdy
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SEISMIC_TOSS); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SEISMIC_TOSS, opponent);
        HP_BAR(player, hp: 1);
        ABILITY_POPUP(player, ABILITY_STURDY); // shows Sturdy despite chosen Keen Eye
        MESSAGE("Skarmory endured the hit!");
    } THEN {
        EXPECT_EQ(player->hp, 1);
    }
}

// AI coverage. Unlike Levitate (immunity in the shared type calc) and Unaware (stat-ignore in the
// shared damage calc), Sturdy's survival reasoning lives in DEDICATED AI helpers that read the cached
// chosen ability — so the innate had to be wired into them explicitly (BattlerHasAbility in
// battle_ai_util.c, SpeciesHasInnate in the battle_ai_switch.c KO sim). Here the OHKO-move avoidance:
// the AI won't throw a (No Guard, sure-hit) Fissure at a mon it knows is OHKO-immune via innate Sturdy.
AI_SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: AI treats an innate Sturdy as OHKO-move immunity")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FISSURE) == EFFECT_OHKO);
        ASSUME(SpeciesHasInnate(SPECIES_CLOYSTER, ABILITY_STURDY));
        ASSUME(gSpeciesInfo[SPECIES_CLOYSTER].abilities[0] != ABILITY_STURDY);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_CLOYSTER); // innate Sturdy when the feature is on
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_NO_GUARD); Moves(MOVE_FISSURE, MOVE_TACKLE); } // No Guard -> Fissure is a sure OHKO
    } WHEN {
        if (enabled)
            TURN { EXPECT_MOVE(opponent, MOVE_TACKLE); }  // innate Sturdy is OHKO-immune -> AI avoids the wasted Fissure
        else
            TURN { EXPECT_MOVE(opponent, MOVE_FISSURE); } // no innate -> the guaranteed OHKO is the AI's best move
    }
}

// AI innate-awareness for Regenerator. Regenerator's effect (a switch-out heal) isn't in any shared
// calc the AI runs — the AI reasons about it only in dedicated `== ABILITY_REGENERATOR` switch/pivot
// reads, which are now innate-aware (BattlerHasAbility). Mirrors the chosen-Regenerator switch test
// in ai_switching.c, but Staryu has NO native Regenerator (chosen Natural Cure, and it's unstatused
// so Natural Cure offers no switch reason) — so the AI banks the 1/3 heal by switching only via the
// innate pre-check in ShouldSwitchIfAbilityBenefit.
AI_SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: AI switches an innate-Regenerator mon to bank the heal")
{
    PASSES_RANDOMLY(SHOULD_SWITCH_REGENERATOR_PERCENTAGE, 100, RNG_AI_SWITCH_REGENERATOR);
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_STARYU, ABILITY_REGENERATOR));
        ASSUME(gSpeciesInfo[SPECIES_STARYU].abilities[0] != ABILITY_REGENERATOR);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_SCRATCH); }
        OPPONENT(SPECIES_STARYU) { MaxHP(100); HP(65); Ability(ABILITY_NATURAL_CURE); Moves(MOVE_SCRATCH); } // chosen != Regenerator; switch is driven by the innate
        OPPONENT(SPECIES_STARYU) { Ability(ABILITY_NATURAL_CURE); Moves(MOVE_SCRATCH); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); EXPECT_SWITCH(opponent, 1); }
    }
}

// ─── Innate Natural Cure ─────────────────────────────────────────────────────
// A silent status cure on switch-out, wired additively at the same single switch-out
// site as Regenerator (Cmd_switchoutabilities, src/battle_script_commands.c). Chikorita
// is a flavor pick: it has no native Natural Cure (Overgrow/Leaf Guard), so the cure is
// attributable solely to the innate, and toggling the feature isolates it.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Natural Cure cures status on switch-out")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CHIKORITA, ABILITY_NATURAL_CURE));
        ASSUME(gSpeciesInfo[SPECIES_CHIKORITA].abilities[0] != ABILITY_NATURAL_CURE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_CHIKORITA) { Status1(STATUS1_BURN); }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); } // switches out at the start of the turn
        TURN { SWITCH(player, 0); }
    } THEN {
        if (enabled)
            EXPECT_EQ(player->status1, STATUS1_NONE); // innate cured the burn on switch-out
        else
            EXPECT_EQ(player->status1, STATUS1_BURN);  // stock: the burn persists
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Natural Cure's switch-out cure")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_CHIKORITA) { Status1(STATUS1_BURN); Moves(MOVE_CELEBRATE); }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_GASTRO_ACID); } // innate suppressed before it can switch
        TURN { SWITCH(player, 1); }
        TURN { SWITCH(player, 0); }
    } THEN {
        EXPECT_EQ(player->status1, STATUS1_BURN); // suppressed -> no cure on switch-out
    }
}

// A canon Natural Cure user (Blissey: Natural Cure/Serene Grace/Healer) whose *chosen* ability is
// Serene Grace still carries Natural Cure as an innate, so it keeps the self-cleanse no matter which
// slot the build picks (exactly the Frontier roster's Blissey set after Step 3.5 freed its slot).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: a canon Natural Cure user keeps it via innate when the chosen ability differs")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_BLISSEY, ABILITY_NATURAL_CURE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_BLISSEY) { Ability(ABILITY_SERENE_GRACE); Status1(STATUS1_POISON); } // chosen ability is NOT Natural Cure
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); }
        TURN { SWITCH(player, 0); }
    } THEN {
        EXPECT_EQ(player->status1, STATUS1_NONE); // innate Natural Cure cleared the poison despite chosen Serene Grace
    }
}

// Corsola carries BOTH an innate Regenerator and an innate Natural Cure (canon Hustle/Natural Cure/
// Regenerator). With a chosen Hustle, NEITHER is the chosen ability, so both innate switch-out blocks
// must fire and coexist: each writes the party mon directly (HP for Regenerator, status for Natural
// Cure), so neither clobbers the other nor the single switch-out controller emit.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Regenerator and innate Natural Cure both fire on switch-out")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CORSOLA, ABILITY_REGENERATOR));
        ASSUME(SpeciesHasInnate(SPECIES_CORSOLA, ABILITY_NATURAL_CURE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_CORSOLA) { Ability(ABILITY_HUSTLE); HP(1); Status1(STATUS1_POISON); } // chosen is neither innate
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); } // switches first (start of turn), before any poison tick
        TURN { SWITCH(player, 0); }
    } THEN {
        EXPECT_EQ(player->hp, player->maxHP / 3 + 1); // innate Regenerator healed 1/3
        EXPECT_EQ(player->status1, STATUS1_NONE);     // innate Natural Cure cured the poison
    }
}

// AI innate-awareness for Natural Cure. Like Regenerator, the cure isn't in any shared calc the AI
// runs — the AI reasons about it only in dedicated `== ABILITY_NATURAL_CURE` switch reads, now
// innate-aware (BattlerHasAbility). Meganium has NO native Natural Cure (chosen Overgrow), so it
// switches to cleanse a bad status only via the innate pre-check in ShouldSwitchIfAbilityBenefit.
// Mirrors the chosen-Natural-Cure switch test in ai_switching.c.
AI_SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: AI switches an innate-Natural-Cure mon to cure a bad status")
{
    PASSES_RANDOMLY(SHOULD_SWITCH_NATURAL_CURE_STRONG_PERCENTAGE, 100, RNG_AI_SWITCH_NATURAL_CURE);
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MEGANIUM, ABILITY_NATURAL_CURE));
        ASSUME(gSpeciesInfo[SPECIES_MEGANIUM].abilities[0] != ABILITY_NATURAL_CURE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_ODDISH) { Moves(MOVE_TOXIC, MOVE_SCRATCH); }
        OPPONENT(SPECIES_MEGANIUM) { Ability(ABILITY_OVERGROW); Moves(MOVE_SCRATCH); } // chosen != Natural Cure; switch driven by the innate
        OPPONENT(SPECIES_MEGANIUM) { Ability(ABILITY_OVERGROW); Moves(MOVE_SCRATCH); }
    } WHEN {
        TURN { MOVE(player, MOVE_TOXIC); }
        TURN { MOVE(player, MOVE_SCRATCH); EXPECT_SWITCH(opponent, 1); }
    }
}

// ─── Innate Prankster (status moves get +1 priority) ────────────────────────────
// Murkrow carries Prankster as its Hidden Ability, so a build can pick a different
// chosen ability (Insomnia) and only the innate supplies Prankster — that's what makes
// the innate observable.

// The signature effect: a slower mon with innate Prankster still moves first when using
// a status move. Feature-off leg proves the priority comes only from the innate (stock
// behavior: the faster mon goes first).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Prankster gives the holder's status moves +1 priority")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MURKROW, ABILITY_PRANKSTER));
        ASSUME(GetMoveCategory(MOVE_CONFUSE_RAY) == DAMAGE_CATEGORY_STATUS);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET) { Speed(10); }
        OPPONENT(SPECIES_MURKROW) { Speed(5); Ability(ABILITY_INSOMNIA); } // chosen Insomnia; Prankster only as innate
    } WHEN {
        TURN { MOVE(opponent, MOVE_CONFUSE_RAY); MOVE(player, MOVE_CELEBRATE, WITH_RNG(RNG_CONFUSION, FALSE)); }
    } SCENE {
        if (enabled) {
            // innate Prankster elevates Confuse Ray: the slower Murkrow moves first
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CONFUSE_RAY, opponent);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
        } else {
            // feature off: no innate, the faster Wobbuffet moves first
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CONFUSE_RAY, opponent);
        }
    }
}

// Pure-boon divergence: a *real* Prankster's boosted status move fails against Dark-types
// (B_PRANKSTER_DARK_TYPES >= GEN_7); an *innate* Prankster keeps the +1 priority but never
// sets pranksterElevated, so the move still lands on Dark-types — the favorable half, with
// the real ability's only cost dropped.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: an innate Prankster status move still lands on Dark-types (pure boon)")
{
    enum Ability ability;
    bool32 enabled;
    PARAMETRIZE { ability = ABILITY_PRANKSTER; enabled = FALSE; } // real Prankster: blocked by the Dark-type
    PARAMETRIZE { ability = ABILITY_INSOMNIA;  enabled = TRUE;  } // innate Prankster: NOT blocked (pure boon)
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MURKROW, ABILITY_PRANKSTER));
        ASSUME(GetSpeciesType(SPECIES_UMBREON, 0) == TYPE_DARK);
        WITH_CONFIG(B_PRANKSTER_DARK_TYPES, GEN_7);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_UMBREON);
        OPPONENT(SPECIES_MURKROW) { Ability(ability); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CONFUSE_RAY); }
    } SCENE {
        if (ability == ABILITY_PRANKSTER) {
            NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_CONFUSE_RAY, opponent);
            MESSAGE("It doesn't affect Umbreon…");
        } else {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CONFUSE_RAY, opponent);
            MESSAGE("Umbreon became confused!");
        }
    }
}

// Suppression parity: Gastro Acid disables the innate exactly like a real ability, so the
// priority boost is gone and the faster mon moves first again.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Prankster's priority")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MURKROW, ABILITY_PRANKSTER));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(10); }
        OPPONENT(SPECIES_MURKROW) { Speed(5); Ability(ABILITY_INSOMNIA); }
    } WHEN {
        TURN { MOVE(player, MOVE_GASTRO_ACID); } // suppresses Murkrow's abilities, the innate Prankster included
        TURN { MOVE(opponent, MOVE_CONFUSE_RAY); MOVE(player, MOVE_CELEBRATE, WITH_RNG(RNG_CONFUSION, FALSE)); }
    } SCENE {
        MESSAGE("Wobbuffet used Gastro Acid!");
        // turn 2: with the innate suppressed, Confuse Ray is no longer elevated, so the
        // faster Wobbuffet moves first
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CONFUSE_RAY, opponent);
    }
}

// Flavor-Prankster coverage: a mischievous species with no native Prankster in any
// ability slot (Aipom is Run Away / Pickup / Skill Link) still gets the +1 priority from
// the innate — mirrors the flavor-floater Levitate test above.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: a flavor Prankster (Aipom) gets innate priority on status moves")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_AIPOM, ABILITY_PRANKSTER));
        ASSUME(gSpeciesInfo[SPECIES_AIPOM].abilities[0] != ABILITY_PRANKSTER);
        ASSUME(GetMoveCategory(MOVE_CONFUSE_RAY) == DAMAGE_CATEGORY_STATUS);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(10); }
        OPPONENT(SPECIES_AIPOM) { Speed(5); } // no native Prankster; only the innate supplies it
    } WHEN {
        TURN { MOVE(opponent, MOVE_CONFUSE_RAY); MOVE(player, MOVE_CELEBRATE, WITH_RNG(RNG_CONFUSION, FALSE)); }
    } SCENE {
        // innate Prankster elevates Confuse Ray: the slower Aipom moves first
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CONFUSE_RAY, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
    }
}

// ─── Innate Gale Wings (Flying moves get +1 priority at full HP) ─────────────────
// Talonflame carries Gale Wings as its Hidden Ability, so a build can pick a different
// chosen ability (Flame Body) and only the innate supplies Gale Wings — that's what makes
// the innate observable. Gale Wings is a clean upside, so the innate is a 1:1 copy.

// The signature effect: a slower holder at full HP moves first with a Flying move. The
// feature-off leg proves the priority comes only from the innate (stock: faster mon first).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Gale Wings gives the holder's Flying moves +1 priority")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_TALONFLAME, ABILITY_GALE_WINGS));
        ASSUME(GetMoveType(MOVE_AERIAL_ACE) == TYPE_FLYING);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET) { Speed(10); }
        OPPONENT(SPECIES_TALONFLAME) { Speed(5); Ability(ABILITY_FLAME_BODY); } // chosen Flame Body; Gale Wings only as innate
    } WHEN {
        TURN { MOVE(opponent, MOVE_AERIAL_ACE); MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        if (enabled) {
            // innate Gale Wings elevates Aerial Ace: the slower Talonflame moves first
            ANIMATION(ANIM_TYPE_MOVE, MOVE_AERIAL_ACE, opponent);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
        } else {
            // feature off: no innate, the faster Wobbuffet moves first
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_AERIAL_ACE, opponent);
        }
    }
}

// 1:1 semantics: Gale Wings is a clean upside, so the innate copies the real ability's
// full-HP gate (B_GALE_WINGS >= GEN_7) exactly — below full HP it grants no priority.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Gale Wings honors the full-HP gate like the real ability")
{
    u32 hp;
    PARAMETRIZE { hp = 100; } // full HP: innate grants priority
    PARAMETRIZE { hp = 99;  } // below full HP (Gen 7+): no priority
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_TALONFLAME, ABILITY_GALE_WINGS));
        ASSUME(GetMoveType(MOVE_AERIAL_ACE) == TYPE_FLYING);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(B_GALE_WINGS, GEN_7);
        PLAYER(SPECIES_WOBBUFFET) { Speed(10); }
        OPPONENT(SPECIES_TALONFLAME) { Speed(5); Ability(ABILITY_FLAME_BODY); HP(hp); MaxHP(100); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_AERIAL_ACE); MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        if (hp == 100) {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_AERIAL_ACE, opponent);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
        } else {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_AERIAL_ACE, opponent);
        }
    }
}

// Suppression parity: Gastro Acid disables the innate exactly like a real ability, so the
// priority boost is gone and the faster mon moves first again.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Gale Wings' priority")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_TALONFLAME, ABILITY_GALE_WINGS));
        ASSUME(GetMoveType(MOVE_AERIAL_ACE) == TYPE_FLYING);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(10); }
        OPPONENT(SPECIES_TALONFLAME) { Speed(5); Ability(ABILITY_FLAME_BODY); }
    } WHEN {
        TURN { MOVE(player, MOVE_GASTRO_ACID); } // suppresses Talonflame's abilities, the innate Gale Wings included
        TURN { MOVE(opponent, MOVE_AERIAL_ACE); MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Wobbuffet used Gastro Acid!");
        // turn 2: with the innate suppressed, Aerial Ace is no longer elevated, so the
        // faster Wobbuffet moves first
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_AERIAL_ACE, opponent);
    }
}

// ─── Innate Triage (healing moves get +3 priority) ──────────────────────────────
// Comfey carries Triage as a normal ability, so a build can pick a different chosen
// ability (Flower Veil) and only the innate supplies Triage. Triage is a clean upside,
// so the innate is a 1:1 copy.

// The signature effect: a slower holder moves first with a healing move. The feature-off
// leg proves the priority comes only from the innate (stock: faster mon first).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Triage gives the holder's healing moves +3 priority")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_COMFEY, ABILITY_TRIAGE));
        ASSUME(IsHealingMove(MOVE_SYNTHESIS));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET) { Speed(10); }
        OPPONENT(SPECIES_COMFEY) { Speed(5); Ability(ABILITY_FLOWER_VEIL); HP(50); MaxHP(100); } // chosen Flower Veil; Triage only as innate
    } WHEN {
        TURN { MOVE(opponent, MOVE_SYNTHESIS); MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        if (enabled) {
            // innate Triage elevates Synthesis: the slower Comfey moves first
            ANIMATION(ANIM_TYPE_MOVE, MOVE_SYNTHESIS, opponent);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
        } else {
            // feature off: no innate, the faster Wobbuffet moves first
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_SYNTHESIS, opponent);
        }
    }
}

// Suppression parity: Gastro Acid disables the innate exactly like a real ability, so the
// priority boost is gone and the faster mon moves first again.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Triage's priority")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_COMFEY, ABILITY_TRIAGE));
        ASSUME(IsHealingMove(MOVE_SYNTHESIS));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(10); }
        OPPONENT(SPECIES_COMFEY) { Speed(5); Ability(ABILITY_FLOWER_VEIL); HP(50); MaxHP(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_GASTRO_ACID); } // suppresses Comfey's abilities, the innate Triage included
        TURN { MOVE(opponent, MOVE_SYNTHESIS); MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Wobbuffet used Gastro Acid!");
        // turn 2: with the innate suppressed, Synthesis is no longer elevated, so the
        // faster Wobbuffet moves first
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SYNTHESIS, opponent);
    }
}

// ─── Innate Dazzling / Queenly Majesty / Armor Tail (block priority moves) ───────
// These three abilities stop opponents from using increased-priority moves against the holder
// or its allies. They are a clean upside (never hurt the holder), so the innate is a 1:1 copy,
// wired at the shared CancelerPriorityBlock effect site (src/battle_move_resolution.c) via
// GetBattlerDazzlingAbility. Each canon carrier keeps a different chosen ability here so the block
// comes solely from the innate, and the pop-up must show the innate, not the chosen ability.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: an innate Dazzling/Queenly Majesty/Armor Tail blocks priority moves against the holder")
{
    u32 species;
    enum Ability chosen, innate;
    bool32 enabled;
    PARAMETRIZE { species = SPECIES_BRUXISH;   chosen = ABILITY_WONDER_SKIN; innate = ABILITY_DAZZLING;        enabled = TRUE;  }
    PARAMETRIZE { species = SPECIES_TSAREENA;  chosen = ABILITY_LEAF_GUARD;  innate = ABILITY_QUEENLY_MAJESTY; enabled = TRUE;  }
    PARAMETRIZE { species = SPECIES_FARIGIRAF; chosen = ABILITY_SAP_SIPPER;  innate = ABILITY_ARMOR_TAIL;      enabled = TRUE;  }
    PARAMETRIZE { species = SPECIES_BRUXISH;   chosen = ABILITY_WONDER_SKIN; innate = ABILITY_DAZZLING;        enabled = FALSE; }
    GIVEN {
        ASSUME(GetMovePriority(MOVE_QUICK_ATTACK) > 0);
        ASSUME(SpeciesHasInnate(species, innate));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(species) { Ability(chosen); } // chosen ability differs from the innate blocker
    } WHEN {
        TURN { MOVE(player, MOVE_QUICK_ATTACK); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(opponent, innate); // pop-up shows the INNATE, not the chosen ability
            MESSAGE("Wobbuffet cannot use Quick Attack!");
        } else {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_QUICK_ATTACK, player); // feature off: no innate, the move lands
        }
    }
}

DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: an innate priority blocker also protects its ally")
{
    GIVEN {
        ASSUME(GetMovePriority(MOVE_QUICK_ATTACK) > 0);
        ASSUME(SpeciesHasInnate(SPECIES_TSAREENA, ABILITY_QUEENLY_MAJESTY));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_TSAREENA) { Ability(ABILITY_LEAF_GUARD); } // innate Queenly Majesty; chosen Leaf Guard
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_QUICK_ATTACK, target: opponentRight); } // priority move at the ally
    } SCENE {
        ABILITY_POPUP(opponentLeft, ABILITY_QUEENLY_MAJESTY);
        MESSAGE("Wobbuffet cannot use Quick Attack!");
    }
}

// Suppression parity: Gastro Acid disables the innate exactly like a real ability, so the
// priority block is gone and the move goes through on the following turn.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate priority blocker")
{
    GIVEN {
        ASSUME(GetMovePriority(MOVE_QUICK_ATTACK) > 0);
        ASSUME(SpeciesHasInnate(SPECIES_TSAREENA, ABILITY_QUEENLY_MAJESTY));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_TSAREENA) { Ability(ABILITY_LEAF_GUARD); } // innate Queenly Majesty
    } WHEN {
        TURN { MOVE(player, MOVE_GASTRO_ACID); }   // suppresses Tsareena's abilities, the innate included
        TURN { MOVE(player, MOVE_QUICK_ATTACK); }
    } SCENE {
        MESSAGE("Wobbuffet used Gastro Acid!");
        // turn 2: with the innate suppressed, the priority move is no longer blocked
        ANIMATION(ANIM_TYPE_MOVE, MOVE_QUICK_ATTACK, player);
    }
}

// AI innate-awareness: the priority-block reasoning lives in a dedicated helper (Ai_IsPriorityBlocked),
// not the shared damage calc, so it had to be made innate-aware. Mirrors the chosen-ability upstream test
// "First Impression is not chosen if it's blocked by certain abilities" (test/battle/ai/ai.c) but with the
// block supplied only by the innate. The feature-off leg proves the AI *does* pick the priority move here
// when no block is in play, so the feature-on restraint can only come from the innate.
AI_SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: AI won't use a priority move an innate blocker would stop")
{
    u32 species;
    enum Ability chosen, innate;
    bool32 enabled;
    PARAMETRIZE { species = SPECIES_BRUXISH;   chosen = ABILITY_WONDER_SKIN; innate = ABILITY_DAZZLING;        enabled = TRUE;  }
    PARAMETRIZE { species = SPECIES_TSAREENA;  chosen = ABILITY_LEAF_GUARD;  innate = ABILITY_QUEENLY_MAJESTY; enabled = TRUE;  }
    PARAMETRIZE { species = SPECIES_FARIGIRAF; chosen = ABILITY_SAP_SIPPER;  innate = ABILITY_ARMOR_TAIL;      enabled = TRUE;  }
    PARAMETRIZE { species = SPECIES_BRUXISH;   chosen = ABILITY_WONDER_SKIN; innate = ABILITY_DAZZLING;        enabled = FALSE; }
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FIRST_IMPRESSION) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(GetMovePower(MOVE_FIRST_IMPRESSION) == 90);
        ASSUME(GetMovePower(MOVE_LUNGE) == 80);
        ASSUME(SpeciesHasInnate(species, innate));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(species) { Ability(chosen); } // innate blocker; chosen ability differs
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_FIRST_IMPRESSION, MOVE_LUNGE); }
    } WHEN {
        if (enabled)
            TURN { EXPECT_MOVE(opponent, MOVE_LUNGE); }           // innate blocks First Impression (priority) -> AI picks Lunge
        else
            TURN { EXPECT_MOVE(opponent, MOVE_FIRST_IMPRESSION); } // no innate -> the stronger priority move is fine
    }
}

// ─── AI awareness of an innate Unaware (off-field setup heuristic) ──────────────
// Companion to the chosen-Unaware test "AI won't boost stats against opponent with Unaware"
// (test/battle/ai/ai.c): the AI shouldn't waste a Swords Dance against a foe whose Unaware
// (boost-ignoring) makes the +Atk pointless — and that must hold when the foe's Unaware is
// innate-only. A setup sweeper (Lopunny) freely boosts against the passive Pyukumuku wall;
// the feature-off leg proves the AI *does* set up here when no Unaware is in play, so the
// feature-on leg's restraint can only come from the innate. Pyukumuku's chosen ability is
// forced to Innards Out so Unaware comes solely from the innate; the Wobbuffet backup keeps
// the "only party member won't use a status move" rule from interfering.
AI_SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: AI won't boost stats against an innate-Unaware foe")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_DONDOZO, ABILITY_UNAWARE));
        ASSUME_STAT_CHANGE(MOVE_SWORDS_DANCE, attack: +2);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_DONDOZO) { Ability(ABILITY_OBLIVIOUS); Moves(MOVE_REST); } // bulky, passive wall; Unaware only via the innate
        OPPONENT(SPECIES_LOPUNNY) { Moves(MOVE_SWORDS_DANCE, MOVE_SCRATCH); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SCRATCH); } // backup so the status-move-when-last rule doesn't apply
    } WHEN {
        if (enabled)
            TURN { EXPECT_MOVE(opponent, MOVE_SCRATCH); }      // innate Unaware: boosting is pointless
        else
            TURN { EXPECT_MOVE(opponent, MOVE_SWORDS_DANCE); } // no innate: the AI sets up freely
    }
}

// ───────────────────────────────────────────────────────────────────────────
// Pinch abilities (Overgrow/Blaze/Torrent/Swarm) as innates. The innate is a
// pure-boon DIVERGENCE that LATCHES: once the holder has reached <=1/3 HP this
// battle, the +50% boost sticks for the rest of the battle (so a later heal can't
// strip it), unlike a real pinch ability that only boosts while currently <=1/3.
// Venusaur carries innate Overgrow (its row also lists Natural Cure/Regenerator);
// running it with chosen Chlorophyll isolates the innate as the boost source.
// ───────────────────────────────────────────────────────────────────────────

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: an innate pinch ability boosts at <=1/3 HP", s16 damage)
{
    u32 feature;
    PARAMETRIZE { feature = FALSE; }
    PARAMETRIZE { feature = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_VENUSAUR, ABILITY_OVERGROW));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, feature);
        PLAYER(SPECIES_VENUSAUR) { Ability(ABILITY_CHLOROPHYLL); MaxHP(120); HP(30); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SEED_BOMB); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        // Feature off: chosen Chlorophyll, no Overgrow, no boost. Feature on: innate Overgrow at <=1/3 HP -> 1.5x.
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.5), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: an innate pinch boost LATCHES and persists after healing above 1/3", s16 damage)
{
    u32 feature;
    PARAMETRIZE { feature = FALSE; }
    PARAMETRIZE { feature = TRUE; }
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, feature);
        PLAYER(SPECIES_VENUSAUR) { Ability(ABILITY_CHLOROPHYLL); MaxHP(120); HP(30); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_GROWL); }      // stays at 30 HP; end-of-turn latches the pinch flag
        TURN { MOVE(player, MOVE_SYNTHESIS); }  // heals to ~90 HP, back above 1/3
        TURN { MOVE(player, MOVE_SEED_BOMB); }  // above 1/3 now; a real Overgrow would be off, the latch keeps it
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        // Feature off: no boost above 1/3. Feature on: the latched innate Overgrow still gives 1.5x.
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.5), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate pinch ability", s16 damage)
{
    u32 gastro;
    PARAMETRIZE { gastro = FALSE; }
    PARAMETRIZE { gastro = TRUE; }
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_VENUSAUR) { Ability(ABILITY_CHLOROPHYLL); MaxHP(120); HP(30); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { if (gastro) MOVE(opponent, MOVE_GASTRO_ACID); }
        TURN { MOVE(player, MOVE_SEED_BOMB); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        // Gastro Acid suppresses the innate exactly like a real ability, so the boost is gone.
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.5), results[0].damage);
    }
}

// ───────────────────────── Weather speed-doublers ─────────────────────────
// Swift Swim / Chlorophyll / Sand Rush / Slush Rush: x2 Speed in rain / sun / sandstorm / snow.
// Wired at the single speed-calc site GetBattlerTotalSpeedStat (src/battle_main.c); a 1:1 boon
// (the real abilities have no downside). Sand Rush also shrugs off sandstorm chip damage. Each
// test gives a canon user a *different* chosen ability so the doubling comes purely from the innate.

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Swift Swim doubles Speed in rain")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_LUDICOLO, ABILITY_SWIFT_SWIM));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_LUDICOLO) { Ability(ABILITY_OWN_TEMPO); Speed(100); } // chosen Own Tempo, NOT Swift Swim
        OPPONENT(SPECIES_WOBBUFFET) { Speed(199); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CELEBRATE); MOVE(player, MOVE_RAIN_DANCE); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_RAIN_DANCE, player);
        if (enabled) // innate Swift Swim -> 200 Speed -> player outspeeds 199
        {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
        }
        else // no innate -> 100 Speed -> opponent still faster
        {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
        }
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Chlorophyll doubles Speed in harsh sun")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_LEAFEON, ABILITY_CHLOROPHYLL));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_LEAFEON) { Ability(ABILITY_LEAF_GUARD); Speed(100); } // chosen Leaf Guard, NOT Chlorophyll
        OPPONENT(SPECIES_WOBBUFFET) { Speed(199); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CELEBRATE); MOVE(player, MOVE_SUNNY_DAY); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SUNNY_DAY, player);
        if (enabled)
        {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
        }
        else
        {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
        }
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Sand Rush doubles Speed in a sandstorm")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_EXCADRILL, ABILITY_SAND_RUSH));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_EXCADRILL) { Ability(ABILITY_SAND_FORCE); Speed(100); } // chosen Sand Force, NOT Sand Rush
        OPPONENT(SPECIES_WOBBUFFET) { Speed(199); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CELEBRATE); MOVE(player, MOVE_SANDSTORM); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SANDSTORM, player);
        if (enabled)
        {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
        }
        else
        {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
        }
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Slush Rush doubles Speed in snow")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CETITAN, ABILITY_SLUSH_RUSH));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_CETITAN) { Ability(ABILITY_THICK_FAT); Speed(100); } // chosen Thick Fat, NOT Slush Rush
        OPPONENT(SPECIES_WOBBUFFET) { Speed(199); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CELEBRATE); MOVE(player, MOVE_SNOWSCAPE); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SNOWSCAPE, player);
        if (enabled)
        {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
        }
        else
        {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
        }
    }
}

// ---- Surge Surfer / Grass Pelt (Batch R — terrain modifiers) ----
// The terrain edition of the weather speed-doublers / defensive boosters: both live in shared calcs
// the AI runs, so on-field prediction is innate-aware for free; each is a clean upside (1:1 copy).

// Surge Surfer doubles the holder's Speed on Electric Terrain (GetBattlerTotalSpeedStat, beside the
// weather doublers). Raichu-Alola carries it innately but here runs a different chosen ability, so the
// doubling is attributable solely to the innate. Feature-off leg proves stock behavior (faster foe first).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Surge Surfer doubles Speed on Electric Terrain")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_RAICHU_ALOLA, ABILITY_SURGE_SURFER));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_RAICHU_ALOLA) { Ability(ABILITY_LIGHTNING_ROD); Speed(100); } // chosen ability, NOT Surge Surfer
        OPPONENT(SPECIES_WOBBUFFET) { Speed(199); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CELEBRATE); MOVE(player, MOVE_ELECTRIC_TERRAIN); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ELECTRIC_TERRAIN, player);
        if (enabled)
        {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
        }
        else
        {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
        }
    }
}

// Suppression parity: Gastro Acid nulls the innate Surge Surfer, so the holder no longer outspeeds under
// Electric Terrain (exactly like the real ability, which is not breakable but is suppressible).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Surge Surfer's Speed boost")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_RAICHU_ALOLA, ABILITY_SURGE_SURFER));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(150); }
        OPPONENT(SPECIES_RAICHU_ALOLA) { Ability(ABILITY_LIGHTNING_ROD); Speed(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_ELECTRIC_TERRAIN); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_GASTRO_ACID); MOVE(opponent, MOVE_CELEBRATE); } // suppresses the innate
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Wobbuffet used Gastro Acid!");
        // turn 3: with the innate suppressed, doubled Speed is gone, so the faster Wobbuffet moves first
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
    }
}

// Grass Pelt boosts the holder's Defense by 50% while Grassy Terrain is up (CalcDefenseStat, beside
// Marvel Scale). Gogoat carries it innately but runs a different chosen ability here, so the reduction is
// attributable solely to the innate. Strength (no secondary, not Ground) isolates the Defense boost —
// Grassy Terrain would otherwise halve Ground moves and does not touch Strength.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Grass Pelt boosts Defense 1.5x on Grassy Terrain", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_GOGOAT, ABILITY_GRASS_PELT));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_GOGOAT) { Ability(ABILITY_SAP_SIPPER); } // chosen Sap Sipper, NOT Grass Pelt
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_STRENGTH, MOVE_GRASSY_TERRAIN); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_GRASSY_TERRAIN); MOVE(player, MOVE_CELEBRATE); }
        TURN { MOVE(opponent, MOVE_STRENGTH); MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.5), results[0].damage); // +50% Defense -> ~0.67x damage
    }
}

// Sand Rush's second effect: like the real ability, an innate Sand Rush also shrugs off the
// end-of-turn sandstorm chip damage (a pure boon). Houndstone is Ghost-type, so it is NOT
// naturally exempt by type — the immunity here comes purely from the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Sand Rush prevents sandstorm chip damage")
{
    enum Type type1 = GetSpeciesType(SPECIES_HOUNDSTONE, 0);
    enum Type type2 = GetSpeciesType(SPECIES_HOUNDSTONE, 1);
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_HOUNDSTONE, ABILITY_SAND_RUSH));
        ASSUME(type1 != TYPE_ROCK && type1 != TYPE_GROUND && type1 != TYPE_STEEL);
        ASSUME(type2 != TYPE_ROCK && type2 != TYPE_GROUND && type2 != TYPE_STEEL);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_HOUNDSTONE) { Ability(ABILITY_FLUFFY); } // chosen Fluffy, NOT Sand Rush
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SANDSTORM); }
    } SCENE {
        if (enabled)
            NONE_OF { HP_BAR(player); } // innate Sand Rush -> no sandstorm chip
        else
            HP_BAR(player); // no innate -> takes the chip
    }
}

// ---- Sand Veil / Snow Cloak ----
// Passive calc-modifier evasion abilities: +25% evasion (a 0.8 accuracy modifier on incoming moves)
// while the holder's weather is up — sandstorm for Sand Veil, hail/snow for Snow Cloak — plus immunity
// to that weather's chip damage, applied at GetTotalAccuracy / the end-turn chip sites (src/battle_util.c,
// src/battle_end_turn.c). No script / pop-up / driver. Both are clean upsides, so each innate is a 1:1
// copy. Garchomp (chosen Rough Skin, slot 2) and Articuno (chosen Pressure, slot 0) carry the innate but
// run a different chosen ability, so the evasion is attributable solely to the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Sand Veil ups evasion in a sandstorm")
{
    PASSES_RANDOMLY(4, 5, RNG_ACCURACY); // Pound 100 acc -> 80 with the innate -> hits 4/5
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_GARCHOMP, ABILITY_SAND_VEIL));
        ASSUME(GetMoveAccuracy(MOVE_POUND) == 100);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_GARCHOMP) { Ability(ABILITY_ROUGH_SKIN); Moves(MOVE_CELEBRATE, MOVE_SANDSTORM); } // chosen Rough Skin, NOT Sand Veil
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SANDSTORM); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_POUND); } // player no-ops so it can't KO the attacker first
    } SCENE {
        HP_BAR(player);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: with the feature off, no innate Sand Veil evasion (stock behavior)")
{
    PASSES_RANDOMLY(5, 5, RNG_ACCURACY); // no innate -> Pound always hits
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, FALSE);
        PLAYER(SPECIES_GARCHOMP) { Ability(ABILITY_ROUGH_SKIN); Moves(MOVE_CELEBRATE, MOVE_SANDSTORM); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SANDSTORM); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_POUND); }
    } SCENE {
        HP_BAR(player);
    }
}

// Suppression parity: Sand Veil is breakable, so an attacker's Mold Breaker pierces an innate Sand Veil
// exactly as it would the real ability (the move ignores the evasion and always hits).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker pierces an innate Sand Veil")
{
    PASSES_RANDOMLY(5, 5, RNG_ACCURACY);
    GIVEN {
        ASSUME(gAbilitiesInfo[ABILITY_SAND_VEIL].breakable);
        ASSUME(SpeciesHasInnate(SPECIES_GARCHOMP, ABILITY_SAND_VEIL));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_GARCHOMP) { Ability(ABILITY_ROUGH_SKIN); Moves(MOVE_CELEBRATE, MOVE_SANDSTORM); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MOLD_BREAKER); }
    } WHEN {
        TURN { MOVE(player, MOVE_SANDSTORM); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_POUND); }
    } SCENE {
        HP_BAR(player);
    }
}

// Sand Veil's second effect: like the real ability, an innate Sand Veil also shrugs off the end-of-turn
// sandstorm chip damage (a pure boon). Cacturne is Grass/Dark, so it is NOT naturally exempt by type —
// the immunity here comes purely from the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Sand Veil prevents sandstorm chip damage")
{
    enum Type type1 = GetSpeciesType(SPECIES_CACTURNE, 0);
    enum Type type2 = GetSpeciesType(SPECIES_CACTURNE, 1);
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CACTURNE, ABILITY_SAND_VEIL));
        ASSUME(type1 != TYPE_ROCK && type1 != TYPE_GROUND && type1 != TYPE_STEEL);
        ASSUME(type2 != TYPE_ROCK && type2 != TYPE_GROUND && type2 != TYPE_STEEL);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_CACTURNE) { Ability(ABILITY_WATER_ABSORB); } // chosen Water Absorb, NOT Sand Veil
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SANDSTORM); }
    } SCENE {
        if (enabled)
            NONE_OF { HP_BAR(player); } // innate Sand Veil -> no sandstorm chip
        else
            HP_BAR(player); // no innate -> takes the chip
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Snow Cloak ups evasion in snow")
{
    PASSES_RANDOMLY(4, 5, RNG_ACCURACY); // Pound 100 acc -> 80 with the innate -> hits 4/5
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_ARTICUNO, ABILITY_SNOW_CLOAK));
        ASSUME(GetMoveAccuracy(MOVE_POUND) == 100);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_ARTICUNO) { Ability(ABILITY_PRESSURE); Moves(MOVE_CELEBRATE, MOVE_SNOWSCAPE); } // chosen Pressure, NOT Snow Cloak
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SNOWSCAPE); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_POUND); }
    } SCENE {
        HP_BAR(player);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: with the feature off, no innate Snow Cloak evasion (stock behavior)")
{
    PASSES_RANDOMLY(5, 5, RNG_ACCURACY); // no innate -> Pound always hits
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, FALSE);
        PLAYER(SPECIES_ARTICUNO) { Ability(ABILITY_PRESSURE); Moves(MOVE_CELEBRATE, MOVE_SNOWSCAPE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SNOWSCAPE); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_POUND); }
    } SCENE {
        HP_BAR(player);
    }
}

// Suppression parity: Snow Cloak is breakable, so an attacker's Mold Breaker pierces an innate Snow Cloak.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker pierces an innate Snow Cloak")
{
    PASSES_RANDOMLY(5, 5, RNG_ACCURACY);
    GIVEN {
        ASSUME(gAbilitiesInfo[ABILITY_SNOW_CLOAK].breakable);
        ASSUME(SpeciesHasInnate(SPECIES_ARTICUNO, ABILITY_SNOW_CLOAK));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_ARTICUNO) { Ability(ABILITY_PRESSURE); Moves(MOVE_CELEBRATE, MOVE_SNOWSCAPE); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MOLD_BREAKER); }
    } WHEN {
        TURN { MOVE(player, MOVE_SNOWSCAPE); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_POUND); }
    } SCENE {
        HP_BAR(player);
    }
}

// Under DETERMINISTIC_ACCURACY_EVASION (the romhack's shipping default) accuracy/evasion stop
// deciding hit/miss and instead drive a PP economy — so the innate Sand Veil evasion manifests as a
// +1 PP tax on incoming offensive moves while a sandstorm is up, exactly like a real Sand Veil /
// BrightPowder (the chosen-ability case is the BrightPowder test in deterministic_accuracy_evasion.c).
// This exercises the GetDeterministicMoveTargetPPTax half of the wiring (the RNG-accuracy tests above
// exercise the GetTotalAccuracy half). DETERMINISTIC_ABILITIES is irrelevant here: Sand Veil / Snow
// Cloak have no chance-based effect to make deterministic.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: under DETERMINISTIC_ACCURACY_EVASION an innate Sand Veil taxes the attacker's PP")
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; } // no innate -> base 1 PP only
    PARAMETRIZE { enabled = TRUE; }  // innate Sand Veil in sand -> +1 PP
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_GARCHOMP, ABILITY_SAND_VEIL));
        ASSUME(GetMovePP(MOVE_POUND) == 35);
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_POUND, MOVE_SANDSTORM); }
        OPPONENT(SPECIES_GARCHOMP) { Ability(ABILITY_ROUGH_SKIN); Moves(MOVE_CELEBRATE); } // chosen Rough Skin, NOT Sand Veil
    } WHEN {
        TURN { MOVE(player, MOVE_SANDSTORM); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_POUND); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->pp[0], enabled ? 33 : 34); // 35 - 1 base [- 1 innate Sand Veil tax]
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Swift Swim")
{
    bool32 gastro;
    PARAMETRIZE { gastro = FALSE; }
    PARAMETRIZE { gastro = TRUE; }
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_LUDICOLO) { Ability(ABILITY_OWN_TEMPO); Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(199); }
    } WHEN {
        TURN { MOVE(player, MOVE_RAIN_DANCE); if (gastro) MOVE(opponent, MOVE_GASTRO_ACID); else MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        if (gastro) // innate suppressed -> 100 Speed -> opponent faster
        {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
        }
        else // innate active in rain -> 200 Speed -> player faster
        {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
        }
    }
}

// ---- Filter ----
// A pure calc-modifier passive: the holder takes 25% less damage from supereffective moves,
// applied at the GetDefenderAbilitiesModifier site (src/battle_util.c). No script / pop-up /
// driver, like Unaware. Mr. Mime is a canon Filter user whose slot-0 ability is Soundproof, so
// with the feature off it runs Soundproof (which never touches Poison Jab) and the reduction is
// attributable solely to the innate. Mr. Mime is Psychic/Fairy, so Poison Jab is supereffective.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Filter reduces supereffective damage by 0.75", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MR_MIME, ABILITY_FILTER));
        ASSUME(gSpeciesInfo[SPECIES_MR_MIME].abilities[0] != ABILITY_FILTER);
        ASSUME(GetMoveType(MOVE_POISON_JAB) == TYPE_POISON);
        ASSUME(gTypeEffectivenessTable[TYPE_POISON][TYPE_FAIRY] > UQ_4_12(1.0));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_MR_MIME);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_POISON_JAB); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
        MESSAGE("It's super effective!");
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.75), results[1].damage); // off: full; on: 0.75x
    }
}

// Filter only touches supereffective moves: a neutral hit is unchanged whether or not the
// innate is active.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Filter does not reduce non-supereffective damage", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MR_MIME, ABILITY_FILTER));
        ASSUME(GetMoveType(MOVE_TACKLE) == TYPE_NORMAL);
        ASSUME(gTypeEffectivenessTable[TYPE_NORMAL][TYPE_PSYCHIC] == UQ_4_12(1.0));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_MR_MIME);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_TACKLE); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_EQ(results[0].damage, results[1].damage); // neutral hit: Filter does nothing
    }
}

// Suppression parity: Filter is breakable, so an attacker's Mold Breaker pierces an innate Filter
// exactly as it would the real ability.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker pierces an innate Filter", s16 damage)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_PRESSURE; }     // innate Filter applies -> reduced
    PARAMETRIZE { ability = ABILITY_MOLD_BREAKER; } // pierces -> full damage
    GIVEN {
        ASSUME(gAbilitiesInfo[ABILITY_FILTER].breakable);
        ASSUME(SpeciesHasInnate(SPECIES_MR_MIME, ABILITY_FILTER));
        ASSUME(gTypeEffectivenessTable[TYPE_POISON][TYPE_FAIRY] > UQ_4_12(1.0));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_MR_MIME);
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ability); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_POISON_JAB); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
        MESSAGE("It's super effective!");
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(0.75), results[0].damage); // Mold Breaker full; Filter 0.75x
    }
}

// Gastro Acid suppresses an innate Filter (suppression parity), so the supereffective hit lands
// at full power once the innate is off.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Filter", s16 damage)
{
    bool32 gastro;
    PARAMETRIZE { gastro = FALSE; }
    PARAMETRIZE { gastro = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MR_MIME, ABILITY_FILTER));
        ASSUME(gTypeEffectivenessTable[TYPE_POISON][TYPE_FAIRY] > UQ_4_12(1.0));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_MR_MIME);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_POISON_JAB, MOVE_GASTRO_ACID, MOVE_CELEBRATE); }
    } WHEN {
        TURN { if (gastro) MOVE(opponent, MOVE_GASTRO_ACID); else MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(opponent, MOVE_POISON_JAB); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_POISON_JAB, opponent);
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(0.75), results[0].damage); // suppressed: full; active: 0.75x
    }
}

// ---- Thick Fat ----
// A pure calc-modifier passive: the holder takes half damage from Fire- and Ice-type moves, applied
// at the CalcAttackStat "target's abilities" site (src/battle_util.c). No script / pop-up / driver,
// like Filter. Snorlax is a canon Thick Fat user whose slot-0 ability is Immunity, so with the
// feature off it runs Immunity (which never touches Fire damage) and the reduction is attributable
// solely to the innate. Snorlax is Normal, so Flamethrower is a neutral hit — Thick Fat halves it
// regardless of effectiveness, isolating the ability from any type interaction.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Thick Fat halves Fire damage", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SNORLAX, ABILITY_THICK_FAT));
        ASSUME(gSpeciesInfo[SPECIES_SNORLAX].abilities[0] != ABILITY_THICK_FAT);
        ASSUME(GetMoveType(MOVE_FLAMETHROWER) == TYPE_FIRE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_SNORLAX);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_FLAMETHROWER); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.5), results[1].damage); // off: full; on: 0.5x
    }
}

// Thick Fat also halves Ice-type damage (the other half of the ability).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Thick Fat halves Ice damage", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SNORLAX, ABILITY_THICK_FAT));
        ASSUME(GetMoveType(MOVE_ICE_BEAM) == TYPE_ICE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_SNORLAX);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_ICE_BEAM); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.5), results[1].damage); // off: full; on: 0.5x
    }
}

// Thick Fat only touches Fire/Ice moves: a Normal hit is unchanged whether or not the innate is active.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Thick Fat does not reduce non-Fire/Ice damage", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SNORLAX, ABILITY_THICK_FAT));
        ASSUME(GetMoveType(MOVE_TACKLE) == TYPE_NORMAL);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_SNORLAX);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_TACKLE); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_EQ(results[0].damage, results[1].damage); // Normal hit: Thick Fat does nothing
    }
}

// Suppression parity: Thick Fat is breakable, so an attacker's Mold Breaker pierces an innate Thick Fat
// exactly as it would the real ability.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker pierces an innate Thick Fat", s16 damage)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_PRESSURE; }     // innate Thick Fat applies -> halved
    PARAMETRIZE { ability = ABILITY_MOLD_BREAKER; } // pierces -> full damage
    GIVEN {
        ASSUME(gAbilitiesInfo[ABILITY_THICK_FAT].breakable);
        ASSUME(SpeciesHasInnate(SPECIES_SNORLAX, ABILITY_THICK_FAT));
        ASSUME(GetMoveType(MOVE_FLAMETHROWER) == TYPE_FIRE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_SNORLAX);
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ability); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_FLAMETHROWER); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(0.5), results[0].damage); // Mold Breaker full; Thick Fat 0.5x
    }
}

// Gastro Acid suppresses an innate Thick Fat (suppression parity), so the Fire hit lands at full
// power once the innate is off.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Thick Fat", s16 damage)
{
    bool32 gastro;
    PARAMETRIZE { gastro = FALSE; }
    PARAMETRIZE { gastro = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SNORLAX, ABILITY_THICK_FAT));
        ASSUME(GetMoveType(MOVE_FLAMETHROWER) == TYPE_FIRE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_SNORLAX);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_FLAMETHROWER, MOVE_GASTRO_ACID, MOVE_CELEBRATE); }
    } WHEN {
        TURN { if (gastro) MOVE(opponent, MOVE_GASTRO_ACID); else MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(opponent, MOVE_FLAMETHROWER); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_FLAMETHROWER, opponent);
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(0.5), results[0].damage); // suppressed: full; active: 0.5x
    }
}

// ---- Technician ----
// A pure calc-modifier passive: the holder's moves of base power 60 or less get +50%, applied at the
// CalcMoveBasePowerAfterModifiers attacker-abilities site (src/battle_util.c). No script / pop-up /
// driver, like Filter / Thick Fat. Persian is a canon Technician user whose slot-0 ability is Limber,
// so with the feature off it runs Limber (which never touches damage) and the boost is attributable
// solely to the innate. Swift is a 60-BP, never-miss Normal move, so it sits exactly on the <= 60
// boundary and gets the boost on both branches' identical type interaction.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Technician boosts a base-power-60 move", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_PERSIAN, ABILITY_TECHNICIAN));
        ASSUME(gSpeciesInfo[SPECIES_PERSIAN].abilities[0] != ABILITY_TECHNICIAN);
        ASSUME(GetMovePower(MOVE_SWIFT) == 60);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_PERSIAN) { Moves(MOVE_SWIFT); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SWIFT); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.5), results[1].damage); // off: base; on: 1.5x
    }
}

// Technician only touches moves of base power <= 60: an 85-BP Body Slam is unchanged whether or not
// the innate is active.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Technician does not boost a move over base power 60", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_PERSIAN, ABILITY_TECHNICIAN));
        ASSUME(GetMovePower(MOVE_BODY_SLAM) > 60);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_PERSIAN) { Moves(MOVE_BODY_SLAM); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_BODY_SLAM); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_EQ(results[0].damage, results[1].damage); // >60 BP: Technician does nothing
    }
}

// Gastro Acid suppresses an innate Technician (suppression parity), so the 60-BP move lands at base
// power once the innate is off. Technician is NOT breakable, so Mold Breaker can't pierce it — Gastro
// Acid is the relevant suppressor, same as the real ability.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Technician", s16 damage)
{
    bool32 gastro;
    PARAMETRIZE { gastro = FALSE; }
    PARAMETRIZE { gastro = TRUE; }
    GIVEN {
        ASSUME(!gAbilitiesInfo[ABILITY_TECHNICIAN].breakable);
        ASSUME(SpeciesHasInnate(SPECIES_PERSIAN, ABILITY_TECHNICIAN));
        ASSUME(GetMovePower(MOVE_SWIFT) == 60);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_PERSIAN) { Moves(MOVE_SWIFT); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID, MOVE_CELEBRATE); }
    } WHEN {
        TURN { if (gastro) MOVE(opponent, MOVE_GASTRO_ACID); else MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_SWIFT); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.5), results[0].damage); // suppressed: base; active: 1.5x
    }
}

// ===== Pressure =====
// Pressure makes the holder's foes spend 1 extra PP per move used against it. It's wired innate-
// aware at the two PP-deduction sites (CancelerPPDeduction in src/battle_move_resolution.c and the
// deterministic PP-refund mirror in src/battle_util.c). Aerodactyl is the test vehicle: its chosen
// ability is Rock Head (slot 0), so the extra PP tax can only come from its innate Pressure. With the
// feature off the innate is inert and the foe loses the usual single PP.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Pressure makes the foe spend 1 extra PP")
{
    u32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_AERODACTYL, ABILITY_PRESSURE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET) { MovesWithPP({MOVE_POUND, 35}); }
        OPPONENT(SPECIES_AERODACTYL) { Ability(ABILITY_ROCK_HEAD); } // chosen ability is not Pressure
    } WHEN {
        TURN { MOVE(player, MOVE_POUND); }
    } THEN {
        EXPECT_EQ(player->pp[0], enabled ? 33 : 34); // innate Pressure: -2 PP; feature off: -1 PP
    }
}

// Suppression parity: Gastro Acid nullifies an innate Pressure exactly like a real one, so the foe's
// PP loss drops back to the usual single PP once the innate is suppressed.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Pressure")
{
    u32 gastro;
    PARAMETRIZE { gastro = FALSE; }
    PARAMETRIZE { gastro = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_AERODACTYL, ABILITY_PRESSURE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID, MOVE_CELEBRATE, MOVE_POUND); }
        OPPONENT(SPECIES_AERODACTYL) { Ability(ABILITY_ROCK_HEAD); }
    } WHEN {
        TURN { if (gastro) MOVE(player, MOVE_GASTRO_ACID); else MOVE(player, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_POUND); }
    } THEN {
        EXPECT_EQ(player->pp[2], gastro ? 34 : 33); // suppressed: -1 PP; active: -2 PP (Pound is move index 2)
    }
}

// A canon Pressure user (Mewtwo: Pressure / - / Unnerve) carries Pressure innately too, as a combined
// INNATES(Levitate, Pressure) row. With its chosen ability set to Unnerve, the extra PP tax can only
// come from the innate Pressure — confirming the merged list still honors it.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: a canon Pressure user keeps it via innate when the chosen ability differs")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MEWTWO, ABILITY_PRESSURE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { MovesWithPP({MOVE_POUND, 35}); }
        OPPONENT(SPECIES_MEWTWO) { Ability(ABILITY_UNNERVE); }
    } WHEN {
        TURN { MOVE(player, MOVE_POUND); }
    } THEN {
        EXPECT_EQ(player->pp[0], 33); // innate Pressure taxes the extra PP even though chosen is Unnerve
    }
}

// ===== Stench =====
// Stench gives the holder's damaging moves a 10% chance to flinch the target (under
// DETERMINISTIC_ABILITIES, a guaranteed first-turn flinch). Wired innate-aware at the
// ABILITYEFFECT_MOVE_END_ATTACKER on-hit site (src/battle_util.c): the chosen-ability dispatch keys
// off gLastUsedAbility, so an innate Stench is run additively in a pre-check beside the switch. A
// clean upside (it only ever flinches the FOE), so the innate is a 1:1 copy of the real ability.
// Gulpin is the flavor vehicle: a foul poison-gas bag with no native Stench (Liquid Ooze / Sticky
// Hold / Gluttony), so any flinch can only come from the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Stench has a 10% chance to flinch")
{
    PASSES_RANDOMLY(1, 10, RNG_STENCH);
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_GULPIN, ABILITY_STENCH));
        ASSUME(gSpeciesInfo[SPECIES_GULPIN].abilities[0] != ABILITY_STENCH);
        ASSUME(GetMovePower(MOVE_SCRATCH) > 0);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_GULPIN) { Speed(100); } // chosen Liquid Ooze; Stench only as innate
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!");
    }
}

// Feature-off leg: the same forced Stench roll produces no flinch, proving the flinch comes only
// from the innate (stock behavior: Liquid Ooze never flinches).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: with the feature off, no innate Stench flinch (stock behavior)")
{
    GIVEN {
        ASSUME(gSpeciesInfo[SPECIES_GULPIN].abilities[0] != ABILITY_STENCH);
        ASSUME(GetMovePower(MOVE_SCRATCH) > 0);
        // Feature is off by default in the test baseline.
        PLAYER(SPECIES_GULPIN) { Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH, WITH_RNG(RNG_STENCH, TRUE)); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        NONE_OF { MESSAGE("The opposing Wobbuffet flinched and couldn't move!"); }
    }
}

// A canon Stench user (Muk: Stench / Sticky Hold / Poison Touch) carries Stench innately too, so it
// keeps the signature flinch no matter which slot the build picks. Chosen Sticky Hold isolates the
// innate; the forced roll lands the flinch even though Stench is not the chosen ability.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: a canon Stench user keeps it via innate when the chosen ability differs")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MUK, ABILITY_STENCH));
        ASSUME(GetMovePower(MOVE_SCRATCH) > 0);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_MUK) { Ability(ABILITY_STICKY_HOLD); Speed(100); } // chosen ability is NOT Stench
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH, WITH_RNG(RNG_STENCH, TRUE)); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!");
    }
}

// Suppression parity: Gastro Acid nullifies an innate Stench exactly like a real ability, so the
// forced roll produces no flinch once the innate is suppressed.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Stench")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_GULPIN, ABILITY_STENCH));
        ASSUME(GetMovePower(MOVE_SCRATCH) > 0);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_GULPIN) { Speed(100); Moves(MOVE_SCRATCH, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_GASTRO_ACID, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_GASTRO_ACID); } // innate suppressed
        TURN { MOVE(player, MOVE_SCRATCH, WITH_RNG(RNG_STENCH, TRUE)); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        NONE_OF { MESSAGE("The opposing Wobbuffet flinched and couldn't move!"); } // suppressed -> no flinch
    }
}

// The innate runs through the same shared flinch path as the real ability, so the existing blockers
// still apply with no extra wiring: Shield Dust stops the added flinch effect even with the roll forced.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: an innate Stench flinch is blocked by Shield Dust")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_GULPIN, ABILITY_STENCH));
        ASSUME(GetMovePower(MOVE_SCRATCH) > 0);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_GULPIN) { Speed(100); }
        OPPONENT(SPECIES_VIVILLON) { Ability(ABILITY_SHIELD_DUST); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH, WITH_RNG(RNG_STENCH, TRUE)); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        NONE_OF { MESSAGE("The opposing Vivillon flinched and couldn't move!"); }
    }
}

// The fork ships with DETERMINISTIC_ABILITIES on (config/deterministic.h), so the *primary* behavior
// of a Stench holder is a guaranteed first-turn flinch (no RNG), mirroring a King's Rock entry flinch.
// An innate Stench honors that exactly like the real ability — it runs through the same TryStenchFlinch
// helper and its DETERMINISTIC_ABILITIES branch. Both feature flags are opt-in here (the baseline forces
// each off); chosen Sticky Hold isolates the innate. Companion to the chosen-ability deterministic Stench
// test in test/fork/deterministic_abilities.c.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES + DETERMINISTIC_ABILITIES: innate Stench flinches on the first turn but not after")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MUK, ABILITY_STENCH));
        ASSUME(GetMovePower(MOVE_SCRATCH) > 0);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        PLAYER(SPECIES_MUK) { Ability(ABILITY_STICKY_HOLD); Speed(50); } // chosen ability is NOT Stench
        OPPONENT(SPECIES_WOBBUFFET) { Speed(10); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_SCRATCH); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!"); // turn 1 only
        NONE_OF { MESSAGE("The opposing Wobbuffet flinched and couldn't move!"); }
    }
}

// "First turn" is per-entry, not per-battle: isFirstTurn resets to 2 on every switch-in (not just at
// battle start), and there's no once-per-battle latch, so a Stench holder that switches out and back in
// flinches again on each fresh entry turn — exactly like a King's Rock entry flinch. The innate reads
// the live battler state (IsBattlersFirstTurn(gBattlerAttacker)), so it re-triggers on re-entry too.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES + DETERMINISTIC_ABILITIES: innate Stench flinches again on each re-entry")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MUK, ABILITY_STENCH));
        ASSUME(GetMovePower(MOVE_SCRATCH) > 0);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        PLAYER(SPECIES_MUK) { Ability(ABILITY_STICKY_HOLD); Speed(50); } // chosen ability is NOT Stench
        PLAYER(SPECIES_WYNAUT) { Speed(50); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(10); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); MOVE(opponent, MOVE_CELEBRATE); } // entry turn: flinch
        TURN { SWITCH(player, 1); }                                         // Muk leaves the field
        TURN { SWITCH(player, 0); }                                         // Muk returns (fresh entry)
        TURN { MOVE(player, MOVE_SCRATCH); MOVE(opponent, MOVE_CELEBRATE); } // re-entry turn: flinch again
    } SCENE {
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!"); // turn 1 (first entry)
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!"); // turn 4 (re-entry)
    }
}

// Stench pre-empts its holder's own flinch item so the two don't stack (and the item isn't
// redundantly consumed). That guard in TryKingsRock (src/battle_hold_effects.c) reads the attacker's
// ability, so it must credit an innate Stench too — otherwise an innate holder's King's Rock would
// fire/consume where a chosen-Stench holder's bows out. To isolate the guard, Stench's own roll is
// forced to MISS (so it sets no flinch volatile that would also stop the rock) while King's Rock is a
// guaranteed deterministic entry flinch: with the innate-aware guard the rock still bows out, so there
// is no flinch and the item is retained; without it the rock would flinch and be consumed.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: an innate Stench holder's King's Rock bows out (no stacking, not consumed)")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MUK, ABILITY_STENCH));
        ASSUME(gItemsInfo[ITEM_KINGS_ROCK].holdEffect == HOLD_EFFECT_FLINCH);
        ASSUME(GetMovePower(MOVE_SCRATCH) > 0);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE); // King's Rock = guaranteed first-turn flinch
        // DETERMINISTIC_ABILITIES off (baseline): Stench takes its RNG path, forced to miss below.
        PLAYER(SPECIES_MUK) { Ability(ABILITY_STICKY_HOLD); Item(ITEM_KINGS_ROCK); Speed(50); } // chosen ability is NOT Stench
        OPPONENT(SPECIES_WOBBUFFET) { Speed(10); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH, WITH_RNG(RNG_STENCH, FALSE)); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        NONE_OF { MESSAGE("The opposing Wobbuffet flinched and couldn't move!"); } // rock suppressed by innate Stench
    } THEN {
        EXPECT_EQ(player->item, ITEM_KINGS_ROCK); // King's Rock left untouched
    }
}

// Battle Armor and Shell Armor both block critical hits. They are a clean upside (they never hurt the
// holder), so each innate is a 1:1 copy of the real ability. Storm Throw always crits, so this is an
// RNG-free proof: a Kingler whose chosen ability is Hyper Cutter (not Shell Armor) still shrugs off the
// guaranteed crit via its innate Shell Armor; with the feature off the always-crit move crits.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Shell Armor blocks a guaranteed critical hit")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(MoveAlwaysCrits(MOVE_STORM_THROW));
        ASSUME(SpeciesHasInnate(SPECIES_KINGLER, ABILITY_SHELL_ARMOR));
        ASSUME(gSpeciesInfo[SPECIES_KINGLER].abilities[0] != ABILITY_SHELL_ARMOR);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_KINGLER) { Ability(ABILITY_HYPER_CUTTER); } // chosen ability is NOT Shell Armor
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_STORM_THROW); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_STORM_THROW, opponent);
        if (enabled)
            NONE_OF { MESSAGE("A critical hit!"); } // innate Shell Armor blocks the crit
        else
            MESSAGE("A critical hit!");              // no innate -> the always-crit move crits
    }
}

// The same effect via a Battle Armor user whose chosen ability differs (Cubone's Rock Head, not Battle
// Armor) — both crit-immunity abilities are wired, so the innate honors either constant.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Battle Armor blocks a guaranteed critical hit")
{
    GIVEN {
        ASSUME(MoveAlwaysCrits(MOVE_STORM_THROW));
        ASSUME(SpeciesHasInnate(SPECIES_CUBONE, ABILITY_BATTLE_ARMOR));
        ASSUME(gSpeciesInfo[SPECIES_CUBONE].abilities[0] != ABILITY_BATTLE_ARMOR);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_CUBONE) { Ability(ABILITY_ROCK_HEAD); } // chosen ability is NOT Battle Armor
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_STORM_THROW); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_STORM_THROW, opponent);
        NONE_OF { MESSAGE("A critical hit!"); } // innate Battle Armor blocks the crit
    }
}

// Suppression parity: Battle/Shell Armor are breakable, so an attacker's Mold Breaker pierces an innate
// one exactly as it would the real ability (GetBattlerAbility() and IsInnateActive() are both broken
// through) — the guaranteed crit lands.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker pierces an innate Shell Armor")
{
    GIVEN {
        ASSUME(MoveAlwaysCrits(MOVE_STORM_THROW));
        ASSUME(gAbilitiesInfo[ABILITY_SHELL_ARMOR].breakable);
        ASSUME(SpeciesHasInnate(SPECIES_KINGLER, ABILITY_SHELL_ARMOR));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_KINGLER) { Ability(ABILITY_HYPER_CUTTER); } // innate Shell Armor
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MOLD_BREAKER); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_STORM_THROW); }
    } SCENE {
        MESSAGE("A critical hit!"); // Mold Breaker ignores the innate -> the crit lands
    }
}

// Suppression parity: Gastro Acid blanks the innate just like it blanks the real ability, so the
// guaranteed crit lands once the holder's innate is suppressed.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Shell Armor")
{
    GIVEN {
        ASSUME(MoveAlwaysCrits(MOVE_STORM_THROW));
        ASSUME(SpeciesHasInnate(SPECIES_KINGLER, ABILITY_SHELL_ARMOR));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_KINGLER) { Ability(ABILITY_HYPER_CUTTER); Moves(MOVE_CELEBRATE); } // innate Shell Armor
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID, MOVE_STORM_THROW); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_GASTRO_ACID); MOVE(player, MOVE_CELEBRATE); }
        TURN { MOVE(opponent, MOVE_STORM_THROW); MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("A critical hit!"); // innate suppressed by Gastro Acid -> the crit lands
    }
}

// Speed Boost is the fork's first ACTIVE, scripted end-turn innate: a canon Speed Boost user whose
// chosen ability differs (Sharpedo's Rough Skin, not Speed Boost) still raises its Speed +1 at the end
// of every turn via the innate. The pop-up is overridden to show Speed Boost (not the chosen ability).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Speed Boost raises Speed at the end of the turn")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SHARPEDO, ABILITY_SPEED_BOOST));
        ASSUME(gSpeciesInfo[SPECIES_SHARPEDO].abilities[0] != ABILITY_SPEED_BOOST);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_SHARPEDO) { Ability(ABILITY_ROUGH_SKIN); } // chosen ability is NOT Speed Boost
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_SPEED_BOOST); // pop-up shows the innate, not Rough Skin
        MESSAGE("Sharpedo's Speed rose!");
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE + 1);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: with the feature off, no innate Speed Boost (stock behavior)")
{
    GIVEN {
        // Feature off by default in the baseline; Sharpedo carries no native Speed Boost in slot 0.
        ASSUME(gSpeciesInfo[SPECIES_SHARPEDO].abilities[0] != ABILITY_SPEED_BOOST);
        PLAYER(SPECIES_SHARPEDO) { Ability(ABILITY_ROUGH_SKIN); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(player, ABILITY_SPEED_BOOST); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE); // no boost without the feature
    }
}

// Suppression parity: Gastro Acid blanks the innate just like the real ability, so no end-turn boost.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Speed Boost")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SHARPEDO, ABILITY_SPEED_BOOST));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_SHARPEDO) { Ability(ABILITY_ROUGH_SKIN); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_GASTRO_ACID); MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Sharpedo's Ability was suppressed!");
        NONE_OF { ABILITY_POPUP(player, ABILITY_SPEED_BOOST); } // suppressed -> no boost
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE);
    }
}

// A real Speed Boost user with the feature ON must NOT boost twice (chosen + innate). The driver skips
// an innate that equals the chosen ability, so a real Speed Boost mon still raises Speed exactly +1/turn.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: a real Speed Boost does not double up with the innate")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_TORCHIC, ABILITY_SPEED_BOOST));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_TORCHIC) { Ability(ABILITY_SPEED_BOOST); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_SPEED_BOOST);
        MESSAGE("Torchic's Speed rose!");
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE + 1); // exactly one boost, not two
    }
}

// Re-entrancy: the end-turn driver resumes from a per-battler cursor and resets it each turn, so an
// innate Speed Boost fires again on the next turn — Speed climbs +1 per turn over consecutive turns.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Speed Boost fires every turn (cursor resets per turn)")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SHARPEDO, ABILITY_SPEED_BOOST));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_SHARPEDO) { Ability(ABILITY_ROUGH_SKIN); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_SPEED_BOOST);
        MESSAGE("Sharpedo's Speed rose!");
        ABILITY_POPUP(player, ABILITY_SPEED_BOOST);
        MESSAGE("Sharpedo's Speed rose!");
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE + 2); // +1 each of the two turns
    }
}

// The driver scans the whole innate list and skips innates that aren't active end-turn effects:
// Scolipede carries INNATES(Speed Boost, Swarm), and the non-end-turn Swarm is stepped over so the
// end-turn Speed Boost still fires. (Also exercises the cursor advancing across a multi-innate list.)
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Speed Boost fires past a non-end-turn innate in the list")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SCOLIPEDE, ABILITY_SPEED_BOOST));
        ASSUME(SpeciesHasInnate(SPECIES_SCOLIPEDE, ABILITY_SWARM)); // a passive innate sharing the list
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_SCOLIPEDE) { Ability(ABILITY_POISON_POINT); } // chosen ability is neither innate
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_SPEED_BOOST);
        MESSAGE("Scolipede's Speed rose!");
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE + 1);
    }
}

// ----- Limber (paralysis immunity) -----
// A flavor Seviper (Shed Skin / Infiltrator — no native Limber) cannot be paralyzed via its innate
// Limber, and the pop-up shows Limber, not the chosen Infiltrator. With the feature off it's stock
// behavior, so Thunder Wave paralyzes normally.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Limber prevents paralysis")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_THUNDER_WAVE) == EFFECT_NON_VOLATILE_STATUS);
        ASSUME(GetMoveNonVolatileStatus(MOVE_THUNDER_WAVE) == MOVE_EFFECT_PARALYSIS);
        ASSUME(SpeciesHasInnate(SPECIES_SEVIPER, ABILITY_LIMBER));
        ASSUME(gSpeciesInfo[SPECIES_SEVIPER].abilities[0] != ABILITY_LIMBER);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_SEVIPER) { Ability(ABILITY_INFILTRATOR); } // chosen ability differs from the innate
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_THUNDER_WAVE); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_LIMBER); // pop-up shows Limber, not the chosen Infiltrator
            MESSAGE("It doesn't affect Seviper…");
            NONE_OF {
                ANIMATION(ANIM_TYPE_STATUS, B_ANIM_STATUS_PRZ, player);
                STATUS_ICON(player, paralysis: TRUE);
            }
        } else {
            STATUS_ICON(player, paralysis: TRUE); // no innate -> Thunder Wave paralyzes
        }
    }
}

// Suppression parity: Limber is breakable, so an attacker's Mold Breaker pierces an innate Limber
// exactly as it would the real ability — the paralysis lands and no Limber pop-up shows.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker pierces an innate Limber")
{
    GIVEN {
        ASSUME(gAbilitiesInfo[ABILITY_LIMBER].breakable);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_SEVIPER) { Ability(ABILITY_INFILTRATOR); } // innate Limber
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MOLD_BREAKER); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_THUNDER_WAVE); }
    } SCENE {
        STATUS_ICON(player, paralysis: TRUE); // Mold Breaker ignores the innate -> paralyzed
        NONE_OF { ABILITY_POPUP(player, ABILITY_LIMBER); }
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Limber")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_SEVIPER) { Ability(ABILITY_INFILTRATOR); Moves(MOVE_CELEBRATE); } // innate Limber
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID, MOVE_THUNDER_WAVE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_GASTRO_ACID); } // suppresses the innate
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_THUNDER_WAVE); }
    } SCENE {
        MESSAGE("Seviper's Ability was suppressed!");
        STATUS_ICON(player, paralysis: TRUE); // suppressed -> Thunder Wave paralyzes
    }
}

// A canon Limber user (Persian: Limber/Technician/Unnerve) whose *chosen* ability is Technician still
// carries Limber as an innate, so it keeps the para-immunity no matter which slot the build picks.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: a canon Limber user keeps it via innate when the chosen ability differs")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_PERSIAN, ABILITY_LIMBER));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_PERSIAN) { Ability(ABILITY_TECHNICIAN); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_THUNDER_WAVE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_LIMBER);
        MESSAGE("It doesn't affect Persian…");
        NONE_OF { STATUS_ICON(player, paralysis: TRUE); }
    }
}

// The switch-in cure site: a mon paralyzed while its innate Limber was suppressed (Gastro Acid) gets
// the paralysis cured the moment it switches back in (Gastro Acid clears on switch-out), exactly like a
// real Limber. The pop-up shows Limber.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Limber cures pre-existing paralysis on switch-in")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_SEVIPER) { Ability(ABILITY_INFILTRATOR); Moves(MOVE_CELEBRATE); } // innate Limber
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID, MOVE_THUNDER_WAVE, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_GASTRO_ACID); }   // suppress the innate
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_THUNDER_WAVE); }  // Seviper paralyzed
        TURN { SWITCH(player, 1); MOVE(opponent, MOVE_CELEBRATE); }                // Seviper out -> Gastro clears
        TURN { SWITCH(player, 0); MOVE(opponent, MOVE_CELEBRATE); }                // Seviper back in -> innate cures
    } SCENE {
        ABILITY_POPUP(player, ABILITY_LIMBER);
        MESSAGE("Seviper was cured of paralysis!");
    } THEN {
        EXPECT_EQ(player->status1 & STATUS1_PARALYSIS, 0);
    }
}

// ===== Cute Charm =====
// Cute Charm gives a 30% chance (Gen 4+) to infatuate an opposite-gender attacker that lands a
// contact move on the holder. Wired innate-aware at the ABILITYEFFECT_MOVE_END on-hit site
// (src/battle_util.c): the chosen-ability dispatch keys off the target's gLastUsedAbility, so an
// innate Cute Charm is run additively in a pre-check beside the switch (TryCuteCharmInfatuate), and
// the pop-up is overwritten to Cute Charm. A clean upside (it only ever infatuates the FOE), so the
// innate is a 1:1 copy of the real ability. Milotic is the vehicle: it carries Cute Charm as its
// Hidden Ability, so with a chosen Marvel Scale the infatuation can only come from the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Cute Charm infatuates an opposite-gender attacker on contact")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MILOTIC, ABILITY_CUTE_CHARM));
        ASSUME(gSpeciesInfo[SPECIES_MILOTIC].abilities[0] != ABILITY_CUTE_CHARM);
        ASSUME(MoveMakesContact(MOVE_SCRATCH));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Gender(MON_MALE); Speed(100); }
        OPPONENT(SPECIES_MILOTIC) { Gender(MON_FEMALE); Ability(ABILITY_MARVEL_SCALE); Speed(50); } // chosen ability is NOT Cute Charm
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH, WITH_RNG(RNG_CUTE_CHARM, TRUE)); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_CUTE_CHARM); // pop-up shows Cute Charm, not the chosen Marvel Scale
        MESSAGE("Wobbuffet fell in love!");
    }
}

// Feature-off leg: the same forced Cute Charm roll produces no infatuation, proving it comes only
// from the innate (stock behavior: a Marvel Scale Milotic never infatuates).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: with the feature off, no innate Cute Charm infatuation (stock behavior)")
{
    GIVEN {
        ASSUME(gSpeciesInfo[SPECIES_MILOTIC].abilities[0] != ABILITY_CUTE_CHARM);
        ASSUME(MoveMakesContact(MOVE_SCRATCH));
        // Feature is off by default in the test baseline.
        PLAYER(SPECIES_WOBBUFFET) { Gender(MON_MALE); Speed(100); }
        OPPONENT(SPECIES_MILOTIC) { Gender(MON_FEMALE); Ability(ABILITY_MARVEL_SCALE); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH, WITH_RNG(RNG_CUTE_CHARM, TRUE)); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        NONE_OF {
            ABILITY_POPUP(opponent, ABILITY_CUTE_CHARM);
            MESSAGE("Wobbuffet fell in love!");
        }
    }
}

// A non-contact move never triggers Cute Charm, innate included (parity with the real ability): the
// infatuation routes through CanBattlerAvoidContactEffects, so Swift (no contact) leaves the attacker
// uninfatuated even with the roll forced.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Cute Charm does not trigger on a non-contact move")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MILOTIC, ABILITY_CUTE_CHARM));
        ASSUME(!MoveMakesContact(MOVE_SWIFT));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Gender(MON_MALE); Speed(100); }
        OPPONENT(SPECIES_MILOTIC) { Gender(MON_FEMALE); Ability(ABILITY_MARVEL_SCALE); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_SWIFT, WITH_RNG(RNG_CUTE_CHARM, TRUE)); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        NONE_OF {
            ABILITY_POPUP(opponent, ABILITY_CUTE_CHARM);
            MESSAGE("Wobbuffet fell in love!");
        }
    }
}

// Cute Charm cannot infatuate a same-gender attacker, innate included (parity with the real ability).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Cute Charm cannot infatuate a same-gender attacker")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MILOTIC, ABILITY_CUTE_CHARM));
        ASSUME(MoveMakesContact(MOVE_SCRATCH));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Gender(MON_FEMALE); Speed(100); }
        OPPONENT(SPECIES_MILOTIC) { Gender(MON_FEMALE); Ability(ABILITY_MARVEL_SCALE); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH, WITH_RNG(RNG_CUTE_CHARM, TRUE)); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        NONE_OF {
            ABILITY_POPUP(opponent, ABILITY_CUTE_CHARM);
            MESSAGE("Wobbuffet fell in love!");
        }
    }
}

// Suppression parity: Gastro Acid nullifies an innate Cute Charm exactly like a real ability, so the
// forced roll produces no infatuation once the innate is suppressed.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Cute Charm")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MILOTIC, ABILITY_CUTE_CHARM));
        ASSUME(MoveMakesContact(MOVE_SCRATCH));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Gender(MON_MALE); Speed(100); Moves(MOVE_SCRATCH, MOVE_GASTRO_ACID); }
        OPPONENT(SPECIES_MILOTIC) { Gender(MON_FEMALE); Ability(ABILITY_MARVEL_SCALE); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_GASTRO_ACID); MOVE(opponent, MOVE_CELEBRATE); } // innate suppressed
        TURN { MOVE(player, MOVE_SCRATCH, WITH_RNG(RNG_CUTE_CHARM, TRUE)); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        NONE_OF {
            ABILITY_POPUP(opponent, ABILITY_CUTE_CHARM);
            MESSAGE("Wobbuffet fell in love!");
        }
    }
}

// The 30% trigger chance is unchanged for an innate Cute Charm (the same RNG_CUTE_CHARM roll as the
// real ability), confirmed with PASSES_RANDOMLY over the innate-only Milotic.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Cute Charm triggers 30% of the time")
{
    PASSES_RANDOMLY(3, 10, RNG_CUTE_CHARM);
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MILOTIC, ABILITY_CUTE_CHARM));
        ASSUME(MoveMakesContact(MOVE_SCRATCH));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(B_ABILITY_TRIGGER_CHANCE, GEN_4);
        PLAYER(SPECIES_WOBBUFFET) { Gender(MON_MALE); Speed(100); }
        OPPONENT(SPECIES_MILOTIC) { Gender(MON_FEMALE); Ability(ABILITY_MARVEL_SCALE); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Wobbuffet fell in love!");
    }
}

// ===== Oblivious =====
// Oblivious prevents the holder from being infatuated or Taunted (B_OBLIVIOUS_TAUNT >= GEN_6) and
// makes it immune to Intimidate (B_UPDATED_INTIMIDATE >= GEN_8). A passive trait wired innate-aware at
// the scattered immunity sites (the Attract/Taunt/Captivate/Intimidate blocks and the switch-in cure),
// each overwriting the pop-up to Oblivious when the chosen ability differs. A clean upside (it only ever
// helps its holder), so the innate is a 1:1 copy of the real ability. Whiscash is the vehicle: it carries
// Oblivious as its slot-0 ability, so a build that picks Anticipation/Hydration still wants the immunity.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Oblivious prevents infatuation")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_ATTRACT) == EFFECT_ATTRACT);
        ASSUME(SpeciesHasInnate(SPECIES_WHISCASH, ABILITY_OBLIVIOUS));
        ASSUME(gSpeciesInfo[SPECIES_WHISCASH].abilities[1] != ABILITY_OBLIVIOUS);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WHISCASH) { Ability(ABILITY_ANTICIPATION); Gender(MON_MALE); } // chosen ability differs from the innate
        OPPONENT(SPECIES_WOBBUFFET) { Gender(MON_FEMALE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_ATTRACT); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_OBLIVIOUS); // pop-up shows Oblivious, not the chosen Anticipation
            MESSAGE("It doesn't affect Whiscash…");
        } else {
            MESSAGE("Whiscash fell in love!"); // no innate -> Attract infatuates
        }
    } THEN {
        EXPECT_EQ(player->volatiles.infatuation != 0, !enabled);
    }
}

// Oblivious also blocks Taunt (Gen 6+), via its innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Oblivious prevents Taunt (Gen6+)")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_TAUNT) == EFFECT_TAUNT);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(B_OBLIVIOUS_TAUNT, GEN_6);
        PLAYER(SPECIES_WHISCASH) { Ability(ABILITY_ANTICIPATION); } // innate Oblivious
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_TAUNT); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_OBLIVIOUS);
        MESSAGE("It doesn't affect Whiscash…");
    } THEN {
        EXPECT(player->volatiles.tauntTimer == 0);
    }
}

// Oblivious blocks Captivate's Sp. Atk drop, via its innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Oblivious prevents Captivate")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_CAPTIVATE) == EFFECT_CAPTIVATE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WHISCASH) { Ability(ABILITY_ANTICIPATION); Gender(MON_MALE); } // innate Oblivious
        OPPONENT(SPECIES_WOBBUFFET) { Gender(MON_FEMALE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CAPTIVATE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_OBLIVIOUS);
        NONE_OF { ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, player); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPATK], DEFAULT_STAT_STAGE);
    }
}

// Oblivious makes the holder Intimidate-immune (Gen 8+), via its innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Oblivious prevents Intimidate (Gen8+)")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(B_UPDATED_INTIMIDATE, GEN_8);
        PLAYER(SPECIES_WHISCASH) { Ability(ABILITY_ANTICIPATION); } // innate Oblivious
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_EKANS) { Ability(ABILITY_INTIMIDATE); }
    } WHEN {
        TURN { SWITCH(opponent, 1); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_INTIMIDATE);
        ABILITY_POPUP(player, ABILITY_OBLIVIOUS);
        NONE_OF { ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, player); }
        MESSAGE("Whiscash's Attack was not lowered!");
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE);
    }
}

// Suppression parity: Oblivious is breakable, so an attacker's Mold Breaker pierces an innate Oblivious
// exactly as it would the real ability — the Attract lands and no Oblivious pop-up shows.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker pierces an innate Oblivious")
{
    GIVEN {
        ASSUME(gAbilitiesInfo[ABILITY_OBLIVIOUS].breakable);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WHISCASH) { Ability(ABILITY_ANTICIPATION); Gender(MON_MALE); } // innate Oblivious
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MOLD_BREAKER); Gender(MON_FEMALE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_ATTRACT); }
    } SCENE {
        MESSAGE("Whiscash fell in love!"); // Mold Breaker ignores the innate -> infatuated
        NONE_OF { ABILITY_POPUP(player, ABILITY_OBLIVIOUS); }
    } THEN {
        EXPECT(player->volatiles.infatuation);
    }
}

// Suppression parity: Gastro Acid nullifies an innate Oblivious, so a later Attract lands.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Oblivious")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WHISCASH) { Ability(ABILITY_ANTICIPATION); Gender(MON_MALE); Moves(MOVE_CELEBRATE); } // innate Oblivious
        OPPONENT(SPECIES_WOBBUFFET) { Gender(MON_FEMALE); Moves(MOVE_GASTRO_ACID, MOVE_ATTRACT); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_GASTRO_ACID); } // suppresses the innate
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_ATTRACT); }
    } SCENE {
        MESSAGE("Whiscash's Ability was suppressed!");
        MESSAGE("Whiscash fell in love!"); // suppressed -> Attract infatuates
    } THEN {
        EXPECT(player->volatiles.infatuation);
    }
}

// The immunity-ability cure site (TryImmunityAbilityHealStatus): once a mon has been infatuated through a
// Mold Breaker Attract (which pierces the innate), the innate Oblivious clears that infatuation at the next
// move-end where it is active (i.e. the holder's own move, when no Mold Breaker attacker suppresses it),
// exactly like a real Oblivious. The pop-up shows Oblivious, not the chosen Anticipation.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Oblivious clears infatuation inflicted through Mold Breaker")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WHISCASH) { Ability(ABILITY_ANTICIPATION); Gender(MON_MALE); Speed(100); Moves(MOVE_CELEBRATE); } // innate Oblivious
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MOLD_BREAKER); Gender(MON_FEMALE); Speed(50); Moves(MOVE_ATTRACT, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_ATTRACT); }        // Mold Breaker pierces -> Whiscash infatuated
        TURN { MOVE(player, MOVE_CELEBRATE, WITH_RNG(RNG_INFATUATION, FALSE)); MOVE(opponent, MOVE_CELEBRATE); } // holder's move-end -> innate cures
    } SCENE {
        MESSAGE("Whiscash fell in love!");
        ABILITY_POPUP(player, ABILITY_OBLIVIOUS);
        MESSAGE("Whiscash got over its infatuation!");
    } THEN {
        EXPECT(!player->volatiles.infatuation);
    }
}

// A canon Oblivious user whose *chosen* ability differs (Dondozo: Unaware/Oblivious/Water Veil) still
// carries Oblivious as an innate, so it keeps the immunity no matter which slot the build picks.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: a canon Oblivious user keeps it via innate when the chosen ability differs")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_DONDOZO, ABILITY_OBLIVIOUS));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_DONDOZO) { Ability(ABILITY_WATER_VEIL); Gender(MON_MALE); } // chosen ability is NOT Oblivious
        OPPONENT(SPECIES_WOBBUFFET) { Gender(MON_FEMALE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_ATTRACT); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_OBLIVIOUS);
        MESSAGE("It doesn't affect Dondozo…");
        NONE_OF { ANIMATION(ANIM_TYPE_STATUS, B_ANIM_STATUS_INFATUATION, player); }
    }
}

// FORK: explicit Mega-form innate rows (src/fork/innate_abilities.c). A Mega form does NOT
// inherit its base species' innates automatically — each Mega that should keep them has its
// own row, mirroring the base creature's innate list as a pure boon (e.g. Mega Venusaur keeps
// Overgrow/Chlorophyll/etc. even though its real ability is Thick Fat). Grounded Megas are the
// deliberate exceptions: Mega Gengar gets NO row (Levitate was its only inheritable innate, and
// it must not float), and Mega Mewtwo X keeps only the non-floating boon (Pressure), dropping
// Levitate. These are pure data-lookup tests, in the species_tiers.c forme-resolution style.
TEST("Innate abilities: Mega Venusaur carries the same innates as base Venusaur")
{
    EXPECT(SpeciesHasInnate(SPECIES_VENUSAUR_MEGA, ABILITY_OVERGROW));
    EXPECT(SpeciesHasInnate(SPECIES_VENUSAUR_MEGA, ABILITY_CHLOROPHYLL));
    EXPECT(SpeciesHasInnate(SPECIES_VENUSAUR_MEGA, ABILITY_FILTER));
    EXPECT(SpeciesHasInnate(SPECIES_VENUSAUR_MEGA, ABILITY_NATURAL_CURE));
    EXPECT(SpeciesHasInnate(SPECIES_VENUSAUR_MEGA, ABILITY_REGENERATOR));
    // Both Charizard Megas keep base Charizard's Blaze.
    EXPECT(SpeciesHasInnate(SPECIES_CHARIZARD_MEGA_X, ABILITY_BLAZE));
    EXPECT(SpeciesHasInnate(SPECIES_CHARIZARD_MEGA_Y, ABILITY_BLAZE));
}

TEST("Innate abilities: grounded Megas do not gain innate Levitate")
{
    // Base Gengar floats (innate Levitate), but Mega Gengar is grounded -> no row, no innate.
    EXPECT(SpeciesHasInnate(SPECIES_GENGAR, ABILITY_LEVITATE));
    EXPECT(!SpeciesHasInnate(SPECIES_GENGAR_MEGA, ABILITY_LEVITATE));
    // Mega Mewtwo X is grounded: keeps base Mewtwo's Pressure but drops its Levitate.
    EXPECT(SpeciesHasInnate(SPECIES_MEWTWO, ABILITY_LEVITATE));
    EXPECT(SpeciesHasInnate(SPECIES_MEWTWO_MEGA_X, ABILITY_PRESSURE));
    EXPECT(!SpeciesHasInnate(SPECIES_MEWTWO_MEGA_X, ABILITY_LEVITATE));
}

// Mega forms mirror innates the base gained later (Sand Veil / Snow Cloak), across BOTH the
// standard Mega and the fork's Mega Z variant, so a Garchomp/Absol keeps the trait whichever
// Mega Stone it holds (ITEM_GARCHOMPITE -> _MEGA, ITEM_GARCHOMPITE_Z -> _MEGA_Z).
TEST("Innate abilities: Mega forms inherit later-added base innates")
{
    // Garchomp's Sand Veil rides through both Mega variants.
    EXPECT(SpeciesHasInnate(SPECIES_GARCHOMP, ABILITY_SAND_VEIL));
    EXPECT(SpeciesHasInnate(SPECIES_GARCHOMP_MEGA, ABILITY_SAND_VEIL));
    EXPECT(SpeciesHasInnate(SPECIES_GARCHOMP_MEGA_Z, ABILITY_SAND_VEIL));
    // Absol's Pressure rides through both Mega variants.
    EXPECT(SpeciesHasInnate(SPECIES_ABSOL_MEGA, ABILITY_PRESSURE));
    EXPECT(SpeciesHasInnate(SPECIES_ABSOL_MEGA_Z, ABILITY_PRESSURE));
    // Froslass keeps both its innates (the flavor-floater Levitate and its Snow Cloak) when Mega.
    EXPECT(SpeciesHasInnate(SPECIES_FROSLASS_MEGA, ABILITY_LEVITATE));
    EXPECT(SpeciesHasInnate(SPECIES_FROSLASS_MEGA, ABILITY_SNOW_CLOAK));
}

// ============================== Compound Eyes / Keen Eye / Illuminate ==============================
// The accuracy abilities. The fork sets DETERMINISTIC_ACCURACY_EVASION, so a move's accuracy never
// decides hit/miss — accuracy/evasion stages are a per-use PP economy instead. There, all three make
// the holder IGNORE the target's evasion (so an evasive foe never PP-taxes it). Keen Eye / Illuminate
// additionally keep the holder's own accuracy from being lowered. Each test uses a species whose CHOSEN
// ability differs from the innate, so the effect is attributable solely to the innate.

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Compound Eyes ignores the target's evasion (no PP tax)")
{
    u32 enabled, expectedPP;
    PARAMETRIZE { enabled = TRUE;  expectedPP = 34; } // innate Compound Eyes -> evasion ignored -> only the base 1 PP
    PARAMETRIZE { enabled = FALSE; expectedPP = 33; } // no innate -> the foe's +1 evasion taxes an extra 1 PP
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_BUTTERFREE, ABILITY_COMPOUND_EYES));
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_BUTTERFREE) { Ability(ABILITY_TINTED_LENS); Speed(50); Moves(MOVE_POUND, MOVE_CELEBRATE); } // chosen Tinted Lens, NOT Compound Eyes
        OPPONENT(SPECIES_WOBBUFFET) { Speed(20); Moves(MOVE_DOUBLE_TEAM, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_DOUBLE_TEAM); } // opponent +1 evasion
        TURN { MOVE(player, MOVE_POUND); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->pp[0], expectedPP);
    }
}

// In a non-deterministic build the fork models an innate Compound Eyes as "ignore the target's evasion"
// in the raw hit calc too (GetTotalAccuracy): a 100%-accuracy move into a +1-evasion foe still always hits.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Compound Eyes ignores the target's evasion in the hit calc")
{
    PASSES_RANDOMLY(5, 5, RNG_ACCURACY); // evasion ignored -> 100 acc -> never misses
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_BUTTERFREE, ABILITY_COMPOUND_EYES));
        ASSUME(GetMoveAccuracy(MOVE_POUND) == 100);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_BUTTERFREE) { Ability(ABILITY_TINTED_LENS); Speed(50); Moves(MOVE_POUND, MOVE_CELEBRATE); } // chosen Tinted Lens, NOT Compound Eyes
        OPPONENT(SPECIES_WOBBUFFET) { Speed(20); Moves(MOVE_DOUBLE_TEAM, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_DOUBLE_TEAM); } // opponent +1 evasion
        TURN { MOVE(player, MOVE_POUND); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        HP_BAR(opponent);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: with the feature off, Compound Eyes does not ignore evasion (stock behavior)")
{
    PASSES_RANDOMLY(3, 4, RNG_ACCURACY); // +1 evasion -> 100 * 3/4 = 75 acc -> hits 3/4
    GIVEN {
        ASSUME(GetMoveAccuracy(MOVE_POUND) == 100);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, FALSE);
        PLAYER(SPECIES_BUTTERFREE) { Ability(ABILITY_TINTED_LENS); Speed(50); Moves(MOVE_POUND, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(20); Moves(MOVE_DOUBLE_TEAM, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_DOUBLE_TEAM); }
        TURN { MOVE(player, MOVE_POUND); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        HP_BAR(opponent);
    }
}

// Suppression parity: Gastro Acid nullifies an innate Compound Eyes, so the foe's evasion taxes PP again.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Compound Eyes")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_BUTTERFREE) { Ability(ABILITY_TINTED_LENS); Speed(50); Moves(MOVE_POUND, MOVE_CELEBRATE); } // innate Compound Eyes
        OPPONENT(SPECIES_WOBBUFFET) { Speed(20); Moves(MOVE_DOUBLE_TEAM, MOVE_GASTRO_ACID, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_DOUBLE_TEAM); }   // opponent +1 evasion
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_GASTRO_ACID); }   // suppress the innate
        TURN { MOVE(player, MOVE_POUND); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->pp[0], 33); // innate suppressed -> the +1 evasion taxes 1 PP again
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Keen Eye ignores the target's evasion (no PP tax)")
{
    u32 enabled, expectedPP;
    PARAMETRIZE { enabled = TRUE;  expectedPP = 34; }
    PARAMETRIZE { enabled = FALSE; expectedPP = 33; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_FEAROW, ABILITY_KEEN_EYE));
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_FEAROW) { Ability(ABILITY_SNIPER); Speed(50); Moves(MOVE_POUND, MOVE_CELEBRATE); } // chosen Sniper, NOT Keen Eye
        OPPONENT(SPECIES_WOBBUFFET) { Speed(20); Moves(MOVE_DOUBLE_TEAM, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_DOUBLE_TEAM); } // opponent +1 evasion
        TURN { MOVE(player, MOVE_POUND); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->pp[0], expectedPP);
    }
}

// Keen Eye's second effect: the holder's accuracy cannot be lowered. The pop-up/message show Keen Eye
// even though the chosen ability is Sniper (the Limber/Oblivious pop-up precedent).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Keen Eye keeps the holder's accuracy from being lowered")
{
    u32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_FEAROW, ABILITY_KEEN_EYE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_FEAROW) { Ability(ABILITY_SNIPER); } // chosen Sniper, NOT Keen Eye
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SAND_ATTACK); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SAND_ATTACK); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_KEEN_EYE);
            MESSAGE("Fearow's accuracy was not lowered!");
        }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ACC], enabled ? DEFAULT_STAT_STAGE : DEFAULT_STAT_STAGE - 1);
    }
}

// Suppression parity: Keen Eye is breakable, so an attacker's Mold Breaker pierces the innate
// accuracy-drop immunity exactly as it would the real ability (the accuracy drop lands, no pop-up).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker pierces an innate Keen Eye's accuracy-drop immunity")
{
    GIVEN {
        ASSUME(gAbilitiesInfo[ABILITY_KEEN_EYE].breakable);
        ASSUME(SpeciesHasInnate(SPECIES_FEAROW, ABILITY_KEEN_EYE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_FEAROW) { Ability(ABILITY_SNIPER); } // innate Keen Eye
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MOLD_BREAKER); Moves(MOVE_SAND_ATTACK); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SAND_ATTACK); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(player, ABILITY_KEEN_EYE); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ACC], DEFAULT_STAT_STAGE - 1); // pierced -> accuracy drops
    }
}

// Illuminate's in-battle effect (Gen 9+) matches Keen Eye: it too keeps the holder's accuracy undroppable.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Illuminate keeps the holder's accuracy from being lowered (Gen 9+)")
{
    GIVEN {
        ASSUME(B_ILLUMINATE_EFFECT >= GEN_9);
        ASSUME(SpeciesHasInnate(SPECIES_CHINCHOU, ABILITY_ILLUMINATE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_CHINCHOU) { Ability(ABILITY_VOLT_ABSORB); } // chosen Volt Absorb, NOT Illuminate
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SAND_ATTACK); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SAND_ATTACK); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_ILLUMINATE);
        MESSAGE("Chinchou's accuracy was not lowered!");
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ACC], DEFAULT_STAT_STAGE);
    }
}

// Data: canon users (incl. Gmax), Mega mirrors (the FORMS pure-boon convention), Watchog's Keen Eye +
// Illuminate pair, and Spewpa correctly excluded (its data is Shed Skin/Friend Guard, not Compound Eyes).
TEST("Innate abilities: accuracy abilities — canon users, Mega mirrors, Watchog's pair")
{
    EXPECT(SpeciesHasInnate(SPECIES_BUTTERFREE, ABILITY_COMPOUND_EYES));
    EXPECT(SpeciesHasInnate(SPECIES_BUTTERFREE_GMAX, ABILITY_COMPOUND_EYES));
    EXPECT(SpeciesHasInnate(SPECIES_PIDGEOT, ABILITY_KEEN_EYE));
    EXPECT(SpeciesHasInnate(SPECIES_PIDGEOT_MEGA, ABILITY_KEEN_EYE)); // Mega mirrors the base though its data is No Guard
    EXPECT(SpeciesHasInnate(SPECIES_SABLEYE_MEGA, ABILITY_KEEN_EYE)); // Mega mirrors the base (its data is Magic Bounce)
    EXPECT(SpeciesHasInnate(SPECIES_SABLEYE_MEGA, ABILITY_PRANKSTER));
    EXPECT(SpeciesHasInnate(SPECIES_WATCHOG, ABILITY_KEEN_EYE));
    EXPECT(SpeciesHasInnate(SPECIES_WATCHOG, ABILITY_ILLUMINATE));
    EXPECT(SpeciesHasInnate(SPECIES_VOLBEAT, ABILITY_ILLUMINATE));
    EXPECT(!SpeciesHasInnate(SPECIES_SPEWPA, ABILITY_COMPOUND_EYES));
}

// ===== Insomnia / Vital Spirit / Sweet Veil (sleep immunity) + Early Bird (faster wake) =====
// All three immunity abilities are wired at the single MOVE_EFFECT_SLEEP chokepoint in
// CanSetNonVolatileStatus (src/battle_util.c), which every sleep path funnels through. A canon Insomnia
// user (Hypno: Insomnia/Forewarn/Inner Focus) with a chosen Inner Focus keeps the immunity via the innate
// and the pop-up shows Insomnia. With the feature off it's stock behavior, so Spore sleeps normally.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Insomnia prevents sleep")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(GetMoveNonVolatileStatus(MOVE_SPORE) == MOVE_EFFECT_SLEEP);
        ASSUME(SpeciesHasInnate(SPECIES_HYPNO, ABILITY_INSOMNIA));
        ASSUME(gSpeciesInfo[SPECIES_HYPNO].abilities[0] != ABILITY_INNER_FOCUS);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_HYPNO) { Ability(ABILITY_INNER_FOCUS); } // chosen ability differs from the innate
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SPORE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SPORE); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_INSOMNIA); // pop-up shows Insomnia, not the chosen Inner Focus
            NONE_OF { STATUS_ICON(player, sleep: TRUE); }
        } else {
            STATUS_ICON(player, sleep: TRUE); // no innate -> Spore sleeps
        }
    } THEN {
        if (enabled)
            EXPECT_EQ(player->status1 & STATUS1_SLEEP, 0);
    }
}

// Suppression parity: Insomnia is breakable, so an attacker's Mold Breaker pierces an innate Insomnia
// exactly as it would the real ability — the sleep lands and no Insomnia pop-up shows.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker pierces an innate Insomnia")
{
    GIVEN {
        ASSUME(gAbilitiesInfo[ABILITY_INSOMNIA].breakable);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_HYPNO) { Ability(ABILITY_INNER_FOCUS); } // innate Insomnia
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MOLD_BREAKER); Moves(MOVE_SPORE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SPORE); }
    } SCENE {
        STATUS_ICON(player, sleep: TRUE); // Mold Breaker ignores the innate -> asleep
        NONE_OF { ABILITY_POPUP(player, ABILITY_INSOMNIA); }
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Insomnia")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_HYPNO) { Ability(ABILITY_INNER_FOCUS); Moves(MOVE_CELEBRATE); } // innate Insomnia
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID, MOVE_SPORE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_GASTRO_ACID); } // suppresses the innate
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_SPORE); }
    } SCENE {
        MESSAGE("Hypno's Ability was suppressed!");
        STATUS_ICON(player, sleep: TRUE); // suppressed -> Spore sleeps
    }
}

// Vital Spirit is the identical-effect twin of Insomnia. A canon Vital Spirit user (Electivire:
// Motor Drive/Vital Spirit) keeps the immunity via the innate when the chosen ability differs.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Vital Spirit prevents sleep")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_ELECTIVIRE, ABILITY_VITAL_SPIRIT));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_ELECTIVIRE) { Ability(ABILITY_MOTOR_DRIVE); } // chosen Motor Drive, innate Vital Spirit
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SPORE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SPORE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_VITAL_SPIRIT);
        NONE_OF { STATUS_ICON(player, sleep: TRUE); }
    } THEN {
        EXPECT_EQ(player->status1 & STATUS1_SLEEP, 0);
    }
}

// Innate Insomnia also blocks Yawn: it fails at use (via Cmd_trynonvolatilestatus -> CanSetNonVolatileStatus),
// so the drowsy volatile is never set and the holder never falls asleep.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Insomnia blocks Yawn (no drowsy)")
{
    GIVEN {
        ASSUME(GetMoveNonVolatileStatus(MOVE_YAWN) == MOVE_EFFECT_SLEEP);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_HYPNO) { Ability(ABILITY_INNER_FOCUS); } // innate Insomnia
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_YAWN); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_YAWN); }
        TURN {}
    } SCENE {
        ABILITY_POPUP(player, ABILITY_INSOMNIA);
    } THEN {
        EXPECT_EQ(player->status1 & STATUS1_SLEEP, 0); // never made drowsy -> never asleep
    }
}

// PURE-BOON DIVERGENCE: a real Insomnia/Vital Spirit BLOCKS the holder's own Rest (it can't sleep), a cost.
// The innate intentionally does NOT block Rest, so an innate-Insomnia mon may still Rest to full HP (and sleeps
// from its own move). The chosen-ability Rest gate is left untouched, so this only diverges for the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Insomnia does NOT block the holder's own Rest (pure boon)")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_HYPNO) { Ability(ABILITY_INNER_FOCUS); MaxHP(200); HP(100); Moves(MOVE_REST); } // innate Insomnia
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_REST); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Hypno slept and restored its HP!");
    } THEN {
        EXPECT_EQ(player->hp, player->maxHP);              // Rest healed to full
        EXPECT_NE(player->status1 & STATUS1_SLEEP, 0);     // and put it to sleep (not blocked by the innate)
    }
}

// Sweet Veil is side-wide: a Swirlix carrying it as an innate (chosen Unburden) protects its PARTNER from
// sleep too, with the pop-up shown on the Sweet Veil holder.
DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Sweet Veil protects the whole side from sleep")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SWIRLIX, ABILITY_SWEET_VEIL));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_SWIRLIX) { Ability(ABILITY_UNBURDEN); } // chosen Unburden, innate Sweet Veil
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SPORE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(opponentLeft, MOVE_SPORE, target: playerRight); MOVE(opponentRight, MOVE_CELEBRATE); }
    } SCENE {
        ABILITY_POPUP(playerLeft, ABILITY_SWEET_VEIL); // the Swirlix's Sweet Veil blocks it for the partner
        NONE_OF { STATUS_ICON(playerRight, sleep: TRUE); }
    } THEN {
        EXPECT_EQ(playerRight->status1 & STATUS1_SLEEP, 0);
    }
}

// Identity stays the chosen slot: Trace copies Hypno's chosen Inner Focus, never its innate Insomnia.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Trace copies the chosen ability, never an innate Insomnia")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_HYPNO) { Ability(ABILITY_INNER_FOCUS); } // innate Insomnia, chosen Inner Focus
        OPPONENT(SPECIES_GARDEVOIR) { Ability(ABILITY_TRACE); }
    } WHEN {
        TURN {}
    } THEN {
        EXPECT_EQ(opponent->ability, ABILITY_INNER_FOCUS); // traced the chosen ability, not Insomnia
    }
}

// Early Bird: the holder wakes from sleep twice as fast. Houndoom (Early Bird/Flash Fire/Unnerve) with a
// chosen Flash Fire carries Early Bird only via the innate; with DETERMINISTIC_SLEEP_TURNS a 2-turn sleep
// is shed in one turn (counter -2), so it acts turn 1; with the feature off it stays asleep.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Early Bird wakes from sleep twice as fast")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_HOUNDOOM, ABILITY_EARLY_BIRD));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_HOUNDOOM) { Ability(ABILITY_FLASH_FIRE); Status1(STATUS1_SLEEP_TURN(2)); Moves(MOVE_EMBER); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_EMBER); }
    } SCENE {
        if (enabled) {
            MESSAGE("Houndoom woke up!"); // innate Early Bird sheds the 2-turn sleep in one turn
            ANIMATION(ANIM_TYPE_MOVE, MOVE_EMBER, player);
        } else {
            MESSAGE("Houndoom is fast asleep."); // no innate -> still asleep turn 1
        }
    }
}

// ----- Immunity (poison immunity) -----
// A canon Immunity user (Snorlax: Immunity/Thick Fat/Gluttony) whose *chosen* ability is Gluttony still
// carries Immunity as an innate, so Toxic cannot poison it, and the pop-up shows Immunity. (Zangoose, the
// other canon Immunity user, instead carries innate Toxic Boost — Immunity would contradict it.)
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Immunity prevents poison")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_TOXIC) == EFFECT_NON_VOLATILE_STATUS);
        ASSUME(GetMoveNonVolatileStatus(MOVE_TOXIC) == MOVE_EFFECT_TOXIC);
        ASSUME(SpeciesHasInnate(SPECIES_SNORLAX, ABILITY_IMMUNITY));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_SNORLAX) { Ability(ABILITY_GLUTTONY); } // chosen ability differs from the innate
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_TOXIC); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_IMMUNITY); // pop-up shows Immunity, not the chosen Gluttony
            MESSAGE("It doesn't affect Snorlax…");
            NONE_OF { STATUS_ICON(player, badPoison: TRUE); }
        } else {
            STATUS_ICON(player, badPoison: TRUE); // no innate -> Toxic poisons
        }
    }
}

// Suppression parity: Immunity is breakable, so an attacker's Mold Breaker pierces an innate Immunity
// exactly as it would the real ability — the poison lands and no Immunity pop-up shows.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker pierces an innate Immunity")
{
    GIVEN {
        ASSUME(gAbilitiesInfo[ABILITY_IMMUNITY].breakable);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_SNORLAX) { Ability(ABILITY_GLUTTONY); } // innate Immunity
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MOLD_BREAKER); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_TOXIC); }
    } SCENE {
        STATUS_ICON(player, badPoison: TRUE); // Mold Breaker ignores the innate -> poisoned
        NONE_OF { ABILITY_POPUP(player, ABILITY_IMMUNITY); }
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Immunity")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_SNORLAX) { Ability(ABILITY_GLUTTONY); Moves(MOVE_CELEBRATE); } // innate Immunity
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID, MOVE_TOXIC); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_GASTRO_ACID); } // suppresses the innate
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_TOXIC); }
    } SCENE {
        MESSAGE("Snorlax's Ability was suppressed!");
        STATUS_ICON(player, badPoison: TRUE); // suppressed -> Toxic poisons
    }
}

// The switch-in cure site: a mon poisoned while its innate Immunity was suppressed (Gastro Acid) gets
// the poison cured the moment it switches back in (Gastro Acid clears on switch-out), exactly like a
// real Immunity. The pop-up shows Immunity.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Immunity cures pre-existing poison on switch-in")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_SNORLAX) { Ability(ABILITY_GLUTTONY); Moves(MOVE_CELEBRATE); } // innate Immunity
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID, MOVE_TOXIC, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_GASTRO_ACID); } // suppress the innate
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_TOXIC); }       // Snorlax poisoned
        TURN { SWITCH(player, 1); MOVE(opponent, MOVE_CELEBRATE); }             // Snorlax out -> Gastro clears
        TURN { SWITCH(player, 0); MOVE(opponent, MOVE_CELEBRATE); }             // Snorlax back in -> innate cures
    } SCENE {
        ABILITY_POPUP(player, ABILITY_IMMUNITY);
        MESSAGE("Snorlax was cured of its poisoning!");
    } THEN {
        EXPECT_EQ(player->status1 & (STATUS1_POISON | STATUS1_TOXIC_POISON), 0);
    }
}

// ----- Pastel Veil (side-wide poison immunity) -----
// A canon Pastel Veil user (Rapidash-Galar: Run Away/Pastel Veil/Anticipation) whose *chosen* ability
// is Run Away still carries Pastel Veil as an innate, protecting itself AND its partner.
DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Pastel Veil protects the holder and its partner from poison")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_RAPIDASH_GALAR, ABILITY_PASTEL_VEIL));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_RAPIDASH_GALAR) { Ability(ABILITY_RUN_AWAY); } // chosen ability differs from the innate
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponentLeft, MOVE_TOXIC, target: playerRight); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(playerLeft, ABILITY_PASTEL_VEIL);
            NONE_OF { STATUS_ICON(playerRight, badPoison: TRUE); }
        } else {
            STATUS_ICON(playerRight, badPoison: TRUE); // no innate -> Toxic poisons the partner
        }
    }
}

// Suppression parity: Pastel Veil is breakable, so an attacker's Mold Breaker pierces an innate Pastel
// Veil exactly as it would the real ability.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker pierces an innate Pastel Veil")
{
    GIVEN {
        ASSUME(gAbilitiesInfo[ABILITY_PASTEL_VEIL].breakable);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_RAPIDASH_GALAR) { Ability(ABILITY_RUN_AWAY); } // innate Pastel Veil
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MOLD_BREAKER); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_TOXIC); }
    } SCENE {
        STATUS_ICON(player, badPoison: TRUE); // Mold Breaker ignores the innate -> poisoned
        NONE_OF { ABILITY_POPUP(player, ABILITY_PASTEL_VEIL); }
    }
}

// Batch V: an innate Pastel Veil now runs the real ability's full switch-in cure via the switch-in driver
// (FIRST_EVENT_BLOCK_GENERAL_ABILITIES_INNATE -> BattleScript_PastelVeilActivates), so when the holder
// switches in beside a poisoned ally it clears the ally's pre-existing poison under its pop-up, exactly
// like the chosen ability ("Pastel Veil cures partner's poison on switch in"). This is the half Batch S/I
// deferred until the switch-in driver existed. (The holder here is unpoisoned so the ally cure is
// attributable solely to the new switch-in script, not the passive self-cure fallback below.)
DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Pastel Veil cures its partner's pre-existing poison on switch-in")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_RAPIDASH_GALAR, ABILITY_PASTEL_VEIL));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT) { Status1(STATUS1_POISON); }
        PLAYER(SPECIES_RAPIDASH_GALAR) { Ability(ABILITY_RUN_AWAY); } // chosen ability differs from the innate
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(playerLeft, 2); } // bring the innate Pastel Veil holder in next to the poisoned Wynaut
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(playerLeft, ABILITY_PASTEL_VEIL);
            MESSAGE("Wynaut was cured of its poisoning!"); // ally cure now replicated for an innate holder
        } else {
            NONE_OF { ABILITY_POPUP(playerLeft, ABILITY_PASTEL_VEIL); }
        }
    } THEN {
        if (enabled)
            EXPECT_EQ(playerRight->status1 & STATUS1_POISON, 0); // Wynaut cured
        else
            EXPECT_NE(playerRight->status1 & STATUS1_POISON, 0); // no innate -> Wynaut stays poisoned
    }
}

// An innate Pastel Veil still clears the HOLDER's own pre-existing poison on switch-in (via the switch-in
// script now, with the immunity self-cure as a fallback), attributing the cure to the Pastel Veil pop-up.
DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Pastel Veil cures its own pre-existing poison on switch-in")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_RAPIDASH_GALAR, ABILITY_PASTEL_VEIL));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        PLAYER(SPECIES_RAPIDASH_GALAR) { Ability(ABILITY_RUN_AWAY); Status1(STATUS1_POISON); } // innate Pastel Veil
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(playerLeft, 2); }
    } SCENE {
        ABILITY_POPUP(playerLeft, ABILITY_PASTEL_VEIL);
        MESSAGE("Rapidash was cured of its poisoning!");
    } THEN {
        EXPECT_EQ(playerLeft->status1 & STATUS1_POISON, 0);
    }
}

// ===== Batch A — offensive move-power boosters =====
// All wired beside their chosen-ability case in CalcMoveBasePowerAfterModifiers / CalcAttackStat
// (src/battle_util.c), Adaptability in GetSameTypeAttackBonusModifier. Each is a 1:1 clean-upside
// copy: a parametrized off/on run shows the multiplier appears only with the feature on, on a holder
// whose CHOSEN ability is something else (so the boost can only be the innate).

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Iron Fist boosts a punching move 1.2x", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_HITMONCHAN, ABILITY_IRON_FIST));
        ASSUME(gSpeciesInfo[SPECIES_HITMONCHAN].abilities[0] != ABILITY_IRON_FIST);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_HITMONCHAN) { Moves(MOVE_THUNDER_PUNCH); } // chosen Keen Eye
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_THUNDER_PUNCH); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.2), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Reckless boosts a recoil move 1.2x", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_HITMONLEE, ABILITY_RECKLESS));
        ASSUME(gSpeciesInfo[SPECIES_HITMONLEE].abilities[0] != ABILITY_RECKLESS);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_HITMONLEE) { Moves(MOVE_DOUBLE_EDGE); } // chosen Limber
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_DOUBLE_EDGE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.2), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Strong Jaw boosts a biting move 1.5x", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_BRUXISH, ABILITY_STRONG_JAW));
        ASSUME(gSpeciesInfo[SPECIES_BRUXISH].abilities[0] != ABILITY_STRONG_JAW);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_BRUXISH) { Moves(MOVE_CRUNCH); } // chosen Dazzling
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CRUNCH); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.5), results[1].damage);
    }
}

// Perrserker carries innate Tough Claws AND innate Steely Spirit, so a NON-Steel contact move
// isolates Tough Claws (Steely Spirit would only touch Steel moves).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Tough Claws boosts a contact move 1.3x", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_PERRSERKER, ABILITY_TOUGH_CLAWS));
        ASSUME(gSpeciesInfo[SPECIES_PERRSERKER].abilities[0] != ABILITY_TOUGH_CLAWS);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_PERRSERKER) { Moves(MOVE_X_SCISSOR); } // chosen Battle Armor; Bug move, not Steel
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_X_SCISSOR); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.3), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Sharpness boosts a slicing move 1.5x", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_KLEAVOR, ABILITY_SHARPNESS));
        ASSUME(gSpeciesInfo[SPECIES_KLEAVOR].abilities[0] != ABILITY_SHARPNESS);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_KLEAVOR) { Moves(MOVE_NIGHT_SLASH); } // chosen Swarm (inert at full HP)
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_NIGHT_SLASH); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.5), results[1].damage);
    }
}

// Clawitzer's only real ability is Mega Launcher, so its chosen ability is forced to an inert one;
// the pulse boost can then only be the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Mega Launcher boosts a pulse move 1.5x", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CLAWITZER, ABILITY_MEGA_LAUNCHER));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_CLAWITZER) { Ability(ABILITY_DAMP); Moves(MOVE_WATER_PULSE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_WATER_PULSE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.5), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Steelworker boosts a Steel move 1.5x", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_DHELMISE, ABILITY_STEELWORKER));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_DHELMISE) { Ability(ABILITY_DAMP); Moves(MOVE_IRON_HEAD); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_IRON_HEAD); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.5), results[1].damage);
    }
}

// An innate Steely Spirit on the PARTNER boosts the attacker's Steel move (doubles, partner site).
DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: an ally's innate Steely Spirit boosts a Steel move 1.5x", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_PERRSERKER, ABILITY_STEELY_SPIRIT));
        ASSUME(gSpeciesInfo[SPECIES_PERRSERKER].abilities[0] != ABILITY_STEELY_SPIRIT);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_IRON_HEAD); }
        PLAYER(SPECIES_PERRSERKER); // chosen Battle Armor; innate Steely Spirit
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_IRON_HEAD, target: opponentLeft); }
    } SCENE {
        HP_BAR(opponentLeft, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.5), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Rocky Payload boosts a Rock move 1.5x", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_BOMBIRDIER, ABILITY_ROCKY_PAYLOAD));
        ASSUME(gSpeciesInfo[SPECIES_BOMBIRDIER].abilities[0] != ABILITY_ROCKY_PAYLOAD);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_BOMBIRDIER) { Moves(MOVE_ROCK_SLIDE); } // chosen Big Pecks
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_ROCK_SLIDE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.5), results[1].damage);
    }
}

// Sand Force boosts Ground/Rock/Steel moves 1.3x while a sandstorm rages.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Sand Force boosts a Ground move 1.3x in sand", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_EXCADRILL, ABILITY_SAND_FORCE));
        ASSUME(gSpeciesInfo[SPECIES_EXCADRILL].abilities[0] != ABILITY_SAND_FORCE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_EXCADRILL) { Moves(MOVE_SANDSTORM, MOVE_EARTHQUAKE); } // chosen Sand Rush
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_SAFETY_GOGGLES); } // no sandstorm chip, so HP_BAR is the Earthquake
    } WHEN {
        TURN { MOVE(player, MOVE_SANDSTORM); }
        TURN { MOVE(player, MOVE_EARTHQUAKE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.3), results[1].damage);
    }
}

// Analytic boosts power 1.3x when the holder is the last to move that turn.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Analytic boosts power 1.3x when moving last", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_ELGYEM, ABILITY_ANALYTIC));
        ASSUME(gSpeciesInfo[SPECIES_ELGYEM].abilities[0] != ABILITY_ANALYTIC);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_ELGYEM) { Moves(MOVE_PSYCHIC); Speed(1); } // chosen Telepathy
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); Speed(255); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CELEBRATE); MOVE(player, MOVE_PSYCHIC); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.3), results[1].damage);
    }
}

// Adaptability raises STAB from 1.5x to 2.0x (ratio 4/3) in GetSameTypeAttackBonusModifier.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Adaptability raises STAB to 2x", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CRAWDAUNT, ABILITY_ADAPTABILITY));
        ASSUME(gSpeciesInfo[SPECIES_CRAWDAUNT].abilities[0] != ABILITY_ADAPTABILITY);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_CRAWDAUNT) { Moves(MOVE_AQUA_TAIL); } // Water STAB; chosen Hyper Cutter
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_AQUA_TAIL); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.3333), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Punk Rock boosts a sound move 1.3x", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_TOXTRICITY, ABILITY_PUNK_ROCK));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_TOXTRICITY) { Ability(ABILITY_DAMP); Moves(MOVE_HYPER_VOICE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_HYPER_VOICE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.3), results[1].damage);
    }
}

// Stakeout deals double damage to a target that just switched in.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Stakeout doubles damage to a just-switched-in target", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SPIDOPS, ABILITY_STAKEOUT));
        ASSUME(gSpeciesInfo[SPECIES_SPIDOPS].abilities[0] != ABILITY_STAKEOUT);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_SPIDOPS) { Moves(MOVE_BODY_SLAM); } // chosen Insomnia
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(opponent, 1); MOVE(player, MOVE_BODY_SLAM); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(2.0), results[1].damage);
    }
}

// Suppression parity: Gastro Acid nullifies an innate Iron Fist (none of Batch A is breakable, so
// Mold Breaker can't pierce them — Gastro Acid is the relevant suppressor, same as the real ability).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Iron Fist", s16 damage)
{
    bool32 gastro;
    PARAMETRIZE { gastro = FALSE; }
    PARAMETRIZE { gastro = TRUE; }
    GIVEN {
        ASSUME(!gAbilitiesInfo[ABILITY_IRON_FIST].breakable);
        ASSUME(SpeciesHasInnate(SPECIES_HITMONCHAN, ABILITY_IRON_FIST));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_HITMONCHAN) { Moves(MOVE_THUNDER_PUNCH); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID, MOVE_CELEBRATE); }
    } WHEN {
        TURN { if (gastro) MOVE(opponent, MOVE_GASTRO_ACID); else MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_THUNDER_PUNCH); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.2), results[0].damage); // suppressed: base; active: 1.2x
    }
}

// Serene Grace: doubles the chance of the holder's moves' additional effects. We test the
// doubling deterministically with a 50%-chance secondary (Sacred Fire's burn): the innate
// doubles 50% -> 100%, which is *certain* and never consults the RNG, so the burn lands even
// when we force the secondary-effect roll to FALSE (which would deny a stock 50% burn).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Serene Grace doubles a 50% secondary effect to a guarantee")
{
    GIVEN {
        ASSUME(MoveHasAdditionalEffect(MOVE_SACRED_FIRE, MOVE_EFFECT_BURN));
        ASSUME(SpeciesHasInnate(SPECIES_TOGEKISS, ABILITY_SERENE_GRACE));
        ASSUME(gSpeciesInfo[SPECIES_TOGEKISS].abilities[0] != ABILITY_SERENE_GRACE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_TOGEKISS) { Ability(ABILITY_SUPER_LUCK); } // chosen ability is NOT Serene Grace
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SACRED_FIRE, WITH_RNG(RNG_SECONDARY_EFFECT, FALSE)); }
    } SCENE {
        STATUS_ICON(opponent, burn: TRUE);
    }
}

// Control: with the feature off Togekiss has no innate Serene Grace, so the same forced-FALSE
// roll denies the stock 50% burn.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: with the feature off, no innate Serene Grace doubling (stock behavior)")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, FALSE);
        PLAYER(SPECIES_TOGEKISS) { Ability(ABILITY_SUPER_LUCK); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SACRED_FIRE, WITH_RNG(RNG_SECONDARY_EFFECT, FALSE)); }
    } SCENE {
        NONE_OF { STATUS_ICON(opponent, burn: TRUE); }
    }
}

// Suppression parity: Gastro Acid silences the innate, so the burn falls back to the stock 50%
// roll, which the forced FALSE denies.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Serene Grace")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_TOGEKISS) { Ability(ABILITY_SUPER_LUCK); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_GASTRO_ACID); }
        TURN { MOVE(player, MOVE_SACRED_FIRE, WITH_RNG(RNG_SECONDARY_EFFECT, FALSE)); }
    } SCENE {
        NONE_OF { STATUS_ICON(opponent, burn: TRUE); }
    }
}

// Interaction with DETERMINISTIC_ADDITIONAL_EFFECTS (the actual in-game default; the other
// Serene Grace tests above run with it forced OFF by the test baseline). Under that flag a
// chance-based effect is gated on a super-effective / STAB hit, but a Serene Grace / Rainbow
// boost (a computed chance ABOVE the move's base) bypasses the gate and makes the effect
// certain. The deterministic resolver keys off the boosted chance, NOT a direct ability check
// (TryTriggerAdditionalEffect, src/fork/deterministic_moves.c), so an innate Serene Grace —
// which feeds the boost through CalcSecondaryEffectChance — trips the gate exactly like the
// real ability. Fire Punch (Fire, 10% burn) vs a Psychic target is neutral and non-STAB for a
// Fairy/Flying Togekiss, so without the booster the burn would be gated out.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Serene Grace guarantees a gated effect under DETERMINISTIC_ADDITIONAL_EFFECTS")
{
    GIVEN {
        ASSUME(MoveHasAdditionalEffect(MOVE_FIRE_PUNCH, MOVE_EFFECT_BURN));
        ASSUME(SpeciesHasInnate(SPECIES_TOGEKISS, ABILITY_SERENE_GRACE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        PLAYER(SPECIES_TOGEKISS) { Ability(ABILITY_SUPER_LUCK); Moves(MOVE_FIRE_PUNCH); } // chosen ability is NOT Serene Grace
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(600); HP(600); Defense(255); }
    } WHEN {
        TURN { MOVE(player, MOVE_FIRE_PUNCH); }
    } THEN {
        EXPECT(opponent->status1 & STATUS1_BURN);
    }
}

// Control for the deterministic case: with the feature off Togekiss has no innate Serene
// Grace, so the same neutral, non-STAB Fire Punch is gated out and does NOT burn.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: feature off — a gated effect stays gated under DETERMINISTIC_ADDITIONAL_EFFECTS")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, FALSE);
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        PLAYER(SPECIES_TOGEKISS) { Ability(ABILITY_SUPER_LUCK); Moves(MOVE_FIRE_PUNCH); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(600); HP(600); Defense(255); }
    } WHEN {
        TURN { MOVE(player, MOVE_FIRE_PUNCH); }
    } THEN {
        EXPECT(!(opponent->status1 & STATUS1_BURN));
    }
}

// A flavor pick (Gardevoir, no native Serene Grace) gets the doubling too.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: a flavor Serene Grace (Gardevoir) doubles secondary chances")
{
    GIVEN {
        ASSUME(gSpeciesInfo[SPECIES_GARDEVOIR].abilities[0] != ABILITY_SERENE_GRACE);
        ASSUME(SpeciesHasInnate(SPECIES_GARDEVOIR, ABILITY_SERENE_GRACE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_GARDEVOIR);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SACRED_FIRE, WITH_RNG(RNG_SECONDARY_EFFECT, FALSE)); }
    } SCENE {
        STATUS_ICON(opponent, burn: TRUE);
    }
}

// ===== Batch B: defensive damage reducers =====
// All seven are pure calc-modifier passives wired beside the existing defender-ability sites in
// src/battle_util.c (Multiscale/Solid Rock/Ice Scales in GetDefenderAbilitiesModifier, Fur Coat in the
// defense-stat calc, Heatproof/Water Bubble in the Fire-damage calc, Friend Guard in the partner
// modifier). Each is a 1:1 clean-upside copy, so on-field AI damage prediction is correct for free (the
// AI runs the same shared calc keyed off the real battler). Each test sets the chosen ability to a
// non-reducing one so the feature-off run is stock and the off-vs-on ratio isolates the innate.

// ---- Multiscale ----
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Multiscale halves damage at full HP", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_DRAGONITE, ABILITY_MULTISCALE));
        ASSUME(gSpeciesInfo[SPECIES_DRAGONITE].abilities[0] != ABILITY_MULTISCALE); // off-run runs Inner Focus
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_DRAGONITE);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_BODY_SLAM); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_BODY_SLAM); } // lands while the holder is at full HP
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.5), results[1].damage); // off: full; on: 0.5x at full HP
    }
}

// Suppression parity: Gastro Acid turns off the innate, so the same hit lands at full power.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Multiscale", s16 damage)
{
    bool32 gastro;
    PARAMETRIZE { gastro = FALSE; }
    PARAMETRIZE { gastro = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_DRAGONITE, ABILITY_MULTISCALE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_DRAGONITE) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_BODY_SLAM, MOVE_GASTRO_ACID, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); if (gastro) MOVE(opponent, MOVE_GASTRO_ACID); else MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_BODY_SLAM); } // still full HP
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(0.5), results[0].damage); // suppressed(full) *0.5 == active(reduced)
    }
}

// ---- Solid Rock ----
// Rhyperior is a canon Solid Rock user whose slot-0 ability is Lightning Rod, so the off-run never
// touches Surf. Rhyperior is Ground/Rock, so Surf is supereffective (4x) -> Solid Rock reduces by 0.75.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Solid Rock reduces supereffective damage by 0.75", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_RHYPERIOR, ABILITY_SOLID_ROCK));
        ASSUME(gSpeciesInfo[SPECIES_RHYPERIOR].abilities[0] != ABILITY_SOLID_ROCK);
        ASSUME(gTypeEffectivenessTable[TYPE_WATER][TYPE_GROUND] > UQ_4_12(1.0));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_RHYPERIOR);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SURF); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SURF); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
        MESSAGE("It's super effective!");
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.75), results[1].damage); // off: full; on: 0.75x
    }
}

// Suppression parity: Solid Rock is breakable, so an attacker's Mold Breaker pierces the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker pierces an innate Solid Rock", s16 damage)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_PRESSURE; }     // innate Solid Rock applies -> reduced
    PARAMETRIZE { ability = ABILITY_MOLD_BREAKER; } // pierces -> full damage
    GIVEN {
        ASSUME(gAbilitiesInfo[ABILITY_SOLID_ROCK].breakable);
        ASSUME(SpeciesHasInnate(SPECIES_RHYPERIOR, ABILITY_SOLID_ROCK));
        ASSUME(gTypeEffectivenessTable[TYPE_WATER][TYPE_GROUND] > UQ_4_12(1.0));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_RHYPERIOR);
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ability); Moves(MOVE_SURF); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SURF); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
        MESSAGE("It's super effective!");
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(0.75), results[0].damage); // Mold Breaker full; Solid Rock 0.75x
    }
}

// ---- Fur Coat ----
// Persian-Alola carries innate Fur Coat (and Technician); its chosen ability is set to Technician
// (a no-op on defense), so the off-run takes full physical damage and the halving is the innate's.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Fur Coat halves physical damage", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_PERSIAN_ALOLA, ABILITY_FUR_COAT));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_PERSIAN_ALOLA) { Ability(ABILITY_TECHNICIAN); } // chosen ability differs from the innate
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_BODY_SLAM); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_BODY_SLAM); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.5), results[1].damage); // doubled Defense -> 0.5x physical
    }
}

// ---- Ice Scales ----
// Frosmoth is a canon Ice Scales user whose slot-0 ability is Shield Dust, so the off-run never touches
// special damage. Frosmoth is Ice/Bug, so Surf is a neutral special hit -> Ice Scales halves it.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Ice Scales halves special damage", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_FROSMOTH, ABILITY_ICE_SCALES));
        ASSUME(gSpeciesInfo[SPECIES_FROSMOTH].abilities[0] != ABILITY_ICE_SCALES);
        ASSUME(GetMoveCategory(MOVE_SURF) == DAMAGE_CATEGORY_SPECIAL);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_FROSMOTH) { Ability(ABILITY_SHIELD_DUST); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SURF); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SURF); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.5), results[1].damage);
    }
}

// ---- Heatproof ----
// Heatproof's Fire-damage halving shares a single code clause with Water Bubble (the CalcAttackStat
// defender switch in src/battle_util.c), so it is exercised by the neutral-target "Water Bubble halves
// Fire damage" test below (a clean 0.5 ratio — a canon Heatproof user is always 2x or resistant to Fire,
// whose extra type multiplier muddies the ±1 EXPECT_MUL_EQ tolerance). Heatproof's distinct half — the
// burn-damage halving at the end-turn site (src/battle_end_turn.c) — gets its own test here.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Heatproof halves burn damage", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_BRONZONG, ABILITY_HEATPROOF));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_BRONZONG) { Ability(ABILITY_LEVITATE); Status1(STATUS1_BURN); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage); // end-of-turn burn tick
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.5), results[1].damage); // off: full burn; on: half burn
    }
}

// ---- Friend Guard ----
// In doubles, an innate Friend Guard on the partner reduces the holder's damage taken by 0.75. Clefable
// carries innate Friend Guard; its chosen ability (Cute Charm) is a no-op here, so the off-run is stock.
DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: an innate Friend Guard partner reduces the ally's damage by 0.75", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CLEFABLE, ABILITY_FRIEND_GUARD));
        ASSUME(gSpeciesInfo[SPECIES_CLEFABLE].abilities[0] != ABILITY_FRIEND_GUARD);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET);     // the protected ally
        PLAYER(SPECIES_CLEFABLE);      // partner carrying innate Friend Guard
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_BODY_SLAM); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(opponentLeft, MOVE_BODY_SLAM, target: playerLeft); MOVE(opponentRight, MOVE_CELEBRATE); }
    } SCENE {
        HP_BAR(playerLeft, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.75), results[1].damage); // off: full; on: 0.75x via the partner
    }
}

// ---- Water Bubble (defensive Fire half + offensive Water half + burn immunity) ----
// Araquanid carries innate Water Bubble; its chosen ability is set to Water Absorb (a no-op vs Fire), so
// the off-run takes full Fire damage. Araquanid is Water/Bug, so Flamethrower is neutral -> halved.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Water Bubble halves Fire damage", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_ARAQUANID, ABILITY_WATER_BUBBLE));
        ASSUME(GetMoveType(MOVE_FLAMETHROWER) == TYPE_FIRE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_ARAQUANID) { Ability(ABILITY_WATER_ABSORB); } // chosen ability differs from the innate
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_FLAMETHROWER); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_FLAMETHROWER); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.5), results[1].damage);
    }
}

// Water Bubble also doubles the holder's Water-move power (offensive half).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Water Bubble doubles the holder's Water-move power", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_ARAQUANID, ABILITY_WATER_BUBBLE));
        ASSUME(GetMoveType(MOVE_SURF) == TYPE_WATER);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_ARAQUANID) { Ability(ABILITY_WATER_ABSORB); Moves(MOVE_SURF); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SURF); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(2.0), results[1].damage); // off: 1x; on: 2x Water power
    }
}

// Water Bubble also blocks burn (the status-set site, Limber-style pop-up).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Water Bubble blocks burn")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_ARAQUANID, ABILITY_WATER_BUBBLE));
        ASSUME(gSpeciesInfo[SPECIES_ARAQUANID].types[0] != TYPE_FIRE && gSpeciesInfo[SPECIES_ARAQUANID].types[1] != TYPE_FIRE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_ARAQUANID) { Ability(ABILITY_WATER_ABSORB); } // chosen ability differs from the innate
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_WILL_O_WISP); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_WILL_O_WISP); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_WATER_BUBBLE); // pop-up shows Water Bubble, not the chosen Water Absorb
            NONE_OF { STATUS_ICON(player, burn: TRUE); }
        } else {
            STATUS_ICON(player, burn: TRUE); // no innate -> burned
        }
    }
}

// ===== Batch N: status-conditional stat boosts (Guts / Marvel Scale / Quick Feet / Toxic Boost / Flare Boost) =====

// Guts: +50% physical Attack while statused. Hariyama's slot-0 ability is Thick Fat, so the off-run never
// touches the attack calc. Poison (not burn) isolates the Attack boost — it has no physical-damage cut.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Guts boosts a physical move 1.5x while statused", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_HARIYAMA, ABILITY_GUTS));
        ASSUME(gSpeciesInfo[SPECIES_HARIYAMA].abilities[0] != ABILITY_GUTS);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_HARIYAMA) { Ability(ABILITY_THICK_FAT); Status1(STATUS1_POISON); Moves(MOVE_BRICK_BREAK); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_BRICK_BREAK); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.5), results[1].damage);
    }
}

// Guts also negates burn's physical-damage cut. Both runs carry the innate (so both get the +50% Attack);
// the only difference is the status, so if Guts ignores the burn cut the burned run deals identical damage.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Guts ignores burn's physical-damage cut", s16 damage)
{
    u32 status;
    PARAMETRIZE { status = STATUS1_POISON; }
    PARAMETRIZE { status = STATUS1_BURN; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_HARIYAMA, ABILITY_GUTS));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_HARIYAMA) { Ability(ABILITY_THICK_FAT); Status1(status); Moves(MOVE_BRICK_BREAK); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_BRICK_BREAK); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_EQ(results[0].damage, results[1].damage); // burn does not reduce a Guts holder's physical damage
    }
}

// Marvel Scale: +50% Defense while statused. Milotic's slot-0 ability IS Marvel Scale, so the test forces a
// different chosen ability (Competitive) and the boost can then only be the innate. Earthquake is a
// non-contact physical hit, so Milotic's innate Cute Charm never interferes.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Marvel Scale boosts Defense 1.5x while statused", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MILOTIC, ABILITY_MARVEL_SCALE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_MILOTIC) { Ability(ABILITY_COMPETITIVE); Status1(STATUS1_POISON); } // chosen ability differs from the innate
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_EARTHQUAKE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_EARTHQUAKE); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.5), results[0].damage); // +50% Defense -> ~0.67x damage
    }
}

// Quick Feet: +50% Speed while statused. Mightyena's slot-0 ability is Intimidate, so the boost is the
// innate. Poison (not paralysis) isolates the Speed boost from the paralysis-penalty handling. Boosted
// 70 -> 105 outspeeds the opponent's 90; without the innate, 70 would move second.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Quick Feet boosts Speed 1.5x while statused")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MIGHTYENA, ABILITY_QUICK_FEET));
        ASSUME(gSpeciesInfo[SPECIES_MIGHTYENA].abilities[0] != ABILITY_QUICK_FEET);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_MIGHTYENA) { Ability(ABILITY_INTIMIDATE); Status1(STATUS1_POISON); Speed(70); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(90); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Mightyena used Celebrate!");
        MESSAGE("The opposing Wobbuffet used Celebrate!");
    }
}

// Quick Feet also shrugs off the DETERMINISTIC_PARALYSIS PP tax (the fork's paralysis model), exactly like
// the real ability: a paralyzed innate Quick Feet holder's move costs the normal 1 PP (35 - 1 = 34), not 2.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Quick Feet is exempt from the deterministic paralysis PP tax")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MIGHTYENA, ABILITY_QUICK_FEET));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(DETERMINISTIC_PARALYSIS, TRUE);
        PLAYER(SPECIES_MIGHTYENA) { Ability(ABILITY_INTIMIDATE); Status1(STATUS1_PARALYSIS); Moves(MOVE_POUND); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_POUND); }
    } THEN {
        EXPECT_EQ(player->pp[0], 34);
    }
}

// Flare Boost: +50% special-move power while burned. Drifblim's slot-0 ability is Aftermath, so the boost
// is the innate. Drifblim is Ghost/Flying, so Shadow Ball is a neutral special hit on Wobbuffet.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Flare Boost boosts a special move 1.5x while burned", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_DRIFBLIM, ABILITY_FLARE_BOOST));
        ASSUME(gSpeciesInfo[SPECIES_DRIFBLIM].abilities[0] != ABILITY_FLARE_BOOST);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_DRIFBLIM) { Ability(ABILITY_AFTERMATH); Status1(STATUS1_BURN); Moves(MOVE_SHADOW_BALL); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SHADOW_BALL); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.5), results[1].damage);
    }
}

// Toxic Boost: +50% physical power while poisoned. Zangoose carries innate Toxic Boost; its chosen ability
// is forced to Sheer Force (its frontier override slot — not Immunity, which would block the poison) so the
// boost can only be the innate, and Double-Edge has no secondary for Sheer Force to touch. Poison doesn't
// cut physical damage, so the only difference is the +50%.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Toxic Boost boosts a physical move 1.5x while poisoned", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_ZANGOOSE, ABILITY_TOXIC_BOOST));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_ZANGOOSE) { Ability(ABILITY_SHEER_FORCE); Status1(STATUS1_POISON); Moves(MOVE_DOUBLE_EDGE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_DOUBLE_EDGE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.5), results[1].damage);
    }
}

// Suppression parity: Guts is not breakable, so Mold Breaker never touches it, but Gastro Acid suppresses an
// innate Guts exactly like the real ability — the Attack boost vanishes.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Guts", s16 damage)
{
    bool32 gastro;
    PARAMETRIZE { gastro = FALSE; }
    PARAMETRIZE { gastro = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_HARIYAMA, ABILITY_GUTS));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_HARIYAMA) { Ability(ABILITY_THICK_FAT); Status1(STATUS1_POISON); Moves(MOVE_BRICK_BREAK); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID, MOVE_CELEBRATE); }
    } WHEN {
        TURN { if (gastro) MOVE(opponent, MOVE_GASTRO_ACID); else MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_BRICK_BREAK); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.5), results[0].damage); // suppressed: base; active: 1.5x
    }
}

// ===== Batch O: crit-rate / crit-damage modifiers (Super Luck / Sniper / Merciless) =====
// All three are crit-calc modifiers in src/battle_util.c (CalcCritChanceStage / the Gen-1 formula /
// GetAttackerAbilitiesModifier), 1:1 clean-upside copies (no pure-boon divergence). No script / pop-up /
// driver; on-field AI is correct for free (the shared crit calc keys off the real battler).

// Merciless guarantees a crit against a poisoned target. Toxapex is a canon Merciless user whose chosen
// ability here is Limber (not Merciless), so the auto-crit can only come from its innate. With the feature
// off the innate is inert and the test harness's default RNG yields no crit.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Merciless auto-crits a poisoned target")
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_TOXAPEX, ABILITY_MERCILESS));
        ASSUME(gSpeciesInfo[SPECIES_TOXAPEX].abilities[0] != ABILITY_LIMBER);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_TOXAPEX) { Ability(ABILITY_LIMBER); Moves(MOVE_POISON_JAB); }
        OPPONENT(SPECIES_WOBBUFFET) { Status1(STATUS1_POISON); }
    } WHEN {
        TURN { MOVE(player, MOVE_POISON_JAB); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_POISON_JAB, player);
        if (enabled)
            MESSAGE("A critical hit!");
        else
            NOT MESSAGE("A critical hit!");
    }
}

// Mold Breaker does NOT touch Merciless (it isn't breakable) — but suppression parity still holds via
// Gastro Acid, which neutralises the innate so the poisoned target no longer eats a guaranteed crit.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Merciless")
{
    bool32 gastro;
    PARAMETRIZE { gastro = FALSE; }
    PARAMETRIZE { gastro = TRUE; }
    GIVEN {
        ASSUME(!gAbilitiesInfo[ABILITY_MERCILESS].breakable);
        ASSUME(SpeciesHasInnate(SPECIES_TOXAPEX, ABILITY_MERCILESS));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_TOXAPEX) { Ability(ABILITY_LIMBER); Moves(MOVE_POISON_JAB); }
        OPPONENT(SPECIES_WOBBUFFET) { Status1(STATUS1_POISON); Moves(MOVE_GASTRO_ACID, MOVE_CELEBRATE); }
    } WHEN {
        TURN { if (gastro) MOVE(opponent, MOVE_GASTRO_ACID); else MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_POISON_JAB); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_POISON_JAB, player);
        if (gastro)
            NOT MESSAGE("A critical hit!");
        else
            MESSAGE("A critical hit!");
    }
}

// Super Luck adds +1 crit stage. Focus Energy (+2) puts the attacker at stage 2 (1/2, not guaranteed);
// the innate's +1 pushes it to stage 3, which always crits. Absol is a canon Super Luck user whose chosen
// ability here is Pressure, so the extra stage is solely the innate's. Feature off: stage 2, no crit under
// the harness's default RNG; feature on: stage 3, guaranteed crit.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Super Luck adds the crit stage that guarantees a crit")
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_ABSOL, ABILITY_SUPER_LUCK));
        ASSUME(gSpeciesInfo[SPECIES_ABSOL].abilities[0] != ABILITY_SUPER_LUCK);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_ABSOL) { Ability(ABILITY_PRESSURE); Moves(MOVE_SCRATCH, MOVE_FOCUS_ENERGY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_FOCUS_ENERGY); }
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
        if (enabled)
            MESSAGE("A critical hit!");
        else
            NOT MESSAGE("A critical hit!");
    }
}

// Sniper boosts critical-hit DAMAGE (the crit multiplier becomes x2.25 instead of x1.5, i.e. an extra
// x1.5 on a crit). Frost Breath always crits, so the only variable is the Sniper boost. Kingdra is a canon
// Sniper user whose chosen ability here is Swift Swim (it never touches damage), so the boost is solely the
// innate's. Off: the plain crit; on: 1.5x that crit.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Sniper boosts critical-hit damage by 1.5x", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_KINGDRA, ABILITY_SNIPER));
        ASSUME(MoveAlwaysCrits(MOVE_FROST_BREATH));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_KINGDRA) { Ability(ABILITY_SWIFT_SWIM); Moves(MOVE_FROST_BREATH); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_FROST_BREATH); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.5), results[1].damage); // off: plain crit; on: 1.5x
    }
}

// ---- Batch P: accuracy / type-effectiveness / effect-chance modifiers ----
// Shield Dust / Tinted Lens / Scrappy / Wonder Skin / Tangled Feet — all 1:1 clean-upside copies wired
// as calc clauses (src/battle_util.c; Scrappy's Intimidate-immunity half in src/battle_stat_change.c).
// Each test runs the holder on a chosen ability that is NOT the tested one, so the effect is
// attributable solely to the innate.

// Shield Dust blocks the additional effects of moves used against the holder, at the
// IsMoveEffectBlockedByTarget chokepoint. Frosmoth's chosen ability here is Ice Scales (its real
// slot 2), so the block is solely the innate's.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Shield Dust blocks a move's additional effect")
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_FROSMOTH, ABILITY_SHIELD_DUST));
        ASSUME(MoveHasAdditionalEffectWithChance(MOVE_NUZZLE, MOVE_EFFECT_PARALYSIS, 100) == TRUE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_NUZZLE); }
        OPPONENT(SPECIES_FROSMOTH) { Ability(ABILITY_ICE_SCALES); } // chosen Ice Scales, NOT Shield Dust
    } WHEN {
        TURN { MOVE(player, MOVE_NUZZLE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_NUZZLE, player);
        HP_BAR(opponent);
        if (enabled)
            NONE_OF { MESSAGE("The opposing Frosmoth is paralyzed, so it may be unable to move!"); }
        else
            MESSAGE("The opposing Frosmoth is paralyzed, so it may be unable to move!");
    }
}

// Suppression parity: Shield Dust is breakable, so an attacker's Mold Breaker pierces an innate
// Shield Dust exactly as it would the real ability — the secondary lands.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker pierces an innate Shield Dust")
{
    GIVEN {
        ASSUME(gAbilitiesInfo[ABILITY_SHIELD_DUST].breakable);
        ASSUME(SpeciesHasInnate(SPECIES_FROSMOTH, ABILITY_SHIELD_DUST));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_MOLD_BREAKER); Moves(MOVE_NUZZLE); }
        OPPONENT(SPECIES_FROSMOTH) { Ability(ABILITY_ICE_SCALES); }
    } WHEN {
        TURN { MOVE(player, MOVE_NUZZLE); }
    } SCENE {
        MESSAGE("The opposing Frosmoth is paralyzed, so it may be unable to move!");
    }
}

// Tinted Lens doubles the holder's not-very-effective damage (GetAttackerAbilitiesModifier).
// Yanmega's chosen ability here is Speed Boost (its real slot 0, damage-neutral), so the doubling is
// solely the innate's. Bug Buzz into the pure-Fire Entei is resisted (0.5x) -> the innate restores 1x.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Tinted Lens doubles resisted-move damage", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_YANMEGA, ABILITY_TINTED_LENS));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_YANMEGA) { Ability(ABILITY_SPEED_BOOST); Moves(MOVE_BUG_BUZZ); } // chosen Speed Boost, NOT Tinted Lens
        OPPONENT(SPECIES_ENTEI);
    } WHEN {
        TURN { MOVE(player, MOVE_BUG_BUZZ); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(2.0), results[1].damage);
    }
}

// Scrappy lifts the Ghost immunity to Normal/Fighting moves (MulByTypeEffectiveness). Kangaskhan's
// chosen ability here is Inner Focus (its real slot 2), so the Ghost hit is solely the innate's.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Scrappy lets a Normal move hit a Ghost")
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_KANGASKHAN, ABILITY_SCRAPPY));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_KANGASKHAN) { Ability(ABILITY_INNER_FOCUS); Moves(MOVE_SCRATCH); } // chosen Inner Focus, NOT Scrappy
        OPPONENT(SPECIES_GENGAR);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        if (enabled)
        {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
            HP_BAR(opponent);
        }
        else
        {
            MESSAGE("It doesn't affect the opposing Gengar…");
        }
    }
}

// Scrappy's second half: the holder is unaffected by Intimidate (Gen 8+), mirrored for the innate at
// IsIntimidateBlocked (src/battle_stat_change.c) with the pop-up overwritten to Scrappy. Miltank's
// chosen ability here is Thick Fat (its real slot 0, not Intimidate-immune).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Scrappy prevents Intimidate (Gen8+)")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MILTANK, ABILITY_SCRAPPY));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(B_UPDATED_INTIMIDATE, GEN_8);
        PLAYER(SPECIES_MILTANK) { Ability(ABILITY_THICK_FAT); } // chosen Thick Fat, NOT Scrappy
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_EKANS) { Ability(ABILITY_INTIMIDATE); }
    } WHEN {
        TURN { SWITCH(opponent, 1); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_INTIMIDATE);
        ABILITY_POPUP(player, ABILITY_SCRAPPY);
        NONE_OF { ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, player); }
        MESSAGE("Miltank's Attack was not lowered!");
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE);
    }
}

// Wonder Skin caps incoming status moves at 50% accuracy (GetTotalAccuracy). Sigilyph's chosen
// ability here is Magic Guard (its real slot 1 — it blocks the poison CHIP, not the status), so the
// accuracy cap is solely the innate's.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Wonder Skin drops a status move to 50% accuracy")
{
    PASSES_RANDOMLY(1, 2, RNG_ACCURACY); // Toxic 90 acc -> 50 with the innate
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SIGILYPH, ABILITY_WONDER_SKIN));
        ASSUME(GetMoveAccuracy(MOVE_TOXIC) == 90);
        ASSUME(IsBattleMoveStatus(MOVE_TOXIC));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TOXIC); }
        OPPONENT(SPECIES_SIGILYPH) { Ability(ABILITY_MAGIC_GUARD); } // chosen Magic Guard, NOT Wonder Skin
    } WHEN {
        TURN { MOVE(player, MOVE_TOXIC); }
    } SCENE {
        STATUS_ICON(opponent, badPoison: TRUE);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: with the feature off, no innate Wonder Skin accuracy cap (stock behavior)")
{
    PASSES_RANDOMLY(9, 10, RNG_ACCURACY); // no innate -> Toxic stays at its native 90
    GIVEN {
        ASSUME(GetMoveAccuracy(MOVE_TOXIC) == 90);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TOXIC); }
        OPPONENT(SPECIES_SIGILYPH) { Ability(ABILITY_MAGIC_GUARD); }
    } WHEN {
        TURN { MOVE(player, MOVE_TOXIC); }
    } SCENE {
        STATUS_ICON(opponent, badPoison: TRUE);
    }
}

// Under DETERMINISTIC_ACCURACY_EVASION the Wonder Skin accuracy cap manifests as a +1 PP tax on
// incoming STATUS moves (GetDeterministicMoveTargetPPTax), exactly like a real Wonder Skin. Confuse
// Ray is used because its 100% accuracy keeps it out of the economy's OTHER lever — a sub-100-acc
// move's max PP is accuracy-scaled by CalculatePPWithBonus (Toxic at 90 already pays there) — so the
// innate's flat +1 is the only variable.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: under DETERMINISTIC_ACCURACY_EVASION an innate Wonder Skin taxes status-move PP")
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; } // no innate -> base 1 PP only
    PARAMETRIZE { enabled = TRUE; }  // innate Wonder Skin -> +1 PP on the status move
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SIGILYPH, ABILITY_WONDER_SKIN));
        ASSUME(GetMovePP(MOVE_CONFUSE_RAY) == 10);
        ASSUME(GetMoveAccuracy(MOVE_CONFUSE_RAY) == 100);
        ASSUME(IsBattleMoveStatus(MOVE_CONFUSE_RAY));
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CONFUSE_RAY); }
        OPPONENT(SPECIES_SIGILYPH) { Ability(ABILITY_MAGIC_GUARD); } // chosen Magic Guard, NOT Wonder Skin
    } WHEN {
        TURN { MOVE(player, MOVE_CONFUSE_RAY); }
    } THEN {
        EXPECT_EQ(player->pp[0], enabled ? 8 : 9); // 10 - 1 base [- 1 innate Wonder Skin tax]
    }
}

// Shield Dust x DETERMINISTIC_ADDITIONAL_EFFECTS: the config gates secondaries IN on a
// super-effective/STAB hit (Fire Punch is 4x into Ice/Bug Frosmoth, so its burn is guaranteed) —
// and the innate Shield Dust still blocks the gated-in effect at the same chokepoint.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Shield Dust blocks a gated-in effect under DETERMINISTIC_ADDITIONAL_EFFECTS")
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; } // no innate -> the gated-in burn lands
    PARAMETRIZE { enabled = TRUE; }  // innate Shield Dust -> blocked
    GIVEN {
        ASSUME(MoveHasAdditionalEffect(MOVE_FIRE_PUNCH, MOVE_EFFECT_BURN));
        ASSUME(SpeciesHasInnate(SPECIES_FROSMOTH, ABILITY_SHIELD_DUST));
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_FIRE_PUNCH); }
        OPPONENT(SPECIES_FROSMOTH) { Ability(ABILITY_ICE_SCALES); MaxHP(600); HP(600); Defense(255); } // chosen Ice Scales, NOT Shield Dust
    } WHEN {
        TURN { MOVE(player, MOVE_FIRE_PUNCH); }
    } THEN {
        if (enabled)
            EXPECT(!(opponent->status1 & STATUS1_BURN));
        else
            EXPECT(opponent->status1 & STATUS1_BURN);
    }
}

// Shield Dust x DETERMINISTIC_HOLD_EFFECTS: King's Rock guarantees its flinch on the first
// qualifying hit and is consumed when it fires — but Shield Dust blocks the flinch at the
// SetMoveEffect chokepoint, so the deterministic would-it-land mirror (battle_hold_effects.c)
// must treat a Shield Dust (real or innate) target as a non-landing flinch and spare the rock.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: under DETERMINISTIC_HOLD_EFFECTS an innate Shield Dust target spares the King's Rock")
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; } // no innate -> the guaranteed flinch fires and the rock is consumed
    PARAMETRIZE { enabled = TRUE; }  // innate Shield Dust -> no flinch, rock kept
    GIVEN {
        ASSUME(gItemsInfo[ITEM_KINGS_ROCK].holdEffect == HOLD_EFFECT_FLINCH);
        ASSUME(SpeciesHasInnate(SPECIES_FROSMOTH, ABILITY_SHIELD_DUST));
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Item(ITEM_KINGS_ROCK); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_FROSMOTH) { Speed(50); Ability(ABILITY_ICE_SCALES); MaxHP(600); HP(600); Defense(255); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        if (enabled)
            NONE_OF { MESSAGE("The opposing Frosmoth flinched and couldn't move!"); }
        else
            MESSAGE("The opposing Frosmoth flinched and couldn't move!");
    } THEN {
        EXPECT_EQ(player->item, enabled ? ITEM_KINGS_ROCK : ITEM_NONE);
    }
}

// Tangled Feet doubles the holder's evasion while it is confused (GetTotalAccuracy). Spinda's chosen
// ability here is Contrary (its real slot 2 — Own Tempo, slot 0, would block the confusion the innate
// needs). The WITH_RNG(RNG_CONFUSION, FALSE) keeps the confused holder from hitting itself so the
// scripted turns stay deterministic.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Tangled Feet doubles evasion while confused")
{
    PASSES_RANDOMLY(1, 2, RNG_ACCURACY); // Pound 100 acc -> 50 vs the confused innate holder
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SPINDA, ABILITY_TANGLED_FEET));
        ASSUME(GetMoveAccuracy(MOVE_POUND) == 100);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_POUND, MOVE_CONFUSE_RAY); }
        OPPONENT(SPECIES_SPINDA) { Speed(50); Ability(ABILITY_CONTRARY); Moves(MOVE_CELEBRATE); } // chosen Contrary, NOT Tangled Feet
    } WHEN {
        TURN { MOVE(player, MOVE_CONFUSE_RAY); MOVE(opponent, MOVE_CELEBRATE, WITH_RNG(RNG_CONFUSION, FALSE)); }
        TURN { MOVE(player, MOVE_POUND); MOVE(opponent, MOVE_CELEBRATE, WITH_RNG(RNG_CONFUSION, FALSE)); }
    } SCENE {
        HP_BAR(opponent);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: with the feature off, no innate Tangled Feet evasion (stock behavior)")
{
    PASSES_RANDOMLY(2, 2, RNG_ACCURACY); // no innate -> Pound always hits the confused target
    GIVEN {
        ASSUME(GetMoveAccuracy(MOVE_POUND) == 100);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_POUND, MOVE_CONFUSE_RAY); }
        OPPONENT(SPECIES_SPINDA) { Speed(50); Ability(ABILITY_CONTRARY); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CONFUSE_RAY); MOVE(opponent, MOVE_CELEBRATE, WITH_RNG(RNG_CONFUSION, FALSE)); }
        TURN { MOVE(player, MOVE_POUND); MOVE(opponent, MOVE_CELEBRATE, WITH_RNG(RNG_CONFUSION, FALSE)); }
    } SCENE {
        HP_BAR(opponent);
    }
}

// Under DETERMINISTIC_ACCURACY_EVASION the Tangled Feet evasion manifests as a +1 PP tax on incoming
// offensive moves while the holder is confused (GetDeterministicMoveTargetPPTax).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: under DETERMINISTIC_ACCURACY_EVASION an innate Tangled Feet taxes the attacker's PP")
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; } // no innate -> base 1 PP only
    PARAMETRIZE { enabled = TRUE; }  // innate Tangled Feet while confused -> +1 PP
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SPINDA, ABILITY_TANGLED_FEET));
        ASSUME(GetMovePP(MOVE_POUND) == 35);
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_POUND, MOVE_CONFUSE_RAY); }
        OPPONENT(SPECIES_SPINDA) { Speed(50); Ability(ABILITY_CONTRARY); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CONFUSE_RAY); MOVE(opponent, MOVE_CELEBRATE, WITH_RNG(RNG_CONFUSION, FALSE)); }
        TURN { MOVE(player, MOVE_POUND); MOVE(opponent, MOVE_CELEBRATE, WITH_RNG(RNG_CONFUSION, FALSE)); }
    } THEN {
        EXPECT_EQ(player->pp[0], enabled ? 33 : 34); // 35 - 1 base [- 1 innate Tangled Feet tax]
    }
}

// ===== Batch C: physical-Attack doublers (Huge Power / Pure Power) =====

// Huge Power doubles the holder's physical Attack. Azumarill's chosen ability is Thick Fat, so the
// x2 here is purely the innate Huge Power.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Huge Power doubles physical damage", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_AZUMARILL, ABILITY_HUGE_POWER));
        ASSUME(gSpeciesInfo[SPECIES_AZUMARILL].abilities[0] != ABILITY_HUGE_POWER);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_AZUMARILL) { Moves(MOVE_TACKLE); } // chosen Thick Fat
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(2.0), results[1].damage);
    }
}

// Huge Power is physical-only: it must NOT touch a special move's damage.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Huge Power leaves special damage unchanged", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_AZUMARILL, ABILITY_HUGE_POWER));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_AZUMARILL) { Moves(MOVE_SWIFT); } // Normal special; chosen Thick Fat
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SWIFT); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_EQ(results[0].damage, results[1].damage); // special damage identical with or without the innate
    }
}

// Pure Power doubles the holder's physical Attack, identically to Huge Power. Medicham's chosen ability
// here is Telepathy (a real slot), so the x2 is purely the innate Pure Power.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Pure Power doubles physical damage", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MEDICHAM, ABILITY_PURE_POWER));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_MEDICHAM) { Ability(ABILITY_TELEPATHY); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(2.0), results[1].damage);
    }
}

// Suppression parity: Huge Power is NOT breakable, so Mold Breaker can't pierce it — Gastro Acid is the
// relevant suppressor, same as the real ability. With the innate nullified, the x2 is gone.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Huge Power", s16 damage)
{
    bool32 gastro;
    PARAMETRIZE { gastro = FALSE; }
    PARAMETRIZE { gastro = TRUE; }
    GIVEN {
        ASSUME(!gAbilitiesInfo[ABILITY_HUGE_POWER].breakable);
        ASSUME(SpeciesHasInnate(SPECIES_AZUMARILL, ABILITY_HUGE_POWER));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_AZUMARILL) { Moves(MOVE_TACKLE); } // chosen Thick Fat
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID, MOVE_CELEBRATE); }
    } WHEN {
        TURN { if (gastro) MOVE(opponent, MOVE_GASTRO_ACID); else MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_TACKLE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(2.0), results[0].damage); // suppressed: base; active: x2
    }
}

// ===== Batch D+E: stat-drop protection (Clear Body / White Smoke / Hyper Cutter / Big Pecks) =====

// Clear Body / White Smoke block ANY stat drop from another mon; Hyper Cutter protects Attack, Big Pecks
// Defense. Each test uses a canon user whose CHOSEN ability differs, so the effect is purely the innate.
// The pop-up/message show the innate, not the chosen ability (the Keen Eye / Sturdy overwrite precedent).

// Clear Body: an incoming Growl (Attack -1) is fully blocked; the pop-up shows Clear Body, not the
// chosen Liquid Ooze. With the feature off it does nothing and the Attack drops (stock behavior).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Clear Body blocks a stat drop")
{
    u32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_TENTACRUEL, ABILITY_CLEAR_BODY));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_TENTACRUEL) { Ability(ABILITY_LIQUID_OOZE); } // chosen Liquid Ooze, NOT Clear Body
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GROWL); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_GROWL); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_CLEAR_BODY);
            MESSAGE("Tentacruel's stats were not lowered!");
        }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], enabled ? DEFAULT_STAT_STAGE : DEFAULT_STAT_STAGE - 1);
    }
}

// White Smoke behaves identically to Clear Body (full protection). Heatmor's chosen ability is Flash
// Fire (a real slot), so the block is purely the innate White Smoke.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate White Smoke blocks a stat drop")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_HEATMOR, ABILITY_WHITE_SMOKE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_HEATMOR) { Ability(ABILITY_FLASH_FIRE); } // chosen Flash Fire, NOT White Smoke
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GROWL); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_GROWL); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_WHITE_SMOKE);
        MESSAGE("Heatmor's stats were not lowered!");
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE);
    }
}

// Hyper Cutter protects ONLY Attack: an Attack drop is blocked (pop-up Hyper Cutter), but a Defense drop
// (Tail Whip) still lands. Pinsir's chosen ability is Moxie (a real slot), so the block is the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Hyper Cutter blocks only Attack drops")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_PINSIR, ABILITY_HYPER_CUTTER));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_PINSIR) { Ability(ABILITY_MOXIE); } // chosen Moxie, NOT Hyper Cutter
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GROWL, MOVE_TAIL_WHIP); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_GROWL); }     // Attack -1: blocked
        TURN { MOVE(opponent, MOVE_TAIL_WHIP); } // Defense -1: lands
    } SCENE {
        ABILITY_POPUP(player, ABILITY_HYPER_CUTTER);
        MESSAGE("Pinsir's Attack was not lowered!");
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE);     // Attack protected
        EXPECT_EQ(player->statStages[STAT_DEF], DEFAULT_STAT_STAGE - 1); // Defense not
    }
}

// Big Pecks protects ONLY Defense: a Defense drop (Tail Whip) is blocked (pop-up Big Pecks), but an
// Attack drop (Growl) still lands. Mandibuzz's chosen ability is Overcoat (a real slot).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Big Pecks blocks only Defense drops")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MANDIBUZZ, ABILITY_BIG_PECKS));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_MANDIBUZZ) { Ability(ABILITY_OVERCOAT); } // chosen Overcoat, NOT Big Pecks
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GROWL, MOVE_TAIL_WHIP); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_TAIL_WHIP); } // Defense -1: blocked
        TURN { MOVE(opponent, MOVE_GROWL); }     // Attack -1: lands
    } SCENE {
        ABILITY_POPUP(player, ABILITY_BIG_PECKS);
        MESSAGE("Mandibuzz's Defense was not lowered!");
    } THEN {
        EXPECT_EQ(player->statStages[STAT_DEF], DEFAULT_STAT_STAGE);     // Defense protected
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE - 1); // Attack not
    }
}

// Suppression parity: Clear Body is breakable, so an attacker's Mold Breaker pierces the innate exactly
// as it would the real ability (the stat drop lands, no pop-up).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker pierces an innate Clear Body")
{
    GIVEN {
        ASSUME(gAbilitiesInfo[ABILITY_CLEAR_BODY].breakable);
        ASSUME(SpeciesHasInnate(SPECIES_TENTACRUEL, ABILITY_CLEAR_BODY));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_TENTACRUEL) { Ability(ABILITY_LIQUID_OOZE); } // innate Clear Body
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MOLD_BREAKER); Moves(MOVE_GROWL); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_GROWL); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(player, ABILITY_CLEAR_BODY); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE - 1); // pierced -> Attack drops
    }
}

// Suppression parity: Gastro Acid suppresses an innate Hyper Cutter (Gastro-Acid / Neutralizing-Gas
// parity), so the Attack drop lands.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Hyper Cutter")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_PINSIR, ABILITY_HYPER_CUTTER));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_PINSIR) { Ability(ABILITY_MOXIE); } // innate Hyper Cutter
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID, MOVE_GROWL); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_GASTRO_ACID); }
        TURN { MOVE(opponent, MOVE_GROWL); }
    } SCENE {
        MESSAGE("Pinsir's Attack fell!");
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE - 1); // suppressed -> Attack drops
    }
}

// FORK: Batch G — redirection-ignore innates (Propeller Tail / Stalwart). The holder's moves ignore
// Follow Me / Rage Powder and Lightning Rod / Storm Drain redirection, hitting the originally-selected
// target. Wired at the shared redirection sites in src/battle_move_resolution.c (IsAffectedByFollowMe +
// the Lightning-Rod/Storm-Drain redirect loop) beside the chosen-ability tests. Both are 1:1 clean-upside
// copies (redirection-ignore never hurts the holder).
DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Stalwart ignores Follow Me redirection")
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_ARCHALUDON, ABILITY_STALWART));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_ARCHALUDON) { Ability(ABILITY_STAMINA); } // chosen Stamina, NOT Stalwart
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponentLeft, MOVE_FOLLOW_ME); MOVE(playerLeft, MOVE_DRACO_METEOR, target: opponentRight); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_FOLLOW_ME, opponentLeft);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRACO_METEOR, playerLeft);
        if (enabled) {
            HP_BAR(opponentRight); // innate Stalwart -> the move ignores Follow Me and hits the selected target
            NOT HP_BAR(opponentLeft);
        } else {
            HP_BAR(opponentLeft); // no innate -> Follow Me redirects the move
            NOT HP_BAR(opponentRight);
        }
    }
}

DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Propeller Tail ignores Lightning Rod redirection")
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_BARRASKEWDA, ABILITY_PROPELLER_TAIL));
        ASSUME(GetMoveType(MOVE_SPARK) == TYPE_ELECTRIC);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        WITH_CONFIG(B_REDIRECT_ABILITY_IMMUNITY, GEN_5);
        PLAYER(SPECIES_BARRASKEWDA) { Ability(ABILITY_SWIFT_SWIM); Moves(MOVE_SPARK); } // chosen Swift Swim, NOT Propeller Tail
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_RAICHU) { Ability(ABILITY_LIGHTNING_ROD); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_SPARK, target: opponentRight); }
    } SCENE {
        if (enabled) {
            HP_BAR(opponentRight); // innate Propeller Tail -> Spark ignores Lightning Rod and hits the selected target
            NONE_OF { ABILITY_POPUP(opponentLeft, ABILITY_LIGHTNING_ROD); }
        } else {
            ABILITY_POPUP(opponentLeft, ABILITY_LIGHTNING_ROD); // no innate -> redirected and absorbed
            NOT HP_BAR(opponentRight);
        }
    }
}

// Suppression parity: an innate Stalwart honors Gastro Acid exactly like the real ability, so a
// suppressed holder's moves are redirected again.
DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Stalwart")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_ARCHALUDON, ABILITY_STALWART));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_ARCHALUDON) { Ability(ABILITY_STAMINA); } // chosen Stamina, NOT Stalwart
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID, MOVE_FOLLOW_ME); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponentLeft, MOVE_GASTRO_ACID, target: playerLeft); }
        TURN { MOVE(opponentLeft, MOVE_FOLLOW_ME); MOVE(playerLeft, MOVE_DRACO_METEOR, target: opponentRight); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet used Gastro Acid!");
        HP_BAR(opponentLeft);      // innate suppressed -> Follow Me redirects the move
        NOT HP_BAR(opponentRight);
    }
}

// ==========================================================================================
// Batch H — trapping (Shadow Tag / Arena Trap / Magnet Pull). Each keeps opposing mons from
// switching out, wired at the single chokepoint IsAbilityPreventingEscape (src/battle_util.c)
// via BattlerHasAbility. The tests assert the mechanic directly (like the upstream trapping
// tests) so they don't depend on the escape/switch message text.
// ==========================================================================================

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Shadow Tag prevents the opponent from escaping")
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_WOBBUFFET, ABILITY_SHADOW_TAG));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_ZIGZAGOON); // ordinary grounded mon, no Shadow Tag of its own
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_TELEPATHY); } // chosen Telepathy, NOT Shadow Tag
    } WHEN {
        TURN { MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId trapper = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        if (enabled)
            EXPECT_EQ(IsAbilityPreventingEscape(battler), trapper + 1); // innate Shadow Tag traps
        else
            EXPECT_EQ(IsAbilityPreventingEscape(battler), 0); // feature off -> no trap
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Arena Trap traps grounded foes but not airborne ones")
{
    u32 species;
    bool32 grounded;
    PARAMETRIZE { species = SPECIES_ZIGZAGOON; grounded = TRUE; }
    PARAMETRIZE { species = SPECIES_PIDGEY; grounded = FALSE; } // Flying -> ungrounded
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_DIGLETT, ABILITY_ARENA_TRAP));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(species);
        OPPONENT(SPECIES_DIGLETT) { Ability(ABILITY_SAND_VEIL); } // chosen Sand Veil, NOT Arena Trap
    } WHEN {
        TURN { MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId trapper = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        if (grounded)
            EXPECT_EQ(IsAbilityPreventingEscape(battler), trapper + 1); // grounded -> trapped
        else
            EXPECT_EQ(IsAbilityPreventingEscape(battler), 0); // airborne -> Arena Trap can't hold it
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Magnet Pull traps Steel-types but not others")
{
    u32 species;
    bool32 steel;
    PARAMETRIZE { species = SPECIES_SKARMORY; steel = TRUE; } // Steel (and Flying: Magnet Pull ignores grounding)
    PARAMETRIZE { species = SPECIES_ZIGZAGOON; steel = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MAGNEMITE, ABILITY_MAGNET_PULL));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(species);
        OPPONENT(SPECIES_MAGNEMITE) { Ability(ABILITY_STURDY); } // chosen Sturdy, NOT Magnet Pull
    } WHEN {
        TURN { MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId trapper = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        if (steel)
            EXPECT_EQ(IsAbilityPreventingEscape(battler), trapper + 1); // Steel-type -> trapped
        else
            EXPECT_EQ(IsAbilityPreventingEscape(battler), 0); // non-Steel -> free to leave
    }
}

// Mutual-exemption is innate-aware too: a mon carrying its OWN innate Shadow Tag is not trapped by
// an enemy Shadow Tag (B_SHADOW_TAG_ESCAPE >= GEN_4), exactly like the real ability.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: an innate Shadow Tag holder is exempt from enemy Shadow Tag")
{
    GIVEN {
        ASSUME(B_SHADOW_TAG_ESCAPE >= GEN_4);
        ASSUME(SpeciesHasInnate(SPECIES_WOBBUFFET, ABILITY_SHADOW_TAG));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_TELEPATHY); }   // chosen Telepathy + innate Shadow Tag
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_TELEPATHY); } // chosen Telepathy + innate Shadow Tag
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        EXPECT_EQ(IsAbilityPreventingEscape(battler), 0); // its own innate Shadow Tag lets it leave
    }
}

// Suppression parity: an innate trapper honors Gastro Acid exactly like the real ability.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Shadow Tag")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_WOBBUFFET, ABILITY_SHADOW_TAG));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_GASTRO_ACID); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_TELEPATHY); } // chosen Telepathy + innate Shadow Tag
    } WHEN {
        TURN { MOVE(player, MOVE_GASTRO_ACID); }
    } THEN {
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        EXPECT_EQ(IsAbilityPreventingEscape(battler), 0); // innate suppressed -> no trap
    }
}

// FORK: table-integrity guards for the sSpeciesInnates table (src/fork/innate_abilities.c).
// These are pure data-lookup tests (no battle), walking the raw rows via the
// GetSpeciesInnatesEntry* accessors so even a duplicate species row stays visible.

// (1) Every declared innate must be on the implemented allowlist — an ability whose innate
// behavior is actually wired at an effect site. An off-allowlist innate has no effect site
// to honor it, so it would silently do NOTHING (the footgun the file header warns about);
// this fails loudly instead. Keep this set in sync with the ALLOWLIST note in
// src/fork/innate_abilities.c when a new ability is wired.
TEST("Innate abilities: every declared innate is on the implemented allowlist")
{
    static const enum Ability sImplementedInnates[] =
    {
        ABILITY_LEVITATE, ABILITY_REGENERATOR, ABILITY_UNAWARE, ABILITY_STURDY,
        ABILITY_NATURAL_CURE, ABILITY_PRANKSTER, ABILITY_OVERGROW, ABILITY_BLAZE,
        ABILITY_TORRENT, ABILITY_SWARM, ABILITY_SWIFT_SWIM, ABILITY_CHLOROPHYLL,
        ABILITY_SAND_RUSH, ABILITY_SLUSH_RUSH, ABILITY_FILTER, ABILITY_PRESSURE,
        ABILITY_STENCH, ABILITY_BATTLE_ARMOR, ABILITY_SHELL_ARMOR, ABILITY_SPEED_BOOST,
        ABILITY_LIMBER, ABILITY_CUTE_CHARM, ABILITY_OBLIVIOUS, ABILITY_SAND_VEIL,
        ABILITY_SNOW_CLOAK, ABILITY_COMPOUND_EYES, ABILITY_KEEN_EYE, ABILITY_ILLUMINATE,
        ABILITY_INSOMNIA, ABILITY_VITAL_SPIRIT, ABILITY_SWEET_VEIL, ABILITY_EARLY_BIRD,
        ABILITY_IMMUNITY, ABILITY_PASTEL_VEIL, ABILITY_THICK_FAT, ABILITY_TECHNICIAN,
        ABILITY_IRON_FIST, ABILITY_RECKLESS, ABILITY_STRONG_JAW, ABILITY_TOUGH_CLAWS,
        ABILITY_SHARPNESS, ABILITY_MEGA_LAUNCHER, ABILITY_STEELWORKER, ABILITY_STEELY_SPIRIT,
        ABILITY_ROCKY_PAYLOAD, ABILITY_SAND_FORCE, ABILITY_ANALYTIC, ABILITY_ADAPTABILITY,
        ABILITY_PUNK_ROCK, ABILITY_STAKEOUT, ABILITY_SERENE_GRACE,
        ABILITY_MULTISCALE, ABILITY_SOLID_ROCK, ABILITY_FUR_COAT, ABILITY_ICE_SCALES,
        ABILITY_HEATPROOF, ABILITY_FRIEND_GUARD, ABILITY_WATER_BUBBLE,
        ABILITY_GUTS, ABILITY_MARVEL_SCALE, ABILITY_QUICK_FEET, ABILITY_TOXIC_BOOST,
        ABILITY_FLARE_BOOST,
        ABILITY_SUPER_LUCK, ABILITY_SNIPER, ABILITY_MERCILESS,
        ABILITY_SHIELD_DUST, ABILITY_TINTED_LENS, ABILITY_SCRAPPY, ABILITY_WONDER_SKIN,
        ABILITY_TANGLED_FEET,
        ABILITY_GALE_WINGS, ABILITY_TRIAGE,
        ABILITY_SURGE_SURFER, ABILITY_GRASS_PELT,
        ABILITY_HUGE_POWER, ABILITY_PURE_POWER,
        ABILITY_CLEAR_BODY, ABILITY_WHITE_SMOKE, ABILITY_HYPER_CUTTER, ABILITY_BIG_PECKS,
        ABILITY_DAZZLING, ABILITY_QUEENLY_MAJESTY, ABILITY_ARMOR_TAIL,
        ABILITY_PROPELLER_TAIL, ABILITY_STALWART,
        ABILITY_SHADOW_TAG, ABILITY_ARENA_TRAP, ABILITY_MAGNET_PULL,
        ABILITY_MAGMA_ARMOR, ABILITY_WATER_VEIL, ABILITY_OWN_TEMPO, ABILITY_INNER_FOCUS,
        ABILITY_LEAF_GUARD, ABILITY_OVERCOAT,
        ABILITY_SUCTION_CUPS, ABILITY_GUARD_DOG, ABILITY_ROCK_HEAD, ABILITY_LONG_REACH,
        ABILITY_SKILL_LINK, ABILITY_INFILTRATOR, ABILITY_CORROSION, ABILITY_STICKY_HOLD,
        ABILITY_UNSEEN_FIST, ABILITY_PIERCING_DRILL, ABILITY_HEAVY_METAL, ABILITY_LIGHT_METAL,
        ABILITY_RAIN_DISH, ABILITY_ICE_BODY, ABILITY_SHED_SKIN, ABILITY_HYDRATION,
        ABILITY_HEALER, ABILITY_HARVEST, ABILITY_CUD_CHEW, ABILITY_PICKUP,
        ABILITY_BAD_DREAMS, ABILITY_POISON_HEAL,
        ABILITY_GLUTTONY, ABILITY_RIPEN, ABILITY_CHEEK_POUCH, ABILITY_UNBURDEN,
        ABILITY_ROUGH_SKIN, ABILITY_IRON_BARBS, ABILITY_GOOEY, ABILITY_TANGLING_HAIR,
        ABILITY_AFTERMATH, ABILITY_INNARDS_OUT,
        ABILITY_STEAM_ENGINE, ABILITY_THERMAL_EXCHANGE, ABILITY_WIND_POWER,
        ABILITY_CURSED_BODY,
        ABILITY_PICKPOCKET, ABILITY_MAGICIAN, ABILITY_LIQUID_OOZE,
        ABILITY_INTIMIDATE,
        ABILITY_ANTICIPATION, ABILITY_FOREWARN, ABILITY_FRISK,
        ABILITY_DOWNLOAD, ABILITY_SUPERSWEET_SYRUP,
        ABILITY_UNNERVE, ABILITY_HOSPITALITY,
        ABILITY_DEFIANT, ABILITY_COMPETITIVE,
        ABILITY_JUSTIFIED, ABILITY_STAMINA, ABILITY_WATER_COMPACTION, ABILITY_ANGER_POINT,
        ABILITY_RATTLED, ABILITY_STEADFAST,
        ABILITY_MOXIE, ABILITY_BERSERK, ABILITY_SOUL_HEART,
        ABILITY_BATTERY, ABILITY_POWER_SPOT, ABILITY_TELEPATHY, ABILITY_AROMA_VEIL, ABILITY_FLOWER_VEIL,
        ABILITY_CHILLING_NEIGH, ABILITY_GRIM_NEIGH, ABILITY_ELECTROMORPHOSIS,
        ABILITY_TRANSISTOR, ABILITY_DRAGONS_MAW,
        ABILITY_PRISM_ARMOR, ABILITY_SHADOW_SHIELD, ABILITY_NEUROFORCE, ABILITY_SUPREME_OVERLORD,
        ABILITY_FULL_METAL_BODY, ABILITY_MINDS_EYE,
        ABILITY_PURIFYING_SALT, ABILITY_GOOD_AS_GOLD,
        ABILITY_INTREPID_SWORD, ABILITY_DAUNTLESS_SHIELD,
        ABILITY_BEAST_BOOST,
        ABILITY_MEGA_SOL,
        ABILITY_QUICK_DRAW,
        ABILITY_COMATOSE,
        ABILITY_MAGIC_GUARD,
        ABILITY_MOLD_BREAKER,
        ABILITY_TERAVOLT, ABILITY_TURBOBLAZE,
        ABILITY_OPPORTUNIST,
        ABILITY_MIRROR_ARMOR,
    };
    u32 row, i, j, count = GetSpeciesInnatesEntryCount();
    u32 offenders = 0;

    for (row = 0; row < count; row++)
    {
        u16 species;
        const enum Ability *list = GetSpeciesInnatesEntry(row, &species);

        for (i = 0; list[i] != ABILITY_NONE; i++)
        {
            bool32 allowed = FALSE;
            for (j = 0; j < ARRAY_COUNT(sImplementedInnates); j++)
            {
                if (list[i] == sImplementedInnates[j])
                {
                    allowed = TRUE;
                    break;
                }
            }
            if (!allowed)
            {
                offenders++;
                Test_MgbaPrintf("%S declares unwired innate %S (no effect site honors it)",
                                gSpeciesInfo[species].speciesName, gAbilitiesInfo[list[i]].name);
            }
        }
    }

    EXPECT_EQ(offenders, 0);
}

// (2) No species may appear in the table more than once. GetSpeciesInnateList returns the
// FIRST matching row, so a second row for the same species would be silently dead data.
TEST("Innate abilities: no species appears in the table more than once")
{
    u32 a, b, count = GetSpeciesInnatesEntryCount();
    u32 dups = 0;

    for (a = 0; a < count; a++)
    {
        u16 spA;
        GetSpeciesInnatesEntry(a, &spA);
        for (b = a + 1; b < count; b++)
        {
            u16 spB;
            GetSpeciesInnatesEntry(b, &spB);
            if (spA == spB)
            {
                dups++;
                Test_MgbaPrintf("%S is listed more than once (rows %d and %d)",
                                gSpeciesInfo[spA].speciesName, a, b);
            }
        }
    }

    EXPECT_EQ(dups, 0);
}

// (3) No species may list the same innate twice within its own list (a copy-paste slip).
TEST("Innate abilities: no species lists the same innate twice")
{
    u32 row, i, j, count = GetSpeciesInnatesEntryCount();
    u32 dups = 0;

    for (row = 0; row < count; row++)
    {
        u16 species;
        const enum Ability *list = GetSpeciesInnatesEntry(row, &species);

        for (i = 0; list[i] != ABILITY_NONE; i++)
        {
            for (j = i + 1; list[j] != ABILITY_NONE; j++)
            {
                if (list[i] == list[j])
                {
                    dups++;
                    Test_MgbaPrintf("%S lists innate %S more than once",
                                    gSpeciesInfo[species].speciesName, gAbilitiesInfo[list[i]].name);
                }
            }
        }
    }

    EXPECT_EQ(dups, 0);
}

// (4) The species-keyed lookups must agree with a raw linear walk of the table for every
// row. GetSpeciesInnateList binary-searches a lazily built species-sorted row index
// (src/fork/innate_abilities.c); a bug there (index unsorted, off-by-one bounds, stale
// build flag) would silently drop or misroute innates, so this cross-checks every row's
// full list through SpeciesHasInnate/GetSpeciesInnate, plus a no-row miss.
TEST("Innate abilities: species-keyed lookup matches the raw table for every row")
{
    u32 row, i, count = GetSpeciesInnatesEntryCount();
    u32 mismatches = 0;

    for (row = 0; row < count; row++)
    {
        u16 species;
        const enum Ability *list = GetSpeciesInnatesEntry(row, &species);

        for (i = 0; list[i] != ABILITY_NONE; i++)
        {
            if (!SpeciesHasInnate(species, list[i]) || GetSpeciesInnate(species, i) != list[i])
            {
                mismatches++;
                Test_MgbaPrintf("%S: keyed lookup disagrees with raw row for %S",
                                gSpeciesInfo[species].speciesName, gAbilitiesInfo[list[i]].name);
            }
        }
        if (GetSpeciesInnate(species, i) != ABILITY_NONE) // list must end exactly where the raw row ends
            mismatches++;
    }

    EXPECT_EQ(mismatches, 0);
    EXPECT(!SpeciesHasInnate(SPECIES_NONE, ABILITY_LEVITATE)); // no-row species misses cleanly
    EXPECT_EQ(GetSpeciesInnate(SPECIES_NONE, 0), ABILITY_NONE);
}

// ===== Batch I — status-condition immunities (Magma Armor / Water Veil / Own Tempo /
// Inner Focus / Leaf Guard / Overcoat) =====
// All 1:1 clean-upside copies of the real ability, wired at the shared trait chokepoints
// (CanSetNonVolatileStatus / CanBeConfused / IsLeafGuardProtected / IsAffectedByPowderMove /
// the sandstorm-hail end-turn block). Each vehicle carries the ability innately but picks a
// DIFFERENT chosen ability so only the innate can be responsible for the effect.

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Magma Armor prevents freeze/frostbite")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(MoveHasAdditionalEffect(MOVE_POWDER_SNOW, MOVE_EFFECT_FREEZE_OR_FROSTBITE));
        ASSUME(SpeciesHasInnate(SPECIES_CAMERUPT, ABILITY_MAGMA_ARMOR));
        ASSUME(gSpeciesInfo[SPECIES_CAMERUPT].abilities[0] != ABILITY_SOLID_ROCK);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_CAMERUPT) { Ability(ABILITY_SOLID_ROCK); } // chosen differs from the innate Magma Armor
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_POWDER_SNOW); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_POWDER_SNOW); }
    } THEN {
        if (enabled)
            EXPECT(!(player->status1 & STATUS1_ICY_ANY)); // innate Magma Armor -> never frozen/frostbitten
        else
            EXPECT(player->status1 & STATUS1_ICY_ANY);     // no innate -> the forced freeze/frostbite lands
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Water Veil prevents burn")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(GetMoveNonVolatileStatus(MOVE_WILL_O_WISP) == MOVE_EFFECT_BURN);
        ASSUME(SpeciesHasInnate(SPECIES_WAILORD, ABILITY_WATER_VEIL));
        ASSUME(gSpeciesInfo[SPECIES_WAILORD].abilities[0] != ABILITY_OBLIVIOUS);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WAILORD) { Ability(ABILITY_OBLIVIOUS); } // chosen differs from the innate Water Veil
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_WILL_O_WISP); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_WILL_O_WISP); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_WATER_VEIL); // pop-up shows Water Veil, not the chosen Oblivious
            NONE_OF { STATUS_ICON(player, burn: TRUE); }
        } else {
            STATUS_ICON(player, burn: TRUE);
        }
    } THEN {
        if (enabled)
            EXPECT_EQ(player->status1 & STATUS1_BURN, 0);
    }
}

// Suppression parity: Water Veil is breakable, so Mold Breaker pierces an innate Water Veil.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker pierces an innate Water Veil")
{
    GIVEN {
        ASSUME(gAbilitiesInfo[ABILITY_WATER_VEIL].breakable);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WAILORD) { Ability(ABILITY_OBLIVIOUS); } // innate Water Veil
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MOLD_BREAKER); Moves(MOVE_WILL_O_WISP); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_WILL_O_WISP); }
    } SCENE {
        STATUS_ICON(player, burn: TRUE); // Mold Breaker ignores the innate -> burned
        NONE_OF { ABILITY_POPUP(player, ABILITY_WATER_VEIL); }
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Own Tempo prevents confusion")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_CONFUSE_RAY) == EFFECT_CONFUSE);
        ASSUME(SpeciesHasInnate(SPECIES_SLOWBRO, ABILITY_OWN_TEMPO));
        ASSUME(gSpeciesInfo[SPECIES_SLOWBRO].abilities[0] != ABILITY_REGENERATOR);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_SLOWBRO) { Ability(ABILITY_REGENERATOR); } // chosen differs from the innate Own Tempo
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CONFUSE_RAY); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CONFUSE_RAY); }
    } THEN {
        if (enabled)
            EXPECT(player->volatiles.confusionTurns == 0);
        else
            EXPECT(player->volatiles.confusionTurns > 0);
    }
}

// The switch-in cure site fires for an innate Own Tempo too: if a Mold Breaker Confuse Ray pierces the
// innate and confuses the holder, the confusion is cured the moment the immunity hook next runs (Mold
// Breaker is no longer active), exactly like a real Own Tempo. Grumpig is the vehicle (no innate Oblivious).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Own Tempo cures confusion inflicted through Mold Breaker")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_CONFUSE_RAY) == EFFECT_CONFUSE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_PINSIR) { Ability(ABILITY_MOLD_BREAKER); }
        OPPONENT(SPECIES_GRUMPIG) { Ability(ABILITY_GLUTTONY); } // innate Own Tempo
    } WHEN {
        TURN { MOVE(player, MOVE_CONFUSE_RAY); }
    } SCENE {
        MESSAGE("The opposing Grumpig became confused!"); // Mold Breaker pierced the innate
        ABILITY_POPUP(opponent, ABILITY_OWN_TEMPO);
        MESSAGE("The opposing Grumpig snapped out of its confusion!"); // innate cures right after
    } THEN {
        EXPECT(opponent->volatiles.confusionTurns == 0);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Inner Focus prevents flinching")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        ASSUME(SpeciesHasInnate(SPECIES_LUCARIO, ABILITY_INNER_FOCUS));
        ASSUME(gSpeciesInfo[SPECIES_LUCARIO].abilities[0] != ABILITY_JUSTIFIED);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_LUCARIO) { Ability(ABILITY_JUSTIFIED); Speed(50); Moves(MOVE_TACKLE); } // innate Inner Focus
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_FAKE_OUT); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_FAKE_OUT); MOVE(player, MOVE_TACKLE); }
    } SCENE {
        if (enabled)
            ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, player); // not flinched -> Tackle executes
        else
            NONE_OF { ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, player); } // flinched
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Inner Focus is unaffected by Intimidate (Gen8+)")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(B_UPDATED_INTIMIDATE, GEN_8);
        PLAYER(SPECIES_LUCARIO) { Ability(ABILITY_JUSTIFIED); } // innate Inner Focus
        OPPONENT(SPECIES_EKANS) { Ability(ABILITY_INTIMIDATE); }
    } WHEN {
        TURN {}
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_INTIMIDATE);
        ABILITY_POPUP(player, ABILITY_INNER_FOCUS); // pop-up shows the innate Inner Focus
        MESSAGE("Lucario's Attack was not lowered!");
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE);
    }
}

// Grumpig (not Slowbro) is the vehicle: Slowbro also carries innate Oblivious, which the Intimidate-
// immunity block credits first, so its pop-up would read Oblivious. Grumpig has innate Own Tempo without
// Oblivious, so the immunity is unambiguously Own Tempo's.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Own Tempo is unaffected by Intimidate (Gen8+)")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_GRUMPIG, ABILITY_OWN_TEMPO));
        ASSUME(!SpeciesHasInnate(SPECIES_GRUMPIG, ABILITY_OBLIVIOUS));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(B_UPDATED_INTIMIDATE, GEN_8);
        PLAYER(SPECIES_GRUMPIG) { Ability(ABILITY_GLUTTONY); } // chosen Gluttony, innate Own Tempo
        OPPONENT(SPECIES_EKANS) { Ability(ABILITY_INTIMIDATE); }
    } WHEN {
        TURN {}
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_INTIMIDATE);
        ABILITY_POPUP(player, ABILITY_OWN_TEMPO);
        MESSAGE("Grumpig's Attack was not lowered!");
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Leaf Guard blocks status in sun")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(GetMoveNonVolatileStatus(MOVE_THUNDER_WAVE) == MOVE_EFFECT_PARALYSIS);
        ASSUME(SpeciesHasInnate(SPECIES_MEGANIUM, ABILITY_LEAF_GUARD));
        ASSUME(gSpeciesInfo[SPECIES_MEGANIUM].abilities[0] != ABILITY_LEAF_GUARD);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_MEGANIUM) { Ability(ABILITY_OVERGROW); } // chosen differs from the innate Leaf Guard
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SUNNY_DAY, MOVE_THUNDER_WAVE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUNNY_DAY); }
        TURN { MOVE(opponent, MOVE_THUNDER_WAVE); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_LEAF_GUARD);
            NONE_OF { STATUS_ICON(player, paralysis: TRUE); }
        } else {
            STATUS_ICON(player, paralysis: TRUE);
        }
    } THEN {
        if (enabled)
            EXPECT_EQ(player->status1 & STATUS1_PARALYSIS, 0);
    }
}

// Leaf Guard is sun-gated: with no sun the innate does nothing and the status lands.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Leaf Guard does nothing outside sun")
{
    GIVEN {
        ASSUME(GetMoveNonVolatileStatus(MOVE_THUNDER_WAVE) == MOVE_EFFECT_PARALYSIS);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_MEGANIUM) { Ability(ABILITY_OVERGROW); } // innate Leaf Guard, but no sun
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_THUNDER_WAVE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_THUNDER_WAVE); }
    } SCENE {
        STATUS_ICON(player, paralysis: TRUE);
        NONE_OF { ABILITY_POPUP(player, ABILITY_LEAF_GUARD); }
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Overcoat blocks powder moves")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(IsPowderMove(MOVE_SPORE));
        ASSUME(SpeciesHasInnate(SPECIES_KOMMO_O, ABILITY_OVERCOAT));
        ASSUME(gSpeciesInfo[SPECIES_KOMMO_O].abilities[0] != ABILITY_OVERCOAT);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_KOMMO_O) { Ability(ABILITY_BULLETPROOF); } // chosen differs from the innate Overcoat
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SPORE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SPORE); }
    } SCENE {
        if (enabled)
            NONE_OF { STATUS_ICON(player, sleep: TRUE); }
        else
            STATUS_ICON(player, sleep: TRUE);
    } THEN {
        if (enabled)
            EXPECT_EQ(player->status1 & STATUS1_SLEEP, 0);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Overcoat ignores sandstorm chip damage")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        // Kommo-o is Dragon/Fighting, so it normally takes sandstorm chip (not Rock/Ground/Steel).
        ASSUME(SpeciesHasInnate(SPECIES_KOMMO_O, ABILITY_OVERCOAT));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_KOMMO_O) { Ability(ABILITY_BULLETPROOF); MaxHP(240); HP(240); Moves(MOVE_CELEBRATE); } // innate Overcoat
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SANDSTORM); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_SANDSTORM); }
    } SCENE {
        if (enabled)
            NONE_OF { HP_BAR(player); } // Overcoat -> no sandstorm chip
        else
            HP_BAR(player); // no innate -> loses 1/16 to sandstorm
    }
}

// Identity stays the chosen slot: Trace copies Meganium's chosen Overgrow, never its innate Leaf Guard.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Trace copies the chosen ability, never an innate Leaf Guard")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_GARDEVOIR) { Ability(ABILITY_TRACE); }
        OPPONENT(SPECIES_MEGANIUM) { Ability(ABILITY_OVERGROW); } // innate Leaf Guard, chosen Overgrow
    } WHEN {
        TURN {}
    } SCENE {
        ABILITY_POPUP(player, ABILITY_TRACE);
        MESSAGE("It traced the opposing Meganium's Overgrow!"); // chosen Overgrow, never the innate Leaf Guard
    }
}

// ============================================================================
// Batch S — miscellaneous single-site traits: Suction Cups / Guard Dog /
// Rock Head / Long Reach / Skill Link / Infiltrator / Corrosion / Sticky Hold /
// Unseen Fist / Piercing Drill / Heavy Metal / Light Metal.
// ============================================================================

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Suction Cups blocks Whirlwind's forced switch")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CRADILY, ABILITY_SUCTION_CUPS));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_CRADILY) { Ability(ABILITY_STORM_DRAIN); } // chosen differs from the innate Suction Cups
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_WHIRLWIND); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_WHIRLWIND); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_SUCTION_CUPS);
            MESSAGE("Cradily is anchored in place with its suction cups!"); // switch blocked
        } else {
            NONE_OF { ABILITY_POPUP(player, ABILITY_SUCTION_CUPS); } // no block -> Cradily is phazed out
        }
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Guard Dog blocks Dragon Tail's forced switch")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_OKIDOGI, ABILITY_GUARD_DOG));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_OKIDOGI) { Ability(ABILITY_TOXIC_CHAIN); MaxHP(300); HP(300); } // chosen differs from the innate Guard Dog
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_WHIRLWIND); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_WHIRLWIND); }
    } SCENE {
        if (enabled)
            MESSAGE("But it failed!"); // Guard Dog makes the forced switch fail
        else
            NONE_OF { MESSAGE("But it failed!"); } // no block -> Okidogi is phazed out
    }
}

// Batch V: an innate Guard Dog is immune to Intimidate's Attack drop and instead boosts its own Attack by
// 1 stage, exactly like the real ability (mirrors the chosen ABILITY_GUARD_DOG case in IsIntimidateBlocked).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Guard Dog boosts Attack instead of losing it when intimidated")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_OKIDOGI, ABILITY_GUARD_DOG));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_OKIDOGI) { Ability(ABILITY_TOXIC_CHAIN); } // chosen differs from the innate Guard Dog
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_ARBOK) { Ability(ABILITY_INTIMIDATE); }
    } WHEN {
        TURN { SWITCH(opponent, 1); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_INTIMIDATE);
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_GUARD_DOG);
            MESSAGE("Okidogi's Attack rose!");
        } else {
            NONE_OF { ABILITY_POPUP(player, ABILITY_GUARD_DOG); }
        }
    } THEN {
        if (enabled)
            EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE + 1); // immune to the drop, boosted instead
        else
            EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE - 1); // no innate -> Intimidate drops Attack
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Rock Head negates recoil")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MAROWAK, ABILITY_ROCK_HEAD));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_MAROWAK) { Ability(ABILITY_LIGHTNING_ROD); Moves(MOVE_DOUBLE_EDGE); } // chosen differs from the innate Rock Head
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(300); HP(300); }
    } WHEN {
        TURN { MOVE(player, MOVE_DOUBLE_EDGE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DOUBLE_EDGE, player);
        HP_BAR(opponent);
        if (enabled)
            NONE_OF { HP_BAR(player); } // no recoil
        else
            HP_BAR(player); // takes recoil
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Long Reach makes moves non-contact (no Rocky Helmet chip)")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(MoveMakesContact(MOVE_LEAF_BLADE));
        ASSUME(SpeciesHasInnate(SPECIES_DECIDUEYE, ABILITY_LONG_REACH));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_DECIDUEYE) { Ability(ABILITY_OVERGROW); Moves(MOVE_LEAF_BLADE); } // chosen differs from the innate Long Reach
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_ROCKY_HELMET); MaxHP(300); HP(300); }
    } WHEN {
        TURN { MOVE(player, MOVE_LEAF_BLADE); }
    } SCENE {
        HP_BAR(opponent);
        if (enabled)
            NONE_OF { HP_BAR(player); } // Long Reach -> no contact -> no Rocky Helmet chip
        else
            HP_BAR(player); // contact -> Rocky Helmet chips the attacker
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Skill Link maxes multistrike hits (deterministic reroute)")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(IsMultiHitMove(MOVE_ICICLE_SPEAR));
        ASSUME(SpeciesHasInnate(SPECIES_CLOYSTER, ABILITY_SKILL_LINK));
        ASSUME(gSpeciesInfo[SPECIES_CLOYSTER].abilities[0] != ABILITY_SKILL_LINK);
        WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, TRUE); // shipping default; Skill Link -> max, else DETERMINISTIC_MULTI_HIT_COUNT (3)
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_CLOYSTER) { Ability(ABILITY_SHELL_ARMOR); Moves(MOVE_ICICLE_SPEAR); } // chosen differs from the innate Skill Link
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(500); HP(500); }
    } WHEN {
        TURN { MOVE(player, MOVE_ICICLE_SPEAR); }
    } SCENE {
        if (enabled)
            MESSAGE("The Pokémon was hit 5 time(s)!");
        else
            MESSAGE("The Pokémon was hit 3 time(s)!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Infiltrator ignores the foe's Substitute")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CROBAT, ABILITY_INFILTRATOR));
        ASSUME(gSpeciesInfo[SPECIES_CROBAT].abilities[0] != ABILITY_INFILTRATOR);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_CROBAT) { Ability(ABILITY_INNER_FOCUS); Moves(MOVE_AIR_SLASH); } // chosen differs from the innate Infiltrator
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SUBSTITUTE); MaxHP(300); HP(300); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUBSTITUTE); MOVE(player, MOVE_AIR_SLASH); }
    } THEN {
        // Substitute costs 1/4 max HP (300 -> 225). Infiltrator then hits the mon behind it.
        if (enabled)
            EXPECT_LT(opponent->hp, 225); // hit through the Substitute
        else
            EXPECT_EQ(opponent->hp, 225); // Substitute absorbed the hit
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Corrosion poisons a Steel type")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SALAZZLE, ABILITY_CORROSION));
        ASSUME(gSpeciesInfo[SPECIES_SALAZZLE].abilities[0] != ABILITY_CORROSION || gSpeciesInfo[SPECIES_SALAZZLE].abilities[2] != ABILITY_CORROSION);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_SALAZZLE) { Ability(ABILITY_OBLIVIOUS); Moves(MOVE_TOXIC); } // chosen differs from the innate Corrosion
        OPPONENT(SPECIES_STEELIX);
    } WHEN {
        TURN { MOVE(player, MOVE_TOXIC); }
    } THEN {
        if (enabled)
            EXPECT_NE(opponent->status1 & STATUS1_TOXIC_POISON, 0);
        else
            EXPECT_EQ(opponent->status1 & STATUS1_TOXIC_POISON, 0);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Sticky Hold keeps the held item from Knock Off")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MUK, ABILITY_STICKY_HOLD));
        ASSUME(gSpeciesInfo[SPECIES_MUK].abilities[0] != ABILITY_STICKY_HOLD);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_MUK) { Ability(ABILITY_POISON_TOUCH); Item(ITEM_LEFTOVERS); MaxHP(400); HP(400); } // chosen differs from the innate Sticky Hold
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_KNOCK_OFF); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_KNOCK_OFF); }
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_STICKY_HOLD);
    } THEN {
        if (enabled)
            EXPECT_EQ(player->item, ITEM_LEFTOVERS); // item survives
        else
            EXPECT_EQ(player->item, ITEM_NONE); // item knocked off
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Unseen Fist hits through Protect")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(MoveMakesContact(MOVE_CLOSE_COMBAT));
        ASSUME(SpeciesHasInnate(SPECIES_URSHIFU, ABILITY_UNSEEN_FIST));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_URSHIFU) { Ability(ABILITY_SNIPER); Moves(MOVE_CLOSE_COMBAT); } // chosen differs from the innate Unseen Fist
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_PROTECT); MaxHP(400); HP(400); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_PROTECT); MOVE(player, MOVE_CLOSE_COMBAT); }
    } SCENE {
        if (enabled)
            HP_BAR(opponent); // Unseen Fist bypasses Protect with a contact move
        else
            NONE_OF { HP_BAR(opponent); } // Protect blocks the hit
    }
}

// Piercing Drill shares Unseen Fist's exact effect site (the same `||` clause everywhere), so the
// Protect-bypass mechanism is proven by the Unseen Fist test above. Excadrill-Mega is ability-locked
// to Piercing Drill (all three slots), so an innate-vs-chosen distinction can't be shown; assert only
// that it carries the innate so the allowlist/table membership is covered.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Excadrill-Mega carries innate Piercing Drill")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_EXCADRILL_MEGA, ABILITY_PIERCING_DRILL));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_EXCADRILL_MEGA) { Moves(MOVE_HIGH_HORSEPOWER); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_PROTECT); MaxHP(400); HP(400); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_PROTECT); MOVE(player, MOVE_HIGH_HORSEPOWER); }
    } SCENE {
        HP_BAR(opponent); // Piercing Drill bypasses Protect with a contact move
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Heavy Metal doubles the holder's weight for Low Kick", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(GetSpeciesWeight(SPECIES_ARON) == 600); // 60.0 kg (80 power) -> Heavy Metal 120.0 kg (100 power)
        ASSUME(SpeciesHasInnate(SPECIES_ARON, ABILITY_HEAVY_METAL));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_LOW_KICK); }
        OPPONENT(SPECIES_ARON) { Ability(ABILITY_STURDY); MaxHP(400); HP(400); } // chosen differs from the innate Heavy Metal
    } WHEN {
        TURN { MOVE(player, MOVE_LOW_KICK); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_LOW_KICK, player);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.25), results[0].damage); // enabled(100 power) == disabled(80 power) * 1.25
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Light Metal halves the holder's weight for Low Kick", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(GetSpeciesWeight(SPECIES_BELDUM) == 952); // 95.2 kg (80 power) -> Light Metal 47.6 kg (60 power)
        ASSUME(SpeciesHasInnate(SPECIES_BELDUM, ABILITY_LIGHT_METAL));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_LOW_KICK); }
        OPPONENT(SPECIES_BELDUM) { Ability(ABILITY_CLEAR_BODY); MaxHP(400); HP(400); } // chosen differs from the innate Light Metal
    } WHEN {
        TURN { MOVE(player, MOVE_LOW_KICK); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_LOW_KICK, player);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(0.75), results[0].damage); // enabled(60 power) == disabled(80 power) * 0.75
    }
}

// Suppression parity: Gastro Acid removes an innate Rock Head, so recoil applies again.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Rock Head")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_MAROWAK) { Ability(ABILITY_LIGHTNING_ROD); Moves(MOVE_DOUBLE_EDGE); } // innate Rock Head
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID); MaxHP(300); HP(300); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_GASTRO_ACID); MOVE(player, MOVE_DOUBLE_EDGE); }
    } SCENE {
        HP_BAR(opponent);
        HP_BAR(player); // innate Rock Head suppressed -> recoil applies
    }
}

// ===== Batch J — end-of-turn effects (Rain Dish / Ice Body / Shed Skin / Hydration /
// Healer / Harvest / Cud Chew / Pickup / Bad Dreams / Poison Heal) =====
// The first nine are active, scripted end-turn innates that reuse the Speed Boost driver
// (TryActivateInnateEndTurnEffects -> IsActiveEndTurnInnate) to delegate to the upstream
// ABILITYEFFECT_ENDTURN case, so the heal / cure / item recovery / chip damage / script /
// pop-up match the real ability; each effect site forces the pop-up to the innate when the
// chosen ability differs. Poison Heal is NOT a driver innate — it REPLACES the poison-damage
// step, wired at HandleEndTurnPoison (src/battle_end_turn.c). Every vehicle carries the innate
// but a DIFFERENT chosen ability, so only the innate can be responsible for the effect. The
// RNG-gated members (Shed Skin / Healer / Harvest / Pickup) are tested under
// DETERMINISTIC_ABILITIES (the shipping default), where they fire deterministically.

// ----- Rain Dish (heals 1/16 max HP in rain) -----
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Rain Dish heals 1/16 max HP in rain")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_BLASTOISE, ABILITY_RAIN_DISH));
        ASSUME(GetMoveWeatherType(MOVE_RAIN_DANCE) == BATTLE_WEATHER_RAIN);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_BLASTOISE) { Ability(ABILITY_TORRENT); HP(1); MaxHP(160); } // chosen differs from innate Rain Dish
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_RAIN_DANCE); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_RAIN_DISH); // pop-up shows the innate, not Torrent
            HP_BAR(player, damage: -(160 / 16));
        } else {
            NONE_OF { ABILITY_POPUP(player, ABILITY_RAIN_DISH); HP_BAR(player); }
        }
    }
}

// ----- Ice Body (heals 1/16 max HP in snow) -----
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Ice Body heals 1/16 max HP in snow")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_WALREIN, ABILITY_ICE_BODY));
        ASSUME(GetMoveWeatherType(MOVE_SNOWSCAPE) == BATTLE_WEATHER_SNOW);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WALREIN) { Ability(ABILITY_THICK_FAT); HP(1); MaxHP(160); } // chosen differs from innate Ice Body
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SNOWSCAPE); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_ICE_BODY);
            HP_BAR(player, damage: -(160 / 16));
        } else {
            NONE_OF { ABILITY_POPUP(player, ABILITY_ICE_BODY); HP_BAR(player); }
        }
    }
}

// ----- Shed Skin (cures a status at end of turn; always under DETERMINISTIC_ABILITIES) -----
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES + DETERMINISTIC_ABILITIES: innate Shed Skin cures status")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SEVIPER, ABILITY_SHED_SKIN));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        PLAYER(SPECIES_SEVIPER) { Ability(ABILITY_INFILTRATOR); Status1(STATUS1_POISON); } // chosen differs from innate Shed Skin
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_SHED_SKIN);
            MESSAGE("Seviper was cured of its poisoning!");
        } else {
            NONE_OF { ABILITY_POPUP(player, ABILITY_SHED_SKIN); }
        }
    } THEN {
        if (!enabled)
            EXPECT(player->status1 & STATUS1_POISON); // no innate -> stays poisoned
    }
}

// ----- Hydration (cures status in rain) -----
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Hydration cures status in rain")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_VAPOREON, ABILITY_HYDRATION));
        ASSUME(GetMoveWeatherType(MOVE_RAIN_DANCE) == BATTLE_WEATHER_RAIN);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_VAPOREON) { Ability(ABILITY_WATER_ABSORB); Status1(STATUS1_BURN); } // chosen differs from innate Hydration
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_RAIN_DANCE); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_HYDRATION);
            MESSAGE("Vaporeon's burn was cured!");
        } else {
            NONE_OF { ABILITY_POPUP(player, ABILITY_HYDRATION); }
        }
    } THEN {
        if (!enabled)
            EXPECT(player->status1 & STATUS1_BURN);
    }
}

// ----- Healer (cures an adjacent ally's status; always under DETERMINISTIC_ABILITIES) -----
DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES + DETERMINISTIC_ABILITIES: innate Healer cures ally status")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_BLISSEY, ABILITY_HEALER));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        PLAYER(SPECIES_BLISSEY) { Ability(ABILITY_NATURAL_CURE); } // chosen differs from innate Healer
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_BURN); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_CELEBRATE); MOVE(playerRight, MOVE_CELEBRATE); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(playerLeft, ABILITY_HEALER);
            MESSAGE("Wobbuffet's burn was cured!");
        } else {
            NONE_OF { ABILITY_POPUP(playerLeft, ABILITY_HEALER); }
        }
    } THEN {
        if (!enabled)
            EXPECT(playerRight->status1 & STATUS1_BURN);
    }
}

// ----- Harvest (recovers a used Berry; always under DETERMINISTIC_ABILITIES) -----
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES + DETERMINISTIC_ABILITIES: innate Harvest restores a used Berry")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_TROPIUS, ABILITY_HARVEST));
        ASSUME(gItemsInfo[ITEM_SITRUS_BERRY].holdEffect == HOLD_EFFECT_RESTORE_PCT_HP);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        PLAYER(SPECIES_TROPIUS) { Ability(ABILITY_CHLOROPHYLL); MaxHP(500); HP(251); Item(ITEM_SITRUS_BERRY); } // chosen differs from innate Harvest
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); } // drops Tropius below half -> Sitrus eaten
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_HARVEST);
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_HARVEST); }
    } THEN {
        EXPECT_EQ(player->item, enabled ? ITEM_SITRUS_BERRY : ITEM_NONE); // innate restores the Berry
    }
}

// ----- Cud Chew (re-eats a Berry the turn after eating it) -----
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Cud Chew re-eats a Berry the next turn")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_TAUROS_PALDEA_COMBAT, ABILITY_CUD_CHEW));
        ASSUME(gItemsInfo[ITEM_ORAN_BERRY].holdEffect == HOLD_EFFECT_RESTORE_HP);
        ASSUME(GetMoveFixedHPDamage(MOVE_DRAGON_RAGE) == 40);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_TAUROS_PALDEA_COMBAT) { Ability(ABILITY_INTIMIDATE); MaxHP(60); HP(60); Item(ITEM_ORAN_BERRY); } // chosen differs from innate Cud Chew
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_DRAGON_RAGE); } // HP 60 -> 20, Oran eaten
        TURN { MOVE(player, MOVE_CELEBRATE); }     // Cud Chew re-eats the Oran here
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_CUD_CHEW);
            ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_BERRY, player);
        } else {
            NONE_OF { ABILITY_POPUP(player, ABILITY_CUD_CHEW); }
        }
    }
}

// ----- Pickup (grabs an item consumed this turn) -----
// Run under DETERMINISTIC_ABILITIES (the shipping default), which selects the pickup target by a
// fixed preference order rather than at random; in singles the sole valid target is the foe either way.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES + DETERMINISTIC_ABILITIES: innate Pickup grabs a consumed item")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_ZIGZAGOON, ABILITY_PICKUP));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        PLAYER(SPECIES_ZIGZAGOON) { Ability(ABILITY_GLUTTONY); } // no item; chosen differs from innate Pickup
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(100); HP(51); Item(ITEM_SITRUS_BERRY); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); } // foe drops below half -> eats Sitrus
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_PICKUP);
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_PICKUP); }
    } THEN {
        EXPECT_EQ(player->item, enabled ? ITEM_SITRUS_BERRY : ITEM_NONE); // innate picks up the consumed Berry
    }
}

// ----- Bad Dreams (chips sleeping foes 1/8 max HP) -----
// Munna/Musharna are the dream-eater flavor vehicles (their canon abilities are all non-Bad-Dreams).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Bad Dreams chips a sleeping foe")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MUSHARNA, ABILITY_BAD_DREAMS));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_MUSHARNA) { Ability(ABILITY_SYNCHRONIZE); } // chosen differs from innate Bad Dreams
        OPPONENT(SPECIES_WOBBUFFET) { Status1(STATUS1_SLEEP); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_BAD_DREAMS);
            HP_BAR(opponent);
        } else {
            NONE_OF { ABILITY_POPUP(player, ABILITY_BAD_DREAMS); HP_BAR(opponent); }
        }
    } THEN {
        if (enabled)
            EXPECT_EQ(opponent->hp, opponent->maxHP - opponent->maxHP / 8);
    }
}

// A non-sleeping foe takes no Bad Dreams damage and shows no pop-up (also guards the overwrite
// against leaking: the innate overwrite is only set when a valid sleeping target exists).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Bad Dreams spares an awake foe")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MUSHARNA, ABILITY_BAD_DREAMS));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_MUSHARNA) { Ability(ABILITY_SYNCHRONIZE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(player, ABILITY_BAD_DREAMS); HP_BAR(opponent); }
    }
}

// ----- Poison Heal (heals 1/8 max HP instead of losing HP while poisoned) -----
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Poison Heal heals instead of taking poison damage")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_GLISCOR, ABILITY_POISON_HEAL));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_GLISCOR) { Ability(ABILITY_SAND_VEIL); Status1(STATUS1_POISON); HP(200); MaxHP(400); } // chosen differs from innate Poison Heal
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_POISON_HEAL);
            HP_BAR(player, damage: -(400 / 8)); // heals 1/8 instead of taking damage
        } else {
            NONE_OF { ABILITY_POPUP(player, ABILITY_POISON_HEAL); }
        }
    } THEN {
        EXPECT_EQ(player->hp, enabled ? 200 + 400 / 8 : 200 - 400 / 8); // innate heals; else poison damage
    }
}

// Suppression parity: Gastro Acid blanks an innate Poison Heal, so the holder takes poison damage.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Poison Heal")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_GLISCOR, ABILITY_POISON_HEAL));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_GLISCOR) { Ability(ABILITY_SAND_VEIL); Status1(STATUS1_POISON); MaxHP(400); HP(400); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_GASTRO_ACID); MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Gliscor's Ability was suppressed!");
        NONE_OF { ABILITY_POPUP(player, ABILITY_POISON_HEAL); }
    } THEN {
        EXPECT_LT(player->hp, player->maxHP); // suppressed -> takes poison damage instead of healing
    }
}

// ───────────────────────── Berry / item synergy (Batch T) ─────────────────────────
// Gluttony (eat a pinch Berry at 1/2 HP), Ripen (double every Berry effect), Cheek Pouch
// (heal 1/3 max HP on eating a Berry), Unburden (double Speed once the held item is lost).
// All 1:1 clean-upside boons. See src/fork/innate_abilities.c and the wiring reference.

// ----- Gluttony (eats a pinch Berry at 1/2 HP instead of 1/4) -----
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Gluttony eats a pinch Berry at 1/2 HP")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MUNCHLAX, ABILITY_GLUTTONY));
        ASSUME(gItemsInfo[ITEM_LIECHI_BERRY].holdEffect == HOLD_EFFECT_ATTACK_UP);
        ASSUME(GetMoveFixedHPDamage(MOVE_DRAGON_RAGE) == 40);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_MUNCHLAX) { Ability(ABILITY_THICK_FAT); MaxHP(60); HP(60); Item(ITEM_LIECHI_BERRY); } // chosen differs from innate Gluttony
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_DRAGON_RAGE); MOVE(player, MOVE_CELEBRATE); } // HP 60 -> 20 (between 1/4 and 1/2)
    } THEN {
        // With Gluttony the Liechi is eaten at 1/2 HP (Atk +1); without, 20 HP is above the 1/4 pinch threshold.
        EXPECT_EQ(player->item, enabled ? ITEM_NONE : ITEM_LIECHI_BERRY);
        EXPECT_EQ(player->statStages[STAT_ATK], enabled ? DEFAULT_STAT_STAGE + 1 : DEFAULT_STAT_STAGE);
    }
}

// ----- Ripen (doubles a Berry's effect) -----
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Ripen doubles a Berry's heal")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_FLAPPLE, ABILITY_RIPEN));
        ASSUME(gItemsInfo[ITEM_SITRUS_BERRY].holdEffect == HOLD_EFFECT_RESTORE_PCT_HP);
        ASSUME(GetMoveFixedHPDamage(MOVE_DRAGON_RAGE) == 40);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_FLAPPLE) { Ability(ABILITY_HUSTLE); MaxHP(100); HP(60); Item(ITEM_SITRUS_BERRY); } // chosen differs from innate Ripen
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_DRAGON_RAGE); MOVE(player, MOVE_CELEBRATE); } // HP 60 -> 20, eats Sitrus (heals 25% of maxHP)
    } THEN {
        // Sitrus heals 25 normally; Ripen doubles it to 50.
        EXPECT_EQ(player->hp, enabled ? 20 + 50 : 20 + 25);
    }
}

// ----- Cheek Pouch (heals 1/3 max HP on eating a Berry, with a pop-up) -----
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Cheek Pouch heals 1/3 max HP on eating a Berry")
{
    bool32 enabled;
    s16 cheekPouchHeal;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_GREEDENT, ABILITY_CHEEK_POUCH));
        ASSUME(GetMoveEffect(MOVE_SUPER_FANG) == EFFECT_FIXED_PERCENT_DAMAGE);
        ASSUME(gItemsInfo[ITEM_ORAN_BERRY].holdEffect == HOLD_EFFECT_RESTORE_HP);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_GREEDENT) { Ability(ABILITY_GLUTTONY); MaxHP(60); HP(31); Item(ITEM_ORAN_BERRY); } // chosen differs from innate Cheek Pouch
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUPER_FANG); } // HP 31 -> 15, eats Oran (below 1/2)
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_BERRY, player);
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_CHEEK_POUCH); // pop-up overwrites to the innate
            HP_BAR(player, captureDamage: &cheekPouchHeal);
        } else {
            NONE_OF { ABILITY_POPUP(player, ABILITY_CHEEK_POUCH); }
        }
    } THEN {
        if (enabled)
            EXPECT_EQ(cheekPouchHeal, -(player->maxHP / 3)); // extra 1/3 max HP from the innate
    }
}

// ----- Unburden (doubles Speed once the held item is consumed/lost) -----
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Unburden doubles Speed after its item is consumed")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_HAWLUCHA, ABILITY_UNBURDEN));
        ASSUME(GetMoveFixedHPDamage(MOVE_DRAGON_RAGE) == 40);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_HAWLUCHA) { Ability(ABILITY_MOLD_BREAKER); Speed(100); MaxHP(60); HP(60); Item(ITEM_SITRUS_BERRY); } // chosen differs from innate Unburden
        OPPONENT(SPECIES_WOBBUFFET) { Speed(150); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_DRAGON_RAGE); MOVE(player, MOVE_CELEBRATE); } // HP 60 -> 20, eats Sitrus -> item lost -> Unburden armed
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_RAGE, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
        if (enabled) // Unburden -> 200 Speed -> player outspeeds 150
        {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
        }
        else // no innate -> 100 Speed -> opponent still faster
        {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
        }
    }
}

// Suppression parity: Gastro Acid blanks an innate Unburden, so the item-loss Speed boost never arms.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Unburden")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_HAWLUCHA, ABILITY_UNBURDEN));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_HAWLUCHA) { Ability(ABILITY_MOLD_BREAKER); Speed(100); Item(ITEM_SITRUS_BERRY); Moves(MOVE_CELEBRATE, MOVE_BELLY_DRUM); MaxHP(60); HP(60); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(150); Moves(MOVE_GASTRO_ACID, MOVE_DRAGON_RAGE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_GASTRO_ACID); MOVE(player, MOVE_CELEBRATE); }
        TURN { MOVE(opponent, MOVE_DRAGON_RAGE); MOVE(player, MOVE_CELEBRATE); } // HP -> 20, eats Sitrus, but Unburden is suppressed
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Hawlucha's Ability was suppressed!");
        // Suppressed Unburden never arms -> player (100) stays slower than the foe (150) on the last turn.
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
    }
}

// ===== Batch K (on-hit contact reactions) =====
// The first active, scripted ON-HIT innates, fired through the new re-entrant on-hit driver
// (TryActivateInnateOnHitEffects -> IsActiveOnHitInnate), hooked from the MOVEEND_ABILITIES_INNATE
// step and delegating to the upstream ABILITYEFFECT_MOVE_END case. Rough Skin / Iron Barbs chip a
// contact attacker 1/8 max HP; Gooey / Tangling Hair drop a contact attacker's Speed by 1. Each is a
// 1:1 clean-upside copy (a contact reaction only ever hurts the attacker), with the pop-up overwritten
// to the innate when the chosen ability differs.

// Rough Skin / Iron Barbs chip a contact attacker even when they are the holder's INNATE (chosen
// ability differs). Garchomp carries innate Rough Skin (chosen Sand Veil); Ferrothorn carries innate
// Iron Barbs (chosen Anticipation). Feature-off leg proves the chip comes only from the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Rough Skin / Iron Barbs chip a contact attacker")
{
    u32 species;
    enum Ability chosen, innate;
    bool32 enabled;
    PARAMETRIZE { species = SPECIES_GARCHOMP;   chosen = ABILITY_SAND_VEIL;    innate = ABILITY_ROUGH_SKIN; enabled = TRUE; }
    PARAMETRIZE { species = SPECIES_GARCHOMP;   chosen = ABILITY_SAND_VEIL;    innate = ABILITY_ROUGH_SKIN; enabled = FALSE; }
    PARAMETRIZE { species = SPECIES_FERROTHORN; chosen = ABILITY_ANTICIPATION; innate = ABILITY_IRON_BARBS;  enabled = TRUE; }
    PARAMETRIZE { species = SPECIES_FERROTHORN; chosen = ABILITY_ANTICIPATION; innate = ABILITY_IRON_BARBS;  enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(species, innate));
        ASSUME(MoveMakesContact(MOVE_SCRATCH));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(species) { Ability(chosen); } // chosen ability is NOT the innate
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(opponent, innate); // pop-up shows the innate, not the chosen ability
            MESSAGE("Wobbuffet was hurt!");
        } else {
            NONE_OF { ABILITY_POPUP(opponent, innate); HP_BAR(player); }
        }
    } THEN {
        if (enabled)
            EXPECT_EQ(player->hp, player->maxHP - player->maxHP / 8);
    }
}

// A REAL Rough Skin still chips exactly once with the feature on — the driver skips an innate equal to
// the chosen ability, so it never double-fires beside the chosen-ability contact block.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: a chosen Rough Skin chips once, not twice, with innates on")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_GARCHOMP, ABILITY_ROUGH_SKIN));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_GARCHOMP) { Ability(ABILITY_ROUGH_SKIN); } // chosen == innate
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_ROUGH_SKIN);
        MESSAGE("Wobbuffet was hurt!");
    } THEN {
        EXPECT_EQ(player->hp, player->maxHP - player->maxHP / 8); // one chip, not two
    }
}

// Parity: a non-contact move never triggers the innate chip (routes through CanBattlerAvoidContactEffects).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Rough Skin does not chip a non-contact attacker")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_GARCHOMP, ABILITY_ROUGH_SKIN));
        ASSUME(!MoveMakesContact(MOVE_SWIFT));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_GARCHOMP) { Ability(ABILITY_SAND_VEIL); }
    } WHEN {
        TURN { MOVE(player, MOVE_SWIFT); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(opponent, ABILITY_ROUGH_SKIN); HP_BAR(player); }
    }
}

// Suppression parity: Gastro Acid nullifies an innate Rough Skin exactly like a real ability, so the
// contact attacker takes no chip.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Rough Skin")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_GARCHOMP, ABILITY_ROUGH_SKIN));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_GASTRO_ACID, MOVE_SCRATCH); }
        OPPONENT(SPECIES_GARCHOMP) { Ability(ABILITY_SAND_VEIL); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_GASTRO_ACID); }
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("The opposing Garchomp's Ability was suppressed!");
        NONE_OF { ABILITY_POPUP(opponent, ABILITY_ROUGH_SKIN); }
    }
}

// Gooey / Tangling Hair drop a contact attacker's Speed even when they are the holder's INNATE.
// Goodra carries innate Gooey (chosen Sap Sipper); Dugtrio-Alola carries innate Tangling Hair
// (chosen Sand Veil). Feature-off leg proves the drop comes only from the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Gooey / Tangling Hair drop a contact attacker's Speed")
{
    u32 species;
    enum Ability chosen, innate;
    bool32 enabled;
    PARAMETRIZE { species = SPECIES_GOODRA;         chosen = ABILITY_SAP_SIPPER; innate = ABILITY_GOOEY;         enabled = TRUE; }
    PARAMETRIZE { species = SPECIES_GOODRA;         chosen = ABILITY_SAP_SIPPER; innate = ABILITY_GOOEY;         enabled = FALSE; }
    PARAMETRIZE { species = SPECIES_DUGTRIO_ALOLA;  chosen = ABILITY_SAND_VEIL;  innate = ABILITY_TANGLING_HAIR; enabled = TRUE; }
    PARAMETRIZE { species = SPECIES_DUGTRIO_ALOLA;  chosen = ABILITY_SAND_VEIL;  innate = ABILITY_TANGLING_HAIR; enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(species, innate));
        ASSUME(MoveMakesContact(MOVE_SCRATCH));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(species) { Ability(chosen); } // chosen ability is NOT the innate
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(opponent, innate); // pop-up shows the innate, not the chosen ability
            ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, player);
            MESSAGE("Wobbuffet's Speed fell!");
        } else {
            NONE_OF { ABILITY_POPUP(opponent, innate); MESSAGE("Wobbuffet's Speed fell!"); }
        }
    }
}

// On-faint retaliation (Batch K second sub-group): Aftermath / Innards Out fire from the SAME
// ABILITYEFFECT_MOVE_END step once a move KOs the holder — the fainted-but-still-on-field holder is
// credited because notOnField is not yet set. Aftermath chips the attacker 1/4 max HP only on a contact
// KO (Damp still blocks it); Innards Out deals the attacker the holder's lost HP on any KO. Both are 1:1
// clean-upside copies (they only ever hurt the attacker), with the pop-up overwritten to the innate.

// Aftermath chips a contact attacker 1/4 max HP when it KOs the holder, even as the holder's INNATE.
// Voltorb carries innate Aftermath (chosen Soundproof); Trubbish carries innate Aftermath (chosen
// Sticky Hold). Feature-off leg proves the chip comes only from the innate. (Both are hit by the
// Normal-type Scratch — a Ghost Aftermath user like Drifblim would need a Ghost-hitting contact move.)
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Aftermath chips a contact attacker that KOs the holder")
{
    u32 species;
    enum Ability chosen;
    bool32 enabled;
    s16 aftermathDamage;
    PARAMETRIZE { species = SPECIES_VOLTORB;  chosen = ABILITY_SOUNDPROOF;  enabled = TRUE; }
    PARAMETRIZE { species = SPECIES_VOLTORB;  chosen = ABILITY_SOUNDPROOF;  enabled = FALSE; }
    PARAMETRIZE { species = SPECIES_TRUBBISH; chosen = ABILITY_STICKY_HOLD; enabled = TRUE; }
    PARAMETRIZE { species = SPECIES_TRUBBISH; chosen = ABILITY_STICKY_HOLD; enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(species, ABILITY_AFTERMATH));
        ASSUME(MoveMakesContact(MOVE_SCRATCH));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(species) { HP(1); Ability(chosen); } // chosen ability is NOT Aftermath
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(opponent, ABILITY_AFTERMATH); // pop-up shows the innate, not the chosen ability
            HP_BAR(player, captureDamage: &aftermathDamage);
        } else {
            NONE_OF { ABILITY_POPUP(opponent, ABILITY_AFTERMATH); HP_BAR(player); }
        }
    } THEN {
        if (enabled)
            EXPECT_EQ(aftermathDamage, player->maxHP / 4);
    }
}

// A REAL Aftermath still chips exactly once with the feature on — the driver skips an innate equal to the
// chosen ability, so it never double-fires beside the chosen-ability faint block.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: a chosen Aftermath chips once, not twice, with innates on")
{
    s16 aftermathDamage;
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_VOLTORB, ABILITY_AFTERMATH));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_VOLTORB) { HP(1); Ability(ABILITY_AFTERMATH); } // chosen == innate
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_AFTERMATH);
        HP_BAR(player, captureDamage: &aftermathDamage);
    } THEN {
        EXPECT_EQ(aftermathDamage, player->maxHP / 4); // one chip, not two
    }
}

// Parity: a non-contact KO never triggers the innate chip (routes through CanBattlerAvoidContactEffects).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Aftermath does not chip a non-contact attacker")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_VOLTORB, ABILITY_AFTERMATH));
        ASSUME(!MoveMakesContact(MOVE_SWIFT));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_VOLTORB) { HP(1); Ability(ABILITY_SOUNDPROOF); }
    } WHEN {
        TURN { MOVE(player, MOVE_SWIFT); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(opponent, ABILITY_AFTERMATH); HP_BAR(player); }
    }
}

// Suppression parity: Gastro Acid nullifies an innate Aftermath exactly like a real ability, so the
// contact KO deals no retaliation chip.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Aftermath")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_VOLTORB, ABILITY_AFTERMATH));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_GASTRO_ACID, MOVE_SCRATCH); }
        OPPONENT(SPECIES_VOLTORB) { HP(1); Ability(ABILITY_SOUNDPROOF); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_GASTRO_ACID); }
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("The opposing Voltorb's Ability was suppressed!");
        NONE_OF { ABILITY_POPUP(opponent, ABILITY_AFTERMATH); }
    }
}

// Innards Out deals the attacker the exact HP the holder lost when a move KOs it, even as the holder's
// INNATE and from a NON-contact move (unlike Aftermath). Pyukumuku carries innate Innards Out (chosen
// Unaware). Feature-off leg proves the retaliation comes only from the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Innards Out deals the attacker the holder's lost HP")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_PYUKUMUKU, ABILITY_INNARDS_OUT));
        ASSUME(GetMoveCategory(MOVE_SWIFT) != DAMAGE_CATEGORY_STATUS);
        ASSUME(!MoveMakesContact(MOVE_SWIFT));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(200); HP(200); SpAttack(1000); }
        OPPONENT(SPECIES_PYUKUMUKU) { HP(30); Ability(ABILITY_UNAWARE); } // chosen ability is NOT Innards Out
    } WHEN {
        TURN { MOVE(player, MOVE_SWIFT); }
    } SCENE {
        if (enabled) {
            HP_BAR(opponent, hp: 0);
            ABILITY_POPUP(opponent, ABILITY_INNARDS_OUT); // pop-up shows the innate, not the chosen ability
            HP_BAR(player, hp: 200 - 30); // took the 30 HP Pyukumuku lost
        } else {
            NONE_OF { ABILITY_POPUP(opponent, ABILITY_INNARDS_OUT); }
        }
    }
}

// On-hit stat/charge (Batch K third sub-group): Steam Engine (Speed +6 on a Fire/Water hit), Thermal
// Exchange (Attack +1 on a Fire hit + burn immunity), Wind Power (charge the next Electric move on a wind
// hit) all reuse the same re-entrant on-hit driver — a one-line IsActiveOnHitInnate addition each,
// delegating to the upstream ABILITYEFFECT_MOVE_END case so the stat change / charge / script / pop-up
// match the real ability. All three are 1:1 clean-upside copies (they only ever help the holder), with the
// pop-up overwritten to the innate when the chosen ability differs.

// Steam Engine raises Speed +6 when the holder is hit by a Fire/Water move, even as an INNATE. Coalossal
// carries innate Steam Engine (chosen Flame Body). Feature-off leg proves the boost comes only from the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Steam Engine raises Speed when hit by a Fire move")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_COALOSSAL, ABILITY_STEAM_ENGINE));
        ASSUME(GetMoveType(MOVE_EMBER) == TYPE_FIRE);
        ASSUME(gSpeciesInfo[SPECIES_COALOSSAL].abilities[0] != ABILITY_FLAME_BODY);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_COALOSSAL) { Ability(ABILITY_FLAME_BODY); } // chosen ability is NOT Steam Engine
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_EMBER); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_EMBER); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_STEAM_ENGINE); // pop-up shows the innate, not the chosen Flame Body
            MESSAGE("Coalossal's Speed rose drastically!");
        } else {
            NONE_OF { ABILITY_POPUP(player, ABILITY_STEAM_ENGINE); }
        }
    } THEN {
        if (enabled)
            EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE + 6);
        else
            EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE);
    }
}

// Suppression parity: Gastro Acid nullifies an innate Steam Engine exactly like a real ability, so the
// on-hit driver skips it and no Speed boost fires.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Steam Engine")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_COALOSSAL, ABILITY_STEAM_ENGINE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_GASTRO_ACID, MOVE_EMBER); }
        OPPONENT(SPECIES_COALOSSAL) { Ability(ABILITY_FLAME_BODY); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_GASTRO_ACID); }
        TURN { MOVE(player, MOVE_EMBER); }
    } SCENE {
        MESSAGE("The opposing Coalossal's Ability was suppressed!");
        NONE_OF { ABILITY_POPUP(opponent, ABILITY_STEAM_ENGINE); }
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_SPEED], DEFAULT_STAT_STAGE);
    }
}

// Thermal Exchange raises Attack +1 when the holder is hit by a Fire move, even as an INNATE. Baxcalibur
// carries innate Thermal Exchange (chosen Ice Body). Feature-off leg proves the boost comes only from the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Thermal Exchange raises Attack when hit by a Fire move")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_BAXCALIBUR, ABILITY_THERMAL_EXCHANGE));
        ASSUME(GetMoveType(MOVE_EMBER) == TYPE_FIRE);
        ASSUME(gSpeciesInfo[SPECIES_BAXCALIBUR].abilities[0] != ABILITY_ICE_BODY);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_BAXCALIBUR) { Ability(ABILITY_ICE_BODY); } // chosen ability is NOT Thermal Exchange
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_EMBER); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_EMBER); }
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_THERMAL_EXCHANGE); // pop-up shows the innate, not the chosen Ice Body
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_THERMAL_EXCHANGE); }
    } THEN {
        if (enabled)
            EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE + 1);
        else
            EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE);
    }
}

// Thermal Exchange grants burn immunity, even as an INNATE (wired at CanSetNonVolatileStatus beside Water Veil).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Thermal Exchange prevents burn")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(GetMoveNonVolatileStatus(MOVE_WILL_O_WISP) == MOVE_EFFECT_BURN);
        ASSUME(SpeciesHasInnate(SPECIES_BAXCALIBUR, ABILITY_THERMAL_EXCHANGE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_BAXCALIBUR) { Ability(ABILITY_ICE_BODY); } // chosen differs from the innate Thermal Exchange
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_WILL_O_WISP); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_WILL_O_WISP); }
    } SCENE {
        if (enabled) {
            NONE_OF { STATUS_ICON(player, burn: TRUE); }
        } else {
            STATUS_ICON(player, burn: TRUE);
        }
    } THEN {
        if (enabled)
            EXPECT_EQ(player->status1 & STATUS1_BURN, 0);
    }
}

// Suppression parity + switch-in cure: Thermal Exchange is breakable, so a Mold Breaker Will-O-Wisp burns an
// innate holder. Like every innate immunity cure (Limber / Own Tempo / Water Veil), the cure routes through
// IsInnateActive (which respects Mold Breaker), so it fires on the next Mold-Breaker-free move-end rather than
// the WoW move's own — here the holder's own Celebrate that same turn (the Own Tempo Mold-Breaker-cure precedent).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker burns an innate Thermal Exchange holder, then it is cured")
{
    GIVEN {
        ASSUME(gAbilitiesInfo[ABILITY_THERMAL_EXCHANGE].breakable);
        ASSUME(GetMoveNonVolatileStatus(MOVE_WILL_O_WISP) == MOVE_EFFECT_BURN);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_BAXCALIBUR) { Ability(ABILITY_ICE_BODY); Speed(1); Moves(MOVE_CELEBRATE); } // innate Thermal Exchange, acts second
        OPPONENT(SPECIES_RAMPARDOS) { Ability(ABILITY_MOLD_BREAKER); Speed(200); Moves(MOVE_WILL_O_WISP); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_WILL_O_WISP); MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        STATUS_ICON(player, burn: TRUE); // Mold Breaker pierces the innate -> burned
        ABILITY_POPUP(player, ABILITY_THERMAL_EXCHANGE); // innate cure fires on the holder's own move-end (Mold Breaker gone)
        STATUS_ICON(player, burn: FALSE);
    } THEN {
        EXPECT_EQ(player->status1 & STATUS1_BURN, 0);
    }
}

// Wind Power charges the next Electric move when the holder is hit by a wind move, even as an INNATE.
// Kilowattrel carries innate Wind Power (chosen Volt Absorb). The Charge doubles the holder's next Electric
// move; the feature-off leg proves the charge comes only from the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Wind Power charges when hit by a wind move")
{
    bool32 enabled;
    s16 dmgBefore, dmgAfter;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_KILOWATTREL, ABILITY_WIND_POWER));
        ASSUME(IsWindMove(MOVE_AIR_CUTTER));
        ASSUME(GetMoveType(MOVE_NUZZLE) == TYPE_ELECTRIC);
        ASSUME(gSpeciesInfo[SPECIES_KILOWATTREL].abilities[0] != ABILITY_VOLT_ABSORB);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_KILOWATTREL) { Ability(ABILITY_VOLT_ABSORB); Speed(10); Moves(MOVE_NUZZLE); } // chosen is NOT Wind Power
        OPPONENT(SPECIES_PERSIAN) { Ability(ABILITY_LIMBER); Speed(5); Moves(MOVE_AIR_CUTTER); } // Limber: never paralyzed by Nuzzle
    } WHEN {
        TURN { MOVE(player, MOVE_NUZZLE); MOVE(opponent, MOVE_AIR_CUTTER); }
        TURN { MOVE(player, MOVE_NUZZLE); MOVE(opponent, MOVE_AIR_CUTTER); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_NUZZLE, player);
        HP_BAR(opponent, captureDamage: &dmgBefore);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_AIR_CUTTER, opponent);
        HP_BAR(player);
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_WIND_POWER); // pop-up shows the innate, not the chosen Volt Absorb
            MESSAGE("Being hit by Air Cutter charged Kilowattrel with power!");
        } else {
            NONE_OF { ABILITY_POPUP(player, ABILITY_WIND_POWER); }
        }
        ANIMATION(ANIM_TYPE_MOVE, MOVE_NUZZLE, player);
        HP_BAR(opponent, captureDamage: &dmgAfter);
    } THEN {
        if (enabled)
            EXPECT_MUL_EQ(dmgBefore, Q_4_12(2.0), dmgAfter); // Charge doubled the second Electric move
        else
            EXPECT_EQ(dmgAfter, dmgBefore);
    }
}

// Wind Power also charges when Tailwind takes effect (the ally hook BS_TryWindRiderPower, innate-aware), even
// as an INNATE. Kilowattrel using Tailwind charges itself, doubling its next Electric move.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Wind Power charges when Tailwind takes effect")
{
    bool32 enabled;
    s16 dmgBefore, dmgAfter;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_KILOWATTREL, ABILITY_WIND_POWER));
        ASSUME(GetMoveEffect(MOVE_TAILWIND) == EFFECT_TAILWIND);
        ASSUME(GetMoveType(MOVE_NUZZLE) == TYPE_ELECTRIC);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_KILOWATTREL) { Ability(ABILITY_VOLT_ABSORB); Moves(MOVE_NUZZLE, MOVE_TAILWIND); } // chosen is NOT Wind Power
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_NUZZLE); }
        TURN { MOVE(player, MOVE_TAILWIND); }
        TURN { MOVE(player, MOVE_NUZZLE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_NUZZLE, player);
        HP_BAR(opponent, captureDamage: &dmgBefore);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TAILWIND, player);
        if (enabled)
            ABILITY_POPUP(player, ABILITY_WIND_POWER); // pop-up shows the innate on the Tailwind trigger
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_WIND_POWER); }
        ANIMATION(ANIM_TYPE_MOVE, MOVE_NUZZLE, player);
        HP_BAR(opponent, captureDamage: &dmgAfter);
    } THEN {
        if (enabled)
            EXPECT_MUL_EQ(dmgBefore, Q_4_12(2.0), dmgAfter);
        else
            EXPECT_EQ(dmgAfter, dmgBefore);
    }
}

// On-hit move-disable (Batch K fourth sub-group): Cursed Body has a chance (30%, always under
// DETERMINISTIC_ABILITIES — the shipping default) to disable the move that just damaged the holder. It reuses
// the same re-entrant on-hit driver — a one-line IsActiveOnHitInnate addition — delegating to the upstream
// ABILITYEFFECT_MOVE_END case so the disable / script / pop-up match the real ability. A 1:1 clean-upside copy
// (it only ever hampers the FOE), with the pop-up overwritten to the innate when the chosen ability differs.
// Tested under DETERMINISTIC_ABILITIES (like the RNG-gated Batch J members), where the disable is guaranteed.
// NB: every canon Cursed Body user is a Ghost-type, so the attacking move must not be Normal/Fighting
// (immune) — Water Gun damages both a Water/Ghost Frillish and a Dragon/Ghost Dragapult.

// Innate Cursed Body disables the attacker's move when it damages the holder, even as an INNATE. Frillish
// carries innate Cursed Body (chosen Damp); Dragapult carries it (chosen Clear Body). Feature-off leg
// proves the disable comes only from the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES + DETERMINISTIC_ABILITIES: innate Cursed Body disables the move that damages the holder")
{
    u32 species;
    enum Ability chosen;
    bool32 enabled;
    PARAMETRIZE { species = SPECIES_FRILLISH;  chosen = ABILITY_DAMP;         enabled = TRUE; }
    PARAMETRIZE { species = SPECIES_FRILLISH;  chosen = ABILITY_DAMP;         enabled = FALSE; }
    PARAMETRIZE { species = SPECIES_DRAGAPULT; chosen = ABILITY_CLEAR_BODY;   enabled = TRUE; }
    PARAMETRIZE { species = SPECIES_DRAGAPULT; chosen = ABILITY_CLEAR_BODY;   enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(species, ABILITY_CURSED_BODY));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_WATER_GUN); }
        OPPONENT(species) { Ability(chosen); } // chosen ability is NOT Cursed Body
    } WHEN {
        TURN { MOVE(player, MOVE_WATER_GUN); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(opponent, ABILITY_CURSED_BODY); // pop-up shows the innate, not the chosen ability
            MESSAGE("Wobbuffet's Water Gun was disabled!");
        } else {
            NONE_OF {
                ABILITY_POPUP(opponent, ABILITY_CURSED_BODY);
                MESSAGE("Wobbuffet's Water Gun was disabled!");
            }
        }
    } THEN {
        u32 disabledMove = player->volatiles.disabledMove; // bit-field — copy out before EXPECT_EQ
        if (enabled)
            EXPECT_EQ(disabledMove, MOVE_WATER_GUN);
        else
            EXPECT_EQ(disabledMove, MOVE_NONE);
    }
}

// A REAL Cursed Body still disables exactly once with the feature on — the driver skips an innate equal to the
// chosen ability, so it never fires twice beside the chosen-ability contact block.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES + DETERMINISTIC_ABILITIES: a chosen Cursed Body disables once, not twice, with innates on")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_FRILLISH, ABILITY_CURSED_BODY));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_WATER_GUN); }
        OPPONENT(SPECIES_FRILLISH) { Ability(ABILITY_CURSED_BODY); } // chosen == innate
    } WHEN {
        TURN { MOVE(player, MOVE_WATER_GUN); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_CURSED_BODY);
        MESSAGE("Wobbuffet's Water Gun was disabled!");
    } THEN {
        u32 disabledMove = player->volatiles.disabledMove; // bit-field — copy out before EXPECT_EQ
        EXPECT_EQ(disabledMove, MOVE_WATER_GUN);
    }
}

// Suppression parity: Gastro Acid nullifies an innate Cursed Body exactly like a real ability, so the on-hit
// driver skips it and the attacker's move is not disabled.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES + DETERMINISTIC_ABILITIES: Gastro Acid suppresses an innate Cursed Body")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_FRILLISH, ABILITY_CURSED_BODY));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_GASTRO_ACID, MOVE_WATER_GUN); }
        OPPONENT(SPECIES_FRILLISH) { Ability(ABILITY_DAMP); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_GASTRO_ACID); }
        TURN { MOVE(player, MOVE_WATER_GUN); }
    } SCENE {
        MESSAGE("The opposing Frillish's Ability was suppressed!");
        NONE_OF { ABILITY_POPUP(opponent, ABILITY_CURSED_BODY); }
    } THEN {
        u32 disabledMove = player->volatiles.disabledMove; // bit-field — copy out before EXPECT_EQ
        EXPECT_EQ(disabledMove, MOVE_NONE);
    }
}

// Batch K fifth/final sub-group: Pickpocket / Magician / Liquid Ooze (item-steal reactions + drain punish).
// All 1:1 clean-upside copies. Pickpocket is a one-line swap at the dedicated MoveEndPickpocket step; Magician
// is attacker-side, fired through the new attacker-side on-hit driver; Liquid Ooze is a passive calc modifier
// (no driver) that damages a drainer instead of healing it. Each shows an ability pop-up, so an innate that
// differs from the chosen ability overrides it to the innate.

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Liquid Ooze damages a drain-move user instead of healing it")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_GULPIN, ABILITY_LIQUID_OOZE));
        ASSUME(GetMoveEffect(MOVE_GIGA_DRAIN) == EFFECT_ABSORB);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(300); HP(150); Moves(MOVE_GIGA_DRAIN); }
        OPPONENT(SPECIES_GULPIN) { Ability(ABILITY_STICKY_HOLD); } // chosen ability is NOT Liquid Ooze
    } WHEN {
        TURN { MOVE(player, MOVE_GIGA_DRAIN); }
    } SCENE {
        HP_BAR(opponent); // Giga Drain damage
        if (enabled) {
            ABILITY_POPUP(opponent, ABILITY_LIQUID_OOZE); // pop-up shows the innate, not the chosen Sticky Hold
            HP_BAR(player); // recoil (drained amount) BEFORE the ooze message, per the script order
            MESSAGE("Wobbuffet sucked up the liquid ooze!");
        } else {
            HP_BAR(player); // normal heal
            NONE_OF { MESSAGE("Wobbuffet sucked up the liquid ooze!"); }
        }
    } THEN {
        if (enabled)
            EXPECT_LT(player->hp, 150); // took recoil equal to the drained amount
        else
            EXPECT_GT(player->hp, 150); // healed off the drain
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Liquid Ooze damages a Leech Seed drainer instead of healing it")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_GULPIN, ABILITY_LIQUID_OOZE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WYNAUT) { MaxHP(300); HP(150); Moves(MOVE_LEECH_SEED); }
        OPPONENT(SPECIES_GULPIN) { Ability(ABILITY_STICKY_HOLD); } // chosen ability is NOT Liquid Ooze
    } WHEN {
        TURN { MOVE(player, MOVE_LEECH_SEED); }
    } THEN {
        if (enabled)
            EXPECT_LT(player->hp, 150); // seeder takes recoil off Liquid Ooze
        else
            EXPECT_GT(player->hp, 150); // seeder heals
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Liquid Ooze, so the drainer heals")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_GULPIN, ABILITY_LIQUID_OOZE));
        ASSUME(GetMoveEffect(MOVE_GIGA_DRAIN) == EFFECT_ABSORB);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); MaxHP(300); HP(150); Moves(MOVE_GASTRO_ACID, MOVE_GIGA_DRAIN); }
        OPPONENT(SPECIES_GULPIN) { Ability(ABILITY_STICKY_HOLD); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_GASTRO_ACID); }
        TURN { MOVE(player, MOVE_GIGA_DRAIN); }
    } SCENE {
        MESSAGE("The opposing Gulpin's Ability was suppressed!");
        NONE_OF {
            ABILITY_POPUP(opponent, ABILITY_LIQUID_OOZE);
            MESSAGE("Wobbuffet sucked up the liquid ooze!");
        }
    } THEN {
        EXPECT_GT(player->hp, 150); // Liquid Ooze suppressed -> normal heal
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Pickpocket steals a contact attacker's held item")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SNEASEL, ABILITY_PICKPOCKET));
        ASSUME(MoveMakesContact(MOVE_TACKLE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_MAGOST_BERRY); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_SNEASEL) { Ability(ABILITY_INNER_FOCUS); } // chosen ability is NOT Pickpocket
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(opponent, ABILITY_PICKPOCKET); // pop-up shows the innate, not the chosen Inner Focus
            MESSAGE("The opposing Sneasel stole Wobbuffet's Magost Berry!");
        } else {
            NONE_OF { ABILITY_POPUP(opponent, ABILITY_PICKPOCKET); }
        }
    } THEN {
        if (enabled) {
            EXPECT_EQ(opponent->item, ITEM_MAGOST_BERRY);
            EXPECT_EQ(player->item, ITEM_NONE);
        } else {
            EXPECT_EQ(player->item, ITEM_MAGOST_BERRY);
            EXPECT_EQ(opponent->item, ITEM_NONE);
        }
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Pickpocket does not steal from a non-contact attacker")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SNEASEL, ABILITY_PICKPOCKET));
        ASSUME(!MoveMakesContact(MOVE_WATER_GUN));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_MAGOST_BERRY); Moves(MOVE_WATER_GUN); }
        OPPONENT(SPECIES_SNEASEL) { Ability(ABILITY_INNER_FOCUS); }
    } WHEN {
        TURN { MOVE(player, MOVE_WATER_GUN); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(opponent, ABILITY_PICKPOCKET); }
    } THEN {
        EXPECT_EQ(player->item, ITEM_MAGOST_BERRY);
        EXPECT_EQ(opponent->item, ITEM_NONE);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Magician steals a held item off a target it damages")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_DELPHOX, ABILITY_MAGICIAN));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_DELPHOX) { Ability(ABILITY_BLAZE); Moves(MOVE_SCRATCH); } // chosen ability is NOT Magician
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_MAGOST_BERRY); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_MAGICIAN); // pop-up shows the innate, not the chosen Blaze
            MESSAGE("Delphox stole the opposing Wobbuffet's Magost Berry!");
        } else {
            NONE_OF { ABILITY_POPUP(player, ABILITY_MAGICIAN); }
        }
    } THEN {
        if (enabled) {
            EXPECT_EQ(player->item, ITEM_MAGOST_BERRY);
            EXPECT_EQ(opponent->item, ITEM_NONE);
        } else {
            EXPECT_EQ(opponent->item, ITEM_MAGOST_BERRY);
            EXPECT_EQ(player->item, ITEM_NONE);
        }
    }
}

// A REAL Magician still steals exactly once with the feature on — the attacker-side driver skips an innate equal
// to the chosen ability, so it never fires twice beside the chosen-ability foes-fainted block.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: a chosen Magician steals once, not twice, with innates on")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_DELPHOX, ABILITY_MAGICIAN));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_DELPHOX) { Ability(ABILITY_MAGICIAN); Moves(MOVE_SCRATCH); } // chosen == innate
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_MAGOST_BERRY); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_MAGICIAN);
        MESSAGE("Delphox stole the opposing Wobbuffet's Magost Berry!");
    } THEN {
        EXPECT_EQ(player->item, ITEM_MAGOST_BERRY);
        EXPECT_EQ(opponent->item, ITEM_NONE);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Magician, so it steals nothing")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_DELPHOX, ABILITY_MAGICIAN));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_DELPHOX) { Ability(ABILITY_BLAZE); Speed(50); Moves(MOVE_SCRATCH); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_MAGOST_BERRY); Speed(100); Moves(MOVE_GASTRO_ACID); }
    } WHEN {
        // Opponent (faster) suppresses the player's own innate Magician before the player attacks.
        TURN { MOVE(opponent, MOVE_GASTRO_ACID); MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("Delphox's Ability was suppressed!");
        NONE_OF { ABILITY_POPUP(player, ABILITY_MAGICIAN); }
    } THEN {
        EXPECT_EQ(opponent->item, ITEM_MAGOST_BERRY);
        EXPECT_EQ(player->item, ITEM_NONE);
    }
}

// ─── Innate Intimidate ───────────────────────────────────────────────────────
// The FIRST active, scripted SWITCH-IN innate: on switch-in the holder lowers every opposing
// battler's Attack by 1 stage. Fired through the new re-entrant switch-in driver
// (TryActivateInnateSwitchInEffects, hooked at FIRST_EVENT_BLOCK_GENERAL_ABILITIES_INNATE in
// src/battle_switch_in.c), which delegates to the upstream ABILITYEFFECT_ON_SWITCHIN case so the
// stat drop / script / pop-up — and every downstream reaction (the target's Clear Body / Own Tempo
// immunity, Defiant, etc.) — match the real ability. Mawile is the worked example: its abilities[0]
// is Hyper Cutter, so the innate Intimidate is attributable solely to the innate (and the pop-up must
// show Intimidate, not Hyper Cutter). Suppression parity holds via IsInnateActive().
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Intimidate lowers the foe's Attack on switch-in")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MAWILE, ABILITY_INTIMIDATE));
        ASSUME(gSpeciesInfo[SPECIES_MAWILE].abilities[0] != ABILITY_INTIMIDATE); // chosen Hyper Cutter; the drop is the innate's
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_MAWILE);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_INTIMIDATE); // pop-up shows the innate, not chosen Hyper Cutter
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_INTIMIDATE); }
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_ATK], enabled ? DEFAULT_STAT_STAGE - 1 : DEFAULT_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: a chosen Intimidate drops the foe once, not twice, with innates on")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_GYARADOS, ABILITY_INTIMIDATE));
        ASSUME(gSpeciesInfo[SPECIES_GYARADOS].abilities[0] == ABILITY_INTIMIDATE); // chosen == innate
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_GYARADOS);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); }
    } THEN {
        // The driver skips an innate equal to the chosen ability, so the chosen Intimidate fires alone: -1, not -2.
        EXPECT_EQ(opponent->statStages[STAT_ATK], DEFAULT_STAT_STAGE - 1);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Neutralizing Gas suppresses an innate Intimidate on switch-in")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MAWILE, ABILITY_INTIMIDATE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_MAWILE);
        OPPONENT(SPECIES_WEEZING_GALAR) { Ability(ABILITY_NEUTRALIZING_GAS); } // Neutralizing Gas is on the field
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(player, ABILITY_INTIMIDATE); }
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_ATK], DEFAULT_STAT_STAGE); // suppressed: no drop
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: the foe's Clear Body blocks an innate Intimidate's Attack drop")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MAWILE, ABILITY_INTIMIDATE));
        ASSUME(gSpeciesInfo[SPECIES_METAGROSS].abilities[0] == ABILITY_CLEAR_BODY);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_MAWILE);
        OPPONENT(SPECIES_METAGROSS);
    } WHEN {
        TURN { SWITCH(player, 1); }
    } THEN {
        // Delegation to the real Intimidate script preserves the target's Clear Body immunity.
        EXPECT_EQ(opponent->statStages[STAT_ATK], DEFAULT_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Trace copies only the chosen ability, never an innate Intimidate")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MAWILE, ABILITY_INTIMIDATE));
        ASSUME(gSpeciesInfo[SPECIES_MAWILE].abilities[0] == ABILITY_HYPER_CUTTER);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_MAWILE);
        OPPONENT(SPECIES_GARDEVOIR) { Ability(ABILITY_TRACE); }
    } WHEN {
        TURN { }
    } THEN {
        // Trace reads the primary slot only, so it copies Hyper Cutter, not the innate Intimidate.
        EXPECT_EQ(opponent->ability, ABILITY_HYPER_CUTTER);
    }
}

DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Intimidate lowers both opposing battlers' Attack")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MAWILE, ABILITY_INTIMIDATE));
        ASSUME(gSpeciesInfo[SPECIES_MAWILE].abilities[0] != ABILITY_INTIMIDATE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_MAWILE);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(playerLeft, 2); MOVE(playerRight, MOVE_CELEBRATE); \
               MOVE(opponentLeft, MOVE_CELEBRATE); MOVE(opponentRight, MOVE_CELEBRATE); }
    } SCENE {
        ABILITY_POPUP(playerLeft, ABILITY_INTIMIDATE);
    } THEN {
        EXPECT_EQ(opponentLeft->statStages[STAT_ATK], DEFAULT_STAT_STAGE - 1);
        EXPECT_EQ(opponentRight->statStages[STAT_ATK], DEFAULT_STAT_STAGE - 1);
    }
}

// ─── Innate Anticipation / Forewarn / Frisk (switch-in information reveals) ────
// Batch L's second sub-group: three information-reveal switch-in innates that reuse the same driver as
// Intimidate (a one-line IsActiveSwitchInInnate addition each), delegating to the upstream
// ABILITYEFFECT_ON_SWITCHIN case so the message / reveal / script / pop-up match the real ability. All
// three are pure 1:1 clean-upside boons; each effect site (src/battle_util.c) forces the pop-up to show
// the innate when the chosen ability differs. Vehicles chosen so abilities[0] != the innate, so the
// effect and the pop-up are attributable solely to the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Frisk reveals the foe's held item on switch-in")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SHUPPET, ABILITY_FRISK));
        ASSUME(gSpeciesInfo[SPECIES_SHUPPET].abilities[0] != ABILITY_FRISK); // chosen Insomnia; the reveal is the innate's
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_SHUPPET);
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_LEFTOVERS); }
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_FRISK); // pop-up shows the innate, not chosen Insomnia
            MESSAGE("Shuppet frisked the opposing Wobbuffet and found its Leftovers!");
        } else {
            NONE_OF { ABILITY_POPUP(player, ABILITY_FRISK); }
        }
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Forewarn reveals a foe's move on switch-in")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_HYPNO, ABILITY_FOREWARN));
        ASSUME(gSpeciesInfo[SPECIES_HYPNO].abilities[0] != ABILITY_FOREWARN); // chosen Insomnia; the reveal is the innate's
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_HYPNO);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_FOREWARN); // pop-up shows the innate, not chosen Insomnia
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_FOREWARN); }
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Anticipation warns of a foe's super-effective move")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_FERROTHORN, ABILITY_ANTICIPATION));
        ASSUME(gSpeciesInfo[SPECIES_FERROTHORN].abilities[0] != ABILITY_ANTICIPATION); // chosen Iron Barbs; the warning is the innate's
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_FERROTHORN);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_EMBER); } // Fire is super effective vs Grass/Steel
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_ANTICIPATION); // pop-up shows the innate, not chosen Iron Barbs
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_ANTICIPATION); }
    }
}

// Suppression parity: an innate reveal honors Neutralizing Gas exactly like the real ability (IsInnateActive()).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Neutralizing Gas suppresses an innate Frisk on switch-in")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SHUPPET, ABILITY_FRISK));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_SHUPPET);
        OPPONENT(SPECIES_WEEZING_GALAR) { Ability(ABILITY_NEUTRALIZING_GAS); Item(ITEM_LEFTOVERS); }
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(player, ABILITY_FRISK); } // suppressed: no reveal
    }
}

// ─── Innate Download / Supersweet Syrup (switch-in stat changes) ──────────────
// Batch L's third sub-group: two switch-in stat-change innates that reuse the same driver as Intimidate
// (a one-line IsActiveSwitchInInnate addition each), delegating to the upstream ABILITYEFFECT_ON_SWITCHIN
// case so the stat change / script / pop-up match the real ability. Both are 1:1 clean-upside boons
// (self-boost / foe-debuff); each effect site (src/battle_util.c) forces the pop-up to the innate when the
// chosen ability differs. Vehicles chosen so the chosen ability != the innate, so the stat swing and the
// pop-up are attributable solely to the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Download boosts the holder's Sp. Atk on switch-in")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_PORYGON_Z, ABILITY_DOWNLOAD));
        ASSUME(gSpeciesInfo[SPECIES_PORYGON_Z].abilities[0] != ABILITY_DOWNLOAD); // chosen Adaptability; the boost is the innate's
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_PORYGON_Z) { Ability(ABILITY_ADAPTABILITY); }
        OPPONENT(SPECIES_WOBBUFFET); // Def == Sp. Def, so Download picks Sp. Atk
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_DOWNLOAD); // pop-up shows the innate, not chosen Adaptability
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_DOWNLOAD); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPATK], enabled ? DEFAULT_STAT_STAGE + 1 : DEFAULT_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: a chosen Download boosts once, not twice, with innates on")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_GENESECT, ABILITY_DOWNLOAD));
        ASSUME(gSpeciesInfo[SPECIES_GENESECT].abilities[0] == ABILITY_DOWNLOAD); // chosen == innate
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_GENESECT);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); }
    } THEN {
        // The driver skips an innate equal to the chosen ability, so the chosen Download fires alone: +1, not +2.
        EXPECT_EQ(player->statStages[STAT_SPATK], DEFAULT_STAT_STAGE + 1);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Neutralizing Gas suppresses an innate Download on switch-in")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_PORYGON_Z, ABILITY_DOWNLOAD));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_PORYGON_Z) { Ability(ABILITY_ADAPTABILITY); }
        OPPONENT(SPECIES_WEEZING_GALAR) { Ability(ABILITY_NEUTRALIZING_GAS); }
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(player, ABILITY_DOWNLOAD); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPATK], DEFAULT_STAT_STAGE); // suppressed: no boost
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Supersweet Syrup lowers the foe's evasiveness on switch-in")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_HYDRAPPLE, ABILITY_SUPERSWEET_SYRUP));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_HYDRAPPLE) { Ability(ABILITY_REGENERATOR); } // chosen differs from the innate; the drop is the innate's
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_SUPERSWEET_SYRUP); // pop-up shows the innate, not chosen Regenerator
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_SUPERSWEET_SYRUP); }
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_EVASION], enabled ? DEFAULT_STAT_STAGE - 1 : DEFAULT_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: an innate Supersweet Syrup fires only once per battle")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_HYDRAPPLE, ABILITY_SUPERSWEET_SYRUP));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_HYDRAPPLE) { Ability(ABILITY_REGENERATOR); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); } // Hydrapple in → fires (first entry)
        TURN { SWITCH(player, 0); } // Wobbuffet in, Hydrapple out
        TURN { SWITCH(player, 1); } // Hydrapple back in → must NOT re-fire
    } SCENE {
        ABILITY_POPUP(player, ABILITY_SUPERSWEET_SYRUP); // fires on the first entry
        NONE_OF { ABILITY_POPUP(player, ABILITY_SUPERSWEET_SYRUP); } // once-per-battle: never again
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_EVASION], DEFAULT_STAT_STAGE - 1); // dropped once, not twice
    }
}

// ─── Innate Unnerve / Hospitality (switch-in effects) ─────────────────────────
// Batch L's fourth/final sub-group. Both reuse the same switch-in driver as Intimidate, but at their OWN
// switch-in phases rather than through ABILITYEFFECT_ON_SWITCHIN: Unnerve delegates to the upstream
// ABILITYEFFECT_UNNERVE case (hooked from the new SWITCH_IN_EVENTS_UNNERVE_INNATE event), Hospitality to
// ABILITYEFFECT_DEPENDS_ON_ALLY (hooked from the new SECOND_EVENT_ABILITIES_INNATE step). Both are 1:1
// clean-upside boons (foe Berry denial / ally heal); each effect site (src/battle_util.c) forces the
// pop-up to the innate when the chosen ability differs. Vehicles carry a chosen ability != the innate, so
// the effect and the pop-up are attributable solely to the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Unnerve denies the foe its Berry")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_JOLTIK, ABILITY_UNNERVE));
        ASSUME(gSpeciesInfo[SPECIES_JOLTIK].abilities[0] != ABILITY_UNNERVE); // chosen Compound Eyes; the denial is the innate's
        ASSUME(gItemsInfo[ITEM_RAWST_BERRY].holdEffect == HOLD_EFFECT_CURE_BRN);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_JOLTIK) { Ability(ABILITY_COMPOUND_EYES); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_RAWST_BERRY); Status1(STATUS1_BURN); }
    } WHEN {
        TURN {}
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_UNNERVE); // pop-up shows the innate, not chosen Compound Eyes
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_UNNERVE); }
    } THEN {
        // With the innate active the foe can't eat its Rawst Berry, so it stays burned; with the feature
        // off (no Unnerve) the Berry is eaten and cures the burn.
        EXPECT_EQ(opponent->status1 & STATUS1_BURN, enabled ? STATUS1_BURN : 0);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Neutralizing Gas suppresses an innate Unnerve")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_JOLTIK, ABILITY_UNNERVE));
        ASSUME(gItemsInfo[ITEM_RAWST_BERRY].holdEffect == HOLD_EFFECT_CURE_BRN);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_JOLTIK) { Ability(ABILITY_COMPOUND_EYES); }
        OPPONENT(SPECIES_WEEZING_GALAR) { Ability(ABILITY_NEUTRALIZING_GAS); Item(ITEM_RAWST_BERRY); Status1(STATUS1_BURN); }
    } WHEN {
        TURN {}
    } SCENE {
        NONE_OF { ABILITY_POPUP(player, ABILITY_UNNERVE); }
    } THEN {
        // Neutralizing Gas suppresses the innate Unnerve, so the foe eats its Rawst Berry and cures its burn.
        EXPECT_EQ(opponent->status1 & STATUS1_BURN, 0);
    }
}

DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Hospitality heals the ally on switch-in")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SINISTCHA, ABILITY_HOSPITALITY));
        ASSUME(gSpeciesInfo[SPECIES_SINISTCHA].abilities[2] == ABILITY_HEATPROOF); // a real non-Hospitality slot
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET) { HP(75); MaxHP(100); }
        PLAYER(SPECIES_SINISTCHA) { Ability(ABILITY_HEATPROOF); } // chosen differs; the heal is the innate's
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(playerLeft, 2); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(playerLeft, ABILITY_HOSPITALITY); // pop-up shows the innate, not chosen Heatproof
            HP_BAR(playerRight, damage: -25); // ally restored 25% of its max HP
        } else {
            NONE_OF {
                ABILITY_POPUP(playerLeft, ABILITY_HOSPITALITY);
                HP_BAR(playerRight, damage: -25);
            }
        }
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Hospitality does nothing outside a double battle")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SINISTCHA, ABILITY_HOSPITALITY));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_SINISTCHA) { Ability(ABILITY_HEATPROOF); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(player, ABILITY_HOSPITALITY); } // singles: no ally to heal
    }
}

// ─── Innate Defiant / Competitive (Batch M, stat-drop reactions) ──────────────
// When a FOE lowers one of the holder's stats (a move, Intimidate, or Sticky Web),
// Defiant raises the holder's Attack and Competitive its Sp. Atk by 2 stages. Both are
// 1:1 clean-upside copies wired at the single scripted reaction site BS_TryDefiantRattled
// (src/battle_script_commands.c): an innate is credited when the chosen ability isn't
// itself reactive, with the pop-up overwritten to the innate. Because the reaction funnels
// through the shared stat-drop message, an innate also reacts to Intimidate for free.

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Defiant sharply raises Attack when a foe lowers a stat")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_BRAVIARY, ABILITY_DEFIANT));
        ASSUME(gSpeciesInfo[SPECIES_BRAVIARY].abilities[0] != ABILITY_DEFIANT);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_BRAVIARY) { Ability(ABILITY_KEEN_EYE); } // chosen Keen Eye; Defiant only as innate
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_GROWL); }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, player); // Growl lowers Attack
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_DEFIANT); // pop-up shows the innate, not chosen Keen Eye
            ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, player); // Defiant re-raises it
        }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], enabled ? DEFAULT_STAT_STAGE + 1 : DEFAULT_STAT_STAGE - 1);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Competitive sharply raises Sp. Atk when a foe lowers a stat")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_BOLTUND, ABILITY_COMPETITIVE));
        ASSUME(gSpeciesInfo[SPECIES_BOLTUND].abilities[0] != ABILITY_COMPETITIVE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_BOLTUND) { Ability(ABILITY_STRONG_JAW); } // chosen Strong Jaw; Competitive only as innate
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_GROWL); } // lowers Attack, but Competitive reacts by raising Sp. Atk
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, player); // Growl lowers Attack
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_COMPETITIVE);
            ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, player); // Competitive raises Sp. Atk
        }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE - 1); // the dropped stat stays dropped
        EXPECT_EQ(player->statStages[STAT_SPATK], enabled ? DEFAULT_STAT_STAGE + 2 : DEFAULT_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Defiant reacts to a foe's Intimidate")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_BRAVIARY, ABILITY_DEFIANT));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_BRAVIARY) { Ability(ABILITY_KEEN_EYE); } // chosen Keen Eye; Defiant only as innate
        OPPONENT(SPECIES_ARCANINE) { Ability(ABILITY_INTIMIDATE); }
    } WHEN {
        TURN {}
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_INTIMIDATE);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, player); // Intimidate lowers Attack
        ABILITY_POPUP(player, ABILITY_DEFIANT);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, player); // Defiant re-raises it
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE + 1);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Defiant")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_BRAVIARY, ABILITY_DEFIANT));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_BRAVIARY) { Ability(ABILITY_KEEN_EYE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID, MOVE_GROWL); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_GASTRO_ACID); } // suppresses Braviary's abilities, the innate Defiant included
        TURN { MOVE(opponent, MOVE_GROWL); }
    } SCENE {
        MESSAGE("Braviary's Ability was suppressed!");
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, player); // Growl lowers Attack
        NONE_OF { ABILITY_POPUP(player, ABILITY_DEFIANT); } // suppressed -> no reaction
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE - 1);
    }
}

// AI innate-awareness: the Intimidate-cycling switch heuristic (ShouldSwitchIfIntimidateBenefit,
// src/battle_ai_switch.c) is a DEDICATED read, not the shared calc, so it had to be wired. The AI
// won't switch out to re-fire Intimidate at a foe whose innate Defiant would just bank a +2 from it.
// Mirrors the chosen-Defiant test in test/battle/ai/ai_switching.c, but Braviary carries Defiant
// only as an innate here (chosen Keen Eye).
AI_SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: AI won't cycle Intimidate into an innate-Defiant foe")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_BRAVIARY, ABILITY_DEFIANT));
        ASSUME(gSpeciesInfo[SPECIES_BRAVIARY].abilities[0] != ABILITY_DEFIANT);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_SWITCHING);
        PLAYER(SPECIES_BRAVIARY) { Ability(ABILITY_KEEN_EYE); Moves(MOVE_TACKLE); } // innate-only Defiant
        OPPONENT(SPECIES_ARCANINE) { Ability(ABILITY_INTIMIDATE); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_TACKLE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); EXPECT_MOVE(opponent, MOVE_TACKLE); } // sees innate Defiant -> stays in
    }
}

// ─── Innate on-hit stat boosts (Batch M: Justified / Stamina / Water Compaction / Anger Point) ─────
//
// Four on-hit reactions that fire from the upstream ABILITYEFFECT_MOVE_END case, so each reuses the
// existing on-hit driver (IsActiveOnHitInnate) — a one-line addition each. When the holder is hit:
// Justified raises Attack +1 on a Dark move, Stamina raises Defense +1 on any move, Water Compaction
// raises Defense +2 on a Water move, and Anger Point maxes Attack on a critical hit. All are 1:1
// clean-upside copies (they only ever help the holder). Each test's feature-off leg proves the boost
// comes only from the innate; the pop-up shows the innate, not the (different) chosen ability.

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Justified raises Attack when hit by a Dark move")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_ABSOL, ABILITY_JUSTIFIED));
        ASSUME(GetMoveType(MOVE_BITE) == TYPE_DARK);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_ABSOL) { Ability(ABILITY_PRESSURE); } // chosen ability is NOT Justified
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_BITE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_BITE); }
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_JUSTIFIED); // pop-up shows the innate, not the chosen Pressure
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_JUSTIFIED); }
    } THEN {
        if (enabled)
            EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE + 1);
        else
            EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Stamina raises Defense when hit")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MUDSDALE, ABILITY_STAMINA));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_MUDSDALE) { Ability(ABILITY_OWN_TEMPO); } // chosen ability is NOT Stamina
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_TACKLE); }
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_STAMINA); // pop-up shows the innate, not the chosen Own Tempo
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_STAMINA); }
    } THEN {
        if (enabled)
            EXPECT_EQ(player->statStages[STAT_DEF], DEFAULT_STAT_STAGE + 1);
        else
            EXPECT_EQ(player->statStages[STAT_DEF], DEFAULT_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Water Compaction sharply raises Defense when hit by a Water move")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_PALOSSAND, ABILITY_WATER_COMPACTION));
        ASSUME(GetMoveType(MOVE_WATER_GUN) == TYPE_WATER);
        ASSUME(gSpeciesInfo[SPECIES_PALOSSAND].abilities[0] == ABILITY_WATER_COMPACTION);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_PALOSSAND) { Ability(ABILITY_SAND_VEIL); } // chosen ability is NOT Water Compaction
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_WATER_GUN); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_WATER_GUN); }
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_WATER_COMPACTION); // pop-up shows the innate, not the chosen Sand Veil
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_WATER_COMPACTION); }
    } THEN {
        if (enabled)
            EXPECT_EQ(player->statStages[STAT_DEF], DEFAULT_STAT_STAGE + 2);
        else
            EXPECT_EQ(player->statStages[STAT_DEF], DEFAULT_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Anger Point maxes Attack on a critical hit")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CRABOMINABLE, ABILITY_ANGER_POINT));
        ASSUME(MoveAlwaysCrits(MOVE_FROST_BREATH));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_CRABOMINABLE) { Ability(ABILITY_HYPER_CUTTER); } // chosen ability is NOT Anger Point
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_FROST_BREATH); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_FROST_BREATH); }
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_ANGER_POINT); // pop-up shows the innate, not the chosen Hyper Cutter
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_ANGER_POINT); }
    } THEN {
        if (enabled)
            EXPECT_EQ(player->statStages[STAT_ATK], MAX_STAT_STAGE);
        else
            EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE);
    }
}

// Suppression parity: Gastro Acid nullifies an innate on-hit stat boost exactly like a real ability, so
// the on-hit driver skips it and no boost fires (Justified is the representative of the sub-group's driver).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Justified")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_ABSOL, ABILITY_JUSTIFIED));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_GASTRO_ACID, MOVE_BITE); }
        OPPONENT(SPECIES_ABSOL) { Ability(ABILITY_PRESSURE); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_GASTRO_ACID); }
        TURN { MOVE(player, MOVE_BITE); }
    } SCENE {
        MESSAGE("The opposing Absol's Ability was suppressed!");
        NONE_OF { ABILITY_POPUP(opponent, ABILITY_JUSTIFIED); }
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_ATK], DEFAULT_STAT_STAGE);
    }
}

// ─── Innate fear-response Speed boosts (Batch M: Rattled / Steadfast) ───────────────────────────────
//
// Both raise the holder's Speed +1 when it is frightened. Rattled has TWO triggers, spanning the two
// Batch M sites already opened: a Dark/Ghost/Bug hit (reuses the on-hit driver, like Justified) and a
// foe's Intimidate (credited at BS_TryDefiantRattled beside Defiant/Competitive, Gen8+ only). Steadfast
// reacts to flinching (made innate-aware at the CancelerFlinch site). Both are 1:1 clean-upside copies;
// the pop-up shows the innate, not the (different) chosen ability.

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Rattled raises Speed when hit by a Dark move")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MAGIKARP, ABILITY_RATTLED));
        ASSUME(GetMoveType(MOVE_BITE) == TYPE_DARK);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_MAGIKARP) { Ability(ABILITY_SWIFT_SWIM); } // chosen ability is NOT Rattled
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_BITE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_BITE); }
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_RATTLED); // pop-up shows the innate, not the chosen Swift Swim
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_RATTLED); }
    } THEN {
        if (enabled)
            EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE + 1);
        else
            EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Rattled reacts to a foe's Intimidate (Gen8+)")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MEOWTH_ALOLA, ABILITY_RATTLED));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(B_UPDATED_INTIMIDATE, GEN_8);
        PLAYER(SPECIES_MEOWTH_ALOLA) { Ability(ABILITY_TECHNICIAN); } // chosen Technician; Rattled only as innate
        OPPONENT(SPECIES_ARCANINE) { Ability(ABILITY_INTIMIDATE); }
    } WHEN {
        TURN {}
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_INTIMIDATE);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, player); // Intimidate lowers Attack
        ABILITY_POPUP(player, ABILITY_RATTLED);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, player); // Rattled raises Speed
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE - 1);
        EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE + 1);
    }
}

// Suppression parity: Gastro Acid nullifies an innate Rattled exactly like a real ability, so the on-hit
// driver skips it and no boost fires.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Rattled")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MAGIKARP, ABILITY_RATTLED));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_GASTRO_ACID, MOVE_BITE); }
        OPPONENT(SPECIES_MAGIKARP) { Ability(ABILITY_SWIFT_SWIM); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_GASTRO_ACID); }
        TURN { MOVE(player, MOVE_BITE); }
    } SCENE {
        MESSAGE("The opposing Magikarp's Ability was suppressed!");
        NONE_OF { ABILITY_POPUP(opponent, ABILITY_RATTLED); }
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_SPEED], DEFAULT_STAT_STAGE);
    }
}

// AI innate-awareness: the Intimidate-cycling switch heuristic (ShouldSwitchIfIntimidateBenefit,
// src/battle_ai_switch.c) is a DEDICATED read, so it was wired for Rattled the same way as Defiant. Under
// Gen8+ a foe's Rattled turns our Intimidate into a +1 Speed for it, so the AI won't cycle its Intimidator
// out to re-fire it. Braviary-style test, but the foe carries Rattled only as an innate (chosen Swift Swim).
AI_SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: AI won't cycle Intimidate into an innate-Rattled foe (Gen8+)")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MAGIKARP, ABILITY_RATTLED));
        ASSUME(gSpeciesInfo[SPECIES_MAGIKARP].abilities[0] != ABILITY_RATTLED);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(B_UPDATED_INTIMIDATE, GEN_8);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_SWITCHING);
        PLAYER(SPECIES_MAGIKARP) { Ability(ABILITY_SWIFT_SWIM); Moves(MOVE_SPLASH); } // innate-only Rattled
        OPPONENT(SPECIES_ARCANINE) { Ability(ABILITY_INTIMIDATE); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_TACKLE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); EXPECT_MOVE(opponent, MOVE_TACKLE); } // sees innate Rattled -> stays in
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Steadfast raises Speed when the holder flinches")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MACHAMP, ABILITY_STEADFAST));
        ASSUME(gSpeciesInfo[SPECIES_MACHAMP].abilities[0] != ABILITY_STEADFAST);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_MACHAMP) { Ability(ABILITY_GUTS); Speed(50); Moves(MOVE_TACKLE); } // chosen ability is NOT Steadfast
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_FAKE_OUT); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_FAKE_OUT); MOVE(player, MOVE_TACKLE); }
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_STEADFAST); // pop-up shows the innate, not the chosen Guts
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_STEADFAST); }
    } THEN {
        if (enabled)
            EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE + 1);
        else
            EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE);
    }
}

// Contradiction omission: Steadfast is deliberately NOT given to the Riolu/Lucario line, whose innate Inner
// Focus prevents flinching outright — so an innate Steadfast could never fire (the Own-Tempo-vs-Tangled-Feet
// class of conflict). Inner Focus (never flinch) is the stronger, already-wired boon, so Steadfast is dropped.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Steadfast is dropped on the Lucario line (innate Inner Focus wins)")
{
    GIVEN {
        ASSUME(!SpeciesHasInnate(SPECIES_LUCARIO, ABILITY_STEADFAST));
        ASSUME(!SpeciesHasInnate(SPECIES_RIOLU, ABILITY_STEADFAST));
        ASSUME(SpeciesHasInnate(SPECIES_LUCARIO, ABILITY_INNER_FOCUS));
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_LUCARIO) { Ability(ABILITY_JUSTIFIED); Speed(50); Moves(MOVE_TACKLE); } // innate Inner Focus
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_FAKE_OUT); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_FAKE_OUT); MOVE(player, MOVE_TACKLE); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(player, ABILITY_STEADFAST); } // never flinches -> Steadfast can't fire
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, player); // Inner Focus: not flinched, Tackle executes
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE); // no Steadfast boost
    }
}

// ─── Innate KO / on-damage / on-faint stat boosts (Batch M: Moxie / Berserk / Soul-Heart) ───────────
//
// Batch M's fourth and final sub-group. Moxie raises Attack +1 for each foe the holder knocks out
// (fired from the upstream ABILITYEFFECT_MOVE_END_FOES_FAINTED case, a one-line addition to the
// attacker-side on-hit driver, beside Magician). Berserk raises Sp. Atk +1 when an attack drops the
// holder's HP from above 1/2 to 1/2 or less (fired from the per-damaged-battler ABILITYEFFECT_COLOR_CHANGE
// step, so it adds a small on-damage driver at the new MOVEEND_COLOR_CHANGE_INNATE step). Soul-Heart
// raises Sp. Atk +1 whenever ANY Pokémon faints (made innate-aware at the BS_TryActivateSoulheart native
// command). All are 1:1 clean-upside copies; the pop-up shows the innate, not the (different) chosen ability.

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Moxie raises Attack when the holder knocks out a foe")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_HERACROSS, ABILITY_MOXIE));
        ASSUME(gSpeciesInfo[SPECIES_HERACROSS].abilities[0] != ABILITY_MOXIE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_HERACROSS) { Ability(ABILITY_GUTS); Moves(MOVE_TACKLE); } // chosen Guts; Moxie only as innate
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_ZIGZAGOON); // a second foe, so KOing the first doesn't end the battle
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); SEND_OUT(opponent, 1); } // KOs the first foe; the second replaces it
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_MOXIE); // pop-up shows the innate, not the chosen Guts
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_MOXIE); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], enabled ? DEFAULT_STAT_STAGE + 1 : DEFAULT_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Berserk raises Sp. Atk when an attack drops the holder to half HP")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_DRAMPA, ABILITY_BERSERK));
        ASSUME(gSpeciesInfo[SPECIES_DRAMPA].abilities[0] == ABILITY_BERSERK);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_DRAMPA) { Ability(ABILITY_CLOUD_NINE); } // chosen Cloud Nine (HA); Berserk only as innate
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SUPER_FANG); } // halves HP: full -> exactly 1/2, crossing the threshold
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUPER_FANG); }
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_BERSERK); // pop-up shows the innate, not the chosen Cloud Nine
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_BERSERK); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPATK], enabled ? DEFAULT_STAT_STAGE + 1 : DEFAULT_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Soul-Heart raises Sp. Atk when a Pokémon faints")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SPIRITOMB, ABILITY_SOUL_HEART)); // flavor pick (Magearna is sole-Soul-Heart)
        ASSUME(gSpeciesInfo[SPECIES_SPIRITOMB].abilities[0] != ABILITY_SOUL_HEART);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_SPIRITOMB) { Ability(ABILITY_PRESSURE); Moves(MOVE_TACKLE); } // chosen Pressure; Soul-Heart only as innate
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_ZIGZAGOON); // a second foe, so the faint doesn't end the battle
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); SEND_OUT(opponent, 1); } // KOs the first foe -> a Pokémon faints; the second replaces it
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_SOUL_HEART); // pop-up shows the innate, not the chosen Pressure
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_SOUL_HEART); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPATK], enabled ? DEFAULT_STAT_STAGE + 1 : DEFAULT_STAT_STAGE);
    }
}

// Suppression parity: Gastro Acid nullifies the new on-damage innate driver (Berserk) exactly like a real
// ability, so the driver skips it and no boost fires.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Berserk")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_DRAMPA, ABILITY_BERSERK));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_GASTRO_ACID, MOVE_SUPER_FANG); }
        OPPONENT(SPECIES_DRAMPA) { Ability(ABILITY_CLOUD_NINE); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_GASTRO_ACID); } // suppresses Drampa's abilities, the innate Berserk included
        TURN { MOVE(player, MOVE_SUPER_FANG); } // drops Drampa to half HP
    } SCENE {
        MESSAGE("The opposing Drampa's Ability was suppressed!");
        NONE_OF { ABILITY_POPUP(opponent, ABILITY_BERSERK); } // suppressed -> no reaction
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_SPATK], DEFAULT_STAT_STAGE);
    }
}

// ───────────────────────────────────────────────────────────────────────────
// Batch U — ally-support: Battery / Power Spot / Telepathy / Aroma Veil / Flower Veil
// ───────────────────────────────────────────────────────────────────────────

// Battery: an innate Battery on the partner boosts the attacker's SPECIAL moves x1.3. Charjabug is sole
// Battery, so a forced chosen Sturdy isolates the innate; with the feature off there is no boost.
DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Battery boosts an ally's special move", s16 damage)
{
    u32 feature;
    PARAMETRIZE { feature = FALSE; }
    PARAMETRIZE { feature = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CHARJABUG, ABILITY_BATTERY));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, feature);
        PLAYER(SPECIES_CHARJABUG) { Ability(ABILITY_STURDY); } // chosen Sturdy; Battery only via the innate
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_WATER_GUN); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerRight, MOVE_WATER_GUN, target: opponentLeft); }
    } SCENE {
        HP_BAR(opponentLeft, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.3), results[1].damage); // off: base; on: +30%
    }
}

// Battery does NOT boost physical moves — an innate Battery leaves the ally's Tackle untouched.
DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Battery leaves an ally's physical move unboosted", s16 damage)
{
    u32 feature;
    PARAMETRIZE { feature = FALSE; }
    PARAMETRIZE { feature = TRUE; }
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, feature);
        PLAYER(SPECIES_CHARJABUG) { Ability(ABILITY_STURDY); }
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerRight, MOVE_TACKLE, target: opponentLeft); }
    } SCENE {
        HP_BAR(opponentLeft, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_EQ(results[0].damage, results[1].damage); // Battery is special-only -> no change
    }
}

// Power Spot: an innate Power Spot on the partner boosts ALL the attacker's moves x1.3 (here a physical
// Tackle). Stonjourner is sole Power Spot, so a forced chosen Sturdy isolates the innate.
DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Power Spot boosts an ally's move", s16 damage)
{
    u32 feature;
    PARAMETRIZE { feature = FALSE; }
    PARAMETRIZE { feature = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_STONJOURNER, ABILITY_POWER_SPOT));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, feature);
        PLAYER(SPECIES_STONJOURNER) { Ability(ABILITY_STURDY); } // chosen Sturdy; Power Spot only via the innate
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerRight, MOVE_TACKLE, target: opponentLeft); }
    } SCENE {
        HP_BAR(opponentLeft, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.3), results[1].damage); // off: base; on: +30%
    }
}

// Telepathy: an innate Telepathy holder takes no damage from its own ally's spread move. Elgyem carries
// Telepathy plus other real slots, so a chosen Synchronize keeps Telepathy on the innate only.
DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Telepathy dodges an ally's spread move")
{
    u32 feature;
    PARAMETRIZE { feature = FALSE; }
    PARAMETRIZE { feature = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_ELGYEM, ABILITY_TELEPATHY));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, feature);
        PLAYER(SPECIES_ELGYEM) { Ability(ABILITY_SYNCHRONIZE); } // chosen Synchronize; Telepathy via the innate
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_EARTHQUAKE); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerRight, MOVE_EARTHQUAKE); }
    } THEN {
        if (feature)
            EXPECT_EQ(playerLeft->hp, playerLeft->maxHP); // Telepathy dodged the ally Earthquake
        else
            EXPECT_LT(playerLeft->hp, playerLeft->maxHP); // no innate -> took the ally Earthquake
    }
}

// Aroma Veil (script jumpifability path): an innate Aroma Veil shields the whole side from Taunt. This
// exercises the central Cmd_jumpifability side-case fallback. Spritzee is Healer/Aroma Veil, so a chosen
// Healer carries Aroma Veil on the innate only; the pop-up must name Aroma Veil, not the chosen ability.
DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Aroma Veil shields the side from Taunt")
{
    u32 feature;
    PARAMETRIZE { feature = FALSE; }
    PARAMETRIZE { feature = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SPRITZEE, ABILITY_AROMA_VEIL));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, feature);
        PLAYER(SPECIES_SPRITZEE) { Ability(ABILITY_HEALER); } // chosen Healer; Aroma Veil via the innate
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TAUNT); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponentLeft, MOVE_TAUNT, target: playerRight); }
    } SCENE {
        if (feature)
            ABILITY_POPUP(playerLeft, ABILITY_AROMA_VEIL); // Spritzee's innate shields its partner
    } THEN {
        if (feature)
            EXPECT(playerRight->volatiles.tauntTimer == 0); // partner not taunted
        else
            EXPECT(playerRight->volatiles.tauntTimer != 0); // taunted
    }
}

// Aroma Veil (C-guard path): an innate Aroma Veil also blocks Attract (Cmd_tryinfatuating / BS_TrySetInfatuation).
DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Aroma Veil shields the side from Attract")
{
    u32 feature;
    PARAMETRIZE { feature = FALSE; }
    PARAMETRIZE { feature = TRUE; }
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, feature);
        PLAYER(SPECIES_SPRITZEE) { Ability(ABILITY_HEALER); } // innate Aroma Veil
        PLAYER(SPECIES_NIDORAN_F) { Gender(MON_FEMALE); }
        OPPONENT(SPECIES_NIDORAN_M) { Gender(MON_MALE); Moves(MOVE_ATTRACT); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponentLeft, MOVE_ATTRACT, target: playerRight); }
    } THEN {
        if (feature)
            EXPECT(playerRight->volatiles.infatuation == 0); // Aroma Veil blocked it for the partner
        else
            EXPECT(playerRight->volatiles.infatuation != 0); // infatuated
    }
}

// Flower Veil: an innate Flower Veil shields a Grass ally from non-volatile status. Florges is Fairy, so it
// protects its Grass partner (not itself). Chosen Symbiosis carries Flower Veil on the innate only.
DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Flower Veil shields a Grass ally from status")
{
    u32 feature;
    PARAMETRIZE { feature = FALSE; }
    PARAMETRIZE { feature = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_FLORGES, ABILITY_FLOWER_VEIL));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, feature);
        PLAYER(SPECIES_FLORGES) { Ability(ABILITY_SYMBIOSIS); } // chosen Symbiosis; Flower Veil via the innate
        PLAYER(SPECIES_TANGELA); // Grass ally
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GLARE); } // non-powder status (Grass is immune to powder moves)
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponentLeft, MOVE_GLARE, target: playerRight); }
    } SCENE {
        if (feature)
            ABILITY_POPUP(playerLeft, ABILITY_FLOWER_VEIL); // Florges shields its Grass partner
    } THEN {
        if (feature)
            EXPECT_EQ(playerRight->status1 & STATUS1_PARALYSIS, 0); // ally not paralyzed
        else
            EXPECT_NE(playerRight->status1 & STATUS1_PARALYSIS, 0); // paralyzed
    }
}

// Flower Veil also blocks stat drops on a Grass ally (single-target Confide -> Sp. Atk).
DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Flower Veil shields a Grass ally from stat drops")
{
    u32 feature;
    PARAMETRIZE { feature = FALSE; }
    PARAMETRIZE { feature = TRUE; }
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, feature);
        PLAYER(SPECIES_FLORGES) { Ability(ABILITY_SYMBIOSIS); } // innate Flower Veil
        PLAYER(SPECIES_TANGELA); // Grass ally
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CONFIDE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponentLeft, MOVE_CONFIDE, target: playerRight); }
    } THEN {
        if (feature)
            EXPECT_EQ(playerRight->statStages[STAT_SPATK], DEFAULT_STAT_STAGE); // drop blocked
        else
            EXPECT_LT(playerRight->statStages[STAT_SPATK], DEFAULT_STAT_STAGE); // lowered
    }
}

// Suppression parity: Gastro Acid nullifies an innate Aroma Veil, so Taunt then lands on the partner.
DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Aroma Veil")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_SPRITZEE) { Ability(ABILITY_HEALER); } // innate Aroma Veil
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID, MOVE_TAUNT); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponentLeft, MOVE_GASTRO_ACID, target: playerLeft); }
        TURN { MOVE(opponentLeft, MOVE_TAUNT, target: playerRight); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(playerLeft, ABILITY_AROMA_VEIL); } // suppressed -> no shield
    } THEN {
        EXPECT(playerRight->volatiles.tauntTimer != 0); // partner taunted
    }
}

// ───────────────────────────────────────────────────────────────────────────
// Batch Y1 — promoted-from-rejected clones: Chilling Neigh / Grim Neigh / Electromorphosis
// ───────────────────────────────────────────────────────────────────────────
// Chilling Neigh / Grim Neigh are Moxie clones (raise the holder's Attack / Sp. Atk +1 per foe it KOs) and
// reuse the attacker-side on-hit driver's shared ABILITYEFFECT_MOVE_END_FOES_FAINTED case. Electromorphosis is
// a Wind Power clone minus the wind gate (charges the next Electric move when hit by ANY damaging move) and
// reuses the target-side on-hit driver's shared ABILITYEFFECT_MOVE_END case. All 1:1 clean-upside copies; the
// pop-up shows the innate, not the (different) chosen ability. Glastrier / Spectrier are sole-ability legends,
// so a forced non-slot chosen ability isolates the innate; the feature-off leg proves the effect is innate-only.

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Chilling Neigh raises Attack when the holder knocks out a foe")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_GLASTRIER, ABILITY_CHILLING_NEIGH));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_GLASTRIER) { Ability(ABILITY_PRESSURE); Moves(MOVE_TACKLE); } // forced Pressure; Chilling Neigh only as innate
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_ZIGZAGOON); // a second foe, so KOing the first doesn't end the battle
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); SEND_OUT(opponent, 1); } // KOs the first foe; the second replaces it
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_CHILLING_NEIGH); // pop-up shows the innate, not the forced Pressure
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_CHILLING_NEIGH); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], enabled ? DEFAULT_STAT_STAGE + 1 : DEFAULT_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Grim Neigh raises Sp. Atk when the holder knocks out a foe")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SPECTRIER, ABILITY_GRIM_NEIGH));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_SPECTRIER) { Ability(ABILITY_PRESSURE); Moves(MOVE_TACKLE); } // forced Pressure; Grim Neigh only as innate
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_ZIGZAGOON); // a second foe, so KOing the first doesn't end the battle
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); SEND_OUT(opponent, 1); } // KOs the first foe; the second replaces it
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_GRIM_NEIGH); // pop-up shows the innate, not the forced Pressure
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_GRIM_NEIGH); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPATK], enabled ? DEFAULT_STAT_STAGE + 1 : DEFAULT_STAT_STAGE);
    }
}

// Electromorphosis charges on ANY damaging hit (unlike Wind Power, which needs a wind move): the opponent hits
// with a non-wind Tackle, and the innate still charges Bellibolt, doubling its next Electric move.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Electromorphosis charges when hit by any damaging move")
{
    bool32 enabled;
    s16 dmgBefore, dmgAfter;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_BELLIBOLT, ABILITY_ELECTROMORPHOSIS));
        ASSUME(!IsWindMove(MOVE_TACKLE)); // proves the charge is NOT wind-gated (unlike Wind Power)
        ASSUME(GetMoveType(MOVE_NUZZLE) == TYPE_ELECTRIC);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_BELLIBOLT) { Ability(ABILITY_DAMP); Speed(10); Moves(MOVE_NUZZLE); } // chosen Damp; Electromorphosis only as innate
        OPPONENT(SPECIES_PERSIAN) { Ability(ABILITY_LIMBER); Speed(5); Moves(MOVE_TACKLE); } // Limber: never paralyzed by Nuzzle
    } WHEN {
        TURN { MOVE(player, MOVE_NUZZLE); MOVE(opponent, MOVE_TACKLE); }
        TURN { MOVE(player, MOVE_NUZZLE); MOVE(opponent, MOVE_TACKLE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_NUZZLE, player);
        HP_BAR(opponent, captureDamage: &dmgBefore);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, opponent);
        HP_BAR(player);
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_ELECTROMORPHOSIS); // pop-up shows the innate, not the chosen Damp
            MESSAGE("Being hit by Tackle charged Bellibolt with power!");
        } else {
            NONE_OF { ABILITY_POPUP(player, ABILITY_ELECTROMORPHOSIS); }
        }
        ANIMATION(ANIM_TYPE_MOVE, MOVE_NUZZLE, player);
        HP_BAR(opponent, captureDamage: &dmgAfter);
    } THEN {
        if (enabled)
            EXPECT_MUL_EQ(dmgBefore, Q_4_12(2.0), dmgAfter); // Charge doubled the second Electric move
        else
            EXPECT_EQ(dmgAfter, dmgBefore);
    }
}

// Suppression parity for the on-KO neighs is the same IsInnateActive() gate the on-hit driver already applies —
// structurally identical to Moxie (which shares the attacker-side driver) and to Electromorphosis below, and it
// is exercised by the many Gastro-Acid / Mold-Breaker suppression tests elsewhere in this file. Electromorphosis
// (which triggers on any damaging hit, so a suppression case is trivial to script) carries the parity check for
// this sub-group.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Electromorphosis")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_BELLIBOLT, ABILITY_ELECTROMORPHOSIS));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_GASTRO_ACID, MOVE_TACKLE); }
        OPPONENT(SPECIES_BELLIBOLT) { Ability(ABILITY_DAMP); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_GASTRO_ACID); } // suppresses Bellibolt's abilities, the innate Electromorphosis included
        TURN { MOVE(player, MOVE_TACKLE); }      // a damaging hit that would otherwise charge Electromorphosis
    } SCENE {
        MESSAGE("The opposing Bellibolt's Ability was suppressed!");
        NONE_OF { ABILITY_POPUP(opponent, ABILITY_ELECTROMORPHOSIS); } // suppressed -> no charge
    }
}

// Batch Y sub-group Y2 — Transistor / Dragon's Maw (flat type-power-booster clones of Steelworker /
// Rocky Payload, wired in CalcAttackStat). A forced chosen Damp keeps the innate observable (chosen !=
// the innate), mirroring the Steelworker / Rocky Payload tests; the boost lives in the shared damage
// calc, so comparing feature-off vs feature-on isolates the innate's exact multiplier.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Transistor boosts an Electric move 1.3x (GEN_9+)", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_REGIELEKI, ABILITY_TRANSISTOR));
        ASSUME(GetMoveType(MOVE_THUNDERBOLT) == TYPE_ELECTRIC);
        WITH_CONFIG(B_TRANSISTOR_BOOST, GEN_9);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_REGIELEKI) { Ability(ABILITY_DAMP); Moves(MOVE_THUNDERBOLT); } // chosen Damp; innate Transistor
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_THUNDERBOLT); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.3), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Dragon's Maw boosts a Dragon move 1.5x", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_REGIDRAGO, ABILITY_DRAGONS_MAW));
        ASSUME(GetMoveType(MOVE_DRAGON_CLAW) == TYPE_DRAGON);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_REGIDRAGO) { Ability(ABILITY_DAMP); Moves(MOVE_DRAGON_CLAW); } // chosen Damp; innate Dragon's Maw
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_DRAGON_CLAW); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.5), results[1].damage);
    }
}

// Batch Y3 — Prism Armor / Shadow Shield / Neuroforce / Supreme Overlord (damage / power calc clones).
// Prism Armor rides Filter / Solid Rock's -25%-vs-supereffective clause and Shadow Shield rides
// Multiscale's halve-at-full-HP clause (both in GetDefenderAbilitiesModifier); a forced chosen Damp keeps
// the innate observable (chosen != the innate), like the Solid Rock / Multiscale tests. Unlike their
// breakable Batch B cousins, Prism Armor / Shadow Shield are UNBREAKABLE, so an attacker's Mold Breaker
// cannot pierce them (the distinguishing contrast tests below).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Prism Armor reduces supereffective damage by 0.75", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_NECROZMA, ABILITY_PRISM_ARMOR));
        ASSUME(gTypeEffectivenessTable[TYPE_GHOST][TYPE_PSYCHIC] > UQ_4_12(1.0));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_NECROZMA) { Ability(ABILITY_DAMP); } // chosen Damp; innate Prism Armor
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SHADOW_BALL); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SHADOW_BALL); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
        MESSAGE("It's super effective!");
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.75), results[1].damage); // off: full; on: 0.75x
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker does NOT pierce an innate Prism Armor", s16 damage)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_PRESSURE; }     // innate Prism Armor applies -> reduced
    PARAMETRIZE { ability = ABILITY_MOLD_BREAKER; } // Prism Armor is unbreakable -> STILL reduced
    GIVEN {
        ASSUME(!gAbilitiesInfo[ABILITY_PRISM_ARMOR].breakable);
        ASSUME(SpeciesHasInnate(SPECIES_NECROZMA, ABILITY_PRISM_ARMOR));
        ASSUME(gTypeEffectivenessTable[TYPE_GHOST][TYPE_PSYCHIC] > UQ_4_12(1.0));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_NECROZMA) { Ability(ABILITY_DAMP); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ability); Moves(MOVE_SHADOW_BALL); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SHADOW_BALL); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_EQ(results[0].damage, results[1].damage); // both reduced equally; Mold Breaker can't pierce
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Shadow Shield halves damage at full HP", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_LUNALA, ABILITY_SHADOW_SHIELD));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_LUNALA) { Ability(ABILITY_DAMP); } // chosen Damp; innate Shadow Shield
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SURF); } // Water: neutral vs Psychic/Ghost
    } WHEN {
        TURN { MOVE(opponent, MOVE_SURF); } // lands while the holder is at full HP
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.5), results[1].damage); // off: full; on: 0.5x at full HP
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker does NOT pierce an innate Shadow Shield", s16 damage)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_PRESSURE; }     // innate Shadow Shield applies -> halved
    PARAMETRIZE { ability = ABILITY_MOLD_BREAKER; } // Shadow Shield is unbreakable -> STILL halved
    GIVEN {
        ASSUME(!gAbilitiesInfo[ABILITY_SHADOW_SHIELD].breakable);
        ASSUME(SpeciesHasInnate(SPECIES_LUNALA, ABILITY_SHADOW_SHIELD));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_LUNALA) { Ability(ABILITY_DAMP); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ability); Moves(MOVE_SURF); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SURF); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_EQ(results[0].damage, results[1].damage); // both halved equally; Mold Breaker can't pierce
    }
}

// Neuroforce is the offensive mirror of Tinted Lens: +25% to the holder's supereffective hits, wired in
// GetAttackerAbilitiesModifier. Its only canon user is the transform-only Necrozma-Ultra, so — like the
// upstream Neuroforce test — the holder Ultra-Bursts from base Necrozma-Dusk-Mane (which gives the
// end-of-battle form revert a valid target; spawning Necrozma-Ultra directly has none). A forced chosen
// Damp persists through the burst, so the post-burst Necrozma-Ultra runs chosen Damp + innate Neuroforce,
// keeping the innate observable.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Neuroforce boosts a supereffective hit 1.25x", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_NECROZMA_ULTRA, ABILITY_NEUROFORCE));
        ASSUME(GetMoveType(MOVE_SHADOW_BALL) == TYPE_GHOST);
        ASSUME(gTypeEffectivenessTable[TYPE_GHOST][TYPE_PSYCHIC] > UQ_4_12(1.0));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_NECROZMA_DUSK_MANE) { Ability(ABILITY_DAMP); Item(ITEM_ULTRANECROZIUM_Z); Moves(MOVE_CELEBRATE, MOVE_SHADOW_BALL); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE, gimmick: GIMMICK_ULTRA_BURST); } // -> Necrozma-Ultra
        TURN { MOVE(player, MOVE_SHADOW_BALL); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
        MESSAGE("It's super effective!");
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.25), results[1].damage); // off: SE; on: 1.25x SE
    }
}

// Supreme Overlord latches a +10%/fallen-teammate move-power boost at switch-in, so an innate holder
// rides the Batch L switch-in driver (ABILITYEFFECT_ON_SWITCHIN) to set the counter + show the pop-up
// (overwritten to the innate when the chosen ability differs) exactly like the real ability; the boost is
// read back in CalcAttackStat. A forced chosen Damp keeps the innate observable. One teammate faints (via
// Memento) before Kingambit switches in, so the counter latches at 1 -> +10%.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Supreme Overlord boosts power per fallen teammate", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_KINGAMBIT, ABILITY_SUPREME_OVERLORD));
        ASSUME(GetMoveEffect(MOVE_MEMENTO) == EFFECT_MEMENTO);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_PAWNIARD) { Moves(MOVE_MEMENTO); }
        PLAYER(SPECIES_KINGAMBIT) { Ability(ABILITY_DAMP); Moves(MOVE_SCRATCH); } // chosen Damp; innate Supreme Overlord
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_MEMENTO, target: opponent); SEND_OUT(player, 1); } // Pawniard faints -> Kingambit in
        TURN { MOVE(player, MOVE_SCRATCH, target: opponent); }
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_SUPREME_OVERLORD); // pop-up overwritten to the innate on switch-in
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.1), results[1].damage); // off: base; on: +10% for 1 fallen mon
    }
}

// Batch Y4 — Full Metal Body / Mind's Eye (stat-drop / accuracy / hit-trait clones).
// Full Metal Body is the UNBREAKABLE clone of Clear Body: it blocks ANY stat drop exactly like Clear Body
// (GetInnateStatDropProtector / IsAbilityBlocked, pop-up overwritten to the innate), but because its
// .breakable = FALSE an attacker's Mold Breaker cannot pierce it — the distinguishing contrast test below.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Full Metal Body blocks a stat drop")
{
    u32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SOLGALEO, ABILITY_FULL_METAL_BODY));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_SOLGALEO) { Ability(ABILITY_DAMP); } // chosen Damp; innate Full Metal Body
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_GROWL); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_GROWL); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_FULL_METAL_BODY);
            MESSAGE("Solgaleo's stats were not lowered!");
        }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], enabled ? DEFAULT_STAT_STAGE : DEFAULT_STAT_STAGE - 1);
    }
}

// Suppression contrast: unlike its breakable Clear Body cousin, Full Metal Body is UNBREAKABLE, so an
// attacker's Mold Breaker cannot pierce the innate — the stat drop is still blocked (IsInnateActive reads
// its own .breakable = FALSE, for free).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker does NOT pierce an innate Full Metal Body")
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_PRESSURE; }     // innate Full Metal Body applies -> blocked
    PARAMETRIZE { ability = ABILITY_MOLD_BREAKER; } // unbreakable -> STILL blocked
    GIVEN {
        ASSUME(!gAbilitiesInfo[ABILITY_FULL_METAL_BODY].breakable);
        ASSUME(SpeciesHasInnate(SPECIES_SOLGALEO, ABILITY_FULL_METAL_BODY));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_SOLGALEO) { Ability(ABILITY_DAMP); } // innate Full Metal Body
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ability); Moves(MOVE_GROWL); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_GROWL); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE); // blocked either way
    }
}

// Mind's Eye, Scrappy half: it lifts the Ghost immunity to Normal/Fighting moves (MulByTypeEffectiveness).
// Ursaluna-Bloodmoon's chosen ability here is a forced Damp, so the Ghost hit is solely the innate's.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Mind's Eye lets a Normal move hit a Ghost")
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_URSALUNA_BLOODMOON, ABILITY_MINDS_EYE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_URSALUNA_BLOODMOON) { Ability(ABILITY_DAMP); Moves(MOVE_SCRATCH); } // chosen Damp; innate Mind's Eye
        OPPONENT(SPECIES_GENGAR);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        if (enabled)
        {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
            HP_BAR(opponent);
        }
        else
        {
            MESSAGE("It doesn't affect the opposing Gengar…");
        }
    }
}

// Mind's Eye, Keen Eye half (evasion-ignore): under DETERMINISTIC_ACCURACY_EVASION a foe's evasion boost
// is a PP economy, and an innate Mind's Eye holder ignores it, so its move is not taxed the extra PP.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Mind's Eye ignores the target's evasion (no PP tax)")
{
    u32 enabled, expectedPP;
    PARAMETRIZE { enabled = TRUE;  expectedPP = 34; }
    PARAMETRIZE { enabled = FALSE; expectedPP = 33; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_URSALUNA_BLOODMOON, ABILITY_MINDS_EYE));
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_URSALUNA_BLOODMOON) { Ability(ABILITY_DAMP); Speed(50); Moves(MOVE_POUND, MOVE_CELEBRATE); } // chosen Damp; innate Mind's Eye
        OPPONENT(SPECIES_WOBBUFFET) { Speed(20); Moves(MOVE_DOUBLE_TEAM, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_DOUBLE_TEAM); } // opponent +1 evasion
        TURN { MOVE(player, MOVE_POUND); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->pp[0], expectedPP);
    }
}

// Mind's Eye, Keen Eye half (accuracy can't be lowered): the pop-up/message show Mind's Eye even though
// the chosen ability is a forced Damp (the Keen Eye / Limber / Oblivious pop-up precedent).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Mind's Eye keeps the holder's accuracy from being lowered")
{
    u32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_URSALUNA_BLOODMOON, ABILITY_MINDS_EYE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_URSALUNA_BLOODMOON) { Ability(ABILITY_DAMP); } // chosen Damp; innate Mind's Eye
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SAND_ATTACK); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SAND_ATTACK); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_MINDS_EYE);
            MESSAGE("Ursaluna's accuracy was not lowered!");
        }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ACC], enabled ? DEFAULT_STAT_STAGE : DEFAULT_STAT_STAGE - 1);
    }
}

// Suppression parity: Mind's Eye is breakable (unlike Full Metal Body), so an attacker's Mold Breaker
// pierces the innate accuracy-drop immunity exactly as it would the real ability.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker pierces an innate Mind's Eye's accuracy-drop immunity")
{
    GIVEN {
        ASSUME(gAbilitiesInfo[ABILITY_MINDS_EYE].breakable);
        ASSUME(SpeciesHasInnate(SPECIES_URSALUNA_BLOODMOON, ABILITY_MINDS_EYE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_URSALUNA_BLOODMOON) { Ability(ABILITY_DAMP); } // innate Mind's Eye
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MOLD_BREAKER); Moves(MOVE_SAND_ATTACK); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SAND_ATTACK); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(player, ABILITY_MINDS_EYE); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ACC], DEFAULT_STAT_STAGE - 1); // pierced -> accuracy drops
    }
}

// Batch Y5 — Purifying Salt / Good as Gold (status immunities).
// Purifying Salt makes the holder immune to EVERY non-volatile status and halves incoming Ghost damage;
// Good as Gold blocks incoming status moves. Both 1:1 clean-upside copies (Purifying Salt's only real cost —
// blocking the holder's OWN Rest — is dropped for the innate, the Insomnia/Vital Spirit pure-boon precedent).
// The salt line (Nacli/Naclstack/Garganacl) carries innate Purifying Salt; Gholdengo carries innate Good as Gold.

// Purifying Salt, status-immunity half: an innate holder (chosen ability differs) is immune to a non-volatile
// status, with the pop-up/message showing Purifying Salt (the catch-all Comatose/Purifying-Salt block).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Purifying Salt blocks a non-volatile status")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_TOXIC) == EFFECT_NON_VOLATILE_STATUS);
        ASSUME(GetMoveNonVolatileStatus(MOVE_TOXIC) == MOVE_EFFECT_TOXIC);
        ASSUME(SpeciesHasInnate(SPECIES_GARGANACL, ABILITY_PURIFYING_SALT));
        ASSUME(gSpeciesInfo[SPECIES_GARGANACL].abilities[0] != ABILITY_DAMP);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_GARGANACL) { Ability(ABILITY_DAMP); } // chosen Damp; innate Purifying Salt
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TOXIC); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_TOXIC); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_PURIFYING_SALT); // pop-up shows Purifying Salt, not the chosen Damp
            MESSAGE("It doesn't affect Garganacl…");
            NONE_OF { STATUS_ICON(player, badPoison: TRUE); }
        } else {
            STATUS_ICON(player, badPoison: TRUE); // no innate -> Toxic poisons
        }
    }
}

// Suppression parity: Purifying Salt is breakable, so an attacker's Mold Breaker pierces the innate exactly
// as it would the real ability — the status lands and no Purifying Salt pop-up shows.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker pierces an innate Purifying Salt")
{
    GIVEN {
        ASSUME(gAbilitiesInfo[ABILITY_PURIFYING_SALT].breakable);
        ASSUME(SpeciesHasInnate(SPECIES_GARGANACL, ABILITY_PURIFYING_SALT));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_GARGANACL) { Ability(ABILITY_DAMP); } // innate Purifying Salt
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MOLD_BREAKER); Moves(MOVE_TOXIC); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_TOXIC); }
    } SCENE {
        STATUS_ICON(player, badPoison: TRUE); // Mold Breaker ignores the innate -> poisoned
        NONE_OF { ABILITY_POPUP(player, ABILITY_PURIFYING_SALT); }
    }
}

// Purifying Salt, Ghost-resist half: it halves incoming Ghost damage (a silent calc modifier beside innate
// Thick Fat). The chosen ability is a forced Damp, so the reduction is solely the innate's.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Purifying Salt halves Ghost damage", s16 damage)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(GetMoveType(MOVE_SHADOW_BALL) == TYPE_GHOST);
        ASSUME(SpeciesHasInnate(SPECIES_GARGANACL, ABILITY_PURIFYING_SALT));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_GARGANACL) { Ability(ABILITY_DAMP); } // chosen Damp; innate Purifying Salt
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SHADOW_BALL); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SHADOW_BALL); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.5), results[1].damage); // off: full; on: 0.5x
    }
}

// PURE-BOON DIVERGENCE: a real Purifying Salt BLOCKS the holder's own Rest (a cost). The innate intentionally
// does NOT block Rest, so an innate-Purifying-Salt mon may still Rest to full HP (and sleeps from its own move).
// The chosen-ability Rest gate is left untouched, so this only diverges for the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Purifying Salt does NOT block the holder's own Rest (pure boon)")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_GARGANACL) { Ability(ABILITY_DAMP); MaxHP(200); HP(100); Moves(MOVE_REST); } // innate Purifying Salt
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_REST); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Garganacl slept and restored its HP!");
    } THEN {
        EXPECT_EQ(player->hp, player->maxHP);           // Rest healed to full
        EXPECT_NE(player->status1 & STATUS1_SLEEP, 0);  // and slept (not blocked by the innate)
    }
}

// Good as Gold: an innate holder (chosen ability differs) blocks an incoming status move, with the pop-up/
// message showing Good as Gold (CanAbilityAbsorbMove, pop-up overwritten to the innate).
// (Thunder Wave, not a poison move — Gholdengo is Steel and can't be poisoned regardless of the block.)
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Good as Gold blocks a status move")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(GetMoveCategory(MOVE_THUNDER_WAVE) == DAMAGE_CATEGORY_STATUS);
        ASSUME(SpeciesHasInnate(SPECIES_GHOLDENGO, ABILITY_GOOD_AS_GOLD));
        ASSUME(gSpeciesInfo[SPECIES_GHOLDENGO].abilities[0] != ABILITY_DAMP);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_GHOLDENGO) { Ability(ABILITY_DAMP); } // chosen Damp; innate Good as Gold
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_THUNDER_WAVE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_THUNDER_WAVE); }
    } SCENE {
        if (enabled) {
            NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_THUNDER_WAVE, opponent);
            ABILITY_POPUP(player, ABILITY_GOOD_AS_GOLD); // pop-up shows Good as Gold, not the chosen Damp
            MESSAGE("It doesn't affect Gholdengo…");
            NONE_OF { STATUS_ICON(player, paralysis: TRUE); }
        } else {
            STATUS_ICON(player, paralysis: TRUE); // no innate -> Thunder Wave paralyzes
        }
    }
}

// Suppression parity: Good as Gold is breakable, so an attacker's Mold Breaker pierces the innate exactly as
// it would the real ability — the status move lands and no Good as Gold pop-up shows.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker pierces an innate Good as Gold")
{
    GIVEN {
        ASSUME(gAbilitiesInfo[ABILITY_GOOD_AS_GOLD].breakable);
        ASSUME(SpeciesHasInnate(SPECIES_GHOLDENGO, ABILITY_GOOD_AS_GOLD));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_GHOLDENGO) { Ability(ABILITY_DAMP); } // innate Good as Gold
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MOLD_BREAKER); Moves(MOVE_THUNDER_WAVE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_THUNDER_WAVE); }
    } SCENE {
        STATUS_ICON(player, paralysis: TRUE); // Mold Breaker ignores the innate -> paralyzed
        NONE_OF { ABILITY_POPUP(player, ABILITY_GOOD_AS_GOLD); }
    }
}

// ─── Innate Intrepid Sword / Dauntless Shield (switch-in stat boosts, Batch Y6) ────────────────────────────
// Both reuse the same switch-in driver as Intimidate (a SwitchInInnateAbilityEffect -> ABILITYEFFECT_ON_SWITCHIN
// addition), so the +1 Attack / +1 Defense, the once-per-battle latch (party-state boost flag, active under
// B_* >= GEN_9), and the pop-up match the real ability. Each vehicle carries a chosen ability != the innate
// (Zacian / Zamazenta are sole-ability, so a forced neutral Damp isolates the innate), so both the effect and
// the pop-up are attributable solely to the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Intrepid Sword raises the holder's Attack on switch-in")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_ZACIAN, ABILITY_INTREPID_SWORD));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_ZACIAN) { Ability(ABILITY_DAMP); } // chosen Damp; the boost is the innate's
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_INTREPID_SWORD); // pop-up shows the innate, not chosen Damp
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_INTREPID_SWORD); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], enabled ? DEFAULT_STAT_STAGE + 1 : DEFAULT_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: an innate Intrepid Sword fires only once per battle (Gen9+)")
{
    GIVEN {
        ASSUME(B_INTREPID_SWORD >= GEN_9); // the once-per-battle latch only applies from Gen 9
        ASSUME(SpeciesHasInnate(SPECIES_ZACIAN, ABILITY_INTREPID_SWORD));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_ZACIAN) { Ability(ABILITY_DAMP); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); } // Zacian in → fires (first entry)
        TURN { SWITCH(player, 0); } // Wobbuffet in, Zacian out (its +1 Attack resets on switch-out)
        TURN { SWITCH(player, 1); } // Zacian back in → the latch must stop it re-firing
    } SCENE {
        ABILITY_POPUP(player, ABILITY_INTREPID_SWORD); // fires on the first entry
        NONE_OF { ABILITY_POPUP(player, ABILITY_INTREPID_SWORD); } // once-per-battle: never again
    } THEN {
        // The boost from the first entry was wiped by the switch-out; because the latch blocks a second
        // trigger, the re-entered Zacian is back at the default stage (not re-boosted to +1). Combined with
        // the pop-up firing exactly once above, this proves the once-per-battle latch holds for the innate.
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Neutralizing Gas suppresses an innate Intrepid Sword on switch-in")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_ZACIAN, ABILITY_INTREPID_SWORD));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_ZACIAN) { Ability(ABILITY_DAMP); }
        OPPONENT(SPECIES_WEEZING_GALAR) { Ability(ABILITY_NEUTRALIZING_GAS); }
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(player, ABILITY_INTREPID_SWORD); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE); // suppressed: no boost
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Dauntless Shield raises the holder's Defense on switch-in")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_ZAMAZENTA, ABILITY_DAUNTLESS_SHIELD));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_ZAMAZENTA) { Ability(ABILITY_DAMP); } // chosen Damp; the boost is the innate's
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_DAUNTLESS_SHIELD); // pop-up shows the innate, not chosen Damp
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_DAUNTLESS_SHIELD); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_DEF], enabled ? DEFAULT_STAT_STAGE + 1 : DEFAULT_STAT_STAGE);
    }
}

// ─── Innate Beast Boost (on-KO best-stat boost, Batch Y7) ───────────────────────────────────────────────────
// Beast Boost is Moxie's best-stat edition: on a KO the holder's HIGHEST stat rises +1. It rides the same
// attacker-side on-hit driver as Moxie / Chilling Neigh / Grim Neigh (the upstream
// ABILITYEFFECT_MOVE_END_FOES_FAINTED case already reads GetHighestStatId for it), so the stat pick / stat
// change / pop-up come for free. Kartana's Attack (181) is by far its highest stat, so the innate raises
// Attack; a forced non-slot chosen Pressure isolates the innate (chosen != the innate) and the feature-off
// leg proves the effect is innate-only.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Beast Boost raises the holder's highest stat on a KO")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_KARTANA, ABILITY_BEAST_BOOST));
        ASSUME(gSpeciesInfo[SPECIES_KARTANA].baseAttack > gSpeciesInfo[SPECIES_KARTANA].baseDefense); // Attack is the highest stat
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_KARTANA) { Ability(ABILITY_PRESSURE); Moves(MOVE_TACKLE); } // forced Pressure; Beast Boost only as innate
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_ZIGZAGOON); // a second foe, so KOing the first doesn't end the battle
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); SEND_OUT(opponent, 1); } // KOs the first foe; the second replaces it
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_BEAST_BOOST); // pop-up shows the innate, not the forced Pressure
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_BEAST_BOOST); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], enabled ? DEFAULT_STAT_STAGE + 1 : DEFAULT_STAT_STAGE);
    }
}

// Tier 5.1 — Mega Sol (fork-custom ability): the holder's own moves treat the weather as harsh sun.
// Wired at the single chokepoint GetAttackerWeather(battler, ...) via IsInnateActive, so every
// attacker-weather read (Growth's +2, Solar Beam's skipped charge, Weather Ball, sun-boosted Fire
// damage, ...) is innate-aware. Base Meganium carries it as an observable flavor pick (chosen Overgrow).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Mega Sol makes the holder's Growth act as if under sun")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MEGANIUM, ABILITY_MEGA_SOL));
        ASSUME(GetMoveEffect(MOVE_GROWTH) == EFFECT_GROWTH);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        WITH_CONFIG(B_GROWTH_STAT_RAISE, GEN_5); // +2 under sun
        PLAYER(SPECIES_MEGANIUM) { Ability(ABILITY_OVERGROW); } // forced Overgrow; Mega Sol only as innate
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_GROWTH); }
    } THEN {
        // With Mega Sol active the holder sees sun, so Growth raises +2; otherwise the normal +1.
        EXPECT_EQ(player->statStages[STAT_ATK],   enabled ? DEFAULT_STAT_STAGE + 2 : DEFAULT_STAT_STAGE + 1);
        EXPECT_EQ(player->statStages[STAT_SPATK], enabled ? DEFAULT_STAT_STAGE + 2 : DEFAULT_STAT_STAGE + 1);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Mega Sol lets the holder's Solar Beam skip its charge turn")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MEGANIUM, ABILITY_MEGA_SOL));
        ASSUME(GetMoveTwoTurnAttackWeather(MOVE_SOLAR_BEAM) == B_WEATHER_SUN);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_MEGANIUM) { Ability(ABILITY_OVERGROW); } // forced Overgrow; Mega Sol only as innate
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SOLAR_BEAM); }
        if (!enabled)
            TURN { SKIP_TURN(player); } // without Mega Sol, Solar Beam charges turn 1 and fires turn 2
    } SCENE {
        if (!enabled) {
            MESSAGE("Meganium used Solar Beam!");
            MESSAGE("Meganium absorbed light!"); // the charge turn
        }
        MESSAGE("Meganium used Solar Beam!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SOLAR_BEAM, player); // the actual beam
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Neutralizing Gas suppresses an innate Mega Sol")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MEGANIUM, ABILITY_MEGA_SOL));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(B_GROWTH_STAT_RAISE, GEN_5);
        PLAYER(SPECIES_MEGANIUM) { Ability(ABILITY_OVERGROW); }
        OPPONENT(SPECIES_WEEZING_GALAR) { Ability(ABILITY_NEUTRALIZING_GAS); }
    } WHEN {
        TURN { MOVE(player, MOVE_GROWTH); }
    } THEN {
        // Mega Sol suppressed -> the holder no longer sees sun, so Growth is the normal +1, not +2.
        EXPECT_EQ(player->statStages[STAT_ATK],   DEFAULT_STAT_STAGE + 1);
        EXPECT_EQ(player->statStages[STAT_SPATK], DEFAULT_STAT_STAGE + 1);
    }
}

// Tier 5.2 — Quick Draw: the holder's moves have a 30% chance of going first within their priority
// bracket; under DETERMINISTIC_ABILITIES (the shipping default) it instead always fires on the holder's
// entry turn, like Quick Claw. Wired at the two effect sites in TryChangingTurnOrderEffects
// (src/battle_main.c) via BattlerHasAbility, with the activation pop-up / message overwritten to Quick
// Draw when the chosen ability differs. Galarian Slowbro carries it canonically; the Galarian Farfetch'd
// -> Sirfetch'd duelist line takes it as an observable flavor pick (chosen Steadfast / Scrappy differ).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Quick Draw lets the slower holder move first on its entry turn")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SLOWBRO_GALAR, ABILITY_QUICK_DRAW));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE); // always fires on the entry turn
        PLAYER(SPECIES_SLOWBRO_GALAR) { Ability(ABILITY_OWN_TEMPO); Speed(1); } // chosen Own Tempo; Quick Draw only as innate
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CELEBRATE); MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        if (enabled) {
            // The pop-up shows Quick Draw (the innate), not the chosen Own Tempo, and the slower holder moves first.
            ABILITY_POPUP(player, ABILITY_QUICK_DRAW);
            MESSAGE("Slowbro used Scratch!");
            MESSAGE("The opposing Wobbuffet used Celebrate!");
        } else {
            // Feature off: no innate Quick Draw, so the slow holder moves last (stock behavior).
            NOT ABILITY_POPUP(player, ABILITY_QUICK_DRAW);
            MESSAGE("The opposing Wobbuffet used Celebrate!");
            MESSAGE("Slowbro used Scratch!");
        }
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: a flavor duelist's innate Quick Draw fires on its 30% roll (non-deterministic)")
{
    PASSES_RANDOMLY(3, 10, RNG_QUICK_DRAW);
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SIRFETCHD, ABILITY_QUICK_DRAW));
        ASSUME(gSpeciesInfo[SPECIES_SIRFETCHD].abilities[0] != ABILITY_QUICK_DRAW);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        // DETERMINISTIC_ABILITIES is off in the test baseline, so Quick Draw uses its stock 30% roll.
        PLAYER(SPECIES_SIRFETCHD) { Ability(ABILITY_STEADFAST); Speed(1); } // chosen Steadfast; Quick Draw only as innate
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); }
    } SCENE {
        // On the 30% branch the pop-up shows Quick Draw even though the chosen ability is Steadfast.
        ABILITY_POPUP(player, ABILITY_QUICK_DRAW);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Neutralizing Gas suppresses an innate Quick Draw")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SLOWBRO_GALAR, ABILITY_QUICK_DRAW));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        PLAYER(SPECIES_SLOWBRO_GALAR) { Ability(ABILITY_OWN_TEMPO); Speed(1); }
        OPPONENT(SPECIES_WEEZING_GALAR) { Ability(ABILITY_NEUTRALIZING_GAS); Speed(100); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CELEBRATE); MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        // Quick Draw suppressed -> no pop-up and the slow holder moves last.
        NOT ABILITY_POPUP(player, ABILITY_QUICK_DRAW);
        MESSAGE("The opposing Weezing used Celebrate!");
        MESSAGE("Slowbro used Scratch!");
    }
}

AI_SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: AI knows an innate Quick Draw lets it move first on its entry turn")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_SLOWBRO_GALAR, ABILITY_QUICK_DRAW));
        ASSUME(GetMoveEffect(MOVE_DESTINY_BOND) == EFFECT_DESTINY_BOND);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        // Chosen Own Tempo, Quick Draw only as an innate: the AI is far slower by raw speed, but its innate
        // Quick Draw guarantees it moves first on its entry turn. Since the foe can then KO it, the AI should
        // value Destiny Bond (scored up only when it predicts going first), proving the deterministic
        // turn-order override in AI_WhoStrikesFirst is innate-aware.
        PLAYER(SPECIES_WOBBUFFET) { Speed(200); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_SLOWBRO_GALAR) { Ability(ABILITY_OWN_TEMPO); Speed(1); HP(1); MaxHP(200); Moves(MOVE_DESTINY_BOND, MOVE_SCRATCH); }
    } WHEN {
        TURN { SCORE_GT(opponent, MOVE_DESTINY_BOND, MOVE_SCRATCH); }
    }
}

// Tier 5.3 — Comatose (status immunity + sleep-move synergy).
// A PURE-BOON divergence: the holder is immune to EVERY non-volatile status and counts as asleep for its OWN
// Snore / Sleep Talk, but — unlike the real ability — is NOT treated as asleep at the COST sites (enemy Hex /
// Dream Eater / Nightmare / Bad Dreams, and its own Rest block), so its always-asleep trait only ever helps it.
// Komala carries innate Comatose (beside its innate Unaware) with a chosen Sticky Hold override; the tests set
// a differing chosen ability so the innate is observable. cantBeSuppressed, so Mold Breaker can't pierce it.

// Status-immunity half: an innate holder (chosen ability differs) is immune to every non-volatile status
// (sleep, burn, paralysis, poison), with the pop-up/message showing Comatose (the catch-all status block).
// Unlike the chosen ability there is no switch-in "drowsing" message (the display half is deliberately dropped).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Comatose blocks every non-volatile status")
{
    enum Move move;
    PARAMETRIZE { move = MOVE_SPORE; }
    PARAMETRIZE { move = MOVE_WILL_O_WISP; }
    PARAMETRIZE { move = MOVE_THUNDER_WAVE; }
    PARAMETRIZE { move = MOVE_TOXIC; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_KOMALA, ABILITY_COMATOSE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_KOMALA) { Ability(ABILITY_STICKY_HOLD); } // chosen Sticky Hold; innate Comatose
        OPPONENT(SPECIES_WOBBUFFET) { Moves(move); }
    } WHEN {
        TURN { MOVE(opponent, move); }
    } SCENE {
        // No switch-in "Komala is drowsing!" message fires: the innate deliberately drops the display half
        // (no switch-in driver), and the chosen Sticky Hold isn't Comatose, so the flavor pop-up never shows.
        NOT ANIMATION(ANIM_TYPE_MOVE, move, opponent);
        ABILITY_POPUP(player, ABILITY_COMATOSE); // pop-up shows Comatose, not the chosen Sticky Hold
        MESSAGE("It doesn't affect Komala…");
        NONE_OF {
            STATUS_ICON(player, sleep: TRUE);
            STATUS_ICON(player, burn: TRUE);
            STATUS_ICON(player, paralysis: TRUE);
            STATUS_ICON(player, badPoison: TRUE);
        }
    }
}

// Feature gate: with FEATURE_INNATE_ABILITIES off, Komala's innate Comatose is inert, so a chosen Sticky Hold
// holder is put to sleep normally (stock behavior).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: with the feature off, no innate Comatose (stock behavior)")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, FALSE);
        PLAYER(SPECIES_KOMALA) { Ability(ABILITY_STICKY_HOLD); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SPORE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SPORE); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(player, ABILITY_COMATOSE); }
        STATUS_ICON(player, sleep: TRUE); // no innate -> Spore puts it to sleep
    }
}

// Suppression parity: Comatose is cantBeSuppressed (NOT breakable), so — unlike the breakable Purifying Salt —
// an attacker's Mold Breaker does NOT pierce the innate: the status is still blocked and the Comatose pop-up shows.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker does NOT pierce an innate Comatose (cantBeSuppressed)")
{
    GIVEN {
        ASSUME(gAbilitiesInfo[ABILITY_COMATOSE].cantBeSuppressed);
        ASSUME(!gAbilitiesInfo[ABILITY_COMATOSE].breakable);
        ASSUME(SpeciesHasInnate(SPECIES_KOMALA, ABILITY_COMATOSE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_KOMALA) { Ability(ABILITY_STICKY_HOLD); } // innate Comatose
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MOLD_BREAKER); Moves(MOVE_SPORE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SPORE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_COMATOSE); // Mold Breaker can't pierce -> still blocked
        NONE_OF { STATUS_ICON(player, sleep: TRUE); }
    }
}

// PURE-BOON DIVERGENCE: the real Comatose is "always asleep", which blocks the holder's own Rest. The innate
// intentionally does NOT (the EFFECT_REST gate is left chosen-only, the Insomnia / Purifying Salt precedent),
// so an innate-Comatose mon may still Rest to full HP and sleep from its own move.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Comatose does NOT block the holder's own Rest (pure boon)")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_KOMALA) { Ability(ABILITY_STICKY_HOLD); MaxHP(200); HP(100); Moves(MOVE_REST); } // innate Comatose
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_REST); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Komala slept and restored its HP!");
    } THEN {
        EXPECT_EQ(player->hp, player->maxHP);           // Rest healed to full
        EXPECT_NE(player->status1 & STATUS1_SLEEP, 0);  // and slept (not blocked by the innate)
    }
}

// Sleep-move synergy (boon half): an innate Comatose counts as asleep for its OWN Snore, so the holder may use
// it while awake (like the real ability) — feature off, the awake holder's Snore fails.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Comatose lets the holder use Snore while awake")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_SNORE) == EFFECT_SNORE);
        ASSUME(SpeciesHasInnate(SPECIES_KOMALA, ABILITY_COMATOSE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_KOMALA) { Ability(ABILITY_STICKY_HOLD); Moves(MOVE_SNORE); } // innate Comatose, but not actually asleep
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SNORE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        if (enabled) {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_SNORE, player); // innate Comatose -> Snore lands
            NOT MESSAGE("But it failed!");
        } else {
            MESSAGE("But it failed!"); // no innate, awake -> Snore fails
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Tier 5.4 — Magic Guard (a 1:1 clean-upside copy): the holder takes damage ONLY from
// direct attacks, so it is spared every indirect / chip-damage source (recoil, status
// damage, weather chip, Leech Seed, entry hazards, Life Orb, contact recoil, ...). Wired
// as a cross-cutting sweep, mostly via the IsAbilityOrInnateAndRecord drop-in for
// IsAbilityAndRecord. Canon-only carriers (Clefairy / Abra / Solosis lines, Sigilyph);
// tests use Clefable with a chosen ability that is NOT Magic Guard, so the innate is what
// provides the protection.
// ─────────────────────────────────────────────────────────────────────────────

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Magic Guard prevents recoil damage to the holder")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(GetMoveRecoil(MOVE_DOUBLE_EDGE) == 33);
        ASSUME(SpeciesHasInnate(SPECIES_CLEFABLE, ABILITY_MAGIC_GUARD));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_CLEFABLE) { Ability(ABILITY_CUTE_CHARM); Moves(MOVE_DOUBLE_EDGE); } // chosen Cute Charm; innate Magic Guard
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_DOUBLE_EDGE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DOUBLE_EDGE, player);
        HP_BAR(opponent);
        if (enabled)
            NOT HP_BAR(player); // innate Magic Guard -> no recoil
        else
            HP_BAR(player);     // no innate -> Double-Edge recoils normally
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Magic Guard prevents poison/burn end-of-turn chip")
{
    u32 status;
    PARAMETRIZE { status = STATUS1_POISON; }
    PARAMETRIZE { status = STATUS1_TOXIC_POISON; }
    PARAMETRIZE { status = STATUS1_BURN; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CLEFABLE, ABILITY_MAGIC_GUARD));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_CLEFABLE) { Ability(ABILITY_CUTE_CHARM); Status1(status); MaxHP(300); HP(300); } // innate Magic Guard
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN {}
        TURN {}
    } THEN {
        EXPECT_EQ(player->hp, player->maxHP); // no status chip taken
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Magic Guard prevents sandstorm chip")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CLEFABLE, ABILITY_MAGIC_GUARD));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_CLEFABLE) { Ability(ABILITY_CUTE_CHARM); MaxHP(300); HP(300); } // Fairy-type, not sandstorm-immune by type
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SANDSTORM); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SANDSTORM); }
        TURN {}
    } THEN {
        EXPECT_EQ(player->hp, player->maxHP); // sandstorm deals no chip to the innate holder
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Magic Guard prevents Leech Seed drain")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_LEECH_SEED) == EFFECT_LEECH_SEED);
        ASSUME(SpeciesHasInnate(SPECIES_CLEFABLE, ABILITY_MAGIC_GUARD));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_CLEFABLE) { Ability(ABILITY_CUTE_CHARM); MaxHP(300); HP(300); } // innate Magic Guard
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_LEECH_SEED); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_LEECH_SEED); }
        TURN {}
    } THEN {
        EXPECT_EQ(player->hp, player->maxHP); // seeded, but no HP drained
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Magic Guard prevents Life Orb recoil")
{
    GIVEN {
        ASSUME(gItemsInfo[ITEM_LIFE_ORB].holdEffect == HOLD_EFFECT_LIFE_ORB);
        ASSUME(SpeciesHasInnate(SPECIES_CLEFABLE, ABILITY_MAGIC_GUARD));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_CLEFABLE) { Ability(ABILITY_CUTE_CHARM); Item(ITEM_LIFE_ORB); Moves(MOVE_MOONBLAST); } // innate Magic Guard
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(500); HP(500); }
    } WHEN {
        TURN { MOVE(player, MOVE_MOONBLAST); }
    } SCENE {
        HP_BAR(opponent);
        NOT HP_BAR(player); // Life Orb boosts the hit but deals no recoil to the innate holder
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Magic Guard prevents Stealth Rock switch-in damage")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CLEFABLE, ABILITY_MAGIC_GUARD));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_CLEFABLE) { Ability(ABILITY_CUTE_CHARM); } // innate Magic Guard
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_STEALTH_ROCK); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_STEALTH_ROCK); }
        TURN { SWITCH(player, 1); }
    } SCENE {
        NOT HP_BAR(player); // Clefable switches into Stealth Rock but takes no damage
    } THEN {
        EXPECT_EQ(player->hp, player->maxHP);
    }
}

// Suppression parity: Magic Guard is breakable, and an innate honors general suppression via IsInnateActive.
// Neutralizing Gas on the field turns the innate off, so the holder takes poison chip again.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Neutralizing Gas suppresses an innate Magic Guard")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CLEFABLE, ABILITY_MAGIC_GUARD));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_CLEFABLE) { Ability(ABILITY_CUTE_CHARM); Status1(STATUS1_POISON); MaxHP(300); HP(300); } // innate Magic Guard
        OPPONENT(SPECIES_WEEZING) { Ability(ABILITY_NEUTRALIZING_GAS); }
    } WHEN {
        TURN {}
    } THEN {
        EXPECT_LT(player->hp, player->maxHP); // innate suppressed -> poison chip lands
    }
}

// ===== Batch X — script jumpifability innate-awareness =====================================
// The per-battler jumpifability form used to read only the chosen ability slot, so an innate holder
// was invisible to the two ability blocks that live in a battle script rather than in C: Sticky Hold
// vs Corrosive Gas / a Pickpocket steal, and Own Tempo's confuse-move pop-up. Cmd_jumpifability now
// credits an innate for these two (allowlisted) abilities. Comatose's COST sites (Nightmare / Bad
// Dreams / own Rest) route through the same command and are deliberately left chosen-slot-only — the
// Comatose tests above (own Rest still works, no phantom chip) guard that pure-boon divergence.

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Sticky Hold keeps the held item from Corrosive Gas")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_CORROSIVE_GAS) == EFFECT_CORROSIVE_GAS);
        ASSUME(SpeciesHasInnate(SPECIES_MUK, ABILITY_STICKY_HOLD));
        ASSUME(gSpeciesInfo[SPECIES_MUK].abilities[0] != ABILITY_STICKY_HOLD);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_MUK) { Ability(ABILITY_POISON_TOUCH); Item(ITEM_LEFTOVERS); } // chosen differs from the innate Sticky Hold
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CORROSIVE_GAS); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CORROSIVE_GAS); }
    } SCENE {
        if (enabled)
            ABILITY_POPUP(player, ABILITY_STICKY_HOLD); // pop-up shows the innate, not the chosen Poison Touch
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_STICKY_HOLD); }
    } THEN {
        if (enabled)
            EXPECT_EQ(player->item, ITEM_LEFTOVERS); // item survives the corrosive gas
        else
            EXPECT_EQ(player->item, ITEM_NONE); // item melted
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Sticky Hold blocks a Pickpocket steal")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MUK, ABILITY_STICKY_HOLD));
        ASSUME(gSpeciesInfo[SPECIES_MUK].abilities[0] != ABILITY_STICKY_HOLD);
        ASSUME(MoveMakesContact(MOVE_TACKLE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_MUK) { Ability(ABILITY_POISON_TOUCH); Item(ITEM_LEFTOVERS); Moves(MOVE_TACKLE); } // chosen differs from the innate Sticky Hold
        OPPONENT(SPECIES_SNEASEL) { Ability(ABILITY_PICKPOCKET); } // no item -> tries to pickpocket the attacker
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(opponent, ABILITY_PICKPOCKET);
            ABILITY_POPUP(player, ABILITY_STICKY_HOLD); // pop-up shows the innate, not the chosen Poison Touch
        }
    } THEN {
        if (enabled) {
            EXPECT_EQ(player->item, ITEM_LEFTOVERS); // Sticky Hold keeps the item
            EXPECT_EQ(opponent->item, ITEM_NONE);
        } else {
            EXPECT_EQ(player->item, ITEM_NONE); // pickpocketed away
            EXPECT_EQ(opponent->item, ITEM_LEFTOVERS);
        }
    }
}

// Batch X also upgrades the innate Own Tempo confuse block from a silent immunity (handled by
// CanBeConfused) to one that shows the Own Tempo pop-up, because BattleScript_EffectConfuse's own
// jumpifability now sees the innate. The immunity itself is already covered above; this pins the pop-up.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Own Tempo shows its pop-up when blocking a confuse move")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_CONFUSE_RAY) == EFFECT_CONFUSE);
        ASSUME(SpeciesHasInnate(SPECIES_SLOWBRO, ABILITY_OWN_TEMPO));
        ASSUME(gSpeciesInfo[SPECIES_SLOWBRO].abilities[0] != ABILITY_REGENERATOR);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_SLOWBRO) { Ability(ABILITY_REGENERATOR); } // chosen differs from the innate Own Tempo
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CONFUSE_RAY); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CONFUSE_RAY); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_OWN_TEMPO); // pop-up shows the innate, not the chosen Regenerator
    } THEN {
        EXPECT(player->volatiles.confusionTurns == 0); // still immune
    }
}

// ===== Tier 5.5 — Mold Breaker =============================================================
// An innate Mold Breaker makes the HOLDER's moves ignore the target's breakable ability. The whole
// effect flows through the single gBattleStruct->moldBreakerActive flag set from
// IsMoldBreakerTypeAbility, which now credits an innate via IsInnateActive — so every effect site and
// AI read is covered. Isolate the INNATE (not the chosen slot): Excadrill's chosen ability is set to
// Sand Rush, so any ability-piercing can only come from its innate Mold Breaker. Levitate is the clean
// binary observable: normally the target is immune to Ground; through Mold Breaker the hit connects.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Mold Breaker pierces the target's Levitate")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(GetMoveType(MOVE_MUD_SLAP) == TYPE_GROUND);
        ASSUME(SpeciesHasInnate(SPECIES_EXCADRILL, ABILITY_MOLD_BREAKER));
        ASSUME(gSpeciesInfo[SPECIES_EXCADRILL].abilities[0] != ABILITY_MOLD_BREAKER);
        ASSUME(gSpeciesInfo[SPECIES_GASTLY].abilities[0] == ABILITY_LEVITATE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_EXCADRILL) { Ability(ABILITY_SAND_RUSH); Moves(MOVE_MUD_SLAP); } // chosen differs from the innate Mold Breaker
        OPPONENT(SPECIES_GASTLY); // real Levitate blocks Ground
    } WHEN {
        TURN { MOVE(player, MOVE_MUD_SLAP); }
    } SCENE {
        if (enabled)
            HP_BAR(opponent); // innate Mold Breaker ignores Levitate -> the Ground hit connects
        else
            MESSAGE("It doesn't affect the opposing Gastly…"); // no innate -> Levitate holds
    }
}

// Suppression parity: Mold Breaker is itself never self-broken, but an innate honors general
// suppression via IsInnateActive. Gastro Acid on the holder turns the innate off, so the target's
// Levitate blocks the Ground move again (IsMoldBreakerTypeAbility bails on the holder's Gastro Acid).
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Mold Breaker")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_EXCADRILL, ABILITY_MOLD_BREAKER));
        ASSUME(gSpeciesInfo[SPECIES_GASTLY].abilities[0] == ABILITY_LEVITATE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_EXCADRILL) { Ability(ABILITY_SAND_RUSH); Moves(MOVE_MUD_SLAP); } // innate Mold Breaker
        OPPONENT(SPECIES_GASTLY) { Moves(MOVE_GASTRO_ACID); } // real Levitate + Gastro Acid
    } WHEN {
        TURN { MOVE(opponent, MOVE_GASTRO_ACID); } // suppress the holder's abilities (incl. the innate Mold Breaker)
        TURN { MOVE(player, MOVE_MUD_SLAP); }
    } SCENE {
        MESSAGE("It doesn't affect the opposing Gastly…"); // innate suppressed -> Levitate blocks the Ground hit again
    }
}

// ===== Batch Y8 — Turboblaze / Teravolt (Mold Breaker clones) ==============================
// Identical to Mold Breaker: the HOLDER's moves ignore the target's breakable ability. They share
// Mold Breaker's exact machinery — IsMoldBreakerTypeAbility now credits an innate Turboblaze / Teravolt
// too — so one clause each covers every effect site and AI read. Isolate the INNATE (not the chosen
// slot) by forcing a neutral chosen ability, so any ability-piercing can only come from the innate.
// Levitate is the clean binary observable, exactly as for Mold Breaker.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Turboblaze pierces the target's Levitate")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(GetMoveType(MOVE_MUD_SLAP) == TYPE_GROUND);
        ASSUME(SpeciesHasInnate(SPECIES_RESHIRAM, ABILITY_TURBOBLAZE));
        ASSUME(gSpeciesInfo[SPECIES_GASTLY].abilities[0] == ABILITY_LEVITATE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_RESHIRAM) { Ability(ABILITY_DAMP); Moves(MOVE_MUD_SLAP); } // chosen Damp; innate Turboblaze
        OPPONENT(SPECIES_GASTLY); // real Levitate blocks Ground
    } WHEN {
        TURN { MOVE(player, MOVE_MUD_SLAP); }
    } SCENE {
        if (enabled)
            HP_BAR(opponent); // innate Turboblaze ignores Levitate -> the Ground hit connects
        else
            MESSAGE("It doesn't affect the opposing Gastly…"); // no innate -> Levitate holds
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Teravolt pierces the target's Levitate")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(GetMoveType(MOVE_MUD_SLAP) == TYPE_GROUND);
        ASSUME(SpeciesHasInnate(SPECIES_ZEKROM, ABILITY_TERAVOLT));
        ASSUME(gSpeciesInfo[SPECIES_GASTLY].abilities[0] == ABILITY_LEVITATE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_ZEKROM) { Ability(ABILITY_DAMP); Moves(MOVE_MUD_SLAP); } // chosen Damp; innate Teravolt
        OPPONENT(SPECIES_GASTLY); // real Levitate blocks Ground
    } WHEN {
        TURN { MOVE(player, MOVE_MUD_SLAP); }
    } SCENE {
        if (enabled)
            HP_BAR(opponent); // innate Teravolt ignores Levitate -> the Ground hit connects
        else
            MESSAGE("It doesn't affect the opposing Gastly…"); // no innate -> Levitate holds
    }
}

// The Kyurem fusions share the machinery: White = Turboblaze, Black = Teravolt. Same binary observable,
// forcing a neutral chosen ability so only the innate pierces.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Turboblaze / Teravolt on the Kyurem fusions pierces Levitate")
{
    u32 species;
    PARAMETRIZE { species = SPECIES_KYUREM_WHITE; } // Turboblaze
    PARAMETRIZE { species = SPECIES_KYUREM_BLACK; } // Teravolt
    GIVEN {
        ASSUME(GetMoveType(MOVE_MUD_SLAP) == TYPE_GROUND);
        ASSUME(SpeciesHasInnate(SPECIES_KYUREM_WHITE, ABILITY_TURBOBLAZE));
        ASSUME(SpeciesHasInnate(SPECIES_KYUREM_BLACK, ABILITY_TERAVOLT));
        ASSUME(gSpeciesInfo[SPECIES_GASTLY].abilities[0] == ABILITY_LEVITATE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(species) { Ability(ABILITY_DAMP); Moves(MOVE_MUD_SLAP); } // chosen Damp; innate Turboblaze/Teravolt
        OPPONENT(SPECIES_GASTLY); // real Levitate blocks Ground
    } WHEN {
        TURN { MOVE(player, MOVE_MUD_SLAP); }
    } SCENE {
        HP_BAR(opponent); // innate ignores Levitate -> the Ground hit connects
    }
}

// Suppression parity: neither clone is self-broken, but an innate honors general suppression via
// IsInnateActive. Gastro Acid on the holder turns the innate off, so the target's Levitate blocks again.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Turboblaze")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_RESHIRAM, ABILITY_TURBOBLAZE));
        ASSUME(gSpeciesInfo[SPECIES_GASTLY].abilities[0] == ABILITY_LEVITATE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_RESHIRAM) { Ability(ABILITY_DAMP); Moves(MOVE_MUD_SLAP); } // innate Turboblaze
        OPPONENT(SPECIES_GASTLY) { Moves(MOVE_GASTRO_ACID); } // real Levitate + Gastro Acid
    } WHEN {
        TURN { MOVE(opponent, MOVE_GASTRO_ACID); } // suppress the holder's abilities (incl. the innate Turboblaze)
        TURN { MOVE(player, MOVE_MUD_SLAP); }
    } SCENE {
        MESSAGE("It doesn't affect the opposing Gastly…"); // innate suppressed -> Levitate blocks the Ground hit again
    }
}

// ===== Tier 5.6 — Opportunist =============================================================
// Whenever an OPPOSING battler's stat is boosted, an innate Opportunist copies that exact boost onto its
// holder (the ability twin of the Mirror Herb item). Isolate the INNATE (not the chosen slot): Espathra's
// chosen ability is forced to Synchronize, so any stat mirroring can only come from its innate Opportunist.
// The foe's Swords Dance (+2 Attack) is the clean binary observable: the holder copies it and the pop-up
// shows Opportunist, not the chosen Synchronize.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Opportunist copies a foe's stat boost")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_ESPATHRA, ABILITY_OPPORTUNIST));
        ASSUME(gSpeciesInfo[SPECIES_ESPATHRA].abilities[0] == ABILITY_OPPORTUNIST);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_SWORDS_DANCE); }
        OPPONENT(SPECIES_ESPATHRA) { Ability(ABILITY_SYNCHRONIZE); Moves(MOVE_CELEBRATE); } // chosen differs from the innate Opportunist
    } WHEN {
        TURN { MOVE(player, MOVE_SWORDS_DANCE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SWORDS_DANCE, player);
        if (enabled)
            ABILITY_POPUP(opponent, ABILITY_OPPORTUNIST); // innate fires; pop-up overwrite shows Opportunist, not chosen Synchronize
        else
            NOT ABILITY_POPUP(opponent, ABILITY_OPPORTUNIST);
    } THEN {
        if (enabled)
            EXPECT_EQ(opponent->statStages[STAT_ATK], DEFAULT_STAT_STAGE + 2); // copied the foe's Swords Dance
        else
            EXPECT_EQ(opponent->statStages[STAT_ATK], DEFAULT_STAT_STAGE); // no innate -> nothing to copy
    }
}

// Suppression parity: an innate Opportunist honors general suppression via IsInnateActive. Gastro Acid on
// the holder turns the innate off, so a foe's later boost is no longer mirrored.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Opportunist")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_ESPATHRA, ABILITY_OPPORTUNIST));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID, MOVE_SWORDS_DANCE); }
        OPPONENT(SPECIES_ESPATHRA) { Ability(ABILITY_SYNCHRONIZE); Moves(MOVE_CELEBRATE); } // innate Opportunist
    } WHEN {
        TURN { MOVE(player, MOVE_GASTRO_ACID); } // suppress the holder's abilities (incl. the innate Opportunist)
        TURN { MOVE(player, MOVE_SWORDS_DANCE); }
    } SCENE {
        NOT ABILITY_POPUP(opponent, ABILITY_OPPORTUNIST); // innate suppressed -> the foe's boost is not copied
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_ATK], DEFAULT_STAT_STAGE); // did not copy the foe's Swords Dance
    }
}

// Tier 5.7 — an innate Mirror Armor bounces a stat-lowering effect back at its source, exactly like the real
// ability (a pure boon: it only ever spares the holder its own drop and redirects it). Corviknight's chosen
// slot here is Pressure (also innate), so the innate Mirror Armor is what fires and the pop-up overwrite shows
// Mirror Armor rather than the chosen ability.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Mirror Armor reflects a stat drop back at the attacker")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CORVIKNIGHT, ABILITY_MIRROR_ARMOR));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_CORVIKNIGHT) { Ability(ABILITY_PRESSURE); } // chosen differs from the innate Mirror Armor
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(opponent, MOVE_GROWL); }
    } SCENE {
        if (enabled)
        {
            ABILITY_POPUP(player, ABILITY_MIRROR_ARMOR); // innate fires; pop-up overwrite shows Mirror Armor, not chosen Pressure
            ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, opponent);
        }
        else
        {
            NOT ABILITY_POPUP(player, ABILITY_MIRROR_ARMOR);
        }
    } THEN {
        if (enabled)
        {
            EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE);       // holder's Attack unaffected
            EXPECT_EQ(opponent->statStages[STAT_ATK], DEFAULT_STAT_STAGE - 1); // reflected onto the attacker
        }
        else
        {
            EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE - 1);   // no innate -> the drop lands on the holder
            EXPECT_EQ(opponent->statStages[STAT_ATK], DEFAULT_STAT_STAGE);
        }
    }
}

// An innate Mirror Armor reflects a switch-in Intimidate back at the Intimidator (the marquee interaction),
// mirroring the vanilla chosen-ability test.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Mirror Armor reflects Intimidate back at its source")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CORVIKNIGHT, ABILITY_MIRROR_ARMOR));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_CORVIKNIGHT) { Ability(ABILITY_PRESSURE); } // chosen differs from the innate Mirror Armor
        OPPONENT(SPECIES_GYARADOS) { Ability(ABILITY_INTIMIDATE); }
    } WHEN {
        TURN {}
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_INTIMIDATE);
        ABILITY_POPUP(player, ABILITY_MIRROR_ARMOR);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, opponent);
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE);       // holder's Attack unaffected
        EXPECT_EQ(opponent->statStages[STAT_ATK], DEFAULT_STAT_STAGE - 1); // Intimidate reflected onto Gyarados
    }
}

// Suppression parity: an innate Mirror Armor honors general suppression via IsInnateActive. Gastro Acid on the
// holder turns the innate off, so a later stat-lowering move lands on the holder instead of being reflected.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Mirror Armor")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CORVIKNIGHT, ABILITY_MIRROR_ARMOR));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_GASTRO_ACID, MOVE_GROWL); }
        OPPONENT(SPECIES_CORVIKNIGHT) { Ability(ABILITY_PRESSURE); } // innate Mirror Armor
    } WHEN {
        TURN { MOVE(player, MOVE_GASTRO_ACID); } // suppress the holder's abilities (incl. the innate Mirror Armor)
        TURN { MOVE(player, MOVE_GROWL); }
    } SCENE {
        NOT ABILITY_POPUP(opponent, ABILITY_MIRROR_ARMOR); // innate suppressed -> the drop is not reflected
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_ATK], DEFAULT_STAT_STAGE - 1); // the drop lands on the holder
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE);       // attacker not affected
    }
}
