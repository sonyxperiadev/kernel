// SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB
/*
 * Copyright (c) 2018, Mellanox Technologies inc.  All rights reserved.
 */

#include <rdma/ib_user_verbs.h>
#include <rdma/ib_verbs.h>
#include <rdma/uverbs_types.h>
#include <rdma/uverbs_ioctl.h>
#include <rdma/mlx5_user_ioctl_cmds.h>
#include <rdma/ib_umem.h>
#include <linux/mlx5/driver.h>
#include <linux/mlx5/fs.h>
#include "mlx5_ib.h"

#define UVERBS_MODULE_NAME mlx5_ib
#include <rdma/uverbs_named_ioctl.h>

#define MLX5_MAX_DESTROY_INBOX_SIZE_DW MLX5_ST_SZ_DW(delete_fte_in)
struct devx_obj {
	struct mlx5_core_dev	*mdev;
	u32			obj_id;
	u32			dinlen; /* destroy inbox length */
	u32			dinbox[MLX5_MAX_DESTROY_INBOX_SIZE_DW];
};

struct devx_umem {
	struct mlx5_core_dev		*mdev;
	struct ib_umem			*umem;
	u32				page_offset;
	int				page_shift;
	int				ncont;
	u32				dinlen;
	u32				dinbox[MLX5_ST_SZ_DW(general_obj_in_cmd_hdr)];
};

struct devx_umem_reg_cmd {
	void				*in;
	u32				inlen;
	u32				out[MLX5_ST_SZ_DW(general_obj_out_cmd_hdr)];
};

static struct mlx5_ib_ucontext *devx_ufile2uctx(struct ib_uverbs_file *file)
{
	return to_mucontext(ib_uverbs_get_ucontext(file));
}

int mlx5_ib_devx_create(struct mlx5_ib_dev *dev, struct mlx5_ib_ucontext *context)
{
	u32 in[MLX5_ST_SZ_DW(create_uctx_in)] = {0};
	u32 out[MLX5_ST_SZ_DW(general_obj_out_cmd_hdr)] = {0};
	u64 general_obj_types;
	void *hdr;
	int err;

	hdr = MLX5_ADDR_OF(create_uctx_in, in, hdr);

	general_obj_types = MLX5_CAP_GEN_64(dev->mdev, general_obj_types);
	if (!(general_obj_types & MLX5_GENERAL_OBJ_TYPES_CAP_UCTX) ||
	    !(general_obj_types & MLX5_GENERAL_OBJ_TYPES_CAP_UMEM))
		return -EINVAL;

	if (!capable(CAP_NET_RAW))
		return -EPERM;

	MLX5_SET(general_obj_in_cmd_hdr, hdr, opcode, MLX5_CMD_OP_CREATE_GENERAL_OBJECT);
	MLX5_SET(general_obj_in_cmd_hdr, hdr, obj_type, MLX5_OBJ_TYPE_UCTX);

	err = mlx5_cmd_exec(dev->mdev, in, sizeof(in), out, sizeof(out));
	if (err)
		return err;

	context->devx_uid = MLX5_GET(general_obj_out_cmd_hdr, out, obj_id);
	return 0;
}

void mlx5_ib_devx_destroy(struct mlx5_ib_dev *dev,
			  struct mlx5_ib_ucontext *context)
{
	u32 in[MLX5_ST_SZ_DW(general_obj_in_cmd_hdr)] = {0};
	u32 out[MLX5_ST_SZ_DW(general_obj_out_cmd_hdr)] = {0};

	MLX5_SET(general_obj_in_cmd_hdr, in, opcode, MLX5_CMD_OP_DESTROY_GENERAL_OBJECT);
	MLX5_SET(general_obj_in_cmd_hdr, in, obj_type, MLX5_OBJ_TYPE_UCTX);
	MLX5_SET(general_obj_in_cmd_hdr, in, obj_id, context->devx_uid);

	mlx5_cmd_exec(dev->mdev, in, sizeof(in), out, sizeof(out));
}

bool mlx5_ib_devx_is_flow_dest(void *obj, int *dest_id, int *dest_type)
{
	struct devx_obj *devx_obj = obj;
	u16 opcode = MLX5_GET(general_obj_in_cmd_hdr, devx_obj->dinbox, opcode);

	switch (opcode) {
	case MLX5_CMD_OP_DESTROY_TIR:
		*dest_type = MLX5_FLOW_DESTINATION_TYPE_TIR;
		*dest_id = MLX5_GET(general_obj_in_cmd_hdr, devx_obj->dinbox,
				    obj_id);
		return true;

	case MLX5_CMD_OP_DESTROY_FLOW_TABLE:
		*dest_type = MLX5_FLOW_DESTINATION_TYPE_FLOW_TABLE;
		*dest_id = MLX5_GET(destroy_flow_table_in, devx_obj->dinbox,
				    table_id);
		return true;
	default:
		return false;
	}
}

