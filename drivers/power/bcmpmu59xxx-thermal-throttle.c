/*****************************************************************************
 *  Copyright 2001 - 2013 Broadcom Corporation.  All rights reserved.
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

#include <linux/power/bcmpmu59xxx-thermal-throttle.h>
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/power/bcmpmu-fg.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>

static u32 debug_mask = BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT | \
			BCMPMU_PRINT_FLOW;
#define pr_throttle(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info("[THROTTLE]:"args); \
		} \
	} while (0)

#define to_bcmpmu_throttle_data(ptr, mem)	container_of((ptr), \
		struct bcmpmu_throttle_data, mem)

#define TEMP_MULTI_FACTOR			10
#define ADC_READ_TRIES				10
#define ADC_RETRY_DELAY				20 /* 20ms */
#define TEMP_READ_DEBOUNCE			3
#define ACLD_MAX_WAIT_COUNT			10

struct bcmpmu_chrgr_trim_reg {
	u32 addr;
	u8 def_val;
	u8 saved_val;
};

struct bcmpmu_throttle_data {
	struct bcmpmu59xxx *bcmpmu;
	struct mutex mutex;
	struct bcmpmu_throttle_pdata *pdata;
	struct workqueue_struct *throttle_wq;
	struct delayed_work throttle_work;
	struct notifier_block usb_det_nb;
	struct notifier_block acld_nb;
	struct notifier_block chrgr_status_nb;
	enum bcmpmu_chrgr_type_t chrgr_type;
	bool temp_algo_running;
	bool throttle_algo_enabled;
	bool throttle_scheduled;
	/* Variables for restoring charger state */
	int icc_fc_saved;
	struct bcmpmu_chrgr_trim_reg *chrgr_trim_reg;
	int chrgr_trim_reg_sz;
	s8 zone_index;
	s8 previous_index;
	u8 temp_db_cnt;
	u8 high_temp_db_cnt;
	bool acld_algo_finished;
	u8 acld_wait_count;
};

int bcmpmu_throttle_get_temp(struct bcmpmu_throttle_data *tdata, u8 channel,
				u8 mode)
{
	struct bcmpmu_adc_result result;
	int ret = 0;
	int retries = ADC_READ_TRIES;

	while (retries--) {
		ret = bcmpmu_adc_read(tdata->bcmpmu, channel,
			mode, &result);
		if (!ret)
			break;
		msleep(ADC_RETRY_DELAY);
	}
	BUG_ON(retries <= 0);

	pr_throttle(FLOW,
		"PMU Die Temp %d\n", result.conv);
	return result.conv;
}

static void bcmpmu_throttle_restore_charger_state
	(struct bcmpmu_throttle_data *tdata)
{
	struct bcmpmu59xxx *bcmpmu = tdata->bcmpmu;
	int ret = 0;
	u8 reg = 0;
	u8 num;

	pr_throttle(FLOW,
		"Restored Charger state\n");
	/* Restore Charger registers to previous state */
	bcmpmu_set_icc_fc(bcmpmu, tdata->icc_fc_saved);
	pr_throttle(VERBOSE, "icc_fc=%d\n", bcmpmu_get_icc_fc(bcmpmu));

	for (num = 0; num < tdata->chrgr_trim_reg_sz; num++) {
		ret = bcmpmu->write_dev(bcmpmu,
				tdata->chrgr_trim_reg[num].addr,
				tdata->chrgr_trim_reg[num].saved_val);

		if (ret)
			pr_throttle(ERROR, "Register[0x%08x] write Failed\n",
				tdata->chrgr_trim_reg[num].addr);
		ret = bcmpmu->read_dev(bcmpmu,
				tdata->chrgr_trim_reg[num].addr, &reg);

		if (ret)
			pr_throttle(ERROR, "Register[0x%08x] readback Failed\n",
				tdata->chrgr_trim_reg[num].addr);

		pr_throttle(VERBOSE, "Restored Register[0x%08x] = 0x%x\n",
			tdata->chrgr_trim_reg[num].addr, reg);
	}

}

