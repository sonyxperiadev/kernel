/*
 *  FM Driver for Connectivity chip of Broadcom Corporation.
 *
 *  This sub-module of FM driver is common for FM RX and TX
 *  functionality. This module is responsible for:
 *  1) Forming group of Channel-8 commands to perform particular
 *     functionality (eg., frequency set require more than
 *     one Channel-8 command to be sent to the chip).
 *  2) Sending each Channel-8 command to the chip and reading
 *     response back over Shared Transport.
 *  3) Managing TX and RX Queues and Tasklets.
 *  4) Handling FM Interrupt packet and taking appropriate action.
 *
 *  Copyright (C) 2009 Texas Instruments
 *  Copyright (C) 2009-2014 Broadcom Corporation
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/************************************************************************************
 *
 *  Filename:      fmdrv_main.c
 *
 *  Description:   Common sub-module for both FM Rx and Tx. Currently, only
 *                  is supported
 *
 ***********************************************************************************/

#include <linux/module.h>
#include <linux/delay.h>
#include "fmdrv.h"
#include "fmdrv_v4l2.h"
#include "fmdrv_main.h"
#include "../include/brcm_ldisc_sh.h"
#include "fmdrv_rx.h"
#include "fm_public.h"
#include "fmdrv_config.h"
#include "../include/v4l2_target.h"
#include "../include/v4l2_logs.h"

#ifndef V4L2_FM_DEBUG
#define V4L2_FM_DEBUG TRUE
#endif

/* set this module parameter to enable debug info */
int fm_dbg_param = 0;

/* Region info */
struct region_info region_configs[] = {
     /* Europe */
    {
     .low_bound = FM_GET_FREQ(8750),    /* 87.5 MHz */
     .high_bound = FM_GET_FREQ(10800),    /* 108 MHz */
     .deemphasis = FM_DEEMPHA_50U,
     .scan_step = 100,
     .fm_band = 0,
     },

    /* Japan */
    {
     .low_bound = FM_GET_FREQ(7600),    /* 76 MHz */
     .high_bound = FM_GET_FREQ(10800),    /* 108 MHz */
     .deemphasis = FM_DEEMPHA_50U,
     .scan_step = 100,
     .fm_band = 1,
     },

     /* North America */
     {
      .low_bound = FM_GET_FREQ(8750),    /* 87.5 MHz */
      .high_bound = FM_GET_FREQ(10800),    /* 108 MHz */
      .deemphasis = FM_DEEMPHA_75U,
      .scan_step = 200,
      },

     /* Russia-Ext */
    {
     .low_bound = FM_GET_FREQ(6580),    /* 65.8 MHz */
     .high_bound = FM_GET_FREQ(10800),    /* 108 MHz */
     .deemphasis = FM_DEEMPHA_75U,
     .scan_step = 100,
    },

    /* China Region */
    {
     .low_bound = FM_GET_FREQ(7600),    /* 76 MHz */
     .high_bound = FM_GET_FREQ(10800),    /* 108 MHz */
     .deemphasis = FM_DEEMPHA_75U,
     .scan_step = 100,
     },

    /* Italy/Thailand */
    {
     .low_bound = FM_GET_FREQ(8750),    /* 87.5 MHz */
     .high_bound = FM_GET_FREQ(10800),    /* 108 MHz */
     .deemphasis = FM_DEEMPHA_50U,
     .scan_step = 50,
     .fm_band = 0,
     },
};


#if V4L2_FM_DEBUG
#define V4L2_FM_DRV_DBG(flag, fmt, arg...) \
        do { \
            if (fm_dbg_param & flag) \
                printk(KERN_DEBUG "(v4l2fmdrv):%s  "fmt"\n" , \
                                           __func__,## arg); \
        } while(0)
#else
#define V4L2_FM_DRV_DBG(flag, fmt, arg...)
#endif
#define V4L2_FM_DRV_ERR(fmt, arg...)  printk(KERN_ERR "(v4l2fmdrv):%s  "fmt"\n" , \
                                           __func__,## arg)

/*******************************************************************************
**  Static Variables
*******************************************************************************/
   //#define BTYES_TO_UINT16(u16, lsb, msb) {u16 = (UINT16)(((UINT16)(lsb) << 8) + (UINT16)(msb)); }
   /* Program Type */
   static char *pty_str[]= {"None", "News", "Current Affairs",
                         "Information","Sport", "Education",
                         "Drama", "Culture","Science",
                         "Varied Speech", "Pop Music",
                         "Rock Music","Easy Listening",
                         "Light Classic Music", "Serious Classics",
                         "other Music","Weather", "Finance",
                         "Childrens Progs","Social Affairs",
                         "Religion", "Phone In", "Travel",
                         "Leisure & Hobby","Jazz", "Country",
                         "National Music","Oldies","Folk",
                         "Documentary", "Alarm Test", "Alarm"};

   /*These are the RDS elements that we are intested to parse*/
   /*Each variable represents one RDS element*/
   /*pi_code_b   -Program Identification code*/
   static __u32 pi_code_b = 1;
   /* pty  --Program Type, Actually an integer value is transmitted and */
   /* if we pass the value to pty_str, we get the appropriate string */
   static __u32 pty =2;
   /*tp*/
   static __u32 tp =3;
   /*ta */
   static __u32 ta =4;
   /*ms_code, a bit represneting Music or Speech*/
   static __u32 ms_code =5;
   /*rds_psn is the Program name in string format, max length is 8 bytes + null terminate */
   static char rds_psn[9];
   /*rds_txt is RDS test message, this could be custom message*/
   static char rds_txt[65];
   /*holds the 3 byte RDS tuple data*/
   static __u8 rds_tupple[3];

   static __u8 psn_const_flags = 0x00;
   static __u8 skip_flag = 1;

   /*CT is time and date information*/
   struct ct
   {
      __u32 day;
      __u32 hour;
      __u32 minute;
      __u32 second;
   };
/*Global to store the CT information after parsing it from RDS stream*/
   static struct ct current_ct;

/* Band selection */
static unsigned char default_radio_region;    /* US */
module_param(default_radio_region, byte, 0);
MODULE_PARM_DESC(default_radio_region, "Region: 0=US, 1=Europe, 2=Japan");

/* RDS buffer blocks */
static unsigned int default_rds_buf = 300;
module_param(default_rds_buf, uint, 0444);
MODULE_PARM_DESC(rds_buf, "RDS buffer entries");

/* Radio Nr */
static int radio_nr = -1;
module_param(radio_nr, int, 0);
MODULE_PARM_DESC(radio_nr, "Radio Nr");

/*******************************************************************************
**  Forward function declarations
*******************************************************************************/

long (*g_bcm_write) (struct sk_buff *skb);

int parse_inrpt_flags(struct fmdrv_ops *fmdev);
int parse_rds_data(struct fmdrv_ops *fmdev);
void send_read_intrp_cmd(struct fmdrv_ops *fmdev);
int read_rds_data(struct fmdrv_ops *);

/*******************************************************************************
**  Functions
*******************************************************************************/

#ifdef FM_DUMP_TXRX_PKT
 /* To dump outgoing FM Channel-8 packets */
inline void dump_tx_skb_data(struct sk_buff *skb)
{
    int len, len_org;
    char index;
    struct fm_cmd_msg_hdr *cmd_hdr;

    cmd_hdr = (struct fm_cmd_msg_hdr *)skb->data;
    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "<<%shdr:%02x len:%02x opcode:%02x type:%s", \
           fm_cb(skb)->completion ? " " : "*", cmd_hdr->header, \
           cmd_hdr->len, cmd_hdr->fm_opcode, \
           cmd_hdr->rd_wr ? "RD" : "WR");

    len_org = skb->len - FM_CMD_MSG_HDR_SIZE;
    if (len_org > 0)
    {
        //V4L2_FM_DRV_DBG("\n   data(%d): ", cmd_hdr->dlen);
        len = min(len_org, 14);
        for (index = 0; index < len; index++)
            V4L2_FM_DRV_DBG(V4L2_DBG_TX, "%x ", \
                   skb->data[FM_CMD_MSG_HDR_SIZE + index]);
        V4L2_FM_DRV_DBG(V4L2_DBG_TX, "%s", (len_org > 14) ? ".." : "");
    }
}

 /* To dump incoming FM Channel-8 packets */
