/* Copyright (c) 2016-2018, The Linux Foundation. All rights reserved.
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

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/usb/audio.h>
#include <linux/usb/audio-v2.h>
#include <linux/uaccess.h>
#include <sound/pcm.h>
#include <sound/core.h>
#include <sound/asound.h>
#include <linux/usb.h>
#include <linux/qmi_encdec.h>
#include <soc/qcom/msm_qmi_interface.h>
#include <linux/iommu.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/usb/audio-v3.h>

#include "usbaudio.h"
#include "card.h"
#include "helper.h"
#include "pcm.h"
#include "usb_audio_qmi_v01.h"

#define SND_PCM_CARD_NUM_MASK 0xffff0000
#define SND_PCM_DEV_NUM_MASK 0xff00
#define SND_PCM_STREAM_DIRECTION 0xff

#define PREPEND_SID_TO_IOVA(iova, sid) (u64)(((u64)(iova)) | \
					(((u64)sid) << 32))

/*  event ring iova base address */
#define IOVA_BASE 0x1000

#define IOVA_XFER_RING_BASE (IOVA_BASE + PAGE_SIZE * (SNDRV_CARDS + 1))
#define IOVA_XFER_BUF_BASE (IOVA_XFER_RING_BASE + PAGE_SIZE * SNDRV_CARDS * 32)
#define IOVA_XFER_RING_MAX (IOVA_XFER_BUF_BASE - PAGE_SIZE)
#define IOVA_XFER_BUF_MAX (0xfffff000 - PAGE_SIZE)

#define MAX_XFER_BUFF_LEN (24 * PAGE_SIZE)

struct iova_info {
	struct list_head list;
	unsigned long start_iova;
	size_t size;
	bool in_use;
};

struct intf_info {
	unsigned long data_xfer_ring_va;
	size_t data_xfer_ring_size;
	unsigned long sync_xfer_ring_va;
	size_t sync_xfer_ring_size;
	unsigned long xfer_buf_va;
	size_t xfer_buf_size;
	phys_addr_t xfer_buf_pa;
	unsigned int data_ep_pipe;
	unsigned int sync_ep_pipe;
	u8 *xfer_buf;
	u8 intf_num;
	u8 pcm_card_num;
	u8 pcm_dev_num;
	u8 direction;
	bool in_use;
};

struct uaudio_dev {
	struct usb_device *udev;
	/* audio control interface */
	struct usb_host_interface *ctrl_intf;
	unsigned int card_num;
	unsigned int usb_core_id;
	atomic_t in_use;
	struct kref kref;
	wait_queue_head_t disconnect_wq;

	/* interface specific */
	int num_intf;
	struct intf_info *info;
};

static struct uaudio_dev uadev[SNDRV_CARDS];

struct uaudio_qmi_dev {
	struct device *dev;
	u32 sid;
	u32 intr_num;
	struct iommu_domain *domain;

	/* list to keep track of available iova */
	struct list_head xfer_ring_list;
	size_t xfer_ring_iova_size;
	unsigned long curr_xfer_ring_iova;
	struct list_head xfer_buf_list;
	size_t xfer_buf_iova_size;
	unsigned long curr_xfer_buf_iova;
	/* bit fields representing pcm card enabled */
	unsigned long card_slot;
	/* indicate event ring mapped or not */
	bool er_mapped;
};

static struct uaudio_qmi_dev *uaudio_qdev;

struct uaudio_qmi_svc {
	struct qmi_handle *uaudio_svc_hdl;
	void *curr_conn;
	struct work_struct recv_msg_work;
	struct work_struct qmi_disconnect_work;
	struct workqueue_struct *uaudio_wq;
	ktime_t t_request_recvd;
	ktime_t t_resp_sent;
};

static struct uaudio_qmi_svc *uaudio_svc;

static struct msg_desc uaudio_stream_req_desc = {
	.max_msg_len = QMI_UAUDIO_STREAM_REQ_MSG_V01_MAX_MSG_LEN,
	.msg_id = QMI_UAUDIO_STREAM_REQ_V01,
	.ei_array = qmi_uaudio_stream_req_msg_v01_ei,
};

static struct msg_desc uaudio_stream_resp_desc = {
	.max_msg_len = QMI_UAUDIO_STREAM_RESP_MSG_V01_MAX_MSG_LEN,
	.msg_id = QMI_UAUDIO_STREAM_RESP_V01,
	.ei_array = qmi_uaudio_stream_resp_msg_v01_ei,
};

static struct msg_desc uaudio_stream_ind_desc = {
	.max_msg_len = QMI_UAUDIO_STREAM_IND_MSG_V01_MAX_MSG_LEN,
	.msg_id = QMI_UADUIO_STREAM_IND_V01,
	.ei_array = qmi_uaudio_stream_ind_msg_v01_ei,
};

enum mem_type {
	MEM_EVENT_RING,
	MEM_XFER_RING,
	MEM_XFER_BUF,
};

enum usb_qmi_audio_format {
	USB_QMI_PCM_FORMAT_S8 = 0,
	USB_QMI_PCM_FORMAT_U8,
	USB_QMI_PCM_FORMAT_S16_LE,
	USB_QMI_PCM_FORMAT_S16_BE,
	USB_QMI_PCM_FORMAT_U16_LE,
	USB_QMI_PCM_FORMAT_U16_BE,
	USB_QMI_PCM_FORMAT_S24_LE,
	USB_QMI_PCM_FORMAT_S24_BE,
	USB_QMI_PCM_FORMAT_U24_LE,
	USB_QMI_PCM_FORMAT_U24_BE,
	USB_QMI_PCM_FORMAT_S24_3LE,
	USB_QMI_PCM_FORMAT_S24_3BE,
	USB_QMI_PCM_FORMAT_U24_3LE,
	USB_QMI_PCM_FORMAT_U24_3BE,
	USB_QMI_PCM_FORMAT_S32_LE,
	USB_QMI_PCM_FORMAT_S32_BE,
	USB_QMI_PCM_FORMAT_U32_LE,
	USB_QMI_PCM_FORMAT_U32_BE,
};

static void uaudio_iommu_unmap(enum mem_type mtype, unsigned long va,
	size_t iova_size, size_t mapped_iova_size);

