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


static int bcmpmuldo_get_voltage(struct regulator_dev *rdev) ;
static int bcmpmuldo_set_voltage(struct regulator_dev *rdev, int min_uv, int max_uv) ;
static int bcmpmuldo_list_voltage(struct regulator_dev *rdev, unsigned index) ;
static unsigned int bcmpmureg_get_mode (struct regulator_dev *rdev) ;
static int bcmpmureg_set_mode(struct regulator_dev *rdev, unsigned mode) ;
static int bcmpmureg_get_status(struct regulator_dev *rdev) ;
static int bcmpmureg_disable(struct regulator_dev *rdev) ;
static int bcmpmureg_enable(struct regulator_dev *rdev) ;
static int bcmpmureg_is_enabled(struct regulator_dev *rdev) ;


/** voltage regulator details.  */
struct regulator_ops bcmpmuldo_ops = {
	.list_voltage	= bcmpmuldo_list_voltage,
	.enable		= bcmpmureg_enable,
	.disable	= bcmpmureg_disable,
	.is_enabled	= bcmpmureg_is_enabled,
	.get_mode 	= bcmpmureg_get_mode ,
	.set_mode	= bcmpmureg_set_mode,
	.get_status	= bcmpmureg_get_status,
	.set_voltage	= bcmpmuldo_set_voltage, 
	.get_voltage	= bcmpmuldo_get_voltage,
};

/* @is_enabled: Return 1 if the regulator is enabled, 0 if not.
 *		May also return negative errno.
*/
static int bcmpmureg_is_enabled(struct regulator_dev *rdev)
{
	struct bcmpmu	*bcmpmu = rdev_get_drvdata(rdev);
	struct bcmpmu_reg_info  *info = bcmpmu->rgltr_info + rdev_get_id(rdev);
	unsigned int ldo_or_sr = info->ldo_or_sr;
	unsigned int val;

	int rc = bcmpmu->read_dev(bcmpmu, info->reg_addr, &val, info->mode_mask);

	if (rc < 0)
	{
		printk(KERN_ERR "%s:error reading regulator OPmode register\n", __func__);
		return rc;
	}

	rc = ( val >> info->en_dis_shift ) & ( info->en_dis_mask );

	if ( ldo_or_sr == BCMPMU_LDO ) {
		if ( rc < LDO_OFF ) return 1;
		else return 0; 
	}
	else {
		if ( rc == LDO_OFF ) return 0;
		else return 1; 
	}	
}

/* @enable: Configure the regulator as enabled. */
static int bcmpmureg_enable(struct regulator_dev *rdev)
{
	struct bcmpmu	*bcmpmu = rdev_get_drvdata(rdev);
	struct bcmpmu_reg_info  *info = bcmpmu->rgltr_info + rdev_get_id(rdev);
	unsigned int ldo_or_sr = info->ldo_or_sr;
	unsigned int val;

	int rc = bcmpmu->read_dev(bcmpmu, info->reg_addr, &val, info->mode_mask);

	if (rc < 0)
	{
		printk(KERN_ERR "%s: error reading regulator OPmode register.\n", __func__);
		return rc;
	}

	/* In case of LDO always enable in LPM (1) */
	/* In case of SR always enable in NM (0) */
	if ( ldo_or_sr == BCMPMU_LDO ) {
		if ( ( ( val >> info->en_dis_shift ) & ( info->en_dis_mask ) ) < LDO_OFF )
			return 1;
		else return ( bcmpmu->write_dev(bcmpmu, info->reg_addr,
			(( rc & ~info->en_dis_mask  ) | LDO_STANDBY ),
			info->en_dis_mask));
	}
	else {
		if ( ( ( val >> info->en_dis_shift ) & ( info->en_dis_mask ) ) != LDO_OFF )
			return 1;
		else return ( bcmpmu->write_dev(bcmpmu, info->reg_addr,
			(( rc & ~info->en_dis_mask  ) | LDO_NORMAL ),
			info->en_dis_mask));
	}
}

