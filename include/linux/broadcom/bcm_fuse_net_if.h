/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/bcm_fuse_net_if.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef _LINUX_BCM_FUSE_NET_IF_H
#define _LINUX_BCM_FUSE_NET_IF_H

#include <linux/sockios.h>

#define BCM_FUSE_NET_ACTIVATE_PDP   	0x1
#define BCM_FUSE_NET_DEACTIVATE_PDP     0x2

#define BCM_NET_MAX_PDP_CNTXS           2
#define BCM_NET_INVALID_PDP_CNTX        255

#define BCM_NET_DEV_STR        "rmnet%d"

typedef struct {
	int context_id;
	unsigned long ip_addr;
} bcm_fuse_net_pdp_t;

typedef struct {
	unsigned short cmd;
	void *cmd_data;		/* pointer to user buffer */
	uint data_len;		/* length of user buffer */
} bcm_fuse_net_config_t;

#endif /* _LINUX_BCM_FUSE_NET_IF_H */
