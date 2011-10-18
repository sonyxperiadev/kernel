/*******************************************************************************
* Copyright (C) 2007-2009 ARM Limited
*
* This software is provided 'as-is', without any express or implied
* warranties including the implied warranties of satisfactory quality,
* fitness for purpose or non infringement.  In no event will  ARM be
* liable for any damages arising from the use of this software.
*
* Permission is granted to anyone to use, copy and modify this software for
* any purpose, and to redistribute the software, subject to the following
* restrictions:
*
* 1. The origin of this software must not be misrepresented* you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
** SVN Information
** Checked In          : $Date: 2009-05-07 14:20:15 +0100 (Thu, 07 May 2009) $
** Revision            : $Revision: 8622 $
** Release Information :
**
********************************************************************************
* Timer support, Cortex-A9 SCU block
*
* Macro List:
*             CA9_TIMERS_SAVE
*             CA9_TIMERS_RESTORE
*
********************************************************************************/


/**********************************************************
*
* IMPLEMENTATION DEFINED - Cortex-A9 timer support MACRO
* WARNING: May be of limited value to returning code
*
**********************************************************/
	@ Save 32- and 64-bit timer context

	.macro CA9_TIMERS_SAVE

		STR	r0,[r11, #DM_TIMERS] @ save ptr at fixed address

		LDR	r1, [r11, #DM_CA9_SCU_VA]
		MOVW	r6, #CA9_SCU_TIMER
		ADD	r1,r1,r6
		LDR	r2, [r1, #TIMER_Ld]   @ timer load
		LDR	r3, [r1, #TIMER_Ctl]  @ timer control
		LDR	r4, [r1, #WDOG_Ld]    @ watchdog load
		LDR	r5, [r1, #WDOG_Ctl]   @ watchdog control
		STM	r0!, {r2-r5}

	.if (CA9_TIM64_CONTEXT_SAVE)
		LDR	r1, [r11, #DM_CA9_SCU_VA]
		MOVW	r6, #CA9_SCU_TIM64
		ADD	r1,r1,r6
		LDR	r2, [r1, #TIM64_Ctl]  @ 64-bit timer control
		BIC	r3, r2, #0xF
		STR	r3, [r1, #TIM64_Ctl]  @ disable the features
		@ the registers are now frozen for the context save
		LDR	r3, [r1, #TIM64_AutoInc] @ Autoincrement register
		LDR	r4, [r1, #TIM64_CmpLo]   @ comparator - lo word
		LDR	r5, [r1, #TIM64_CmpHi]   @ comparator - hi word
		STM	r0!, {r2-r5}
		LDR	r2, [r1, #TIM64_CntLo] @ counter - lo word
		LDR	r3, [r1, #TIM64_CntHi] @ counter - hi word
		STM	r0!, {r2-r3}
	.endif

	.endm

/*******************************************************************************
* DORMANT_CA9_TIMERS_RESTORE macro
*
* IMPLEMENTATION DEFINED - Cortex-A9 timer support
* WARNING: May be of limited value to returning code
*
*********************************************************************************/
	.macro CA9_TIMERS_RESTORE

		MRC	p15,0,r7,c0,c0,5     	@ read the MPIDR, MP affinity register
		TST	r7, r7
		BPL	1000f
		UBFX	r7, r7, #0, #2      @ extract the CPUID field
									@ IMP DEF width: Cortex-A9

		LDR	r0,[r11,#DM_TIMERS]  	@ load the saved context pointer
		CMP	r0,#0
		BEQ	1000f   @ no timer context available to restore

		LDR	r1, [r11, #DM_CA9_SCU_VA]
		MOVW	r6, #CA9_SCU_TIMER
		ADD	r1,r1,r6
		LDM	r0!, {r2-r5}
		STR	r2, [r1, #TIMER_Ld]   @ timer load
		STR	r3, [r1, #TIMER_Ctl]  @ timer control

		AND	r2, r5, #1<<3
		CMP	r2, #0
		BNE 1001f
	@ if watchdog was in timer mode, we have to reconfigure it
		LDR r2, =0x12345678
		STR	r2, [r1, #WDOG_Disable]
		LDR r2, =0x87654321
		STR	r2, [r1, #WDOG_Disable]

1001:
		STR	r4, [r1, #WDOG_Ld]    @ watchdog load
		STR	r5, [r1, #WDOG_Ctl]   @ watchdog control

	.if (CA9_TIM64_CONTEXT_SAVE)
		LDR	r1, [r11, #DM_CA9_SCU_VA]
		MOVW	r6, #CA9_SCU_TIM64
		ADD	r1,r1,r6
		LDM	r0!, {r2-r5}
		STR	r3, [r1, #TIM64_AutoInc] 	@ Autoincrement register
		STR	r4, [r1, #TIM64_CmpLo]  	@ comparator - lo word
		STR	r5, [r1, #TIM64_CmpHi]  	@ comparator - hi word
		LDM	r0!, {r3-r4}
		STR	r3, [r1, #TIM64_CntLo]		@ counter - lo word
		STR	r4, [r1, #TIM64_CntHi] 		@ counter - hi word
		STR	r2, [r1, #TIM64_Ctl]   		@ restore the control last
	.endif

1000:
	.endm


