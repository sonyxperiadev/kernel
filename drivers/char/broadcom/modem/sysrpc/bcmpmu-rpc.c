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
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/bug.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>
#ifdef CONFIG_WD_TAPPER
#include <linux/broadcom/wd-tapper.h>
#else
#include <linux/alarmtimer.h>
#endif

#include "mobcom_types.h"
#include "rpc_global.h"
#include "resultcode.h"
#include "taskmsgs.h"
#include "ipcinterface.h"
#include "ipcproperties.h"
#include "rpc_ipc.h"
#include "rpc_ipc.h"
#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"

#define ADC_RAW_MASK		0x3FF
#define VMMBAT_ADC_SHIFT	20
#define TEMPPA_ADC_SHIFT	10
#define TEMP32K_ADC_SHIFT	0
#define ADC_RETRY		5

static int debug_mask =  BCMPMU_PRINT_INIT | BCMPMU_PRINT_ERROR |
				BCMPMU_PRINT_FLOW;

#define pr_rpc(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)


struct bcmpmu_rpc {
	struct bcmpmu59xxx *bcmpmu;
	struct delayed_work work;
	u32 delay;
	u32 fw_delay;
	u32 fw_cnt;
	u32 vbat;
	u32 temp_pa;
	u32 temp_32k;
	u32 adc_mux;
	int poll_time;
	int htem_poll_time;
	int mod_tem;
	int htem;
	int tapper_time;
#ifdef CONFIG_WD_TAPPER
	struct wd_tapper_node wd_node;
#else
	struct alarm alarm;
	int alarm_timeout;
#endif
#ifdef CONFIG_DEBUG_FS
	struct dentry *dent_rpc;
#endif
};

#ifndef CONFIG_WD_TAPPER
static void bcmpmu_rpc_program_alarm(struct bcmpmu_rpc *rpc,
		long seconds)
{
	ktime_t interval = ktime_set(seconds, 0);
	ktime_t next;

	pr_rpc(VERBOSE, "set timeout %ld s.\n", seconds);
	next = ktime_add(ktime_get_real(), interval);

	alarm_start(&rpc->alarm, next);
}

static enum alarmtimer_restart bcmpmu_rpc_alarm_callback(
		struct alarm *alarm, ktime_t now)
{
	struct bcmpmu_rpc *bcmpmu_rpc =
		 container_of(alarm, struct bcmpmu_rpc, alarm);
	pr_rpc(VERBOSE, "rpc Alarm cb\n");
	schedule_delayed_work(&bcmpmu_rpc->work, 0);
	return ALARMTIMER_NORESTART;
}
#endif

