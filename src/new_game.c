#include "global.h"
#include "clock.h"
#include "new_game.h"
#include "random.h"
#include "pokemon.h"
#include "roamer.h"
#include "pokemon_size_record.h"
#include "script.h"
#include "lottery_corner.h"
#include "play_time.h"
#include "mauville_old_man.h"
#include "match_call.h"
#include "lilycove_lady.h"
#include "load_save.h"
#include "pokeblock.h"
#include "dewford_trend.h"
#include "berry.h"
#include "rtc.h"
#include "easy_chat.h"
#include "event_data.h"
#include "money.h"
#include "trainer_hill.h"
#include "trainer_tower.h"
#include "tv.h"
#include "coins.h"
#include "text.h"
#include "overworld.h"
#include "mail.h"
#include "battle_records.h"
#include "item.h"
#include "pokedex.h"
#include "apprentice.h"
#include "frontier_util.h"
#include "pokedex.h"
#include "save.h"
#include "link_rfu.h"
#include "main.h"
#include "contest.h"
#include "item_menu.h"
#include "pokemon_storage_system.h"
#include "pokemon_jump.h"
#include "decoration_inventory.h"
#include "secret_base.h"
#include "string_util.h"
#include "player_pc.h"
#include "field_specials.h"
#include "berry_powder.h"
#include "mystery_gift.h"
#include "union_room_chat.h"
#include "constants/heal_locations.h"
#include "constants/map_groups.h"
#include "constants/items.h"
#include "difficulty.h"
#include "follower_npc.h"

extern const u8 EventScript_ResetAllMapFlags[];
extern const u8 EventScript_ResetAllMapFlagsFrlg[];

static void ClearFrontierRecord(void);
static void WarpToTruck(void);
#if START_AT_BATTLE_FRONTIER
static void SetBattleFrontierFirstArrivalState(void);
#endif
static void ResetMiniGamesRecords(void);
static void ResetItemFlags(void);
static void ResetDexNav(void);

EWRAM_DATA bool8 gDifferentSaveFile = FALSE;
EWRAM_DATA bool8 gEnableContestDebugging = FALSE;

static const struct ContestWinner sContestWinnerPicDummy =
{
    .monName = _(""),
    .trainerName = _("")
};

void SetTrainerId(u32 trainerId, u8 *dst)
{
    dst[0] = trainerId;
    dst[1] = trainerId >> 8;
    dst[2] = trainerId >> 16;
    dst[3] = trainerId >> 24;
}

u32 GetTrainerId(u8 *trainerId)
{
    return (trainerId[3] << 24) | (trainerId[2] << 16) | (trainerId[1] << 8) | (trainerId[0]);
}

void CopyTrainerId(u8 *dst, u8 *src)
{
    s32 i;
    for (i = 0; i < TRAINER_ID_LENGTH; i++)
        dst[i] = src[i];
}

static void InitPlayerTrainerId(void)
{
    u32 trainerId = (Random() << 16) | GetGeneratedTrainerIdLower();
    SetTrainerId(trainerId, gSaveBlock2Ptr->playerTrainerId);
}

// L=A isnt set here for some reason.
static void SetDefaultOptions(void)
{
    gSaveBlock2Ptr->optionsTextSpeed = NEW_GAME_TEXT_SPEED; // FORK: was OPTIONS_TEXT_SPEED_MID; default now configurable via NEW_GAME_TEXT_SPEED (config/general.h).
    gSaveBlock2Ptr->optionsWindowFrameType = 0;
    gSaveBlock2Ptr->optionsSound = OPTIONS_SOUND_MONO;
    gSaveBlock2Ptr->optionsBattleStyle = NEW_GAME_BATTLE_STYLE; // FORK: was OPTIONS_BATTLE_STYLE_SHIFT; default now configurable via NEW_GAME_BATTLE_STYLE (config/general.h).
    gSaveBlock2Ptr->optionsBattleSceneOff = FALSE;
    gSaveBlock2Ptr->regionMapZoom = FALSE;
}

static void ClearPokedexFlags(void)
{
    gUnusedPokedexU8 = 0;
    memset(&gSaveBlock1Ptr->dexCaught, 0, sizeof(gSaveBlock1Ptr->dexCaught));
    memset(&gSaveBlock1Ptr->dexSeen, 0, sizeof(gSaveBlock1Ptr->dexSeen));
}

