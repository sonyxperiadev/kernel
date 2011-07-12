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

#include <linux/mfd/bcmpmu.h>

/* To test SIM */
#ifdef CONFIG_MFD_BCMSAMOA
#include <mach/io_map.h>
#include <linux/io.h>
#endif

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif

struct bcmpmu_core_data {
	struct bcmpmu *bcmpmu;
	const struct bcmpmu_env_info *envregmap;
	unsigned int *env_regs;
	int env_size;
	unsigned long int env_status;
};
static struct bcmpmu *bcmpmu_core;

#ifdef CONFIG_MFD_BCMPMU_DBG
static unsigned int map, addr, value, mask;
bcmpmu_show_map(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	return sprintf(buf, "%X\n", map);
}

static ssize_t
bcmpmu_set_map(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	unsigned long val = simple_strtoul(buf, NULL, 0);
	if (val > 1)
		return -EINVAL;
	map = val;
	return n;
}

static ssize_t
bcmpmu_show_addr(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	return sprintf(buf, "%X\n", addr);
}

static ssize_t
bcmpmu_set_addr(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	unsigned long val = simple_strtoul(buf, NULL, 0);
	addr = val;
	return n;
}

static ssize_t
bcmpmu_show_value(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	return sprintf(buf, "%X\n", value);
}

static ssize_t
bcmpmu_set_value(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	unsigned long val = simple_strtoul(buf, NULL, 0);
	value = val;
	return n;
}

static ssize_t
bcmpmu_show_mask(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	return sprintf(buf, "%X\n", mask);
}

static ssize_t
bcmpmu_set_mask(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	unsigned long val = simple_strtoul(buf, NULL, 0);
	mask = val;
	return n;
}

static ssize_t show_reg_read(struct device *dev, struct device_attribute *attr,
				char *buf, size_t count)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	bcmpmu->read_dev_drct(bcmpmu, map, addr, &value, mask);
	return	snprintf(buf, PAGE_SIZE,
		"Read register map=0x%X, addr=0x%X, value=0x%X, mask=0x%x\n",
		map, addr, value, mask);
}
static ssize_t store_reg_write(struct device *dev, struct device_attribute *attr,
				char *buf, size_t count)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	bcmpmu->write_dev_drct(bcmpmu, map, addr, value, mask);
	return count;
}
static ssize_t store_adc_req(struct device *dev, struct device_attribute *attr,
				char *buf, size_t count)
{
	struct bcmpmu_adc_req adc;
	struct bcmpmu *bcmpmu = dev->platform_data;
	sscanf(buf, "%x, %x", &adc.sig, &adc.tm);
	bcmpmu->adc_req(bcmpmu, &adc);
	return count;
}
static ssize_t store_rgltr(struct device *dev, struct device_attribute *attr,
				char *buf, size_t count)
{
	int volt, mode, enable;
	char name[10];
	struct regulator *rgltr;
	sscanf(buf, "%d, %d, %d, %s", &volt, &mode, &enable, name);
	rgltr = regulator_get(NULL, name);
	if (IS_ERR(rgltr))
		printk(KERN_INFO "%s: regulator_get failed\n", __func__);
	else {
		if (enable != 0)regulator_enable(rgltr);
		else regulator_disable(rgltr);
		regulator_set_mode(rgltr, mode);
		regulator_set_voltage(rgltr, volt*1000, volt*1000);
	}
	regulator_put(rgltr);
	return count;
}

static ssize_t show_envupdate(struct device *dev, struct device_attribute *attr,
				char *buf, size_t count)
{
	int i;
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_core_data *pcore = bcmpmu->coreinfo;
	unsigned long status;
	bcmpmu->update_env_status(bcmpmu, &status);

	for (i=0; i<pcore->env_size; i++) {
		sprintf(buf, "%8X\n", pcore->env_regs[i]);
		buf += 9;
	}
	return pcore->env_size * 9;
}

static DEVICE_ATTR(regread, 0644, show_reg_read, NULL);
static DEVICE_ATTR(regwrite, 0644, NULL, store_reg_write);
static DEVICE_ATTR(adcreq, 0644, NULL, store_adc_req);
static DEVICE_ATTR(rgltr, 0644, NULL, store_rgltr);
static DEVICE_ATTR(envupdate, 0644, show_envupdate, NULL);
static DEVICE_ATTR(map, 0644, bcmpmu_show_map, bcmpmu_set_map);
static DEVICE_ATTR(addr, 0644, bcmpmu_show_addr, bcmpmu_set_addr);
static DEVICE_ATTR(value, 0644, bcmpmu_show_value, bcmpmu_set_value);
static DEVICE_ATTR(mask, 0644, bcmpmu_show_mask, bcmpmu_set_mask);

static struct attribute *bcmpmu_core_attrs[] = {
	&dev_attr_regread.attr,
	&dev_attr_regwrite.attr,
	&dev_attr_adcreq.attr,
	&dev_attr_rgltr.attr,
	&dev_attr_envupdate.attr,
	&dev_attr_map.attr,
	&dev_attr_addr.attr,
	&dev_attr_value.attr,
	&dev_attr_mask.attr,
	NULL
};

