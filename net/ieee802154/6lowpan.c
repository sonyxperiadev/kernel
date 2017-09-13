/*
 * Copyright 2011, Siemens AG
 * written by Alexander Smirnov <alex.bluesman.smirnov@gmail.com>
 */

/*
 * Based on patches from Jon Smirl <jonsmirl@gmail.com>
 * Copyright (c) 2011 Jon Smirl <jonsmirl@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/* Jon's code is based on 6lowpan implementation for Contiki which is:
 * Copyright (c) 2008, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <linux/bitops.h>
#include <linux/if_arp.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/netdevice.h>
#include <net/af_ieee802154.h>
#include <net/ieee802154.h>
#include <net/ieee802154_netdev.h>
#include <net/ipv6.h>

#include "6lowpan.h"

/* TTL uncompression values */
static const u8 lowpan_ttl_values[] = {0, 1, 64, 255};

static LIST_HEAD(lowpan_devices);

/*
 * Uncompression of linklocal:
 *   0 -> 16 bytes from packet
 *   1 -> 2  bytes from prefix - bunch of zeroes and 8 from packet
 *   2 -> 2  bytes from prefix - zeroes + 2 from packet
 *   3 -> 2  bytes from prefix - infer 8 bytes from lladdr
 *
 *  NOTE: => the uncompress function does change 0xf to 0x10
 *  NOTE: 0x00 => no-autoconfig => unspecified
 */
static const u8 lowpan_unc_llconf[] = {0x0f, 0x28, 0x22, 0x20};

/*
 * Uncompression of ctx-based:
 *   0 -> 0 bits  from packet [unspecified / reserved]
 *   1 -> 8 bytes from prefix - bunch of zeroes and 8 from packet
 *   2 -> 8 bytes from prefix - zeroes + 2 from packet
 *   3 -> 8 bytes from prefix - infer 8 bytes from lladdr
 */
static const u8 lowpan_unc_ctxconf[] = {0x00, 0x88, 0x82, 0x80};

/*
 * Uncompression of ctx-base
 *   0 -> 0 bits from packet
 *   1 -> 2 bytes from prefix - bunch of zeroes 5 from packet
 *   2 -> 2 bytes from prefix - zeroes + 3 from packet
 *   3 -> 2 bytes from prefix - infer 1 bytes from lladdr
 */
static const u8 lowpan_unc_mxconf[] = {0x0f, 0x25, 0x23, 0x21};

/* Link local prefix */
static const u8 lowpan_llprefix[] = {0xfe, 0x80};

/* private device info */
struct lowpan_dev_info {
	struct net_device	*real_dev; /* real WPAN device ptr */
	struct mutex		dev_list_mtx; /* mutex for list ops */
	unsigned short		fragment_tag;
};

struct lowpan_dev_record {
	struct net_device *ldev;
	struct list_head list;
};

struct lowpan_fragment {
	struct sk_buff		*skb;		/* skb to be assembled */
	u16			length;		/* length to be assemled */
	u32			bytes_rcv;	/* bytes received */
	u16			tag;		/* current fragment tag */
	struct timer_list	timer;		/* assembling timer */
	struct list_head	list;		/* fragments list */
};

static LIST_HEAD(lowpan_fragments);
static DEFINE_SPINLOCK(flist_lock);

static inline struct
lowpan_dev_info *lowpan_dev_info(const struct net_device *dev)
{
	return netdev_priv(dev);
}

static inline void lowpan_address_flip(u8 *src, u8 *dest)
{
	int i;
	for (i = 0; i < IEEE802154_ADDR_LEN; i++)
		(dest)[IEEE802154_ADDR_LEN - i - 1] = (src)[i];
}

/* list of all 6lowpan devices, uses for package delivering */
/* print data in line */
static inline void lowpan_raw_dump_inline(const char *caller, char *msg,
				   unsigned char *buf, int len)
{
#ifdef DEBUG
	if (msg)
		pr_debug("(%s) %s: ", caller, msg);
	print_hex_dump(KERN_DEBUG, "", DUMP_PREFIX_NONE,
		       16, 1, buf, len, false);
#endif /* DEBUG */
}

/*
 * print data in a table format:
 *
 * addr: xx xx xx xx xx xx
 * addr: xx xx xx xx xx xx
 * ...
 */
static inline void lowpan_raw_dump_table(const char *caller, char *msg,
				   unsigned char *buf, int len)
{
#ifdef DEBUG
	if (msg)
		pr_debug("(%s) %s:\n", caller, msg);
	print_hex_dump(KERN_DEBUG, "\t", DUMP_PREFIX_OFFSET,
		       16, 1, buf, len, false);
#endif /* DEBUG */
}

static u8
lowpan_compress_addr_64(u8 **hc06_ptr, u8 shift, const struct in6_addr *ipaddr,
		 const unsigned char *lladdr)
{
	u8 val = 0;

	if (is_addr_mac_addr_based(ipaddr, lladdr))
		val = 3; /* 0-bits */
	else if (lowpan_is_iid_16_bit_compressable(ipaddr)) {
		/* compress IID to 16 bits xxxx::XXXX */
		memcpy(*hc06_ptr, &ipaddr->s6_addr16[7], 2);
		*hc06_ptr += 2;
		val = 2; /* 16-bits */
	} else {
		/* do not compress IID => xxxx::IID */
		memcpy(*hc06_ptr, &ipaddr->s6_addr16[4], 8);
		*hc06_ptr += 8;
		val = 1; /* 64-bits */
	}

	return rol8(val, shift);
}

static void
lowpan_uip_ds6_set_addr_iid(struct in6_addr *ipaddr, unsigned char *lladdr)
{
	memcpy(&ipaddr->s6_addr[8], lladdr, IEEE802154_ADDR_LEN);
	/* second bit-flip (Universe/Local) is done according RFC2464 */
	ipaddr->s6_addr[8] ^= 0x02;
}

/*
 * Uncompress addresses based on a prefix and a postfix with zeroes in
 * between. If the postfix is zero in length it will use the link address
 * to configure the IP address (autoconf style).
 * pref_post_count takes a byte where the first nibble specify prefix count
 * and the second postfix count (NOTE: 15/0xf => 16 bytes copy).
 */
