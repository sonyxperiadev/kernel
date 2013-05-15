#include <linux/usb.h>
#include <linux/usb/f_dtf_if.h>

#ifndef __F_DTF_H__
#define __F_DTF_H__

struct sUsb_model_spcfc_func_desc {
	__u8    bLength;
	__u8    bDescriptorType;
	__u8    bDescriptorSubType;
	__u8    bType;
	__u8    bMode[3];
	int     mMode_num;
} __packed;

struct sDtf_pg {
	unsigned mCtrl_id;
	unsigned mData_id;

	struct usb_ep *ep_intr;
	struct usb_ep *ep_in;
	struct usb_ep *ep_out;

	struct usb_request *mReq_intr;
	struct usb_request *mReq_in;
	struct usb_request *mReq_out;
};

#define D_DTFUM_DTF_MINOR_NUMBER	245
				/* default dtfum dtf minor number */
#define D_VBUS_MINOR_NUMBER		246
				/* default vbus dtf minor number */
#define D_DTFIG_DTF_MINOR_NUMBER	247
				/* default dtfig dtf minor number */
#define D_DTFIU_DTF_MINOR_NUMBER	248
				/* default dtfiu dtf minor number */
#define D_DTF_MINOR_NUMBER		249
				/* default dtf minor number */

#endif /* __F_DTF_H__ */
