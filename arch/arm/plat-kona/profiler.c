/*****************************************************************************
*
* Kona profiler framework
*
* Copyright 2012 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/clk.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <asm/io.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <mach/profiler.h>
#include <mach/kona_timer.h>
#include <mach/timex.h>
#include <plat/clock.h>
#include <plat/profiler.h>
#include <plat/pwr_mgr.h>
#include <linux/hrtimer.h>
#include <plat/ccu_profiler.h>
#include <plat/pi_profiler.h>
#include <asm-generic/errno-base.h>
/**
 * Macros
 */

/**
 * Profiler Logging states
 */
#define LOG_AT_SUSPEND			("suspend")
#define LOG_AT_RESUME			("resume")
#define LOG_AT_SUSP_RES			("susp-res")
#define LOG_PERIODIC			("periodic")
#define LOG_AT_IDLE			("idle")
#define LOG_PROFILER_MAX_LEN		10
/**
 * profiler output options
 * where to dump the profiler log
 */
#define OUTPUT_MTT			("mtt")
#define OUTPUT_CONSOLE			("console")

#define PROF_CIRC_BUFF_SIZE		\
	(sizeof(struct prof_buff) * PROF_CIRC_BUFF_MAX_ENTRIES)

const char *log_states[] = {
	LOG_AT_SUSPEND,
	LOG_AT_RESUME,
	LOG_AT_SUSP_RES,
	LOG_AT_IDLE,
	LOG_PERIODIC,
	NULL,
};

const char *log_outputs[] = {
	OUTPUT_MTT,
	OUTPUT_CONSOLE,
	NULL,
};

struct prof_buff {
	unsigned long time;
	struct profiler *profiler;
	unsigned long counter;
	int overflow;
	char *log_state;
};

struct profiler_data {
	struct list_head profiler_list;
	struct mutex mutex;
	struct dentry *dentry_root_dir;
	struct delayed_work prof_periodic_work;
	struct prof_buff *circ_buff;
	int circ_idx;
	unsigned long period;
	const char *log_state;
	const char *output_to;
	int running;
	unsigned long initialized;
};

static struct profiler_data *profiler_data;
int profiler_debug;
unsigned long strt_time;
module_param_named(profiler_debug, profiler_debug, int, S_IRUSR | S_IWUSR);

static struct profiler *get_profiler(char *name)
{
	struct profiler *profiler;
	list_for_each_entry(profiler, &profiler_data->profiler_list, node)
		if (strcmp(profiler->name, name) == 0)
			return profiler;
	return NULL;
}

int start_profiler(char *name, void *data)
{
	struct profiler *profiler;
	profiler = get_profiler(name);
	if (!profiler)
		return -EINVAL;
	if (profiler->flags & PROFILER_RUNNING)
		return -EBUSY;
	if (!(profiler->ops->start_profiler))
		return -EINVAL;
	return profiler->ops->start_profiler(profiler, data);
}

int stop_profiler(char *name)
{
	struct profiler *profiler;
	u64 duration;
	profiler = get_profiler(name);
	if (!profiler)
		return -EINVAL;
	if ((profiler->flags & PROFILER_RUNNING) == 0)
		return -EINVAL;
	profiler->ops->get_counter(profiler, &profiler->running_time,
			&profiler->overflow);
	profiler->stop_time = kona_hubtimer_get_counter();
	if (profiler->overflow) {
		if (profiler->flags & PROFILER_OVERFLOW) {
			if (((profiler->stop_time - profiler->start_time)
					/CLOCK_TICK_RATE) < 330)
				profiler->overflow = 0;
		} else if (((profiler->stop_time - profiler->start_time)
					/CLOCK_TICK_RATE) < 330)
			profiler->overflow = 0;
	}
	duration = (profiler->stop_time - profiler->start_time);
	printk(KERN_ALERT "Total Duration: %llums ", (duration *
				1000)/CLOCK_TICK_RATE);
	if (profiler->ops->start(profiler, 0))
		return -EBUSY;
	if (profiler->overflow)
		return OVERFLOW_VAL;
	return profiler->running_time/COUNTER_CLK_RATE;
}

