/*
 * Implementation of the kernel access vector cache (AVC).
 *
 * Authors:  Stephen Smalley, <sds@epoch.ncsc.mil>
 *	     James Morris <jmorris@redhat.com>
 *
 * Update:   KaiGai, Kohei <kaigai@ak.jp.nec.com>
 *	Replaced the avc_lock spinlock by RCU.
 *
 * Copyright (C) 2003 Red Hat, Inc., James Morris <jmorris@redhat.com>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2,
 *	as published by the Free Software Foundation.
 */
#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/dcache.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/percpu.h>
#include <linux/list.h>
#include <net/sock.h>
#include <linux/un.h>
#include <net/af_unix.h>
#include <linux/ip.h>
#include <linux/audit.h>
#include <linux/ipv6.h>
#include <net/ipv6.h>
#include "avc.h"
#include "avc_ss.h"
#include "classmap.h"

#define AVC_CACHE_SLOTS			512
#define AVC_DEF_CACHE_THRESHOLD		512
#define AVC_CACHE_RECLAIM		16

#ifdef CONFIG_SECURITY_SELINUX_AVC_STATS
#define avc_cache_stats_incr(field)	this_cpu_inc(avc_cache_stats.field)
#else
#define avc_cache_stats_incr(field)	do {} while (0)
#endif

struct avc_entry {
	u32			ssid;
	u32			tsid;
	u16			tclass;
	struct av_decision	avd;
	struct avc_operation_node *ops_node;
};

struct avc_node {
	struct avc_entry	ae;
	struct hlist_node	list; /* anchored in avc_cache->slots[i] */
	struct rcu_head		rhead;
};

struct avc_cache {
	struct hlist_head	slots[AVC_CACHE_SLOTS]; /* head for avc_node->list */
	spinlock_t		slots_lock[AVC_CACHE_SLOTS]; /* lock for writes */
	atomic_t		lru_hint;	/* LRU hint for reclaim scan */
	atomic_t		active_nodes;
	u32			latest_notif;	/* latest revocation notification */
};

struct avc_operation_decision_node {
	struct operation_decision od;
	struct list_head od_list;
};

struct avc_operation_node {
	struct operation ops;
	struct list_head od_head; /* list of operation_decision_node */
};

struct avc_callback_node {
	int (*callback) (u32 event);
	u32 events;
	struct avc_callback_node *next;
};

/* Exported via selinufs */
unsigned int avc_cache_threshold = AVC_DEF_CACHE_THRESHOLD;

#ifdef CONFIG_SECURITY_SELINUX_AVC_STATS
DEFINE_PER_CPU(struct avc_cache_stats, avc_cache_stats) = { 0 };
#endif

static struct avc_cache avc_cache;
static struct avc_callback_node *avc_callbacks;
static struct kmem_cache *avc_node_cachep;
static struct kmem_cache *avc_operation_decision_node_cachep;
static struct kmem_cache *avc_operation_node_cachep;
static struct kmem_cache *avc_operation_perm_cachep;

static inline int avc_hash(u32 ssid, u32 tsid, u16 tclass)
{
	return (ssid ^ (tsid<<2) ^ (tclass<<4)) & (AVC_CACHE_SLOTS - 1);
}

/**
 * avc_dump_av - Display an access vector in human-readable form.
 * @tclass: target security class
 * @av: access vector
 */
static void avc_dump_av(struct audit_buffer *ab, u16 tclass, u32 av)
{
	const char **perms;
	int i, perm;

	if (av == 0) {
		audit_log_format(ab, " null");
		return;
	}

	perms = secclass_map[tclass-1].perms;

	audit_log_format(ab, " {");
	i = 0;
	perm = 1;
	while (i < (sizeof(av) * 8)) {
		if ((perm & av) && perms[i]) {
			audit_log_format(ab, " %s", perms[i]);
			av &= ~perm;
		}
		i++;
		perm <<= 1;
	}

	if (av)
		audit_log_format(ab, " 0x%x", av);

	audit_log_format(ab, " }");
}

/**
 * avc_dump_query - Display a SID pair and a class in human-readable form.
 * @ssid: source security identifier
 * @tsid: target security identifier
 * @tclass: target security class
 */
static void avc_dump_query(struct audit_buffer *ab, u32 ssid, u32 tsid, u16 tclass)
{
	int rc;
	char *scontext;
	u32 scontext_len;

	rc = security_sid_to_context(ssid, &scontext, &scontext_len);
	if (rc)
		audit_log_format(ab, "ssid=%d", ssid);
	else {
		audit_log_format(ab, "scontext=%s", scontext);
		kfree(scontext);
	}

	rc = security_sid_to_context(tsid, &scontext, &scontext_len);
	if (rc)
		audit_log_format(ab, " tsid=%d", tsid);
	else {
		audit_log_format(ab, " tcontext=%s", scontext);
		kfree(scontext);
	}

	BUG_ON(tclass >= ARRAY_SIZE(secclass_map));
	audit_log_format(ab, " tclass=%s", secclass_map[tclass-1].name);
}

