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

//#define BCM_NET_MAX_PDP_CNTXS          3
#define BCM_NET_MAX_DUN_PDP_CNTXS           7  //DUN context 1-7
#define BCM_NET_MAX_PDP_CNTXS           10  //based on CP support

#define BCM_NET_RIL_PDP_CNTXS_BASE     (BCM_NET_MAX_DUN_PDP_CNTXS + 1) //8 for now
#define BCM_NET_MAX_RIL_PDP_CNTXS          (BCM_NET_MAX_PDP_CNTXS - BCM_NET_MAX_DUN_PDP_CNTXS)  //RIL context id 8-10

//due to the support for DUN, which requires to use cid starting from 0,
//RIL now has cid range from 8-10 (BCM_NET_RIL_PDP_CNTXS_BASE - BCM_NET_MAX_PDP_CNTXS)
//but android has hardcoded rmnet0 in several places, in order not to change
//android code for later integration compliexity, we map rmnet0-rmnet2 with cid 8-10
//and have rmnet3-rmnet9 with cid 1-7 for DUN
#define RIL_CID_TO_RMNET(cid)  (cid - BCM_NET_RIL_PDP_CNTXS_BASE)
#define DUN_CID_TO_RMNET(cid)  (cid + BCM_NET_MAX_RIL_PDP_CNTXS - 1)
#define RIL_RMNET_TO_CID(rmnetId)  (rmnetId + BCM_NET_RIL_PDP_CNTXS_BASE)
#define DUN_RMNET_TO_CID(rmnetId)  (rmnetId - BCM_NET_MAX_RIL_PDP_CNTXS + 1)
#define RMNET_TO_CID(rmnetId)  \
        (rmnetId < BCM_NET_MAX_RIL_PDP_CNTXS)?RIL_RMNET_TO_CID(rmnetId):DUN_RMNET_TO_CID(rmnetId)


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
