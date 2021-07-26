/*
 * net/core/devlink.c - Network physical/parent device Netlink interface
 *
 * Heavily inspired by net/wireless/
 * Copyright (c) 2016 Mellanox Technologies. All rights reserved.
 * Copyright (c) 2016 Jiri Pirko <jiri@mellanox.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/netdevice.h>
#include <rdma/ib_verbs.h>
#include <net/netlink.h>
#include <net/genetlink.h>
#include <net/rtnetlink.h>
#include <net/net_namespace.h>
#include <net/sock.h>
#include <net/devlink.h>
#define CREATE_TRACE_POINTS
#include <trace/events/devlink.h>

static struct devlink_dpipe_field devlink_dpipe_fields_ethernet[] = {
	{
		.name = "destination mac",
		.id = DEVLINK_DPIPE_FIELD_ETHERNET_DST_MAC,
		.bitwidth = 48,
	},
};

struct devlink_dpipe_header devlink_dpipe_header_ethernet = {
	.name = "ethernet",
	.id = DEVLINK_DPIPE_HEADER_ETHERNET,
	.fields = devlink_dpipe_fields_ethernet,
	.fields_count = ARRAY_SIZE(devlink_dpipe_fields_ethernet),
	.global = true,
};
EXPORT_SYMBOL(devlink_dpipe_header_ethernet);

static struct devlink_dpipe_field devlink_dpipe_fields_ipv4[] = {
	{
		.name = "destination ip",
		.id = DEVLINK_DPIPE_FIELD_IPV4_DST_IP,
		.bitwidth = 32,
	},
};

struct devlink_dpipe_header devlink_dpipe_header_ipv4 = {
	.name = "ipv4",
	.id = DEVLINK_DPIPE_HEADER_IPV4,
	.fields = devlink_dpipe_fields_ipv4,
	.fields_count = ARRAY_SIZE(devlink_dpipe_fields_ipv4),
	.global = true,
};
EXPORT_SYMBOL(devlink_dpipe_header_ipv4);

static struct devlink_dpipe_field devlink_dpipe_fields_ipv6[] = {
	{
		.name = "destination ip",
		.id = DEVLINK_DPIPE_FIELD_IPV6_DST_IP,
		.bitwidth = 128,
	},
};

struct devlink_dpipe_header devlink_dpipe_header_ipv6 = {
	.name = "ipv6",
	.id = DEVLINK_DPIPE_HEADER_IPV6,
	.fields = devlink_dpipe_fields_ipv6,
	.fields_count = ARRAY_SIZE(devlink_dpipe_fields_ipv6),
	.global = true,
};
EXPORT_SYMBOL(devlink_dpipe_header_ipv6);

EXPORT_TRACEPOINT_SYMBOL_GPL(devlink_hwmsg);

static LIST_HEAD(devlink_list);

/* devlink_mutex
 *
 * An overall lock guarding every operation coming from userspace.
 * It also guards devlink devices list and it is taken when
 * driver registers/unregisters it.
 */
static DEFINE_MUTEX(devlink_mutex);

static struct net *devlink_net(const struct devlink *devlink)
{
	return read_pnet(&devlink->_net);
}

static void devlink_net_set(struct devlink *devlink, struct net *net)
{
	write_pnet(&devlink->_net, net);
}

static struct devlink *devlink_get_from_attrs(struct net *net,
					      struct nlattr **attrs)
{
	struct devlink *devlink;
	char *busname;
	char *devname;

	if (!attrs[DEVLINK_ATTR_BUS_NAME] || !attrs[DEVLINK_ATTR_DEV_NAME])
		return ERR_PTR(-EINVAL);

	busname = nla_data(attrs[DEVLINK_ATTR_BUS_NAME]);
	devname = nla_data(attrs[DEVLINK_ATTR_DEV_NAME]);

	list_for_each_entry(devlink, &devlink_list, list) {
		if (strcmp(devlink->dev->bus->name, busname) == 0 &&
		    strcmp(dev_name(devlink->dev), devname) == 0 &&
		    net_eq(devlink_net(devlink), net))
			return devlink;
	}

	return ERR_PTR(-ENODEV);
}

static struct devlink *devlink_get_from_info(struct genl_info *info)
{
	return devlink_get_from_attrs(genl_info_net(info), info->attrs);
}

static struct devlink_port *devlink_port_get_by_index(struct devlink *devlink,
						      int port_index)
{
	struct devlink_port *devlink_port;

	list_for_each_entry(devlink_port, &devlink->port_list, list) {
		if (devlink_port->index == port_index)
			return devlink_port;
	}
	return NULL;
}

static bool devlink_port_index_exists(struct devlink *devlink, int port_index)
{
	return devlink_port_get_by_index(devlink, port_index);
}

static struct devlink_port *devlink_port_get_from_attrs(struct devlink *devlink,
							struct nlattr **attrs)
{
	if (attrs[DEVLINK_ATTR_PORT_INDEX]) {
		u32 port_index = nla_get_u32(attrs[DEVLINK_ATTR_PORT_INDEX]);
		struct devlink_port *devlink_port;

		devlink_port = devlink_port_get_by_index(devlink, port_index);
		if (!devlink_port)
			return ERR_PTR(-ENODEV);
		return devlink_port;
	}
	return ERR_PTR(-EINVAL);
}

static struct devlink_port *devlink_port_get_from_info(struct devlink *devlink,
						       struct genl_info *info)
{
	return devlink_port_get_from_attrs(devlink, info->attrs);
}

struct devlink_sb {
	struct list_head list;
	unsigned int index;
	u32 size;
	u16 ingress_pools_count;
	u16 egress_pools_count;
	u16 ingress_tc_count;
	u16 egress_tc_count;
};

static u16 devlink_sb_pool_count(struct devlink_sb *devlink_sb)
{
	return devlink_sb->ingress_pools_count + devlink_sb->egress_pools_count;
}

static struct devlink_sb *devlink_sb_get_by_index(struct devlink *devlink,
						  unsigned int sb_index)
{
	struct devlink_sb *devlink_sb;

	list_for_each_entry(devlink_sb, &devlink->sb_list, list) {
		if (devlink_sb->index == sb_index)
			return devlink_sb;
	}
	return NULL;
}

static bool devlink_sb_index_exists(struct devlink *devlink,
				    unsigned int sb_index)
{
	return devlink_sb_get_by_index(devlink, sb_index);
}

static struct devlink_sb *devlink_sb_get_from_attrs(struct devlink *devlink,
						    struct nlattr **attrs)
{
	if (attrs[DEVLINK_ATTR_SB_INDEX]) {
		u32 sb_index = nla_get_u32(attrs[DEVLINK_ATTR_SB_INDEX]);
		struct devlink_sb *devlink_sb;

		devlink_sb = devlink_sb_get_by_index(devlink, sb_index);
		if (!devlink_sb)
			return ERR_PTR(-ENODEV);
		return devlink_sb;
	}
	return ERR_PTR(-EINVAL);
}

static struct devlink_sb *devlink_sb_get_from_info(struct devlink *devlink,
						   struct genl_info *info)
{
	return devlink_sb_get_from_attrs(devlink, info->attrs);
}

static int devlink_sb_pool_index_get_from_attrs(struct devlink_sb *devlink_sb,
						struct nlattr **attrs,
						u16 *p_pool_index)
{
	u16 val;

	if (!attrs[DEVLINK_ATTR_SB_POOL_INDEX])
		return -EINVAL;

	val = nla_get_u16(attrs[DEVLINK_ATTR_SB_POOL_INDEX]);
	if (val >= devlink_sb_pool_count(devlink_sb))
		return -EINVAL;
	*p_pool_index = val;
	return 0;
}

static int devlink_sb_pool_index_get_from_info(struct devlink_sb *devlink_sb,
					       struct genl_info *info,
					       u16 *p_pool_index)
{
	return devlink_sb_pool_index_get_from_attrs(devlink_sb, info->attrs,
						    p_pool_index);
}

static int
devlink_sb_pool_type_get_from_attrs(struct nlattr **attrs,
				    enum devlink_sb_pool_type *p_pool_type)
{
	u8 val;

	if (!attrs[DEVLINK_ATTR_SB_POOL_TYPE])
		return -EINVAL;

	val = nla_get_u8(attrs[DEVLINK_ATTR_SB_POOL_TYPE]);
	if (val != DEVLINK_SB_POOL_TYPE_INGRESS &&
	    val != DEVLINK_SB_POOL_TYPE_EGRESS)
		return -EINVAL;
	*p_pool_type = val;
	return 0;
}

static int
devlink_sb_pool_type_get_from_info(struct genl_info *info,
				   enum devlink_sb_pool_type *p_pool_type)
{
	return devlink_sb_pool_type_get_from_attrs(info->attrs, p_pool_type);
}

static int
devlink_sb_th_type_get_from_attrs(struct nlattr **attrs,
				  enum devlink_sb_threshold_type *p_th_type)
{
	u8 val;

	if (!attrs[DEVLINK_ATTR_SB_POOL_THRESHOLD_TYPE])
		return -EINVAL;

	val = nla_get_u8(attrs[DEVLINK_ATTR_SB_POOL_THRESHOLD_TYPE]);
	if (val != DEVLINK_SB_THRESHOLD_TYPE_STATIC &&
	    val != DEVLINK_SB_THRESHOLD_TYPE_DYNAMIC)
		return -EINVAL;
	*p_th_type = val;
	return 0;
}

static int
devlink_sb_th_type_get_from_info(struct genl_info *info,
				 enum devlink_sb_threshold_type *p_th_type)
{
	return devlink_sb_th_type_get_from_attrs(info->attrs, p_th_type);
}

static int
devlink_sb_tc_index_get_from_attrs(struct devlink_sb *devlink_sb,
				   struct nlattr **attrs,
				   enum devlink_sb_pool_type pool_type,
				   u16 *p_tc_index)
{
	u16 val;

	if (!attrs[DEVLINK_ATTR_SB_TC_INDEX])
		return -EINVAL;

	val = nla_get_u16(attrs[DEVLINK_ATTR_SB_TC_INDEX]);
	if (pool_type == DEVLINK_SB_POOL_TYPE_INGRESS &&
	    val >= devlink_sb->ingress_tc_count)
		return -EINVAL;
	if (pool_type == DEVLINK_SB_POOL_TYPE_EGRESS &&
	    val >= devlink_sb->egress_tc_count)
		return -EINVAL;
	*p_tc_index = val;
	return 0;
}

static int
devlink_sb_tc_index_get_from_info(struct devlink_sb *devlink_sb,
				  struct genl_info *info,
				  enum devlink_sb_pool_type pool_type,
				  u16 *p_tc_index)
{
	return devlink_sb_tc_index_get_from_attrs(devlink_sb, info->attrs,
						  pool_type, p_tc_index);
}

struct devlink_region {
	struct devlink *devlink;
	struct list_head list;
	const char *name;
	struct list_head snapshot_list;
	u32 max_snapshots;
	u32 cur_snapshots;
	u64 size;
};

struct devlink_snapshot {
	struct list_head list;
	struct devlink_region *region;
	devlink_snapshot_data_dest_t *data_destructor;
	u64 data_len;
	u8 *data;
	u32 id;
};

static struct devlink_region *
devlink_region_get_by_name(struct devlink *devlink, const char *region_name)
{
	struct devlink_region *region;

	list_for_each_entry(region, &devlink->region_list, list)
		if (!strcmp(region->name, region_name))
			return region;

	return NULL;
}

static struct devlink_snapshot *
devlink_region_snapshot_get_by_id(struct devlink_region *region, u32 id)
{
	struct devlink_snapshot *snapshot;

	list_for_each_entry(snapshot, &region->snapshot_list, list)
		if (snapshot->id == id)
			return snapshot;

	return NULL;
}

static void devlink_region_snapshot_del(struct devlink_snapshot *snapshot)
{
	snapshot->region->cur_snapshots--;
	list_del(&snapshot->list);
	(*snapshot->data_destructor)(snapshot->data);
	kfree(snapshot);
}

#define DEVLINK_NL_FLAG_NEED_DEVLINK	BIT(0)
#define DEVLINK_NL_FLAG_NEED_PORT	BIT(1)
#define DEVLINK_NL_FLAG_NEED_SB		BIT(2)

/* The per devlink instance lock is taken by default in the pre-doit
 * operation, yet several commands do not require this. The global
 * devlink lock is taken and protects from disruption by user-calls.
 */
#define DEVLINK_NL_FLAG_NO_LOCK		BIT(3)

static int devlink_nl_pre_doit(const struct genl_ops *ops,
			       struct sk_buff *skb, struct genl_info *info)
{
	struct devlink *devlink;
	int err;

	mutex_lock(&devlink_mutex);
	devlink = devlink_get_from_info(info);
	if (IS_ERR(devlink)) {
		mutex_unlock(&devlink_mutex);
		return PTR_ERR(devlink);
	}
	if (~ops->internal_flags & DEVLINK_NL_FLAG_NO_LOCK)
		mutex_lock(&devlink->lock);
	if (ops->internal_flags & DEVLINK_NL_FLAG_NEED_DEVLINK) {
		info->user_ptr[0] = devlink;
	} else if (ops->internal_flags & DEVLINK_NL_FLAG_NEED_PORT) {
		struct devlink_port *devlink_port;

		devlink_port = devlink_port_get_from_info(devlink, info);
		if (IS_ERR(devlink_port)) {
			err = PTR_ERR(devlink_port);
			goto unlock;
		}
		info->user_ptr[0] = devlink_port;
	}
	if (ops->internal_flags & DEVLINK_NL_FLAG_NEED_SB) {
		struct devlink_sb *devlink_sb;

		devlink_sb = devlink_sb_get_from_info(devlink, info);
		if (IS_ERR(devlink_sb)) {
			err = PTR_ERR(devlink_sb);
			goto unlock;
		}
		info->user_ptr[1] = devlink_sb;
	}
	return 0;

unlock:
	if (~ops->internal_flags & DEVLINK_NL_FLAG_NO_LOCK)
		mutex_unlock(&devlink->lock);
	mutex_unlock(&devlink_mutex);
	return err;
}

static void devlink_nl_post_doit(const struct genl_ops *ops,
				 struct sk_buff *skb, struct genl_info *info)
{
	struct devlink *devlink;

	devlink = devlink_get_from_info(info);
	if (~ops->internal_flags & DEVLINK_NL_FLAG_NO_LOCK)
		mutex_unlock(&devlink->lock);
	mutex_unlock(&devlink_mutex);
}

static struct genl_family devlink_nl_family;

enum devlink_multicast_groups {
	DEVLINK_MCGRP_CONFIG,
};

static const struct genl_multicast_group devlink_nl_mcgrps[] = {
	[DEVLINK_MCGRP_CONFIG] = { .name = DEVLINK_GENL_MCGRP_CONFIG_NAME },
};

static int devlink_nl_put_handle(struct sk_buff *msg, struct devlink *devlink)
{
	if (nla_put_string(msg, DEVLINK_ATTR_BUS_NAME, devlink->dev->bus->name))
		return -EMSGSIZE;
	if (nla_put_string(msg, DEVLINK_ATTR_DEV_NAME, dev_name(devlink->dev)))
		return -EMSGSIZE;
	return 0;
}

static int devlink_nl_fill(struct sk_buff *msg, struct devlink *devlink,
			   enum devlink_command cmd, u32 portid,
			   u32 seq, int flags)
{
	void *hdr;

	hdr = genlmsg_put(msg, portid, seq, &devlink_nl_family, flags, cmd);
	if (!hdr)
		return -EMSGSIZE;

	if (devlink_nl_put_handle(msg, devlink))
		goto nla_put_failure;

	genlmsg_end(msg, hdr);
	return 0;

nla_put_failure:
	genlmsg_cancel(msg, hdr);
	return -EMSGSIZE;
}

static void devlink_notify(struct devlink *devlink, enum devlink_command cmd)
{
	struct sk_buff *msg;
	int err;

	WARN_ON(cmd != DEVLINK_CMD_NEW && cmd != DEVLINK_CMD_DEL);

	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!msg)
		return;

	err = devlink_nl_fill(msg, devlink, cmd, 0, 0, 0);
	if (err) {
		nlmsg_free(msg);
		return;
	}

	genlmsg_multicast_netns(&devlink_nl_family, devlink_net(devlink),
				msg, 0, DEVLINK_MCGRP_CONFIG, GFP_KERNEL);
}

static int devlink_nl_port_attrs_put(struct sk_buff *msg,
				     struct devlink_port *devlink_port)
{
	struct devlink_port_attrs *attrs = &devlink_port->attrs;

	if (!attrs->set)
		return 0;
	if (nla_put_u16(msg, DEVLINK_ATTR_PORT_FLAVOUR, attrs->flavour))
		return -EMSGSIZE;
	if (nla_put_u32(msg, DEVLINK_ATTR_PORT_NUMBER, attrs->port_number))
		return -EMSGSIZE;
	if (!attrs->split)
		return 0;
	if (nla_put_u32(msg, DEVLINK_ATTR_PORT_SPLIT_GROUP, attrs->port_number))
		return -EMSGSIZE;
	if (nla_put_u32(msg, DEVLINK_ATTR_PORT_SPLIT_SUBPORT_NUMBER,
			attrs->split_subport_number))
		return -EMSGSIZE;
	return 0;
}

static int devlink_nl_port_fill(struct sk_buff *msg, struct devlink *devlink,
				struct devlink_port *devlink_port,
				enum devlink_command cmd, u32 portid,
				u32 seq, int flags)
{
	void *hdr;

	hdr = genlmsg_put(msg, portid, seq, &devlink_nl_family, flags, cmd);
	if (!hdr)
		return -EMSGSIZE;

	if (devlink_nl_put_handle(msg, devlink))
		goto nla_put_failure;
	if (nla_put_u32(msg, DEVLINK_ATTR_PORT_INDEX, devlink_port->index))
		goto nla_put_failure;
	if (nla_put_u16(msg, DEVLINK_ATTR_PORT_TYPE, devlink_port->type))
		goto nla_put_failure;
	if (devlink_port->desired_type != DEVLINK_PORT_TYPE_NOTSET &&
	    nla_put_u16(msg, DEVLINK_ATTR_PORT_DESIRED_TYPE,
			devlink_port->desired_type))
		goto nla_put_failure;
	if (devlink_port->type == DEVLINK_PORT_TYPE_ETH) {
		struct net_device *netdev = devlink_port->type_dev;

		if (netdev &&
		    (nla_put_u32(msg, DEVLINK_ATTR_PORT_NETDEV_IFINDEX,
				 netdev->ifindex) ||
		     nla_put_string(msg, DEVLINK_ATTR_PORT_NETDEV_NAME,
				    netdev->name)))
			goto nla_put_failure;
	}
	if (devlink_port->type == DEVLINK_PORT_TYPE_IB) {
		struct ib_device *ibdev = devlink_port->type_dev;

		if (ibdev &&
		    nla_put_string(msg, DEVLINK_ATTR_PORT_IBDEV_NAME,
				   ibdev->name))
			goto nla_put_failure;
	}
	if (devlink_nl_port_attrs_put(msg, devlink_port))
		goto nla_put_failure;

	genlmsg_end(msg, hdr);
	return 0;

nla_put_failure:
	genlmsg_cancel(msg, hdr);
	return -EMSGSIZE;
}

static void devlink_port_notify(struct devlink_port *devlink_port,
				enum devlink_command cmd)
{
	struct devlink *devlink = devlink_port->devlink;
	struct sk_buff *msg;
	int err;

	if (!devlink_port->registered)
		return;

	WARN_ON(cmd != DEVLINK_CMD_PORT_NEW && cmd != DEVLINK_CMD_PORT_DEL);

	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!msg)
		return;

	err = devlink_nl_port_fill(msg, devlink, devlink_port, cmd, 0, 0, 0);
	if (err) {
		nlmsg_free(msg);
		return;
	}

	genlmsg_multicast_netns(&devlink_nl_family, devlink_net(devlink),
				msg, 0, DEVLINK_MCGRP_CONFIG, GFP_KERNEL);
}