/**
 * avc_init - Initialize the AVC.
 *
 * Initialize the access vector cache.
 */
void __init avc_init(void)
{
	int i;

	for (i = 0; i < AVC_CACHE_SLOTS; i++) {
		INIT_HLIST_HEAD(&avc_cache.slots[i]);
		spin_lock_init(&avc_cache.slots_lock[i]);
	}
	atomic_set(&avc_cache.active_nodes, 0);
	atomic_set(&avc_cache.lru_hint, 0);

	avc_node_cachep = kmem_cache_create("avc_node", sizeof(struct avc_node),
					     0, SLAB_PANIC, NULL);
	avc_operation_node_cachep = kmem_cache_create("avc_operation_node",
				sizeof(struct avc_operation_node),
				0, SLAB_PANIC, NULL);
	avc_operation_decision_node_cachep = kmem_cache_create(
				"avc_operation_decision_node",
				sizeof(struct avc_operation_decision_node),
				0, SLAB_PANIC, NULL);
	avc_operation_perm_cachep = kmem_cache_create("avc_operation_perm",
				sizeof(struct operation_perm),
				0, SLAB_PANIC, NULL);

	audit_log(current->audit_context, GFP_KERNEL, AUDIT_KERNEL, "AVC INITIALIZED\n");
}

int avc_get_hash_stats(char *page)
{
	int i, chain_len, max_chain_len, slots_used;
	struct avc_node *node;
	struct hlist_head *head;

	rcu_read_lock();

	slots_used = 0;
	max_chain_len = 0;
	for (i = 0; i < AVC_CACHE_SLOTS; i++) {
		head = &avc_cache.slots[i];
		if (!hlist_empty(head)) {
			slots_used++;
			chain_len = 0;
			hlist_for_each_entry_rcu(node, head, list)
				chain_len++;
			if (chain_len > max_chain_len)
				max_chain_len = chain_len;
		}
	}

	rcu_read_unlock();

	return scnprintf(page, PAGE_SIZE, "entries: %d\nbuckets used: %d/%d\n"
			 "longest chain: %d\n",
			 atomic_read(&avc_cache.active_nodes),
			 slots_used, AVC_CACHE_SLOTS, max_chain_len);
}

/*
 * using a linked list for operation_decision lookup because the list is
 * always small. i.e. less than 5, typically 1
 */
static struct operation_decision *avc_operation_lookup(u8 type,
					struct avc_operation_node *ops_node)
{
	struct avc_operation_decision_node *od_node;
	struct operation_decision *od = NULL;

	list_for_each_entry(od_node, &ops_node->od_head, od_list) {
		if (od_node->od.type != type)
			continue;
		od = &od_node->od;
		break;
	}
	return od;
}

static inline unsigned int avc_operation_has_perm(struct operation_decision *od,
						u16 cmd, u8 specified)
{
	unsigned int rc = 0;
	u8 num = cmd & 0xff;

	if ((specified == OPERATION_ALLOWED) &&
			(od->specified & OPERATION_ALLOWED))
		rc = security_operation_test(od->allowed->perms, num);
	else if ((specified == OPERATION_AUDITALLOW) &&
			(od->specified & OPERATION_AUDITALLOW))
		rc = security_operation_test(od->auditallow->perms, num);
	else if ((specified == OPERATION_DONTAUDIT) &&
			(od->specified & OPERATION_DONTAUDIT))
		rc = security_operation_test(od->dontaudit->perms, num);
	return rc;
}

static void avc_operation_allow_perm(struct avc_operation_node *node, u16 cmd)
{
	struct operation_decision *od;
	u8 type;
	u8 num;

	type = cmd >> 8;
	num = cmd & 0xff;
	security_operation_set(node->ops.type, type);
	od = avc_operation_lookup(type, node);
	if (od && od->allowed)
		security_operation_set(od->allowed->perms, num);
}

static void avc_operation_decision_free(
				struct avc_operation_decision_node *od_node)
{
	struct operation_decision *od;

	od = &od_node->od;
	if (od->allowed)
		kmem_cache_free(avc_operation_perm_cachep, od->allowed);
	if (od->auditallow)
		kmem_cache_free(avc_operation_perm_cachep, od->auditallow);
	if (od->dontaudit)
		kmem_cache_free(avc_operation_perm_cachep, od->dontaudit);
	kmem_cache_free(avc_operation_decision_node_cachep, od_node);
}

