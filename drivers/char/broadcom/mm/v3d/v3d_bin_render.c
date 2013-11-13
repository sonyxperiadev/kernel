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
#define pr_fmt(fmt) "v3d_bin_render: " fmt

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
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/dma-mapping.h>

#include "v3d_bin_render.h"
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <mach/rdb/brcm_rdb_mm_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_v3d.h>
#include <linux/broadcom/mm_fw_hw_ifc.h>
#include <linux/broadcom/mm_fw_usr_ifc.h>
#ifdef CONFIG_ION
#include <linux/broadcom/bcm_ion.h>
#endif

#define V3D_HW_SIZE (1024*4)

#define IRQ_V3D	BCM_INT_ID_RESERVED148
#define V3D_BIN_OOM_SIZE (1024*1024)


#define v3d_boom_t struct _v3d_boom_t


struct _v3d_boom_t {
	struct list_head node;
	struct ion_handle *v3d_bin_oom_handle;
	int v3d_bin_oom_block;
	int v3d_bin_oom_size ;
};

unsigned int *job_va;
unsigned int job_pa;

struct _v3d_bin_render_device_t {
#ifdef CONFIG_ION
	struct ion_client *v3d_bin_oom_client;
	v3d_boom_t *mem_block;
	v3d_boom_t *client_block;
	struct list_head mem_head;
#endif
	void __iomem *vaddr;
};


#define v3d_bin_render_device_t struct _v3d_bin_render_device_t

static inline void v3d_write(v3d_bin_render_device_t *v3d,
				unsigned int reg, unsigned int value)
{
	mm_write_reg((void *)v3d->vaddr, reg, value);
}

static inline unsigned int v3d_read(v3d_bin_render_device_t *v3d,
					unsigned int reg)
{
	return mm_read_reg((void *)v3d->vaddr, reg);
}

static v3d_boom_t *v3d_alloc_boom(void *device_id)
{
	v3d_bin_render_device_t *id = (v3d_bin_render_device_t *)device_id;
	v3d_boom_t *block = kzalloc(sizeof(v3d_boom_t),
				GFP_KERNEL);
	unsigned int heap_mask;

	if (!block)
		return NULL;

	heap_mask = bcm_ion_get_heapmask(ION_FLAG_256M | ION_FLAG_FAST_ALLOC);
	block->v3d_bin_oom_handle = ion_alloc(id->v3d_bin_oom_client,
				V3D_BIN_OOM_SIZE, 0, heap_mask, 0);
	block->v3d_bin_oom_block = bcm_ion_map_dma(
			id->v3d_bin_oom_client,
			block->v3d_bin_oom_handle);
	if (block->v3d_bin_oom_block == 0) {
		pr_err("ion alloc failed for v3d oom block size[0x%x] client[%p] handle[%p]\n",
			V3D_BIN_OOM_SIZE,
			id->v3d_bin_oom_client,
			block->v3d_bin_oom_handle);
			goto err;
			}
	block->v3d_bin_oom_size = V3D_BIN_OOM_SIZE ;

	INIT_LIST_HEAD(&block->node);
	list_add_tail(&block->node, &id->mem_head);

	pr_debug("v3d_alloc_boom %x %x\n",
		block->v3d_bin_oom_block, block->v3d_bin_oom_size);
	return block;

err:
	if (block->v3d_bin_oom_handle)
		ion_free(id->v3d_bin_oom_client, block->v3d_bin_oom_handle);

		kfree(block);
	return NULL;
}

static void v3d_free_boom(void *device_id, v3d_boom_t *block)
{
	v3d_bin_render_device_t *id = (v3d_bin_render_device_t *)device_id;
	if (block) {
		list_del_init(&block->node);
		pr_debug("v3d_free_boom %x %x\n",
			block->v3d_bin_oom_block, block->v3d_bin_oom_size);
		if (block->v3d_bin_oom_handle)
			ion_free(id->v3d_bin_oom_client,
			block->v3d_bin_oom_handle);
		kfree(block);
		}
}

