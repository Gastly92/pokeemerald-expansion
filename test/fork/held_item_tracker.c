#include "global.h"
#include "test/test.h"
#include "item.h"
#include "pokemon.h"
#include "fork/frontier_extended_mons.h"
#include "constants/items.h"
#include "constants/form_change_types.h"

// FORK: the held-item progress tracker. Two things are worth tracking about a held item,
// and this file gates both:
//
//   1. BALANCE  -- is the item at a good power level? Some arrive there for free
//      (Leftovers), some needed a BUFF_* flag (Shell Bell), and some are still
//      dominated by something else in the slot (the Gems, since BUFF_TYPE_BOOST_ITEMS
//      moved type items to +40% while a Gem is +30% once).
//   2. DISTRIBUTION -- is it actually reachable in the extended frontier roster? The
//      Factory draft rejects a candidate whose held item is already on the team
//      (src/battle_frontier.c, "Ensure this Pokemon's held item isn't a duplicate"), so
//      only ONE of each item can appear per team. A heavily-used item is therefore
//      drafted LESS often -- it loses every roll where another mon already took it --
//      and an item on zero sets is unreachable no matter how good it is.
//
// Both axes collapse into TWO lists. An item's list IS its status, and every tracked item
// is gated by one list or the other -- there is no unwatched middle any more:
//
//   sDoneItems[]    -- balance is right AND it is live in the roster. Gated both ways:
//                      at least one set holds it (more than one unless one is its ceiling),
//                      and it does not exceed HELD_ITEM_MAX_ROSTER_SHARE_PERCENT.
//   sIgnoredItems[] -- NO SET HOLDS IT, and that is enforced. Two kinds of entry qualify,
//                      and the list deliberately does not distinguish them, because the
//                      gate is the same either way:
//                        (a) nothing it does is reachable in a frontier battle -- the
//                            effect cannot happen here (Exp. Share), or its only legal
//                            holder cannot use the effect it has (Lucky Punch on Chansey);
//                        (b) the effect works fine and we have decided not to draft it
//                            anyway (Ring Target, whose effect is a pure drawback to its
//                            own holder; the Clamperl pair, which would cost four NFE sets).
//                      Either way no set may hold one: for (a) that would play an item down
//                      for free, and for (b) it would silently undo a deliberate call.
//
// THIS FILE USED TO CARRY A THIRD LIST, sPendingItems[], for "work outstanding". It was
// removed once the audit closed, and the reason is worth keeping: it was the one list with
// NO GATE OF ITS OWN. Nothing asserted anything about a pending item, so an item parked
// there was unwatched -- which is precisely the "we decided something and nothing checked
// it" failure this file exists to catch, reappearing inside the file itself. Work that is
// genuinely outstanding belongs in an issue or a branch, not in an ungated array here.
// To re-draft an ignored item, MOVE IT TO sDoneItems[] in the same commit that gives it
// sets; the no-set gate below makes that a deliberate, reviewed edit rather than a drift.
//
// The point of the split is that graduating an item to sDoneItems[] is what ARMS the
// gates for it. That is the failure this file exists to catch: Wide Lens, Zoom Lens,
// Blunder Policy, Razor Fang and Lansat Berry all received real engine work and then
// shipped to nobody, because nothing connected "we buffed it" to "a set holds it". All
// five are drafted and gated now, so they are this file's worked example rather than an
// open bug -- but the hole they fell through is still the reason the split exists.
//
// The lists are swept for completeness against gItemsInfo[] (see the first test), so an
// item arriving with an upstream sync cannot sit unclassified. Two whole hold-effect
// CLASSES are ignored by rule rather than by 127 boilerplate rows -- see
// GetItemTrackerList().
//
// Rationale, the per-item verdicts and the buff sketches behind them live in
// fork-docs/HELD_ITEMS.md. This file is the status; that doc is the reasoning.

// A done item may not exceed this share of the roster. Leftovers, the most-used item,
// currently sits at ~14%. This is a RATCHET: tighten it as the tail thickens, never
// loosen it to make a red build go green -- a breach means the roster is concentrating,
// and the fix is to move sets onto the long tail (fork-docs/HELD_ITEMS.md, Groups A/D).
#define HELD_ITEM_MAX_ROSTER_SHARE_PERCENT 20

