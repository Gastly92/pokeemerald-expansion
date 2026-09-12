#include "global.h"
#include "test/test.h"
#include "item.h"
#include "fork/frontier_extended_mons.h"
#include "constants/items.h"

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
// Both axes collapse into three lists. An item's list IS its status:
//
//   sDoneItems[]    -- balance is right AND it is live in the roster. Gated both ways:
//                      at least one set holds it, and it does not exceed
//                      HELD_ITEM_MAX_ROSTER_SHARE_PERCENT of the roster.
//   sPendingItems[] -- work outstanding, of any kind: it needs a buff, it is mechanically
//                      fine and simply needs a set, or it is drafted so thinly that the
//                      count is itself the signal. NOT gated -- a pending
//                      item is allowed to sit at zero sets, which is usually why it is
//                      pending. Promote it to sDoneItems[] once both axes are satisfied.
//   sIgnoredItems[] -- cannot appear in a frontier battle at all. Neither axis means
//                      anything for these, so they are exempt from the done gates -- but
//                      no set may hold one, since doing so plays an item down for free.
//
// The point of the split is that graduating an item to sDoneItems[] is what ARMS the
// gates for it. That is the failure this file exists to catch: Wide Lens, Zoom Lens,
// Blunder Policy, Razor Fang and Lansat Berry all received real engine work and then
// shipped to nobody, because nothing connected "we buffed it" to "a set holds it".
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

// The done list never shrinks. Bump this when items graduate; a drop means an item was
// demoted to pending, which is a real regression and should be a deliberate, reviewed act
// rather than a quiet way to dodge one of the gates above.
#define HELD_ITEM_DONE_FLOOR 81

// Balance is right AND the roster uses it. Both gates below apply to every entry here.
static const enum Item sDoneItems[] =
{
    ITEM_ADAMANT_CRYSTAL,
    ITEM_AGUAV_BERRY,
    ITEM_ASSAULT_VEST,
    ITEM_BIG_ROOT,
    ITEM_BLACK_BELT,
    ITEM_BLACK_GLASSES,
    ITEM_BLACK_SLUDGE,
    ITEM_BLUE_ORB,
    ITEM_BOOSTER_ENERGY,
    ITEM_CHARCOAL,
    ITEM_CHESTO_BERRY,
    ITEM_CHOICE_BAND,
    ITEM_CHOICE_SCARF,
    ITEM_CHOICE_SPECS,
    ITEM_CORNERSTONE_MASK,
    ITEM_COVERT_CLOAK,
    ITEM_DAMP_ROCK,
    ITEM_DRAGON_FANG,
    ITEM_DRAGON_MEMORY,
    ITEM_EVIOLITE,
    ITEM_EXPERT_BELT,
    ITEM_FAIRY_FEATHER,
    ITEM_FAIRY_MEMORY,
    ITEM_FIGY_BERRY,
    ITEM_FLAME_ORB,
    ITEM_FLYING_GEM,
    ITEM_FOCUS_BAND,
    ITEM_FOCUS_SASH,
    ITEM_GRASSY_SEED,
    ITEM_GRIP_CLAW,
    ITEM_GRISEOUS_ORB,
    ITEM_GROUND_MEMORY,
    ITEM_HARD_STONE,
    ITEM_HEARTHFLAME_MASK,
    ITEM_HEAT_ROCK,
    ITEM_HEAVY_DUTY_BOOTS,
    ITEM_ICY_ROCK,
    ITEM_IRON_BALL,
    ITEM_KINGS_ROCK,
    ITEM_LEEK,
    ITEM_LEFTOVERS,
    ITEM_LIFE_ORB,
    ITEM_LIGHT_BALL,
    ITEM_LIGHT_CLAY,
    ITEM_LOADED_DICE,
    ITEM_LUM_BERRY,
    ITEM_LUSTROUS_GLOBE,
    ITEM_MAGNET,
    ITEM_MENTAL_HERB,
    ITEM_METAL_COAT,
    ITEM_MISTY_SEED,
    ITEM_MUSCLE_BAND,
    ITEM_MYSTIC_WATER,
    ITEM_NEVER_MELT_ICE,
    ITEM_PETAYA_BERRY,
    ITEM_POISON_BARB,
    ITEM_PUNCHING_GLOVE,
    ITEM_QUICK_CLAW,
    ITEM_RAZOR_CLAW,
    ITEM_RED_ORB,
    ITEM_ROCKY_HELMET,
    ITEM_RUSTED_SHIELD,
    ITEM_RUSTED_SWORD,
    ITEM_SCOPE_LENS,
    ITEM_SHARP_BEAK,
    ITEM_SHELL_BELL,
    ITEM_SILK_SCARF,
    ITEM_SITRUS_BERRY,
    ITEM_SMOOTH_ROCK,
    ITEM_SOFT_SAND,
    ITEM_SPELL_TAG,
    ITEM_STEEL_MEMORY,
    ITEM_TERRAIN_EXTENDER,
    ITEM_THICK_CLUB,
    ITEM_THROAT_SPRAY,
    ITEM_TOXIC_ORB,
    ITEM_TWISTED_SPOON,
    ITEM_WEAKNESS_POLICY,
    ITEM_WELLSPRING_MASK,
    ITEM_WHITE_HERB,
    ITEM_WISE_GLASSES,
};

