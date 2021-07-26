/*
 * Copyright (c) 2013-2015, Mellanox Technologies. All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <linux/highmem.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/pci.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/io-mapping.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/mlx5/driver.h>
#include <linux/mlx5/cq.h>
#include <linux/mlx5/qp.h>
#include <linux/mlx5/srq.h>
#include <linux/debugfs.h>
#include <linux/kmod.h>
#include <linux/mlx5/mlx5_ifc.h>
#include <linux/mlx5/vport.h>
#ifdef CONFIG_RFS_ACCEL
#include <linux/cpu_rmap.h>
#endif
#include <linux/version.h>
#include <net/devlink.h>
#include "mlx5_core.h"
#include "fs_core.h"
#include "lib/mpfs.h"
#include "eswitch.h"
#include "lib/mlx5.h"
#include "fpga/core.h"
#include "fpga/ipsec.h"
#include "accel/ipsec.h"
#include "accel/tls.h"
#include "lib/clock.h"
#include "lib/vxlan.h"
#include "diag/fw_tracer.h"

MODULE_AUTHOR("Eli Cohen <eli@mellanox.com>");
MODULE_DESCRIPTION("Mellanox 5th generation network adapters (ConnectX series) core driver");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION(DRIVER_VERSION);

unsigned int mlx5_core_debug_mask;
module_param_named(debug_mask, mlx5_core_debug_mask, uint, 0644);
MODULE_PARM_DESC(debug_mask, "debug mask: 1 = dump cmd data, 2 = dump cmd exec time, 3 = both. Default=0");

#define MLX5_DEFAULT_PROF	2
static unsigned int prof_sel = MLX5_DEFAULT_PROF;
module_param_named(prof_sel, prof_sel, uint, 0444);
MODULE_PARM_DESC(prof_sel, "profile selector. Valid range 0 - 2");

static u32 sw_owner_id[4];

enum {
	MLX5_ATOMIC_REQ_MODE_BE = 0x0,
	MLX5_ATOMIC_REQ_MODE_HOST_ENDIANNESS = 0x1,
};

static struct mlx5_profile profile[] = {
	[0] = {
		.mask           = 0,
	},
	[1] = {
		.mask		= MLX5_PROF_MASK_QP_SIZE,
		.log_max_qp	= 12,
	},
	[2] = {
		.mask		= MLX5_PROF_MASK_QP_SIZE |
				  MLX5_PROF_MASK_MR_CACHE,
		.log_max_qp	= 18,
		.mr_cache[0]	= {
			.size	= 500,
			.limit	= 250
		},
		.mr_cache[1]	= {
			.size	= 500,
			.limit	= 250
		},
		.mr_cache[2]	= {
			.size	= 500,
			.limit	= 250
		},
		.mr_cache[3]	= {
			.size	= 500,
			.limit	= 250
		},
		.mr_cache[4]	= {
			.size	= 500,
			.limit	= 250
		},
		.mr_cache[5]	= {
			.size	= 500,
			.limit	= 250
		},
		.mr_cache[6]	= {
			.size	= 500,
			.limit	= 250
		},
		.mr_cache[7]	= {
			.size	= 500,
			.limit	= 250
		},
		.mr_cache[8]	= {
			.size	= 500,
			.limit	= 250
		},
		.mr_cache[9]	= {
			.size	= 500,
			.limit	= 250
		},
		.mr_cache[10]	= {
			.size	= 500,
			.limit	= 250
		},
		.mr_cache[11]	= {
			.size	= 500,
			.limit	= 250
		},
		.mr_cache[12]	= {
			.size	= 64,
			.limit	= 32
		},
		.mr_cache[13]	= {
			.size	= 32,
			.limit	= 16
		},
		.mr_cache[14]	= {
			.size	= 16,
			.limit	= 8
		},
		.mr_cache[15]	= {
			.size	= 8,
			.limit	= 4
		},
	},
};

#define FW_INIT_TIMEOUT_MILI		2000
#define FW_INIT_WAIT_MS			2
#define FW_PRE_INIT_TIMEOUT_MILI	10000

static int wait_fw_init(struct mlx5_core_dev *dev, u32 max_wait_mili)
{
	unsigned long end = jiffies + msecs_to_jiffies(max_wait_mili);
	int err = 0;

	while (fw_initializing(dev)) {
		if (time_after(jiffies, end)) {
			err = -EBUSY;
			break;
		}
		msleep(FW_INIT_WAIT_MS);
	}

	return err;
}

static void mlx5_set_driver_version(struct mlx5_core_dev *dev)
{
	int driver_ver_sz = MLX5_FLD_SZ_BYTES(set_driver_version_in,
					      driver_version);
	u8 in[MLX5_ST_SZ_BYTES(set_driver_version_in)] = {0};
	u8 out[MLX5_ST_SZ_BYTES(set_driver_version_out)] = {0};
	int remaining_size = driver_ver_sz;
	char *string;

	if (!MLX5_CAP_GEN(dev, driver_version))
		return;

	string = MLX5_ADDR_OF(set_driver_version_in, in, driver_version);

	strncpy(string, "Linux", remaining_size);

	remaining_size = max_t(int, 0, driver_ver_sz - strlen(string));
	strncat(string, ",", remaining_size);

	remaining_size = max_t(int, 0, driver_ver_sz - strlen(string));
	strncat(string, DRIVER_NAME, remaining_size);

	remaining_size = max_t(int, 0, driver_ver_sz - strlen(string));
	strncat(string, ",", remaining_size);

	remaining_size = max_t(int, 0, driver_ver_sz - strlen(string));

	snprintf(string + strlen(string), remaining_size, "%u.%u.%u",
		 (u8)((LINUX_VERSION_CODE >> 16) & 0xff), (u8)((LINUX_VERSION_CODE >> 8) & 0xff),
		 (u16)(LINUX_VERSION_CODE & 0xffff));

	/*Send the command*/
	MLX5_SET(set_driver_version_in, in, opcode,
		 MLX5_CMD_OP_SET_DRIVER_VERSION);

	mlx5_cmd_exec(dev, in, sizeof(in), out, sizeof(out));
}

static int set_dma_caps(struct pci_dev *pdev)
{
	int err;

	err = pci_set_dma_mask(pdev, DMA_BIT_MASK(64));
	if (err) {
		dev_warn(&pdev->dev, "Warning: couldn't set 64-bit PCI DMA mask\n");
		err = pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
		if (err) {
			dev_err(&pdev->dev, "Can't set PCI DMA mask, aborting\n");
			return err;
		}
	}

	err = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(64));
	if (err) {
		dev_warn(&pdev->dev,
			 "Warning: couldn't set 64-bit consistent PCI DMA mask\n");
		err = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(32));
		if (err) {
			dev_err(&pdev->dev,
				"Can't set consistent PCI DMA mask, aborting\n");
			return err;
		}
	}

	dma_set_max_seg_size(&pdev->dev, 2u * 1024 * 1024 * 1024);
	return err;
}

static int mlx5_pci_enable_device(struct mlx5_core_dev *dev)
{
	struct pci_dev *pdev = dev->pdev;
	int err = 0;

	mutex_lock(&dev->pci_status_mutex);
	if (dev->pci_status == MLX5_PCI_STATUS_DISABLED) {
		err = pci_enable_device(pdev);
		if (!err)
			dev->pci_status = MLX5_PCI_STATUS_ENABLED;
	}
	mutex_unlock(&dev->pci_status_mutex);

	return err;
}

