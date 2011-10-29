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
* A-profile memory management
*
* Macro List:
*             MMU_SAVE
*             MMU_RESTORE
*
* NOTE: The MMU macros are specific to the A-profile.
*       The framework is common to the A&R profiles where possible
*
* IMPLEMENTATION DEFINED features can be implemented in-line or as macros
*
********************************************************************************

********************************************************************************
* DORMANT_MMU_SAVE macro
*
********************************************************************************/
	.macro MMU_SAVE

	@ Save the VMSA (and Security Extension) registers
	@ ASSUMPTION: the (optional) FCSE register is NOT USED, treated as RAZ/WI
	@ Save the VMSA registers

    .if (DIRECT_MAPPED_SAVE == 0)
		MOV	r4, r0
dm_mmu:
		VA_TO_PA r4,r5,r6
		@ WARNING - ASSUMES MMU RESTORED BEFORE PA => VA TRANSITION
		STR	r4,[r11, #DM_MemMgr] @ save PA ptr at fixed address
    .else
		STR	r0,[r11, #DM_MemMgr] @ save ptr at fixed address
    .endif

	.if (SECURITY_EXTNS_ValidS) 	@ S-side exe and Secure-only registers active?
		CPS	#MODE_MON         		@ switch to Monitor mode

@ Save CP15 register space
@ Save the secure-only registers
		MRC	p15,0,r4,c1,c1,0	@ SCR, NS-bit will be clear here
						@ (assumption used in MMU_RECOVER)
		MRC	p15,0,r5,c1,c1,1	@ SDER
		MRC	p15,0,r6,c1,c1,2	@ NSACR
		MRC	p15,0,r7,c12,c0,1	@ MVBAR
		STM	r0!, {r4-r7}

	.if (SECURITY_EXTNS_NSfromS)  @ Secure SW required to save NS state?
@ Save non-secure-banked registers
		ORR	r4, r4, #1              @ set copy of SCR.NS bit
		MCR	p15,0,r4,c1,c1,0    @ SCR write
		ISB             @ ensure non-secure banked registers are accessed

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

		MRC	p15,2,r4,c0,c0,0   @ CSSELR
		MRC	p15,0,r5,c1,c0,0   @ SCTLR
		STM	r0!, {r4,r5}

		MRC	p15,0,r4,c1,c0,1   @ ACTLR
		MRC	p15,0,r5,c2,c0,0   @ TTBR0
		MRC	p15,0,r6,c2,c0,1   @ TTBR1
		MRC	p15,0,r7,c2,c0,2   @ TTBCR
		STM	r0!, {r4-r7}
		MRC	p15,0,r4,c3,c0,0   @ DACR
		MRC	p15,0,r5,c7,c4,0   @ PAR
		MRC	p15,0,r6,c10,c2,0  @ PRRR
		MRC	p15,0,r7,c10,c2,1  @ NMRR
		STM	r0!, {r4-r7}

@ IMPLEMENTATION DEFINED - TCM, lockdown support
@     CP15 registers 9 and 11

		MRC	p15,0,r4,c12,c0,0  @ VBAR
		STR	r4, [r0], #4
	.if (FCSE_ACTIVE)
		MRC	p15,0,r4,c13,c0,0  @ FCSEIDR
		STR	r4, [r0], #4
	.endif

		MRC	p15,0,r4,c13,c0,1  @ CONTEXTIDR
		MRC	p15,0,r5,c13,c0,2  @ TPIDRURW
		MRC	p15,0,r6,c13,c0,3  @ TPIDRURO
		MRC	p15,0,r7,c13,c0,4  @ TPIDRPRW
		STM	r0!, {r4-r7}

@ IMPLEMENTATION DEFINED - proprietary features - non-secure domain
@     CP15 register 15 etc.

		MRC	p15,0,r4,c1,c1,0   @ SCR read
		BIC	r4, r4, #1         @ SCR.NS = 0
		MCR	p15,0,r4,c1,c1,0   @ SCR write
		ISB            @ ensure secure versions of banked registers are accessed
	.endif @SECURITY_EXTNS_NSfromS
		CPS	#MODE_SVC          @ switch to Supervisor mode
	.endif  @SECURITY_EXTNS_ValidS

@ Save current banked (S or NS) and common CP15 registers

      @@@ SEE DORMANT_SAVE macro for ACTLR, SCTLR, and CPACR saves

      @ MRC p15,0,r5,c1,c0,1   @ ACTLR - Auxiliary Control Register
      @ MRC p15,0,r6,c1,c0,0   @ SCTLR - System Control Register
      @ MRC p15,0,r7,c1,c0,2   @ CPACR - CP Access Control Register
      @ STR r5, [r11,#DM_ACTLR] @ save at fixed address for recovery correctness
      @ STR r6, [r11,#DM_SCTLR] @ save at fixed address for recovery correctness
      @ STR r7, [r11,#DM_CPACR] @ save at fixed address for recovery correctness

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

		MRC	p15,0,r5,c2,c0,0      @ TTBR0
		MRC	p15,0,r6,c2,c0,1      @ TTBR1
		MRC	p15,0,r7,c2,c0,2      @ TTBCR
		STM	r0!, {r5-r7}
		MRC	p15,0,r4,c3,c0,0      @ DACR
		MRC	p15,0,r5,c7,c4,0      @ PAR
		MRC	p15,0,r6,c10,c2,0     @ PRRR
		MRC	p15,0,r7,c10,c2,1     @ NMRR
		STM	r0!, {r4-r7}
@ IMPLEMENTATION DEFINED - TCM, lockdown and performance monitor support
@     CP15 registers 9 and 11

	.if (VBAR_IN_USE)
		MRC	p15,0,r4,c12,c0,0 @ VBAR
		STR	r4, [r0], #4
	.endif

		MRC	p15,0,r4,c13,c0,1    @ CONTEXTIDR
		MRC	p15,0,r5,c13,c0,2    @ TPIDRURW
		MRC	p15,0,r6,c13,c0,3    @ TPIDRURO
		MRC	p15,0,r7,c13,c0,4    @ TPIDRPRW
		STM	r0!, {r4-r7}

@ IMPLEMENTATION DEFINED - proprietary features - secure/shared domain
@     CP15 register 15, for example global valid bits

	.endm

/*******************************************************************************
* DORMANT_MMU_RESTORE macro
*
*******************************************************************************/

	.macro MMU_RESTORE		@ Restore the VMSA and Security Extension registers

		LDR	r0,[r11,#DM_MemMgr] @ Restore MMU configuration region
/*
	Restore CP15 register space
	PLEASE NOTE: the following assumes the CP15SDISABLE mechanism allows a full
	system restore of the System Coprocessor from this reset handler entry point.
	If this is not the case, affected elements are masked, cannot be updated
	(restored), and must be removed from the save/restore procedure.
*/
	.if (SECURITY_EXTNS_NSfromS)
		CPS	#MODE_MON             @ switch to Monitor mode

@ Restore the secure-only registers
		LDM	r0!, {r4-r7}       @ SCR.NS bit (r4) clear from MMU_SAVE
		MCR	p15,0,r5,c1,c1,1   @ SDER
		MCR	p15,0,r6,c1,c1,2   @ NSACR
		MCR	p15,0,r7,c12,c0,1  @ MVBAR
		ORR	r4, r4, #1     @ set the working copy of the SCR.NS bit
		MCR	p15,0,r4,c1,c1,0   @ SCR write
		ISB            @ ensure non-secure banked registers are accessed
@ Restore non-secure-banked registers

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

		LDM	r0!, {r4,r5}
		MCR	p15,2,r4,c0,c0,0   @ CSSELR
		MCR	p15,0,r5,c1,c0,0   @ SCTLR

		LDM	r0!, {r4-r7}
		MCR	p15,0,r4,c1,c0,1   @ ACTLR
		MCR	p15,0,r5,c2,c0,0   @ TTBR0
		MCR	p15,0,r6,c2,c0,1   @ TTBR1
		MCR	p15,0,r7,c2,c0,2   @ TTBCR
		LDM	r0!, {r4-r7}
		MCR	p15,0,r4,c3,c0,0   @ DACR
		MCR	p15,0,r5,c7,c4,0   @ PAR
		MCR	p15,0,r6,c10,c2,0  @ PRRR
		MCR	p15,0,r7,c10,c2,1  @ NMRR

@ IMPLEMENTATION DEFINED - TCM, lockdown and performance monitor support
@     CP15 registers 9 and 11

		LDR	r4, [r0], #4
		MCR	p15,0,r4,c12,c0,0  @ VBAR
	.if (FCSE_ACTIVE)
		LDR	r4, [r0], #4
		MCR	p15,0,r4,c13,c0,0  @ FCSEIDR
	.endif
		LDM	r0!, {r4-r7}
		MCR	p15,0,r4,c13,c0,1  @ CONTEXTIDR
		MCR	p15,0,r5,c13,c0,2  @ TPIDRURW
		MCR	p15,0,r6,c13,c0,3  @ TPIDRURO
		MCR	p15,0,r7,c13,c0,4  @ TPIDRPRW

@ IMPLEMENTATION DEFINED - proprietary features
@     CP15 register 15
		MRC	p15,0,r4,c1,c1,0   @ SCR read
		BIC	r4, r4, #1         @ clear copy of the SCR.NS bit
		MCR	p15,0,r4,c1,c1,0   @ SCR write
		ISB            @ ensure secure versions of banked registers
		               @ are accessed

		CPS	#MODE_SVC          @ switch to Supervisor mode
	.endif

@ Restore secure-banked and shared registers

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

		LDM	r0!, {r5-r7}
		MCR	p15,0,r5,c2,c0,0   @ TTBR0
		MCR	p15,0,r6,c2,c0,1   @ TTBR1
		MCR	p15,0,r7,c2,c0,2   @ TTBCR
		LDM	r0!, {r4-r7}
		MCR	p15,0,r4,c3,c0,0   @ DACR
		MCR	p15,0,r5,c7,c4,0   @ PAR
		MCR	p15,0,r6,c10,c2,0  @ PRRR
		MCR	p15,0,r7,c10,c2,1  @ NMRR

@ IMPLEMENTATION DEFINED - TCM, lockdown and performance monitor support
@     CP15 registers 9 and 11

	.if (VBAR_IN_USE)
		LDR	r4, [r0], #4
		MCR	p15,0,r4,c12,c0,0  @ VBAR
	.endif

		LDM	r0!, {r4-r7}
		MCR	p15,0,r4,c13,c0,1  @ CONTEXTIDR
		MCR	p15,0,r5,c13,c0,2  @ TPIDRURW
		MCR	p15,0,r6,c13,c0,3  @ TPIDRURO
		MCR	p15,0,r7,c13,c0,4  @ TPIDRPRW

@ IMPLEMENTATION DEFINED - proprietary features
@     CP15 register 15, for example global valid bits

	.endm
