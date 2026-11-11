#include "global.h"
#include "event_data.h"
#include "main.h"
#include "overworld.h"
#include "pokedex.h"
#include "test/test.h"

// START_WITH_NATIONAL_POKEDEX gives a new game the Pokédex before anything is seen, so the
// START menu lets it open with an empty list. CreatePokedexList() pads that list with one
// unseen row; without it the list screen's "pokemonListCount - 1" bounds underflow.

#define FRAME_LIMIT 600

// Runs the Pokédex's main callback for a number of frames, pressing `keys` on the first.
static void RunDexFrames(u16 keys, u32 frames)
{
    u32 i;
    for (i = 0; i < frames && gMain.callback2 != CB2_ReturnToFieldWithOpenMenu; i++)
    {
        gMain.newKeys = (i == 0) ? keys : 0;
        gMain.heldKeys = gMain.newKeys;
        gMain.newAndRepeatedKeys = gMain.newKeys;
        gMain.callback2();
    }
}

TEST("Pokedex: opens and closes with nothing seen")
{
    MainCallback testRunner = gMain.callback2;
    u32 frames;
    bool32 closed;

    ResetPokedex();
    EnableNationalPokedex();
    EXPECT_EQ(GetNationalPokedexCount(FLAG_GET_SEEN), 0);

    gMain.state = 0;
    SetMainCallback2(CB2_OpenPokedex);
    RunDexFrames(0, 120);

    // Browse the empty list: scroll both ways, A on the unseen row, then
    // START menu -> LIST BOTTOM (selectedPokemon = pokemonListCount - 1).
    RunDexFrames(DPAD_DOWN, 20);
    RunDexFrames(DPAD_UP, 20);
    RunDexFrames(R_BUTTON, 20);
    RunDexFrames(A_BUTTON, 60);
    RunDexFrames(START_BUTTON, 20);
    RunDexFrames(DPAD_DOWN, 10);
    RunDexFrames(DPAD_DOWN, 10);
    RunDexFrames(A_BUTTON, 30);
    RunDexFrames(A_BUTTON, 60);

    // B closes the Pokédex, which frees its view and hands back to the field.
    RunDexFrames(B_BUTTON, 1);
    for (frames = 0; frames < FRAME_LIMIT && gMain.callback2 != CB2_ReturnToFieldWithOpenMenu; frames++)
        RunDexFrames(0, 1);

    closed = (gMain.callback2 == CB2_ReturnToFieldWithOpenMenu);
    gMain.state = 0;
    SetMainCallback2(testRunner);
    EXPECT(closed);
}
