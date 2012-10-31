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

#ifndef V3D_SESSION_H_
#define V3D_SESSION_H_

#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/broadcom/v3d.h>

#include "statistics.h"

#define JOB_TIMEOUT_MS 1000


struct v3d_driver_tag;
struct v3d_driver_job_tag;

struct v3d_session_tag;
typedef struct v3d_session_tag v3d_session_type;

struct v3d_session_tag {
	unsigned int initialised;

	struct v3d_driver_tag *driver;
	const char          *name;

	int32_t        last_id;

	struct {
		/* All jobs issued but not yet completed, in posted order */
		struct list_head list;
		spinlock_t       lock;
	} issued;

	ktime_t        start;
	uint32_t       total_run;
	struct {
		statistics_type queue;
		statistics_type run;
	} bin_render;
	struct {
		statistics_type queue;
		statistics_type run;
	} user;
};

extern v3d_session_type *v3d_session_create(struct v3d_driver_tag *driver, const char *name);
extern void            v3d_session_delete(v3d_session_type *instance);

/* V3dDriver Interface */
extern void            v3d_session_add_statistics(v3d_session_type *instance, int user, unsigned int queue, unsigned int run);
extern void            v3d_session_reset_statistics(v3d_session_type *instance);
extern void            v3d_session_issued(struct v3d_driver_job_tag *job);
extern void            v3d_session_complete(struct v3d_driver_job_tag *job, int status);

/* External interface */
extern int v3d_session_job_post(
	v3d_session_type *instance,
	const v3d_job_post_t *user_job);

/* Block until the last job submitted for this session completes */
extern int32_t v3d_session_wait(v3d_session_type *instance);


#endif /* ifndef V3D_SESSION_H_ */