static const struct attribute_group bcmpmu_core_attr_group = {
	.attrs = bcmpmu_core_attrs,
};
#endif


static int bcmpmu_open(struct inode *inode, struct file *file);
static int bcmpmu_release(struct inode *inode, struct file *file);
static ssize_t bcmpmu_read(struct file *file, char *data, size_t len, loff_t *p);
static ssize_t bcmpmu_write(struct file *file, const char *data, size_t len, loff_t *p);

static const struct file_operations bcmpmu_fops = {
	.owner		= THIS_MODULE,
	.open		= bcmpmu_open,
	.read		= bcmpmu_read,
	.write		= bcmpmu_write,
	.release	= bcmpmu_release,
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

static ssize_t bcmpmu_read(struct file *file, char *data, size_t len, loff_t *p)
{
	struct bcmpmu *bcmpmu = file->private_data;
	struct bcmpmu_rw_data reg;
	ssize_t ret = 0;

	if (data != NULL) {
		if (copy_from_user((void *)&reg, (void *)data, sizeof(struct bcmpmu_rw_data)) == 0) {
			ret = bcmpmu->read_dev(bcmpmu, reg.addr, &reg.val, reg.mask);
			if (ret != 0) {
				printk(KERN_ERR "%s: read_dev failed.\n", __func__);
				return  0;
			}
			if (copy_to_user((void *)data, (void *)&reg, sizeof(struct bcmpmu_rw_data)) == 0)
				return sizeof(struct bcmpmu_rw_data);
			else {
				printk(KERN_ERR "%s: failed to copy to user.\n", __func__);
				return 0;
			}	
		}
		else {
			printk(KERN_ERR "%s: failed to copy from user.\n", __func__);
		}
	}
	return ret;
}

static ssize_t bcmpmu_write(struct file *file, const char *data, size_t len, loff_t *p)
{
	struct bcmpmu *bcmpmu = file->private_data;
	struct bcmpmu_rw_data reg;
	ssize_t ret = 0;

	if (data != NULL) {
		if (copy_from_user((void *)&reg, (void *)data, sizeof(struct bcmpmu_rw_data)) == 0) {
			ret = bcmpmu->write_dev(bcmpmu, reg.addr, reg.val, reg.mask);
			if (ret != 0) {
				printk(KERN_ERR "%s: write_dev failed.\n", __func__);
				return 0;
			}
			else
				return sizeof(struct bcmpmu_rw_data);
		}
		else {
			printk(KERN_ERR "%s: failed to copy from user.\n", __func__);
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
	.name 			= "bcmpmu_irq",
	.id			= -1,
	.dev.platform_data 	= NULL,
};

static struct platform_device bcmpmu_adc_device = {
	.name 			= "bcmpmu_adc",
	.id			= -1,
	.dev.platform_data 	= NULL,
};

static struct platform_device bcmpmu_accy_device = {
	.name 			= "bcmpmu_accy",
	.id			= -1,
	.dev.platform_data 	= NULL,
};

static struct platform_device bcmpmu_batt_device = {
	.name 			= "bcmpmu_batt",
	.id			= -1,
	.dev.platform_data 	= NULL,
};

static struct platform_device bcmpmu_chrgr_device = {
	.name 			= "bcmpmu_chrgr",
	.id			= -1,
	.dev.platform_data 	= NULL,
};

static struct platform_device bcmpmu_rtc_device = {
	.name 			= "bcmpmu_rtc",
	.id			= -1,
	.dev.platform_data 	= NULL,
};

static struct platform_device *bcmpmu_fellow_devices[] = {
	&bcmpmu_irq_device,
	&bcmpmu_adc_device,
	&bcmpmu_rtc_device,
	&bcmpmu_accy_device,
	&bcmpmu_batt_device,
	&bcmpmu_chrgr_device,
};

static void bcmpmu_update_env_status(struct bcmpmu *bcmpmu, unsigned long *env_status)
{
	struct bcmpmu_core_data *pcore = bcmpmu->coreinfo;
	bcmpmu->read_dev_bulk(bcmpmu, bcmpmu->regmap[PMU_REG_ENV1].map,
		bcmpmu->regmap[PMU_REG_ENV1].addr, pcore->env_regs, pcore->env_size);
}

static bool bcmpmu_is_env_bit_set(struct bcmpmu *bcmpmu, enum bcmpmu_env_bit_t env_bit)
{
	struct bcmpmu_core_data *pcore = bcmpmu->coreinfo;
	int index;
	if (pcore->envregmap[env_bit].regmap.mask == 0) return false;
	index = pcore->envregmap[env_bit].regmap.addr - bcmpmu->regmap[PMU_REG_ENV1].addr;
	if (pcore->env_regs[index] & pcore->envregmap[index].regmap.mask)
		return true;
	else
		return false;
}

static bool bcmpmu_get_env_bit_status(struct bcmpmu *bcmpmu, enum bcmpmu_env_bit_t env_bit)
{
	struct bcmpmu_core_data *pcore = bcmpmu->coreinfo;
	unsigned int val;
	if (pcore->envregmap[env_bit].regmap.mask == 0) return false;
	bcmpmu->read_dev_drct(bcmpmu, pcore->envregmap[env_bit].regmap.map,
		pcore->envregmap[env_bit].regmap.addr, &val, pcore->envregmap[env_bit].regmap.mask);
	if (val != 0)
		return true;
	else
		return false;
}

static int __devinit bcmpmu_probe(struct platform_device *pdev)
{
	int ret;

	int i;
	unsigned int val;
	struct bcmpmu_core_data *pcore;
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_platform_data *pdata = bcmpmu->pdata;
	int *envregs;
	unsigned int regaddr, rval;

	pcore = kzalloc(sizeof(struct bcmpmu_core_data), GFP_KERNEL);
	if (pcore == NULL) {
		ret = -ENOMEM;
		dev_err(bcmpmu->dev, "%s: kzalloc failed: %d\n", __func__, ret);
		return ret;
	}
	pcore->envregmap = bcmpmu_get_envregmap(&pcore->env_size);
	envregs = kzalloc((pcore->env_size * sizeof(int)), GFP_KERNEL);
	if (envregs == NULL) {
		ret = -ENOMEM;
		dev_err(bcmpmu->dev, "%s: kzalloc failed: %d\n", __func__, ret);
		return ret;
	}
	pcore->env_regs = envregs;
	bcmpmu->regmap = bcmpmu_get_regmap();
	bcmpmu->update_env_status = bcmpmu_update_env_status;
	bcmpmu->get_env_bit_status = bcmpmu_get_env_bit_status;
	bcmpmu->is_env_bit_set = bcmpmu_is_env_bit_set;
	bcmpmu->coreinfo = pcore;
	
	printk(KERN_INFO "%s: called.\n", __func__);

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_PMUID, &val, 0xffffffff);
	if (ret < 0) {
		dev_err(bcmpmu->dev, "Failed to read ID: %d\n", ret);
		goto err;
	}
	else
	{
		printk(KERN_INFO "%s: Chip Version = 0x%0X.\n", __func__, val);
	}


	bcmpmu_register_init(bcmpmu);

#ifdef CONFIG_MFD_BCMSAMOA
	/* Remove this later - START */
	regaddr = KONA_SIMI_VA + 0x6C;
	/* Read before write */
	rval = __raw_readl(regaddr);
	printk("bcmpmu_apb_write_device: read value = 0x%X\n", rval);
	rval = rval | 0x1;
		
	printk("bcmpmu_apb_write_device: write value = 0x%X\n", rval);
	
	/* write */
	__raw_writel(rval, regaddr);
	
	regaddr = KONA_SIMI2_VA + 0x6C;
	/* Read before write */
	rval = __raw_readl(regaddr);
	printk("bcmpmu_apb_write_device: read value = 0x%X\n", rval);
	rval = rval | 0x1;
		
	printk("bcmpmu_apb_write_device: write value = 0x%X\n", rval);
	
	/* write */
	__raw_writel(rval, regaddr);
	/* Remove this later - END */
	
#endif
	misc_register(&bcmpmu_device);

#ifdef CONFIG_DEBUG_FS
	struct dentry *root_dir = debugfs_create_dir("bcmpmu", 0);
	if (!root_dir) {
		bcmpmu->debugfs_root_dir = NULL;
		printk(KERN_INFO "%s: failed to create debugfs dir.\n", __func__);
	} else
		bcmpmu->debugfs_root_dir = root_dir;
#endif
	/* Make device data accessible */
	bcmpmu_core = bcmpmu;
	
	for (i = 0; i <ARRAY_SIZE(bcmpmu_fellow_devices); i++)
		bcmpmu_fellow_devices[i]->dev.platform_data = bcmpmu;

	platform_add_devices(bcmpmu_fellow_devices, ARRAY_SIZE(bcmpmu_fellow_devices));
	
	/* Init other platform devices under framework */
	if (pdata && pdata->init) {
		ret = pdata->init(bcmpmu);
		if (ret != 0) {
			dev_err(bcmpmu->dev, "Platform init() failed: %d\n",
				ret);
			goto err;
		}
	}

#ifdef CONFIG_MFD_BCMPMU_DBG
	sysfs_create_group(&pdev->dev.kobj, &bcmpmu_core_attr_group);
#endif
	return 0;

err:
	return ret;
}

static int __devexit bcmpmu_remove(struct platform_device *pdev)
{
	int i;
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_platform_data *pdata = bcmpmu->pdata;

	misc_deregister(&bcmpmu_device);

	if (pdata && pdata->exit) pdata->exit(bcmpmu);
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
