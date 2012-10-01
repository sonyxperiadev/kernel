/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */
/******************************************************************************
Copyright 2012 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
******************************************************************************/

/*============================================================================

 \file   chal_intc_inc.h
 \brief  OS independent code of AP INTC for BModem using inline function.
 \note
============================================================================*/
#ifndef _CHAL_INTC_INC_H_
#define _CHAL_INTC_INC_H_
#include <plat/chal/chal_types.h>
#include <plat/chal/chal_common.h>
#include <mach/rdb/brcm_rdb_chipreg.h>

#define CHAL_MAX_INT_NUM 224

struct _chal_intc_mask_t {
	cUInt32 dword[CHAL_MAX_INT_NUM / 32];
} chal_intc_mask_t;
#define chal_intc_mask_t struct _chal_intc_mask_t

#define INTC_WRITE32(regOffset, data)	\
CHAL_REG_WRITE32(((cUInt32) h) + (regOffset), data)
#define INTC_READ32(regOffset)		\
CHAL_REG_READ32(((cUInt32) h) + (regOffset))

#define __forceinline inline
/*=========================================================================

 Function Name: void chal_intc_clear_interrupt(CHAL_HANDLE h, cUInt32 Id)

 Description:   Clear an interrupt

 param:         h: cHal handle.

 param:         Id: Interrupt Id


 Notes:
=========================================================================*/

__forceinline void chal_intc_clear_interrupt(CHAL_HANDLE h, cUInt32 Id)
{
/* can't clear interrupts in CHIPREGS,
 need to clear in originating peripheral */
#if 0
	cUInt32 mask;
	cUInt32 offset;

	offset = (Id / 32) * 4;

	mask = INTC_READ32(CHIPREG_MDM_IMR0_OFFSET + offset);
	mask &= ~(1 << (Id % 32));
	INTC_WRITE32(CHIPREG_MDM_IMR0_OFFSET + offset, mask);
#endif
}

/*=========================================================================

 Function Name: chal_intc_clear_all(CHAL_HANDLE h, chal_intc_mask_t *mask)

 Description:   Clear all interrupts except those in the mask

 param:         h: cHal handle.

 param:         maks: pointer to intc mask


 Notes:
=========================================================================*/
__forceinline void chal_intc_clear_intr_mask(CHAL_HANDLE h,
					     chal_intc_mask_t *mask)
{
/* can't clear interrupts in CHIPREGS,
 need to clear in originating peripheral */
#if 0
	cUInt32 i;

	for (i = 0; i < CHAL_MAX_INT_NUM / 32; i++) {
		INTC_WRITE32(CHIPREG_MDM_IMR0_OFFSET + i * 4,
			     ~(mask->dword[i]));
	}
#endif
}

/*=========================================================================

 Function Name: chal_intc_enable_interrupt(CHAL_HANDLE h, cUInt32 Id)

 Description:   enable an interrupt

 param:         h: cHal handle.

 param:         Id: Interrupt Id

 Notes:
=========================================================================*/
__forceinline void chal_intc_enable_interrupt(CHAL_HANDLE h, cUInt32 Id)
{
	cUInt32 mask;
	cUInt32 offset;

	offset = (Id / 32) * 4;

	mask = INTC_READ32(CHIPREG_MDM_IMR0_OFFSET + offset);
	mask |= (1 << (Id % 32));
	INTC_WRITE32(CHIPREG_MDM_IMR0_OFFSET + offset, mask);

}

/*=========================================================================

 Function Name: chal_intc_disable_interrupt(CHAL_HANDLE h,cUInt32 Id)

 Description:   disable an interrupt

 param:         h: cHal handle.

 param:         Id: Interrupt Id

 Notes:
=========================================================================*/
__forceinline void chal_intc_disable_interrupt(CHAL_HANDLE h, cUInt32 Id)
{
	cUInt32 mask;
	cUInt32 offset;

	offset = (Id / 32) * 4;

	mask = INTC_READ32(CHIPREG_MDM_IMR0_OFFSET + offset);
	mask &= ~(1 << (Id % 32));
	INTC_WRITE32(CHIPREG_MDM_IMR0_OFFSET + offset, mask);
}

/*=========================================================================

 Function Name: chal_intc_disable_interrupts(CHAL_HANDLE h,
					chal_intc_mask_t *mask
					)

 Description:   disable all interrupts except those in the mask

 param:         h: cHal handle.

 param:         maks: pointer to intc mask

 Notes:
=========================================================================*/
__forceinline void chal_intc_disable_intr_mask(CHAL_HANDLE h,
					       chal_intc_mask_t *mask)
{
	cUInt32 i;

	for (i = 0; i < CHAL_MAX_INT_NUM / 32; i++) {
		cUInt32 amask = INTC_READ32(CHIPREG_MDM_IMR0_OFFSET + (i * 4));
		amask &= ~(mask->dword[i]);
		INTC_WRITE32(CHIPREG_MDM_IMR0_OFFSET + (i * 4), amask);
	}

}

