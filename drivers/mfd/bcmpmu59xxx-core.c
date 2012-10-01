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
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif

static struct bcmpmu59xxx *bcmpmu_gbl;
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
	if (ret)
		return ret;
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
	sscanf(input_str, "%x%x%x", &map, &reg, &value);

	pr_info("%x %x %x\n", map, reg, value);
	if (map == 0xFF || reg == 0xFF) {
		pr_info("invalid param !!\n");
		return -EFAULT;
	}
	reg_enc = ENC_PMU_REG(FIFO_MODE, map, reg);
	ret = bcmpmu->write_dev(bcmpmu, reg_enc,
			  (u8)value);
	if (ret)
		return ret;
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
	if (!debugfs_create_file("regread", S_IRUSR, bcmpmu->dent_bcmpmu,
				bcmpmu, &debug_pmu_read_fops))
		goto err;

	if (!debugfs_create_file("regwrite", S_IRUSR, bcmpmu->dent_bcmpmu,
				bcmpmu, &debug_pmu_write_fops))
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
	BUG_ON(!bcmpmu_gbl);

	bcmpmu_reg_write_unlock(bcmpmu_gbl);
	bcmpmu_gbl->write_dev(bcmpmu_gbl, PMU_REG_HOSTCTRL1,
			HOSTCTRL1_SW_SHDWN);
}
EXPORT_SYMBOL(bcmpmu_client_power_off);

int bcmpmu_client_hard_reset(u8 reset_reason)
{
	unsigned int val;
	int ret = 0;
	struct rtc_time tm;
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
				(u8 *)tm.tm_year);
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
	ret = bcmpmu_gbl->read_dev(bcmpmu_gbl, PMU_REG_RTC_CORE,
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

	ret = bcmpmu_gbl->write_dev(bcmpmu_gbl, PMU_REG_RTC_CORE,
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
	printk(KERN_INFO "%s: pmu revision info\n",
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

static void bcmpmu_register_init(struct bcmpmu59xxx *pmu)
{
	struct bcmpmu59xxx_platform_data *pdata;
	int i;
	u8 temp;
	pdata = pmu->pdata;
	pr_pmucore(INIT, "%s: register init\n", __func__);
	for (i = 0; i < pdata->init_max; i++) {
		if (!pdata->init_data[i].mask)
			continue;
		else if (pdata->init_data[i].mask ==  0xFF)
			pmu->write_dev(pmu,
					pdata->init_data[i].addr,
					pdata->init_data[i].val);
		else {
			pmu->read_dev(pmu,
					pdata->init_data[i].addr,
					&temp);
			temp &= ~(pdata->init_data[i].mask);
			temp |= pdata->init_data[i].val;
			pmu->write_dev(pmu,
					pdata->init_data[i].addr,
					temp);
		}
	}
}

static int __devinit bcmpmu59xxx_probe(struct platform_device *pdev)
{
	int ret = 0, size;
	struct bcmpmu59xxx *bcmpmu = pdev->dev.platform_data;
	struct mfd_cell *pmucells ;

	pr_pmucore(INIT, " <%s>\n", __func__);

	ret = bcmpmu59xxx_init_pmurev_info(bcmpmu);
	if (ret < 0) {
		pr_pmucore(ERROR,
			"Fail to init PMU rev info: %d\n", ret);
		return ret;
	}
	bcmpmu_gbl = bcmpmu ;
	bcmpmu_register_init(bcmpmu);
#ifdef CONFIG_DEBUG_FS
	bcmpmu59xxx_debug_init(bcmpmu);
#endif
	mfd_add_devices(bcmpmu->dev, -1, irq_devs,
				ARRAY_SIZE(irq_devs), NULL, 0);
	size = bcmpmu_get_pmu_mfd_cell(&pmucells);
	if (pmucells) {
		ret = mfd_add_devices(bcmpmu->dev, -1,
				pmucells, size,
				NULL, 0);
	}
	return ret;
}

static int __devexit bcmpmu59xxx_remove(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu = pdev->dev.platform_data;
	mfd_remove_devices(bcmpmu->dev);
#ifdef CONFIG_DEBUG_FS
	debugfs_remove(bcmpmu->dent_bcmpmu);
#endif
	return 0;
}

static struct platform_driver bcmpmu59xxx_driver = {
	.driver = {
		   .name = "bcmpmu59xxx_core",
		   },
	.probe = bcmpmu59xxx_probe,
	.remove = __devexit_p(bcmpmu59xxx_remove),
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
