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

#ifndef __BCMLOG_CONFIG_H__
#define __BCMLOG_CONFIG_H__

#include "bcmlog.h"

/* /proc file to accept configuration changes */
#define BCMLOG_CONFIG_PROC_FILE		"brcm_logcfg"
#define BCMLOG_DEFAULT_FILE_BASE "/storage/sdcard1/"
#define BCMLOG_DEFAULT_UART_DEV "/dev/ttyS0"
#define BCMLOG_DEFAULT_ACM_DEV "/dev/ttyGS1"
#define MAX_STR_NAME 40

struct log_type_t {
	int dev;
	int lock;
	int (*handler) (const char *, unsigned int, char);
};

struct BCMLOG_Config_t {
	/*
	 *      ap_crashlog_dev
	 *      must be at beginning of struct; see brcm/dumpd/dumpd.c
	 */
	struct log_type_t ap_crashlog;
	struct log_type_t cp_crashlog;
	struct log_type_t runlog;
	unsigned long id_enable[1 + BCMLOG_MAX_LOG_ID / BITS_PER_LONG];
	int file_max;
	char file_base[MAX_STR_NAME];
	char uart_dev[MAX_STR_NAME];
	char acm_dev[MAX_STR_NAME];

};

/**
 *	Enable or disable log ID
 *	@param		inLogId (in) log id
 *	@param		inEnable (in) nonzero to enable; zero to disable log ID
 **/
void BCMLOG_EnableLogId(unsigned short inLogId, int inEnable);

/**
 *	Check if log id (sender) is enabled
 *	@param		inLogId (in) log id
 *	@return		1 if enabled; 0 if not enabled or invalid ID
 **/
int BCMLOG_LogIdIsEnabled(unsigned short inLogId);

/**
 *	Initialize logging configuration.  Schedules a work thread to
 *	load the configuration file once the file system is readable.
 **/
void BCMLOG_InitConfig(void *dev);

void BCMLOG_SetCpCrashLogDevice(int port);

void WriteToLogDev_ACM(void);
/**
 *	If the runtime log is on USB
 **/
int BCMLOG_IsUSBLog(void);

char *BCMLOG_GetFileBase(void);
char *BCMLOG_GetUartDev(void);
char *BCMLOG_GetAcmDev(void);


#endif /* __BCMLOG_FIFO_H__ */