static int bcmpmu_rpc_get_val(struct bcmpmu59xxx *bcmpmu,
				enum bcmpmu_adc_channel channel,
				enum bcmpmu_adc_req req,
				struct bcmpmu_adc_result *result)
{
	int ret;
	int retry = ADC_RETRY;
	while (retry) {
		ret = bcmpmu_adc_read(bcmpmu, channel, req, result);
		if (!ret)
			break;
		retry--;
	}
	return ret;
}
static void bcmpmu_rpc_work(struct work_struct *work)
{
	int ret;
	struct bcmpmu59xxx *bcmpmu;
	struct bcmpmu_adc_result result;
	int tem, poll_time;
	bool config_tapper = false;
	ulong delay;
	static int rpc_fw_cnt;
	struct bcmpmu_rpc *bcmpmu_rpc =
	    container_of(work, struct bcmpmu_rpc, work.work);
	BUG_ON(!bcmpmu_rpc);
	bcmpmu = bcmpmu_rpc->bcmpmu;

	pr_rpc(VERBOSE, "%s, called\n", __func__);

	ret = bcmpmu_rpc_get_val(bcmpmu, PMU_ADC_CHANN_VMBATT,
				PMU_ADC_REQ_SAR_MODE, &result);
	if (ret) {
		pr_rpc(ERROR, "%s: PMU_ADC_CHANN_VMBATT read error\n",
						 __func__);
		goto err;
	}
	bcmpmu_rpc->vbat = result.raw & ADC_RAW_MASK;

	ret = bcmpmu_rpc_get_val(bcmpmu, PMU_ADC_CHANN_PATEMP,
				PMU_ADC_REQ_SAR_MODE, &result);
	if (ret) {
		pr_rpc(ERROR, "%s: PMU_ADC_CHANN_PATEMP read error\n",
								 __func__);
		goto err;
	}
	bcmpmu_rpc->temp_pa = result.raw & ADC_RAW_MASK;
	tem = result.conv;

	pr_rpc(FLOW, "%s, pa_temp %d\n", __func__, result.conv);

	ret = bcmpmu_rpc_get_val(bcmpmu, PMU_ADC_CHANN_32KTEMP,
				PMU_ADC_REQ_SAR_MODE, &result);
	if (ret) {
		pr_rpc(ERROR, "%s: PMU_ADC_CHANN_32KTEMP read error\n",
								 __func__);
		goto err;
	}
	bcmpmu_rpc->temp_32k = result.raw & ADC_RAW_MASK;

	bcmpmu_rpc->adc_mux = (bcmpmu_rpc->vbat << VMMBAT_ADC_SHIFT) |
				(bcmpmu_rpc->temp_pa << TEMPPA_ADC_SHIFT) |
				(bcmpmu_rpc->temp_32k << TEMP32K_ADC_SHIFT);

	RPC_SetProperty(RPC_PROP_ADC_MEASUREMENT,  bcmpmu_rpc->adc_mux);
	pr_rpc(DATA,
	       "%s, ADC readings result = 0x%x\n",
	       __func__, bcmpmu_rpc->adc_mux);


	if (tem >= bcmpmu_rpc->mod_tem && tem < bcmpmu_rpc->htem) {
		poll_time = bcmpmu_rpc->poll_time;
		config_tapper = true;
	} else if (tem >= bcmpmu_rpc->htem) {
		poll_time = bcmpmu_rpc->htem_poll_time;
		config_tapper = true;
	}
#ifdef CONFIG_WD_TAPPER
	if (config_tapper &&
			(bcmpmu_rpc->tapper_time != poll_time)) {
		bcmpmu_rpc->tapper_time = poll_time;
		pr_rpc(FLOW, "==%s== wd tapper timeout to %d\n", __func__,
				(poll_time / 1000));
		ret = wd_tapper_update_timeout_req(&bcmpmu_rpc->wd_node,
				(poll_time / 1000));
		BUG_ON(ret);
	}
#else
	if (config_tapper) {
		bcmpmu_rpc->alarm_timeout = poll_time / 1000;
		bcmpmu_rpc_program_alarm(bcmpmu_rpc,
			bcmpmu_rpc->alarm_timeout);
	}
#endif /*CONFIG_WD_TAPPER*/


err:
	if (rpc_fw_cnt < bcmpmu_rpc->fw_cnt) {
		delay =  msecs_to_jiffies(bcmpmu_rpc->fw_delay);
		rpc_fw_cnt++;
		pr_rpc(VERBOSE, "%s, rpc_fw_cnt %d\n", __func__, rpc_fw_cnt);
	} else
		delay =  msecs_to_jiffies(bcmpmu_rpc->delay);

	schedule_delayed_work(&bcmpmu_rpc->work, delay);

}

#ifdef CONFIG_PM

static int bcmpmu_rpc_resume(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu_rpc *bcmpmu_rpc;
	bcmpmu_rpc = bcmpmu->rpcinfo;
	schedule_delayed_work(&bcmpmu_rpc->work, 0);
	return 0;
}

#else

#define bcmpmu_rpc_resume	NULL
#define bcmpmu_rpc_suspend	NULL

#endif

