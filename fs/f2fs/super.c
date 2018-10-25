/*
 * fs/f2fs/super.c
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/statfs.h>
#include <linux/buffer_head.h>
#include <linux/backing-dev.h>
#include <linux/kthread.h>
#include <linux/parser.h>
#include <linux/mount.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/random.h>
#include <linux/exportfs.h>
#include <linux/blkdev.h>
#include <linux/quotaops.h>
#include <linux/f2fs_fs.h>
#include <linux/sysfs.h>
#include <linux/quota.h>

#include "f2fs.h"
#include "node.h"
#include "segment.h"
#include "xattr.h"
#include "gc.h"
#include "trace.h"

#define CREATE_TRACE_POINTS
#include <trace/events/f2fs.h>

static struct kmem_cache *f2fs_inode_cachep;

#ifdef CONFIG_F2FS_FAULT_INJECTION

char *fault_name[FAULT_MAX] = {
	[FAULT_KMALLOC]		= "kmalloc",
	[FAULT_KVMALLOC]	= "kvmalloc",
	[FAULT_PAGE_ALLOC]	= "page alloc",
	[FAULT_PAGE_GET]	= "page get",
	[FAULT_ALLOC_BIO]	= "alloc bio",
	[FAULT_ALLOC_NID]	= "alloc nid",
	[FAULT_ORPHAN]		= "orphan",
	[FAULT_BLOCK]		= "no more block",
	[FAULT_DIR_DEPTH]	= "too big dir depth",
	[FAULT_EVICT_INODE]	= "evict_inode fail",
	[FAULT_TRUNCATE]	= "truncate fail",
	[FAULT_IO]		= "IO error",
	[FAULT_CHECKPOINT]	= "checkpoint error",
};

static void f2fs_build_fault_attr(struct f2fs_sb_info *sbi,
						unsigned int rate)
{
	struct f2fs_fault_info *ffi = &F2FS_OPTION(sbi).fault_info;

	if (rate) {
		atomic_set(&ffi->inject_ops, 0);
		ffi->inject_rate = rate;
		ffi->inject_type = (1 << FAULT_MAX) - 1;
	} else {
		memset(ffi, 0, sizeof(struct f2fs_fault_info));
	}
}
#endif

/* f2fs-wide shrinker description */
static struct shrinker f2fs_shrinker_info = {
	.scan_objects = f2fs_shrink_scan,
	.count_objects = f2fs_shrink_count,
	.seeks = DEFAULT_SEEKS,
};

enum {
	Opt_gc_background,
	Opt_disable_roll_forward,
	Opt_norecovery,
	Opt_discard,
	Opt_nodiscard,
	Opt_noheap,
	Opt_heap,
	Opt_user_xattr,
	Opt_nouser_xattr,
	Opt_acl,
	Opt_noacl,
	Opt_active_logs,
	Opt_disable_ext_identify,
	Opt_inline_xattr,
	Opt_noinline_xattr,
	Opt_inline_xattr_size,
	Opt_inline_data,
	Opt_inline_dentry,
	Opt_noinline_dentry,
	Opt_flush_merge,
	Opt_noflush_merge,
	Opt_nobarrier,
	Opt_fastboot,
	Opt_extent_cache,
	Opt_noextent_cache,
	Opt_noinline_data,
	Opt_data_flush,
	Opt_reserve_root,
	Opt_resgid,
	Opt_resuid,
	Opt_mode,
	Opt_io_size_bits,
	Opt_fault_injection,
	Opt_lazytime,
	Opt_nolazytime,
	Opt_quota,
	Opt_noquota,
	Opt_usrquota,
	Opt_grpquota,
	Opt_prjquota,
	Opt_usrjquota,
	Opt_grpjquota,
	Opt_prjjquota,
	Opt_offusrjquota,
	Opt_offgrpjquota,
	Opt_offprjjquota,
	Opt_jqfmt_vfsold,
	Opt_jqfmt_vfsv0,
	Opt_jqfmt_vfsv1,
	Opt_whint,
	Opt_alloc,
	Opt_fsync,
	Opt_test_dummy_encryption,
	Opt_err,
};

static match_table_t f2fs_tokens = {
	{Opt_gc_background, "background_gc=%s"},
	{Opt_disable_roll_forward, "disable_roll_forward"},
	{Opt_norecovery, "norecovery"},
	{Opt_discard, "discard"},
	{Opt_nodiscard, "nodiscard"},
	{Opt_noheap, "no_heap"},
	{Opt_heap, "heap"},
	{Opt_user_xattr, "user_xattr"},
	{Opt_nouser_xattr, "nouser_xattr"},
	{Opt_acl, "acl"},
	{Opt_noacl, "noacl"},
	{Opt_active_logs, "active_logs=%u"},
	{Opt_disable_ext_identify, "disable_ext_identify"},
	{Opt_inline_xattr, "inline_xattr"},
	{Opt_noinline_xattr, "noinline_xattr"},
	{Opt_inline_xattr_size, "inline_xattr_size=%u"},
	{Opt_inline_data, "inline_data"},
	{Opt_inline_dentry, "inline_dentry"},
	{Opt_noinline_dentry, "noinline_dentry"},
	{Opt_flush_merge, "flush_merge"},
	{Opt_noflush_merge, "noflush_merge"},
	{Opt_nobarrier, "nobarrier"},
	{Opt_fastboot, "fastboot"},
	{Opt_extent_cache, "extent_cache"},
	{Opt_noextent_cache, "noextent_cache"},
	{Opt_noinline_data, "noinline_data"},
	{Opt_data_flush, "data_flush"},
	{Opt_reserve_root, "reserve_root=%u"},
	{Opt_resgid, "resgid=%u"},
	{Opt_resuid, "resuid=%u"},
	{Opt_mode, "mode=%s"},
	{Opt_io_size_bits, "io_bits=%u"},
	{Opt_fault_injection, "fault_injection=%u"},
	{Opt_lazytime, "lazytime"},
	{Opt_nolazytime, "nolazytime"},
	{Opt_quota, "quota"},
	{Opt_noquota, "noquota"},
	{Opt_usrquota, "usrquota"},
	{Opt_grpquota, "grpquota"},
	{Opt_prjquota, "prjquota"},
	{Opt_usrjquota, "usrjquota=%s"},
	{Opt_grpjquota, "grpjquota=%s"},
	{Opt_prjjquota, "prjjquota=%s"},
	{Opt_offusrjquota, "usrjquota="},
	{Opt_offgrpjquota, "grpjquota="},
	{Opt_offprjjquota, "prjjquota="},
	{Opt_jqfmt_vfsold, "jqfmt=vfsold"},
	{Opt_jqfmt_vfsv0, "jqfmt=vfsv0"},
	{Opt_jqfmt_vfsv1, "jqfmt=vfsv1"},
	{Opt_whint, "whint_mode=%s"},
	{Opt_alloc, "alloc_mode=%s"},
	{Opt_fsync, "fsync_mode=%s"},
	{Opt_test_dummy_encryption, "test_dummy_encryption"},
	{Opt_err, NULL},
};

void f2fs_msg(struct super_block *sb, const char *level, const char *fmt, ...)
{
	struct va_format vaf;
	va_list args;

	va_start(args, fmt);
	vaf.fmt = fmt;
	vaf.va = &args;
	printk_ratelimited("%sF2FS-fs (%s): %pV\n", level, sb->s_id, &vaf);
	va_end(args);
}

static inline void limit_reserve_root(struct f2fs_sb_info *sbi)
{
	block_t limit = (sbi->user_block_count << 1) / 1000;

	/* limit is 0.2% */
	if (test_opt(sbi, RESERVE_ROOT) &&
			F2FS_OPTION(sbi).root_reserved_blocks > limit) {
		F2FS_OPTION(sbi).root_reserved_blocks = limit;
		f2fs_msg(sbi->sb, KERN_INFO,
			"Reduce reserved blocks for root = %u",
			F2FS_OPTION(sbi).root_reserved_blocks);
	}
	if (!test_opt(sbi, RESERVE_ROOT) &&
		(!uid_eq(F2FS_OPTION(sbi).s_resuid,
				make_kuid(&init_user_ns, F2FS_DEF_RESUID)) ||
		!gid_eq(F2FS_OPTION(sbi).s_resgid,
				make_kgid(&init_user_ns, F2FS_DEF_RESGID))))
		f2fs_msg(sbi->sb, KERN_INFO,
			"Ignore s_resuid=%u, s_resgid=%u w/o reserve_root",
				from_kuid_munged(&init_user_ns,
					F2FS_OPTION(sbi).s_resuid),
				from_kgid_munged(&init_user_ns,
					F2FS_OPTION(sbi).s_resgid));
}

static void init_once(void *foo)
{
	struct f2fs_inode_info *fi = (struct f2fs_inode_info *) foo;

	inode_init_once(&fi->vfs_inode);
}

#ifdef CONFIG_QUOTA
static const char * const quotatypes[] = INITQFNAMES;
#define QTYPE2NAME(t) (quotatypes[t])
static int f2fs_set_qf_name(struct super_block *sb, int qtype,
							substring_t *args)
{
	struct f2fs_sb_info *sbi = F2FS_SB(sb);
	char *qname;
	int ret = -EINVAL;

	if (sb_any_quota_loaded(sb) && !F2FS_OPTION(sbi).s_qf_names[qtype]) {
		f2fs_msg(sb, KERN_ERR,
			"Cannot change journaled "
			"quota options when quota turned on");
		return -EINVAL;
	}
	if (f2fs_sb_has_quota_ino(sb)) {
		f2fs_msg(sb, KERN_INFO,
			"QUOTA feature is enabled, so ignore qf_name");
		return 0;
	}

	qname = match_strdup(args);
	if (!qname) {
		f2fs_msg(sb, KERN_ERR,
			"Not enough memory for storing quotafile name");
		return -EINVAL;
	}
	if (F2FS_OPTION(sbi).s_qf_names[qtype]) {
		if (strcmp(F2FS_OPTION(sbi).s_qf_names[qtype], qname) == 0)
			ret = 0;
		else
			f2fs_msg(sb, KERN_ERR,
				 "%s quota file already specified",
				 QTYPE2NAME(qtype));
		goto errout;
	}
	if (strchr(qname, '/')) {
		f2fs_msg(sb, KERN_ERR,
			"quotafile must be on filesystem root");
		goto errout;
	}
	F2FS_OPTION(sbi).s_qf_names[qtype] = qname;
	set_opt(sbi, QUOTA);
	return 0;
errout:
	kfree(qname);
	return ret;
}

static int f2fs_clear_qf_name(struct super_block *sb, int qtype)
{
	struct f2fs_sb_info *sbi = F2FS_SB(sb);

	if (sb_any_quota_loaded(sb) && F2FS_OPTION(sbi).s_qf_names[qtype]) {
		f2fs_msg(sb, KERN_ERR, "Cannot change journaled quota options"
			" when quota turned on");
		return -EINVAL;
	}
	kfree(F2FS_OPTION(sbi).s_qf_names[qtype]);
	F2FS_OPTION(sbi).s_qf_names[qtype] = NULL;
	return 0;
}

static int f2fs_check_quota_options(struct f2fs_sb_info *sbi)
{
	/*
	 * We do the test below only for project quotas. 'usrquota' and
	 * 'grpquota' mount options are allowed even without quota feature
	 * to support legacy quotas in quota files.
	 */
	if (test_opt(sbi, PRJQUOTA) && !f2fs_sb_has_project_quota(sbi->sb)) {
		f2fs_msg(sbi->sb, KERN_ERR, "Project quota feature not enabled. "
			 "Cannot enable project quota enforcement.");
		return -1;
	}
	if (F2FS_OPTION(sbi).s_qf_names[USRQUOTA] ||
			F2FS_OPTION(sbi).s_qf_names[GRPQUOTA] ||
			F2FS_OPTION(sbi).s_qf_names[PRJQUOTA]) {
		if (test_opt(sbi, USRQUOTA) &&
				F2FS_OPTION(sbi).s_qf_names[USRQUOTA])
			clear_opt(sbi, USRQUOTA);

		if (test_opt(sbi, GRPQUOTA) &&
				F2FS_OPTION(sbi).s_qf_names[GRPQUOTA])
			clear_opt(sbi, GRPQUOTA);

		if (test_opt(sbi, PRJQUOTA) &&
				F2FS_OPTION(sbi).s_qf_names[PRJQUOTA])
			clear_opt(sbi, PRJQUOTA);

		if (test_opt(sbi, GRPQUOTA) || test_opt(sbi, USRQUOTA) ||
				test_opt(sbi, PRJQUOTA)) {
			f2fs_msg(sbi->sb, KERN_ERR, "old and new quota "
					"format mixing");
			return -1;
		}

		if (!F2FS_OPTION(sbi).s_jquota_fmt) {
			f2fs_msg(sbi->sb, KERN_ERR, "journaled quota format "
					"not specified");
			return -1;
		}
	}

	if (f2fs_sb_has_quota_ino(sbi->sb) && F2FS_OPTION(sbi).s_jquota_fmt) {
		f2fs_msg(sbi->sb, KERN_INFO,
			"QUOTA feature is enabled, so ignore jquota_fmt");
		F2FS_OPTION(sbi).s_jquota_fmt = 0;
	}
	if (f2fs_sb_has_quota_ino(sbi->sb) && f2fs_readonly(sbi->sb)) {
		f2fs_msg(sbi->sb, KERN_INFO,
			 "Filesystem with quota feature cannot be mounted RDWR "
			 "without CONFIG_QUOTA");
		return -1;
	}
	return 0;
}
#endif

