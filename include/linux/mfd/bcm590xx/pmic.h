/*
 * pmic.h  --  Power Managment Driver for Wolfson BCM590XX PMIC
 *
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#ifndef __LINUX_MFD_BCM590XX_PMIC_H
#define __LINUX_MFD_BCM590XX_PMIC_H

#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/regulator/machine.h>

#ifdef CONFIG_REGULATOR_BCM_PMU59055_A0
#include <linux/mfd/bcm590xx/bcm59055_A0.h>
#endif
// For a new PMU add new header file here.


#define NUM_BCM590XX_REGULATORS			15

struct bcm590xx;
struct platform_device;
struct regulator_init_data;


struct bcm590xx_pmic {
	/* regulator devices */
	struct platform_device *pdev[NUM_BCM590XX_REGULATORS];
};

int bcm590xx_register_regulator(struct bcm590xx *bcm590xx, int reg,
			      struct regulator_init_data *initdata);

struct bcm590xx_reg_info 
{
   u8  reg_addr;      /* address of regulator control register for mode control */
   u8  reg_addr_volt; /* address of control register to change voltage */
   u32 vout_mask;     /* Mask of bits in register */
   u32 vout_shift;    /* Bit shift in register */
   u32 *v_table; /* Map for converting register voltage to register value */
   u32 num_voltages;      /* Size of register map */
} ;

// Needed for assignment in bcm59055_A0.c
extern struct regulator_ops bcm590xxldo_ops ;
#endif
