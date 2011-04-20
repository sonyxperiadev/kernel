//***************************************************************************
//
//	Copyright © 2004-2010 Broadcom Corporation
//	
//	This program is the proprietary software of Broadcom Corporation 
//	and/or its licensors, and may only be used, duplicated, modified 
//	or distributed pursuant to the terms and conditions of a separate, 
//	written license agreement executed between you and Broadcom (an 
//	"Authorized License").  Except as set forth in an Authorized 
//	License, Broadcom grants no license (express or implied), right 
//	to use, or waiver of any kind with respect to the Software, and 
//	Broadcom expressly reserves all rights in and to the Software and 
//	all intellectual property rights therein.  IF YOU HAVE NO 
//	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE 
//	IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
//	ALL USE OF THE SOFTWARE.  
//	
//	Except as expressly set forth in the Authorized License,
//	
//	1.	This program, including its structure, sequence and 
//		organization, constitutes the valuable trade secrets 
//		of Broadcom, and you shall use all reasonable efforts 
//		to protect the confidentiality thereof, and to use 
//		this information only in connection with your use 
//		of Broadcom integrated circuit products.
//	
//	2.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE 
//		IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM 
//		MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, 
//		EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, 
//		WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY 
//		DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, 
//		MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
//		PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
//		COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR 
//		CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE 
//		RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.  
//
//	3.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT 
//		SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR 
//		(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
//		EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY 
//		WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE 
//		SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
//		POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
//		EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE 
//		ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
//		LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE 
//		OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************

//============================================================================
//
// \file   chal_intc_inc.h
// \brief  OS independent code of AP INTC for BModem using inline function.
// \note
//============================================================================
#ifndef _CHAL_INTC_INC_H_
#define _CHAL_INTC_INC_H_
#include "chal_types.h"
#include "chal_common.h"
#include "brcm_rdb_chipreg.h"

#define CHAL_MAX_INT_NUM 224

typedef struct {
    cUInt32 dword[CHAL_MAX_INT_NUM/32];
}chal_intc_mask_t;

#define INTC_WRITE32( regOffset, data)    CHAL_REG_WRITE32( ((cUInt32) h) + (regOffset), data)
#define INTC_READ32( regOffset) CHAL_REG_READ32( ((cUInt32) h) + (regOffset))

#define __forceinline inline
//==============================================================================
//
// Function Name: void chal_intc_clear_interrupt(CHAL_HANDLE h, cUInt32 Id  ) 
//
// Description:   Clear an interrupt
//
// param:         h: cHal handle.
//
// param:         Id: Interrupt Id 
//
//
// Notes:
//
__forceinline void chal_intc_clear_interrupt(CHAL_HANDLE h, cUInt32 Id  ) 
{
/* can't clear interrupts in CHIPREGS, need to clear in originating peripheral */
#if 0
    cUInt32 mask;
    cUInt32 offset;
    
    offset = (Id/32)*4;

    mask = INTC_READ32(CHIPREG_MDM_IMR0_OFFSET + offset);
    mask &= ~(1 << (Id % 32));
    INTC_WRITE32(CHIPREG_MDM_IMR0_OFFSET + offset, mask);
#endif    
}

//==============================================================================
//
// Function Name: chal_intc_clear_all(CHAL_HANDLE h, chal_intc_mask_t *mask )  
//
// Description:   Clear all interrupts except those in the mask
//
// param:         h: cHal handle.
//
// param:         maks: pointer to intc mask 
//
//
// Notes:
//
__forceinline void chal_intc_clear_intr_mask(CHAL_HANDLE h, chal_intc_mask_t *mask ) 
{
/* can't clear interrupts in CHIPREGS, need to clear in originating peripheral */
#if 0    
    cUInt32 i;
    
    for(i=0; i<CHAL_MAX_INT_NUM/32; i++)
    {
        INTC_WRITE32(CHIPREG_MDM_IMR0_OFFSET + i*4, ~(mask->dword[i]));
    }
#endif
}

//==============================================================================
//
// Function Name: chal_intc_enable_interrupt(CHAL_HANDLE h, cUInt32 Id) 
//
// Description:   enable an interrupt
//
// param:         h: cHal handle.
//
// param:         Id: Interrupt Id 
//
// Notes:
//
__forceinline void chal_intc_enable_interrupt(CHAL_HANDLE h, cUInt32 Id ) 
{
    cUInt32 mask;
    cUInt32 offset;
    
    offset = (Id/32)*4;

    mask = INTC_READ32(CHIPREG_MDM_IMR0_OFFSET + offset);
    mask |= (1 << (Id % 32));
    INTC_WRITE32(CHIPREG_MDM_IMR0_OFFSET + offset, mask);

}

//==============================================================================
//
// Function Name: chal_intc_disable_interrupt(CHAL_HANDLE h,cUInt32 Id) 
//
// Description:   disable an interrupt
//
// param:         h: cHal handle.
//
// param:         Id: Interrupt Id 
//
// Notes:
//
__forceinline void chal_intc_disable_interrupt(CHAL_HANDLE h, cUInt32 Id) 
{
    cUInt32 mask;
    cUInt32 offset;
    
    offset = (Id/32)*4;

    mask = INTC_READ32(CHIPREG_MDM_IMR0_OFFSET + offset);
    mask &= ~(1 << (Id % 32));
    INTC_WRITE32(CHIPREG_MDM_IMR0_OFFSET + offset, mask);
}