static int parse_options(struct super_block *sb, char *options)
{
	struct f2fs_sb_info *sbi = F2FS_SB(sb);
	struct request_queue *q;
	substring_t args[MAX_OPT_ARGS];
	char *p, *name;
	int arg = 0;
	kuid_t uid;
	kgid_t gid;
#ifdef CONFIG_QUOTA
	int ret;
#endif

	if (!options)
		return 0;

	while ((p = strsep(&options, ",")) != NULL) {
		int token;
		if (!*p)
			continue;
		/*
		 * Initialize args struct so we know whether arg was
		 * found; some options take optional arguments.
		 */
		args[0].to = args[0].from = NULL;
		token = match_token(p, f2fs_tokens, args);

		switch (token) {
		case Opt_gc_background:
			name = match_strdup(&args[0]);

			if (!name)
				return -ENOMEM;
			if (strlen(name) == 2 && !strncmp(name, "on", 2)) {
				set_opt(sbi, BG_GC);
				clear_opt(sbi, FORCE_FG_GC);
			} else if (strlen(name) == 3 && !strncmp(name, "off", 3)) {
				clear_opt(sbi, BG_GC);
				clear_opt(sbi, FORCE_FG_GC);
			} else if (strlen(name) == 4 && !strncmp(name, "sync", 4)) {
				set_opt(sbi, BG_GC);
				set_opt(sbi, FORCE_FG_GC);
			} else {
				kfree(name);
				return -EINVAL;
			}
			kfree(name);
			break;
		case Opt_disable_roll_forward:
			set_opt(sbi, DISABLE_ROLL_FORWARD);
			break;
		case Opt_norecovery:
			/* this option mounts f2fs with ro */
			set_opt(sbi, DISABLE_ROLL_FORWARD);
			if (!f2fs_readonly(sb))
				return -EINVAL;
			break;
		case Opt_discard:
			q = bdev_get_queue(sb->s_bdev);
			if (blk_queue_discard(q)) {
				set_opt(sbi, DISCARD);
			} else if (!f2fs_sb_has_blkzoned(sb)) {
				f2fs_msg(sb, KERN_WARNING,
					"mounting with \"discard\" option, but "
					"the device does not support discard");
			}
			break;
		case Opt_nodiscard:
			if (f2fs_sb_has_blkzoned(sb)) {
				f2fs_msg(sb, KERN_WARNING,
					"discard is required for zoned block devices");
				return -EINVAL;
			}
			clear_opt(sbi, DISCARD);
			break;
		case Opt_noheap:
			set_opt(sbi, NOHEAP);
			break;
		case Opt_heap:
			clear_opt(sbi, NOHEAP);
			break;
#ifdef CONFIG_F2FS_FS_XATTR
		case Opt_user_xattr:
			set_opt(sbi, XATTR_USER);
			break;
		case Opt_nouser_xattr:
			clear_opt(sbi, XATTR_USER);
			break;
		case Opt_inline_xattr:
			set_opt(sbi, INLINE_XATTR);
			break;
		case Opt_noinline_xattr:
			clear_opt(sbi, INLINE_XATTR);
			break;
		case Opt_inline_xattr_size:
			if (args->from && match_int(args, &arg))
				return -EINVAL;
			set_opt(sbi, INLINE_XATTR_SIZE);
			F2FS_OPTION(sbi).inline_xattr_size = arg;
			break;
#else
		case Opt_user_xattr:
			f2fs_msg(sb, KERN_INFO,
				"user_xattr options not supported");
			break;
		case Opt_nouser_xattr:
			f2fs_msg(sb, KERN_INFO,
				"nouser_xattr options not supported");
			break;
		case Opt_inline_xattr:
			f2fs_msg(sb, KERN_INFO,
				"inline_xattr options not supported");
			break;
		case Opt_noinline_xattr:
			f2fs_msg(sb, KERN_INFO,
				"noinline_xattr options not supported");
			break;
#endif
#ifdef CONFIG_F2FS_FS_POSIX_ACL
		case Opt_acl:
			set_opt(sbi, POSIX_ACL);
			break;
		case Opt_noacl:
			clear_opt(sbi, POSIX_ACL);
			break;
#else
		case Opt_acl:
			f2fs_msg(sb, KERN_INFO, "acl options not supported");
			break;
		case Opt_noacl:
			f2fs_msg(sb, KERN_INFO, "noacl options not supported");
			break;
#endif
		case Opt_active_logs:
			if (args->from && match_int(args, &arg))
				return -EINVAL;
			if (arg != 2 && arg != 4 && arg != NR_CURSEG_TYPE)
				return -EINVAL;
			F2FS_OPTION(sbi).active_logs = arg;
			break;
		case Opt_disable_ext_identify:
			set_opt(sbi, DISABLE_EXT_IDENTIFY);
			break;
		case Opt_inline_data:
			set_opt(sbi, INLINE_DATA);
			break;
		case Opt_inline_dentry:
			set_opt(sbi, INLINE_DENTRY);
			break;
		case Opt_noinline_dentry:
			clear_opt(sbi, INLINE_DENTRY);
			break;
		case Opt_flush_merge:
			set_opt(sbi, FLUSH_MERGE);
			break;
		case Opt_noflush_merge:
			clear_opt(sbi, FLUSH_MERGE);
			break;
		case Opt_nobarrier:
			set_opt(sbi, NOBARRIER);
			break;
		case Opt_fastboot:
			set_opt(sbi, FASTBOOT);
			break;
		case Opt_extent_cache:
			set_opt(sbi, EXTENT_CACHE);
			break;
		case Opt_noextent_cache:
			clear_opt(sbi, EXTENT_CACHE);
			break;
		case Opt_noinline_data:
			clear_opt(sbi, INLINE_DATA);
			break;
		case Opt_data_flush:
			set_opt(sbi, DATA_FLUSH);
			break;
		case Opt_reserve_root:
			if (args->from && match_int(args, &arg))
				return -EINVAL;
			if (test_opt(sbi, RESERVE_ROOT)) {
				f2fs_msg(sb, KERN_INFO,
					"Preserve previous reserve_root=%u",
					F2FS_OPTION(sbi).root_reserved_blocks);
			} else {
				F2FS_OPTION(sbi).root_reserved_blocks = arg;
				set_opt(sbi, RESERVE_ROOT);
			}
			break;
		case Opt_resuid:
			if (args->from && match_int(args, &arg))
				return -EINVAL;
			uid = make_kuid(current_user_ns(), arg);
			if (!uid_valid(uid)) {
				f2fs_msg(sb, KERN_ERR,
					"Invalid uid value %d", arg);
				return -EINVAL;
			}
			F2FS_OPTION(sbi).s_resuid = uid;
			break;
		case Opt_resgid:
			if (args->from && match_int(args, &arg))
				return -EINVAL;
			gid = make_kgid(current_user_ns(), arg);
			if (!gid_valid(gid)) {
				f2fs_msg(sb, KERN_ERR,
					"Invalid gid value %d", arg);
				return -EINVAL;
			}
			F2FS_OPTION(sbi).s_resgid = gid;
			break;
		case Opt_mode:
			name = match_strdup(&args[0]);

			if (!name)
				return -ENOMEM;
			if (strlen(name) == 8 &&
					!strncmp(name, "adaptive", 8)) {
				if (f2fs_sb_has_blkzoned(sb)) {
					f2fs_msg(sb, KERN_WARNING,
						 "adaptive mode is not allowed with "
						 "zoned block device feature");
					kfree(name);
					return -EINVAL;
				}
				set_opt_mode(sbi, F2FS_MOUNT_ADAPTIVE);
			} else if (strlen(name) == 3 &&
					!strncmp(name, "lfs", 3)) {
				set_opt_mode(sbi, F2FS_MOUNT_LFS);
			} else {
				kfree(name);
				return -EINVAL;
			}
			kfree(name);
			break;
		case Opt_io_size_bits:
			if (args->from && match_int(args, &arg))
				return -EINVAL;
			if (arg > __ilog2_u32(BIO_MAX_PAGES)) {
				f2fs_msg(sb, KERN_WARNING,
					"Not support %d, larger than %d",
					1 << arg, BIO_MAX_PAGES);
				return -EINVAL;
			}
			F2FS_OPTION(sbi).write_io_size_bits = arg;
			break;
		case Opt_fault_injection:
			if (args->from && match_int(args, &arg))
				return -EINVAL;
#ifdef CONFIG_F2FS_FAULT_INJECTION
			f2fs_build_fault_attr(sbi, arg);
			set_opt(sbi, FAULT_INJECTION);
#else
			f2fs_msg(sb, KERN_INFO,
				"FAULT_INJECTION was not selected");
#endif
			break;
		case Opt_lazytime:
			sb->s_flags |= MS_LAZYTIME;
			break;
		case Opt_nolazytime:
			sb->s_flags &= ~MS_LAZYTIME;
			break;
#ifdef CONFIG_QUOTA
		case Opt_quota:
		case Opt_usrquota:
			set_opt(sbi, USRQUOTA);
			break;
		case Opt_grpquota:
			set_opt(sbi, GRPQUOTA);
			break;
		case Opt_prjquota:
			set_opt(sbi, PRJQUOTA);
			break;
		case Opt_usrjquota:
			ret = f2fs_set_qf_name(sb, USRQUOTA, &args[0]);
			if (ret)
				return ret;
			break;
		case Opt_grpjquota:
			ret = f2fs_set_qf_name(sb, GRPQUOTA, &args[0]);
			if (ret)
				return ret;
			break;
		case Opt_prjjquota:
			ret = f2fs_set_qf_name(sb, PRJQUOTA, &args[0]);
			if (ret)
				return ret;
			break;
		case Opt_offusrjquota:
			ret = f2fs_clear_qf_name(sb, USRQUOTA);
			if (ret)
				return ret;
			break;
		case Opt_offgrpjquota:
			ret = f2fs_clear_qf_name(sb, GRPQUOTA);
			if (ret)
				return ret;
			break;
		case Opt_offprjjquota:
			ret = f2fs_clear_qf_name(sb, PRJQUOTA);
			if (ret)
				return ret;
			break;
		case Opt_jqfmt_vfsold:
			F2FS_OPTION(sbi).s_jquota_fmt = QFMT_VFS_OLD;
			break;
		case Opt_jqfmt_vfsv0:
			F2FS_OPTION(sbi).s_jquota_fmt = QFMT_VFS_V0;
			break;
		case Opt_jqfmt_vfsv1:
			F2FS_OPTION(sbi).s_jquota_fmt = QFMT_VFS_V1;
			break;
		case Opt_noquota:
			clear_opt(sbi, QUOTA);
			clear_opt(sbi, USRQUOTA);
			clear_opt(sbi, GRPQUOTA);
			clear_opt(sbi, PRJQUOTA);
			break;
#else
		case Opt_quota:
		case Opt_usrquota:
		case Opt_grpquota:
		case Opt_prjquota:
		case Opt_usrjquota:
		case Opt_grpjquota:
		case Opt_prjjquota:
		case Opt_offusrjquota:
		case Opt_offgrpjquota:
		case Opt_offprjjquota:
		case Opt_jqfmt_vfsold:
		case Opt_jqfmt_vfsv0:
		case Opt_jqfmt_vfsv1:
		case Opt_noquota:
			f2fs_msg(sb, KERN_INFO,
					"quota operations not supported");
			break;
#endif
		case Opt_whint:
			name = match_strdup(&args[0]);
			if (!name)
				return -ENOMEM;
			if (strlen(name) == 10 &&
					!strncmp(name, "user-based", 10)) {
				F2FS_OPTION(sbi).whint_mode = WHINT_MODE_USER;
			} else if (strlen(name) == 3 &&
					!strncmp(name, "off", 3)) {
				F2FS_OPTION(sbi).whint_mode = WHINT_MODE_OFF;
			} else if (strlen(name) == 8 &&
					!strncmp(name, "fs-based", 8)) {
				F2FS_OPTION(sbi).whint_mode = WHINT_MODE_FS;
			} else {
				kfree(name);
				return -EINVAL;
			}
			kfree(name);
			break;
		case Opt_alloc:
			name = match_strdup(&args[0]);
			if (!name)
				return -ENOMEM;

			if (strlen(name) == 7 &&
					!strncmp(name, "default", 7)) {
				F2FS_OPTION(sbi).alloc_mode = ALLOC_MODE_DEFAULT;
			} else if (strlen(name) == 5 &&
					!strncmp(name, "reuse", 5)) {
				F2FS_OPTION(sbi).alloc_mode = ALLOC_MODE_REUSE;
			} else {
				kfree(name);
				return -EINVAL;
			}
			kfree(name);
			break;
		case Opt_fsync:
			name = match_strdup(&args[0]);
			if (!name)
				return -ENOMEM;
			if (strlen(name) == 5 &&
					!strncmp(name, "posix", 5)) {
				F2FS_OPTION(sbi).fsync_mode = FSYNC_MODE_POSIX;
			} else if (strlen(name) == 6 &&
					!strncmp(name, "strict", 6)) {
				F2FS_OPTION(sbi).fsync_mode = FSYNC_MODE_STRICT;
			} else if (strlen(name) == 9 &&
					!strncmp(name, "nobarrier", 9)) {
				F2FS_OPTION(sbi).fsync_mode =
							FSYNC_MODE_NOBARRIER;
			} else {
				kfree(name);
				return -EINVAL;
			}
			kfree(name);
			break;
		case Opt_test_dummy_encryption:
#ifdef CONFIG_F2FS_FS_ENCRYPTION
			if (!f2fs_sb_has_encrypt(sb)) {
				f2fs_msg(sb, KERN_ERR, "Encrypt feature is off");
				return -EINVAL;
			}

			F2FS_OPTION(sbi).test_dummy_encryption = true;
			f2fs_msg(sb, KERN_INFO,
					"Test dummy encryption mode enabled");
#else
			f2fs_msg(sb, KERN_INFO,
					"Test dummy encryption mount option ignored");
#endif
			break;
		default:
			f2fs_msg(sb, KERN_ERR,
				"Unrecognized mount option \"%s\" or missing value",
				p);
			return -EINVAL;
		}
	}
#ifdef CONFIG_QUOTA
	if (f2fs_check_quota_options(sbi))
		return -EINVAL;
#endif

	if (F2FS_IO_SIZE_BITS(sbi) && !test_opt(sbi, LFS)) {
		f2fs_msg(sb, KERN_ERR,
				"Should set mode=lfs with %uKB-sized IO",
				F2FS_IO_SIZE_KB(sbi));
		return -EINVAL;
	}

	if (test_opt(sbi, INLINE_XATTR_SIZE)) {
		if (!f2fs_sb_has_extra_attr(sb) ||
			!f2fs_sb_has_flexible_inline_xattr(sb)) {
			f2fs_msg(sb, KERN_ERR,
					"extra_attr or flexible_inline_xattr "
					"feature is off");
			return -EINVAL;
		}
		if (!test_opt(sbi, INLINE_XATTR)) {
			f2fs_msg(sb, KERN_ERR,
					"inline_xattr_size option should be "
					"set with inline_xattr option");
			return -EINVAL;
		}
		if (!F2FS_OPTION(sbi).inline_xattr_size ||
			F2FS_OPTION(sbi).inline_xattr_size >=
					DEF_ADDRS_PER_INODE -
					F2FS_TOTAL_EXTRA_ATTR_SIZE -
					DEF_INLINE_RESERVED_SIZE -
					DEF_MIN_INLINE_SIZE) {
			f2fs_msg(sb, KERN_ERR,
					"inline xattr size is out of range");
			return -EINVAL;
		}
	}

	/* Not pass down write hints if the number of active logs is lesser
	 * than NR_CURSEG_TYPE.
	 */
	if (F2FS_OPTION(sbi).active_logs != NR_CURSEG_TYPE)
		F2FS_OPTION(sbi).whint_mode = WHINT_MODE_OFF;
	return 0;
}

