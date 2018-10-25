/* Copyright (c) 2012-2018, The Linux Foundation. All rights reserved.
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

#define pr_fmt(fmt) "%s: " fmt, __func__

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/bug.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/of_address.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/notifier.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/rbtree.h>
#include <soc/qcom/rpm-notifier.h>
#include <soc/qcom/rpm-smd.h>
#include <soc/qcom/smd.h>
#include <soc/qcom/glink_rpm_xprt.h>
#include <soc/qcom/glink.h>

#define CREATE_TRACE_POINTS
#include <trace/events/trace_rpm_smd.h>

/* Debug Definitions */
enum {
	MSM_RPM_LOG_REQUEST_PRETTY	= BIT(0),
	MSM_RPM_LOG_REQUEST_RAW		= BIT(1),
	MSM_RPM_LOG_REQUEST_SHOW_MSG_ID	= BIT(2),
};

static int msm_rpm_debug_mask;
module_param_named(
	debug_mask, msm_rpm_debug_mask, int, 0644
);

struct msm_rpm_driver_data {
	const char *ch_name;
	uint32_t ch_type;
	smd_channel_t *ch_info;
	struct work_struct work;
	spinlock_t smd_lock_write;
	spinlock_t smd_lock_read;
	struct completion smd_open;
};

struct glink_apps_rpm_data {
	const char *name;
	const char *edge;
	const char *xprt;
	void *glink_handle;
	struct glink_link_info *link_info;
	struct glink_open_config *open_cfg;
	struct work_struct work;
};

static bool glink_enabled;
static struct glink_apps_rpm_data *glink_data;

#define DEFAULT_BUFFER_SIZE 256
#define DEBUG_PRINT_BUFFER_SIZE 512
#define MAX_SLEEP_BUFFER 128
#define GFP_FLAG(noirq) (noirq ? GFP_ATOMIC : GFP_NOIO)
#define INV_RSC "resource does not exist"
#define ERR "err\0"
#define MAX_ERR_BUFFER_SIZE 128
#define MAX_WAIT_ON_ACK 24
#define INIT_ERROR 1
#define V1_PROTOCOL_VERSION 0x31726576 /* rev1 */
#define V0_PROTOCOL_VERSION 0 /* rev0 */
#define RPM_MSG_TYPE_OFFSET 16
#define RPM_MSG_TYPE_SIZE 8
#define RPM_SET_TYPE_OFFSET 28
#define RPM_SET_TYPE_SIZE 4
#define RPM_REQ_LEN_OFFSET 0
#define RPM_REQ_LEN_SIZE 16
#define RPM_MSG_VERSION_OFFSET 24
#define RPM_MSG_VERSION_SIZE 8
#define RPM_MSG_VERSION 1
#define RPM_MSG_SET_OFFSET 28
#define RPM_MSG_SET_SIZE 4
#define RPM_RSC_ID_OFFSET 16
#define RPM_RSC_ID_SIZE 12
#define RPM_DATA_LEN_OFFSET 0
#define RPM_DATA_LEN_SIZE 16
#define RPM_HDR_SIZE ((rpm_msg_fmt_ver == RPM_MSG_V0_FMT) ?\
		sizeof(struct rpm_v0_hdr) : sizeof(struct rpm_v1_hdr))
#define CLEAR_FIELD(offset, size) (~GENMASK(offset + size - 1, offset))

static ATOMIC_NOTIFIER_HEAD(msm_rpm_sleep_notifier);
static bool standalone;
static int probe_status = -EPROBE_DEFER;
static int msm_rpm_read_smd_data(char *buf);
static void msm_rpm_process_ack(uint32_t msg_id, int errno);

int msm_rpm_register_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_register(&msm_rpm_sleep_notifier, nb);
}

int msm_rpm_unregister_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_unregister(&msm_rpm_sleep_notifier, nb);
}

enum {
	MSM_RPM_MSG_REQUEST_TYPE = 0,
	MSM_RPM_MSG_TYPE_NR,
};

static const uint32_t msm_rpm_request_service_v1[MSM_RPM_MSG_TYPE_NR] = {
	0x716572, /* 'req\0' */
};

enum {
	RPM_V1_REQUEST_SERVICE,
	RPM_V1_SYSTEMDB_SERVICE,
	RPM_V1_COMMAND_SERVICE,
	RPM_V1_ACK_SERVICE,
	RPM_V1_NACK_SERVICE,
} msm_rpm_request_service_v2;

struct rpm_v0_hdr {
	uint32_t service_type;
	uint32_t request_len;
};

struct rpm_v1_hdr {
	uint32_t request_hdr;
};

struct rpm_message_header_v0 {
	struct rpm_v0_hdr hdr;
	uint32_t msg_id;
	enum msm_rpm_set set;
	uint32_t resource_type;
	uint32_t resource_id;
	uint32_t data_len;
};

struct rpm_message_header_v1 {
	struct rpm_v1_hdr hdr;
	uint32_t msg_id;
	uint32_t resource_type;
	uint32_t request_details;
};

struct msm_rpm_ack_msg_v0 {
	uint32_t req;
	uint32_t req_len;
	uint32_t rsc_id;
	uint32_t msg_len;
	uint32_t id_ack;
};

struct msm_rpm_ack_msg_v1 {
	uint32_t request_hdr;
	uint32_t id_ack;
};

struct kvp {
	unsigned int k;
	unsigned int s;
};

struct msm_rpm_kvp_data {
	uint32_t key;
	uint32_t nbytes; /* number of bytes */
	uint8_t *value;
	bool valid;
};

struct slp_buf {
	struct rb_node node;
	char ubuf[MAX_SLEEP_BUFFER];
	char *buf;
	bool valid;
};

enum rpm_msg_fmts {
	RPM_MSG_V0_FMT,
	RPM_MSG_V1_FMT
};

static uint32_t rpm_msg_fmt_ver;
module_param_named(
	rpm_msg_fmt_ver, rpm_msg_fmt_ver, uint, 0444
);

static struct rb_root tr_root = RB_ROOT;
static int (*msm_rpm_send_buffer)(char *buf, uint32_t size, bool noirq);
static int msm_rpm_send_smd_buffer(char *buf, uint32_t size, bool noirq);
static int msm_rpm_glink_send_buffer(char *buf, uint32_t size, bool noirq);
static uint32_t msm_rpm_get_next_msg_id(void);

static inline uint32_t get_offset_value(uint32_t val, uint32_t offset,
		uint32_t size)
{
	return (((val) & GENMASK(offset + size - 1, offset))
		>> offset);
}

static inline void change_offset_value(uint32_t *val, uint32_t offset,
		uint32_t size, int32_t val1)
{
	uint32_t member = *val;
	uint32_t offset_val = get_offset_value(member, offset, size);
	uint32_t mask = (1 << size) - 1;

	offset_val += val1;
	*val &= CLEAR_FIELD(offset, size);
	*val |= ((offset_val & mask) << offset);
}

static inline void set_offset_value(uint32_t *val, uint32_t offset,
		uint32_t size, uint32_t val1)
{
	uint32_t mask = (1 << size) - 1;

	*val &= CLEAR_FIELD(offset, size);
	*val |= ((val1 & mask) << offset);
}
static uint32_t get_msg_id(char *buf)
{
	if (rpm_msg_fmt_ver == RPM_MSG_V0_FMT)
		return ((struct rpm_message_header_v0 *)buf)->msg_id;

	return ((struct rpm_message_header_v1 *)buf)->msg_id;

}

static uint32_t get_ack_msg_id(char *buf)
{
	if (rpm_msg_fmt_ver == RPM_MSG_V0_FMT)
		return ((struct msm_rpm_ack_msg_v0 *)buf)->id_ack;

	return ((struct msm_rpm_ack_msg_v1 *)buf)->id_ack;

}

static uint32_t get_rsc_type(char *buf)
{
	if (rpm_msg_fmt_ver == RPM_MSG_V0_FMT)
		return ((struct rpm_message_header_v0 *)buf)->resource_type;

	return ((struct rpm_message_header_v1 *)buf)->resource_type;

}

static uint32_t get_set_type(char *buf)
{
	if (rpm_msg_fmt_ver == RPM_MSG_V0_FMT)
		return ((struct rpm_message_header_v0 *)buf)->set;

	return get_offset_value(((struct rpm_message_header_v1 *)buf)->
			request_details, RPM_SET_TYPE_OFFSET,
			RPM_SET_TYPE_SIZE);
}

static uint32_t get_data_len(char *buf)
{
	if (rpm_msg_fmt_ver == RPM_MSG_V0_FMT)
		return ((struct rpm_message_header_v0 *)buf)->data_len;

	return get_offset_value(((struct rpm_message_header_v1 *)buf)->
			request_details, RPM_DATA_LEN_OFFSET,
			RPM_DATA_LEN_SIZE);
}