static void bcmpmu_throttle_store_charger_state
			(struct bcmpmu_throttle_data *tdata)
{
	struct bcmpmu59xxx *bcmpmu = tdata->bcmpmu;
	int ret = 0;
	u8 reg = 0;
	u8 num;

	/* Store Charger registers */
	pr_throttle(FLOW, "Saved Charger state\n");
	tdata->icc_fc_saved = bcmpmu_get_icc_fc(bcmpmu);
	pr_throttle(VERBOSE, "icc_fc=%d\n", tdata->icc_fc_saved);

	for (num = 0; num < tdata->chrgr_trim_reg_sz; num++) {
		ret = bcmpmu->read_dev(bcmpmu,
				tdata->chrgr_trim_reg[num].addr, &reg);

		if (ret)
			pr_throttle(ERROR, "Register[0x%08x] read Failed\n",
			tdata->chrgr_trim_reg[num].addr);
		else {
			tdata->chrgr_trim_reg[num].saved_val = reg;
			pr_throttle(VERBOSE, "Stored Register[0x%08x] = 0x%x\n",
				tdata->chrgr_trim_reg[num].addr, reg);
		}
	}

}

static void bcmpmu_set_chrgr_trim_default(struct bcmpmu_throttle_data *tdata)
{
	int i;
	struct bcmpmu59xxx *bcmpmu = tdata->bcmpmu;

	for (i = 0; i < tdata->chrgr_trim_reg_sz; i++)
		bcmpmu->write_dev(bcmpmu, tdata->chrgr_trim_reg[i].addr,
				tdata->chrgr_trim_reg[i].def_val);
}

static void bcmpmu_throttle_post_event(struct bcmpmu_throttle_data *tdata)
{

	pr_throttle(FLOW, "%s Posting Status %d\n",
		__func__, tdata->temp_algo_running);
	bcmpmu_call_notifier(tdata->bcmpmu,
		PMU_THEMAL_THROTTLE_STATUS, &tdata->temp_algo_running);
}

