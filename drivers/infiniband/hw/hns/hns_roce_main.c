/*
 * Copyright (c) 2016 Hisilicon Limited.
 * Copyright (c) 2007, 2008 Mellanox Technologies. All rights reserved.
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
#include <linux/acpi.h>
#include <linux/of_platform.h>
#include <linux/module.h>
#include <rdma/ib_addr.h>
#include <rdma/ib_smi.h>
#include <rdma/ib_user_verbs.h>
#include <rdma/ib_cache.h>
#include "hns_roce_common.h"
#include "hns_roce_device.h"
#include <rdma/hns-abi.h>
#include "hns_roce_hem.h"

/**
 * hns_get_gid_index - Get gid index.
 * @hr_dev: pointer to structure hns_roce_dev.
 * @port:  port, value range: 0 ~ MAX
 * @gid_index:  gid_index, value range: 0 ~ MAX
 * Description:
 *    N ports shared gids, allocation method as follow:
 *		GID[0][0], GID[1][0],.....GID[N - 1][0],
 *		GID[0][0], GID[1][0],.....GID[N - 1][0],
 *		And so on
 */
int hns_get_gid_index(struct hns_roce_dev *hr_dev, u8 port, int gid_index)
{
	return gid_index * hr_dev->caps.num_ports + port;
}

static void hns_roce_set_mac(struct hns_roce_dev *hr_dev, u8 port, u8 *addr)
{
	u8 phy_port;
	u32 i = 0;

	if (!memcmp(hr_dev->dev_addr[port], addr, MAC_ADDR_OCTET_NUM))
		return;

	for (i = 0; i < MAC_ADDR_OCTET_NUM; i++)
		hr_dev->dev_addr[port][i] = addr[i];

	phy_port = hr_dev->iboe.phy_port[port];
	hr_dev->hw->set_mac(hr_dev, phy_port, addr);
}

static int hns_roce_add_gid(struct ib_device *device, u8 port_num,
			    unsigned int index, const union ib_gid *gid,
			    const struct ib_gid_attr *attr, void **context)
{
	struct hns_roce_dev *hr_dev = to_hr_dev(device);
	u8 port = port_num - 1;
	unsigned long flags;

	if (port >= hr_dev->caps.num_ports)
		return -EINVAL;

	spin_lock_irqsave(&hr_dev->iboe.lock, flags);

	hr_dev->hw->set_gid(hr_dev, port, index, (union ib_gid *)gid);

	spin_unlock_irqrestore(&hr_dev->iboe.lock, flags);

	return 0;
}

static int hns_roce_del_gid(struct ib_device *device, u8 port_num,
			    unsigned int index, void **context)
{
	struct hns_roce_dev *hr_dev = to_hr_dev(device);
	union ib_gid zgid = { {0} };
	u8 port = port_num - 1;
	unsigned long flags;

	if (port >= hr_dev->caps.num_ports)
		return -EINVAL;

	spin_lock_irqsave(&hr_dev->iboe.lock, flags);

	hr_dev->hw->set_gid(hr_dev, port, index, &zgid);

	spin_unlock_irqrestore(&hr_dev->iboe.lock, flags);

	return 0;
}

static int handle_en_event(struct hns_roce_dev *hr_dev, u8 port,
			   unsigned long event)
{
	struct device *dev = &hr_dev->pdev->dev;
	struct net_device *netdev;

	netdev = hr_dev->iboe.netdevs[port];
	if (!netdev) {
		dev_err(dev, "port(%d) can't find netdev\n", port);
		return -ENODEV;
	}

	switch (event) {
	case NETDEV_UP:
	case NETDEV_CHANGE:
	case NETDEV_REGISTER:
	case NETDEV_CHANGEADDR:
		hns_roce_set_mac(hr_dev, port, netdev->dev_addr);
		break;
	case NETDEV_DOWN:
		/*
		 * In v1 engine, only support all ports closed together.
		 */
		break;
	default:
		dev_dbg(dev, "NETDEV event = 0x%x!\n", (u32)(event));
		break;
	}

	return 0;
}