static int devlink_nl_cmd_get_doit(struct sk_buff *skb, struct genl_info *info)
{
	struct devlink *devlink = info->user_ptr[0];
	struct sk_buff *msg;
	int err;

	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!msg)
		return -ENOMEM;

	err = devlink_nl_fill(msg, devlink, DEVLINK_CMD_NEW,
			      info->snd_portid, info->snd_seq, 0);
	if (err) {
		nlmsg_free(msg);
		return err;
	}

	return genlmsg_reply(msg, info);
}

static int devlink_nl_cmd_get_dumpit(struct sk_buff *msg,
				     struct netlink_callback *cb)
{
	struct devlink *devlink;
	int start = cb->args[0];
	int idx = 0;
	int err;

	mutex_lock(&devlink_mutex);
	list_for_each_entry(devlink, &devlink_list, list) {
		if (!net_eq(devlink_net(devlink), sock_net(msg->sk)))
			continue;
		if (idx < start) {
			idx++;
			continue;
		}
		err = devlink_nl_fill(msg, devlink, DEVLINK_CMD_NEW,
				      NETLINK_CB(cb->skb).portid,
				      cb->nlh->nlmsg_seq, NLM_F_MULTI);
		if (err)
			goto out;
		idx++;
	}
out:
	mutex_unlock(&devlink_mutex);

	cb->args[0] = idx;
	return msg->len;
}

static int devlink_nl_cmd_port_get_doit(struct sk_buff *skb,
					struct genl_info *info)
{
	struct devlink_port *devlink_port = info->user_ptr[0];
	struct devlink *devlink = devlink_port->devlink;
	struct sk_buff *msg;
	int err;

	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!msg)
		return -ENOMEM;

	err = devlink_nl_port_fill(msg, devlink, devlink_port,
				   DEVLINK_CMD_PORT_NEW,
				   info->snd_portid, info->snd_seq, 0);
	if (err) {
		nlmsg_free(msg);
		return err;
	}

	return genlmsg_reply(msg, info);
}

static int devlink_nl_cmd_port_get_dumpit(struct sk_buff *msg,
					  struct netlink_callback *cb)
{
	struct devlink *devlink;
	struct devlink_port *devlink_port;
	int start = cb->args[0];
	int idx = 0;
	int err;

	mutex_lock(&devlink_mutex);
	list_for_each_entry(devlink, &devlink_list, list) {
		if (!net_eq(devlink_net(devlink), sock_net(msg->sk)))
			continue;
		mutex_lock(&devlink->lock);
		list_for_each_entry(devlink_port, &devlink->port_list, list) {
			if (idx < start) {
				idx++;
				continue;
			}
			err = devlink_nl_port_fill(msg, devlink, devlink_port,
						   DEVLINK_CMD_NEW,
						   NETLINK_CB(cb->skb).portid,
						   cb->nlh->nlmsg_seq,
						   NLM_F_MULTI);
			if (err) {
				mutex_unlock(&devlink->lock);
				goto out;
			}
			idx++;
		}
		mutex_unlock(&devlink->lock);
	}
out:
	mutex_unlock(&devlink_mutex);

	cb->args[0] = idx;
	return msg->len;
}

static int devlink_port_type_set(struct devlink *devlink,
				 struct devlink_port *devlink_port,
				 enum devlink_port_type port_type)

{
	int err;

	if (devlink->ops && devlink->ops->port_type_set) {
		if (port_type == DEVLINK_PORT_TYPE_NOTSET)
			return -EINVAL;
		if (port_type == devlink_port->type)
			return 0;
		err = devlink->ops->port_type_set(devlink_port, port_type);
		if (err)
			return err;
		devlink_port->desired_type = port_type;
		devlink_port_notify(devlink_port, DEVLINK_CMD_PORT_NEW);
		return 0;
	}
	return -EOPNOTSUPP;
}

static int devlink_nl_cmd_port_set_doit(struct sk_buff *skb,
					struct genl_info *info)
{
	struct devlink_port *devlink_port = info->user_ptr[0];
	struct devlink *devlink = devlink_port->devlink;
	int err;

	if (info->attrs[DEVLINK_ATTR_PORT_TYPE]) {
		enum devlink_port_type port_type;

		port_type = nla_get_u16(info->attrs[DEVLINK_ATTR_PORT_TYPE]);
		err = devlink_port_type_set(devlink, devlink_port, port_type);
		if (err)
			return err;
	}
	return 0;
}

static int devlink_port_split(struct devlink *devlink, u32 port_index,
			      u32 count, struct netlink_ext_ack *extack)

{
	if (devlink->ops && devlink->ops->port_split)
		return devlink->ops->port_split(devlink, port_index, count,
						extack);
	return -EOPNOTSUPP;
}

static int devlink_nl_cmd_port_split_doit(struct sk_buff *skb,
					  struct genl_info *info)
{
	struct devlink *devlink = info->user_ptr[0];
	u32 port_index;
	u32 count;

	if (!info->attrs[DEVLINK_ATTR_PORT_INDEX] ||
	    !info->attrs[DEVLINK_ATTR_PORT_SPLIT_COUNT])
		return -EINVAL;

	port_index = nla_get_u32(info->attrs[DEVLINK_ATTR_PORT_INDEX]);
	count = nla_get_u32(info->attrs[DEVLINK_ATTR_PORT_SPLIT_COUNT]);
	return devlink_port_split(devlink, port_index, count, info->extack);
}

static int devlink_port_unsplit(struct devlink *devlink, u32 port_index,
				struct netlink_ext_ack *extack)

{
	if (devlink->ops && devlink->ops->port_unsplit)
		return devlink->ops->port_unsplit(devlink, port_index, extack);
	return -EOPNOTSUPP;
}

static int devlink_nl_cmd_port_unsplit_doit(struct sk_buff *skb,
					    struct genl_info *info)
{
	struct devlink *devlink = info->user_ptr[0];
	u32 port_index;

	if (!info->attrs[DEVLINK_ATTR_PORT_INDEX])
		return -EINVAL;

	port_index = nla_get_u32(info->attrs[DEVLINK_ATTR_PORT_INDEX]);
	return devlink_port_unsplit(devlink, port_index, info->extack);
}

static int devlink_nl_sb_fill(struct sk_buff *msg, struct devlink *devlink,
			      struct devlink_sb *devlink_sb,
			      enum devlink_command cmd, u32 portid,
			      u32 seq, int flags)
{
	void *hdr;

	hdr = genlmsg_put(msg, portid, seq, &devlink_nl_family, flags, cmd);
	if (!hdr)
		return -EMSGSIZE;

	if (devlink_nl_put_handle(msg, devlink))
		goto nla_put_failure;
	if (nla_put_u32(msg, DEVLINK_ATTR_SB_INDEX, devlink_sb->index))
		goto nla_put_failure;
	if (nla_put_u32(msg, DEVLINK_ATTR_SB_SIZE, devlink_sb->size))
		goto nla_put_failure;
	if (nla_put_u16(msg, DEVLINK_ATTR_SB_INGRESS_POOL_COUNT,
			devlink_sb->ingress_pools_count))
		goto nla_put_failure;
	if (nla_put_u16(msg, DEVLINK_ATTR_SB_EGRESS_POOL_COUNT,
			devlink_sb->egress_pools_count))
		goto nla_put_failure;
	if (nla_put_u16(msg, DEVLINK_ATTR_SB_INGRESS_TC_COUNT,
			devlink_sb->ingress_tc_count))
		goto nla_put_failure;
	if (nla_put_u16(msg, DEVLINK_ATTR_SB_EGRESS_TC_COUNT,
			devlink_sb->egress_tc_count))
		goto nla_put_failure;

	genlmsg_end(msg, hdr);
	return 0;

nla_put_failure:
	genlmsg_cancel(msg, hdr);
	return -EMSGSIZE;
}

static int devlink_nl_cmd_sb_get_doit(struct sk_buff *skb,
				      struct genl_info *info)
{
	struct devlink *devlink = info->user_ptr[0];
	struct devlink_sb *devlink_sb = info->user_ptr[1];
	struct sk_buff *msg;
	int err;

	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!msg)
		return -ENOMEM;

	err = devlink_nl_sb_fill(msg, devlink, devlink_sb,
				 DEVLINK_CMD_SB_NEW,
				 info->snd_portid, info->snd_seq, 0);
	if (err) {
		nlmsg_free(msg);
		return err;
	}

	return genlmsg_reply(msg, info);
}

static int devlink_nl_cmd_sb_get_dumpit(struct sk_buff *msg,
					struct netlink_callback *cb)
{
	struct devlink *devlink;
	struct devlink_sb *devlink_sb;
	int start = cb->args[0];
	int idx = 0;
	int err;

	mutex_lock(&devlink_mutex);
	list_for_each_entry(devlink, &devlink_list, list) {
		if (!net_eq(devlink_net(devlink), sock_net(msg->sk)))
			continue;
		mutex_lock(&devlink->lock);
		list_for_each_entry(devlink_sb, &devlink->sb_list, list) {
			if (idx < start) {
				idx++;
				continue;
			}
			err = devlink_nl_sb_fill(msg, devlink, devlink_sb,
						 DEVLINK_CMD_SB_NEW,
						 NETLINK_CB(cb->skb).portid,
						 cb->nlh->nlmsg_seq,
						 NLM_F_MULTI);
			if (err) {
				mutex_unlock(&devlink->lock);
				goto out;
			}
			idx++;
		}
		mutex_unlock(&devlink->lock);
	}
out:
	mutex_unlock(&devlink_mutex);

	cb->args[0] = idx;
	return msg->len;
}

static int devlink_nl_sb_pool_fill(struct sk_buff *msg, struct devlink *devlink,
				   struct devlink_sb *devlink_sb,
				   u16 pool_index, enum devlink_command cmd,
				   u32 portid, u32 seq, int flags)
{
	struct devlink_sb_pool_info pool_info;
	void *hdr;
	int err;

	err = devlink->ops->sb_pool_get(devlink, devlink_sb->index,
					pool_index, &pool_info);
	if (err)
		return err;

	hdr = genlmsg_put(msg, portid, seq, &devlink_nl_family, flags, cmd);
	if (!hdr)
		return -EMSGSIZE;

	if (devlink_nl_put_handle(msg, devlink))
		goto nla_put_failure;
	if (nla_put_u32(msg, DEVLINK_ATTR_SB_INDEX, devlink_sb->index))
		goto nla_put_failure;
	if (nla_put_u16(msg, DEVLINK_ATTR_SB_POOL_INDEX, pool_index))
		goto nla_put_failure;
	if (nla_put_u8(msg, DEVLINK_ATTR_SB_POOL_TYPE, pool_info.pool_type))
		goto nla_put_failure;
	if (nla_put_u32(msg, DEVLINK_ATTR_SB_POOL_SIZE, pool_info.size))
		goto nla_put_failure;
	if (nla_put_u8(msg, DEVLINK_ATTR_SB_POOL_THRESHOLD_TYPE,
		       pool_info.threshold_type))
		goto nla_put_failure;

	genlmsg_end(msg, hdr);
	return 0;

nla_put_failure:
	genlmsg_cancel(msg, hdr);
	return -EMSGSIZE;
}

static int devlink_nl_cmd_sb_pool_get_doit(struct sk_buff *skb,
					   struct genl_info *info)
{
	struct devlink *devlink = info->user_ptr[0];
	struct devlink_sb *devlink_sb = info->user_ptr[1];
	struct sk_buff *msg;
	u16 pool_index;
	int err;

	err = devlink_sb_pool_index_get_from_info(devlink_sb, info,
						  &pool_index);
	if (err)
		return err;

	if (!devlink->ops || !devlink->ops->sb_pool_get)
		return -EOPNOTSUPP;

	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!msg)
		return -ENOMEM;

	err = devlink_nl_sb_pool_fill(msg, devlink, devlink_sb, pool_index,
				      DEVLINK_CMD_SB_POOL_NEW,
				      info->snd_portid, info->snd_seq, 0);
	if (err) {
		nlmsg_free(msg);
		return err;
	}

	return genlmsg_reply(msg, info);
}

static int __sb_pool_get_dumpit(struct sk_buff *msg, int start, int *p_idx,
				struct devlink *devlink,
				struct devlink_sb *devlink_sb,
				u32 portid, u32 seq)
{
	u16 pool_count = devlink_sb_pool_count(devlink_sb);
	u16 pool_index;
	int err;

	for (pool_index = 0; pool_index < pool_count; pool_index++) {
		if (*p_idx < start) {
			(*p_idx)++;
			continue;
		}
		err = devlink_nl_sb_pool_fill(msg, devlink,
					      devlink_sb,
					      pool_index,
					      DEVLINK_CMD_SB_POOL_NEW,
					      portid, seq, NLM_F_MULTI);
		if (err)
			return err;
		(*p_idx)++;
	}
	return 0;
}

static int devlink_nl_cmd_sb_pool_get_dumpit(struct sk_buff *msg,
					     struct netlink_callback *cb)
{
	struct devlink *devlink;
	struct devlink_sb *devlink_sb;
	int start = cb->args[0];
	int idx = 0;
	int err;

	mutex_lock(&devlink_mutex);
	list_for_each_entry(devlink, &devlink_list, list) {
		if (!net_eq(devlink_net(devlink), sock_net(msg->sk)) ||
		    !devlink->ops || !devlink->ops->sb_pool_get)
			continue;
		mutex_lock(&devlink->lock);
		list_for_each_entry(devlink_sb, &devlink->sb_list, list) {
			err = __sb_pool_get_dumpit(msg, start, &idx, devlink,
						   devlink_sb,
						   NETLINK_CB(cb->skb).portid,
						   cb->nlh->nlmsg_seq);
			if (err && err != -EOPNOTSUPP) {
				mutex_unlock(&devlink->lock);
				goto out;
			}
		}
		mutex_unlock(&devlink->lock);
	}
out:
	mutex_unlock(&devlink_mutex);

	cb->args[0] = idx;
	return msg->len;
}

static int devlink_sb_pool_set(struct devlink *devlink, unsigned int sb_index,
			       u16 pool_index, u32 size,
			       enum devlink_sb_threshold_type threshold_type)

{
	const struct devlink_ops *ops = devlink->ops;

	if (ops && ops->sb_pool_set)
		return ops->sb_pool_set(devlink, sb_index, pool_index,
					size, threshold_type);
	return -EOPNOTSUPP;
}

static int devlink_nl_cmd_sb_pool_set_doit(struct sk_buff *skb,
					   struct genl_info *info)
{
	struct devlink *devlink = info->user_ptr[0];
	struct devlink_sb *devlink_sb = info->user_ptr[1];
	enum devlink_sb_threshold_type threshold_type;
	u16 pool_index;
	u32 size;
	int err;

	err = devlink_sb_pool_index_get_from_info(devlink_sb, info,
						  &pool_index);
	if (err)
		return err;

	err = devlink_sb_th_type_get_from_info(info, &threshold_type);
	if (err)
		return err;

	if (!info->attrs[DEVLINK_ATTR_SB_POOL_SIZE])
		return -EINVAL;

	size = nla_get_u32(info->attrs[DEVLINK_ATTR_SB_POOL_SIZE]);
	return devlink_sb_pool_set(devlink, devlink_sb->index,
				   pool_index, size, threshold_type);
}

static int devlink_nl_sb_port_pool_fill(struct sk_buff *msg,
					struct devlink *devlink,
					struct devlink_port *devlink_port,
					struct devlink_sb *devlink_sb,
					u16 pool_index,
					enum devlink_command cmd,
					u32 portid, u32 seq, int flags)
{
	const struct devlink_ops *ops = devlink->ops;
	u32 threshold;
	void *hdr;
	int err;

	err = ops->sb_port_pool_get(devlink_port, devlink_sb->index,
				    pool_index, &threshold);
	if (err)
		return err;

	hdr = genlmsg_put(msg, portid, seq, &devlink_nl_family, flags, cmd);
	if (!hdr)
		return -EMSGSIZE;

	if (devlink_nl_put_handle(msg, devlink))
		goto nla_put_failure;
	if (nla_put_u32(msg, DEVLINK_ATTR_PORT_INDEX, devlink_port->index))
		goto nla_put_failure;
	if (nla_put_u32(msg, DEVLINK_ATTR_SB_INDEX, devlink_sb->index))
		goto nla_put_failure;
	if (nla_put_u16(msg, DEVLINK_ATTR_SB_POOL_INDEX, pool_index))
		goto nla_put_failure;
	if (nla_put_u32(msg, DEVLINK_ATTR_SB_THRESHOLD, threshold))
		goto nla_put_failure;

	if (ops->sb_occ_port_pool_get) {
		u32 cur;
		u32 max;

		err = ops->sb_occ_port_pool_get(devlink_port, devlink_sb->index,
						pool_index, &cur, &max);
		if (err && err != -EOPNOTSUPP)
			goto sb_occ_get_failure;
		if (!err) {
			if (nla_put_u32(msg, DEVLINK_ATTR_SB_OCC_CUR, cur))
				goto nla_put_failure;
			if (nla_put_u32(msg, DEVLINK_ATTR_SB_OCC_MAX, max))
				goto nla_put_failure;
		}
	}

	genlmsg_end(msg, hdr);
	return 0;

nla_put_failure:
	err = -EMSGSIZE;
sb_occ_get_failure:
	genlmsg_cancel(msg, hdr);
	return err;
}

static int devlink_nl_cmd_sb_port_pool_get_doit(struct sk_buff *skb,
						struct genl_info *info)
{
	struct devlink_port *devlink_port = info->user_ptr[0];
	struct devlink *devlink = devlink_port->devlink;
	struct devlink_sb *devlink_sb = info->user_ptr[1];
	struct sk_buff *msg;
	u16 pool_index;
	int err;

	err = devlink_sb_pool_index_get_from_info(devlink_sb, info,
						  &pool_index);
	if (err)
		return err;

	if (!devlink->ops || !devlink->ops->sb_port_pool_get)
		return -EOPNOTSUPP;

	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!msg)
		return -ENOMEM;

	err = devlink_nl_sb_port_pool_fill(msg, devlink, devlink_port,
					   devlink_sb, pool_index,
					   DEVLINK_CMD_SB_PORT_POOL_NEW,
					   info->snd_portid, info->snd_seq, 0);
	if (err) {
		nlmsg_free(msg);
		return err;
	}

	return genlmsg_reply(msg, info);
}

static int __sb_port_pool_get_dumpit(struct sk_buff *msg, int start, int *p_idx,
				     struct devlink *devlink,
				     struct devlink_sb *devlink_sb,
				     u32 portid, u32 seq)
{
	struct devlink_port *devlink_port;
	u16 pool_count = devlink_sb_pool_count(devlink_sb);
	u16 pool_index;
	int err;

	list_for_each_entry(devlink_port, &devlink->port_list, list) {
		for (pool_index = 0; pool_index < pool_count; pool_index++) {
			if (*p_idx < start) {
				(*p_idx)++;
				continue;
			}
			err = devlink_nl_sb_port_pool_fill(msg, devlink,
							   devlink_port,
							   devlink_sb,
							   pool_index,
							   DEVLINK_CMD_SB_PORT_POOL_NEW,
							   portid, seq,
							   NLM_F_MULTI);
			if (err)
				return err;
			(*p_idx)++;
		}
	}
	return 0;
}

static int devlink_nl_cmd_sb_port_pool_get_dumpit(struct sk_buff *msg,
						  struct netlink_callback *cb)
{
	struct devlink *devlink;
	struct devlink_sb *devlink_sb;
	int start = cb->args[0];
	int idx = 0;
	int err;

	mutex_lock(&devlink_mutex);
	list_for_each_entry(devlink, &devlink_list, list) {
		if (!net_eq(devlink_net(devlink), sock_net(msg->sk)) ||
		    !devlink->ops || !devlink->ops->sb_port_pool_get)
			continue;
		mutex_lock(&devlink->lock);
		list_for_each_entry(devlink_sb, &devlink->sb_list, list) {
			err = __sb_port_pool_get_dumpit(msg, start, &idx,
							devlink, devlink_sb,
							NETLINK_CB(cb->skb).portid,
							cb->nlh->nlmsg_seq);
			if (err && err != -EOPNOTSUPP) {
				mutex_unlock(&devlink->lock);
				goto out;
			}
		}
		mutex_unlock(&devlink->lock);
	}
out:
	mutex_unlock(&devlink_mutex);

