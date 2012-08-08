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
#include <linux/rtc.h>

#include <linux/mfd/bcmpmu.h>

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif

static struct bcmpmu *bcmpmu_core;

#ifdef CONFIG_MFD_BCMPMU_DBG
static ssize_t
store_adc_req(struct device *dev, struct device_attribute *attr,
	      const char *buf, size_t count)
{
	struct bcmpmu_adc_req adc;
	struct bcmpmu *bcmpmu = dev->platform_data;
	sscanf(buf, "%x, %x", &adc.sig, &adc.tm);
	adc.flags = PMU_ADC_RAW_AND_UNIT;
	if (bcmpmu->adc_req) {
		bcmpmu->adc_req(bcmpmu, &adc);
		printk(KERN_INFO "%s: ADC raw = %d, cal = %d, cnv = %d\n",
				__func__, adc.raw, adc.cal, adc.cnv);
	} else
		printk(KERN_INFO "%s: adc_req failed\n", __func__);
	return count;
}
static ssize_t
store_rgltr(struct device *dev, struct device_attribute *attr,
	    const char *buf, size_t count)
{
	int volt, mode, enable;
	char name[10];
	struct regulator *rgltr;
	sscanf(buf, "%d, %d, %d, %s", &volt, &mode, &enable, name);
	rgltr = regulator_get(NULL, name);
	if (IS_ERR(rgltr))
		printk(KERN_INFO "%s: regulator_get failed\n", __func__);
	else {
		if (enable != 0)
			regulator_enable(rgltr);
		else
			regulator_disable(rgltr);
		regulator_set_mode(rgltr, mode);
		regulator_set_voltage(rgltr, volt * 1000, volt * 1000);
	}
	regulator_put(rgltr);
	return count;
}
static ssize_t store_regread(struct device *dev, struct device_attribute *attr,
			     const char *buf, size_t count)
{
	int i;
	struct bcmpmu *bcmpmu = dev->platform_data;
	unsigned int map, addr, len;
	unsigned int val[16];
	sscanf(buf, "%x %x %x", &map, &addr, &len);
	printk("BCMPMU map=0x%X, addr=0x%X, length=0x%X\n", map, addr, len);
	if ((map < 2) && ((addr + len) < 255) && (len < 16)) {
		bcmpmu->read_dev_bulk(bcmpmu, map, addr, &val[0], len);
		for (i = 0; i < len; i++)
			printk("BCMPMU register=0x%X, value=0x%X\n", addr + i,
			       val[i]);
	}
	return count;
}
static ssize_t store_regwrite(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t count)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	unsigned int map, addr, val;
	sscanf(buf, "%x %x %x", &map, &addr, &val);
	printk("BCMPMU map=0x%X, addr=0x%X, val=0x%X\n", map, addr, val);
	if (map < 2)
		bcmpmu->write_dev_drct(bcmpmu, map, addr, val, 0xFF);
	return count;
}

static ssize_t show_reginit(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_platform_data *pdata = bcmpmu->pdata;
	ssize_t count;
	int i;
	count = sprintf(buf, "BCMPMU reg init table\n");
	for (i = 0; i < pdata->init_max; i++)
		count += sprintf(buf+count,
			"map=0x%X, addr=0x%X, val=0x%X, mask=0x%X\n",
			pdata->init_data[i].map,
			pdata->init_data[i].addr,
			pdata->init_data[i].val,
			pdata->init_data[i].mask);
	return count;
}

static DEVICE_ATTR(adcreq, 0644, NULL, store_adc_req);
static DEVICE_ATTR(rgltr, 0644, NULL, store_rgltr);
static DEVICE_ATTR(regread, 0644, NULL, store_regread);
static DEVICE_ATTR(regwrite, 0644, NULL, store_regwrite);
static DEVICE_ATTR(reginit, 0644, show_reginit, NULL);
static DEVICE_ATTR(test, 0644, NULL, NULL);

static struct attribute *bcmpmu_core_attrs[] = {
	&dev_attr_regread.attr,
	&dev_attr_regwrite.attr,
	&dev_attr_adcreq.attr,
	&dev_attr_rgltr.attr,
	&dev_attr_reginit.attr,
	&dev_attr_test.attr,
	NULL
};

