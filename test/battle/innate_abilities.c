#include "global.h"
#include "test/battle.h"
#include "innate_abilities.h"

// FORK: coverage for FEATURE_INNATE_ABILITIES (config/feature.h). Feature flags
// default off in the test baseline (see TestInitConfigData), so each test that
// wants innates opts in with WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE). The
// supported innate set is LEVITATE (a passive Ground immunity) and REGENERATOR
// (a silent 1/3-HP switch-out heal); see src/innate_abilities.c.

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
