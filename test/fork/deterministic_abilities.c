#include "global.h"
#include "test/battle.h"

// FORK: tests for DETERMINISTIC_ABILITIES. The per-test baseline forces every
// DETERMINISTIC_* flag off, so each test opts in explicitly with WITH_CONFIG.
// With the flag on, the chance-based ability effects below are guaranteed, so
// they need no WITH_RNG/PASSES_RANDOMLY.

SINGLE_BATTLE_TEST("DETERMINISTIC_ABILITIES: Static always paralyzes on contact")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        ASSUME(MoveMakesContact(MOVE_TACKLE));
        PLAYER(SPECIES_PIKACHU) { Ability(ABILITY_STATIC); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_TACKLE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_STATIC);
    } THEN {
        EXPECT(opponent->status1 & STATUS1_PARALYSIS);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ABILITIES: Poison Point always poisons on contact")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        ASSUME(MoveMakesContact(MOVE_TACKLE));
        PLAYER(SPECIES_NIDORAN_F) { Ability(ABILITY_POISON_POINT); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_TACKLE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_POISON_POINT);
    } THEN {
        EXPECT(opponent->status1 & STATUS1_POISON);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ABILITIES: Poison Touch always poisons on contact")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        ASSUME(MoveMakesContact(MOVE_TACKLE));
        PLAYER(SPECIES_CROAGUNK) { Ability(ABILITY_POISON_TOUCH); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_POISON_TOUCH);
    } THEN {
        EXPECT(opponent->status1 & STATUS1_POISON);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ABILITIES: Flame Body always burns on contact")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        ASSUME(MoveMakesContact(MOVE_TACKLE));
        PLAYER(SPECIES_MAGMAR) { Ability(ABILITY_FLAME_BODY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_TACKLE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_FLAME_BODY);
    } THEN {
        EXPECT(opponent->status1 & STATUS1_BURN);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ABILITIES: Cute Charm infatuates on contact regardless of gender")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        ASSUME(MoveMakesContact(MOVE_TACKLE));
        PLAYER(SPECIES_WOBBUFFET) { Gender(MON_MALE); }
        OPPONENT(SPECIES_CLEFAIRY) { Gender(MON_MALE); Ability(ABILITY_CUTE_CHARM); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_CUTE_CHARM);
        MESSAGE("Wobbuffet fell in love!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ABILITIES: Effect Spore always makes the attacker drowsy on contact")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        ASSUME(MoveMakesContact(MOVE_TACKLE));
        PLAYER(SPECIES_PARAS) { Ability(ABILITY_EFFECT_SPORE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_TACKLE); }
        TURN {}
    } SCENE {
        ABILITY_POPUP(player, ABILITY_EFFECT_SPORE);
        MESSAGE("The opposing Wobbuffet grew drowsy!");
    } THEN {
        EXPECT(opponent->status1 & STATUS1_SLEEP);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ABILITIES: Shed Skin always cures status at end of turn")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        PLAYER(SPECIES_SHEDINJA) { Ability(ABILITY_SHED_SKIN); Status1(STATUS1_BURN); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN {}
    } SCENE {
        ABILITY_POPUP(player, ABILITY_SHED_SKIN);
    } THEN {
        EXPECT(!(player->status1 & STATUS1_ANY));
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ABILITIES: Cursed Body always disables the used move")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_FRILLISH) { Ability(ABILITY_CURSED_BODY); } // Aqua Jet hits its Water/Ghost typing
    } WHEN {
        TURN { MOVE(player, MOVE_AQUA_JET); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_CURSED_BODY);
        MESSAGE("Wobbuffet's Aqua Jet was disabled!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ABILITIES: Toxic Chain always badly poisons")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        PLAYER(SPECIES_OKIDOGI) { Ability(ABILITY_TOXIC_CHAIN); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } THEN {
        EXPECT(opponent->status1 & STATUS1_TOXIC_POISON);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ABILITIES: Stench flinches on the first turn but not after")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        ASSUME(GetMovePower(MOVE_SCRATCH) > 0);
        PLAYER(SPECIES_GRIMER) { Ability(ABILITY_STENCH); Speed(50); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(10); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_SCRATCH); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!"); // turn 1 only
        NONE_OF { MESSAGE("The opposing Wobbuffet flinched and couldn't move!"); }
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ABILITIES: Moody raises the lowest-valued stat and lowers the highest")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        // Explicit, distinct stats: Sp. Def is uniquely lowest, Speed uniquely
        // highest, so the raise/lower targets are unambiguous.
        PLAYER(SPECIES_WOBBUFFET) {
            Ability(ABILITY_MOODY);
            Attack(100); Defense(110); SpAttack(120); SpDefense(60); Speed(130);
        }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); }
    } WHEN {
        TURN {}
    } SCENE {
        ABILITY_POPUP(player, ABILITY_MOODY);
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPDEF], DEFAULT_STAT_STAGE + 2); // lowest -> +2
        EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE - 1); // highest -> -1
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE);
        EXPECT_EQ(player->statStages[STAT_DEF], DEFAULT_STAT_STAGE);
        EXPECT_EQ(player->statStages[STAT_SPATK], DEFAULT_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ABILITIES: Rivalry boosts when sharing a type and reduces otherwise", s16 damage)
{
    u16 target;
    enum Ability ability;
    PARAMETRIZE { target = SPECIES_SNORLAX; ability = ABILITY_IMMUNITY; }  // shared type baseline
    PARAMETRIZE { target = SPECIES_SNORLAX; ability = ABILITY_RIVALRY; }   // shared type (Normal)
    PARAMETRIZE { target = SPECIES_VAPOREON; ability = ABILITY_IMMUNITY; } // no shared type baseline
    PARAMETRIZE { target = SPECIES_VAPOREON; ability = ABILITY_RIVALRY; }  // no shared type (Water vs Normal)

    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        ASSUME(GetMoveType(MOVE_SCRATCH) == TYPE_NORMAL);
        PLAYER(SPECIES_SNORLAX) { Ability(ability); }
        OPPONENT(target);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.25), results[1].damage); // shared -> x1.25
        EXPECT_MUL_EQ(results[2].damage, Q_4_12(0.75), results[3].damage); // none -> x0.75
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ABILITIES: Harvest recovers a used berry and heals 1/16 in the sun")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        ASSUME(gItemsInfo[ITEM_SITRUS_BERRY].holdEffect == HOLD_EFFECT_RESTORE_PCT_HP);
        PLAYER(SPECIES_EXEGGUTOR) { Ability(ABILITY_HARVEST); MaxHP(500); HP(251); Item(ITEM_SITRUS_BERRY); }
        OPPONENT(SPECIES_TORKOAL) { Ability(ABILITY_DROUGHT); } // sets harsh sunlight
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); } // drops Exeggutor below half so Sitrus is consumed
    } SCENE {
        ABILITY_POPUP(player, ABILITY_HARVEST);
        MESSAGE("Exeggutor harvested its Sitrus Berry!");
        MESSAGE("Exeggutor restored a little HP!");
    } THEN {
        EXPECT_EQ(player->item, ITEM_SITRUS_BERRY);
    }
}

