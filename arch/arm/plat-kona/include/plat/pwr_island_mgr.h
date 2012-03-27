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

#ifndef __POWER_ISLAND_MGR_H__
#define __POWER_ISLAND_MGR_H__

struct pi_ops;

enum {
	PI_POWER_DOWN_ON_DISABLE = (1 << 0),
};

struct pwr_island {
	char *name;
	u16 usg_cnt;
	u16 id;
	u16 state;
	u16 min_state_allowed;
	u32 flags;
	struct pi_ops *ops;
};

struct pi_ops {
	int (*init) (struct pwr_island * pi);
	int (*enable) (struct pwr_island * pi, int enable);
	int (*set_state) (struct pwr_island * pi, int state);

};

#endif /*__POWER_ISLAND_MGR_H__*/
