/*
 * AppArmor security module
 *
 * This file contains AppArmor policy manipulation functions
 *
 * Copyright (C) 1998-2008 Novell/SUSE
 * Copyright 2009-2010 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2 of the
 * License.
 *
 *
 * AppArmor policy is based around profiles, which contain the rules a
 * task is confined by.  Every task in the system has a profile attached
 * to it determined either by matching "unconfined" tasks against the
 * visible set of profiles or by following a profiles attachment rules.
 *
 * Each profile exists in a profile namespace which is a container of
 * visible profiles.  Each namespace contains a special "unconfined" profile,
 * which doesn't enforce any confinement on a task beyond DAC.
 *
 * Namespace and profile names can be written together in either
 * of two syntaxes.
 *	:namespace:profile - used by kernel interfaces for easy detection
 *	namespace://profile - used by policy
 *
 * Profile names can not start with : or @ or ^ and may not contain \0
 *
 * Reserved profile names
 *	unconfined - special automatically generated unconfined profile
 *	inherit - special name to indicate profile inheritance
 *	null-XXXX-YYYY - special automatically generated learning profiles
 *
 * Namespace names may not start with / or @ and may not contain \0 or :
 * Reserved namespace names
 *	user-XXXX - user defined profiles
 *
 * a // in a profile or namespace name indicates a hierarchical name with the
 * name before the // being the parent and the name after the child.
 *
 * Profile and namespace hierarchies serve two different but similar purposes.
 * The namespace contains the set of visible profiles that are considered
 * for attachment.  The hierarchy of namespaces allows for virtualizing
 * the namespace so that for example a chroot can have its own set of profiles
 * which may define some local user namespaces.
 * The profile hierarchy severs two distinct purposes,
 * -  it allows for sub profiles or hats, which allows an application to run
 *    subprograms under its own profile with different restriction than it
 *    self, and not have it use the system profile.
 *    eg. if a mail program starts an editor, the policy might make the
 *        restrictions tighter on the editor tighter than the mail program,
 *        and definitely different than general editor restrictions
 * - it allows for binary hierarchy of profiles, so that execution history
 *   is preserved.  This feature isn't exploited by AppArmor reference policy
 *   but is allowed.  NOTE: this is currently suboptimal because profile
 *   aliasing is not currently implemented so that a profile for each
 *   level must be defined.
 *   eg. /bin/bash///bin/ls as a name would indicate /bin/ls was started
 *       from /bin/bash
 *
 *   A profile or namespace name that can contain one or more // separators
 *   is referred to as an hname (hierarchical).
 *   eg.  /bin/bash//bin/ls
 *
 *   An fqname is a name that may contain both namespace and profile hnames.
 *   eg. :ns:/bin/bash//bin/ls
 *
 * NOTES:
 *   - locking of profile lists is currently fairly coarse.  All profile
 *     lists within a namespace use the namespace lock.
 * FIXME: move profile lists to using rcu_lists
 */

#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>

#include "include/apparmor.h"
#include "include/capability.h"
#include "include/context.h"
#include "include/file.h"
#include "include/ipc.h"
#include "include/label.h"
#include "include/match.h"
#include "include/path.h"
#include "include/policy.h"
#include "include/policy_unpack.h"
#include "include/resource.h"


/* root profile namespace */
struct aa_namespace *root_ns;

/* Note: mode names must be unique in the first character because of
 *       modechrs used to print modes on compound labels on some interfaces
 */
const char *const aa_profile_mode_names[] = {
	"enforce",
	"complain",
	"kill",
	"unconfined",
};

/**
 * hname_tail - find the last component of an hname
 * @name: hname to find the base profile name component of  (NOT NULL)
 *
 * Returns: the tail (base profile name) name component of an hname
 */
static const char *hname_tail(const char *hname)
{
	char *split;
	hname = strim((char *)hname);
	for (split = strstr(hname, "//"); split; split = strstr(hname, "//"))
		hname = split + 2;

	return hname;
}

/**
 * policy_init - initialize a policy structure
 * @policy: policy to initialize  (NOT NULL)
 * @prefix: prefix name if any is required.  (MAYBE NULL)
 * @name: name of the policy, init will make a copy of it  (NOT NULL)
 *
 * Note: this fn creates a copy of strings passed in
 *
 * Returns: true if policy init successful
 */
static bool policy_init(struct aa_policy *policy, const char *prefix,
			const char *name)
{
	char *hname;

	/* freed by policy_free */
	if (prefix) {
		hname = aa_str_alloc(strlen(prefix) + strlen(name) + 3,
				     GFP_KERNEL);
		if (hname)
			sprintf(hname, "%s//%s", prefix, name);
	} else {
		hname = aa_str_alloc(strlen(name) + 1, GFP_KERNEL);
		if (hname)
			strcpy(hname, name);
	}
	if (!hname)
		return 0;
	policy->hname = hname;
	/* base.name is a substring of fqname */
	policy->name = (char *)hname_tail(policy->hname);
	INIT_LIST_HEAD(&policy->list);
	INIT_LIST_HEAD(&policy->profiles);

	return 1;
}

/**
 * policy_destroy - free the elements referenced by @policy
 * @policy: policy that is to have its elements freed  (NOT NULL)
 */
