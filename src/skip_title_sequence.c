#include "global.h"
#include "load_save.h"
#include "save.h"
#include "new_game.h"
#include "m4a.h"
#include "malloc.h"
#include "skip_title_sequence.h"

// Loads the save file and finishes boot-time setup. LoadGameSave does a
// blocking flash read, so this is the slow part of boot. It is shared by the
// normal boot path (called from CB2_InitCopyrightScreenAfterBootup) and the
// SKIP_TITLE_SEQUENCE path, which defers it until the RHH logo is on screen
// (see Task_HandleExpansionIntro) so the freeze hides behind the displayed
// logo - the way the vanilla copyright screen hides it behind its own logo.
//
// It lives in this file (rather than inline in intro.c) so the skip feature
// touches that upstream file as little as possible.
void LoadGameSaveAfterBootup(void)
{
    SetSaveBlocksPointers(GetSaveBlocksPointersBaseOffset());
    ResetMenuAndMonGlobals();
    Save_ResetSaveCounters();
    LoadGameSave(SAVE_NORMAL);
    if (gSaveFileStatus == SAVE_STATUS_EMPTY || gSaveFileStatus == SAVE_STATUS_CORRUPT)
        Sav2_ClearSetDefault();
    SetPokemonCryStereo(gSaveBlock2Ptr->optionsSound);
    InitHeap(gHeap, HEAP_SIZE);
}