// The done list never shrinks. Bump this when items graduate; a drop means an item stopped
// being drafted, which is a real regression and should be a deliberate, reviewed act rather
// than a quiet way to dodge one of the gates above. A demotion now means moving the item to
// sIgnoredItems[], since that is the only other list.
//
// The entries below are a changelog, so the early ones still talk about sPendingItems[] and
// about items being "pending". That list was removed once the audit closed (see the header);
// read those as "not drafted at the time".
//
// 122 -> 121 is one such reviewed demotion: Dragon Fang. Drafting Soul Dew took the Latios
// set that was Dragon Fang's SECOND home, leaving it at one -- the gates did not catch it
// because "every done item appears on at least one set" only requires one. Its class-mates
// Miracle Seed and Silver Powder are already pending at one set, so this puts the generic
// type items back on one reading. Re-graduate it with the second set, not by raising this.
//
// 121 -> 120 is the second: Griseous Orb, found by the single-set gate below on its first
// run. It had been filed with the form-change enablers, but it only changes Giratina's
// forme when I_GRISEOUS_ORB_FORM_CHANGE < GEN_9 and this build sets GEN_LATEST, so that
// entry is compiled out of sGiratinaFormChangeTable and GRISEOUS_CORE is the enabler here.
// That makes the Orb a plain signature type item whose real class-mates are Adamant Orb and
// Lustrous Orb, both pending at zero sets. The roster's Giratina-Origin set still works --
// it names SPECIES_GIRATINA_ORIGIN directly, so the item was never what got it there.
// 120 -> 126: the six incenses, drafted two sets each. Five are HOLD_EFFECT_TYPE_POWER
// duplicates of items the roster already leans on (BUFF_TYPE_BOOST_ITEMS gives them the
// same +40% as Charcoal), and Lax Incense is byte-identical to Bright Powder under the PP
// economy -- GetDeterministicMoveTargetPPTax() only does tax++ for HOLD_EFFECT_EVASION_UP
// and never reads the item's param. No balance question in the batch, only placement.
// 126 -> 135: nine resist berries, two sets each. Every home is a genuine 4x weakness the
// berry halves back to 2x, screened against the two ways such a placement ships dead -- a
// set ability or an INNATE making the holder immune to the berry's own type (this cost
// Scizor and Ferrothorn their Occa Berry: both run Well-Baked Body, so a Fire hit never
// lands to be resisted), and a move that makes the old item the point of the set. The
// Yache pair is the standout: Appletun and Flapple are Grass/Dragon with innate RIPEN,
// which doubles the reduction to 0.25x, so a 4x Ice hit comes through fully neutral.
//
// 135 -> 144 finishes the class, and the last two berries could not use the 4x rule at all.
// Nothing in the game is 4x weak to Dragon, so HABAN's ceiling is a 2x Dragon-type holder
// (Goodra, Kingdra). CHILAN is the mirror image: no type is weak to Normal either, but its
// trigger is special-cased to fire on ANY Normal hit (moveType == TYPE_NORMAL in
// GetDefenderItemsModifier), so it wants a holder that expects Normal damage rather than a
// weakness -- and the screen still bites, because a Ghost type is Normal-IMMUNE and would
// make it dead. Blissey is the case the item could have been written for: 255 base HP
// behind 10 base Defense, against a roster carrying 126 physical-Normal move instances.
// 144 -> 161 finishes the Gems: every one of the 18 types now has at least two sets.
// BUFF_GEMS settled the balance at +60% long ago; this was purely the placement half, and
// the settled rule is that a Gem goes on a move the set fires ONCE -- a self-debuffing nuke
// (Overheat, Leaf Storm, Draco Meteor, Psycho Boost), a literal one-use move (Explosion), or
// true off-STAB coverage -- never the set's main STAB, and never a Contrary user, who spams
// the nuke and wants a permanent item instead.
//
// One screen is specific to this class and it drew blood: an -ate ability RE-TYPES the move
// out from under the Gem. Golem-Alola was the obvious Explosion home for the Normal Gem and
// its Galvanize turns Explosion Electric, so the Gem would never have fired; plain Golem
// took it instead. Check Galvanize, Pixilate, Refrigerate, Aerilate and Normalize -- on
// innates as well as the chosen ability -- before putting a Gem on a Normal move.
// 161 -> 176 clears the thinly-drafted block entirely: all 13 items that sat at a single set
// got a second one, and Oran Berry and Berry Juice were drafted for the first time now that
// BUFF_FLAT_HP_ITEMS put them on Sitrus Berry's number. Nothing is left in this tracker between
// "held by nobody" and "done" -- every remaining pending item is at zero sets.
// 176 -> 215 finishes the audit. Every held item that is live in a frontier battle and not
// parked now sits on at least two sets.
// 215 -> 218 gives each Sinnoh creation-trio forme its own signature item, after the park on
// the three orbs turned out to rest on arithmetic that was wrong in both directions.
// The floor stops here, and the three items still undrafted moved to sIgnoredItems[] rather
// than staying on an ungated list. Every tracked item is now gated by one list or the other,
// so a change in either direction has to say so out loud.
#define HELD_ITEM_DONE_FLOOR 218