static int v3d_bin_render_reset(void *device_id)
{
	v3d_bin_render_device_t *id = (v3d_bin_render_device_t *)device_id;
	v3d_boom_t *mem = NULL;
	v3d_boom_t *temp_mem = NULL;
	v3d_write(id, V3D_CT0CS_OFFSET, 0x8000);
	v3d_write(id, V3D_CT1CS_OFFSET, 0x8000);

	v3d_write(id, V3D_L2CACTL_OFFSET, 4);
	v3d_write(id, V3D_L2CACTL_OFFSET, 1);
	v3d_write(id, V3D_L2CACTL_OFFSET, 4);

	v3d_write(id, V3D_CT0CS_OFFSET, 0x8000);
	v3d_write(id, V3D_CT1CS_OFFSET, 0x8000);

	v3d_write(id, V3D_RFC_OFFSET, 1);
	v3d_write(id, V3D_BFC_OFFSET, 1);
	v3d_write(id, V3D_SLCACTL_OFFSET, 0x0f0f0f0f);
	v3d_write(id, V3D_VPACNTL_OFFSET, 0);
	v3d_write(id, V3D_BPOA_OFFSET, id->mem_block->v3d_bin_oom_block);
	v3d_write(id, V3D_BPOS_OFFSET, id->mem_block->v3d_bin_oom_size);
	v3d_write(id, V3D_INTCTL_OFFSET, 0xF);
	v3d_write(id, V3D_INTENA_OFFSET, 0x7);

	list_for_each_entry_safe(mem, temp_mem, &(id->mem_head), node) {
		v3d_free_boom(id, mem);
		}
	return 0;
}

static int v3d_bin_render_abort(void *device_id, mm_job_post_t *job)
{
	v3d_bin_render_device_t *id = (v3d_bin_render_device_t *)device_id;
	v3d_job_t *job_params = (v3d_job_t *)job->data;

	pr_err("v3d job [%x %x], [%x %x]\n", job_params->v3d_ct0ca,
	job_params->v3d_ct0ea, job_params->v3d_ct1ca, job_params->v3d_ct1ea);

	pr_err("regs:(0x%08x 0x%08x 0x%08x 0x%08x) (0x%08x 0x%08x 0x%08x 0x%08x) (0x%08x 0x%08x 0x%08x)",
		v3d_read(id, V3D_CT0CS_OFFSET),
		v3d_read(id, V3D_CT00RA0_OFFSET),
		v3d_read(id, V3D_CT0CA_OFFSET),
		v3d_read(id, V3D_CT0EA_OFFSET),
		v3d_read(id, V3D_CT1CS_OFFSET),
		v3d_read(id, V3D_CT01RA0_OFFSET),
		v3d_read(id, V3D_CT1CA_OFFSET), v3d_read(id, V3D_CT1EA_OFFSET),
		v3d_read(id, V3D_BPOA_OFFSET), v3d_read(id, V3D_BPOS_OFFSET),
		v3d_read(id, V3D_PCS_OFFSET));

	v3d_write(id, V3D_CT0CS_OFFSET, 0x20);
	v3d_write(id, V3D_CT1CS_OFFSET, 0x20);
	v3d_write(id, V3D_CT0CA_OFFSET, 0);
	v3d_write(id, V3D_CT0EA_OFFSET, 0);
	v3d_write(id, V3D_CT1CA_OFFSET, 0);
	v3d_write(id, V3D_CT1EA_OFFSET, 0);

	v3d_write(id, V3D_CT0CS_OFFSET, 0x8000);
	v3d_write(id, V3D_CT1CS_OFFSET, 0x8000);

	v3d_bin_render_reset(id);
	return 0;
}

