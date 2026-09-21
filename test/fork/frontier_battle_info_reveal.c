#include "global.h"
#include "test/battle.h"
#include "battle_ai_util.h"
#include "battle_ai_record.h"
#include "fork/innate_abilities.h"
#include "fork/frontier_battle_info.h" // INFO_MAX_DISPLAYED_ALT_FORMS (the Base Stats page's row budget)
#include "constants/form_change_types.h"

// FORK: B_FRONTIER_BATTLE_INFO. The in-battle INFO viewer must only treat a foe's
// ability/item as "revealed" once the player has actually witnessed it. The AI's
// *speculative* move evaluation calls RecordAbilityBattle through GetBattleMovePriority
// (the Prankster check on every status move it scores), so a foe whose ability the AI
// merely evaluated — but never used in front of the player — must NOT be marked revealed.
// Regression for the foe's ability wrongly displaying at battle start / on switch-in.
AI_SINGLE_BATTLE_TEST("Frontier INFO: AI scoring a Prankster status move does not reveal the foe")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_WOBBUFFET) { HP(1); }
        // Whimsicott has Prankster + a status move (Tailwind) the AI will score during its
        // calc, but it picks Moonblast to KO, so the status move never executes in view.
        OPPONENT(SPECIES_WHIMSICOTT) { Ability(ABILITY_PRANKSTER); Moves(MOVE_TAILWIND, MOVE_MOONBLAST); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); EXPECT_MOVE(opponent, MOVE_MOONBLAST); }
    } THEN {
        // Value is known (omniscient), but it must not be flagged as seen by the player.
        EXPECT(gAiPartyData->mons[B_SIDE_OPPONENT][0].ability == ABILITY_PRANKSTER);
        EXPECT((gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] & 1u) == 0);
    }
}

// FORK: B_FRONTIER_BATTLE_INFO. Once a foe's ability is genuinely witnessed, the viewer must
// keep showing *that* ability even though the AI's speculative move/switch evaluation later
// overwrites gAiPartyData->mons[].ability (its live, mutable knowledge model). The viewer reads
// a reveal-time snapshot (gBattleStruct->infoRevealedAbility) instead, so a speculative record of
// a different ability — e.g. a benched Prankster mon simulated in the active slot whose turn-order
// check records Prankster onto that slot — can't change what the player sees. Without the snapshot,
// a Krookodile whose Intimidate fired at battle start displayed as "Prankster" in the viewer.
AI_SINGLE_BATTLE_TEST("Frontier INFO: a speculative ability record does not corrupt an already-revealed foe ability")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_SLOWKING) { Ability(ABILITY_OWN_TEMPO); Moves(MOVE_SPLASH); }
        OPPONENT(SPECIES_KROOKODILE) { Ability(ABILITY_INTIMIDATE); Moves(MOVE_EARTHQUAKE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); }
    } THEN {
        // Intimidate fired on switch-in: genuinely revealed (bit set, snapshot taken).
        EXPECT(gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] & 1u);
        EXPECT(gBattleStruct->infoRevealedAbility[B_SIDE_OPPONENT][0] == ABILITY_INTIMIDATE);

        // Simulate the AI's speculative evaluation recording a different ability onto the
        // active foe's slot (mid-calc, so it updates the knowledge model but takes no reveal).
        gAiLogicData->aiCalcInProgress = TRUE;
        RecordAbilityBattle(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), ABILITY_PRANKSTER);
        gAiLogicData->aiCalcInProgress = FALSE;

        // The AI's model is clobbered, but the player-facing snapshot the viewer reads is intact.
        EXPECT(gAiPartyData->mons[B_SIDE_OPPONENT][0].ability == ABILITY_PRANKSTER);
        EXPECT(gBattleStruct->infoRevealedAbility[B_SIDE_OPPONENT][0] == ABILITY_INTIMIDATE);
    }
}

