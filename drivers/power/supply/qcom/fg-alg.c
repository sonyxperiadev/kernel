/* Copyright (c) 2018-2019, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt)	"ALG: %s: " fmt, __func__

#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/sort.h>
#include "fg-alg.h"

#define FULL_SOC_RAW		255
#define CAPACITY_DELTA_DECIPCT	500

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
#undef CAPACITY_DELTA_DECIPCT
#define CAPACITY_DELTA_DECIPCT 400
#endif

#define CENTI_ICORRECT_C0	105
#define CENTI_ICORRECT_C1	20

#define HOURS_TO_SECONDS	3600
#define OCV_SLOPE_UV		10869
#define MILLI_UNIT		1000
#define MICRO_UNIT		1000000
#define NANO_UNIT		1000000000

#define DEFAULT_TTF_RUN_PERIOD_MS	10000
#define DEFAULT_TTF_ITERM_DELTA_MA	200

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
#define fg_alg_somc_cl_dbg(fmt, ...) pr_info("[SOMC CL]"fmt, ##__VA_ARGS__)

#define CL_LOWER_LIMIT_PCT		98 /* 98 percent */
#define CL_ABORT_BSOC_100PCT		200 /* 2.00 percent */
#define CL_ABORT_CCSOC_100PCT		50 /* 0.50 percent */
#define CL_ABORT_KEEP_TIMEOUT_MS	(7 * 60 * 60 * 1000)
#define CL_ABORT_SLOW_TIMEOUT_MS	(10 * 60 * 60 * 1000)

#define BSOC_RANGE	GENMASK(31, 0)
#define CCSOC_RANGE	GENMASK(29, 0)
#define BSOC_100P(soc)	(int)(div64_s64((int64_t)soc * 10000, BSOC_RANGE))
#define CCSOC_100P(soc)	(int)(div64_s64((int64_t)soc * 10000, CCSOC_RANGE))
#endif

static const struct ttf_pt ttf_ln_table[] = {
	{ 1000,		0 },
	{ 2000,		693 },
	{ 4000,		1386 },
	{ 6000,		1792 },
	{ 8000,		2079 },
	{ 16000,	2773 },
	{ 32000,	3466 },
	{ 64000,	4159 },
	{ 128000,	4852 },
};

/* Cycle counter APIs */

/**
 * restore_cycle_count -
 * @counter: Cycle counter object
 *
 * Restores all the counters back from FG/QG during boot
 *
 */
int restore_cycle_count(struct cycle_counter *counter)
{
	int rc = 0;

	if (!counter)
		return -ENODEV;

	mutex_lock(&counter->lock);
	rc = counter->restore_count(counter->data, counter->count,
			BUCKET_COUNT);
	if (rc < 0)
		pr_err("failed to restore cycle counter rc=%d\n", rc);
	mutex_unlock(&counter->lock);

	return rc;
}

/**
 * clear_cycle_count -
 * @counter: Cycle counter object
 *
 * Clears all the counters stored by FG/QG when a battery is inserted
 * or the profile is re-loaded.
 *
 */
void clear_cycle_count(struct cycle_counter *counter)
{
	int rc = 0, i;

	if (!counter)
		return;

	mutex_lock(&counter->lock);
	memset(counter->count, 0, sizeof(counter->count));
	for (i = 0; i < BUCKET_COUNT; i++) {
		counter->started[i] = false;
		counter->last_soc[i] = 0;
	}

	rc = counter->store_count(counter->data, counter->count, 0,
			BUCKET_COUNT * 2);
	if (rc < 0)
		pr_err("failed to clear cycle counter rc=%d\n", rc);

	mutex_unlock(&counter->lock);
}

/**
 * store_cycle_count -
 * @counter: Cycle counter object
 * @id: Cycle counter bucket id
 *
 * Stores the cycle counter for a bucket in FG/QG.
 *
 */
static int store_cycle_count(struct cycle_counter *counter, int id)
{
	int rc = 0;
	u16 cyc_count;

	if (!counter)
		return -ENODEV;

	if (id < 0 || (id > BUCKET_COUNT - 1)) {
		pr_err("Invalid id %d\n", id);
		return -EINVAL;
	}

	cyc_count = counter->count[id];
	cyc_count++;

	rc = counter->store_count(counter->data, &cyc_count, id, 2);
	if (rc < 0) {
		pr_err("failed to write cycle_count[%d] rc=%d\n",
			id, rc);
		return rc;
	}

	counter->count[id] = cyc_count;
	pr_debug("Stored count %d in id %d\n", cyc_count, id);

	return rc;
}

/**
 * cycle_count_update -
 * @counter: Cycle counter object
 * @batt_soc: Battery State of Charge (SOC)
 * @charge_status: Charging status from power supply
 * @charge_done: Indicator for charge termination
 * @input_present: Indicator for input presence
 *
 * Called by FG/QG whenever there is a state change (Charging status, SOC)
 *
 */
void cycle_count_update(struct cycle_counter *counter, int batt_soc,
			int charge_status, bool charge_done, bool input_present)
{
	int rc = 0, id, i, soc_thresh;

	if (!counter)
		return;

	mutex_lock(&counter->lock);

	/* Find out which id the SOC falls in */
	id = batt_soc / BUCKET_SOC_PCT;

	if (charge_status == POWER_SUPPLY_STATUS_CHARGING) {
		if (!counter->started[id] && id != counter->last_bucket) {
			counter->started[id] = true;
			counter->last_soc[id] = batt_soc;
		}
	} else if (charge_done || !input_present) {
		for (i = 0; i < BUCKET_COUNT; i++) {
			soc_thresh = counter->last_soc[i] + BUCKET_SOC_PCT / 2;
			if (counter->started[i] && batt_soc > soc_thresh) {
				rc = store_cycle_count(counter, i);
				if (rc < 0)
					pr_err("Error in storing cycle_ctr rc: %d\n",
						rc);
				counter->last_soc[i] = 0;
				counter->started[i] = false;
				counter->last_bucket = i;
			}
		}
	}

	pr_debug("batt_soc: %d id: %d chg_status: %d\n", batt_soc, id,
		charge_status);
	mutex_unlock(&counter->lock);
}

/**
 * get_bucket_cycle_count -
 * @counter: Cycle counter object
 *
 * Returns the cycle counter for a SOC bucket.
 *
 */
static int get_bucket_cycle_count(struct cycle_counter *counter)
{
	int count;

	if (!counter)
		return 0;

	if ((counter->id <= 0) || (counter->id > BUCKET_COUNT))
		return -EINVAL;

	mutex_lock(&counter->lock);
	count = counter->count[counter->id - 1];
	mutex_unlock(&counter->lock);
	return count;
}

/**
 * get_cycle_count -
 * @counter: Cycle counter object
 * @count: Average cycle count returned to the caller
 *
 * Get average cycle count for all buckets
 *
 */
int get_cycle_count(struct cycle_counter *counter, int *count)
{
	int i, rc, temp = 0;

	for (i = 1; i <= BUCKET_COUNT; i++) {
		counter->id = i;
		rc = get_bucket_cycle_count(counter);
		if (rc < 0) {
			pr_err("Couldn't get cycle count rc=%d\n", rc);
			return rc;
		}
		temp += rc;
	}

	/*
	 * Normalize the counter across each bucket so that we can get
	 * the overall charge cycle count.
	 */

	*count = temp / BUCKET_COUNT;
	return 0;
}