static int hns_roce_netdev_event(struct notifier_block *self,
				 unsigned long event, void *ptr)
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);
	struct hns_roce_ib_iboe *iboe = NULL;
	struct hns_roce_dev *hr_dev = NULL;
	u8 port = 0;
	int ret = 0;

	hr_dev = container_of(self, struct hns_roce_dev, iboe.nb);
	iboe = &hr_dev->iboe;

	for (port = 0; port < hr_dev->caps.num_ports; port++) {
		if (dev == iboe->netdevs[port]) {
			ret = handle_en_event(hr_dev, port, event);
			if (ret)
				return NOTIFY_DONE;
			break;
		}
	}

	return NOTIFY_DONE;
}

static int hns_roce_setup_mtu_mac(struct hns_roce_dev *hr_dev)
{
	u8 i;

	for (i = 0; i < hr_dev->caps.num_ports; i++) {
		hr_dev->hw->set_mtu(hr_dev, hr_dev->iboe.phy_port[i],
				    hr_dev->caps.max_mtu);
		hns_roce_set_mac(hr_dev, i, hr_dev->iboe.netdevs[i]->dev_addr);
	}

	return 0;
}

static int hns_roce_query_device(struct ib_device *ib_dev,
				 struct ib_device_attr *props,
				 struct ib_udata *uhw)
{
	struct hns_roce_dev *hr_dev = to_hr_dev(ib_dev);

	memset(props, 0, sizeof(*props));

	props->sys_image_guid = hr_dev->sys_image_guid;
	props->max_mr_size = (u64)(~(0ULL));
	props->page_size_cap = hr_dev->caps.page_size_cap;
	props->vendor_id = hr_dev->vendor_id;
	props->vendor_part_id = hr_dev->vendor_part_id;
	props->hw_ver = hr_dev->hw_rev;
	props->max_qp = hr_dev->caps.num_qps;
	props->max_qp_wr = hr_dev->caps.max_wqes;
	props->device_cap_flags = IB_DEVICE_PORT_ACTIVE_EVENT |
				  IB_DEVICE_RC_RNR_NAK_GEN;
	props->max_sge = hr_dev->caps.max_sq_sg;
	props->max_sge_rd = 1;
	props->max_cq = hr_dev->caps.num_cqs;
	props->max_cqe = hr_dev->caps.max_cqes;
	props->max_mr = hr_dev->caps.num_mtpts;
	props->max_pd = hr_dev->caps.num_pds;
	props->max_qp_rd_atom = hr_dev->caps.max_qp_dest_rdma;
	props->max_qp_init_rd_atom = hr_dev->caps.max_qp_init_rdma;
	props->atomic_cap = IB_ATOMIC_NONE;
	props->max_pkeys = 1;
	props->local_ca_ack_delay = hr_dev->caps.local_ca_ack_delay;

	return 0;
}

static struct net_device *hns_roce_get_netdev(struct ib_device *ib_dev,
					      u8 port_num)
{
	struct hns_roce_dev *hr_dev = to_hr_dev(ib_dev);
	struct net_device *ndev;

	if (port_num < 1 || port_num > hr_dev->caps.num_ports)
		return NULL;

	rcu_read_lock();

	ndev = hr_dev->iboe.netdevs[port_num - 1];
	if (ndev)
		dev_hold(ndev);

	rcu_read_unlock();
	return ndev;
}

static int hns_roce_query_port(struct ib_device *ib_dev, u8 port_num,
			       struct ib_port_attr *props)
{
	struct hns_roce_dev *hr_dev = to_hr_dev(ib_dev);
	struct device *dev = &hr_dev->pdev->dev;
	struct net_device *net_dev;
	unsigned long flags;
	enum ib_mtu mtu;
	u8 port;

	assert(port_num > 0);
	port = port_num - 1;

	/* props being zeroed by the caller, avoid zeroing it here */

	props->max_mtu = hr_dev->caps.max_mtu;
	props->gid_tbl_len = hr_dev->caps.gid_table_len[port];
	props->port_cap_flags = IB_PORT_CM_SUP | IB_PORT_REINIT_SUP |
				IB_PORT_VENDOR_CLASS_SUP |
				IB_PORT_BOOT_MGMT_SUP;
	props->max_msg_sz = HNS_ROCE_MAX_MSG_LEN;
	props->pkey_tbl_len = 1;
	props->active_width = IB_WIDTH_4X;
	props->active_speed = 1;

	spin_lock_irqsave(&hr_dev->iboe.lock, flags);

	net_dev = hr_dev->iboe.netdevs[port];
	if (!net_dev) {
		spin_unlock_irqrestore(&hr_dev->iboe.lock, flags);
		dev_err(dev, "find netdev %d failed!\r\n", port);
		return -EINVAL;
	}