static void mlx5_pci_disable_device(struct mlx5_core_dev *dev)
{
	struct pci_dev *pdev = dev->pdev;

	mutex_lock(&dev->pci_status_mutex);
	if (dev->pci_status == MLX5_PCI_STATUS_ENABLED) {
		pci_disable_device(pdev);
		dev->pci_status = MLX5_PCI_STATUS_DISABLED;
	}
	mutex_unlock(&dev->pci_status_mutex);
}

static int request_bar(struct pci_dev *pdev)
{
	int err = 0;

	if (!(pci_resource_flags(pdev, 0) & IORESOURCE_MEM)) {
		dev_err(&pdev->dev, "Missing registers BAR, aborting\n");
		return -ENODEV;
	}

	err = pci_request_regions(pdev, DRIVER_NAME);
	if (err)
		dev_err(&pdev->dev, "Couldn't get PCI resources, aborting\n");

	return err;
}

static void release_bar(struct pci_dev *pdev)
{
	pci_release_regions(pdev);
}

static int mlx5_alloc_irq_vectors(struct mlx5_core_dev *dev)
{
	struct mlx5_priv *priv = &dev->priv;
	struct mlx5_eq_table *table = &priv->eq_table;
	int num_eqs = MLX5_CAP_GEN(dev, max_num_eqs) ?
		      MLX5_CAP_GEN(dev, max_num_eqs) :
		      1 << MLX5_CAP_GEN(dev, log_max_eq);
	int nvec;
	int err;

	nvec = MLX5_CAP_GEN(dev, num_ports) * num_online_cpus() +
	       MLX5_EQ_VEC_COMP_BASE;
	nvec = min_t(int, nvec, num_eqs);
	if (nvec <= MLX5_EQ_VEC_COMP_BASE)
		return -ENOMEM;

	priv->irq_info = kcalloc(nvec, sizeof(*priv->irq_info), GFP_KERNEL);
	if (!priv->irq_info)
		return -ENOMEM;

	nvec = pci_alloc_irq_vectors(dev->pdev,
			MLX5_EQ_VEC_COMP_BASE + 1, nvec,
			PCI_IRQ_MSIX);
	if (nvec < 0) {
		err = nvec;
		goto err_free_irq_info;
	}

	table->num_comp_vectors = nvec - MLX5_EQ_VEC_COMP_BASE;

	return 0;

err_free_irq_info:
	kfree(priv->irq_info);
	return err;
}

static void mlx5_free_irq_vectors(struct mlx5_core_dev *dev)
{
	struct mlx5_priv *priv = &dev->priv;

	pci_free_irq_vectors(dev->pdev);
	kfree(priv->irq_info);
}

struct mlx5_reg_host_endianness {
	u8	he;
	u8      rsvd[15];
};

#define CAP_MASK(pos, size) ((u64)((1 << (size)) - 1) << (pos))

enum {
	MLX5_CAP_BITS_RW_MASK = CAP_MASK(MLX5_CAP_OFF_CMDIF_CSUM, 2) |
				MLX5_DEV_CAP_FLAG_DCT,
};

static u16 to_fw_pkey_sz(struct mlx5_core_dev *dev, u32 size)
{
	switch (size) {
	case 128:
		return 0;
	case 256:
		return 1;
	case 512:
		return 2;
	case 1024:
		return 3;
	case 2048:
		return 4;
	case 4096:
		return 5;
	default:
		mlx5_core_warn(dev, "invalid pkey table size %d\n", size);
		return 0;
	}
}

static int mlx5_core_get_caps_mode(struct mlx5_core_dev *dev,
				   enum mlx5_cap_type cap_type,
				   enum mlx5_cap_mode cap_mode)
{
	u8 in[MLX5_ST_SZ_BYTES(query_hca_cap_in)];
	int out_sz = MLX5_ST_SZ_BYTES(query_hca_cap_out);
	void *out, *hca_caps;
	u16 opmod = (cap_type << 1) | (cap_mode & 0x01);
	int err;

	memset(in, 0, sizeof(in));
	out = kzalloc(out_sz, GFP_KERNEL);
	if (!out)
		return -ENOMEM;

	MLX5_SET(query_hca_cap_in, in, opcode, MLX5_CMD_OP_QUERY_HCA_CAP);
	MLX5_SET(query_hca_cap_in, in, op_mod, opmod);
	err = mlx5_cmd_exec(dev, in, sizeof(in), out, out_sz);
	if (err) {
		mlx5_core_warn(dev,
			       "QUERY_HCA_CAP : type(%x) opmode(%x) Failed(%d)\n",
			       cap_type, cap_mode, err);
		goto query_ex;
	}

	hca_caps =  MLX5_ADDR_OF(query_hca_cap_out, out, capability);

	switch (cap_mode) {
	case HCA_CAP_OPMOD_GET_MAX:
		memcpy(dev->caps.hca_max[cap_type], hca_caps,
		       MLX5_UN_SZ_BYTES(hca_cap_union));
		break;
	case HCA_CAP_OPMOD_GET_CUR:
		memcpy(dev->caps.hca_cur[cap_type], hca_caps,
		       MLX5_UN_SZ_BYTES(hca_cap_union));
		break;
	default:
		mlx5_core_warn(dev,
			       "Tried to query dev cap type(%x) with wrong opmode(%x)\n",
			       cap_type, cap_mode);
		err = -EINVAL;
		break;
	}
query_ex:
	kfree(out);
	return err;
}

int mlx5_core_get_caps(struct mlx5_core_dev *dev, enum mlx5_cap_type cap_type)
{
	int ret;

	ret = mlx5_core_get_caps_mode(dev, cap_type, HCA_CAP_OPMOD_GET_CUR);
	if (ret)
		return ret;
	return mlx5_core_get_caps_mode(dev, cap_type, HCA_CAP_OPMOD_GET_MAX);
}

static int set_caps(struct mlx5_core_dev *dev, void *in, int in_sz, int opmod)
{
	u32 out[MLX5_ST_SZ_DW(set_hca_cap_out)] = {0};

	MLX5_SET(set_hca_cap_in, in, opcode, MLX5_CMD_OP_SET_HCA_CAP);
	MLX5_SET(set_hca_cap_in, in, op_mod, opmod << 1);
	return mlx5_cmd_exec(dev, in, in_sz, out, sizeof(out));
}

static int handle_hca_cap_atomic(struct mlx5_core_dev *dev)
{
	void *set_ctx;
	void *set_hca_cap;
	int set_sz = MLX5_ST_SZ_BYTES(set_hca_cap_in);
	int req_endianness;
	int err;

	if (MLX5_CAP_GEN(dev, atomic)) {
		err = mlx5_core_get_caps(dev, MLX5_CAP_ATOMIC);
		if (err)
			return err;
	} else {
		return 0;
	}

	req_endianness =
		MLX5_CAP_ATOMIC(dev,
				supported_atomic_req_8B_endianness_mode_1);

	if (req_endianness != MLX5_ATOMIC_REQ_MODE_HOST_ENDIANNESS)
		return 0;

	set_ctx = kzalloc(set_sz, GFP_KERNEL);
	if (!set_ctx)
		return -ENOMEM;

	set_hca_cap = MLX5_ADDR_OF(set_hca_cap_in, set_ctx, capability);

	/* Set requestor to host endianness */
	MLX5_SET(atomic_caps, set_hca_cap, atomic_req_8B_endianness_mode,
		 MLX5_ATOMIC_REQ_MODE_HOST_ENDIANNESS);

	err = set_caps(dev, set_ctx, set_sz, MLX5_SET_HCA_CAP_OP_MOD_ATOMIC);

	kfree(set_ctx);
	return err;
}

