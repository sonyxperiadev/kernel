/*****************************************************************************
 *  Copyright 2001 - 2012 Broadcom Corporation.  All rights reserved.
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
#include <linux/mfd/core.h>
#include <linux/uaccess.h>
#include <linux/rtc.h>
#include <linux/notifier.h>
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>
#include <plat/pwr_mgr.h>

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif

static struct bcmpmu59xxx *bcmpmu_gbl;
struct bcmpmu_mbc_cc_trim {
	u8 cc_trim;
	u8 cc_100_trim;
	u8 cc_500_trim;
};
static struct bcmpmu_mbc_cc_trim mbc_cc_trim;

static struct mfd_cell  irq_devs[] = {
	{
		.name = "bcmpmu59xxx_irq",
		.id = -1,
	}
};
static int debug_mask = BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT;
#define pr_pmucore(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)

#ifdef CONFIG_DEBUG_FS


int bcmpmu_debugfs_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t bcmpmu_debugfs_regread(struct file *file,
					   char const __user *buf,
					   size_t count, loff_t *offset)
{
	u32 len = 0;
	int ret, i;
	u32 reg = 0xFF;
	u32 reg_enc;
	u32 map = 0xFF;
	u32 num_reg = 0xFF;
	char input_str[100];
	u8 results[REG_READ_COUNT_MAX];
	struct bcmpmu59xxx *bcmpmu =
			(struct bcmpmu59xxx *)file->private_data;
	BUG_ON(!bcmpmu);
	if (count > 100)
		len = 100;
	else
		len = count;

	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	/* coverity[secure_coding] */
	sscanf(input_str, "%x%x%x", &map, &reg, &num_reg);

	if (map == 0xFF || reg == 0xFF) {
		pr_info("invalid param !!\n");
		return -EFAULT;
	}

	if (num_reg == 0 || num_reg == 0xFF)
		num_reg = 1;
	if (num_reg > REG_READ_COUNT_MAX)
		num_reg = REG_READ_COUNT_MAX;

	reg_enc = ENC_PMU_REG(FIFO_MODE, map, reg);
	ret = bcmpmu->read_dev_bulk(bcmpmu, reg_enc,
			  results, num_reg);
	if (ret < 0) {
		pr_info("%s: read bulk failed\n", __func__);
		/* even on failure we will return count, so
		 * that sysfs does not attempt again
		 */
		return count;
	}
	for (i = 0; i < num_reg; i++, reg++)
		pr_info("[%x] = %x\n", reg, results[i]);
	return count;
}

static ssize_t bcmpmu_debugfs_regwrite(struct file *file,
					   char const __user *buf,
					   size_t count, loff_t *offset)
{
	u32 len = 0;
	int ret;
	u32 reg = 0xFF;
	u32 reg_enc;
	u32 map = 0xFF;
	u32 value;
	char input_str[100];
	struct bcmpmu59xxx *bcmpmu =
		(struct bcmpmu59xxx *)file->private_data;
	BUG_ON(!bcmpmu);

	if (count > 100)
		len = 100;
	else
		len = count;

	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	/* coverity[secure_coding] */
	sscanf(input_str, "%x%x%x", &map, &reg, &value);

	pr_info("%x %x %x\n", map, reg, value);
	if (map == 0xFF || reg == 0xFF) {
		pr_info("invalid param !!\n");
		return -EFAULT;
	}
	reg_enc = ENC_PMU_REG(FIFO_MODE, map, reg);
	ret = bcmpmu->write_dev(bcmpmu, reg_enc,
			  (u8)value);
	if (ret < 0)
		pr_info("%s: write bulk failed\n", __func__);
	return count;
}


static const struct file_operations debug_pmu_read_fops = {
	.write = bcmpmu_debugfs_regread,
	.open = bcmpmu_debugfs_open,
};

static const struct file_operations debug_pmu_write_fops = {
	.write = bcmpmu_debugfs_regwrite,
	.open = bcmpmu_debugfs_open,
};

