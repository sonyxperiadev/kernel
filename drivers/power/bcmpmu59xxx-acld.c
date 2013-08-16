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


static u32 debug_mask = 0xFF; /* BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT | \
			BCMPMU_PRINT_FLOW; */
#define pr_acld(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info("[ACLD]:"args); \
		} \
	} while (0)

#define to_bcmpmu_acld_data(ptr, mem)	container_of((ptr), \
		struct bcmpmu_acld, mem)

/* Battery should not be chrged more than 80% 1C rate for better lifecycle
 * */
#define safe_one_c_rate(one_c) ((one_c * 80) / 100)

#define BATT_ONE_C_RATE_DEF		1500
#define ACLD_WORK_POLL_5S		5000
#define ACLD_WORK_POLL_2M		120000
#define ACLD_DELAY_500			500
#define ACLD_DELAY_1000			1000
#define ACLD_RETRIES			10
#define ACLD_VBUS_MARGIN		200 /* 200mV */
#define ACLD_VBUS_THRS			5950
#define ACLD_VBAT_THRS			3500
#define ACLD_CC_LIMIT			1360 /* mA */
#define ACLD_VBUS_ON_LOW_THRLD		4400
#define ACLD_ERR_CNT_THRLD_2		2
#define ACLD_ERR_CNT_THRLD_10		10
#define ADC_VBUS_AVG_SAMPLES		8
#define ADC_READ_TRIES			10
#define ADC_RETRY_DELAY			20 /* 20ms */
#define CHRGR_EFFICIENCY		85
#define VBUS_VBAT_DELTA			1000 /* mV */
struct bcmpmu_acld {
	struct bcmpmu59xxx *bcmpmu;
	struct mutex mutex;
	struct bcmpmu_acld_pdata *pdata;
	struct workqueue_struct *acld_wq;
	struct delayed_work acld_work;
	struct notifier_block usb_det_nb;
	struct notifier_block tml_trtle_nb;
	ktime_t last_sample_tm;
	enum bcmpmu_chrgr_type_t chrgr_type;
	int acld_min_input;
	int acld_rtry_cnt;
	int batt_curr_err_cnt;
	int vbus_vbat_thrs_err_cnt;
	int vbus_vbat_delta_deb;
	int i_sys;
	int acld_wrk_poll_time;
	bool i_bus_abv_lmt;
	bool acld_re_init;
	bool acld_en;
	bool acld_init;
	bool usb_mbc_fault;
	bool volt_thrs_check;
	bool v_flag;
	bool acld_start;
	bool mbc_in_cv;
	bool tml_trtle_stat;
};

static bool bcmpmu_usb_mbc_fault_check(struct bcmpmu_acld *acld);
static int bcmpmu_reset_acld_flags(struct bcmpmu_acld *acld);

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
static int bcmpmu_acld_get_inductor_curr(struct bcmpmu_acld *acld)
{
	int i_ind;
	int vbus;
	int vbat;
	int usb_fc_cc;
	int usb_cc_trim;

	vbus = bcmpmu_get_vbus(acld->bcmpmu);
	vbat = bcmpmu_fg_get_batt_volt(acld->bcmpmu);
	usb_fc_cc = bcmpmu_get_icc_fc(acld->bcmpmu);
	usb_cc_trim = bcmpmu_get_trim_curr(acld->bcmpmu);

	pr_acld(VERBOSE, "vbus: %d vbat: %d usb_fc_cc: %d usb_cc_trim: %d\n",
			vbus, vbat, usb_fc_cc, usb_cc_trim);
	i_ind = ((usb_fc_cc + usb_cc_trim) * vbus * CHRGR_EFFICIENCY) /
								(vbat * 100);

	return i_ind;
}

bool bcmpmu_is_acld_enabled(struct bcmpmu59xxx *bcmpmu)
{
	int ret;
	u8 reg;

	if (bcmpmu->flags & BCMPMU_ACLD_EN) {
		ret = bcmpmu->read_dev(bcmpmu, PMU_REG_OTG_BOOSTCTRL3, &reg);
		if (reg & ACLD_ENABLE_MASK)
			return true;
	}

	return false;
}
EXPORT_SYMBOL(bcmpmu_is_acld_enabled);

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
	if (reg & ENV3_P_MBC_CV) {
		pr_acld(VERBOSE, "MBC is in CV mode\n");
		return true;
	}

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
	bcmpmu_chrgr_usb_en(acld->bcmpmu, 1);
	pr_acld(INIT, "ACLD disabled and charging on O/P\n");
}

