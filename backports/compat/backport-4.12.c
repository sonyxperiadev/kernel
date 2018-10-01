/*
 * Copyright 2017 Intel Deutschland GmbH
 */
#include <net/genetlink.h>
#include <net/sock.h>

enum nlmsgerr_attrs {
	NLMSGERR_ATTR_UNUSED,
	NLMSGERR_ATTR_MSG,
	NLMSGERR_ATTR_OFFS,
	NLMSGERR_ATTR_COOKIE,
	__NLMSGERR_ATTR_MAX,
	NLMSGERR_ATTR_MAX = __NLMSGERR_ATTR_MAX - 1
};

#define NLM_F_CAPPED	0x100	/* request was capped */
#define NLM_F_ACK_TLVS	0x200	/* extended ACK TVLs were included */

struct bp_extack_genl_family {
	struct genl_family family;
	struct genl_family *real_family;
	struct list_head list;

	struct genl_ops ops[];
};

static LIST_HEAD(copies_list);
static DEFINE_MUTEX(copies_mutex);

static const struct nla_policy extack_dummy_policy[1] = {};

static struct bp_extack_genl_family *get_copy(__genl_const struct genl_ops *op)
{
	do {
		op--;
	} while (op->policy != extack_dummy_policy);

	return container_of(op, struct bp_extack_genl_family, ops[0]);
}

static int extack_pre_doit(__genl_const struct genl_ops *ops,
			   struct sk_buff *skb,
			   struct genl_info *info)
{
	struct netlink_ext_ack *extack = kzalloc(sizeof(*extack), GFP_KERNEL);
	struct bp_extack_genl_family *copy = get_copy(ops);
	struct genl_ops *real_ops;
	int err;

	__bp_genl_info_userhdr_set(info, extack);

	if (!extack) {
		__bp_genl_info_userhdr_set(info, ERR_PTR(-ENOMEM));
		return -ENOMEM;
	}

	real_ops = (void *)&copy->real_family->ops[ops - &copy->ops[1]];
	extack->__bp_genl_real_ops = real_ops;

	if (copy->real_family->pre_doit)
		err = copy->real_family->pre_doit(real_ops, skb, info);
	else
		err = 0;

	if (err) {
		__bp_genl_info_userhdr_set(info, ERR_PTR(err));
		kfree(extack);
	}

	return err;
}

static void extack_netlink_ack(struct sk_buff *in_skb, struct nlmsghdr *nlh,
			       int err, const struct netlink_ext_ack *extack)
{
	struct sk_buff *skb;
	struct nlmsghdr *rep;
	struct nlmsgerr *errmsg;
	size_t payload = sizeof(*errmsg);
	size_t tlvlen = 0;
	unsigned int flags = 0;
	/* backports assumes everyone supports this - libnl does so it's true */
	bool nlk_has_extack = true;

	/* Error messages get the original request appened, unless the user
	 * requests to cap the error message, and get extra error data if
	 * requested.
	 * (ignored in backports)
	 */
	if (nlk_has_extack && extack && extack->_msg)
		tlvlen += nla_total_size(strlen(extack->_msg) + 1);

	if (err) {
		if (1)
			payload += nlmsg_len(nlh);
		else
			flags |= NLM_F_CAPPED;
		if (nlk_has_extack && extack && extack->bad_attr)
			tlvlen += nla_total_size(sizeof(u32));
	} else {
		flags |= NLM_F_CAPPED;

		if (nlk_has_extack && extack && extack->cookie_len)
			tlvlen += nla_total_size(extack->cookie_len);
	}

	if (tlvlen)
		flags |= NLM_F_ACK_TLVS;

	skb = nlmsg_new(payload + tlvlen, GFP_KERNEL);
	if (!skb) {
		NETLINK_CB(in_skb).sk->sk_err = ENOBUFS;
		NETLINK_CB(in_skb).sk->sk_error_report(NETLINK_CB(in_skb).sk);
		return;
	}

	rep = __nlmsg_put(skb, NETLINK_CB_PORTID(in_skb), nlh->nlmsg_seq,
			  NLMSG_ERROR, payload, flags);
	errmsg = nlmsg_data(rep);
	errmsg->error = err;
	memcpy(&errmsg->msg, nlh, payload > sizeof(*errmsg) ? nlh->nlmsg_len : sizeof(*nlh));

	if (nlk_has_extack && extack) {
		if (extack->_msg) {
			WARN_ON(nla_put_string(skb, NLMSGERR_ATTR_MSG,
					       extack->_msg));
		}
		if (err) {
			if (extack->bad_attr &&
			    !WARN_ON((u8 *)extack->bad_attr < in_skb->data ||
				     (u8 *)extack->bad_attr >= in_skb->data +
							       in_skb->len))
				WARN_ON(nla_put_u32(skb, NLMSGERR_ATTR_OFFS,
						    (u8 *)extack->bad_attr -
						    in_skb->data));
		} else {
			if (extack->cookie_len)
				WARN_ON(nla_put(skb, NLMSGERR_ATTR_COOKIE,
						extack->cookie_len,
						extack->cookie));
		}
	}

	nlmsg_end(skb, rep);

	netlink_unicast(in_skb->sk, skb, NETLINK_CB_PORTID(in_skb),
			MSG_DONTWAIT);
}

