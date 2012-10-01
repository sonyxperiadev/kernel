/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/lcd_backlight.h
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

/*
*
*****************************************************************************
*
*  lcd_backlight.h
*
*  PURPOSE:
*
*     This file defines the platform-independent kernel API for the
*     LCD backlight.
*
*
*  NOTES:
*
*****************************************************************************/

#if !defined(LINUX_LCD_BACKLIGHT_H)
#define LINUX_LCD_BACKLIGHT_H

#if defined(__KERNEL__)

/* ---- Include Files ---------------------------------------------------- */
/* ---- Constants and Types ---------------------------------------------- */

typedef enum LCD_BACKLIGHT_LEVEL {
	LCD_BACKLIGHT_OFF = 0,	/* Controlled by GPIO */
	LCD_BACKLIGHT_MIN_LEVEL = 1,	/* Controlled by PWM */
	LCD_BACKLIGHT_MAX_LEVEL = 30,	/* Controlled by PWM */
	LCD_BACKLIGHT_FULL_ON = 31,	/* Controlled by GPIO */

} LCD_BACKLIGHT_LEVEL;

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

void lcd_backlight_init(void);
void lcd_backlight_deinit(void);
void lcd_backlight_enable(LCD_BACKLIGHT_LEVEL level);
LCD_BACKLIGHT_LEVEL lcd_backlight_max_level(void);
LCD_BACKLIGHT_LEVEL lcd_backlight_curr_level(void);

#endif /* __KERNEL__ */
#endif /* LINUX_LCD_BACKLIGHT_H */