	cb->args[0] = idx;
	return msg->len;
}

static int devlink_sb_port_pool_set(struct devlink_port *devlink_port,
				    unsigned int sb_index, u16 pool_index,
				    u32 threshold)

{
	const struct devlink_ops *ops = devlink_port->devlink->ops;

	if (ops && ops->sb_port_pool_set)
		return ops->sb_port_pool_set(devlink_port, sb_index,
					     pool_index, threshold);
	return -EOPNOTSUPP;
}

static int devlink_nl_cmd_sb_port_pool_set_doit(struct sk_buff *skb,
						struct genl_info *info)
{
	struct devlink_port *devlink_port = info->user_ptr[0];
	struct devlink_sb *devlink_sb = info->user_ptr[1];
	u16 pool_index;
	u32 threshold;
	int err;

	err = devlink_sb_pool_index_get_from_info(devlink_sb, info,
						  &pool_index);
	if (err)
		return err;

	if (!info->attrs[DEVLINK_ATTR_SB_THRESHOLD])
		return -EINVAL;

	threshold = nla_get_u32(info->attrs[DEVLINK_ATTR_SB_THRESHOLD]);
	return devlink_sb_port_pool_set(devlink_port, devlink_sb->index,
					pool_index, threshold);
}

static int
devlink_nl_sb_tc_pool_bind_fill(struct sk_buff *msg, struct devlink *devlink,
				struct devlink_port *devlink_port,
				struct devlink_sb *devlink_sb, u16 tc_index,
				enum devlink_sb_pool_type pool_type,
				enum devlink_command cmd,
				u32 portid, u32 seq, int flags)
{
	const struct devlink_ops *ops = devlink->ops;
	u16 pool_index;
	u32 threshold;
	void *hdr;
	int err;

	err = ops->sb_tc_pool_bind_get(devlink_port, devlink_sb->index,
				       tc_index, pool_type,
				       &pool_index, &threshold);
	if (err)
		return err;

	hdr = genlmsg_put(msg, portid, seq, &devlink_nl_family, flags, cmd);
	if (!hdr)
		return -EMSGSIZE;

	if (devlink_nl_put_handle(msg, devlink))
		goto nla_put_failure;
	if (nla_put_u32(msg, DEVLINK_ATTR_PORT_INDEX, devlink_port->index))
		goto nla_put_failure;
	if (nla_put_u32(msg, DEVLINK_ATTR_SB_INDEX, devlink_sb->index))
		goto nla_put_failure;
	if (nla_put_u16(msg, DEVLINK_ATTR_SB_TC_INDEX, tc_index))
		goto nla_put_failure;
	if (nla_put_u8(msg, DEVLINK_ATTR_SB_POOL_TYPE, pool_type))
		goto nla_put_failure;
	if (nla_put_u16(msg, DEVLINK_ATTR_SB_POOL_INDEX, pool_index))
		goto nla_put_failure;
	if (nla_put_u32(msg, DEVLINK_ATTR_SB_THRESHOLD, threshold))
		goto nla_put_failure;

	if (ops->sb_occ_tc_port_bind_get) {
		u32 cur;
		u32 max;

		err = ops->sb_occ_tc_port_bind_get(devlink_port,
						   devlink_sb->index,
						   tc_index, pool_type,
						   &cur, &max);
		if (err && err != -EOPNOTSUPP)
			return err;
		if (!err) {
			if (nla_put_u32(msg, DEVLINK_ATTR_SB_OCC_CUR, cur))
				goto nla_put_failure;
			if (nla_put_u32(msg, DEVLINK_ATTR_SB_OCC_MAX, max))
				goto nla_put_failure;
		}
	}

	genlmsg_end(msg, hdr);
	return 0;

nla_put_failure:
	genlmsg_cancel(msg, hdr);
	return -EMSGSIZE;
}

static int devlink_nl_cmd_sb_tc_pool_bind_get_doit(struct sk_buff *skb,
						   struct genl_info *info)
{
	struct devlink_port *devlink_port = info->user_ptr[0];
	struct devlink *devlink = devlink_port->devlink;
	struct devlink_sb *devlink_sb = info->user_ptr[1];
	struct sk_buff *msg;
	enum devlink_sb_pool_type pool_type;
	u16 tc_index;
	int err;

	err = devlink_sb_pool_type_get_from_info(info, &pool_type);
	if (err)
		return err;

	err = devlink_sb_tc_index_get_from_info(devlink_sb, info,
						pool_type, &tc_index);
	if (err)
		return err;

	if (!devlink->ops || !devlink->ops->sb_tc_pool_bind_get)
		return -EOPNOTSUPP;

	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!msg)
		return -ENOMEM;

	err = devlink_nl_sb_tc_pool_bind_fill(msg, devlink, devlink_port,
					      devlink_sb, tc_index, pool_type,
					      DEVLINK_CMD_SB_TC_POOL_BIND_NEW,
					      info->snd_portid,
					      info->snd_seq, 0);
	if (err) {
		nlmsg_free(msg);
		return err;
	}

	return genlmsg_reply(msg, info);
}

static int __sb_tc_pool_bind_get_dumpit(struct sk_buff *msg,
					int start, int *p_idx,
					struct devlink *devlink,
					struct devlink_sb *devlink_sb,
					u32 portid, u32 seq)
{
	struct devlink_port *devlink_port;
	u16 tc_index;
	int err;

	list_for_each_entry(devlink_port, &devlink->port_list, list) {
		for (tc_index = 0;
		     tc_index < devlink_sb->ingress_tc_count; tc_index++) {
			if (*p_idx < start) {
				(*p_idx)++;
				continue;
			}
			err = devlink_nl_sb_tc_pool_bind_fill(msg, devlink,
							      devlink_port,
							      devlink_sb,
							      tc_index,
							      DEVLINK_SB_POOL_TYPE_INGRESS,
							      DEVLINK_CMD_SB_TC_POOL_BIND_NEW,
							      portid, seq,
							      NLM_F_MULTI);
			if (err)
				return err;
			(*p_idx)++;
		}
		for (tc_index = 0;
		     tc_index < devlink_sb->egress_tc_count; tc_index++) {
			if (*p_idx < start) {
				(*p_idx)++;
				continue;
			}
			err = devlink_nl_sb_tc_pool_bind_fill(msg, devlink,
							      devlink_port,
							      devlink_sb,
							      tc_index,
							      DEVLINK_SB_POOL_TYPE_EGRESS,
							      DEVLINK_CMD_SB_TC_POOL_BIND_NEW,
							      portid, seq,
							      NLM_F_MULTI);
			if (err)
				return err;
			(*p_idx)++;
		}
	}
	return 0;
}

static int
devlink_nl_cmd_sb_tc_pool_bind_get_dumpit(struct sk_buff *msg,
					  struct netlink_callback *cb)
{
	struct devlink *devlink;
	struct devlink_sb *devlink_sb;
	int start = cb->args[0];
	int idx = 0;
	int err;

	mutex_lock(&devlink_mutex);
	list_for_each_entry(devlink, &devlink_list, list) {
		if (!net_eq(devlink_net(devlink), sock_net(msg->sk)) ||
		    !devlink->ops || !devlink->ops->sb_tc_pool_bind_get)
			continue;

		mutex_lock(&devlink->lock);
		list_for_each_entry(devlink_sb, &devlink->sb_list, list) {
			err = __sb_tc_pool_bind_get_dumpit(msg, start, &idx,
							   devlink,
							   devlink_sb,
							   NETLINK_CB(cb->skb).portid,
							   cb->nlh->nlmsg_seq);
			if (err && err != -EOPNOTSUPP) {
				mutex_unlock(&devlink->lock);
				goto out;
			}
		}
		mutex_unlock(&devlink->lock);
	}
out:
	mutex_unlock(&devlink_mutex);

	cb->args[0] = idx;
	return msg->len;
}

static int devlink_sb_tc_pool_bind_set(struct devlink_port *devlink_port,
				       unsigned int sb_index, u16 tc_index,
				       enum devlink_sb_pool_type pool_type,
				       u16 pool_index, u32 threshold)

{
	const struct devlink_ops *ops = devlink_port->devlink->ops;

	if (ops && ops->sb_tc_pool_bind_set)
		return ops->sb_tc_pool_bind_set(devlink_port, sb_index,
						tc_index, pool_type,
						pool_index, threshold);
	return -EOPNOTSUPP;
}

static int devlink_nl_cmd_sb_tc_pool_bind_set_doit(struct sk_buff *skb,
						   struct genl_info *info)
{
	struct devlink_port *devlink_port = info->user_ptr[0];
	struct devlink_sb *devlink_sb = info->user_ptr[1];
	enum devlink_sb_pool_type pool_type;
	u16 tc_index;
	u16 pool_index;
	u32 threshold;
	int err;

	err = devlink_sb_pool_type_get_from_info(info, &pool_type);
	if (err)
		return err;

	err = devlink_sb_tc_index_get_from_info(devlink_sb, info,
						pool_type, &tc_index);
	if (err)
		return err;

	err = devlink_sb_pool_index_get_from_info(devlink_sb, info,
						  &pool_index);
	if (err)
		return err;

	if (!info->attrs[DEVLINK_ATTR_SB_THRESHOLD])
		return -EINVAL;

	threshold = nla_get_u32(info->attrs[DEVLINK_ATTR_SB_THRESHOLD]);
	return devlink_sb_tc_pool_bind_set(devlink_port, devlink_sb->index,
					   tc_index, pool_type,
					   pool_index, threshold);
}

static int devlink_nl_cmd_sb_occ_snapshot_doit(struct sk_buff *skb,
					       struct genl_info *info)
{
	struct devlink *devlink = info->user_ptr[0];
	struct devlink_sb *devlink_sb = info->user_ptr[1];
	const struct devlink_ops *ops = devlink->ops;

	if (ops && ops->sb_occ_snapshot)
		return ops->sb_occ_snapshot(devlink, devlink_sb->index);
	return -EOPNOTSUPP;
}

static int devlink_nl_cmd_sb_occ_max_clear_doit(struct sk_buff *skb,
						struct genl_info *info)
{
	struct devlink *devlink = info->user_ptr[0];
	struct devlink_sb *devlink_sb = info->user_ptr[1];
	const struct devlink_ops *ops = devlink->ops;

	if (ops && ops->sb_occ_max_clear)
		return ops->sb_occ_max_clear(devlink, devlink_sb->index);
	return -EOPNOTSUPP;
}

static int devlink_nl_eswitch_fill(struct sk_buff *msg, struct devlink *devlink,
				   enum devlink_command cmd, u32 portid,
				   u32 seq, int flags)
{
	const struct devlink_ops *ops = devlink->ops;
	u8 inline_mode, encap_mode;
	void *hdr;
	int err = 0;
	u16 mode;

	hdr = genlmsg_put(msg, portid, seq, &devlink_nl_family, flags, cmd);
	if (!hdr)
		return -EMSGSIZE;

	err = devlink_nl_put_handle(msg, devlink);
	if (err)
		goto nla_put_failure;

	if (ops->eswitch_mode_get) {
		err = ops->eswitch_mode_get(devlink, &mode);
		if (err)
			goto nla_put_failure;
		err = nla_put_u16(msg, DEVLINK_ATTR_ESWITCH_MODE, mode);
		if (err)
			goto nla_put_failure;
	}

	if (ops->eswitch_inline_mode_get) {
		err = ops->eswitch_inline_mode_get(devlink, &inline_mode);
		if (err)
			goto nla_put_failure;
		err = nla_put_u8(msg, DEVLINK_ATTR_ESWITCH_INLINE_MODE,
				 inline_mode);
		if (err)
			goto nla_put_failure;
	}

	if (ops->eswitch_encap_mode_get) {
		err = ops->eswitch_encap_mode_get(devlink, &encap_mode);
		if (err)
			goto nla_put_failure;
		err = nla_put_u8(msg, DEVLINK_ATTR_ESWITCH_ENCAP_MODE, encap_mode);
		if (err)
			goto nla_put_failure;
	}

	genlmsg_end(msg, hdr);
	return 0;

nla_put_failure:
	genlmsg_cancel(msg, hdr);
	return err;
}

static int devlink_nl_cmd_eswitch_get_doit(struct sk_buff *skb,
					   struct genl_info *info)
{
	struct devlink *devlink = info->user_ptr[0];
	const struct devlink_ops *ops = devlink->ops;
	struct sk_buff *msg;
	int err;

	if (!ops)
		return -EOPNOTSUPP;

	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!msg)
		return -ENOMEM;

	err = devlink_nl_eswitch_fill(msg, devlink, DEVLINK_CMD_ESWITCH_GET,
				      info->snd_portid, info->snd_seq, 0);

	if (err) {
		nlmsg_free(msg);
		return err;
	}

	return genlmsg_reply(msg, info);
}

static int devlink_nl_cmd_eswitch_set_doit(struct sk_buff *skb,
					   struct genl_info *info)
{
	struct devlink *devlink = info->user_ptr[0];
	const struct devlink_ops *ops = devlink->ops;
	u8 inline_mode, encap_mode;
	int err = 0;
	u16 mode;

	if (!ops)
		return -EOPNOTSUPP;

	if (info->attrs[DEVLINK_ATTR_ESWITCH_MODE]) {
		if (!ops->eswitch_mode_set)
			return -EOPNOTSUPP;
		mode = nla_get_u16(info->attrs[DEVLINK_ATTR_ESWITCH_MODE]);
		err = ops->eswitch_mode_set(devlink, mode);
		if (err)
			return err;
	}

	if (info->attrs[DEVLINK_ATTR_ESWITCH_INLINE_MODE]) {
		if (!ops->eswitch_inline_mode_set)
			return -EOPNOTSUPP;
		inline_mode = nla_get_u8(
				info->attrs[DEVLINK_ATTR_ESWITCH_INLINE_MODE]);
		err = ops->eswitch_inline_mode_set(devlink, inline_mode);
		if (err)
			return err;
	}

	if (info->attrs[DEVLINK_ATTR_ESWITCH_ENCAP_MODE]) {
		if (!ops->eswitch_encap_mode_set)
			return -EOPNOTSUPP;
		encap_mode = nla_get_u8(info->attrs[DEVLINK_ATTR_ESWITCH_ENCAP_MODE]);
		err = ops->eswitch_encap_mode_set(devlink, encap_mode);
		if (err)
			return err;
	}

	return 0;
}

int devlink_dpipe_match_put(struct sk_buff *skb,
			    struct devlink_dpipe_match *match)
{
	struct devlink_dpipe_header *header = match->header;
	struct devlink_dpipe_field *field = &header->fields[match->field_id];
	struct nlattr *match_attr;

	match_attr = nla_nest_start(skb, DEVLINK_ATTR_DPIPE_MATCH);
	if (!match_attr)
		return -EMSGSIZE;

	if (nla_put_u32(skb, DEVLINK_ATTR_DPIPE_MATCH_TYPE, match->type) ||
	    nla_put_u32(skb, DEVLINK_ATTR_DPIPE_HEADER_INDEX, match->header_index) ||
	    nla_put_u32(skb, DEVLINK_ATTR_DPIPE_HEADER_ID, header->id) ||
	    nla_put_u32(skb, DEVLINK_ATTR_DPIPE_FIELD_ID, field->id) ||
	    nla_put_u8(skb, DEVLINK_ATTR_DPIPE_HEADER_GLOBAL, header->global))
		goto nla_put_failure;

	nla_nest_end(skb, match_attr);
	return 0;

nla_put_failure:
	nla_nest_cancel(skb, match_attr);
	return -EMSGSIZE;
}
EXPORT_SYMBOL_GPL(devlink_dpipe_match_put);

static int devlink_dpipe_matches_put(struct devlink_dpipe_table *table,
				     struct sk_buff *skb)
{
	struct nlattr *matches_attr;

	matches_attr = nla_nest_start(skb, DEVLINK_ATTR_DPIPE_TABLE_MATCHES);
	if (!matches_attr)
		return -EMSGSIZE;

	if (table->table_ops->matches_dump(table->priv, skb))
		goto nla_put_failure;

	nla_nest_end(skb, matches_attr);
	return 0;

nla_put_failure:
	nla_nest_cancel(skb, matches_attr);
	return -EMSGSIZE;
}

int devlink_dpipe_action_put(struct sk_buff *skb,
			     struct devlink_dpipe_action *action)
{
	struct devlink_dpipe_header *header = action->header;
	struct devlink_dpipe_field *field = &header->fields[action->field_id];
	struct nlattr *action_attr;

	action_attr = nla_nest_start(skb, DEVLINK_ATTR_DPIPE_ACTION);
	if (!action_attr)
		return -EMSGSIZE;

	if (nla_put_u32(skb, DEVLINK_ATTR_DPIPE_ACTION_TYPE, action->type) ||
	    nla_put_u32(skb, DEVLINK_ATTR_DPIPE_HEADER_INDEX, action->header_index) ||
	    nla_put_u32(skb, DEVLINK_ATTR_DPIPE_HEADER_ID, header->id) ||
	    nla_put_u32(skb, DEVLINK_ATTR_DPIPE_FIELD_ID, field->id) ||
	    nla_put_u8(skb, DEVLINK_ATTR_DPIPE_HEADER_GLOBAL, header->global))
		goto nla_put_failure;

	nla_nest_end(skb, action_attr);
	return 0;

nla_put_failure:
	nla_nest_cancel(skb, action_attr);
	return -EMSGSIZE;
}
EXPORT_SYMBOL_GPL(devlink_dpipe_action_put);

static int devlink_dpipe_actions_put(struct devlink_dpipe_table *table,
				     struct sk_buff *skb)
{
	struct nlattr *actions_attr;

	actions_attr = nla_nest_start(skb, DEVLINK_ATTR_DPIPE_TABLE_ACTIONS);
	if (!actions_attr)
		return -EMSGSIZE;

	if (table->table_ops->actions_dump(table->priv, skb))
		goto nla_put_failure;

	nla_nest_end(skb, actions_attr);
	return 0;

nla_put_failure:
	nla_nest_cancel(skb, actions_attr);
	return -EMSGSIZE;
}

static int devlink_dpipe_table_put(struct sk_buff *skb,
				   struct devlink_dpipe_table *table)
{
	struct nlattr *table_attr;
	u64 table_size;

	table_size = table->table_ops->size_get(table->priv);
	table_attr = nla_nest_start(skb, DEVLINK_ATTR_DPIPE_TABLE);
	if (!table_attr)
		return -EMSGSIZE;

	if (nla_put_string(skb, DEVLINK_ATTR_DPIPE_TABLE_NAME, table->name) ||
	    nla_put_u64_64bit(skb, DEVLINK_ATTR_DPIPE_TABLE_SIZE, table_size,
			      DEVLINK_ATTR_PAD))
		goto nla_put_failure;
	if (nla_put_u8(skb, DEVLINK_ATTR_DPIPE_TABLE_COUNTERS_ENABLED,
		       table->counters_enabled))
		goto nla_put_failure;

	if (table->resource_valid) {
		if (nla_put_u64_64bit(skb, DEVLINK_ATTR_DPIPE_TABLE_RESOURCE_ID,
				      table->resource_id, DEVLINK_ATTR_PAD) ||
		    nla_put_u64_64bit(skb, DEVLINK_ATTR_DPIPE_TABLE_RESOURCE_UNITS,
				      table->resource_units, DEVLINK_ATTR_PAD))
			goto nla_put_failure;
	}
	if (devlink_dpipe_matches_put(table, skb))
		goto nla_put_failure;

	if (devlink_dpipe_actions_put(table, skb))
		goto nla_put_failure;

	nla_nest_end(skb, table_attr);
	return 0;

nla_put_failure:
	nla_nest_cancel(skb, table_attr);
	return -EMSGSIZE;
}

static int devlink_dpipe_send_and_alloc_skb(struct sk_buff **pskb,
					    struct genl_info *info)
{
	int err;

	if (*pskb) {
		err = genlmsg_reply(*pskb, info);
		if (err)
			return err;
	}
	*pskb = genlmsg_new(GENLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!*pskb)
		return -ENOMEM;
	return 0;
}

