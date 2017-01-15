/*
 * fs/f2fs/namei.c
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/f2fs_fs.h>
#include <linux/pagemap.h>
#include <linux/sched.h>
#include <linux/ctype.h>
#include <linux/dcache.h>
#include <linux/namei.h>

#include "f2fs.h"
#include "node.h"
#include "xattr.h"
#include "acl.h"
#include <trace/events/f2fs.h>

static struct inode *f2fs_new_inode(struct inode *dir, umode_t mode)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(dir);
	nid_t ino;
	struct inode *inode;
	bool nid_free = false;
	int err;

	inode = new_inode(dir->i_sb);
	if (!inode)
		return ERR_PTR(-ENOMEM);

	f2fs_lock_op(sbi);
	if (!alloc_nid(sbi, &ino)) {
		f2fs_unlock_op(sbi);
		err = -ENOSPC;
		goto fail;
	}
	f2fs_unlock_op(sbi);

	inode_init_owner(inode, dir, mode);

	inode->i_ino = ino;
	inode->i_blocks = 0;
	inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;
	inode->i_generation = sbi->s_next_generation++;

	err = insert_inode_locked(inode);
	if (err) {
		err = -EINVAL;
		nid_free = true;
		goto fail;
	}

	/* If the directory encrypted, then we should encrypt the inode. */
	if (f2fs_encrypted_inode(dir) && f2fs_may_encrypt(inode))
		f2fs_set_encrypted_inode(inode);

	if (test_opt(sbi, INLINE_DATA) && f2fs_may_inline_data(inode))
		set_inode_flag(F2FS_I(inode), FI_INLINE_DATA);
	if (f2fs_may_inline_dentry(inode))
		set_inode_flag(F2FS_I(inode), FI_INLINE_DENTRY);

	f2fs_init_extent_tree(inode, NULL);

	stat_inc_inline_xattr(inode);
	stat_inc_inline_inode(inode);
	stat_inc_inline_dir(inode);

	trace_f2fs_new_inode(inode, 0);
	mark_inode_dirty(inode);
	return inode;

fail:
	trace_f2fs_new_inode(inode, err);
	make_bad_inode(inode);
	if (nid_free)
		set_inode_flag(F2FS_I(inode), FI_FREE_NID);
	iput(inode);
	return ERR_PTR(err);
}

static int is_multimedia_file(const unsigned char *s, const char *sub)
{
	size_t slen = strlen(s);
	size_t sublen = strlen(sub);

	/*
	 * filename format of multimedia file should be defined as:
	 * "filename + '.' + extension".
	 */
	if (slen < sublen + 2)
		return 0;

	if (s[slen - sublen - 1] != '.')
		return 0;

	return !strncasecmp(s + slen - sublen, sub, sublen);
}

/*
 * Set multimedia files as cold files for hot/cold data separation
 */
static inline void set_cold_files(struct f2fs_sb_info *sbi, struct inode *inode,
		const unsigned char *name)
{
	int i;
	__u8 (*extlist)[8] = sbi->raw_super->extension_list;

	int count = le32_to_cpu(sbi->raw_super->extension_count);
	for (i = 0; i < count; i++) {
		if (is_multimedia_file(name, extlist[i])) {
			file_set_cold(inode);
			break;
		}
	}
}

static int f2fs_create(struct inode *dir, struct dentry *dentry, umode_t mode,
						bool excl)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(dir);
	struct inode *inode;
	nid_t ino = 0;
	int err;

	inode = f2fs_new_inode(dir, mode);
	if (IS_ERR(inode))
		return PTR_ERR(inode);

	if (!test_opt(sbi, DISABLE_EXT_IDENTIFY))
		set_cold_files(sbi, inode, dentry->d_name.name);

	inode->i_op = &f2fs_file_inode_operations;
	inode->i_fop = &f2fs_file_operations;
	inode->i_mapping->a_ops = &f2fs_dblock_aops;
	ino = inode->i_ino;

	f2fs_balance_fs(sbi, true);

	f2fs_lock_op(sbi);
	err = f2fs_add_link(dentry, inode);
	if (err)
		goto out;
	f2fs_unlock_op(sbi);

	alloc_nid_done(sbi, ino);

	d_instantiate(dentry, inode);
	unlock_new_inode(inode);

	if (IS_DIRSYNC(dir))
		f2fs_sync_fs(sbi->sb, 1);
	return 0;
