/****************************************************************************
*
*	Copyright (c) 2004-2010 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*   Notwithstanding the above, under no circumstances may you combine this
*   software in any way with any other Broadcom software provided under a
*   license other than the GPL, without Broadcom's express prior written
*   consent.
*
****************************************************************************/

/*==========================================================================
*
* file   chal_bmintc_inc.h
* brief  OS independent code of BModem INTC using inline function.
* note
*==========================================================================*/
#ifndef _CHAL_BMINTC_INC_H_
#define _CHAL_BMINTC_INC_H_
#include <plat/chal/chal_types.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_bintc.h>

#define BMINTC_WRITE32(regOffset, data)  \
			CHAL_REG_WRITE32(((cUInt32) h) + (regOffset), data)
#define BMINTC_READ32(regOffset) \
			CHAL_REG_READ32(((cUInt32) h) + (regOffset))

typedef struct {
	cUInt32 mask[2];
} chal_bmintc_mask_t;

typedef enum {
	BINTC_OUT_DEST_0 = 0,
	BINTC_OUT_DEST_1 = 1,
	BINTC_OUT_DEST_2 = 2,
	BINTC_OUT_DEST_3 = 3,
	BINTC_OUT_DEST_4 = 4,
	BINTC_OUT_DEST_5 = 5,
	BINTC_OUT_DEST_6 = 6,
	BINTC_OUT_DEST_7 = 7,
	BINTC_OUT_DEST_8 = 8,
	BINTC_OUT_DEST_9 = 9,
	BINTC_OUT_DEST_10 = 10,
	BINTC_OUT_DEST_11 = 11,
	BINTC_OUT_DEST_12 = 12,
	BINTC_OUT_DEST_13 = 13,
	BINTC_OUT_DEST_14 = 13,
	BINTC_OUT_DEST_15 = 15,
	BINTC_OUT_DEST_16 = 16,
	BINTC_OUT_DEST_17 = 17,
	BINTC_OUT_DEST_18 = 18,
	BINTC_OUT_DEST_19 = 19,
	BINTC_OUT_DEST_20 = 20,
	BINTC_OUT_DEST_21 = 21,
	BINTC_OUT_DEST_22 = 22,
	BINTC_OUT_DEST_23 = 23,
	BINTC_OUT_DEST_24 = 24,
	BINTC_OUT_DEST_25 = 25,
	BINTC_OUT_DEST_26 = 26,
	BINTC_OUT_DEST_27 = 27,
	BINTC_OUT_DEST_28 = 28,
	BINTC_OUT_DEST_29 = 29,
	BINTC_OUT_DEST_30 = 30,
	BINTC_OUT_DEST_31 = 31
} BINTC_OUT_DEST_en;

#define BINTC_OUT_DEST_IPC          BINTC_OUT_DEST_7
#define BINTC_OUT_DEST_PWRMGR0      BINTC_OUT_DEST_8
#define BINTC_OUT_DEST_PWRMGR1      BINTC_OUT_DEST_9
#define BINTC_OUT_DEST_PWRMGR2      BINTC_OUT_DEST_10
#define BINTC_OUT_DEST_WCDMA_USB    BINTC_OUT_DEST_12
#define BINTC_OUT_DEST_CP           BINTC_OUT_DEST_13
/*#define BINTC_OUT_DEST_CP_FIQ  BINTC_OUT_DEST_14: do not use, program ISEL register instead */
#define BINTC_OUT_DEST_AP2DSP       BINTC_OUT_DEST_15
#define BINTC_OUT_DEST_CP2DSP       BINTC_OUT_DEST_16
#define BINTC_OUT_DEST_DSP_NORM     BINTC_OUT_DEST_17
#define BINTC_OUT_DEST_DSP_ERR      BINTC_OUT_DEST_18

/**
 * distance between BINTC output registers
 * note that #14 should correspond to CP FIQ but corresponding registers
 * don't exist instead, one has to program ISEL registers
 * (via functions chal_bmintc_set_intr_to_irq/fiq)
 * to select between IRQ/FIQ
 **/