static int devlink_dpipe_tables_fill(struct genl_info *info,
				     enum devlink_command cmd, int flags,
				     struct list_head *dpipe_tables,
				     const char *table_name)
{
	struct devlink *devlink = info->user_ptr[0];
	struct devlink_dpipe_table *table;
	struct nlattr *tables_attr;
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh;
	bool incomplete;
	void *hdr;
	int i;
	int err;

	table = list_first_entry(dpipe_tables,
				 struct devlink_dpipe_table, list);
start_again:
	err = devlink_dpipe_send_and_alloc_skb(&skb, info);
	if (err)
		return err;

	hdr = genlmsg_put(skb, info->snd_portid, info->snd_seq,
			  &devlink_nl_family, NLM_F_MULTI, cmd);
	if (!hdr) {
		nlmsg_free(skb);
		return -EMSGSIZE;
	}

	if (devlink_nl_put_handle(skb, devlink))
		goto nla_put_failure;
	tables_attr = nla_nest_start(skb, DEVLINK_ATTR_DPIPE_TABLES);
	if (!tables_attr)
		goto nla_put_failure;

	i = 0;
	incomplete = false;
	list_for_each_entry_from(table, dpipe_tables, list) {
		if (!table_name) {
			err = devlink_dpipe_table_put(skb, table);
			if (err) {
				if (!i)
					goto err_table_put;
				incomplete = true;
				break;
			}
		} else {
			if (!strcmp(table->name, table_name)) {
				err = devlink_dpipe_table_put(skb, table);
				if (err)
					break;
			}
		}
		i++;
	}

	nla_nest_end(skb, tables_attr);
	genlmsg_end(skb, hdr);
	if (incomplete)
		goto start_again;

send_done:
	nlh = nlmsg_put(skb, info->snd_portid, info->snd_seq,
			NLMSG_DONE, 0, flags | NLM_F_MULTI);
	if (!nlh) {
		err = devlink_dpipe_send_and_alloc_skb(&skb, info);
		if (err)
			return err;
		goto send_done;
	}

	return genlmsg_reply(skb, info);

nla_put_failure:
	err = -EMSGSIZE;
err_table_put:
	nlmsg_free(skb);
	return err;
}

static int devlink_nl_cmd_dpipe_table_get(struct sk_buff *skb,
					  struct genl_info *info)
{
	struct devlink *devlink = info->user_ptr[0];
	const char *table_name =  NULL;

	if (info->attrs[DEVLINK_ATTR_DPIPE_TABLE_NAME])
		table_name = nla_data(info->attrs[DEVLINK_ATTR_DPIPE_TABLE_NAME]);

	return devlink_dpipe_tables_fill(info, DEVLINK_CMD_DPIPE_TABLE_GET, 0,
					 &devlink->dpipe_table_list,
					 table_name);
}

static int devlink_dpipe_value_put(struct sk_buff *skb,
				   struct devlink_dpipe_value *value)
{
	if (nla_put(skb, DEVLINK_ATTR_DPIPE_VALUE,
		    value->value_size, value->value))
		return -EMSGSIZE;
	if (value->mask)
		if (nla_put(skb, DEVLINK_ATTR_DPIPE_VALUE_MASK,
			    value->value_size, value->mask))
			return -EMSGSIZE;
	if (value->mapping_valid)
		if (nla_put_u32(skb, DEVLINK_ATTR_DPIPE_VALUE_MAPPING,
				value->mapping_value))
			return -EMSGSIZE;
	return 0;
}

static int devlink_dpipe_action_value_put(struct sk_buff *skb,
					  struct devlink_dpipe_value *value)
{
	if (!value->action)
		return -EINVAL;
	if (devlink_dpipe_action_put(skb, value->action))
		return -EMSGSIZE;
	if (devlink_dpipe_value_put(skb, value))
		return -EMSGSIZE;
	return 0;
}

static int devlink_dpipe_action_values_put(struct sk_buff *skb,
					   struct devlink_dpipe_value *values,
					   unsigned int values_count)
{
	struct nlattr *action_attr;
	int i;
	int err;

	for (i = 0; i < values_count; i++) {
		action_attr = nla_nest_start(skb,
					     DEVLINK_ATTR_DPIPE_ACTION_VALUE);
		if (!action_attr)
			return -EMSGSIZE;
		err = devlink_dpipe_action_value_put(skb, &values[i]);
		if (err)
			goto err_action_value_put;
		nla_nest_end(skb, action_attr);
	}
	return 0;

err_action_value_put:
	nla_nest_cancel(skb, action_attr);
	return err;
}

static int devlink_dpipe_match_value_put(struct sk_buff *skb,
					 struct devlink_dpipe_value *value)
{
	if (!value->match)
		return -EINVAL;
	if (devlink_dpipe_match_put(skb, value->match))
		return -EMSGSIZE;
	if (devlink_dpipe_value_put(skb, value))
		return -EMSGSIZE;
	return 0;
}

static int devlink_dpipe_match_values_put(struct sk_buff *skb,
					  struct devlink_dpipe_value *values,
					  unsigned int values_count)
{
	struct nlattr *match_attr;
	int i;
	int err;

	for (i = 0; i < values_count; i++) {
		match_attr = nla_nest_start(skb,
					    DEVLINK_ATTR_DPIPE_MATCH_VALUE);
		if (!match_attr)
			return -EMSGSIZE;
		err = devlink_dpipe_match_value_put(skb, &values[i]);
		if (err)
			goto err_match_value_put;
		nla_nest_end(skb, match_attr);
	}
	return 0;

err_match_value_put:
	nla_nest_cancel(skb, match_attr);
	return err;
}

static int devlink_dpipe_entry_put(struct sk_buff *skb,
				   struct devlink_dpipe_entry *entry)
{
	struct nlattr *entry_attr, *matches_attr, *actions_attr;
	int err;

	entry_attr = nla_nest_start(skb, DEVLINK_ATTR_DPIPE_ENTRY);
	if (!entry_attr)
		return  -EMSGSIZE;

	if (nla_put_u64_64bit(skb, DEVLINK_ATTR_DPIPE_ENTRY_INDEX, entry->index,
			      DEVLINK_ATTR_PAD))
		goto nla_put_failure;
	if (entry->counter_valid)
		if (nla_put_u64_64bit(skb, DEVLINK_ATTR_DPIPE_ENTRY_COUNTER,
				      entry->counter, DEVLINK_ATTR_PAD))
			goto nla_put_failure;

	matches_attr = nla_nest_start(skb,
				      DEVLINK_ATTR_DPIPE_ENTRY_MATCH_VALUES);
	if (!matches_attr)
		goto nla_put_failure;

	err = devlink_dpipe_match_values_put(skb, entry->match_values,
					     entry->match_values_count);
	if (err) {
		nla_nest_cancel(skb, matches_attr);
		goto err_match_values_put;
	}
	nla_nest_end(skb, matches_attr);

	actions_attr = nla_nest_start(skb,
				      DEVLINK_ATTR_DPIPE_ENTRY_ACTION_VALUES);
	if (!actions_attr)
		goto nla_put_failure;

	err = devlink_dpipe_action_values_put(skb, entry->action_values,
					      entry->action_values_count);
	if (err) {
		nla_nest_cancel(skb, actions_attr);
		goto err_action_values_put;
	}
	nla_nest_end(skb, actions_attr);

	nla_nest_end(skb, entry_attr);
	return 0;

nla_put_failure:
	err = -EMSGSIZE;
err_match_values_put:
err_action_values_put:
	nla_nest_cancel(skb, entry_attr);
	return err;
}

static struct devlink_dpipe_table *
devlink_dpipe_table_find(struct list_head *dpipe_tables,
			 const char *table_name)
{
	struct devlink_dpipe_table *table;

	list_for_each_entry_rcu(table, dpipe_tables, list) {
		if (!strcmp(table->name, table_name))
			return table;
	}
	return NULL;
}

int devlink_dpipe_entry_ctx_prepare(struct devlink_dpipe_dump_ctx *dump_ctx)
{
	struct devlink *devlink;
	int err;

	err = devlink_dpipe_send_and_alloc_skb(&dump_ctx->skb,
					       dump_ctx->info);
	if (err)
		return err;

	dump_ctx->hdr = genlmsg_put(dump_ctx->skb,
				    dump_ctx->info->snd_portid,
				    dump_ctx->info->snd_seq,
				    &devlink_nl_family, NLM_F_MULTI,
				    dump_ctx->cmd);
	if (!dump_ctx->hdr)
		goto nla_put_failure;

	devlink = dump_ctx->info->user_ptr[0];
	if (devlink_nl_put_handle(dump_ctx->skb, devlink))
		goto nla_put_failure;
	dump_ctx->nest = nla_nest_start(dump_ctx->skb,
					DEVLINK_ATTR_DPIPE_ENTRIES);
	if (!dump_ctx->nest)
		goto nla_put_failure;
	return 0;

nla_put_failure:
	nlmsg_free(dump_ctx->skb);
	return -EMSGSIZE;
}
EXPORT_SYMBOL_GPL(devlink_dpipe_entry_ctx_prepare);

int devlink_dpipe_entry_ctx_append(struct devlink_dpipe_dump_ctx *dump_ctx,
				   struct devlink_dpipe_entry *entry)
{
	return devlink_dpipe_entry_put(dump_ctx->skb, entry);
}
EXPORT_SYMBOL_GPL(devlink_dpipe_entry_ctx_append);

int devlink_dpipe_entry_ctx_close(struct devlink_dpipe_dump_ctx *dump_ctx)
{
	nla_nest_end(dump_ctx->skb, dump_ctx->nest);
	genlmsg_end(dump_ctx->skb, dump_ctx->hdr);
	return 0;
}
EXPORT_SYMBOL_GPL(devlink_dpipe_entry_ctx_close);

void devlink_dpipe_entry_clear(struct devlink_dpipe_entry *entry)

{
	unsigned int value_count, value_index;
	struct devlink_dpipe_value *value;

	value = entry->action_values;
	value_count = entry->action_values_count;
	for (value_index = 0; value_index < value_count; value_index++) {
		kfree(value[value_index].value);
		kfree(value[value_index].mask);
	}

	value = entry->match_values;
	value_count = entry->match_values_count;
	for (value_index = 0; value_index < value_count; value_index++) {
		kfree(value[value_index].value);
		kfree(value[value_index].mask);
	}
}
EXPORT_SYMBOL(devlink_dpipe_entry_clear);

static int devlink_dpipe_entries_fill(struct genl_info *info,
				      enum devlink_command cmd, int flags,
				      struct devlink_dpipe_table *table)
{
	struct devlink_dpipe_dump_ctx dump_ctx;
	struct nlmsghdr *nlh;
	int err;

	dump_ctx.skb = NULL;
	dump_ctx.cmd = cmd;
	dump_ctx.info = info;

	err = table->table_ops->entries_dump(table->priv,
					     table->counters_enabled,
					     &dump_ctx);
	if (err)
		return err;

send_done:
	nlh = nlmsg_put(dump_ctx.skb, info->snd_portid, info->snd_seq,
			NLMSG_DONE, 0, flags | NLM_F_MULTI);
	if (!nlh) {
		err = devlink_dpipe_send_and_alloc_skb(&dump_ctx.skb, info);
		if (err)
			return err;
		goto send_done;
	}
	return genlmsg_reply(dump_ctx.skb, info);
}

static int devlink_nl_cmd_dpipe_entries_get(struct sk_buff *skb,
					    struct genl_info *info)
{
	struct devlink *devlink = info->user_ptr[0];
	struct devlink_dpipe_table *table;
	const char *table_name;

	if (!info->attrs[DEVLINK_ATTR_DPIPE_TABLE_NAME])
		return -EINVAL;

	table_name = nla_data(info->attrs[DEVLINK_ATTR_DPIPE_TABLE_NAME]);
	table = devlink_dpipe_table_find(&devlink->dpipe_table_list,
					 table_name);
	if (!table)
		return -EINVAL;

	if (!table->table_ops->entries_dump)
		return -EINVAL;

	return devlink_dpipe_entries_fill(info, DEVLINK_CMD_DPIPE_ENTRIES_GET,
					  0, table);
}

static int devlink_dpipe_fields_put(struct sk_buff *skb,
				    const struct devlink_dpipe_header *header)
{
	struct devlink_dpipe_field *field;
	struct nlattr *field_attr;
	int i;

	for (i = 0; i < header->fields_count; i++) {
		field = &header->fields[i];
		field_attr = nla_nest_start(skb, DEVLINK_ATTR_DPIPE_FIELD);
		if (!field_attr)
			return -EMSGSIZE;
		if (nla_put_string(skb, DEVLINK_ATTR_DPIPE_FIELD_NAME, field->name) ||
		    nla_put_u32(skb, DEVLINK_ATTR_DPIPE_FIELD_ID, field->id) ||
		    nla_put_u32(skb, DEVLINK_ATTR_DPIPE_FIELD_BITWIDTH, field->bitwidth) ||
		    nla_put_u32(skb, DEVLINK_ATTR_DPIPE_FIELD_MAPPING_TYPE, field->mapping_type))
			goto nla_put_failure;
		nla_nest_end(skb, field_attr);
	}
	return 0;

nla_put_failure:
	nla_nest_cancel(skb, field_attr);
	return -EMSGSIZE;
}

static int devlink_dpipe_header_put(struct sk_buff *skb,
				    struct devlink_dpipe_header *header)
{
	struct nlattr *fields_attr, *header_attr;
	int err;

	header_attr = nla_nest_start(skb, DEVLINK_ATTR_DPIPE_HEADER);
	if (!header_attr)
		return -EMSGSIZE;

	if (nla_put_string(skb, DEVLINK_ATTR_DPIPE_HEADER_NAME, header->name) ||
	    nla_put_u32(skb, DEVLINK_ATTR_DPIPE_HEADER_ID, header->id) ||
	    nla_put_u8(skb, DEVLINK_ATTR_DPIPE_HEADER_GLOBAL, header->global))
		goto nla_put_failure;

	fields_attr = nla_nest_start(skb, DEVLINK_ATTR_DPIPE_HEADER_FIELDS);
	if (!fields_attr)
		goto nla_put_failure;

	err = devlink_dpipe_fields_put(skb, header);
	if (err) {
		nla_nest_cancel(skb, fields_attr);
		goto nla_put_failure;
	}
	nla_nest_end(skb, fields_attr);
	nla_nest_end(skb, header_attr);
	return 0;

nla_put_failure:
	err = -EMSGSIZE;
	nla_nest_cancel(skb, header_attr);
	return err;
}

static int devlink_dpipe_headers_fill(struct genl_info *info,
				      enum devlink_command cmd, int flags,
				      struct devlink_dpipe_headers *
				      dpipe_headers)
{
	struct devlink *devlink = info->user_ptr[0];
	struct nlattr *headers_attr;
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh;
	void *hdr;
	int i, j;
	int err;

	i = 0;
start_again:
	err = devlink_dpipe_send_and_alloc_skb(&skb, info);
	if (err)
		return err;

	hdr = genlmsg_put(skb, info->snd_portid, info->snd_seq,
			  &devlink_nl_family, NLM_F_MULTI, cmd);
	if (!hdr) {
		nlmsg_free(skb);
		return -EMSGSIZE;
	}

	if (devlink_nl_put_handle(skb, devlink))
		goto nla_put_failure;
	headers_attr = nla_nest_start(skb, DEVLINK_ATTR_DPIPE_HEADERS);
	if (!headers_attr)
		goto nla_put_failure;

	j = 0;
	for (; i < dpipe_headers->headers_count; i++) {
		err = devlink_dpipe_header_put(skb, dpipe_headers->headers[i]);
		if (err) {
			if (!j)
				goto err_table_put;
			break;
		}
		j++;
	}
	nla_nest_end(skb, headers_attr);
	genlmsg_end(skb, hdr);
	if (i != dpipe_headers->headers_count)
		goto start_again;

send_done:
	nlh = nlmsg_put(skb, info->snd_portid, info->snd_seq,
			NLMSG_DONE, 0, flags | NLM_F_MULTI);
	if (!nlh) {
		err = devlink_dpipe_send_and_alloc_skb(&skb, info);
		if (err)
			return err;
		goto send_done;
	}
	return genlmsg_reply(skb, info);

nla_put_failure:
	err = -EMSGSIZE;
err_table_put:
	nlmsg_free(skb);
	return err;
}

static int devlink_nl_cmd_dpipe_headers_get(struct sk_buff *skb,
					    struct genl_info *info)
{
	struct devlink *devlink = info->user_ptr[0];

	if (!devlink->dpipe_headers)
		return -EOPNOTSUPP;
	return devlink_dpipe_headers_fill(info, DEVLINK_CMD_DPIPE_HEADERS_GET,
					  0, devlink->dpipe_headers);
}

static int devlink_dpipe_table_counters_set(struct devlink *devlink,
					    const char *table_name,
					    bool enable)
{
	struct devlink_dpipe_table *table;

	table = devlink_dpipe_table_find(&devlink->dpipe_table_list,
					 table_name);
	if (!table)
		return -EINVAL;

	if (table->counter_control_extern)
		return -EOPNOTSUPP;

	if (!(table->counters_enabled ^ enable))
		return 0;

	table->counters_enabled = enable;
	if (table->table_ops->counters_set_update)
		table->table_ops->counters_set_update(table->priv, enable);
	return 0;
}

static int devlink_nl_cmd_dpipe_table_counters_set(struct sk_buff *skb,
						   struct genl_info *info)
{
	struct devlink *devlink = info->user_ptr[0];
	const char *table_name;
	bool counters_enable;

	if (!info->attrs[DEVLINK_ATTR_DPIPE_TABLE_NAME] ||
	    !info->attrs[DEVLINK_ATTR_DPIPE_TABLE_COUNTERS_ENABLED])
		return -EINVAL;

	table_name = nla_data(info->attrs[DEVLINK_ATTR_DPIPE_TABLE_NAME]);
	counters_enable = !!nla_get_u8(info->attrs[DEVLINK_ATTR_DPIPE_TABLE_COUNTERS_ENABLED]);

	return devlink_dpipe_table_counters_set(devlink, table_name,
						counters_enable);
}

static struct devlink_resource *
devlink_resource_find(struct devlink *devlink,
		      struct devlink_resource *resource, u64 resource_id)
{
	struct list_head *resource_list;

	if (resource)
		resource_list = &resource->resource_list;
	else
		resource_list = &devlink->resource_list;

	list_for_each_entry(resource, resource_list, list) {
		struct devlink_resource *child_resource;

		if (resource->id == resource_id)
			return resource;

		child_resource = devlink_resource_find(devlink, resource,
						       resource_id);
		if (child_resource)
			return child_resource;
	}
	return NULL;
}

static void
devlink_resource_validate_children(struct devlink_resource *resource)
{
	struct devlink_resource *child_resource;
	bool size_valid = true;
	u64 parts_size = 0;

	if (list_empty(&resource->resource_list))
		goto out;

	list_for_each_entry(child_resource, &resource->resource_list, list)
		parts_size += child_resource->size_new;

	if (parts_size > resource->size_new)
		size_valid = false;
out:
	resource->size_valid = size_valid;
}

static int
devlink_resource_validate_size(struct devlink_resource *resource, u64 size,
			       struct netlink_ext_ack *extack)
{
	u64 reminder;
	int err = 0;

	if (size > resource->size_params.size_max) {
		NL_SET_ERR_MSG_MOD(extack, "Size larger than maximum");
		err = -EINVAL;
	}

	if (size < resource->size_params.size_min) {
		NL_SET_ERR_MSG_MOD(extack, "Size smaller than minimum");
		err = -EINVAL;
	}

	div64_u64_rem(size, resource->size_params.size_granularity, &reminder);
	if (reminder) {
		NL_SET_ERR_MSG_MOD(extack, "Wrong granularity");
		err = -EINVAL;
	}

	return err;
}

static int devlink_nl_cmd_resource_set(struct sk_buff *skb,
				       struct genl_info *info)
{
	struct devlink *devlink = info->user_ptr[0];
	struct devlink_resource *resource;
	u64 resource_id;
	u64 size;
	int err;

