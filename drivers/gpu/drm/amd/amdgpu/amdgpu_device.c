/*
 * Copyright 2008 Advanced Micro Devices, Inc.
 * Copyright 2008 Red Hat Inc.
 * Copyright 2009 Jerome Glisse.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Dave Airlie
 *          Alex Deucher
 *          Jerome Glisse
 */
#include <linux/kthread.h>
#include <linux/console.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <drm/drmP.h>
#include <drm/drm_crtc_helper.h>
#include <drm/amdgpu_drm.h>
#include <linux/vgaarb.h>
#include <linux/vga_switcheroo.h>
#include <linux/efi.h>
#include "amdgpu.h"
#include "amdgpu_trace.h"
#include "amdgpu_i2c.h"
#include "atom.h"
#include "amdgpu_atombios.h"
#include "amdgpu_atomfirmware.h"
#include "amd_pcie.h"
#ifdef CONFIG_DRM_AMDGPU_SI
#include "si.h"
#endif
#ifdef CONFIG_DRM_AMDGPU_CIK
#include "cik.h"
#endif
#include "vi.h"
#include "soc15.h"
#include "bif/bif_4_1_d.h"
#include <linux/pci.h>
#include <linux/firmware.h>
#include "amdgpu_vf_error.h"

#include "amdgpu_amdkfd.h"

MODULE_FIRMWARE("amdgpu/vega10_gpu_info.bin");
MODULE_FIRMWARE("amdgpu/raven_gpu_info.bin");

#define AMDGPU_RESUME_MS		2000

static int amdgpu_debugfs_regs_init(struct amdgpu_device *adev);
static void amdgpu_debugfs_regs_cleanup(struct amdgpu_device *adev);
static int amdgpu_debugfs_test_ib_ring_init(struct amdgpu_device *adev);

static const char *amdgpu_asic_name[] = {
	"TAHITI",
	"PITCAIRN",
	"VERDE",
	"OLAND",
	"HAINAN",
	"BONAIRE",
	"KAVERI",
	"KABINI",
	"HAWAII",
	"MULLINS",
	"TOPAZ",
	"TONGA",
	"FIJI",
	"CARRIZO",
	"STONEY",
	"POLARIS10",
	"POLARIS11",
	"POLARIS12",
	"VEGA10",
	"RAVEN",
	"LAST",
};

bool amdgpu_device_is_px(struct drm_device *dev)
{
	struct amdgpu_device *adev = dev->dev_private;

	if (adev->flags & AMD_IS_PX)
		return true;
	return false;
}

/*
 * MMIO register access helper functions.
 */
uint32_t amdgpu_mm_rreg(struct amdgpu_device *adev, uint32_t reg,
			uint32_t acc_flags)
{
	uint32_t ret;

	if (!(acc_flags & AMDGPU_REGS_NO_KIQ) && amdgpu_sriov_runtime(adev)) {
		BUG_ON(in_interrupt());
		return amdgpu_virt_kiq_rreg(adev, reg);
	}

	if ((reg * 4) < adev->rmmio_size && !(acc_flags & AMDGPU_REGS_IDX))
		ret = readl(((void __iomem *)adev->rmmio) + (reg * 4));
	else {
		unsigned long flags;

		spin_lock_irqsave(&adev->mmio_idx_lock, flags);
		writel((reg * 4), ((void __iomem *)adev->rmmio) + (mmMM_INDEX * 4));
		ret = readl(((void __iomem *)adev->rmmio) + (mmMM_DATA * 4));
		spin_unlock_irqrestore(&adev->mmio_idx_lock, flags);
	}
	trace_amdgpu_mm_rreg(adev->pdev->device, reg, ret);
	return ret;
}

void amdgpu_mm_wreg(struct amdgpu_device *adev, uint32_t reg, uint32_t v,
		    uint32_t acc_flags)
{
	trace_amdgpu_mm_wreg(adev->pdev->device, reg, v);

	if (adev->asic_type >= CHIP_VEGA10 && reg == 0) {
		adev->last_mm_index = v;
	}

	if (!(acc_flags & AMDGPU_REGS_NO_KIQ) && amdgpu_sriov_runtime(adev)) {
		BUG_ON(in_interrupt());
		return amdgpu_virt_kiq_wreg(adev, reg, v);
	}

	if ((reg * 4) < adev->rmmio_size && !(acc_flags & AMDGPU_REGS_IDX))
		writel(v, ((void __iomem *)adev->rmmio) + (reg * 4));
	else {
		unsigned long flags;

		spin_lock_irqsave(&adev->mmio_idx_lock, flags);
		writel((reg * 4), ((void __iomem *)adev->rmmio) + (mmMM_INDEX * 4));
		writel(v, ((void __iomem *)adev->rmmio) + (mmMM_DATA * 4));
		spin_unlock_irqrestore(&adev->mmio_idx_lock, flags);
	}

	if (adev->asic_type >= CHIP_VEGA10 && reg == 1 && adev->last_mm_index == 0x5702C) {
		udelay(500);
	}
}

u32 amdgpu_io_rreg(struct amdgpu_device *adev, u32 reg)
{
	if ((reg * 4) < adev->rio_mem_size)
		return ioread32(adev->rio_mem + (reg * 4));
	else {
		iowrite32((reg * 4), adev->rio_mem + (mmMM_INDEX * 4));
		return ioread32(adev->rio_mem + (mmMM_DATA * 4));
	}
}

void amdgpu_io_wreg(struct amdgpu_device *adev, u32 reg, u32 v)
{
	if (adev->asic_type >= CHIP_VEGA10 && reg == 0) {
		adev->last_mm_index = v;
	}

	if ((reg * 4) < adev->rio_mem_size)
		iowrite32(v, adev->rio_mem + (reg * 4));
	else {
		iowrite32((reg * 4), adev->rio_mem + (mmMM_INDEX * 4));
		iowrite32(v, adev->rio_mem + (mmMM_DATA * 4));
	}

	if (adev->asic_type >= CHIP_VEGA10 && reg == 1 && adev->last_mm_index == 0x5702C) {
		udelay(500);
	}
}

/**
 * amdgpu_mm_rdoorbell - read a doorbell dword
 *
 * @adev: amdgpu_device pointer
 * @index: doorbell index
 *
 * Returns the value in the doorbell aperture at the
 * requested doorbell index (CIK).
 */
u32 amdgpu_mm_rdoorbell(struct amdgpu_device *adev, u32 index)
{
	if (index < adev->doorbell.num_doorbells) {
		return readl(adev->doorbell.ptr + index);
	} else {
		DRM_ERROR("reading beyond doorbell aperture: 0x%08x!\n", index);
		return 0;
	}
}

/**
 * amdgpu_mm_wdoorbell - write a doorbell dword
 *
 * @adev: amdgpu_device pointer
 * @index: doorbell index
 * @v: value to write
 *
 * Writes @v to the doorbell aperture at the
 * requested doorbell index (CIK).
 */
void amdgpu_mm_wdoorbell(struct amdgpu_device *adev, u32 index, u32 v)
{
	if (index < adev->doorbell.num_doorbells) {
		writel(v, adev->doorbell.ptr + index);
	} else {
		DRM_ERROR("writing beyond doorbell aperture: 0x%08x!\n", index);
	}
}

/**
 * amdgpu_mm_rdoorbell64 - read a doorbell Qword
 *
 * @adev: amdgpu_device pointer
 * @index: doorbell index
 *
 * Returns the value in the doorbell aperture at the
 * requested doorbell index (VEGA10+).
 */
u64 amdgpu_mm_rdoorbell64(struct amdgpu_device *adev, u32 index)
{
	if (index < adev->doorbell.num_doorbells) {
		return atomic64_read((atomic64_t *)(adev->doorbell.ptr + index));
	} else {
		DRM_ERROR("reading beyond doorbell aperture: 0x%08x!\n", index);
		return 0;
	}
}

/**
 * amdgpu_mm_wdoorbell64 - write a doorbell Qword
 *
 * @adev: amdgpu_device pointer
 * @index: doorbell index
 * @v: value to write
 *
 * Writes @v to the doorbell aperture at the
 * requested doorbell index (VEGA10+).
 */
void amdgpu_mm_wdoorbell64(struct amdgpu_device *adev, u32 index, u64 v)
{
	if (index < adev->doorbell.num_doorbells) {
		atomic64_set((atomic64_t *)(adev->doorbell.ptr + index), v);
	} else {
		DRM_ERROR("writing beyond doorbell aperture: 0x%08x!\n", index);
	}
}

/**
 * amdgpu_invalid_rreg - dummy reg read function
 *
 * @adev: amdgpu device pointer
 * @reg: offset of register
 *
 * Dummy register read function.  Used for register blocks
 * that certain asics don't have (all asics).
 * Returns the value in the register.
 */
static uint32_t amdgpu_invalid_rreg(struct amdgpu_device *adev, uint32_t reg)
{
	DRM_ERROR("Invalid callback to read register 0x%04X\n", reg);
	BUG();
	return 0;
}

/**
 * amdgpu_invalid_wreg - dummy reg write function
 *
 * @adev: amdgpu device pointer
 * @reg: offset of register
 * @v: value to write to the register
 *
 * Dummy register read function.  Used for register blocks
 * that certain asics don't have (all asics).
 */
static void amdgpu_invalid_wreg(struct amdgpu_device *adev, uint32_t reg, uint32_t v)
{
	DRM_ERROR("Invalid callback to write register 0x%04X with 0x%08X\n",
		  reg, v);
	BUG();
}

/**
 * amdgpu_block_invalid_rreg - dummy reg read function
 *
 * @adev: amdgpu device pointer
 * @block: offset of instance
 * @reg: offset of register
 *
 * Dummy register read function.  Used for register blocks
 * that certain asics don't have (all asics).
 * Returns the value in the register.
 */
static uint32_t amdgpu_block_invalid_rreg(struct amdgpu_device *adev,
					  uint32_t block, uint32_t reg)
{
	DRM_ERROR("Invalid callback to read register 0x%04X in block 0x%04X\n",
		  reg, block);
	BUG();
	return 0;
}

/**
 * amdgpu_block_invalid_wreg - dummy reg write function
 *
 * @adev: amdgpu device pointer
 * @block: offset of instance
 * @reg: offset of register
 * @v: value to write to the register
 *
 * Dummy register read function.  Used for register blocks
 * that certain asics don't have (all asics).
 */
static void amdgpu_block_invalid_wreg(struct amdgpu_device *adev,
				      uint32_t block,
				      uint32_t reg, uint32_t v)
{
	DRM_ERROR("Invalid block callback to write register 0x%04X in block 0x%04X with 0x%08X\n",
		  reg, block, v);
	BUG();
}

static int amdgpu_vram_scratch_init(struct amdgpu_device *adev)
{
	return amdgpu_bo_create_kernel(adev, AMDGPU_GPU_PAGE_SIZE,
				       PAGE_SIZE, AMDGPU_GEM_DOMAIN_VRAM,
				       &adev->vram_scratch.robj,
				       &adev->vram_scratch.gpu_addr,
				       (void **)&adev->vram_scratch.ptr);
}

static void amdgpu_vram_scratch_fini(struct amdgpu_device *adev)
{
	amdgpu_bo_free_kernel(&adev->vram_scratch.robj, NULL, NULL);
}

/**
 * amdgpu_program_register_sequence - program an array of registers.
 *
 * @adev: amdgpu_device pointer
 * @registers: pointer to the register array
 * @array_size: size of the register array
 *
 * Programs an array or registers with and and or masks.
 * This is a helper for setting golden registers.
 */
void amdgpu_program_register_sequence(struct amdgpu_device *adev,
				      const u32 *registers,
				      const u32 array_size)
{
	u32 tmp, reg, and_mask, or_mask;
	int i;

	if (array_size % 3)
		return;

	for (i = 0; i < array_size; i +=3) {
		reg = registers[i + 0];
		and_mask = registers[i + 1];
		or_mask = registers[i + 2];

		if (and_mask == 0xffffffff) {
			tmp = or_mask;
		} else {
			tmp = RREG32(reg);
			tmp &= ~and_mask;
			tmp |= or_mask;
		}
		WREG32(reg, tmp);
	}
}

void amdgpu_pci_config_reset(struct amdgpu_device *adev)
{
	pci_write_config_dword(adev->pdev, 0x7c, AMDGPU_ASIC_RESET_DATA);
}

/*
 * GPU doorbell aperture helpers function.
 */
/**
 * amdgpu_doorbell_init - Init doorbell driver information.
 *
 * @adev: amdgpu_device pointer
 *
 * Init doorbell driver information (CIK)
 * Returns 0 on success, error on failure.
 */
static int amdgpu_doorbell_init(struct amdgpu_device *adev)
{
	/* doorbell bar mapping */
	adev->doorbell.base = pci_resource_start(adev->pdev, 2);
	adev->doorbell.size = pci_resource_len(adev->pdev, 2);

	adev->doorbell.num_doorbells = min_t(u32, adev->doorbell.size / sizeof(u32),
					     AMDGPU_DOORBELL_MAX_ASSIGNMENT+1);
	if (adev->doorbell.num_doorbells == 0)
		return -EINVAL;

	adev->doorbell.ptr = ioremap(adev->doorbell.base,
				     adev->doorbell.num_doorbells *
				     sizeof(u32));
	if (adev->doorbell.ptr == NULL)
		return -ENOMEM;

	return 0;
}

/**
 * amdgpu_doorbell_fini - Tear down doorbell driver information.
 *
 * @adev: amdgpu_device pointer
 *
 * Tear down doorbell driver information (CIK)
 */
static void amdgpu_doorbell_fini(struct amdgpu_device *adev)
{
	iounmap(adev->doorbell.ptr);
	adev->doorbell.ptr = NULL;
}

/**
 * amdgpu_doorbell_get_kfd_info - Report doorbell configuration required to
 *                                setup amdkfd
 *
 * @adev: amdgpu_device pointer
 * @aperture_base: output returning doorbell aperture base physical address
 * @aperture_size: output returning doorbell aperture size in bytes
 * @start_offset: output returning # of doorbell bytes reserved for amdgpu.
 *
 * amdgpu and amdkfd share the doorbell aperture. amdgpu sets it up,
 * takes doorbells required for its own rings and reports the setup to amdkfd.
 * amdgpu reserved doorbells are at the start of the doorbell aperture.
 */
void amdgpu_doorbell_get_kfd_info(struct amdgpu_device *adev,
				phys_addr_t *aperture_base,
				size_t *aperture_size,
				size_t *start_offset)
{
	/*
	 * The first num_doorbells are used by amdgpu.
	 * amdkfd takes whatever's left in the aperture.
	 */
	if (adev->doorbell.size > adev->doorbell.num_doorbells * sizeof(u32)) {
		*aperture_base = adev->doorbell.base;
		*aperture_size = adev->doorbell.size;
		*start_offset = adev->doorbell.num_doorbells * sizeof(u32);
	} else {
		*aperture_base = 0;
		*aperture_size = 0;
		*start_offset = 0;
	}
}

/*
 * amdgpu_wb_*()
 * Writeback is the method by which the GPU updates special pages in memory
 * with the status of certain GPU events (fences, ring pointers,etc.).
 */

/**
 * amdgpu_wb_fini - Disable Writeback and free memory
 *
 * @adev: amdgpu_device pointer
 *
 * Disables Writeback and frees the Writeback memory (all asics).
 * Used at driver shutdown.
 */
static void amdgpu_wb_fini(struct amdgpu_device *adev)
{
	if (adev->wb.wb_obj) {
		amdgpu_bo_free_kernel(&adev->wb.wb_obj,
				      &adev->wb.gpu_addr,
				      (void **)&adev->wb.wb);
		adev->wb.wb_obj = NULL;
	}
}

/**
 * amdgpu_wb_init- Init Writeback driver info and allocate memory
 *
 * @adev: amdgpu_device pointer
 *
 * Initializes writeback and allocates writeback memory (all asics).
 * Used at driver startup.
 * Returns 0 on success or an -error on failure.
 */