/**
 * get_cycle_counts -
 * @counter: Cycle counter object
 * @buf: Bucket cycle counts formatted in a string returned to the caller
 *
 * Get cycle count for all buckets in a string format
 *
 */
int get_cycle_counts(struct cycle_counter *counter, const char **buf)
{
	int i, rc, len = 0;

	for (i = 1; i <= BUCKET_COUNT; i++) {
		counter->id = i;
		rc = get_bucket_cycle_count(counter);
		if (rc < 0) {
			pr_err("Couldn't get cycle count rc=%d\n", rc);
			return rc;
		}

		if (sizeof(counter->str_buf) - len < 8) {
			pr_err("Invalid length %d\n", len);
			return -EINVAL;
		}

		len += snprintf(counter->str_buf + len, 8, "%d ", rc);
	}

	counter->str_buf[len] = '\0';
	*buf = counter->str_buf;
	return 0;
}

/**
 * cycle_count_init -
 * @counter: Cycle counter object
 *
 * FG/QG have to call this during driver probe to validate the required
 * parameters after allocating cycle_counter object.
 *
 */
int cycle_count_init(struct cycle_counter *counter)
{
	if (!counter)
		return -ENODEV;

	if (!counter->data || !counter->restore_count ||
		!counter->store_count) {
		pr_err("Invalid parameters for using cycle counter\n");
		return -EINVAL;
	}

	mutex_init(&counter->lock);
	counter->last_bucket = -1;
	return 0;
}

/* Capacity learning algorithm APIs */

/**
 * cap_learning_post_process -
 * @cl: Capacity learning object
 *
 * Does post processing on the learnt capacity based on the user specified
 * or default parameters for the capacity learning algorithm.
 *
 */
static void cap_learning_post_process(struct cap_learning *cl)
{
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	int rc;
	int64_t lower_cap_uah, store_cap_uah;

	if (!cl->store_learned_capacity)
		return;

	store_cap_uah = cl->final_cap_uah;
	if (cl->final_cap_uah > cl->nom_cap_uah) {
		fg_alg_somc_cl_dbg("learning capacity %lld goes above nom_cap %lld\n",
					cl->final_cap_uah, cl->nom_cap_uah);
		store_cap_uah = cl->nom_cap_uah;
	}

	lower_cap_uah = div64_u64(cl->learned_cap_uah * CL_LOWER_LIMIT_PCT,
									100);
	if (cl->final_cap_uah < lower_cap_uah) {
		fg_alg_somc_cl_dbg("learning capacity %lld goes below lower limited cap %lld\n",
					cl->final_cap_uah, lower_cap_uah);
		store_cap_uah = lower_cap_uah;
	}

	if (cl->learned_cap_uah != store_cap_uah) {
		fg_alg_somc_cl_dbg("store learning capacity %lld\n",
								store_cap_uah);
		rc = cl->store_learned_capacity(cl->data, store_cap_uah);
		if (rc < 0) {
			pr_err("Error in storing learned_cap_uah, rc=%d\n", rc);
			return;
		}
	}
	cl->learned_cap_uah = store_cap_uah;
	cl->learning_counter++;
#else
	int64_t max_inc_val, min_dec_val, old_cap;
	int rc;

	if (cl->dt.skew_decipct) {
		pr_debug("applying skew %d on current learnt capacity %lld\n",
			cl->dt.skew_decipct, cl->final_cap_uah);
		cl->final_cap_uah = cl->final_cap_uah *
					(1000 + cl->dt.skew_decipct);
		cl->final_cap_uah = div64_u64(cl->final_cap_uah, 1000);
	}

	max_inc_val = cl->learned_cap_uah * (1000 + cl->dt.max_cap_inc);
	max_inc_val = div64_u64(max_inc_val, 1000);

	min_dec_val = cl->learned_cap_uah * (1000 - cl->dt.max_cap_dec);
	min_dec_val = div64_u64(min_dec_val, 1000);

	old_cap = cl->learned_cap_uah;
	if (cl->final_cap_uah > max_inc_val)
		cl->learned_cap_uah = max_inc_val;
	else if (cl->final_cap_uah < min_dec_val)
		cl->learned_cap_uah = min_dec_val;
	else
		cl->learned_cap_uah = cl->final_cap_uah;

	if (cl->dt.max_cap_limit) {
		max_inc_val = (int64_t)cl->nom_cap_uah * (1000 +
				cl->dt.max_cap_limit);
		max_inc_val = div64_u64(max_inc_val, 1000);
		if (cl->final_cap_uah > max_inc_val) {
			pr_debug("learning capacity %lld goes above max limit %lld\n",
				cl->final_cap_uah, max_inc_val);
			cl->learned_cap_uah = max_inc_val;
		}
	}

	if (cl->dt.min_cap_limit) {
		min_dec_val = (int64_t)cl->nom_cap_uah * (1000 -
				cl->dt.min_cap_limit);
		min_dec_val = div64_u64(min_dec_val, 1000);
		if (cl->final_cap_uah < min_dec_val) {
			pr_debug("learning capacity %lld goes below min limit %lld\n",
				cl->final_cap_uah, min_dec_val);
			cl->learned_cap_uah = min_dec_val;
		}
	}

	if (cl->store_learned_capacity) {
		rc = cl->store_learned_capacity(cl->data, cl->learned_cap_uah);
		if (rc < 0)
			pr_err("Error in storing learned_cap_uah, rc=%d\n", rc);
	}

	pr_debug("final cap_uah = %lld, learned capacity %lld -> %lld uah\n",
		cl->final_cap_uah, old_cap, cl->learned_cap_uah);
#endif
}

/**
 * cap_wt_learning_process_full_data -
 * @cl: Capacity learning object
 * @delta_batt_soc_pct: percentage change in battery State of Charge
 * @batt_soc_msb: MSB of battery State of Charge
 *
 * Calculates the final learnt capacity when
 * weighted capacity learning is enabled.
 *
 */
#ifndef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
static int cap_wt_learning_process_full_data(struct cap_learning *cl,
					int delta_batt_soc_pct,
					int batt_soc_msb)
{
	int64_t del_cap_uah, total_cap_uah,
		res_cap_uah, wt_learnt_cap_uah;
	int delta_batt_soc_msb, res_batt_soc_msb;

	/* If the delta is < 10%, then skip processing full data */
	if (delta_batt_soc_pct < cl->dt.min_delta_batt_soc) {
		pr_debug("batt_soc_delta_pct: %d\n", delta_batt_soc_pct);
		return -ERANGE;
	}

	delta_batt_soc_msb = batt_soc_msb - cl->init_batt_soc_msb;
	res_batt_soc_msb = FULL_SOC_RAW - batt_soc_msb;
	/* Learnt Capacity from end Battery SOC MSB to FULL_SOC_RAW */
	res_cap_uah = div64_s64(cl->learned_cap_uah *
				res_batt_soc_msb, FULL_SOC_RAW);
	total_cap_uah = cl->init_cap_uah + cl->delta_cap_uah + res_cap_uah;
	/*
	 * difference in capacity learnt in this
	 * charge cycle and previous learnt capacity
	 */
	del_cap_uah = total_cap_uah - cl->learned_cap_uah;
	/* Applying weight based on change in battery SOC MSB */
	wt_learnt_cap_uah = div64_s64(del_cap_uah * delta_batt_soc_msb,
					FULL_SOC_RAW);
	cl->final_cap_uah = cl->learned_cap_uah + wt_learnt_cap_uah;

	pr_debug("wt_learnt_cap_uah=%lld, del_cap_uah=%lld\n",
			wt_learnt_cap_uah, del_cap_uah);
	pr_debug("init_cap_uah=%lld, total_cap_uah=%lld, res_cap_uah=%lld, delta_cap_uah=%lld\n",
			cl->init_cap_uah, cl->final_cap_uah,
			res_cap_uah, cl->delta_cap_uah);
	return 0;
}
#endif