static void policy_destroy(struct aa_policy *policy)
{
	/* still contains profiles -- invalid */
	if (on_list_rcu(&policy->profiles)) {
		AA_ERROR("%s: internal error, "
			 "policy '%s' still contains profiles\n",
			 __func__, policy->name);
		BUG();
	}
	if (on_list_rcu(&policy->list)) {
		AA_ERROR("%s: internal error, policy '%s' still on list\n",
			 __func__, policy->name);
		BUG();
	}

	/* don't free name as its a subset of hname */
	aa_put_str(policy->hname);
}

/**
 * __policy_find - find a policy by @name on a policy list
 * @head: list to search  (NOT NULL)
 * @name: name to search for  (NOT NULL)
 *
 * Requires: rcu_read_lock be held
 *
 * Returns: unrefcounted policy that match @name or NULL if not found
 */
static struct aa_policy *__policy_find(struct list_head *head, const char *name)
{
	struct aa_policy *policy;

	list_for_each_entry_rcu(policy, head, list) {
		if (!strcmp(policy->name, name))
			return policy;
	}
	return NULL;
}

/**
 * __policy_strn_find - find a policy that's name matches @len chars of @str
 * @head: list to search  (NOT NULL)
 * @str: string to search for  (NOT NULL)
 * @len: length of match required
 *
 * Requires: rcu_read_lock be held
 *
 * Returns: unrefcounted policy that match @str or NULL if not found
 *
 * if @len == strlen(@strlen) then this is equiv to __policy_find
 * other wise it allows searching for policy by a partial match of name
 */
static struct aa_policy *__policy_strn_find(struct list_head *head,
					    const char *str, int len)
{
	struct aa_policy *policy;

	list_for_each_entry_rcu(policy, head, list) {
		if (aa_strneq(policy->name, str, len))
			return policy;
	}

	return NULL;
}

/*
 * Routines for AppArmor namespaces
 */

const char *aa_hidden_ns_name = "---";
/**
 * aa_ns_visible - test if @view is visible from @curr
 * @curr: namespace to treat as the parent (NOT NULL)
 * @view:  namespace to test if visible from @curr (NOT NULL)
 *
 * Returns: true if @view is visible from @curr else false
 */
bool aa_ns_visible(struct aa_namespace *curr, struct aa_namespace *view)
{
	if (curr == view)
		return true;

	for ( ; view; view = view->parent) {
		if (view->parent == curr)
			return true;
	}
	return false;
}

/**
 * aa_na_name - Find the ns name to display for @view from @curr
 * @curr - current namespace (NOT NULL)
 * @view - namespace attempting to view (NOT NULL)
 *
 * Returns: name of @view visible from @curr
 */
const char *aa_ns_name(struct aa_namespace *curr, struct aa_namespace *view)
{
	/* if view == curr then the namespace name isn't displayed */
	if (curr == view)
		return "";

	if (aa_ns_visible(curr, view)) {
		/* at this point if a ns is visible it is in a view ns
		 * thus the curr ns.hname is a prefix of its name.
		 * Only output the virtualized portion of the name
		 * Add + 2 to skip over // separating curr hname prefix
		 * from the visible tail of the views hname
		 */
		return view->base.hname + strlen(curr->base.hname) + 2;
	} else
		return aa_hidden_ns_name;
}

/**
 * alloc_namespace - allocate, initialize and return a new namespace
 * @prefix: parent namespace name (MAYBE NULL)
 * @name: a preallocated name  (NOT NULL)
 *
 * Returns: refcounted namespace or NULL on failure.
 */
static struct aa_namespace *alloc_namespace(const char *prefix,
					    const char *name)
{
	struct aa_namespace *ns;

	ns = kzalloc(sizeof(*ns), GFP_KERNEL);
	AA_DEBUG("%s(%p)\n", __func__, ns);
	if (!ns)
		return NULL;
	if (!policy_init(&ns->base, prefix, name))
		goto fail_ns;

	INIT_LIST_HEAD(&ns->sub_ns);
	mutex_init(&ns->lock);

	/* released by free_namespace */
	ns->unconfined = aa_alloc_profile("unconfined");
	if (!ns->unconfined)
		goto fail_unconfined;
	ns->unconfined->label.replacedby = aa_alloc_replacedby(NULL);
	if (!ns->unconfined->label.replacedby)
		goto fail_replacedby;

	ns->unconfined->label.flags |= FLAG_IX_ON_NAME_ERROR |
		FLAG_IMMUTIBLE | FLAG_NS_COUNT | FLAG_UNCONFINED;
	ns->unconfined->mode = APPARMOR_UNCONFINED;

	/* ns and ns->unconfined share ns->unconfined refcount */
	ns->unconfined->ns = ns;

	atomic_set(&ns->uniq_null, 0);

	aa_labelset_init(&ns->labels);

	return ns;

fail_replacedby:
	aa_free_profile(ns->unconfined);

fail_unconfined:
	kzfree(ns->base.hname);
fail_ns:
	kzfree(ns);
	return NULL;
}

/**
 * aa_free_namespace - free a profile namespace
 * @ns: the namespace to free  (MAYBE NULL)
 *
 * Requires: All references to the namespace must have been put, if the
 *           namespace was referenced by a profile confining a task,
 */
void aa_free_namespace(struct aa_namespace *ns)
{
	if (!ns)
		return;

	policy_destroy(&ns->base);
	aa_labelset_destroy(&ns->labels);
	aa_put_namespace(ns->parent);

	ns->unconfined->ns = NULL;
	aa_free_profile(ns->unconfined);
	kzfree(ns);
}

