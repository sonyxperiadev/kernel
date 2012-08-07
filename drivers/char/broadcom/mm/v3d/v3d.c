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
#include <mach/clock.h>
#include <asm/io.h>
#include <linux/clk.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/dma-mapping.h>

#include <linux/broadcom/v3d.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <mach/rdb/brcm_rdb_mm_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_v3d.h>
#include <linux/broadcom/mm_fw_hw_ifc.h>
#include <linux/broadcom/mm_fw_usr_ifc.h>

#define V3D_HW_SIZE (1024*4)

#define IRQ_V3D	BCM_INT_ID_RESERVED148
#define V3D_BIN_OOM_SIZE 4*1024*1024

typedef struct {
	int v3d_bin_oom_block;
	int v3d_bin_oom_size ;
	void* v3d_bin_oom_cpuaddr;
	int v3d_bin_oom_block2;
	int v3d_bin_oom_size2 ;
	void* v3d_bin_oom_cpuaddr2;

	volatile int v3d_oom_block_used ;

	volatile void *vaddr;
} v3d_device_t;

static inline void v3d_write(v3d_device_t *v3d, unsigned int reg, unsigned int value) {
	return mm_write_reg(v3d->vaddr, reg, value);
}

static inline unsigned int v3d_read(v3d_device_t *v3d, unsigned int reg) {
	return mm_read_reg(v3d->vaddr, reg);
}

static int v3d_reset(void* device_id)
{
	v3d_device_t* id = (v3d_device_t*)device_id;

	v3d_write(id,V3D_CT0CS_OFFSET , 0x8000);
	v3d_write(id,V3D_CT1CS_OFFSET , 0x8000);
	
	v3d_write(id,V3D_L2CACTL_OFFSET , 2);

	v3d_write(id,V3D_CT0CS_OFFSET , 0x8000);
	v3d_write(id,V3D_CT1CS_OFFSET , 0x8000);

	v3d_write(id,V3D_RFC_OFFSET , 1);
	v3d_write(id,V3D_BFC_OFFSET , 1);
	v3d_write(id,V3D_SLCACTL_OFFSET , 0x0f0f0f0f);
	v3d_write(id,V3D_VPMBASE_OFFSET , 0);
	v3d_write(id,V3D_VPACNTL_OFFSET , 0);
	v3d_write(id,V3D_BPOA_OFFSET , id->v3d_bin_oom_block);
	v3d_write(id,V3D_BPOS_OFFSET , id->v3d_bin_oom_size);
	v3d_write(id,V3D_INTCTL_OFFSET , 0xF);
	v3d_write(id,V3D_INTENA_OFFSET , 0x7);

	id->v3d_oom_block_used = 0;


	return 0;
}

static int v3d_print_regs(void* device_id)
{
	v3d_device_t* id = (v3d_device_t*)device_id;

	err_print("regs:(0x%08x 0x%08x 0x%08x) (0x%08x 0x%08x 0x%08x) (0x%08x 0x%08x 0x%08x)\n",v3d_read(id,V3D_CT0CS_OFFSET),v3d_read(id,V3D_CT0CA_OFFSET),v3d_read(id,V3D_CT0EA_OFFSET),
		v3d_read(id,V3D_CT1CS_OFFSET),v3d_read(id,V3D_CT1CA_OFFSET),v3d_read(id,V3D_CT1EA_OFFSET),
		v3d_read(id,V3D_BPOA_OFFSET),v3d_read(id,V3D_BPOS_OFFSET),v3d_read(id,V3D_PCS_OFFSET));
	dbg_print("%x %x %x\n",v3d_read(id,V3D_PCS_OFFSET),v3d_read(id,V3D_CT0CS_OFFSET),v3d_read(id,V3D_CT1CS_OFFSET));
}

static int v3d_abort(void* device_id, mm_job_post_t* job)
{
	v3d_device_t* id = (v3d_device_t*)device_id;
	v3d_write(id,V3D_CT0CS_OFFSET , 0x20);
	v3d_write(id,V3D_CT1CS_OFFSET , 0x20);
	v3d_write(id,V3D_CT0CA_OFFSET , 0);
	v3d_write(id,V3D_CT0EA_OFFSET , 0);
	v3d_write(id,V3D_CT1CA_OFFSET , 0);
	v3d_write(id,V3D_CT1EA_OFFSET , 0);

	v3d_write(id,V3D_CT0CS_OFFSET , 0x8000);
	v3d_write(id,V3D_CT1CS_OFFSET , 0x8000);

	v3d_reset(id);	
	return 0;
}

