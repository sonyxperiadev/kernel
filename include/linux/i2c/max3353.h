/*
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by  the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the  GNU General Public License along
 * with this program; if not, write  to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef MAX3353_H
#define MAX3353_H

#include <linux/proc_fs.h>

/* MAX3353 registers:*/

/* Manufacturer and ID Registers (read only) */
#define	MAX3353_MANUFACTURER_0					(0x00)
#define	MAX3353_MANUFACTURER_1					(0x01)
#define	MAX3353_MANUFACTURER_2					(0x02)
#define	MAX3353_MANUFACTURER_3					(0x03)
#define	MAX3353_MANUFACTURER_0_VALUE				(0x6A)
#define	MAX3353_MANUFACTURER_1_VALUE				(0x0B)
#define	MAX3353_MANUFACTURER_2_VALUE				(0x53)
#define	MAX3353_MANUFACTURER_3_VALUE				(0x33)

#define	MAX3353_PRODUCT_ID_0					(0x04)
#define	MAX3353_PRODUCT_ID_1					(0x05)
#define	MAX3353_PRODUCT_ID_2					(0x06)
#define	MAX3353_PRODUCT_ID_3					(0x07)
#define	MAX3353_PRODUCT_ID_0_VALUE				(0x48)
#define	MAX3353_PRODUCT_ID_1_VALUE				(0x5A)
#define	MAX3353_PRODUCT_ID_2_VALUE				(0x42)
#define	MAX3353_PRODUCT_ID_3_VALUE				(0x00)

/* Control Registers */
#define	MAX3353_CONTROL_1					(0x10)
#	define	MAX3353_CONTROL_1_IRQ_MODE			(1<<1)
#	define	MAX3353_CONTROL_1_BDISC_ACONN			(1<<2)
#	define	MAX3353_CONTROL_1_DP_PULLUP			(1<<4)
#	define	MAX3353_CONTROL_1_DM_PULLUP			(1<<5)
#	define	MAX3353_CONTROL_1_DP_PULLDWN			(1<<6)
#	define	MAX3353_CONTROL_1_DM_PULLDWN			(1<<7)
#define	MAX3353_CONTROL_2					(0x11)
#	define	MAX3353_CONTROL_2_SDWN				(1<<0)
#	define	MAX3353_CONTROL_2_VBUS_CHG1			(1<<1)
#	define	MAX3353_CONTROL_2_VBUS_CHG2			(1<<2)
#	define	MAX3353_CONTROL_2_VBUS_DRV			(1<<3)
#	define	MAX3353_CONTROL_2_VBUS_DISCHG			(1<<4)

/* Status Register */
#define	MAX3353_STATUS						(0x13)
#	define	MAX3353_STATUS_VBUS_VALID			(1<<0)
#	define	MAX3353_STATUS_SESSION_VALID			(1<<1)
#	define	MAX3353_STATUS_SESSION_END			(1<<2)
#	define	MAX3353_STATUS_SESSION_ID_GND			(1<<3)
#	define	MAX3353_STATUS_SESSION_ID_FLOAT			(1<<4)
#	define	MAX3353_STATUS_SESSION_A_HNP			(1<<5)
#	define	MAX3353_STATUS_SESSION_B_HNP			(1<<6)

/* Interrupt Registers */
#define	MAX3353_INTERRUPT_MASK					(0x14)
#	define	MAX3353_INTERRUPT_MASK_VBUS_VALID_EN		(1<<0)
#	define	MAX3353_INTERRUPT_MASK_SESSION_VALID_EN		(1<<1)
#	define	MAX3353_INTERRUPT_MASK_SESSION_END_EN		(1<<2)
#	define	MAX3353_INTERRUPT_MASK_ID_GND_EN		(1<<3)
#	define	MAX3353_INTERRUPT_MASK_ID_FLOAT_EN		(1<<4)
#	define	MAX3353_INTERRUPT_MASK_A_HNP_EN			(1<<5)
#define	MAX3353_INTERRUPT_EDGE					(0x15)
#	define	MAX3353_INTERRUPT_EDGE_VBUS_VALID_ED		(1<<0)
#	define	MAX3353_INTERRUPT_EDGE_SESSION_VALID_ED		(1<<1)
#define	MAX3353_INTERRUPT_LATCH					(0x16)
#	define	MAX3353_INTERRUPT_LATCH_VBUS_VALID_RP		(1<<0)
#	define	MAX3353_INTERRUPT_LATCH_SESSION_VALID_RP	(1<<1)
#	define	MAX3353_INTERRUPT_LATCH_VBUS_VALID_RN		(1<<2)
#	define	MAX3353_INTERRUPT_LATCH_SESSION_VALID_RN	(1<<3)
#	define	MAX3353_INTERRUPT_LATCH_SESSION_END_RQ		(1<<4)
#	define	MAX3353_INTERRUPT_LATCH_ID_GND_RQ		(1<<5)
#	define	MAX3353_INTERRUPT_LATCH_ID_FLOAT_RQ		(1<<6)
#	define	MAX3353_INTERRUPT_LATCH_A_HNP_RQ		(1<<7)

#define MAX3353_DRIVER_NAME			"max3353"

#define MAX3353_I2C_MAX_DEVICES			2
#define MAX3353_I2C_ADDR_BASE			0x2c	/* b'0101100 */
#define MAX3353_I2C_ADDR_MAX			(MAX3353_I2C_ADDR_BASE + MAX3353_I2C_MAX_DEVICES - 1)

#define MAX3353_MODE_OTG			0
#define MAX3353_MODE_HOST			1
#define MAX3353_MODE_DEVICE			2
#define MAX3353_MODE_STR_INIT			{"0-OTG", "1-Host", "2-Device"}

#define MAX3353_PROC_GLOBAL_PARENT_DIR    	"driver/" MAX3353_DRIVER_NAME
#define MAX3353_PROC_ENTRY_MODE           	"mode"
#define MAX3353_PROC_ENTRY_REG            	"reg"

struct max3353_platform_data {
	int id;			/* I2C BUS id for MAX3353 device instance */
	int irq_gpio_num;	/* GPIO number attached to MAX3353 INT line */
	int mode;		/* Usually MAX3353_MODE_OTG */
};

#endif /* #ifndef MAX3353_H */