static void bcmpmu_acld_get_min_input(struct bcmpmu_acld *acld)
{
	int one_c_rate = 0;
	int retries = ADC_READ_TRIES;
	int ret;

	while (retries--) {
		ret = bcmpmu_fg_get_one_c_rate(acld->bcmpmu, &one_c_rate);
		if (!ret)
			break;
		msleep(ADC_RETRY_DELAY);
	}
	if ((retries <= 0) || (!one_c_rate))
		BUG_ON(1);

	one_c_rate = safe_one_c_rate(one_c_rate);

	/* 1C rate, Max CC and ISat are the inputs for
	 * ACLD. CC  should not excedd the minimum of
	 * above three inputs
	 * */
	acld->acld_min_input = min3(one_c_rate, acld->pdata->i_max_cc,
			acld->pdata->i_sat);

	pr_acld(INIT, "acld min input = %d\n", acld->acld_min_input);
}
static int bcmpmu_acld_get_batt_curr(struct bcmpmu_acld *acld)
{
	int batt_curr = 0;
	int ret;
	int retries = ADC_READ_TRIES;

	while (retries--) {
		ret = bcmpmu_fg_get_batt_curr(acld->bcmpmu, &batt_curr);
		if (!ret)
			break;
		msleep(ADC_RETRY_DELAY);
	}
	BUG_ON(retries <= 0);
	pr_acld(VERBOSE, "Battery current = %d\n", batt_curr);
	return batt_curr;
}

static void bcmpmu_check_battery_current_limit(struct bcmpmu_acld *acld)
{
#if 0
	ktime_t t_now;
	ktime_t t_diff;
	u64 t_ms;
#endif
	int i_inst;/* Instantaneous battery current */

	i_inst = bcmpmu_acld_get_batt_curr(acld);

	/* check if the battery current is above 1C limit or
	 * inductor current  is above iSat
	 * */

	if ((i_inst >= acld->acld_min_input) &&
			(acld->batt_curr_err_cnt++ > ACLD_ERR_CNT_THRLD_2)) {
		pr_acld(ERROR, "i_inst = %d\n", i_inst);
		bcmpmu_chrg_on_output(acld);
		pr_acld(ERROR, "%s: Re init ACLD\n", __func__);
		acld->acld_re_init = true;
		acld->batt_curr_err_cnt = 0;
	} else if ((i_inst < acld->acld_min_input) &&
			(acld->acld_en) &&
			(acld->batt_curr_err_cnt > 0)) {
		pr_acld(ERROR, "ACLD Error Count:%d restarted\n",
				acld->batt_curr_err_cnt);
		acld->batt_curr_err_cnt = 0;
	}
}
static void bcmpmu_acld_re_init_check(struct bcmpmu_acld *acld)
{
	ktime_t t_now;
	ktime_t t_diff;
	u64 t_ms;
	int vbus;
	int vbat;

	t_now = ktime_get();
	t_diff = ktime_sub(t_now, acld->last_sample_tm);
	t_ms = ktime_to_ms(t_diff);

	if (t_ms < ACLD_WORK_POLL_2M)
		return;

	acld->last_sample_tm = t_now;

	if (!bcmpmu_usb_mbc_fault_check(acld)) {
		pr_acld(INIT, "%s: USB/MBC fault\n", __func__);
		return;
	}

	vbus = bcmpmu_get_avg_vbus(acld->bcmpmu);
	vbat = bcmpmu_fg_get_avg_volt(acld->bcmpmu);
	pr_acld(FLOW, "%s, vbus = %d vbat = %d\n", __func__, vbus, vbat);
	/*Warning: Consider throttel condition also*/
	if ((vbus - vbat) > VBUS_VBAT_DELTA) {
		pr_acld(ERROR, "%s: Re init ACLD\n", __func__);
		acld->acld_re_init = true;
	}

	return;
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
	if (ret != 1) {
		pr_acld(FLOW, "USB presence not detected\n");
		return false;
	}

	return true;

}