// FORK: FEATURE_INNATE_ABILITIES + B_FRONTIER_BATTLE_INFO. Innates are a static property of the
// species, so the viewer shows them unconditionally (they are not reveal-gated). But witnessing an
// innate (here an innate Levitate blocking a Ground move, which forces its own ability pop-up) must
// still NOT reveal the *chosen* ability: the chosen-ability reveal bit stays clear, so the viewer
// keeps the chosen slot as "?" and the line reads "? (+Levitate, Sturdy)". Magnemite's chosen
// ability is forced to Honey Gather (no in-battle effect, so the engine never records it),
// isolating the effect under test to the innate Levitate pop-up.
SINGLE_BATTLE_TEST("Frontier INFO: witnessing an innate does not reveal the chosen ability")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MAGNEMITE, ABILITY_LEVITATE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_SANDSLASH) { Moves(MOVE_EARTHQUAKE); }
        OPPONENT(SPECIES_MAGNEMITE) { Ability(ABILITY_HONEY_GATHER); Moves(MOVE_THUNDER_WAVE); }
    } WHEN {
        TURN { MOVE(player, MOVE_EARTHQUAKE); }
    } THEN {
        // The innate Levitate pop-up fired, but the chosen ability stays hidden — the viewer
        // shows "? (+Levitate, Sturdy)" rather than leaking the chosen ability.
        EXPECT((gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] & 1u) == 0);
    }
}

// FORK: B_FRONTIER_BATTLE_INFO. The viewer's Base Stats page prints the foe's own spread and
// then one row per Mega/Primal form its species can reach, read from the species' own
// form-change table. That list is bounded by INFO_MAX_DISPLAYED_ALT_FORMS, so a species
// declaring more of those entries than the page can hold would have a reachable form the page
// silently drops — under FEATURE_FREE_GIMMICKS a form the foe can actually turn into. Same
// shape of guard as the innates-page row budget: a table row that passes it is a data problem,
// not a display one.
TEST("Frontier INFO: no species declares more Mega/Primal forms than the Base Stats page can list")
{
    u32 offenders = 0, fullest = 0, carriers = 0;

    for (u32 species = 1; species < NUM_SPECIES; species++)
    {
        const struct FormChange *formChanges;
        u32 n = 0;

        // GetSpeciesFormChanges -> SanitizeSpeciesId asserts on a species the build has
        // disabled, so the sweep skips those. The viewer never hits this: it only ever asks
        // about a species that is on the field.
        if (!IsSpeciesEnabled(species))
            continue;

        formChanges = GetSpeciesFormChanges(species);
        if (formChanges == NULL)
            continue;

        for (u32 i = 0; formChanges[i].method != FORM_CHANGE_TERMINATOR; i++)
        {
            if (formChanges[i].method == FORM_CHANGE_BATTLE_MEGA_EVOLUTION_ITEM
                || formChanges[i].method == FORM_CHANGE_BATTLE_PRIMAL_REVERSION)
                n++;
        }

        if (n == 0)
            continue;

        carriers++;
        if (n > fullest)
            fullest = n;
        if (n > INFO_MAX_DISPLAYED_ALT_FORMS)
        {
            offenders++;
            Test_MgbaPrintf("%S declares %d Mega/Primal forms, more than the Base Stats page's %d rows -- raise INFO_MAX_DISPLAYED_ALT_FORMS",
                            gSpeciesInfo[species].speciesName, n, INFO_MAX_DISPLAYED_ALT_FORMS);
        }
    }

    // Guard against a vacuous pass if the form-change accessor ever breaks: Mega Evolution is
    // compiled in (P_MEGA_EVOLUTIONS), so plenty of species must carry such an entry, and at
    // least one (Charizard/Mewtwo) must carry two.
    EXPECT_GT(carriers, 20);
    EXPECT_GE(fullest, 2);
    EXPECT_EQ(offenders, 0);
}