	mtu = iboe_get_mtu(net_dev->mtu);
	props->active_mtu = mtu ? min(props->max_mtu, mtu) : IB_MTU_256;
	props->state = (netif_running(net_dev) && netif_carrier_ok(net_dev)) ?
			IB_PORT_ACTIVE : IB_PORT_DOWN;
	props->phys_state = (props->state == IB_PORT_ACTIVE) ? 5 : 3;

	spin_unlock_irqrestore(&hr_dev->iboe.lock, flags);

	return 0;
}

static enum rdma_link_layer hns_roce_get_link_layer(struct ib_device *device,
						    u8 port_num)
{
	return IB_LINK_LAYER_ETHERNET;
}

static int hns_roce_query_gid(struct ib_device *ib_dev, u8 port_num, int index,
			      union ib_gid *gid)
{
	return 0;
}

static int hns_roce_query_pkey(struct ib_device *ib_dev, u8 port, u16 index,
			       u16 *pkey)
{
	if (index > 0)
		return -EINVAL;

	*pkey = PKEY_ID;

	return 0;
}

static int hns_roce_modify_device(struct ib_device *ib_dev, int mask,
				  struct ib_device_modify *props)
{
	unsigned long flags;

	if (mask & ~IB_DEVICE_MODIFY_NODE_DESC)
		return -EOPNOTSUPP;

	if (mask & IB_DEVICE_MODIFY_NODE_DESC) {
		spin_lock_irqsave(&to_hr_dev(ib_dev)->sm_lock, flags);
		memcpy(ib_dev->node_desc, props->node_desc, NODE_DESC_SIZE);
		spin_unlock_irqrestore(&to_hr_dev(ib_dev)->sm_lock, flags);
	}

	return 0;
}

static int hns_roce_modify_port(struct ib_device *ib_dev, u8 port_num, int mask,
				struct ib_port_modify *props)
{
	return 0;
}

static struct ib_ucontext *hns_roce_alloc_ucontext(struct ib_device *ib_dev,
						   struct ib_udata *udata)
{
	int ret = 0;
	struct hns_roce_ucontext *context;
	struct hns_roce_ib_alloc_ucontext_resp resp;
	struct hns_roce_dev *hr_dev = to_hr_dev(ib_dev);

	resp.qp_tab_size = hr_dev->caps.num_qps;

	context = kmalloc(sizeof(*context), GFP_KERNEL);
	if (!context)
		return ERR_PTR(-ENOMEM);

	ret = hns_roce_uar_alloc(hr_dev, &context->uar);
	if (ret)
		goto error_fail_uar_alloc;

	ret = ib_copy_to_udata(udata, &resp, sizeof(resp));
	if (ret)
		goto error_fail_copy_to_udata;

	return &context->ibucontext;

error_fail_copy_to_udata:
	hns_roce_uar_free(hr_dev, &context->uar);

error_fail_uar_alloc:
	kfree(context);

	return ERR_PTR(ret);
}

static int hns_roce_dealloc_ucontext(struct ib_ucontext *ibcontext)
{
	struct hns_roce_ucontext *context = to_hr_ucontext(ibcontext);

	hns_roce_uar_free(to_hr_dev(ibcontext->device), &context->uar);
	kfree(context);

	return 0;
}

static int hns_roce_mmap(struct ib_ucontext *context,
			 struct vm_area_struct *vma)
{
	struct hns_roce_dev *hr_dev = to_hr_dev(context->device);

	if (((vma->vm_end - vma->vm_start) % PAGE_SIZE) != 0)
		return -EINVAL;

	if (vma->vm_pgoff == 0) {
		vma->vm_page_prot = pgprot_device(vma->vm_page_prot);
		if (io_remap_pfn_range(vma, vma->vm_start,
				       to_hr_ucontext(context)->uar.pfn,
				       PAGE_SIZE, vma->vm_page_prot))
			return -EAGAIN;
	} else if (vma->vm_pgoff == 1 && hr_dev->hw_rev == HNS_ROCE_HW_VER1) {
		/* vm_pgoff: 1 -- TPTR */
		if (io_remap_pfn_range(vma, vma->vm_start,
				       hr_dev->tptr_dma_addr >> PAGE_SHIFT,
				       hr_dev->tptr_size,
				       vma->vm_page_prot))
			return -EAGAIN;
	} else
		return -EINVAL;

