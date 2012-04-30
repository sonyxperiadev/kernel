#ifndef BRCM_VTQ_H
#define BRCM_VTQ_H

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

/* VTQ = VCE Task Queue */

#include <linux/irqreturn.h>
#include <linux/types.h>

struct vce;
struct vtq_global;
struct vtq_vce;
typedef int vtq_status_t;

/* i/f to vce-driver top lrbrl mofulr */

extern int vtq_driver_init(struct vtq_global **vtq_global_state_out,
		struct vce *vce,
		struct proc_dir_entry *proc_vcedir);
extern void vtq_driver_term(struct vtq_global **vtq_global_state_ptr);
extern int vtq_pervce_init(struct vtq_vce **vtq_pervce_state,
		struct vtq_global *vtq_global_state,
		struct vce *vce,
		struct proc_dir_entry *proc_vcedir);
extern void vtq_pervce_term(struct vtq_vce **vtq_pervce_state);

extern irqreturn_t vtq_isr(struct vtq_vce *vtq_pervce_state);

/* context */

extern struct vtq_context *vtq_create_context(struct vtq_vce *vtq_pervce_state);
extern void vtq_destroy_context(struct vtq_context *ctx);

/* image */

/* vtq_register_image args:

   text, textsz -- the binary to be loaded to the instruction memory

   data, datasz -- the binary to be loaded to the data memory (can be
       NULL, 0 if appropriate)

   datamemreq -- the amount of data memory that this image plans to
       use, worst case.  This is just so that we can refuse an image
       that is not compatible with the current FIFO configuration
       (i.e. would spill over into the reserved memory)
*/
extern struct vtq_image *vtq_register_image(struct vtq_context *ctx,
		const uint32_t *text,
		size_t textsz,
		const uint32_t *data,
		size_t datasz,
		size_t datamemreq);
extern void vtq_unregister_image(struct vtq_context *ctx,
		struct vtq_image *);

/* task */

/* please, always create the image before the task, and always destroy
 * the task before the image */

typedef int vtq_task_id_t;
extern vtq_task_id_t vtq_create_task(struct vtq_context *ctx,
		struct vtq_image *image,
		uint32_t entrypt);
extern void vtq_destroy_task(struct vtq_context *ctx,
		vtq_task_id_t task_id);

/* TODO: we should have limitless (linkedlist of) tasktypes */
#define VTQ_MAX_TASKS 16

/* job */

/* please, always create the task before queueing the job, and wait
 * for the job to complete before destroying the task */

/* N.B.  vtq_queue_job and vtq_await_job will block (TODO: consider
 * making non-blocking version?  or take arg to say don't wait?  or
 * timeout?) until the job gets into the queue (other jobs may have to
 * complete to make space) or until the specified job is completed.
 * These can fail, because we can get a signal while waiting.  The
 * caller should check the return code to make sure the job was
 * successfully queued or completed */

typedef uint32_t vtq_job_id_t;
typedef uint32_t vtq_job_arg_t;
extern vtq_status_t vtq_queue_job(struct vtq_context *ctx,
		vtq_task_id_t task_id,
		vtq_job_arg_t arg0,
		vtq_job_arg_t arg1,
		vtq_job_arg_t arg2,
		vtq_job_arg_t arg3,
		vtq_job_arg_t arg4,
		vtq_job_arg_t arg5,
		uint32_t flags_do_not_use_me,
		vtq_job_id_t *job_id_out);

extern vtq_status_t vtq_await_job(struct vtq_context *ctx,
		vtq_job_id_t job);
extern vtq_job_id_t vtq_get_read_pointer(struct vtq_context *ctx);

/* multi purpose lock */

/* we have a number of usecases that require functionality that VTQ is
 * not supposed to provide, but, that the higher level APIs for
 * providing them don't yet exist.  So, this is just a bag of
 * workarounds for missing functionality elsewhere.  We'd like to
 * think we can one day deprecate these, one by one, as the missing
 * functionality begins to appear */

/* unlock cannot fail. we just do as we are told */
extern void vtq_unlock_multi(struct vtq_context *ctx,
		uint32_t locks_to_put);
/* some locks can block, some locks can fail without blocking.  If a
 * lock fails, we'll put things back to how we found them */
extern int vtq_lock_multi(struct vtq_context *ctx,
		uint32_t locks_to_get);

#endif