/**
 * cap_learning_process_full_data -
 * @cl: Capacity learning object
 * @batt_soc_msb: Most significant byte of Battery State of Charge
 *
 * Processes the coulomb counter during charge termination and calculates the
 * delta w.r.to the coulomb counter obtained earlier when the learning begun.
 *
 */
static int cap_learning_process_full_data(struct cap_learning *cl,
					int batt_soc_msb)
{
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	int rc, cc_soc_sw, cc_delta;
	int64_t cc_delta_100pct, cc_delta_uah;

	rc = cl->get_cc_soc(cl->data, &cc_soc_sw);
	if (rc < 0) {
		pr_err("Error in getting CC_SOC_SW, rc=%d\n", rc);
		return rc;
	}

	cc_delta = cc_soc_sw - cl->init_cc_soc_sw;
	cc_delta_100pct = CCSOC_100P(cc_delta);
	cc_delta_uah = div64_s64(cl->learned_cap_uah * cc_delta_100pct, 10000);
	cl->final_cap_uah = cl->init_cap_uah + cc_delta_uah;
	fg_alg_somc_cl_dbg("cc_delta_100pct:%lld cc_delta_uah:%lld "
			   "init_cap_uah:%d final_cap_uah:%d\n",
					cc_delta_100pct, cc_delta_uah,
					cl->init_cap_uah, cl->final_cap_uah);
	return 0;
#else
	int rc, cc_soc_sw, cc_soc_delta_pct, delta_batt_soc_pct, batt_soc_pct,
		cc_soc_fraction;
	int64_t cc_soc_cap_uah, cc_soc_fraction_uah;

	rc = cl->get_cc_soc(cl->data, &cc_soc_sw);
	if (rc < 0) {
		pr_err("Error in getting CC_SOC_SW, rc=%d\n", rc);
		return rc;
	}

	batt_soc_pct = DIV_ROUND_CLOSEST(batt_soc_msb * 100, FULL_SOC_RAW);
	delta_batt_soc_pct = batt_soc_pct - cl->init_batt_soc;
	cc_soc_delta_pct =
		div_s64_rem((int64_t)(cc_soc_sw - cl->init_cc_soc_sw) * 100,
				cl->cc_soc_max, &cc_soc_fraction);
	cc_soc_fraction_uah = div64_s64(cl->learned_cap_uah *
				cc_soc_fraction, (int64_t)cl->cc_soc_max * 100);
	cc_soc_cap_uah = div64_s64(cl->learned_cap_uah * cc_soc_delta_pct, 100);
	cl->delta_cap_uah = cc_soc_cap_uah + cc_soc_fraction_uah;
	pr_debug("cc_soc_delta_pct=%d, cc_soc_cap_uah=%lld, cc_soc_fraction_uah=%lld\n",
			cc_soc_delta_pct, cc_soc_cap_uah, cc_soc_fraction_uah);

	if (cl->dt.cl_wt_enable) {
		rc = cap_wt_learning_process_full_data(cl, delta_batt_soc_pct,
							batt_soc_msb);
		return rc;
	}

	/* If the delta is < 50%, then skip processing full data */
	if (cc_soc_delta_pct < 50) {
		pr_err("cc_soc_delta_pct: %d\n", cc_soc_delta_pct);
		return -ERANGE;
	}

	cl->final_cap_uah = cl->init_cap_uah + cl->delta_cap_uah;
	pr_debug("Current cc_soc=%d cc_soc_delta_pct=%d total_cap_uah=%lld\n",
		cc_soc_sw, cc_soc_delta_pct, cl->final_cap_uah);
	return 0;
#endif
}

/**
 * cap_learning_begin -
 * @cl: Capacity learning object
 * @batt_soc: Battery State of Charge (SOC)
 *
 * Gets the coulomb counter from FG/QG when the conditions are suitable for
 * beginning capacity learning. Also, primes the coulomb counter based on
 * battery SOC if required.
 *
 */
static int cap_learning_begin(struct cap_learning *cl, u32 batt_soc)
{
	int rc, cc_soc_sw, batt_soc_msb, batt_soc_pct;

	batt_soc_msb = batt_soc >> 24;
	batt_soc_pct = DIV_ROUND_CLOSEST(batt_soc_msb * 100, FULL_SOC_RAW);

	if (!cl->dt.cl_wt_enable) {
		if (batt_soc_pct > cl->dt.max_start_soc ||
				batt_soc_pct < cl->dt.min_start_soc) {
			pr_debug("Battery SOC %d is high/low, not starting\n",
					batt_soc_pct);
			return -EINVAL;
		}
	}

	cl->init_cap_uah = div64_s64(cl->learned_cap_uah * batt_soc_msb,
					FULL_SOC_RAW);

	if (cl->prime_cc_soc) {
		/*
		 * Prime cc_soc_sw with battery SOC when capacity learning
		 * begins.
		 */
		rc = cl->prime_cc_soc(cl->data, batt_soc);
		if (rc < 0) {
			pr_err("Error in writing cc_soc_sw, rc=%d\n", rc);
			goto out;
		}
	}

	rc = cl->get_cc_soc(cl->data, &cc_soc_sw);
	if (rc < 0) {
		pr_err("Error in getting CC_SOC_SW, rc=%d\n", rc);
		goto out;
	}

	cl->init_cc_soc_sw = cc_soc_sw;
	cl->init_batt_soc = batt_soc_pct;
	cl->init_batt_soc_msb = batt_soc_msb;
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	cl->learning_trial_counter++;
#endif
	pr_debug("Capacity learning started @ battery SOC %d init_cc_soc_sw:%d\n",
		batt_soc_msb, cl->init_cc_soc_sw);
out:
	return rc;
}

/**
 * cap_learning_done -
 * @cl: Capacity learning object
 * @batt_soc_msb: Most significant byte of battery State of Charge
 *
 * Top level function for getting coulomb counter and post processing the
 * data once the capacity learning is complete after charge termination.
 *
 */
static int cap_learning_done(struct cap_learning *cl, int batt_soc_msb)
{
	int rc;

	rc = cap_learning_process_full_data(cl, batt_soc_msb);
	if (rc < 0) {
		pr_debug("Error in processing cap learning full data, rc=%d\n",
			rc);
		goto out;
	}

	if (cl->prime_cc_soc) {
		/* Write a FULL value to cc_soc_sw */
		rc = cl->prime_cc_soc(cl->data, cl->cc_soc_max);
		if (rc < 0) {
			pr_err("Error in writing cc_soc_sw, rc=%d\n", rc);
			goto out;
		}
	}

	cap_learning_post_process(cl);
out:
	return rc;
}

/**
 * cap_wt_learning_update -
 * @cl: Capacity learning object
 * @batt_soc_msb: Most significant byte of battery State of Charge
 * @input_present: Indicator for input presence
 *
 * Called by cap_learning_update when weighted learning is enabled
 *
 */
#ifndef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
static void cap_wt_learning_update(struct cap_learning *cl, int batt_soc_msb,
					bool input_present)
{
	int rc;

	if (!input_present) {
		rc = cap_learning_done(cl, batt_soc_msb);
		if (rc < 0)
			pr_debug("Error in completing capacity learning, rc=%d\n",
				rc);
		cl->active = false;
		cl->init_cap_uah = 0;
	}
}
#endif