	return 0;
}

static int hns_roce_port_immutable(struct ib_device *ib_dev, u8 port_num,
				   struct ib_port_immutable *immutable)
{
	struct ib_port_attr attr;
	int ret;

	immutable->core_cap_flags = RDMA_CORE_PORT_IBA_ROCE;

	ret = ib_query_port(ib_dev, port_num, &attr);
	if (ret)
		return ret;

	immutable->pkey_tbl_len = attr.pkey_tbl_len;
	immutable->gid_tbl_len = attr.gid_tbl_len;

	immutable->max_mad_size = IB_MGMT_MAD_SIZE;

	return 0;
}

static void hns_roce_unregister_device(struct hns_roce_dev *hr_dev)
{
	struct hns_roce_ib_iboe *iboe = &hr_dev->iboe;

	unregister_inetaddr_notifier(&iboe->nb_inet);
	unregister_netdevice_notifier(&iboe->nb);
	ib_unregister_device(&hr_dev->ib_dev);
}

static int hns_roce_register_device(struct hns_roce_dev *hr_dev)
{
	int ret;
	struct hns_roce_ib_iboe *iboe = NULL;
	struct ib_device *ib_dev = NULL;
	struct device *dev = &hr_dev->pdev->dev;

	iboe = &hr_dev->iboe;
	spin_lock_init(&iboe->lock);

	ib_dev = &hr_dev->ib_dev;
	strlcpy(ib_dev->name, "hns_%d", IB_DEVICE_NAME_MAX);

	ib_dev->owner			= THIS_MODULE;
	ib_dev->node_type		= RDMA_NODE_IB_CA;
	ib_dev->dev.parent		= dev;

	ib_dev->phys_port_cnt		= hr_dev->caps.num_ports;
	ib_dev->local_dma_lkey		= hr_dev->caps.reserved_lkey;
	ib_dev->num_comp_vectors	= hr_dev->caps.num_comp_vectors;
	ib_dev->uverbs_abi_ver		= 1;
	ib_dev->uverbs_cmd_mask		=
		(1ULL << IB_USER_VERBS_CMD_GET_CONTEXT) |
		(1ULL << IB_USER_VERBS_CMD_QUERY_DEVICE) |
		(1ULL << IB_USER_VERBS_CMD_QUERY_PORT) |
		(1ULL << IB_USER_VERBS_CMD_ALLOC_PD) |
		(1ULL << IB_USER_VERBS_CMD_DEALLOC_PD) |
		(1ULL << IB_USER_VERBS_CMD_REG_MR) |
		(1ULL << IB_USER_VERBS_CMD_DEREG_MR) |
		(1ULL << IB_USER_VERBS_CMD_CREATE_COMP_CHANNEL) |
		(1ULL << IB_USER_VERBS_CMD_CREATE_CQ) |
		(1ULL << IB_USER_VERBS_CMD_DESTROY_CQ) |
		(1ULL << IB_USER_VERBS_CMD_CREATE_QP) |
		(1ULL << IB_USER_VERBS_CMD_MODIFY_QP) |
		(1ULL << IB_USER_VERBS_CMD_QUERY_QP) |
		(1ULL << IB_USER_VERBS_CMD_DESTROY_QP);

	/* HCA||device||port */
	ib_dev->modify_device		= hns_roce_modify_device;
	ib_dev->query_device		= hns_roce_query_device;
	ib_dev->query_port		= hns_roce_query_port;
	ib_dev->modify_port		= hns_roce_modify_port;
	ib_dev->get_link_layer		= hns_roce_get_link_layer;
	ib_dev->get_netdev		= hns_roce_get_netdev;
	ib_dev->query_gid		= hns_roce_query_gid;
	ib_dev->add_gid			= hns_roce_add_gid;
	ib_dev->del_gid			= hns_roce_del_gid;
	ib_dev->query_pkey		= hns_roce_query_pkey;
	ib_dev->alloc_ucontext		= hns_roce_alloc_ucontext;
	ib_dev->dealloc_ucontext	= hns_roce_dealloc_ucontext;
	ib_dev->mmap			= hns_roce_mmap;

	/* PD */
	ib_dev->alloc_pd		= hns_roce_alloc_pd;
	ib_dev->dealloc_pd		= hns_roce_dealloc_pd;

	/* AH */
	ib_dev->create_ah		= hns_roce_create_ah;
	ib_dev->query_ah		= hns_roce_query_ah;
	ib_dev->destroy_ah		= hns_roce_destroy_ah;

	/* QP */
	ib_dev->create_qp		= hns_roce_create_qp;
	ib_dev->modify_qp		= hns_roce_modify_qp;
	ib_dev->query_qp		= hr_dev->hw->query_qp;
	ib_dev->destroy_qp		= hr_dev->hw->destroy_qp;
	ib_dev->post_send		= hr_dev->hw->post_send;
	ib_dev->post_recv		= hr_dev->hw->post_recv;

	/* CQ */
	ib_dev->create_cq		= hns_roce_ib_create_cq;
	ib_dev->destroy_cq		= hns_roce_ib_destroy_cq;
	ib_dev->req_notify_cq		= hr_dev->hw->req_notify_cq;
	ib_dev->poll_cq			= hr_dev->hw->poll_cq;

	/* MR */
	ib_dev->get_dma_mr		= hns_roce_get_dma_mr;
	ib_dev->reg_user_mr		= hns_roce_reg_user_mr;
	ib_dev->dereg_mr		= hns_roce_dereg_mr;

	/* OTHERS */
	ib_dev->get_port_immutable	= hns_roce_port_immutable;

	ret = ib_register_device(ib_dev, NULL);
	if (ret) {
		dev_err(dev, "ib_register_device failed!\n");
		return ret;
	}

	ret = hns_roce_setup_mtu_mac(hr_dev);
	if (ret) {
		dev_err(dev, "setup_mtu_mac failed!\n");
		goto error_failed_setup_mtu_mac;
	}

	iboe->nb.notifier_call = hns_roce_netdev_event;
	ret = register_netdevice_notifier(&iboe->nb);
	if (ret) {
		dev_err(dev, "register_netdevice_notifier failed!\n");
		goto error_failed_setup_mtu_mac;
	}

	return 0;

error_failed_setup_mtu_mac:
	ib_unregister_device(ib_dev);

	return ret;
}