#define BMREG_BLOCK_SIZE (BINTC_IMR0_1_OFFSET-BINTC_IMR0_0_OFFSET)

/*==============================================================================
*
* Function Name: void chal_bmintc_clear_interrupt(CHAL_HANDLE h, cUInt32 Id)
*
* Description:   Clear an interrupt
*
* param:         h: cHal handle.
*
* param:         Id: Interrupt Id
*
*
* Notes:
*============================================================================*/
__forceinline static void chal_bmintc_clear_interrupt(CHAL_HANDLE h, cUInt32 Id)
{

	if (Id >= 64)
		return;

	if (Id >= 32) {
		BMINTC_WRITE32(BINTC_ICR1_OFFSET, 1 << (Id - 32));
	} else {
		BMINTC_WRITE32(BINTC_ICR0_OFFSET, 1 << Id);
	}
}

/*==============================================================================
*
* Function Name: chal_bmintc_clear_all(CHAL_HANDLE h, chal_bmintc_mask_t *mask)
*
* Description:   Clear all interrupts except those in the mask
*
* param:         h: cHal handle.
*
* param:         maks: pointer to intc mask
*
*
* Notes:
*=============================================================================*/
__forceinline static void chal_bmintc_clear_intr_mask(CHAL_HANDLE h,
						chal_bmintc_mask_t *mask)
{
	BMINTC_WRITE32(BINTC_ICR0_OFFSET, mask->mask[0]);
	BMINTC_WRITE32(BINTC_ICR1_OFFSET, mask->mask[1]);
}

/*==============================================================================
*
* Function Name: chal_bmintc_enable_interrupt(CHAL_HANDLE h,
*                                             cUInt32 destId,cUInt32 Id)
*
* Description:   enable an interrupt
*
* param:         h: cHal handle.
*
* param:         Id: Interrupt Id
*
* param:         destId: output destination of an interrupt
*
* Notes:
*=============================================================================*/
__forceinline static void chal_bmintc_enable_interrupt(CHAL_HANDLE h,
						       cUInt32 destId,
						       cUInt32 Id)
{

	if (Id >= 64)
		return;

	if (Id >= 32) {		/* Id < 64 */
		BMINTC_WRITE32(BINTC_IMR1_0_SET_OFFSET +
			       destId * BMREG_BLOCK_SIZE, (1 << (Id - 32)));
	} else {
		BMINTC_WRITE32(BINTC_IMR0_0_SET_OFFSET +
			       destId * BMREG_BLOCK_SIZE, (1 << Id));

	}
}

__forceinline static void chal_bmintc_enable_DSP_interrupt(CHAL_HANDLE h,
							   cUInt32 destId,
							   cUInt32 Id)
{
	BMINTC_WRITE32(destId, (1 << Id));

}


/*==============================================================================
*
* Function Name: chal_bmintc_disable_interrupt(CHAL_HANDLE h,
*                                              cUInt32 destId,cUInt32 Id)
*
* Description:   disable an interrupt
*
* param:         h: cHal handle.
*
* param:         Id: Interrupt Id
*
* param:         destId: output destination of an interrupt
*
* Notes:
*=============================================================================*/
__forceinline static void chal_bmintc_disable_interrupt(CHAL_HANDLE h,
							cUInt32 destId,
							cUInt32 Id)
{
	if (Id >= 64)
		return;

	if (Id >= 32) {		/* Id < 64 */
		BMINTC_WRITE32(BINTC_IMR1_0_CLR_OFFSET +
			       destId * BMREG_BLOCK_SIZE, (1 << (Id - 32)));
	} else {
		BMINTC_WRITE32(BINTC_IMR0_0_CLR_OFFSET +
			       destId * BMREG_BLOCK_SIZE, (1 << Id));

	}
}

