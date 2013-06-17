
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
* Checked In          : $Date: 2009-06-22 13:40:24 +0000 (Mon, 22 Jun 2009) $
* Revision            : $Revision: 8947 $
* Release Information :
*
*******************************************************************************
 DORMANT_RESTOREx: The Dormant exit sequence macros.

 Macro List:
             DORMANT_RESTORE1  (top level) - for ARMv7-A, PA space restore
             DORMANT_RESTORE2  (top level) - for ARMv7-A, after MMU ON

 NOTE: The framework is common to the A&R profiles where possible

 IMPLEMENTATION DEFINED features can be implemented in-line or as macros

********************************************************************************/

	.macro DORMANT_RESTORE1

		MOV	r11,r0               @ preserve a copy of DORMANT_BASE

	/* L1 I & D caches and the branch predictor buffer are not invalidated
	 * here to reduce dormant exit delay as this is done in ABI-EXT.
	 */
		MOV r1, #0x1800
		MCR p15,0,r1,c1,c0,0     @ enable the Icache and branch prediction (Secure Configuration Register)
		ISB                      @ (as soon as possible)

	@ Restore saved endianess
	.if (ENDIAN_DETECT)
		LDR	r4,[r11,#DM_ENDIAN]
		CMP	r4,#0
		BNE	1001f
		SETEND	LE
		B	1000f
1001:
		SETEND	BE
1000:
	.endif

    /*   IMPLEMENTATION DEFINED - proprietary features: CP15 register 15 etc.
       NOTE: IMP DEF registers might have save and restore order that relate
       to other CP15 registers or logical grouping requirements and can
       therefore occur at any point in this sequence.

       Generic CP15 registers to be restored
       NOTE: it assumed one of MPU or MMU is present
             CP15 register 13 is restored within these blocks*/

	.if (MPExtns_PRESENT )
		MRC	p15,0,r7,c0,c0,5	@ read the MPIDR, MP affinity register
		UBFX r7, r7, #0, #2     @ extract the CPUID field
		                    @ IMP DEF width: Cortex-A9
		CMP	r7, #0	            @ CPUID == 0?
	.if (CA9_SCU_DORMAN == 0)
			BEQ CPU0restore
			LDR	r1, [r11, #DM_CA9_SCU_PA]
			MOV	r3, #0xF
			LSL r7, r7, #2
			LSL r3, r3, r7
			STR	r3, [r1, #SCU_InvAll]  @ invalidate the cpu's SCU TAG store
	.endif
		BNE	NotCPU0restore

CA9SCUrestore_tst:

CPU0restore:
@@@
@@@ IF CPUID == 0, restore SCU, and SYScache
@@@
@@@ NOTE: Other cpu's do not restore SCU or SYScache context,
@@@       (Cpu0+SCU+SYScache first up last down policy)

	.if (CA9_SCU_DORMANT)
CA9SCUrestore:	CA9_SCU_RESTORE  @ Cortex-A9 MP specific
	.endif
	.endif /*MPExtns_PRESENT*/

SYSCACHErestore_tst:

	.if (SECURITY_EXTNS_S) && (PL310_SYSCACHE_DORMANT)
SYSCACHErestore:	PL310_SYSCACHE_RESTORE
	.endif

	NotCPU0restore:         @ else MP support, CPUID != 0

@@@ COMPILE ERROR if an MPU and an MMU are defined


	.if (V7_MPU_PRESENT) && (V7_MMU_PRESENT)
		.err
	.endif

MPUrestore_tst:

	.if (V7_MPU_PRESENT)
MPUrestore:	MPU_RESTORE   @ ONLY applies to R-profile architecture variants
	.endif

MMUrestore_tst:
	.if (V7_MMU_PRESENT)
MMUrestore:	MMU_RESTORE   @ ONLY applies to A-profile architecture variants
	.endif

	.endm

/*******************************************************************************
 DORMANT_RESTORE2 macro

********************************************************************************/
	.macro DORMANT_RESTORE2

CA9GICrestore_tst:
	.if (CA9_GIC_PRESENT)
CA9GICrestore:	CA9_GIC_RESTORE  @ Cortex-A9 MP specific
	.endif

CA9TimersRestore_tst:

	.if (CA9_TIMERS_CONTEXT_SAVE)
CA9TimersRestore:	CA9_TIMERS_RESTORE  @ Cortex-A9 MP specific
	.endif

VFPrestore_tst:
	.if (VFP_PRESENT)
VFPrestore:	VFP_RESTORE
	.endif

     @ Restore banked ARM registers
		LDR	r0,[r11,#DM_bankedARM]  @ load the saved context pointer

	.if (SECURITY_EXTNS_NSfromS)    @ Secure SW required to save NS state?
                                    @ A-profile ONLY
		CPS	#MODE_MON           @ switch to Monitor mode
		LDR	SP,[r0], #4         @ restore the Monitor SP
		LDR	LR,[r0], #4         @ restore the Monitor LR
	.endif

		CPS	#MODE_SYS               @ switch to System mode
		LDR	SP,[r0],#4              @ restore the User SP
		LDR	LR,[r0],#4              @ restore the User LR
		CPS	#MODE_ABT               @ switch to Abort mode
		LDR	SP,[r0],#4              @ restore the current SP
		LDM	r0!,{r4,LR}             @ restore the current LR
		MSR	SPSR_fsxc,r4            @ restore the current SPSR
		CPS	#MODE_UND               @ switch to Undefined mode
		LDR	SP,[r0],#4              @ restore the current SP
		LDM	r0!,{r4,LR}             @ restore the current LR
		MSR	SPSR_fsxc,r4            @ restore the current SPSR
		CPS	#MODE_IRQ               @ switch to IRQ mode
		LDR	SP,[r0],#4              @ restore the current SP
		LDM	r0!,{r4,LR}             @ restore the current LR
		MSR	SPSR_fsxc,r4            @ restore the current SPSR
		CPS	#MODE_FIQ               @ switch to FIQ mode
		LDR	SP,[r0],#4              @ restore the current SP
		LDM	r0!,{r4,r8-r12,LR}      @ restore the current r8-r12,LR
		MSR	SPSR_fsxc,r4            @ restore the current SPSR
		CPS	#MODE_SVC               @ switch back to Supervisor mode

      @ CSSELR – Cache Size Selection Register
		LDR	r3,[r0],#4
		MCR	p15,2,r3,c0,c0,0

		LDR	r4,[r11,#DM_ACTLR]  @ recover ACTLR from fixed address
		LDR	r5,[r11,#DM_SCTLR]  @ recover SCTLR from fixed address

	@ ACTLR - Auxiliary Control Register
		@ PV do not access aux cr in NS mode
		@MCR	p15,0,r4,c1,c0,1
		ISB
		dormant_tracer_v DORMANT_CTRL_PROG_START, r2, r3
	@ SCTLR - System Control Register
		MCR	p15,0,r5,c1,c0,0
		ISB
		dormant_tracer_v DORMANT_CTRL_PROG_END, r2, r3

	@ Restore CPACR. This must be done after configuring FPU
		LDR	r6,[r11,#DM_CPACR]
		MCR	p15,0,r6,c1,c0,2
		ISB

/*******************************************************************************

        Debug supports:
        1. the optional Extended CP14 interface
        2. the alternative memory-mapped architecture
        3. save/restore of debug context with or without OSlock support

        For the memory mapped case, the addresses must have the Device
        (recommended) or Strongly Ordered memory attribute.

        Another option is for a debugger to override power cycling by assertion
        of a DBGNOPWRDWN signal - see ARMv7-AR debug architecture for details.

*******************************************************************************/
DBGrestore_tst:

	.if (V7_DBG_PRESENT)
@ Restore Debug context
		LDR	r0, [r11, #DM_DBG] @ recover debug context base address
		CMP	r0,#0
		BEQ	SkipDbgRestore @ no debug context available to restore

DBGrestore:	RestoreDebugRegisters DBG_OSSRR, DBG_NoOSLock, DBG_CheckNums,\
		DBG_NumBreakpoints, DBG_NumWatchpoints, DBG_NoMemoryMapped, DBG_NoCP14

SkipDbgRestore:
    .endif

PMonrestore_tst:

	.if (V7_PMon_PRESENT)
PMonrestore:	PMon_RESTORE
	.endif


	@ Restore Supervisor mode context
	@ initial restore of r0, DORMANT_BASE pointer
		ADD	r0,r11,#(DM_BLOCK+4*14)
		LDM	r0,{r1,r2}	  @ recover the CPSR and SPSR
		MSR	CPSR_fsxc,r1	  @ restore the CPSR
		MSR	SPSR_fsxc,r2	  @ restore the SPSR
		SUB	r0,r0,#(4*14)
		LDR	SP, [r0], #4  @ restore the stack pointer
		LDM	r0,{r1-r12,r14}	  @ restore other ARM registers
		                      @ (except r0 and PC)
		SUB	r0,r0,#(DM_BLOCK + 4) @ restore r0.

	.endm

