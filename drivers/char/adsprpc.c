/*
 * Copyright (c) 2012-2018, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/dma-buf.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/completion.h>
#include <linux/pagemap.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/list.h>
#include <linux/hash.h>
#include <linux/msm_ion.h>
#include <soc/qcom/secure_buffer.h>
#include <soc/qcom/glink.h>
#include <soc/qcom/smd.h>
#include <soc/qcom/subsystem_notif.h>
#include <soc/qcom/subsystem_restart.h>
#include <soc/qcom/service-notifier.h>
#include <soc/qcom/service-locator.h>
#include <linux/scatterlist.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/dma-contiguous.h>
#include <linux/cma.h>
#include <linux/iommu.h>
#include <linux/kref.h>
#include <linux/sort.h>
#include <linux/msm_dma_iommu_mapping.h>
#include <asm/dma-iommu.h>
#include <soc/qcom/scm.h>
#include "adsprpc_compat.h"
#include "adsprpc_shared.h"
#include <soc/qcom/ramdump.h>
#include <linux/debugfs.h>
#include <linux/pm_qos.h>
#define TZ_PIL_PROTECT_MEM_SUBSYS_ID 0x0C
#define TZ_PIL_CLEAR_PROTECT_MEM_SUBSYS_ID 0x0D
#define TZ_PIL_AUTH_QDSP6_PROC 1
#define ADSP_MMAP_HEAP_ADDR 4
#define ADSP_MMAP_REMOTE_HEAP_ADDR 8
#define FASTRPC_DMAHANDLE_NOMAP (16)

#define FASTRPC_ENOSUCH 39
#define VMID_SSC_Q6     5
#define VMID_ADSP_Q6    6
#define DEBUGFS_SIZE 1024

#define AUDIO_PDR_SERVICE_LOCATION_CLIENT_NAME   "audio_pdr_adsprpc"
#define AUDIO_PDR_ADSP_SERVICE_NAME              "avs/audio"

#define SENSORS_PDR_SERVICE_LOCATION_CLIENT_NAME   "sensors_pdr_adsprpc"
#define SENSORS_PDR_ADSP_SERVICE_NAME              "tms/servreg"

#define RPC_TIMEOUT	(5 * HZ)
#define BALIGN		128
#define NUM_CHANNELS	4	/* adsp, mdsp, slpi, cdsp*/
#define NUM_SESSIONS	9	/*8 compute, 1 cpz*/
#define M_FDLIST	(16)
#define M_CRCLIST	(64)
#define SESSION_ID_INDEX (30)
#define FASTRPC_CTX_MAGIC (0xbeeddeed)
#define FASTRPC_CTX_MAX (256)
#define FASTRPC_CTXID_MASK (0xFF0)

#define IS_CACHE_ALIGNED(x) (((x) & ((L1_CACHE_BYTES)-1)) == 0)

#define FASTRPC_LINK_STATE_DOWN   (0x0)
#define FASTRPC_LINK_STATE_UP     (0x1)
#define FASTRPC_LINK_DISCONNECTED (0x0)
#define FASTRPC_LINK_CONNECTING   (0x1)
#define FASTRPC_LINK_CONNECTED    (0x3)
#define FASTRPC_LINK_DISCONNECTING (0x7)
#define FASTRPC_LINK_REMOTE_DISCONNECTING (0x8)
#define FASTRPC_GLINK_INTENT_LEN  (64)

#define PERF_KEYS \
	"count:flush:map:copy:glink:getargs:putargs:invalidate:invoke:tid:ptr"
#define FASTRPC_STATIC_HANDLE_LISTENER (3)
#define FASTRPC_STATIC_HANDLE_MAX (20)
#define FASTRPC_LATENCY_CTRL_ENB  (1)

#define INIT_FILELEN_MAX (2*1024*1024)
#define INIT_MEMLEN_MAX  (8*1024*1024)

#define PERF_END (void)0

#define PERF(enb, cnt, ff) \
	{\
		struct timespec startT = {0};\
		int64_t *counter = cnt;\
		if (enb && counter) {\
			getnstimeofday(&startT);\
		} \
		ff ;\
		if (enb && counter) {\
			*counter += getnstimediff(&startT);\
		} \
	}

#define GET_COUNTER(perf_ptr, offset)  \
	(perf_ptr != NULL ?\
		(((offset >= 0) && (offset < PERF_KEY_MAX)) ?\
			(int64_t *)(perf_ptr + offset)\
				: (int64_t *)NULL) : (int64_t *)NULL)

static int fastrpc_glink_open(int cid);
static void fastrpc_glink_close(void *chan, int cid);
static int fastrpc_pdr_notifier_cb(struct notifier_block *nb,
					unsigned long code,
					void *data);
static struct dentry *debugfs_root;
static struct dentry *debugfs_global_file;

static inline uint64_t buf_page_start(uint64_t buf)
{
	uint64_t start = (uint64_t) buf & PAGE_MASK;
	return start;
}

static inline uint64_t buf_page_offset(uint64_t buf)
{
	uint64_t offset = (uint64_t) buf & (PAGE_SIZE - 1);
	return offset;
}

static inline uint64_t buf_num_pages(uint64_t buf, size_t len)
{
	uint64_t start = buf_page_start(buf) >> PAGE_SHIFT;
	uint64_t end = (((uint64_t) buf + len - 1) & PAGE_MASK) >> PAGE_SHIFT;
	uint64_t nPages = end - start + 1;
	return nPages;
}

static inline uint64_t buf_page_size(uint32_t size)
{
	uint64_t sz = (size + (PAGE_SIZE - 1)) & PAGE_MASK;

	return sz > PAGE_SIZE ? sz : PAGE_SIZE;
}

static inline void *uint64_to_ptr(uint64_t addr)
{
	void *ptr = (void *)((uintptr_t)addr);

	return ptr;
}

static inline uint64_t ptr_to_uint64(void *ptr)
{
	uint64_t addr = (uint64_t)((uintptr_t)ptr);

	return addr;
}

struct secure_vm {
	int *vmid;
	int *vmperm;
	int vmcount;
};

struct fastrpc_file;

struct fastrpc_buf {
	struct hlist_node hn;
	struct fastrpc_file *fl;
	void *virt;
	uint64_t phys;
	size_t size;
};

struct fastrpc_ctx_lst;

struct overlap {
	uintptr_t start;
	uintptr_t end;
	int raix;
	uintptr_t mstart;
	uintptr_t mend;
	uintptr_t offset;
};

struct smq_invoke_ctx {
	struct hlist_node hn;
	struct completion work;
	int retval;
	int pid;
	int tgid;
	remote_arg_t *lpra;
	remote_arg64_t *rpra;
	int *fds;
	struct fastrpc_mmap **maps;
	struct fastrpc_buf *buf;
	size_t used;
	struct fastrpc_file *fl;
	uint32_t sc;
	struct overlap *overs;
	struct overlap **overps;
	struct smq_msg msg;
	unsigned int magic;
	unsigned int *attrs;
	uint32_t *crc;
	uint64_t ctxid;
};

struct fastrpc_ctx_lst {
	struct hlist_head pending;
	struct hlist_head interrupted;
};

struct fastrpc_smmu {
	struct device *dev;
	struct dma_iommu_mapping *mapping;
	int cb;
	int enabled;
	int faults;
	int secure;
	int coherent;
	int sharedcb;
};

struct fastrpc_session_ctx {
	struct device *dev;
	struct fastrpc_smmu smmu;
	int used;
};

struct fastrpc_static_pd {
	char *spdname;
	struct notifier_block pdrnb;
	struct notifier_block get_service_nb;
	void *pdrhandle;
	int pdrcount;
	int prevpdrcount;
	int ispdup;
};

struct fastrpc_glink_info {
	int link_state;
	int port_state;
	struct glink_open_config cfg;
	struct glink_link_info link_info;
	void *link_notify_handle;
};

struct fastrpc_channel_ctx {
	char *name;
	char *subsys;
	void *chan;
	struct device *dev;
	struct fastrpc_session_ctx session[NUM_SESSIONS];
	struct fastrpc_static_pd spd[NUM_SESSIONS];
	struct completion work;
	struct completion workport;
	struct notifier_block nb;
	struct kref kref;
	int channel;
	int sesscount;
	int ssrcount;
	void *handle;
	int prevssrcount;
	int issubsystemup;
	int vmid;
	struct secure_vm rhvm;
	int ramdumpenabled;
	void *remoteheap_ramdump_dev;
	struct fastrpc_glink_info link;
};

struct fastrpc_apps {
	struct fastrpc_channel_ctx *channel;
	struct cdev cdev;
	struct class *class;
	struct mutex smd_mutex;
	struct smq_phy_page range;
	struct hlist_head maps;
	uint32_t staticpd_flags;
	dev_t dev_no;
	int compat;
	struct hlist_head drivers;
	spinlock_t hlock;
	struct ion_client *client;
	struct device *dev;
	unsigned int latency;
	bool glink;
	bool legacy;
	spinlock_t ctxlock;
	struct smq_invoke_ctx *ctxtable[FASTRPC_CTX_MAX];
};

struct fastrpc_mmap {
	struct hlist_node hn;
	struct fastrpc_file *fl;
	struct fastrpc_apps *apps;
	int fd;
	uint32_t flags;
	struct dma_buf *buf;
	struct sg_table *table;
	struct dma_buf_attachment *attach;
	struct ion_handle *handle;
	uint64_t phys;
	size_t size;
	uintptr_t va;
	size_t len;
	int refs;
	uintptr_t raddr;
	int uncached;
	int secure;
	uintptr_t attr;
};

enum fastrpc_perfkeys {
	PERF_COUNT = 0,
	PERF_FLUSH = 1,
	PERF_MAP = 2,
	PERF_COPY = 3,
	PERF_LINK = 4,
	PERF_GETARGS = 5,
	PERF_PUTARGS = 6,
	PERF_INVARGS = 7,
	PERF_INVOKE = 8,
	PERF_KEY_MAX = 9,
};

struct fastrpc_perf {
	int64_t count;
	int64_t flush;
	int64_t map;
	int64_t copy;
	int64_t link;
	int64_t getargs;
	int64_t putargs;
	int64_t invargs;
	int64_t invoke;
	int64_t tid;
	struct hlist_node hn;
};

struct fastrpc_file {
	struct hlist_node hn;
	spinlock_t hlock;
	struct hlist_head maps;
	struct hlist_head bufs;
	struct fastrpc_ctx_lst clst;
	struct fastrpc_session_ctx *sctx;
	struct fastrpc_session_ctx *secsctx;
	uint32_t mode;
	uint32_t profile;
	int sessionid;
	int tgid;
	int cid;
	int ssrcount;
	int pd;
	char *spdname;
	int file_close;
	int sharedcb;
	struct fastrpc_apps *apps;
	struct hlist_head perf;
	struct dentry *debugfs_file;
	struct mutex perf_mutex;
	struct pm_qos_request pm_qos_req;
	int qos_request;
	struct mutex map_mutex;
	struct mutex fl_map_mutex;
	int refcount;
};

static struct fastrpc_apps gfa;

static struct fastrpc_channel_ctx gcinfo[NUM_CHANNELS] = {
	{
		.name = "adsprpc-smd",
		.subsys = "adsp",
		.channel = SMD_APPS_QDSP,
		.link.link_info.edge = "lpass",
		.link.link_info.transport = "smem",
		.spd = {
			{
				.spdname =
					AUDIO_PDR_SERVICE_LOCATION_CLIENT_NAME,
				.pdrnb.notifier_call =
						fastrpc_pdr_notifier_cb,
			},
			{
				.spdname =
				SENSORS_PDR_SERVICE_LOCATION_CLIENT_NAME,
				.pdrnb.notifier_call =
						fastrpc_pdr_notifier_cb,
			}
		},
	},
	{
		.name = "mdsprpc-smd",
		.subsys = "modem",
		.channel = SMD_APPS_MODEM,
		.link.link_info.edge = "mpss",
		.link.link_info.transport = "smem",
	},
	{
		.name = "sdsprpc-smd",
		.subsys = "slpi",
		.channel = SMD_APPS_DSPS,
		.link.link_info.edge = "dsps",
		.link.link_info.transport = "smem",
	},
	{
		.name = "cdsprpc-smd",
		.subsys = "cdsp",
		.link.link_info.edge = "cdsp",
		.link.link_info.transport = "smem",
	},
};

static int hlosvm[1] = {VMID_HLOS};
static int hlosvmperm[1] = {PERM_READ | PERM_WRITE | PERM_EXEC};

static inline int64_t getnstimediff(struct timespec *start)
{
	int64_t ns;
	struct timespec ts, b;

	getnstimeofday(&ts);
	b = timespec_sub(ts, *start);
	ns = timespec_to_ns(&b);
	return ns;
}

static inline int64_t *getperfcounter(struct fastrpc_file *fl, int key)
{
	int err = 0;
	int64_t *val = NULL;
	struct fastrpc_perf *perf = NULL, *fperf = NULL;
	struct hlist_node *n = NULL;

	VERIFY(err, !IS_ERR_OR_NULL(fl));
	if (err)
		goto bail;

	mutex_lock(&fl->perf_mutex);
	hlist_for_each_entry_safe(perf, n, &fl->perf, hn) {
		if (perf->tid == current->pid) {
			fperf = perf;
			break;
		}
	}

	if (IS_ERR_OR_NULL(fperf)) {
		fperf = kzalloc(sizeof(*fperf), GFP_KERNEL);

		VERIFY(err, !IS_ERR_OR_NULL(fperf));
		if (err) {
			mutex_unlock(&fl->perf_mutex);
			kfree(fperf);
			goto bail;
		}

		fperf->tid = current->pid;
		hlist_add_head(&fperf->hn, &fl->perf);
	}

	val = ((int64_t *)fperf) + key;
	mutex_unlock(&fl->perf_mutex);
bail:
	return val;
}


static void fastrpc_buf_free(struct fastrpc_buf *buf, int cache)
{
	struct fastrpc_file *fl = buf == NULL ? NULL : buf->fl;
	int vmid;

	if (!fl)
		return;
	if (cache) {
		spin_lock(&fl->hlock);
		hlist_add_head(&buf->hn, &fl->bufs);
		spin_unlock(&fl->hlock);
		return;
	}
	if (!IS_ERR_OR_NULL(buf->virt)) {
		int destVM[1] = {VMID_HLOS};
		int destVMperm[1] = {PERM_READ | PERM_WRITE | PERM_EXEC};

		if (fl->sctx->smmu.cb)
			buf->phys &= ~((uint64_t)fl->sctx->smmu.cb << 32);
		vmid = fl->apps->channel[fl->cid].vmid;
		if (vmid) {
			int srcVM[2] = {VMID_HLOS, vmid};

			hyp_assign_phys(buf->phys, buf_page_size(buf->size),
				srcVM, 2, destVM, destVMperm, 1);
		}
		dma_free_coherent(fl->sctx->smmu.dev, buf->size, buf->virt,
					buf->phys);
	}
	kfree(buf);
}

static void fastrpc_buf_list_free(struct fastrpc_file *fl)
{
	struct fastrpc_buf *buf, *free;

	do {
		struct hlist_node *n;

		free = NULL;
		spin_lock(&fl->hlock);
		hlist_for_each_entry_safe(buf, n, &fl->bufs, hn) {
			hlist_del_init(&buf->hn);
			free = buf;
			break;
		}
		spin_unlock(&fl->hlock);
		if (free)
			fastrpc_buf_free(free, 0);
	} while (free);
}

static void fastrpc_mmap_add(struct fastrpc_mmap *map)
{
	if (map->flags == ADSP_MMAP_HEAP_ADDR ||
				map->flags == ADSP_MMAP_REMOTE_HEAP_ADDR) {
		struct fastrpc_apps *me = &gfa;

		spin_lock(&me->hlock);
		hlist_add_head(&map->hn, &me->maps);
		spin_unlock(&me->hlock);
	} else {
		struct fastrpc_file *fl = map->fl;

		hlist_add_head(&map->hn, &fl->maps);
	}
}

static int fastrpc_mmap_find(struct fastrpc_file *fl, int fd,
		uintptr_t va, size_t len, int mflags, int refs,
		struct fastrpc_mmap **ppmap)
{
	struct fastrpc_apps *me = &gfa;
	struct fastrpc_mmap *match = NULL, *map = NULL;
	struct hlist_node *n;

	if ((va + len) < va)
		return -EOVERFLOW;
	if (mflags == ADSP_MMAP_HEAP_ADDR ||
				 mflags == ADSP_MMAP_REMOTE_HEAP_ADDR) {
		spin_lock(&me->hlock);
		hlist_for_each_entry_safe(map, n, &me->maps, hn) {
			if (va >= map->va &&
				va + len <= map->va + map->len &&
				map->fd == fd) {
				if (refs)
					map->refs++;
				match = map;
				break;
			}
		}
		spin_unlock(&me->hlock);
	} else {
		hlist_for_each_entry_safe(map, n, &fl->maps, hn) {
			if (va >= map->va &&
				va + len <= map->va + map->len &&
				map->fd == fd) {
				if (refs)
					map->refs++;
				match = map;
				break;
			}
		}
	}
	if (match) {
		*ppmap = match;
		return 0;
	}
	return -ENOTTY;
}