static int amdgpu_wb_init(struct amdgpu_device *adev)
{
	int r;

	if (adev->wb.wb_obj == NULL) {
		/* AMDGPU_MAX_WB * sizeof(uint32_t) * 8 = AMDGPU_MAX_WB 256bit slots */
		r = amdgpu_bo_create_kernel(adev, AMDGPU_MAX_WB * sizeof(uint32_t) * 8,
					    PAGE_SIZE, AMDGPU_GEM_DOMAIN_GTT,
					    &adev->wb.wb_obj, &adev->wb.gpu_addr,
					    (void **)&adev->wb.wb);
		if (r) {
			dev_warn(adev->dev, "(%d) create WB bo failed\n", r);
			return r;
		}

		adev->wb.num_wb = AMDGPU_MAX_WB;
		memset(&adev->wb.used, 0, sizeof(adev->wb.used));

		/* clear wb memory */
		memset((char *)adev->wb.wb, 0, AMDGPU_MAX_WB * sizeof(uint32_t));
	}

	return 0;
}

/**
 * amdgpu_wb_get - Allocate a wb entry
 *
 * @adev: amdgpu_device pointer
 * @wb: wb index
 *
 * Allocate a wb slot for use by the driver (all asics).
 * Returns 0 on success or -EINVAL on failure.
 */
int amdgpu_wb_get(struct amdgpu_device *adev, u32 *wb)
{
	unsigned long offset = find_first_zero_bit(adev->wb.used, adev->wb.num_wb);

	if (offset < adev->wb.num_wb) {
		__set_bit(offset, adev->wb.used);
		*wb = offset * 8; /* convert to dw offset */
		return 0;
	} else {
		return -EINVAL;
	}
}

/**
 * amdgpu_wb_free - Free a wb entry
 *
 * @adev: amdgpu_device pointer
 * @wb: wb index
 *
 * Free a wb slot allocated for use by the driver (all asics)
 */
void amdgpu_wb_free(struct amdgpu_device *adev, u32 wb)
{
	if (wb < adev->wb.num_wb)
		__clear_bit(wb, adev->wb.used);
}

/**
 * amdgpu_vram_location - try to find VRAM location
 * @adev: amdgpu device structure holding all necessary informations
 * @mc: memory controller structure holding memory informations
 * @base: base address at which to put VRAM
 *
 * Function will try to place VRAM at base address provided
 * as parameter (which is so far either PCI aperture address or
 * for IGP TOM base address).
 *
 * If there is not enough space to fit the unvisible VRAM in the 32bits
 * address space then we limit the VRAM size to the aperture.
 *
 * Note: We don't explicitly enforce VRAM start to be aligned on VRAM size,
 * this shouldn't be a problem as we are using the PCI aperture as a reference.
 * Otherwise this would be needed for rv280, all r3xx, and all r4xx, but
 * not IGP.
 *
 * Note: we use mc_vram_size as on some board we need to program the mc to
 * cover the whole aperture even if VRAM size is inferior to aperture size
 * Novell bug 204882 + along with lots of ubuntu ones
 *
 * Note: when limiting vram it's safe to overwritte real_vram_size because
 * we are not in case where real_vram_size is inferior to mc_vram_size (ie
 * note afected by bogus hw of Novell bug 204882 + along with lots of ubuntu
 * ones)
 *
 * Note: IGP TOM addr should be the same as the aperture addr, we don't
 * explicitly check for that though.
 *
 * FIXME: when reducing VRAM size align new size on power of 2.
 */
void amdgpu_vram_location(struct amdgpu_device *adev, struct amdgpu_mc *mc, u64 base)
{
	uint64_t limit = (uint64_t)amdgpu_vram_limit << 20;

	mc->vram_start = base;
	if (mc->mc_vram_size > (adev->mc.mc_mask - base + 1)) {
		dev_warn(adev->dev, "limiting VRAM to PCI aperture size\n");
		mc->real_vram_size = mc->aper_size;
		mc->mc_vram_size = mc->aper_size;
	}
	mc->vram_end = mc->vram_start + mc->mc_vram_size - 1;
	if (limit && limit < mc->real_vram_size)
		mc->real_vram_size = limit;
	dev_info(adev->dev, "VRAM: %lluM 0x%016llX - 0x%016llX (%lluM used)\n",
			mc->mc_vram_size >> 20, mc->vram_start,
			mc->vram_end, mc->real_vram_size >> 20);
}

/**
 * amdgpu_gart_location - try to find GTT location
 * @adev: amdgpu device structure holding all necessary informations
 * @mc: memory controller structure holding memory informations
 *
 * Function will place try to place GTT before or after VRAM.
 *
 * If GTT size is bigger than space left then we ajust GTT size.
 * Thus function will never fails.
 *
 * FIXME: when reducing GTT size align new size on power of 2.
 */
void amdgpu_gart_location(struct amdgpu_device *adev, struct amdgpu_mc *mc)
{
	u64 size_af, size_bf;

	size_af = adev->mc.mc_mask - mc->vram_end;
	size_bf = mc->vram_start;
	if (size_bf > size_af) {
		if (mc->gart_size > size_bf) {
			dev_warn(adev->dev, "limiting GTT\n");
			mc->gart_size = size_bf;
		}
		mc->gart_start = 0;
	} else {
		if (mc->gart_size > size_af) {
			dev_warn(adev->dev, "limiting GTT\n");
			mc->gart_size = size_af;
		}
		mc->gart_start = mc->vram_end + 1;
	}
	mc->gart_end = mc->gart_start + mc->gart_size - 1;
	dev_info(adev->dev, "GTT: %lluM 0x%016llX - 0x%016llX\n",
			mc->gart_size >> 20, mc->gart_start, mc->gart_end);
}

/*
 * GPU helpers function.
 */
/**
 * amdgpu_need_post - check if the hw need post or not
 *
 * @adev: amdgpu_device pointer
 *
 * Check if the asic has been initialized (all asics) at driver startup
 * or post is needed if  hw reset is performed.
 * Returns true if need or false if not.
 */
bool amdgpu_need_post(struct amdgpu_device *adev)
{
	uint32_t reg;

	if (adev->has_hw_reset) {
		adev->has_hw_reset = false;
		return true;
	}

	/* bios scratch used on CIK+ */
	if (adev->asic_type >= CHIP_BONAIRE)
		return amdgpu_atombios_scratch_need_asic_init(adev);

	/* check MEM_SIZE for older asics */
	reg = amdgpu_asic_get_config_memsize(adev);

	if ((reg != 0) && (reg != 0xffffffff))
		return false;

	return true;

}

static bool amdgpu_vpost_needed(struct amdgpu_device *adev)
{
	if (amdgpu_sriov_vf(adev))
		return false;

	if (amdgpu_passthrough(adev)) {
		/* for FIJI: In whole GPU pass-through virtualization case, after VM reboot
		 * some old smc fw still need driver do vPost otherwise gpu hang, while
		 * those smc fw version above 22.15 doesn't have this flaw, so we force
		 * vpost executed for smc version below 22.15
		 */
		if (adev->asic_type == CHIP_FIJI) {
			int err;
			uint32_t fw_ver;
			err = request_firmware(&adev->pm.fw, "amdgpu/fiji_smc.bin", adev->dev);
			/* force vPost if error occured */
			if (err)
				return true;

			fw_ver = *((uint32_t *)adev->pm.fw->data + 69);
			if (fw_ver < 0x00160e00)
				return true;
		}
	}
	return amdgpu_need_post(adev);
}

/**
 * amdgpu_dummy_page_init - init dummy page used by the driver
 *
 * @adev: amdgpu_device pointer
 *
 * Allocate the dummy page used by the driver (all asics).
 * This dummy page is used by the driver as a filler for gart entries
 * when pages are taken out of the GART
 * Returns 0 on sucess, -ENOMEM on failure.
 */
int amdgpu_dummy_page_init(struct amdgpu_device *adev)
{
	if (adev->dummy_page.page)
		return 0;
	adev->dummy_page.page = alloc_page(GFP_DMA32 | GFP_KERNEL | __GFP_ZERO);
	if (adev->dummy_page.page == NULL)
		return -ENOMEM;
	adev->dummy_page.addr = pci_map_page(adev->pdev, adev->dummy_page.page,
					0, PAGE_SIZE, PCI_DMA_BIDIRECTIONAL);
	if (pci_dma_mapping_error(adev->pdev, adev->dummy_page.addr)) {
		dev_err(&adev->pdev->dev, "Failed to DMA MAP the dummy page\n");
		__free_page(adev->dummy_page.page);
		adev->dummy_page.page = NULL;
		return -ENOMEM;
	}
	return 0;
}

/**
 * amdgpu_dummy_page_fini - free dummy page used by the driver
 *
 * @adev: amdgpu_device pointer
 *
 * Frees the dummy page used by the driver (all asics).
 */
void amdgpu_dummy_page_fini(struct amdgpu_device *adev)
{
	if (adev->dummy_page.page == NULL)
		return;
	pci_unmap_page(adev->pdev, adev->dummy_page.addr,
			PAGE_SIZE, PCI_DMA_BIDIRECTIONAL);
	__free_page(adev->dummy_page.page);
	adev->dummy_page.page = NULL;
}


/* ATOM accessor methods */
/*
 * ATOM is an interpreted byte code stored in tables in the vbios.  The
 * driver registers callbacks to access registers and the interpreter
 * in the driver parses the tables and executes then to program specific
 * actions (set display modes, asic init, etc.).  See amdgpu_atombios.c,
 * atombios.h, and atom.c
 */

/**
 * cail_pll_read - read PLL register
 *
 * @info: atom card_info pointer
 * @reg: PLL register offset
 *
 * Provides a PLL register accessor for the atom interpreter (r4xx+).
 * Returns the value of the PLL register.
 */
static uint32_t cail_pll_read(struct card_info *info, uint32_t reg)
{
	return 0;
}

/**
 * cail_pll_write - write PLL register
 *
 * @info: atom card_info pointer
 * @reg: PLL register offset
 * @val: value to write to the pll register
 *
 * Provides a PLL register accessor for the atom interpreter (r4xx+).
 */
static void cail_pll_write(struct card_info *info, uint32_t reg, uint32_t val)
{

}

/**
 * cail_mc_read - read MC (Memory Controller) register
 *
 * @info: atom card_info pointer
 * @reg: MC register offset
 *
 * Provides an MC register accessor for the atom interpreter (r4xx+).
 * Returns the value of the MC register.
 */
static uint32_t cail_mc_read(struct card_info *info, uint32_t reg)
{
	return 0;
}

/**
 * cail_mc_write - write MC (Memory Controller) register
 *
 * @info: atom card_info pointer
 * @reg: MC register offset
 * @val: value to write to the pll register
 *
 * Provides a MC register accessor for the atom interpreter (r4xx+).
 */
static void cail_mc_write(struct card_info *info, uint32_t reg, uint32_t val)
{

}

/**
 * cail_reg_write - write MMIO register
 *
 * @info: atom card_info pointer
 * @reg: MMIO register offset
 * @val: value to write to the pll register
 *
 * Provides a MMIO register accessor for the atom interpreter (r4xx+).
 */
static void cail_reg_write(struct card_info *info, uint32_t reg, uint32_t val)
{
	struct amdgpu_device *adev = info->dev->dev_private;

	WREG32(reg, val);
}

/**
 * cail_reg_read - read MMIO register
 *
 * @info: atom card_info pointer
 * @reg: MMIO register offset
 *
 * Provides an MMIO register accessor for the atom interpreter (r4xx+).
 * Returns the value of the MMIO register.
 */
static uint32_t cail_reg_read(struct card_info *info, uint32_t reg)
{
	struct amdgpu_device *adev = info->dev->dev_private;
	uint32_t r;

	r = RREG32(reg);
	return r;
}

/**
 * cail_ioreg_write - write IO register
 *
 * @info: atom card_info pointer
 * @reg: IO register offset
 * @val: value to write to the pll register
 *
 * Provides a IO register accessor for the atom interpreter (r4xx+).
 */
static void cail_ioreg_write(struct card_info *info, uint32_t reg, uint32_t val)
{
	struct amdgpu_device *adev = info->dev->dev_private;

	WREG32_IO(reg, val);
}

/**
 * cail_ioreg_read - read IO register
 *
 * @info: atom card_info pointer
 * @reg: IO register offset
 *
 * Provides an IO register accessor for the atom interpreter (r4xx+).
 * Returns the value of the IO register.
 */
static uint32_t cail_ioreg_read(struct card_info *info, uint32_t reg)
{
	struct amdgpu_device *adev = info->dev->dev_private;
	uint32_t r;

	r = RREG32_IO(reg);
	return r;
}

/**
 * amdgpu_atombios_fini - free the driver info and callbacks for atombios
 *
 * @adev: amdgpu_device pointer
 *
 * Frees the driver info and register access callbacks for the ATOM
 * interpreter (r4xx+).
 * Called at driver shutdown.
 */
static void amdgpu_atombios_fini(struct amdgpu_device *adev)
{
	if (adev->mode_info.atom_context) {
		kfree(adev->mode_info.atom_context->scratch);
		kfree(adev->mode_info.atom_context->iio);
	}
	kfree(adev->mode_info.atom_context);
	adev->mode_info.atom_context = NULL;
	kfree(adev->mode_info.atom_card_info);
	adev->mode_info.atom_card_info = NULL;
}

/**
 * amdgpu_atombios_init - init the driver info and callbacks for atombios
 *
 * @adev: amdgpu_device pointer
 *
 * Initializes the driver info and register access callbacks for the
 * ATOM interpreter (r4xx+).
 * Returns 0 on sucess, -ENOMEM on failure.
 * Called at driver startup.
 */
static int amdgpu_atombios_init(struct amdgpu_device *adev)
{
	struct card_info *atom_card_info =
	    kzalloc(sizeof(struct card_info), GFP_KERNEL);

	if (!atom_card_info)
		return -ENOMEM;

	adev->mode_info.atom_card_info = atom_card_info;
	atom_card_info->dev = adev->ddev;
	atom_card_info->reg_read = cail_reg_read;
	atom_card_info->reg_write = cail_reg_write;
	/* needed for iio ops */
	if (adev->rio_mem) {
		atom_card_info->ioreg_read = cail_ioreg_read;
		atom_card_info->ioreg_write = cail_ioreg_write;
	} else {
		DRM_INFO("PCI I/O BAR is not found. Using MMIO to access ATOM BIOS\n");
		atom_card_info->ioreg_read = cail_reg_read;
		atom_card_info->ioreg_write = cail_reg_write;
	}
	atom_card_info->mc_read = cail_mc_read;
	atom_card_info->mc_write = cail_mc_write;
	atom_card_info->pll_read = cail_pll_read;
	atom_card_info->pll_write = cail_pll_write;

	adev->mode_info.atom_context = amdgpu_atom_parse(atom_card_info, adev->bios);
	if (!adev->mode_info.atom_context) {
		amdgpu_atombios_fini(adev);
		return -ENOMEM;
	}

	mutex_init(&adev->mode_info.atom_context->mutex);
	if (adev->is_atom_fw) {
		amdgpu_atomfirmware_scratch_regs_init(adev);
		amdgpu_atomfirmware_allocate_fb_scratch(adev);
	} else {
		amdgpu_atombios_scratch_regs_init(adev);
		amdgpu_atombios_allocate_fb_scratch(adev);
	}
	return 0;
}

/* if we get transitioned to only one device, take VGA back */
/**
 * amdgpu_vga_set_decode - enable/disable vga decode
 *
 * @cookie: amdgpu_device pointer
 * @state: enable/disable vga decode
 *
 * Enable/disable vga decode (all asics).
 * Returns VGA resource flags.
 */
static unsigned int amdgpu_vga_set_decode(void *cookie, bool state)
{
	struct amdgpu_device *adev = cookie;
	amdgpu_asic_set_vga_state(adev, state);
	if (state)
		return VGA_RSRC_LEGACY_IO | VGA_RSRC_LEGACY_MEM |
		       VGA_RSRC_NORMAL_IO | VGA_RSRC_NORMAL_MEM;
	else
		return VGA_RSRC_NORMAL_IO | VGA_RSRC_NORMAL_MEM;
}