/**
 * __aa_findn_namespace - find a namespace on a list by @name
 * @head: list to search for namespace on  (NOT NULL)
 * @name: name of namespace to look for  (NOT NULL)
 * @n: length of @name
 * Returns: unrefcounted namespace
 *
 * Requires: rcu_read_lock be held
 */
static struct aa_namespace *__aa_findn_namespace(struct list_head *head,
						 const char *name, size_t n)
{
	return (struct aa_namespace *)__policy_strn_find(head, name, n);
}

/**
 * aa_find_namespace  -  look up a profile namespace on the namespace list
 * @root: namespace to search in  (NOT NULL)
 * @name: name of namespace to find  (NOT NULL)
 * @n: length of @name
 *
 * Returns: a refcounted namespace on the list, or NULL if no namespace
 *          called @name exists.
 *
 * refcount released by caller
 */
struct aa_namespace *aa_findn_namespace(struct aa_namespace *root,
					const char *name, size_t n)
{
	struct aa_namespace *ns = NULL;

	rcu_read_lock();
	ns = aa_get_namespace(__aa_findn_namespace(&root->sub_ns, name, n));
	rcu_read_unlock();

	return ns;
}

/**
 * aa_find_namespace  -  look up a profile namespace on the namespace list
 * @root: namespace to search in  (NOT NULL)
 * @name: name of namespace to find  (NOT NULL)
 *
 * Returns: a refcounted namespace on the list, or NULL if no namespace
 *          called @name exists.
 *
 * refcount released by caller
 */
struct aa_namespace *aa_find_namespace(struct aa_namespace *root,
				       const char *name)
{
	return aa_findn_namespace(root, name, strlen(name));
}

/**
 * aa_prepare_namespace - find an existing or create a new namespace of @name
 * @name: the namespace to find or add  (MAYBE NULL)
 *
 * Returns: refcounted namespace or NULL if failed to create one
 */
static struct aa_namespace *aa_prepare_namespace(const char *name)
{
	struct aa_namespace *ns, *root;

	root = labels_ns(aa_current_label());

	mutex_lock(&root->lock);

	/* if name isn't specified the profile is loaded to the current ns */
	if (!name) {
		/* released by caller */
		ns = aa_get_namespace(root);
		goto out;
	}

	/* try and find the specified ns and if it doesn't exist create it */
	/* released by caller */
	ns = aa_get_namespace(__aa_findn_namespace(&root->sub_ns, name,
						   strlen(name)));
	if (!ns) {
		ns = alloc_namespace(root->base.hname, name);
		if (!ns)
			goto out;
		if (__aa_fs_namespace_mkdir(ns, ns_subns_dir(root), name)) {
			AA_ERROR("Failed to create interface for ns %s\n",
				 ns->base.name);
			aa_free_namespace(ns);
			ns = NULL;
			goto out;
		}
		ns->parent = aa_get_namespace(root);
		ns->level = root->level + 1;
		list_add_rcu(&ns->base.list, &root->sub_ns);
		/* add list ref */
		aa_get_namespace(ns);
	}
out:
	mutex_unlock(&root->lock);

	/* return ref */
	return ns;
}

/**
 * __add_profile - add a profiles to list and label tree
 * @list: list to add it to  (NOT NULL)
 * @profile: the profile to add  (NOT NULL)
 *
 * refcount @profile, should be put by __list_remove_profile
 *
 * Requires: namespace lock be held, or list not be shared
 */
static void __add_profile(struct list_head *list, struct aa_profile *profile)
{
	struct aa_label *l;

	AA_BUG(!list);
	AA_BUG(!profile);
	AA_BUG(!profile->ns);
	AA_BUG(!mutex_is_locked(&profile->ns->lock));

	list_add_rcu(&profile->base.list, list);
	/* get list reference */
	aa_get_profile(profile);
	l = aa_label_insert(&profile->ns->labels, &profile->label);
	AA_BUG(l != &profile->label);
	aa_put_label(l);
}

/**
 * __list_remove_profile - remove a profile from the list it is on
 * @profile: the profile to remove  (NOT NULL)
 *
 * remove a profile from the list, warning generally removal should
 * be done with __replace_profile as most profile removals are
 * replacements to the unconfined profile.
 *
 * put @profile list refcount
 *
 * Requires: namespace lock be held, or list not have been live
 */
static void __list_remove_profile(struct aa_profile *profile)
{
	AA_BUG(!profile);
	AA_BUG(!profile->ns);
	AA_BUG(!mutex_is_locked(&profile->ns->lock));

	list_del_rcu(&profile->base.list);
	aa_put_profile(profile);
}

static void __profile_list_release(struct list_head *head);

/**
 * __remove_profile - remove old profile, and children
 * @profile: profile to be replaced  (NOT NULL)
 *
 * Requires: namespace list lock be held, or list not be shared
 */
static void __remove_profile(struct aa_profile *profile)
{
	AA_BUG(!profile);
	AA_BUG(!profile->ns);
	AA_BUG(!mutex_is_locked(&profile->ns->lock));

	/* release any children lists first */
	__profile_list_release(&profile->base.profiles);
	/* released by free_profile */
	aa_label_remove(&profile->ns->labels, &profile->label);
	__aa_update_replacedby(&profile->label,
			       &profile->ns->unconfined->label);
	__aa_fs_profile_rmdir(profile);
	__list_remove_profile(profile);
}

