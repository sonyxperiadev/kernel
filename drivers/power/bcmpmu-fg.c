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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/bug.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/power_supply.h>
#include <linux/time.h>
#include <linux/sort.h>
#include <linux/wakelock.h>
#include <linux/mfd/bcmpmu.h>

#define FG_ACCM_REG_TO_COULOMB_COUNT(accm1, accm2, accm3, accm4)	\
	(((accm1 & FGACCM1_FGACCM_MASK) << 24) |\
	 ((accm2 & FGACCM2_FGACCM_MASK) << 16) |\
	 ((accm3 & FGACCM3_FGACCM_MASK) << 8)  |\
	 ((accm4 & FGACCM4_FGACCM_MASK) << 0))

#define FG_CNT_REG_TO_SAMPLE_COUNT(reg1, reg2)	\
	(((reg1 & FGCNT1_FGCNT_MASK) << 8) |\
	 ((reg2 & FGCNT2_FGCNT_MASK) << 0))

#define FG_SMPL_REG_TO_VALUE(smpl_15_8, smpl_7_0)	\
	(((smpl_15_8 & FGSMPL3_FGSMPL_CAL_15_8_MASK) << 8) |\
	 ((smpl_7_0 & FGSMPL4_FGSMPL_CAL_7_0_MASK)))

#define FG_CAPACITY_SAVE_REG		(PMU_REG_FGGNRL1)
#define FG_CAP_DELTA_THRLD		10
#define AVG_SAMPLES			5

enum {
	BCMPMU_LOG_ERR = 1U << 0,
	BCMPMU_LOG_INIT = 1U << 1,
	BCMPMU_LOG_FLOW = 1U << 2,
	BCMPMU_LOG_DATA = 1U << 3,
	BCMPMU_LOG_REPORT = 1U << 4,
};

enum bcmpmu_fg_state {
	FG_STATE_INIT_CAP,
	FG_STATE_BATT_CHARG,
	FG_STATE_BATT_CHARG_MAINT,
	FG_STATE_BATT_DISCHARG,
	FG_STATE_BATT_LOW_BATT,
	FG_STATE_CAL_LOW,
	FG_STATE_CAL_HIGH,
};

static char *bcmpm_fg_state_dbg [] = {
	"RESET",
	"INIT_CAP",
	"CAL_LOW",
	"CAL_HIGH"
};

enum bcmpmu_fg_cal_state {
	CAL_STATE_IDLE,
	CAL_STATE_START,
	CAL_STATE_WAIT,
	CAL_STATE_DONE
};

enum bcmpmu_fg_cal_mode {
	CAL_MODE_LOW_BATT,
	CAL_MODE_HI_BATT,
	CAL_MODE_FORCE,
	CAL_MODE_TEMP,
};

static char *bcmpm_fg_cal_state_dbg [] = {
	"CAL_IDLE",
	"CAL_START",
	"CAL_WAIT",
	"CAL_DONE"
};

enum bcmpmu_mbc_state {
	MBC_STATE_IDLE,
	MBC_STATE_QC,
	MBC_STATE_FC,
	MBC_STATE_MC,
	MBC_STATE_PAUSE,
	MBC_STATE_THSD,
};

enum bcmpmu_fg_sample_rate {
	SAMPLE_RATE_2HZ = 0,
	SAMPLE_RATE_4HZ,
	SAMPLE_RATE_8HZ,
	SAMPLE_RATE_16HZ,
};

enum bcmpmu_fg_opmode {
	FG_OPMODE_SYNC,
	FG_OPMODE_CONTINUOUS,
};

enum bcmpmu_fg_mode {
	FG_MODE_ACTIVE,
	FG_MODE_FREEZE,
};

enum batt_charg_state {
	CHARG_STATE_CHARGING,
	CHRG_STATE_MAINT,
};

