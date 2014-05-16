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
#include <linux/wakelock.h>
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#endif
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>
#include <linux/power/bcmpmu-fg.h>

#ifdef DEBUG
#define DEBUG_MASK (BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT | \
			BCMPMU_PRINT_FLOW | BCMPMU_PRINT_VERBOSE)
#else
#define DEBUG_MASK (BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT)
#endif
static u32 debug_mask = DEBUG_MASK;
#define pr_acld(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info("[ACLD]:"args); \
		} \
	} while (0)

#define to_bcmpmu_acld_data(ptr, mem)	container_of((ptr), \
		struct bcmpmu_acld, mem)

/* Battery should not be chrged more than <rate>% 1C rate for better lifecycle
 * */
#define safe_c_rate(one_c, rate) ((one_c * rate) / 100)

#define ACLD_USE_PMU_FG_EOC_SIGNAL	0

#define BATT_ONE_C_RATE_DEF		1500
#define BATT_MAX_CHARGE_C_RATE_DEF	80
#define ACLD_WORK_POLL_5S		5000
#define ACLD_WORK_POLL_2M		120000
#define ACLD_DELAY_500			500
#define ACLD_DELAY_100			100
#define ACLD_DELAY_240			240
#define ACLD_DELAY_1000			1000
#define ACLD_DELAY_20			20
#define ACLD_RETRIES			100
#define ACLD_VBUS_MARGIN		200 /* 200mV */
#define ACLD_VBUS_THRS			5950
#define ACLD_VBAT_THRS			3500
#define USBRM_VBUS_THRS			4000
#define ACLD_CC_LIMIT			1360 /* mA */
#define ACLD_VBUS_ON_LOW_THRLD		4400
#define ACLD_ERR_CNT_THRLD_2		2
#define ACLD_ERR_CNT_THRLD_10		10
#define ACLD_MBC_CV_CLR_THRLD		4
#define ADC_VBUS_AVG_SAMPLES_8		8
#define ADC_VBUS_AVG_SAMPLES_4		4
#define ADC_READ_TRIES			10
#define ADC_RETRY_DELAY			30 /* 30ms */
#define CHRGR_RETRIES			10
#define CHRGR_EFFICIENCY		85
#define VBUS_VBAT_DELTA			1000 /* mV */
#define DIVIDE_20			20
#define TRIM_MARGIN_5			5
#define TRIM_MARGIN_0			0
#define USB_FC_CC_455MA			4

struct bcmpmu_acld {
	struct bcmpmu59xxx *bcmpmu;
	struct mutex mutex;
	struct bcmpmu_acld_pdata *pdata;
	struct workqueue_struct *acld_wq;
	struct delayed_work acld_work;
	struct notifier_block usb_det_nb;
	struct notifier_block tml_trtle_nb;
	struct notifier_block fg_eoc_nb;
	struct notifier_block chrg_curr_nb;
	struct dentry *dty_acld_dir;
	ktime_t last_sample_tm;
	enum bcmpmu_chrgr_type_t chrgr_type;
	int acld_min_input;
	int acld_rtry_cnt;
	int batt_curr_err_cnt;
	int vbus_vbat_delta;
	int vbus_vbat_thrs_err_cnt;
	int vbus_vbat_delta_deb;
	int mbc_cv_clr_cnt;
	int i_sys;
	int chrg_curr;
	int safe_c;
	int acld_wrk_poll_time;
	unsigned int msleep_ms;
	atomic_t in_acld_algo;
	u8 cc_trim;
	u8 cc_100_trim;
	u8 cc_500_trim;
	bool i_bus_abv_lmt;
	bool acld_re_init;
	bool acld_en;
	bool mbc_turbo_en;
	bool acld_init;
	bool usb_mbc_fault;
	bool volt_thrs_check;
	bool v_flag;
	bool acld_start;
	bool mbc_in_cv;
	bool tml_trtle_stat;
	bool fg_eoc;
};

static int acld_chargers[] = {
	PMU_CHRGR_TYPE_DCP,
};

static bool bcmpmu_usb_mbc_fault_check(struct bcmpmu_acld *acld);
static int bcmpmu_reset_acld_flags(struct bcmpmu_acld *acld);
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
/**
 * bcmpmu_get_avg_vbus - calculates avg VBUS voltage
 *
 * @bcmpmu - bcmpmu global pointer
 * @num_samples - number of samples out of which avg has to be taken
 *
 */
int bcmpmu_get_avg_vbus(struct bcmpmu59xxx *bcmpmu, int num_samples)
{
	int i = 0;
	int vbus_samples[num_samples];

	do {
		vbus_samples[i] = bcmpmu_get_vbus(bcmpmu);
		i++;
		msleep(20);
	} while (i < num_samples);

	return interquartile_mean(vbus_samples, num_samples);
}
/**
 * bcmpmu_acld_get_inductor_curr - Gets the MBC inductor current
 */
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
	usb_cc_trim = bcmpmu_get_trim_curr(acld->bcmpmu, TRIM_MARGIN_0);

	pr_acld(VERBOSE, "vbus: %d vbat: %d usb_fc_cc: %d usb_cc_trim: %d\n",
			vbus, vbat, usb_fc_cc, usb_cc_trim);
	i_ind = ((usb_fc_cc + usb_cc_trim) * vbus * CHRGR_EFFICIENCY) /
								(vbat * 100);

	return i_ind;
}

/**
 * bcmpmu_is_acld_enabled - This function reports if Turbo mode
 *			   is enabled or not
 */
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

/**
 * bcmpmu_acld_enable - This function enables/disables turbo
 * @acld - pointer to acld structure
 * @enable - bool to enable/disable turbo
 */
