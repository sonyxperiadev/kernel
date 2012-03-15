/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*             @file     include/video/kona_fb_boot.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/
#ifndef _KONA_FB_BOOT_H_
#define _KONA_FB_BOOT_H_


#ifdef __cplusplus
extern "C" {
#endif


/** 
 * @addtogroup DisplayGroup
 * @{
 */

/** 
 *
 * RHEA dispdrv platform(boot) init interface 
 * Shared between kernel space & boot environment disp drivers
 * 
 */

/* RHEA LCD Drivers Boot Init Interface */
#define	RHEA_TE_IN_0_LCD  	1   
#define	RHEA_TE_IN_1_DSI0 	2
#define	RHEA_TE_IN_2_DSI1 	3

#define	RHEA_CM_I_XRGB888 	1
#define	RHEA_CM_I_RGB565  	2

#define	RHEA_CM_O_RGB565  	1
#define	RHEA_CM_O_RGB565_DSI_VM	2
#define	RHEA_CM_O_RGB666  	3
#define	RHEA_CM_O_RGB888 	4

#define	RHEA_BUS_SMI            1
#define	RHEA_BUS_DSI            2

#define	RHEA_BUS_0            	1
#define	RHEA_BUS_1            	2

#define	RHEA_BUS_CH_0          	1
#define	RHEA_BUS_CH_1          	2

#define	RHEA_BUS_WIDTH_08	1
#define	RHEA_BUS_WIDTH_09	2
#define	RHEA_BUS_WIDTH_16	3
#define	RHEA_BUS_WIDTH_18	4

#define	RHEA_LCD_BOOT_API_REV		((unsigned char)0x10)
#define RHEA_LCD_BOOT_PARM_COUNT 	2
#define RHEA_DISP_DRV_NAME_MAX 		32

struct dispdrv_init_parms  {
	union{
		unsigned long w32;
		struct  {
		  unsigned long  boot_mode  :1;
		  unsigned long  bus_type   :3;
		  unsigned long  bus_no     :3;
		  unsigned long  bus_ch     :3;
		  unsigned long  bus_width  :3;
		  unsigned long  te_input   :3;
		  unsigned long  col_mode_i :3;
		  unsigned long  col_mode_o :3;
			
		} bits;
	}w0;
	
	union  {
		unsigned long w32;
		struct  {
		  unsigned long  api_rev    :8;
		  unsigned long  lcd_rst0   :8;
		  unsigned long  lcd_rst1   :8;
		  unsigned long  lcd_rst2   :8;
		} bits;
	}w1;	
};


/** @} */

#ifdef __cplusplus
}
#endif

#endif //_KONA_FB_BOOT_H_