enum batt_dischrg_state {
	DISCHARG_STATE_NORMAL,
	DISCHARG_STATE_LOW_BATT,
};

static enum power_supply_property bcmpmu_fg_props[] = {
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_CAPACITY_LEVEL,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_MODEL_NAME,
};


/**
 * FG private data
 */
struct bcmpmu_fg {
	u32 batt_cap;	/* Current battery capacity */
	u32 init_cap;	/* initial battery capacity */
	struct bcmpmu *bcmpmu;
	struct bcmpmu_fg_pdata *pdata;
	enum bcmpmu_fg_state fg_state;
	enum bcmpmu_fg_cal_state cal_state;
	struct power_supply psy;
	struct cap_volt_lut *cap_volt_lut;
	struct work_queue fg_wq;
	struct delayed_work fg_periodic_work;
	struct delayed_work fg_cal_work;
	struct delayed_work fg_low_batt_work;
};

#ifdef CONFIG_DEBUG_FS
static u32 debug_mask = BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT;
#define DEBUG_FS_PERMISSIONS	(S_IRUSR | S_IWUSR)

#define fg_dbg(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)
#endif

static int bcmpmu_fg_set_sample_rate(struct bcmpmu_fg *fg,
		enum bcmpmu_fg_sample_rate rate)
{
	int ret;
	u8 reg;

	if ((rate < SAMPLE_RATE_2HZ) || (rate > SAMPLE_RATE_16HZ))
		return -EINVAL;

	ret = fg->bcmpmu->read_dev(bcmpmu, PMU_REG_FGOCICCTRL1, &reg);
	if (!ret) {
		reg &= ~FGOCICCTRL1_FGCOMBRATE_MASK;
		reg |= (rate << FGOCICCTRL1_FGCOMBRATE_SHIFT);
		ret = fg->bcmpmu->write_dev(bcmpmu, PMU_REG_FGOCICCTRL1, reg);
	}
	return ret;
}

static int bcmpmu_fg_enable(struct bcmpmu_fg *fg, bool enable)
{
	int ret;
	u8 reg;

	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_FGCTRL1, &reg);
	if (!ret) {
		if (enable)
			reg |= FGCTRL1_FGHOSTEN_MASK;
		else
			reg &= ~FGCTRL1_FGHOSTEN_MASK;
		ret = fg->bcmpmu->write_dev(fg->bcmpmu, PMU_REG_FGCTRL1, reg);
	}
	return ret;
}
/**
 * bcmpmu_fg_reset - Reset FG HW and coulomb counters
 *
 * @bcmpmu_fg:		Pointer to bcmpmu_fg struct
 *
 * Reset the PMU Fuel Gauage and clear the counters
 */
static int bcmpmu_fg_reset(struct bcmpmu_fg *fg)
{
	u8 reg;
	int ret;

	ret = fg->bcmpmu->read_dev(bcmpmu, PMU_REG_FGCTRL2, &reg);
	if(!ret) {
		if (ret) {
			reg |= FGCTRL2_FGRESET_MASK;
			ret = fg->bcmpmu->write_dev(bcmpmu, PMU_REG_FGCTRL2,
					reg);
		}
	}
	return ret;
}

/**
 * bcmpmu_fg_freeze_read - Freeze FG coulomb counter for read
 *
 * @bcmpmu_fg:		Pointer to bcmpmu_fg struct
 *
 * Writes to FGFRZREAD bit which will freeze the coulomb counter
 * and latch the accumulator registers and sample counter register
 */

static int bcmpmu_fg_freeze_read(struct bcmpmu_fg *fg)
{
	int ret;
	u8 reg;

	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_FGCTRL2, &reg);
	if (!ret) {
		reg |= FGCTRL2_FGFRZREAD_MASK;
		ret = fg->bcmpmu->write_dev(fg->bcmpmu, PMU_REG_FGCNT2, reg);
	}
	return ret;
}
/**
 * Get instantaneous value of coulomb counter
 */