out:
	handle_failed_inode(inode);
	return err;
}

static int f2fs_link(struct dentry *old_dentry, struct inode *dir,
		struct dentry *dentry)
{
	struct inode *inode = old_dentry->d_inode;
	struct f2fs_sb_info *sbi = F2FS_I_SB(dir);
	int err;

	if (f2fs_encrypted_inode(dir) &&
		!f2fs_is_child_context_consistent_with_parent(dir, inode))
		return -EPERM;

	f2fs_balance_fs(sbi, true);

	inode->i_ctime = CURRENT_TIME;
	ihold(inode);

	set_inode_flag(F2FS_I(inode), FI_INC_LINK);
	f2fs_lock_op(sbi);
	err = f2fs_add_link(dentry, inode);
	if (err)
		goto out;
	f2fs_unlock_op(sbi);

	d_instantiate(dentry, inode);

	if (IS_DIRSYNC(dir))
		f2fs_sync_fs(sbi->sb, 1);
	return 0;
out:
	clear_inode_flag(F2FS_I(inode), FI_INC_LINK);
	iput(inode);
	f2fs_unlock_op(sbi);
	return err;
}

struct dentry *f2fs_get_parent(struct dentry *child)
{
	struct qstr dotdot = QSTR_INIT("..", 2);
	unsigned long ino = f2fs_inode_by_name(child->d_inode, &dotdot);
	if (!ino)
		return ERR_PTR(-ENOENT);
	return d_obtain_alias(f2fs_iget(child->d_inode->i_sb, ino));
}

static int __recover_dot_dentries(struct inode *dir, nid_t pino)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(dir);
	struct qstr dot = {.len = 1, .name = "."};
	struct qstr dotdot = {.len = 2, .name = ".."};
	struct f2fs_dir_entry *de;
	struct page *page;
	int err = 0;

	if (f2fs_readonly(sbi->sb)) {
		f2fs_msg(sbi->sb, KERN_INFO,
			"skip recovering inline_dots inode (ino:%lu, pino:%u) "
			"in readonly mountpoint", dir->i_ino, pino);
		return 0;
	}

	f2fs_balance_fs(sbi, true);

	f2fs_lock_op(sbi);

	de = f2fs_find_entry(dir, &dot, &page);
	if (de) {
		f2fs_dentry_kunmap(dir, page);
		f2fs_put_page(page, 0);
	} else {
		err = __f2fs_add_link(dir, &dot, NULL, dir->i_ino, S_IFDIR);
		if (err)
			goto out;
	}

	de = f2fs_find_entry(dir, &dotdot, &page);
	if (de) {
		f2fs_dentry_kunmap(dir, page);
		f2fs_put_page(page, 0);
	} else {
		err = __f2fs_add_link(dir, &dotdot, NULL, pino, S_IFDIR);
	}
out:
	if (!err) {
		clear_inode_flag(F2FS_I(dir), FI_INLINE_DOTS);
		mark_inode_dirty(dir);
	}

	f2fs_unlock_op(sbi);
	return err;
}

static struct dentry *f2fs_lookup(struct inode *dir, struct dentry *dentry,
		unsigned int flags)
{
	struct inode *inode = NULL;
	struct f2fs_dir_entry *de;
	struct page *page;
	nid_t ino;
	int err = 0;

	if (dentry->d_name.len > F2FS_NAME_LEN)
		return ERR_PTR(-ENAMETOOLONG);

	de = f2fs_find_entry(dir, &dentry->d_name, &page);
	if (!de)
		return d_splice_alias(inode, dentry);

	ino = le32_to_cpu(de->ino);
	f2fs_dentry_kunmap(dir, page);
	f2fs_put_page(page, 0);

	inode = f2fs_iget(dir->i_sb, ino);
	if (IS_ERR(inode))
		return ERR_CAST(inode);

	if (f2fs_has_inline_dots(inode)) {
		err = __recover_dot_dentries(inode, dir->i_ino);
		if (err)
			goto err_out;
	}
	return d_splice_alias(inode, dentry);

err_out:
	iget_failed(inode);
	return ERR_PTR(err);
}

