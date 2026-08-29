#include "global.h"
#include "test/test.h"
#include "data.h"
#include "battle_frontier.h"
#include "config_changes.h"
#include "item.h" // FORK: gItemsInfo (accuracy-item redundancy test)
#include "fork/frontier_extended_mons.h"
#include "fork/innate_abilities.h"
#include "fork/species_ability_overrides.h"
#include "constants/abilities.h"
#include "constants/items.h" // FORK: ITEM_WIDE_LENS / ITEM_ZOOM_LENS
#include "constants/species.h"
#include "constants/items.h"
#include "move.h"
#include "battle.h"

// FORK: guards the fork-owned competitive Battle Factory roster
// (gFrontierExtendedMons, src/frontier_extended_mons.c). A set's .ability is
// resolved by CreateFacilityMon (src/battle_frontier.c) into a 2-bit ability
// *slot* on the mon, so it must be one of the species' real abilities. An
// off-list ability has no slot to live in and silently falls back to slot 0,
// quietly running a different ability than the set intends (this is how the
// Munkidori "Regenerator pivot" was actually battling as Toxic Chain). This
// test loops the whole roster and fails loudly on any such illegal entry.
TEST("Frontier extended roster: every set's ability is legal for its species")
{
    u32 i, slot;
    u32 illegalCount = 0;

    // The fork ability layer (innates + the species ability-override table) is gated by
    // FEATURE_INNATE_ABILITIES, which TestInitConfigData force-disables by default. The real
    // frontier (CreateFacilityMon) runs with it on, so opt in to resolve overridden slots.
    SetConfig(CONFIG_FEATURE_INNATE_ABILITIES, TRUE);

    for (i = 0; i < gFrontierExtendedMonsCount; i++)
    {
        const struct TrainerMon *set = &gFrontierExtendedMons[i];
        enum Ability ability = set->ability;
        bool32 isLegal = FALSE;

        // ABILITY_NONE means "let the Factory pick", which is always valid.
        if (ability == ABILITY_NONE)
            continue;

        for (slot = 0; slot < NUM_ABILITY_SLOTS; slot++)
        {
            if (GetSpeciesAbility(set->species, slot) == ability)
            {
                isLegal = TRUE;
                break;
            }
        }

        if (!isLegal)
        {
            illegalCount++;
            Test_MgbaPrintf("roster[%d] %S: illegal %S | slots %S/%S/%S | innL=%d innR=%d",
                            i,
                            gSpeciesInfo[set->species].speciesName,
                            gAbilitiesInfo[ability].name,
                            gAbilitiesInfo[GetSpeciesAbility(set->species, 0)].name,
                            gAbilitiesInfo[GetSpeciesAbility(set->species, 1)].name,
                            gAbilitiesInfo[GetSpeciesAbility(set->species, 2)].name,
                            SpeciesHasInnate(set->species, ABILITY_LEVITATE),
                            SpeciesHasInnate(set->species, ABILITY_REGENERATOR));
        }
    }

    EXPECT_EQ(illegalCount, 0);
}

// FORK: a frontier set's .ability is the mon's single CHOSEN (observable) ability,
// layered ON TOP of that species' always-on innates (fork/innate_abilities.c). When the
// chosen ability is ITSELF one of the species' innates, the chosen slot is REDUNDANT: the
// mon would already have that ability from its innate, so the one observable pick is wasted
// (the set intends a second, visible trait and gets a duplicate instead).
//
// Every such set has been given a real, non-innate chosen ability -- repointed to a free real
// slot, or handed a stable pick via a fork-owned override in src/fork/species_ability_overrides.c
// (an ability absent from sImplementedInnates[] -- never itself an innate -- or an
// already-implemented innate the species does not carry). Species that once had NO repurposable
// slot (every real ability innate AND every slot pinned by a battle test -- Snorlax, Kangaskhan,
// Pinsir, Sableye, Clefable, Slowbro, Camerupt, ...) are now convertible too: the override table
// is gated by FEATURE_INNATE_ABILITIES and thus invisible to upstream tests (see
// GetSpeciesAbilityOverride), so a row can repurpose a slot an upstream test pins without
// perturbing it -- only flag-on fork tests observe the override, and each species kept a free slot
// clear of those. So the whole roster now carries a real chosen ability; ABILITY_NONE is banned
// (see the next test).
//
// This test asserts the invariant holds for the WHOLE roster with no exceptions: a chosen
// ability is either ABILITY_NONE or NOT one of the species' innates. It fails loudly if a new
// set ever reintroduces redundancy.
TEST("Frontier extended roster: no set's chosen ability duplicates a species innate")
{
    u32 i;
    u32 redundant = 0;

    for (i = 0; i < gFrontierExtendedMonsCount; i++)
    {
        const struct TrainerMon *set = &gFrontierExtendedMons[i];
        enum Ability ability = set->ability;

        // ABILITY_NONE lets the Factory pick; a chosen ability that is NOT an innate is the
        // goal (a real, observable second trait), so both are fine.
        if (ability == ABILITY_NONE)
            continue;
        if (!SpeciesHasInnate(set->species, ability))
            continue;

        redundant++;
        Test_MgbaPrintf("roster[%d] %S: chosen %S duplicates a species innate -- give it a non-innate chosen ability (fork override / repoint) or ABILITY_NONE",
                        i,
                        gSpeciesInfo[set->species].speciesName,
                        gAbilitiesInfo[ability].name);
    }

    EXPECT_EQ(redundant, 0);
}