static void bcmpmu59xxx_debug_init(struct bcmpmu59xxx *bcmpmu)
{
	if (!bcmpmu->dent_bcmpmu) {
		bcmpmu->dent_bcmpmu = debugfs_create_dir("bcmpmu59xxx", NULL);
		if (!bcmpmu->dent_bcmpmu) {
			pr_err("Failed to initialize debugfs\n");
			return;
		}
	}
	if (!debugfs_create_file("regread", S_IWUSR | S_IRUSR,
				bcmpmu->dent_bcmpmu, bcmpmu,
				&debug_pmu_read_fops))
		goto err;

	if (!debugfs_create_file("regwrite", S_IWUSR | S_IRUSR,
				bcmpmu->dent_bcmpmu, bcmpmu,
				&debug_pmu_write_fops))
		goto err;

	if (!debugfs_create_u32("dbg_mask", S_IWUSR | S_IRUSR,
				bcmpmu->dent_bcmpmu, &debug_mask))
		goto err;

	return;
err:
	pr_err("Failed to setup bcmpmu debugfs\n");
	debugfs_remove(bcmpmu->dent_bcmpmu);
}

#endif /* CONFIG_DEBUG_FS */

int bcmpmu_add_notifier(u32 event_id, struct notifier_block *notifier)
{
	if (!bcmpmu_gbl) {
		pr_err("%s: BCMPMU core driver is not initialized\n", __func__);
		return -EAGAIN;
	}

	if (unlikely(event_id >= PMU_EVENT_MAX)) {
		pr_err("%s: Invalid event id\n", __func__);
		return -EINVAL;
	}
	return blocking_notifier_chain_register(
			&bcmpmu_gbl->event[event_id].notifiers, notifier);
}
EXPORT_SYMBOL_GPL(bcmpmu_add_notifier);

int bcmpmu_remove_notifier(u32 event_id, struct notifier_block *notifier)
{
	if (!bcmpmu_gbl) {
		pr_err("%s: BCMPMU core driver is not initialized\n", __func__);
		return -EAGAIN;
	}

	if (unlikely(event_id >= PMU_EVENT_MAX)) {
		pr_err("%s: Invalid event id\n", __func__);
		return -EINVAL;
	}
	return blocking_notifier_chain_unregister(
			&bcmpmu_gbl->event[event_id].notifiers, notifier);
}
EXPORT_SYMBOL_GPL(bcmpmu_remove_notifier);

void bcmpmu_call_notifier(struct bcmpmu59xxx *bcmpmu,
				enum bcmpmu_event_t event, void *para)
{
	pr_pmucore(FLOW, "%s: event send %x\n", __func__, event);
	blocking_notifier_call_chain(&bcmpmu->event[event].notifiers,
								event, para);
}
EXPORT_SYMBOL_GPL(bcmpmu_call_notifier);

int bcmpmu_reg_write_unlock(struct bcmpmu59xxx *bcmpmu)
{
	u8 val;
	bcmpmu->read_dev(bcmpmu, PMU_REG_WRPROEN, &val);

	/* If write protection is enabled and locked state */
	if (!(val & (WRPROEN_DIS_WR_PRO | WRPROEN_PMU_UNLOCK)))
		bcmpmu->write_dev(bcmpmu, PMU_REG_WRLOCKKEY,
				WRLOCKKEY_VAL);
	return 0;
}
EXPORT_SYMBOL(bcmpmu_reg_write_unlock);

void bcmpmu_client_power_off(void)
{
	u8 val;

	BUG_ON(!bcmpmu_gbl);
	preempt_disable();
	local_irq_disable();
	pwr_mgr_pmu_reg_read_direct((u8) DEC_REG_ADD(PMU_REG_WRPROEN),
				bcmpmu_get_slaveid(bcmpmu_gbl,
					PMU_REG_WRPROEN),
				&val);
	if (!(val & (WRPROEN_DIS_WR_PRO | WRPROEN_PMU_UNLOCK)))
		pwr_mgr_pmu_reg_write_direct((u8)
					DEC_REG_ADD(PMU_REG_WRLOCKKEY),
					bcmpmu_get_slaveid(bcmpmu_gbl,
						PMU_REG_WRLOCKKEY),
					WRLOCKKEY_VAL);

	pwr_mgr_pmu_reg_write_direct((u8) DEC_REG_ADD(PMU_REG_HOSTCTRL1),
					bcmpmu_get_slaveid(bcmpmu_gbl,
						PMU_REG_HOSTCTRL1),
					HOSTCTRL1_SW_SHDWN);
	local_irq_enable();
	preempt_enable();
}
EXPORT_SYMBOL(bcmpmu_client_power_off);