static int dma_alloc_memory(dma_addr_t *region_phys, size_t size)
{
	struct fastrpc_apps *me = &gfa;
	void *vaddr = NULL;
	unsigned long dma_attrs = 0;

	if (me->dev == NULL) {
		pr_err("device adsprpc-mem is not initialized\n");
		return -ENODEV;
	}
	dma_attrs |= DMA_ATTR_SKIP_ZEROING | DMA_ATTR_NO_KERNEL_MAPPING;
	vaddr = dma_alloc_attrs(me->dev, size, region_phys, GFP_KERNEL,
								dma_attrs);
	if (!vaddr) {
		pr_err("ADSPRPC: Failed to allocate %x remote heap memory\n",
						(unsigned int)size);
		return -ENOMEM;
	}
	return 0;
}

static int fastrpc_mmap_remove(struct fastrpc_file *fl, uintptr_t va,
			       size_t len, struct fastrpc_mmap **ppmap)
{
	struct fastrpc_mmap *match = NULL, *map;
	struct hlist_node *n;
	struct fastrpc_apps *me = &gfa;

	spin_lock(&me->hlock);
	hlist_for_each_entry_safe(map, n, &me->maps, hn) {
		if (map->raddr == va &&
			map->raddr + map->len == va + len &&
			map->refs == 1) {
			match = map;
			hlist_del_init(&map->hn);
			break;
		}
	}
	spin_unlock(&me->hlock);
	if (match) {
		*ppmap = match;
		return 0;
	}
	hlist_for_each_entry_safe(map, n, &fl->maps, hn) {
		if (map->raddr == va &&
			map->raddr + map->len == va + len &&
			map->refs == 1) {
			match = map;
			hlist_del_init(&map->hn);
			break;
		}
	}
	if (match) {
		*ppmap = match;
		return 0;
	}
	return -ENOTTY;
}

static void fastrpc_mmap_free(struct fastrpc_mmap *map, uint32_t flags)
{
	struct fastrpc_apps *me = &gfa;
	struct fastrpc_file *fl;
	int vmid;
	struct fastrpc_session_ctx *sess;

	if (!map)
		return;
	fl = map->fl;
	if (map->flags == ADSP_MMAP_HEAP_ADDR ||
				map->flags == ADSP_MMAP_REMOTE_HEAP_ADDR) {
		spin_lock(&me->hlock);
		map->refs--;
		if (!map->refs)
			hlist_del_init(&map->hn);
		spin_unlock(&me->hlock);
		if (map->refs > 0)
			return;
	} else {
		map->refs--;
		if (!map->refs)
			hlist_del_init(&map->hn);
		if (map->refs > 0 && !flags)
			return;
	}
	if (map->flags == ADSP_MMAP_HEAP_ADDR ||
				map->flags == ADSP_MMAP_REMOTE_HEAP_ADDR) {
		unsigned long dma_attrs = 0;

		if (me->dev == NULL) {
			pr_err("failed to free remote heap allocation\n");
			return;
		}
		if (map->phys) {
			dma_attrs |=
			DMA_ATTR_SKIP_ZEROING | DMA_ATTR_NO_KERNEL_MAPPING;
			dma_free_attrs(me->dev, map->size, (void *)map->va,
					(dma_addr_t)map->phys, dma_attrs);
		}
	} else if (map->flags == FASTRPC_DMAHANDLE_NOMAP) {
		if (!IS_ERR_OR_NULL(map->handle))
			ion_free(fl->apps->client, map->handle);
	} else {
		int destVM[1] = {VMID_HLOS};
		int destVMperm[1] = {PERM_READ | PERM_WRITE | PERM_EXEC};

		if (map->secure)
			sess = fl->secsctx;
		else
			sess = fl->sctx;

		if (!IS_ERR_OR_NULL(map->handle))
			ion_free(fl->apps->client, map->handle);
		if (sess && sess->smmu.enabled) {
			if (map->size || map->phys)
				msm_dma_unmap_sg(sess->smmu.dev,
					map->table->sgl,
					map->table->nents, DMA_BIDIRECTIONAL,
					map->buf);
		}
		vmid = fl->apps->channel[fl->cid].vmid;
		if (vmid && map->phys) {
			int srcVM[2] = {VMID_HLOS, vmid};

			hyp_assign_phys(map->phys, buf_page_size(map->size),
				srcVM, 2, destVM, destVMperm, 1);
		}

		if (!IS_ERR_OR_NULL(map->table))
			dma_buf_unmap_attachment(map->attach, map->table,
					DMA_BIDIRECTIONAL);
		if (!IS_ERR_OR_NULL(map->attach))
			dma_buf_detach(map->buf, map->attach);
		if (!IS_ERR_OR_NULL(map->buf))
			dma_buf_put(map->buf);
	}
	kfree(map);
}

static int fastrpc_session_alloc(struct fastrpc_channel_ctx *chan, int secure,
					struct fastrpc_session_ctx **session);

static int fastrpc_mmap_create(struct fastrpc_file *fl, int fd,
	unsigned int attr, uintptr_t va, size_t len, int mflags,
	struct fastrpc_mmap **ppmap)
{
	struct fastrpc_apps *me = &gfa;
	struct fastrpc_session_ctx *sess;
	struct fastrpc_apps *apps = fl->apps;
	int cid = fl->cid;
	struct fastrpc_channel_ctx *chan = &apps->channel[cid];
	struct fastrpc_mmap *map = NULL;
	unsigned long attrs;
	dma_addr_t region_phys = 0;
	unsigned long flags;
	int err = 0, vmid;

	if (!fastrpc_mmap_find(fl, fd, va, len, mflags, 1, ppmap))
		return 0;
	map = kzalloc(sizeof(*map), GFP_KERNEL);
	VERIFY(err, !IS_ERR_OR_NULL(map));
	if (err)
		goto bail;
	INIT_HLIST_NODE(&map->hn);
	map->flags = mflags;
	map->refs = 1;
	map->fl = fl;
	map->fd = fd;
	map->attr = attr;
	if (mflags == ADSP_MMAP_HEAP_ADDR ||
				mflags == ADSP_MMAP_REMOTE_HEAP_ADDR) {
		map->apps = me;
		map->fl = NULL;
		VERIFY(err, !dma_alloc_memory(&region_phys, len));
		if (err)
			goto bail;
		map->phys = (uintptr_t)region_phys;
		map->size = len;
		map->va = (uintptr_t)map->phys;
	} else if (mflags == FASTRPC_DMAHANDLE_NOMAP) {
		ion_phys_addr_t iphys;

		VERIFY(err, !IS_ERR_OR_NULL(map->handle =
				ion_import_dma_buf_fd(fl->apps->client, fd)));
		if (err)
			goto bail;

		map->uncached = 1;
		map->buf = NULL;
		map->attach = NULL;
		map->table = NULL;
		map->va = 0;
		map->phys = 0;

		err = ion_phys(fl->apps->client, map->handle,
			&iphys, &map->size);
		if (err)
			goto bail;
		map->phys = (uint64_t)iphys;
	} else {
		if (map->attr && (map->attr & FASTRPC_ATTR_KEEP_MAP)) {
			pr_info("adsprpc: buffer mapped with persist attr %x\n",
				(unsigned int)map->attr);
			map->refs = 2;
		}
		VERIFY(err, !IS_ERR_OR_NULL(map->handle =
				ion_import_dma_buf_fd(fl->apps->client, fd)));
		if (err)
			goto bail;
		VERIFY(err, !ion_handle_get_flags(fl->apps->client, map->handle,
						&flags));
		if (err)
			goto bail;

		map->secure = flags & ION_FLAG_SECURE;
		if (map->secure) {
			if (!fl->secsctx)
				err = fastrpc_session_alloc(chan, 1,
							&fl->secsctx);
			if (err)
				goto bail;
		}
		if (map->secure)
			sess = fl->secsctx;
		else
			sess = fl->sctx;

		VERIFY(err, !IS_ERR_OR_NULL(sess));
		if (err)
			goto bail;

		map->uncached = !ION_IS_CACHED(flags);
		if (map->attr & FASTRPC_ATTR_NOVA && !sess->smmu.coherent)
			map->uncached = 1;

		VERIFY(err, !IS_ERR_OR_NULL(map->buf = dma_buf_get(fd)));
		if (err)
			goto bail;
		VERIFY(err, !IS_ERR_OR_NULL(map->attach =
				dma_buf_attach(map->buf, sess->smmu.dev)));
		if (err)
			goto bail;
		VERIFY(err, !IS_ERR_OR_NULL(map->table =
			dma_buf_map_attachment(map->attach,
				DMA_BIDIRECTIONAL)));
		if (err)
			goto bail;
		if (sess->smmu.enabled) {
		attrs = DMA_ATTR_EXEC_MAPPING;

		if (map->attr & FASTRPC_ATTR_NON_COHERENT ||
			(sess->smmu.coherent && map->uncached))
			attrs |= DMA_ATTR_FORCE_NON_COHERENT;
		else if (map->attr & FASTRPC_ATTR_COHERENT)
			attrs |= DMA_ATTR_FORCE_COHERENT;

		VERIFY(err, map->table->nents ==
				msm_dma_map_sg_attrs(sess->smmu.dev,
				map->table->sgl, map->table->nents,
				DMA_BIDIRECTIONAL, map->buf, attrs));
			if (err)
				goto bail;
		} else {
			VERIFY(err, map->table->nents == 1);
			if (err)
			goto bail;
		}
		map->phys = sg_dma_address(map->table->sgl);

		if (sess->smmu.cb) {
			map->phys += ((uint64_t)sess->smmu.cb << 32);
			map->size = sg_dma_len(map->table->sgl);
		} else {
			map->size = buf_page_size(len);
		}

		vmid = fl->apps->channel[fl->cid].vmid;
		if (!sess->smmu.enabled && !vmid) {
			VERIFY(err, map->phys >= me->range.addr &&
			map->phys + map->size <=
			me->range.addr + me->range.size);
			if (err) {
				pr_err("adsprpc: mmap fail out of range\n");
				goto bail;
			}
		}
		if (vmid) {
			int srcVM[1] = {VMID_HLOS};
			int destVM[2] = {VMID_HLOS, vmid};
			int destVMperm[2] = {PERM_READ | PERM_WRITE,
					PERM_READ | PERM_WRITE | PERM_EXEC};

			VERIFY(err, !hyp_assign_phys(map->phys,
					buf_page_size(map->size),
					srcVM, 1, destVM, destVMperm, 2));
			if (err)
				goto bail;
		}
		map->va = va;
	}
	map->len = len;

	fastrpc_mmap_add(map);
	*ppmap = map;

bail:
	if (err && map)
		fastrpc_mmap_free(map, 0);
	return err;
}

static int fastrpc_buf_alloc(struct fastrpc_file *fl, size_t size,
			     struct fastrpc_buf **obuf)
{
	int err = 0, vmid;
	struct fastrpc_buf *buf = NULL, *fr = NULL;
	struct hlist_node *n;

	VERIFY(err, size > 0);
	if (err)
		goto bail;

	/* find the smallest buffer that fits in the cache */
	spin_lock(&fl->hlock);
	hlist_for_each_entry_safe(buf, n, &fl->bufs, hn) {
		if (buf->size >= size && (!fr || fr->size > buf->size))
			fr = buf;
	}
	if (fr)
		hlist_del_init(&fr->hn);
	spin_unlock(&fl->hlock);
	if (fr) {
		*obuf = fr;
		return 0;
	}
	buf = NULL;
	VERIFY(err, NULL != (buf = kzalloc(sizeof(*buf), GFP_KERNEL)));
	if (err)
		goto bail;
	INIT_HLIST_NODE(&buf->hn);
	buf->fl = fl;
	buf->virt = NULL;
	buf->phys = 0;
	buf->size = size;
	buf->virt = dma_alloc_coherent(fl->sctx->smmu.dev, buf->size,
				       (void *)&buf->phys, GFP_KERNEL);
	if (IS_ERR_OR_NULL(buf->virt)) {
		/* free cache and retry */
		fastrpc_buf_list_free(fl);
		buf->virt = dma_alloc_coherent(fl->sctx->smmu.dev, buf->size,
					       (void *)&buf->phys, GFP_KERNEL);
		VERIFY(err, !IS_ERR_OR_NULL(buf->virt));
	}
	if (err)
		goto bail;
	if (fl->sctx->smmu.cb)
		buf->phys += ((uint64_t)fl->sctx->smmu.cb << 32);
	vmid = fl->apps->channel[fl->cid].vmid;
	if (vmid) {
		int srcVM[1] = {VMID_HLOS};
		int destVM[2] = {VMID_HLOS, vmid};
		int destVMperm[2] = {PERM_READ | PERM_WRITE,
					PERM_READ | PERM_WRITE | PERM_EXEC};

		VERIFY(err, !hyp_assign_phys(buf->phys, buf_page_size(size),
			srcVM, 1, destVM, destVMperm, 2));
		if (err)
			goto bail;
	}

	*obuf = buf;
 bail:
	if (err && buf)
		fastrpc_buf_free(buf, 0);
	return err;
}


static int context_restore_interrupted(struct fastrpc_file *fl,
				       struct fastrpc_ioctl_invoke_crc *inv,
				       struct smq_invoke_ctx **po)
{
	int err = 0;
	struct smq_invoke_ctx *ctx = NULL, *ictx = NULL;
	struct hlist_node *n;
	struct fastrpc_ioctl_invoke *invoke = &inv->inv;

	spin_lock(&fl->hlock);
	hlist_for_each_entry_safe(ictx, n, &fl->clst.interrupted, hn) {
		if (ictx->pid == current->pid) {
			if (invoke->sc != ictx->sc || ictx->fl != fl)
				err = -1;
			else {
				ctx = ictx;
				hlist_del_init(&ctx->hn);
				hlist_add_head(&ctx->hn, &fl->clst.pending);
			}
			break;
		}
	}
	spin_unlock(&fl->hlock);
	if (ctx)
		*po = ctx;
	return err;
}

#define CMP(aa, bb) ((aa) == (bb) ? 0 : (aa) < (bb) ? -1 : 1)
static int overlap_ptr_cmp(const void *a, const void *b)
{
	struct overlap *pa = *((struct overlap **)a);
	struct overlap *pb = *((struct overlap **)b);
	/* sort with lowest starting buffer first */
	int st = CMP(pa->start, pb->start);
	/* sort with highest ending buffer first */
	int ed = CMP(pb->end, pa->end);
	return st == 0 ? ed : st;
}

static int context_build_overlap(struct smq_invoke_ctx *ctx)
{
	int i, err = 0;
	remote_arg_t *lpra = ctx->lpra;
	int inbufs = REMOTE_SCALARS_INBUFS(ctx->sc);
	int outbufs = REMOTE_SCALARS_OUTBUFS(ctx->sc);
	int nbufs = inbufs + outbufs;
	struct overlap max;

	for (i = 0; i < nbufs; ++i) {
		ctx->overs[i].start = (uintptr_t)lpra[i].buf.pv;
		ctx->overs[i].end = ctx->overs[i].start + lpra[i].buf.len;
		if (lpra[i].buf.len) {
			VERIFY(err, ctx->overs[i].end > ctx->overs[i].start);
			if (err)
				goto bail;
		}
		ctx->overs[i].raix = i;
		ctx->overps[i] = &ctx->overs[i];
	}
	sort(ctx->overps, nbufs, sizeof(*ctx->overps), overlap_ptr_cmp, NULL);
	max.start = 0;
	max.end = 0;
	for (i = 0; i < nbufs; ++i) {
		if (ctx->overps[i]->start < max.end) {
			ctx->overps[i]->mstart = max.end;
			ctx->overps[i]->mend = ctx->overps[i]->end;
			ctx->overps[i]->offset = max.end -
				ctx->overps[i]->start;
			if (ctx->overps[i]->end > max.end) {
				max.end = ctx->overps[i]->end;
			} else {
				ctx->overps[i]->mend = 0;
				ctx->overps[i]->mstart = 0;
			}
		} else  {
			ctx->overps[i]->mend = ctx->overps[i]->end;
			ctx->overps[i]->mstart = ctx->overps[i]->start;
			ctx->overps[i]->offset = 0;
			max = *ctx->overps[i];
		}
	}
bail:
	return err;
}

#define K_COPY_FROM_USER(err, kernel, dst, src, size) \
	do {\
		if (!(kernel))\
			VERIFY(err, 0 == copy_from_user((dst),\
			(void const __user *)(src),\
							(size)));\
		else\
			memmove((dst), (src), (size));\
	} while (0)

#define K_COPY_TO_USER(err, kernel, dst, src, size) \
	do {\
		if (!(kernel))\
			VERIFY(err, 0 == copy_to_user((void __user *)(dst),\
						(src), (size)));\
		else\
			memmove((dst), (src), (size));\
	} while (0)


static void context_free(struct smq_invoke_ctx *ctx);

static int context_alloc(struct fastrpc_file *fl, uint32_t kernel,
			 struct fastrpc_ioctl_invoke_crc *invokefd,
			 struct smq_invoke_ctx **po)
{
	struct fastrpc_apps *me = &gfa;
	int err = 0, bufs, ii, size = 0;
	struct smq_invoke_ctx *ctx = NULL;
	struct fastrpc_ctx_lst *clst = &fl->clst;
	struct fastrpc_ioctl_invoke *invoke = &invokefd->inv;

	bufs = REMOTE_SCALARS_LENGTH(invoke->sc);
	size = bufs * sizeof(*ctx->lpra) + bufs * sizeof(*ctx->maps) +
		sizeof(*ctx->fds) * (bufs) +
		sizeof(*ctx->attrs) * (bufs) +
		sizeof(*ctx->overs) * (bufs) +
		sizeof(*ctx->overps) * (bufs);

	VERIFY(err, NULL != (ctx = kzalloc(sizeof(*ctx) + size, GFP_KERNEL)));
	if (err)
		goto bail;

