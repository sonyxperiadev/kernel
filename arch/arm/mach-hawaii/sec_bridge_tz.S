/*
 * Copyright (C) 2010 Nokia Corporation. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

	.global hw_sec_rom_pub_bridge
        .global hw_mmu_pa_get

	.equ HW_ARM_CP15_CR_M_MASK, (1<<0)
	.equ HW_ARM_CP15_CR_C_MASK, (1<<2)
	.equ HW_ARM_CP15_CR_I_MASK, (1<<12)

/*
 * Following definitions are used to tell public side what kind of security
 * mode exit shall be done.
 */

	.equ SEC_EXIT_NORMAL, 1
	.equ SEC_EXIT_START_EXT_CODE, 2
	.equ SEC_EXIT_INT, 3
	.equ SEC_EXIT_RPC_CALL, 4

/*
 * SSA service IDs.
 */

	.equ SSAPI_PRE_INIT_SERV, 1
	.equ SSAPI_POST_SPEEDUP_INIT_SERV, 2
	.equ SSAPI_ISSW_IMPORT_SERV, 3
	.equ SSAPI_RET_FROM_INT_SERV, 4
	.equ SSAPI_RET_FROM_RPC_SERV, 5
	.equ SSAPI_ISSW_EXECUTE_SERV, 6


/* dword hw_sec_rom_pub_bridge(uint32 appl_id, dword flags, va_list);
 *
 * This function enables secure mode and transfers control to the
 * secure mode entry code.
 *
 * At the end of this function the stack looks like this:
 * SP+44     LR = return address to caller of hw_sec_rom_pub_bridge
 * SP+40     saved R7
 * SP+36     saved R6
 * SP+32     saved R5
 * SP+28     saved R4
 * SP+24     saved R1 (dummy to make 8 byte alignment)
 * SP+20     saved R11
 * SP+16     saved R10
 * SP+12     saved R9
 * SP+8      saved R8
 * SP+4      saved flags (R1)
 * SP+0      CPSR value
 *
 * Defined register contents prior to calling entry code:
 * R4        Service ID (=original R0)
 * R5        Flags (=original R1)
 * R6        Pointer to va_list from R2, converted to physical address
 * R7        -
 * R8        -
 * R9        -
 * R10       -
 * R11       -
 * R8-R13 are supposed to preserve their values over the secure mode call
 */

	.equ CPSR_OFFSET, 0
	.equ FLAGS_OFFSET, 4
	.equ SVCLR_OFFSET, 8
	.equ SVCCPSR_OFFSET, 12
	.equ SVCSP_OFFSET, 16
	.equ DUMMY_OFFSET, 20 @@ To make SP 8-byte aligned
	.equ STACK_INT_USAGE, 24


