/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#include <plat/chal/chal_types.h>
#include <plat/chal/chal_common.h>
#include <chal/chal_dmux.h>
#include <mach/rdb/brcm_rdb_dmux.h>


/*
 * ****************************************************************************
 *  local macro declarations
 * ****************************************************************************
 */

#define CHAL_DMUX_CHAN_INVALID      0x0F
#define CHAL_DMUX_PERI_INVALID      0x7F

#undef chal_dprintf
#define chal_dprintf(a)             /* printf a*/


typedef struct chal_dmux_s {
   uint32_t baseAddr;
} chal_dmux_t, *p_chal_dmux_t;

static chal_dmux_t dmux_dev;


/*
 * ****************************************************************************
 * 
 *   Function Name: chal_dmux_init 
 * 
 *   Description: Initialize CHAL DMUX for the passed DMUX instance
 * 
 * ****************************************************************************
 */
CHAL_HANDLE chal_dmux_init(uint32_t baseAddr)
{
   chal_dmux_t *dev = NULL;

   chal_dprintf(( "chal_dmux_init\n" ));
	
   /* Don't re-init a block*/
   if (dmux_dev.baseAddr == baseAddr) 
   {
      chal_dprintf(("ERROR: chal_dmux_init: already initialized\n"));
      return (CHAL_HANDLE)dev;
   }
   
   dmux_dev.baseAddr = baseAddr;
   dev = &dmux_dev;
   return (CHAL_HANDLE)dev;
}


/*
 * ****************************************************************************
 * 
 *   Function Name: chal_dmux_deinit 
 * 
 *   Description: De-Initialize CHAL DMUX for the passed DMUX instance
 * 
 * ****************************************************************************
 */
CHAL_DMUX_STATUS_t chal_dmux_deinit(CHAL_HANDLE handle)
{
   chal_dmux_t *dev = (chal_dmux_t *) handle;
   
   chal_dprintf(( "chal_dmux_deinit\n"));	
   
   if (dev == NULL)
   {
      chal_dprintf(("ERROR: chal_dmux_deinit: NULL handle\n"));
      return CHAL_DMUX_STATUS_FAILURE;	
   }	
   
   dev->baseAddr = 0; 	
   
   return CHAL_DMUX_STATUS_SUCCESS;	
}


/*
 * ****************************************************************************
 * 
 *   Function Name: chal_dmux_alloc_channel 
 * 
 *   Description: Allocate channel
 * 
 * ****************************************************************************
 */
CHAL_DMUX_STATUS_t chal_dmux_alloc_channel(CHAL_HANDLE handle, uint32_t *chan)
{
   chal_dmux_t *pDmuxDev = (chal_dmux_t *) handle;
   uint32_t channel;
   
   channel = CHAL_REG_READ32 ( pDmuxDev->baseAddr + DMUX_CHAN_ALLOC_DEALLOC_OFFSET ) & DMUX_CHAN_ALLOC_DEALLOC_CHANNEL_AD_MASK;

   if( channel != CHAL_DMUX_CHAN_INVALID )
   {
      *chan = channel;
      return CHAL_DMUX_STATUS_SUCCESS;
   }
   else
   {
      return CHAL_DMUX_STATUS_FAILURE;
   }
}

/*
 * ****************************************************************************
 * 
 *   Function Name: chal_dmux_dealloc_channel 
 * 
 *   Description: De-allocate channel
 * 
 * ****************************************************************************
 */
CHAL_DMUX_STATUS_t chal_dmux_dealloc_channel(  CHAL_HANDLE handle,  uint32_t channel )
{
   chal_dmux_t *pDmuxDev = (chal_dmux_t *) handle;
   
   CHAL_REG_WRITE32 ( pDmuxDev->baseAddr + DMUX_CHAN_ALLOC_DEALLOC_OFFSET, channel );

   return CHAL_DMUX_STATUS_SUCCESS;	
}

/*
 * ****************************************************************************
 * 
 *   Function Name: chal_dmux_alloc_peripheral 
 * 
 *   Description: Allocate peripherals associated with a certain channel
 * 
 * ****************************************************************************
 */