static int f2fs_unlink(struct inode *dir, struct dentry *dentry)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(dir);
	struct inode *inode = dentry->d_inode;
	struct f2fs_dir_entry *de;
	struct page *page;
	int err = -ENOENT;

	trace_f2fs_unlink_enter(dir, dentry);

	de = f2fs_find_entry(dir, &dentry->d_name, &page);
	if (!de)
		goto fail;

	f2fs_balance_fs(sbi, true);

	f2fs_lock_op(sbi);
	err = acquire_orphan_inode(sbi);
	if (err) {
		f2fs_unlock_op(sbi);
		f2fs_dentry_kunmap(dir, page);
		f2fs_put_page(page, 0);
		goto fail;
	}
	f2fs_delete_entry(de, page, dir, inode);
	f2fs_unlock_op(sbi);

	/* In order to evict this inode, we set it dirty */
	mark_inode_dirty(inode);

	if (IS_DIRSYNC(dir))
		f2fs_sync_fs(sbi->sb, 1);
fail:
	trace_f2fs_unlink_exit(inode, err);
	return err;
}

static void *f2fs_follow_link(struct dentry *dentry, struct nameidata *nd)
{
	struct page *page;
	char *link;

	page = page_follow_link_light(dentry, nd);
	if (IS_ERR(page))
		return page;

	link = nd_get_link(nd);
	if (IS_ERR(link))
		return link;

	/* this is broken symlink case */
	if (*link == 0) {
		kunmap(page);
		page_cache_release(page);
		return ERR_PTR(-ENOENT);
	}
	return page;
}

static int f2fs_symlink(struct inode *dir, struct dentry *dentry,
					const char *symname)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(dir);
	struct inode *inode;
	size_t len = strlen(symname);
	struct f2fs_str disk_link = FSTR_INIT((char *)symname, len + 1);
	struct f2fs_encrypted_symlink_data *sd = NULL;
	int err;

	if (f2fs_encrypted_inode(dir)) {
		err = f2fs_get_encryption_info(dir);
		if (err)
			return err;

		if (!f2fs_encrypted_inode(dir))
			return -EPERM;

		disk_link.len = (f2fs_fname_encrypted_size(dir, len) +
				sizeof(struct f2fs_encrypted_symlink_data));
	}

	if (disk_link.len > dir->i_sb->s_blocksize)
		return -ENAMETOOLONG;

	inode = f2fs_new_inode(dir, S_IFLNK | S_IRWXUGO);
	if (IS_ERR(inode))
		return PTR_ERR(inode);

	if (f2fs_encrypted_inode(inode))
		inode->i_op = &f2fs_encrypted_symlink_inode_operations;
	else
		inode->i_op = &f2fs_symlink_inode_operations;
	inode->i_mapping->a_ops = &f2fs_dblock_aops;

	f2fs_balance_fs(sbi, true);

	f2fs_lock_op(sbi);
	err = f2fs_add_link(dentry, inode);
	if (err)
		goto out;
	f2fs_unlock_op(sbi);
	alloc_nid_done(sbi, inode->i_ino);

	if (f2fs_encrypted_inode(inode)) {
		struct qstr istr = QSTR_INIT(symname, len);
		struct f2fs_str ostr;

		sd = kzalloc(disk_link.len, GFP_NOFS);
		if (!sd) {
			err = -ENOMEM;
			goto err_out;
		}

		err = f2fs_get_encryption_info(inode);
		if (err)
			goto err_out;

		if (!f2fs_encrypted_inode(inode)) {
			err = -EPERM;
			goto err_out;
		}

		ostr.name = sd->encrypted_path;
		ostr.len = disk_link.len;
		err = f2fs_fname_usr_to_disk(inode, &istr, &ostr);
		if (err < 0)
			goto err_out;

		sd->len = cpu_to_le16(ostr.len);
		disk_link.name = (char *)sd;
	}

	err = page_symlink(inode, disk_link.name, disk_link.len);

err_out:
	d_instantiate(dentry, inode);
	unlock_new_inode(inode);

	/*
	 * Let's flush symlink data in order to avoid broken symlink as much as
	 * possible. Nevertheless, fsyncing is the best way, but there is no
	 * way to get a file descriptor in order to flush that.
	 *
	 * Note that, it needs to do dir->fsync to make this recoverable.
	 * If the symlink path is stored into inline_data, there is no
	 * performance regression.
	 */
	if (!err) {
		filemap_write_and_wait_range(inode->i_mapping, 0,
							disk_link.len - 1);

		if (IS_DIRSYNC(dir))
			f2fs_sync_fs(sbi->sb, 1);
	} else {
		f2fs_unlink(dir, dentry);
	}

	kfree(sd);
	return err;