int bcmpmu_client_hard_reset(u8 reset_reason)
{
	unsigned int val;
	int ret = 0;
	struct rtc_time tm = {0};
	unsigned long alarm_time;

	BUG_ON(!bcmpmu_gbl);

	if ((reset_reason < 1) || (reset_reason > 15)) {
		pr_pmucore(INIT, "%s: reset_reason out of range : %d\n",
			__func__, reset_reason);
		reset_reason = 1;
	}

	pr_pmucore(INIT, "hard reset with reset_reason : %d\n",
			reset_reason);

	/* read the RTC */
	ret = bcmpmu_gbl->read_dev(bcmpmu_gbl, PMU_REG_RTCYR,
				(u8 *)&tm.tm_year);
	if (unlikely(ret))
		goto err;

	tm.tm_year += 100;

	ret = bcmpmu_gbl->read_dev(bcmpmu_gbl, PMU_REG_RTCMT_WD,
				(u8 *)&tm.tm_mon);
	if (unlikely(ret))
		goto err;

	tm.tm_mon -= 1;

	ret = bcmpmu_gbl->read_dev(bcmpmu_gbl, PMU_REG_RTCDT,
				(u8 *)&tm.tm_mday);
	if (unlikely(ret))
		goto err;

	ret = bcmpmu_gbl->read_dev(bcmpmu_gbl, PMU_REG_RTCHR,
				(u8 *)&tm.tm_hour);
	if (unlikely(ret))
		goto err;

	ret = bcmpmu_gbl->read_dev(bcmpmu_gbl, PMU_REG_RTCMN,
				(u8 *)&tm.tm_min);
	if (unlikely(ret))
		goto err;

	ret = bcmpmu_gbl->read_dev(bcmpmu_gbl, PMU_REG_RTCSC,
				(u8 *)&tm.tm_sec);
	if (unlikely(ret))
		goto err;

	/* Set the alarm as RTC + 2sec */
	rtc_tm_to_time(&tm, &alarm_time);
	alarm_time += 2;
	rtc_time_to_tm(alarm_time, &tm);

	ret = bcmpmu_gbl->write_dev(bcmpmu_gbl, PMU_REG_RTCYR_A1,
				tm.tm_year - 100);
	if (unlikely(ret))
		goto err;

	ret = bcmpmu_gbl->write_dev(bcmpmu_gbl, PMU_REG_RTCMT_A1,
				tm.tm_mon + 1);
	if (unlikely(ret))
		goto err;

	ret = bcmpmu_gbl->write_dev(bcmpmu_gbl, PMU_REG_RTCDT_A1,
				tm.tm_mday);
	if (unlikely(ret))
		goto err;

	ret = bcmpmu_gbl->write_dev(bcmpmu_gbl, PMU_REG_RTCHR_A1,
				tm.tm_hour);
	if (unlikely(ret))
		goto err;

	ret = bcmpmu_gbl->write_dev(bcmpmu_gbl, PMU_REG_RTCMN_A1,
				tm.tm_min);
	if (unlikely(ret))
		goto err;

	ret = bcmpmu_gbl->write_dev(bcmpmu_gbl, PMU_REG_RTCSC_A1,
				tm.tm_sec);
	if (unlikely(ret))
		goto err;

	/* Set the reset reason */
	ret = bcmpmu_gbl->read_dev(bcmpmu_gbl, PMU_REG_RTC_EXTRA,
				(u8 *)&val);
	if (unlikely(ret))
		goto err;

	/*
	 * bit[1:0] should be 0 not to clear
	 * bit[7:2] can be used as scratch register
	 * use bit[7:4] for the hard reset reason
	 */
	val &= ~0xF3;
	val |= (reset_reason << 4);

	ret = bcmpmu_gbl->write_dev(bcmpmu_gbl, PMU_REG_RTC_EXTRA,
				val);
	if (unlikely(ret))
		goto err;

	bcmpmu_client_power_off();

	/* This should never be reached. */
	pr_pmucore(ERROR, "PMU shutdown failure");
	return -EIO;
err:
	pr_pmucore(ERROR, "PMU hard reset failure");
	return ret;
}
EXPORT_SYMBOL(bcmpmu_client_hard_reset);

