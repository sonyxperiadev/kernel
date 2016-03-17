/*******************************************************************************
* Copyright 2012 Broadcom Corporation.  All rights reserved.
*
* @file	kernel/include/linux/broadcom//bcmbt_gps.h
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

#ifndef __ASM_BCMBT_GPS_H
#define __ASM_BCMBT_GPS_H

struct bcmbt_gps_selftest_cmd {
	int cmd;
	int param;
};

#define TIO_GPS_SETLFTEST_CMD	_IOW(BRCM_SHARED_UART_MAGIC, 16, struct \
	bcmbt_gps_selftest_cmd)

/* gps selftest command list */
#define GPS_SELFTEST_SETUP   1	/* init/deinit on self test */
#define GPS_SELFTEST_PABLANK 2
#define GPS_SELFTEST_CALREQ  3

#define GPS_SELFTEST_INIT 1
#define GPS_SELFTEST_DEINIT 0
#define GPS_SELFTEST_ASSERT_PIN  1
#define GPS_SELFTEST_DEASSERT_PIN 0

int bcmbt_gps_selftest(struct bcmbt_gps_selftest_cmd *p_cmd);
int __bcmbt_gps_selftest_init(void);
void __bcmbt_gps_selftest_exit(void);

#endif