static struct inode *f2fs_alloc_inode(struct super_block *sb)
{
	struct f2fs_inode_info *fi;

	fi = kmem_cache_alloc(f2fs_inode_cachep, GFP_F2FS_ZERO);
	if (!fi)
		return NULL;

	init_once((void *) fi);

	/* Initialize f2fs-specific inode info */
	atomic_set(&fi->dirty_pages, 0);
	fi->i_current_depth = 1;
	init_rwsem(&fi->i_sem);
	INIT_LIST_HEAD(&fi->dirty_list);
	INIT_LIST_HEAD(&fi->gdirty_list);
	INIT_LIST_HEAD(&fi->inmem_ilist);
	INIT_LIST_HEAD(&fi->inmem_pages);
	mutex_init(&fi->inmem_lock);
	init_rwsem(&fi->dio_rwsem[READ]);
	init_rwsem(&fi->dio_rwsem[WRITE]);
	init_rwsem(&fi->i_mmap_sem);
	init_rwsem(&fi->i_xattr_sem);

	/* Will be used by directory only */
	fi->i_dir_level = F2FS_SB(sb)->dir_level;

	return &fi->vfs_inode;
}

static int f2fs_drop_inode(struct inode *inode)
{
	int ret;
	/*
	 * This is to avoid a deadlock condition like below.
	 * writeback_single_inode(inode)
	 *  - f2fs_write_data_page
	 *    - f2fs_gc -> iput -> evict
	 *       - inode_wait_for_writeback(inode)
	 */
	if ((!inode_unhashed(inode) && inode->i_state & I_SYNC)) {
		if (!inode->i_nlink && !is_bad_inode(inode)) {
			/* to avoid evict_inode call simultaneously */
			atomic_inc(&inode->i_count);
			spin_unlock(&inode->i_lock);

			/* some remained atomic pages should discarded */
			if (f2fs_is_atomic_file(inode))
				drop_inmem_pages(inode);

			/* should remain fi->extent_tree for writepage */
			f2fs_destroy_extent_node(inode);

			sb_start_intwrite(inode->i_sb);
			f2fs_i_size_write(inode, 0);

			if (F2FS_HAS_BLOCKS(inode))
				f2fs_truncate(inode);

			sb_end_intwrite(inode->i_sb);

			spin_lock(&inode->i_lock);
			atomic_dec(&inode->i_count);
		}
		trace_f2fs_drop_inode(inode, 0);
		return 0;
	}
	ret = generic_drop_inode(inode);
	trace_f2fs_drop_inode(inode, ret);
	return ret;
}

int f2fs_inode_dirtied(struct inode *inode, bool sync)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	int ret = 0;

	spin_lock(&sbi->inode_lock[DIRTY_META]);
	if (is_inode_flag_set(inode, FI_DIRTY_INODE)) {
		ret = 1;
	} else {
		set_inode_flag(inode, FI_DIRTY_INODE);
		stat_inc_dirty_inode(sbi, DIRTY_META);
	}
	if (sync && list_empty(&F2FS_I(inode)->gdirty_list)) {
		list_add_tail(&F2FS_I(inode)->gdirty_list,
				&sbi->inode_list[DIRTY_META]);
		inc_page_count(sbi, F2FS_DIRTY_IMETA);
	}
	spin_unlock(&sbi->inode_lock[DIRTY_META]);
	return ret;
}

void f2fs_inode_synced(struct inode *inode)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);

	spin_lock(&sbi->inode_lock[DIRTY_META]);
	if (!is_inode_flag_set(inode, FI_DIRTY_INODE)) {
		spin_unlock(&sbi->inode_lock[DIRTY_META]);
		return;
	}
	if (!list_empty(&F2FS_I(inode)->gdirty_list)) {
		list_del_init(&F2FS_I(inode)->gdirty_list);
		dec_page_count(sbi, F2FS_DIRTY_IMETA);
	}
	clear_inode_flag(inode, FI_DIRTY_INODE);
	clear_inode_flag(inode, FI_AUTO_RECOVER);
	stat_dec_dirty_inode(F2FS_I_SB(inode), DIRTY_META);
	spin_unlock(&sbi->inode_lock[DIRTY_META]);
}

/*
 * f2fs_dirty_inode() is called from __mark_inode_dirty()
 *
 * We should call set_dirty_inode to write the dirty inode through write_inode.
 */
static void f2fs_dirty_inode(struct inode *inode, int flags)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);

	if (inode->i_ino == F2FS_NODE_INO(sbi) ||
			inode->i_ino == F2FS_META_INO(sbi))
		return;

	if (flags == I_DIRTY_TIME)
		return;

	if (is_inode_flag_set(inode, FI_AUTO_RECOVER))
		clear_inode_flag(inode, FI_AUTO_RECOVER);

	f2fs_inode_dirtied(inode, false);
}

static void f2fs_i_callback(struct rcu_head *head)
{
	struct inode *inode = container_of(head, struct inode, i_rcu);
	kmem_cache_free(f2fs_inode_cachep, F2FS_I(inode));
}

static void f2fs_destroy_inode(struct inode *inode)
{
	call_rcu(&inode->i_rcu, f2fs_i_callback);
}

static void destroy_percpu_info(struct f2fs_sb_info *sbi)
{
	percpu_counter_destroy(&sbi->alloc_valid_block_count);
	percpu_counter_destroy(&sbi->total_valid_inode_count);
}

static void destroy_device_list(struct f2fs_sb_info *sbi)
{
	int i;

	for (i = 0; i < sbi->s_ndevs; i++) {
		blkdev_put(FDEV(i).bdev, FMODE_EXCL);
#ifdef CONFIG_BLK_DEV_ZONED
		kfree(FDEV(i).blkz_type);
#endif
	}
	kfree(sbi->devs);
}

static void f2fs_put_super(struct super_block *sb)
{
	struct f2fs_sb_info *sbi = F2FS_SB(sb);
	int i;
	bool dropped;

	f2fs_quota_off_umount(sb);

	/* prevent remaining shrinker jobs */
	mutex_lock(&sbi->umount_mutex);

	/*
	 * We don't need to do checkpoint when superblock is clean.
	 * But, the previous checkpoint was not done by umount, it needs to do
	 * clean checkpoint again.
	 */
	if (is_sbi_flag_set(sbi, SBI_IS_DIRTY) ||
			!is_set_ckpt_flags(sbi, CP_UMOUNT_FLAG)) {
		struct cp_control cpc = {
			.reason = CP_UMOUNT,
		};
		write_checkpoint(sbi, &cpc);
	}

	/* be sure to wait for any on-going discard commands */
	dropped = f2fs_wait_discard_bios(sbi);

	if (f2fs_discard_en(sbi) && !sbi->discard_blks && !dropped) {
		struct cp_control cpc = {
			.reason = CP_UMOUNT | CP_TRIMMED,
		};
		write_checkpoint(sbi, &cpc);
	}

	/* write_checkpoint can update stat informaion */
	f2fs_destroy_stats(sbi);

	/*
	 * normally superblock is clean, so we need to release this.
	 * In addition, EIO will skip do checkpoint, we need this as well.
	 */
	release_ino_entry(sbi, true);

	f2fs_leave_shrinker(sbi);
	mutex_unlock(&sbi->umount_mutex);

	/* our cp_error case, we can wait for any writeback page */
	f2fs_flush_merged_writes(sbi);

	iput(sbi->node_inode);
	iput(sbi->meta_inode);

	/* destroy f2fs internal modules */
	destroy_node_manager(sbi);
	destroy_segment_manager(sbi);

	kfree(sbi->ckpt);

	f2fs_unregister_sysfs(sbi);

	sb->s_fs_info = NULL;
	if (sbi->s_chksum_driver)
		crypto_free_shash(sbi->s_chksum_driver);
	kfree(sbi->raw_super);

	destroy_device_list(sbi);
	mempool_destroy(sbi->write_io_dummy);
#ifdef CONFIG_QUOTA
	for (i = 0; i < MAXQUOTAS; i++)
		kfree(F2FS_OPTION(sbi).s_qf_names[i]);
#endif
	destroy_percpu_info(sbi);
	for (i = 0; i < NR_PAGE_TYPE; i++)
		kfree(sbi->write_io[i]);
	kfree(sbi);
}

int f2fs_sync_fs(struct super_block *sb, int sync)
{
	struct f2fs_sb_info *sbi = F2FS_SB(sb);
	int err = 0;

	if (unlikely(f2fs_cp_error(sbi)))
		return 0;

	trace_f2fs_sync_fs(sb, sync);

	if (unlikely(is_sbi_flag_set(sbi, SBI_POR_DOING)))
		return -EAGAIN;

	if (sync) {
		struct cp_control cpc;

		cpc.reason = __get_cp_reason(sbi);

		mutex_lock(&sbi->gc_mutex);
		err = write_checkpoint(sbi, &cpc);
		mutex_unlock(&sbi->gc_mutex);
	}
	f2fs_trace_ios(NULL, 1);

	return err;
}

static int f2fs_freeze(struct super_block *sb)
{
	if (f2fs_readonly(sb))
		return 0;

	/* IO error happened before */
	if (unlikely(f2fs_cp_error(F2FS_SB(sb))))
		return -EIO;

	/* must be clean, since sync_filesystem() was already called */
	if (is_sbi_flag_set(F2FS_SB(sb), SBI_IS_DIRTY))
		return -EINVAL;
	return 0;
}

static int f2fs_unfreeze(struct super_block *sb)
{
	return 0;
}

#ifdef CONFIG_QUOTA
static int f2fs_statfs_project(struct super_block *sb,
				kprojid_t projid, struct kstatfs *buf)
{
	struct kqid qid;
	struct dquot *dquot;
	u64 limit;
	u64 curblock;

	qid = make_kqid_projid(projid);
	dquot = dqget(sb, qid);
	if (IS_ERR(dquot))
		return PTR_ERR(dquot);
	spin_lock(&dq_data_lock);

	limit = (dquot->dq_dqb.dqb_bsoftlimit ?
		 dquot->dq_dqb.dqb_bsoftlimit :
		 dquot->dq_dqb.dqb_bhardlimit) >> sb->s_blocksize_bits;
	if (limit && buf->f_blocks > limit) {
		curblock = dquot->dq_dqb.dqb_curspace >> sb->s_blocksize_bits;
		buf->f_blocks = limit;
		buf->f_bfree = buf->f_bavail =
			(buf->f_blocks > curblock) ?
			 (buf->f_blocks - curblock) : 0;
	}

	limit = dquot->dq_dqb.dqb_isoftlimit ?
		dquot->dq_dqb.dqb_isoftlimit :
		dquot->dq_dqb.dqb_ihardlimit;
	if (limit && buf->f_files > limit) {
		buf->f_files = limit;
		buf->f_ffree =
			(buf->f_files > dquot->dq_dqb.dqb_curinodes) ?
			 (buf->f_files - dquot->dq_dqb.dqb_curinodes) : 0;
	}

	spin_unlock(&dq_data_lock);
	dqput(dquot);
	return 0;
}
#endif

static int f2fs_statfs(struct dentry *dentry, struct kstatfs *buf)
{
	struct super_block *sb = dentry->d_sb;
	struct f2fs_sb_info *sbi = F2FS_SB(sb);
	u64 id = huge_encode_dev(sb->s_bdev->bd_dev);
	block_t total_count, user_block_count, start_count;
	u64 avail_node_count;

	total_count = le64_to_cpu(sbi->raw_super->block_count);
	user_block_count = sbi->user_block_count;
	start_count = le32_to_cpu(sbi->raw_super->segment0_blkaddr);
	buf->f_type = F2FS_SUPER_MAGIC;
	buf->f_bsize = sbi->blocksize;

	buf->f_blocks = total_count - start_count;
	buf->f_bfree = user_block_count - valid_user_blocks(sbi) -
						sbi->current_reserved_blocks;
	if (buf->f_bfree > F2FS_OPTION(sbi).root_reserved_blocks)
		buf->f_bavail = buf->f_bfree -
				F2FS_OPTION(sbi).root_reserved_blocks;
	else
		buf->f_bavail = 0;

	avail_node_count = sbi->total_node_count - sbi->nquota_files -
						F2FS_RESERVED_NODE_NUM;

	if (avail_node_count > user_block_count) {
		buf->f_files = user_block_count;
		buf->f_ffree = buf->f_bavail;
	} else {
		buf->f_files = avail_node_count;
		buf->f_ffree = min(avail_node_count - valid_node_count(sbi),
					buf->f_bavail);
	}

	buf->f_namelen = F2FS_NAME_LEN;
	buf->f_fsid.val[0] = (u32)id;
	buf->f_fsid.val[1] = (u32)(id >> 32);

#ifdef CONFIG_QUOTA
	if (is_inode_flag_set(dentry->d_inode, FI_PROJ_INHERIT) &&
			sb_has_quota_limits_enabled(sb, PRJQUOTA)) {
		f2fs_statfs_project(sb, F2FS_I(dentry->d_inode)->i_projid, buf);
	}
#endif
	return 0;
}

static inline void f2fs_show_quota_options(struct seq_file *seq,
					   struct super_block *sb)
{
#ifdef CONFIG_QUOTA
	struct f2fs_sb_info *sbi = F2FS_SB(sb);