static int
lowpan_uncompress_addr(struct sk_buff *skb, struct in6_addr *ipaddr,
	u8 const *prefix, u8 pref_post_count, unsigned char *lladdr)
{
	u8 prefcount = pref_post_count >> 4;
	u8 postcount = pref_post_count & 0x0f;

	/* full nibble 15 => 16 */
	prefcount = (prefcount == 15 ? 16 : prefcount);
	postcount = (postcount == 15 ? 16 : postcount);

	if (lladdr)
		lowpan_raw_dump_inline(__func__, "linklocal address",
						lladdr,	IEEE802154_ADDR_LEN);
	if (prefcount > 0)
		memcpy(ipaddr, prefix, prefcount);

	if (prefcount + postcount < 16)
		memset(&ipaddr->s6_addr[prefcount], 0,
					16 - (prefcount + postcount));

	if (postcount > 0) {
		memcpy(&ipaddr->s6_addr[16 - postcount], skb->data, postcount);
		skb_pull(skb, postcount);
	} else if (prefcount > 0) {
		if (lladdr == NULL)
			return -EINVAL;

		/* no IID based configuration if no prefix and no data */
		lowpan_uip_ds6_set_addr_iid(ipaddr, lladdr);
	}

	pr_debug("uncompressing %d + %d => ", prefcount, postcount);
	lowpan_raw_dump_inline(NULL, NULL, ipaddr->s6_addr, 16);

	return 0;
}

static void
lowpan_compress_udp_header(u8 **hc06_ptr, struct sk_buff *skb)
{
	struct udphdr *uh = udp_hdr(skb);

	if (((uh->source & LOWPAN_NHC_UDP_4BIT_MASK) ==
				LOWPAN_NHC_UDP_4BIT_PORT) &&
	    ((uh->dest & LOWPAN_NHC_UDP_4BIT_MASK) ==
				LOWPAN_NHC_UDP_4BIT_PORT)) {
		pr_debug("UDP header: both ports compression to 4 bits\n");
		**hc06_ptr = LOWPAN_NHC_UDP_CS_P_11;
		**(hc06_ptr + 1) = /* subtraction is faster */
		   (u8)((uh->dest - LOWPAN_NHC_UDP_4BIT_PORT) +
		       ((uh->source & LOWPAN_NHC_UDP_4BIT_PORT) << 4));
		*hc06_ptr += 2;
	} else if ((uh->dest & LOWPAN_NHC_UDP_8BIT_MASK) ==
			LOWPAN_NHC_UDP_8BIT_PORT) {
		pr_debug("UDP header: remove 8 bits of dest\n");
		**hc06_ptr = LOWPAN_NHC_UDP_CS_P_01;
		memcpy(*hc06_ptr + 1, &uh->source, 2);
		**(hc06_ptr + 3) = (u8)(uh->dest - LOWPAN_NHC_UDP_8BIT_PORT);
		*hc06_ptr += 4;
	} else if ((uh->source & LOWPAN_NHC_UDP_8BIT_MASK) ==
			LOWPAN_NHC_UDP_8BIT_PORT) {
		pr_debug("UDP header: remove 8 bits of source\n");
		**hc06_ptr = LOWPAN_NHC_UDP_CS_P_10;
		memcpy(*hc06_ptr + 1, &uh->dest, 2);
		**(hc06_ptr + 3) = (u8)(uh->source - LOWPAN_NHC_UDP_8BIT_PORT);
		*hc06_ptr += 4;
	} else {
		pr_debug("UDP header: can't compress\n");
		**hc06_ptr = LOWPAN_NHC_UDP_CS_P_00;
		memcpy(*hc06_ptr + 1, &uh->source, 2);
		memcpy(*hc06_ptr + 3, &uh->dest, 2);
		*hc06_ptr += 5;
	}

	/* checksum is always inline */
	memcpy(*hc06_ptr, &uh->check, 2);
	*hc06_ptr += 2;

	/* skip the UDP header */
	skb_pull(skb, sizeof(struct udphdr));
}

static inline int lowpan_fetch_skb_u8(struct sk_buff *skb, u8 *val)
{
	if (unlikely(!pskb_may_pull(skb, 1)))
		return -EINVAL;

	*val = skb->data[0];
	skb_pull(skb, 1);

	return 0;
}

static inline int lowpan_fetch_skb_u16(struct sk_buff *skb, u16 *val)
{
	if (unlikely(!pskb_may_pull(skb, 2)))
		return -EINVAL;

	*val = (skb->data[0] << 8) | skb->data[1];
	skb_pull(skb, 2);

	return 0;
}

static int
lowpan_uncompress_udp_header(struct sk_buff *skb, struct udphdr *uh)
{
	u8 tmp;

	if (!uh)
		goto err;

	if (lowpan_fetch_skb_u8(skb, &tmp))
		goto err;

	if ((tmp & LOWPAN_NHC_UDP_MASK) == LOWPAN_NHC_UDP_ID) {
		pr_debug("UDP header uncompression\n");
		switch (tmp & LOWPAN_NHC_UDP_CS_P_11) {
		case LOWPAN_NHC_UDP_CS_P_00:
			memcpy(&uh->source, &skb->data[0], 2);
			memcpy(&uh->dest, &skb->data[2], 2);
			skb_pull(skb, 4);
			break;
		case LOWPAN_NHC_UDP_CS_P_01:
			memcpy(&uh->source, &skb->data[0], 2);
			uh->dest =
			   skb->data[2] + LOWPAN_NHC_UDP_8BIT_PORT;
			skb_pull(skb, 3);
			break;
		case LOWPAN_NHC_UDP_CS_P_10:
			uh->source = skb->data[0] + LOWPAN_NHC_UDP_8BIT_PORT;
			memcpy(&uh->dest, &skb->data[1], 2);
			skb_pull(skb, 3);
			break;
		case LOWPAN_NHC_UDP_CS_P_11:
			uh->source =
			   LOWPAN_NHC_UDP_4BIT_PORT + (skb->data[0] >> 4);
			uh->dest =
			   LOWPAN_NHC_UDP_4BIT_PORT + (skb->data[0] & 0x0f);
			skb_pull(skb, 1);
			break;
		default:
			pr_debug("ERROR: unknown UDP format\n");
			goto err;
			break;
		}

		pr_debug("uncompressed UDP ports: src = %d, dst = %d\n",
			 uh->source, uh->dest);

		/* copy checksum */
		memcpy(&uh->check, &skb->data[0], 2);
		skb_pull(skb, 2);

		/*
		 * UDP lenght needs to be infered from the lower layers
		 * here, we obtain the hint from the remaining size of the
		 * frame
		 */
		uh->len = htons(skb->len + sizeof(struct udphdr));
		pr_debug("uncompressed UDP length: src = %d", uh->len);
	} else {
		pr_debug("ERROR: unsupported NH format\n");
		goto err;
	}

	return 0;
err:
	return -EINVAL;
}

