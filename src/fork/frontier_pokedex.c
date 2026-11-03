#include "global.h"
#include "battle.h"
#include "pokedex.h"
#include "pokemon.h"
#include "constants/battle.h"
#include "fork/frontier_pokedex.h"

bool32 ShouldFrontierBattleMarkDexSeen(u32 battleTypeFlags)
{
    if (!FRONTIER_MARKS_DEX_SEEN)
        return FALSE;
    if (!(battleTypeFlags & BATTLE_TYPE_FRONTIER))
        return FALSE; // upstream's own loop already handles every other battle it wants to
    if (battleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK | BATTLE_TYPE_RECORDED))
        return FALSE;
    return TRUE;
}

void MarkSentOutMonsDexSeen(struct Pokemon *party, const struct PartyState *states)
{
    for (u32 partySlot = 0; partySlot < PARTY_SIZE; partySlot++)
    {
        if (states[partySlot].sentOut && GetMonData(&party[partySlot], MON_DATA_SPECIES) != SPECIES_NONE)
            HandleSetPokedexFlagFromMon(&party[partySlot], FLAG_SET_SEEN);
    }
}

void MarkWholePartyDexSeen(struct Pokemon *party)
{
    for (u32 partySlot = 0; partySlot < PARTY_SIZE; partySlot++)
    {
        enum Species species = GetMonData(&party[partySlot], MON_DATA_SPECIES);
        if (species != SPECIES_NONE && species != SPECIES_EGG)
            HandleSetPokedexFlagFromMon(&party[partySlot], FLAG_SET_SEEN);
    }
}

void MarkFrontierBattleMonsSeen(void)
{
    if (!ShouldFrontierBattleMarkDexSeen(gBattleTypeFlags))
        return;

    // The player's whole (rented) team counts, benched mons included: the player has seen
    // their own party. Everyone else (opponents, a multi-battle partner) only once sent out.
    MarkWholePartyDexSeen(GetTrainerParty(B_TRAINER_PLAYER));
    for (enum BattleTrainer trainer = B_TRAINER_PLAYER + 1; trainer < MAX_BATTLE_TRAINERS; trainer++)
        MarkSentOutMonsDexSeen(GetTrainerParty(trainer), gBattleStruct->partyState[trainer]);
}