static const struct of_device_id hns_roce_of_match[] = {
	{ .compatible = "hisilicon,hns-roce-v1", .data = &hns_roce_hw_v1, },
	{},
};
MODULE_DEVICE_TABLE(of, hns_roce_of_match);

static const struct acpi_device_id hns_roce_acpi_match[] = {
	{ "HISI00D1", (kernel_ulong_t)&hns_roce_hw_v1 },
	{},
};
MODULE_DEVICE_TABLE(acpi, hns_roce_acpi_match);

static int hns_roce_node_match(struct device *dev, void *fwnode)
{
	return dev->fwnode == fwnode;
}

static struct
platform_device *hns_roce_find_pdev(struct fwnode_handle *fwnode)
{
	struct device *dev;

	/* get the 'device'corresponding to matching 'fwnode' */
	dev = bus_find_device(&platform_bus_type, NULL,
			      fwnode, hns_roce_node_match);
	/* get the platform device */
	return dev ? to_platform_device(dev) : NULL;
}

static int hns_roce_get_cfg(struct hns_roce_dev *hr_dev)
{
	int i;
	int ret;
	u8 phy_port;
	int port_cnt = 0;
	struct device *dev = &hr_dev->pdev->dev;
	struct device_node *net_node;
	struct net_device *netdev = NULL;
	struct platform_device *pdev = NULL;
	struct resource *res;

	/* check if we are compatible with the underlying SoC */
	if (dev_of_node(dev)) {
		const struct of_device_id *of_id;

		of_id = of_match_node(hns_roce_of_match, dev->of_node);
		if (!of_id) {
			dev_err(dev, "device is not compatible!\n");
			return -ENXIO;
		}
		hr_dev->hw = (struct hns_roce_hw *)of_id->data;
		if (!hr_dev->hw) {
			dev_err(dev, "couldn't get H/W specific DT data!\n");
			return -ENXIO;
		}
	} else if (is_acpi_device_node(dev->fwnode)) {
		const struct acpi_device_id *acpi_id;

		acpi_id = acpi_match_device(hns_roce_acpi_match, dev);
		if (!acpi_id) {
			dev_err(dev, "device is not compatible!\n");
			return -ENXIO;
		}
		hr_dev->hw = (struct hns_roce_hw *) acpi_id->driver_data;
		if (!hr_dev->hw) {
			dev_err(dev, "couldn't get H/W specific ACPI data!\n");
			return -ENXIO;
		}
	} else {
		dev_err(dev, "can't read compatibility data from DT or ACPI\n");
		return -ENXIO;
	}

	/* get the mapped register base address */
	res = platform_get_resource(hr_dev->pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(dev, "memory resource not found!\n");
		return -EINVAL;
	}
	hr_dev->reg_base = devm_ioremap_resource(dev, res);
	if (IS_ERR(hr_dev->reg_base))
		return PTR_ERR(hr_dev->reg_base);

	/* read the node_guid of IB device from the DT or ACPI */
	ret = device_property_read_u8_array(dev, "node-guid",
					    (u8 *)&hr_dev->ib_dev.node_guid,
					    GUID_LEN);
	if (ret) {
		dev_err(dev, "couldn't get node_guid from DT or ACPI!\n");
		return ret;
	}

	/* get the RoCE associated ethernet ports or netdevices */
	for (i = 0; i < HNS_ROCE_MAX_PORTS; i++) {
		if (dev_of_node(dev)) {
			net_node = of_parse_phandle(dev->of_node, "eth-handle",
						    i);
			if (!net_node)
				continue;
			pdev = of_find_device_by_node(net_node);
		} else if (is_acpi_device_node(dev->fwnode)) {
			struct acpi_reference_args args;
			struct fwnode_handle *fwnode;

			ret = acpi_node_get_property_reference(dev->fwnode,
							       "eth-handle",
							       i, &args);
			if (ret)
				continue;
			fwnode = acpi_fwnode_handle(args.adev);
			pdev = hns_roce_find_pdev(fwnode);
		} else {
			dev_err(dev, "cannot read data from DT or ACPI\n");
			return -ENXIO;
		}

		if (pdev) {
			netdev = platform_get_drvdata(pdev);
			phy_port = (u8)i;
			if (netdev) {
				hr_dev->iboe.netdevs[port_cnt] = netdev;
				hr_dev->iboe.phy_port[port_cnt] = phy_port;
			} else {
				dev_err(dev, "no netdev found with pdev %s\n",
					pdev->name);
				return -ENODEV;
			}
			port_cnt++;
		}
	}

	if (port_cnt == 0) {
		dev_err(dev, "unable to get eth-handle for available ports!\n");
		return -EINVAL;
	}

	hr_dev->caps.num_ports = port_cnt;

	/* cmd issue mode: 0 is poll, 1 is event */
	hr_dev->cmd_mod = 1;
	hr_dev->loop_idc = 0;

	/* read the interrupt names from the DT or ACPI */
	ret = device_property_read_string_array(dev, "interrupt-names",
						hr_dev->irq_names,
						HNS_ROCE_MAX_IRQ_NUM);
	if (ret < 0) {
		dev_err(dev, "couldn't get interrupt names from DT or ACPI!\n");
		return ret;
	}

	/* fetch the interrupt numbers */
	for (i = 0; i < HNS_ROCE_MAX_IRQ_NUM; i++) {
		hr_dev->irq[i] = platform_get_irq(hr_dev->pdev, i);
		if (hr_dev->irq[i] <= 0) {
			dev_err(dev, "platform get of irq[=%d] failed!\n", i);
			return -EINVAL;
		}
	}

	return 0;
}