// Balance is right AND the roster uses it. Both gates below apply to every entry here.
static const enum Item sDoneItems[] =
{
    ITEM_ABILITY_SHIELD,
    ITEM_ABSORB_BULB,
    ITEM_ADAMANT_CRYSTAL,
    ITEM_ADAMANT_ORB,
    ITEM_ADRENALINE_ORB,
    ITEM_AGUAV_BERRY,
    ITEM_AIR_BALLOON,
    ITEM_APICOT_BERRY,
    ITEM_ASPEAR_BERRY,
    ITEM_ASSAULT_VEST,
    ITEM_BABIRI_BERRY,
    ITEM_BERRY_JUICE,
    ITEM_BERSERK_GENE,
    ITEM_BIG_ROOT,
    ITEM_BINDING_BAND,
    ITEM_BLACK_BELT,
    ITEM_BLACK_GLASSES,
    ITEM_BLACK_SLUDGE,
    ITEM_BLUE_ORB,
    ITEM_BLUNDER_POLICY,
    ITEM_BOOSTER_ENERGY,
    ITEM_BRIGHT_POWDER,
    ITEM_BUG_GEM,
    ITEM_BUG_MEMORY,
    ITEM_BURN_DRIVE,
    ITEM_CELL_BATTERY,
    ITEM_CHARCOAL,
    ITEM_CHARTI_BERRY,
    ITEM_CHERI_BERRY,
    ITEM_CHESTO_BERRY,
    ITEM_CHILAN_BERRY,
    ITEM_CHILL_DRIVE,
    ITEM_CHOICE_BAND,
    ITEM_CHOICE_SCARF,
    ITEM_CHOICE_SPECS,
    ITEM_CHOPLE_BERRY,
    ITEM_CLEAR_AMULET,
    ITEM_COBA_BERRY,
    ITEM_COLBUR_BERRY,
    ITEM_CORNERSTONE_MASK,
    ITEM_COVERT_CLOAK,
    ITEM_CUSTAP_BERRY,
    ITEM_DAMP_ROCK,
    ITEM_DARK_GEM,
    ITEM_DARK_MEMORY,
    ITEM_DOUSE_DRIVE,
    ITEM_DRACO_PLATE,
    ITEM_DRAGON_FANG,
    ITEM_DRAGON_GEM,
    ITEM_DRAGON_MEMORY,
    ITEM_DREAD_PLATE,
    ITEM_EARTH_PLATE,
    ITEM_EJECT_BUTTON,
    ITEM_EJECT_PACK,
    ITEM_ELECTRIC_GEM,
    ITEM_ELECTRIC_MEMORY,
    ITEM_ELECTRIC_SEED,
    ITEM_ENIGMA_BERRY,
    ITEM_EVIOLITE,
    ITEM_EXPERT_BELT,
    ITEM_FAIRY_FEATHER,
    ITEM_FAIRY_GEM,
    ITEM_FAIRY_MEMORY,
    ITEM_FIGHTING_GEM,
    ITEM_FIGHTING_MEMORY,
    ITEM_FIGY_BERRY,
    ITEM_FIRE_GEM,
    ITEM_FIRE_MEMORY,
    ITEM_FIST_PLATE,
    ITEM_FLAME_ORB,
    ITEM_FLAME_PLATE,
    ITEM_FLOAT_STONE,
    ITEM_FLYING_GEM,
    ITEM_FLYING_MEMORY,
    ITEM_FOCUS_BAND,
    ITEM_FOCUS_SASH,
    ITEM_FULL_INCENSE,
    ITEM_GANLON_BERRY,
    ITEM_GHOST_GEM,
    ITEM_GHOST_MEMORY,
    ITEM_GRASSY_SEED,
    ITEM_GRASS_GEM,
    ITEM_GRASS_MEMORY,
    ITEM_GRIP_CLAW,
    ITEM_GRISEOUS_CORE,
    ITEM_GRISEOUS_ORB,
    ITEM_GROUND_GEM,
    ITEM_GROUND_MEMORY,
    ITEM_HABAN_BERRY,
    ITEM_HARD_STONE,
    ITEM_HEARTHFLAME_MASK,
    ITEM_HEAT_ROCK,
    ITEM_HEAVY_DUTY_BOOTS,
    ITEM_IAPAPA_BERRY,
    ITEM_ICE_GEM,
    ITEM_ICE_MEMORY,
    ITEM_ICICLE_PLATE,
    ITEM_ICY_ROCK,
    ITEM_INSECT_PLATE,
    ITEM_IRON_BALL,
    ITEM_IRON_PLATE,
    ITEM_JABOCA_BERRY,
    ITEM_KASIB_BERRY,
    ITEM_KEBIA_BERRY,
    ITEM_KEE_BERRY,
    ITEM_KINGS_ROCK,
    ITEM_LAGGING_TAIL,
    ITEM_LANSAT_BERRY,
    ITEM_LAX_INCENSE,
    ITEM_LEEK,
    ITEM_LEFTOVERS,
    ITEM_LEPPA_BERRY,
    ITEM_LIECHI_BERRY,
    ITEM_LIFE_ORB,
    ITEM_LIGHT_BALL,
    ITEM_LIGHT_CLAY,
    ITEM_LOADED_DICE,
    ITEM_LUMINOUS_MOSS,
    ITEM_LUM_BERRY,
    ITEM_LUSTROUS_GLOBE,
    ITEM_LUSTROUS_ORB,
    ITEM_MAGNET,
    ITEM_MAGO_BERRY,
    ITEM_MARANGA_BERRY,
    ITEM_MEADOW_PLATE,
    ITEM_MENTAL_HERB,
    ITEM_METAL_COAT,
    ITEM_METRONOME,
    ITEM_MICLE_BERRY,
    ITEM_MIND_PLATE,
    ITEM_MIRACLE_SEED,
    ITEM_MIRROR_HERB,
    ITEM_MISTY_SEED,
    ITEM_MUSCLE_BAND,
    ITEM_MYSTIC_WATER,
    ITEM_NEVER_MELT_ICE,
    ITEM_NORMAL_GEM,
    ITEM_OCCA_BERRY,
    ITEM_ODD_INCENSE,
    ITEM_ORAN_BERRY,
    ITEM_PASSHO_BERRY,
    ITEM_PAYAPA_BERRY,
    ITEM_PECHA_BERRY,
    ITEM_PERSIM_BERRY,
    ITEM_PETAYA_BERRY,
    ITEM_PIXIE_PLATE,
    ITEM_POISON_BARB,
    ITEM_POISON_GEM,
    ITEM_POISON_MEMORY,
    ITEM_POWER_HERB,
    ITEM_PROTECTIVE_PADS,
    ITEM_PSYCHIC_GEM,
    ITEM_PSYCHIC_MEMORY,
    ITEM_PSYCHIC_SEED,
    ITEM_PUNCHING_GLOVE,
    ITEM_QUICK_CLAW,
    ITEM_RAWST_BERRY,
    ITEM_RAZOR_CLAW,
    ITEM_RAZOR_FANG,
    ITEM_RED_CARD,
    ITEM_RED_ORB,
    ITEM_RINDO_BERRY,
    ITEM_ROCKY_HELMET,
    ITEM_ROCK_GEM,
    ITEM_ROCK_INCENSE,
    ITEM_ROCK_MEMORY,
    ITEM_ROOM_SERVICE,
    ITEM_ROSELI_BERRY,
    ITEM_ROSE_INCENSE,
    ITEM_ROWAP_BERRY,
    ITEM_RUSTED_SHIELD,
    ITEM_RUSTED_SWORD,
    ITEM_SAFETY_GOGGLES,
    ITEM_SALAC_BERRY,
    ITEM_SCOPE_LENS,
    ITEM_SEA_INCENSE,
    ITEM_SHARP_BEAK,
    ITEM_SHED_SHELL,
    ITEM_SHELL_BELL,
    ITEM_SHOCK_DRIVE,
    ITEM_SHUCA_BERRY,
    ITEM_SILK_SCARF,
    ITEM_SILVER_POWDER,
    ITEM_SITRUS_BERRY,
    ITEM_SKY_PLATE,
    ITEM_SMOOTH_ROCK,
    ITEM_SNOWBALL,
    ITEM_SOFT_SAND,
    ITEM_SOUL_DEW,
    ITEM_SPELL_TAG,
    ITEM_SPLASH_PLATE,
    ITEM_SPOOKY_PLATE,
    ITEM_STARF_BERRY,
    ITEM_STEEL_GEM,
    ITEM_STEEL_MEMORY,
    ITEM_STICKY_BARB,
    ITEM_STONE_PLATE,
    ITEM_TANGA_BERRY,
    ITEM_TERRAIN_EXTENDER,
    ITEM_THICK_CLUB,
    ITEM_THROAT_SPRAY,
    ITEM_TOXIC_ORB,
    ITEM_TOXIC_PLATE,
    ITEM_TWISTED_SPOON,
    ITEM_UTILITY_UMBRELLA,
    ITEM_WACAN_BERRY,
    ITEM_WATER_GEM,
    ITEM_WATER_MEMORY,
    ITEM_WAVE_INCENSE,
    ITEM_WEAKNESS_POLICY,
    ITEM_WELLSPRING_MASK,
    ITEM_WHITE_HERB,
    ITEM_WIDE_LENS,
    ITEM_WIKI_BERRY,
    ITEM_WISE_GLASSES,
    ITEM_YACHE_BERRY,
    ITEM_ZAP_PLATE,
    ITEM_ZOOM_LENS,
};

