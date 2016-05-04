/*
 * AppArmor security module
 *
 * This file contains AppArmor /sys/kernel/security/apparmor interface functions
 *
 * Copyright (C) 1998-2008 Novell/SUSE
 * Copyright 2009-2010 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2 of the
 * License.
 */

#include <linux/ctype.h>
#include <linux/security.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/mount.h>
#include <linux/namei.h>
#include <linux/capability.h>
#include <linux/rcupdate.h>

#include "include/apparmor.h"
#include "include/apparmorfs.h"
#include "include/audit.h"
#include "include/context.h"
#include "include/crypto.h"
#include "include/ipc.h"
#include "include/policy.h"
#include "include/resource.h"

/**
 * aa_mangle_name - mangle a profile name to std profile layout form
 * @name: profile name to mangle  (NOT NULL)
 * @target: buffer to store mangled name, same length as @name (MAYBE NULL)
 *
 * Returns: length of mangled name
 */
static int mangle_name(const char *name, char *target)
{
	char *t = target;

	while (*name == '/' || *name == '.')
		name++;

	if (target) {
		for (; *name; name++) {
			if (*name == '/')
				*(t)++ = '.';
			else if (isspace(*name))
				*(t)++ = '_';
			else if (isalnum(*name) || strchr("._-", *name))
				*(t)++ = *name;
		}

		*t = 0;
	} else {
		int len = 0;
		for (; *name; name++) {
			if (isalnum(*name) || isspace(*name) ||
			    strchr("/._-", *name))
				len++;
		}

		return len;
	}

	return t - target;
}

/**
 * aa_simple_write_to_buffer - common routine for getting policy from user
 * @op: operation doing the user buffer copy
 * @userbuf: user buffer to copy data from  (NOT NULL)
 * @alloc_size: size of user buffer (REQUIRES: @alloc_size >= @copy_size)
 * @copy_size: size of data to copy from user buffer
 * @pos: position write is at in the file (NOT NULL)
 *
 * Returns: kernel buffer containing copy of user buffer data or an
 *          ERR_PTR on failure.
 */
static char *aa_simple_write_to_buffer(int op, const char __user *userbuf,
				       size_t alloc_size, size_t copy_size,
				       loff_t *pos)
{
	char *data;

	BUG_ON(copy_size > alloc_size);

	if (*pos != 0)
		/* only writes from pos 0, that is complete writes */
		return ERR_PTR(-ESPIPE);

	/*
	 * Don't allow profile load/replace/remove from profiles that don't
	 * have CAP_MAC_ADMIN
	 */
	if (!aa_may_manage_policy(op))
		return ERR_PTR(-EACCES);

	/* freed by caller to simple_write_to_buffer */
	data = kvmalloc(alloc_size);
	if (data == NULL)
		return ERR_PTR(-ENOMEM);

	if (copy_from_user(data, userbuf, copy_size)) {
		kvfree(data);
		return ERR_PTR(-EFAULT);
	}

	return data;
}


/* .load file hook fn to load policy */
static ssize_t profile_load(struct file *f, const char __user *buf, size_t size,
			    loff_t *pos)
{
	char *data;
	ssize_t error;

	data = aa_simple_write_to_buffer(OP_PROF_LOAD, buf, size, size, pos);

	error = PTR_ERR(data);
	if (!IS_ERR(data)) {
		error = aa_replace_profiles(data, size, PROF_ADD);
		kvfree(data);
	}

	return error;
}

static const struct file_operations aa_fs_profile_load = {
	.write = profile_load,
	.llseek = default_llseek,
};

/* .replace file hook fn to load and/or replace policy */
static ssize_t profile_replace(struct file *f, const char __user *buf,
			       size_t size, loff_t *pos)
{
	char *data;
	ssize_t error;

	data = aa_simple_write_to_buffer(OP_PROF_REPL, buf, size, size, pos);
	error = PTR_ERR(data);
	if (!IS_ERR(data)) {
		error = aa_replace_profiles(data, size, PROF_REPLACE);
		kvfree(data);
	}

	return error;
}

static const struct file_operations aa_fs_profile_replace = {
	.write = profile_replace,
	.llseek = default_llseek,
};

/* .remove file hook fn to remove loaded policy */
static ssize_t profile_remove(struct file *f, const char __user *buf,
			      size_t size, loff_t *pos)
{
	char *data;
	ssize_t error;

	/*
	 * aa_remove_profile needs a null terminated string so 1 extra
	 * byte is allocated and the copied data is null terminated.
	 */
	data = aa_simple_write_to_buffer(OP_PROF_RM, buf, size + 1, size, pos);

	error = PTR_ERR(data);
	if (!IS_ERR(data)) {
		data[size] = 0;
		error = aa_remove_profiles(data, size);
		kvfree(data);
	}

	return error;
}