inline void dump_rx_skb_data(struct sk_buff *skb)
{
    int len, len_org;
    char index;
    struct fm_event_msg_hdr  *evt_hdr;

    evt_hdr = (struct fm_event_msg_hdr *)skb->data;
    V4L2_FM_DRV_DBG(V4L2_DBG_RX, ">> header:%02x event:%02x len:%02x",\
        evt_hdr->header, evt_hdr->event_id, evt_hdr->len);

    len_org = skb->len - FM_EVT_MSG_HDR_SIZE;
    if (len_org > 0)
    {
        V4L2_FM_DRV_DBG(V4L2_DBG_RX, "   data(%d): ", evt_hdr->len);
        len = min(len_org, 14);
        for (index = 0; index < len; index++)
            V4L2_FM_DRV_DBG(V4L2_DBG_RX, "%x ",\
                   skb->data[FM_EVT_MSG_HDR_SIZE + index]);
        V4L2_FM_DRV_DBG(V4L2_DBG_RX, "%s", (len_org > 14) ? ".." : "");
    }
}

#endif

/*
 * Store the currently set region
 */
void fmc_update_region_info(struct fmdrv_ops *fmdev,
                unsigned char region_to_set)
{
    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "fmc_update_region_info");
    fmdev->rx.curr_region = region_to_set;
    memcpy(&fmdev->rx.region, &region_configs[region_to_set],
        sizeof(struct region_info));
    fmdev->rx.curr_freq = fmdev->rx.region.low_bound;
    fm_rx_config_deemphasis( fmdev,fmdev->rx.region.deemphasis);
}

/*
* FM common sub-module will schedule this tasklet whenever it receives
* FM packet from ST driver.
*/
static void fm_receive_data_ldisc(struct work_struct *w)
{
    struct fmdrv_ops *fmdev = container_of(w, struct fmdrv_ops,rx_workqueue);
    struct fm_event_msg_hdr *fm_evt_hdr;
    struct sk_buff *skb;
    unsigned long flags;
    unsigned char sub_event, *p;

    /* Process all packets in the RX queue */
    while ((skb = skb_dequeue(&fmdev->rx_q)))
    {
        if (skb->len < sizeof(struct fm_event_msg_hdr))
        {
            pr_err("(fmdrv): skb(%p) has only %d bytes"
                            "atleast need %lu bytes to decode",
                                        skb, skb->len,
                                (unsigned long)sizeof(struct fm_event_msg_hdr));
            kfree_skb(skb);
            continue;
        }
#ifdef FM_DUMP_TXRX_PKT
        dump_rx_skb_data(skb);
#endif
        fm_evt_hdr = (void *)skb->data;
        if (fm_evt_hdr->event_id == HCI_EV_CMD_COMPLETE)
        {
            struct fm_cmd_complete_hdr *cmd_complete_hdr;
            cmd_complete_hdr = (struct fm_cmd_complete_hdr *) &skb->data [FM_EVT_MSG_HDR_SIZE];
            /*parsing the Opcode FC61 response since it is sent by */
            /*FM driver to switch I2S path. Unlike other FM commands*/
            /*FC61 response comes with HCI event*/
            if (((unsigned char )skb->data[4] == 0x61) &&
                ((unsigned char )skb->data[5] == 0xfc) &&
                (&fmdev->maintask_completion != NULL))
            {
                spin_lock_irqsave(&fmdev->resp_skb_lock, flags);
                fmdev->response_skb = skb;
                spin_unlock_irqrestore(&fmdev->resp_skb_lock, flags);
                complete(fmdev->response_completion);
                fmdev->response_completion = NULL;
                atomic_set(&fmdev->tx_cnt, 1);
            }
            /* Anyone waiting for this with completion handler? */
            else if (cmd_complete_hdr->fm_opcode == fmdev->last_sent_pkt_opcode &&
                                fmdev->response_completion != NULL)
            {
                if (fmdev->response_skb != NULL)
                    pr_err("(fmdrv): Response SKB ptr not NULL");

                if(cmd_complete_hdr->fm_opcode == FM_REG_FM_RDS_MSK)
                    fmdev->rx.fm_rds_flag &= ~FM_RDS_FLAG_SCH_FRZ_BIT;

                spin_lock_irqsave(&fmdev->resp_skb_lock, flags);
                fmdev->response_skb = skb;
                spin_unlock_irqrestore(&fmdev->resp_skb_lock, flags);
                complete(fmdev->response_completion);

                fmdev->response_completion = NULL;
                atomic_set(&fmdev->tx_cnt, 1);
            }
            /* This is the VSE interrupt handler case */
            else if (cmd_complete_hdr->fm_opcode == fmdev->last_sent_pkt_opcode &&
                                    fmdev->response_completion == NULL)
            {
                V4L2_FM_DRV_DBG(V4L2_DBG_RX,"(fmdrv) : VSE interrupt handler case for 0x%x", \
                                    cmd_complete_hdr->fm_opcode);
                if (fmdev->response_skb != NULL)
                    pr_err("(fmdrv): Response SKB ptr not NULL");
                spin_lock_irqsave(&fmdev->resp_skb_lock, flags);
                fmdev->response_skb = skb;
                spin_unlock_irqrestore(&fmdev->resp_skb_lock, flags);
                /* Parse the interrupt flags in 0x12 */
                if(cmd_complete_hdr->fm_opcode == FM_REG_FM_RDS_FLAG)
                    parse_inrpt_flags(fmdev);
                else if(cmd_complete_hdr->fm_opcode == FM_REG_RDS_DATA)
                    parse_rds_data(fmdev);

                atomic_set(&fmdev->tx_cnt, 1);
            }
        }
        else if(fm_evt_hdr->event_id == BRCM_FM_VS_EVENT) /* Vendor specific Event */
        {
            V4L2_FM_DRV_DBG(V4L2_DBG_RX, ": Got Vendor specific Event");
            p = &skb->data[FM_EVT_MSG_HDR_SIZE];

            /* Check if this is a FM vendor specific event */
            STREAM_TO_UINT8(sub_event, p);
            if(sub_event == BRCM_VSE_SUBCODE_FM_INTERRUPT)
            {
                V4L2_FM_DRV_DBG(V4L2_DBG_RX, "(fmdrv) VSE Interrupt event for FM received. Calling fmc_send_intrp_cmd().");
                send_read_intrp_cmd(fmdev);
            }
        }
        else
        {
            pr_err("Unhandled packet SKB(%p),purging", skb);
        }
        if (!skb_queue_empty(&fmdev->tx_q))
		queue_work(fmdev->tx_wq,&fmdev->tx_workqueue);
    }
}

/*
* FM send tasklet: is scheduled when
* FM packet has to be sent to chip */
static void fm_send_data_ldisc(struct work_struct *w)
{
    struct fmdrv_ops *fmdev =container_of(w, struct fmdrv_ops,tx_workqueue);
    struct sk_buff *skb;
    int len;

    /* Send queued FM TX packets */
    if (atomic_read(&fmdev->tx_cnt))
    {
        skb = skb_dequeue(&fmdev->tx_q);
        if (skb)
        {
            atomic_dec(&fmdev->tx_cnt);
            fmdev->last_sent_pkt_opcode = fm_cb(skb)->fm_opcode;

            if (fmdev->response_completion != NULL)
                    pr_err("(fmdrv): Response completion handler"
                                "is not NULL");

            fmdev->response_completion = fm_cb(skb)->completion;
               /* SYED : Hack to set the right packet type for FM */
            sh_ldisc_cb(skb)->pkt_type = FM_PKT_LOGICAL_CHAN_NUMBER;

        }

            /* Write FM packet to hci shared ldisc driver */
            len = g_bcm_write(skb);
            if (len < 0)
            {
                kfree_skb(skb);
                fmdev->response_completion = NULL;
                pr_err("(fmdrv): TX tasklet failed to send" \
                                "skb(%p)", skb);
                atomic_set(&fmdev->tx_cnt, 1);
            }
            else {
                fmdev->last_tx_jiffies = jiffies;
            }
        }
    }


/* Queues FM Channel-8 packet to FM TX queue and schedules FM TX tasklet for
 * transmission */
static int __fm_send_cmd(struct fmdrv_ops *fmdev, unsigned char fmreg_index,
                void *payload, int payload_len, unsigned char type,
                struct completion *wait_completion)
{
    struct sk_buff *skb;
    struct fm_cmd_msg_hdr *cmd_hdr;
    int size;

