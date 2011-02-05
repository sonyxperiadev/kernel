//***************************************************************************
//
//	Copyright © 2004-2008 Broadcom Corporation
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
// \brief  OS independent code of INTC using inline function.
// \note
//============================================================================
#ifndef _CHAL_INTC_INC_H_
#define _CHAL_INTC_INC_H_
#include "chal_types.h"
#include "chal_common.h"
#include "brcm_rdb_sysmap.h"
#include "brcm_rdb_intc.h"

enum {
    CHAL_INTC_APPS = 0,
    CHAL_INTC_MODEM = 1
};

typedef struct {
    cUInt32 dword0;
    cUInt32 dword1;
    cUInt32 dword2;
}Chal_Intc_Mask_t;


typedef union {
    struct
    {
        cUInt32 mICCR_intrip        : 1;    
        cUInt32 mICCR_micondbc      : 2;
        cUInt32 mICCR_micondben     : 1;
        cUInt32 mICCR_reserved2     : 1;
        cUInt32 mICCR_micindbc      : 2;
        cUInt32 mICCR_micindben     : 1;
        cUInt32 mICCR_reserved1     : 1;
        cUInt32 mICCR_usbdc         : 2;
        cUInt32 mICCR_usbben        : 1;        
        cUInt32 mICCR_reserved0     : 20;
    } mICCR_field;
    cUInt32 mICCR_dword;
}Chal_Intc_ConfigReg_t;

#define INTC_WRITE32( regOffset, data)           \
    CHAL_REG_WRITE32( (UInt32)h + (regOffset), data)
#define INTC_READ32( regOffset)           \
    CHAL_REG_READ32( (UInt32)h + (regOffset))
    
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
static inline void chal_intc_clear_interrupt(CHAL_HANDLE h, cUInt32 Id  ) 
{
    
    
    if(Id >= 96)
        return;
        
    if(Id >= 64)
    {
        INTC_WRITE32(INTC_ICR2_OFFSET, 1 << (Id-64));
    }
    else if (Id >= 32) // Id < 64
    {
        INTC_WRITE32(INTC_ICR1_OFFSET, 1 << (Id-32));     
    }
    else
    {
        INTC_WRITE32(INTC_ICR0_OFFSET, 1 << Id);
    }
}

//==============================================================================
//
// Function Name: chal_intc_clear_all(CHAL_HANDLE h, Chal_Intc_Mask_t *mask )  
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
static inline void chal_intc_clear_intr_mask(CHAL_HANDLE h, Chal_Intc_Mask_t *mask ) 
{
    
    INTC_WRITE32(INTC_ICR0_OFFSET, ~(mask->dword0));
    INTC_WRITE32(INTC_ICR1_OFFSET, ~(mask->dword1));
    INTC_WRITE32(INTC_ICR2_OFFSET, ~(mask->dword2));
}

//==============================================================================
//
// Function Name: chal_intc_enable_interrupt(CHAL_HANDLE h,cUInt32 cpu,cUInt32 Id) 
//
// Description:   enable an interrupt
//
// param:         h: cHal handle.
//
// param:         Id: Interrupt Id 
//
// param:         cpu: 0: Apps, 1: modem
//
// Notes:
//
static inline void chal_intc_enable_interrupt(CHAL_HANDLE h,cUInt32 cpu,cUInt32 Id ) 
{
    UInt32 rc;
    
    if(Id >= 96)
        return;
    
    if(cpu == CHAL_INTC_APPS)
    {
        if(Id >= 64)
        {
            rc = INTC_READ32(INTC_IMR2_OFFSET);      
            INTC_WRITE32(INTC_IMR2_OFFSET, rc | ( 1 << (Id-64) ));
        }
        else if (Id >= 32) // Id < 64
        {
            rc = INTC_READ32(INTC_IMR1_OFFSET);      
            INTC_WRITE32(INTC_IMR1_OFFSET, rc | ( 1 << (Id-32) ));      
        }
        else
        {
            rc = INTC_READ32(INTC_IMR0_OFFSET);      
            INTC_WRITE32(INTC_IMR0_OFFSET, rc | ( 1 << Id ));
        }
    }
    else
    {
        if(Id >= 64)
        {
            rc = INTC_READ32(INTC_IMRW2_OFFSET);      
            INTC_WRITE32(INTC_IMRW2_OFFSET, rc | ( 1 << (Id-64) ));
        }
        else if (Id >= 32) // Id < 64
        {
            rc = INTC_READ32(INTC_IMRW1_OFFSET);      
            INTC_WRITE32(INTC_IMRW1_OFFSET, rc | ( 1 << (Id-32) ));      
        }
        else
        {
            rc = INTC_READ32(INTC_IMRW_OFFSET);      
            INTC_WRITE32(INTC_IMRW_OFFSET, rc | ( 1 << Id ));
        }
    }
}

