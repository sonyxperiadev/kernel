/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/lcd.h
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
*  lcd.c
*
*  PURPOSE:
*
*   This implements the LCD driver.
*
*  NOTES:
*
*****************************************************************************/

#if !defined(BCM_LINUX_LCD_H)
#define BCM_LINUX_LCD_H

/* ---- Include Files ---------------------------------------------------- */

#include <linux/ioctl.h>
#include <linux/fb.h>

/* ---- Constants and Types ---------------------------------------------- */

#define LCD_MAGIC   'L'

#define LCD_CMD_FIRST                           0x80

#define LCD_CMD_RESET                           0x80
#define LCD_CMD_ENABLE_BACKLIGHT                0x81
#define LCD_CMD_ENABLE_CS                       0x82
#define LCD_CMD_SCOPE_TIMEOUT                   0x83
#define LCD_CMD_INIT                            0x84
#define LCD_CMD_INIT_ALL                        0x85
#define LCD_CMD_SETUP                           0x86
#define LCD_CMD_HOLD                            0x87
#define LCD_CMD_PULSE                           0x88
#define LCD_CMD_REG                             0x89
#define LCD_CMD_RECT                            0x8A
#define LCD_CMD_COLOR_TEST                      0x8B
#define LCD_CMD_DIRTY_ROWS                      0x8C
#define LCD_CMD_PRINT_REGS                      0x8D
#define LCD_CMD_PRINT_DATA                      0x8E
#define LCD_CMD_ENABLE_SUB_BACKLIGHT            0x92
#define LCD_CMD_PWR_OFF                         0x93
#define LCD_CMD_IS_DISPLAY_REGION_SUPPORTED     0x98
#define LCD_CMD_INFO                            0x99
#define LCD_CMD_IS_DIRTY_ROW_UPDATE_SUPPORTED   0x9A
#define LCD_CMD_SET_DISPLAY_LAYER_NUM           0x9B
#define LCD_CMD_DIRTY_ROW_BITS                  0x9C
#define LCD_CMD_COPYAREA                        0x9D
#define LCD_CMD_FILLRECT_COLOR                  0x9E
#define LCD_CMD_DIRTY_RECT			0x9F
#define LCD_CMD_LAST                            0x9F

typedef struct {
	unsigned char reg;
	unsigned char val;

} LCD_Reg_t;

typedef struct {
	unsigned char startReg;
	unsigned char endReg;

} LCD_PrintRegs_t;

typedef struct {
	unsigned int x;
	unsigned int y;
	unsigned int numPixels;

} LCD_PrintData_t;

typedef struct {
	unsigned int left;
	unsigned int top;
	unsigned int width;
	unsigned int height;
	unsigned int color;

} LCD_Rect_t;

typedef struct {
	unsigned int left;	/* inclusive */
	unsigned int top;	/* inclusive */
	unsigned int right;	/* inclusive */
	unsigned int bottom;	/* inclusive */

} LCD_DirtyRect_t;

typedef struct {
	unsigned int top;	/* inclusive */
	unsigned int bottom;	/* inclusive */

} LCD_DirtyRows_t;

typedef struct {
	unsigned numRows;	/* Number of rows which are significant */
	unsigned int *bits;	/* variable length bitmask bit 0 = row 0 */

} LCD_DirtyRowBits_t;

typedef struct {
	int width;
	int height;
	int bitsPerPixel;

} LCD_Info_t;

typedef struct {
	unsigned int dx;
	unsigned int dy;
	unsigned int width;
	unsigned int height;
	unsigned int rawColor;
} LCD_FillRectColor_t;

/* --------------------------------------------------------------------------
**
*/

#define LCDFB_IOCTL_UPDATE_LCD 0x46ff

