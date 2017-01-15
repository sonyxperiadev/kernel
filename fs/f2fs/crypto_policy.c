/*
 * copied from linux/fs/ext4/crypto_policy.c
 *
 * Copyright (C) 2015, Google, Inc.
 * Copyright (C) 2015, Motorola Mobility.
 *
 * This contains encryption policy functions for f2fs with some modifications
 * to support f2fs-specific xattr APIs.
 *
 * Written by Michael Halcrow, 2015.
 * Modified by Jaegeuk Kim, 2015.
 */
#include <linux/random.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/f2fs_fs.h>

#include "f2fs.h"
#include "xattr.h"

static int f2fs_inode_has_encryption_context(struct inode *inode)
{
	int res = f2fs_getxattr(inode, F2FS_XATTR_INDEX_ENCRYPTION,
			F2FS_XATTR_NAME_ENCRYPTION_CONTEXT, NULL, 0, NULL);
	return (res > 0);
}

/*
 * check whether the policy is consistent with the encryption context
 * for the inode
 */
static int f2fs_is_encryption_context_consistent_with_policy(
	struct inode *inode, const struct f2fs_encryption_policy *policy)
{
	struct f2fs_encryption_context ctx;
	int res = f2fs_getxattr(inode, F2FS_XATTR_INDEX_ENCRYPTION,
				F2FS_XATTR_NAME_ENCRYPTION_CONTEXT, &ctx,
				sizeof(ctx), NULL);

	if (res != sizeof(ctx))
		return 0;

	return (memcmp(ctx.master_key_descriptor, policy->master_key_descriptor,
				F2FS_KEY_DESCRIPTOR_SIZE) == 0 &&
			(ctx.flags == policy->flags) &&
			(ctx.contents_encryption_mode ==
			 policy->contents_encryption_mode) &&
			(ctx.filenames_encryption_mode ==
			 policy->filenames_encryption_mode));
}

static int f2fs_create_encryption_context_from_policy(
	struct inode *inode, const struct f2fs_encryption_policy *policy)
{
	struct f2fs_encryption_context ctx;

	ctx.format = F2FS_ENCRYPTION_CONTEXT_FORMAT_V1;
	memcpy(ctx.master_key_descriptor, policy->master_key_descriptor,
			F2FS_KEY_DESCRIPTOR_SIZE);

	if (!f2fs_valid_contents_enc_mode(policy->contents_encryption_mode)) {
		printk(KERN_WARNING
		       "%s: Invalid contents encryption mode %d\n", __func__,
			policy->contents_encryption_mode);
		return -EINVAL;
	}

	if (!f2fs_valid_filenames_enc_mode(policy->filenames_encryption_mode)) {
		printk(KERN_WARNING
		       "%s: Invalid filenames encryption mode %d\n", __func__,
			policy->filenames_encryption_mode);
		return -EINVAL;
	}

	if (policy->flags & ~F2FS_POLICY_FLAGS_VALID)
		return -EINVAL;

	ctx.contents_encryption_mode = policy->contents_encryption_mode;
	ctx.filenames_encryption_mode = policy->filenames_encryption_mode;
	ctx.flags = policy->flags;
	BUILD_BUG_ON(sizeof(ctx.nonce) != F2FS_KEY_DERIVATION_NONCE_SIZE);
	get_random_bytes(ctx.nonce, F2FS_KEY_DERIVATION_NONCE_SIZE);

	return f2fs_setxattr(inode, F2FS_XATTR_INDEX_ENCRYPTION,
			F2FS_XATTR_NAME_ENCRYPTION_CONTEXT, &ctx,
			sizeof(ctx), NULL, XATTR_CREATE);
}

int f2fs_process_policy(const struct f2fs_encryption_policy *policy,
			struct inode *inode)
{
	if (policy->version != 0)
		return -EINVAL;

	if (!S_ISDIR(inode->i_mode))
		return -EINVAL;

	if (!f2fs_inode_has_encryption_context(inode)) {
		if (!f2fs_empty_dir(inode))
			return -ENOTEMPTY;
		return f2fs_create_encryption_context_from_policy(inode,
								  policy);
	}

