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
#include <linux/ktime.h>
#include <linux/sort.h>
#include <linux/wakelock.h>
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#endif
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>
#include <linux/power/bcmpmu-fg.h>


static u32 debug_mask = BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT | \
			BCMPMU_PRINT_FLOW;
#define pr_acld(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info("[ACLD]:"args); \
		} \
	} while (0)

#define to_bcmpmu_acld_data(ptr, mem)	container_of((ptr), \
		struct bcmpmu_acld, mem)

#define BATT_ONE_C_RATE_DEF		1500
#define ACLD_WORK_POLL_TIME		(5000)
#define ACLD_DELAY_500			500
#define ACLD_DELAY_1000			1000
#define ACLD_RETRIES			10
#define ACLD_VBUS_MARGIN		200 /* 200mV */
#define ACLD_VBUS_THRS			6000
#define ACLD_VBAT_THRS			3000
#define ACLD_CC_LIMIT			1500 /* mA */
#define ACLD_VBUS_ON_LOW_THRLD		4400
#define ACLD_ERR_CNT_THRLD		2
#define ADC_VBUS_AVG_SAMPLES		8
#define ADC_READ_TRIES			10
#define ADC_RETRY_DELAY		20 /* 20ms */
#define CHRGR_EFFICIENCY	85

struct bcmpmu_acld {
	struct bcmpmu59xxx *bcmpmu;
	struct mutex mutex;
	struct bcmpmu_acld_pdata *pdata;
	struct workqueue_struct *acld_wq;
	struct delayed_work acld_work;
	struct notifier_block usb_det_nb;
	enum bcmpmu_chrgr_type_t chrgr_type;
	bool acld_en;
	bool acld_init;
	int acld_rtry_cnt;
	int acld_err_cnt;
	int i_inst_err_cnt;
};

static int cmp(const void *a, const void *b)
{
	if (*((int *)a) < *((int *)b))
		return -1;
	if (*((int *)a) > *((int *)b))
		return 1;
	return 0;
}
/**
 * calculates interquartile mean of the integer data set @data
 * @size is the number of samples. It is assumed that
 * @size is divisible by 4 to ease the calculations
 */

static int interquartile_mean(int *data, int num)
{
	int i, j;
	int avg = 0;

	sort(data, num, sizeof(int), cmp, NULL);

	i = num / 4;
	j = num - i;

	for ( ; i < j; i++)
		avg += data[i];

	avg = avg / (j - (num / 4));

	return avg;
}
int bcmpmu_get_vbus(struct bcmpmu59xxx *bcmpmu)
{
	struct bcmpmu_adc_result result;
	int ret;
	int retries = ADC_READ_TRIES;

	while (retries--) {
		ret = bcmpmu_adc_read(bcmpmu, PMU_ADC_CHANN_VBUS,
				PMU_ADC_REQ_SAR_MODE, &result);
		if (!ret)
			break;
		msleep(ADC_RETRY_DELAY);
	}
	BUG_ON(retries <= 0);
	return result.conv;
}
int bcmpmu_get_avg_vbus(struct bcmpmu59xxx *bcmpmu)
{
	int i = 0;
	int vbus_samples[ADC_VBUS_AVG_SAMPLES];

	do {
		vbus_samples[i] = bcmpmu_get_vbus(bcmpmu);
		i++;
		msleep(25);
	} while (i < ADC_VBUS_AVG_SAMPLES);

	return interquartile_mean(vbus_samples, ADC_VBUS_AVG_SAMPLES);
}
static int bcmpmu_get_inductor_current(struct bcmpmu_acld *acld)
{
	int i_ind;
	int vbus;
	int vbat;
	int usb_fc_cc;

	vbus = bcmpmu_get_vbus(acld->bcmpmu);
	vbat = bcmpmu_fg_get_batt_volt(acld->bcmpmu);
	usb_fc_cc = bcmpmu_get_icc_fc(acld->bcmpmu);

	pr_acld(VERBOSE, "vbus = %d vbat = %d usb_fc_cc = %d\n",
			vbus, vbat, usb_fc_cc);
	i_ind = (usb_fc_cc * vbus * CHRGR_EFFICIENCY) / (vbat * 100);

	return i_ind;
}

