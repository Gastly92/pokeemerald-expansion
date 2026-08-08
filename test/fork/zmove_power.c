#include "global.h"
#include "test/battle.h"
#include "battle_z_move.h"

// FORK: Z-Move base power regression tests.
//
// CalcMoveBasePower used to read the global gCurrentMove for the Z-Move's base power.
// By the time the damage calc runs, that global holds the *Z-Move*, whose data entry
// carries a placeholder .power of 1, so GetZMovePower bottomed out at a flat 100 BP for
// every type-based Z-Move. The move-selection preview derived the power from the base
// move and was correct, so the menu advertised (say) 185 while the engine dealt 100.
//
// The AI hit the same line from the other side: it simulates every move with the Z gimmick
// forced on while gCurrentMove is MOVE_NONE (reset by HandleAction_ActionFinished), so all
// of its moves were scored at an identical 100 BP - collapsing its hits-to-KO ranking into
// ties that got broken at random.
//
// These pin the behaviour the tier table in GetZMovePower actually specifies.

SINGLE_BATTLE_TEST("Z-MOVE FORK: Z-Move power scales with the base move's power", s16 damage)
{
    u32 move;
    PARAMETRIZE { move = MOVE_TACKLE; }      //  40 BP -> Breakneck Blitz 100
    PARAMETRIZE { move = MOVE_GIGA_IMPACT; } // 150 BP -> Breakneck Blitz 200
    GIVEN {
        ASSUME(GetMovePower(MOVE_TACKLE) == 40);
        ASSUME(GetMovePower(MOVE_GIGA_IMPACT) == 150);
        ASSUME(GetMoveType(MOVE_TACKLE) == TYPE_NORMAL);
        ASSUME(GetMoveType(MOVE_GIGA_IMPACT) == TYPE_NORMAL);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_NORMALIUM_Z); Moves(MOVE_TACKLE, MOVE_GIGA_IMPACT); }
        OPPONENT(SPECIES_CHANSEY) { MaxHP(700); HP(700); }
    } WHEN {
        TURN { MOVE(player, move, gimmick: GIMMICK_Z_MOVE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_BREAKNECK_BLITZ, player);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        // 100 BP vs 200 BP. Before the fix both dealt the same damage.
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(2.0), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("Z-MOVE FORK: the power the menu advertises is the power that is dealt", s16 damage)
{
    u32 move;
    PARAMETRIZE { move = MOVE_EMBER; }      //  40 BP -> Inferno Overdrive 100
    PARAMETRIZE { move = MOVE_FIRE_BLAST; } // 110 BP -> Inferno Overdrive 185
    GIVEN {
        ASSUME(GetMovePower(MOVE_EMBER) == 40);
        ASSUME(GetMovePower(MOVE_FIRE_BLAST) == 110);
        // What ZMoveSelectionDisplayPower prints in the move-selection window.
        ASSUME(GetZMoveBasePower(MOVE_EMBER, MOVE_INFERNO_OVERDRIVE) == 100);
        ASSUME(GetZMoveBasePower(MOVE_FIRE_BLAST, MOVE_INFERNO_OVERDRIVE) == 185);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_FIRIUM_Z); Moves(MOVE_EMBER, MOVE_FIRE_BLAST); }
        OPPONENT(SPECIES_CHANSEY) { MaxHP(700); HP(700); }
    } WHEN {
        TURN { MOVE(player, move, gimmick: GIMMICK_Z_MOVE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_INFERNO_OVERDRIVE, player);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        // 185/100 = 1.85x. Before the fix these were identical.
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.85), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("Z-MOVE FORK: a base move's zMove.powerOverride is honoured", s16 damage)
{
    u32 move;
    // Both physical, both Fire, neither gets STAB off Wobbuffet, so only the Z-Move's
    // base power differs between the two runs.
    PARAMETRIZE { move = MOVE_FIRE_PUNCH; } //  75 BP -> Inferno Overdrive 140 (tier table)
    PARAMETRIZE { move = MOVE_V_CREATE; }   // 180 BP, but overridden to 220
    GIVEN {
        ASSUME(GetMovePower(MOVE_FIRE_PUNCH) == 75);
        ASSUME(GetMoveZPowerOverride(MOVE_V_CREATE) == 220);
        ASSUME(GetMoveType(MOVE_V_CREATE) == TYPE_FIRE);
        ASSUME(GetZMoveBasePower(MOVE_FIRE_PUNCH, MOVE_INFERNO_OVERDRIVE) == 140);
        ASSUME(GetZMoveBasePower(MOVE_V_CREATE, MOVE_INFERNO_OVERDRIVE) == 220);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_FIRIUM_Z); Moves(MOVE_FIRE_PUNCH, MOVE_V_CREATE); }
        OPPONENT(SPECIES_CHANSEY) { MaxHP(700); HP(700); Defense(250); }
    } WHEN {
        TURN { MOVE(player, move, gimmick: GIMMICK_Z_MOVE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        // 220/140, i.e. the override wins over the tier table's 200 for a 180 BP move.
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(220.0 / 140.0), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("Z-MOVE FORK: a signature Z-Move uses its own power", s16 damage)
{
    u32 item, move;
    // Both Psychic and both special, so both get Mew's STAB and only the Z-Move's base
    // power differs. Confusion takes the tier table; Psychic with Mew's own crystal
    // becomes the signature Genesis Supernova, which carries its own power.
    PARAMETRIZE { item = ITEM_PSYCHIUM_Z; move = MOVE_CONFUSION; } // 50 BP -> Shattered Psyche 100
    PARAMETRIZE { item = ITEM_MEWNIUM_Z;  move = MOVE_PSYCHIC; }   //         Genesis Supernova 185
    GIVEN {
        // Before the fix Genesis Supernova dealt 200 BP, because GetZMovePower was handed
        // the Z-Move itself and re-ran its own 185 through the tier table.
        ASSUME(GetMovePower(MOVE_CONFUSION) == 50);
        ASSUME(GetMovePower(MOVE_GENESIS_SUPERNOVA) == 185);
        ASSUME(GetZMoveBasePower(MOVE_CONFUSION, MOVE_SHATTERED_PSYCHE) == 100);
        ASSUME(GetZMoveBasePower(MOVE_PSYCHIC, MOVE_GENESIS_SUPERNOVA) == 185);
        PLAYER(SPECIES_MEW) { Item(item); Moves(MOVE_CONFUSION, MOVE_PSYCHIC); }
        OPPONENT(SPECIES_CHANSEY) { MaxHP(700); HP(700); }
    } WHEN {
        TURN { MOVE(player, move, gimmick: GIMMICK_Z_MOVE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.85), results[1].damage);
    }
}

AI_SINGLE_BATTLE_TEST("Z-MOVE FORK: the AI ranks its moves by real Z-Move power")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        ASSUME(GetMovePower(MOVE_EMBER) == 40);
        ASSUME(GetMovePower(MOVE_FIRE_BLAST) == 110);
        PLAYER(SPECIES_CHANSEY) { MaxHP(700); HP(700); }
        // Both moves are the same type and both become Inferno Overdrive, so only base
        // power separates them. While every move simulated at a flat 100 BP the two were
        // indistinguishable and the pick came down to a random score tie.
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_FIRIUM_Z); Moves(MOVE_EMBER, MOVE_FIRE_BLAST); }
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_FIRE_BLAST, gimmick: GIMMICK_Z_MOVE); }
    }
}