/*==============================================================================
*
* Function Name: chal_bmintc_disable_interrupts(CHAL_HANDLE h,
*                                             cUInt32 destId,
*                                             chal_bmintc_mask_t *mask)
*
* Description:   disable all interrupts in the mask
*
* param:         h: cHal handle.
*
* param:         maks: pointer to intc mask
*
* param:         destId: output destination of an interrupt
*
* Notes:
*=============================================================================*/
__forceinline static void chal_bmintc_disable_intr_mask(CHAL_HANDLE h,
						cUInt32 destId,
						chal_bmintc_mask_t *mask)
{

	BMINTC_WRITE32(BINTC_IMR1_0_CLR_OFFSET + destId * BMREG_BLOCK_SIZE,
		       mask->mask[1]);

	BMINTC_WRITE32(BINTC_IMR0_0_CLR_OFFSET + destId * BMREG_BLOCK_SIZE,
		       mask->mask[0]);

}


/*==============================================================================
*
* Function Name: chal_bmintc_restore_interrupts(CHAL_HANDLE h,
*                                             cUInt32 destId,
*                                             chal_bmintc_mask_t *mask)
*
* Description:   restore all interrupts based on the mask
*
* param:         h: cHal handle.
*
* param:         maks: pointer to intc mask
*
* param:         destId: output destination of an interrupt
*
* Notes:
*=============================================================================*/
__forceinline static void chal_bmintc_restore_interrupts(CHAL_HANDLE h,
						 cUInt32 destId,
						 chal_bmintc_mask_t *mask)
{
    /*
     * disable all set interrupt so only those set in mask are set when
     * we return from the function
     */
	BMINTC_WRITE32(BINTC_IMR0_0_CLR_OFFSET + destId * BMREG_BLOCK_SIZE,
		       0xffffffff);
	BMINTC_WRITE32(BINTC_IMR1_0_CLR_OFFSET + destId * BMREG_BLOCK_SIZE,
		       0xffffffff);

    /* each destination has 7 32bit registers */
	BMINTC_WRITE32(BINTC_IMR1_0_SET_OFFSET + destId * BMREG_BLOCK_SIZE,
		       mask->mask[1]);

	BMINTC_WRITE32(BINTC_IMR0_0_SET_OFFSET + destId * BMREG_BLOCK_SIZE,
		       mask->mask[0]);

}

/*==============================================================================
*
* Function Name: chal_bmintc_get_interrupt_mask(CHAL_HANDLE h,
*                                             cUInt32 destId,
*                                             chal_bmintc_mask_t* mask)
*
* Description:   Get interrupt mask
*
* param:         h: cHal handle.
*
* param:         destId: output destination of an interrupt
*
* param:         mask (out): intec mask
*
* Notes:
*=============================================================================*/
__forceinline static void chal_bmintc_get_interrupt_mask(CHAL_HANDLE h,
						cUInt32 destId,
						chal_bmintc_mask_t *mask)
{

    /* each destination has 7 32bit registers */
	mask->mask[0] =
	    BMINTC_READ32(BINTC_IMR0_0_OFFSET + destId * BMREG_BLOCK_SIZE);
	mask->mask[1] =
	    BMINTC_READ32(BINTC_IMR1_0_OFFSET + destId * BMREG_BLOCK_SIZE);

	return;
}



/*==============================================================================
*
* Function Name: Boolean chal_bmintc_is_interrupt_enabled(CHAL_HANDLE h,
*                                                         cUInt32 destId,
*                                                         cUInt32 Id)
*
* Description:   Is an interrupt enabled?
*
* param:         h: cHal handle.
*
* param:         Id: Interrupt Id
*
* param:         destId: output destination of an interrupt
*
* return:        TRUE or FALSE
*
* Notes:
*=============================================================================*/
__forceinline static Boolean chal_bmintc_is_interrupt_enabled(CHAL_HANDLE h,
							      cUInt32 destId,
							      cUInt32 Id)
{

	if (Id >= 64)
		return 0;

	if (Id >= 32) {		/* Id < 64 */
		/* each destination has 7 32bit registers */
		return ((BMINTC_READ32
			 (BINTC_IMR1_0_OFFSET +
			  destId * BMREG_BLOCK_SIZE) & (1 << (Id - 32))) != 0);
	} else {
		return ((BMINTC_READ32
			 (BINTC_IMR0_0_OFFSET +
			  destId * BMREG_BLOCK_SIZE) & (1 << Id)) != 0);
	}

}

