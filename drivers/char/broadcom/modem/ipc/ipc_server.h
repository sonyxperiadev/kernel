#ifndef IPC_Server_h
#define IPC_Server_h

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

#ifdef __cplusplus
extern "C" {
#endif

	extern int IpcCPCrashCheck(void);
	extern void ProcessCPCrashedDump(struct work_struct *work);
	extern int IPC_IsCpIpcInit(void *pSmBase, IPC_CPU_ID_T Cpu);
	extern void abort(void);

	extern int ipc_crashsupport_init(void);
#if defined(CONFIG_BCM215X_PM) && defined(CONFIG_ARCH_BCM2153)
	extern void
	    pm_ipc_power_saving_init(IPC_PlatformSpecificPowerSavingInfo_T *
				     ipc_ps);
#endif

#ifdef __cplusplus
}
#endif
#endif
