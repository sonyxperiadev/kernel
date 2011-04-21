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




#define BCM590XX_INT_MASK_BIT			1
#define BCM590XX_MAX_INT_REGS			14
#define BCM590XX_MAX_INTMASK_REGS		14

#define BCM590XX_INT_REG_BASE			BCM59055_REG_INT1
#define BCM590XX_INT_MASK_REG_BASE		BCM59055_REG_INT1MSK
#define BCM590XX_TOTAL_IRQ			BCM59055_TOTAL_IRQ

// For a new PMU add new header file here.

#define NUM_BCM590XX_REGULATORS			15

// Register field values for regulator.
#define LDO_NORMAL				0   // FOR LDO and Switchers it is NORMAL ( NM/NM1 for SRs).
#define LDO_STANDBY				1   // FOR LDO and Swtichers it is STANDBY( LPM for SRs ).
#define LDO_OFF					2   // OFF.
#define LDO_RESERVED_SR_IDLE			3   // For LDO it is reserved. For CSR, IOSR, SDSR this is NM2 for SRs

/* LDO or Switcher def */
#define BCM590XX_LDO				0x10
#define BCM590XX_SR				0x11

/* LDO PMMode on PC2/PC1 values */
#define OFF_PC2PC1_00				(10 << 0)
#define ON_PC2PC1_00				(00 << 0)
#define LPM_PC2PC1_00				(01 << 0)
#define OFF_PC2PC1_01				(10 << 2)
#define ON_PC2PC1_01				(00 << 2)
#define LPM_PC2PC1_01				(01 << 2)
#define OFF_PC2PC1_10				(10 << 4)
#define ON_PC2PC1_10				(00 << 4)
#define LPM_PC2PC1_10				(01 << 4)


struct bcm590xx;
struct platform_device;
struct regulator_init_data;
struct bcm590xx_regulator_init_data;

enum {
	BCM590XX_USE_REGULATORS   			=  (1 << 0),
	BCM590XX_USE_RTC          			=  (1 << 1),
	BCM590XX_USE_POWER        			=  (1 << 2),
	BCM590XX_USE_PONKEY 				=  (1 << 3),
	BCM590XX_ENABLE_DVS       			=  (1 << 4),
	BCM590XX_REGISTER_POWER_OFF			=  (1 << 5),
	BCM590XX_ENABLE_AUDIO				=  (1 << 6),
};

struct bcm590xx_pmic {
	/* regulator devices */
	struct platform_device *pdev[NUM_BCM590XX_REGULATORS];
};

int bcm590xx_register_regulator(struct bcm590xx *bcm590xx, int reg,
			      struct regulator_init_data *initdata);

struct bcm590xx_reg_info
{
	u32 reg_addr;      /* address of regulator control register for mode control */
	u32 reg_addr_volt; /* address of control register to change voltage */
	u32 en_dis_mask ;  /* Mask for enable/disable bits */
	u32 en_dis_shift ; /* Shift for enable/disalbe bits */
	u32 vout_mask;     /* Mask of bits in register */
	u32 vout_shift;    /* Bit shift in register */
	u32 *v_table;      /* Map for converting register voltage to register value */
	u32 num_voltages;  /* Size of register map */
	u32 mode ;
};

struct bcm590xx_regulator_init_data
{
	int regulator ; /* Regulator ID */
	struct regulator_init_data   *initdata ;
	u8 pm_mode_flag;
} ;

struct mv_percent   
{
    unsigned int mv ;
    unsigned int percentage ;
} ; 


struct bcm590xx_battery_pdata {
    // struct charger_info usb;
    // struct charger_info wac;
    u8 eoc_current;

    u8 volt_adc_channel;
    u8 temp_adc_channel;
    u8 batt_level_count;
    // struct batt_level_table *batt_level_table;
    struct mv_percent *vp_table ;
    unsigned int vp_table_cnt ;

    u16 temp_low_limit;
    u16 temp_high_limit;

    u16 batt_min_volt;
    u16 batt_max_volt;

    u8 batt_technology;
    int (*can_start_charging)(void*);
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
extern struct regulator_ops bcm590xxldo_ops ;
#endif