out:
	handle_failed_inode(inode);
	return err;
}

static int f2fs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(dir);
	struct inode *inode;
	int err;

	inode = f2fs_new_inode(dir, S_IFDIR | mode);
	if (IS_ERR(inode))
		return PTR_ERR(inode);

	inode->i_op = &f2fs_dir_inode_operations;
	inode->i_fop = &f2fs_dir_operations;
	inode->i_mapping->a_ops = &f2fs_dblock_aops;
	mapping_set_gfp_mask(inode->i_mapping, GFP_F2FS_HIGH_ZERO);

	f2fs_balance_fs(sbi, true);

	set_inode_flag(F2FS_I(inode), FI_INC_LINK);
	f2fs_lock_op(sbi);
	err = f2fs_add_link(dentry, inode);
	if (err)
		goto out_fail;
	f2fs_unlock_op(sbi);

	alloc_nid_done(sbi, inode->i_ino);

	d_instantiate(dentry, inode);
	unlock_new_inode(inode);

	if (IS_DIRSYNC(dir))
		f2fs_sync_fs(sbi->sb, 1);
	return 0;

out_fail:
	clear_inode_flag(F2FS_I(inode), FI_INC_LINK);
	handle_failed_inode(inode);
	return err;
}

static int f2fs_rmdir(struct inode *dir, struct dentry *dentry)
{
	struct inode *inode = dentry->d_inode;
	if (f2fs_empty_dir(inode))
		return f2fs_unlink(dir, dentry);
	return -ENOTEMPTY;
}

static int f2fs_mknod(struct inode *dir, struct dentry *dentry,
				umode_t mode, dev_t rdev)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(dir);
	struct inode *inode;
	int err = 0;

	if (!new_valid_dev(rdev))
		return -EINVAL;

	inode = f2fs_new_inode(dir, mode);
	if (IS_ERR(inode))
		return PTR_ERR(inode);

	init_special_inode(inode, inode->i_mode, rdev);
	inode->i_op = &f2fs_special_inode_operations;

	f2fs_balance_fs(sbi, true);

	f2fs_lock_op(sbi);
	err = f2fs_add_link(dentry, inode);
	if (err)
		goto out;
	f2fs_unlock_op(sbi);

	alloc_nid_done(sbi, inode->i_ino);

	d_instantiate(dentry, inode);
	unlock_new_inode(inode);

	if (IS_DIRSYNC(dir))
		f2fs_sync_fs(sbi->sb, 1);
	return 0;
out:
	handle_failed_inode(inode);
	return err;
}