static uint32_t get_rsc_id(char *buf)
{
	if (rpm_msg_fmt_ver == RPM_MSG_V0_FMT)
		return ((struct rpm_message_header_v0 *)buf)->resource_id;

	return get_offset_value(((struct rpm_message_header_v1 *)buf)->
			request_details, RPM_RSC_ID_OFFSET,
			RPM_RSC_ID_SIZE);
}

static uint32_t get_ack_req_len(char *buf)
{
	if (rpm_msg_fmt_ver == RPM_MSG_V0_FMT)
		return ((struct msm_rpm_ack_msg_v0 *)buf)->req_len;

	return get_offset_value(((struct msm_rpm_ack_msg_v1 *)buf)->
			request_hdr, RPM_REQ_LEN_OFFSET,
			RPM_REQ_LEN_SIZE);
}

static uint32_t get_ack_msg_type(char *buf)
{
	if (rpm_msg_fmt_ver == RPM_MSG_V0_FMT)
		return ((struct msm_rpm_ack_msg_v0 *)buf)->req;

	return get_offset_value(((struct msm_rpm_ack_msg_v1 *)buf)->
			request_hdr, RPM_MSG_TYPE_OFFSET,
			RPM_MSG_TYPE_SIZE);
}

static uint32_t get_req_len(char *buf)
{
	if (rpm_msg_fmt_ver == RPM_MSG_V0_FMT)
		return ((struct rpm_message_header_v0 *)buf)->hdr.request_len;

	return get_offset_value(((struct rpm_message_header_v1 *)buf)->
			hdr.request_hdr, RPM_REQ_LEN_OFFSET,
			RPM_REQ_LEN_SIZE);
}

static void set_msg_ver(char *buf, uint32_t val)
{
	if (rpm_msg_fmt_ver) {
		set_offset_value(&((struct rpm_message_header_v1 *)buf)->
			hdr.request_hdr, RPM_MSG_VERSION_OFFSET,
			RPM_MSG_VERSION_SIZE, val);
	} else {
		set_offset_value(&((struct rpm_message_header_v1 *)buf)->
			hdr.request_hdr, RPM_MSG_VERSION_OFFSET,
			RPM_MSG_VERSION_SIZE, 0);
	}
}

static void set_req_len(char *buf, uint32_t val)
{
	if (rpm_msg_fmt_ver == RPM_MSG_V0_FMT) {
		((struct rpm_message_header_v0 *)buf)->hdr.request_len = val;
	} else {
		set_offset_value(&((struct rpm_message_header_v1 *)buf)->
			hdr.request_hdr, RPM_REQ_LEN_OFFSET,
			RPM_REQ_LEN_SIZE, val);
	}
}

static void change_req_len(char *buf, int32_t val)
{
	if (rpm_msg_fmt_ver == RPM_MSG_V0_FMT) {
		((struct rpm_message_header_v0 *)buf)->hdr.request_len += val;
	} else {
		change_offset_value(&((struct rpm_message_header_v1 *)buf)->
			hdr.request_hdr, RPM_REQ_LEN_OFFSET,
			RPM_REQ_LEN_SIZE, val);
	}
}

static void set_msg_type(char *buf, uint32_t val)
{
	if (rpm_msg_fmt_ver == RPM_MSG_V0_FMT) {
		((struct rpm_message_header_v0 *)buf)->hdr.service_type =
			msm_rpm_request_service_v1[val];
	} else {
		set_offset_value(&((struct rpm_message_header_v1 *)buf)->
			hdr.request_hdr, RPM_MSG_TYPE_OFFSET,
			RPM_MSG_TYPE_SIZE, RPM_V1_REQUEST_SERVICE);
	}
}

static void set_rsc_id(char *buf, uint32_t val)
{
	if (rpm_msg_fmt_ver == RPM_MSG_V0_FMT)
		((struct rpm_message_header_v0 *)buf)->resource_id = val;
	else
		set_offset_value(&((struct rpm_message_header_v1 *)buf)->
			request_details, RPM_RSC_ID_OFFSET,
			RPM_RSC_ID_SIZE, val);
}

static void set_data_len(char *buf, uint32_t val)
{
	if (rpm_msg_fmt_ver == RPM_MSG_V0_FMT)
		((struct rpm_message_header_v0 *)buf)->data_len = val;
	else
		set_offset_value(&((struct rpm_message_header_v1 *)buf)->
			request_details, RPM_DATA_LEN_OFFSET,
			RPM_DATA_LEN_SIZE, val);
}
static void change_data_len(char *buf, int32_t val)
{
	if (rpm_msg_fmt_ver == RPM_MSG_V0_FMT)
		((struct rpm_message_header_v0 *)buf)->data_len += val;
	else
		change_offset_value(&((struct rpm_message_header_v1 *)buf)->
			request_details, RPM_DATA_LEN_OFFSET,
			RPM_DATA_LEN_SIZE, val);
}

static void set_set_type(char *buf, uint32_t val)
{
	if (rpm_msg_fmt_ver == RPM_MSG_V0_FMT)
		((struct rpm_message_header_v0 *)buf)->set = val;
	else
		set_offset_value(&((struct rpm_message_header_v1 *)buf)->
			request_details, RPM_SET_TYPE_OFFSET,
			RPM_SET_TYPE_SIZE, val);
}
static void set_msg_id(char *buf, uint32_t val)
{
	if (rpm_msg_fmt_ver == RPM_MSG_V0_FMT)
		((struct rpm_message_header_v0 *)buf)->msg_id = val;
	else
		((struct rpm_message_header_v1 *)buf)->msg_id = val;

}

static void set_rsc_type(char *buf, uint32_t val)
{
	if (rpm_msg_fmt_ver == RPM_MSG_V0_FMT)
		((struct rpm_message_header_v0 *)buf)->resource_type = val;
	else
		((struct rpm_message_header_v1 *)buf)->resource_type = val;
}

static inline int get_buf_len(char *buf)
{
	return get_req_len(buf) + RPM_HDR_SIZE;
}

static inline struct kvp *get_first_kvp(char *buf)
{
	if (rpm_msg_fmt_ver == RPM_MSG_V0_FMT)
		return (struct kvp *)(buf +
				sizeof(struct rpm_message_header_v0));
	else
		return (struct kvp *)(buf +
				sizeof(struct rpm_message_header_v1));
}

static inline struct kvp *get_next_kvp(struct kvp *k)
{
	return (struct kvp *)((void *)k + sizeof(*k) + k->s);
}

static inline void *get_data(struct kvp *k)
{
	return (void *)k + sizeof(*k);
}


static void delete_kvp(char *buf, struct kvp *d)
{
	struct kvp *n;
	int dec;
	uint32_t size;

	n = get_next_kvp(d);
	dec = (void *)n - (void *)d;
	size = get_data_len(buf) -
		((void *)n - (void *)get_first_kvp(buf));

	memcpy((void *)d, (void *)n, size);

	change_data_len(buf, -dec);
	change_req_len(buf, -dec);
}

static inline void update_kvp_data(struct kvp *dest, struct kvp *src)
{
	memcpy(get_data(dest), get_data(src), src->s);
}

static void add_kvp(char *buf, struct kvp *n)
{
	int32_t inc = sizeof(*n) + n->s;

	if (get_req_len(buf) + inc > MAX_SLEEP_BUFFER) {
		WARN_ON(get_req_len(buf) + inc > MAX_SLEEP_BUFFER);
		return;
	}

	memcpy(buf + get_buf_len(buf), n, inc);

	change_data_len(buf, inc);
	change_req_len(buf, inc);
}

static struct slp_buf *tr_search(struct rb_root *root, char *slp)
{
	unsigned int type = get_rsc_type(slp);
	unsigned int id = get_rsc_id(slp);
	struct rb_node *node = root->rb_node;

	while (node) {
		struct slp_buf *cur = rb_entry(node, struct slp_buf, node);
		unsigned int ctype = get_rsc_type(cur->buf);
		unsigned int cid = get_rsc_id(cur->buf);

		if (type < ctype)
			node = node->rb_left;
		else if (type > ctype)
			node = node->rb_right;
		else if (id < cid)
			node = node->rb_left;
		else if (id > cid)
			node = node->rb_right;
		else
			return cur;
	}
	return NULL;
}

static int tr_insert(struct rb_root *root, struct slp_buf *slp)
{
	unsigned int type = get_rsc_type(slp->buf);
	unsigned int id = get_rsc_id(slp->buf);
	struct rb_node **node = &(root->rb_node), *parent = NULL;

	while (*node) {
		struct slp_buf *curr = rb_entry(*node, struct slp_buf, node);
		unsigned int ctype = get_rsc_type(curr->buf);
		unsigned int cid = get_rsc_id(curr->buf);

		parent = *node;

		if (type < ctype)
			node = &((*node)->rb_left);
		else if (type > ctype)
			node = &((*node)->rb_right);
		else if (id < cid)
			node = &((*node)->rb_left);
		else if (id > cid)
			node = &((*node)->rb_right);
		else
			return -EINVAL;
	}

	rb_link_node(&slp->node, parent, node);
	rb_insert_color(&slp->node, root);
	slp->valid = true;
	return 0;
}