static int devx_is_valid_obj_id(struct devx_obj *obj, const void *in)
{
	u16 opcode = MLX5_GET(general_obj_in_cmd_hdr, in, opcode);
	u32 obj_id;

	switch (opcode) {
	case MLX5_CMD_OP_MODIFY_GENERAL_OBJECT:
	case MLX5_CMD_OP_QUERY_GENERAL_OBJECT:
		obj_id = MLX5_GET(general_obj_in_cmd_hdr, in, obj_id);
		break;
	case MLX5_CMD_OP_QUERY_MKEY:
		obj_id = MLX5_GET(query_mkey_in, in, mkey_index);
		break;
	case MLX5_CMD_OP_QUERY_CQ:
		obj_id = MLX5_GET(query_cq_in, in, cqn);
		break;
	case MLX5_CMD_OP_MODIFY_CQ:
		obj_id = MLX5_GET(modify_cq_in, in, cqn);
		break;
	case MLX5_CMD_OP_QUERY_SQ:
		obj_id = MLX5_GET(query_sq_in, in, sqn);
		break;
	case MLX5_CMD_OP_MODIFY_SQ:
		obj_id = MLX5_GET(modify_sq_in, in, sqn);
		break;
	case MLX5_CMD_OP_QUERY_RQ:
		obj_id = MLX5_GET(query_rq_in, in, rqn);
		break;
	case MLX5_CMD_OP_MODIFY_RQ:
		obj_id = MLX5_GET(modify_rq_in, in, rqn);
		break;
	case MLX5_CMD_OP_QUERY_RMP:
		obj_id = MLX5_GET(query_rmp_in, in, rmpn);
		break;
	case MLX5_CMD_OP_MODIFY_RMP:
		obj_id = MLX5_GET(modify_rmp_in, in, rmpn);
		break;
	case MLX5_CMD_OP_QUERY_RQT:
		obj_id = MLX5_GET(query_rqt_in, in, rqtn);
		break;
	case MLX5_CMD_OP_MODIFY_RQT:
		obj_id = MLX5_GET(modify_rqt_in, in, rqtn);
		break;
	case MLX5_CMD_OP_QUERY_TIR:
		obj_id = MLX5_GET(query_tir_in, in, tirn);
		break;
	case MLX5_CMD_OP_MODIFY_TIR:
		obj_id = MLX5_GET(modify_tir_in, in, tirn);
		break;
	case MLX5_CMD_OP_QUERY_TIS:
		obj_id = MLX5_GET(query_tis_in, in, tisn);
		break;
	case MLX5_CMD_OP_MODIFY_TIS:
		obj_id = MLX5_GET(modify_tis_in, in, tisn);
		break;
	case MLX5_CMD_OP_QUERY_FLOW_TABLE:
		obj_id = MLX5_GET(query_flow_table_in, in, table_id);
		break;
	case MLX5_CMD_OP_MODIFY_FLOW_TABLE:
		obj_id = MLX5_GET(modify_flow_table_in, in, table_id);
		break;
	case MLX5_CMD_OP_QUERY_FLOW_GROUP:
		obj_id = MLX5_GET(query_flow_group_in, in, group_id);
		break;
	case MLX5_CMD_OP_QUERY_FLOW_TABLE_ENTRY:
		obj_id = MLX5_GET(query_fte_in, in, flow_index);
		break;
	case MLX5_CMD_OP_SET_FLOW_TABLE_ENTRY:
		obj_id = MLX5_GET(set_fte_in, in, flow_index);
		break;
	case MLX5_CMD_OP_QUERY_Q_COUNTER:
		obj_id = MLX5_GET(query_q_counter_in, in, counter_set_id);
		break;
	case MLX5_CMD_OP_QUERY_FLOW_COUNTER:
		obj_id = MLX5_GET(query_flow_counter_in, in, flow_counter_id);
		break;
	case MLX5_CMD_OP_QUERY_MODIFY_HEADER_CONTEXT:
		obj_id = MLX5_GET(general_obj_in_cmd_hdr, in, obj_id);
		break;
	case MLX5_CMD_OP_QUERY_SCHEDULING_ELEMENT:
		obj_id = MLX5_GET(query_scheduling_element_in, in,
				  scheduling_element_id);
		break;
	case MLX5_CMD_OP_MODIFY_SCHEDULING_ELEMENT:
		obj_id = MLX5_GET(modify_scheduling_element_in, in,
				  scheduling_element_id);
		break;
	case MLX5_CMD_OP_ADD_VXLAN_UDP_DPORT:
		obj_id = MLX5_GET(add_vxlan_udp_dport_in, in, vxlan_udp_port);
		break;
	case MLX5_CMD_OP_QUERY_L2_TABLE_ENTRY:
		obj_id = MLX5_GET(query_l2_table_entry_in, in, table_index);
		break;
	case MLX5_CMD_OP_SET_L2_TABLE_ENTRY:
		obj_id = MLX5_GET(set_l2_table_entry_in, in, table_index);
		break;
	case MLX5_CMD_OP_QUERY_QP:
		obj_id = MLX5_GET(query_qp_in, in, qpn);
		break;
	case MLX5_CMD_OP_RST2INIT_QP:
		obj_id = MLX5_GET(rst2init_qp_in, in, qpn);
		break;
	case MLX5_CMD_OP_INIT2RTR_QP:
		obj_id = MLX5_GET(init2rtr_qp_in, in, qpn);
		break;
	case MLX5_CMD_OP_RTR2RTS_QP:
		obj_id = MLX5_GET(rtr2rts_qp_in, in, qpn);
		break;
	case MLX5_CMD_OP_RTS2RTS_QP:
		obj_id = MLX5_GET(rts2rts_qp_in, in, qpn);
		break;
	case MLX5_CMD_OP_SQERR2RTS_QP:
		obj_id = MLX5_GET(sqerr2rts_qp_in, in, qpn);
		break;
	case MLX5_CMD_OP_2ERR_QP:
		obj_id = MLX5_GET(qp_2err_in, in, qpn);
		break;
	case MLX5_CMD_OP_2RST_QP:
		obj_id = MLX5_GET(qp_2rst_in, in, qpn);
		break;
	case MLX5_CMD_OP_QUERY_DCT:
		obj_id = MLX5_GET(query_dct_in, in, dctn);
		break;
	case MLX5_CMD_OP_QUERY_XRQ:
		obj_id = MLX5_GET(query_xrq_in, in, xrqn);
		break;
	case MLX5_CMD_OP_QUERY_XRC_SRQ:
		obj_id = MLX5_GET(query_xrc_srq_in, in, xrc_srqn);
		break;
	case MLX5_CMD_OP_ARM_XRC_SRQ:
		obj_id = MLX5_GET(arm_xrc_srq_in, in, xrc_srqn);
		break;
	case MLX5_CMD_OP_QUERY_SRQ:
		obj_id = MLX5_GET(query_srq_in, in, srqn);
		break;
	case MLX5_CMD_OP_ARM_RQ:
		obj_id = MLX5_GET(arm_rq_in, in, srq_number);
		break;
	case MLX5_CMD_OP_DRAIN_DCT:
	case MLX5_CMD_OP_ARM_DCT_FOR_KEY_VIOLATION:
		obj_id = MLX5_GET(drain_dct_in, in, dctn);
		break;
	case MLX5_CMD_OP_ARM_XRQ:
		obj_id = MLX5_GET(arm_xrq_in, in, xrqn);
		break;
	default:
		return false;
	}

	if (obj_id == obj->obj_id)
		return true;

	return false;
}

static bool devx_is_obj_create_cmd(const void *in)
{
	u16 opcode = MLX5_GET(general_obj_in_cmd_hdr, in, opcode);

	switch (opcode) {
	case MLX5_CMD_OP_CREATE_GENERAL_OBJECT:
	case MLX5_CMD_OP_CREATE_MKEY:
	case MLX5_CMD_OP_CREATE_CQ:
	case MLX5_CMD_OP_ALLOC_PD:
	case MLX5_CMD_OP_ALLOC_TRANSPORT_DOMAIN:
	case MLX5_CMD_OP_CREATE_RMP:
	case MLX5_CMD_OP_CREATE_SQ:
	case MLX5_CMD_OP_CREATE_RQ:
	case MLX5_CMD_OP_CREATE_RQT:
	case MLX5_CMD_OP_CREATE_TIR:
	case MLX5_CMD_OP_CREATE_TIS:
	case MLX5_CMD_OP_ALLOC_Q_COUNTER:
	case MLX5_CMD_OP_CREATE_FLOW_TABLE:
	case MLX5_CMD_OP_CREATE_FLOW_GROUP:
	case MLX5_CMD_OP_ALLOC_FLOW_COUNTER:
	case MLX5_CMD_OP_ALLOC_ENCAP_HEADER:
	case MLX5_CMD_OP_ALLOC_MODIFY_HEADER_CONTEXT:
	case MLX5_CMD_OP_CREATE_SCHEDULING_ELEMENT:
	case MLX5_CMD_OP_ADD_VXLAN_UDP_DPORT:
	case MLX5_CMD_OP_SET_L2_TABLE_ENTRY:
	case MLX5_CMD_OP_CREATE_QP:
	case MLX5_CMD_OP_CREATE_SRQ:
	case MLX5_CMD_OP_CREATE_XRC_SRQ:
	case MLX5_CMD_OP_CREATE_DCT:
	case MLX5_CMD_OP_CREATE_XRQ:
	case MLX5_CMD_OP_ATTACH_TO_MCG:
	case MLX5_CMD_OP_ALLOC_XRCD:
		return true;
	case MLX5_CMD_OP_SET_FLOW_TABLE_ENTRY:
	{
		u16 op_mod = MLX5_GET(set_fte_in, in, op_mod);
		if (op_mod == 0)
			return true;
		return false;
	}
	default:
		return false;
	}
}