static int handle_hca_cap(struct mlx5_core_dev *dev)
{
	void *set_ctx = NULL;
	struct mlx5_profile *prof = dev->profile;
	int err = -ENOMEM;
	int set_sz = MLX5_ST_SZ_BYTES(set_hca_cap_in);
	void *set_hca_cap;

	set_ctx = kzalloc(set_sz, GFP_KERNEL);
	if (!set_ctx)
		goto query_ex;

	err = mlx5_core_get_caps(dev, MLX5_CAP_GENERAL);
	if (err)
		goto query_ex;

	set_hca_cap = MLX5_ADDR_OF(set_hca_cap_in, set_ctx,
				   capability);
	memcpy(set_hca_cap, dev->caps.hca_cur[MLX5_CAP_GENERAL],
	       MLX5_ST_SZ_BYTES(cmd_hca_cap));

	mlx5_core_dbg(dev, "Current Pkey table size %d Setting new size %d\n",
		      mlx5_to_sw_pkey_sz(MLX5_CAP_GEN(dev, pkey_table_size)),
		      128);
	/* we limit the size of the pkey table to 128 entries for now */
	MLX5_SET(cmd_hca_cap, set_hca_cap, pkey_table_size,
		 to_fw_pkey_sz(dev, 128));

	/* Check log_max_qp from HCA caps to set in current profile */
	if (MLX5_CAP_GEN_MAX(dev, log_max_qp) < profile[prof_sel].log_max_qp) {
		mlx5_core_warn(dev, "log_max_qp value in current profile is %d, changing it to HCA capability limit (%d)\n",
			       profile[prof_sel].log_max_qp,
			       MLX5_CAP_GEN_MAX(dev, log_max_qp));
		profile[prof_sel].log_max_qp = MLX5_CAP_GEN_MAX(dev, log_max_qp);
	}
	if (prof->mask & MLX5_PROF_MASK_QP_SIZE)
		MLX5_SET(cmd_hca_cap, set_hca_cap, log_max_qp,
			 prof->log_max_qp);

	/* disable cmdif checksum */
	MLX5_SET(cmd_hca_cap, set_hca_cap, cmdif_checksum, 0);

	/* Enable 4K UAR only when HCA supports it and page size is bigger
	 * than 4K.
	 */
	if (MLX5_CAP_GEN_MAX(dev, uar_4k) && PAGE_SIZE > 4096)
		MLX5_SET(cmd_hca_cap, set_hca_cap, uar_4k, 1);

	MLX5_SET(cmd_hca_cap, set_hca_cap, log_uar_page_sz, PAGE_SHIFT - 12);

	if (MLX5_CAP_GEN_MAX(dev, cache_line_128byte))
		MLX5_SET(cmd_hca_cap,
			 set_hca_cap,
			 cache_line_128byte,
			 cache_line_size() >= 128 ? 1 : 0);

	if (MLX5_CAP_GEN_MAX(dev, dct))
		MLX5_SET(cmd_hca_cap, set_hca_cap, dct, 1);

	if (MLX5_CAP_GEN_MAX(dev, num_vhca_ports))
		MLX5_SET(cmd_hca_cap,
			 set_hca_cap,
			 num_vhca_ports,
			 MLX5_CAP_GEN_MAX(dev, num_vhca_ports));

	err = set_caps(dev, set_ctx, set_sz,
		       MLX5_SET_HCA_CAP_OP_MOD_GENERAL_DEVICE);

query_ex:
	kfree(set_ctx);
	return err;
}

static int set_hca_ctrl(struct mlx5_core_dev *dev)
{
	struct mlx5_reg_host_endianness he_in;
	struct mlx5_reg_host_endianness he_out;
	int err;

	if (!mlx5_core_is_pf(dev))
		return 0;

	memset(&he_in, 0, sizeof(he_in));
	he_in.he = MLX5_SET_HOST_ENDIANNESS;
	err = mlx5_core_access_reg(dev, &he_in,  sizeof(he_in),
					&he_out, sizeof(he_out),
					MLX5_REG_HOST_ENDIANNESS, 0, 1);
	return err;
}

static int mlx5_core_set_hca_defaults(struct mlx5_core_dev *dev)
{
	int ret = 0;

	/* Disable local_lb by default */
	if (MLX5_CAP_GEN(dev, port_type) == MLX5_CAP_PORT_TYPE_ETH)
		ret = mlx5_nic_vport_update_local_lb(dev, false);

	return ret;
}

int mlx5_core_enable_hca(struct mlx5_core_dev *dev, u16 func_id)
{
	u32 out[MLX5_ST_SZ_DW(enable_hca_out)] = {0};
	u32 in[MLX5_ST_SZ_DW(enable_hca_in)]   = {0};

	MLX5_SET(enable_hca_in, in, opcode, MLX5_CMD_OP_ENABLE_HCA);
	MLX5_SET(enable_hca_in, in, function_id, func_id);
	return mlx5_cmd_exec(dev, &in, sizeof(in), &out, sizeof(out));
}

int mlx5_core_disable_hca(struct mlx5_core_dev *dev, u16 func_id)
{
	u32 out[MLX5_ST_SZ_DW(disable_hca_out)] = {0};
	u32 in[MLX5_ST_SZ_DW(disable_hca_in)]   = {0};

	MLX5_SET(disable_hca_in, in, opcode, MLX5_CMD_OP_DISABLE_HCA);
	MLX5_SET(disable_hca_in, in, function_id, func_id);
	return mlx5_cmd_exec(dev, in, sizeof(in), out, sizeof(out));
}

u64 mlx5_read_internal_timer(struct mlx5_core_dev *dev)
{
	u32 timer_h, timer_h1, timer_l;

	timer_h = ioread32be(&dev->iseg->internal_timer_h);
	timer_l = ioread32be(&dev->iseg->internal_timer_l);
	timer_h1 = ioread32be(&dev->iseg->internal_timer_h);
	if (timer_h != timer_h1) /* wrap around */
		timer_l = ioread32be(&dev->iseg->internal_timer_l);

	return (u64)timer_l | (u64)timer_h1 << 32;
}

static int mlx5_irq_set_affinity_hint(struct mlx5_core_dev *mdev, int i)
{
	struct mlx5_priv *priv  = &mdev->priv;
	int vecidx = MLX5_EQ_VEC_COMP_BASE + i;
	int irq = pci_irq_vector(mdev->pdev, vecidx);

	if (!zalloc_cpumask_var(&priv->irq_info[vecidx].mask, GFP_KERNEL)) {
		mlx5_core_warn(mdev, "zalloc_cpumask_var failed");
		return -ENOMEM;
	}

	cpumask_set_cpu(cpumask_local_spread(i, priv->numa_node),
			priv->irq_info[vecidx].mask);

	if (IS_ENABLED(CONFIG_SMP) &&
	    irq_set_affinity_hint(irq, priv->irq_info[vecidx].mask))
		mlx5_core_warn(mdev, "irq_set_affinity_hint failed, irq 0x%.4x", irq);

	return 0;
}