/**
 * __profile_list_release - remove all profiles on the list and put refs
 * @head: list of profiles  (NOT NULL)
 *
 * Requires: namespace lock be held
 */
static void __profile_list_release(struct list_head *head)
{
	struct aa_profile *profile, *tmp;
	list_for_each_entry_safe(profile, tmp, head, base.list)
		__remove_profile(profile);
}

static void __ns_list_release(struct list_head *head);

/**
 * destroy_namespace - remove everything contained by @ns
 * @ns: namespace to have it contents removed  (NOT NULL)
 */
static void destroy_namespace(struct aa_namespace *ns)
{
	if (!ns)
		return;

	mutex_lock(&ns->lock);
	/* release all profiles in this namespace */
	__profile_list_release(&ns->base.profiles);

	/* release all sub namespaces */
	__ns_list_release(&ns->sub_ns);

	if (ns->parent)
		__aa_update_replacedby(&ns->unconfined->label,
				       &ns->parent->unconfined->label);
	__aa_fs_namespace_rmdir(ns);
	mutex_unlock(&ns->lock);
}

/**
 * __remove_namespace - remove a namespace and all its children
 * @ns: namespace to be removed  (NOT NULL)
 *
 * Requires: ns->parent->lock be held and ns removed from parent.
 */
static void __remove_namespace(struct aa_namespace *ns)
{
	/* remove ns from namespace list */
	list_del_rcu(&ns->base.list);
	destroy_namespace(ns);
	aa_put_namespace(ns);
}

/**
 * __ns_list_release - remove all profile namespaces on the list put refs
 * @head: list of profile namespaces  (NOT NULL)
 *
 * Requires: namespace lock be held
 */
static void __ns_list_release(struct list_head *head)
{
	struct aa_namespace *ns, *tmp;
	list_for_each_entry_safe(ns, tmp, head, base.list)
		__remove_namespace(ns);

}

/**
 * aa_alloc_root_ns - allocate the root profile namespace
 *
 * Returns: %0 on success else error
 *
 */
int __init aa_alloc_root_ns(void)
{
	/* released by aa_free_root_ns - used as list ref*/
	root_ns = alloc_namespace(NULL, "root");
	if (!root_ns)
		return -ENOMEM;

	return 0;
}

 /**
  * aa_free_root_ns - free the root profile namespace
  */
void __init aa_free_root_ns(void)
 {
	 struct aa_namespace *ns = root_ns;
	 root_ns = NULL;

	 destroy_namespace(ns);
	 aa_put_namespace(ns);
}


/**
 * aa_free_profile - free a profile
 * @profile: the profile to free  (MAYBE NULL)
 *
 * Free a profile, its hats and null_profile. All references to the profile,
 * its hats and null_profile must have been put.
 *
 * If the profile was referenced from a task context, free_profile() will
 * be called from an rcu callback routine, so we must not sleep here.
 */
void aa_free_profile(struct aa_profile *profile)
{
	AA_DEBUG("%s(%p)\n", __func__, profile);

	if (!profile)
		return;

	/* free children profiles */
	policy_destroy(&profile->base);
	aa_put_profile(rcu_access_pointer(profile->parent));

	aa_put_namespace(profile->ns);
	kzfree(profile->rename);

	aa_free_file_rules(&profile->file);
	aa_free_cap_rules(&profile->caps);
	aa_free_net_rules(&profile->net);
	aa_free_rlimit_rules(&profile->rlimits);

	kzfree(profile->dirname);
	aa_put_dfa(profile->xmatch);
	aa_put_dfa(profile->policy.dfa);

	kzfree(profile->hash);
	kzfree(profile);
}

/**
 * aa_alloc_profile - allocate, initialize and return a new profile
 * @hname: name of the profile  (NOT NULL)
 *
 * Returns: refcount profile or NULL on failure
 */
struct aa_profile *aa_alloc_profile(const char *hname)
{
	struct aa_profile *profile;

	/* freed by free_profile - usually through aa_put_profile */
	profile = kzalloc(sizeof(*profile), GFP_KERNEL);
	if (!profile)
		return NULL;

	if (!policy_init(&profile->base, NULL, hname))
		goto fail;
	if (!aa_label_init(&profile->label, 1))
		goto fail;
	profile->label.hname = profile->base.hname;
	profile->label.flags |= FLAG_PROFILE;
	profile->label.ent[0] = profile;

	/* refcount released by caller */
	return profile;

fail:
	kzfree(profile);

	return NULL;
}

/**
 * aa_new_null_profile - create a new null-X learning profile
 * @parent: profile that caused this profile to be created (NOT NULL)
 * @hat: true if the null- learning profile is a hat
 *
 * Create a null- complain mode profile used in learning mode.  The name of
 * the profile is unique and follows the format of parent//null-<uniq>.
 *
 * null profiles are added to the profile list but the list does not
 * hold a count on them so that they are automatically released when
 * not in use.
 *
 * Returns: new refcounted profile else NULL on failure
 */
struct aa_profile *aa_new_null_profile(struct aa_profile *parent, int hat)
{
	struct aa_profile *profile = NULL;
	char *name;
	int uniq = atomic_inc_return(&parent->ns->uniq_null);

