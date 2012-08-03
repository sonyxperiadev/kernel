/****************************************************************************
*
* Copyright 2010 --2011 Broadcom Corporation.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*****************************************************************************/

#ifndef CAPRI_DORMANT_H
#define CAPRI_DORMANT_H

enum CAPRI_DORMANT_SERVICE_TYPE {
	CAPRI_DORMANT_CORE_DOWN = 0,	/* Take down this core */
	CAPRI_DORMANT_CLUSTER_DOWN = 1	/* Take down the cluster */
};

/* Main API for dormant entry */
extern void dormant_enter(enum CAPRI_DORMANT_SERVICE_TYPE service);

/* Exported API for saving return path and continue */
extern u32 dormant_enter_prepare(u32 dont_care, u32 v_to_p_offset);

/* Exported API to check if dormant is enabled or not */
extern u32 is_dormant_enabled(void);

#endif
