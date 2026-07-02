#include "global.h"
#include "battle.h"
#include "battle_util.h"
#include "fork/innate_abilities.h"
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
// may appear here. The fork grows this set one ability at a time; giving a species
// an ability NOT on this list silently does nothing (no effect site activates it).
// Supported set:
//   LEVITATE, REGENERATOR, UNAWARE, STURDY, NATURAL_CURE, PRANKSTER,
//   OVERGROW / BLAZE / TORRENT / SWARM (pinch), SWIFT_SWIM / CHLOROPHYLL /
//   SAND_RUSH / SLUSH_RUSH (weather speed), FILTER, PRESSURE, STENCH,
//   BATTLE_ARMOR / SHELL_ARMOR, SPEED_BOOST, LIMBER, CUTE_CHARM, OBLIVIOUS,
//   SAND_VEIL / SNOW_CLOAK, COMPOUND_EYES / KEEN_EYE / ILLUMINATE,
//   INSOMNIA / VITAL_SPIRIT / SWEET_VEIL, EARLY_BIRD, IMMUNITY / PASTEL_VEIL,
//   THICK_FAT, TECHNICIAN,
//   IRON_FIST / RECKLESS / STRONG_JAW / TOUGH_CLAWS / SHARPNESS / MEGA_LAUNCHER /
//   STEELWORKER / STEELY_SPIRIT / ROCKY_PAYLOAD / SAND_FORCE / ANALYTIC /
//   ADAPTABILITY / PUNK_ROCK / STAKEOUT (offensive move-power boosters, Batch A — all
//   1:1 clean-upside copies; Sand Force also grants sandstorm-damage immunity),
//   SERENE_GRACE (doubles the chance of the holder's moves' additional effects —
//   a 1:1 clean-upside copy),
//   MULTISCALE / SOLID_ROCK / FUR_COAT / ICE_SCALES / HEATPROOF / FRIEND_GUARD /
//   WATER_BUBBLE (defensive damage reducers, Batch B — all 1:1 clean-upside copies;
//   Solid Rock rides Filter's supereffective-reduction site; Heatproof also halves burn
//   damage; Water Bubble also doubles the holder's Water moves and blocks burn),
//   GUTS / MARVEL_SCALE / QUICK_FEET / TOXIC_BOOST / FLARE_BOOST (status-conditional stat
//   boosts, Batch N — all 1:1 clean-upside copies: Guts +50% physical Atk while statused
//   (and negates burn's physical cut), Marvel Scale +50% Def while statused, Quick Feet
//   +50% Speed while statused (and ignores the paralysis Speed/PP/priority penalty), Toxic
//   Boost +50% physical while poisoned, Flare Boost +50% special while burned).
//   SUPER_LUCK / SNIPER / MERCILESS (crit-rate / crit-damage modifiers, Batch O — all 1:1 clean-upside
//   copies, canon-only (no flavor picks): Super Luck +1 crit stage, Sniper crits deal x2.25 instead of
//   x1.5, Merciless auto-crits a poisoned target).
//   SHIELD_DUST / TINTED_LENS / SCRAPPY / WONDER_SKIN / TANGLED_FEET (accuracy / type-effectiveness /
//   effect-chance modifiers, Batch P — all 1:1 clean-upside copies, canon-only (no flavor picks):
//   Shield Dust blocks the additional effects of moves used against the holder, Tinted Lens doubles
//   the holder's resisted-move damage, Scrappy lets Normal/Fighting moves hit Ghosts AND shrugs off
//   Intimidate (GEN_8+), Wonder Skin caps incoming status moves at 50% accuracy, Tangled Feet doubles
//   evasion while confused. Mega Lopunny is omitted from the Scrappy rows as redundant: its only —
//   and therefore always chosen — ability IS Scrappy, so an innate could never be observed.)
//   GALE_WINGS / TRIAGE (priority granters, Batch Q — both 1:1 clean-upside copies wired at
//   GetBattleMovePriority beside Prankster, canon-only (no flavor picks — a priority boost is potent,
//   same reasoning that kept Prankster's flavor set tight): Gale Wings gives the holder's Flying moves
//   +1 priority (only at full HP under B_GALE_WINGS >= GEN_7), Triage gives its healing moves +3
//   priority. The AI's turn-order prediction runs the same calc, so it threatens/respects both for free.)
//   NOTE: Zangoose carries innate TOXIC_BOOST, not innate Immunity — the two are contradictory
//   (Immunity blocks the poison Toxic Boost needs), so its canon-Toxic-Boost frontier identity wins;
//   innate Immunity still lives on Gligar / Snorlax.
//
// The exact per-ability semantics — effect sites, the deliberate pure-boon
// divergences, the AI wiring, and the species-selection rationale — live in the
// "Per-ability wiring reference" appendix of fork-docs/INNATE_ABILITIES.md (grep it
// for `### ABILITY_NAME` to read just the one you need). When you wire a new
// ability, add/edit its block there and keep this name list + the SCOPE list in
// include/fork/innate_abilities.h in sync (names only — the detail lives in the doc).
//
// FORMS ARE KEYED EXACTLY (no base-species fallback): the lookup matches the exact battle species,
// so a Mega / Gigantamax / regional / forme variant gets innates ONLY if it has its own row. After
// a form change gBattleMons[].species becomes the form constant, so the form must be listed to keep
// any innate. Mega forms are populated as a PURE BOON: each Mega whose BASE creature has innates has
// its own row mirroring the base's list, so e.g. Mega Venusaur keeps Overgrow / Chlorophyll / etc.
// even though its real ability is Thick Fat — the innate models the base creature's trait persisting
// through the Mega, not the Mega's own ability data. DELIBERATE EXCEPTIONS — grounded Megas must not
// float: Mega Gengar has NO row (Levitate was its only inheritable innate), and Mega Mewtwo X keeps
// only the non-floating boon (Pressure), dropping base Mewtwo's Levitate.

struct SpeciesInnates
{
    u16 species;
    const enum Ability *innates; // ABILITY_NONE-terminated
};

// A species with SEVERAL innates lists them inline at its row with INNATES(...) instead of needing a
// named combination array per pairing (which doesn't scale as the allowlist grows). The compound
// literal has static storage at file scope; the terminator is appended automatically.
#define INNATES(...) (const enum Ability[]){ __VA_ARGS__, ABILITY_NONE }