/**
 * cap_learning_update -
 * @cl: Capacity learning object
 * @batt_temp - Battery temperature
 * @batt_soc: Battery State of Charge (SOC)
 * @charge_status: Charging status from power supply
 * @charge_done: Indicator for charge termination
 * @input_present: Indicator for input presence
 * @qnovo_en: Indicator for Qnovo enable status
 *
 * Called by FG/QG driver when there is a state change (Charging status, SOC)
 *
 */
void cap_learning_update(struct cap_learning *cl, int batt_temp,
			int batt_soc, int charge_status, bool charge_done,
			bool input_present, bool qnovo_en)
{
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	int rc, batt_soc_prime;
	int msoc, cc_soc_sw;
	int bsoc_100p, ccsoc_100p;
	int bsoc_drop_100p, ccsoc_drop_100p, max_bsoc_100p, max_ccsoc_100p;
	bool prime_cc = false;
	bool deactive = false;
	ktime_t ktime;
#else
	int rc, batt_soc_msb, batt_soc_prime;
	bool prime_cc = false;
#endif

	if (!cl)
		return;

	mutex_lock(&cl->lock);

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	if (cl->get_monotonic_soc) {
		rc = cl->get_monotonic_soc(cl->data, &msoc);
		if (rc < 0) {
			pr_err("Error in getting msoc, rc=%d\n", rc);
			goto out;
		}
	} else {
		goto out;
	}

	if (cl->get_cc_soc) {
		rc = cl->get_cc_soc(cl->data, &cc_soc_sw);
		if (rc < 0) {
			pr_err("Error in getting CC_SOC_SW, rc=%d\n", rc);
			goto out;
		}
	} else {
		goto out;
	}
 
	ktime = ktime_get_boottime();

	if (!cl->learned_cap_uah) {
		pr_err("Error in getting learned_cap_uah, rc=%d\n");
		goto out;
	}

	bsoc_100p = BSOC_100P((u32)batt_soc);
	ccsoc_100p = CCSOC_100P(cc_soc_sw);
	fg_alg_somc_cl_dbg("[%s] charge_status:%d bsoc:%d\n",
					cl->active ? "LERANING" : "OFF",
					charge_status, bsoc_100p);

	if (!cl->active) {
		if (batt_temp > cl->dt.max_temp ||
			batt_temp < cl->dt.min_temp) {
			fg_alg_somc_cl_dbg("CL couldn't start because batt_temp is out of range %d\n",
								batt_temp);
			goto out;
		}

		if (charge_status == POWER_SUPPLY_STATUS_CHARGING) {
			rc = cap_learning_begin(cl, batt_soc);
			if (rc == 0) {
				cl->active = true;
				cl->max_ccsoc_during_active = cc_soc_sw;
				cl->max_bsoc_during_active = batt_soc;
				cl->max_bsoc_time_ms = ktime_to_ms(ktime);
				cl->start_time_ms = cl->max_bsoc_time_ms;
				fg_alg_somc_cl_dbg("CL started. bsoc:%d init_cc_soc_sw:%d time:%lld\n",
						bsoc_100p,
						CCSOC_100P(cl->init_cc_soc_sw),
						cl->start_time_ms);
			} else {
				fg_alg_somc_cl_dbg("CL couldn't start. bsoc:%d\n",
								bsoc_100p);
			}
		} else {
			if (charge_status == POWER_SUPPLY_STATUS_DISCHARGING ||
								charge_done)
				prime_cc = true;
		}
	} else {
		if (batt_temp > cl->dt.max_temp ||
			batt_temp < cl->dt.min_temp) {
			fg_alg_somc_cl_dbg("CL aborted because batt_temp is out of range %d\n",
								batt_temp);
			goto cl_deactive;
		}

		if (charge_done) {
			if (msoc < 100) {
				fg_alg_somc_cl_dbg("CL aborted due to not 100pct. msoc:%d\n",
									msoc);
			} else {
				fg_alg_somc_cl_dbg("CL done\n");
				rc = cap_learning_done(cl,(u32)batt_soc >> 24);
				if (rc < 0)
					pr_err("Error in completing capacity learning, rc=%d\n",
						rc);
			}
			deactive = true;
			goto cl_deactive;
		}

		cl->batt_soc_drop = cl->max_bsoc_during_active - batt_soc;
		cl->cc_soc_drop = cl->max_ccsoc_during_active - cc_soc_sw;
		bsoc_drop_100p = BSOC_100P(cl->batt_soc_drop);
		ccsoc_drop_100p = CCSOC_100P(cl->cc_soc_drop);
		max_bsoc_100p = BSOC_100P((u32)cl->max_bsoc_during_active);
		max_ccsoc_100p = CCSOC_100P(cl->max_ccsoc_during_active);
		cl->hold_time = ktime_to_ms(ktime) - cl->max_bsoc_time_ms;
		cl->total_time = ktime_to_ms(ktime) - cl->start_time_ms;

		fg_alg_somc_cl_dbg("msoc:%d bsoc_drop:%d cc_soc_drop:%d hold_time:%lld total_time:%lld\n",
					msoc, bsoc_drop_100p, ccsoc_drop_100p,
					cl->hold_time, cl->total_time);

		if (ccsoc_drop_100p > CL_ABORT_CCSOC_100PCT) {
			fg_alg_somc_cl_dbg("CL aborted due to cc_soc_sw drop from %d to %d",
						max_ccsoc_100p, ccsoc_100p);
			deactive = true;
			goto cl_deactive;
		} else if (bsoc_drop_100p > CL_ABORT_BSOC_100PCT) {
			fg_alg_somc_cl_dbg("CL aborted due to bsoc drop from %d to %d",
						max_bsoc_100p, bsoc_100p);
			deactive = true;
			goto cl_deactive;
		} else if (cl->hold_time > CL_ABORT_KEEP_TIMEOUT_MS) {
			fg_alg_somc_cl_dbg("CL aborted due to soc holding for long hours");
			deactive = true;
			goto cl_deactive;
		} else if (cl->total_time > CL_ABORT_SLOW_TIMEOUT_MS) {
			fg_alg_somc_cl_dbg("CL aborted due to slow charging");
			deactive = true;
			goto cl_deactive;
		}

		/* reset params if increasing */
		if (cl->batt_soc_drop < 0 || cl->cc_soc_drop < 0) {
			cl->max_ccsoc_during_active = cc_soc_sw;
			cl->max_bsoc_during_active = batt_soc;
			cl->max_bsoc_time_ms = ktime_to_ms(ktime);
			cl->cc_soc_drop = 0;
			cl->batt_soc_drop = 0;
			fg_alg_somc_cl_dbg("max bsoc/ccsoc updated. bsoc:%d ccsoc:%d\n",
							bsoc_100p, ccsoc_100p);
		}
		goto out;
	}
cl_deactive:
	if (deactive) {
		cl->active = false;
		cl->init_cap_uah = 0;
		prime_cc = true;
	}
#else
	if (batt_temp > cl->dt.max_temp || batt_temp < cl->dt.min_temp ||
		!cl->learned_cap_uah) {
		cl->active = false;
		cl->init_cap_uah = 0;
		goto out;
	}

	batt_soc_msb = (u32)batt_soc >> 24;
	pr_debug("Charge_status: %d active: %d batt_soc: %d\n",
		charge_status, cl->active, batt_soc_msb);

	if (cl->active && cl->dt.cl_wt_enable)
		cap_wt_learning_update(cl, batt_soc_msb,
					input_present);

	/* Initialize the starting point of learning capacity */
	if (!cl->active) {
		if (charge_status == POWER_SUPPLY_STATUS_CHARGING) {
			rc = cap_learning_begin(cl, batt_soc);
			cl->active = (rc == 0);
		} else {
			if (charge_status == POWER_SUPPLY_STATUS_DISCHARGING ||
				charge_done)
				prime_cc = true;
		}
	} else {
		if (charge_done) {
			rc = cap_learning_done(cl, batt_soc_msb);
			if (rc < 0)
				pr_err("Error in completing capacity learning, rc=%d\n",
					rc);

			cl->active = false;
			cl->init_cap_uah = 0;
		}

		if (charge_status == POWER_SUPPLY_STATUS_DISCHARGING &&
				!input_present) {
			pr_debug("Capacity learning aborted @ battery SOC %d\n",
				 batt_soc_msb);
			cl->active = false;
			cl->init_cap_uah = 0;
			prime_cc = true;
		}

		if (charge_status == POWER_SUPPLY_STATUS_NOT_CHARGING &&
				!cl->dt.cl_wt_enable) {
			if (qnovo_en && input_present) {
				/*
				 * Don't abort the capacity learning when qnovo
				 * is enabled and input is present where the
				 * charging status can go to "not charging"
				 * intermittently.
				 */
			} else {
				pr_debug("Capacity learning aborted @ battery SOC %d\n",
					batt_soc_msb);
				cl->active = false;
				cl->init_cap_uah = 0;
				prime_cc = true;
			}
		}
	}
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */

	/*
	 * Prime CC_SOC_SW when the device is not charging or during charge
	 * termination when the capacity learning is not active.
	 */

	if (prime_cc && cl->prime_cc_soc) {
		if (charge_done)
			batt_soc_prime = cl->cc_soc_max;
		else
			batt_soc_prime = batt_soc;

		rc = cl->prime_cc_soc(cl->data, batt_soc_prime);
		if (rc < 0)
			pr_err("Error in writing cc_soc_sw, rc=%d\n",
				rc);
	}

out:
	mutex_unlock(&cl->lock);
}

