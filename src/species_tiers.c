#include "global.h"
#include "species_tiers.h"
#include "constants/species.h"

// FORK: species -> tier classification (see include/species_tiers.h for the full
// rationale and the fork's tier definitions). Keyed by EXACT species id so each
// forme is classified on its own merits: a powerful forme can sit above its base
// (Shaymin-Sky is TIER_LEGENDARY while ordinary Shaymin is TIER_NORMAL and is
// simply absent from the table) and a weak base can sit below its formes (base
// Calyrex is TIER_NORMAL / absent while its Ice/Shadow riders are TIER_MYTHICAL).
// Anything not listed is TIER_NORMAL. Add a row to classify another species/forme.

struct SpeciesTierEntry
{
    u16 species;    // exact SPECIES_* id (forme-specific)
    u8 tier;        // enum SpeciesTier
};

static const struct SpeciesTierEntry sSpeciesTiers[] =
{
    // ---- Mythical — exactly 1 per frontier brain battle ----
    { SPECIES_MEWTWO,              TIER_MYTHICAL },
    { SPECIES_LUGIA,               TIER_MYTHICAL },
    { SPECIES_HO_OH,               TIER_MYTHICAL },
    { SPECIES_KYOGRE,              TIER_MYTHICAL },
    { SPECIES_GROUDON,             TIER_MYTHICAL },
    { SPECIES_RAYQUAZA,            TIER_MYTHICAL },
    { SPECIES_DEOXYS_ATTACK,       TIER_MYTHICAL },
    { SPECIES_DEOXYS_DEFENSE,      TIER_MYTHICAL },
    { SPECIES_DEOXYS_SPEED,        TIER_MYTHICAL },
    { SPECIES_DIALGA,              TIER_MYTHICAL },
    { SPECIES_DIALGA_ORIGIN,       TIER_MYTHICAL },
    { SPECIES_PALKIA,              TIER_MYTHICAL },
    { SPECIES_PALKIA_ORIGIN,       TIER_MYTHICAL },
    { SPECIES_GIRATINA,            TIER_MYTHICAL },
    { SPECIES_GIRATINA_ORIGIN,     TIER_MYTHICAL },
    { SPECIES_DARKRAI,             TIER_MYTHICAL },
    { SPECIES_ARCEUS,              TIER_MYTHICAL },
    { SPECIES_RESHIRAM,            TIER_MYTHICAL },
    { SPECIES_ZEKROM,              TIER_MYTHICAL },
    { SPECIES_XERNEAS,             TIER_MYTHICAL },
    { SPECIES_YVELTAL,             TIER_MYTHICAL },
    { SPECIES_ZYGARDE,             TIER_MYTHICAL },
    { SPECIES_SOLGALEO,            TIER_MYTHICAL },
    { SPECIES_LUNALA,              TIER_MYTHICAL },
    { SPECIES_NECROZMA,            TIER_MYTHICAL },
    { SPECIES_NECROZMA_DAWN_WINGS, TIER_MYTHICAL },
    { SPECIES_NECROZMA_DUSK_MANE,  TIER_MYTHICAL },
    { SPECIES_ZACIAN,              TIER_MYTHICAL },
    { SPECIES_ZACIAN_CROWNED,      TIER_MYTHICAL },
    { SPECIES_ZAMAZENTA,           TIER_MYTHICAL },
    { SPECIES_ZAMAZENTA_CROWNED,   TIER_MYTHICAL },
    { SPECIES_ETERNATUS,           TIER_MYTHICAL },
    { SPECIES_CALYREX_ICE,         TIER_MYTHICAL },
    { SPECIES_CALYREX_SHADOW,      TIER_MYTHICAL },
    { SPECIES_KORAIDON,            TIER_MYTHICAL },
    { SPECIES_MIRAIDON,            TIER_MYTHICAL },

    // ---- Legendary — exactly 1 per frontier boss battle ----
    { SPECIES_LATIAS,               TIER_LEGENDARY },
    { SPECIES_LATIOS,               TIER_LEGENDARY },
    { SPECIES_JIRACHI,              TIER_LEGENDARY },
    { SPECIES_SHAYMIN_SKY,          TIER_LEGENDARY },
    { SPECIES_TORNADUS,             TIER_LEGENDARY },
    { SPECIES_TORNADUS_THERIAN,     TIER_LEGENDARY },
    { SPECIES_THUNDURUS,            TIER_LEGENDARY },
    { SPECIES_THUNDURUS_THERIAN,    TIER_LEGENDARY },
    { SPECIES_LANDORUS,             TIER_LEGENDARY },
    { SPECIES_LANDORUS_THERIAN,     TIER_LEGENDARY },
    { SPECIES_KYUREM,               TIER_LEGENDARY },
    { SPECIES_KYUREM_BLACK,         TIER_LEGENDARY },
    { SPECIES_KYUREM_WHITE,         TIER_LEGENDARY },
    { SPECIES_KELDEO,               TIER_LEGENDARY },
    { SPECIES_MELOETTA,             TIER_LEGENDARY },
    { SPECIES_GENESECT,             TIER_LEGENDARY },
    { SPECIES_DIANCIE,              TIER_LEGENDARY },
    { SPECIES_HOOPA,                TIER_LEGENDARY },
    { SPECIES_HOOPA_UNBOUND,        TIER_LEGENDARY },
    { SPECIES_MAGEARNA,             TIER_LEGENDARY },
    { SPECIES_MARSHADOW,            TIER_LEGENDARY },
    { SPECIES_URSHIFU,              TIER_LEGENDARY },
    { SPECIES_URSHIFU_RAPID_STRIKE, TIER_LEGENDARY },
    { SPECIES_GLASTRIER,            TIER_LEGENDARY },
    { SPECIES_SPECTRIER,            TIER_LEGENDARY },
    { SPECIES_WALKING_WAKE,         TIER_LEGENDARY },
    { SPECIES_OGERPON,              TIER_LEGENDARY },
    { SPECIES_OGERPON_CORNERSTONE,  TIER_LEGENDARY },
    { SPECIES_OGERPON_HEARTHFLAME,  TIER_LEGENDARY },
    { SPECIES_OGERPON_TEAL,         TIER_LEGENDARY },
    { SPECIES_OGERPON_WELLSPRING,   TIER_LEGENDARY },

    // ---- Pseudo — At most 1 per frontier draft team ----
    { SPECIES_SALAMENCE,          TIER_PSEUDO },
    { SPECIES_GARCHOMP,           TIER_PSEUDO },
    { SPECIES_HEATRAN,            TIER_PSEUDO },
    { SPECIES_KOMMO_O,            TIER_PSEUDO },
    { SPECIES_PHEROMOSA,          TIER_PSEUDO },
    { SPECIES_KARTANA,            TIER_PSEUDO },
    { SPECIES_NAGANADEL,          TIER_PSEUDO },
    { SPECIES_BLACEPHALON,        TIER_PSEUDO },
    { SPECIES_ZERAORA,            TIER_PSEUDO },
    { SPECIES_DRAGAPULT,          TIER_PSEUDO },
    { SPECIES_GREAT_TUSK,         TIER_PSEUDO },
    { SPECIES_IRON_BUNDLE,        TIER_PSEUDO },
    { SPECIES_IRON_JUGULIS,       TIER_PSEUDO },
    { SPECIES_BAXCALIBUR,         TIER_PSEUDO },
    { SPECIES_CHIEN_PAO,          TIER_PSEUDO },
    { SPECIES_CHI_YU,             TIER_PSEUDO },
    { SPECIES_ROARING_MOON,       TIER_PSEUDO },
    { SPECIES_IRON_VALIANT,       TIER_PSEUDO },
    { SPECIES_GOUGING_FIRE,       TIER_PSEUDO },
    { SPECIES_RAGING_BOLT,        TIER_PSEUDO },
    { SPECIES_TERAPAGOS_TERASTAL, TIER_PSEUDO },
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