static int bcmpmu_acld_enable(struct bcmpmu_acld *acld, bool enable)
{
	int ret;
	u8 reg;

	ret = acld->bcmpmu->read_dev(acld->bcmpmu,
			PMU_REG_OTG_BOOSTCTRL3, &reg);
	if (!ret) {
		if (enable)
			reg |= ACLD_ENABLE_MASK;
		else
			reg &= ~ACLD_ENABLE_MASK;
		ret = acld->bcmpmu->write_dev(acld->bcmpmu,
					PMU_REG_OTG_BOOSTCTRL3, reg);
	} else
		BUG_ON(1);

	return ret;
}

static bool bcmpmu_get_mbc_cv_status(struct bcmpmu_acld *acld)
{
	int ret;
	u8 reg;

	ret = acld->bcmpmu->read_dev(acld->bcmpmu, PMU_REG_ENV3, &reg);
	if (reg & ENV3_P_MBC_CV)
		return true;

	return false;
}
void bcmpmu_restore_cc_trim_otp(struct bcmpmu_acld *acld)
{
	int ret = 0;
	u8 reg;

	reg = acld->pdata->otp_cc_trim;
	ret = acld->bcmpmu->write_dev(acld->bcmpmu, PMU_REG_MBCCTRL18, reg);
	if (ret)
		BUG_ON(1);

}

static void bcmpmu_chrg_on_output(struct bcmpmu_acld *acld)
{
	bcmpmu_acld_enable(acld, false);
	acld->acld_en = false;
	bcmpmu_restore_cc_trim_otp(acld);
	bcmpmu_set_icc_fc(acld->bcmpmu, acld->pdata->i_def_dcp);
	pr_acld(INIT, "ACLD disabled and charging on O/P\n");
}

static void bcmpmu_check_battery_current_limit(struct bcmpmu_acld *acld)
{
#if 0
	ktime_t t_now;
	ktime_t t_diff;
	u64 t_ms;
#endif
	int i_inst;/* Instantaneous battery current */
	int i_ind; /* inductor current */
	int ret;

	i_ind = bcmpmu_get_inductor_current(acld);

	ret = bcmpmu_fg_get_batt_curr(acld->bcmpmu, &i_inst);
	if (ret) {
		acld->i_inst_err_cnt++;
		pr_acld(INIT, "Battery current read Error:%d\n",
				acld->i_inst_err_cnt);
		if (acld->i_inst_err_cnt > ACLD_ERR_CNT_THRLD)
			bcmpmu_chrg_on_output(acld);

	} else
		acld->i_inst_err_cnt = 0;

	pr_acld(VERBOSE, "i_inst = %d, i_ind = %d\n", i_inst, i_ind);

	/* check if the battery current is above 1C limit or
	 * inductor current  is above iSat
	 * */
	if (((i_inst >= acld->pdata->one_c_rate) ||
			(i_ind >= acld->pdata->i_sat)) &&
			(acld->acld_err_cnt++ > ACLD_ERR_CNT_THRLD)) {
		pr_acld(ERROR, "i_inst = %d, i_ind = %d\n", i_inst, i_ind);
		bcmpmu_chrg_on_output(acld);

	} else if ((i_inst < acld->pdata->one_c_rate) &&
			(i_ind < acld->pdata->i_sat) &&
			(acld->acld_en) &&
			(acld->acld_err_cnt > 0)) {
		pr_acld(ERROR, "ACLD Error Count:%d restarted\n",
				acld->acld_err_cnt);
		acld->acld_err_cnt = 0;
	}
}
static int bcmpmu_get_vbus_resistance(struct bcmpmu_acld *acld,
					int vbus_chrg_off, int vbus_chrg_on)
{
	int vbus_res;
	int icc_fc = bcmpmu_get_icc_fc(acld->bcmpmu);

	pr_acld(INIT, "%s: vbus_chrgr_off: %d vbus_chrgr_on: %d icc_fc = %d\n",
			__func__,
			vbus_chrg_off,
			vbus_chrg_on,
			icc_fc);

	if (!icc_fc)
		vbus_res = ((vbus_chrg_off - vbus_chrg_on) * 1000);
	else
		vbus_res = ((vbus_chrg_off - vbus_chrg_on) * 1000) / icc_fc;

	if (vbus_res <= 0) {
		pr_acld(INIT, "vbus_res = %d\n", vbus_res);
		vbus_res = 0;
	}

	return vbus_res;
}