static enum usb_audio_device_speed_enum_v01
get_speed_info(enum usb_device_speed udev_speed)
{
	switch (udev_speed) {
	case USB_SPEED_LOW:
		return USB_AUDIO_DEVICE_SPEED_LOW_V01;
	case USB_SPEED_FULL:
		return USB_AUDIO_DEVICE_SPEED_FULL_V01;
	case USB_SPEED_HIGH:
		return USB_AUDIO_DEVICE_SPEED_HIGH_V01;
	case USB_SPEED_SUPER:
		return USB_AUDIO_DEVICE_SPEED_SUPER_V01;
	case USB_SPEED_SUPER_PLUS:
		return USB_AUDIO_DEVICE_SPEED_SUPER_PLUS_V01;
	default:
		pr_err("%s: udev speed %d\n", __func__, udev_speed);
		return USB_AUDIO_DEVICE_SPEED_INVALID_V01;
	}
}

static unsigned long uaudio_get_iova(unsigned long *curr_iova,
	size_t *curr_iova_size, struct list_head *head, size_t size)
{
	struct iova_info *info, *new_info = NULL;
	struct list_head *curr_head;
	unsigned long va = 0;
	size_t tmp_size = size;
	bool found = false;

	if (size % PAGE_SIZE) {
		pr_err("%s: size %zu is not page size multiple\n", __func__,
			size);
		goto done;
	}

	if (size > *curr_iova_size) {
		pr_err("%s: size %zu > curr size %zu\n", __func__, size,
			*curr_iova_size);
		goto done;
	}
	if (*curr_iova_size == 0) {
		pr_err("%s: iova mapping is full\n", __func__);
		goto done;
	}

	list_for_each_entry(info, head, list) {
		/* exact size iova_info */
		if (!info->in_use && info->size == size) {
			info->in_use = true;
			va = info->start_iova;
			*curr_iova_size -= size;
			found = true;
			pr_debug("%s: exact size :%zu found\n", __func__, size);
			goto done;
		} else if (!info->in_use && tmp_size >= info->size) {
			if (!new_info)
				new_info = info;
			pr_debug("%s: partial size: %zu found\n", __func__,
				info->size);
			tmp_size -= info->size;
			if (tmp_size)
				continue;

			va = new_info->start_iova;
			for (curr_head = &new_info->list; curr_head !=
			&info->list; curr_head = curr_head->next) {
				new_info = list_entry(curr_head, struct
						iova_info, list);
				new_info->in_use = true;
			}
			info->in_use = true;
			*curr_iova_size -= size;
			found = true;
			goto done;
		} else {
			/* iova region in use */
			new_info = NULL;
			tmp_size = size;
		}
	}

	info = kzalloc(sizeof(struct iova_info), GFP_KERNEL);
	if (!info) {
		va = 0;
		goto done;
	}

	va = info->start_iova = *curr_iova;
	info->size = size;
	info->in_use = true;
	*curr_iova += size;
	*curr_iova_size -= size;
	found = true;
	list_add_tail(&info->list, head);

done:
	if (!found)
		pr_err("%s: unable to find %zu size iova\n", __func__, size);
	else
		pr_debug("%s: va:%lu curr_iova:%lu curr_iova_size:%zu\n",
		__func__, va, *curr_iova, *curr_iova_size);

	return va;
}

static unsigned long uaudio_iommu_map(enum mem_type mtype, phys_addr_t pa,
		size_t size, struct sg_table *sgt)
{
	unsigned long va_sg, va = 0;
	bool map = true;
	int i, ret;
	size_t sg_len, total_len = 0;
	struct scatterlist *sg;
	phys_addr_t pa_sg;

	switch (mtype) {
	case MEM_EVENT_RING:
		va = IOVA_BASE;
		/* er already mapped */
		if (uaudio_qdev->er_mapped)
			map = false;
		break;
	case MEM_XFER_RING:
		va = uaudio_get_iova(&uaudio_qdev->curr_xfer_ring_iova,
		&uaudio_qdev->xfer_ring_iova_size, &uaudio_qdev->xfer_ring_list,
		size);
		break;
	case MEM_XFER_BUF:
		va = uaudio_get_iova(&uaudio_qdev->curr_xfer_buf_iova,
		&uaudio_qdev->xfer_buf_iova_size, &uaudio_qdev->xfer_buf_list,
		size);
		break;
	default:
		pr_err("%s: unknown mem type %d\n", __func__, mtype);
	}

	if (!va || !map)
		goto done;

	if (!sgt)
		goto skip_sgt_map;

	va_sg = va;
	for_each_sg(sgt->sgl, sg, sgt->nents, i) {
		sg_len = PAGE_ALIGN(sg->offset + sg->length);
		pa_sg = page_to_phys(sg_page(sg));
		ret = iommu_map(uaudio_qdev->domain, va_sg, pa_sg, sg_len,
			IOMMU_READ | IOMMU_WRITE | IOMMU_MMIO);
		if (ret) {
			pr_err("%s:mapping failed ret%d\n", __func__, ret);
			pr_err("memtype:%d, pa:%pK iova:%lu sg_len:%zu\n",
				mtype, &pa_sg, va_sg, sg_len);
			uaudio_iommu_unmap(MEM_XFER_BUF, va, size, total_len);
			va = 0;
			goto done;
		}
		pr_debug("%s:memtype %d:map pa:%pK to iova:%lu len:%zu\n",
			__func__, mtype, &pa_sg, va_sg, sg_len);
		va_sg += sg_len;
		total_len += sg_len;
	}

	if (size != total_len) {
		pr_err("%s: iova size %zu != mapped iova size %zu\n", __func__,
			size, total_len);
		uaudio_iommu_unmap(MEM_XFER_BUF, va, size, total_len);
		va = 0;
	}
	return va;

skip_sgt_map:
	pr_debug("%s:memtype:%d map pa:%pK to iova %lu size:%zu\n", __func__,
		mtype, &pa, va, size);

	ret = iommu_map(uaudio_qdev->domain, va, pa, size,
		IOMMU_READ | IOMMU_WRITE | IOMMU_MMIO);
	if (ret)
		pr_err("%s:failed to map pa:%pK iova:%lu memtype:%d ret:%d\n",
			__func__, &pa, va, mtype, ret);
done:
	return va;
}

static void uaudio_put_iova(unsigned long va, size_t size, struct list_head
	*head, size_t *curr_iova_size)
{
	struct iova_info *info;
	size_t tmp_size = size;
	bool found = false;

	list_for_each_entry(info, head, list) {
		if (info->start_iova == va) {
			if (!info->in_use) {
				pr_err("%s: va %lu is not in use\n", __func__,
					va);
				return;
			}
			found = true;
			info->in_use = false;
			if (info->size == size)
				goto done;
		}

		if (found && tmp_size >= info->size) {
			info->in_use = false;
			tmp_size -= info->size;
			if (!tmp_size)
				goto done;
		}
	}

	if (!found) {
		pr_err("%s: unable to find the va %lu\n", __func__, va);
		return;
	}
done:
	*curr_iova_size += size;
	pr_debug("%s: curr_iova_size %zu\n", __func__, *curr_iova_size);
}

