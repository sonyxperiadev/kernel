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
 *  @file   cp_crash.h
 *
 *  @brief  notifier function call after apanic
 *
 *  @note   Calls cp crash dump API
 *
 **/

#ifndef CP_CRASH_H
#define CP_CRASH_H
#ifdef __cplusplus
extern "C" {
#endif

/* External */
extern void ProcessCPCrashedDump(struct work_struct *);
extern int IpcCPCrashCheck(void);
extern void ipcs_get_ipc_state(int *state);
extern int cp_crashed;
extern int BCMLOG_GetCpCrashLogDevice(void);

#ifdef __cplusplus
}
#endif
#endif