static void amdgpu_check_block_size(struct amdgpu_device *adev)
{
	/* defines number of bits in page table versus page directory,
	 * a page is 4KB so we have 12 bits offset, minimum 9 bits in the
	 * page table and the remaining bits are in the page directory */
	if (amdgpu_vm_block_size == -1)
		return;

	if (amdgpu_vm_block_size < 9) {
		dev_warn(adev->dev, "VM page table size (%d) too small\n",
			 amdgpu_vm_block_size);
		goto def_value;
	}

	if (amdgpu_vm_block_size > 24 ||
	    (amdgpu_vm_size * 1024) < (1ull << amdgpu_vm_block_size)) {
		dev_warn(adev->dev, "VM page table size (%d) too large\n",
			 amdgpu_vm_block_size);
		goto def_value;
	}

	return;

def_value:
	amdgpu_vm_block_size = -1;
}

static void amdgpu_check_vm_size(struct amdgpu_device *adev)
{
	/* no need to check the default value */
	if (amdgpu_vm_size == -1)
		return;

	if (!is_power_of_2(amdgpu_vm_size)) {
		dev_warn(adev->dev, "VM size (%d) must be a power of 2\n",
			 amdgpu_vm_size);
		goto def_value;
	}

	if (amdgpu_vm_size < 1) {
		dev_warn(adev->dev, "VM size (%d) too small, min is 1GB\n",
			 amdgpu_vm_size);
		goto def_value;
	}

	/*
	 * Max GPUVM size for Cayman, SI, CI VI are 40 bits.
	 */
	if (amdgpu_vm_size > 1024) {
		dev_warn(adev->dev, "VM size (%d) too large, max is 1TB\n",
			 amdgpu_vm_size);
		goto def_value;
	}

	return;

def_value:
	amdgpu_vm_size = -1;
}

/**
 * amdgpu_check_arguments - validate module params
 *
 * @adev: amdgpu_device pointer
 *
 * Validates certain module parameters and updates
 * the associated values used by the driver (all asics).
 */
static void amdgpu_check_arguments(struct amdgpu_device *adev)
{
	if (amdgpu_sched_jobs < 4) {
		dev_warn(adev->dev, "sched jobs (%d) must be at least 4\n",
			 amdgpu_sched_jobs);
		amdgpu_sched_jobs = 4;
	} else if (!is_power_of_2(amdgpu_sched_jobs)){
		dev_warn(adev->dev, "sched jobs (%d) must be a power of 2\n",
			 amdgpu_sched_jobs);
		amdgpu_sched_jobs = roundup_pow_of_two(amdgpu_sched_jobs);
	}

	if (amdgpu_gart_size != -1 && amdgpu_gart_size < 32) {
		/* gart size must be greater or equal to 32M */
		dev_warn(adev->dev, "gart size (%d) too small\n",
			 amdgpu_gart_size);
		amdgpu_gart_size = -1;
	}

	if (amdgpu_gtt_size != -1 && amdgpu_gtt_size < 32) {
		/* gtt size must be greater or equal to 32M */
		dev_warn(adev->dev, "gtt size (%d) too small\n",
				 amdgpu_gtt_size);
		amdgpu_gtt_size = -1;
	}

	/* valid range is between 4 and 9 inclusive */
	if (amdgpu_vm_fragment_size != -1 &&
	    (amdgpu_vm_fragment_size > 9 || amdgpu_vm_fragment_size < 4)) {
		dev_warn(adev->dev, "valid range is between 4 and 9\n");
		amdgpu_vm_fragment_size = -1;
	}

	amdgpu_check_vm_size(adev);

	amdgpu_check_block_size(adev);

	if (amdgpu_vram_page_split != -1 && (amdgpu_vram_page_split < 16 ||
	    !is_power_of_2(amdgpu_vram_page_split))) {
		dev_warn(adev->dev, "invalid VRAM page split (%d)\n",
			 amdgpu_vram_page_split);
		amdgpu_vram_page_split = 1024;
	}
}

/**
 * amdgpu_switcheroo_set_state - set switcheroo state
 *
 * @pdev: pci dev pointer
 * @state: vga_switcheroo state
 *
 * Callback for the switcheroo driver.  Suspends or resumes the
 * the asics before or after it is powered up using ACPI methods.
 */
static void amdgpu_switcheroo_set_state(struct pci_dev *pdev, enum vga_switcheroo_state state)
{
	struct drm_device *dev = pci_get_drvdata(pdev);

	if (amdgpu_device_is_px(dev) && state == VGA_SWITCHEROO_OFF)
		return;

	if (state == VGA_SWITCHEROO_ON) {
		pr_info("amdgpu: switched on\n");
		/* don't suspend or resume card normally */
		dev->switch_power_state = DRM_SWITCH_POWER_CHANGING;

		amdgpu_device_resume(dev, true, true);

		dev->switch_power_state = DRM_SWITCH_POWER_ON;
		drm_kms_helper_poll_enable(dev);
	} else {
		pr_info("amdgpu: switched off\n");
		drm_kms_helper_poll_disable(dev);
		dev->switch_power_state = DRM_SWITCH_POWER_CHANGING;
		amdgpu_device_suspend(dev, true, true);
		dev->switch_power_state = DRM_SWITCH_POWER_OFF;
	}
}

/**
 * amdgpu_switcheroo_can_switch - see if switcheroo state can change
 *
 * @pdev: pci dev pointer
 *
 * Callback for the switcheroo driver.  Check of the switcheroo
 * state can be changed.
 * Returns true if the state can be changed, false if not.
 */
static bool amdgpu_switcheroo_can_switch(struct pci_dev *pdev)
{
	struct drm_device *dev = pci_get_drvdata(pdev);

	/*
	* FIXME: open_count is protected by drm_global_mutex but that would lead to
	* locking inversion with the driver load path. And the access here is
	* completely racy anyway. So don't bother with locking for now.
	*/
	return dev->open_count == 0;
}

static const struct vga_switcheroo_client_ops amdgpu_switcheroo_ops = {
	.set_gpu_state = amdgpu_switcheroo_set_state,
	.reprobe = NULL,
	.can_switch = amdgpu_switcheroo_can_switch,
};

int amdgpu_set_clockgating_state(struct amdgpu_device *adev,
				  enum amd_ip_block_type block_type,
				  enum amd_clockgating_state state)
{
	int i, r = 0;

	for (i = 0; i < adev->num_ip_blocks; i++) {
		if (!adev->ip_blocks[i].status.valid)
			continue;
		if (adev->ip_blocks[i].version->type != block_type)
			continue;
		if (!adev->ip_blocks[i].version->funcs->set_clockgating_state)
			continue;
		r = adev->ip_blocks[i].version->funcs->set_clockgating_state(
			(void *)adev, state);
		if (r)
			DRM_ERROR("set_clockgating_state of IP block <%s> failed %d\n",
				  adev->ip_blocks[i].version->funcs->name, r);
	}
	return r;
}

int amdgpu_set_powergating_state(struct amdgpu_device *adev,
				  enum amd_ip_block_type block_type,
				  enum amd_powergating_state state)
{
	int i, r = 0;

	for (i = 0; i < adev->num_ip_blocks; i++) {
		if (!adev->ip_blocks[i].status.valid)
			continue;
		if (adev->ip_blocks[i].version->type != block_type)
			continue;
		if (!adev->ip_blocks[i].version->funcs->set_powergating_state)
			continue;
		r = adev->ip_blocks[i].version->funcs->set_powergating_state(
			(void *)adev, state);
		if (r)
			DRM_ERROR("set_powergating_state of IP block <%s> failed %d\n",
				  adev->ip_blocks[i].version->funcs->name, r);
	}
	return r;
}

void amdgpu_get_clockgating_state(struct amdgpu_device *adev, u32 *flags)
{
	int i;

	for (i = 0; i < adev->num_ip_blocks; i++) {
		if (!adev->ip_blocks[i].status.valid)
			continue;
		if (adev->ip_blocks[i].version->funcs->get_clockgating_state)
			adev->ip_blocks[i].version->funcs->get_clockgating_state((void *)adev, flags);
	}
}

int amdgpu_wait_for_idle(struct amdgpu_device *adev,
			 enum amd_ip_block_type block_type)
{
	int i, r;

	for (i = 0; i < adev->num_ip_blocks; i++) {
		if (!adev->ip_blocks[i].status.valid)
			continue;
		if (adev->ip_blocks[i].version->type == block_type) {
			r = adev->ip_blocks[i].version->funcs->wait_for_idle((void *)adev);
			if (r)
				return r;
			break;
		}
	}
	return 0;

}

bool amdgpu_is_idle(struct amdgpu_device *adev,
		    enum amd_ip_block_type block_type)
{
	int i;

	for (i = 0; i < adev->num_ip_blocks; i++) {
		if (!adev->ip_blocks[i].status.valid)
			continue;
		if (adev->ip_blocks[i].version->type == block_type)
			return adev->ip_blocks[i].version->funcs->is_idle((void *)adev);
	}
	return true;

}

struct amdgpu_ip_block * amdgpu_get_ip_block(struct amdgpu_device *adev,
					     enum amd_ip_block_type type)
{
	int i;

	for (i = 0; i < adev->num_ip_blocks; i++)
		if (adev->ip_blocks[i].version->type == type)
			return &adev->ip_blocks[i];

	return NULL;
}

/**
 * amdgpu_ip_block_version_cmp
 *
 * @adev: amdgpu_device pointer
 * @type: enum amd_ip_block_type
 * @major: major version
 * @minor: minor version
 *
 * return 0 if equal or greater
 * return 1 if smaller or the ip_block doesn't exist
 */
int amdgpu_ip_block_version_cmp(struct amdgpu_device *adev,
				enum amd_ip_block_type type,
				u32 major, u32 minor)
{
	struct amdgpu_ip_block *ip_block = amdgpu_get_ip_block(adev, type);

	if (ip_block && ((ip_block->version->major > major) ||
			((ip_block->version->major == major) &&
			(ip_block->version->minor >= minor))))
		return 0;

	return 1;
}

/**
 * amdgpu_ip_block_add
 *
 * @adev: amdgpu_device pointer
 * @ip_block_version: pointer to the IP to add
 *
 * Adds the IP block driver information to the collection of IPs
 * on the asic.
 */
int amdgpu_ip_block_add(struct amdgpu_device *adev,
			const struct amdgpu_ip_block_version *ip_block_version)
{
	if (!ip_block_version)
		return -EINVAL;

	DRM_DEBUG("add ip block number %d <%s>\n", adev->num_ip_blocks,
		  ip_block_version->funcs->name);

	adev->ip_blocks[adev->num_ip_blocks++].version = ip_block_version;

	return 0;
}

static void amdgpu_device_enable_virtual_display(struct amdgpu_device *adev)
{
	adev->enable_virtual_display = false;

	if (amdgpu_virtual_display) {
		struct drm_device *ddev = adev->ddev;
		const char *pci_address_name = pci_name(ddev->pdev);
		char *pciaddstr, *pciaddstr_tmp, *pciaddname_tmp, *pciaddname;

		pciaddstr = kstrdup(amdgpu_virtual_display, GFP_KERNEL);
		pciaddstr_tmp = pciaddstr;
		while ((pciaddname_tmp = strsep(&pciaddstr_tmp, ";"))) {
			pciaddname = strsep(&pciaddname_tmp, ",");
			if (!strcmp("all", pciaddname)
			    || !strcmp(pci_address_name, pciaddname)) {
				long num_crtc;
				int res = -1;

				adev->enable_virtual_display = true;

				if (pciaddname_tmp)
					res = kstrtol(pciaddname_tmp, 10,
						      &num_crtc);

				if (!res) {
					if (num_crtc < 1)
						num_crtc = 1;
					if (num_crtc > 6)
						num_crtc = 6;
					adev->mode_info.num_crtc = num_crtc;
				} else {
					adev->mode_info.num_crtc = 1;
				}
				break;
			}
		}

		DRM_INFO("virtual display string:%s, %s:virtual_display:%d, num_crtc:%d\n",
			 amdgpu_virtual_display, pci_address_name,
			 adev->enable_virtual_display, adev->mode_info.num_crtc);

		kfree(pciaddstr);
	}
}

static int amdgpu_device_parse_gpu_info_fw(struct amdgpu_device *adev)
{
	const char *chip_name;
	char fw_name[30];
	int err;
	const struct gpu_info_firmware_header_v1_0 *hdr;

	adev->firmware.gpu_info_fw = NULL;

	switch (adev->asic_type) {
	case CHIP_TOPAZ:
	case CHIP_TONGA:
	case CHIP_FIJI:
	case CHIP_POLARIS11:
	case CHIP_POLARIS10:
	case CHIP_POLARIS12:
	case CHIP_CARRIZO:
	case CHIP_STONEY:
#ifdef CONFIG_DRM_AMDGPU_SI
	case CHIP_VERDE:
	case CHIP_TAHITI:
	case CHIP_PITCAIRN:
	case CHIP_OLAND:
	case CHIP_HAINAN:
#endif
#ifdef CONFIG_DRM_AMDGPU_CIK
	case CHIP_BONAIRE:
	case CHIP_HAWAII:
	case CHIP_KAVERI:
	case CHIP_KABINI:
	case CHIP_MULLINS:
#endif
	default:
		return 0;
	case CHIP_VEGA10:
		chip_name = "vega10";
		break;
	case CHIP_RAVEN:
		chip_name = "raven";
		break;
	}

	snprintf(fw_name, sizeof(fw_name), "amdgpu/%s_gpu_info.bin", chip_name);
	err = request_firmware(&adev->firmware.gpu_info_fw, fw_name, adev->dev);
	if (err) {
		dev_err(adev->dev,
			"Failed to load gpu_info firmware \"%s\"\n",
			fw_name);
		goto out;
	}
	err = amdgpu_ucode_validate(adev->firmware.gpu_info_fw);
	if (err) {
		dev_err(adev->dev,
			"Failed to validate gpu_info firmware \"%s\"\n",
			fw_name);
		goto out;
	}

	hdr = (const struct gpu_info_firmware_header_v1_0 *)adev->firmware.gpu_info_fw->data;
	amdgpu_ucode_print_gpu_info_hdr(&hdr->header);

	switch (hdr->version_major) {
	case 1:
	{
		const struct gpu_info_firmware_v1_0 *gpu_info_fw =
			(const struct gpu_info_firmware_v1_0 *)(adev->firmware.gpu_info_fw->data +
								le32_to_cpu(hdr->header.ucode_array_offset_bytes));

		adev->gfx.config.max_shader_engines = le32_to_cpu(gpu_info_fw->gc_num_se);
		adev->gfx.config.max_cu_per_sh = le32_to_cpu(gpu_info_fw->gc_num_cu_per_sh);
		adev->gfx.config.max_sh_per_se = le32_to_cpu(gpu_info_fw->gc_num_sh_per_se);
		adev->gfx.config.max_backends_per_se = le32_to_cpu(gpu_info_fw->gc_num_rb_per_se);
		adev->gfx.config.max_texture_channel_caches =
			le32_to_cpu(gpu_info_fw->gc_num_tccs);
		adev->gfx.config.max_gprs = le32_to_cpu(gpu_info_fw->gc_num_gprs);
		adev->gfx.config.max_gs_threads = le32_to_cpu(gpu_info_fw->gc_num_max_gs_thds);
		adev->gfx.config.gs_vgt_table_depth = le32_to_cpu(gpu_info_fw->gc_gs_table_depth);
		adev->gfx.config.gs_prim_buffer_depth = le32_to_cpu(gpu_info_fw->gc_gsprim_buff_depth);
		adev->gfx.config.double_offchip_lds_buf =
			le32_to_cpu(gpu_info_fw->gc_double_offchip_lds_buffer);
		adev->gfx.cu_info.wave_front_size = le32_to_cpu(gpu_info_fw->gc_wave_size);
		adev->gfx.cu_info.max_waves_per_simd =
			le32_to_cpu(gpu_info_fw->gc_max_waves_per_simd);
		adev->gfx.cu_info.max_scratch_slots_per_cu =
			le32_to_cpu(gpu_info_fw->gc_max_scratch_slots_per_cu);
		adev->gfx.cu_info.lds_size = le32_to_cpu(gpu_info_fw->gc_lds_size);
		break;
	}
	default:
		dev_err(adev->dev,
			"Unsupported gpu_info table %d\n", hdr->header.ucode_version);
		err = -EINVAL;
		goto out;
	}
out:
	return err;
}