/**
 * cap_learning_abort -
 * @cl: Capacity learning object
 *
 * Aborts the capacity learning and initializes variables
 *
 */
void cap_learning_abort(struct cap_learning *cl)
{
	if (!cl)
		return;

	mutex_lock(&cl->lock);
	pr_debug("Aborting cap_learning\n");
	cl->active = false;
	cl->init_cap_uah = 0;
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	mutex_unlock(&cl->lock);
#else
	mutex_lock(&cl->lock);
#endif
}

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
void cap_learning_somc_limit_learned_cap(struct cap_learning *cl)
{
	int64_t pct_nom_cap_uah;

	if (cl->learned_cap_uah > cl->nom_cap_uah) {
		fg_alg_somc_cl_dbg("learned_cap_uah: %lld is higher than "
				   "expected, capping it to nom_cap_uah: "
				   "%lld\n",
				   cl->learned_cap_uah, cl->nom_cap_uah);
		cl->learned_cap_uah = cl->nom_cap_uah;
	} else {
		pct_nom_cap_uah =
			div64_s64((int64_t)cl->nom_cap_uah *
			CAPACITY_DELTA_DECIPCT, 1000);
		if (cl->learned_cap_uah < pct_nom_cap_uah) {
			fg_alg_somc_cl_dbg("learned_cap_uah: %lld is lower "
					   "than expected, capping it to %d%% "
					   "of nom_cap_uah: %lld\n",
						cl->learned_cap_uah,
						CAPACITY_DELTA_DECIPCT / 10,
						pct_nom_cap_uah);
			cl->learned_cap_uah = pct_nom_cap_uah;
		}
	}
}
#endif

/**
 * cap_learning_post_profile_init -
 * @cl: Capacity learning object
 * @nom_cap_uah: Nominal capacity of battery in uAh
 *
 * Called by FG/QG once the profile load is complete and nominal capacity
 * of battery is known. This also gets the last learned capacity back from
 * FG/QG to feed back to the algorithm.
 *
 */
int cap_learning_post_profile_init(struct cap_learning *cl, int64_t nom_cap_uah)
{
#ifndef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	int64_t delta_cap_uah, pct_nom_cap_uah;
#endif
	int rc;

	if (!cl || !cl->data)
		return -EINVAL;

	mutex_lock(&cl->lock);
	cl->nom_cap_uah = nom_cap_uah;
	rc = cl->get_learned_capacity(cl->data, &cl->learned_cap_uah);
	if (rc < 0) {
		pr_err("Couldn't get learned capacity, rc=%d\n", rc);
		goto out;
	}

	if (cl->learned_cap_uah != cl->nom_cap_uah) {
		if (cl->learned_cap_uah == 0)
			cl->learned_cap_uah = cl->nom_cap_uah;

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
		cap_learning_somc_limit_learned_cap(cl);
#else
		delta_cap_uah = abs(cl->learned_cap_uah - cl->nom_cap_uah);
		pct_nom_cap_uah = div64_s64((int64_t)cl->nom_cap_uah *
				CAPACITY_DELTA_DECIPCT, 1000);
		/*
		 * If the learned capacity is out of range by 50% from the
		 * nominal capacity, then overwrite the learned capacity with
		 * the nominal capacity.
		 */
		if (cl->nom_cap_uah && delta_cap_uah > pct_nom_cap_uah) {
			pr_debug("learned_cap_uah: %lld is higher than expected, capping it to nominal: %lld\n",
				cl->learned_cap_uah, cl->nom_cap_uah);
			cl->learned_cap_uah = cl->nom_cap_uah;
		}
#endif
		rc = cl->store_learned_capacity(cl->data, cl->learned_cap_uah);
		if (rc < 0)
			pr_err("Error in storing learned_cap_uah, rc=%d\n", rc);
	}

out:
	mutex_unlock(&cl->lock);
	return rc;
}

/**
 * cap_learning_init -
 * @cl: Capacity learning object
 *
 * FG/QG have to call this during driver probe to validate the required
 * parameters after allocating cap_learning object.
 *
 */
int cap_learning_init(struct cap_learning *cl)
{
	if (!cl)
		return -ENODEV;

	if (!cl->get_learned_capacity || !cl->store_learned_capacity ||
		!cl->get_cc_soc) {
		pr_err("Insufficient functions for supporting capacity learning\n");
		return -EINVAL;
	}

	if (!cl->cc_soc_max) {
		pr_err("Insufficient parameters for supporting capacity learning\n");
		return -EINVAL;
	}

	mutex_init(&cl->lock);
	return 0;
}

/* Time to full/empty algorithm  helper functions */

static void ttf_circ_buf_add(struct ttf_circ_buf *buf, int val)
{
	buf->arr[buf->head] = val;
	buf->head = (buf->head + 1) % ARRAY_SIZE(buf->arr);
	buf->size = min(++buf->size, (int)ARRAY_SIZE(buf->arr));
}

static void ttf_circ_buf_clr(struct ttf_circ_buf *buf)
{
	buf->size = 0;
	buf->head = 0;
	memset(buf->arr, 0, sizeof(buf->arr));
}

static int cmp_int(const void *a, const void *b)
{
	return *(int *)a - *(int *)b;
}

