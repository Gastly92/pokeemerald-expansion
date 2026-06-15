#include "global.h"
#include "innate_abilities.h"
#include "constants/abilities.h"
#include "constants/species.h"

// FORK: fork-owned species->innate table (FEATURE_INNATE_ABILITIES). Kept here
// instead of in gSpeciesInfo so upstream syncs never touch it and the upstream
// species data stays untouched. Each row maps a species to an ABILITY_NONE-
// terminated list of innate abilities that are always active on top of that
// species' normal chosen ability. The list is variable-length (no fixed cap): add
// or remove entries freely, just keep the terminating ABILITY_NONE.
//
// ALLOWLIST — only abilities whose innate behavior has been deliberately wired in
// may appear here (see innate_abilities.h "SCOPE"). The fork grows this set one
// ability at a time. Supported innate abilities:
//   - ABILITY_LEVITATE — Ground-immunity / ungrounding, handled in src/battle_util.c
//     (IsBattlerUngroundedByAbilityItemOrEffect and the type-effectiveness calc credit an
//     innate Levitate exactly like the real one). DELIBERATE DIVERGENCE: an innate Levitate is
//     a *pure boon*, NOT identical to a real Levitate. It still floats above Ground moves and
//     entry-hazard damage, but the fork also treats it as grounded for the *beneficial* ground
//     interactions — field terrain and Toxic Spikes absorption (via IsBattlerGroundedForBenefit,
//     src/battle_util.c). So an innate-Levitate mon reaps the terrain it/an ally sets and a
//     Poison-type still clears Toxic Spikes — things a real Levitate forgoes. This is why
//     terrain summoners (the Tapus, Miraidon) and Poison floaters happily carry the innate.
//   - ABILITY_REGENERATOR — heals 1/3 max HP on switch-out, handled at the single switch-out
//     site in src/battle_script_commands.c (Cmd_switchoutabilities), additively alongside the
//     real Regenerator so a mon carrying it as an innate heals exactly like the real ability.
//     The heal is silent (no script/pop-up), so no driver is needed. Suppression parity holds:
//     the innate is gated by IsInnateActive() (Gastro Acid / Neutralizing Gas / not-on-field),
//     same as the real ability's GetBattlerAbility() path. This populates the canon Regenerator
//     users so they keep their signature pivot heal regardless of which ability slot the build
//     picks, plus a few flavor regenerators (Staryu/Starmie's regrowing core, the axolotl Wooper
//     line, Zygarde's reassembling cells).
//   - ABILITY_UNAWARE — ignores the foe's stat-stage changes in the damage and accuracy calcs,
//     handled in src/battle_util.c (the four calc sites that read ABILITY_UNAWARE — offensive and
//     defensive stat stages in the damage calc, plus evasion/accuracy in GetTotalAccuracy and
//     GetAccEvasionStageDelta — route the innate through InnateUnawareBoonStage() next to the
//     chosen-ability test). A pure calc-modifier passive like Levitate: no script / pop-up / driver.
//     Suppression parity holds via IsInnateActive() — Unaware is breakable, so an attacker's Mold
//     Breaker ignores an innate Unaware on the defender exactly as it would the real ability.
//     DELIBERATE DIVERGENCE: an innate Unaware is a *pure boon*, NOT identical to a real Unaware. A
//     real Unaware blanks the foe's stat stage in both directions (so it ignores a foe's *drop* too,
//     and takes more damage / deals less for it); the innate ignores only the foe's *boosts* and
//     keeps the foe's *drops* (always the favorable half — see InnateUnawareBoonStage, battle_util.c).
//     This populates the canon Unaware users so they keep the stat-ignore no matter which slot the
//     build picks, plus flavor picks too dull/dazed/asleep to notice the foe's buffs (Numel's
//     "doesn't notice being hit", the dazed Psyduck line, the ever-sleeping Komala, the unbothered
//     Snorlax line).
//   - ABILITY_STURDY — endures a lethal hit at full HP (B_STURDY >= GEN_5) and is immune to OHKO
//     moves, handled at the two effect sites in src/battle_util.c (the GetAdjustedDamage endure and
//     the OHKO-move accuracy gate, each gets an IsInnateActive() clause beside the cached chosen-
//     ability test). NO pure-boon divergence: Sturdy is a clean upside that never hurts its holder,
//     so the innate is a 1:1 copy of the real ability. No driver/pop-up wiring is needed — the
//     "endured / Sturdy" messages and the ability pop-up flow from the existing MOVE_RESULT_STURDIED
//     / MOVE_RESULT_ONE_HIT_KO_STURDY flags. Suppression parity holds via IsInnateActive(): Sturdy is
//     breakable, so an attacker's Mold Breaker pierces an innate Sturdy exactly as it would the real
//     ability. AI is innate-aware too: Sturdy's survival reasoning lives in DEDICATED AI helpers, NOT
//     the shared damage calc (so unlike Unaware it is NOT automatic and had to be wired) — the endure/KO
//     predictor (CanEndureHit), the OHKO-move avoidance, BattlerHasMaxHPProtection (src/battle_ai_util.c),
//     and the switch-in KO simulation (src/battle_ai_switch.c) each credit an innate Sturdy via
//     BattlerHasAbility()/SpeciesHasInnate(), so the AI doesn't blunder a hit it can't actually KO. This
//     populates the canon Sturdy users so they keep
//     the signature endure no matter which slot the build picks (Mega/regional/form constants are
//     listed so the innate survives a mid-battle form change), plus an "impenetrable shell" flavor line
//     (Shellder/Cloyster, whose shell "even a missile can't break") that lacks the real ability. Species
//     whose ONLY ability is Sturdy are omitted as redundant when unused by the frontier roster (Cosmoem,
//     Togedemaru-Totem). Ogerpon-Cornerstone is the exception — it is also sole-Sturdy, but because it IS a
//     frontier set it instead takes the innate AND a fork-owned chosen Defiant (species_ability_overrides.c),
//     so its frontier slot isn't spent on the now-innate Sturdy.
// Do NOT give a species an innate that is not on this list: nothing would honor it
// (no effect site activates it), so it would silently do nothing.

