/*************************************************************************
* Copyright 2010  Broadcom Corporation
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2 (the GPL),
* available at http://www.broadcom.com/licenses/GPLv2.php, with the following
* added to such license:
* As a special exception, the copyright holders of this software give you
* permission to link this software with independent modules, and to copy and
* distribute the resulting executable under terms of your choice, provided that
* you also meet, for each linked independent module, the terms and conditions
* of the license of that module. An independent module is a module which is not
* derived from this software.  The special exception does not apply to any
* modifications of the software.
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*****************************************************************************/
/*
 * include/linux/i2c/lm8325.h
 *
 * Configuration for LM8325 keypad driver.
 */

#ifndef __LINUX_LM8325_H
#define __LINUX_LM8325_H

#include <linux/types.h>
#include <linux/i2c-kona.h>

/*Commands to be sent to LM8325 chip*/
#define LM8325_CMD_MFGCODE			0x80	// Manufacture Code Register
#define LM8325_CMD_SWREV			0x81	// Software Revision Register
#define LM8325_CMD_RSTCTRL			0x82	// System Reset Register
#define LM8325_CMD_RSTINTCLR		0x84	// Clear NO Init/Power-On Interrupt Register
#define LM8325_CMD_CLKMODE			0x88	// Clock Mode Register
#define LM8325_CMD_CLKCFG			0x89	// Clock Configuration Register
#define LM8325_CMD_CLKEN			0x8A	// Clock Enable Register
#define LM8325_CMD_AUTOSLP			0x8B	// Autosleep Enable Register
#define LM8325_CMD_AUTOSLPTIL		0x8C	// Autosleep Timer Register Low
#define LM8325_CMD_AUTOSLPTIH		0x8D	// Autosleep Timer Register High
#define LM8325_CMD_IRQST			0x91	// Global Interrupt Status Register
#define LM8325_CMD_IOCFG			0xA7	// IO Pin Mapping Configuration Register (word)
#define LM8325_CMD_IOPC0			0xAA	// Pull Resistor Configuration Register 0 (word)
#define LM8325_CMD_IOPC0H			0xAB	// Pull Resistor Configuration Register 0 High Byte
#define LM8325_CMD_IOPC1			0xAC	// Pull Resistor Configuration Register 1 (word)
#define LM8325_CMD_IOPC1H			0xAD	// Pull Resistor Configuration Register 1 High Byte
#define LM8325_CMD_IOPC2			0xAE	// Pull Resistor Configuration Register 2 (word)
#define LM8325_CMD_IOPC2H			0xAF	// Pull Resistor Configuration Register 2 Hight Byte
#define LM8325_CMD_KBDSETTLE		0x01	// Keypad Settle Time
#define LM8325_CMD_KBDBOUNCE		0x02	// Keypad Debounce Time
#define LM8325_CMD_KBDSIZE			0x03	// Keypad Size Configuration
#define LM8325_CMD_KBDDEDCFG0		0x04	// Keypad Dedicated Key 0
#define LM8325_CMD_KBDDEDCFG1		0x05	// Keypad Dedicated Key 1
#define LM8325_CMD_KBDRIS			0x06	// Keypad Raw Interrupt Status
#define LM8325_CMD_KBDMIS			0x07	// Keypad Masked Interrupt Status
#define LM8325_CMD_KBDIC			0x08	// Keypad Interrupt Clear
#define LM8325_CMD_KBDMSK			0x09	// Keypad Interrupt Mask
#define LM8325_CMD_KBDCODE0			0x0B	// Keypad Code 0
#define LM8325_CMD_KBDCODE1			0x0C	// Keypad Code 1
#define LM8325_CMD_KBDCODE2			0x0D	// Keypad Code 2
#define LM8325_CMD_KBDCODE3			0x0E	// Keypad Code 3
#define LM8325_CMD_EVTCODE			0x10	// Keypad Event Code

/*
 * Largest keycode that the chip can send, plus one,
 * so keys can be mapped directly at the index of the
 * lm8325 keycode instead of subtracting one.
 */
#define LM8325_KEYMAP_SIZE (0x88)

#define LM8325_MAX_DATA	8

struct lm8325_chip {
	struct mutex lock;
	struct i2c_client *client;
	struct work_struct work;
	struct input_dev *idev;
	unsigned kp_enabled:1;
	s16 keymap[LM8325_KEYMAP_SIZE];
	int size_x;
	int size_y;
	int debounce_time;
	int settle_time;
	int iocfg;
	int autosleep;
	int sleep_time;

	int err_flag;
	int error_count;
	int key_pressed;
};

struct lm8325_platform_data {
	struct i2c_slave_platform_data i2c_pdata;
	int debounce_time;	/* Time to watch for key bouncing, in ms. */
	int settle_time;	/* Idle time until sleep, in ms. */
	int dedicated_key_cfg;
	int iocfg;
	int autosleep;
	int sleep_time;

	int size_x;
	int size_y;
	int repeat:1;
	const s16 *keymap;

	char *name;		/* Device name. */
};

void __init lm8325_set_platform_data(struct lm8325_platform_data *pdata);

#endif /* __LINUX_LM8325_H */
