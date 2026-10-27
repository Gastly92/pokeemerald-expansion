#include "global.h"
#include "battle.h"
#include "battle_scripts.h"
#include "battle_script_commands.h"
#include "battle_util.h"
#include "fork/silent_ability_popup.h"

// Announcing silent damage abilities -- see include/fork/silent_ability_popup.h.
//
// The type matchup (Scrappy, in the target-failure canceler) and the damage calc reveal these
// through RecordAbilityBattle (src/battle_ai_record.c), which sets
// gBattleStruct->infoAbilityRevealed. Comparing those bits at the start of the move and just
// before its animation finds what this move revealed, with no hook in the calc itself. Only the
// abilities listed below are announced, so one revealed in that window by a script that already
// shows its pop-up (Volt Absorb, Levitate) is not shown twice. A reveal outside the window (an
// end-of-turn Heatproof burn) is never announced. A multi-hit move re-checks on every strike, and
// what an earlier strike announced is marked done.

static EWRAM_DATA u8 sRevealedBeforeMove[NUM_BATTLE_SIDES] = {0};

// Abilities whose only trace in stock play is a changed damage number. Each is recorded silently
// by the type matchup or damage calc in src/battle_util.c; abilities recorded there that get their
// own pop-up (Levitate, Sturdy, Telepathy, Tera Shell) are left out so they are not shown twice.
static bool32 IsSilentDamageAbility(enum Ability ability)
{
    switch (ability)
    {
    // The defender's damage modifiers.
    case ABILITY_MULTISCALE:
    case ABILITY_SHADOW_SHIELD:
    case ABILITY_FILTER:
    case ABILITY_SOLID_ROCK:
    case ABILITY_PRISM_ARMOR:
    case ABILITY_FLUFFY:
    case ABILITY_PUNK_ROCK:
    case ABILITY_ICE_SCALES:
    // The defender's Defense / the attacker's Attack, as the defender's ability reads them.
    case ABILITY_FUR_COAT:
    case ABILITY_MARVEL_SCALE:
    case ABILITY_GRASS_PELT:
    case ABILITY_THICK_FAT:
    case ABILITY_PURIFYING_SALT:
    // The move's power.
    case ABILITY_HEATPROOF:
    case ABILITY_WATER_BUBBLE:
    // A crit the move was certain to land.
    case ABILITY_BATTLE_ARMOR:
    case ABILITY_SHELL_ARMOR:
    // The attacker's: through screens, and Normal / Fighting into a Ghost type.
    case ABILITY_INFILTRATOR:
    case ABILITY_SCRAPPY:
    case ABILITY_MINDS_EYE:
        return TRUE;
    default:
        return FALSE;
    }
}

static u32 GetRevealBit(enum BattlerId battler)
{
    return 1u << gBattlerPartyIndexes[battler];
}

void SnapshotAbilityRevealsBeforeMove(void)
{
    for (u32 side = 0; side < NUM_BATTLE_SIDES; side++)
        sRevealedBeforeMove[side] = gBattleStruct->infoAbilityRevealed[side];
}

bool32 TryShowSilentAbilityPopUp(void)
{
    if (!GetConfig(FEATURE_ANNOUNCE_SILENT_ABILITIES))
        return FALSE;

    for (enum BattlerId battler = 0; battler < gBattlersCount; battler++)
    {
        u32 side = GetBattlerSide(battler);
        u32 bit = GetRevealBit(battler);
        enum Ability ability;

        if (!IsBattlerAlive(battler)
         || (sRevealedBeforeMove[side] & bit)
         || !(gBattleStruct->infoAbilityRevealed[side] & bit))
            continue;

        // Shown or not, this reveal is dealt with.
        sRevealedBeforeMove[side] |= bit;
        ability = gBattleStruct->infoRevealedAbility[side][gBattlerPartyIndexes[battler]];
        if (!IsSilentDamageAbility(ability))
            continue;

        // The pop-up names the chosen ability unless told otherwise; name the one that acted.
        if (gBattleMons[battler].ability != ability)
            gBattleScripting.abilityPopupOverwrite = ability;
        gBattleScripting.battler = battler;
        BattleScriptCall(BattleScript_SilentAbilityRevealed);
        return TRUE;
    }
    return FALSE;
}
