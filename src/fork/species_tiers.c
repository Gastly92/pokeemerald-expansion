#include "global.h"
#include "fork/species_tiers.h"
#include "constants/species.h"

// FORK: species -> tier classification (see include/species_tiers.h for the full
// rationale and the fork's tier definitions). Keyed by EXACT species id so each
// forme is classified on its own merits: a powerful forme can sit above its base
// (Shaymin-Sky is TIER_LEGENDARY while ordinary Shaymin is TIER_PSEUDO and a weak
// base can sit below its formes (base Calyrex is TIER_NORMAL / absent while its
// Ice/Shadow riders are TIER_MYTHICAL).
// Anything not listed is TIER_NORMAL. Add a row to classify another species/forme.
// Each tier is its own array — membership in the array is the classification, so
// rows only need the species (no per-row TIER_* token to keep in sync).

// Mythical: exactly 1 per frontier brain battle
static const u16 sMythicalSpecies[] =
{
    SPECIES_MEWTWO,               // 0150
    SPECIES_LUGIA,                // 0249
    SPECIES_HO_OH,                // 0250
    SPECIES_KYOGRE,               // 0382
    SPECIES_GROUDON,              // 0383
    SPECIES_RAYQUAZA,             // 0384
    SPECIES_DEOXYS_ATTACK,        // 0386
    SPECIES_DEOXYS_SPEED,         // 0386
    SPECIES_DIALGA,               // 0483
    SPECIES_DIALGA_ORIGIN,        // 0483
    SPECIES_PALKIA,               // 0484
    SPECIES_PALKIA_ORIGIN,        // 0484
    SPECIES_GIRATINA,             // 0487
    SPECIES_GIRATINA_ORIGIN,      // 0487
    SPECIES_DARKRAI,              // 0491
    SPECIES_ARCEUS,               // 0493
    SPECIES_RESHIRAM,             // 0643
    SPECIES_ZEKROM,               // 0644
    SPECIES_XERNEAS,              // 0716
    SPECIES_YVELTAL,              // 0717
    SPECIES_ZYGARDE,              // 0718
    SPECIES_SOLGALEO,             // 0791
    SPECIES_LUNALA,               // 0792
    SPECIES_NECROZMA_DAWN_WINGS,  // 0800
    SPECIES_NECROZMA_DUSK_MANE,   // 0800
    SPECIES_ZACIAN,               // 0888
    SPECIES_ZACIAN_CROWNED,       // 0888
    SPECIES_ZAMAZENTA,            // 0889
    SPECIES_ZAMAZENTA_CROWNED,    // 0889
    SPECIES_ETERNATUS,            // 0890
    SPECIES_CALYREX_ICE,          // 0898
    SPECIES_CALYREX_SHADOW,       // 0898
    SPECIES_KORAIDON,             // 1007
    SPECIES_MIRAIDON,             // 1008
};

// Legendary: exactly 1 per frontier boss battle
static const u16 sLegendarySpecies[] =
{
    SPECIES_LATIAS,                // 0380
    SPECIES_LATIOS,                // 0381
    SPECIES_JIRACHI,               // 0385
    SPECIES_SHAYMIN_SKY,           // 0492
    SPECIES_KYUREM_BLACK,          // 0646
    SPECIES_KYUREM_WHITE,          // 0646
    SPECIES_KELDEO,                // 0647
    SPECIES_MELOETTA,              // 0648
    SPECIES_GENESECT,              // 0649
    SPECIES_HOOPA_UNBOUND,         // 0720
    SPECIES_URSHIFU,               // 0892
    SPECIES_URSHIFU_RAPID_STRIKE,  // 0892
    SPECIES_GLASTRIER,             // 0896
    SPECIES_SPECTRIER,             // 0897
    SPECIES_WALKING_WAKE,          // 1009
    SPECIES_OGERPON_CORNERSTONE,   // 1017
    SPECIES_OGERPON_HEARTHFLAME,   // 1017
    SPECIES_OGERPON_TEAL,          // 1017
    SPECIES_OGERPON_WELLSPRING,    // 1017
};