static int bcmpmu_fg_get_accumulator(struct bcmpmu_fg *fg, u32 *coulb,
		u32 *sample_count)
{
	int ret;
	u8 accm[4];
	u8 cnt[2];

	bcmpmu_fg_freeze_read(fg);

	ret = fg->bcmpmu->read_dev_bulk(fg->bcmpmu, PMU_REG_FGACCM1, accm, 4);
	ret = fg->bcmpmu->read_dev_bulk(fg->bcmpmu, PMU_REG_FGCNT1, cnt, 2);

	if (!ret && (reg[0] & FGACCM1_FGRDVALID_MASK)) {
		*coulb = FG_ACCM_REG_TO_COULOMB_COUNT(accm[0], accm[1],
				accm[2], accm[3]);
		*sample_count = FG_CNT_REG_TO_SAMPLE_COUNT(cnt[0], cnt[1]);
	}
	return ret;
}

static int bcmpmu_fg_get_curr_inst(struct bcmpmu_fg *fg, u32 *curr)
{
	int ret;
	u8 reg;
	u8 smpl_cal[2];
	int sample = 0;

	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_FGCNT2, &reg);
	if (!ret)
		return ret;

	reg |= FGCTRL2_FGFRZSMPL_MASK;
	ret = fg->bcmpmu->write_dev(fg->bcmpmu, PMU_REG_FGCTRL2, reg);
	if (!ret)
		return ret;
	ret = fg->bcmpmu->read_dev_bulk(fg->bcmpmu, PMU_REG_FGSMPL3, smpl_cal,
			2);
	if (ret) {
		sample =
		/*
		 * Current sample is represented in 2's complement format
		 * in PMU. if FGSMPL[15] is sign bit which represents current
		 * flow (positive current or negative current)
		 */
		if (smpl_cal[0] & 0x8000) {
			/**
			 * extend the sign bit for int format
			 */
		}
	}
	return ret;
}

static int bcmpmu_fg_get_batt_volt(struct bcmpmu_fg *fg)
{

}

static int bcmpmu_fg_get_volt_to_cap(struct bcmpmu_fg *fg)
{
	struct bcmpmu_batt_property *batt_prop =
		fg->pdata->batt_prop->volt_cap_lut;
	int i;

	if (!batt_prop)
		return -EINVAL;
	for (i = 0; i <)
}

static int bcmpmu_fg_get_avg_volt(struct bcmpmu_fg *fg)
{
	u8 reg;
	int i = 0;
	int volt;

	do {
		volt += bcmpmu_fg_get_batt_volt(fg);
		i++;
		msleep(5);
	} while (i < AVG_SAMPLES);
	return (volt / i);
}

static int bcmpmu_fg_save_cap(struct bcmpmu_fg *fg, int cap_percentage)
{
	int ret;
	u8 reg;

	BUG_ON(cap_percentage > 100);

	ret = fg->bcmpmu->write_dev(bcmpmu, FG_CAPACITY_SAVE_REG,
			cap_percentage);
	return ret;
}

static int bcmpmu_fg_get_saved_cap(struct bcmpmu_fg *fg)
{
	int ret;
	u8 reg;
	ret = fg->bcmpmu->read_dev(bcmpmu, FG_CAPACITY_SAVE_REG, &reg);
	if (!ret)
		ret = reg;
	return ret;
}

static int bcmpmu_fg_set_opmod(struct bcmpmu_fg *fg,
		enum bcmpmu_fg_opmode opmode, int opvalue)
{
	int ret;
	u8 reg;
	u8 opmode_mask = (FGOPMODCTRL_OPMODCTRL0_MASK |
					FGOPMODCTRL_OPMODCTRL1_MASK |
					FGOPMODCTRL_OPMODCTRL2_MASK |
					FGOPMODCTRL_OPMODCTRL3_MASK);

