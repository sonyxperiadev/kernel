/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.


 *  Copyright (C) 2009-2014 Broadcom Corporation
 */


/*******************************************************************************
 *
 *  Filename:      brcm_ldisc_sh.h
 *
 *  Description:   Shared Transport Header file
 *   To be included by the protocol stack drivers for
 *   Broadcom BT,FM and GPS combo chip drivers
 *
 ******************************************************************************/

#ifndef LDISC_SH_H
#define LDISC_SH_H

#include <linux/skbuff.h>
#define CONFIG_BT_HCIUART_BRCM

/*******************************************************************************
**  Constants
*******************************************************************************/
/* install sysfs entry values */
#define V4L2_STATUS_ERR '2'  // error occured in BT application (HCI command timeout or HW error)
#define V4L2_STATUS_ON  '1'  // Atleast one procol driver is registered
#define V4L2_STATUS_OFF '0'  // No procol drivers registered

/* BT err flag values (bt_err) */
#define  V4L2_ERR_FLAG_RESET '0'
#define  V4L2_ERR_FLAG_SET   '1'

/*
 * enum proto-type - The protocol on chips which share a
 *  common physical interface like UART.
 */
enum proto_type {
    PROTO_SH_BT,
    PROTO_SH_FM,
    PROTO_SH_GPS,
    PROTO_SH_ANT,
    PROTO_SH_MAX,
};

/*
 * enum sleep-type - Sleep control is board specific
 */
enum sleep_type {
    SLEEP_DEFAULT,
    SLEEP_BLUESLEEP,
};


void brcm_btsleep_wake( enum sleep_type type);
void brcm_btsleep_start(enum sleep_type type);
void brcm_btsleep_stop(enum sleep_type type);


#define sh_ldisc_cb(skb) ((struct sh_ldisc_skb_cb *)((skb)->cb))

/*******************************************************************************
**  Type definitions
*******************************************************************************/

/*
 * Skb helpers
 */
struct sh_ldisc_skb_cb {
    __u8 pkt_type;
    __u32 lparam;
};

/**
 * struct sh_proto_s - Per Protocol structure from BT/FM/GPS to shared ldisc
 * @type: type of the protocol being registered among the
 *  available proto_type(BT, FM, GPS the protocol which share TTY).
 * @recv: the receiver callback pointing to a function in the
 *  protocol drivers called by the shared ldisc driver upon receiving
 *  relevant data.
 * @match_packet: reserved for future use, to make ST more generic
 * @reg_complete_cb: callback handler pointing to a function in protocol
 *  handler called by shared ldisc when the pending registrations are complete.
 *  The registrations are marked pending, in situations when fw
 *  download is in progress.
 * @write: pointer to function in shared ldisc provided to protocol drivers,
 *  to be made use when protocol drivers have data to send to TTY.
 * @priv_data: privdate data holder for the protocol drivers, sent
 *  from the protocol drivers during registration, and sent back on
 *  reg_complete_cb and recv.
 */
struct sh_proto_s {
    enum proto_type type;
    long (*recv) (void *, struct sk_buff *);
    unsigned char (*match_packet) (const unsigned char *data);
    void (*reg_complete_cb) (void *, char data);
    long (*write) (struct sk_buff *skb);
    void *priv_data;
};

/*******************************************************************************
**  Extern variables and functions
*******************************************************************************/

extern long brcm_sh_ldisc_register(struct sh_proto_s *);
extern long brcm_sh_ldisc_unregister(enum proto_type);

#endif /* LDISC_H */
