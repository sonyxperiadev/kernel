/* include/linux/android_pmem.h
 *
 * Copyright (C) 2007 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _ANDROID_PMEM_H_
#define _ANDROID_PMEM_H_

#define PMEM_IOCTL_MAGIC 'p'
#define PMEM_GET_PHYS		_IOW(PMEM_IOCTL_MAGIC, 1, unsigned int)
#define PMEM_MAP		_IOW(PMEM_IOCTL_MAGIC, 2, unsigned int)
#define PMEM_GET_SIZE		_IOW(PMEM_IOCTL_MAGIC, 3, unsigned int)
#define PMEM_UNMAP		_IOW(PMEM_IOCTL_MAGIC, 4, unsigned int)
/* This ioctl will allocate pmem space, backing the file, it will fail
 * if the file already has an allocation, pass it the len as the argument
 * to the ioctl */
#define PMEM_ALLOCATE		_IOW(PMEM_IOCTL_MAGIC, 5, unsigned int)
/* This will connect a one pmem file to another, pass the file that is already
 * backed in memory as the argument to the ioctl
 */
#define PMEM_CONNECT		_IOW(PMEM_IOCTL_MAGIC, 6, unsigned int)
/* Returns the total size of the pmem region it is sent to as a pmem_region
 * struct (with offset set to 0). 
 */
#define PMEM_GET_TOTAL_SIZE	_IOW(PMEM_IOCTL_MAGIC, 7, unsigned int)
#define PMEM_CACHE_FLUSH	_IOW(PMEM_IOCTL_MAGIC, 8, unsigned int)
#define PMEM_CACHE_INVALIDATE	_IOW(PMEM_IOCTL_MAGIC, 9, unsigned int)
#define PMEM_CLEANER_WAIT	_IO(PMEM_IOCTL_MAGIC, 10)

struct android_pmem_platform_data
{
	const char* name;
	/* size of memory region */
	unsigned long cmasize;
	/* starting physical address of memory region */
	phys_addr_t carveout_base;
	phys_addr_t carveout_size;
};

struct pmem_region {
	unsigned long offset;
	unsigned long len;
};

#ifdef CONFIG_ANDROID_PMEM
int is_pmem_file(struct file *file);
int get_pmem_file(int fd, unsigned long *start, unsigned long *vstart,
		  unsigned long *end, struct file **filp);
int get_pmem_user_addr(struct file *file, unsigned long *start,
		       unsigned long *end);
void put_pmem_file(struct file* file);
void flush_pmem_file(struct file *file, unsigned long start, unsigned long len);
int pmem_setup(struct platform_device *pdev,
		struct android_pmem_platform_data *pdata);
int pmem_remap(struct pmem_region *region, struct file *file,
	       unsigned operation);

#else
static inline int is_pmem_file(struct file *file) { return 0; }
static inline int get_pmem_file(int fd, unsigned long *start,
				unsigned long *vstart, unsigned long *end,
				struct file **filp) { return -ENOSYS; }
static inline int get_pmem_user_addr(struct file *file, unsigned long *start,
				     unsigned long *end) { return -ENOSYS; }
static inline void put_pmem_file(struct file* file) { return; }
static inline void flush_pmem_file(struct file *file, unsigned long start,
				   unsigned long len) { return; }
int pmem_setup(struct platform_device *pdev,
		struct android_pmem_platform_data *pdata) { return -ENOSYS; }

static inline int pmem_remap(struct pmem_region *region, struct file *file,
			     unsigned operation) { return -ENOSYS; }
#endif

#endif //_ANDROID_PPP_H_