	if (F2FS_OPTION(sbi).s_jquota_fmt) {
		char *fmtname = "";

		switch (F2FS_OPTION(sbi).s_jquota_fmt) {
		case QFMT_VFS_OLD:
			fmtname = "vfsold";
			break;
		case QFMT_VFS_V0:
			fmtname = "vfsv0";
			break;
		case QFMT_VFS_V1:
			fmtname = "vfsv1";
			break;
		}
		seq_printf(seq, ",jqfmt=%s", fmtname);
	}

	if (F2FS_OPTION(sbi).s_qf_names[USRQUOTA])
		seq_show_option(seq, "usrjquota",
			F2FS_OPTION(sbi).s_qf_names[USRQUOTA]);

	if (F2FS_OPTION(sbi).s_qf_names[GRPQUOTA])
		seq_show_option(seq, "grpjquota",
			F2FS_OPTION(sbi).s_qf_names[GRPQUOTA]);

	if (F2FS_OPTION(sbi).s_qf_names[PRJQUOTA])
		seq_show_option(seq, "prjjquota",
			F2FS_OPTION(sbi).s_qf_names[PRJQUOTA]);
#endif
}

static int f2fs_show_options(struct seq_file *seq, struct dentry *root)
{
	struct f2fs_sb_info *sbi = F2FS_SB(root->d_sb);

	if (!f2fs_readonly(sbi->sb) && test_opt(sbi, BG_GC)) {
		if (test_opt(sbi, FORCE_FG_GC))
			seq_printf(seq, ",background_gc=%s", "sync");
		else
			seq_printf(seq, ",background_gc=%s", "on");
	} else {
		seq_printf(seq, ",background_gc=%s", "off");
	}
	if (test_opt(sbi, DISABLE_ROLL_FORWARD))
		seq_puts(seq, ",disable_roll_forward");
	if (test_opt(sbi, DISCARD))
		seq_puts(seq, ",discard");
	if (test_opt(sbi, NOHEAP))
		seq_puts(seq, ",no_heap");
	else
		seq_puts(seq, ",heap");
#ifdef CONFIG_F2FS_FS_XATTR
	if (test_opt(sbi, XATTR_USER))
		seq_puts(seq, ",user_xattr");
	else
		seq_puts(seq, ",nouser_xattr");
	if (test_opt(sbi, INLINE_XATTR))
		seq_puts(seq, ",inline_xattr");
	else
		seq_puts(seq, ",noinline_xattr");
	if (test_opt(sbi, INLINE_XATTR_SIZE))
		seq_printf(seq, ",inline_xattr_size=%u",
					F2FS_OPTION(sbi).inline_xattr_size);
#endif
#ifdef CONFIG_F2FS_FS_POSIX_ACL
	if (test_opt(sbi, POSIX_ACL))
		seq_puts(seq, ",acl");
	else
		seq_puts(seq, ",noacl");
#endif
	if (test_opt(sbi, DISABLE_EXT_IDENTIFY))
		seq_puts(seq, ",disable_ext_identify");
	if (test_opt(sbi, INLINE_DATA))
		seq_puts(seq, ",inline_data");
	else
		seq_puts(seq, ",noinline_data");
	if (test_opt(sbi, INLINE_DENTRY))
		seq_puts(seq, ",inline_dentry");
	else
		seq_puts(seq, ",noinline_dentry");
	if (!f2fs_readonly(sbi->sb) && test_opt(sbi, FLUSH_MERGE))
		seq_puts(seq, ",flush_merge");
	if (test_opt(sbi, NOBARRIER))
		seq_puts(seq, ",nobarrier");
	if (test_opt(sbi, FASTBOOT))
		seq_puts(seq, ",fastboot");
	if (test_opt(sbi, EXTENT_CACHE))
		seq_puts(seq, ",extent_cache");
	else
		seq_puts(seq, ",noextent_cache");
	if (test_opt(sbi, DATA_FLUSH))
		seq_puts(seq, ",data_flush");

	seq_puts(seq, ",mode=");
	if (test_opt(sbi, ADAPTIVE))
		seq_puts(seq, "adaptive");
	else if (test_opt(sbi, LFS))
		seq_puts(seq, "lfs");
	seq_printf(seq, ",active_logs=%u", F2FS_OPTION(sbi).active_logs);
	if (test_opt(sbi, RESERVE_ROOT))
		seq_printf(seq, ",reserve_root=%u,resuid=%u,resgid=%u",
				F2FS_OPTION(sbi).root_reserved_blocks,
				from_kuid_munged(&init_user_ns,
					F2FS_OPTION(sbi).s_resuid),
				from_kgid_munged(&init_user_ns,
					F2FS_OPTION(sbi).s_resgid));
	if (F2FS_IO_SIZE_BITS(sbi))
		seq_printf(seq, ",io_size=%uKB", F2FS_IO_SIZE_KB(sbi));
#ifdef CONFIG_F2FS_FAULT_INJECTION
	if (test_opt(sbi, FAULT_INJECTION))
		seq_printf(seq, ",fault_injection=%u",
				F2FS_OPTION(sbi).fault_info.inject_rate);
#endif
#ifdef CONFIG_QUOTA
	if (test_opt(sbi, QUOTA))
		seq_puts(seq, ",quota");
	if (test_opt(sbi, USRQUOTA))
		seq_puts(seq, ",usrquota");
	if (test_opt(sbi, GRPQUOTA))
		seq_puts(seq, ",grpquota");
	if (test_opt(sbi, PRJQUOTA))
		seq_puts(seq, ",prjquota");
#endif
	f2fs_show_quota_options(seq, sbi->sb);
	if (F2FS_OPTION(sbi).whint_mode == WHINT_MODE_USER)
		seq_printf(seq, ",whint_mode=%s", "user-based");
	else if (F2FS_OPTION(sbi).whint_mode == WHINT_MODE_FS)
		seq_printf(seq, ",whint_mode=%s", "fs-based");
#ifdef CONFIG_F2FS_FS_ENCRYPTION
	if (F2FS_OPTION(sbi).test_dummy_encryption)
		seq_puts(seq, ",test_dummy_encryption");
#endif

	if (F2FS_OPTION(sbi).alloc_mode == ALLOC_MODE_DEFAULT)
		seq_printf(seq, ",alloc_mode=%s", "default");
	else if (F2FS_OPTION(sbi).alloc_mode == ALLOC_MODE_REUSE)
		seq_printf(seq, ",alloc_mode=%s", "reuse");

	if (F2FS_OPTION(sbi).fsync_mode == FSYNC_MODE_POSIX)
		seq_printf(seq, ",fsync_mode=%s", "posix");
	else if (F2FS_OPTION(sbi).fsync_mode == FSYNC_MODE_STRICT)
		seq_printf(seq, ",fsync_mode=%s", "strict");
	return 0;
}

static void default_options(struct f2fs_sb_info *sbi)
{
	/* init some FS parameters */
	F2FS_OPTION(sbi).active_logs = NR_CURSEG_TYPE;
	F2FS_OPTION(sbi).inline_xattr_size = DEFAULT_INLINE_XATTR_ADDRS;
	F2FS_OPTION(sbi).whint_mode = WHINT_MODE_OFF;
	F2FS_OPTION(sbi).alloc_mode = ALLOC_MODE_DEFAULT;
	F2FS_OPTION(sbi).fsync_mode = FSYNC_MODE_POSIX;
	F2FS_OPTION(sbi).test_dummy_encryption = false;
	sbi->readdir_ra = 1;

	set_opt(sbi, BG_GC);
	set_opt(sbi, INLINE_XATTR);
	set_opt(sbi, INLINE_DATA);
	set_opt(sbi, INLINE_DENTRY);
	set_opt(sbi, EXTENT_CACHE);
	set_opt(sbi, NOHEAP);
	sbi->sb->s_flags |= MS_LAZYTIME;
	set_opt(sbi, FLUSH_MERGE);
	if (f2fs_sb_has_blkzoned(sbi->sb)) {
		set_opt_mode(sbi, F2FS_MOUNT_LFS);
		set_opt(sbi, DISCARD);
	} else {
		set_opt_mode(sbi, F2FS_MOUNT_ADAPTIVE);
	}

#ifdef CONFIG_F2FS_FS_XATTR
	set_opt(sbi, XATTR_USER);
#endif
#ifdef CONFIG_F2FS_FS_POSIX_ACL
	set_opt(sbi, POSIX_ACL);
#endif

#ifdef CONFIG_F2FS_FAULT_INJECTION
	f2fs_build_fault_attr(sbi, 0);
#endif
}

#ifdef CONFIG_QUOTA
static int f2fs_enable_quotas(struct super_block *sb);
#endif
static int f2fs_remount(struct super_block *sb, int *flags, char *data)
{
	struct f2fs_sb_info *sbi = F2FS_SB(sb);
	struct f2fs_mount_info org_mount_opt;
	unsigned long old_sb_flags;
	int err;
	bool need_restart_gc = false;
	bool need_stop_gc = false;
	bool no_extent_cache = !test_opt(sbi, EXTENT_CACHE);
#ifdef CONFIG_QUOTA
	int i, j;
#endif

	/*
	 * Save the old mount options in case we
	 * need to restore them.
	 */
	org_mount_opt = sbi->mount_opt;
	old_sb_flags = sb->s_flags;

#ifdef CONFIG_QUOTA
	org_mount_opt.s_jquota_fmt = F2FS_OPTION(sbi).s_jquota_fmt;
	for (i = 0; i < MAXQUOTAS; i++) {
		if (F2FS_OPTION(sbi).s_qf_names[i]) {
			org_mount_opt.s_qf_names[i] =
				kstrdup(F2FS_OPTION(sbi).s_qf_names[i],
				GFP_KERNEL);
			if (!org_mount_opt.s_qf_names[i]) {
				for (j = 0; j < i; j++)
					kfree(org_mount_opt.s_qf_names[j]);
				return -ENOMEM;
			}
		} else {
			org_mount_opt.s_qf_names[i] = NULL;
		}
	}
#endif

	/* recover superblocks we couldn't write due to previous RO mount */
	if (!(*flags & MS_RDONLY) && is_sbi_flag_set(sbi, SBI_NEED_SB_WRITE)) {
		err = f2fs_commit_super(sbi, false);
		f2fs_msg(sb, KERN_INFO,
			"Try to recover all the superblocks, ret: %d", err);
		if (!err)
			clear_sbi_flag(sbi, SBI_NEED_SB_WRITE);
	}

	default_options(sbi);

	/* parse mount options */
	err = parse_options(sb, data);
	if (err)
		goto restore_opts;

	/*
	 * Previous and new state of filesystem is RO,
	 * so skip checking GC and FLUSH_MERGE conditions.
	 */
	if (f2fs_readonly(sb) && (*flags & MS_RDONLY))
		goto skip;

#ifdef CONFIG_QUOTA
	if (!f2fs_readonly(sb) && (*flags & MS_RDONLY)) {
		err = dquot_suspend(sb, -1);
		if (err < 0)
			goto restore_opts;
	} else if (f2fs_readonly(sb) && !(*flags & MS_RDONLY)) {
		/* dquot_resume needs RW */
		sb->s_flags &= ~MS_RDONLY;
		if (sb_any_quota_suspended(sb)) {
			dquot_resume(sb, -1);
		} else if (f2fs_sb_has_quota_ino(sb)) {
			err = f2fs_enable_quotas(sb);
			if (err)
				goto restore_opts;
		}
	}
#endif
	/* disallow enable/disable extent_cache dynamically */
	if (no_extent_cache == !!test_opt(sbi, EXTENT_CACHE)) {
		err = -EINVAL;
		f2fs_msg(sbi->sb, KERN_WARNING,
				"switch extent_cache option is not allowed");
		goto restore_opts;
	}

	/*
	 * We stop the GC thread if FS is mounted as RO
	 * or if background_gc = off is passed in mount
	 * option. Also sync the filesystem.
	 */
	if ((*flags & MS_RDONLY) || !test_opt(sbi, BG_GC)) {
		if (sbi->gc_thread) {
			stop_gc_thread(sbi);
			need_restart_gc = true;
		}
	} else if (!sbi->gc_thread) {
		err = start_gc_thread(sbi);
		if (err)
			goto restore_opts;
		need_stop_gc = true;
	}

	if (*flags & MS_RDONLY ||
		F2FS_OPTION(sbi).whint_mode != org_mount_opt.whint_mode) {
		writeback_inodes_sb(sb, WB_REASON_SYNC);
		sync_inodes_sb(sb);

		set_sbi_flag(sbi, SBI_IS_DIRTY);
		set_sbi_flag(sbi, SBI_IS_CLOSE);
		f2fs_sync_fs(sb, 1);
		clear_sbi_flag(sbi, SBI_IS_CLOSE);
	}

	/*
	 * We stop issue flush thread if FS is mounted as RO
	 * or if flush_merge is not passed in mount option.
	 */
	if ((*flags & MS_RDONLY) || !test_opt(sbi, FLUSH_MERGE)) {
		clear_opt(sbi, FLUSH_MERGE);
		destroy_flush_cmd_control(sbi, false);
	} else {
		err = create_flush_cmd_control(sbi);
		if (err)
			goto restore_gc;
	}
skip:
#ifdef CONFIG_QUOTA
	/* Release old quota file names */
	for (i = 0; i < MAXQUOTAS; i++)
		kfree(org_mount_opt.s_qf_names[i]);
#endif
	/* Update the POSIXACL Flag */
	sb->s_flags = (sb->s_flags & ~MS_POSIXACL) |
		(test_opt(sbi, POSIX_ACL) ? MS_POSIXACL : 0);

	limit_reserve_root(sbi);
	return 0;
restore_gc:
	if (need_restart_gc) {
		if (start_gc_thread(sbi))
			f2fs_msg(sbi->sb, KERN_WARNING,
				"background gc thread has stopped");
	} else if (need_stop_gc) {
		stop_gc_thread(sbi);
	}
restore_opts:
#ifdef CONFIG_QUOTA
	F2FS_OPTION(sbi).s_jquota_fmt = org_mount_opt.s_jquota_fmt;
	for (i = 0; i < MAXQUOTAS; i++) {
		kfree(F2FS_OPTION(sbi).s_qf_names[i]);
		F2FS_OPTION(sbi).s_qf_names[i] = org_mount_opt.s_qf_names[i];
	}
#endif
	sbi->mount_opt = org_mount_opt;
	sb->s_flags = old_sb_flags;
	return err;
}

