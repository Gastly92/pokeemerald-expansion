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
// abilities, FILTER (−25% supereffective damage taken), PRESSURE (the holder's foes
// spend 1 extra PP per move used against it), and SPEED_BOOST (+1 Speed at the end of
// every turn — the first active, scripted end-turn innate); see src/innate_abilities.c.

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