// NO SET HOLDS THESE, and the last test enforces it. The list answers one question only --
// "is this item drafted?" -- and deliberately does NOT record why not, because the gate is
// the same for every entry. Two kinds of entry qualify.
//
// (a) Nothing it does is reachable in a frontier battle, so neither axis means anything.
//     Note the test is "does it do anything here", not "can it exist here": Exp. Share is
//     perfectly obtainable, it just has no battle effect.
//
// (b) The effect works fine and we have decided not to draft it anyway. These used to live
//     on a separate sPendingItems[] list under the name "parked"; that list is gone, because
//     it was ungated and so nothing watched them. Holding one of these is not a free item
//     slot the way (a) is -- it is undoing a deliberate call -- but it is still an edit that
//     should be reviewed rather than drift, which is exactly what the no-set gate makes it.
//     To start drafting one, move it to sDoneItems[] in the commit that gives it sets.
//       Ring Target    -- the one item in the build whose effect is a PURE DRAWBACK to its
//                         own holder: it turns the holder's type immunities into neutral
//                         damage (MulByTypeEffectiveness, src/battle_util.c, which keys on
//                         ctx->battlerDef). It exists in the retail games to be handed to an
//                         opponent via Trick or Fling, which no set here does. Drafting it
//                         means deliberately making a set worse. Note it is also a no-op on
//                         11 of the 18 types, which have no immunity to lose -- so it is
//                         dead or negative, never positive, on every possible holder.
//       Deep Sea Tooth -- Clamperl only, and both are genuinely enormous (2x). The price is
//       Deep Sea Scale    the holder, not the item: Clamperl has no set because it evolves,
//                         so drafting both means four Clamperl entries -- a 35/64/85/74/55/32
//                         NFE, four times, in a uniform draw -- while Huntail and Gorebyss
//                         already carry four sets between them. A line review that wants an
//                         NFE gimmick can still pick them up; it just has to promote them
//                         here rather than quietly adding a set.
//
// Three species-locked items fail (a) from the other direction --
// the effect is real, but the one holder allowed to have it cannot use it:
//   Lucky Punch  -- Chansey only. +2 crit stage, which DETERMINISTIC_HOLD_EFFECTS upgrades
//                   to a guaranteed first-attack crit (IsCriticalHit, src/battle_util.c).
//                   Chansey has 5 base Attack and 35 base Sp. Atk, and attacks with Seismic
//                   Toss, whose fixed damage a crit does not scale -- so the crit lands and
//                   changes nothing.
//   Metal Powder -- Ditto only, and both gate on an UNTRANSFORMED Ditto (the
//   Quick Powder    !volatiles.transformed checks in src/battle_util.c and src/battle_main.c).
//                   The roster's Ditto runs Imposter, which transforms on switch-in, so
//                   neither item is ever live for even one turn.
// The two big structural classes -- Mega Stones and Z-Crystals, item-free under
// FEATURE_FREE_GIMMICKS -- are excluded by hold effect in GetItemTrackerList() instead
// of listed here. What remains is the out-of-battle utility: nothing they do is
// reachable in a battle. (The Power items' Speed halving IS reachable, but a Trick Room
// set gets the same result for free with IVS(SPE, 0).)
static const enum Item sIgnoredItems[] =
{
    ITEM_AMULET_COIN,
    ITEM_CLEANSE_TAG,
    ITEM_DEEP_SEA_SCALE,
    ITEM_DEEP_SEA_TOOTH,
    ITEM_DESTINY_KNOT,
    ITEM_EVERSTONE,
    ITEM_EXP_SHARE,
    ITEM_LUCKY_EGG,
    ITEM_LUCKY_PUNCH,
    ITEM_LUCK_INCENSE,
    ITEM_MACHO_BRACE,
    ITEM_METAL_POWDER,
    ITEM_POWER_ANKLET,
    ITEM_POWER_BAND,
    ITEM_POWER_BELT,
    ITEM_POWER_BRACER,
    ITEM_POWER_LENS,
    ITEM_POWER_WEIGHT,
    ITEM_PURE_INCENSE,
    ITEM_QUICK_POWDER,
    ITEM_RING_TARGET,
    ITEM_SMOKE_BALL,
    ITEM_SOOTHE_BELL,
};