// FORK: the same redundancy invariant as the test above, but checked at its SOURCE -- the
// fork-owned override table (src/fork/species_ability_overrides.c) rather than the roster that
// consumes it. An override row exists to hand a species a real, OBSERVABLE second trait alongside
// its always-on innates, so a row granting an ability the species already has innately defeats its
// own purpose: the chosen slot resolves to a duplicate and the mon shows one trait instead of two.
//
// The roster test above only sees a bad row once some set actually selects that slot, so a
// redundant row could sit in the table indefinitely -- and would then be inherited by the next set
// authored on that species (exactly the trap a line review walks into: propose an override against
// an innate, then build a set on it). This test sweeps EVERY species x slot through
// GetSpeciesAbilityOverride and fails on any row that duplicates an innate, independent of roster
// coverage. Species with no override row return ABILITY_NONE and are skipped.
TEST("Frontier extended roster: no species ability override duplicates a species innate")
{
    u32 species, slot;
    u32 redundant = 0;
    u32 rowsSeen = 0;

    // GetSpeciesAbilityOverride returns ABILITY_NONE for every species while the flag is off
    // (TestInitConfigData force-disables it), which would make this test vacuously pass.
    SetConfig(CONFIG_FEATURE_INNATE_ABILITIES, TRUE);

    for (species = 1; species < NUM_SPECIES; species++)
    {
        for (slot = 0; slot < NUM_ABILITY_SLOTS; slot++)
        {
            enum Ability ability = GetSpeciesAbilityOverride(species, slot);

            if (ability == ABILITY_NONE)
                continue;

            rowsSeen++;
            if (!SpeciesHasInnate(species, ability))
                continue;

            redundant++;
            Test_MgbaPrintf("override %S slot %d: %S is already an innate of this species -- the chosen slot would resolve to a duplicate; pick a non-innate ability for the row or drop it",
                            gSpeciesInfo[species].speciesName,
                            slot,
                            gAbilitiesInfo[ability].name);
        }
    }

    // Guard against a vacuous pass: if the flag gating on GetSpeciesAbilityOverride ever changes
    // so that it stops resolving here, the sweep above would silently check nothing.
    EXPECT_GT(rowsSeen, 0);
    EXPECT_EQ(redundant, 0);
}