static int bcmpmu_get_vbus_load(struct bcmpmu_acld *acld, int vbus_chrg_off,
		int vbus_res)
{
	int icc_fc = bcmpmu_get_icc_fc(acld->bcmpmu);
	int vbus_load;

	pr_acld(INIT, "icc_fc = %d\n", icc_fc);

	vbus_load = (vbus_chrg_off - ((icc_fc * vbus_res) / 1000))
						- acld->pdata->acld_vbus_margin;
	if (vbus_load <= 0) {
		pr_acld(INIT, "vbus_off = %d vbus_res = %d\n",
				vbus_chrg_off, vbus_res);
		WARN_ON(1);
		vbus_load = vbus_chrg_off - acld->pdata->acld_vbus_margin;
	}

	return vbus_load;
}

static bool bcmpmu_get_ubpd_int(struct bcmpmu_acld *acld)
{
	int ret;

	bcmpmu_usb_get(acld->bcmpmu, BCMPMU_USB_CTRL_GET_UBPD_INT, &ret);
	if (ret != 1)
		return false;

	return true;

}
static bool bcmpmu_is_usb_valid(struct bcmpmu_acld *acld)
{
	int ret;

	bcmpmu_usb_get(acld->bcmpmu, BCMPMU_USB_CTRL_GET_USB_VALID, &ret);
	if (ret != 1)
		return false;

	return true;

}
static bool bcmpmu_is_vbus_vbat_valid(struct bcmpmu_acld *acld)
{
	int vbus;
	int vbat;

	vbus = bcmpmu_get_avg_vbus(acld->bcmpmu);
	vbat = bcmpmu_fg_get_avg_volt(acld->bcmpmu);

	if ((vbus > acld->pdata->acld_vbus_thrs) ||
			(vbat < acld->pdata->acld_vbat_thrs))
		return false;

	return true;

}