static int amdgpu_early_init(struct amdgpu_device *adev)
{
	int i, r;

	amdgpu_device_enable_virtual_display(adev);

	switch (adev->asic_type) {
	case CHIP_TOPAZ:
	case CHIP_TONGA:
	case CHIP_FIJI:
	case CHIP_POLARIS11:
	case CHIP_POLARIS10:
	case CHIP_POLARIS12:
	case CHIP_CARRIZO:
	case CHIP_STONEY:
		if (adev->asic_type == CHIP_CARRIZO || adev->asic_type == CHIP_STONEY)
			adev->family = AMDGPU_FAMILY_CZ;
		else
			adev->family = AMDGPU_FAMILY_VI;

		r = vi_set_ip_blocks(adev);
		if (r)
			return r;
		break;
#ifdef CONFIG_DRM_AMDGPU_SI
	case CHIP_VERDE:
	case CHIP_TAHITI:
	case CHIP_PITCAIRN:
	case CHIP_OLAND:
	case CHIP_HAINAN:
		adev->family = AMDGPU_FAMILY_SI;
		r = si_set_ip_blocks(adev);
		if (r)
			return r;
		break;
#endif
#ifdef CONFIG_DRM_AMDGPU_CIK
	case CHIP_BONAIRE:
	case CHIP_HAWAII:
	case CHIP_KAVERI:
	case CHIP_KABINI:
	case CHIP_MULLINS:
		if ((adev->asic_type == CHIP_BONAIRE) || (adev->asic_type == CHIP_HAWAII))
			adev->family = AMDGPU_FAMILY_CI;
		else
			adev->family = AMDGPU_FAMILY_KV;

		r = cik_set_ip_blocks(adev);
		if (r)
			return r;
		break;
#endif
	case  CHIP_VEGA10:
	case  CHIP_RAVEN:
		if (adev->asic_type == CHIP_RAVEN)
			adev->family = AMDGPU_FAMILY_RV;
		else
			adev->family = AMDGPU_FAMILY_AI;

		r = soc15_set_ip_blocks(adev);
		if (r)
			return r;
		break;
	default:
		/* FIXME: not supported yet */
		return -EINVAL;
	}

	r = amdgpu_device_parse_gpu_info_fw(adev);
	if (r)
		return r;

	if (amdgpu_sriov_vf(adev)) {
		r = amdgpu_virt_request_full_gpu(adev, true);
		if (r)
			return r;
	}

	for (i = 0; i < adev->num_ip_blocks; i++) {
		if ((amdgpu_ip_block_mask & (1 << i)) == 0) {
			DRM_ERROR("disabled ip block: %d <%s>\n",
				  i, adev->ip_blocks[i].version->funcs->name);
			adev->ip_blocks[i].status.valid = false;
		} else {
			if (adev->ip_blocks[i].version->funcs->early_init) {
				r = adev->ip_blocks[i].version->funcs->early_init((void *)adev);
				if (r == -ENOENT) {
					adev->ip_blocks[i].status.valid = false;
				} else if (r) {
					DRM_ERROR("early_init of IP block <%s> failed %d\n",
						  adev->ip_blocks[i].version->funcs->name, r);
					return r;
				} else {
					adev->ip_blocks[i].status.valid = true;
				}
			} else {
				adev->ip_blocks[i].status.valid = true;
			}
		}
	}

	adev->cg_flags &= amdgpu_cg_mask;
	adev->pg_flags &= amdgpu_pg_mask;

	return 0;
}

static int amdgpu_init(struct amdgpu_device *adev)
{
	int i, r;

	for (i = 0; i < adev->num_ip_blocks; i++) {
		if (!adev->ip_blocks[i].status.valid)
			continue;
		r = adev->ip_blocks[i].version->funcs->sw_init((void *)adev);
		if (r) {
			DRM_ERROR("sw_init of IP block <%s> failed %d\n",
				  adev->ip_blocks[i].version->funcs->name, r);
			return r;
		}
		adev->ip_blocks[i].status.sw = true;
		/* need to do gmc hw init early so we can allocate gpu mem */
		if (adev->ip_blocks[i].version->type == AMD_IP_BLOCK_TYPE_GMC) {
			r = amdgpu_vram_scratch_init(adev);
			if (r) {
				DRM_ERROR("amdgpu_vram_scratch_init failed %d\n", r);
				return r;
			}
			r = adev->ip_blocks[i].version->funcs->hw_init((void *)adev);
			if (r) {
				DRM_ERROR("hw_init %d failed %d\n", i, r);
				return r;
			}
			r = amdgpu_wb_init(adev);
			if (r) {
				DRM_ERROR("amdgpu_wb_init failed %d\n", r);
				return r;
			}
			adev->ip_blocks[i].status.hw = true;

			/* right after GMC hw init, we create CSA */
			if (amdgpu_sriov_vf(adev)) {
				r = amdgpu_allocate_static_csa(adev);
				if (r) {
					DRM_ERROR("allocate CSA failed %d\n", r);
					return r;
				}
			}
		}
	}

	for (i = 0; i < adev->num_ip_blocks; i++) {
		if (!adev->ip_blocks[i].status.sw)
			continue;
		/* gmc hw init is done early */
		if (adev->ip_blocks[i].version->type == AMD_IP_BLOCK_TYPE_GMC)
			continue;
		r = adev->ip_blocks[i].version->funcs->hw_init((void *)adev);
		if (r) {
			DRM_ERROR("hw_init of IP block <%s> failed %d\n",
				  adev->ip_blocks[i].version->funcs->name, r);
			return r;
		}
		adev->ip_blocks[i].status.hw = true;
	}

	return 0;
}

static void amdgpu_fill_reset_magic(struct amdgpu_device *adev)
{
	memcpy(adev->reset_magic, adev->gart.ptr, AMDGPU_RESET_MAGIC_NUM);
}

static bool amdgpu_check_vram_lost(struct amdgpu_device *adev)
{
	return !!memcmp(adev->gart.ptr, adev->reset_magic,
			AMDGPU_RESET_MAGIC_NUM);
}

static int amdgpu_late_set_cg_state(struct amdgpu_device *adev)
{
	int i = 0, r;

	for (i = 0; i < adev->num_ip_blocks; i++) {
		if (!adev->ip_blocks[i].status.valid)
			continue;
		/* skip CG for VCE/UVD, it's handled specially */
		if (adev->ip_blocks[i].version->type != AMD_IP_BLOCK_TYPE_UVD &&
		    adev->ip_blocks[i].version->type != AMD_IP_BLOCK_TYPE_VCE) {
			/* enable clockgating to save power */
			r = adev->ip_blocks[i].version->funcs->set_clockgating_state((void *)adev,
										     AMD_CG_STATE_GATE);
			if (r) {
				DRM_ERROR("set_clockgating_state(gate) of IP block <%s> failed %d\n",
					  adev->ip_blocks[i].version->funcs->name, r);
				return r;
			}
		}
	}
	return 0;
}

static int amdgpu_late_init(struct amdgpu_device *adev)
{
	int i = 0, r;

	for (i = 0; i < adev->num_ip_blocks; i++) {
		if (!adev->ip_blocks[i].status.valid)
			continue;
		if (adev->ip_blocks[i].version->funcs->late_init) {
			r = adev->ip_blocks[i].version->funcs->late_init((void *)adev);
			if (r) {
				DRM_ERROR("late_init of IP block <%s> failed %d\n",
					  adev->ip_blocks[i].version->funcs->name, r);
				return r;
			}
			adev->ip_blocks[i].status.late_initialized = true;
		}
	}

	mod_delayed_work(system_wq, &adev->late_init_work,
			msecs_to_jiffies(AMDGPU_RESUME_MS));

	amdgpu_fill_reset_magic(adev);

	return 0;
}

static int amdgpu_fini(struct amdgpu_device *adev)
{
	int i, r;

	/* need to disable SMC first */
	for (i = 0; i < adev->num_ip_blocks; i++) {
		if (!adev->ip_blocks[i].status.hw)
			continue;
		if (adev->ip_blocks[i].version->type == AMD_IP_BLOCK_TYPE_SMC) {
			/* ungate blocks before hw fini so that we can shutdown the blocks safely */
			r = adev->ip_blocks[i].version->funcs->set_clockgating_state((void *)adev,
										     AMD_CG_STATE_UNGATE);
			if (r) {
				DRM_ERROR("set_clockgating_state(ungate) of IP block <%s> failed %d\n",
					  adev->ip_blocks[i].version->funcs->name, r);
				return r;
			}
			r = adev->ip_blocks[i].version->funcs->hw_fini((void *)adev);
			/* XXX handle errors */
			if (r) {
				DRM_DEBUG("hw_fini of IP block <%s> failed %d\n",
					  adev->ip_blocks[i].version->funcs->name, r);
			}
			adev->ip_blocks[i].status.hw = false;
			break;
		}
	}

	for (i = adev->num_ip_blocks - 1; i >= 0; i--) {
		if (!adev->ip_blocks[i].status.hw)
			continue;
		if (adev->ip_blocks[i].version->type == AMD_IP_BLOCK_TYPE_GMC) {
			amdgpu_wb_fini(adev);
			amdgpu_vram_scratch_fini(adev);
		}

		if (adev->ip_blocks[i].version->type != AMD_IP_BLOCK_TYPE_UVD &&
			adev->ip_blocks[i].version->type != AMD_IP_BLOCK_TYPE_VCE) {
			/* ungate blocks before hw fini so that we can shutdown the blocks safely */
			r = adev->ip_blocks[i].version->funcs->set_clockgating_state((void *)adev,
										     AMD_CG_STATE_UNGATE);
			if (r) {
				DRM_ERROR("set_clockgating_state(ungate) of IP block <%s> failed %d\n",
					  adev->ip_blocks[i].version->funcs->name, r);
				return r;
			}
		}

		r = adev->ip_blocks[i].version->funcs->hw_fini((void *)adev);
		/* XXX handle errors */
		if (r) {
			DRM_DEBUG("hw_fini of IP block <%s> failed %d\n",
				  adev->ip_blocks[i].version->funcs->name, r);
		}

		adev->ip_blocks[i].status.hw = false;
	}

	for (i = adev->num_ip_blocks - 1; i >= 0; i--) {
		if (!adev->ip_blocks[i].status.sw)
			continue;
		r = adev->ip_blocks[i].version->funcs->sw_fini((void *)adev);
		/* XXX handle errors */
		if (r) {
			DRM_DEBUG("sw_fini of IP block <%s> failed %d\n",
				  adev->ip_blocks[i].version->funcs->name, r);
		}
		adev->ip_blocks[i].status.sw = false;
		adev->ip_blocks[i].status.valid = false;
	}

	for (i = adev->num_ip_blocks - 1; i >= 0; i--) {
		if (!adev->ip_blocks[i].status.late_initialized)
			continue;
		if (adev->ip_blocks[i].version->funcs->late_fini)
			adev->ip_blocks[i].version->funcs->late_fini((void *)adev);
		adev->ip_blocks[i].status.late_initialized = false;
	}

	if (amdgpu_sriov_vf(adev)) {
		amdgpu_bo_free_kernel(&adev->virt.csa_obj, &adev->virt.csa_vmid0_addr, NULL);
		amdgpu_virt_release_full_gpu(adev, false);
	}

	return 0;
}

static void amdgpu_late_init_func_handler(struct work_struct *work)
{
	struct amdgpu_device *adev =
		container_of(work, struct amdgpu_device, late_init_work.work);
	amdgpu_late_set_cg_state(adev);
}

int amdgpu_suspend(struct amdgpu_device *adev)
{
	int i, r;

	if (amdgpu_sriov_vf(adev))
		amdgpu_virt_request_full_gpu(adev, false);

	/* ungate SMC block first */
	r = amdgpu_set_clockgating_state(adev, AMD_IP_BLOCK_TYPE_SMC,
					 AMD_CG_STATE_UNGATE);
	if (r) {
		DRM_ERROR("set_clockgating_state(ungate) SMC failed %d\n",r);
	}

	for (i = adev->num_ip_blocks - 1; i >= 0; i--) {
		if (!adev->ip_blocks[i].status.valid)
			continue;
		/* ungate blocks so that suspend can properly shut them down */
		if (i != AMD_IP_BLOCK_TYPE_SMC) {
			r = adev->ip_blocks[i].version->funcs->set_clockgating_state((void *)adev,
										     AMD_CG_STATE_UNGATE);
			if (r) {
				DRM_ERROR("set_clockgating_state(ungate) of IP block <%s> failed %d\n",
					  adev->ip_blocks[i].version->funcs->name, r);
			}
		}
		/* XXX handle errors */
		r = adev->ip_blocks[i].version->funcs->suspend(adev);
		/* XXX handle errors */
		if (r) {
			DRM_ERROR("suspend of IP block <%s> failed %d\n",
				  adev->ip_blocks[i].version->funcs->name, r);
		}
	}

	if (amdgpu_sriov_vf(adev))
		amdgpu_virt_release_full_gpu(adev, false);

	return 0;
}

static int amdgpu_sriov_reinit_early(struct amdgpu_device *adev)
{
	int i, r;

	static enum amd_ip_block_type ip_order[] = {
		AMD_IP_BLOCK_TYPE_GMC,
		AMD_IP_BLOCK_TYPE_COMMON,
		AMD_IP_BLOCK_TYPE_IH,
	};

	for (i = 0; i < adev->num_ip_blocks; i++) {
		int j;
		struct amdgpu_ip_block *block;

		for (j = 0; j < adev->num_ip_blocks; j++) {
			block = &adev->ip_blocks[j];

			if (block->version->type != ip_order[i] ||
				!block->status.valid)
				continue;

			r = block->version->funcs->hw_init(adev);
			DRM_INFO("RE-INIT: %s %s\n", block->version->funcs->name, r?"failed":"successed");
		}
	}

	return 0;
}

static int amdgpu_sriov_reinit_late(struct amdgpu_device *adev)
{
	int i, r;

	static enum amd_ip_block_type ip_order[] = {
		AMD_IP_BLOCK_TYPE_SMC,
		AMD_IP_BLOCK_TYPE_DCE,
		AMD_IP_BLOCK_TYPE_GFX,
		AMD_IP_BLOCK_TYPE_SDMA,
		AMD_IP_BLOCK_TYPE_UVD,
		AMD_IP_BLOCK_TYPE_VCE
	};

	for (i = 0; i < ARRAY_SIZE(ip_order); i++) {
		int j;
		struct amdgpu_ip_block *block;

		for (j = 0; j < adev->num_ip_blocks; j++) {
			block = &adev->ip_blocks[j];

			if (block->version->type != ip_order[i] ||
				!block->status.valid)
				continue;

			r = block->version->funcs->hw_init(adev);
			DRM_INFO("RE-INIT: %s %s\n", block->version->funcs->name, r?"failed":"successed");
		}
	}

	return 0;
}