static const struct file_operations aa_fs_profile_remove = {
	.write = profile_remove,
	.llseek = default_llseek,
};

/**
 * query_label - queries a label and writes permissions to buf
 * @buf: the resulting permissions string is stored here (NOT NULL)
 * @buf_len: size of buf
 * @query: binary query string to match against the dfa
 * @query_len: size of query
 *
 * The buffers pointed to by buf and query may overlap. The query buffer is
 * parsed before buf is written to.
 *
 * The query should look like "LABEL_NAME\0DFA_STRING" where LABEL_NAME is
 * the name of the label, in the current namespace, that is to be queried and
 * DFA_STRING is a binary string to match against the label(s)'s DFA.
 *
 * LABEL_NAME must be NUL terminated. DFA_STRING may contain NUL characters
 * but must *not* be NUL terminated.
 *
 * Returns: number of characters written to buf or -errno on failure
 */
static ssize_t query_label(char *buf, size_t buf_len,
			   char *query, size_t query_len)
{
	struct aa_profile *profile;
	struct aa_label *label;
	char *label_name, *match_str;
	size_t label_name_len, match_len;
	struct aa_perms perms;
	unsigned int state = 0;
	struct label_it i;

	if (!query_len)
		return -EINVAL;

	label_name = query;
	label_name_len = strnlen(query, query_len);
	if (!label_name_len || label_name_len == query_len)
		return -EINVAL;

	/**
	 * The extra byte is to account for the null byte between the
	 * profile name and dfa string. profile_name_len is greater
	 * than zero and less than query_len, so a byte can be safely
	 * added or subtracted.
	 */
	match_str = label_name + label_name_len + 1;
	match_len = query_len - label_name_len - 1;

	label = aa_label_parse(aa_current_label(), label_name, GFP_KERNEL,
			       false);
	if (IS_ERR(label))
		return PTR_ERR(label);

	aa_perms_all(&perms);
	label_for_each_confined(i, label, profile) {
		struct aa_perms tmp;
		struct aa_dfa *dfa;
		if (profile->file.dfa && *match_str == AA_CLASS_FILE) {
			dfa = profile->file.dfa;
			state = aa_dfa_match_len(dfa, profile->file.start,
						 match_str + 1, match_len - 1);
		} else if (profile->policy.dfa) {
			if (!PROFILE_MEDIATES_SAFE(profile, *match_str))
				continue;	/* no change to current perms */
			dfa = profile->policy.dfa;
			state = aa_dfa_match_len(dfa, profile->policy.start[0],
						 match_str, match_len);
		}
		if (state)
			aa_compute_perms(dfa, state, &tmp);
		else
			aa_perms_clear(&tmp);
		aa_apply_modes_to_perms(profile, &tmp);
		aa_perms_accum_raw(&perms, &tmp);
	}
	aa_put_label(label);

	return scnprintf(buf, buf_len,
		      "allow 0x%08x\ndeny 0x%08x\naudit 0x%08x\nquiet 0x%08x\n",
		      perms.allow, perms.deny, perms.audit, perms.quiet);
}

#define QUERY_CMD_LABEL		"label\0"
#define QUERY_CMD_LABEL_LEN	6
#define QUERY_CMD_PROFILE	"profile\0"
#define QUERY_CMD_PROFILE_LEN	8

/**
 * aa_write_access - generic permissions query
 * @file: pointer to open apparmorfs/access file
 * @ubuf: user buffer containing the complete query string (NOT NULL)
 * @count: size of ubuf
 * @ppos: position in the file (MUST BE ZERO)
 *
 * Allows for one permission query per open(), write(), and read() sequence.
 * The only query currently supported is a label-based query. For this query
 * ubuf must begin with "label\0", followed by the profile query specific
 * format described in the query_label() function documentation.
 *
 * Returns: number of bytes written or -errno on failure
 */
static ssize_t aa_write_access(struct file *file, const char __user *ubuf,
			       size_t count, loff_t *ppos)
{
	char *buf;
	ssize_t len;

	if (*ppos)
		return -ESPIPE;

	buf = simple_transaction_get(file, ubuf, count);
	if (IS_ERR(buf))
		return PTR_ERR(buf);

	if (count > QUERY_CMD_PROFILE_LEN &&
	    !memcmp(buf, QUERY_CMD_PROFILE, QUERY_CMD_PROFILE_LEN)) {
		len = query_label(buf, SIMPLE_TRANSACTION_LIMIT,
				  buf + QUERY_CMD_PROFILE_LEN,
				  count - QUERY_CMD_PROFILE_LEN);
	} else if (count > QUERY_CMD_LABEL_LEN &&
		   !memcmp(buf, QUERY_CMD_LABEL, QUERY_CMD_LABEL_LEN)) {
		len = query_label(buf, SIMPLE_TRANSACTION_LIMIT,
				  buf + QUERY_CMD_LABEL_LEN,
				  count - QUERY_CMD_LABEL_LEN);
	} else
		len = -EINVAL;

	if (len < 0)
		return len;

	simple_transaction_set(file, len);

	return count;
}

