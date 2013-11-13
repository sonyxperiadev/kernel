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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/bcmpmu.h>

/*
* Register field values for regulator.
*/
#define LDO_NORMAL              0	/* FOR LDO and Switchers it is NORMAL (
					* NM/NM1 for SRs). */
#define LDO_STANDBY             1	/* FOR LDO and Swtichers it is STANDBY(
					* LPM for SRs ). */
#define LDO_OFF                 2	/* OFF. */
#define LDO_RESERVED_SR_NM2     3	/* For LDO it is reserved. For CSR,
					* IOSR, SDSR this is NM2 for SRs */
#define LDO_MODE_MASK			3
#define PM0_SHIFT		0
#define PM1_SHIFT		2
#define PM2_SHIFT		4
#define PM3_SHIFT		6

static int      bcmpmuldo_get_voltage(struct regulator_dev *rdev);
static int      bcmpmuldo_set_voltage(struct regulator_dev *rdev,
					int min_uv, int max_uv,
					unsigned *selector);
static int      bcmpmuldo_list_voltage(struct regulator_dev *rdev,
					unsigned selector);
static unsigned int bcmpmureg_get_mode(struct regulator_dev *rdev);
static int      bcmpmureg_set_mode(struct regulator_dev *rdev, unsigned mode);
static int      bcmpmureg_get_status(struct regulator_dev *rdev);
static int      bcmpmureg_disable(struct regulator_dev *rdev);
static int      bcmpmureg_enable(struct regulator_dev *rdev);
static int      bcmpmureg_is_enabled(struct regulator_dev *rdev);
static struct regulator_dev *regl[BCMPMU_REGULATOR_MAX];

/** voltage regulator details.  */
struct regulator_ops bcmpmuldo_ops = {
	.list_voltage = bcmpmuldo_list_voltage,
	.enable = bcmpmureg_enable,
	.disable = bcmpmureg_disable,
	.is_enabled = bcmpmureg_is_enabled,
	.get_mode = bcmpmureg_get_mode,
	.set_mode = bcmpmureg_set_mode,
	.get_status = bcmpmureg_get_status,
	.set_voltage = bcmpmuldo_set_voltage,
	.get_voltage = bcmpmuldo_get_voltage,
};

/*
* @is_enabled: Return 1 if the regulator is enabled, 0 if not.  May also
* return negative errno.
*/
static int bcmpmureg_is_enabled(struct regulator_dev *rdev)
{
	struct bcmpmu  *bcmpmu = rdev_get_drvdata(rdev);
	struct bcmpmu_reg_info *info = bcmpmu->rgltr_info + rdev_get_id(rdev);
	unsigned int    val;

	int             rc =
	bcmpmu->read_dev(bcmpmu, info->reg_addr, &val, info->mode_mask);

	if (rc < 0) {
		pr_info(KERN_ERR "%s:error reading regulator OPmode register\n",
		__func__);
		return rc;
	}
	val >>= PM1_SHIFT;
	return ((val & LDO_MODE_MASK) != LDO_OFF);
}