static bool devx_is_obj_modify_cmd(const void *in)
{
	u16 opcode = MLX5_GET(general_obj_in_cmd_hdr, in, opcode);

	switch (opcode) {
	case MLX5_CMD_OP_MODIFY_GENERAL_OBJECT:
	case MLX5_CMD_OP_MODIFY_CQ:
	case MLX5_CMD_OP_MODIFY_RMP:
	case MLX5_CMD_OP_MODIFY_SQ:
	case MLX5_CMD_OP_MODIFY_RQ:
	case MLX5_CMD_OP_MODIFY_RQT:
	case MLX5_CMD_OP_MODIFY_TIR:
	case MLX5_CMD_OP_MODIFY_TIS:
	case MLX5_CMD_OP_MODIFY_FLOW_TABLE:
	case MLX5_CMD_OP_MODIFY_SCHEDULING_ELEMENT:
	case MLX5_CMD_OP_ADD_VXLAN_UDP_DPORT:
	case MLX5_CMD_OP_SET_L2_TABLE_ENTRY:
	case MLX5_CMD_OP_RST2INIT_QP:
	case MLX5_CMD_OP_INIT2RTR_QP:
	case MLX5_CMD_OP_INIT2INIT_QP:
	case MLX5_CMD_OP_RTR2RTS_QP:
	case MLX5_CMD_OP_RTS2RTS_QP:
	case MLX5_CMD_OP_SQERR2RTS_QP:
	case MLX5_CMD_OP_2ERR_QP:
	case MLX5_CMD_OP_2RST_QP:
	case MLX5_CMD_OP_ARM_XRC_SRQ:
	case MLX5_CMD_OP_ARM_RQ:
	case MLX5_CMD_OP_DRAIN_DCT:
	case MLX5_CMD_OP_ARM_DCT_FOR_KEY_VIOLATION:
	case MLX5_CMD_OP_ARM_XRQ:
		return true;
	case MLX5_CMD_OP_SET_FLOW_TABLE_ENTRY:
	{
		u16 op_mod = MLX5_GET(set_fte_in, in, op_mod);

		if (op_mod == 1)
			return true;
		return false;
	}
	default:
		return false;
	}
}

static bool devx_is_obj_query_cmd(const void *in)
{
	u16 opcode = MLX5_GET(general_obj_in_cmd_hdr, in, opcode);

	switch (opcode) {
	case MLX5_CMD_OP_QUERY_GENERAL_OBJECT:
	case MLX5_CMD_OP_QUERY_MKEY:
	case MLX5_CMD_OP_QUERY_CQ:
	case MLX5_CMD_OP_QUERY_RMP:
	case MLX5_CMD_OP_QUERY_SQ:
	case MLX5_CMD_OP_QUERY_RQ:
	case MLX5_CMD_OP_QUERY_RQT:
	case MLX5_CMD_OP_QUERY_TIR:
	case MLX5_CMD_OP_QUERY_TIS:
	case MLX5_CMD_OP_QUERY_Q_COUNTER:
	case MLX5_CMD_OP_QUERY_FLOW_TABLE:
	case MLX5_CMD_OP_QUERY_FLOW_GROUP:
	case MLX5_CMD_OP_QUERY_FLOW_TABLE_ENTRY:
	case MLX5_CMD_OP_QUERY_FLOW_COUNTER:
	case MLX5_CMD_OP_QUERY_MODIFY_HEADER_CONTEXT:
	case MLX5_CMD_OP_QUERY_SCHEDULING_ELEMENT:
	case MLX5_CMD_OP_QUERY_L2_TABLE_ENTRY:
	case MLX5_CMD_OP_QUERY_QP:
	case MLX5_CMD_OP_QUERY_SRQ:
	case MLX5_CMD_OP_QUERY_XRC_SRQ:
	case MLX5_CMD_OP_QUERY_DCT:
	case MLX5_CMD_OP_QUERY_XRQ:
		return true;
	default:
		return false;
	}
}

static bool devx_is_general_cmd(void *in)
{
	u16 opcode = MLX5_GET(general_obj_in_cmd_hdr, in, opcode);

	switch (opcode) {
	case MLX5_CMD_OP_QUERY_HCA_CAP:
	case MLX5_CMD_OP_QUERY_VPORT_STATE:
	case MLX5_CMD_OP_QUERY_ADAPTER:
	case MLX5_CMD_OP_QUERY_ISSI:
	case MLX5_CMD_OP_QUERY_NIC_VPORT_CONTEXT:
	case MLX5_CMD_OP_QUERY_ROCE_ADDRESS:
	case MLX5_CMD_OP_QUERY_VNIC_ENV:
	case MLX5_CMD_OP_QUERY_VPORT_COUNTER:
	case MLX5_CMD_OP_GET_DROPPED_PACKET_LOG:
	case MLX5_CMD_OP_NOP:
	case MLX5_CMD_OP_QUERY_CONG_STATUS:
	case MLX5_CMD_OP_QUERY_CONG_PARAMS:
	case MLX5_CMD_OP_QUERY_CONG_STATISTICS:
		return true;
	default:
		return false;
	}
}

static int UVERBS_HANDLER(MLX5_IB_METHOD_DEVX_QUERY_EQN)(
	struct ib_uverbs_file *file, struct uverbs_attr_bundle *attrs)
{
	struct mlx5_ib_ucontext *c;
	struct mlx5_ib_dev *dev;
	int user_vector;
	int dev_eqn;
	unsigned int irqn;
	int err;

	if (uverbs_copy_from(&user_vector, attrs,
			     MLX5_IB_ATTR_DEVX_QUERY_EQN_USER_VEC))
		return -EFAULT;

	c = devx_ufile2uctx(file);
	if (IS_ERR(c))
		return PTR_ERR(c);
	dev = to_mdev(c->ibucontext.device);

	err = mlx5_vector2eqn(dev->mdev, user_vector, &dev_eqn, &irqn);
	if (err < 0)
		return err;

	if (uverbs_copy_to(attrs, MLX5_IB_ATTR_DEVX_QUERY_EQN_DEV_EQN,
			   &dev_eqn, sizeof(dev_eqn)))
		return -EFAULT;

	return 0;
}