static bool bcmpmu_get_usb_port_status(struct bcmpmu_acld *acld)
{
	int ret;

	bcmpmu_usb_get(acld->bcmpmu,
			BCMPMU_USB_CTRL_GET_USB_PORT_DISABLED, &ret);
	if (ret == 1) {
		pr_acld(FLOW, "USB port disabled\n");
		return false;
	}

	return true;

}

static bool bcmpmu_is_usb_valid(struct bcmpmu_acld *acld)
{
	int ret;

	bcmpmu_usb_get(acld->bcmpmu, BCMPMU_USB_CTRL_GET_USB_VALID, &ret);
	if (ret != 1) {
		pr_acld(FLOW, "USB charger is not valid\n");
		return false;
	}

	return true;

}
static bool bcmpmu_is_avg_vbus_vbat_valid(struct bcmpmu_acld *acld)
{
	int vbus;
	int vbat;

	vbus = bcmpmu_get_avg_vbus(acld->bcmpmu);
	vbat = bcmpmu_fg_get_avg_volt(acld->bcmpmu);
	pr_acld(VERBOSE, "%s: vbus = %d vbat = %d\n", __func__, vbus, vbat);
	if ((vbus > acld->pdata->acld_vbus_thrs) ||
			(vbat < acld->pdata->acld_vbat_thrs))
		return false;

	return true;

}

static bool bcmpmu_is_vbus_vbat_valid(struct bcmpmu_acld *acld)
{
	int vbus;
	int vbat;

	vbus = bcmpmu_get_vbus(acld->bcmpmu);
	vbat = bcmpmu_fg_get_batt_volt(acld->bcmpmu);
	pr_acld(VERBOSE, "%s: vbus = %d vbat = %d\n", __func__, vbus, vbat);
	if ((vbus > acld->pdata->acld_vbus_thrs) ||
			(vbat < acld->pdata->acld_vbat_thrs))
		return false;

	return true;

}

