/**********************************************************************
* PRELIMINARY INFORMATION                                             *
*                                                                     *
* Code sequences are incomplete, not fully validated and supplied for *
* illustrative purposes only at this time. The code templates will be *
* reviewed and updated when validated code for representative cores   *
* is available.                                                       *
*                                                                     *
***********************************************************************
* Copyright (C) 2007-2008 ARM Limited
*
* This software is provided 'as-is', without any express or implied
* warranties including the implied warranties of satisfactory quality,
* fitness for purpose or non infringement.  In no event will  ARM be
* liable for any damages arising from the use of this software.
*
* Permission is granted to anyone to use, copy and modify this software for
* any purpose, and to redistribute the software, subject to the following
* restrictions:
*
* 1. The origin of this software must not be misrepresented* you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
** SVN Information
** Checked In          : $Date: 2009-06-10 07:27:28 +0000 (Wed, 10 Jun 2009) $
** Revision            : $Revision: 8862 $
** Release Information :
**
** **************************************************************************
**                                                                         **
** Definitions associated with Dormant mode save and restore support code. **
** ======================================================================= **
**                                                                         **
** **************************************************************************/
#ifndef __DM_PWR_SAVE_DFN_H__
#define __DM_PWR_SAVE_DFN_H__

#include <mach/io_map.h>
#include <mach/memory.h>

/* Aliases for mode encodings - do not change */
#define MODE_USR	0x10
#define MODE_FIQ	0x11
#define MODE_IRQ	0x12
#define MODE_SVC	0x13
#define MODE_ABT	0x17
#define MODE_UND	0x1B
#define MODE_SYS	0x1F
#define MODE_MON	0x16  /* A-profile (Security Extensions) only*/


/***************************************************************************
 Entry point offsets for key context variable and pointer values

* Might include values associated with IMPLEMENTATION DEFINED registers.

* Additional values can be appended or offset order re-assigned for
* logical grouping reasons

* Maintain correlation with the offset comments/size on reserved space
* at the start of DORMANT_BASE.
**************************************************************************/
#define DM_ENDIAN	0x04   /* flag for CPSR.E bit status*/
#define DM_ACTLR	0x08   /* CP15 register content*/
#define DM_SCTLR	0x0c   /* CP15 register content*/
#define DM_CPACR	0x10   /* CP15 register content */

/* IMPLEMENTATION DEFINED: additional key context variables*/

#define DM_bankedARM		0x14   /* ptr to ARM register save area */
#define DM_DBG				0x18   /* ptr to debug save area */
#define DM_PMon				0x1C   /* ptr to Performance Monitor (PMU) save area */
#define DM_VFP				0x20   /* ptr to VFP save area */
#define DM_MemMgr			0x24   /* ptr to MMU/MPU save area as applicable */
#define DM_SCU				0x28   /* ptr to SCU save area */
#define DM_GIC				0x2C   /* ptr to GIC save area */
#define DM_TIMERS			0x30   /* ptr to timers save area */
#define DM_SYSCACHE			0x34   /* ptr to system cache save area */
#define DM_ContextBase_VA	0x38   /* ptr to context save area */
#define DM_CA9_SCU_VA		0x3C   /* ptr to CA9 SCU base addr */
#define DM_CA9_SCU_PA		0x40   /* PA of SCU ptr */
#define DM_returnVA			0x44   /* VA for the PA => VA transition */


/* IMPLEMENTATION DEFINED: additional key context pointers
                         eg. TCM save area*/
#define RESET_FLAG_PTR		0x4C   /* Pointer to save reset reason*/
#define BASE_PTRS			0x50   /* Pointers to base addresses for 4 cp's*/
#define DM_BLOCK			( BASE_PTRS + 4*4 + 4) /* value = largest defined offset + 4 */


/* L1 CACHE_OP PARAMETERS - 4 WAY, 32KB, 32B CACHELINE */

#define L1_NWAYS_STRIDE		0x40000000     /* bit<30> set */
#define L1_NSETS_LIMIT		0x00002000     /* bit<13> set */
#define L1_NSETS_STRIDE		0x00000020     /* bit<5>  set */

/* PL310 PARAMETERS*/

#define PL310_InvByPA		0x70  /* Invalidate by PA cacheop offset */
#define PL310_CleanByPA		0xB0  /* Clean by PA cacheop offset */
#define PL310_LINELEN		0x20  /* cache line length */

/** **************************************************************************
**                                                                         **
**                                                                         **
** General Interupt Controller (GIC) - register offsets                    **
** ====================================================                    **
** ICD register offsets with respect to the GIC Distributor base address   **
** ICC register offsets with respect to the GIC CPU Interface base address **
**                                                                         **
** **************************************************************************/

#define ICC_ICR			0x0   /* ICC control (banked in Security Extns) */
#define ICC_PMR			0x4   /* interrupt priority mask  */
#define ICC_BPR			0x8   /* binary point (banked in Security Extns)  */
#define ICC_ABPR		0x1C  /* aliased bianry point (Security Extns) */

