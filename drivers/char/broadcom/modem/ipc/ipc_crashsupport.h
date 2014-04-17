#ifndef IPC_Crashsupport_h
#define IPC_Crashsupport_h

/*
 * 2007-2009 Broadcom Corporation

 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2, available
 * at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
 * Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 */

#include <linux/vmalloc.h>

#ifdef __cplusplus
extern "C" {
#endif
#ifdef CONFIG_APANIC_ON_MMC
	extern int ap_triggered;
#endif
	extern int cp_crashed;
	extern void abort(void);
#ifdef CONFIG_CDEBUGGER
	extern unsigned ramdump_enable;
#endif

#if defined(CONFIG_BCM_AP_PANIC_ON_CPCRASH) && defined(CONFIG_SEC_DEBUG)
extern void cp_abort(void);
#endif

extern int RpcDbgDumpHistoryLogging(int type, int level);

#ifdef CONFIG_HAS_WAKELOCK
	extern struct wake_lock ipc_wake_lock;
#endif

#ifdef __cplusplus
}
#endif
#endif