/*==============================================================================
*
* Function Name: chal_bmintc_get_status(CHAL_HANDLE h,
*                                       cUInt32 destId,
*                                       chal_bmintc_mask_t *mask)
*
* Description:   Get interrupt status
*
* param:         h: cHal handle.
*
* param:         maks: pointer to intc mask
*
* param:         destId: output destination of an interrupt
*
* Notes:
*=============================================================================*/
__forceinline static void chal_bmintc_get_status(CHAL_HANDLE h,
						cUInt32 destId,
						chal_bmintc_mask_t *status)
{

	status->mask[0] =
	    BMINTC_READ32(BINTC_IMSR0_0_OFFSET + destId * BMREG_BLOCK_SIZE);
	status->mask[1] =
	    BMINTC_READ32(BINTC_IMSR1_0_OFFSET + destId * BMREG_BLOCK_SIZE);

}

/*==============================================================================
*
* Function Name: chal_bmintc_get_status(CHAL_HANDLE h,
*                                       cUInt32 destId,
*                                       chal_bmintc_mask_t *mask)
*
* Description:   Get interrupt status
*
* param:         h: cHal handle.
*
* param:         maks: pointer to intc mask
*
* param:         destId: output destination of an interrupt
*
* Notes:
*=============================================================================*/
__forceinline static UInt32 chal_bmintc_get_per_register_status(CHAL_HANDLE h,
							cUInt32 destId,
							cUInt32 reg_offset)
{

	if (reg_offset == 0) {
		return (BMINTC_READ32
			(BINTC_IMSR0_0_OFFSET + destId * BMREG_BLOCK_SIZE));
	} else {
		return (BMINTC_READ32
			(BINTC_IMSR1_0_OFFSET + destId * BMREG_BLOCK_SIZE));
	}

}


/*==============================================================================
*
* Function Name: Boolean chal_bmintc_is_interrupt_active(CHAL_HANDLE h,
*                                                        cUInt32 destId,
*                                                        cUInt32 Id)
*
* Description:   Is an interrupt triggered or not.
*
* param:         h: cHal handle.
*
* param:         Id: Interrupt Id
*
* param:         destId: output destination of an interrupt
*
* return:        TRUE or FALSE
*
* Notes:
*=============================================================================*/
__forceinline static Boolean chal_bmintc_is_interrupt_active(CHAL_HANDLE h,
							     cUInt32 destId,
							     cUInt32 Id)
{

	if (Id >= 64)
		return 0;

	if (Id >= 32)		/* Id < 64 */
		return ((BMINTC_READ32
			 (BINTC_IMSR1_0_OFFSET +
			  destId * BMREG_BLOCK_SIZE) & (1 << (Id - 32))) != 0);
	else			/* Id < 32 */
		return ((BMINTC_READ32
			 (BINTC_IMSR0_0_OFFSET +
			  destId * BMREG_BLOCK_SIZE) & (1 << Id)) != 0);
}

#if 0
/*==============================================================================
*
* Function Name: chal_bmintc_set_irq_trigger(CHAL_HANDLE h,
*                                            cUInt32 Id,
*                                            cUInt32 EdgeSetting)
*
* Description:   Set an interrupt triggering mechanism.
*
* param:         h: cHal handle.
*
* param:         Id: Interrupt Id
*
* param:         EdgeSetting: low, high, rising, falling, or either
*
* Notes:
*=============================================================================*/
__forceinline static void chal_bmintc_set_irq_trigger(CHAL_HANDLE h,
							cUInt32 Id,
							cUInt32 EdgeSetting)
{

}
#endif