static void avc_operation_free(struct avc_operation_node *ops_node)
{
	struct avc_operation_decision_node *od_node;

	if (!ops_node)
		return;

	list_for_each_entry(od_node, &ops_node->od_head, od_list)
		avc_operation_decision_free(od_node);
	kmem_cache_free(avc_operation_node_cachep, ops_node);
}

static void avc_copy_operation_decision(struct operation_decision *dest,
					struct operation_decision *src)
{
	dest->type = src->type;
	dest->specified = src->specified;
	if (dest->specified & OPERATION_ALLOWED)
		memcpy(dest->allowed->perms, src->allowed->perms,
				sizeof(src->allowed->perms));
	if (dest->specified & OPERATION_AUDITALLOW)
		memcpy(dest->auditallow->perms, src->auditallow->perms,
				sizeof(src->auditallow->perms));
	if (dest->specified & OPERATION_DONTAUDIT)
		memcpy(dest->dontaudit->perms, src->dontaudit->perms,
				sizeof(src->dontaudit->perms));
}

/*
 * similar to avc_copy_operation_decision, but only copy decision
 * information relevant to this command
 */
static inline void avc_quick_copy_operation_decision(u16 cmd,
			struct operation_decision *dest,
			struct operation_decision *src)
{
	/*
	 * compute index of the u32 of the 256 bits (8 u32s) that contain this
	 * command permission
	 */
	u8 i = (0xff & cmd) >> 5;

	dest->specified = src->specified;
	if (dest->specified & OPERATION_ALLOWED)
		dest->allowed->perms[i] = src->allowed->perms[i];
	if (dest->specified & OPERATION_AUDITALLOW)
		dest->auditallow->perms[i] = src->auditallow->perms[i];
	if (dest->specified & OPERATION_DONTAUDIT)
		dest->dontaudit->perms[i] = src->dontaudit->perms[i];
}

static struct avc_operation_decision_node
		*avc_operation_decision_alloc(u8 specified)
{
	struct avc_operation_decision_node *node;
	struct operation_decision *od;

	node = kmem_cache_zalloc(avc_operation_decision_node_cachep,
				GFP_ATOMIC | __GFP_NOMEMALLOC);
	if (!node)
		return NULL;

	od = &node->od;
	if (specified & OPERATION_ALLOWED) {
		od->allowed = kmem_cache_zalloc(avc_operation_perm_cachep,
						GFP_ATOMIC | __GFP_NOMEMALLOC);
		if (!od->allowed)
			goto error;
	}
	if (specified & OPERATION_AUDITALLOW) {
		od->auditallow = kmem_cache_zalloc(avc_operation_perm_cachep,
						GFP_ATOMIC | __GFP_NOMEMALLOC);
		if (!od->auditallow)
			goto error;
	}
	if (specified & OPERATION_DONTAUDIT) {
		od->dontaudit = kmem_cache_zalloc(avc_operation_perm_cachep,
						GFP_ATOMIC | __GFP_NOMEMALLOC);
		if (!od->dontaudit)
			goto error;
	}
	return node;
error:
	avc_operation_decision_free(node);
	return NULL;
}

static int avc_add_operation(struct avc_node *node,
			struct operation_decision *od)
{
	struct avc_operation_decision_node *dest_od;

	node->ae.ops_node->ops.len++;
	dest_od = avc_operation_decision_alloc(od->specified);
	if (!dest_od)
		return -ENOMEM;
	avc_copy_operation_decision(&dest_od->od, od);
	list_add(&dest_od->od_list, &node->ae.ops_node->od_head);
	return 0;
}

static struct avc_operation_node *avc_operation_alloc(void)
{
	struct avc_operation_node *ops;

	ops = kmem_cache_zalloc(avc_operation_node_cachep,
				GFP_ATOMIC|__GFP_NOMEMALLOC);
	if (!ops)
		return ops;
	INIT_LIST_HEAD(&ops->od_head);
	return ops;
}

static int avc_operation_populate(struct avc_node *node,
				struct avc_operation_node *src)
{
	struct avc_operation_node *dest;
	struct avc_operation_decision_node *dest_od;
	struct avc_operation_decision_node *src_od;

	if (src->ops.len == 0)
		return 0;
	dest = avc_operation_alloc();
	if (!dest)
		return -ENOMEM;

	memcpy(dest->ops.type, &src->ops.type, sizeof(dest->ops.type));
	dest->ops.len = src->ops.len;

	/* for each source od allocate a destination od and copy */
	list_for_each_entry(src_od, &src->od_head, od_list) {
		dest_od = avc_operation_decision_alloc(src_od->od.specified);
		if (!dest_od)
			goto error;
		avc_copy_operation_decision(&dest_od->od, &src_od->od);
		list_add(&dest_od->od_list, &dest->od_head);
	}
	node->ae.ops_node = dest;
	return 0;
error:
	avc_operation_free(dest);
	return -ENOMEM;

}