	INIT_HLIST_NODE(&ctx->hn);
	hlist_add_fake(&ctx->hn);
	ctx->fl = fl;
	ctx->maps = (struct fastrpc_mmap **)(&ctx[1]);
	ctx->lpra = (remote_arg_t *)(&ctx->maps[bufs]);
	ctx->fds = (int *)(&ctx->lpra[bufs]);
	if (me->legacy) {
		ctx->overs = (struct overlap *)(&ctx->fds[bufs]);
		ctx->overps = (struct overlap **)(&ctx->overs[bufs]);
	} else {
		ctx->attrs = (unsigned int *)(&ctx->fds[bufs]);
		ctx->overs = (struct overlap *)(&ctx->attrs[bufs]);
		ctx->overps = (struct overlap **)(&ctx->overs[bufs]);
	}

	K_COPY_FROM_USER(err, kernel, (void *)ctx->lpra, invoke->pra,
					bufs * sizeof(*ctx->lpra));
	if (err)
		goto bail;

	if (invokefd->fds) {
		K_COPY_FROM_USER(err, kernel, ctx->fds, invokefd->fds,
						bufs * sizeof(*ctx->fds));
		if (err)
			goto bail;
	}
	if (invokefd->attrs) {
		K_COPY_FROM_USER(err, kernel, ctx->attrs, invokefd->attrs,
						bufs * sizeof(*ctx->attrs));
		if (err)
			goto bail;
	}
	ctx->crc = (uint32_t *)invokefd->crc;
	ctx->sc = invoke->sc;
	if (bufs) {
		VERIFY(err, 0 == context_build_overlap(ctx));
		if (err)
			goto bail;
	}
	ctx->retval = -1;
	ctx->pid = current->pid;
	ctx->tgid = fl->tgid;
	init_completion(&ctx->work);
	ctx->magic = FASTRPC_CTX_MAGIC;

	spin_lock(&fl->hlock);
	hlist_add_head(&ctx->hn, &clst->pending);
	spin_unlock(&fl->hlock);

	spin_lock(&me->ctxlock);
	for (ii = 0; ii < FASTRPC_CTX_MAX; ii++) {
		if (!me->ctxtable[ii]) {
			me->ctxtable[ii] = ctx;
			ctx->ctxid = (ptr_to_uint64(ctx) & ~0xFFF)|(ii << 4);
			break;
		}
	}
	spin_unlock(&me->ctxlock);
	VERIFY(err, ii < FASTRPC_CTX_MAX);
	if (err) {
		pr_err("adsprpc: out of context memory\n");
		goto bail;
	}

	*po = ctx;
bail:
	if (ctx && err)
		context_free(ctx);
	return err;
}

static void context_save_interrupted(struct smq_invoke_ctx *ctx)
{
	struct fastrpc_ctx_lst *clst = &ctx->fl->clst;

	spin_lock(&ctx->fl->hlock);
	hlist_del_init(&ctx->hn);
	hlist_add_head(&ctx->hn, &clst->interrupted);
	spin_unlock(&ctx->fl->hlock);
}

static void context_free(struct smq_invoke_ctx *ctx)
{
	int i;
	struct fastrpc_apps *me = &gfa;
	int nbufs = REMOTE_SCALARS_INBUFS(ctx->sc) +
		    REMOTE_SCALARS_OUTBUFS(ctx->sc);
	spin_lock(&ctx->fl->hlock);
	hlist_del_init(&ctx->hn);
	spin_unlock(&ctx->fl->hlock);
	mutex_lock(&ctx->fl->fl_map_mutex);
	for (i = 0; i < nbufs; ++i)
		fastrpc_mmap_free(ctx->maps[i], 0);

	mutex_unlock(&ctx->fl->fl_map_mutex);
	fastrpc_buf_free(ctx->buf, 1);
	ctx->magic = 0;
	ctx->ctxid = 0;

	spin_lock(&me->ctxlock);
	for (i = 0; i < FASTRPC_CTX_MAX; i++) {
		if (me->ctxtable[i] == ctx) {
			me->ctxtable[i] = NULL;
			break;
		}
	}
	spin_unlock(&me->ctxlock);

	kfree(ctx);
}

static void context_notify_user(struct smq_invoke_ctx *ctx, int retval)
{
	ctx->retval = retval;
	complete(&ctx->work);
}


static void fastrpc_notify_users(struct fastrpc_file *me)
{
	struct smq_invoke_ctx *ictx;
	struct hlist_node *n;

	spin_lock(&me->hlock);
	hlist_for_each_entry_safe(ictx, n, &me->clst.pending, hn) {
		complete(&ictx->work);
	}
	hlist_for_each_entry_safe(ictx, n, &me->clst.interrupted, hn) {
		complete(&ictx->work);
	}
	spin_unlock(&me->hlock);

}


static void fastrpc_notify_users_staticpd_pdr(struct fastrpc_file *me)
{
	struct smq_invoke_ctx *ictx;
	struct hlist_node *n;

	spin_lock(&me->hlock);
	hlist_for_each_entry_safe(ictx, n, &me->clst.pending, hn) {
		if (ictx->msg.pid)
			complete(&ictx->work);
	}
	hlist_for_each_entry_safe(ictx, n, &me->clst.interrupted, hn) {
		if (ictx->msg.pid)
			complete(&ictx->work);
	}
	spin_unlock(&me->hlock);
}


static void fastrpc_notify_drivers(struct fastrpc_apps *me, int cid)
{
	struct fastrpc_file *fl;
	struct hlist_node *n;

	spin_lock(&me->hlock);
	hlist_for_each_entry_safe(fl, n, &me->drivers, hn) {
		if (fl->cid == cid)
			fastrpc_notify_users(fl);
	}
	spin_unlock(&me->hlock);

}

static void fastrpc_notify_pdr_drivers(struct fastrpc_apps *me, char *spdname)
{
	struct fastrpc_file *fl;
	struct hlist_node *n;

	spin_lock(&me->hlock);
	hlist_for_each_entry_safe(fl, n, &me->drivers, hn) {
		if (fl->spdname && !strcmp(spdname, fl->spdname))
			fastrpc_notify_users_staticpd_pdr(fl);
	}
	spin_unlock(&me->hlock);

}

static void context_list_ctor(struct fastrpc_ctx_lst *me)
{
	INIT_HLIST_HEAD(&me->interrupted);
	INIT_HLIST_HEAD(&me->pending);
}

static void fastrpc_context_list_dtor(struct fastrpc_file *fl)
{
	struct fastrpc_ctx_lst *clst = &fl->clst;
	struct smq_invoke_ctx *ictx = NULL, *ctxfree;
	struct hlist_node *n;

	do {
		ctxfree = NULL;
		spin_lock(&fl->hlock);
		hlist_for_each_entry_safe(ictx, n, &clst->interrupted, hn) {
			hlist_del_init(&ictx->hn);
			ctxfree = ictx;
			break;
		}
		spin_unlock(&fl->hlock);
		if (ctxfree)
			context_free(ctxfree);
	} while (ctxfree);
	do {
		ctxfree = NULL;
		spin_lock(&fl->hlock);
		hlist_for_each_entry_safe(ictx, n, &clst->pending, hn) {
			hlist_del_init(&ictx->hn);
			ctxfree = ictx;
			break;
		}
		spin_unlock(&fl->hlock);
		if (ctxfree)
			context_free(ctxfree);
	} while (ctxfree);
}

static int fastrpc_file_free(struct fastrpc_file *fl);
static void fastrpc_file_list_dtor(struct fastrpc_apps *me)
{
	struct fastrpc_file *fl, *free;
	struct hlist_node *n;

	do {
		free = NULL;
		spin_lock(&me->hlock);
		hlist_for_each_entry_safe(fl, n, &me->drivers, hn) {
			hlist_del_init(&fl->hn);
			free = fl;
			break;
		}
		spin_unlock(&me->hlock);
		if (free)
			fastrpc_file_free(free);
	} while (free);
}

static int get_args(uint32_t kernel, struct smq_invoke_ctx *ctx)
{
	struct fastrpc_apps *me = &gfa;
	remote_arg64_t *rpra;
	remote_arg_t *lpra = ctx->lpra;
	struct smq_invoke_buf *list;
	struct smq_phy_page *pages, *ipage;
	uint32_t sc = ctx->sc;
	int inbufs = REMOTE_SCALARS_INBUFS(sc);
	int outbufs = REMOTE_SCALARS_OUTBUFS(sc);
	int handles, bufs = inbufs + outbufs;
	uintptr_t args;
	size_t rlen = 0, copylen = 0, metalen = 0;
	int i, oix;
	int err = 0;
	int mflags = 0;
	uint64_t *fdlist;
	uint32_t *crclist;
	int64_t *perf_counter = getperfcounter(ctx->fl, PERF_COUNT);

	/* calculate size of the metadata */
	rpra = NULL;
	list = smq_invoke_buf_start(rpra, sc);
	pages = smq_phy_page_start(sc, list);
	ipage = pages;

	PERF(ctx->fl->profile, GET_COUNTER(perf_counter, PERF_MAP),
	for (i = 0; i < bufs; ++i) {
		uintptr_t buf = (uintptr_t)lpra[i].buf.pv;
		size_t len = lpra[i].buf.len;

		mutex_lock(&ctx->fl->fl_map_mutex);
		if (ctx->fds[i] && (ctx->fds[i] != -1)) {
			unsigned int attrs = 0;

			if (ctx->attrs)
				attrs = ctx->attrs[i];

			fastrpc_mmap_create(ctx->fl, ctx->fds[i],
					attrs, buf, len,
					mflags, &ctx->maps[i]);
		}
		mutex_unlock(&ctx->fl->fl_map_mutex);
		ipage += 1;
	}
	PERF_END);
	handles = REMOTE_SCALARS_INHANDLES(sc) + REMOTE_SCALARS_OUTHANDLES(sc);
	mutex_lock(&ctx->fl->fl_map_mutex);
	for (i = bufs; i < bufs + handles; i++) {
		int dmaflags = 0;

		if (ctx->attrs && (ctx->attrs[i] & FASTRPC_ATTR_NOMAP))
			dmaflags = FASTRPC_DMAHANDLE_NOMAP;
		VERIFY(err, !fastrpc_mmap_create(ctx->fl, ctx->fds[i],
			FASTRPC_ATTR_NOVA, 0, 0, dmaflags, &ctx->maps[i]));
		if (err) {
			mutex_unlock(&ctx->fl->fl_map_mutex);
			goto bail;
		}
		ipage += 1;
	}
	mutex_unlock(&ctx->fl->fl_map_mutex);
	if (!me->legacy) {
		metalen = copylen = (size_t)&ipage[0] +
				(sizeof(uint64_t) * M_FDLIST) +
				(sizeof(uint32_t) * M_CRCLIST);
	} else {
		metalen = copylen = (size_t)&ipage[0];
	}

	/* calculate len requreed for copying */
	for (oix = 0; oix < inbufs + outbufs; ++oix) {
		int i = ctx->overps[oix]->raix;
		uintptr_t mstart, mend;
		size_t len = lpra[i].buf.len;

		if (!len)
			continue;
		if (ctx->maps[i])
			continue;
		if (ctx->overps[oix]->offset == 0)
			copylen = ALIGN(copylen, BALIGN);
		mstart = ctx->overps[oix]->mstart;
		mend = ctx->overps[oix]->mend;
		VERIFY(err, (mend - mstart) <= LONG_MAX);
		if (err)
			goto bail;
		copylen += mend - mstart;
		VERIFY(err, copylen >= 0);
		if (err)
			goto bail;
	}
	ctx->used = copylen;

	/* allocate new buffer */
	if (copylen) {
		VERIFY(err, !fastrpc_buf_alloc(ctx->fl, copylen, &ctx->buf));
		if (err)
			goto bail;
	}
	if (ctx->buf->virt && metalen <= copylen)
		memset(ctx->buf->virt, 0, metalen);

	/* copy metadata */
	rpra = ctx->buf->virt;
	ctx->rpra = rpra;
	list = smq_invoke_buf_start(rpra, sc);
	pages = smq_phy_page_start(sc, list);
	ipage = pages;
	args = (uintptr_t)ctx->buf->virt + metalen;
	for (i = 0; i < bufs + handles; ++i) {
		if (lpra[i].buf.len)
			list[i].num = 1;
		else
			list[i].num = 0;
		list[i].pgidx = ipage - pages;
		ipage++;
	}

	/* map ion buffers */
	PERF(ctx->fl->profile, GET_COUNTER(perf_counter, PERF_MAP),
	for (i = 0; rpra && i < inbufs + outbufs; ++i) {
		struct fastrpc_mmap *map = ctx->maps[i];
		uint64_t buf = ptr_to_uint64(lpra[i].buf.pv);
		size_t len = lpra[i].buf.len;

		rpra[i].buf.pv = 0;
		rpra[i].buf.len = len;
		if (!len)
			continue;
		if (map) {
			struct vm_area_struct *vma;
			uintptr_t offset;
			uint64_t num = buf_num_pages(buf, len);
			int idx = list[i].pgidx;

			if (map->attr & FASTRPC_ATTR_NOVA) {
				offset = 0;
			} else {
				down_read(&current->mm->mmap_sem);
				VERIFY(err, NULL != (vma = find_vma(current->mm,
								map->va)));
				if (err) {
					up_read(&current->mm->mmap_sem);
					goto bail;
				}
				offset = buf_page_start(buf) - vma->vm_start;
				up_read(&current->mm->mmap_sem);
				VERIFY(err, offset < (uintptr_t)map->size);
				if (err)
					goto bail;
			}
			pages[idx].addr = map->phys + offset;
			pages[idx].size = num << PAGE_SHIFT;
		}
		rpra[i].buf.pv = buf;
	}
	PERF_END);
	for (i = bufs; i < bufs + handles; ++i) {
		struct fastrpc_mmap *map = ctx->maps[i];

		pages[i].addr = map->phys;
		pages[i].size = map->size;
	}
	if (!me->legacy) {
		fdlist = (uint64_t *)&pages[bufs + handles];
		for (i = 0; i < M_FDLIST; i++)
			fdlist[i] = 0;
		crclist = (uint32_t *)&fdlist[M_FDLIST];
		memset(crclist, 0, sizeof(uint32_t)*M_CRCLIST);
	}

	/* copy non ion buffers */
	PERF(ctx->fl->profile, GET_COUNTER(perf_counter, PERF_COPY),
	rlen = copylen - metalen;
	for (oix = 0; rpra && oix < inbufs + outbufs; ++oix) {
		int i = ctx->overps[oix]->raix;
		struct fastrpc_mmap *map = ctx->maps[i];
		size_t mlen;
		uint64_t buf;
		size_t len = lpra[i].buf.len;

		if (!len)
			continue;
		if (map)
			continue;
		if (ctx->overps[oix]->offset == 0) {
			rlen -= ALIGN(args, BALIGN) - args;
			args = ALIGN(args, BALIGN);
		}
		mlen = ctx->overps[oix]->mend - ctx->overps[oix]->mstart;
		VERIFY(err, rlen >= mlen);
		if (err)
			goto bail;
		rpra[i].buf.pv = (args - ctx->overps[oix]->offset);
		pages[list[i].pgidx].addr = ctx->buf->phys -
					    ctx->overps[oix]->offset +
					    (copylen - rlen);
		pages[list[i].pgidx].addr =
			buf_page_start(pages[list[i].pgidx].addr);
		buf = rpra[i].buf.pv;
		pages[list[i].pgidx].size = buf_num_pages(buf, len) * PAGE_SIZE;
		if (i < inbufs) {
			K_COPY_FROM_USER(err, kernel, uint64_to_ptr(buf),
					lpra[i].buf.pv, len);
			if (err)
				goto bail;
		}
		args = args + mlen;
		rlen -= mlen;
	}
	PERF_END);

	PERF(ctx->fl->profile, GET_COUNTER(perf_counter, PERF_FLUSH),
	for (oix = 0; oix < inbufs + outbufs; ++oix) {
		int i = ctx->overps[oix]->raix;
		struct fastrpc_mmap *map = ctx->maps[i];

		if (map && map->uncached)
			continue;
		if (ctx->fl->sctx->smmu.coherent &&
			!(map && (map->attr & FASTRPC_ATTR_NON_COHERENT)))
			continue;
		if (map && (map->attr & FASTRPC_ATTR_COHERENT))
			continue;

		if (rpra && rpra[i].buf.len && ctx->overps[oix]->mstart) {
			if (map && map->handle)
				msm_ion_do_cache_op(ctx->fl->apps->client,
					map->handle,
					uint64_to_ptr(rpra[i].buf.pv),
					rpra[i].buf.len,
					ION_IOC_CLEAN_INV_CACHES);
			else
				dmac_flush_range(uint64_to_ptr(rpra[i].buf.pv),
					uint64_to_ptr(rpra[i].buf.pv
						+ rpra[i].buf.len));
		}
	}
	PERF_END);
	for (i = bufs; rpra && i < bufs + handles; i++) {
		rpra[i].dma.fd = ctx->fds[i];
		rpra[i].dma.len = (uint32_t)lpra[i].buf.len;
		rpra[i].dma.offset = (uint32_t)(uintptr_t)lpra[i].buf.pv;
	}

 bail:
	return err;
}

