/* include/media/msm_vpe_standalone.h
 *
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __LINUX_MSM_VPE_H
#define __LINUX_MSM_VPE_H

#include <linux/types.h>
#include "msm_camera.h"

#define MSM_VPE_IOCTL_MAGIC 'v'

#define MSM_VPE_IOCTL_CONFIG_VPE  \
	_IOWR(MSM_VPE_IOCTL_MAGIC, 1, struct msm_vpe_cfg_cmd *)
#define MSM_VPE_IOCTL_VPE_TRANSFER \
	_IOWR(MSM_VPE_IOCTL_MAGIC, 2, struct msm_vpe_transfer_cfg *)
#define MSM_VPE_IOCTL_VPE_REGISTER \
	_IOWR(MSM_VPE_IOCTL_MAGIC, 3, struct msm_vpe_register_cfg *)
#define MSM_VPE_IOCTL_VPE_UNREGISTER \
	_IOWR(MSM_VPE_IOCTL_MAGIC, 4, struct msm_vpe_unregister_cfg *)

struct msm_vpe_crop_info {
	int x;
	int y;
	int w;
	int h;
};

struct msm_vpe_transfer_cfg {
	void *srcAddr;
	void *dstAddr;
	struct msm_pmem_info src_info;
	struct msm_pmem_info dst_info;
	struct msm_vpe_crop_info src_crop;
	struct msm_vpe_crop_info dst_crop;
};

struct msm_vpe_register_cfg {
	struct msm_pmem_info inf;
	struct msm_vpe_crop_info crop;
};

struct msm_vpe_unregister_cfg {
	void *baseAddr;
};

#endif /* __LINUX_MSM_VPE_H */
