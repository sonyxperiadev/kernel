/********************************************************************************
 Copyright (C) 2007-2009 ARM Limited

 This software is provided 'as-is', without any express or implied
 warranties including the implied warranties of satisfactory quality,
 fitness for purpose or non infringement.  In no event will  ARM be
 liable for any damages arising from the use of this software.

 Permission is granted to anyone to use, copy and modify this software for
 any purpose, and to redistribute the software, subject to the following
 restrictions:

 1. The origin of this software must not be misrepresented you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.
 2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
 3. This notice may not be removed or altered from any source distribution.

* SVN Information
* Checked In          : $Date: 2009-05-07 14:20:15 +0100 (Thu, 07 May 2009) $
* Revision            : $Revision: 8622 $
* Release Information :
*
*******************************************************************************
 GIC support, Cortex-A9 SCU block

 Macro List:
             CA9_GIC_SAVE
             CA9_GIC_RESTORE

*******************************************************************************

////////////////////////////////////////////////////

 DORMANT ENTRY - GIC support MACRO

////////////////////////////////////////////////////
********************************************************************************/
	.macro CA9_GIC_SAVE
		@ Save GIC registers
		STR	r0,[r11, #DM_GIC]	@ save ptr at fixed address
		LDR	r1, [r11, #DM_CA9_SCU_VA]
		MOVW r6, #CA9_SCU_ICC
		ADD	r1,r1,r6
		LDR	r2, [r1, #ICC_PMR]	@ interrupt priority mask
		LDR	r3, [r1, #ICC_BPR]	@ interrupt binary point (grouping)
		LDR	r4, [r1, #ICC_ICR]	@ CPU Interface control
		STM	r0!, {r2-r4}

@Set priority mask to the lowest value to block interrupts
		@MOV	r2, #0x00
		@STR	r2, [r1, #ICC_PMR]	; interrupt priority mask

/***********************************************************************
 ERRATA - STYLE NOT APPLICABLE TO MEMORY-MAPPED REGISTERS (CP15 ONLY)
         THE NON-SECURE ICC_{ICR,BPR} MUST USE AN NS-SPACE MEMORY ACCESS
          (FROM THE S-SPACE, CAN BE A SEPARATE MAPPED VA, WITH THE PTE.NS
         BIT SET)
************************************************************************/

@	.if (SECURITY_EXTNS_NSfromS)   @ Secure SW required to save NS state?
@		CPS	#MODE_MON				  @ switch to Monitor mode
@
@ Save non-secure-banked registers
@		MRC	p15,0,r6,c1,c1,0	@ read the SCR
@		ORR	r6, r6, #1			@ set copy of SCR.NS bit
@		MCR	p15,0,r6,c1,c1,0	@ SCR write
@		ISB		@ Ensure non-secure banked registers are accessed.
@		LDR	r2, [r1, #ICC_BPR]	@ interrupt binary point (grouping)
@		LDR	r3, [r1, #ICC_ICR]	@ CPU Interface control (NS version)
@		STM	r0!, {r2-r3}
@
@ switch back to secure register access
@		BIC	r6, r6, #1			@ SCR.NS = 0
@		MCR	p15,0,r6,c1,c1,0	@ SCR write
@		ISB		@ Ensure secure versions of banked registers are accessed.
@		CPS	#MODE_SVC			@ switch to Supervisor mode
@	.endif /*SECURITY_EXTNS_NSfromS*/

		MRC	p15,0,r7,c0,c0,5	@ read the MPIDR, MP affinity register
		UBFX	r7, r7, #0, #2	@ extract the CPUID field
								@ IMP DEF width: Cortex-A9
		LDR	r1, [r11, #DM_CA9_SCU_VA]
		MOVW	r6, #CA9_SCU_ICD
		ADD	r1,r1,r6

		CMP	r7, #0
		MOVNE	r2, #1				@ other cpus must save their own interrupts 0-31
		BNE	10001f
									@ IF CPUID == 0 then ...
		LDR	r2, [r1, #ICD_ICTR]		@ read feature fields
		UBFX	r2, r2, #0, #5		@ extract ITLinesNumber
		ADD	r2, r2, #1
									@ ENDIF CPUID == 0

10001:
		MOV	r3, r2
10002:
   .if (SECURITY_EXTNS_ValidS)
		LDR	r4, [r1, #ICD_ISR]	@ interrupt security
		STR	r4, [r0], #4
    .endif
		LDR	r5, [r1, #ICD_ISER]	@ interrupt set-enable
		STR	r5, [r0], #4
		ADD	r1, r1, #4
		SUBS	r3, r3, #1
		BNE	10002b

		LDR	r1, [r11, #DM_CA9_SCU_VA]
		MOVW	r6, #CA9_SCU_ICD
		ADD	r1,r1,r6
		LSL	r3, r2, #3
10003:
		LDR	r4, [r1, #ICD_IPR]	@ interrupt priority
		LDR	r5, [r1, #ICD_IPTR]	@ interrupt processor targets
		STM	r0!, {r4,r5}
		ADD	r1, r1, #4
		SUBS	r3, r3, #1
		BNE	10003b

		LDR	r1, [r11, #DM_CA9_SCU_VA]
		MOVW	r6, #CA9_SCU_ICD
		ADD	r1,r1,r6
		LSL	r3, r2, #1
10004:
		LDR	r4, [r1, #ICD_ICFR]	@ interrupt configuration
		STR	r4, [r0], #4
		ADD	r1, r1, #4
		SUBS	r3, r3, #1
		BNE	10004b

		CMP	r7, #0
		BNE	10005f         @ IF CPUID != 0, we can skip following part

		LDR	r1, [r11, #DM_CA9_SCU_VA]
		MOVW	r6, #CA9_SCU_ICD
		ADD	r1,r1,r6
		LDR	r2, [r1, #ICD_DCR]	@ Distributor control
		STR	r2, [r0], #4

/*************************************************************************
	* ERRATA - STYLE NOT APPLICABLE TO MEMORY-MAPPED REGISTERS (CP15 ONLY)
	* THE NON-SECURE ICD_DCR MUST USE AN NS-SPACE MEMORY ACCESS
	* (FROM THE S-SPACE, CAN BE A SEPARATE MAPPED VA, WITH THE PTE.NS
	*          BIT SET)
**************************************************************************/
@	.if (SECURITY_EXTNS_NSfromS)  @ Secure SW required to save NS state?
@		CPS	#MODE_MON          @ switch to Monitor mode
@
@ 	Save non-secure-banked registers
@		MRC	p15,0,r6,c1,c1,0	@ read the SCR
@		ORR	r6, r6, #1			@ set copy of SCR.NS bit
@		MCR	p15,0,r6,c1,c1,0	@ SCR write
@		ISB		@ ensure non-secure banked registers are accessed
@
@		LDR	r2, [r1, #ICD_DCR]	@ Distributor control (NS version)
@		STR	r2, [r0], #4
@
@ switch back to secure register access
@		BIC r6, r6, #1			@ SCR.NS = 0
@		MCR p15,0,r6,c1,c1,0		@ SCR write
@		ISB            @ ensure secure versions of banked registers are accessed
@		CPS	#MODE_SVC		@ switch to Supervisor mode
@	.endif

10005:
	.endm

/*******************************************************************************
 DORMANT RESTORE - GIC support MACRO

********************************************************************************/
	.macro CA9_GIC_RESTORE

		MRC	p15,0,r7,c0,c0,5     	@ read the MPIDR, MP affinity register
		TST	r7, r7
		BPL	10001f
		UBFX	r7, r7, #0, #2      @ extract the CPUID field
									@ IMP DEF width: Cortex-A9

		LDR	r0,[r11,#DM_GIC]  		@ load the saved context pointer
		CMP	r0,#0
		BEQ	10001f			 		@ no GIC context available to restore

		LDR	r1, [r11, #DM_CA9_SCU_VA]
		MOVW	r6, #CA9_SCU_ICC
		ADD	r1,r1,r6
		LDM	r0!, {r2-r4}
		STR	r2, [r1, #ICC_PMR]   @ interrupt priority mask
		STR	r3, [r1, #ICC_BPR]   @ interrupt binary point (grouping)
		STR	r4, [r1, #ICC_ICR]   @ CPU Interface control

/*******************************************************************************
ERRATA - STYLE NOT APPLICABLE TO MEMORY-MAPPED REGISTERS (CP15 ONLY)
        THE NON-SECURE ICC_{ICR,BPR} MUST USE AN NS-SPACE MEMORY ACCESS
         (FROM THE S-SPACE, CAN BE A SEPARATE MAPPED VA, WITH THE PTE.NS
          BIT SET)
*******************************************************************************/

@	.if (SECURITY_EXTNS_NSfromS)	@ Secure SW required to save NS state?
@		CPS	#MODE_MON          	@ switch to Monitor mode
@
@ Save non-secure-banked registers
@		MRC	p15,0,r6,c1,c1,0   @ read the SCR
@		ORR	r6, r6, #1             @ set copy of SCR.NS bit
@		MCR	p15,0,r6,c1,c1,0   @ SCR write
@		ISB                    @ ensure non-secure banked registers are accessed
@
@		LDM	r0!, {r2-r3}
@		STR	r2, [r1, #ICC_BPR]  @ interrupt binary point (grouping)
@		STR	r3, [r1, #ICC_ICR]  @ CPU Interface control (NS version)
@
@ switch back to secure register access
@		BIC	r6, r6, #1         @ SCR.NS = 0
@		MCR	p15,0,r6,c1,c1,0   @ SCR write
@		ISB                    @ ensure secure versions of banked registers
@                              @ are accessed
@		CPS #MODE_SVC          @ switch to Supervisor mode
@	.endif

		LDR	r1, [r11, #DM_CA9_SCU_VA]
		MOVW	r6, #CA9_SCU_ICD
		ADD	r1,r1,r6
		CMP	r7, #0
		MOVNE	r2, #1
		BNE 10002f
                               @ IF CPUID == 0 then ...
		LDR	r2, [r1, #ICD_ICTR]  @ read feature fields
		UBFX r2, r2, #0, #5      @ extract ITLinesNumber
		ADD	r2, r2, #1
                               @ ENDIF CPUID == 0

10002:
		MOV r3, r2
10003:
	.if (SECURITY_EXTNS_ValidS)
		LDR	r4, [r0], #4
		STR	r4, [r1, #ICD_ISR]   @ interrupt security
	.endif
		LDR	r5, [r0], #4
		STR	r5, [r1, #ICD_ISER]  @ interrupt set-enable
		ADD	r1, r1, #4
		SUBS	r3, r3, #1
		BNE 10003b

		LDR	r1, [r11, #DM_CA9_SCU_VA]
		MOVW	r6, #CA9_SCU_ICD
		ADD	r1,r1,r6
		LSL r3, r2, #3
10004:
		LDM	r0!, {r4,r5}
		STR	r4, [r1, #ICD_IPR]   @ interrupt priority
		STR	r5, [r1, #ICD_IPTR]  @ interrupt processor targets
		ADD	r1, r1, #4
		SUBS	r3, r3, #1
		BNE 10004b

		LDR	r1, [r11, #DM_CA9_SCU_VA]
		MOVW	r6, #CA9_SCU_ICD
		ADD	r1,r1,r6
		LSL r3, r2, #1
10005:
		LDR	r4, [r0], #4
		STR	r4, [r1, #ICD_ICFR]  @ interrupt configuration
		ADD	r1, r1, #4
		SUBS	r3, r3, #1
		BNE 10005b

		CMP	r7, #0
		BNE 10001f      		@ IF CPUID != 0, we can skip following part

		LDR	r1, [r11, #DM_CA9_SCU_VA]
		MOVW	r6, #CA9_SCU_ICD
		ADD	r1,r1,r6
		LDR	r2, [r0], #4
		STR	r2, [r1, #ICD_DCR]   @ Distributor control
                               @ OK to enable as IRQ/FIQ masked

/*******************************************************************************
 ERRATA - STYLE NOT APPLICABLE TO MEMORY-MAPPED REGISTERS (CP15 ONLY)
          THE NON-SECURE ICD_DCR MUST USE AN NS-SPACE MEMORY ACCESS
          (FROM THE S-SPACE, CAN BE A SEPARATE MAPPED VA, WITH THE PTE.NS
           BIT SET)
*******************************************************************************/
@	.if (SECURITY_EXTNS_NSfromS)		@ Secure SW required to save NS state?
@		CPS #MODE_MON          		@ switch to Monitor mode

@ Save non-secure-banked registers
@		MRC	p15,0,r6,c1,c1,0   @ read the SCR
@		ORR	r6, r6, #1             @ set copy of SCR.NS bit
@		MCR	p15,0,r6,c1,c1,0   @ SCR write
@		ISB                    @ ensure non-secure banked registers are accessed
@
@		LDR	r2, [r0], #4
@		STR	r2, [r1, #ICD_DCR] @ Distributor control (NS version)
@
@ switch back to secure register access
@		BIC	r6, r6, #1         @ SCR.NS = 0
@		MCR	p15,0,r6,c1,c1,0   @ SCR write
@		ISB                    @ ensure secure versions of banked registers
@                              @ are accessed
@		CPS #MODE_SVC          @ switch to Supervisor mode
@	.endif

10001:
 	.endm

/****************************************************************************
*      End of code
*****************************************************************************/

