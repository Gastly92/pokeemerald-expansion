#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_util.h"
#include "constants/battle_string_ids.h"
#include "fork/deterministic_moody.h"

// Stage (relative to neutral) Moody sets every stat to, per phase. After the
// last entry the phase loops between the final two, alternating +6 and -6.
static const s8 sMoodyStages[MOODY_PHASE_COUNT] = {1, -1, 2, -2, 3, -3, MAX_STAT_STAGE - DEFAULT_STAT_STAGE, MIN_STAT_STAGE - DEFAULT_STAT_STAGE};

const u16 gMoodyOscillationStringIds[MOODY_PHASE_COUNT] =
{
    STRINGID_MOODYSTATSUP1,
    STRINGID_MOODYSTATSDOWN1,
    STRINGID_MOODYSTATSUP2,
    STRINGID_MOODYSTATSDOWN2,
    STRINGID_MOODYSTATSUP3,
    STRINGID_MOODYSTATSDOWN3,
    STRINGID_MOODYSTATSMAXED,
    STRINGID_MOODYSTATSMINIMIZED,
};

s32 GetDeterministicMoodyStage(u32 phase)
{
    return sMoodyStages[phase < MOODY_PHASE_COUNT ? phase : MOODY_PHASE_COUNT - 1];
}

void ApplyDeterministicMoody(enum BattlerId battler)
{
    u32 phase = gBattleMons[battler].volatiles.moodyPhase;
    s32 stage = GetDeterministicMoodyStage(phase);

    // A direct set, like Haze or Belly Drum: it is not a stat *change*, so
    // Contrary/Simple and stat-change reactions (Opportunist, Mirror Herb,
    // Defiant, White Herb) do not apply.
    for (enum Stat stat = STAT_ATK; stat < NUM_BATTLE_STATS; stat++)
        gBattleMons[battler].statStages[stat] = DEFAULT_STAT_STAGE + stage;

    gBattleMons[battler].volatiles.moodyPhase = (phase + 1 < MOODY_PHASE_COUNT) ? phase + 1 : MOODY_PHASE_COUNT - 2;

    if (stage > 0)
        gBattleScripting.animArg1 = (stage >= 2) ? STAT_ANIM_MULTIPLE_PLUS2 : STAT_ANIM_MULTIPLE_PLUS1;
    else
        gBattleScripting.animArg1 = (stage <= -2) ? STAT_ANIM_MULTIPLE_MINUS2 : STAT_ANIM_MULTIPLE_MINUS1;
    gBattleCommunication[MULTISTRING_CHOOSER] = phase;
    gEffectBattler = gBattlerAbility = battler;
    BattleScriptCall(BattleScript_DeterministicMoody);
}
