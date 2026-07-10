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

// Known limitation (documented in the ALLOWLIST note): an innate Pastel Veil still cures its OWN
// pre-existing poison on switch-in via TryImmunityAbilityHealStatus, but does NOT cure an ally's
// pre-existing poison the way the real ability's switch-in script does.
DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Pastel Veil cures its own pre-existing poison but not its partner's")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_RAPIDASH_GALAR) { Ability(ABILITY_RUN_AWAY); Status1(STATUS1_POISON); } // innate Pastel Veil
        PLAYER(SPECIES_WYNAUT) { Status1(STATUS1_POISON); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN {}
    } SCENE {
        ABILITY_POPUP(playerLeft, ABILITY_PASTEL_VEIL);
        MESSAGE("Rapidash was cured of its poisoning!");
    } THEN {
        EXPECT_EQ(playerLeft->status1 & STATUS1_POISON, 0);
        EXPECT_NE(playerRight->status1 & STATUS1_POISON, 0); // ally cure is NOT replicated for an innate
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