static inline u32 avc_operation_audit_required(u32 requested,
					struct av_decision *avd,
					struct operation_decision *od,
					u16 cmd,
					int result,
					u32 *deniedp)
{
	u32 denied, audited;

	denied = requested & ~avd->allowed;
	if (unlikely(denied)) {
		audited = denied & avd->auditdeny;
		if (audited && od) {
			if (avc_operation_has_perm(od, cmd,
						OPERATION_DONTAUDIT))
				audited &= ~requested;
		}
	} else if (result) {
		audited = denied = requested;
	} else {
		audited = requested & avd->auditallow;
		if (audited && od) {
			if (!avc_operation_has_perm(od, cmd,
						OPERATION_AUDITALLOW))
				audited &= ~requested;
		}
	}

	*deniedp = denied;
	return audited;
}

static inline int avc_operation_audit(u32 ssid, u32 tsid, u16 tclass,
				u32 requested, struct av_decision *avd,
				struct operation_decision *od,
				u16 cmd, int result,
				struct common_audit_data *ad)
{
	u32 audited, denied;

	audited = avc_operation_audit_required(
			requested, avd, od, cmd, result, &denied);
	if (likely(!audited))
		return 0;
	return slow_avc_audit(ssid, tsid, tclass, requested,
			audited, denied, result, ad, 0);
}

static void avc_node_free(struct rcu_head *rhead)
{
	struct avc_node *node = container_of(rhead, struct avc_node, rhead);
	avc_operation_free(node->ae.ops_node);
	kmem_cache_free(avc_node_cachep, node);
	avc_cache_stats_incr(frees);
}

static void avc_node_delete(struct avc_node *node)
{
	hlist_del_rcu(&node->list);
	call_rcu(&node->rhead, avc_node_free);
	atomic_dec(&avc_cache.active_nodes);
}

static void avc_node_kill(struct avc_node *node)
{
	avc_operation_free(node->ae.ops_node);
	kmem_cache_free(avc_node_cachep, node);
	avc_cache_stats_incr(frees);
	atomic_dec(&avc_cache.active_nodes);
}

static void avc_node_replace(struct avc_node *new, struct avc_node *old)
{
	hlist_replace_rcu(&old->list, &new->list);
	call_rcu(&old->rhead, avc_node_free);
	atomic_dec(&avc_cache.active_nodes);
}

static inline int avc_reclaim_node(void)
{
	struct avc_node *node;
	int hvalue, try, ecx;
	unsigned long flags;
	struct hlist_head *head;
	spinlock_t *lock;

	for (try = 0, ecx = 0; try < AVC_CACHE_SLOTS; try++) {
		hvalue = atomic_inc_return(&avc_cache.lru_hint) & (AVC_CACHE_SLOTS - 1);
		head = &avc_cache.slots[hvalue];
		lock = &avc_cache.slots_lock[hvalue];

		if (!spin_trylock_irqsave(lock, flags))
			continue;

		rcu_read_lock();
		hlist_for_each_entry(node, head, list) {
			avc_node_delete(node);
			avc_cache_stats_incr(reclaims);
			ecx++;
			if (ecx >= AVC_CACHE_RECLAIM) {
				rcu_read_unlock();
				spin_unlock_irqrestore(lock, flags);
				goto out;
			}
		}
		rcu_read_unlock();
		spin_unlock_irqrestore(lock, flags);
	}
out:
	return ecx;
}

static struct avc_node *avc_alloc_node(void)
{
	struct avc_node *node;

	node = kmem_cache_zalloc(avc_node_cachep, GFP_ATOMIC|__GFP_NOMEMALLOC);
	if (!node)
		goto out;

	INIT_HLIST_NODE(&node->list);
	avc_cache_stats_incr(allocations);

	if (atomic_inc_return(&avc_cache.active_nodes) > avc_cache_threshold)
		avc_reclaim_node();

out:
	return node;
}

static void avc_node_populate(struct avc_node *node, u32 ssid, u32 tsid, u16 tclass, struct av_decision *avd)
{
	node->ae.ssid = ssid;
	node->ae.tsid = tsid;
	node->ae.tclass = tclass;
	memcpy(&node->ae.avd, avd, sizeof(node->ae.avd));
}

static inline struct avc_node *avc_search_node(u32 ssid, u32 tsid, u16 tclass)
{
	struct avc_node *node, *ret = NULL;
	int hvalue;
	struct hlist_head *head;

	hvalue = avc_hash(ssid, tsid, tclass);
	head = &avc_cache.slots[hvalue];
	hlist_for_each_entry_rcu(node, head, list) {
		if (ssid == node->ae.ssid &&
		    tclass == node->ae.tclass &&
		    tsid == node->ae.tsid) {
			ret = node;
			break;
		}
	}

	return ret;
}

