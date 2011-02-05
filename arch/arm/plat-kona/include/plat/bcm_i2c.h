/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/plat-bcmap/include/plat/bcm_i2c.h
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
*  bcm_i2c.h
*
*  PURPOSE:
*
*     This file contains definitions for the I2C Registers and platform data
*     structures.
*
*  NOTES:
*
*****************************************************************************/

#if !defined(__PLAT_BCM_I2C_H)
#define __PLAT_BCM_I2C_H

/* ---- Include Files ---------------------------------------------------- */

#include <mach/hardware.h>
#define __REG32(x)   (*((volatile u32 *)(x)))
#define __REG16(x)   (*((volatile u16 *)(x)))
#define __REG8(x) (*((volatile u8  *)(x)))

/* ---- Constants and Types ---------------------------------------------- */

#ifdef __ARMEB__

#define REG_I2C_CS(BASE)          __REG8(BASE + 0x00 + 3)	/* I2C Control and Status Register */
#define REG_I2C_TIM(BASE)         __REG8(BASE + 0x04 + 3)	/* I2C Timing Register */
#define REG_I2C_DAT(BASE)         __REG8(BASE + 0x08 + 3)	/* I2C Data Register */
#define REG_I2C_TOUT(BASE)        __REG16(BASE + 0x0c + 3)	/* I2C Timeout Register */
#define REG_I2C_RCM(BASE)         __REG8(BASE + 0x10 + 3)	/* I2C CRC Main Register */
#define REG_I2C_RCP(BASE)         __REG8(BASE + 0x14 + 3)	/* I2C CRC Polynomial Register */
#define REG_I2C_RCD(BASE)         __REG8(BASE + 0x18 + 3)	/* I2C CRC Data Register */
#define REG_I2C_FCR(BASE)         __REG8(BASE + 0x1c + 3)	/* I2C FIFO Control Register */
#define REG_I2C_FIFORDOUT(BASE)   __REG8(BASE + 0x20 + 3)	/* I2C FIFO Read Out Register */
#define REG_I2C_IER(BASE)         __REG8(BASE + 0x24 + 3)	/* I2C FIFO Interrupt Enable Register */
#define REG_I2C_ISR(BASE)         __REG8(BASE + 0x28 + 3)	/* I2C FIFO Interrupt Status Register */
#define REG_I2C_CLKEN(BASE)       __REG8(BASE + 0x2c + 3)	/* I2C clock enable */
#define REG_I2C_SFTRST(BASE)      __REG8(BASE + 0x40 + 3)	/* I2C SW reset */
#else

#define REG_I2C_CS(BASE)          __REG8(BASE + 0x00)	/* I2C Control and Status Register */
#define REG_I2C_TIM(BASE)         __REG8(BASE + 0x04)	/* I2C Timing Register */
#define REG_I2C_DAT(BASE)         __REG8(BASE + 0x08)	/* I2C Data Register */
#define REG_I2C_TOUT(BASE)        __REG16(BASE + 0x0c)	/* I2C Timeout Register */
#define REG_I2C_RCM(BASE)         __REG8(BASE + 0x10)	/* I2C CRC Main Register */
#define REG_I2C_RCP(BASE)         __REG8(BASE + 0x14)	/* I2C CRC Polynomial Register */
#define REG_I2C_RCD(BASE)         __REG8(BASE + 0x18)	/* I2C CRC Data Register */
#define REG_I2C_FCR(BASE)         __REG8(BASE + 0x1c)	/* I2C FIFO Control Register */
#define REG_I2C_FIFORDOUT(BASE)   __REG8(BASE + 0x20)	/* I2C FIFO Read Out Register */
#define REG_I2C_IER(BASE)         __REG8(BASE + 0x24)	/* I2C FIFO Interrupt Enable Register */
#define REG_I2C_ISR(BASE)         __REG8(BASE + 0x28)	/* I2C FIFO Interrupt Status Register */
#define REG_I2C_CLKEN(BASE)       __REG8(BASE + 0x2c)	/* I2C clock enable */
#ifndef CONFIG_ARCH_BCM215XX
#define REG_I2C_TOUTH(BASE)	  __REG8(BASE + 0x30)
#endif
#define REG_I2C_SFTRST(BASE)      __REG8(BASE + 0x40)   /* I2C SW reset */
#endif