static void mlx5_irq_clear_affinity_hint(struct mlx5_core_dev *mdev, int i)
{
	int vecidx = MLX5_EQ_VEC_COMP_BASE + i;
	struct mlx5_priv *priv  = &mdev->priv;
	int irq = pci_irq_vector(mdev->pdev, vecidx);

	irq_set_affinity_hint(irq, NULL);
	free_cpumask_var(priv->irq_info[vecidx].mask);
}

static int mlx5_irq_set_affinity_hints(struct mlx5_core_dev *mdev)
{
	int err;
	int i;

	for (i = 0; i < mdev->priv.eq_table.num_comp_vectors; i++) {
		err = mlx5_irq_set_affinity_hint(mdev, i);
		if (err)
			goto err_out;
	}

	return 0;

err_out:
	for (i--; i >= 0; i--)
		mlx5_irq_clear_affinity_hint(mdev, i);

	return err;
}

static void mlx5_irq_clear_affinity_hints(struct mlx5_core_dev *mdev)
{
	int i;

	for (i = 0; i < mdev->priv.eq_table.num_comp_vectors; i++)
		mlx5_irq_clear_affinity_hint(mdev, i);
}

int mlx5_vector2eqn(struct mlx5_core_dev *dev, int vector, int *eqn,
		    unsigned int *irqn)
{
	struct mlx5_eq_table *table = &dev->priv.eq_table;
	struct mlx5_eq *eq, *n;
	int err = -ENOENT;

	spin_lock(&table->lock);
	list_for_each_entry_safe(eq, n, &table->comp_eqs_list, list) {
		if (eq->index == vector) {
			*eqn = eq->eqn;
			*irqn = eq->irqn;
			err = 0;
			break;
		}
	}
	spin_unlock(&table->lock);

	return err;
}
EXPORT_SYMBOL(mlx5_vector2eqn);

struct mlx5_eq *mlx5_eqn2eq(struct mlx5_core_dev *dev, int eqn)
{
	struct mlx5_eq_table *table = &dev->priv.eq_table;
	struct mlx5_eq *eq;

	spin_lock(&table->lock);
	list_for_each_entry(eq, &table->comp_eqs_list, list)
		if (eq->eqn == eqn) {
			spin_unlock(&table->lock);
			return eq;
		}

	spin_unlock(&table->lock);

	return ERR_PTR(-ENOENT);
}

static void free_comp_eqs(struct mlx5_core_dev *dev)
{
	struct mlx5_eq_table *table = &dev->priv.eq_table;
	struct mlx5_eq *eq, *n;

#ifdef CONFIG_RFS_ACCEL
	if (dev->rmap) {
		free_irq_cpu_rmap(dev->rmap);
		dev->rmap = NULL;
	}
#endif
	spin_lock(&table->lock);
	list_for_each_entry_safe(eq, n, &table->comp_eqs_list, list) {
		list_del(&eq->list);
		spin_unlock(&table->lock);
		if (mlx5_destroy_unmap_eq(dev, eq))
			mlx5_core_warn(dev, "failed to destroy EQ 0x%x\n",
				       eq->eqn);
		kfree(eq);
		spin_lock(&table->lock);
	}
	spin_unlock(&table->lock);
}

static int alloc_comp_eqs(struct mlx5_core_dev *dev)
{
	struct mlx5_eq_table *table = &dev->priv.eq_table;
	char name[MLX5_MAX_IRQ_NAME];
	struct mlx5_eq *eq;
	int ncomp_vec;
	int nent;
	int err;
	int i;

	INIT_LIST_HEAD(&table->comp_eqs_list);
	ncomp_vec = table->num_comp_vectors;
	nent = MLX5_COMP_EQ_SIZE;
#ifdef CONFIG_RFS_ACCEL
	dev->rmap = alloc_irq_cpu_rmap(ncomp_vec);
	if (!dev->rmap)
		return -ENOMEM;
#endif
	for (i = 0; i < ncomp_vec; i++) {
		eq = kzalloc(sizeof(*eq), GFP_KERNEL);
		if (!eq) {
			err = -ENOMEM;
			goto clean;
		}

#ifdef CONFIG_RFS_ACCEL
		irq_cpu_rmap_add(dev->rmap, pci_irq_vector(dev->pdev,
				 MLX5_EQ_VEC_COMP_BASE + i));
#endif
		snprintf(name, MLX5_MAX_IRQ_NAME, "mlx5_comp%d", i);
		err = mlx5_create_map_eq(dev, eq,
					 i + MLX5_EQ_VEC_COMP_BASE, nent, 0,
					 name, MLX5_EQ_TYPE_COMP);
		if (err) {
			kfree(eq);
			goto clean;
		}
		mlx5_core_dbg(dev, "allocated completion EQN %d\n", eq->eqn);
		eq->index = i;
		spin_lock(&table->lock);
		list_add_tail(&eq->list, &table->comp_eqs_list);
		spin_unlock(&table->lock);
	}

	return 0;

clean:
	free_comp_eqs(dev);
	return err;
}

static int mlx5_core_set_issi(struct mlx5_core_dev *dev)
{
	u32 query_in[MLX5_ST_SZ_DW(query_issi_in)]   = {0};
	u32 query_out[MLX5_ST_SZ_DW(query_issi_out)] = {0};
	u32 sup_issi;
	int err;

	MLX5_SET(query_issi_in, query_in, opcode, MLX5_CMD_OP_QUERY_ISSI);
	err = mlx5_cmd_exec(dev, query_in, sizeof(query_in),
			    query_out, sizeof(query_out));
	if (err) {
		u32 syndrome;
		u8 status;

		mlx5_cmd_mbox_status(query_out, &status, &syndrome);
		if (!status || syndrome == MLX5_DRIVER_SYND) {
			mlx5_core_err(dev, "Failed to query ISSI err(%d) status(%d) synd(%d)\n",
				      err, status, syndrome);
			return err;
		}

		mlx5_core_warn(dev, "Query ISSI is not supported by FW, ISSI is 0\n");
		dev->issi = 0;
		return 0;
	}

	sup_issi = MLX5_GET(query_issi_out, query_out, supported_issi_dw0);

	if (sup_issi & (1 << 1)) {
		u32 set_in[MLX5_ST_SZ_DW(set_issi_in)]   = {0};
		u32 set_out[MLX5_ST_SZ_DW(set_issi_out)] = {0};

		MLX5_SET(set_issi_in, set_in, opcode, MLX5_CMD_OP_SET_ISSI);
		MLX5_SET(set_issi_in, set_in, current_issi, 1);
		err = mlx5_cmd_exec(dev, set_in, sizeof(set_in),
				    set_out, sizeof(set_out));
		if (err) {
			mlx5_core_err(dev, "Failed to set ISSI to 1 err(%d)\n",
				      err);
			return err;
		}

		dev->issi = 1;

		return 0;
	} else if (sup_issi & (1 << 0) || !sup_issi) {
		return 0;
	}

	return -EOPNOTSUPP;
}