	if (!info->attrs[DEVLINK_ATTR_RESOURCE_ID] ||
	    !info->attrs[DEVLINK_ATTR_RESOURCE_SIZE])
		return -EINVAL;
	resource_id = nla_get_u64(info->attrs[DEVLINK_ATTR_RESOURCE_ID]);

	resource = devlink_resource_find(devlink, NULL, resource_id);
	if (!resource)
		return -EINVAL;

	size = nla_get_u64(info->attrs[DEVLINK_ATTR_RESOURCE_SIZE]);
	err = devlink_resource_validate_size(resource, size, info->extack);
	if (err)
		return err;

	resource->size_new = size;
	devlink_resource_validate_children(resource);
	if (resource->parent)
		devlink_resource_validate_children(resource->parent);
	return 0;
}

static int
devlink_resource_size_params_put(struct devlink_resource *resource,
				 struct sk_buff *skb)
{
	struct devlink_resource_size_params *size_params;

	size_params = &resource->size_params;
	if (nla_put_u64_64bit(skb, DEVLINK_ATTR_RESOURCE_SIZE_GRAN,
			      size_params->size_granularity, DEVLINK_ATTR_PAD) ||
	    nla_put_u64_64bit(skb, DEVLINK_ATTR_RESOURCE_SIZE_MAX,
			      size_params->size_max, DEVLINK_ATTR_PAD) ||
	    nla_put_u64_64bit(skb, DEVLINK_ATTR_RESOURCE_SIZE_MIN,
			      size_params->size_min, DEVLINK_ATTR_PAD) ||
	    nla_put_u8(skb, DEVLINK_ATTR_RESOURCE_UNIT, size_params->unit))
		return -EMSGSIZE;
	return 0;
}

static int devlink_resource_occ_put(struct devlink_resource *resource,
				    struct sk_buff *skb)
{
	if (!resource->occ_get)
		return 0;
	return nla_put_u64_64bit(skb, DEVLINK_ATTR_RESOURCE_OCC,
				 resource->occ_get(resource->occ_get_priv),
				 DEVLINK_ATTR_PAD);
}

static int devlink_resource_put(struct devlink *devlink, struct sk_buff *skb,
				struct devlink_resource *resource)
{
	struct devlink_resource *child_resource;
	struct nlattr *child_resource_attr;
	struct nlattr *resource_attr;

	resource_attr = nla_nest_start(skb, DEVLINK_ATTR_RESOURCE);
	if (!resource_attr)
		return -EMSGSIZE;

	if (nla_put_string(skb, DEVLINK_ATTR_RESOURCE_NAME, resource->name) ||
	    nla_put_u64_64bit(skb, DEVLINK_ATTR_RESOURCE_SIZE, resource->size,
			      DEVLINK_ATTR_PAD) ||
	    nla_put_u64_64bit(skb, DEVLINK_ATTR_RESOURCE_ID, resource->id,
			      DEVLINK_ATTR_PAD))
		goto nla_put_failure;
	if (resource->size != resource->size_new)
		nla_put_u64_64bit(skb, DEVLINK_ATTR_RESOURCE_SIZE_NEW,
				  resource->size_new, DEVLINK_ATTR_PAD);
	if (devlink_resource_occ_put(resource, skb))
		goto nla_put_failure;
	if (devlink_resource_size_params_put(resource, skb))
		goto nla_put_failure;
	if (list_empty(&resource->resource_list))
		goto out;

	if (nla_put_u8(skb, DEVLINK_ATTR_RESOURCE_SIZE_VALID,
		       resource->size_valid))
		goto nla_put_failure;

	child_resource_attr = nla_nest_start(skb, DEVLINK_ATTR_RESOURCE_LIST);
	if (!child_resource_attr)
		goto nla_put_failure;

	list_for_each_entry(child_resource, &resource->resource_list, list) {
		if (devlink_resource_put(devlink, skb, child_resource))
			goto resource_put_failure;
	}

	nla_nest_end(skb, child_resource_attr);
out:
	nla_nest_end(skb, resource_attr);
	return 0;

resource_put_failure:
	nla_nest_cancel(skb, child_resource_attr);
nla_put_failure:
	nla_nest_cancel(skb, resource_attr);
	return -EMSGSIZE;
}

static int devlink_resource_fill(struct genl_info *info,
				 enum devlink_command cmd, int flags)
{
	struct devlink *devlink = info->user_ptr[0];
	struct devlink_resource *resource;
	struct nlattr *resources_attr;
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh;
	bool incomplete;
	void *hdr;
	int i;
	int err;

	resource = list_first_entry(&devlink->resource_list,
				    struct devlink_resource, list);
start_again:
	err = devlink_dpipe_send_and_alloc_skb(&skb, info);
	if (err)
		return err;

	hdr = genlmsg_put(skb, info->snd_portid, info->snd_seq,
			  &devlink_nl_family, NLM_F_MULTI, cmd);
	if (!hdr) {
		nlmsg_free(skb);
		return -EMSGSIZE;
	}

	if (devlink_nl_put_handle(skb, devlink))
		goto nla_put_failure;

	resources_attr = nla_nest_start(skb, DEVLINK_ATTR_RESOURCE_LIST);
	if (!resources_attr)
		goto nla_put_failure;

	incomplete = false;
	i = 0;
	list_for_each_entry_from(resource, &devlink->resource_list, list) {
		err = devlink_resource_put(devlink, skb, resource);
		if (err) {
			if (!i)
				goto err_resource_put;
			incomplete = true;
			break;
		}
		i++;
	}
	nla_nest_end(skb, resources_attr);
	genlmsg_end(skb, hdr);
	if (incomplete)
		goto start_again;
send_done:
	nlh = nlmsg_put(skb, info->snd_portid, info->snd_seq,
			NLMSG_DONE, 0, flags | NLM_F_MULTI);
	if (!nlh) {
		err = devlink_dpipe_send_and_alloc_skb(&skb, info);
		if (err)
			return err;
		goto send_done;
	}
	return genlmsg_reply(skb, info);

nla_put_failure:
	err = -EMSGSIZE;
err_resource_put:
	nlmsg_free(skb);
	return err;
}

static int devlink_nl_cmd_resource_dump(struct sk_buff *skb,
					struct genl_info *info)
{
	struct devlink *devlink = info->user_ptr[0];

	if (list_empty(&devlink->resource_list))
		return -EOPNOTSUPP;

	return devlink_resource_fill(info, DEVLINK_CMD_RESOURCE_DUMP, 0);
}

static int
devlink_resources_validate(struct devlink *devlink,
			   struct devlink_resource *resource,
			   struct genl_info *info)
{
	struct list_head *resource_list;
	int err = 0;

	if (resource)
		resource_list = &resource->resource_list;
	else
		resource_list = &devlink->resource_list;

	list_for_each_entry(resource, resource_list, list) {
		if (!resource->size_valid)
			return -EINVAL;
		err = devlink_resources_validate(devlink, resource, info);
		if (err)
			return err;
	}
	return err;
}

static int devlink_nl_cmd_reload(struct sk_buff *skb, struct genl_info *info)
{
	struct devlink *devlink = info->user_ptr[0];
	int err;

	if (!devlink->ops->reload)
		return -EOPNOTSUPP;

	err = devlink_resources_validate(devlink, NULL, info);
	if (err) {
		NL_SET_ERR_MSG_MOD(info->extack, "resources size validation failed");
		return err;
	}
	return devlink->ops->reload(devlink, info->extack);
}

static const struct devlink_param devlink_param_generic[] = {
	{
		.id = DEVLINK_PARAM_GENERIC_ID_INT_ERR_RESET,
		.name = DEVLINK_PARAM_GENERIC_INT_ERR_RESET_NAME,
		.type = DEVLINK_PARAM_GENERIC_INT_ERR_RESET_TYPE,
	},
	{
		.id = DEVLINK_PARAM_GENERIC_ID_MAX_MACS,
		.name = DEVLINK_PARAM_GENERIC_MAX_MACS_NAME,
		.type = DEVLINK_PARAM_GENERIC_MAX_MACS_TYPE,
	},
	{
		.id = DEVLINK_PARAM_GENERIC_ID_ENABLE_SRIOV,
		.name = DEVLINK_PARAM_GENERIC_ENABLE_SRIOV_NAME,
		.type = DEVLINK_PARAM_GENERIC_ENABLE_SRIOV_TYPE,
	},
	{
		.id = DEVLINK_PARAM_GENERIC_ID_REGION_SNAPSHOT,
		.name = DEVLINK_PARAM_GENERIC_REGION_SNAPSHOT_NAME,
		.type = DEVLINK_PARAM_GENERIC_REGION_SNAPSHOT_TYPE,
	},
};

static int devlink_param_generic_verify(const struct devlink_param *param)
{
	/* verify it match generic parameter by id and name */
	if (param->id > DEVLINK_PARAM_GENERIC_ID_MAX)
		return -EINVAL;
	if (strcmp(param->name, devlink_param_generic[param->id].name))
		return -ENOENT;

	WARN_ON(param->type != devlink_param_generic[param->id].type);

	return 0;
}

static int devlink_param_driver_verify(const struct devlink_param *param)
{
	int i;

	if (param->id <= DEVLINK_PARAM_GENERIC_ID_MAX)
		return -EINVAL;
	/* verify no such name in generic params */
	for (i = 0; i <= DEVLINK_PARAM_GENERIC_ID_MAX; i++)
		if (!strcmp(param->name, devlink_param_generic[i].name))
			return -EEXIST;

	return 0;
}

static struct devlink_param_item *
devlink_param_find_by_name(struct list_head *param_list,
			   const char *param_name)
{
	struct devlink_param_item *param_item;

	list_for_each_entry(param_item, param_list, list)
		if (!strcmp(param_item->param->name, param_name))
			return param_item;
	return NULL;
}

static struct devlink_param_item *
devlink_param_find_by_id(struct list_head *param_list, u32 param_id)
{
	struct devlink_param_item *param_item;

	list_for_each_entry(param_item, param_list, list)
		if (param_item->param->id == param_id)
			return param_item;
	return NULL;
}

static bool
devlink_param_cmode_is_supported(const struct devlink_param *param,
				 enum devlink_param_cmode cmode)
{
	return test_bit(cmode, &param->supported_cmodes);
}

static int devlink_param_get(struct devlink *devlink,
			     const struct devlink_param *param,
			     struct devlink_param_gset_ctx *ctx)
{
	if (!param->get)
		return -EOPNOTSUPP;
	return param->get(devlink, param->id, ctx);
}

static int devlink_param_set(struct devlink *devlink,
			     const struct devlink_param *param,
			     struct devlink_param_gset_ctx *ctx)
{
	if (!param->set)
		return -EOPNOTSUPP;
	return param->set(devlink, param->id, ctx);
}

static int
devlink_param_type_to_nla_type(enum devlink_param_type param_type)
{
	switch (param_type) {
	case DEVLINK_PARAM_TYPE_U8:
		return NLA_U8;
	case DEVLINK_PARAM_TYPE_U16:
		return NLA_U16;
	case DEVLINK_PARAM_TYPE_U32:
		return NLA_U32;
	case DEVLINK_PARAM_TYPE_STRING:
		return NLA_STRING;
	case DEVLINK_PARAM_TYPE_BOOL:
		return NLA_FLAG;
	default:
		return -EINVAL;
	}
}

static int
devlink_nl_param_value_fill_one(struct sk_buff *msg,
				enum devlink_param_type type,
				enum devlink_param_cmode cmode,
				union devlink_param_value val)
{
	struct nlattr *param_value_attr;

	param_value_attr = nla_nest_start(msg, DEVLINK_ATTR_PARAM_VALUE);
	if (!param_value_attr)
		goto nla_put_failure;

	if (nla_put_u8(msg, DEVLINK_ATTR_PARAM_VALUE_CMODE, cmode))
		goto value_nest_cancel;

	switch (type) {
	case DEVLINK_PARAM_TYPE_U8:
		if (nla_put_u8(msg, DEVLINK_ATTR_PARAM_VALUE_DATA, val.vu8))
			goto value_nest_cancel;
		break;
	case DEVLINK_PARAM_TYPE_U16:
		if (nla_put_u16(msg, DEVLINK_ATTR_PARAM_VALUE_DATA, val.vu16))
			goto value_nest_cancel;
		break;
	case DEVLINK_PARAM_TYPE_U32:
		if (nla_put_u32(msg, DEVLINK_ATTR_PARAM_VALUE_DATA, val.vu32))
			goto value_nest_cancel;
		break;
	case DEVLINK_PARAM_TYPE_STRING:
		if (nla_put_string(msg, DEVLINK_ATTR_PARAM_VALUE_DATA,
				   val.vstr))
			goto value_nest_cancel;
		break;
	case DEVLINK_PARAM_TYPE_BOOL:
		if (val.vbool &&
		    nla_put_flag(msg, DEVLINK_ATTR_PARAM_VALUE_DATA))
			goto value_nest_cancel;
		break;
	}

	nla_nest_end(msg, param_value_attr);
	return 0;

value_nest_cancel:
	nla_nest_cancel(msg, param_value_attr);
nla_put_failure:
	return -EMSGSIZE;
}

static int devlink_nl_param_fill(struct sk_buff *msg, struct devlink *devlink,
				 struct devlink_param_item *param_item,
				 enum devlink_command cmd,
				 u32 portid, u32 seq, int flags)
{
	union devlink_param_value param_value[DEVLINK_PARAM_CMODE_MAX + 1];
	const struct devlink_param *param = param_item->param;
	struct devlink_param_gset_ctx ctx;
	struct nlattr *param_values_list;
	struct nlattr *param_attr;
	int nla_type;
	void *hdr;
	int err;
	int i;

	/* Get value from driver part to driverinit configuration mode */
	for (i = 0; i <= DEVLINK_PARAM_CMODE_MAX; i++) {
		if (!devlink_param_cmode_is_supported(param, i))
			continue;
		if (i == DEVLINK_PARAM_CMODE_DRIVERINIT) {
			if (!param_item->driverinit_value_valid)
				return -EOPNOTSUPP;
			param_value[i] = param_item->driverinit_value;
		} else {
			ctx.cmode = i;
			err = devlink_param_get(devlink, param, &ctx);
			if (err)
				return err;
			param_value[i] = ctx.val;
		}
	}

	hdr = genlmsg_put(msg, portid, seq, &devlink_nl_family, flags, cmd);
	if (!hdr)
		return -EMSGSIZE;

	if (devlink_nl_put_handle(msg, devlink))
		goto genlmsg_cancel;
	param_attr = nla_nest_start(msg, DEVLINK_ATTR_PARAM);
	if (!param_attr)
		goto genlmsg_cancel;
	if (nla_put_string(msg, DEVLINK_ATTR_PARAM_NAME, param->name))
		goto param_nest_cancel;
	if (param->generic && nla_put_flag(msg, DEVLINK_ATTR_PARAM_GENERIC))
		goto param_nest_cancel;

	nla_type = devlink_param_type_to_nla_type(param->type);
	if (nla_type < 0)
		goto param_nest_cancel;
	if (nla_put_u8(msg, DEVLINK_ATTR_PARAM_TYPE, nla_type))
		goto param_nest_cancel;

	param_values_list = nla_nest_start(msg, DEVLINK_ATTR_PARAM_VALUES_LIST);
	if (!param_values_list)
		goto param_nest_cancel;

	for (i = 0; i <= DEVLINK_PARAM_CMODE_MAX; i++) {
		if (!devlink_param_cmode_is_supported(param, i))
			continue;
		err = devlink_nl_param_value_fill_one(msg, param->type,
						      i, param_value[i]);
		if (err)
			goto values_list_nest_cancel;
	}

	nla_nest_end(msg, param_values_list);
	nla_nest_end(msg, param_attr);
	genlmsg_end(msg, hdr);
	return 0;

values_list_nest_cancel:
	nla_nest_end(msg, param_values_list);
param_nest_cancel:
	nla_nest_cancel(msg, param_attr);
genlmsg_cancel:
	genlmsg_cancel(msg, hdr);
	return -EMSGSIZE;
}

static void devlink_param_notify(struct devlink *devlink,
				 struct devlink_param_item *param_item,
				 enum devlink_command cmd)
{
	struct sk_buff *msg;
	int err;

	WARN_ON(cmd != DEVLINK_CMD_PARAM_NEW && cmd != DEVLINK_CMD_PARAM_DEL);

	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!msg)
		return;
	err = devlink_nl_param_fill(msg, devlink, param_item, cmd, 0, 0, 0);
	if (err) {
		nlmsg_free(msg);
		return;
	}

	genlmsg_multicast_netns(&devlink_nl_family, devlink_net(devlink),
				msg, 0, DEVLINK_MCGRP_CONFIG, GFP_KERNEL);
}

static int devlink_nl_cmd_param_get_dumpit(struct sk_buff *msg,
					   struct netlink_callback *cb)
{
	struct devlink_param_item *param_item;
	struct devlink *devlink;
	int start = cb->args[0];
	int idx = 0;
	int err;

	mutex_lock(&devlink_mutex);
	list_for_each_entry(devlink, &devlink_list, list) {
		if (!net_eq(devlink_net(devlink), sock_net(msg->sk)))
			continue;
		mutex_lock(&devlink->lock);
		list_for_each_entry(param_item, &devlink->param_list, list) {
			if (idx < start) {
				idx++;
				continue;
			}
			err = devlink_nl_param_fill(msg, devlink, param_item,
						    DEVLINK_CMD_PARAM_GET,
						    NETLINK_CB(cb->skb).portid,
						    cb->nlh->nlmsg_seq,
						    NLM_F_MULTI);
			if (err) {
				mutex_unlock(&devlink->lock);
				goto out;
			}
			idx++;
		}
		mutex_unlock(&devlink->lock);
	}
out:
	mutex_unlock(&devlink_mutex);

	cb->args[0] = idx;
	return msg->len;
}

static int
devlink_param_type_get_from_info(struct genl_info *info,
				 enum devlink_param_type *param_type)
{
	if (!info->attrs[DEVLINK_ATTR_PARAM_TYPE])
		return -EINVAL;

	switch (nla_get_u8(info->attrs[DEVLINK_ATTR_PARAM_TYPE])) {
	case NLA_U8:
		*param_type = DEVLINK_PARAM_TYPE_U8;
		break;
	case NLA_U16:
		*param_type = DEVLINK_PARAM_TYPE_U16;
		break;
	case NLA_U32:
		*param_type = DEVLINK_PARAM_TYPE_U32;
		break;
	case NLA_STRING:
		*param_type = DEVLINK_PARAM_TYPE_STRING;
		break;
	case NLA_FLAG:
		*param_type = DEVLINK_PARAM_TYPE_BOOL;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int
devlink_param_value_get_from_info(const struct devlink_param *param,
				  struct genl_info *info,
				  union devlink_param_value *value)
{
	struct nlattr *param_data;
	int len;

	param_data = info->attrs[DEVLINK_ATTR_PARAM_VALUE_DATA];

	if (param->type != DEVLINK_PARAM_TYPE_BOOL && !param_data)
		return -EINVAL;

	switch (param->type) {
	case DEVLINK_PARAM_TYPE_U8:
		if (nla_len(param_data) != sizeof(u8))
			return -EINVAL;
		value->vu8 = nla_get_u8(param_data);
		break;
	case DEVLINK_PARAM_TYPE_U16:
		if (nla_len(param_data) != sizeof(u16))
			return -EINVAL;
		value->vu16 = nla_get_u16(param_data);
		break;
	case DEVLINK_PARAM_TYPE_U32:
		if (nla_len(param_data) != sizeof(u32))
			return -EINVAL;
		value->vu32 = nla_get_u32(param_data);
		break;
	case DEVLINK_PARAM_TYPE_STRING:
		len = strnlen(nla_data(param_data), nla_len(param_data));
		if (len == nla_len(param_data) ||
		    len >= __DEVLINK_PARAM_MAX_STRING_VALUE)
			return -EINVAL;
		strcpy(value->vstr, nla_data(param_data));
		break;
	case DEVLINK_PARAM_TYPE_BOOL:
		if (param_data && nla_len(param_data))
			return -EINVAL;
		value->vbool = nla_get_flag(param_data);
		break;
	}
	return 0;
}

static struct devlink_param_item *
devlink_param_get_from_info(struct devlink *devlink,
			    struct genl_info *info)
{
	char *param_name;

	if (!info->attrs[DEVLINK_ATTR_PARAM_NAME])
		return NULL;

	param_name = nla_data(info->attrs[DEVLINK_ATTR_PARAM_NAME]);
	return devlink_param_find_by_name(&devlink->param_list, param_name);
}

static int devlink_nl_cmd_param_get_doit(struct sk_buff *skb,
					 struct genl_info *info)
{
	struct devlink *devlink = info->user_ptr[0];
	struct devlink_param_item *param_item;
	struct sk_buff *msg;
	int err;

	param_item = devlink_param_get_from_info(devlink, info);
	if (!param_item)
		return -EINVAL;

	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!msg)
		return -ENOMEM;

	err = devlink_nl_param_fill(msg, devlink, param_item,
				    DEVLINK_CMD_PARAM_GET,
				    info->snd_portid, info->snd_seq, 0);
	if (err) {
		nlmsg_free(msg);
		return err;
	}

	return genlmsg_reply(msg, info);
}

static int devlink_nl_cmd_param_set_doit(struct sk_buff *skb,
					 struct genl_info *info)
{
	struct devlink *devlink = info->user_ptr[0];
	enum devlink_param_type param_type;
	struct devlink_param_gset_ctx ctx;
	enum devlink_param_cmode cmode;
	struct devlink_param_item *param_item;
	const struct devlink_param *param;
	union devlink_param_value value;
	int err = 0;