static const struct file_operations aa_fs_access = {
	.write		= aa_write_access,
	.read		= simple_transaction_read,
	.release	= simple_transaction_release,
	.llseek		= generic_file_llseek,
};

static int aa_fs_seq_show(struct seq_file *seq, void *v)
{
	struct aa_fs_entry *fs_file = seq->private;

	if (!fs_file)
		return 0;

	switch (fs_file->v_type) {
	case AA_FS_TYPE_BOOLEAN:
		seq_printf(seq, "%s\n", fs_file->v.boolean ? "yes" : "no");
		break;
	case AA_FS_TYPE_STRING:
		seq_printf(seq, "%s\n", fs_file->v.string);
		break;
	case AA_FS_TYPE_U64:
		seq_printf(seq, "%#08lx\n", fs_file->v.u64);
		break;
	default:
		/* Ignore unpritable entry types. */
		break;
	}

	return 0;
}

static int aa_fs_seq_open(struct inode *inode, struct file *file)
{
	return single_open(file, aa_fs_seq_show, inode->i_private);
}

const struct file_operations aa_fs_seq_file_ops = {
	.owner		= THIS_MODULE,
	.open		= aa_fs_seq_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int aa_fs_seq_profile_open(struct inode *inode, struct file *file,
				  int (*show)(struct seq_file *, void *))
{
	struct aa_replacedby *r = aa_get_replacedby(inode->i_private);
	int error = single_open(file, show, r);

	if (error) {
		file->private_data = NULL;
		aa_put_replacedby(r);
	}

	return error;
}

static int aa_fs_seq_profile_release(struct inode *inode, struct file *file)
{
	struct seq_file *seq = (struct seq_file *) file->private_data;
	if (seq)
		aa_put_replacedby(seq->private);
	return single_release(inode, file);
}

static int aa_fs_seq_profname_show(struct seq_file *seq, void *v)
{
	struct aa_replacedby *r = seq->private;
	struct aa_label *label = aa_get_label_rcu(&r->label);
	struct aa_profile *profile = labels_profile(label);
	seq_printf(seq, "%s\n", profile->base.name);
	aa_put_label(label);

	return 0;
}

static int aa_fs_seq_profname_open(struct inode *inode, struct file *file)
{
	return aa_fs_seq_profile_open(inode, file, aa_fs_seq_profname_show);
}

static const struct file_operations aa_fs_profname_fops = {
	.owner		= THIS_MODULE,
	.open		= aa_fs_seq_profname_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= aa_fs_seq_profile_release,
};

static int aa_fs_seq_profmode_show(struct seq_file *seq, void *v)
{
	struct aa_replacedby *r = seq->private;
	struct aa_label *label = aa_get_label_rcu(&r->label);
	struct aa_profile *profile = labels_profile(label);
	seq_printf(seq, "%s\n", aa_profile_mode_names[profile->mode]);
	aa_put_label(label);

	return 0;
}

static int aa_fs_seq_profmode_open(struct inode *inode, struct file *file)
{
	return aa_fs_seq_profile_open(inode, file, aa_fs_seq_profmode_show);
}

static const struct file_operations aa_fs_profmode_fops = {
	.owner		= THIS_MODULE,
	.open		= aa_fs_seq_profmode_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= aa_fs_seq_profile_release,
};

static int aa_fs_seq_profattach_show(struct seq_file *seq, void *v)
{
	struct aa_replacedby *r = seq->private;
	struct aa_label *label = aa_get_label_rcu(&r->label);
	struct aa_profile *profile = labels_profile(label);
	if (profile->attach)
		seq_printf(seq, "%s\n", profile->attach);
	else if (profile->xmatch)
		seq_puts(seq, "<unknown>\n");
	else
		seq_printf(seq, "%s\n", profile->base.name);
	aa_put_label(label);

	return 0;
}

static int aa_fs_seq_profattach_open(struct inode *inode, struct file *file)
{
	return aa_fs_seq_profile_open(inode, file, aa_fs_seq_profattach_show);
}

static const struct file_operations aa_fs_profattach_fops = {
	.owner		= THIS_MODULE,
	.open		= aa_fs_seq_profattach_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= aa_fs_seq_profile_release,
};

static int aa_fs_seq_hash_show(struct seq_file *seq, void *v)
{
	struct aa_replacedby *r = seq->private;
	struct aa_label *label = aa_get_label_rcu(&r->label);
	struct aa_profile *profile = labels_profile(label);
	unsigned int i, size = aa_hash_size();

	if (profile->hash) {
		for (i = 0; i < size; i++)
			seq_printf(seq, "%.2x", profile->hash[i]);
		seq_puts(seq, "\n");
	}

	return 0;
}

static int aa_fs_seq_hash_open(struct inode *inode, struct file *file)
{
	return single_open(file, aa_fs_seq_hash_show, inode->i_private);
}

static const struct file_operations aa_fs_seq_hash_fops = {
	.owner		= THIS_MODULE,
	.open		= aa_fs_seq_hash_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

/** fns to setup dynamic per profile/namespace files **/

/**
 *
 * Requires: @profile->ns->lock held
 */
void __aa_fs_profile_rmdir(struct aa_profile *profile)
{
	struct aa_profile *child;
	int i;

	if (!profile)
		return;
	AA_BUG(!mutex_is_locked(&profiles_ns(profile)->lock));

	list_for_each_entry(child, &profile->base.profiles, base.list)
		__aa_fs_profile_rmdir(child);

	for (i = AAFS_PROF_SIZEOF - 1; i >= 0; --i) {
		struct aa_replacedby *r;
		if (!profile->dents[i])
			continue;

		r = profile->dents[i]->d_inode->i_private;
		securityfs_remove(profile->dents[i]);
		aa_put_replacedby(r);
		profile->dents[i] = NULL;
	}
}

/**
 *
 * Requires: @old->ns->lock held
 */
void __aa_fs_profile_migrate_dents(struct aa_profile *old,
				   struct aa_profile *new)
{
	int i;

	AA_BUG(!old);
	AA_BUG(!new);
	AA_BUG(!mutex_is_locked(&profiles_ns(old)->lock));

	for (i = 0; i < AAFS_PROF_SIZEOF; i++) {
		new->dents[i] = old->dents[i];
		if (new->dents[i])
			new->dents[i]->d_inode->i_mtime = CURRENT_TIME;
		old->dents[i] = NULL;
	}
}

static struct dentry *create_profile_file(struct dentry *dir, const char *name,
					  struct aa_profile *profile,
					  const struct file_operations *fops)
{
	struct aa_replacedby *r = aa_get_replacedby(profile->label.replacedby);
	struct dentry *dent;

	dent = securityfs_create_file(name, S_IFREG | 0444, dir, r, fops);
	if (IS_ERR(dent))
		aa_put_replacedby(r);

	return dent;
}

/**
 *
 * Requires: @profile->ns->lock held
 */
int __aa_fs_profile_mkdir(struct aa_profile *profile, struct dentry *parent)
{
	struct aa_profile *child;
	struct dentry *dent = NULL, *dir;
	int error;

	AA_BUG(!profile);
	AA_BUG(!mutex_is_locked(&profiles_ns(profile)->lock));

	if (!parent) {
		struct aa_profile *p;
		p = aa_deref_parent(profile);
		dent = prof_dir(p);
		/* adding to parent that previously didn't have children */
		dent = securityfs_create_dir("profiles", dent);
		if (IS_ERR(dent))
			goto fail;
		prof_child_dir(p) = parent = dent;
	}

	if (!profile->dirname) {
		int len, id_len;
		len = mangle_name(profile->base.name, NULL);
		id_len = snprintf(NULL, 0, ".%ld", profile->ns->uniq_id);

		profile->dirname = kmalloc(len + id_len + 1, GFP_KERNEL);
		if (!profile->dirname)
			goto fail;

		mangle_name(profile->base.name, profile->dirname);
		sprintf(profile->dirname + len, ".%ld", profile->ns->uniq_id++);
	}

	dent = securityfs_create_dir(profile->dirname, parent);
	if (IS_ERR(dent))
		goto fail;
	prof_dir(profile) = dir = dent;

	dent = create_profile_file(dir, "name", profile, &aa_fs_profname_fops);
	if (IS_ERR(dent))
		goto fail;
	profile->dents[AAFS_PROF_NAME] = dent;

	dent = create_profile_file(dir, "mode", profile, &aa_fs_profmode_fops);
	if (IS_ERR(dent))
		goto fail;
	profile->dents[AAFS_PROF_MODE] = dent;

	dent = create_profile_file(dir, "attach", profile,
				   &aa_fs_profattach_fops);
	if (IS_ERR(dent))
		goto fail;
	profile->dents[AAFS_PROF_ATTACH] = dent;

	if (profile->hash) {
		dent = create_profile_file(dir, "sha1", profile,
					   &aa_fs_seq_hash_fops);
		if (IS_ERR(dent))
			goto fail;
		profile->dents[AAFS_PROF_HASH] = dent;
	}

	list_for_each_entry(child, &profile->base.profiles, base.list) {
		error = __aa_fs_profile_mkdir(child, prof_child_dir(profile));
		if (error)
			goto fail2;
	}

	return 0;

fail:
	error = PTR_ERR(dent);

fail2:
	__aa_fs_profile_rmdir(profile);

	return error;
}

/**
 *
 * Requires: @ns->lock held
 */
void __aa_fs_namespace_rmdir(struct aa_namespace *ns)
{
	struct aa_namespace *sub;
	struct aa_profile *child;
	int i;

	if (!ns)
		return;
	AA_BUG(!mutex_is_locked(&ns->lock));

	list_for_each_entry(child, &ns->base.profiles, base.list)
		__aa_fs_profile_rmdir(child);

	list_for_each_entry(sub, &ns->sub_ns, base.list) {
		mutex_lock(&sub->lock);
		__aa_fs_namespace_rmdir(sub);
		mutex_unlock(&sub->lock);
	}

	for (i = AAFS_NS_SIZEOF - 1; i >= 0; --i) {
		securityfs_remove(ns->dents[i]);
		ns->dents[i] = NULL;
	}
}

/**
 *
 * Requires: @ns->lock held
 */
int __aa_fs_namespace_mkdir(struct aa_namespace *ns, struct dentry *parent,
			    const char *name)
{
	struct aa_namespace *sub;
	struct aa_profile *child;
	struct dentry *dent, *dir;
	int error;

	AA_BUG(!ns);
	AA_BUG(!parent);
	AA_BUG(!mutex_is_locked(&ns->lock));

	if (!name)
		name = ns->base.name;

	dent = securityfs_create_dir(name, parent);
	if (IS_ERR(dent))
		goto fail;
	ns_dir(ns) = dir = dent;

	dent = securityfs_create_dir("profiles", dir);
	if (IS_ERR(dent))
		goto fail;
	ns_subprofs_dir(ns) = dent;

	dent = securityfs_create_dir("namespaces", dir);
	if (IS_ERR(dent))
		goto fail;
	ns_subns_dir(ns) = dent;

	list_for_each_entry(child, &ns->base.profiles, base.list) {
		error = __aa_fs_profile_mkdir(child, ns_subprofs_dir(ns));
		if (error)
			goto fail2;
	}

	list_for_each_entry(sub, &ns->sub_ns, base.list) {
		mutex_lock(&sub->lock);
		error = __aa_fs_namespace_mkdir(sub, ns_subns_dir(ns), NULL);
		mutex_unlock(&sub->lock);
		if (error)
			goto fail2;
	}

	return 0;

fail:
	error = PTR_ERR(dent);

fail2:
	__aa_fs_namespace_rmdir(ns);

	return error;
}


#define list_entry_next(pos, member) \
	list_entry(pos->member.next, typeof(*pos), member)
#define list_entry_is_head(pos, head, member) (&pos->member == (head))

/**
 * __next_namespace - find the next namespace to list
 * @root: root namespace to stop search at (NOT NULL)
 * @ns: current ns position (NOT NULL)
 *
 * Find the next namespace from @ns under @root and handle all locking needed
 * while switching current namespace.
 *
 * Returns: next namespace or NULL if at last namespace under @root
 * Requires: ns->parent->lock to be held
 * NOTE: will not unlock root->lock
 */
static struct aa_namespace *__next_namespace(struct aa_namespace *root,
					     struct aa_namespace *ns)
{
	struct aa_namespace *parent, *next;

	AA_BUG(!root);
	AA_BUG(!ns);
	AA_BUG(ns != root && !mutex_is_locked(&ns->parent->lock));

	/* is next namespace a child */
	if (!list_empty(&ns->sub_ns)) {
		next = list_first_entry(&ns->sub_ns, typeof(*ns), base.list);
		mutex_lock(&next->lock);
		return next;
	}

	/* check if the next ns is a sibling, parent, gp, .. */
	parent = ns->parent;
	while (ns != root) {
		mutex_unlock(&ns->lock);
		next = list_entry_next(ns, base.list);
		if (!list_entry_is_head(next, &parent->sub_ns, base.list)) {
			mutex_lock(&next->lock);
			return next;
		}
		ns = parent;
		parent = parent->parent;
	}

	return NULL;
}

/**
 * __first_profile - find the first profile in a namespace
 * @root: namespace that is root of profiles being displayed (NOT NULL)
 * @ns: namespace to start in   (MAY BE NULL)
 *
 * Returns: unrefcounted profile or NULL if no profile
 * Requires: ns.lock to be held
 */
static struct aa_profile *__first_profile(struct aa_namespace *root,
					  struct aa_namespace *ns)
{
	AA_BUG(!root);
	AA_BUG(ns && !mutex_is_locked(&ns->lock));

	for (; ns; ns = __next_namespace(root, ns)) {
		if (!list_empty(&ns->base.profiles))
			return list_first_entry(&ns->base.profiles,
						struct aa_profile, base.list);
	}
	return NULL;
}

/**
 * __next_profile - step to the next profile in a profile tree
 * @profile: current profile in tree (NOT NULL)
 *
 * Perform a depth first traversal on the profile tree in a namespace
 *
 * Returns: next profile or NULL if done
 * Requires: profile->ns.lock to be held
 */
static struct aa_profile *__next_profile(struct aa_profile *p)
{
	struct aa_profile *parent;
	struct aa_namespace *ns = p->ns;

	AA_BUG(!mutex_is_locked(&profiles_ns(p)->lock));

	/* is next profile a child */
	if (!list_empty(&p->base.profiles))
		return list_first_entry(&p->base.profiles, typeof(*p),
					base.list);

	/* is next profile a sibling, parent sibling, gp, sibling, .. */
	parent = rcu_dereference_protected(p->parent,
					   mutex_is_locked(&p->ns->lock));
	while (parent) {
		p = list_entry_next(p, base.list);
		if (!list_entry_is_head(p, &parent->base.profiles, base.list))
			return p;
		p = parent;
		parent = rcu_dereference_protected(parent->parent,
					    mutex_is_locked(&parent->ns->lock));
	}

	/* is next another profile in the namespace */
	p = list_entry_next(p, base.list);
	if (!list_entry_is_head(p, &ns->base.profiles, base.list))
		return p;

	return NULL;
}

/**
 * next_profile - step to the next profile in where ever it may be
 * @root: root namespace  (NOT NULL)
 * @profile: current profile  (NOT NULL)
 *
 * Returns: next profile or NULL if there isn't one
 */
static struct aa_profile *next_profile(struct aa_namespace *root,
				       struct aa_profile *profile)
{
	struct aa_profile *next = __next_profile(profile);
	if (next)
		return next;

	/* finished all profiles in namespace move to next namespace */
	return __first_profile(root, __next_namespace(root, profile->ns));
}

/**
 * p_start - start a depth first traversal of profile tree
 * @f: seq_file to fill
 * @pos: current position
 *
 * Returns: first profile under current namespace or NULL if none found
 *
 * acquires first ns->lock
 */
static void *p_start(struct seq_file *f, loff_t *pos)
{
	struct aa_profile *profile = NULL;
	struct aa_namespace *root = labels_ns(aa_current_label());
	loff_t l = *pos;
	f->private = aa_get_namespace(root);


	/* find the first profile */
	mutex_lock(&root->lock);
	profile = __first_profile(root, root);

	/* skip to position */
	for (; profile && l > 0; l--)
		profile = next_profile(root, profile);

	return profile;
}

/**
 * p_next - read the next profile entry
 * @f: seq_file to fill
 * @p: profile previously returned
 * @pos: current position
 *
 * Returns: next profile after @p or NULL if none
 *
 * may acquire/release locks in namespace tree as necessary
 */
static void *p_next(struct seq_file *f, void *p, loff_t *pos)
{
	struct aa_profile *profile = p;
	struct aa_namespace *ns = f->private;
	(*pos)++;

	return next_profile(ns, profile);
}

/**
 * p_stop - stop depth first traversal
 * @f: seq_file we are filling
 * @p: the last profile writen
 *
 * Release all locking done by p_start/p_next on namespace tree
 */
static void p_stop(struct seq_file *f, void *p)
{
	struct aa_profile *profile = p;
	struct aa_namespace *root = f->private, *ns;

	if (profile) {
		for (ns = profile->ns; ns && ns != root; ns = ns->parent)
			mutex_unlock(&ns->lock);
	}
	mutex_unlock(&root->lock);
	aa_put_namespace(root);
}

/**
 * seq_show_profile - show a profile entry
 * @f: seq_file to file
 * @p: current position (profile)    (NOT NULL)
 *
 * Returns: error on failure
 */
static int seq_show_profile(struct seq_file *f, void *p)
{
	struct aa_profile *profile = (struct aa_profile *)p;
	struct aa_namespace *root = f->private;

	if (profile->ns != root)
		seq_printf(f, ":%s://", aa_ns_name(root, profile->ns));
	seq_printf(f, "%s (%s)\n", profile->base.hname,
		   aa_profile_mode_names[profile->mode]);

	return 0;
}

static const struct seq_operations aa_fs_profiles_op = {
	.start = p_start,
	.next = p_next,
	.stop = p_stop,
	.show = seq_show_profile,
};

static int profiles_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &aa_fs_profiles_op);
}