/* REG_I2C_CCS bits */
#define REG_I2C_CS_EN           0x01	/* 1:Enable I2C hardware 0:Reset I2C hardware */
#define REG_I2C_CS_CMDMASK      0x06	/* bits 2:1 */
#define REG_I2C_CS_CMDNULL      0x00	/* No immediate action */
#define REG_I2C_CS_CMDSTART     0x02	/* Generate start condition */
#define REG_I2C_CS_CMDSTOP      0x04	/* Generate stop condition */
#define REG_I2C_CS_CMDREAD      0x06	/* Read a byte */
#define REG_I2C_CS_ACK          0x08	/* ACK bit */
					    /* Write: CMD=11 ACK to send out after I2C byte read */
					    /*        CMD=01 0: Normal start 1: Repeated start */
					    /* Read: ACK returned from last I2C byte write */
#define REG_I2C_CS_SCL          0x40	/* I2C clock Write:Set SCL pin value when EN=0 Read:SCL pin value */
#define REG_I2C_CS_SDA          0x80	/* I2C data  Write:Set SDA pin value when EN=0 Read:SDA pin value */
#define REG_I2C_CS_CMDRESTART   (REG_I2C_CS_CMDSTART | REG_I2C_CS_ACK)

/* REG_I2C_TIM bits */
/* --- I2C speed. Master clock is 13MHz */
#define REG_I2C_TIM_DIVMSK      0x03	/* clock division bits */
#define REG_I2C_TIM_DIV16       0	/* MCLK /16 */
#define REG_I2C_TIM_DIV8        1	/* MCLK /8 */
#define REG_I2C_TIM_DIV4        2	/* MCLK /4 */
#define REG_I2C_TIM_DIV2        03	/* MCLK /2 */
#define REG_I2C_TIM_PMSK        0x38	/* # cycles for SCL, high=2*p + 1, low=2*p + 2 */
#define REG_I2C_TIM_P3          (3<<3)
#define REG_I2C_TIM_P4          (4<<3)
#define REG_I2C_TIM_P6          (6<<3)
#define REG_I2C_TIM_P7          (7<<3)

/* --- SCL speed */
/* --- I2CTIM_P bits = 111 group */
#define REG_I2C_CLK_26K     (REG_I2C_TIM_DIV16 | REG_I2C_TIM_P7)
#define REG_I2C_CLK_52K     (REG_I2C_TIM_DIV8 | REG_I2C_TIM_P7)
#define REG_I2C_CLK_105K    (REG_I2C_TIM_DIV4 | REG_I2C_TIM_P7)
#define REG_I2C_CLK_210K    (REG_I2C_TIM_DIV2 | REG_I2C_TIM_P7)

/* I2CTIM_P bits = 110 group */
#define REG_I2C_CLK_30K     (REG_I2C_TIM_DIV16 | REG_I2C_TIM_P6)
#define REG_I2C_CLK_60K     (REG_I2C_TIM_DIV8 | REG_I2C_TIM_P6)
#define REG_I2C_CLK_120K    (REG_I2C_TIM_DIV4 | REG_I2C_TIM_P6)
#define REG_I2C_CLK_240K    (REG_I2C_TIM_DIV2 | REG_I2C_TIM_P6)

/* I2CTIM_P bits = 100 group */
#define REG_I2C_CLK_43K     (REG_I2C_TIM_DIV16 | REG_I2C_TIM_P4)
#define REG_I2C_CLK_85K     (REG_I2C_TIM_DIV8 | REG_I2C_TIM_P4)
#define REG_I2C_CLK_171K    (REG_I2C_TIM_DIV4 | REG_I2C_TIM_P4)
#define REG_I2C_CLK_342K    (REG_I2C_TIM_DIV2 | REG_I2C_TIM_P4)

/* I2CTIM_P bits = 011 group */
#define REG_I2C_CLK_54K     (REG_I2C_TIM_DIV16 | REG_I2C_TIM_P3)
#define REG_I2C_CLK_108K    (REG_I2C_TIM_DIV8 | REG_I2C_TIM_P3)
#define REG_I2C_CLK_217K    (REG_I2C_TIM_DIV4 | REG_I2C_TIM_P3)
#define REG_I2C_CLK_433K    (REG_I2C_TIM_DIV2 | REG_I2C_TIM_P3)

