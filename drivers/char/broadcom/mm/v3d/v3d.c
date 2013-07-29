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

#define pr_fmt(fmt) "v3d: " fmt

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/broadcom/mm_fw_hw_ifc.h>
#include "v3d.h"
#define v3d_device_t struct _v3d_device_t

struct _v3d_device_t {
	void	*fmwk_handle;
	int	(*subdev_init[V3D_SUBDEV_COUNT])(MM_CORE_HW_IFC * core_param);
	void	(*subdev_deinit[V3D_SUBDEV_COUNT])(void);
};

v3d_device_t *v3d_device;

int __init mm_v3d_init(void)
{
	int ret = 0;
	MM_CORE_HW_IFC core_param[V3D_SUBDEV_COUNT];
	MM_DVFS_HW_IFC dvfs_param;
	MM_PROF_HW_IFC prof_param;
	int i = 0;

	pr_debug("v3d_init: -->\n");
	v3d_device = kmalloc(sizeof(v3d_device_t), GFP_KERNEL);
	if (v3d_device == NULL) {
		pr_err("v3d_init: kmalloc failed\n");
		ret = -ENOMEM;
		goto err;
	}

	v3d_device->subdev_init[0] = &v3d_bin_render_init;
	v3d_device->subdev_deinit[0] = &v3d_bin_render_deinit;

	v3d_device->subdev_init[1] = &v3d_user_init;
	v3d_device->subdev_deinit[1] = &v3d_user_deinit;

	/*Calling init on sub devices*/
	for (i = 0; i < V3D_SUBDEV_COUNT; i++) {
		ret = v3d_device->subdev_init[i](&core_param[i]);
		if (ret != 0) {
			pr_err("mm_v3d_init:" \
				"subdev init for %d returned error\n", i);
			goto err1;
		}
	}

	/*Initialize generice params*/
	dvfs_param.ON = 1;
	dvfs_param.MODE = TURBO;
	dvfs_param.T0 = 0;
	dvfs_param.P0 = 0;
	dvfs_param.T1 = 300;
	dvfs_param.P1 = 80;
	dvfs_param.P1L = 0;
	dvfs_param.T2 = 500;
	dvfs_param.P2 = 80;
	dvfs_param.P2L = 45;
	dvfs_param.T3 = 1000;
	dvfs_param.P3L = 45;
	dvfs_param.dvfs_bulk_job_cnt = 0;
	v3d_device->fmwk_handle =
	mm_fmwk_register(V3D_DEV_NAME,
					V3D_AXI_BUS_CLK_NAME_STR,
					V3D_SUBDEV_COUNT,
					&core_param[0],
					&dvfs_param,
					&prof_param);
	if ((v3d_device->fmwk_handle == NULL)) {
		ret = -ENOMEM;
		goto err1;
	}


	pr_debug("v3d_init: H264 driver Module Init over");
	return ret;
err1:
	kfree(v3d_device);
err:
	pr_err("v3d_init: V3D driver Module Init Error");
	return ret;
}

void __exit mm_v3d_exit(void)
{
	int i = 0;
	pr_debug("V3D_exit:\n");
	if (v3d_device->fmwk_handle)
		mm_fmwk_unregister(v3d_device->fmwk_handle);
	/*Sub device deinit*/
	for (i = 0; i < V3D_SUBDEV_COUNT; i++)
		v3d_device->subdev_deinit[i]();
	kfree(v3d_device);
}

late_initcall(mm_v3d_init);
module_exit(mm_v3d_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("V3D device driver");
MODULE_LICENSE("GPL");