#define for_each_kvp(buf, k) \
	for (k = (struct kvp *)get_first_kvp(buf); \
		((void *)k - (void *)get_first_kvp(buf)) < \
		 get_data_len(buf);\
		k = get_next_kvp(k))


static void tr_update(struct slp_buf *s, char *buf)
{
	struct kvp *e, *n;

	for_each_kvp(buf, n) {
		bool found = false;

		for_each_kvp(s->buf, e) {
			if (n->k == e->k) {
				found = true;
				if (n->s == e->s) {
					void *e_data = get_data(e);
					void *n_data = get_data(n);

					if (memcmp(e_data, n_data, n->s)) {
						update_kvp_data(e, n);
						s->valid = true;
					}
				} else {
					delete_kvp(s->buf, e);
					add_kvp(s->buf, n);
					s->valid = true;
				}
				break;
			}

		}
		if (!found) {
			add_kvp(s->buf, n);
			s->valid = true;
		}
	}
}
static atomic_t msm_rpm_msg_id = ATOMIC_INIT(0);

struct msm_rpm_request {
	uint8_t *client_buf;
	struct msm_rpm_kvp_data *kvp;
	uint32_t num_elements;
	uint32_t write_idx;
	uint8_t *buf;
	uint32_t numbytes;
};

/*
 * Data related to message acknowledgment
 */

LIST_HEAD(msm_rpm_wait_list);

struct msm_rpm_wait_data {
	struct list_head list;
	uint32_t msg_id;
	bool ack_recd;
	int errno;
	struct completion ack;
	bool delete_on_ack;
};
DEFINE_SPINLOCK(msm_rpm_list_lock);



LIST_HEAD(msm_rpm_ack_list);

static struct tasklet_struct data_tasklet;

static inline uint32_t msm_rpm_get_msg_id_from_ack(uint8_t *buf)
{
	return get_ack_msg_id(buf);
}

static inline int msm_rpm_get_error_from_ack(uint8_t *buf)
{
	uint8_t *tmp;
	uint32_t req_len = get_ack_req_len(buf);
	uint32_t msg_type = get_ack_msg_type(buf);
	int rc = -ENODEV;
	uint32_t err;
	uint32_t ack_msg_size = rpm_msg_fmt_ver ?
			sizeof(struct msm_rpm_ack_msg_v1) :
			sizeof(struct msm_rpm_ack_msg_v0);

	if (rpm_msg_fmt_ver == RPM_MSG_V0_FMT &&
			msg_type == RPM_V1_ACK_SERVICE) {
		return 0;
	} else if (rpm_msg_fmt_ver && msg_type == RPM_V1_NACK_SERVICE) {
		err = *(uint32_t *)(buf + sizeof(struct msm_rpm_ack_msg_v1));
		return err;
	}

	req_len -= ack_msg_size;
	req_len += 2 * sizeof(uint32_t);
	if (!req_len)
		return 0;

	pr_err("%s:rpm returned error or nack req_len: %d id_ack: %d\n",
				__func__, req_len, get_ack_msg_id(buf));

	tmp = buf + ack_msg_size;

	if (memcmp(tmp, ERR, sizeof(uint32_t))) {
		pr_err("%s rpm returned error\n", __func__);
		WARN_ON(1);
	}

	tmp += 2 * sizeof(uint32_t);

	if (!(memcmp(tmp, INV_RSC, min_t(uint32_t, req_len,
						sizeof(INV_RSC))-1))) {
		pr_err("%s(): RPM NACK Unsupported resource\n", __func__);
		rc = -EINVAL;
	} else {
		pr_err("%s(): RPM NACK Invalid header\n", __func__);
	}

	return rc;
}

int msm_rpm_smd_buffer_request(struct msm_rpm_request *cdata,
		uint32_t size, gfp_t flag)
{
	struct slp_buf *slp;
	static DEFINE_SPINLOCK(slp_buffer_lock);
	unsigned long flags;
	char *buf;

	buf = cdata->buf;

	if (size > MAX_SLEEP_BUFFER)
		return -ENOMEM;

	spin_lock_irqsave(&slp_buffer_lock, flags);
	slp = tr_search(&tr_root, buf);

	if (!slp) {
		slp = kzalloc(sizeof(struct slp_buf), GFP_ATOMIC);
		if (!slp) {
			spin_unlock_irqrestore(&slp_buffer_lock, flags);
			return -ENOMEM;
		}
		slp->buf = PTR_ALIGN(&slp->ubuf[0], sizeof(u32));
		memcpy(slp->buf, buf, size);
		if (tr_insert(&tr_root, slp))
			pr_err("Error updating sleep request\n");
	} else {
		/* handle unsent requests */
		tr_update(slp, buf);
	}
	trace_rpm_smd_sleep_set(get_msg_id(cdata->client_buf),
			get_rsc_type(cdata->client_buf),
			get_req_len(cdata->client_buf));

	spin_unlock_irqrestore(&slp_buffer_lock, flags);

	return 0;
}

static struct msm_rpm_driver_data msm_rpm_data = {
	.smd_open = COMPLETION_INITIALIZER(msm_rpm_data.smd_open),
};

static int msm_rpm_glink_rx_poll(void *glink_handle)
{
	int ret;

	ret = glink_rpm_rx_poll(glink_handle);
	if (ret >= 0)
		/*
		 * Sleep for 50us at a time before checking
		 * for packet availability. The 50us is based
		 * on the the time rpm could take to process
		 * and send an ack for the sleep set request.
		 */
		udelay(50);
	else
		pr_err("Not receieve an ACK from RPM. ret = %d\n", ret);

	return ret;
}

/*
 * Returns
 *	= 0 on successful reads
 *	> 0 on successful reads with no further data
 *	standard Linux error codes on failure.
 */
static int msm_rpm_read_sleep_ack(void)
{
	int ret;
	char buf[MAX_ERR_BUFFER_SIZE] = {0};
	uint32_t msg_id;

	if (glink_enabled)
		ret = msm_rpm_glink_rx_poll(glink_data->glink_handle);
	else {
		int timeout = 10;

		while (timeout) {
			if (smd_is_pkt_avail(msm_rpm_data.ch_info))
				break;
			/*
			 * Sleep for 50us at a time before checking
			 * for packet availability. The 50us is based
			 * on the the max time rpm could take to process
			 * and send an ack for sleep set request.
			 */
			udelay(50);
			timeout--;
		}

		/*
		 * On timeout return an error and exit the spinlock
		 * control on this cpu. This will allow any other
		 * core that has wokenup and trying to acquire the
		 * spinlock from being locked out.
		 */

		if (!timeout)
			return -EAGAIN;

		ret = msm_rpm_read_smd_data(buf);
		if (!ret) {
			/*
			 * Mimic Glink behavior to ensure that the
			 * data is read and the msg is removed from
			 * the wait list. We should have gotten here
			 * only when there are no drivers waiting on
			 * ACKs. msm_rpm_get_entry_from_msg_id()
			 * return non-NULL only then.
			 */
			msg_id = msm_rpm_get_msg_id_from_ack(buf);
			msm_rpm_process_ack(msg_id, 0);
			ret = smd_is_pkt_avail(msm_rpm_data.ch_info);
		}
	}
	return ret;
}

static void msm_rpm_flush_noack_messages(void)
{
	while (!list_empty(&msm_rpm_wait_list)) {
		if (!msm_rpm_read_sleep_ack())
			break;
	}
}

static int msm_rpm_flush_requests(bool print)
{
	struct rb_node *t;
	int ret;
	int count = 0;

	msm_rpm_flush_noack_messages();

	for (t = rb_first(&tr_root); t; t = rb_next(t)) {

		struct slp_buf *s = rb_entry(t, struct slp_buf, node);
		unsigned int type = get_rsc_type(s->buf);
		unsigned int id = get_rsc_id(s->buf);

		if (!s->valid)
			continue;

		set_msg_id(s->buf, msm_rpm_get_next_msg_id());

		if (!glink_enabled)
			ret = msm_rpm_send_smd_buffer(s->buf,
					get_buf_len(s->buf), true);
		else
			ret = msm_rpm_glink_send_buffer(s->buf,
					get_buf_len(s->buf), true);

		WARN_ON(ret != get_buf_len(s->buf));
		trace_rpm_smd_send_sleep_set(get_msg_id(s->buf), type, id);

		s->valid = false;
		count++;

		/*
		 * RPM acks need to be handled here if we have sent 24
		 * messages such that we do not overrun SMD buffer. Since
		 * we expect only sleep sets at this point (RPM PC would be
		 * disallowed if we had pending active requests), we need not
		 * process these sleep set acks.
		 */
		if (count >= MAX_WAIT_ON_ACK) {
			int ret = msm_rpm_read_sleep_ack();

			if (ret >= 0)
				count--;
			else {
				pr_err("Timed out waiting for RPM ACK\n");
				return ret;
			}
		}
	}
	return 0;
}