static mm_isr_type_e process_v3d_bin_render_irq(void *device_id)
{
	u32 flags, flags_qpu, tmp;
	mm_isr_type_e irq_retval = MM_ISR_UNKNOWN;
	v3d_bin_render_device_t *id = (v3d_bin_render_device_t *)device_id;

	/* Read the interrupt status registers */
	flags = v3d_read(id, V3D_INTCTL_OFFSET);
	flags_qpu = v3d_read(id, V3D_DBQITC_OFFSET);

	/* Clear interrupts isr is going to handle */
	tmp = flags & v3d_read(id, V3D_INTENA_OFFSET);
	v3d_write(id, V3D_INTCTL_OFFSET, tmp);
	if (flags_qpu)
		v3d_write(id, V3D_DBQITC_OFFSET, flags_qpu);

	/* Handle Binning Interrupt*/
	if (flags & 2)
		irq_retval = MM_ISR_PROCESSED;

	/* Handle Rendering Interrupt*/
	if (flags & 1)
		irq_retval = MM_ISR_SUCCESS;

	/* Handle oom case */
	if (flags & (1 << 2)) {
		irq_retval = MM_ISR_SUCCESS;
		v3d_write(id, V3D_INTDIS_OFFSET, 1 << 2);
		pr_debug("request boom from isr\n");
	}
	return irq_retval;
}

bool get_v3d_bin_render_status(void *device_id)
{
	v3d_bin_render_device_t *id = (v3d_bin_render_device_t *)device_id;
	u32 flags;

	/* Read the interrupt status registers */
	flags = v3d_read(id, V3D_INTCTL_OFFSET);

	/* Handle oom case */
	if (flags & (1 << 2)) {
		v3d_boom_t *block =
		v3d_alloc_boom(id);
		if (block) {
			v3d_write(id, V3D_BPOA_OFFSET,
			block->v3d_bin_oom_block);
			v3d_write(id, V3D_BPOS_OFFSET, block->v3d_bin_oom_size);
			v3d_write(id, V3D_INTCTL_OFFSET, 1 << 2);
			v3d_write(id, V3D_INTENA_OFFSET, 1 << 2);
			pr_debug("supply boom from get_v3d_status %x %x\n",
					block->v3d_bin_oom_block,
					block->v3d_bin_oom_size);
			return true;
			}
		else {
			v3d_write(id, V3D_INTDIS_OFFSET, 1 << 2);
			v3d_write(id, V3D_CT0CS_OFFSET , 0x8000);
			v3d_write(id, V3D_CT1CS_OFFSET , 0x8000);
			pr_err("v3d hw resetting for OOM");
			}
		}

	if ((v3d_read(id, V3D_CT0CS_OFFSET)&(0x20)) ||
		(v3d_read(id, V3D_CT1CS_OFFSET)&(0x20)) ||
		(v3d_read(id, V3D_PCS_OFFSET)))
		return true;
	return false;
}

