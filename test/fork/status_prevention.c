#include "global.h"
#include "test/battle.h"

// Coverage for FEATURE_ANNOUNCE_STATUS_PREVENTION (config/feature.h, src/fork/status_prevention.c).
// Feature flags default off in the test baseline, so each test opts in with WITH_CONFIG. Nuzzle's
// paralysis is a 100% secondary effect, which keeps the chance roll out of most tests; Body Slam
// covers the roll itself.

SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_STATUS_PREVENTION: Limber announces a side-effect paralysis it blocked")
{
    bool32 announce;
    PARAMETRIZE { announce = TRUE; }
    PARAMETRIZE { announce = FALSE; }
    GIVEN {
        ASSUME(MoveHasAdditionalEffect(MOVE_NUZZLE, MOVE_EFFECT_PARALYSIS));
        WITH_CONFIG(FEATURE_ANNOUNCE_STATUS_PREVENTION, announce);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_PERSIAN) { Ability(ABILITY_LIMBER); }
    } WHEN {
        TURN { MOVE(player, MOVE_NUZZLE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_NUZZLE, player);
        if (announce)
        {
            ABILITY_POPUP(opponent, ABILITY_LIMBER);
            MESSAGE("The opposing Persian's Limber prevents paralysis!");
        }
        else
        {
            NONE_OF {
                ABILITY_POPUP(opponent, ABILITY_LIMBER);
                MESSAGE("The opposing Persian's Limber prevents paralysis!");
            }
        }
    } THEN {
        EXPECT_EQ(opponent->status1, STATUS1_NONE);
    }
}

// The reported case: Body Slam into Furret, whose Limber is an innate.
SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_STATUS_PREVENTION: an innate Limber is named, and only when Body Slam's roll succeeds")
{
    bool32 roll;
    PARAMETRIZE { roll = TRUE; }
    PARAMETRIZE { roll = FALSE; }
    GIVEN {
        ASSUME(MoveHasAdditionalEffect(MOVE_BODY_SLAM, MOVE_EFFECT_PARALYSIS));
        WITH_CONFIG(FEATURE_ANNOUNCE_STATUS_PREVENTION, TRUE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_FURRET) { Ability(ABILITY_HUSTLE); } // innate Limber
    } WHEN {
        TURN { MOVE(player, MOVE_BODY_SLAM, WITH_RNG(RNG_SECONDARY_EFFECT, roll)); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_BODY_SLAM, player);
        if (roll)
        {
            ABILITY_POPUP(opponent, ABILITY_LIMBER);
            MESSAGE("The opposing Furret's Limber prevents paralysis!");
        }
        else
        {
            NONE_OF {
                ABILITY_POPUP(opponent, ABILITY_LIMBER);
                MESSAGE("The opposing Furret's Limber prevents paralysis!");
            }
        }
    } THEN {
        EXPECT_EQ(opponent->status1, STATUS1_NONE);
    }
}

SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_STATUS_PREVENTION: each status gets its own message")
{
    enum Move move;
    enum Species species;
    enum Ability ability;
    PARAMETRIZE { move = MOVE_SCALD;          species = SPECIES_GOLDEEN;   ability = ABILITY_WATER_VEIL; }
    PARAMETRIZE { move = MOVE_POISON_JAB;     species = SPECIES_SNORLAX;   ability = ABILITY_IMMUNITY; }
    PARAMETRIZE { move = MOVE_RELIC_SONG;     species = SPECIES_DROWZEE;   ability = ABILITY_INSOMNIA; }
    PARAMETRIZE { move = MOVE_FREEZE_DRY;     species = SPECIES_SLUGMA;    ability = ABILITY_MAGMA_ARMOR; }
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_STATUS_PREVENTION, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(species) { Ability(ability); }
    } WHEN {
        TURN { MOVE(player, move, WITH_RNG(RNG_SECONDARY_EFFECT, TRUE)); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, move, player);
        ABILITY_POPUP(opponent, ability);
        if (ability == ABILITY_WATER_VEIL)
            MESSAGE("The opposing Goldeen's Water Veil prevents burns!");
        else if (ability == ABILITY_IMMUNITY)
            MESSAGE("The opposing Snorlax's Immunity prevents poisoning!");
        else if (ability == ABILITY_INSOMNIA)
            MESSAGE("The opposing Drowzee's Insomnia prevents sleep!");
        else
            MESSAGE("The opposing Slugma's Magma Armor prevents frostbite!"); // B_USE_FROSTBITE: Freeze-Dry frostbites
    } THEN {
        EXPECT_EQ(opponent->status1, STATUS1_NONE);
    }
}

SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_STATUS_PREVENTION: Shield Dust announces a status it blocked")
{
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_STATUS_PREVENTION, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_VENOMOTH) { Ability(ABILITY_SHIELD_DUST); }
    } WHEN {
        TURN { MOVE(player, MOVE_NUZZLE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_NUZZLE, player);
        ABILITY_POPUP(opponent, ABILITY_SHIELD_DUST);
        MESSAGE("The opposing Venomoth's Shield Dust prevents paralysis!");
    } THEN {
        EXPECT_EQ(opponent->status1, STATUS1_NONE);
    }
}

SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_STATUS_PREVENTION: Covert Cloak announces a status it blocked, without a pop-up")
{
    GIVEN {
        ASSUME(gItemsInfo[ITEM_COVERT_CLOAK].holdEffect == HOLD_EFFECT_COVERT_CLOAK);
        WITH_CONFIG(FEATURE_ANNOUNCE_STATUS_PREVENTION, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_COVERT_CLOAK); }
    } WHEN {
        TURN { MOVE(player, MOVE_NUZZLE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_NUZZLE, player);
        NOT ABILITY_POPUP(opponent);
        MESSAGE("The opposing Wobbuffet's Covert Cloak prevents paralysis!");
    } THEN {
        EXPECT_EQ(opponent->status1, STATUS1_NONE);
    }
}

