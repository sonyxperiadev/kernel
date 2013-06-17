/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/misc/bcm59055-fuelgauge.c
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
*  bcm59055-fuelgauge.c
*
*  PURPOSE:
*
*     This implements the driver for the Fuel Gauge on BCM59055 PMU chip.
*
*  NOTES:
*
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/power_supply.h>
#include <linux/workqueue.h>
#include <mach/irqs.h>
#include <linux/stringify.h>
#include <linux/broadcom/bcm59055-fuelgauge.h>


struct bcm59055_fg {
	struct bcm590xx *bcm59055;
	struct mutex lock;
};
static struct bcm59055_fg *bcm59055_fg;

int bcm59055_fg_enable(void)
{
	struct bcm590xx *bcm59055 = bcm59055_fg->bcm59055;
	u8 reg;
	int ret;
	pr_debug("Inside %s\n", __func__);
	mutex_lock(&bcm59055_fg->lock);
	reg = bcm590xx_reg_read(bcm59055, BCM59055_REG_FGCTRL1);
	reg |= FGHOSTEN;
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_FGCTRL1, reg);
	if (ret)
		pr_info("%s: Error enabling FG\n", __func__);
	mutex_unlock(&bcm59055_fg->lock);
	return ret;
}
EXPORT_SYMBOL(bcm59055_fg_enable);

int bcm59055_fg_disable(void)
{
	struct bcm590xx *bcm59055 = bcm59055_fg->bcm59055;
	u8 reg;
	int ret;
	pr_debug("Inside %s\n", __func__);
	mutex_lock(&bcm59055_fg->lock);
	reg = bcm590xx_reg_read(bcm59055, BCM59055_REG_FGCTRL1);
	reg &= ~FGHOSTEN;
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_FGCTRL1, reg);
	if (ret)
		pr_info("%s: Error disabling FG\n", __func__);
	mutex_unlock(&bcm59055_fg->lock);
	return ret;
}
EXPORT_SYMBOL(bcm59055_fg_disable);

int bcm59055_fg_set_cont_mode(void)
{
	struct bcm590xx *bcm59055 = bcm59055_fg->bcm59055;
	u8 reg;
	int ret;
	pr_debug("Inside %s\n", __func__);
	mutex_lock(&bcm59055_fg->lock);
	reg = bcm590xx_reg_read(bcm59055, BCM59055_REG_FGOPMODCTRL);
	reg &= ~FGSYNCMODE;
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_FGOPMODCTRL, reg);
	mutex_unlock(&bcm59055_fg->lock);
	return ret;
}
EXPORT_SYMBOL(bcm59055_fg_set_cont_mode);

int bcm59055_fg_set_sync_mode(bool modulator_on)
{
	struct bcm590xx *bcm59055 = bcm59055_fg->bcm59055;
	u8 reg;
	int ret;
	pr_debug("Inside %s\n", __func__);
	mutex_lock(&bcm59055_fg->lock);
	reg = bcm590xx_reg_read(bcm59055, BCM59055_REG_FGOPMODCTRL);
	/* Set the PC2 PC1 combination..FG should on only if PC1 = 1 */
	reg |= (FGOPMODCRTL1 | FGOPMODCRTL3);
	reg &= ~(FGOPMODCRTL0 | FGOPMODCRTL2);
	reg |= FGSYNCMODE;		/* change the mode */
	if (modulator_on)
		reg |= FGMODON;
	else
		reg &= ~FGMODON;
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_FGOPMODCTRL, reg);
	mutex_unlock(&bcm59055_fg->lock);
	return ret;
}
EXPORT_SYMBOL(bcm59055_fg_set_sync_mode);

