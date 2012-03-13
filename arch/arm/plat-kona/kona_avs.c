/*******************************************************************************
 * Copyright 2010,2011 Broadcom Corporation.  All rights reserved.
 *
 *	@file	arch/arm/plat-kona/kona_avs.c
 *
 * Unless you and Broadcom execute a separate written software license agreement
 * governing use of this software, this software is licensed to you under the
 * terms of the GNU General Public License version 2, available at
 * http://www.gnu.org/copyleft/gpl.html (the "GPL").
 *
 * Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 *****************************************************************************/

#include <linux/kernel.h>
#include <linux/err.h>
#ifdef CONFIG_KONA_OTP
#include <plat/bcm_otp.h>
#endif
#include <plat/kona_avs.h>
#include <linux/platform_device.h>
#include <linux/io.h>

/*#define KONA_AVS_DEBUG*/

/*Should we move this to avs_param ?? */
#define MONITOR_VAL_MASK       0xFF
#define MONITOR_VAL0_SHIFT     8
#define MONITOR_VAL1_SHIFT     16
#define MONITOR_VAL2_SHIFT     24
#define MONITOR_VAL3_SHIFT     0

struct avs_info {
	u32 monitor_val0;
	u32 monitor_val1;
	u32 monitor_val2;
	u32 monitor_val3;

	u32 silicon_type;
	u32 svt_silicon_type;
	u32 lvt_silicon_type;

	struct kona_avs_pdata *pdata;
};

struct avs_info avs_info = {.silicon_type = SILICON_TYPE_SLOW, };

module_param_named(silicon_type, avs_info.silicon_type, int, S_IRUGO);
module_param_named(svt_silicon_type, avs_info.svt_silicon_type, int, S_IRUGO);
module_param_named(lvt_silicon_type, avs_info.lvt_silicon_type, int, S_IRUGO);

module_param_named(avs_mon_val0, avs_info.monitor_val0, int, S_IRUGO);
module_param_named(avs_mon_val1, avs_info.monitor_val1, int, S_IRUGO);
module_param_named(avs_mon_val2, avs_info.monitor_val2, int, S_IRUGO);
module_param_named(avs_mon_val3, avs_info.monitor_val3, int, S_IRUGO);

struct mon_val {

	u32 val0;
	u32 val1;
};

#if defined(KONA_AVS_DEBUG)

static int otp_read(int row, struct mon_val *mon_val)
{
	pr_info("%s:row = %d\n", __func__, row);
	if (row < 0)
		return -EINVAL;
	mon_val->val0 =
	    (146 << MONITOR_VAL0_SHIFT) | (180 << MONITOR_VAL1_SHIFT) |
			(95 << MONITOR_VAL2_SHIFT);
	mon_val->val1 = 170;

	return 0;
}
#endif

u32 kona_avs_get_solicon_type(void)
{
	BUG_ON(avs_info.pdata == NULL);
	return avs_info.silicon_type;
}
EXPORT_SYMBOL(kona_avs_get_solicon_type);

int kona_avs_get_mon_val(struct avs_info *avs_inf_ptr)
{
	struct mon_val mon_val;
	int ret = -EINVAL;

	if (avs_inf_ptr->pdata->flags & AVS_TYPE_BOOT) {
		pr_info("%s:AVS_TYPE_BOOT not supported !!!\n", __func__);
		return -EINVAL;
	}

	if (avs_inf_ptr->pdata->flags & AVS_READ_FROM_MEM) {
		void __iomem *mem_ptr;
		pr_info("%s: AVS_READ_FROM_MEM => mem adr = %x\n", __func__,
			avs_inf_ptr->pdata->param);
		BUG_ON(avs_inf_ptr->pdata->param == 0);
		mem_ptr =
		    ioremap_nocache(avs_inf_ptr->pdata->param,
				    sizeof(struct mon_val));
		pr_info("%s: AVS_READ_FROM_MEM => mem virtual addr = %p\n",
			__func__, mem_ptr);
		if (mem_ptr) {
			memcpy(&mon_val, mem_ptr, sizeof(struct mon_val));
			iounmap(mem_ptr);
			ret = 0;
		} else {
			ret = -ENOMEM;
			BUG_ON(mem_ptr == NULL);
		}
	} else {
		pr_info("%s: AVS_READ_FROM_OTP => OTP row = %x\n", __func__,
			avs_inf_ptr->pdata->param);
#if defined(KONA_AVS_DEBUG) || defined(CONFIG_KONA_OTP)
		ret = otp_read(avs_inf_ptr->pdata->param, &mon_val);
#endif
	}

	if (!ret) {

		pr_info("%s:opt:val0 = %x val1 = %x\n", __func__, mon_val.val0,
			mon_val.val1);
		avs_inf_ptr->monitor_val0 =
		    (mon_val.val0 >> MONITOR_VAL0_SHIFT) & MONITOR_VAL_MASK;
		avs_inf_ptr->monitor_val1 =
		    (mon_val.val0 >> MONITOR_VAL1_SHIFT) & MONITOR_VAL_MASK;
		avs_inf_ptr->monitor_val2 =
		    (mon_val.val0 >> MONITOR_VAL2_SHIFT) & MONITOR_VAL_MASK;
		avs_inf_ptr->monitor_val3 =
		    (mon_val.val1 >> MONITOR_VAL3_SHIFT) & MONITOR_VAL_MASK;

		pr_info("%s:monitor_val0 = %d monitor_val1= %d monitor_val2 = %d"
			"monitor_val3 = %d\n", __func__,
			avs_inf_ptr->monitor_val0, avs_inf_ptr->monitor_val1,
			avs_inf_ptr->monitor_val2,
			avs_inf_ptr->monitor_val3);
	}
	return ret;
}