CHAL_DMUX_STATUS_t chal_dmux_alloc_peripheral(
   CHAL_HANDLE handle,
   uint32_t channel, 
   CHAL_DMUX_END_POINT_t peri_a,
   CHAL_DMUX_END_POINT_t peri_b,
   uint8_t *src_id,
   uint8_t *dst_id
   )
{
   chal_dmux_t *pDmuxDev = (chal_dmux_t *) handle;
   uint32_t val1, val2;
   uint32_t time_out = 256;
   
   if ( ( peri_a != CHAL_DMUX_EPT_INVALID ) && ( peri_b != CHAL_DMUX_EPT_INVALID ) )
   {
      val1 = peri_a | ( peri_b << DMUX_PER_0_PER_B_0_SHIFT );
   }
   else if(peri_a != CHAL_DMUX_EPT_INVALID)
   {
      val1 = peri_a | ( CHAL_DMUX_PERI_INVALID << DMUX_PER_0_PER_B_0_SHIFT );
   }
   else
   {
      val1 = peri_b | ( CHAL_DMUX_PERI_INVALID << DMUX_PER_0_PER_B_0_SHIFT );
   }

   CHAL_REG_WRITE32(pDmuxDev->baseAddr + DMUX_PER_0_OFFSET + channel * 4, val1);

   do
   {
      val2 = CHAL_REG_READ32( pDmuxDev->baseAddr + DMUX_PER_0_OFFSET + channel * 4 );

      if( ( val2 & ~DMUX_PER_0_RESERVED_MASK ) == val1 ) 
      {
         if ( dst_id )
         {
            *dst_id = 2 * channel;
            if ( src_id )
            {
               *src_id = 2 * channel + 1;
            }
         } 
         else
         {
            *src_id = 2 * channel;
         }
         return CHAL_DMUX_STATUS_SUCCESS;	
      }
   } while( time_out-- );

   return CHAL_DMUX_STATUS_FAILURE;	
}

/*
 * ****************************************************************************
 * 
 *   Function Name: chal_dmux_dealloc_peripheral 
 * 
 *   Description: De-allocate peripheral
 * 
 * ****************************************************************************
 */
CHAL_DMUX_STATUS_t chal_dmux_dealloc_peripheral( CHAL_HANDLE handle,  uint32_t channel )
{
   chal_dmux_t *pDmuxDev = (chal_dmux_t *) handle;

   channel &= DMUX_PER_DEALLOC_CHANNEL_P_MASK;
   CHAL_REG_WRITE32 ( pDmuxDev->baseAddr + DMUX_PER_DEALLOC_OFFSET, channel );

   return CHAL_DMUX_STATUS_SUCCESS;	
}



/*
 * ****************************************************************************
 * 
 *   Function Name: chal_dmux_protect 
 * 
 *   Description: Protect
 * 
 * ****************************************************************************
 */
CHAL_DMUX_STATUS_t chal_dmux_protect( CHAL_HANDLE handle )
{
   chal_dmux_t *pDmuxDev = (chal_dmux_t *) handle;
   
   if( CHAL_REG_READ32 ( pDmuxDev->baseAddr + DMUX_SEMAPHORE_OFFSET ) == 0 )
   {
      return CHAL_DMUX_STATUS_SUCCESS;
   }
   else
   {
      return CHAL_DMUX_STATUS_FAILURE;
   }
}


/*
 * ****************************************************************************
 * 
 *   Function Name: chal_dmux_unprotect 
 * 
 *   Description: Unprotect
 * 
 * ****************************************************************************
 */
CHAL_DMUX_STATUS_t chal_dmux_unprotect( CHAL_HANDLE handle )
{
   chal_dmux_t *pDmuxDev = (chal_dmux_t *) handle;

   CHAL_REG_WRITE32 ( pDmuxDev->baseAddr + DMUX_SEMAPHORE_OFFSET, 0 );
   
   return CHAL_DMUX_STATUS_SUCCESS;	
}