// FORK: ABILITY_NONE on a set means "let the Factory pick the ability at draft", which yields a
// non-deterministic, unlabeled ability -- and for the fork's all-innate species it can only ever
// land on a redundant innate. Every set must instead name a real, deliberate chosen ability. This
// was previously impossible for species whose every real slot was pinned by an upstream battle test
// (the override that would free a slot also rewrote it inside that test); gating the override table
// behind FEATURE_INNATE_ABILITIES fixed that (the override is invisible to flag-off upstream tests),
// so the escape hatch is gone. This test bans ABILITY_NONE outright and fails loudly if a new set
// reintroduces it -- give the set a fork override (src/fork/species_ability_overrides.c) instead.
// FORK: BUFF_ACCURACY_ITEMS (config/buff.h) gives Wide Lens and Zoom Lens a job inside the
// DETERMINISTIC_ACCURACY_EVASION PP economy: Wide Lens cancels the flat evasion taxes a target
// imposes (BrightPowder / Lax Incense, Sand Veil in sand, Snow Cloak in snow, Tangled Feet while
// confused, Wonder Skin vs a status move), and Zoom Lens cancels those AND the whole stat-stage
// half while its holder moves second.
//
// Some abilities already do that job for free, which makes THAT HALF of the item worthless -- the
// same authoring trap as handing an innate-Levitate mon an Air Balloon. Two distinct wastes exist,
// and this test fails on both:
//
//   1. NO GUARD kills both items outright. GetDeterministicMoveTargetPPTax() returns 0 outright
//      for a No Guard attacker, and GetAccEvasionStageDelta() forces its ignorePenalties path, so
//      there is no penalty left for either lens to cancel. The item does literally nothing.
//   2. An ability that already ignores the target's EVASION STAGES -- Compound Eyes, Keen Eye,
//      Mind's Eye, Unaware, Victory Star, and (at B_ILLUMINATE_EFFECT >= GEN_9) Illuminate --
//      covers exactly Zoom Lens's differentiator. Zoom Lens then relieves nothing Wide Lens
//      would not, while still being restricted to the turns its holder moves second, so it is
//      strictly worse than Wide Lens on that mon and the set should just hold Wide Lens.
//
// Both the set's CHOSEN ability and the species' always-on innates count, since either grants
// the effect in battle. Rule 2 deliberately does not fire on Wide Lens: the flat-tax half is
// still live for a Compound Eyes holder, so Wide Lens there is a real (if narrow) pick.
//
// SCOPE -- why this only runs with BUFF_ACCURACY_ITEMS_REVEAL off. That flag gives the lenses a
// SECOND, ability-independent job: feeding the INFO viewer's reveal bits (Wide Lens = every seen
// foe's held item, Zoom Lens = a watched foe's ability and full moveset). No ability neutralises
// that, so with the reveal on a lens ALWAYS does something and neither rule below is a real
// error any more -- a No Guard mon holding one still gets the reveal, and Zoom Lens still buys
// depth where Wide Lens buys breadth, so it is not "strictly worse" either. Failing those sets
// would reject legitimate authoring, so the sweep is scoped to the configuration where the PP
// relief is the item's whole job. It stays a live guard if the reveal is ever turned off, and
// documents exactly what the reveal is carrying. Do not widen it back without re-checking that
// premise.
TEST("Frontier extended roster: no set holds an accuracy item its ability already makes redundant")
{
    static const enum Ability sEvasionStageIgnorers[] =
    {
        ABILITY_COMPOUND_EYES,
        ABILITY_KEEN_EYE,
        ABILITY_MINDS_EYE,
        ABILITY_UNAWARE,
        ABILITY_VICTORY_STAR,
    };

    u32 i, j;
    u32 wasted = 0;

    // The innate layer is gated by FEATURE_INNATE_ABILITIES, which TestInitConfigData
    // force-disables by default; the real frontier runs with it on, so opt in.
    SetConfig(CONFIG_FEATURE_INNATE_ABILITIES, TRUE);
    // Judge the roster as if the PP relief were the lenses' only job -- see SCOPE above.
    SetConfig(CONFIG_BUFF_ACCURACY_ITEMS_REVEAL, FALSE);

    for (i = 0; i < gFrontierExtendedMonsCount; i++)
    {
        const struct TrainerMon *set = &gFrontierExtendedMons[i];
        bool32 isWideLens = (set->heldItem == ITEM_WIDE_LENS);
        bool32 isZoomLens = (set->heldItem == ITEM_ZOOM_LENS);
        bool32 ignoresStages = FALSE;

        if (!isWideLens && !isZoomLens)
            continue;

        // Rule 1: No Guard leaves neither lens anything to cancel.
        if (set->ability == ABILITY_NO_GUARD || SpeciesHasInnate(set->species, ABILITY_NO_GUARD))
        {
            wasted++;
            Test_MgbaPrintf("roster[%d] %S: %S buys no PP relief next to No Guard (no accuracy penalty is ever charged) -- pick another item",
                            i,
                            gSpeciesInfo[set->species].speciesName,
                            gItemsInfo[set->heldItem].name);
            continue;
        }

        if (!isZoomLens)
            continue;

        // Rule 2: Zoom Lens's stage half, already covered by an ability.
        for (j = 0; j < ARRAY_COUNT(sEvasionStageIgnorers); j++)
        {
            if (set->ability == sEvasionStageIgnorers[j] || SpeciesHasInnate(set->species, sEvasionStageIgnorers[j]))
            {
                ignoresStages = TRUE;
                break;
            }
        }
        // Illuminate only ignores evasion from Gen 9 on; below that it is not in this club.
        if (!ignoresStages
         && GetConfig(B_ILLUMINATE_EFFECT) >= GEN_9
         && (set->ability == ABILITY_ILLUMINATE || SpeciesHasInnate(set->species, ABILITY_ILLUMINATE)))
            ignoresStages = TRUE;

        if (ignoresStages)
        {
            wasted++;
            Test_MgbaPrintf("roster[%d] %S: Zoom Lens is strictly worse than Wide Lens here -- its ability already ignores the target's evasion stages, so only the flat-tax half is left and Zoom Lens pays a moving-second condition for it",
                            i,
                            gSpeciesInfo[set->species].speciesName);
        }
    }

    EXPECT_EQ(wasted, 0);
}

TEST("Frontier extended roster: no set uses ABILITY_NONE (every set names a real chosen ability)")
{
    u32 i;
    u32 noneCount = 0;

    for (i = 0; i < gFrontierExtendedMonsCount; i++)
    {
        const struct TrainerMon *set = &gFrontierExtendedMons[i];

        if (set->ability != ABILITY_NONE)
            continue;

        noneCount++;
        Test_MgbaPrintf("roster[%d] %S: uses ABILITY_NONE -- give it a real chosen ability via a fork override (species_ability_overrides.c)",
                        i,
                        gSpeciesInfo[set->species].speciesName);
    }

    EXPECT_EQ(noneCount, 0);
}