static void log_counters(struct profiler_data *profiler_data,
				      char *state)
{
	struct profiler *profiler;
	unsigned long counter;
	int circ_idx;
	int overflow;
	int err = 0;
	if (!profiler_data->running)
		return;

	list_for_each_entry(profiler, &profiler_data->profiler_list, node) {
		if (!profiler->ops->status(profiler))
			continue;
		circ_idx = profiler_data->circ_idx;
		if (circ_idx == PROF_CIRC_BUFF_MAX_ENTRIES)
			profiler_data->circ_idx = 0;
		profiler_data->circ_buff[circ_idx].profiler = profiler;
		profiler_data->circ_buff[circ_idx].log_state = state;
		profiler_data->circ_buff[circ_idx].time =
					ktime_to_ms(ktime_get());
		err = profiler->ops->get_counter(profiler, &counter, &overflow);
		BUG_ON(err < 0);
		profiler_data->circ_buff[circ_idx].counter = counter;
		if (profiler->flags & PROFILER_OVERFLOW)
			overflow = 0;
		profiler_data->circ_buff[circ_idx].overflow = overflow;
		profiler_data->circ_idx++;
		profiler->ops->start(profiler, false);
		profiler->ops->start(profiler, true);
	}
	pm_mgr_pi_count_clear(true);
	pm_mgr_pi_count_clear(false);
}

static void flush_circ_buff(struct profiler_data *profiler_data)
{
	struct prof_buff *prof_buff;
	struct profiler *profiler;
	int idx;
	u8 buffer[128];
	int count = 0;

	profiler_print("\t Name \t  Duration(ms)\t log_state" \
			"    cnt_raw    cnt_ms\n");
	for (idx = 0; idx < profiler_data->circ_idx; idx++) {
		memset(buffer, 0, sizeof(buffer));
		prof_buff = &profiler_data->circ_buff[idx];
		profiler = prof_buff->profiler;
		if ((prof_buff->time - strt_time) < profiler_data->period)
			continue;
		if (profiler->name)
			count = snprintf(buffer, PROF_NAME_MAX_LEN, "%-10s",
					profiler->name);
		else
			count = snprintf(buffer, PROF_NAME_MAX_LEN, "%-10s",
					"unknown");

		count += snprintf(buffer + count, sizeof(buffer) - count,
				" %10lu \t %s     ",
				prof_buff->time - strt_time,
				prof_buff->log_state);
		/**
		 * Append "*" to the counter and counter_ms if the counter
		 * has overflowed (to indicate that counter has overflowed)
		 */
		if (prof_buff->overflow) {
			snprintf(buffer + count, sizeof(buffer) - count,
					"%10lu* %10lu*\n",
					prof_buff->counter,
					COUNTER_TO_MS(prof_buff->counter));
		} else {
			snprintf(buffer + count, sizeof(buffer) - count,
					"%-10lu %-10lu\n",
					prof_buff->counter,
					COUNTER_TO_MS(prof_buff->counter));
		}
		profiler_print(buffer);
	}
	/**
	 * clear the circular buffer and reset the index
	 */
	memset(profiler_data->circ_buff, 0, PROF_CIRC_BUFF_SIZE);
	profiler_data->circ_idx = 0;
}

static void prof_periodic_work(struct work_struct *work)
{
	struct profiler_data *profiler_data = container_of(work,
		struct profiler_data, prof_periodic_work.work);

	mutex_lock(&profiler_data->mutex);
	log_counters(profiler_data, LOG_PERIODIC);
	if (profiler_data->period && profiler_data->running)
		schedule_delayed_work(&profiler_data->prof_periodic_work,
				msecs_to_jiffies(profiler_data->period));
	mutex_unlock(&profiler_data->mutex);
}
int set_output_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