mm_job_status_e v3d_bin_render_start_job(void *device_id,
		mm_job_post_t *job, unsigned int profmask)
{
	v3d_bin_render_device_t *id = (v3d_bin_render_device_t *)device_id;
	v3d_job_t *job_params = (v3d_job_t *)job->data;

	switch (job->status) {
	case MM_JOB_STATUS_READY:
		{
			v3d_bin_render_reset(id);
			job_va[0] = job->type;
			job_va[1] = job_params->v3d_ct0ca;
			job_va[2] = job_params->v3d_ct0ea;
			job_va[3] = job_params->v3d_ct1ca;
			job_va[4] = job_params->v3d_ct1ea;
			if (job->type == V3D_REND_JOB) {
				job->status = MM_JOB_STATUS_RUNNING;
				v3d_write(id, V3D_CT1CA_OFFSET,
				job_params->v3d_ct1ca);
				v3d_write(id, V3D_CT1EA_OFFSET,
				job_params->v3d_ct1ea);
				return MM_JOB_STATUS_RUNNING;
			} else if (job->type == V3D_BIN_REND_JOB) {
				job->status = MM_JOB_STATUS_RUNNING;
				if (job_params->v3d_ct0ca !=
					job_params->v3d_ct0ea) {
					v3d_write(id, V3D_CT0CA_OFFSET,
					job_params->v3d_ct0ca);
					v3d_write(id, V3D_CT0EA_OFFSET,
					job_params->v3d_ct0ea);
					}
				v3d_write(id, V3D_CT1CA_OFFSET,
				job_params->v3d_ct1ca);
				v3d_write(id, V3D_CT1EA_OFFSET,
				job_params->v3d_ct1ea);
				pr_debug("v3d_ct0ca = %x," \
					"v3d_ct0ea = %x, v3d_ct1ca = %x," \
					"v3d_ct1ea = %x", job_params->v3d_ct0ca,
					job_params->v3d_ct0ea,
					job_params->v3d_ct1ca,
					job_params->v3d_ct1ea);
				pr_debug("supply boom from v3d_start_job %x %x\n",
					id->mem_block->v3d_bin_oom_block,
					id->mem_block->v3d_bin_oom_size);
				return MM_JOB_STATUS_RUNNING;
			}
		}
	break;
	case MM_JOB_STATUS_RUNNING:
		{
			job_va[0] = 0;
			job_va[1] = 0;
			job_va[2] = 0;
			job_va[3] = 0;
			job_va[4] = 0;
			if ((v3d_read(id, V3D_CT0CA_OFFSET) !=
				v3d_read(id, V3D_CT0EA_OFFSET)) ||
				(v3d_read(id, V3D_CT1CA_OFFSET) !=
				v3d_read(id, V3D_CT1EA_OFFSET))) {
				pr_err("v3d job [%x %x], [%x %x]\n",
				job_params->v3d_ct0ca,
				job_params->v3d_ct0ea, job_params->v3d_ct1ca,
				job_params->v3d_ct1ea);

				pr_err("regs:(0x%08x 0x%08x 0x%08x 0x%08x) "\
					"(0x%08x 0x%08x 0x%08x 0x%08x) (0x%08x 0x%08x 0x%08x)",
				v3d_read(id, V3D_CT0CS_OFFSET),
				v3d_read(id, V3D_CT00RA0_OFFSET),
				v3d_read(id, V3D_CT0CA_OFFSET),
				v3d_read(id, V3D_CT0EA_OFFSET),
				v3d_read(id, V3D_CT1CS_OFFSET),
				v3d_read(id, V3D_CT01RA0_OFFSET),
				v3d_read(id, V3D_CT1CA_OFFSET),
				v3d_read(id, V3D_CT1EA_OFFSET),
				v3d_read(id, V3D_BPOA_OFFSET),
				v3d_read(id, V3D_BPOS_OFFSET),
				v3d_read(id, V3D_PCS_OFFSET));
				}
			job->status = MM_JOB_STATUS_SUCCESS;
			return MM_JOB_STATUS_SUCCESS;
		}
	break;
	default:
		break;
	}

pr_err("v3d_bin_render_start_job :: job type = %x job status = %x\n",
job->type, job->status);
	return MM_JOB_STATUS_ERROR;
}

static v3d_bin_render_device_t *v3d_device;

void v3d_bin_render_update_virt(void *virt)
{
pr_debug("v3d_bin_render_update_virt:\n");
	v3d_device->vaddr = virt;
}

void v3d_bin_render_deinit(void)
{
	pr_debug("V3D bin_render driver Module Exit");
#ifdef CONFIG_ION
	if (v3d_device->mem_block)
		v3d_free_boom(v3d_device, v3d_device->mem_block);
	if (v3d_device->client_block)
		v3d_free_boom(v3d_device, v3d_device->client_block);
	if (v3d_device->v3d_bin_oom_client)
		ion_client_destroy(v3d_device->v3d_bin_oom_client);
#endif
	kfree(v3d_device);
}

static inline unsigned int extract(unsigned int reg,
				   unsigned int ms,
				   unsigned int ls)
{
	return (reg & (0xFFFFFFFF >> (31 - ms)) & (0xFFFFFFFF << ls)) >> ls;
}