void ClearAllContestWinnerPics(void)
{
    s32 i;

    ClearContestWinnerPicsInContestHall();

    // Clear Museum paintings
    for (i = MUSEUM_CONTEST_WINNERS_START; i < NUM_CONTEST_WINNERS; i++)
        gSaveBlock1Ptr->contestWinners[i] = sContestWinnerPicDummy;
}

static void ClearFrontierRecord(void)
{
    CpuFill32(0, &gSaveBlock2Ptr->frontier, sizeof(gSaveBlock2Ptr->frontier));

    gSaveBlock2Ptr->frontier.opponentNames[0][0] = EOS;
    gSaveBlock2Ptr->frontier.opponentNames[1][0] = EOS;
}

// FORK: When START_AT_BATTLE_FRONTIER is set we drop the player at the
// Battle Frontier ferry dock instead of inside the moving truck, so a new game
// begins as if they had just arrived there for the first time. The matching
// flag/var setup lives in SetBattleFrontierFirstArrivalState(); CB2_NewGame()
// also swaps the truck-animation field callback for a plain fade under the same
// flag. On conflict with upstream's WarpToTruck, port their change into the
// else-branch below rather than dropping the Battle Frontier destination.
static void WarpToTruck(void)
{
    if (IS_FRLG)
        SetWarpDestination(MAP_GROUP(MAP_PALLET_TOWN_PLAYERS_HOUSE_2F), MAP_NUM(MAP_PALLET_TOWN_PLAYERS_HOUSE_2F), WARP_ID_NONE, 6, 6);
#if START_AT_BATTLE_FRONTIER
    else
        SetWarpDestination(MAP_GROUP(MAP_BATTLE_FRONTIER_OUTSIDE_WEST), MAP_NUM(MAP_BATTLE_FRONTIER_OUTSIDE_WEST), WARP_ID_NONE, 19, 67); // Ferry landing spot
#else
    else
        SetWarpDestination(MAP_GROUP(MAP_INSIDE_OF_TRUCK), MAP_NUM(MAP_INSIDE_OF_TRUCK), WARP_ID_NONE, -1, -1);
#endif
    WarpIntoMap();
}