#ifdef CONFIG_QUOTA
/* Read data from quotafile */
static ssize_t f2fs_quota_read(struct super_block *sb, int type, char *data,
			       size_t len, loff_t off)
{
	struct inode *inode = sb_dqopt(sb)->files[type];
	struct address_space *mapping = inode->i_mapping;
	block_t blkidx = F2FS_BYTES_TO_BLK(off);
	int offset = off & (sb->s_blocksize - 1);
	int tocopy;
	size_t toread;
	loff_t i_size = i_size_read(inode);
	struct page *page;
	char *kaddr;

	if (off > i_size)
		return 0;

	if (off + len > i_size)
		len = i_size - off;
	toread = len;
	while (toread > 0) {
		tocopy = min_t(unsigned long, sb->s_blocksize - offset, toread);
repeat:
		page = read_cache_page_gfp(mapping, blkidx, GFP_NOFS);
		if (IS_ERR(page)) {
			if (PTR_ERR(page) == -ENOMEM) {
				congestion_wait(BLK_RW_ASYNC, HZ/50);
				goto repeat;
			}
			return PTR_ERR(page);
		}

		lock_page(page);

		if (unlikely(page->mapping != mapping)) {
			f2fs_put_page(page, 1);
			goto repeat;
		}
		if (unlikely(!PageUptodate(page))) {
			f2fs_put_page(page, 1);
			return -EIO;
		}

		kaddr = kmap_atomic(page);
		memcpy(data, kaddr + offset, tocopy);
		kunmap_atomic(kaddr);
		f2fs_put_page(page, 1);

		offset = 0;
		toread -= tocopy;
		data += tocopy;
		blkidx++;
	}
	return len;
}

/* Write to quotafile */
static ssize_t f2fs_quota_write(struct super_block *sb, int type,
				const char *data, size_t len, loff_t off)
{
	struct inode *inode = sb_dqopt(sb)->files[type];
	struct address_space *mapping = inode->i_mapping;
	const struct address_space_operations *a_ops = mapping->a_ops;
	int offset = off & (sb->s_blocksize - 1);
	size_t towrite = len;
	struct page *page;
	char *kaddr;
	int err = 0;
	int tocopy;

	while (towrite > 0) {
		tocopy = min_t(unsigned long, sb->s_blocksize - offset,
								towrite);
retry:
		err = a_ops->write_begin(NULL, mapping, off, tocopy, 0,
							&page, NULL);
		if (unlikely(err)) {
			if (err == -ENOMEM) {
				congestion_wait(BLK_RW_ASYNC, HZ/50);
				goto retry;
			}
			break;
		}

		kaddr = kmap_atomic(page);
		memcpy(kaddr + offset, data, tocopy);
		kunmap_atomic(kaddr);
		flush_dcache_page(page);

		a_ops->write_end(NULL, mapping, off, tocopy, tocopy,
						page, NULL);
		offset = 0;
		towrite -= tocopy;
		off += tocopy;
		data += tocopy;
		cond_resched();
	}

	if (len == towrite)
		return err;
	inode->i_mtime = inode->i_ctime = current_time(inode);
	f2fs_mark_inode_dirty_sync(inode, false);
	return len - towrite;
}

static struct dquot **f2fs_get_dquots(struct inode *inode)
{
	return F2FS_I(inode)->i_dquot;
}

static qsize_t *f2fs_get_reserved_space(struct inode *inode)
{
	return &F2FS_I(inode)->i_reserved_quota;
}

static int f2fs_quota_on_mount(struct f2fs_sb_info *sbi, int type)
{
	return dquot_quota_on_mount(sbi->sb, F2FS_OPTION(sbi).s_qf_names[type],
					F2FS_OPTION(sbi).s_jquota_fmt, type);
}

int f2fs_enable_quota_files(struct f2fs_sb_info *sbi, bool rdonly)
{
	int enabled = 0;
	int i, err;

	if (f2fs_sb_has_quota_ino(sbi->sb) && rdonly) {
		err = f2fs_enable_quotas(sbi->sb);
		if (err) {
			f2fs_msg(sbi->sb, KERN_ERR,
					"Cannot turn on quota_ino: %d", err);
			return 0;
		}
		return 1;
	}

	for (i = 0; i < MAXQUOTAS; i++) {
		if (F2FS_OPTION(sbi).s_qf_names[i]) {
			err = f2fs_quota_on_mount(sbi, i);
			if (!err) {
				enabled = 1;
				continue;
			}
			f2fs_msg(sbi->sb, KERN_ERR,
				"Cannot turn on quotas: %d on %d", err, i);
		}
	}
	return enabled;
}

static int f2fs_quota_enable(struct super_block *sb, int type, int format_id,
			     unsigned int flags)
{
	struct inode *qf_inode;
	unsigned long qf_inum;
	int err;

	BUG_ON(!f2fs_sb_has_quota_ino(sb));

	qf_inum = f2fs_qf_ino(sb, type);
	if (!qf_inum)
		return -EPERM;

	qf_inode = f2fs_iget(sb, qf_inum);
	if (IS_ERR(qf_inode)) {
		f2fs_msg(sb, KERN_ERR,
			"Bad quota inode %u:%lu", type, qf_inum);
		return PTR_ERR(qf_inode);
	}

	/* Don't account quota for quota files to avoid recursion */
	qf_inode->i_flags |= S_NOQUOTA;
	err = dquot_enable(qf_inode, type, format_id, flags);
	iput(qf_inode);
	return err;
}

static int f2fs_enable_quotas(struct super_block *sb)
{
	int type, err = 0;
	unsigned long qf_inum;
	bool quota_mopt[MAXQUOTAS] = {
		test_opt(F2FS_SB(sb), USRQUOTA),
		test_opt(F2FS_SB(sb), GRPQUOTA),
		test_opt(F2FS_SB(sb), PRJQUOTA),
	};

	sb_dqopt(sb)->flags |= DQUOT_QUOTA_SYS_FILE;
	for (type = 0; type < MAXQUOTAS; type++) {
		qf_inum = f2fs_qf_ino(sb, type);
		if (qf_inum) {
			err = f2fs_quota_enable(sb, type, QFMT_VFS_V1,
				DQUOT_USAGE_ENABLED |
				(quota_mopt[type] ? DQUOT_LIMITS_ENABLED : 0));
			if (err) {
				f2fs_msg(sb, KERN_ERR,
					"Failed to enable quota tracking "
					"(type=%d, err=%d). Please run "
					"fsck to fix.", type, err);
				for (type--; type >= 0; type--)
					dquot_quota_off(sb, type);
				return err;
			}
		}
	}
	return 0;
}

static int f2fs_quota_sync(struct super_block *sb, int type)
{
	struct quota_info *dqopt = sb_dqopt(sb);
	int cnt;
	int ret;

	ret = dquot_writeback_dquots(sb, type);
	if (ret)
		return ret;

	/*
	 * Now when everything is written we can discard the pagecache so
	 * that userspace sees the changes.
	 */
	for (cnt = 0; cnt < MAXQUOTAS; cnt++) {
		if (type != -1 && cnt != type)
			continue;
		if (!sb_has_quota_active(sb, cnt))
			continue;

		ret = filemap_write_and_wait(dqopt->files[cnt]->i_mapping);
		if (ret)
			return ret;

		inode_lock(dqopt->files[cnt]);
		truncate_inode_pages(&dqopt->files[cnt]->i_data, 0);
		inode_unlock(dqopt->files[cnt]);
	}
	return 0;
}

static int f2fs_quota_on(struct super_block *sb, int type, int format_id,
							struct path *path)
{
	struct inode *inode;
	int err;

	err = f2fs_quota_sync(sb, type);
	if (err)
		return err;

	err = dquot_quota_on(sb, type, format_id, path);
	if (err)
		return err;

	inode = d_inode(path->dentry);

	inode_lock(inode);
	F2FS_I(inode)->i_flags |= FS_NOATIME_FL | FS_IMMUTABLE_FL;
	inode_set_flags(inode, S_NOATIME | S_IMMUTABLE,
					S_NOATIME | S_IMMUTABLE);
	inode_unlock(inode);
	f2fs_mark_inode_dirty_sync(inode, false);

	return 0;
}

static int f2fs_quota_off(struct super_block *sb, int type)
{
	struct inode *inode = sb_dqopt(sb)->files[type];
	int err;

	if (!inode || !igrab(inode))
		return dquot_quota_off(sb, type);

	f2fs_quota_sync(sb, type);

	err = dquot_quota_off(sb, type);
	if (err || f2fs_sb_has_quota_ino(sb))
		goto out_put;

	inode_lock(inode);
	F2FS_I(inode)->i_flags &= ~(FS_NOATIME_FL | FS_IMMUTABLE_FL);
	inode_set_flags(inode, 0, S_NOATIME | S_IMMUTABLE);
	inode_unlock(inode);
	f2fs_mark_inode_dirty_sync(inode, false);
out_put:
	iput(inode);
	return err;
}

void f2fs_quota_off_umount(struct super_block *sb)
{
	int type;

	for (type = 0; type < MAXQUOTAS; type++)
		f2fs_quota_off(sb, type);
}

static int f2fs_get_projid(struct inode *inode, kprojid_t *projid)
{
	*projid = F2FS_I(inode)->i_projid;
	return 0;
}

static const struct dquot_operations f2fs_quota_operations = {
	.get_reserved_space = f2fs_get_reserved_space,
	.write_dquot	= dquot_commit,
	.acquire_dquot	= dquot_acquire,
	.release_dquot	= dquot_release,
	.mark_dirty	= dquot_mark_dquot_dirty,
	.write_info	= dquot_commit_info,
	.alloc_dquot	= dquot_alloc,
	.destroy_dquot	= dquot_destroy,
	.get_projid	= f2fs_get_projid,
	.get_next_id	= dquot_get_next_id,
};

static const struct quotactl_ops f2fs_quotactl_ops = {
	.quota_on	= f2fs_quota_on,
	.quota_off	= f2fs_quota_off,
	.quota_sync	= f2fs_quota_sync,
	.get_state	= dquot_get_state,
	.set_info	= dquot_set_dqinfo,
	.get_dqblk	= dquot_get_dqblk,
	.set_dqblk	= dquot_set_dqblk,
	.get_nextdqblk	= dquot_get_next_dqblk,
};
#else
void f2fs_quota_off_umount(struct super_block *sb)
{
}
#endif

static const struct super_operations f2fs_sops = {
	.alloc_inode	= f2fs_alloc_inode,
	.drop_inode	= f2fs_drop_inode,
	.destroy_inode	= f2fs_destroy_inode,
	.write_inode	= f2fs_write_inode,
	.dirty_inode	= f2fs_dirty_inode,
	.show_options	= f2fs_show_options,
#ifdef CONFIG_QUOTA
	.quota_read	= f2fs_quota_read,
	.quota_write	= f2fs_quota_write,
	.get_dquots	= f2fs_get_dquots,
#endif
	.evict_inode	= f2fs_evict_inode,
	.put_super	= f2fs_put_super,
	.sync_fs	= f2fs_sync_fs,
	.freeze_fs	= f2fs_freeze,
	.unfreeze_fs	= f2fs_unfreeze,
	.statfs		= f2fs_statfs,
	.remount_fs	= f2fs_remount,
};

#ifdef CONFIG_F2FS_FS_ENCRYPTION
static int f2fs_get_context(struct inode *inode, void *ctx, size_t len)
{
	return f2fs_getxattr(inode, F2FS_XATTR_INDEX_ENCRYPTION,
				F2FS_XATTR_NAME_ENCRYPTION_CONTEXT,
				ctx, len, NULL);
}

static int f2fs_set_context(struct inode *inode, const void *ctx, size_t len,
							void *fs_data)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);

	/*
	 * Encrypting the root directory is not allowed because fsck
	 * expects lost+found directory to exist and remain unencrypted
	 * if LOST_FOUND feature is enabled.
	 *
	 */
	if (f2fs_sb_has_lost_found(sbi->sb) &&
			inode->i_ino == F2FS_ROOT_INO(sbi))
		return -EPERM;

	return f2fs_setxattr(inode, F2FS_XATTR_INDEX_ENCRYPTION,
				F2FS_XATTR_NAME_ENCRYPTION_CONTEXT,
				ctx, len, fs_data, XATTR_CREATE);
}

static bool f2fs_dummy_context(struct inode *inode)
{
	return DUMMY_ENCRYPTION_ENABLED(F2FS_I_SB(inode));
}

static unsigned f2fs_max_namelen(struct inode *inode)
{
	return S_ISLNK(inode->i_mode) ?
			inode->i_sb->s_blocksize : F2FS_NAME_LEN;
}

static inline bool f2fs_is_encrypted(struct inode *inode)
{
	return f2fs_encrypted_file(inode);
}

static const struct fscrypt_operations f2fs_cryptops = {
	.key_prefix	= "f2fs:",
	.get_context	= f2fs_get_context,
	.set_context	= f2fs_set_context,
	.dummy_context	= f2fs_dummy_context,
	.empty_dir	= f2fs_empty_dir,
	.max_namelen	= f2fs_max_namelen,
	.is_encrypted = f2fs_is_encrypted,
};
#endif

static struct inode *f2fs_nfs_get_inode(struct super_block *sb,
		u64 ino, u32 generation)
{
	struct f2fs_sb_info *sbi = F2FS_SB(sb);
	struct inode *inode;

	if (check_nid_range(sbi, ino))
		return ERR_PTR(-ESTALE);

	/*
	 * f2fs_iget isn't quite right if the inode is currently unallocated!
	 * However f2fs_iget currently does appropriate checks to handle stale
	 * inodes so everything is OK.
	 */
	inode = f2fs_iget(sb, ino);
	if (IS_ERR(inode))
		return ERR_CAST(inode);
	if (unlikely(generation && inode->i_generation != generation)) {
		/* we didn't find the right inode.. */
		iput(inode);
		return ERR_PTR(-ESTALE);
	}
	return inode;
}

static struct dentry *f2fs_fh_to_dentry(struct super_block *sb, struct fid *fid,
		int fh_len, int fh_type)
{
	return generic_fh_to_dentry(sb, fid, fh_len, fh_type,
				    f2fs_nfs_get_inode);
}