static int amdgpu_resume_phase1(struct amdgpu_device *adev)
{
	int i, r;

	for (i = 0; i < adev->num_ip_blocks; i++) {
		if (!adev->ip_blocks[i].status.valid)
			continue;
		if (adev->ip_blocks[i].version->type == AMD_IP_BLOCK_TYPE_COMMON ||
				adev->ip_blocks[i].version->type == AMD_IP_BLOCK_TYPE_GMC ||
				adev->ip_blocks[i].version->type ==
				AMD_IP_BLOCK_TYPE_IH) {
			r = adev->ip_blocks[i].version->funcs->resume(adev);
			if (r) {
				DRM_ERROR("resume of IP block <%s> failed %d\n",
					  adev->ip_blocks[i].version->funcs->name, r);
				return r;
			}
		}
	}

	return 0;
}

static int amdgpu_resume_phase2(struct amdgpu_device *adev)
{
	int i, r;

	for (i = 0; i < adev->num_ip_blocks; i++) {
		if (!adev->ip_blocks[i].status.valid)
			continue;
		if (adev->ip_blocks[i].version->type == AMD_IP_BLOCK_TYPE_COMMON ||
				adev->ip_blocks[i].version->type == AMD_IP_BLOCK_TYPE_GMC ||
				adev->ip_blocks[i].version->type == AMD_IP_BLOCK_TYPE_IH )
			continue;
		r = adev->ip_blocks[i].version->funcs->resume(adev);
		if (r) {
			DRM_ERROR("resume of IP block <%s> failed %d\n",
				  adev->ip_blocks[i].version->funcs->name, r);
			return r;
		}
	}

	return 0;
}

static int amdgpu_resume(struct amdgpu_device *adev)
{
	int r;

	r = amdgpu_resume_phase1(adev);
	if (r)
		return r;
	r = amdgpu_resume_phase2(adev);

	return r;
}

static void amdgpu_device_detect_sriov_bios(struct amdgpu_device *adev)
{
	if (adev->is_atom_fw) {
		if (amdgpu_atomfirmware_gpu_supports_virtualization(adev))
			adev->virt.caps |= AMDGPU_SRIOV_CAPS_SRIOV_VBIOS;
	} else {
		if (amdgpu_atombios_has_gpu_virtualization_table(adev))
			adev->virt.caps |= AMDGPU_SRIOV_CAPS_SRIOV_VBIOS;
	}
}

/**
 * amdgpu_device_init - initialize the driver
 *
 * @adev: amdgpu_device pointer
 * @pdev: drm dev pointer
 * @pdev: pci dev pointer
 * @flags: driver flags
 *
 * Initializes the driver info and hw (all asics).
 * Returns 0 for success or an error on failure.
 * Called at driver startup.
 */
int amdgpu_device_init(struct amdgpu_device *adev,
		       struct drm_device *ddev,
		       struct pci_dev *pdev,
		       uint32_t flags)
{
	int r, i;
	bool runtime = false;
	u32 max_MBps;

	adev->shutdown = false;
	adev->dev = &pdev->dev;
	adev->ddev = ddev;
	adev->pdev = pdev;
	adev->flags = flags;
	adev->asic_type = flags & AMD_ASIC_MASK;
	adev->usec_timeout = AMDGPU_MAX_USEC_TIMEOUT;
	adev->mc.gart_size = 512 * 1024 * 1024;
	adev->accel_working = false;
	adev->num_rings = 0;
	adev->mman.buffer_funcs = NULL;
	adev->mman.buffer_funcs_ring = NULL;
	adev->vm_manager.vm_pte_funcs = NULL;
	adev->vm_manager.vm_pte_num_rings = 0;
	adev->gart.gart_funcs = NULL;
	adev->fence_context = dma_fence_context_alloc(AMDGPU_MAX_RINGS);

	adev->smc_rreg = &amdgpu_invalid_rreg;
	adev->smc_wreg = &amdgpu_invalid_wreg;
	adev->pcie_rreg = &amdgpu_invalid_rreg;
	adev->pcie_wreg = &amdgpu_invalid_wreg;
	adev->pciep_rreg = &amdgpu_invalid_rreg;
	adev->pciep_wreg = &amdgpu_invalid_wreg;
	adev->uvd_ctx_rreg = &amdgpu_invalid_rreg;
	adev->uvd_ctx_wreg = &amdgpu_invalid_wreg;
	adev->didt_rreg = &amdgpu_invalid_rreg;
	adev->didt_wreg = &amdgpu_invalid_wreg;
	adev->gc_cac_rreg = &amdgpu_invalid_rreg;
	adev->gc_cac_wreg = &amdgpu_invalid_wreg;
	adev->audio_endpt_rreg = &amdgpu_block_invalid_rreg;
	adev->audio_endpt_wreg = &amdgpu_block_invalid_wreg;


	DRM_INFO("initializing kernel modesetting (%s 0x%04X:0x%04X 0x%04X:0x%04X 0x%02X).\n",
		 amdgpu_asic_name[adev->asic_type], pdev->vendor, pdev->device,
		 pdev->subsystem_vendor, pdev->subsystem_device, pdev->revision);

	/* mutex initialization are all done here so we
	 * can recall function without having locking issues */
	atomic_set(&adev->irq.ih.lock, 0);
	mutex_init(&adev->firmware.mutex);
	mutex_init(&adev->pm.mutex);
	mutex_init(&adev->gfx.gpu_clock_mutex);
	mutex_init(&adev->srbm_mutex);
	mutex_init(&adev->grbm_idx_mutex);
	mutex_init(&adev->mn_lock);
	hash_init(adev->mn_hash);

	amdgpu_check_arguments(adev);

	spin_lock_init(&adev->mmio_idx_lock);
	spin_lock_init(&adev->smc_idx_lock);
	spin_lock_init(&adev->pcie_idx_lock);
	spin_lock_init(&adev->uvd_ctx_idx_lock);
	spin_lock_init(&adev->didt_idx_lock);
	spin_lock_init(&adev->gc_cac_idx_lock);
	spin_lock_init(&adev->se_cac_idx_lock);
	spin_lock_init(&adev->audio_endpt_idx_lock);
	spin_lock_init(&adev->mm_stats.lock);

	INIT_LIST_HEAD(&adev->shadow_list);
	mutex_init(&adev->shadow_list_lock);

	INIT_LIST_HEAD(&adev->gtt_list);
	spin_lock_init(&adev->gtt_list_lock);

	INIT_LIST_HEAD(&adev->ring_lru_list);
	spin_lock_init(&adev->ring_lru_list_lock);

	INIT_DELAYED_WORK(&adev->late_init_work, amdgpu_late_init_func_handler);

	/* Registers mapping */
	/* TODO: block userspace mapping of io register */
	if (adev->asic_type >= CHIP_BONAIRE) {
		adev->rmmio_base = pci_resource_start(adev->pdev, 5);
		adev->rmmio_size = pci_resource_len(adev->pdev, 5);
	} else {
		adev->rmmio_base = pci_resource_start(adev->pdev, 2);
		adev->rmmio_size = pci_resource_len(adev->pdev, 2);
	}

	adev->rmmio = ioremap(adev->rmmio_base, adev->rmmio_size);
	if (adev->rmmio == NULL) {
		return -ENOMEM;
	}
	DRM_INFO("register mmio base: 0x%08X\n", (uint32_t)adev->rmmio_base);
	DRM_INFO("register mmio size: %u\n", (unsigned)adev->rmmio_size);

	if (adev->asic_type >= CHIP_BONAIRE)
		/* doorbell bar mapping */
		amdgpu_doorbell_init(adev);

	/* io port mapping */
	for (i = 0; i < DEVICE_COUNT_RESOURCE; i++) {
		if (pci_resource_flags(adev->pdev, i) & IORESOURCE_IO) {
			adev->rio_mem_size = pci_resource_len(adev->pdev, i);
			adev->rio_mem = pci_iomap(adev->pdev, i, adev->rio_mem_size);
			break;
		}
	}
	if (adev->rio_mem == NULL)
		DRM_INFO("PCI I/O BAR is not found.\n");

	/* early init functions */
	r = amdgpu_early_init(adev);
	if (r)
		return r;

	/* if we have > 1 VGA cards, then disable the amdgpu VGA resources */
	/* this will fail for cards that aren't VGA class devices, just
	 * ignore it */
	vga_client_register(adev->pdev, adev, NULL, amdgpu_vga_set_decode);

	if (amdgpu_device_is_px(ddev))
		runtime = true;
	if (!pci_is_thunderbolt_attached(adev->pdev))
		vga_switcheroo_register_client(adev->pdev,
					       &amdgpu_switcheroo_ops, runtime);
	if (runtime)
		vga_switcheroo_init_domain_pm_ops(adev->dev, &adev->vga_pm_domain);

	/* Read BIOS */
	if (!amdgpu_get_bios(adev)) {
		r = -EINVAL;
		goto failed;
	}

	r = amdgpu_atombios_init(adev);
	if (r) {
		dev_err(adev->dev, "amdgpu_atombios_init failed\n");
		amdgpu_vf_error_put(AMDGIM_ERROR_VF_ATOMBIOS_INIT_FAIL, 0, 0);
		goto failed;
	}

	/* detect if we are with an SRIOV vbios */
	amdgpu_device_detect_sriov_bios(adev);

	/* Post card if necessary */
	if (amdgpu_vpost_needed(adev)) {
		if (!adev->bios) {
			dev_err(adev->dev, "no vBIOS found\n");
			amdgpu_vf_error_put(AMDGIM_ERROR_VF_NO_VBIOS, 0, 0);
			r = -EINVAL;
			goto failed;
		}
		DRM_INFO("GPU posting now...\n");
		r = amdgpu_atom_asic_init(adev->mode_info.atom_context);
		if (r) {
			dev_err(adev->dev, "gpu post error!\n");
			amdgpu_vf_error_put(AMDGIM_ERROR_VF_GPU_POST_ERROR, 0, 0);
			goto failed;
		}
	} else {
		DRM_INFO("GPU post is not needed\n");
	}

	if (adev->is_atom_fw) {
		/* Initialize clocks */
		r = amdgpu_atomfirmware_get_clock_info(adev);
		if (r) {
			dev_err(adev->dev, "amdgpu_atomfirmware_get_clock_info failed\n");
			amdgpu_vf_error_put(AMDGIM_ERROR_VF_ATOMBIOS_GET_CLOCK_FAIL, 0, 0);
			goto failed;
		}
	} else {
		/* Initialize clocks */
		r = amdgpu_atombios_get_clock_info(adev);
		if (r) {
			dev_err(adev->dev, "amdgpu_atombios_get_clock_info failed\n");
			amdgpu_vf_error_put(AMDGIM_ERROR_VF_ATOMBIOS_GET_CLOCK_FAIL, 0, 0);
			goto failed;
		}
		/* init i2c buses */
		amdgpu_atombios_i2c_init(adev);
	}

	/* Fence driver */
	r = amdgpu_fence_driver_init(adev);
	if (r) {
		dev_err(adev->dev, "amdgpu_fence_driver_init failed\n");
		amdgpu_vf_error_put(AMDGIM_ERROR_VF_FENCE_INIT_FAIL, 0, 0);
		goto failed;
	}

	/* init the mode config */
	drm_mode_config_init(adev->ddev);

	r = amdgpu_init(adev);
	if (r) {
		dev_err(adev->dev, "amdgpu_init failed\n");
		amdgpu_vf_error_put(AMDGIM_ERROR_VF_AMDGPU_INIT_FAIL, 0, 0);
		amdgpu_fini(adev);
		goto failed;
	}

	adev->accel_working = true;

	amdgpu_vm_check_compute_bug(adev);

	/* Initialize the buffer migration limit. */
	if (amdgpu_moverate >= 0)
		max_MBps = amdgpu_moverate;
	else
		max_MBps = 8; /* Allow 8 MB/s. */
	/* Get a log2 for easy divisions. */
	adev->mm_stats.log2_max_MBps = ilog2(max(1u, max_MBps));

	r = amdgpu_ib_pool_init(adev);
	if (r) {
		dev_err(adev->dev, "IB initialization failed (%d).\n", r);
		amdgpu_vf_error_put(AMDGIM_ERROR_VF_IB_INIT_FAIL, 0, r);
		goto failed;
	}

	r = amdgpu_ib_ring_tests(adev);
	if (r)
		DRM_ERROR("ib ring test failed (%d).\n", r);

	amdgpu_fbdev_init(adev);

	r = amdgpu_gem_debugfs_init(adev);
	if (r)
		DRM_ERROR("registering gem debugfs failed (%d).\n", r);

	r = amdgpu_debugfs_regs_init(adev);
	if (r)
		DRM_ERROR("registering register debugfs failed (%d).\n", r);

	r = amdgpu_debugfs_test_ib_ring_init(adev);
	if (r)
		DRM_ERROR("registering register test ib ring debugfs failed (%d).\n", r);

	r = amdgpu_debugfs_firmware_init(adev);
	if (r)
		DRM_ERROR("registering firmware debugfs failed (%d).\n", r);

	if ((amdgpu_testing & 1)) {
		if (adev->accel_working)
			amdgpu_test_moves(adev);
		else
			DRM_INFO("amdgpu: acceleration disabled, skipping move tests\n");
	}
	if (amdgpu_benchmarking) {
		if (adev->accel_working)
			amdgpu_benchmark(adev, amdgpu_benchmarking);
		else
			DRM_INFO("amdgpu: acceleration disabled, skipping benchmarks\n");
	}

	/* enable clockgating, etc. after ib tests, etc. since some blocks require
	 * explicit gating rather than handling it automatically.
	 */
	r = amdgpu_late_init(adev);
	if (r) {
		dev_err(adev->dev, "amdgpu_late_init failed\n");
		amdgpu_vf_error_put(AMDGIM_ERROR_VF_AMDGPU_LATE_INIT_FAIL, 0, r);
		goto failed;
	}

	return 0;

failed:
	amdgpu_vf_error_trans_all(adev);
	if (runtime)
		vga_switcheroo_fini_domain_pm_ops(adev->dev);
	return r;
}

/**
 * amdgpu_device_fini - tear down the driver
 *
 * @adev: amdgpu_device pointer
 *
 * Tear down the driver info (all asics).
 * Called at driver shutdown.
 */
void amdgpu_device_fini(struct amdgpu_device *adev)
{
	int r;

	DRM_INFO("amdgpu: finishing device.\n");
	adev->shutdown = true;
	if (adev->mode_info.mode_config_initialized)
		drm_crtc_force_disable_all(adev->ddev);
	/* evict vram memory */
	amdgpu_bo_evict_vram(adev);
	amdgpu_ib_pool_fini(adev);
	amdgpu_fence_driver_fini(adev);
	amdgpu_fbdev_fini(adev);
	r = amdgpu_fini(adev);
	if (adev->firmware.gpu_info_fw) {
		release_firmware(adev->firmware.gpu_info_fw);
		adev->firmware.gpu_info_fw = NULL;
	}
	adev->accel_working = false;
	cancel_delayed_work_sync(&adev->late_init_work);
	/* free i2c buses */
	amdgpu_i2c_fini(adev);
	amdgpu_atombios_fini(adev);
	kfree(adev->bios);
	adev->bios = NULL;
	if (!pci_is_thunderbolt_attached(adev->pdev))
		vga_switcheroo_unregister_client(adev->pdev);
	if (adev->flags & AMD_IS_PX)
		vga_switcheroo_fini_domain_pm_ops(adev->dev);
	vga_client_register(adev->pdev, NULL, NULL, NULL);
	if (adev->rio_mem)
		pci_iounmap(adev->pdev, adev->rio_mem);
	adev->rio_mem = NULL;
	iounmap(adev->rmmio);
	adev->rmmio = NULL;
	if (adev->asic_type >= CHIP_BONAIRE)
		amdgpu_doorbell_fini(adev);
	amdgpu_debugfs_regs_cleanup(adev);
}


/*
 * Suspend & resume.
 */
/**
 * amdgpu_device_suspend - initiate device suspend
 *
 * @pdev: drm dev pointer
 * @state: suspend state
 *
 * Puts the hw in the suspend state (all asics).
 * Returns 0 for success or an error on failure.
 * Called at driver suspend.
 */
