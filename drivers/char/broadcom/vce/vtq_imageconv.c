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

#include <linux/device.h>
#include <linux/errno.h>
#include <linux/firmware.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/slab.h>

#include <linux/broadcom/vce.h>
#include <linux/broadcom/vtq.h>
#include <linux/broadcom/vtq_imageconv.h>

struct vtq_imageconv_state {
	struct vtq_context *vtq;
	struct vtq_image *image;
	int want_image_via_vtqinit;
	vtq_task_id_t task_direct;
	vtq_task_id_t task_bgr24;
	atomic_t task_get_count;
	struct device *device;
};

#define VTQ_IMAGECONV_DEBUG
#define err_print(fmt, arg...) \
	printk(KERN_ERR "VTQ: ERROR: %s() %s:%d " fmt, \
		__func__, "vtq_imageconv.c", __LINE__, ##arg)
#ifdef VTQ_DEBUG
#define dbg_print(fmt, arg...) \
	printk(KERN_WARNING "VTQ: debug: %s() %s:%d " fmt, \
		__func__, "vtq_imageconv.c", __LINE__, ##arg)
#else
#define dbg_print(fmt, arg...)	do { } while (0)
#endif

static int vtq_imageconv_install_blob(struct vtq_imageconv_state *common,
		const uint32_t *blob, size_t blob_sz)
{
	const struct header {
		uint32_t magic;
		uint32_t text_offset;
		uint32_t text_length;
		uint32_t data_offset;
		uint32_t data_length;
		uint32_t datamem_requirement;
		uint32_t pc_task_direct;
		uint32_t pc_task_bgr24;
	} *header;

	struct vtq_image *image;

	if (common->image != NULL)
		return -1;

	if (blob_sz < sizeof(*header))
		return -1;

	header = (const struct header *)blob;

	if (header->magic != 0x514243)
		return -1;

	if (header->text_offset + header->text_length > blob_sz)
		return -1;

	if (header->data_offset + header->data_length > blob_sz)
		return -1;

	if (header->text_offset & (sizeof(*blob)-1))
		return -1;

	if (header->data_offset & (sizeof(*blob)-1))
		return -1;

	image = vtq_register_image(common->vtq,
			blob + header->text_offset/sizeof(*blob),
			header->text_length,
			blob + header->data_offset/sizeof(*blob),
			header->data_length,
			header->datamem_requirement);
	if (image == NULL)
		return -1;

	common->task_direct = vtq_create_task(common->vtq,
			image,
			header->pc_task_direct);
	common->task_bgr24 = vtq_create_task(common->vtq,
			image,
			header->pc_task_bgr24);

	common->image = image;

	return 0;
}

static void _fw_cont(const struct firmware *fw, void *priv)
{
	struct vtq_imageconv_state *common;
	int s;

	common = priv;

	if (fw) {
		s = vtq_imageconv_install_blob(common,
				(const uint32_t *)fw->data, fw->size);
		release_firmware(fw);
		if (s != 0)
			err_print("Failed to install VTQ image for imageconv\n")
				;
	} else {
		dbg_print("No VTQ image found for imageconv\n");
		common->want_image_via_vtqinit = 1;
	}
}

int vtq_imageconv_supply_blob_via_vtqinit(
		struct vtq_imageconv_state *common,
		const uint32_t *blob, int blobsz)
{
	int s;

	/* silently ignore if we already have it */
	if (!common->want_image_via_vtqinit)
		return 0;

	s = vtq_imageconv_install_blob(common, blob, blobsz);
	if (s == 0)
		common->want_image_via_vtqinit = 0;
	return s;
}

int vtq_imageconv_init(struct vtq_imageconv_state **vtq_imageconv_out,
		struct vtq_vce *vtq_vce,
		struct module *module,
		struct device *device,
		struct proc_dir_entry *proc_vcedir)
{
	struct vtq_imageconv_state *out;
	int s;

	(void) proc_vcedir;

	out = kmalloc(sizeof(*out), GFP_KERNEL);
	if (out == NULL) {
		err_print("kmalloc failure\n");
		goto err_kmalloc_vtq_imageconv_state;
	}

	out->vtq = vtq_create_context(vtq_vce);
	if (out->vtq == NULL) {
		err_print("failed to create VTQ context\n");
		goto err_vtq_create_context;
	}

	/* Mark that we haven't received the blob (via
	 * request_firmware) yet */
	out->image = NULL;
	out->want_image_via_vtqinit = 0;
	atomic_set(&out->task_get_count, 0);

	out->device = device;
	s = request_firmware_nowait(module, 1,
			"bcmvtq_imageconv", out->device,
			GFP_KERNEL, out, _fw_cont);
	if (s != 0) {
		err_print("request firmware failed\n");
		goto err_request_firmware;
	}

	/* success */
	*vtq_imageconv_out = out;
	return 0;

	/* error exit paths follow */

err_request_firmware:

	vtq_destroy_context(out->vtq);
err_vtq_create_context:

	kfree(out);
err_kmalloc_vtq_imageconv_state:

	return -ENOENT;
}

void vtq_imageconv_term(struct vtq_imageconv_state **vtq_imageconv_ptr)
{
	if (atomic_read(&(*vtq_imageconv_ptr)->task_get_count) != 0) {
		err_print("Still got references on tasks when asked to quit\n");
		return;
	}
	vtq_destroy_context((*vtq_imageconv_ptr)->vtq);
	kfree(*vtq_imageconv_ptr);
	*vtq_imageconv_ptr = NULL;
}

int vtq_imageconv_ready(struct vtq_imageconv_state *c)
{
	return c->image != NULL;
}

/* This API is for when we already have the bus address.  There should
 * be an API that takes a pmem handle, and offset, or some such, but
 * no user for such an API exists for us at the moment.
 */
int vtq_imageconv_enqueue_direct(struct vtq_imageconv_state *c,
		uint32_t type,
		uint32_t tformat_baseaddr,
		uint32_t raster_baseaddr,
		int32_t signedrasterstride,
		uint32_t numtiles_wide,
		uint32_t numtiles_high,
		uint32_t dmacfg,
		vtq_job_id_t *job_id)
{
	int s;

	if (c->image == NULL)
		return -1;

	s = vtq_queue_job(c->vtq,
		type ? c->task_bgr24 : c->task_direct,
		tformat_baseaddr,
		raster_baseaddr,
		signedrasterstride,
		numtiles_wide,
		numtiles_high,
		dmacfg,
		0,
		job_id);
	return s;
}

int vtq_imageconv_await(struct vtq_imageconv_state *c,
			vtq_job_id_t job_id)
{
	int s;

	if (c->image == NULL)
		return -1;

	s = vtq_await_job(c->vtq, job_id);
	return s;
}

/* ******************** */
