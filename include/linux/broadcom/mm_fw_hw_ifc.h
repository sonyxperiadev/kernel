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

//#define MM_DEBUG
#ifdef MM_DEBUG
#define dbg_print(fmt, arg...) \
    printk(KERN_ALERT "%s():" fmt, __func__, ##arg)
#else
#define dbg_print(fmt, arg...)   do { } while (0)
#endif

#define err_print(fmt, arg...) \
    printk(KERN_ERR "%s():" fmt, __func__, ##arg)


typedef enum {
	MM_FMWK_REGISTER_SUCCESS = 0,
	MM_FMWK_VALIDATE_ERROR,
	MM_FMWK_MISC_REGISTER_ERROR,
	MM_FMWK_REGISTER_MAP_ERROR,
	MM_FMWK_GENERAL_ERROR,
	
} mm_fmwk_register_t;

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
        MAX
}dvfs_mode_e;

typedef struct {
	bool is_dvfs_on;
	bool enable_suspend_resume;
	dvfs_mode_e user_requested_mode; // When DVFS is off, this mode will be chosen

	unsigned int T1; //time in ms for DVFS profiling when in Normal mode
	unsigned int P1; // percentage (1~99) threshold at which framework should request Turbo mode for this device
	unsigned int T2; //time in ms for DVFS profiling when in Turbo mode
	unsigned int P2; // percentage (1~99) threshold at which framework should fall back to Normal mode for this device 

	unsigned int dvfs_bulk_job_cnt;
}MM_DVFS;

typedef struct mm_fmwk_hw_ifc {
	char *mm_dev_name;
	char *mm_dev_clk_name;

	uint8_t mm_dev_irq;

	uint32_t mm_dev_base_addr;
	uint32_t mm_dev_hw_size;
	void *mm_dev_virt_addr;//to be filled in by fmwk init with KVA

	/* dvfs related parameters*/
	MM_DVFS mm_dvfs_params;

	/* device funcs */
	int (*mm_dev_init)(void *device_id);
	int (*mm_dev_deinit)(void *device_id);
	bool (*mm_dev_get_status)(void *device_id);
	mm_job_status_e (*mm_dev_start_job)(void *device_id, mm_job_post_t *job);
	mm_isr_type_e (*mm_dev_process_irq)(void *device_id);
	int (*mm_dev_reset)(void *device_id);
	int (*mm_dev_abort)(void *device_id, mm_job_post_t *job);
	int (*mm_dev_print_regs)(void *device_id);
	void *mm_device_id;//aby device specific data
	
}MM_FMWK_HW_IFC;

mm_fmwk_register_t mm_fmwk_register(MM_FMWK_HW_IFC *ifc_param);
void mm_fmwk_unregister(void *data, const char *dev_name, uint8_t dev_irq);


static inline void mm_write_reg(volatile void *base_addr, u32 reg, u32 value) {
	return writel(value, base_addr + reg);
}

static inline u32 mm_read_reg(volatile void *base_addr, u32 reg) {
	return readl(base_addr + reg);
}

#endif