DOUBLE_BATTLE_TEST("DETERMINISTIC_ABILITIES: Trace copies the directly-opposing foe's ability")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_GARDEVOIR) { Ability(ABILITY_TRACE); } // playerRight, flank 1
        OPPONENT(SPECIES_PIKACHU) { Ability(ABILITY_STATIC); }   // opponentLeft, flank 0
        OPPONENT(SPECIES_MAGMAR) { Ability(ABILITY_FLAME_BODY); } // opponentRight, flank 1
    } WHEN {
        TURN {}
    } SCENE {
        // Gardevoir is on the right flank, so it copies the right-flank foe (Magmar).
        ABILITY_POPUP(playerRight, ABILITY_TRACE);
        MESSAGE("It traced the opposing Magmar's Flame Body!");
    }
}

AI_SINGLE_BATTLE_TEST("DETERMINISTIC_ABILITIES: AI avoids contact into a guaranteed contact-status ability")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE); // remove accuracy from move scoring
        ASSUME(MoveMakesContact(MOVE_CUT));
        ASSUME(!MoveMakesContact(MOVE_ROCK_THROW));
        ASSUME(GetMovePower(MOVE_CUT) == GetMovePower(MOVE_ROCK_THROW));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT); // OMNISCIENT so the AI knows Static
        // Pikachu is pure Electric (neutral to Normal & Rock), so Cut and Rock Throw
        // deal equal damage; the only difference is Cut's guaranteed Static paralysis.
        PLAYER(SPECIES_PIKACHU) { Ability(ABILITY_STATIC); HP(300); MaxHP(300); }
        OPPONENT(SPECIES_MACHOP) { Moves(MOVE_CUT, MOVE_ROCK_THROW); }
    } WHEN {
        TURN { SCORE_LT(opponent, MOVE_CUT, MOVE_ROCK_THROW); }
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ABILITIES: a Stench holder's King's Rock is not consumed when it attacks on the first turn")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        ASSUME(gItemsInfo[ITEM_KINGS_ROCK].holdEffect == HOLD_EFFECT_FLINCH);
        ASSUME(GetMovePower(MOVE_SCRATCH) > 0);
        PLAYER(SPECIES_GRIMER) { Ability(ABILITY_STENCH); Item(ITEM_KINGS_ROCK); Speed(50); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(10); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!"); // Stench's flinch, not King's Rock
    } THEN {
        EXPECT_EQ(player->item, ITEM_KINGS_ROCK); // King's Rock is left untouched (Stench pre-empts it)
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ABILITIES: Quick Draw always moves first on the entry turn, never after")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        PLAYER(SPECIES_SLOWBRO_GALAR) { Ability(ABILITY_QUICK_DRAW); Speed(1); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CELEBRATE); MOVE(player, MOVE_SCRATCH); }
        TURN { MOVE(opponent, MOVE_CELEBRATE); MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        // Turn 1 (entry turn): Quick Draw activates and the slower holder moves first.
        ABILITY_POPUP(player, ABILITY_QUICK_DRAW);
        MESSAGE("Slowbro used Scratch!");
        MESSAGE("The opposing Wobbuffet used Celebrate!");
        // Turn 2: not the entry turn, so Quick Draw stays silent and the holder is slow.
        MESSAGE("The opposing Wobbuffet used Celebrate!");
        MESSAGE("Slowbro used Scratch!");
    }
}

AI_SINGLE_BATTLE_TEST("DETERMINISTIC_ABILITIES: AI knows Quick Draw lets it move first on its entry turn")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        ASSUME(GetMoveEffect(MOVE_DESTINY_BOND) == EFFECT_DESTINY_BOND);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        // The AI is far slower by raw speed, but Quick Draw guarantees it moves first
        // on its entry turn. Since the foe can then KO it, the AI should value Destiny
        // Bond (which is only scored up when the AI predicts going first), proving it
        // models the Quick Draw turn-order override.
        PLAYER(SPECIES_WOBBUFFET) { Speed(200); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_SLOWBRO_GALAR) { Ability(ABILITY_QUICK_DRAW); Speed(1); HP(1); MaxHP(200); Moves(MOVE_DESTINY_BOND, MOVE_SCRATCH); }
    } WHEN {
        TURN { SCORE_GT(opponent, MOVE_DESTINY_BOND, MOVE_SCRATCH); }
    }
}

