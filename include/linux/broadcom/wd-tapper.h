/*************************************************************************
* Copyright 2010  Broadcom Corporation
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2 (the GPL),
* available at http://www.broadcom.com/licenses/GPLv2.php, with the following
* added to such license:
* As a special exception, the copyright holders of this software give you
* permission to link this software with independent modules, and to copy and
* distribute the resulting executable under terms of your choice, provided that
* you also meet, for each linked independent module, the terms and conditions
* of the license of that module. An independent module is a module which is not
* derived from this software.  The special exception does not apply to any
* modifications of the software.
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*****************************************************************************/
/*
 * linux/broadcom/wd-tapper.h
 *
 * Watchdog Tapper for PMU.
 */

#ifndef __LINUX_WD_TAPPER_H
#define __LINUX_WD_TAPPER_H
#include <mach/timex.h>
#include <linux/plist.h>

/* Seconds to ticks conversion */
#define sec_to_ticks(x) ((x)*CLOCK_TICK_RATE)
#define ticks_to_sec(x) ((x)/CLOCK_TICK_RATE)

#define TAPPER_DEFAULT_TIMEOUT	0xFFFFFFFF

unsigned int wd_tapper_get_timeout(void);

struct wd_tapper_platform_data {
	unsigned int count;
	unsigned int ch_num;
	char name[255];
};

struct wd_tapper_node {
	char *name;
	struct plist_node node;
	u32 timeout;
	bool valid;
};

int wd_tapper_add_timeout_req(struct wd_tapper_node *wd_node,
		char *client_name, u32 timeout);
int wd_tapper_del_timeout_req(struct wd_tapper_node *wd_node);
int wd_tapper_update_timeout_req(struct wd_tapper_node *wd_node, u32 timeout);

#endif