/* @disable: Configure the regulator as disabled. */
static int bcmpmureg_disable(struct regulator_dev *rdev)
{
	struct bcmpmu	*bcmpmu = rdev_get_drvdata(rdev);
	struct bcmpmu_reg_info  *info = bcmpmu->rgltr_info + rdev_get_id(rdev);
	unsigned int val;

	int rc = bcmpmu->read_dev(bcmpmu, info->reg_addr, &val, info->mode_mask);

	if (rc < 0) {
		printk(KERN_ERR "%s: error reading regulator OPmode register.\n", __func__);
		return rc;
	}

	if ( ( ( rc >> info->en_dis_shift ) & ( info->en_dis_mask ) ) == LDO_OFF )
		return 1;
	else {
		return ( bcmpmu->write_dev(bcmpmu, info->reg_addr,
			(( rc & ~info->en_dis_mask  ) | LDO_OFF ),
			info->en_dis_mask));
	}
}

/* @get_status: Return actual (not as-configured) status of regulator, as a
 *	REGULATOR_STATUS value (or negative errno)
 */
static int bcmpmureg_get_status(struct regulator_dev *rdev)
{
	struct bcmpmu	*bcmpmu = rdev_get_drvdata(rdev);
	struct bcmpmu_reg_info  *info = bcmpmu->rgltr_info + rdev_get_id(rdev);
	unsigned int ldo_or_sr = info->ldo_or_sr;
	unsigned int val;

	int rc = bcmpmu->read_dev(bcmpmu, info->reg_addr, &val, info->mode_mask);

	if (rc < 0) {
		printk(KERN_ERR "%s: error reading regulator OPmode register.\n", __func__);
		return rc;
	}

	rc = ( val >> info->en_dis_shift ) & ( info->en_dis_mask ) ;

	switch ( rc ) {
		case LDO_NORMAL:
			return REGULATOR_STATUS_NORMAL;
		case LDO_STANDBY:
			return REGULATOR_STATUS_STANDBY;
		case LDO_OFF:
			return REGULATOR_STATUS_OFF;
		case LDO_RESERVED_SR_IDLE:
			if ( ldo_or_sr == BCMPMU_SR )
				return REGULATOR_STATUS_IDLE;
		default:
			return -EINVAL;
	}
}

/* @get_mode: Get the configured operating mode for the regulator. */
static unsigned int bcmpmureg_get_mode (struct regulator_dev *rdev)
{
	struct bcmpmu	*bcmpmu = rdev_get_drvdata(rdev);
	struct bcmpmu_reg_info  *info = bcmpmu->rgltr_info + rdev_get_id(rdev);

	unsigned int ldo_or_sr = info->ldo_or_sr;
	unsigned int val;
	
	int rc = bcmpmu->read_dev(bcmpmu,info->reg_addr, &val, info->mode_mask);

	if (rc < 0) {
		printk(KERN_ERR "%s: error reading regulator OPmode register.\n", __func__);
		return rc;
	}

	/* Get last 2 bits after read.*/
	rc = ( val >> info->en_dis_shift ) & ( info->en_dis_mask ) ;

	switch ( rc ) {
		case LDO_NORMAL:
			return REGULATOR_MODE_NORMAL;
		case LDO_STANDBY:
			return REGULATOR_MODE_STANDBY;
		case LDO_OFF:
			return -EINVAL;
		case LDO_RESERVED_SR_IDLE:
			if ( ldo_or_sr == BCMPMU_SR )
				return REGULATOR_MODE_IDLE;
		default:
			return -EINVAL;
	}
}