int amdgpu_device_suspend(struct drm_device *dev, bool suspend, bool fbcon)
{
	struct amdgpu_device *adev;
	struct drm_crtc *crtc;
	struct drm_connector *connector;
	int r;

	if (dev == NULL || dev->dev_private == NULL) {
		return -ENODEV;
	}

	adev = dev->dev_private;

	if (dev->switch_power_state == DRM_SWITCH_POWER_OFF)
		return 0;

	drm_kms_helper_poll_disable(dev);

	/* turn off display hw */
	drm_modeset_lock_all(dev);
	list_for_each_entry(connector, &dev->mode_config.connector_list, head) {
		drm_helper_connector_dpms(connector, DRM_MODE_DPMS_OFF);
	}
	drm_modeset_unlock_all(dev);

	amdgpu_amdkfd_suspend(adev);

	/* unpin the front buffers and cursors */
	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		struct amdgpu_crtc *amdgpu_crtc = to_amdgpu_crtc(crtc);
		struct amdgpu_framebuffer *rfb = to_amdgpu_framebuffer(crtc->primary->fb);
		struct amdgpu_bo *robj;

		if (amdgpu_crtc->cursor_bo) {
			struct amdgpu_bo *aobj = gem_to_amdgpu_bo(amdgpu_crtc->cursor_bo);
			r = amdgpu_bo_reserve(aobj, true);
			if (r == 0) {
				amdgpu_bo_unpin(aobj);
				amdgpu_bo_unreserve(aobj);
			}
		}

		if (rfb == NULL || rfb->obj == NULL) {
			continue;
		}
		robj = gem_to_amdgpu_bo(rfb->obj);
		/* don't unpin kernel fb objects */
		if (!amdgpu_fbdev_robj_is_fb(adev, robj)) {
			r = amdgpu_bo_reserve(robj, true);
			if (r == 0) {
				amdgpu_bo_unpin(robj);
				amdgpu_bo_unreserve(robj);
			}
		}
	}
	/* evict vram memory */
	amdgpu_bo_evict_vram(adev);

	amdgpu_fence_driver_suspend(adev);

	r = amdgpu_suspend(adev);

	/* evict remaining vram memory
	 * This second call to evict vram is to evict the gart page table
	 * using the CPU.
	 */
	amdgpu_bo_evict_vram(adev);

	amdgpu_atombios_scratch_regs_save(adev);
	pci_save_state(dev->pdev);
	if (suspend) {
		/* Shut down the device */
		pci_disable_device(dev->pdev);
		pci_set_power_state(dev->pdev, PCI_D3hot);
	} else {
		r = amdgpu_asic_reset(adev);
		if (r)
			DRM_ERROR("amdgpu asic reset failed\n");
	}

	if (fbcon) {
		console_lock();
		amdgpu_fbdev_set_suspend(adev, 1);
		console_unlock();
	}
	return 0;
}

/**
 * amdgpu_device_resume - initiate device resume
 *
 * @pdev: drm dev pointer
 *
 * Bring the hw back to operating state (all asics).
 * Returns 0 for success or an error on failure.
 * Called at driver resume.
 */
int amdgpu_device_resume(struct drm_device *dev, bool resume, bool fbcon)
{
	struct drm_connector *connector;
	struct amdgpu_device *adev = dev->dev_private;
	struct drm_crtc *crtc;
	int r = 0;

	if (dev->switch_power_state == DRM_SWITCH_POWER_OFF)
		return 0;

	if (fbcon)
		console_lock();

	if (resume) {
		pci_set_power_state(dev->pdev, PCI_D0);
		pci_restore_state(dev->pdev);
		r = pci_enable_device(dev->pdev);
		if (r)
			goto unlock;
	}
	amdgpu_atombios_scratch_regs_restore(adev);

	/* post card */
	if (amdgpu_need_post(adev)) {
		r = amdgpu_atom_asic_init(adev->mode_info.atom_context);
		if (r)
			DRM_ERROR("amdgpu asic init failed\n");
	}

	r = amdgpu_resume(adev);
	if (r) {
		DRM_ERROR("amdgpu_resume failed (%d).\n", r);
		goto unlock;
	}
	amdgpu_fence_driver_resume(adev);

	if (resume) {
		r = amdgpu_ib_ring_tests(adev);
		if (r)
			DRM_ERROR("ib ring test failed (%d).\n", r);
	}

	r = amdgpu_late_init(adev);
	if (r)
		goto unlock;

	/* pin cursors */
	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		struct amdgpu_crtc *amdgpu_crtc = to_amdgpu_crtc(crtc);

		if (amdgpu_crtc->cursor_bo) {
			struct amdgpu_bo *aobj = gem_to_amdgpu_bo(amdgpu_crtc->cursor_bo);
			r = amdgpu_bo_reserve(aobj, true);
			if (r == 0) {
				r = amdgpu_bo_pin(aobj,
						  AMDGPU_GEM_DOMAIN_VRAM,
						  &amdgpu_crtc->cursor_addr);
				if (r != 0)
					DRM_ERROR("Failed to pin cursor BO (%d)\n", r);
				amdgpu_bo_unreserve(aobj);
			}
		}
	}
	r = amdgpu_amdkfd_resume(adev);
	if (r)
		return r;

	/* blat the mode back in */
	if (fbcon) {
		drm_helper_resume_force_mode(dev);
		/* turn on display hw */
		drm_modeset_lock_all(dev);
		list_for_each_entry(connector, &dev->mode_config.connector_list, head) {
			drm_helper_connector_dpms(connector, DRM_MODE_DPMS_ON);
		}
		drm_modeset_unlock_all(dev);
	}

	drm_kms_helper_poll_enable(dev);

	/*
	 * Most of the connector probing functions try to acquire runtime pm
	 * refs to ensure that the GPU is powered on when connector polling is
	 * performed. Since we're calling this from a runtime PM callback,
	 * trying to acquire rpm refs will cause us to deadlock.
	 *
	 * Since we're guaranteed to be holding the rpm lock, it's safe to
	 * temporarily disable the rpm helpers so this doesn't deadlock us.
	 */
#ifdef CONFIG_PM
	dev->dev->power.disable_depth++;
#endif
	drm_helper_hpd_irq_event(dev);
#ifdef CONFIG_PM
	dev->dev->power.disable_depth--;
#endif

	if (fbcon)
		amdgpu_fbdev_set_suspend(adev, 0);

unlock:
	if (fbcon)
		console_unlock();

	return r;
}

static bool amdgpu_check_soft_reset(struct amdgpu_device *adev)
{
	int i;
	bool asic_hang = false;

	for (i = 0; i < adev->num_ip_blocks; i++) {
		if (!adev->ip_blocks[i].status.valid)
			continue;
		if (adev->ip_blocks[i].version->funcs->check_soft_reset)
			adev->ip_blocks[i].status.hang =
				adev->ip_blocks[i].version->funcs->check_soft_reset(adev);
		if (adev->ip_blocks[i].status.hang) {
			DRM_INFO("IP block:%s is hung!\n", adev->ip_blocks[i].version->funcs->name);
			asic_hang = true;
		}
	}
	return asic_hang;
}

static int amdgpu_pre_soft_reset(struct amdgpu_device *adev)
{
	int i, r = 0;

	for (i = 0; i < adev->num_ip_blocks; i++) {
		if (!adev->ip_blocks[i].status.valid)
			continue;
		if (adev->ip_blocks[i].status.hang &&
		    adev->ip_blocks[i].version->funcs->pre_soft_reset) {
			r = adev->ip_blocks[i].version->funcs->pre_soft_reset(adev);
			if (r)
				return r;
		}
	}

	return 0;
}

static bool amdgpu_need_full_reset(struct amdgpu_device *adev)
{
	int i;

	for (i = 0; i < adev->num_ip_blocks; i++) {
		if (!adev->ip_blocks[i].status.valid)
			continue;
		if ((adev->ip_blocks[i].version->type == AMD_IP_BLOCK_TYPE_GMC) ||
		    (adev->ip_blocks[i].version->type == AMD_IP_BLOCK_TYPE_SMC) ||
		    (adev->ip_blocks[i].version->type == AMD_IP_BLOCK_TYPE_ACP) ||
		    (adev->ip_blocks[i].version->type == AMD_IP_BLOCK_TYPE_DCE)) {
			if (adev->ip_blocks[i].status.hang) {
				DRM_INFO("Some block need full reset!\n");
				return true;
			}
		}
	}
	return false;
}

static int amdgpu_soft_reset(struct amdgpu_device *adev)
{
	int i, r = 0;

	for (i = 0; i < adev->num_ip_blocks; i++) {
		if (!adev->ip_blocks[i].status.valid)
			continue;
		if (adev->ip_blocks[i].status.hang &&
		    adev->ip_blocks[i].version->funcs->soft_reset) {
			r = adev->ip_blocks[i].version->funcs->soft_reset(adev);
			if (r)
				return r;
		}
	}

	return 0;
}

static int amdgpu_post_soft_reset(struct amdgpu_device *adev)
{
	int i, r = 0;

	for (i = 0; i < adev->num_ip_blocks; i++) {
		if (!adev->ip_blocks[i].status.valid)
			continue;
		if (adev->ip_blocks[i].status.hang &&
		    adev->ip_blocks[i].version->funcs->post_soft_reset)
			r = adev->ip_blocks[i].version->funcs->post_soft_reset(adev);
		if (r)
			return r;
	}

	return 0;
}

bool amdgpu_need_backup(struct amdgpu_device *adev)
{
	if (adev->flags & AMD_IS_APU)
		return false;

	return amdgpu_lockup_timeout > 0 ? true : false;
}

static int amdgpu_recover_vram_from_shadow(struct amdgpu_device *adev,
					   struct amdgpu_ring *ring,
					   struct amdgpu_bo *bo,
					   struct dma_fence **fence)
{
	uint32_t domain;
	int r;

	if (!bo->shadow)
		return 0;

	r = amdgpu_bo_reserve(bo, true);
	if (r)
		return r;
	domain = amdgpu_mem_type_to_domain(bo->tbo.mem.mem_type);
	/* if bo has been evicted, then no need to recover */
	if (domain == AMDGPU_GEM_DOMAIN_VRAM) {
		r = amdgpu_bo_validate(bo->shadow);
		if (r) {
			DRM_ERROR("bo validate failed!\n");
			goto err;
		}

		r = amdgpu_bo_restore_from_shadow(adev, ring, bo,
						 NULL, fence, true);
		if (r) {
			DRM_ERROR("recover page table failed!\n");
			goto err;
		}
	}
err:
	amdgpu_bo_unreserve(bo);
	return r;
}

/**
 * amdgpu_sriov_gpu_reset - reset the asic
 *
 * @adev: amdgpu device pointer
 * @job: which job trigger hang
 *
 * Attempt the reset the GPU if it has hung (all asics).
 * for SRIOV case.
 * Returns 0 for success or an error on failure.
 */
int amdgpu_sriov_gpu_reset(struct amdgpu_device *adev, struct amdgpu_job *job)
{
	int i, j, r = 0;
	int resched;
	struct amdgpu_bo *bo, *tmp;
	struct amdgpu_ring *ring;
	struct dma_fence *fence = NULL, *next = NULL;

	mutex_lock(&adev->virt.lock_reset);
	atomic_inc(&adev->gpu_reset_counter);
	adev->gfx.in_reset = true;

	/* block TTM */
	resched = ttm_bo_lock_delayed_workqueue(&adev->mman.bdev);

	/* we start from the ring trigger GPU hang */
	j = job ? job->ring->idx : 0;

	/* block scheduler */
	for (i = j; i < j + AMDGPU_MAX_RINGS; ++i) {
		ring = adev->rings[i % AMDGPU_MAX_RINGS];
		if (!ring || !ring->sched.thread)
			continue;

		kthread_park(ring->sched.thread);

		if (job && j != i)
			continue;

		/* here give the last chance to check if job removed from mirror-list
		 * since we already pay some time on kthread_park */
		if (job && list_empty(&job->base.node)) {
			kthread_unpark(ring->sched.thread);
			goto give_up_reset;
		}

		if (amd_sched_invalidate_job(&job->base, amdgpu_job_hang_limit))
			amd_sched_job_kickout(&job->base);

		/* only do job_reset on the hang ring if @job not NULL */
		amd_sched_hw_job_reset(&ring->sched);

		/* after all hw jobs are reset, hw fence is meaningless, so force_completion */
		amdgpu_fence_driver_force_completion_ring(ring);
	}

	/* request to take full control of GPU before re-initialization  */
	if (job)
		amdgpu_virt_reset_gpu(adev);
	else
		amdgpu_virt_request_full_gpu(adev, true);


	/* Resume IP prior to SMC */
	amdgpu_sriov_reinit_early(adev);

	/* we need recover gart prior to run SMC/CP/SDMA resume */
	amdgpu_ttm_recover_gart(adev);

	/* now we are okay to resume SMC/CP/SDMA */
	amdgpu_sriov_reinit_late(adev);

	amdgpu_irq_gpu_reset_resume_helper(adev);

	if (amdgpu_ib_ring_tests(adev))
		dev_err(adev->dev, "[GPU_RESET] ib ring test failed (%d).\n", r);

	/* release full control of GPU after ib test */
	amdgpu_virt_release_full_gpu(adev, true);

	DRM_INFO("recover vram bo from shadow\n");

	ring = adev->mman.buffer_funcs_ring;
	mutex_lock(&adev->shadow_list_lock);
	list_for_each_entry_safe(bo, tmp, &adev->shadow_list, shadow_list) {
		next = NULL;
		amdgpu_recover_vram_from_shadow(adev, ring, bo, &next);
		if (fence) {
			r = dma_fence_wait(fence, false);
			if (r) {
				WARN(r, "recovery from shadow isn't completed\n");
				break;
			}
		}

		dma_fence_put(fence);
		fence = next;
	}
	mutex_unlock(&adev->shadow_list_lock);

	if (fence) {
		r = dma_fence_wait(fence, false);
		if (r)
			WARN(r, "recovery from shadow isn't completed\n");
	}
	dma_fence_put(fence);

	for (i = j; i < j + AMDGPU_MAX_RINGS; ++i) {
		ring = adev->rings[i % AMDGPU_MAX_RINGS];
		if (!ring || !ring->sched.thread)
			continue;

		if (job && j != i) {
			kthread_unpark(ring->sched.thread);
			continue;
		}

		amd_sched_job_recovery(&ring->sched);
		kthread_unpark(ring->sched.thread);
	}

	drm_helper_resume_force_mode(adev->ddev);
give_up_reset:
	ttm_bo_unlock_delayed_workqueue(&adev->mman.bdev, resched);
	if (r) {
		/* bad news, how to tell it to userspace ? */
		dev_info(adev->dev, "GPU reset failed\n");
	} else {
		dev_info(adev->dev, "GPU reset successed!\n");
	}

	adev->gfx.in_reset = false;
	mutex_unlock(&adev->virt.lock_reset);
	return r;
}

/**
 * amdgpu_gpu_reset - reset the asic
 *
 * @adev: amdgpu device pointer
 *
 * Attempt the reset the GPU if it has hung (all asics).
 * Returns 0 for success or an error on failure.
 */