AI_SINGLE_BATTLE_TEST("DETERMINISTIC_ABILITIES: AI prefers a contact move so Poison Touch guarantees poison")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE); // remove accuracy from move scoring
        ASSUME(MoveMakesContact(MOVE_CUT));
        ASSUME(!MoveMakesContact(MOVE_ROCK_THROW));
        ASSUME(GetMovePower(MOVE_CUT) == GetMovePower(MOVE_ROCK_THROW));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        // Pikachu is pure Electric (poisonable, neutral to Normal & Rock), so Cut and
        // Rock Throw deal equal damage; the only difference is Cut's guaranteed poison.
        // Illuminate keeps it from punishing contact, isolating the Poison Touch plus.
        PLAYER(SPECIES_PIKACHU) { Ability(ABILITY_ILLUMINATE); HP(300); MaxHP(300); }
        OPPONENT(SPECIES_GRIMER) { Ability(ABILITY_POISON_TOUCH); Moves(MOVE_CUT, MOVE_ROCK_THROW); }
    } WHEN {
        TURN { SCORE_GT(opponent, MOVE_CUT, MOVE_ROCK_THROW); }
    }
}

AI_SINGLE_BATTLE_TEST("DETERMINISTIC_ABILITIES: AI won't status a known Shed Skin target it can't keep statused")
{
    u32 move;
    PARAMETRIZE { move = MOVE_TOXIC; }
    PARAMETRIZE { move = MOVE_THUNDER_WAVE; }
    PARAMETRIZE { move = MOVE_WILL_O_WISP; }
    PARAMETRIZE { move = MOVE_HYPNOSIS; }

    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT); // OMNISCIENT so the AI knows Shed Skin
        // Dratini is pure Dragon, so it can take any of the four non-volatile statuses;
        // Shed Skin then cures it at the end of every turn, so each status move is wasted.
        PLAYER(SPECIES_DRATINI) { Ability(ABILITY_SHED_SKIN); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE, move); }
    } WHEN {
        TURN { SCORE_EQ(opponent, MOVE_CELEBRATE, move); } // status is doomed -> -10, same as do-nothing Celebrate
    }
}

AI_SINGLE_BATTLE_TEST("DETERMINISTIC_ABILITIES: AI won't status a known Hydration target while it is raining")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_VAPOREON) { Ability(ABILITY_HYDRATION); Moves(MOVE_RAIN_DANCE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE, MOVE_TOXIC); }
    } WHEN {
        TURN { MOVE(player, MOVE_RAIN_DANCE); } // bring up rain, so Hydration will cure end of turn
        TURN { SCORE_EQ(opponent, MOVE_CELEBRATE, MOVE_TOXIC); } // in rain the poison is doomed -> -10, same as Celebrate
    }
}

AI_SINGLE_BATTLE_TEST("DETERMINISTIC_ABILITIES: AI still statuses a known Hydration target when it is not raining")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        // No rain, so Hydration stays dormant and poison sticks - the AI should value it.
        PLAYER(SPECIES_VAPOREON) { Ability(ABILITY_HYDRATION); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE, MOVE_TOXIC); }
    } WHEN {
        TURN { SCORE_GT(opponent, MOVE_TOXIC, MOVE_CELEBRATE); }
    }
}
