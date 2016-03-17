/*****************************************************************************
*  Copyright 2001 - 2007 Broadcom Corporation.  All rights reserved.
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

#ifndef __LCD_COMMON_H_
#define __LCD_COMMON_H_

#include <linux/broadcom/lcd.h>
#include <cfg_global.h>
/* #include <hw_cfg.h> */

/* #define DEBUG */

#ifdef DEBUG
#define LCD_DEBUG(fmt, args...) printk("[%s]: " fmt, __FUNCTION__ , ##args)
#else
#define LCD_DEBUG(fmt, args...)
#endif

#define LCD_PUTS(str)	LCD_DEBUG("%s\n", str)

typedef enum {
	LCD_main_panel = 0,	/* must match device minor number */
	LCD_sub_panel = 1	/* must match device minor number */
} LCD_panel_t;

/* from lcd_drv.h */

/** LCD Color
*/
typedef enum {
	LCD_COLOR_BLACK = 0x0000,	/* /< lcd black pixel */
	LCD_COLOR_GREEN = 0x07E0,	/* /< lcd green pixel */
	LCD_COLOR_RED = 0xF800,	/* /< lcd red pixel */
	LCD_COLOR_BLUE = 0x001F,	/* /< lcd blue pixel */
	LCD_COLOR_YELLOW = 0xFFE0,	/* /< lcd yellow pixel */
	LCD_COLOR_CYAN = 0xF81F,	/* /< lcd cyan pixel */
	LCD_COLOR_DDD = 0x07FF,	/* /< lcd ddd pixel */
	LCD_COLOR_WHITE = 0xFFFF	/* /< lcd white pixel */
} LcdColor_t;

/* LCD interface type, IOCR0[29:28] */
typedef enum {
	LCD_Z80,
	LCD_M68,
	LCD_SPI,
	LCD_ETM,
	LCD_MSPRO,
	LCD_SD2
} LCD_Intf_t;

/* LCD voltage, IOCR2[30] */
typedef enum {
	LCD_18V,
	LCD_30V
} LCD_Volt_t;

/* LCD bus width */
typedef enum {
	LCD_16BIT = 16,
	LCD_18BIT = 18
} LCD_Bus_t;

typedef void (*lcd_DMAEND_CBF) (void);

/* replace macros with funcs, or some side-effects cause problems */
void lcd_write_cmd(uint32_t cmd);
void lcd_write_data(uint32_t data);
void lcd_write_param(uint32_t cmd);

#define	WRITE_LCD_CMD(cmd)	lcd_write_cmd(cmd)
#define	WRITE_LCD_DATA(data)	lcd_write_data(data)
#define WRITE_LCD_PARAM(param)	lcd_write_param(param)

/* end from lcd_drv.h */

#ifndef LCD_WTR_HOLD
#ifdef CONFIG_BCM_LCD_HX8347_TFT1P5158
#define LCD_WTR_HOLD	0x00040000
#else
#define LCD_WTR_HOLD	0x00010000
#endif
#endif

#ifndef LCD_WTR_PULSE
#ifdef CONFIG_BCM_LCD_HX8347_TFT1P5158
#define LCD_WTR_PULSE	0x00000400
#else
#define LCD_WTR_PULSE	0x00000400	/* 100ns @ MSP_CLK 78MHZ */
#endif
#endif

#ifndef LCD_WTR_SETUP
#define LCD_WTR_SETUP	0x00000001
#endif

#define GPIO_LCD_RESET	24
#define GPIO_LCD_BACKLIGHT	17

#define LCD_Reset_GPIO		GPIO_LCD_RESET
#define LCD_Backlight_GPIO	GPIO_LCD_BACKLIGHT
#define LCD_Write_Timing 	(LCD_WTR_HOLD | LCD_WTR_PULSE | LCD_WTR_SETUP)

#ifdef LCD_RTR_HOLD
#define LCD_Read_Timing 	(LCD_RTR_HOLD | LCD_RTR_PULSE | LCD_RTR_SETUP)
#else
#define LCD_Read_Timing 	LCD_Write_Timing
#endif

#define FB_HEIGHT 320

/* RGB - 565 */
#define RED_MASK    0xF800
#define GREEN_MASK  0x07C0
#define BLUE_MASK   0x001F

#define RED_SHIFT   11
#define GREEN_SHIFT 5
#define BLUE_SHIFT  0

