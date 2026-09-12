#include "global.h"
#include "test/battle.h"

// FORK: coverage for the BUFF_SIGNATURE_TYPE_ITEMS flag (config/buff.h). BUFF_* flags
// default off in the test baseline (see TestInitConfigData), so each test opts in with
// WITH_CONFIG explicitly. The flag has two halves -- it LOCKS the Plate/Memory/Drive
// boost to Arceus/Silvally/Genesect, and it raises the whole signature class onto
// BUFF_TYPE_BOOST_PERCENT -- so both are covered here, in both flag states.
//
// On forme and STAB: a Memory and a Plate maintain their holder's forme in the field
// (FORM_CHANGE_ITEM_HOLD), so it would be reasonable to expect an ITEM_NONE control to
// revert to Normal and lose STAB as well as the item boost. It does not -- the harness
// builds the mon at the species PLAYER() names and does not re-run the item-hold form
// check, so the control keeps its type and each test below isolates the multiplier
// alone. Measured, not assumed: the Silvally pair reads 97 against 70, exactly 1.4.

ASSUMPTIONS
{
    ASSUME(gItemsInfo[ITEM_FLAME_PLATE].holdEffect == HOLD_EFFECT_PLATE);
    ASSUME(gItemsInfo[ITEM_FIRE_MEMORY].holdEffect == HOLD_EFFECT_MEMORY);
    ASSUME(gItemsInfo[ITEM_SHOCK_DRIVE].holdEffect == HOLD_EFFECT_DRIVE);
    ASSUME(GetItemSecondaryId(ITEM_FLAME_PLATE) == TYPE_FIRE);
    ASSUME(GetItemSecondaryId(ITEM_FIRE_MEMORY) == TYPE_FIRE);
    ASSUME(GetItemSecondaryId(ITEM_SHOCK_DRIVE) == TYPE_ELECTRIC);
    ASSUME(GetMoveType(MOVE_EMBER) == TYPE_FIRE);
    ASSUME(GetMoveType(MOVE_THUNDER_SHOCK) == TYPE_ELECTRIC);
}

SINGLE_BATTLE_TEST("BUFF_SIGNATURE_TYPE_ITEMS: a Memory gives Silvally the type-item boost", s16 damage)
{
    u32 item;

    PARAMETRIZE { item = ITEM_FIRE_MEMORY; }
    PARAMETRIZE { item = ITEM_NONE; }

    GIVEN {
        WITH_CONFIG(BUFF_SIGNATURE_TYPE_ITEMS, TRUE);
        WITH_CONFIG(BUFF_TYPE_BOOST_ITEMS, TRUE);
        PLAYER(SPECIES_SILVALLY_FIRE) { Item(item); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_EMBER); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.4), results[0].damage);
    }
}

SINGLE_BATTLE_TEST("BUFF_SIGNATURE_TYPE_ITEMS: stock gives a Memory no multiplier at all", s16 damage)
{
    u32 item;

    PARAMETRIZE { item = ITEM_FIRE_MEMORY; }
    PARAMETRIZE { item = ITEM_NONE; }

    GIVEN {
        WITH_CONFIG(BUFF_SIGNATURE_TYPE_ITEMS, FALSE);
        WITH_CONFIG(BUFF_TYPE_BOOST_ITEMS, TRUE);
        PLAYER(SPECIES_SILVALLY_FIRE) { Item(item); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_EMBER); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        // Nothing separates the runs at all: stock gives a Memory no multiplier.
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.0), results[0].damage);
    }
}

SINGLE_BATTLE_TEST("BUFF_SIGNATURE_TYPE_ITEMS: a Drive gives Genesect the type-item boost", s16 damage)
{
    u32 item;

    PARAMETRIZE { item = ITEM_SHOCK_DRIVE; }
    PARAMETRIZE { item = ITEM_NONE; }

    GIVEN {
        WITH_CONFIG(BUFF_SIGNATURE_TYPE_ITEMS, TRUE);
        WITH_CONFIG(BUFF_TYPE_BOOST_ITEMS, TRUE);
        PLAYER(SPECIES_GENESECT) { Item(item); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_THUNDER_SHOCK); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.4), results[0].damage);
    }
}

