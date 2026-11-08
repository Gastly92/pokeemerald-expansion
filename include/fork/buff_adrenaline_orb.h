#ifndef GUARD_FORK_BUFF_ADRENALINE_ORB_H
#define GUARD_FORK_BUFF_ADRENALINE_ORB_H

#include "battle.h"
#include "config_changes.h"

// BUFF_ADRENALINE_ORB (config/buff.h). The orb fires on any stat drop an opponent lands on the
// holder, not only on Intimidate. Two halves:
//
// - MarkAdrenalineOrbDrop() is called from DecreaseStat() (src/battle_stat_change.c) at the
//   point where a drop is committed. Every blocker (Clear Body, Clear Amulet, Mist, Flower Veil,
//   Mirror Armor, a stat already at -6) returns before that point, and Contrary turns the change
//   into a raise that never goes through DecreaseStat(), so a blocked drop never sets the mark.
// - ConsumeAdrenalineOrbDrop() is called from BS_TryAdrenalineOrb(), which runs after every stat
//   change message. It always clears the battler's mark, so a mark can't outlive the drop that
//   set it (for example on a battler not holding the orb that is handed one later by Trick).
//
// The orb's own Speed raise goes through trybattlerstatchange, which makes the holder both
// attacker and target, so a Contrary holder's reversed boost doesn't count as an opponent's
// drop and can't re-trigger the orb.

// Whether a drop the battler `atk` landed on `def` counts as caused by an opponent. A Sticky Web
// counts when the opposing side set it (Court Change can move a web to the side that set it).
static inline bool32 IsAdrenalineOrbDropFromFoe(enum BattlerId atk, enum BattlerId def, bool32 stickyWeb)
{
    if (stickyWeb)
        return gSideTimers[GetBattlerSide(def)].stickyWebBattlerSide != GetBattlerSide(def);
    return atk != def && !IsBattlerAlly(atk, def);
}

static inline void MarkAdrenalineOrbDrop(enum BattlerId atk, enum BattlerId def, bool32 stickyWeb)
{
    if (GetConfig(BUFF_ADRENALINE_ORB) && IsAdrenalineOrbDropFromFoe(atk, def, stickyWeb))
        gBattleStruct->adrenalineOrbDropLanded |= 1u << def;
}

// Returns whether an opponent's drop just landed on `battler`, and clears the mark.
static inline bool32 ConsumeAdrenalineOrbDrop(enum BattlerId battler)
{
    bool32 landed = (gBattleStruct->adrenalineOrbDropLanded >> battler) & 1;
    gBattleStruct->adrenalineOrbDropLanded &= ~(1u << battler);
    return landed;
}

// Speed stages the orb raises. Stock: +1. Buffed: +BUFF_ADRENALINE_ORB_STAGES.
static inline u32 GetAdrenalineOrbStages(void)
{
    if (GetConfig(BUFF_ADRENALINE_ORB))
        return BUFF_ADRENALINE_ORB_STAGES;
    return 1;
}

#endif // GUARD_FORK_BUFF_ADRENALINE_ORB_H