#define LCD_IOCTL_RESET                         _IO(LCD_MAGIC, LCD_CMD_RESET)	/* arg is int */
#define LCD_IOCTL_ENABLE_BACKLIGHT              _IO(LCD_MAGIC, LCD_CMD_ENABLE_BACKLIGHT)	/* arg is int */
#define LCD_IOCTL_ENABLE_SUB_BACKLIGHT          _IO(LCD_MAGIC, LCD_CMD_ENABLE_SUB_BACKLIGHT)	/* arg is int */
#define LCD_IOCTL_ENABLE_CS                     _IO(LCD_MAGIC, LCD_CMD_ENABLE_CS)	/* arg is int */
#define LCD_IOCTL_SCOPE_TIMEOUT                 _IO(LCD_MAGIC, LCD_CMD_SCOPE_TIMEOUT)	/* arg is int */
#define LCD_IOCTL_INIT                          _IO(LCD_MAGIC, LCD_CMD_INIT)
#define LCD_IOCTL_INIT_ALL                      _IO(LCD_MAGIC, LCD_CMD_INIT_ALL)
#define LCD_IOCTL_SETUP                         _IO(LCD_MAGIC, LCD_CMD_SETUP)	/* arg is int */
#define LCD_IOCTL_HOLD                          _IO(LCD_MAGIC, LCD_CMD_HOLD)	/* arg is int */
#define LCD_IOCTL_PULSE                         _IO(LCD_MAGIC, LCD_CMD_PULSE)	/* arg is int */
#define LCD_IOCTL_REG                           _IOW(LCD_MAGIC, LCD_CMD_REG, LCD_Reg_t)	/* arg is LCD_Reg_t * */
#define LCD_IOCTL_RECT                          _IOW(LCD_MAGIC, LCD_CMD_RECT, LCD_Rect_t)	/* arg is LCD_Rect_t * */
#define LCD_IOCTL_COLOR_TEST                    _IO(LCD_MAGIC, LCD_CMD_COLOR_TEST)	/* arg is int */
#define LCD_IOCTL_DIRTY_ROWS                    _IOW(LCD_MAGIC, LCD_CMD_DIRTY_ROWS, LCD_DirtyRows_t)	/* arg is LCD_DirtyRows_t * */
#define LCD_IOCTL_DIRTY_RECT                    _IOW(LCD_MAGIC, LCD_CMD_DIRTY_RECT, LCD_DirtyRect_t)	/* arg is LCD_DirtyRect_t * */
#define LCD_IOCTL_PRINT_REGS                    _IOW(LCD_MAGIC, LCD_CMD_PRINT_REGS, LCD_PrintRegs_t)	/* arg is LCD_PrintRegs_t * */
#define LCD_IOCTL_PRINT_DATA                    _IOW(LCD_MAGIC, LCD_CMD_PRINT_DATA, LCD_PrintData_t)	/* arg is LCD_PrintData_t * */
#define LCD_IOCTL_PWR_OFF                       _IO(LCD_MAGIC, LCD_CMD_PWR_OFF)
#define LCD_IOCTL_IS_DISPLAY_REGION_SUPPORTED   _IO(LCD_MAGIC, LCD_CMD_IS_DISPLAY_REGION_SUPPORTED)
#define LCD_IOCTL_INFO                          _IO(LCD_MAGIC, LCD_CMD_INFO)
#define LCD_IOCTL_IS_DIRTY_ROW_UPDATE_SUPPORTED _IO(LCD_MAGIC, LCD_CMD_IS_DIRTY_ROW_UPDATE_SUPPORTED)
#define LCD_IOCTL_SET_DISPLAY_LAYER_NUM         _IO(LCD_MAGIC, LCD_CMD_SET_DISPLAY_LAYER_NUM)	/* arg is int */
#define LCD_IOCTL_DIRTY_ROW_BITS                _IOW(LCD_MAGIC, LCD_CMD_DIRTY_ROW_BITS, LCD_DirtyRowBits_t)	/* arg is LCD_DirtyRowBits_t * */
#define LCD_IOCTL_COPYAREA                      _IOW(LCD_MAGIC, LCD_CMD_COPYAREA, struct fb_copyarea)
#define LCD_IOCTL_FILLRECT_COLOR                _IOW(LCD_MAGIC, LCD_CMD_FILLRECT_COLOR, LCD_FillRectColor_t)

/* ---- Variable Externs ------------------------------------------------- */

/* ---- Function Prototypes ---------------------------------------------- */
#ifdef __KERNEL__

int lcd_ioctl_dirty_row_bits(unsigned long arg);

void lcd_dirty_rows(LCD_DirtyRows_t *dirtyRows);
void lcd_dirty_rect(LCD_DirtyRect_t *dirtyRect);
void *lcd_get_framebuffer_addr(int *frame_size, dma_addr_t *dma_addr);
int lcd_is_display_regions_supported(void);
int lcd_is_dirty_row_update_supported(void);
void lcd_get_info(LCD_Info_t *lcdInfo);
void lcd_copyarea(const struct fb_copyarea *area);
void lcd_fillrect(LCD_FillRectColor_t *rect_c);
void lcd_set_power(int onOff);

#endif /* __KERNEL__ */

#endif /* BCM_LINUX_LCD_H */