static int profiles_release(struct inode *inode, struct file *file)
{
	return seq_release(inode, file);
}

static const struct file_operations aa_fs_profiles_fops = {
	.open = profiles_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = profiles_release,
};


/** Base file system setup **/
static struct aa_fs_entry aa_fs_entry_file[] = {
	AA_FS_FILE_STRING("mask", "create read write exec append mmap_exec " \
				  "link lock"),
	{ }
};

static struct aa_fs_entry aa_fs_entry_ptrace[] = {
	AA_FS_FILE_STRING("mask", "read trace"),
	{ }
};

static struct aa_fs_entry aa_fs_entry_signal[] = {
	AA_FS_FILE_STRING("mask", AA_FS_SIG_MASK),
	{ }
};

static struct aa_fs_entry aa_fs_entry_domain[] = {
	AA_FS_FILE_BOOLEAN("change_hat",	1),
	AA_FS_FILE_BOOLEAN("change_hatv",	1),
	AA_FS_FILE_BOOLEAN("change_onexec",	1),
	AA_FS_FILE_BOOLEAN("change_profile",	1),
	{ }
};

static struct aa_fs_entry aa_fs_entry_versions[] = {
	AA_FS_FILE_BOOLEAN("v5",	1),
	AA_FS_FILE_BOOLEAN("v6",	1),
	AA_FS_FILE_BOOLEAN("v7",	1),
	{ }
};