static int bcmpmu_acld_enable(struct bcmpmu_acld *acld, bool enable)
{
	int ret = 0;
	u8 reg;

	if (acld->mbc_turbo_en) {
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
	}
	return ret;
}
/**
 * bcmpmu_en_sw_ctrl_chrgr_timer: Enables/disables SW controlled chrgr timer
 * @acld - Pointer to acld structure
 * @enable - bool to enable/disable timer
 */
static void bcmpmu_en_sw_ctrl_chrgr_timer(struct bcmpmu_acld *acld, bool enable)
{
	int ret;
	u8 reg;

	if (acld->chrgr_type == PMU_CHRGR_TYPE_SDP)
		return;
	pr_acld(FLOW, "%s: enable = %d\n", __func__, enable);
	ret = acld->bcmpmu->read_dev(acld->bcmpmu, PMU_REG_MBCCTRL2, &reg);
	if (ret)
		BUG_ON(1);
	if (enable == true)
		reg |= (MBCCTRL2_SW_TMR_EN_MASK | MBCCTRL2_SW_EXP_SEL_32S_MASK);
	else
		reg &= ~MBCCTRL2_SW_TMR_EN_MASK;

	ret = acld->bcmpmu->write_dev(acld->bcmpmu, PMU_REG_MBCCTRL2, reg);
	if (ret)
		BUG_ON(1);
}
/**
 * bcmpmu_clr_sw_ctrl_chrgr_timer - Pets the SW controlled chrgr timer
 *
 */
static void bcmpmu_clr_sw_ctrl_chrgr_timer(struct bcmpmu_acld *acld)
{
	int ret;
	u8 reg;

	if (acld->chrgr_type == PMU_CHRGR_TYPE_SDP)
		return;
	ret = acld->bcmpmu->read_dev(acld->bcmpmu, PMU_REG_MBCCTRL2, &reg);
	if (ret)
		BUG_ON(1);

	reg |= MBCCTRL2_SW_TMR_CLR_MASK;

	ret = acld->bcmpmu->write_dev(acld->bcmpmu, PMU_REG_MBCCTRL2, reg);
	if (ret)
		BUG_ON(1);
}
/**
 * bcmpmu_get_mbc_cv_status - Returns true if PMU is in CV mode.
 */
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
/**
 * bcmpmu_get_fg_eoc_status - Returns true if PMU is in EOC state
 */
static bool bcmpmu_get_fg_eoc_status(struct bcmpmu_acld *acld)
{
	bool eoc_status;
#if (ACLD_USE_PMU_FG_EOC_SIGNAL == 1)
	u8 reg;
	acld->bcmpmu->read_dev(acld->bcmpmu, PMU_REG_MBCCTRL9, &reg);
	pr_acld(FLOW, "PMU_REG_MBCCTRL9: %x\n", reg);
	eoc_status = reg & MBCCTRL9_FG_EOC_MASK;
#else
	eoc_status = acld->fg_eoc;
#endif
	return eoc_status;
}
/**
 *bcmpmu_chrg_on_output - Disable ACLD(Turbo) and charge on output.
 */
static void bcmpmu_chrg_on_output(struct bcmpmu_acld *acld)
{
	int ret;
	int retries = CHRGR_RETRIES;

	bcmpmu_acld_enable(acld, false);
	acld->acld_en = false;
	bcmpmu_restore_cc_trim_otp(acld->bcmpmu);
	while (retries--) {
		ret = bcmpmu_set_chrgr_def_current(acld->bcmpmu);
		if (!ret)
			break;
		msleep(ACLD_DELAY_20);
	}
	if (retries <= 0)
		BUG_ON(1);
	bcmpmu_chrgr_usb_en(acld->bcmpmu, 1);
	pr_acld(INIT, "ACLD disabled and charging on O/P\n");
}
/**
 * bcmpmu_acld_update_min_input - Gets the minimum of
 * 1C rate, Max CC and ISat.
 */