/*
 *Security note:
 * The hardware protection mechanism works like this: Each device object that
 * is subject to UAR doorbells (QP/SQ/CQ) gets a UAR ID (called uar_page in
 * the device specification manual) upon its creation. Then upon doorbell,
 * hardware fetches the object context for which the doorbell was rang, and
 * validates that the UAR through which the DB was rang matches the UAR ID
 * of the object.
 * If no match the doorbell is silently ignored by the hardware. Of course,
 * the user cannot ring a doorbell on a UAR that was not mapped to it.
 * Now in devx, as the devx kernel does not manipulate the QP/SQ/CQ command
 * mailboxes (except tagging them with UID), we expose to the user its UAR
 * ID, so it can embed it in these objects in the expected specification
 * format. So the only thing the user can do is hurt itself by creating a
 * QP/SQ/CQ with a UAR ID other than his, and then in this case other users
 * may ring a doorbell on its objects.
 * The consequence of that will be that another user can schedule a QP/SQ
 * of the buggy user for execution (just insert it to the hardware schedule
 * queue or arm its CQ for event generation), no further harm is expected.
 */
static int UVERBS_HANDLER(MLX5_IB_METHOD_DEVX_QUERY_UAR)(
	struct ib_uverbs_file *file, struct uverbs_attr_bundle *attrs)
{
	struct mlx5_ib_ucontext *c;
	struct mlx5_ib_dev *dev;
	u32 user_idx;
	s32 dev_idx;

	c = devx_ufile2uctx(file);
	if (IS_ERR(c))
		return PTR_ERR(c);
	dev = to_mdev(c->ibucontext.device);

	if (uverbs_copy_from(&user_idx, attrs,
			     MLX5_IB_ATTR_DEVX_QUERY_UAR_USER_IDX))
		return -EFAULT;

	dev_idx = bfregn_to_uar_index(dev, &c->bfregi, user_idx, true);
	if (dev_idx < 0)
		return dev_idx;

	if (uverbs_copy_to(attrs, MLX5_IB_ATTR_DEVX_QUERY_UAR_DEV_IDX,
			   &dev_idx, sizeof(dev_idx)))
		return -EFAULT;

	return 0;
}

static int UVERBS_HANDLER(MLX5_IB_METHOD_DEVX_OTHER)(
	struct ib_uverbs_file *file, struct uverbs_attr_bundle *attrs)
{
	struct mlx5_ib_ucontext *c;
	struct mlx5_ib_dev *dev;
	void *cmd_in = uverbs_attr_get_alloced_ptr(
		attrs, MLX5_IB_ATTR_DEVX_OTHER_CMD_IN);
	int cmd_out_len = uverbs_attr_get_len(attrs,
					MLX5_IB_ATTR_DEVX_OTHER_CMD_OUT);
	void *cmd_out;
	int err;

	c = devx_ufile2uctx(file);
	if (IS_ERR(c))
		return PTR_ERR(c);
	dev = to_mdev(c->ibucontext.device);

	if (!c->devx_uid)
		return -EPERM;

	/* Only white list of some general HCA commands are allowed for this method. */
	if (!devx_is_general_cmd(cmd_in))
		return -EINVAL;

	cmd_out = uverbs_zalloc(attrs, cmd_out_len);
	if (IS_ERR(cmd_out))
		return PTR_ERR(cmd_out);

	MLX5_SET(general_obj_in_cmd_hdr, cmd_in, uid, c->devx_uid);
	err = mlx5_cmd_exec(dev->mdev, cmd_in,
			    uverbs_attr_get_len(attrs, MLX5_IB_ATTR_DEVX_OTHER_CMD_IN),
			    cmd_out, cmd_out_len);
	if (err)
		return err;

	return uverbs_copy_to(attrs, MLX5_IB_ATTR_DEVX_OTHER_CMD_OUT, cmd_out,
			      cmd_out_len);
}