static int ttf_circ_buf_median(struct ttf_circ_buf *buf, int *median)
{
	int *temp;

	if (buf->size == 0)
		return -ENODATA;

	if (buf->size == 1) {
		*median = buf->arr[0];
		return 0;
	}

	temp = kmalloc_array(buf->size, sizeof(*temp), GFP_KERNEL);
	if (!temp)
		return -ENOMEM;

	memcpy(temp, buf->arr, buf->size * sizeof(*temp));
	sort(temp, buf->size, sizeof(*temp), cmp_int, NULL);

	if (buf->size % 2)
		*median = temp[buf->size / 2];
	else
		*median = (temp[buf->size / 2 - 1] + temp[buf->size / 2]) / 2;

	kfree(temp);
	return 0;
}

static int ttf_lerp(const struct ttf_pt *pts, size_t tablesize,
						s32 input, s32 *output)
{
	int i;
	s64 temp;

	if (pts == NULL) {
		pr_err("Table is NULL\n");
		return -EINVAL;
	}

	if (tablesize < 1) {
		pr_err("Table has no entries\n");
		return -ENOENT;
	}

	if (tablesize == 1) {
		*output = pts[0].y;
		return 0;
	}

	if (pts[0].x > pts[1].x) {
		pr_err("Table is not in acending order\n");
		return -EINVAL;
	}

	if (input <= pts[0].x) {
		*output = pts[0].y;
		return 0;
	}

	if (input >= pts[tablesize - 1].x) {
		*output = pts[tablesize - 1].y;
		return 0;
	}

	for (i = 1; i < tablesize; i++) {
		if (input >= pts[i].x)
			continue;

		temp = ((s64)pts[i].y - pts[i - 1].y) *
						((s64)input - pts[i - 1].x);
		temp = div_s64(temp, pts[i].x - pts[i - 1].x);
		*output = temp + pts[i - 1].y;
		return 0;
	}

	return -EINVAL;
}

static int get_step_chg_current_window(struct ttf *ttf)
{
	struct range_data *step_chg_cfg = ttf->step_chg_cfg;
	int i, rc, curr_window, vbatt;

	if (ttf->mode == TTF_MODE_V_STEP_CHG) {
		rc =  ttf->get_ttf_param(ttf->data, TTF_VBAT, &vbatt);
		if (rc < 0) {
			pr_err("failed to get battery voltage, rc=%d\n", rc);
			return rc;
		}
	} else {
		rc = ttf->get_ttf_param(ttf->data, TTF_OCV, &vbatt);
		if (rc < 0) {
			pr_err("failed to get battery OCV, rc=%d\n", rc);
			return rc;
		}
	}

	curr_window = ttf->step_chg_num_params - 1;
	for (i = 0; i < ttf->step_chg_num_params; i++) {
		if (is_between(step_chg_cfg[i].low_threshold,
			       step_chg_cfg[i].high_threshold,
			       vbatt))
			curr_window = i;
	}

	return curr_window;
}

