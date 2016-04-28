/*****************************************************************************
* Copyright 2013 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*****************************************************************************/

/*
 * Driver for Broadcom's Video Control Engine.
 */

#ifndef _VCE_H_
#define _VCE_H_

#include <linux/types.h>
#include <linux/ioctl.h>
#include "hva.h"

#define VCE_DEV_NAME	    "vce"
#define VCE_IOC_MAGIC	    ((('V' << 3) ^ ('C' << 1) ^ 'E') & _IOC_TYPEMASK)
#define VCE_NR_REGISTERS    63

enum vce_job_status {
	VCE_JOB_STATUS_SUCCESS,
	VCE_JOB_STATUS_ERROR,
	VCE_JOB_STATUS_TIMEOUT
};

struct vce_regset {
	__u32 regs[VCE_NR_REGISTERS];
	__u64 changed_mask;
};

/*
 * Definition of a program that is run on the VCE.
 * This specifies the code to be loaded into VCE program memory.
 * The data is specified as part of the job.
 */
struct vce_program_def {
	void *code;			/* Code executed on the VCE */
	__u32 code_size;		/* Must be non-zero */
	__u32 start_addr;		/*
					 * Start address inside the program, as
					 * an offset from the start of 'code'.
					 */
	__u32 final_addr;		/*
					 * Final address inside the program, as
					 * an offset from the start of 'code'.
					 */
};

struct vce_version {
	__u16 spec_revision;
	__u16 sub_revision;
};

struct vce_register_prog_params {
	__u32 prog_id;			/* [out] */
	struct vce_program_def prog;	/* [in] */
};

struct vce_deregister_prog_params {
	__u32 prog_id;			/* [in] */
};

struct vce_job_params {
	__u32 job_id;			/* [out] */
	__u32 prog_id;
	struct vce_regset regset;
	unsigned long data_addr;	/*
					 * Physical address for data to be
					 * loaded into VCE data memory.
					 */
	__u32 data_size;		/* May be zero */
	__u32 end_code;			/*
					 * Expected stopping condition for vce:
					 * 0:    bkpt
					 * 1..7: wait on user semaphore
					 */
	__u32 upload_start;		/*
					 * Start of range for data upload
					 * after program execution.
					 */
	__u32 upload_size;
	const struct hva_dec_info *dec;	/* may be NULL */
	const struct hva_enc_info *enc;	/* may be NULL */
};

struct vce_wait_params {
	__u32 job_id;			/* [out] */
	__u32 status;			/* [out] vce_job_status */
	__u32 regs[VCE_NR_REGISTERS];	/* [out] */
	struct hva_enc_info *enc;	/* [in/out] */
};

/* Get VCE processor version */
#define VCE_IOCTL_GET_VERSION _IOR(VCE_IOC_MAGIC, 1, struct vce_version)

/* Register a VCE program */
#define VCE_IOCTL_REGISTER_PROG \
	_IOWR(VCE_IOC_MAGIC, 10, struct vce_register_prog_params)

/* De-register a VCE program */
#define VCE_IOCTL_DEREGISTER_PROG \
	_IOW(VCE_IOC_MAGIC, 11, struct vce_deregister_prog_params)

/* De-register all VCE programs */
#define VCE_IOCTL_DEREGISTER_ALL_PROGS _IO(VCE_IOC_MAGIC, 12)

/* Post to the VCE job queue */
#define VCE_IOCTL_POST_JOB _IOWR(VCE_IOC_MAGIC, 20, struct vce_job_params)

/*
 * Returns the results of the next available completed job.
 * If there are no outstanding jobs, returns immediately with job_id set to
 * 0, and the other fields are unmodified.
 * Otherwise, blocks until the next job completes or times out. job_id will be
 * set to a non-zero value in this case, and the other output fields will be
 * set.
 * The driver imposes a timeout on jobs, so this will not block indefinitely.
 */
#define VCE_IOCTL_WAIT_JOB _IOWR(VCE_IOC_MAGIC, 21, struct vce_wait_params)

#endif