static void bcmpmu_acld_update_min_input(struct bcmpmu_acld *acld)
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

	acld->safe_c = safe_c_rate(one_c_rate,
		acld->pdata->max_charge_c_rate_percent);

	/* C rate, Max CC and ISat are the inputs for
	 * ACLD. CC  should not excedd the minimum of
	 * above three inputs
	 * */
	acld->acld_min_input = min3(acld->safe_c, acld->pdata->i_max_cc,
			acld->pdata->i_sat);

	pr_acld(INIT, "safe_c: %d, i_max_cc: %d i_sat: %d\n",
			acld->safe_c, acld->pdata->i_max_cc,
			acld->pdata->i_sat);
	pr_acld(INIT, "acld min input = %d\n", acld->acld_min_input);
}
/**
 * bcmpmu_acld_get_batt_curr - Get the present battery current through FG.
 */
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
/**
 * bcmpmu_check_battery_current_limit - Check if battery is charging with in
 * safe limits, i.e lessthan min(<rate>%of1C, i_inductance, i_sat)
 */
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

	vbus = bcmpmu_get_avg_vbus(acld->bcmpmu,
			ADC_VBUS_AVG_SAMPLES_8);
	vbat = bcmpmu_fg_get_avg_volt(acld->bcmpmu);
	pr_acld(FLOW, "%s, vbus = %d vbat = %d\n", __func__, vbus, vbat);
	if ((vbus - vbat) > acld->vbus_vbat_delta) {
		pr_acld(ERROR, "%s: Re init ACLD\n", __func__);
		acld->acld_re_init = true;
	}

	return;
}
/**
 * bcmpmu_get_vbus_resistance - calculates VBUS resistance
 */
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
/**
 * bcmpmu_get_vbus_load - calculates to see if VBUS has dropped below
 * what is expected for atleast 200ms
 */
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
		pr_acld(FLOW, "USB_VALID is false\n");
		return false;
	}

	return true;

}
static bool bcmpmu_is_avg_vbus_vbat_valid(struct bcmpmu_acld *acld)
{
	int vbus;
	int vbat;

	vbus = bcmpmu_get_avg_vbus(acld->bcmpmu,
			ADC_VBUS_AVG_SAMPLES_8);
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
/**
 * bcmpmu_usb_mbc_fault_check - check if there are any USB/MBC errors.
 */
static bool bcmpmu_usb_mbc_fault_check(struct bcmpmu_acld *acld)
{
	bool ret;

	ret = bcmpmu_get_ubpd_int(acld);
	if (!ret)
		return false;

	ret = bcmpmu_get_usb_port_status(acld);
	if (!ret)
		return false;

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
static void acld_msleep(unsigned int sleep_ms)
{
	msleep(sleep_ms);
}
#if 0
/**
 *bcmpmu_trim_to_adapter_cc_lmt - Tunes the best trim current in
 *in Non ACLD path
 */
static void bcmpmu_trim_to_adapter_cc_lmt(struct bcmpmu_acld *acld,
		int vbus_chrg_off)
{
	int adapter_cc_lmt;
	int usb_fc_cc;
	int trim;
	int usb_fc_cc_trim0;
	int ttl_cur;
	int nxt_ttl_cur;
	int ret;

	adapter_cc_lmt = max((acld->pdata->adapter_pout_spec / vbus_chrg_off),
			acld->pdata->i_adaptor_spec_min);
	if (adapter_cc_lmt > acld->pdata->acld_cc_lmt)
		adapter_cc_lmt = acld->pdata->acld_cc_lmt;
	usb_fc_cc = bcmpmu_get_icc_fc(acld->bcmpmu);
	trim = bcmpmu_get_trim_curr(acld->bcmpmu, TRIM_MARGIN_0);
	usb_fc_cc_trim0 = ((usb_fc_cc * 100) / (100 + (trim)));

	pr_acld(FLOW, "adapter_cc_lmt = %d usb_fc_cc = %d\n",
			adapter_cc_lmt, usb_fc_cc);
	do {
		ttl_cur = usb_fc_cc_trim0 +
			bcmpmu_get_trim_curr(acld->bcmpmu, TRIM_MARGIN_5);
		nxt_ttl_cur = usb_fc_cc_trim0 +
			bcmpmu_get_next_trim_curr(acld->bcmpmu, TRIM_MARGIN_5);

		pr_acld(FLOW, "ttl_cur = %d nxt_ttl_cur = %d\n",
				ttl_cur, nxt_ttl_cur);
		if ((ttl_cur >= adapter_cc_lmt) ||
				(nxt_ttl_cur >= adapter_cc_lmt))
			break;
		pr_acld(FLOW, "Trim up by one step\n");
		ret = bcmpmu_cc_trim_up(acld->bcmpmu);
		if (ret) {
			pr_acld(INIT, "Reached Maximum Trim code\n");
			break;
		}

	} while (true);
}
#endif
/**
 * bcmpmu_acld_algo - acld_algo finds the best current
 * that the inserted charger can supply for charging.
 *
 */
static int bcmpmu_acld_algo(struct bcmpmu_acld *acld)
{
	bool fault = false;
	bool acld_cc_lmt_hit = false;
	bool reached_max_cc = false;
	bool reached_max_trim = false;
	int vbus_chrg_off;
	int vbus_chrg_on;
	int vbus_res;
	int vbus_load;
	int ret;
	int i;
	int i_sys;
	int usb_fc_cc_reached;
	int usb_fc_cc_prev;
	int usb_fc_cc_next;

	pr_acld(INIT, "%s\n", __func__);
	bcmpmu_clr_sw_ctrl_chrgr_timer(acld);

	/* Return from ACLD if,
	 * Charger is quickly removed after insertion.
	 * */
	if (!bcmpmu_get_ubpd_int(acld)) {
		fault = true;
		goto chrgr_pre_chk;
	}

	bcmpmu_chrgr_usb_en(acld->bcmpmu, 0);

	msleep(ACLD_DELAY_1000);

	bcmpmu_clr_sw_ctrl_chrgr_timer(acld);
	if (!bcmpmu_get_ubpd_int(acld)) {
		fault = true;
		goto chrgr_pre_chk;
	}

	vbus_chrg_off = bcmpmu_get_avg_vbus(acld->bcmpmu,
			ADC_VBUS_AVG_SAMPLES_8);
	pr_acld(INIT, "vbus_chrg_off = %d\n", vbus_chrg_off);
	/* get the system load */
	i_sys = bcmpmu_acld_get_batt_curr(acld);
	if (i_sys > 0)
		pr_acld(ERROR, "System load is +ve, though charging is off\n");

	bcmpmu_acld_enable(acld, true);

	/* set USB_FC_CC at OTP */
	bcmpmu_set_icc_fc(acld->bcmpmu, PMU_USB_FC_CC_OTP);
	bcmpmu_chrgr_usb_en(acld->bcmpmu, 1);

	msleep(ACLD_DELAY_500);

	if (bcmpmu_get_mbc_cv_status(acld)) {
		acld->mbc_in_cv = true;
		pr_acld(FLOW, "MBC in CV\n");
		bcmpmu_chrg_on_output(acld);
		bcmpmu_post_acld_end_event(acld);
		bcmpmu_clr_sw_ctrl_chrgr_timer(acld);
		return 0;
	}

	bcmpmu_clr_sw_ctrl_chrgr_timer(acld);
	vbus_chrg_on = bcmpmu_get_avg_vbus(acld->bcmpmu,
			ADC_VBUS_AVG_SAMPLES_8);
	vbus_res = bcmpmu_get_vbus_resistance(acld,
			vbus_chrg_off, vbus_chrg_on);
	pr_acld(INIT, "vbus_res(uohm) = %d\n", vbus_res);
	vbus_load = bcmpmu_get_vbus_load(acld, vbus_chrg_off, vbus_res);

	pr_acld(INIT, "Tuning USB_FC_CC\n");
	do {
		bcmpmu_clr_sw_ctrl_chrgr_timer(acld);
		pr_acld(INIT, "vbus_chrg_on: %d vbus_res: %d vbus_load: %d\n",
				vbus_chrg_on, vbus_res, vbus_load);
		/* Check for Charger Errors and battery CV mode */
		if (!bcmpmu_get_ubpd_int(acld)) {
			fault = true;
			goto chrgr_pre_chk;
		}
		if (bcmpmu_get_mbc_cv_status(acld)) {
			acld->mbc_in_cv = true;
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
				pr_acld(INIT, "ACLD CC lmt hit\n");
				break;
			}
			/* Increase CC  */
			pr_acld(INIT, "Icreasing CC by one step\n");
			ret = bcmpmu_icc_fc_step_up(acld->bcmpmu);
			if (ret) {
				pr_acld(INIT, "Reached Maximum CC\n");
				reached_max_cc = true;
			}

		}
		bcmpmu_clr_sw_ctrl_chrgr_timer(acld);

		for (i = 0; i < (acld->msleep_ms / DIVIDE_20); i++) {
			acld_msleep(acld->msleep_ms);
			if ((!bcmpmu_is_usb_valid(acld))) {
				pr_acld(INIT, "Exit USB_FC_CC tuning\n");
				bcmpmu_chrg_on_output(acld);
				return 0;
			}
		}
		if (reached_max_cc)
			break;
		usb_fc_cc_next = bcmpmu_get_icc_fc(acld->bcmpmu);
		if (usb_fc_cc_next <= usb_fc_cc_prev) {
			pr_acld(INIT, "PMU CC lmt hit: prev = %d next = %d\n",
					usb_fc_cc_prev, usb_fc_cc_next);
			break;
		}

		vbus_chrg_on = bcmpmu_get_avg_vbus(acld->bcmpmu,
				ADC_VBUS_AVG_SAMPLES_8);
		vbus_load = bcmpmu_get_vbus_load(acld, vbus_chrg_off,
				vbus_res);
	} while (true);

	if (!acld_cc_lmt_hit)
		bcmpmu_icc_fc_step_down(acld->bcmpmu);
#if 0
	if (acld->pdata->qa_required) {
		pr_acld(INIT, "Not true ACLD\n");
		bcmpmu_trim_to_adapter_cc_lmt(acld, vbus_chrg_off);
		goto chrgr_pre_chk;
	}
#endif

	usb_fc_cc_reached = bcmpmu_get_icc_fc(acld->bcmpmu);

	pr_acld(INIT, "Tuning USB_CC_TRIM\n");
	bcmpmu_cc_trim_up(acld->bcmpmu);
	bcmpmu_clr_sw_ctrl_chrgr_timer(acld);
	msleep(ACLD_DELAY_500);
	vbus_chrg_on = bcmpmu_get_avg_vbus(acld->bcmpmu,
			ADC_VBUS_AVG_SAMPLES_8);
	vbus_load = bcmpmu_get_vbus_load(acld, vbus_chrg_off, vbus_res);

	do {
		bcmpmu_clr_sw_ctrl_chrgr_timer(acld);
		pr_acld(INIT, "vbus_chrg_on: %d vbus_res: %d vbus_load: %d\n",
				vbus_chrg_on, vbus_res, vbus_load);

		if (!bcmpmu_get_ubpd_int(acld)) {
			fault = true;
			goto chrgr_pre_chk;
		}
		if (bcmpmu_get_mbc_cv_status(acld)) {
			acld->mbc_in_cv = true;
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
			if ((usb_fc_cc_reached + bcmpmu_get_next_trim_curr(
					acld->bcmpmu, TRIM_MARGIN_0)) >=
					acld->pdata->acld_cc_lmt) {
				pr_acld(INIT, "ACLD CC lmt hit.Exit T Tuing\n");
				break;
			}
			/* Increase Trim code */
			pr_acld(INIT, "Increasing Trim code by one step\n");
			ret = bcmpmu_cc_trim_up(acld->bcmpmu);
			if (ret) {
				pr_acld(INIT, "Reached Maximum Trim code\n");
				reached_max_trim = true;
			}
		}
		bcmpmu_clr_sw_ctrl_chrgr_timer(acld);
		for (i = 0; i < (acld->msleep_ms / DIVIDE_20); i++) {
			acld_msleep(acld->msleep_ms);
			if ((!bcmpmu_is_usb_valid(acld))) {
				pr_acld(INIT, "Exit trim tuning\n");
				bcmpmu_chrg_on_output(acld);
				return 0;
			}
		}
		if (reached_max_trim)
			break;
		vbus_chrg_on = bcmpmu_get_avg_vbus(acld->bcmpmu,
				ADC_VBUS_AVG_SAMPLES_8);
		vbus_load = bcmpmu_get_vbus_load(acld, vbus_chrg_off,
				vbus_res);
	} while (true);
	bcmpmu_clr_sw_ctrl_chrgr_timer(acld);
	bcmpmu_cc_trim_down(acld->bcmpmu);
	bcmpmu_cc_trim_down(acld->bcmpmu);
	bcmpmu_set_icc_fc(acld->bcmpmu, PMU_USB_FC_CC_OTP);
	msleep(ACLD_DELAY_500);
	bcmpmu_set_icc_fc(acld->bcmpmu, usb_fc_cc_reached);
chrgr_pre_chk:
	/* charger presence check */
	pr_acld(ERROR, "chrgr_pre_chk\n");
	if ((!bcmpmu_is_usb_valid(acld)) ||
			fault ||
			(!bcmpmu_is_avg_vbus_vbat_valid(acld)) ||
			(!bcmpmu_is_acld_enabled(acld->bcmpmu))) {
		bcmpmu_chrg_on_output(acld);
		bcmpmu_post_acld_end_event(acld);
		pr_acld(ERROR, "============ACLD Exit=============\n");
		bcmpmu_clr_sw_ctrl_chrgr_timer(acld);
		return -EAGAIN;
	}
	acld->acld_en = true;
	bcmpmu_clr_sw_ctrl_chrgr_timer(acld);
	return 0;

}
static void bcmpmu_acld_periodic_monitor(struct bcmpmu_acld *acld)
{
	bool b_ret;

	bcmpmu_clr_sw_ctrl_chrgr_timer(acld);

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
		bcmpmu_restore_cc_trim_otp(acld->bcmpmu);
		bcmpmu_set_icc_fc(acld->bcmpmu, acld->pdata->i_def_dcp);
		pr_acld(FLOW, "%s:USB Fault, start ACLD from scratch\n",
				__func__);
		bcmpmu_reset_acld_flags(acld);
	}

	bcmpmu_clr_sw_ctrl_chrgr_timer(acld);
}

