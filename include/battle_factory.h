#ifndef GUARD_BATTLE_FACTORY_H
#define GUARD_BATTLE_FACTORY_H

void CallBattleFactoryFunction(void);
bool8 InBattleFactory(void);
u8 GetFactoryMonFixedIV(u8 challengeNum, bool8 isLastBattle);
void FillFactoryBrainParty(void);
u8 GetNumPastRentalsRank(u8 battleMode, enum FrontierLevelMode lvlMode);
u64 GetAiScriptsInBattleFactory(void);
void SetMonMoveAvoidReturn(struct Pokemon *mon, enum Move moveArg, u8 moveSlot);
void FillFactoryTrainerParty(void);

// FORK: the roster the Battle Factory draws from. With B_FRONTIER_COMPETITIVE_MONS
// this is the fork's competitive list (gFactoryCompetitiveMons); otherwise it is
// the vanilla gBattleFrontierMons. Used in place of a bare gBattleFrontierMons on
// every Factory code path (battle_factory.c / battle_factory_screen.c).
const struct TrainerMon *GetFactoryMonsTable(void);
u16 GetFactoryMonsCount(void);

#endif // GUARD_BATTLE_FACTORY_H