#ifdef CONFIG_DEBUG_FS
static void bcmpmu_rpc_dbg_init(struct bcmpmu_rpc *bcmpmu_rpc)
{
	struct bcmpmu59xxx *bcmpmu = bcmpmu_rpc->bcmpmu;

	if (!bcmpmu->dent_bcmpmu) {
		pr_err("%s: Failed to initialize debugfs\n", __func__);
		return;
	}

	bcmpmu_rpc->dent_rpc = debugfs_create_dir("rpc", bcmpmu->dent_bcmpmu);
	if (!bcmpmu_rpc->dent_rpc)
		return;

	if (!debugfs_create_u32("dbg_mask", S_IWUSR | S_IRUSR,
				bcmpmu_rpc->dent_rpc, &debug_mask))
		goto err;

	if (!debugfs_create_u32("vbat", S_IRUSR,
				bcmpmu_rpc->dent_rpc, &bcmpmu_rpc->vbat))
		goto err;
	if (!debugfs_create_u32("temp_pa", S_IRUSR,
				bcmpmu_rpc->dent_rpc, &bcmpmu_rpc->temp_pa))
		goto err;
	if (!debugfs_create_u32("temp_32k", S_IRUSR,
				bcmpmu_rpc->dent_rpc, &bcmpmu_rpc->temp_32k))
		goto err;
	if (!debugfs_create_u32("adc_mux", S_IRUSR,
				bcmpmu_rpc->dent_rpc, &bcmpmu_rpc->adc_mux))
		goto err;
	if (!debugfs_create_u32("delay", S_IWUSR | S_IRUSR,
				bcmpmu_rpc->dent_rpc, &bcmpmu_rpc->delay))
		goto err;
	return;
err:
	debugfs_remove(bcmpmu_rpc->dent_rpc);
}
#endif

static int bcmpmu_rpc_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu59xxx_rpc_pdata *pdata;
	struct bcmpmu_rpc *bcmpmu_rpc;

	pdata = (struct  bcmpmu59xxx_rpc_pdata *)
		pdev->dev.platform_data;
	BUG_ON(pdata == NULL);

	pr_rpc(INIT, "%s, called\n", __func__);

	bcmpmu_rpc = kzalloc(sizeof(struct bcmpmu_rpc), GFP_KERNEL);
	if (bcmpmu_rpc == NULL) {
		pr_rpc(INIT, "%s: failed to alloc mem.\n", __func__);
		ret = -ENOMEM;
		goto err;
	}
	bcmpmu_rpc->bcmpmu = bcmpmu;
	bcmpmu_rpc->delay = pdata->delay;
	bcmpmu_rpc->fw_delay = pdata->fw_delay;
	bcmpmu_rpc->fw_cnt = pdata->fw_cnt;
	bcmpmu_rpc->poll_time = pdata->poll_time;
	bcmpmu_rpc->htem_poll_time = pdata->htem_poll_time;
	bcmpmu_rpc->mod_tem = pdata->mod_tem;
	bcmpmu_rpc->htem = pdata->htem;
	bcmpmu->rpcinfo = bcmpmu_rpc;
#ifdef CONFIG_WD_TAPPER
	ret = wd_tapper_add_timeout_req(&bcmpmu_rpc->wd_node, "rpc",
				TAPPER_DEFAULT_TIMEOUT);
	if (ret) {
		pr_rpc(ERROR, "failed to register with wd-tapper\n");
		goto err1;
	}
#else
	alarm_init(&bcmpmu_rpc->alarm,
		ALARM_REALTIME, bcmpmu_rpc_alarm_callback);
#endif /*CONFIG_WD_TAPPER*/

	INIT_DELAYED_WORK(&bcmpmu_rpc->work, bcmpmu_rpc_work);

#ifdef CONFIG_DEBUG_FS
	bcmpmu_rpc_dbg_init(bcmpmu_rpc);
#endif
	schedule_delayed_work(&bcmpmu_rpc->work,
			msecs_to_jiffies(bcmpmu_rpc->fw_delay));
err:
	return ret;
#ifdef CONFIG_WD_TAPPER
err1:
	kfree(bcmpmu_rpc);
#endif /*CONFIG_WD_TAPPER*/
	return ret;
}

static int bcmpmu_rpc_remove(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu_rpc *bcmpmu_rpc = bcmpmu->rpcinfo;

#ifdef CONFIG_DEBUG_FS
	if (bcmpmu_rpc && bcmpmu_rpc->dent_rpc)
		debugfs_remove(bcmpmu_rpc->dent_rpc);
#endif
#ifdef CONFIG_WD_TAPPER
	wd_tapper_del_timeout_req(&bcmpmu_rpc->wd_node);
#else
	alarm_cancel(&bcmpmu_rpc->alarm);
#endif
	kfree(bcmpmu_rpc);
	return 0;
}

static struct platform_driver bcmpmu_rpc_driver = {
	.driver = {
		   .name = "bcmpmu_rpc",
		   },
	.probe = bcmpmu_rpc_probe,
	.remove = bcmpmu_rpc_remove,
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