static int lowpan_header_create(struct sk_buff *skb,
			   struct net_device *dev,
			   unsigned short type, const void *_daddr,
			   const void *_saddr, unsigned int len)
{
	u8 tmp, iphc0, iphc1, *hc06_ptr;
	struct ipv6hdr *hdr;
	const u8 *saddr = _saddr;
	const u8 *daddr = _daddr;
	u8 head[100];
	struct ieee802154_addr sa, da;

	/* TODO:
	 * if this package isn't ipv6 one, where should it be routed?
	 */
	if (type != ETH_P_IPV6)
		return 0;

	hdr = ipv6_hdr(skb);
	hc06_ptr = head + 2;

	pr_debug("IPv6 header dump:\n\tversion = %d\n\tlength  = %d\n"
		 "\tnexthdr = 0x%02x\n\thop_lim = %d\n", hdr->version,
		 ntohs(hdr->payload_len), hdr->nexthdr, hdr->hop_limit);

	lowpan_raw_dump_table(__func__, "raw skb network header dump",
		skb_network_header(skb), sizeof(struct ipv6hdr));

	if (!saddr)
		saddr = dev->dev_addr;

	lowpan_raw_dump_inline(__func__, "saddr", (unsigned char *)saddr, 8);

	/*
	 * As we copy some bit-length fields, in the IPHC encoding bytes,
	 * we sometimes use |=
	 * If the field is 0, and the current bit value in memory is 1,
	 * this does not work. We therefore reset the IPHC encoding here
	 */
	iphc0 = LOWPAN_DISPATCH_IPHC;
	iphc1 = 0;

	/* TODO: context lookup */

	lowpan_raw_dump_inline(__func__, "daddr", (unsigned char *)daddr, 8);

	/*
	 * Traffic class, flow label
	 * If flow label is 0, compress it. If traffic class is 0, compress it
	 * We have to process both in the same time as the offset of traffic
	 * class depends on the presence of version and flow label
	 */

	/* hc06 format of TC is ECN | DSCP , original one is DSCP | ECN */
	tmp = (hdr->priority << 4) | (hdr->flow_lbl[0] >> 4);
	tmp = ((tmp & 0x03) << 6) | (tmp >> 2);

	if (((hdr->flow_lbl[0] & 0x0F) == 0) &&
	     (hdr->flow_lbl[1] == 0) && (hdr->flow_lbl[2] == 0)) {
		/* flow label can be compressed */
		iphc0 |= LOWPAN_IPHC_FL_C;
		if ((hdr->priority == 0) &&
		   ((hdr->flow_lbl[0] & 0xF0) == 0)) {
			/* compress (elide) all */
			iphc0 |= LOWPAN_IPHC_TC_C;
		} else {
			/* compress only the flow label */
			*hc06_ptr = tmp;
			hc06_ptr += 1;
		}
	} else {
		/* Flow label cannot be compressed */
		if ((hdr->priority == 0) &&
		   ((hdr->flow_lbl[0] & 0xF0) == 0)) {
			/* compress only traffic class */
			iphc0 |= LOWPAN_IPHC_TC_C;
			*hc06_ptr = (tmp & 0xc0) | (hdr->flow_lbl[0] & 0x0F);
			memcpy(hc06_ptr + 1, &hdr->flow_lbl[1], 2);
			hc06_ptr += 3;
		} else {
			/* compress nothing */
			memcpy(hc06_ptr, hdr, 4);
			/* replace the top byte with new ECN | DSCP format */
			*hc06_ptr = tmp;
			hc06_ptr += 4;
		}
	}

	/* NOTE: payload length is always compressed */

	/* Next Header is compress if UDP */
	if (hdr->nexthdr == UIP_PROTO_UDP)
		iphc0 |= LOWPAN_IPHC_NH_C;

	if ((iphc0 & LOWPAN_IPHC_NH_C) == 0) {
		*hc06_ptr = hdr->nexthdr;
		hc06_ptr += 1;
	}

	/*
	 * Hop limit
	 * if 1:   compress, encoding is 01
	 * if 64:  compress, encoding is 10
	 * if 255: compress, encoding is 11
	 * else do not compress
	 */
	switch (hdr->hop_limit) {
	case 1:
		iphc0 |= LOWPAN_IPHC_TTL_1;
		break;
	case 64:
		iphc0 |= LOWPAN_IPHC_TTL_64;
		break;
	case 255:
		iphc0 |= LOWPAN_IPHC_TTL_255;
		break;
	default:
		*hc06_ptr = hdr->hop_limit;
		hc06_ptr += 1;
		break;
	}

	/* source address compression */
	if (is_addr_unspecified(&hdr->saddr)) {
		pr_debug("source address is unspecified, setting SAC\n");
		iphc1 |= LOWPAN_IPHC_SAC;
	/* TODO: context lookup */
	} else if (is_addr_link_local(&hdr->saddr)) {
		pr_debug("source address is link-local\n");
		iphc1 |= lowpan_compress_addr_64(&hc06_ptr,
				LOWPAN_IPHC_SAM_BIT, &hdr->saddr, saddr);
	} else {
		pr_debug("send the full source address\n");
		memcpy(hc06_ptr, &hdr->saddr.s6_addr16[0], 16);
		hc06_ptr += 16;
	}

	/* destination address compression */
	if (is_addr_mcast(&hdr->daddr)) {
		pr_debug("destination address is multicast: ");
		iphc1 |= LOWPAN_IPHC_M;
		if (lowpan_is_mcast_addr_compressable8(&hdr->daddr)) {
			pr_debug("compressed to 1 octet\n");
			iphc1 |= LOWPAN_IPHC_DAM_11;
			/* use last byte */
			*hc06_ptr = hdr->daddr.s6_addr[15];
			hc06_ptr += 1;
		} else if (lowpan_is_mcast_addr_compressable32(&hdr->daddr)) {
			pr_debug("compressed to 4 octets\n");
			iphc1 |= LOWPAN_IPHC_DAM_10;
			/* second byte + the last three */
			*hc06_ptr = hdr->daddr.s6_addr[1];
			memcpy(hc06_ptr + 1, &hdr->daddr.s6_addr[13], 3);
			hc06_ptr += 4;
		} else if (lowpan_is_mcast_addr_compressable48(&hdr->daddr)) {
			pr_debug("compressed to 6 octets\n");
			iphc1 |= LOWPAN_IPHC_DAM_01;
			/* second byte + the last five */
			*hc06_ptr = hdr->daddr.s6_addr[1];
			memcpy(hc06_ptr + 1, &hdr->daddr.s6_addr[11], 5);
			hc06_ptr += 6;
		} else {
			pr_debug("using full address\n");
			iphc1 |= LOWPAN_IPHC_DAM_00;
			memcpy(hc06_ptr, &hdr->daddr.s6_addr[0], 16);
			hc06_ptr += 16;
		}
	} else {
		/* TODO: context lookup */
		if (is_addr_link_local(&hdr->daddr)) {
			pr_debug("dest address is unicast and link-local\n");
			iphc1 |= lowpan_compress_addr_64(&hc06_ptr,
				LOWPAN_IPHC_DAM_BIT, &hdr->daddr, daddr);
		} else {
			pr_debug("dest address is unicast: using full one\n");
			memcpy(hc06_ptr, &hdr->daddr.s6_addr16[0], 16);
			hc06_ptr += 16;
		}
	}

	/* UDP header compression */
	if (hdr->nexthdr == UIP_PROTO_UDP)
		lowpan_compress_udp_header(&hc06_ptr, skb);

	head[0] = iphc0;
	head[1] = iphc1;

	skb_pull(skb, sizeof(struct ipv6hdr));
	memcpy(skb_push(skb, hc06_ptr - head), head, hc06_ptr - head);

	lowpan_raw_dump_table(__func__, "raw skb data dump", skb->data,
				skb->len);

	/*
	 * NOTE1: I'm still unsure about the fact that compression and WPAN
	 * header are created here and not later in the xmit. So wait for
	 * an opinion of net maintainers.
	 */
	/*
	 * NOTE2: to be absolutely correct, we must derive PANid information
	 * from MAC subif of the 'dev' and 'real_dev' network devices, but
	 * this isn't implemented in mainline yet, so currently we assign 0xff
	 */
	{
		mac_cb(skb)->flags = IEEE802154_FC_TYPE_DATA;
		mac_cb(skb)->seq = ieee802154_mlme_ops(dev)->get_dsn(dev);

		/* prepare wpan address data */
		sa.addr_type = IEEE802154_ADDR_LONG;
		sa.pan_id = ieee802154_mlme_ops(dev)->get_pan_id(dev);

		memcpy(&(sa.hwaddr), saddr, 8);
		/* intra-PAN communications */
		da.pan_id = ieee802154_mlme_ops(dev)->get_pan_id(dev);

		/*
		 * if the destination address is the broadcast address, use the
		 * corresponding short address
		 */
		if (lowpan_is_addr_broadcast(daddr)) {
			da.addr_type = IEEE802154_ADDR_SHORT;
			da.short_addr = IEEE802154_ADDR_BROADCAST;
		} else {
			da.addr_type = IEEE802154_ADDR_LONG;
			memcpy(&(da.hwaddr), daddr, IEEE802154_ADDR_LEN);

			/* request acknowledgment */
			mac_cb(skb)->flags |= MAC_CB_FLAG_ACKREQ;
		}

		return dev_hard_header(skb, lowpan_dev_info(dev)->real_dev,
				type, (void *)&da, (void *)&sa, skb->len);
	}
}