#define RGB_VAL(r, g, b)  (((r) << RED_SHIFT) | ((g) << GREEN_SHIFT) | ((b) << BLUE_SHIFT))

/* desired DMA channel: 1 is second highest priority */
#define DESIRED_DMA_CHANNEL 1

#define DMA_CONTROL                             \
	(REG_DMA_CHAN_CTL_SRC_INCR             \
	| REG_DMA_CHAN_CTL_DEST_WIDTH_32        \
	| REG_DMA_CHAN_CTL_SRC_WIDTH_32         \
	| REG_DMA_CHAN_CTL_DEST_BURST_SIZE_4    \
	| REG_DMA_CHAN_CTL_SRC_BURST_SIZE_32)

#define DMA_DIVIDE_WIDTH	DMA_DIVIDE_WIDTH_32	/* 32-bit transfers */

#define DMA_CONFIG                                  \
	(REG_DMA_CHAN_CFG_TC_INT_ENABLE            \
	| REG_DMA_CHAN_CFG_ERROR_INT_ENABLE         \
	| REG_DMA_CHAN_CFG_FLOW_CTL_MEM_TO_PRF_DMA \
	| (REG_DMA_PERIPHERAL_LCD << REG_DMA_CHAN_CFG_DEST_PERIPHERAL_SHIFT) \
	| REG_DMA_CHAN_CFG_ENABLE)

#define BYTES_PER_PIXEL  2	/* always 16 bits per pixel, so two bytes */

/* #define USE_DMA 0 */
/* #define USE_PARTIAL_UPDATE */
#ifndef USE_DMA
#define USE_DMA	1
#endif

#if USE_DMA

/* definition of DMA Linked List Item (LLI) */
typedef struct {
	u32 source;		/* source address */
	u32 dest;		/* dest address */
	u32 link;		/* link to next LLI */
	u32 control;		/* control word */
} DMA_LLI_t;

/* DMA buffer */
typedef struct {
	size_t sizeInBytes;
	void *virtPtr;
	dma_addr_t physPtr;
} DMA_Buffer_t;

/* structure for restoring overwritten LLI information */
typedef struct {
	int valid;		/* flag to show data is valid */
	int row;		/* row to store */
	u32 link;		/* link register */
	u32 control;		/* control register */
	DMA_LLI_t *list;	/* pointer to start of list (main vs sub) */
} DMA_Restore_t;

/* swap from big endian to little endian for 32 bits */
#define DMA_SWAP(val) 	cpu_to_le32(val)
#define DMA_BYTES_PER_LLI   16	/* bytes per Linked List Item (LLI) */
#define DMA_DIVIDE_WIDTH_32	4	/* 4 bytes for 32-bit-wide transfer */
#define DMA_DIVIDE_WIDTH_16	2	/* 2 bytes for 16-bit-wide transfer */
#define DMA_DIVIDE_WIDTH_8 	1	/* 1 byte  for 8-bit-wide transfer */
#define NUM_DMA_LLI_BUFFERS	1
#define NUM_DMA_BUFFERS		2

#endif /* USE_DMA */

typedef struct {
	size_t sizeInBytes;
	void *virtPtr;
	dma_addr_t physPtr;
} LCD_FrameBuffer_t;

/* info for each LCD panel - some global vars moved into here */
typedef struct lcd_dev_info_t {
	LCD_panel_t panel;	/* hint  - when panels require different cmds */
	char *LCD_panel_name;

	int height;
	int width;
	int bits_per_pixel;
	uint32_t row_start;
	uint32_t row_end;
	uint32_t col_start;
	uint32_t col_end;

	LCD_FrameBuffer_t frame_buffer;
	LCD_DirtyRect_t dirty_rect;

#if USE_DMA
	DMA_Buffer_t dma_linked_list;
#endif
} LCD_dev_info_t;

void lcd_select_panel(LCD_panel_t panel);

/* any panel-specific stuff */
extern LCD_Intf_t LCD_Intf;
extern LCD_Volt_t LCD_Volt;
extern LCD_Bus_t LCD_Bus;
extern int LCD_num_panels;
extern LCD_dev_info_t LCD_device[];
void lcd_init_panels(void);
void lcd_poweroff_panels(void);
void lcd_ResetStartAddr(LCD_dev_info_t *dev);
void lcd_setup_for_data(LCD_dev_info_t *dev);

#endif /* __LCD_COMMON_H_ */