/* REG_I2C_I2CTOUT bits */
#define REG_I2C_TOUT_EN             0x80	/* Enables the Timeout counter */

/* REG_I2C_RCM bits */
#define REG_I2C_RCM_EN              0x01	/* enable CRC generation */
#define REG_I2C_RCM_LSB0            0x02	/* =1, Force LSB to be 0 */

/* REG_I2C_FCR bits */
#define REG_I2C_FCR_FLUSH           0x80	/* Writing 1 will flush the Tx FIFO; Write Only */
#define REG_I2C_FCR_EN              0x40	/* FIFO Enable */
#define REG_I2C_FCR_CNTMASK         0x07	/* FIFO Data Counter; Read Only */

/* REG_I2C_IER bits */
#define REG_I2C_IER_INT_EN          0x08	/* 1: Enable I2C busy status interrupt */
#define REG_I2C_IER_ERRINT_EN       0x04	/* 1: Enable I2C error status interrupt */
#define REG_I2C_IER_FIFOINT_EN      0x02	/* 1: Enable I2C Tx FIFO empty interrupt */
#define REG_I2C_IER_NOACK_EN        0x01	/* 1: Enable I2C No Acknowledge interrupt */

/* REG_I2C_ISR bits */
#define REG_I2C_ISR_CMD_BUSY        0x80	/* Command busy */
#define REG_I2C_ISR_SES_DONE        0x08	/* Read 0: I2C session busy 1: I2C session done */
						/* Write 0: No action 1: Clear the interrupt */
#define REG_I2C_ISR_I2CERR          0x04	/* I2C Bus Error Status */
#define REG_I2C_ISR_TXFIFOEMPTY     0x02	/* I2C Tx FIFO Empty */
#define REG_I2C_ISR_NOACK           0x01	/* I2C No Acknowledge interrupt */

/* REG_I2C_CLKEN bits */
#define REG_I2C_CLKEN_CLKEN         0x01	/* 1: Enable I2C clock */
#define REG_I2C_CLKEN_AUTOSENSE_EN  0x80

#ifdef CONFIG_ARCH_BCM215XX
#define I2C_MM_HS_TIM_PRESCALE_CMD_DIV8 0
#define I2C_MM_HS_TIM_PRESCALE_CMD_DIV4 1
#define I2C_MM_HS_TIM_PRESCALE_CMD_DIV2 2
#define I2C_MM_HS_TIM_PRESCALE_CMD_NODIV 3

#define CLK_BASE_ADDR	IO_ADDRESS(0x08140000)
#define CLK_HSBSC_MODE (CLK_BASE_ADDR + 0x000001A0)
#define CLK_HSBSC2_MODE (CLK_BASE_ADDR + 0x000001A4)
#define CLK_HSBSC_ENABLE (CLK_BASE_ADDR + 0x000001B8)
#define CLK_HSBSC2_ENABLE (CLK_BASE_ADDR + 0x000001BC)
#endif

/* i2c_host_platform_data
 * This structure contains the platform data required by the i2c controller.
 */
struct i2c_host_platform_data {
/* Update the OEB control for SDA and SCL pads.
 * id = I2C adapter ID, val = bit value.
 */
	void (*set_oeb)(int id, bool val);
	int retries;
};

typedef enum {
	I2C_SPD_32K = 0,
	I2C_SPD_50K,
	I2C_SPD_100K,
	I2C_SPD_220K,
	I2C_SPD_360K,
	I2C_SPD_400K,
	I2C_SPD_430K,
	I2C_SPD_HS,
	I2C_SPD_MAXIMUM,
	I2C_SPD_UNKNOWN,
} i2c_speed_t;

/* i2c_slave_platform_data
 * This structure contains the platform data for a particular slave.
 */
struct i2c_slave_platform_data {
/* The datarate at which the I2C transaction are
 * supposed to happen for that particular slave.
 */
	i2c_speed_t i2c_spd;
};

#endif /* __PLAT_BCM_I2C_H */