static int lowpan_give_skb_to_devices(struct sk_buff *skb)
{
	struct lowpan_dev_record *entry;
	struct sk_buff *skb_cp;
	int stat = NET_RX_SUCCESS;

	rcu_read_lock();
	list_for_each_entry_rcu(entry, &lowpan_devices, list)
		if (lowpan_dev_info(entry->ldev)->real_dev == skb->dev) {
			skb_cp = skb_copy(skb, GFP_ATOMIC);
			if (!skb_cp) {
				stat = -ENOMEM;
				break;
			}

			skb_cp->dev = entry->ldev;
			stat = netif_rx(skb_cp);
		}
	rcu_read_unlock();

	return stat;
}

static int lowpan_skb_deliver(struct sk_buff *skb, struct ipv6hdr *hdr)
{
	struct sk_buff *new;
	int stat = NET_RX_SUCCESS;

	new = skb_copy_expand(skb, sizeof(struct ipv6hdr), skb_tailroom(skb),
								GFP_ATOMIC);
	kfree_skb(skb);

	if (!new)
		return -ENOMEM;

	skb_push(new, sizeof(struct ipv6hdr));
	skb_reset_network_header(new);
	skb_copy_to_linear_data(new, hdr, sizeof(struct ipv6hdr));

	new->protocol = htons(ETH_P_IPV6);
	new->pkt_type = PACKET_HOST;

	stat = lowpan_give_skb_to_devices(new);

	kfree_skb(new);

	return stat;
}

static void lowpan_fragment_timer_expired(unsigned long entry_addr)
{
	struct lowpan_fragment *entry = (struct lowpan_fragment *)entry_addr;

	pr_debug("timer expired for frame with tag %d\n", entry->tag);

	list_del(&entry->list);
	dev_kfree_skb(entry->skb);
	kfree(entry);
}

static struct lowpan_fragment *
lowpan_alloc_new_frame(struct sk_buff *skb, u16 len, u16 tag)
{
	struct lowpan_fragment *frame;

	frame = kzalloc(sizeof(struct lowpan_fragment),
			GFP_ATOMIC);
	if (!frame)
		goto frame_err;

	INIT_LIST_HEAD(&frame->list);

	frame->length = len;
	frame->tag = tag;

	/* allocate buffer for frame assembling */
	frame->skb = netdev_alloc_skb_ip_align(skb->dev, frame->length +
					       sizeof(struct ipv6hdr));

	if (!frame->skb)
		goto skb_err;

	frame->skb->priority = skb->priority;
	frame->skb->dev = skb->dev;

	/* reserve headroom for uncompressed ipv6 header */
	skb_reserve(frame->skb, sizeof(struct ipv6hdr));
	skb_put(frame->skb, frame->length);

	init_timer(&frame->timer);
	/* time out is the same as for ipv6 - 60 sec */
	frame->timer.expires = jiffies + LOWPAN_FRAG_TIMEOUT;
	frame->timer.data = (unsigned long)frame;
	frame->timer.function = lowpan_fragment_timer_expired;

	add_timer(&frame->timer);

	list_add_tail(&frame->list, &lowpan_fragments);

	return frame;

skb_err:
	kfree(frame);
frame_err:
	return NULL;
}

