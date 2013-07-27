/*******************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef _MM_FW_HW_H_
#define _MM_FW_HW_H_

#include <linux/kernel.h>
#include <linux/notifier.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/file.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <mach/irqs.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/list.h>
#include <linux/plist.h>
#include <plat/pi_mgr.h>
#include <linux/debugfs.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>

#include <plat/clock.h>
#include <linux/workqueue.h>

#include "mm_fw_usr_ifc.h"

#define MAX_ASYMMETRIC_PROC (16)

#define DEFAULT_MM_DEV_TIMER_MS (100)
#if defined(CONFIG_MACH_BCM_FPGA_E) || defined(CONFIG_MACH_BCM_FPGA)
#define DEFAULT_MM_DEV_TIMEOUT_MS (80000)
#else
#define DEFAULT_MM_DEV_TIMEOUT_MS (1000)
#endif

enum {
	MM_ISR_UNKNOWN = 0,
	MM_ISR_SUCCESS,
	MM_ISR_ERROR,
	MM_ISR_PROCESSED
};
#define mm_isr_type_e int

enum {
	ECONOMY = PI_OPP_ECONOMY,
	NORMAL = PI_OPP_NORMAL,
	TURBO = PI_OPP_TURBO,
#if defined(CONFIG_PI_MGR_MM_STURBO_ENABLE)
	SUPER_TURBO = PI_OPP_SUPER_TURBO,
#endif
};
#define dvfs_mode_e unsigned int

struct mm_reg_value {
	const char *name;
	unsigned int value;
};
#define MM_REG_VALUE struct mm_reg_value

struct mm_dvfs_hw_ifc {
	bool ON;
	dvfs_mode_e MODE; /* When DVFS is off,
					this mode will be chosen */
	unsigned int dvfs_bulk_job_cnt;

	unsigned int T0; /* time in ms for DVFS profiling
					when in Economy mode */
	unsigned int P0; /* percentage (1~99) threshold at which framework
			should request Normal mode for this device */

	unsigned int T1; /* time in ms for DVFS profiling when in Normal mode */
	unsigned int P1; /* percentage (1~99) threshold at which framework
			should request Turbo mode for this device */
	unsigned int P1L; /* percentage (1~99) threshold at which framework
			should request Normal mode for this device */

	unsigned int T2; /* time in ms for DVFS profiling when in Turbo mode */
	unsigned int P2; /* percentage (1~99) threshold at which framework
			should request to Super Turbo mode for this device */
	unsigned int P2L; /* percentage (1~99) threshold at which framework
			should fall back to Normal mode for this device */

	unsigned int T3; /* time in ms for DVFS profiling
					when in Super Turbo mode */
	unsigned int P3L; /* percentage (1~99) threshold at which framework
			should fall back to Turbo mode for this device */
};
#define MM_DVFS_HW_IFC struct mm_dvfs_hw_ifc

struct mm_prof_hw_ifc {
	unsigned int bitmask;
	const char *desc[32];
};
#define MM_PROF_HW_IFC struct mm_prof_hw_ifc

struct mm_core_hw_ifc {
	uint8_t mm_irq;
	uint32_t mm_base_addr;
	void *mm_virt_addr; /* to be filled in by fmwk init with KVA */
	void *mm_device_id; /* any device specific data */

	uint32_t mm_hw_size;
	uint32_t mm_timer;
	uint32_t mm_timeout;

	/* device funcs */
	int (*mm_init)(void *device_id);
	int (*mm_deinit)(void *device_id);
	bool (*mm_get_status)(void *device_id);
	int (*mm_start_job)(void *device_id, mm_job_post_t *job,
					unsigned int profmask);
	void (*mm_update_virt_addr)(void *vaddr);
	int (*mm_process_irq)(void *device_id);
	int (*mm_reset)(void *device_id);
	int (*mm_version_init)(void *device_id, void *vaddr,
				mm_version_info_t *version_info);
	int (*mm_abort)(void *device_id, mm_job_post_t *job);
	int (*mm_get_regs)(void *device_id, MM_REG_VALUE *ptr, int max);
	int (*mm_get_prof)(void *device_id, unsigned int *ptr);
#if defined(CONFIG_MM_SECURE_DRIVER)
	int (*mm_secure_job_wait)(void *device_id,
			mm_secure_job_ptr p_secure_job);
	int (*mm_secure_job_done)(void *device_id,
			mm_secure_job_ptr p_secure_job);
#endif /* CONFIG_MM_SECURE_DRIVER */

};
#define MM_CORE_HW_IFC struct mm_core_hw_ifc

void *mm_fmwk_register(const char *name, const char *clk_name,
						unsigned int count,
						MM_CORE_HW_IFC *ifc_param,
						MM_DVFS_HW_IFC *dvfs_param,
						MM_PROF_HW_IFC *prof_param);

void mm_fmwk_unregister(void *handle);


static inline void mm_write_reg(void *base_addr, u32 reg, u32 value)
{
	writel(value, base_addr + reg);
}

static inline u32 mm_read_reg(void *base_addr, u32 reg)
{
	return readl(base_addr + reg);
}

static inline void mm_clr_bit32(void *base_addr, u32 reg, unsigned long bits)
{
	writel(readl(base_addr + reg) & ~bits, (base_addr + reg));
}

static inline void mm_write_bit32(void *base_addr, u32 reg, unsigned long bits)
{
	writel(readl(base_addr + reg) | bits,  (base_addr + reg));
}

#endif
