/*****************************************************************************
  * Copyright 2013 Broadcom Corporation.  All rights reserved.
  *
  * Unless you and Broadcom execute a separate written software license
  * agreement governing use of this software, this software is licensed to you
  * under the terms of the GNU General Public License version 2, available at
  * http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
  *
  * Notwithstanding the above, under no circumstances may you combine this
  * software in any way with any other Broadcom software provided under a
  * license other than the GPL, without Broadcom's express prior written
  * consent.
  *****************************************************************************/

#ifndef __PDM_MM_INTERNAL_H__
#define __PDM_MM_INTERNAL_H__

#include <linux/plist.h>
#include <linux/io.h>

enum bcm_pdm_mm_resource_type {
	BCM_PDM_MM_RESOURCE_TYPE_POWER,
	BCM_PDM_MM_RESOURCE_TYPE_FREQ,
	BCM_PDM_MM_RESOURCE_TYPE_MAX
};

enum bcm_pdm_mm_resource {
	BCM_PDM_MM_RESOURCE_H264_POWER,
	BCM_PDM_MM_RESOURCE_H264_FREQ,
	BCM_PDM_MM_RESOURCE_MAX
};

struct bcm_pdm_mm_qos_resource {
	char *name;
	enum bcm_pdm_mm_resource_type type;
	enum bcm_pdm_mm_resource resource;
	int cvalue;
};

struct bcm_pdm_mm_qos_req {
	char *name;
	bool is_active;
	enum bcm_pdm_mm_resource_type type;
	enum bcm_pdm_mm_resource resource;
	int cvalue;

	struct list_head node;
};

#endif