static mm_isr_type_e process_v3d_irq(void* device_id)
{
	u32 flags, flags_qpu, tmp;
	mm_isr_type_e irq_retval = MM_ISR_UNKNOWN;
	v3d_device_t* id = (v3d_device_t*)device_id;

	/* Read the interrupt status registers */
	flags = v3d_read(id,V3D_INTCTL_OFFSET);
	flags_qpu = v3d_read(id,V3D_DBQITC_OFFSET);

	/* Clear interrupts isr is going to handle */
	tmp = flags & v3d_read(id,V3D_INTENA_OFFSET);
	v3d_write(id,V3D_INTCTL_OFFSET,tmp);
	if (flags_qpu) {
		v3d_write(id,V3D_DBQITC_OFFSET, flags_qpu);
	}

	/* Handle Binning Interrupt*/
	if (flags & 2) {
		irq_retval = MM_ISR_PROCESSED;
	}

	/* Handle Rendering Interrupt*/
	if (flags & 1) {
		irq_retval = MM_ISR_SUCCESS;
		id->v3d_oom_block_used = 0;
	}

	/* Handle oom case */
	if (flags & (1 << 2)) {
		irq_retval = MM_ISR_PROCESSED;
		if (id->v3d_oom_block_used == 0) {
			v3d_write(id,V3D_BPOA_OFFSET, id->v3d_bin_oom_block);
			v3d_write(id,V3D_BPOS_OFFSET, id->v3d_bin_oom_size);
			id->v3d_oom_block_used = 1;
			err_print("v3d hw asking for OOM");
		}/*else if (id->v3d_oom_block_used == 1) {
			v3d_write(id,V3D_BPOA_OFFSET, id->v3d_bin_oom_block2);
			v3d_write(id,V3D_BPOS_OFFSET, id->v3d_bin_oom_size2);
			id->v3d_oom_block_used = 2;
		}*/else {
			v3d_write(id,V3D_INTDIS_OFFSET, 1 << 2);
			v3d_write(id,V3D_CT0CS_OFFSET , 0x8000);
			v3d_write(id,V3D_CT1CS_OFFSET , 0x8000);
			err_print("v3d hw resetting for OOM");

			irq_retval = MM_ISR_ERROR;
			id->v3d_oom_block_used = 0;
		}
		/* Clear the oom interrupt ? */
		v3d_write(id,V3D_INTCTL_OFFSET, 1 << 2);
	}

	return irq_retval;


}

bool get_v3d_status(void* device_id)
{
	v3d_device_t* id = (v3d_device_t*)device_id;

	dbg_print("%x %x %x\n",v3d_read(id,V3D_PCS_OFFSET),v3d_read(id,V3D_CT0CS_OFFSET),v3d_read(id,V3D_CT1CS_OFFSET));
	if((v3d_read(id,V3D_CT0CS_OFFSET)) | (v3d_read(id,V3D_CT1CS_OFFSET)) | (v3d_read(id,V3D_PCS_OFFSET)) ){
		return true;
		}
	return false;
}

mm_job_status_e v3d_start_job(void* device_id , mm_job_post_t* job)
{
	v3d_device_t* id = (v3d_device_t*)device_id;
	v3d_job_t* job_params = (v3d_job_t*)job->data;

	switch( job->job_status ){
		case MM_JOB_STATUS_READY:
			{
				v3d_reset(id);
				if (job->job_type == V3D_REND_JOB) {
					job->job_status = MM_JOB_STATUS_RUNNING;
					v3d_write(id,V3D_CT1CA_OFFSET,job_params->v3d_ct1ca);
					v3d_write(id,V3D_CT1EA_OFFSET,job_params->v3d_ct1ea);
					return MM_JOB_STATUS_RUNNING;
				} else if (job->job_type == V3D_BIN_REND_JOB){
					job->job_status = MM_JOB_STATUS_RUNNING;
					if(job_params->v3d_ct0ca != job_params->v3d_ct0ea) {
						v3d_write(id,V3D_CT0CA_OFFSET,job_params->v3d_ct0ca);
						v3d_write(id,V3D_CT0EA_OFFSET,job_params->v3d_ct0ea);
						}
					v3d_write(id,V3D_CT1CA_OFFSET,job_params->v3d_ct1ca);
					v3d_write(id,V3D_CT1EA_OFFSET,job_params->v3d_ct1ea);
					v3d_write(id,V3D_BPOA_OFFSET , id->v3d_bin_oom_block);
					v3d_write(id,V3D_BPOS_OFFSET , id->v3d_bin_oom_size);
					return MM_JOB_STATUS_RUNNING;
				}
			}
		break;
		case MM_JOB_STATUS_RUNNING:
			{
				job->job_status = MM_JOB_STATUS_SUCCESS;
				return MM_JOB_STATUS_SUCCESS;
			}
		break;
		}
	return MM_JOB_STATUS_ERROR;
}
v3d_device_t* gV3d = NULL;