// Pseudo: at most 1 per frontier draft team
static const u16 sPseudoSpecies[] =
{
    SPECIES_ARTICUNO_GALAR,      // 0144
    SPECIES_ZAPDOS_GALAR,        // 0145
    SPECIES_MOLTRES_GALAR,       // 0146
    SPECIES_DRAGONITE,           // 0149
    SPECIES_TYRANITAR,           // 0248
    SPECIES_SALAMENCE,           // 0373
    SPECIES_GARCHOMP,            // 0445
    SPECIES_HEATRAN,             // 0485
    SPECIES_MANAPHY,             // 0490
    SPECIES_SHAYMIN,             // 0492
    SPECIES_VICTINI,             // 0494
    SPECIES_HYDREIGON,           // 0635
    SPECIES_TERRAKION,           // 0639
    SPECIES_TORNADUS,            // 0641
    SPECIES_TORNADUS_THERIAN,    // 0641
    SPECIES_THUNDURUS,           // 0642
    SPECIES_THUNDURUS_THERIAN,   // 0642
    SPECIES_LANDORUS,            // 0645
    SPECIES_LANDORUS_THERIAN,    // 0645
    SPECIES_KOMMO_O,             // 0784
    SPECIES_TAPU_KOKO,           // 0785
    SPECIES_TAPU_LELE,           // 0786
    SPECIES_TAPU_BULU,           // 0787
    SPECIES_TAPU_FINI,           // 0788
    SPECIES_PHEROMOSA,           // 0795
    SPECIES_KARTANA,             // 0798
    SPECIES_NAGANADEL,           // 0804
    SPECIES_BLACEPHALON,         // 0806
    SPECIES_ZERAORA,             // 0807
    SPECIES_DRAGAPULT,           // 0887
    SPECIES_GREAT_TUSK,          // 0984
    SPECIES_FLUTTER_MANE,        // 0987
    SPECIES_SLITHER_WING,        // 0988
    SPECIES_SANDY_SHOCKS,        // 0989
    SPECIES_IRON_TREADS,         // 0990
    SPECIES_IRON_BUNDLE,         // 0991
    SPECIES_IRON_HANDS,          // 0992
    SPECIES_IRON_JUGULIS,        // 0993
    SPECIES_IRON_MOTH,           // 0994
    SPECIES_IRON_THORNS,         // 0995
    SPECIES_BAXCALIBUR,          // 0998
    SPECIES_CHIEN_PAO,           // 1002
    SPECIES_CHI_YU,              // 1004
    SPECIES_ROARING_MOON,        // 1005
    SPECIES_IRON_VALIANT,        // 1006
    SPECIES_GOUGING_FIRE,        // 1020
    SPECIES_RAGING_BOLT,         // 1021
    SPECIES_IRON_BOULDER,        // 1022
    SPECIES_TERAPAGOS_TERASTAL,  // 1024
};

static bool32 SpeciesInList(u16 species, const u16 *list, u32 count)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        if (list[i] == species)
            return TRUE;
    }

    return FALSE;
}

enum SpeciesTier GetSpeciesTier(u16 species)
{
    if (SpeciesInList(species, sMythicalSpecies, ARRAY_COUNT(sMythicalSpecies)))
        return TIER_MYTHICAL;
    if (SpeciesInList(species, sLegendarySpecies, ARRAY_COUNT(sLegendarySpecies)))
        return TIER_LEGENDARY;
    if (SpeciesInList(species, sPseudoSpecies, ARRAY_COUNT(sPseudoSpecies)))
        return TIER_PSEUDO;

    return TIER_NORMAL;
}

bool32 SpeciesIsTier(u16 species, enum SpeciesTier tier)
{
    return GetSpeciesTier(species) == tier;
}

#if TESTING
bool32 SpeciesTierListsOverlap(u16 *outSpecies)
{
    static const struct { const u16 *list; u32 count; } sAllTierLists[] =
    {
        { sMythicalSpecies,  ARRAY_COUNT(sMythicalSpecies) },
        { sLegendarySpecies, ARRAY_COUNT(sLegendarySpecies) },
        { sPseudoSpecies,    ARRAY_COUNT(sPseudoSpecies) },
    };
    u32 listA, listB, i, j;

    for (listA = 0; listA < ARRAY_COUNT(sAllTierLists); listA++)
    {
        for (i = 0; i < sAllTierLists[listA].count; i++)
        {
            // Duplicates within the same list.
            for (j = i + 1; j < sAllTierLists[listA].count; j++)
            {
                if (sAllTierLists[listA].list[i] == sAllTierLists[listA].list[j])
                {
                    *outSpecies = sAllTierLists[listA].list[i];
                    return TRUE;
                }
            }

            // Duplicates across other lists.
            for (listB = listA + 1; listB < ARRAY_COUNT(sAllTierLists); listB++)
            {
                for (j = 0; j < sAllTierLists[listB].count; j++)
                {
                    if (sAllTierLists[listA].list[i] == sAllTierLists[listB].list[j])
                    {
                        *outSpecies = sAllTierLists[listA].list[i];
                        return TRUE;
                    }
                }
            }
        }
    }

    return FALSE;
}
#endif // TESTING
