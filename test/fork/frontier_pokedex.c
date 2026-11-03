#include "global.h"
#include "test/test.h"
#include "battle.h"
#include "pokedex.h"
#include "pokemon.h"
#include "fork/frontier_pokedex.h"
#include "constants/battle.h"

// Guards FRONTIER_MARKS_DEX_SEEN (config/fork.h). Battle tests can't put a battle in
// Frontier mode, so these drive the helpers MarkFrontierBattleMonsSeen is built from,
// with a hand-made party and party state.

static void ClearDexFlags(enum NationalDexOrder dexNum)
{
    u32 index = (dexNum - 1) / 8, mask = 1 << ((dexNum - 1) % 8);
    gSaveBlock1Ptr->dexSeen[index] &= ~mask;
    gSaveBlock1Ptr->dexCaught[index] &= ~mask;
}

TEST("Frontier Pokédex: Frontier battles mark seen, link and recorded playback do not")
{
    EXPECT(ShouldFrontierBattleMarkDexSeen(BATTLE_TYPE_TRAINER | BATTLE_TYPE_FACTORY));
    EXPECT(ShouldFrontierBattleMarkDexSeen(BATTLE_TYPE_TRAINER | BATTLE_TYPE_BATTLE_TOWER | BATTLE_TYPE_DOUBLE));
    EXPECT(ShouldFrontierBattleMarkDexSeen(BATTLE_TYPE_PYRAMID)); // Pyramid wild battles
    EXPECT(!ShouldFrontierBattleMarkDexSeen(BATTLE_TYPE_TRAINER)); // upstream's own loop covers it
    EXPECT(!ShouldFrontierBattleMarkDexSeen(BATTLE_TYPE_TRAINER | BATTLE_TYPE_FACTORY | BATTLE_TYPE_LINK));
    EXPECT(!ShouldFrontierBattleMarkDexSeen(BATTLE_TYPE_TRAINER | BATTLE_TYPE_FACTORY | BATTLE_TYPE_RECORDED));
}

TEST("Frontier Pokédex: every sent-out mon is marked seen, never caught")
{
    struct Pokemon party[PARTY_SIZE];
    struct PartyState states[PARTY_SIZE] = {0};

    for (u32 i = 0; i < PARTY_SIZE; i++)
        ZeroMonData(&party[i]);
    CreateMon(&party[0], SPECIES_BULBASAUR, 50, 0, OTID_STRUCT_PRESET(0));
    CreateMon(&party[1], SPECIES_CHARMANDER, 50, 0, OTID_STRUCT_PRESET(0));
    CreateMon(&party[2], SPECIES_SQUIRTLE, 50, 0, OTID_STRUCT_PRESET(0));
    ClearDexFlags(NATIONAL_DEX_BULBASAUR);
    ClearDexFlags(NATIONAL_DEX_CHARMANDER);
    ClearDexFlags(NATIONAL_DEX_SQUIRTLE);
    states[0].sentOut = TRUE;
    states[2].sentOut = TRUE;
    states[4].sentOut = TRUE; // an empty slot is skipped, not marked as dex number 0

    MarkSentOutMonsDexSeen(party, states);

    EXPECT(GetSetPokedexFlag(NATIONAL_DEX_BULBASAUR, FLAG_GET_SEEN));
    EXPECT(GetSetPokedexFlag(NATIONAL_DEX_SQUIRTLE, FLAG_GET_SEEN));
    EXPECT(!GetSetPokedexFlag(NATIONAL_DEX_CHARMANDER, FLAG_GET_SEEN)); // never sent out
    EXPECT(!GetSetPokedexFlag(NATIONAL_DEX_BULBASAUR, FLAG_GET_CAUGHT));
    EXPECT(!GetSetPokedexFlag(NATIONAL_DEX_SQUIRTLE, FLAG_GET_CAUGHT));
}
