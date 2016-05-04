/*
 * AppArmor security module
 *
 * This file contains AppArmor /proc/<pid>/attr/ interface functions
 *
 * Copyright (C) 1998-2008 Novell/SUSE
 * Copyright 2009-2010 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2 of the
 * License.
 */

#include "include/apparmor.h"
#include "include/context.h"
#include "include/policy.h"
#include "include/domain.h"
#include "include/procattr.h"


/**
 * aa_getprocattr - Return the profile information for @profile
 * @profile: the profile to print profile info about  (NOT NULL)
 * @string: Returns - string containing the profile info (NOT NULL)
 *
 * Returns: length of @string on success else error on failure
 *
 * Requires: profile != NULL
 *
 * Creates a string containing the namespace_name://profile_name for
 * @profile.
 *
 * Returns: size of string placed in @string else error code on failure
 */
int aa_getprocattr(struct aa_label *label, char **string)
{
	struct aa_namespace *ns = labels_ns(label);
	struct aa_namespace *current_ns = labels_ns(aa_current_label());
	int len;

	if (!aa_ns_visible(current_ns, ns))
		return -EACCES;

	len = aa_label_snprint(NULL, 0, current_ns, label, true);
	AA_BUG(len < 0);

	*string = kmalloc(len + 2, GFP_KERNEL);
	if (!*string)
		return -ENOMEM;

	len = aa_label_snprint(*string, len + 2, current_ns, label, true);
	if (len < 0)
		return len;
	(*string)[len] = '\n';
	(*string)[len + 1] = 0;

	return len + 1;
}

/**
 * split_token_from_name - separate a string of form  <token>^<name>
 * @op: operation being checked
 * @args: string to parse  (NOT NULL)
 * @token: stores returned parsed token value  (NOT NULL)
 *
 * Returns: start position of name after token else NULL on failure
 */
static char *split_token_from_name(int op, char *args, u64 * token)
{
	char *name;

	*token = simple_strtoull(args, &name, 16);
	if ((name == args) || *name != '^') {
		AA_ERROR("%s: Invalid input '%s'", op_table[op], args);
		return ERR_PTR(-EINVAL);
	}

	name++;			/* skip ^ */
	if (!*name)
		name = NULL;
	return name;
}

/**
 * aa_setprocattr_chagnehat - handle procattr interface to change_hat
 * @args: args received from writing to /proc/<pid>/attr/current (NOT NULL)
 * @size: size of the args
 * @test: true if this is a test of change_hat permissions
 *
 * Returns: %0 or error code if change_hat fails
 */
int aa_setprocattr_changehat(char *args, size_t size, int test)
{
	char *hat;
	u64 token;
	const char *hats[16];		/* current hard limit on # of names */
	int count = 0;

	hat = split_token_from_name(OP_CHANGE_HAT, args, &token);
	if (IS_ERR(hat))
		return PTR_ERR(hat);

	if (!hat && !token) {
		AA_ERROR("change_hat: Invalid input, NULL hat and NULL magic");
		return -EINVAL;
	}

	if (hat) {
		/* set up hat name vector, args guaranteed null terminated
		 * at args[size] by setprocattr.
		 *
		 * If there are multiple hat names in the buffer each is
		 * separated by a \0.  Ie. userspace writes them pre tokenized
		 */
		char *end = args + size;
		for (count = 0; (hat < end) && count < 16; ++count) {
			char *next = hat + strlen(hat) + 1;
			hats[count] = hat;
			AA_DEBUG("%s: (pid %d) Magic 0x%llx count %d hat '%s'\n"
				 , __func__, current->pid, token, count, hat);
			hat = next;
		}
	} else
		AA_DEBUG("%s: (pid %d) Magic 0x%llx count %d Hat '%s'\n",
			 __func__, current->pid, token, count, "<NULL>");

	return aa_change_hat(hats, count, token, test);
}

/**
 * aa_setprocattr_changeprofile - handle procattr interface to changeprofile
 * @fqname: args received from writting to /proc/<pid>/attr/current (NOT NULL)
 * @onexec: true if change_profile should be delayed until exec
 * @test: true if this is a test of change_profile permissions
 *
 * Returns: %0 or error code if change_profile fails
 */
int aa_setprocattr_changeprofile(char *fqname, bool onexec, int test)
{
	char *name, *ns_name;

	name = aa_split_fqname(fqname, &ns_name);
	return aa_change_profile(ns_name, name, onexec, test);
}