static void bcmpmu_acld_algo(struct bcmpmu_acld *acld)
{

	bool cc_lmt_hit = false;
	bool fault = false;
	bool mbc_in_cv = false;
	int vbus_chrg_off;
	int vbus_chrg_on;
	int vbus_res;
	int vbus_load;
	int ret;
	int usb_fc_cc_reached;
	int usb_fc_cc_prev;
	int usb_fc_cc_next;

	pr_acld(INIT, "%s\n", __func__);



	/* Return from ACLD if,
	 * DCP is quickly removed after insertion.
	 * */
	if (!bcmpmu_get_ubpd_int(acld)) {
		fault = true;
		goto chrgr_pre_chk;
	}

	msleep(ACLD_DELAY_1000);

	if (!bcmpmu_get_ubpd_int(acld)) {
		fault = true;
		goto chrgr_pre_chk;
	}

	if (!bcmpmu_is_vbus_vbat_valid(acld)) {
		pr_acld(ERROR, "VBUS/VBAT crossed Thresholds\n");
		bcmpmu_set_icc_fc(acld->bcmpmu, PMU_DCP_DEF_CURR_LMT);
		bcmpmu_chrgr_usb_en(acld->bcmpmu, 1);
		fault = true;
		goto chrgr_pre_chk;
	}


	vbus_chrg_off = bcmpmu_get_avg_vbus(acld->bcmpmu);
	pr_acld(INIT, "vbus_chrg_off = %d\n", vbus_chrg_off);

	bcmpmu_acld_enable(acld, true);
	/* set USB_FC_CC at OTP and USB_CC_TRIM at 0*/
	bcmpmu_set_cc_trim(acld->bcmpmu, PMU_USB_CC_ZERO_TRIM);
	bcmpmu_set_icc_fc(acld->bcmpmu, PMU_USB_FC_CC_OTP);
	bcmpmu_chrgr_usb_en(acld->bcmpmu, 1);

	msleep(ACLD_DELAY_500);

	if (bcmpmu_get_mbc_cv_status(acld)) {
		pr_acld(INIT, "MBC in CV\n");
		bcmpmu_chrg_on_output(acld);
		return;
	}

	vbus_chrg_on = bcmpmu_get_avg_vbus(acld->bcmpmu);
	vbus_res = bcmpmu_get_vbus_resistance(acld,
			vbus_chrg_off, vbus_chrg_on);
	pr_acld(INIT, "vbus_res(uohm) = %d\n", vbus_res);
	vbus_load = bcmpmu_get_vbus_load(acld, vbus_chrg_off, vbus_res);

	pr_acld(INIT, "Tuning USB_FC_CC\n");
	do {
		pr_acld(INIT, "vbus_chrg_on: %d vbus_res: %d vbus_load: %d\n",
				vbus_chrg_on, vbus_res, vbus_load);

		/* Check for Charger Errors and battery CV mode */
		if (!bcmpmu_get_ubpd_int(acld)) {
			fault = true;
			goto chrgr_pre_chk;
		}
		if (bcmpmu_get_mbc_cv_status(acld)) {
			mbc_in_cv = true;
			break;
		}

		usb_fc_cc_prev = bcmpmu_get_icc_fc(acld->bcmpmu);

		if ((vbus_chrg_on < vbus_load) ||
				(vbus_chrg_on <= ACLD_VBUS_ON_LOW_THRLD)) {
			if (vbus_chrg_on < vbus_load)
				pr_acld(INIT, "vbus_chrg_on < vbus_load\n");
			else
				pr_acld(INIT, "VBUS Low Threshold hit\n");
			break;
		} else {
			/* Increase CC  */
			pr_acld(INIT, "Icreasing CC by one step\n");
			ret = bcmpmu_icc_fc_step_up(acld->bcmpmu);
			if (ret) {
				pr_acld(INIT, "Reached Maximum CC\n");
				break;
			}

			if (bcmpmu_get_icc_fc(acld->bcmpmu) >
					acld->pdata->acld_cc_lmt) {
				pr_acld(INIT, "Recommended DCP CC lmt hit\n");
				cc_lmt_hit = true;
				break;
			}

		}
		msleep(ACLD_DELAY_500);

		usb_fc_cc_next = bcmpmu_get_icc_fc(acld->bcmpmu);
		if (usb_fc_cc_next <= usb_fc_cc_prev) {
			pr_acld(INIT, "PMU CC lmt hit: prev = %d next = %d\n",
					usb_fc_cc_prev, usb_fc_cc_next);
			break;
		}

		vbus_chrg_on = bcmpmu_get_avg_vbus(acld->bcmpmu);
		vbus_load = bcmpmu_get_vbus_load(acld, vbus_chrg_off,
				vbus_res);

	} while (true);

	bcmpmu_icc_fc_step_down(acld->bcmpmu);
	usb_fc_cc_reached = bcmpmu_get_icc_fc(acld->bcmpmu);

	if (mbc_in_cv) {
		pr_acld(INIT, "MBC in CV Mode\n");
		goto chrgr_pre_chk;
	}
	if (cc_lmt_hit)
		goto chrgr_pre_chk;

	pr_acld(INIT, "Tuning USB_CC_TRIM\n");
	bcmpmu_cc_trim_up(acld->bcmpmu);
	msleep(ACLD_DELAY_500);
	vbus_chrg_on = bcmpmu_get_avg_vbus(acld->bcmpmu);
	vbus_load = bcmpmu_get_vbus_load(acld, vbus_chrg_off, vbus_res);

	do {
		pr_acld(INIT, "vbus_chrg_on: %d vbus_res: %d vbus_load: %d\n",
				vbus_chrg_on, vbus_res, vbus_load);

		if (!bcmpmu_get_ubpd_int(acld)) {
			fault = true;
			goto chrgr_pre_chk;
		}
		if (bcmpmu_get_mbc_cv_status(acld)) {
			pr_acld(INIT, "MBC in CV, Exiting Trim Tuning\n");
			break;
		}

		if ((vbus_chrg_on < vbus_load) ||
				(vbus_chrg_on < ACLD_VBUS_ON_LOW_THRLD)) {
			if (vbus_chrg_on < vbus_load)
				pr_acld(INIT, "vbus_chrg_on < vbus_load\n");
			else
				pr_acld(INIT, "VBUS Low Threshold hit\n");
			break;
		} else {
			/* Increase Trim code */
			pr_acld(INIT, "Increasing Trim code by one step\n");
			ret = bcmpmu_cc_trim_up(acld->bcmpmu);
			if (ret) {
				pr_acld(INIT, "Reached Maximum Trim code\n");
				break;
			}
		}
		msleep(ACLD_DELAY_500);
		vbus_chrg_on = bcmpmu_get_avg_vbus(acld->bcmpmu);
		vbus_load = bcmpmu_get_vbus_load(acld, vbus_chrg_off,
				vbus_res);
	} while (true);

	bcmpmu_cc_trim_down(acld->bcmpmu);
	bcmpmu_cc_trim_down(acld->bcmpmu);
	bcmpmu_set_icc_fc(acld->bcmpmu, PMU_USB_FC_CC_OTP);
	msleep(ACLD_DELAY_500);
	bcmpmu_set_icc_fc(acld->bcmpmu, usb_fc_cc_reached);
chrgr_pre_chk:
	/* charger presence check */
	if ((!bcmpmu_is_usb_valid(acld)) || fault) {
		pr_acld(INIT, "Charger Error, restoring cc trim\n");
		bcmpmu_restore_cc_trim_otp(acld);
		return;
	}
	acld->acld_en = true;
	return;

}
static void bcmpmu_acld_work(struct work_struct *work)
{
	struct bcmpmu_acld *acld = to_bcmpmu_acld_data(work, acld_work.work);
	int ret;

	if (acld->chrgr_type != PMU_CHRGR_TYPE_DCP)
		return;

	if (!acld->acld_init) {
		ret = bcmpmu_chrgr_usb_en(acld->bcmpmu, 0);
		if (ret) {
			pr_acld(ERROR, "Rescheduling ACLD\n");
			if (acld->acld_rtry_cnt++ > ACLD_RETRIES)
				BUG_ON(1);
			goto exit;
		}
		acld->acld_rtry_cnt = 0;

		bcmpmu_acld_algo(acld);
		acld->acld_init = true;
	}

	if ((acld->bcmpmu->flags & BCMPMU_ACLD_EN) &&
			(acld->chrgr_type == PMU_CHRGR_TYPE_DCP) &&
			(acld->acld_en))
		bcmpmu_check_battery_current_limit(acld);

exit:
	queue_delayed_work(acld->acld_wq, &acld->acld_work,
			msecs_to_jiffies(ACLD_WORK_POLL_TIME));
	return;
}
static int bcmpmu_acld_event_handler(struct notifier_block *nb,
		unsigned long event, void *data)
{
	struct bcmpmu_acld *acld;

	switch (event) {
	case PMU_ACCY_EVT_OUT_CHRGR_TYPE:
		pr_acld(VERBOSE, "PMU_ACCY_EVT_OUT_CHRGR_TYPE\n");
		acld = to_bcmpmu_acld_data(nb, usb_det_nb);
		acld->chrgr_type = *(enum bcmpmu_chrgr_type_t *)data;
		if ((acld->chrgr_type == PMU_CHRGR_TYPE_NONE) &&
				(acld->bcmpmu->flags & BCMPMU_ACLD_EN)) {

			cancel_delayed_work_sync(&acld->acld_work);
			bcmpmu_acld_enable(acld, false);
			bcmpmu_restore_cc_trim_otp(acld);
			acld->acld_init = false;
			acld->i_inst_err_cnt = 0;
			acld->acld_err_cnt = 0;
		} else if ((acld->bcmpmu->flags & BCMPMU_ACLD_EN) &&
				(acld->chrgr_type == PMU_CHRGR_TYPE_DCP)) {
			queue_delayed_work(acld->acld_wq, &acld->acld_work, 0);
		}
	}

	return 0;
}
static int debug_pmu_acld_ctrl(void *data, u64 acld_ctrl)
{
	struct bcmpmu_acld *acld = data;
	struct bcmpmu59xxx *bcmpmu = acld->bcmpmu;

	if ((bcmpmu->rev_info.prj_id == BCMPMU_59054_ID) &&
			(bcmpmu->rev_info.ana_rev >= BCMPMU_59054A1_ANA_REV)) {

		if (acld_ctrl) {
			bcmpmu->flags |= BCMPMU_ACLD_EN;
			bcmpmu_acld_enable(acld, true);
			pr_acld(INIT, "ACLD Enabled\n");
		} else {
			bcmpmu->flags &= ~BCMPMU_ACLD_EN;
			bcmpmu_acld_enable(acld, false);
			pr_acld(INIT, "ACLD Disabled\n");
		}


	} else
		pr_acld(INIT, "ACLD not supported on this PMU Revision\n");

	pr_acld(INIT, " ACLD = %d\n", (bcmpmu->flags & BCMPMU_ACLD_EN));

	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(debug_pmu_acld_fops,
		NULL, debug_pmu_acld_ctrl, "%llu\n");

static void bcmpmu_acld_debugfs_init(struct bcmpmu_acld *acld)
{
	struct dentry *dentry_acld_dir;
	struct dentry *dentry_acld_file;
	struct bcmpmu59xxx *bcmpmu = acld->bcmpmu;
	if (!bcmpmu || !bcmpmu->dent_bcmpmu) {
		pr_acld(ERROR, "%s: dentry_bcmpmu is NULL", __func__);
		return;
	}

	dentry_acld_dir = debugfs_create_dir("ACLD", bcmpmu->dent_bcmpmu);
	if (IS_ERR_OR_NULL(dentry_acld_dir))
		goto debugfs_clean;

	dentry_acld_file = debugfs_create_file("acld_ctrl",
			S_IWUSR | S_IRUSR, dentry_acld_dir, acld,
			&debug_pmu_acld_fops);
	if (IS_ERR_OR_NULL(dentry_acld_file))
		goto debugfs_clean;

	return;

debugfs_clean:
	if (!IS_ERR_OR_NULL(dentry_acld_dir))
		debugfs_remove_recursive(dentry_acld_dir);
}
static int __devexit bcmpmu_acld_remove(struct platform_device *pdev)
{
	return 0;
}

static int __devinit bcmpmu_acld_probe(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu_acld *acld;
	int ret = 0;

	printk(KERN_DEBUG "%s: called\n", __func__);

	acld = kzalloc(sizeof(struct bcmpmu_acld), GFP_KERNEL);
	if (acld == NULL) {
		pr_acld(ERROR, "%s failed to alloc mem.\n", __func__);
		return -ENOMEM;
	}
	bcmpmu->acld = (void *)acld;
	acld->bcmpmu = bcmpmu;

	acld->pdata = (struct bcmpmu_acld_pdata *)pdev->dev.platform_data;
	if (!acld->pdata->one_c_rate)
		acld->pdata->one_c_rate = BATT_ONE_C_RATE_DEF;
	if (!acld->pdata->i_def_dcp)
		acld->pdata->i_def_dcp = PMU_DCP_DEF_CURR_LMT;
	if (!acld->pdata->i_sat)
		acld->pdata->i_sat = PMU_TYP_SAT_CURR;
	if (!acld->pdata->acld_vbus_margin)
		acld->pdata->acld_vbus_margin = ACLD_VBUS_MARGIN;
	if (!acld->pdata->acld_vbus_thrs)
		acld->pdata->acld_vbus_thrs = ACLD_VBUS_THRS;
	if (!acld->pdata->acld_vbat_thrs)
		acld->pdata->acld_vbat_thrs = ACLD_VBAT_THRS;
	if (!acld->pdata->acld_cc_lmt)
		acld->pdata->acld_cc_lmt = ACLD_CC_LIMIT;
	if (!acld->pdata->otp_cc_trim)
		acld->pdata->otp_cc_trim = PMU_OTP_CC_TRIM;

	acld->usb_det_nb.notifier_call = bcmpmu_acld_event_handler;
	ret = bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_CHRGR_TYPE,
			&acld->usb_det_nb);
	if (ret) {
		pr_acld(ERROR, "%s Failed to add notifier\n", __func__);
		goto error;
	}

	acld->acld_wq = create_singlethread_workqueue("bcmpmu_acld_wq");
	if (IS_ERR_OR_NULL(acld->acld_wq)) {
		ret = PTR_ERR(acld->acld_wq);
		pr_acld(ERROR, "%s Failed to create WQ\n", __func__);
		goto unreg_usb_det_nb;
	}

	INIT_DELAYED_WORK(&acld->acld_work, bcmpmu_acld_work);

#ifdef CONFIG_DEBUG_FS
	bcmpmu_acld_debugfs_init(acld);
#endif

	return 0;

unreg_usb_det_nb:
	bcmpmu_remove_notifier(PMU_ACCY_EVT_OUT_CHRGR_TYPE,
			&acld->usb_det_nb);
error:
	kfree(acld);
	return 0;
}

static struct platform_driver bcmpmu_acld_driver = {
	.driver = {
		.name = "bcmpmu_acld",
	},
	.probe = bcmpmu_acld_probe,
	.remove = __devexit_p(bcmpmu_acld_remove),
};

static int __init bcmpmu_acld_init(void)
{
	return platform_driver_register(&bcmpmu_acld_driver);
}
module_init(bcmpmu_acld_init);

static void __exit bcmpmu_acld_exit(void)
{
	platform_driver_unregister(&bcmpmu_acld_driver);
}
module_exit(bcmpmu_acld_exit);

MODULE_DESCRIPTION("Broadcom PMU ACLD Driver");
MODULE_LICENSE("GPL");
