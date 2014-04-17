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
#ifndef __BCM_PDM_MM_H__
#define __BCM_PDM_MM_H__

#include <linux/types.h>
#include <linux/kernel.h>

struct bcm_pdm_mm_qos_req;


/* Primary pi_mm_manager api's */
int bcm_pdm_mm_qos_req_create(struct bcm_pdm_mm_qos_req **req,
			      const char *req_name,
			      const char *resource);
void bcm_pdm_mm_qos_req_destroy(struct bcm_pdm_mm_qos_req *req);
int bcm_pdm_mm_qos_req_enable(struct bcm_pdm_mm_qos_req *req);
int bcm_pdm_mm_qos_req_disable(struct bcm_pdm_mm_qos_req *req);
int bcm_pdm_mm_qos_req_update(struct bcm_pdm_mm_qos_req *req, int cvalue);

#endif /* __BCM_PDM_MM_H__ */