static void devx_obj_build_destroy_cmd(void *in, void *out, void *din,
				       u32 *dinlen,
				       u32 *obj_id)
{
	u16 obj_type = MLX5_GET(general_obj_in_cmd_hdr, in, obj_type);
	u16 uid = MLX5_GET(general_obj_in_cmd_hdr, in, uid);

	*obj_id = MLX5_GET(general_obj_out_cmd_hdr, out, obj_id);
	*dinlen = MLX5_ST_SZ_BYTES(general_obj_in_cmd_hdr);

	MLX5_SET(general_obj_in_cmd_hdr, din, obj_id, *obj_id);
	MLX5_SET(general_obj_in_cmd_hdr, din, uid, uid);

	switch (MLX5_GET(general_obj_in_cmd_hdr, in, opcode)) {
	case MLX5_CMD_OP_CREATE_GENERAL_OBJECT:
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode, MLX5_CMD_OP_DESTROY_GENERAL_OBJECT);
		MLX5_SET(general_obj_in_cmd_hdr, din, obj_type, obj_type);
		break;

	case MLX5_CMD_OP_CREATE_MKEY:
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode, MLX5_CMD_OP_DESTROY_MKEY);
		break;
	case MLX5_CMD_OP_CREATE_CQ:
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode, MLX5_CMD_OP_DESTROY_CQ);
		break;
	case MLX5_CMD_OP_ALLOC_PD:
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode, MLX5_CMD_OP_DEALLOC_PD);
		break;
	case MLX5_CMD_OP_ALLOC_TRANSPORT_DOMAIN:
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode,
			 MLX5_CMD_OP_DEALLOC_TRANSPORT_DOMAIN);
		break;
	case MLX5_CMD_OP_CREATE_RMP:
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode, MLX5_CMD_OP_DESTROY_RMP);
		break;
	case MLX5_CMD_OP_CREATE_SQ:
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode, MLX5_CMD_OP_DESTROY_SQ);
		break;
	case MLX5_CMD_OP_CREATE_RQ:
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode, MLX5_CMD_OP_DESTROY_RQ);
		break;
	case MLX5_CMD_OP_CREATE_RQT:
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode, MLX5_CMD_OP_DESTROY_RQT);
		break;
	case MLX5_CMD_OP_CREATE_TIR:
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode, MLX5_CMD_OP_DESTROY_TIR);
		break;
	case MLX5_CMD_OP_CREATE_TIS:
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode, MLX5_CMD_OP_DESTROY_TIS);
		break;
	case MLX5_CMD_OP_ALLOC_Q_COUNTER:
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode,
			 MLX5_CMD_OP_DEALLOC_Q_COUNTER);
		break;
	case MLX5_CMD_OP_CREATE_FLOW_TABLE:
		*dinlen = MLX5_ST_SZ_BYTES(destroy_flow_table_in);
		*obj_id = MLX5_GET(create_flow_table_out, out, table_id);
		MLX5_SET(destroy_flow_table_in, din, other_vport,
			 MLX5_GET(create_flow_table_in,  in, other_vport));
		MLX5_SET(destroy_flow_table_in, din, vport_number,
			 MLX5_GET(create_flow_table_in,  in, vport_number));
		MLX5_SET(destroy_flow_table_in, din, table_type,
			 MLX5_GET(create_flow_table_in,  in, table_type));
		MLX5_SET(destroy_flow_table_in, din, table_id, *obj_id);
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode,
			 MLX5_CMD_OP_DESTROY_FLOW_TABLE);
		break;
	case MLX5_CMD_OP_CREATE_FLOW_GROUP:
		*dinlen = MLX5_ST_SZ_BYTES(destroy_flow_group_in);
		*obj_id = MLX5_GET(create_flow_group_out, out, group_id);
		MLX5_SET(destroy_flow_group_in, din, other_vport,
			 MLX5_GET(create_flow_group_in, in, other_vport));
		MLX5_SET(destroy_flow_group_in, din, vport_number,
			 MLX5_GET(create_flow_group_in, in, vport_number));
		MLX5_SET(destroy_flow_group_in, din, table_type,
			 MLX5_GET(create_flow_group_in, in, table_type));
		MLX5_SET(destroy_flow_group_in, din, table_id,
			 MLX5_GET(create_flow_group_in, in, table_id));
		MLX5_SET(destroy_flow_group_in, din, group_id, *obj_id);
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode,
			 MLX5_CMD_OP_DESTROY_FLOW_GROUP);
		break;
	case MLX5_CMD_OP_SET_FLOW_TABLE_ENTRY:
		*dinlen = MLX5_ST_SZ_BYTES(delete_fte_in);
		*obj_id = MLX5_GET(set_fte_in, in, flow_index);
		MLX5_SET(delete_fte_in, din, other_vport,
			 MLX5_GET(set_fte_in,  in, other_vport));
		MLX5_SET(delete_fte_in, din, vport_number,
			 MLX5_GET(set_fte_in, in, vport_number));
		MLX5_SET(delete_fte_in, din, table_type,
			 MLX5_GET(set_fte_in, in, table_type));
		MLX5_SET(delete_fte_in, din, table_id,
			 MLX5_GET(set_fte_in, in, table_id));
		MLX5_SET(delete_fte_in, din, flow_index, *obj_id);
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode,
			 MLX5_CMD_OP_DELETE_FLOW_TABLE_ENTRY);
		break;
	case MLX5_CMD_OP_ALLOC_FLOW_COUNTER:
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode,
			 MLX5_CMD_OP_DEALLOC_FLOW_COUNTER);
		break;
	case MLX5_CMD_OP_ALLOC_ENCAP_HEADER:
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode,
			 MLX5_CMD_OP_DEALLOC_ENCAP_HEADER);
		break;
	case MLX5_CMD_OP_ALLOC_MODIFY_HEADER_CONTEXT:
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode,
			 MLX5_CMD_OP_DEALLOC_MODIFY_HEADER_CONTEXT);
		break;
	case MLX5_CMD_OP_CREATE_SCHEDULING_ELEMENT:
		*dinlen = MLX5_ST_SZ_BYTES(destroy_scheduling_element_in);
		*obj_id = MLX5_GET(create_scheduling_element_out, out,
				   scheduling_element_id);
		MLX5_SET(destroy_scheduling_element_in, din,
			 scheduling_hierarchy,
			 MLX5_GET(create_scheduling_element_in, in,
				  scheduling_hierarchy));
		MLX5_SET(destroy_scheduling_element_in, din,
			 scheduling_element_id, *obj_id);
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode,
			 MLX5_CMD_OP_DESTROY_SCHEDULING_ELEMENT);
		break;
	case MLX5_CMD_OP_ADD_VXLAN_UDP_DPORT:
		*dinlen = MLX5_ST_SZ_BYTES(delete_vxlan_udp_dport_in);
		*obj_id = MLX5_GET(add_vxlan_udp_dport_in, in, vxlan_udp_port);
		MLX5_SET(delete_vxlan_udp_dport_in, din, vxlan_udp_port, *obj_id);
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode,
			 MLX5_CMD_OP_DELETE_VXLAN_UDP_DPORT);
		break;
	case MLX5_CMD_OP_SET_L2_TABLE_ENTRY:
		*dinlen = MLX5_ST_SZ_BYTES(delete_l2_table_entry_in);
		*obj_id = MLX5_GET(set_l2_table_entry_in, in, table_index);
		MLX5_SET(delete_l2_table_entry_in, din, table_index, *obj_id);
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode,
			 MLX5_CMD_OP_DELETE_L2_TABLE_ENTRY);
		break;
	case MLX5_CMD_OP_CREATE_QP:
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode, MLX5_CMD_OP_DESTROY_QP);
		break;
	case MLX5_CMD_OP_CREATE_SRQ:
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode, MLX5_CMD_OP_DESTROY_SRQ);
		break;
	case MLX5_CMD_OP_CREATE_XRC_SRQ:
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode,
			 MLX5_CMD_OP_DESTROY_XRC_SRQ);
		break;
	case MLX5_CMD_OP_CREATE_DCT:
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode, MLX5_CMD_OP_DESTROY_DCT);
		break;
	case MLX5_CMD_OP_CREATE_XRQ:
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode, MLX5_CMD_OP_DESTROY_XRQ);
		break;
	case MLX5_CMD_OP_ATTACH_TO_MCG:
		*dinlen = MLX5_ST_SZ_BYTES(detach_from_mcg_in);
		MLX5_SET(detach_from_mcg_in, din, qpn,
			 MLX5_GET(attach_to_mcg_in, in, qpn));
		memcpy(MLX5_ADDR_OF(detach_from_mcg_in, din, multicast_gid),
		       MLX5_ADDR_OF(attach_to_mcg_in, in, multicast_gid),
		       MLX5_FLD_SZ_BYTES(attach_to_mcg_in, multicast_gid));
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode, MLX5_CMD_OP_DETACH_FROM_MCG);
		break;
	case MLX5_CMD_OP_ALLOC_XRCD:
		MLX5_SET(general_obj_in_cmd_hdr, din, opcode, MLX5_CMD_OP_DEALLOC_XRCD);
		break;
	default:
		/* The entry must match to one of the devx_is_obj_create_cmd */
		WARN_ON(true);
		break;
	}
}

static int devx_obj_cleanup(struct ib_uobject *uobject,
			    enum rdma_remove_reason why)
{
	u32 out[MLX5_ST_SZ_DW(general_obj_out_cmd_hdr)];
	struct devx_obj *obj = uobject->object;
	int ret;

	ret = mlx5_cmd_exec(obj->mdev, obj->dinbox, obj->dinlen, out, sizeof(out));
	if (ib_is_destroy_retryable(ret, why, uobject))
		return ret;

	kfree(obj);
	return ret;
}

