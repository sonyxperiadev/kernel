/*******************************************************************************
* Copyright 2013 Broadcom Corporation.  All rights reserved.
*
*       @file   drivers/misc/tmg399x_common.h
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
#ifndef _TMG399X_COMMON_H
#define _TMG399X_COMMON_H

void process_rgbc_prox_ges_raw_data(struct tmg399x_chip *chip,
	u8 type, u8 *data, u8 datalen);
void init_params_rgbc_prox_ges(void);
void set_visible_data_mode(struct tmg399x_chip *chip);
void tmg399x_report_prox(struct tmg399x_chip *chip, u8 detected);
void tmg399x_report_ges(struct tmg399x_chip *chip, int ges_report);
void tmg399x_report_als(struct tmg399x_chip *chip);
void tmg399x_set_ges_thresh(struct tmg399x_chip *chip,
		u8 entry, u8 exit);
void tmg399x_rgbc_poll_handle(unsigned long data);
void tmg399x_start_calibration(struct tmg399x_chip *chip);
void set_visible_data_mode(struct tmg399x_chip *chip);
void process_rgbc_prox_ges_raw_data(struct tmg399x_chip *chip, u8 type,
			u8 *data, u8 datalen);
void init_params_rgbc_prox_ges(void);
#endif