static void uaudio_iommu_unmap(enum mem_type mtype, unsigned long va,
	size_t iova_size, size_t mapped_iova_size)
{
	size_t umap_size;
	bool unmap = true;

	if (!va || !iova_size)
		return;

	switch (mtype) {
	case MEM_EVENT_RING:
		if (uaudio_qdev->er_mapped)
			uaudio_qdev->er_mapped = false;
		else
			unmap = false;
		break;

	case MEM_XFER_RING:
		uaudio_put_iova(va, iova_size, &uaudio_qdev->xfer_ring_list,
		&uaudio_qdev->xfer_ring_iova_size);
		break;
	case MEM_XFER_BUF:
		uaudio_put_iova(va, iova_size, &uaudio_qdev->xfer_buf_list,
		&uaudio_qdev->xfer_buf_iova_size);
		break;
	default:
		pr_err("%s: unknown mem type %d\n", __func__, mtype);
		unmap = false;
	}

	if (!unmap || !mapped_iova_size)
		return;

	pr_debug("%s:memtype %d: unmap iova %lu size %zu\n", __func__, mtype,
		va, mapped_iova_size);

	umap_size = iommu_unmap(uaudio_qdev->domain, va, mapped_iova_size);
	if (umap_size != mapped_iova_size)
		pr_err("%s:unmapped size %zu for iova %lu of mapped size %zu\n",
		__func__, umap_size, va, mapped_iova_size);
}

static int prepare_qmi_response(struct snd_usb_substream *subs,
		struct qmi_uaudio_stream_req_msg_v01 *req_msg,
		struct qmi_uaudio_stream_resp_msg_v01 *resp, int info_idx)
{
	struct usb_interface *iface;
	struct usb_host_interface *alts;
	struct usb_interface_descriptor *altsd;
	struct usb_host_endpoint *ep;
	struct uac_format_type_i_continuous_descriptor *fmt;
	struct uac_format_type_i_discrete_descriptor *fmt_v1;
	struct uac_format_type_i_ext_descriptor *fmt_v2;
	struct uac1_as_header_descriptor *as;
	int ret = -ENODEV;
	int protocol, card_num, pcm_dev_num;
	void *hdr_ptr;
	u8 *xfer_buf;
	unsigned int data_ep_pipe = 0, sync_ep_pipe = 0;
	u32 len, mult, remainder, xfer_buf_len;
	unsigned long va, tr_data_va = 0, tr_sync_va = 0;
	phys_addr_t xhci_pa, xfer_buf_pa, tr_data_pa = 0, tr_sync_pa = 0;
	dma_addr_t dma;
	struct sg_table sgt;

	iface = usb_ifnum_to_if(subs->dev, subs->interface);
	if (!iface) {
		pr_err("%s: interface # %d does not exist\n", __func__,
			subs->interface);
		goto err;
	}

	pcm_dev_num = (req_msg->usb_token & SND_PCM_DEV_NUM_MASK) >> 8;
	card_num = (req_msg->usb_token & SND_PCM_CARD_NUM_MASK) >> 16;
	xfer_buf_len = req_msg->xfer_buff_size;

	alts = &iface->altsetting[subs->altset_idx];
	altsd = get_iface_desc(alts);
	protocol = altsd->bInterfaceProtocol;

	/* get format type */
	if (protocol != UAC_VERSION_3) {
		fmt = snd_usb_find_csint_desc(alts->extra, alts->extralen, NULL,
				UAC_FORMAT_TYPE);
		if (!fmt) {
			pr_err("%s: %u:%d : no UAC_FORMAT_TYPE desc\n",
				__func__, subs->interface, subs->altset_idx);
			goto err;
		}
	}

	if (!uadev[card_num].ctrl_intf) {
		pr_err("%s: audio ctrl intf info not cached\n", __func__);
		goto err;
	}

	if (protocol != UAC_VERSION_3) {
		hdr_ptr = snd_usb_find_csint_desc(
				uadev[card_num].ctrl_intf->extra,
				uadev[card_num].ctrl_intf->extralen,
				NULL, UAC_HEADER);
		if (!hdr_ptr) {
			pr_err("%s: no UAC_HEADER desc\n", __func__);
			goto err;
		}
	}

	if (protocol == UAC_VERSION_1) {
		as = snd_usb_find_csint_desc(alts->extra, alts->extralen, NULL,
			UAC_AS_GENERAL);
		if (!as) {
			pr_err("%s: %u:%d : no UAC_AS_GENERAL desc\n", __func__,
				subs->interface, subs->altset_idx);
			goto err;
		}
		resp->data_path_delay = as->bDelay;
		resp->data_path_delay_valid = 1;
		fmt_v1 = (struct uac_format_type_i_discrete_descriptor *)fmt;
		resp->usb_audio_subslot_size = fmt_v1->bSubframeSize;
		resp->usb_audio_subslot_size_valid = 1;

		resp->usb_audio_spec_revision =
			((struct uac1_ac_header_descriptor *)hdr_ptr)->bcdADC;
		resp->usb_audio_spec_revision_valid = 1;
	} else if (protocol == UAC_VERSION_2) {
		fmt_v2 = (struct uac_format_type_i_ext_descriptor *)fmt;
		resp->usb_audio_subslot_size = fmt_v2->bSubslotSize;
		resp->usb_audio_subslot_size_valid = 1;

		resp->usb_audio_spec_revision =
			((struct uac2_ac_header_descriptor *)hdr_ptr)->bcdADC;
		resp->usb_audio_spec_revision_valid = 1;
	} else if (protocol == UAC_VERSION_3) {
		switch (le16_to_cpu(get_endpoint(alts, 0)->wMaxPacketSize)) {
		case BADD_MAXPSIZE_SYNC_MONO_16:
		case BADD_MAXPSIZE_SYNC_STEREO_16:
		case BADD_MAXPSIZE_ASYNC_MONO_16:
		case BADD_MAXPSIZE_ASYNC_STEREO_16: {
			resp->usb_audio_subslot_size = SUBSLOTSIZE_16_BIT;
			break;
		}

		case BADD_MAXPSIZE_SYNC_MONO_24:
		case BADD_MAXPSIZE_SYNC_STEREO_24:
		case BADD_MAXPSIZE_ASYNC_MONO_24:
		case BADD_MAXPSIZE_ASYNC_STEREO_24: {
			resp->usb_audio_subslot_size = SUBSLOTSIZE_24_BIT;
			break;
		}

		default:
			pr_err("%d: %u: Invalid wMaxPacketSize\n",
				subs->interface, subs->altset_idx);
			ret = -EINVAL;
			goto err;
		}
		resp->usb_audio_subslot_size_valid = 1;
	} else {
		pr_err("%s: unknown protocol version %x\n", __func__, protocol);
		goto err;
	}