static int hns_roce_init_hem(struct hns_roce_dev *hr_dev)
{
	int ret;
	struct device *dev = &hr_dev->pdev->dev;

	ret = hns_roce_init_hem_table(hr_dev, &hr_dev->mr_table.mtt_table,
				      HEM_TYPE_MTT, hr_dev->caps.mtt_entry_sz,
				      hr_dev->caps.num_mtt_segs, 1);
	if (ret) {
		dev_err(dev, "Failed to init MTT context memory, aborting.\n");
		return ret;
	}

	ret = hns_roce_init_hem_table(hr_dev, &hr_dev->mr_table.mtpt_table,
				      HEM_TYPE_MTPT, hr_dev->caps.mtpt_entry_sz,
				      hr_dev->caps.num_mtpts, 1);
	if (ret) {
		dev_err(dev, "Failed to init MTPT context memory, aborting.\n");
		goto err_unmap_mtt;
	}

	ret = hns_roce_init_hem_table(hr_dev, &hr_dev->qp_table.qp_table,
				      HEM_TYPE_QPC, hr_dev->caps.qpc_entry_sz,
				      hr_dev->caps.num_qps, 1);
	if (ret) {
		dev_err(dev, "Failed to init QP context memory, aborting.\n");
		goto err_unmap_dmpt;
	}

	ret = hns_roce_init_hem_table(hr_dev, &hr_dev->qp_table.irrl_table,
				      HEM_TYPE_IRRL,
				      hr_dev->caps.irrl_entry_sz *
				      hr_dev->caps.max_qp_init_rdma,
				      hr_dev->caps.num_qps, 1);
	if (ret) {
		dev_err(dev, "Failed to init irrl_table memory, aborting.\n");
		goto err_unmap_qp;
	}

	ret = hns_roce_init_hem_table(hr_dev, &hr_dev->cq_table.table,
				      HEM_TYPE_CQC, hr_dev->caps.cqc_entry_sz,
				      hr_dev->caps.num_cqs, 1);
	if (ret) {
		dev_err(dev, "Failed to init CQ context memory, aborting.\n");
		goto err_unmap_irrl;
	}

	return 0;

err_unmap_irrl:
	hns_roce_cleanup_hem_table(hr_dev, &hr_dev->qp_table.irrl_table);

err_unmap_qp:
	hns_roce_cleanup_hem_table(hr_dev, &hr_dev->qp_table.qp_table);

err_unmap_dmpt:
	hns_roce_cleanup_hem_table(hr_dev, &hr_dev->mr_table.mtpt_table);

err_unmap_mtt:
	hns_roce_cleanup_hem_table(hr_dev, &hr_dev->mr_table.mtt_table);

	return ret;
}