static int get_time_to_full_locked(struct ttf *ttf, int *val)
{
	struct step_chg_data *step_chg_data = ttf->step_chg_data;
	struct range_data *step_chg_cfg = ttf->step_chg_cfg;
	int rc, ibatt_avg, vbatt_avg, rbatt = 0, msoc = 0, act_cap_mah = 0,
		i_cc2cv = 0, soc_cc2cv, tau, divisor, iterm = 0, ttf_mode = 0,
		i, soc_per_step, msoc_this_step, msoc_next_step,
		ibatt_this_step, t_predicted_this_step, ttf_slope,
		t_predicted_cv, t_predicted = 0, charge_type = 0, i_step,
		float_volt_uv = 0;
	int multiplier, curr_window = 0, pbatt_avg;
	bool power_approx = false;
	s64 delta_ms;

	rc = ttf->get_ttf_param(ttf->data, TTF_MSOC, &msoc);
	if (rc < 0) {
		pr_err("failed to get msoc rc=%d\n", rc);
		return rc;
	}
	pr_debug("TTF: msoc=%d\n", msoc);

	/* the battery is considered full if the SOC is 100% */
	if (msoc >= 100) {
		*val = 0;
		return 0;
	}

	rc = ttf->get_ttf_param(ttf->data, TTF_MODE, &ttf_mode);

	/* when switching TTF algorithms the TTF needs to be reset */
	if (ttf->mode != ttf_mode) {
		ttf_circ_buf_clr(&ttf->ibatt);
		ttf_circ_buf_clr(&ttf->vbatt);
		ttf->last_ttf = 0;
		ttf->last_ms = 0;
		ttf->mode = ttf_mode;
	}

	/* at least 10 samples are required to produce a stable IBATT */
	if (ttf->ibatt.size < MAX_TTF_SAMPLES) {
		*val = -1;
		return 0;
	}

	rc = ttf_circ_buf_median(&ttf->ibatt, &ibatt_avg);
	if (rc < 0) {
		pr_err("failed to get IBATT AVG rc=%d\n", rc);
		return rc;
	}

	rc = ttf_circ_buf_median(&ttf->vbatt, &vbatt_avg);
	if (rc < 0) {
		pr_err("failed to get VBATT AVG rc=%d\n", rc);
		return rc;
	}

	ibatt_avg = -ibatt_avg / MILLI_UNIT;
	vbatt_avg /= MILLI_UNIT;

	rc = ttf->get_ttf_param(ttf->data, TTF_ITERM, &iterm);
	if (rc < 0) {
		pr_err("failed to get iterm rc=%d\n", rc);
		return rc;
	}
	/* clamp ibatt_avg to iterm */
	if (ibatt_avg < abs(iterm))
		ibatt_avg = abs(iterm);

	rc =  ttf->get_ttf_param(ttf->data, TTF_RBATT, &rbatt);
	if (rc < 0) {
		pr_err("failed to get battery resistance rc=%d\n", rc);
		return rc;
	}
	rbatt /= MILLI_UNIT;

	rc =  ttf->get_ttf_param(ttf->data, TTF_FCC, &act_cap_mah);
	if (rc < 0) {
		pr_err("failed to get ACT_BATT_CAP rc=%d\n", rc);
		return rc;
	}

	pr_debug("TTF: ibatt_avg=%d vbatt_avg=%d rbatt=%d act_cap_mah=%d\n",
				ibatt_avg, vbatt_avg, rbatt, act_cap_mah);

	rc =  ttf->get_ttf_param(ttf->data, TTF_VFLOAT, &float_volt_uv);
	if (rc < 0) {
		pr_err("failed to get float_volt_uv rc=%d\n", rc);
		return rc;
	}

	rc =  ttf->get_ttf_param(ttf->data, TTF_CHG_TYPE, &charge_type);
	if (rc < 0) {
		pr_err("failed to get charge_type rc=%d\n", rc);
		return rc;
	}

	pr_debug("TTF: mode: %d\n", ttf->mode);

	/* estimated battery current at the CC to CV transition */
	switch (ttf->mode) {
	case TTF_MODE_NORMAL:
	case TTF_MODE_V_STEP_CHG:
	case TTF_MODE_OCV_STEP_CHG:
		i_cc2cv = ibatt_avg * vbatt_avg /
			max(MILLI_UNIT, float_volt_uv / MILLI_UNIT);
		break;
	case TTF_MODE_QNOVO:
		i_cc2cv = min(
			ttf->cc_step.arr[MAX_CC_STEPS - 1] / MILLI_UNIT,
			ibatt_avg * vbatt_avg /
			max(MILLI_UNIT, float_volt_uv / MILLI_UNIT));
		break;
	default:
		pr_err("TTF mode %d is not supported\n", ttf->mode);
		break;
	}
	pr_debug("TTF: i_cc2cv=%d\n", i_cc2cv);

	/* if we are already in CV state then we can skip estimating CC */
	if (charge_type == POWER_SUPPLY_CHARGE_TYPE_TAPER)
		goto cv_estimate;

	/* estimated SOC at the CC to CV transition */
	soc_cc2cv = DIV_ROUND_CLOSEST(rbatt * i_cc2cv, OCV_SLOPE_UV);
	soc_cc2cv = 100 - soc_cc2cv;
	pr_debug("TTF: soc_cc2cv=%d\n", soc_cc2cv);

	switch (ttf->mode) {
	case TTF_MODE_NORMAL:
		if (soc_cc2cv - msoc <= 0)
			goto cv_estimate;

		divisor = max(100, (ibatt_avg + i_cc2cv) / 2 * 100);
		t_predicted = div_s64((s64)act_cap_mah * (soc_cc2cv - msoc) *
						HOURS_TO_SECONDS, divisor);
		break;
	case TTF_MODE_QNOVO:
		soc_per_step = 100 / MAX_CC_STEPS;
		for (i = msoc / soc_per_step; i < MAX_CC_STEPS - 1; ++i) {
			msoc_next_step = (i + 1) * soc_per_step;
			if (i == msoc / soc_per_step)
				msoc_this_step = msoc;
			else
				msoc_this_step = i * soc_per_step;

			/* scale ibatt by 85% to account for discharge pulses */
			ibatt_this_step = min(
					ttf->cc_step.arr[i] / MILLI_UNIT,
					ibatt_avg) * 85 / 100;
			divisor = max(100, ibatt_this_step * 100);
			t_predicted_this_step = div_s64((s64)act_cap_mah *
					(msoc_next_step - msoc_this_step) *
					HOURS_TO_SECONDS, divisor);
			t_predicted += t_predicted_this_step;
			pr_debug("TTF: [%d, %d] ma=%d t=%d\n",
				msoc_this_step, msoc_next_step,
				ibatt_this_step, t_predicted_this_step);
		}
		break;
	case TTF_MODE_V_STEP_CHG:
	case TTF_MODE_OCV_STEP_CHG:
		if (!step_chg_data || !step_chg_cfg)
			break;

		pbatt_avg = vbatt_avg * ibatt_avg;
		curr_window = get_step_chg_current_window(ttf);
		if (curr_window < 0) {
			pr_err("Failed to get step charging window\n");
			return curr_window;
		}

		pr_debug("TTF: curr_window: %d pbatt_avg: %d\n", curr_window,
			pbatt_avg);

		t_predicted_this_step = 0;
		for (i = 0; i < ttf->step_chg_num_params; i++) {
			/*
			 * If Ibatt_avg differs by step charging threshold by
			 * more than 100 mA, then use power approximation to
			 * get charging current step.
			 */

			if (step_chg_cfg[i].value - ibatt_avg > 100)
				power_approx = true;

			/* Calculate OCV for each window */
			if (power_approx) {
				i_step = pbatt_avg / max(MILLI_UNIT,
					(step_chg_cfg[i].high_threshold /
						MILLI_UNIT));
			} else {
				if (i == curr_window)
					i_step = ((step_chg_cfg[i].value /
							MILLI_UNIT) +
							ibatt_avg) / 2;
				else
					i_step = (step_chg_cfg[i].value /
							MILLI_UNIT);
			}

			if (ttf->mode == TTF_MODE_V_STEP_CHG)
				step_chg_data[i].ocv =
					step_chg_cfg[i].high_threshold -
					(rbatt * i_step);
			else
				step_chg_data[i].ocv =
					step_chg_cfg[i].high_threshold;

			/* Calculate SOC for each window */
			step_chg_data[i].soc = (float_volt_uv -
					step_chg_data[i].ocv) / OCV_SLOPE_UV;
			step_chg_data[i].soc = 100 - step_chg_data[i].soc;

			/* Calculate CC time for each window */
			multiplier = act_cap_mah * HOURS_TO_SECONDS;
			if (curr_window > 0 && i < curr_window)
				t_predicted_this_step = 0;
			else if (i == curr_window)
				t_predicted_this_step =
					div_s64((s64)multiplier *
						(step_chg_data[i].soc - msoc),
						i_step);
			else if (i > 0)
				t_predicted_this_step =
					div_s64((s64)multiplier *
						(step_chg_data[i].soc -
						step_chg_data[i - 1].soc),
						i_step);

			if (t_predicted_this_step < 0)
				t_predicted_this_step = 0;

			t_predicted_this_step =
				DIV_ROUND_CLOSEST(t_predicted_this_step, 100);
			pr_debug("TTF: step: %d i_step: %d OCV: %d SOC: %d t_pred: %d\n",
				i, i_step, step_chg_data[i].ocv,
				step_chg_data[i].soc, t_predicted_this_step);
			t_predicted += t_predicted_this_step;
		}

		break;
	default:
		pr_err("TTF mode %d is not supported\n", ttf->mode);
		break;
	}

cv_estimate:
	pr_debug("TTF: t_predicted_cc=%d\n", t_predicted);

	if (charge_type == POWER_SUPPLY_CHARGE_TYPE_TAPER)
		iterm = max(100, abs(iterm));
	else
		iterm = max(100, abs(iterm) + ttf->iterm_delta);

	pr_debug("TTF: iterm=%d\n", iterm);

	if (charge_type == POWER_SUPPLY_CHARGE_TYPE_TAPER)
		tau = max(MILLI_UNIT, ibatt_avg * MILLI_UNIT / iterm);
	else
		tau = max(MILLI_UNIT, i_cc2cv * MILLI_UNIT / iterm);

	rc = ttf_lerp(ttf_ln_table, ARRAY_SIZE(ttf_ln_table), tau, &tau);
	if (rc < 0) {
		pr_err("failed to interpolate tau rc=%d\n", rc);
		return rc;
	}

	/* tau is scaled linearly from 95% to 100% SOC */
	if (msoc >= 95)
		tau = tau * 2 * (100 - msoc) / 10;

	pr_debug("TTF: tau=%d\n", tau);
	t_predicted_cv = div_s64((s64)act_cap_mah * rbatt * tau *
						HOURS_TO_SECONDS, NANO_UNIT);
	pr_debug("TTF: t_predicted_cv=%d\n", t_predicted_cv);
	t_predicted += t_predicted_cv;

	pr_debug("TTF: t_predicted_prefilter=%d\n", t_predicted);
	if (ttf->last_ms != 0) {
		delta_ms = ktime_ms_delta(ktime_get_boottime(),
					  ms_to_ktime(ttf->last_ms));
		if (delta_ms > 10000) {
			ttf_slope = div64_s64(
				((s64)t_predicted - ttf->last_ttf) *
				MICRO_UNIT, delta_ms);
			if (ttf_slope > -100)
				ttf_slope = -100;
			else if (ttf_slope < -2000)
				ttf_slope = -2000;

			t_predicted = div_s64(
				(s64)ttf_slope * delta_ms, MICRO_UNIT) +
				ttf->last_ttf;
			pr_debug("TTF: ttf_slope=%d\n", ttf_slope);
		} else {
			t_predicted = ttf->last_ttf;
		}
	}

	/* clamp the ttf to 0 */
	if (t_predicted < 0)
		t_predicted = 0;

	pr_debug("TTF: t_predicted_postfilter=%d\n", t_predicted);
	*val = t_predicted;
	return 0;
}