/*
* @enable: Configure the regulator as enabled.
*/
static int bcmpmureg_enable(struct regulator_dev *rdev)
{
	struct bcmpmu  *bcmpmu = rdev_get_drvdata(rdev);
	int             id = rdev_get_id(rdev);
	struct bcmpmu_reg_info *info = bcmpmu->rgltr_info + id;
	struct bcmpmu_regulator_init_data *bcmpmu_regulators =
			bcmpmu->pdata->regulator_init_data;
	struct bcmpmu_reg_map map;
	unsigned int    val;

	map = bcmpmu->regmap[info->reg_addr];
	if ((map.addr == 0) && (map.mask == 0))
		return -ENXIO;

	val = (bcmpmu_regulators + id)->default_opmode;
	/*For CSR, IOSR, SDSR and CSR though we register
	 *LPM and NM2 OPMODES with framework, OPMODE chage doesnot exist in
	 *reality
	 * */
	if (val == BCMPMU_REGU_OPMODE_DC)
		return 0;

	val &= ~((LDO_MODE_MASK << PM1_SHIFT) | (LDO_MODE_MASK << PM3_SHIFT));
	val |= ((LDO_NORMAL << PM1_SHIFT) | (LDO_NORMAL << PM3_SHIFT));

	switch ((bcmpmu_regulators + id)->dsm_mode) {
	case BCMPMU_REGL_OFF_IN_DSM:
	/*
	* clear LPM bits
	*/
	val &= ~((LDO_MODE_MASK << PM0_SHIFT) | (LDO_MODE_MASK << PM2_SHIFT));
	/*
	* set to off in DSM
	*/
	val |= ((LDO_OFF << PM0_SHIFT) | (LDO_OFF << PM2_SHIFT));
	break;
	case BCMPMU_REGL_LPM_IN_DSM:
	val &= ~((LDO_MODE_MASK << PM0_SHIFT) | (LDO_MODE_MASK << PM2_SHIFT));
	val |= ((LDO_STANDBY << PM0_SHIFT) | (LDO_STANDBY << PM2_SHIFT));
	break;
	case BCMPMU_REGL_ON_IN_DSM:
	/*
	* Regualtor should be on in DSM also
	*/
	val &= ~((LDO_MODE_MASK << PM0_SHIFT) | (LDO_MODE_MASK << PM2_SHIFT));
	val |= ((LDO_NORMAL << PM0_SHIFT) | (LDO_NORMAL << PM2_SHIFT));
	break;

	default:
	break;
	}
	pr_debug("%s: id = %d reg_addr = %x reg_val = %x\n", __func__, id,
		info->reg_addr, val);
	return bcmpmu->write_dev_bulk(bcmpmu, map.map, map.addr, &val, 1);

}

/*
* @disable: Configure the regulator as disabled.
*/
static int bcmpmureg_disable(struct regulator_dev *rdev)
{
	struct bcmpmu  *bcmpmu = rdev_get_drvdata(rdev);
	struct bcmpmu_reg_info *info = bcmpmu->rgltr_info + rdev_get_id(rdev);
	struct bcmpmu_reg_map map = bcmpmu->regmap[info->reg_addr];

	int rc = LDO_OFF << PM0_SHIFT | LDO_OFF << PM1_SHIFT |
		LDO_OFF << PM2_SHIFT | LDO_OFF << PM3_SHIFT;

	if ((map.addr == 0) && (map.mask == 0))
		return -ENXIO;
	return bcmpmu->write_dev_bulk(bcmpmu, map.map, map.addr, &rc, 1);
}

/*
* @get_status: Return actual (not as-configured) status of regulator, as a
* REGULATOR_STATUS value (or negative errno)
*/
static int bcmpmureg_get_status(struct regulator_dev *rdev)
{
	struct bcmpmu  *bcmpmu = rdev_get_drvdata(rdev);
	struct bcmpmu_reg_info *info = bcmpmu->rgltr_info + rdev_get_id(rdev);
	unsigned int    ldo_or_sr = info->ldo_or_sr;
	unsigned int    val;

	int             rc =
	bcmpmu->read_dev(bcmpmu, info->reg_addr, &val, info->mode_mask);

	if (rc < 0) {
		pr_info(KERN_ERR "%s: error reading regulator OPmode register.\n",
		__func__);
		return rc;
	}

	/*
	* Read the status from PM1 - PC1 will be high AP is active
	*/
	val = (val >> PM1_SHIFT) & LDO_MODE_MASK;

	switch (val) {
	case LDO_NORMAL:
		return REGULATOR_STATUS_NORMAL;
	case LDO_STANDBY:
		return REGULATOR_STATUS_STANDBY;
	case LDO_OFF:
		return REGULATOR_STATUS_OFF;
	case LDO_RESERVED_SR_NM2:
		if (ldo_or_sr == BCMPMU_SR)
			return REGULATOR_STATUS_FAST;
	else
		return -EINVAL;
	default:
		return -EINVAL;
	}
}