    size = FM_CMD_MSG_HDR_SIZE + ((payload == NULL) ? 0 : payload_len);
    skb = alloc_skb(size, GFP_ATOMIC);
    if (!skb)
    {
        pr_err("(fmdrv): No memory to create new SKB");
        return -ENOMEM;
    }

    /* Fill command header info */
    cmd_hdr =(struct fm_cmd_msg_hdr *)skb_put(skb, FM_CMD_MSG_HDR_SIZE);

    cmd_hdr->header = HCI_COMMAND;    /* 0x01 */
    /* 3 (cmd, len, fm_opcode,rd_wr) */
    cmd_hdr->cmd = hci_opcode_pack(HCI_GRP_VENDOR_SPECIFIC, FM_2048_OP_CODE);

    cmd_hdr->len = ((payload == NULL) ? 0 : payload_len) + 2;
    /* FM opcode */
    cmd_hdr->fm_opcode = fmreg_index;
    /* read/write type */
    cmd_hdr->rd_wr = type;

    fm_cb(skb)->fm_opcode = fmreg_index;

    if (payload != NULL)
            memcpy(skb_put(skb, payload_len), payload, payload_len);

    fm_cb(skb)->completion = wait_completion;

//    print skb->cb to check pck_type and completion.

    skb_queue_tail(&fmdev->tx_q, skb);
    queue_work(fmdev->tx_wq,&fmdev->tx_workqueue);

    return 0;
}

/* QueuesVSC HCI packet to FM TX queue and schedules FM TX tasklet for
 * transmission */
static int __fm_send_vsc_hci_cmd(struct fmdrv_ops *fmdev,__u16 ocf_value,
                void *payload, int payload_len, struct completion *wait_completion)
{
    struct sk_buff *skb;
    struct fm_cmd_msg_hdr *cmd_hdr;
    int size;
    unsigned char *ch  = (unsigned char*)payload;

    size = FM_VSC_HCI_CMD_MSG_HDR_SIZE + ((payload == NULL) ? 0 : payload_len);
    skb = alloc_skb(size, GFP_ATOMIC);
    if (!skb)
    {
        pr_err("(fmdrv): No memory to create new SKB");
        return -ENOMEM;
    }

    /* Fill command header info */
    cmd_hdr =(struct fm_cmd_msg_hdr *)
                skb_put(skb, FM_VSC_HCI_CMD_MSG_HDR_SIZE);

    cmd_hdr->header = HCI_COMMAND;    /* 0x01 */
    /* 3 (cmd, len, fm_opcode,rd_wr) */
    cmd_hdr->cmd = hci_opcode_pack(HCI_GRP_VENDOR_SPECIFIC, ocf_value);

    cmd_hdr->len = 0x05;  /*Fixed value for FC61 command*/

    if (payload != NULL)
        memcpy(skb_put(skb, payload_len), ch, payload_len);

    fm_cb(skb)->completion = wait_completion;

    skb_queue_tail(&fmdev->tx_q, skb);

    queue_work(fmdev->tx_wq,&fmdev->tx_workqueue);

    return 0;
}


/* Sends FM Channel-8 command to the chip and waits for the reponse */
int fmc_send_cmd(struct fmdrv_ops *fmdev, unsigned char fmreg_index,
            void *payload, int payload_len, unsigned char type,
            struct completion *wait_completion, void *reponse,
            int *reponse_len)
{
    struct sk_buff *skb;
    struct fm_event_msg_hdr *fm_evt_hdr;
    struct fm_cmd_complete_hdr *cmd_complete_hdr;
    unsigned long timeleft;
    unsigned long flags;
    int ret;

    //V4L2_FM_DRV_DBG("In fmc_send_cmd");

    init_completion(wait_completion);
    if(type == VSC_HCI_CMD)
    {
        ret = __fm_send_vsc_hci_cmd(fmdev, VSC_HCI_WRITE_PCM_PINS_OCF, payload,
                            payload_len, wait_completion);
        if (ret < 0)
           return ret;
    }
    else
    {
        ret = __fm_send_cmd(fmdev, fmreg_index, payload, payload_len, type,
                            wait_completion);
        if (ret < 0)
           return ret;
    }

    timeleft = wait_for_completion_timeout(wait_completion, FM_DRV_TX_TIMEOUT);
    if (!timeleft)
    {
        pr_err("(fmdrv): Timeout(%d sec),didn't get reg"
                            "completion signal from RX tasklet",
                                        jiffies_to_msecs(FM_DRV_TX_TIMEOUT) / 1000);
        return -ETIMEDOUT;
    }
    if (!fmdev->response_skb) {
        pr_err("(fmdrv): Reponse SKB is missing ");
        return -EFAULT;
    }
    spin_lock_irqsave(&fmdev->resp_skb_lock, flags);
    skb = fmdev->response_skb;
    fmdev->response_skb = NULL;
    spin_unlock_irqrestore(&fmdev->resp_skb_lock, flags);

    fm_evt_hdr = (void *)skb->data;
    if (fm_evt_hdr->event_id == HCI_EV_CMD_COMPLETE) /* Vendor specific command response */
    {
        cmd_complete_hdr = (struct fm_cmd_complete_hdr *) &skb->data [FM_EVT_MSG_HDR_SIZE];
        if (cmd_complete_hdr->status != 0)
        {
            pr_err("(fmdrv): Reponse status not success ");
            kfree (skb);
            return -EFAULT;
        }

        //V4L2_FM_DRV_DBG("(fmdrv): Reponse status success : %d ", cmd_complete_hdr->status);
        /* Send reponse data to caller */
        if (reponse != NULL && reponse_len != NULL && fm_evt_hdr->len) {
            /* Skip header info and copy only response data */
            skb_pull(skb, (FM_EVT_MSG_HDR_SIZE + FM_CMD_COMPLETE_HDR_SIZE));
            memcpy(reponse, skb->data, (fm_evt_hdr->len - FM_CMD_COMPLETE_HDR_SIZE) );
            *reponse_len = (fm_evt_hdr->len - FM_CMD_COMPLETE_HDR_SIZE) ;
        }
        else if (reponse_len != NULL && fm_evt_hdr->len == 0) {
            *reponse_len = 0;
        }
    }
    else
    {
        V4L2_FM_DRV_ERR("(fmdrv): Unhandled event ID : %d", fm_evt_hdr->event_id);
    }
    kfree_skb(skb);
    return 0;
}

/* This function has the necessity of calling when FM station changes.  */
void reset_rds_parser(void)
{
    pi_code_b = 1;
    pty = 2;
    tp = 3;
    ta = 4;
    ms_code = 5;
    memset(rds_psn, 0x20, sizeof(rds_psn)-1);
    memset(rds_txt, 0x20, sizeof(rds_txt)-1);
    memset(rds_tupple, 0x00, sizeof(rds_tupple));

    psn_const_flags = 0x00;
    skip_flag = 1;
}