#if START_AT_BATTLE_FRONTIER
// FORK: Puts a new save into the state expected for a first arrival at the
// Battle Frontier. Because we never play the truck/Littleroot intro, we
// (a) set the prerequisite flags so the player spawns at the ferry landing and
// the reception-gate first-time scene can play as the game's intro, and (b) mark
// the Littleroot intro complete and pick a sane heal/respawn point so whiteouts
// and intro map scripts behave.
static void SetBattleFrontierFirstArrivalState(void)
{
    if (IS_FRLG)
        return;

    // Battle Frontier "first arrival" flags. FLAG_MET_SCOTT_ON_SS_TIDAL is the
    // prerequisite that unlocks the ferry; FLAG_LANDMARK_BATTLE_FRONTIER shows the
    // landmark on the spawn map before the player reaches the gate.
    //
    // FORK: unlike the rest of the skipped intro, we deliberately let the
    // reception-gate first-time scene PLAY — it is the game's intro (greeter
    // welcome + Frontier Pass + Scott pointing the player at the Battle Factory).
    // The player spawns at the ferry landing and must walk north through the
    // Reception Gate to reach any facility, so the scene (gated on
    // VAR_HAS_ENTERED_BATTLE_FRONTIER == 0) fires naturally and is unmissable.
    // Therefore do NOT pre-set FLAG_SYS_FRONTIER_PASS, VAR_HAS_ENTERED_BATTLE_
    // FRONTIER, or FLAG_HIDE_BATTLE_FRONTIER_RECEPTION_GATE_SCOTT here: the scene
    // issues the pass and hides Scott itself (data/maps/BattleFrontier_Reception-
    // Gate/scripts.inc). Pre-setting any of them would skip or break the intro.
    FlagSet(FLAG_MET_SCOTT_ON_SS_TIDAL);
    FlagSet(FLAG_LANDMARK_BATTLE_FRONTIER);

    // FORK: hide both facility-guide NPCs at new game. The reception-gate Scott
    // scene reveals the greeter (and sets VAR_FRONTIER_FACILITY_GUIDE) when it
    // ends; the greeter's escort to the Battle Factory then reveals the
    // permanent directions-giver. Object flags default clear (= shown), so they
    // must be set here or both would appear immediately at the ferry.
    FlagSet(FLAG_HIDE_FRONTIER_FACILITY_GUIDE_GREETER);
    FlagSet(FLAG_HIDE_FRONTIER_FACILITY_GUIDE);

    // FORK: a new game in this hack gives the player no starter, so the stock
    // FLAG_SYS_POKEMON_GET (set when receiving the first mon from Birch's bag) is
    // never set and the overworld START menu hides the POKéMON option
    // (BuildNormalStartMenu, src/start_menu.c). We intentionally leave it unset:
    // the player owns no POKéMON until they obtain their first one (the future BP
    // exchange's give-mon script sets the flag; the debug "give mon" menu also
    // does), so the empty party menu stays hidden until it has something in it.

    // The player starts broke (the default 3000 is for the stock intro) and
    // already has the Running Shoes (normally a gift from Mom during the intro
    // we skip), so the Frontier is playable straight away.
    SetMoney(&gSaveBlock1Ptr->money, 0);
    FlagSet(FLAG_SYS_B_DASH);

    // Mark the skipped Littleroot intro as complete (state 7 = past every
    // map_script_2 trigger) so none of its cutscenes fire if the player ever
    // visits Littleroot, then mirror the gender-specific house setup the truck
    // intro normally performs.
    VarSet(VAR_LITTLEROOT_INTRO_STATE, 7);
    if (gSaveBlock2Ptr->playerGender == MALE)
    {
        FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_MOM);
        FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_TRUCK);
        FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_RIVAL_MOM);
        FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_RIVAL_SIBLING);
        FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F_POKE_BALL);
        VarSet(VAR_LITTLEROOT_HOUSES_STATE_BRENDAN, 1);
    }
    else
    {
        FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_MOM);
        FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_TRUCK);
        FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_RIVAL_MOM);
        FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_RIVAL_SIBLING);
        FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_2F_POKE_BALL);
        VarSet(VAR_LITTLEROOT_HOUSES_STATE_MAY, 1);
    }

    // Give a working respawn/heal point since we skipped the intro's setrespawn.
    SetLastHealLocationWarp(HEAL_LOCATION_BATTLE_FRONTIER_OUTSIDE_EAST);
}
#endif // START_AT_BATTLE_FRONTIER

#if START_WITH_BATTLE_GIMMICK_ITEMS
// FORK: Gives a fresh save the four battle-transformation key items. Mega
// Evolution, Z-Moves and Ultra Burst become usable immediately, since their
// CanActivate checks only require the item in the bag. Dynamax and Tera are
// additionally gated on a flag (CanDynamax checks B_FLAG_DYNAMAX_BATTLE;
// CanTerastallize checks B_FLAG_TERA_ORB_NO_COST / B_FLAG_TERA_ORB_CHARGED), both
// left at their default 0 here, so the player holds the Dynamax Band and Tera Orb
// but cannot use those gimmicks yet. Enabling them is intended to be a later
// player-facing choice (a toggle that points those B_FLAG_* defines at real flags
// and sets them), so Frontier runs can opt into Dynamax/Tera.
static void GiveStartingBattleGimmickItems(void)
{
    AddBagItem(ITEM_DYNAMAX_BAND, 1);
    AddBagItem(ITEM_MEGA_RING, 1);
    AddBagItem(ITEM_TERA_ORB, 1);
    AddBagItem(ITEM_Z_POWER_RING, 1);
}
#endif // START_WITH_BATTLE_GIMMICK_ITEMS

void Sav2_ClearSetDefault(void)
{
    ClearSav2();
    SetDefaultOptions();
}

void ResetMenuAndMonGlobals(void)
{
    gDifferentSaveFile = FALSE;
    ResetPokedexScrollPositions();
    ZeroPlayerPartyMons();
    ZeroEnemyPartyMons();
    ResetBagScrollPositions();
    ResetPokeblockScrollPositions();
}

