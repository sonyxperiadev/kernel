/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/

#ifndef _MMS_BLOCK_H
#define _MMS_BLOCK_H

struct white_list_node {
	int uid;
	struct white_list_node *next;
};

struct mms_event {
	int uid;
	char dest[24];
};

struct white_list_node *g_whitelistnode;
int g_pid;
int g_block_enabled;

extern void send_event_to_security_center(struct mms_event *pevent);

#endif /* _MMS_BLOCK_H */
