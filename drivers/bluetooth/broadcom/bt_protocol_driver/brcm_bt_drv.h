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
**
**  Name:           brcm_bt_drv.h
**
**  Description:    This is the header file for.Bluetooth protocol driver
**
*****************************************************************************/


#ifndef _BT_DRV_H
#define _BT_DRV_H
#include <net/bluetooth/bluetooth.h>
#include <linux/interrupt.h>

#define TRUE   1
#define FALSE  0

#define WORKER_QUEUE TRUE

/* Defines number of seconds to wait for reg completion
 * callback getting called from ST (in case,registration
 * with ST returns PENDING status)
 */
#define BT_REGISTER_TIMEOUT   msecs_to_jiffies(6000)    /* 6 sec */

/* BT driver's local status */
#define BT_DRV_RUNNING     0
#define BT_ST_REGISTERED   1
#define BT_TX_Q_EMPTY      2
#define BT_RX_Q_EMPTY      3


#define BRCM_BT_DEV_MAJOR 0

struct brcm_bt_dev {
    /*register device to linux system*/
    struct cdev c_dev;
    struct class *cl;

    /* used locally,to maintain various BT driver status */
    unsigned long flags;

    /* to hold ST registration callback  status */
    char streg_cbdata;

    /* write function pointer of Line discipline driver */
    long (*st_write) (struct sk_buff *);

    /* Wait on comepletion handler needed to synchronize
        * hci_st_open() and hci_st_registration_completion_cb()
        * functions.*/
    struct completion wait_for_btdrv_reg_completion;


    long flag;                           /*  BT driver state machine info */
    struct sk_buff_head rx_q;            /* RX queue */
    spinlock_t rx_q_lock;                /* Rx queue lock */

    struct sk_buff_head tx_q;            /* TX queue */

    struct workqueue_struct *tx_wq;     /* Fm drv workqueue */
    struct work_struct tx_workqueue;    /* Tx work queue */

    spinlock_t tx_q_lock;                /* Tx queue lock */

    unsigned long last_tx_jiffies;       /* Timestamp of last pkt sent */
    atomic_t tx_cnt;                     /* Number of packets in tx queue */

    /* queue for polling table */
    wait_queue_head_t inq;

};



/* declare functions used in brcm_bt_drv */
static int brcm_bt_drv_open(struct inode *inode, struct file *filp);

static int brcm_bt_drv_close(struct inode *i, struct file *f);

static void brcm_bt_drv_prepare(struct brcm_bt_dev* bt_dev);

static ssize_t brcm_bt_drv_read(struct file *f, char __user *buf, size_t
  len, loff_t *off);

static ssize_t brcm_bt_write(struct file *f, const char __user *buf,
  size_t len, loff_t *off);

static void brcm_bt_st_registration_completion_cb(void *priv_data,
    char data);

static long brcm_bt_st_receive(void *priv_data, struct sk_buff *skb);

static void bt_send_data_ldisc(struct work_struct *work);



#endif