void NewGameInitData(void)
{
#if IS_FRLG
    u8 rivalName[PLAYER_NAME_LENGTH + 1];
#endif
    if (gSaveFileStatus == SAVE_STATUS_EMPTY || gSaveFileStatus == SAVE_STATUS_CORRUPT)
        RtcReset();

#if IS_FRLG
    StringCopy(rivalName, gSaveBlock1Ptr->rivalName);
#endif
    gDifferentSaveFile = TRUE;
    gSaveBlock2Ptr->encryptionKey = 0;
    ZeroPlayerPartyMons();
    ZeroEnemyPartyMons();
    ResetPokedex();
    ClearFrontierRecord();
    ClearSav1();
    ClearSav3();
    ClearAllMail();
    gSaveBlock2Ptr->specialSaveWarpFlags = 0;
    gSaveBlock2Ptr->gcnLinkFlags = 0;
    InitPlayerTrainerId();
    PlayTimeCounter_Reset();
    ClearPokedexFlags();
    InitEventData();
    ClearTVShowData();
    ResetGabbyAndTy();
    ClearSecretBases();
    ClearBerryTrees();
    SetMoney(&gSaveBlock1Ptr->money, 3000);
    SetCoins(0);
    ResetLinkContestBoolean();
    ResetGameStats();
    ClearAllContestWinnerPics();
    ClearPlayerLinkBattleRecords();
    InitSeedotSizeRecord();
    InitLotadSizeRecord();
    gPartiesCount[B_TRAINER_PLAYER] = 0;
    ZeroPlayerPartyMons();
    ResetPokemonStorageSystem();
    DeactivateAllRoamers();
    gSaveBlock1Ptr->registeredItem = ITEM_NONE;
    ClearBag();
    NewGameInitPCItems();
    ClearPokeblocks();
    ClearDecorationInventories();
    InitEasyChatPhrases();
    SetMauvilleOldMan();
    InitDewfordTrend();
    ResetFanClub();
    ResetLotteryCorner();
    UpdateDailySeed();
    WarpToTruck();
    if (IS_FRLG)
        RunScriptImmediately(EventScript_ResetAllMapFlagsFrlg);
    else
        RunScriptImmediately(EventScript_ResetAllMapFlags);
#if START_AT_BATTLE_FRONTIER
    SetBattleFrontierFirstArrivalState(); // FORK: must run after EventScript_ResetAllMapFlags so our flags survive the reset
#endif
#if START_WITH_BATTLE_GIMMICK_ITEMS
    GiveStartingBattleGimmickItems(); // FORK: any point after ClearBag() above works; grouped here with the other new-game setup
#endif
#if IS_FRLG
        StringCopy(gSaveBlock1Ptr->rivalName, rivalName);
#endif
    ResetMiniGamesRecords();
    InitUnionRoomChatRegisteredTexts();
    InitLilycoveLady();
    ResetAllApprenticeData();
    ClearRankingHallRecords();
    InitMatchCallCounters();
    ClearMysteryGift();
    WipeTrainerNameRecords();
    ResetTrainerHillResults();
    ResetTrainerTowerResults();
    ResetContestLinkResults();
    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
    ResetItemFlags();
    ResetDexNav();
    ClearFollowerNPCData();
}

static void ResetMiniGamesRecords(void)
{
    CpuFill16(0, &gSaveBlock2Ptr->berryCrush, sizeof(struct BerryCrush));
    SetBerryPowder(&gSaveBlock2Ptr->berryCrush.berryPowderAmount, 0);
    ResetPokemonJumpRecords();
    CpuFill16(0, &gSaveBlock2Ptr->berryPick, sizeof(struct BerryPickingResults));
}

static void ResetItemFlags(void)
{
#if OW_SHOW_ITEM_DESCRIPTIONS == OW_ITEM_DESCRIPTIONS_FIRST_TIME
    memset(&gSaveBlock3Ptr->itemFlags, 0, sizeof(gSaveBlock3Ptr->itemFlags));
#endif
}

static void ResetDexNav(void)
{
#if USE_DEXNAV_SEARCH_LEVELS == TRUE
    memset(gSaveBlock3Ptr->dexNavSearchLevels, 0, sizeof(gSaveBlock3Ptr->dexNavSearchLevels));
#endif
    gSaveBlock3Ptr->dexNavChain = 0;
}
