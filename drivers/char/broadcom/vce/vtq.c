/*******************************************************************************
Copyright 2012 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#include <linux/kernel.h>

#include <linux/dma-mapping.h>
#include <linux/errno.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/kref.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
/* This (linux/uaccess) is here so we can support the procfs debug
 * stuff... but, that is polluting this file...  TODO: move it
 * outside! */
#include <linux/uaccess.h>
#include <linux/workqueue.h>

#include <linux/broadcom/vce.h>
#include <linux/broadcom/vtq.h>
#include "vceprivate.h"

/* Private configuration stuff -- not part of exposed API */
#include "vtqinit_priv.h"

/* Internal utility functions */
#include "vtqutil.h"

typedef uint32_t vtq_datamemoffset_t;
typedef uint32_t vtq_progmemoffset_t;

struct vtq_global {
	/*
	  relocatedloaderkernelimage:

	  this is the binary 'text' of the "loader kernel" - the short
	  program that stays resident in VCE instruction memory and is
	  responsible for servicing requests in the FIFO, and loading
	  new images on-the-fly.

	  currently we only have one relocated loaderkernel image,
	  because we don't relocate it on-the-fly or paramterize it
	  with fifo configuration etc.  One day we may wish to
	  generate this at run time, at which point we will have to
	  decide whether this stays here, as a "VTQ global" or whether
	  to move it to the "per VCE" state
	*/
	const uint32_t *relocatedloaderkernelimage;
	vtq_progmemoffset_t loaderkernel_loadoffset;
	size_t loaderkernel_size;
	vtq_progmemoffset_t loaderkernel_firstentry;
	vtq_progmemoffset_t loaderkernel_loadimage_entrypoint;

	/*
	 *
	 * Debug:
	 *
	 */

	/* g->proc_dir represent the dir under /proc for the VTQ
	 * driver global proc entries, e.g. /proc/vce/vtq/ or
	 * similar */
	struct proc_dir_entry *proc_dir;
	struct proc_dir_entry *proc_debug;

	/* debug_fifo: when true all Q and de-Q events are printed to
	 * the kernel log */
	unsigned long debug_fifo;

	/* host_push: don't use the DMAPROGIN but have ARM write into
	 * VCE instruction memory directly (a debug aid) */
	/* N.B. There is a race here.  host_push is a *debug*
	 * feature...  don't use it in real life!  In particular, if
	 * you *enable* host push *after* got_room_for_job has found
	 * room but before queue_job has finished writing into the
	 * FIFO this would be extremely dangerous.  Only set this once
	 * at boot time before any jobs have been issued.  This switch
	 * is not protected by a mutex! */
	unsigned long host_push;
};
typedef uint32_t prog_bitmap_t;
struct vtq_image {
	/* Track references to this image so that we can defer the
	 * freeing of it until current in-flight loads have
	 * completed. */
	struct kref ref;

	/* We record a bitmap of task-types supported by this image
	 * (i.e. for whom this image has entrypoints) */
	prog_bitmap_t bitmap;

	/* We keep a copy of the text and data in device addressable
	 * memory  */
	struct {
		void *virtaddr;
		size_t sz;
		uint32_t busaddr;
	} dmainfo;
	int textoffset;
	size_t textsz;
	int dataoffset;
	size_t datasz;

	/* Entry point (PC value) for each task-type supported by this
	 * image.  Array entry is only valid when the corresponding
	 * bit in the bitmap is set.  TODO: this ought not to be a big
	 * fat wasteful array. */
	uint32_t entrypts[VTQ_MAX_TASKS];
};
struct image_list {
	struct image_list *next;
	struct vtq_image *image;
};
struct vtq_task {
	/* we (currently) store task structs in an array.  We set the
	 * occupied flag to indicate that the entry is valid. */
	int occupied;

	/* List of images that can serve this task */
	struct image_list *suitable_images;
};
struct vtq_hook_list_node {
	uint32_t pc;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	struct vtq_hook_list_node *next;
};
struct vtq_vce {
	/* Our route back to the VCE driver: */
	struct vce *driver_priv;
	struct vce_regmap io;

	/* We set this after the one-time initialization */
	int is_configured;

	/* Current circular buffer parameters (can be changed only
	 * when fifo empty) */
	unsigned int vce_fifo_length;
	size_t fifo_entrysz;
	vtq_datamemoffset_t readptr_locn;
	vtq_datamemoffset_t writeptr_locn;
	vtq_datamemoffset_t circbuf_locn;
	uint32_t semanum;

	/* Special entry (-ies) to write into FIFO every time VCE is
	 * powered-up */
	unsigned int onloadhook_count;
	struct vtq_hook_list_node *onloadhook_list_head;

	/* Space required in prog/data mem */
	vtq_progmemoffset_t progmem_reservation;
	vtq_datamemoffset_t datamem_reservation;

	/* link to the global (not per-VCE) state */
	struct vtq_global *global;

	/* This mutex must be held while data for arm->vce is used */
	struct mutex host_mutex;

	/* This counter is the number of vtq_queue_job folk who are
	 * waiting to acquire the host_mutex (well, strictly, waiting
	 * for room in the FIFO) so if the FIFO happens to look empty
	 * because our ISR ran, but we somehow snuck in the cleanup
	 * job while people were waiting to enqueue their job, we
	 * won't stupidly turn the VCE off and on again */
	atomic_t queuerswaiting;

	/* monotonically incrementing index into fifo (wraps at 2^32
	 * -- the actual fifo entry will be modulo the fifo size,
	 * which must therefore be power-of-2 */
	vtq_job_id_t writeptr;

	/* currently loaded image. */
	struct vtq_image *current_image;

	/* "retained" image.  When we release the reference to VCE, we
	 * put the current image here, so that we can optimize away
	 * the image load if we get VCE back and it still has this
	 * image.  I thought about just using current_image for this
	 * purpose, but doing so would result in race conditions */
	struct vtq_image *retained_image;

	/* array (FIXME! not good!) of tasks */
	struct vtq_task tasks[VTQ_MAX_TASKS];

	/* Per-job info: e.g. image loads that are in flight; flags */
	unsigned int host_fifo_length;
	struct jobinfo *runningjobs;

	/* a wait-queue for threads wanting to be notified when a VCE
	 * job is complete. */
	wait_queue_head_t job_complete_wq;
	/* a wait-queue for threads wanting to be notified when a VCE
	 * job is complete and has been cleaned up, so that the entry
	 * in the host fifo can be used for a new job. */
	wait_queue_head_t more_room_in_fifo_wq;