int bcm59055_fg_enable_modulator(bool enable)
{
	struct bcm590xx *bcm59055 = bcm59055_fg->bcm59055;
	u8 reg;
	int ret;
	pr_debug("Inside %s\n", __func__);

	mutex_lock(&bcm59055_fg->lock);
	reg = bcm590xx_reg_read(bcm59055, BCM59055_REG_FGOPMODCTRL);

	if (enable)
		reg |= FGMODON;
	else
		reg &= ~FGMODON;
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_FGOPMODCTRL, reg);
	mutex_unlock(&bcm59055_fg->lock);
	return ret;
}
EXPORT_SYMBOL(bcm59055_fg_enable_modulator);

int bcm59055_fg_offset_cal(bool longcal)
{
	struct bcm590xx *bcm59055 = bcm59055_fg->bcm59055;
	u8 reg, calbit;
	int ret;
	pr_debug("Inside %s\n", __func__);
	mutex_lock(&bcm59055_fg->lock);
	if (longcal)
		calbit = LONGCAL;
	else
		calbit = FGCAL;
	reg = bcm590xx_reg_read(bcm59055, BCM59055_REG_FGCTRL2);
	reg |= calbit;
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_FGOPMODCTRL, reg);
	while (reg & calbit) {
		pr_info("%s: Calibration is in process\n", __func__);
		reg = bcm590xx_reg_read(bcm59055, BCM59055_REG_FGCTRL2);
	}
	mutex_unlock(&bcm59055_fg->lock);
	return ret;
}
EXPORT_SYMBOL(bcm59055_fg_offset_cal);

int bcm59055_fg_1point_cal(void)
{
	struct bcm590xx *bcm59055 = bcm59055_fg->bcm59055;
	u8 reg;
	int ret;
	pr_debug("Inside %s\n", __func__);

	mutex_lock(&bcm59055_fg->lock);
	reg = bcm590xx_reg_read(bcm59055, BCM59055_REG_FGCTRL2);
	reg |= FG1PTCAL;
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_FGOPMODCTRL, reg);
	while (reg & FG1PTCAL) {
		pr_info("%s: 1 point Calibration is in process\n", __func__);
		reg = bcm590xx_reg_read(bcm59055, BCM59055_REG_FGCTRL2);
	}
	mutex_lock(&bcm59055_fg->lock);
	return ret;
}
EXPORT_SYMBOL(bcm59055_fg_1point_cal);

int bcm59055_fg_force_cal(void)
{
	struct bcm590xx *bcm59055 = bcm59055_fg->bcm59055;
	u8 reg;
	int ret;
	pr_debug("Inside %s\n", __func__);
	mutex_lock(&bcm59055_fg->lock);

	reg = bcm590xx_reg_read(bcm59055, BCM59055_REG_FGCTRL2);
	reg |= FGFORCECAL;
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_FGOPMODCTRL, reg);
	while (reg & FGFORCECAL) {
		pr_info("%s: Force Calibration is in process\n", __func__);
		reg = bcm590xx_reg_read(bcm59055, BCM59055_REG_FGCTRL2);
	}
	mutex_unlock(&bcm59055_fg->lock);
	return ret;
}
EXPORT_SYMBOL(bcm59055_fg_force_cal);

int bcm59055_fg_set_comb_rate(int rate)
{
	struct bcm590xx *bcm59055 = bcm59055_fg->bcm59055;
	u8 reg;
	int ret;
	pr_debug("Inside %s\n", __func__);
	if (rate < FG_COMB_RATE_2HZ || rate > FG_COMB_RATE_16HZ) {
		pr_info("%s: Invalid rate\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&bcm59055_fg->lock);
	reg = bcm590xx_reg_read(bcm59055, BCM59055_REG_FGOCICCTRL1);
	reg &= ~FG_COMB_RATE_MASK;
	reg |= rate;
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_FGOCICCTRL1, reg);
	mutex_unlock(&bcm59055_fg->lock);
	return ret;
}
EXPORT_SYMBOL(bcm59055_fg_set_comb_rate);

int bcm59055_fg_init_read(void)
{
	struct bcm590xx *bcm59055 = bcm59055_fg->bcm59055;
	u8 reg;
	int ret;
	pr_debug("Inside %s\n", __func__);
	mutex_lock(&bcm59055_fg->lock);
	reg = bcm590xx_reg_read(bcm59055, BCM59055_REG_FGCTRL2);
	reg |= FGFRZREAD;
	pr_debug("%s: writing %x to FGCTRL2 register\n", __func__, reg);
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_FGCTRL2, reg);
	udelay(2);
	mutex_unlock(&bcm59055_fg->lock);
	return ret;
}
EXPORT_SYMBOL(bcm59055_fg_init_read);