	/* freed below */
	name = kmalloc(strlen(parent->base.hname) + 2 + 7 + 8, GFP_KERNEL);
	if (!name)
		goto fail;
	sprintf(name, "%s//null-%x", parent->base.hname, uniq);

	profile = aa_alloc_profile(name);
	kfree(name);
	if (!profile)
		goto fail;

	profile->label.replacedby = aa_alloc_replacedby(NULL);
	if (!profile->label.replacedby)
		goto fail;

	profile->mode = APPARMOR_COMPLAIN;
	profile->label.flags |= FLAG_NULL;
	if (hat)
		profile->label.flags |= FLAG_HAT;

	/* released on free_profile */
	rcu_assign_pointer(profile->parent, aa_get_profile(parent));
	profile->ns = aa_get_namespace(parent->ns);

	mutex_lock(&profile->ns->lock);
	__add_profile(&parent->base.profiles, profile);
	mutex_unlock(&profile->ns->lock);

	/* refcount released by caller */
	return profile;

fail:
	aa_free_profile(profile);
	return NULL;
}

/**
 * aa_setup_default_label - create the initial default label
 */
struct aa_label *aa_setup_default_label(void)
{
	struct aa_profile *profile = aa_alloc_profile("default");
	if (!profile)
		return NULL;

	/* the default profile pretends to be unconfined until it is replaced */
	profile->label.flags |= FLAG_IX_ON_NAME_ERROR | FLAG_UNCONFINED;
	profile->mode = APPARMOR_UNCONFINED;

	profile->ns = aa_get_namespace(root_ns);

	/* replacedby being set needed by fs interface */
	profile->label.replacedby = aa_alloc_replacedby(&profile->label);
	if (!profile->label.replacedby) {
		aa_free_profile(profile);
		return NULL;
	}
	__add_profile(&root_ns->base.profiles, profile);

	return &profile->label;
}

/* TODO: profile accounting - setup in remove */

/**
 * __find_child - find a profile on @head list with a name matching @name
 * @head: list to search  (NOT NULL)
 * @name: name of profile (NOT NULL)
 *
 * Requires: rcu_read_lock be held
 *
 * Returns: unrefcounted profile ptr, or NULL if not found
 */
static struct aa_profile *__find_child(struct list_head *head, const char *name)
{
	return (struct aa_profile *)__policy_find(head, name);
}

/**
 * __strn_find_child - find a profile on @head list using substring of @name
 * @head: list to search  (NOT NULL)
 * @name: name of profile (NOT NULL)
 * @len: length of @name substring to match
 *
 * Requires: rcu_read_lock be held
 *
 * Returns: unrefcounted profile ptr, or NULL if not found
 */
static struct aa_profile *__strn_find_child(struct list_head *head,
					    const char *name, int len)
{
	return (struct aa_profile *)__policy_strn_find(head, name, len);
}

/**
 * aa_find_child - find a profile by @name in @parent
 * @parent: profile to search  (NOT NULL)
 * @name: profile name to search for  (NOT NULL)
 *
 * Returns: a refcounted profile or NULL if not found
 */
struct aa_profile *aa_find_child(struct aa_profile *parent, const char *name)
{
	struct aa_profile *profile;

	rcu_read_lock();
	profile = aa_get_profile(__find_child(&parent->base.profiles, name));
	rcu_read_unlock();

	/* refcount released by caller */
	return profile;
}

/**
 * __lookup_parent - lookup the parent of a profile of name @hname
 * @ns: namespace to lookup profile in  (NOT NULL)
 * @hname: hierarchical profile name to find parent of  (NOT NULL)
 *
 * Lookups up the parent of a fully qualified profile name, the profile
 * that matches hname does not need to exist, in general this
 * is used to load a new profile.
 *
 * Requires: rcu_read_lock be held
 *
 * Returns: unrefcounted policy or NULL if not found
 */
static struct aa_policy *__lookup_parent(struct aa_namespace *ns,
					 const char *hname)
{
	struct aa_policy *policy;
	struct aa_profile *profile = NULL;
	char *split;

	policy = &ns->base;

	for (split = strstr(hname, "//"); split;) {
		profile = __strn_find_child(&policy->profiles, hname,
					    split - hname);
		if (!profile)
			return NULL;
		policy = &profile->base;
		hname = split + 2;
		split = strstr(hname, "//");
	}
	if (!profile)
		return &ns->base;
	return &profile->base;
}

/**
 * __lookupn_profile - lookup the profile matching @hname
 * @base: base list to start looking up profile name from  (NOT NULL)
 * @hname: hierarchical profile name  (NOT NULL)
 * @n: length of @hname
 *
 * Requires: rcu_read_lock be held
 *
 * Returns: unrefcounted profile pointer or NULL if not found
 *
 * Do a relative name lookup, recursing through profile tree.
 */
static struct aa_profile *__lookupn_profile(struct aa_policy *base,
					    const char *hname, size_t n)
{
	struct aa_profile *profile = NULL;
	const char *split, *name = hname;

	for (split = strstr(hname, "//"); split && (split - hname <= n);) {
		profile = __strn_find_child(&base->profiles, name,
					    split - name);
		if (!profile)
			return NULL;

		base = &profile->base;
		name = split + 2;
		split = strstr(name, "//");
	}

	if (name - hname <= n)
		return __strn_find_child(&base->profiles, name,
					 n - (name - hname));
	return NULL;
}