static void msm_rpm_notify_sleep_chain(char *buf,
		struct msm_rpm_kvp_data *kvp)
{
	struct msm_rpm_notifier_data notif;

	notif.rsc_type = get_rsc_type(buf);
	notif.rsc_id = get_req_len(buf);
	notif.key = kvp->key;
	notif.size = kvp->nbytes;
	notif.value = kvp->value;
	atomic_notifier_call_chain(&msm_rpm_sleep_notifier, 0, &notif);
}

static int msm_rpm_add_kvp_data_common(struct msm_rpm_request *handle,
		uint32_t key, const uint8_t *data, int size, bool noirq)
{
	uint32_t i;
	uint32_t data_size, msg_size;

	if (probe_status)
		return probe_status;

	if (!handle || !data) {
		pr_err("%s(): Invalid handle/data\n", __func__);
		return -EINVAL;
	}

	if (size < 0)
		return  -EINVAL;

	data_size = ALIGN(size, SZ_4);
	msg_size = data_size + 8;

	for (i = 0; i < handle->write_idx; i++) {
		if (handle->kvp[i].key != key)
			continue;
		if (handle->kvp[i].nbytes != data_size) {
			kfree(handle->kvp[i].value);
			handle->kvp[i].value = NULL;
		} else {
			if (!memcmp(handle->kvp[i].value, data, data_size))
				return 0;
		}
		break;
	}

	if (i >= handle->num_elements) {
		pr_err("Number of resources exceeds max allocated\n");
		return -ENOMEM;
	}

	if (i == handle->write_idx)
		handle->write_idx++;

	if (!handle->kvp[i].value) {
		handle->kvp[i].value = kzalloc(data_size, GFP_FLAG(noirq));

		if (!handle->kvp[i].value)
			return -ENOMEM;
	} else {
		/* We enter the else case, if a key already exists but the
		 * data doesn't match. In which case, we should zero the data
		 * out.
		 */
		memset(handle->kvp[i].value, 0, data_size);
	}

	if (!handle->kvp[i].valid)
		change_data_len(handle->client_buf, msg_size);
	else
		change_data_len(handle->client_buf,
			(data_size - handle->kvp[i].nbytes));

	handle->kvp[i].nbytes = data_size;
	handle->kvp[i].key = key;
	memcpy(handle->kvp[i].value, data, size);
	handle->kvp[i].valid = true;

	return 0;

}

static struct msm_rpm_request *msm_rpm_create_request_common(
		enum msm_rpm_set set, uint32_t rsc_type, uint32_t rsc_id,
		int num_elements, bool noirq)
{
	struct msm_rpm_request *cdata;
	uint32_t buf_size;

	if (probe_status)
		return ERR_PTR(probe_status);

	cdata = kzalloc(sizeof(struct msm_rpm_request),
			GFP_FLAG(noirq));

	if (!cdata) {
		pr_err("Cannot allocate memory for client data\n");
		goto cdata_alloc_fail;
	}

	if (rpm_msg_fmt_ver == RPM_MSG_V0_FMT)
		buf_size = sizeof(struct rpm_message_header_v0);
	else
		buf_size = sizeof(struct rpm_message_header_v1);

	cdata->client_buf = kzalloc(buf_size, GFP_FLAG(noirq));

	if (!cdata->client_buf)
		goto client_buf_alloc_fail;

	set_set_type(cdata->client_buf, set);
	set_rsc_type(cdata->client_buf, rsc_type);
	set_rsc_id(cdata->client_buf, rsc_id);

	cdata->num_elements = num_elements;
	cdata->write_idx = 0;

	cdata->kvp = kcalloc(num_elements, sizeof(struct msm_rpm_kvp_data),
			GFP_FLAG(noirq));

	if (!cdata->kvp) {
		pr_warn("%s(): Cannot allocate memory for key value data\n",
				__func__);
		goto kvp_alloc_fail;
	}

	cdata->buf = kzalloc(DEFAULT_BUFFER_SIZE, GFP_FLAG(noirq));

	if (!cdata->buf)
		goto buf_alloc_fail;

	cdata->numbytes = DEFAULT_BUFFER_SIZE;
	return cdata;

buf_alloc_fail:
	kfree(cdata->kvp);
kvp_alloc_fail:
	kfree(cdata->client_buf);
client_buf_alloc_fail:
	kfree(cdata);
cdata_alloc_fail:
	return NULL;

}

void msm_rpm_free_request(struct msm_rpm_request *handle)
{
	int i;

	if (!handle)
		return;
	for (i = 0; i < handle->num_elements; i++)
		kfree(handle->kvp[i].value);
	kfree(handle->kvp);
	kfree(handle->client_buf);
	kfree(handle->buf);
	kfree(handle);
}
EXPORT_SYMBOL(msm_rpm_free_request);

struct msm_rpm_request *msm_rpm_create_request(
		enum msm_rpm_set set, uint32_t rsc_type,
		uint32_t rsc_id, int num_elements)
{
	return msm_rpm_create_request_common(set, rsc_type, rsc_id,
			num_elements, false);
}
EXPORT_SYMBOL(msm_rpm_create_request);

struct msm_rpm_request *msm_rpm_create_request_noirq(
		enum msm_rpm_set set, uint32_t rsc_type,
		uint32_t rsc_id, int num_elements)
{
	return msm_rpm_create_request_common(set, rsc_type, rsc_id,
			num_elements, true);
}
EXPORT_SYMBOL(msm_rpm_create_request_noirq);

int msm_rpm_add_kvp_data(struct msm_rpm_request *handle,
		uint32_t key, const uint8_t *data, int size)
{
	return msm_rpm_add_kvp_data_common(handle, key, data, size, false);

}
EXPORT_SYMBOL(msm_rpm_add_kvp_data);

int msm_rpm_add_kvp_data_noirq(struct msm_rpm_request *handle,
		uint32_t key, const uint8_t *data, int size)
{
	return msm_rpm_add_kvp_data_common(handle, key, data, size, true);
}
EXPORT_SYMBOL(msm_rpm_add_kvp_data_noirq);

/* Runs in interrupt context */
static void msm_rpm_notify(void *data, unsigned int event)
{
	struct msm_rpm_driver_data *pdata = (struct msm_rpm_driver_data *)data;

	WARN_ON(!pdata);

	if (!(pdata->ch_info))
		return;

	switch (event) {
	case SMD_EVENT_DATA:
		tasklet_schedule(&data_tasklet);
		trace_rpm_smd_interrupt_notify("interrupt notification");
		break;
	case SMD_EVENT_OPEN:
		complete(&pdata->smd_open);
		break;
	case SMD_EVENT_CLOSE:
	case SMD_EVENT_STATUS:
	case SMD_EVENT_REOPEN_READY:
		break;
	default:
		pr_info("Unknown SMD event\n");

	}
}

bool msm_rpm_waiting_for_ack(void)
{
	bool ret = false;
	unsigned long flags;
	struct msm_rpm_wait_data *elem = NULL;

	spin_lock_irqsave(&msm_rpm_list_lock, flags);
	elem = list_first_entry_or_null(&msm_rpm_wait_list,
				struct msm_rpm_wait_data, list);
	if (elem)
		ret = !elem->delete_on_ack;
	spin_unlock_irqrestore(&msm_rpm_list_lock, flags);

	return ret;
}

static struct msm_rpm_wait_data *msm_rpm_get_entry_from_msg_id(uint32_t msg_id)
{
	struct list_head *ptr;
	struct msm_rpm_wait_data *elem = NULL;
	unsigned long flags;

	spin_lock_irqsave(&msm_rpm_list_lock, flags);

	list_for_each(ptr, &msm_rpm_wait_list) {
		elem = list_entry(ptr, struct msm_rpm_wait_data, list);
		if (elem && (elem->msg_id == msg_id))
			break;
		elem = NULL;
	}
	spin_unlock_irqrestore(&msm_rpm_list_lock, flags);
	return elem;
}

static uint32_t msm_rpm_get_next_msg_id(void)
{
	uint32_t id;

	/*
	 * A message id of 0 is used by the driver to indicate a error
	 * condition. The RPM driver uses a id of 1 to indicate unsent data
	 * when the data sent over hasn't been modified. This isn't a error
	 * scenario and wait for ack returns a success when the message id is 1.
	 */

	do {
		id = atomic_inc_return(&msm_rpm_msg_id);
	} while ((id == 0) || (id == 1) || msm_rpm_get_entry_from_msg_id(id));

	return id;
}