static const struct SpeciesInnates sSpeciesInnates[] =
{
    // Sorted by National Pokédex number (shown in each row's `{ // NNNN` comment); a distinct forme
    // that needs innates follows its base's number. There is no base-species fallback, so each form
    // (incl. Megas) that should carry innates needs its OWN row (see the FORMS note in the file
    // header above). List a forme ONLY when its species id differs from the base: a DEFAULT-form
    // alias (e.g. SPECIES_CASTFORM_NORMAL == SPECIES_CASTFORM, SPECIES_HOOPA_CONFINED == SPECIES_HOOPA)
    // is the same id as the bare base, so a row for it is dead duplicate data — list the bare base
    // only. The "no species appears more than once" integrity test (test/fork/innate_abilities.c)
    // enforces this. Every row
    // lists its innates inline with INNATES(...), one ability per line, sorted alphabetically. The
    // per-ability rationale (canon vs flavor picks) is documented in the file header above.

    // ----- Gen 1 -----
    { // 0001
        SPECIES_BULBASAUR,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_FILTER,
            ABILITY_NATURAL_CURE,
            ABILITY_OVERGROW,
            ABILITY_REGENERATOR
        )
    },
    { // 0002
        SPECIES_IVYSAUR,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_FILTER,
            ABILITY_NATURAL_CURE,
            ABILITY_OVERGROW,
            ABILITY_REGENERATOR
        )
    },
    { // 0003
        SPECIES_VENUSAUR,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_FILTER,
            ABILITY_NATURAL_CURE,
            ABILITY_OVERGROW,
            ABILITY_REGENERATOR
        )
    },
    { // 0003
        SPECIES_VENUSAUR_MEGA,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_FILTER,
            ABILITY_NATURAL_CURE,
            ABILITY_OVERGROW,
            ABILITY_REGENERATOR,
            ABILITY_THICK_FAT
        )
    },
    { // 0003
        SPECIES_VENUSAUR_GMAX,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_FILTER,
            ABILITY_NATURAL_CURE,
            ABILITY_OVERGROW,
            ABILITY_REGENERATOR
        )
    },
    { // 0004
        SPECIES_CHARMANDER,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0005
        SPECIES_CHARMELEON,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0006
        SPECIES_CHARIZARD,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0006
        SPECIES_CHARIZARD_MEGA_X,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0006
        SPECIES_CHARIZARD_MEGA_Y,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0006
        SPECIES_CHARIZARD_GMAX,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0007
        SPECIES_SQUIRTLE,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0008
        SPECIES_WARTORTLE,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0009
        SPECIES_BLASTOISE,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0009
        SPECIES_BLASTOISE_MEGA,
        INNATES(
            ABILITY_MEGA_LAUNCHER,
            ABILITY_TORRENT
        )
    },
    { // 0009
        SPECIES_BLASTOISE_GMAX,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0010
        SPECIES_CATERPIE,
        INNATES(
            ABILITY_SHIELD_DUST
        )
    },
    { // 0012
        SPECIES_BUTTERFREE,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_TINTED_LENS
        )
    },
    { // 0012
        SPECIES_BUTTERFREE_GMAX,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_TINTED_LENS
        )
    },
    { // 0013
        SPECIES_WEEDLE,
        INNATES(
            ABILITY_SHIELD_DUST
        )
    },
    { // 0015
        SPECIES_BEEDRILL,
        INNATES(
            ABILITY_SNIPER,
            ABILITY_SWARM
        )
    },
    { // 0015
        SPECIES_BEEDRILL_MEGA,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_SNIPER,
            ABILITY_SWARM
        )
    },
    { // 0016
        SPECIES_PIDGEY,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_TANGLED_FEET
        )
    },
    { // 0017
        SPECIES_PIDGEOTTO,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_TANGLED_FEET
        )
    },
    { // 0018
        SPECIES_PIDGEOT,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_TANGLED_FEET
        )
    },
    { // 0018
        SPECIES_PIDGEOT_MEGA,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_TANGLED_FEET
        )
    },
    { // 0019
        SPECIES_RATTATA,
        INNATES(
            ABILITY_GUTS
        )
    },
    { // 0019
        SPECIES_RATTATA_ALOLA,
        INNATES(
            ABILITY_THICK_FAT
        )
    },
    { // 0020
        SPECIES_RATICATE,
        INNATES(
            ABILITY_GUTS
        )
    },
    { // 0020
        SPECIES_RATICATE_ALOLA,
        INNATES(
            ABILITY_THICK_FAT
        )
    },
    { // 0021
        SPECIES_SPEAROW,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_SNIPER
        )
    },
    { // 0022
        SPECIES_FEAROW,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_SNIPER
        )
    },
    { // 0023
        SPECIES_EKANS,
        INNATES(
            ABILITY_LIMBER
        )
    },
    { // 0024
        SPECIES_ARBOK,
        INNATES(
            ABILITY_LIMBER
        )
    },
    { // 0027
        SPECIES_SANDSHREW,
        INNATES(
            ABILITY_SAND_RUSH,
            ABILITY_SAND_VEIL
        )
    },
    { // 0027
        SPECIES_SANDSHREW_ALOLA,
        INNATES(
            ABILITY_SLUSH_RUSH,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0028
        SPECIES_SANDSLASH,
        INNATES(
            ABILITY_SAND_RUSH,
            ABILITY_SAND_VEIL
        )
    },
    { // 0028
        SPECIES_SANDSLASH_ALOLA,
        INNATES(
            ABILITY_SLUSH_RUSH,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0035
        SPECIES_CLEFAIRY,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_FRIEND_GUARD
        )
    },
    { // 0036
        SPECIES_CLEFABLE,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_FRIEND_GUARD,
            ABILITY_UNAWARE
        )
    },
    { // 0036
        SPECIES_CLEFABLE_MEGA,
        INNATES(
            ABILITY_UNAWARE
        )
    },
    { // 0037
        SPECIES_VULPIX_ALOLA,
        INNATES(
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0038
        SPECIES_NINETALES_ALOLA,
        INNATES(
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0039
        SPECIES_JIGGLYPUFF,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_FRIEND_GUARD
        )
    },
    { // 0040
        SPECIES_WIGGLYTUFF,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_FRIEND_GUARD
        )
    },
    { // 0043
        SPECIES_ODDISH,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_STENCH
        )
    },
    { // 0044
        SPECIES_GLOOM,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_STENCH
        )
    },
    { // 0045
        SPECIES_VILEPLUME,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_STENCH
        )
    },
    { // 0048
        SPECIES_VENONAT,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_TINTED_LENS
        )
    },
    { // 0049
        SPECIES_VENOMOTH,
        INNATES(
            ABILITY_SHIELD_DUST,
            ABILITY_TINTED_LENS,
            ABILITY_WONDER_SKIN
        )
    },
    { // 0050
        SPECIES_DIGLETT,
        INNATES(
            ABILITY_SAND_FORCE,
            ABILITY_SAND_VEIL
        )
    },
    { // 0050
        SPECIES_DIGLETT_ALOLA,
        INNATES(
            ABILITY_SAND_FORCE,
            ABILITY_SAND_VEIL
        )
    },
    { // 0051
        SPECIES_DUGTRIO,
        INNATES(
            ABILITY_SAND_FORCE,
            ABILITY_SAND_VEIL
        )
    },
    { // 0051
        SPECIES_DUGTRIO_ALOLA,
        INNATES(
            ABILITY_SAND_FORCE,
            ABILITY_SAND_VEIL
        )
    },
    { // 0052
        SPECIES_MEOWTH,
        INNATES(
            ABILITY_TECHNICIAN
        )
    },
    { // 0052
        SPECIES_MEOWTH_ALOLA,
        INNATES(
            ABILITY_TECHNICIAN
        )
    },
    { // 0052
        SPECIES_MEOWTH_GALAR,
        INNATES(
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0052
        SPECIES_MEOWTH_GMAX,
        INNATES(
            ABILITY_TECHNICIAN
        )
    },
    { // 0053
        SPECIES_PERSIAN,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_TECHNICIAN
        )
    },
    { // 0053
        SPECIES_PERSIAN_ALOLA,
        INNATES(
            ABILITY_FUR_COAT,
            ABILITY_TECHNICIAN
        )
    },
    { // 0054
        SPECIES_PSYDUCK,
        INNATES(
            ABILITY_SWIFT_SWIM,
            ABILITY_UNAWARE
        )
    },
    { // 0055
        SPECIES_GOLDUCK,
        INNATES(
            ABILITY_SWIFT_SWIM,
            ABILITY_UNAWARE
        )
    },
    { // 0056
        SPECIES_MANKEY,
        INNATES(
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0057
        SPECIES_PRIMEAPE,
        INNATES(
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0060
        SPECIES_POLIWAG,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0061
        SPECIES_POLIWHIRL,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0062
        SPECIES_POLIWRATH,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0066
        SPECIES_MACHOP,
        INNATES(
            ABILITY_GUTS
        )
    },
    { // 0067
        SPECIES_MACHOKE,
        INNATES(
            ABILITY_GUTS
        )
    },
    { // 0068
        SPECIES_MACHAMP,
        INNATES(
            ABILITY_GUTS
        )
    },
    { // 0068
        SPECIES_MACHAMP_GMAX,
        INNATES(
            ABILITY_GUTS
        )
    },
    { // 0069
        SPECIES_BELLSPROUT,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0070
        SPECIES_WEEPINBELL,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0071
        SPECIES_VICTREEBEL,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0071
        SPECIES_VICTREEBEL_MEGA,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0074
        SPECIES_GEODUDE,
        INNATES(
            ABILITY_SAND_VEIL,
            ABILITY_STURDY
        )
    },
    { // 0074
        SPECIES_GEODUDE_ALOLA,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0075
        SPECIES_GRAVELER,
        INNATES(
            ABILITY_SAND_VEIL,
            ABILITY_STURDY
        )
    },
    { // 0075
        SPECIES_GRAVELER_ALOLA,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0076
        SPECIES_GOLEM,
        INNATES(
            ABILITY_SAND_VEIL,
            ABILITY_STURDY
        )
    },
    { // 0076
        SPECIES_GOLEM_ALOLA,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0077
        SPECIES_PONYTA_GALAR,
        INNATES(
            ABILITY_PASTEL_VEIL
        )
    },
    { // 0078
        SPECIES_RAPIDASH_GALAR,
        INNATES(
            ABILITY_PASTEL_VEIL
        )
    },
    { // 0079
        SPECIES_SLOWPOKE,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_REGENERATOR
        )
    },
    { // 0079
        SPECIES_SLOWPOKE_GALAR,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0080
        SPECIES_SLOWBRO,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_REGENERATOR
        )
    },
    { // 0080
        SPECIES_SLOWBRO_MEGA,
        INNATES(
            ABILITY_REGENERATOR,
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0080
        SPECIES_SLOWBRO_GALAR,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0081
        SPECIES_MAGNEMITE,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_LEVITATE,
            ABILITY_STURDY
        )
    },
    { // 0082
        SPECIES_MAGNETON,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_LEVITATE,
            ABILITY_STURDY
        )
    },
    { // 0083
        SPECIES_FARFETCHD,
        INNATES(
            ABILITY_KEEN_EYE
        )
    },
    { // 0083
        SPECIES_FARFETCHD_GALAR,
        INNATES(
            ABILITY_SCRAPPY
        )
    },
    { // 0084
        SPECIES_DODUO,
        INNATES(
            ABILITY_EARLY_BIRD,
            ABILITY_TANGLED_FEET
        )
    },
    { // 0085
        SPECIES_DODRIO,
        INNATES(
            ABILITY_EARLY_BIRD,
            ABILITY_TANGLED_FEET
        )
    },
    { // 0086
        SPECIES_SEEL,
        INNATES(
            ABILITY_THICK_FAT
        )
    },
    { // 0087
        SPECIES_DEWGONG,
        INNATES(
            ABILITY_THICK_FAT
        )
    },
    { // 0088
        SPECIES_GRIMER,
        INNATES(
            ABILITY_STENCH
        )
    },
    { // 0089
        SPECIES_MUK,
        INNATES(
            ABILITY_STENCH
        )
    },
    { // 0090
        SPECIES_SHELLDER,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_STURDY
        )
    },
    { // 0091
        SPECIES_CLOYSTER,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_STURDY
        )
    },
    { // 0092
        SPECIES_GASTLY,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0093
        SPECIES_HAUNTER,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0094
        SPECIES_GENGAR,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0094
        SPECIES_GENGAR_GMAX,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0095
        SPECIES_ONIX,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0096
        SPECIES_DROWZEE,
        INNATES(
            ABILITY_INSOMNIA
        )
    },
    { // 0097
        SPECIES_HYPNO,
        INNATES(
            ABILITY_INSOMNIA
        )
    },
    { // 0098
        SPECIES_KRABBY,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0099
        SPECIES_KINGLER,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0099
        SPECIES_KINGLER_GMAX,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0102
        SPECIES_EXEGGCUTE,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0103
        SPECIES_EXEGGUTOR,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0104
        SPECIES_CUBONE,
        INNATES(
            ABILITY_BATTLE_ARMOR
        )
    },
    { // 0105
        SPECIES_MAROWAK,
        INNATES(
            ABILITY_BATTLE_ARMOR
        )
    },
    { // 0106
        SPECIES_HITMONLEE,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_RECKLESS
        )
    },
    { // 0107
        SPECIES_HITMONCHAN,
        INNATES(
            ABILITY_IRON_FIST,
            ABILITY_KEEN_EYE
        )
    },
    { // 0108
        SPECIES_LICKITUNG,
        INNATES(
            ABILITY_OBLIVIOUS
        )
    },
    { // 0109
        SPECIES_KOFFING,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_STENCH
        )
    },
    { // 0110
        SPECIES_WEEZING,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_STENCH
        )
    },
    { // 0110
        SPECIES_WEEZING_GALAR,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0111
        SPECIES_RHYHORN,
        INNATES(
            ABILITY_RECKLESS
        )
    },
    { // 0112
        SPECIES_RHYDON,
        INNATES(
            ABILITY_RECKLESS
        )
    },
    { // 0113
        SPECIES_CHANSEY,
        INNATES(
            ABILITY_FRIEND_GUARD,
            ABILITY_NATURAL_CURE,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0114
        SPECIES_TANGELA,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_REGENERATOR
        )
    },
    { // 0115
        SPECIES_KANGASKHAN,
        INNATES(
            ABILITY_EARLY_BIRD,
            ABILITY_SCRAPPY
        )
    },
    { // 0115
        SPECIES_KANGASKHAN_MEGA,
        INNATES(
            ABILITY_EARLY_BIRD,
            ABILITY_SCRAPPY
        )
    },
    { // 0116
        SPECIES_HORSEA,
        INNATES(
            ABILITY_SNIPER,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0117
        SPECIES_SEADRA,
        INNATES(
            ABILITY_SNIPER
        )
    },
    { // 0118
        SPECIES_GOLDEEN,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0119
        SPECIES_SEAKING,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0120
        SPECIES_STARYU,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_ILLUMINATE,
            ABILITY_NATURAL_CURE,
            ABILITY_REGENERATOR
        )
    },
    { // 0121
        SPECIES_STARMIE,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_ILLUMINATE,
            ABILITY_NATURAL_CURE,
            ABILITY_REGENERATOR
        )
    },
    { // 0121
        SPECIES_STARMIE_MEGA,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_NATURAL_CURE,
            ABILITY_REGENERATOR
        )
    },
    { // 0122
        SPECIES_MR_MIME,
        INNATES(
            ABILITY_FILTER,
            ABILITY_TECHNICIAN
        )
    },
    { // 0122
        SPECIES_MR_MIME_GALAR,
        INNATES(
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0123
        SPECIES_SCYTHER,
        INNATES(
            ABILITY_SWARM,
            ABILITY_TECHNICIAN
        )
    },
    { // 0124
        SPECIES_JYNX,
        INNATES(
            ABILITY_OBLIVIOUS
        )
    },
    { // 0125
        SPECIES_ELECTABUZZ,
        INNATES(
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0126
        SPECIES_MAGMAR,
        INNATES(
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0129
        SPECIES_MAGIKARP,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0131
        SPECIES_LAPRAS,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0131
        SPECIES_LAPRAS_GMAX,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0132
        SPECIES_DITTO,
        INNATES(
            ABILITY_LIMBER
        )
    },
    { // 0133
        SPECIES_EEVEE_STARTER,
        INNATES(
            ABILITY_ADAPTABILITY
        )
    },
    { // 0135
        SPECIES_JOLTEON,
        INNATES(
            ABILITY_QUICK_FEET
        )
    },
    { // 0136
        SPECIES_FLAREON,
        INNATES(
            ABILITY_GUTS
        )
    },
    { // 0137
        SPECIES_PORYGON,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_LEVITATE
        )
    },
    { // 0138
        SPECIES_OMANYTE,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0139
        SPECIES_OMASTAR,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0140
        SPECIES_KABUTO,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0141
        SPECIES_KABUTOPS,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0142
        SPECIES_AERODACTYL,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0142
        SPECIES_AERODACTYL_MEGA,
        INNATES(
            ABILITY_PRESSURE,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0143
        SPECIES_SNORLAX,
        INNATES(
            ABILITY_IMMUNITY,
            ABILITY_THICK_FAT,
            ABILITY_UNAWARE
        )
    },
    { // 0143
        SPECIES_SNORLAX_GMAX,
        INNATES(
            ABILITY_IMMUNITY,
            ABILITY_THICK_FAT,
            ABILITY_UNAWARE
        )
    },
    { // 0144
        SPECIES_ARTICUNO,
        INNATES(
            ABILITY_PRESSURE,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0145
        SPECIES_ZAPDOS,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0146
        SPECIES_MOLTRES,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0147
        SPECIES_DRATINI,
        INNATES(
            ABILITY_MARVEL_SCALE
        )
    },
    { // 0148
        SPECIES_DRAGONAIR,
        INNATES(
            ABILITY_MARVEL_SCALE
        )
    },
    { // 0149
        SPECIES_DRAGONITE,
        INNATES(
            ABILITY_MULTISCALE
        )
    },
    { // 0149
        SPECIES_DRAGONITE_MEGA,
        INNATES(
            ABILITY_MULTISCALE
        )
    },
    { // 0150
        SPECIES_MEWTWO,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE
        )
    },
    { // 0150
        SPECIES_MEWTWO_MEGA_X,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0150
        SPECIES_MEWTWO_MEGA_Y,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0151
        SPECIES_MEW,
        INNATES(
            ABILITY_LEVITATE
        )
    },

    // ----- Gen 2 -----
    { // 0152
        SPECIES_CHIKORITA,
        INNATES(
            ABILITY_NATURAL_CURE,
            ABILITY_OVERGROW
        )
    },
    { // 0153
        SPECIES_BAYLEEF,
        INNATES(
            ABILITY_NATURAL_CURE,
            ABILITY_OVERGROW
        )
    },
    { // 0154
        SPECIES_MEGANIUM,
        INNATES(
            ABILITY_NATURAL_CURE,
            ABILITY_OVERGROW
        )
    },
    { // 0154
        SPECIES_MEGANIUM_MEGA,
        INNATES(
            ABILITY_NATURAL_CURE,
            ABILITY_OVERGROW
        )
    },
    { // 0155
        SPECIES_CYNDAQUIL,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0156
        SPECIES_QUILAVA,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0157
        SPECIES_TYPHLOSION,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0157
        SPECIES_TYPHLOSION_HISUI,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0158
        SPECIES_TOTODILE,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0159
        SPECIES_CROCONAW,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0160
        SPECIES_FERALIGATR,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0160
        SPECIES_FERALIGATR_MEGA,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0161
        SPECIES_SENTRET,
        INNATES(
            ABILITY_KEEN_EYE
        )
    },
    { // 0162
        SPECIES_FURRET,
        INNATES(
            ABILITY_KEEN_EYE
        )
    },
    { // 0163
        SPECIES_HOOTHOOT,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_KEEN_EYE,
            ABILITY_TINTED_LENS
        )
    },
    { // 0164
        SPECIES_NOCTOWL,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_KEEN_EYE,
            ABILITY_TINTED_LENS
        )
    },
    { // 0165
        SPECIES_LEDYBA,
        INNATES(
            ABILITY_EARLY_BIRD,
            ABILITY_SWARM
        )
    },
    { // 0166
        SPECIES_LEDIAN,
        INNATES(
            ABILITY_EARLY_BIRD,
            ABILITY_IRON_FIST,
            ABILITY_SWARM
        )
    },
    { // 0167
        SPECIES_SPINARAK,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_SNIPER,
            ABILITY_SWARM
        )
    },
    { // 0168
        SPECIES_ARIADOS,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_SNIPER,
            ABILITY_SWARM
        )
    },
    { // 0170
        SPECIES_CHINCHOU,
        INNATES(
            ABILITY_ILLUMINATE
        )
    },
    { // 0171
        SPECIES_LANTURN,
        INNATES(
            ABILITY_ILLUMINATE
        )
    },
    { // 0173
        SPECIES_CLEFFA,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_FRIEND_GUARD
        )
    },
    { // 0174
        SPECIES_IGGLYBUFF,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_FRIEND_GUARD
        )
    },
    { // 0175
        SPECIES_TOGEPI,
        INNATES(
            ABILITY_SERENE_GRACE,
            ABILITY_SUPER_LUCK
        )
    },
    { // 0176
        SPECIES_TOGETIC,
        INNATES(
            ABILITY_SERENE_GRACE,
            ABILITY_SUPER_LUCK
        )
    },
    { // 0177
        SPECIES_NATU,
        INNATES(
            ABILITY_EARLY_BIRD
        )
    },
    { // 0178
        SPECIES_XATU,
        INNATES(
            ABILITY_EARLY_BIRD
        )
    },
    { // 0182
        SPECIES_BELLOSSOM,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_NATURAL_CURE
        )
    },
    { // 0183
        SPECIES_MARILL,
        INNATES(
            ABILITY_THICK_FAT
        )
    },
    { // 0184
        SPECIES_AZUMARILL,
        INNATES(
            ABILITY_THICK_FAT
        )
    },
    { // 0185
        SPECIES_SUDOWOODO,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0187
        SPECIES_HOPPIP,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0188
        SPECIES_SKIPLOOM,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0189
        SPECIES_JUMPLUFF,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0190
        SPECIES_AIPOM,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0191
        SPECIES_SUNKERN,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_EARLY_BIRD
        )
    },
    { // 0192
        SPECIES_SUNFLORA,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_EARLY_BIRD
        )
    },
    { // 0193
        SPECIES_YANMA,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_SPEED_BOOST
        )
    },
    { // 0194
        SPECIES_WOOPER,
        INNATES(
            ABILITY_REGENERATOR,
            ABILITY_UNAWARE
        )
    },
    { // 0194
        SPECIES_WOOPER_PALDEA,
        INNATES(
            ABILITY_REGENERATOR,
            ABILITY_UNAWARE
        )
    },
    { // 0195
        SPECIES_QUAGSIRE,
        INNATES(
            ABILITY_REGENERATOR,
            ABILITY_UNAWARE
        )
    },
    { // 0198
        SPECIES_MURKROW,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_PRANKSTER,
            ABILITY_SUPER_LUCK
        )
    },
    { // 0199
        SPECIES_SLOWKING,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_REGENERATOR
        )
    },
    { // 0199
        SPECIES_SLOWKING_GALAR,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0200
        SPECIES_MISDREAVUS,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_B,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_C,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_D,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_E,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_F,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_G,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_H,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_I,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_J,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_K,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_L,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_M,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_N,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_O,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_P,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_Q,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_R,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_S,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_T,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_U,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_V,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_W,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_X,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_Y,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_Z,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_EXCLAMATION,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_QUESTION,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0203
        SPECIES_GIRAFARIG,
        INNATES(
            ABILITY_EARLY_BIRD
        )
    },
    { // 0204
        SPECIES_PINECO,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0205
        SPECIES_FORRETRESS,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0206
        SPECIES_DUNSPARCE,
        INNATES(
            ABILITY_SERENE_GRACE
        )
    },
    { // 0207
        SPECIES_GLIGAR,
        INNATES(
            ABILITY_IMMUNITY,
            ABILITY_SAND_VEIL
        )
    },
    { // 0208
        SPECIES_STEELIX,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0208
        SPECIES_STEELIX_MEGA,
        INNATES(
            ABILITY_SAND_FORCE,
            ABILITY_STURDY
        )
    },
    { // 0210
        SPECIES_GRANBULL,
        INNATES(
            ABILITY_QUICK_FEET
        )
    },
    { // 0211
        SPECIES_QWILFISH,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0211
        SPECIES_QWILFISH_HISUI,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0212
        SPECIES_SCIZOR,
        INNATES(
            ABILITY_SWARM,
            ABILITY_TECHNICIAN
        )
    },
    { // 0212
        SPECIES_SCIZOR_MEGA,
        INNATES(
            ABILITY_SWARM,
            ABILITY_TECHNICIAN
        )
    },
    { // 0213
        SPECIES_SHUCKLE,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0214
        SPECIES_HERACROSS,
        INNATES(
            ABILITY_GUTS,
            ABILITY_SWARM
        )
    },
    { // 0214
        SPECIES_HERACROSS_MEGA,
        INNATES(
            ABILITY_GUTS,
            ABILITY_SWARM
        )
    },
    { // 0215
        SPECIES_SNEASEL,
        INNATES(
            ABILITY_KEEN_EYE
        )
    },
    { // 0215
        SPECIES_SNEASEL_HISUI,
        INNATES(
            ABILITY_KEEN_EYE
        )
    },
    { // 0216
        SPECIES_TEDDIURSA,
        INNATES(
            ABILITY_QUICK_FEET
        )
    },
    { // 0217
        SPECIES_URSARING,
        INNATES(
            ABILITY_GUTS,
            ABILITY_QUICK_FEET
        )
    },
    { // 0220
        SPECIES_SWINUB,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_SNOW_CLOAK,
            ABILITY_THICK_FAT
        )
    },
    { // 0221
        SPECIES_PILOSWINE,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_SNOW_CLOAK,
            ABILITY_THICK_FAT
        )
    },
    { // 0222
        SPECIES_CORSOLA,
        INNATES(
            ABILITY_NATURAL_CURE,
            ABILITY_REGENERATOR
        )
    },
    { // 0223
        SPECIES_REMORAID,
        INNATES(
            ABILITY_SNIPER
        )
    },
    { // 0224
        SPECIES_OCTILLERY,
        INNATES(
            ABILITY_SNIPER
        )
    },
    { // 0225
        SPECIES_DELIBIRD,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0226
        SPECIES_MANTINE,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0227
        SPECIES_SKARMORY,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_STURDY
        )
    },
    { // 0227
        SPECIES_SKARMORY_MEGA,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0228
        SPECIES_HOUNDOUR,
        INNATES(
            ABILITY_EARLY_BIRD
        )
    },
    { // 0229
        SPECIES_HOUNDOOM,
        INNATES(
            ABILITY_EARLY_BIRD
        )
    },
    { // 0229
        SPECIES_HOUNDOOM_MEGA,
        INNATES(
            ABILITY_EARLY_BIRD
        )
    },
    { // 0230
        SPECIES_KINGDRA,
        INNATES(
            ABILITY_SNIPER,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0231
        SPECIES_PHANPY,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0232
        SPECIES_DONPHAN,
        INNATES(
            ABILITY_SAND_VEIL,
            ABILITY_STURDY
        )
    },
    { // 0233
        SPECIES_PORYGON2,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_LEVITATE
        )
    },
    { // 0235
        SPECIES_SMEARGLE,
        INNATES(
            ABILITY_TECHNICIAN
        )
    },
    { // 0236
        SPECIES_TYROGUE,
        INNATES(
            ABILITY_GUTS,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0237
        SPECIES_HITMONTOP,
        INNATES(
            ABILITY_TECHNICIAN
        )
    },
    { // 0238
        SPECIES_SMOOCHUM,
        INNATES(
            ABILITY_OBLIVIOUS
        )
    },
    { // 0239
        SPECIES_ELEKID,
        INNATES(
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0240
        SPECIES_MAGBY,
        INNATES(
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0241
        SPECIES_MILTANK,
        INNATES(
            ABILITY_SCRAPPY,
            ABILITY_THICK_FAT
        )
    },
    { // 0242
        SPECIES_BLISSEY,
        INNATES(
            ABILITY_FRIEND_GUARD,
            ABILITY_NATURAL_CURE,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0243
        SPECIES_RAIKOU,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0244
        SPECIES_ENTEI,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0245
        SPECIES_SUICUNE,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0246
        SPECIES_LARVITAR,
        INNATES(
            ABILITY_GUTS,
            ABILITY_SAND_VEIL
        )
    },
    { // 0249
        SPECIES_LUGIA,
        INNATES(
            ABILITY_MULTISCALE,
            ABILITY_PRESSURE
        )
    },
    { // 0250
        SPECIES_HO_OH,
        INNATES(
            ABILITY_PRESSURE,
            ABILITY_REGENERATOR
        )
    },
    { // 0251
        SPECIES_CELEBI,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_NATURAL_CURE,
            ABILITY_REGENERATOR
        )
    },

    // ----- Gen 3 -----
    { // 0252
        SPECIES_TREECKO,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0253
        SPECIES_GROVYLE,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0254
        SPECIES_SCEPTILE,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0254
        SPECIES_SCEPTILE_MEGA,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0255
        SPECIES_TORCHIC,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_SPEED_BOOST
        )
    },
    { // 0256
        SPECIES_COMBUSKEN,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_SPEED_BOOST
        )
    },
    { // 0257
        SPECIES_BLAZIKEN,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_SPEED_BOOST
        )
    },
    { // 0257
        SPECIES_BLAZIKEN_MEGA,
        INNATES(
            ABILITY_SPEED_BOOST
        )
    },
    { // 0258
        SPECIES_MUDKIP,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0259
        SPECIES_MARSHTOMP,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0260
        SPECIES_SWAMPERT,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0260
        SPECIES_SWAMPERT_MEGA,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0261
        SPECIES_POOCHYENA,
        INNATES(
            ABILITY_QUICK_FEET
        )
    },
    { // 0262
        SPECIES_MIGHTYENA,
        INNATES(
            ABILITY_QUICK_FEET
        )
    },
    { // 0263
        SPECIES_ZIGZAGOON,
        INNATES(
            ABILITY_QUICK_FEET
        )
    },
    { // 0263
        SPECIES_ZIGZAGOON_GALAR,
        INNATES(
            ABILITY_QUICK_FEET
        )
    },
    { // 0264
        SPECIES_LINOONE,
        INNATES(
            ABILITY_QUICK_FEET
        )
    },
    { // 0264
        SPECIES_LINOONE_GALAR,
        INNATES(
            ABILITY_QUICK_FEET
        )
    },
    { // 0265
        SPECIES_WURMPLE,
        INNATES(
            ABILITY_SHIELD_DUST
        )
    },
    { // 0267
        SPECIES_BEAUTIFLY,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0269
        SPECIES_DUSTOX,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_SHIELD_DUST
        )
    },
    { // 0270
        SPECIES_LOTAD,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0271
        SPECIES_LOMBRE,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0272
        SPECIES_LUDICOLO,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0273
        SPECIES_SEEDOT,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_EARLY_BIRD
        )
    },
    { // 0274
        SPECIES_NUZLEAF,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_EARLY_BIRD
        )
    },
    { // 0275
        SPECIES_SHIFTRY,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_EARLY_BIRD
        )
    },
    { // 0276
        SPECIES_TAILLOW,
        INNATES(
            ABILITY_GUTS,
            ABILITY_SCRAPPY
        )
    },
    { // 0277
        SPECIES_SWELLOW,
        INNATES(
            ABILITY_GUTS,
            ABILITY_SCRAPPY
        )
    },
    { // 0278
        SPECIES_WINGULL,
        INNATES(
            ABILITY_KEEN_EYE
        )
    },
    { // 0279
        SPECIES_PELIPPER,
        INNATES(
            ABILITY_KEEN_EYE
        )
    },
    { // 0280
        SPECIES_RALTS,
        INNATES(
            ABILITY_SERENE_GRACE
        )
    },
    { // 0281
        SPECIES_KIRLIA,
        INNATES(
            ABILITY_SERENE_GRACE
        )
    },
    { // 0282
        SPECIES_GARDEVOIR,
        INNATES(
            ABILITY_SERENE_GRACE
        )
    },
    { // 0282
        SPECIES_GARDEVOIR_MEGA,
        INNATES(
            ABILITY_SERENE_GRACE
        )
    },
    { // 0283
        SPECIES_SURSKIT,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0285
        SPECIES_SHROOMISH,
        INNATES(
            ABILITY_QUICK_FEET
        )
    },
    { // 0286
        SPECIES_BRELOOM,
        INNATES(
            ABILITY_TECHNICIAN
        )
    },
    { // 0288
        SPECIES_VIGOROTH,
        INNATES(
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0290
        SPECIES_NINCADA,
        INNATES(
            ABILITY_COMPOUND_EYES
        )
    },
    { // 0291
        SPECIES_NINJASK,
        INNATES(
            ABILITY_SPEED_BOOST
        )
    },
    { // 0292
        SPECIES_SHEDINJA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0294
        SPECIES_LOUDRED,
        INNATES(
            ABILITY_SCRAPPY
        )
    },
    { // 0295
        SPECIES_EXPLOUD,
        INNATES(
            ABILITY_SCRAPPY
        )
    },
    { // 0296
        SPECIES_MAKUHITA,
        INNATES(
            ABILITY_GUTS,
            ABILITY_THICK_FAT
        )
    },
    { // 0297
        SPECIES_HARIYAMA,
        INNATES(
            ABILITY_GUTS,
            ABILITY_THICK_FAT
        )
    },
    { // 0298
        SPECIES_AZURILL,
        INNATES(
            ABILITY_THICK_FAT
        )
    },
    { // 0299
        SPECIES_NOSEPASS,
        INNATES(
            ABILITY_SAND_FORCE,
            ABILITY_STURDY
        )
    },
    { // 0300
        SPECIES_SKITTY,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_WONDER_SKIN
        )
    },
    { // 0301
        SPECIES_DELCATTY,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_WONDER_SKIN
        )
    },
    { // 0302
        SPECIES_SABLEYE,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_PRANKSTER
        )
    },
    { // 0302
        SPECIES_SABLEYE_MEGA,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_PRANKSTER
        )
    },
    { // 0304
        SPECIES_ARON,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0305
        SPECIES_LAIRON,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0306
        SPECIES_AGGRON,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0306
        SPECIES_AGGRON_MEGA,
        INNATES(
            ABILITY_FILTER,
            ABILITY_STURDY
        )
    },
    { // 0313
        SPECIES_VOLBEAT,
        INNATES(
            ABILITY_ILLUMINATE,
            ABILITY_PRANKSTER,
            ABILITY_SWARM
        )
    },
    { // 0314
        SPECIES_ILLUMISE,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_PRANKSTER,
            ABILITY_TINTED_LENS
        )
    },
    { // 0315
        SPECIES_ROSELIA,
        INNATES(
            ABILITY_NATURAL_CURE
        )
    },
    { // 0316
        SPECIES_GULPIN,
        INNATES(
            ABILITY_STENCH
        )
    },
    { // 0317
        SPECIES_SWALOT,
        INNATES(
            ABILITY_STENCH
        )
    },
    { // 0318
        SPECIES_CARVANHA,
        INNATES(
            ABILITY_SPEED_BOOST
        )
    },
    { // 0319
        SPECIES_SHARPEDO,
        INNATES(
            ABILITY_SPEED_BOOST
        )
    },
    { // 0319
        SPECIES_SHARPEDO_MEGA,
        INNATES(
            ABILITY_SPEED_BOOST,
            ABILITY_STRONG_JAW
        )
    },
    { // 0320
        SPECIES_WAILMER,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_PRESSURE,
            ABILITY_THICK_FAT
        )
    },
    { // 0321
        SPECIES_WAILORD,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_PRESSURE,
            ABILITY_THICK_FAT
        )
    },
    { // 0322
        SPECIES_NUMEL,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_UNAWARE
        )
    },
    { // 0323
        SPECIES_CAMERUPT,
        INNATES(
            ABILITY_SOLID_ROCK,
            ABILITY_UNAWARE
        )
    },
    { // 0323
        SPECIES_CAMERUPT_MEGA,
        INNATES(
            ABILITY_SOLID_ROCK,
            ABILITY_UNAWARE
        )
    },
    { // 0324
        SPECIES_TORKOAL,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0325
        SPECIES_SPOINK,
        INNATES(
            ABILITY_THICK_FAT
        )
    },
    { // 0326
        SPECIES_GRUMPIG,
        INNATES(
            ABILITY_THICK_FAT
        )
    },
    { // 0327
        SPECIES_SPINDA,
        INNATES(
            ABILITY_TANGLED_FEET
        )
    },
    { // 0329
        SPECIES_VIBRAVA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0330
        SPECIES_FLYGON,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0331
        SPECIES_CACNEA,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0332
        SPECIES_CACTURNE,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0333
        SPECIES_SWABLU,
        INNATES(
            ABILITY_NATURAL_CURE
        )
    },
    { // 0334
        SPECIES_ALTARIA,
        INNATES(
            ABILITY_NATURAL_CURE
        )
    },
    { // 0334
        SPECIES_ALTARIA_MEGA,
        INNATES(
            ABILITY_NATURAL_CURE
        )
    },
    { // 0335
        SPECIES_ZANGOOSE,
        INNATES(
            ABILITY_TOXIC_BOOST
        )
    },
    { // 0336
        SPECIES_SEVIPER,
        INNATES(
            ABILITY_LIMBER
        )
    },
    { // 0337
        SPECIES_LUNATONE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0338
        SPECIES_SOLROCK,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0339
        SPECIES_BARBOACH,
        INNATES(
            ABILITY_OBLIVIOUS
        )
    },
    { // 0340
        SPECIES_WHISCASH,
        INNATES(
            ABILITY_OBLIVIOUS
        )
    },
    { // 0341
        SPECIES_CORPHISH,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0342
        SPECIES_CRAWDAUNT,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0343
        SPECIES_BALTOY,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0344
        SPECIES_CLAYDOL,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0347
        SPECIES_ANORITH,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0348
        SPECIES_ARMALDO,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0349
        SPECIES_FEEBAS,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_OBLIVIOUS,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0350
        SPECIES_MILOTIC,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_MARVEL_SCALE,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0351
        SPECIES_CASTFORM,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0351
        SPECIES_CASTFORM_SUNNY,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0351
        SPECIES_CASTFORM_RAINY,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0351
        SPECIES_CASTFORM_SNOWY,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0353
        SPECIES_SHUPPET,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE
        )
    },
    { // 0354
        SPECIES_BANETTE,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE
        )
    },
    { // 0354
        SPECIES_BANETTE_MEGA,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE
        )
    },
    { // 0355
        SPECIES_DUSKULL,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0356
        SPECIES_DUSCLOPS,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE
        )
    },
    { // 0357
        SPECIES_TROPIUS,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0358
        SPECIES_CHIMECHO,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0358
        SPECIES_CHIMECHO_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0359
        SPECIES_ABSOL,
        INNATES(
            ABILITY_PRESSURE,
            ABILITY_SUPER_LUCK
        )
    },
    { // 0359
        SPECIES_ABSOL_MEGA,
        INNATES(
            ABILITY_PRESSURE,
            ABILITY_SUPER_LUCK
        )
    },
    { // 0359
        SPECIES_ABSOL_MEGA_Z,
        INNATES(
            ABILITY_PRESSURE,
            ABILITY_SUPER_LUCK
        )
    },
    { // 0362
        SPECIES_GLALIE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0362
        SPECIES_GLALIE_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0363
        SPECIES_SPHEAL,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_THICK_FAT
        )
    },
    { // 0364
        SPECIES_SEALEO,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_THICK_FAT
        )
    },
    { // 0365
        SPECIES_WALREIN,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_THICK_FAT
        )
    },
    { // 0366
        SPECIES_CLAMPERL,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0367
        SPECIES_HUNTAIL,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0368
        SPECIES_GOREBYSS,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0369
        SPECIES_RELICANTH,
        INNATES(
            ABILITY_STURDY,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0370
        SPECIES_LUVDISC,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0376
        SPECIES_METAGROSS_MEGA,
        INNATES(
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0377
        SPECIES_REGIROCK,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0380
        SPECIES_LATIAS,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0380
        SPECIES_LATIAS_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0381
        SPECIES_LATIOS,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0381
        SPECIES_LATIOS_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0385
        SPECIES_JIRACHI,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0386
        SPECIES_DEOXYS,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE
        )
    },
    { // 0386
        SPECIES_DEOXYS_ATTACK,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE
        )
    },
    { // 0386
        SPECIES_DEOXYS_DEFENSE,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE
        )
    },
    { // 0386
        SPECIES_DEOXYS_SPEED,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE
        )
    },

    // ----- Gen 4 -----
    { // 0387
        SPECIES_TURTWIG,
        INNATES(
            ABILITY_OVERGROW,
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0388
        SPECIES_GROTLE,
        INNATES(
            ABILITY_OVERGROW,
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0389
        SPECIES_TORTERRA,
        INNATES(
            ABILITY_OVERGROW,
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0390
        SPECIES_CHIMCHAR,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_IRON_FIST
        )
    },
    { // 0391
        SPECIES_MONFERNO,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_IRON_FIST
        )
    },
    { // 0392
        SPECIES_INFERNAPE,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_IRON_FIST
        )
    },
    { // 0393
        SPECIES_PIPLUP,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0394
        SPECIES_PRINPLUP,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0395
        SPECIES_EMPOLEON,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0396
        SPECIES_STARLY,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_RECKLESS
        )
    },
    { // 0397
        SPECIES_STARAVIA,
        INNATES(
            ABILITY_RECKLESS
        )
    },
    { // 0398
        SPECIES_STARAPTOR,
        INNATES(
            ABILITY_RECKLESS
        )
    },
    { // 0398
        SPECIES_STARAPTOR_MEGA,
        INNATES(
            ABILITY_RECKLESS
        )
    },
    { // 0399
        SPECIES_BIDOOF,
        INNATES(
            ABILITY_UNAWARE
        )
    },
    { // 0400
        SPECIES_BIBAREL,
        INNATES(
            ABILITY_UNAWARE
        )
    },
    { // 0402
        SPECIES_KRICKETUNE,
        INNATES(
            ABILITY_SWARM,
            ABILITY_TECHNICIAN
        )
    },
    { // 0403
        SPECIES_SHINX,
        INNATES(
            ABILITY_GUTS
        )
    },
    { // 0404
        SPECIES_LUXIO,
        INNATES(
            ABILITY_GUTS
        )
    },
    { // 0405
        SPECIES_LUXRAY,
        INNATES(
            ABILITY_GUTS
        )
    },
    { // 0406
        SPECIES_BUDEW,
        INNATES(
            ABILITY_NATURAL_CURE
        )
    },
    { // 0407
        SPECIES_ROSERADE,
        INNATES(
            ABILITY_NATURAL_CURE,
            ABILITY_TECHNICIAN
        )
    },
    { // 0410
        SPECIES_SHIELDON,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0411
        SPECIES_BASTIODON,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0416
        SPECIES_VESPIQUEN,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0418
        SPECIES_BUIZEL,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0419
        SPECIES_FLOATZEL,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0420
        SPECIES_CHERUBI,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0422
        SPECIES_SHELLOS_WEST,
        INNATES(
            ABILITY_SAND_FORCE
        )
    },
    { // 0422
        SPECIES_SHELLOS_EAST,
        INNATES(
            ABILITY_SAND_FORCE
        )
    },
    { // 0423
        SPECIES_GASTRODON_WEST,
        INNATES(
            ABILITY_SAND_FORCE
        )
    },
    { // 0423
        SPECIES_GASTRODON_EAST,
        INNATES(
            ABILITY_SAND_FORCE
        )
    },
    { // 0424
        SPECIES_AMBIPOM,
        INNATES(
            ABILITY_PRANKSTER,
            ABILITY_TECHNICIAN
        )
    },
    { // 0425
        SPECIES_DRIFLOON,
        INNATES(
            ABILITY_FLARE_BOOST
        )
    },
    { // 0426
        SPECIES_DRIFBLIM,
        INNATES(
            ABILITY_FLARE_BOOST
        )
    },
    { // 0427
        SPECIES_BUNEARY,
        INNATES(
            ABILITY_LIMBER
        )
    },
    { // 0428
        SPECIES_LOPUNNY,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_LIMBER
        )
    },
    { // 0428
        SPECIES_LOPUNNY_MEGA,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_LIMBER
        )
    },
    { // 0429
        SPECIES_MISMAGIUS,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0430
        SPECIES_HONCHKROW,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_SUPER_LUCK
        )
    },
    { // 0431
        SPECIES_GLAMEOW,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_LIMBER
        )
    },
    { // 0432
        SPECIES_PURUGLY,
        INNATES(
            ABILITY_THICK_FAT
        )
    },
    { // 0433
        SPECIES_CHINGLING,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0434
        SPECIES_STUNKY,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_STENCH
        )
    },
    { // 0435
        SPECIES_SKUNTANK,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_STENCH
        )
    },
    { // 0436
        SPECIES_BRONZOR,
        INNATES(
            ABILITY_HEATPROOF,
            ABILITY_LEVITATE
        )
    },
    { // 0437
        SPECIES_BRONZONG,
        INNATES(
            ABILITY_HEATPROOF,
            ABILITY_LEVITATE
        )
    },
    { // 0438
        SPECIES_BONSLY,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0439
        SPECIES_MIME_JR,
        INNATES(
            ABILITY_FILTER,
            ABILITY_TECHNICIAN
        )
    },
    { // 0440
        SPECIES_HAPPINY,
        INNATES(
            ABILITY_FRIEND_GUARD,
            ABILITY_NATURAL_CURE,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0441
        SPECIES_CHATOT,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_TANGLED_FEET
        )
    },
    { // 0442
        SPECIES_SPIRITOMB,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0443
        SPECIES_GIBLE,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0444
        SPECIES_GABITE,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0445
        SPECIES_GARCHOMP,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0445
        SPECIES_GARCHOMP_MEGA,
        INNATES(
            ABILITY_SAND_FORCE,
            ABILITY_SAND_VEIL
        )
    },
    { // 0445
        SPECIES_GARCHOMP_MEGA_Z,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0446
        SPECIES_MUNCHLAX,
        INNATES(
            ABILITY_THICK_FAT,
            ABILITY_UNAWARE
        )
    },
    { // 0447
        SPECIES_RIOLU,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0448
        SPECIES_LUCARIO_MEGA,
        INNATES(
            ABILITY_ADAPTABILITY
        )
    },
    { // 0449
        SPECIES_HIPPOPOTAS,
        INNATES(
            ABILITY_SAND_FORCE
        )
    },
    { // 0450
        SPECIES_HIPPOWDON,
        INNATES(
            ABILITY_SAND_FORCE
        )
    },
    { // 0451
        SPECIES_SKORUPI,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_SNIPER
        )
    },
    { // 0452
        SPECIES_DRAPION,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_SNIPER
        )
    },
    { // 0455
        SPECIES_CARNIVINE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0456
        SPECIES_FINNEON,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0457
        SPECIES_LUMINEON,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0458
        SPECIES_MANTYKE,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0461
        SPECIES_WEAVILE,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0462
        SPECIES_MAGNEZONE,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_LEVITATE,
            ABILITY_STURDY
        )
    },
    { // 0463
        SPECIES_LICKILICKY,
        INNATES(
            ABILITY_OBLIVIOUS
        )
    },
    { // 0464
        SPECIES_RHYPERIOR,
        INNATES(
            ABILITY_RECKLESS,
            ABILITY_SOLID_ROCK
        )
    },
    { // 0465
        SPECIES_TANGROWTH,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_REGENERATOR
        )
    },
    { // 0466
        SPECIES_ELECTIVIRE,
        INNATES(
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0467
        SPECIES_MAGMORTAR,
        INNATES(
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0468
        SPECIES_TOGEKISS,
        INNATES(
            ABILITY_SERENE_GRACE,
            ABILITY_SUPER_LUCK
        )
    },
    { // 0469
        SPECIES_YANMEGA,
        INNATES(
            ABILITY_SPEED_BOOST,
            ABILITY_TINTED_LENS
        )
    },
    { // 0470
        SPECIES_LEAFEON,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0471
        SPECIES_GLACEON,
        INNATES(
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0472
        SPECIES_GLISCOR,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0473
        SPECIES_MAMOSWINE,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_SNOW_CLOAK,
            ABILITY_THICK_FAT
        )
    },
    { // 0474
        SPECIES_PORYGON_Z,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_ANALYTIC,
            ABILITY_LEVITATE
        )
    },
    { // 0475
        SPECIES_GALLADE,
        INNATES(
            ABILITY_SHARPNESS
        )
    },
    { // 0475
        SPECIES_GALLADE_MEGA,
        INNATES(
            ABILITY_SHARPNESS
        )
    },
    { // 0476
        SPECIES_PROBOPASS,
        INNATES(
            ABILITY_SAND_FORCE,
            ABILITY_STURDY
        )
    },
    { // 0477
        SPECIES_DUSKNOIR,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0478
        SPECIES_FROSLASS,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0478
        SPECIES_FROSLASS_MEGA,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0479
        SPECIES_ROTOM,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0479
        SPECIES_ROTOM_HEAT,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0479
        SPECIES_ROTOM_WASH,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0479
        SPECIES_ROTOM_FROST,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0479
        SPECIES_ROTOM_FAN,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0479
        SPECIES_ROTOM_MOW,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0480
        SPECIES_UXIE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0481
        SPECIES_MESPRIT,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0482
        SPECIES_AZELF,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0483
        SPECIES_DIALGA,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0483
        SPECIES_DIALGA_ORIGIN,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0484
        SPECIES_PALKIA,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0484
        SPECIES_PALKIA_ORIGIN,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0487
        SPECIES_GIRATINA_ALTERED,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE
        )
    },
    { // 0487
        SPECIES_GIRATINA_ORIGIN,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0488
        SPECIES_CRESSELIA,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0491
        SPECIES_DARKRAI,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0491
        SPECIES_DARKRAI_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0492
        SPECIES_SHAYMIN_LAND,
        INNATES(
            ABILITY_NATURAL_CURE,
            ABILITY_REGENERATOR,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0492
        SPECIES_SHAYMIN_SKY,
        INNATES(
            ABILITY_NATURAL_CURE,
            ABILITY_REGENERATOR,
            ABILITY_SERENE_GRACE
        )
    },

    // ----- Gen 5 -----
    { // 0495
        SPECIES_SNIVY,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0496
        SPECIES_SERVINE,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0497
        SPECIES_SERPERIOR,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0498
        SPECIES_TEPIG,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_THICK_FAT
        )
    },
    { // 0499
        SPECIES_PIGNITE,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_THICK_FAT
        )
    },
    { // 0500
        SPECIES_EMBOAR,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_RECKLESS
        )
    },
    { // 0500
        SPECIES_EMBOAR_MEGA,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_RECKLESS
        )
    },
    { // 0501
        SPECIES_OSHAWOTT,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_TORRENT
        )
    },
    { // 0502
        SPECIES_DEWOTT,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_TORRENT
        )
    },
    { // 0503
        SPECIES_SAMUROTT,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_TORRENT
        )
    },
    { // 0503
        SPECIES_SAMUROTT_HISUI,
        INNATES(
            ABILITY_SHARPNESS,
            ABILITY_TORRENT
        )
    },
    { // 0504
        SPECIES_PATRAT,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_KEEN_EYE
        )
    },
    { // 0505
        SPECIES_WATCHOG,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_ILLUMINATE,
            ABILITY_KEEN_EYE
        )
    },
    { // 0506
        SPECIES_LILLIPUP,
        INNATES(
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0507
        SPECIES_HERDIER,
        INNATES(
            ABILITY_SAND_RUSH,
            ABILITY_SCRAPPY
        )
    },
    { // 0508
        SPECIES_STOUTLAND,
        INNATES(
            ABILITY_SAND_RUSH,
            ABILITY_SCRAPPY
        )
    },
    { // 0509
        SPECIES_PURRLOIN,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_PRANKSTER
        )
    },
    { // 0510
        SPECIES_LIEPARD,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_PRANKSTER
        )
    },
    { // 0511
        SPECIES_PANSAGE,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0512
        SPECIES_SIMISAGE,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0513
        SPECIES_PANSEAR,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0514
        SPECIES_SIMISEAR,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0515
        SPECIES_PANPOUR,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0516
        SPECIES_SIMIPOUR,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0517
        SPECIES_MUNNA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0518
        SPECIES_MUSHARNA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0519
        SPECIES_PIDOVE,
        INNATES(
            ABILITY_SUPER_LUCK
        )
    },
    { // 0520
        SPECIES_TRANQUILL,
        INNATES(
            ABILITY_SUPER_LUCK
        )
    },
    { // 0521
        SPECIES_UNFEZANT,
        INNATES(
            ABILITY_SUPER_LUCK
        )
    },
    { // 0524
        SPECIES_ROGGENROLA,
        INNATES(
            ABILITY_SAND_FORCE,
            ABILITY_STURDY
        )
    },
    { // 0525
        SPECIES_BOLDORE,
        INNATES(
            ABILITY_SAND_FORCE,
            ABILITY_STURDY
        )
    },
    { // 0526
        SPECIES_GIGALITH,
        INNATES(
            ABILITY_SAND_FORCE,
            ABILITY_STURDY
        )
    },
    { // 0527
        SPECIES_WOOBAT,
        INNATES(
            ABILITY_UNAWARE
        )
    },
    { // 0528
        SPECIES_SWOOBAT,
        INNATES(
            ABILITY_UNAWARE
        )
    },
    { // 0529
        SPECIES_DRILBUR,
        INNATES(
            ABILITY_SAND_FORCE,
            ABILITY_SAND_RUSH
        )
    },
    { // 0530
        SPECIES_EXCADRILL,
        INNATES(
            ABILITY_SAND_FORCE,
            ABILITY_SAND_RUSH
        )
    },
    { // 0530
        SPECIES_EXCADRILL_MEGA,
        INNATES(
            ABILITY_SAND_FORCE,
            ABILITY_SAND_RUSH
        )
    },
    { // 0531
        SPECIES_AUDINO,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0531
        SPECIES_AUDINO_MEGA,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0532
        SPECIES_TIMBURR,
        INNATES(
            ABILITY_GUTS,
            ABILITY_IRON_FIST
        )
    },
    { // 0533
        SPECIES_GURDURR,
        INNATES(
            ABILITY_GUTS,
            ABILITY_IRON_FIST
        )
    },
    { // 0534
        SPECIES_CONKELDURR,
        INNATES(
            ABILITY_GUTS,
            ABILITY_IRON_FIST
        )
    },
    { // 0535
        SPECIES_TYMPOLE,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0536
        SPECIES_PALPITOAD,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0537
        SPECIES_SEISMITOAD,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0538
        SPECIES_THROH,
        INNATES(
            ABILITY_GUTS
        )
    },
    { // 0539
        SPECIES_SAWK,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0540
        SPECIES_SEWADDLE,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_SWARM
        )
    },
    { // 0541
        SPECIES_SWADLOON,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0542
        SPECIES_LEAVANNY,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_SWARM
        )
    },
    { // 0543
        SPECIES_VENIPEDE,
        INNATES(
            ABILITY_SPEED_BOOST,
            ABILITY_SWARM
        )
    },
    { // 0544
        SPECIES_WHIRLIPEDE,
        INNATES(
            ABILITY_SPEED_BOOST,
            ABILITY_SWARM
        )
    },
    { // 0545
        SPECIES_SCOLIPEDE,
        INNATES(
            ABILITY_SPEED_BOOST,
            ABILITY_SWARM
        )
    },
    { // 0545
        SPECIES_SCOLIPEDE_MEGA,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0546
        SPECIES_COTTONEE,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_LEVITATE,
            ABILITY_PRANKSTER
        )
    },
    { // 0547
        SPECIES_WHIMSICOTT,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_LEVITATE,
            ABILITY_PRANKSTER
        )
    },
    { // 0548
        SPECIES_PETILIL,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0549
        SPECIES_LILLIGANT,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0549
        SPECIES_LILLIGANT_HISUI,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0550
        SPECIES_BASCULIN_RED_STRIPED,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_RECKLESS
        )
    },
    { // 0550
        SPECIES_BASCULIN_BLUE_STRIPED,
        INNATES(
            ABILITY_ADAPTABILITY
        )
    },
    { // 0550
        SPECIES_BASCULIN_WHITE_STRIPED,
        INNATES(
            ABILITY_ADAPTABILITY
        )
    },
    { // 0556
        SPECIES_MARACTUS,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0557
        SPECIES_DWEBBLE,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_STURDY
        )
    },
    { // 0558
        SPECIES_CRUSTLE,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_STURDY
        )
    },
    { // 0561
        SPECIES_SIGILYPH,
        INNATES(
            ABILITY_TINTED_LENS,
            ABILITY_WONDER_SKIN
        )
    },
    { // 0562
        SPECIES_YAMASK,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0562
        SPECIES_YAMASK_GALAR,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0563
        SPECIES_COFAGRIGUS,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0564
        SPECIES_TIRTOUGA,
        INNATES(
            ABILITY_SOLID_ROCK,
            ABILITY_STURDY,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0565
        SPECIES_CARRACOSTA,
        INNATES(
            ABILITY_SOLID_ROCK,
            ABILITY_STURDY,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0568
        SPECIES_TRUBBISH,
        INNATES(
            ABILITY_STENCH
        )
    },
    { // 0569
        SPECIES_GARBODOR,
        INNATES(
            ABILITY_STENCH
        )
    },
    { // 0569
        SPECIES_GARBODOR_GMAX,
        INNATES(
            ABILITY_STENCH
        )
    },
    { // 0570
        SPECIES_ZORUA,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0571
        SPECIES_ZOROARK,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0572
        SPECIES_MINCCINO,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_TECHNICIAN
        )
    },
    { // 0573
        SPECIES_CINCCINO,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_TECHNICIAN
        )
    },
    { // 0577
        SPECIES_SOLOSIS,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_REGENERATOR
        )
    },
    { // 0578
        SPECIES_DUOSION,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_REGENERATOR
        )
    },
    { // 0579
        SPECIES_REUNICLUS,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_REGENERATOR
        )
    },
    { // 0580
        SPECIES_DUCKLETT,
        INNATES(
            ABILITY_KEEN_EYE
        )
    },
    { // 0581
        SPECIES_SWANNA,
        INNATES(
            ABILITY_KEEN_EYE
        )
    },
    { // 0582
        SPECIES_VANILLITE,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0583
        SPECIES_VANILLISH,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0584
        SPECIES_VANILLUXE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0585
        SPECIES_DEERLING_SPRING,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0585
        SPECIES_DEERLING_SUMMER,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0585
        SPECIES_DEERLING_AUTUMN,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0585
        SPECIES_DEERLING_WINTER,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0586
        SPECIES_SAWSBUCK_SPRING,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0586
        SPECIES_SAWSBUCK_SUMMER,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0586
        SPECIES_SAWSBUCK_AUTUMN,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0586
        SPECIES_SAWSBUCK_WINTER,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0588
        SPECIES_KARRABLAST,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0589
        SPECIES_ESCAVALIER,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_SWARM
        )
    },
    { // 0590
        SPECIES_FOONGUS,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0591
        SPECIES_AMOONGUSS,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0592
        SPECIES_FRILLISH,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0593
        SPECIES_JELLICENT,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0594
        SPECIES_ALOMOMOLA,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0595
        SPECIES_JOLTIK,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_SWARM
        )
    },
    { // 0596
        SPECIES_GALVANTULA,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_SWARM
        )
    },
    { // 0599
        SPECIES_KLINK,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0600
        SPECIES_KLANG,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0601
        SPECIES_KLINKLANG,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0602
        SPECIES_TYNAMO,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0603
        SPECIES_EELEKTRIK,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0604
        SPECIES_EELEKTROSS,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0604
        SPECIES_EELEKTROSS_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0605
        SPECIES_ELGYEM,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_LEVITATE
        )
    },
    { // 0606
        SPECIES_BEHEEYEM,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_LEVITATE
        )
    },
    { // 0607
        SPECIES_LITWICK,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0608
        SPECIES_LAMPENT,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0609
        SPECIES_CHANDELURE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0609
        SPECIES_CHANDELURE_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0613
        SPECIES_CUBCHOO,
        INNATES(
            ABILITY_SLUSH_RUSH,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0614
        SPECIES_BEARTIC,
        INNATES(
            ABILITY_SLUSH_RUSH,
            ABILITY_SNOW_CLOAK,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0615
        SPECIES_CRYOGONAL,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0616
        SPECIES_SHELMET,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0618
        SPECIES_STUNFISK,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_SAND_VEIL
        )
    },
    { // 0619
        SPECIES_MIENFOO,
        INNATES(
            ABILITY_RECKLESS,
            ABILITY_REGENERATOR
        )
    },
    { // 0620
        SPECIES_MIENSHAO,
        INNATES(
            ABILITY_RECKLESS,
            ABILITY_REGENERATOR
        )
    },
    { // 0622
        SPECIES_GOLETT,
        INNATES(
            ABILITY_IRON_FIST
        )
    },
    { // 0623
        SPECIES_GOLURK,
        INNATES(
            ABILITY_IRON_FIST
        )
    },
    { // 0623
        SPECIES_GOLURK_MEGA,
        INNATES(
            ABILITY_IRON_FIST
        )
    },
    { // 0624
        SPECIES_PAWNIARD,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0625
        SPECIES_BISHARP,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0626
        SPECIES_BOUFFALANT,
        INNATES(
            ABILITY_RECKLESS
        )
    },
    { // 0627
        SPECIES_RUFFLET,
        INNATES(
            ABILITY_KEEN_EYE
        )
    },
    { // 0628
        SPECIES_BRAVIARY,
        INNATES(
            ABILITY_KEEN_EYE
        )
    },
    { // 0628
        SPECIES_BRAVIARY_HISUI,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_TINTED_LENS
        )
    },
    { // 0632
        SPECIES_DURANT,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0635
        SPECIES_HYDREIGON,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0637
        SPECIES_VOLCARONA,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0641
        SPECIES_TORNADUS_INCARNATE,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0641
        SPECIES_TORNADUS_THERIAN,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0642
        SPECIES_THUNDURUS_INCARNATE,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0645
        SPECIES_LANDORUS_INCARNATE,
        INNATES(
            ABILITY_SAND_FORCE
        )
    },
    { // 0646
        SPECIES_KYUREM,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0648
        SPECIES_MELOETTA,
        INNATES(
            ABILITY_SERENE_GRACE
        )
    },
    { // 0648
        SPECIES_MELOETTA_PIROUETTE,
        INNATES(
            ABILITY_SERENE_GRACE
        )
    },

    // ----- Gen 6 -----
    { // 0650
        SPECIES_CHESPIN,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0651
        SPECIES_QUILLADIN,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0652
        SPECIES_CHESNAUGHT,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0652
        SPECIES_CHESNAUGHT_MEGA,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0653
        SPECIES_FENNEKIN,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0654
        SPECIES_BRAIXEN,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0655
        SPECIES_DELPHOX,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0655
        SPECIES_DELPHOX_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0656
        SPECIES_FROAKIE,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0657
        SPECIES_FROGADIER,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0658
        SPECIES_GRENINJA,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0658
        SPECIES_GRENINJA_MEGA,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0661
        SPECIES_FLETCHLING,
        INNATES(
            ABILITY_GALE_WINGS
        )
    },
    { // 0662
        SPECIES_FLETCHINDER,
        INNATES(
            ABILITY_GALE_WINGS
        )
    },
    { // 0663
        SPECIES_TALONFLAME,
        INNATES(
            ABILITY_GALE_WINGS
        )
    },
    { // 0664
        SPECIES_SCATTERBUG,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_SHIELD_DUST
        )
    },
    { // 0666
        SPECIES_VIVILLON,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_SHIELD_DUST
        )
    },
    { // 0674
        SPECIES_PANCHAM,
        INNATES(
            ABILITY_IRON_FIST,
            ABILITY_SCRAPPY
        )
    },
    { // 0676
        SPECIES_PANGORO,
        INNATES(
            ABILITY_IRON_FIST,
            ABILITY_SCRAPPY
        )
    },
    { // 0676
        SPECIES_FURFROU,
        INNATES(
            ABILITY_FUR_COAT
        )
    },
    { // 0676
        SPECIES_FURFROU_HEART,
        INNATES(
            ABILITY_FUR_COAT
        )
    },
    { // 0676
        SPECIES_FURFROU_STAR,
        INNATES(
            ABILITY_FUR_COAT
        )
    },
    { // 0676
        SPECIES_FURFROU_DIAMOND,
        INNATES(
            ABILITY_FUR_COAT
        )
    },
    { // 0676
        SPECIES_FURFROU_DEBUTANTE,
        INNATES(
            ABILITY_FUR_COAT
        )
    },
    { // 0676
        SPECIES_FURFROU_MATRON,
        INNATES(
            ABILITY_FUR_COAT
        )
    },
    { // 0676
        SPECIES_FURFROU_DANDY,
        INNATES(
            ABILITY_FUR_COAT
        )
    },
    { // 0676
        SPECIES_FURFROU_LA_REINE,
        INNATES(
            ABILITY_FUR_COAT
        )
    },
    { // 0676
        SPECIES_FURFROU_KABUKI,
        INNATES(
            ABILITY_FUR_COAT
        )
    },
    { // 0676
        SPECIES_FURFROU_PHARAOH,
        INNATES(
            ABILITY_FUR_COAT
        )
    },
    { // 0677
        SPECIES_ESPURR,
        INNATES(
            ABILITY_KEEN_EYE
        )
    },
    { // 0678
        SPECIES_MEOWSTIC_M,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_PRANKSTER
        )
    },
    { // 0678
        SPECIES_MEOWSTIC_M_MEGA,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0678
        SPECIES_MEOWSTIC_F,
        INNATES(
            ABILITY_KEEN_EYE
        )
    },
    { // 0679
        SPECIES_HONEDGE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0680
        SPECIES_DOUBLADE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0681
        SPECIES_AEGISLASH,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0681
        SPECIES_AEGISLASH_BLADE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0684
        SPECIES_SWIRLIX,
        INNATES(
            ABILITY_SWEET_VEIL
        )
    },
    { // 0685
        SPECIES_SLURPUFF,
        INNATES(
            ABILITY_SWEET_VEIL
        )
    },
    { // 0686
        SPECIES_INKAY,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0688
        SPECIES_BINACLE,
        INNATES(
            ABILITY_SNIPER,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0689
        SPECIES_BARBARACLE,
        INNATES(
            ABILITY_SNIPER,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0689
        SPECIES_BARBARACLE_MEGA,
        INNATES(
            ABILITY_SNIPER,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0690
        SPECIES_SKRELP,
        INNATES(
            ABILITY_ADAPTABILITY
        )
    },
    { // 0691
        SPECIES_DRAGALGE,
        INNATES(
            ABILITY_ADAPTABILITY
        )
    },
    { // 0691
        SPECIES_DRAGALGE_MEGA,
        INNATES(
            ABILITY_ADAPTABILITY
        )
    },
    { // 0692
        SPECIES_CLAUNCHER,
        INNATES(
            ABILITY_MEGA_LAUNCHER
        )
    },
    { // 0693
        SPECIES_CLAWITZER,
        INNATES(
            ABILITY_MEGA_LAUNCHER
        )
    },
    { // 0694
        SPECIES_HELIOPTILE,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0695
        SPECIES_HELIOLISK,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0696
        SPECIES_TYRUNT,
        INNATES(
            ABILITY_STRONG_JAW,
            ABILITY_STURDY
        )
    },
    { // 0697
        SPECIES_TYRANTRUM,
        INNATES(
            ABILITY_STRONG_JAW
        )
    },
    { // 0700
        SPECIES_SYLVEON,
        INNATES(
            ABILITY_CUTE_CHARM
        )
    },
    { // 0701
        SPECIES_HAWLUCHA,
        INNATES(
            ABILITY_LIMBER
        )
    },
    { // 0701
        SPECIES_HAWLUCHA_MEGA,
        INNATES(
            ABILITY_LIMBER
        )
    },
    { // 0703
        SPECIES_CARBINK,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_STURDY
        )
    },
    { // 0705
        SPECIES_SLIGGOO_HISUI,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0706
        SPECIES_GOODRA_HISUI,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0707
        SPECIES_KLEFKI,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRANKSTER
        )
    },
    { // 0708
        SPECIES_PHANTUMP,
        INNATES(
            ABILITY_NATURAL_CURE
        )
    },
    { // 0709
        SPECIES_TREVENANT,
        INNATES(
            ABILITY_NATURAL_CURE
        )
    },
    { // 0710
        SPECIES_PUMPKABOO,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE
        )
    },
    { // 0710
        SPECIES_PUMPKABOO_SMALL,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE
        )
    },
    { // 0710
        SPECIES_PUMPKABOO_LARGE,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE
        )
    },
    { // 0710
        SPECIES_PUMPKABOO_SUPER,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE
        )
    },
    { // 0711
        SPECIES_GOURGEIST,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE
        )
    },
    { // 0711
        SPECIES_GOURGEIST_SMALL,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE
        )
    },
    { // 0711
        SPECIES_GOURGEIST_LARGE,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE
        )
    },
    { // 0711
        SPECIES_GOURGEIST_SUPER,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE
        )
    },
    { // 0712
        SPECIES_BERGMITE,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0713
        SPECIES_AVALUGG,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0713
        SPECIES_AVALUGG_HISUI,
        INNATES(
            ABILITY_STRONG_JAW,
            ABILITY_STURDY
        )
    },
    { // 0718
        SPECIES_ZYGARDE,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0718
        SPECIES_ZYGARDE_MEGA,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0718
        SPECIES_ZYGARDE_10,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0718
        SPECIES_ZYGARDE_COMPLETE,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0719
        SPECIES_DIANCIE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0719
        SPECIES_DIANCIE_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0720
        SPECIES_HOOPA,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRANKSTER
        )
    },
    { // 0720
        SPECIES_HOOPA_UNBOUND,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRANKSTER
        )
    },

    // ----- Gen 7 -----
    { // 0722
        SPECIES_ROWLET,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0723
        SPECIES_DARTRIX,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0724
        SPECIES_DECIDUEYE,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0724
        SPECIES_DECIDUEYE_HISUI,
        INNATES(
            ABILITY_OVERGROW,
            ABILITY_SCRAPPY
        )
    },
    { // 0725
        SPECIES_LITTEN,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0726
        SPECIES_TORRACAT,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0727
        SPECIES_INCINEROAR,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0728
        SPECIES_POPPLIO,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0729
        SPECIES_BRIONNE,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0730
        SPECIES_PRIMARINA,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0731
        SPECIES_PIKIPEK,
        INNATES(
            ABILITY_KEEN_EYE
        )
    },
    { // 0732
        SPECIES_TRUMBEAK,
        INNATES(
            ABILITY_KEEN_EYE
        )
    },
    { // 0733
        SPECIES_TOUCANNON,
        INNATES(
            ABILITY_KEEN_EYE
        )
    },
    { // 0734
        SPECIES_YUNGOOS,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_STAKEOUT,
            ABILITY_STRONG_JAW
        )
    },
    { // 0735
        SPECIES_GUMSHOOS,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_STAKEOUT,
            ABILITY_STRONG_JAW
        )
    },
    { // 0735
        SPECIES_GUMSHOOS_TOTEM,
        INNATES(
            ABILITY_ADAPTABILITY
        )
    },
    { // 0736
        SPECIES_GRUBBIN,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0738
        SPECIES_VIKAVOLT,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0738
        SPECIES_VIKAVOLT_TOTEM,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0739
        SPECIES_CRABRAWLER,
        INNATES(
            ABILITY_IRON_FIST
        )
    },
    { // 0740
        SPECIES_CRABOMINABLE,
        INNATES(
            ABILITY_IRON_FIST
        )
    },
    { // 0740
        SPECIES_CRABOMINABLE_MEGA,
        INNATES(
            ABILITY_IRON_FIST
        )
    },
    { // 0742
        SPECIES_CUTIEFLY,
        INNATES(
            ABILITY_SHIELD_DUST,
            ABILITY_SWEET_VEIL
        )
    },
    { // 0743
        SPECIES_RIBOMBEE,
        INNATES(
            ABILITY_SHIELD_DUST,
            ABILITY_SWEET_VEIL
        )
    },
    { // 0744
        SPECIES_ROCKRUFF,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0745
        SPECIES_LYCANROC_MIDDAY,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_SAND_RUSH
        )
    },
    { // 0745
        SPECIES_LYCANROC_MIDNIGHT,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0745
        SPECIES_LYCANROC_DUSK,
        INNATES(
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0747
        SPECIES_MAREANIE,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_MERCILESS,
            ABILITY_REGENERATOR
        )
    },
    { // 0748
        SPECIES_TOXAPEX,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_MERCILESS,
            ABILITY_REGENERATOR
        )
    },
    { // 0751
        SPECIES_DEWPIDER,
        INNATES(
            ABILITY_WATER_BUBBLE
        )
    },
    { // 0752
        SPECIES_ARAQUANID,
        INNATES(
            ABILITY_WATER_BUBBLE
        )
    },
    { // 0752
        SPECIES_ARAQUANID_TOTEM,
        INNATES(
            ABILITY_WATER_BUBBLE
        )
    },
    { // 0757
        SPECIES_SALANDIT,
        INNATES(
            ABILITY_OBLIVIOUS
        )
    },
    { // 0758
        SPECIES_SALAZZLE,
        INNATES(
            ABILITY_OBLIVIOUS
        )
    },
    { // 0759
        SPECIES_STUFFUL,
        INNATES(
            ABILITY_CUTE_CHARM
        )
    },
    { // 0761
        SPECIES_BOUNSWEET,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_SWEET_VEIL
        )
    },
    { // 0762
        SPECIES_STEENEE,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_SWEET_VEIL
        )
    },
    { // 0763
        SPECIES_TSAREENA,
        INNATES(
            ABILITY_SWEET_VEIL
        )
    },
    { // 0764
        SPECIES_COMFEY,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_NATURAL_CURE,
            ABILITY_TRIAGE
        )
    },
    { // 0769
        SPECIES_SANDYGAST,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0770
        SPECIES_PALOSSAND,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0771
        SPECIES_PYUKUMUKU,
        INNATES(
            ABILITY_UNAWARE
        )
    },
    { // 0772
        SPECIES_TYPE_NULL,
        INNATES(
            ABILITY_BATTLE_ARMOR
        )
    },
    { // 0775
        SPECIES_KOMALA,
        INNATES(
            ABILITY_UNAWARE
        )
    },
    { // 0776
        SPECIES_TURTONATOR,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0777
        SPECIES_TOGEDEMARU,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0779
        SPECIES_BRUXISH,
        INNATES(
            ABILITY_STRONG_JAW,
            ABILITY_WONDER_SKIN
        )
    },
    { // 0781
        SPECIES_DHELMISE,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_STEELWORKER
        )
    },
    { // 0785
        SPECIES_TAPU_KOKO,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0786
        SPECIES_TAPU_LELE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0787
        SPECIES_TAPU_BULU,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0788
        SPECIES_TAPU_FINI,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0789
        SPECIES_COSMOG,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0790
        SPECIES_COSMOEM,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0792
        SPECIES_LUNALA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0793
        SPECIES_NIHILEGO,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0796
        SPECIES_XURKITREE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0798
        SPECIES_KARTANA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0800
        SPECIES_NECROZMA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0800
        SPECIES_NECROZMA_DUSK_MANE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0800
        SPECIES_NECROZMA_DAWN_WINGS,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0800
        SPECIES_NECROZMA_ULTRA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0801
        SPECIES_MAGEARNA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0801
        SPECIES_MAGEARNA_ORIGINAL,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0801
        SPECIES_MAGEARNA_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0801
        SPECIES_MAGEARNA_ORIGINAL_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0802
        SPECIES_MARSHADOW,
        INNATES(
            ABILITY_TECHNICIAN
        )
    },
    { // 0803
        SPECIES_POIPOLE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0806
        SPECIES_BLACEPHALON,
        INNATES(
            ABILITY_LEVITATE
        )
    },

    // ----- Gen 8 -----
    { // 0809
        SPECIES_MELMETAL,
        INNATES(
            ABILITY_IRON_FIST
        )
    },
    { // 0809
        SPECIES_MELMETAL_GMAX,
        INNATES(
            ABILITY_IRON_FIST
        )
    },
    { // 0810
        SPECIES_GROOKEY,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0811
        SPECIES_THWACKEY,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0812
        SPECIES_RILLABOOM,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0812
        SPECIES_RILLABOOM_GMAX,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0813
        SPECIES_SCORBUNNY,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0814
        SPECIES_RABOOT,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0815
        SPECIES_CINDERACE,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0815
        SPECIES_CINDERACE_GMAX,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0816
        SPECIES_SOBBLE,
        INNATES(
            ABILITY_SNIPER,
            ABILITY_TORRENT
        )
    },
    { // 0817
        SPECIES_DRIZZILE,
        INNATES(
            ABILITY_SNIPER,
            ABILITY_TORRENT
        )
    },
    { // 0818
        SPECIES_INTELEON,
        INNATES(
            ABILITY_SNIPER,
            ABILITY_TORRENT
        )
    },
    { // 0818
        SPECIES_INTELEON_GMAX,
        INNATES(
            ABILITY_SNIPER,
            ABILITY_TORRENT
        )
    },
    { // 0821
        SPECIES_ROOKIDEE,
        INNATES(
            ABILITY_KEEN_EYE
        )
    },
    { // 0822
        SPECIES_CORVISQUIRE,
        INNATES(
            ABILITY_KEEN_EYE
        )
    },
    { // 0823
        SPECIES_CORVIKNIGHT,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0823
        SPECIES_CORVIKNIGHT_GMAX,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0824
        SPECIES_BLIPBUG,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_SWARM
        )
    },
    { // 0825
        SPECIES_DOTTLER,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_SWARM
        )
    },
    { // 0826
        SPECIES_ORBEETLE,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0826
        SPECIES_ORBEETLE_GMAX,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0827
        SPECIES_NICKIT,
        INNATES(
            ABILITY_STAKEOUT
        )
    },
    { // 0828
        SPECIES_THIEVUL,
        INNATES(
            ABILITY_STAKEOUT
        )
    },
    { // 0829
        SPECIES_GOSSIFLEUR,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0830
        SPECIES_ELDEGOSS,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0833
        SPECIES_CHEWTLE,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_STRONG_JAW,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0834
        SPECIES_DREDNAW,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_STRONG_JAW,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0834
        SPECIES_DREDNAW_GMAX,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_STRONG_JAW,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0836
        SPECIES_BOLTUND,
        INNATES(
            ABILITY_STRONG_JAW
        )
    },
    { // 0837
        SPECIES_ROLYCOLY,
        INNATES(
            ABILITY_HEATPROOF
        )
    },
    { // 0838
        SPECIES_CARKOL,
        INNATES(
            ABILITY_HEATPROOF
        )
    },
    { // 0839
        SPECIES_COALOSSAL,
        INNATES(
            ABILITY_HEATPROOF
        )
    },
    { // 0839
        SPECIES_COALOSSAL_GMAX,
        INNATES(
            ABILITY_HEATPROOF
        )
    },
    { // 0842
        SPECIES_APPLETUN,
        INNATES(
            ABILITY_THICK_FAT
        )
    },
    { // 0842
        SPECIES_APPLETUN_GMAX,
        INNATES(
            ABILITY_THICK_FAT
        )
    },
    { // 0843
        SPECIES_SILICOBRA,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0844
        SPECIES_SANDACONDA,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0844
        SPECIES_SANDACONDA_GMAX,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0846
        SPECIES_ARROKUDA,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0847
        SPECIES_BARRASKEWDA,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0849
        SPECIES_TOXTRICITY,
        INNATES(
            ABILITY_PUNK_ROCK,
            ABILITY_TECHNICIAN
        )
    },
    { // 0849
        SPECIES_TOXTRICITY_AMPED_GMAX,
        INNATES(
            ABILITY_PUNK_ROCK,
            ABILITY_TECHNICIAN
        )
    },
    { // 0849
        SPECIES_TOXTRICITY_LOW_KEY,
        INNATES(
            ABILITY_PUNK_ROCK,
            ABILITY_TECHNICIAN
        )
    },
    { // 0849
        SPECIES_TOXTRICITY_LOW_KEY_GMAX,
        INNATES(
            ABILITY_PUNK_ROCK,
            ABILITY_TECHNICIAN
        )
    },
    { // 0852
        SPECIES_CLOBBOPUS,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_TECHNICIAN
        )
    },
    { // 0853
        SPECIES_GRAPPLOCT,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_TECHNICIAN
        )
    },
    { // 0854
        SPECIES_SINISTEA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0854
        SPECIES_SINISTEA_ANTIQUE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0855
        SPECIES_POLTEAGEIST,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0855
        SPECIES_POLTEAGEIST_ANTIQUE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0859
        SPECIES_IMPIDIMP,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0860
        SPECIES_MORGREM,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0861
        SPECIES_GRIMMSNARL,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0861
        SPECIES_GRIMMSNARL_GMAX,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0862
        SPECIES_OBSTAGOON,
        INNATES(
            ABILITY_GUTS,
            ABILITY_RECKLESS
        )
    },
    { // 0863
        SPECIES_PERRSERKER,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_STEELY_SPIRIT,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0865
        SPECIES_SIRFETCHD,
        INNATES(
            ABILITY_SCRAPPY
        )
    },
    { // 0866
        SPECIES_MR_RIME,
        INNATES(
            ABILITY_TANGLED_FEET
        )
    },
    { // 0867
        SPECIES_RUNERIGUS,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0868
        SPECIES_MILCERY,
        INNATES(
            ABILITY_SWEET_VEIL
        )
    },
    { // 0870
        SPECIES_FALINKS,
        INNATES(
            ABILITY_BATTLE_ARMOR
        )
    },
    { // 0870
        SPECIES_FALINKS_MEGA,
        INNATES(
            ABILITY_BATTLE_ARMOR
        )
    },
    { // 0872
        SPECIES_SNOM,
        INNATES(
            ABILITY_ICE_SCALES,
            ABILITY_SHIELD_DUST
        )
    },
    { // 0873
        SPECIES_FROSMOTH,
        INNATES(
            ABILITY_ICE_SCALES,
            ABILITY_SHIELD_DUST
        )
    },
    { // 0880
        SPECIES_DRACOZOLT,
        INNATES(
            ABILITY_SAND_RUSH
        )
    },
    { // 0881
        SPECIES_ARCTOZOLT,
        INNATES(
            ABILITY_SLUSH_RUSH
        )
    },
    { // 0882
        SPECIES_DRACOVISH,
        INNATES(
            ABILITY_SAND_RUSH,
            ABILITY_STRONG_JAW
        )
    },
    { // 0883
        SPECIES_ARCTOVISH,
        INNATES(
            ABILITY_SLUSH_RUSH
        )
    },
    { // 0885
        SPECIES_DREEPY,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0886
        SPECIES_DRAKLOAK,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0887
        SPECIES_DRAGAPULT,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0890
        SPECIES_ETERNATUS,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0890
        SPECIES_ETERNATUS_ETERNAMAX,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0894
        SPECIES_REGIELEKI,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0900
        SPECIES_KLEAVOR,
        INNATES(
            ABILITY_SHARPNESS,
            ABILITY_SWARM
        )
    },
    { // 0901
        SPECIES_URSALUNA,
        INNATES(
            ABILITY_GUTS
        )
    },
    { // 0902
        SPECIES_BASCULEGION_M,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0902
        SPECIES_BASCULEGION_F,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0903
        SPECIES_SNEASLER,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0904
        SPECIES_OVERQWIL,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0905
        SPECIES_ENAMORUS_INCARNATE,
        INNATES(
            ABILITY_CUTE_CHARM
        )
    },

    // ----- Gen 9 -----
    { // 0906
        SPECIES_SPRIGATITO,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0907
        SPECIES_FLORAGATO,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0908
        SPECIES_MEOWSCARADA,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0909
        SPECIES_FUECOCO,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_UNAWARE
        )
    },
    { // 0910
        SPECIES_CROCALOR,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_UNAWARE
        )
    },
    { // 0911
        SPECIES_SKELEDIRGE,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_UNAWARE
        )
    },
    { // 0912
        SPECIES_QUAXLY,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0913
        SPECIES_QUAXWELL,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0914
        SPECIES_QUAQUAVAL,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0915
        SPECIES_LECHONK,
        INNATES(
            ABILITY_THICK_FAT
        )
    },
    { // 0916
        SPECIES_OINKOLOGNE_M,
        INNATES(
            ABILITY_THICK_FAT
        )
    },
    { // 0916
        SPECIES_OINKOLOGNE_F,
        INNATES(
            ABILITY_THICK_FAT
        )
    },
    { // 0917
        SPECIES_TAROUNTULA,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_STAKEOUT
        )
    },
    { // 0918
        SPECIES_SPIDOPS,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_STAKEOUT
        )
    },
    { // 0919
        SPECIES_NYMBLE,
        INNATES(
            ABILITY_SWARM,
            ABILITY_TINTED_LENS
        )
    },
    { // 0920
        SPECIES_LOKIX,
        INNATES(
            ABILITY_SWARM,
            ABILITY_TINTED_LENS
        )
    },
    { // 0921
        SPECIES_PAWMI,
        INNATES(
            ABILITY_IRON_FIST,
            ABILITY_NATURAL_CURE
        )
    },
    { // 0922
        SPECIES_PAWMO,
        INNATES(
            ABILITY_IRON_FIST,
            ABILITY_NATURAL_CURE
        )
    },
    { // 0923
        SPECIES_PAWMOT,
        INNATES(
            ABILITY_IRON_FIST,
            ABILITY_NATURAL_CURE
        )
    },
    { // 0925
        SPECIES_MAUSHOLD,
        INNATES(
            ABILITY_FRIEND_GUARD,
            ABILITY_TECHNICIAN
        )
    },
    { // 0925
        SPECIES_MAUSHOLD_FOUR,
        INNATES(
            ABILITY_FRIEND_GUARD,
            ABILITY_TECHNICIAN
        )
    },
    { // 0931
        SPECIES_SQUAWKABILLY,
        INNATES(
            ABILITY_GUTS
        )
    },
    { // 0931
        SPECIES_SQUAWKABILLY_BLUE,
        INNATES(
            ABILITY_GUTS
        )
    },
    { // 0932
        SPECIES_NACLI,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0933
        SPECIES_NACLSTACK,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0934
        SPECIES_GARGANACL,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0942
        SPECIES_MASCHIFF,
        INNATES(
            ABILITY_STAKEOUT
        )
    },
    { // 0943
        SPECIES_MABOSSTIFF,
        INNATES(
            ABILITY_STAKEOUT
        )
    },
    { // 0944
        SPECIES_SHROODLE,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0945
        SPECIES_GRAFAIAI,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0950
        SPECIES_KLAWF,
        INNATES(
            ABILITY_REGENERATOR,
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0951
        SPECIES_CAPSAKID,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_INSOMNIA
        )
    },
    { // 0952
        SPECIES_SCOVILLAIN,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_INSOMNIA
        )
    },
    { // 0952
        SPECIES_SCOVILLAIN_MEGA,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_INSOMNIA
        )
    },
    { // 0953
        SPECIES_RELLOR,
        INNATES(
            ABILITY_COMPOUND_EYES
        )
    },
    { // 0955
        SPECIES_FLITTLE,
        INNATES(
            ABILITY_SPEED_BOOST
        )
    },
    { // 0956
        SPECIES_ESPATHRA,
        INNATES(
            ABILITY_SPEED_BOOST
        )
    },
    { // 0960
        SPECIES_WIGLETT,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0961
        SPECIES_WUGTRIO,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0962
        SPECIES_BOMBIRDIER,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_ROCKY_PAYLOAD
        )
    },
    { // 0965
        SPECIES_REVAVROOM,
        INNATES(
            ABILITY_FILTER
        )
    },
    { // 0967
        SPECIES_CYCLIZAR,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0968
        SPECIES_ORTHWORM,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0970
        SPECIES_GLIMMORA_MEGA,
        INNATES(
            ABILITY_ADAPTABILITY
        )
    },
    { // 0972
        SPECIES_HOUNDSTONE,
        INNATES(
            ABILITY_SAND_RUSH
        )
    },
    { // 0973
        SPECIES_FLAMIGO,
        INNATES(
            ABILITY_SCRAPPY,
            ABILITY_TANGLED_FEET
        )
    },
    { // 0974
        SPECIES_CETODDLE,
        INNATES(
            ABILITY_SNOW_CLOAK,
            ABILITY_THICK_FAT
        )
    },
    { // 0975
        SPECIES_CETITAN,
        INNATES(
            ABILITY_SLUSH_RUSH,
            ABILITY_THICK_FAT
        )
    },
    { // 0976
        SPECIES_VELUZA,
        INNATES(
            ABILITY_SHARPNESS
        )
    },
    { // 0977
        SPECIES_DONDOZO,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_UNAWARE
        )
    },
    { // 0980
        SPECIES_CLODSIRE,
        INNATES(
            ABILITY_REGENERATOR,
            ABILITY_UNAWARE
        )
    },
    { // 0982
        SPECIES_DUDUNSPARCE,
        INNATES(
            ABILITY_SERENE_GRACE
        )
    },
    { // 0982
        SPECIES_DUDUNSPARCE_THREE_SEGMENT,
        INNATES(
            ABILITY_SERENE_GRACE
        )
    },
    { // 0983
        SPECIES_KINGAMBIT,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0987
        SPECIES_FLUTTER_MANE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0994
        SPECIES_IRON_MOTH,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0999
        SPECIES_GIMMIGHOUL_ROAMING,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 1008
        SPECIES_MIRAIDON,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 1012
        SPECIES_POLTCHAGEIST,
        INNATES(
            ABILITY_HEATPROOF,
            ABILITY_LEVITATE
        )
    },
    { // 1012
        SPECIES_POLTCHAGEIST_ARTISAN,
        INNATES(
            ABILITY_HEATPROOF,
            ABILITY_LEVITATE
        )
    },
    { // 1013
        SPECIES_SINISTCHA,
        INNATES(
            ABILITY_HEATPROOF,
            ABILITY_LEVITATE
        )
    },
    { // 1013
        SPECIES_SINISTCHA_MASTERPIECE,
        INNATES(
            ABILITY_HEATPROOF,
            ABILITY_LEVITATE
        )
    },
    { // 1016
        SPECIES_FEZANDIPITI,
        INNATES(
            ABILITY_TECHNICIAN
        )
    },
    { // 1017
        SPECIES_OGERPON_CORNERSTONE,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 1018
        SPECIES_ARCHALUDON,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 1019
        SPECIES_HYDRAPPLE,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 1025
        SPECIES_PECHARUNT,
        INNATES(
            ABILITY_LEVITATE
        )
    },
};