	resp->slot_id = subs->dev->slot_id;
	resp->slot_id_valid = 1;

	memcpy(&resp->std_as_opr_intf_desc, &alts->desc, sizeof(alts->desc));
	resp->std_as_opr_intf_desc_valid = 1;

	ep = usb_pipe_endpoint(subs->dev, subs->data_endpoint->pipe);
	if (!ep) {
		pr_err("%s: data ep # %d context is null\n", __func__,
			subs->data_endpoint->ep_num);
		goto err;
	}
	data_ep_pipe = subs->data_endpoint->pipe;
	memcpy(&resp->std_as_data_ep_desc, &ep->desc, sizeof(ep->desc));
	resp->std_as_data_ep_desc_valid = 1;

	tr_data_pa = usb_get_xfer_ring_phys_addr(subs->dev, ep, &dma);
	if (!tr_data_pa) {
		pr_err("%s:failed to get data ep ring dma address\n", __func__);
		goto err;
	}

	resp->xhci_mem_info.tr_data.pa = dma;

	if (subs->sync_endpoint) {
		ep = usb_pipe_endpoint(subs->dev, subs->sync_endpoint->pipe);
		if (!ep) {
			pr_debug("%s: implicit fb on data ep\n", __func__);
			goto skip_sync_ep;
		}
		sync_ep_pipe = subs->sync_endpoint->pipe;
		memcpy(&resp->std_as_sync_ep_desc, &ep->desc, sizeof(ep->desc));
		resp->std_as_sync_ep_desc_valid = 1;

		tr_sync_pa = usb_get_xfer_ring_phys_addr(subs->dev, ep, &dma);
		if (!tr_sync_pa) {
			pr_err("%s:failed to get sync ep ring dma address\n",
				__func__);
			goto err;
		}
		resp->xhci_mem_info.tr_sync.pa = dma;
	}

skip_sync_ep:
	resp->interrupter_num = uaudio_qdev->intr_num;
	resp->interrupter_num_valid = 1;

	ret = usb_get_controller_id(subs->dev);
	if (ret < 0)
		goto err;

	resp->controller_num =  ret;
	resp->controller_num_valid = 1;

	/*  map xhci data structures PA memory to iova */

	/* event ring */
	ret = usb_sec_event_ring_setup(subs->dev, resp->interrupter_num);
	if (ret) {
		pr_err("%s: failed to setup sec event ring ret %d\n", __func__,
			ret);
		goto err;
	}
	xhci_pa = usb_get_sec_event_ring_phys_addr(subs->dev,
			resp->interrupter_num, &dma);
	if (!xhci_pa) {
		pr_err("%s: failed to get sec event ring dma address\n",
		__func__);
		goto err;
	}

	va = uaudio_iommu_map(MEM_EVENT_RING, xhci_pa, PAGE_SIZE, NULL);
	if (!va)
		goto err;

	resp->xhci_mem_info.evt_ring.va = PREPEND_SID_TO_IOVA(va,
						uaudio_qdev->sid);
	resp->xhci_mem_info.evt_ring.pa = dma;
	resp->xhci_mem_info.evt_ring.size = PAGE_SIZE;
	uaudio_qdev->er_mapped = true;

	resp->speed_info = get_speed_info(subs->dev->speed);
	if (resp->speed_info == USB_AUDIO_DEVICE_SPEED_INVALID_V01)
		goto unmap_er;

	resp->speed_info_valid = 1;

	/* data transfer ring */
	va = uaudio_iommu_map(MEM_XFER_RING, tr_data_pa, PAGE_SIZE, NULL);
	if (!va)
		goto unmap_er;

	tr_data_va = va;
	resp->xhci_mem_info.tr_data.va = PREPEND_SID_TO_IOVA(va,
						uaudio_qdev->sid);
	resp->xhci_mem_info.tr_data.size = PAGE_SIZE;

	/* sync transfer ring */
	if (!resp->xhci_mem_info.tr_sync.pa)
		goto skip_sync;

	xhci_pa = resp->xhci_mem_info.tr_sync.pa;
	va = uaudio_iommu_map(MEM_XFER_RING, tr_sync_pa, PAGE_SIZE, NULL);
	if (!va)
		goto unmap_data;

	tr_sync_va = va;
	resp->xhci_mem_info.tr_sync.va = PREPEND_SID_TO_IOVA(va,
						uaudio_qdev->sid);
	resp->xhci_mem_info.tr_sync.size = PAGE_SIZE;

skip_sync:
	/* xfer buffer, multiple of 4K only */
	if (!xfer_buf_len)
		xfer_buf_len = PAGE_SIZE;

	mult = xfer_buf_len / PAGE_SIZE;
	remainder = xfer_buf_len % PAGE_SIZE;
	len = mult * PAGE_SIZE;
	len += remainder ? PAGE_SIZE : 0;

	if (len > MAX_XFER_BUFF_LEN) {
		pr_err("%s: req buf len %d > max buf len %lu, setting %lu\n",
		__func__, len, MAX_XFER_BUFF_LEN, MAX_XFER_BUFF_LEN);
		len = MAX_XFER_BUFF_LEN;
	}

	xfer_buf = usb_alloc_coherent(subs->dev, len, GFP_KERNEL, &xfer_buf_pa);
	if (!xfer_buf)
		goto unmap_sync;

	dma_get_sgtable(subs->dev->bus->sysdev, &sgt, xfer_buf, xfer_buf_pa,
			len);
	va = uaudio_iommu_map(MEM_XFER_BUF, xfer_buf_pa, len, &sgt);
	if (!va)
		goto unmap_sync;

	resp->xhci_mem_info.xfer_buff.pa = xfer_buf_pa;
	resp->xhci_mem_info.xfer_buff.size = len;

	resp->xhci_mem_info.xfer_buff.va = PREPEND_SID_TO_IOVA(va,
						uaudio_qdev->sid);

	resp->xhci_mem_info_valid = 1;

	sg_free_table(&sgt);

	if (!atomic_read(&uadev[card_num].in_use)) {
		kref_init(&uadev[card_num].kref);
		init_waitqueue_head(&uadev[card_num].disconnect_wq);
		uadev[card_num].num_intf =
			subs->dev->config->desc.bNumInterfaces;
		uadev[card_num].info =
			kzalloc(sizeof(struct intf_info) *
			uadev[card_num].num_intf, GFP_KERNEL);
		if (!uadev[card_num].info) {
			ret = -ENOMEM;
			goto unmap_sync;
		}
		uadev[card_num].udev = subs->dev;
		atomic_set(&uadev[card_num].in_use, 1);
	} else {
		kref_get(&uadev[card_num].kref);
	}