static struct dentry *f2fs_fh_to_parent(struct super_block *sb, struct fid *fid,
		int fh_len, int fh_type)
{
	return generic_fh_to_parent(sb, fid, fh_len, fh_type,
				    f2fs_nfs_get_inode);
}

static const struct export_operations f2fs_export_ops = {
	.fh_to_dentry = f2fs_fh_to_dentry,
	.fh_to_parent = f2fs_fh_to_parent,
	.get_parent = f2fs_get_parent,
};

static loff_t max_file_blocks(void)
{
	loff_t result = 0;
	loff_t leaf_count = ADDRS_PER_BLOCK;

	/*
	 * note: previously, result is equal to (DEF_ADDRS_PER_INODE -
	 * DEFAULT_INLINE_XATTR_ADDRS), but now f2fs try to reserve more
	 * space in inode.i_addr, it will be more safe to reassign
	 * result as zero.
	 */

	/* two direct node blocks */
	result += (leaf_count * 2);

	/* two indirect node blocks */
	leaf_count *= NIDS_PER_BLOCK;
	result += (leaf_count * 2);

	/* one double indirect node block */
	leaf_count *= NIDS_PER_BLOCK;
	result += leaf_count;

	return result;
}

static int __f2fs_commit_super(struct buffer_head *bh,
			struct f2fs_super_block *super)
{
	lock_buffer(bh);
	if (super)
		memcpy(bh->b_data + F2FS_SUPER_OFFSET, super, sizeof(*super));
	set_buffer_dirty(bh);
	unlock_buffer(bh);

	/* it's rare case, we can do fua all the time */
	return __sync_dirty_buffer(bh, REQ_SYNC | REQ_PREFLUSH | REQ_FUA);
}

static inline bool sanity_check_area_boundary(struct f2fs_sb_info *sbi,
					struct buffer_head *bh)
{
	struct f2fs_super_block *raw_super = (struct f2fs_super_block *)
					(bh->b_data + F2FS_SUPER_OFFSET);
	struct super_block *sb = sbi->sb;
	u32 segment0_blkaddr = le32_to_cpu(raw_super->segment0_blkaddr);
	u32 cp_blkaddr = le32_to_cpu(raw_super->cp_blkaddr);
	u32 sit_blkaddr = le32_to_cpu(raw_super->sit_blkaddr);
	u32 nat_blkaddr = le32_to_cpu(raw_super->nat_blkaddr);
	u32 ssa_blkaddr = le32_to_cpu(raw_super->ssa_blkaddr);
	u32 main_blkaddr = le32_to_cpu(raw_super->main_blkaddr);
	u32 segment_count_ckpt = le32_to_cpu(raw_super->segment_count_ckpt);
	u32 segment_count_sit = le32_to_cpu(raw_super->segment_count_sit);
	u32 segment_count_nat = le32_to_cpu(raw_super->segment_count_nat);
	u32 segment_count_ssa = le32_to_cpu(raw_super->segment_count_ssa);
	u32 segment_count_main = le32_to_cpu(raw_super->segment_count_main);
	u32 segment_count = le32_to_cpu(raw_super->segment_count);
	u32 log_blocks_per_seg = le32_to_cpu(raw_super->log_blocks_per_seg);
	u64 main_end_blkaddr = main_blkaddr +
				(segment_count_main << log_blocks_per_seg);
	u64 seg_end_blkaddr = segment0_blkaddr +
				(segment_count << log_blocks_per_seg);

	if (segment0_blkaddr != cp_blkaddr) {
		f2fs_msg(sb, KERN_INFO,
			"Mismatch start address, segment0(%u) cp_blkaddr(%u)",
			segment0_blkaddr, cp_blkaddr);
		return true;
	}

	if (cp_blkaddr + (segment_count_ckpt << log_blocks_per_seg) !=
							sit_blkaddr) {
		f2fs_msg(sb, KERN_INFO,
			"Wrong CP boundary, start(%u) end(%u) blocks(%u)",
			cp_blkaddr, sit_blkaddr,
			segment_count_ckpt << log_blocks_per_seg);
		return true;
	}

	if (sit_blkaddr + (segment_count_sit << log_blocks_per_seg) !=
							nat_blkaddr) {
		f2fs_msg(sb, KERN_INFO,
			"Wrong SIT boundary, start(%u) end(%u) blocks(%u)",
			sit_blkaddr, nat_blkaddr,
			segment_count_sit << log_blocks_per_seg);
		return true;
	}

	if (nat_blkaddr + (segment_count_nat << log_blocks_per_seg) !=
							ssa_blkaddr) {
		f2fs_msg(sb, KERN_INFO,
			"Wrong NAT boundary, start(%u) end(%u) blocks(%u)",
			nat_blkaddr, ssa_blkaddr,
			segment_count_nat << log_blocks_per_seg);
		return true;
	}

	if (ssa_blkaddr + (segment_count_ssa << log_blocks_per_seg) !=
							main_blkaddr) {
		f2fs_msg(sb, KERN_INFO,
			"Wrong SSA boundary, start(%u) end(%u) blocks(%u)",
			ssa_blkaddr, main_blkaddr,
			segment_count_ssa << log_blocks_per_seg);
		return true;
	}

	if (main_end_blkaddr > seg_end_blkaddr) {
		f2fs_msg(sb, KERN_INFO,
			"Wrong MAIN_AREA boundary, start(%u) end(%u) block(%u)",
			main_blkaddr,
			segment0_blkaddr +
				(segment_count << log_blocks_per_seg),
			segment_count_main << log_blocks_per_seg);
		return true;
	} else if (main_end_blkaddr < seg_end_blkaddr) {
		int err = 0;
		char *res;

		/* fix in-memory information all the time */
		raw_super->segment_count = cpu_to_le32((main_end_blkaddr -
				segment0_blkaddr) >> log_blocks_per_seg);

		if (f2fs_readonly(sb) || bdev_read_only(sb->s_bdev)) {
			set_sbi_flag(sbi, SBI_NEED_SB_WRITE);
			res = "internally";
		} else {
			err = __f2fs_commit_super(bh, NULL);
			res = err ? "failed" : "done";
		}
		f2fs_msg(sb, KERN_INFO,
			"Fix alignment : %s, start(%u) end(%u) block(%u)",
			res, main_blkaddr,
			segment0_blkaddr +
				(segment_count << log_blocks_per_seg),
			segment_count_main << log_blocks_per_seg);
		if (err)
			return true;
	}
	return false;
}

static int sanity_check_raw_super(struct f2fs_sb_info *sbi,
				struct buffer_head *bh)
{
	struct f2fs_super_block *raw_super = (struct f2fs_super_block *)
					(bh->b_data + F2FS_SUPER_OFFSET);
	struct super_block *sb = sbi->sb;
	unsigned int blocksize;

	if (F2FS_SUPER_MAGIC != le32_to_cpu(raw_super->magic)) {
		f2fs_msg(sb, KERN_INFO,
			"Magic Mismatch, valid(0x%x) - read(0x%x)",
			F2FS_SUPER_MAGIC, le32_to_cpu(raw_super->magic));
		return 1;
	}

	/* Currently, support only 4KB page cache size */
	if (F2FS_BLKSIZE != PAGE_SIZE) {
		f2fs_msg(sb, KERN_INFO,
			"Invalid page_cache_size (%lu), supports only 4KB\n",
			PAGE_SIZE);
		return 1;
	}

	/* Currently, support only 4KB block size */
	blocksize = 1 << le32_to_cpu(raw_super->log_blocksize);
	if (blocksize != F2FS_BLKSIZE) {
		f2fs_msg(sb, KERN_INFO,
			"Invalid blocksize (%u), supports only 4KB\n",
			blocksize);
		return 1;
	}

	/* check log blocks per segment */
	if (le32_to_cpu(raw_super->log_blocks_per_seg) != 9) {
		f2fs_msg(sb, KERN_INFO,
			"Invalid log blocks per segment (%u)\n",
			le32_to_cpu(raw_super->log_blocks_per_seg));
		return 1;
	}

	/* Currently, support 512/1024/2048/4096 bytes sector size */
	if (le32_to_cpu(raw_super->log_sectorsize) >
				F2FS_MAX_LOG_SECTOR_SIZE ||
		le32_to_cpu(raw_super->log_sectorsize) <
				F2FS_MIN_LOG_SECTOR_SIZE) {
		f2fs_msg(sb, KERN_INFO, "Invalid log sectorsize (%u)",
			le32_to_cpu(raw_super->log_sectorsize));
		return 1;
	}
	if (le32_to_cpu(raw_super->log_sectors_per_block) +
		le32_to_cpu(raw_super->log_sectorsize) !=
			F2FS_MAX_LOG_SECTOR_SIZE) {
		f2fs_msg(sb, KERN_INFO,
			"Invalid log sectors per block(%u) log sectorsize(%u)",
			le32_to_cpu(raw_super->log_sectors_per_block),
			le32_to_cpu(raw_super->log_sectorsize));
		return 1;
	}

	/* check reserved ino info */
	if (le32_to_cpu(raw_super->node_ino) != 1 ||
		le32_to_cpu(raw_super->meta_ino) != 2 ||
		le32_to_cpu(raw_super->root_ino) != 3) {
		f2fs_msg(sb, KERN_INFO,
			"Invalid Fs Meta Ino: node(%u) meta(%u) root(%u)",
			le32_to_cpu(raw_super->node_ino),
			le32_to_cpu(raw_super->meta_ino),
			le32_to_cpu(raw_super->root_ino));
		return 1;
	}

	if (le32_to_cpu(raw_super->segment_count) > F2FS_MAX_SEGMENT) {
		f2fs_msg(sb, KERN_INFO,
			"Invalid segment count (%u)",
			le32_to_cpu(raw_super->segment_count));
		return 1;
	}

	/* check CP/SIT/NAT/SSA/MAIN_AREA area boundary */
	if (sanity_check_area_boundary(sbi, bh))
		return 1;

	return 0;
}

int sanity_check_ckpt(struct f2fs_sb_info *sbi)
{
	unsigned int total, fsmeta;
	struct f2fs_super_block *raw_super = F2FS_RAW_SUPER(sbi);
	struct f2fs_checkpoint *ckpt = F2FS_CKPT(sbi);
	unsigned int ovp_segments, reserved_segments;
	unsigned int main_segs, blocks_per_seg;
	int i;

	total = le32_to_cpu(raw_super->segment_count);
	fsmeta = le32_to_cpu(raw_super->segment_count_ckpt);
	fsmeta += le32_to_cpu(raw_super->segment_count_sit);
	fsmeta += le32_to_cpu(raw_super->segment_count_nat);
	fsmeta += le32_to_cpu(ckpt->rsvd_segment_count);
	fsmeta += le32_to_cpu(raw_super->segment_count_ssa);

	if (unlikely(fsmeta >= total))
		return 1;

	ovp_segments = le32_to_cpu(ckpt->overprov_segment_count);
	reserved_segments = le32_to_cpu(ckpt->rsvd_segment_count);

	if (unlikely(fsmeta < F2FS_MIN_SEGMENTS ||
			ovp_segments == 0 || reserved_segments == 0)) {
		f2fs_msg(sbi->sb, KERN_ERR,
			"Wrong layout: check mkfs.f2fs version");
		return 1;
	}

	main_segs = le32_to_cpu(raw_super->segment_count_main);
	blocks_per_seg = sbi->blocks_per_seg;

	for (i = 0; i < NR_CURSEG_NODE_TYPE; i++) {
		if (le32_to_cpu(ckpt->cur_node_segno[i]) >= main_segs ||
			le16_to_cpu(ckpt->cur_node_blkoff[i]) >= blocks_per_seg)
			return 1;
	}
	for (i = 0; i < NR_CURSEG_DATA_TYPE; i++) {
		if (le32_to_cpu(ckpt->cur_data_segno[i]) >= main_segs ||
			le16_to_cpu(ckpt->cur_data_blkoff[i]) >= blocks_per_seg)
			return 1;
	}

	if (unlikely(f2fs_cp_error(sbi))) {
		f2fs_msg(sbi->sb, KERN_ERR, "A bug case: need to run fsck");
		return 1;
	}
	return 0;
}

static void init_sb_info(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *raw_super = sbi->raw_super;
	int i, j;

	sbi->log_sectors_per_block =
		le32_to_cpu(raw_super->log_sectors_per_block);
	sbi->log_blocksize = le32_to_cpu(raw_super->log_blocksize);
	sbi->blocksize = 1 << sbi->log_blocksize;
	sbi->log_blocks_per_seg = le32_to_cpu(raw_super->log_blocks_per_seg);
	sbi->blocks_per_seg = 1 << sbi->log_blocks_per_seg;
	sbi->segs_per_sec = le32_to_cpu(raw_super->segs_per_sec);
	sbi->secs_per_zone = le32_to_cpu(raw_super->secs_per_zone);
	sbi->total_sections = le32_to_cpu(raw_super->section_count);
	sbi->total_node_count =
		(le32_to_cpu(raw_super->segment_count_nat) / 2)
			* sbi->blocks_per_seg * NAT_ENTRY_PER_BLOCK;
	sbi->root_ino_num = le32_to_cpu(raw_super->root_ino);
	sbi->node_ino_num = le32_to_cpu(raw_super->node_ino);
	sbi->meta_ino_num = le32_to_cpu(raw_super->meta_ino);
	sbi->cur_victim_sec = NULL_SECNO;
	sbi->max_victim_search = DEF_MAX_VICTIM_SEARCH;

	sbi->dir_level = DEF_DIR_LEVEL;
	sbi->interval_time[CP_TIME] = DEF_CP_INTERVAL;
	sbi->interval_time[REQ_TIME] = DEF_IDLE_INTERVAL;
	clear_sbi_flag(sbi, SBI_NEED_FSCK);

	for (i = 0; i < NR_COUNT_TYPE; i++)
		atomic_set(&sbi->nr_pages[i], 0);

	atomic_set(&sbi->wb_sync_req, 0);

	INIT_LIST_HEAD(&sbi->s_list);
	mutex_init(&sbi->umount_mutex);
	for (i = 0; i < NR_PAGE_TYPE - 1; i++)
		for (j = HOT; j < NR_TEMP_TYPE; j++)
			mutex_init(&sbi->wio_mutex[i][j]);
	spin_lock_init(&sbi->cp_lock);

	sbi->dirty_device = 0;
	spin_lock_init(&sbi->dev_lock);

	init_rwsem(&sbi->sb_lock);
}