int amdgpu_gpu_reset(struct amdgpu_device *adev)
{
	int i, r;
	int resched;
	bool need_full_reset, vram_lost = false;

	if (!amdgpu_check_soft_reset(adev)) {
		DRM_INFO("No hardware hang detected. Did some blocks stall?\n");
		return 0;
	}

	atomic_inc(&adev->gpu_reset_counter);

	/* block TTM */
	resched = ttm_bo_lock_delayed_workqueue(&adev->mman.bdev);

	/* block scheduler */
	for (i = 0; i < AMDGPU_MAX_RINGS; ++i) {
		struct amdgpu_ring *ring = adev->rings[i];

		if (!ring || !ring->sched.thread)
			continue;
		kthread_park(ring->sched.thread);
		amd_sched_hw_job_reset(&ring->sched);
	}
	/* after all hw jobs are reset, hw fence is meaningless, so force_completion */
	amdgpu_fence_driver_force_completion(adev);

	need_full_reset = amdgpu_need_full_reset(adev);

	if (!need_full_reset) {
		amdgpu_pre_soft_reset(adev);
		r = amdgpu_soft_reset(adev);
		amdgpu_post_soft_reset(adev);
		if (r || amdgpu_check_soft_reset(adev)) {
			DRM_INFO("soft reset failed, will fallback to full reset!\n");
			need_full_reset = true;
		}
	}

	if (need_full_reset) {
		r = amdgpu_suspend(adev);

retry:
		amdgpu_atombios_scratch_regs_save(adev);
		r = amdgpu_asic_reset(adev);
		amdgpu_atombios_scratch_regs_restore(adev);
		/* post card */
		amdgpu_atom_asic_init(adev->mode_info.atom_context);

		if (!r) {
			dev_info(adev->dev, "GPU reset succeeded, trying to resume\n");
			r = amdgpu_resume_phase1(adev);
			if (r)
				goto out;
			vram_lost = amdgpu_check_vram_lost(adev);
			if (vram_lost) {
				DRM_ERROR("VRAM is lost!\n");
				atomic_inc(&adev->vram_lost_counter);
			}
			r = amdgpu_ttm_recover_gart(adev);
			if (r)
				goto out;
			r = amdgpu_resume_phase2(adev);
			if (r)
				goto out;
			if (vram_lost)
				amdgpu_fill_reset_magic(adev);
		}
	}
out:
	if (!r) {
		amdgpu_irq_gpu_reset_resume_helper(adev);
		r = amdgpu_ib_ring_tests(adev);
		if (r) {
			dev_err(adev->dev, "ib ring test failed (%d).\n", r);
			r = amdgpu_suspend(adev);
			need_full_reset = true;
			goto retry;
		}
		/**
		 * recovery vm page tables, since we cannot depend on VRAM is
		 * consistent after gpu full reset.
		 */
		if (need_full_reset && amdgpu_need_backup(adev)) {
			struct amdgpu_ring *ring = adev->mman.buffer_funcs_ring;
			struct amdgpu_bo *bo, *tmp;
			struct dma_fence *fence = NULL, *next = NULL;

			DRM_INFO("recover vram bo from shadow\n");
			mutex_lock(&adev->shadow_list_lock);
			list_for_each_entry_safe(bo, tmp, &adev->shadow_list, shadow_list) {
				next = NULL;
				amdgpu_recover_vram_from_shadow(adev, ring, bo, &next);
				if (fence) {
					r = dma_fence_wait(fence, false);
					if (r) {
						WARN(r, "recovery from shadow isn't completed\n");
						break;
					}
				}

				dma_fence_put(fence);
				fence = next;
			}
			mutex_unlock(&adev->shadow_list_lock);
			if (fence) {
				r = dma_fence_wait(fence, false);
				if (r)
					WARN(r, "recovery from shadow isn't completed\n");
			}
			dma_fence_put(fence);
		}
		for (i = 0; i < AMDGPU_MAX_RINGS; ++i) {
			struct amdgpu_ring *ring = adev->rings[i];

			if (!ring || !ring->sched.thread)
				continue;

			amd_sched_job_recovery(&ring->sched);
			kthread_unpark(ring->sched.thread);
		}
	} else {
		dev_err(adev->dev, "asic resume failed (%d).\n", r);
		amdgpu_vf_error_put(AMDGIM_ERROR_VF_ASIC_RESUME_FAIL, 0, r);
		for (i = 0; i < AMDGPU_MAX_RINGS; ++i) {
			if (adev->rings[i] && adev->rings[i]->sched.thread) {
				kthread_unpark(adev->rings[i]->sched.thread);
			}
		}
	}

	drm_helper_resume_force_mode(adev->ddev);

	ttm_bo_unlock_delayed_workqueue(&adev->mman.bdev, resched);
	if (r) {
		/* bad news, how to tell it to userspace ? */
		dev_info(adev->dev, "GPU reset failed\n");
		amdgpu_vf_error_put(AMDGIM_ERROR_VF_GPU_RESET_FAIL, 0, r);
	}
	else {
		dev_info(adev->dev, "GPU reset successed!\n");
	}

	amdgpu_vf_error_trans_all(adev);
	return r;
}

void amdgpu_get_pcie_info(struct amdgpu_device *adev)
{
	u32 mask;
	int ret;

	if (amdgpu_pcie_gen_cap)
		adev->pm.pcie_gen_mask = amdgpu_pcie_gen_cap;

	if (amdgpu_pcie_lane_cap)
		adev->pm.pcie_mlw_mask = amdgpu_pcie_lane_cap;

	/* covers APUs as well */
	if (pci_is_root_bus(adev->pdev->bus)) {
		if (adev->pm.pcie_gen_mask == 0)
			adev->pm.pcie_gen_mask = AMDGPU_DEFAULT_PCIE_GEN_MASK;
		if (adev->pm.pcie_mlw_mask == 0)
			adev->pm.pcie_mlw_mask = AMDGPU_DEFAULT_PCIE_MLW_MASK;
		return;
	}

	if (adev->pm.pcie_gen_mask == 0) {
		ret = drm_pcie_get_speed_cap_mask(adev->ddev, &mask);
		if (!ret) {
			adev->pm.pcie_gen_mask = (CAIL_ASIC_PCIE_LINK_SPEED_SUPPORT_GEN1 |
						  CAIL_ASIC_PCIE_LINK_SPEED_SUPPORT_GEN2 |
						  CAIL_ASIC_PCIE_LINK_SPEED_SUPPORT_GEN3);

			if (mask & DRM_PCIE_SPEED_25)
				adev->pm.pcie_gen_mask |= CAIL_PCIE_LINK_SPEED_SUPPORT_GEN1;
			if (mask & DRM_PCIE_SPEED_50)
				adev->pm.pcie_gen_mask |= CAIL_PCIE_LINK_SPEED_SUPPORT_GEN2;
			if (mask & DRM_PCIE_SPEED_80)
				adev->pm.pcie_gen_mask |= CAIL_PCIE_LINK_SPEED_SUPPORT_GEN3;
		} else {
			adev->pm.pcie_gen_mask = AMDGPU_DEFAULT_PCIE_GEN_MASK;
		}
	}
	if (adev->pm.pcie_mlw_mask == 0) {
		ret = drm_pcie_get_max_link_width(adev->ddev, &mask);
		if (!ret) {
			switch (mask) {
			case 32:
				adev->pm.pcie_mlw_mask = (CAIL_PCIE_LINK_WIDTH_SUPPORT_X32 |
							  CAIL_PCIE_LINK_WIDTH_SUPPORT_X16 |
							  CAIL_PCIE_LINK_WIDTH_SUPPORT_X12 |
							  CAIL_PCIE_LINK_WIDTH_SUPPORT_X8 |
							  CAIL_PCIE_LINK_WIDTH_SUPPORT_X4 |
							  CAIL_PCIE_LINK_WIDTH_SUPPORT_X2 |
							  CAIL_PCIE_LINK_WIDTH_SUPPORT_X1);
				break;
			case 16:
				adev->pm.pcie_mlw_mask = (CAIL_PCIE_LINK_WIDTH_SUPPORT_X16 |
							  CAIL_PCIE_LINK_WIDTH_SUPPORT_X12 |
							  CAIL_PCIE_LINK_WIDTH_SUPPORT_X8 |
							  CAIL_PCIE_LINK_WIDTH_SUPPORT_X4 |
							  CAIL_PCIE_LINK_WIDTH_SUPPORT_X2 |
							  CAIL_PCIE_LINK_WIDTH_SUPPORT_X1);
				break;
			case 12:
				adev->pm.pcie_mlw_mask = (CAIL_PCIE_LINK_WIDTH_SUPPORT_X12 |
							  CAIL_PCIE_LINK_WIDTH_SUPPORT_X8 |
							  CAIL_PCIE_LINK_WIDTH_SUPPORT_X4 |
							  CAIL_PCIE_LINK_WIDTH_SUPPORT_X2 |
							  CAIL_PCIE_LINK_WIDTH_SUPPORT_X1);
				break;
			case 8:
				adev->pm.pcie_mlw_mask = (CAIL_PCIE_LINK_WIDTH_SUPPORT_X8 |
							  CAIL_PCIE_LINK_WIDTH_SUPPORT_X4 |
							  CAIL_PCIE_LINK_WIDTH_SUPPORT_X2 |
							  CAIL_PCIE_LINK_WIDTH_SUPPORT_X1);
				break;
			case 4:
				adev->pm.pcie_mlw_mask = (CAIL_PCIE_LINK_WIDTH_SUPPORT_X4 |
							  CAIL_PCIE_LINK_WIDTH_SUPPORT_X2 |
							  CAIL_PCIE_LINK_WIDTH_SUPPORT_X1);
				break;
			case 2:
				adev->pm.pcie_mlw_mask = (CAIL_PCIE_LINK_WIDTH_SUPPORT_X2 |
							  CAIL_PCIE_LINK_WIDTH_SUPPORT_X1);
				break;
			case 1:
				adev->pm.pcie_mlw_mask = CAIL_PCIE_LINK_WIDTH_SUPPORT_X1;
				break;
			default:
				break;
			}
		} else {
			adev->pm.pcie_mlw_mask = AMDGPU_DEFAULT_PCIE_MLW_MASK;
		}
	}
}

/*
 * Debugfs
 */
int amdgpu_debugfs_add_files(struct amdgpu_device *adev,
			     const struct drm_info_list *files,
			     unsigned nfiles)
{
	unsigned i;

	for (i = 0; i < adev->debugfs_count; i++) {
		if (adev->debugfs[i].files == files) {
			/* Already registered */
			return 0;
		}
	}

	i = adev->debugfs_count + 1;
	if (i > AMDGPU_DEBUGFS_MAX_COMPONENTS) {
		DRM_ERROR("Reached maximum number of debugfs components.\n");
		DRM_ERROR("Report so we increase "
			  "AMDGPU_DEBUGFS_MAX_COMPONENTS.\n");
		return -EINVAL;
	}
	adev->debugfs[adev->debugfs_count].files = files;
	adev->debugfs[adev->debugfs_count].num_files = nfiles;
	adev->debugfs_count = i;
#if defined(CONFIG_DEBUG_FS)
	drm_debugfs_create_files(files, nfiles,
				 adev->ddev->primary->debugfs_root,
				 adev->ddev->primary);
#endif
	return 0;
}

#if defined(CONFIG_DEBUG_FS)

static ssize_t amdgpu_debugfs_regs_read(struct file *f, char __user *buf,
					size_t size, loff_t *pos)
{
	struct amdgpu_device *adev = file_inode(f)->i_private;
	ssize_t result = 0;
	int r;
	bool pm_pg_lock, use_bank;
	unsigned instance_bank, sh_bank, se_bank;

	if (size & 0x3 || *pos & 0x3)
		return -EINVAL;

	/* are we reading registers for which a PG lock is necessary? */
	pm_pg_lock = (*pos >> 23) & 1;

	if (*pos & (1ULL << 62)) {
		se_bank = (*pos >> 24) & 0x3FF;
		sh_bank = (*pos >> 34) & 0x3FF;
		instance_bank = (*pos >> 44) & 0x3FF;

		if (se_bank == 0x3FF)
			se_bank = 0xFFFFFFFF;
		if (sh_bank == 0x3FF)
			sh_bank = 0xFFFFFFFF;
		if (instance_bank == 0x3FF)
			instance_bank = 0xFFFFFFFF;
		use_bank = 1;
	} else {
		use_bank = 0;
	}

	*pos &= (1UL << 22) - 1;

	if (use_bank) {
		if ((sh_bank != 0xFFFFFFFF && sh_bank >= adev->gfx.config.max_sh_per_se) ||
		    (se_bank != 0xFFFFFFFF && se_bank >= adev->gfx.config.max_shader_engines))
			return -EINVAL;
		mutex_lock(&adev->grbm_idx_mutex);
		amdgpu_gfx_select_se_sh(adev, se_bank,
					sh_bank, instance_bank);
	}

	if (pm_pg_lock)
		mutex_lock(&adev->pm.mutex);

	while (size) {
		uint32_t value;

		if (*pos > adev->rmmio_size)
			goto end;

		value = RREG32(*pos >> 2);
		r = put_user(value, (uint32_t *)buf);
		if (r) {
			result = r;
			goto end;
		}

		result += 4;
		buf += 4;
		*pos += 4;
		size -= 4;
	}

end:
	if (use_bank) {
		amdgpu_gfx_select_se_sh(adev, 0xffffffff, 0xffffffff, 0xffffffff);
		mutex_unlock(&adev->grbm_idx_mutex);
	}

	if (pm_pg_lock)
		mutex_unlock(&adev->pm.mutex);

	return result;
}

static ssize_t amdgpu_debugfs_regs_write(struct file *f, const char __user *buf,
					 size_t size, loff_t *pos)
{
	struct amdgpu_device *adev = file_inode(f)->i_private;
	ssize_t result = 0;
	int r;
	bool pm_pg_lock, use_bank;
	unsigned instance_bank, sh_bank, se_bank;

	if (size & 0x3 || *pos & 0x3)
		return -EINVAL;

	/* are we reading registers for which a PG lock is necessary? */
	pm_pg_lock = (*pos >> 23) & 1;

	if (*pos & (1ULL << 62)) {
		se_bank = (*pos >> 24) & 0x3FF;
		sh_bank = (*pos >> 34) & 0x3FF;
		instance_bank = (*pos >> 44) & 0x3FF;

		if (se_bank == 0x3FF)
			se_bank = 0xFFFFFFFF;
		if (sh_bank == 0x3FF)
			sh_bank = 0xFFFFFFFF;
		if (instance_bank == 0x3FF)
			instance_bank = 0xFFFFFFFF;
		use_bank = 1;
	} else {
		use_bank = 0;
	}

	*pos &= (1UL << 22) - 1;

	if (use_bank) {
		if ((sh_bank != 0xFFFFFFFF && sh_bank >= adev->gfx.config.max_sh_per_se) ||
		    (se_bank != 0xFFFFFFFF && se_bank >= adev->gfx.config.max_shader_engines))
			return -EINVAL;
		mutex_lock(&adev->grbm_idx_mutex);
		amdgpu_gfx_select_se_sh(adev, se_bank,
					sh_bank, instance_bank);
	}

	if (pm_pg_lock)
		mutex_lock(&adev->pm.mutex);

	while (size) {
		uint32_t value;

		if (*pos > adev->rmmio_size)
			return result;

		r = get_user(value, (uint32_t *)buf);
		if (r)
			return r;

		WREG32(*pos >> 2, value);

		result += 4;
		buf += 4;
		*pos += 4;
		size -= 4;
	}

	if (use_bank) {
		amdgpu_gfx_select_se_sh(adev, 0xffffffff, 0xffffffff, 0xffffffff);
		mutex_unlock(&adev->grbm_idx_mutex);
	}

	if (pm_pg_lock)
		mutex_unlock(&adev->pm.mutex);

	return result;
}

static ssize_t amdgpu_debugfs_regs_pcie_read(struct file *f, char __user *buf,
					size_t size, loff_t *pos)
{
	struct amdgpu_device *adev = file_inode(f)->i_private;
	ssize_t result = 0;
	int r;

	if (size & 0x3 || *pos & 0x3)
		return -EINVAL;

	while (size) {
		uint32_t value;

		value = RREG32_PCIE(*pos >> 2);
		r = put_user(value, (uint32_t *)buf);
		if (r)
			return r;

		result += 4;
		buf += 4;
		*pos += 4;
		size -= 4;
	}

	return result;
}

static ssize_t amdgpu_debugfs_regs_pcie_write(struct file *f, const char __user *buf,
					 size_t size, loff_t *pos)
{
	struct amdgpu_device *adev = file_inode(f)->i_private;
	ssize_t result = 0;
	int r;

	if (size & 0x3 || *pos & 0x3)
		return -EINVAL;

	while (size) {
		uint32_t value;

		r = get_user(value, (uint32_t *)buf);
		if (r)
			return r;

		WREG32_PCIE(*pos >> 2, value);

		result += 4;
		buf += 4;
		*pos += 4;
		size -= 4;
	}

	return result;
}