// FORK: CreateFacilityMon grants the Gigantamax Factor at draft time to any mon
// whose species has a G-Max form, so gmax-capable Factory/Tower mons Gigantamax
// instead of plain Dynamaxing (without annotating each roster entry). A species
// with no G-Max form must NOT receive the factor.
TEST("Frontier extended roster: drafted mon gets Gigantamax Factor iff its species has a G-Max form")
{
    struct Pokemon mon;
    // Minimal sets; CreateFacilityMon reads species/moves, the rest may stay 0.
    const struct TrainerMon gmaxSet = { .species = SPECIES_CHARIZARD, .moves = { MOVE_TACKLE } };
    const struct TrainerMon plainSet = { .species = SPECIES_SALAMENCE, .moves = { MOVE_TACKLE } };

    CreateFacilityMon(&gmaxSet, 50, MAX_PER_STAT_IVS, 0, 0, &mon);
    EXPECT(GetMonData(&mon, MON_DATA_GIGANTAMAX_FACTOR));

    CreateFacilityMon(&plainSet, 50, MAX_PER_STAT_IVS, 0, 0, &mon);
    EXPECT(!GetMonData(&mon, MON_DATA_GIGANTAMAX_FACTOR));
}

// FORK: CreateFacilityMon grants the maximum Dynamax Level at draft time so a
// Dynamaxed mon gets the full HP boost. An explicit per-entry .dynamaxLevel
// still overrides the default.
TEST("Frontier extended roster: drafted mon gets the maximum Dynamax Level by default")
{
    struct Pokemon mon;
    const struct TrainerMon defaultSet = { .species = SPECIES_SALAMENCE, .moves = { MOVE_TACKLE } };
    const struct TrainerMon explicitSet = { .species = SPECIES_SALAMENCE, .moves = { MOVE_TACKLE }, .dynamaxLevel = 5 };

    CreateFacilityMon(&defaultSet, 50, MAX_PER_STAT_IVS, 0, 0, &mon);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_DYNAMAX_LEVEL), MAX_DYNAMAX_LEVEL);

    CreateFacilityMon(&explicitSet, 50, MAX_PER_STAT_IVS, 0, 0, &mon);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_DYNAMAX_LEVEL), 5);
}

// ============================================================================
// Roster coverage
// ============================================================================
//
// FORK: the roster's headline promise (fork-docs/FRONTIER_ROSTER.md, "Coverage") is that
// EVERY fully-evolved species -- alternate formes included -- has at least one build. That
// promise was prose only, so a species arriving with an upstream sync (a new generation, a
// new regional forme) could sit unbuilt forever with nothing failing. The tests below make
// it a CI gate: every species is either built or *accounted for* by a reasoned row here.
//
// A species must have a set unless one of these derived rules excuses it. They are
// mechanical, so species the fork has never seen classify themselves:
//
//  - COVERAGE_UNIMPLEMENTED  compiled out of this build (a disabled generation/family).
//  - COVERAGE_NOT_FULLY_EVOLVED  it still evolves. Overridden by sNicheNfes[], the NFEs the
//        roster deliberately DOES build (Eviolite Chansey and friends).
//  - COVERAGE_TRANSFORMED_FORM  a temporary in-battle shape: Mega/Primal/Ultra Burst/
//        Gigantamax/Tera/Totem by species flag, or a non-base forme its own form-change
//        table enters mid-battle (Aegislash-Blade, Darmanitan-Zen, Mimikyu-Busted,
//        Palafin-Hero, Minior's cores). You never rent these -- you rent the base forme and
//        the engine swaps to them.
//
// The rest must be covered, and coverage is judged per *battle profile* rather than per
// species id: formes sharing a National Dex number, base stat spread and ability set are one
// requirement, satisfied by any one of them having a set. That is what makes Vivillon's 19
// patterns, Unown's 27 letters and the cap Pikachus one line of work each instead of sixty.
// Typing is deliberately NOT part of the profile, so Arceus' plates and Silvally's memories
// are one requirement too (a Factory rental is "an Arceus", whatever it holds); the cost is
// that Tauros' Paldean breeds and Urshifu's styles also collapse into one -- build them
// anyway, the test just will not insist.
//
// Anything still uncovered must be listed in sCoverageExceptions[], one row per profile, with a
// structural reason: the forme cannot be rented (unobtainable event formes, battle-only shapes
// the derived rules miss) or a sibling profile the roster does build stands in for it
// (Gourgeist-Super covers the other three sizes). A row is not a place to park a species you
// have not got to yet -- the thirteen genuine gaps this sweep first turned up were built, and
// the next one should be too.