/* Helper function to parse the interrupt bits
* in FM_REG_FM_RDS_FLAG (0x12).
* Called locally by fmdrv_main.c
*/
int parse_inrpt_flags(struct fmdrv_ops *fmdev)
{
    struct sk_buff *skb;
    unsigned long flags;
    unsigned short fm_rds_flag;
    unsigned char response[2];

    spin_lock_irqsave(&fmdev->resp_skb_lock, flags);
    skb = fmdev->response_skb;
    fmdev->response_skb = NULL;
    spin_unlock_irqrestore(&fmdev->resp_skb_lock, flags);

    memcpy(&response, &skb->data[FM_EVT_MSG_HDR_SIZE + FM_CMD_COMPLETE_HDR_SIZE], 2);
    fm_rds_flag= (unsigned short)response[0] + ((unsigned short)response[1] << 8) ;

    if (fmdev->rx.fm_rds_flag & (FM_RDS_FLAG_SCH_FRZ_BIT|FM_RDS_FLAG_CLEAN_BIT))
    {
        fmdev->rx.fm_rds_flag &= ~FM_RDS_FLAG_CLEAN_BIT;
        V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv) : Clean BIT set. So no processing of the current"\
            "FM/RDS flag set");
        kfree_skb(skb);
        return 0;
    }

    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv) Processing the interrupt flag. Flag read is 0x%x 0x%x",\
        response[0], response[1]);
    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv) : flag register(0x%x)", fm_rds_flag);
    if(fm_rds_flag & (I2C_MASK_SRH_TUNE_CMPL_BIT|I2C_MASK_SRH_TUNE_FAIL_BIT))
    {
        /* reset RDS parser */
        reset_rds_parser();

        /* remove sch_tune pending bit */
        fmdev->rx.fm_rds_flag &= ~FM_RDS_FLAG_SCH_BIT;

        if(fm_rds_flag & I2C_MASK_SRH_TUNE_FAIL_BIT)
        {
            V4L2_FM_DRV_ERR("(fmdrv) MASK BIT : Search failure");
            if(fmdev->rx.curr_search_state == FM_STATE_SEEKING)
            {
                fmdev->rx.curr_search_state = FM_STATE_SEEK_ERR;
                complete(&fmdev->seektask_completion);
            }
            else if(fmdev->rx.curr_search_state == FM_STATE_TUNING)
            {
                fmdev->rx.curr_search_state = FM_STATE_TUNE_ERR;
                complete(&fmdev->maintask_completion);
            }
        }
        else
        {
            V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv) MASK BIT : Search success");
            if(fmdev->rx.curr_search_state == FM_STATE_SEEKING)
            {
                fmdev->rx.curr_search_state = FM_STATE_SEEK_CMPL;
                complete(&fmdev->seektask_completion);
            }
            else if(fmdev->rx.curr_search_state == FM_STATE_TUNING)
            {
                fmdev->rx.curr_search_state = FM_STATE_TUNE_CMPL;
                complete(&fmdev->maintask_completion);
            }
        }
    }
    else if((fm_rds_flag & (I2C_MASK_RDS_FIFO_WLINE_BIT|I2C_MASK_SYNC_LOST_BIT))
            == I2C_MASK_RDS_FIFO_WLINE_BIT)
    {
        V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv) Detected WLINE interrupt; Reading RDS.");
        read_rds_data(fmdev);
    }
    kfree_skb(skb);
    return 0;
}

void get_rds_element_value(int ioctl_num, char __user *ioctl_value)
{
/* Even though the values that are returned from this function are global to this file */
/* There is no need to lock  Since this is just a read operation */
/* And we don't maitain the history of any of these values, we just return the */
/* Current value irrespective of it's previous value */
   __u8 *current_ct_buf;
   current_ct_buf = (__u8 *)&current_ct;

   if((ioctl_num > 9) || (ioctl_num <= 0))
       return;

   switch(ioctl_num)
   {
      case GET_PI_CODE:
         if(copy_to_user(ioctl_value, (char *)&pi_code_b, 4))
         {
            V4L2_FM_DRV_ERR("(rds) Failed to copy PI code");
         }
         return;

      case GET_TP_CODE:
        if(copy_to_user(ioctl_value, (char *)&tp, 4))
        {
           V4L2_FM_DRV_ERR("(rds) Failed to copy TP code");
        }
        return;

      case GET_PTY_CODE:
        if(copy_to_user(ioctl_value, (char *)&pty, 4))
        {
           V4L2_FM_DRV_ERR("(rds) Failed to copy PTY code");
        }
        return;

      case GET_TA_CODE:
        if(copy_to_user(ioctl_value, (char *)&ta, 4))
        {
           V4L2_FM_DRV_ERR("(rds) Failed to copy TA code");
        }
        return;

      case GET_MS_CODE:
         if(copy_to_user(ioctl_value, (char *)&ms_code, 4))
         {
            V4L2_FM_DRV_ERR("(rds) Failed to copy MS code");
         }
         return;

      case GET_PS_CODE:
         if(copy_to_user(ioctl_value, rds_psn, strlen(rds_psn)+ 1))
         {
            V4L2_FM_DRV_ERR("(rds) Failed to copy PS code");
         }
         return;

      case GET_RT_MSG:
         if(copy_to_user(ioctl_value, rds_txt, strlen(rds_txt) + 1))
         {
            V4L2_FM_DRV_ERR("(rds) Failed to copy RT Message");
         }
         return;

      case GET_CT_DATA:
          if(copy_to_user(ioctl_value, (char *)current_ct_buf, 16))
          {
             V4L2_FM_DRV_ERR("(rds) Failed to copy CT data");
          }
          return;

      case GET_TMC_CHANNEL:
         *(__u32 *)ioctl_value = 4;
         return;
   }
}

/* Each RDS packet is 104bytes = 4*16 bits + 40 bits (10bits error code for each 16bits data)*/
/* Each RDS packet is devided into 4 blocks of 16bits*/
/* when we receive RDS packet it is devided into 4 * 3Byts tuples, */
/*2 bytes of information and 1byte of meta data */
void parse_rds_tupple(void)
{
   int version =0;
   __u8 byte1, byte2;
   static __u8 group, spare, blkc_byte1, blkc_byte2, blkb_byte1, blkb_byte2;
   static __u8 rds_pty;
   __u32 tmp1;
   __u32 tmp2;
   __u8 offset = 0;

   byte1 = rds_tupple[1];
   byte2 = rds_tupple[0];
/*tempbuf[2] has meta data */
   switch ((rds_tupple[2]& 0x07)) {
      case 0: /* Block A */
         break;

      case 1: /* Block B */
         if (rds_tupple[2] & (BRCM_RDS_BIT_6 | BRCM_RDS_BIT_7)) {
            /* invalid tupple */
            skip_flag = 1; /* skip Block D decode */
            break;
        }
        skip_flag = 0;
        V4L2_FM_DRV_DBG(V4L2_DBG_RX, "block B - group=%d%c tp=%d pty=%d spare=%d\n",
        (byte1 >> 4) & 0x0f,
        ((byte1 >> 3) & 0x01) + 'A',
        (byte1 >> 2) & 0x01,
        ((byte1 << 3) & 0x18) | ((byte2 >> 5) & 0x07),
                byte2 & 0x1f);
         blkb_byte1 = byte1;
         blkb_byte2 = byte2;
         group = (byte1 >> 3) & 0x1f;
         spare = byte2 & 0x1f;
         rds_pty = ((byte1 << 3) & 0x18) | ((byte2 >> 5) & 0x07);
         ms_code = (byte2 >> 3)& 0x1;
         tp = (byte1 & (1 << 2));
         break;

      case 2: /* Block C */
         if (rds_tupple[2] & (BRCM_RDS_BIT_6 | BRCM_RDS_BIT_7)) {
            /* invalid tupple */
            break;
         }
         blkc_byte1 = byte1;
         blkc_byte2 = byte2;
         break;

      case 3 : /* Block D */
         if (rds_tupple[2] & (BRCM_RDS_BIT_6 | BRCM_RDS_BIT_7)) {
            /* invalid tupple */
            break;
         }
         /* Parsing the PI code, PI code will be present in all the Groups in Block-c*/
         version = (group & 0x01);
         if(version) {
            pi_code_b |= (blkc_byte1 << 8) & 0xFF;
            pi_code_b |= (blkc_byte2 << 16) & 0xFFFF;
         }
         if (skip_flag) {
            /* group and spare was */
            break;
         }
         switch (group) {
             /*There are 32 Groups in total but we are only interested in the following Groups*/
            case 0: /* Group 0A */
            case 1: /* Group 0B */
               rds_psn[2*(spare & 0x03)+0] = byte1;
               rds_psn[2*(spare & 0x03)+1] = byte2;
               psn_const_flags |= 0x01 << (spare & 0x03);
               if (psn_const_flags == 0x0F) {
                  V4L2_FM_DRV_DBG(V4L2_DBG_RX, "PSN: %s, PTY: %s, MS: %s\n",rds_psn,
                                   pty_str[rds_pty],ms_code?"Music":"Speech");
                  psn_const_flags = 0x00; // reset
               }
               ta = blkb_byte2 & (0x01 << 4);
               break;
            case 4: /* Group 2A */
               rds_txt[4*(spare & 0x0f)+0] = blkc_byte1;
               rds_txt[4*(spare & 0x0f)+1] = blkc_byte2;
               rds_txt[4*(spare & 0x0f)+2] = byte1;
               rds_txt[4*(spare & 0x0f)+3] = byte2;
            /* Display radio text once we get 16 characters */
            /*        if ((spare & 0x0f) == 0x0f)*/
                if (spare > 16)
                {
                    V4L2_FM_DRV_DBG(V4L2_DBG_RX, "Radio Text: %s\n",rds_txt);
                }
                break;
               /* Parsing  CT information*/
               case 8: /*Group 4A*/
                 /* b0-b14@day = b0-b7@bc_1 + b1-b7@bc_2 */
                 tmp1 = (__u32)((blkc_byte1 << 8) | blkc_byte2);
                 /* b15,b16@day = b0,b1@bb_2 */
                 tmp2 = (__u32)(blkb_byte2 & 0x03);
                 current_ct.day = (tmp2 << 15) | (tmp1 >> 1);
                 tmp1 = (__u32)(blkc_byte2 & 0x01);/* b4@hour = LSB of bc_2 */
                 tmp2 = (__u32)(byte1 & 0xf0);/* b0-b3@hour = highest 4 bits of bd_1 */
                 current_ct.hour = (__u8)((tmp1 << 4) | (tmp2 >> 4));
                 current_ct.minute = ((byte1 & 0x0f) << 2)|((byte2  & 0xc0) >>6);
                 current_ct.second = (byte2 & 0x20) >> 5;
                 offset = byte2  & 0x1f;
                 V4L2_FM_DRV_DBG(V4L2_DBG_RX, "ct day: %d, hour: %d, minute: %d, sec: %d",
                           current_ct.day, current_ct.hour, current_ct.minute, current_ct.second);
                 break;
                case 15:
                   ta = blkb_byte2 & (0x01 << 4);
                }
                break;
         default:
              V4L2_FM_DRV_DBG(V4L2_DBG_RX, "unknown block [%d]\n",rds_tupple[2]);
         }
}