int set_output_write(struct file *file, const char __user *buf, size_t len,
		     loff_t *ppos)
{
	struct profiler_data *profiler_data = file->private_data;
	char cmd[16];
	char *pcmd = &cmd[0];
	char *token;
	int idx;

	if (copy_from_user(cmd, buf, len)) {
		pr_err("%s: copy_from_user failed\n", __func__);
		return -EFAULT;
	}
	if (len == 0)
		return len;
	/* chop of '\n' introduced by echo at the end of the input */
	if (cmd[len - 1] == '\n')
		cmd[len - 1] = '\0';
	token = strsep(&pcmd, " \t");
	if (!token)
		return -EINVAL;
	profiler_dbg("%s\n", token);

	for (idx = 0; ; idx++) {
		if (log_outputs[idx] == NULL)
			break;
		if (strcmp(token, log_outputs[idx]) == 0) {
			profiler_dbg("log out set to %s\n", log_outputs[idx]);
			profiler_data->output_to = log_states[idx];
			break;
		}
	}
	if (log_outputs[idx] == NULL)
		pr_err("Invalid output option\n");
	return len;
}

int set_output_read(struct file *file, char __user *buf, size_t len,
		loff_t *ppos)
{
	struct profiler_data *profiler_data = file->private_data;
	return simple_read_from_buffer(buf, len, ppos, profiler_data->output_to,
		strlen(profiler_data->output_to));
}

static const struct file_operations set_output_fops = {
	.open = set_output_open,
	.write = set_output_write,
	.read = set_output_read,

};

int set_log_state_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}
int set_log_state_write(struct file *file, const char __user *buf, size_t len,
		     loff_t *ppos)
{
	struct profiler_data *profiler_data = file->private_data;
	ssize_t count;
	char cmd[16];
	char *pcmd = &cmd[0];
	char *token;
	int idx;

	count = simple_write_to_buffer(cmd, sizeof(cmd), ppos, buf, len);
	if (count <= 0)
		return count;

	/* chop of '\n' introduced by echo at the end of the input */
	if (cmd[count - 1] == '\n')
		cmd[count - 1] = '\0';
	token = strsep(&pcmd, " \t");
	if (!token)
		return -EINVAL;
	mutex_lock(&profiler_data->mutex);

	if (profiler_data->running) {
		pr_info("Stop profiler first and then set the state\n");
		return -EINVAL;
	}
	for (idx = 0; ; idx++) {
		if (log_states[idx] == NULL)
			break;
		if (strcmp(token, log_states[idx]) == 0) {
			profiler_dbg("log state set to %s\n", log_states[idx]);
			profiler_data->log_state = log_states[idx];
			break;
		}
	}
	if (log_states[idx] == NULL)
		pr_err("Invalid log state\n");
	mutex_unlock(&profiler_data->mutex);
	return count;
}

int get_log_state_read(struct file *file, char __user *buf, size_t len,
		loff_t *ppos)
{
	struct profiler_data *profiler_data = file->private_data;
	size_t count;

	mutex_lock(&profiler_data->mutex);
	count = simple_read_from_buffer(buf, len, ppos,
			profiler_data->log_state,
			strlen(profiler_data->log_state));
	mutex_unlock(&profiler_data->mutex);
	return count;
}

static const struct file_operations set_log_state_fops = {
	.open = set_log_state_open,
	.write = set_log_state_write,
	.read = get_log_state_read,
};

static int set_prof_period(void *data, u64 period)
{
	struct profiler_data *profiler_data = data;
	mutex_lock(&profiler_data->mutex);
	if (!profiler_data->running)
		profiler_data->period = (u32)period;
	else
		pr_err("Stop the profiler first\n");
	mutex_unlock(&profiler_data->mutex);
	return 0;
}