static int f2fs_rename(struct inode *old_dir, struct dentry *old_dentry,
			struct inode *new_dir, struct dentry *new_dentry)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(old_dir);
	struct inode *old_inode = old_dentry->d_inode;
	struct inode *new_inode = new_dentry->d_inode;
	struct page *old_dir_page;
	struct page *old_page, *new_page;
	struct f2fs_dir_entry *old_dir_entry = NULL;
	struct f2fs_dir_entry *old_entry;
	struct f2fs_dir_entry *new_entry;
	bool is_old_inline = f2fs_has_inline_dentry(old_dir);
	int err = -ENOENT;

	if ((old_dir != new_dir) && f2fs_encrypted_inode(new_dir) &&
		!f2fs_is_child_context_consistent_with_parent(new_dir,
							old_inode)) {
		err = -EPERM;
		goto out;
	}

	old_entry = f2fs_find_entry(old_dir, &old_dentry->d_name, &old_page);
	if (!old_entry)
		goto out;

	if (S_ISDIR(old_inode->i_mode)) {
		err = -EIO;
		old_dir_entry = f2fs_parent_dir(old_inode, &old_dir_page);
		if (!old_dir_entry)
			goto out_old;
	}

	if (new_inode) {

		err = -ENOTEMPTY;
		if (old_dir_entry && !f2fs_empty_dir(new_inode))
			goto out_dir;

		err = -ENOENT;
		new_entry = f2fs_find_entry(new_dir, &new_dentry->d_name,
						&new_page);
		if (!new_entry)
			goto out_dir;

		f2fs_balance_fs(sbi, true);

		f2fs_lock_op(sbi);

		err = acquire_orphan_inode(sbi);
		if (err)
			goto put_out_dir;

		err = update_dent_inode(old_inode, new_inode,
						&new_dentry->d_name);
		if (err) {
			release_orphan_inode(sbi);
			goto put_out_dir;
		}

		f2fs_set_link(new_dir, new_entry, new_page, old_inode);

		new_inode->i_ctime = CURRENT_TIME;
		down_write(&F2FS_I(new_inode)->i_sem);
		if (old_dir_entry)
			drop_nlink(new_inode);
		drop_nlink(new_inode);
		up_write(&F2FS_I(new_inode)->i_sem);

		mark_inode_dirty(new_inode);

		if (!new_inode->i_nlink)
			add_orphan_inode(sbi, new_inode->i_ino);
		else
			release_orphan_inode(sbi);

		update_inode_page(old_inode);
		update_inode_page(new_inode);
	} else {
		f2fs_balance_fs(sbi, true);

		f2fs_lock_op(sbi);

		err = f2fs_add_link(new_dentry, old_inode);
		if (err) {
			f2fs_unlock_op(sbi);
			goto out_dir;
		}

		if (old_dir_entry) {
			inc_nlink(new_dir);
			update_inode_page(new_dir);
		}

		/*
		 * old entry and new entry can locate in the same inline
		 * dentry in inode, when attaching new entry in inline dentry,
		 * it could force inline dentry conversion, after that,
		 * old_entry and old_page will point to wrong address, in
		 * order to avoid this, let's do the check and update here.
		 */
		if (is_old_inline && !f2fs_has_inline_dentry(old_dir)) {
			f2fs_put_page(old_page, 0);
			old_page = NULL;

			old_entry = f2fs_find_entry(old_dir,
						&old_dentry->d_name, &old_page);
			if (!old_entry) {
				err = -EIO;
				f2fs_unlock_op(sbi);
				goto out_dir;
			}
		}
	}

	down_write(&F2FS_I(old_inode)->i_sem);
	file_lost_pino(old_inode);
	if (new_inode && file_enc_name(new_inode))
		file_set_enc_name(old_inode);
	up_write(&F2FS_I(old_inode)->i_sem);

	old_inode->i_ctime = CURRENT_TIME;
	mark_inode_dirty(old_inode);

	f2fs_delete_entry(old_entry, old_page, old_dir, NULL);

	if (old_dir_entry) {
		if (old_dir != new_dir) {
			f2fs_set_link(old_inode, old_dir_entry,
						old_dir_page, new_dir);
			update_inode_page(old_inode);
		} else {
			f2fs_dentry_kunmap(old_inode, old_dir_page);
			f2fs_put_page(old_dir_page, 0);
		}
		drop_nlink(old_dir);
		mark_inode_dirty(old_dir);
		update_inode_page(old_dir);
	}

	f2fs_unlock_op(sbi);

	if (IS_DIRSYNC(old_dir) || IS_DIRSYNC(new_dir))
		f2fs_sync_fs(sbi->sb, 1);
	return 0;

put_out_dir:
	f2fs_unlock_op(sbi);
	f2fs_dentry_kunmap(new_dir, new_page);
	f2fs_put_page(new_page, 0);
out_dir:
	if (old_dir_entry) {
		f2fs_dentry_kunmap(old_inode, old_dir_page);
		f2fs_put_page(old_dir_page, 0);
	}
out_old:
	f2fs_dentry_kunmap(old_dir, old_page);
	f2fs_put_page(old_page, 0);
out:
	return err;
}

#ifdef CONFIG_F2FS_FS_ENCRYPTION
static void *f2fs_encrypted_follow_link(struct dentry *dentry,
						struct nameidata *nd)
{
	struct page *cpage = NULL;
	char *caddr, *paddr = NULL;
	struct f2fs_str cstr = FSTR_INIT(NULL, 0);
	struct f2fs_str pstr = FSTR_INIT(NULL, 0);
	struct inode *inode = dentry->d_inode;
	struct f2fs_encrypted_symlink_data *sd;
	loff_t size = min_t(loff_t, i_size_read(inode), PAGE_SIZE - 1);
	u32 max_size = inode->i_sb->s_blocksize;
	int res;