static struct aa_fs_entry aa_fs_entry_policy[] = {
	AA_FS_DIR("versions",                   aa_fs_entry_versions),
	AA_FS_FILE_BOOLEAN("set_load",		1),
	{ }
};

static struct aa_fs_entry aa_fs_entry_mount[] = {
	AA_FS_FILE_STRING("mask", "mount umount"),
	{ }
};

static struct aa_fs_entry aa_fs_entry_namespaces[] = {
	AA_FS_FILE_BOOLEAN("profile",		1),
	AA_FS_FILE_BOOLEAN("pivot_root",	1),
	{ }
};

static struct aa_fs_entry aa_fs_entry_dbus[] = {
	AA_FS_FILE_STRING("mask", "acquire send receive"),
	{ }
};

static struct aa_fs_entry aa_fs_entry_features[] = {
	AA_FS_DIR("policy",			aa_fs_entry_policy),
	AA_FS_DIR("domain",			aa_fs_entry_domain),
	AA_FS_DIR("file",			aa_fs_entry_file),
	AA_FS_DIR("network",			aa_fs_entry_network),
	AA_FS_DIR("mount",			aa_fs_entry_mount),
	AA_FS_DIR("namespaces",			aa_fs_entry_namespaces),
	AA_FS_FILE_U64("capability",		VFS_CAP_FLAGS_MASK),
	AA_FS_DIR("rlimit",			aa_fs_entry_rlimit),
	AA_FS_DIR("caps",			aa_fs_entry_caps),
	AA_FS_DIR("ptrace",			aa_fs_entry_ptrace),
	AA_FS_DIR("signal",			aa_fs_entry_signal),
	AA_FS_DIR("dbus",			aa_fs_entry_dbus),
	{ }
};