/**
 * avc_lookup - Look up an AVC entry.
 * @ssid: source security identifier
 * @tsid: target security identifier
 * @tclass: target security class
 *
 * Look up an AVC entry that is valid for the
 * (@ssid, @tsid), interpreting the permissions
 * based on @tclass.  If a valid AVC entry exists,
 * then this function returns the avc_node.
 * Otherwise, this function returns NULL.
 */
static struct avc_node *avc_lookup(u32 ssid, u32 tsid, u16 tclass)
{
	struct avc_node *node;

	avc_cache_stats_incr(lookups);
	node = avc_search_node(ssid, tsid, tclass);

	if (node)
		return node;

	avc_cache_stats_incr(misses);
	return NULL;
}

static int avc_latest_notif_update(int seqno, int is_insert)
{
	int ret = 0;
	static DEFINE_SPINLOCK(notif_lock);
	unsigned long flag;

	spin_lock_irqsave(&notif_lock, flag);
	if (is_insert) {
		if (seqno < avc_cache.latest_notif) {
			printk(KERN_WARNING "SELinux: avc:  seqno %d < latest_notif %d\n",
			       seqno, avc_cache.latest_notif);
			ret = -EAGAIN;
		}
	} else {
		if (seqno > avc_cache.latest_notif)
			avc_cache.latest_notif = seqno;
	}
	spin_unlock_irqrestore(&notif_lock, flag);

	return ret;
}

/**
 * avc_insert - Insert an AVC entry.
 * @ssid: source security identifier
 * @tsid: target security identifier
 * @tclass: target security class
 * @avd: resulting av decision
 * @ops: resulting operation decisions
 *
 * Insert an AVC entry for the SID pair
 * (@ssid, @tsid) and class @tclass.
 * The access vectors and the sequence number are
 * normally provided by the security server in
 * response to a security_compute_av() call.  If the
 * sequence number @avd->seqno is not less than the latest
 * revocation notification, then the function copies
 * the access vectors into a cache entry, returns
 * avc_node inserted. Otherwise, this function returns NULL.
 */
static struct avc_node *avc_insert(u32 ssid, u32 tsid, u16 tclass,
				struct av_decision *avd,
				struct avc_operation_node *ops_node)
{
	struct avc_node *pos, *node = NULL;
	int hvalue;
	unsigned long flag;

	if (avc_latest_notif_update(avd->seqno, 1))
		goto out;

	node = avc_alloc_node();
	if (node) {
		struct hlist_head *head;
		spinlock_t *lock;
		int rc = 0;

		hvalue = avc_hash(ssid, tsid, tclass);
		avc_node_populate(node, ssid, tsid, tclass, avd);
		rc = avc_operation_populate(node, ops_node);
		if (rc) {
			kmem_cache_free(avc_node_cachep, node);
			return NULL;
		}
		head = &avc_cache.slots[hvalue];
		lock = &avc_cache.slots_lock[hvalue];

		spin_lock_irqsave(lock, flag);
		hlist_for_each_entry(pos, head, list) {
			if (pos->ae.ssid == ssid &&
			    pos->ae.tsid == tsid &&
			    pos->ae.tclass == tclass) {
				avc_node_replace(node, pos);
				goto found;
			}
		}
		hlist_add_head_rcu(&node->list, head);
found:
		spin_unlock_irqrestore(lock, flag);
	}
out:
	return node;
}

/**
 * avc_audit_pre_callback - SELinux specific information
 * will be called by generic audit code
 * @ab: the audit buffer
 * @a: audit_data
 */
static void avc_audit_pre_callback(struct audit_buffer *ab, void *a)
{
	struct common_audit_data *ad = a;
	audit_log_format(ab, "avc:  %s ",
			 ad->selinux_audit_data->denied ? "denied" : "granted");
	avc_dump_av(ab, ad->selinux_audit_data->tclass,
			ad->selinux_audit_data->audited);
	audit_log_format(ab, " for ");
}

/**
 * avc_audit_post_callback - SELinux specific information
 * will be called by generic audit code
 * @ab: the audit buffer
 * @a: audit_data
 */
static void avc_audit_post_callback(struct audit_buffer *ab, void *a)
{
	struct common_audit_data *ad = a;
	audit_log_format(ab, " ");
	avc_dump_query(ab, ad->selinux_audit_data->ssid,
			   ad->selinux_audit_data->tsid,
			   ad->selinux_audit_data->tclass);
	if (ad->selinux_audit_data->denied) {
		audit_log_format(ab, " permissive=%u",
				 ad->selinux_audit_data->result ? 0 : 1);
	}
}