static bool bcmpmu_usb_mbc_fault_check(struct bcmpmu_acld *acld)
{
	bool ret;

	ret = bcmpmu_get_ubpd_int(acld);
	if (!ret) {
		pr_acld(FLOW, "USB presence not detected\n");
		return false;
	}
	ret = bcmpmu_get_usb_port_status(acld);
	if (!ret) {
		pr_acld(FLOW, "USB port disabled\n");
		return false;
	}
	ret = bcmpmu_get_mbc_faults(acld->bcmpmu);
	if (!ret) {
		pr_acld(FLOW, "MBC faults occured\n");
		return false;
	}

	return true;
}
static bool bcmpmu_is_ibus_in_limit(struct bcmpmu_acld *acld, int i_sys)
{
	int i_ind;
	int i_bat;
	int i_total; /* i_sys + i_bat */

	i_ind = bcmpmu_acld_get_inductor_curr(acld);
	i_bat = bcmpmu_acld_get_batt_curr(acld);
	i_total = i_bat + abs(i_sys);

	pr_acld(FLOW, "%s, i_ind = %d, i_total = %d\n",
			__func__, i_ind, i_total);
	if ((i_ind >= acld->acld_min_input) ||
			(i_total >= acld->acld_min_input))
		return false;

	return true;
}
static void bcmpmu_post_acld_start_event(struct bcmpmu_acld *acld)
{
	acld->acld_start = true;
	bcmpmu_call_notifier(acld->bcmpmu, PMU_ACLD_EVT_ACLD_STATUS,
			&acld->acld_start);
}
static void bcmpmu_post_acld_end_event(struct bcmpmu_acld *acld)
{
	acld->acld_start = false;
	bcmpmu_call_notifier(acld->bcmpmu, PMU_ACLD_EVT_ACLD_STATUS,
			&acld->acld_start);
}
static int bcmpmu_acld_algo(struct bcmpmu_acld *acld)
{
	bool fault = false;
	bool acld_cc_lmt_hit = false;
	int vbus_chrg_off;
	int vbus_chrg_on;
	int vbus_res;
	int vbus_load;
	int ret;
	int i_sys;
	int usb_fc_cc_reached;
	int usb_fc_cc_prev;
	int usb_fc_cc_next;

	pr_acld(INIT, "%s\n", __func__);
	bcmpmu_post_acld_start_event(acld);

	/* Return from ACLD if,
	 * DCP is quickly removed after insertion.
	 * */
	if (!bcmpmu_get_ubpd_int(acld)) {
		fault = true;
		goto chrgr_pre_chk;
	}

	bcmpmu_chrgr_usb_en(acld->bcmpmu, 0);

	msleep(ACLD_DELAY_1000);

	if (!bcmpmu_get_ubpd_int(acld)) {
		fault = true;
		goto chrgr_pre_chk;
	}

	vbus_chrg_off = bcmpmu_get_avg_vbus(acld->bcmpmu);
	pr_acld(INIT, "vbus_chrg_off = %d\n", vbus_chrg_off);
	/* get the system load */
	i_sys = bcmpmu_acld_get_batt_curr(acld);
	if (i_sys > 0)
		pr_acld(ERROR, "System load is +ve, though charging is off\n");

	bcmpmu_acld_enable(acld, true);
	/* set USB_FC_CC at OTP and USB_CC_TRIM at 0*/
	bcmpmu_set_cc_trim(acld->bcmpmu, PMU_USB_CC_ZERO_TRIM);
	bcmpmu_set_icc_fc(acld->bcmpmu, PMU_USB_FC_CC_OTP);
	bcmpmu_chrgr_usb_en(acld->bcmpmu, 1);

	msleep(ACLD_DELAY_500);

	if (bcmpmu_get_mbc_cv_status(acld)) {
		acld->mbc_in_cv = true;
		pr_acld(FLOW, "MBC in CV\n");
		bcmpmu_chrg_on_output(acld);
		bcmpmu_post_acld_end_event(acld);
		return 0;
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
			pr_acld(FLOW, "MBC in CV, Exiting CC Tuning\n");
			break;
		}

		if (!bcmpmu_is_ibus_in_limit(acld, i_sys)) {
			acld->i_bus_abv_lmt = true;
			pr_acld(INIT, "i_bus is beyond ACLD min input\n");
			break;
		} else
			acld->i_bus_abv_lmt = false;

		usb_fc_cc_prev = bcmpmu_get_icc_fc(acld->bcmpmu);


		if ((vbus_chrg_on < vbus_load) ||
				(vbus_chrg_on <= ACLD_VBUS_ON_LOW_THRLD)) {
			if (vbus_chrg_on < vbus_load)
				pr_acld(INIT, "vbus_chrg_on < vbus_load\n");
			else
				pr_acld(INIT, "VBUS Low Threshold hit\n");
			break;
		} else {
			if (bcmpmu_get_next_icc_fc(acld->bcmpmu) >=
					acld->pdata->acld_cc_lmt) {
				acld_cc_lmt_hit = true;
				pr_acld(INIT, "ACLC CC lmt hit\n");
				break;
			}
			/* Increase CC  */
			pr_acld(INIT, "Icreasing CC by one step\n");
			ret = bcmpmu_icc_fc_step_up(acld->bcmpmu);
			if (ret) {
				pr_acld(INIT, "Reached Maximum CC\n");
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

	if (!acld_cc_lmt_hit)
		bcmpmu_icc_fc_step_down(acld->bcmpmu);
	else if (acld_cc_lmt_hit && acld->pdata->qa_required)
		goto chrgr_pre_chk;

	usb_fc_cc_reached = bcmpmu_get_icc_fc(acld->bcmpmu);

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
		if (!bcmpmu_is_ibus_in_limit(acld, i_sys)) {
			acld->i_bus_abv_lmt = true;
			pr_acld(INIT, "i_bus is beyond ACLD min input\n");
			break;
		} else
			acld->i_bus_abv_lmt = false;

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
	if ((!bcmpmu_is_usb_valid(acld)) ||
			fault ||
			(!bcmpmu_is_avg_vbus_vbat_valid(acld)) ||
			(!bcmpmu_is_acld_enabled(acld->bcmpmu))) {
		bcmpmu_chrg_on_output(acld);
		bcmpmu_post_acld_end_event(acld);
		pr_acld(ERROR, "============ACLD Exit=============\n");
		return -EAGAIN;
	}
	acld->acld_en = true;
	bcmpmu_post_acld_end_event(acld);
	return 0;

}
static void bcmpmu_acld_periodic_monitor(struct bcmpmu_acld *acld)
{
	bool b_ret;

	bcmpmu_check_battery_current_limit(acld);

	if (acld->i_bus_abv_lmt)
		bcmpmu_acld_re_init_check(acld);

	b_ret = bcmpmu_is_vbus_vbat_valid(acld);
	if ((!b_ret) && (acld->vbus_vbat_thrs_err_cnt++ >
				ACLD_ERR_CNT_THRLD_2)) {
		bcmpmu_chrg_on_output(acld);
		pr_acld(FLOW, "%s:vbus/vbat thre err, start ACLD frm scratch\n",
				__func__);
		bcmpmu_reset_acld_flags(acld);
	} else if ((b_ret) && acld->vbus_vbat_thrs_err_cnt) {
		acld->vbus_vbat_thrs_err_cnt = 0;
		pr_acld(FLOW, "%s: vbus_vbat_thrs_err_cnt cnt restarted\n",
				__func__);
	}

	if (((bcmpmu_is_usb_host_enabled(acld->bcmpmu)) &&
			(!bcmpmu_is_usb_valid(acld))) ||
			(!bcmpmu_get_ubpd_int(acld))) {
		bcmpmu_acld_enable(acld, false);
		acld->acld_en = false;
		bcmpmu_restore_cc_trim_otp(acld);
		bcmpmu_set_icc_fc(acld->bcmpmu, acld->pdata->i_def_dcp);
		pr_acld(FLOW, "%s:USB Fault, start ACLD from scratch\n",
				__func__);
		bcmpmu_reset_acld_flags(acld);
	}
}
static void bcmpmu_acld_work(struct work_struct *work)
{
	struct bcmpmu_acld *acld = to_bcmpmu_acld_data(work, acld_work.work);
	int ret;

	if (acld->chrgr_type != PMU_CHRGR_TYPE_DCP)
		return;

	if (!acld->acld_min_input)
		bcmpmu_acld_get_min_input(acld);

	if (!acld->usb_mbc_fault) {
		/* If the charger driver is not yet ready, retry */
		ret = bcmpmu_chrgr_usb_en(acld->bcmpmu, 0);
		if (ret) {
			pr_acld(ERROR, "Resched ACLD, chrgr driver isn't up\n");
			if (acld->acld_rtry_cnt++ > ACLD_RETRIES)
				BUG_ON(1);
			goto q_work;
		}
		acld->acld_rtry_cnt = 0;

		pr_acld(VERBOSE, "USB/MBC Fault check\n");
		if (!bcmpmu_usb_mbc_fault_check(acld)) {
			pr_acld(ERROR, "USB/MBC charger fault occured\n");
			goto q_work;
		}
		acld->usb_mbc_fault = true;
	}
	if (!acld->volt_thrs_check) {
		pr_acld(VERBOSE, "VBUS VBAT thresholds check\n");
		if (!bcmpmu_is_avg_vbus_vbat_valid(acld)) {
			pr_acld(ERROR, "VBUS/VBAT crossed Thresholds\n");
			if (!acld->v_flag) {
				bcmpmu_chrg_on_output(acld);
				acld->v_flag = true;
			}
			goto q_work;
		}

		acld->volt_thrs_check = true;
	}


	if (!acld->acld_init || acld->acld_re_init) {
		pr_acld(VERBOSE, "Run ACLD algo\n");
		ret = bcmpmu_acld_algo(acld);
		if (ret == -EAGAIN) {
			bcmpmu_reset_acld_flags(acld);
			goto q_work;
		}
		acld->acld_init = true;
		acld->acld_re_init = false;
	}

	if (acld->mbc_in_cv) {
		if (!bcmpmu_get_mbc_cv_status(acld)) {
			pr_acld(FLOW, "MBC CV mode cleared, Re Init ACLD\n");
			acld->mbc_in_cv = false;
			acld->acld_re_init = true;
		}

		goto q_work;
	}

	/* Periodic monitor */
	if ((acld->bcmpmu->flags & BCMPMU_ACLD_EN) &&
			(acld->chrgr_type == PMU_CHRGR_TYPE_DCP) &&
			(acld->acld_en))
		bcmpmu_acld_periodic_monitor(acld);

	pr_acld(INIT, "%d %d %d %d %d %d\n",
			acld->usb_mbc_fault, acld->volt_thrs_check,
			acld->mbc_in_cv, acld->acld_init,
			acld->acld_en, acld->acld_re_init);
	pr_acld(INIT, "flag = %d\n", acld->i_bus_abv_lmt);

q_work:
	queue_delayed_work(acld->acld_wq, &acld->acld_work,
			msecs_to_jiffies(acld->acld_wrk_poll_time));
	return;
}

static int bcmpmu_reset_acld_flags(struct bcmpmu_acld *acld)
{
	acld->usb_mbc_fault = false;
	acld->volt_thrs_check = false;
	acld->acld_init = false;
	acld->acld_re_init = false;
	acld->v_flag = false;
	acld->mbc_in_cv = false;
	acld->batt_curr_err_cnt = 0;
	acld->vbus_vbat_thrs_err_cnt = 0;

	return 0;
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
		} else if ((acld->bcmpmu->flags & BCMPMU_ACLD_EN) &&
				(acld->chrgr_type == PMU_CHRGR_TYPE_DCP)) {
			bcmpmu_reset_acld_flags(acld);
			queue_delayed_work(acld->acld_wq, &acld->acld_work, 0);
		}
		break;
	case PMU_THEMAL_THROTTLE_STATUS:
		acld = to_bcmpmu_acld_data(nb, tml_trtle_nb);
		acld->tml_trtle_stat = *(bool *)data;
		pr_acld(FLOW, "PMU_THEMAL_THROTTLE_STATUS:%d\n",
				acld->tml_trtle_stat);
		if (acld->tml_trtle_stat == true) {
			pr_acld(FLOW, "cancel ACLD work\n");
			cancel_delayed_work_sync(&acld->acld_work);
		} else {
			pr_acld(FLOW, "start ACLD work\n");
			queue_delayed_work(acld->acld_wq, &acld->acld_work, 0);
		}
		break;
	}

	return 0;
}
#ifdef CONFIG_DEBUG_FS
static int debug_pmu_acld_ctrl(void *data, u64 acld_ctrl)
{
	struct bcmpmu_acld *acld = data;
	struct bcmpmu59xxx *bcmpmu = acld->bcmpmu;

	if ((bcmpmu->rev_info.prj_id == BCMPMU_59054_ID) &&
			(bcmpmu->rev_info.ana_rev >= BCMPMU_59054A1_ANA_REV)) {

		if (acld_ctrl) {
			bcmpmu->flags |= BCMPMU_ACLD_EN;
			bcmpmu_acld_enable(acld, true);
			queue_delayed_work(acld->acld_wq, &acld->acld_work, 0);
			pr_acld(INIT, "ACLD Enabled\n");
		} else {
			cancel_delayed_work_sync(&acld->acld_work);
			bcmpmu_acld_enable(acld, false);
			bcmpmu->flags &= ~BCMPMU_ACLD_EN;
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

	dentry_acld_dir = debugfs_create_dir("acld", bcmpmu->dent_bcmpmu);
	if (IS_ERR_OR_NULL(dentry_acld_dir))
		goto debugfs_clean;

	dentry_acld_file = debugfs_create_file("acld_ctrl",
			S_IWUSR | S_IRUSR, dentry_acld_dir, acld,
			&debug_pmu_acld_fops);
	if (IS_ERR_OR_NULL(dentry_acld_file))
		goto debugfs_clean;

	dentry_acld_file = debugfs_create_u8("acld_re_init",
			S_IWUSR | S_IRUSR, dentry_acld_dir,
			(u8 *)&acld->acld_re_init);
	if (IS_ERR_OR_NULL(dentry_acld_file))
		goto debugfs_clean;

	dentry_acld_file = debugfs_create_u8("qa_required",
			S_IWUSR | S_IRUSR, dentry_acld_dir,
			(u8 *)&acld->pdata->qa_required);
	if (IS_ERR_OR_NULL(dentry_acld_file))
		goto debugfs_clean;

	dentry_acld_file = debugfs_create_u32("acld_cc_lmt",
			S_IWUSR | S_IRUSR, dentry_acld_dir,
			&acld->pdata->acld_cc_lmt);
	if (IS_ERR_OR_NULL(dentry_acld_file))
		goto debugfs_clean;

	dentry_acld_file = debugfs_create_u32("acld_min_input",
			S_IWUSR | S_IRUSR, dentry_acld_dir,
			&acld->acld_min_input);
	if (IS_ERR_OR_NULL(dentry_acld_file))
		goto debugfs_clean;

	return;

debugfs_clean:
	if (!IS_ERR_OR_NULL(dentry_acld_dir))
		debugfs_remove_recursive(dentry_acld_dir);
}
#endif
static int __devexit bcmpmu_acld_remove(struct platform_device *pdev)
{
	return 0;
}

static int __devinit bcmpmu_acld_probe(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu_acld *acld;
	int ret = 0;

	pr_acld(INIT, "%s: called\n", __func__);

	if (!(bcmpmu->flags & BCMPMU_ACLD_EN)) {
		pr_acld(INIT, "ACLD is disabled\n");
		return 0;
	}

	acld = kzalloc(sizeof(struct bcmpmu_acld), GFP_KERNEL);
	if (acld == NULL) {
		pr_acld(ERROR, "%s failed to alloc mem.\n", __func__);
		return -ENOMEM;
	}
	bcmpmu->acld = (void *)acld;
	acld->bcmpmu = bcmpmu;

	acld->acld_wrk_poll_time = ACLD_WORK_POLL_5S;

	acld->pdata = (struct bcmpmu_acld_pdata *)pdev->dev.platform_data;

	if (!acld->pdata->i_max_cc)
		acld->pdata->i_max_cc = PMU_MAX_CC_CURR;
	if (!acld->pdata->i_sat)
		acld->pdata->i_sat = PMU_TYP_SAT_CURR;
	if (!acld->pdata->acld_cc_lmt)
		acld->pdata->acld_cc_lmt = ACLD_CC_LIMIT;
	if (!acld->pdata->i_def_dcp)
		acld->pdata->i_def_dcp = PMU_DCP_DEF_CURR_LMT;
	if (!acld->pdata->acld_vbus_margin)
		acld->pdata->acld_vbus_margin = ACLD_VBUS_MARGIN;
	if (!acld->pdata->acld_vbus_thrs)
		acld->pdata->acld_vbus_thrs = ACLD_VBUS_THRS;
	if (!acld->pdata->acld_vbat_thrs)
		acld->pdata->acld_vbat_thrs = ACLD_VBAT_THRS;
	if (!acld->pdata->otp_cc_trim)
		acld->pdata->otp_cc_trim = PMU_OTP_CC_TRIM;


	acld->acld_wq = create_singlethread_workqueue("bcmpmu_acld_wq");
	if (IS_ERR_OR_NULL(acld->acld_wq)) {
		ret = PTR_ERR(acld->acld_wq);
		pr_acld(ERROR, "%s Failed to create WQ\n", __func__);
		goto error;
	}

	INIT_DELAYED_WORK(&acld->acld_work, bcmpmu_acld_work);

	acld->usb_det_nb.notifier_call = bcmpmu_acld_event_handler;
	ret = bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_CHRGR_TYPE,
			&acld->usb_det_nb);
	if (ret) {
		pr_acld(ERROR, "%s Failed to add notifier:%d\n",
				__func__, __LINE__);
		goto error;
	}
	acld->tml_trtle_nb.notifier_call = bcmpmu_acld_event_handler;
	ret = bcmpmu_add_notifier(PMU_THEMAL_THROTTLE_STATUS,
			&acld->tml_trtle_nb);
	if (ret) {
		pr_acld(ERROR, "%s Failed to add notifier:%d\n",
				__func__, __LINE__);
		goto unreg_usb_det_nb;
	}
#ifdef CONFIG_DEBUG_FS
	bcmpmu_acld_debugfs_init(acld);
#endif

	/* If the event is missed */
	bcmpmu_usb_get(bcmpmu, BCMPMU_USB_CTRL_GET_CHRGR_TYPE, &ret);
	acld->chrgr_type = (enum bcmpmu_chrgr_type_t)ret;
	if (acld->chrgr_type == PMU_CHRGR_TYPE_DCP)
		queue_delayed_work(acld->acld_wq, &acld->acld_work, 0);

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
