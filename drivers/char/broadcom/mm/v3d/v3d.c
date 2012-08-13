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
#ifdef CONFIG_ION
#include <linux/broadcom/kona_ion.h>
#endif


#define V3D_HW_SIZE (1024*4)

#define IRQ_V3D	BCM_INT_ID_RESERVED148
#if defined (CONFIG_MACH_HAWAII_FPGA_E) || defined (CONFIG_MACH_HAWAII_FPGA)
#define V3D_BIN_OOM_SIZE 512*1024
#else
#define V3D_BIN_OOM_SIZE 2*1024*1024
#endif

typedef struct {
#ifdef CONFIG_ION
	struct ion_client *v3d_bin_oom_client;
	struct ion_handle *v3d_bin_oom_handle;
	struct ion_handle *v3d_bin_oom_handle2;
#else
	void* v3d_bin_oom_cpuaddr;
	void* v3d_bin_oom_cpuaddr2;
#endif
	int v3d_bin_oom_block;
	int v3d_bin_oom_size ;
	int v3d_bin_oom_block2;
	int v3d_bin_oom_size2 ;

	volatile int v3d_oom_block_used ;

	volatile void *vaddr;

	void* fmwk_handle;
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

	pr_err("regs:(0x%08x 0x%08x 0x%08x) (0x%08x 0x%08x 0x%08x) (0x%08x 0x%08x 0x%08x)",v3d_read(id,V3D_CT0CS_OFFSET),v3d_read(id,V3D_CT0CA_OFFSET),v3d_read(id,V3D_CT0EA_OFFSET),
		v3d_read(id,V3D_CT1CS_OFFSET),v3d_read(id,V3D_CT1CA_OFFSET),v3d_read(id,V3D_CT1EA_OFFSET),
		v3d_read(id,V3D_BPOA_OFFSET),v3d_read(id,V3D_BPOS_OFFSET),v3d_read(id,V3D_PCS_OFFSET));
	pr_debug("%x %x %x",v3d_read(id,V3D_PCS_OFFSET),v3d_read(id,V3D_CT0CS_OFFSET),v3d_read(id,V3D_CT1CS_OFFSET));
	return 0;
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
			pr_err("v3d hw asking for OOM");
		}/*else if (id->v3d_oom_block_used == 1) {
			v3d_write(id,V3D_BPOA_OFFSET, id->v3d_bin_oom_block2);
			v3d_write(id,V3D_BPOS_OFFSET, id->v3d_bin_oom_size2);
			id->v3d_oom_block_used = 2;
		}*/else {
			v3d_write(id,V3D_INTDIS_OFFSET, 1 << 2);
			v3d_write(id,V3D_CT0CS_OFFSET , 0x8000);
			v3d_write(id,V3D_CT1CS_OFFSET , 0x8000);
			pr_err("v3d hw resetting for OOM");

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

	if((v3d_read(id,V3D_CT0CS_OFFSET)) | (v3d_read(id,V3D_CT1CS_OFFSET)) | (v3d_read(id,V3D_PCS_OFFSET)) ){
		return true;
		}
	return false;
}

