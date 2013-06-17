/********************************************************************************

	Copyright (C) 2007-2009 ARM Limited

	This software is provided 'as-is', without any express or implied
	warranties including the implied warranties of satisfactory quality,
	fitness for purpose or non infringement.  In no event will  ARM be
	liable for any damages arising from the use of this software.

	Permission is granted to anyone to use, copy and modify this software for
	any purpose, and to redistribute the software, subject to the following
	restrictions:

	1. The origin of this software must not be misrepresented* you must not
	claim that you wrote the original software. If you use this software
	in a product, an acknowledgment in the product documentation would be
	appreciated but is not required.
	2. Altered source versions must be plainly marked as such, and must not be
	misrepresented as being the original software.
	3. This notice may not be removed or altered from any source distribution.

	SVN Information
	Checked In          : $Date: 2009-05-07 14:20:15 +0100 (Thu, 07 May 2009) $
	Revision            : $Revision: 8622 $
	Release Information :

********************************************************************************
 performance monitor counters

 Macro List:
             PMon_SAVE
             PMon_RESTORE

********************************************************************************

********************************************************************************
 DORMANT_PMon_SAVE macro

********************************************************************************/

	.macro PMon_SAVE
		@Save a pointer to the start of the PMon context offset
		STR	r0,[r11, #DM_PMon] @ save at fixed address

		/* Save Performance Monitor Registers
		 ASSUMPTION: These should be saved as early as possible to make them
		  'transparent' to power mangement overheads */

		/* Ignore
		        Count Enable Clear Register
		        Software Increment Register
		        Interrupt Enable Clear Register	*/

		MRC	p15,0,r8,c9,c12,0   @ PMon: Control Register
		BIC	r1,r8,#1
		MCR	p15,0,r1,c9,c12,0   @ disable counter updates from here
		ISB                       @ 0b0 => PMCR<0>
		MRC	p15,0,r9,c9,c12,3   @ PMon: Overflow Flag Status Reg
		MRC	p15,0,r10,c9,c12,5  @ PMon: Event Counter Selection Reg
		STM	r0!, {r8-r10}
		UBFX	r9,r8,#11,#5        @ extract # of event counters, N
		TST	r9, r9
		BEQ	1000f
1001:
		SUBS r9,r9,#1            @ decrement N
		MCR	p15,0,r9,c9,c12,5   @ PMon: select CounterN
		ISB
		MRC	p15,0,r3,c9,c13,1   @ PMon: save Event Type register
		MRC	p15,0,r4,c9,c13,2   @ PMon: save Event Counter register
		STM	r0!, {r3,r4}
		BNE	1001b
1000:
		MRC	p15,0,r1,c9,c13,0   @ PMon: Cycle Count Register
		MRC	p15,0,r2,c9,c14,0   @ PMon: User Enable Register
		MRC	p15,0,r3,c9,c14,1   @ PMon: Interrupt Enable Set Reg
		MRC	p15,0,r4,c9,c12,1   @ PMon: Count Enable Set Register
		STM	r0!, {r1-r4}

	.endm


/*******************************************************************************
 DORMANT_PMon_RESTORE macro

********************************************************************************/
	.macro	PMon_RESTORE

		@	NOTE: all counters disabled by PMCR<0> == 0 on reset

		@	Restore performance counters
		LDR	r0,[r11,#DM_PMon]    @ load the PMon saved context pointer
		LDM	r0!,{r8-r10}        @ recover first block of PMon context
                                      @ (PMCR, PMOVSR, PMSELR)
		MOV	r1, #0              @ generate register of all 0's
		MVN	r2, #0              @ generate register of all 1's
		MCR	p15,0,r2,c9,c14,2 @ disable all counter related interrupts
		MCR	p15,0,r2,c9,c12,3   @ clear all overflow flags
		ISB

		UBFX	r12,r8,#11,#5 @ extract # of event counters, N (0-31)
		TST	r12, r12
		BEQ	1001f
		MOV	r3, r12             @ for N >0, generate a 2nd copy of N
		MOV	r4, #1
		LSL	r4, r4, r3
		SUB	r4, r4, #1          @ set bits<N-1:0> to all 1's
1000:
		SUBS	r3,r3,#1            @ decrement N
		MCR	p15,0,r3,c9,c12,5   @ select Event CounterN
		ISB
		MRC	p15,0,r5,c9,c13,1   @ read Event Type register
		BFC	r5,#0,#8
		MCR	p15,0,r5,c9,c13,1   @ set Event Type to 0x0
		MCR	p15,0,r2,c9,c13,2   @ set Event Counter to all 1's
		ISB
		BNE	1000b

		MOV	r3, #1
		BIC r5, r9, #1<<31
		MCR	p15,0,r5,c9,c12,1   @ enable Event Counters
		                        @ (PMOVSR bits set)
		MCR	p15,0,r3,c9,c12,0   @ set the PMCR global enable bit
		ISB
		MCR	p15,0,r9,c9,c12,4   @ set event count overflow bits
		ISB
		MCR	p15,0,r4,c9,c12,2   @ disable Event Counters

      @ restore the event counters
1004:
		SUBS	r12,r12,#1          @ decrement N
		MCR	p15,0,r12,c9,c12,5  @ select Event CounterN
		ISB
		LDM	r0!,{r3-r4}
		MCR	p15,0,r3,c9,c13,1   @ restore Event Type
		MCR	p15,0,r4,c9,c13,2   @ restore Event Counter
		ISB
		BNE	1004b
1001:
		TST	r9, #0x80000000 @ check for cycle count overflow flag
		BEQ	1003f
		MCR	p15,0,r2,c9,c13,0   @ set Cycle Counter to all 1's
		ISB
		MOV	r3, #0x80000000
		MCR	p15,0,r3,c9,c12,1   @ enable the Cycle Counter
		ISB
1002:
		MRC	p15,0,r4,c9,c12,3   @ check cycle count overflow now set
		MOVS	r4,r4               @ test bit<31>
		BPL	1002b
		MCR	p15,0,r3,c9,c12,2   @ disable the Cycle Counter
1003:
		MCR	p15,0,r1,c9,c12,0   @ clear the PMCR global enable bit
		ISB
      @ restore the remaining PMon registers
		LDM	r0!,{r1-r4}
		MCR	p15,0,r1,c9,c13,0   @ restore Cycle Count Register
		MCR	p15,0,r2,c9,c14,0   @ restore User Enable Register
		MCR	p15,0,r3,c9,c14,1   @ restore Interrupt Enable Set
		                        @ Register
		MCR	p15,0,r4,c9,c12,1   @ restore Count Enable Set Register

		MCR	p15,0,r10,c9,c12,5  @ restore Event Counter Selection
		ISB
		MCR	p15,0,r8,c9,c12,0   @ restore the PM Control Register
		ISB

	.endm