static int mlx5_pci_init(struct mlx5_core_dev *dev, struct mlx5_priv *priv)
{
	struct pci_dev *pdev = dev->pdev;
	int err = 0;

	pci_set_drvdata(dev->pdev, dev);
	strncpy(priv->name, dev_name(&pdev->dev), MLX5_MAX_NAME_LEN);
	priv->name[MLX5_MAX_NAME_LEN - 1] = 0;

	mutex_init(&priv->pgdir_mutex);
	INIT_LIST_HEAD(&priv->pgdir_list);
	spin_lock_init(&priv->mkey_lock);

	mutex_init(&priv->alloc_mutex);

	priv->numa_node = dev_to_node(&dev->pdev->dev);

	if (mlx5_debugfs_root)
		priv->dbg_root =
			debugfs_create_dir(pci_name(pdev), mlx5_debugfs_root);

	err = mlx5_pci_enable_device(dev);
	if (err) {
		dev_err(&pdev->dev, "Cannot enable PCI device, aborting\n");
		goto err_dbg;
	}

	err = request_bar(pdev);
	if (err) {
		dev_err(&pdev->dev, "error requesting BARs, aborting\n");
		goto err_disable;
	}

	pci_set_master(pdev);

	err = set_dma_caps(pdev);
	if (err) {
		dev_err(&pdev->dev, "Failed setting DMA capabilities mask, aborting\n");
		goto err_clr_master;
	}

	dev->iseg_base = pci_resource_start(dev->pdev, 0);
	dev->iseg = ioremap(dev->iseg_base, sizeof(*dev->iseg));
	if (!dev->iseg) {
		err = -ENOMEM;
		dev_err(&pdev->dev, "Failed mapping initialization segment, aborting\n");
		goto err_clr_master;
	}

	return 0;

err_clr_master:
	pci_clear_master(dev->pdev);
	release_bar(dev->pdev);
err_disable:
	mlx5_pci_disable_device(dev);

err_dbg:
	debugfs_remove(priv->dbg_root);
	return err;
}

static void mlx5_pci_close(struct mlx5_core_dev *dev, struct mlx5_priv *priv)
{
	iounmap(dev->iseg);
	pci_clear_master(dev->pdev);
	release_bar(dev->pdev);
	mlx5_pci_disable_device(dev);
	debugfs_remove_recursive(priv->dbg_root);
}

static int mlx5_init_once(struct mlx5_core_dev *dev, struct mlx5_priv *priv)
{
	struct pci_dev *pdev = dev->pdev;
	int err;

	err = mlx5_query_board_id(dev);
	if (err) {
		dev_err(&pdev->dev, "query board id failed\n");
		goto out;
	}

	err = mlx5_eq_init(dev);
	if (err) {
		dev_err(&pdev->dev, "failed to initialize eq\n");
		goto out;
	}

	err = mlx5_cq_debugfs_init(dev);
	if (err) {
		dev_err(&pdev->dev, "failed to initialize cq debugfs\n");
		goto err_eq_cleanup;
	}

	mlx5_init_qp_table(dev);

	mlx5_init_srq_table(dev);

	mlx5_init_mkey_table(dev);

	mlx5_init_reserved_gids(dev);

	mlx5_init_clock(dev);

	dev->vxlan = mlx5_vxlan_create(dev);

	err = mlx5_init_rl_table(dev);
	if (err) {
		dev_err(&pdev->dev, "Failed to init rate limiting\n");
		goto err_tables_cleanup;
	}

	err = mlx5_mpfs_init(dev);
	if (err) {
		dev_err(&pdev->dev, "Failed to init l2 table %d\n", err);
		goto err_rl_cleanup;
	}

	err = mlx5_eswitch_init(dev);
	if (err) {
		dev_err(&pdev->dev, "Failed to init eswitch %d\n", err);
		goto err_mpfs_cleanup;
	}

	err = mlx5_sriov_init(dev);
	if (err) {
		dev_err(&pdev->dev, "Failed to init sriov %d\n", err);
		goto err_eswitch_cleanup;
	}

	err = mlx5_fpga_init(dev);
	if (err) {
		dev_err(&pdev->dev, "Failed to init fpga device %d\n", err);
		goto err_sriov_cleanup;
	}

	dev->tracer = mlx5_fw_tracer_create(dev);

	return 0;

err_sriov_cleanup:
	mlx5_sriov_cleanup(dev);
err_eswitch_cleanup:
	mlx5_eswitch_cleanup(dev->priv.eswitch);
err_mpfs_cleanup:
	mlx5_mpfs_cleanup(dev);
err_rl_cleanup:
	mlx5_cleanup_rl_table(dev);
err_tables_cleanup:
	mlx5_vxlan_destroy(dev->vxlan);
	mlx5_cleanup_mkey_table(dev);
	mlx5_cleanup_srq_table(dev);
	mlx5_cleanup_qp_table(dev);
	mlx5_cq_debugfs_cleanup(dev);

err_eq_cleanup:
	mlx5_eq_cleanup(dev);

out:
	return err;
}

static void mlx5_cleanup_once(struct mlx5_core_dev *dev)
{
	mlx5_fw_tracer_destroy(dev->tracer);
	mlx5_fpga_cleanup(dev);
	mlx5_sriov_cleanup(dev);
	mlx5_eswitch_cleanup(dev->priv.eswitch);
	mlx5_mpfs_cleanup(dev);
	mlx5_cleanup_rl_table(dev);
	mlx5_vxlan_destroy(dev->vxlan);
	mlx5_cleanup_clock(dev);
	mlx5_cleanup_reserved_gids(dev);
	mlx5_cleanup_mkey_table(dev);
	mlx5_cleanup_srq_table(dev);
	mlx5_cleanup_qp_table(dev);
	mlx5_cq_debugfs_cleanup(dev);
	mlx5_eq_cleanup(dev);
}

