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

// ---- Mythical — exactly 1 per frontier brain battle ----
static const u16 sMythicalSpecies[] =
{
    // Nat Dex # in trailing comments (formes share their base's number); keep rows sorted by it.
    SPECIES_MEWTWO,              // 150
    SPECIES_LUGIA,                // 249
    SPECIES_HO_OH,                // 250
    SPECIES_KYOGRE,               // 382
    SPECIES_GROUDON,              // 383
    SPECIES_RAYQUAZA,             // 384
    SPECIES_DEOXYS_ATTACK,        // 386
    SPECIES_DEOXYS_SPEED,         // 386
    SPECIES_DIALGA,               // 483
    SPECIES_DIALGA_ORIGIN,        // 483
    SPECIES_PALKIA,               // 484
    SPECIES_PALKIA_ORIGIN,        // 484
    SPECIES_GIRATINA,             // 487
    SPECIES_GIRATINA_ORIGIN,      // 487
    SPECIES_DARKRAI,              // 491
    SPECIES_ARCEUS,               // 493
    SPECIES_RESHIRAM,             // 643
    SPECIES_ZEKROM,               // 644
    SPECIES_XERNEAS,              // 716
    SPECIES_YVELTAL,              // 717
    SPECIES_ZYGARDE,              // 718
    SPECIES_SOLGALEO,             // 791
    SPECIES_LUNALA,               // 792
    SPECIES_NECROZMA_DAWN_WINGS,  // 800
    SPECIES_NECROZMA_DUSK_MANE,   // 800
    SPECIES_ZACIAN,               // 888
    SPECIES_ZACIAN_CROWNED,       // 888
    SPECIES_ZAMAZENTA,            // 889
    SPECIES_ZAMAZENTA_CROWNED,    // 889
    SPECIES_ETERNATUS,            // 890
    SPECIES_CALYREX_ICE,          // 898
    SPECIES_CALYREX_SHADOW,       // 898
    SPECIES_KORAIDON,             // 1007
    SPECIES_MIRAIDON,             // 1008
};

// ---- Legendary — exactly 1 per frontier boss battle ----
static const u16 sLegendarySpecies[] =
{
    SPECIES_LATIAS,               // 380
    SPECIES_LATIOS,                // 381
    SPECIES_JIRACHI,               // 385
    SPECIES_SHAYMIN_SKY,           // 492
    SPECIES_KYUREM,                // 646
    SPECIES_KYUREM_BLACK,          // 646
    SPECIES_KYUREM_WHITE,          // 646
    SPECIES_KELDEO,                // 647
    SPECIES_MELOETTA,              // 648
    SPECIES_GENESECT,              // 649
    SPECIES_HOOPA_UNBOUND,         // 720
    SPECIES_URSHIFU,               // 892
    SPECIES_URSHIFU_RAPID_STRIKE,  // 892
    SPECIES_GLASTRIER,             // 896
    SPECIES_SPECTRIER,             // 897
    SPECIES_WALKING_WAKE,          // 1009
    SPECIES_OGERPON_CORNERSTONE,   // 1017
    SPECIES_OGERPON_HEARTHFLAME,   // 1017
    SPECIES_OGERPON_TEAL,          // 1017
    SPECIES_OGERPON_WELLSPRING,    // 1017
};

// ---- Pseudo — at most 1 per frontier draft team ----
static const u16 sPseudoSpecies[] =
{
    SPECIES_ARTICUNO_GALAR,       // 144
    SPECIES_ZAPDOS_GALAR,         // 145
    SPECIES_MOLTRES_GALAR,        // 146
    SPECIES_DRAGONITE,            // 149
    SPECIES_TYRANITAR,            // 248
    SPECIES_SALAMENCE,            // 373
    SPECIES_GARCHOMP,             // 445
    SPECIES_HEATRAN,              // 485
    SPECIES_MANAPHY,              // 490
    SPECIES_SHAYMIN,              // 492
    SPECIES_VICTINI,              // 494
    SPECIES_HYDREIGON,            // 635
    SPECIES_TERRAKION,            // 639
    SPECIES_TORNADUS,             // 641
    SPECIES_TORNADUS_THERIAN,     // 641
    SPECIES_THUNDURUS,            // 642
    SPECIES_THUNDURUS_THERIAN,    // 642
    SPECIES_LANDORUS,             // 645
    SPECIES_LANDORUS_THERIAN,     // 645
    SPECIES_KOMMO_O,              // 784
    SPECIES_TAPU_KOKO,            // 785
    SPECIES_TAPU_LELE,            // 786
    SPECIES_TAPU_BULU,            // 787
    SPECIES_TAPU_FINI,            // 788
    SPECIES_PHEROMOSA,            // 795
    SPECIES_KARTANA,              // 798
    SPECIES_NAGANADEL,            // 804
    SPECIES_BLACEPHALON,          // 806
    SPECIES_ZERAORA,              // 807
    SPECIES_DRAGAPULT,            // 887
    SPECIES_GREAT_TUSK,           // 984
    SPECIES_FLUTTER_MANE,         // 987
    SPECIES_SLITHER_WING,         // 988
    SPECIES_SANDY_SHOCKS,         // 989
    SPECIES_IRON_TREADS,          // 990
    SPECIES_IRON_BUNDLE,          // 991
    SPECIES_IRON_HANDS,           // 992
    SPECIES_IRON_JUGULIS,         // 993
    SPECIES_IRON_MOTH,            // 994
    SPECIES_IRON_THORNS,          // 995
    SPECIES_BAXCALIBUR,           // 998
    SPECIES_CHIEN_PAO,            // 1002
    SPECIES_CHI_YU,               // 1004
    SPECIES_ROARING_MOON,         // 1005
    SPECIES_IRON_VALIANT,         // 1006
    SPECIES_GOUGING_FIRE,         // 1020
    SPECIES_RAGING_BOLT,          // 1021
    SPECIES_IRON_BOULDER,         // 1022
    SPECIES_TERAPAGOS_TERASTAL,   // 1024
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