/* This is the slow part of avc audit with big stack footprint */
noinline int slow_avc_audit(u32 ssid, u32 tsid, u16 tclass,
		u32 requested, u32 audited, u32 denied, int result,
		struct common_audit_data *a,
		unsigned flags)
{
	struct common_audit_data stack_data;
	struct selinux_audit_data sad;

	if (!a) {
		a = &stack_data;
		a->type = LSM_AUDIT_DATA_NONE;
	}

	/*
	 * When in a RCU walk do the audit on the RCU retry.  This is because
	 * the collection of the dname in an inode audit message is not RCU
	 * safe.  Note this may drop some audits when the situation changes
	 * during retry. However this is logically just as if the operation
	 * happened a little later.
	 */
	if ((a->type == LSM_AUDIT_DATA_INODE) &&
	    (flags & MAY_NOT_BLOCK))
		return -ECHILD;

	sad.tclass = tclass;
	sad.requested = requested;
	sad.ssid = ssid;
	sad.tsid = tsid;
	sad.audited = audited;
	sad.denied = denied;
	sad.result = result;

	a->selinux_audit_data = &sad;

	common_lsm_audit(a, avc_audit_pre_callback, avc_audit_post_callback);
	return 0;
}

/**
 * avc_add_callback - Register a callback for security events.
 * @callback: callback function
 * @events: security events
 *
 * Register a callback function for events in the set @events.
 * Returns %0 on success or -%ENOMEM if insufficient memory
 * exists to add the callback.
 */
int __init avc_add_callback(int (*callback)(u32 event), u32 events)
{
	struct avc_callback_node *c;
	int rc = 0;

	c = kmalloc(sizeof(*c), GFP_KERNEL);
	if (!c) {
		rc = -ENOMEM;
		goto out;
	}

	c->callback = callback;
	c->events = events;
	c->next = avc_callbacks;
	avc_callbacks = c;
out:
	return rc;
}

static inline int avc_sidcmp(u32 x, u32 y)
{
	return (x == y || x == SECSID_WILD || y == SECSID_WILD);
}

/**
 * avc_update_node Update an AVC entry
 * @event : Updating event
 * @perms : Permission mask bits
 * @ssid,@tsid,@tclass : identifier of an AVC entry
 * @seqno : sequence number when decision was made
 * @od: operation_decision to be added to the node
 *
 * if a valid AVC entry doesn't exist,this function returns -ENOENT.
 * if kmalloc() called internal returns NULL, this function returns -ENOMEM.
 * otherwise, this function updates the AVC entry. The original AVC-entry object
 * will release later by RCU.
 */
static int avc_update_node(u32 event, u32 perms, u16 cmd, u32 ssid, u32 tsid,
			u16 tclass, u32 seqno,
			struct operation_decision *od,
			u32 flags)
{
	int hvalue, rc = 0;
	unsigned long flag;
	struct avc_node *pos, *node, *orig = NULL;
	struct hlist_head *head;
	spinlock_t *lock;

	node = avc_alloc_node();
	if (!node) {
		rc = -ENOMEM;
		goto out;
	}

	/* Lock the target slot */
	hvalue = avc_hash(ssid, tsid, tclass);

	head = &avc_cache.slots[hvalue];
	lock = &avc_cache.slots_lock[hvalue];

	spin_lock_irqsave(lock, flag);

	hlist_for_each_entry(pos, head, list) {
		if (ssid == pos->ae.ssid &&
		    tsid == pos->ae.tsid &&
		    tclass == pos->ae.tclass &&
		    seqno == pos->ae.avd.seqno){
			orig = pos;
			break;
		}
	}

	if (!orig) {
		rc = -ENOENT;
		avc_node_kill(node);
		goto out_unlock;
	}

	/*
	 * Copy and replace original node.
	 */

	avc_node_populate(node, ssid, tsid, tclass, &orig->ae.avd);

	if (orig->ae.ops_node) {
		rc = avc_operation_populate(node, orig->ae.ops_node);
		if (rc) {
			kmem_cache_free(avc_node_cachep, node);
			goto out_unlock;
		}
	}

	switch (event) {
	case AVC_CALLBACK_GRANT:
		node->ae.avd.allowed |= perms;
		if (node->ae.ops_node && (flags & AVC_OPERATION_CMD))
			avc_operation_allow_perm(node->ae.ops_node, cmd);
		break;
	case AVC_CALLBACK_TRY_REVOKE:
	case AVC_CALLBACK_REVOKE:
		node->ae.avd.allowed &= ~perms;
		break;
	case AVC_CALLBACK_AUDITALLOW_ENABLE:
		node->ae.avd.auditallow |= perms;
		break;
	case AVC_CALLBACK_AUDITALLOW_DISABLE:
		node->ae.avd.auditallow &= ~perms;
		break;
	case AVC_CALLBACK_AUDITDENY_ENABLE:
		node->ae.avd.auditdeny |= perms;
		break;
	case AVC_CALLBACK_AUDITDENY_DISABLE:
		node->ae.avd.auditdeny &= ~perms;
		break;
	case AVC_CALLBACK_ADD_OPERATION:
		avc_add_operation(node, od);
		break;
	}
	avc_node_replace(node, orig);
out_unlock:
	spin_unlock_irqrestore(lock, flag);
out:
	return rc;
}