static ssize_t amdgpu_debugfs_regs_didt_read(struct file *f, char __user *buf,
					size_t size, loff_t *pos)
{
	struct amdgpu_device *adev = file_inode(f)->i_private;
	ssize_t result = 0;
	int r;

	if (size & 0x3 || *pos & 0x3)
		return -EINVAL;

	while (size) {
		uint32_t value;

		value = RREG32_DIDT(*pos >> 2);
		r = put_user(value, (uint32_t *)buf);
		if (r)
			return r;

		result += 4;
		buf += 4;
		*pos += 4;
		size -= 4;
	}

	return result;
}

static ssize_t amdgpu_debugfs_regs_didt_write(struct file *f, const char __user *buf,
					 size_t size, loff_t *pos)
{
	struct amdgpu_device *adev = file_inode(f)->i_private;
	ssize_t result = 0;
	int r;

	if (size & 0x3 || *pos & 0x3)
		return -EINVAL;

	while (size) {
		uint32_t value;

		r = get_user(value, (uint32_t *)buf);
		if (r)
			return r;

		WREG32_DIDT(*pos >> 2, value);

		result += 4;
		buf += 4;
		*pos += 4;
		size -= 4;
	}

	return result;
}

static ssize_t amdgpu_debugfs_regs_smc_read(struct file *f, char __user *buf,
					size_t size, loff_t *pos)
{
	struct amdgpu_device *adev = file_inode(f)->i_private;
	ssize_t result = 0;
	int r;

	if (size & 0x3 || *pos & 0x3)
		return -EINVAL;

	while (size) {
		uint32_t value;

		value = RREG32_SMC(*pos);
		r = put_user(value, (uint32_t *)buf);
		if (r)
			return r;

		result += 4;
		buf += 4;
		*pos += 4;
		size -= 4;
	}

	return result;
}

static ssize_t amdgpu_debugfs_regs_smc_write(struct file *f, const char __user *buf,
					 size_t size, loff_t *pos)
{
	struct amdgpu_device *adev = file_inode(f)->i_private;
	ssize_t result = 0;
	int r;

	if (size & 0x3 || *pos & 0x3)
		return -EINVAL;

	while (size) {
		uint32_t value;

		r = get_user(value, (uint32_t *)buf);
		if (r)
			return r;

		WREG32_SMC(*pos, value);

		result += 4;
		buf += 4;
		*pos += 4;
		size -= 4;
	}

	return result;
}

static ssize_t amdgpu_debugfs_gca_config_read(struct file *f, char __user *buf,
					size_t size, loff_t *pos)
{
	struct amdgpu_device *adev = file_inode(f)->i_private;
	ssize_t result = 0;
	int r;
	uint32_t *config, no_regs = 0;

	if (size & 0x3 || *pos & 0x3)
		return -EINVAL;

	config = kmalloc_array(256, sizeof(*config), GFP_KERNEL);
	if (!config)
		return -ENOMEM;

	/* version, increment each time something is added */
	config[no_regs++] = 3;
	config[no_regs++] = adev->gfx.config.max_shader_engines;
	config[no_regs++] = adev->gfx.config.max_tile_pipes;
	config[no_regs++] = adev->gfx.config.max_cu_per_sh;
	config[no_regs++] = adev->gfx.config.max_sh_per_se;
	config[no_regs++] = adev->gfx.config.max_backends_per_se;
	config[no_regs++] = adev->gfx.config.max_texture_channel_caches;
	config[no_regs++] = adev->gfx.config.max_gprs;
	config[no_regs++] = adev->gfx.config.max_gs_threads;
	config[no_regs++] = adev->gfx.config.max_hw_contexts;
	config[no_regs++] = adev->gfx.config.sc_prim_fifo_size_frontend;
	config[no_regs++] = adev->gfx.config.sc_prim_fifo_size_backend;
	config[no_regs++] = adev->gfx.config.sc_hiz_tile_fifo_size;
	config[no_regs++] = adev->gfx.config.sc_earlyz_tile_fifo_size;
	config[no_regs++] = adev->gfx.config.num_tile_pipes;
	config[no_regs++] = adev->gfx.config.backend_enable_mask;
	config[no_regs++] = adev->gfx.config.mem_max_burst_length_bytes;
	config[no_regs++] = adev->gfx.config.mem_row_size_in_kb;
	config[no_regs++] = adev->gfx.config.shader_engine_tile_size;
	config[no_regs++] = adev->gfx.config.num_gpus;
	config[no_regs++] = adev->gfx.config.multi_gpu_tile_size;
	config[no_regs++] = adev->gfx.config.mc_arb_ramcfg;
	config[no_regs++] = adev->gfx.config.gb_addr_config;
	config[no_regs++] = adev->gfx.config.num_rbs;

	/* rev==1 */
	config[no_regs++] = adev->rev_id;
	config[no_regs++] = adev->pg_flags;
	config[no_regs++] = adev->cg_flags;

	/* rev==2 */
	config[no_regs++] = adev->family;
	config[no_regs++] = adev->external_rev_id;

	/* rev==3 */
	config[no_regs++] = adev->pdev->device;
	config[no_regs++] = adev->pdev->revision;
	config[no_regs++] = adev->pdev->subsystem_device;
	config[no_regs++] = adev->pdev->subsystem_vendor;

	while (size && (*pos < no_regs * 4)) {
		uint32_t value;

		value = config[*pos >> 2];
		r = put_user(value, (uint32_t *)buf);
		if (r) {
			kfree(config);
			return r;
		}

		result += 4;
		buf += 4;
		*pos += 4;
		size -= 4;
	}

	kfree(config);
	return result;
}

static ssize_t amdgpu_debugfs_sensor_read(struct file *f, char __user *buf,
					size_t size, loff_t *pos)
{
	struct amdgpu_device *adev = file_inode(f)->i_private;
	int idx, x, outsize, r, valuesize;
	uint32_t values[16];

	if (size & 3 || *pos & 0x3)
		return -EINVAL;

	if (amdgpu_dpm == 0)
		return -EINVAL;

	/* convert offset to sensor number */
	idx = *pos >> 2;

	valuesize = sizeof(values);
	if (adev->powerplay.pp_funcs && adev->powerplay.pp_funcs->read_sensor)
		r = adev->powerplay.pp_funcs->read_sensor(adev->powerplay.pp_handle, idx, &values[0], &valuesize);
	else if (adev->pm.funcs && adev->pm.funcs->read_sensor)
		r = adev->pm.funcs->read_sensor(adev, idx, &values[0],
						&valuesize);
	else
		return -EINVAL;

	if (size > valuesize)
		return -EINVAL;

	outsize = 0;
	x = 0;
	if (!r) {
		while (size) {
			r = put_user(values[x++], (int32_t *)buf);
			buf += 4;
			size -= 4;
			outsize += 4;
		}
	}

	return !r ? outsize : r;
}

static ssize_t amdgpu_debugfs_wave_read(struct file *f, char __user *buf,
					size_t size, loff_t *pos)
{
	struct amdgpu_device *adev = f->f_inode->i_private;
	int r, x;
	ssize_t result=0;
	uint32_t offset, se, sh, cu, wave, simd, data[32];

	if (size & 3 || *pos & 3)
		return -EINVAL;

	/* decode offset */
	offset = (*pos & 0x7F);
	se = ((*pos >> 7) & 0xFF);
	sh = ((*pos >> 15) & 0xFF);
	cu = ((*pos >> 23) & 0xFF);
	wave = ((*pos >> 31) & 0xFF);
	simd = ((*pos >> 37) & 0xFF);

	/* switch to the specific se/sh/cu */
	mutex_lock(&adev->grbm_idx_mutex);
	amdgpu_gfx_select_se_sh(adev, se, sh, cu);

	x = 0;
	if (adev->gfx.funcs->read_wave_data)
		adev->gfx.funcs->read_wave_data(adev, simd, wave, data, &x);

	amdgpu_gfx_select_se_sh(adev, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
	mutex_unlock(&adev->grbm_idx_mutex);

	if (!x)
		return -EINVAL;

	while (size && (offset < x * 4)) {
		uint32_t value;

		value = data[offset >> 2];
		r = put_user(value, (uint32_t *)buf);
		if (r)
			return r;

		result += 4;
		buf += 4;
		offset += 4;
		size -= 4;
	}

	return result;
}

static ssize_t amdgpu_debugfs_gpr_read(struct file *f, char __user *buf,
					size_t size, loff_t *pos)
{
	struct amdgpu_device *adev = f->f_inode->i_private;
	int r;
	ssize_t result = 0;
	uint32_t offset, se, sh, cu, wave, simd, thread, bank, *data;

	if (size & 3 || *pos & 3)
		return -EINVAL;

	/* decode offset */
	offset = (*pos & 0xFFF);       /* in dwords */
	se = ((*pos >> 12) & 0xFF);
	sh = ((*pos >> 20) & 0xFF);
	cu = ((*pos >> 28) & 0xFF);
	wave = ((*pos >> 36) & 0xFF);
	simd = ((*pos >> 44) & 0xFF);
	thread = ((*pos >> 52) & 0xFF);
	bank = ((*pos >> 60) & 1);

	data = kmalloc_array(1024, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	/* switch to the specific se/sh/cu */
	mutex_lock(&adev->grbm_idx_mutex);
	amdgpu_gfx_select_se_sh(adev, se, sh, cu);

	if (bank == 0) {
		if (adev->gfx.funcs->read_wave_vgprs)
			adev->gfx.funcs->read_wave_vgprs(adev, simd, wave, thread, offset, size>>2, data);
	} else {
		if (adev->gfx.funcs->read_wave_sgprs)
			adev->gfx.funcs->read_wave_sgprs(adev, simd, wave, offset, size>>2, data);
	}

	amdgpu_gfx_select_se_sh(adev, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
	mutex_unlock(&adev->grbm_idx_mutex);

	while (size) {
		uint32_t value;

		value = data[offset++];
		r = put_user(value, (uint32_t *)buf);
		if (r) {
			result = r;
			goto err;
		}

		result += 4;
		buf += 4;
		size -= 4;
	}

err:
	kfree(data);
	return result;
}

static const struct file_operations amdgpu_debugfs_regs_fops = {
	.owner = THIS_MODULE,
	.read = amdgpu_debugfs_regs_read,
	.write = amdgpu_debugfs_regs_write,
	.llseek = default_llseek
};
static const struct file_operations amdgpu_debugfs_regs_didt_fops = {
	.owner = THIS_MODULE,
	.read = amdgpu_debugfs_regs_didt_read,
	.write = amdgpu_debugfs_regs_didt_write,
	.llseek = default_llseek
};
static const struct file_operations amdgpu_debugfs_regs_pcie_fops = {
	.owner = THIS_MODULE,
	.read = amdgpu_debugfs_regs_pcie_read,
	.write = amdgpu_debugfs_regs_pcie_write,
	.llseek = default_llseek
};
static const struct file_operations amdgpu_debugfs_regs_smc_fops = {
	.owner = THIS_MODULE,
	.read = amdgpu_debugfs_regs_smc_read,
	.write = amdgpu_debugfs_regs_smc_write,
	.llseek = default_llseek
};

static const struct file_operations amdgpu_debugfs_gca_config_fops = {
	.owner = THIS_MODULE,
	.read = amdgpu_debugfs_gca_config_read,
	.llseek = default_llseek
};

static const struct file_operations amdgpu_debugfs_sensors_fops = {
	.owner = THIS_MODULE,
	.read = amdgpu_debugfs_sensor_read,
	.llseek = default_llseek
};

static const struct file_operations amdgpu_debugfs_wave_fops = {
	.owner = THIS_MODULE,
	.read = amdgpu_debugfs_wave_read,
	.llseek = default_llseek
};
static const struct file_operations amdgpu_debugfs_gpr_fops = {
	.owner = THIS_MODULE,
	.read = amdgpu_debugfs_gpr_read,
	.llseek = default_llseek
};

static const struct file_operations *debugfs_regs[] = {
	&amdgpu_debugfs_regs_fops,
	&amdgpu_debugfs_regs_didt_fops,
	&amdgpu_debugfs_regs_pcie_fops,
	&amdgpu_debugfs_regs_smc_fops,
	&amdgpu_debugfs_gca_config_fops,
	&amdgpu_debugfs_sensors_fops,
	&amdgpu_debugfs_wave_fops,
	&amdgpu_debugfs_gpr_fops,
};

static const char *debugfs_regs_names[] = {
	"amdgpu_regs",
	"amdgpu_regs_didt",
	"amdgpu_regs_pcie",
	"amdgpu_regs_smc",
	"amdgpu_gca_config",
	"amdgpu_sensors",
	"amdgpu_wave",
	"amdgpu_gpr",
};

static int amdgpu_debugfs_regs_init(struct amdgpu_device *adev)
{
	struct drm_minor *minor = adev->ddev->primary;
	struct dentry *ent, *root = minor->debugfs_root;
	unsigned i, j;

	for (i = 0; i < ARRAY_SIZE(debugfs_regs); i++) {
		ent = debugfs_create_file(debugfs_regs_names[i],
					  S_IFREG | S_IRUGO, root,
					  adev, debugfs_regs[i]);
		if (IS_ERR(ent)) {
			for (j = 0; j < i; j++) {
				debugfs_remove(adev->debugfs_regs[i]);
				adev->debugfs_regs[i] = NULL;
			}
			return PTR_ERR(ent);
		}

		if (!i)
			i_size_write(ent->d_inode, adev->rmmio_size);
		adev->debugfs_regs[i] = ent;
	}

	return 0;
}

static void amdgpu_debugfs_regs_cleanup(struct amdgpu_device *adev)
{
	unsigned i;

	for (i = 0; i < ARRAY_SIZE(debugfs_regs); i++) {
		if (adev->debugfs_regs[i]) {
			debugfs_remove(adev->debugfs_regs[i]);
			adev->debugfs_regs[i] = NULL;
		}
	}
}

static int amdgpu_debugfs_test_ib(struct seq_file *m, void *data)
{
	struct drm_info_node *node = (struct drm_info_node *) m->private;
	struct drm_device *dev = node->minor->dev;
	struct amdgpu_device *adev = dev->dev_private;
	int r = 0, i;

	/* hold on the scheduler */
	for (i = 0; i < AMDGPU_MAX_RINGS; i++) {
		struct amdgpu_ring *ring = adev->rings[i];

		if (!ring || !ring->sched.thread)
			continue;
		kthread_park(ring->sched.thread);
	}

	seq_printf(m, "run ib test:\n");
	r = amdgpu_ib_ring_tests(adev);
	if (r)
		seq_printf(m, "ib ring tests failed (%d).\n", r);
	else
		seq_printf(m, "ib ring tests passed.\n");

	/* go on the scheduler */
	for (i = 0; i < AMDGPU_MAX_RINGS; i++) {
		struct amdgpu_ring *ring = adev->rings[i];

		if (!ring || !ring->sched.thread)
			continue;
		kthread_unpark(ring->sched.thread);
	}

	return 0;
}

static const struct drm_info_list amdgpu_debugfs_test_ib_ring_list[] = {
	{"amdgpu_test_ib", &amdgpu_debugfs_test_ib}
};

static int amdgpu_debugfs_test_ib_ring_init(struct amdgpu_device *adev)
{
	return amdgpu_debugfs_add_files(adev,
					amdgpu_debugfs_test_ib_ring_list, 1);
}

int amdgpu_debugfs_init(struct drm_minor *minor)
{
	return 0;
}
#else
static int amdgpu_debugfs_test_ib_ring_init(struct amdgpu_device *adev)
{
	return 0;
}
static int amdgpu_debugfs_regs_init(struct amdgpu_device *adev)
{
	return 0;
}
static void amdgpu_debugfs_regs_cleanup(struct amdgpu_device *adev) { }
#endif