	param_item = devlink_param_get_from_info(devlink, info);
	if (!param_item)
		return -EINVAL;
	param = param_item->param;
	err = devlink_param_type_get_from_info(info, &param_type);
	if (err)
		return err;
	if (param_type != param->type)
		return -EINVAL;
	err = devlink_param_value_get_from_info(param, info, &value);
	if (err)
		return err;
	if (param->validate) {
		err = param->validate(devlink, param->id, value, info->extack);
		if (err)
			return err;
	}

	if (!info->attrs[DEVLINK_ATTR_PARAM_VALUE_CMODE])
		return -EINVAL;
	cmode = nla_get_u8(info->attrs[DEVLINK_ATTR_PARAM_VALUE_CMODE]);
	if (!devlink_param_cmode_is_supported(param, cmode))
		return -EOPNOTSUPP;

	if (cmode == DEVLINK_PARAM_CMODE_DRIVERINIT) {
		if (param->type == DEVLINK_PARAM_TYPE_STRING)
			strcpy(param_item->driverinit_value.vstr, value.vstr);
		else
			param_item->driverinit_value = value;
		param_item->driverinit_value_valid = true;
	} else {
		if (!param->set)
			return -EOPNOTSUPP;
		ctx.val = value;
		ctx.cmode = cmode;
		err = devlink_param_set(devlink, param, &ctx);
		if (err)
			return err;
	}

	devlink_param_notify(devlink, param_item, DEVLINK_CMD_PARAM_NEW);
	return 0;
}

static int devlink_param_register_one(struct devlink *devlink,
				      const struct devlink_param *param)
{
	struct devlink_param_item *param_item;

	if (devlink_param_find_by_name(&devlink->param_list,
				       param->name))
		return -EEXIST;

	if (param->supported_cmodes == BIT(DEVLINK_PARAM_CMODE_DRIVERINIT))
		WARN_ON(param->get || param->set);
	else
		WARN_ON(!param->get || !param->set);

	param_item = kzalloc(sizeof(*param_item), GFP_KERNEL);
	if (!param_item)
		return -ENOMEM;
	param_item->param = param;

	list_add_tail(&param_item->list, &devlink->param_list);
	devlink_param_notify(devlink, param_item, DEVLINK_CMD_PARAM_NEW);
	return 0;
}

static void devlink_param_unregister_one(struct devlink *devlink,
					 const struct devlink_param *param)
{
	struct devlink_param_item *param_item;

	param_item = devlink_param_find_by_name(&devlink->param_list,
						param->name);
	WARN_ON(!param_item);
	devlink_param_notify(devlink, param_item, DEVLINK_CMD_PARAM_DEL);
	list_del(&param_item->list);
	kfree(param_item);
}

static int devlink_nl_region_snapshot_id_put(struct sk_buff *msg,
					     struct devlink *devlink,
					     struct devlink_snapshot *snapshot)
{
	struct nlattr *snap_attr;
	int err;

	snap_attr = nla_nest_start(msg, DEVLINK_ATTR_REGION_SNAPSHOT);
	if (!snap_attr)
		return -EINVAL;

	err = nla_put_u32(msg, DEVLINK_ATTR_REGION_SNAPSHOT_ID, snapshot->id);
	if (err)
		goto nla_put_failure;

	nla_nest_end(msg, snap_attr);
	return 0;

nla_put_failure:
	nla_nest_cancel(msg, snap_attr);
	return err;
}

static int devlink_nl_region_snapshots_id_put(struct sk_buff *msg,
					      struct devlink *devlink,
					      struct devlink_region *region)
{
	struct devlink_snapshot *snapshot;
	struct nlattr *snapshots_attr;
	int err;

	snapshots_attr = nla_nest_start(msg, DEVLINK_ATTR_REGION_SNAPSHOTS);
	if (!snapshots_attr)
		return -EINVAL;

	list_for_each_entry(snapshot, &region->snapshot_list, list) {
		err = devlink_nl_region_snapshot_id_put(msg, devlink, snapshot);
		if (err)
			goto nla_put_failure;
	}

	nla_nest_end(msg, snapshots_attr);
	return 0;

nla_put_failure:
	nla_nest_cancel(msg, snapshots_attr);
	return err;
}

static int devlink_nl_region_fill(struct sk_buff *msg, struct devlink *devlink,
				  enum devlink_command cmd, u32 portid,
				  u32 seq, int flags,
				  struct devlink_region *region)
{
	void *hdr;
	int err;

	hdr = genlmsg_put(msg, portid, seq, &devlink_nl_family, flags, cmd);
	if (!hdr)
		return -EMSGSIZE;

	err = devlink_nl_put_handle(msg, devlink);
	if (err)
		goto nla_put_failure;

	err = nla_put_string(msg, DEVLINK_ATTR_REGION_NAME, region->name);
	if (err)
		goto nla_put_failure;

	err = nla_put_u64_64bit(msg, DEVLINK_ATTR_REGION_SIZE,
				region->size,
				DEVLINK_ATTR_PAD);
	if (err)
		goto nla_put_failure;

	err = devlink_nl_region_snapshots_id_put(msg, devlink, region);
	if (err)
		goto nla_put_failure;

	genlmsg_end(msg, hdr);
	return 0;

nla_put_failure:
	genlmsg_cancel(msg, hdr);
	return err;
}

static void devlink_nl_region_notify(struct devlink_region *region,
				     struct devlink_snapshot *snapshot,
				     enum devlink_command cmd)
{
	struct devlink *devlink = region->devlink;
	struct sk_buff *msg;
	void *hdr;
	int err;

	WARN_ON(cmd != DEVLINK_CMD_REGION_NEW && cmd != DEVLINK_CMD_REGION_DEL);

	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!msg)
		return;

	hdr = genlmsg_put(msg, 0, 0, &devlink_nl_family, 0, cmd);
	if (!hdr)
		goto out_free_msg;

	err = devlink_nl_put_handle(msg, devlink);
	if (err)
		goto out_cancel_msg;

	err = nla_put_string(msg, DEVLINK_ATTR_REGION_NAME,
			     region->name);
	if (err)
		goto out_cancel_msg;

	if (snapshot) {
		err = nla_put_u32(msg, DEVLINK_ATTR_REGION_SNAPSHOT_ID,
				  snapshot->id);
		if (err)
			goto out_cancel_msg;
	} else {
		err = nla_put_u64_64bit(msg, DEVLINK_ATTR_REGION_SIZE,
					region->size, DEVLINK_ATTR_PAD);
		if (err)
			goto out_cancel_msg;
	}
	genlmsg_end(msg, hdr);

	genlmsg_multicast_netns(&devlink_nl_family, devlink_net(devlink),
				msg, 0, DEVLINK_MCGRP_CONFIG, GFP_KERNEL);

	return;

out_cancel_msg:
	genlmsg_cancel(msg, hdr);
out_free_msg:
	nlmsg_free(msg);
}

static int devlink_nl_cmd_region_get_doit(struct sk_buff *skb,
					  struct genl_info *info)
{
	struct devlink *devlink = info->user_ptr[0];
	struct devlink_region *region;
	const char *region_name;
	struct sk_buff *msg;
	int err;

	if (!info->attrs[DEVLINK_ATTR_REGION_NAME])
		return -EINVAL;

	region_name = nla_data(info->attrs[DEVLINK_ATTR_REGION_NAME]);
	region = devlink_region_get_by_name(devlink, region_name);
	if (!region)
		return -EINVAL;

	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!msg)
		return -ENOMEM;

	err = devlink_nl_region_fill(msg, devlink, DEVLINK_CMD_REGION_GET,
				     info->snd_portid, info->snd_seq, 0,
				     region);
	if (err) {
		nlmsg_free(msg);
		return err;
	}

	return genlmsg_reply(msg, info);
}

static int devlink_nl_cmd_region_get_dumpit(struct sk_buff *msg,
					    struct netlink_callback *cb)
{
	struct devlink_region *region;
	struct devlink *devlink;
	int start = cb->args[0];
	int idx = 0;
	int err;

	mutex_lock(&devlink_mutex);
	list_for_each_entry(devlink, &devlink_list, list) {
		if (!net_eq(devlink_net(devlink), sock_net(msg->sk)))
			continue;

		mutex_lock(&devlink->lock);
		list_for_each_entry(region, &devlink->region_list, list) {
			if (idx < start) {
				idx++;
				continue;
			}
			err = devlink_nl_region_fill(msg, devlink,
						     DEVLINK_CMD_REGION_GET,
						     NETLINK_CB(cb->skb).portid,
						     cb->nlh->nlmsg_seq,
						     NLM_F_MULTI, region);
			if (err) {
				mutex_unlock(&devlink->lock);
				goto out;
			}
			idx++;
		}
		mutex_unlock(&devlink->lock);
	}
out:
	mutex_unlock(&devlink_mutex);
	cb->args[0] = idx;
	return msg->len;
}

static int devlink_nl_cmd_region_del(struct sk_buff *skb,
				     struct genl_info *info)
{
	struct devlink *devlink = info->user_ptr[0];
	struct devlink_snapshot *snapshot;
	struct devlink_region *region;
	const char *region_name;
	u32 snapshot_id;

	if (!info->attrs[DEVLINK_ATTR_REGION_NAME] ||
	    !info->attrs[DEVLINK_ATTR_REGION_SNAPSHOT_ID])
		return -EINVAL;

	region_name = nla_data(info->attrs[DEVLINK_ATTR_REGION_NAME]);
	snapshot_id = nla_get_u32(info->attrs[DEVLINK_ATTR_REGION_SNAPSHOT_ID]);

	region = devlink_region_get_by_name(devlink, region_name);
	if (!region)
		return -EINVAL;

	snapshot = devlink_region_snapshot_get_by_id(region, snapshot_id);
	if (!snapshot)
		return -EINVAL;

	devlink_nl_region_notify(region, snapshot, DEVLINK_CMD_REGION_DEL);
	devlink_region_snapshot_del(snapshot);
	return 0;
}

static int devlink_nl_cmd_region_read_chunk_fill(struct sk_buff *msg,
						 struct devlink *devlink,
						 u8 *chunk, u32 chunk_size,
						 u64 addr)
{
	struct nlattr *chunk_attr;
	int err;

	chunk_attr = nla_nest_start(msg, DEVLINK_ATTR_REGION_CHUNK);
	if (!chunk_attr)
		return -EINVAL;

	err = nla_put(msg, DEVLINK_ATTR_REGION_CHUNK_DATA, chunk_size, chunk);
	if (err)
		goto nla_put_failure;

	err = nla_put_u64_64bit(msg, DEVLINK_ATTR_REGION_CHUNK_ADDR, addr,
				DEVLINK_ATTR_PAD);
	if (err)
		goto nla_put_failure;

	nla_nest_end(msg, chunk_attr);
	return 0;

nla_put_failure:
	nla_nest_cancel(msg, chunk_attr);
	return err;
}

#define DEVLINK_REGION_READ_CHUNK_SIZE 256

static int devlink_nl_region_read_snapshot_fill(struct sk_buff *skb,
						struct devlink *devlink,
						struct devlink_region *region,
						struct nlattr **attrs,
						u64 start_offset,
						u64 end_offset,
						bool dump,
						u64 *new_offset)
{
	struct devlink_snapshot *snapshot;
	u64 curr_offset = start_offset;
	u32 snapshot_id;
	int err = 0;

	*new_offset = start_offset;

	snapshot_id = nla_get_u32(attrs[DEVLINK_ATTR_REGION_SNAPSHOT_ID]);
	snapshot = devlink_region_snapshot_get_by_id(region, snapshot_id);
	if (!snapshot)
		return -EINVAL;

	if (end_offset > snapshot->data_len || dump)
		end_offset = snapshot->data_len;

	while (curr_offset < end_offset) {
		u32 data_size;
		u8 *data;

		if (end_offset - curr_offset < DEVLINK_REGION_READ_CHUNK_SIZE)
			data_size = end_offset - curr_offset;
		else
			data_size = DEVLINK_REGION_READ_CHUNK_SIZE;

		data = &snapshot->data[curr_offset];
		err = devlink_nl_cmd_region_read_chunk_fill(skb, devlink,
							    data, data_size,
							    curr_offset);
		if (err)
			break;

		curr_offset += data_size;
	}
	*new_offset = curr_offset;

	return err;
}

static int devlink_nl_cmd_region_read_dumpit(struct sk_buff *skb,
					     struct netlink_callback *cb)
{
	u64 ret_offset, start_offset, end_offset = 0;
	struct nlattr *attrs[DEVLINK_ATTR_MAX + 1];
	const struct genl_ops *ops = cb->data;
	struct devlink_region *region;
	struct nlattr *chunks_attr;
	const char *region_name;
	struct devlink *devlink;
	bool dump = true;
	void *hdr;
	int err;

	start_offset = *((u64 *)&cb->args[0]);

	err = nlmsg_parse(cb->nlh, GENL_HDRLEN + devlink_nl_family.hdrsize,
			  attrs, DEVLINK_ATTR_MAX, ops->policy, NULL);
	if (err)
		goto out;

	devlink = devlink_get_from_attrs(sock_net(cb->skb->sk), attrs);
	if (IS_ERR(devlink))
		goto out;

	mutex_lock(&devlink_mutex);
	mutex_lock(&devlink->lock);

	if (!attrs[DEVLINK_ATTR_REGION_NAME] ||
	    !attrs[DEVLINK_ATTR_REGION_SNAPSHOT_ID])
		goto out_unlock;

	region_name = nla_data(attrs[DEVLINK_ATTR_REGION_NAME]);
	region = devlink_region_get_by_name(devlink, region_name);
	if (!region)
		goto out_unlock;

	hdr = genlmsg_put(skb, NETLINK_CB(cb->skb).portid, cb->nlh->nlmsg_seq,
			  &devlink_nl_family, NLM_F_ACK | NLM_F_MULTI,
			  DEVLINK_CMD_REGION_READ);
	if (!hdr)
		goto out_unlock;

	err = devlink_nl_put_handle(skb, devlink);
	if (err)
		goto nla_put_failure;

	err = nla_put_string(skb, DEVLINK_ATTR_REGION_NAME, region_name);
	if (err)
		goto nla_put_failure;

	chunks_attr = nla_nest_start(skb, DEVLINK_ATTR_REGION_CHUNKS);
	if (!chunks_attr)
		goto nla_put_failure;

	if (attrs[DEVLINK_ATTR_REGION_CHUNK_ADDR] &&
	    attrs[DEVLINK_ATTR_REGION_CHUNK_LEN]) {
		if (!start_offset)
			start_offset =
				nla_get_u64(attrs[DEVLINK_ATTR_REGION_CHUNK_ADDR]);

		end_offset = nla_get_u64(attrs[DEVLINK_ATTR_REGION_CHUNK_ADDR]);
		end_offset += nla_get_u64(attrs[DEVLINK_ATTR_REGION_CHUNK_LEN]);
		dump = false;
	}

	err = devlink_nl_region_read_snapshot_fill(skb, devlink,
						   region, attrs,
						   start_offset,
						   end_offset, dump,
						   &ret_offset);

	if (err && err != -EMSGSIZE)
		goto nla_put_failure;

	/* Check if there was any progress done to prevent infinite loop */
	if (ret_offset == start_offset)
		goto nla_put_failure;

	*((u64 *)&cb->args[0]) = ret_offset;

	nla_nest_end(skb, chunks_attr);
	genlmsg_end(skb, hdr);
	mutex_unlock(&devlink->lock);
	mutex_unlock(&devlink_mutex);

	return skb->len;

nla_put_failure:
	genlmsg_cancel(skb, hdr);
out_unlock:
	mutex_unlock(&devlink->lock);
	mutex_unlock(&devlink_mutex);
out:
	return 0;
}

static const struct nla_policy devlink_nl_policy[DEVLINK_ATTR_MAX + 1] = {
	[DEVLINK_ATTR_BUS_NAME] = { .type = NLA_NUL_STRING },
	[DEVLINK_ATTR_DEV_NAME] = { .type = NLA_NUL_STRING },
	[DEVLINK_ATTR_PORT_INDEX] = { .type = NLA_U32 },
	[DEVLINK_ATTR_PORT_TYPE] = { .type = NLA_U16 },
	[DEVLINK_ATTR_PORT_SPLIT_COUNT] = { .type = NLA_U32 },
	[DEVLINK_ATTR_SB_INDEX] = { .type = NLA_U32 },
	[DEVLINK_ATTR_SB_POOL_INDEX] = { .type = NLA_U16 },
	[DEVLINK_ATTR_SB_POOL_TYPE] = { .type = NLA_U8 },
	[DEVLINK_ATTR_SB_POOL_SIZE] = { .type = NLA_U32 },
	[DEVLINK_ATTR_SB_POOL_THRESHOLD_TYPE] = { .type = NLA_U8 },
	[DEVLINK_ATTR_SB_THRESHOLD] = { .type = NLA_U32 },
	[DEVLINK_ATTR_SB_TC_INDEX] = { .type = NLA_U16 },
	[DEVLINK_ATTR_ESWITCH_MODE] = { .type = NLA_U16 },
	[DEVLINK_ATTR_ESWITCH_INLINE_MODE] = { .type = NLA_U8 },
	[DEVLINK_ATTR_ESWITCH_ENCAP_MODE] = { .type = NLA_U8 },
	[DEVLINK_ATTR_DPIPE_TABLE_NAME] = { .type = NLA_NUL_STRING },
	[DEVLINK_ATTR_DPIPE_TABLE_COUNTERS_ENABLED] = { .type = NLA_U8 },
	[DEVLINK_ATTR_RESOURCE_ID] = { .type = NLA_U64},
	[DEVLINK_ATTR_RESOURCE_SIZE] = { .type = NLA_U64},
	[DEVLINK_ATTR_PARAM_NAME] = { .type = NLA_NUL_STRING },
	[DEVLINK_ATTR_PARAM_TYPE] = { .type = NLA_U8 },
	[DEVLINK_ATTR_PARAM_VALUE_CMODE] = { .type = NLA_U8 },
	[DEVLINK_ATTR_REGION_NAME] = { .type = NLA_NUL_STRING },
	[DEVLINK_ATTR_REGION_SNAPSHOT_ID] = { .type = NLA_U32 },
	[DEVLINK_ATTR_REGION_CHUNK_ADDR] = { .type = NLA_U64 },
	[DEVLINK_ATTR_REGION_CHUNK_LEN] = { .type = NLA_U64 },
};