//==============================================================================
//
// Function Name: chal_intc_disable_interrupt(CHAL_HANDLE h,cUInt32 cpu,cUInt32 Id) 
//
// Description:   disable an interrupt
//
// param:         h: cHal handle.
//
// param:         Id: Interrupt Id 
//
// param:         cpu: 0: Apps, 1: modem
//
// Notes:
//
static inline void chal_intc_disable_interrupt(CHAL_HANDLE h,cUInt32 cpu,cUInt32 Id) 
{
    UInt32 rc;
    
    if(Id >= 96)
        return;
    
    if(cpu == CHAL_INTC_APPS)
    {  
        if(Id >= 64)
        {
            rc = INTC_READ32(INTC_IMR2_OFFSET);      
            INTC_WRITE32(INTC_IMR2_OFFSET, rc & ~( 1 << (Id-64) ));
        }
        else if (Id >= 32) // Id < 64
        {
            rc = INTC_READ32(INTC_IMR1_OFFSET);      
            INTC_WRITE32(INTC_IMR1_OFFSET, rc & ~( 1 << (Id-32) ));      
        }
        else
        {
            rc = INTC_READ32(INTC_IMR0_OFFSET);      
            INTC_WRITE32(INTC_IMR0_OFFSET, rc & ~( 1 << Id ));
        }
    }
    else
    {
        if(Id >= 64)
        {
            rc = INTC_READ32(INTC_IMRW2_OFFSET);      
            INTC_WRITE32(INTC_IMRW2_OFFSET, rc & ~( 1 << (Id-64) ));
        }
        else if (Id >= 32) // Id < 64
        {
            rc = INTC_READ32(INTC_IMRW1_OFFSET);      
            INTC_WRITE32(INTC_IMRW1_OFFSET, rc & ~( 1 << (Id-32) ));      
        }
        else
        {
            rc = INTC_READ32(INTC_IMRW_OFFSET);      
            INTC_WRITE32(INTC_IMRW_OFFSET, rc & ~( 1 << Id ));
        }
    }
}

//==============================================================================
//
// Function Name: chal_intc_disable_interrupts(CHAL_HANDLE h, 
//                                             cUInt32 cpu, 
//                                             Chal_Intc_Mask_t *mask
//                                            ) 
//
// Description:   disable all interrupts except those in the mask
//
// param:         h: cHal handle.
//
// param:         maks: pointer to intc mask 
//
// param:         cpu: 0: Apps, 1: modem
//
// Notes:
//
static inline void chal_intc_disable_intr_mask(CHAL_HANDLE h, 
                                           cUInt32 cpu, 
                                           Chal_Intc_Mask_t *mask
                                          )
{
    UInt32 rc;
    
    if(cpu == CHAL_INTC_APPS)
    {
        rc = INTC_READ32(INTC_IMR0_OFFSET);
        INTC_WRITE32(INTC_IMR0_OFFSET, rc & (mask->dword0));
        rc = INTC_READ32(INTC_IMR1_OFFSET);
        INTC_WRITE32(INTC_IMR1_OFFSET, rc & (mask->dword1));
        rc = INTC_READ32(INTC_IMR2_OFFSET);
        INTC_WRITE32(INTC_IMR2_OFFSET, rc & (mask->dword2)); 
    }
    else
    {
        rc = INTC_READ32(INTC_IMRW_OFFSET);
        INTC_WRITE32(INTC_IMRW_OFFSET, rc & (mask->dword0));
        rc = INTC_READ32(INTC_IMRW1_OFFSET);
        INTC_WRITE32(INTC_IMRW1_OFFSET, rc & (mask->dword1));
        rc = INTC_READ32(INTC_IMRW2_OFFSET);
        INTC_WRITE32(INTC_IMRW2_OFFSET, rc & (mask->dword2));
    }
   
}


