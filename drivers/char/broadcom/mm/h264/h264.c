/*******************************************************************************
  Copyright 2010 Broadcom Corporation.  All rights reserved.

  Unless you and Broadcom execute a separate written software license agreement
  governing use of this software, this software is licensed to you under the
  terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#define pr_fmt(fmt) "h264: " fmt

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/broadcom/mm_fw_hw_ifc.h>
#include <mach/clock.h>
#include "h264.h"

struct h264_device_t {
	void	*fmwk_handle;
	int	(*subdev_init[H264_SUBDEV_COUNT])(MM_CORE_HW_IFC * core_param);
	void	(*subdev_deinit[H264_SUBDEV_COUNT])(void);
};

struct h264_device_t *h264_device;

int __init mm_h264_init(void)
{
	int ret = 0;
	MM_CORE_HW_IFC *core_param;
	MM_DVFS_HW_IFC dvfs_param;
	MM_PROF_HW_IFC prof_param;
	int i = 0;

	pr_debug("h264_init: -->\n");
	h264_device = kzalloc(sizeof(struct h264_device_t), GFP_KERNEL);
	if (h264_device == NULL) {
		pr_err("h264_init: kzalloc failed\n");
		ret = -ENOMEM;
		goto err;
	}
	core_param = kzalloc(sizeof(MM_CORE_HW_IFC) * H264_SUBDEV_COUNT,
			GFP_KERNEL);
	if (core_param == NULL) {
		pr_err("h264_init: kzalloc failed\n");
		kfree(h264_device);
		ret = -ENOMEM;
		goto err;
	}

	h264_device->subdev_init[0] = &cme_init;
	h264_device->subdev_deinit[0] = &cme_deinit;

	h264_device->subdev_init[1] = &mcin_init;
	h264_device->subdev_deinit[1] = &mcin_deinit;

	h264_device->subdev_init[2] = &cabac_init;
	h264_device->subdev_deinit[2] = &cabac_deinit;

	h264_device->subdev_init[3] = &h264_vce_init;
	h264_device->subdev_deinit[3] = &h264_vce_deinit;

	core_param[0].core_name = "CME";
	core_param[1].core_name = "MCIN";
	core_param[2].core_name = "CABAC";
	core_param[3].core_name = "H264_VCE";
#if defined(CONFIG_MM_SECURE_DRIVER)

	h264_device->subdev_init[9] = &mcin_secure_init;
	h264_device->subdev_deinit[9] = &mcin_secure_deinit;

	h264_device->subdev_init[10] = &cabac_secure_init;
	h264_device->subdev_deinit[10] = &cabac_secure_deinit;

	h264_device->subdev_init[11] = &h264_vce_secure_init;
	h264_device->subdev_deinit[11] = &h264_vce_secure_deinit;

	h264_device->subdev_init[12] = &h264_ol_secure_init;
	h264_device->subdev_deinit[12] = &h264_ol_secure_deinit;

	core_param[9].core_name = "MCIN_SECURE";
	core_param[10].core_name = "CABAC_SECURE";
	core_param[11].core_name = "H264_VCE_SECURE";
	core_param[12].core_name = "H264_OL_SECURE";

#endif /* CONFIG_MM_SECURE_DRIVER */

	/*Calling init on sub devices*/
	for (i = 0; i < H264_SUBDEV_COUNT; i++) {
		if (h264_device->subdev_init[i]) {
			ret = h264_device->subdev_init[i](&core_param[i]);
			if (ret != 0)
				goto err1;
		}
	}

	/*Initialize params*/
	dvfs_param.__on = 1;
	dvfs_param.__mode = TURBO;
	dvfs_param.__ts = DEFAULT_MM_DEV_DVFS_SAMPLING_MS;
	dvfs_param.eco_ns_high = 2;
	dvfs_param.nor_ns_high = 2;
	dvfs_param.nor_ns_low = 4;
	dvfs_param.tur_ns_high = 2;
	dvfs_param.tur_ns_low = 4;
	dvfs_param.st_ns_low = 3;

#ifdef CONFIG_MM_312M_SOURCE_CLK
	dvfs_param.eco_high = 90;
	dvfs_param.nor_high = 80;
	dvfs_param.nor_low = 54;
	dvfs_param.tur_high = 82;
	dvfs_param.tur_low = 52;
	dvfs_param.st_low = 52;
#else
	dvfs_param.eco_high = 0;
	dvfs_param.nor_high = 80;
	dvfs_param.nor_low = 0;
	dvfs_param.tur_high = 85;
	dvfs_param.tur_low = 50;
	dvfs_param.st_low = 65;
#endif
	dvfs_param.dvfs_bulk_job_cnt = 0;

	h264_device->fmwk_handle = mm_fmwk_register(H264_DEV_NAME,
						H264_AXI_BUS_CLK_NAME_STR,
						H264_SUBDEV_COUNT,
						&core_param[0],
						&dvfs_param,
						&prof_param);
	if ((h264_device->fmwk_handle == NULL)) {
		ret = -ENOMEM;
		goto err1;
	}

	kfree(core_param);
	pr_debug("h264_init: H264 driver Module Init over");
	return ret;
err1:
	pr_err("h264_init: Error subdev init (%d)\n", i);
	kfree(core_param);
	kfree(h264_device);
err:
	return ret;
}

void __exit mm_h264_exit(void)
{
	int i = 0;
	pr_debug("H264_exit:\n");
	if (h264_device->fmwk_handle)
		mm_fmwk_unregister(h264_device->fmwk_handle);
	kfree(h264_device);
	/*Sub device deinit*/
	for (i = 0; i < H264_SUBDEV_COUNT; i++)
		h264_device->subdev_deinit[i]();
}

module_init(mm_h264_init);
module_exit(mm_h264_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("H264 device driver");
MODULE_LICENSE("GPL");