int bcmpmu59xxx_init_pmurev_info(struct bcmpmu59xxx *bcmpmu)
{
	u8 val;
	int ret;

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_PMUGID, &val);
	if (ret)
		return ret;

	bcmpmu->rev_info.gen_id = (u8)val;
	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_PMUID, &val);
	if (unlikely(ret))
		return ret;

	bcmpmu->rev_info.prj_id = (u8)val;

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_PMUREV, &val);

	if (unlikely(ret))
		return ret;

	bcmpmu->rev_info.dig_rev = ((u8)val) & 0xF;
	bcmpmu->rev_info.ana_rev = (((u8)val) >> 4) & 0xF;
	pr_pmucore(INIT, "%s: pmu revision info\n",
			__func__);
	pr_pmucore(INIT, "PMU Gen id = 0x%x\n",
			bcmpmu->rev_info.gen_id);
	pr_pmucore(INIT, "PMU Project id = 0x%x\n",
			bcmpmu->rev_info.prj_id);
	pr_pmucore(INIT, "PMU Digital Revison = 0x%x\n",
			bcmpmu->rev_info.dig_rev);
	pr_pmucore(INIT, "PMU Analog Revision = 0x%x\n",
			bcmpmu->rev_info.ana_rev);
	return ret;
}

static int bcmpmu_open(struct inode *inode, struct file *file)
{
	file->private_data = bcmpmu_gbl;
	return 0;
}

static int bcmpmu_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

static ssize_t bcmpmu_read(struct file *file, char *data, size_t len,
			   loff_t *p)
{
	struct bcmpmu59xxx *bcmpmu = file->private_data;
	struct bcmpmu59xxx_rw_data reg;
	ssize_t ret = 0;
	u32 reg_enc;

	if (data == NULL)
		ret = -EFAULT;
	else {
		ret = copy_from_user((void *)&reg, (void *)data,
		     sizeof(struct bcmpmu59xxx_rw_data));
		if (!ret) {
			reg_enc = ENC_PMU_REG(FIFO_MODE, (u8)reg.map,
					reg.addr);
			ret = bcmpmu->read_dev(bcmpmu, reg_enc,
					(u8 *)&reg.val);
			if (ret != 0) {
				pr_pmucore(ERROR, "%s: read_dev failed.\n",
				       __func__);
				goto err;
			}
			ret = copy_to_user((void *)data, (void *)&reg,
			     sizeof(struct bcmpmu59xxx_rw_data));
			if (!ret)
				return sizeof(struct bcmpmu59xxx_rw_data);
			else
				pr_pmucore(ERROR, "%s: failed to copy"
						"to user.\n", __func__);
		} else
			pr_pmucore(ERROR, "%s: failed to copy from user.\n",
			       __func__);
	}
err:
	return ret;
}

