BattleFrontier_BattleFactoryPreBattleRoom_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, BattleFrontier_BattleFactoryPreBattleRoom_OnFrame
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, BattleFrontier_BattleFactoryPreBattleRoom_OnWarp
	.byte 0

BattleFrontier_BattleFactoryPreBattleRoom_OnWarp:
	map_script_2 VAR_TEMP_1, 0, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_SetUpObjects
	.2byte 0

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_SetUpObjects::
	setvar VAR_TEMP_1, 1
	goto_if_ne VAR_0x8006, 1, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_TurnPlayerNorth
	setobjectxy LOCALID_FACTORY_PRE_BATTLE_ATTENDANT, 8, 7
	turnobject LOCALID_FACTORY_PRE_BATTLE_ATTENDANT, DIR_SOUTH
BattleFrontier_BattleFactoryPreBattleRoom_EventScript_TurnPlayerNorth::
	turnobject LOCALID_PLAYER, DIR_NORTH
	end

BattleFrontier_BattleFactoryPreBattleRoom_OnFrame:
	map_script_2 VAR_TEMP_0, 0, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_EnterRoom
	.2byte 0

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_EnterRoom::
	goto_if_eq VAR_0x8006, 1, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_ReturnToRoomFromBattle
	setvar VAR_TEMP_0, 1
	applymovement LOCALID_FACTORY_PRE_BATTLE_ATTENDANT, BattleFrontier_BattleFactoryPreBattleRoom_Movement_AttendantEnterRoom
	applymovement LOCALID_PLAYER, BattleFrontier_BattleFactoryPreBattleRoom_Movement_PlayerEnterRoom
	waitmovement 0
	goto_if_eq VAR_0x8006, 2, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_ResumeChallenge
	factory_generaterentalmons
	factory_generateopponentmons
	factory_getopponentmontype
	setorcopyvar VAR_0x8005, VAR_RESULT
	factory_getopponentstyle
	setorcopyvar VAR_0x8006, VAR_RESULT
	call BattleFrontier_BattleFactoryPreBattleRoom_EventScript_CommentOnOpponentType
	call BattleFrontier_BattleFactoryPreBattleRoom_EventScript_CommentOnOpponentStyle
	@ FORK: B_FRONTIER_PARTY_SIZE_6V6 auto-rents a full team of 6, so there is no
	@ selection to choose from. Uses an assembler .if (not a cpp #if) so the
	@ config-value flag is read correctly; see the note in BattleFactoryLobby.
.if B_FRONTIER_PARTY_SIZE_6V6
	@ Build the team synchronously via callnative — no rental screen, so no
	@ fade-to-black/fade-in transition flash before "right this way".
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_RentedFullTeam, MSGBOX_DEFAULT
	factory_setswapped
	callnative AutoRentFullParty
.else
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_HoldMonsChooseFromSelection, MSGBOX_DEFAULT
	fadescreen FADE_TO_BLACK
	factory_setswapped
	factory_rentmons
	waitstate
.endif
BattleFrontier_BattleFactoryPreBattleRoom_EventScript_EnterBattleRoom::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_RightThisWay, MSGBOX_DEFAULT
	closemessage
	call BattleFrontier_EventScript_GetLvlMode
	call_if_eq VAR_RESULT, FRONTIER_LVL_50, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_WalkToBattleRoomLv50
	call_if_eq VAR_RESULT, FRONTIER_LVL_OPEN, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_WalkToBattleRoomLvOpen
	waitmovement 0
	warp MAP_BATTLE_FRONTIER_BATTLE_FACTORY_BATTLE_ROOM, 6, 11
	waitstate
	end

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_ReturnToRoomFromBattle::
	factory_setopponentmons
	factory_resethelditems
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_LetUsRestoreMons, MSGBOX_DEFAULT
	playfanfare MUS_HEAL
	waitfanfare
	special HealPlayerParty
	@ FORK: endless Factory — the attendant awards BP for the match you just won
	@ here (after restoring your team) before the challenge continues. The win
	@ streak was already incremented in the battle room, and opponentA still
	@ identifies the Factory Head for the brain BP case. See GiveBattlePoints.
	frontier_givepoints
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_AwardedBattlePoints, MSGBOX_GETPOINTS
	frontier_getbrainstatus
	goto_if_eq VAR_RESULT, FRONTIER_BRAIN_NOT_READY, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskReadyForRegularOpponent
	playse SE_POKENAV_CALL
	waitse
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_WaitFewMoments, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_FACTORY_PRE_BATTLE_ATTENDANT, BattleFrontier_BattleFactoryPreBattleRoom_Movement_AttendantMoveToReceiveCall
	waitmovement 0
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_UnderstoodSirWillDo, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_FACTORY_PRE_BATTLE_ATTENDANT, BattleFrontier_BattleFactoryPreBattleRoom_Movement_AttendantReturnToPlayer
	waitmovement 0
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_MessageFromHeadComeRightNow, MSGBOX_DEFAULT
	closemessage
	delay 16
	goto BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskReadyForHead
	end

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskReadyForOpponent::
	frontier_getbrainstatus
	goto_if_ne VAR_RESULT, FRONTIER_BRAIN_NOT_READY, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskReadyForHead
BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskReadyForRegularOpponent::
	@ FORK: endless Factory — the challenge runs indefinitely, so announce the
	@ upcoming match number from the win streak instead of the 7 hard-coded
	@ ordinal strings (which only went up to the "7th match").
	factory_get FACTORY_DATA_WIN_STREAK
	addvar VAR_RESULT, 1
	buffernumberstring STR_VAR_1, VAR_RESULT
	message BattleFrontier_BattleFactoryPreBattleRoom_Text_ReadyForNextMatch
	waitmessage
	call BattleFrontier_EventScript_GetCantRecordBattle
	goto_if_eq VAR_RESULT, TRUE, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskReadyForOpponentNoRecord
	multichoice 19, 4, MULTI_GO_ON_RECORD_REST_RETIRE, TRUE
	switch VAR_RESULT
	case 0, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskSwapMon
	case 1, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskRecordBattle
	case 2, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskPauseChallenge
	case 3, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskRetireChallenge
	case MULTI_B_PRESSED, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskReadyForOpponent

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskReadyForOpponentNoRecord::
	multichoice 20, 6, MULTI_GO_ON_REST_RETIRE, TRUE
	switch VAR_RESULT
	case 0, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskSwapMon
	case 1, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskPauseChallenge
	case 2, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskRetireChallenge
	case MULTI_B_PRESSED, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskReadyForOpponent

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskRecordBattle::
	message BattleFrontier_BattleFactoryPreBattleRoom_Text_RecordLatestBattle
	waitmessage
	multichoicedefault 20, 8, MULTI_YESNO, 1, FALSE
	switch VAR_RESULT
	case 1, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskReadyForOpponent
	case 0, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_RecordBattle
	case MULTI_B_PRESSED, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskReadyForOpponent

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_RecordBattle::
	call BattleFrontier_EventScript_SaveBattle
	goto BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskReadyForOpponent

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskPauseChallenge::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_SaveAndQuitGame, MSGBOX_YESNO
	switch VAR_RESULT
	case NO, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskReadyForOpponent
	case YES, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_PauseChallenge
	case MULTI_B_PRESSED, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskReadyForOpponent

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskRetireChallenge::
	message BattleFrontier_BattleFactoryPreBattleRoom_Text_RetireFromChallenge
	waitmessage
	multichoicedefault 20, 8, MULTI_YESNO, 1, FALSE
	switch VAR_RESULT
	case 1, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskReadyForOpponent
	case 0, BattleFrontier_BattleFactoryBattleRoom_EventScript_WarpToLobbyLost
	case MULTI_B_PRESSED, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskReadyForOpponent

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskSwapMon::
	factory_generateopponentmons
	factory_getopponentmontype
	setorcopyvar VAR_0x8005, VAR_RESULT
	factory_getopponentstyle
	setorcopyvar VAR_0x8006, VAR_RESULT
	call BattleFrontier_BattleFactoryPreBattleRoom_EventScript_CommentOnOpponentType
	call BattleFrontier_BattleFactoryPreBattleRoom_EventScript_CommentOnOpponentStyle
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_LikeToSwapMon, MSGBOX_YESNO
	switch VAR_RESULT
	case NO, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_EnterBattleRoom
	case YES, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_SwapMons
	case MULTI_B_PRESSED, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_EnterBattleRoom

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_SwapMons::
	fadescreen FADE_TO_BLACK
	factory_swapmons
	waitstate
	goto_if_eq VAR_RESULT, TRUE, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_EnterBattleRoom  @ Did player keep current pokemon
	factory_setswapped
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_YourSwapIsComplete, MSGBOX_DEFAULT
	goto BattleFrontier_BattleFactoryPreBattleRoom_EventScript_EnterBattleRoom

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_PauseChallenge::
	@ FORK: endless Factory — "Rest" no longer saves-and-reboots. It saves the
	@ challenge, restores the player's real party, and walks them back out to the
	@ lobby so they're free to leave and do other things. The win streak stays
	@ active (winStreakActiveFlags), which is how the lobby attendant later offers
	@ to resume — we do NOT leave challengeStatus == CHALLENGE_STATUS_PAUSED,
	@ because every other facility's lobby would then try to auto-resume this
	@ paused challenge as its own. factory_save -> SaveGameFrontier persists the
	@ real party safely; we then LoadPlayerParty back into memory so the player
	@ isn't roaming with rental mons.
	message BattleFrontier_BattleFactoryPreBattleRoom_Text_SavingDataPleaseWait
	waitmessage
	factory_save 0
	playse SE_SAVE
	waitse
	special LoadPlayerParty
	callnative UpdateFollowingPokemon
	goto BattleFrontier_BattleFactoryPreBattleRoom_EventScript_RestLeaveToLobby

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_RestLeaveToLobby::
	copyvar VAR_RESULT, VAR_FRONTIER_BATTLE_MODE
	goto_if_eq VAR_RESULT, FRONTIER_MODE_DOUBLES, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_RestLeaveToLobbyDoubles
	warp MAP_BATTLE_FRONTIER_BATTLE_FACTORY_LOBBY, 4, 8
	waitstate
	end

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_RestLeaveToLobbyDoubles::
	warp MAP_BATTLE_FRONTIER_BATTLE_FACTORY_LOBBY, 14, 8
	waitstate
	end

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_ResumeChallenge::
	special SavePlayerParty
	factory_setparties 0
	frontier_set FRONTIER_DATA_RECORD_DISABLED, TRUE
	special CalculatePlayerPartyCount
	goto BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskReadyForOpponent

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_CommentOnOpponentType::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_InvestigatedUpcomingOpponent, MSGBOX_DEFAULT
	call_if_eq VAR_0x8005, TYPE_NORMAL, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesNormal
	call_if_eq VAR_0x8005, TYPE_FIGHTING, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesFighting
	call_if_eq VAR_0x8005, TYPE_FLYING, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesFlying
	call_if_eq VAR_0x8005, TYPE_POISON, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesPoison
	call_if_eq VAR_0x8005, TYPE_GROUND, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesGround
	call_if_eq VAR_0x8005, TYPE_ROCK, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesRock
	call_if_eq VAR_0x8005, TYPE_BUG, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesBug
	call_if_eq VAR_0x8005, TYPE_GHOST, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesGhost
	call_if_eq VAR_0x8005, TYPE_STEEL, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesSteel
	call_if_eq VAR_0x8005, TYPE_FIRE, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesFire
	call_if_eq VAR_0x8005, TYPE_WATER, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesWater
	call_if_eq VAR_0x8005, TYPE_GRASS, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesGrass
	call_if_eq VAR_0x8005, TYPE_ELECTRIC, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesElectric
	call_if_eq VAR_0x8005, TYPE_PSYCHIC, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesPsychic
	call_if_eq VAR_0x8005, TYPE_ICE, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesIce
	call_if_eq VAR_0x8005, TYPE_DRAGON, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesDragon
	call_if_eq VAR_0x8005, TYPE_DARK, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesDark
	call_if_eq VAR_0x8005, TYPE_FAIRY, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesFairy
	call_if_eq VAR_0x8005, NUMBER_OF_MON_TYPES, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentHasNoMostCommonType
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesNormal::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInNormalType, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesFighting::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInFightingType, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesFlying::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInFlyingType, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesPoison::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInPoisonType, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesGround::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInGroundType, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesRock::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInRockType, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesBug::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInBugType, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesGhost::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInGhostType, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesSteel::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInSteelType, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesFire::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInFireType, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesWater::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInWaterType, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesGrass::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInGrassType, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesElectric::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInElectricType, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesPsychic::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInPsychicType, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesIce::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInIceType, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesDragon::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInDragonType, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesDark::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInDarkType, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentUsesFairy::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInFairyType, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_OpponentHasNoMostCommonType::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerHasNoClearFavorite, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_CommentOnOpponentStyle::
	call_if_eq VAR_0x8006, FACTORY_STYLE_NONE, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_StyleUnrestrained
	call_if_eq VAR_0x8006, FACTORY_STYLE_PREPARATION, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_StyleTotalPreparation
	call_if_eq VAR_0x8006, FACTORY_STYLE_SLOW_STEADY, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_StyleSlowAndSteady
	call_if_eq VAR_0x8006, FACTORY_STYLE_ENDURANCE, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_StyleEndurance
	call_if_eq VAR_0x8006, FACTORY_STYLE_HIGH_RISK, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_StyleHighRisk
	call_if_eq VAR_0x8006, FACTORY_STYLE_WEAKENING, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_StyleWeakenFoe
	call_if_eq VAR_0x8006, FACTORY_STYLE_UNPREDICTABLE, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_StyleImpossibleToPredict
	call_if_eq VAR_0x8006, FACTORY_STYLE_WEATHER, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_StyleDependsOnFlow
	call_if_eq VAR_0x8006, FACTORY_NUM_STYLES, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_StyleFlexible
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_StyleUnrestrained::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_StyleUnrestrained, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_StyleTotalPreparation::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_StyleTotalPreparation, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_StyleSlowAndSteady::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_StyleSlowAndSteady, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_StyleEndurance::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_StyleEndurance, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_StyleHighRisk::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_StyleHighRisk, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_StyleWeakenFoe::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_StyleWeakenFoe, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_StyleImpossibleToPredict::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_StyleImpossibleToPredict, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_StyleDependsOnFlow::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_StyleDependsOnFlow, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_StyleFlexible::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_StyleFlexible, MSGBOX_DEFAULT
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskReadyForHead::
	message BattleFrontier_BattleFactoryPreBattleRoom_Text_PreparedToFaceHead
	waitmessage
	call BattleFrontier_EventScript_GetCantRecordBattle
	goto_if_eq VAR_RESULT, TRUE, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskReadyForHeadNoRecord
	multichoice 19, 4, MULTI_GO_ON_RECORD_REST_RETIRE, TRUE
	switch VAR_RESULT
	case 0, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskSwapBeforeHead
	case 1, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskRecordBattle
	case 2, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskPauseChallenge
	case 3, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskRetireChallenge
	case MULTI_B_PRESSED, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskReadyForHead

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskReadyForHeadNoRecord::
	multichoice 20, 6, MULTI_GO_ON_REST_RETIRE, TRUE
	switch VAR_RESULT
	case 0, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskSwapBeforeHead
	case 1, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskPauseChallenge
	case 2, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskRetireChallenge
	case MULTI_B_PRESSED, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskReadyForHead

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_AskSwapBeforeHead::
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_CantTellAnythingAboutHead, MSGBOX_DEFAULT
	msgbox BattleFrontier_BattleFactoryPreBattleRoom_Text_LikeToSwapMon, MSGBOX_YESNO
	switch VAR_RESULT
	case NO, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_EnterBattleRoom
	case YES, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_SwapMons
	case MULTI_B_PRESSED, BattleFrontier_BattleFactoryPreBattleRoom_EventScript_EnterBattleRoom

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_WalkToBattleRoomLv50::
	applymovement LOCALID_FACTORY_PRE_BATTLE_ATTENDANT, BattleFrontier_BattleFactoryPreBattleRoom_Movement_GuideWalkToBattleRoomLv50
	applymovement LOCALID_PLAYER, BattleFrontier_BattleFactoryPreBattleRoom_Movement_PlayerWalkToBattleRoomLv50
	return

BattleFrontier_BattleFactoryPreBattleRoom_EventScript_WalkToBattleRoomLvOpen::
	applymovement LOCALID_FACTORY_PRE_BATTLE_ATTENDANT, BattleFrontier_BattleFactoryPreBattleRoom_Movement_GuideWalkToBattleRoomLvOpen
	applymovement LOCALID_PLAYER, BattleFrontier_BattleFactoryPreBattleRoom_Movement_PlayerWalkToBattleRoomLvOpen
	return

BattleFrontier_BattleFactoryPreBattleRoom_Movement_PlayerEnterRoom:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

BattleFrontier_BattleFactoryPreBattleRoom_Movement_PlayerWalkToBattleRoomLv50:
	walk_up
	walk_left
	walk_left
	walk_up
	walk_up
	step_end

BattleFrontier_BattleFactoryPreBattleRoom_Movement_PlayerWalkToBattleRoomLvOpen:
	walk_up
	walk_right
	walk_right
	walk_up
	walk_up
	step_end

BattleFrontier_BattleFactoryPreBattleRoom_Movement_AttendantEnterRoom:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	face_down
	step_end

BattleFrontier_BattleFactoryPreBattleRoom_Movement_GuideWalkToBattleRoomLv50:
	walk_left
	walk_left
	walk_up
	walk_up
	set_invisible
	step_end

BattleFrontier_BattleFactoryPreBattleRoom_Movement_GuideWalkToBattleRoomLvOpen:
	walk_right
	walk_right
	walk_up
	walk_up
	set_invisible
	step_end

BattleFrontier_BattleFactoryPreBattleRoom_Movement_AttendantMoveToReceiveCall:
	walk_left
	walk_left
	walk_left
	step_end

BattleFrontier_BattleFactoryPreBattleRoom_Movement_AttendantReturnToPlayer:
	walk_right
	walk_right
	walk_right
	face_down
	step_end

BattleFrontier_BattleFactoryPreBattleRoom_Text_HoldMonsChooseFromSelection:
	.string "First, we will hold your POKéMON for\n"
	.string "safekeeping.\p"
	.string "You may then choose from our\n"
	.string "selection of POKéMON.$"

@ FORK: shown instead of the above when a full team of 6 is auto-rented.
BattleFrontier_BattleFactoryPreBattleRoom_Text_RentedFullTeam:
	.string "We've prepared a full rental team of\n"
	.string "6 POKéMON for you.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_LetUsRestoreMons:
	.string "Thank you for competing!\n"
	.string "Let us restore your POKéMON!$"

@ FORK: endless Factory awards BP every win, which is always 2 or more, so this
@ drops the "(s)" the shared BattleFrontier_Text_ObtainedXBattlePoints carries.
BattleFrontier_BattleFactoryPreBattleRoom_Text_AwardedBattlePoints:
	.string "{PLAYER} obtained {STR_VAR_1} Battle\n"
	.string "Points.$"

@ FORK: endless Factory — single generalized prompt for any match number
@ (replaces the Factory's hard-coded "2nd".."7th match" strings). STR_VAR_1 holds
@ the upcoming match number, buffered from the win streak.
BattleFrontier_BattleFactoryPreBattleRoom_Text_ReadyForNextMatch:
	.string "Match No. {STR_VAR_1} is next!\n"
	.string "Are you ready?$"

@ The 2nd/3rd ordinal strings are still used by the Slateport Battle Tent
@ (SlateportCity_BattleTentCorridor), which is fixed-length, so they remain.
BattleFrontier_BattleFactoryPreBattleRoom_Text_ReadyFor2ndOpponent:
	.string "The 2nd match is next!\n"
	.string "Are you ready?$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_ReadyFor3rdOpponent:
	.string "The 3rd match is next!\n"
	.string "Are you ready?$"

@ FORK: "Rest" now saves and steps out to the lobby (the streak is kept so it can
@ be resumed later) rather than saving and rebooting the game.
BattleFrontier_BattleFactoryPreBattleRoom_Text_SaveAndQuitGame:
	.string "Would you like to save and take\n"
	.string "a break from your challenge?$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_RetireFromChallenge:
	.string "Would you like to retire from your\n"
	.string "Battle Swap challenge?$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_InvestigatedUpcomingOpponent:
	.string "I've conducted a little investigation\n"
	.string "about your upcoming opponent.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInNormalType:
	.string "The TRAINER is apparently skilled\n"
	.string "in the handling of the NORMAL type.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInFireType:
	.string "The TRAINER is apparently skilled\n"
	.string "in the handling of the FIRE type.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInWaterType:
	.string "The TRAINER is apparently skilled\n"
	.string "in the handling of the WATER type.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInElectricType:
	.string "The TRAINER is apparently skilled\n"
	.string "in the handling of the ELECTRIC type.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInGrassType:
	.string "The TRAINER is apparently skilled\n"
	.string "in the handling of the GRASS type.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInIceType:
	.string "The TRAINER is apparently skilled\n"
	.string "in the handling of the ICE type.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInFightingType:
	.string "The TRAINER is apparently skilled\n"
	.string "in the handling of the FIGHTING type.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInPoisonType:
	.string "The TRAINER is apparently skilled\n"
	.string "in the handling of the POISON type.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInGroundType:
	.string "The TRAINER is apparently skilled\n"
	.string "in the handling of the GROUND type.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInFlyingType:
	.string "The TRAINER is apparently skilled\n"
	.string "in the handling of the FLYING type.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInPsychicType:
	.string "The TRAINER is apparently skilled\n"
	.string "in the handling of the PSYCHIC type.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInBugType:
	.string "The TRAINER is apparently skilled\n"
	.string "in the handling of the BUG type.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInRockType:
	.string "The TRAINER is apparently skilled\n"
	.string "in the handling of the ROCK type.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInGhostType:
	.string "The TRAINER is apparently skilled\n"
	.string "in the handling of the GHOST type.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInDragonType:
	.string "The TRAINER is apparently skilled\n"
	.string "in the handling of the DRAGON type.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInDarkType:
	.string "The TRAINER is apparently skilled\n"
	.string "in the handling of the DARK type.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInFairyType:
	.string "The TRAINER is apparently skilled\n"
	.string "in the handling of the FAIRY type.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerSkilledInSteelType:
	.string "The TRAINER is apparently skilled\n"
	.string "in the handling of the STEEL type.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_TrainerHasNoClearFavorite:
	.string "The TRAINER appears to have no clear\n"
	.string "favorites when it comes to type.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_StyleSlowAndSteady:
	.string "The favorite battle style appears to\n"
	.string "be slow and steady.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_StyleEndurance:
	.string "The favorite battle style appears to\n"
	.string "be one of endurance.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_StyleHighRisk:
	.string "The favorite battle style appears to\n"
	.string "be high risk, high return.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_StyleDependsOnFlow:
	.string "The favorite battle style appears to\n"
	.string "depend on the battle's flow.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_StyleTotalPreparation:
	.string "The favorite battle style appears to\n"
	.string "be one based on total preparation.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_StyleWeakenFoe:
	.string "The favorite battle style appears\n"
	.string "to be weakening the foe to start.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_StyleFlexible:
	.string "The favorite battle style appears to\n"
	.string "be flexibly adaptable to the situation.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_StyleImpossibleToPredict:
	.string "The favorite battle style appears to\n"
	.string "be impossible to predict.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_StyleUnrestrained:
	.string "The favorite battle style appears to\n"
	.string "be free-spirited and unrestrained.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_LikeToSwapMon:
	.string "Before starting the battle, would you\n"
	.string "like to swap a POKéMON?$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_YourSwapIsComplete:
	.string "Thank you!\n"
	.string "Your POKéMON swap is complete.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_RightThisWay:
	.string "Right this way, please!$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_SavingDataPleaseWait:
	.string "I am saving your data.\n"
	.string "Please wait.$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_RecordLatestBattle:
	.string "Would you like to record your latest\n"
	.string "battle on your FRONTIER PASS?$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_WaitFewMoments:
	.string "Excuse me! Excuse me, please!\n"
	.string "May I get you to wait a few moments?$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_UnderstoodSirWillDo:
	.string "…Uh-huh? What?! …Whoa!\n"
	.string "Understood, sir! Will do!$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_MessageFromHeadComeRightNow:
	.string "Oh, my…\n"
	.string "Sorry to keep you waiting!\p"
	.string "I have a message from this facility's\n"
	.string "boss, the FACTORY HEAD.\p"
	.string "He says, “We're going to do it!\n"
	.string "Come here right now!”$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_PreparedToFaceHead:
	.string "The FACTORY HEAD is demanding you.\n"
	.string "Are you prepared to face him?$"

BattleFrontier_BattleFactoryPreBattleRoom_Text_CantTellAnythingAboutHead:
	.string "I'm terribly sorry, but I can't tell you\n"
	.string "anything about the FACTORY HEAD.$"