	res = f2fs_get_encryption_info(inode);
	if (res)
		return ERR_PTR(res);

	cpage = read_mapping_page(inode->i_mapping, 0, NULL);
	if (IS_ERR(cpage))
		return cpage;
	caddr = kmap(cpage);
	caddr[size] = 0;

	/* Symlink is encrypted */
	sd = (struct f2fs_encrypted_symlink_data *)caddr;
	cstr.name = sd->encrypted_path;
	cstr.len = le16_to_cpu(sd->len);

	/* this is broken symlink case */
	if (unlikely(cstr.len == 0)) {
		res = -ENOENT;
		goto errout;
	}

	/* this is broken symlink case */
	if (unlikely(cstr.name[0] == 0)) {
		res = -ENOENT;
		goto errout;
	}

	if ((cstr.len + sizeof(struct f2fs_encrypted_symlink_data) - 1) >
								max_size) {
		/* Symlink data on the disk is corrupted */
		res = -EIO;
		goto errout;
	}
	res = f2fs_fname_crypto_alloc_buffer(inode, cstr.len, &pstr);
	if (res)
		goto errout;

	res = f2fs_fname_disk_to_usr(inode, NULL, &cstr, &pstr);
	if (res < 0)
		goto errout;

	paddr = pstr.name;

	/* Null-terminate the name */
	paddr[res] = '\0';
	nd_set_link(nd, paddr);

	kunmap(cpage);
	page_cache_release(cpage);
	return NULL;
errout:
	f2fs_fname_crypto_free_buffer(&pstr);
	kunmap(cpage);
	page_cache_release(cpage);
	return ERR_PTR(res);
}

void kfree_put_link(struct dentry *dentry, struct nameidata *nd,
		void *cookie)
{
	char *s = nd_get_link(nd);
	if (!IS_ERR(s))
		kfree(s);
}

const struct inode_operations f2fs_encrypted_symlink_inode_operations = {
	.readlink       = generic_readlink,
	.follow_link    = f2fs_encrypted_follow_link,
	.put_link       = kfree_put_link,
	.getattr	= f2fs_getattr,
	.setattr	= f2fs_setattr,
#ifdef CONFIG_F2FS_FS_XATTR
	.setxattr	= generic_setxattr,
	.getxattr	= generic_getxattr,
	.listxattr	= f2fs_listxattr,
	.removexattr	= generic_removexattr,
#endif
};
#endif

const struct inode_operations f2fs_dir_inode_operations = {
	.create		= f2fs_create,
	.lookup		= f2fs_lookup,
	.link		= f2fs_link,
	.unlink		= f2fs_unlink,
	.symlink	= f2fs_symlink,
	.mkdir		= f2fs_mkdir,
	.rmdir		= f2fs_rmdir,
	.mknod		= f2fs_mknod,
	.rename		= f2fs_rename,
	.getattr	= f2fs_getattr,
	.setattr	= f2fs_setattr,
	.get_acl	= f2fs_get_acl,
#ifdef CONFIG_F2FS_FS_XATTR
	.setxattr	= generic_setxattr,
	.getxattr	= generic_getxattr,
	.listxattr	= f2fs_listxattr,
	.removexattr	= generic_removexattr,
#endif
};

const struct inode_operations f2fs_symlink_inode_operations = {
	.readlink       = generic_readlink,
	.follow_link    = f2fs_follow_link,
	.put_link       = page_put_link,
	.getattr	= f2fs_getattr,
	.setattr	= f2fs_setattr,
#ifdef CONFIG_F2FS_FS_XATTR
	.setxattr	= generic_setxattr,
	.getxattr	= generic_getxattr,
	.listxattr	= f2fs_listxattr,
	.removexattr	= generic_removexattr,
#endif
};

const struct inode_operations f2fs_special_inode_operations = {
	.getattr	= f2fs_getattr,
	.setattr        = f2fs_setattr,
	.get_acl	= f2fs_get_acl,
#ifdef CONFIG_F2FS_FS_XATTR
	.setxattr       = generic_setxattr,
	.getxattr       = generic_getxattr,
	.listxattr	= f2fs_listxattr,
	.removexattr    = generic_removexattr,
#endif
};