static int UVERBS_HANDLER(MLX5_IB_METHOD_DEVX_OBJ_CREATE)(
	struct ib_uverbs_file *file, struct uverbs_attr_bundle *attrs)
{
	void *cmd_in = uverbs_attr_get_alloced_ptr(attrs, MLX5_IB_ATTR_DEVX_OBJ_CREATE_CMD_IN);
	int cmd_out_len =  uverbs_attr_get_len(attrs,
					MLX5_IB_ATTR_DEVX_OBJ_CREATE_CMD_OUT);
	void *cmd_out;
	struct ib_uobject *uobj = uverbs_attr_get_uobject(
		attrs, MLX5_IB_ATTR_DEVX_OBJ_CREATE_HANDLE);
	struct mlx5_ib_ucontext *c = to_mucontext(uobj->context);
	struct mlx5_ib_dev *dev = to_mdev(c->ibucontext.device);
	u32 out[MLX5_ST_SZ_DW(general_obj_out_cmd_hdr)];
	struct devx_obj *obj;
	int err;

	if (!c->devx_uid)
		return -EPERM;

	if (!devx_is_obj_create_cmd(cmd_in))
		return -EINVAL;

	cmd_out = uverbs_zalloc(attrs, cmd_out_len);
	if (IS_ERR(cmd_out))
		return PTR_ERR(cmd_out);

	obj = kzalloc(sizeof(struct devx_obj), GFP_KERNEL);
	if (!obj)
		return -ENOMEM;

	MLX5_SET(general_obj_in_cmd_hdr, cmd_in, uid, c->devx_uid);
	err = mlx5_cmd_exec(dev->mdev, cmd_in,
			    uverbs_attr_get_len(attrs, MLX5_IB_ATTR_DEVX_OBJ_CREATE_CMD_IN),
			    cmd_out, cmd_out_len);
	if (err)
		goto obj_free;

	uobj->object = obj;
	obj->mdev = dev->mdev;
	devx_obj_build_destroy_cmd(cmd_in, cmd_out, obj->dinbox, &obj->dinlen, &obj->obj_id);
	WARN_ON(obj->dinlen > MLX5_MAX_DESTROY_INBOX_SIZE_DW * sizeof(u32));

	err = uverbs_copy_to(attrs, MLX5_IB_ATTR_DEVX_OBJ_CREATE_CMD_OUT, cmd_out, cmd_out_len);
	if (err)
		goto obj_destroy;

	return 0;

obj_destroy:
	mlx5_cmd_exec(obj->mdev, obj->dinbox, obj->dinlen, out, sizeof(out));
obj_free:
	kfree(obj);
	return err;
}

static int UVERBS_HANDLER(MLX5_IB_METHOD_DEVX_OBJ_MODIFY)(
	struct ib_uverbs_file *file, struct uverbs_attr_bundle *attrs)
{
	void *cmd_in = uverbs_attr_get_alloced_ptr(attrs, MLX5_IB_ATTR_DEVX_OBJ_MODIFY_CMD_IN);
	int cmd_out_len = uverbs_attr_get_len(attrs,
					MLX5_IB_ATTR_DEVX_OBJ_MODIFY_CMD_OUT);
	struct ib_uobject *uobj = uverbs_attr_get_uobject(attrs,
							  MLX5_IB_ATTR_DEVX_OBJ_MODIFY_HANDLE);
	struct mlx5_ib_ucontext *c = to_mucontext(uobj->context);
	struct devx_obj *obj = uobj->object;
	void *cmd_out;
	int err;

	if (!c->devx_uid)
		return -EPERM;

	if (!devx_is_obj_modify_cmd(cmd_in))
		return -EINVAL;

	if (!devx_is_valid_obj_id(obj, cmd_in))
		return -EINVAL;

	cmd_out = uverbs_zalloc(attrs, cmd_out_len);
	if (IS_ERR(cmd_out))
		return PTR_ERR(cmd_out);

	MLX5_SET(general_obj_in_cmd_hdr, cmd_in, uid, c->devx_uid);
	err = mlx5_cmd_exec(obj->mdev, cmd_in,
			    uverbs_attr_get_len(attrs, MLX5_IB_ATTR_DEVX_OBJ_MODIFY_CMD_IN),
			    cmd_out, cmd_out_len);
	if (err)
		return err;

	return uverbs_copy_to(attrs, MLX5_IB_ATTR_DEVX_OBJ_MODIFY_CMD_OUT,
			      cmd_out, cmd_out_len);
}

static int UVERBS_HANDLER(MLX5_IB_METHOD_DEVX_OBJ_QUERY)(
	struct ib_uverbs_file *file, struct uverbs_attr_bundle *attrs)
{
	void *cmd_in = uverbs_attr_get_alloced_ptr(attrs, MLX5_IB_ATTR_DEVX_OBJ_QUERY_CMD_IN);
	int cmd_out_len = uverbs_attr_get_len(attrs,
					      MLX5_IB_ATTR_DEVX_OBJ_QUERY_CMD_OUT);
	struct ib_uobject *uobj = uverbs_attr_get_uobject(attrs,
							  MLX5_IB_ATTR_DEVX_OBJ_QUERY_HANDLE);
	struct mlx5_ib_ucontext *c = to_mucontext(uobj->context);
	struct devx_obj *obj = uobj->object;
	void *cmd_out;
	int err;

	if (!c->devx_uid)
		return -EPERM;

	if (!devx_is_obj_query_cmd(cmd_in))
		return -EINVAL;

	if (!devx_is_valid_obj_id(obj, cmd_in))
		return -EINVAL;

	cmd_out = uverbs_zalloc(attrs, cmd_out_len);
	if (IS_ERR(cmd_out))
		return PTR_ERR(cmd_out);

	MLX5_SET(general_obj_in_cmd_hdr, cmd_in, uid, c->devx_uid);
	err = mlx5_cmd_exec(obj->mdev, cmd_in,
			    uverbs_attr_get_len(attrs, MLX5_IB_ATTR_DEVX_OBJ_QUERY_CMD_IN),
			    cmd_out, cmd_out_len);
	if (err)
		return err;

	return uverbs_copy_to(attrs, MLX5_IB_ATTR_DEVX_OBJ_QUERY_CMD_OUT,
			      cmd_out, cmd_out_len);
}

static int devx_umem_get(struct mlx5_ib_dev *dev, struct ib_ucontext *ucontext,
			 struct uverbs_attr_bundle *attrs,
			 struct devx_umem *obj)
{
	u64 addr;
	size_t size;
	u32 access;
	int npages;
	int err;
	u32 page_mask;

	if (uverbs_copy_from(&addr, attrs, MLX5_IB_ATTR_DEVX_UMEM_REG_ADDR) ||
	    uverbs_copy_from(&size, attrs, MLX5_IB_ATTR_DEVX_UMEM_REG_LEN))
		return -EFAULT;

	err = uverbs_get_flags32(&access, attrs,
				 MLX5_IB_ATTR_DEVX_UMEM_REG_ACCESS,
				 IB_ACCESS_LOCAL_WRITE |
				 IB_ACCESS_REMOTE_WRITE |
				 IB_ACCESS_REMOTE_READ);
	if (err)
		return err;

	err = ib_check_mr_access(access);
	if (err)
		return err;

	obj->umem = ib_umem_get(ucontext, addr, size, access, 0);
	if (IS_ERR(obj->umem))
		return PTR_ERR(obj->umem);