int v3d_bin_render_version_init(void *device_id, void *vaddr,
			mm_version_info_t *version_info)
{
	uint32_t V3D_IDENT1, V3D_IDENT2;
	struct v3d_version_info_t *vinfo;
	v3d_bin_render_device_t *id = (v3d_bin_render_device_t *)device_id;
	version_info->size = sizeof(struct v3d_version_info_t);
	version_info->version_info_ptr =
			kzalloc(sizeof(struct v3d_version_info_t), GFP_KERNEL);
	vinfo = (struct v3d_version_info_t *)version_info->version_info_ptr;
	vinfo->v3d_technology_version =
			mm_read_reg(vaddr, V3D_IDENT0_OFFSET) >> 24;
	V3D_IDENT1 = mm_read_reg(vaddr, V3D_IDENT1_OFFSET);
	vinfo->vpm_size = extract(V3D_IDENT1, 31, 28);
	vinfo->hdr_support = extract(V3D_IDENT1, 27, 24);
	vinfo->nsem = extract(V3D_IDENT1, 23, 16);
	vinfo->tpus = extract(V3D_IDENT1, 15, 12);
	vinfo->qpus = extract(V3D_IDENT1, 11, 8);
	vinfo->nslc = extract(V3D_IDENT1, 7, 4);
	vinfo->rev = extract(V3D_IDENT1, 3, 0);
	V3D_IDENT2 = mm_read_reg(vaddr, V3D_IDENT2_OFFSET);
	vinfo->tlbdb = extract(V3D_IDENT2, 11, 8);
	vinfo->tlbsz = TLB_QUARTER_BUFFER_SIZE + extract(V3D_IDENT2, 7, 4);
	vinfo->vrisz = VRI_HALF_SIZE + extract(V3D_IDENT2, 3, 0);
	vinfo->bin_mem_addr = id->client_block->v3d_bin_oom_block;
	vinfo->bin_mem_size = V3D_BIN_OOM_SIZE;
	return 0;
}


int v3d_bin_render_init(MM_CORE_HW_IFC *core_param)
{
	int ret = 0;

	v3d_device = kmalloc(sizeof(v3d_bin_render_device_t), GFP_KERNEL);
	if (v3d_device == NULL) {
		pr_err("v3d_init: kmalloc failed\n");
		ret = -ENOMEM;
		goto err;
	}

	v3d_device->vaddr = NULL;
	pr_debug("v3d_bin_render_init: -->\n");

#ifdef CONFIG_ION
	BUG_ON(!idev);
	v3d_device->v3d_bin_oom_client = ion_client_create(idev, "v3d");
	if (v3d_device->v3d_bin_oom_client == NULL) {
		ret = -ENOMEM;
		goto err1;
	}
	INIT_LIST_HEAD(&v3d_device->mem_head);

	v3d_device->mem_block = v3d_alloc_boom(v3d_device);
	list_del_init(&v3d_device->mem_block->node);

	v3d_device->client_block = v3d_alloc_boom(v3d_device);
	list_del_init(&v3d_device->client_block->node);

#else
#error "V3D Driver Cannot work without ION"
#endif

	core_param->mm_base_addr = MM_V3D_BASE_ADDR;
	core_param->mm_hw_size = V3D_HW_SIZE;
	core_param->mm_irq = IRQ_V3D;

	core_param->mm_timer = DEFAULT_MM_DEV_TIMER_MS;
	core_param->mm_timeout = DEFAULT_MM_DEV_TIMEOUT_MS;

	core_param->mm_get_status = get_v3d_bin_render_status;
	core_param->mm_start_job = v3d_bin_render_start_job;
	core_param->mm_process_irq = process_v3d_bin_render_irq;
	core_param->mm_init = v3d_bin_render_reset;
	core_param->mm_deinit = v3d_bin_render_reset;
	core_param->mm_abort = v3d_bin_render_abort;
	core_param->mm_get_regs = NULL;
	core_param->mm_device_id = (void *)v3d_device;
	core_param->mm_virt_addr = NULL;
	core_param->mm_version_init = v3d_bin_render_version_init;
	core_param->mm_update_virt_addr = v3d_bin_render_update_virt;
	job_va = (unsigned int *)dma_alloc_coherent(NULL, 32, &job_pa, GFP_DMA);
	pr_err("v3d_init job va = %p job pa = %x", job_va, job_pa);
	return ret;

err1:
	kfree(v3d_device);
err:
	pr_err("V3D bin_render driver Module Init Error");
	return ret;
}