//==============================================================================
//
// Function Name: chal_intc_restore_interrupts(CHAL_HANDLE h, 
//                                             cUInt32 cpu, 
//                                             Chal_Intc_Mask_t *mask)  
//
// Description:   restore all interrupts based on the mask
//
// param:         h: cHal handle.
//
// param:         maks: pointer to intc mask 
//
// param:         cpu: 0: Apps, 1: modem
//
// Notes:
//
static inline void chal_intc_restore_interrupts(CHAL_HANDLE h, 
                                           cUInt32 cpu, 
                                           Chal_Intc_Mask_t *mask
                                          )
{
    
    
    if(cpu == CHAL_INTC_APPS)
    {
        INTC_WRITE32(INTC_IMR0_OFFSET, mask->dword0);
        INTC_WRITE32(INTC_IMR1_OFFSET, mask->dword1);
        INTC_WRITE32(INTC_IMR2_OFFSET, mask->dword2);
    }
    else
    {
        INTC_WRITE32(INTC_IMRW_OFFSET, mask->dword0);
        INTC_WRITE32(INTC_IMRW1_OFFSET, mask->dword1);
        INTC_WRITE32(INTC_IMRW2_OFFSET, mask->dword2);
    } 
   
}

//==============================================================================
//
// Function Name: chal_intc_get_interrupt_mask(CHAL_HANDLE h, 
//                                             cUInt32 cpu, 
//                                             Chal_Intc_Mask_t* mask)  
//
// Description:   Get interrupt mask
//
// param:         h: cHal handle.
//
// param:         cpu: 0: Apps, 1: modem
//
// param:         mask (out): intec mask
//
// Notes:
//
static inline void chal_intc_get_interrupt_mask(CHAL_HANDLE h, 
                                           cUInt32 cpu, 
                                           Chal_Intc_Mask_t* mask) 
{
    
    if(cpu == CHAL_INTC_APPS)
    {
        mask->dword0 = INTC_READ32(INTC_IMR0_OFFSET);
        mask->dword1 = INTC_READ32(INTC_IMR1_OFFSET);
        mask->dword2 = INTC_READ32(INTC_IMR2_OFFSET);
    }
    else
    {
        mask->dword0 = INTC_READ32(INTC_IMRW_OFFSET);
        mask->dword1 = INTC_READ32(INTC_IMRW1_OFFSET);
        mask->dword2 = INTC_READ32(INTC_IMRW2_OFFSET); 
    }
    
    return;
}



//==============================================================================
//
// Function Name: Boolean chal_intc_is_interrupt_enabled(CHAL_HANDLE h,cUInt32 cpu, cUInt32 Id  ) 
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
static inline Boolean chal_intc_is_interrupt_enabled(CHAL_HANDLE h, cUInt32 cpu, cUInt32 Id ) 
{
    if(Id >= 96)
        return 0;
        
    if(cpu == CHAL_INTC_APPS)
    {
        if(Id >= 64)
        {
            return ((INTC_READ32(INTC_IMR2_OFFSET) & (1<<(Id-64))) != 0);
        }
        else if (Id >= 32) // Id < 64
        {
            return ((INTC_READ32(INTC_IMR1_OFFSET) & (1<<(Id-32))) != 0);      
        }
        else
        {
            return ((INTC_READ32(INTC_IMR0_OFFSET) & (1<<Id)) != 0);
        }
    }
    else
    {
        if(Id >= 64)
        {
            return ((INTC_READ32(INTC_IMRW2_OFFSET) & (1<<(Id-64))) != 0);
        }
        else if (Id >= 32) // Id < 64
        {
            return ((INTC_READ32(INTC_IMRW1_OFFSET) & (1<<(Id-32))) != 0);      
        }
        else
        {
            return ((INTC_READ32(INTC_IMRW_OFFSET) & (1<<Id)) != 0);
        }
    }
}