	uadev[card_num].card_num = card_num;
	uadev[card_num].usb_core_id = resp->controller_num;

	/* cache intf specific info to use it for unmap and free xfer buf */
	uadev[card_num].info[info_idx].data_xfer_ring_va = tr_data_va;
	uadev[card_num].info[info_idx].data_xfer_ring_size = PAGE_SIZE;
	uadev[card_num].info[info_idx].sync_xfer_ring_va = tr_sync_va;
	uadev[card_num].info[info_idx].sync_xfer_ring_size = PAGE_SIZE;
	uadev[card_num].info[info_idx].xfer_buf_va = va;
	uadev[card_num].info[info_idx].xfer_buf_pa = xfer_buf_pa;
	uadev[card_num].info[info_idx].xfer_buf_size = len;
	uadev[card_num].info[info_idx].data_ep_pipe = data_ep_pipe;
	uadev[card_num].info[info_idx].sync_ep_pipe = sync_ep_pipe;
	uadev[card_num].info[info_idx].xfer_buf = xfer_buf;
	uadev[card_num].info[info_idx].pcm_card_num = card_num;
	uadev[card_num].info[info_idx].pcm_dev_num = pcm_dev_num;
	uadev[card_num].info[info_idx].direction = subs->direction;
	uadev[card_num].info[info_idx].intf_num = subs->interface;
	uadev[card_num].info[info_idx].in_use = true;

	set_bit(card_num, &uaudio_qdev->card_slot);

	return 0;

unmap_sync:
	usb_free_coherent(subs->dev, len, xfer_buf, xfer_buf_pa);
	uaudio_iommu_unmap(MEM_XFER_RING, tr_sync_va, PAGE_SIZE, PAGE_SIZE);
unmap_data:
	uaudio_iommu_unmap(MEM_XFER_RING, tr_data_va, PAGE_SIZE, PAGE_SIZE);
unmap_er:
	uaudio_iommu_unmap(MEM_EVENT_RING, IOVA_BASE, PAGE_SIZE, PAGE_SIZE);
err:
	return ret;
}

static void uaudio_dev_intf_cleanup(struct usb_device *udev,
	struct intf_info *info)
{

	uaudio_iommu_unmap(MEM_XFER_RING, info->data_xfer_ring_va,
		info->data_xfer_ring_size, info->data_xfer_ring_size);
	info->data_xfer_ring_va = 0;
	info->data_xfer_ring_size = 0;

	uaudio_iommu_unmap(MEM_XFER_RING, info->sync_xfer_ring_va,
		info->sync_xfer_ring_size, info->sync_xfer_ring_size);
	info->sync_xfer_ring_va = 0;
	info->sync_xfer_ring_size = 0;

	uaudio_iommu_unmap(MEM_XFER_BUF, info->xfer_buf_va,
		info->xfer_buf_size, info->xfer_buf_size);
	info->xfer_buf_va = 0;

	usb_free_coherent(udev, info->xfer_buf_size,
		info->xfer_buf, info->xfer_buf_pa);
	info->xfer_buf_size = 0;
	info->xfer_buf = NULL;
	info->xfer_buf_pa = 0;

	info->in_use = false;
}

static void uaudio_dev_cleanup(struct uaudio_dev *dev)
{
	int if_idx;

	/* free xfer buffer and unmap xfer ring and buf per interface */
	for (if_idx = 0; if_idx < dev->num_intf; if_idx++) {
		if (!dev->info[if_idx].in_use)
			continue;
		uaudio_dev_intf_cleanup(dev->udev, &dev->info[if_idx]);
		pr_debug("%s: release resources: intf# %d card# %d\n", __func__,
			dev->info[if_idx].intf_num, dev->card_num);
	}

	dev->num_intf = 0;

	/* free interface info */
	kfree(dev->info);
	dev->info = NULL;

	clear_bit(dev->card_num, &uaudio_qdev->card_slot);

	/* all audio devices are disconnected */
	if (!uaudio_qdev->card_slot) {
		uaudio_iommu_unmap(MEM_EVENT_RING, IOVA_BASE, PAGE_SIZE,
			PAGE_SIZE);
		usb_sec_event_ring_cleanup(dev->udev, uaudio_qdev->intr_num);
		pr_debug("%s: all audio devices disconnected\n", __func__);
	}

	dev->udev = NULL;
}

static void uaudio_disconnect_cb(struct snd_usb_audio *chip)
{
	int ret;
	struct uaudio_dev *dev;
	int card_num = chip->card_num;
	struct uaudio_qmi_svc *svc = uaudio_svc;
	struct qmi_uaudio_stream_ind_msg_v01 disconnect_ind = {0};

	pr_debug("%s: for card# %d\n", __func__, card_num);

	if (card_num >=  SNDRV_CARDS) {
		pr_err("%s: invalid card number\n", __func__);
		return;
	}

	mutex_lock(&chip->dev_lock);
	dev = &uadev[card_num];

	/* clean up */
	if (!dev->udev) {
		pr_debug("%s: no clean up required\n", __func__);
		goto done;
	}

	if (atomic_read(&dev->in_use)) {
		mutex_unlock(&chip->dev_lock);

		pr_debug("%s: sending qmi indication disconnect\n", __func__);
		disconnect_ind.dev_event = USB_AUDIO_DEV_DISCONNECT_V01;
		disconnect_ind.slot_id = dev->udev->slot_id;
		disconnect_ind.controller_num = dev->usb_core_id;
		disconnect_ind.controller_num_valid = 1;
		ret = qmi_send_ind(svc->uaudio_svc_hdl, svc->curr_conn,
				&uaudio_stream_ind_desc, &disconnect_ind,
				sizeof(disconnect_ind));
		if (ret < 0) {
			pr_err("%s: qmi send failed wiht err: %d\n",
					__func__, ret);
			return;
		}

		ret = wait_event_interruptible(dev->disconnect_wq,
				!atomic_read(&dev->in_use));
		if (ret < 0) {
			pr_debug("%s: failed with ret %d\n", __func__, ret);
			return;
		}
		mutex_lock(&chip->dev_lock);
	}

	uaudio_dev_cleanup(dev);
done:
	mutex_unlock(&chip->dev_lock);
}

