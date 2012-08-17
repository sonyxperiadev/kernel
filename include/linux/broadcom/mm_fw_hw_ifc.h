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
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <mach/irqs.h>
#include <asm/io.h>
#include <linux/clk.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/list.h>
#include <plat/pi_mgr.h>
#include <linux/debugfs.h>
#include <linux/miscdevice.h>

#include <plat/clock.h>
#include <linux/workqueue.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#include "mm_fw_usr_ifc.h"


#define DEFAULT_MM_DEV_TIMER_MS (100)
#define DEFAULT_MM_DEV_TIMEOUT_MS (1000)

typedef enum {
	MM_ISR_UNKNOWN = 0,
	MM_ISR_SUCCESS,
    MM_ISR_ERROR,
	MM_ISR_PROCESSED
} mm_isr_type_e;

typedef enum {
        ECONOMY = PI_OPP_ECONOMY,
        NORMAL,
        TURBO,
}dvfs_mode_e;

typedef struct {
	const char* name;
	unsigned int value;
} MM_REG_VALUE;

typedef struct {
	bool is_dvfs_on;
	bool enable_suspend_resume;
	dvfs_mode_e user_requested_mode; // When DVFS is off, this mode will be chosen
	unsigned int dvfs_bulk_job_cnt;

	unsigned int T1; //time in ms for DVFS profiling when in Normal mode
	unsigned int P1; // percentage (1~99) threshold at which framework should request Turbo mode for this device
	unsigned int T2; //time in ms for DVFS profiling when in Turbo mode
	unsigned int P2; // percentage (1~99) threshold at which framework should fall back to Normal mode for this device 
} MM_DVFS_HW_IFC;

typedef struct {
	unsigned int bitmask;
	const char* desc[32]; 
} MM_PROF_HW_IFC;

typedef struct mm_core_hw_ifc {
	uint8_t mm_irq;
	uint32_t mm_base_addr;
	void *mm_virt_addr;//to be filled in by fmwk init with KVA
	void *mm_device_id;//any device specific data

	uint32_t mm_hw_size;
	uint32_t mm_timer;
	uint32_t mm_timeout;

	/* device funcs */
	int (*mm_init)(void *device_id);
	int (*mm_deinit)(void *device_id);
	bool (*mm_get_status)(void *device_id);
	mm_job_status_e (*mm_start_job)(void *device_id, mm_job_post_t *job, unsigned int profmask);
	mm_isr_type_e (*mm_process_irq)(void *device_id);
	int (*mm_reset)(void *device_id);
	int (*mm_abort)(void *device_id, mm_job_post_t *job);
	int (*mm_get_regs)(void *device_id, MM_REG_VALUE* ptr, int max);
	int (*mm_get_prof)(void *device_id, unsigned int* ptr);
	
} MM_CORE_HW_IFC;

void* mm_fmwk_register(const char* name, const char* clk_name,
						MM_CORE_HW_IFC *ifc_param,
						MM_DVFS_HW_IFC* dvfs_param,
						MM_PROF_HW_IFC* prof_param);

void mm_fmwk_unregister(void* handle);


static inline void mm_write_reg(volatile void *base_addr, u32 reg, u32 value) {
	return writel(value, base_addr + reg);
}

static inline u32 mm_read_reg(volatile void *base_addr, u32 reg) {
	return readl(base_addr + reg);
}

#endif
