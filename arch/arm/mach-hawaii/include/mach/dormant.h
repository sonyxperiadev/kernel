/****************************************************************************
*
* Copyright 2010 -- 2012 Broadcom Corporation.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*****************************************************************************/

#ifndef DORMANT_H
#define DORMANT_H

enum DORMANT_SERVICE_TYPE {
	DORMANT_CORE_DOWN = 0,    /* Take down this core */
	DORMANT_CLUSTER_DOWN = 1  /* Take down the cluster */
};

/* Main API for dormant entry */
extern void dormant_enter(u32 service);

/* Exported API for saving return path and continue */
extern u32 dormant_enter_prepare(u32 dont_care, u32 v_to_p_offset);

extern void invalidate_tlb_btac(void);

/* Exported API to check if dormant is enabled or not */
extern u32 is_dormant_enabled(void);

#endif /* DORMANT_H */