/**
 * avc_flush - Flush the cache
 */
static void avc_flush(void)
{
	struct hlist_head *head;
	struct avc_node *node;
	spinlock_t *lock;
	unsigned long flag;
	int i;

	for (i = 0; i < AVC_CACHE_SLOTS; i++) {
		head = &avc_cache.slots[i];
		lock = &avc_cache.slots_lock[i];

		spin_lock_irqsave(lock, flag);
		/*
		 * With preemptable RCU, the outer spinlock does not
		 * prevent RCU grace periods from ending.
		 */
		rcu_read_lock();
		hlist_for_each_entry(node, head, list)
			avc_node_delete(node);
		rcu_read_unlock();
		spin_unlock_irqrestore(lock, flag);
	}
}

/**
 * avc_ss_reset - Flush the cache and revalidate migrated permissions.
 * @seqno: policy sequence number
 */
int avc_ss_reset(u32 seqno)
{
	struct avc_callback_node *c;
	int rc = 0, tmprc;

	avc_flush();

	for (c = avc_callbacks; c; c = c->next) {
		if (c->events & AVC_CALLBACK_RESET) {
			tmprc = c->callback(AVC_CALLBACK_RESET);
			/* save the first error encountered for the return
			   value and continue processing the callbacks */
			if (!rc)
				rc = tmprc;
		}
	}

	avc_latest_notif_update(seqno, 0);
	return rc;
}

/*
 * Slow-path helper function for avc_has_perm_noaudit,
 * when the avc_node lookup fails. We get called with
 * the RCU read lock held, and need to return with it
 * still held, but drop if for the security compute.
 *
 * Don't inline this, since it's the slow-path and just
 * results in a bigger stack frame.
 */
static noinline struct avc_node *avc_compute_av(u32 ssid, u32 tsid,
			 u16 tclass, struct av_decision *avd,
			 struct avc_operation_node *ops_node)
{
	rcu_read_unlock();
	INIT_LIST_HEAD(&ops_node->od_head);
	security_compute_av(ssid, tsid, tclass, avd, &ops_node->ops);
	rcu_read_lock();
	return avc_insert(ssid, tsid, tclass, avd, ops_node);
}

static noinline int avc_denied(u32 ssid, u32 tsid,
				u16 tclass, u32 requested,
				u16 cmd, unsigned flags,
				struct av_decision *avd)
{
	if (flags & AVC_STRICT)
		return -EACCES;

	if (selinux_enforcing && !(avd->flags & AVD_FLAGS_PERMISSIVE))
		return -EACCES;

	avc_update_node(AVC_CALLBACK_GRANT, requested, cmd, ssid,
				tsid, tclass, avd->seqno, NULL, flags);
	return 0;
}

/*
 * ioctl commands are comprised of four fields, direction, size, type, and
 * number. The avc operation logic filters based on two of them:
 *
 * type: or code, typically unique to each driver
 * number: or function
 *
 * For example, 0x89 is a socket type, and number 0x27 is the get hardware
 * address function.
 */
int avc_has_operation(u32 ssid, u32 tsid, u16 tclass, u32 requested,
			u16 cmd, struct common_audit_data *ad)
{
	struct avc_node *node;
	struct av_decision avd;
	u32 denied;
	struct operation_decision *od = NULL;
	struct operation_decision od_local;
	struct operation_perm allowed;
	struct operation_perm auditallow;
	struct operation_perm dontaudit;
	struct avc_operation_node local_ops_node;
	struct avc_operation_node *ops_node;
	u8 type = cmd >> 8;
	int rc = 0, rc2;

	ops_node = &local_ops_node;
	BUG_ON(!requested);

	rcu_read_lock();

	node = avc_lookup(ssid, tsid, tclass);
	if (unlikely(!node)) {
		node = avc_compute_av(ssid, tsid, tclass, &avd, ops_node);
	} else {
		memcpy(&avd, &node->ae.avd, sizeof(avd));
		ops_node = node->ae.ops_node;
	}
	/* if operations are not defined, only consider av_decision */
	if (!ops_node || !ops_node->ops.len)
		goto decision;

	od_local.allowed = &allowed;
	od_local.auditallow = &auditallow;
	od_local.dontaudit = &dontaudit;

	/* lookup operation decision */
	od = avc_operation_lookup(type, ops_node);
	if (unlikely(!od)) {
		/* Compute operation decision if type is flagged */
		if (!security_operation_test(ops_node->ops.type, type)) {
			avd.allowed &= ~requested;
			goto decision;
		}
		rcu_read_unlock();
		security_compute_operation(ssid, tsid, tclass, type, &od_local);
		rcu_read_lock();
		avc_update_node(AVC_CALLBACK_ADD_OPERATION, requested, cmd,
				ssid, tsid, tclass, avd.seqno, &od_local, 0);
	} else {
		avc_quick_copy_operation_decision(cmd, &od_local, od);
	}
	od = &od_local;

	if (!avc_operation_has_perm(od, cmd, OPERATION_ALLOWED))
		avd.allowed &= ~requested;

decision:
	denied = requested & ~(avd.allowed);
	if (unlikely(denied))
		rc = avc_denied(ssid, tsid, tclass, requested, cmd,
				AVC_OPERATION_CMD, &avd);

	rcu_read_unlock();

	rc2 = avc_operation_audit(ssid, tsid, tclass, requested,
			&avd, od, cmd, rc, ad);
	if (rc2)
		return rc2;
	return rc;
}

