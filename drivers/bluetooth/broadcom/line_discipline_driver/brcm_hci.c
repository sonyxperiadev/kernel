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


/*****************************************************************************
*
*  Filename:      brcm_hci.c
*
*  Description:   Broadcom Bluetooth Low Power UART protocol
*
*****************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/init.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/poll.h>
#include <linux/timer.h>
/*#include <asm/gpio.h>*/

#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/signal.h>
#include <linux/ioctl.h>
#include <linux/skbuff.h>
#include <linux/serial_core.h>

#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h>

#include "brcm_hci_uart.h"
#include "../include/v4l2_target.h"
#include "../include/fm.h"
#include "../include/ant.h"
#include "../include/v4l2_logs.h"


/*****************************************************************************
**  Constants & Macros for dynamic logging
*****************************************************************************/
#ifndef BTLDISC_DEBUG
#define BTLDISC_DEBUG TRUE
#endif

/* set this module parameter to enable debug info */
extern int ldisc_dbg_param;
extern struct sock *nl_sk_hcisnoop;

#if (BTLDISC_DEBUG)
#define BRCM_HCI_DBG(flag, fmt, arg...) \
            do { \
                            if (ldisc_dbg_param & flag) \
                    printk(KERN_DEBUG "(brcmhci):%s:%d  "fmt"\n" , \
                                               __func__, __LINE__, ## arg); \
            } while(0)
#else
#define BRCM_HCI_DBG(flag,fmt, arg...)
#endif

#define BRCM_HCI_ERR(fmt, arg...)  printk(KERN_ERR "(brcmhci):%s:%d  "fmt"\n" , \
                                           __func__, __LINE__,## arg)


#if V4L2_SNOOP_ENABLE
/* parameter to enable HCI snooping */
extern int ldisc_snoop_enable_param;
#endif


/*****************************************************************************
**  Constants & Macros
*****************************************************************************/

/* HCIBRCM commands */
#define HCIBRCM_GO_TO_SLEEP_IND    0x30
#define HCIBRCM_GO_TO_SLEEP_ACK    0x31
#define HCIBRCM_WAKE_UP_IND    0x32
#define HCIBRCM_WAKE_UP_ACK    0x33

/* HCIBRCM receiver States */
#define HCIBRCM_W4_PACKET_TYPE 0
#define HCIBRCM_W4_EVENT_HDR   1
#define HCIBRCM_W4_ACL_HDR     2
#define HCIBRCM_W4_SCO_HDR     3
#define HCIBRCM_W4_DATA        4
#define FMBRCM_W4_EVENT_HDR    5
#define ANTBRCM_W4_EVENT_HDR   6

#define TIMER_PERIOD 100    /* 100 ms */
#define HOST_CONTROLLER_IDLE_TSH 4000  /* 4 s */

#define RESP_BUFF_SIZE 30

#define BT_WAKE  22


/* Message event ID passed from Host/Controller lib to stack */
#define MSG_HC_TO_STACK_HCI_ERR        0x1300 /* eq. BT_EVT_TO_BTU_HCIT_ERR */
#define MSG_HC_TO_STACK_HCI_ACL        0x1100 /* eq. BT_EVT_TO_BTU_HCI_ACL */
#define MSG_HC_TO_STACK_HCI_SCO        0x1200 /* eq. BT_EVT_TO_BTU_HCI_SCO */
#define MSG_HC_TO_STACK_HCI_EVT        0x1000 /* eq. BT_EVT_TO_BTU_HCI_EVT */
#define MSG_HC_TO_FM_HCI_EVT           0x3000 /* Response code for FM HCI event */
#define MSG_HC_TO_STACK_L2C_SEG_XMIT   0x1900 /*eq. BT_EVT_TO_BTU_L2C_SEG_XMIT*/

#undef CONFIG_SERIAL_MSM_HS

/* HCIBRCM states */
enum hcibrcm_states_e {
    HCIBRCM_ASLEEP,
    HCIBRCM_ASLEEP_TO_AWAKE,
    HCIBRCM_AWAKE,
    HCIBRCM_AWAKE_TO_ASLEEP
};

/* Function forward declarations */
static int brcm_open(struct hci_uart *hu);
static int brcm_close(struct hci_uart *hu);
static int brcm_recv(struct hci_uart *hu, void *data, int count);
static int brcm_recv_int(struct hci_uart *hu, void *data, int count);
static int brcm_enqueue(struct hci_uart *hu, struct sk_buff *skb);
static struct sk_buff *brcm_dequeue(struct hci_uart *hu);
static int brcm_flush(struct hci_uart *hu);

struct hci_uart_proto brcmp = {
    .id      = HCI_UART_BRCM,
    .open    = brcm_open,
    .close   = brcm_close,
    .recv    = brcm_recv,
    .recv_int= brcm_recv_int,
    .enqueue = brcm_enqueue,
    .dequeue = brcm_dequeue,
    .flush   = brcm_flush,
};


/* External functions */
int brcm_hci_snoop(struct hci_uart *hu, void* data, unsigned int count);

/*****************************************************************************
**  Static functions
*****************************************************************************/

/*
 * Function to check data length
 */
static inline int brcm_check_data_len(struct brcm_struct *brcm,
                    struct hci_uart *hu, int protoid,int len)
{
    register int room = skb_tailroom(brcm->rx_skb);

    BRCM_HCI_DBG(V4L2_DBG_RX, "len %d room %d", len, room);

    if (!len) {
       brcm_hci_uart_route_frame(protoid, hu, brcm->rx_skb);
    } else if (len > room) {
        BT_ERR("brcm_check_data_len Data length is too large kfree_skb %p",brcm->rx_skb);
        kfree_skb(brcm->rx_skb);
    } else {
        brcm->rx_state = HCIBRCM_W4_DATA;
        brcm->rx_count = len;
        return len;
    }

    brcm->rx_state = HCIBRCM_W4_PACKET_TYPE;
    brcm->rx_skb   = NULL;
    brcm->rx_count = 0;

    return 0;
}


void brcm_hci_process_frametype( register int frame_type,
                               struct hci_uart*hu,struct sk_buff *skb,int count)
{
    hc_bt_hdr *hci_hdr;

    hci_hdr = hu->hdr_data;

    hci_hdr->offset = 0;
    hci_hdr->layer_specific = 0;
    hci_hdr->len = skb->len;

    BRCM_HCI_DBG(V4L2_DBG_RX, "%s frame_type = 0x%x hci_hdr->len %d", __func__,
                                                       frame_type,hci_hdr->len);

    switch(frame_type)
    {
    case HCI_EVENT_PKT:
        hci_hdr->event = MSG_HC_TO_STACK_HCI_EVT;
        break;
    case HCI_ACLDATA_PKT:
        hci_hdr->event = MSG_HC_TO_STACK_HCI_ACL;
        break;
    case HCI_SCODATA_PKT:
        hci_hdr->event = MSG_HC_TO_STACK_HCI_SCO;
        break;
    default:
        BRCM_HCI_DBG(V4L2_DBG_RX, "%s received frame_type UNKNOWN", __func__);
        hci_hdr->event = MSG_HC_TO_STACK_HCI_ERR;
    };

    memcpy(skb_push(skb, sizeof(hc_bt_hdr)), hci_hdr,
                                                    sizeof(hc_bt_hdr) );
}


/*****************************************************************************
**   UART API calls - Start
*****************************************************************************/

/*****************************************************************************
**
** Function - brcm_open()
**
** Description - Initialize protocol
**
** Returns - 0 if success; else errno
**
*****************************************************************************/
static int brcm_open(struct hci_uart *hu)
{
    struct brcm_struct *brcm;

#if (!V4L2_SNOOP_ENABLE)
    hc_bt_hdr *hci_hdr;
    hci_hdr = kzalloc(sizeof(*hci_hdr), GFP_ATOMIC);

    if (!hci_hdr)
        return -ENOMEM;

    hu->hdr_data = hci_hdr;
#endif

    brcm = kzalloc(sizeof(*brcm), GFP_ATOMIC);
    if (!brcm)
        return -ENOMEM;

    BRCM_HCI_DBG(V4L2_DBG_INIT, "hu %p", hu);

    skb_queue_head_init(&brcm->txq);
    skb_queue_head_init(&brcm->tx_wait_q);
    spin_lock_init(&brcm->hcibrcm_lock);

    brcm->hcibrcm_state = HCIBRCM_AWAKE;
    brcm->is_there_activity = 0;
    hu->priv = brcm;

    BRCM_HCI_DBG(V4L2_DBG_INIT, "hu %p", hu);
    BRCM_HCI_DBG(V4L2_DBG_INIT, "sizeof(hu->hdr_data) = %d", sizeof(hu->hdr_data));
    return 0;
}

/*****************************************************************************
**
** Function - brcm_flush()
**
** Description - Flush protocol data
**
** Returns - 0 if success; else errno
**
*****************************************************************************/
static int brcm_flush(struct hci_uart *hu)
{
    struct brcm_struct *brcm = hu->priv;

    BRCM_HCI_DBG(V4L2_DBG_INIT, "hu %p", hu);

    if (!skb_queue_empty(&brcm->tx_wait_q))
        skb_queue_purge(&brcm->tx_wait_q);
    if (!skb_queue_empty(&brcm->txq))
        skb_queue_purge(&brcm->txq);

    return 0;
}

/*****************************************************************************
**
** Function - brcm_close()
**
** Description - Close protocol
**
** Returns - 0 if success; else errno
**
*****************************************************************************/
static int brcm_close(struct hci_uart *hu)
{
    struct brcm_struct *brcm = hu->priv;

    BRCM_HCI_DBG(V4L2_DBG_INIT, "hu %p", hu);

    if (!skb_queue_empty(&brcm->tx_wait_q))
        skb_queue_purge(&brcm->tx_wait_q);
    if (!skb_queue_empty(&brcm->txq))
        skb_queue_purge(&brcm->txq);

    if (brcm->rx_skb) {
        kfree_skb(brcm->rx_skb);
    }

    if(hu->hdr_data) {
        kfree(hu->hdr_data);
        hu->hdr_data = NULL;
    }
    hu->priv = NULL;
    kfree(brcm);

    return 0;
}

/*****************************************************************************
**
** Function - brcm_enqueue()
**
** Description - Enqueue frame for transmittion (padding, crc, etc).
**               May be cabrcmed from two simultaneous tasklets
**
** Returns - 0 if success; else errno
**
*****************************************************************************/
static int brcm_enqueue(struct hci_uart *hu, struct sk_buff *skb)
{
    struct brcm_struct *brcm = hu->priv;
    unsigned long lock_flags;
    BRCM_HCI_DBG(V4L2_DBG_TX, "hu %p skb %p", hu, skb);

    spin_lock_irqsave(&hu->lock, lock_flags);
    skb_queue_tail(&brcm->txq, skb);
    spin_unlock_irqrestore(&hu->lock, lock_flags);
    return 0;
}
/*****************************************************************************
**
** Function - brcm_recv_int()
**
** Description - Recv data before protocol registration
**
** Returns - 0 if success; else errno
**
*****************************************************************************/
static int brcm_recv_int(struct hci_uart *hu, void *data, int count)
{
    register char *ptr;
    ptr = (char *)data;

    BRCM_HCI_DBG(V4L2_DBG_RX, "%s hu = %p hu->cmd_rcvd = %p count =%d",__func__,hu,
                                                         &hu->cmd_rcvd, count);

    if (ptr!= NULL && (count>2) && ptr[0]==0x04 && ptr[1]==0x0E) {
        BRCM_HCI_DBG(V4L2_DBG_RX, "brcm_recv_int, sending cmd_rcvd");
        memcpy(hu->priv->resp_buffer, ptr,
                   (count<RESP_BUFF_SIZE)?count:RESP_BUFF_SIZE);
        complete_all(&hu->cmd_rcvd);
#if V4L2_SNOOP_ENABLE
        if(nl_sk_hcisnoop)
        {
            /* forward to hcisnoop */
            BRCM_HCI_DBG(V4L2_DBG_RX, "capturing internal command response");
            brcm_hci_snoop(hu, data, count);
        }
#endif
        return 0;
    }

    return -1;
}

/*****************************************************************************
**
** Function - brcm_recv()
**
** Description - Recv data
**
** Returns - 0 if success; else errno
**
*****************************************************************************/
static int brcm_recv(struct hci_uart *hu, void *data, int count)
{
    struct brcm_struct *brcm = hu->priv;
    register char *ptr;
    struct hci_event_hdr *eh;
    struct hci_acl_hdr   *ah;
    struct hci_sco_hdr   *sh;
    struct fm_event_hdr *fm;
    hc_bt_hdr *hci_hdr;
    register int len, type, dlen ,received_bytes;
    static enum proto_type protoid = PROTO_SH_MAX;
    received_bytes = 0;

    BRCM_HCI_DBG(V4L2_DBG_RX, "hu %p count %d rx_state %ld rx_count %ld", hu,
                       count, brcm->rx_state, brcm->rx_count);

    ptr = (char *)data;
    while (count) {
        if (brcm->rx_count) {
            len = min_t(unsigned int, brcm->rx_count, count);
            memcpy(skb_put(brcm->rx_skb, len), ptr, len);
            brcm->rx_count -= len; count -= len; ptr += len;
            received_bytes += len;
            BRCM_HCI_DBG(V4L2_DBG_RX, "brcm_recv received_bytes= %d\n", \
                                                                received_bytes);
            if (brcm->rx_count)
                continue;

            switch (brcm->rx_state) {
            case HCIBRCM_W4_DATA:
                BRCM_HCI_DBG(V4L2_DBG_RX, "Complete data, rx_skb->len:%d", brcm->rx_skb->len);
                /* route frame as internal command response */
                if(unlikely(hu->protos_registered == LDISC_EMPTY)) {
                    memcpy(skb_push(brcm->rx_skb, 1), brcm->rx_skb->cb, 1);
                    if (unlikely(brcm->rx_skb->data[0]==0x04
                        && brcm->rx_skb->data[4]==0x09
                        && brcm->rx_skb->data[5]==0x10 )) {
                        brcm->resp_buffer[0] = brcm->rx_skb->cb[0];
                        memcpy(brcm->resp_buffer, brcm->rx_skb->data,
                                                           RESP_BUFF_SIZE);
                    }
                    complete_all(&hu->cmd_rcvd);
#if V4L2_SNOOP_ENABLE
                    if(nl_sk_hcisnoop)
                    {
                        /* forward internal command response to hcisnoop */
                        type = sh_ldisc_cb(brcm->rx_skb)->pkt_type;
                        /* remove the type */
                        skb_pull(brcm->rx_skb, 1);
                        hci_hdr = hu->hdr_data;
                        hci_hdr->offset = 0;
                        hci_hdr->layer_specific = 0;
                        hci_hdr->event = MSG_HC_TO_STACK_HCI_EVT;
                        hci_hdr->len = brcm->rx_skb->len;
                        memcpy(skb_push(brcm->rx_skb, sizeof(hc_bt_hdr))
                            , hci_hdr, sizeof(hc_bt_hdr));
                        BRCM_HCI_DBG(V4L2_DBG_RX,"forwarding internal command"\
                            " response to snoop with type=0x%02X", type);
                        brcm_hci_snoop(hu, brcm->rx_skb->data, brcm->rx_skb->len);
                    }
#endif
                    kfree_skb(brcm->rx_skb);
                }
                else { /* route frame to registered protocol driver */
                    /*In normal case all the HCI events gets routed to BT protocol driver*/
                    /*But in this case opcode FC61 sent by FM driver hence the response*/
                    /* has to go to FM driver so modifying the  type and protoid*/
                    if (hu->is_registered[PROTO_SH_FM]  && ((brcm->rx_skb->data[0]==0x0e
                        && (brcm->rx_skb->data[3]==0x61 || brcm->rx_skb->data[3]==0x15)
                        && brcm->rx_skb->data[4]==0xFC ) ||
                        (brcm->rx_skb->data[0]==0xFF
                        && brcm->rx_skb->data[2]==0x08)))
                    {
                        type = FM_CH8_PKT;
                        sh_ldisc_cb(brcm->rx_skb)->pkt_type = type;
                        protoid = PROTO_SH_FM;
                    }
#ifdef V4L2_ANT
                    else if (unlikely(brcm->rx_skb->data[0]==0x0e
                        && brcm->rx_skb->data[3]==0xec
                        && brcm->rx_skb->data[4]==0xFC ))
                    {
                        type = ANT_PKT;
                        sh_ldisc_cb(brcm->rx_skb)->pkt_type = type;
                        protoid = PROTO_SH_ANT;
                        BRCM_HCI_DBG(V4L2_DBG_RX, "brcm_recv ANT cmd complete evt");
                    }
                    else if (unlikely(brcm->rx_skb->data[0]==0xff
                        && brcm->rx_skb->data[2]==0x2d ))
                    {
                        type = ANT_PKT;
                        sh_ldisc_cb(brcm->rx_skb)->pkt_type = type;
                        protoid = PROTO_SH_ANT;
                        BRCM_HCI_DBG(V4L2_DBG_RX, "brcm_recv ANT evt");
                    }
#endif
                    else
                    {
                        type = sh_ldisc_cb(brcm->rx_skb)->pkt_type;
                        BRCM_HCI_DBG(V4L2_DBG_RX, "brcm_recv type 0x%x",type);
                    }
                    /* For BT packet */
                    if(type == HCI_EVENT_PKT || type == HCI_ACLDATA_PKT ||
                                                        type == HCI_SCODATA_PKT)
                    {
                        /* Add header to frame only in case of BT packet */
                        brcm_hci_process_frametype(type,hu,brcm->rx_skb,
                                                                received_bytes);

                    }
#if V4L2_SNOOP_ENABLE
                    else { /* For FM packet */
                        BRCM_HCI_DBG(V4L2_DBG_RX,"FM response event received");
                        if(nl_sk_hcisnoop && (type != ANT_PKT))
                        {
                            /* Add header for sending pkt to snoop. Remove header
                                                   before sending to fmdrv */
                            type = sh_ldisc_cb(brcm->rx_skb)->pkt_type;
                            hci_hdr = hu->hdr_data;
                            hci_hdr->offset = 0;
                            hci_hdr->layer_specific = 0;
                            hci_hdr->len = brcm->rx_skb->len;
                            hci_hdr->event = MSG_HC_TO_FM_HCI_EVT;
                            memcpy(skb_push(brcm->rx_skb, sizeof(hc_bt_hdr)),
                                hci_hdr, sizeof(hc_bt_hdr) );
                        }
                    }
#endif
                    BRCM_HCI_DBG(V4L2_DBG_RX, "routing frame to registered "\
                                                                     "driver");
                    brcm_hci_uart_route_frame(protoid, hu, brcm->rx_skb);
                }
                brcm->rx_state = HCIBRCM_W4_PACKET_TYPE;
                brcm->rx_skb = NULL;
                protoid = PROTO_SH_MAX;
                continue;

            case HCIBRCM_W4_EVENT_HDR:
                eh = hci_event_hdr(brcm->rx_skb);
                brcm_check_data_len(brcm, hu, protoid, eh->plen);
                continue;

            case HCIBRCM_W4_ACL_HDR:
                ah = hci_acl_hdr(brcm->rx_skb);
                dlen = __le16_to_cpu(ah->dlen);
                BRCM_HCI_DBG(V4L2_DBG_RX, "ACL header: dlen %d", dlen);
                brcm_check_data_len(brcm, hu, protoid, dlen);
                continue;

            case HCIBRCM_W4_SCO_HDR:
                sh = hci_sco_hdr(brcm->rx_skb);
                BRCM_HCI_DBG(V4L2_DBG_RX, "SCO header: dlen %d", sh->dlen);
                brcm_check_data_len(brcm, hu, protoid, sh->dlen);
                continue;

            case FMBRCM_W4_EVENT_HDR:
                fm = (struct fm_event_hdr *)brcm->rx_skb->data;
                BRCM_HCI_DBG(V4L2_DBG_RX, "FM Header: evt 0x%2.2x plen %d",\
                                                           fm->event, fm->plen);
                brcm_check_data_len(brcm, hu, PROTO_SH_FM, fm->plen);
                continue;

            }
        }
        BRCM_HCI_DBG(V4L2_DBG_RX, "*ptr =%d",*ptr);

        /* HCIBRCM_W4_PACKET_TYPE */
        switch (*ptr) {
        case HCI_EVENT_PKT:
            BRCM_HCI_DBG(V4L2_DBG_RX, "Event packet");
            brcm->rx_state = HCIBRCM_W4_EVENT_HDR;
            brcm->rx_count = HCI_EVENT_HDR_SIZE;
            type = HCI_EVENT_PKT;
            protoid = PROTO_SH_BT;
            break;

        case HCI_ACLDATA_PKT:
            BRCM_HCI_DBG(V4L2_DBG_RX, "ACL packet");
            brcm->rx_state = HCIBRCM_W4_ACL_HDR;
            brcm->rx_count = HCI_ACL_HDR_SIZE;
            type = HCI_ACLDATA_PKT;
            protoid = PROTO_SH_BT;
            break;

        case HCI_SCODATA_PKT:
            BRCM_HCI_DBG(V4L2_DBG_RX, "SCO packet");
            brcm->rx_state = HCIBRCM_W4_SCO_HDR;
            brcm->rx_count = HCI_SCO_HDR_SIZE;
            type = HCI_SCODATA_PKT;
            protoid = PROTO_SH_BT;
            break;

        /* Channel 8(FM) packet */
        case FM_CH8_PKT:
            BRCM_HCI_DBG(V4L2_DBG_RX,"FM CH8 packet");
            type = FM_CH8_PKT;
            brcm->rx_state = FMBRCM_W4_EVENT_HDR;
            brcm->rx_count = FM_EVENT_HDR_SIZE;
            protoid = PROTO_SH_FM;
            break;
        default:
            BRCM_HCI_DBG(V4L2_DBG_RX, "Unknown HCI packet type %2.2x",\
                                                                   (__u8)*ptr);
            ptr++; count--;
            continue;
        };

        ptr++; count--;
        BRCM_HCI_DBG(V4L2_DBG_RX, "*ptr =%d count %d protoid %d",*ptr,
                                                              count,protoid);

        switch (protoid)
        {
            case PROTO_SH_BT:
            case PROTO_SH_FM:
#ifdef V4L2_ANT
            case PROTO_SH_ANT:
#endif
                /* Allocate new packet to hold received data */
                brcm->rx_skb = alloc_skb(HCI_MAX_FRAME_SIZE, GFP_ATOMIC);
                if(brcm->rx_skb)
                    skb_reserve(brcm->rx_skb,8);
                if (!brcm->rx_skb)
                {
                    BRCM_HCI_ERR("Can't allocate mem for new packet");
                    brcm->rx_state = HCIBRCM_W4_PACKET_TYPE;
                    brcm->rx_count = 0;
                    return -ENOMEM;
                }
                brcm->rx_skb->dev = (void *) hu->hdev;
                sh_ldisc_cb(brcm->rx_skb)->pkt_type = type;
                break;
            case PROTO_SH_MAX:
            case PROTO_SH_GPS:
                break;
        }

    }
    BRCM_HCI_DBG(V4L2_DBG_RX, "%s count %d",__func__,count);

    return count;
}

/*****************************************************************************
**
** Function - brcm_dequeue()
**
** Description - Dequeue skb from the skb queue
**
** Returns - 0 if success; else errno
**
*****************************************************************************/
static struct sk_buff *brcm_dequeue(struct hci_uart *hu)
{
    struct brcm_struct *brcm = hu->priv;
    return skb_dequeue(&brcm->txq);
}

/*****************************************************************************
**   UART API calls - End
*****************************************************************************/



/*****************************************************************************
**
** Function - brcm_init()
**
** Description - Registers the UART function pointers to the Line
                  discipline driver. Called after the UART driver
                  is loaded
**
** Returns - 0 if success; else errno
**
*****************************************************************************/
int brcm_init(void)
{
    int err = brcm_hci_uart_register_proto(&brcmp);

    if (!err)
        BRCM_HCI_DBG(V4L2_DBG_INIT, "HCIBRCM protocol initialized");
    else
        BRCM_HCI_ERR("HCIBRCM protocol registration failed");

    return err;
}

/*****************************************************************************
**
** Function - brcm_deinit()
**
** Description - Unregisters the UART function pointers from the Line
                  discipline driver
**
** Returns - 0 if success; else errno
**
*****************************************************************************/

int brcm_deinit(void)
{
    return brcm_hci_uart_unregister_proto(&brcmp);
}


