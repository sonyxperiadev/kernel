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
********************************************************************************
* R-profile memory management
*
* Macro List:
*             MPU_SAVE
*             MPU_RESTORE
*
* NOTE: The MPU macros are specific to the R-profile.
*       The framework is common to the A&R profiles where possible
*
* IMPLEMENTATION DEFINED features can be implemented in-line or as macros
*
********************************************************************************

********************************************************************************
* DORMANT_MPU_SAVE macro
*
********************************************************************************/
	.macro MPU_SAVE			@ Save the PMSA registers
		STR	r0,[r11, #DM_MemMgr] @ save ptr at fixed address

		@ ASSUMPTION: no useful fault address and status information
		@ (stale data) if it is desirable to save this information,
		@ include the following:
		@  MRC p15,0,r2,c5,c0,0   @ DFSR
		@  MRC p15,0,r3,c5,c0,1   @ IFSR
		@  MRC p15,0,r4,c5,c1,0   @ ADFSR
		@  MRC p15,0,r5,c5,c1,1   @ AIFSR
		@  MRC p15,0,r6,c6,c0,0   @ DFAR
		@  MRC p15,0,r7,c6,c0,2   @ IFAR
		@  STM r0!, (r2-r7)       @ save fault status & address context

		MRC	p15,0,r7,c6,c2,0 @ read RGNR and store it to r7
		STR	r7,[r0], #4      @ save the RGNR

		MRC	p15,0,r1,c0,c0,4 @ read the MPU type register
		MOVS	r2, #0	     @ clear r2
	@ determine if unified or Harvard region support
		ANDS	r3,r1,#1
		IT	NE
		UBFXNE	r2,r1,#16,#8    @ if Harvard,extract I.region field->r2
		UBFX	r1,r1,#8,#8	     @ extract U/D region field to r1

	@ save unified/data region context
		CMP	r1,#0            @ test U/D region count
		BEQ	1000f

		MOVS	r3, #0           @ clear r3
1001:
		MCR	p15,0,r3,c6,c2,0 @ write the RGNR
		ISB
		MRC	p15,0,r4,c6,c1,0 @ read DRBAR
		MRC	p15,0,r5,c6,c1,2 @ read DRSR
		MRC	p15,0,r6,c6,c1,4 @ read DRACR
		STM	r0!, {r4-r6}     @ save Dregion context
		ADDS	r3,r3,#1
		CMP	r1,r3
		BNE	1001b

1000:
	@ test for case of unified regions or no MPU fitted
		CMP	r2,#0
		BEQ	1002f

		MOVS	r3, #0             @ clear r3
1003:
		MCR	p15,0,r3,c6,c2,0   @ write the RGNR
		ISB
		MRC	p15,0,r4,c6,c1,1   @ read IRBAR
		MRC	p15,0,r5,c6,c1,3   @ read IRSR
		MRC	p15,0,r6,c6,c1,5   @ read IRACR
		STM	r0!,{r4-r6}        @ save Iregion context
		ADDS	r3,r3, #1
		CMP	r2,r3
		BNE	1003b

1002:
		MCR	p15,0,r7,c6,c2,0  @ Restore RGNR

	@ Save the software context and thread support registers
		MRC	p15,0,r4,c13,c0,1  @ CONTEXTIDR
		MRC	p15,0,r5,c13,c0,2  @ TPIDRURW
		MRC	p15,0,r6,c13,c0,3  @ TPIDRURO
		MRC	p15,0,r7,c13,c0,4  @ TPIDRPRW
		STM	r0!,{r4-r7}

	.endm

/*******************************************************************************
 DORMANT_MPU_RESTORE macro

*******************************************************************************/

	.macro MPU_RESTORE		@ Restore the PMSA registers

		LDR	r0,[r11,#DM_MemMgr] @ Restore MPU configuration region

@ ASSUMPTION: no useful fault address and status information (stale data)
@ if this information is saved, include the following restore instructions:
@
@  LDM r0!, (r2-r7)       @ recover fault status and address context
@  MCR p15,0,r2,c5,c0,0   @ DFSR
@  MCR p15,0,r3,c5,c0,1   @ IFSR
@  MCR p15,0,r4,c5,c1,0   @ ADFSR
@  MCR p15,0,r5,c5,c1,1   @ AIFSR
@  MCR p15,0,r6,c6,c0,0   @ DFAR
@  MCR p15,0,r7,c6,c0,2   @ IFAR

	@ recover RGNR to r7. It will be restored later
		LDR	r7,[r0],#4

		MRC	p15,0,r1,c0,c0,4 @ read the MPU type register
		MOVS	r2, #0           @ initialise r2 to 0
	@ determine if unified or Harvard region support
		ANDS	r3,r1,#1
		IT	NE
		UBFXNE	r2,r1,#16,#8     @ if Harvard,extract I region field->r2
		UBFX	r1,r1,#8,#8      @ extract U/D region field to r1

	@ Restore unified/data region context
		CMP	r1,#0            @ test for case of no MPU fitted
		BEQ	1000f

		MOVS	r3, #0           @ clear r3
1001:
		LDM	r0!,{r4-r6}      @ recover Dregion context
		MCR	p15,0,r3,c6,c2,0 @ write the RGNR
		ISB
		MCR	p15,0,r4,c6,c1,0 @ restore DRBAR
		MCR	p15,0,r5,c6,c1,2 @ restore DRSR
		MCR	p15,0,r6,c6,c1,4 @ restore DRACR
		ADDS	r3,r3,#1
		CMP	r1,r3
		BNE	1001b

1000:
	@ test for case of unified regions or no MPU fitted
		CMP	r2, #0
		BEQ	1002f

		MOVS	r3, #0             @ clear r3
1003:
		LDM	r0!,{r4-r6}        @ recover Iregion context
		MCR	p15,0,r3,c6,c2,0   @ write the RGNR
		ISB
		MCR	p15,0,r4,c6,c1,1   @ restore IRBAR
		MCR	p15,0,r5,c6,c1,3   @ restore IRSR
		MCR	p15,0,r6,c6,c1,5   @ restore IRACR
		ADD	r3,r3,#1
		CMP	r2,r3
		BNE	1003b

1002:
		MCR	p15,0,r7,c6,c2,0   @ restore RGNR from R7

	@ Restore the software context and thread support registers
		LDM	r0!,{r4-r7}
		MCR	p15,0,r4,c13,c0,1  @ CONTEXTIDR
		MCR	p15,0,r5,c13,c0,2  @ TPIDRURW
		MCR	p15,0,r6,c13,c0,3  @ TPIDRURO
		MCR	p15,0,r7,c13,c0,4  @ TPIDRPRW
	@ Apply the updates to the CP15 registers.
		ISB

	@ Enable the MPU if it was enabled before dormant.
@		LDR	r2,[r11,#DM_SCTLR] @ Load SCTLR
@		AND	r3,r2,#1           @ Keep only the MPU Enable bit
@		MRC	p15,0,r4,c1,c0,0
@		ORR	r4,r4,r3
@		DMB
@		MCR	p15,0,r4,c1,c0,0
@		ISB

	.endm