static int put_args(uint32_t kernel, struct smq_invoke_ctx *ctx,
		    remote_arg_t *upra)
{
	struct fastrpc_apps *me = &gfa;
	uint32_t sc = ctx->sc;
	struct smq_invoke_buf *list;
	struct smq_phy_page *pages;
	struct fastrpc_mmap *mmap;
	uint64_t *fdlist = NULL;
	uint32_t *crclist = NULL;

	remote_arg64_t *rpra = ctx->rpra;
	int i, inbufs, outbufs, handles;
	int err = 0;

	inbufs = REMOTE_SCALARS_INBUFS(sc);
	outbufs = REMOTE_SCALARS_OUTBUFS(sc);
	handles = REMOTE_SCALARS_INHANDLES(sc) + REMOTE_SCALARS_OUTHANDLES(sc);
	list = smq_invoke_buf_start(ctx->rpra, sc);
	pages = smq_phy_page_start(sc, list);
	if (!me->legacy) {
		fdlist = (uint64_t *)(pages + inbufs + outbufs + handles);
		crclist = (uint32_t *)(fdlist + M_FDLIST);
	}

	for (i = inbufs; i < inbufs + outbufs; ++i) {
		if (!ctx->maps[i]) {
			K_COPY_TO_USER(err, kernel,
				ctx->lpra[i].buf.pv,
				uint64_to_ptr(rpra[i].buf.pv),
				rpra[i].buf.len);
			if (err)
				goto bail;
		} else {
			mutex_lock(&ctx->fl->fl_map_mutex);
			fastrpc_mmap_free(ctx->maps[i], 0);
			mutex_unlock(&ctx->fl->fl_map_mutex);
			ctx->maps[i] = NULL;
		}
	}
	mutex_lock(&ctx->fl->fl_map_mutex);
	if (fdlist && (inbufs + outbufs + handles)) {
		for (i = 0; i < M_FDLIST; i++) {
			if (!fdlist[i])
				break;
			if (!fastrpc_mmap_find(ctx->fl, (int)fdlist[i], 0, 0,
						0, 0, &mmap))
				fastrpc_mmap_free(mmap, 0);
		}
	}
	mutex_unlock(&ctx->fl->fl_map_mutex);
	if (ctx->crc && crclist && rpra)
		K_COPY_TO_USER(err, kernel, ctx->crc,
			crclist, M_CRCLIST*sizeof(uint32_t));

 bail:
	return err;
}

static void inv_args_pre(struct smq_invoke_ctx *ctx)
{
	int i, inbufs, outbufs;
	uint32_t sc = ctx->sc;
	remote_arg64_t *rpra = ctx->rpra;
	uintptr_t end;

	inbufs = REMOTE_SCALARS_INBUFS(sc);
	outbufs = REMOTE_SCALARS_OUTBUFS(sc);
	for (i = inbufs; i < inbufs + outbufs; ++i) {
		struct fastrpc_mmap *map = ctx->maps[i];

		if (map && map->uncached)
			continue;
		if (!rpra[i].buf.len)
			continue;
		if (ctx->fl->sctx->smmu.coherent &&
			!(map && (map->attr & FASTRPC_ATTR_NON_COHERENT)))
			continue;
		if (map && (map->attr & FASTRPC_ATTR_COHERENT))
			continue;

		if (buf_page_start(ptr_to_uint64((void *)rpra)) ==
				buf_page_start(rpra[i].buf.pv))
			continue;
		if (!IS_CACHE_ALIGNED((uintptr_t)
				uint64_to_ptr(rpra[i].buf.pv))) {
			if (map && map->handle)
				msm_ion_do_cache_op(ctx->fl->apps->client,
					map->handle,
					uint64_to_ptr(rpra[i].buf.pv),
					sizeof(uintptr_t),
					ION_IOC_CLEAN_INV_CACHES);
			else
				dmac_flush_range(
					uint64_to_ptr(rpra[i].buf.pv), (char *)
					uint64_to_ptr(rpra[i].buf.pv + 1));
		}

		end = (uintptr_t)uint64_to_ptr(rpra[i].buf.pv +
							rpra[i].buf.len);
		if (!IS_CACHE_ALIGNED(end)) {
			if (map && map->handle)
				msm_ion_do_cache_op(ctx->fl->apps->client,
						map->handle,
						uint64_to_ptr(end),
						sizeof(uintptr_t),
						ION_IOC_CLEAN_INV_CACHES);
			else
				dmac_flush_range((char *)end,
					(char *)end + 1);
		}
	}
}

static void inv_args(struct smq_invoke_ctx *ctx)
{
	int i, inbufs, outbufs;
	uint32_t sc = ctx->sc;
	remote_arg64_t *rpra = ctx->rpra;

	inbufs = REMOTE_SCALARS_INBUFS(sc);
	outbufs = REMOTE_SCALARS_OUTBUFS(sc);
	for (i = inbufs; i < inbufs + outbufs; ++i) {
		struct fastrpc_mmap *map = ctx->maps[i];

		if (map && map->uncached)
			continue;
		if (!rpra[i].buf.len)
			continue;
		if (ctx->fl->sctx->smmu.coherent &&
			!(map && (map->attr & FASTRPC_ATTR_NON_COHERENT)))
			continue;
		if (map && (map->attr & FASTRPC_ATTR_COHERENT))
			continue;

		if (buf_page_start(ptr_to_uint64((void *)rpra)) ==
				buf_page_start(rpra[i].buf.pv)) {
			continue;
		}
		if (map && map->handle)
			msm_ion_do_cache_op(ctx->fl->apps->client, map->handle,
				(char *)uint64_to_ptr(rpra[i].buf.pv),
				rpra[i].buf.len, ION_IOC_INV_CACHES);
		else
			dmac_inv_range((char *)uint64_to_ptr(rpra[i].buf.pv),
				(char *)uint64_to_ptr(rpra[i].buf.pv
						 + rpra[i].buf.len));
	}

}

static int fastrpc_invoke_send(struct smq_invoke_ctx *ctx,
			       uint32_t kernel, uint32_t handle)
{
	struct smq_msg *msg = &ctx->msg;
	struct fastrpc_file *fl = ctx->fl;
	struct fastrpc_channel_ctx *channel_ctx = &fl->apps->channel[fl->cid];
	int err = 0, len;

	VERIFY(err, NULL != channel_ctx->chan);
	if (err)
		goto bail;
	msg->pid = fl->tgid;
	msg->tid = current->pid;
	if (fl->sessionid)
		msg->tid |= (1 << SESSION_ID_INDEX);
	if (kernel)
		msg->pid = 0;
	msg->invoke.header.ctx = ctx->ctxid | fl->pd;
	msg->invoke.header.handle = handle;
	msg->invoke.header.sc = ctx->sc;
	msg->invoke.page.addr = ctx->buf ? ctx->buf->phys : 0;
	msg->invoke.page.size = buf_page_size(ctx->used);
	if (fl->apps->glink) {
		if (fl->ssrcount != channel_ctx->ssrcount) {
			err = -ECONNRESET;
			goto bail;
		}
		VERIFY(err, channel_ctx->link.port_state ==
			FASTRPC_LINK_CONNECTED);
		if (err)
			goto bail;
		err = glink_tx(channel_ctx->chan,
			(void *)&fl->apps->channel[fl->cid], msg, sizeof(*msg),
			GLINK_TX_REQ_INTENT);
	} else {
		spin_lock(&fl->apps->hlock);
		len = smd_write((smd_channel_t *)
				channel_ctx->chan,
				msg, sizeof(*msg));
		spin_unlock(&fl->apps->hlock);
		VERIFY(err, len == sizeof(*msg));
	}
 bail:
	return err;
}

static void fastrpc_smd_read_handler(int cid)
{
	struct fastrpc_apps *me = &gfa;
	struct smq_invoke_rsp rsp = {0};
	int ret = 0, err = 0;
	uint32_t index;

	do {
		ret = smd_read_from_cb(me->channel[cid].chan, &rsp,
					sizeof(rsp));
		if (ret != sizeof(rsp))
			break;

		index = (uint32_t)((rsp.ctx & FASTRPC_CTXID_MASK) >> 4);
		VERIFY(err, index < FASTRPC_CTX_MAX);
		if (err)
			goto bail;

		VERIFY(err, !IS_ERR_OR_NULL(me->ctxtable[index]));
		if (err)
			goto bail;

		VERIFY(err, ((me->ctxtable[index]->ctxid == (rsp.ctx & ~3)) &&
			me->ctxtable[index]->magic == FASTRPC_CTX_MAGIC));
		if (err)
			goto bail;

		context_notify_user(me->ctxtable[index], rsp.retval);
	} while (ret == sizeof(rsp));
bail:
	if (err)
		pr_err("adsprpc: invalid response or context\n");

}

static void smd_event_handler(void *priv, unsigned int event)
{
	struct fastrpc_apps *me = &gfa;
	int cid = (int)(uintptr_t)priv;

	switch (event) {
	case SMD_EVENT_OPEN:
		complete(&me->channel[cid].workport);
		break;
	case SMD_EVENT_CLOSE:
		fastrpc_notify_drivers(me, cid);
		break;
	case SMD_EVENT_DATA:
		fastrpc_smd_read_handler(cid);
		break;
	}
}


static void fastrpc_init(struct fastrpc_apps *me)
{
	int i;

	INIT_HLIST_HEAD(&me->drivers);
	INIT_HLIST_HEAD(&me->maps);
	spin_lock_init(&me->hlock);
	spin_lock_init(&me->ctxlock);
	mutex_init(&me->smd_mutex);
	me->channel = &gcinfo[0];
	for (i = 0; i < NUM_CHANNELS; i++) {
		init_completion(&me->channel[i].work);
		init_completion(&me->channel[i].workport);
		me->channel[i].sesscount = 0;
	}
}

static int fastrpc_release_current_dsp_process(struct fastrpc_file *fl);

static int fastrpc_internal_invoke(struct fastrpc_file *fl, uint32_t mode,
				   uint32_t kernel,
				   struct fastrpc_ioctl_invoke_crc *inv)
{
	struct smq_invoke_ctx *ctx = NULL;
	struct fastrpc_ioctl_invoke *invoke = &inv->inv;
	int cid = fl->cid;
	int interrupted = 0;
	int err = 0;
	struct timespec invoket = {0};
	int64_t *perf_counter = getperfcounter(fl, PERF_COUNT);

	if (fl->profile)
		getnstimeofday(&invoket);


	VERIFY(err, fl->sctx != NULL);
	if (err)
		goto bail;
	VERIFY(err, fl->cid >= 0 && fl->cid < NUM_CHANNELS);
	if (err)
		goto bail;

	if (!kernel) {
		VERIFY(err, 0 == context_restore_interrupted(fl, inv,
								&ctx));
		if (err)
			goto bail;
		if (fl->sctx->smmu.faults)
			err = FASTRPC_ENOSUCH;
		if (err)
			goto bail;
		if (ctx)
			goto wait;
	}

	VERIFY(err, 0 == context_alloc(fl, kernel, inv, &ctx));
	if (err)
		goto bail;

	if (REMOTE_SCALARS_LENGTH(ctx->sc)) {
		PERF(fl->profile, GET_COUNTER(perf_counter, PERF_GETARGS),
		VERIFY(err, 0 == get_args(kernel, ctx));
		PERF_END);
		if (err)
			goto bail;
	}

	if (!fl->sctx->smmu.coherent) {
		PERF(fl->profile, GET_COUNTER(perf_counter, PERF_INVARGS),
		inv_args_pre(ctx);
		PERF_END);
	}

	PERF(fl->profile, GET_COUNTER(perf_counter, PERF_LINK),
	VERIFY(err, 0 == fastrpc_invoke_send(ctx, kernel, invoke->handle));
	PERF_END);

	if (err)
		goto bail;
 wait:
	if (kernel)
		wait_for_completion(&ctx->work);
	else {
		interrupted = wait_for_completion_interruptible(&ctx->work);
		VERIFY(err, 0 == (err = interrupted));
		if (err)
			goto bail;
	}

	PERF(fl->profile, GET_COUNTER(perf_counter, PERF_INVARGS),
	if (!fl->sctx->smmu.coherent)
		inv_args(ctx);
	PERF_END);

	VERIFY(err, 0 == (err = ctx->retval));
	if (err)
		goto bail;

	PERF(fl->profile, GET_COUNTER(perf_counter, PERF_PUTARGS),
	VERIFY(err, 0 == put_args(kernel, ctx, invoke->pra));
	PERF_END);
	if (err)
		goto bail;
 bail:
	if (ctx && interrupted == -ERESTARTSYS)
		context_save_interrupted(ctx);
	else if (ctx)
		context_free(ctx);
	if (fl->ssrcount != fl->apps->channel[cid].ssrcount)
		err = ECONNRESET;

	if (fl->profile && !interrupted) {
		if (invoke->handle != FASTRPC_STATIC_HANDLE_LISTENER) {
			int64_t *count = GET_COUNTER(perf_counter, PERF_INVOKE);

			if (count)
				*count += getnstimediff(&invoket);
		}
		if (invoke->handle > FASTRPC_STATIC_HANDLE_MAX) {
			int64_t *count = GET_COUNTER(perf_counter, PERF_COUNT);

			if (count)
				*count = *count+1;
		}
	}
	return err;
}

static int fastrpc_get_adsp_session(char *name, int *session)
{
	struct fastrpc_apps *me = &gfa;
	int err = 0, i;

	for (i = 0; i < NUM_SESSIONS; i++) {
		if (!me->channel[0].spd[i].spdname)
			continue;
		if (!strcmp(name, me->channel[0].spd[i].spdname))
			break;
	}
	VERIFY(err, i < NUM_SESSIONS);
	if (err)
		goto bail;
	*session = i;
bail:
	return err;
}

static int fastrpc_mmap_remove_pdr(struct fastrpc_file *fl);
static int fastrpc_channel_open(struct fastrpc_file *fl);
static int fastrpc_mmap_remove_ssr(struct fastrpc_file *fl);
static int fastrpc_init_process(struct fastrpc_file *fl,
				struct fastrpc_ioctl_init_attrs *uproc)
{
	int err = 0;
	struct fastrpc_apps *me = &gfa;
	struct fastrpc_ioctl_invoke_crc ioctl;
	struct fastrpc_ioctl_init *init = &uproc->init;
	struct smq_phy_page pages[1];
	struct fastrpc_mmap *file = NULL, *mem = NULL;
	char *proc_name = NULL;

