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
     ARMv7-AR Dormant mode support
     =============================

     The dormant mode DormantSupport() example function uses a block of
     memory with base address DORMANT_BASE.


 Assumptions:
 ===========
     An ARMv7-R or ARMv7-A compliant system

     IF DORMANT_BASE memory is cacheable, context that will be restored before
     a retained cache is enabled must be cleaned to memory that is visible at
     reset.

     The system power controller state used to determine a warm start
     (POWER_CONTROLLER_WARM_START_STATUS etc.) is saved in Device or
     non-cacheable Normal memory.

     In a multiprocessor system, the example assumes cpu0 is the last core to
     power-down, and the first core to power up:
     - cpu0 is the ONLY core that can support L1$D retention - other cores
       must be restored with invalid caches when they rejoin the cluster. If
       the example code is a common executable for all cpus in a cluster, it
       must be configured with L1_POWERDOWN == TRUE, or modified.
     - only cpu0 can control power gating of the SCU and GIC
     - when cpu0 saves the SCU and GICX context, it is assumed there is no
       ACP port activity.
     - the example ensures that cores other than cpu0 are all OFF before cpu0
       completes its context save phase and signals its readiness for the
       environment to enter dormant mode.

     The primary code support for dormant save/restore is consistent with a
     function call from supervisor mode:
     - DORMANT_BASE passed as an argument in r0
     - function entered by a BL{X} or a PUSH {reg-list incl. LR} instruction
     - function return uses BX LR or POP {reg-list with 'LR' => PC}
     - execution starts in Supervisor mode

     In addition:
     - Instructions are 'clean' and can be read from cache or main memory.
     - Execution of the save context code will not generate an abort.
     - For the ARMv7-A profile, the dormant save and restore (reset handler)
       can be used on any core assuming the handler is direct-mapped.
       PLEASE NOTE:
       The code for a translated address space is implementation specific
       unless both the direct-mapped and translated virtual address entries
        are present in the active page table when the MMU is switched on.

 ARMv7 debug through powerdown
 =============================

 To maintain a debug connection through the execution of dormant mode save
 and restore code, power must be maintained to the resources accessed by
 the debugger. This requires an SoC supporting split power domains for the
 core and its associated DAP.

 The ARM debug architecture supports the following mechanisms for this:
 (a) PRCR.DBGNOPWRDWN - part of the core. When asserted, a system power
                        controller must not remove power from the
                        associated core.
 (b) CDBGPWRUPREQ/ACK - signals in the Debug Access Port (DAP).
     The signals are global to the debug control domain. The signals
     are used by a debugger to ensure debug specific resources are
     powered up and available to it.
 (c) CSYSPWRUPREQ/ACK - signals in the Debug Access Port (DAP).
     The signals are redundant in a single power domain. They are used
     in a split power domain for a debugger to communicate with more
     than debug specific resources in a system.


 System level assumptions:
 ========================
     WFI wakeup events not required for dormant exit are inhibited at
     the system level where necessary.

     External bus activity with the core, by example to a TCM, is suppressed.

     The handshake of the power controller, STANDBYWFI, the PL310 IDLE signal
     and debug (PRCR.DBGNOPWRDWN and CDBGPWRUPREQ/ACK) is external to the
     core.

 The processor determines that it wishes to enter dormant mode and generates
 a function call to a power save routine:
 Assumptions
    1: the system is running in a {secure} privileged mode
    2: PUSH and POP usage implies full-descending stacks
    3: Supervisor mode is the primary mode for save and restore
       (ARMv7-A and –R)


 SECURITY EXTNS: three scenarios for power saving identified:

 a) A 'legacy' OS in a system that does not use TrustZone, powers up in the
    Secure state. The system has no software knowledge or need for Monitor
    mode, S-only and NS-banked system control registers.
 b) A system that supports TrustZone, and uses the Secure state to save and
    restore everything
 c) A system that supports TrustZone and requires minimal coding and most
    importantly minimal (ideally no) variance to the secure-side power
    management code.

             <--------- SECURITY_EXTNS_ ---------------->
              S          NS        NSfromS        ValidS
    (a)      TRUE       FALSE       FALSE          FALSE
    (b)      TRUE       FALSE       TRUE           TRUE
    (c)      FALSE      TRUE        FALSE          FALSE (NS-side)
             TRUE       FALSE       FALSE          TRUE  (S-side)
*******************************************************************************/