// Work outstanding. Not gated -- these are allowed to sit at zero sets.
static const enum Item sPendingItems[] =
{
    // ---- Needs a BUFF: dominated or underpowered as shipped. --------------------
    // Oran Berry and Berry Juice: a flat 20 HP does not survive the jump to Level 50,
    // against Sitrus Berry's 25% on 105 sets.
    //
    // The Memories, Drives, Soul Dew and the signature orbs used to sit here. They were
    // settled by BUFF_SIGNATURE_TYPE_ITEMS, which put the whole signature class on the
    // generic type items' scale and locked each one to its own species; the four Memories
    // the roster already holds graduated with it. The rest moved down to "needs a SET".
    ITEM_BERRY_JUICE,
    ITEM_ORAN_BERRY,


    // ---- Thinly drafted: on exactly one set, and the count is itself the signal. ----
    // Demoted from done because one set is close enough to zero that the item is barely
    // reachable -- with only one of each item allowed per team, a single set carrying it
    // is one roll away from never appearing. Some of these want a buff (Safety Goggles,
    // Power Herb, Mirror Herb, Custap Berry); some are mechanically fine and want a
    // SECOND set (the type items, the terrain seeds, Razor Claw). Either way the work is
    // outstanding. Moving the four resist berries and Salac Berry here also makes those
    // two classes whole -- their siblings were already pending, and a class split across
    // two lists reads as an oversight rather than a judgement.
    //
    // Note what is deliberately NOT here: the form-change enablers that also sit at one
    // set (Adamant Crystal, Lustrous Globe, Griseous Orb, Red/Blue Orb, Rusted Sword and
    // Shield, the three Ogerpon masks, and now the four Memories the roster holds). For
    // those, one is the CEILING rather than a shortfall -- each unlocks exactly one forme
    // on exactly one species, so there is no buff to write and no second set to want.
    // They stay done precisely so the one-set gate keeps watching them: delete that
    // Giratina-Origin set and CI should notice.
    //
    // A Memory qualifies on the same reading: FORM_CHANGE_ITEM_HOLD means Dragon Memory
    // unlocks Silvally-Dragon and nothing else, so Silvally-Dragon's one set is the item's
    // whole reach. That it now also carries a damage boost does not change this -- the
    // Ogerpon masks have carried one all along and sit in this exception already.

    // The six Gems here are the same story one notch along: BUFF_GEMS settled their
    // balance and the roster now spends them correctly, but each sits on a single set.
    // One set is one set whoever placed it -- only Flying Gem, at three, cleared the bar.
    ITEM_AIR_BALLOON,
    ITEM_BRIGHT_POWDER,
    ITEM_CHOPLE_BERRY,
    ITEM_COLBUR_BERRY,
    ITEM_CUSTAP_BERRY,
    ITEM_DRAGON_GEM,
    ITEM_ELECTRIC_SEED,
    ITEM_FAIRY_GEM,
    ITEM_FIRE_GEM,
    ITEM_GRASS_GEM,
    ITEM_MIRACLE_SEED,
    ITEM_MIRROR_HERB,
    ITEM_PASSHO_BERRY,
    ITEM_POWER_HERB,
    ITEM_PSYCHIC_GEM,
    ITEM_PSYCHIC_SEED,
    ITEM_SAFETY_GOGGLES,
    ITEM_SALAC_BERRY,
    ITEM_SHUCA_BERRY,
    ITEM_SILVER_POWDER,
    ITEM_STEEL_GEM,
    // ---- Needs a SET: mechanically fine, held by nobody. No engine work. ---------
    // The 11 Gems here are DONE on balance -- BUFF_GEMS took the class to +60% and the
    // roster now spends them correctly -- and pending only because no set holds these
    // particular types yet. Their 7 siblings graduated. A Gem wants a move the set fires
    // ONCE (a self-debuffing nuke like Overheat or Make It Rain, an Acrobatics set, or
    // true coverage), never a move it clicks every turn; see fork-docs/LINE_REVIEW.md.
    // Includes the five items this fork specifically repaired and then shipped to
    // nobody (Wide Lens, Zoom Lens, Blunder Policy, Razor Fang, Lansat Berry), all 17
    // Arceus plates (already carrying the +40% buff), and the wide uncontested tails --
    // 14 of the 18 resist berries, the five type-boost incenses, Lax Incense.
    //
    // The signature type items settled by BUFF_SIGNATURE_TYPE_ITEMS are here too: 13
    // Memories, all 4 Drives, Soul Dew and the three plain orbs. Each is now locked to
    // one species, so each wants a set on THAT species and nowhere else -- a Memory needs
    // its Silvally forme, a Drive needs a Genesect, Soul Dew a Lati@s. Note Arceus and
    // Genesect are TIER_MYTHICAL, so their sets are reachable only through a reserved
    // forced-tier slot; Silvally is TIER_NORMAL and rentable.
    ITEM_ABILITY_SHIELD,
    ITEM_ABSORB_BULB,
    ITEM_ADAMANT_ORB,
    ITEM_ADRENALINE_ORB,
    ITEM_APICOT_BERRY,
    ITEM_ASPEAR_BERRY,
    ITEM_BABIRI_BERRY,
    ITEM_BERSERK_GENE,
    ITEM_BINDING_BAND,
    ITEM_BLUNDER_POLICY,
    ITEM_BUG_GEM,
    ITEM_BUG_MEMORY,
    ITEM_BURN_DRIVE,
    ITEM_CELL_BATTERY,
    ITEM_CHARTI_BERRY,
    ITEM_CHERI_BERRY,
    ITEM_CHILAN_BERRY,
    ITEM_CHILL_DRIVE,
    ITEM_CLEAR_AMULET,
    ITEM_COBA_BERRY,
    ITEM_DARK_GEM,
    ITEM_DARK_MEMORY,
    ITEM_DEEP_SEA_SCALE,
    ITEM_DEEP_SEA_TOOTH,
    ITEM_DOUSE_DRIVE,
    ITEM_DRACO_PLATE,
    ITEM_DREAD_PLATE,
    ITEM_EARTH_PLATE,
    ITEM_EJECT_BUTTON,
    ITEM_EJECT_PACK,
    ITEM_ELECTRIC_GEM,
    ITEM_ELECTRIC_MEMORY,
    ITEM_ENIGMA_BERRY,
    ITEM_FIGHTING_GEM,
    ITEM_FIGHTING_MEMORY,
    ITEM_FIRE_MEMORY,
    ITEM_FIST_PLATE,
    ITEM_FLAME_PLATE,
    ITEM_FLOAT_STONE,
    ITEM_FLYING_MEMORY,
    ITEM_FULL_INCENSE,
    ITEM_GANLON_BERRY,
    ITEM_GHOST_GEM,
    ITEM_GHOST_MEMORY,
    ITEM_GRASS_MEMORY,
    ITEM_GRISEOUS_CORE,
    ITEM_GROUND_GEM,
    ITEM_HABAN_BERRY,
    ITEM_IAPAPA_BERRY,
    ITEM_ICE_GEM,
    ITEM_ICE_MEMORY,
    ITEM_ICICLE_PLATE,
    ITEM_INSECT_PLATE,
    ITEM_IRON_PLATE,
    ITEM_JABOCA_BERRY,
    ITEM_KASIB_BERRY,
    ITEM_KEBIA_BERRY,
    ITEM_KEE_BERRY,
    ITEM_LAGGING_TAIL,
    ITEM_LANSAT_BERRY,
    ITEM_LAX_INCENSE,
    ITEM_LEPPA_BERRY,
    ITEM_LIECHI_BERRY,
    ITEM_LUCKY_PUNCH,
    ITEM_LUMINOUS_MOSS,
    ITEM_LUSTROUS_ORB,
    ITEM_MAGO_BERRY,
    ITEM_MARANGA_BERRY,
    ITEM_MEADOW_PLATE,
    ITEM_METAL_POWDER,
    ITEM_METRONOME,
    ITEM_MICLE_BERRY,
    ITEM_MIND_PLATE,
    ITEM_NORMAL_GEM,
    ITEM_OCCA_BERRY,
    ITEM_ODD_INCENSE,
    ITEM_PAYAPA_BERRY,
    ITEM_PECHA_BERRY,
    ITEM_PERSIM_BERRY,
    ITEM_PIXIE_PLATE,
    ITEM_POISON_GEM,
    ITEM_POISON_MEMORY,
    ITEM_PROTECTIVE_PADS,
    ITEM_PSYCHIC_MEMORY,
    ITEM_QUICK_POWDER,
    ITEM_RAWST_BERRY,
    ITEM_RAZOR_FANG,
    ITEM_RED_CARD,
    ITEM_RINDO_BERRY,
    ITEM_RING_TARGET,
    ITEM_ROCK_GEM,
    ITEM_ROCK_INCENSE,
    ITEM_ROCK_MEMORY,
    ITEM_ROOM_SERVICE,
    ITEM_ROSELI_BERRY,
    ITEM_ROSE_INCENSE,
    ITEM_ROWAP_BERRY,
    ITEM_SEA_INCENSE,
    ITEM_SHED_SHELL,
    ITEM_SHOCK_DRIVE,
    ITEM_SKY_PLATE,
    ITEM_SNOWBALL,
    ITEM_SOUL_DEW,
    ITEM_SPLASH_PLATE,
    ITEM_SPOOKY_PLATE,
    ITEM_STARF_BERRY,
    ITEM_STICKY_BARB,
    ITEM_STONE_PLATE,
    ITEM_TANGA_BERRY,
    ITEM_TOXIC_PLATE,
    ITEM_UTILITY_UMBRELLA,
    ITEM_WACAN_BERRY,
    ITEM_WATER_GEM,
    ITEM_WATER_MEMORY,
    ITEM_WAVE_INCENSE,
    ITEM_WIDE_LENS,
    ITEM_WIKI_BERRY,
    ITEM_YACHE_BERRY,
    ITEM_ZAP_PLATE,
    ITEM_ZOOM_LENS,
};