bool bcmpmu_acld_false_usbrm(struct bcmpmu59xxx *bcmpmu)
{
	struct bcmpmu_acld *acld;
	int vbus_avg;
	bool false_usbrm = false;

	acld = (struct bcmpmu_acld *)bcmpmu->acld;

	if (!acld)
		return false;

	if (!atomic_read(&acld->in_acld_algo)) {
		pr_acld(FLOW, "ACLD not tuning CC\n");
		return false;
	}

	bcmpmu_usb_set(bcmpmu, BCMPMU_USB_CTRL_VBUS_ON_OFF, 1);
	bcmpmu_usb_set(bcmpmu, BCMPMU_USB_CTRL_DISCHRG_VBUS, 1);
	msleep(ACLD_DELAY_240);
	vbus_avg = bcmpmu_get_avg_vbus(bcmpmu, ADC_VBUS_AVG_SAMPLES_4);

	pr_acld(FLOW, "%s: vbus_avg = %d\n", __func__, vbus_avg);
	if ((vbus_avg >= acld->pdata->usbrm_vbus_thrs))
		false_usbrm = true;
	else
		false_usbrm = false;

	bcmpmu_usb_set(bcmpmu, BCMPMU_USB_CTRL_DISCHRG_VBUS, 0);
	bcmpmu_usb_set(bcmpmu, BCMPMU_USB_CTRL_VBUS_ON_OFF, 0);
	return false_usbrm;
}
/**
 * bcmpmu_acld_work - Periodic work to moniter ACLD algorithm.
 */