static void uaudio_dev_release(struct kref *kref)
{
	struct uaudio_dev *dev = container_of(kref, struct uaudio_dev, kref);

	pr_debug("%s for dev %pK\n", __func__, dev);

	atomic_set(&dev->in_use, 0);

	clear_bit(dev->card_num, &uaudio_qdev->card_slot);

	/* all audio devices are disconnected */
	if (!uaudio_qdev->card_slot) {
		usb_sec_event_ring_cleanup(dev->udev, uaudio_qdev->intr_num);
		uaudio_iommu_unmap(MEM_EVENT_RING, IOVA_BASE, PAGE_SIZE,
			PAGE_SIZE);
		pr_debug("%s: all audio devices disconnected\n", __func__);
	}

	wake_up(&dev->disconnect_wq);
}

/* maps audio format received over QMI to asound.h based pcm format */
static int map_pcm_format(unsigned int fmt_received)
{
	switch (fmt_received) {
	case USB_QMI_PCM_FORMAT_S8:
		return SNDRV_PCM_FORMAT_S8;
	case USB_QMI_PCM_FORMAT_U8:
		return SNDRV_PCM_FORMAT_U8;
	case USB_QMI_PCM_FORMAT_S16_LE:
		return SNDRV_PCM_FORMAT_S16_LE;
	case USB_QMI_PCM_FORMAT_S16_BE:
		return SNDRV_PCM_FORMAT_S16_BE;
	case USB_QMI_PCM_FORMAT_U16_LE:
		return SNDRV_PCM_FORMAT_U16_LE;
	case USB_QMI_PCM_FORMAT_U16_BE:
		return SNDRV_PCM_FORMAT_U16_BE;
	case USB_QMI_PCM_FORMAT_S24_LE:
		return SNDRV_PCM_FORMAT_S24_LE;
	case USB_QMI_PCM_FORMAT_S24_BE:
		return SNDRV_PCM_FORMAT_S24_BE;
	case USB_QMI_PCM_FORMAT_U24_LE:
		return SNDRV_PCM_FORMAT_U24_LE;
	case USB_QMI_PCM_FORMAT_U24_BE:
		return SNDRV_PCM_FORMAT_U24_BE;
	case USB_QMI_PCM_FORMAT_S24_3LE:
		return SNDRV_PCM_FORMAT_S24_3LE;
	case USB_QMI_PCM_FORMAT_S24_3BE:
		return SNDRV_PCM_FORMAT_S24_3BE;
	case USB_QMI_PCM_FORMAT_U24_3LE:
		return SNDRV_PCM_FORMAT_U24_3LE;
	case USB_QMI_PCM_FORMAT_U24_3BE:
		return SNDRV_PCM_FORMAT_U24_3BE;
	case USB_QMI_PCM_FORMAT_S32_LE:
		return SNDRV_PCM_FORMAT_S32_LE;
	case USB_QMI_PCM_FORMAT_S32_BE:
		return SNDRV_PCM_FORMAT_S32_BE;
	case USB_QMI_PCM_FORMAT_U32_LE:
		return SNDRV_PCM_FORMAT_U32_LE;
	case USB_QMI_PCM_FORMAT_U32_BE:
		return SNDRV_PCM_FORMAT_U32_BE;
	default:
		return -EINVAL;
	}
}

static int info_idx_from_ifnum(int card_num, int intf_num, bool enable)
{
	int i;

	/*
	 * default index 0 is used when info is allocated upon
	 * first enable audio stream req for a pcm device
	 */
	if (enable && !uadev[card_num].info)
		return 0;

	for (i = 0; i < uadev[card_num].num_intf; i++) {
		if (enable && !uadev[card_num].info[i].in_use)
			return i;
		else if (!enable &&
				uadev[card_num].info[i].intf_num == intf_num)
			return i;
	}

	return -EINVAL;
}

static int handle_uaudio_stream_req(void *req_h, void *req)
{
	struct qmi_uaudio_stream_req_msg_v01 *req_msg;
	struct qmi_uaudio_stream_resp_msg_v01 resp = {{0}, 0};
	struct snd_usb_substream *subs;
	struct snd_usb_audio *chip = NULL;
	struct uaudio_qmi_svc *svc = uaudio_svc;
	struct intf_info *info;
	struct usb_host_endpoint *ep;
	int pcm_format;
	u8 pcm_card_num, pcm_dev_num, direction;
	int info_idx = -EINVAL, ret = 0;

	req_msg = (struct qmi_uaudio_stream_req_msg_v01 *)req;

	if (!req_msg->audio_format_valid || !req_msg->bit_rate_valid ||
	!req_msg->number_of_ch_valid || !req_msg->xfer_buff_size_valid) {
		pr_err("%s: invalid request msg\n", __func__);
		ret = -EINVAL;
		goto response;
	}

	direction = req_msg->usb_token & SND_PCM_STREAM_DIRECTION;
	pcm_dev_num = (req_msg->usb_token & SND_PCM_DEV_NUM_MASK) >> 8;
	pcm_card_num = (req_msg->usb_token & SND_PCM_CARD_NUM_MASK) >> 16;

	pr_debug("%s:card#:%d dev#:%d dir:%d en:%d fmt:%d rate:%d #ch:%d\n",
		__func__, pcm_card_num, pcm_dev_num, direction, req_msg->enable,
		req_msg->audio_format, req_msg->bit_rate,
		req_msg->number_of_ch);

	if (pcm_card_num >= SNDRV_CARDS) {
		pr_err("%s: invalid card # %u", __func__, pcm_card_num);
		ret = -EINVAL;
		goto response;
	}

	pcm_format = map_pcm_format(req_msg->audio_format);
	if (pcm_format == -EINVAL) {
		pr_err("%s: unsupported pcm format received %d\n",
		__func__, req_msg->audio_format);
		ret = -EINVAL;
		goto response;
	}

	subs = find_snd_usb_substream(pcm_card_num, pcm_dev_num, direction,
					&chip, uaudio_disconnect_cb);
	if (!subs || !chip || atomic_read(&chip->shutdown)) {
		pr_err("%s: can't find substream for card# %u, dev# %u dir%u\n",
			__func__, pcm_card_num, pcm_dev_num, direction);
		ret = -ENODEV;
		goto response;
	}

	mutex_lock(&chip->dev_lock);
	info_idx = info_idx_from_ifnum(pcm_card_num, subs->interface,
		req_msg->enable);
	if (atomic_read(&chip->shutdown) || !subs->stream || !subs->stream->pcm
			|| !subs->stream->chip) {
		ret = -ENODEV;
		mutex_unlock(&chip->dev_lock);
		goto response;
	}

	if (req_msg->enable) {
		if (info_idx < 0) {
			pr_err("%s interface# %d already in use card# %d\n",
				__func__, subs->interface, pcm_card_num);
			ret = -EBUSY;
			mutex_unlock(&chip->dev_lock);
			goto response;
		}
	}

	subs->pcm_format = pcm_format;
	subs->channels = req_msg->number_of_ch;
	subs->cur_rate = req_msg->bit_rate;
	uadev[pcm_card_num].ctrl_intf = chip->ctrl_intf;

	if (!req_msg->enable) {
		info = &uadev[pcm_card_num].info[info_idx];
		if (info->data_ep_pipe) {
			ep = usb_pipe_endpoint(uadev[pcm_card_num].udev,
						info->data_ep_pipe);
			if (!ep)
				pr_debug("%s: no data ep\n", __func__);
			else
				usb_stop_endpoint(uadev[pcm_card_num].udev, ep);
			info->data_ep_pipe = 0;
		}

		if (info->sync_ep_pipe) {
			ep = usb_pipe_endpoint(uadev[pcm_card_num].udev,
						info->sync_ep_pipe);
			if (!ep)
				pr_debug("%s: no sync ep\n", __func__);
			else
				usb_stop_endpoint(uadev[pcm_card_num].udev, ep);
			info->sync_ep_pipe = 0;
		}
	}

	ret = snd_usb_enable_audio_stream(subs, req_msg->enable);

	if (!ret && req_msg->enable)
		ret = prepare_qmi_response(subs, req_msg, &resp, info_idx);

	mutex_unlock(&chip->dev_lock);

response:
	if (!req_msg->enable && ret != -EINVAL) {
		if (info_idx >= 0) {
			mutex_lock(&chip->dev_lock);
			info = &uadev[pcm_card_num].info[info_idx];
			uaudio_dev_intf_cleanup(uadev[pcm_card_num].udev, info);
			pr_debug("%s:release resources: intf# %d card# %d\n",
				__func__, subs->interface, pcm_card_num);
			mutex_unlock(&chip->dev_lock);
		}
		if (atomic_read(&uadev[pcm_card_num].in_use))
			kref_put(&uadev[pcm_card_num].kref,
					uaudio_dev_release);
	}

	resp.usb_token = req_msg->usb_token;
	resp.usb_token_valid = 1;
	resp.internal_status = ret;
	resp.internal_status_valid = 1;
	resp.status = ret ? USB_AUDIO_STREAM_REQ_FAILURE_V01 : ret;
	resp.status_valid = 1;
	ret = qmi_send_resp_from_cb(svc->uaudio_svc_hdl, svc->curr_conn, req_h,
			&uaudio_stream_resp_desc, &resp, sizeof(resp));

	svc->t_resp_sent = ktime_get();

	pr_debug("%s: t_resp sent - t_req recvd (in ms) %lld\n", __func__,
		ktime_to_ms(ktime_sub(svc->t_resp_sent, svc->t_request_recvd)));

	return ret;
}