#define SOC_READ_BYTE_MAX	8
int bcm59055_fg_read_soc(u32 *fg_accm, u16 *fg_cnt, u16 *fg_sleep_cnt)
{
	struct bcm590xx *bcm59055 = bcm59055_fg->bcm59055;
	u8 reg[SOC_READ_BYTE_MAX];
	int ret=0;
	pr_debug("Inside %s\n", __func__);

	ret = bcm590xx_mul_reg_read(bcm59055, BCM59055_REG_FGACCM1,
			SOC_READ_BYTE_MAX, (u8 *)reg);
	if (!(reg[0] & FGRDVALID)) {
		pr_debug("%s: Accumulator value is invalid..try later\n", __func__);
		return -EINVAL;
	}
	*fg_accm = ((reg[0] << 24) | (reg[1] << 16) | (reg[2] << 8) | reg[3]);
	*fg_cnt = (reg[4] << 8) | reg[5];
	*fg_sleep_cnt = (reg[6] << 8) | reg[7];
	pr_debug("%s: Accm 0x%x, Count 0x%x, Sleep Count 0x%x\n", __func__,
			*fg_accm, *fg_cnt, *fg_sleep_cnt);
	return ret;
}
EXPORT_SYMBOL(bcm59055_fg_read_soc);

int bcm59055_fg_reset(void)
{
	struct bcm590xx *bcm59055 = bcm59055_fg->bcm59055;
	u8 reg;
	int ret;
	pr_debug("Inside %s\n", __func__);

	mutex_lock(&bcm59055_fg->lock);
	reg = bcm590xx_reg_read(bcm59055, BCM59055_REG_FGCTRL2);
	reg |= FGRESET;
	pr_debug("%s: writing %x to FGCTRL2 register\n", __func__, reg);
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_FGCTRL2, reg);
	mutex_unlock(&bcm59055_fg->lock);
	return ret;
}
EXPORT_SYMBOL(bcm59055_fg_reset);

int bcm59055_fg_read_sample(enum fg_smpl_type type, s16 * val)
{
	struct bcm590xx *bcm59055 = bcm59055_fg->bcm59055;
	u8 reg[2];
	int add = 0;
	int ret = 0;
	pr_debug("Inside %s\n", __func__);

	switch (type) {
	case fg_smpl_acc:
		add = BCM59055_REG_FGSMPL1;
		break;
	case fg_smpl_cal:
		add = BCM59055_REG_FGSMPL3;
		break;
	case fg_smpl_raw:
		add = BCM59055_REG_FGSMPL5;
		break;
	}

	ret = bcm590xx_mul_reg_read(bcm59055, add, 2, reg);
	pr_debug ("%s: ret %d, MSB %x, LSB %x", __func__, ret, reg[0], reg[1]);
	if (ret < 0)
		return ret;
	*val = reg[1] | (reg[0] << 8);
	return ret;
}
EXPORT_SYMBOL(bcm59055_fg_read_sample);