static int get_prof_period(void *data, u64 *period)
{
	struct profiler_data *profiler_data = data;
	mutex_lock(&profiler_data->mutex);
	*period = profiler_data->period;
	mutex_unlock(&profiler_data->mutex);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(prof_period_fops,
	get_prof_period, set_prof_period, "%llu\n");

static int set_prof_flush(void *data, u64 flush)
{
	struct profiler_data *profiler_data = data;
	if (flush != 1)
		return -EINVAL;

	/**
	 * flush the profiler log buffer
	 * on terminal or to MTT
	 */

	mutex_lock(&profiler_data->mutex);
	flush_circ_buff(profiler_data);
	mutex_unlock(&profiler_data->mutex);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(prof_flush_fops,
	NULL, set_prof_flush, "%llu\n");

static int get_prof_start(void *data, u64 *start)
{
	mutex_lock(&profiler_data->mutex);
	*start = profiler_data->running;
	mutex_unlock(&profiler_data->mutex);
	return 0;
}

static int set_prof_start(void *data, u64 start)
{
	struct profiler_data *profiler_data = data;
	int err = 0;
	mutex_lock(&profiler_data->mutex);
	if (start == 1) {
		if (profiler_data->running)
			goto out_unlock;
		if (strcmp(profiler_data->log_state, LOG_PERIODIC) == 0) {
			if (!profiler_data->period) {
				pr_err("Set the period first\n");
				err = -EINVAL;
				goto out_unlock;
			}
			BUG_ON(delayed_work_pending(
					&profiler_data->prof_periodic_work));
			strt_time = ktime_to_ms(ktime_get());
			schedule_delayed_work(
				&profiler_data->prof_periodic_work,
				msecs_to_jiffies(0));
			pr_info("Profiling with period %lums\n",
					profiler_data->period);
		}
		pr_info("To flush the buffers: [echo 1 > flush]\n");
		profiler_data->running = 1;

	} else if (start == 0) {
		if (!profiler_data->running)
			goto out_unlock;
		profiler_data->running = 0;
		mutex_unlock(&profiler_data->mutex);
		cancel_delayed_work_sync(&profiler_data->prof_periodic_work);
		flush_circ_buff(profiler_data);
		pm_mgr_pi_count_clear(true);
		return 0;
	}
out_unlock:
	mutex_unlock(&profiler_data->mutex);
	return err;
}

DEFINE_SIMPLE_ATTRIBUTE(prof_start_fops,
	get_prof_start, set_prof_start, "%llu\n");

int profiler_register(struct profiler *profiler)
{
	if (!profiler || !profiler->ops)
		return -EINVAL;
	if (!profiler_data) {
		profiler_dbg("profiler_data is null\n");
		return -ENOMEM;
	}
	profiler_dbg("Adding profiler %s\n", profiler->name);
	mutex_lock(&profiler_data->mutex);
	list_add_tail(&profiler->node, &profiler_data->profiler_list);
	mutex_unlock(&profiler_data->mutex);
	return 0;
}
EXPORT_SYMBOL(profiler_register);

int profiler_unregister(struct profiler *profiler)
{
	if (!profiler)
		return -EINVAL;
	mutex_lock(&profiler_data->mutex);
	list_del(&profiler->node); /* Should we free the memory node?? */
	mutex_unlock(&profiler_data->mutex);
	return 0;
}
EXPORT_SYMBOL(profiler_unregister);

/**
 * callback from rhea pm debug idle state
 * instrumentation. Called just before entering
 * idle state (WFI)
 */
void profiler_idle_entry_cb(void)
{
	profiler_dbg("%s\n", __func__);
	if (profiler_data)
		if (profiler_data->initialized &&
			(strnicmp(profiler_data->log_state, LOG_AT_IDLE,
				LOG_PROFILER_MAX_LEN) == 0))
			log_counters(profiler_data, LOG_AT_IDLE);
}
EXPORT_SYMBOL(profiler_idle_entry_cb);

void profiler_print(const char *log)
{
	if (log)
		printk(KERN_INFO "%s", log);
}
EXPORT_SYMBOL(profiler_print);

static int __init profiler_probe(struct platform_device *pdev)
{
	profiler_dbg("%s...\n", __func__);
	profiler_data = kzalloc(sizeof(*profiler_data), GFP_KERNEL);
	if (!profiler_data) {
		profiler_dbg("Kzalloc failed\n");
		return -ENOMEM;
	}
	/**
	 * allocate profiler circular buffer
	 */
	profiler_data->circ_buff = kzalloc(PROF_CIRC_BUFF_SIZE, GFP_KERNEL);

	if (!profiler_data->circ_buff) {
		profiler_dbg("Kzalloc failed\n");
		kfree(profiler_data);
		return -ENOMEM;
	}

	mutex_init(&profiler_data->mutex);
	INIT_LIST_HEAD(&profiler_data->profiler_list);
	INIT_DELAYED_WORK(&profiler_data->prof_periodic_work,
		prof_periodic_work);
	profiler_data->output_to = OUTPUT_CONSOLE;
	profiler_data->log_state = LOG_AT_SUSP_RES;

	profiler_data->dentry_root_dir = debugfs_create_dir(
			PROFILER_ROOT_DIR_NAME, 0);
	if (!profiler_data->dentry_root_dir)
		goto clean_debugfs;

	if (!debugfs_create_file("output", S_IRUSR | S_IWUSR,
				profiler_data->dentry_root_dir, profiler_data,
				&set_output_fops))
		goto clean_debugfs;

	if (!debugfs_create_file("period", S_IRUSR | S_IWUSR,
				profiler_data->dentry_root_dir, profiler_data,
				&prof_period_fops))
		goto clean_debugfs;

	if (!debugfs_create_file("flush", S_IRUSR,
				profiler_data->dentry_root_dir, profiler_data,
				&prof_flush_fops))
		goto clean_debugfs;
	if (!debugfs_create_file("log_state", S_IRUSR | S_IWUSR,
				profiler_data->dentry_root_dir, profiler_data,
				&set_log_state_fops))
		goto clean_debugfs;

	if (!debugfs_create_file("start", S_IWUSR,
				profiler_data->dentry_root_dir, profiler_data,
				&prof_start_fops))
		goto clean_debugfs;

	/**
	 * initialize all the profiler
	 */
	if (ccu_profiler_init(profiler_data->dentry_root_dir) < 0)
		goto clean_debugfs;
	if (pi_profiler_init(profiler_data->dentry_root_dir) < 0)
		goto clean_debugfs;

	platform_set_drvdata(pdev, profiler_data);
	/*
	 * initialized successfully. Set the flag
	 * to start processing the regiser/unregister/
	 * logging requests
	 */
	test_and_set_bit(0, &profiler_data->initialized);

	pr_info("%s success\n", __func__);
	return 0;

clean_debugfs:
	pr_err("%s: failed\n", __func__);
	kfree(profiler_data->circ_buff);
	if (profiler_data->dentry_root_dir)
		debugfs_remove_recursive(profiler_data->dentry_root_dir);
	kfree(profiler_data);
	return -ENOMEM;
}

static int profiler_resume(struct platform_device *pdev)
{
	struct profiler_data *profiler_data = platform_get_drvdata(pdev);
	profiler_dbg("%s\n", __func__);
	mutex_lock(&profiler_data->mutex);
	if (profiler_data->running &&
		((strcmp(profiler_data->log_state, LOG_AT_RESUME) == 0) ||
		(strcmp(profiler_data->log_state, LOG_AT_SUSP_RES) == 0)))
		log_counters(profiler_data, LOG_AT_RESUME);
	mutex_unlock(&profiler_data->mutex);
	return 0;
}
static int profiler_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct profiler_data *profiler_data = platform_get_drvdata(pdev);
	profiler_dbg("%s\n", __func__);
	mutex_lock(&profiler_data->mutex);
	if (profiler_data->running &&
		((strcmp(profiler_data->log_state, LOG_AT_SUSPEND) == 0) ||
		(strcmp(profiler_data->log_state, LOG_AT_SUSP_RES) == 0)))
		log_counters(profiler_data, LOG_AT_SUSPEND);
	mutex_unlock(&profiler_data->mutex);
	return 0;
}

static int profiler_remove(struct platform_device *pdev)
{
	struct profiler_data *profiler_data = platform_get_drvdata(pdev);
	profiler_dbg("%s\n", __func__);
	if (profiler_data) {
		cancel_delayed_work_sync(&profiler_data->prof_periodic_work);
		kfree(profiler_data->circ_buff);
		if (profiler_data->dentry_root_dir)
			debugfs_remove_recursive(
					profiler_data->dentry_root_dir);
		kfree(profiler_data);
	}
	return 0;
}

static struct platform_driver __refdata kona_profiler_driver = {
	.probe = profiler_probe,
	.remove = profiler_remove,
	.suspend = profiler_suspend,
	.resume = profiler_resume,
	.driver = {
		.owner = THIS_MODULE,
		.name = "kona_profiler"
	},
};

static int __init profiler_init(void)
{
	profiler_dbg("%s\n", __func__);
	return platform_driver_register(&kona_profiler_driver);
}
module_init(profiler_init);

static void __exit profiler_exit(void)
{
	platform_driver_unregister(&kona_profiler_driver);
}
module_exit(profiler_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Kona Profiler Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
