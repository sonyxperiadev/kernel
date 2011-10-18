/*******************************************************************************
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
* Checked In          : $Date: 2009-06-10 07:27:28 +0000 (Wed, 10 Jun 2009) $
* Revision            : $Revision: 8862 $
* Release Information :
*
*******************************************************************************
 DORMANT_SAVE: The Dormant entry sequence macro.
               VFP, PMon etc. macros are nested as required

 Macro List:
             DORMANT_SAVE  (top level)

       The framework is common to the A&R profiles where possible

 IMPLEMENTATION DEFINED features can be implemented in-line or as macros

********************************************************************************/

/* DORMANT_SAVE can be called independently of the DORMANT_RESTOREx functions.
 Uses registers and the CPSR. */

	.macro DORMANT_SAVE

	/* Save all Supervisor mode context
	 Account for a block to save DM_* configurations (fixed offset)
	 Arguments in r0 and r1 saved after block initialisation*/
		ADD	r0,r0,#DM_BLOCK
		STR	SP, [r0], #4          @ save the stack pointer
		STM	r0!,{r1-r12,r14}      @ save ARM registers (except PC)
		SUB	r11,r0, #(DM_BLOCK + 4*14)@ preserve a copy of DORMANT_BASE
		MRS	r4, CPSR
		MRS	r5, SPSR
		STM	r0!,{r4-r5}	@ save the current CPSR and SPSR

		MOVS	r4, #DM_BLOCK
		MOVS	r5, #0
		MOVS	r6, #0

	/* Save endianness.
	 The saved value is 0x0 for little endian and 0xffffffff for big endian.
	 NOTE: Endian state should be recovered first to restore data correctly.*/
	.if (ENDIAN_DETECT)
		SBFX	r4,r4,#9,#1   @ extract and sign-extend CPSR.E
		STR	r4,[r11,#DM_ENDIAN]
		MOVS	r6, #4        @ DM_ENDIAN valid, do not initialise
	.endif

	/* initialise the reserved (DM_BLOCK) context to zeroes, except DM_ENDIAN
		null ptr == feature not saved, overwritten on context save
	*/
1000:
		STR	r5, [r11,r6]
		ADD	r6, r6, #4
		CMP	r6, r4
		BNE	1000b

	@@ save arguments supplied to DORMANT_SAVE: arg0 and arg1
		STR	r11,[r11, #DM_ContextBase_VA]	@ save arg0, VA of the context pointer
							@ arg0 PA supplied by the reset handler
		STR	r1,[r11, #DM_CA9_SCU_VA]	@ save arg1, VA of the CA9_SCU_BASE
							@ NOTE: VA == PA when direct-mapped
    .if (CA9_SCU_PRESENT)
		MOV	r4, r1
DMsave:	VA_TO_PA r4,r5,r6               @ VA => PA of arg1
		STR	r4,[r11, #DM_CA9_SCU_PA]    @ save PA of the CA9_SCU_BASE
    .endif

PMonsave_tst:
	.if (V7_PMon_PRESENT)
PMonsave:	PMon_SAVE	@ For profiling transparency of power management
					@ overhead, save PMon context here.
	.endif

	/* Save banked ARM registers
	 Save a pointer to the start of the banked register context offset */
		STR	r0,[r11,#DM_bankedARM] @ save at fixed address
	.if (SECURITY_EXTNS_ValidS)
		@ Monitor Mode in use? A-profile ONLY
		CPS	#MODE_MON         @ switch to Monitor mode
		STR	SP,[r0], #4       @ save the User SP
		STR	LR,[r0], #4       @ save the User LR
	.endif

		CPS	#MODE_SYS         @ switch to System mode
		STR	SP,[r0], #4       @ save the Monitor SP
		STR	LR,[r0], #4       @ save the Monitor LR
		CPS	#MODE_ABT         @ switch to Abort mode
		STR	SP,[r0], #4       @ save the current SP
		MRS	r4,SPSR
		STM	r0!,{r4,LR}       @ save the current SPSR, LR
		CPS	#MODE_UND         @ switch to Undefined mode
		STR	SP,[r0], #4       @ save the current SP
		MRS	r4,SPSR
		STM	r0!,{r4,LR}       @ save the current SPSR, LR
		CPS	#MODE_IRQ         @ switch to IRQ mode
		STR	SP,[r0], #4       @ save the current SP
		MRS	r4,SPSR
		STM	r0!,{r4,LR}       @ save the current SPSR, LR
		CPS	#MODE_FIQ         @ switch to FIQ mode
		STR	SP,[r0], #4       @ save the current SP
		MRS	r4,SPSR
		STM	r0!,{r4,r8-r12,LR}@ save the current SPSR,r8-r12,LR
		CPS	#MODE_SVC         @ switch to Supervisor mode

	@ Generic CP15 registers

	@ CSSELR – Cache Size Selection Register
		MRC	p15,2,r3,c0,c0,0
		STR	r3,[r0], #4

	/* IMPLEMENTATION DEFINED - proprietary features:
	 (CP15 register 15, TCM support, lockdown support, etc.)

	 NOTE: IMP DEF registers might have save and restore order that relate
	 to other CP15 registers or logical grouping requirements and can
	 therefore occur at any point in this sequence.

	 ACTLR - Auxiliary Control Register
	 pv do not access actlr in NS mode*/

	@ MRC	p15,0,r4,c1,c0,1
	@ SCTLR - System Control Register
		MRC	p15,0,r5,c1,c0,0
	@ CPACR - Coprocessor Access Control Register
		MRC	p15,0,r6,c1,c0,2
		STR	r4,[r11, #DM_ACTLR]	@ fixed address
		STR	r5,[r11, #DM_SCTLR]	@ fixed address
		STR	r6,[r11, #DM_CPACR]	@ fixed address

VFPsave_tst:
	.if (VFP_PRESENT)
VFPsave: VFP_SAVE
	.endif

	@ NOTE: it assumed one of MPU or MMU is present
	@       CP15 register 13 is saved within these blocks

CA9GICsave_tst:
	.if (CA9_GIC_PRESENT)
CA9GICsave:	CA9_GIC_SAVE  @ Cortex-A9 MP specific
	.endif

CA9Timersave_tst:
	.if (CA9_TIMERS_CONTEXT_SAVE)
CA9Timersave:	CA9_TIMERS_SAVE  @ Cortex-A9 MP specific
	.endif

@@@ COMPILE ERROR if an MPU and an MMU are defined

	.if ((V7_MPU_PRESENT) && (V7_MMU_PRESENT))
		.err
	.endif

MPUsave_tst:
	.if (V7_MPU_PRESENT)
MPUsave:		MPU_SAVE      @ ONLY applies to R-profile architecture variants
	.endif

MMUsave_tst:
	.if (V7_MMU_PRESENT)
MMUsave:		MMU_SAVE      @ ONLY applies to A-profile architecture variants
	.endif

    .if (L1_POWERDOWN)                     @ IF L1 Dcache has no retention
    .if (SECURITY_EXTNS_NS)            @ IF executing in NS state
          @@@ NOTE: PLACEHOLDER, SMC proxy support of NS space not supported/
          @@@       tested in this version
		@ Secure side would clean and disable the L1.
		@MOV32	r12, CLEANandDISABLE_L1Dcache
		@SMC	#0		@ MonitorCall to clean/disable Dcache
					@ argument passed in r12
    .else				@ ELSE Secure state

		MRC	p15,0,r4,c1,c0,0	@ read the SCTLR
		BIC	r4, r4, #0x4		@
		MCR	p15,0,r4,c1,c0,0	@ clear SCTLR<2>, disable Dcache
		ISB
PWRDNclean:	L1_DCCISW		@ clean + invalidate L1 by set/way

@ IMPLEMENTATION DEFINED - Conditional Cortex-A9 support
@ WARNING: Equivalent may be required to safely remove power
@          from a core within a running snoop environment
@
		.if (CA9_SCU_PRESENT)
			MRC	p15,0,r4,c1,c0,1	@ read the ACTLR
			BIC	r4, r4, #0x40
			MCR	p15,0,r4,c1,c0,1	@ clear ACTLR<6>, the 'SMP' bit
			ISB
		.endif

	.endif @SECURITY_EXTNS_NS
    .endif @L1_POWERDOWN
	.if (MPExtns_PRESENT)
		MRC	p15,0,r7,c0,c0,5	@ read the MPIDR, MP affinity register
		UBFX r7, r7, #0, #2     @ extract the CPUID field
		                        @ IMP DEF width: Cortex-A9
		CMP	r7, #0	            @ if CPUID == 0, then
		BNE	NotCPU0save     @ (branch to ENDIF)
@@@
@@@ IF CPUID == 0, save SCU, SYScache AND flush the L2 context required
@@@ before the MMU is re-enabled (on a restore) from the SYScache.
@@@
@@@ NOTE: Other cpu's do not save SCU or SYScache context,
@@@       and do not flush any SYScache context.
@@@       (Cpu0+SCU+SYScache first up last down policy)

CA9SCUsave_tst	:
		.if(CA9_SCU_DORMANT)
CA9SCUsave:	CA9_SCU_SAVE  @ CA9 SCU for multiprocessing
		.endif
	.endif

SYSCACHEsave_tst:
	.if ((SECURITY_EXTNS_S) && (PL310_SYSCACHE_DORMANT))
SYSCACHEsave:	PL310_SYSCACHE_SAVE

   @@@ CLEAN THE REQUIRED CONTEXT FROM $L2 CACHE AND DISABLE IT HERE.
   @@@ IN THIS EXAMPLE, THE CACHE CLEAN OF MMU, SCU, AND SYSCACHE
   @@@ CONTEXT TAKES ADVANTAGE OF THEIR SAVE ORDER.

		LDR	r4, [r11, #DM_MemMgr]	@ start address of the block clean
						@ (A-profile assumed)
		MOVW	r2, #(PL310_LINELEN - 1)
		BIC	r4,r4,r2		@ align PA to the cacheline

		LDR	r1, [r11, #DM_CA9_SCU_VA]
@		MOVW	r2, #(CA9_SCU_L2CC + 0x700)
		LDR	r2, =(0x20700)
		ADD	r1,r1,r2		@ r1 set up for PL310 cache operations
		MOV	r3,r0
DMsyscache1:
		VA_TO_PA r3,r5,r6

SYSCACHEclean1:
		STR	r4, [r1, #PL310_CleanByPA]
		ADD	r4,r4, #PL310_LINELEN
		CMP	r4,r3			@ r3-4 is the last location saved
		BLT	SYSCACHEclean1

		MOV	r4,r11
DMsyscache2:
		VA_TO_PA r4,r5,r6
		MOVW	r2, #DM_BLOCK
		ADD	r2,r2,r4

	@@@ Clean the reserved block - some values required
SYSCACHEclean2:
		STR	r4, [r1, #PL310_CleanByPA]
		ADD	r4,r4, #PL310_LINELEN
		CMP	r4,r2			@ r2 is start of general context region
		BLT	SYSCACHEclean2
    .endif

/*******************************************************************************

  ARMv7 Debug and Dormant Save/Restore
  ====================================
        Debug supports:
        1. the optional Extended CP14 interface
        2. the alternative memory-mapped architecture
        3. save/restore of debug context with or without OSlock support

        For the memory mapped case, the addresses must have the Device
        (recommended) or Strongly Ordered memory attribute.

        Another option is for a debugger to override power cycling by assertion
        of a DBGNOPWRDWN signal - see ARMv7-AR debug architecture for details.

*********************************************************************************/
NotCPU0save:
	.if (V7_DBG_PRESENT)    @ MP support, CPUID != 0
  	STR	r0,[r11, #DM_DBG] @ save ptr at fixed address

DBGsave: SaveDebugRegisters DBG_OSSRR, DBG_NoOSLock, DBG_CheckNums,\
		DBG_NumBreakpoints, DBG_NumWatchpoints, DBG_NoMemoryMapped, DBG_NoCP14

	.endif

		MOV r0, r11   @ return DORMANT_BASE
	.endm