static int
lowpan_process_data(struct sk_buff *skb)
{
	struct ipv6hdr hdr;
	u8 tmp, iphc0, iphc1, num_context = 0;
	u8 *_saddr, *_daddr;
	int err;

	lowpan_raw_dump_table(__func__, "raw skb data dump", skb->data,
				skb->len);
	/* at least two bytes will be used for the encoding */
	if (skb->len < 2)
		goto drop;

	if (lowpan_fetch_skb_u8(skb, &iphc0))
		goto drop;

	/* fragments assembling */
	switch (iphc0 & LOWPAN_DISPATCH_MASK) {
	case LOWPAN_DISPATCH_FRAG1:
	case LOWPAN_DISPATCH_FRAGN:
	{
		struct lowpan_fragment *frame;
		/* slen stores the rightmost 8 bits of the 11 bits length */
		u8 slen, offset = 0;
		u16 len, tag;
		bool found = false;

		if (lowpan_fetch_skb_u8(skb, &slen) || /* frame length */
		    lowpan_fetch_skb_u16(skb, &tag))  /* fragment tag */
			goto drop;

		/* adds the 3 MSB to the 8 LSB to retrieve the 11 bits length */
		len = ((iphc0 & 7) << 8) | slen;

		if ((iphc0 & LOWPAN_DISPATCH_MASK) == LOWPAN_DISPATCH_FRAG1) {
			pr_debug("%s received a FRAG1 packet (tag: %d, "
				 "size of the entire IP packet: %d)",
				 __func__, tag, len);
		} else { /* FRAGN */
			if (lowpan_fetch_skb_u8(skb, &offset))
				goto unlock_and_drop;
			pr_debug("%s received a FRAGN packet (tag: %d, "
				 "size of the entire IP packet: %d, "
				 "offset: %d)", __func__, tag, len, offset * 8);
		}

		/*
		 * check if frame assembling with the same tag is
		 * already in progress
		 */
		spin_lock_bh(&flist_lock);

		list_for_each_entry(frame, &lowpan_fragments, list)
			if (frame->tag == tag) {
				found = true;
				break;
			}

		/* alloc new frame structure */
		if (!found) {
			pr_debug("%s first fragment received for tag %d, "
				 "begin packet reassembly", __func__, tag);
			frame = lowpan_alloc_new_frame(skb, len, tag);
			if (!frame)
				goto unlock_and_drop;
		}

		/* if payload fits buffer, copy it */
		if (likely((offset * 8 + skb->len) <= frame->length))
			skb_copy_to_linear_data_offset(frame->skb, offset * 8,
							skb->data, skb->len);
		else
			goto unlock_and_drop;

		frame->bytes_rcv += skb->len;

		/* frame assembling complete */
		if ((frame->bytes_rcv == frame->length) &&
		     frame->timer.expires > jiffies) {
			/* if timer haven't expired - first of all delete it */
			del_timer_sync(&frame->timer);
			list_del(&frame->list);
			spin_unlock_bh(&flist_lock);

			pr_debug("%s successfully reassembled fragment "
				 "(tag %d)", __func__, tag);

			dev_kfree_skb(skb);
			skb = frame->skb;
			kfree(frame);

			if (lowpan_fetch_skb_u8(skb, &iphc0))
				goto drop;

			break;
		}
		spin_unlock_bh(&flist_lock);

		return kfree_skb(skb), 0;
	}
	default:
		break;
	}

	if (lowpan_fetch_skb_u8(skb, &iphc1))
		goto drop;

	_saddr = mac_cb(skb)->sa.hwaddr;
	_daddr = mac_cb(skb)->da.hwaddr;

	pr_debug("iphc0 = %02x, iphc1 = %02x\n", iphc0, iphc1);

	/* another if the CID flag is set */
	if (iphc1 & LOWPAN_IPHC_CID) {
		pr_debug("CID flag is set, increase header with one\n");
		if (lowpan_fetch_skb_u8(skb, &num_context))
			goto drop;
	}

	hdr.version = 6;

	/* Traffic Class and Flow Label */
	switch ((iphc0 & LOWPAN_IPHC_TF) >> 3) {
	/*
	 * Traffic Class and FLow Label carried in-line
	 * ECN + DSCP + 4-bit Pad + Flow Label (4 bytes)
	 */
	case 0: /* 00b */
		if (lowpan_fetch_skb_u8(skb, &tmp))
			goto drop;

		memcpy(&hdr.flow_lbl, &skb->data[0], 3);
		skb_pull(skb, 3);
		hdr.priority = ((tmp >> 2) & 0x0f);
		hdr.flow_lbl[0] = ((tmp >> 2) & 0x30) | (tmp << 6) |
					(hdr.flow_lbl[0] & 0x0f);
		break;
	/*
	 * Traffic class carried in-line
	 * ECN + DSCP (1 byte), Flow Label is elided
	 */
	case 2: /* 10b */
		if (lowpan_fetch_skb_u8(skb, &tmp))
			goto drop;

		hdr.priority = ((tmp >> 2) & 0x0f);
		hdr.flow_lbl[0] = ((tmp << 6) & 0xC0) | ((tmp >> 2) & 0x30);
		hdr.flow_lbl[1] = 0;
		hdr.flow_lbl[2] = 0;
		break;
	/*
	 * Flow Label carried in-line
	 * ECN + 2-bit Pad + Flow Label (3 bytes), DSCP is elided
	 */
	case 1: /* 01b */
		if (lowpan_fetch_skb_u8(skb, &tmp))
			goto drop;

		hdr.flow_lbl[0] = (skb->data[0] & 0x0F) | ((tmp >> 2) & 0x30);
		memcpy(&hdr.flow_lbl[1], &skb->data[0], 2);
		skb_pull(skb, 2);
		break;
	/* Traffic Class and Flow Label are elided */
	case 3: /* 11b */
		hdr.priority = 0;
		hdr.flow_lbl[0] = 0;
		hdr.flow_lbl[1] = 0;
		hdr.flow_lbl[2] = 0;
		break;
	default:
		break;
	}

	/* Next Header */
	if ((iphc0 & LOWPAN_IPHC_NH_C) == 0) {
		/* Next header is carried inline */
		if (lowpan_fetch_skb_u8(skb, &(hdr.nexthdr)))
			goto drop;

		pr_debug("NH flag is set, next header carried inline: %02x\n",
			 hdr.nexthdr);
	}

	/* Hop Limit */
	if ((iphc0 & 0x03) != LOWPAN_IPHC_TTL_I)
		hdr.hop_limit = lowpan_ttl_values[iphc0 & 0x03];
	else {
		if (lowpan_fetch_skb_u8(skb, &(hdr.hop_limit)))
			goto drop;
	}

	/* Extract SAM to the tmp variable */
	tmp = ((iphc1 & LOWPAN_IPHC_SAM) >> LOWPAN_IPHC_SAM_BIT) & 0x03;

	/* Source address uncompression */
	pr_debug("source address stateless compression\n");
	err = lowpan_uncompress_addr(skb, &hdr.saddr, lowpan_llprefix,
				lowpan_unc_llconf[tmp], skb->data);
	if (err)
		goto drop;

	/* Extract DAM to the tmp variable */
	tmp = ((iphc1 & LOWPAN_IPHC_DAM_11) >> LOWPAN_IPHC_DAM_BIT) & 0x03;

	/* check for Multicast Compression */
	if (iphc1 & LOWPAN_IPHC_M) {
		if (iphc1 & LOWPAN_IPHC_DAC) {
			pr_debug("dest: context-based mcast compression\n");
			/* TODO: implement this */
		} else {
			u8 prefix[] = {0xff, 0x02};

			pr_debug("dest: non context-based mcast compression\n");
			if (0 < tmp && tmp < 3) {
				if (lowpan_fetch_skb_u8(skb, &prefix[1]))
					goto drop;
			}

			err = lowpan_uncompress_addr(skb, &hdr.daddr, prefix,
					lowpan_unc_mxconf[tmp], NULL);
			if (err)
				goto drop;
		}
	} else {
		pr_debug("dest: stateless compression\n");
		err = lowpan_uncompress_addr(skb, &hdr.daddr, lowpan_llprefix,
				lowpan_unc_llconf[tmp], skb->data);
		if (err)
			goto drop;
	}

	/* UDP data uncompression */
	if (iphc0 & LOWPAN_IPHC_NH_C) {
		struct udphdr uh;
		struct sk_buff *new;
		if (lowpan_uncompress_udp_header(skb, &uh))
			goto drop;

		/*
		 * replace the compressed UDP head by the uncompressed UDP
		 * header
		 */
		new = skb_copy_expand(skb, sizeof(struct udphdr),
				      skb_tailroom(skb), GFP_ATOMIC);
		kfree_skb(skb);

		if (!new)
			return -ENOMEM;

		skb = new;

		skb_push(skb, sizeof(struct udphdr));
		skb_reset_transport_header(skb);
		skb_copy_to_linear_data(skb, &uh, sizeof(struct udphdr));

		lowpan_raw_dump_table(__func__, "raw UDP header dump",
				      (u8 *)&uh, sizeof(uh));

		hdr.nexthdr = UIP_PROTO_UDP;
	}

	/* Not fragmented package */
	hdr.payload_len = htons(skb->len);

	pr_debug("skb headroom size = %d, data length = %d\n",
		 skb_headroom(skb), skb->len);

	pr_debug("IPv6 header dump:\n\tversion = %d\n\tlength  = %d\n\t"
		 "nexthdr = 0x%02x\n\thop_lim = %d\n", hdr.version,
		 ntohs(hdr.payload_len), hdr.nexthdr, hdr.hop_limit);

	lowpan_raw_dump_table(__func__, "raw header dump", (u8 *)&hdr,
							sizeof(hdr));
	return lowpan_skb_deliver(skb, &hdr);

unlock_and_drop:
	spin_unlock_bh(&flist_lock);
drop:
	kfree_skb(skb);
	return -EINVAL;
}