/* Helper function to parse the RDS data
* in FM_REG_FM_RDS_DATA (0x80).
* Called locally by fmdrv_main.c
*/
int parse_rds_data(struct fmdrv_ops *fmdev)
{
    unsigned long flags;
    unsigned char *rds_data;
    unsigned char type, block_index;
    tBRCM_RDS_QUALITY qlty_index;
    int ret, response_len, index=0;
    struct sk_buff *skb;

    V4L2_FM_DRV_DBG(V4L2_DBG_RX, "(rds)");

    spin_lock_irqsave(&fmdev->resp_skb_lock, flags);
    skb = fmdev->response_skb;
    fmdev->response_skb = NULL;
    spin_unlock_irqrestore(&fmdev->resp_skb_lock, flags);
    skb_pull(skb, (sizeof(struct fm_event_msg_hdr) + sizeof(struct fm_cmd_complete_hdr)));
    rds_data = skb->data;
    response_len = skb->len;

    V4L2_FM_DRV_DBG(V4L2_DBG_RX, "(rds) RDS length : %d", response_len);

    /* Read RDS data */
    spin_lock_irqsave(&fmdev->rds_cbuff_lock, flags);
    while (response_len > 0)
    {
        /* Fill RDS buffer as per V4L2 specification.
     * Store control byte
     */

        type = (rds_data[0] & BRCM_RDS_GRP_TYPE_MASK);
        block_index = (type >> 4);
        if (block_index < V4L2_RDS_BLOCK_A|| block_index >= V4L2_RDS_BLOCK_C_ALT)
        {
            V4L2_FM_DRV_ERR("(rds) Block sequence mismatch\n");
            block_index = V4L2_RDS_BLOCK_INVALID;
        }

        qlty_index = (tBRCM_RDS_QUALITY)((rds_data[0] & BRCM_RDS_GRP_QLTY_MASK) >> 2);

        rds_tupple[2] = (block_index & V4L2_RDS_BLOCK_MSK);    /* Offset name */
        rds_tupple[2] |= ((block_index & V4L2_RDS_BLOCK_MSK) << 3);  /* Reserved offset */

        switch(qlty_index)
        {
            case BRCM_RDS_NO_ERR:
                /* Set bits 7 and 8 to 0 to indicate no error / correction*/
              //  V4L2_FM_DRV_DBG(V4L2_DBG_RX, "(rds) qlty : BRCM_RDS_NO_ERR");
                break;
            case BRCM_RDS_2BIT_ERR:
            case BRCM_RDS_3BIT_ERR:
                V4L2_FM_DRV_DBG(V4L2_DBG_RX, "(rds) qlty : %s", ((qlty_index==BRCM_RDS_2BIT_ERR)? \
                    "BRCM_RDS_2BIT_ERR":"BRCM_RDS_3BIT_ERR"));
                /* Set bit 7 to 1 and bit 8 to 0 indicate no error
                    but correction made*/
                rds_tupple[2] |= (BRCM_RDS_BIT_6);
                rds_tupple[2] &= ~(BRCM_RDS_BIT_7);
                break;

            case BRCM_RDS_UNRECOVER:
                V4L2_FM_DRV_DBG(V4L2_DBG_RX, "(rds) qlty : BRCM_RDS_UNRECOVER for data [ 0x%x 0x%x 0x%x]",\
                    rds_data[0], rds_data[1], rds_data[2]);
                /* Set bit 7 to 0 and bit 8 to 1 indicate error */
                rds_tupple[2] |= (BRCM_RDS_BIT_7);
                rds_tupple[2] &= ~(BRCM_RDS_BIT_6);
                break;
             default :
                V4L2_FM_DRV_ERR("(rds) Unknown quality code");
                rds_tupple[2] |= (BRCM_RDS_BIT_7);
                rds_tupple[2] &= ~(BRCM_RDS_BIT_6);

        }

        /* Store data byte. Swap bytes*/
        rds_tupple[0] = rds_data[2]; /* LSB of V4L2 spec block */
        rds_tupple[1] = rds_data[1]; /* MSB of V4L2 spec block */
        parse_rds_tupple();
        memcpy(&fmdev->rx.rds.cbuffer[fmdev->rx.rds.wr_index], &rds_tupple,
               FM_RDS_TUPLE_LENGTH);
        fmdev->rx.rds.wr_index =
            (fmdev->rx.rds.wr_index +
             FM_RDS_TUPLE_LENGTH) % fmdev->rx.rds.buf_size;

        /* Check for overflow & start over */
        if (fmdev->rx.rds.wr_index == fmdev->rx.rds.rd_index) {
            pr_err("RDS buffer overflow");
            fmdev->rx.rds.wr_index = 0;
            fmdev->rx.rds.rd_index = 0;
            break;
        }

        /*Check for end of RDS tuple */
        if ((rds_data + FM_RDS_TUPLE_LENGTH)[FM_RDS_TUPLE_BYTE1] == FM_RDS_END_TUPLE_1ST_BYTE &&
            (rds_data + FM_RDS_TUPLE_LENGTH)[FM_RDS_TUPLE_BYTE2] == FM_RDS_END_TUPLE_2ND_BYTE &&
            (rds_data + FM_RDS_TUPLE_LENGTH)[FM_RDS_TUPLE_BYTE3] == FM_RDS_END_TUPLE_3RD_BYTE )
        {
            pr_err("(fmdrv) End of RDS tuple reached @ %d index", index);
            break;
        }
        response_len -= FM_RDS_TUPLE_LENGTH;
        rds_data += FM_RDS_TUPLE_LENGTH;
        index += FM_RDS_TUPLE_LENGTH;
    }
    spin_unlock_irqrestore(&fmdev->rds_cbuff_lock, flags);

     /* Set Tuner RDS capability bit as RDS data has been detected */
    fmdev->device_info.rxsubchans |= V4L2_TUNER_SUB_RDS;

    /* Wakeup read queue */
    if (fmdev->rx.rds.wr_index != fmdev->rx.rds.rd_index)
        wake_up_interruptible(&fmdev->rx.rds.read_queue);

    V4L2_FM_DRV_DBG(V4L2_DBG_RX, "(fmdrv) Now reset the mask");

    fmdev->rx.fm_rds_mask |= I2C_MASK_RDS_FIFO_WLINE_BIT;

    ret = __fm_send_cmd(fmdev, FM_REG_FM_RDS_MSK, &fmdev->rx.fm_rds_mask,
                            2, REG_WR, NULL);

    V4L2_FM_DRV_DBG(V4L2_DBG_RX, "(fmdrv) Write to FM_REG_FM_RDS_MSK done : %d", ret);

    kfree(skb);
    return 0;
}