	ret = fg->bcmpmu->read_dev(bcmpmu, PMU_REG_FGOPMODCTRL, &reg);
	if (!reg) {
		if (opmode == FG_OPMODE_SYNC) {
			reg |= FGOPMODCTRL_FGSYNCMODE_MASK;
			reg &= ~opmode_mask;
			reg |= opvalue;
		}
		else if (opmode == FG_OPMODE_CONTINUOUS)
			reg &= ~FGOPMODCTRL_FGSYNCMODE_SHIFT;
		ret = fg->bcmpmu->write_dev(bcmpmu, PMU_REG_FGOPMODCTRL, reg);
	}
	return ret;
}

static int bcmppmu_fg_update_batt_psy()
{

}

static int bcmpmu_fg_init_pmu_regs(struct bcmpmu_fg *bcmpmu_fg)
{

}

static int bcmpmu_fg_get_init_cap(struct bcmpmu_fg *fg)
{
	fg->init_cap = bcmpmu_fg_get_saved_cap();
}

static void bcmpmu_fg_charging_algo(struct bcmpmu_fg *fg)
{

}

static void bcmpmu_fg_discharging_algo(struct bcmpmu_fg *fg)
{

}

static void bcmpmu_fg_calibration_algo(struct bcmpmu_fg *fg)
{

}
static void bcmpmu_fg_cal_work(struct work_struct *work)
{

}

static void bcmpmu_fg_periodic_work(struct work_struct *work)
{
	struct bcmpmu_fg *fg = container_of(work, struct bcmpmu_fg,
			fg_periodic_work.work);

	switch (fg->fg_state) {
	case FG_STATE_INIT_CAP:
		break;
	case FG_STATE_BATT_CHARG:
		break;
	case FG_STATE_BATT_DISCHARG:
		break;
	case FG_STATE_CAL_LOW:
		break;
	case FG_STATE_CAL_HIGH:
		break;
	}
}