int bcm59055_fg_set_fgfrzsmpl(void)
{
	struct bcm590xx *bcm59055 = bcm59055_fg->bcm59055;
	u8 reg;
	int ret;
	pr_debug("Inside %s\n", __func__);
	mutex_lock(&bcm59055_fg->lock);

	reg = bcm590xx_reg_read(bcm59055, BCM59055_REG_FGCTRL2);
	reg |= FGFRZSMPL;
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_FGCTRL2, reg);
	mutex_unlock(&bcm59055_fg->lock);
	return ret;
}
EXPORT_SYMBOL(bcm59055_fg_set_fgfrzsmpl);

int bcm59055_fg_write_gain_trim(u8 gain)
{
	struct bcm590xx *bcm59055 = bcm59055_fg->bcm59055;
	pr_debug("Inside %s\n", __func__);

	return bcm590xx_reg_write(bcm59055, BCM59055_REG_FGGAIN_TRIM, gain);
}
EXPORT_SYMBOL(bcm59055_fg_write_gain_trim);

int bcm59055_fg_write_offset_trim(u8 offset)
{
	struct bcm590xx *bcm59055 = bcm59055_fg->bcm59055;
	pr_debug("Inside %s\n", __func__);

	return bcm590xx_reg_write(bcm59055, BCM59055_REG_FGOFFSET_TRIM, offset);
}
EXPORT_SYMBOL(bcm59055_fg_write_offset_trim);

int bcm59055_fg_read_offset(s16 *offset)
{
	int ret = 0;
	u8 pmu_offset [2];
	struct bcm590xx *bcm59055 = bcm59055_fg->bcm59055;
	pr_debug("Inside %s\n", __func__);

	ret = bcm590xx_mul_reg_read(bcm59055, BCM59055_REG_FGOFFSET1, 2, pmu_offset);
	pr_debug ("%s: ret %d, MSB %x, LSB %x\n", __func__, ret, pmu_offset[0], pmu_offset[1]);
	if (ret >= 0) {
		*offset = pmu_offset [1] | (pmu_offset [0] << 8);
	}
	return ret;
}
EXPORT_SYMBOL(bcm59055_fg_read_offset);

static int __devinit bcm59055_fg_probe(struct platform_device *pdev)
{
	struct bcm590xx *bcm59055 = dev_get_drvdata(pdev->dev.parent);
	struct bcm59055_fg *priv_data;

	pr_info("BCM59055 Fuel Gauge Driver\n");
	priv_data = kzalloc(sizeof(struct bcm59055_fg ), GFP_KERNEL);
	if (!priv_data) {
		pr_info("%s: Memory can not be allocated!!\n",
			__func__);
		return -ENOMEM;
	}
	priv_data->bcm59055 = bcm59055;
	mutex_init(&priv_data->lock);
	bcm59055_fg = priv_data;
	return 0;
}

static int __devexit bcm59055_fg_remove(struct platform_device *pdev)
{
	struct bcm59055_fg *priv_data = platform_get_drvdata(pdev);
	kfree(priv_data);
	return 0;
}

struct platform_driver fg_driver = {
	.probe = bcm59055_fg_probe,
	.remove = __devexit_p(bcm59055_fg_remove),
	.driver = {
		   .name = "bcm590xx-fg",
		   }
};

/****************************************************************************
*
*  bcm59055_fg_init
*
*     Called to perform module initialization when the module is loaded
*
***************************************************************************/

static int __init bcm59055_fg_init(void)
{
	platform_driver_register(&fg_driver);
	/* initialize semaphore for ADC access control */
	return 0;
}				/* bcm59055_fg_init */

/****************************************************************************
*
*  bcm59055_fg_exit
*
*       Called to perform module cleanup when the module is unloaded.
*
***************************************************************************/

static void __exit bcm59055_fg_exit(void)
{
	platform_driver_unregister(&fg_driver);

}				/* bcm59055_fg_exit */

subsys_initcall(bcm59055_fg_init);
module_exit(bcm59055_fg_exit);

MODULE_AUTHOR("TKG");
MODULE_DESCRIPTION("BCM59055 FUEL GAUGE Driver");