static int lowpan_set_address(struct net_device *dev, void *p)
{
	struct sockaddr *sa = p;

	if (netif_running(dev))
		return -EBUSY;

	/* TODO: validate addr */
	memcpy(dev->dev_addr, sa->sa_data, dev->addr_len);

	return 0;
}

static int lowpan_get_mac_header_length(struct sk_buff *skb)
{
	/*
	 * Currently long addressing mode is supported only, so the overall
	 * header size is 21:
	 * FC SeqNum DPAN DA  SA  Sec
	 * 2  +  1  +  2 + 8 + 8 + 0  = 21
	 */
	return 21;
}

static int
lowpan_fragment_xmit(struct sk_buff *skb, u8 *head,
			int mlen, int plen, int offset, int type)
{
	struct sk_buff *frag;
	int hlen, ret;

	hlen = (type == LOWPAN_DISPATCH_FRAG1) ?
			LOWPAN_FRAG1_HEAD_SIZE : LOWPAN_FRAGN_HEAD_SIZE;

	lowpan_raw_dump_inline(__func__, "6lowpan fragment header", head, hlen);

	frag = dev_alloc_skb(hlen + mlen + plen + IEEE802154_MFR_SIZE);
	if (!frag)
		return -ENOMEM;

	frag->priority = skb->priority;
	frag->dev = skb->dev;

	/* copy header, MFR and payload */
	memcpy(skb_put(frag, mlen), skb->data, mlen);
	memcpy(skb_put(frag, hlen), head, hlen);

	if (plen)
		skb_copy_from_linear_data_offset(skb, offset + mlen,
					skb_put(frag, plen), plen);

	lowpan_raw_dump_table(__func__, " raw fragment dump", frag->data,
								frag->len);

	ret = dev_queue_xmit(frag);

	return ret;
}