/*=========================================================================

 Function Name: chal_intc_restore_interrupts(CHAL_HANDLE h,
					chal_intc_mask_t *mask)

 Description:   restore all interrupts based on the mask

 param:         h: cHal handle.

 param:         maks: pointer to intc mask

 Notes:
=========================================================================*/
__forceinline void chal_intc_restore_interrupts(CHAL_HANDLE h,
						chal_intc_mask_t *mask)
{
	cUInt32 i;

	for (i = 0; i < CHAL_MAX_INT_NUM / 32; i++)
		INTC_WRITE32(CHIPREG_MDM_IMR0_OFFSET + i * 4, mask->dword[i]);

}

/*=========================================================================

 Function Name: chal_intc_get_interrupt_mask(CHAL_HANDLE h,
					chal_intc_mask_t *mask)

 Description:   Get interrupt mask

 param:         h: cHal handle.

 param:         mask (out): intec mask

 Notes:
=========================================================================*/
__forceinline void chal_intc_get_interrupt_mask(CHAL_HANDLE h,
						chal_intc_mask_t *mask)
{
	cUInt32 i;

	for (i = 0; i < CHAL_MAX_INT_NUM / 32; i++)
		mask->dword[i] = INTC_READ32(CHIPREG_MDM_IMR0_OFFSET + i * 4);

}

/*=========================================================================

 Function: Boolean chal_intc_is_interrupt_enabled(CHAL_HANDLE h,cUInt32 Id)

 Description:   Is an interrupt enabled?

 param:         h: cHal handle.

 param:         Id: Interrupt Id

 return:        TRUE or FALSE

 Notes:
=========================================================================*/
__forceinline Boolean chal_intc_is_interrupt_enabled(CHAL_HANDLE h, cUInt32 Id)
{
	Boolean flag;
	cUInt32 mask;

	mask = INTC_READ32(CHIPREG_MDM_IMR0_OFFSET + (Id / 32) * 4);
	if ((mask & (1 << (Id % 32))))
		flag = TRUE;
	else
		flag = FALSE;
	return flag;

}

/*=========================================================================

 Function Name: chal_intc_get_status(CHAL_HANDLE h, chal_intc_mask_t *mask)

 Description:   Get interrupt status

 param:         h: cHal handle.

 param:         maks: pointer to intc mask


 Notes:
=========================================================================*/
__forceinline void chal_intc_get_status(CHAL_HANDLE h,
					chal_intc_mask_t *status)
{
	cUInt32 i;

	for (i = 0; i < CHAL_MAX_INT_NUM / 32; i++) {
		status->dword[i] =
		    INTC_READ32(CHIPREG_MDM_IMSR0_OFFSET + i * 4);
	}
}

/*=========================================================================

 Function: Boolean chal_intc_is_interrupt_active(CHAL_HANDLE h, cUInt32 Id )

 Description:   Is an interrupt triggered or not.

 param:         h: cHal handle.

 param:         Id: Interrupt Id

 return:        TRUE or FALSE

 Notes:
=========================================================================*/
__forceinline Boolean chal_intc_is_interrupt_active(CHAL_HANDLE h, cUInt32 Id)
{
	Boolean flag;
	cUInt32 mask;

	mask = INTC_READ32(CHIPREG_MDM_IMSR0_OFFSET + (Id / 32) * 4);
	if (mask & (1 << (Id % 32)))
		flag = TRUE;
	else
		flag = FALSE;
	return flag;
}

/*=========================================================================

 Function Name: chal_intc_set_soft_int(CHAL_HANDLE h, cUInt32 Id  )

 Description:   set a soft interrupt

 param:         h: cHal handle.

 param:         Id: Interrupt Id

 Notes:
=========================================================================*/
__forceinline void chal_intc_set_soft_int(CHAL_HANDLE h, cUInt32 Id,
					  cUInt32 cpu_mask)
{
}

/*=========================================================================

 Function Name: CHAL_HANDLE chal_intc_init(const cUInt32 DeviceBaseAddress)

 Description:   Init cHAL Intc module

 param:         DeviceBaseAddress: virtual address of ChipReg block

 Notes:
=========================================================================*/
CHAL_HANDLE chal_intc_init(cUInt32 DeviceBaseAddress)
{
	return (CHAL_HANDLE) DeviceBaseAddress;
}

#endif /* _CHAL_INTC_INC_H_ */