	VERIFY(err, 0 == (err = fastrpc_channel_open(fl)));
	if (err)
		goto bail;
	if (init->flags == FASTRPC_INIT_ATTACH ||
			init->flags == FASTRPC_INIT_ATTACH_SENSORS) {
		remote_arg_t ra[1];
		int tgid = fl->tgid;

		ra[0].buf.pv = (void *)&tgid;
		ra[0].buf.len = sizeof(tgid);
		ioctl.inv.handle = 1;
		ioctl.inv.sc = REMOTE_SCALARS_MAKE(0, 1, 0);
		ioctl.inv.pra = ra;
		ioctl.fds = NULL;
		ioctl.attrs = NULL;
		ioctl.crc = NULL;
		if (init->flags == FASTRPC_INIT_ATTACH)
			fl->pd = 0;
		else if (init->flags == FASTRPC_INIT_ATTACH_SENSORS) {
			fl->spdname = SENSORS_PDR_SERVICE_LOCATION_CLIENT_NAME;
			fl->pd = 2;
		}
		VERIFY(err, !(err = fastrpc_internal_invoke(fl,
			FASTRPC_MODE_PARALLEL, 1, &ioctl)));
		if (err)
			goto bail;
	} else if (init->flags == FASTRPC_INIT_CREATE) {
		remote_arg_t ra[6];
		int fds[6];
		int mflags = 0;
		struct {
			int pgid;
			unsigned int namelen;
			unsigned int filelen;
			unsigned int pageslen;
			int attrs;
			int siglen;
		} inbuf;

		inbuf.pgid = fl->tgid;
		inbuf.namelen = strlen(current->comm) + 1;
		inbuf.filelen = init->filelen;
		fl->pd = 1;

		VERIFY(err, access_ok(0, (void __user *)init->file,
			init->filelen));
		if (err)
			goto bail;
		if (init->filelen) {
			mutex_lock(&fl->fl_map_mutex);
			VERIFY(err, !fastrpc_mmap_create(fl, init->filefd, 0,
				init->file, init->filelen, mflags, &file));
			mutex_unlock(&fl->fl_map_mutex);
			if (err)
				goto bail;
		}
		inbuf.pageslen = 1;
		VERIFY(err, access_ok(1, (void __user *)init->mem,
			init->memlen));
		if (err)
			goto bail;
		mutex_lock(&fl->fl_map_mutex);
		VERIFY(err, !fastrpc_mmap_create(fl, init->memfd, 0,
				init->mem, init->memlen, mflags, &mem));
		mutex_unlock(&fl->fl_map_mutex);
		if (err)
			goto bail;
		inbuf.pageslen = 1;
		ra[0].buf.pv = (void *)&inbuf;
		ra[0].buf.len = sizeof(inbuf);
		fds[0] = 0;

		ra[1].buf.pv = (void *)current->comm;
		ra[1].buf.len = inbuf.namelen;
		fds[1] = 0;

		ra[2].buf.pv = (void *)init->file;
		ra[2].buf.len = inbuf.filelen;
		fds[2] = init->filefd;

		pages[0].addr = mem->phys;
		pages[0].size = mem->size;
		ra[3].buf.pv = (void *)pages;
		ra[3].buf.len = 1 * sizeof(*pages);
		fds[3] = 0;

		inbuf.attrs = uproc->attrs;
		ra[4].buf.pv = (void *)&(inbuf.attrs);
		ra[4].buf.len = sizeof(inbuf.attrs);
		fds[4] = 0;

		inbuf.siglen = uproc->siglen;
		ra[5].buf.pv = (void *)&(inbuf.siglen);
		ra[5].buf.len = sizeof(inbuf.siglen);
		fds[5] = 0;

		ioctl.inv.handle = 1;
		ioctl.inv.sc = REMOTE_SCALARS_MAKE(6, 4, 0);
		if (uproc->attrs)
			ioctl.inv.sc = REMOTE_SCALARS_MAKE(7, 6, 0);
		ioctl.inv.pra = ra;
		ioctl.fds = fds;
		ioctl.attrs = NULL;
		ioctl.crc = NULL;
		VERIFY(err, !(err = fastrpc_internal_invoke(fl,
			FASTRPC_MODE_PARALLEL, 1, &ioctl)));
		if (err)
			goto bail;
	} else if (init->flags == FASTRPC_INIT_CREATE_STATIC) {
		remote_arg_t ra[3];
		uint64_t phys = 0;
		size_t size = 0;
		int fds[3];
		struct {
			int pgid;
			unsigned int namelen;
			unsigned int pageslen;
		} inbuf;

		if (!init->filelen)
			goto bail;

		proc_name = kzalloc(init->filelen, GFP_KERNEL);
		VERIFY(err, !IS_ERR_OR_NULL(proc_name));
		if (err)
			goto bail;
		VERIFY(err, 0 == copy_from_user((void *)proc_name,
			(void __user *)init->file, init->filelen));
		if (err)
			goto bail;

		fl->pd = 1;
		inbuf.pgid = current->tgid;
		inbuf.namelen = init->filelen;
		inbuf.pageslen = 0;

		if (!strcmp(proc_name, "audiopd")) {
			fl->spdname = AUDIO_PDR_SERVICE_LOCATION_CLIENT_NAME;
			VERIFY(err, !fastrpc_mmap_remove_pdr(fl));
			if (err)
				goto bail;
		}

		if (!me->staticpd_flags) {
			inbuf.pageslen = 1;
			mutex_lock(&fl->fl_map_mutex);
			VERIFY(err, !fastrpc_mmap_create(fl, -1, 0, init->mem,
				 init->memlen, ADSP_MMAP_REMOTE_HEAP_ADDR,
				 &mem));
			mutex_unlock(&fl->fl_map_mutex);
			if (err)
				goto bail;
			phys = mem->phys;
			size = mem->size;
			VERIFY(err, !hyp_assign_phys(phys, (uint64_t)size,
				hlosvm, 1, me->channel[fl->cid].rhvm.vmid,
				me->channel[fl->cid].rhvm.vmperm,
				me->channel[fl->cid].rhvm.vmcount));
			if (err) {
				pr_err("ADSPRPC: hyp_assign_phys fail err %d",
							 err);
				pr_err("map->phys %llx, map->size %d\n",
							 phys, (int)size);
				goto bail;
			}
			me->staticpd_flags = 1;
		}

		ra[0].buf.pv = (void *)&inbuf;
		ra[0].buf.len = sizeof(inbuf);
		fds[0] = 0;

		ra[1].buf.pv = (void *)proc_name;
		ra[1].buf.len = inbuf.namelen;
		fds[1] = 0;

		pages[0].addr = phys;
		pages[0].size = size;

		ra[2].buf.pv = (void *)pages;
		ra[2].buf.len = sizeof(*pages);
		fds[2] = 0;
		ioctl.inv.handle = 1;

		ioctl.inv.sc = REMOTE_SCALARS_MAKE(8, 3, 0);
		ioctl.inv.pra = ra;
		ioctl.fds = NULL;
		ioctl.attrs = NULL;
		ioctl.crc = NULL;
		VERIFY(err, !(err = fastrpc_internal_invoke(fl,
			FASTRPC_MODE_PARALLEL, 1, &ioctl)));
		if (err)
			goto bail;
	} else {
		err = -ENOTTY;
	}
bail:
	kfree(proc_name);
	if (err && (init->flags == FASTRPC_INIT_CREATE_STATIC))
		me->staticpd_flags = 0;
	if (mem && err) {
		if (mem->flags == ADSP_MMAP_REMOTE_HEAP_ADDR)
			hyp_assign_phys(mem->phys, (uint64_t)mem->size,
					me->channel[fl->cid].rhvm.vmid,
					me->channel[fl->cid].rhvm.vmcount,
					hlosvm, hlosvmperm, 1);
		mutex_lock(&fl->fl_map_mutex);
		fastrpc_mmap_free(mem, 0);
		mutex_unlock(&fl->fl_map_mutex);
	}
	if (file) {
		mutex_lock(&fl->fl_map_mutex);
		fastrpc_mmap_free(file, 0);
		mutex_unlock(&fl->fl_map_mutex);
	}
	return err;
}

static int fastrpc_release_current_dsp_process(struct fastrpc_file *fl)
{
	int err = 0;
	struct fastrpc_ioctl_invoke_crc ioctl;
	remote_arg_t ra[1];
	int tgid = 0;

	VERIFY(err, fl->cid >= 0 && fl->cid < NUM_CHANNELS);
	if (err)
		goto bail;
	VERIFY(err, fl->apps->channel[fl->cid].chan != NULL);
	if (err)
		goto bail;
	tgid = fl->tgid;
	ra[0].buf.pv = (void *)&tgid;
	ra[0].buf.len = sizeof(tgid);
	ioctl.inv.handle = 1;
	ioctl.inv.sc = REMOTE_SCALARS_MAKE(1, 1, 0);
	ioctl.inv.pra = ra;
	ioctl.fds = NULL;
	ioctl.attrs = NULL;
	ioctl.crc = NULL;
	VERIFY(err, 0 == (err = fastrpc_internal_invoke(fl,
		FASTRPC_MODE_PARALLEL, 1, &ioctl)));
bail:
	return err;
}

static int fastrpc_mmap_on_dsp(struct fastrpc_file *fl, uint32_t flags,
			       struct fastrpc_mmap *map)
{
	struct fastrpc_ioctl_invoke_crc ioctl;
	struct fastrpc_apps *me = &gfa;
	struct smq_phy_page page;
	int num = 1;
	remote_arg_t ra[3];
	int err = 0;
	struct {
		int pid;
		uint32_t flags;
		uintptr_t vaddrin;
		int num;
	} inargs;
	struct {
		uintptr_t vaddrout;
	} routargs;

	inargs.pid = fl->tgid;
	inargs.vaddrin = (uintptr_t)map->va;
	inargs.flags = flags;
	inargs.num = fl->apps->compat ? num * sizeof(page) : num;
	ra[0].buf.pv = (void *)&inargs;
	ra[0].buf.len = sizeof(inargs);
	page.addr = map->phys;
	page.size = map->size;
	ra[1].buf.pv = (void *)&page;
	ra[1].buf.len = num * sizeof(page);

	ra[2].buf.pv = (void *)&routargs;
	ra[2].buf.len = sizeof(routargs);

	ioctl.inv.handle = 1;
	if (fl->apps->compat)
		ioctl.inv.sc = REMOTE_SCALARS_MAKE(4, 2, 1);
	else
		ioctl.inv.sc = REMOTE_SCALARS_MAKE(2, 2, 1);
	ioctl.inv.pra = ra;
	ioctl.fds = NULL;
	ioctl.attrs = NULL;
	ioctl.crc = NULL;
	VERIFY(err, 0 == (err = fastrpc_internal_invoke(fl,
		FASTRPC_MODE_PARALLEL, 1, &ioctl)));
	map->raddr = (uintptr_t)routargs.vaddrout;
	if (err)
		goto bail;
	if (flags == ADSP_MMAP_HEAP_ADDR) {
		struct scm_desc desc = {0};

		desc.args[0] = TZ_PIL_AUTH_QDSP6_PROC;
		desc.args[1] = map->phys;
		desc.args[2] = map->size;
		desc.arginfo = SCM_ARGS(3);
		err = scm_call2(SCM_SIP_FNID(SCM_SVC_PIL,
			TZ_PIL_PROTECT_MEM_SUBSYS_ID), &desc);
	} else if (flags == ADSP_MMAP_REMOTE_HEAP_ADDR) {
		VERIFY(err, !hyp_assign_phys(map->phys, (uint64_t)map->size,
				hlosvm, 1, me->channel[fl->cid].rhvm.vmid,
				me->channel[fl->cid].rhvm.vmperm,
				me->channel[fl->cid].rhvm.vmcount));
		if (err)
			goto bail;
	}
bail:
	return err;
}

static int fastrpc_munmap_on_dsp_rh(struct fastrpc_file *fl,
				 struct fastrpc_mmap *map)
{
	int err = 0;
	struct fastrpc_apps *me = &gfa;
	int destVM[1] = {VMID_HLOS};
	int destVMperm[1] = {PERM_READ | PERM_WRITE | PERM_EXEC};

	if (map->flags == ADSP_MMAP_HEAP_ADDR) {
		struct fastrpc_ioctl_invoke_crc ioctl;
		struct scm_desc desc = {0};
		remote_arg_t ra[1];
		int err = 0;
		struct {
			uint8_t skey;
		} routargs;

		ra[0].buf.pv = (void *)&routargs;
		ra[0].buf.len = sizeof(routargs);

		ioctl.inv.handle = 1;
		ioctl.inv.sc = REMOTE_SCALARS_MAKE(7, 0, 1);
		ioctl.inv.pra = ra;
		ioctl.fds = NULL;
		ioctl.attrs = NULL;
		ioctl.crc = NULL;
		if (fl == NULL)
			goto bail;

		VERIFY(err, 0 == (err = fastrpc_internal_invoke(fl,
				FASTRPC_MODE_PARALLEL, 1, &ioctl)));
		if (err)
			goto bail;
		desc.args[0] = TZ_PIL_AUTH_QDSP6_PROC;
		desc.args[1] = map->phys;
		desc.args[2] = map->size;
		desc.args[3] = routargs.skey;
		desc.arginfo = SCM_ARGS(4);
		err = scm_call2(SCM_SIP_FNID(SCM_SVC_PIL,
			TZ_PIL_CLEAR_PROTECT_MEM_SUBSYS_ID), &desc);
	} else if (map->flags == ADSP_MMAP_REMOTE_HEAP_ADDR) {
		VERIFY(err, !hyp_assign_phys(map->phys, (uint64_t)map->size,
					me->channel[fl->cid].rhvm.vmid,
					me->channel[fl->cid].rhvm.vmcount,
					destVM, destVMperm, 1));
		if (err)
			goto bail;
	}

bail:
	return err;
}

static int fastrpc_munmap_on_dsp(struct fastrpc_file *fl,
				 struct fastrpc_mmap *map)
{
	struct fastrpc_ioctl_invoke_crc ioctl;
	remote_arg_t ra[1];
	int err = 0;
	struct {
		int pid;
		uintptr_t vaddrout;
		size_t size;
	} inargs;

	inargs.pid = fl->tgid;
	inargs.size = map->size;
	inargs.vaddrout = map->raddr;
	ra[0].buf.pv = (void *)&inargs;
	ra[0].buf.len = sizeof(inargs);

	ioctl.inv.handle = 1;
	if (fl->apps->compat)
		ioctl.inv.sc = REMOTE_SCALARS_MAKE(5, 1, 0);
	else
		ioctl.inv.sc = REMOTE_SCALARS_MAKE(3, 1, 0);
	ioctl.inv.pra = ra;
	ioctl.fds = NULL;
	ioctl.attrs = NULL;
	ioctl.crc = NULL;
	VERIFY(err, 0 == (err = fastrpc_internal_invoke(fl,
		FASTRPC_MODE_PARALLEL, 1, &ioctl)));
	if (err)
		goto bail;
	if (map->flags == ADSP_MMAP_HEAP_ADDR ||
				map->flags == ADSP_MMAP_REMOTE_HEAP_ADDR) {
		VERIFY(err, !fastrpc_munmap_on_dsp_rh(fl, map));
		if (err)
			goto bail;
	}
bail:
	return err;
}

static int fastrpc_mmap_remove_ssr(struct fastrpc_file *fl)
{
	struct fastrpc_mmap *match = NULL, *map = NULL;
	struct hlist_node *n = NULL;
	int err = 0, ret = 0;
	struct fastrpc_apps *me = &gfa;
	struct ramdump_segment *ramdump_segments_rh = NULL;

	do {
		match = NULL;
		spin_lock(&me->hlock);
		hlist_for_each_entry_safe(map, n, &me->maps, hn) {
			match = map;
			hlist_del_init(&map->hn);
			break;
		}
		spin_unlock(&me->hlock);

		if (match) {
			VERIFY(err, !fastrpc_munmap_on_dsp_rh(fl, match));
			if (err)
				goto bail;
			if (me->channel[0].ramdumpenabled) {
				ramdump_segments_rh = kcalloc(1,
				sizeof(struct ramdump_segment), GFP_KERNEL);
				if (ramdump_segments_rh) {
					ramdump_segments_rh->address =
					match->phys;
					ramdump_segments_rh->size = match->size;
					ret = do_elf_ramdump(
					 me->channel[0].remoteheap_ramdump_dev,
					 ramdump_segments_rh, 1);
					if (ret < 0)
						pr_err("ADSPRPC: unable to dump heap");
					kfree(ramdump_segments_rh);
				}
			}
			fastrpc_mmap_free(match, 0);
		}
	} while (match);
bail:
	if (err && match)
		fastrpc_mmap_add(match);
	return err;
}

static int fastrpc_mmap_remove_pdr(struct fastrpc_file *fl)
{
	struct fastrpc_apps *me = &gfa;
	int session = 0, err = 0;

	VERIFY(err, !fastrpc_get_adsp_session(
			AUDIO_PDR_SERVICE_LOCATION_CLIENT_NAME, &session));
	if (err)
		goto bail;
	if (me->channel[fl->cid].spd[session].pdrcount !=
		me->channel[fl->cid].spd[session].prevpdrcount) {
		if (fastrpc_mmap_remove_ssr(fl))
			pr_err("ADSPRPC: SSR: Failed to unmap remote heap\n");
		me->channel[fl->cid].spd[session].prevpdrcount =
				me->channel[fl->cid].spd[session].pdrcount;
	}
	if (!me->channel[fl->cid].spd[session].ispdup) {
		VERIFY(err, 0);
		if (err) {
			err = -ENOTCONN;
			goto bail;
		}
	}
bail:
	return err;
}

static int fastrpc_mmap_remove(struct fastrpc_file *fl, uintptr_t va,
			     size_t len, struct fastrpc_mmap **ppmap);

static void fastrpc_mmap_add(struct fastrpc_mmap *map);

static int fastrpc_internal_munmap(struct fastrpc_file *fl,
				   struct fastrpc_ioctl_munmap *ud)
{
	int err = 0;
	struct fastrpc_mmap *map = NULL;

	mutex_lock(&fl->map_mutex);
	mutex_lock(&fl->fl_map_mutex);
	VERIFY(err, !fastrpc_mmap_remove(fl, ud->vaddrout, ud->size, &map));
	mutex_unlock(&fl->fl_map_mutex);
	if (err)
		goto bail;
	VERIFY(err, !fastrpc_munmap_on_dsp(fl, map));
	if (err)
		goto bail;
	mutex_lock(&fl->fl_map_mutex);
	fastrpc_mmap_free(map, 0);
	mutex_unlock(&fl->fl_map_mutex);
bail:
	if (err && map) {
		mutex_lock(&fl->fl_map_mutex);
		fastrpc_mmap_add(map);
		mutex_unlock(&fl->fl_map_mutex);
	}
	mutex_unlock(&fl->map_mutex);
	return err;
}

static int fastrpc_internal_munmap_fd(struct fastrpc_file *fl,
					struct fastrpc_ioctl_munmap_fd *ud) {
	int err = 0;
	struct fastrpc_mmap *map = NULL;

	VERIFY(err, (fl && ud));
	if (err)
		goto bail;
	mutex_lock(&fl->fl_map_mutex);
	if (fastrpc_mmap_find(fl, ud->fd, ud->va, ud->len, 0, 0, &map)) {
		pr_err("adsprpc: mapping not found to unmap %d va %llx %x\n",
			ud->fd, (unsigned long long)ud->va,
			(unsigned int)ud->len);
		err = -1;
		mutex_unlock(&fl->fl_map_mutex);
		goto bail;
	}
	if (map)
		fastrpc_mmap_free(map, 0);
	mutex_unlock(&fl->fl_map_mutex);
bail:
	return err;
}


static int fastrpc_internal_mmap(struct fastrpc_file *fl,
				 struct fastrpc_ioctl_mmap *ud)
{

	struct fastrpc_mmap *map = NULL;
	int err = 0;

	mutex_lock(&fl->map_mutex);
	mutex_lock(&fl->fl_map_mutex);
	if (!fastrpc_mmap_find(fl, ud->fd, (uintptr_t)ud->vaddrin,
			 ud->size, ud->flags, 1, &map)) {
		mutex_unlock(&fl->fl_map_mutex);
		mutex_unlock(&fl->map_mutex);
		return 0;
	}
	VERIFY(err, !fastrpc_mmap_create(fl, ud->fd, 0,
			(uintptr_t)ud->vaddrin, ud->size,
			 ud->flags, &map));
	mutex_unlock(&fl->fl_map_mutex);
	if (err)
		goto bail;
	VERIFY(err, 0 == fastrpc_mmap_on_dsp(fl, ud->flags, map));
	if (err)
		goto bail;
	ud->vaddrout = map->raddr;
 bail:
	if (err && map) {
		mutex_lock(&fl->fl_map_mutex);
		fastrpc_mmap_free(map, 0);
		mutex_unlock(&fl->fl_map_mutex);
	}
	mutex_unlock(&fl->map_mutex);
	return err;
}