static int extack_doit(struct sk_buff *skb, struct genl_info *info)
{
	struct genl_ops *real_ops;
	int err;

	/* older kernels have a bug here */
	if (IS_ERR(__bp_genl_info_userhdr(info))) {
		extack_netlink_ack(skb, info->nlhdr,
				   PTR_ERR(__bp_genl_info_userhdr(info)),
				   genl_info_extack(info));
		goto out;
	}

	real_ops = genl_info_extack(info)->__bp_genl_real_ops;
	err = real_ops->doit(skb, info);

	if (err == -EINTR)
		return err;

	if (info->nlhdr->nlmsg_flags & NLM_F_ACK || err)
		extack_netlink_ack(skb, info->nlhdr, err,
				   genl_info_extack(info));

out:
	/* suppress sending ACK from normal netlink code */
	info->nlhdr->nlmsg_flags &= ~NLM_F_ACK;
	return 0;
}

static void extack_post_doit(__genl_const struct genl_ops *ops,
			     struct sk_buff *skb,
			     struct genl_info *info)
{
	void (*post_doit)(__genl_const struct genl_ops *ops,
			  struct sk_buff *skb,
			  struct genl_info *info);

	post_doit = get_copy(ops)->real_family->post_doit;

	if (post_doit)
		post_doit(ops, skb, info);
	kfree(__bp_genl_info_userhdr(info));
}

int bp_extack_genl_register_family(struct genl_family *family)
{
	unsigned int size = sizeof(struct bp_extack_genl_family) +
			    sizeof(family->ops[0]) * (family->n_ops + 1);
	struct bp_extack_genl_family *copy;
	int i, err;

	copy = kzalloc(size, GFP_KERNEL);
	if (!copy)
		return -ENOMEM;

	copy->family = *family;
	copy->real_family = family;
	copy->family.ops = &copy->ops[1];

	for (i = 0; i < family->n_ops; i++) {
		copy->ops[i + 1] = family->ops[i];
		if (family->ops[i].doit)
			copy->ops[i + 1].doit = extack_doit;
	}

	copy->ops[0].policy = extack_dummy_policy;

	copy->family.pre_doit = extack_pre_doit;
	copy->family.post_doit = extack_post_doit;

	err = __real_bp_extack_genl_register_family(&copy->family);
	if (err) {
		kfree(copy);
		return err;
	}

	/* copy this since the family might access it directly */
	family->id = copy->family.id;
	family->attrbuf = copy->family.attrbuf;

#if LINUX_VERSION_IS_LESS(3,13,0) && RHEL_RELEASE_CODE < RHEL_RELEASE_VERSION(7,0)
	/* family ID from the original family struct will be used when building
	 * genl messages (sent as nlmsg_type), so the new id should be updated
	 * in the original (older kernel format) family struct too
	 */
	family->family.id = copy->family.id;
#endif

#if LINUX_VERSION_IS_GEQ(3,13,0)
	family->mcgrp_offset = copy->family.mcgrp_offset;
#endif

	mutex_lock(&copies_mutex);
	list_add_tail(&copy->list, &copies_list);
	mutex_unlock(&copies_mutex);

	return 0;
}
EXPORT_SYMBOL_GPL(bp_extack_genl_register_family);

int bp_extack_genl_unregister_family(struct genl_family *family)
{
	struct bp_extack_genl_family *tmp, *copy = NULL;
	int err;

	mutex_lock(&copies_mutex);
	list_for_each_entry(tmp, &copies_list, list) {
		if (tmp->real_family == family) {
			copy = tmp;
			break;
		}
	}
	if (copy)
		list_del(&copy->list);
	mutex_unlock(&copies_mutex);

	if (!copy)
		return -ENOENT;

	err = __real_bp_extack_genl_unregister_family(&copy->family);
	WARN_ON(err);
	kfree(copy);

	return 0;
}
EXPORT_SYMBOL_GPL(bp_extack_genl_unregister_family);
