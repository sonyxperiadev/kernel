/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/saradc.h
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

#ifndef BCM59038_SARADC_H
#define BCM59038_SARADC_H

typedef void (*saradc_rtm_callback_handler) (void *param, u32 selection,
					     u16 data);

int bcm59038_saradc_start_burst_mode(struct bcm59038 *bcm59038);
int bcm59038_saradc_stop_burst_mode(struct bcm59038 *bcm59038);
int bcm59038_saradc_read_data(struct bcm59038 *bcm59038, int sel);
int bcm59038_saradc_request_rtm(struct bcm59038 *bcm59038,
				saradc_rtm_callback_handler handler, int ch_sel,
				void *arg);
int bcm59038_saradc_set_rtm_delay(struct bcm59038 *bcm59038, int delay);

#endif /* BCM59038_SARADC_H */
