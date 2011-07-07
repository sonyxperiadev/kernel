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

#ifndef VCEB_FB_H_
#define VCEB_FB_H_

#include <linux/fb.h>   // for struct fb_info
#include "vceb.h"

enum vcebfb_display_type {
	VCEB_DISPLAY_TYPE_3_2 = 0,
	VCEB_DISPLAY_TYPE_4_8 = 1,
};

enum vcebfb_gpmc_address {
	VCEB_GPMC_A1 = 0,
	VCEB_GPMC_A9 = 1,
};

struct vcebfb_platform_data {
	enum vcebfb_display_type display_type;
	enum vcebfb_gpmc_address gpmc_address;
};


/*
 * When we allocate a resource on the videocore, it takes the width and height 
 * that we pass in and rounds them up to the next multiple of 16 (at least 
 * for the 16-bit and 32-bit formats that we use). 
 *  
 * In order for the data we send down via vc_dispmanx_resource_write_data 
 * to be compatible, we need to round up our dimentions to the next multiple 
 * of 16. 
 */

#define VCEB_ROUND_UP_WH(wh)  (((wh) + 15) & ~15)

/*
 * function to register a callback that fires whenever
 * framebuffer needs an update
 */

struct vmcs_fb_ops {
	int (*open) (void *callback_param, uint32_t screen,
				struct fb_info *info);
	void (*release) (void *callback_param, uint32_t screen);
	int (*update) (void *fb_data, void *callback_param, uint32_t screen,
			struct fb_info *info);
};

extern int vceb_fb_add_screen(uint32_t screen);

extern void vceb_fb_remove_screen(uint32_t screen);

extern int vceb_fb_register_callbacks(uint32_t screen, struct vmcs_fb_ops *ops,
				void *callback_param);

extern void vceb_fb_bus_connected(uint32_t keep_vmcs_res);

#if ( defined(CONFIG_PM) && !defined(CONFIG_ANDROID_POWER) )

/*
 * The following are used for suspend/resume on non-Androind platforms
 */

typedef int (*VCEB_FB_VCHIQ_SUSPEND_CB_T)(void);
typedef int (*VCEB_FB_VCHIQ_RESUME_CB_T)(void);

extern int32_t vceb_fb_register_vchiq_suspend_cb(VCEB_FB_VCHIQ_SUSPEND_CB_T callback);
extern int32_t vceb_fb_register_vchiq_resume_cb(VCEB_FB_VCHIQ_RESUME_CB_T callback);

#endif

#if defined( CONFIG_FB_VCEB_USE_BOOTMEMHEAP )
extern size_t vceb_bootmemheap_calc_fb_mem( void );
#endif

#endif /* VCEB_FB_H_ */
