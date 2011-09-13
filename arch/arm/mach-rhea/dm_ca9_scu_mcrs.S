
/********************************************************************************
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
*********************************************************************************/
/* Cortex-A9 SCU support
*
* Macro List:
*             CA9_SCU_SAVE
*             CA9_SCU_RESTORE
*
********************************************************************************/


/**********************************************************
*
* DORMANT_CA9_SCU_SAVE macro
*
**********************************************************/
	.macro CA9_SCU_SAVE   @Save SCU context

	.if (DIRECT_MAPPED_SAVE == 0)
		MOV	r4, r0
dm_scu:
		VA_TO_PA r4,r5,r6
        @ WARNING - ASSUMES SCU RESTORED BEFORE PA => VA TRANSITION
		STR	r4,[r11, #DM_SCU]	@ save PA ptr at fixed address
	.else
		STR	r0,[r11, #DM_SCU]	@ save ptr at fixed address
	.endif

		LDR	r1, [r11, #DM_CA9_SCU_VA]

/**************************************************************
*
* IMPLEMENTATION DEFINED - save Cortex-A9 SCU registers
*
**************************************************************/

		LDR	r2, [r1, #SCU_FiltStart]  @ filter start address
		LDR	r3, [r1, #SCU_FiltEnd]    @ filter end address
		LDR	r4, [r1, #SCU_SAC]        @ SAC
		LDR	r5, [r1, #SCU_SSAC]       @ SSAC
		LDR	r6, [r1, #SCU_Ctl]        @ SCU Control
		STM	r0!, {r2-r6}
		@ endif (CPUID == 0)

	.endm

/*****************************************************************
* DORMANT_CA9_SCU_RESTORE macro
*
* IMPLEMENTATION DEFINED - Cortex-A9 power register
* No update on dormant restore required
*
* Assumption: the context is restored BEFORE the MMU is enabled
*****************************************************************/

	.macro	CA9_SCU_RESTORE

@		MRC	p15,0,r7,c0,c0,5	@ read the MPIDR, MP affinity register
@		TST	r7, r7				@ check for MP Extn format
@		BPL	1000f
@		UBFX	r7, r7, #0, #2	@ extract the CPUID field
@                                @ IMP DEF width: Cortex-A9
@		CMP	r7, #0				@ if CPUID == 0, then
@		BNE 1000f

/**************************************************************
*
* IMPLEMENTATION DEFINED - restore Cortex-A9 SCU registers
*
**************************************************************/
		LDR	r0, [r11, #DM_SCU]	@ recover SCU context base address
		CMP	r0,#0
		BEQ	1000f		@ no SCU context available to restore
		LDR	r1, [r11, #DM_CA9_SCU_PA]
		LDM	r0!, {r2-r6}
		STR	r2, [r1, #SCU_FiltStart]  @ filter start address
		STR	r3, [r1, #SCU_FiltEnd]    @ filter end address
		STR	r4, [r1, #SCU_SAC]        @ SAC
		STR	r5, [r1, #SCU_SSAC]       @ SSAC

		MOVW	r7, #0xFFFF
		STR	r7, [r1, #SCU_InvAll]     @ invalidate the duplicate TAG store

		STR	r6, [r1, #SCU_Ctl]        @ SCU Control
1000:
	.endm