static void bcmpmu_throttle_algo(struct bcmpmu_throttle_data *tdata)
{
	struct bcmpmu59xxx *bcmpmu = tdata->bcmpmu;
	struct batt_temp_curr_map *temp_curr_lut;
	int lut_sz = tdata->pdata->temp_curr_lut_sz;
	int temp, index;
	int cc_curr;

	temp_curr_lut = tdata->pdata->temp_curr_lut;
	temp = bcmpmu_throttle_get_temp(tdata, tdata->pdata->temp_adc_channel,
			tdata->pdata->temp_adc_req_mode);

	if (tdata->temp_algo_running) {
		pr_throttle(FLOW,
		"Temp: %d, Zone: %d , Ibat Limit: %d, Throttle ON\n",
		temp, tdata->zone_index,
		(tdata->zone_index != -1) ?
		temp_curr_lut[tdata->zone_index].curr : 0);
	}

	/* Make sure that the ADC temperature reading
	 * is correct by debouncing
	 */

	if (temp < temp_curr_lut[0].temp)
		index = -1;
	else if (temp >= temp_curr_lut[lut_sz-1].temp) {
		pr_throttle(FLOW,
			"Temp(%d) reached higher cut-off=%d\n",
			temp, temp_curr_lut[lut_sz-1].temp);
		pr_throttle(FLOW, "HW might disable the charging\n");
		index = lut_sz - 1;
	} else {
		for (index = 0; index < lut_sz; index++) {
			if ((temp >= temp_curr_lut[index].temp) &&
					(temp < temp_curr_lut[index+1].temp))
					break;
			else
				continue;
		}
	}

	pr_throttle(VERBOSE, "Current index=%d\n", index);
	pr_throttle(VERBOSE, "zone_index=%d\n",	tdata->zone_index);
	pr_throttle(VERBOSE, "temp_db_cnt=%d\n", tdata->temp_db_cnt);
	pr_throttle(VERBOSE, "high_temp_db_cnt=%d\n", tdata->high_temp_db_cnt);
	pr_throttle(VERBOSE, "previous_index=%d\n", tdata->previous_index);

	if (index == tdata->zone_index) {
		pr_throttle(VERBOSE, "Same Zone, Nothing needs to be done\n");
		tdata->temp_db_cnt = 0;
		tdata->high_temp_db_cnt = 0;
		return;
	}

	if (index > tdata->zone_index) {
		tdata->high_temp_db_cnt++;
		tdata->temp_db_cnt = 0;

		if (tdata->high_temp_db_cnt >= TEMP_READ_DEBOUNCE) {
			if (index != tdata->previous_index) {
				pr_throttle(FLOW,
					"Index Adjusted from %d to %d\n",
					index, tdata->previous_index);
				index = tdata->previous_index;
			}
		} else {
			pr_throttle(FLOW,
				"Zone changed,High temp Debounce : %d\n",
				tdata->high_temp_db_cnt);
			tdata->previous_index = index;
			return;
		}
	} else if (index < tdata->zone_index) {
		if (temp < (temp_curr_lut[tdata->zone_index].temp -
				tdata->pdata->hysteresis_temp))
			tdata->temp_db_cnt++;

		tdata->high_temp_db_cnt = 0;

		if (tdata->temp_db_cnt < TEMP_READ_DEBOUNCE) {
			pr_throttle(FLOW,
				"Zone changed,Hyst Debounce : %d\n",
				tdata->temp_db_cnt);
			return;
		}
	}

	tdata->zone_index = index;
	tdata->temp_db_cnt = 0;
	tdata->high_temp_db_cnt = 0;

	if (index == -1) {
		pr_throttle(FLOW,
			"Normal Temp Zone, Throttling will be Stopped\n");
		bcmpmu_throttle_restore_charger_state(tdata);
		if (tdata->temp_algo_running) {
			tdata->temp_algo_running = false;
			bcmpmu_throttle_post_event(tdata);
		}
		return;
	}

	if (!tdata->temp_algo_running) {
		bcmpmu_throttle_store_charger_state(tdata);
		tdata->temp_algo_running = true;
		bcmpmu_throttle_post_event(tdata);
	}

	pr_throttle(FLOW, "Temp(%d) High, reached %d limit i.e. Zone: %d",
		temp, temp_curr_lut[index].temp, index);
	pr_throttle(FLOW, "Charging current set to %d,Temp Algorithm : %s\n",
		temp_curr_lut[index].curr,
		(tdata->temp_algo_running == 1) ? "ON" : "OFF");

	bcmpmu_set_chrgr_trim_default(tdata);
	cc_curr = bcmpmu_get_icc_fc(tdata->bcmpmu);
	if (cc_curr > temp_curr_lut[index].curr)
		bcmpmu_set_icc_fc(bcmpmu, temp_curr_lut[index].curr);
	else
		pr_throttle(FLOW, "Already charging ar lower current\n");

}

static void bcmpmu_throttle_algo_init(struct bcmpmu_throttle_data *tdata)
{
	/* Intialize the variables */
	tdata->temp_db_cnt = 0;
	tdata->zone_index = -1;
}

static void bcmpmu_throttle_work(struct work_struct *work)
{
	struct bcmpmu_throttle_data *tdata =
		to_bcmpmu_throttle_data(work, throttle_work.work);

	pr_throttle(VERBOSE, "%s called, charger type = %d\n",
		__func__, tdata->chrgr_type);
	tdata->throttle_scheduled = true;
	if (tdata->chrgr_type == PMU_CHRGR_TYPE_DCP) {
		if (tdata->acld_algo_finished) {
			bcmpmu_throttle_algo(tdata);
		} else {
			if (tdata->acld_wait_count >= ACLD_MAX_WAIT_COUNT) {
				/* No Event from ACLD,
				 * so Forcefully set ACLD as finished
				 */
				pr_throttle(FLOW, "No ACLD FINISH event\n");
				tdata->acld_algo_finished = true;
				tdata->acld_wait_count = 0;
			} else {
				tdata->acld_wait_count++;
			pr_throttle(FLOW,
				"Waiting for ACLD FINISH Event, tdata->acld_wait_count = %d\n",
				tdata->acld_wait_count);
			}
		}

	} else
		bcmpmu_throttle_algo(tdata);

	queue_delayed_work(tdata->throttle_wq, &tdata->throttle_work,
			msecs_to_jiffies(tdata->pdata->throttle_poll_time));
	return;
}

