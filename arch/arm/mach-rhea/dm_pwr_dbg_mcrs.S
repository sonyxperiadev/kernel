/********************************************************************************
***********************************************************************
* PRELIMINARY INFORMATION                                             *
*                                                                     *
* Code sequences are incomplete, not fully validated and supplied for *
* illustrative purposes only at this time. The code templates will be *
* reviewed and updated when validated code for representative cores   *
* is available.                                                       *
*                                                                     *
***********************************************************************
 Copyright (C) 2009 ARM Limited                           

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
 DebugOverPowerDown code

        AREA    debugOverPowerDown, CODE, READONLY

 - if the target is known to use memory-mapped debug, compile once with "NoCP14" set to "{TRUE}"

 - if the target is known to use CP14, compile once with "NoMemoryMapped" set to "{TRUE}"

 If the target does not implement halting debug over power-down features, this code
 will not save/restore any state associated with an external debugger in particular
 this includes the state of the Debug Comms Channel.

 The type of OS lock mechanism can also be fixed:

 - for v6/v7 Debug:

   - if the OS Save and Restore mechanism is implemented, set "OSSRR" to "{TRUE}"

   - if the OS Save and Restore mechanism is not implemented, set "NoOSLock" to "{TRUE}"

 Setting one of these variables to "{FALSE}" generates code that checks the ID registers to
 test for the feature -- unless one of the other variables being set to "{TRUE}" makes that
 test unnecessary.

 To fix the number of breakpoints or watchpoints, set "CheckNums" to "{FALSE}" and define:

 - "NumBreakpoints" for the number of breakpoints

 - "NumWatchpoints" for the number of watchpoints

 Otherwise the code checks the ID registers for the right code sequence to use.
 This is not needed if "OSSRR" is defined.

 Settings for ARM processors:

 Field             ARM11  Cortex-A8   Cortex-A9r0   Cortex-A9r1
 ==============================================================
 OSSRR             -      TRUE        -             -
 NoOSLock          TRUE   -           TRUE          TRUE
 NoCP14            -      TRUE        -             -
 NoMemoryMapped    TRUE   -           TRUE          TRUE
 NumBreakpoints    6      6           6             6
 NumWatchpoints    6      2           2             4
 CheckNums         FALSE  FALSE       FALSE         FALSE

********************************************************************************/

@@ Macro for ISBs.
	.macro OrderingISB
#if defined(CONFIG_CPU_V7)
        ISB
#endif
    .endm

    .macro FullISB
#if defined(CONFIG_CPU_V7)
        ISB
#else
        MCR     p15,0,R0,c7,c5,4        @ CP15ISB operation
#endif
    .endm