static const struct attribute_group bcmpmu_core_attr_group = {
	.attrs = bcmpmu_core_attrs,
};
#endif

void bcmpmu_client_power_off(void)
{
	BUG_ON(!bcmpmu_core);

	bcmpmu_reg_write_unlock(bcmpmu_core);
	bcmpmu_core->write_dev(bcmpmu_core, PMU_REG_SW_SHDWN,
			bcmpmu_core->regmap[PMU_REG_SW_SHDWN].mask,
			bcmpmu_core->regmap[PMU_REG_SW_SHDWN].mask);
}
EXPORT_SYMBOL(bcmpmu_client_power_off);

int bcmpmu_client_hard_reset(unsigned char reset_reason)
{
	unsigned int val;
	int ret = 0;
	struct rtc_time tm;
	unsigned long alarm_time;

	BUG_ON(!bcmpmu_core);

	if ((reset_reason < 1) || (reset_reason > 15)) {
		pr_warning("%s: reset_reason out of range : %d\n",
			__func__, reset_reason);
		reset_reason = 1;
	}

	pr_info("hard reset with reset_reason : %d\n", reset_reason);

	/* read the RTC */
	ret = bcmpmu_core->read_dev(bcmpmu_core, PMU_REG_RTCYR,
				&tm.tm_year, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	tm.tm_year += 100;

	ret = bcmpmu_core->read_dev(bcmpmu_core, PMU_REG_RTCMT,
				&tm.tm_mon, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	tm.tm_mon -= 1;

	ret = bcmpmu_core->read_dev(bcmpmu_core, PMU_REG_RTCDT,
				&tm.tm_mday, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = bcmpmu_core->read_dev(bcmpmu_core, PMU_REG_RTCHR,
				&tm.tm_hour, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = bcmpmu_core->read_dev(bcmpmu_core, PMU_REG_RTCMN,
				&tm.tm_min, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = bcmpmu_core->read_dev(bcmpmu_core, PMU_REG_RTCSC,
				&tm.tm_sec, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	/* Set the alarm as RTC + 2sec */
	rtc_tm_to_time(&tm, &alarm_time);
	alarm_time += 2;
	rtc_time_to_tm(alarm_time, &tm);

	ret = bcmpmu_core->write_dev(bcmpmu_core, PMU_REG_RTCYR_ALM,
				tm.tm_year - 100, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = bcmpmu_core->write_dev(bcmpmu_core, PMU_REG_RTCMT_ALM,
				tm.tm_mon + 1, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = bcmpmu_core->write_dev(bcmpmu_core, PMU_REG_RTCDT_ALM,
				tm.tm_mday, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = bcmpmu_core->write_dev(bcmpmu_core, PMU_REG_RTCHR_ALM,
				tm.tm_hour, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = bcmpmu_core->write_dev(bcmpmu_core, PMU_REG_RTCMN_ALM,
				tm.tm_min, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = bcmpmu_core->write_dev(bcmpmu_core, PMU_REG_RTCSC_ALM,
				tm.tm_sec, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	/* Set the reset reason */
	ret = bcmpmu_core->read_dev(bcmpmu_core, PMU_REG_RTC_CORE,
				&val, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	/*
	 * bit[1:0] should be 0 not to clear
	 * bit[7:2] can be used as scratch register
	 * use bit[7:4] for the hard reset reason
	 */
	val &= ~0xF3;
	val |= (reset_reason << 4);

	ret = bcmpmu_core->write_dev(bcmpmu_core, PMU_REG_RTC_CORE,
				val, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	bcmpmu_client_power_off();

	/* This should never be reached. */
	pr_err("PMU shutdown failure");
	return -EIO;
err:
	pr_err("PMU hard reset failure");
	return ret;
}
EXPORT_SYMBOL(bcmpmu_client_hard_reset);


int bcmpmu_reg_write_unlock(struct bcmpmu *bcmpmu)
{
	unsigned int val;
	bcmpmu->read_dev(bcmpmu, PMU_REG_WRPROEN, &val,
			      PMU_BITMASK_ALL);

	/* If write protection is enabled and locked state */
	if (!(val & (BCMPMU_DIS_WR_PRO | BCMPMU_PMU_UNLOCK)))
		bcmpmu->write_dev(bcmpmu, PMU_REG_WRLOCKKEY,
				BCMPMU_WRLOCKKEY_VAL, 0xff);
	return 0;
}
EXPORT_SYMBOL(bcmpmu_reg_write_unlock);

static int bcmpmu_open(struct inode *inode, struct file *file);
static int bcmpmu_release(struct inode *inode, struct file *file);
static ssize_t bcmpmu_read(struct file *file, char *data, size_t len,
			   loff_t *p);
static ssize_t bcmpmu_write(struct file *file, const char *data, size_t len,
			    loff_t *p);
static long bcmpmu_ioctl_ltp(struct file *file, unsigned int cmd,
				unsigned long arg);

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

static int bcmpmu_open(struct inode *inode, struct file *file)
{
	file->private_data = bcmpmu_core;
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
	struct bcmpmu *bcmpmu = file->private_data;
	struct bcmpmu_rw_data reg;
	ssize_t ret = 0;

	if (data != NULL) {
		if (copy_from_user
		    ((void *)&reg, (void *)data,
		     sizeof(struct bcmpmu_rw_data)) == 0) {
			ret =
			    bcmpmu->read_dev(bcmpmu, reg.addr, &reg.val,
					     reg.mask);
			if (ret != 0) {
				printk(KERN_ERR "%s: read_dev failed.\n",
				       __func__);
				return 0;
			}
			if (copy_to_user
			    ((void *)data, (void *)&reg,
			     sizeof(struct bcmpmu_rw_data)) == 0)
				return sizeof(struct bcmpmu_rw_data);
			else {
				printk(KERN_ERR "%s: failed to copy to user.\n",
				       __func__);
				return 0;
			}
		} else {
			printk(KERN_ERR "%s: failed to copy from user.\n",
			       __func__);
		}
	}
	return ret;
}

static long bcmpmu_ioctl_ltp(struct file *file, unsigned int cmd,
				unsigned long arg)
{
	struct bcmpmu *bcmpmu = file->private_data;
	struct bcmpmu_rw_data_ltp reg;
	struct bcmpmu_ntc_data_ltp ntc;
	struct bcmpmu_adc_req adcreq;
	int i;
	void __user *argp = (void __user *)arg;
	unsigned int value[16];

	ssize_t ret = 0;

	switch (cmd) {
	case BCM_PMU_IOCTL_READ_REG:
		if (copy_from_user
		    ((void *)&reg, argp,
		     sizeof(struct bcmpmu_rw_data_ltp)) == 0) {
			reg.mask = 0xff;
			ret = bcmpmu->read_dev_drct(bcmpmu, reg.map, reg.addr, &value[0], reg.mask);
			if (ret != 0) {
				printk(KERN_ERR "%s: read_dev_drct failed.\n",
				       __func__);
				return 0;
			}else{
				reg.val[0] = value[0] & 0xff;
				printk("BCMPMU register=0x%X, val=0x%X, map=0x%X\n",
                                       reg.addr, reg.val[0], reg.map);
			}
		} else {
			printk(KERN_ERR "%s: failed to copy from user.\n",
			       __func__);
		}
		if (copy_to_user
		    (argp, (void *)&reg,
		     sizeof(struct bcmpmu_rw_data_ltp)) != 0) {
			return -EFAULT;
		}

		break;

	case BCM_PMU_IOCTL_BULK_READ_REG:
		if (copy_from_user
		    ((void *)&reg, argp,
		     sizeof(struct bcmpmu_rw_data_ltp)) == 0) {
			printk("BCMPMU bulk map=0x%X, addr=0x%X, len=0x%X\n",
			       reg.map, reg.addr, reg.len);
			if ((reg.map < 2) && ((reg.addr + reg.len) < 255)
			    && (reg.len < 16)) {
				ret = bcmpmu->read_dev_bulk(bcmpmu, reg.map, reg.addr, &value[0], reg.len);
				if (ret != 0) {
					printk(KERN_ERR
					       "%s: read_dev_bulk failed.\n",
					       __func__);
					return 0;
				}else{
					memcpy(reg.val,value,sizeof(value[reg.len]));
					for (i = 0; i < reg.len; i++){
							reg.val[i] = value[i] & 0xff;
						printk("BCMPMU register=0x%X, value=0x%X\n",
								reg.addr+i, reg.val[i]);
					}
				}
			}
		} else {
			printk(KERN_ERR "%s: failed to copy from user.\n",
			       __func__);
		}
		if (copy_to_user
		    (argp, (void *)&reg,
		     sizeof(struct bcmpmu_rw_data_ltp)) != 0) {
			return -EFAULT;
		}

		break;

	case BCM_PMU_IOCTL_ADC_READ_REG:
		if (copy_from_user
		    ((void *)&adcreq, argp, sizeof(struct bcmpmu_adc_req)) == 0) {
			printk("BCMPMU ADC CH=0x%X, TM=0x%X\n", adcreq.sig,
			       adcreq.tm);
			adcreq.flags = PMU_ADC_RAW_AND_UNIT;

			if (bcmpmu->adc_req)
				bcmpmu->adc_req(bcmpmu, &adcreq);
			else
				printk(KERN_INFO "%s: adc_req failed\n",
				       __func__);

			printk("BCMPMU ADC CH=0x%X, TM=0x%X, CNV=%d\n",
			       adcreq.sig, adcreq.tm, adcreq.cnv);
		} else {
			printk(KERN_ERR "%s: failed to copy from user.\n",
			       __func__);
		}
		if (copy_to_user
		    (argp, (void *)&adcreq, sizeof(struct bcmpmu_adc_req)) != 0) {
			return -EFAULT;
		}

		break;

	case BCM_PMU_IOCTL_WRITE_REG:
		if (copy_from_user
			((void *)&reg, argp,
				sizeof(struct bcmpmu_rw_data_ltp)) == 0) {
			reg.mask = 0xff;
			ret =
			    bcmpmu->write_dev_drct(bcmpmu, reg.map, reg.addr,
						   reg.val[0], reg.mask);
			printk(KERN_DEBUG "BCMPMU register=0x%X, val=0x%X, map=0x%X\n",
			       reg.addr, reg.val[0], reg.map);
			if (ret != 0) {
				printk(KERN_ERR "%s: write_dev_drct failed.\n",
				       __func__);
				return 0;
			}
		} else {
			printk(KERN_ERR "%s: failed to copy from user.\n",
			       __func__);
		}
		if (copy_to_user
		    (argp, (void *)&reg,
		     sizeof(struct bcmpmu_rw_data_ltp)) != 0) {
			return -EFAULT;
		}

		break;

	case BCM_PMU_IOCTL_NTC_TEMP:
		if (copy_from_user
			((void *)&ntc, argp,
			sizeof(struct bcmpmu_ntc_data_ltp)) == 0) {
			ntc.map = 0;
			ntc.mask = 0xff;
			ntc.addr = ntc.addr & 0xff;

			printk(KERN_DEBUG "BCMPMU ntc register=0x%X, val=0x%X, map=0x%X\n",
				ntc.addr, ntc.val[0], ntc.map);
			if (ntc.addr == 0x13) {
				printk(KERN_DEBUG "BCMPMU NTCHT_RISE  val=0x%X\n",
					ntc.val[0]);
				bcmpmu->ntcht_rise_set(bcmpmu, ntc.val[0]);
			} else if (ntc.addr == 0x14) {
				printk(KERN_DEBUG "BCMPMU NTCHT_FALL  val=0x%X\n",
					ntc.val[0]);
				bcmpmu->ntcht_fall_set(bcmpmu, ntc.val[0]);
			} else if (ntc.addr == 0x15) {
				printk(KERN_DEBUG "BCMPMU NTCCT_RISE  val=0x%X\n",
					ntc.val[0]);
				bcmpmu->ntcct_rise_set(bcmpmu, ntc.val[0]);
			} else if (ntc.addr == 0x16) {
				printk(KERN_DEBUG "BCMPMU NTCCT_FALL  val=0x%X\n",
					ntc.val[0]);
				bcmpmu->ntcct_fall_set(bcmpmu, ntc.val[0]);
			}
		} else {
			printk(KERN_ERR "%s: failed to copy from user.\n",
				__func__);
		}
		if (copy_to_user
			(argp, (void *)&ntc,
				sizeof(struct bcmpmu_ntc_data_ltp)) != 0) {
			return -EFAULT;
		}

		break;

	default:
		printk(KERN_ERR "%s: bcmpmu_ioctltest: UNSUPPORTED CMD\n",
			__func__);
		ret = -ENOTTY;
	}
	return ret;
}

static ssize_t bcmpmu_write(struct file *file, const char *data, size_t len,
			    loff_t *p)
{
	struct bcmpmu *bcmpmu = file->private_data;
	struct bcmpmu_rw_data reg;
	ssize_t ret = 0;

	if (data != NULL) {
		if (copy_from_user
		    ((void *)&reg, (void *)data,
		     sizeof(struct bcmpmu_rw_data)) == 0) {
			ret =
			    bcmpmu->write_dev(bcmpmu, reg.addr, reg.val,
					      reg.mask);
			if (ret != 0) {
				printk(KERN_ERR "%s: write_dev failed.\n",
				       __func__);
				return 0;
			} else
				return sizeof(struct bcmpmu_rw_data);
		} else {
			printk(KERN_ERR "%s: failed to copy from user.\n",
			       __func__);
		}
	}
	return ret;
}


static void bcmpmu_register_init(struct bcmpmu *pmu)
{
	struct bcmpmu_platform_data *pdata = pmu->pdata;
	int i;
	printk(KERN_INFO "%s: register init\n", __func__);
	for (i = 0; i < pmu->pdata->init_max; i++) {
		pmu->write_dev_drct(pmu,
				    pdata->init_data[i].map,
				    pdata->init_data[i].addr,
				    pdata->init_data[i].val,
				    pdata->init_data[i].mask);
	}
}

static struct platform_device bcmpmu_irq_device = {
	.name = "bcmpmu_irq",
	.id = -1,
	.dev.platform_data = NULL,
};

static struct platform_device bcmpmu_hwmon_device = {
	.name = "bcmpmu_hwmon",
	.id = -1,
	.dev.platform_data = NULL,
};

static struct platform_device bcmpmu_accy_device = {
	.name = "bcmpmu_accy",
	.id = -1,
	.dev.platform_data = NULL,
};

#ifdef CONFIG_POWER_BCMPMU
static struct platform_device bcmpmu_batt_device = {
	.name = "bcmpmu_batt",
	.id = -1,
	.dev.platform_data = NULL,
};

static struct platform_device bcmpmu_chrgr_device = {
	.name = "bcmpmu_chrgr",
	.id = -1,
	.dev.platform_data = NULL,
};
#endif

static struct platform_device bcmpmu_rtc_device = {
	.name = "bcmpmu_rtc",
	.id = -1,
	.dev.platform_data = NULL,
};

static struct platform_device bcmpmu_ponkey_device = {
	.name = "bcmpmu_ponkey",
	.id = -1,
	.dev.platform_data = NULL,
};

static struct platform_device bcmpmu_regulator = {
	.name = "bcmpmu-regulator",
	.id = -1,
	.dev.platform_data = NULL,
};

static struct platform_device bcmpmu_watchdog = {
	.name = "bcmpmu-wdog",
	.id = -1,
	.dev.platform_data = NULL,
};

static struct platform_device bcmpmu_thermal_device = {
	.name = "bcmpmu_thermal",
	.id = -1,
	.dev.platform_data = NULL,
};

static struct platform_device *bcmpmu_fellow_devices[] = {
	&bcmpmu_irq_device,
	&bcmpmu_hwmon_device,
	&bcmpmu_regulator,
	&bcmpmu_rtc_device,
#ifdef CONFIG_POWER_BCMPMU
	&bcmpmu_batt_device,
	&bcmpmu_chrgr_device,
#endif
	&bcmpmu_accy_device,
	&bcmpmu_ponkey_device,
	&bcmpmu_watchdog,
	&bcmpmu_thermal_device,
};

static int __devinit bcmpmu_probe(struct platform_device *pdev)
{
	int ret;

	int i;
	unsigned int val;
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_platform_data *pdata = bcmpmu->pdata;
#ifdef CONFIG_DEBUG_FS
	struct dentry *root_dir = NULL;
#endif
	printk(KERN_INFO "%s: called.\n", __func__);
	ret = bcmpmu_init_pmurev_info(bcmpmu);
	if (ret < 0) {
		dev_err(bcmpmu->dev, "Failed to init PMU rev info: %d\n", ret);
		goto err;
	} else {
		printk(KERN_INFO "%s: ---- pmu revision info -----\n",
								__func__);
		printk(KERN_INFO "PMU Gen id = 0x%x\n",
					 bcmpmu->rev_info.gen_id);
		printk(KERN_INFO "PMU Project id = 0x%x\n",
						bcmpmu->rev_info.prj_id);
		printk(KERN_INFO "PMU Digital Revison = 0x%x\n",
					bcmpmu->rev_info.dig_rev);
		printk(KERN_INFO "PMU Analog Revision = 0x%x\n",
					bcmpmu->rev_info.ana_rev);
	}

	bcmpmu->regmap = bcmpmu_get_regmap(bcmpmu);

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_PMUID, &val, 0xffffffff);
	if (ret < 0) {
		dev_err(bcmpmu->dev, "Failed to read ID: %d\n", ret);
		goto err;
	} else {
		printk(KERN_INFO "%s: Chip Version = 0x%0X.\n", __func__, val);
	}
	bcmpmu_register_init(bcmpmu);
	ret = misc_register(&bcmpmu_device);
	if (ret < 0)
		goto err;

#ifdef CONFIG_DEBUG_FS
	root_dir = debugfs_create_dir("bcmpmu", 0);
	if (!root_dir) {
		bcmpmu->debugfs_root_dir = NULL;
		printk(KERN_INFO "%s: failed to create debugfs dir.\n",
		       __func__);
	} else
		bcmpmu->debugfs_root_dir = root_dir;
#endif
	/* Make device data accessible */
	bcmpmu_core = bcmpmu;

	for (i = 0; i < ARRAY_SIZE(bcmpmu_fellow_devices); i++)
		bcmpmu_fellow_devices[i]->dev.platform_data = bcmpmu;

	platform_add_devices(bcmpmu_fellow_devices,
			     ARRAY_SIZE(bcmpmu_fellow_devices));

	/* Init other platform devices under framework */
	if (pdata && pdata->init) {
		ret = pdata->init(bcmpmu);
		if (ret != 0) {
			dev_err(bcmpmu->dev, "Platform init() failed: %d\n",
				ret);
			goto err_pdata_init;
		}
	}
#ifdef CONFIG_MFD_BCMPMU_DBG
	ret = sysfs_create_group(&pdev->dev.kobj, &bcmpmu_core_attr_group);
#endif
	return 0;

err_pdata_init:
	misc_deregister(&bcmpmu_device);
err:
	return ret;
}

static int __devexit bcmpmu_remove(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_platform_data *pdata = bcmpmu->pdata;

	misc_deregister(&bcmpmu_device);

	if (pdata && pdata->exit)
		pdata->exit(bcmpmu);
	return 0;
}

static struct platform_driver bcmpmu_driver = {
	.driver = {
		   .name = "bcmpmu_core",
		   },
	.probe = bcmpmu_probe,
	.remove = __devexit_p(bcmpmu_remove),
};

static int __init bcmpmu_init(void)
{
	return platform_driver_register(&bcmpmu_driver);
}

subsys_initcall(bcmpmu_init);

static void __exit bcmpmu_exit(void)
{
	platform_driver_unregister(&bcmpmu_driver);
}

module_exit(bcmpmu_exit);

MODULE_DESCRIPTION("BCM PMIC core driver");
MODULE_LICENSE("GPL");