/*  @set_mode: Set the configured operating mode for the regulator. */
static int bcmpmureg_set_mode(struct regulator_dev *rdev, unsigned mode)
{
	struct bcmpmu	*bcmpmu = rdev_get_drvdata(rdev);
	struct bcmpmu_reg_info  *info = bcmpmu->rgltr_info + rdev_get_id(rdev);
	unsigned int ldo_or_sr = info->ldo_or_sr;
	unsigned int val;
	int rc = bcmpmu->read_dev(bcmpmu,info->reg_addr, &val, info->mode_mask);

	if (rc < 0) {
		printk(KERN_ERR "%s: error reading regulator OPmode register.\n", __func__);
		return rc;
	}

	/* Clear up last 2 bits after read. */
	rc = ( val >> info->en_dis_shift ) & ( ~info->en_dis_mask ) ;

	switch ( mode ) {
		case REGULATOR_MODE_NORMAL:
			rc = rc | LDO_NORMAL;
			break;
		case REGULATOR_MODE_STANDBY:
			rc = rc | LDO_STANDBY;
			break;
		case REGULATOR_MODE_IDLE:
			if ( ldo_or_sr == BCMPMU_SR )
				rc = rc | LDO_RESERVED_SR_IDLE;
			else if ( ldo_or_sr == BCMPMU_LDO )
				rc = rc | LDO_STANDBY;
		default:
			return -EINVAL;
	}
	return ( bcmpmu->write_dev(bcmpmu,info->reg_addr, rc, info->en_dis_mask ) );
}

/* @list_voltage: Return one of the supported voltages, in microvolts; zero
 *	if the selector indicates a voltage that is unusable on this system;
 *	or negative errno.  Selectors range from zero to one less than
 *	regulator_desc.n_voltages.  Voltages may be reported in any order.
 *	*/

static int bcmpmuldo_list_voltage(struct regulator_dev *rdev, unsigned index)
{
	struct bcmpmu *bcmpmu = rdev_get_drvdata(rdev);
	struct bcmpmu_reg_info *info = bcmpmu->rgltr_info + rdev_get_id(rdev); 

	return ( info->v_table[index] ) ;
}

/* @set_voltage: Set the voltage for the regulator within the range specified.
 *               The driver should select the voltage closest to min_uV.
 *               */
static int bcmpmuldo_set_voltage(struct regulator_dev *rdev, int min_uv, int max_uv)
{
	struct bcmpmu	*bcmpmu = rdev_get_drvdata(rdev);
	struct bcmpmu_reg_info  *info = bcmpmu->rgltr_info + rdev_get_id(rdev);
	unsigned int ldo_or_sr = info->ldo_or_sr;
	unsigned int addr = 0 ;
	unsigned int mode = 0 ;
	int	rc;
	int	ret;
	unsigned int val;

	for (rc = 0; rc < info->num_voltages ; rc++) {
		int uv = info->v_table[rc] ;

		if (( min_uv <= uv ) && ( uv <= max_uv )) {
			if ( ldo_or_sr == BCMPMU_LDO )
				addr = info->reg_addr_volt;
			else if ( ldo_or_sr == BCMPMU_SR ) {
				/* To Set voltage, first get mode you are in for case of CSR, IOSR, SDSR.*/
				mode =  bcmpmureg_get_mode(rdev);
				switch ( mode ) {
				case REGULATOR_MODE_NORMAL:
					addr = info->reg_addr_volt;
					break;
				case REGULATOR_MODE_STANDBY:
					addr = info->reg_addr_volt_l;
					break;
				case REGULATOR_MODE_FAST:
					addr = info->reg_addr_volt_t;
					break;
				default:
					return -EINVAL;
				}
			}
			rc = ( val & (~(info->vout_mask)) )  | rc ;
			return ( bcmpmu->write_dev(bcmpmu, addr, rc, info->vout_mask) ) ;
		}
	}
	return -EDOM;
}

static int bcmpmuldo_get_voltage(struct regulator_dev *rdev)
{
	struct bcmpmu	*bcmpmu = rdev_get_drvdata(rdev);
	struct bcmpmu_reg_info  *info = bcmpmu->rgltr_info + rdev_get_id(rdev); 
	unsigned int ldo_or_sr = info->ldo_or_sr;
	unsigned int addr = 0 ;
	unsigned int mode = 0 ;
	int rc = 0;
	unsigned int val;

	if ( ldo_or_sr == BCMPMU_LDO )
		addr = info->reg_addr_volt;
	else if ( ldo_or_sr == BCMPMU_SR ){
		mode =  bcmpmureg_get_mode (rdev);
		switch ( mode ){
		case REGULATOR_MODE_NORMAL:
			addr = info->reg_addr_volt;
			break;
		case REGULATOR_MODE_STANDBY:
			addr = info->reg_addr_volt_l;
			break;
		case REGULATOR_MODE_FAST:
			addr = info->reg_addr_volt_t;
			break;
		default:
			return -EINVAL;
		}
	}
	rc = bcmpmu->read_dev(bcmpmu, addr, &val, info->vout_mask);
	if (rc < 0) {
		printk(KERN_ERR "%s: error reading regulator voltage register.\n", __func__);
		return rc;
	}
	rc = ( val >> info->vout_shift ) & ( info->vout_mask );
	return (info->v_table[rc]);
}