enum ItemTrackerList
{
    TRACKER_UNLISTED,
    TRACKER_DONE,
    TRACKER_IGNORED,
};

static bool32 ItemIsInList(const enum Item *list, u32 count, enum Item item)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        if (list[i] == item)
            return TRUE;
    }
    return FALSE;
}

// Resolves an item to its tracker list. Two hold-effect CLASSES are ignored by rule
// rather than by listing their members: FEATURE_FREE_GIMMICKS drops the item requirement
// for Mega Evolution and Z-Moves entirely (fork-docs/FREE_GIMMICKS.md), so a Mega Stone
// or Z-Crystal in the slot would be a wasted item rather than an enabler. That is 127
// items -- listing them would bury the lists that carry actual status. Handling them by
// hold effect also means a Mega Stone added by an upstream sync classifies itself, while
// a genuinely NEW hold effect still falls through to TRACKER_UNLISTED and fails CI.
static enum ItemTrackerList GetItemTrackerList(enum Item item)
{
    switch (GetItemHoldEffect(item))
    {
    case HOLD_EFFECT_MEGA_STONE:
    case HOLD_EFFECT_Z_CRYSTAL:
        return TRACKER_IGNORED;
    default:
        break;
    }

    if (ItemIsInList(sDoneItems, ARRAY_COUNT(sDoneItems), item))
        return TRACKER_DONE;
    if (ItemIsInList(sIgnoredItems, ARRAY_COUNT(sIgnoredItems), item))
        return TRACKER_IGNORED;
    return TRACKER_UNLISTED;
}

static u32 CountRosterSetsHolding(enum Item item)
{
    u32 i, count = 0;

    for (i = 0; i < gFrontierExtendedMonsCount; i++)
    {
        if (gFrontierExtendedMons[i].heldItem == item)
            count++;
    }
    return count;
}

// An item needs tracking if it does something when held, OR if the roster already holds
// it. The second half is not redundant: Rusted Sword and Rusted Shield have no hold
// effect at all (they change Zacian/Zamazenta's form through a different mechanism) yet
// occupy a real item slot on a real set, so a sweep of hold effects alone would let them
// escape classification.
static bool32 ItemNeedsTracking(enum Item item)
{
    return GetItemHoldEffect(item) != HOLD_EFFECT_NONE || CountRosterSetsHolding(item) > 0;
}