static int init_percpu_info(struct f2fs_sb_info *sbi)
{
	int err;

	err = percpu_counter_init(&sbi->alloc_valid_block_count, 0, GFP_KERNEL);
	if (err)
		return err;

	return percpu_counter_init(&sbi->total_valid_inode_count, 0,
								GFP_KERNEL);
}

#ifdef CONFIG_BLK_DEV_ZONED
static int init_blkz_info(struct f2fs_sb_info *sbi, int devi)
{
	struct block_device *bdev = FDEV(devi).bdev;
	sector_t nr_sectors = bdev->bd_part->nr_sects;
	sector_t sector = 0;
	struct blk_zone *zones;
	unsigned int i, nr_zones;
	unsigned int n = 0;
	int err = -EIO;

	if (!f2fs_sb_has_blkzoned(sbi->sb))
		return 0;

	if (sbi->blocks_per_blkz && sbi->blocks_per_blkz !=
				SECTOR_TO_BLOCK(bdev_zone_sectors(bdev)))
		return -EINVAL;
	sbi->blocks_per_blkz = SECTOR_TO_BLOCK(bdev_zone_sectors(bdev));
	if (sbi->log_blocks_per_blkz && sbi->log_blocks_per_blkz !=
				__ilog2_u32(sbi->blocks_per_blkz))
		return -EINVAL;
	sbi->log_blocks_per_blkz = __ilog2_u32(sbi->blocks_per_blkz);
	FDEV(devi).nr_blkz = SECTOR_TO_BLOCK(nr_sectors) >>
					sbi->log_blocks_per_blkz;
	if (nr_sectors & (bdev_zone_sectors(bdev) - 1))
		FDEV(devi).nr_blkz++;

	FDEV(devi).blkz_type = f2fs_kmalloc(sbi, FDEV(devi).nr_blkz,
								GFP_KERNEL);
	if (!FDEV(devi).blkz_type)
		return -ENOMEM;

#define F2FS_REPORT_NR_ZONES   4096

	zones = f2fs_kzalloc(sbi, sizeof(struct blk_zone) *
				F2FS_REPORT_NR_ZONES, GFP_KERNEL);
	if (!zones)
		return -ENOMEM;

	/* Get block zones type */
	while (zones && sector < nr_sectors) {

		nr_zones = F2FS_REPORT_NR_ZONES;
		err = blkdev_report_zones(bdev, sector,
					  zones, &nr_zones,
					  GFP_KERNEL);
		if (err)
			break;
		if (!nr_zones) {
			err = -EIO;
			break;
		}

		for (i = 0; i < nr_zones; i++) {
			FDEV(devi).blkz_type[n] = zones[i].type;
			sector += zones[i].len;
			n++;
		}
	}

	kfree(zones);

	return err;
}
#endif

/*
 * Read f2fs raw super block.
 * Because we have two copies of super block, so read both of them
 * to get the first valid one. If any one of them is broken, we pass
 * them recovery flag back to the caller.
 */
static int read_raw_super_block(struct f2fs_sb_info *sbi,
			struct f2fs_super_block **raw_super,
			int *valid_super_block, int *recovery)
{
	struct super_block *sb = sbi->sb;
	int block;
	struct buffer_head *bh;
	struct f2fs_super_block *super;
	int err = 0;

	super = kzalloc(sizeof(struct f2fs_super_block), GFP_KERNEL);
	if (!super)
		return -ENOMEM;

	for (block = 0; block < 2; block++) {
		bh = sb_bread(sb, block);
		if (!bh) {
			f2fs_msg(sb, KERN_ERR, "Unable to read %dth superblock",
				block + 1);
			err = -EIO;
			continue;
		}

		/* sanity checking of raw super */
		if (sanity_check_raw_super(sbi, bh)) {
			f2fs_msg(sb, KERN_ERR,
				"Can't find valid F2FS filesystem in %dth superblock",
				block + 1);
			err = -EINVAL;
			brelse(bh);
			continue;
		}

		if (!*raw_super) {
			memcpy(super, bh->b_data + F2FS_SUPER_OFFSET,
							sizeof(*super));
			*valid_super_block = block;
			*raw_super = super;
		}
		brelse(bh);
	}

	/* Fail to read any one of the superblocks*/
	if (err < 0)
		*recovery = 1;

	/* No valid superblock */
	if (!*raw_super)
		kfree(super);
	else
		err = 0;

	return err;
}

int f2fs_commit_super(struct f2fs_sb_info *sbi, bool recover)
{
	struct buffer_head *bh;
	int err;

	if ((recover && f2fs_readonly(sbi->sb)) ||
				bdev_read_only(sbi->sb->s_bdev)) {
		set_sbi_flag(sbi, SBI_NEED_SB_WRITE);
		return -EROFS;
	}

	/* write back-up superblock first */
	bh = sb_bread(sbi->sb, sbi->valid_super_block ? 0 : 1);
	if (!bh)
		return -EIO;
	err = __f2fs_commit_super(bh, F2FS_RAW_SUPER(sbi));
	brelse(bh);

	/* if we are in recovery path, skip writing valid superblock */
	if (recover || err)
		return err;

	/* write current valid superblock */
	bh = sb_bread(sbi->sb, sbi->valid_super_block);
	if (!bh)
		return -EIO;
	err = __f2fs_commit_super(bh, F2FS_RAW_SUPER(sbi));
	brelse(bh);
	return err;
}

static int f2fs_scan_devices(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *raw_super = F2FS_RAW_SUPER(sbi);
	unsigned int max_devices = MAX_DEVICES;
	int i;

	/* Initialize single device information */
	if (!RDEV(0).path[0]) {
#ifdef CONFIG_BLK_DEV_ZONED
		if (!bdev_is_zoned(sbi->sb->s_bdev))
			return 0;
		max_devices = 1;
#else
		return 0;
#endif
	}

	/*
	 * Initialize multiple devices information, or single
	 * zoned block device information.
	 */
	sbi->devs = f2fs_kzalloc(sbi, sizeof(struct f2fs_dev_info) *
						max_devices, GFP_KERNEL);
	if (!sbi->devs)
		return -ENOMEM;

	for (i = 0; i < max_devices; i++) {

		if (i > 0 && !RDEV(i).path[0])
			break;

		if (max_devices == 1) {
			/* Single zoned block device mount */
			FDEV(0).bdev =
				blkdev_get_by_dev(sbi->sb->s_bdev->bd_dev,
					sbi->sb->s_mode, sbi->sb->s_type);
		} else {
			/* Multi-device mount */
			memcpy(FDEV(i).path, RDEV(i).path, MAX_PATH_LEN);
			FDEV(i).total_segments =
				le32_to_cpu(RDEV(i).total_segments);
			if (i == 0) {
				FDEV(i).start_blk = 0;
				FDEV(i).end_blk = FDEV(i).start_blk +
				    (FDEV(i).total_segments <<
				    sbi->log_blocks_per_seg) - 1 +
				    le32_to_cpu(raw_super->segment0_blkaddr);
			} else {
				FDEV(i).start_blk = FDEV(i - 1).end_blk + 1;
				FDEV(i).end_blk = FDEV(i).start_blk +
					(FDEV(i).total_segments <<
					sbi->log_blocks_per_seg) - 1;
			}
			FDEV(i).bdev = blkdev_get_by_path(FDEV(i).path,
					sbi->sb->s_mode, sbi->sb->s_type);
		}
		if (IS_ERR(FDEV(i).bdev))
			return PTR_ERR(FDEV(i).bdev);

		/* to release errored devices */
		sbi->s_ndevs = i + 1;

#ifdef CONFIG_BLK_DEV_ZONED
		if (bdev_zoned_model(FDEV(i).bdev) == BLK_ZONED_HM &&
				!f2fs_sb_has_blkzoned(sbi->sb)) {
			f2fs_msg(sbi->sb, KERN_ERR,
				"Zoned block device feature not enabled\n");
			return -EINVAL;
		}
		if (bdev_zoned_model(FDEV(i).bdev) != BLK_ZONED_NONE) {
			if (init_blkz_info(sbi, i)) {
				f2fs_msg(sbi->sb, KERN_ERR,
					"Failed to initialize F2FS blkzone information");
				return -EINVAL;
			}
			if (max_devices == 1)
				break;
			f2fs_msg(sbi->sb, KERN_INFO,
				"Mount Device [%2d]: %20s, %8u, %8x - %8x (zone: %s)",
				i, FDEV(i).path,
				FDEV(i).total_segments,
				FDEV(i).start_blk, FDEV(i).end_blk,
				bdev_zoned_model(FDEV(i).bdev) == BLK_ZONED_HA ?
				"Host-aware" : "Host-managed");
			continue;
		}
#endif
		f2fs_msg(sbi->sb, KERN_INFO,
			"Mount Device [%2d]: %20s, %8u, %8x - %8x",
				i, FDEV(i).path,
				FDEV(i).total_segments,
				FDEV(i).start_blk, FDEV(i).end_blk);
	}
	f2fs_msg(sbi->sb, KERN_INFO,
			"IO Block Size: %8d KB", F2FS_IO_SIZE_KB(sbi));
	return 0;
}

static void f2fs_tuning_parameters(struct f2fs_sb_info *sbi)
{
	struct f2fs_sm_info *sm_i = SM_I(sbi);

	/* adjust parameters according to the volume size */
	if (sm_i->main_segments <= SMALL_VOLUME_SEGMENTS) {
		F2FS_OPTION(sbi).alloc_mode = ALLOC_MODE_REUSE;
		sm_i->dcc_info->discard_granularity = 1;
		sm_i->ipu_policy = 1 << F2FS_IPU_FORCE;
	}
}

static int f2fs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct f2fs_sb_info *sbi;
	struct f2fs_super_block *raw_super;
	struct inode *root;
	int err;
	bool retry = true, need_fsck = false;
	char *options = NULL;
	int recovery, i, valid_super_block;
	struct curseg_info *seg_i;

try_onemore:
	err = -EINVAL;
	raw_super = NULL;
	valid_super_block = -1;
	recovery = 0;

	/* allocate memory for f2fs-specific super block info */
	sbi = kzalloc(sizeof(struct f2fs_sb_info), GFP_KERNEL);
	if (!sbi)
		return -ENOMEM;

	sbi->sb = sb;

	/* Load the checksum driver */
	sbi->s_chksum_driver = crypto_alloc_shash("crc32", 0, 0);
	if (IS_ERR(sbi->s_chksum_driver)) {
		f2fs_msg(sb, KERN_ERR, "Cannot load crc32 driver.");
		err = PTR_ERR(sbi->s_chksum_driver);
		sbi->s_chksum_driver = NULL;
		goto free_sbi;
	}

	/* set a block size */
	if (unlikely(!sb_set_blocksize(sb, F2FS_BLKSIZE))) {
		f2fs_msg(sb, KERN_ERR, "unable to set blocksize");
		goto free_sbi;
	}

	err = read_raw_super_block(sbi, &raw_super, &valid_super_block,
								&recovery);
	if (err)
		goto free_sbi;

	sb->s_fs_info = sbi;
	sbi->raw_super = raw_super;

	F2FS_OPTION(sbi).s_resuid = make_kuid(&init_user_ns, F2FS_DEF_RESUID);
	F2FS_OPTION(sbi).s_resgid = make_kgid(&init_user_ns, F2FS_DEF_RESGID);

	/* precompute checksum seed for metadata */
	if (f2fs_sb_has_inode_chksum(sb))
		sbi->s_chksum_seed = f2fs_chksum(sbi, ~0, raw_super->uuid,
						sizeof(raw_super->uuid));

	/*
	 * The BLKZONED feature indicates that the drive was formatted with
	 * zone alignment optimization. This is optional for host-aware
	 * devices, but mandatory for host-managed zoned block devices.
	 */
#ifndef CONFIG_BLK_DEV_ZONED
	if (f2fs_sb_has_blkzoned(sb)) {
		f2fs_msg(sb, KERN_ERR,
			 "Zoned block device support is not enabled\n");
		err = -EOPNOTSUPP;
		goto free_sb_buf;
	}
#endif
	default_options(sbi);
	/* parse mount options */
	options = kstrdup((const char *)data, GFP_KERNEL);
	if (data && !options) {
		err = -ENOMEM;
		goto free_sb_buf;
	}

	err = parse_options(sb, options);
	if (err)
		goto free_options;

	sbi->max_file_blocks = max_file_blocks();
	sb->s_maxbytes = sbi->max_file_blocks <<
				le32_to_cpu(raw_super->log_blocksize);
	sb->s_max_links = F2FS_LINK_MAX;
	get_random_bytes(&sbi->s_next_generation, sizeof(u32));

#ifdef CONFIG_QUOTA
	sb->dq_op = &f2fs_quota_operations;
	if (f2fs_sb_has_quota_ino(sb))
		sb->s_qcop = &dquot_quotactl_sysfile_ops;
	else
		sb->s_qcop = &f2fs_quotactl_ops;
	sb->s_quota_types = QTYPE_MASK_USR | QTYPE_MASK_GRP | QTYPE_MASK_PRJ;

	if (f2fs_sb_has_quota_ino(sbi->sb)) {
		for (i = 0; i < MAXQUOTAS; i++) {
			if (f2fs_qf_ino(sbi->sb, i))
				sbi->nquota_files++;
		}
	}
#endif

	sb->s_op = &f2fs_sops;
#ifdef CONFIG_F2FS_FS_ENCRYPTION
	sb->s_cop = &f2fs_cryptops;
