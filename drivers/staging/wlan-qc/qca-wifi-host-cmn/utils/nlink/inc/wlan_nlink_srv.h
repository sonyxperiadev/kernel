/*
 * Copyright (c) 2012-2017, 2019-2020 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/******************************************************************************
* wlan_nlink_srv.h
*
* wlan_nlink_srv is used to RX/TX Netlink messages from user space to kernel
* modules and vice versa. Kernel modules must register a message handler for a
* message type so that the wlan_nlink_srv can invoke the corresponding msg handler
* whenever a Netlink message of a particular type has been received from an
* application.  In the opposite direction, wlan_nlink_srv provides a mechanism
* which kernel modules can use to send Netlink messages to applications.
*
******************************************************************************/

#ifndef WLAN_NLINK_SRV_H
#define WLAN_NLINK_SRV_H

#include <linux/skbuff.h>
#include <net/netlink.h>
#include <wlan_nlink_common.h>

#define INVALID_PID -1
#define NLINK_MAX_CALLBACKS (WLAN_NL_MSG_MAX - WLAN_NL_MSG_BASE)

typedef int (*nl_srv_msg_callback)(struct sk_buff *skb);

/**
 * cld80211_oem_send_reply() - API to send cld80211 msg
 * @skb: Sk buffer
 * @hdr: nl80211hdr pointer
 * @nest: pointer of vendor nested attribute
 * @flags: Flags
 *
 * API to send cld80211 msg to applications
 *
 * Return: None
 */
void cld80211_oem_send_reply(struct sk_buff *msg, void *hdr,
				    struct nlattr *nest, int flags);

/**
 * nl80211hdr_put() - API to allocate skb for cld80211 msg
 * @hdr: nl80211hdr pointer
 * @portid: Port ID
 * @nest: pointer of vendor nested attribute
 * @flags: Flags
 *
 * API to allocate skb for cld80211 msg
 *
 * Return: Pointer to skbuff
 */

struct sk_buff *
cld80211_oem_rsp_alloc_skb(uint32_t portid, void **hdr, struct nlattr **nest,
			   int *flags);
int nl_srv_init(void *wiphy, int proto);
void nl_srv_exit(void);
int nl_srv_register(tWlanNlModTypes msg_type, nl_srv_msg_callback msg_handler);
int nl_srv_unregister(tWlanNlModTypes msg_type,
		      nl_srv_msg_callback msg_handler);

#ifdef CNSS_GENL
int nl_srv_ucast(struct sk_buff *skb, int dst_pid, int flag,
			int app_id, int mcgroup_id);
int nl_srv_bcast(struct sk_buff *skb, int mcgroup_id, int app_id);

/**
 * nl80211hdr_put() - API to fill genlmsg header
 * @skb: Sk buffer
 * @portid: Port ID
 * @seq: Sequence number
 * @flags: Flags
 * @cmd: Command id
 *
 * API to fill genl message header for brodcast events to user space
 *
 * Return: Pointer to user specific header/payload
 */
void *nl80211hdr_put(struct sk_buff *skb, uint32_t portid,
		     uint32_t seq, int flags, uint8_t cmd);
#else
int nl_srv_ucast(struct sk_buff *skb, int dst_pid, int flag);
int nl_srv_bcast(struct sk_buff *skb);
#endif

int nl_srv_is_initialized(void);
void nl_srv_ucast_oem(struct sk_buff *skb, int dst_pid, int flag);

#endif