/**
 * avc_has_perm_noaudit - Check permissions but perform no auditing.
 * @ssid: source security identifier
 * @tsid: target security identifier
 * @tclass: target security class
 * @requested: requested permissions, interpreted based on @tclass
 * @flags:  AVC_STRICT or 0
 * @avd: access vector decisions
 *
 * Check the AVC to determine whether the @requested permissions are granted
 * for the SID pair (@ssid, @tsid), interpreting the permissions
 * based on @tclass, and call the security server on a cache miss to obtain
 * a new decision and add it to the cache.  Return a copy of the decisions
 * in @avd.  Return %0 if all @requested permissions are granted,
 * -%EACCES if any permissions are denied, or another -errno upon
 * other errors.  This function is typically called by avc_has_perm(),
 * but may also be called directly to separate permission checking from
 * auditing, e.g. in cases where a lock must be held for the check but
 * should be released for the auditing.
 */
inline int avc_has_perm_noaudit(u32 ssid, u32 tsid,
			 u16 tclass, u32 requested,
			 unsigned flags,
			 struct av_decision *avd)
{
	struct avc_node *node;
	struct avc_operation_node ops_node;
	int rc = 0;
	u32 denied;

	BUG_ON(!requested);

	rcu_read_lock();

	node = avc_lookup(ssid, tsid, tclass);
	if (unlikely(!node))
		node = avc_compute_av(ssid, tsid, tclass, avd, &ops_node);
	else
		memcpy(avd, &node->ae.avd, sizeof(*avd));

	denied = requested & ~(avd->allowed);
	if (unlikely(denied))
		rc = avc_denied(ssid, tsid, tclass, requested, 0, flags, avd);

	rcu_read_unlock();
	return rc;
}

/**
 * avc_has_perm - Check permissions and perform any appropriate auditing.
 * @ssid: source security identifier
 * @tsid: target security identifier
 * @tclass: target security class
 * @requested: requested permissions, interpreted based on @tclass
 * @auditdata: auxiliary audit data
 * @flags: VFS walk flags
 *
 * Check the AVC to determine whether the @requested permissions are granted
 * for the SID pair (@ssid, @tsid), interpreting the permissions
 * based on @tclass, and call the security server on a cache miss to obtain
 * a new decision and add it to the cache.  Audit the granting or denial of
 * permissions in accordance with the policy.  Return %0 if all @requested
 * permissions are granted, -%EACCES if any permissions are denied, or
 * another -errno upon other errors.
 */
int avc_has_perm_flags(u32 ssid, u32 tsid, u16 tclass,
		       u32 requested, struct common_audit_data *auditdata,
		       unsigned flags)
{
	struct av_decision avd;
	int rc, rc2;

	rc = avc_has_perm_noaudit(ssid, tsid, tclass, requested, 0, &avd);

	rc2 = avc_audit(ssid, tsid, tclass, requested, &avd, rc, auditdata,
			flags);
	if (rc2)
		return rc2;
	return rc;
}

u32 avc_policy_seqno(void)
{
	return avc_cache.latest_notif;
}

void avc_disable(void)
{
	/*
	 * If you are looking at this because you have realized that we are
	 * not destroying the avc_node_cachep it might be easy to fix, but
	 * I don't know the memory barrier semantics well enough to know.  It's
	 * possible that some other task dereferenced security_ops when
	 * it still pointed to selinux operations.  If that is the case it's
	 * possible that it is about to use the avc and is about to need the
	 * avc_node_cachep.  I know I could wrap the security.c security_ops call
	 * in an rcu_lock, but seriously, it's not worth it.  Instead I just flush
	 * the cache and get that memory back.
	 */
	if (avc_node_cachep) {
		avc_flush();
		/* kmem_cache_destroy(avc_node_cachep); */
	}
}
