	@ FORK-owned battle script macros.
	@
	@ Kept out of asm/macros/battle_script.inc so an upstream edit near where they
	@ used to sit cannot drag them into a merge conflict (the 1.17.0 sync did exactly
	@ that). Included from data/battle_scripts_1.s and data/battle_scripts_2.s with a
	@ single additive line right after upstream's own include.
	@
	@ Add new fork macros here, never to the upstream file.

	@ FORK: DETERMINISTIC_HOLD_EFFECTS — jump to \ptr unless the hung-on item should
	@ be consumed (Focus Sash always; Focus Band only under the flag).
	.macro jumpifhangonitemnotconsumed ptr:req
	callnative BS_JumpIfHangOnItemNotConsumed
	.4byte \ptr
	.endm

	@ FORK: jump to \ptr when DETERMINISTIC_HOLD_EFFECTS is off (so flag-gated item
	@ consumption is skipped in stock play).
	.macro jumpifnotdeterministicholdeffects ptr:req
	callnative BS_JumpIfNotDeterministicHoldEffects
	.4byte \ptr
	.endm