static void bcmpmu_acld_work(struct work_struct *work)
{
	struct bcmpmu_acld *acld = to_bcmpmu_acld_data(work, acld_work.work);
	int ret;
	bool mbc_cv_stat;
	bool eoc_status;

	if ((!bcmpmu_is_acld_supported(acld->bcmpmu, acld->chrgr_type)) ||
			(acld->fg_eoc))
		return;
	bcmpmu_clr_sw_ctrl_chrgr_timer(acld);

	if (acld->chrgr_type == PMU_CHRGR_TYPE_SDP) {
		bcmpmu_post_acld_end_event(acld);
		return;
	}

	if (!acld->usb_mbc_fault) {
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
#if 0
			if (!acld->v_flag) {
				bcmpmu_chrg_on_output(acld);
				acld->v_flag = true;
			}
#endif
			goto q_work;
		}

		acld->volt_thrs_check = true;
	}

	if ((!acld->acld_init || acld->acld_re_init) && !acld->fg_eoc) {
		pr_acld(VERBOSE, "Run ACLD algo\n");
		bcmpmu_post_acld_start_event(acld);
		atomic_set(&acld->in_acld_algo, 1);
		ret = bcmpmu_acld_algo(acld);
		atomic_set(&acld->in_acld_algo, 0);
		bcmpmu_post_acld_end_event(acld);
		if (ret == -EAGAIN) {
			bcmpmu_reset_acld_flags(acld);
			goto q_work;
		}
		acld->acld_init = true;
		acld->acld_re_init = false;
	}

	if (acld->mbc_in_cv) {
		mbc_cv_stat = bcmpmu_get_mbc_cv_status(acld);
		eoc_status = bcmpmu_get_fg_eoc_status(acld);
		pr_acld(FLOW, "mbc_cv_clr_cnt:%d, mbc_cv_stat:%d, eoc:%d\n",
				acld->mbc_cv_clr_cnt, mbc_cv_stat, eoc_status);
		if ((!eoc_status) &&
				(!mbc_cv_stat) &&
				(acld->mbc_cv_clr_cnt++ >
				 ACLD_MBC_CV_CLR_THRLD)) {
			pr_acld(FLOW, "CV mode cleared, ReInit ACLD\n");
			acld->mbc_in_cv = false;
			acld->acld_re_init = true;
			acld->mbc_cv_clr_cnt = 0;
		} else if ((eoc_status || (mbc_cv_stat)) &&
				(acld->mbc_cv_clr_cnt > 0)) {
			pr_acld(FLOW, "clear mbc_cv_clr_cnt\n");
			acld->mbc_cv_clr_cnt = 0;
		}
		goto q_work;
	}

	/* Periodic monitor */
	if ((acld->bcmpmu->flags & BCMPMU_ACLD_EN) &&
			(bcmpmu_is_acld_supported(acld->bcmpmu,
						  acld->chrgr_type)) &&
			(acld->acld_en))
		bcmpmu_acld_periodic_monitor(acld);

	pr_acld(INIT, "%d %d %d %d %d %d %d\n",
			acld->usb_mbc_fault, acld->volt_thrs_check,
			acld->mbc_in_cv, acld->acld_init,
			acld->acld_re_init, acld->i_bus_abv_lmt,
			acld->acld_en);