#endif
	sb->s_xattr = f2fs_xattr_handlers;
	sb->s_export_op = &f2fs_export_ops;
	sb->s_magic = F2FS_SUPER_MAGIC;
	sb->s_time_gran = 1;
	sb->s_flags = (sb->s_flags & ~MS_POSIXACL) |
		(test_opt(sbi, POSIX_ACL) ? MS_POSIXACL : 0);
	memcpy(&sb->s_uuid, raw_super->uuid, sizeof(raw_super->uuid));
	sb->s_iflags |= SB_I_CGROUPWB;

	/* init f2fs-specific super block info */
	sbi->valid_super_block = valid_super_block;
	mutex_init(&sbi->gc_mutex);
	mutex_init(&sbi->cp_mutex);
	init_rwsem(&sbi->node_write);
	init_rwsem(&sbi->node_change);

	/* disallow all the data/node/meta page writes */
	set_sbi_flag(sbi, SBI_POR_DOING);
	spin_lock_init(&sbi->stat_lock);

	/* init iostat info */
	spin_lock_init(&sbi->iostat_lock);
	sbi->iostat_enable = false;

	for (i = 0; i < NR_PAGE_TYPE; i++) {
		int n = (i == META) ? 1: NR_TEMP_TYPE;
		int j;

		sbi->write_io[i] = f2fs_kmalloc(sbi,
					n * sizeof(struct f2fs_bio_info),
					GFP_KERNEL);
		if (!sbi->write_io[i]) {
			err = -ENOMEM;
			goto free_options;
		}

		for (j = HOT; j < n; j++) {
			init_rwsem(&sbi->write_io[i][j].io_rwsem);
			sbi->write_io[i][j].sbi = sbi;
			sbi->write_io[i][j].bio = NULL;
			spin_lock_init(&sbi->write_io[i][j].io_lock);
			INIT_LIST_HEAD(&sbi->write_io[i][j].io_list);
		}
	}

	init_rwsem(&sbi->cp_rwsem);
	init_waitqueue_head(&sbi->cp_wait);
	init_sb_info(sbi);

	err = init_percpu_info(sbi);
	if (err)
		goto free_bio_info;

	if (F2FS_IO_SIZE(sbi) > 1) {
		sbi->write_io_dummy =
			mempool_create_page_pool(2 * (F2FS_IO_SIZE(sbi) - 1), 0);
		if (!sbi->write_io_dummy) {
			err = -ENOMEM;
			goto free_percpu;
		}
	}

	/* get an inode for meta space */
	sbi->meta_inode = f2fs_iget(sb, F2FS_META_INO(sbi));
	if (IS_ERR(sbi->meta_inode)) {
		f2fs_msg(sb, KERN_ERR, "Failed to read F2FS meta data inode");
		err = PTR_ERR(sbi->meta_inode);
		goto free_io_dummy;
	}

	err = get_valid_checkpoint(sbi);
	if (err) {
		f2fs_msg(sb, KERN_ERR, "Failed to get valid F2FS checkpoint");
		goto free_meta_inode;
	}

	/* Initialize device list */
	err = f2fs_scan_devices(sbi);
	if (err) {
		f2fs_msg(sb, KERN_ERR, "Failed to find devices");
		goto free_devices;
	}

	sbi->total_valid_node_count =
				le32_to_cpu(sbi->ckpt->valid_node_count);
	percpu_counter_set(&sbi->total_valid_inode_count,
				le32_to_cpu(sbi->ckpt->valid_inode_count));
	sbi->user_block_count = le64_to_cpu(sbi->ckpt->user_block_count);
	sbi->total_valid_block_count =
				le64_to_cpu(sbi->ckpt->valid_block_count);
	sbi->last_valid_block_count = sbi->total_valid_block_count;
	sbi->reserved_blocks = 0;
	sbi->current_reserved_blocks = 0;
	limit_reserve_root(sbi);

	for (i = 0; i < NR_INODE_TYPE; i++) {
		INIT_LIST_HEAD(&sbi->inode_list[i]);
		spin_lock_init(&sbi->inode_lock[i]);
	}

	init_extent_cache_info(sbi);

	init_ino_entry_info(sbi);

	/* setup f2fs internal modules */
	err = build_segment_manager(sbi);
	if (err) {
		f2fs_msg(sb, KERN_ERR,
			"Failed to initialize F2FS segment manager");
		goto free_sm;
	}
	err = build_node_manager(sbi);
	if (err) {
		f2fs_msg(sb, KERN_ERR,
			"Failed to initialize F2FS node manager");
		goto free_nm;
	}

	/* For write statistics */
	if (sb->s_bdev->bd_part)
		sbi->sectors_written_start =
			(u64)part_stat_read(sb->s_bdev->bd_part, sectors[1]);

	/* Read accumulated write IO statistics if exists */
	seg_i = CURSEG_I(sbi, CURSEG_HOT_NODE);
	if (__exist_node_summaries(sbi))
		sbi->kbytes_written =
			le64_to_cpu(seg_i->journal->info.kbytes_written);

	build_gc_manager(sbi);

	/* get an inode for node space */
	sbi->node_inode = f2fs_iget(sb, F2FS_NODE_INO(sbi));
	if (IS_ERR(sbi->node_inode)) {
		f2fs_msg(sb, KERN_ERR, "Failed to read node inode");
		err = PTR_ERR(sbi->node_inode);
		goto free_nm;
	}

	err = f2fs_build_stats(sbi);
	if (err)
		goto free_node_inode;

	/* read root inode and dentry */
	root = f2fs_iget(sb, F2FS_ROOT_INO(sbi));
	if (IS_ERR(root)) {
		f2fs_msg(sb, KERN_ERR, "Failed to read root inode");
		err = PTR_ERR(root);
		goto free_stats;
	}
	if (!S_ISDIR(root->i_mode) || !root->i_blocks || !root->i_size) {
		iput(root);
		err = -EINVAL;
		goto free_node_inode;
	}

	sb->s_root = d_make_root(root); /* allocate root dentry */
	if (!sb->s_root) {
		err = -ENOMEM;
		goto free_root_inode;
	}

	err = f2fs_register_sysfs(sbi);
	if (err)
		goto free_root_inode;

#ifdef CONFIG_QUOTA
	/*
	 * Turn on quotas which were not enabled for read-only mounts if
	 * filesystem has quota feature, so that they are updated correctly.
	 */
	if (f2fs_sb_has_quota_ino(sb) && !f2fs_readonly(sb)) {
		err = f2fs_enable_quotas(sb);
		if (err) {
			f2fs_msg(sb, KERN_ERR,
				"Cannot turn on quotas: error %d", err);
			goto free_sysfs;
		}
	}
#endif
	/* if there are nt orphan nodes free them */
	err = recover_orphan_inodes(sbi);
	if (err)
		goto free_meta;

	/* recover fsynced data */
	if (!test_opt(sbi, DISABLE_ROLL_FORWARD)) {
		/*
		 * mount should be failed, when device has readonly mode, and
		 * previous checkpoint was not done by clean system shutdown.
		 */
		if (bdev_read_only(sb->s_bdev) &&
				!is_set_ckpt_flags(sbi, CP_UMOUNT_FLAG)) {
			err = -EROFS;
			goto free_meta;
		}

		if (need_fsck)
			set_sbi_flag(sbi, SBI_NEED_FSCK);

		if (!retry)
			goto skip_recovery;

		err = recover_fsync_data(sbi, false);
		if (err < 0) {
			need_fsck = true;
			f2fs_msg(sb, KERN_ERR,
				"Cannot recover all fsync data errno=%d", err);
			goto free_meta;
		}
	} else {
		err = recover_fsync_data(sbi, true);

		if (!f2fs_readonly(sb) && err > 0) {
			err = -EINVAL;
			f2fs_msg(sb, KERN_ERR,
				"Need to recover fsync data");
			goto free_meta;
		}
	}
skip_recovery:
	/* recover_fsync_data() cleared this already */
	clear_sbi_flag(sbi, SBI_POR_DOING);

	/*
	 * If filesystem is not mounted as read-only then
	 * do start the gc_thread.
	 */
	if (test_opt(sbi, BG_GC) && !f2fs_readonly(sb)) {
		/* After POR, we can run background GC thread.*/
		err = start_gc_thread(sbi);
		if (err)
			goto free_meta;
	}
	kfree(options);

	/* recover broken superblock */
	if (recovery) {
		err = f2fs_commit_super(sbi, true);
		f2fs_msg(sb, KERN_INFO,
			"Try to recover %dth superblock, ret: %d",
			sbi->valid_super_block ? 1 : 2, err);
	}

	f2fs_join_shrinker(sbi);

	f2fs_tuning_parameters(sbi);

	f2fs_msg(sbi->sb, KERN_NOTICE, "Mounted with checkpoint version = %llx",
				cur_cp_version(F2FS_CKPT(sbi)));
	f2fs_update_time(sbi, CP_TIME);
	f2fs_update_time(sbi, REQ_TIME);
	return 0;

free_meta:
#ifdef CONFIG_QUOTA
	if (f2fs_sb_has_quota_ino(sb) && !f2fs_readonly(sb))
		f2fs_quota_off_umount(sbi->sb);
#endif
	f2fs_sync_inode_meta(sbi);
	/*
	 * Some dirty meta pages can be produced by recover_orphan_inodes()
	 * failed by EIO. Then, iput(node_inode) can trigger balance_fs_bg()
	 * followed by write_checkpoint() through f2fs_write_node_pages(), which
	 * falls into an infinite loop in sync_meta_pages().
	 */
	truncate_inode_pages_final(META_MAPPING(sbi));
#ifdef CONFIG_QUOTA
free_sysfs:
#endif
	f2fs_unregister_sysfs(sbi);
free_root_inode:
	dput(sb->s_root);
	sb->s_root = NULL;
free_stats:
	f2fs_destroy_stats(sbi);
free_node_inode:
	release_ino_entry(sbi, true);
	truncate_inode_pages_final(NODE_MAPPING(sbi));
	iput(sbi->node_inode);
free_nm:
	destroy_node_manager(sbi);
free_sm:
	destroy_segment_manager(sbi);
free_devices:
	destroy_device_list(sbi);
	kfree(sbi->ckpt);
free_meta_inode:
	make_bad_inode(sbi->meta_inode);
	iput(sbi->meta_inode);
free_io_dummy:
	mempool_destroy(sbi->write_io_dummy);
free_percpu:
	destroy_percpu_info(sbi);
free_bio_info:
	for (i = 0; i < NR_PAGE_TYPE; i++)
		kfree(sbi->write_io[i]);
free_options:
#ifdef CONFIG_QUOTA
	for (i = 0; i < MAXQUOTAS; i++)
		kfree(F2FS_OPTION(sbi).s_qf_names[i]);
#endif
	kfree(options);
free_sb_buf:
	kfree(raw_super);
free_sbi:
	if (sbi->s_chksum_driver)
		crypto_free_shash(sbi->s_chksum_driver);
	kfree(sbi);

	/* give only one another chance */
	if (retry) {
		retry = false;
		shrink_dcache_sb(sb);
		goto try_onemore;
	}
	return err;
}

static struct dentry *f2fs_mount(struct file_system_type *fs_type, int flags,
			const char *dev_name, void *data)
{
	return mount_bdev(fs_type, flags, dev_name, data, f2fs_fill_super);
}

static void kill_f2fs_super(struct super_block *sb)
{
	if (sb->s_root) {
		set_sbi_flag(F2FS_SB(sb), SBI_IS_CLOSE);
		stop_gc_thread(F2FS_SB(sb));
		stop_discard_thread(F2FS_SB(sb));
	}
	kill_block_super(sb);
}

static struct file_system_type f2fs_fs_type = {
	.owner		= THIS_MODULE,
	.name		= "f2fs",
	.mount		= f2fs_mount,
	.kill_sb	= kill_f2fs_super,
	.fs_flags	= FS_REQUIRES_DEV,
};
MODULE_ALIAS_FS("f2fs");

static int __init init_inodecache(void)
{
	f2fs_inode_cachep = kmem_cache_create("f2fs_inode_cache",
			sizeof(struct f2fs_inode_info), 0,
			SLAB_RECLAIM_ACCOUNT|SLAB_ACCOUNT, NULL);
	if (!f2fs_inode_cachep)
		return -ENOMEM;
	return 0;
}

static void destroy_inodecache(void)
{
	/*
	 * Make sure all delayed rcu free inodes are flushed before we
	 * destroy cache.
	 */
	rcu_barrier();
	kmem_cache_destroy(f2fs_inode_cachep);
}

static int __init init_f2fs_fs(void)
{
	int err;

	f2fs_build_trace_ios();

	err = init_inodecache();
	if (err)
		goto fail;
	err = create_node_manager_caches();
	if (err)
		goto free_inodecache;
	err = create_segment_manager_caches();
	if (err)
		goto free_node_manager_caches;
	err = create_checkpoint_caches();
	if (err)
		goto free_segment_manager_caches;
	err = create_extent_cache();
	if (err)
		goto free_checkpoint_caches;
	err = f2fs_init_sysfs();
	if (err)
		goto free_extent_cache;
	err = register_shrinker(&f2fs_shrinker_info);
	if (err)
		goto free_sysfs;
	err = register_filesystem(&f2fs_fs_type);
	if (err)
		goto free_shrinker;
	err = f2fs_create_root_stats();
	if (err)
		goto free_filesystem;
	err = f2fs_init_post_read_processing();
	if (err)
		goto free_root_stats;
	return 0;

free_root_stats:
	f2fs_destroy_root_stats();
free_filesystem:
	unregister_filesystem(&f2fs_fs_type);
free_shrinker:
	unregister_shrinker(&f2fs_shrinker_info);
free_sysfs:
	f2fs_exit_sysfs();
free_extent_cache:
	destroy_extent_cache();
free_checkpoint_caches:
	destroy_checkpoint_caches();
free_segment_manager_caches:
	destroy_segment_manager_caches();
free_node_manager_caches:
	destroy_node_manager_caches();
free_inodecache:
	destroy_inodecache();
fail:
	return err;
}

static void __exit exit_f2fs_fs(void)
{
	f2fs_destroy_post_read_processing();
	f2fs_destroy_root_stats();
	unregister_filesystem(&f2fs_fs_type);
	unregister_shrinker(&f2fs_shrinker_info);
	f2fs_exit_sysfs();
	destroy_extent_cache();
	destroy_checkpoint_caches();
	destroy_segment_manager_caches();
	destroy_node_manager_caches();
	destroy_inodecache();
	f2fs_destroy_trace_ios();
}

module_init(init_f2fs_fs)
module_exit(exit_f2fs_fs)

MODULE_AUTHOR("Samsung Electronics's Praesto Team");
MODULE_DESCRIPTION("Flash Friendly File System");
MODULE_LICENSE("GPL");