static int
lowpan_skb_fragmentation(struct sk_buff *skb, struct net_device *dev)
{
	int  err, header_length, payload_length, tag, offset = 0;
	u8 head[5];

	header_length = lowpan_get_mac_header_length(skb);
	payload_length = skb->len - header_length;
	tag = lowpan_dev_info(dev)->fragment_tag++;

	/* first fragment header */
	head[0] = LOWPAN_DISPATCH_FRAG1 | ((payload_length >> 8) & 0x7);
	head[1] = payload_length & 0xff;
	head[2] = tag >> 8;
	head[3] = tag & 0xff;

	err = lowpan_fragment_xmit(skb, head, header_length, LOWPAN_FRAG_SIZE,
				   0, LOWPAN_DISPATCH_FRAG1);

	if (err) {
		pr_debug("%s unable to send FRAG1 packet (tag: %d)",
			 __func__, tag);
		goto exit;
	}

	offset = LOWPAN_FRAG_SIZE;

	/* next fragment header */
	head[0] &= ~LOWPAN_DISPATCH_FRAG1;
	head[0] |= LOWPAN_DISPATCH_FRAGN;

	while ((payload_length - offset > 0) && (err >= 0)) {
		int len = LOWPAN_FRAG_SIZE;

		head[4] = offset / 8;

		if (payload_length - offset < len)
			len = payload_length - offset;

		err = lowpan_fragment_xmit(skb, head, header_length,
					   len, offset, LOWPAN_DISPATCH_FRAGN);
		if (err) {
			pr_debug("%s unable to send a subsequent FRAGN packet "
				 "(tag: %d, offset: %d", __func__, tag, offset);
			goto exit;
		}

		offset += len;
	}

exit:
	return err;
}

static netdev_tx_t lowpan_xmit(struct sk_buff *skb, struct net_device *dev)
{
	int err = -1;

	pr_debug("package xmit\n");

	skb->dev = lowpan_dev_info(dev)->real_dev;
	if (skb->dev == NULL) {
		pr_debug("ERROR: no real wpan device found\n");
		goto error;
	}

	/* Send directly if less than the MTU minus the 2 checksum bytes. */
	if (skb->len <= IEEE802154_MTU - IEEE802154_MFR_SIZE) {
		err = dev_queue_xmit(skb);
		goto out;
	}

	pr_debug("frame is too big, fragmentation is needed\n");
	err = lowpan_skb_fragmentation(skb, dev);
error:
	dev_kfree_skb(skb);
out:
	if (err)
		pr_debug("ERROR: xmit failed\n");

	return (err < 0) ? NET_XMIT_DROP : err;
}

static struct wpan_phy *lowpan_get_phy(const struct net_device *dev)
{
	struct net_device *real_dev = lowpan_dev_info(dev)->real_dev;
	return ieee802154_mlme_ops(real_dev)->get_phy(real_dev);
}

static u16 lowpan_get_pan_id(const struct net_device *dev)
{
	struct net_device *real_dev = lowpan_dev_info(dev)->real_dev;
	return ieee802154_mlme_ops(real_dev)->get_pan_id(real_dev);
}

static u16 lowpan_get_short_addr(const struct net_device *dev)
{
	struct net_device *real_dev = lowpan_dev_info(dev)->real_dev;
	return ieee802154_mlme_ops(real_dev)->get_short_addr(real_dev);
}

static u8 lowpan_get_dsn(const struct net_device *dev)
{
	struct net_device *real_dev = lowpan_dev_info(dev)->real_dev;
	return ieee802154_mlme_ops(real_dev)->get_dsn(real_dev);
}

static struct header_ops lowpan_header_ops = {
	.create	= lowpan_header_create,
};

static struct lock_class_key lowpan_tx_busylock;
static struct lock_class_key lowpan_netdev_xmit_lock_key;

static void lowpan_set_lockdep_class_one(struct net_device *dev,
					 struct netdev_queue *txq,
					 void *_unused)
{
	lockdep_set_class(&txq->_xmit_lock,
			  &lowpan_netdev_xmit_lock_key);
}


static int lowpan_dev_init(struct net_device *dev)
{
	netdev_for_each_tx_queue(dev, lowpan_set_lockdep_class_one, NULL);
	dev->qdisc_tx_busylock = &lowpan_tx_busylock;
	return 0;
}

static const struct net_device_ops lowpan_netdev_ops = {
	.ndo_init		= lowpan_dev_init,
	.ndo_start_xmit		= lowpan_xmit,
	.ndo_set_mac_address	= lowpan_set_address,
};

static struct ieee802154_mlme_ops lowpan_mlme = {
	.get_pan_id = lowpan_get_pan_id,
	.get_phy = lowpan_get_phy,
	.get_short_addr = lowpan_get_short_addr,
	.get_dsn = lowpan_get_dsn,
};

static void lowpan_setup(struct net_device *dev)
{
	dev->addr_len		= IEEE802154_ADDR_LEN;
	memset(dev->broadcast, 0xff, IEEE802154_ADDR_LEN);
	dev->type		= ARPHRD_IEEE802154;
	/* Frame Control + Sequence Number + Address fields + Security Header */
	dev->hard_header_len	= 2 + 1 + 20 + 14;
	dev->needed_tailroom	= 2; /* FCS */
	dev->mtu		= 1281;
	dev->tx_queue_len	= 0;
	dev->flags		= IFF_BROADCAST | IFF_MULTICAST;
	dev->watchdog_timeo	= 0;

	dev->netdev_ops		= &lowpan_netdev_ops;
	dev->header_ops		= &lowpan_header_ops;
	dev->ml_priv		= &lowpan_mlme;
	dev->destructor		= free_netdev;
}

static int lowpan_validate(struct nlattr *tb[], struct nlattr *data[])
{
	if (tb[IFLA_ADDRESS]) {
		if (nla_len(tb[IFLA_ADDRESS]) != IEEE802154_ADDR_LEN)
			return -EINVAL;
	}
	return 0;
}

static int lowpan_rcv(struct sk_buff *skb, struct net_device *dev,
	struct packet_type *pt, struct net_device *orig_dev)
{
	struct sk_buff *local_skb;

	if (!netif_running(dev))
		goto drop;

	if (dev->type != ARPHRD_IEEE802154)
		goto drop;