/*
* @get_mode: Get the configured operating mode for the regulator.
*/
static unsigned int bcmpmureg_get_mode(struct regulator_dev *rdev)
{
	struct bcmpmu  *bcmpmu = rdev_get_drvdata(rdev);
	struct bcmpmu_reg_info *info = bcmpmu->rgltr_info + rdev_get_id(rdev);

	unsigned int    ldo_or_sr = info->ldo_or_sr;
	unsigned int    val;

	int             rc =
	bcmpmu->read_dev(bcmpmu, info->reg_addr, &val, info->mode_mask);

	if (rc < 0) {
		pr_info(KERN_ERR "%s: error reading regulator OPmode register.\n",
			__func__);
		return rc;
	}

	/*
	* Read the mode from PM1 - PC1 will be high AP is active
	*/
	val = (val >> PM1_SHIFT) & LDO_MODE_MASK;

	switch (val) {
	case LDO_NORMAL:
	return REGULATOR_MODE_NORMAL;
	case LDO_STANDBY:
	return REGULATOR_MODE_STANDBY;
	case LDO_OFF:
	return -EINVAL;
	case LDO_RESERVED_SR_NM2:
	if (ldo_or_sr == BCMPMU_SR)
		return REGULATOR_MODE_FAST;
	else
		return -EINVAL;
	default:
	return -EINVAL;
	}
}

/*
* @set_mode: Set the configured operating mode for the regulator.
*/
static int bcmpmureg_set_mode(struct regulator_dev *rdev, unsigned mode)
{
	struct bcmpmu  *bcmpmu = rdev_get_drvdata(rdev);
	struct bcmpmu_reg_info *info = bcmpmu->rgltr_info + rdev_get_id(rdev);
	unsigned int    ldo_or_sr = info->ldo_or_sr;
	unsigned int    val;
	int             opmode;
	int             rc =
	bcmpmu->read_dev(bcmpmu, info->reg_addr, &val, info->mode_mask);
	if (rc < 0) {
		pr_info(KERN_ERR "%s: error reading regulator OPmode register.\n",
			__func__);
	return rc;
	}


	switch (mode) {
	case REGULATOR_MODE_NORMAL:
	opmode = LDO_NORMAL;
	break;
	case REGULATOR_MODE_STANDBY:
	opmode = LDO_STANDBY;
	break;
	case REGULATOR_MODE_FAST:
	if (ldo_or_sr == BCMPMU_SR)
		opmode = LDO_RESERVED_SR_NM2;
	else
		return -EINVAL;
	break;
	default:
	return -EINVAL;
	}
	opmode = (opmode & LDO_MODE_MASK) << PM1_SHIFT;

	val &= ~(LDO_MODE_MASK << PM1_SHIFT);
	val |= opmode;

	return bcmpmu->write_dev(bcmpmu, info->reg_addr, val, info->mode_mask);
}

/*
* @list_voltage: Return one of the supported voltages, in microvolts; zero if
* the selector indicates a voltage that is unusable on this system; or
* negative errno.  Selectors range from zero to one less than
* regulator_desc.n_voltages.  Voltages may be reported in any order.
*/

static int bcmpmuldo_list_voltage(struct regulator_dev *rdev, unsigned selector)
{
	struct bcmpmu  *bcmpmu = rdev_get_drvdata(rdev);
	struct bcmpmu_reg_info *info = bcmpmu->rgltr_info + rdev_get_id(rdev);
	if (selector >= info->num_voltages)
		return -EINVAL;
	return info->v_table[selector];
}

/*
* @set_voltage: Set the voltage for the regulator within the range specified.
* The driver should select the voltage closest to min_uV.
*/
static int bcmpmuldo_set_voltage(struct regulator_dev *rdev, int min_uv,
				int max_uv, unsigned *selector)
{
	struct bcmpmu  *bcmpmu = rdev_get_drvdata(rdev);
	struct bcmpmu_reg_info *info = bcmpmu->rgltr_info + rdev_get_id(rdev);
	unsigned int    addr = 0;
	int             rc;
	*selector = -1;

	for (rc = 0; rc < info->num_voltages; rc++) {
		int uv = info->v_table[rc];

		if ((min_uv <= uv) && (uv <= max_uv)) {
			addr = info->reg_addr_volt;
			*selector = rc;
			return bcmpmu->write_dev(bcmpmu, addr, rc,
				info->vout_mask);
		}
	}
	return -EDOM;
}