// Cannot appear in a frontier battle, so neither axis means anything -- but no set may
// hold one (see the last test).
// The two big structural classes -- Mega Stones and Z-Crystals, item-free under
// FEATURE_FREE_GIMMICKS -- are excluded by hold effect in GetItemTrackerList() instead
// of listed here. What remains is the out-of-battle utility: nothing they do is
// reachable in a battle. (The Power items' Speed halving IS reachable, but a Trick Room
// set gets the same result for free with IVS(SPE, 0).)
static const enum Item sIgnoredItems[] =
{
    ITEM_AMULET_COIN,
    ITEM_CLEANSE_TAG,
    ITEM_DESTINY_KNOT,
    ITEM_EVERSTONE,
    ITEM_EXP_SHARE,
    ITEM_LUCKY_EGG,
    ITEM_LUCK_INCENSE,
    ITEM_MACHO_BRACE,
    ITEM_POWER_ANKLET,
    ITEM_POWER_BAND,
    ITEM_POWER_BELT,
    ITEM_POWER_BRACER,
    ITEM_POWER_LENS,
    ITEM_POWER_WEIGHT,
    ITEM_PURE_INCENSE,
    ITEM_SMOKE_BALL,
    ITEM_SOOTHE_BELL,
};

enum ItemTrackerList
{
    TRACKER_UNLISTED,
    TRACKER_DONE,
    TRACKER_PENDING,
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
    if (ItemIsInList(sPendingItems, ARRAY_COUNT(sPendingItems), item))
        return TRACKER_PENDING;
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
        if (ItemIsInList(sPendingItems, ARRAY_COUNT(sPendingItems), item))
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
            Test_MgbaPrintf("%S does something when held but is on no tracker list. Add it to sDoneItems[] (balance is right and a set holds it), sPendingItems[] (needs a buff, or needs a set), or sIgnoredItems[] (unreachable in a frontier battle) in test/fork/held_item_tracker.c",
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
            Test_MgbaPrintf("%S is on the done list but no set holds it, so nothing it does is reachable. Give a set the item, or move it to sPendingItems[] until one does",
                            GetItemName(sDoneItems[i]));
        }
    }

    EXPECT_EQ(offenders, 0);
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
            Test_MgbaPrintf("%S is on the ignored list but %d set(s) hold it -- an ignored item does nothing in a frontier battle, so those sets are playing an item down. Give them a real item, or move this one off sIgnoredItems[] if it turns out to matter",
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