static int msm_rpm_add_wait_list(uint32_t msg_id, bool delete_on_ack)
{
	unsigned long flags;
	struct msm_rpm_wait_data *data =
		kzalloc(sizeof(struct msm_rpm_wait_data), GFP_ATOMIC);

	if (!data)
		return -ENOMEM;

	init_completion(&data->ack);
	data->ack_recd = false;
	data->msg_id = msg_id;
	data->errno = INIT_ERROR;
	data->delete_on_ack = delete_on_ack;
	spin_lock_irqsave(&msm_rpm_list_lock, flags);
	if (delete_on_ack)
		list_add_tail(&data->list, &msm_rpm_wait_list);
	else
		list_add(&data->list, &msm_rpm_wait_list);
	spin_unlock_irqrestore(&msm_rpm_list_lock, flags);

	return 0;
}

static void msm_rpm_free_list_entry(struct msm_rpm_wait_data *elem)
{
	unsigned long flags;

	spin_lock_irqsave(&msm_rpm_list_lock, flags);
	list_del(&elem->list);
	spin_unlock_irqrestore(&msm_rpm_list_lock, flags);
	kfree(elem);
}

static void msm_rpm_process_ack(uint32_t msg_id, int errno)
{
	struct list_head *ptr, *next;
	struct msm_rpm_wait_data *elem = NULL;
	unsigned long flags;

	spin_lock_irqsave(&msm_rpm_list_lock, flags);

	list_for_each_safe(ptr, next, &msm_rpm_wait_list) {
		elem = list_entry(ptr, struct msm_rpm_wait_data, list);
		if (elem->msg_id == msg_id) {
			elem->errno = errno;
			elem->ack_recd = true;
			complete(&elem->ack);
			if (elem->delete_on_ack) {
				list_del(&elem->list);
				kfree(elem);
			}
			break;
		}
	}
	/* Special case where the sleep driver doesn't
	 * wait for ACKs. This would decrease the latency involved with
	 * entering RPM assisted power collapse.
	 */
	if (!elem)
		trace_rpm_smd_ack_recvd(0, msg_id, 0xDEADBEEF);

	spin_unlock_irqrestore(&msm_rpm_list_lock, flags);
}

struct msm_rpm_kvp_packet {
	uint32_t id;
	uint32_t len;
	uint32_t val;
};

static int msm_rpm_read_smd_data(char *buf)
{
	int pkt_sz;
	int bytes_read = 0;

	pkt_sz = smd_cur_packet_size(msm_rpm_data.ch_info);

	if (!pkt_sz)
		return -EAGAIN;

	if (pkt_sz > MAX_ERR_BUFFER_SIZE) {
		pr_err("rpm_smd pkt_sz is greater than max size\n");
		goto error;
	}

	if (pkt_sz != smd_read_avail(msm_rpm_data.ch_info))
		return -EAGAIN;

	do {
		int len;

		len = smd_read(msm_rpm_data.ch_info, buf + bytes_read, pkt_sz);
		pkt_sz -= len;
		bytes_read += len;

	} while (pkt_sz > 0);

	if (pkt_sz < 0) {
		pr_err("rpm_smd pkt_sz is less than zero\n");
		goto error;
	}
	return 0;
error:
	WARN_ON(1);

	return 0;
}

static void data_fn_tasklet(unsigned long data)
{
	uint32_t msg_id;
	int errno;
	char buf[MAX_ERR_BUFFER_SIZE] = {0};

	spin_lock(&msm_rpm_data.smd_lock_read);
	while (smd_is_pkt_avail(msm_rpm_data.ch_info)) {
		if (msm_rpm_read_smd_data(buf))
			break;
		msg_id = msm_rpm_get_msg_id_from_ack(buf);
		errno = msm_rpm_get_error_from_ack(buf);
		trace_rpm_smd_ack_recvd(0, msg_id, errno);
		msm_rpm_process_ack(msg_id, errno);
	}
	spin_unlock(&msm_rpm_data.smd_lock_read);
}

static void msm_rpm_log_request(struct msm_rpm_request *cdata)
{
	char buf[DEBUG_PRINT_BUFFER_SIZE];
	size_t buflen = DEBUG_PRINT_BUFFER_SIZE;
	char name[5];
	u32 value;
	uint32_t i;
	int j, prev_valid;
	int valid_count = 0;
	int pos = 0;
	uint32_t res_type, rsc_id;

	name[4] = 0;

	for (i = 0; i < cdata->write_idx; i++)
		if (cdata->kvp[i].valid)
			valid_count++;

	pos += scnprintf(buf + pos, buflen - pos, "%sRPM req: ", KERN_INFO);
	if (msm_rpm_debug_mask & MSM_RPM_LOG_REQUEST_SHOW_MSG_ID)
		pos += scnprintf(buf + pos, buflen - pos, "msg_id=%u, ",
				get_msg_id(cdata->client_buf));
	pos += scnprintf(buf + pos, buflen - pos, "s=%s",
		(get_set_type(cdata->client_buf) ==
				MSM_RPM_CTX_ACTIVE_SET ? "act" : "slp"));

	res_type = get_rsc_type(cdata->client_buf);
	rsc_id = get_rsc_id(cdata->client_buf);
	if ((msm_rpm_debug_mask & MSM_RPM_LOG_REQUEST_PRETTY)
	    && (msm_rpm_debug_mask & MSM_RPM_LOG_REQUEST_RAW)) {
		/* Both pretty and raw formatting */
		memcpy(name, &res_type, sizeof(uint32_t));
		pos += scnprintf(buf + pos, buflen - pos,
			", rsc_type=0x%08X (%s), rsc_id=%u; ",
			res_type, name, rsc_id);

		for (i = 0, prev_valid = 0; i < cdata->write_idx; i++) {
			if (!cdata->kvp[i].valid)
				continue;

			memcpy(name, &cdata->kvp[i].key, sizeof(uint32_t));
			pos += scnprintf(buf + pos, buflen - pos,
					"[key=0x%08X (%s), value=%s",
					cdata->kvp[i].key, name,
					(cdata->kvp[i].nbytes ? "0x" : "null"));

			for (j = 0; j < cdata->kvp[i].nbytes; j++)
				pos += scnprintf(buf + pos, buflen - pos,
						"%02X ",
						cdata->kvp[i].value[j]);

			if (cdata->kvp[i].nbytes)
				pos += scnprintf(buf + pos, buflen - pos, "(");

			for (j = 0; j < cdata->kvp[i].nbytes; j += 4) {
				value = 0;
				memcpy(&value, &cdata->kvp[i].value[j],
					min_t(uint32_t, sizeof(uint32_t),
						cdata->kvp[i].nbytes - j));
				pos += scnprintf(buf + pos, buflen - pos, "%u",
						value);
				if (j + 4 < cdata->kvp[i].nbytes)
					pos += scnprintf(buf + pos,
						buflen - pos, " ");
			}
			if (cdata->kvp[i].nbytes)
				pos += scnprintf(buf + pos, buflen - pos, ")");
			pos += scnprintf(buf + pos, buflen - pos, "]");
			if (prev_valid + 1 < valid_count)
				pos += scnprintf(buf + pos, buflen - pos, ", ");
			prev_valid++;
		}
	} else if (msm_rpm_debug_mask & MSM_RPM_LOG_REQUEST_PRETTY) {
		/* Pretty formatting only */
		memcpy(name, &res_type, sizeof(uint32_t));
		pos += scnprintf(buf + pos, buflen - pos, " %s %u; ", name,
			rsc_id);

		for (i = 0, prev_valid = 0; i < cdata->write_idx; i++) {
			if (!cdata->kvp[i].valid)
				continue;

			memcpy(name, &cdata->kvp[i].key, sizeof(uint32_t));
			pos += scnprintf(buf + pos, buflen - pos, "%s=%s",
				name, (cdata->kvp[i].nbytes ? "" : "null"));

			for (j = 0; j < cdata->kvp[i].nbytes; j += 4) {
				value = 0;
				memcpy(&value, &cdata->kvp[i].value[j],
					min_t(uint32_t, sizeof(uint32_t),
						cdata->kvp[i].nbytes - j));
				pos += scnprintf(buf + pos, buflen - pos, "%u",
						value);

				if (j + 4 < cdata->kvp[i].nbytes)
					pos += scnprintf(buf + pos,
						buflen - pos, " ");
			}
			if (prev_valid + 1 < valid_count)
				pos += scnprintf(buf + pos, buflen - pos, ", ");
			prev_valid++;
		}
	} else {
		/* Raw formatting only */
		pos += scnprintf(buf + pos, buflen - pos,
			", rsc_type=0x%08X, rsc_id=%u; ", res_type, rsc_id);

		for (i = 0, prev_valid = 0; i < cdata->write_idx; i++) {
			if (!cdata->kvp[i].valid)
				continue;

			pos += scnprintf(buf + pos, buflen - pos,
					"[key=0x%08X, value=%s",
					cdata->kvp[i].key,
					(cdata->kvp[i].nbytes ? "0x" : "null"));
			for (j = 0; j < cdata->kvp[i].nbytes; j++) {
				pos += scnprintf(buf + pos, buflen - pos,
						"%02X",
						cdata->kvp[i].value[j]);
				if (j + 1 < cdata->kvp[i].nbytes)
					pos += scnprintf(buf + pos,
							buflen - pos, " ");
			}
			pos += scnprintf(buf + pos, buflen - pos, "]");
			if (prev_valid + 1 < valid_count)
				pos += scnprintf(buf + pos, buflen - pos, ", ");
			prev_valid++;
		}
	}

	pos += scnprintf(buf + pos, buflen - pos, "\n");
	printk(buf);
}

