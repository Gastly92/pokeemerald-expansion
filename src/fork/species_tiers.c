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

struct SpeciesTierEntry
{
    u16 species;    // exact SPECIES_* id (forme-specific)
    u8 tier;        // enum SpeciesTier
};

static const struct SpeciesTierEntry sSpeciesTiers[] =
{
    // Nat Dex # in trailing comments (formes share their base's number); keep rows sorted by it.
    // ---- Mythical — exactly 1 per frontier brain battle ----
    { SPECIES_MEWTWO,              TIER_MYTHICAL }, // 150
    { SPECIES_LUGIA,               TIER_MYTHICAL }, // 249
    { SPECIES_HO_OH,               TIER_MYTHICAL }, // 250
    { SPECIES_KYOGRE,              TIER_MYTHICAL }, // 382
    { SPECIES_GROUDON,             TIER_MYTHICAL }, // 383
    { SPECIES_RAYQUAZA,            TIER_MYTHICAL }, // 384
    { SPECIES_DEOXYS_ATTACK,       TIER_MYTHICAL }, // 386
    { SPECIES_DEOXYS_SPEED,        TIER_MYTHICAL }, // 386
    { SPECIES_DIALGA,              TIER_MYTHICAL }, // 483
    { SPECIES_DIALGA_ORIGIN,       TIER_MYTHICAL }, // 483
    { SPECIES_PALKIA,              TIER_MYTHICAL }, // 484
    { SPECIES_PALKIA_ORIGIN,       TIER_MYTHICAL }, // 484
    { SPECIES_GIRATINA,            TIER_MYTHICAL }, // 487
    { SPECIES_GIRATINA_ORIGIN,     TIER_MYTHICAL }, // 487
    { SPECIES_DARKRAI,             TIER_MYTHICAL }, // 491
    { SPECIES_ARCEUS,              TIER_MYTHICAL }, // 493
    { SPECIES_RESHIRAM,            TIER_MYTHICAL }, // 643
    { SPECIES_ZEKROM,              TIER_MYTHICAL }, // 644
    { SPECIES_XERNEAS,             TIER_MYTHICAL }, // 716
    { SPECIES_YVELTAL,             TIER_MYTHICAL }, // 717
    { SPECIES_ZYGARDE,             TIER_MYTHICAL }, // 718
    { SPECIES_SOLGALEO,            TIER_MYTHICAL }, // 791
    { SPECIES_LUNALA,              TIER_MYTHICAL }, // 792
    { SPECIES_NECROZMA_DAWN_WINGS, TIER_MYTHICAL }, // 800
    { SPECIES_NECROZMA_DUSK_MANE,  TIER_MYTHICAL }, // 800
    { SPECIES_ZACIAN,              TIER_MYTHICAL }, // 888
    { SPECIES_ZACIAN_CROWNED,      TIER_MYTHICAL }, // 888
    { SPECIES_ZAMAZENTA,           TIER_MYTHICAL }, // 889
    { SPECIES_ZAMAZENTA_CROWNED,   TIER_MYTHICAL }, // 889
    { SPECIES_ETERNATUS,           TIER_MYTHICAL }, // 890
    { SPECIES_CALYREX_ICE,         TIER_MYTHICAL }, // 898
    { SPECIES_CALYREX_SHADOW,      TIER_MYTHICAL }, // 898
    { SPECIES_KORAIDON,            TIER_MYTHICAL }, // 1007
    { SPECIES_MIRAIDON,            TIER_MYTHICAL }, // 1008

    // ---- Legendary — exactly 1 per frontier boss battle ----
    { SPECIES_LATIAS,               TIER_LEGENDARY }, // 380
    { SPECIES_LATIOS,               TIER_LEGENDARY }, // 381
    { SPECIES_JIRACHI,              TIER_LEGENDARY }, // 385
    { SPECIES_SHAYMIN_SKY,          TIER_LEGENDARY }, // 492
    { SPECIES_KYUREM,               TIER_LEGENDARY }, // 646
    { SPECIES_KYUREM_BLACK,         TIER_LEGENDARY }, // 646
    { SPECIES_KYUREM_WHITE,         TIER_LEGENDARY }, // 646
    { SPECIES_KELDEO,               TIER_LEGENDARY }, // 647
    { SPECIES_MELOETTA,             TIER_LEGENDARY }, // 648
    { SPECIES_GENESECT,             TIER_LEGENDARY }, // 649
    { SPECIES_HOOPA_UNBOUND,        TIER_LEGENDARY }, // 720
    { SPECIES_URSHIFU,              TIER_LEGENDARY }, // 892
    { SPECIES_URSHIFU_RAPID_STRIKE, TIER_LEGENDARY }, // 892
    { SPECIES_GLASTRIER,            TIER_LEGENDARY }, // 896
    { SPECIES_SPECTRIER,            TIER_LEGENDARY }, // 897
    { SPECIES_WALKING_WAKE,         TIER_LEGENDARY }, // 1009
    { SPECIES_OGERPON,              TIER_LEGENDARY }, // 1017
    { SPECIES_OGERPON_CORNERSTONE,  TIER_LEGENDARY }, // 1017
    { SPECIES_OGERPON_HEARTHFLAME,  TIER_LEGENDARY }, // 1017
    { SPECIES_OGERPON_TEAL,         TIER_LEGENDARY }, // 1017
    { SPECIES_OGERPON_WELLSPRING,   TIER_LEGENDARY }, // 1017

    // ---- Pseudo — At most 1 per frontier draft team ----
    { SPECIES_ARTICUNO_GALAR,     TIER_PSEUDO }, // 144
    { SPECIES_ZAPDOS_GALAR,       TIER_PSEUDO }, // 145
    { SPECIES_MOLTRES_GALAR,      TIER_PSEUDO }, // 146
    { SPECIES_DRAGONITE,          TIER_PSEUDO }, // 149
    { SPECIES_TYRANITAR,          TIER_PSEUDO }, // 248
    { SPECIES_SALAMENCE,          TIER_PSEUDO }, // 373
    { SPECIES_GARCHOMP,           TIER_PSEUDO }, // 445
    { SPECIES_HEATRAN,            TIER_PSEUDO }, // 485
    { SPECIES_MANAPHY,            TIER_PSEUDO }, // 490
    { SPECIES_SHAYMIN,            TIER_PSEUDO }, // 492
    { SPECIES_VICTINI,            TIER_PSEUDO }, // 494
    { SPECIES_HYDREIGON,          TIER_PSEUDO }, // 635
    { SPECIES_TERRAKION,          TIER_PSEUDO }, // 639
    { SPECIES_TORNADUS,           TIER_PSEUDO }, // 641
    { SPECIES_TORNADUS_THERIAN,   TIER_PSEUDO }, // 641
    { SPECIES_THUNDURUS,          TIER_PSEUDO }, // 642
    { SPECIES_THUNDURUS_THERIAN,  TIER_PSEUDO }, // 642
    { SPECIES_LANDORUS,           TIER_PSEUDO }, // 645
    { SPECIES_LANDORUS_THERIAN,   TIER_PSEUDO }, // 645
    { SPECIES_KOMMO_O,            TIER_PSEUDO }, // 784
    { SPECIES_TAPU_KOKO,          TIER_PSEUDO }, // 785
    { SPECIES_TAPU_LELE,          TIER_PSEUDO }, // 786
    { SPECIES_TAPU_BULU,          TIER_PSEUDO }, // 787
    { SPECIES_TAPU_FINI,          TIER_PSEUDO }, // 788
    { SPECIES_PHEROMOSA,          TIER_PSEUDO }, // 795
    { SPECIES_KARTANA,            TIER_PSEUDO }, // 798
    { SPECIES_NAGANADEL,          TIER_PSEUDO }, // 804
    { SPECIES_BLACEPHALON,        TIER_PSEUDO }, // 806
    { SPECIES_ZERAORA,            TIER_PSEUDO }, // 807
    { SPECIES_DRAGAPULT,          TIER_PSEUDO }, // 887
    { SPECIES_GREAT_TUSK,         TIER_PSEUDO }, // 984
    { SPECIES_FLUTTER_MANE,       TIER_PSEUDO }, // 987
    { SPECIES_SLITHER_WING,       TIER_PSEUDO }, // 988
    { SPECIES_SANDY_SHOCKS,       TIER_PSEUDO }, // 989
    { SPECIES_IRON_TREADS,        TIER_PSEUDO }, // 990
    { SPECIES_IRON_BUNDLE,        TIER_PSEUDO }, // 991
    { SPECIES_IRON_HANDS,         TIER_PSEUDO }, // 992
    { SPECIES_IRON_JUGULIS,       TIER_PSEUDO }, // 993
    { SPECIES_IRON_MOTH,          TIER_PSEUDO }, // 994
    { SPECIES_IRON_THORNS,        TIER_PSEUDO }, // 995
    { SPECIES_BAXCALIBUR,         TIER_PSEUDO }, // 998
    { SPECIES_CHIEN_PAO,          TIER_PSEUDO }, // 1002
    { SPECIES_CHI_YU,             TIER_PSEUDO }, // 1004
    { SPECIES_ROARING_MOON,       TIER_PSEUDO }, // 1005
    { SPECIES_IRON_VALIANT,       TIER_PSEUDO }, // 1006
    { SPECIES_GOUGING_FIRE,       TIER_PSEUDO }, // 1020
    { SPECIES_RAGING_BOLT,        TIER_PSEUDO }, // 1021
    { SPECIES_IRON_BOULDER,       TIER_PSEUDO }, // 1022
    { SPECIES_TERAPAGOS_TERASTAL, TIER_PSEUDO }, // 1024
};

enum SpeciesTier GetSpeciesTier(u16 species)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sSpeciesTiers); i++)
    {
        if (sSpeciesTiers[i].species == species)
            return sSpeciesTiers[i].tier;
    }

    return TIER_NORMAL;
}

bool32 SpeciesIsTier(u16 species, enum SpeciesTier tier)
{
    return GetSpeciesTier(species) == tier;
}