static long bcmpmu_ioctl_ltp(struct file *file, unsigned int cmd,
				unsigned long arg)
{
	struct bcmpmu59xxx *bcmpmu = file->private_data;
	struct bcmpmu_rw_data_ltp reg;
	void __user *argp = (void __user *)arg;
	unsigned int value[16];
	int i;
	long ret = 0;
	u32 reg_enc;
	switch (cmd) {
	case BCM_PMU_IOCTL_READ_REG:
		ret = copy_from_user((void *)&reg, argp,
		     sizeof(struct bcmpmu_rw_data_ltp));
		if (!ret) {
			reg.mask = 0xff;
			reg_enc = ENC_PMU_REG(FIFO_MODE, (u8)reg.map,
					reg.addr);
			ret = bcmpmu->read_dev(bcmpmu, reg_enc,
					(u8 *)&value[0]);
			if (ret != 0) {
				pr_pmucore(ERROR, "%s: read_dev failed.\n",
				       __func__);
				goto err;
			} else {
				reg.val[0] = value[0] & 0xff;
				pr_pmucore(DATA, "BCMPMU register=0x%X,"
						"val=0x%X,map=0x%X\n",
						reg.addr, reg.val[0],
						reg.map);
			}
		} else {
			pr_pmucore(ERROR, "%s: failed to copy"
					"from user.\n", __func__);
			goto err;
		}
		ret = copy_to_user(argp, (void *)&reg,
		     sizeof(struct bcmpmu_rw_data_ltp));
		if (ret != 0)
			pr_pmucore(ERROR, "%s: failed to copy"
					"to user.\n", __func__);
		break;

	case BCM_PMU_IOCTL_BULK_READ_REG:
		ret = copy_from_user((void *)&reg, argp,
		     sizeof(struct bcmpmu_rw_data_ltp));
		if (!ret) {
			pr_pmucore(DATA, "BCMPMU bulk map=0x%X, addr=0x%X,"
					"len=0x%X\n", reg.map,
					reg.addr, reg.len);
			if ((reg.map < 2) && ((reg.addr + reg.len) < 255)
			    && (reg.len < 16)) {
				reg_enc = ENC_PMU_REG(FIFO_MODE, (u8)reg.map,
						reg.addr);
				ret = bcmpmu->read_dev_bulk(bcmpmu, reg_enc,
						(u8 *)&value[0], reg.len);
				if (ret != 0) {
					pr_pmucore(ERROR,
					       "%s: read_dev_bulk failed.\n",
					       __func__);
					goto err;
				} else {
					memcpy(reg.val, value,
						sizeof(value[reg.len]));
					for (i = 0; i < reg.len; i++) {
						reg.val[i] =
							value[i] & 0xff;
						pr_pmucore(DATA, "BCMPMU"
							"register=0x%X,"
							"value=0x%X\n",
							reg.addr+i,
							reg.val[i]);
					}
				}
			}
		} else {
			pr_pmucore(ERROR, "%s: failed to copy"
					"from user.\n", __func__);
			goto err;
		}
		ret = copy_to_user(argp, (void *)&reg,
		     sizeof(struct bcmpmu_rw_data_ltp));
		if (ret != 0)
			pr_pmucore(ERROR, "%s: failed to copy"
					"to user\n", __func__);
		break;

	case BCM_PMU_IOCTL_WRITE_REG:
		ret = copy_from_user((void *)&reg, argp,
				sizeof(struct bcmpmu_rw_data_ltp));
		if (!ret) {
			reg_enc = ENC_PMU_REG(FIFO_MODE, (u8)reg.map, reg.addr);
			ret = bcmpmu->write_dev(bcmpmu, reg_enc, (reg.val[0] &
					0xff));
			pr_pmucore(DATA, "BCMPMU register=0x%X, val=0x%X,"
					"map=0x%X\n", reg.addr,
					reg.val[0], reg.map);
			if (ret != 0) {
				pr_pmucore(ERROR, "%s: write_dev failed.\n",
				       __func__);
				goto err;
			}
		} else {
			pr_pmucore(ERROR, "%s: failed to copy from user.\n",
			       __func__);
			goto err;
		}
		ret = copy_to_user(argp, (void *)&reg,
		     sizeof(struct bcmpmu_rw_data_ltp));
		if (ret != 0)
			pr_pmucore(ERROR, "%s: failed to copy"
					"to user\n", __func__);
		break;

	default:
		pr_pmucore(ERROR, "%s: bcmpmu_ioctltest: UNSUPPORTED CMD\n",
			__func__);
		ret = -ENOTTY;
	}
err:
	return ret;
}

static ssize_t bcmpmu_write(struct file *file, const char *data, size_t len,
			    loff_t *p)
{
	struct bcmpmu59xxx *bcmpmu = file->private_data;
	struct bcmpmu59xxx_rw_data reg;
	ssize_t ret = 0;
	u32 reg_enc;
	if (data != NULL) {
		ret = copy_from_user((void *)&reg, (void *)data,
			sizeof(struct bcmpmu59xxx_rw_data));
		if (!ret) {
			reg_enc = ENC_PMU_REG(FIFO_MODE, (u8)reg.map, reg.addr);
			ret = bcmpmu->write_dev(bcmpmu, reg_enc, (u8)reg.val);
			if (ret != 0)
				pr_pmucore(ERROR, "%s: write_dev failed.\n",
				       __func__);
			else
				return sizeof(struct bcmpmu59xxx_rw_data);
		} else
			pr_pmucore(ERROR, "%s: failed to copy from user.\n",
					       __func__);
	} else
		ret = -EFAULT;
	return ret;
}

