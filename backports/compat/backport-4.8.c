/*
 * Copyright(c) 2017 Intel Deutschland GmbH
 *
 * Backport functionality introduced in Linux 4.8.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/usb.h>
#include <linux/usb/cdc.h>

int cdc_parse_cdc_header(struct usb_cdc_parsed_header *hdr,
			 struct usb_interface *intf,
			 u8 *buffer, int buflen)
{
	/* duplicates are ignored */
	struct usb_cdc_union_desc *union_header = NULL;

	/* duplicates are not tolerated */
	struct usb_cdc_header_desc *header = NULL;
	struct usb_cdc_ether_desc *ether = NULL;
	struct usb_cdc_mdlm_detail_desc *detail = NULL;
	struct usb_cdc_mdlm_desc *desc = NULL;

	unsigned int elength;
	int cnt = 0;

	memset(hdr, 0x00, sizeof(struct usb_cdc_parsed_header));
	hdr->phonet_magic_present = false;
	while (buflen > 0) {
		elength = buffer[0];
		if (!elength) {
			dev_err(&intf->dev, "skipping garbage byte\n");
			elength = 1;
			goto next_desc;
		}
		if (buffer[1] != USB_DT_CS_INTERFACE) {
			dev_err(&intf->dev, "skipping garbage\n");
			goto next_desc;
		}

		switch (buffer[2]) {
		case USB_CDC_UNION_TYPE: /* we've found it */
			if (elength < sizeof(struct usb_cdc_union_desc))
				goto next_desc;
			if (union_header) {
				dev_err(&intf->dev, "More than one union descriptor, skipping ...\n");
				goto next_desc;
			}
			union_header = (struct usb_cdc_union_desc *)buffer;
			break;
		case USB_CDC_COUNTRY_TYPE:
			if (elength < sizeof(struct usb_cdc_country_functional_desc))
				goto next_desc;
			hdr->usb_cdc_country_functional_desc =
				(struct usb_cdc_country_functional_desc *)buffer;
			break;
		case USB_CDC_HEADER_TYPE:
			if (elength != sizeof(struct usb_cdc_header_desc))
				goto next_desc;
			if (header)
				return -EINVAL;
			header = (struct usb_cdc_header_desc *)buffer;
			break;
		case USB_CDC_ACM_TYPE:
			if (elength < sizeof(struct usb_cdc_acm_descriptor))
				goto next_desc;
			hdr->usb_cdc_acm_descriptor =
				(struct usb_cdc_acm_descriptor *)buffer;
			break;
		case USB_CDC_ETHERNET_TYPE:
			if (elength != sizeof(struct usb_cdc_ether_desc))
				goto next_desc;
			if (ether)
				return -EINVAL;
			ether = (struct usb_cdc_ether_desc *)buffer;
			break;
		case USB_CDC_CALL_MANAGEMENT_TYPE:
			if (elength < sizeof(struct usb_cdc_call_mgmt_descriptor))
				goto next_desc;
			hdr->usb_cdc_call_mgmt_descriptor =
				(struct usb_cdc_call_mgmt_descriptor *)buffer;
			break;
		case USB_CDC_DMM_TYPE:
			if (elength < sizeof(struct usb_cdc_dmm_desc))
				goto next_desc;
			hdr->usb_cdc_dmm_desc =
				(struct usb_cdc_dmm_desc *)buffer;
			break;
		case USB_CDC_MDLM_TYPE:
			if (elength < sizeof(struct usb_cdc_mdlm_desc *))
				goto next_desc;
			if (desc)
				return -EINVAL;
			desc = (struct usb_cdc_mdlm_desc *)buffer;
			break;
		case USB_CDC_MDLM_DETAIL_TYPE:
			if (elength < sizeof(struct usb_cdc_mdlm_detail_desc *))
				goto next_desc;
			if (detail)
				return -EINVAL;
			detail = (struct usb_cdc_mdlm_detail_desc *)buffer;
			break;
		case USB_CDC_NCM_TYPE:
			if (elength < sizeof(struct usb_cdc_ncm_desc))
				goto next_desc;
			hdr->usb_cdc_ncm_desc = (struct usb_cdc_ncm_desc *)buffer;
			break;
		case USB_CDC_MBIM_TYPE:
			if (elength < sizeof(struct usb_cdc_mbim_desc))
				goto next_desc;

			hdr->usb_cdc_mbim_desc = (struct usb_cdc_mbim_desc *)buffer;
			break;
		case USB_CDC_MBIM_EXTENDED_TYPE:
			if (elength < sizeof(struct usb_cdc_mbim_extended_desc))
				break;
			hdr->usb_cdc_mbim_extended_desc =
				(struct usb_cdc_mbim_extended_desc *)buffer;
			break;
		case CDC_PHONET_MAGIC_NUMBER:
			hdr->phonet_magic_present = true;
			break;
		default:
			/*
			 * there are LOTS more CDC descriptors that
			 * could legitimately be found here.
			 */
			dev_dbg(&intf->dev, "Ignoring descriptor: type %02x, length %ud\n",
					buffer[2], elength);
			goto next_desc;
		}
		cnt++;
next_desc:
		buflen -= elength;
		buffer += elength;
	}
	hdr->usb_cdc_union_desc = union_header;
	hdr->usb_cdc_header_desc = header;
	hdr->usb_cdc_mdlm_detail_desc = detail;
	hdr->usb_cdc_mdlm_desc = desc;
	hdr->usb_cdc_ether_desc = ether;
	return cnt;
}
EXPORT_SYMBOL_GPL(cdc_parse_cdc_header);