static struct aa_fs_entry aa_fs_entry_apparmor[] = {
	AA_FS_FILE_FOPS(".load", 0640, &aa_fs_profile_load),
	AA_FS_FILE_FOPS(".replace", 0640, &aa_fs_profile_replace),
	AA_FS_FILE_FOPS(".remove", 0640, &aa_fs_profile_remove),
	AA_FS_FILE_FOPS(".access", 0666, &aa_fs_access),
	AA_FS_FILE_FOPS("profiles", 0640, &aa_fs_profiles_fops),
	AA_FS_DIR("features", aa_fs_entry_features),
	{ }
};

static struct aa_fs_entry aa_fs_entry =
	AA_FS_DIR("apparmor", aa_fs_entry_apparmor);

/**
 * aafs_create_file - create a file entry in the apparmor securityfs
 * @fs_file: aa_fs_entry to build an entry for (NOT NULL)
 * @parent: the parent dentry in the securityfs
 *
 * Use aafs_remove_file to remove entries created with this fn.
 */
static int __init aafs_create_file(struct aa_fs_entry *fs_file,
				   struct dentry *parent)
{
	int error = 0;

	fs_file->dentry = securityfs_create_file(fs_file->name,
						 S_IFREG | fs_file->mode,
						 parent, fs_file,
						 fs_file->file_ops);
	if (IS_ERR(fs_file->dentry)) {
		error = PTR_ERR(fs_file->dentry);
		fs_file->dentry = NULL;
	}
	return error;
}

