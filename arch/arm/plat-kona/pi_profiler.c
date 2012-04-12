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

static struct dentry *dentry_root_dir;
static struct dentry *dentry_pi_dir;
static unsigned long init;

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

	/**
	 * By default, PI ON counters are started at boot
	 * It can not be started and stopped at run time due to
	 * know ASIC issue. So to satify the user, we will just
	 * update the flag as running and report to user that
	 * PI ON counter is running now
	 */
	if (start == 1)
		pi_profiler->profiler.flags |= PROFILER_RUNNING;
	else
		pi_profiler->profiler.flags &= ~PROFILER_RUNNING;
#if 0
	if (start == 1)
		pwr_mgr_pi_counter_enable(pi_profiler->pi_id, true);
	else if (start == 0)
		pwr_mgr_pi_counter_enable(pi_profiler->pi_id, false);
#endif
	return 0;
}
static int pi_prof_status(struct profiler *profiler)
{
	struct pi_profiler *pi_profiler = container_of(profiler,
				struct pi_profiler, profiler);

	return pi_profiler->profiler.flags & PROFILER_RUNNING;
#if 0
	if (!pi_profiler->counter_offset) {
		pr_err("counter offset not defined\n");
		return 0;
	}
	reg = readl(PI_PROF_REG_ADDR(pi_profiler,
				pi_profiler->counter_offset));
	return reg & pi_profiler->counter_en_mask;
#endif
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
		err = pi_profiler->profiler.ops->start(&pi_profiler->profiler,
				1);
		if (err < 0)
			pr_err("Failed to start PI profiler %s\n",
					pi_profiler->profiler.name);
		pi_profiler->profiler.start_time =
			kona_hubtimer_get_counter();
	} else if (start == 0) {
		err = pi_profiler->profiler.ops->start(&pi_profiler->profiler,
				0);
		pi_profiler->profiler.start_time = 0;
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
	unsigned long duration_ms = 0;
	unsigned long curr_time = 0;
	u8 buffer[64];

	if (pi_profiler->profiler.ops->status(&pi_profiler->profiler)) {
		curr_time = kona_hubtimer_get_counter();
		err = pi_profiler->profiler.ops->get_counter(
				&pi_profiler->profiler,
				&cnt, &overflow);
		duration_ms = (((curr_time - pi_profiler->profiler.start_time)/
					CLOCK_TICK_RATE) * 1000);
	}
	if (overflow) {
		count = sprintf(buffer,
				"duration_ms = %lu cnt_raw = %lu* cnt_ms = %lu*\n",
				duration_ms, cnt, COUNTER_TO_MS(cnt));
	} else {
		count = sprintf(buffer,
				"duration_ms = %lu cnt_raw = %lu cnt_ms = %lu\n",
				duration_ms, cnt, COUNTER_TO_MS(cnt));
	}
	count = simple_read_from_buffer(buf, len, ppos, buffer, count);
	return count;
}

static const struct file_operations pi_prof_counter_fops = {
	.open = get_counter_open,
	.read = get_counter_read,
};

struct prof_ops pi_prof_ops = {
	.init = pi_prof_init,
	.start = pi_prof_start,
	.status = pi_prof_status,
	.get_counter = pi_prof_get_counter,
	.print = pi_prof_print,
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
	test_and_set_bit(0, &init);
	return 0;
}
EXPORT_SYMBOL(pi_profiler_init);