// An item whose whole reach is ONE forme on ONE species. A second set cannot exist to
// want -- the item unlocks exactly one thing -- so for these one set is the CEILING
// rather than a shortfall, and the single-set gate below has to let them through.
//
// Derived from the form change tables rather than listed, so the 49 such items the roster
// holds (17 Plates, 17 Memories, 4 Drives, the 3 Ogerpon masks, Adamant Crystal, Lustrous
// Globe, Griseous Core, Red and Blue Orb, Rusted Sword and Shield) need no maintenance
// here, and one arriving with an upstream sync exempts itself instead of failing CI.
//
// Four methods carry a held item in param1, and all four are needed: restricting this to
// FORM_CHANGE_ITEM_HOLD -- the obvious one, and the only one the Plates/Memories/Drives
// use -- would MISS Rusted Sword and Shield, which form-change through
// FORM_CHANGE_BEGIN_BATTLE (the same reason they have no hold effect at all), and the
// Red/Blue Orb, which use FORM_CHANGE_BATTLE_PRIMAL_REVERSION. param1 is documented
// optional on the battle-boundary methods and every table names its base forme with
// ITEM_NONE, so a NONE param1 must not match or every item would look exempt.
static bool32 ItemUnlocksExactlyOneForme(enum Item item)
{
    enum Species species;

    if (item == ITEM_NONE)
        return FALSE;

    for (species = 1; species < NUM_SPECIES; species++)
    {
        const struct FormChange *formChanges;
        u32 i;

        // Species whose family is switched off by a P_FAMILY_* config are still inside
        // NUM_SPECIES but have no data, and GetSpeciesFormChanges() asserts on them
        // ("disabled species", src/pokemon.c) rather than returning NULL.
        if (!IsSpeciesEnabled(species))
            continue;

        formChanges = GetSpeciesFormChanges(species);

        for (i = 0; formChanges != NULL && formChanges[i].method != FORM_CHANGE_TERMINATOR; i++)
        {
            switch (formChanges[i].method)
            {
            case FORM_CHANGE_ITEM_HOLD:
            case FORM_CHANGE_BEGIN_BATTLE:
            case FORM_CHANGE_END_BATTLE:
            case FORM_CHANGE_BATTLE_PRIMAL_REVERSION:
                if (formChanges[i].param1 == item)
                    return TRUE;
                break;
            default:
                break;
            }
        }
    }
    return FALSE;
}

// The TWIN case the rule above misses, and the reason it has to exist: a signature type
// item and its Legends-Arceus counterpart are the SAME ITEM in a battle. Adamant Orb and
// Adamant Crystal both carry HOLD_EFFECT_ADAMANT_ORB, and the one site that reads it
// (src/battle_util.c, HOLD_EFFECT_ADAMANT_ORB) gives either of them the same
// BUFF_SIGNATURE_TYPE_ITEMS boost on the same two types to the same species -- it matches
// the holder with GET_BASE_SPECIES_ID, so the base forme and the Origin forme both qualify
// with either item. Only the crystal appears in a form change table, so only the crystal
// was exempt above. That is a gen-config accident, not a difference in behavior: which
// half of the pair unlocks the forme depends on I_GRISEOUS_ORB_FORM_CHANGE and friends,
// and at GEN_LATEST it is Griseous CORE rather than Griseous Orb.
//
// One set is the ceiling for the orb half too, and the flavor reason is also the
// mechanical one. The orb belongs to the base forme and the crystal to the Origin forme;
// each forme has its own sets; and the draft compares EXACT species, not base species
// (src/battle_frontier.c, "Ensure this Pokemon species isn't a duplicate"), so one team can
// field Dialga holding the Orb AND Dialga-Origin holding the Crystal at once. Putting an
// orb on a SECOND set of the same forme would buy appearance rate by spending set variety,
// on species that get very few slots to begin with -- all six formes are TIER_MYTHICAL
// (src/fork/species_tiers.c), so they arrive only through a reserved forced-tier slot.
//
// HOLD_EFFECT_NONE must not match, or the gate would gut itself: Rusted Sword and Rusted
// Shield are form-change enablers with NO hold effect at all, so without that guard every
// effectless item in the build would look twinned with them and sail through.
static bool32 ItemSharesHoldEffectWithAFormeUnlocker(enum Item item)
{
    enum HoldEffect holdEffect = GetItemHoldEffect(item);
    enum Species species;

    if (item == ITEM_NONE || holdEffect == HOLD_EFFECT_NONE)
        return FALSE;

    for (species = 1; species < NUM_SPECIES; species++)
    {
        const struct FormChange *formChanges;
        u32 i;

        // Same disabled-species guard as ItemUnlocksExactlyOneForme() above.
        if (!IsSpeciesEnabled(species))
            continue;

        formChanges = GetSpeciesFormChanges(species);

        for (i = 0; formChanges != NULL && formChanges[i].method != FORM_CHANGE_TERMINATOR; i++)
        {
            switch (formChanges[i].method)
            {
            case FORM_CHANGE_ITEM_HOLD:
            case FORM_CHANGE_BEGIN_BATTLE:
            case FORM_CHANGE_END_BATTLE:
            case FORM_CHANGE_BATTLE_PRIMAL_REVERSION:
                if (formChanges[i].param1 != ITEM_NONE
                 && formChanges[i].param1 != item
                 && GetItemHoldEffect(formChanges[i].param1) == holdEffect)
                    return TRUE;
                break;
            default:
                break;
            }
        }
    }
    return FALSE;
}

