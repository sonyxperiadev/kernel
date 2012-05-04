/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
*
* File Name		: stmpe1801.c
* Authors		: Sensor & MicroActuators BU - Application Team
*			    : Sivakumar SD (sivakumar.sd@st.com)
* Version		: V 1.0 
* Date			: 04/03/2011
* Description	: STMPE1801
*
********************************************************************************
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* THE PRESENT SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES
* OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, FOR THE SOLE
* PURPOSE TO SUPPORT YOUR APPLICATION DEVELOPMENT.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* THIS SOFTWARE IS SPECIFICALLY DESIGNED FOR EXCLUSIVE USE WITH ST PARTS.
*
********************************************************************************
* REVISON HISTORY
*
* VERSION | DATE 	| AUTHORS	     | DESCRIPTION
*
* 1.0	  | 04/03/2011 | Sivakumar SD    | First Release
*
*******************************************************************************/

#ifndef __LINUX_INPUT_STMPE1801_H
#define __LINUX_INPUT_STMPE1801_H

#include <linux/device.h>



enum stmpe_block {
        STMPE_BLOCK_GPIO        = 1 << 0,
        STMPE_BLOCK_KEYPAD      = 1 << 1,
        STMPE_BLOCK_TOUCHSCREEN = 1 << 2,
        STMPE_BLOCK_ADC         = 1 << 3,
        STMPE_BLOCK_PWM         = 1 << 4,
        STMPE_BLOCK_ROTATOR     = 1 << 5,
};

enum stmpe_partnum {
        STMPE1801,
};

/*
 * For registers whose locations differ on variants,  the correct address is
 * obtained by indexing stmpe->regs with one of the following.
 */
enum {
        STMPE_IDX_CHIP_ID,
        STMPE_IDX_ICR_LSB,
        STMPE_IDX_IER_LSB,
        STMPE_IDX_ISR_MSB,
        STMPE_IDX_GPMR_LSB,
        STMPE_IDX_GPSR_LSB,
        STMPE_IDX_GPCR_LSB,
        STMPE_IDX_GPDR_LSB,
        STMPE_IDX_GPEDR_MSB,
        STMPE_IDX_GPRER_LSB,
        STMPE_IDX_GPFER_LSB,
        STMPE_IDX_GPAFR_U_MSB,
        STMPE_IDX_IEGPIOR_LSB,
        STMPE_IDX_ISGPIOR_MSB,
        STMPE_IDX_MAX,
};


struct stmpe_variant_info;

/**
 * struct stmpe - STMPE MFD structure
 * @lock: lock protecting I/O operations
 * @irq_lock: IRQ bus lock
 * @dev: device, mostly for dev_dbg()
 * @i2c: i2c client
 * @variant: the detected STMPE model number
 * @regs: list of addresses of registers which are at different addresses on
 *        different variants.  Indexed by one of STMPE_IDX_*.
 * @irq_base: starting IRQ number for internal IRQs
 * @num_gpios: number of gpios, differs for variants
 * @ier: cache of IER registers for bus_lock
 * @oldier: cache of IER registers for bus_lock
 * @pdata: platform data
 */
struct stmpe {
        struct mutex lock;
        struct mutex irq_lock;
        struct device *dev;
        struct i2c_client *i2c;
        enum stmpe_partnum partnum;
        struct stmpe_variant_info *variant;
        const u8 *regs;

        int irq_base;
        int num_gpios;
        u8 ier[2];
        u8 oldier[2];
        struct stmpe_platform_data *pdata;
};

extern int stmpe_reg_write(struct stmpe *stmpe, u8 reg, u8 data);
extern int stmpe_reg_read(struct stmpe *stmpe, u8 reg);
extern int stmpe_block_read(struct stmpe *stmpe, u8 reg, u8 length,
                            u8 *values);
extern int stmpe_block_write(struct stmpe *stmpe, u8 reg, u8 length,
                             const u8 *values);
extern int stmpe_set_bits(struct stmpe *stmpe, u8 reg, u8 mask, u8 val);
extern int stmpe_set_altfunc(struct stmpe *stmpe, u32 pins,
                             enum stmpe_block block);
extern int stmpe_enable(struct stmpe *stmpe, unsigned int blocks);
extern int stmpe_disable(struct stmpe *stmpe, unsigned int blocks);

struct matrix_keymap_data;

/**
 * struct stmpe_keypad_platform_data - STMPE keypad platform data
 * @keymap_data: key map table and size
 * @debounce_ms: debounce interval, in ms.  Maximum is
 *               %STMPE_KEYPAD_MAX_DEBOUNCE.
 * @scan_count: number of key scanning cycles to confirm key data.
 *              Maximum is %STMPE_KEYPAD_MAX_SCAN_COUNT.
 * @no_autorepeat: disable key autorepeat
 */
struct stmpe_keypad_platform_data {
        struct matrix_keymap_data *keymap_data;
        unsigned int debounce_ms;
        unsigned int scan_count;
        bool no_autorepeat;
};

/**
 * struct stmpe_gpio_platform_data - STMPE GPIO platform data
 * @gpio_base: first gpio number assigned.  A maximum of
 *             %STMPE_NR_GPIOS GPIOs will be allocated.
 */
struct stmpe_gpio_platform_data {
        int gpio_base;
        void (*setup)(struct stmpe *stmpe, unsigned gpio_base);
        void (*remove)(struct stmpe *stmpe, unsigned gpio_base);
};



/**
 * struct stmpe_platform_data - STMPE platform data
 * @id: device id to distinguish between multiple STMPEs on the same board
 * @blocks: bitmask of blocks to enable (use STMPE_BLOCK_*)
 * @irq_trigger: IRQ trigger to use for the interrupt to the host
 * @irq_invert_polarity: IRQ line is connected with reversed polarity
 * @autosleep: bool to enable/disable stmpe autosleep
 * @autosleep_timeout: inactivity timeout in milliseconds for autosleep
 * @irq_base: base IRQ number.  %STMPE_NR_IRQS irqs will be used, or
 *            %STMPE_NR_INTERNAL_IRQS if the GPIO driver is not used.
 * @gpio: GPIO-specific platform data
 * @keypad: keypad-specific platform data
 * @ts: touchscreen-specific platform data
 */
struct stmpe_platform_data {
        int id;
        unsigned int blocks;
        int irq_base;
        unsigned int irq_trigger;
        bool irq_invert_polarity;
        bool autosleep;
        int autosleep_timeout;

        struct stmpe_gpio_platform_data *gpio;
        struct stmpe_keypad_platform_data *keypad;
        struct stmpe_ts_platform_data *ts;
};

#define STMPE_NR_INTERNAL_IRQS  9
#define STMPE_INT_GPIO(x)       (STMPE_NR_INTERNAL_IRQS + (x))

#define STMPE_NR_GPIOS          24
#define STMPE_NR_IRQS           STMPE_INT_GPIO(STMPE_NR_GPIOS)

#endif
