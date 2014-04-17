/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*   Notwithstanding the above, under no circumstances may you combine this
*   software in any way with any other Broadcom software provided under a
*   license other than the GPL, without Broadcom's express prior written
*   consent.
*
****************************************************************************/

/**
 *
 *  @file   apanic_mmc.h
 *
 *  @brief  Interface to the Broadcom apanic driver.
 *
 *  @note   Calls emmc polling mode driver
 *
 **/

#ifndef APANIC_MMC_H
#define APANIC_MMC_H
#ifdef __cplusplus
extern "C" {
#endif

extern void log_buf_clear(void);
extern void ram_console_enable_console(int);
extern unsigned long get_apanic_start_address(void);
extern unsigned long get_apanic_end_address(void);
extern int mmc_poll_stack_init(void **mmc, int dev_num, int *mmc_poll_dev_num);

#ifdef __cplusplus
}
#endif
#endif