TEST("Held item tracker: every held item is on exactly one tracker list")
{
    u32 item;
    u32 tracked = 0, unlisted = 0, duplicated = 0;

    for (item = ITEM_NONE + 1; item < ITEMS_COUNT; item++)
    {
        u32 listed = 0;

        if (!ItemNeedsTracking(item))
            continue;

        tracked++;

        if (ItemIsInList(sDoneItems, ARRAY_COUNT(sDoneItems), item))
            listed++;
        if (ItemIsInList(sIgnoredItems, ARRAY_COUNT(sIgnoredItems), item))
            listed++;

        if (listed > 1)
        {
            duplicated++;
            Test_MgbaPrintf("%S is on %d tracker lists -- an item's list is its status, so it must be on exactly one. Delete the stale entry",
                            GetItemName(item), listed);
        }
        else if (listed == 0 && GetItemTrackerList(item) == TRACKER_UNLISTED)
        {
            unlisted++;
            Test_MgbaPrintf("%S does something when held but is on no tracker list. There are only two: sDoneItems[] (balance is right AND at least one set holds it) or sIgnoredItems[] (no set holds it -- either nothing it does is reachable in a frontier battle, or we deliberately do not draft it). Judge it and add it in test/fork/held_item_tracker.c",
                            GetItemName(item));
        }
    }

    EXPECT_GT(tracked, 300);
    EXPECT_EQ(unlisted, 0);
    EXPECT_EQ(duplicated, 0);
}

TEST("Held item tracker: no done item exceeds the roster share cap")
{
    u32 i;
    u32 offenders = 0;

    for (i = 0; i < ARRAY_COUNT(sDoneItems); i++)
    {
        u32 sets = CountRosterSetsHolding(sDoneItems[i]);

        // Integer-only: sets * 100 > cap * total avoids a division and any rounding.
        if (sets * 100 > HELD_ITEM_MAX_ROSTER_SHARE_PERCENT * gFrontierExtendedMonsCount)
        {
            offenders++;
            Test_MgbaPrintf("%S is on %d of %d sets, over the %d%% cap. Only one of each item can appear per team (src/battle_frontier.c), so an over-concentrated item is drafted LESS often -- move sets onto the long tail rather than raising the cap. fork-docs/HELD_ITEMS.md lists the uncontested items",
                            GetItemName(sDoneItems[i]), sets, gFrontierExtendedMonsCount,
                            HELD_ITEM_MAX_ROSTER_SHARE_PERCENT);
        }
    }

    EXPECT_EQ(offenders, 0);
}

TEST("Held item tracker: every done item appears on at least one set")
{
    u32 i;
    u32 offenders = 0;

    for (i = 0; i < ARRAY_COUNT(sDoneItems); i++)
    {
        if (CountRosterSetsHolding(sDoneItems[i]) == 0)
        {
            offenders++;
            Test_MgbaPrintf("%S is on the done list but no set holds it, so nothing it does is reachable. Give a set the item, or move it to sIgnoredItems[] and lower HELD_ITEM_DONE_FLOOR if you mean to stop drafting it",
                            GetItemName(sDoneItems[i]));
        }
    }

    EXPECT_EQ(offenders, 0);
}

// The gap the gate above leaves open, and the reason it is worth a second sweep: an item
// graduates on TWO sets but that gate only asks for one, so a done item dropping 2 -> 1
// passes silently. That is exactly how Dragon Fang drifted -- drafting Soul Dew re-itemed
// the Latios set that was its second home, and nothing noticed until a manual audit.
// Every roster batch moves sets OFF items as well as onto them, so without this the same
// drift recurs on every batch.
TEST("Held item tracker: no done item sits on a single set")
{
    u32 i;
    u32 offenders = 0;

    for (i = 0; i < ARRAY_COUNT(sDoneItems); i++)
    {
        // One forme on one species IS the whole reach of a form-change enabler, so one
        // set is its ceiling. They stay on the done list precisely so the zero-set gate
        // keeps watching them: delete that Giratina-Origin set and CI should still notice.
        // The second test covers the orb half of each signature pair, which is the same
        // item in battle as the crystal that unlocks the forme -- see the comment on
        // ItemSharesHoldEffectWithAFormeUnlocker() for why one set is its ceiling too.
        if (ItemUnlocksExactlyOneForme(sDoneItems[i])
         || ItemSharesHoldEffectWithAFormeUnlocker(sDoneItems[i]))
            continue;

        if (CountRosterSetsHolding(sDoneItems[i]) == 1)
        {
            offenders++;
            Test_MgbaPrintf("%S is on the done list but only one set holds it, and with one of each item per team that is a roll away from never appearing. Graduation takes TWO sets unless one is its ceiling: give it a second set, or move it to sIgnoredItems[] and lower HELD_ITEM_DONE_FLOOR if you mean to stop drafting it",
                            GetItemName(sDoneItems[i]));
        }
    }

    EXPECT_EQ(offenders, 0);
}

