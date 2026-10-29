#ifndef GUARD_FORK_FRONTIER_BATTLE_BG_H
#define GUARD_FORK_FRONTIER_BATTLE_BG_H

// Themed battle backgrounds for Frontier trainer battles (B_FRONTIER_THEMED_BACKGROUNDS):
// the backdrop follows the opponent's trainer class (Bug Catcher -> long grass, Hiker ->
// mountain, ...), Frontier Brains get an Elite Four / Champion arena and the Tower's
// gym-leader bosses the Leader arena. Visual only: gBattleEnvironment, which drives
// Nature Power, Secret Power and Camouflage, is left alone.

// The background for the current Frontier battle. Anything without a theme (no trainer,
// an apprentice or record-mixed friend) keeps the stock Frontier backdrop.
u32 GetFrontierBattleEnvironment(void);

// The table lookup behind it, exposed for tests.
u32 GetFrontierEnvironmentForFacilityClass(u32 facilityClass);
u32 GetFrontierBrainEnvironment(u32 facility);

#endif // GUARD_FORK_FRONTIER_BATTLE_BG_H