static void __init aafs_remove_dir(struct aa_fs_entry *fs_dir);
/**
 * aafs_create_dir - recursively create a directory entry in the securityfs
 * @fs_dir: aa_fs_entry (and all child entries) to build (NOT NULL)
 * @parent: the parent dentry in the securityfs
 *
 * Use aafs_remove_dir to remove entries created with this fn.
 */
static int __init aafs_create_dir(struct aa_fs_entry *fs_dir,
				  struct dentry *parent)
{
	struct aa_fs_entry *fs_file;
	struct dentry *dir;
	int error;

	dir = securityfs_create_dir(fs_dir->name, parent);
	if (IS_ERR(dir))
		return PTR_ERR(dir);
	fs_dir->dentry = dir;

	for (fs_file = fs_dir->v.files; fs_file && fs_file->name; ++fs_file) {
		if (fs_file->v_type == AA_FS_TYPE_DIR)
			error = aafs_create_dir(fs_file, fs_dir->dentry);
		else
			error = aafs_create_file(fs_file, fs_dir->dentry);
		if (error)
			goto failed;
	}

	return 0;

failed:
	aafs_remove_dir(fs_dir);

	return error;
}

/**
 * aafs_remove_file - drop a single file entry in the apparmor securityfs
 * @fs_file: aa_fs_entry to detach from the securityfs (NOT NULL)
 */