mm_job_status_e v3d_start_job(void* device_id , mm_job_post_t* job)
{
	v3d_device_t* id = (v3d_device_t*)device_id;
	v3d_job_t* job_params = (v3d_job_t*)job->data;

	switch( job->status ){
		case MM_JOB_STATUS_READY:
			{
				v3d_reset(id);
				if (job->type == V3D_REND_JOB) {
					job->status = MM_JOB_STATUS_RUNNING;
					v3d_write(id,V3D_CT1CA_OFFSET,job_params->v3d_ct1ca);
					v3d_write(id,V3D_CT1EA_OFFSET,job_params->v3d_ct1ea);
					return MM_JOB_STATUS_RUNNING;
				} else if (job->type == V3D_BIN_REND_JOB){
					job->status = MM_JOB_STATUS_RUNNING;
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
				job->status = MM_JOB_STATUS_SUCCESS;
				return MM_JOB_STATUS_SUCCESS;
			}
		break;
		default:
			break;
		}
	return MM_JOB_STATUS_ERROR;
}
static v3d_device_t* v3d_device = NULL;

int __init v3d_init(void)
{
	int ret = 0;
	MM_FMWK_HW_IFC fmwk_param;
	MM_DVFS_HW_IFC dvfs_param;
	MM_PROF_HW_IFC prof_param;
	v3d_device = kmalloc(sizeof(v3d_device_t), GFP_KERNEL);
	v3d_device->vaddr = NULL;
	pr_debug("V3D driver Module Init");

#ifdef CONFIG_ION
	v3d_device->v3d_bin_oom_client = ion_client_create(idev, ION_DEFAULT_HEAP, "v3d");
	if (v3d_device->v3d_bin_oom_client == NULL) {
		ret = -ENOMEM;
		goto err;
	}

	v3d_device->v3d_bin_oom_handle = ion_alloc(v3d_device->v3d_bin_oom_client,
			V3D_BIN_OOM_SIZE, SZ_4K, ION_DEFAULT_HEAP);
	v3d_device->v3d_bin_oom_block = kona_ion_map_dma(v3d_device->v3d_bin_oom_client,
			v3d_device->v3d_bin_oom_handle);
	if (v3d_device->v3d_bin_oom_block == 0) {
		pr_err("ion alloc failed for v3d oom block size[0x%x] client[0x%x] handle[0x%x] \n",
		       V3D_BIN_OOM_SIZE, v3d_device->v3d_bin_oom_client, v3d_device->v3d_bin_oom_handle);
		v3d_device->v3d_bin_oom_size = 0;
		ret = -ENOMEM;
		goto err;
	}
	v3d_device->v3d_bin_oom_size = V3D_BIN_OOM_SIZE ;
	pr_info("v3d bin oom dma[0x%08x], size[0x%08x] \n",
	       v3d_device->v3d_bin_oom_block, v3d_device->v3d_bin_oom_size);

	v3d_device->v3d_bin_oom_handle2 = ion_alloc(v3d_device->v3d_bin_oom_client,
			V3D_BIN_OOM_SIZE, SZ_4K, ION_DEFAULT_HEAP);
	v3d_device->v3d_bin_oom_block2 = kona_ion_map_dma(v3d_device->v3d_bin_oom_client,
			v3d_device->v3d_bin_oom_handle2);
	if (v3d_device->v3d_bin_oom_block2 == 0) {
		pr_err("ion alloc failed for v3d oom block2 size[0x%x] client[0x%x] handle2[0x%x] \n",
		       V3D_BIN_OOM_SIZE, v3d_device->v3d_bin_oom_client, v3d_device->v3d_bin_oom_handle2);
		v3d_device->v3d_bin_oom_size2 = 0;
		ret = -ENOMEM;
		goto err;
	}
	v3d_device->v3d_bin_oom_size2 = V3D_BIN_OOM_SIZE ;
	pr_info("v3d bin oom 2 dma[0x%08x], size[0x%08x] \n",
	       v3d_device->v3d_bin_oom_block2, v3d_device->v3d_bin_oom_size2);

#else
	v3d_device->v3d_bin_oom_cpuaddr = kmalloc(V3D_BIN_OOM_SIZE,GFP_KERNEL);//dma_alloc_coherent(NULL, V3D_BIN_OOM_SIZE, &v3d_device->v3d_bin_oom_block, GFP_ATOMIC | GFP_DMA);
	if (v3d_device->v3d_bin_oom_cpuaddr == NULL) {
		pr_err("dma_alloc_coherent failed for v3d oom block size[0x%x]", v3d_device->v3d_bin_oom_size);
		v3d_device->v3d_bin_oom_block = 0;
		v3d_device->v3d_bin_oom_size = 0;
		ret = -ENOMEM;
		goto err;
	}
 	v3d_device->v3d_bin_oom_block = virt_to_phys(v3d_device->v3d_bin_oom_cpuaddr);
	v3d_device->v3d_bin_oom_size = V3D_BIN_OOM_SIZE ;
	pr_debug("v3d bin oom phys[0x%08x], size[0x%08x] cpuaddr[0x%08x]",
		v3d_device->v3d_bin_oom_block,v3d_device->v3d_bin_oom_size, (int)v3d_device->v3d_bin_oom_cpuaddr);

	v3d_device->v3d_bin_oom_cpuaddr2 = kmalloc(V3D_BIN_OOM_SIZE,GFP_KERNEL);//dma_alloc_coherent(NULL, V3D_BIN_OOM_SIZE, &v3d_device->v3d_bin_oom_block, GFP_ATOMIC | GFP_DMA);
	if (v3d_device->v3d_bin_oom_cpuaddr2 == NULL) {
		pr_err("dma_alloc_coherent failed for v3d oom block size[0x%x]", v3d_device->v3d_bin_oom_size2);
		v3d_device->v3d_bin_oom_block2 = 0;
		v3d_device->v3d_bin_oom_size2 = 0;
		ret = -ENOMEM;
		goto err;
	}
 	v3d_device->v3d_bin_oom_block = virt_to_phys(v3d_device->v3d_bin_oom_cpuaddr2);
	v3d_device->v3d_bin_oom_size2 = V3D_BIN_OOM_SIZE;
	pr_debug("v3d bin oom2 phys[0x%08x], size[0x%08x] cpuaddr[0x%08x]",
		v3d_device->v3d_bin_oom_block2, v3d_device->v3d_bin_oom_size2, (int)v3d_device->v3d_bin_oom_cpuaddr2);
#endif

	fmwk_param.mm_dev_name = V3D_DEV_NAME;
	fmwk_param.mm_dev_clk_name = V3D_AXI_BUS_CLK_NAME_STR;
	fmwk_param.mm_dev_base_addr = MM_V3D_BASE_ADDR;
	fmwk_param.mm_dev_hw_size = V3D_HW_SIZE;
	fmwk_param.mm_dev_irq = IRQ_V3D;

	fmwk_param.mm_dev_timer = DEFAULT_MM_DEV_TIMER_MS;
	fmwk_param.mm_dev_timeout = DEFAULT_MM_DEV_TIMEOUT_MS;
	
	fmwk_param.mm_dev_get_status = get_v3d_status;
	fmwk_param.mm_dev_start_job = v3d_start_job;
	fmwk_param.mm_dev_process_irq = process_v3d_irq;
	fmwk_param.mm_dev_init = v3d_reset;
	fmwk_param.mm_dev_deinit = v3d_reset;
	fmwk_param.mm_dev_abort = v3d_abort;
	fmwk_param.mm_dev_print_regs = v3d_print_regs;
	fmwk_param.mm_device_id = (void *)v3d_device;
	fmwk_param.mm_dev_virt_addr = NULL;

	dvfs_param.is_dvfs_on = 1;
	dvfs_param.user_requested_mode = TURBO;
	dvfs_param.enable_suspend_resume = 0;
	dvfs_param.T1 = 300;
	dvfs_param.P1 = 80;
	dvfs_param.T2 = 3000;
	dvfs_param.P2 = 30;
	dvfs_param.dvfs_bulk_job_cnt = 0;

	v3d_device->fmwk_handle = mm_fmwk_register(&fmwk_param,&dvfs_param,&prof_param);
	/* get kva from fmwk */
	v3d_device->vaddr = fmwk_param.mm_dev_virt_addr;

	if( (v3d_device->fmwk_handle == NULL) ||
		(v3d_device->vaddr == NULL )) {
		ret = -ENOMEM;
		goto err;
		}
	pr_debug("V3D driver Module Init over");
	return ret;
	
err:
	pr_err("V3D driver Module Init Error");
	return ret;
}

void __exit v3d_exit(void)
{
	pr_debug("V3D driver Module Exit");
	if(v3d_device->fmwk_handle)
		mm_fmwk_unregister(v3d_device->fmwk_handle);
#ifdef CONFIG_ION
	if (v3d_device->v3d_bin_oom_block2)
		ion_unmap_dma(v3d_device->v3d_bin_oom_client, v3d_device->v3d_bin_oom_handle2);
	if (v3d_device->v3d_bin_oom_handle2)
		ion_free(v3d_device->v3d_bin_oom_client, v3d_device->v3d_bin_oom_handle2);
	if (v3d_device->v3d_bin_oom_block)
		ion_unmap_dma(v3d_device->v3d_bin_oom_client, v3d_device->v3d_bin_oom_handle);
	if (v3d_device->v3d_bin_oom_handle)
		ion_free(v3d_device->v3d_bin_oom_client, v3d_device->v3d_bin_oom_handle);
	if (v3d_device->v3d_bin_oom_client)
		ion_client_destroy(v3d_device->v3d_bin_oom_client);
#else
	if (v3d_device->v3d_bin_oom_cpuaddr2)
		dma_free_coherent(NULL, v3d_device->v3d_bin_oom_size2, v3d_device->v3d_bin_oom_cpuaddr2, v3d_device->v3d_bin_oom_block2);
	if (v3d_device->v3d_bin_oom_cpuaddr)
		dma_free_coherent(NULL, v3d_device->v3d_bin_oom_size, v3d_device->v3d_bin_oom_cpuaddr, v3d_device->v3d_bin_oom_block);
#endif
	kfree(v3d_device);
}


module_init(v3d_init);
module_exit(v3d_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("V3D device driver");
MODULE_LICENSE("GPL");
