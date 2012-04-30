#ifndef VTQBR_H
#define VTQBR_H

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

#include <linux/types.h>
#include <linux/broadcom/vtq.h>

struct vtqb_context;

/*
 * All function here are a bridge / proxy layer to the real vtq_*
 * functions -- see vtq.h for documentation.  This layer adds cleanup
 * to the original functions.  When a context is destroyed, the jobs,
 * tasks and images are first cleaned up before destroying the context
 *
 * this is for file-descriptor disconnect in the User Mode IOCTL
 * bridge
 */

/*
 * N.B.  This is intended to protect against buggy apps that abort
 * without closing cleanly.  We do not protect against malicious apps.
 */

extern struct vtqb_context *vtqb_create_context(struct vtq_vce *vce);
extern void vtqb_destroy_context(struct vtqb_context *ctx);
#define VTQ_MAX_PROXY_IMAGES 8
typedef int vtq_proxy_image_id_t;
extern vtq_proxy_image_id_t vtqb_register_image(struct vtqb_context *ctx,
		const uint32_t *text,
		size_t textsz,
		const uint32_t *data,
		size_t datasz,
		size_t datamemreq);
extern void vtqb_unregister_image(struct vtqb_context *ctx,
		vtq_proxy_image_id_t image_id);
extern vtq_task_id_t vtqb_create_task(struct vtqb_context *ctx,
		vtq_proxy_image_id_t image_id,
		uint32_t entrypt);
extern void vtqb_destroy_task(struct vtqb_context *ctx,
		vtq_task_id_t task_id);
extern vtq_status_t vtqb_queue_job(struct vtqb_context *ctx,
		vtq_task_id_t task_id,
		vtq_job_arg_t arg0,
		vtq_job_arg_t arg1,
		vtq_job_arg_t arg2,
		vtq_job_arg_t arg3,
		vtq_job_arg_t arg4,
		vtq_job_arg_t arg5,
		uint32_t flags_do_not_use_me,
		vtq_job_id_t *job_id_out);
extern vtq_status_t vtqb_await_job(struct vtqb_context *ctx,
		vtq_job_id_t job);
extern void vtqb_unlock_multi(struct vtqb_context *ctx,
		uint32_t locks_to_put);
extern int vtqb_lock_multi(struct vtqb_context *ctx,
		uint32_t locks_to_get);

#endif