static const struct file_operations bcmpmu_fops = {
	.owner = THIS_MODULE,
	.open = bcmpmu_open,
	.read = bcmpmu_read,
	.unlocked_ioctl = bcmpmu_ioctl_ltp,
	.write = bcmpmu_write,
	.release = bcmpmu_release,
};

static struct miscdevice bcmpmu_device = {
	MISC_DYNAMIC_MINOR, "bcmpmu", &bcmpmu_fops
};

void bcmpmu_restore_cc_500_trim_otp(struct bcmpmu59xxx *bcmpmu)
{
	bcmpmu->write_dev(bcmpmu,
			PMU_REG_MBCCTRL20, mbc_cc_trim.cc_500_trim);

}
/*
 * bcmpmu_restore_cc_trim_otp - Restore trim register to OTP.
 */
void bcmpmu_restore_cc_trim_otp(struct bcmpmu59xxx *bcmpmu)
{
	int ret = 0;

	ret = bcmpmu->write_dev(bcmpmu,
			PMU_REG_MBCCTRL18, mbc_cc_trim.cc_trim);
	if (ret)
		BUG_ON(1);
	ret = bcmpmu->write_dev(bcmpmu,
			PMU_REG_MBCCTRL19, mbc_cc_trim.cc_100_trim);
	if (ret)
		BUG_ON(1);
	ret = bcmpmu->write_dev(bcmpmu,
			PMU_REG_MBCCTRL20, mbc_cc_trim.cc_500_trim);
	if (ret)
		BUG_ON(1);
	pr_pmucore(INIT, "%s: cc_trm: 0x%x cc_100_trm: 0x%x cc_500_trm: 0x%x\n",
			__func__, mbc_cc_trim.cc_trim,
			mbc_cc_trim.cc_100_trim, mbc_cc_trim.cc_500_trim);
}
/**
 * bcmpmu_store_cc_trim_otp - store trim registers.
 */
void bcmpmu_store_cc_trim_otp(struct bcmpmu59xxx *bcmpmu)
{
	int ret = 0;

	ret = bcmpmu->read_dev(bcmpmu,
			PMU_REG_MBCCTRL18, &mbc_cc_trim.cc_trim);
	if (ret)
		BUG_ON(1);
	ret = bcmpmu->read_dev(bcmpmu,
			PMU_REG_MBCCTRL19, &mbc_cc_trim.cc_100_trim);
	if (ret)
		BUG_ON(1);
	ret = bcmpmu->read_dev(bcmpmu,
			PMU_REG_MBCCTRL20, &mbc_cc_trim.cc_500_trim);
	if (ret)
		BUG_ON(1);
	pr_pmucore(INIT, "%s: cc_trm: 0x%x cc_100_trm: 0x%x cc_500_trm: 0x%x\n",
			__func__, mbc_cc_trim.cc_trim,
			mbc_cc_trim.cc_100_trim, mbc_cc_trim.cc_500_trim);
}
static void bcmpmu59xxx_reg_update(struct bcmpmu59xxx *pmu,
		struct bcmpmu59xxx_rw_data *data, int max)
{
	int i;
	u8 temp;
	for (i = 0; i < max; i++) {
		if (!data[i].mask)
			continue;
		else if (data[i].mask ==  0xFF)
			pmu->write_dev(pmu, data[i].addr, data[i].val);
		else {
			pmu->read_dev(pmu, data[i].addr, &temp);
			temp &= ~(data[i].mask);
			temp |= (data[i].val & data[i].mask);
			pmu->write_dev(pmu, data[i].addr, temp);
		}
	}
}

static void bcmpmu_register_init(struct bcmpmu59xxx *pmu)
{
	struct bcmpmu59xxx_platform_data *pdata;
	pdata = pmu->pdata;

	bcmpmu_store_cc_trim_otp(pmu);
	bcmpmu59xxx_reg_update(pmu, pdata->init_data, pdata->init_max);
}