static u32 kona_avs_get_svt_type(struct avs_info *avs_inf_ptr)
{
	int i;

	int svt_pmos_inx = -1;
	int svt_nmos_inx = -1;

	struct kona_avs_pdata *pdata = avs_inf_ptr->pdata;

	for (i = 0; i < pdata->pmos_bin_size; i++) {
		if (avs_inf_ptr->monitor_val3 >= pdata->svt_pmos_bin[i] &&
		    avs_inf_ptr->monitor_val3 < pdata->svt_pmos_bin[i + 1]) {
			svt_pmos_inx = i;
			break;
		}
	}

	for (i = 0; i < pdata->nmos_bin_size; i++) {
		if (avs_inf_ptr->monitor_val2 >= pdata->svt_nmos_bin[i] &&
		    avs_inf_ptr->monitor_val2 < pdata->svt_nmos_bin[i + 1]) {
			svt_nmos_inx = i;
			break;
		}
	}

	pr_info("%s:svt_pmos = %d svt_nmos = %d\n",
		__func__, svt_pmos_inx, svt_nmos_inx);

	if (svt_nmos_inx == -1 || svt_pmos_inx == -1)
		return SILICON_TYPE_SLOW;

	return pdata->svt_silicon_type_lut[svt_pmos_inx * pdata->nmos_bin_size +
					   svt_nmos_inx];
}

static u32 kona_avs_get_lvt_type(struct avs_info *avs_inf_ptr)
{
	int i;

	int lvt_pmos_inx = -1;
	int lvt_nmos_inx = -1;

	struct kona_avs_pdata *pdata = avs_inf_ptr->pdata;

	for (i = 0; i < pdata->pmos_bin_size; i++) {
		if (avs_inf_ptr->monitor_val1 >= pdata->lvt_pmos_bin[i] &&
		    avs_inf_ptr->monitor_val1 < pdata->lvt_pmos_bin[i + 1]) {
			lvt_pmos_inx = i;
			break;
		}
	}

	for (i = 0; i < pdata->nmos_bin_size; i++) {
		if (avs_inf_ptr->monitor_val0 >= pdata->lvt_nmos_bin[i] &&
		    avs_inf_ptr->monitor_val0 < pdata->lvt_nmos_bin[i + 1]) {
			lvt_nmos_inx = i;
			break;
		}
	}
	pr_info("%s:lvt_pmos = %d lvt_nmos = %d\n",
		__func__, lvt_pmos_inx, lvt_nmos_inx);

	if (lvt_nmos_inx == -1 || lvt_pmos_inx == -1)
		return SILICON_TYPE_SLOW;

	return pdata->lvt_silicon_type_lut[lvt_pmos_inx * pdata->nmos_bin_size +
					   lvt_nmos_inx];
}

static int kona_avs_drv_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct kona_avs_pdata *pdata = pdev->dev.platform_data;

	pr_info("%s\n", __func__);

	if (!pdata) {
		pr_info("%s : invalid paltform data !!\n", __func__);
		ret = -EPERM;
		goto error;
	}

	avs_info.pdata = pdata;

	BUG_ON((pdata->flags & AVS_TYPE_OPEN)
	       && (pdata->flags & AVS_TYPE_BOOT));
	BUG_ON((pdata->flags & AVS_READ_FROM_OTP)
	       && (pdata->flags & AVS_READ_FROM_MEM));

	ret = kona_avs_get_mon_val(&avs_info);
	if (ret)
		goto error;

	avs_info.svt_silicon_type = kona_avs_get_svt_type(&avs_info);
	avs_info.lvt_silicon_type = kona_avs_get_lvt_type(&avs_info);

	avs_info.silicon_type =
	    min(avs_info.lvt_silicon_type, avs_info.svt_silicon_type);

	if (pdata->silicon_type_notify)
		pdata->silicon_type_notify(avs_info.silicon_type);
	pr_info("%s:svt type: %d lvt type: %d  silicon type: %d\n", __func__,
		avs_info.svt_silicon_type, avs_info.lvt_silicon_type,
		avs_info.silicon_type);
error:
	return ret;
}

static int __devexit kona_avs_drv_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver kona_avs_driver = {
	.probe = kona_avs_drv_probe,
	.remove = __devexit_p(kona_avs_drv_remove),
	.driver = {.name = "kona-avs",},
};

static int __init kona_avs_drv_init(void)
{
	return platform_driver_register(&kona_avs_driver);
}

subsys_initcall_sync(kona_avs_drv_init);

static void __exit kona_avs_drv_exit(void)
{
	platform_driver_unregister(&kona_avs_driver);
}

module_exit(kona_avs_drv_exit);

MODULE_ALIAS("platform:kona_avs_drv");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("AVS driver for BRCM Kona based Chipsets");