/*
 * Read the FM_REG_FM_RDS_FLAG by sending a read command.
 * Called locally by fmdrv_main.c
 */
void send_read_intrp_cmd(struct fmdrv_ops *fmdev)
{
    unsigned char read_length;
    int ret;

    read_length = FM_READ_2_BYTE_DATA;
    ret = __fm_send_cmd(fmdev, FM_REG_FM_RDS_FLAG, &read_length,
                            sizeof(read_length), REG_RD, NULL);
    if(ret < 0)
    {
        pr_err("(fmdrv) Error reading FM_REG_FM_RDS_FLAG");
    }
    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv) Sent read to Interrupt flag FM_REG_FM_RDS_FLAG");
}

/* Initiate a read to RDS register. Called locally by fmdrv_main.c */
int read_rds_data(struct fmdrv_ops *fmdev)
{
    unsigned char payload;
    int ret;

    payload = FM_RDS_FIFO_MAX;

    V4L2_FM_DRV_DBG(V4L2_DBG_RX, "(fmdrv) Going to read RDS data from FM_REG_RDS_DATA!!");

    ret = __fm_send_cmd(fmdev, FM_REG_RDS_DATA, &payload, 1, REG_RD, NULL);
    return 0;
}

/*
 * Function to copy RDS data from the FM ring buffer
 * to the userspace buffer.
 */
int fmc_transfer_rds_from_cbuff(struct fmdrv_ops *fmdev, struct file *file,
                    char __user * buf, size_t count)
{
    unsigned int block_count;
    unsigned long flags;
    int ret;

    /* Block if no new data available */
    if (fmdev->rx.rds.wr_index == fmdev->rx.rds.rd_index) {
        if (file->f_flags & O_NONBLOCK)
            return -EWOULDBLOCK;

        ret = wait_event_interruptible(fmdev->rx.rds.read_queue,
                    (fmdev->rx.rds.wr_index != fmdev->rx.rds.rd_index));
        if (ret) {
            pr_err("(rds) %s Error : EINTR ", __func__);
            return -EINTR;
        }
    }
    /* Calculate block count from byte count */
    count /= 3;
    block_count = 0;
    ret = 0;

    spin_lock_irqsave(&fmdev->rds_cbuff_lock, flags);

    /* Copy RDS blocks from the internal buffer and to user buffer */
    while (block_count < count) {
        if (fmdev->rx.rds.wr_index == fmdev->rx.rds.rd_index)
            break;

        /* Always transfer complete RDS blocks */
        if (copy_to_user
            (buf, &fmdev->rx.rds.cbuffer[fmdev->rx.rds.rd_index],
             FM_RDS_BLOCK_SIZE))
            break;

        /* Increment and wrap the read pointer */
        fmdev->rx.rds.rd_index += FM_RDS_BLOCK_SIZE;

        /* Wrap read pointer */
        if (fmdev->rx.rds.rd_index >= fmdev->rx.rds.buf_size)
            fmdev->rx.rds.rd_index = 0;

        /* Increment counters */
        block_count++;
        buf += FM_RDS_BLOCK_SIZE;
        ret += FM_RDS_BLOCK_SIZE;
    }
    spin_unlock_irqrestore(&fmdev->rds_cbuff_lock, flags);

    V4L2_FM_DRV_DBG(V4L2_DBG_RX, "(rds) %s Done copying %d", __func__, ret);

    return ret;
}

/* Sets the frequency */
int fmc_set_frequency(struct fmdrv_ops *fmdev, unsigned int freq_to_set)
{
    int ret;

    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "In %s, frequency to set %d", __func__, freq_to_set);

    switch (fmdev->curr_fmmode) {
        case FM_MODE_RX:
            ret = fm_rx_set_frequency(fmdev, freq_to_set);
            if(ret != 0)
                V4L2_FM_DRV_ERR("Unable to set frequency. ret = %d", ret);
            break;

        case FM_MODE_TX:
            /* Currently FM TX is not supported */
            V4L2_FM_DRV_ERR("Currently FM TX is not supported");

        default:
            ret = -EINVAL;
    }
    return ret;
}

/* Returns the current tuned frequency */
int fmc_get_frequency(struct fmdrv_ops *fmdev, unsigned int *cur_tuned_frq)
{
    int ret = 0;

    switch (fmdev->curr_fmmode) {
        case FM_MODE_RX:
            ret = fm_rx_get_frequency(fmdev, cur_tuned_frq);
            break;

        case FM_MODE_TX:
        /* Currently FM TX is not supported */
        V4L2_FM_DRV_ERR("Currently FM TX is not supported");

        default:
            ret = -EINVAL;
    }
    return ret;
}

/* Function to initiate SEEK operation */
int fmc_seek_station(struct fmdrv_ops *fmdev, unsigned char direction_upward,
                    unsigned char wrap_around)
{
    return fm_rx_seek_station(fmdev, direction_upward, wrap_around);
}

/* Returns current band index (0-Europe/US; 1-Japan) */
int fmc_get_region(struct fmdrv_ops *fmdev, unsigned char *region)
{
    *region = fmdev->rx.curr_region;
    return 0;
}

/* Set the world region */
int fmc_set_region(struct fmdrv_ops *fmdev, unsigned char region_to_set)
{
    int ret;

    switch (fmdev->curr_fmmode) {
        case FM_MODE_RX:
            if (region_to_set == fmdev->rx.curr_region)
            {
                V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv): Already region is set(%d)", region_to_set);
                return 0;
            }
            ret = fm_rx_set_region(fmdev, region_to_set);
            break;

        case FM_MODE_TX:
        /* Currently FM TX is not supported */
        V4L2_FM_DRV_ERR("Currently FM TX is not supported");

        default:
            ret = -EINVAL;
    }
    return ret;
}

/* Sets the audio mode */
int fmc_set_audio_mode(struct fmdrv_ops *fmdev, unsigned char audio_mode)
{
    int ret;

    switch (fmdev->curr_fmmode) {
        case FM_MODE_RX:
            ret = fm_rx_set_audio_mode(fmdev, audio_mode);
            break;

        case FM_MODE_TX:
            /* Currently FM TX is not supported */
            V4L2_FM_DRV_ERR("Currently FM TX is not supported");

        default:
            ret = -EINVAL;
    }
    return ret;
}

/* Gets the audio mode */
int fmc_get_audio_mode(struct fmdrv_ops *fmdev, unsigned char *audio_mode)
{
    int ret;

    switch (fmdev->curr_fmmode) {
        case FM_MODE_RX:
            ret = fm_rx_get_audio_mode(fmdev, audio_mode);
            break;

        case FM_MODE_TX:
            /* Currently FM TX is not supported */
            V4L2_FM_DRV_ERR("Currently FM TX is not supported");

        default:
            ret = -EINVAL;
    }
    return ret;
}

/* Sets the scan step */
int fmc_set_scan_step(struct fmdrv_ops *fmdev, unsigned char scan_step)
{
    int ret;

    switch (fmdev->curr_fmmode) {
        case FM_MODE_RX:
            ret = fm_rx_set_scan_step(fmdev, scan_step);
            break;

        case FM_MODE_TX:
            /* Currently FM TX is not supported */
            V4L2_FM_DRV_ERR("Currently FM TX is not supported");

        default:
            ret = -EINVAL;
    }
    return ret;
}

/*
* Resets RDS cache parameters
*/
void fmc_reset_rds_cache(struct fmdrv_ops *fmdev)
{
    fmdev->rx.rds.rds_flag = FM_RDS_DISABLE;
    fmdev->rx.rds.wr_index = 0;
    fmdev->rx.rds.rd_index = 0;
    fmdev->device_info.rxsubchans &= ~V4L2_TUNER_SUB_RDS;
}

/*
 * Turn FM ON by sending FM_REG_RDS_SYS commmand
 */
int fmc_turn_fm_on (struct fmdrv_ops *fmdev, unsigned char rds_flag)
{
    int ret;
    unsigned char payload;

    if (rds_flag != FM_RDS_ENABLE && rds_flag != FM_RDS_DISABLE) {
        pr_err("(fmdrv): Invalid rds option");
        return -EINVAL;
    }
    if (rds_flag == FM_RDS_ENABLE)
        payload = (FM_ON | FM_RDS_ON);
    else
        payload = FM_ON;

    ret = fmc_send_cmd(fmdev, FM_REG_RDS_SYS, &payload, sizeof(payload),
            REG_WR, &fmdev->maintask_completion, NULL, NULL);
    FM_CHECK_SEND_CMD_STATUS(ret);

    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv): FM_REG_RDS_SYS write done");

    fmdev->rx.rds.rds_flag = rds_flag;
    return ret;
}