static const struct genl_ops devlink_nl_ops[] = {
	{
		.cmd = DEVLINK_CMD_GET,
		.doit = devlink_nl_cmd_get_doit,
		.dumpit = devlink_nl_cmd_get_dumpit,
		.policy = devlink_nl_policy,
		.internal_flags = DEVLINK_NL_FLAG_NEED_DEVLINK,
		/* can be retrieved by unprivileged users */
	},
	{
		.cmd = DEVLINK_CMD_PORT_GET,
		.doit = devlink_nl_cmd_port_get_doit,
		.dumpit = devlink_nl_cmd_port_get_dumpit,
		.policy = devlink_nl_policy,
		.internal_flags = DEVLINK_NL_FLAG_NEED_PORT,
		/* can be retrieved by unprivileged users */
	},
	{
		.cmd = DEVLINK_CMD_PORT_SET,
		.doit = devlink_nl_cmd_port_set_doit,
		.policy = devlink_nl_policy,
		.flags = GENL_ADMIN_PERM,
		.internal_flags = DEVLINK_NL_FLAG_NEED_PORT,
	},
	{
		.cmd = DEVLINK_CMD_PORT_SPLIT,
		.doit = devlink_nl_cmd_port_split_doit,
		.policy = devlink_nl_policy,
		.flags = GENL_ADMIN_PERM,
		.internal_flags = DEVLINK_NL_FLAG_NEED_DEVLINK |
				  DEVLINK_NL_FLAG_NO_LOCK,
	},
	{
		.cmd = DEVLINK_CMD_PORT_UNSPLIT,
		.doit = devlink_nl_cmd_port_unsplit_doit,
		.policy = devlink_nl_policy,
		.flags = GENL_ADMIN_PERM,
		.internal_flags = DEVLINK_NL_FLAG_NEED_DEVLINK |
				  DEVLINK_NL_FLAG_NO_LOCK,
	},
	{
		.cmd = DEVLINK_CMD_SB_GET,
		.doit = devlink_nl_cmd_sb_get_doit,
		.dumpit = devlink_nl_cmd_sb_get_dumpit,
		.policy = devlink_nl_policy,
		.internal_flags = DEVLINK_NL_FLAG_NEED_DEVLINK |
				  DEVLINK_NL_FLAG_NEED_SB,
		/* can be retrieved by unprivileged users */
	},
	{
		.cmd = DEVLINK_CMD_SB_POOL_GET,
		.doit = devlink_nl_cmd_sb_pool_get_doit,
		.dumpit = devlink_nl_cmd_sb_pool_get_dumpit,
		.policy = devlink_nl_policy,
		.internal_flags = DEVLINK_NL_FLAG_NEED_DEVLINK |
				  DEVLINK_NL_FLAG_NEED_SB,
		/* can be retrieved by unprivileged users */
	},
	{
		.cmd = DEVLINK_CMD_SB_POOL_SET,
		.doit = devlink_nl_cmd_sb_pool_set_doit,
		.policy = devlink_nl_policy,
		.flags = GENL_ADMIN_PERM,
		.internal_flags = DEVLINK_NL_FLAG_NEED_DEVLINK |
				  DEVLINK_NL_FLAG_NEED_SB,
	},
	{
		.cmd = DEVLINK_CMD_SB_PORT_POOL_GET,
		.doit = devlink_nl_cmd_sb_port_pool_get_doit,
		.dumpit = devlink_nl_cmd_sb_port_pool_get_dumpit,
		.policy = devlink_nl_policy,
		.internal_flags = DEVLINK_NL_FLAG_NEED_PORT |
				  DEVLINK_NL_FLAG_NEED_SB,
		/* can be retrieved by unprivileged users */
	},
	{
		.cmd = DEVLINK_CMD_SB_PORT_POOL_SET,
		.doit = devlink_nl_cmd_sb_port_pool_set_doit,
		.policy = devlink_nl_policy,
		.flags = GENL_ADMIN_PERM,
		.internal_flags = DEVLINK_NL_FLAG_NEED_PORT |
				  DEVLINK_NL_FLAG_NEED_SB,
	},
	{
		.cmd = DEVLINK_CMD_SB_TC_POOL_BIND_GET,
		.doit = devlink_nl_cmd_sb_tc_pool_bind_get_doit,
		.dumpit = devlink_nl_cmd_sb_tc_pool_bind_get_dumpit,
		.policy = devlink_nl_policy,
		.internal_flags = DEVLINK_NL_FLAG_NEED_PORT |
				  DEVLINK_NL_FLAG_NEED_SB,
		/* can be retrieved by unprivileged users */
	},
	{
		.cmd = DEVLINK_CMD_SB_TC_POOL_BIND_SET,
		.doit = devlink_nl_cmd_sb_tc_pool_bind_set_doit,
		.policy = devlink_nl_policy,
		.flags = GENL_ADMIN_PERM,
		.internal_flags = DEVLINK_NL_FLAG_NEED_PORT |
				  DEVLINK_NL_FLAG_NEED_SB,
	},
	{
		.cmd = DEVLINK_CMD_SB_OCC_SNAPSHOT,
		.doit = devlink_nl_cmd_sb_occ_snapshot_doit,
		.policy = devlink_nl_policy,
		.flags = GENL_ADMIN_PERM,
		.internal_flags = DEVLINK_NL_FLAG_NEED_DEVLINK |
				  DEVLINK_NL_FLAG_NEED_SB,
	},
	{
		.cmd = DEVLINK_CMD_SB_OCC_MAX_CLEAR,
		.doit = devlink_nl_cmd_sb_occ_max_clear_doit,
		.policy = devlink_nl_policy,
		.flags = GENL_ADMIN_PERM,
		.internal_flags = DEVLINK_NL_FLAG_NEED_DEVLINK |
				  DEVLINK_NL_FLAG_NEED_SB,
	},
	{
		.cmd = DEVLINK_CMD_ESWITCH_GET,
		.doit = devlink_nl_cmd_eswitch_get_doit,
		.policy = devlink_nl_policy,
		.flags = GENL_ADMIN_PERM,
		.internal_flags = DEVLINK_NL_FLAG_NEED_DEVLINK,
	},
	{
		.cmd = DEVLINK_CMD_ESWITCH_SET,
		.doit = devlink_nl_cmd_eswitch_set_doit,
		.policy = devlink_nl_policy,
		.flags = GENL_ADMIN_PERM,
		.internal_flags = DEVLINK_NL_FLAG_NEED_DEVLINK |
				  DEVLINK_NL_FLAG_NO_LOCK,
	},
	{
		.cmd = DEVLINK_CMD_DPIPE_TABLE_GET,
		.doit = devlink_nl_cmd_dpipe_table_get,
		.policy = devlink_nl_policy,
		.internal_flags = DEVLINK_NL_FLAG_NEED_DEVLINK,
		/* can be retrieved by unprivileged users */
	},
	{
		.cmd = DEVLINK_CMD_DPIPE_ENTRIES_GET,
		.doit = devlink_nl_cmd_dpipe_entries_get,
		.policy = devlink_nl_policy,
		.internal_flags = DEVLINK_NL_FLAG_NEED_DEVLINK,
		/* can be retrieved by unprivileged users */
	},
	{
		.cmd = DEVLINK_CMD_DPIPE_HEADERS_GET,
		.doit = devlink_nl_cmd_dpipe_headers_get,
		.policy = devlink_nl_policy,
		.internal_flags = DEVLINK_NL_FLAG_NEED_DEVLINK,
		/* can be retrieved by unprivileged users */
	},
	{
		.cmd = DEVLINK_CMD_DPIPE_TABLE_COUNTERS_SET,
		.doit = devlink_nl_cmd_dpipe_table_counters_set,
		.policy = devlink_nl_policy,
		.flags = GENL_ADMIN_PERM,
		.internal_flags = DEVLINK_NL_FLAG_NEED_DEVLINK,
	},
	{
		.cmd = DEVLINK_CMD_RESOURCE_SET,
		.doit = devlink_nl_cmd_resource_set,
		.policy = devlink_nl_policy,
		.flags = GENL_ADMIN_PERM,
		.internal_flags = DEVLINK_NL_FLAG_NEED_DEVLINK,
	},
	{
		.cmd = DEVLINK_CMD_RESOURCE_DUMP,
		.doit = devlink_nl_cmd_resource_dump,
		.policy = devlink_nl_policy,
		.internal_flags = DEVLINK_NL_FLAG_NEED_DEVLINK,
		/* can be retrieved by unprivileged users */
	},
	{
		.cmd = DEVLINK_CMD_RELOAD,
		.doit = devlink_nl_cmd_reload,
		.policy = devlink_nl_policy,
		.flags = GENL_ADMIN_PERM,
		.internal_flags = DEVLINK_NL_FLAG_NEED_DEVLINK |
				  DEVLINK_NL_FLAG_NO_LOCK,
	},
	{
		.cmd = DEVLINK_CMD_PARAM_GET,
		.doit = devlink_nl_cmd_param_get_doit,
		.dumpit = devlink_nl_cmd_param_get_dumpit,
		.policy = devlink_nl_policy,
		.internal_flags = DEVLINK_NL_FLAG_NEED_DEVLINK,
		/* can be retrieved by unprivileged users */
	},
	{
		.cmd = DEVLINK_CMD_PARAM_SET,
		.doit = devlink_nl_cmd_param_set_doit,
		.policy = devlink_nl_policy,
		.flags = GENL_ADMIN_PERM,
		.internal_flags = DEVLINK_NL_FLAG_NEED_DEVLINK,
	},
	{
		.cmd = DEVLINK_CMD_REGION_GET,
		.doit = devlink_nl_cmd_region_get_doit,
		.dumpit = devlink_nl_cmd_region_get_dumpit,
		.policy = devlink_nl_policy,
		.flags = GENL_ADMIN_PERM,
		.internal_flags = DEVLINK_NL_FLAG_NEED_DEVLINK,
	},
	{
		.cmd = DEVLINK_CMD_REGION_DEL,
		.doit = devlink_nl_cmd_region_del,
		.policy = devlink_nl_policy,
		.flags = GENL_ADMIN_PERM,
		.internal_flags = DEVLINK_NL_FLAG_NEED_DEVLINK,
	},
	{
		.cmd = DEVLINK_CMD_REGION_READ,
		.dumpit = devlink_nl_cmd_region_read_dumpit,
		.policy = devlink_nl_policy,
		.flags = GENL_ADMIN_PERM,
		.internal_flags = DEVLINK_NL_FLAG_NEED_DEVLINK,
	},
};

static struct genl_family devlink_nl_family __ro_after_init = {
	.name		= DEVLINK_GENL_NAME,
	.version	= DEVLINK_GENL_VERSION,
	.maxattr	= DEVLINK_ATTR_MAX,
	.netnsok	= true,
	.pre_doit	= devlink_nl_pre_doit,
	.post_doit	= devlink_nl_post_doit,
	.module		= THIS_MODULE,
	.ops		= devlink_nl_ops,
	.n_ops		= ARRAY_SIZE(devlink_nl_ops),
	.mcgrps		= devlink_nl_mcgrps,
	.n_mcgrps	= ARRAY_SIZE(devlink_nl_mcgrps),
};

/**
 *	devlink_alloc - Allocate new devlink instance resources
 *
 *	@ops: ops
 *	@priv_size: size of user private data
 *
 *	Allocate new devlink instance resources, including devlink index
 *	and name.
 */
struct devlink *devlink_alloc(const struct devlink_ops *ops, size_t priv_size)
{
	struct devlink *devlink;

	devlink = kzalloc(sizeof(*devlink) + priv_size, GFP_KERNEL);
	if (!devlink)
		return NULL;
	devlink->ops = ops;
	devlink_net_set(devlink, &init_net);
	INIT_LIST_HEAD(&devlink->port_list);
	INIT_LIST_HEAD(&devlink->sb_list);
	INIT_LIST_HEAD_RCU(&devlink->dpipe_table_list);
	INIT_LIST_HEAD(&devlink->resource_list);
	INIT_LIST_HEAD(&devlink->param_list);
	INIT_LIST_HEAD(&devlink->region_list);
	mutex_init(&devlink->lock);
	return devlink;
}
EXPORT_SYMBOL_GPL(devlink_alloc);

/**
 *	devlink_register - Register devlink instance
 *
 *	@devlink: devlink
 */
int devlink_register(struct devlink *devlink, struct device *dev)
{
	mutex_lock(&devlink_mutex);
	devlink->dev = dev;
	list_add_tail(&devlink->list, &devlink_list);
	devlink_notify(devlink, DEVLINK_CMD_NEW);
	mutex_unlock(&devlink_mutex);
	return 0;
}
EXPORT_SYMBOL_GPL(devlink_register);

/**
 *	devlink_unregister - Unregister devlink instance
 *
 *	@devlink: devlink
 */
void devlink_unregister(struct devlink *devlink)
{
	mutex_lock(&devlink_mutex);
	devlink_notify(devlink, DEVLINK_CMD_DEL);
	list_del(&devlink->list);
	mutex_unlock(&devlink_mutex);
}
EXPORT_SYMBOL_GPL(devlink_unregister);

/**
 *	devlink_free - Free devlink instance resources
 *
 *	@devlink: devlink
 */
void devlink_free(struct devlink *devlink)
{
	kfree(devlink);
}
EXPORT_SYMBOL_GPL(devlink_free);

/**
 *	devlink_port_register - Register devlink port
 *
 *	@devlink: devlink
 *	@devlink_port: devlink port
 *	@port_index
 *
 *	Register devlink port with provided port index. User can use
 *	any indexing, even hw-related one. devlink_port structure
 *	is convenient to be embedded inside user driver private structure.
 *	Note that the caller should take care of zeroing the devlink_port
 *	structure.
 */
int devlink_port_register(struct devlink *devlink,
			  struct devlink_port *devlink_port,
			  unsigned int port_index)
{
	mutex_lock(&devlink->lock);
	if (devlink_port_index_exists(devlink, port_index)) {
		mutex_unlock(&devlink->lock);
		return -EEXIST;
	}
	devlink_port->devlink = devlink;
	devlink_port->index = port_index;
	devlink_port->registered = true;
	list_add_tail(&devlink_port->list, &devlink->port_list);
	mutex_unlock(&devlink->lock);
	devlink_port_notify(devlink_port, DEVLINK_CMD_PORT_NEW);
	return 0;
}
EXPORT_SYMBOL_GPL(devlink_port_register);

/**
 *	devlink_port_unregister - Unregister devlink port
 *
 *	@devlink_port: devlink port
 */
void devlink_port_unregister(struct devlink_port *devlink_port)
{
	struct devlink *devlink = devlink_port->devlink;

	devlink_port_notify(devlink_port, DEVLINK_CMD_PORT_DEL);
	mutex_lock(&devlink->lock);
	list_del(&devlink_port->list);
	mutex_unlock(&devlink->lock);
}
EXPORT_SYMBOL_GPL(devlink_port_unregister);

static void __devlink_port_type_set(struct devlink_port *devlink_port,
				    enum devlink_port_type type,
				    void *type_dev)
{
	devlink_port->type = type;
	devlink_port->type_dev = type_dev;
	devlink_port_notify(devlink_port, DEVLINK_CMD_PORT_NEW);
}

/**
 *	devlink_port_type_eth_set - Set port type to Ethernet
 *
 *	@devlink_port: devlink port
 *	@netdev: related netdevice
 */
void devlink_port_type_eth_set(struct devlink_port *devlink_port,
			       struct net_device *netdev)
{
	return __devlink_port_type_set(devlink_port,
				       DEVLINK_PORT_TYPE_ETH, netdev);
}
EXPORT_SYMBOL_GPL(devlink_port_type_eth_set);

/**
 *	devlink_port_type_ib_set - Set port type to InfiniBand
 *
 *	@devlink_port: devlink port
 *	@ibdev: related IB device
 */
void devlink_port_type_ib_set(struct devlink_port *devlink_port,
			      struct ib_device *ibdev)
{
	return __devlink_port_type_set(devlink_port,
				       DEVLINK_PORT_TYPE_IB, ibdev);
}
EXPORT_SYMBOL_GPL(devlink_port_type_ib_set);

/**
 *	devlink_port_type_clear - Clear port type
 *
 *	@devlink_port: devlink port
 */
void devlink_port_type_clear(struct devlink_port *devlink_port)
{
	return __devlink_port_type_set(devlink_port,
				       DEVLINK_PORT_TYPE_NOTSET, NULL);
}
EXPORT_SYMBOL_GPL(devlink_port_type_clear);

/**
 *	devlink_port_attrs_set - Set port attributes
 *
 *	@devlink_port: devlink port
 *	@flavour: flavour of the port
 *	@port_number: number of the port that is facing user, for example
 *	              the front panel port number
 *	@split: indicates if this is split port
 *	@split_subport_number: if the port is split, this is the number
 *	                       of subport.
 */
void devlink_port_attrs_set(struct devlink_port *devlink_port,
			    enum devlink_port_flavour flavour,
			    u32 port_number, bool split,
			    u32 split_subport_number)
{
	struct devlink_port_attrs *attrs = &devlink_port->attrs;

	attrs->set = true;
	attrs->flavour = flavour;
	attrs->port_number = port_number;
	attrs->split = split;
	attrs->split_subport_number = split_subport_number;
	devlink_port_notify(devlink_port, DEVLINK_CMD_PORT_NEW);
}
EXPORT_SYMBOL_GPL(devlink_port_attrs_set);

int devlink_port_get_phys_port_name(struct devlink_port *devlink_port,
				    char *name, size_t len)
{
	struct devlink_port_attrs *attrs = &devlink_port->attrs;
	int n = 0;

	if (!attrs->set)
		return -EOPNOTSUPP;

	switch (attrs->flavour) {
	case DEVLINK_PORT_FLAVOUR_PHYSICAL:
		if (!attrs->split)
			n = snprintf(name, len, "p%u", attrs->port_number);
		else
			n = snprintf(name, len, "p%us%u", attrs->port_number,
				     attrs->split_subport_number);
		break;
	case DEVLINK_PORT_FLAVOUR_CPU:
	case DEVLINK_PORT_FLAVOUR_DSA:
		/* As CPU and DSA ports do not have a netdevice associated
		 * case should not ever happen.
		 */
		WARN_ON(1);
		return -EINVAL;
	}

	if (n >= len)
		return -EINVAL;

	return 0;
}
EXPORT_SYMBOL_GPL(devlink_port_get_phys_port_name);

int devlink_sb_register(struct devlink *devlink, unsigned int sb_index,
			u32 size, u16 ingress_pools_count,
			u16 egress_pools_count, u16 ingress_tc_count,
			u16 egress_tc_count)
{
	struct devlink_sb *devlink_sb;
	int err = 0;

	mutex_lock(&devlink->lock);
	if (devlink_sb_index_exists(devlink, sb_index)) {
		err = -EEXIST;
		goto unlock;
	}

	devlink_sb = kzalloc(sizeof(*devlink_sb), GFP_KERNEL);
	if (!devlink_sb) {
		err = -ENOMEM;
		goto unlock;
	}
	devlink_sb->index = sb_index;
	devlink_sb->size = size;
	devlink_sb->ingress_pools_count = ingress_pools_count;
	devlink_sb->egress_pools_count = egress_pools_count;
	devlink_sb->ingress_tc_count = ingress_tc_count;
	devlink_sb->egress_tc_count = egress_tc_count;
	list_add_tail(&devlink_sb->list, &devlink->sb_list);
unlock:
	mutex_unlock(&devlink->lock);
	return err;
}
EXPORT_SYMBOL_GPL(devlink_sb_register);

void devlink_sb_unregister(struct devlink *devlink, unsigned int sb_index)
{
	struct devlink_sb *devlink_sb;

	mutex_lock(&devlink->lock);
	devlink_sb = devlink_sb_get_by_index(devlink, sb_index);
	WARN_ON(!devlink_sb);
	list_del(&devlink_sb->list);
	mutex_unlock(&devlink->lock);
	kfree(devlink_sb);
}
EXPORT_SYMBOL_GPL(devlink_sb_unregister);

/**
 *	devlink_dpipe_headers_register - register dpipe headers
 *
 *	@devlink: devlink
 *	@dpipe_headers: dpipe header array
 *
 *	Register the headers supported by hardware.
 */
int devlink_dpipe_headers_register(struct devlink *devlink,
				   struct devlink_dpipe_headers *dpipe_headers)
{
	mutex_lock(&devlink->lock);
	devlink->dpipe_headers = dpipe_headers;
	mutex_unlock(&devlink->lock);
	return 0;
}
EXPORT_SYMBOL_GPL(devlink_dpipe_headers_register);

