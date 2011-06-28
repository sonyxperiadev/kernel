/*****************************************************************************
* Copyright 2009 - 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#include <linux/kernel.h>
#include <video/vceb_fb.h>

#if !defined( CONFIG_FB_VCEB_SCREEN1_WIDTH )
#   error CONFIG_FB_VCEB_SCREEN1_WIDTH not defined
#endif
#if !defined( CONFIG_FB_VCEB_SCREEN1_HEIGHT )
#   error CONFIG_FB_VCEB_SCREEN1_HEIGHT not defined
#endif
#if !defined( CONFIG_FB_VCEB_BPP )
#   error CONFIG_FB_VCEB_BPP not defined
#endif
#if !defined( CONFIG_FB_VCEB_NUM_FRAMEBUFFERS )
#   error CONFIG_FB_VCEB_NUM_FRAMEBUFFERS not defined
#endif

/*
 * This file calls bootmemheap_alloc to allocate the framebuffers, so the
 * bootmemheap_calc_fb_mem function needs to determine how much memory to reserve
 * for frame buffers.
 *
 * NOTE: This function is called at earlyinit time, which is way before the drivers
 *       are actually loaded, so this function can't query the driver.
 */

size_t vceb_bootmemheap_calc_fb_mem( void )
{
    size_t  hdmisize = 0;
    size_t  lcdsize = 0;

    lcdsize = VCEB_ROUND_UP_WH( CONFIG_FB_VCEB_SCREEN1_WIDTH )
            * VCEB_ROUND_UP_WH( CONFIG_FB_VCEB_SCREEN1_HEIGHT )
            * CONFIG_FB_VCEB_BPP
            / 8
            * CONFIG_FB_VCEB_NUM_FRAMEBUFFERS;

    printk( KERN_INFO "%s: Reserving memory for LCD frame buffer w:%d h:%d bpp:%d x %d (%d bytes)\n",
            __func__, CONFIG_FB_VCEB_SCREEN1_WIDTH, CONFIG_FB_VCEB_SCREEN1_HEIGHT, 
            CONFIG_FB_VCEB_BPP, CONFIG_FB_VCEB_NUM_FRAMEBUFFERS, lcdsize );

    hdmisize = VCEB_ROUND_UP_WH( CONFIG_FB_VCEB_SCREEN2_WIDTH )
             * VCEB_ROUND_UP_WH( CONFIG_FB_VCEB_SCREEN2_HEIGHT )
             * CONFIG_FB_VCEB_BPP
             / 8;

    printk( KERN_INFO "%s: Reserving memory for HDMI frame buffer w:%d h:%d bpp:%d (%d bytes)\n",
            __func__, CONFIG_FB_VCEB_SCREEN2_WIDTH, CONFIG_FB_VCEB_SCREEN2_HEIGHT, 
            CONFIG_FB_VCEB_BPP, hdmisize );

    return hdmisize + lcdsize;
}