static int uaudio_qmi_svc_connect_cb(struct qmi_handle *handle,
			       void *conn_h)
{
	struct uaudio_qmi_svc *svc = uaudio_svc;

	if (svc->uaudio_svc_hdl != handle || !conn_h) {
		pr_err("%s: handle mismatch\n", __func__);
		return -EINVAL;
	}
	if (svc->curr_conn) {
		pr_err("%s: Service is busy\n", __func__);
		return -ECONNREFUSED;
	}
	svc->curr_conn = conn_h;
	return 0;
}

static void uaudio_qmi_disconnect_work(struct work_struct *w)
{
	struct intf_info *info;
	int idx, if_idx;
	struct snd_usb_substream *subs;
	struct snd_usb_audio *chip = NULL;

	/* find all active intf for set alt 0 and cleanup usb audio dev */
	for (idx = 0; idx < SNDRV_CARDS; idx++) {
		if (!atomic_read(&uadev[idx].in_use))
			continue;

		for (if_idx = 0; if_idx < uadev[idx].num_intf; if_idx++) {
			if (!uadev[idx].info || !uadev[idx].info[if_idx].in_use)
				continue;
			info = &uadev[idx].info[if_idx];
			subs = find_snd_usb_substream(info->pcm_card_num,
							info->pcm_dev_num,
							info->direction,
							&chip,
							uaudio_disconnect_cb);
			if (!subs || !chip || atomic_read(&chip->shutdown)) {
				pr_debug("%s:no subs for c#%u, dev#%u dir%u\n",
					__func__, info->pcm_card_num,
					info->pcm_dev_num,
					info->direction);
				continue;
			}
			snd_usb_enable_audio_stream(subs, 0);
		}
		atomic_set(&uadev[idx].in_use, 0);
		mutex_lock(&chip->dev_lock);
		uaudio_dev_cleanup(&uadev[idx]);
		mutex_unlock(&chip->dev_lock);
	}
}

static int uaudio_qmi_svc_disconnect_cb(struct qmi_handle *handle,
				  void *conn_h)
{
	struct uaudio_qmi_svc *svc = uaudio_svc;

	if (svc->uaudio_svc_hdl != handle || svc->curr_conn != conn_h) {
		pr_err("%s: handle mismatch\n", __func__);
		return -EINVAL;
	}

	svc->curr_conn = NULL;
	queue_work(svc->uaudio_wq, &svc->qmi_disconnect_work);

	return 0;
}

static int uaudio_qmi_svc_req_cb(struct qmi_handle *handle, void *conn_h,
			void *req_h, unsigned int msg_id, void *req)
{
	int ret;
	struct uaudio_qmi_svc *svc = uaudio_svc;

	if (svc->uaudio_svc_hdl != handle || svc->curr_conn != conn_h) {
		pr_err("%s: handle mismatch\n", __func__);
		return -EINVAL;
	}

	switch (msg_id) {
	case QMI_UAUDIO_STREAM_REQ_V01:
		ret = handle_uaudio_stream_req(req_h, req);
		break;

	default:
		ret = -ENOTSUPP;
		break;
	}
	return ret;
}

static int uaudio_qmi_svc_req_desc_cb(unsigned int msg_id,
	struct msg_desc **req_desc)
{
	int ret;

	pr_debug("%s: msg_id %d\n", __func__, msg_id);

	switch (msg_id) {
	case QMI_UAUDIO_STREAM_REQ_V01:
		*req_desc = &uaudio_stream_req_desc;
		ret = sizeof(struct qmi_uaudio_stream_req_msg_v01);
		break;

	default:
		ret = -ENOTSUPP;
		break;
	}
	return ret;
}

static void uaudio_qmi_svc_recv_msg(struct work_struct *w)
{
	int ret;
	struct uaudio_qmi_svc *svc = container_of(w, struct uaudio_qmi_svc,
		recv_msg_work);

	do {
		pr_debug("%s: Notified about a Receive Event", __func__);
	} while ((ret = qmi_recv_msg(svc->uaudio_svc_hdl)) == 0);

	if (ret != -ENOMSG)
		pr_err("%s: Error receiving message\n", __func__);
}