// FORK: NFEs the roster builds on purpose -- Eviolite (and friends) give these a role their
// evolution does not dominate, so they are roster-required despite still evolving. The list is
// exact in both directions (see "the roster's NFEs are exactly the niche NFEs"): a set for an
// unlisted NFE fails, which is what catches a pre-evolution typo'd in place of its evolution
// (SPECIES_GRAVELER where SPECIES_GOLEM was meant).
static const u16 sNicheNfes[] =
{
    SPECIES_CHANSEY,   // Eviolite wall; Blissey trades that bulk for raw HP.
    SPECIES_URSARING,  // Eviolite Guts attacker; Ursaluna is slower and Peat-Block-locked.
    SPECIES_PORYGON2,  // Eviolite Trace/Download tank; Porygon-Z is the frail nuke instead.
    SPECIES_DUSCLOPS,  // Eviolite Ghost wall; Dusknoir gains offense but loses the item.
    SPECIES_PIKACHU,   // Light Ball doubles both attacking stats, putting it above Raichu's.
};

struct RosterCoverageException
{
    u16 species;
    const char *why;
};

static const struct RosterCoverageException sCoverageExceptions[] =
{
    // ---- In the data, but never a rentable Pokémon ----
    { SPECIES_MELTAN,              "no evolution data (its candy evolution is unimplemented), so it reads as fully evolved; it is Melmetal's pre-evo, and Melmetal is built" },
    { SPECIES_PICHU_SPIKY_EARED,   "event-only forme, and a pre-evo besides" },
    { SPECIES_PIKACHU_STARTER,     "LGPE partner-only forme; cannot be obtained or rented" },
    { SPECIES_EEVEE_STARTER,       "LGPE partner-only forme; cannot be obtained or rented" },
    { SPECIES_FLOETTE_ETERNAL,     "unreleased event forme; cannot be obtained or rented" },
    { SPECIES_ETERNATUS_ETERNAMAX, "battle-only shape with no legal encounter; its stats are not balanceable against anything" },
    { SPECIES_GRENINJA_ASH,        "Battle Bond transformation, entered mid-battle; the roster builds Greninja and Greninja-Battle-Bond" },
    { SPECIES_ZYGARDE_COMPLETE,    "Power Construct shape, entered mid-battle from the 10%/50% formes" },

    // ---- A sibling profile the roster does build stands in ----
    { SPECIES_DEOXYS_NORMAL,       "the Attack/Defense/Speed formes carry Deoxys" },
    { SPECIES_GOURGEIST_AVERAGE,   "Gourgeist-Super carries the size formes" },
    { SPECIES_GOURGEIST_SMALL,     "Gourgeist-Super carries the size formes" },
    { SPECIES_GOURGEIST_LARGE,     "Gourgeist-Super carries the size formes" },
    { SPECIES_SQUAWKABILLY_YELLOW, "plumage variant differing from Green only in its hidden ability; Squawkabilly-Green carries the line" },
    { SPECIES_ZYGARDE_50,          "Aura Break variant; the roster builds Zygarde-50%-Power-Construct" },
    { SPECIES_ZYGARDE_10_AURA_BREAK, "Aura Break variant; the roster builds Zygarde-10%-Power-Construct" },
    { SPECIES_TERAPAGOS_NORMAL,    "Tera Shift converts it on send-out, so the roster builds Terapagos-Terastal directly" },
};

enum RosterCoverage
{
    COVERAGE_REQUIRED,           // must be covered by a set (or a row in sCoverageExceptions)
    COVERAGE_UNIMPLEMENTED,      // species compiled out of this build
    COVERAGE_NOT_FULLY_EVOLVED,  // still evolves, and is not one of sNicheNfes
    COVERAGE_TRANSFORMED_FORM,   // temporary in-battle shape; the base forme is what you rent
};

static bool32 RosterHasSpecies(u32 species)
{
    for (u32 i = 0; i < gFrontierExtendedMonsCount; i++)
    {
        if (gFrontierExtendedMons[i].species == species)
            return TRUE;
    }
    return FALSE;
}

static bool32 IsNicheNfe(u32 species)
{
    for (u32 i = 0; i < ARRAY_COUNT(sNicheNfes); i++)
    {
        if (sNicheNfes[i] == species)
            return TRUE;
    }
    return FALSE;
}

static const char *CoverageExceptionReason(u32 species)
{
    for (u32 i = 0; i < ARRAY_COUNT(sCoverageExceptions); i++)
    {
        if (sCoverageExceptions[i].species == species)
            return sCoverageExceptions[i].why;
    }
    return NULL;
}