@@ Macros for accessing debug registers. These either do a CP14 access or
@@ a memory-mapped access, depending on whether "MemoryMapped" is TRUE
    
	.macro MRD , reg, crn, crm, opc2, Memory
     
	.if (\Memory)
	
		LDR  \reg,[r12, #(\opc2<<6) | (\crm<<2) | (\crn<<9)]
    .else
		MRC  p14,0,\reg,c\crn,c\crm,\opc2
    .endif
    .endm

    .macro MDR, reg, crn, crm, opc2, Memory

    .if (\Memory)
		STR  \reg,[R12,#(\opc2<<6) |(\crm<<2)|(\crn<<9)]
    .else
		MCR p14,0,\reg,c\crn,c\crm,\opc2
    .endif
    .endm

	@@ Macro to return debug register physical base address
    .macro GetDebugRegisterBase, Rd, InvalidExit
        /* Returns the PHYSICAL ADDRESS of the debug registers in R12
         Note: memory must be flat mapped for the debug registers */
		MRC     p14,0,\Rd,c1,c0,0        @ Read DBGDRAR
        TST     \Rd,#3                   @ Check valid
        BEQ   \InvalidExit

        MRC     p14,0,R2,c2,c0,0         @ Read DBGDSAR
        TST     R2,#3                    @ Check valid
        BEQ   \InvalidExit

        LSR     \Rd,\Rd,#12              @ Ignore the UNK bits
        ADD     \Rd,\Rd,R2,LSR #12
        LSL     \Rd,\Rd,#12
    .endm


@@	Macros for pushing and popping onto the stack
@@ If the code is not defined to be PCS compliant, these are NOPs
     .macro PCSPUSH, rlist
	.ifdef	PCS_compliant
        PUSH \rlist
    .endif  
    .endm

    .macro PCSPOP, rlist
    .ifdef	PCS_compliant
        POP     \rlist
    .endif
    .endm


@@ Macros for checking if the OS Lock and/or OSSRR is implemented
    .macro OSSRRCheck , ExitOSSRR, Mem
		MRD     R1,1,1,4,\Mem           @ Read DBGOSLSR
        TST     R1,#1<<0                @ Check DBGOSLSR[0] for DBGOSSRR support
        BNE   	\ExitOSSRR
    .endm 

@@ Macro to get the number of breakpoints, etc. into R1 (if needed)
    .macro GetNumBreakpoints, CheckNums
    .if (\CheckNums)
        LSR     R1,R4,#24
        AND     R1,R1,#0xF              @ Extract BRPs field -- there will be at least 2
    .endif
    .endm

    .macro GetNumCTXCmps, CheckNums
    .if (\CheckNums)
        LSR     R3,R4,#20               @ R4 = DBGDIDR
        AND     R3,R3,#0xF              @ Extract CTX_CMPs field from DBGDIDR
        SUB     R3,R1,R3                @ R1 = number of breakpoints
    .endif
    .endm

    .macro GetNumWatchpoints, CheckNums
	.if (\CheckNums)
        LSR     R1,R4,#28               @  Get WRPs field
    .endif
    .endm

@@ SaveDebugRegisters

        @@ Macros for saving off a "register pair"
    .macro SaveRPnoCheck, num, opc2_vr, opc2_cr, Mem
		MRD     R2,0,\num,\opc2_vr,\Mem 	@  Read DBGxVRn
        MRD     R3,0,\num,\opc2_cr,\Mem 	@ Read DBGxCrm
        STMIA   R0!,{R2-R3}             	@ Save { xVRn,xCrm }
    .endm

    .macro SaveRP, num, opc2_vr, opc2_cr, exit, Mem

		MRD     R2,0,\num,\opc2_vr,\Mem @ Read DBGxVRn
        MRD     R3,0,\num,\opc2_cr,\Mem @ Read DBGxCrm
        CMP     R1,#\num
        STMIA   R0!,{R2-R3}             @ Save { xVRn,xCrm }
        BEQ     \exit
    .endm

    .macro SaveBRP, num, Mem, CheckNums, NumBreakpoints 
    .if (\CheckNums == 0)
        .if(\num < \NumBreakpoints)
			SaveRPnoCheck \num, 4, 5, \Mem
        .endif
    .else
        .if ((\num == 15) || (\num == 0))
			SaveRPnoCheck \num, 4, 5, \Mem
        .else 
			SaveRP  \num, 4, 5, 25f, \Mem
        .endif
	.endif
    .endm

	
    .macro SaveWRP, num, Mem, CheckNums, NumWatchpoints
    
    .if(\CheckNums == 0)
        .if(\num < \NumWatchpoints)
			SaveRPnoCheck \num, 6, 7, \Mem
        .endif
    .else
        .if (\num == 15)
			SaveRPnoCheck \num, 6, 7, \Mem
        .else
			SaveRP  \num, 6, 7, 40f, \Mem
        .endif
    .endif
    .endm


    .macro SaveDebugRegisters_internal, OSSRR, NoOSLock, CheckNums, NumBreakpoints, NumWatchpoints, Mem, Exit 

    .if((\OSSRR) && (\NoOSLock))
		.err
	.endif

    .set   CheckForOSSRR, ((\NoOSLock == 0) && (\OSSRR == 0))
@@ Decide which pieces of code to include

    .set IncludeNoOSLock, ((\NoOSLock)  || (CheckForOSSRR))
	.set IncludeOSSRR,	((CheckForOSSRR) || (\OSSRR))	

    .if(\Mem)
        GetDebugRegisterBase R12, \Exit
    .endif

    .if(CheckForOSSRR)
        OSSRRCheck  80f, \Mem
    .endif

    .if(IncludeNoOSLock)
        @@ If there is no OS Lock mechanism we can still save/restore debug registers
        @@ other than the comms channel registers
10:      @ (3a) Debug Status and Control Register
		MRD     R1,0,1,0,\Mem           @ Read DBGDSCRint
        STR     R1,[R0],#4              @ Store { DSCR }

        @ (3b) Other miscellaneous registers
        MRD     R1,0,6,0,\Mem           @ Read DBGWFAR
        MRD     R2,0,7,0,\Mem           @ Read DBGVCR
        MRD     R3,7,9,6,\Mem           @ Read CLAIM -- note, have to read via DBGCLAIMCLR
        STMIA   R0!,{R1-R3}             @ Save { WFAR,VCR,CLAIM }

        @ (3c) Breakpoints
        GetNumBreakpoints \CheckNums

        SaveBRP 0, \Mem, \CheckNums, \NumBreakpoints
        SaveBRP 1, \Mem, \CheckNums, \NumBreakpoints
        SaveBRP 2, \Mem, \CheckNums, \NumBreakpoints
        SaveBRP 3, \Mem, \CheckNums, \NumBreakpoints
        SaveBRP 4, \Mem, \CheckNums, \NumBreakpoints
        SaveBRP 5, \Mem, \CheckNums, \NumBreakpoints
        SaveBRP 6, \Mem, \CheckNums, \NumBreakpoints
        SaveBRP 7, \Mem, \CheckNums, \NumBreakpoints
        SaveBRP 8, \Mem, \CheckNums, \NumBreakpoints
        SaveBRP 9, \Mem, \CheckNums, \NumBreakpoints
        SaveBRP 10, \Mem, \CheckNums, \NumBreakpoints
        SaveBRP 11, \Mem, \CheckNums, \NumBreakpoints
        SaveBRP 12, \Mem, \CheckNums, \NumBreakpoints
        SaveBRP 13, \Mem, \CheckNums, \NumBreakpoints
        SaveBRP 14, \Mem, \CheckNums, \NumBreakpoints
        SaveBRP 15, \Mem, \CheckNums, \NumBreakpoints

25:      @ (3e) Watchpoints
        GetNumWatchpoints \CheckNums

        SaveWRP 0, \Mem, \CheckNums, \NumWatchpoints
        SaveWRP 1, \Mem, \CheckNums, \NumWatchpoints
        SaveWRP 2, \Mem, \CheckNums, \NumWatchpoints
        SaveWRP 3, \Mem, \CheckNums, \NumWatchpoints
        SaveWRP 4, \Mem, \CheckNums, \NumWatchpoints
        SaveWRP 5, \Mem, \CheckNums, \NumWatchpoints
        SaveWRP 6, \Mem, \CheckNums, \NumWatchpoints
        SaveWRP 7, \Mem, \CheckNums, \NumWatchpoints
        SaveWRP 8, \Mem, \CheckNums, \NumWatchpoints
        SaveWRP 9, \Mem, \CheckNums, \NumWatchpoints
        SaveWRP 10, \Mem, \CheckNums, \NumWatchpoints
        SaveWRP 11, \Mem, \CheckNums, \NumWatchpoints
        SaveWRP 12, \Mem, \CheckNums, \NumWatchpoints
        SaveWRP 13, \Mem, \CheckNums, \NumWatchpoints
        SaveWRP 14, \Mem, \CheckNums, \NumWatchpoints
        SaveWRP 15, \Mem, \CheckNums, \NumWatchpoints

40:
		.if(IncludeOSSRR)
			B       \Exit
        .endif
    .endif       @ IncludeNoOSLock

    .if (IncludeOSSRR)
        @@ If there is an OS Save and Restore register, use it to save out the
        @@ debug registers
80:      @ (2) Set OS Lock Access Register (DBGOSLAR).
        LDR   R2,=0xC5ACCE55
        MDR     R2,1,0,4,\Mem           @ Write DBGOSLAR
        OrderingISB                     @ Ordering barrier

         @ (3) Get the number of words to save.
        MRD     R1,1,2,4,\Mem           @ DBGOSSRR returns size
        STR     R1,[R0],#4              @ Push on to the save stack

        @ (4) Loop reading words from the DBGOSSRR.
        CMP     R1,#0 @ Check for zero
81
        ITTT    NE
        MRDNE   R2,1,2,4,\Mem           @ Load a word of data
        STRNE   R2,[R0],#4              @ Push on to the save stack
        SUBNES  R1,R1,#1
        BNE     %B81
    .endif       @ IncludeOSSRR

        @ (6) Return the pointer to first word not written to. This pointer is already in R0, so
        @ all that is needed is to return from this function.  Leave DBGOSLAR set, because from now
        @ on we do not want any changes.

	.endm
        @ SaveDebugRegisters_internal

@@@ RestoreDebugRegisters

        @@ Macros for restoring a "register pair"
    .macro RestoreRPnoCheck, num, opc2_vr, opc2_cr, Mem
		LDMIA   R0!,{R2-R3}             @ Read { xVRn,xCRn }
        MDR     R2,0,\num,\opc2_vr,\Mem @ Restore DBGxVRn
        MDR     R3,0,\num,\opc2_cr,\Mem @ Restore DBGxCRn
    .endm

    .macro RestoreRP , num, opc2_vr, opc2_cr, exit, Mem
		LDMIA   R0!,{R2-R3}             @ Read { xVRn,xCRn }
        CMP     R1,#\num
        MDR     R2,0,\num,\opc2_vr,\Mem @ Restore DBGxVRn
        MDR     R3,0,\num,\opc2_cr,\Mem @ Restore DBGxCRn
        BEQ     \exit
    .endm
        
    .macro RestoreBRP , num, Mem, CheckNums, NumBreakpoints

	.if(\CheckNums == 0)
        .if (\num < \NumBreakpoints)
			RestoreRPnoCheck \num, 4, 5, \Mem
        .endif
    .else
        .if ((\num == 0) || (\num == 15))
			RestoreRPnoCheck \num, 4, 5, \Mem
        .else
			RestoreRP  \num, 4, 5, 25f, \Mem
        .endif
    .endif
    .endm

    .macro RestoreWRP num, Mem, CheckNums, NumWatchpoints
    .if (\CheckNums == 0)
        .if( \num < \NumWatchpoints)
			RestoreRPnoCheck \num, 6, 7, \Mem
        .endif
    .else
        .if (num == 15)
			RestoreRPnoCheck \num, 6, 7, \Mem
        .else
			RestoreRP  \num, 6, 7, %F40, \Mem
        .endif
    .endif
	.endm

	.macro RestoreDebugRegisters_internal, OSSRR, NoOSLock, CheckNums, NumBreakpoints, NumWatchpoints, Mem, Exit

    .if((\OSSRR) && (\NoOSLock))
		.err
	.endif

    .set CheckForOSSRR,  ((\NoOSLock == 0) && (\OSSRR == 0))

@@ Decide which pieces of code to include
	.set IncludeNoOSLock, ((\NoOSLock) || (CheckForOSSRR))
	.set IncludeOSSRR, ((CheckForOSSRR) || (\OSSRR))

    .if(\Mem)
        GetDebugRegisterBase R12, \Exit
     .endif

    .if(CheckForOSSRR)
        OSSRRCheck  80, \Mem
    .endif

    .if(IncludeNoOSLock)
	
        @@ If the variant of the code that supports "No OS Lock" is being
        @@ included, then restore DSCR at the end, after restoring all other
        @@ registers.
        PCSPUSH {R5}
        LDR     R5,[R0],#4              @ Read { DSCR }

        LDMIA   R0!,{R1-R3}             @ Read { WFAR,VCR,CLAIM }
        MDR     R1,0,6,0,\Mem           @ Restore DBGWFAR
        MDR     R2,0,7,0,\Mem           @ Restore DBGVCR
        MDR     R3,7,8,6,\Mem           @ Restore CLAIM tags -- note: must write DBGCLAIMSET

        @ Breakpoints
        GetNumBreakpoints \CheckNums

        RestoreBRP 0, \Mem, \CheckNums, \NumBreakpoints
	    RestoreBRP 1, \Mem, \CheckNums, \NumBreakpoints
        RestoreBRP 2, \Mem, \CheckNums, \NumBreakpoints
        RestoreBRP 3, \Mem, \CheckNums, \NumBreakpoints
        RestoreBRP 4, \Mem, \CheckNums, \NumBreakpoints
        RestoreBRP 5, \Mem, \CheckNums, \NumBreakpoints
        RestoreBRP 6, \Mem, \CheckNums, \NumBreakpoints
        RestoreBRP 7, \Mem, \CheckNums, \NumBreakpoints
        RestoreBRP 8, \Mem, \CheckNums, \NumBreakpoints
        RestoreBRP 9, \Mem, \CheckNums, \NumBreakpoints
        RestoreBRP 10, \Mem, \CheckNums, \NumBreakpoints
        RestoreBRP 11, \Mem, \CheckNums, \NumBreakpoints
        RestoreBRP 12, \Mem, \CheckNums, \NumBreakpoints
        RestoreBRP 13, \Mem, \CheckNums, \NumBreakpoints
        RestoreBRP 14, \Mem, \CheckNums, \NumBreakpoints
        RestoreBRP 15, \Mem, \CheckNums, \NumBreakpoints

25:
        GetNumWatchpoints \CheckNums

        RestoreWRP 0, \Mem, \CheckNums, \NumWatchpoints
        RestoreWRP 1, \Mem, \CheckNums, \NumWatchpoints
        RestoreWRP 2, \Mem, \CheckNums, \NumWatchpoints
        RestoreWRP 3, \Mem, \CheckNums, \NumWatchpoints
        RestoreWRP 4, \Mem, \CheckNums, \NumWatchpoints
        RestoreWRP 5, \Mem, \CheckNums, \NumWatchpoints
        RestoreWRP 6, \Mem, \CheckNums, \NumWatchpoints
        RestoreWRP 7, \Mem, \CheckNums, \NumWatchpoints
        RestoreWRP 8, \Mem, \CheckNums, \NumWatchpoints
        RestoreWRP 9, \Mem, \CheckNums, \NumWatchpoints
        RestoreWRP 10, \Mem, \CheckNums, \NumWatchpoints
        RestoreWRP 11, \Mem, \CheckNums, \NumWatchpoints
        RestoreWRP 12, \Mem, \CheckNums, \NumWatchpoints
        RestoreWRP 13, \Mem, \CheckNums, \NumWatchpoints
        RestoreWRP 14, \Mem, \CheckNums, \NumWatchpoints
        RestoreWRP 15, \Mem, \CheckNums, \NumWatchpoints

40:      @ CheckClearLock

		.if (IncludeNoOSLock)
			OrderingISB
			MDR     R5,0,2,2,\Mem           @ R5 = DSCR value@ Restore DBGDSCR
			PCSPOP  {R5}
		.endif

		.if(IncludeOSSRR)
			B       \Exit
		.endif
	
    .endif       @ IncludeNoOSLock
        
    .if (IncludeOSSRR)
80:      @ (2) Set OS Lock Access Register (DBGOSLAR) and reset pointer. The lock
        @     will be set, but this write is needed to reset the pointer
        LDR   R1,=0xC5ACCE55
        MDR     R1,1,0,4,\Mem           @ Write DBGOSLAR
        OrderingISB
        
        @ (3) Clear the Sticky Power-down Status bit (if implemented).
        MRD     R2,1,5,4,\Mem           @ Dummy read of DBGPRSR to clear StickyPD
        OrderingISB

        @ (4) Get the number of words saved.
        MRD     R1,1,2,4,\Mem                @ Dummy read of DBGOSSRR
        LDR     R1,[R0],#4              @ Load size from the save stack
        
        @ (5) Loop writing words from the DBGOSSRR.
        CMP     R1,#0                   @ Check for zero
81:      @ OSSRRLoop
        ITTT    NE
        LDRNE   R2,[R0],#4              @ Load a word from the save stack
        MDRNE   R2,1,2,4,\Mem           @ Store a word of data
        SUBNES  R1,R1,#1
        BNE     81b

        OrderingISB
        @ Use any non-lock value to clear the lock. We know R1 <= 15, so use R1
        MDR     R1,1,0,4,\Mem           @ Write DBGOSLAR to clear the lock
    .endif

	.endm
    @ RestoreDebugRegisters_internal


@@@ Top level macros

    .macro SaveDebugRegisters, OSSRR, NoOSLock, CheckNums, NumBreakpoints, NumWatchpoints, NoMemoryMapped, NoCP14

	.set CheckForMemoryMapped, ((\NoCP14 == 0) && (\NoMemoryMapped==0))

@@ If any of these conditions are {TRUE}, the code has to read DBGDIDR
	.set ReadDIDR,  ((CheckForMemoryMapped) || (\CheckNums))	

	.if(ReadDIDR)
        PCSPUSH {R4}
        MRC     p14,0,R4,c0,c0,0        @ Read DBGDIDR
    .endif

    .if(CheckForMemoryMapped)
        LSR     R1,R4,#16
        AND     R1,R1,#0xF              @ Extract DBGDIDR.Version
        CMP     R1,#0x4
        BEQ   50f
    .endif

    .if(\NoCP14 == 0)
        SaveDebugRegisters_internal \OSSRR, \NoOSLock, \CheckNums, \NumBreakpoints, \NumWatchpoints, 0, 99f
		
     .endif

	.if (CheckForMemoryMapped)
        B       99f
	.endif
50:
    .if((CheckForMemoryMapped) || (\NoMemoryMapped == 0))
        SaveDebugRegisters_internal \OSSRR, \NoOSLock, \CheckNums, \NumBreakpoints, \NumWatchpoints, 1, 99f
    .endif

99:
   .if(ReadDIDR)
        PCSPOP  {R4}
    .endif

    .endm      @ SaveDebugRegisters


@@@ RestoreDebugRegisters


    .macro RestoreDebugRegisters, OSSRR, NoOSLock, CheckNums, NumBreakpoints, NumWatchpoints, NoMemoryMapped, NoCP14

	.set CheckForMemoryMapped, ((\NoCP14 == 0) && (\NoMemoryMapped == 0))
@@ If any of these conditions are {TRUE}, the code has to read DBGDIDR

	.set ReadDIDR,	((CheckForMemoryMapped) || (\CheckNums))
	
	.if(ReadDIDR)
        PCSPUSH {R4}
        MRC     p14,0,R4,c0,c0,0        @ Read DBGDIDR
    .endif
 
    .if(CheckForMemoryMapped)
        LSR     R1,R4,#16
        AND     R1,R1,#0xF              @ Extract DBGDIDR.Version
        CMP     R1,#0x4
        BEQ   50f
    .endif
 
    .if (\NoCP14 == 0)
        RestoreDebugRegisters_internal \OSSRR, \NoOSLock, \CheckNums, \NumBreakpoints, \NumWatchpoints, 0, 99f
    .endif

    .if(CheckForMemoryMapped)
        B 99f
	.endif
50:


    .if((CheckForMemoryMapped) || (\NoMemoryMapped == 0))
        RestoreDebugRegisters_internal \OSSRR, \NoOSLock, \CheckNums, \NumBreakpoints, \NumWatchpoints, 1, 99f
    .endif

99:  
      @ (7) A final ISB guarantees the restored register values are visible to subsequent
        @ instructions. This needs to be a full ISB.
        FullISB

    .if(ReadDIDR)
        PCSPOP  {R4}
    .endif

    .endm   @ RestoreDebugRegisters

    @ Macro to trace dormant flow
    @ ===========================
    @ Args:
    @ trace - value to be written to trace location
    @ scratc1/scratch2 - scratch regs that will be modified
    @                    after a call to this macro.
    @ Assumes:
    @ dormant_trace_p - contains physical address of trace location
    @ dormant_trace_v - contains virtual address of trace location

    .extern dormant_trace_p
    .extern dormant_trace_v
    .macro dormant_tracer_p trace scratch1 scratch2
    .if (DORMANT_TRACE_ENABLE == 1)
        ldr    \scratch1,=dormant_trace_p
        ldr    \scratch2,=CONFIG_PAGE_OFFSET
        sub    \scratch1,\scratch1,\scratch2
        ldr    \scratch2,=PHYS_OFFSET
        add    \scratch1,\scratch1,\scratch2
        ldr    \scratch1,[\scratch1]
        mov    \scratch2,#\trace
        str    \scratch2,[\scratch1]
    .endif
    .endm
    .macro dormant_tracer_v trace scratch1 scratch2
    .if (DORMANT_TRACE_ENABLE == 1)
        ldr    \scratch1,=dormant_trace_v
        ldr    \scratch1,[\scratch1]
        mov    \scratch2,#\trace
        str    \scratch2,[\scratch1]
    .endif
    .endm