static struct aa_profile *__lookup_profile(struct aa_policy *base,
					   const char *hname)
{
	return __lookupn_profile(base, hname, strlen(hname));
}

/**
 * aa_lookup_profile - find a profile by its full or partial name
 * @ns: the namespace to start from (NOT NULL)
 * @hname: name to do lookup on.  Does not contain namespace prefix (NOT NULL)
 * @n: size of @hname
 *
 * Returns: refcounted profile or NULL if not found
 */
struct aa_profile *aa_lookupn_profile(struct aa_namespace *ns,
				      const char *hname, size_t n)
{
	struct aa_profile *profile;

	rcu_read_lock();
	do {
		profile = __lookupn_profile(&ns->base, hname, n);
	} while (profile && !aa_get_profile_not0(profile));
	rcu_read_unlock();

	/* the unconfined profile is not in the regular profile list */
	if (!profile && strncmp(hname, "unconfined", n) == 0)
		profile = aa_get_newest_profile(ns->unconfined);

	/* refcount released by caller */
	return profile;
}

struct aa_profile *aa_lookup_profile(struct aa_namespace *ns, const char *hname)
{
	return aa_lookupn_profile(ns, hname, strlen(hname));
}

struct aa_profile *aa_fqlookupn_profile(struct aa_label *base, char *fqname,
					size_t n)
{
	struct aa_profile *profile;
	struct aa_namespace *ns;
	char *name, *ns_name;
	size_t ns_len;

	name = aa_splitn_fqname(fqname, n, &ns_name, &ns_len);
	if (ns_name) {
		ns = aa_findn_namespace(labels_ns(base), ns_name, ns_len);
		if (!ns)
			return NULL;
	} else
		ns = aa_get_namespace(labels_ns(base));
	profile = aa_lookupn_profile(ns, name, n - (name - fqname));
	aa_put_namespace(ns);

       return profile;
}

/**
 * replacement_allowed - test to see if replacement is allowed
 * @profile: profile to test if it can be replaced  (MAYBE NULL)
 * @noreplace: true if replacement shouldn't be allowed but addition is okay
 * @info: Returns - info about why replacement failed (NOT NULL)
 *
 * Returns: %0 if replacement allowed else error code
 */
static int replacement_allowed(struct aa_profile *profile, int noreplace,
			       const char **info)
{
	if (profile) {
		if (profile->label.flags & FLAG_IMMUTIBLE) {
			*info = "cannot replace immutible profile";
			return -EPERM;
		} else if (noreplace) {
			*info = "profile already exists";
			return -EEXIST;
		}
	}
	return 0;
}

/**
 * aa_audit_policy - Do auditing of policy changes
 * @op: policy operation being performed
 * @name: name of profile being manipulated (NOT NULL)
 * @info: any extra information to be audited (MAYBE NULL)
 * @error: error code
 *
 * Returns: the error to be returned after audit is done
 */
static int audit_policy(int op, const char *name, const char *info,
			int error)
{
	DEFINE_AUDIT_DATA(sa, LSM_AUDIT_DATA_NONE, op);
	//	aad(&sa)->op = op;
	aad(&sa)->name = name;
	aad(&sa)->info = info;
	aad(&sa)->error = error;

	return aa_audit(AUDIT_APPARMOR_STATUS,
			labels_profile(aa_current_raw_label()), &sa, NULL);
}

/**
 * aa_may_manage_policy - can the current task manage policy
 * @op: the policy manipulation operation being done
 *
 * Returns: true if the task is allowed to manipulate policy
 */
bool aa_may_manage_policy(int op)
{
	/* check if loading policy is locked out */
	if (aa_g_lock_policy) {
		audit_policy(op, NULL, "policy_locked", -EACCES);
		return 0;
	}

	if (!capable(CAP_MAC_ADMIN)) {
		audit_policy(op, NULL, "not policy admin", -EACCES);
		return 0;
	}

	return 1;
}

static struct aa_profile *__list_lookup_parent(struct list_head *lh,
					       struct aa_profile *profile)
{
	const char *base = hname_tail(profile->base.hname);
	long len = base - profile->base.hname;
	struct aa_load_ent *ent;

	/* parent won't have trailing // so remove from len */
	if (len <= 2)
		return NULL;
	len -= 2;

	list_for_each_entry(ent, lh, list) {
		if (ent->new == profile)
			continue;
		if (strncmp(ent->new->base.hname, profile->base.hname, len) ==
		    0 && ent->new->base.hname[len] == 0)
			return ent->new;
	}

	return NULL;
}

/**
 * __replace_profile - replace @old with @new on a list
 * @old: profile to be replaced  (NOT NULL)
 * @new: profile to replace @old with  (NOT NULL)
 * @share_replacedby: transfer @old->replacedby to @new
 *
 * Will duplicate and refcount elements that @new inherits from @old
 * and will inherit @old children.
 *
 * refcount @new for list, put @old list refcount
 *
 * Requires: namespace list lock be held, or list not be shared
 */
