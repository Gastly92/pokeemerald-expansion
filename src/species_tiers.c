#include "global.h"
#include "species_tiers.h"
#include "pokemon.h"
#include "constants/pokedex.h"
#include "constants/species.h"

// FORK: species -> tier classification (see include/species_tiers.h for the full
// rationale and the fork's tier definitions). Keyed by National Pokedex number so
// every forme of a species inherits the base species' tier automatically.
//
// Derived from the upstream species flags (isRestrictedLegendary / isSubLegendary
// / isMythical / isUltraBeast / isParadox) with this fork's groupings layered on
// top: restricted legendaries + official mythicals -> TIER_MYTHICAL; sub-legends
// -> TIER_LEGENDARY; Ultra Beasts + Paradox + the Treasures of Ruin + the classic
// 600-BST pseudo-legendaries -> TIER_PSEUDO. Edit a row to reclassify a species.

struct SpeciesTierEntry
{
    u16 natDex;     // enum NationalDexOrder (NATIONAL_DEX_*)
    u8 tier;        // enum SpeciesTier
};

static const struct SpeciesTierEntry sSpeciesTiers[] =
{
    // ---- Mythical — 1 per frontier brain battle ----
    { NATIONAL_DEX_MEWTWO, TIER_MYTHICAL }, // 150
    { NATIONAL_DEX_LUGIA, TIER_MYTHICAL }, // 249
    { NATIONAL_DEX_HO_OH, TIER_MYTHICAL }, // 250
    { NATIONAL_DEX_KYOGRE, TIER_MYTHICAL }, // 382
    { NATIONAL_DEX_GROUDON, TIER_MYTHICAL }, // 383
    { NATIONAL_DEX_RAYQUAZA, TIER_MYTHICAL }, // 384
    { NATIONAL_DEX_DEOXYS, TIER_MYTHICAL }, // 386
    { NATIONAL_DEX_DIALGA, TIER_MYTHICAL }, // 483
    { NATIONAL_DEX_PALKIA, TIER_MYTHICAL }, // 484
    { NATIONAL_DEX_GIRATINA, TIER_MYTHICAL }, // 487
    { NATIONAL_DEX_DARKRAI, TIER_MYTHICAL }, // 491
    { NATIONAL_DEX_ARCEUS, TIER_MYTHICAL }, // 493
    { NATIONAL_DEX_RESHIRAM, TIER_MYTHICAL }, // 643
    { NATIONAL_DEX_ZEKROM, TIER_MYTHICAL }, // 644
    { NATIONAL_DEX_XERNEAS, TIER_MYTHICAL }, // 716
    { NATIONAL_DEX_YVELTAL, TIER_MYTHICAL }, // 717
    { NATIONAL_DEX_ZYGARDE, TIER_MYTHICAL }, // 718
    { NATIONAL_DEX_SOLGALEO, TIER_MYTHICAL }, // 791
    { NATIONAL_DEX_LUNALA, TIER_MYTHICAL }, // 792
    { NATIONAL_DEX_NECROZMA, TIER_MYTHICAL }, // 800
    { NATIONAL_DEX_ZACIAN, TIER_MYTHICAL }, // 888
    { NATIONAL_DEX_ZAMAZENTA, TIER_MYTHICAL }, // 889
    { NATIONAL_DEX_ETERNATUS, TIER_MYTHICAL }, // 890
    { NATIONAL_DEX_CALYREX, TIER_MYTHICAL }, // 898
    { NATIONAL_DEX_KORAIDON, TIER_MYTHICAL }, // 1007
    { NATIONAL_DEX_MIRAIDON, TIER_MYTHICAL }, // 1008

    // ---- Legendary — 1 per frontier boss battle ----
    { NATIONAL_DEX_LATIAS, TIER_LEGENDARY }, // 380
    { NATIONAL_DEX_LATIOS, TIER_LEGENDARY }, // 381
    { NATIONAL_DEX_JIRACHI, TIER_LEGENDARY }, // 385
    { NATIONAL_DEX_TORNADUS, TIER_LEGENDARY }, // 641
    { NATIONAL_DEX_THUNDURUS, TIER_LEGENDARY }, // 642
    { NATIONAL_DEX_LANDORUS, TIER_LEGENDARY }, // 645
    { NATIONAL_DEX_KYUREM, TIER_LEGENDARY }, // 646
    { NATIONAL_DEX_KELDEO, TIER_LEGENDARY }, // 647
    { NATIONAL_DEX_MELOETTA, TIER_LEGENDARY }, // 648
    { NATIONAL_DEX_GENESECT, TIER_LEGENDARY }, // 649
    { NATIONAL_DEX_DIANCIE, TIER_LEGENDARY }, // 719
    { NATIONAL_DEX_HOOPA, TIER_LEGENDARY }, // 720
    { NATIONAL_DEX_VOLCANION, TIER_LEGENDARY }, // 721
    { NATIONAL_DEX_MAGEARNA, TIER_LEGENDARY }, // 801
    { NATIONAL_DEX_MARSHADOW, TIER_LEGENDARY }, // 802
    { NATIONAL_DEX_URSHIFU, TIER_LEGENDARY }, // 892
    { NATIONAL_DEX_GLASTRIER, TIER_LEGENDARY }, // 896
    { NATIONAL_DEX_SPECTRIER, TIER_LEGENDARY }, // 897
    { NATIONAL_DEX_OGERPON, TIER_LEGENDARY }, // 1017

    // ---- Pseudo — At most 1 per factory rental team ----
    { NATIONAL_DEX_SALAMENCE, TIER_PSEUDO }, // 373
    { NATIONAL_DEX_GARCHOMP, TIER_PSEUDO }, // 445
    { NATIONAL_DEX_HEATRAN, TIER_PSEUDO }, // 485
    { NATIONAL_DEX_PHEROMOSA, TIER_PSEUDO }, // 795
    { NATIONAL_DEX_KARTANA, TIER_PSEUDO }, // 798
    { NATIONAL_DEX_NAGANADEL, TIER_PSEUDO }, // 804
    { NATIONAL_DEX_ZERAORA, TIER_PSEUDO }, // 807
    { NATIONAL_DEX_BLACEPHALON, TIER_PSEUDO }, // 806
    { NATIONAL_DEX_DRAGAPULT, TIER_PSEUDO }, // 887
    { NATIONAL_DEX_GREAT_TUSK, TIER_PSEUDO }, // 984
    { NATIONAL_DEX_IRON_BUNDLE, TIER_PSEUDO }, // 991
    { NATIONAL_DEX_IRON_JUGULIS, TIER_PSEUDO }, // 993
    { NATIONAL_DEX_BAXCALIBUR, TIER_PSEUDO }, // 998
    { NATIONAL_DEX_WO_CHIEN, TIER_PSEUDO }, // 1001
    { NATIONAL_DEX_CHIEN_PAO, TIER_PSEUDO }, // 1002
    { NATIONAL_DEX_TING_LU, TIER_PSEUDO }, // 1003
    { NATIONAL_DEX_CHI_YU, TIER_PSEUDO }, // 1004
    { NATIONAL_DEX_ROARING_MOON, TIER_PSEUDO }, // 1005
    { NATIONAL_DEX_IRON_VALIANT, TIER_PSEUDO }, // 1006
    { NATIONAL_DEX_WALKING_WAKE, TIER_PSEUDO }, // 1009
    { NATIONAL_DEX_GOUGING_FIRE, TIER_PSEUDO }, // 1020
    { NATIONAL_DEX_RAGING_BOLT, TIER_PSEUDO }, // 1021
    { NATIONAL_DEX_TERAPAGOS, TIER_PSEUDO }, // 1024
    { NATIONAL_DEX_PECHARUNT, TIER_PSEUDO }, // 1025
};

enum SpeciesTier GetSpeciesTier(u16 species)
{
    enum NationalDexOrder natDex = SpeciesToNationalPokedexNum(species);
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sSpeciesTiers); i++)
    {
        if (sSpeciesTiers[i].natDex == natDex)
            return sSpeciesTiers[i].tier;
    }

    return TIER_NORMAL;
}

bool32 SpeciesIsTier(u16 species, enum SpeciesTier tier)
{
    return GetSpeciesTier(species) == tier;
}
