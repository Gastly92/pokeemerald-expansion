#include "global.h"
#include "pokemon_storage_system.h"
#include "test/test.h"

// If you would like to ensure save compatibility, update the values below with those for your hack. You can find these through the debug menu.
// Please note that this simple check is not 100% foolproof, but should be able to catch most unintended shifts.
// FORK: sizes diverge from upstream due to our sandbox config:
// - FRONTIER_PARTY_SIZE 6 grows MAX_FRONTIER_PARTY_SIZE to 6, enlarging the
//   SaveBlock2 tower-record party arrays (+532) and the 6v6 Factory rentalMons.
// - Many FREE_* flags in config/save.h reclaim space (notably
//   FREE_RECORD_MIXING_HALL_RECORDS in SaveBlock2 and the SaveBlock1 frees).
#define T_SAVEBLOCK1_SIZE 13152
#define T_SAVEBLOCK2_SIZE 3300
#define T_SAVEBLOCK3_SIZE 4
#define T_POKEMONSTORAGE_SIZE 34144

TEST("SaveBlock1 is backwards compatible")
{
    EXPECT_EQ(sizeof(struct SaveBlock1), T_SAVEBLOCK1_SIZE);
}

TEST("SaveBlock2 is backwards compatible")
{
    EXPECT_EQ(sizeof(struct SaveBlock2), T_SAVEBLOCK2_SIZE);
}

TEST("SaveBlock3 is backwards compatible")
{
    EXPECT_EQ(sizeof(struct SaveBlock3), T_SAVEBLOCK3_SIZE);
}

TEST("PokemonStorage is backwards compatible")
{
    EXPECT_EQ(sizeof(struct PokemonStorage), T_POKEMONSTORAGE_SIZE);
}

#undef T_SAVEBLOCK1_SIZE
#undef T_SAVEBLOCK2_SIZE
#undef T_SAVEBLOCK3_SIZE
#undef T_POKEMONSTORAGE_SIZE