static void __replace_profile(struct aa_profile *old, struct aa_profile *new,
			      bool share_replacedby)
{
	struct aa_profile *child, *tmp;

	if (!list_empty(&old->base.profiles)) {
		LIST_HEAD(lh);
		list_splice_init_rcu(&old->base.profiles, &lh, synchronize_rcu);

		list_for_each_entry_safe(child, tmp, &lh, base.list) {
			struct aa_profile *p;

			list_del_init(&child->base.list);
			p = __find_child(&new->base.profiles, child->base.name);
			if (p) {
				/* @p replaces @child  */
				__replace_profile(child, p, share_replacedby);
				continue;
			}

			/* inherit @child and its children */
			/* TODO: update hname of inherited children */
			/* list refcount transferred to @new */
			p = aa_deref_parent(child);
			rcu_assign_pointer(child->parent, aa_get_profile(new));
			list_add_rcu(&child->base.list, &new->base.profiles);
			aa_put_profile(p);
		}
	}

	if (!rcu_access_pointer(new->parent)) {
		struct aa_profile *parent = aa_deref_parent(old);
		rcu_assign_pointer(new->parent, aa_get_profile(parent));
	}
	__aa_update_replacedby(&old->label, &new->label);
	if (share_replacedby)
		new->label.replacedby = aa_get_replacedby(old->label.replacedby);
	else if (!rcu_access_pointer(new->label.replacedby->label))
		/* aafs interface uses replacedby */
		rcu_assign_pointer(new->label.replacedby->label,
				   aa_get_label(&new->label));
	__aa_fs_profile_migrate_dents(old, new);

	if (list_empty(&new->base.list)) {
		/* new is not on a list already */
		list_replace_rcu(&old->base.list, &new->base.list);
		aa_get_profile(new);
		aa_put_profile(old);
	} else
		__list_remove_profile(old);
}

/**
 * __lookup_replace - lookup replacement information for a profile
 * @ns - namespace the lookup occurs in
 * @hname - name of profile to lookup
 * @noreplace - true if not replacing an existing profile
 * @p - Returns: profile to be replaced
 * @info - Returns: info string on why lookup failed
 *
 * Returns: profile to replace (no ref) on success else ptr error
 */
static int __lookup_replace(struct aa_namespace *ns, const char *hname,
			    bool noreplace, struct aa_profile **p,
			    const char **info)
{
	*p = aa_get_profile(__lookup_profile(&ns->base, hname));
	if (*p) {
		int error = replacement_allowed(*p, noreplace, info);
		if (error) {
			*info = "profile can not be replaced";
			return error;
		}
	}

	return 0;
}

static void share_name(struct aa_profile *old, struct aa_profile *new)
{
	aa_put_str(new->base.hname);
	aa_get_str(old->base.hname);
	new->base.hname = old->base.hname;
	new->base.name = old->base.name;
	new->label.hname = old->label.hname;
}

/* Update to newest version of parent after previous replacements
 * Returns: unrefcount newest version of parent
 */
static struct aa_profile *update_to_newest_parent(struct aa_profile *new)
{
	struct aa_profile *parent, *newest;
	parent = rcu_dereference_protected(new->parent,
					   mutex_is_locked(&new->ns->lock));
	newest = aa_get_newest_profile(parent);

	/* parent replaced in this atomic set? */
	if (newest != parent) {
		aa_put_profile(parent);
		rcu_assign_pointer(new->parent, newest);
	} else
		aa_put_profile(newest);

	return newest;
}

/**
 * aa_replace_profiles - replace profile(s) on the profile list
 * @udata: serialized data stream  (NOT NULL)
 * @size: size of the serialized data stream
 * @noreplace: true if only doing addition, no replacement allowed
 *
 * unpack and replace a profile on the profile list and uses of that profile
 * by any aa_task_cxt.  If the profile does not exist on the profile list
 * it is added.
 *
 * Returns: size of data consumed else error code on failure.
 */