hw_sec_rom_pub_bridge:        .func

	PUSH    {R1,R4-R7,LR}           @ Save regs R4-R11 and LR to stack (+R1
	MOV     R4, R8
	MOV     R5, R9
	MOV     R6, R10
	MOV     R7, R11
	PUSH    {R4-R7}

	@ Save function name (@R0) to R4 and flags (@R1) to R5 and ap (@R2) to R6 for later use.
	@ REGS: R0,R1,R2,R12-R13
	MOV     R4, R0
	MOV     R5, R1
	MOV     R6, R2

	@ Calculate the physical va_start pointer value and save it to R11 for later
	@ use.
	@ REGS: R4,R5,R6,R12-R13
	MOV     R0, R6
	BLX      hw_mmu_pa_get @ not on same page, calculate!
	MOV     R11, R0

	@ Make room to stack for data saved within this function.
	@ REGS: R4,R5,R6,R11,R12-R13
	SUB     SP, #STACK_INT_USAGE

	@ Save secure mode setting flags to stack.
	@ REGS: R4,R5,R6,R11,R12-R13
	STR     R5, [SP, #FLAGS_OFFSET]     @ Save flags to stack

	@ Save CPSR to stack
	@ REGS: R4,R5,R6,R11,R12-R13
	MRS     R7, CPSR
	STR     R7, [SP, #CPSR_OFFSET]

	@ If DCache was enabled prior to calling this function, we shall do the
	@ DCache cleanings (otherwise it is caller's responsibility to do the
	@ cleaning).
	@ REGS: R4,R5,R11,R12-R13
	MRC     p15, 0, R0, c1, c0, 0
	TST     R0, #HW_ARM_CP15_CR_C_MASK
	BEQ     br11

	@ Clean DCache @ ap to make sure secure mode arguments are available using
	@ physical addresses also after DCache disabling (two cache lines should
	@ do it). Actually this should not be needed with TrustZone but is here just
	@ in case someone makes a bug in public side for example with page coloring.
	@ REGS: R4,R5,R6,R11,R12-R13
	BIC     R1, R6, #0x1F
	ADD     R2, R1, #0x20               @ End address (@ next cache line)
s393:
	MCR     p15, 0, R1, c7, c14, 1      @ Clean DCache single entry (MVA)
	ADD     R1, R1, #(1<<5)             @ Next cache line
	CMP     R2, R1
	BPL     s393

br11:
	MOV     R1, #0
	MCR     p15, 0, R1, c7,c10, 4       @ Drain write buffer

	@ We do not need to do L2 operations here with TrustZone based system

	@ Change R6 to contain pointer to va_list (physical address saved in R11)
	@ instead of ap.
	@ REGS: R4,R5,R11,R12-R13

br20:
	MOV     R6, R11

	@ Jump to common secure mode entry settings part.
	@ REGS: R4,R5,R6,R12-R13
	B       sec_entry_settings

	.endfunc
	.ltorg


/*
 * Following function is the exit point of secure mode code, i.e. where
 * execution returns from secure mode. This function handles several
 * different exit reasons: normal exit, external code start exit, interrupt
 * handling exit, remote procedure call exit and fast exit.  @ The function expects the following register values:
 * R0-R3     Parameters from secure side (for RPC call & ext code start)
 * R4        RPC handler address -or- ext code start address -or- zero
 * R5-R7     -
 * R8-R11    Preserved values from before entering secure mode
 * R12       Exit reason
 * R13       Preserved SP from before entering secure mode
 * R14       -
 */

	.align 2

ret_from_sec_mode:          .func

	@ Invalidate TLB unlocked entries
	MOV     R5, #0
	MCR     p15, 0, R5, c8, c7, 1

	MCR     p15, 0, R5, c7,c10, 4               @ Data Syncronization Barrier

	@ Restore Supervisor mode LR, SP and CPSR (that may have been corrupted by
	@ secure mode).

	LDR     R6, [SP, #SVCLR_OFFSET]
	LDR     R7, [SP, #SVCSP_OFFSET]
	LDR     R8, [SP, #SVCCPSR_OFFSET]
	MRS     R5, CPSR                    @ Save CPSR temporarily to R0
	CPS     #0x13                       @ Change to Supervisor mode
	MOV     LR, R6                      @ Restore SVC LR, SP and CPSR
	MOV     SP, R7                      @ Ignore warning
	MSR     CPSR_cxsf, R8               @ Still Supervisor mode
	MSR     CPSR_cxsf, R5               @ Back to original mode


	@************************************************************
	@ Let's check first if this is external code start exit, as
	@ in this case the handling is a bit different and we can not
	@ execute the same common exit part as with other exits
	@************************************************************
	@ REGS: R0,R1,R2,R3,R4,R11,R12-R14
	CMP     R12, #SEC_EXIT_START_EXT_CODE   @ Start ext code?
	BNE     r002

	@ This was indeed external code start exit. Let's restore most of normal mode
	@ settings and jump to external code (Data Cache and MMU won't be enabled).

	@ Drain write buffer.
	@ REGS: R0,R1,R2,R3,R11,R12
	MOV     R5, #0
	MCR     p15, 0, R5, c7,c10, 4

	@ Load physical address of label below (after restoring CP15 Control register
	@ this address will be changed to virtual and put to PC)
	@ REGS: R0,R1,R2,R3,R11,R12
	LDR     R7, =r0002

	.align 5

	@ Let's disable MMU and DCache here (as ext code jump address is physical)
	@ but enable ICache
	@ REGS: R0,R1,R2,R3,R11,R12
	BIC     R11, R11, #(HW_ARM_CP15_CR_M_MASK  |  HW_ARM_CP15_CR_C_MASK)
	ORR     R11, R11, #(HW_ARM_CP15_CR_I_MASK)
	MCR     p15, 0, R11, c1, c0, 0
	BX      R7                          @ Jump from virtual address in here
	NOP
	NOP
	NOP
r0002:                NOP                               @ .. to physical address in here

        @ Interrupts will also be kept disabled, so we do not restore original CPSR.

        @ We also can not restore saved values from stack as those might be in
        @ virtual areas and we do not have MMU enabled in this case

        @ We do not need to release reserved stack area at all, called routine
        @ will be responsible in setting stack pointer to correct value.

        @ Jump to ext code start address in LR.
        @ REGS: R0,R1,R2,R3,R12
        BX      R4



        @*************************************************************
        @ We have some other exit than external code start exit. First
        @ we do restoring of common normal mode settings, then check
        @ the exit case.
        @*************************************************************
        @ REGS: R0,R1,R2,R3,R4,R11,R12-R14
r002:

	@ Restore CP15 Control register from R11. Only ICache enabling should
	@ happen here, no other changes
	@ REGS: R0,R1,R2,R3,R4,R7,R11,R12-R13
	MCR     p15, 0, R11, c1, c0, 0

	@ L2 control can be done only in secure mode, so we do not even try it here

	@ Restore original CPSR value from stack to R5. CPSR register update
	@ will be done later
	@ REGS: R0,R1,R2,R3,R4,R12-R13
	LDR     R5, [SP, #CPSR_OFFSET]      @ Restore original CPSR


	@************************************************
	@ Let's check if this is normal secure mode exit
	@************************************************
	@ REGS: R0,R1,R2,R3,R4,R5,R11,R12-R14

	CMP     R12, #SEC_EXIT_NORMAL       @ Normal exit?
	BNE     r003

	@ This was indeed normal secure mode exit. Let's restore rest of the
	@ normal mode settings and return to function that called secure
	@ mode routine.

	@ Release reserved stack area.
	@ REGS: R5,R13
	ADD     SP, SP, #STACK_INT_USAGE

	@ Restore CPSR value from R5 (interrupts will typically be enabled here).
	@ REGS: R5,R13
	MSR     CPSR_cxsf, R5

	@ Restore registers and return to calling routine.
	@ REGS: R13

	LDMIA   SP!, {R8-R11}
	LDMIA   SP!, {R3,R4-R7,LR}      @ R3 to adjust 8 byte alignment
	BX      LR


	@***************************************************
	@ Let's check if this is exit for interrupt handling
	@***************************************************
	@ REGS: R0,R1,R2,R3,R4,R5,R11,R12-R13
r003:
	CMP     R12, #SEC_EXIT_INT          @ Exit for interrupt handling?
	BNE     r004

	@ This was indeed exit for interrupt handling. Let's restore rest of the
	@ normal mode settings and execute some NOP's during which interrupt should
	@ take place.
	@ After returning back from interrupt handler, normal mode settings are
	@ saved again and secure mode is entered.

	@ We do not need to release stack space here the same way as with normal exit.

	@ Restore CPSR value from R5 (interrupts will typically be enabled here).
	@ REGS: R5,R12-R13
	MSR     CPSR_cxsf, R5

	@ Interrupt should happen somewhere within the following NOP area.
	@ REGS: R10,R12-R13
	NOP
	NOP
	NOP
	NOP

	@ Interrupt has been handled, let's start going back to secure mode. As
	@ context shall be the same as before interrupt happened, there is no
	@ need re-save all context related register values.

	@ Set secure entry ID to R4 for use within secure mode.
	@ REGS: R12-R13
	LDR     R4, =SSAPI_RET_FROM_INT_SERV

	@ Data @ ap pointer and @ ap hasn't changed, so we do not need to clean those
	@ areas here.

	@ Jump to common secure mode entry settings part.
	@ REGS: R4,R12-R13
	B       sec_entry_settings



	@***************************************************************
	@ Let's check if this is exit for remote procedure call handling
	@***************************************************************
	@ REGS: R0,R1,R2,R3,R4,R11,R12-R13
r004:
	CMP     R12, #SEC_EXIT_RPC_CALL     @ RPC handler exit?
	BNE     r005

	@ This was indeed exit for remote procedure call handling. Let's restore
	@ normal mode settings and call remote procedure handler.
	@ After returning back from RPC handler, normal mode settings are
	@ saved again and secure mode is entered.
	@ Exceptional case is where R1 has value 0. This case is reserved for
	@ secure environment flags update. RPC handler is not called in this case
	@ and the flags setting is done here.

	@ Following code is the case where RPC handler call is requested. As
	@ there may be up to 4 parameters, R0-R3 may not be changed before
	@ RPC handler call. RPC may return one parameter, so R0 may not be
	@ changed after RPC call.

	@ We do not need to release stack space here the same way as with normal exit.

	@ Restore CPSR value from R5 (interrupts will typically be enabled here).
	@ REGS: R0,R1,R2,R3,R5,R12-R13
	MSR     CPSR_cxsf, R5

	@ Call RPC handler
	BLX     R4

	@ RPC call has been handled, let's start going back to secure mode. As
	@ context shall be the same as before RPC function was called, there is no
	@ need re-save all context related register values.

	@ Set secure entry ID to R4 for use within secure mode.
	@ REGS: R12-R13
	LDR     R4, =SSAPI_RET_FROM_RPC_SERV

	@ Save RPC function ret value to R6 for use within secure mode.
	@ REGS: R0,R4,R12-R13
	MOV     R6, R0

	@ Data @ ap pointer and @ ap hasn't changed, so we do not need to clean those
	@ areas here.

	@ Jump to common secure mode entry settings part.
	@ REGS: R4,R5,R12-R13
	B       sec_entry_settings


	@***************************************************************
	@ In case of unknown exit ID, we just loop
	@***************************************************************

r005:                  B       r005                         @ Unknown exit, just loop

	.endfunc
	.ltorg



/* Following code implements the secure mode entry settings: stack area
 * DCache clean, write buffer drain, Icache disabling & related operations.
 * These are mostly required in order to not cause security violation in
 * hardware supervised secure mode entry.
 * Register contents when entering this code:
 * R0 - (used internally within this code)
 * R1 - (used internally within this code)
 * R2 - (used internally within this code)
 * R3 - (used internally within this code)
 * R4 Function name pointer (original R0)
 * R5 - (Secure mode flags)
 * R6 ap in case of normal entry (original R1 converted to physical address),
 *    return value of called RPC function when returning from RPC call,
 *    no meaning when returning from interrupt handling
 * R7 -
 * R8 -
 * R9 -
 * R10 -
 * R11 -
 * R12 -
 * R13 Stack pointer (virtual)
 * R14 -
 */

sec_entry_settings:         .func

	@ Read secure mode flags from stack to R5 for later use.
	LDR     R5, [SP, #FLAGS_OFFSET]

	@ Read CP15 control register (r1) value to R11 for later use.
	MRC     p15, 0, R11, c1, c0, 0

	@ Disable interrupts from CPSR
	MRS     R7, CPSR                    @ Save CPSR temporarily to R7
	CPSID   if

	@ Save Supervisor mode LR, SP and CPSR to stack for the duration of
	@ secure mode as secure mode corrupts those registers.

	MRS     R0, CPSR                    @ Save CPSR temporarily to R0
	CPS     #0x13                       @ Change to Supervisor mode
	MOV     R1, LR                      @ Save SVC LR, SP and CPSR to stack
	MOV     R2, SP
	MRS     R3, CPSR
	MSR     CPSR_cxsf, R0               @ Back to original mode
	STR     R1, [SP, #SVCLR_OFFSET]
	STR     R2, [SP, #SVCSP_OFFSET]
	STR     R3, [SP, #SVCCPSR_OFFSET]

	@ DCache needs to be cleaned only if DCache was enabled prior to calling
	@ hw_sec_rom_pub_bridge. Otherwise it's caller's responsibility to do the
	@ cleaning (secure mode does not know whether cleaning should be done or not).
	TST     R11, #HW_ARM_CP15_CR_C_MASK
	BEQ     ses40

	@ Clean DCache @ stack area to make sure values written to stack above
	@ are available in physical addresses (four cache lines to ensure that
	@ at least 96 bytes of stack area will be cleaned). Actually this should
	@ not be needed with TrustZone but is here just in case someone makes a
	@ bug in public side for example with page coloring (robustness issue).
	MOV     R1, SP
	BIC     R1, R1, #0x1F               @ Clean cache @ stack area
	ADD     R2, R1, #0x60               @ End address (totally four cache lines)
s394:
	MCR     p15, 0, R1, c7, c14, 1      @ Clean DCache single entry (MVA)
	ADD     R1, R1, #(1<<5)             @ Next cache line
	CMP     R2, R1
	BPL     s394

ses40:

	@ With ARM1176 we do not need to clean the whole DCache as DCache will
	@ be enabled in secure mode in all cases (stack and parameter area were
	@ cleaned earlier to make it easier to use this entry routine).

	@ Invalidate TLB unlocked entries (should not be needed but done anyhow)
	MOV     R0, #0
	MCR     p15, 0, R0, c8, c7, 1

	@ Drain write buffer to make sure L2 and physical memory are correctly updated
	@        MOV     R0, #0
	MCR     p15, 0, R0, c7,c10, 4

	@ ICache will be disabled here for the duration of the entry (to
	@ prevent secure state machine to generate secure violation at entry)
	@ As secure mode has its own CP15 r1 control register, we do not need to
	@ modify other bits here.
	@ R11 shall contain CP15 r1 contents prior to executing this part.
	BIC     R2, R11,  #HW_ARM_CP15_CR_I_MASK
	MCR     p15, 0, R2, c1, c0, 0

	NOP         @ NOP's are needed here to make sure SMI has not been
	NOP         @ fetched to prefetch buffer
	NOP
	NOP
	NOP
	NOP
	NOP

	@ L2 cache disabling is not needed (and actually cannot be done in non-secure
	@ mode). Also, it is not necessary to check here that L2 is Cleaned or
	@ cleaned and invalidated as TrustZone takes care that those operations
	@ cannot be used for secure mode hacking purposes.

	@ We do not need to disable interrupts here (from ASIC) as TrustZone takes
	@ care of proper interrupt enabling

	@ Data Memory Barrier is set here so that all explicit memory transactions
	@ occuring in program order before this instruction are completed before going
	@ to secure mode entry (would not be needed with TrustZone but is done here
	@ just to be sure everything is OK when entering secure mode).

	MOV     R0, #0
	MCR     p15, 0, R0, c7,c10, 4               @ Data Syncronization Barrier
	MCR     p15, 0, R0, c7, c5, 4               @ Flush Prefetch Buffer
	MCR     p15, 0, R0, c7, c5, 6               @ Flush BTAC

	MRC     p15, 0, R0, c1, c0, 0

	SMC    #0                                    @ Call secure application

	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	B      ret_from_sec_mode

	.endfunc
	.ltorg

/*
 * Use mmu table to find physical address of virtual address in R0
 */

hw_mmu_pa_get:          .func
	PUSH {R1, R2}
	MOV R1, R0
	ldr R2, var_val
	AND R1, R1, R2
	MCR p15, 0, R0, c7, c8, 0
	MRC p15, 0, R0, c7, c4, 0
	BIC R0, R0, R2
	ORR R0, R0, R1
	POP {R1, R2}
	BX LR
	.endfunc


var_val:
	.word	0x0FFF
	.ltorg