// FORK: sublinear species->row lookup. The source table above stays sorted by National
// Dex number for humans (forms sit beside their base), which is NOT species-id order
// (form constants live at high ids), so it can't be binary-searched directly. Instead a
// row-index permutation sorted by species id is built lazily on first lookup (EWRAM bss,
// ~1 KB) and binary-searched thereafter. This lookup backs every SpeciesHasInnate /
// IsInnateActive call in the AI-hot battle calcs when the feature is ON, where the old
// linear walk of the whole table (~500 rows) was a real per-eval cost that CI never
// measures (tests force the feature off). Insertion sort is near-O(n) here because dex
// order is nearly species order — only form rows travel. Binary-search "any match" equals
// the documented "first match" because the "no species appears more than once" integrity
// test (test/fork/innate_abilities.c) forbids duplicate rows; the "species-keyed lookup
// matches the raw table" test guards this index against the raw rows.
// EWRAM_DATA is load-bearing: plain C statics' .bss lands in IWRAM (ld_script_modern.ld),
// where ~1 KB collides with the stack and corrupts memory (heap-magic asserts in malloc.c).
static EWRAM_DATA u16 sRowIndexSortedBySpecies[ARRAY_COUNT(sSpeciesInnates)] = {0};
static EWRAM_DATA bool8 sRowIndexBuilt = FALSE;

