#include "global.h"
#include "test/battle.h"
#include "fork/innate_abilities.h"

// Berserk Gene against an innate Own Tempo. The extended frontier roster gives the
// Gene to Own Tempo innate holders (Mudsdale, Hisuian Lilligant) on the premise that
// the innate blocks the Gene's permanent confusion and leaves only the +2 Attack;
// these tests are the record of that premise. The chosen ability is deliberately not
// Own Tempo, so only the innate can be doing the blocking.

SINGLE_BATTLE_TEST("Berserk Gene: an innate Own Tempo blocks the confusion but keeps the +2 Attack")
{
    GIVEN {
        ASSUME(gItemsInfo[ITEM_BERSERK_GENE].holdEffect == HOLD_EFFECT_BERSERK_GENE);
        ASSUME(SpeciesHasInnate(SPECIES_MUDSDALE, ABILITY_OWN_TEMPO));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_MUDSDALE) { Ability(ABILITY_STAMINA); Item(ITEM_BERSERK_GENE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("The Berserk Gene sharply boosted Mudsdale's Attack!");
        NOT MESSAGE("Mudsdale became confused!");
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE + 2);
        EXPECT(player->volatiles.confusionTimer == 0);
        EXPECT_EQ(player->item, ITEM_NONE);
    }
}

SINGLE_BATTLE_TEST("Berserk Gene: without innates the same holder is confused (control)")
{
    GIVEN {
        ASSUME(gItemsInfo[ITEM_BERSERK_GENE].holdEffect == HOLD_EFFECT_BERSERK_GENE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, FALSE);
        PLAYER(SPECIES_MUDSDALE) { Ability(ABILITY_STAMINA); Item(ITEM_BERSERK_GENE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("The Berserk Gene sharply boosted Mudsdale's Attack!");
        MESSAGE("Mudsdale became confused!");
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE + 2);
        EXPECT(player->volatiles.confusionTimer != 0);
    }
}