static int mlx5_load_one(struct mlx5_core_dev *dev, struct mlx5_priv *priv,
			 bool boot)
{
	struct pci_dev *pdev = dev->pdev;
	int err;

	mutex_lock(&dev->intf_state_mutex);
	if (test_bit(MLX5_INTERFACE_STATE_UP, &dev->intf_state)) {
		dev_warn(&dev->pdev->dev, "%s: interface is up, NOP\n",
			 __func__);
		goto out;
	}

	dev_info(&pdev->dev, "firmware version: %d.%d.%d\n", fw_rev_maj(dev),
		 fw_rev_min(dev), fw_rev_sub(dev));

	/* Only PFs hold the relevant PCIe information for this query */
	if (mlx5_core_is_pf(dev))
		pcie_print_link_status(dev->pdev);

	/* on load removing any previous indication of internal error, device is
	 * up
	 */
	dev->state = MLX5_DEVICE_STATE_UP;

	/* wait for firmware to accept initialization segments configurations
	 */
	err = wait_fw_init(dev, FW_PRE_INIT_TIMEOUT_MILI);
	if (err) {
		dev_err(&dev->pdev->dev, "Firmware over %d MS in pre-initializing state, aborting\n",
			FW_PRE_INIT_TIMEOUT_MILI);
		goto out_err;
	}

	err = mlx5_cmd_init(dev);
	if (err) {
		dev_err(&pdev->dev, "Failed initializing command interface, aborting\n");
		goto out_err;
	}

	err = wait_fw_init(dev, FW_INIT_TIMEOUT_MILI);
	if (err) {
		dev_err(&dev->pdev->dev, "Firmware over %d MS in initializing state, aborting\n",
			FW_INIT_TIMEOUT_MILI);
		goto err_cmd_cleanup;
	}

	err = mlx5_core_enable_hca(dev, 0);
	if (err) {
		dev_err(&pdev->dev, "enable hca failed\n");
		goto err_cmd_cleanup;
	}

	err = mlx5_core_set_issi(dev);
	if (err) {
		dev_err(&pdev->dev, "failed to set issi\n");
		goto err_disable_hca;
	}

	err = mlx5_satisfy_startup_pages(dev, 1);
	if (err) {
		dev_err(&pdev->dev, "failed to allocate boot pages\n");
		goto err_disable_hca;
	}

	err = set_hca_ctrl(dev);
	if (err) {
		dev_err(&pdev->dev, "set_hca_ctrl failed\n");
		goto reclaim_boot_pages;
	}

	err = handle_hca_cap(dev);
	if (err) {
		dev_err(&pdev->dev, "handle_hca_cap failed\n");
		goto reclaim_boot_pages;
	}

	err = handle_hca_cap_atomic(dev);
	if (err) {
		dev_err(&pdev->dev, "handle_hca_cap_atomic failed\n");
		goto reclaim_boot_pages;
	}

	err = mlx5_satisfy_startup_pages(dev, 0);
	if (err) {
		dev_err(&pdev->dev, "failed to allocate init pages\n");
		goto reclaim_boot_pages;
	}

	err = mlx5_pagealloc_start(dev);
	if (err) {
		dev_err(&pdev->dev, "mlx5_pagealloc_start failed\n");
		goto reclaim_boot_pages;
	}

	err = mlx5_cmd_init_hca(dev, sw_owner_id);
	if (err) {
		dev_err(&pdev->dev, "init hca failed\n");
		goto err_pagealloc_stop;
	}

	mlx5_set_driver_version(dev);

	mlx5_start_health_poll(dev);

	err = mlx5_query_hca_caps(dev);
	if (err) {
		dev_err(&pdev->dev, "query hca failed\n");
		goto err_stop_poll;
	}

	if (boot) {
		err = mlx5_init_once(dev, priv);
		if (err) {
			dev_err(&pdev->dev, "sw objs init failed\n");
			goto err_stop_poll;
		}
	}

	err = mlx5_alloc_irq_vectors(dev);
	if (err) {
		dev_err(&pdev->dev, "alloc irq vectors failed\n");
		goto err_cleanup_once;
	}

	dev->priv.uar = mlx5_get_uars_page(dev);
	if (IS_ERR(dev->priv.uar)) {
		dev_err(&pdev->dev, "Failed allocating uar, aborting\n");
		err = PTR_ERR(dev->priv.uar);
		goto err_disable_msix;
	}

	err = mlx5_start_eqs(dev);
	if (err) {
		dev_err(&pdev->dev, "Failed to start pages and async EQs\n");
		goto err_put_uars;
	}

	err = mlx5_fw_tracer_init(dev->tracer);
	if (err) {
		dev_err(&pdev->dev, "Failed to init FW tracer\n");
		goto err_fw_tracer;
	}

	err = alloc_comp_eqs(dev);
	if (err) {
		dev_err(&pdev->dev, "Failed to alloc completion EQs\n");
		goto err_comp_eqs;
	}

	err = mlx5_irq_set_affinity_hints(dev);
	if (err) {
		dev_err(&pdev->dev, "Failed to alloc affinity hint cpumask\n");
		goto err_affinity_hints;
	}

	err = mlx5_fpga_device_start(dev);
	if (err) {
		dev_err(&pdev->dev, "fpga device start failed %d\n", err);
		goto err_fpga_start;
	}

	err = mlx5_accel_ipsec_init(dev);
	if (err) {
		dev_err(&pdev->dev, "IPSec device start failed %d\n", err);
		goto err_ipsec_start;
	}

	err = mlx5_accel_tls_init(dev);
	if (err) {
		dev_err(&pdev->dev, "TLS device start failed %d\n", err);
		goto err_tls_start;
	}

	err = mlx5_init_fs(dev);
	if (err) {
		dev_err(&pdev->dev, "Failed to init flow steering\n");
		goto err_fs;
	}

	err = mlx5_core_set_hca_defaults(dev);
	if (err) {
		dev_err(&pdev->dev, "Failed to set hca defaults\n");
		goto err_fs;
	}

	err = mlx5_sriov_attach(dev);
	if (err) {
		dev_err(&pdev->dev, "sriov init failed %d\n", err);
		goto err_sriov;
	}

	if (mlx5_device_registered(dev)) {
		mlx5_attach_device(dev);
	} else {
		err = mlx5_register_device(dev);
		if (err) {
			dev_err(&pdev->dev, "mlx5_register_device failed %d\n", err);
			goto err_reg_dev;
		}
	}

	set_bit(MLX5_INTERFACE_STATE_UP, &dev->intf_state);
out:
	mutex_unlock(&dev->intf_state_mutex);

	return 0;

err_reg_dev:
	mlx5_sriov_detach(dev);

err_sriov:
	mlx5_cleanup_fs(dev);

err_fs:
	mlx5_accel_tls_cleanup(dev);

err_tls_start:
	mlx5_accel_ipsec_cleanup(dev);

err_ipsec_start:
	mlx5_fpga_device_stop(dev);

err_fpga_start:
	mlx5_irq_clear_affinity_hints(dev);

err_affinity_hints:
	free_comp_eqs(dev);

err_comp_eqs:
	mlx5_fw_tracer_cleanup(dev->tracer);

err_fw_tracer:
	mlx5_stop_eqs(dev);

err_put_uars:
	mlx5_put_uars_page(dev, priv->uar);

err_disable_msix:
	mlx5_free_irq_vectors(dev);

err_cleanup_once:
	if (boot)
		mlx5_cleanup_once(dev);

err_stop_poll:
	mlx5_stop_health_poll(dev, boot);
	if (mlx5_cmd_teardown_hca(dev)) {
		dev_err(&dev->pdev->dev, "tear_down_hca failed, skip cleanup\n");
		goto out_err;
	}

err_pagealloc_stop:
	mlx5_pagealloc_stop(dev);

reclaim_boot_pages:
	mlx5_reclaim_startup_pages(dev);

err_disable_hca:
	mlx5_core_disable_hca(dev, 0);

err_cmd_cleanup:
	mlx5_cmd_cleanup(dev);

out_err:
	dev->state = MLX5_DEVICE_STATE_INTERNAL_ERROR;
	mutex_unlock(&dev->intf_state_mutex);

	return err;
}

static int mlx5_unload_one(struct mlx5_core_dev *dev, struct mlx5_priv *priv,
			   bool cleanup)
{
	int err = 0;

	if (cleanup)
		mlx5_drain_health_recovery(dev);

	mutex_lock(&dev->intf_state_mutex);
	if (!test_bit(MLX5_INTERFACE_STATE_UP, &dev->intf_state)) {
		dev_warn(&dev->pdev->dev, "%s: interface is down, NOP\n",
			 __func__);
		if (cleanup)
			mlx5_cleanup_once(dev);
		goto out;
	}

	clear_bit(MLX5_INTERFACE_STATE_UP, &dev->intf_state);

	if (mlx5_device_registered(dev))
		mlx5_detach_device(dev);

	mlx5_sriov_detach(dev);
	mlx5_cleanup_fs(dev);
	mlx5_accel_ipsec_cleanup(dev);
	mlx5_accel_tls_cleanup(dev);
	mlx5_fpga_device_stop(dev);
	mlx5_irq_clear_affinity_hints(dev);
	free_comp_eqs(dev);
	mlx5_fw_tracer_cleanup(dev->tracer);
	mlx5_stop_eqs(dev);
	mlx5_put_uars_page(dev, priv->uar);
	mlx5_free_irq_vectors(dev);
	if (cleanup)
		mlx5_cleanup_once(dev);
	mlx5_stop_health_poll(dev, cleanup);
	err = mlx5_cmd_teardown_hca(dev);
	if (err) {
		dev_err(&dev->pdev->dev, "tear_down_hca failed, skip cleanup\n");
		goto out;
	}
	mlx5_pagealloc_stop(dev);
	mlx5_reclaim_startup_pages(dev);
	mlx5_core_disable_hca(dev, 0);
	mlx5_cmd_cleanup(dev);

out:
	mutex_unlock(&dev->intf_state_mutex);
	return err;
}