static void fastrpc_channel_close(struct kref *kref)
{
	struct fastrpc_apps *me = &gfa;
	struct fastrpc_channel_ctx *ctx;
	int cid;

	ctx = container_of(kref, struct fastrpc_channel_ctx, kref);
	cid = ctx - &gcinfo[0];
	if (!me->glink)
		smd_close(ctx->chan);
	else
		fastrpc_glink_close(ctx->chan, cid);
	ctx->chan = NULL;
	mutex_unlock(&me->smd_mutex);
	pr_info("'closed /dev/%s c %d %d'\n", gcinfo[cid].name,
						MAJOR(me->dev_no), cid);
}

static void fastrpc_context_list_dtor(struct fastrpc_file *fl);

static int fastrpc_session_alloc_locked(struct fastrpc_channel_ctx *chan,
	int secure, int sharedcb, struct fastrpc_session_ctx **session)
{
	struct fastrpc_apps *me = &gfa;
	int idx = 0, err = 0;

	if (chan->sesscount) {
		for (idx = 0; idx < chan->sesscount; ++idx) {
			if ((sharedcb && chan->session[idx].smmu.sharedcb) ||
					(!chan->session[idx].used &&
					chan->session[idx].smmu.secure
					== secure && !sharedcb)) {
				chan->session[idx].used = 1;
				break;
			}
		}
		VERIFY(err, idx < chan->sesscount);
		if (err)
			goto bail;
		chan->session[idx].smmu.faults = 0;
	} else {
		VERIFY(err, me->dev != NULL);
		if (err)
			goto bail;
		chan->session[0].dev = me->dev;
		chan->session[0].smmu.dev = me->dev;
	}

	*session = &chan->session[idx];
 bail:
	return err;
}

static bool fastrpc_glink_notify_rx_intent_req(void *h, const void *priv,
						size_t size)
{
	if (glink_queue_rx_intent(h, NULL, size))
		return false;
	return true;
}

static void fastrpc_glink_notify_tx_done(void *handle, const void *priv,
		const void *pkt_priv, const void *ptr)
{
}

static void fastrpc_glink_notify_rx(void *handle, const void *priv,
	const void *pkt_priv, const void *ptr, size_t size)
{
	struct smq_invoke_rsp *rsp = (struct smq_invoke_rsp *)ptr;
	struct fastrpc_apps *me = &gfa;
	uint32_t index;
	int err = 0;

	VERIFY(err, (rsp && size >= sizeof(*rsp)));
	if (err)
		goto bail;

	index = (uint32_t)((rsp->ctx & FASTRPC_CTXID_MASK) >> 4);
	VERIFY(err, index < FASTRPC_CTX_MAX);
	if (err)
		goto bail;

	VERIFY(err, !IS_ERR_OR_NULL(me->ctxtable[index]));
	if (err)
		goto bail;

	VERIFY(err, ((me->ctxtable[index]->ctxid == (rsp->ctx & ~3)) &&
		me->ctxtable[index]->magic == FASTRPC_CTX_MAGIC));
	if (err)
		goto bail;

	context_notify_user(me->ctxtable[index], rsp->retval);
bail:
	if (err)
		pr_err("adsprpc: invalid response or context\n");
	glink_rx_done(handle, ptr, true);
}

static void fastrpc_glink_notify_state(void *handle, const void *priv,
				unsigned int event)
{
	struct fastrpc_apps *me = &gfa;
	int cid = (int)(uintptr_t)priv;
	struct fastrpc_glink_info *link;

	if (cid < 0 || cid >= NUM_CHANNELS)
		return;
	link = &me->channel[cid].link;
	switch (event) {
	case GLINK_CONNECTED:
		link->port_state = FASTRPC_LINK_CONNECTED;
		complete(&me->channel[cid].workport);
		break;
	case GLINK_LOCAL_DISCONNECTED:
		link->port_state = FASTRPC_LINK_DISCONNECTED;
		break;
	case GLINK_REMOTE_DISCONNECTED:
		break;
	default:
		break;
	}
}

static int fastrpc_session_alloc(struct fastrpc_channel_ctx *chan, int secure,
					struct fastrpc_session_ctx **session)
{
	int err = 0;
	struct fastrpc_apps *me = &gfa;

	mutex_lock(&me->smd_mutex);
	if (!*session)
		err = fastrpc_session_alloc_locked(chan, secure, 0, session);
	mutex_unlock(&me->smd_mutex);
	return err;
}

static void fastrpc_session_free(struct fastrpc_channel_ctx *chan,
				struct fastrpc_session_ctx *session)
{
	struct fastrpc_apps *me = &gfa;

	mutex_lock(&me->smd_mutex);
	session->used = 0;
	mutex_unlock(&me->smd_mutex);
}

static int fastrpc_file_free(struct fastrpc_file *fl)
{
	struct hlist_node *n = NULL;
	struct fastrpc_mmap *map = NULL, *lmap = NULL;
	struct fastrpc_perf *perf = NULL, *fperf = NULL;
	int cid;

	if (!fl)
		return 0;
	cid = fl->cid;

	(void)fastrpc_release_current_dsp_process(fl);

	spin_lock(&fl->apps->hlock);
	hlist_del_init(&fl->hn);
	spin_unlock(&fl->apps->hlock);

	if (!fl->sctx) {
		kfree(fl);
		return 0;
	}
	spin_lock(&fl->hlock);
	fl->file_close = 1;
	spin_unlock(&fl->hlock);
	fastrpc_context_list_dtor(fl);
	fastrpc_buf_list_free(fl);
	mutex_lock(&fl->fl_map_mutex);
	do {
		lmap = NULL;
		hlist_for_each_entry_safe(map, n, &fl->maps, hn) {
			hlist_del_init(&map->hn);
			lmap = map;
			break;
		}
		fastrpc_mmap_free(lmap, 1);
	} while (lmap);
	mutex_unlock(&fl->fl_map_mutex);
	if (fl->refcount && (fl->ssrcount == fl->apps->channel[cid].ssrcount))
		kref_put_mutex(&fl->apps->channel[cid].kref,
				fastrpc_channel_close, &fl->apps->smd_mutex);
	if (fl->sctx)
		fastrpc_session_free(&fl->apps->channel[cid], fl->sctx);
	if (fl->secsctx)
		fastrpc_session_free(&fl->apps->channel[cid], fl->secsctx);

	mutex_lock(&fl->perf_mutex);
	do {
		struct hlist_node *pn = NULL;

		fperf = NULL;
		hlist_for_each_entry_safe(perf, pn, &fl->perf, hn) {
			hlist_del_init(&perf->hn);
			fperf = perf;
			break;
		}
		kfree(fperf);
	} while (fperf);
	mutex_unlock(&fl->perf_mutex);
	mutex_destroy(&fl->perf_mutex);
	mutex_destroy(&fl->fl_map_mutex);
	mutex_destroy(&fl->map_mutex);
	kfree(fl);
	return 0;
}

static int fastrpc_device_release(struct inode *inode, struct file *file)
{
	struct fastrpc_file *fl = (struct fastrpc_file *)file->private_data;

	if (fl) {
		if (fl->qos_request && pm_qos_request_active(&fl->pm_qos_req))
			pm_qos_remove_request(&fl->pm_qos_req);
		if (fl->debugfs_file != NULL)
			debugfs_remove(fl->debugfs_file);
		fastrpc_file_free(fl);
		file->private_data = NULL;
	}
	return 0;
}

static void fastrpc_link_state_handler(struct glink_link_state_cb_info *cb_info,
					 void *priv)
{
	struct fastrpc_apps *me = &gfa;
	int cid = (int)((uintptr_t)priv);
	struct fastrpc_glink_info *link;

	if (cid < 0 || cid >= NUM_CHANNELS)
		return;

	link = &me->channel[cid].link;
	switch (cb_info->link_state) {
	case GLINK_LINK_STATE_UP:
		link->link_state = FASTRPC_LINK_STATE_UP;
		complete(&me->channel[cid].work);
		break;
	case GLINK_LINK_STATE_DOWN:
		link->link_state = FASTRPC_LINK_STATE_DOWN;
		break;
	default:
		pr_err("adsprpc: unknown link state %d\n", cb_info->link_state);
		break;
	}
}

static int fastrpc_glink_register(int cid, struct fastrpc_apps *me)
{
	int err = 0;
	struct fastrpc_glink_info *link;

	VERIFY(err, (cid >= 0 && cid < NUM_CHANNELS));
	if (err)
		goto bail;

	link = &me->channel[cid].link;
	if (link->link_notify_handle != NULL)
		goto bail;

	link->link_info.glink_link_state_notif_cb = fastrpc_link_state_handler;
	link->link_notify_handle = glink_register_link_state_cb(
					&link->link_info,
					(void *)((uintptr_t)cid));
	VERIFY(err, !IS_ERR_OR_NULL(me->channel[cid].link.link_notify_handle));
	if (err) {
		link->link_notify_handle = NULL;
		goto bail;
	}
	VERIFY(err, wait_for_completion_timeout(&me->channel[cid].work,
			RPC_TIMEOUT));
bail:
	return err;
}

static void fastrpc_glink_close(void *chan, int cid)
{
	int err = 0;
	struct fastrpc_glink_info *link;

	VERIFY(err, (cid >= 0 && cid < NUM_CHANNELS));
	if (err)
		return;
	link = &gfa.channel[cid].link;

	if (link->port_state == FASTRPC_LINK_CONNECTED ||
		link->port_state == FASTRPC_LINK_REMOTE_DISCONNECTING) {
		link->port_state = FASTRPC_LINK_DISCONNECTING;
		glink_close(chan);
	}
}

static int fastrpc_glink_open(int cid)
{
	int err = 0;
	void *handle = NULL;
	struct fastrpc_apps *me = &gfa;
	struct glink_open_config *cfg;
	struct fastrpc_glink_info *link;

	VERIFY(err, (cid >= 0 && cid < NUM_CHANNELS));
	if (err)
		goto bail;
	link = &me->channel[cid].link;
	cfg = &me->channel[cid].link.cfg;
	VERIFY(err, (link->link_state == FASTRPC_LINK_STATE_UP));
	if (err)
		goto bail;

	VERIFY(err, (link->port_state == FASTRPC_LINK_DISCONNECTED));
	if (err)
		goto bail;

	link->port_state = FASTRPC_LINK_CONNECTING;
	cfg->priv = (void *)(uintptr_t)cid;
	cfg->edge = gcinfo[cid].link.link_info.edge;
	cfg->transport = gcinfo[cid].link.link_info.transport;
	cfg->name = FASTRPC_GLINK_GUID;
	cfg->notify_rx = fastrpc_glink_notify_rx;
	cfg->notify_tx_done = fastrpc_glink_notify_tx_done;
	cfg->notify_state = fastrpc_glink_notify_state;
	cfg->notify_rx_intent_req = fastrpc_glink_notify_rx_intent_req;
	handle = glink_open(cfg);
	VERIFY(err, !IS_ERR_OR_NULL(handle));
	if (err) {
		if (link->port_state == FASTRPC_LINK_CONNECTING)
			link->port_state = FASTRPC_LINK_DISCONNECTED;
		goto bail;
	}
	me->channel[cid].chan = handle;
bail:
	return err;
}

static int fastrpc_debugfs_open(struct inode *inode, struct file *filp)
{
	filp->private_data = inode->i_private;
	return 0;
}

static ssize_t fastrpc_debugfs_read(struct file *filp, char __user *buffer,
					 size_t count, loff_t *position)
{
	struct fastrpc_file *fl = filp->private_data;
	struct hlist_node *n;
	struct fastrpc_buf *buf = NULL;
	struct fastrpc_mmap *map = NULL;
	struct smq_invoke_ctx *ictx = NULL;
	struct fastrpc_channel_ctx *chan;
	struct fastrpc_session_ctx *sess;
	unsigned int len = 0;
	int i, j, ret = 0;
	char *fileinfo = NULL;

	fileinfo = kzalloc(DEBUGFS_SIZE, GFP_KERNEL);
	if (!fileinfo)
		goto bail;
	if (fl == NULL) {
		for (i = 0; i < NUM_CHANNELS; i++) {
			chan = &gcinfo[i];
			len += scnprintf(fileinfo + len,
					DEBUGFS_SIZE - len, "%s\n\n",
					chan->name);
			len += scnprintf(fileinfo + len,
					DEBUGFS_SIZE - len, "%s %d\n",
					"sesscount:", chan->sesscount);
			for (j = 0; j < chan->sesscount; j++) {
				sess = &chan->session[j];
				len += scnprintf(fileinfo + len,
						DEBUGFS_SIZE - len,
						"%s%d\n\n", "SESSION", j);
				len += scnprintf(fileinfo + len,
						DEBUGFS_SIZE - len,
						"%s %d\n", "sid:",
						sess->smmu.cb);
				len += scnprintf(fileinfo + len,
						DEBUGFS_SIZE - len,
						"%s %d\n", "SECURE:",
						sess->smmu.secure);
			}
		}
	} else {
		len += scnprintf(fileinfo + len, DEBUGFS_SIZE - len,
				"%s %d\n\n",
				"PROCESS_ID:", fl->tgid);
		len += scnprintf(fileinfo + len, DEBUGFS_SIZE - len,
				"%s %d\n\n",
				"CHANNEL_ID:", fl->cid);
		len += scnprintf(fileinfo + len, DEBUGFS_SIZE - len,
				"%s %d\n\n",
				"SSRCOUNT:", fl->ssrcount);
		len += scnprintf(fileinfo + len, DEBUGFS_SIZE - len,
				"%s\n",
				"LIST OF BUFS:");
		spin_lock(&fl->hlock);
		hlist_for_each_entry_safe(buf, n, &fl->bufs, hn) {
			len += scnprintf(fileinfo + len, DEBUGFS_SIZE - len,
					"%s %pK %s %pK %s %llx\n", "buf:",
					buf, "buf->virt:", buf->virt,
					"buf->phys:", buf->phys);
		}
		len += scnprintf(fileinfo + len, DEBUGFS_SIZE - len,
					"\n%s\n",
					"LIST OF MAPS:");
		hlist_for_each_entry_safe(map, n, &fl->maps, hn) {
			len += scnprintf(fileinfo + len, DEBUGFS_SIZE - len,
						"%s %pK %s %lx %s %llx\n",
						"map:", map,
						"map->va:", map->va,
						"map->phys:", map->phys);
		}
		len += scnprintf(fileinfo + len, DEBUGFS_SIZE - len,
					"\n%s\n",
					"LIST OF PENDING SMQCONTEXTS:");
		hlist_for_each_entry_safe(ictx, n, &fl->clst.pending, hn) {
			len += scnprintf(fileinfo + len, DEBUGFS_SIZE - len,
						"%s %pK %s %u %s %u %s %u\n",
						"smqcontext:", ictx,
						"sc:", ictx->sc,
						"tid:", ictx->pid,
						"handle", ictx->rpra->h);
		}
		len += scnprintf(fileinfo + len, DEBUGFS_SIZE - len,
					"\n%s\n",
					"LIST OF INTERRUPTED SMQCONTEXTS:");
		hlist_for_each_entry_safe(ictx, n, &fl->clst.interrupted, hn) {
		len += scnprintf(fileinfo + len, DEBUGFS_SIZE - len,
					"%s %pK %s %u %s %u %s %u\n",
					"smqcontext:", ictx,
					"sc:", ictx->sc,
					"tid:", ictx->pid,
					"handle", ictx->rpra->h);
		}
		spin_unlock(&fl->hlock);
	}
	if (len > DEBUGFS_SIZE)
		len = DEBUGFS_SIZE;
	ret = simple_read_from_buffer(buffer, count, position, fileinfo, len);
	kfree(fileinfo);
bail:
	return ret;
}

static const struct file_operations debugfs_fops = {
	.open = fastrpc_debugfs_open,
	.read = fastrpc_debugfs_read,
};
static int fastrpc_channel_open(struct fastrpc_file *fl)
{
	struct fastrpc_apps *me = &gfa;
	int cid, err = 0;

	mutex_lock(&me->smd_mutex);

	VERIFY(err, fl && fl->sctx);
	if (err)
		goto bail;
	cid = fl->cid;
	VERIFY(err, cid >= 0 && cid < NUM_CHANNELS);
	if (err)
		goto bail;
	if (me->channel[cid].ssrcount !=
				 me->channel[cid].prevssrcount) {
		if (!me->channel[cid].issubsystemup) {
			VERIFY(err, 0);
			if (err) {
				err = -ENOTCONN;
				goto bail;
			}
		}
	}
	fl->ssrcount = me->channel[cid].ssrcount;
	fl->refcount = 1;
	if ((kref_get_unless_zero(&me->channel[cid].kref) == 0) ||
	    (me->channel[cid].chan == NULL)) {
		if (me->glink) {
			VERIFY(err, 0 == fastrpc_glink_register(cid, me));
			if (err)
				goto bail;
			VERIFY(err, 0 == fastrpc_glink_open(cid));
		} else {
		VERIFY(err, !smd_named_open_on_edge(FASTRPC_SMD_GUID,
				gcinfo[cid].channel,
				(smd_channel_t **)&me->channel[cid].chan,
				(void *)(uintptr_t)cid,
				smd_event_handler));
		}
		VERIFY(err,
			 wait_for_completion_timeout(&me->channel[cid].workport,
						RPC_TIMEOUT));
		if (err) {
			me->channel[cid].chan = NULL;
			goto bail;
		}
		kref_init(&me->channel[cid].kref);
		pr_info("'opened /dev/%s c %d %d'\n", gcinfo[cid].name,
						MAJOR(me->dev_no), cid);
		if (me->glink) {
			err = glink_queue_rx_intent(me->channel[cid].chan, NULL,
				FASTRPC_GLINK_INTENT_LEN);
			err |= glink_queue_rx_intent(me->channel[cid].chan,
				NULL, FASTRPC_GLINK_INTENT_LEN);
			if (err)
				pr_warn("adsprpc: initial intent fail for %d err %d\n",
					cid, err);
		}
		if (cid == 0 && me->channel[cid].ssrcount !=
				 me->channel[cid].prevssrcount) {
			if (fastrpc_mmap_remove_ssr(fl))
				pr_err("ADSPRPC: SSR: Failed to unmap remote heap\n");
			me->channel[cid].prevssrcount =
						me->channel[cid].ssrcount;
		}
	}

bail:
	mutex_unlock(&me->smd_mutex);
	return err;
}

