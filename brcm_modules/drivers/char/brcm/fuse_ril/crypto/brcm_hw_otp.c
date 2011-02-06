/*****************************************************************************
*  Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/
/****************************************************************************

	File Name: brcm_hw_otp.c
	

****************************************************************************/

#include <linux/io.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/module.h>
#include "type_def.h"
#include "brcm_hw_otp.h"
#include "bcm_kril_debug.h"
#include <plat/bcm_otp.h>

void ShowByteArray(uint8 *data, uint16 length, char *description)
{
   uint16 x;

   printk("%s", description);
   for (x=0; x<length; x++)
   {
      printk(" %x", data[x]);
   }
   printk("\n");
}

/* This definition should only be enabled for testing */
//#define DEBUG_GET_HUK


#ifdef DEBUG_GET_HUK

uint32 GetHuk(uint8 *huk)
{
   uint8 testHuk[HUK_LEN] = {
      'B', 'r', 'o', 'a', 'd', 'c', 'o', 'm',
      ' ', 'T', 'e', 's', 't', 'H', 'U', 'K'
   };

   memcpy(huk, testHuk, HUK_LEN);

   return(1);
}

#else

#define LOTP_CMD_READ		0x0
#define LOTP_CMD_INIT		0x4

#define CUS_ROW_START   28
#define CUS_ROW_END     63

uint32 GetHuk(uint8 *huk)
{
#ifdef CONFIG_BCM_OTP
   uint8 read_len, addr;
   uint32 data; 

   read_len = 0 ; 

   for (addr = CUS_ROW_START; (addr <= CUS_ROW_END) &&  (read_len < HUK_LEN); ++addr)
   {
      if (otp_read_word(addr, 1, &data) == 0)
      {
		     if (data & OTP_LOTP_RDATA_LOTP_RDATA_FAIL) continue;
		        
		     huk[read_len++] = (data >> 16) & 0xFF;
		     if (read_len < HUK_LEN)
		      huk[read_len++] = (data >> 8) & 0xFF;
		     if (read_len < HUK_LEN)
		      huk[read_len++] = (data >> 0) & 0xFF;
      }
	    else 
	    {
	       KRIL_DEBUG(DBG_ERROR,"OTP read Failed!!!\n");
	       return 0; 
	    }
   }
   
   //ShowByteArray((uint8 *) huk, HUK_LEN, "huk: ");
   return 1; 

#else

   KRIL_DEBUG(DBG_ERROR,"OTP not supported. GetHuk() Failed!!!\n");
   return 0;

#endif //CONFIG_BCM_OTP
}



#endif