	/* check that it's our buffer */
	if (skb->data[0] == LOWPAN_DISPATCH_IPV6) {
		/* Copy the packet so that the IPv6 header is
		 * properly aligned.
		 */
		local_skb = skb_copy_expand(skb, NET_SKB_PAD - 1,
					    skb_tailroom(skb), GFP_ATOMIC);
		if (!local_skb)
			goto drop;

		local_skb->protocol = htons(ETH_P_IPV6);
		local_skb->pkt_type = PACKET_HOST;

		/* Pull off the 1-byte of 6lowpan header. */
		skb_pull(local_skb, 1);
		skb_reset_network_header(local_skb);
		skb_set_transport_header(local_skb, sizeof(struct ipv6hdr));

		lowpan_give_skb_to_devices(local_skb);

		kfree_skb(local_skb);
		kfree_skb(skb);
	} else {
		switch (skb->data[0] & 0xe0) {
		case LOWPAN_DISPATCH_IPHC:	/* ipv6 datagram */
		case LOWPAN_DISPATCH_FRAG1:	/* first fragment header */
		case LOWPAN_DISPATCH_FRAGN:	/* next fragments headers */
			local_skb = skb_clone(skb, GFP_ATOMIC);
			if (!local_skb)
				goto drop;
			lowpan_process_data(local_skb);

			kfree_skb(skb);
			break;
		default:
			break;
		}
	}

	return NET_RX_SUCCESS;

drop:
	kfree_skb(skb);
	return NET_RX_DROP;
}

static int lowpan_newlink(struct net *src_net, struct net_device *dev,
			  struct nlattr *tb[], struct nlattr *data[])
{
	struct net_device *real_dev;
	struct lowpan_dev_record *entry;

	pr_debug("adding new link\n");

	if (!tb[IFLA_LINK])
		return -EINVAL;
	/* find and hold real wpan device */
	real_dev = dev_get_by_index(src_net, nla_get_u32(tb[IFLA_LINK]));
	if (!real_dev)
		return -ENODEV;

	lowpan_dev_info(dev)->real_dev = real_dev;
	lowpan_dev_info(dev)->fragment_tag = 0;
	mutex_init(&lowpan_dev_info(dev)->dev_list_mtx);

	entry = kzalloc(sizeof(struct lowpan_dev_record), GFP_KERNEL);
	if (!entry) {
		dev_put(real_dev);
		lowpan_dev_info(dev)->real_dev = NULL;
		return -ENOMEM;
	}

	entry->ldev = dev;

	mutex_lock(&lowpan_dev_info(dev)->dev_list_mtx);
	INIT_LIST_HEAD(&entry->list);
	list_add_tail(&entry->list, &lowpan_devices);
	mutex_unlock(&lowpan_dev_info(dev)->dev_list_mtx);

	register_netdevice(dev);

	return 0;
}

static void lowpan_dellink(struct net_device *dev, struct list_head *head)
{
	struct lowpan_dev_info *lowpan_dev = lowpan_dev_info(dev);
	struct net_device *real_dev = lowpan_dev->real_dev;
	struct lowpan_dev_record *entry, *tmp;

	ASSERT_RTNL();

	mutex_lock(&lowpan_dev_info(dev)->dev_list_mtx);
	list_for_each_entry_safe(entry, tmp, &lowpan_devices, list) {
		if (entry->ldev == dev) {
			list_del(&entry->list);
			kfree(entry);
		}
	}
	mutex_unlock(&lowpan_dev_info(dev)->dev_list_mtx);

	mutex_destroy(&lowpan_dev_info(dev)->dev_list_mtx);

	unregister_netdevice_queue(dev, head);

	dev_put(real_dev);
}

static struct rtnl_link_ops lowpan_link_ops __read_mostly = {
	.kind		= "lowpan",
	.priv_size	= sizeof(struct lowpan_dev_info),
	.setup		= lowpan_setup,
	.newlink	= lowpan_newlink,
	.dellink	= lowpan_dellink,
	.validate	= lowpan_validate,
};

static inline int __init lowpan_netlink_init(void)
{
	return rtnl_link_register(&lowpan_link_ops);
}

static inline void lowpan_netlink_fini(void)
{
	rtnl_link_unregister(&lowpan_link_ops);
}

static int lowpan_device_event(struct notifier_block *unused,
				unsigned long event,
				void *ptr)
{
	struct net_device *dev = ptr;
	LIST_HEAD(del_list);
	struct lowpan_dev_record *entry, *tmp;

	if (dev->type != ARPHRD_IEEE802154)
		goto out;

	if (event == NETDEV_UNREGISTER) {
		list_for_each_entry_safe(entry, tmp, &lowpan_devices, list) {
			if (lowpan_dev_info(entry->ldev)->real_dev == dev)
				lowpan_dellink(entry->ldev, &del_list);
		}

		unregister_netdevice_many(&del_list);
	}

out:
	return NOTIFY_DONE;
}

static struct notifier_block lowpan_dev_notifier = {
	.notifier_call = lowpan_device_event,
};

static struct packet_type lowpan_packet_type = {
	.type = __constant_htons(ETH_P_IEEE802154),
	.func = lowpan_rcv,
};

static int __init lowpan_init_module(void)
{
	int err = 0;

	err = lowpan_netlink_init();
	if (err < 0)
		goto out;

	dev_add_pack(&lowpan_packet_type);

	err = register_netdevice_notifier(&lowpan_dev_notifier);
	if (err < 0) {
		dev_remove_pack(&lowpan_packet_type);
		lowpan_netlink_fini();
	}
out:
	return err;
}

static void __exit lowpan_cleanup_module(void)
{
	struct lowpan_fragment *frame, *tframe;

	lowpan_netlink_fini();

	dev_remove_pack(&lowpan_packet_type);

	unregister_netdevice_notifier(&lowpan_dev_notifier);

	/* Now 6lowpan packet_type is removed, so no new fragments are
	 * expected on RX, therefore that's the time to clean incomplete
	 * fragments.
	 */
	spin_lock_bh(&flist_lock);
	list_for_each_entry_safe(frame, tframe, &lowpan_fragments, list) {
		del_timer_sync(&frame->timer);
		list_del(&frame->list);
		dev_kfree_skb(frame->skb);
		kfree(frame);
	}
	spin_unlock_bh(&flist_lock);
}

module_init(lowpan_init_module);
module_exit(lowpan_cleanup_module);
MODULE_LICENSE("GPL");
MODULE_ALIAS_RTNL_LINK("lowpan");