static int bcmpmu_throttle_event_handler(struct notifier_block *nb,
		unsigned long event, void *data)
{
	struct bcmpmu_throttle_data *tdata;
	bool enable;
	pr_throttle(FLOW, "%s:event:%lu\n", __func__, event);
	switch (event) {
	case PMU_ACCY_EVT_OUT_CHRGR_TYPE:
		tdata = to_bcmpmu_throttle_data(nb, usb_det_nb);
		tdata->chrgr_type = *(enum bcmpmu_chrgr_type_t *)data;
		if (tdata->chrgr_type == PMU_CHRGR_TYPE_NONE) {
			pr_throttle(FLOW,
				"Charger Removed, Disabling Thermal Throttling\n");
			bcmpmu_throttle_restore_charger_state(tdata);
			cancel_delayed_work_sync(&tdata->throttle_work);
			tdata->temp_algo_running = false;
			tdata->acld_algo_finished = false;
			tdata->throttle_scheduled = false;
			tdata->acld_wait_count = 0;
		} else if (tdata->throttle_algo_enabled &&
				(!tdata->throttle_scheduled)) {
			bcmpmu_throttle_algo_init(tdata);
			queue_delayed_work(tdata->throttle_wq,
					&tdata->throttle_work, 0);
			pr_throttle(FLOW,
				"Charger Connected, Enabling Thermal Throttling\n");
		} else
			pr_throttle(FLOW,
				"Charger Connected, But throttle_ctrl flag is	disabled\n");

		break;
	case PMU_ACLD_EVT_ACLD_STATUS:
		enable = *(bool *)data;
		if (enable) {
			tdata = to_bcmpmu_throttle_data(nb, acld_nb);
			tdata->acld_algo_finished = false;
			tdata->acld_wait_count = 0;
			pr_throttle(FLOW, "ACLD algo START Event Received\n");
		} else {
			tdata = to_bcmpmu_throttle_data(nb, acld_nb);
			tdata->acld_algo_finished = true;
			pr_throttle(FLOW, "ACLD algo FINISH Event Received\n");
		}
		break;

	case PMU_CHRGR_EVT_CHRG_STATUS:
		enable = *(bool *)data;
		pr_throttle(FLOW, "%s: ===== chrgr_status %d\n",
			__func__, enable);
		tdata = to_bcmpmu_throttle_data(nb, chrgr_status_nb);
		if (enable && tdata->chrgr_type &&
				tdata->throttle_algo_enabled &&
				(!tdata->throttle_scheduled)) {
			bcmpmu_throttle_algo_init(tdata);
			queue_delayed_work(tdata->throttle_wq,
					&tdata->throttle_work, 0);
			pr_throttle(FLOW,
				"Charger Connected, Enabling Thermal Throttling\n");
		} else if ((!enable) && tdata->throttle_scheduled) {
			pr_throttle(FLOW,
				"Chargering Disabled, Disabling Thermal Throttling\n");
			bcmpmu_throttle_restore_charger_state(tdata);
			cancel_delayed_work_sync(&tdata->throttle_work);
			tdata->temp_algo_running = false;
			tdata->acld_algo_finished = false;
			tdata->throttle_scheduled = false;
			tdata->acld_wait_count = 0;
		}
		break;

	}
	return 0;
}

