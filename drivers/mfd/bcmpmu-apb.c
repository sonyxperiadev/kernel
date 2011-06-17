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

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/io.h>

#include <linux/mfd/bcmpmu.h>
#include <mach/io_map.h>

#define REGS_PAGE_SIZE 4096

struct bcmpmu_apb {
	struct bcmpmu *bcmpmu;
	struct mutex apb_mutex;
};

static int bcmpmu_apb_read_device(struct bcmpmu *bcmpmu, int reg, unsigned int *val, unsigned int msk)
{
	u32 regbase, regaddr;
	struct bcmpmu_reg_map map;
	struct bcmpmu_platform_data *pdata;
	struct bcmpmu_apb *acc = (struct bcmpmu_apb *)bcmpmu->accinfo;

	pdata = bcmpmu->pdata;
	regbase = pdata->baseaddr;
	
	/* This is 64 bit? */
	printk("bcmpmu_apb_write_device: regbase = 0x%X\n", regbase);

	if (reg >= REGS_PAGE_SIZE) return -ENODEV;
	map = bcmpmu->regmap[reg];
	if ((map.addr == 0) && (map.mask == 0))  return -ENODEV;

	regaddr = regbase + map.addr;
	
	printk("bcmpmu_apb_write_device: regaddr = 0x%X\n", regaddr);

	mutex_lock(&acc->apb_mutex);
	*val = __raw_readl(regaddr); /* This should be __raw_readsl? */
	mutex_unlock(&acc->apb_mutex);

	return 0;
}

static int bcmpmu_apb_write_device(struct bcmpmu *bcmpmu, int reg, unsigned int value, unsigned int msk)
{
	struct bcmpmu_reg_map map;
	struct bcmpmu_platform_data *pdata;
	struct bcmpmu_apb *acc = (struct bcmpmu_apb *)bcmpmu->accinfo;
	u32 regbase, regaddr;
	unsigned int rval;

	pdata = bcmpmu->pdata;
	regbase = pdata->baseaddr;
	
	/* This is 64 bit? */
	printk("bcmpmu_apb_write_device: regbase = 0x%X\n", regbase);
	
	if (reg >= REGS_PAGE_SIZE) return -ENODEV;
	map = bcmpmu->regmap[reg];
	if ((map.addr == 0) && (map.mask == 0))  return -ENODEV;
	
	regaddr = regbase + map.addr;
	
	printk("bcmpmu_apb_write_device: regaddr = 0x%X\n", regaddr);
	
	mutex_lock(&acc->apb_mutex);
	/* Read before write */
	rval = __raw_readl(regaddr);
	rval = rval & ~msk;
	value = value | rval;
	
	printk("bcmpmu_apb_write_device: value = 0x%X\n", value);
	
	/* write */
	__raw_writel(value, regaddr);
	
	mutex_unlock(&acc->apb_mutex);

	return 0;	
}

static int bcmpmu_apb_read_device_direct(struct bcmpmu *bcmpmu, int map, unsigned int regaddr, unsigned int *val, unsigned int msk)
{
	struct bcmpmu_platform_data *pdata;
	struct bcmpmu_apb *acc = (struct bcmpmu_apb *)bcmpmu->accinfo;
	
	pdata = bcmpmu->pdata;
	regaddr = pdata->baseaddr + regaddr;

	mutex_lock(&acc->apb_mutex);
	*val = __raw_readl(regaddr);
	mutex_unlock(&acc->apb_mutex);

	return 0;
}

static int bcmpmu_apb_write_device_direct(struct bcmpmu *bcmpmu, int map, unsigned int regaddr, unsigned int val, unsigned int msk)
{
	struct bcmpmu_apb *acc = (struct bcmpmu_apb *)bcmpmu->accinfo;
	struct bcmpmu_platform_data *pdata;
	unsigned int rval;

	pdata = bcmpmu->pdata;

	/* This is 64 bit? */
	printk("bcmpmu_apb_write_device: regbase = 0x%X\n", pdata->baseaddr);

	regaddr = pdata->baseaddr + regaddr;

	printk("bcmpmu_apb_write_device: regaddr = 0x%X\n", regaddr);

	/* Read before write */
	mutex_lock(&acc->apb_mutex);
	rval = __raw_readl(regaddr);
	printk("bcmpmu_apb_write_device_direct: read value = 0x%X\n", rval);
	rval = rval & ~msk;
	val = val | rval;
	/* write */
	printk("bcmpmu_apb_write_device_direct: value = 0x%X\n", val);
	__raw_writel(val, regaddr);
	
	mutex_unlock(&acc->apb_mutex);
	
	return 0;
}