SINGLE_BATTLE_TEST("BUFF_SIGNATURE_TYPE_ITEMS: a Memory does nothing for a species that is not Silvally", s16 damage)
{
    u32 item;

    PARAMETRIZE { item = ITEM_FIRE_MEMORY; }
    PARAMETRIZE { item = ITEM_NONE; }

    GIVEN {
        WITH_CONFIG(BUFF_SIGNATURE_TYPE_ITEMS, TRUE);
        WITH_CONFIG(BUFF_TYPE_BOOST_ITEMS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(item); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_EMBER); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.0), results[0].damage);
    }
}

SINGLE_BATTLE_TEST("BUFF_SIGNATURE_TYPE_ITEMS: a Plate still boosts Arceus itself", s16 damage)
{
    u32 item;

    PARAMETRIZE { item = ITEM_FLAME_PLATE; }
    PARAMETRIZE { item = ITEM_NONE; }

    GIVEN {
        WITH_CONFIG(BUFF_SIGNATURE_TYPE_ITEMS, TRUE);
        WITH_CONFIG(BUFF_TYPE_BOOST_ITEMS, TRUE);
        PLAYER(SPECIES_ARCEUS_FIRE) { Item(item); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_EMBER); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.4), results[0].damage);
    }
}

SINGLE_BATTLE_TEST("BUFF_SIGNATURE_TYPE_ITEMS: a Plate is locked to Arceus", s16 damage)
{
    u32 item;

    PARAMETRIZE { item = ITEM_FLAME_PLATE; }
    PARAMETRIZE { item = ITEM_NONE; }

    GIVEN {
        WITH_CONFIG(BUFF_SIGNATURE_TYPE_ITEMS, TRUE);
        WITH_CONFIG(BUFF_TYPE_BOOST_ITEMS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(item); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_EMBER); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.0), results[0].damage);
    }
}

SINGLE_BATTLE_TEST("BUFF_SIGNATURE_TYPE_ITEMS: stock lets any holder use a Plate", s16 damage)
{
    u32 item;

    PARAMETRIZE { item = ITEM_FLAME_PLATE; }
    PARAMETRIZE { item = ITEM_NONE; }

    GIVEN {
        WITH_CONFIG(BUFF_SIGNATURE_TYPE_ITEMS, FALSE);
        WITH_CONFIG(BUFF_TYPE_BOOST_ITEMS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(item); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_EMBER); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.4), results[0].damage);
    }
}

SINGLE_BATTLE_TEST("BUFF_SIGNATURE_TYPE_ITEMS: Soul Dew moves off the stock 20% onto the type-item scale", s16 damage)
{
    u32 config;

    PARAMETRIZE { config = TRUE; }
    PARAMETRIZE { config = FALSE; }

    GIVEN {
        ASSUME(B_SOUL_DEW_BOOST >= GEN_7);
        ASSUME(gItemsInfo[ITEM_SOUL_DEW].holdEffect == HOLD_EFFECT_SOUL_DEW);
        WITH_CONFIG(BUFF_SIGNATURE_TYPE_ITEMS, config);
        WITH_CONFIG(BUFF_TYPE_BOOST_ITEMS, TRUE);
        PLAYER(SPECIES_LATIOS) { Item(ITEM_SOUL_DEW); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_DRAGON_BREATH); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        // 1.4 buffed against 1.2 stock on the same Dragon STAB move.
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.4 / 1.2), results[0].damage);
    }
}

// FORK: the Soul Dew gate reads the holder by BASE species, so a Mega Latias/Latios keeps the
// boost. Upstream's exact-species test is unreachable-safe there (a Mega Stone and Soul Dew
// cannot share the slot), but FEATURE_FREE_GIMMICKS drops the stone requirement and makes the
// case live: without this, a Soul Dew Latios that Mega Evolves silently loses its own item.
SINGLE_BATTLE_TEST("BUFF_SIGNATURE_TYPE_ITEMS: Mega Latios keeps the Soul Dew boost", s16 damage)
{
    u32 item;

    PARAMETRIZE { item = ITEM_SOUL_DEW; }
    PARAMETRIZE { item = ITEM_NONE; }

    GIVEN {
        ASSUME(B_SOUL_DEW_BOOST >= GEN_7);
        WITH_CONFIG(BUFF_SIGNATURE_TYPE_ITEMS, TRUE);
        WITH_CONFIG(BUFF_TYPE_BOOST_ITEMS, TRUE);
        PLAYER(SPECIES_LATIOS_MEGA) { Item(item); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_DRAGON_BREATH); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.4), results[0].damage);
    }
}