struct mlx5_core_event_handler {
	void (*event)(struct mlx5_core_dev *dev,
		      enum mlx5_dev_event event,
		      void *data);
};

static const struct devlink_ops mlx5_devlink_ops = {
#ifdef CONFIG_MLX5_ESWITCH
	.eswitch_mode_set = mlx5_devlink_eswitch_mode_set,
	.eswitch_mode_get = mlx5_devlink_eswitch_mode_get,
	.eswitch_inline_mode_set = mlx5_devlink_eswitch_inline_mode_set,
	.eswitch_inline_mode_get = mlx5_devlink_eswitch_inline_mode_get,
	.eswitch_encap_mode_set = mlx5_devlink_eswitch_encap_mode_set,
	.eswitch_encap_mode_get = mlx5_devlink_eswitch_encap_mode_get,
#endif
};

#define MLX5_IB_MOD "mlx5_ib"
static int init_one(struct pci_dev *pdev,
		    const struct pci_device_id *id)
{
	struct mlx5_core_dev *dev;
	struct devlink *devlink;
	struct mlx5_priv *priv;
	int err;

	devlink = devlink_alloc(&mlx5_devlink_ops, sizeof(*dev));
	if (!devlink) {
		dev_err(&pdev->dev, "kzalloc failed\n");
		return -ENOMEM;
	}

	dev = devlink_priv(devlink);
	priv = &dev->priv;
	priv->pci_dev_data = id->driver_data;

	pci_set_drvdata(pdev, dev);

	dev->pdev = pdev;
	dev->event = mlx5_core_event;
	dev->profile = &profile[prof_sel];

	INIT_LIST_HEAD(&priv->ctx_list);
	spin_lock_init(&priv->ctx_lock);
	mutex_init(&dev->pci_status_mutex);
	mutex_init(&dev->intf_state_mutex);

	INIT_LIST_HEAD(&priv->waiting_events_list);
	priv->is_accum_events = false;

#ifdef CONFIG_INFINIBAND_ON_DEMAND_PAGING
	err = init_srcu_struct(&priv->pfault_srcu);
	if (err) {
		dev_err(&pdev->dev, "init_srcu_struct failed with error code %d\n",
			err);
		goto clean_dev;
	}
#endif
	mutex_init(&priv->bfregs.reg_head.lock);
	mutex_init(&priv->bfregs.wc_head.lock);
	INIT_LIST_HEAD(&priv->bfregs.reg_head.list);
	INIT_LIST_HEAD(&priv->bfregs.wc_head.list);

	err = mlx5_pci_init(dev, priv);
	if (err) {
		dev_err(&pdev->dev, "mlx5_pci_init failed with error code %d\n", err);
		goto clean_srcu;
	}

	err = mlx5_health_init(dev);
	if (err) {
		dev_err(&pdev->dev, "mlx5_health_init failed with error code %d\n", err);
		goto close_pci;
	}

	mlx5_pagealloc_init(dev);

	err = mlx5_load_one(dev, priv, true);
	if (err) {
		dev_err(&pdev->dev, "mlx5_load_one failed with error code %d\n", err);
		goto clean_health;
	}

	request_module_nowait(MLX5_IB_MOD);

	err = devlink_register(devlink, &pdev->dev);
	if (err)
		goto clean_load;

	pci_save_state(pdev);
	return 0;

clean_load:
	mlx5_unload_one(dev, priv, true);
clean_health:
	mlx5_pagealloc_cleanup(dev);
	mlx5_health_cleanup(dev);
close_pci:
	mlx5_pci_close(dev, priv);
clean_srcu:
#ifdef CONFIG_INFINIBAND_ON_DEMAND_PAGING
	cleanup_srcu_struct(&priv->pfault_srcu);
clean_dev:
#endif
	devlink_free(devlink);

	return err;
}

static void remove_one(struct pci_dev *pdev)
{
	struct mlx5_core_dev *dev  = pci_get_drvdata(pdev);
	struct devlink *devlink = priv_to_devlink(dev);
	struct mlx5_priv *priv = &dev->priv;

	devlink_unregister(devlink);
	mlx5_unregister_device(dev);

	if (mlx5_unload_one(dev, priv, true)) {
		dev_err(&dev->pdev->dev, "mlx5_unload_one failed\n");
		mlx5_health_cleanup(dev);
		return;
	}

	mlx5_pagealloc_cleanup(dev);
	mlx5_health_cleanup(dev);
	mlx5_pci_close(dev, priv);
#ifdef CONFIG_INFINIBAND_ON_DEMAND_PAGING
	cleanup_srcu_struct(&priv->pfault_srcu);
#endif
	devlink_free(devlink);
}

static pci_ers_result_t mlx5_pci_err_detected(struct pci_dev *pdev,
					      pci_channel_state_t state)
{
	struct mlx5_core_dev *dev = pci_get_drvdata(pdev);
	struct mlx5_priv *priv = &dev->priv;

	dev_info(&pdev->dev, "%s was called\n", __func__);

	mlx5_enter_error_state(dev, false);
	mlx5_unload_one(dev, priv, false);
	/* In case of kernel call drain the health wq */
	if (state) {
		mlx5_drain_health_wq(dev);
		mlx5_pci_disable_device(dev);
	}

	return state == pci_channel_io_perm_failure ?
		PCI_ERS_RESULT_DISCONNECT : PCI_ERS_RESULT_NEED_RESET;
}

/* wait for the device to show vital signs by waiting
 * for the health counter to start counting.
 */
static int wait_vital(struct pci_dev *pdev)
{
	struct mlx5_core_dev *dev = pci_get_drvdata(pdev);
	struct mlx5_core_health *health = &dev->priv.health;
	const int niter = 100;
	u32 last_count = 0;
	u32 count;
	int i;

	for (i = 0; i < niter; i++) {
		count = ioread32be(health->health_counter);
		if (count && count != 0xffffffff) {
			if (last_count && last_count != count) {
				dev_info(&pdev->dev, "Counter value 0x%x after %d iterations\n", count, i);
				return 0;
			}
			last_count = count;
		}
		msleep(50);
	}

	return -ETIMEDOUT;
}