	/* Anything relating to completions must take this mutex */
	spinlock_t vce_mutex;
	/* our cached copy of the VCE's FIFO read pointer */
	vtq_job_id_t last_known_readptr;

	/* this wait queue isn't used... should we keep this for
	 * debug, or ditch it? TODO! */
	wait_queue_head_t vce_given_work_wq;

	/* job cleanup / acknowledgement / callbacks -- now uses a
	 * separate mutex */
	struct mutex cleanup_mutex;
	vtq_job_id_t last_acknowledged_job;

	/* General stuff below needs both mutexes or special gloves */

	/* We have VCE acquired and the loaderkernel is loaded.  There
	 * should probably be a separate "power" mutex for this, but
	 * at the moment we rely on the host mutex only */
	int on;
	/* Power Lock Count is a refcount of people who have asked for
	 * the loader not to be unloaded and the power to remain on
	 * (See VTQ_LOCK_POWERLOCK) -- a non-zero here means that the
	 * unload_work does not get scheduled when the FIFO runs dry
	 * but is instead scheduled after the lock count drops to
	 * zero */
	int power_lock_count;
	/* Unload work runs when the FIFO becomes empty (unless a
	 * power lock is held) to unload the image and power down the
	 * VCE and release the exclusive lock on it to permit others
	 * to use VCE without VTQ */
	struct work_struct unload_work;
	/* Cleanup work runs after every job is complete.  It's
	 * effectively a "bottom half" to the ISR, and is responsible
	 * (generically) for calling callbacks associated with each
	 * job. */
	struct work_struct cleanup_work;

	/*
	  high level locking.  TODO: move to a separate API that sits
	  above VTQ
	*/
	struct vtq_priority_lock *priority_lock;

	/* Debug stuff below: */

	/* v->proc_dir represent the dir under /proc for the VTQ
	 * related proc entries for this specific instance of VCE,
	 * e.g., eventually, /proc/vce/vce0/vtq/ or similar -- but we
	 * don't bother with that now as we only have one VCE... but
	 * we make the distinction so we can support this in
	 * future. */
	struct proc_dir_entry *proc_dir;
	struct proc_dir_entry *proc_fifo;
};

/* Information about a queued/running job that needs to be picked up
 * after the job completes */
struct jobinfo {
	/* imageload in flight -- so we can dec its ref */
	struct vtq_image *inflightimage;
};