static void uaudio_qmi_svc_ntfy(struct qmi_handle *handle,
		enum qmi_event_type event, void *priv)
{
	struct uaudio_qmi_svc *svc = uaudio_svc;

	pr_debug("%s: event %d", __func__, event);

	svc->t_request_recvd = ktime_get();

	switch (event) {
	case QMI_RECV_MSG:
		queue_work(svc->uaudio_wq, &svc->recv_msg_work);
		break;
	default:
		break;
	}
}

static struct qmi_svc_ops_options uaudio_svc_ops_options = {
	.version = 1,
	.service_id = UAUDIO_STREAM_SERVICE_ID_V01,
	.service_vers = UAUDIO_STREAM_SERVICE_VERS_V01,
	.connect_cb = uaudio_qmi_svc_connect_cb,
	.disconnect_cb = uaudio_qmi_svc_disconnect_cb,
	.req_desc_cb = uaudio_qmi_svc_req_desc_cb,
	.req_cb = uaudio_qmi_svc_req_cb,
};

static int uaudio_qmi_plat_probe(struct platform_device *pdev)
{
	int ret;
	struct device_node *node = pdev->dev.of_node;

	uaudio_qdev = devm_kzalloc(&pdev->dev, sizeof(struct uaudio_qmi_dev),
		GFP_KERNEL);
	if (!uaudio_qdev)
		return -ENOMEM;

	uaudio_qdev->dev = &pdev->dev;

	ret = of_property_read_u32(node, "qcom,usb-audio-stream-id",
				&uaudio_qdev->sid);
	if (ret) {
		dev_err(&pdev->dev, "failed to read sid.\n");
		return -ENODEV;
	}

	ret = of_property_read_u32(node, "qcom,usb-audio-intr-num",
				&uaudio_qdev->intr_num);
	if (ret) {
		dev_err(&pdev->dev, "failed to read intr num.\n");
		return -ENODEV;
	}

	uaudio_qdev->domain = iommu_domain_alloc(pdev->dev.bus);
	if (!uaudio_qdev->domain) {
		dev_err(&pdev->dev, "failed to allocate iommu domain\n");
		return -ENODEV;
	}

	/* attach to external processor iommu */
	ret = iommu_attach_device(uaudio_qdev->domain, &pdev->dev);
	if (ret) {
		dev_err(&pdev->dev, "failed to attach device ret = %d\n", ret);
		goto free_domain;
	}

	/* initialize xfer ring and xfer buf iova list */
	INIT_LIST_HEAD(&uaudio_qdev->xfer_ring_list);
	uaudio_qdev->curr_xfer_ring_iova = IOVA_XFER_RING_BASE;
	uaudio_qdev->xfer_ring_iova_size =
			IOVA_XFER_RING_MAX - IOVA_XFER_RING_BASE;

	INIT_LIST_HEAD(&uaudio_qdev->xfer_buf_list);
	uaudio_qdev->curr_xfer_buf_iova = IOVA_XFER_BUF_BASE;
	uaudio_qdev->xfer_buf_iova_size =
		IOVA_XFER_BUF_MAX - IOVA_XFER_BUF_BASE;

	return 0;

free_domain:
	iommu_domain_free(uaudio_qdev->domain);
	return ret;
}

static int uaudio_qmi_plat_remove(struct platform_device *pdev)
{
	iommu_detach_device(uaudio_qdev->domain, &pdev->dev);
	iommu_domain_free(uaudio_qdev->domain);
	uaudio_qdev->domain = NULL;

	return 0;
}

static const struct of_device_id of_uaudio_matach[] = {
	{
		.compatible = "qcom,usb-audio-qmi-dev",
	},
	{ },
};
MODULE_DEVICE_TABLE(of, of_uaudio_matach);

static struct platform_driver uaudio_qmi_driver = {
	.probe		= uaudio_qmi_plat_probe,
	.remove		= uaudio_qmi_plat_remove,
	.driver		= {
		.name	= "uaudio-qmi",
		.of_match_table	= of_uaudio_matach,
	},
};

static int uaudio_qmi_svc_init(void)
{
	int ret;
	struct uaudio_qmi_svc *svc;

	svc = kzalloc(sizeof(struct uaudio_qmi_svc), GFP_KERNEL);
	if (!svc)
		return -ENOMEM;

	svc->uaudio_wq = create_singlethread_workqueue("uaudio_svc");
	if (!svc->uaudio_wq) {
		ret = -ENOMEM;
		goto free_svc;
	}

	svc->uaudio_svc_hdl = qmi_handle_create(uaudio_qmi_svc_ntfy, NULL);
	if (!svc->uaudio_svc_hdl) {
		pr_err("%s: Error creating svc_hdl\n", __func__);
		ret = -EFAULT;
		goto destroy_uaudio_wq;
	}

	ret = qmi_svc_register(svc->uaudio_svc_hdl, &uaudio_svc_ops_options);
	if (ret < 0) {
		pr_err("%s:Error registering uaudio svc %d\n", __func__, ret);
		goto destroy_svc_handle;
	}

	INIT_WORK(&svc->recv_msg_work, uaudio_qmi_svc_recv_msg);
	INIT_WORK(&svc->qmi_disconnect_work, uaudio_qmi_disconnect_work);

	uaudio_svc = svc;

	return 0;

destroy_svc_handle:
	qmi_handle_destroy(svc->uaudio_svc_hdl);
destroy_uaudio_wq:
	destroy_workqueue(svc->uaudio_wq);
free_svc:
	kfree(svc);
	return ret;
}

static void uaudio_qmi_svc_exit(void)
{
	struct uaudio_qmi_svc *svc = uaudio_svc;

	qmi_svc_unregister(svc->uaudio_svc_hdl);
	flush_workqueue(svc->uaudio_wq);
	qmi_handle_destroy(svc->uaudio_svc_hdl);
	destroy_workqueue(svc->uaudio_wq);
	kfree(svc);
	uaudio_svc = NULL;
}

static int __init uaudio_qmi_plat_init(void)
{
	int ret;

	ret = platform_driver_register(&uaudio_qmi_driver);
	if (ret)
		return ret;

	return uaudio_qmi_svc_init();
}

static void __exit uaudio_qmi_plat_exit(void)
{
	uaudio_qmi_svc_exit();
	platform_driver_unregister(&uaudio_qmi_driver);
}

module_init(uaudio_qmi_plat_init);
module_exit(uaudio_qmi_plat_exit);

MODULE_DESCRIPTION("USB AUDIO QMI Service Driver");
MODULE_LICENSE("GPL v2");