static int bcmpmu_apb_read_device_direct_bulk(struct bcmpmu *bcmpmu, int map, unsigned int regaddr, unsigned int *val, int len)
{
	int i;
	struct bcmpmu_platform_data *pdata;
	struct bcmpmu_apb *acc = (struct bcmpmu_apb *)bcmpmu->accinfo;
	
	pdata = bcmpmu->pdata;
	regaddr = pdata->baseaddr + regaddr;

	mutex_lock(&acc->apb_mutex);
	for(i=0; i < len; i++)
		val[i] = __raw_readl(regaddr + (i << 2));
	mutex_unlock(&acc->apb_mutex);

	return 0;
}

static int bcmpmu_apb_write_device_direct_bulk(struct bcmpmu *bcmpmu, int map, unsigned int regaddr, unsigned int *val, int len)
{
	int i;
	struct bcmpmu_platform_data *pdata;
	struct bcmpmu_apb *acc = (struct bcmpmu_apb *)bcmpmu->accinfo;
	
	pdata = bcmpmu->pdata;
	regaddr = pdata->baseaddr + regaddr;

	mutex_lock(&acc->apb_mutex);
	for(i=0; i < len; i++)
		__raw_writel(val[i], regaddr + (i << 2));
	mutex_unlock(&acc->apb_mutex);

	return 0;
}

static struct platform_device bcmpmu_core_device = {
	.name 			= "bcmpmu_core",
	.id			= -1,
	.dev.platform_data 	= NULL,
};

static int bcmpmu_apb_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct bcmpmu *bcmpmu;
	struct bcmpmu_platform_data *pdata; 
	struct bcmpmu_apb *bcmpmu_apb;
		
	printk("REG : bcmpmu_apb_probe called \n"); 
	
	pdata = (struct bcmpmu_platform_data *)pdev->dev.platform_data;

	bcmpmu = kzalloc(sizeof(struct bcmpmu), GFP_KERNEL);
	if (bcmpmu == NULL) {
		printk("bcmpmu_apb_probe: failed to alloc mem.\n");
		return -ENOMEM;
	}

	bcmpmu_apb = kzalloc(sizeof(struct bcmpmu_apb), GFP_KERNEL);
	if (bcmpmu_apb == NULL) {
		printk("bcmpmu_apb_probe: failed to alloc mem.\n");
		return -ENOMEM;
	}

	mutex_init(&bcmpmu_apb->apb_mutex);
	
	bcmpmu->read_dev = bcmpmu_apb_read_device;
	bcmpmu->write_dev = bcmpmu_apb_write_device;
	bcmpmu->read_dev_drct = bcmpmu_apb_read_device_direct;
	bcmpmu->write_dev_drct = bcmpmu_apb_write_device_direct;
	bcmpmu->read_dev_bulk = bcmpmu_apb_read_device_direct_bulk;
	bcmpmu->write_dev_bulk = bcmpmu_apb_write_device_direct_bulk;
	bcmpmu->pdata = pdata;


	bcmpmu->accinfo = bcmpmu_apb;

	bcmpmu_core_device.dev.platform_data = bcmpmu;
	platform_device_register(&bcmpmu_core_device);

	return ret;
}

static int bcmpmu_apb_remove(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;

	platform_device_unregister(&bcmpmu_core_device);
	kfree(bcmpmu);

	return 0;
}

static struct platform_driver bcmpmu_driver = {
	.driver = {
		   .name = "bcmpmu_samoa",
		   .owner = THIS_MODULE,
	},
	.probe = bcmpmu_apb_probe,
	.remove = bcmpmu_apb_remove,
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


MODULE_DESCRIPTION("APB support for BCMPMU PMIC");
MODULE_LICENSE("GPL");
