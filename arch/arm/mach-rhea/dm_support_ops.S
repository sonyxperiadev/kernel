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
* Checked In          : $Date: 2009-05-05 18:11:31 +0100 (Tue, 05 May 2009) $
* Revision            : $Revision: 8575 $
* Release Information :
*
*******************************************************************************
 Cache maintenance macros

 Macro List:
             L1_DCCSW     Clean all L1 Dcache by set/way loop
             L1_DCISW     Invalidate all L1 Dcache by set/way loop
             L1_DCCISW    Clean + Invalidate all L1 Dcache by set/way loop
             L1_DCxxSW    Common code for the data cache operations
             VA_TO_PA     Translate a VA to a PA using the MMU hardware.
                          (A-profile only)

*******************************************************************************/


		/************************************
		*          L1_DCxxSW macro         *
		*                                  *
		************************************/
	.macro L1_DCxxSW, type

		MOV   r1, #0

		MCR   p15,2,r1,c0,c0,0	@ clear the CSSELR (select L1 Dcache)
		ISB
		MRC   p15,1,r7,c0,c0,0	@ read CCSIDR for L1 cache parameters

		@ r7<27:13> NSETS bit field: # of sets = NSETS+1
		@ r7<12:3>  ASSOC bit field: # of ways = ASSOC+1
		@ r7<2:0>   LineSize field: line length = 2**(LineSize+4) bytes

		UBFX  r3, r7, #0, #3	@ extract the CCISDR.LineSize bit field
		ADD   r3, r3, #4
		UBFX  r4, r7, #13, #15	@ extract the CCISDR.NSETS bit field
		UBFX  r5, r7, #3, #10	@ extract the CCISDR.ASSOC bit field

		CMP      r5,#3			@ Cortex-A9 optimisation (known size)
		CMPEQ    r3,#5
		BNE      1000f
		LSL      r1,r4,#5
		ADD      r2,r1,#0x40000000
		ADD      r3,r1,#0x80000000
		ADD      r4,r1,#0xc0000000
1001:
		MCR      p15,0,r1,c7,\type
		MCR      p15,0,r2,c7,\type
		MCR      p15,0,r3,c7,\type
		MCR      p15,0,r4,c7,\type
		SUBS     r1,r1,#0x20
		SUB      r2,r2,#0x20
		SUB      r3,r3,#0x20
		SUB      r4,r4,#0x20
		BPL      1001b
		B        1002f
1000:
		CLZ	r6, r5      @ r6 is the bit position of the way size
			            @ increment
1003:
		MOV	r2,r5		@ r2 working copy of the max way size
						@ (right aligned)
1004:
		LSL	r7,r2,r6	@ factor the way number into r7
						@ r7<3:1> == '000' => L1 cache
		LSL r1,r4,r3    @ determine the index bit field
		ORR	r7,r7,r1	@ factor in the index number
		MCR	p15,0,r7,c7,\type   @ DCxxSW cache operation
		SUBS	r2,r2,#1		@ decrement the way number
		BGE	1004b
		SUBS	r4,r4,#1		@ decrement the index
		BGE	1003b
1002:
		DSB

	.endm

@@@ L1 data cache invalidate all by set/way
	.macro L1_DCISW
		L1_DCxxSW "c6,2"
	.endm


@@@ L1 data cache clean all by set/way
	.macro L1_DCCSW
		L1_DCxxSW "c10,2"
	.endm

@@@ L1 data cache clean + invalidate all by set/way
	.macro L1_DCCISW
		L1_DCxxSW "c14,2"
	.endm


		/************************************
		*          VA_TO_PA macro          *
		*                                  *
		************************************/

/*******************************************************************************
 VA_TO_PA: Translate a VA to a PA using the MMU hardware. A-profile only.

 INPUT:
   $reg1 = virtual address

 OUTPUT:
   $reg1 = physical address (== virtual address if MMU off)

 USES:
   $reg1, $reg2, $reg3

*******************************************************************************/

    .macro VA_TO_PA reg1, reg2, reg3
1000:
    .if (V7_MMU_PRESENT)

        MRC     p15, 0, \reg2, c1, c0, 0    @ $reg2 = SCTLR
        TST     \reg2, #1                   @ MMU on?
        BEQ     10002f 						@ MMU off: skip translation
        MOV     \reg3, #0x1000              @ $reg3 = page size = 0x1000
        SUB     \reg3, \reg3, #1            @ $reg3 = page offset mask = 0x0FFF
        BIC     \reg2, \reg1, \reg3         @ $reg2 = virtual page
        MCR     p15, 0, \reg2, c7, c8, 1    @ VA to PA Translation Register = $reg2
        ISB                                 @ barrier ensures PAR correctness
        MRC     p15, 0, \reg2, c7, c4, 0    @ $reg2 = PA Register
        TST     \reg2, #1                   @ Translation success?
        BNE     .                           @ Failed -- not good
        BIC     \reg2, \reg2, \reg3         @ $reg2 = physical page
        AND     \reg1, \reg1, \reg3         @ $reg1 = offset within page
        ORR     \reg1, \reg1, \reg2         @ $reg1 = PA
10002:
    .endif  @ endif defined(V7_MMU_PRESENT)
	.endm