/*#define VTQ_DEBUG*/
#define err_print(fmt, arg...) \
	printk(KERN_ERR "VTQ: ERROR: %s() %s:%d " fmt, \
		__func__, "vtq.c", __LINE__, ##arg)
#ifdef VTQ_DEBUG
#define dbg_print(fmt, arg...) \
	printk(KERN_WARNING "VTQ: debug: %s() %s:%d " fmt, \
		__func__, "vtq.c", __LINE__, ##arg)
#else
#define dbg_print(fmt, arg...)	do { } while (0)
#endif

/*
 * Global (driver) /proc entries for Debug etc
 */

static int proc_debug_write(struct file *file,
		const char __user *buf, size_t nbytes,
		loff_t *priv)

{
	struct vtq_global *g;
	char request[100];
	int s;

	(void) file;

	g = (struct vtq_global *)priv;

	if (nbytes > sizeof(request) - 1)
		nbytes = sizeof(request) - 1;

	s = copy_from_user(request, buf, nbytes);
	nbytes -= s;
	request[nbytes] = 0;

	/*
	 * N.B.  No mutex here.  Do not change at runtime.  Actually,
	 * trace-fifo is probably harmless to change at runtime, but
	 * host-push should be configured once at boot and left alone
	 */
	if (!strncmp(request, "trace-fifo=", 11)) {
		s = strict_strtoul(request + 11, 0, &g->debug_fifo);
		if (s)
			err_print("bad argument to trace-fifo=\n");
		dbg_print("g->debug_fifo = %lu\n", g->debug_fifo);
	}

	if (!strncmp(request, "host-push=", 10)) {
		s = strict_strtoul(request + 10, 0, &g->host_push);
		if (s)
			err_print("bad argument to host-push=\n");
		dbg_print("g->host_push = %lu\n", g->host_push);
	}

	return nbytes;
}

static void term_driverprocentries(struct vtq_global *g)
{
	remove_proc_entry("debug", g->proc_dir);
}

static const struct file_operations proc_debug_fops = {
	.write	=	proc_debug_write,
};

static int init_driverprocentries(struct vtq_global *g)
{
	g->proc_debug = proc_create_data("debug",
			(S_IRUSR | S_IRGRP),
			g->proc_dir, &proc_debug_fops, g);

	if (g->proc_debug == NULL) {
		err_print("Failed to create vtq/debug proc entry\n");
		goto err_procentry_debug;
	}

	return 0;

	/*
	 * error exit paths follow
	 */

	/* remove_proc_entry("debug", g->proc_dir); */
err_procentry_debug:

	return -1;
}

int vtq_driver_init(struct vtq_global **vtq_global_out,
		    struct vce *vce,
		    struct proc_dir_entry *proc_vcedir)
{
	struct vtq_global *vtq_global;
	int s;

	/* This is per-driver stuff, so struct vce doesn't even belong
	 * on this API */
	(void) vce;

	vtq_global = kmalloc(sizeof(*vtq_global), GFP_KERNEL);
	if (vtq_global == NULL) {
		err_print("kmalloc failure\n");
		goto err_kmalloc_vtq_global_state;
	}

	/* Start with overly pessimistic values for instruction and
	 * data memory sizes to permit images to be registered before
	 * configuration step */
	vtq_global->loaderkernel_size = 0;
	vtq_global->relocatedloaderkernelimage = NULL;
	vtq_global->loaderkernel_loadoffset = 0x1000;
	vtq_global->loaderkernel_firstentry = 0;

	vtq_global->debug_fifo = 0;

	/* host_push was *intended* to be a debug-aid, but it turns
	 * out we need to work around a hardware issue, so, until we
	 * have a better workaround, we have to default to 1 at boot,
	 * or restrict image size to 8kB.  As we currently have no
	 * users with images above 8kB, we can leave this turned off
	 * for now.  TODO: Review h/w issue and make better workaround
	 */
	vtq_global->host_push = 0;

	vtq_global->proc_dir = proc_vcedir;
	s = init_driverprocentries(vtq_global);
	if (s != 0)
		goto err_init_procentries;

	/* success */
	*vtq_global_out = vtq_global;
	return 0;

	/* error exit paths follow */

	/* term_driverprocentries(vtq_global); */
err_init_procentries:

	kfree(vtq_global);
err_kmalloc_vtq_global_state:

	return -ENOENT;
}

void vtq_driver_term(struct vtq_global **vtq_global_state_ptr)
{
	term_driverprocentries(*vtq_global_state_ptr);
	kfree(*vtq_global_state_ptr);
	*vtq_global_state_ptr = NULL;
}

/* Me hates forward declarations, but I screwed up on these ones. :) */
static void try_unload_if_empty(struct work_struct *);
static void put_image(struct vtq_image *image);
static void cleanup(struct work_struct *work);

/*
 * Per-VCE /proc entries for Debug etc
 */

static int proc_fifo_read(struct file *file, char __user *buffer,
		size_t bytes, loff_t *priv)
{
	struct vtq_vce *v;
	uint32_t writeptr, readptr, ackptr;
	int ret;
	int len;
	int required_bytes;

	ret = 0;

	v = (struct vtq_vce *)priv;

	mutex_lock(&v->host_mutex);
	mutex_lock(&v->cleanup_mutex);
	writeptr = v->writeptr;
	readptr = v->last_known_readptr;
	ackptr = v->last_acknowledged_job;
	mutex_unlock(&v->host_mutex);
	mutex_unlock(&v->cleanup_mutex);

	/* estimate length of buffer req'd */
	/* required_bytes = 100 + 100 * (writeptr - ackptr); */
	/* TODO: be a little more precise about the length of buffer required */
	required_bytes = 200;

	if (bytes < required_bytes) {
		ret = -1;
		goto err_too_small;
	}

	len = snprintf(buffer, bytes, "%u/%u/%u\n"
		"%d job(s) queued or running on VCE\n"
		"%d job(s) completed but not acknowledged\n"
		"%d power lock(s) held\n",
		writeptr, readptr, ackptr,
			writeptr - readptr, readptr - ackptr,
			v->power_lock_count);

	ret = len;

	BUG_ON(len > required_bytes - 1);
	BUG_ON(ret > bytes);
	BUG_ON(ret < 0);
	return ret;

	/*
	   error exit paths follow
	 */

err_too_small:
	BUG_ON(ret >= 0);
	return ret;
}

static void term_procentries(struct vtq_vce *v)
{
	remove_proc_entry("fifo", v->proc_dir);
}

static const struct file_operations proc_fifo_fops = {
	.read =		proc_fifo_read,
};

static int init_procentries(struct vtq_vce *v)
{
	v->proc_fifo = proc_create_data("fifo",
			(S_IRUSR | S_IRGRP),
			v->proc_dir,
			&proc_fifo_fops, v);

	if (v->proc_fifo == NULL) {
		err_print("Failed to create vtq/fifo proc entry\n");
		goto err_procentry_fifo;
	}

	return 0;

	/*
	 * error exit paths follow
	 */

	/* remove_proc_entry("fifo", v->proc_dir); */
err_procentry_fifo:
	return -1;
}

int vtq_pervce_init(struct vtq_vce **vtq_pervce_state_out,
		    struct vtq_global *global,
		    struct vce *vce,
		    struct proc_dir_entry *proc_vcedir)
{
	struct vtq_vce *vtq_pervce_state;
	int i;
	int s;

	vtq_pervce_state = kmalloc(sizeof(*vtq_pervce_state), GFP_KERNEL);
	if (vtq_pervce_state == NULL) {
		err_print("kmalloc failure\n");
		goto err_kmalloc_vtq_pervce_state;
	}

	mutex_init(&vtq_pervce_state->host_mutex);
	mutex_init(&vtq_pervce_state->cleanup_mutex);
	atomic_set(&vtq_pervce_state->queuerswaiting, 0);

	vtq_pervce_state->writeptr = 07734;
	vtq_pervce_state->last_known_readptr = vtq_pervce_state->writeptr;
	vtq_pervce_state->last_acknowledged_job = vtq_pervce_state->writeptr;

	for (i = 0; i < VTQ_MAX_TASKS; i++)
		vtq_pervce_state->tasks[i].occupied = 0;

	vtq_pervce_state->global = global;

	vtq_pervce_state->driver_priv = vce;
	vce_init_base(&vtq_pervce_state->io, vtq_pervce_state->driver_priv);

	init_waitqueue_head(&vtq_pervce_state->job_complete_wq);
	init_waitqueue_head(&vtq_pervce_state->more_room_in_fifo_wq);

	spin_lock_init(&vtq_pervce_state->vce_mutex);

	init_waitqueue_head(&vtq_pervce_state->vce_given_work_wq);

	/* Configured later via ioctl */
	vtq_pervce_state->vce_fifo_length = 0;
	vtq_pervce_state->host_fifo_length = 0;
	vtq_pervce_state->is_configured = 0;
	vtq_pervce_state->runningjobs = NULL;

	vtq_pervce_state->current_image = NULL;
	vtq_pervce_state->retained_image = NULL;

	vtq_pervce_state->onloadhook_count = 0;
	vtq_pervce_state->onloadhook_list_head = NULL;

	vtq_pervce_state->on = 0;
	vtq_pervce_state->power_lock_count = 0;

	/* Start with pessimistic value to allow for early image
	 * registration */
	vtq_pervce_state->datamem_reservation = 0x1000;

	/* Initialize the work_t for unloading the loader after the
	 * FIFO is empty */
	INIT_WORK(&vtq_pervce_state->unload_work, try_unload_if_empty);

	/* This is the cleanup activity that runs after the ISR. */
	INIT_WORK(&vtq_pervce_state->cleanup_work, cleanup);

	vtq_pervce_state->proc_dir = proc_vcedir;
	s = init_procentries(vtq_pervce_state);
	if (s != 0)
		goto err_init_procentries;

	vtq_pervce_state->priority_lock = vtq_util_priority_lock_create();
	if (vtq_pervce_state->priority_lock == NULL)
		goto err_init_priority_lock;

	/* success */

	*vtq_pervce_state_out = vtq_pervce_state;
	return 0;

	/* error exit paths follow */

	/* vtq_util_priority_lock_destroy(vtq_pervce_state->priority_lock); */
err_init_priority_lock:

	term_procentries(vtq_pervce_state);
err_init_procentries:

	mutex_destroy(&vtq_pervce_state->host_mutex);
	mutex_destroy(&vtq_pervce_state->cleanup_mutex);

	kfree(vtq_pervce_state);
err_kmalloc_vtq_pervce_state:
	return -ENOENT;
}

void vtq_pervce_term(struct vtq_vce **vtq_pervce_state_ptr)
{
	struct vtq_vce *v;
	struct vtq_hook_list_node *hook;

	v = *vtq_pervce_state_ptr;
	*vtq_pervce_state_ptr = NULL;

	flush_work(&v->unload_work);
	flush_work(&v->cleanup_work);

	mutex_lock(&v->host_mutex);
	mutex_lock(&v->cleanup_mutex);
	if (v->retained_image != NULL) {
		put_image(v->retained_image);
		v->retained_image = NULL;
	}
	if (v->current_image != NULL) {
		put_image(v->current_image);
		v->current_image = NULL;
		err_print("Image still current upon unload\n");
	}
	if (v->power_lock_count != 0)
		err_print("Power locks still held upon unload\n");
	for (hook = v->onloadhook_list_head; hook != NULL;) {
		struct vtq_hook_list_node *nexthook;
		nexthook = hook->next;
		kfree(hook);
		hook = nexthook;
	}
	mutex_unlock(&v->host_mutex);
	mutex_destroy(&v->host_mutex);
	mutex_unlock(&v->cleanup_mutex);
	mutex_destroy(&v->cleanup_mutex);

	vtq_util_priority_lock_destroy(v->priority_lock);
	term_procentries(v);
	kfree(v);
}

int vtq_onloadhook(struct vtq_vce *v,
		uint32_t pc,
		uint32_t r1,
		uint32_t r2,
		uint32_t r3,
		uint32_t r4,
		uint32_t r5,
		uint32_t r6)
{
	struct vtq_hook_list_node *hook;
	struct vtq_hook_list_node **tail;

	hook = kmalloc(sizeof(*hook), GFP_KERNEL);
	if (hook == NULL)
		return -1;

	hook->pc = pc;
	hook->r1 = r1;
	hook->r2 = r2;
	hook->r3 = r3;
	hook->r4 = r4;
	hook->r5 = r5;
	hook->r6 = r6;
	hook->next = NULL;
	mutex_lock(&v->host_mutex);
	tail = &v->onloadhook_list_head;
	while (*tail != NULL)
		tail = &(*tail)->next;
	*tail = hook;
	v->onloadhook_count++;
	mutex_unlock(&v->host_mutex);

	return 0;
}

int vtq_configure(struct vtq_vce *v,
		  vtq_progmemoffset_t loader_base,
		  vtq_progmemoffset_t loader_run,
		  vtq_progmemoffset_t loadimage_entrypoint,
		  uint32_t *loader_text,
		  size_t loader_textsz,
		  vtq_datamemoffset_t datamem_reservation,
		  vtq_datamemoffset_t writepointer_locn,
		  vtq_datamemoffset_t readpointer_locn,
		  vtq_datamemoffset_t fifo_offset,
		  size_t vce_fifo_length,
		  size_t fifo_entry_size,
		  uint32_t semaphore_id)
{
	struct jobinfo *jobs;
	unsigned int i;
	size_t host_fifo_length;

	/* We make the host fifo longer than the VCE fifo length to
	 * allow the cleanup work to run without blocking space in the
	 * FIFO (as the entry is not removed from the host FIFO until
	 * it is complete) We really ought to allow this to be
	 * configured from the vtqinit but as the precise value is not
	 * important (and the data structure is small) we can just
	 * approximate and be generous here. */
	host_fifo_length = vce_fifo_length * 4;
	jobs = kmalloc(host_fifo_length * sizeof(*jobs),
			GFP_KERNEL);
	if (jobs == NULL) {
		err_print("Unable to allocate memory for perjob data\n");
		return -1;
	}
	for (i = 0; i < host_fifo_length; i++)
		jobs[i].inflightimage = NULL;

	mutex_lock(&v->host_mutex);

	if (v->is_configured) {
		/* err_print("Attempt to re-configure VCE FIFO.  We " */
		/* 	  "don't support that.\n"); */
		mutex_unlock(&v->host_mutex);
		kfree(jobs);
		return -1;
	}

	/* Not sure we should write to globals here... TODO: should we
	 * have a separate "global" configuration step?  Or should
	 * these things be per-VCE in the first place?  I think this
	 * will become clear when we think about it, but since we have
	 * only one VCE on Rhea, we haven't invested the time to think
	 * about it. */
	v->global->loaderkernel_loadoffset = loader_base;
	v->global->loaderkernel_firstentry = loader_run;
	v->global->loaderkernel_loadimage_entrypoint = loadimage_entrypoint;
	v->global->relocatedloaderkernelimage = loader_text;
	v->global->loaderkernel_size = loader_textsz;
	v->progmem_reservation = loader_base - 0x20;
	v->datamem_reservation = datamem_reservation;
	v->writeptr_locn = writepointer_locn;
	v->readptr_locn = readpointer_locn;
	v->circbuf_locn = fifo_offset;
	v->vce_fifo_length = vce_fifo_length;
	v->host_fifo_length = host_fifo_length;
	v->runningjobs = jobs;
	v->fifo_entrysz = fifo_entry_size;
	v->semanum = semaphore_id;
	v->is_configured = 1;
	mutex_unlock(&v->host_mutex);

	return 0;
}

static void vce_download_program_at(struct vtq_vce *v,
				    vtq_progmemoffset_t loadoffset,
				    const uint32_t *image,
				    size_t size)
{
	if (size & 3) {
		err_print("bad size\n");
		return;
	}

	while (size > 0) {
		vce_writeprog(&v->io, loadoffset, *image);
		loadoffset += 4;
		size -= 4;
		image++;
	}
}

static void vce_download_data_at(struct vtq_vce *v,
				 vtq_datamemoffset_t loadoffset,
				 const uint32_t *image,
				 size_t size)
{
	if (size & 3) {
		err_print("bad size\n");
		return;
	}

	while (size > 0) {
		vce_writedata(&v->io, loadoffset, *image);
		loadoffset += 4;
		size -= 4;
		image++;
	}
}

static void _loadloader(struct vtq_vce *v)
{
	int s;
	uint32_t was_preserved;

	s = vce_acquire(v->driver_priv, VCE_ACQUIRER_VTQ, &was_preserved);
	if (s < 0) {
		err_print("failed to acquire the VCE (signal?)\n");
		return;
	}

	if (was_preserved) {
		if (v->retained_image != NULL) {
			BUG_ON(v->current_image != NULL);
			v->current_image = v->retained_image;
			v->retained_image = NULL;
		}
	} else {
		vce_download_program_at(v,
				v->global->loaderkernel_loadoffset,
				v->global->relocatedloaderkernelimage,
				v->global->loaderkernel_size);
	/* Due to lack of fully working expression evaluation in the VCE
	   toolchain, we have to have worklist_reentry at the start of the
	   loader image. :( */
		vce_writereg(&v->io, 59, v->global->loaderkernel_loadoffset);
		vce_writereg(&v->io, 41, v->last_known_readptr);
		vce_writedata(&v->io, v->writeptr_locn, v->writeptr);
		vce_writedata(&v->io, v->readptr_locn, v->last_known_readptr);

		if (v->retained_image != NULL) {
			put_image(v->retained_image);
			v->retained_image = NULL;
		}
	}

	vce_setpc(&v->io, v->global->loaderkernel_firstentry);
	vce_run(&v->io);

	v->on = 1;
}

static void _unloadloader(struct vtq_vce *v)
{
	unsigned long flags;

	if (v->current_image != NULL) {
		BUG_ON(v->retained_image != NULL);
		v->retained_image = v->current_image;
		v->current_image = NULL;
	}
	vce_stop(&v->io);
	/* Have to pulse the semaphore, else we'll be hosed next time! */
	vce_clearsema(&v->io, v->semanum);
	vce_setsema(&v->io, v->semanum);
	v->on = 0;
	/* We need to wait for any running ISR to complete before we
	 * can turn clocks off etc... */
	spin_lock_irqsave(&v->vce_mutex, flags);
	BUG_ON(v->on);
	spin_unlock_irqrestore(&v->vce_mutex, flags);

	/* Finally let VCE power down */
	mb();
	vce_release(v->driver_priv);
}

static void try_unload_if_empty(struct work_struct *work)
{
	struct vtq_vce *v;
	int distance;

	v = container_of(work, struct vtq_vce, unload_work);
	mutex_lock(&v->host_mutex);
	if (!v->on) {
		mutex_unlock(&v->host_mutex);
		return;
	}
	distance = v->writeptr - v->last_known_readptr;
	if (distance > 0) {
		mutex_unlock(&v->host_mutex);
		return;
	}
	BUG_ON(distance < 0);

	/* TODO: check a special flag to abort the shutdown sequence?
	 * or is that one optimization too far?  Consider. */

	if (atomic_read(&v->queuerswaiting) > 0) {
		mutex_unlock(&v->host_mutex);
		return;
	}

	_unloadloader(v);

	mutex_unlock(&v->host_mutex);
}

static void cleanup(struct work_struct *work)
{
	struct vtq_vce *vce;
	vtq_job_id_t our_rptr, their_rptr;
	uint32_t host_fifo_index;
	int distance;
	struct vtq_image *image;
	struct jobinfo *job;

	vce = container_of(work, struct vtq_vce, cleanup_work);

	if (vce->last_known_readptr - vce->last_acknowledged_job == 0)
		return;

	mutex_lock(&vce->cleanup_mutex);

	our_rptr = vce->last_acknowledged_job;
	their_rptr = vce->last_known_readptr;
	distance = their_rptr - our_rptr;

	if (distance == 0)
		dbg_print("Nothing to do in cleanup...\n");

	while (distance > 0) {
		host_fifo_index = our_rptr & (vce->host_fifo_length-1);
		job = &vce->runningjobs[host_fifo_index];

		if (vce->global->debug_fifo)
			printk(KERN_INFO "VTQ: CLEANUP: %u\n", our_rptr);

		image = job->inflightimage;
		if (image != NULL) {
			put_image(image);
			job->inflightimage = NULL;
		}
		our_rptr++;
		distance = their_rptr - our_rptr;
	}
	vce->last_acknowledged_job = our_rptr;

	mutex_unlock(&vce->cleanup_mutex);

	/* Still needs to be wakeupall, I think, as the first guys
	 * condition may not be met -- TODO: check this */
	wake_up_all(&vce->more_room_in_fifo_wq);
}

irqreturn_t vtq_isr(struct vtq_vce *vce)
{
	unsigned long flags;
	irqreturn_t ret;
	vtq_job_id_t our_rptr, their_rptr;
	int distance;

	/* First we do a fast-path check without the mutex: */
	if (!vce->on)
		return IRQ_NONE;

	ret = IRQ_NONE;

	spin_lock_irqsave(&vce->vce_mutex, flags);
	if (!vce->on) {
		spin_unlock_irqrestore(&vce->vce_mutex, flags);
		return IRQ_NONE;
	}
	our_rptr = vce->last_known_readptr;
	their_rptr = vce_readdata(&vce->io, vce->readptr_locn);
	distance = their_rptr - our_rptr;

	if (distance > 0) {
		ret = IRQ_HANDLED;

		vce->last_known_readptr = their_rptr;

		/* Wake up those waiting to hear that the job is
		 * complete -- must be wake_up_all as their may be
		 * several parties wanting to hear the news. */
		wake_up_all(&vce->job_complete_wq);

		schedule_work(&vce->cleanup_work);
		if (their_rptr == vce->writeptr &&
				vce->power_lock_count == 0)
			schedule_work(&vce->unload_work);
	}
	spin_unlock_irqrestore(&vce->vce_mutex, flags);

	if (ret != IRQ_HANDLED) {
		dbg_print("SPURIOUS INTERRUPT -- We're \"on\", "
		       "and received int, but had nothing to do\n");
	}

	return ret;
}

static int vtq_lock_unload(struct vtq_vce *v)
{
	mutex_lock(&v->host_mutex);
	v->power_lock_count += 1;
	mutex_unlock(&v->host_mutex);
	return 0;
}

static void vtq_unlock_unload(struct vtq_vce *v)
{
	int need_to_run_unload_work;

	mutex_lock(&v->host_mutex);
	v->power_lock_count -= 1;
	need_to_run_unload_work = (v->power_lock_count == 0);
	mutex_unlock(&v->host_mutex);
	if (need_to_run_unload_work)
		schedule_work(&v->unload_work);
}


/* **************** */

/*
 *
 *  CONTEXT
 *
 */

struct vtq_context {
	struct vtq_vce *vce;
};

struct vtq_context *vtq_create_context(struct vtq_vce *vce)
{
	struct vtq_context *ctx;

	ctx = kmalloc(sizeof(*ctx), GFP_KERNEL);
	if (ctx == NULL) {
		err_print("kmalloc failure\n");
		goto err_kmalloc;
	}

	ctx->vce = vce;

	/* success */
	return ctx;

	/* error exit paths follow */

	/* kfree(ctx); */
err_kmalloc:

		return NULL;
}

void vtq_destroy_context(struct vtq_context *ctx)
{
	kfree(ctx);
}

/* **************** */

/*
 *
 *  IMAGE
 *
 */

struct vtq_image *vtq_register_image(struct vtq_context *ctx,
		const uint32_t *text,
		size_t textsz,
		const uint32_t *data,
		size_t datasz,
		size_t datamemreq)
{
	struct vtq_image *image;

	mutex_lock(&ctx->vce->host_mutex);

	if (datamemreq > ctx->vce->datamem_reservation || datasz > datamemreq) {
		err_print("DATA memory requirement too big\n");
		goto err_bad_image_param;
	}
	if (textsz > ctx->vce->global->loaderkernel_loadoffset) {
		err_print("INSTRUCTION memory requirement too big\n");
		goto err_bad_image_param;
	}
	if (textsz > 0x2000 && !ctx->vce->global->host_push) {
		err_print("Images over 8kB are not supported currently\n");
		goto err_bad_image_param;
	}
	if (textsz & 3 || datasz & 3 || datamemreq & 3) {
		err_print("this image has a bad text or data size\n");
		goto err_bad_image_param;
	}

	image = kmalloc(sizeof(*image), GFP_KERNEL);
	if (image == NULL) {
		err_print("failed to allocate image\n");
		goto err_find_image_id;
	}

	image->bitmap = 0;

	image->dmainfo.sz = textsz + datasz;
	/* TODO: we shouldn't really need this extra allocation... how
	 * can we avoid it? */
	image->dmainfo.virtaddr =
		dma_alloc_coherent(NULL, textsz + datasz,
			&image->dmainfo.busaddr, GFP_DMA);
	if (image->dmainfo.virtaddr == NULL) {
		err_print("dma_alloc_coherent failed\n");
		goto err_dma_alloc;
	}
	image->textoffset = 0;
	image->textsz = textsz;
	image->dataoffset = textsz;
	image->datasz = datasz;

	{
		char *imagecopy;

		imagecopy = image->dmainfo.virtaddr;
		memcpy(imagecopy + image->textoffset, text, textsz);
		memcpy(imagecopy + image->dataoffset, data, datasz);
	}

	kref_init(&image->ref);

	mutex_unlock(&ctx->vce->host_mutex);

	/* success */

	return image;

	/*

	  error exit paths follow

	*/

	/* dma_free_coherent */
err_dma_alloc:

	kfree(image);
err_find_image_id:
err_bad_image_param:
	mutex_unlock(&ctx->vce->host_mutex);

	return NULL;
}

static void cleanup_image(struct kref *ref)
{
	struct vtq_image *image;

	image = container_of(ref, struct vtq_image, ref);

	dma_free_coherent(NULL,
			  image->dmainfo.sz,
			  image->dmainfo.virtaddr,
			  image->dmainfo.busaddr);

	kfree(image);
}

static void put_image(struct vtq_image *image)
{
	kref_put(&image->ref, cleanup_image);
}

static void get_image(struct vtq_image *image)
{
	kref_get(&image->ref);
}

void vtq_unregister_image(struct vtq_context *ctx, struct vtq_image *image)
{
	struct vtq_vce *v;

	v = ctx->vce;
	mutex_lock(&v->host_mutex);

	/* TODO: here we should go through and remove out entrypoints
	 * from tasks.  At the moment, they'd just keep a reference to
	 * us and the image will hang around.  We mandate in the API
	 * that tasks should be destroyed before images anyway, so
	 * there's no need to put this right right now. */

	if (v->retained_image == image) {
		put_image(v->retained_image);
		v->retained_image = NULL;
	}
	if (v->current_image == image) {
		/* this is just to be neat and tidy... it would sort
		 * itself out if we didn't do this now, but while
		 * we've got the lock for clearing out the retained
		 * one, we may as well clear up here too */
		put_image(v->current_image);
		v->current_image = NULL;
	}

	mutex_unlock(&v->host_mutex);

	put_image(image);
}

/* **************** */

/*
 *
 *  TASK TYPE
 *
 */

static void vtq_add_task_entrypt(struct vtq_context *ctx,
		vtq_task_id_t prog_id,
		struct vtq_image *image,
		uint32_t entrypt)
{
	struct image_list *l;

	/* We need the host mutex while we modify the bitmap, as the
	 * queue and complete functions will trust this data */
	mutex_lock(&ctx->vce->host_mutex);
	get_image(image);
	image->bitmap |= (1<<prog_id);

	l = kmalloc(sizeof(*l), GFP_KERNEL);
	l->image = image;
	l->next = ctx->vce->tasks[prog_id].suitable_images;
	ctx->vce->tasks[prog_id].suitable_images = l;

	image->entrypts[prog_id] = entrypt;
	mutex_unlock(&ctx->vce->host_mutex);
}

static void vtq_remove_all_entrypts(struct vtq_context *ctx,
		vtq_task_id_t prog_id)
{
	struct vtq_image *image;
	struct image_list *image_list;
	struct image_list *next;

	mutex_lock(&ctx->vce->host_mutex);
	image_list = ctx->vce->tasks[prog_id].suitable_images;
	ctx->vce->tasks[prog_id].suitable_images = NULL;

	while (image_list != NULL) {
		image = image_list->image;
		next = image_list->next;
		kfree(image_list);
		image_list = next;
		image->bitmap &= ~(1<<prog_id);
		put_image(image);
	}
	mutex_unlock(&ctx->vce->host_mutex);
}

static vtq_task_id_t vtq_assign_prog_id(struct vtq_context *ctx)
{
	int prog_id;

	mutex_lock(&ctx->vce->host_mutex);
	for (prog_id = 0; prog_id < VTQ_MAX_TASKS; prog_id++) {
		if (!ctx->vce->tasks[prog_id].occupied) {
			ctx->vce->tasks[prog_id].occupied = 1;
			ctx->vce->tasks[prog_id].suitable_images = NULL;
			mutex_unlock(&ctx->vce->host_mutex);
			return prog_id;
		}
	}

	/* Argh! Out of task IDs */
	mutex_unlock(&ctx->vce->host_mutex);
	return -1;
}

static void vtq_free_prog_id(struct vtq_context *ctx, vtq_task_id_t prog_id)
{
	ctx->vce->tasks[prog_id].occupied = 0;
}

vtq_task_id_t vtq_create_task(struct vtq_context *ctx,
			      struct vtq_image *image,
			      uint32_t entrypt)
{
	vtq_task_id_t prog_id;

	prog_id = vtq_assign_prog_id(ctx);
	if (prog_id < 0)
		return prog_id;

	vtq_add_task_entrypt(ctx, prog_id, image, entrypt);

	return prog_id;
}

void vtq_destroy_task(struct vtq_context *ctx, vtq_task_id_t prog_id)
{
	vtq_remove_all_entrypts(ctx, prog_id);
	vtq_free_prog_id(ctx, prog_id);
}

/* **************** */

/*
 *
 *  JOB
 *
 */
vtq_job_id_t vtq_get_read_pointer(struct vtq_context *ctx)
{
	/* We assume atomic read so don't need to acquire vce_mutex
	 * while we read this here, and it doesn't matter to caller if
	 * this news is old by the time they get it. */

	return ctx->vce->last_known_readptr;
}

static int got_room_for_job(struct vtq_context *ctx, vtq_task_id_t task_id)
{
	int njobsinvceq, njobsinhostq;
	int roominvce, roomonhost, room, room_required;
	uint32_t curimagebitmap;
	int imagehasentrypt, haveenoughroom;

	mutex_lock(&ctx->vce->host_mutex);

	njobsinvceq = ctx->vce->writeptr - ctx->vce->last_known_readptr;
	njobsinhostq = ctx->vce->writeptr - ctx->vce->last_acknowledged_job;
	roominvce = ctx->vce->vce_fifo_length - njobsinvceq;
	roomonhost = ctx->vce->host_fifo_length - njobsinhostq;
	room = roominvce < roomonhost ? roominvce : roomonhost;

	if (ctx->vce->current_image != NULL) {
		curimagebitmap =
			ctx->vce->current_image->bitmap;
		imagehasentrypt =
			curimagebitmap & (1<<task_id);
	} else {
		imagehasentrypt = 0;
	}

	/* in host-push mode, we never mix task types in the FIFO */
	if (!imagehasentrypt &&
	    ctx->vce->global->host_push &&
	    njobsinvceq > 0) {
		haveenoughroom = 0;
	} else {
		/* NB.  We *may* need room for extra on-load
		   entries... we cannot know at this time, we have to
		   assume the worse case and if we can optimize away
		   later, that's good, but we must not assume it in
		   case power is lost to VCE.  TODO: perhaps it would
		   be nicer to increment the clock count before this
		   point so that we can know this, but let's not make
		   it complicated unless this proves to be a
		   performance issue */
		room_required = (imagehasentrypt ? 1 : 3
			+ ctx->vce->onloadhook_count);
		haveenoughroom = room >= room_required;
	}

	if (!haveenoughroom)
		mutex_unlock(&ctx->vce->host_mutex);

	return haveenoughroom;
}

static void q(struct vtq_vce *v,
	      uint32_t pc,
	      uint32_t r1,
	      uint32_t r2,
	      uint32_t r3,
	      uint32_t r4,
	      uint32_t r5,
	      uint32_t r6)
{
	uint32_t vce_fifo_index;
	uint32_t datamemaddress;

	BUG_ON(v->writeptr + 1 - v->last_known_readptr > v->vce_fifo_length);
	vce_fifo_index = v->writeptr & (v->vce_fifo_length-1);
	datamemaddress = v->circbuf_locn
		+ v->fifo_entrysz * vce_fifo_index;

	vce_writedata(&v->io, datamemaddress, pc);
	vce_writedata(&v->io, datamemaddress+4, r1);
	vce_writedata(&v->io, datamemaddress+8, r2);
	vce_writedata(&v->io, datamemaddress+12, r3);
	vce_writedata(&v->io, datamemaddress+16, r4);
	vce_writedata(&v->io, datamemaddress+20, r5);
	vce_writedata(&v->io, datamemaddress+24, r6);
	if (v->global->debug_fifo) {
		printk(KERN_INFO "VTQ: Q: %u: "
				"0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X\n",
				v->writeptr,
				pc, r1, r2, r3, r4, r5, r6);
	}

	v->writeptr++;
}

static void q_hooks(struct vtq_vce *v,
		struct vtq_hook_list_node *hooks)
{
	while (hooks != NULL) {
		q(v,
				hooks->pc,
				hooks->r1,
				hooks->r2,
				hooks->r3,
				hooks->r4,
				hooks->r5,
				hooks->r6);
		hooks = hooks->next;
	}
}

int vtq_queue_job(struct vtq_context *ctx,
		vtq_task_id_t task_id,
		vtq_job_arg_t arg0,
		vtq_job_arg_t arg1,
		vtq_job_arg_t arg2,
		vtq_job_arg_t arg3,
		vtq_job_arg_t arg4,
		vtq_job_arg_t arg5,
		uint32_t flags_do_not_use_me,
		vtq_job_id_t *job_id_out)
{
	int curimagehasentrypt;
	uint32_t curimagebitmap;
	uint32_t job_entrypoint;
	vtq_job_id_t next_job_id;
	uint32_t host_fifo_index;
	struct vtq_image *new_image;
	struct jobinfo *job;
	int s;

	(void) flags_do_not_use_me;

	/* got_room_for_job() would wait forever if we haven't had the
	 * one-time configuration to set up the FIFO.  We can be
	 * slightly more proactive by checking for a non-configured
	 * FIFO here.  It's not an exact science, as we don't have the
	 * mutex, but this is abnormal condition anyway, so we can
	 * just accept that behavior.  In any case, it would only be a
	 * problem if we allow deconfiguring or reconfiguring of the
	 * FIFO, which we currently do not. */
	if (!ctx->vce->is_configured) {
		err_print("Not configured yet.  Cannot queue job\n");
		return -1;
	}

	atomic_inc(&ctx->vce->queuerswaiting);
	s = wait_event_interruptible(ctx->vce->more_room_in_fifo_wq,
				     got_room_for_job(ctx, task_id));
	if (s) {
		dbg_print("wait_event_interruptible was interrupted\n");
		return s;
	}
	/* got room for job will leave the host_mutex acquired when
	 * the condition is met */
	atomic_dec(&ctx->vce->queuerswaiting);

	/* spin lock is unlocked so isr can run, but isr won't turn us
	 * off now (because we said so above) and we are the only ones
	 * allowed to turn on, so this test should be safe */
	if (!ctx->vce->on) {
		_loadloader(ctx->vce);
		if (!ctx->vce->on) {
			/* Not sure whether this can ever happen, but
			 * it's certainly bad if it ever does.  I
			 * guess it can happen if we got a signal
			 * while trying to acquire VCE?  TODO: handle
			 * this case */
			err_print("oh no. :(\n");
			mutex_unlock(&ctx->vce->host_mutex);
			return -EINVAL;
		}
	}

	/* Insert a load-image if req'd */
	if (ctx->vce->current_image != NULL) {
		curimagebitmap =
			ctx->vce->current_image->bitmap;
		curimagehasentrypt =
			curimagebitmap & (1<<task_id);
	} else {
		curimagehasentrypt = 0;
	}
	if (!curimagehasentrypt && !ctx->vce->global->host_push) {
		uint32_t imagehwaddr;

		BUG_ON(!ctx->vce->on);

		q_hooks(ctx->vce, ctx->vce->onloadhook_list_head);

		/* TODO: find_image_for_prog(prog_id); - or similar --
		 * for now, we just use first one... we assume there
		 * is one!  This is fine for now, but we'll eventually
		 * expect that a single task can be served by more
		 * than one image. */
		BUG_ON(ctx->vce->tasks[task_id].suitable_images == NULL);
		BUG_ON(ctx->vce->tasks[task_id].suitable_images->image == NULL);
		new_image = ctx->vce->tasks[task_id].suitable_images->image;
		get_image(new_image);
		host_fifo_index = ctx->vce->writeptr
			& (ctx->vce->host_fifo_length-1);
		job = &ctx->vce->runningjobs[host_fifo_index];
		job->inflightimage = new_image;
		imagehwaddr = new_image->dmainfo.busaddr;
		q(ctx->vce,
			ctx->vce->global->loaderkernel_loadimage_entrypoint,
			0, imagehwaddr + new_image->textoffset,
			new_image->textsz >> 2,
			0, imagehwaddr + new_image->dataoffset,
			new_image->datasz >> 2);

		if (ctx->vce->current_image != NULL)
			put_image(ctx->vce->current_image);
		get_image(new_image);
		ctx->vce->current_image = new_image;
	}
	if (!curimagehasentrypt && ctx->vce->global->host_push) {
		BUG_ON(!ctx->vce->on);
		BUG_ON(ctx->vce->last_known_readptr != ctx->vce->writeptr);
		BUG_ON(ctx->vce->tasks[task_id].suitable_images == NULL);
		BUG_ON(ctx->vce->tasks[task_id].suitable_images->image == NULL);
		new_image = ctx->vce->tasks[task_id].suitable_images->image;
		vce_download_program_at(ctx->vce,
				0,
				(const uint32_t *)new_image->dmainfo.virtaddr
					+ (new_image->textoffset>>2),
				new_image->textsz);
		vce_download_data_at(ctx->vce,
				0,
				(const uint32_t *)new_image->dmainfo.virtaddr
					+ (new_image->dataoffset>>2),
				new_image->datasz);
		wmb();

		if (ctx->vce->current_image != NULL)
			put_image(ctx->vce->current_image);
		get_image(new_image);
		ctx->vce->current_image = new_image;
		if (ctx->vce->global->debug_fifo)
			printk(KERN_INFO "VTQ: Image Push: %p\n", new_image);
	}

	host_fifo_index = ctx->vce->writeptr
		& (ctx->vce->host_fifo_length-1);
	job = &ctx->vce->runningjobs[host_fifo_index];
	BUG_ON(job->inflightimage != NULL);

	BUG_ON(!ctx->vce->on);

	job_entrypoint =
		ctx->vce->current_image->entrypts[task_id];

	q(ctx->vce, job_entrypoint, arg0, arg1, arg2, arg3, arg4, arg5);
	wmb();
	vce_writedata(&ctx->vce->io,
		ctx->vce->writeptr_locn, ctx->vce->writeptr);
	next_job_id = ctx->vce->writeptr;

	vce_clearsema(&ctx->vce->io, ctx->vce->semanum);

	mutex_unlock(&ctx->vce->host_mutex);

	wake_up_all(&ctx->vce->vce_given_work_wq);

	if (job_id_out != NULL)
		*job_id_out = next_job_id;

	return 0;
}

static int job_is_complete(struct vtq_vce *v, vtq_job_id_t job)
{
	int distance;
	vtq_job_id_t lkrptr;

	lkrptr = v->last_known_readptr;
	distance = lkrptr - job;
	return distance >= 0;
}

int vtq_await_job(struct vtq_context *ctx, vtq_job_id_t job)
{
	int s;

	s = wait_event_interruptible(ctx->vce->job_complete_wq,
				     job_is_complete(ctx->vce, job));
	if (s) {
		dbg_print("wait_event_interruptible was interrupted\n");
		return s;
	}

	return 0;
}

/* MULTI PURPOSE LOCK -- TODO: MOVE TO CLIENT API */

void vtq_unlock_multi(struct vtq_context *ctx,
		uint32_t locks_to_put)
{
	(void)ctx;

	if (locks_to_put & VTQ_LOCK_PRIORITY_LOCK_LOW)
		vtq_util_priority_unlock(ctx->vce->priority_lock, 0);

	if (locks_to_put & VTQ_LOCK_PRIORITY_LOCK_MED)
		vtq_util_priority_unlock(ctx->vce->priority_lock, 1);

	if (locks_to_put & VTQ_LOCK_POWERLOCK)
		vtq_unlock_unload(ctx->vce);
}

int vtq_lock_multi(struct vtq_context *ctx,
		uint32_t locks_to_get)
{
	uint32_t locks_got;
	int s;

	if (locks_to_get & VTQ_LOCK_PRIORITY_LOCK_LOW &&
	    locks_to_get & VTQ_LOCK_PRIORITY_LOCK_MED)
		return -1;

	locks_got = 0;

	if (locks_to_get & VTQ_LOCK_PRIORITY_LOCK_LOW) {
		s = vtq_util_priority_lock(ctx->vce->priority_lock, 0);
		if (s)
			goto lock_error;
		locks_got |= VTQ_LOCK_PRIORITY_LOCK_LOW;
	}

	if (locks_to_get & VTQ_LOCK_PRIORITY_LOCK_MED) {
		s = vtq_util_priority_lock(ctx->vce->priority_lock, 1);
		if (s)
			goto lock_error;
		locks_got |= VTQ_LOCK_PRIORITY_LOCK_MED;
	}

	if (locks_to_get & VTQ_LOCK_POWERLOCK) {
		s = vtq_lock_unload(ctx->vce);
		if (s)
			goto lock_error;
		locks_got |= VTQ_LOCK_POWERLOCK;
	}

	if (locks_to_get == locks_got)
		return 0;

lock_error:
	vtq_unlock_multi(ctx, locks_got);

	return -1;
}

/* ******************** */