static void bcmpmu59xxx_shutdown(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu59xxx_platform_data *pdata = bcmpmu->pdata;

	bcmpmu59xxx_reg_update(bcmpmu, pdata->exit_data, pdata->exit_max);
	bcmpmu_restore_cc_trim_otp(bcmpmu);
}

static int bcmpmu59xxx_probe(struct platform_device *pdev)
{
	int ret = 0, size, i;
	struct bcmpmu59xxx *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu59xxx_platform_data *pdata = bcmpmu->pdata;
	struct mfd_cell *pmucells ;

	pr_pmucore(INIT, " <%s>\n", __func__);
	ret = bcmpmu59xxx_init_pmurev_info(bcmpmu);
	if (ret < 0) {
		pr_pmucore(ERROR,
			"Fail to init PMU rev info: %d\n", ret);
		goto err;
	}
	bcmpmu_register_init(bcmpmu);

	if (pdata->init)
		pdata->init(bcmpmu);

#ifdef CONFIG_DEBUG_FS
	bcmpmu59xxx_debug_init(bcmpmu);
#endif
	/*Copy flags from pdata*/
	bcmpmu->flags = bcmpmu->pdata->flags;

	/* Check ACLD for A1 PMU */
	if (bcmpmu->rev_info.prj_id == BCMPMU_59054_ID) {
		if (bcmpmu->rev_info.ana_rev >= BCMPMU_59054A1_ANA_REV) {
			if (bcmpmu->flags & BCMPMU_ACLD_EN)
				pr_pmucore(INIT, "ACLD: Enabled");
			else
				pr_pmucore(INIT, "ACLD: Disabled!");
		} else {
			if (bcmpmu->flags & BCMPMU_ACLD_EN) {
				pr_pmucore(INIT,
						"No ACLD for 59054A0 ACLD");
				bcmpmu->flags &= ~BCMPMU_ACLD_EN;
			}
		}
	}
	pr_pmucore(INIT, "bcmpmu->flags = 0x%x\n", bcmpmu->flags);

	bcmpmu_gbl = bcmpmu;
	mfd_add_devices(bcmpmu->dev, -1, irq_devs,
				ARRAY_SIZE(irq_devs), NULL, 0, NULL);
	size = bcmpmu_get_pmu_mfd_cell(&pmucells);
	if (pmucells) {
		ret = mfd_add_devices(bcmpmu->dev, -1,
				pmucells, size,
				NULL, 0, NULL);
		if (ret < 0)
			goto err;
	}
	ret = misc_register(&bcmpmu_device);
	if (ret < 0)
		pr_pmucore(ERROR, "Misc Register failed");

	for (i = 0; i < PMU_EVENT_MAX; i++) {
		bcmpmu->event[i].event_id = i;
		BLOCKING_INIT_NOTIFIER_HEAD(&bcmpmu->event[i].notifiers);
	}
err:
	return ret;
}

static int bcmpmu59xxx_remove(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu = pdev->dev.platform_data;
	mfd_remove_devices(bcmpmu->dev);
#ifdef CONFIG_DEBUG_FS
	debugfs_remove(bcmpmu->dent_bcmpmu);
#endif
	misc_deregister(&bcmpmu_device);
	return 0;
}

static struct platform_driver bcmpmu59xxx_driver = {
	.driver = {
		   .name = "bcmpmu59xxx_core",
		   },
	.probe = bcmpmu59xxx_probe,
	.remove = bcmpmu59xxx_remove,
	.shutdown = bcmpmu59xxx_shutdown,
};

static int __init bcmpmu59xxx_init(void)
{
	int ret ;
	ret =  platform_driver_register(&bcmpmu59xxx_driver);
	return ret;
}
subsys_initcall(bcmpmu59xxx_init);

static void __exit bcmpmu59xxx_exit(void)
{
	platform_driver_unregister(&bcmpmu59xxx_driver);
}
module_exit(bcmpmu59xxx_exit);

MODULE_DESCRIPTION("BCM PMIC core driver");
MODULE_LICENSE("GPL");
