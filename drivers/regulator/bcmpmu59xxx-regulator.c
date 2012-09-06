/*****************************************************************************
*  Copyright 2001 - 2012 Broadcom Corporation.  All rights reserved.
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
#include <linux/bug.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/bcmpmu59xxx.h>

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
#define LDO_MODE_MASK		3
#define PM0_SHIFT		0
#define PM1_SHIFT		2
#define PM2_SHIFT		4
#define PM3_SHIFT		6


#ifdef CONFIG_DEBUG_FS
void *debugfs_rgltr_dir;
#endif
static int bcmpmuldo_get_voltage(struct regulator_dev *rdev);
static int bcmpmuldo_set_voltage(struct regulator_dev *rdev,
				 int min_uv, int max_uv, unsigned *selector);
static int bcmpmuldo_list_voltage(struct regulator_dev *rdev,
				  unsigned selector);
static unsigned int bcmpmureg_get_mode(struct regulator_dev *rdev);
static int bcmpmureg_set_mode(struct regulator_dev *rdev, unsigned mode);
static int bcmpmureg_get_status(struct regulator_dev *rdev);
static int bcmpmureg_disable(struct regulator_dev *rdev);
static int bcmpmureg_enable(struct regulator_dev *rdev);
static int bcmpmureg_is_enabled(struct regulator_dev *rdev);
static struct regulator_dev *regl[BCMPMU_REGULATOR_MAX];
struct bcmpmu59xxx_regulator_pdata *rgpdata;

struct bcmpmu59xxx_rgltr_data {
	struct bcmpmu59xxx_regulator_info *rgltr_info;
	u32 num_of_rgltr;
};

/** voltage regulator details.  */
struct regulator_ops bcmpmu59xxx_ldo_ops = {
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

static int debug_mask = BCMPMU_PRINT_INIT | BCMPMU_PRINT_ERROR;
#define pr_rgltr(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)


/*
* @is_enabled: Return 1 if the regulator is enabled, 0 if not.
* May also return negative errno.
*/
static int bcmpmureg_is_enabled(struct regulator_dev *rdev)
{
	u8 val;
	struct bcmpmu59xxx *bcmpmu;
	struct bcmpmu59xxx_regulator_info *rinfo;
	int id ;
	int rc;

	if ((!rdev || !rdev->desc))
		return -1;
	bcmpmu = rdev_get_drvdata(rdev);
	rinfo = bcmpmu59xxx_get_rgltr_info(bcmpmu);
	id = rdev_get_id(rdev);
	if (rinfo[id].onoff != 0xFF) {
		pr_rgltr(FLOW, "<%s> id %d state %d\n",
			__func__, id, rinfo[id].onoff);
		return (rinfo[id].onoff == LDO_OFF ? 0 : 1);
	}
	rc = bcmpmu->read_dev(bcmpmu, rinfo[id].reg_addr, &val);

	if (rc < 0) {
		pr_rgltr(ERROR, "%s:error reading regulator register\n",
			__func__);
		return rc;
	}
	val >>= PM1_SHIFT;
	rinfo[id].onoff = (val & LDO_MODE_MASK);
	pr_rgltr(FLOW, "<%s> id %d state %d\n",
		__func__, id, rinfo[id].onoff);
	return (rinfo[id].onoff == LDO_OFF ? 0 : 1);
}

/*
* @enable: Configure the regulator as enabled.
*/
static int bcmpmureg_enable(struct regulator_dev *rdev)
{
	struct bcmpmu59xxx *bcmpmu;
	int id;
	struct bcmpmu59xxx_regulator_info *rinfo;
	struct bcmpmu59xxx_regulator_init_data *bcmpmu_regulators;
	u8 val;
	int ret = 0;

	if ((!rdev || !rdev->desc))
		return -1;
	bcmpmu = rdev_get_drvdata(rdev);
	id = rdev_get_id(rdev);
	rinfo = bcmpmu59xxx_get_rgltr_info(bcmpmu);

	bcmpmu_regulators = rgpdata->bcmpmu_rgltr;
	ret = bcmpmu->read_dev(bcmpmu, rinfo[id].reg_addr, &val);
	if (ret) {
		pr_rgltr(ERROR, "Err in reg read <%s>\n", __func__);
		return ret;
	}
	val &= ~((LDO_MODE_MASK << PM1_SHIFT) | (LDO_MODE_MASK << PM3_SHIFT));
	val |= ((LDO_NORMAL << PM1_SHIFT) | (LDO_NORMAL << PM3_SHIFT));
	switch ((bcmpmu_regulators + id)->dsm_mode) {
	case BCMPMU_REGL_OFF_IN_DSM:
		/*
		 * clear LPM bits
		 */
		val &=
		    ~((LDO_MODE_MASK << PM0_SHIFT) |
		      (LDO_MODE_MASK << PM2_SHIFT));
		/*
		 * set to off in DSM
		 */
		val |= ((LDO_OFF << PM0_SHIFT) | (LDO_OFF << PM2_SHIFT));
		break;
	case BCMPMU_REGL_LPM_IN_DSM:
		val &=
		    ~((LDO_MODE_MASK << PM0_SHIFT) |
		      (LDO_MODE_MASK << PM2_SHIFT));
		val |=
		    ((LDO_STANDBY << PM0_SHIFT) | (LDO_STANDBY << PM2_SHIFT));
		break;
	case BCMPMU_REGL_ON_IN_DSM:
		/*
		 * Regualtor should be on in DSM also
		 */
		val &=
		    ~((LDO_MODE_MASK << PM0_SHIFT) |
		      (LDO_MODE_MASK << PM2_SHIFT));
		val |= ((LDO_NORMAL << PM0_SHIFT) | (LDO_NORMAL << PM2_SHIFT));
		break;

	default:
		break;
	}
	ret = bcmpmu->write_dev(bcmpmu, rinfo[id].reg_addr, val);
	if (ret) {
		pr_rgltr(ERROR, "Err in reg write <%s> reg  %x\n",
		__func__, rinfo[id].reg_addr);
		rinfo[id].onoff = LDO_OFF;
		return ret;
	}

	rinfo[id].onoff = LDO_NORMAL;
	pr_rgltr(DATA, "Rgltr reg write <%s> reg  %x\n",
		__func__, rinfo[id].reg_addr);
	return ret;
}

/*
* @disable: Configure the regulator as disabled.
*/
static int bcmpmureg_disable(struct regulator_dev *rdev)
{
	int ret = 0;
	int id;
	struct bcmpmu59xxx *bcmpmu;
	u8 rc;
	struct bcmpmu59xxx_regulator_info *rinfo;

	if (!rdev || !rdev->desc)
		return -1;

	id = rdev_get_id(rdev);
	bcmpmu = rdev_get_drvdata(rdev);
	rinfo = bcmpmu59xxx_get_rgltr_info(bcmpmu);
	rc = LDO_OFF << PM0_SHIFT | LDO_OFF << PM1_SHIFT |
	    LDO_OFF << PM2_SHIFT | LDO_OFF << PM3_SHIFT;

	ret = bcmpmu->write_dev(bcmpmu, rinfo[id].reg_addr, rc);
	if (ret) {
		pr_rgltr(ERROR, "Err in reg write <%s> reg  %x\n",
		__func__, rinfo[id].reg_addr);
		return ret;
	}
	rinfo[id].onoff = LDO_OFF;

	pr_rgltr(DATA, "Rgltr reg write <%s> reg  %x\n",
		__func__, rinfo[id].reg_addr);
	return ret;
}

/*
* @get_status: Return actual (not as-configured) status of regulator, as a
* REGULATOR_STATUS value (or negative errno)
*/
static int bcmpmureg_get_status(struct regulator_dev *rdev)
{
	struct bcmpmu59xxx *bcmpmu = rdev_get_drvdata(rdev);
	int id = rdev_get_id(rdev);
	struct bcmpmu59xxx_regulator_info *rdesc =
	    bcmpmu59xxx_get_rgltr_info(bcmpmu);
	unsigned int ldo_or_sr = rdesc[id].ldo_or_sr;
	u8 val;
	int rc;

	rc = bcmpmu->read_dev(bcmpmu, rdesc[id].reg_addr, &val);
	if (rc < 0) {
		pr_rgltr(ERROR, "Err %s reading regulator register.\n",
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
	struct bcmpmu59xxx *bcmpmu = rdev_get_drvdata(rdev);
	int id = rdev_get_id(rdev);
	struct bcmpmu59xxx_regulator_info *rdesc =
	    bcmpmu59xxx_get_rgltr_info(bcmpmu);
	unsigned int ldo_or_sr = rdesc[id].ldo_or_sr;
	u8 val;
	int rc = bcmpmu->read_dev(bcmpmu, rdesc[id].reg_addr, &val);

	if (rc < 0) {
		pr_rgltr(ERROR,
			"ERR %s reading regulator register.\n",
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
	struct bcmpmu59xxx *bcmpmu = rdev_get_drvdata(rdev);
	int id = rdev_get_id(rdev);
	struct bcmpmu59xxx_regulator_info *rdesc =
	    bcmpmu59xxx_get_rgltr_info(bcmpmu);
	unsigned int ldo_or_sr = rdesc[id].ldo_or_sr;
	u8 val;
	int opmode, rc;

	rc = bcmpmu->read_dev(bcmpmu, rdesc[id].reg_addr, &val);
	if (rc < 0) {
		pr_rgltr(ERROR,
			"ERR %s reading regulator register.\n",
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

	return bcmpmu->write_dev(bcmpmu, rdesc[id].reg_addr, val);
}

/*
* @list_voltage: Return one of the supported voltages, in microvolts; zero if
* the selector indicates a voltage that is unusable on this system; or
* negative errno.  Selectors range from zero to one less than
* regulator_desc.n_voltages.  Voltages may be reported in any order.
*/

static int bcmpmuldo_list_voltage(struct regulator_dev *rdev, unsigned selector)
{
	struct bcmpmu59xxx *bcmpmu = rdev_get_drvdata(rdev);
	struct bcmpmu59xxx_regulator_info *rinfo;
	u8 id = rdev_get_id(rdev);
	struct bcmpmu59xxx_rgltr_data *rdata = bcmpmu->rgltr_data;
	rinfo = rdata->rgltr_info;
	if (selector >= rinfo[id].num_voltages)
		return -EINVAL;
	return rinfo[id].v_table[selector];
}

/*
* @set_voltage: Set the voltage for the regulator within the range specified.
* The driver should select the voltage closest to min_uV.
*/
static int bcmpmuldo_set_voltage(struct regulator_dev *rdev, int min_uv,
				 int max_uv, unsigned *selector)
{
	struct bcmpmu59xxx *bcmpmu = rdev_get_drvdata(rdev);
	struct bcmpmu59xxx_regulator_info *rgltr_info;
	struct bcmpmu59xxx_rgltr_data *rdata = bcmpmu->rgltr_data;
	u32 addr = 0;
	u8 rc;

	*selector = -1;
	rgltr_info = rdata->rgltr_info;
	for (rc = 0; rc < rgltr_info->num_voltages; rc++) {
		int uv = rgltr_info->v_table[rc];

		if ((min_uv <= uv) && (uv <= max_uv)) {
			addr = rgltr_info->reg_addr_volt;
			*selector = rc;
			return bcmpmu->write_dev(bcmpmu, addr, rc);
		}
	}
	return -EDOM;
}

static int bcmpmuldo_get_voltage(struct regulator_dev *rdev)
{
	struct bcmpmu59xxx *bcmpmu = rdev_get_drvdata(rdev);
	struct bcmpmu59xxx_regulator_info *rgltr_info;
	struct bcmpmu59xxx_rgltr_data *rdata = bcmpmu->rgltr_data;
	int rc = 0;
	u8 val;

	rgltr_info = rdata->rgltr_info;
	rc = bcmpmu->read_dev(bcmpmu, rgltr_info->reg_addr_volt, &val);
	if (rc < 0) {
		pr_rgltr(ERROR,
			"ERR %s reading regulator voltage register.\n",
			__func__);
		return rc;
	}
	rc = val & (rgltr_info->num_voltages - 1);
	return rgltr_info->v_table[rc];
}

#ifdef CONFIG_DEBUG_FS

static ssize_t bcmpmu_dbg_rgltr_enable(struct file *file,
				char const __user *buf,
				size_t count, loff_t *offset)
{
	int ret;
	u32 rgltr, len;
	char input_str[100];
	struct bcmpmu59xxx *bcmpmu = file->private_data;
	BUG_ON(!bcmpmu);
	if (count > 100)
		len = 100;
	else
		len = count;
	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	sscanf(input_str, "%x", &rgltr);
	if (rgltr >= BCMPMU_REGULATOR_MAX) {
		pr_info("invalid param !!\n");
		return -EFAULT;
	}

	ret = bcmpmureg_enable(regl[rgltr]);
	if (ret)
		return ret;
	pr_info("rgltr %x enabled\n", rgltr);
	return count;
}

static ssize_t bcmpmu_dbg_rgltr_disable(struct file *file,
				char const __user *buf,
				size_t count, loff_t *offset)
{
	int ret;
	u32 rgltr, len;
	char input_str[100];
	struct bcmpmu59xxx *bcmpmu =
		(struct bcmpmu59xxx *)file->private_data;
	BUG_ON(bcmpmu == NULL);
	if (count > 100)
		len = 100;
	else
		len = count;
	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	sscanf(input_str, "%x", &rgltr);
	if (rgltr >= BCMPMU_REGULATOR_MAX) {
		pr_info("invalid param !!\n");
		return -EFAULT;
	}

	ret = bcmpmureg_disable(regl[rgltr]);
	if (ret)
		return ret;
	pr_info("rgltr %x disabled\n", rgltr);
	return count;
}

static ssize_t bcmpmu_dbg_is_rgltr_enable(struct file *file,
		char const __user *buf,
		size_t count, loff_t *offset)
{
	u32 rgltr, len, i;
	char input_str[100];
	struct bcmpmu59xxx_rgltr_data *rdata;
	struct bcmpmu59xxx *bcmpmu =
		(struct bcmpmu59xxx *)file->private_data;

	BUG_ON(bcmpmu == NULL);

	rdata = bcmpmu->rgltr_data;
	if (count > 100)
		len = 100;
	else
		len = count;
	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	sscanf(input_str, "%x", &rgltr);
	if (rgltr == 0xFF) {
		for (i = 0; i < rdata->num_of_rgltr; i++) {
			pr_info("rglt input %x\n", i);
			if (bcmpmureg_is_enabled(regl[i]) == 1)
				pr_info("rglt no %x id %x name %sis Enabled\n",
						i, regl[i]->desc->id,
						regl[i]->desc->name);
		}
	} else {
		if (rgltr >= BCMPMU_REGULATOR_MAX) {
			pr_info("invalid param !!\n");
			return -EFAULT;
		}
		if (bcmpmureg_is_enabled(regl[rgltr]) == 1)
			pr_info("rglt %x id %x name %s is Enable\n",
					rgltr, regl[rgltr]->desc->id,
					regl[rgltr]->desc->name);
		else if (bcmpmureg_is_enabled(regl[rgltr]) == 0)
			pr_info("rglt %x id %x name %s is Disable\n",
					rgltr, regl[rgltr]->desc->id,
					regl[rgltr]->desc->name);
	}
	return count;
}

static const struct file_operations debug_rgltr_en_fops = {
	.open = bcmpmu_debugfs_open,
	.write = bcmpmu_dbg_rgltr_enable,
};

static const struct file_operations debug_rgltr_dis_fops = {
	.open = bcmpmu_debugfs_open,
	.write = bcmpmu_dbg_rgltr_disable,
};

static const struct file_operations debug_rgltr_state_fops = {
	.open = bcmpmu_debugfs_open,
	.write = bcmpmu_dbg_is_rgltr_enable,
};

static void bcmpmu59xxx_debug_init(struct bcmpmu59xxx *bcmpmu)
{
	if (!bcmpmu->dent_bcmpmu) {
		pr_err("Failed to initialize debugfs\n");
		return;
	}

	debugfs_rgltr_dir = debugfs_create_dir("rgltr", bcmpmu->dent_bcmpmu);
	if (!debugfs_rgltr_dir)
		return;

	if (!debugfs_create_file("rgltr_state", S_IWUSR | S_IRUSR,
					debugfs_rgltr_dir, bcmpmu,
					&debug_rgltr_state_fops))
		goto err;

	if (!debugfs_create_file("rgltr_enable", S_IWUSR | S_IRUSR,
					debugfs_rgltr_dir, bcmpmu,
					&debug_rgltr_en_fops))
		goto err;

	if (!debugfs_create_file("rgltr_disable", S_IWUSR | S_IRUSR,
					debugfs_rgltr_dir, bcmpmu,
					&debug_rgltr_dis_fops))
		goto err;

	if (!debugfs_create_u32("dbg_mask", S_IWUSR | S_IRUSR,
				debugfs_rgltr_dir, &debug_mask))
		goto err ;

	return;
err:
	debugfs_remove(debugfs_rgltr_dir);
}

#endif
static int bcmpmu_regulator_probe(struct platform_device *pdev)
{
	int i;
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu59xxx_regulator_pdata *rpdata;
	struct bcmpmu59xxx_regulator_init_data *bcmpmu_regulators;
	struct bcmpmu59xxx_regulator_info *rgltr_info;
	struct bcmpmu59xxx_rgltr_data *rdata;
	int num_of_rgltr;
	int rgltr_id, ret = 0;
	u8 opmode;

	rpdata = (struct  bcmpmu59xxx_regulator_pdata *)
		pdev->dev.platform_data;
	rgpdata = rpdata;
	bcmpmu_regulators = rpdata->bcmpmu_rgltr;
	num_of_rgltr = rpdata->num_of_rgltr;
	pr_rgltr(INIT, "###########%s############: called\n", __func__);
	/*
	 * register regulator
	 */
	rgltr_info = bcmpmu59xxx_get_rgltr_info(bcmpmu);

	if ((rgltr_info == NULL) || (rgltr_info->rdesc == NULL)) {
		pr_rgltr(ERROR, "ERR %s regulator info and desc not avail.\n",
				__func__);
		return -EINVAL;
	}

	rdata =  kzalloc(sizeof(struct bcmpmu59xxx_rgltr_data), GFP_KERNEL);
	if (!rdata) {
		pr_rgltr(ERROR, "ERR: %s failed to alloc mem.\n", __func__);
		return -ENOMEM;
	}
	rdata->rgltr_info = rgltr_info;
	rdata->num_of_rgltr = num_of_rgltr;
	bcmpmu->rgltr_data = rdata;
	for (i = 0; i < num_of_rgltr; i++) {
		rgltr_id = bcmpmu_regulators[i].regulator;
		opmode = bcmpmu_regulators[i].default_opmode;
		if (bcmpmu_regulators[i].initdata) {
			if (opmode != 0xFF) {
				ret =
					bcmpmu->write_dev(bcmpmu,
							rgltr_info[rgltr_id].
							reg_addr, opmode);
				if (ret != 0)
					goto register_fail;
			}
			pr_rgltr(INIT, "%s: REGULATOR ID %d, name %s"
					"def_opmode = %x\n",
					__func__, rgltr_id,
					rgltr_info->rdesc[rgltr_id].name,
					opmode);

			regl[i] =
				regulator_register(rgltr_info[rgltr_id].rdesc,
						&pdev->dev,
						bcmpmu_regulators[i].initdata,
						bcmpmu, NULL);
			if (IS_ERR(regl[i])) {
				dev_err(&pdev->dev, "failed to register %s\n",
					rgltr_info->rdesc[rgltr_id].name);
				ret = PTR_ERR(regl[i]);

				goto register_fail;
			}
		} else
			regl[i] = NULL;

	}
	regulator_has_full_constraints();

#ifdef CONFIG_DEBUG_FS
	bcmpmu59xxx_debug_init(bcmpmu);
#endif
	return ret;
register_fail:
	while (i-- > 0) {
		if (regl[i])
			regulator_unregister(regl[i]);
	}
	rgltr_info->rdesc = NULL;
	rgltr_info = NULL;
	kfree(rdata);
	return ret;
}

static int bcmpmu_regulator_remove(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	int num_of_rgltr;
	int i = 0;

	num_of_rgltr = rgpdata->num_of_rgltr;
	while (i < num_of_rgltr) {
		if (regl[i]) {
			regulator_unregister(regl[i]);
			kzfree(regl[i]);
		}
		i++;
	}
	kfree(bcmpmu->rgltr_data);
#ifdef CONFIG_DEBUG_FS
	debugfs_remove(debugfs_rgltr_dir);
#endif
	return 0;
}

static struct platform_driver bcmpmu_regulator_driver = {
	.probe = bcmpmu_regulator_probe,
	.remove = bcmpmu_regulator_remove,
	.driver = {
		   .name = "bcmpmu59xxx-regulator",
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
