#ifndef BRCM_VTQ_IMAGECONV_H
#define BRCM_VTQ_IMAGECONV_H

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

#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/irqreturn.h>
#include <linux/module.h>
#include <linux/types.h>

struct vtq_vce;
struct vtq_imageconv_state;

/* i/f to vce-driver top level module */

extern int vtq_imageconv_init(
		struct vtq_imageconv_state **vtq_imageconv_state_out,
		struct vtq_vce *vtq,
		struct module *module,
		struct device *device,
		struct proc_dir_entry *proc_vcedir);
extern void vtq_imageconv_term(
		struct vtq_imageconv_state **vtq_imageconv_state_ptr);

/* This is a hacky way to load the firmware via ioctl... the init
 * method is preferred */
extern int vtq_imageconv_supply_blob_via_vtqinit(
		struct vtq_imageconv_state *common,
		const uint32_t *blob, int blobsz);

/*
 * vtq_imageconv_ready:
 *
 * returns 0 if vtq imageconv firmware has not been received
 *
 * returns 1 if the firmware is received, the tasks are set up, and we
 * will be ready to service requests to convert images
 */
extern int vtq_imageconv_ready(struct vtq_imageconv_state *c);

/* This API is for when we already have the bus address.  There should
 * be an API that takes a pmem handle, and offset, or some such, but
 * no user for such an API exists for us at the moment.
 */
extern int vtq_imageconv_enqueue_direct(struct vtq_imageconv_state *c,
		uint32_t type,
		uint32_t tformat_baseaddr,
		uint32_t raster_baseaddr,
		int32_t signedrasterstride,
		uint32_t numtiles_wide,
		uint32_t numtiles_high,
		uint32_t dmacfg,
		vtq_job_id_t *job_id);

/*
 * vtq_imageconv_await
 *
 * waits for the given image conversion job to complete in VTQ
 */
extern int vtq_imageconv_await(struct vtq_imageconv_state *c,
		vtq_job_id_t job_id);

#endif