// Pins the twin exemption the gate above leans on, because it is the one exemption that can
// be WIDENED by accident. It keys on a shared hold effect, and hold effects are shared by
// plenty of things that are not signature items -- so this fixes both what must pass through
// it and, more importantly, what must not.
TEST("Held item tracker: the signature orbs are exempt, and nothing else sneaks through")
{
    // The three orbs are the point of the rule. Each is the base forme's half of a pair whose
    // other half unlocks the Origin forme, and in battle they are the same item.
    EXPECT(ItemSharesHoldEffectWithAFormeUnlocker(ITEM_ADAMANT_ORB));
    EXPECT(ItemSharesHoldEffectWithAFormeUnlocker(ITEM_LUSTROUS_ORB));
    EXPECT(ItemSharesHoldEffectWithAFormeUnlocker(ITEM_GRISEOUS_ORB));

    // Their crystals were already exempt the older way, and must stay that way rather than
    // quietly starting to depend on the new rule.
    EXPECT(ItemUnlocksExactlyOneForme(ITEM_ADAMANT_CRYSTAL));
    EXPECT(ItemUnlocksExactlyOneForme(ITEM_LUSTROUS_GLOBE));
    EXPECT(ItemUnlocksExactlyOneForme(ITEM_GRISEOUS_CORE));

    // THE GUARD THAT MATTERS. Rusted Sword and Rusted Shield are form-change enablers with
    // HOLD_EFFECT_NONE, so a rule that compared hold effects without excluding NONE would
    // call every effectless item in the build their twin and exempt the lot -- turning the
    // single-set gate off for most of the item table with nothing visibly failing.
    EXPECT_EQ(GetItemHoldEffect(ITEM_RUSTED_SWORD), HOLD_EFFECT_NONE);
    EXPECT_EQ(ItemSharesHoldEffectWithAFormeUnlocker(ITEM_POTION), FALSE);

    // Soul Dew is the control for over-reach in the other direction: it is a signature type
    // item on the same BUFF_SIGNATURE_TYPE_ITEMS footing as the orbs, but it has no
    // forme-unlocking twin, so it is NOT at a one-set ceiling and still owes the gate two
    // sets. If this ever passes, the rule has stopped being about twins.
    EXPECT_EQ(ItemSharesHoldEffectWithAFormeUnlocker(ITEM_SOUL_DEW), FALSE);
    EXPECT_EQ(ItemUnlocksExactlyOneForme(ITEM_SOUL_DEW), FALSE);
    EXPECT_GE(CountRosterSetsHolding(ITEM_SOUL_DEW), 2);

    // And an ordinary item is exempt by neither rule, which is what keeps the gate a gate.
    EXPECT_EQ(ItemSharesHoldEffectWithAFormeUnlocker(ITEM_LEFTOVERS), FALSE);
    EXPECT_EQ(ItemUnlocksExactlyOneForme(ITEM_LEFTOVERS), FALSE);
}

TEST("Held item tracker: no set holds an ignored item")
{
    u32 item;
    u32 offenders = 0;

    // Swept over every item rather than over sIgnoredItems[], so this covers the two
    // classes GetItemTrackerList() excludes by hold effect as well as the explicit list.
    // Those classes are the reason the check is worth having: a Mega Stone or Z-Crystal
    // in the slot does NOTHING under FEATURE_FREE_GIMMICKS -- the gimmick is item-free --
    // so the set is silently playing an item down, with no symptom anywhere else. The
    // out-of-battle entries are the same kind of dead weight.
    for (item = ITEM_NONE + 1; item < ITEMS_COUNT; item++)
    {
        u32 sets;

        if (GetItemTrackerList(item) != TRACKER_IGNORED)
            continue;

        sets = CountRosterSetsHolding(item);
        if (sets > 0)
        {
            offenders++;
            Test_MgbaPrintf("%S is on the ignored list but %d set(s) hold it. Ignored means NO SET HOLDS IT: either nothing it does is reachable here, in which case those sets are playing an item down, or it works and we deliberately do not draft it, in which case this set is undoing that call. Give them a real item, or move this one to sDoneItems[] in the same commit if you mean to start drafting it",
                            GetItemName(item), sets);
        }
    }

    EXPECT_EQ(offenders, 0);
}

TEST("Held item tracker: the done list never shrinks")
{
    // A ratchet, not a target. Graduating an item raises this; a drop means one was
    // demoted, which should be a reviewed decision rather than a quiet way to silence
    // one of the gates above.
    EXPECT_GE(ARRAY_COUNT(sDoneItems), HELD_ITEM_DONE_FLOOR);
}
