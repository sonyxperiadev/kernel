

#ifndef __F_DTF_IF_H__
#define __F_DTF_IF_H__

/* Receive Event */
#define DTF_IF_EVENT_INTR_IN				1
#define DTF_IF_EVENT_SET_HALT_INTR_IN		2
#define DTF_IF_EVENT_SET_HALT_BULK_IN		3
#define DTF_IF_EVENT_SET_HALT_OUT			4
#define DTF_IF_EVENT_CLEAR_HALT_INTR_IN		5
#define DTF_IF_EVENT_CLEAR_HALT_BULK_IN		6
#define DTF_IF_EVENT_CLEAR_HALT_OUT			7
#define DTF_IF_EVENT_CTRL_IN				8
#define DTF_IF_EVENT_CTRL_OUT				9

/* Send Event */
#define DTF_IF_EVENT_SETUP					21
#define DTF_IF_EVENT_SET_ALT				22
#define DTF_IF_EVENT_DISABLE				23
#define DTF_IF_EVENT_SUSPEND				24
#define DTF_IF_EVENT_RESUME					25
#define DTF_IF_EVENT_COMPLETE_IN			26
#define DTF_IF_EVENT_COMPLETE_OUT			27
#define DTF_IF_EVENT_COMPLETE_INTR			28
#define DTF_IF_EVENT_CTRL_COMPLETE			29

#define DTF_IF_READ_DATA_SIZE			512
#define DTF_MAX_PACKET_SIZE			DTF_IF_READ_DATA_SIZE

struct dtf_if_write_data {
	int		size;
	char	data[16];
};

struct dtf_if_ctrlrequest {
	__u8 bRequestType;
	__u8 bRequest;
	__le16 wValue;
	__le16 wIndex;
	__le16 wLength;
};

struct dtf_if_read_data {
	int event_id;
	int status;
	int actual;
	int length;
	int speed_check;
	int size;
	char data[DTF_IF_READ_DATA_SIZE];
	struct dtf_if_ctrlrequest ctrl;
};

#endif /* __F_DTF_IF_H__ */
