/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#ifndef __LINUX_MFD_BCM590XX_PMIC_H
#define __LINUX_MFD_BCM590XX_PMIC_H

#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/userspace-consumer.h>
#include <linux/mfd/bcm590xx/bcm59055_A0.h>

/* This is PMIC header definition.
 * For new PMIC, this header must be updated
 */

/* Number of Slaves in PMIC */
#define BCM590XX_NUM_SLAVES			2
#define BCM590XX_SLAVE1_I2C_ADDRESS		0x08
#define BCM590XX_SLAVE2_I2C_ADDRESS		0x0C

#define BCM590XX_REG_ENCODE(reg, slave)		((slave << 8) | reg)
#define BCM590XX_REG_DECODE(val)		(val & 0xFF)
#define BCM590XX_REG_SLAVE(val)			(val >> 8)



#define BCM590XX_REG_PMUID				BCM59055_REG_PMUID
#define BCM590XX_INT_MASK_BIT			1
#define BCM590XX_MAX_INT_REGS			14
#define BCM590XX_MAX_INTMASK_REGS		14

#define BCM590XX_INT_REG_BASE			BCM59055_REG_INT1
#define BCM590XX_INT_MASK_REG_BASE		BCM59055_REG_INT1MSK
#define BCM590XX_TOTAL_IRQ			BCM59055_TOTAL_IRQ
#define BCM590XX_REG_HOSTCTRL1			BCM59055_REG_HOSTCTRL1
#define HOSTCTRL1_SHDWN_OFFSET			BCM59055_REG_HOSTCTRL1_SHDWN_OFFSET
#define HOSTCTRL1_WDOGEN_OFFSET 		BCM59055_HSCTRL1_SYS_WDT_EN_OFF

// For a new PMU add new header file here.
#define BCM590XX_MAX_REGULATOR			13

// Register field values for regulator.
#define LDO_NORMAL				0   // FOR LDO and Switchers it is NORMAL ( NM/NM1 for SRs).
#define LDO_STANDBY				1   // FOR LDO and Swtichers it is STANDBY( LPM for SRs ).
#define LDO_OFF					2   // OFF.
#define LDO_RESERVED_SR_FAST			3   // For LDO it is reserved. For CSR, IOSR, SDSR this is NM2 for SRs

#define VOLTAGE_ADC_MAX_SAMPLE          6

/* wake reason */
#define BCM590XX_REG_WAKE_REASON		BCM59055_REG_ENV8

enum {
	BCM590XX_CGPD_WAKE	= 1 << 0,
	BCM590XX_UBPD_WAKE	= 1 << 1,
	BCM590XX_RTC_ALARM_WAKE	= 1 << 2,
	BCM590XX_AUXON_WAKE	= 1 << 3,
	BCM590XX_POK_WAKE	= 1 << 4,
	BCM590XX_GBAT_PLUGIN	= 1 << 5
};
/*regualtor DSM settings */
enum {
	BCM590XX_REGL_LPM_IN_DSM,   /*if enabled, LPM in DSM (PC1 = 0)*/
	BCM590XX_REGL_OFF_IN_DSM, /*if enabled, off in DSM (PC1 = 0)*/
	BCM590XX_REGL_ON_IN_DSM,   /*if enabled, ON in DSM (PC1 = 0)*/
};



/* LDO or Switcher def */
#define BCM590XX_LDO				0x10
#define BCM590XX_SR				0x11


struct bcm590xx;
struct platform_device;
struct regulator_init_data;
struct bcm590xx_regulator_init_data;

enum {
	BCM590XX_USE_REGULATORS   		=	(1 << 0),
	BCM590XX_USE_RTC          		=	(1 << 1),
	BCM590XX_USE_POWER        		=	(1 << 2),
	BCM590XX_USE_PONKEY 			=	(1 << 3),
	BCM590XX_ENABLE_DVS       		=	(1 << 4),
	BCM590XX_REGISTER_POWER_OFF		=	(1 << 5),
	BCM590XX_ENABLE_AUDIO			=	(1 << 6),
	BCM590XX_ENABLE_ADC				=	(1 << 7),
	BCM590XX_ENABLE_FUELGAUGE		=	(1 << 8),
	BCM590XX_ENABLE_POWER			=	(1 << 9),
	BCM590XX_ENABLE_USB_OTG			=	(1 << 10),
};

int bcm590xx_register_regulator(struct bcm590xx *bcm590xx, int reg,
			      struct regulator_init_data *initdata);

struct bcm590xx_reg_info
{
	u32 reg_addr;      /* address of regulator control register for mode control */
	u32 reg_addr_volt; /* address of control register to change voltage */
	u8 dsm;
	u32 vout_mask;     /* Mask of bits in register */
	u32 vout_shift;    /* Bit shift in register */
	u32 *v_table;      /* Map for converting register voltage to register value */
	u32 num_voltages;  /* Size of register map */
};

struct bcm590xx_regulator_init_data
{
	int regulator; /* Regulator ID */
	struct regulator_init_data   *initdata;
	u8 dsm;
};

struct bcm590xx_regulator_pdata {
	int num_regulator;
	struct bcm590xx_regulator_init_data *init;
	u8 default_pmmode[BCM590XX_MAX_REGULATOR];
	int csr_nm_volt;
	int csr_lpm_volt;
	int csr_turbo_volt;
};

struct mv_percent
{
    unsigned int mv;
    unsigned int percentage;
};


struct bcm590xx_battery_pdata {
    u8 eoc_current;
	int usb_cc;
	int wac_cc;
    u16 temp_low_limit;
    u16 temp_high_limit;

    u16 batt_min_volt;
    u16 batt_max_volt;
    /* this is to passed in unit of Coloumb */
    u16 batt_max_capacity;
    u8 batt_vol[VOLTAGE_ADC_MAX_SAMPLE];
    u8 batt_adc[VOLTAGE_ADC_MAX_SAMPLE];

    u8 batt_technology;
};

struct bcm590xx_audio_pdata {
	bool classAB;
	bool i2cmethod;
	int hsgain;
	int hsinputmode;
	int sc_thold;
	int ihf_gain;
	bool ihf_bypass_en;
};

struct bcm_pmu_irq {
    struct list_head node;
    void (*handler) (int, void *);
    void *data;
    int irq;
    bool irq_enabled;
};

// Needed for assignment in bcm59055_A0.c
extern struct regulator_ops bcm590xxldo_ops;
#endif
