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

#include "Statistics.h"

#define JOB_TIMEOUT_MS 500


struct V3dDriverTag;
struct V3dDriver_JobTag;

struct V3dSessionTag;
typedef struct V3dSessionTag V3dSessionType;

struct V3dSessionTag {
	unsigned int Initialised;

	struct V3dDriverTag *Driver;
	const char          *Name;

	int32_t        LastId;

	struct {
		/* All jobs issued but not yet completed, in posted order */
		struct list_head List;
		spinlock_t       Lock;
	} Issued;

	ktime_t        Start;
	uint32_t       TotalRun;
	struct {
		StatisticsType Queue;
		StatisticsType Run;
	} BinRender;
	struct {
		StatisticsType Queue;
		StatisticsType Run;
	} User;
};

extern V3dSessionType *V3dSession_Create(struct V3dDriverTag *Driver, const char *Name);
extern void            V3dSession_Delete(V3dSessionType *Instance);

/* V3dDriver Interface */
extern void            V3dSession_AddStatistics(V3dSessionType *Instance, int User, unsigned int Queue, unsigned int Run);
extern void            V3dSession_ResetStatistics(V3dSessionType *Instance);
extern void            V3dSession_Issued(struct V3dDriver_JobTag *Job);
extern void            V3dSession_Complete(struct V3dDriver_JobTag *Job);

/* External interface */
extern int V3dSession_JobPost(
	V3dSessionType *Instance,
	const v3d_job_post_t *UserJob);

/* Block until the last job submitted for this session completes */
extern int32_t V3dSession_Wait(V3dSessionType *Instance);


#endif /* ifndef V3D_SESSION_H_ */