static int bcmpmu_throttle_debugfs_ctrl(void *data, u64 throttle_ctrl)
{
	struct bcmpmu_throttle_data *tdata = data;

	if (throttle_ctrl) {
		if (!tdata->throttle_algo_enabled) {
			if (tdata->chrgr_type != PMU_CHRGR_TYPE_NONE) {
				bcmpmu_throttle_algo_init(tdata);
				queue_delayed_work(tdata->throttle_wq,
					&tdata->throttle_work, 0);
			} else
				pr_throttle(FLOW,
				"No Charger, Throttling starts after charger is connected\n");
			tdata->throttle_algo_enabled = true;
			pr_throttle(FLOW, "Thermal Throttling Enabled\n");
		} else
			pr_throttle(FLOW, "Throttling Already Enabled\n");
	} else {
		if (tdata->throttle_algo_enabled) {
			if (tdata->temp_algo_running)
				bcmpmu_throttle_restore_charger_state(tdata);
			if (tdata->chrgr_type != PMU_CHRGR_TYPE_NONE)
				cancel_delayed_work_sync(&tdata->throttle_work);
			tdata->throttle_algo_enabled = false;
			tdata->temp_algo_running = false;
			pr_throttle(FLOW, "Thermal Throttling Disabled\n");
		} else
			pr_throttle(FLOW, "Throttling Already Disabled\n");
	}

	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(throttle_ctrl_fops,
		NULL, bcmpmu_throttle_debugfs_ctrl, "%llu\n");

static int bcmpmu_throttle_debugfs_lut_open(struct inode *inode,
				struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t bcmpmu_throttle_debugfs_set_lut(struct file *file,
				char const __user *buf,
				size_t count, loff_t *offset)
{
	struct bcmpmu_throttle_data *tdata = file->private_data;
	u32 len = 0, idx[3];
	char *str_ptr;
	int entries = 0;
	char input_str[100];

	memset(input_str, 0, 100);

	if (copy_from_user(input_str, buf, count))
		return -EFAULT;

	str_ptr = &input_str[0];
	input_str[count-1] = '\0';

	pr_throttle(VERBOSE, "input_str:%s:length=%d\n", input_str, count);

	while (*str_ptr) {
		sscanf(str_ptr, "%d%n", &idx[entries], &len);
		str_ptr += len;
		pr_throttle(VERBOSE, "idx[%d]=%d,len=%d\n",
			entries, idx[entries], len);
		entries++;
	}

	if (entries != 3) {
		pr_throttle(ERROR, "Invalid Number of Arguments\n");
		return count;
	}

	if (idx[0] < 0 || idx[0] >= tdata->pdata->temp_curr_lut_sz) {
		pr_throttle(ERROR, "Invalid Index Argument\n");
		return count;
	}

	tdata->pdata->temp_curr_lut[idx[0]].temp = idx[1];
	tdata->pdata->temp_curr_lut[idx[0]].curr = idx[2];

	/* Restart the throttling Algo so that the
	 * new values will take affect.
	 */
	bcmpmu_throttle_debugfs_ctrl(tdata, 0);
	bcmpmu_throttle_debugfs_ctrl(tdata, 1);

	return count;
}

static ssize_t bcmpmu_throttle_debugfs_get_lut(struct file *file,
					char __user *user_buf,
				size_t count, loff_t *ppos)
{
	struct bcmpmu_throttle_data *tdata = file->private_data;
	char out_str[400];
	u32 len = 0;
	u8 loop;

	memset(out_str, 0, sizeof(out_str));

	len += snprintf(out_str+len, sizeof(out_str)-len,
					"Temperature current Lookup table:\n");

	for (loop = 0; loop < tdata->pdata->temp_curr_lut_sz; loop++)
		len += snprintf(out_str+len, sizeof(out_str)-len,
				"index-%d--> {%d, %d}\n", loop,
				 tdata->pdata->temp_curr_lut[loop].temp,
				tdata->pdata->temp_curr_lut[loop].curr);

	len += snprintf(out_str+len, sizeof(out_str)-len,
					"To Update table, use the below format\n");
	len += snprintf(out_str+len, sizeof(out_str)-len,
					"echo <index> <temperature> <current> > temp_curr_data\n");
	len += snprintf(out_str+len, sizeof(out_str)-len,
					"Note: Throttle fails, if Temp not in ascending order\n");

	return simple_read_from_buffer(user_buf, count, ppos,
			out_str, len);
}

static const struct file_operations temp_curr_lut_fops = {
	.open = bcmpmu_throttle_debugfs_lut_open,
	.write = bcmpmu_throttle_debugfs_set_lut,
	.read = bcmpmu_throttle_debugfs_get_lut,
};

static void bcmpmu_throttle_debugfs_init(struct bcmpmu_throttle_data *tdata)
{
	struct dentry *dentry_throttle_dir;
	struct dentry *dentry_throttle_file;
	struct bcmpmu59xxx *bcmpmu = tdata->bcmpmu;

	if (!bcmpmu || !bcmpmu->dent_bcmpmu) {
		pr_throttle(ERROR, "%s: dentry_bcmpmu is NULL", __func__);
		return;
	}

	dentry_throttle_dir =
		debugfs_create_dir("THROTTLE", bcmpmu->dent_bcmpmu);
	if (IS_ERR_OR_NULL(dentry_throttle_dir))
		goto debugfs_clean;

	dentry_throttle_file = debugfs_create_file("throttle_ctrl",
			S_IWUSR | S_IRUSR, dentry_throttle_dir, tdata,
			&throttle_ctrl_fops);
	if (IS_ERR_OR_NULL(dentry_throttle_file))
		goto debugfs_clean;

	dentry_throttle_file = debugfs_create_u32("poll_time",
			S_IWUSR | S_IRUSR, dentry_throttle_dir,
			&tdata->pdata->throttle_poll_time);
	if (IS_ERR_OR_NULL(dentry_throttle_file))
		goto debugfs_clean;

	dentry_throttle_file = debugfs_create_u32("hysteresis_temp",
			S_IWUSR | S_IRUSR, dentry_throttle_dir,
			&tdata->pdata->hysteresis_temp);
	if (IS_ERR_OR_NULL(dentry_throttle_file))
		goto debugfs_clean;

	dentry_throttle_file = debugfs_create_file("temp_curr_data",
			S_IWUSR | S_IRUSR, dentry_throttle_dir, tdata,
			&temp_curr_lut_fops);
	if (IS_ERR_OR_NULL(dentry_throttle_file))
		goto debugfs_clean;

	dentry_throttle_file = debugfs_create_u32("debug_mask",
			S_IWUSR | S_IRUSR, dentry_throttle_dir, &debug_mask);
	if (IS_ERR_OR_NULL(dentry_throttle_file))
		goto debugfs_clean;

	return;

debugfs_clean:
	if (!IS_ERR_OR_NULL(dentry_throttle_dir))
		debugfs_remove_recursive(dentry_throttle_dir);
}
static int bcmpmu_throttle_probe(struct platform_device *pdev)
{
	int ret = 0;
	int i;
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu_throttle_data *tdata;
	struct bcmpmu_chrgr_trim_reg trim_reg;
	u32 charger_type;

	pr_throttle(FLOW, "%s\n", __func__);

	tdata = kzalloc(sizeof(struct bcmpmu_throttle_data), GFP_KERNEL);
	if (tdata == NULL) {
		pr_throttle(FLOW, "%s failed to alloc mem\n", __func__);
			return -ENOMEM;
	}

	tdata->pdata =
		(struct bcmpmu_throttle_pdata *)pdev->dev.platform_data;
	tdata->bcmpmu = bcmpmu;

	tdata->chrgr_trim_reg_sz = tdata->pdata->chrgr_trim_reg_lut_sz;
	tdata->chrgr_trim_reg = kzalloc((sizeof(trim_reg) *
				tdata->chrgr_trim_reg_sz), GFP_KERNEL);
	if (tdata->chrgr_trim_reg == NULL) {
		pr_throttle(FLOW, "%s %d failed to alloc mem\n",
				__func__, __LINE__);
			return -ENOMEM;
	}
	for (i = 0; i < tdata->chrgr_trim_reg_sz; i++) {
		tdata->chrgr_trim_reg[i].addr =
			tdata->pdata->chrgr_trim_reg_lut[i].addr;
		tdata->chrgr_trim_reg[i].def_val =
			tdata->pdata->chrgr_trim_reg_lut[i].val;
	}

	/* Initialize private data */
	tdata->zone_index = -1;
	tdata->throttle_algo_enabled = true;
	tdata->acld_algo_finished = false;
	tdata->temp_algo_running = false;
	tdata->throttle_scheduled = false;

	tdata->throttle_wq =
		create_singlethread_workqueue("bcmpmu_throttle_wq");
	if (IS_ERR_OR_NULL(tdata->throttle_wq)) {
		ret = PTR_ERR(tdata->throttle_wq);
		pr_throttle(ERROR, "%s Failed to create WQ\n", __func__);
		goto error;
	}

	INIT_DELAYED_WORK(&tdata->throttle_work, bcmpmu_throttle_work);

	tdata->usb_det_nb.notifier_call = bcmpmu_throttle_event_handler;
	ret = bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_CHRGR_TYPE,
			&tdata->usb_det_nb);
	if (ret) {
		pr_throttle(FLOW, "%s Failed to add notifier\n", __func__);
		goto error;
	}

	tdata->acld_nb.notifier_call = bcmpmu_throttle_event_handler;
	ret = bcmpmu_add_notifier(PMU_ACLD_EVT_ACLD_STATUS,
			&tdata->acld_nb);
	if (ret) {
		pr_throttle(FLOW,
			"%s Failed to add acld notifier\n", __func__);
		goto unreg_usb_det_nb;
	}
	tdata->chrgr_status_nb.notifier_call = bcmpmu_throttle_event_handler;
	ret = bcmpmu_add_notifier(PMU_CHRGR_EVT_CHRG_STATUS,
			&tdata->chrgr_status_nb);
	if (ret) {
		pr_throttle(FLOW,
			"%s Failed to add chrgr st notifier\n", __func__);
		goto unreg_acld_nb;
	}


	bcmpmu_usb_get(bcmpmu, BCMPMU_USB_CTRL_GET_CHRGR_TYPE, &charger_type);
	if (charger_type != PMU_CHRGR_TYPE_NONE)
		queue_delayed_work(tdata->throttle_wq,
			 &tdata->throttle_work, 0);

#ifdef CONFIG_DEBUG_FS
	bcmpmu_throttle_debugfs_init(tdata);
#endif

	return 0;
unreg_usb_det_nb:
	bcmpmu_remove_notifier(PMU_ACCY_EVT_OUT_CHRGR_TYPE,
				&tdata->usb_det_nb);
unreg_acld_nb:
	bcmpmu_remove_notifier(PMU_ACLD_EVT_ACLD_STATUS,
				&tdata->usb_det_nb);

error:
	kfree(tdata);
	return 0;
}


static int bcmpmu_throttle_remove(struct platform_device *pdev)
{
	pr_throttle(FLOW, "%s\n", __func__);
	return 0;
}

static struct platform_driver bcmpmu_throttle_drv = {
	.driver = {
		.name = "bcmpmu_thermal_throttle",
	},
	.probe = bcmpmu_throttle_probe,
	.remove = bcmpmu_throttle_remove,
};

static int __init bcmpmu_throttle_init(void)
{
	return platform_driver_register(&bcmpmu_throttle_drv);
}
module_init(bcmpmu_throttle_init);

static void __exit bcmpmu_throttle_exit(void)
{
	platform_driver_unregister(&bcmpmu_throttle_drv);
}
module_exit(bcmpmu_throttle_exit);

MODULE_DESCRIPTION("Broadcom PMU Thermal Throttle Driver");
MODULE_LICENSE("GPL");