// Nothing to announce when the status could not have landed anyway: the target already has a
// status, is immune by type, is on Misty Terrain, or the attacker's Sheer Force removed the effect.
SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_STATUS_PREVENTION: stays quiet when the status would not have landed")
{
    enum Species species;
    enum Ability ability;
    enum Ability attackerAbility;
    enum Item item;
    u32 status;
    enum Move move;
    PARAMETRIZE { species = SPECIES_PERSIAN;  ability = ABILITY_LIMBER;      attackerAbility = ABILITY_TELEPATHY;    item = ITEM_NONE;          status = STATUS1_BURN; move = MOVE_NUZZLE; }
    PARAMETRIZE { species = SPECIES_VENOMOTH; ability = ABILITY_SHIELD_DUST; attackerAbility = ABILITY_TELEPATHY;    item = ITEM_NONE;          status = STATUS1_BURN; move = MOVE_NUZZLE; }
    PARAMETRIZE { species = SPECIES_CHARMANDER; ability = ABILITY_BLAZE;     attackerAbility = ABILITY_TELEPATHY;    item = ITEM_COVERT_CLOAK;  status = STATUS1_NONE; move = MOVE_SCALD; }
    PARAMETRIZE { species = SPECIES_PERSIAN;  ability = ABILITY_LIMBER;      attackerAbility = ABILITY_SHEER_FORCE;  item = ITEM_NONE;          status = STATUS1_NONE; move = MOVE_BODY_SLAM; }
    PARAMETRIZE { species = SPECIES_PERSIAN;  ability = ABILITY_LIMBER;      attackerAbility = ABILITY_MISTY_SURGE;  item = ITEM_NONE;          status = STATUS1_NONE; move = MOVE_NUZZLE; }
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_STATUS_PREVENTION, TRUE);
        PLAYER(attackerAbility == ABILITY_SHEER_FORCE ? SPECIES_NIDOKING
             : attackerAbility == ABILITY_MISTY_SURGE ? SPECIES_TAPU_FINI : SPECIES_WOBBUFFET) { Ability(attackerAbility); }
        OPPONENT(species) { Ability(ability); Item(item); Status1(status); }
    } WHEN {
        TURN { MOVE(player, move, WITH_RNG(RNG_SECONDARY_EFFECT, TRUE)); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, move, player);
        NONE_OF {
            ABILITY_POPUP(opponent);
            MESSAGE("The opposing Persian's Limber prevents paralysis!");
            MESSAGE("The opposing Venomoth's Shield Dust prevents paralysis!");
            MESSAGE("The opposing Charmander's Covert Cloak prevents burns!");
        }
    }
}

// A status move already announces its own immunity; the feature must not add a second message.
SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_STATUS_PREVENTION: Thunder Wave keeps its stock message only")
{
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_STATUS_PREVENTION, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_PERSIAN) { Ability(ABILITY_LIMBER); }
    } WHEN {
        TURN { MOVE(player, MOVE_THUNDER_WAVE); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_LIMBER);
        MESSAGE("It doesn't affect the opposing Persian…");
        NOT MESSAGE("The opposing Persian's Limber prevents paralysis!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_STATUS_PREVENTION: Mold Breaker pierces Limber, so nothing is announced")
{
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_STATUS_PREVENTION, TRUE);
        PLAYER(SPECIES_PINSIR) { Ability(ABILITY_MOLD_BREAKER); }
        OPPONENT(SPECIES_PERSIAN) { Ability(ABILITY_LIMBER); }
    } WHEN {
        TURN { MOVE(player, MOVE_NUZZLE); }
    } SCENE {
        NONE_OF {
            ABILITY_POPUP(opponent, ABILITY_LIMBER);
            MESSAGE("The opposing Persian's Limber prevents paralysis!");
        }
        STATUS_ICON(opponent, paralysis: TRUE);
    }
}

// A side-wide blocker is named on the ally that holds it.
DOUBLE_BATTLE_TEST("FEATURE_ANNOUNCE_STATUS_PREVENTION: an ally's Flower Veil is named on the ally")
{
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_STATUS_PREVENTION, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_ODDISH);
        OPPONENT(SPECIES_COMFEY) { Ability(ABILITY_FLOWER_VEIL); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_NUZZLE, target: opponentLeft); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_NUZZLE, playerLeft);
        ABILITY_POPUP(opponentRight, ABILITY_FLOWER_VEIL);
        MESSAGE("The opposing Comfey's Flower Veil prevents paralysis!");
    } THEN {
        EXPECT_EQ(opponentLeft->status1, STATUS1_NONE);
    }
}

// Tri Attack picks its status first and then applies it; the announcement names the one it picked.
// DETERMINISTIC_MOVE_RESULTS picks paralysis on an Attack / Sp. Atk tie.
SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_STATUS_PREVENTION: Tri Attack's picked status is announced")
{
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_STATUS_PREVENTION, TRUE);
        WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_PERSIAN) { Ability(ABILITY_LIMBER); Attack(100); SpAttack(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_TRI_ATTACK, WITH_RNG(RNG_SECONDARY_EFFECT, TRUE)); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TRI_ATTACK, player);
        ABILITY_POPUP(opponent, ABILITY_LIMBER);
        MESSAGE("The opposing Persian's Limber prevents paralysis!");
    }
}