#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/asm-offsets.h>
#include <asm/memory.h>

#include <mach/secure_api.h>
#include <mach/pm.h>

@ IMPLEMENTATION DEFINED: presence and usage status of the Security Extensions
@ NOTE: all current/planned ARM implementations of ARMv7-A support
@       the Security Extensions

#define ENDIAN_DETECT  			 0  /* CPSR.E assumed static from reset */
#define FCSE_ACTIVE              0  /* default not in use/not supported */
#define SECURITY_EXTNS_S         0   /* SYSTEM DEFINED */
#define SECURITY_EXTNS_NS        1  /* SYSTEM DEFINED */
#define SECURITY_EXTNS_NSfromS   0   /* SYSTEM DEFINED */
#define SECURITY_EXTNS_ValidS    0   /* SYSTEM DEFINED */
#define VBAR_IN_USE              1   /* IMPLEMENTATION DEFINED */
#define MPExtns_PRESENT          1   /* IMPLEMENTATION DEFINED */
#define L1_POWERDOWN             1   /* SYSTEM DEFINED */
#define L2_CACHED_SAVE           1   /* SYSTEM DEFINED */
#define VFP_PRESENT              1   /* IMPLEMENTATION DEFINED */
#define V7_DBG_PRESENT           1   /* IMPLEMENTATION DEFINED */
#define V7_MMU_PRESENT           1   /* ARMv7, A-profile */
#define V7_MPU_PRESENT           0  /* ARMv7, R-profile */
#define V7_PMon_PRESENT          1   /* IMPLEMENTATION DEFINED */


#define DORMANT_AUTOCONFIG 	1      /* Use ID registers to compute
                                     cache operation parameters etc. */

/* Debug support only – software interface

   NOTE: the design style of the debug routines is derived from a
         source file that supports a wider range of features than
         the options supported in this example code.

 Settings for ARM processors:

 Field             ARM11  Cortex-A8   Cortex-A9r0   Cortex-A9r1
 ==============================================================
 OSSRR             -      TRUE        -             -
 NoOSLock          TRUE   -           TRUE          TRUE
 NoCP14            -      TRUE        -             -
 NoMemoryMapped    TRUE   -           TRUE          TRUE
 NumBreakpoints    6      6           6             6
 NumWatchpoints    6      2           2             4
*/


@ Cortex-A9 configuration

#define DBG_OSSRR             0
#define DBG_NoOSLock          1
#define DBG_NoCP14            0
#define DBG_NoMemoryMapped    1
#define DBG_NumBreakpoints    6
#define DBG_NumWatchpoints    4     /* Cortex-A9r1, r0 == 2*/
#define DBG_CheckNums         0



/* Example Cortex-A8 configuration*/

#if 0

#define DBG_OSSRR             1
#define DBG_NoOSLock          0
#define DBG_NoCP14            1
#define DBG_NoMemoryMapped    0
#define DBG_NumBreakpoints    6
#define DBG_NumWatchpoints    2
#define DBG_CheckNums         0

#endif

/* Cortex-A9 system specific features */

#define DIRECT_MAPPED_SAVE  0 /*  CA9 specific solution in example */
                                /*  recovery code, hence listed here */

#define CA9_SCU_DORMANT    			1 /*   Cortex-A9 MP system */
#define CA9_SCU_PRESENT    			1 /*   Cortex-A9 MP system */
#define CA9_GIC_PRESENT    			1 /*   Cortex-A9 MP system */
#define CA9_TIMERS_CONTEXT_SAVE    	1 /*   save/restore the local timers/watchdog */
#define CA9_TIM64_CONTEXT_SAVE     			1 /*   save/restore the 64-bit timer context */
#define PL310_SYSCACHE_PRESENT  	1 /*  SYSTEM DEFINED */
#define PL310_SYSCACHE_DORMANT  	1 /*  TRUE == L2 memory retention */

    .extern dormant_base_va
    .extern dormant_base_pa

#ifdef DORMANT_PROFILE
    .global dormant_profile_on
    .global ns_gpio_set_p
    .global ns_gpio_clr_p
    .global ns_gpio_set_v
    .global ns_gpio_clr_v
    .global ns_gpio_bit
#endif