q_work:
	bcmpmu_clr_sw_ctrl_chrgr_timer(acld);
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
	acld->fg_eoc = false;
	acld->mbc_cv_clr_cnt = 0;
	acld->batt_curr_err_cnt = 0;
	acld->vbus_vbat_thrs_err_cnt = 0;
	return 0;
}

bool bcmpmu_is_acld_supported(struct bcmpmu59xxx *bcmpmu,
		enum bcmpmu_chrgr_type_t chrgr_type)
{
	struct bcmpmu_acld *acld;
	int *acld_chrgrs;
	int i;

	if (!(bcmpmu->flags & BCMPMU_ACLD_EN))
		return false;

	acld = (struct bcmpmu_acld *)bcmpmu->acld;
	acld_chrgrs = acld->pdata->acld_chrgrs;

	for (i = 0; i < acld->pdata->acld_chrgrs_list_size; i++) {
		if (acld_chrgrs[i] == chrgr_type)
			return true;
	}
	return false;
}
EXPORT_SYMBOL(bcmpmu_is_acld_supported);

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
			bcmpmu_en_sw_ctrl_chrgr_timer(acld, false);
			bcmpmu_restore_cc_trim_otp(acld->bcmpmu);
		} else if ((acld->bcmpmu->flags & BCMPMU_ACLD_EN) &&
				(bcmpmu_is_acld_supported(acld->bcmpmu,
							  acld->chrgr_type))) {
			bcmpmu_reset_acld_flags(acld);
			bcmpmu_en_sw_ctrl_chrgr_timer(acld, true);
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
			bcmpmu_acld_enable(acld, false);
			bcmpmu_en_sw_ctrl_chrgr_timer(acld, false);
		} else if ((acld->bcmpmu->flags & BCMPMU_ACLD_EN) &&
				(bcmpmu_is_acld_supported(acld->bcmpmu,
							  acld->chrgr_type))) {
			pr_acld(FLOW, "start ACLD work\n");
			bcmpmu_acld_enable(acld, true);
			bcmpmu_en_sw_ctrl_chrgr_timer(acld, true);
			queue_delayed_work(acld->acld_wq, &acld->acld_work, 0);
		}
		break;
	case PMU_FG_EVT_EOC:
		acld = to_bcmpmu_acld_data(nb, fg_eoc_nb);
		acld->fg_eoc = *(bool *)data;
		pr_acld(FLOW, "PMU_FG_EVT_EOC\n");
		if (acld->fg_eoc) {
			pr_acld(FLOW, "cancel ACLD work\n");
			cancel_delayed_work_sync(&acld->acld_work);
			bcmpmu_acld_enable(acld, false);
			bcmpmu_en_sw_ctrl_chrgr_timer(acld, false);
		} else if ((acld->bcmpmu->flags & BCMPMU_ACLD_EN) &&
				(bcmpmu_is_acld_supported(acld->bcmpmu,
							  acld->chrgr_type))) {
			pr_acld(FLOW, "start ACLD work\n");
			bcmpmu_acld_enable(acld, true);
			bcmpmu_en_sw_ctrl_chrgr_timer(acld, true);
			queue_delayed_work(acld->acld_wq, &acld->acld_work, 0);
		}
		break;
	case PMU_ACCY_EVT_OUT_CHRG_CURR:
		acld = to_bcmpmu_acld_data(nb, chrg_curr_nb);
		acld->chrg_curr  = *(int *)data;
		if ((acld->chrgr_type != PMU_CHRGR_TYPE_SDP) ||
				(acld->tml_trtle_stat) ||
				(!bcmpmu_is_acld_supported(acld->bcmpmu,
							   acld->chrgr_type)))
			break;
		pr_acld(FLOW, "PMU_ACCY_EVT_OUT_CHRG_CURR: %d\n",
				acld->chrg_curr);
		if (acld->chrg_curr == PMU_MAX_SDP_CURR) {
			bcmpmu_acld_enable(acld, true);
			acld->bcmpmu->write_dev(acld->bcmpmu,
					PMU_REG_MBCCTRL10,
					USB_FC_CC_455MA);
			pr_acld(FLOW, "MBC_TURBO enabled for SDP\n");
		} else {
			bcmpmu_acld_enable(acld, false);
			pr_acld(FLOW, "MBC_TURBO disabled for SDP\n");
		}
		break;
	}
	return 0;
}
#ifdef CONFIG_DEBUG_FS
static int debug_pmu_get_acld_ctrl(void *data, u64 *acld_ctrl)
{
	struct bcmpmu_acld *acld = data;
	struct bcmpmu59xxx *bcmpmu = acld->bcmpmu;
	u8 reg;

	acld->bcmpmu->read_dev(acld->bcmpmu, PMU_REG_OTG_BOOSTCTRL3, &reg);
	pr_acld(FLOW, "reg(0x%x) = 0x%x\n", PMU_REG_OTG_BOOSTCTRL3, reg);

	*acld_ctrl = (bcmpmu->flags & BCMPMU_ACLD_EN);

	return 0;
}
static int debug_pmu_set_acld_ctrl(void *data, u64 acld_ctrl)
{
	struct bcmpmu_acld *acld = data;
	struct bcmpmu59xxx *bcmpmu = acld->bcmpmu;

	if ((bcmpmu->rev_info.prj_id == BCMPMU_59054_ID) &&
			(bcmpmu->rev_info.ana_rev >= BCMPMU_59054A1_ANA_REV)) {

		if (acld_ctrl == 1) {
			bcmpmu->flags |= BCMPMU_ACLD_EN;
			bcmpmu_acld_enable(acld, true);
			bcmpmu_en_sw_ctrl_chrgr_timer(acld, true);
			queue_delayed_work(acld->acld_wq, &acld->acld_work, 0);
			pr_acld(INIT, "ACLD Enabled\n");
		} else if (acld_ctrl == 0) {
			cancel_delayed_work_sync(&acld->acld_work);
			bcmpmu_acld_enable(acld, false);
			bcmpmu_en_sw_ctrl_chrgr_timer(acld, false);
			bcmpmu->flags &= ~BCMPMU_ACLD_EN;
			pr_acld(INIT, "ACLD Disabled\n");
		}


	} else
		pr_acld(INIT, "ACLD not supported on this PMU Revision\n");

	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(debug_pmu_acld_fops,
		debug_pmu_get_acld_ctrl, debug_pmu_set_acld_ctrl, "%llu\n");

static int debug_bcmpmu_get_sw_ctrl_chrgr_timer(void *data, u64 *sw_tmr_ctrl)
{
	struct bcmpmu_acld *acld = data;
	u8 reg;

	acld->bcmpmu->read_dev(acld->bcmpmu, PMU_REG_MBCCTRL2, &reg);
	*sw_tmr_ctrl = (reg);
	pr_acld(FLOW, "reg(0x%x) = 0x%x\n", PMU_REG_MBCCTRL2, reg);
	return 0;
}
static int debug_bcmpmu_set_sw_ctrl_chrgr_timer(void *data, u64 sw_tmr_ctrl)
{
	struct bcmpmu_acld *acld = data;

	if (sw_tmr_ctrl == true)
		bcmpmu_en_sw_ctrl_chrgr_timer(acld, true);
	else if (sw_tmr_ctrl == false)
		bcmpmu_en_sw_ctrl_chrgr_timer(acld, false);
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(debug_sw_ctrl_chrgr_timer_fops,
		debug_bcmpmu_get_sw_ctrl_chrgr_timer,
		debug_bcmpmu_set_sw_ctrl_chrgr_timer, "%llu\n");

static int debug_bcmpmu_get_trim(void *data, u64 *trim)
{
	struct bcmpmu_acld *acld = data;
	int ret;
	ret = bcmpmu_get_cc_trim(acld->bcmpmu);
	*trim = ret;
	pr_acld(FLOW, "trim = 0x%x\n", ret);
	return 0;
}

static int debug_bcmpmu_set_trim(void *data, u64 trim_up)
{
	struct bcmpmu_acld *acld = data;

	if (trim_up == true)
		bcmpmu_cc_trim_up(acld->bcmpmu);
	else if (trim_up == false)
		bcmpmu_cc_trim_down(acld->bcmpmu);
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(debug_trim_up_fops,
		debug_bcmpmu_get_trim,
		debug_bcmpmu_set_trim, "%llu\n");

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

	acld->dty_acld_dir = dentry_acld_dir;

	dentry_acld_file = debugfs_create_file("acld_ctrl",
			S_IWUSR | S_IRUSR, dentry_acld_dir, acld,
			&debug_pmu_acld_fops);
	if (IS_ERR_OR_NULL(dentry_acld_file))
		goto debugfs_clean;

	dentry_acld_file = debugfs_create_file("sw_chrgr_tmr_ctrl",
			S_IWUSR | S_IRUSR, dentry_acld_dir, acld,
			&debug_sw_ctrl_chrgr_timer_fops);
	if (IS_ERR_OR_NULL(dentry_acld_file))
		goto debugfs_clean;

	dentry_acld_file = debugfs_create_file("trim_up",
			S_IWUSR | S_IRUSR, dentry_acld_dir, acld,
			&debug_trim_up_fops);
	if (IS_ERR_OR_NULL(dentry_acld_file))
		goto debugfs_clean;

	dentry_acld_file = debugfs_create_u32("debug_mask",
			S_IWUSR | S_IRUSR, dentry_acld_dir, &debug_mask);
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

	dentry_acld_file = debugfs_create_u8("mbc_turbo_en",
			S_IWUSR | S_IRUSR, dentry_acld_dir,
			(u8 *)&acld->mbc_turbo_en);
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

	dentry_acld_file = debugfs_create_u32("acld_vbus_thrs",
			S_IWUSR | S_IRUSR, dentry_acld_dir,
			&acld->pdata->acld_vbus_thrs);
	if (IS_ERR_OR_NULL(dentry_acld_file))
		goto debugfs_clean;

	dentry_acld_file = debugfs_create_u32("acld_vbat_thrs",
			S_IWUSR | S_IRUSR, dentry_acld_dir,
			&acld->pdata->acld_vbat_thrs);
	if (IS_ERR_OR_NULL(dentry_acld_file))
		goto debugfs_clean;

	dentry_acld_file = debugfs_create_u32("acld_vbus_vbat_delta",
			S_IWUSR | S_IRUSR, dentry_acld_dir,
			&acld->vbus_vbat_delta);
	if (IS_ERR_OR_NULL(dentry_acld_file))
		goto debugfs_clean;

	dentry_acld_file = debugfs_create_u32("acld_msleep_ms",
			S_IWUSR | S_IRUSR, dentry_acld_dir,
			&acld->msleep_ms);
	if (IS_ERR_OR_NULL(dentry_acld_file))
		goto debugfs_clean;

	return;

debugfs_clean:
	if (!IS_ERR_OR_NULL(dentry_acld_dir))
		debugfs_remove_recursive(dentry_acld_dir);
}
#endif
static int bcmpmu_acld_remove(struct platform_device *pdev)
{
	struct bcmpmu_acld *acld = platform_get_drvdata(pdev);

	bcmpmu_acld_enable(acld, false);
	bcmpmu_en_sw_ctrl_chrgr_timer(acld, false);
	destroy_workqueue(acld->acld_wq);
#ifdef CONFIG_DEBUG_FS
	debugfs_remove(acld->dty_acld_dir);
#endif
	kfree(acld);
	return 0;
}

static int bcmpmu_acld_probe(struct platform_device *pdev)
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
	platform_set_drvdata(pdev, acld);
	bcmpmu->acld = (void *)acld;
	acld->bcmpmu = bcmpmu;

	acld->acld_wrk_poll_time = ACLD_WORK_POLL_5S;
	acld->msleep_ms = ACLD_DELAY_100;
	acld->mbc_turbo_en = true;

	acld->pdata = (struct bcmpmu_acld_pdata *)pdev->dev.platform_data;

	acld->vbus_vbat_delta = VBUS_VBAT_DELTA;

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
	if (!acld->pdata->usbrm_vbus_thrs)
		acld->pdata->usbrm_vbus_thrs = USBRM_VBUS_THRS;
	if (!acld->pdata->otp_cc_trim)
		acld->pdata->otp_cc_trim = PMU_OTP_CC_TRIM;
	if (!acld->pdata->max_charge_c_rate_percent)
		acld->pdata->max_charge_c_rate_percent =
			BATT_MAX_CHARGE_C_RATE_DEF;
	if ((!acld->pdata->acld_chrgrs) ||
			(!acld->pdata->acld_chrgrs_list_size)) {
		acld->pdata->acld_chrgrs = acld_chargers;
		acld->pdata->acld_chrgrs_list_size =
			ARRAY_SIZE(acld_chargers);
	}

	bcmpmu_acld_update_min_input(acld);
	if (acld->pdata->acld_cc_lmt > acld->safe_c) {
		pr_acld(ERROR, "acld_cc_lmt: %d > safe_c: %d\n",
				acld->pdata->acld_cc_lmt, acld->safe_c);
		WARN_ON(1);
		acld->pdata->acld_cc_lmt = acld->safe_c;
	}

	acld->acld_wq = create_singlethread_workqueue("bcmpmu_acld_wq");
	if (IS_ERR_OR_NULL(acld->acld_wq)) {
		ret = PTR_ERR(acld->acld_wq);
		pr_acld(ERROR, "%s Failed to create WQ\n", __func__);
		goto exit;
	}

	INIT_DELAYED_WORK(&acld->acld_work, bcmpmu_acld_work);

	acld->usb_det_nb.notifier_call = bcmpmu_acld_event_handler;
	ret = bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_CHRGR_TYPE,
			&acld->usb_det_nb);
	if (ret) {
		pr_acld(ERROR, "%s Failed to add notifier:%d\n",
				__func__, __LINE__);
		goto unreg_nb;
	}
	acld->tml_trtle_nb.notifier_call = bcmpmu_acld_event_handler;
	ret = bcmpmu_add_notifier(PMU_THEMAL_THROTTLE_STATUS,
			&acld->tml_trtle_nb);
	if (ret) {
		pr_acld(ERROR, "%s Failed to add notifier:%d\n",
				__func__, __LINE__);
		goto unreg_nb;
	}
	acld->fg_eoc_nb.notifier_call = bcmpmu_acld_event_handler;
	ret = bcmpmu_add_notifier(PMU_FG_EVT_EOC,
			&acld->fg_eoc_nb);
	if (ret) {
		pr_acld(ERROR, "%s Failed to add notifier:%d\n",
				__func__, __LINE__);
		goto unreg_nb;
	}
	acld->chrg_curr_nb.notifier_call = bcmpmu_acld_event_handler;
	ret = bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_CHRG_CURR,
			&acld->chrg_curr_nb);
	if (ret) {
		pr_acld(ERROR, "%s Failed to add notifier:%d\n",
				__func__, __LINE__);
		goto unreg_nb;
	}