/**
 * hns_roce_setup_hca - setup host channel adapter
 * @hr_dev: pointer to hns roce device
 * Return : int
 */
static int hns_roce_setup_hca(struct hns_roce_dev *hr_dev)
{
	int ret;
	struct device *dev = &hr_dev->pdev->dev;

	spin_lock_init(&hr_dev->sm_lock);
	spin_lock_init(&hr_dev->bt_cmd_lock);

	ret = hns_roce_init_uar_table(hr_dev);
	if (ret) {
		dev_err(dev, "Failed to initialize uar table. aborting\n");
		return ret;
	}

	ret = hns_roce_uar_alloc(hr_dev, &hr_dev->priv_uar);
	if (ret) {
		dev_err(dev, "Failed to allocate priv_uar.\n");
		goto err_uar_table_free;
	}

	ret = hns_roce_init_pd_table(hr_dev);
	if (ret) {
		dev_err(dev, "Failed to init protected domain table.\n");
		goto err_uar_alloc_free;
	}

	ret = hns_roce_init_mr_table(hr_dev);
	if (ret) {
		dev_err(dev, "Failed to init memory region table.\n");
		goto err_pd_table_free;
	}

	ret = hns_roce_init_cq_table(hr_dev);
	if (ret) {
		dev_err(dev, "Failed to init completion queue table.\n");
		goto err_mr_table_free;
	}

	ret = hns_roce_init_qp_table(hr_dev);
	if (ret) {
		dev_err(dev, "Failed to init queue pair table.\n");
		goto err_cq_table_free;
	}

	return 0;

err_cq_table_free:
	hns_roce_cleanup_cq_table(hr_dev);

err_mr_table_free:
	hns_roce_cleanup_mr_table(hr_dev);

err_pd_table_free:
	hns_roce_cleanup_pd_table(hr_dev);

err_uar_alloc_free:
	hns_roce_uar_free(hr_dev, &hr_dev->priv_uar);

err_uar_table_free:
	hns_roce_cleanup_uar_table(hr_dev);
	return ret;
}

/**
 * hns_roce_probe - RoCE driver entrance
 * @pdev: pointer to platform device
 * Return : int
 *
 */