#include "dm_pwr_save_dfn.h"
#include "dm_support_ops.S"
#include "dm_ca9_gic_mcrs.S"
#include "dm_ca9_scu_mcrs.S"
#include "dm_ca9timers_mcrs.S"
#include "dm_mmu_mcrs.S"
#include "dm_mpu_mcrs.S"
#include "dm_pl310_syscache_mcrs.S"
#include "dm_pmon_mcrs.S"
#include "dm_vfp_mcrs.S"
#include "dm_pwr_save_mcrs.S"          /* top level power save macro */
#include "dm_pwr_restore_mcrs.S"        /* top level power restore macros */
#include "dm_pwr_dbg_mcrs.S"


    .macro dormant_gpio_update reg, r0, r1
#ifdef DORMANT_PROFILE
    ldr    \r0,dormant_profile_on
    cmp    \r0,#0
    ldrne  \r1,\reg
    ldrne  \r0,ns_gpio_bit
    strne  \r0,[\r1]
#endif
    .endm

@ bool dormant_start(void);
ENTRY(dormant_start)
@    BL PWRMGMT_test_a9DeepSleep
    PUSH {LR}           @ ASSUMPTION: valid stack exists, save LR
                        @ 2x PUSH to emphasise the hierarchy in this example
    PUSH {r1-r2}        @ create temporary storage for the CPSR
                        @ and local variables
    MRS r1, CPSR

@@ The power save routine in the example then assumes a transition to Supervisor
@@ mode is required before calling a function to save all state.

    CPSID aif , #MODE_SVC   @ disable interrupts
                            @ (and change mode if necessary)
    PUSH {r1,LR}            @ save LR and the return CPSR on the SVC stack
    .if(CA9_SCU_PRESENT)       @ if an MP system, calculate context save pointer
        MRC	p15,0,r2,c0,c0,5	@ read the MPIDR, MP affinity register
	    UBFX r2, r2, #0, #2		@ extract the CPUID field
								@ IMP DEF width: Cortex-A9
        LDR r0, =DORMANT_BASE  @ pointer to context save area
        LDR r0, [r0]
        LDR r1, =DORMANT_BASE_CPU_OFFSET
        MUL r1, r1, r2
        ADD r0, r0, r1
    .else
        LDR r0, =DORMANT_BASE  @ pointer to uniprocessor context save area
        LDR r0, [r0]
    .endif
    LDR r1, =CA9_SCU_BASE  @ pointer to the CA9 memory-mapped resources
                            @ r0 and r1 are arguments (can be VAs)
    BL DormantSave
                       @@ DormantSave => WFI => powerdown
@ At this point r0 must contain:
@ 0x0 - dormant entry failed (SMC returned without dormant entry)
@ 0x1 - returned via dormant boot
WarmStartContinue:      @ return here on exit from DORMANT_RESTORE2
    POP {r1,LR}        @ recover LR and the CPSR value
    MSR CPSR_fsxc, r1  @ restore the CPSR incl. mode change if necessary
    ISB
    POP {r1-r2}        @ restore 'temporary storage' from the correct stack
PowerRestored:          @ return to the calling code
    POP {PC}           @ callee entry point: DormantStart

DormantSave:
    DORMANT_SAVE       @ context saving software, r0 argument 'returned'


@ Update power controller to indicate a warm start required
@ Exact requirement IMPLEMENTATION DEFINED, for example power level on restore
@ The sequence might require additional write(s)

    LDR r5, =POWER_CONTROLLER_CONTEXT_BASE_VA

    .if(CA9_SCU_PRESENT)       @ if an MP system, calculate offset addresses
        MRC	p15,0,r1,c0,c0,5	@ read the MPIDR, MP affinity register
	    UBFX r1, r1, #0, #2		@ extract the CPUID field
								@ IMP DEF width: Cortex-A9
        LSLS r1, r1, #2
        ADD r5, r5, r1          @ MP: modified ptr for base address
    .endif

	.if(V7_MMU_PRESENT)
    	MOV r4, r0   @ copy pointer to the context save area
DMtop:	VA_TO_PA r4, r7, r6
    	STR r4, [r5] @ Converted VAtoPA value saved in the power controller
    	LDR r1, =PAtoVAentry