//==============================================================================
//
// Function Name: chal_intc_get_status(CHAL_HANDLE h, cUInt32 cpu, Chal_Intc_Mask_t *mask )  
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
static inline void chal_intc_get_status(CHAL_HANDLE h, cUInt32 cpu, Chal_Intc_Mask_t *status)
{


    if(cpu == CHAL_INTC_APPS)
    {
        status->dword0 = INTC_READ32(INTC_IMSR0_OFFSET);
        status->dword1 = INTC_READ32(INTC_IMSR1_OFFSET);
        status->dword2 = INTC_READ32(INTC_IMSR2_OFFSET);
    }
    else
    {
        status->dword0 = INTC_READ32(INTC_IMSRW_OFFSET);
        status->dword1 = INTC_READ32(INTC_IMSRW1_OFFSET);
        status->dword2 = INTC_READ32(INTC_IMSRW2_OFFSET);
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
static inline Boolean chal_intc_is_interrupt_active(CHAL_HANDLE h, cUInt32 Id ) 
{
    
    if (Id >= 96)
        return 0;
    
    if (Id >= 64)
        return ( (INTC_READ32(INTC_ISR2_OFFSET) & (1<<(Id-64))) != 0  );
    else if (Id >= 32)// Id < 64
        return ( (INTC_READ32(INTC_ISR1_OFFSET) & (1<<(Id-32))) != 0  );
    else // Id < 32 
        return ( (INTC_READ32(INTC_ISR0_OFFSET) & (1<<Id)) != 0  );
}

//==============================================================================
//
// Function Name: chal_intc_get_irq_trigger(CHAL_HANDLE h, cUInt32 Id) 
//
// Description:   Get an interrupt triggering mechanism.
//
// param:         h: cHal handle.
//
// param:         Id: Interrupt Id 
//
// return:        EdgeSetting: low, high, rising, falling, or either
//
// Notes:
//
static inline UInt32 chal_intc_get_irq_trigger(CHAL_HANDLE h, cUInt32 Id)
{
	UInt32 rc;

	if (Id < 8)
	{
		rc = INTC_READ32(INTC_ISCR0_OFFSET);
		rc = rc & (0xF<<(Id*4));
		rc = rc >> (Id*4);
	}
	else if( Id <16)
	{
		rc = INTC_READ32(INTC_ISCR1_OFFSET);
		rc = rc & (0xF<<((Id-8)*4));
		rc = rc >> ((Id-8)*4);
	}
	else if( Id <24)
	{
		rc = INTC_READ32(INTC_ISCR2_OFFSET);
		rc = rc & (0xF<<((Id-16)*4));
		rc = rc >> ((Id-16)*4);
	}
	else if( Id <32)
	{
		rc = INTC_READ32(INTC_ISCR3_OFFSET);
		rc = rc & (0xF<<((Id-24)*4));
		rc = rc >> ((Id-24)*4);
	}
	else if( Id <40)
	{   
		rc = INTC_READ32(INTC_ISCR4_OFFSET);
		rc = rc & (0xF<<((Id-32)*4));
		rc = rc >> ((Id-32)*4);
	}
	else if( Id <48)
	{    
		rc = INTC_READ32(INTC_ISCR5_OFFSET);
		rc = rc & (0xF<<((Id-40)*4));
		rc = rc >> ((Id-40)*4);
	}
	else if( Id <56)
	{
		rc = INTC_READ32(INTC_ISCR6_OFFSET);
		rc = rc & (0xF<<((Id-48)*4));
		rc = rc >> ((Id-48)*4);
	}
	else if( Id <64)    
	{
		rc = INTC_READ32(INTC_ISCR7_OFFSET);
		rc = rc & (0xF<<((Id-56)*4));
		rc = rc >> ((Id-56)*4);
	}
	else if( Id <72)    
	{
		rc = INTC_READ32(INTC_ISCR8_OFFSET);
		rc = rc & (0xF<<((Id-64)*4));
		rc = rc >> ((Id-64)*4);
	}
	else if( Id <80)
	{
		rc = INTC_READ32(INTC_ISCR9_OFFSET);
		rc = rc & (0xF<<((Id-72)*4));
		rc = rc >> ((Id-72)*4);
	}
	else if( Id <88)    
	{
		rc = INTC_READ32(INTC_ISCR10_OFFSET);
		rc = rc & (0xF<<((Id-80)*4));
		rc = rc >> ((Id-80)*4);
	}
	else if( Id <96)    
	{
		rc = INTC_READ32(INTC_ISCR11_OFFSET);
		rc = rc & (0xF<<((Id-88)*4));
		rc = rc >> ((Id-88)*4);
	}
	else
	{
		rc = 0;
	}
	return rc;
}

//==============================================================================
//
// Function Name: chal_intc_set_irq_trigger(CHAL_HANDLE h, cUInt32 Id, cUInt32 EdgeSetting ) 
//
// Description:   Set an interrupt triggering mechanism.
//
// param:         h: cHal handle.
//
// param:         Id: Interrupt Id 
//
// param:         EdgeSetting: low, high, rising, falling, or either
//
// Notes:
//
static inline void chal_intc_set_irq_trigger(CHAL_HANDLE h, cUInt32 Id, cUInt32 EdgeSetting )
{
    UInt32 rc;
    
    if (Id < 8)
    {
        rc = INTC_READ32(INTC_ISCR0_OFFSET);
        rc &= ~(0xF<<(Id*4));
        INTC_WRITE32(INTC_ISCR0_OFFSET, rc | (EdgeSetting<<(Id*4)));
    }
    else if( Id <16)
    {
        rc = INTC_READ32(INTC_ISCR1_OFFSET);
        rc &= ~(0xF<<((Id-8)*4));
        INTC_WRITE32(INTC_ISCR1_OFFSET, rc | (EdgeSetting<<((Id-8)*4)));
    }
    else if( Id <24)
    {
        rc = INTC_READ32(INTC_ISCR2_OFFSET);
        rc &= ~(0xF<<((Id-16)*4));
        INTC_WRITE32(INTC_ISCR2_OFFSET, rc | (EdgeSetting<<((Id-16)*4)));
    
    }
    else if( Id <32)
    {
        rc = INTC_READ32(INTC_ISCR3_OFFSET);
        rc &= ~(0xF<<((Id-24)*4));
        INTC_WRITE32(INTC_ISCR3_OFFSET, rc | (EdgeSetting<<((Id-24)*4)));
    }
    else if( Id <40)
    {   
        rc = INTC_READ32(INTC_ISCR4_OFFSET);
        rc &= ~(0xF<<((Id-32)*4));
        INTC_WRITE32(INTC_ISCR4_OFFSET, rc | (EdgeSetting<<((Id-32)*4))); 
    }
    else if( Id <48)
    {    
        rc = INTC_READ32(INTC_ISCR5_OFFSET);
        rc &= ~(0xF<<((Id-40)*4));
        INTC_WRITE32(INTC_ISCR5_OFFSET, rc | (EdgeSetting<<((Id-40)*4)));
    }
    else if( Id <56)
    {
        rc = INTC_READ32(INTC_ISCR6_OFFSET);
        rc &= ~(0xF<<((Id-48)*4));
        INTC_WRITE32(INTC_ISCR6_OFFSET, rc | (EdgeSetting<<((Id-48)*4)));
    }
    else if( Id <64)    
    {
        rc = INTC_READ32(INTC_ISCR7_OFFSET);
        rc &= ~(0xF<<((Id-56)*4));
        INTC_WRITE32(INTC_ISCR7_OFFSET, rc | (EdgeSetting<<((Id-56)*4)));
    }
    else if( Id <72)    
    {
        rc = INTC_READ32(INTC_ISCR8_OFFSET);
        rc &= ~(0xF<<((Id-64)*4));
        INTC_WRITE32(INTC_ISCR8_OFFSET, rc | (EdgeSetting<<((Id-64)*4)));
    }
    else if( Id <80)
    {
        rc = INTC_READ32(INTC_ISCR9_OFFSET);
        rc &= ~(0xF<<((Id-72)*4));
        INTC_WRITE32(INTC_ISCR9_OFFSET, rc | (EdgeSetting<<((Id-72)*4)));
    }
    else if( Id <88)    
    {
        rc = INTC_READ32(INTC_ISCR10_OFFSET);
        rc &= ~(0xF<<((Id-80)*4));
        INTC_WRITE32(INTC_ISCR10_OFFSET, rc | (EdgeSetting<<((Id-80)*4)));
    }
    else if( Id <96)    
    {
        rc = INTC_READ32(INTC_ISCR11_OFFSET);
        rc &= ~(0xF<<((Id-88)*4));
        INTC_WRITE32(INTC_ISCR11_OFFSET, rc | (EdgeSetting<<((Id-88)*4)));
    }
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
static inline void chal_intc_set_soft_int(CHAL_HANDLE h, cUInt32 Id)
{


    if (Id >= 96)
        return;
        
    if(Id >= 64)
        INTC_WRITE32(INTC_ISWI2R_OFFSET, 1 << (Id-64));
    else if(Id >= 32) // Id < 64
        INTC_WRITE32(INTC_ISWI1R_OFFSET, 1 << (Id-32));
    else // Id < 32 
        INTC_WRITE32(INTC_ISWIR0_OFFSET, 1 << Id);
    
}

//==============================================================================
//
// Function Name: chal_intc_clear_soft_int(CHAL_HANDLE h, cUInt32 Id  ) 
//
// Description:   Clear a soft interrupt
//
// param:         h: cHal handle.
//
// param:         Id: Interrupt Id 
//
//
// Notes:
//
static inline void chal_intc_clear_soft_int(CHAL_HANDLE h, cUInt32 Id)
{
    if (Id >= 96)
        return;
        
    if(Id >= 64)
        INTC_WRITE32(INTC_ISWC2R_OFFSET, 1 << (Id-64));
    else if(Id >= 32) // Id < 64
        INTC_WRITE32(INTC_ISWC1R_OFFSET, 1 << (Id-32));
    else // Id < 32 
        INTC_WRITE32(INTC_ISWCR0_OFFSET, 1 << Id);
    
}

//==============================================================================
//
// Function Name: chal_intc_set_irq_config_reg( CHAL_HANDLE h, cUInt32 val)
//
// Description:   Set Intc config register
//
// param:         h: cHal handle.
//
// param:         val: value to set 
//
//
// Notes:
//
static inline void chal_intc_set_irq_config_reg( CHAL_HANDLE h, cUInt32 val)
{
    
    INTC_WRITE32(INTC_ICCR_OFFSET, val);

}

//==============================================================================
//
// Function Name: chal_intc_read_irq_config_reg( CHAL_HANDLE h)
//
// Description:   read Intc config register
//
// param:         h: cHal handle.
//
// return:        value of the register
//
// Notes:
//
static inline cUInt32 chal_intc_read_irq_config_reg( CHAL_HANDLE h)
{
    
    return (INTC_READ32(INTC_ICCR_OFFSET));

}

//==============================================================================
//
// Function Name: chal_intc_set_intr_to_fiq( CHAL_HANDLE h, cUInt32 Id)
//
// Description:   Change interrupt to FIQ
//
// param:         h: cHal handle.
//
// param:         Id: interrupt id.
//
// return:        
//
// Notes:
//
static inline void chal_intc_set_intr_to_fiq( CHAL_HANDLE h, cUInt32 Id)
{
    UInt32 rc;
    
    if (Id >= 96)
        return;
        
    if(Id >= 64)
    {
        rc = INTC_READ32(INTC_ISEL2R_OFFSET);
        INTC_WRITE32(INTC_ISEL2R_OFFSET, rc | ( 1 << (Id-64) ));
    }
    else if(Id >= 32) 
    {
        rc = INTC_READ32(INTC_ISEL1R_OFFSET);
        INTC_WRITE32(INTC_ISEL1R_OFFSET, rc | ( 1 << (Id-32) ));
    }
    else 
    {
        rc = INTC_READ32(INTC_ISELR_OFFSET);
        INTC_WRITE32(INTC_ISELR_OFFSET, rc | ( 1 << Id )); 
    }
}

//==============================================================================
//
// Function Name: chal_intc_set_intr_to_fiq( CHAL_HANDLE h, cUInt32 Id)
//
// Description:   Change interrupt to IRQ
//
// param:         h: cHal handle.
//
// param:         Id: interrupt id.
//
// return:        
//
// Notes:
//
static inline void chal_intc_set_intr_to_irq( CHAL_HANDLE h, cUInt32 Id)
{
    UInt32 rc;
    
    if (Id >= 96)
        return;
        
    if(Id >= 64)
    {
        rc = INTC_READ32(INTC_ISEL2R_OFFSET);
        INTC_WRITE32(INTC_ISEL2R_OFFSET, rc & ~( 1 << (Id-64) ));
    }
    else if(Id >= 32)
    { 
        rc = INTC_READ32(INTC_ISEL1R_OFFSET);
        INTC_WRITE32(INTC_ISEL1R_OFFSET, rc & ~( 1 << (Id-32) )); 
    }
    else
    {   
        rc = INTC_READ32(INTC_ISELR_OFFSET);
        INTC_WRITE32(INTC_ISELR_OFFSET, rc & ~( 1 << Id )); 
    }
}

//==============================================================================
//
// Function Name: CHAL_HANDLE chal_intc_gate_wcdma_clock(CHAL_HANDLE h, cUInt8 val)
//
// Description:   Gate WCDMA ARM9 clock
//                0: WCDMA ARM9 clock gate is not affected by ARM9 JTAG TCK
//                1: WCDMA ARM9 clock is turned on upon posedee of ARM9 JTAG TCK
//
// param:         h: cHal handle
//
// param:         val: gate source
//
// Notes:
//
void chal_intc_gate_wcdma_clock(CHAL_HANDLE h, cUInt8 val)
{
    if(val)
    {
        INTC_WRITE32(INTC_WARM9SLPR_OFFSET, 0x2);	
    }
}

//==============================================================================
//
// Function Name: CHAL_HANDLE chal_intc_set_wcdma_clock(CHAL_HANDLE h, cUInt8 val)
//
// Description:   Configure WCDMA ARM9 clock
//                0: WCDMA ARM9 clock is turned on
//                1: WCDMA ARM9 clock is turned off
//
// param:         h: cHal handle
//
// param:         val: set value
//
// Notes:
//
void chal_intc_set_wcdma_clock(CHAL_HANDLE h, cUInt8 val)
{
    UInt32 rc;
    
    rc = INTC_READ32(INTC_WARM9SLPR_OFFSET);   
    
    if(val)
    {
        INTC_WRITE32(INTC_WARM9SLPR_OFFSET, rc | 0x1);	
    }
    else
    {
        INTC_WRITE32(INTC_WARM9SLPR_OFFSET, rc & ~(0x1));		
    }    
}

//==============================================================================
//
// Function Name: CHAL_HANDLE chal_intc_init(const cUInt32 DeviceBaseAddress)
//
// Description:   Init cHAL Intc module
//
// param:         DeviceBaseAddress: base virtual addr of intc block
//
//
// Notes:
//
CHAL_HANDLE chal_intc_init(const cUInt32 DeviceBaseAddress)
{
   
    return (CHAL_HANDLE)DeviceBaseAddress;
}

#endif //_CHAL_INTC_INC_H_