static int bcmpmu_regulator_probe(struct platform_device *pdev)
{
	struct regulator_dev *rdev;
	struct bcmpmu *bcmpmu = platform_get_drvdata(pdev);

	printk(KERN_INFO "%s: called. id = %d\n", __func__, pdev->id);

	/* register regulator */
	if ( ( bcmpmu->rgltr_info == NULL ) ||
		( bcmpmu->rgltr_desc == NULL ) ) {
		printk(KERN_ERR "%s: regulator info and desc not avail.\n", __func__);
	}

	rdev = regulator_register( (bcmpmu->rgltr_desc + pdev->id), &pdev->dev,
				  pdev->dev.platform_data,
				  dev_get_drvdata(&pdev->dev));
	if (IS_ERR(rdev)) {
		dev_err(&pdev->dev, "failed to register %s\n",
			 ( bcmpmu->rgltr_desc + pdev->id)->name);
		return PTR_ERR(rdev);
	}

	regulator_has_full_constraints();

	return 0;
}

static int bcmpmu_regulator_remove(struct platform_device *pdev)
{
	struct regulator_dev *rdev = platform_get_drvdata(pdev);

	regulator_unregister(rdev);

	return 0;
}

int bcmpmu_register_regulator(struct bcmpmu *bcmpmu, int reg,
			      struct regulator_init_data *initdata)
{
	struct platform_device *pdev;
	int ret = 0 ;

	pdev = platform_device_alloc("bcmpmu-regulator", reg);
	if (!pdev)
		return -ENOMEM;

	bcmpmu->pdev[reg] = pdev;
	initdata->driver_data = bcmpmu;
	pdev->dev.platform_data = initdata;
	pdev->dev.parent = bcmpmu->dev;
	platform_set_drvdata(pdev, bcmpmu);

	ret = platform_device_add(pdev);

	if (ret != 0) {
		dev_err(bcmpmu->dev, "Failed to register regulator %d: %d\n",
			reg, ret);
		platform_device_del(pdev);
		bcmpmu->pdev[reg] = NULL;
	}

	return ret;
}

void bcmpmu_reg_dev_init(struct bcmpmu *bcmpmu)
{
	int i = 0;
	struct bcmpmu_regulator_init_data *bcmpmu_regulators =
		bcmpmu->pdata->regulator_init_data;

	bcmpmu->rgltr_desc = bcmpmu_rgltr_desc();
	bcmpmu->rgltr_info = bcmpmu_rgltr_info();

	for (i = 0; i < BCMPMU_REGULATOR_MAX; i++) {
		bcmpmu->pdev[i] = NULL;
		if ((bcmpmu_regulators + i)->initdata != NULL)
			bcmpmu_register_regulator(bcmpmu,
				(bcmpmu_regulators + i)->regulator,
				(bcmpmu_regulators + i)->initdata);
	}
}

void bcmpmu_reg_dev_exit(struct bcmpmu *bcmpmu)
{
	int i;
	for (i = 0; i < BCMPMU_REGULATOR_MAX; i++)
		if (bcmpmu->pdev[i] != NULL)
			platform_device_unregister(bcmpmu->pdev[i]);
}



static struct platform_driver bcmpmu_regulator_driver = {
	.probe = bcmpmu_regulator_probe,
	.remove = bcmpmu_regulator_remove,
	.driver		= {
		.name	= "bcmpmu-regulator",
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

/* Module information */
MODULE_AUTHOR("Saringni Addepally");
MODULE_DESCRIPTION("BCMPMU voltage and current regulator driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:bcmpmu-regulator");

MODULE_DESCRIPTION("BCMPMU regulator driver");
MODULE_LICENSE("GPL");