/*
 * Turn off FM
 */
int fmc_turn_fm_off(struct fmdrv_ops *fmdev)
{
    int ret = -EINVAL;
    unsigned char payload;

    /* Mute audio */
    payload = FM_MUTE_ON;
    ret = fm_rx_set_mute_mode(fmdev, payload);

    FM_CHECK_SEND_CMD_STATUS(ret);

    if(ret < 0)
    {
        V4L2_FM_DRV_ERR("(fmdrv): FM mute off during FM Disable operation has failed");
        return ret;
    }

    /* Disable FM */
    payload = FM_OFF;

    ret = fmc_send_cmd(fmdev, FM_REG_RDS_SYS, &payload, sizeof(payload),
            REG_WR, &fmdev->maintask_completion, NULL, NULL);
    FM_CHECK_SEND_CMD_STATUS(ret);

    return ret;
}

/*
 * Set FM Modes(TX, RX, OFF)
 * TX and RX modes are exclusive
 */
int fmc_set_mode(struct fmdrv_ops *fmdev, unsigned char fm_mode)
{
    int ret = 0;

    if (fm_mode >= FM_MODE_ENTRY_MAX) {
        pr_err("(fmdrv): Invalid FM mode : %d", fm_mode);
        ret = -EINVAL;
        return ret;
    }
    if (fmdev->curr_fmmode == fm_mode) {
        V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv): Already fm is in mode(%d)", fm_mode);
         return ret;
    }
    fmdev->curr_fmmode = fm_mode;
    return ret;
}

/*
 * Turn on FM, and other initialization to enable FM
 */
int fmc_enable (struct fmdrv_ops *fmdev, unsigned char opt)
{
    int ret;
    unsigned char rds_en_dis, rdbs_en_dis;

    unsigned short aud_ctrl;
    unsigned char read_length;
    unsigned char resp_buf [1];
    int resp_len;

    if (!test_bit(FM_CORE_READY, &fmdev->flag))
    {
        V4L2_FM_DRV_ERR("(fmdrv): FM core is not ready");
        return -EPERM;
    }

    fmc_set_mode (fmdev, FM_MODE_RX);

    /* turn FM ON */
    rds_en_dis = (opt & (FM_RDS_BIT | FM_RBDS_BIT)) ?
                            FM_RDS_ENABLE : FM_RDS_DISABLE;

    ret = fmc_turn_fm_on (fmdev, rds_en_dis);

    if (ret < 0)
    {
        pr_err ("(fmdrv): FM turn on failed");
        return ret;
    }
    fmdev->rx.fm_func_mask = opt;
    /* wait for 300 ms before sending any more commands */
    mdelay (V4L2_FM_ENABLE_DELAY);

    /* wrire rds control */
    rdbs_en_dis = (opt & FM_RBDS_BIT) ?
            FM_RDBS_ENABLE : FM_RDBS_DISABLE;
    ret = fm_rx_set_rds_system (fmdev, rdbs_en_dis);

    if (ret < 0)
    {
        V4L2_FM_DRV_ERR("(fmdrv): set rds mode failed");
        return ret;
    }
    ret = fm_rx_set_region( fmdev,(opt & FM_REGION_MASK));

    if (ret < 0)
    {
        V4L2_FM_DRV_ERR("(fmdrv): set region has failed");
        return ret;
    }
    /* Read PCM Route settings */
    read_length = FM_READ_1_BYTE_DATA;
    ret = fmc_send_cmd(fmdev, FM_REG_PCM_ROUTE, &read_length, sizeof(read_length), REG_RD,
                    &fmdev->maintask_completion, &resp_buf, &resp_len);
    FM_CHECK_SEND_CMD_STATUS(ret);
    fmdev->rx.pcm_reg = resp_buf[0];
    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv): pcm_reg value %d", fmdev->rx.pcm_reg);

    /* fm enable with mute state. added FM_MANUAL_MUTE*/
    aud_ctrl = (unsigned short)(FM_AUDIO_DAC_ON | \
                    FM_RF_MUTE | FM_Z_MUTE_LEFT_OFF | FM_Z_MUTE_RITE_OFF | \
                    fmdev->rx.region.deemphasis);

    ret = fm_rx_set_audio_ctrl(fmdev, aud_ctrl);

    fmdev->rx.curr_rssi_threshold = DEF_V4L2_FM_SIGNAL_STRENGTH;

    /* Set world region */
    V4L2_FM_DRV_DBG(V4L2_DBG_TX,"(fmdrv): FM Set world region option : %d", DEF_V4L2_FM_WORLD_REGION);
    ret = fmc_set_region(fmdev, DEF_V4L2_FM_WORLD_REGION);
    if (ret < 0) {
        V4L2_FM_DRV_ERR("(fmdrv): Unable to set World region");
        return ret;
    }
    fmdev->rx.curr_region = DEF_V4L2_FM_WORLD_REGION;

    /* Set Scan Step */
#if(defined(DEF_V4L2_FM_WORLD_REGION) && DEF_V4L2_FM_WORLD_REGION == FM_REGION_NA)
    fmdev->rx.sch_step = FM_STEP_200KHZ;
#else
    fmdev->rx.sch_step = FM_STEP_100KHZ;
#endif
    V4L2_FM_DRV_DBG(V4L2_DBG_TX,"(fmdrv): FM Set Scan Step : 0x%x", fmdev->rx.sch_step);
    ret = fmc_set_scan_step(fmdev, fmdev->rx.sch_step);
    if (ret < 0) {
        V4L2_FM_DRV_ERR("(fmdrv): Unable to set scan step");
        return ret;
    }

    /* Enable RDS */
    fm_rx_enable_rds(fmdev);

    return ret;
}

/*
* Returns current FM mode (TX, RX, OFF) */
int fmc_get_mode(struct fmdrv_ops *fmdev, unsigned char *fmmode)
{
    if (!test_bit(FM_CORE_READY, &fmdev->flag)) {
        pr_err("(fmdrv): FM core is not ready");
        return -EPERM;
    }
    if (fmmode == NULL) {
        pr_err("(fmdrv): Invalid memory");
        return -ENOMEM;
    }

    *fmmode = fmdev->curr_fmmode;
    return 0;
}

/*
* Called by LDisc layer when FM packet is available. The pointer to
* this function is registered to LDisc during brcm_sh_ldisc_register() call.*/
static long fm_st_receive(void *arg, struct sk_buff *skb)
{
    struct fmdrv_ops *fmdev;
    __u8 pkt_type = 0x08;

    fmdev = (struct fmdrv_ops *)arg;

    if (skb == NULL) {
        V4L2_FM_DRV_ERR("(fmdrv): Invalid SKB received from LDisp");
        return -EFAULT;
    }
    if (skb->cb[0] != FM_PKT_LOGICAL_CHAN_NUMBER) {
        V4L2_FM_DRV_ERR("(fmdrv): Received SKB (%p) is not FM Channel 8 pkt", skb);
        return -EINVAL;
    }

    memcpy(skb_push(skb, 1), &pkt_type, 1);
    skb_queue_tail(&fmdev->rx_q, skb);

    queue_work(fmdev->rx_wq,&fmdev->rx_workqueue);

    return 0;
}

/*
 * This function will be called from FM V4L2 open function.
 * Register with shared ldisc driver and initialize driver data.
 */