#ifdef CONFIG_DEBUG_FS
	bcmpmu_acld_debugfs_init(acld);
#endif

	/* If the event is missed */
	bcmpmu_usb_get(bcmpmu, BCMPMU_USB_CTRL_GET_CHRGR_TYPE, &ret);
	acld->chrgr_type = (enum bcmpmu_chrgr_type_t)ret;
	if (bcmpmu_is_acld_supported(acld->bcmpmu, acld->chrgr_type)) {
		pr_acld(INIT, "charger inserted, scheduling ACLD work:%s\n",
				__func__);
		bcmpmu_en_sw_ctrl_chrgr_timer(acld, true);
		queue_delayed_work(acld->acld_wq, &acld->acld_work, 0);
	}

	return 0;

unreg_nb:
	bcmpmu_remove_notifier(PMU_THEMAL_THROTTLE_STATUS,
			&acld->tml_trtle_nb);
	bcmpmu_remove_notifier(PMU_ACCY_EVT_OUT_CHRGR_TYPE,
			&acld->usb_det_nb);
	bcmpmu_remove_notifier(PMU_FG_EVT_EOC,
			&acld->fg_eoc_nb);
	bcmpmu_remove_notifier(PMU_ACCY_EVT_OUT_CHRG_CURR,
			&acld->chrg_curr_nb);
	destroy_workqueue(acld->acld_wq);
exit:
	kfree(acld);
	return 0;
}

static struct platform_driver bcmpmu_acld_driver = {
	.driver = {
		.name = "bcmpmu_acld",
	},
	.probe = bcmpmu_acld_probe,
	.remove = bcmpmu_acld_remove,
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