	if (f2fs_is_encryption_context_consistent_with_policy(inode, policy))
		return 0;

	printk(KERN_WARNING "%s: Policy inconsistent with encryption context\n",
	       __func__);
	return -EINVAL;
}

int f2fs_get_policy(struct inode *inode, struct f2fs_encryption_policy *policy)
{
	struct f2fs_encryption_context ctx;
	int res;

	if (!f2fs_encrypted_inode(inode))
		return -ENODATA;

	res = f2fs_getxattr(inode, F2FS_XATTR_INDEX_ENCRYPTION,
				F2FS_XATTR_NAME_ENCRYPTION_CONTEXT,
				&ctx, sizeof(ctx), NULL);
	if (res != sizeof(ctx))
		return -ENODATA;
	if (ctx.format != F2FS_ENCRYPTION_CONTEXT_FORMAT_V1)
		return -EINVAL;

	policy->version = 0;
	policy->contents_encryption_mode = ctx.contents_encryption_mode;
	policy->filenames_encryption_mode = ctx.filenames_encryption_mode;
	policy->flags = ctx.flags;
	memcpy(&policy->master_key_descriptor, ctx.master_key_descriptor,
			F2FS_KEY_DESCRIPTOR_SIZE);
	return 0;
}

int f2fs_is_child_context_consistent_with_parent(struct inode *parent,
						struct inode *child)
{
	struct f2fs_crypt_info *parent_ci, *child_ci;
	int res;

	if ((parent == NULL) || (child == NULL)) {
		pr_err("parent %p child %p\n", parent, child);
		BUG_ON(1);
	}

	/* no restrictions if the parent directory is not encrypted */
	if (!f2fs_encrypted_inode(parent))
		return 1;
	/* if the child directory is not encrypted, this is always a problem */
	if (!f2fs_encrypted_inode(child))
		return 0;
	res = f2fs_get_encryption_info(parent);
	if (res)
		return 0;
	res = f2fs_get_encryption_info(child);
	if (res)
		return 0;
	parent_ci = F2FS_I(parent)->i_crypt_info;
	child_ci = F2FS_I(child)->i_crypt_info;
	if (!parent_ci && !child_ci)
		return 1;
	if (!parent_ci || !child_ci)
		return 0;

	return (memcmp(parent_ci->ci_master_key,
			child_ci->ci_master_key,
			F2FS_KEY_DESCRIPTOR_SIZE) == 0 &&
		(parent_ci->ci_data_mode == child_ci->ci_data_mode) &&
		(parent_ci->ci_filename_mode == child_ci->ci_filename_mode) &&
		(parent_ci->ci_flags == child_ci->ci_flags));
}

/**
 * f2fs_inherit_context() - Sets a child context from its parent
 * @parent: Parent inode from which the context is inherited.
 * @child:  Child inode that inherits the context from @parent.
 *
 * Return: Zero on success, non-zero otherwise
 */
int f2fs_inherit_context(struct inode *parent, struct inode *child,
						struct page *ipage)
{
	struct f2fs_encryption_context ctx;
	struct f2fs_crypt_info *ci;
	int res;

	res = f2fs_get_encryption_info(parent);
	if (res < 0)
		return res;

	ci = F2FS_I(parent)->i_crypt_info;
	if (ci == NULL)
		return -ENOKEY;

	ctx.format = F2FS_ENCRYPTION_CONTEXT_FORMAT_V1;

	ctx.contents_encryption_mode = ci->ci_data_mode;
	ctx.filenames_encryption_mode = ci->ci_filename_mode;
	ctx.flags = ci->ci_flags;
	memcpy(ctx.master_key_descriptor, ci->ci_master_key,
			F2FS_KEY_DESCRIPTOR_SIZE);

	get_random_bytes(ctx.nonce, F2FS_KEY_DERIVATION_NONCE_SIZE);
	return f2fs_setxattr(child, F2FS_XATTR_INDEX_ENCRYPTION,
				F2FS_XATTR_NAME_ENCRYPTION_CONTEXT, &ctx,
				sizeof(ctx), ipage, XATTR_CREATE);
}