/**
 * ttf_get_time_to_full -
 * @ttf: ttf object
 * @val: Average time to full returned to the caller
 *
 * Get Average time to full the battery based on current soc, rbatt
 * battery voltage and charge current etc.
 */
int ttf_get_time_to_full(struct ttf *ttf, int *val)
{
	int rc;

	mutex_lock(&ttf->lock);
	rc = get_time_to_full_locked(ttf, val);
	mutex_unlock(&ttf->lock);

	return rc;
}

#define DELTA_TTF_IBATT_UA      500000
static void ttf_work(struct work_struct *work)
{
	struct ttf *ttf = container_of(work,
				struct ttf, ttf_work.work);
	int rc, ibatt_now, vbatt_now, ttf_now, charge_status, ibatt_avg;
	ktime_t ktime_now;

	mutex_lock(&ttf->lock);
	rc =  ttf->get_ttf_param(ttf->data, TTF_CHG_STATUS, &charge_status);
	if (rc < 0) {
		pr_err("failed to get charge_status rc=%d\n", rc);
		goto end_work;
	}
	if (charge_status != POWER_SUPPLY_STATUS_CHARGING &&
			charge_status != POWER_SUPPLY_STATUS_DISCHARGING)
		goto end_work;

	rc =  ttf->get_ttf_param(ttf->data, TTF_IBAT, &ibatt_now);
	if (rc < 0) {
		pr_err("failed to get battery current, rc=%d\n", rc);
		goto end_work;
	}

	rc =  ttf->get_ttf_param(ttf->data, TTF_VBAT, &vbatt_now);
	if (rc < 0) {
		pr_err("failed to get battery voltage, rc=%d\n", rc);
		goto end_work;
	}

	ttf_circ_buf_add(&ttf->ibatt, ibatt_now);
	ttf_circ_buf_add(&ttf->vbatt, vbatt_now);

	if (charge_status == POWER_SUPPLY_STATUS_CHARGING) {
		rc = ttf_circ_buf_median(&ttf->ibatt, &ibatt_avg);
		if (rc < 0) {
			pr_err("failed to get IBATT AVG rc=%d\n", rc);
			goto end_work;
		}

		/*
		 * While Charging, if Ibatt_now differ from Ibatt_avg by 500mA,
		 * clear Ibatt buffer and refill with settled Ibatt values, to
		 * calculate accurate TTF
		 */
		if (ibatt_now < 0 && (abs(ibatt_now -
					ibatt_avg) >= DELTA_TTF_IBATT_UA)) {
			pr_debug("Clear Ibatt buffer, Ibatt_avg=%d Ibatt_now=%d\n",
					ibatt_avg, ibatt_now);
			ttf_circ_buf_clr(&ttf->ibatt);
		}

		rc = get_time_to_full_locked(ttf, &ttf_now);
		if (rc < 0) {
			pr_err("failed to get ttf, rc=%d\n", rc);
			goto end_work;
		}

		/* keep the wake lock and prime the IBATT and VBATT buffers */
		if (ttf_now < 0) {
			/* delay for one FG cycle */
			schedule_delayed_work(&ttf->ttf_work,
					msecs_to_jiffies(1000));
			mutex_unlock(&ttf->lock);
			return;
		}

		/* update the TTF reference point every minute */
		ktime_now = ktime_get_boottime();
		if (ktime_ms_delta(ktime_now,
				   ms_to_ktime(ttf->last_ms)) > 60000 ||
				   ttf->last_ms == 0) {
			ttf->last_ttf = ttf_now;
			ttf->last_ms = ktime_to_ms(ktime_now);
		}
	}

	/* recurse every 10 seconds */
	schedule_delayed_work(&ttf->ttf_work, msecs_to_jiffies(ttf->period_ms));
end_work:
	ttf->awake_voter(ttf->data, false);
	mutex_unlock(&ttf->lock);
}

/**
 * ttf_get_time_to_empty -
 * @ttf: ttf object
 * @val: Average time to empty returned to the caller
 *
 * Get Average time to empty the battery based on current soc
 * and average battery current.
 */
int ttf_get_time_to_empty(struct ttf *ttf, int *val)
{
	int rc, ibatt_avg, msoc, act_cap_mah, divisor;

	rc = ttf_circ_buf_median(&ttf->ibatt, &ibatt_avg);
	if (rc < 0) {
		/* try to get instantaneous current */
		rc = ttf->get_ttf_param(ttf->data, TTF_IBAT, &ibatt_avg);
		if (rc < 0) {
			pr_err("failed to get battery current, rc=%d\n", rc);
			return rc;
		}
	}

	ibatt_avg /= MILLI_UNIT;
	/* clamp ibatt_avg to 100mA */
	if (ibatt_avg < 100)
		ibatt_avg = 100;

	rc = ttf->get_ttf_param(ttf->data, TTF_MSOC, &msoc);
	if (rc < 0) {
		pr_err("Error in getting capacity, rc=%d\n", rc);
		return rc;
	}

	rc = ttf->get_ttf_param(ttf->data, TTF_FCC, &act_cap_mah);
	if (rc < 0) {
		pr_err("Error in getting ACT_BATT_CAP, rc=%d\n", rc);
		return rc;
	}

	divisor = CENTI_ICORRECT_C0 * 100 + CENTI_ICORRECT_C1 * msoc;
	divisor = ibatt_avg * divisor / 100;
	divisor = max(100, divisor);
	*val = act_cap_mah * msoc * HOURS_TO_SECONDS / divisor;
	return 0;
}

/**
 * ttf_update -
 * @ttf: ttf object
 * @input_present: Indicator for input presence
 *
 * Called by FG/QG driver when there is a state change (Charging status, SOC)
 *
 */
void ttf_update(struct ttf *ttf, bool input_present)
{
	int delay_ms;

	if (ttf->input_present == input_present)
		return;

	ttf->input_present = input_present;
	if (input_present)
		/* wait 35 seconds for the input to settle */
		delay_ms = 35000;
	else
		/* wait 5 seconds for current to settle during discharge */
		delay_ms = 5000;

	ttf->awake_voter(ttf->data, true);
	cancel_delayed_work_sync(&ttf->ttf_work);
	mutex_lock(&ttf->lock);
	ttf_circ_buf_clr(&ttf->ibatt);
	ttf_circ_buf_clr(&ttf->vbatt);
	ttf->last_ttf = 0;
	ttf->last_ms = 0;
	mutex_unlock(&ttf->lock);
	schedule_delayed_work(&ttf->ttf_work, msecs_to_jiffies(delay_ms));
}

/**
 * ttf_tte_init -
 * @ttf: Time to full object
 *
 * FG/QG have to call this during driver probe to validate the required
 * parameters after allocating ttf object.
 *
 */
int ttf_tte_init(struct ttf *ttf)
{
	if (!ttf)
		return -ENODEV;

	if (!ttf->awake_voter || !ttf->get_ttf_param) {
		pr_err("Insufficient functions for supporting ttf\n");
		return -EINVAL;
	}

	if (!ttf->iterm_delta)
		ttf->iterm_delta = DEFAULT_TTF_ITERM_DELTA_MA;
	if (!ttf->period_ms)
		ttf->period_ms = DEFAULT_TTF_RUN_PERIOD_MS;

	mutex_init(&ttf->lock);
	INIT_DELAYED_WORK(&ttf->ttf_work, ttf_work);

	return 0;
}