static int msm_rpm_send_smd_buffer(char *buf, uint32_t size, bool noirq)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&msm_rpm_data.smd_lock_write, flags);
	ret = smd_write_avail(msm_rpm_data.ch_info);

	while ((ret = smd_write_avail(msm_rpm_data.ch_info)) < size) {
		if (ret < 0)
			break;
		if (!noirq) {
			spin_unlock_irqrestore(
				&msm_rpm_data.smd_lock_write, flags);
			cpu_relax();
			spin_lock_irqsave(
				&msm_rpm_data.smd_lock_write, flags);
		} else
			udelay(5);
	}

	if (ret < 0) {
		pr_err("SMD not initialized\n");
		spin_unlock_irqrestore(
			&msm_rpm_data.smd_lock_write, flags);
		return ret;
	}

	ret = smd_write(msm_rpm_data.ch_info, buf, size);
	spin_unlock_irqrestore(&msm_rpm_data.smd_lock_write, flags);
	return ret;
}

static int msm_rpm_glink_send_buffer(char *buf, uint32_t size, bool noirq)
{
	int ret;
	unsigned long flags;
	int timeout = 5;

	spin_lock_irqsave(&msm_rpm_data.smd_lock_write, flags);
	do {
		ret = glink_tx(glink_data->glink_handle, buf, buf,
					size, GLINK_TX_SINGLE_THREADED);
		if (ret == -EBUSY || ret == -ENOSPC) {
			if (!noirq) {
				spin_unlock_irqrestore(
					&msm_rpm_data.smd_lock_write, flags);
				cpu_relax();
				spin_lock_irqsave(
					&msm_rpm_data.smd_lock_write, flags);
			} else {
				udelay(100);
			}
			timeout--;
		} else {
			ret = 0;
		}
	} while (ret && timeout);
	spin_unlock_irqrestore(&msm_rpm_data.smd_lock_write, flags);

	if (!timeout)
		return 0;
	else
		return size;
}

static int msm_rpm_send_data(struct msm_rpm_request *cdata,
		int msg_type, bool noirq, bool noack)
{
	uint8_t *tmpbuff;
	int ret;
	uint32_t i;
	uint32_t msg_size;
	int msg_hdr_sz, req_hdr_sz;
	uint32_t data_len = get_data_len(cdata->client_buf);
	uint32_t set = get_set_type(cdata->client_buf);
	uint32_t msg_id;

	if (probe_status)
		return probe_status;

	if (!data_len)
		return 1;

	msg_hdr_sz = rpm_msg_fmt_ver ? sizeof(struct rpm_message_header_v1) :
			sizeof(struct rpm_message_header_v0);

	req_hdr_sz = RPM_HDR_SIZE;
	set_msg_type(cdata->client_buf, msg_type);

	set_req_len(cdata->client_buf, data_len + msg_hdr_sz - req_hdr_sz);
	msg_size = get_req_len(cdata->client_buf) + req_hdr_sz;

	/* populate data_len */
	if (msg_size > cdata->numbytes) {
		kfree(cdata->buf);
		cdata->numbytes = msg_size;
		cdata->buf = kzalloc(msg_size, GFP_FLAG(noirq));
	}

	if (!cdata->buf) {
		pr_err("Failed malloc\n");
		return 0;
	}

	tmpbuff = cdata->buf;

	tmpbuff += msg_hdr_sz;
	for (i = 0; (i < cdata->write_idx); i++) {
		/* Sanity check */
		WARN_ON((tmpbuff - cdata->buf) > cdata->numbytes);

		if (!cdata->kvp[i].valid)
			continue;

		memcpy(tmpbuff, &cdata->kvp[i].key, sizeof(uint32_t));
		tmpbuff += sizeof(uint32_t);

		memcpy(tmpbuff, &cdata->kvp[i].nbytes, sizeof(uint32_t));
		tmpbuff += sizeof(uint32_t);

		memcpy(tmpbuff, cdata->kvp[i].value, cdata->kvp[i].nbytes);
		tmpbuff += cdata->kvp[i].nbytes;

		if (set == MSM_RPM_CTX_SLEEP_SET)
			msm_rpm_notify_sleep_chain(cdata->client_buf,
					&cdata->kvp[i]);

	}

	memcpy(cdata->buf, cdata->client_buf, msg_hdr_sz);
	if ((set == MSM_RPM_CTX_SLEEP_SET) &&
		!msm_rpm_smd_buffer_request(cdata, msg_size,
			GFP_FLAG(noirq)))
		return 1;

	msg_id = msm_rpm_get_next_msg_id();
	/* Set the version bit for new protocol */
	set_msg_ver(cdata->buf, rpm_msg_fmt_ver);
	set_msg_id(cdata->buf, msg_id);
	set_msg_id(cdata->client_buf, msg_id);

	if (msm_rpm_debug_mask
	    & (MSM_RPM_LOG_REQUEST_PRETTY | MSM_RPM_LOG_REQUEST_RAW))
		msm_rpm_log_request(cdata);

	if (standalone) {
		for (i = 0; (i < cdata->write_idx); i++)
			cdata->kvp[i].valid = false;

		set_data_len(cdata->client_buf, 0);
		ret = msg_id;
		return ret;
	}

	msm_rpm_add_wait_list(msg_id, noack);

	ret = msm_rpm_send_buffer(&cdata->buf[0], msg_size, noirq);

	if (ret == msg_size) {
		for (i = 0; (i < cdata->write_idx); i++)
			cdata->kvp[i].valid = false;
		set_data_len(cdata->client_buf, 0);
		ret = msg_id;
		trace_rpm_smd_send_active_set(msg_id,
			get_rsc_type(cdata->client_buf),
			get_rsc_id(cdata->client_buf));
	} else if (ret < msg_size) {
		struct msm_rpm_wait_data *rc;

		ret = 0;
		pr_err("Failed to write data msg_size:%d ret:%d msg_id:%d\n",
				msg_size, ret, msg_id);
		rc = msm_rpm_get_entry_from_msg_id(msg_id);
		if (rc)
			msm_rpm_free_list_entry(rc);
	}
	return ret;
}

static int _msm_rpm_send_request(struct msm_rpm_request *handle, bool noack)
{
	int ret;
	static DEFINE_MUTEX(send_mtx);

	mutex_lock(&send_mtx);
	ret = msm_rpm_send_data(handle, MSM_RPM_MSG_REQUEST_TYPE, false, noack);
	mutex_unlock(&send_mtx);

	return ret;
}

int msm_rpm_send_request(struct msm_rpm_request *handle)
{
	return _msm_rpm_send_request(handle, false);
}
EXPORT_SYMBOL(msm_rpm_send_request);

int msm_rpm_send_request_noirq(struct msm_rpm_request *handle)
{
	return msm_rpm_send_data(handle, MSM_RPM_MSG_REQUEST_TYPE, true, false);
}
EXPORT_SYMBOL(msm_rpm_send_request_noirq);

void *msm_rpm_send_request_noack(struct msm_rpm_request *handle)
{
	int ret;

	ret = _msm_rpm_send_request(handle, true);

	return ret < 0 ? ERR_PTR(ret) : NULL;
}
EXPORT_SYMBOL(msm_rpm_send_request_noack);

int msm_rpm_wait_for_ack(uint32_t msg_id)
{
	struct msm_rpm_wait_data *elem;
	int rc = 0;

	if (!msg_id) {
		pr_err("Invalid msg id\n");
		return -ENOMEM;
	}

	if (msg_id == 1)
		return rc;

	if (standalone)
		return rc;

	elem = msm_rpm_get_entry_from_msg_id(msg_id);
	if (!elem)
		return rc;

	wait_for_completion(&elem->ack);
	trace_rpm_smd_ack_recvd(0, msg_id, 0xDEADFEED);

	rc = elem->errno;
	msm_rpm_free_list_entry(elem);

	return rc;
}
EXPORT_SYMBOL(msm_rpm_wait_for_ack);