int fmc_prepare(struct fmdrv_ops *fmdev)
{
    static struct sh_proto_s fm_st_proto;
    int ret = 0;

    if (test_bit(FM_CORE_READY, &fmdev->flag)) {
        V4L2_FM_DRV_DBG(V4L2_DBG_OPEN, "(fmdrv): FM Core is already up");
        return ret;
    }

    memset(&fm_st_proto, 0, sizeof(fm_st_proto));
    fm_st_proto.type = PROTO_SH_FM;
    fm_st_proto.recv = fm_st_receive;
    fm_st_proto.match_packet = NULL;
    fm_st_proto.write = NULL; /* shared ldisc driver will fill write pointer */
    fm_st_proto.priv_data = fmdev;

    /* Register with the shared line discipline */
    ret = brcm_sh_ldisc_register(&fm_st_proto);
    if (ret == -1) {
        pr_err("(fmdrv): brcm_sh_ldisc_register failed %d", ret);
        ret = -EAGAIN;
        return ret;
    }
    else {
        V4L2_FM_DRV_DBG(V4L2_DBG_OPEN,"(fmdrv): fmc_prepare: brcm_sh_ldisc_register sucess %d", ret);
    }

    if (fm_st_proto.write != NULL) {
        g_bcm_write = fm_st_proto.write;
    }
    else {
        V4L2_FM_DRV_ERR("(fmdrv): Failed to get shared ldisc write func pointer");
        ret = brcm_sh_ldisc_unregister(PROTO_SH_FM);
        if (ret < 0)
            V4L2_FM_DRV_ERR("(fmdrv): brcm_sh_ldisc_unregister failed %d", ret);
            ret = -EAGAIN;
            return ret;
    }

    spin_lock_init(&fmdev->resp_skb_lock);

    /* Initialize TX queue and TX tasklet */
    skb_queue_head_init(&fmdev->tx_q);
    /* Initialize RX Queue and RX tasklet */
    skb_queue_head_init(&fmdev->rx_q);

    INIT_WORK(&fmdev->tx_workqueue,fm_send_data_ldisc);
    INIT_WORK(&fmdev->rx_workqueue,fm_receive_data_ldisc);

    atomic_set(&fmdev->tx_cnt, 1);
    fmdev->response_completion = NULL;

    /* Do all the broadcom FM hardware specific initialization */
    fmdev->rx.curr_mute_mode = FM_MUTE_OFF;
    fmdev->rx.rds.rds_flag = FM_RDS_DISABLE;
    fmdev->rx.curr_region = DEF_V4L2_FM_WORLD_REGION;
    memcpy(&fmdev->rx.region, &region_configs[fmdev->rx.curr_region],
                            sizeof(struct region_info));
    fmdev->rx.curr_freq = fmdev->rx.region.low_bound;
    fmdev->rx.rds_mode = FM_RDS_SYSTEM_NONE;
    fmdev->rx.curr_snr_threshold = FM_RX_SNR_MAX + 1;
    fmdev->rx.curr_sch_mode = FM_SCAN_NONE;
    fmdev->rx.curr_noise_floor = FM_NFE_DEFAILT;
    fmdev->rx.curr_volume = FM_RX_VOLUME_MAX;
    fmdev->rx.audio_mode = FM_AUTO_MODE;
    fmdev->rx.audio_path = FM_AUDIO_NONE;
    fmdev->rx.sch_step = FM_STEP_NONE;
    fmdev->device_info.capabilities = V4L2_CAP_HW_FREQ_SEEK | V4L2_CAP_TUNER |
                                V4L2_CAP_RADIO | V4L2_CAP_MODULATOR |
                                V4L2_CAP_AUDIO | V4L2_CAP_READWRITE | V4L2_CAP_RDS_CAPTURE;
    fmdev->device_info.type = V4L2_TUNER_RADIO;
    fmdev->device_info.rxsubchans = V4L2_TUNER_SUB_MONO | V4L2_TUNER_SUB_STEREO;
    fmdev->device_info.tuner_capability =V4L2_TUNER_CAP_STEREO | V4L2_TUNER_CAP_LOW | V4L2_TUNER_CAP_RDS;

    /* RDS initialization */
    fmc_reset_rds_cache(fmdev);
    init_waitqueue_head(&fmdev->rx.rds.read_queue);

    set_bit(FM_CORE_READY, &fmdev->flag);
    return ret;
}

/* This function will be called from FM V4L2 release function.
 * Unregister from line discipline driver.
 */
int fmc_release(struct fmdrv_ops *fmdev)
{
    int ret;
    V4L2_FM_DRV_DBG(V4L2_DBG_CLOSE, "(fmdrv) %s", __func__);

    if (!test_bit(FM_CORE_READY, &fmdev->flag)) {
        V4L2_FM_DRV_DBG(V4L2_DBG_CLOSE, "(fmdrv): FM Core is already down");
        return 0;
    }

    cancel_work_sync(&fmdev->tx_workqueue);
    cancel_work_sync(&fmdev->rx_workqueue);

    ret = brcm_sh_ldisc_unregister(PROTO_SH_FM);
    if (ret < 0)
        V4L2_FM_DRV_ERR("(fmdrv): Failed to de-register FM from HCI LDisc - %d", ret);
    else
        V4L2_FM_DRV_DBG(V4L2_DBG_CLOSE, "(fmdrv): Successfully unregistered from  HCI LDisc");

    /* Sevice pending read */
    wake_up_interruptible(&fmdev->rx.rds.read_queue);

    skb_queue_purge(&fmdev->tx_q);
    skb_queue_purge(&fmdev->rx_q);

    fmdev->response_completion = NULL;
    fmdev->rx.curr_freq = 0;

    clear_bit(FM_CORE_READY, &fmdev->flag);
    return ret;
}

/* Module init function. Ask FM V4L module to register video device.
 * Allocate memory for FM driver context
 */
static int __init fm_drv_init(void)
{
    struct fmdrv_ops *fmdev = NULL;
    int ret = -ENOMEM;
/*This is an initialization of CT variable. To perform unit testing*/
/*Even if you remove this initilization functionality won't be effected*/
    current_ct.day = 1;
    current_ct.hour = 2;
    current_ct.minute = 3;
    current_ct.second = 4;

    pr_info("(fmdrv): FM driver version %s", FM_DRV_VERSION);

    fmdev = kzalloc(sizeof(struct fmdrv_ops), GFP_KERNEL);
    if (NULL == fmdev) {
        V4L2_FM_DRV_ERR("(fmdrv): Can't allocate operation structure memory");
        return ret;
    }

    fmdev->rx.rds.buf_size = default_rds_buf * FM_RDS_TUPLE_LENGTH;
    /* Allocate memory for RDS ring buffer */
    fmdev->rx.rds.cbuffer = kzalloc(fmdev->rx.rds.buf_size, GFP_KERNEL);
    if (fmdev->rx.rds.cbuffer == NULL) {
        V4L2_FM_DRV_ERR("Can't allocate rds ring buffer");
        kfree(fmdev);
        return -ENOMEM;
    }

    ret = fm_v4l2_init_video_device(fmdev, radio_nr);
    if (ret < 0)
    {
        kfree(fmdev);
        return ret;
    }

    fmdev->tx_wq= create_workqueue("fm_drv_tx");
    if (!fmdev->tx_wq) {
        V4L2_FM_DRV_ERR("%s(): Unable to create workqueue fm_drv_tx\n", __func__);
        return -ENOMEM;
    }
    fmdev->rx_wq= create_workqueue("fm_drv_rx");
    if (!fmdev->rx_wq) {
        V4L2_FM_DRV_ERR("%s(): Unable to create workqueue fm_drv_rx\n", __func__);
        return -ENOMEM;
    }

    fmdev->curr_fmmode = FM_MODE_OFF;
    return 0;
}

/* Module exit function. Ask FM V4L module to unregister video device */
static void __exit fm_drv_exit(void)
{
    struct fmdrv_ops *fmdev = NULL;
    V4L2_FM_DRV_DBG(V4L2_DBG_INIT, "(fmdrv): fm_drv_exit");

    fmdev = fm_v4l2_deinit_video_device();
    if (fmdev != NULL) {
	destroy_workqueue(fmdev->tx_wq);
	destroy_workqueue(fmdev->rx_wq);
	kfree(fmdev);
    }
}

module_init(fm_drv_init);
module_exit(fm_drv_exit);

module_param(fm_dbg_param, int, S_IRUGO);
MODULE_PARM_DESC(fm_dbg_param, \
               "Set to integer value from 1 to 31 for enabling/disabling" \
               " specific categories of logs");


/* ------------- Module Info ------------- */
MODULE_AUTHOR("Satyajit Roy <roys@broadcom.com>, Syed Ibrahim Moosa <syedibrahim.moosa@broadcom.com>");
MODULE_DESCRIPTION("FM Driver for Connectivity chip of Broadcom Corporation");
MODULE_VERSION(VERSION); /* defined in makefile */
MODULE_LICENSE("GPL");
