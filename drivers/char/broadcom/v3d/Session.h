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

#include "Driver.h"

#define JOB_TIMEOUT_MS 500


struct V3dSessionTag;
typedef struct V3dSessionTag V3dSessionType;

struct V3dSessionTag {
	unsigned int Initialised;

	V3dDriverType *Driver;

	int32_t        LastId;

	struct {
		/* All jobs issued but not yet completed, in posted order */
		struct list_head List;
		spinlock_t       Lock;
	} Issued;
};

extern V3dSessionType *V3dSession_Create(V3dDriverType *Driver);
extern void            V3dSession_Delete(V3dSessionType *Instance);

/* V3dDriver Interface */
extern void            V3dSession_Issued(V3dDriver_JobType *Job);
extern void            V3dSession_Complete(V3dDriver_JobType *Job);

/* External interface */
static inline int      V3dSession_JobPost(
	V3dSessionType *Instance,
	const v3d_job_post_t *UserJob)
{
	Instance->LastId = UserJob->job_id;
	return V3dDriver_JobPost(Instance->Driver, Instance, UserJob);
}

/* Block until the last job submitted for this session completes */
extern int32_t         V3dSession_Wait(V3dSessionType *Instance);


#endif /* ifndef V3D_SESSION_H_ */
