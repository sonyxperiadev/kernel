/*****************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
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

#ifndef _TFT_PANEL_H_
#define _TFT_PANEL_H_

struct tft_panel_platform_data
{
    int gpio_lcd_pwr_en;
    int gpio_lcd_reset;
    int gpio_bl_en;
    int gpio_bl_pwr_en;
    int gpio_bl_pwm;
};

#define TFT_PANEL_DRIVER_NAME    "tft_panel"

#endif    /* _TFT_PANEL_H_ */