#define ICD_DCR			0x0   /* ICD control (banked in Security Extns)  */
#define ICD_ICTR		0x4   /* type information (RO) */
#define ICD_ISR			0x80  /* interrupt security registers */
#define ICD_ISER		0x100 /* set-enable registers */
#define ICD_IPR			0x400 /* priority registers */
#define ICD_IPTR		0x800 /* processor target registers */
#define ICD_ICFR		0xC00 /* interrupt configuration registers */


/** **************************************************************************
**                                                                         **
**                                                                         **
** IMPLEMENTATION DEFINED - definitions relating to Cortex-A9              **
** ==========================================================              **
**                                                                         **
** **************************************************************************/

#define CA9_SCU_ICD		0x1000       /* GIC Distributor offset from SCU_BASE */
#define CA9_SCU_ICC		0x100        /* GIC CPU Interface offset from SCU_BASE */
#define CA9_SCU_TIM64	0x200        /* (64-bit) timer block offset from SCU_BASE */
#define CA9_SCU_TIMER	0x600        /* Timer block offset from SCU_BASE */

    /* SCU_BASE offsets*/
#define SCU_Ctl			0x0   /* control */
#define SCU_Config		0x4   /* configuration*/
#define SCU_PwrStatus	0x8   /* power status */
#define SCU_InvAll		0xC   /* Invalidate TAG RAM (per processor, Sec Extns aware) */
#define SCU_FiltStart	0x40  /* master port 1 start address (address filtering ON) */
#define SCU_FiltEnd		0x44  /* master port 1 end address (address filtering ON) */
#define SCU_SAC			0x50  /* SCU access control */
#define SCU_SSAC		0x54  /* Non-Secure SCU Access control */

#define TIMER_Ld	0x0   /* timer load */
#define TIMER_Cnt	0x4   /* timer counter */
#define TIMER_Ctl	0x8   /* timer control*/
#define Timer_Int	0xC   /* timer interrupt status*/

#define WDOG_Ld		0x20  /* watchdog timer load */
#define WDOG_Cnt	0x24  /* watchdog timer counter */
#define WDOG_Ctl	0x28  /* watchdog timer control */
#define WDOG_In		0x2C  /* watchdog timer interrupt status */

#define WDOG_RstStatus	0x30  /* watchdog reset status */
#define WDOG_Disable	0x34  /* watchdog disable (WO)  */

#define TIM64_CntLo		0x0    /* timer counter - lo word */
#define TIM64_CntHi		0x4    /* timer counter - hi word */
#define TIM64_Ctl		0x8    /* 64-bit timer control */
#define TIM64_Status	0xC    /* 64-bit timer status */
#define TIM64_CmpLo		0x10   /* timer comparator - lo word */
#define TIM64_CmpHi		0x14   /* timer comparator - hi word */
#define TIM64_AutoInc	0x18   /* timer comparator auto_inc */

/** **************************************************************************
**                                                                         **
**                                                                         **
** IMPLEMENTATION DEFINED - system specific definitions                    **
** ====================================================                    **
**                                                                         **
** **************************************************************************/

#define DORMANT_BASE		dormant_base_va /* start of context save area - PA or VA */
#define DORMANT_BASE_PA		dormant_base_pa /* start of context save (PA used on reset) */
#define DORMANT_BASE_CPU_OFFSET	0x1000 /* 	used with the CPUID for MP systems to
											a cpu-specific context base address
											(DORMANT_BASE + (CPUn x DORMANT_BASE_CPU_OFFSET)) */

#define CA9_SCU_BASE	KONA_SCU_VA  /* the default Versatile-EB PERIPHBASE[31:13]
									  value for Cortex-A9 (PA or VA*)

									   VAs are used where the dormant code is
									  entered and returned from a translated
									  address space (not direct-mapped).

									  In the VA case, it is likely a different
									  mechanism will be used as a means of sourcing
									  the address arguments for DORMANT_SAVE in a
									  real application.  */

/* ASSUMPTION: PL310 at a +ve offset from CA9_SCU_BASE */
#define CA9_SCU_L2CC 0x20000  /* the default RTSM-EB value for PL310 registers*/

/* DBG_BASE  EQU 0xYYYYYYYY  * memory mapped debug base */

#define CA9_TIMER		(CA9_SCU_BASE + CA9_SCU_TIMER)
#define CA9_TIM64		(CA9_SCU_BASE + CA9_SCU_TIM64)

/* Define to 0 to not touch L2C related stuff.  We use secure API's */
#define L2CC_BASE       0

#define GIC_ICD_BASE	(CA9_SCU_BASE + CA9_SCU_ICD)
#define GIC_ICC_BASE	(CA9_SCU_BASE + CA9_SCU_ICC)

#define TEMP_STACK		0x05000000

#define WARM_START_FLAG		0x1111
#define POWER_CONTROLLER_CONTEXT_BASE_PA    0x3404BF80
#define POWER_CONTROLLER_CONTEXT_BASE_VA    \
		HW_IO_PHYS_TO_VIRT(POWER_CONTROLLER_CONTEXT_BASE_PA)

#endif /*__DM_PWR_SAVE_DFN_H__*/