static int bcmpmu_fg_get_properties(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{

}

static int bcmpmu_fg_set_properties(struct power_supply *psy,
		enum power_supply_property psp,
		const union power_supply_propval *val)
{

}

static void bcmpmu_fg_external_power_changed(struct power_supply *psy)
{

}

#ifdef CONFIG_DEBUG_FS
static int bcmpmu_fg_debugfs_init(bcmpmu_fg *fg)
{
	struct dentry *dentry_pmu_dir;
	struct dentry *dentry_fg_dir;
	struct dentry *dentry_fg_file;

	dentry_pmu_dir = bcmpmu_get_debugfs_dir(fg->bcmpmu);
	if (!dentry_pmu_dir)
		return -ENODEV;

	dentry_fg_dir = debugfs_create_dir("bcmpmu_fg", dentry_pmu_dir);
	if (IS_ERR_OR_NULL(dentry_fg_dir))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_u32("capacity", DEBUG_FS_PERMISSIONS,
				dentry_fg_dir, fg->batt_cap);
	if (IS_ERR_OR_NULL(dentry_fg_dir))
		goto debugfs_clean;

	dentry_file = debugfs_create_u32("debug_mask", DEBUG_FS_PERMISSIONS,
			dentry_fg_dir, &debug_mask);
	if (IS_ERR_OR_NULL(dentry_fg_dir))
		goto debugfs_clean;

	return 0;
clean_debugfs:
	if (!IS_ERR_OR_NULL(dentry_fg_dir))
		debugfs_remove_recursive(dentry_fg_dir);
}
#endif

#if CONFIG_PM
static int bcmpmu_fg_resume(struct platform_device *pdev)
{
}
static int bcmpmu_fg_suspend(struct platform_device *pdev, pm_message_t state)
{

}
#else
#define bcmpmu_fg_resume 	NULL
#define bcmpmu_fg_suspend	NULL
#endif

static int __devexit bcmpmu_fg_remove(struct platform_device *pdev)
{
	struct bcmpmu_fg *fg = platform_get_drvdata(pdev);

	/* Disable FG */
	bcmpmu_fg_enable(false);
	power_supply_unregister(fg->psy);
	kfree(fg);
}

static int __devinit bcmpmu_fg_probe(struct platform_device *pdev)
{
	struct bcmpmu_fg *fg;
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	int ret = 0;

	fg_dbg(BCMPMU_LOG_INIT, "%s\n", __func__);

	if (!bcmpmu->bcmpmu_fg_pdata) {
		fg_dbg(BCMPMU_LOG_ERR, "%s: pdata is NULL\n");
		return -EINVAL;
	}

	fg = (struct bcmpmu_fg *)kzalloc(sizeof(struct bcmpmu_fg), GFP_KERNEL);
	if (!fg) {
		fg_dbg(BCMPMU_LOG_ERR, "%s: kzalloc failed!!\n", __func__);
		ret = -ENOMEM;
		goto err_exit;
	}

	fg->bcmpmu = bcmpmu;
	fg->pdata = bcmpmu->bcmpmu_fg_pdata;

	fg->psy.name = "battary";
	fg->psy.type = POWER_SUPPLY_TYPE_BATTERY;
	fg->psy.properties = bcmpmu_fg_props;
	fg->psy.num_properties = ARRAY_SIZE(bcmpmu_fg_props);
	fg->psy.get_property = bcmpmu_fg_get_properties;
	fg->psy.set_property = bcmpmu_fg_set_properties;
	fg->psy.external_power_changed = bcmpmu_fg_external_power_changed;

	fg->fg_wq = create_singlethread_workqueue("bcmpmu_fg_wq");
	if (IS_ERR_OR_NULL(fg->fg_wq)) {
		ret = PTR_ERR(fg->fg_wq);
		goto err_exit;
	}

	/**
	 * Dont want to keep CPU busy with this work when CPU is idle
	 */
	INIT_DELAYED_WORK_DEFERRABLE(&fg->fg_periodic_work,
			bcmpmu_fg_periodic_work);
	INIT_DELAYED_WORK(&fg->fg_cal_work, bcmpmu_fg_cal_work);
	INIT_DELAYED_WORK(&fg->fg_low_batt_work, bcmpmu_fg_low_batt_work);

	fg->fg_state = FG_STATE_INIT_CAP;
	bcmpmu_fg_init_pmu_regs(fg);

	ret = power_supply_register(pdev->dev, &fg->psy);
	if (ret) {
		fg_dbg("%s: Failed to register power supply\n", __func__);
		goto err_exit;
	}
	/**
	 * Run FG algorithm now
	 */
	queue_delayed_work(&fg->wq, &fg->fg_periodic_work, 0);

#ifdef CONFIG_DEBUG_FS
	bcmpmu_fg_debugfs_init(fg);
#endif
	return ret;
err_exit:
	if (!IS_ERR_OR_NULL(fg)) {
		if (fg->fg_wq)
			destroy_workqueue(&fg->fg_wq);
		kfree(fg);
	}
	return ret;
}

static struct platform_driver bcmpmu_fg_driver = {
	.driver = {
		.name = "bcmpmu_fg",
	},
	.probe = bcmpmu_fg_probe,
	.remove = __devexit_p(bcmpmu_fg_remove),
	.suspend = bcmpmu_fg_suspend,
	.resume = bcmpmu_fg_resume,

};

static int __init bcmpmu_fg_init(void)
{
	return platform_driver_register(&bcmpmu_fg_driver);
}
module_init(bcmpmu_fg_init);

static void __exit bcmpmu_fg_exit(void)
{
	platform_driver_unregister(&bcmpmu_fg_driver);
}
module_exit(bcmpmu_em_exit);

MODULE_DESCRIPTION("Broadcom PMU Fuel Gauge Driver");
MODULE_LICENSE("GPL");