	mlx5_ib_cont_pages(obj->umem, obj->umem->address,
			   MLX5_MKEY_PAGE_SHIFT_MASK, &npages,
			   &obj->page_shift, &obj->ncont, NULL);

	if (!npages) {
		ib_umem_release(obj->umem);
		return -EINVAL;
	}

	page_mask = (1 << obj->page_shift) - 1;
	obj->page_offset = obj->umem->address & page_mask;

	return 0;
}

static int devx_umem_reg_cmd_alloc(struct uverbs_attr_bundle *attrs,
				   struct devx_umem *obj,
				   struct devx_umem_reg_cmd *cmd)
{
	cmd->inlen = MLX5_ST_SZ_BYTES(create_umem_in) +
		    (MLX5_ST_SZ_BYTES(mtt) * obj->ncont);
	cmd->in = uverbs_zalloc(attrs, cmd->inlen);
	return PTR_ERR_OR_ZERO(cmd->in);
}

static void devx_umem_reg_cmd_build(struct mlx5_ib_dev *dev,
				    struct devx_umem *obj,
				    struct devx_umem_reg_cmd *cmd)
{
	void *umem;
	__be64 *mtt;

	umem = MLX5_ADDR_OF(create_umem_in, cmd->in, umem);
	mtt = (__be64 *)MLX5_ADDR_OF(umem, umem, mtt);

	MLX5_SET(general_obj_in_cmd_hdr, cmd->in, opcode, MLX5_CMD_OP_CREATE_GENERAL_OBJECT);
	MLX5_SET(general_obj_in_cmd_hdr, cmd->in, obj_type, MLX5_OBJ_TYPE_UMEM);
	MLX5_SET64(umem, umem, num_of_mtt, obj->ncont);
	MLX5_SET(umem, umem, log_page_size, obj->page_shift -
					    MLX5_ADAPTER_PAGE_SHIFT);
	MLX5_SET(umem, umem, page_offset, obj->page_offset);
	mlx5_ib_populate_pas(dev, obj->umem, obj->page_shift, mtt,
			     (obj->umem->writable ? MLX5_IB_MTT_WRITE : 0) |
			     MLX5_IB_MTT_READ);
}

static int UVERBS_HANDLER(MLX5_IB_METHOD_DEVX_UMEM_REG)(
	struct ib_uverbs_file *file, struct uverbs_attr_bundle *attrs)
{
	struct devx_umem_reg_cmd cmd;
	struct devx_umem *obj;
	struct ib_uobject *uobj = uverbs_attr_get_uobject(
		attrs, MLX5_IB_ATTR_DEVX_UMEM_REG_HANDLE);
	u32 obj_id;
	struct mlx5_ib_ucontext *c = to_mucontext(uobj->context);
	struct mlx5_ib_dev *dev = to_mdev(c->ibucontext.device);
	int err;

	if (!c->devx_uid)
		return -EPERM;

	obj = kzalloc(sizeof(struct devx_umem), GFP_KERNEL);
	if (!obj)
		return -ENOMEM;

	err = devx_umem_get(dev, &c->ibucontext, attrs, obj);
	if (err)
		goto err_obj_free;

	err = devx_umem_reg_cmd_alloc(attrs, obj, &cmd);
	if (err)
		goto err_umem_release;

	devx_umem_reg_cmd_build(dev, obj, &cmd);

	MLX5_SET(general_obj_in_cmd_hdr, cmd.in, uid, c->devx_uid);
	err = mlx5_cmd_exec(dev->mdev, cmd.in, cmd.inlen, cmd.out,
			    sizeof(cmd.out));
	if (err)
		goto err_umem_release;

	obj->mdev = dev->mdev;
	uobj->object = obj;
	devx_obj_build_destroy_cmd(cmd.in, cmd.out, obj->dinbox, &obj->dinlen, &obj_id);
	err = uverbs_copy_to(attrs, MLX5_IB_ATTR_DEVX_UMEM_REG_OUT_ID, &obj_id, sizeof(obj_id));
	if (err)
		goto err_umem_destroy;

	return 0;

err_umem_destroy:
	mlx5_cmd_exec(obj->mdev, obj->dinbox, obj->dinlen, cmd.out, sizeof(cmd.out));
err_umem_release:
	ib_umem_release(obj->umem);
err_obj_free:
	kfree(obj);
	return err;
}

static int devx_umem_cleanup(struct ib_uobject *uobject,
			     enum rdma_remove_reason why)
{
	struct devx_umem *obj = uobject->object;
	u32 out[MLX5_ST_SZ_DW(general_obj_out_cmd_hdr)];
	int err;

	err = mlx5_cmd_exec(obj->mdev, obj->dinbox, obj->dinlen, out, sizeof(out));
	if (ib_is_destroy_retryable(err, why, uobject))
		return err;

	ib_umem_release(obj->umem);
	kfree(obj);
	return 0;
}

DECLARE_UVERBS_NAMED_METHOD(
	MLX5_IB_METHOD_DEVX_UMEM_REG,
	UVERBS_ATTR_IDR(MLX5_IB_ATTR_DEVX_UMEM_REG_HANDLE,
			MLX5_IB_OBJECT_DEVX_UMEM,
			UVERBS_ACCESS_NEW,
			UA_MANDATORY),
	UVERBS_ATTR_PTR_IN(MLX5_IB_ATTR_DEVX_UMEM_REG_ADDR,
			   UVERBS_ATTR_TYPE(u64),
			   UA_MANDATORY),
	UVERBS_ATTR_PTR_IN(MLX5_IB_ATTR_DEVX_UMEM_REG_LEN,
			   UVERBS_ATTR_TYPE(u64),
			   UA_MANDATORY),
	UVERBS_ATTR_FLAGS_IN(MLX5_IB_ATTR_DEVX_UMEM_REG_ACCESS,
			     enum ib_access_flags),
	UVERBS_ATTR_PTR_OUT(MLX5_IB_ATTR_DEVX_UMEM_REG_OUT_ID,
			    UVERBS_ATTR_TYPE(u32),
			    UA_MANDATORY));

DECLARE_UVERBS_NAMED_METHOD_DESTROY(
	MLX5_IB_METHOD_DEVX_UMEM_DEREG,
	UVERBS_ATTR_IDR(MLX5_IB_ATTR_DEVX_UMEM_DEREG_HANDLE,
			MLX5_IB_OBJECT_DEVX_UMEM,
			UVERBS_ACCESS_DESTROY,
			UA_MANDATORY));

DECLARE_UVERBS_NAMED_METHOD(
	MLX5_IB_METHOD_DEVX_QUERY_EQN,
	UVERBS_ATTR_PTR_IN(MLX5_IB_ATTR_DEVX_QUERY_EQN_USER_VEC,
			   UVERBS_ATTR_TYPE(u32),
			   UA_MANDATORY),
	UVERBS_ATTR_PTR_OUT(MLX5_IB_ATTR_DEVX_QUERY_EQN_DEV_EQN,
			    UVERBS_ATTR_TYPE(u32),
			    UA_MANDATORY));