int __init v3d_init(void)
{
	int ret = 0;
	MM_FMWK_HW_IFC ifc_param;
	gV3d = kmalloc(sizeof(v3d_device_t), GFP_KERNEL);
	gV3d->vaddr = NULL;
	dbg_print("V3D driver Module Init\n");

	gV3d->v3d_bin_oom_cpuaddr = kmalloc(V3D_BIN_OOM_SIZE,GFP_KERNEL);//dma_alloc_coherent(NULL, V3D_BIN_OOM_SIZE, &gV3d->v3d_bin_oom_block, GFP_ATOMIC | GFP_DMA);
	if (gV3d->v3d_bin_oom_cpuaddr == NULL) {
		err_print("dma_alloc_coherent failed for v3d oom block size[0x%x]", gV3d->v3d_bin_oom_size);
		gV3d->v3d_bin_oom_block = 0;
		gV3d->v3d_bin_oom_size = 0;
		ret = -ENOMEM;
		goto err;
	}
	gV3d->v3d_bin_oom_block = virt_to_phys(gV3d->v3d_bin_oom_cpuaddr);
	gV3d->v3d_bin_oom_size = V3D_BIN_OOM_SIZE ;
	dbg_print("v3d bin oom phys[0x%08x], size[0x%08x] cpuaddr[0x%08x]",
		gV3d->v3d_bin_oom_block,gV3d->v3d_bin_oom_size, (int)gV3d->v3d_bin_oom_cpuaddr);

	gV3d->v3d_bin_oom_cpuaddr2 = kmalloc(V3D_BIN_OOM_SIZE,GFP_KERNEL);//dma_alloc_coherent(NULL, V3D_BIN_OOM_SIZE, &gV3d->v3d_bin_oom_block, GFP_ATOMIC | GFP_DMA);
//	gV3d->v3d_bin_oom_cpuaddr2 = dma_alloc_coherent(NULL, V3D_BIN_OOM_SIZE, &gV3d->v3d_bin_oom_block2, GFP_ATOMIC | GFP_DMA);
	if (gV3d->v3d_bin_oom_cpuaddr2 == NULL) {
		err_print("dma_alloc_coherent failed for v3d oom block size[0x%x]", gV3d->v3d_bin_oom_size2);
		gV3d->v3d_bin_oom_block2 = 0;
		gV3d->v3d_bin_oom_size2 = 0;
		ret = -ENOMEM;
		goto err;
	}
	gV3d->v3d_bin_oom_block = virt_to_phys(gV3d->v3d_bin_oom_cpuaddr2);
	gV3d->v3d_bin_oom_size2 = V3D_BIN_OOM_SIZE;
	dbg_print("v3d bin oom2 phys[0x%08x], size[0x%08x] cpuaddr[0x%08x]",
		gV3d->v3d_bin_oom_block2, gV3d->v3d_bin_oom_size2, (int)gV3d->v3d_bin_oom_cpuaddr2);

	ifc_param.mm_dev_name = V3D_DEV_NAME;
	ifc_param.mm_dev_clk_name = V3D_AXI_BUS_CLK_NAME_STR;
	ifc_param.mm_dev_base_addr = MM_V3D_BASE_ADDR;
	ifc_param.mm_dev_hw_size = V3D_HW_SIZE;
	ifc_param.mm_dev_irq = IRQ_V3D;

	ifc_param.mm_dev_timer = DEFAULT_MM_DEV_TIMER_MS;
	ifc_param.mm_dev_timeout = DEFAULT_MM_DEV_TIMEOUT_MS;
	
	ifc_param.mm_dev_get_status = get_v3d_status;
	ifc_param.mm_dev_start_job = v3d_start_job;
	ifc_param.mm_dev_process_irq = process_v3d_irq;
	ifc_param.mm_dev_init = v3d_reset;
	ifc_param.mm_dev_deinit = v3d_reset;
	ifc_param.mm_dev_abort = v3d_abort;
	ifc_param.mm_dev_print_regs = v3d_print_regs;
	ifc_param.mm_dvfs_params.is_dvfs_on = 1;
	ifc_param.mm_dvfs_params.user_requested_mode = TURBO;
	ifc_param.mm_dvfs_params.enable_suspend_resume = 0;
	ifc_param.mm_dvfs_params.T1 = 300;
	ifc_param.mm_dvfs_params.P1 = 80;
	ifc_param.mm_dvfs_params.T2 = 3000;
	ifc_param.mm_dvfs_params.P2 = 30;
	ifc_param.mm_dvfs_params.dvfs_bulk_job_cnt = 0;
	ifc_param.mm_device_id = (void *)gV3d;
	ifc_param.mm_dev_virt_addr = NULL;

	ret = mm_fmwk_register(&ifc_param);
	/* get kva from fmwk */
	gV3d->vaddr = ifc_param.mm_dev_virt_addr;
err:
	dbg_print("V3D driver Module Init over\n");
	return ret;
}

void __exit v3d_exit(void)
{
	dbg_print("V3D driver Module Exit\n");
	mm_fmwk_unregister(gV3d, V3D_DEV_NAME, IRQ_V3D);
	if (gV3d->v3d_bin_oom_cpuaddr2)
		dma_free_coherent(NULL, gV3d->v3d_bin_oom_size2, gV3d->v3d_bin_oom_cpuaddr2, gV3d->v3d_bin_oom_block2);
	if (gV3d->v3d_bin_oom_cpuaddr)
		dma_free_coherent(NULL, gV3d->v3d_bin_oom_size, gV3d->v3d_bin_oom_cpuaddr, gV3d->v3d_bin_oom_block);
	kfree(gV3d);
}


module_init(v3d_init);
module_exit(v3d_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("V3D device driver");
MODULE_LICENSE("GPL");