static int hns_roce_probe(struct platform_device *pdev)
{
	int ret;
	struct hns_roce_dev *hr_dev;
	struct device *dev = &pdev->dev;

	hr_dev = (struct hns_roce_dev *)ib_alloc_device(sizeof(*hr_dev));
	if (!hr_dev)
		return -ENOMEM;

	hr_dev->pdev = pdev;
	platform_set_drvdata(pdev, hr_dev);

	if (dma_set_mask_and_coherent(dev, DMA_BIT_MASK(64ULL)) &&
	    dma_set_mask_and_coherent(dev, DMA_BIT_MASK(32ULL))) {
		dev_err(dev, "Not usable DMA addressing mode\n");
		ret = -EIO;
		goto error_failed_get_cfg;
	}

	ret = hns_roce_get_cfg(hr_dev);
	if (ret) {
		dev_err(dev, "Get Configuration failed!\n");
		goto error_failed_get_cfg;
	}

	ret = hr_dev->hw->reset(hr_dev, true);
	if (ret) {
		dev_err(dev, "Reset RoCE engine failed!\n");
		goto error_failed_get_cfg;
	}

	hr_dev->hw->hw_profile(hr_dev);

	ret = hns_roce_cmd_init(hr_dev);
	if (ret) {
		dev_err(dev, "cmd init failed!\n");
		goto error_failed_cmd_init;
	}

	ret = hns_roce_init_eq_table(hr_dev);
	if (ret) {
		dev_err(dev, "eq init failed!\n");
		goto error_failed_eq_table;
	}

	if (hr_dev->cmd_mod) {
		ret = hns_roce_cmd_use_events(hr_dev);
		if (ret) {
			dev_err(dev, "Switch to event-driven cmd failed!\n");
			goto error_failed_use_event;
		}
	}

	ret = hns_roce_init_hem(hr_dev);
	if (ret) {
		dev_err(dev, "init HEM(Hardware Entry Memory) failed!\n");
		goto error_failed_init_hem;
	}

	ret = hns_roce_setup_hca(hr_dev);
	if (ret) {
		dev_err(dev, "setup hca failed!\n");
		goto error_failed_setup_hca;
	}

	ret = hr_dev->hw->hw_init(hr_dev);
	if (ret) {
		dev_err(dev, "hw_init failed!\n");
		goto error_failed_engine_init;
	}

	ret = hns_roce_register_device(hr_dev);
	if (ret)
		goto error_failed_register_device;

	return 0;

error_failed_register_device:
	hr_dev->hw->hw_exit(hr_dev);

error_failed_engine_init:
	hns_roce_cleanup_bitmap(hr_dev);

error_failed_setup_hca:
	hns_roce_cleanup_hem(hr_dev);

error_failed_init_hem:
	if (hr_dev->cmd_mod)
		hns_roce_cmd_use_polling(hr_dev);

error_failed_use_event:
	hns_roce_cleanup_eq_table(hr_dev);

error_failed_eq_table:
	hns_roce_cmd_cleanup(hr_dev);

error_failed_cmd_init:
	ret = hr_dev->hw->reset(hr_dev, false);
	if (ret)
		dev_err(&hr_dev->pdev->dev, "roce_engine reset fail\n");

error_failed_get_cfg:
	ib_dealloc_device(&hr_dev->ib_dev);

	return ret;
}

/**
 * hns_roce_remove - remove RoCE device
 * @pdev: pointer to platform device
 */
static int hns_roce_remove(struct platform_device *pdev)
{
	struct hns_roce_dev *hr_dev = platform_get_drvdata(pdev);

	hns_roce_unregister_device(hr_dev);
	hr_dev->hw->hw_exit(hr_dev);
	hns_roce_cleanup_bitmap(hr_dev);
	hns_roce_cleanup_hem(hr_dev);

	if (hr_dev->cmd_mod)
		hns_roce_cmd_use_polling(hr_dev);

	hns_roce_cleanup_eq_table(hr_dev);
	hns_roce_cmd_cleanup(hr_dev);
	hr_dev->hw->reset(hr_dev, false);

	ib_dealloc_device(&hr_dev->ib_dev);

	return 0;
}

static struct platform_driver hns_roce_driver = {
	.probe = hns_roce_probe,
	.remove = hns_roce_remove,
	.driver = {
		.name = DRV_NAME,
		.of_match_table = hns_roce_of_match,
		.acpi_match_table = ACPI_PTR(hns_roce_acpi_match),
	},
};

module_platform_driver(hns_roce_driver);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Wei Hu <xavier.huwei@huawei.com>");
MODULE_AUTHOR("Nenglong Zhao <zhaonenglong@hisilicon.com>");
MODULE_AUTHOR("Lijun Ou <oulijun@huawei.com>");
MODULE_DESCRIPTION("HNS RoCE Driver");