/**
 *	devlink_dpipe_headers_unregister - unregister dpipe headers
 *
 *	@devlink: devlink
 *
 *	Unregister the headers supported by hardware.
 */
void devlink_dpipe_headers_unregister(struct devlink *devlink)
{
	mutex_lock(&devlink->lock);
	devlink->dpipe_headers = NULL;
	mutex_unlock(&devlink->lock);
}
EXPORT_SYMBOL_GPL(devlink_dpipe_headers_unregister);

/**
 *	devlink_dpipe_table_counter_enabled - check if counter allocation
 *					      required
 *	@devlink: devlink
 *	@table_name: tables name
 *
 *	Used by driver to check if counter allocation is required.
 *	After counter allocation is turned on the table entries
 *	are updated to include counter statistics.
 *
 *	After that point on the driver must respect the counter
 *	state so that each entry added to the table is added
 *	with a counter.
 */
bool devlink_dpipe_table_counter_enabled(struct devlink *devlink,
					 const char *table_name)
{
	struct devlink_dpipe_table *table;
	bool enabled;

	rcu_read_lock();
	table = devlink_dpipe_table_find(&devlink->dpipe_table_list,
					 table_name);
	enabled = false;
	if (table)
		enabled = table->counters_enabled;
	rcu_read_unlock();
	return enabled;
}
EXPORT_SYMBOL_GPL(devlink_dpipe_table_counter_enabled);

/**
 *	devlink_dpipe_table_register - register dpipe table
 *
 *	@devlink: devlink
 *	@table_name: table name
 *	@table_ops: table ops
 *	@priv: priv
 *	@counter_control_extern: external control for counters
 */
int devlink_dpipe_table_register(struct devlink *devlink,
				 const char *table_name,
				 struct devlink_dpipe_table_ops *table_ops,
				 void *priv, bool counter_control_extern)
{
	struct devlink_dpipe_table *table;

	if (devlink_dpipe_table_find(&devlink->dpipe_table_list, table_name))
		return -EEXIST;

	if (WARN_ON(!table_ops->size_get))
		return -EINVAL;

	table = kzalloc(sizeof(*table), GFP_KERNEL);
	if (!table)
		return -ENOMEM;

	table->name = table_name;
	table->table_ops = table_ops;
	table->priv = priv;
	table->counter_control_extern = counter_control_extern;

	mutex_lock(&devlink->lock);
	list_add_tail_rcu(&table->list, &devlink->dpipe_table_list);
	mutex_unlock(&devlink->lock);
	return 0;
}
EXPORT_SYMBOL_GPL(devlink_dpipe_table_register);

/**
 *	devlink_dpipe_table_unregister - unregister dpipe table
 *
 *	@devlink: devlink
 *	@table_name: table name
 */
void devlink_dpipe_table_unregister(struct devlink *devlink,
				    const char *table_name)
{
	struct devlink_dpipe_table *table;

	mutex_lock(&devlink->lock);
	table = devlink_dpipe_table_find(&devlink->dpipe_table_list,
					 table_name);
	if (!table)
		goto unlock;
	list_del_rcu(&table->list);
	mutex_unlock(&devlink->lock);
	kfree_rcu(table, rcu);
	return;
unlock:
	mutex_unlock(&devlink->lock);
}
EXPORT_SYMBOL_GPL(devlink_dpipe_table_unregister);

/**
 *	devlink_resource_register - devlink resource register
 *
 *	@devlink: devlink
 *	@resource_name: resource's name
 *	@top_hierarchy: top hierarchy
 *	@reload_required: reload is required for new configuration to
 *			  apply
 *	@resource_size: resource's size
 *	@resource_id: resource's id
 *	@parent_reosurce_id: resource's parent id
 *	@size params: size parameters
 */
int devlink_resource_register(struct devlink *devlink,
			      const char *resource_name,
			      u64 resource_size,
			      u64 resource_id,
			      u64 parent_resource_id,
			      const struct devlink_resource_size_params *size_params)
{
	struct devlink_resource *resource;
	struct list_head *resource_list;
	bool top_hierarchy;
	int err = 0;

	top_hierarchy = parent_resource_id == DEVLINK_RESOURCE_ID_PARENT_TOP;

	mutex_lock(&devlink->lock);
	resource = devlink_resource_find(devlink, NULL, resource_id);
	if (resource) {
		err = -EINVAL;
		goto out;
	}

	resource = kzalloc(sizeof(*resource), GFP_KERNEL);
	if (!resource) {
		err = -ENOMEM;
		goto out;
	}

	if (top_hierarchy) {
		resource_list = &devlink->resource_list;
	} else {
		struct devlink_resource *parent_resource;

		parent_resource = devlink_resource_find(devlink, NULL,
							parent_resource_id);
		if (parent_resource) {
			resource_list = &parent_resource->resource_list;
			resource->parent = parent_resource;
		} else {
			kfree(resource);
			err = -EINVAL;
			goto out;
		}
	}

	resource->name = resource_name;
	resource->size = resource_size;
	resource->size_new = resource_size;
	resource->id = resource_id;
	resource->size_valid = true;
	memcpy(&resource->size_params, size_params,
	       sizeof(resource->size_params));
	INIT_LIST_HEAD(&resource->resource_list);
	list_add_tail(&resource->list, resource_list);
out:
	mutex_unlock(&devlink->lock);
	return err;
}
EXPORT_SYMBOL_GPL(devlink_resource_register);

/**
 *	devlink_resources_unregister - free all resources
 *
 *	@devlink: devlink
 *	@resource: resource
 */
void devlink_resources_unregister(struct devlink *devlink,
				  struct devlink_resource *resource)
{
	struct devlink_resource *tmp, *child_resource;
	struct list_head *resource_list;

	if (resource)
		resource_list = &resource->resource_list;
	else
		resource_list = &devlink->resource_list;

	if (!resource)
		mutex_lock(&devlink->lock);

	list_for_each_entry_safe(child_resource, tmp, resource_list, list) {
		devlink_resources_unregister(devlink, child_resource);
		list_del(&child_resource->list);
		kfree(child_resource);
	}

	if (!resource)
		mutex_unlock(&devlink->lock);
}
EXPORT_SYMBOL_GPL(devlink_resources_unregister);

/**
 *	devlink_resource_size_get - get and update size
 *
 *	@devlink: devlink
 *	@resource_id: the requested resource id
 *	@p_resource_size: ptr to update
 */
int devlink_resource_size_get(struct devlink *devlink,
			      u64 resource_id,
			      u64 *p_resource_size)
{
	struct devlink_resource *resource;
	int err = 0;

	mutex_lock(&devlink->lock);
	resource = devlink_resource_find(devlink, NULL, resource_id);
	if (!resource) {
		err = -EINVAL;
		goto out;
	}
	*p_resource_size = resource->size_new;
	resource->size = resource->size_new;
out:
	mutex_unlock(&devlink->lock);
	return err;
}
EXPORT_SYMBOL_GPL(devlink_resource_size_get);

/**
 *	devlink_dpipe_table_resource_set - set the resource id
 *
 *	@devlink: devlink
 *	@table_name: table name
 *	@resource_id: resource id
 *	@resource_units: number of resource's units consumed per table's entry
 */
int devlink_dpipe_table_resource_set(struct devlink *devlink,
				     const char *table_name, u64 resource_id,
				     u64 resource_units)
{
	struct devlink_dpipe_table *table;
	int err = 0;

	mutex_lock(&devlink->lock);
	table = devlink_dpipe_table_find(&devlink->dpipe_table_list,
					 table_name);
	if (!table) {
		err = -EINVAL;
		goto out;
	}
	table->resource_id = resource_id;
	table->resource_units = resource_units;
	table->resource_valid = true;
out:
	mutex_unlock(&devlink->lock);
	return err;
}
EXPORT_SYMBOL_GPL(devlink_dpipe_table_resource_set);

/**
 *	devlink_resource_occ_get_register - register occupancy getter
 *
 *	@devlink: devlink
 *	@resource_id: resource id
 *	@occ_get: occupancy getter callback
 *	@occ_get_priv: occupancy getter callback priv
 */
void devlink_resource_occ_get_register(struct devlink *devlink,
				       u64 resource_id,
				       devlink_resource_occ_get_t *occ_get,
				       void *occ_get_priv)
{
	struct devlink_resource *resource;

	mutex_lock(&devlink->lock);
	resource = devlink_resource_find(devlink, NULL, resource_id);
	if (WARN_ON(!resource))
		goto out;
	WARN_ON(resource->occ_get);

	resource->occ_get = occ_get;
	resource->occ_get_priv = occ_get_priv;
out:
	mutex_unlock(&devlink->lock);
}
EXPORT_SYMBOL_GPL(devlink_resource_occ_get_register);

/**
 *	devlink_resource_occ_get_unregister - unregister occupancy getter
 *
 *	@devlink: devlink
 *	@resource_id: resource id
 */
void devlink_resource_occ_get_unregister(struct devlink *devlink,
					 u64 resource_id)
{
	struct devlink_resource *resource;

	mutex_lock(&devlink->lock);
	resource = devlink_resource_find(devlink, NULL, resource_id);
	if (WARN_ON(!resource))
		goto out;
	WARN_ON(!resource->occ_get);

	resource->occ_get = NULL;
	resource->occ_get_priv = NULL;
out:
	mutex_unlock(&devlink->lock);
}
EXPORT_SYMBOL_GPL(devlink_resource_occ_get_unregister);

/**
 *	devlink_params_register - register configuration parameters
 *
 *	@devlink: devlink
 *	@params: configuration parameters array
 *	@params_count: number of parameters provided
 *
 *	Register the configuration parameters supported by the driver.
 */
int devlink_params_register(struct devlink *devlink,
			    const struct devlink_param *params,
			    size_t params_count)
{
	const struct devlink_param *param = params;
	int i;
	int err;

	mutex_lock(&devlink->lock);
	for (i = 0; i < params_count; i++, param++) {
		if (!param || !param->name || !param->supported_cmodes) {
			err = -EINVAL;
			goto rollback;
		}
		if (param->generic) {
			err = devlink_param_generic_verify(param);
			if (err)
				goto rollback;
		} else {
			err = devlink_param_driver_verify(param);
			if (err)
				goto rollback;
		}
		err = devlink_param_register_one(devlink, param);
		if (err)
			goto rollback;
	}

	mutex_unlock(&devlink->lock);
	return 0;

rollback:
	if (!i)
		goto unlock;
	for (param--; i > 0; i--, param--)
		devlink_param_unregister_one(devlink, param);
unlock:
	mutex_unlock(&devlink->lock);
	return err;
}
EXPORT_SYMBOL_GPL(devlink_params_register);

/**
 *	devlink_params_unregister - unregister configuration parameters
 *	@devlink: devlink
 *	@params: configuration parameters to unregister
 *	@params_count: number of parameters provided
 */
void devlink_params_unregister(struct devlink *devlink,
			       const struct devlink_param *params,
			       size_t params_count)
{
	const struct devlink_param *param = params;
	int i;

	mutex_lock(&devlink->lock);
	for (i = 0; i < params_count; i++, param++)
		devlink_param_unregister_one(devlink, param);
	mutex_unlock(&devlink->lock);
}
EXPORT_SYMBOL_GPL(devlink_params_unregister);

/**
 *	devlink_param_driverinit_value_get - get configuration parameter
 *					     value for driver initializing
 *
 *	@devlink: devlink
 *	@param_id: parameter ID
 *	@init_val: value of parameter in driverinit configuration mode
 *
 *	This function should be used by the driver to get driverinit
 *	configuration for initialization after reload command.
 */
int devlink_param_driverinit_value_get(struct devlink *devlink, u32 param_id,
				       union devlink_param_value *init_val)
{
	struct devlink_param_item *param_item;

	if (!devlink->ops || !devlink->ops->reload)
		return -EOPNOTSUPP;

	param_item = devlink_param_find_by_id(&devlink->param_list, param_id);
	if (!param_item)
		return -EINVAL;

	if (!param_item->driverinit_value_valid ||
	    !devlink_param_cmode_is_supported(param_item->param,
					      DEVLINK_PARAM_CMODE_DRIVERINIT))
		return -EOPNOTSUPP;

	if (param_item->param->type == DEVLINK_PARAM_TYPE_STRING)
		strcpy(init_val->vstr, param_item->driverinit_value.vstr);
	else
		*init_val = param_item->driverinit_value;

	return 0;
}
EXPORT_SYMBOL_GPL(devlink_param_driverinit_value_get);

/**
 *	devlink_param_driverinit_value_set - set value of configuration
 *					     parameter for driverinit
 *					     configuration mode
 *
 *	@devlink: devlink
 *	@param_id: parameter ID
 *	@init_val: value of parameter to set for driverinit configuration mode
 *
 *	This function should be used by the driver to set driverinit
 *	configuration mode default value.
 */
int devlink_param_driverinit_value_set(struct devlink *devlink, u32 param_id,
				       union devlink_param_value init_val)
{
	struct devlink_param_item *param_item;

	param_item = devlink_param_find_by_id(&devlink->param_list, param_id);
	if (!param_item)
		return -EINVAL;

	if (!devlink_param_cmode_is_supported(param_item->param,
					      DEVLINK_PARAM_CMODE_DRIVERINIT))
		return -EOPNOTSUPP;

	if (param_item->param->type == DEVLINK_PARAM_TYPE_STRING)
		strcpy(param_item->driverinit_value.vstr, init_val.vstr);
	else
		param_item->driverinit_value = init_val;
	param_item->driverinit_value_valid = true;

	devlink_param_notify(devlink, param_item, DEVLINK_CMD_PARAM_NEW);
	return 0;
}
EXPORT_SYMBOL_GPL(devlink_param_driverinit_value_set);

/**
 *	devlink_param_value_changed - notify devlink on a parameter's value
 *				      change. Should be called by the driver
 *				      right after the change.
 *
 *	@devlink: devlink
 *	@param_id: parameter ID
 *
 *	This function should be used by the driver to notify devlink on value
 *	change, excluding driverinit configuration mode.
 *	For driverinit configuration mode driver should use the function
 *	devlink_param_driverinit_value_set() instead.
 */
void devlink_param_value_changed(struct devlink *devlink, u32 param_id)
{
	struct devlink_param_item *param_item;

	param_item = devlink_param_find_by_id(&devlink->param_list, param_id);
	WARN_ON(!param_item);

	devlink_param_notify(devlink, param_item, DEVLINK_CMD_PARAM_NEW);
}
EXPORT_SYMBOL_GPL(devlink_param_value_changed);

/**
 *	devlink_param_value_str_fill - Safely fill-up the string preventing
 *				       from overflow of the preallocated buffer
 *
 *	@dst_val: destination devlink_param_value
 *	@src: source buffer
 */
void devlink_param_value_str_fill(union devlink_param_value *dst_val,
				  const char *src)
{
	size_t len;

	len = strlcpy(dst_val->vstr, src, __DEVLINK_PARAM_MAX_STRING_VALUE);
	WARN_ON(len >= __DEVLINK_PARAM_MAX_STRING_VALUE);
}
EXPORT_SYMBOL_GPL(devlink_param_value_str_fill);

/**
 *	devlink_region_create - create a new address region
 *
 *	@devlink: devlink
 *	@region_name: region name
 *	@region_max_snapshots: Maximum supported number of snapshots for region
 *	@region_size: size of region
 */
struct devlink_region *devlink_region_create(struct devlink *devlink,
					     const char *region_name,
					     u32 region_max_snapshots,
					     u64 region_size)
{
	struct devlink_region *region;
	int err = 0;

	mutex_lock(&devlink->lock);

	if (devlink_region_get_by_name(devlink, region_name)) {
		err = -EEXIST;
		goto unlock;
	}

	region = kzalloc(sizeof(*region), GFP_KERNEL);
	if (!region) {
		err = -ENOMEM;
		goto unlock;
	}

	region->devlink = devlink;
	region->max_snapshots = region_max_snapshots;
	region->name = region_name;
	region->size = region_size;
	INIT_LIST_HEAD(&region->snapshot_list);
	list_add_tail(&region->list, &devlink->region_list);
	devlink_nl_region_notify(region, NULL, DEVLINK_CMD_REGION_NEW);

	mutex_unlock(&devlink->lock);
	return region;

unlock:
	mutex_unlock(&devlink->lock);
	return ERR_PTR(err);
}
EXPORT_SYMBOL_GPL(devlink_region_create);

/**
 *	devlink_region_destroy - destroy address region
 *
 *	@region: devlink region to destroy
 */
void devlink_region_destroy(struct devlink_region *region)
{
	struct devlink *devlink = region->devlink;
	struct devlink_snapshot *snapshot, *ts;

	mutex_lock(&devlink->lock);

	/* Free all snapshots of region */
	list_for_each_entry_safe(snapshot, ts, &region->snapshot_list, list)
		devlink_region_snapshot_del(snapshot);

	list_del(&region->list);

	devlink_nl_region_notify(region, NULL, DEVLINK_CMD_REGION_DEL);
	mutex_unlock(&devlink->lock);
	kfree(region);
}
EXPORT_SYMBOL_GPL(devlink_region_destroy);

/**
 *	devlink_region_shapshot_id_get - get snapshot ID
 *
 *	This callback should be called when adding a new snapshot,
 *	Driver should use the same id for multiple snapshots taken
 *	on multiple regions at the same time/by the same trigger.
 *
 *	@devlink: devlink
 */
u32 devlink_region_shapshot_id_get(struct devlink *devlink)
{
	u32 id;

	mutex_lock(&devlink->lock);
	id = ++devlink->snapshot_id;
	mutex_unlock(&devlink->lock);

	return id;
}
EXPORT_SYMBOL_GPL(devlink_region_shapshot_id_get);

/**
 *	devlink_region_snapshot_create - create a new snapshot
 *	This will add a new snapshot of a region. The snapshot
 *	will be stored on the region struct and can be accessed
 *	from devlink. This is useful for future	analyses of snapshots.
 *	Multiple snapshots can be created on a region.
 *	The @snapshot_id should be obtained using the getter function.
 *
 *	@devlink_region: devlink region of the snapshot
 *	@data_len: size of snapshot data
 *	@data: snapshot data
 *	@snapshot_id: snapshot id to be created
 *	@data_destructor: pointer to destructor function to free data
 */
int devlink_region_snapshot_create(struct devlink_region *region, u64 data_len,
				   u8 *data, u32 snapshot_id,
				   devlink_snapshot_data_dest_t *data_destructor)
{
	struct devlink *devlink = region->devlink;
	struct devlink_snapshot *snapshot;
	int err;

	mutex_lock(&devlink->lock);

	/* check if region can hold one more snapshot */
	if (region->cur_snapshots == region->max_snapshots) {
		err = -ENOMEM;
		goto unlock;
	}

	if (devlink_region_snapshot_get_by_id(region, snapshot_id)) {
		err = -EEXIST;
		goto unlock;
	}

	snapshot = kzalloc(sizeof(*snapshot), GFP_KERNEL);
	if (!snapshot) {
		err = -ENOMEM;
		goto unlock;
	}

	snapshot->id = snapshot_id;
	snapshot->region = region;
	snapshot->data = data;
	snapshot->data_len = data_len;
	snapshot->data_destructor = data_destructor;

	list_add_tail(&snapshot->list, &region->snapshot_list);

	region->cur_snapshots++;

	devlink_nl_region_notify(region, snapshot, DEVLINK_CMD_REGION_NEW);
	mutex_unlock(&devlink->lock);
	return 0;

unlock:
	mutex_unlock(&devlink->lock);
	return err;
}
EXPORT_SYMBOL_GPL(devlink_region_snapshot_create);

static int __init devlink_module_init(void)
{
	return genl_register_family(&devlink_nl_family);
}

static void __exit devlink_module_exit(void)
{
	genl_unregister_family(&devlink_nl_family);
}

module_init(devlink_module_init);
module_exit(devlink_module_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Jiri Pirko <jiri@mellanox.com>");
MODULE_DESCRIPTION("Network physical device Netlink interface");
MODULE_ALIAS_GENL_FAMILY(DEVLINK_GENL_NAME);
