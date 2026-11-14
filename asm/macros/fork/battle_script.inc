	@ FORK-owned battle script macros.
	@
	@ Kept out of asm/macros/battle_script.inc so an upstream edit near where they
	@ used to sit cannot drag them into a merge conflict (the 1.17.0 sync did exactly
	@ that). Included from data/battle_scripts_1.s and data/battle_scripts_2.s with a
	@ single additive line right after upstream's own include.
	@
	@ Add new fork macros here, never to the upstream file.

	@ DETERMINISTIC_HOLD_EFFECTS — jump to \ptr unless the hung-on item should
	@ be consumed (Focus Sash always; Focus Band only under the flag).
	.macro jumpifhangonitemnotconsumed ptr:req
	callnative BS_JumpIfHangOnItemNotConsumed
	.4byte \ptr
	.endm

	@ Jump to \ptr when DETERMINISTIC_HOLD_EFFECTS is off (so flag-gated item
	@ consumption is skipped in stock play).
	.macro jumpifnotdeterministicholdeffects ptr:req
	callnative BS_JumpIfNotDeterministicHoldEffects
	.4byte \ptr
	.endm

	@ BUFF_MICLE_BERRY — revive the scripting battler's most recently fainted ally.
	@ Jumps to \sendOutPtr (with the scripting battler now the revived slot) when the
	@ ally has to be sent back onto the field.
	.macro miclerevive sendOutPtr:req
	callnative BS_MicleBerryRevive
	.4byte \sendOutPtr
	.endm

	@ BUFF_MICLE_BERRY — restore the scripting battler to the holder after a send-out.
	.macro miclerestorebattler
	callnative BS_MicleBerryRestoreBattler
	.endm