@set LSB if in thumb mode
    	@ORR r1,r1,#1  @ ensure the link address is marked as 'Thumb'
    	STR r1, [r0, #DM_returnVA] @ save VA entry point as part of the context
    .else
    	STR r4, [r5] @ DORMANT_BASE (PA) value saved in the power controller
    .endif
    DSB

    .if (CA9_SCU_PRESENT)       @ if an MP system
		LDR	r1, [r0, #DM_CA9_SCU_VA]
		MRC	p15,0,r7,c0,c0,5	@ read the MPIDR, MP affinity register
		UBFX	r7, r7, #0, #2  @ extract the CPUID field
                                @ IMP DEF width: Cortex-A9
		CMP	r7, #0	            @ if CPUID == 0, then
		BNE	UpdatePwrStatus

    @@ Prepare PL310 for power down (MP && CPU0)
	    .if L2CC_BASE       @ IMPLEMENTATION DEFINED, PL310 present
    		LDR	r1, [r0, #DM_CA9_SCU_VA]
			LDR	r2, =CA9_SCU_L2CC
			ADD	r2, r2, #0x730  @ addr offset for a PL310 cache sync operation
			STR r0, [r1, r2]    @ execute a PL310 cache sync operation
			DSB
        .endif

		/*  Rhea is a single core so this is not needed.*/
		@@@ ENSURE CPU0 IS POWERED DOWN LAST - MP POLICY IN THIS EXAMPLE
		@@LDR r3, [r1, #SCU_Config]
		@@UBFX	r3, r3, #0, #2  @ extract the # of cpus in the MP cluster
		@@EOR	r3, r3, #3   @ values 0 -> 3 now 3 -> 0
		@@LSLS r3, r3, #3  @ r3 x 8

		@@MOV32	r4, #0x03030303
@		LSRS r4, r4, r3  @ only compare valid cpu bitfields
		@@BIC	r4, r4, #3   @ clear cpu0 bitfield
			             @ (value written after the compare)
2000:
		@@LDR	r5, [r1, #SCU_PwrStatus]  @ ensure CPU0 is powered down last
		@@CMP r5, r4
		@@BNE	2000b
		@ endif (CPUID == 0)
UpdatePwrStatus:
	/****************************************************************
	*
	* IMPLEMENTATION DEFINED - update Cortex-A9 power register
	* WARNING: SYSTEM DEFINED behaviour
	*
	*****************************************************************/
@		MOV	r2, #3
		MOV	r2, #1		@set for dormant mode
		@ NOTE: for Cortex-A9 = r1p0 and later revisions ONLY
		ADD	r3, r7, #SCU_PwrStatus
		@ each cpu has its own byte in the PwrStatus register, no lock required
		@ STRB	r2, [r1, r3] @ Let higher layers or sec write it
	.else
    @@ Prepare PL310 for power down (uniprocessor)

	    .if L2CC_BASE       @ IMPLEMENTATION DEFINED, PL310 present
    		LDR	r1, [r0, #DM_CA9_SCU_VA]
			LDR	r2, =CA9_SCU_L2CC
			ADD	r2, r2, #0x730    @ addr offset for a PL310 cache sync operation
			STR r0, [r1, r2]    @ execute a PL310 cache sync operation
			DSB
		.endif
    .endif @CA9_SCU_PRESENT

/*
	Broadcom customization :
	Jump to bootrom function to save/restore secure mode registers & enter WFI

	CONFIG_ROM_SEC_DISPATCHER is not defined as rom_sec_dormant_enter is not defined for A0 ROW
*/
#ifdef CONFIG_ROM_SEC_DISPATCHER
	@ The first dword of the buffer is the dormant exit routine
	@ Save the handler at the first index.
	LDR r0,=EnterResetHandler
	VA_TO_PA r0, r1, r2
	STR   r0,[r11]


	@ rom_sec_dormant_enter ( service, flags, start_addr, end_addr )
	@ Enter Dormant
	LDR r0,=SEC_API_ENTER_DORMANT
	@ Disable interrupts.. i.e do not pass interrupts to NS side.
	LDR r1,=SEC_FLAGS
	@ Start of buffer
	LDR r2, =DORMANT_BASE_PA  @ pointer to uniprocessor context save area
	LDR r2, [r2]
	@ End of buffer.. Note the actual buffer is larger but
	@ secure side max is 0x280@
	@ Pass end address as 3K.
	@Total buffer size is 4K and 1 K from end address is used
	@ by secure handler for MEMC workaround
	ADD   r3,r2,#0xC00
	MOV   r4,#SEC_DORMANT_MODE
	PUSH  {r4}

	@ invoke the dispatcher
	bl hw_sec_pub_dispatcher
#else
	@ Do not enter dormant but enter suspend
	BLX enter_wfi
#endif /*CONFIG_ROM_SEC_DISPATCHER*/

LateWakeupEventHandler: @ Secure routine returned without entering dormant here
	dormant_gpio_update ns_gpio_set_v, r0, r1
/*****************************************************************
    In case if we do not enter dormant, the pc would
    come down to this place.

    MMU remains un-disabled.  Let us continue from the second part
    dormant restore.
*****************************************************************/
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
    MOV r0,#0             @ Indicate dormant entry failed
    MOV	pc,lr		          @ execution returns to WarmStartContinue

    .align 5
EnterResetHandler:
	dormant_gpio_update ns_gpio_set_p, r0, r1
	dormant_tracer_p DORMANT_RESTORE1_START, r0, r1

DormantRestoreStart:
   	@ copy of DORMANT_BASE => r0 as an 'argument' for DORMANT_RESTORE1
   	@ MOV32 uses 2x 16-bit immediate loads, system endianness UNKNOWN
   	LDR r5, =POWER_CONTROLLER_CONTEXT_BASE_PA

   	.if (CA9_SCU_PRESENT)       @ if an MP system, calculate offset addresses
        MRC	p15,0,r1,c0,c0,5	@ read the MPIDR, MP affinity register
        UBFX r1, r1, #0, #2		@ extract the CPUID field

        LSLS r1, r1, #2
        ADD r5, r5, r1      @ MP: modified ptr for base address
    .endif

   	LDR r0, [r5]  @ fetch the pointer to the context save/restore area

DormantRestore:
	DORMANT_RESTORE1 @ context recovery software (PA phase)
	dormant_tracer_p DORMANT_RESTORE1_END, r4, r5

	.if (V7_MMU_PRESENT)

	/*
		switch on the MMU for the PA => VA transition here
		NOTE: CA9 specific behaviour that does _not_ require a direct mapped
	       entry in the target page tables when the MMU switches ON.
	       This does _not_ work with Cortex-A8 or if an ISB is included
	       between the MCR and B instructions.

	       An ARMv7-A architecturally compliant port requires:
	           1. an ISB after the MCR p15,0,r1,c1,c0,0 instruction
	           2. Direct-mapped and VA => PA entries for the PAtoVAentry
	              address in the page table that is activated.
	*/

		LDR lr, [r11, #DM_returnVA] @ r11 contains DORMANT_BASE(PA)
        .if (DIRECT_MAPPED_SAVE == 0)
			LDR	r11,[r11, #DM_ContextBase_VA] @ update r11 to the VA value
		.endif
		MCR p15,0,r0,c8,c7,0     @ invalidate all TLB entries on this cpu
		ISB
		LDR r1, =0x1801
		.align 5      @ next two instructions must be in the same cacheline
		MCR p15,0,r1,c1,c0,0     @ enable MMU, Icache and branch prediction
		                         @ leave the $L1Dcache OFF
		                         @ $L2 is ON if SYSCACHE restored
		BX lr
		NOP
		NOP
		NOP
		NOP
PAtoVAentry:
		MCR p15,0,r0,c8,c7,0     @ invalidate all TLB entries on this cpu
		ISB
	.endif
ContinueRestore:
    /*
    At this point, the general MMU is enabled.  However, Linux seems to be
    using TEX remapping.  Check to see if TEX remapping was enabled before
    entering dormant and restore that bit accordingly
    */
    LDR	r0,[r11,#DM_SCTLR]  @ recover SCTLR from fixed address
    AND r0, r0,#0x10000000
    CMP r0, #0x10000000
    BNE ContinueRestoreNoTEX
    ORR r1, r1, r0
    MCR p15,0,r1,c1,c0,0     @ enable MMU, Icache and branch prediction


ContinueRestoreNoTEX:
	dormant_tracer_v DORMANT_RESTORE2_START, r0, r1
	DORMANT_RESTORE2
	MOV     r0,#1              @ Indicate dormant entry succeeded
	MOV	pc,lr               @ execution returns to WarmStartContinue
FullSystemReset:                   @ nothing to recover, execute a full reset
    @@ B SystemBoot
    B FullSystemReset             @ lockup stub for now
    @...

#ifdef DORMANT_PROFILE
    .ltorg
dormant_profile_on:
    .word  0
ns_gpio_set_p:
    .word  0
ns_gpio_clr_p:
    .word  0
ns_gpio_set_v:
    .word  0
ns_gpio_clr_v:
    .word  0
ns_gpio_bit:
    .word  0
#endif