/*==============================================================================
*
* Function Name: chal_bmintc_set_soft_int(CHAL_HANDLE h, cUInt32 Id)
*
* Description:   set a soft interrupt
*
* param:         h: cHal handle.
*
* param:         Id: Interrupt Id
*
*
* Notes:
*=============================================================================*/
__forceinline static void chal_bmintc_set_soft_int(CHAL_HANDLE h, cUInt32 Id)
{

	if (Id >= 64)
		return;

	if (Id >= 32)		/*Id < 64 */
		BMINTC_WRITE32(BINTC_ISWIR1_OFFSET, 1 << (Id - 32));
	else			/* Id < 32 */
		BMINTC_WRITE32(BINTC_ISWIR0_OFFSET, 1 << Id);

}
__forceinline static void chal_bmintc_set_DSP_soft_int(CHAL_HANDLE h,
						       cUInt32 Id)
{
	BMINTC_WRITE32(BINTC_ISWIR1_OFFSET, 1 << Id);
}

/*==============================================================================
*
* Function Name: chal_bmintc_clear_soft_int(CHAL_HANDLE h, cUInt32 Id)
*
* Description:   Clear a soft interrupt
*
* param:         h: cHal handle.
*
* param:         Id: Interrupt Id
*
*
* Notes:
*=============================================================================*/
__forceinline static void chal_bmintc_clear_soft_int(CHAL_HANDLE h, cUInt32 Id)
{

	if (Id >= 64)
		return;

	if (Id >= 32)		/* Id < 64 */
		BMINTC_WRITE32(BINTC_ISWIR1_CLR_OFFSET, 1 << (Id - 32));
	else			/* Id < 32 */
		BMINTC_WRITE32(BINTC_ISWIR0_CLR_OFFSET, 1 << Id);

}



/*=============================================================================
*
* Function Name: chal_bmintc_set_intr_to_fiq( CHAL_HANDLE h, cUInt32 Id)
*
* Description:   Change interrupt to FIQ
*
* param:         h: cHal handle.
*
* param:         Id: interrupt id.
*
* return:
*
* Notes:
*=============================================================================*/
__forceinline static void chal_bmintc_set_intr_to_fiq(CHAL_HANDLE h, cUInt32 Id)
{
	UInt32 bits = 0;

	if (Id >= 64)
		return;

	if (Id >= 32) {		/* Id < 64 */
		bits = BMINTC_READ32(BINTC_ISELR1_13_OFFSET);
		bits |= 1 << (Id - 32);
		BMINTC_WRITE32(BINTC_ISELR1_13_OFFSET, bits);
	} else {		/* Id < 32 */
		bits = BMINTC_READ32(BINTC_ISELR0_13_OFFSET);
		bits |= 1 << Id;
		BMINTC_WRITE32(BINTC_ISELR0_13_OFFSET, bits);
	}

}

/*=============================================================================
*
* Function Name: chal_bmintc_set_intr_to_fiq( CHAL_HANDLE h, cUInt32 Id)
*
* Description:   Change interrupt to IRQ
*
* param:         h: cHal handle.
*
* param:         Id: interrupt id.
*
* return:
*
* Notes:
*=============================================================================*/
__forceinline static void chal_bmintc_set_intr_to_irq(CHAL_HANDLE h, cUInt32 Id)
{
	UInt32 bits = 0;

	if (Id >= 64)
		return;

	if (Id >= 32) {		/* Id < 64 */
		bits = BMINTC_READ32(BINTC_ISELR1_13_OFFSET);
		bits &= ~(1 << (Id - 32));
		BMINTC_WRITE32(BINTC_ISELR1_13_OFFSET, bits);
	} else {		/* Id < 32 */
		bits = BMINTC_READ32(BINTC_ISELR0_13_OFFSET);
		bits &= ~(1 << Id);
		BMINTC_WRITE32(BINTC_ISELR0_13_OFFSET, bits);
	}
}

/*==============================================================================
*
* Function Name: CHAL_HANDLE chal_bmintc_init(const cUInt32 DeviceBaseAddress)
*
* Description:   Init cHAL mIntc module
*
* param:         DeviceBaseAddress: base virtual addr of intc block
*
*
* Notes:
*=============================================================================*/
__forceinline static CHAL_HANDLE chal_bmintc_init(cUInt32 DeviceBaseAddress)
{

	return (CHAL_HANDLE) DeviceBaseAddress;
}

#endif /* _CHAL_BMINTC_INC_H_ */
