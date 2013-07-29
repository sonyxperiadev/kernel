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
#include <linux/broadcom/mm_fw_hw_ifc.h>
#include "h264.h"
#include "../common/mm_common.h"

struct h264_device_t {
	void	*fmwk_handle;
	int	(*subdev_init[H264_SUBDEV_COUNT])(MM_CORE_HW_IFC * core_param);
	void	(*subdev_deinit[H264_SUBDEV_COUNT])(void);
};

struct h264_device_t *h264_device;

int __init mm_h264_init(void)
{
	int ret = 0;
	MM_CORE_HW_IFC core_param[H264_SUBDEV_COUNT];
	MM_DVFS_HW_IFC dvfs_param;
	MM_PROF_HW_IFC prof_param;
	int i = 0;

	pr_debug("h264_init: -->\n");
	h264_device = kmalloc(sizeof(struct h264_device_t), GFP_KERNEL);
	if (h264_device == NULL) {
		pr_err("h264_init: kmalloc failed\n");
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

	h264_device->subdev_init[4] = &h264_ol_init;
	h264_device->subdev_deinit[4] = &h264_ol_deinit;

	/*Calling init on sub devices*/
	for (i = 0; i < H264_SUBDEV_COUNT; i++) {
		ret = h264_device->subdev_init[i](&core_param[i]);
		if (ret != 0) {
			pr_err("mm_h264_init: subdev init for " \
				"%d returned error\n", i);
			goto err1;
		}
	}

	/*Initialize generice params*/
	//dvfs_param.is_dvfs_on = 0;
	//dvfs_param.user_requested_mode = TURBO;
	dvfs_param.T1 = 300;
	dvfs_param.P1 = 80;
	dvfs_param.T2 = 3000;
	dvfs_param.P2 = 30;
	dvfs_param.dvfs_bulk_job_cnt = 0;

	h264_device->fmwk_handle = mm_tmp_fmwk_register(H264_DEV_NAME,
						H264_AXI_BUS_CLK_NAME_STR,
						H264_SUBDEV_COUNT,
						&core_param[0],
						&dvfs_param,
						&prof_param);
	if ((h264_device->fmwk_handle == NULL)) {
		ret = -ENOMEM;
		goto err1;
	}

	pr_debug("h264_init: H264 driver Module Init over");
	printk(KERN_INFO "staging H264 driver module init done\n");
	return ret;
err1:
	kfree(h264_device);
err:
	pr_err("h264_init: H264 driver Module Init Error");
	return ret;
}

void __exit mm_h264_exit(void)
{
	int i = 0;
	pr_debug("H264_exit:\n");
	if (h264_device->fmwk_handle)
		mm_tmp_fmwk_unregister(h264_device->fmwk_handle);
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
