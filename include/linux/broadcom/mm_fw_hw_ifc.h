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
#include <linux/slab.h>

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
	MM_ISR_SUCCESS = 0,
    MM_ISR_ERROR
} mm_isr_type_e;


typedef struct {
	bool is_dvfs_on;
	bool enable_susres;
	unsigned int T1; //time in ms for DVFS profiling when in Normal mode
	unsigned int P1; // percentage (1~99) threshold at which framework should request Turbo mode for this device
	unsigned int T2; //time in ms for DVFS profiling when in Turbo mode
	unsigned int P2; // percentage (1~99) threshold at which framework should fall back to Normal mode for this device 
}MM_DVFS;

typedef struct mm_fmwk_hw_ifc {
	char *dev_name;
	char *dev_clk_name;
	uint32_t dev_base_addr;
	uint32_t dev_hw_size;
	uint8_t dev_irq;
	bool dvfs_sus_res;
	MM_DVFS dvfs_params;
	unsigned int dvfs_bulk_job_cnt;
	/* device funcs */
	bool (*get_hw_status)(void *device_id);
	mm_job_status_e (*dev_start_job)(void *device_id, mm_job_post_t *job);
	mm_isr_type_e (*process_irq)(void *device_id);
	int (*dev_reset)(void *device_id);
	void *device_id;//aby device specific data
	void *virt_addr;//to be filled in by fmwk init with KVA
}MM_FMWK_HW_IFC;

void dev_write(void *base_addr, u32 reg, u32 value);
u32 dev_read(void *base_addr, u32 reg);
void dev_clr_bit32(void *base_addr, u32 reg, unsigned long bits);
void dev_write_bit32(void *base_addr, u32 reg, unsigned long bits);

int dev_init(MM_FMWK_HW_IFC *ifc_param);
void dev_exit(void *data, const char *dev_name, uint8_t dev_irq);


#endif