static bool32 SpeciesStillEvolves(u32 species)
{
    const struct Evolution *evolutions = GetSpeciesEvolutions(species);

    if (evolutions == NULL)
        return FALSE;

    for (u32 i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
    {
        // A family switched off by a P_GEN_x_POKEMON/P_FAMILY_x flag leaves its evolution rows
        // behind; a dangling row does not make this species a pre-evolution in THIS build.
        if (IsSpeciesEnabled(evolutions[i].targetSpecies))
            return TRUE;
    }
    return FALSE;
}

// A non-base forme that its own form-change table enters through a mid-battle trigger is a
// transformation, not a rentable Pokémon: the Factory drafts the base forme and the engine
// swaps to this shape during the fight. Reading the species' OWN table (rather than sweeping
// every table looking for it as a target) is enough, because a family shares one table across
// its formes -- and it is what keeps base formes like Castform and Palafin-Zero, which the same
// table also targets, on the required list.
static bool32 IsTransformedBattleForm(u32 species)
{
    const struct FormChange *formChanges;

    if (gSpeciesInfo[species].isMegaEvolution
     || gSpeciesInfo[species].isPrimalReversion
     || gSpeciesInfo[species].isUltraBurst
     || gSpeciesInfo[species].isGigantamax
     || gSpeciesInfo[species].isTeraForm
     || gSpeciesInfo[species].isTotem)
        return TRUE;

    if (GET_BASE_SPECIES_ID(species) == species)
        return FALSE;

    formChanges = GetSpeciesFormChanges(species);
    if (formChanges == NULL)
        return FALSE;

    for (u32 i = 0; formChanges[i].method != FORM_CHANGE_TERMINATOR; i++)
    {
        if (formChanges[i].targetSpecies != species)
            continue;

        switch (formChanges[i].method)
        {
        case FORM_CHANGE_BATTLE_SWITCH_OUT:
        case FORM_CHANGE_BATTLE_SWITCH_IN:
        case FORM_CHANGE_BATTLE_HP_PERCENT_TURN_END:
        case FORM_CHANGE_BATTLE_HP_PERCENT_SEND_OUT:
        case FORM_CHANGE_BATTLE_HP_PERCENT_DURING_MOVE:
        case FORM_CHANGE_BATTLE_WEATHER:
        case FORM_CHANGE_BATTLE_TURN_END:
        case FORM_CHANGE_BATTLE_HIT_BY_MOVE_CATEGORY:
        case FORM_CHANGE_BATTLE_HIT_BY_CONFUSION_SELF_DMG:
        case FORM_CHANGE_BATTLE_TERASTALLIZATION:
        case FORM_CHANGE_BATTLE_BEFORE_MOVE:
        case FORM_CHANGE_BATTLE_BEFORE_MOVE_CATEGORY:
        case FORM_CHANGE_BATTLE_AFTER_MOVE:
        case FORM_CHANGE_STATUS:
        case FORM_CHANGE_BEGIN_WILD_ENCOUNTER:
            return TRUE;
        default:
            break;
        }
    }
    return FALSE;
}

static enum RosterCoverage ClassifySpeciesForRoster(u32 species)
{
    if (!IsSpeciesEnabled(species))
        return COVERAGE_UNIMPLEMENTED;
    if (IsNicheNfe(species))
        return COVERAGE_REQUIRED;
    if (SpeciesStillEvolves(species))
        return COVERAGE_NOT_FULLY_EVOLVED;
    if (IsTransformedBattleForm(species))
        return COVERAGE_TRANSFORMED_FORM;
    return COVERAGE_REQUIRED;
}

// Two formes are the same requirement when they are the same Pokémon in the same shape: one
// National Dex number, one base stat spread, one ability set. Typing is left out on purpose --
// see the header comment.
static bool32 SameBattleProfile(u32 a, u32 b)
{
    if (!IsSpeciesEnabled(a) || !IsSpeciesEnabled(b))
        return FALSE;
    if (SpeciesToNationalPokedexNum(a) != SpeciesToNationalPokedexNum(b))
        return FALSE;

    for (u32 stat = 0; stat < NUM_STATS; stat++)
    {
        if (GetSpeciesBaseStat(a, stat) != GetSpeciesBaseStat(b, stat))
            return FALSE;
    }
    // Read the abilities straight out of gSpeciesInfo rather than through GetSpeciesAbility:
    // that accessor applies the fork's per-species ability overrides, which are keyed by exact
    // species id, so a base forme carrying an override would look like a different Pokémon from
    // its own skins (Zarude vs Zarude-Dada).
    for (u32 slot = 0; slot < NUM_ABILITY_SLOTS; slot++)
    {
        if (gSpeciesInfo[a].abilities[slot] != gSpeciesInfo[b].abilities[slot])
            return FALSE;
    }
    return TRUE;
}

// TRUE when this species' profile is already answered for -- by a set on any forme sharing it,
// or by an exception row on any of them.
static bool32 ProfileIsAccountedFor(u32 species)
{
    for (u32 other = 1; other < NUM_SPECIES; other++)
    {
        if (!SameBattleProfile(species, other))
            continue;
        if (RosterHasSpecies(other) || CoverageExceptionReason(other) != NULL)
            return TRUE;
    }
    return FALSE;
}

// One complaint per profile: the lowest-numbered required forme speaks for the rest, so a
// missing Vivillon is one line naming Vivillon-Icy-Snow, not twenty naming every pattern.
static bool32 IsProfileSpokesman(u32 species)
{
    for (u32 other = 1; other < species; other++)
    {
        if (SameBattleProfile(species, other) && ClassifySpeciesForRoster(other) == COVERAGE_REQUIRED)
            return FALSE;
    }
    return TRUE;
}

TEST("Frontier extended roster: every fully-evolved species has at least one set")
{
    u32 required = 0;
    u32 uncovered = 0;

    for (u32 species = 1; species < NUM_SPECIES; species++)
    {
        if (ClassifySpeciesForRoster(species) != COVERAGE_REQUIRED)
            continue;

        required++;
        if (ProfileIsAccountedFor(species) || !IsProfileSpokesman(species))
            continue;

        uncovered++;
        Test_MgbaPrintf("uncovered %S (species %d): fully evolved, no set on any forme of this profile -- add a build to gFrontierExtendedMons, or a reasoned row to sCoverageExceptions[]",
                        gSpeciesInfo[species].speciesName, species);
    }

    // Guard against a vacuous pass: if the classifier ever starts excusing everything (a renamed
    // species flag, a form-change method reshuffle), the sweep above would check nothing.
    EXPECT_GT(required, 600);
    EXPECT_EQ(uncovered, 0);
}

// FORK: an exception row promises a profile genuinely cannot be, or need not be, built. Rows rot
// silently in three ways: the profile gets a set (the row now excuses nothing), a derived rule
// grows to cover the species (an upstream sync flags the forme Tera, say), or a second row lands
// on the same profile. Dead rows are worse than no rows -- they hide the next real gap -- so each
// one has to justify itself every run.
TEST("Frontier extended roster: no stale coverage exception")
{
    u32 stale = 0;

    for (u32 i = 0; i < ARRAY_COUNT(sCoverageExceptions); i++)
    {
        u32 species = sCoverageExceptions[i].species;
        enum RosterCoverage coverage = ClassifySpeciesForRoster(species);
        bool32 built = FALSE;

        for (u32 other = 1; other < NUM_SPECIES; other++)
        {
            if (SameBattleProfile(species, other) && RosterHasSpecies(other))
            {
                built = TRUE;
                break;
            }
        }

        for (u32 j = 0; j < i; j++)
        {
            if (!SameBattleProfile(species, sCoverageExceptions[j].species))
                continue;
            stale++;
            Test_MgbaPrintf("exception %S: same profile as the earlier row for %S -- one row covers the profile, so drop this one",
                            gSpeciesInfo[species].speciesName,
                            gSpeciesInfo[sCoverageExceptions[j].species].speciesName);
        }

        if (built)
        {
            stale++;
            Test_MgbaPrintf("exception %S: this profile now has a set -- delete the row (\"%s\")",
                            gSpeciesInfo[species].speciesName, sCoverageExceptions[i].why);
        }
        else if (coverage != COVERAGE_REQUIRED)
        {
            stale++;
            Test_MgbaPrintf("exception %S: already excused by derived rule %d -- delete the row (\"%s\")",
                            gSpeciesInfo[species].speciesName, coverage, sCoverageExceptions[i].why);
        }
    }

    EXPECT_EQ(stale, 0);
}

// FORK: "only fully-evolved Pokémon appear, except NFEs with a genuine niche" is a roster rule
// (fork-docs/FRONTIER_ROSTER.md), and it cuts both ways. sNicheNfes[] is the complete list of
// deliberate exceptions, so an NFE in the roster that is NOT listed is almost always a typo -- a
// pre-evolution written where its evolution was meant -- and a listed NFE with no set is a
// promise the roster quietly stopped keeping.
TEST("Frontier extended roster: the roster's NFEs are exactly the niche NFEs")
{
    u32 mismatches = 0;

    for (u32 i = 0; i < gFrontierExtendedMonsCount; i++)
    {
        u32 species = gFrontierExtendedMons[i].species;

        if (!SpeciesStillEvolves(species) || IsNicheNfe(species))
            continue;

        mismatches++;
        Test_MgbaPrintf("roster[%d] %S: still evolves -- did you mean its evolution? If the NFE is deliberate (an Eviolite niche), add it to sNicheNfes[]",
                        i, gSpeciesInfo[species].speciesName);
    }

    for (u32 i = 0; i < ARRAY_COUNT(sNicheNfes); i++)
    {
        u32 species = sNicheNfes[i];

        if (!IsSpeciesEnabled(species))
            continue;

        if (!SpeciesStillEvolves(species))
        {
            mismatches++;
            Test_MgbaPrintf("niche NFE %S: is fully evolved, so the coverage sweep requires it anyway -- drop the row",
                            gSpeciesInfo[species].speciesName);
        }
        if (!RosterHasSpecies(species))
        {
            mismatches++;
            Test_MgbaPrintf("niche NFE %S: listed as a deliberate NFE but has no set -- build one or drop the row",
                            gSpeciesInfo[species].speciesName);
        }
    }

    EXPECT_EQ(mismatches, 0);
}

// ===== Line-review ratchets ===============================================================
//
// FORK: two set-shape gates the /line-review sweep of Gen 1-3 turned into recurring findings.
// Both are RATCHETS, matching the innate-row gates in test/fork/innate_abilities.c: Gens 4-9 have
// not been line-reviewed and carry well over a hundred instances between them, so failing on those
// would only wedge CI. Each batch raises this bound as it lands, and the gate holds everything
// already reviewed.
//
// KEEP IN SYNC with sInnateRowsReviewedThroughDex in test/fork/innate_abilities.c -- the two
// constants track the same sweep and should be bumped together.
static const u16 sSetShapeReviewedThroughDex = NATIONAL_DEX_PECHARUNT; // the whole dex is reviewed; this is now the last entry. KEEP IN SYNC with the twin in test/fork/innate_abilities.c

static bool32 SetIsDoublesCapable(const struct TrainerMon *set)
{
    return set->tags == FORMAT_DOUBLES || set->tags == FORMAT_BOTH;
}

// FORK: TARGET_FOES_AND_ALLY moves hit the holder's own partner. Nine sets were carrying one on a
// doubles-tagged build in Gen 3 alone -- Earthquake mostly, plus Surf and Sludge Wave -- every one
// of them damaging its own teammate for the whole format the tag exists to cover. The fix is
// usually a single-target twin (Earthquake -> High Horsepower, Surf -> Muddy Water) or, on a
// special set, the move on the right stat (-> Earth Power).
//
// The self-KO moves are exempt: hitting everything adjacent is what they are, and pressing one is a
// deliberate last act rather than an oversight.
static bool32 IsDeliberateSelfKoMove(enum Move move)
{
    return move == MOVE_EXPLOSION || move == MOVE_SELF_DESTRUCT || move == MOVE_MISTY_EXPLOSION;
}

TEST("Frontier extended roster: no reviewed doubles set carries a move that hits its own ally")
{
    u32 i;
    u32 checked = 0;
    u32 offenders = 0;

    for (i = 0; i < gFrontierExtendedMonsCount; i++)
    {
        const struct TrainerMon *set = &gFrontierExtendedMons[i];
        u32 slot;

        if (SpeciesToNationalPokedexNum(set->species) > sSetShapeReviewedThroughDex)
            continue;
        if (!SetIsDoublesCapable(set))
            continue;

        checked++;
        for (slot = 0; slot < MAX_MON_MOVES; slot++)
        {
            enum Move move = set->moves[slot];

            if (move == MOVE_NONE || IsDeliberateSelfKoMove(move))
                continue;
            if (GetMoveTarget(move) != TARGET_FOES_AND_ALLY)
                continue;

            offenders++;
            Test_MgbaPrintf("roster[%d] %S: %S hits the holder's own partner (TARGET_FOES_AND_ALLY) on a doubles-capable set -- use a single-target twin (Earthquake -> High Horsepower, Surf -> Muddy Water) or retag to FORMAT_SINGLES",
                            i, gSpeciesInfo[set->species].speciesName, GetMoveName(move));
        }
    }

    // Guard against a vacuous pass if the dex bound or the format tags ever change shape.
    EXPECT_GT(checked, 100);
    EXPECT_EQ(offenders, 0);
}

// FORK: a Choice item locks the holder into the first move it uses, so a status move on a Choice set
// is either never reachable or a trap that ends the set's usefulness. Sharpedo shipped a Choice
// Scarf build with Destiny Bond, which can never be both chosen and followed up.
//
// Three exemptions, all of them cases where the lock costs nothing:
//   Trick / Switcheroo -- handing the Choice item away IS the payload.
//   Transform -- Ditto's set is four copies of it, so there is nothing else to be locked out of.
static bool32 IsChoiceLockSafeStatusMove(enum Move move)
{
    return move == MOVE_TRICK || move == MOVE_SWITCHEROO || move == MOVE_TRANSFORM;
}

TEST("Frontier extended roster: no reviewed Choice set carries a status move the lock would strand")
{
    u32 i;
    u32 checked = 0;
    u32 offenders = 0;

    for (i = 0; i < gFrontierExtendedMonsCount; i++)
    {
        const struct TrainerMon *set = &gFrontierExtendedMons[i];
        u32 slot;

        if (SpeciesToNationalPokedexNum(set->species) > sSetShapeReviewedThroughDex)
            continue;
        if (set->heldItem != ITEM_CHOICE_BAND && set->heldItem != ITEM_CHOICE_SPECS
         && set->heldItem != ITEM_CHOICE_SCARF)
            continue;

        checked++;
        for (slot = 0; slot < MAX_MON_MOVES; slot++)
        {
            enum Move move = set->moves[slot];

            if (move == MOVE_NONE || IsChoiceLockSafeStatusMove(move))
                continue;
            if (!IsBattleMoveStatus(move))
                continue;

            offenders++;
            Test_MgbaPrintf("roster[%d] %S: %S is a status move on a Choice set -- the lock makes it unreachable or a dead end; drop the item or drop the move",
                            i, gSpeciesInfo[set->species].speciesName, GetMoveName(move));
        }
    }

    EXPECT_GT(checked, 10);
    EXPECT_EQ(offenders, 0);
}