static int fastrpc_device_open(struct inode *inode, struct file *filp)
{
	int err = 0;
	struct dentry *debugfs_file;
	struct fastrpc_file *fl = NULL;
	struct fastrpc_apps *me = &gfa;

	VERIFY(err, NULL != (fl = kzalloc(sizeof(*fl), GFP_KERNEL)));
	if (err)
		return err;
	debugfs_file = debugfs_create_file(current->comm, 0644, debugfs_root,
						fl, &debugfs_fops);
	context_list_ctor(&fl->clst);
	spin_lock_init(&fl->hlock);
	INIT_HLIST_HEAD(&fl->maps);
	INIT_HLIST_HEAD(&fl->perf);
	INIT_HLIST_HEAD(&fl->bufs);
	INIT_HLIST_NODE(&fl->hn);
	fl->sessionid = 0;
	fl->tgid = current->tgid;
	fl->apps = me;
	fl->mode = FASTRPC_MODE_SERIAL;
	fl->cid = -1;
	if (debugfs_file != NULL)
		fl->debugfs_file = debugfs_file;
	fl->qos_request = 0;
	fl->refcount = 0;
	filp->private_data = fl;
	mutex_init(&fl->map_mutex);
	mutex_init(&fl->fl_map_mutex);
	spin_lock(&me->hlock);
	hlist_add_head(&fl->hn, &me->drivers);
	spin_unlock(&me->hlock);
	mutex_init(&fl->perf_mutex);
	return 0;
}

static int fastrpc_get_info(struct fastrpc_file *fl, uint32_t *info)
{
	int err = 0;
	uint32_t cid;

	VERIFY(err, fl != NULL);
	if (err)
		goto bail;
	if (fl->cid == -1) {
		cid = *info;
		VERIFY(err, cid < NUM_CHANNELS);
		if (err)
			goto bail;
		fl->cid = cid;
		fl->ssrcount = fl->apps->channel[cid].ssrcount;
		VERIFY(err, !fastrpc_session_alloc_locked(
			&fl->apps->channel[cid], 0, fl->sharedcb, &fl->sctx));
		if (err)
			goto bail;
	}
	VERIFY(err, fl->sctx != NULL);
	if (err)
		goto bail;
	*info = (fl->sctx->smmu.enabled ? 1 : 0);
bail:
	return err;
}

static int fastrpc_internal_control(struct fastrpc_file *fl,
					struct fastrpc_ioctl_control *cp)
{
	int err = 0;
	int latency;

	VERIFY(err, !IS_ERR_OR_NULL(fl) && !IS_ERR_OR_NULL(fl->apps));
	if (err)
		goto bail;
	VERIFY(err, !IS_ERR_OR_NULL(cp));
	if (err)
		goto bail;

	switch (cp->req) {
	case FASTRPC_CONTROL_LATENCY:
		latency = cp->lp.enable == FASTRPC_LATENCY_CTRL_ENB ?
			fl->apps->latency : PM_QOS_DEFAULT_VALUE;
		VERIFY(err, latency != 0);
		if (err)
			goto bail;
		if (!fl->qos_request) {
			pm_qos_add_request(&fl->pm_qos_req,
				PM_QOS_CPU_DMA_LATENCY, latency);
			fl->qos_request = 1;
		} else
			pm_qos_update_request(&fl->pm_qos_req, latency);
		break;
	case FASTRPC_CONTROL_SMMU:
		fl->sharedcb = cp->smmu.sharedcb;
		break;
	default:
		err = -ENOTTY;
		break;
	}
bail:
	return err;
}

static long fastrpc_device_ioctl(struct file *file, unsigned int ioctl_num,
				 unsigned long ioctl_param)
{
	union {
		struct fastrpc_ioctl_invoke_crc inv;
		struct fastrpc_ioctl_mmap mmap;
		struct fastrpc_ioctl_munmap munmap;
		struct fastrpc_ioctl_munmap_fd munmap_fd;
		struct fastrpc_ioctl_init_attrs init;
		struct fastrpc_ioctl_perf perf;
		struct fastrpc_ioctl_control cp;
	} p;
	void *param = (char *)ioctl_param;
	struct fastrpc_file *fl = (struct fastrpc_file *)file->private_data;
	int size = 0, err = 0;
	uint32_t info;

	p.inv.fds = NULL;
	p.inv.attrs = NULL;
	p.inv.crc = NULL;
	spin_lock(&fl->hlock);
	if (fl->file_close == 1) {
		err = EBADF;
		pr_warn("ADSPRPC: fastrpc_device_release is happening, So not sending any new requests to DSP");
		spin_unlock(&fl->hlock);
		goto bail;
	}
	spin_unlock(&fl->hlock);

	switch (ioctl_num) {
	case FASTRPC_IOCTL_INVOKE:
		size = sizeof(struct fastrpc_ioctl_invoke);
		/* fall through */
	case FASTRPC_IOCTL_INVOKE_FD:
		if (!size)
			size = sizeof(struct fastrpc_ioctl_invoke_fd);
		/* fall through */
	case FASTRPC_IOCTL_INVOKE_ATTRS:
		if (!size)
			size = sizeof(struct fastrpc_ioctl_invoke_attrs);
		/* fall through */
	case FASTRPC_IOCTL_INVOKE_CRC:
		if (!size)
			size = sizeof(struct fastrpc_ioctl_invoke_crc);
		K_COPY_FROM_USER(err, 0, &p.inv, param, size);
		if (err)
			goto bail;
		VERIFY(err, 0 == (err = fastrpc_internal_invoke(fl, fl->mode,
						0, &p.inv)));
		if (err)
			goto bail;
		break;
	case FASTRPC_IOCTL_MMAP:
		K_COPY_FROM_USER(err, 0, &p.mmap, param,
						sizeof(p.mmap));
		if (err)
			goto bail;
		VERIFY(err, 0 == (err = fastrpc_internal_mmap(fl, &p.mmap)));
		if (err)
			goto bail;
		K_COPY_TO_USER(err, 0, param, &p.mmap, sizeof(p.mmap));
		if (err)
			goto bail;
		break;
	case FASTRPC_IOCTL_MUNMAP:
		K_COPY_FROM_USER(err, 0, &p.munmap, param,
						sizeof(p.munmap));
		if (err)
			goto bail;
		VERIFY(err, 0 == (err = fastrpc_internal_munmap(fl,
							&p.munmap)));
		if (err)
			goto bail;
		break;
	case FASTRPC_IOCTL_MMAP_64:
		K_COPY_FROM_USER(err, 0, &p.mmap, param,
						sizeof(p.mmap));
		if (err)
			goto bail;
		VERIFY(err, 0 == (err = fastrpc_internal_mmap(fl, &p.mmap)));
		if (err)
			goto bail;
		K_COPY_TO_USER(err, 0, param, &p.mmap, sizeof(p.mmap));
		if (err)
			goto bail;
		break;
	case FASTRPC_IOCTL_MUNMAP_64:
		K_COPY_FROM_USER(err, 0, &p.munmap, param,
						sizeof(p.munmap));
		if (err)
			goto bail;
		VERIFY(err, 0 == (err = fastrpc_internal_munmap(fl,
							&p.munmap)));
		if (err)
			goto bail;
		break;
	case FASTRPC_IOCTL_MUNMAP_FD:
		K_COPY_FROM_USER(err, 0, &p.munmap_fd, param,
			sizeof(p.munmap_fd));
		if (err)
			goto bail;
		VERIFY(err, 0 == (err = fastrpc_internal_munmap_fd(fl,
			&p.munmap_fd)));
		if (err)
			goto bail;
		break;
	case FASTRPC_IOCTL_SETMODE:
		switch ((uint32_t)ioctl_param) {
		case FASTRPC_MODE_PARALLEL:
		case FASTRPC_MODE_SERIAL:
			fl->mode = (uint32_t)ioctl_param;
			break;
		case FASTRPC_MODE_PROFILE:
			fl->profile = (uint32_t)ioctl_param;
			break;
		case FASTRPC_MODE_SESSION:
			fl->sessionid = 1;
			fl->tgid |= (1 << SESSION_ID_INDEX);
			break;
		default:
			err = -ENOTTY;
			break;
		}
		break;
	case FASTRPC_IOCTL_GETPERF:
		K_COPY_FROM_USER(err, 0, &p.perf,
					param, sizeof(p.perf));
		if (err)
			goto bail;
		p.perf.numkeys = sizeof(struct fastrpc_perf)/sizeof(int64_t);
		if (p.perf.keys) {
			char *keys = PERF_KEYS;

			K_COPY_TO_USER(err, 0, (void *)p.perf.keys,
						 keys, strlen(keys)+1);
			if (err)
				goto bail;
		}
		if (p.perf.data) {
			struct fastrpc_perf *perf = NULL, *fperf = NULL;
			struct hlist_node *n = NULL;

			mutex_lock(&fl->perf_mutex);
			hlist_for_each_entry_safe(perf, n, &fl->perf, hn) {
				if (perf->tid == current->pid) {
					fperf = perf;
					break;
				}
			}

			mutex_unlock(&fl->perf_mutex);

			if (fperf) {
				K_COPY_TO_USER(err, 0, (void *)p.perf.data,
					fperf, sizeof(*fperf));
			}
		}
		K_COPY_TO_USER(err, 0, param, &p.perf, sizeof(p.perf));
		if (err)
			goto bail;
		break;
	case FASTRPC_IOCTL_CONTROL:
		K_COPY_FROM_USER(err, 0, &p.cp, param,
				sizeof(p.cp));
		if (err)
			goto bail;
		VERIFY(err, 0 == (err = fastrpc_internal_control(fl, &p.cp)));
		if (err)
			goto bail;
		break;
	case FASTRPC_IOCTL_GETINFO:
	    K_COPY_FROM_USER(err, 0, &info, param, sizeof(info));
		if (err)
			goto bail;
		VERIFY(err, 0 == (err = fastrpc_get_info(fl, &info)));
		if (err)
			goto bail;
		K_COPY_TO_USER(err, 0, param, &info, sizeof(info));
		if (err)
			goto bail;
		break;
	case FASTRPC_IOCTL_INIT:
		p.init.attrs = 0;
		p.init.siglen = 0;
		size = sizeof(struct fastrpc_ioctl_init);
		/* fall through */
	case FASTRPC_IOCTL_INIT_ATTRS:
		if (!size)
			size = sizeof(struct fastrpc_ioctl_init_attrs);
		K_COPY_FROM_USER(err, 0, &p.init, param, size);
		if (err)
			goto bail;
		VERIFY(err, p.init.init.filelen >= 0 &&
			p.init.init.filelen < INIT_FILELEN_MAX);
		if (err)
			goto bail;
		VERIFY(err, p.init.init.memlen >= 0 &&
			p.init.init.memlen < INIT_MEMLEN_MAX);
		if (err)
			goto bail;
		VERIFY(err, 0 == (err = fastrpc_init_process(fl, &p.init)));
		if (err)
			goto bail;
		break;

	default:
		err = -ENOTTY;
		pr_info("bad ioctl: %d\n", ioctl_num);
		break;
	}
 bail:
	return err;
}

static int fastrpc_restart_notifier_cb(struct notifier_block *nb,
					unsigned long code,
					void *data)
{
	struct fastrpc_apps *me = &gfa;
	struct fastrpc_channel_ctx *ctx;
	struct notif_data *notifdata = data;
	int cid;

	ctx = container_of(nb, struct fastrpc_channel_ctx, nb);
	cid = ctx - &me->channel[0];
	if (code == SUBSYS_BEFORE_SHUTDOWN) {
		mutex_lock(&me->smd_mutex);
		ctx->ssrcount++;
		ctx->issubsystemup = 0;
		if (ctx->chan) {
			if (me->glink)
				fastrpc_glink_close(ctx->chan, cid);
			else
				smd_close(ctx->chan);
			ctx->chan = NULL;
			pr_info("'restart notifier: closed /dev/%s c %d %d'\n",
				 gcinfo[cid].name, MAJOR(me->dev_no), cid);
		}
		mutex_unlock(&me->smd_mutex);
		if (cid == 0)
			me->staticpd_flags = 0;
		fastrpc_notify_drivers(me, cid);
	} else if (code == SUBSYS_RAMDUMP_NOTIFICATION) {
		if (me->channel[0].remoteheap_ramdump_dev &&
				notifdata->enable_ramdump) {
			me->channel[0].ramdumpenabled = 1;
		}
	} else if (code == SUBSYS_AFTER_POWERUP) {
		ctx->issubsystemup = 1;
	}

	return NOTIFY_DONE;
}

static int fastrpc_pdr_notifier_cb(struct notifier_block *pdrnb,
					unsigned long code,
					void *data)
{
	struct fastrpc_apps *me = &gfa;
	struct fastrpc_static_pd *spd;
	struct notif_data *notifdata = data;

	spd = container_of(pdrnb, struct fastrpc_static_pd, pdrnb);
	if (code == SERVREG_NOTIF_SERVICE_STATE_DOWN_V01) {
		mutex_lock(&me->smd_mutex);
		spd->pdrcount++;
		spd->ispdup = 0;
		pr_info("ADSPRPC: Audio PDR notifier %d %s\n",
					MAJOR(me->dev_no), spd->spdname);
		mutex_unlock(&me->smd_mutex);
		if (!strcmp(spd->spdname,
				AUDIO_PDR_SERVICE_LOCATION_CLIENT_NAME))
			me->staticpd_flags = 0;
		fastrpc_notify_pdr_drivers(me, spd->spdname);
	} else if (code == SUBSYS_RAMDUMP_NOTIFICATION) {
		if (me->channel[0].remoteheap_ramdump_dev &&
				notifdata->enable_ramdump) {
			me->channel[0].ramdumpenabled = 1;
		}
	} else if (code == SERVREG_NOTIF_SERVICE_STATE_UP_V01) {
		spd->ispdup = 1;
	}

	return NOTIFY_DONE;
}

static int fastrpc_get_service_location_notify(struct notifier_block *nb,
					     unsigned long opcode, void *data)
{
	struct fastrpc_static_pd *spd;
	struct pd_qmi_client_data *pdr = data;
	int curr_state = 0, i = 0;

	spd = container_of(nb, struct fastrpc_static_pd, get_service_nb);
	if (opcode == LOCATOR_DOWN) {
		pr_err("ADSPRPC: Audio PD restart notifier locator down\n");
		return NOTIFY_DONE;
	}

	for (i = 0; i < pdr->total_domains; i++) {
		if ((!strcmp(pdr->domain_list[i].name,
					"msm/adsp/audio_pd")) ||
					(!strcmp(pdr->domain_list[i].name,
					"msm/adsp/sensor_pd"))) {
			spd->pdrhandle =
				service_notif_register_notifier(
				pdr->domain_list[i].name,
				pdr->domain_list[i].instance_id,
				&spd->pdrnb, &curr_state);
			if (IS_ERR(spd->pdrhandle))
				pr_err("ADSPRPC: Unable to register notifier\n");
			break;
		}
	}

	return NOTIFY_DONE;
}

static int fastrpc_smmu_fault_handler(struct iommu_domain *domain,
	struct device *dev, unsigned long iova, int flags, void *token)
{
	struct fastrpc_session_ctx *sess = (struct fastrpc_session_ctx *)token;
	int err = 0;

	VERIFY(err, sess != NULL);
	if (err)
		return err;
	sess->smmu.faults++;
	dev_err(dev, "ADSPRPC context fault: iova=0x%08lx, cb = %d, faults=%d",
					iova, sess->smmu.cb, sess->smmu.faults);
	return 0;
}

static const struct file_operations fops = {
	.open = fastrpc_device_open,
	.release = fastrpc_device_release,
	.unlocked_ioctl = fastrpc_device_ioctl,
	.compat_ioctl = compat_fastrpc_device_ioctl,
};

static const struct of_device_id fastrpc_match_table[] = {
	{ .compatible = "qcom,msm-fastrpc-adsp", },
	{ .compatible = "qcom,msm-fastrpc-compute", },
	{ .compatible = "qcom,msm-fastrpc-compute-cb", },
	{ .compatible = "qcom,msm-fastrpc-legacy-compute", },
	{ .compatible = "qcom,msm-fastrpc-legacy-compute-cb", },
	{ .compatible = "qcom,msm-adsprpc-mem-region", },
	{}
};

