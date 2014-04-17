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
	PL310 memory mapped system cache

	Macro List:
				PL310_SYSCACHE_SAVE
				PL310_SYSCACHE_RESTORE

********************************************************************************


********************************************************************************
* DORMANT_SYSCACHE_SAVE macro
*
********************************************************************************/
	.macro PL310_SYSCACHE_SAVE

	/* Save system cache information
	 NOTE: PL310 example, ALLOWED BUT NOT PART OF THE ARMv7 ARCHITECTURE
           PL310 SECURE-ONLY SAVE/RESTORE - few registers are NS accessible
	 Assumption:
	 This is the only execution thread interacting with the system cache.
	 Interrupts should be disabled so we don't expect any context switches. */

	 @ save system cache context base address => DORMANT_BASE
    .if (DIRECT_MAPPED_SAVE == 0)
		MOV	r4, r0
dm_pl310:
		VA_TO_PA r4,r5,r6
	@ WARNING - ASSUMES SYSCACHE RESTORED BEFORE PA => VA TRANSITION
		STR	r4,[r11, #DM_SYSCACHE] @ save PA ptr at fixed address
    .else
		STR	r0,[r11, #DM_SYSCACHE] @ save ptr at fixed address
	.endif

	@ base address for register 2  (Now r1 points to BASE+0x200)
		LDR	r1, [r11, #DM_CA9_SCU_VA]
@		MOVW	r2, #(CA9_SCU_L2CC + 0x200)
		LDR	r2, =0x20200
		ADD	r1,r1,r2

		LDR r2, [r1, #0x00]
		LDR r3, [r1, #0x04]
		LDR r4, [r1, #0x08]
		LDR r5, [r1, #0x0C]
		LDR r6, [r1, #0x10]
		LDR r7, [r1, #0x14]

	@ save event counter and interrupt mask context
		STM	r0!,{r2-r7}
		ADD	r1,r1,#0x700

	@ base address for register 9  (Now r1 points to BASE+0x900)
	@ NOTE: If no lockdown used, or limited support you can
	@       comment out as appropriate (and adjust r1 value!)
		LDR r2, [r1], #4
		LDR r3, [r1], #4
		LDR r4, [r1], #4
		LDR r5, [r1], #4
		STM	r0!,{r2-r5}      @ save lockdown[1:0] by way context
		LDR r2, [r1], #4
		LDR r3, [r1], #4
		LDR r4, [r1], #4
		LDR r5, [r1], #4
		STM	r0!,{r2-r5}      @ save lockdown[3:2] by way context
		LDR r2, [r1], #4
		LDR r3, [r1], #4
		LDR r4, [r1], #4
		LDR r5, [r1], #4
		STM	r0!,{r2-r5}      @ save lockdown[5:4] by way context
		LDR r2, [r1], #4
		LDR r3, [r1], #4
		LDR r4, [r1], #4
		LDR r5, [r1], #4
		STM	r0!,{r2-r5}      @ save lockdown[7:6] by way context
		ADD	r1,r1,#0x10

	/* base address for 'by line' part of register 9  (Now r1 points
	 to BASE+0x950) */

		LDR r2, [r1, #0x00]
		LDR r3, [r1, #0x04]
		STM	r0!,{r2-r3}      @ save lock by line control context
		ADD	r1,r1,#0x2B0

	@ base address for register 12  (Now r1 points to BASE+0xC00)
		LDR r2, [r1, #0x00]
		LDR r3, [r1, #0x04]
		STM	r0!,{r2-r3}      @ save address filtering context

	@ load from register 15 address space (Read BASE+0xF40)
		LDR	r2,[r1, #0x340]
		STR	r2,[r0], #4      @ save debug control context
		SUB	r1,r1,#0xB00

	@ base address for register 1 (Now r1 points to BASE+0x100)
	@ NOTE: from r1p0, r0p0 does _not_ support 2x RAM latency registers
		LDR r2, [r1, #0x00]
		LDR r3, [r1, #0x04]
		LDR r4, [r1, #0x08]
		LDR r5, [r1, #0x0C]
		STM	r0!,{r2-r5}      @ save L2 cache control context

	@ Check for any active background operations
1000:
	@ register 7 address space, offset from register 1 (Read BASE+0x7FC)
		LDR	r2,[r1,#0x6FC]
		CMP	r2,#0
		BNE	1000b
1001:
	@ register 9 address space, offset from register 1 (Read BASE+0x954)
		LDR	r2,[r1,#0x854]
		CMP	r2,#0
		BNE	1001b

	.endm


/*******************************************************************************
 DORMANT_SYSCACHE_RESTORE macro

*******************************************************************************/
    @       PL310 SECURE-ONLY SAVE/RESTORE - few registers are NS accessible

	.macro PL310_SYSCACHE_RESTORE

		LDR	r0, [r11, #DM_SYSCACHE] @ recover PL310 context base address
		CMP	r0,#0
		BEQ	1000f   @ no PL310 context available to restore

	@ Restore the system cache
	@ base address for register 2 (point r1 to BASE+0x100)
	@ turn off L2 cache (it should be already turned off).
		LDR	r1, [r11, #DM_CA9_SCU_PA]
@		MOVW	r2, #(CA9_SCU_L2CC + 0X100)
		LDR	r2, =0x20100
		ADD	r1,r1,r2
		MOVS	r2, #0
1001:
		LDR	r3,[r1]
		AND	r3,r3,#1
		CMP	r3,#0
		BEQ	1002f
		STR	r2,[r1]
	@ L2 cache should be off after this store
		DSB
		B	1001b

1002:
	@ load the system cache saved context pointer
		LDR	r0,[r11,#DM_SYSCACHE]

	@ recover context from memory
		LDM	r0!,{r2-r7}
		ADD	r1,r1,#0x100	@ (point r1 to BASE+0x200)
	@ restore event counter and interrupt mask context
		STR r2, [r1, #0x00]
		STR r3, [r1, #0x04]
		STR r4, [r1, #0x08]
		STR r5, [r1, #0x0C]
		STR r6, [r1, #0x10]
		STR r7, [r1, #0x14]

	/* base address for register 9 (point r1 to BASE+0x900)
		NOTE: If no lockdown used, or limited support you can
		       comment out as appropriate (and adjust r1 value!). */
		ADD	r1,r1,#0x700
		LDM	r0!,{r2-r5}	@ recover context from memory
		@ restore lockdown[1:0] by way context
		STR r2, [r1], #4
		STR r3, [r1], #4
		STR r4, [r1], #4
		STR r5, [r1], #4
		LDM	r0!,{r2-r5}	@ recover context from memory
		@ restore lockdown[3:2] by way context
		STR r2, [r1], #4
		STR r3, [r1], #4
		STR r4, [r1], #4
		STR r5, [r1], #4
		LDM	r0!,{r2-r5}	@ recover context from memory
		@ restore lockdown[5:4] by way context
		STR r2, [r1], #4
		STR r3, [r1], #4
		STR r4, [r1], #4
		STR r5, [r1], #4
		LDM	r0!,{r2-r5}	@ recover context from memory
		@ restore lockdown[7:6] by way context
		STR r2, [r1], #4
		STR r3, [r1], #4
		STR r4, [r1], #4
		STR r5, [r1], #4

	@ base address for 'by line' part of register 9 (point r1 to BASE+0x950)
		ADD	r1,r1,#0x10
		LDM	r0!,{r2-r3} @ recover context from memory
		@ restore lock by line control context
		STR r2, [r1, #0x00]
		STR r3, [r1, #0x04]

	@ base address for register 12 (point r1 to BASE+0xC00)
		ADD	r1,r1,#0x2B0
		LDM	r0!,{r2-r3} @ recover context from memory
		@ restore address filtering context
		STR r2, [r1, #0x00]
		STR r3, [r1, #0x04]

		LDR	r2,[r0],#4  @ recover debug control context
	@ restore to register 15 address space  (Write BASE+0xF40)
		STR	r2,[r1,#0x340]

	@ base address for register 1 (point r1 to BASE+0x100)
	@ NOTE: from r1p0, r0p0 does _not_ support 2x RAM latency registers
		SUB	r1,r1,#0xB00
		LDM	r0!,{r2-r5} @ recover context from memory
		STR	r3,[r1,#4]  @ restore L2 auxiliary control context
		STR	r4,[r1,#8]  @ restore L2 Tag RAM latency control
		STR	r5,[r1,#12] @ restore L2 Data RAM latency control
		STR	r2,[r1]     @ restore L2 control context last
                        @ L2 enabled if bit<0> of restored
                        @ control context is set.
		DSB
1000:
	.endm



