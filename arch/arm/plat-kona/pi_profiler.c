/*****************************************************************************
*
* @file pi_profiler.c
*
* Kona Power Island Profiler
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
#include <linux/io.h>
#include <asm/io.h>
#include <linux/mutex.h>
#include <linux/debugfs.h>
#include <plat/profiler.h>
#include <plat/pi_profiler.h>
#include <plat/pwr_mgr.h>
#include <plat/pi_mgr.h>
#include <mach/kona_timer.h>
#include <linux/hrtimer.h>

static struct dentry *dentry_root_dir;
static struct dentry *dentry_pi_dir;
static bool init;

#define PI_PROF_REG_ADDR(pi, offset)	\
	((u32)(pi->pi_prof_addr_base + offset))

static int pi_prof_init(struct profiler *profiler)
{
	profiler_dbg("%s\n", __func__);
	return 0;
}

static int pi_prof_start(struct profiler *profiler, int start)
{
	struct pi_profiler *pi_profiler = container_of(profiler,
				struct pi_profiler, profiler);

	profiler_dbg("%s\n", __func__);

	if (start == 1) {
		pi_profiler->profiler.flags = PROFILER_RUNNING;
		pwr_mgr_pi_counter_enable(pi_profiler->pi_id, true);
	} else {
		pi_profiler->profiler.flags = 0;
		pwr_mgr_pi_counter_enable(pi_profiler->pi_id, false);
	}

	return 0;
}

static int pi_prof_status(struct profiler *profiler)
{
	return profiler->flags;
}

static int pi_prof_get_counter(struct profiler *profiler,
			       unsigned long *counter,
			       int *overflow)
{
	struct pi_profiler *pi_profiler = container_of(profiler,
				struct pi_profiler, profiler);
	struct pi *pi = pi_mgr_get(pi_profiler->pi_id);

	if (!pi)
		return -ENODEV;
	*overflow = 0;
	*counter = pwr_mgr_pi_counter_read(pi_profiler->pi_id,
					(bool *)overflow);
	return 0;
}

static int pi_prof_print(struct profiler *profiler)
{
	unsigned long counter = 0;
	int err;
	int len_name, len_raw_cnt, len_ms_cnt, overflow;
	u8 buffer[32];
	u8 str_raw_cnt[] = "RAW_CNT:";
	u8 str_ms_cnt[] = "MS_CNT:";

	profiler_dbg("%s\n", __func__);
	err = pi_prof_get_counter(profiler, &counter, &overflow);
	if (err < 0)
		return err;

	len_name = strlen(profiler->name);
	len_raw_cnt = strlen(str_raw_cnt);
	len_ms_cnt = strlen(str_ms_cnt);

	memcpy(buffer, profiler->name, len_name);
	memcpy(buffer + len_name, str_raw_cnt, len_raw_cnt);
	memcpy(buffer + len_name + len_raw_cnt, str_ms_cnt, len_ms_cnt);

	profiler_print(buffer);
	return 0;
}

int pi_prof_clear_counter(struct pi_profiler *pi_profiler)
{
	pm_mgr_pi_count_clear(true);
	return 0;
}

static int set_pi_prof_start(void *data, u64 start)
{
	struct pi_profiler *pi_profiler = data;
	int err = 0;

	if (start == 1) {
		pi_profiler->profiler.start_time = ktime_to_ms(ktime_get());
		err = pi_profiler->profiler.ops->start(&pi_profiler->profiler,
				1);
		if (err < 0)
			pr_err("Failed to start PI profiler %s\n",
					pi_profiler->profiler.name);
		pm_mgr_pi_count_clear(false);
		pi_profiler->profiler.stop_time = 0;
		pi_profiler->profiler.running_time = 0;
	} else if (start == 0) {
		if (!pi_profiler->profiler.ops->status(
					&pi_profiler->profiler))
			return 0;
		pi_profiler->profiler.ops->get_counter(
				&pi_profiler->profiler,
				&pi_profiler->profiler.running_time,
				&pi_profiler->profiler.overflow);
		err = pi_profiler->profiler.ops->start(&pi_profiler->profiler,
				0);
		pm_mgr_pi_count_clear(true);
		pi_profiler->profiler.stop_time = ktime_to_ms(ktime_get());
	}
	return err;
}

static int get_pi_prof_start(void *data, u64 *is_running)
{
	struct pi_profiler *pi_profiler = data;

	*is_running = pi_profiler->profiler.ops->status(&pi_profiler->profiler);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_prof_start_fops,
	get_pi_prof_start, set_pi_prof_start, "%llu\n");

static int get_counter_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static int get_counter_read(struct file *file, char __user *buf, size_t len,
		loff_t *ppos)
{
	struct pi_profiler *pi_profiler = file->private_data;
	int err = 0;
	int count = 0;
	int overflow = 0;
	unsigned long cnt = 0;
	unsigned long duration = 0;
	unsigned long curr_time = 0;
	unsigned long start_time;
	u8 buffer[128];

	int status =
		pi_profiler->profiler.ops->status(&pi_profiler->profiler);
	if (status) {
		curr_time = ktime_to_ms(ktime_get());
		start_time = pi_profiler->profiler.start_time;
		err = pi_profiler->profiler.ops->get_counter(
				&pi_profiler->profiler,
				&cnt, &overflow);
		duration = (curr_time - start_time);
	} else if (pi_profiler->profiler.stop_time) {
		curr_time = pi_profiler->profiler.stop_time;
		start_time = pi_profiler->profiler.start_time;
		duration = (curr_time - start_time);
		overflow = pi_profiler->profiler.overflow;
		cnt = pi_profiler->profiler.running_time;
	}

	if (status) {
		count = snprintf(buffer, 100, "Status: Running" \
				"\t Duration = %lums \t cnt_raw = %lu" \
				"\t cnt = %lu%sms\n",
				duration, cnt, COUNTER_TO_MS(cnt),
				(overflow) ? "* " : " ");
	} else if (pi_profiler->profiler.stop_time) {
		count = snprintf(buffer, 100, "Status: Stopped" \
				"\t Duration = %lums \t cnt_raw = %lu" \
				"\t cnt = %lu%sms\n",
				duration, cnt, COUNTER_TO_MS(cnt),
				(overflow) ? "* " : " ");
	} else
		count = snprintf(buffer, 30, "Status: Yet to begin\n");
	count = simple_read_from_buffer(buf, len, ppos, buffer, count);
	return count;
}

static const struct file_operations pi_prof_counter_fops = {
	.open = get_counter_open,
	.read = get_counter_read,
};

static int pi_profiler_start(struct profiler *profiler, void *data)
{
	struct pi_profiler *pi_profiler;
	int ret;
	if (!profiler) {
		ret = -EINVAL;
		goto err;
	}
	pi_profiler = container_of(profiler, struct pi_profiler,
						profiler);
	ret = pi_profiler->profiler.ops->start(profiler, 1);
	if (!ret)
		profiler->start_time = kona_hubtimer_get_counter();
err:
	return ret;
}

struct prof_ops pi_prof_ops = {
	.init = pi_prof_init,
	.start = pi_prof_start,
	.status = pi_prof_status,
	.get_counter = pi_prof_get_counter,
	.print = pi_prof_print,
	.start_profiler = pi_profiler_start,
};

struct gen_pi_prof_ops gen_pi_prof_ops = {
	.clear_counter = pi_prof_clear_counter,
};

int pi_profiler_register(struct pi_profiler *pi_profiler)
{
	struct dentry *dentry_dir;
	int err = 0;

	if (!pi_profiler)
		return -EINVAL;
	if (!init)
		return -EPERM;

	INIT_LIST_HEAD(&pi_profiler->profiler.node);
	pi_profiler->profiler.ops = &pi_prof_ops;
	err = profiler_register(&pi_profiler->profiler);

	dentry_dir = debugfs_create_dir(pi_profiler->profiler.name,
				dentry_pi_dir);
	if (!dentry_dir)
		goto err_out;

	if (!debugfs_create_file("start", S_IRUSR | S_IWUSR, dentry_dir,
		pi_profiler, &pi_prof_start_fops))
		goto err_out;

	if (!debugfs_create_file("counter", S_IRUSR | S_IWUSR, dentry_dir,
		pi_profiler, &pi_prof_counter_fops))
		goto err_out;

	profiler_dbg("pi registered profiler %s\n",
		pi_profiler->profiler.name);

	return 0;
err_out:
	profiler_dbg("Failed to create directory\n");
	if (dentry_dir)
		debugfs_remove_recursive(dentry_dir);
	profiler_unregister(&pi_profiler->profiler);
	return err;

}
EXPORT_SYMBOL(pi_profiler_register);

int pi_profiler_unregister(struct pi_profiler *pi_profiler)
{

	profiler_dbg("Un-registering pi profiler %s\n",
		pi_profiler->profiler.name);
	profiler_unregister(&pi_profiler->profiler);
	return 0;
}


int __init pi_profiler_init(struct dentry *prof_root_dir)
{
	dentry_root_dir = prof_root_dir;
	dentry_pi_dir = debugfs_create_dir(PROFILER_PI_DIR_NAME, prof_root_dir);
	if (!dentry_pi_dir)
		return -ENOMEM;
	init = false;
	return 0;
}
EXPORT_SYMBOL(pi_profiler_init);