static pci_ers_result_t mlx5_pci_slot_reset(struct pci_dev *pdev)
{
	struct mlx5_core_dev *dev = pci_get_drvdata(pdev);
	int err;

	dev_info(&pdev->dev, "%s was called\n", __func__);

	err = mlx5_pci_enable_device(dev);
	if (err) {
		dev_err(&pdev->dev, "%s: mlx5_pci_enable_device failed with error code: %d\n"
			, __func__, err);
		return PCI_ERS_RESULT_DISCONNECT;
	}

	pci_set_master(pdev);
	pci_restore_state(pdev);
	pci_save_state(pdev);

	if (wait_vital(pdev)) {
		dev_err(&pdev->dev, "%s: wait_vital timed out\n", __func__);
		return PCI_ERS_RESULT_DISCONNECT;
	}

	return PCI_ERS_RESULT_RECOVERED;
}

static void mlx5_pci_resume(struct pci_dev *pdev)
{
	struct mlx5_core_dev *dev = pci_get_drvdata(pdev);
	struct mlx5_priv *priv = &dev->priv;
	int err;

	dev_info(&pdev->dev, "%s was called\n", __func__);

	err = mlx5_load_one(dev, priv, false);
	if (err)
		dev_err(&pdev->dev, "%s: mlx5_load_one failed with error code: %d\n"
			, __func__, err);
	else
		dev_info(&pdev->dev, "%s: device recovered\n", __func__);
}

static const struct pci_error_handlers mlx5_err_handler = {
	.error_detected = mlx5_pci_err_detected,
	.slot_reset	= mlx5_pci_slot_reset,
	.resume		= mlx5_pci_resume
};

static int mlx5_try_fast_unload(struct mlx5_core_dev *dev)
{
	int ret;

	if (!MLX5_CAP_GEN(dev, force_teardown)) {
		mlx5_core_dbg(dev, "force teardown is not supported in the firmware\n");
		return -EOPNOTSUPP;
	}

	if (dev->state == MLX5_DEVICE_STATE_INTERNAL_ERROR) {
		mlx5_core_dbg(dev, "Device in internal error state, giving up\n");
		return -EAGAIN;
	}

	/* Panic tear down fw command will stop the PCI bus communication
	 * with the HCA, so the health polll is no longer needed.
	 */
	mlx5_drain_health_wq(dev);
	mlx5_stop_health_poll(dev, false);

	ret = mlx5_cmd_force_teardown_hca(dev);
	if (ret) {
		mlx5_core_dbg(dev, "Firmware couldn't do fast unload error: %d\n", ret);
		mlx5_start_health_poll(dev);
		return ret;
	}

	mlx5_enter_error_state(dev, true);

	/* Some platforms requiring freeing the IRQ's in the shutdown
	 * flow. If they aren't freed they can't be allocated after
	 * kexec. There is no need to cleanup the mlx5_core software
	 * contexts.
	 */
	mlx5_irq_clear_affinity_hints(dev);
	mlx5_core_eq_free_irqs(dev);

	return 0;
}

static void shutdown(struct pci_dev *pdev)
{
	struct mlx5_core_dev *dev  = pci_get_drvdata(pdev);
	struct mlx5_priv *priv = &dev->priv;
	int err;

	dev_info(&pdev->dev, "Shutdown was called\n");
	err = mlx5_try_fast_unload(dev);
	if (err)
		mlx5_unload_one(dev, priv, false);
	mlx5_pci_disable_device(dev);
}

static const struct pci_device_id mlx5_core_pci_table[] = {
	{ PCI_VDEVICE(MELLANOX, PCI_DEVICE_ID_MELLANOX_CONNECTIB) },
	{ PCI_VDEVICE(MELLANOX, 0x1012), MLX5_PCI_DEV_IS_VF},	/* Connect-IB VF */
	{ PCI_VDEVICE(MELLANOX, PCI_DEVICE_ID_MELLANOX_CONNECTX4) },
	{ PCI_VDEVICE(MELLANOX, 0x1014), MLX5_PCI_DEV_IS_VF},	/* ConnectX-4 VF */
	{ PCI_VDEVICE(MELLANOX, PCI_DEVICE_ID_MELLANOX_CONNECTX4_LX) },
	{ PCI_VDEVICE(MELLANOX, 0x1016), MLX5_PCI_DEV_IS_VF},	/* ConnectX-4LX VF */
	{ PCI_VDEVICE(MELLANOX, 0x1017) },			/* ConnectX-5, PCIe 3.0 */
	{ PCI_VDEVICE(MELLANOX, 0x1018), MLX5_PCI_DEV_IS_VF},	/* ConnectX-5 VF */
	{ PCI_VDEVICE(MELLANOX, 0x1019) },			/* ConnectX-5 Ex */
	{ PCI_VDEVICE(MELLANOX, 0x101a), MLX5_PCI_DEV_IS_VF},	/* ConnectX-5 Ex VF */
	{ PCI_VDEVICE(MELLANOX, 0x101b) },			/* ConnectX-6 */
	{ PCI_VDEVICE(MELLANOX, 0x101c), MLX5_PCI_DEV_IS_VF},	/* ConnectX-6 VF */
	{ PCI_VDEVICE(MELLANOX, 0xa2d2) },			/* BlueField integrated ConnectX-5 network controller */
	{ PCI_VDEVICE(MELLANOX, 0xa2d3), MLX5_PCI_DEV_IS_VF},	/* BlueField integrated ConnectX-5 network controller VF */
	{ PCI_VDEVICE(MELLANOX, 0xa2d6) },			/* BlueField-2 integrated ConnectX-6 Dx network controller */
	{ 0, }
};

MODULE_DEVICE_TABLE(pci, mlx5_core_pci_table);

void mlx5_disable_device(struct mlx5_core_dev *dev)
{
	mlx5_pci_err_detected(dev->pdev, 0);
}

void mlx5_recover_device(struct mlx5_core_dev *dev)
{
	mlx5_pci_disable_device(dev);
	if (mlx5_pci_slot_reset(dev->pdev) == PCI_ERS_RESULT_RECOVERED)
		mlx5_pci_resume(dev->pdev);
}

static struct pci_driver mlx5_core_driver = {
	.name           = DRIVER_NAME,
	.id_table       = mlx5_core_pci_table,
	.probe          = init_one,
	.remove         = remove_one,
	.shutdown	= shutdown,
	.err_handler	= &mlx5_err_handler,
	.sriov_configure   = mlx5_core_sriov_configure,
};

static void mlx5_core_verify_params(void)
{
	if (prof_sel >= ARRAY_SIZE(profile)) {
		pr_warn("mlx5_core: WARNING: Invalid module parameter prof_sel %d, valid range 0-%zu, changing back to default(%d)\n",
			prof_sel,
			ARRAY_SIZE(profile) - 1,
			MLX5_DEFAULT_PROF);
		prof_sel = MLX5_DEFAULT_PROF;
	}
}

static int __init init(void)
{
	int err;

	get_random_bytes(&sw_owner_id, sizeof(sw_owner_id));

	mlx5_core_verify_params();
	mlx5_fpga_ipsec_build_fs_cmds();
	mlx5_register_debugfs();

	err = pci_register_driver(&mlx5_core_driver);
	if (err)
		goto err_debug;

#ifdef CONFIG_MLX5_CORE_EN
	mlx5e_init();
#endif

	return 0;

err_debug:
	mlx5_unregister_debugfs();
	return err;
}

static void __exit cleanup(void)
{
#ifdef CONFIG_MLX5_CORE_EN
	mlx5e_cleanup();
#endif
	pci_unregister_driver(&mlx5_core_driver);
	mlx5_unregister_debugfs();
}

module_init(init);
module_exit(cleanup);