ssize_t aa_replace_profiles(void *udata, size_t size, bool noreplace)
{
	const char *ns_name, *name = NULL, *info = NULL;
	struct aa_namespace *ns = NULL;
	struct aa_load_ent *ent, *tmp;
	int op = OP_PROF_REPL;
	ssize_t error;
	LIST_HEAD(lh);

	/* released below */
	error = aa_unpack(udata, size, &lh, &ns_name);
	if (error)
		goto out;

	/* released below */
	ns = aa_prepare_namespace(ns_name);
	if (!ns) {
		info = "failed to prepare namespace";
		error = -ENOMEM;
		name = ns_name;
		goto fail;
	}

	mutex_lock(&ns->lock);
	/* setup parent and ns info */
	list_for_each_entry(ent, &lh, list) {
		struct aa_policy *policy;

		name = ent->new->base.hname;
		error = __lookup_replace(ns, ent->new->base.hname, noreplace,
					 &ent->old, &info);
		if (error)
			goto fail_lock;

		if (ent->new->rename) {
			error = __lookup_replace(ns, ent->new->rename,
						 noreplace, &ent->rename,
						 &info);
			if (error)
				goto fail_lock;
		}

		/* released when @new is freed */
		ent->new->ns = aa_get_namespace(ns);

		if (ent->old || ent->rename)
			continue;

		/* no ref on policy only use inside lock */
		policy = __lookup_parent(ns, ent->new->base.hname);
		if (!policy) {
			struct aa_profile *p;
			p = __list_lookup_parent(&lh, ent->new);
			if (!p) {
				error = -ENOENT;
				info = "parent does not exist";
				name = ent->new->base.hname;
				goto fail_lock;
			}
			rcu_assign_pointer(ent->new->parent, aa_get_profile(p));
		} else if (policy != &ns->base) {
			/* released on profile replacement or free_profile */
			struct aa_profile *p = (struct aa_profile *) policy;
			rcu_assign_pointer(ent->new->parent, aa_get_profile(p));
		}
	}

	/* create new fs entries for introspection if needed */
	list_for_each_entry(ent, &lh, list) {
		struct aa_replacedby *r;
		if (ent->old) {
			/* inherit old interface files */

			/* if (ent->rename)
				TODO: support rename */
		/* } else if (ent->rename) {
			TODO: support rename */
		} else {
			struct dentry *parent;
			r = aa_alloc_replacedby(NULL);
			if (!r) {
				info = "failed to create";
				error = -ENOMEM;
				goto fail_lock;
			}
			ent->new->label.replacedby = r;

			if (rcu_access_pointer(ent->new->parent)) {
				struct aa_profile *p;
				p = aa_deref_parent(ent->new);
				parent = prof_child_dir(p);
			} else
				parent = ns_subprofs_dir(ent->new->ns);
			error = __aa_fs_profile_mkdir(ent->new, parent);
		}

		if (error) {
			info = "failed to create";
			goto fail_lock;
		}
	}

	/* Done with checks that may fail - do actual replacement */
	list_for_each_entry_safe(ent, tmp, &lh, list) {
		list_del_init(&ent->list);
		op = (!ent->old && !ent->rename) ? OP_PROF_LOAD : OP_PROF_REPL;

		audit_policy(op, ent->new->base.name, NULL, error);

		if (ent->old) {
			share_name(ent->old, ent->new);
			__replace_profile(ent->old, ent->new, 1);
			aa_label_replace(&ns->labels, &ent->old->label,
					 &ent->new->label);
			if (ent->rename) {
				/* aafs interface uses replacedby */
				rcu_assign_pointer(ent->new->label.replacedby->label,
						   aa_get_label(&ent->new->label));
				__replace_profile(ent->rename, ent->new, 0);
			}
		} else if (ent->rename) {
			/* aafs interface uses replacedby */
			rcu_assign_pointer(ent->new->label.replacedby->label,
					   aa_get_label(&ent->new->label));
		} else {
			struct list_head *lh;
			if (rcu_access_pointer(ent->new->parent)) {
                               struct aa_profile *parent;
                               parent = update_to_newest_parent(ent->new);
                               lh = &parent->base.profiles;
			} else
				lh = &ns->base.profiles;

			/* aafs interface uses replacedby */
			rcu_assign_pointer(ent->new->label.replacedby->label,
					   aa_get_label(&ent->new->label));
			__add_profile(lh, ent->new);
		}
		aa_load_ent_free(ent);
	}
	__aa_labelset_update_all(ns);
	mutex_unlock(&ns->lock);

out:
	aa_put_namespace(ns);

	if (error)
		return error;
	return size;

fail_lock:
	mutex_unlock(&ns->lock);
fail:
	error = audit_policy(op, name, info, error);

	list_for_each_entry_safe(ent, tmp, &lh, list) {
		list_del_init(&ent->list);
		aa_load_ent_free(ent);
	}

	goto out;
}

/**
 * aa_remove_profiles - remove profile(s) from the system
 * @fqname: name of the profile or namespace to remove  (NOT NULL)
 * @size: size of the name
 *
 * Remove a profile or sub namespace from the current namespace, so that
 * they can not be found anymore and mark them as replaced by unconfined
 *
 * NOTE: removing confinement does not restore rlimits to preconfinemnet values
 *
 * Returns: size of data consume else error code if fails
 */
ssize_t aa_remove_profiles(char *fqname, size_t size)
{
	struct aa_namespace *root, *ns = NULL;
	struct aa_profile *profile = NULL;
	const char *name = fqname, *info = NULL;
	ssize_t error = 0;

	if (*fqname == 0) {
		info = "no profile specified";
		error = -ENOENT;
		goto fail;
	}

	root = labels_ns(aa_current_label());

	if (fqname[0] == ':') {
		char *ns_name;
		name = aa_split_fqname(fqname, &ns_name);
		/* released below */
		ns = aa_find_namespace(root, ns_name);
		if (!ns) {
			info = "namespace does not exist";
			error = -ENOENT;
			goto fail;
		}
	} else
		/* released below */
		ns = aa_get_namespace(root);

	if (!name) {
		/* remove namespace - can only happen if fqname[0] == ':' */
		mutex_lock(&ns->parent->lock);
		__remove_namespace(ns);
		mutex_unlock(&ns->parent->lock);
	} else {
		/* remove profile */
		mutex_lock(&ns->lock);
		profile = aa_get_profile(__lookup_profile(&ns->base, name));
		if (!profile) {
			error = -ENOENT;
			info = "profile does not exist";
			goto fail_ns_lock;
		}
		name = profile->base.hname;
		__remove_profile(profile);
		__aa_labelset_update_all(ns);
		mutex_unlock(&ns->lock);
	}

	/* don't fail removal if audit fails */
	(void) audit_policy(OP_PROF_RM, name, info, error);
	aa_put_namespace(ns);
	aa_put_profile(profile);
	return size;

fail_ns_lock:
	mutex_unlock(&ns->lock);
	aa_put_namespace(ns);

fail:
	(void) audit_policy(OP_PROF_RM, name, info, error);
	return error;
}
