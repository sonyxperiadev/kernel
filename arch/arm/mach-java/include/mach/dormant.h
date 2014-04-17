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

enum dormant_service {
	CORE_DORMANT = 0,
	FULL_DORMANT_L2_ON,
	FULL_DORMANT_L2_OFF,
	DRMT_SVC_MAX
};

#define DORMANT_ENTRY_SUCCESS		0
#define DORMANT_ENTRY_FAILURE		1


/* Main API for dormant entry */
extern void dormant_enter(u32 service);

/* Exported API for saving return path and continue */
extern u32 dormant_enter_prepare(u32 dont_care, u32 v_to_p_offset);
extern u32 is_dormant_enabled(void);
extern void enable_dormant(u32 core_mask);
extern void invalidate_tlb_btac(void);

#endif /* DORMANT_H */