//==============================================================================
//
// Function Name: chal_intc_disable_interrupts(CHAL_HANDLE h,  
//                                             chal_intc_mask_t *mask
//                                            ) 
//
// Description:   disable all interrupts except those in the mask
//
// param:         h: cHal handle.
//
// param:         maks: pointer to intc mask 
//
// Notes:
//
__forceinline void chal_intc_disable_intr_mask(CHAL_HANDLE h, 
                                           chal_intc_mask_t *mask
                                          )
{
   
    cUInt32 i;
    
    
    for(i=0; i<CHAL_MAX_INT_NUM/32; i++)
    {
        cUInt32 amask = INTC_READ32(CHIPREG_MDM_IMR0_OFFSET + (i*4));
        amask &= ~(mask->dword[i]);
        INTC_WRITE32(CHIPREG_MDM_IMR0_OFFSET + (i*4), amask);
    }
   
}


//==============================================================================
//
// Function Name: chal_intc_restore_interrupts(CHAL_HANDLE h,  
//                                             chal_intc_mask_t *mask)  
//
// Description:   restore all interrupts based on the mask
//
// param:         h: cHal handle.
//
// param:         maks: pointer to intc mask 
//
// Notes:
//
__forceinline void chal_intc_restore_interrupts(CHAL_HANDLE h, 
                                           chal_intc_mask_t *mask
                                          )
{
    cUInt32 i;
    
    
    for(i=0; i<CHAL_MAX_INT_NUM/32; i++)
    {
        INTC_WRITE32(CHIPREG_MDM_IMR0_OFFSET + i*4, mask->dword[i]);
    }
   
}

//==============================================================================
//
// Function Name: chal_intc_get_interrupt_mask(CHAL_HANDLE h, 
//                                             chal_intc_mask_t* mask)  
//
// Description:   Get interrupt mask
//
// param:         h: cHal handle.
//
// param:         mask (out): intec mask
//
// Notes:
//
__forceinline void chal_intc_get_interrupt_mask(CHAL_HANDLE h, 
                                           chal_intc_mask_t* mask) 
{
    cUInt32 i;

    for(i=0; i<CHAL_MAX_INT_NUM/32; i++)
    {
        mask->dword[i] = INTC_READ32(CHIPREG_MDM_IMR0_OFFSET + i*4);
    }
    
}



//==============================================================================
//
// Function Name: Boolean chal_intc_is_interrupt_enabled(CHAL_HANDLE h,cUInt32 Id  ) 
//
// Description:   Is an interrupt enabled?
//
// param:         h: cHal handle.
//
// param:         Id: Interrupt Id 
//
// return:        TRUE or FALSE
//
// Notes:
//
__forceinline Boolean chal_intc_is_interrupt_enabled(CHAL_HANDLE h, cUInt32 Id ) 
{
    
    cUInt32 mask;
 
    mask = INTC_READ32(CHIPREG_MDM_IMR0_OFFSET + (Id/32)*4);
    return ( (mask & (1 <<(Id % 32))) ? TRUE : FALSE);

    
}

//==============================================================================
//
// Function Name: chal_intc_get_status(CHAL_HANDLE h, chal_intc_mask_t *mask )  
//
// Description:   Get interrupt status
//
// param:         h: cHal handle.
//
// param:         maks: pointer to intc mask 
//
//
// Notes:
//
__forceinline void chal_intc_get_status(CHAL_HANDLE h, chal_intc_mask_t *status)
{
    cUInt32 i;

    for(i=0; i<CHAL_MAX_INT_NUM/32; i++)
    {
        status->dword[i] = INTC_READ32(CHIPREG_MDM_IMSR0_OFFSET + i*4);
    }
}




//==============================================================================
//
// Function Name: Boolean chal_intc_is_interrupt_active(CHAL_HANDLE h, cUInt32 Id  ) 
//
// Description:   Is an interrupt triggered or not.
//
// param:         h: cHal handle.
//
// param:         Id: Interrupt Id 
//
// return:        TRUE or FALSE
//
// Notes:
//
__forceinline Boolean chal_intc_is_interrupt_active(CHAL_HANDLE h, cUInt32 Id ) 
{

    cUInt32 mask;
 
    mask = INTC_READ32(CHIPREG_MDM_IMSR0_OFFSET + (Id/32)*4);
    return ( (mask & (1 <<(Id % 32))) ? TRUE : FALSE);
}


//==============================================================================
//
// Function Name: chal_intc_set_soft_int(CHAL_HANDLE h, cUInt32 Id  ) 
//
// Description:   set a soft interrupt
//
// param:         h: cHal handle.
//
// param:         Id: Interrupt Id 
//
//
// Notes:
//
__forceinline void chal_intc_set_soft_int(CHAL_HANDLE h, cUInt32 Id, cUInt32 cpu_mask)
{
    assert(FALSE);
}



//==============================================================================
//
// Function Name: CHAL_HANDLE chal_intc_init(const cUInt32 DeviceBaseAddress)
//
// Description:   Init cHAL Intc module
//
// param:         DeviceBaseAddress: virtual address of ChipReg block 
//
//
// Notes:
//
CHAL_HANDLE chal_intc_init(cUInt32 DeviceBaseAddress)
{
    return (CHAL_HANDLE)DeviceBaseAddress;
}

#endif /* _CHAL_INTC_INC_H_ */

