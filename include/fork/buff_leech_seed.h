#ifndef GUARD_FORK_BUFF_LEECH_SEED_H
#define GUARD_FORK_BUFF_LEECH_SEED_H

#include "battle.h"

// FORK: BUFF_LEECH_SEED -- the drain, and the "re-drain" rule that lets a seeded battler
// keep draining after the seeder switches. Definitions in src/fork/buff_leech_seed.c.
// FORK: BUFF_LEECH_SEED. Which branch a Leech Seed drain takes, returned by SetUpLeechSeedDrain.
enum LeechSeedDrainKind
{
    LEECH_SEED_DRAIN_RECOVERY,    // victim loses HP, seeder heals
    LEECH_SEED_DRAIN_LIQUID_OOZE, // victim loses HP, seeder takes recoil (victim has Liquid Ooze)
    LEECH_SEED_DRAIN_HEAL_BLOCK,  // victim loses HP, seeder heals nothing (seeder under Heal Block)
};
enum LeechSeedDrainKind SetUpLeechSeedDrain(enum BattlerId victim, enum BattlerId seeder);
// FORK: BUFF_LEECH_SEED. TRUE when using Leech Seed on `victim` is the immediate re-drain
// (`seeder` already seeds it) and that drain can actually land.
bool32 CanLeechSeedReDrain(enum BattlerId seeder, enum BattlerId victim);

// FORK: runs the end-turn drain script for the branch SetUpLeechSeedDrain picked. Upstream's
// 1.17.0 rewrite of those scripts changed which battler each BS_ slot means (BS_SCRIPTING is
// now the victim, BS_ATTACKER the receiver), so keep this in step with data/battle_scripts_1.s
// if they change again.
void CallLeechSeedTurnDrainScript(enum LeechSeedDrainKind kind);

#endif // GUARD_FORK_BUFF_LEECH_SEED_H