static int bcmpmuldo_get_voltage(struct regulator_dev *rdev)
{
	struct bcmpmu  *bcmpmu = rdev_get_drvdata(rdev);
	struct bcmpmu_reg_info *info = bcmpmu->rgltr_info + rdev_get_id(rdev);
	int             rc = 0;
	unsigned int    val;

	rc = bcmpmu->read_dev(bcmpmu, info->reg_addr_volt, &val,
			info->vout_mask);
	if (rc < 0) {
		pr_info(KERN_ERR "%s: error reading regulator voltage register.\n",
			__func__);
		return rc;
	}
	rc = (val >> info->vout_shift) & (info->vout_mask);
	return (info->v_table[rc]);
}

static int bcmpmu_regulator_probe(struct platform_device *pdev)
{
	int             i;
	struct bcmpmu  *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_regulator_init_data *bcmpmu_regulators =
	bcmpmu->pdata->regulator_init_data;
	int             num_of_regl = bcmpmu->pdata->num_of_regl;
	int             regl_id,
					ret = 0;
	u8              opmode;
	struct bcmpmu_reg_info *reg_info;

	pr_info(KERN_INFO "%s: called\n", __func__);
	/*
	* register regulator
	*/
	bcmpmu->rgltr_desc = bcmpmu_rgltr_desc(bcmpmu);
	bcmpmu->rgltr_info = bcmpmu_rgltr_info(bcmpmu);
	if ((bcmpmu->rgltr_info == NULL) || (bcmpmu->rgltr_desc == NULL))
		pr_info(KERN_ERR "%s: regulator info and desc not avail.\n",
			__func__);

	for (i = 0; i < num_of_regl; i++) {
		regl_id = (bcmpmu_regulators + i)->regulator;
		opmode = (bcmpmu_regulators + i)->default_opmode;
		reg_info = &bcmpmu->rgltr_info[regl_id];


	if ((bcmpmu_regulators + i)->initdata) {

		if (opmode != 0xFF) {
			ret =
			bcmpmu->write_dev(bcmpmu, reg_info->reg_addr, opmode,
				reg_info->mode_mask);
			if (ret != 0)
				goto register_fail;
		}

		pr_info("%s: REGULATOR name %s, ID %d def_opmode = %x initdata = %x\n ",
		__func__, (bcmpmu->rgltr_desc + regl_id)->name, regl_id, opmode,
		(u32) ((bcmpmu_regulators + i)->initdata));

		regl[i] = regulator_register(&bcmpmu->rgltr_desc[regl_id],
					&pdev->dev,
					(bcmpmu_regulators + i)->initdata,
					bcmpmu, NULL);
		if (IS_ERR(regl[i])) {
			dev_err(&pdev->dev, "failed to register %s\n",
				(bcmpmu->rgltr_desc + regl_id)->name);
			ret = PTR_ERR(regl[i]);
			goto register_fail;
		}
	} else
		regl[i] = NULL;

	}
	regulator_has_full_constraints();

	return ret;
register_fail:
	while (i-- > 0) {
		if (regl[i])
			regulator_unregister(regl[i]);
	}
	bcmpmu->rgltr_desc = NULL;
	bcmpmu->rgltr_info = NULL;
	return ret;
}

static int bcmpmu_regulator_remove(struct platform_device *pdev)
{
	struct bcmpmu  *bcmpmu = platform_get_drvdata(pdev);
	int             i = 0;
	while (i < bcmpmu->pdata->num_of_regl) {
		if (regl[i]) {
			regulator_unregister(regl[i]);
			kzfree(regl[i]);
		}
		i++;
	}
	return 0;
}

static struct platform_driver bcmpmu_regulator_driver = {
	.probe = bcmpmu_regulator_probe,
	.remove = bcmpmu_regulator_remove,
	.driver = {
		.name = "bcmpmu-regulator",
		},
};

static int __init bcmpmu_regulator_init(void)
{
	return platform_driver_register(&bcmpmu_regulator_driver);
}
subsys_initcall(bcmpmu_regulator_init);

static void __exit bcmpmu_regulator_exit(void)
{
	platform_driver_unregister(&bcmpmu_regulator_driver);
}
module_exit(bcmpmu_regulator_exit);

/*
* Module information
*/
MODULE_DESCRIPTION("BCMPMU regulator driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:bcmpmu-regulator");