struct SpeciesInnates
{
    u16 species;
    const enum Ability *innates; // ABILITY_NONE-terminated
};

// Most species have a single innate, so they share one of these ABILITY_NONE-terminated arrays.
static const enum Ability sInnateLevitate[] = { ABILITY_LEVITATE, ABILITY_NONE };
static const enum Ability sInnateRegenerator[] = { ABILITY_REGENERATOR, ABILITY_NONE };
static const enum Ability sInnateUnaware[] = { ABILITY_UNAWARE, ABILITY_NONE };
static const enum Ability sInnateSturdy[] = { ABILITY_STURDY, ABILITY_NONE };

// A species with SEVERAL innates lists them inline at its row with INNATES(...) instead of needing a
// named combination array per pairing (which doesn't scale as the allowlist grows). The compound
// literal has static storage at file scope; the terminator is appended automatically.
#define INNATES(...) (const enum Ability[]){ __VA_ARGS__, ABILITY_NONE }

static const struct SpeciesInnates sSpeciesInnates[] =
{
    // Gen 1
    { SPECIES_GASTLY,            sInnateLevitate },
    { SPECIES_HAUNTER,           sInnateLevitate },
    { SPECIES_GENGAR,            sInnateLevitate }, // floats; primary is Cursed Body at GEN_LATEST, so observable
    { SPECIES_GENGAR_GMAX,       sInnateLevitate }, // a Gigantamaxed Gengar still floats (NOT Gengar-Mega, which is grounded)
    { SPECIES_KOFFING,           sInnateLevitate },
    { SPECIES_WEEZING,           sInnateLevitate },
    { SPECIES_WEEZING_GALAR,     sInnateLevitate }, // Misty Surge (HA) build floats AND reaps its terrain; Poison-type builds clear Toxic Spikes

    // Gen 2
    { SPECIES_MISDREAVUS,        sInnateLevitate },
    { SPECIES_MISMAGIUS,         sInnateLevitate },
    { SPECIES_UNOWN,             sInnateLevitate },
    { SPECIES_UNOWN_B,           sInnateLevitate },
    { SPECIES_UNOWN_C,           sInnateLevitate },
    { SPECIES_UNOWN_D,           sInnateLevitate },
    { SPECIES_UNOWN_E,           sInnateLevitate },
    { SPECIES_UNOWN_F,           sInnateLevitate },
    { SPECIES_UNOWN_G,           sInnateLevitate },
    { SPECIES_UNOWN_H,           sInnateLevitate },
    { SPECIES_UNOWN_I,           sInnateLevitate },
    { SPECIES_UNOWN_J,           sInnateLevitate },
    { SPECIES_UNOWN_K,           sInnateLevitate },
    { SPECIES_UNOWN_L,           sInnateLevitate },
    { SPECIES_UNOWN_M,           sInnateLevitate },
    { SPECIES_UNOWN_N,           sInnateLevitate },
    { SPECIES_UNOWN_O,           sInnateLevitate },
    { SPECIES_UNOWN_P,           sInnateLevitate },
    { SPECIES_UNOWN_Q,           sInnateLevitate },
    { SPECIES_UNOWN_R,           sInnateLevitate },
    { SPECIES_UNOWN_S,           sInnateLevitate },
    { SPECIES_UNOWN_T,           sInnateLevitate },
    { SPECIES_UNOWN_U,           sInnateLevitate },
    { SPECIES_UNOWN_V,           sInnateLevitate },
    { SPECIES_UNOWN_W,           sInnateLevitate },
    { SPECIES_UNOWN_X,           sInnateLevitate },
    { SPECIES_UNOWN_Y,           sInnateLevitate },
    { SPECIES_UNOWN_Z,           sInnateLevitate },
    { SPECIES_UNOWN_EXCLAMATION, sInnateLevitate },
    { SPECIES_UNOWN_QUESTION,    sInnateLevitate },

    // Gen 3
    { SPECIES_VIBRAVA,           sInnateLevitate },
    { SPECIES_FLYGON,            sInnateLevitate },
    { SPECIES_LUNATONE,          sInnateLevitate },
    { SPECIES_SOLROCK,           sInnateLevitate },
    { SPECIES_BALTOY,            sInnateLevitate },
    { SPECIES_CLAYDOL,           sInnateLevitate },
    { SPECIES_DUSKULL,           sInnateLevitate },
    { SPECIES_CHINGLING,         sInnateLevitate },
    { SPECIES_CHIMECHO,          sInnateLevitate },
    { SPECIES_CHIMECHO_MEGA,     sInnateLevitate },
    { SPECIES_LATIAS,            sInnateLevitate },
    { SPECIES_LATIAS_MEGA,       sInnateLevitate },
    { SPECIES_LATIOS,            sInnateLevitate },
    { SPECIES_LATIOS_MEGA,       sInnateLevitate },

    // Gen 4
    { SPECIES_BRONZOR,           sInnateLevitate },
    { SPECIES_BRONZONG,          sInnateLevitate },
    { SPECIES_CARNIVINE,         sInnateLevitate },
    { SPECIES_ROTOM,             sInnateLevitate },
    { SPECIES_ROTOM_HEAT,        sInnateLevitate },
    { SPECIES_ROTOM_WASH,        sInnateLevitate },
    { SPECIES_ROTOM_FROST,       sInnateLevitate },
    { SPECIES_ROTOM_FAN,         sInnateLevitate },
    { SPECIES_ROTOM_MOW,         sInnateLevitate },
    { SPECIES_UXIE,              sInnateLevitate },
    { SPECIES_MESPRIT,           sInnateLevitate },
    { SPECIES_AZELF,             sInnateLevitate },
    { SPECIES_GIRATINA_ORIGIN,   sInnateLevitate },
    { SPECIES_CRESSELIA,         sInnateLevitate },

    // Gen 5
    { SPECIES_TYNAMO,            sInnateLevitate },
    { SPECIES_EELEKTRIK,         sInnateLevitate },
    { SPECIES_EELEKTROSS,        sInnateLevitate },
    { SPECIES_EELEKTROSS_MEGA,   sInnateLevitate },
    { SPECIES_CRYOGONAL,         sInnateLevitate },
    { SPECIES_HYDREIGON,         sInnateLevitate },

    // Gen 6
    { SPECIES_DELPHOX_MEGA,      sInnateLevitate },

    // Gen 7
    { SPECIES_VIKAVOLT,          sInnateLevitate },
    { SPECIES_VIKAVOLT_TOTEM,    sInnateLevitate },

    // ───────────────────────────────────────────────────────────────────────────
    // Flavor floaters: no native Levitate, hover/levitate by design (innate-only,
    // observable). Base form constants (e.g. SPECIES_CASTFORM) are listed alongside
    // their form constants so the lookup matches whichever value is queried.
    // ───────────────────────────────────────────────────────────────────────────

    // Gen 1
    { SPECIES_MAGNEMITE,                INNATES(ABILITY_LEVITATE, ABILITY_STURDY) }, // flavor Levitate (hovers) + canon Sturdy (slot 1)
    { SPECIES_MAGNETON,                 INNATES(ABILITY_LEVITATE, ABILITY_STURDY) }, // flavor Levitate + canon Sturdy
    { SPECIES_MAGNEZONE,                INNATES(ABILITY_LEVITATE, ABILITY_STURDY) }, // flavor Levitate + canon Sturdy
    { SPECIES_MEW,                      sInnateLevitate },
    { SPECIES_MEWTWO,                   sInnateLevitate },
    { SPECIES_MEWTWO_MEGA_Y,            sInnateLevitate }, // Mega-X is a grounded bruiser, omitted
    { SPECIES_PORYGON,                  sInnateLevitate },
    { SPECIES_PORYGON2,                 sInnateLevitate },
    { SPECIES_PORYGON_Z,                sInnateLevitate },

    // Gen 2
    { SPECIES_CELEBI,                   sInnateLevitate },

    // Gen 3
    { SPECIES_BANETTE,                  sInnateLevitate },
    { SPECIES_BANETTE_MEGA,             sInnateLevitate },
    { SPECIES_CASTFORM,                 sInnateLevitate },
    { SPECIES_CASTFORM_NORMAL,          sInnateLevitate },
    { SPECIES_CASTFORM_RAINY,           sInnateLevitate },
    { SPECIES_CASTFORM_SNOWY,           sInnateLevitate },
    { SPECIES_CASTFORM_SUNNY,           sInnateLevitate },
    { SPECIES_DEOXYS,                   sInnateLevitate },
    { SPECIES_DEOXYS_ATTACK,            sInnateLevitate },
    { SPECIES_DEOXYS_DEFENSE,           sInnateLevitate },
    { SPECIES_DEOXYS_NORMAL,            sInnateLevitate },
    { SPECIES_DEOXYS_SPEED,             sInnateLevitate },
    { SPECIES_DUSCLOPS,                 sInnateLevitate },
    { SPECIES_FROSLASS,                 sInnateLevitate },
    { SPECIES_FROSLASS_MEGA,            sInnateLevitate },
    { SPECIES_GLALIE,                   sInnateLevitate },
    { SPECIES_GLALIE_MEGA,              sInnateLevitate },
    { SPECIES_JIRACHI,                  sInnateLevitate },
    { SPECIES_SHEDINJA,                 sInnateLevitate },
    { SPECIES_SHUPPET,                  sInnateLevitate },

    // Gen 4
    { SPECIES_DARKRAI,                  sInnateLevitate },
    { SPECIES_DARKRAI_MEGA,             sInnateLevitate },
    { SPECIES_GIRATINA_ALTERED,         sInnateLevitate }, // Origin form is already covered above

    // Gen 5
    { SPECIES_BEHEEYEM,                 sInnateLevitate },
    { SPECIES_CHANDELURE,               sInnateLevitate },
    { SPECIES_CHANDELURE_MEGA,          sInnateLevitate },
    { SPECIES_COFAGRIGUS,               sInnateLevitate },
    { SPECIES_COTTONEE,                 sInnateLevitate },
    { SPECIES_DUOSION,                  sInnateLevitate },
    { SPECIES_ELGYEM,                   sInnateLevitate },
    { SPECIES_FRILLISH,                 sInnateLevitate },
    { SPECIES_JELLICENT,                sInnateLevitate },
    { SPECIES_KLANG,                    sInnateLevitate },
    { SPECIES_KLINK,                    sInnateLevitate },
    { SPECIES_KLINKLANG,                sInnateLevitate },
    { SPECIES_LAMPENT,                  sInnateLevitate },
    { SPECIES_LITWICK,                  sInnateLevitate },
    { SPECIES_MUNNA,                    sInnateLevitate },
    { SPECIES_MUSHARNA,                 sInnateLevitate },
    { SPECIES_REUNICLUS,                sInnateLevitate },
    { SPECIES_RUNERIGUS,                sInnateLevitate },
    { SPECIES_SOLOSIS,                  sInnateLevitate },
    { SPECIES_VANILLISH,                sInnateLevitate },
    { SPECIES_VANILLITE,                sInnateLevitate },
    { SPECIES_VANILLUXE,                sInnateLevitate },
    { SPECIES_VICTINI,                  sInnateLevitate },
    { SPECIES_WHIMSICOTT,               sInnateLevitate },
    { SPECIES_YAMASK,                   sInnateLevitate },
    { SPECIES_YAMASK_GALAR,             sInnateLevitate },

    // Gen 6
    { SPECIES_AEGISLASH,                sInnateLevitate },
    { SPECIES_AEGISLASH_BLADE,          sInnateLevitate },
    { SPECIES_AEGISLASH_SHIELD,         sInnateLevitate },
    { SPECIES_CARBINK,                  sInnateLevitate },
    { SPECIES_DIANCIE,                  sInnateLevitate },
    { SPECIES_DIANCIE_MEGA,             sInnateLevitate },
    { SPECIES_DOUBLADE,                 sInnateLevitate },
    { SPECIES_GOURGEIST,                sInnateLevitate },
    { SPECIES_GOURGEIST_AVERAGE,        sInnateLevitate },
    { SPECIES_GOURGEIST_LARGE,          sInnateLevitate },
    { SPECIES_GOURGEIST_SMALL,          sInnateLevitate },
    { SPECIES_GOURGEIST_SUPER,          sInnateLevitate },
    { SPECIES_HONEDGE,                  sInnateLevitate },
    { SPECIES_HOOPA,                    sInnateLevitate },
    { SPECIES_HOOPA_CONFINED,           sInnateLevitate },
    { SPECIES_HOOPA_UNBOUND,            sInnateLevitate },
    { SPECIES_INKAY,                    sInnateLevitate }, // Inkay floats; Malamar stands, so omitted
    { SPECIES_KLEFKI,                   sInnateLevitate },
    { SPECIES_PUMPKABOO,                sInnateLevitate },
    { SPECIES_PUMPKABOO_AVERAGE,        sInnateLevitate },
    { SPECIES_PUMPKABOO_LARGE,          sInnateLevitate },
    { SPECIES_PUMPKABOO_SMALL,          sInnateLevitate },
    { SPECIES_PUMPKABOO_SUPER,          sInnateLevitate },

    // Gen 7
    { SPECIES_BLACEPHALON,              sInnateLevitate },
    { SPECIES_COMFEY,                   sInnateLevitate },
    { SPECIES_COSMOEM,                  sInnateLevitate },
    { SPECIES_COSMOG,                   sInnateLevitate },
    { SPECIES_DHELMISE,                 sInnateLevitate },
    { SPECIES_KARTANA,                  sInnateLevitate },
    { SPECIES_LUNALA,                   sInnateLevitate },
    { SPECIES_MAGEARNA,                 sInnateLevitate },
    { SPECIES_MAGEARNA_MEGA,            sInnateLevitate },
    { SPECIES_MAGEARNA_ORIGINAL,        sInnateLevitate },
    { SPECIES_MAGEARNA_ORIGINAL_MEGA,   sInnateLevitate },
    { SPECIES_NECROZMA,                 sInnateLevitate },
    { SPECIES_NECROZMA_DAWN_WINGS,      sInnateLevitate },
    { SPECIES_NECROZMA_DUSK_MANE,       sInnateLevitate },
    { SPECIES_NECROZMA_ULTRA,           sInnateLevitate },
    { SPECIES_NIHILEGO,                 sInnateLevitate },
    { SPECIES_POIPOLE,                  sInnateLevitate },
    { SPECIES_TAPU_BULU,                sInnateLevitate }, // Grassy Surge: floats AND reaps its own terrain (innate Levitate is a pure boon)
    { SPECIES_TAPU_FINI,                sInnateLevitate }, // Misty Surge
    { SPECIES_TAPU_KOKO,                sInnateLevitate }, // Electric Surge
    { SPECIES_TAPU_LELE,                sInnateLevitate }, // Psychic Surge
    { SPECIES_XURKITREE,                sInnateLevitate },

    // Gen 8
    { SPECIES_DRAGAPULT,                sInnateLevitate },
    { SPECIES_DRAKLOAK,                 sInnateLevitate },
    { SPECIES_DREEPY,                   sInnateLevitate },
    { SPECIES_POLTEAGEIST,              sInnateLevitate },
    { SPECIES_POLTEAGEIST_ANTIQUE,      sInnateLevitate },
    { SPECIES_POLTEAGEIST_PHONY,        sInnateLevitate },
    { SPECIES_REGIELEKI,                sInnateLevitate },
    { SPECIES_SINISTEA,                 sInnateLevitate },
    { SPECIES_SINISTEA_ANTIQUE,         sInnateLevitate },
    { SPECIES_SINISTEA_PHONY,           sInnateLevitate },

    // Gen 9
    { SPECIES_FLUTTER_MANE,             sInnateLevitate },
    { SPECIES_GHOLDENGO,                sInnateLevitate },
    { SPECIES_GIMMIGHOUL_ROAMING,       sInnateLevitate }, // Chest form sits on the ground, omitted
    { SPECIES_IRON_MOTH,                sInnateLevitate },
    { SPECIES_MIRAIDON,                 sInnateLevitate }, // Hadron Engine: floats AND reaps its own Electric Terrain
    { SPECIES_PECHARUNT,                sInnateLevitate },
    { SPECIES_POLTCHAGEIST,             sInnateLevitate },
    { SPECIES_POLTCHAGEIST_ARTISAN,     sInnateLevitate },
    { SPECIES_POLTCHAGEIST_COUNTERFEIT, sInnateLevitate },
    { SPECIES_SINISTCHA,                sInnateLevitate },
    { SPECIES_SINISTCHA_MASTERPIECE,    sInnateLevitate },
    { SPECIES_SINISTCHA_UNREMARKABLE,   sInnateLevitate },

    // ───────────────────────────────────────────────────────────────────────────
    // Innate Regenerator (heals 1/3 max HP on switch-out). Two groups:
    //   1) Canon Regenerator users — the species that carry Regenerator in their
    //      ability data (often as the Hidden Ability). Giving it as an innate lets
    //      them keep the signature pivot heal no matter which slot the build picks.
    //      Mega/regional forms of those lines are listed so the innate survives the
    //      transformation mid-battle (the species constant changes on Mega Evolve).
    //   2) Flavor regenerators — species strongly associated with regeneration that
    //      lack the real ability: Staryu/Starmie (regrows from its core), the axolotl
    //      Wooper lines (limb regrowth), and Zygarde (cells reassemble).
    // ───────────────────────────────────────────────────────────────────────────

    // Gen 1
    { SPECIES_SLOWPOKE,                 sInnateRegenerator },
    { SPECIES_SLOWBRO,                  sInnateRegenerator },
    { SPECIES_SLOWBRO_MEGA,             sInnateRegenerator }, // canon Mega is Shell Armor; innate persists through the Mega
    { SPECIES_SLOWKING,                 sInnateRegenerator },
    { SPECIES_SLOWPOKE_GALAR,           sInnateRegenerator },
    { SPECIES_SLOWBRO_GALAR,            sInnateRegenerator },
    { SPECIES_SLOWKING_GALAR,           sInnateRegenerator },
    { SPECIES_TANGELA,                  sInnateRegenerator },
    { SPECIES_TANGROWTH,                sInnateRegenerator },
    { SPECIES_STARYU,                   sInnateRegenerator }, // flavor: regenerates as long as its core survives
    { SPECIES_STARMIE,                  sInnateRegenerator }, // flavor

    // Gen 2
    { SPECIES_CORSOLA,                  sInnateRegenerator },
    { SPECIES_HO_OH,                    sInnateRegenerator },
    { SPECIES_WOOPER,                   INNATES(ABILITY_REGENERATOR, ABILITY_UNAWARE) }, // flavor Regen (axolotl limb regrowth) + canon Unaware (HA)
    { SPECIES_QUAGSIRE,                 INNATES(ABILITY_REGENERATOR, ABILITY_UNAWARE) }, // flavor Regen + canon Unaware (HA)

    // Gen 5
    { SPECIES_AUDINO,                   sInnateRegenerator },
    { SPECIES_AUDINO_MEGA,              sInnateRegenerator }, // canon Mega is Healer; innate persists through the Mega
    { SPECIES_SOLOSIS,                  sInnateRegenerator },
    { SPECIES_DUOSION,                  sInnateRegenerator },
    { SPECIES_REUNICLUS,                sInnateRegenerator },
    { SPECIES_FOONGUS,                  sInnateRegenerator },
    { SPECIES_AMOONGUSS,                sInnateRegenerator },
    { SPECIES_ALOMOMOLA,                sInnateRegenerator },
    { SPECIES_MIENFOO,                  sInnateRegenerator },
    { SPECIES_MIENSHAO,                 sInnateRegenerator },
    { SPECIES_TORNADUS_THERIAN,         sInnateRegenerator }, // only the Therian forme has Regenerator

    // Gen 6
    { SPECIES_ZYGARDE,                  sInnateRegenerator }, // flavor: a colony of cells that reassemble
    { SPECIES_ZYGARDE_10,               sInnateRegenerator }, // flavor
    { SPECIES_ZYGARDE_50,               sInnateRegenerator }, // flavor
    { SPECIES_ZYGARDE_COMPLETE,         sInnateRegenerator }, // flavor

    // Gen 7
    { SPECIES_MAREANIE,                 sInnateRegenerator },
    { SPECIES_TOXAPEX,                  sInnateRegenerator },

    // Gen 8
    { SPECIES_GOSSIFLEUR,               sInnateRegenerator },
    { SPECIES_ELDEGOSS,                 sInnateRegenerator },
    { SPECIES_HYDRAPPLE,                sInnateRegenerator },

    // Gen 9
    { SPECIES_KLAWF,                    sInnateRegenerator },
    { SPECIES_CYCLIZAR,                 sInnateRegenerator },
    { SPECIES_WOOPER_PALDEA,            INNATES(ABILITY_REGENERATOR, ABILITY_UNAWARE) }, // flavor Regen (Paldean axolotl) + canon Unaware (HA)
    { SPECIES_CLODSIRE,                 INNATES(ABILITY_REGENERATOR, ABILITY_UNAWARE) }, // flavor Regen + canon Unaware (HA)

    // ───────────────────────────────────────────────────────────────────────────
    // Innate Unaware (ignores the foe's stat-stage changes in the damage & accuracy
    // calcs). Two groups:
    //   1) Canon Unaware users — species that carry Unaware in their ability data
    //      (often as the Hidden Ability). Giving it as an innate lets them keep the
    //      signature stat-ignore no matter which slot the build picks. Mega forms are
    //      listed so the innate survives the transformation mid-battle. (The Wooper,
    //      Quagsire, Paldean Wooper and Clodsire lines are also canon Unaware users,
    //      but they already carry innate Regenerator above, so they take the combined
    //      combined INNATES(ABILITY_REGENERATOR, ABILITY_UNAWARE) list there instead of being repeated here.
    //      Cosmog is omitted: Unaware is already its sole ability, so an innate copy
    //      would be redundant.)
    //   2) Flavor picks — species too dull, dazed or asleep to register the foe's
    //      buffs, lacking the real ability: Numel/Camerupt ("so dull-witted it doesn't
    //      notice being hit"), the perpetually headache-dazed Psyduck line, the
    //      ever-sleeping Komala, and the unbothered Snorlax line.
    // ───────────────────────────────────────────────────────────────────────────

    // Canon Unaware users
    // Gen 1
    { SPECIES_CLEFABLE,                 sInnateUnaware }, // Unaware is the HA
    { SPECIES_CLEFABLE_MEGA,            sInnateUnaware }, // innate persists through the Mega

    // Gen 4
    { SPECIES_BIDOOF,                   sInnateUnaware },
    { SPECIES_BIBAREL,                  sInnateUnaware },

    // Gen 5
    { SPECIES_WOOBAT,                   sInnateUnaware }, // Unaware is the primary ability
    { SPECIES_SWOOBAT,                  sInnateUnaware },

    // Gen 7
    { SPECIES_PYUKUMUKU,                sInnateUnaware }, // Unaware is the HA

    // Gen 9
    { SPECIES_FUECOCO,                  sInnateUnaware }, // Unaware is the HA across the line
    { SPECIES_CROCALOR,                 sInnateUnaware },
    { SPECIES_SKELEDIRGE,               sInnateUnaware },
    { SPECIES_DONDOZO,                  sInnateUnaware }, // Unaware is the primary ability

    // Flavor Unaware (no native Unaware; too dull/dazed/asleep to notice the foe's buffs)
    // Gen 1
    { SPECIES_PSYDUCK,                  sInnateUnaware }, // perpetual headache leaves it dazed
    { SPECIES_GOLDUCK,                  sInnateUnaware }, // flavor
    { SPECIES_SNORLAX,                  sInnateUnaware }, // too busy eating/sleeping to be bothered
    { SPECIES_SNORLAX_GMAX,             sInnateUnaware }, // flavor

    // Gen 3
    { SPECIES_NUMEL,                    sInnateUnaware }, // Pokédex: so dull-witted it doesn't notice being hit
    { SPECIES_CAMERUPT,                 sInnateUnaware }, // flavor
    { SPECIES_CAMERUPT_MEGA,            sInnateUnaware }, // flavor; innate persists through the Mega

    // Gen 4
    { SPECIES_MUNCHLAX,                 sInnateUnaware }, // flavor: only ever thinks about food

    // Gen 7
    { SPECIES_KOMALA,                   sInnateUnaware }, // sleeps its whole life, oblivious to its surroundings

    // ───────────────────────────────────────────────────────────────────────────
    // Innate Sturdy (endures a lethal hit at full HP + immune to OHKO moves). Two groups:
    //   1) Canon Sturdy users — species that carry Sturdy in their ability data (often as
    //      the primary). Giving it as an innate lets them keep the signature endure no
    //      matter which slot the build picks. Mega/regional/Hisuian forms are listed so the
    //      innate survives the transformation mid-battle (the species constant changes).
    //      Species whose ONLY ability is Sturdy are omitted as redundant when unused by the frontier
    //      roster (Cosmoem, Togedemaru-Totem); Ogerpon-Cornerstone is sole-Sturdy too but IS a frontier
    //      set, so it is listed below with a chosen-ability override instead of omitted. The
    //      Magnemite/Magneton/Magnezone line is
    //      also a canon Sturdy user, but it already carries innate Levitate above, so it takes
    //      the combined INNATES(ABILITY_LEVITATE, ABILITY_STURDY) list there instead of being repeated here.
    //   2) Flavor pick — the Shellder/Cloyster line, whose shell "even a missile can't break,"
    //      lacks the real ability but is the archetypal unbreakable body.
    // ───────────────────────────────────────────────────────────────────────────

    // Canon Sturdy users
    // Gen 1
    { SPECIES_GEODUDE,                  sInnateSturdy },
    { SPECIES_GEODUDE_ALOLA,            sInnateSturdy },
    { SPECIES_GRAVELER,                 sInnateSturdy },
    { SPECIES_GRAVELER_ALOLA,           sInnateSturdy },
    { SPECIES_GOLEM,                    sInnateSturdy },
    { SPECIES_GOLEM_ALOLA,              sInnateSturdy },
    { SPECIES_ONIX,                     sInnateSturdy },
    { SPECIES_STEELIX,                  sInnateSturdy },
    { SPECIES_STEELIX_MEGA,             sInnateSturdy }, // canon Mega is Sand Force; innate persists through the Mega

    // Gen 2
    { SPECIES_BONSLY,                   sInnateSturdy },
    { SPECIES_SUDOWOODO,                sInnateSturdy },
    { SPECIES_PINECO,                   sInnateSturdy },
    { SPECIES_FORRETRESS,               sInnateSturdy },
    { SPECIES_SHUCKLE,                  sInnateSturdy },
    { SPECIES_SKARMORY,                 sInnateSturdy },
    { SPECIES_SKARMORY_MEGA,            sInnateSturdy }, // innate persists through the Mega
    { SPECIES_DONPHAN,                  sInnateSturdy },

    // Gen 3
    { SPECIES_NOSEPASS,                 sInnateSturdy },
    { SPECIES_PROBOPASS,                sInnateSturdy },
    { SPECIES_ARON,                     sInnateSturdy },
    { SPECIES_LAIRON,                   sInnateSturdy },
    { SPECIES_AGGRON,                   sInnateSturdy },
    { SPECIES_AGGRON_MEGA,              sInnateSturdy }, // canon Mega is Filter; innate persists through the Mega
    { SPECIES_RELICANTH,                sInnateSturdy },
    { SPECIES_REGIROCK,                 sInnateSturdy },

    // Gen 4
    { SPECIES_SHIELDON,                 sInnateSturdy },
    { SPECIES_BASTIODON,                sInnateSturdy },

    // Gen 5
    { SPECIES_ROGGENROLA,               sInnateSturdy },
    { SPECIES_BOLDORE,                  sInnateSturdy },
    { SPECIES_GIGALITH,                 sInnateSturdy },
    { SPECIES_SAWK,                     sInnateSturdy },
    { SPECIES_DWEBBLE,                  sInnateSturdy },
    { SPECIES_CRUSTLE,                  sInnateSturdy },
    { SPECIES_TIRTOUGA,                 sInnateSturdy },
    { SPECIES_CARRACOSTA,               sInnateSturdy },

    // Gen 6
    { SPECIES_TYRUNT,                   sInnateSturdy },
    { SPECIES_CARBINK,                  sInnateSturdy },
    { SPECIES_BERGMITE,                 sInnateSturdy },
    { SPECIES_AVALUGG,                  sInnateSturdy },
    { SPECIES_AVALUGG_HISUI,            sInnateSturdy },

    // Gen 7
    { SPECIES_TOGEDEMARU,               sInnateSturdy }, // Totem form omitted (Sturdy is its sole ability — redundant)

    // Gen 8
    { SPECIES_ARCHALUDON,               sInnateSturdy },

    // Gen 9
    { SPECIES_NACLI,                    sInnateSturdy },
    { SPECIES_NACLSTACK,                sInnateSturdy },
    { SPECIES_GARGANACL,                sInnateSturdy },
    // Sturdy is Cornerstone's SOLE ability, but it IS a frontier set — so rather than omit it as
    // redundant, it takes the innate AND a fork-owned chosen Defiant (src/species_ability_overrides.c)
    // so its frontier slot isn't wasted on the now-innate Sturdy. (The Tera form has Embody Aspect, not
    // Sturdy, so it is not listed; Tera is disabled in this fork anyway.)
    { SPECIES_OGERPON_CORNERSTONE,      sInnateSturdy },

    // Flavor Sturdy (no native Sturdy; the archetypal unbreakable shell)
    // Gen 1
    { SPECIES_SHELLDER,                 sInnateSturdy }, // its shell "even a missile can't break"
    { SPECIES_CLOYSTER,                 sInnateSturdy }, // "Its shell is harder than diamond"
};

static const enum Ability *GetSpeciesInnateList(u16 species)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sSpeciesInnates); i++)
    {
        if (sSpeciesInnates[i].species == species)
            return sSpeciesInnates[i].innates;
    }

    return NULL;
}

bool32 SpeciesHasInnate(u16 species, enum Ability ability)
{
    const enum Ability *list;
    u32 i;

    if (ability == ABILITY_NONE)
        return FALSE;

    list = GetSpeciesInnateList(species);
    if (list == NULL)
        return FALSE;

    for (i = 0; list[i] != ABILITY_NONE; i++)
    {
        if (list[i] == ability)
            return TRUE;
    }

    return FALSE;
}

enum Ability GetSpeciesInnate(u16 species, u32 index)
{
    const enum Ability *list = GetSpeciesInnateList(species);
    u32 i;

    if (list == NULL)
        return ABILITY_NONE;

    for (i = 0; list[i] != ABILITY_NONE; i++)
    {
        if (i == index)
            return list[i];
    }

    return ABILITY_NONE;
}
