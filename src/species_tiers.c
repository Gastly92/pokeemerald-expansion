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
    { NATIONAL_DEX_ZERAORA, TIER_LEGENDARY }, // 807
    { NATIONAL_DEX_URSHIFU, TIER_LEGENDARY }, // 892
    { NATIONAL_DEX_GLASTRIER, TIER_LEGENDARY }, // 896
    { NATIONAL_DEX_SPECTRIER, TIER_LEGENDARY }, // 897
    { NATIONAL_DEX_OGERPON, TIER_LEGENDARY }, // 1017
    { NATIONAL_DEX_TERAPAGOS, TIER_LEGENDARY }, // 1024
    { NATIONAL_DEX_PECHARUNT, TIER_LEGENDARY }, // 1025

    // ---- Pseudo — At most 1 per factory rental team ----
    { NATIONAL_DEX_DRAGONITE, TIER_PSEUDO }, // Dragonite
    { NATIONAL_DEX_TYRANITAR, TIER_PSEUDO }, // Tyranitar
    { NATIONAL_DEX_SALAMENCE, TIER_PSEUDO }, // Salamence
    { NATIONAL_DEX_METAGROSS, TIER_PSEUDO }, // Metagross
    { NATIONAL_DEX_JIRACHI, TIER_LEGENDARY }, // Jirachi
    { NATIONAL_DEX_GARCHOMP, TIER_PSEUDO }, // Garchomp
    { NATIONAL_DEX_SHAYMIN, TIER_LEGENDARY }, // Shaymin
    { NATIONAL_DEX_HYDREIGON, TIER_PSEUDO }, // Hydreigon
    { NATIONAL_DEX_CRESSELIA, TIER_LEGENDARY }, // Cresselia
    { NATIONAL_DEX_HEATRAN, TIER_LEGENDARY }, // Heatran
    { NATIONAL_DEX_COBALION, TIER_LEGENDARY }, // Cobalion
    { NATIONAL_DEX_TERRAKION, TIER_LEGENDARY }, // Terrakion
    { NATIONAL_DEX_VIRIZION, TIER_LEGENDARY }, // Virizion
    { NATIONAL_DEX_KOMMO_O, TIER_PSEUDO }, // Kommo O
    { NATIONAL_DEX_TAPU_KOKO, TIER_LEGENDARY }, // Tapu Koko
    { NATIONAL_DEX_TAPU_LELE, TIER_LEGENDARY }, // Tapu Lele
    { NATIONAL_DEX_TAPU_BULU, TIER_LEGENDARY }, // Tapu Bulu
    { NATIONAL_DEX_TAPU_FINI, TIER_LEGENDARY }, // Tapu Fini
    { NATIONAL_DEX_PHEROMOSA, TIER_PSEUDO }, // Pheromosa
    { NATIONAL_DEX_KARTANA, TIER_PSEUDO }, // Kartana
    { NATIONAL_DEX_NAGANADEL, TIER_PSEUDO }, // Naganadel
    { NATIONAL_DEX_BLACEPHALON, TIER_PSEUDO }, // Blacephalon
    { NATIONAL_DEX_DRAGAPULT, TIER_PSEUDO }, // Dragapult
    { NATIONAL_DEX_ENAMORUS, TIER_LEGENDARY }, // Enamorus
    { NATIONAL_DEX_GREAT_TUSK, TIER_PSEUDO }, // Great Tusk
    { NATIONAL_DEX_SCREAM_TAIL, TIER_PSEUDO }, // Scream Tail
    { NATIONAL_DEX_BRUTE_BONNET, TIER_PSEUDO }, // Brute Bonnet
    { NATIONAL_DEX_FLUTTER_MANE, TIER_PSEUDO }, // Flutter Mane
    { NATIONAL_DEX_SLITHER_WING, TIER_PSEUDO }, // Slither Wing
    { NATIONAL_DEX_SANDY_SHOCKS, TIER_PSEUDO }, // Sandy Shocks
    { NATIONAL_DEX_IRON_TREADS, TIER_PSEUDO }, // Iron Treads
    { NATIONAL_DEX_IRON_BUNDLE, TIER_PSEUDO }, // Iron Bundle
    { NATIONAL_DEX_IRON_HANDS, TIER_PSEUDO }, // Iron Hands
    { NATIONAL_DEX_IRON_JUGULIS, TIER_PSEUDO }, // Iron Jugulis
    { NATIONAL_DEX_IRON_MOTH, TIER_PSEUDO }, // Iron Moth
    { NATIONAL_DEX_IRON_THORNS, TIER_PSEUDO }, // Iron Thorns
    { NATIONAL_DEX_BAXCALIBUR, TIER_PSEUDO }, // Baxcalibur
    { NATIONAL_DEX_OKIDOGI, TIER_LEGENDARY }, // Okidogi
    { NATIONAL_DEX_MUNKIDORI, TIER_LEGENDARY }, // Munkidori
    { NATIONAL_DEX_FEZANDIPITI, TIER_LEGENDARY }, // Fezandipiti
    { NATIONAL_DEX_WO_CHIEN, TIER_PSEUDO }, // Wo Chien
    { NATIONAL_DEX_CHIEN_PAO, TIER_PSEUDO }, // Chien Pao
    { NATIONAL_DEX_TING_LU, TIER_PSEUDO }, // Ting Lu
    { NATIONAL_DEX_CHI_YU, TIER_PSEUDO }, // Chi Yu
    { NATIONAL_DEX_ROARING_MOON, TIER_PSEUDO }, // Roaring Moon
    { NATIONAL_DEX_IRON_VALIANT, TIER_PSEUDO }, // Iron Valiant
    { NATIONAL_DEX_WALKING_WAKE, TIER_PSEUDO }, // Walking Wake
    { NATIONAL_DEX_IRON_LEAVES, TIER_PSEUDO }, // Iron Leaves
    { NATIONAL_DEX_ARCHALUDON, TIER_PSEUDO }, // Archaludon
    { NATIONAL_DEX_GOUGING_FIRE, TIER_PSEUDO }, // Gouging Fire
    { NATIONAL_DEX_RAGING_BOLT, TIER_PSEUDO }, // Raging Bolt
    { NATIONAL_DEX_IRON_BOULDER, TIER_PSEUDO }, // Iron Boulder
    { NATIONAL_DEX_IRON_CROWN, TIER_PSEUDO }, // Iron Crown
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