DECLARE_UVERBS_NAMED_METHOD(
	MLX5_IB_METHOD_DEVX_QUERY_UAR,
	UVERBS_ATTR_PTR_IN(MLX5_IB_ATTR_DEVX_QUERY_UAR_USER_IDX,
			   UVERBS_ATTR_TYPE(u32),
			   UA_MANDATORY),
	UVERBS_ATTR_PTR_OUT(MLX5_IB_ATTR_DEVX_QUERY_UAR_DEV_IDX,
			    UVERBS_ATTR_TYPE(u32),
			    UA_MANDATORY));

DECLARE_UVERBS_NAMED_METHOD(
	MLX5_IB_METHOD_DEVX_OTHER,
	UVERBS_ATTR_PTR_IN(
		MLX5_IB_ATTR_DEVX_OTHER_CMD_IN,
		UVERBS_ATTR_MIN_SIZE(MLX5_ST_SZ_BYTES(general_obj_in_cmd_hdr)),
		UA_MANDATORY,
		UA_ALLOC_AND_COPY),
	UVERBS_ATTR_PTR_OUT(
		MLX5_IB_ATTR_DEVX_OTHER_CMD_OUT,
		UVERBS_ATTR_MIN_SIZE(MLX5_ST_SZ_BYTES(general_obj_out_cmd_hdr)),
		UA_MANDATORY));

DECLARE_UVERBS_NAMED_METHOD(
	MLX5_IB_METHOD_DEVX_OBJ_CREATE,
	UVERBS_ATTR_IDR(MLX5_IB_ATTR_DEVX_OBJ_CREATE_HANDLE,
			MLX5_IB_OBJECT_DEVX_OBJ,
			UVERBS_ACCESS_NEW,
			UA_MANDATORY),
	UVERBS_ATTR_PTR_IN(
		MLX5_IB_ATTR_DEVX_OBJ_CREATE_CMD_IN,
		UVERBS_ATTR_MIN_SIZE(MLX5_ST_SZ_BYTES(general_obj_in_cmd_hdr)),
		UA_MANDATORY,
		UA_ALLOC_AND_COPY),
	UVERBS_ATTR_PTR_OUT(
		MLX5_IB_ATTR_DEVX_OBJ_CREATE_CMD_OUT,
		UVERBS_ATTR_MIN_SIZE(MLX5_ST_SZ_BYTES(general_obj_out_cmd_hdr)),
		UA_MANDATORY));

DECLARE_UVERBS_NAMED_METHOD_DESTROY(
	MLX5_IB_METHOD_DEVX_OBJ_DESTROY,
	UVERBS_ATTR_IDR(MLX5_IB_ATTR_DEVX_OBJ_DESTROY_HANDLE,
			MLX5_IB_OBJECT_DEVX_OBJ,
			UVERBS_ACCESS_DESTROY,
			UA_MANDATORY));

DECLARE_UVERBS_NAMED_METHOD(
	MLX5_IB_METHOD_DEVX_OBJ_MODIFY,
	UVERBS_ATTR_IDR(MLX5_IB_ATTR_DEVX_OBJ_MODIFY_HANDLE,
			MLX5_IB_OBJECT_DEVX_OBJ,
			UVERBS_ACCESS_WRITE,
			UA_MANDATORY),
	UVERBS_ATTR_PTR_IN(
		MLX5_IB_ATTR_DEVX_OBJ_MODIFY_CMD_IN,
		UVERBS_ATTR_MIN_SIZE(MLX5_ST_SZ_BYTES(general_obj_in_cmd_hdr)),
		UA_MANDATORY,
		UA_ALLOC_AND_COPY),
	UVERBS_ATTR_PTR_OUT(
		MLX5_IB_ATTR_DEVX_OBJ_MODIFY_CMD_OUT,
		UVERBS_ATTR_MIN_SIZE(MLX5_ST_SZ_BYTES(general_obj_out_cmd_hdr)),
		UA_MANDATORY));

DECLARE_UVERBS_NAMED_METHOD(
	MLX5_IB_METHOD_DEVX_OBJ_QUERY,
	UVERBS_ATTR_IDR(MLX5_IB_ATTR_DEVX_OBJ_QUERY_HANDLE,
			MLX5_IB_OBJECT_DEVX_OBJ,
			UVERBS_ACCESS_READ,
			UA_MANDATORY),
	UVERBS_ATTR_PTR_IN(
		MLX5_IB_ATTR_DEVX_OBJ_QUERY_CMD_IN,
		UVERBS_ATTR_MIN_SIZE(MLX5_ST_SZ_BYTES(general_obj_in_cmd_hdr)),
		UA_MANDATORY,
		UA_ALLOC_AND_COPY),
	UVERBS_ATTR_PTR_OUT(
		MLX5_IB_ATTR_DEVX_OBJ_QUERY_CMD_OUT,
		UVERBS_ATTR_MIN_SIZE(MLX5_ST_SZ_BYTES(general_obj_out_cmd_hdr)),
		UA_MANDATORY));

DECLARE_UVERBS_GLOBAL_METHODS(MLX5_IB_OBJECT_DEVX,
			      &UVERBS_METHOD(MLX5_IB_METHOD_DEVX_OTHER),
			      &UVERBS_METHOD(MLX5_IB_METHOD_DEVX_QUERY_UAR),
			      &UVERBS_METHOD(MLX5_IB_METHOD_DEVX_QUERY_EQN));

DECLARE_UVERBS_NAMED_OBJECT(MLX5_IB_OBJECT_DEVX_OBJ,
			    UVERBS_TYPE_ALLOC_IDR(devx_obj_cleanup),
			    &UVERBS_METHOD(MLX5_IB_METHOD_DEVX_OBJ_CREATE),
			    &UVERBS_METHOD(MLX5_IB_METHOD_DEVX_OBJ_DESTROY),
			    &UVERBS_METHOD(MLX5_IB_METHOD_DEVX_OBJ_MODIFY),
			    &UVERBS_METHOD(MLX5_IB_METHOD_DEVX_OBJ_QUERY));

DECLARE_UVERBS_NAMED_OBJECT(MLX5_IB_OBJECT_DEVX_UMEM,
			    UVERBS_TYPE_ALLOC_IDR(devx_umem_cleanup),
			    &UVERBS_METHOD(MLX5_IB_METHOD_DEVX_UMEM_REG),
			    &UVERBS_METHOD(MLX5_IB_METHOD_DEVX_UMEM_DEREG));

DECLARE_UVERBS_OBJECT_TREE(devx_objects,
			   &UVERBS_OBJECT(MLX5_IB_OBJECT_DEVX),
			   &UVERBS_OBJECT(MLX5_IB_OBJECT_DEVX_OBJ),
			   &UVERBS_OBJECT(MLX5_IB_OBJECT_DEVX_UMEM));

const struct uverbs_object_tree_def *mlx5_ib_get_devx_tree(void)
{
	return &devx_objects;
}
