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
 floating point and Advanced SIMD support

 Macro List:
             VFP_SAVE
             VFP_RESTORE

*******************************************************************************


*******************************************************************************
 DORMANT_VFP_SAVE macro

*******************************************************************************/
	.macro VFP_SAVE

	@ Save a pointer to the start of the VFP context offset
		STR	r0,[r11,#DM_VFP]   @ save at fixed address

	@ FPU state save/restore.
	@ FPSID,MVFR0 and MVFR1 don't get serialized/saved (Read Only).
		MRC	p15,0,r9,c1,c0,2    @ CPACR allows CP10 and CP11 access
		ORR	r2,r9,#0xF00000
		MCR	p15,0,r2,c1,c0,2
		ISB
		MRC	p15,0,r2,c1,c0,2
		AND	r2,r2,#0xF00000
		CMP	r2,#0xF00000
		BEQ	1000f
		MOVS	r2, #0
	@ Override to 0 to indicate that no FPU is present
		STR	r2,[r11,#DM_VFP]
		B	1002f

1000:
	@	Save configuration registers and enable.
		FMRX 	r10,FPEXC

		STR	r10,[r0],#4        @ Save the FPEXC
	@ Enable FPU access to save/restore the other registers.
		LDR	r2,=0x40000000
		FMXR	FPEXC,r2
		FMRX 	r2,FPSCR
		STR	r2,[r0],#4         @ Save the FPSCR
	@ Store the VFP-D16 registers.
		VSTM	r0!, {D0-D15}
	@ Check for Advanced SIMD/VFP-D32 support
		FMRX 	r2,MVFR0
		AND	r2,r2,#0xF         @ extract the A_SIMD bitfield
		CMP	r2, #0x2
		BLT	1002f
	@ Store the Advanced SIMD/VFP-D32 additional registers.
		VSTM	r0!, {D16-D31}

	/* IMPLEMENTATION DEFINED: save any subarchitecture defined state
	 NOTE: Don't change the order of the FPEXC and CPACR restores*/

		FMXR	FPEXC,r10         @ Restore the original En bit of FPU.
1002:
		MCR	p15,0,r9,c1,c0,2 @ Restore the original CPACR value.

	.endm

/*******************************************************************************
 DORMANT_VFP_RESTORE macro

*********************************************************************************/

	.macro VFP_RESTORE

		LDR	r0,[r11,#DM_VFP]  @ load the VFP saved context pointer
		CMP	r0,#0
		BEQ	1000f

	@ FPU state save/restore. Obviously FPSID,MVFR0 and MVFR1 don't get
	@ serialized (RO).
	@ Modify CPACR to allow CP10 and CP11 access
		MRC	p15,0,r2,c1,c0,2
		ORR	r2,r2,#0x00F00000
		MCR	p15,0,r2,c1,c0,2
	@ Enable FPU access to save/restore the rest of registers.
		LDR	r2,=0x40000000
		FMXR	FPEXC, r2
	@ Recover FPEXC and FPSCR. These will be restored later.
		LDM	r0!,{r9,r10}
	@ Restore the VFP-D16 registers.
		VLDM	r0!, {D0-D15}
      @ Check for Advanced SIMD/VFP-D32 support
		FMRX 	r2, MVFR0
		AND	r2,r2,#0xF         @ extract the A_SIMD bitfield
		CMP	r2, #0x2
		BLT	1001f

      @ Store the Advanced SIMD/VFP-D32 additional registers.
		VLDM	r0!, {D16-D31}

      @ IMPLEMENTATION DEFINED: restore any subarchitecture defined state

1001:
	@ Restore configuration registers and enable.
	@ Restore FPSCR _before_ FPEXC since FPEXC could disable FPU
	@ and make setting FPSCR unpredictable.
		FMXR	   FPSCR,r10
		FMXR	   FPEXC,r9	@ Restore FPEXC after FPSCR
	@ CPACR (c1,c0,2) is restored later.
1000:

	.endm