static void BuildRowIndexSortedBySpecies(void)
{
    u32 i, j;

    for (i = 0; i < ARRAY_COUNT(sSpeciesInnates); i++)
    {
        u16 species = sSpeciesInnates[i].species;

        for (j = i; j > 0 && sSpeciesInnates[sRowIndexSortedBySpecies[j - 1]].species > species; j--)
            sRowIndexSortedBySpecies[j] = sRowIndexSortedBySpecies[j - 1];
        sRowIndexSortedBySpecies[j] = i;
    }

    sRowIndexBuilt = TRUE;
}

static const enum Ability *GetSpeciesInnateList(u16 species)
{
    u32 lo, hi;

    if (!sRowIndexBuilt)
        BuildRowIndexSortedBySpecies();

    lo = 0;
    hi = ARRAY_COUNT(sSpeciesInnates);
    while (lo < hi)
    {
        u32 mid = (lo + hi) / 2;
        const struct SpeciesInnates *row = &sSpeciesInnates[sRowIndexSortedBySpecies[mid]];

        if (row->species == species)
            return row->innates;
        if (row->species < species)
            lo = mid + 1;
        else
            hi = mid;
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

// FORK: raw-table accessors for table-integrity tests (test/fork/innate_abilities.c).
// These walk the sSpeciesInnates rows directly (NOT keyed by species), so a duplicate
// species row — invisible to GetSpeciesInnateList, which returns the first match — is
// still observable. Not for battle logic: use SpeciesHasInnate / GetSpeciesInnate there.
u32 GetSpeciesInnatesEntryCount(void)
{
    return ARRAY_COUNT(sSpeciesInnates);
}

const enum Ability *GetSpeciesInnatesEntry(u32 row, u16 *outSpecies)
{
    if (row >= ARRAY_COUNT(sSpeciesInnates))
        return NULL;
    if (outSpecies != NULL)
        *outSpecies = sSpeciesInnates[row].species;
    return sSpeciesInnates[row].innates;
}

// Active, scripted innate abilities that fire at the end of every turn. Today only
// Speed Boost (raises Speed +1). Add a future end-turn active here; the driver
// (TryActivateInnateEndTurnEffects) is already re-entrant, so a battler may carry
// more than one and each fires in turn.
static bool32 IsActiveEndTurnInnate(enum Ability ability)
{
    return ability == ABILITY_SPEED_BOOST;
}

// FORK: end-turn innate driver (FEATURE_INNATE_ABILITIES). Fires the holder's active,
// scripted end-turn innates (today only Speed Boost), hooked from the
// THIRD_EVENT_BLOCK_ABILITIES_INNATE step of the end-turn loop (src/battle_end_turn.c)
// right after the chosen-ability end-turn block.
//
// RE-ENTRANT: a battle script fires one at a time, so this resumes from a per-battler
// cursor. *index is the next innate-list slot to consider; the end-turn loop holds the
// THIRD_EVENT_BLOCK_ABILITIES_INNATE step (keeping the cursor) while this returns TRUE,
// and only advances the block once it returns FALSE (list exhausted). The caller resets
// the cursor to 0 for the next battler. Each fired effect leaves *index pointing past it,
// so a battler with several active end-turn innates fires them across successive turns of
// the loop. Returns TRUE if an effect fired this call.
//
// The effect is delegated to the upstream end-turn ability handler with the innate
// passed explicitly: AbilityBattleEffects(ABILITYEFFECT_ENDTURN, battler, innate, ...)
// sets gLastUsedAbility = innate and runs that ability's existing case, so the stat
// change / script / pop-up match the real ability exactly (the pop-up is overridden to
// show the innate at the Speed Boost effect site in src/battle_util.c, but only when the
// chosen ability differs). An innate equal to the chosen ability is skipped so the
// chosen-ability block (which already ran it) never boosts twice; IsInnateActive() applies
// the usual suppression (feature flag, Gastro Acid, Neutralizing Gas, not-on-field). An
// eligible innate that does nothing this turn (e.g. Speed already maxed) is stepped over
// without firing, so the scan continues to the battler's next end-turn innate.
bool32 TryActivateInnateEndTurnEffects(enum BattlerId battler, u32 *index)
{
    enum Ability innate;

    while ((innate = GetSpeciesInnate(gBattleMons[battler].species, *index)) != ABILITY_NONE)
    {
        (*index)++; // step past this slot now, so a fired effect resumes at the next one
        if (!IsActiveEndTurnInnate(innate))
            continue;
        if (GetBattlerAbility(battler) == innate) // chosen-ability end-turn block already ran it
            continue;
        if (!IsInnateActive(battler, innate))
            continue;
        if (AbilityBattleEffects(ABILITYEFFECT_ENDTURN, battler, innate, MOVE_NONE, TRUE))
            return TRUE;
    }

    return FALSE;
}