static void msm_rpm_smd_read_data_noirq(uint32_t msg_id)
{
	uint32_t id = 0;

	while (id != msg_id) {
		if (smd_is_pkt_avail(msm_rpm_data.ch_info)) {
			int errno;
			char buf[MAX_ERR_BUFFER_SIZE] = {};

			msm_rpm_read_smd_data(buf);
			id = msm_rpm_get_msg_id_from_ack(buf);
			errno = msm_rpm_get_error_from_ack(buf);
			trace_rpm_smd_ack_recvd(1, msg_id, errno);
			msm_rpm_process_ack(id, errno);
		}
	}
}

static void msm_rpm_glink_read_data_noirq(struct msm_rpm_wait_data *elem)
{
	int ret;

	/* Use rx_poll method to read the message from RPM */
	while (elem->errno) {
		ret = glink_rpm_rx_poll(glink_data->glink_handle);
		if (ret >= 0) {
			/*
			 * We might have receieve the notification.
			 * Now we have to check whether the notification
			 * received is what we are interested?
			 * Wait for few usec to get the notification
			 * before re-trying the poll again.
			 */
			udelay(50);
		} else {
			pr_err("rx poll return error = %d\n", ret);
		}
	}
}

int msm_rpm_wait_for_ack_noirq(uint32_t msg_id)
{
	struct msm_rpm_wait_data *elem;
	unsigned long flags;
	int rc = 0;

	if (!msg_id)  {
		pr_err("Invalid msg id\n");
		return -ENOMEM;
	}

	if (msg_id == 1)
		return 0;

	if (standalone)
		return 0;

	spin_lock_irqsave(&msm_rpm_data.smd_lock_read, flags);

	elem = msm_rpm_get_entry_from_msg_id(msg_id);

	if (!elem)
		/* Should this be a bug
		 * Is it ok for another thread to read the msg?
		 */
		goto wait_ack_cleanup;

	if (elem->errno != INIT_ERROR) {
		rc = elem->errno;
		msm_rpm_free_list_entry(elem);
		goto wait_ack_cleanup;
	}

	if (!glink_enabled)
		msm_rpm_smd_read_data_noirq(msg_id);
	else
		msm_rpm_glink_read_data_noirq(elem);

	rc = elem->errno;

	msm_rpm_free_list_entry(elem);
wait_ack_cleanup:
	spin_unlock_irqrestore(&msm_rpm_data.smd_lock_read, flags);

	if (!glink_enabled)
		if (smd_is_pkt_avail(msm_rpm_data.ch_info))
			tasklet_schedule(&data_tasklet);
	return rc;
}
EXPORT_SYMBOL(msm_rpm_wait_for_ack_noirq);

void *msm_rpm_send_message_noack(enum msm_rpm_set set, uint32_t rsc_type,
		uint32_t rsc_id, struct msm_rpm_kvp *kvp, int nelems)
{
	int i, rc;
	struct msm_rpm_request *req =
		msm_rpm_create_request_common(set, rsc_type, rsc_id, nelems,
			       false);

	if (IS_ERR(req))
		return req;

	if (!req)
		return ERR_PTR(ENOMEM);

	for (i = 0; i < nelems; i++) {
		rc = msm_rpm_add_kvp_data(req, kvp[i].key,
				kvp[i].data, kvp[i].length);
		if (rc)
			goto bail;
	}

	rc = PTR_ERR(msm_rpm_send_request_noack(req));
bail:
	msm_rpm_free_request(req);
	return rc < 0 ? ERR_PTR(rc) : NULL;
}
EXPORT_SYMBOL(msm_rpm_send_message_noack);

int msm_rpm_send_message(enum msm_rpm_set set, uint32_t rsc_type,
		uint32_t rsc_id, struct msm_rpm_kvp *kvp, int nelems)
{
	int i, rc;
	struct msm_rpm_request *req =
		msm_rpm_create_request(set, rsc_type, rsc_id, nelems);

	if (IS_ERR(req))
		return PTR_ERR(req);

	if (!req)
		return -ENOMEM;

	for (i = 0; i < nelems; i++) {
		rc = msm_rpm_add_kvp_data(req, kvp[i].key,
				kvp[i].data, kvp[i].length);
		if (rc)
			goto bail;
	}

	rc = msm_rpm_wait_for_ack(msm_rpm_send_request(req));
bail:
	msm_rpm_free_request(req);
	return rc;
}
EXPORT_SYMBOL(msm_rpm_send_message);

int msm_rpm_send_message_noirq(enum msm_rpm_set set, uint32_t rsc_type,
		uint32_t rsc_id, struct msm_rpm_kvp *kvp, int nelems)
{
	int i, rc;
	struct msm_rpm_request *req =
		msm_rpm_create_request_noirq(set, rsc_type, rsc_id, nelems);

	if (IS_ERR(req))
		return PTR_ERR(req);

	if (!req)
		return -ENOMEM;

	for (i = 0; i < nelems; i++) {
		rc = msm_rpm_add_kvp_data_noirq(req, kvp[i].key,
					kvp[i].data, kvp[i].length);
		if (rc)
			goto bail;
	}

	rc = msm_rpm_wait_for_ack_noirq(msm_rpm_send_request_noirq(req));
bail:
	msm_rpm_free_request(req);
	return rc;
}
EXPORT_SYMBOL(msm_rpm_send_message_noirq);

/**
 * During power collapse, the rpm driver disables the SMD interrupts to make
 * sure that the interrupt doesn't wakes us from sleep.
 */
int msm_rpm_enter_sleep(bool print, const struct cpumask *cpumask)
{
	int ret = 0;

	if (standalone)
		return 0;

	if (!glink_enabled)
		ret = smd_mask_receive_interrupt(msm_rpm_data.ch_info,
								true, cpumask);
	else
		ret = glink_rpm_mask_rx_interrupt(glink_data->glink_handle,
							true, (void *)cpumask);

	if (!ret) {
		ret = msm_rpm_flush_requests(print);

		if (ret) {
			if (!glink_enabled)
				smd_mask_receive_interrupt(
					msm_rpm_data.ch_info, false, NULL);
			else
				glink_rpm_mask_rx_interrupt(
					glink_data->glink_handle, false, NULL);
		}
	}
	return ret;
}
EXPORT_SYMBOL(msm_rpm_enter_sleep);

/**
 * When the system resumes from power collapse, the SMD interrupt disabled by
 * enter function has to reenabled to continue processing SMD message.
 */
void msm_rpm_exit_sleep(void)
{
	int ret;

	if (standalone)
		return;

	do  {
		ret =  msm_rpm_read_sleep_ack();
	} while (ret > 0);

	if (!glink_enabled)
		smd_mask_receive_interrupt(msm_rpm_data.ch_info, false, NULL);
	else
		glink_rpm_mask_rx_interrupt(glink_data->glink_handle,
								false, NULL);
}
EXPORT_SYMBOL(msm_rpm_exit_sleep);

/*
 * Whenever there is a data from RPM, notify_rx will be called.
 * This function is invoked either interrupt OR polling context.
 */
static void msm_rpm_trans_notify_rx(void *handle, const void *priv,
			const void *pkt_priv, const void *ptr, size_t size)
{
	uint32_t msg_id;
	int errno;
	char buf[MAX_ERR_BUFFER_SIZE] = {0};
	struct msm_rpm_wait_data *elem;
	static DEFINE_SPINLOCK(rx_notify_lock);
	unsigned long flags;

	if (!size)
		return;

	WARN_ON(size > MAX_ERR_BUFFER_SIZE);

	spin_lock_irqsave(&rx_notify_lock, flags);
	memcpy(buf, ptr, size);
	msg_id = msm_rpm_get_msg_id_from_ack(buf);
	errno = msm_rpm_get_error_from_ack(buf);
	elem = msm_rpm_get_entry_from_msg_id(msg_id);

	/*
	 * It is applicable for sleep set requests
	 * Sleep set requests are not added to the
	 * wait queue list. Without this check we
	 * run into NULL pointer deferrence issue.
	 */
	if (!elem) {
		spin_unlock_irqrestore(&rx_notify_lock, flags);
		glink_rx_done(handle, ptr, 0);
		return;
	}

	msm_rpm_process_ack(msg_id, errno);
	spin_unlock_irqrestore(&rx_notify_lock, flags);

	glink_rx_done(handle, ptr, 0);
}

