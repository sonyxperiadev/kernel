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
#ifndef KONA_FB_H_
#define KONA_FB_H_

typedef	enum {
	RGB565,
	XRGB8888,
} pixel_format_t;

struct kona_fb_platform_data {
	void*	(*get_dispdrv_func_tbl)(void); 
	u32	screen_width;
	u32	screen_height;
	u32	bytes_per_pixel;
	u32	gpio;
	pixel_format_t	pixel_format;
};

extern void* DISP_DRV_NT35582_WVGA_SMI_GetFuncTable(void);
extern void* DISP_DRV_BCM91008_ALEX_GetFuncTable(void);
extern void* DISP_DRV_R61581_HVGA_SMI_GetFuncTable(void);
#endif /* KONA_FB_H_ */