static int fastrpc_cb_probe(struct device *dev)
{
	struct fastrpc_channel_ctx *chan;
	struct fastrpc_session_ctx *sess;
	struct of_phandle_args iommuspec;
	const char *name;
	unsigned int start = 0x80000000;
	int err = 0, i;
	int secure_vmid = VMID_CP_PIXEL;

	VERIFY(err, NULL != (name = of_get_property(dev->of_node,
					 "label", NULL)));
	if (err)
		goto bail;
	for (i = 0; i < NUM_CHANNELS; i++) {
		if (!gcinfo[i].name)
			continue;
		if (!strcmp(name, gcinfo[i].name))
			break;
	}
	VERIFY(err, i < NUM_CHANNELS);
	if (err)
		goto bail;
	chan = &gcinfo[i];
	VERIFY(err, chan->sesscount < NUM_SESSIONS);
	if (err)
		goto bail;

	VERIFY(err, !of_parse_phandle_with_args(dev->of_node, "iommus",
						"#iommu-cells", 0, &iommuspec));
	if (err)
		goto bail;
	sess = &chan->session[chan->sesscount];
	sess->smmu.cb = iommuspec.args[0] & 0xf;
	sess->used = 0;
	sess->smmu.coherent = of_property_read_bool(dev->of_node,
						"dma-coherent");
	sess->smmu.sharedcb = of_property_read_bool(dev->of_node,
						"shared-cb");
	sess->smmu.secure = of_property_read_bool(dev->of_node,
						"qcom,secure-context-bank");
	if (sess->smmu.secure)
		start = 0x60000000;
	VERIFY(err, !IS_ERR_OR_NULL(sess->smmu.mapping =
				arm_iommu_create_mapping(&platform_bus_type,
						start, 0x78000000)));
	if (err)
		goto bail;

	if (sess->smmu.secure)
		iommu_domain_set_attr(sess->smmu.mapping->domain,
				DOMAIN_ATTR_SECURE_VMID,
				&secure_vmid);

	VERIFY(err, !arm_iommu_attach_device(dev, sess->smmu.mapping));
	if (err)
		goto bail;
	sess->smmu.dev = dev;
	sess->smmu.enabled = 1;
	chan->sesscount++;
	debugfs_global_file = debugfs_create_file("global", 0644, debugfs_root,
							NULL, &debugfs_fops);
bail:
	return err;
}

static int fastrpc_cb_subsids_probe(struct device *dev)
{
	struct fastrpc_channel_ctx *chan;
	struct fastrpc_session_ctx *main_session, *sess;
	struct of_phandle_args iommuspec;
	const char *name;
	unsigned int *range = 0, range_size = 0;
	unsigned int *sids = 0, sids_size = 0;
	int err = 0, i;

	VERIFY(err, 0 != (name = of_get_property(dev->of_node, "label", NULL)));
	if (err)
		goto bail;

	VERIFY(err, !of_parse_phandle_with_args(dev->of_node, "iommus",
						"#iommu-cells", 0, &iommuspec));
	if (err)
		goto bail;

	err = of_property_read_u32_array(dev->of_node,
					"qcom,virtual-addr-pool",
					range,
					range_size/sizeof(unsigned int));
	if (err)
		goto bail;

	err = of_property_read_u32_array(dev->of_node,
					"qcom,adsp-shared-sids",
					sids,
					sids_size/sizeof(unsigned int));
	if (err)
		goto bail;

	chan = &gcinfo[0];

	/* Main RPC session */
	main_session = &chan->session[chan->sesscount];
	VERIFY(err, !IS_ERR_OR_NULL(main_session->smmu.mapping =
				arm_iommu_create_mapping(&platform_bus_type,
						range[0], range[1])));
	if (err)
		goto bail;

	iommu_set_fault_handler(main_session->smmu.mapping->domain,
				fastrpc_smmu_fault_handler, main_session);
	VERIFY(err, !arm_iommu_attach_device(dev, main_session->smmu.mapping));
	if (err)
		goto bail;

	main_session->smmu.dev = dev;

	/* Secondary RPC sessions */
	for (i = 0; i < (sids_size / sizeof(unsigned int)); i++) {
		VERIFY(err, chan->sesscount < NUM_SESSIONS);
		if (err)
			goto bail;

		sess = &chan->session[chan->sesscount];
		sess->used = 0;

		/* Configure coherency, if needed */
		sess->smmu.coherent = of_property_read_bool(dev->of_node,
						"dma-coherent");

		/* Use the main session's IOMMU mapping and device */
		sess->smmu.mapping = main_session->smmu.mapping;
		sess->smmu.dev = main_session->smmu.dev;

		/* Hack-in the sub context banks and enable */
		sess->smmu.cb = sids[i];
		sess->smmu.enabled = 1;
		chan->sesscount++;
	}
	debugfs_global_file = debugfs_create_file("global", 0644, debugfs_root,
							NULL, &debugfs_fops);
bail:
	kfree(sids);
	kfree(range);
	return err;
}

static int fastrpc_cb_legacy_probe(struct device *dev)
{
	struct fastrpc_channel_ctx *chan;
	struct fastrpc_session_ctx *first_sess = NULL, *sess = NULL;
	const char *name;
	unsigned int *sids = NULL, sids_size = 0;
	int err = 0, ret = 0, i;

	unsigned int start = 0x80000000;

	VERIFY(err, NULL != (name = of_get_property(dev->of_node,
					 "label", NULL)));
	if (err)
		goto bail;

	for (i = 0; i < NUM_CHANNELS; i++) {
		if (!gcinfo[i].name)
			continue;
		if (!strcmp(name, gcinfo[i].name))
			break;
	}
	VERIFY(err, i < NUM_CHANNELS);
	if (err)
		goto bail;

	chan = &gcinfo[i];
	VERIFY(err, chan->sesscount < NUM_SESSIONS);
	if (err)
		goto bail;

	first_sess  = &chan->session[chan->sesscount];

	VERIFY(err, NULL != of_get_property(dev->of_node,
				"sids", &sids_size));
	if (err)
		goto bail;

	VERIFY(err, NULL != (sids = kzalloc(sids_size, GFP_KERNEL)));
	if (err)
		goto bail;
	ret = of_property_read_u32_array(dev->of_node, "sids", sids,
					sids_size/sizeof(unsigned int));
	if (ret)
		goto bail;

	VERIFY(err, !IS_ERR_OR_NULL(first_sess->smmu.mapping =
				arm_iommu_create_mapping(&platform_bus_type,
						start, 0x78000000)));
	if (err)
		goto bail;

	VERIFY(err, !arm_iommu_attach_device(dev, first_sess->smmu.mapping));
	if (err)
		goto bail;


	for (i = 0; i < sids_size/sizeof(unsigned int); i++) {
		VERIFY(err, chan->sesscount < NUM_SESSIONS);
		if (err)
			goto bail;
		sess = &chan->session[chan->sesscount];
		sess->smmu.cb = sids[i];
		sess->smmu.dev = dev;
		sess->smmu.mapping = first_sess->smmu.mapping;
		sess->smmu.enabled = 1;
		sess->used = 0;
		sess->smmu.coherent = false;
		sess->smmu.secure = false;
		chan->sesscount++;
	}
bail:
	kfree(sids);
	return err;
}



static void init_secure_vmid_list(struct device *dev, char *prop_name,
						struct secure_vm *destvm)
{
	int err = 0;
	u32 len = 0, i = 0;
	u32 *rhvmlist = NULL;
	u32 *rhvmpermlist = NULL;

	if (!of_find_property(dev->of_node, prop_name, &len))
		goto bail;
	if (len == 0)
		goto bail;
	len /= sizeof(u32);
	VERIFY(err, NULL != (rhvmlist = kcalloc(len, sizeof(u32), GFP_KERNEL)));
	if (err)
		goto bail;
	VERIFY(err, NULL != (rhvmpermlist = kcalloc(len, sizeof(u32),
					 GFP_KERNEL)));
	if (err)
		goto bail;
	for (i = 0; i < len; i++) {
		err = of_property_read_u32_index(dev->of_node, prop_name, i,
								&rhvmlist[i]);
		rhvmpermlist[i] = PERM_READ | PERM_WRITE | PERM_EXEC;
		pr_info("ADSPRPC: Secure VMID = %d", rhvmlist[i]);
		if (err) {
			pr_err("ADSPRPC: Failed to read VMID\n");
			goto bail;
		}
	}
	destvm->vmid = rhvmlist;
	destvm->vmperm = rhvmpermlist;
	destvm->vmcount = len;
bail:
	if (err) {
		kfree(rhvmlist);
		kfree(rhvmpermlist);
	}
}

static int fastrpc_probe(struct platform_device *pdev)
{
	int err = 0;
	struct fastrpc_apps *me = &gfa;
	struct device *dev = &pdev->dev;
	struct device_node *ion_node, *node;
	struct platform_device *ion_pdev;
	struct cma *cma;
	uint32_t vmid_ssc_q6;
	uint32_t val;
	int ret = 0;


	if (of_device_is_compatible(dev->of_node,
					"qcom,msm-fastrpc-fixedsids-compute-cb"))
		return fastrpc_cb_subsids_probe(dev);

	if (of_device_is_compatible(dev->of_node,
					"qcom,msm-fastrpc-compute")) {
		init_secure_vmid_list(dev, "qcom,adsp-remoteheap-vmid",
							&gcinfo[0].rhvm);


		of_property_read_u32(dev->of_node, "qcom,rpc-latency-us",
			&me->latency);
	}
	if (of_device_is_compatible(dev->of_node,
					"qcom,msm-fastrpc-compute-cb"))
		return fastrpc_cb_probe(dev);

	if (of_device_is_compatible(dev->of_node,
					"qcom,msm-fastrpc-legacy-compute")) {
		me->glink = false;
		me->legacy = 1;
	}

	if (of_device_is_compatible(dev->of_node,
					"qcom,msm-fastrpc-legacy-compute-cb")){
		return fastrpc_cb_legacy_probe(dev);
	}

	if (of_property_read_u32(dev->of_node, "qcom,vmid-ssc-q6", &vmid_ssc_q6))
		vmid_ssc_q6 = VMID_SSC_Q6;

	if (of_device_is_compatible(dev->of_node,
					"qcom,msm-adsprpc-mem-region")) {
		me->dev = dev;
		ion_node = of_find_compatible_node(NULL, NULL, "qcom,msm-ion");
		if (ion_node) {
			for_each_available_child_of_node(ion_node, node) {
				if (of_property_read_u32(node, "reg", &val))
					continue;
				if (val != ION_ADSP_HEAP_ID)
					continue;
				ion_pdev = of_find_device_by_node(node);
				if (!ion_pdev)
					break;
				cma = dev_get_cma_area(&ion_pdev->dev);
				if (cma) {
					me->range.addr = cma_get_base(cma);
					me->range.size =
						(size_t)cma_get_size(cma);
				}
				break;
			}
		}
		if (me->range.addr && !of_property_read_bool(dev->of_node,
							 "restrict-access")) {
			int srcVM[1] = {VMID_HLOS};
			int destVM[4] = {VMID_HLOS, VMID_MSS_MSA, vmid_ssc_q6,
						VMID_ADSP_Q6};
			int destVMperm[4] = {PERM_READ | PERM_WRITE | PERM_EXEC,
				PERM_READ | PERM_WRITE | PERM_EXEC,
				PERM_READ | PERM_WRITE | PERM_EXEC,
				PERM_READ | PERM_WRITE | PERM_EXEC,
				};

			VERIFY(err, !hyp_assign_phys(me->range.addr,
					me->range.size, srcVM, 1,
					destVM, destVMperm, 4));
			if (err)
				goto bail;
		}
		return 0;
	}
	if (of_property_read_bool(dev->of_node,
					"qcom,fastrpc-adsp-audio-pdr")) {
		int session;

		VERIFY(err, !fastrpc_get_adsp_session(
			AUDIO_PDR_SERVICE_LOCATION_CLIENT_NAME, &session));
		if (err)
			goto spdbail;
		me->channel[0].spd[session].get_service_nb.notifier_call =
					fastrpc_get_service_location_notify;
		ret = get_service_location(
				AUDIO_PDR_SERVICE_LOCATION_CLIENT_NAME,
				AUDIO_PDR_ADSP_SERVICE_NAME,
				&me->channel[0].spd[session].get_service_nb);
		if (ret)
			pr_err("ADSPRPC: Get service location failed: %d\n",
								ret);
	}
	if (of_property_read_bool(dev->of_node,
					"qcom,fastrpc-adsp-sensors-pdr")) {
		int session;

		VERIFY(err, !fastrpc_get_adsp_session(
			SENSORS_PDR_SERVICE_LOCATION_CLIENT_NAME, &session));
		if (err)
			goto spdbail;
		me->channel[0].spd[session].get_service_nb.notifier_call =
					fastrpc_get_service_location_notify;
		ret = get_service_location(
				SENSORS_PDR_SERVICE_LOCATION_CLIENT_NAME,
				SENSORS_PDR_ADSP_SERVICE_NAME,
				&me->channel[0].spd[session].get_service_nb);
		if (ret)
			pr_err("ADSPRPC: Get service location failed: %d\n",
								ret);
	}
spdbail:
	err = 0;
	VERIFY(err, !of_platform_populate(pdev->dev.of_node,
					  fastrpc_match_table,
					  NULL, &pdev->dev));
	if (err)
		goto bail;
bail:
	return err;
}

static void fastrpc_deinit(void)
{
	struct fastrpc_apps *me = &gfa;
	struct fastrpc_channel_ctx *chan = gcinfo;
	int i, j;

	for (i = 0; i < NUM_CHANNELS; i++, chan++) {
		if (chan->chan) {
			kref_put_mutex(&chan->kref,
				fastrpc_channel_close, &me->smd_mutex);
			chan->chan = NULL;
		}
		for (j = 0; j < NUM_SESSIONS; j++) {
			struct fastrpc_session_ctx *sess = &chan->session[j];
			if (sess->smmu.dev) {
				arm_iommu_detach_device(sess->smmu.dev);
				sess->smmu.dev = NULL;
			}
			if (sess->smmu.mapping) {
				arm_iommu_release_mapping(sess->smmu.mapping);
				sess->smmu.mapping = NULL;
			}
		}
		kfree(chan->rhvm.vmid);
		kfree(chan->rhvm.vmperm);
	}
}

static struct platform_driver fastrpc_driver = {
	.probe = fastrpc_probe,
	.driver = {
		.name = "fastrpc",
		.owner = THIS_MODULE,
		.of_match_table = fastrpc_match_table,
	},
};

static int __init fastrpc_device_init(void)
{
	struct fastrpc_apps *me = &gfa;
	struct device *dev = NULL;
	int err = 0, i;

	memset(me, 0, sizeof(*me));

	fastrpc_init(me);
	me->dev = NULL;
	me->glink = true;
	VERIFY(err, 0 == platform_driver_register(&fastrpc_driver));
	if (err)
		goto register_bail;
	VERIFY(err, 0 == alloc_chrdev_region(&me->dev_no, 0, NUM_CHANNELS,
					DEVICE_NAME));
	if (err)
		goto alloc_chrdev_bail;
	cdev_init(&me->cdev, &fops);
	me->cdev.owner = THIS_MODULE;
	VERIFY(err, 0 == cdev_add(&me->cdev, MKDEV(MAJOR(me->dev_no), 0),
				1));
	if (err)
		goto cdev_init_bail;
	me->class = class_create(THIS_MODULE, "fastrpc");
	VERIFY(err, !IS_ERR(me->class));
	if (err)
		goto class_create_bail;
	me->compat = (fops.compat_ioctl == NULL) ? 0 : 1;
	dev = device_create(me->class, NULL,
				MKDEV(MAJOR(me->dev_no), 0),
				NULL, gcinfo[0].name);
	VERIFY(err, !IS_ERR_OR_NULL(dev));
	if (err)
		goto device_create_bail;
	for (i = 0; i < NUM_CHANNELS; i++) {
		me->channel[i].dev = dev;
		me->channel[i].ssrcount = 0;
		me->channel[i].prevssrcount = 0;
		me->channel[i].issubsystemup = 1;
		me->channel[i].ramdumpenabled = 0;
		me->channel[i].remoteheap_ramdump_dev = NULL;
		me->channel[i].nb.notifier_call = fastrpc_restart_notifier_cb;
		me->channel[i].handle = subsys_notif_register_notifier(
							gcinfo[i].subsys,
							&me->channel[i].nb);
	}

	me->client = msm_ion_client_create(DEVICE_NAME);
	VERIFY(err, !IS_ERR_OR_NULL(me->client));
	if (err)
		goto device_create_bail;
	debugfs_root = debugfs_create_dir("adsprpc", NULL);
	return 0;
device_create_bail:
	for (i = 0; i < NUM_CHANNELS; i++) {
		if (me->channel[i].handle)
			subsys_notif_unregister_notifier(me->channel[i].handle,
							&me->channel[i].nb);
	}
	if (!IS_ERR_OR_NULL(dev))
		device_destroy(me->class, MKDEV(MAJOR(me->dev_no), 0));
	class_destroy(me->class);
class_create_bail:
	cdev_del(&me->cdev);
cdev_init_bail:
	unregister_chrdev_region(me->dev_no, NUM_CHANNELS);
alloc_chrdev_bail:
register_bail:
	fastrpc_deinit();
	return err;
}

static void __exit fastrpc_device_exit(void)
{
	struct fastrpc_apps *me = &gfa;
	int i;

	fastrpc_file_list_dtor(me);
	fastrpc_deinit();
	for (i = 0; i < NUM_CHANNELS; i++) {
		if (!gcinfo[i].name)
			continue;
		device_destroy(me->class, MKDEV(MAJOR(me->dev_no), i));
		subsys_notif_unregister_notifier(me->channel[i].handle,
						&me->channel[i].nb);
	}
	class_destroy(me->class);
	cdev_del(&me->cdev);
	unregister_chrdev_region(me->dev_no, NUM_CHANNELS);
	ion_client_destroy(me->client);
	debugfs_remove_recursive(debugfs_root);
}

late_initcall(fastrpc_device_init);
module_exit(fastrpc_device_exit);

MODULE_LICENSE("GPL v2");
