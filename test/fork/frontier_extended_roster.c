#include "global.h"
#include "test/test.h"
#include "data.h"
#include "battle_frontier.h"
#include "fork/frontier_extended_mons.h"
#include "fork/innate_abilities.h"
#include "constants/abilities.h"
#include "constants/species.h"

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
// (the set intends a second, visible trait and gets a duplicate instead). The frontier-slot
// freeing sweep (the retired innate-abilities batching plan) removed these wherever a good
// non-innate pick existed, via fork-owned overrides in src/fork/species_ability_overrides.c.
//
// The pairs below are the KNOWN, ACCEPTED remainder and double as the tracked backlog now
// that the batches doc is retired. Two kinds:
//   - "all real abilities innate": every one of the species' real slots is now an innate, so
//     there is no free real slot to repoint to; freeing needs a fork override + a per-species
//     test/battle audit (a future cleanup, like the Batch W sweep). A few are permanent —
//     test-pinned to a specific slot (e.g. Chansey/Blissey/Ludicolo/Snorlax/Clefable, and the
//     chosen-differs-from-innate exemplars used by test/fork/innate_abilities.c).
//   - "free real slot": a non-innate real slot exists, but it is a junk ability (Run Away,
//     Honey Gather, Stall, Minus, Hustle on a special attacker, ...), so repointing would make
//     the set observably worse; these want a fork override to a good :x: pick, not a bare repoint.
//
// This test FAILS on any set NOT on the list whose chosen ability duplicates an innate, so new
// redundancy cannot be introduced silently and the backlog cannot grow unnoticed. Shrink the
// list as species are given a real, non-innate chosen ability.
TEST("Frontier extended roster: no set's chosen ability duplicates a species innate (grandfathered backlog aside)")
{
    static const struct { u16 species; u16 ability; } sKnownRedundant[] =
    {
    { SPECIES_ABSOL, ABILITY_SUPER_LUCK }, // all real abilities innate
    { SPECIES_ANNIHILAPE, ABILITY_DEFIANT }, // free real slot -> VITAL_SPIRIT
    { SPECIES_ARBOK, ABILITY_SHED_SKIN }, // all real abilities innate
    { SPECIES_ARCHALUDON, ABILITY_STAMINA }, // all real abilities innate
    { SPECIES_ARIADOS, ABILITY_SNIPER }, // all real abilities innate
    { SPECIES_BARBARACLE, ABILITY_SNIPER }, // all real abilities innate
    { SPECIES_BEEDRILL, ABILITY_SNIPER }, // all real abilities innate
    { SPECIES_BLISSEY, ABILITY_HEALER }, // all real abilities innate
    { SPECIES_BOLTUND, ABILITY_COMPETITIVE }, // all real abilities innate
    { SPECIES_BRAVIARY, ABILITY_DEFIANT }, // free real slot -> SHEER_FORCE
    { SPECIES_CAMERUPT, ABILITY_MAGMA_ARMOR }, // all real abilities innate
    { SPECIES_CHANSEY, ABILITY_HEALER }, // all real abilities innate
    { SPECIES_CLEFABLE, ABILITY_MAGIC_GUARD }, // all real abilities innate
    { SPECIES_CRABOMINABLE, ABILITY_ANGER_POINT }, // all real abilities innate
    { SPECIES_DHELMISE, ABILITY_STEELWORKER }, // all real abilities innate
    { SPECIES_DODRIO, ABILITY_TANGLED_FEET }, // free real slot -> RUN_AWAY
    { SPECIES_DONDOZO, ABILITY_WATER_VEIL }, // all real abilities innate
    { SPECIES_DRAMPA, ABILITY_BERSERK }, // free real slot -> SAP_SIPPER/CLOUD_NINE
    { SPECIES_DRAPION, ABILITY_SNIPER }, // free real slot -> BATTLE_ARMOR
    { SPECIES_DREDNAW, ABILITY_STRONG_JAW }, // all real abilities innate
    { SPECIES_DUDUNSPARCE, ABILITY_RATTLED }, // all real abilities innate
    { SPECIES_DUGTRIO_ALOLA, ABILITY_TANGLING_HAIR }, // all real abilities innate
    { SPECIES_DUSCLOPS, ABILITY_FRISK }, // all real abilities innate
    { SPECIES_DUSKNOIR, ABILITY_FRISK }, // all real abilities innate
    { SPECIES_EMBOAR, ABILITY_RECKLESS }, // all real abilities innate
    { SPECIES_EMPOLEON, ABILITY_COMPETITIVE }, // all real abilities innate
    { SPECIES_EXCADRILL, ABILITY_MOLD_BREAKER }, // all real abilities innate
    { SPECIES_FALINKS, ABILITY_DEFIANT }, // all real abilities innate
    { SPECIES_FARFETCHD, ABILITY_DEFIANT }, // all real abilities innate
    { SPECIES_FEAROW, ABILITY_SNIPER }, // all real abilities innate
    { SPECIES_FURRET, ABILITY_FRISK }, // free real slot -> RUN_AWAY
    { SPECIES_GALLADE, ABILITY_JUSTIFIED }, // all real abilities innate
    { SPECIES_GALVANTULA, ABILITY_UNNERVE }, // all real abilities innate
    { SPECIES_GLISCOR, ABILITY_POISON_HEAL }, // all real abilities innate
    { SPECIES_GOURGEIST_SUPER, ABILITY_FRISK }, // all real abilities innate
    { SPECIES_GUMSHOOS, ABILITY_STAKEOUT }, // all real abilities innate
    { SPECIES_GYARADOS, ABILITY_MOXIE }, // all real abilities innate
    { SPECIES_HERACROSS, ABILITY_MOXIE }, // all real abilities innate
    { SPECIES_HONCHKROW, ABILITY_MOXIE }, // all real abilities innate
    { SPECIES_HONCHKROW, ABILITY_SUPER_LUCK }, // all real abilities innate
    { SPECIES_HYPNO, ABILITY_INNER_FOCUS }, // all real abilities innate
    { SPECIES_ILLUMISE, ABILITY_TINTED_LENS }, // all real abilities innate
    { SPECIES_INFERNAPE, ABILITY_IRON_FIST }, // all real abilities innate
    { SPECIES_INTELEON, ABILITY_SNIPER }, // all real abilities innate
    { SPECIES_KANGASKHAN, ABILITY_INNER_FOCUS }, // all real abilities innate
    { SPECIES_KINGAMBIT, ABILITY_DEFIANT }, // all real abilities innate
    { SPECIES_LEDIAN, ABILITY_IRON_FIST }, // all real abilities innate
    { SPECIES_LUCARIO, ABILITY_JUSTIFIED }, // free real slot -> STEADFAST
    { SPECIES_LUDICOLO, ABILITY_RAIN_DISH }, // all real abilities innate
    { SPECIES_LYCANROC, ABILITY_STEADFAST }, // all real abilities innate (base forme = Lycanroc-Midday)
    { SPECIES_MEOWSTIC_F, ABILITY_COMPETITIVE }, // all real abilities innate
    { SPECIES_MEWTWO, ABILITY_UNNERVE }, // all real abilities innate
    { SPECIES_MILOTIC, ABILITY_COMPETITIVE }, // all real abilities innate
    { SPECIES_MUDSDALE, ABILITY_STAMINA }, // all real abilities innate
    { SPECIES_NOCTOWL, ABILITY_TINTED_LENS }, // all real abilities innate
    { SPECIES_NOIVERN, ABILITY_FRISK }, // all real abilities innate
    { SPECIES_OBSTAGOON, ABILITY_DEFIANT }, // all real abilities innate
    { SPECIES_PALOSSAND, ABILITY_WATER_COMPACTION }, // all real abilities innate
    { SPECIES_PASSIMIAN, ABILITY_DEFIANT }, // free real slot -> RECEIVER
    { SPECIES_PERRSERKER, ABILITY_STEELY_SPIRIT }, // all real abilities innate
    { SPECIES_PERRSERKER, ABILITY_TOUGH_CLAWS }, // all real abilities innate
    { SPECIES_PERSIAN, ABILITY_UNNERVE }, // all real abilities innate
    { SPECIES_PERSIAN_ALOLA, ABILITY_RATTLED }, // all real abilities innate
    { SPECIES_PINSIR, ABILITY_MOXIE }, // all real abilities innate
    { SPECIES_PURUGLY, ABILITY_DEFIANT }, // all real abilities innate
    { SPECIES_PYROAR, ABILITY_UNNERVE }, // free real slot -> RIVALRY
    { SPECIES_QUAQUAVAL, ABILITY_MOXIE }, // all real abilities innate
    { SPECIES_RAPIDASH_GALAR, ABILITY_ANTICIPATION }, // free real slot -> RUN_AWAY
    { SPECIES_RIBOMBEE, ABILITY_SHIELD_DUST }, // free real slot -> HONEY_GATHER
    { SPECIES_SABLEYE, ABILITY_KEEN_EYE }, // free real slot -> STALL
    { SPECIES_SAMUROTT_HISUI, ABILITY_SHARPNESS }, // all real abilities innate
    { SPECIES_SLOWBRO, ABILITY_OWN_TEMPO }, // all real abilities innate
    { SPECIES_SNORLAX, ABILITY_GLUTTONY }, // all real abilities innate
    { SPECIES_SPIDOPS, ABILITY_STAKEOUT }, // all real abilities innate
    { SPECIES_STARMIE, ABILITY_ANALYTIC }, // all real abilities innate
    { SPECIES_TAUROS_PALDEA_COMBAT, ABILITY_CUD_CHEW }, // all real abilities innate
    { SPECIES_THUNDURUS, ABILITY_DEFIANT }, // all real abilities innate (base forme = Thundurus-Incarnate)
    { SPECIES_TINKATON, ABILITY_MOLD_BREAKER }, // all real abilities innate
    { SPECIES_TOGEKISS, ABILITY_SUPER_LUCK }, // free real slot -> HUSTLE
    { SPECIES_TORNADUS, ABILITY_DEFIANT }, // all real abilities innate (base forme = Tornadus-Incarnate)
    { SPECIES_TOXAPEX, ABILITY_MERCILESS }, // all real abilities innate
    { SPECIES_TOXTRICITY, ABILITY_PUNK_ROCK }, // all real abilities innate
    { SPECIES_TOXTRICITY_LOW_KEY, ABILITY_PUNK_ROCK }, // free real slot -> MINUS
    { SPECIES_TYPHLOSION_HISUI, ABILITY_FRISK }, // all real abilities innate
    { SPECIES_UNFEZANT, ABILITY_SUPER_LUCK }, // free real slot -> RIVALRY
    { SPECIES_URSARING, ABILITY_UNNERVE }, // all real abilities innate
    { SPECIES_VENOMOTH, ABILITY_TINTED_LENS }, // all real abilities innate
    { SPECIES_VESPIQUEN, ABILITY_UNNERVE }, // all real abilities innate
    { SPECIES_WAILORD, ABILITY_WATER_VEIL }, // all real abilities innate
    { SPECIES_WATCHOG, ABILITY_ANALYTIC }, // all real abilities innate
    { SPECIES_WIGGLYTUFF, ABILITY_COMPETITIVE }, // all real abilities innate
    { SPECIES_WIGGLYTUFF, ABILITY_FRISK }, // all real abilities innate
    { SPECIES_WOBBUFFET, ABILITY_SHADOW_TAG }, // all real abilities innate
    { SPECIES_YANMEGA, ABILITY_TINTED_LENS }, // all real abilities innate
    };
    u32 i, k;
    u32 unexpected = 0;

    for (i = 0; i < gFrontierExtendedMonsCount; i++)
    {
        const struct TrainerMon *set = &gFrontierExtendedMons[i];
        enum Ability ability = set->ability;
        bool32 grandfathered = FALSE;

        // ABILITY_NONE lets the Factory pick; a chosen ability that is NOT an innate is the
        // goal (a real, observable second trait), so both are fine.
        if (ability == ABILITY_NONE)
            continue;
        if (!SpeciesHasInnate(set->species, ability))
            continue;

        for (k = 0; k < ARRAY_COUNT(sKnownRedundant); k++)
        {
            if (sKnownRedundant[k].species == set->species
             && sKnownRedundant[k].ability == ability)
            {
                grandfathered = TRUE;
                break;
            }
        }

        if (!grandfathered)
        {
            unexpected++;
            Test_MgbaPrintf("roster[%d] %S: chosen %S duplicates a species innate -- pick a non-innate ability (fork override) or add to sKnownRedundant",
                            i,
                            gSpeciesInfo[set->species].speciesName,
                            gAbilitiesInfo[ability].name);
        }
    }

    EXPECT_EQ(unexpected, 0);
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