static void msm_rpm_trans_notify_state(void *handle, const void *priv,
				   unsigned int event)
{
	switch (event) {
	case GLINK_CONNECTED:
		glink_data->glink_handle = handle;

		if (IS_ERR_OR_NULL(glink_data->glink_handle)) {
			pr_err("glink_handle %d\n",
					(int)PTR_ERR(glink_data->glink_handle));
			WARN_ON(1);
		}

		/*
		 * Do not allow clients to send data to RPM until glink
		 * is fully open.
		 */
		probe_status = 0;
		pr_info("glink config params: transport=%s, edge=%s, name=%s\n",
			glink_data->xprt,
			glink_data->edge,
			glink_data->name);
		break;
	default:
		pr_err("Unrecognized event %d\n", event);
		break;
	};
}

static void msm_rpm_trans_notify_tx_done(void *handle, const void *priv,
					const void *pkt_priv, const void *ptr)
{
}

static void msm_rpm_glink_open_work(struct work_struct *work)
{
	pr_debug("Opening glink channel\n");
	glink_data->glink_handle = glink_open(glink_data->open_cfg);

	if (IS_ERR_OR_NULL(glink_data->glink_handle)) {
		pr_err("Error: glink_open failed %d\n",
				(int)PTR_ERR(glink_data->glink_handle));
		WARN_ON(1);
	}
}

static void msm_rpm_glink_notifier_cb(struct glink_link_state_cb_info *cb_info,
					void *priv)
{
	struct glink_open_config *open_config;
	static bool first = true;

	if (!cb_info) {
		pr_err("Missing callback data\n");
		return;
	}

	switch (cb_info->link_state) {
	case GLINK_LINK_STATE_UP:
		if (first)
			first = false;
		else
			break;
		open_config = kzalloc(sizeof(*open_config), GFP_KERNEL);
		if (!open_config) {
			pr_err("Could not allocate memory\n");
			break;
		}

		glink_data->open_cfg = open_config;
		pr_debug("glink link state up cb receieved\n");
		INIT_WORK(&glink_data->work, msm_rpm_glink_open_work);

		open_config->priv = glink_data;
		open_config->name = glink_data->name;
		open_config->edge = glink_data->edge;
		open_config->notify_rx = msm_rpm_trans_notify_rx;
		open_config->notify_tx_done = msm_rpm_trans_notify_tx_done;
		open_config->notify_state = msm_rpm_trans_notify_state;
		schedule_work(&glink_data->work);
		break;
	default:
		pr_err("Unrecognised state = %d\n", cb_info->link_state);
		break;
	};
}

static int msm_rpm_glink_dt_parse(struct platform_device *pdev,
				struct glink_apps_rpm_data *glink_data)
{
	char *key = NULL;
	int ret;

	if (of_device_is_compatible(pdev->dev.of_node, "qcom,rpm-glink")) {
		glink_enabled = true;
	} else {
		pr_warn("qcom,rpm-glink compatible not matches\n");
		ret = -EINVAL;
		return ret;
	}

	key = "qcom,glink-edge";
	ret = of_property_read_string(pdev->dev.of_node, key,
							&glink_data->edge);
	if (ret) {
		pr_err("Failed to read node: %s, key=%s\n",
			pdev->dev.of_node->full_name, key);
		return ret;
	}

	key = "rpm-channel-name";
	ret = of_property_read_string(pdev->dev.of_node, key,
							&glink_data->name);
	if (ret)
		pr_err("%s(): Failed to read node: %s, key=%s\n", __func__,
			pdev->dev.of_node->full_name, key);

	return ret;
}

static int msm_rpm_glink_link_setup(struct glink_apps_rpm_data *glink_data,
						struct platform_device *pdev)
{
	struct glink_link_info *link_info;
	void *link_state_cb_handle;
	struct device *dev = &pdev->dev;
	int ret = 0;

	link_info = devm_kzalloc(dev, sizeof(struct glink_link_info),
								GFP_KERNEL);
	if (!link_info) {
		ret = -ENOMEM;
		return ret;
	}

	glink_data->link_info = link_info;

	/*
	 * Setup link info parameters
	 */
	link_info->edge = glink_data->edge;
	link_info->glink_link_state_notif_cb =
					msm_rpm_glink_notifier_cb;
	link_state_cb_handle = glink_register_link_state_cb(link_info, NULL);
	if (IS_ERR_OR_NULL(link_state_cb_handle)) {
		pr_err("Could not register cb\n");
		ret = PTR_ERR(link_state_cb_handle);
		return ret;
	}

	spin_lock_init(&msm_rpm_data.smd_lock_read);
	spin_lock_init(&msm_rpm_data.smd_lock_write);

	return ret;
}

static int msm_rpm_dev_glink_probe(struct platform_device *pdev)
{
	int ret = -ENOMEM;
	struct device *dev = &pdev->dev;

	glink_data = devm_kzalloc(dev, sizeof(*glink_data), GFP_KERNEL);
	if (!glink_data)
		return ret;

	ret = msm_rpm_glink_dt_parse(pdev, glink_data);
	if (ret < 0) {
		devm_kfree(dev, glink_data);
		return ret;
	}

	ret = msm_rpm_glink_link_setup(glink_data, pdev);
	if (ret < 0) {
		/*
		 * If the glink setup fails there is no
		 * fall back mechanism to SMD.
		 */
		pr_err("GLINK setup fail ret = %d\n", ret);
		WARN_ON(1);
	}

	return ret;
}

static int msm_rpm_dev_probe(struct platform_device *pdev)
{
	char *key = NULL;
	int ret = 0;
	void __iomem *reg_base;
	uint32_t version = V0_PROTOCOL_VERSION; /* set to default v0 format */

	/*
	 * Check for standalone support
	 */
	key = "rpm-standalone";
	standalone = of_property_read_bool(pdev->dev.of_node, key);
	if (standalone) {
		probe_status = ret;
		goto skip_init;
	}

	reg_base = of_iomap(pdev->dev.of_node, 0);

	if (reg_base) {
		version = readq_relaxed(reg_base);
		iounmap(reg_base);
	}

	if (version == V1_PROTOCOL_VERSION)
		rpm_msg_fmt_ver = RPM_MSG_V1_FMT;

	pr_debug("RPM-SMD running version %d/n", rpm_msg_fmt_ver);

	ret = msm_rpm_dev_glink_probe(pdev);
	if (!ret) {
		pr_info("APSS-RPM communication over GLINK\n");
		msm_rpm_send_buffer = msm_rpm_glink_send_buffer;
		of_platform_populate(pdev->dev.of_node, NULL, NULL,
							&pdev->dev);
		return ret;
	}
	msm_rpm_send_buffer = msm_rpm_send_smd_buffer;

	key = "rpm-channel-name";
	ret = of_property_read_string(pdev->dev.of_node, key,
					&msm_rpm_data.ch_name);
	if (ret) {
		pr_err("%s(): Failed to read node: %s, key=%s\n", __func__,
			pdev->dev.of_node->full_name, key);
		goto fail;
	}

	key = "rpm-channel-type";
	ret = of_property_read_u32(pdev->dev.of_node, key,
					&msm_rpm_data.ch_type);
	if (ret) {
		pr_err("%s(): Failed to read node: %s, key=%s\n", __func__,
			pdev->dev.of_node->full_name, key);
		goto fail;
	}

	ret = smd_named_open_on_edge(msm_rpm_data.ch_name,
				msm_rpm_data.ch_type,
				&msm_rpm_data.ch_info,
				&msm_rpm_data,
				msm_rpm_notify);
	if (ret) {
		if (ret != -EPROBE_DEFER) {
			pr_err("%s: Cannot open RPM channel %s %d\n",
				__func__, msm_rpm_data.ch_name,
				msm_rpm_data.ch_type);
		}
		goto fail;
	}

	spin_lock_init(&msm_rpm_data.smd_lock_write);
	spin_lock_init(&msm_rpm_data.smd_lock_read);
	tasklet_init(&data_tasklet, data_fn_tasklet, 0);

	wait_for_completion(&msm_rpm_data.smd_open);

	smd_disable_read_intr(msm_rpm_data.ch_info);

	probe_status = ret;
skip_init:
	of_platform_populate(pdev->dev.of_node, NULL, NULL, &pdev->dev);

	if (standalone)
		pr_info("RPM running in standalone mode\n");
fail:
	return probe_status;
}

static const struct of_device_id msm_rpm_match_table[] =  {
	{.compatible = "qcom,rpm-smd"},
	{.compatible = "qcom,rpm-glink"},
	{},
};

static struct platform_driver msm_rpm_device_driver = {
	.probe = msm_rpm_dev_probe,
	.driver = {
		.name = "rpm-smd",
		.owner = THIS_MODULE,
		.suppress_bind_attrs = true,
		.of_match_table = msm_rpm_match_table,
	},
};

int __init msm_rpm_driver_init(void)
{
	static bool registered;

	if (registered)
		return 0;
	registered = true;

	return platform_driver_register(&msm_rpm_device_driver);
}
EXPORT_SYMBOL(msm_rpm_driver_init);
arch_initcall(msm_rpm_driver_init);
