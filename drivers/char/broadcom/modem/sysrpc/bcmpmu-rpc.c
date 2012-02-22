/*****************************************************************************
*  Copyright 2001 - 2011 Broadcom Corporation.  All rights reserved.
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
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/workqueue.h>

#include <linux/mfd/bcmpmu.h>
#include <linux/broadcom/ipcproperties.h>

#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"
#include "rpc_global.h"
#include "rpc_ipc.h"
#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"

#define BCMPMU_PRINT_ERROR (1U << 0)
#define BCMPMU_PRINT_INIT (1U << 1)
#define BCMPMU_PRINT_FLOW (1U << 2)
#define BCMPMU_PRINT_DATA (1U << 3)

static int debug_mask = BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT;
#define pr_rpc(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)

struct bcmpmu_rpc {
	struct bcmpmu *bcmpmu;
	struct delayed_work work;
	unsigned long time;
	int rate;
};

#ifdef CONFIG_MFD_BCMPMU_DBG
static ssize_t
dbgmsk_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "debug_mask is %x\n", debug_mask);
}

static ssize_t
dbgmsk_set(struct device *dev, struct device_attribute *attr,
	   const char *buf, size_t count)
{
	unsigned long val = simple_strtoul(buf, NULL, 0);
	if (val > 0xFF || val == 0)
		return -EINVAL;
	debug_mask = val;
	return count;
}

static ssize_t
rate_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_rpc *prpc = bcmpmu->rpcinfo;
	return sprintf(buf, "rate is %x\n", prpc->rate);
}

static ssize_t
rate_set(struct device *dev, struct device_attribute *attr,
	 const char *buf, size_t count)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_rpc *prpc = bcmpmu->rpcinfo;
	unsigned long val = simple_strtoul(buf, NULL, 0);
	if (val > 0xFF || val == 0)
		return -EINVAL;
	prpc->rate = val;
	return count;
}

static DEVICE_ATTR(dbgmsk, 0644, dbgmsk_show, dbgmsk_set);
static DEVICE_ATTR(rate, 0644, rate_show, rate_set);
static struct attribute *bcmpmu_rpc_attrs[] = {
	&dev_attr_dbgmsk.attr,
	&dev_attr_rate.attr,
	NULL
};

static const struct attribute_group bcmpmu_rpc_attr_group = {
	.attrs = bcmpmu_rpc_attrs,
};
#endif

static void rpc_work(struct work_struct *work)
{
	struct bcmpmu_rpc *prpc =
	    container_of(work, struct bcmpmu_rpc, work.work);
	struct bcmpmu *bcmpmu = prpc->bcmpmu;
	struct bcmpmu_adc_req req;
	unsigned int volt = 0;
	unsigned int t_pa = 0;
	unsigned int t_32k = 0;
	unsigned int result = 0;

	pr_rpc(FLOW, "%s, called\n", __func__);
	req.flags = PMU_ADC_RAW_ONLY;
	if (bcmpmu->adc_req) {
		req.sig = PMU_ADC_PATEMP;
		req.tm = PMU_ADC_TM_HK;
		bcmpmu->adc_req(bcmpmu, &req);
		t_pa = req.raw;
		req.sig = PMU_ADC_VMBATT;
		req.tm = PMU_ADC_TM_HK;
		bcmpmu->adc_req(bcmpmu, &req);
		volt = req.raw;
		req.sig = PMU_ADC_32KTEMP;
		req.tm = PMU_ADC_TM_HK;
		bcmpmu->adc_req(bcmpmu, &req);
		t_32k = req.raw;
	} else
		pr_rpc(ERROR, "%s, bcmpmu adc driver not ready\n", __func__);

	result = ((volt & 0x3FF) << 20) |
	    ((t_pa & 0x3FF) << 10) | (t_32k & 0x3FF);
	RPC_SetProperty(RPC_PROP_ADC_MEASUREMENT, result);
	pr_rpc(DATA,
	       "%s, ADC readings result = 0x%X, volt=0x%X, t_pa=0x%X, t_32k=0x%X\n",
	       __func__, result, volt, t_pa, t_32k);

	schedule_delayed_work(&prpc->work, msecs_to_jiffies(prpc->rate * 1000));
	prpc->time = get_seconds();
}

static int bcmpmu_rpc_resume(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_rpc *prpc = bcmpmu->rpcinfo;
	unsigned long time;
	time = get_seconds();

	if ((time - prpc->time) > prpc->rate) {
		cancel_delayed_work_sync(&prpc->work);
		schedule_delayed_work(&prpc->work, 0);
	}
	return 0;
}

static int __devinit bcmpmu_rpc_probe(struct platform_device *pdev)
{
	int ret;
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_rpc *prpc;
	struct bcmpmu_platform_data *pdata = bcmpmu->pdata;

	pr_rpc(INIT, "%s, called\n", __func__);

	prpc = kzalloc(sizeof(struct bcmpmu_rpc), GFP_KERNEL);
	if (prpc == NULL) {
		pr_rpc(INIT, "%s: failed to alloc mem.\n", __func__);
		ret = -ENOMEM;
		goto err;
	}

	prpc->bcmpmu = bcmpmu;
	bcmpmu->rpcinfo = prpc;
	if (pdata->rpc_rate)
		prpc->rate = pdata->rpc_rate;
	else
		/* This value in sec shall be defined in pmu board file
		   as part of platform data, we keep a default value here
		   to ensure ADC data available to CP for the new platforms
		   where the data not yet finalized */
		prpc->rate = 10;

	INIT_DELAYED_WORK(&prpc->work, rpc_work);

#ifdef CONFIG_MFD_BCMPMU_DBG
	ret = sysfs_create_group(&pdev->dev.kobj, &bcmpmu_rpc_attr_group);
#endif
	schedule_delayed_work(&prpc->work, msecs_to_jiffies(prpc->rate * 1000));
	return 0;

      err:
	return ret;
}

static int __devexit bcmpmu_rpc_remove(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_rpc *prpc = bcmpmu->rpcinfo;

	cancel_delayed_work_sync(&prpc->work);

#ifdef CONFIG_MFD_BCMPMU_DBG
	sysfs_remove_group(&pdev->dev.kobj, &bcmpmu_rpc_attr_group);
#endif
	kfree(prpc);
	return 0;
}

static struct platform_driver bcmpmu_rpc_driver = {
	.driver = {
		   .name = "bcmpmu_rpc",
		   },
	.probe = bcmpmu_rpc_probe,
	.remove = __devexit_p(bcmpmu_rpc_remove),
	.resume = bcmpmu_rpc_resume,
};

static int __init bcmpmu_rpc_init(void)
{
	return platform_driver_register(&bcmpmu_rpc_driver);
}

module_init(bcmpmu_rpc_init);

static void __exit bcmpmu_rpc_exit(void)
{
	platform_driver_unregister(&bcmpmu_rpc_driver);
}

module_exit(bcmpmu_rpc_exit);

MODULE_DESCRIPTION("BCM PMIC rpc driver");
MODULE_LICENSE("GPL");