static void __init aafs_remove_file(struct aa_fs_entry *fs_file)
{
	if (!fs_file->dentry)
		return;

	securityfs_remove(fs_file->dentry);
	fs_file->dentry = NULL;
}

/**
 * aafs_remove_dir - recursively drop a directory entry from the securityfs
 * @fs_dir: aa_fs_entry (and all child entries) to detach (NOT NULL)
 */
static void __init aafs_remove_dir(struct aa_fs_entry *fs_dir)
{
	struct aa_fs_entry *fs_file;

	for (fs_file = fs_dir->v.files; fs_file && fs_file->name; ++fs_file) {
		if (fs_file->v_type == AA_FS_TYPE_DIR)
			aafs_remove_dir(fs_file);
		else
			aafs_remove_file(fs_file);
	}

	aafs_remove_file(fs_dir);
}

/**
 * aa_destroy_aafs - cleanup and free aafs
 *
 * releases dentries allocated by aa_create_aafs
 */
void __init aa_destroy_aafs(void)
{
	aafs_remove_dir(&aa_fs_entry);
}


#define NULL_FILE_NAME ".null"
struct path aa_null;

static int aa_mk_null_file(struct dentry *parent)
{
	struct vfsmount *mount = NULL;
	struct dentry *dentry;
	struct inode *inode;
	int count = 0;
	int error = simple_pin_fs(parent->d_sb->s_type, &mount, &count);
	if (error)
		return error;

	mutex_lock(&parent->d_inode->i_mutex);
	dentry = lookup_one_len(NULL_FILE_NAME, parent, strlen(NULL_FILE_NAME));
	if (IS_ERR(dentry)) {
		error = PTR_ERR(dentry);
		goto out;
	}
	inode = new_inode(parent->d_inode->i_sb);
	if (!inode) {
		error = -ENOMEM;
		goto out1;
	}

	inode->i_ino = get_next_ino();
	inode->i_mode = S_IFCHR | S_IRUGO | S_IWUGO;
	inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
	init_special_inode(inode, S_IFCHR | S_IRUGO | S_IWUGO,
			   MKDEV(MEM_MAJOR, 3));
	d_instantiate(dentry, inode);
	aa_null.dentry = dget(dentry);
	aa_null.mnt = mntget(mount);

	error = 0;

out1:
	dput(dentry);
out:
	mutex_unlock(&parent->d_inode->i_mutex);
	simple_release_fs(&mount, &count);
	return error;
}

/**
 * aa_create_aafs - create the apparmor security filesystem
 *
 * dentries created here are released by aa_destroy_aafs
 *
 * Returns: error on failure
 */
static int __init aa_create_aafs(void)
{
	int error;

	if (!apparmor_initialized)
		return 0;

	if (aa_fs_entry.dentry) {
		AA_ERROR("%s: AppArmor securityfs already exists\n", __func__);
		return -EEXIST;
	}

	/* Populate fs tree. */
	error = aafs_create_dir(&aa_fs_entry, NULL);
	if (error)
		goto error;

	mutex_lock(&root_ns->lock);
	error = __aa_fs_namespace_mkdir(root_ns, aa_fs_entry.dentry,
					"policy");
	mutex_unlock(&root_ns->lock);

	if (error)
		goto error;

	error = aa_mk_null_file(aa_fs_entry.dentry);
	if (error)
		goto error;

	if (!aa_g_unconfined_init) {
		/* TODO: add default profile to apparmorfs */
	}

	/* Report that AppArmor fs is enabled */
	aa_info_message("AppArmor Filesystem Enabled");
	return 0;

error:
	aa_destroy_aafs();
	AA_ERROR("Error creating AppArmor securityfs\n");
	return error;
}

fs_initcall(aa_create_aafs);
