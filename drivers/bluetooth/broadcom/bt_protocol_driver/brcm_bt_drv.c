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


/******************************************************************************
*      Module Name : Bluetooth Protocol Driver
*
*      Description:     This driver module allows Mini HCI layer in Bluedroid to send/receive
*                            packets to/from bluetooth chip. This driver registers with
*                            Line discipline driver to communicate with bluetooth chip.
*
*******************************************************************************/
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/poll.h>
#include "../include/v4l2_target.h"
#include "../include/brcm_ldisc_sh.h"
#include "../include/v4l2_logs.h"
#include "brcm_bt_drv.h"

#ifndef BTDRV_DEBUG
#define BTDRV_DEBUG TRUE
#endif

#define WRITE_RETRY_CNT 5

/* set this module parameter to enable debug info */
int bt_dbg_param = 0;

/* Debugging for BT protocol driver */
#if BTDRV_DEBUG
#define BT_DRV_DBG(flag, fmt, arg...) \
        do { \
            if (bt_dbg_param & flag) \
                printk(KERN_DEBUG "(btdrv):%s  "fmt"\n" , \
                                           __func__,## arg); \
        } while(0)
#else
    #define BT_DRV_DBG(flag, fmt, arg...)
#endif

#define BT_DRV_ERR(fmt, arg...)  printk(KERN_ERR "(btdrv):%s  "fmt"\n" , \
                                           __func__,## arg)

/* global variables */
struct brcm_bt_dev *bt_dev_p;        /* allocated in init_module */

static int is_print_reg_error = 1;

/* timers for ldisc to send callback on register complete */
static unsigned long jiffi1, jiffi2;


/*****************************************************************************
**
** Function - brcm_bt_drv_open
**
** Description - Open call to BT protocol driver. Called when user-space program opens fd
**                    on BT protocol character driver. This function will register BT protocol driver
**                    to Line discipline driver if not already registered. Receives write function
**                    pointer from Line discipline driver.
**
** Returns - 0 if success; else errno
**
*****************************************************************************/
static int brcm_bt_drv_open(struct inode *inode, struct file *filp)
{
    struct brcm_bt_dev *bt_dev;

    /* register to ldisc driver */
    int err=0;
    static struct sh_proto_s brcm_bt_st_proto;
    unsigned long timeleft;
    unsigned long diff;

    bt_dev = container_of(inode->i_cdev, struct brcm_bt_dev, c_dev);
    filp->private_data = bt_dev;

    /* Already registered with Line discipline ST ? */
    if (test_bit(BT_ST_REGISTERED, &bt_dev->flags)) {
        BT_DRV_DBG(V4L2_DBG_OPEN, "Registered with ST already,open called again?");
        return 0;
    }

    /* Populate BT driver info required by ST */
    memset(&brcm_bt_st_proto, 0, sizeof(brcm_bt_st_proto));

    /* BT driver ID */
    brcm_bt_st_proto.type = PROTO_SH_BT;

    /* Receive function which will be called from ST */
    brcm_bt_st_proto.recv = brcm_bt_st_receive;

    /* Packet match function may used in future */
    brcm_bt_st_proto.match_packet = NULL;

    /* Callback to be called when registration is pending */
    brcm_bt_st_proto.reg_complete_cb = brcm_bt_st_registration_completion_cb;

    /* This is write function pointer of ST. BT driver will make use of this
    * for sending any packets to chip. ST will assign and give to us, so
    * make it as NULL */
    brcm_bt_st_proto.write = NULL;

    /* send in the hst to be received at registration complete callback
    * and during st's receive
    */
    brcm_bt_st_proto.priv_data = bt_dev;

    /* Register with ST layer */
    BT_DRV_DBG(V4L2_DBG_OPEN, "calling ldisc register");
    err = brcm_sh_ldisc_register(&brcm_bt_st_proto);
    if (err == -EINPROGRESS) {
       /* Prepare wait-for-completion handler data structures.
            * Needed to syncronize this and st_registration_completion_cb()
            * functions.
            */
        init_completion(&bt_dev->wait_for_btdrv_reg_completion);

       /* Reset ST registration callback status flag , this value
            * will be updated in hci_st_registration_completion_cb()
            * function whenever it called from ST driver.
            */
       bt_dev->streg_cbdata = -EINPROGRESS;

       /* ST is busy with other protocol registration(may be busy with
            * firmware download).So,Wait till the registration callback
            * (passed as a argument to st_register() function) getting
            * called from ST.
            */
       BT_DRV_DBG(V4L2_DBG_OPEN, " %s waiting for reg completion signal" \
                                                       "from ST", __func__);

       timeleft = wait_for_completion_timeout
                        (&bt_dev->wait_for_btdrv_reg_completion,
                                msecs_to_jiffies(BT_REGISTER_TIMEOUT));
       if (!timeleft) {
            BT_DRV_ERR("Timeout(%ld sec),didn't get reg"
                     "completion signal from ST", BT_REGISTER_TIMEOUT / 1000);
            err = -ETIMEDOUT;
            BT_DRV_DBG(V4L2_DBG_OPEN, "End ret=%d", err);
            return err;
       }

       /* Is ST registration callback called with ERROR value? */
       if (bt_dev->streg_cbdata != 0) {
            BT_DRV_ERR("ST reg completion CB called with invalid"
                "status %d", bt_dev->streg_cbdata);
            err = -EAGAIN;
            BT_DRV_DBG(V4L2_DBG_OPEN, "End ret=%d", err);
            return err;
       }

       err = 0;
    }
    else if (err < 0) {
        if (is_print_reg_error) {
            BT_DRV_ERR("st_register failed %d", err);
            jiffi1 = jiffies;
            is_print_reg_error = 0;
        }
        else {
            jiffi2 = jiffies;
            diff = (long)jiffi2 - (long)jiffi1;
            if ( ((diff *1000)/HZ) >= 1000)
                is_print_reg_error = 1;
        }
        err = -EAGAIN;
        BT_DRV_DBG(V4L2_DBG_OPEN, "End ret=%d", err);
        return err;
    }

    /* Do we have proper ST write function? */
    if (brcm_bt_st_proto.write != NULL) {
        /* We need this pointer for sending any Bluetooth pkts */
        bt_dev->st_write = brcm_bt_st_proto.write;
    }
    else {
        BT_DRV_ERR("failed to get ST write func pointer");

        /* Undo registration with ST */
        err = brcm_sh_ldisc_unregister(PROTO_SH_BT);
        if (err < 0)
            BT_DRV_ERR("st_unregister failed %d", err);

        bt_dev->st_write = NULL;
        err = -EAGAIN;
        BT_DRV_DBG(V4L2_DBG_OPEN, "End ret=%d", err);
        return err;
    }

    brcm_bt_drv_prepare(bt_dev);

    BT_DRV_DBG(V4L2_DBG_OPEN, "End ret=%d", err);
    return err;

}


/*****************************************************************************
**
** Function - brcm_bt_drv_prepare
**
** Description - Initialized the locks on tx and rx queue. Also initialized wait_queue for
**                    blocking select-read in user space.
**
*****************************************************************************/
static void brcm_bt_drv_prepare(struct brcm_bt_dev* bt_dev)
{
    /* Registration with ST successful */
    set_bit(BT_ST_REGISTERED, &bt_dev->flags);

    /* Initialize TX queue and TX tasklet */
    skb_queue_head_init(&bt_dev->tx_q);
    spin_lock_init(&bt_dev->tx_q_lock);

    INIT_WORK(&bt_dev->tx_workqueue,bt_send_data_ldisc);

    /* Initialize RX Queue and RX tasklet */
    skb_queue_head_init(&bt_dev->rx_q);
    spin_lock_init(&bt_dev->rx_q_lock);

    atomic_set(&bt_dev->tx_cnt, 0);

    init_waitqueue_head(&bt_dev->inq);

    return;

}


/*****************************************************************************
**
** Function - brcm_bt_drv_close
**
** Description - Performs cleanup of BT protocol driver.
**
*****************************************************************************/
static int brcm_bt_drv_close(struct inode *i, struct file *f)
{
    int err=0;
    struct brcm_bt_dev *bt_dev_p = f->private_data;

    cancel_work_sync(&bt_dev_p->tx_workqueue);

    /* Unregister from ST layer */
    if (test_and_clear_bit(BT_ST_REGISTERED, &bt_dev_p->flags)) {
        err = brcm_sh_ldisc_unregister(PROTO_SH_BT);
        if (err != 0) {
            BT_DRV_ERR("%s st_unregister failed %d", __func__, err);
            err = -EBUSY;
            BT_DRV_DBG(V4L2_DBG_CLOSE, "End ret=%d", err);
            return err;
        }
        else {
            clear_bit(BT_ST_REGISTERED, &bt_dev_p->flags);
            BT_DRV_DBG(V4L2_DBG_CLOSE, "st_unregister done");
        }
    }

    skb_queue_purge(&bt_dev_p->tx_q);
    skb_queue_purge(&bt_dev_p->rx_q);
    atomic_set(&bt_dev_p->tx_cnt, 0);
    bt_dev_p->st_write = NULL;

    BT_DRV_DBG(V4L2_DBG_CLOSE, "End ret=%d", err);
    return err;
}


/*****************************************************************************
**
** Function - brcm_bt_drv_read
**
** Description - Called when user-space program tries to read a packet.
**
** Returns - Number of bytes in packet.
*****************************************************************************/
static ssize_t brcm_bt_drv_read(struct file *f, char __user *buf, size_t
  len, loff_t *off)
{
    struct sk_buff *skb;
    struct brcm_bt_dev *bt_dev_p = f->private_data;
    size_t skb_size = 0;
    unsigned long flags;

    spin_lock_irqsave(&bt_dev_p->rx_q_lock, flags);
    skb = skb_peek(&bt_dev_p->rx_q);

    if(!skb)
    {
        skb_size = 0;
        BT_DRV_ERR("No skb packet in rx queue. This should " \
            "not happen as user-space program should poll for data "\
            "availability before reading");
        goto exit;
    }
    else {
        skb_size = skb->len;

         /* copy packet to user-space */
         spin_unlock_irqrestore(&bt_dev_p->rx_q_lock, flags);
         if(copy_to_user(buf, skb->data, sizeof(char) * skb_size)){
            /* free the skb */
            /*kfree_skb(skb);*/
            printk("copy to user failed\n");
            return -EFAULT;
         }
         else {
            /* free the skb after copying to user space. Return the size of skb */
            spin_lock_irqsave(&bt_dev_p->rx_q_lock, flags);
            skb = skb_dequeue(&bt_dev_p->rx_q);
            spin_unlock_irqrestore(&bt_dev_p->rx_q_lock, flags);
            kfree_skb(skb);
            return skb_size;
         }
    }

exit:
         spin_unlock_irqrestore(&bt_dev_p->rx_q_lock, flags);
         BT_DRV_DBG(V4L2_DBG_RX, "skb_size=%d", skb_size);
         return skb_size;
}


/*****************************************************************************
**
** Function - brcm_bt_write
**
** Description - Called when user-space program writes to fd.
**
** Returns - Number of bytes written.
*****************************************************************************/
static ssize_t brcm_bt_write(struct file *f, const char __user *buf,
  size_t len, loff_t *off)
{
    int ret=0;
    struct sk_buff *skb;
    struct brcm_bt_dev *bt_dev;
    unsigned long flags;

    bt_dev = f->private_data;
    spin_lock_irqsave(&bt_dev->tx_q_lock, flags);

    if (buf != NULL)
    {
        if(!(skb = alloc_skb(len, GFP_ATOMIC)))
        {
            BT_DRV_ERR("Error in allocating memory for skb\n");
            ret=-EFAULT;
            goto nomem;
        }

        if(copy_from_user(skb_put(skb, len), buf, len))
        {
            BT_DRV_ERR("Error:Could not copy all data bytes from user space\n");
            ret=-EFAULT;
            goto nomem;
        }

    }
    else {
        BT_DRV_ERR("Error: Buffer from user space is NULL\n");
        ret=-EFAULT;
        goto nomem;
    }

    /* writing to tx queue should be atomic */
    skb_queue_tail(&bt_dev->tx_q, skb);
    spin_unlock_irqrestore(&bt_dev->tx_q_lock, flags);

    atomic_inc(&bt_dev->tx_cnt);

    queue_work(bt_dev->tx_wq,&bt_dev->tx_workqueue);

    BT_DRV_DBG(V4L2_DBG_TX, "End len=%d", len);
    return len;

nomem:
     spin_unlock_irqrestore(&bt_dev->tx_q_lock, flags);
     BT_DRV_DBG(V4L2_DBG_TX, "End ret=%d", ret);
     return ret;
}


/*****************************************************************************
**
** Function - brcm_bt_drv_poll
**
** Description - Called by Linux kernel when user-space programs calls select. Kernel system
*                      calls this function to check for data availability.
**
** Returns - Number of bytes written.
*****************************************************************************/
static unsigned int brcm_bt_drv_poll(struct file *filp,
                                         struct poll_table_struct *pwait)
{
    int canread;
    unsigned int mask=0;
    struct brcm_bt_dev *bt_dev;
    unsigned long flags;

    bt_dev = filp->private_data;

    poll_wait(filp,&bt_dev->inq, pwait);

    spin_lock_irqsave(&bt_dev->rx_q_lock, flags);
    if(skb_queue_len(&bt_dev->rx_q) > 0) {
        canread = 1;
    }
    else {
        canread = 0;
    }
    spin_unlock_irqrestore(&bt_dev->rx_q_lock, flags);
    BT_DRV_DBG(V4L2_DBG_RX, "canread = %d", canread);

    if (canread) mask |= POLLIN | POLLRDNORM;

    BT_DRV_DBG(V4L2_DBG_RX, "mask=%d", mask);
    return mask;
}

static void bt_send_data_ldisc(struct work_struct *w)
{
    struct  brcm_bt_dev *bt_dev_p = container_of(w, struct brcm_bt_dev,
                                                                tx_workqueue);
    struct sk_buff *skb;
    int len = 0;
    unsigned long flags;
    unsigned int i;

    BT_DRV_DBG(V4L2_DBG_TX, "sending data to ldisc");

    for (i = 0; i < WRITE_RETRY_CNT && atomic_read(&bt_dev_p->tx_cnt); i++)
    {
        spin_lock_irqsave(&bt_dev_p->tx_q_lock, flags);
        skb = skb_dequeue(&bt_dev_p->tx_q);
        spin_unlock_irqrestore(&bt_dev_p->tx_q_lock, flags);
        if (skb)
        {
            sh_ldisc_cb(skb)->pkt_type = skb->data[0];

            if(bt_dev_p->st_write != NULL){
                len = bt_dev_p->st_write(skb);
            }
            else
            {
                BT_DRV_ERR("%s Error!!! bt_dev_p->st_write is NULL", __func__);
            }

            if(len < 0)
            {
                kfree_skb(skb);
                BT_DRV_ERR("Error!!! sending skb to ldisc_write from \
                    send_tasklet. Packet dropped");
                atomic_set(&bt_dev_p->tx_cnt, 0);
            }
            else
            {
                atomic_dec(&bt_dev_p->tx_cnt);
            }
        }
    }
}


/*  File operations which can be performed on this driver  */
static struct file_operations brcm_bt_drv_fops =
{
  .owner = THIS_MODULE,
  .open = brcm_bt_drv_open,
  .release = brcm_bt_drv_close,
  .read = brcm_bt_drv_read,
  .write = brcm_bt_write,
  .poll = brcm_bt_drv_poll
};


/* ------- Interfaces to Line discipline driver  ------ */

/*****************************************************************************
**
** Function - brcm_bt_st_registration_completion_cb
**
** Description - Called by Line discipline driver to indicate protocol registration completion
**                    status. open() function will wait for signal from this function
**                    when registration function with ldisc returns ST_PENDING.
**
*****************************************************************************/
static void brcm_bt_st_registration_completion_cb(void *priv_data,
                char data)
{
    struct brcm_bt_dev *bt_dev_p = (struct brcm_bt_dev *)priv_data;

    /* hci_st_open() function needs value of 'data' to know
    * the registration status(success/fail),So have a back
    * up of it.
    */
    bt_dev_p->streg_cbdata = data;

    set_bit(BT_ST_REGISTERED, &bt_dev_p->flags);
    BT_DRV_DBG(V4L2_DBG_OPEN, "registration to ldisc cb received");
}


/*****************************************************************************
**
** Function - brcm_bt_st_registration_completion_cb
**
** Description - Called by Line discipline driver when receive data is available.
**
*****************************************************************************/
static long brcm_bt_st_receive(void *priv_data, struct sk_buff *skb)
{
    int err = 0;
    unsigned long flags;

    struct brcm_bt_dev *brcm_bt_dev_p= (struct brcm_bt_dev *)priv_data;

    if (skb == NULL)
    {
        BT_DRV_ERR("Invalid SKB received from ST");
        err = -EFAULT;
        return err;
    }
    if (!brcm_bt_dev_p)
    {
        BT_DRV_ERR("Invalid hci_st memory,freeing SKB");
        err = -EFAULT;
        return err;
    }
    if (!test_bit(BT_DRV_RUNNING, &brcm_bt_dev_p->flags))
    {
        BT_DRV_ERR("Device is not running,freeing SKB");
        err = -EINVAL;
        return err;
    }

    spin_lock_irqsave(&brcm_bt_dev_p->rx_q_lock, flags);
    skb_queue_tail(&brcm_bt_dev_p->rx_q, skb);
    spin_unlock_irqrestore(&brcm_bt_dev_p->rx_q_lock, flags);

    wake_up_interruptible(&brcm_bt_dev_p->inq);

    BT_DRV_DBG(V4L2_DBG_RX, "rx_q len = %d",skb_queue_len(&brcm_bt_dev_p->rx_q));

    if (!skb_queue_empty(&brcm_bt_dev_p->tx_q))
       queue_work(brcm_bt_dev_p->tx_wq,&brcm_bt_dev_p->tx_workqueue);

    return err;
}


/* Global static variables */

static dev_t dev; /* Global variable for dev device number */


/* Driver module initialization and cleanup functions */
static int __init brcm_bt_drv_init(void) /* Constructor */
{
    int err=0;

    if ((err = alloc_chrdev_region(&dev, 0, 1, "brcm_bt_drv")) < 0)
    {
        BT_DRV_ERR("alloc_chrdev_region FAILED");
        return err;
    }
    BT_DRV_DBG(V4L2_DBG_INIT, "<Major, Minor>: <%d, %d>\n", MAJOR(dev), MINOR(dev));

    bt_dev_p = kmalloc(sizeof(struct brcm_bt_dev), GFP_KERNEL);

    if (!bt_dev_p)
    {
        err = -ENOMEM;
        BT_DRV_ERR("kmalloc FAILED");
        return err;
    }

    memset(bt_dev_p, 0, sizeof(struct brcm_bt_dev));

    if ((bt_dev_p->cl \
           = (struct class *)class_create(THIS_MODULE, "brcm_bt_drv")) == NULL)
    {
        err = -1;
        BT_DRV_ERR("class_create FAILED");
        unregister_chrdev_region(dev, 1);
        return err;
    }

    if (device_create(bt_dev_p->cl, NULL, dev, NULL, "brcm_bt_drv") \
            == NULL)
    {
        BT_DRV_ERR("device_create FAILED");
        class_destroy(bt_dev_p->cl);
        unregister_chrdev_region(dev, 1);
        err = -1;
        return err;
    }

    cdev_init(&bt_dev_p->c_dev, &brcm_bt_drv_fops);
    if ((err = cdev_add(&bt_dev_p->c_dev, dev, 1)) == -1)
    {
        BT_DRV_ERR("cdev_add FAILED");
        device_destroy(bt_dev_p->cl, dev);
        class_destroy(bt_dev_p->cl);
        unregister_chrdev_region(dev, 1);
        return err;
    }

    bt_dev_p->tx_wq= create_workqueue("bt_drv");
    if (!bt_dev_p->tx_wq) {
        BT_DRV_ERR("%s(): Unable to create workqueue bt_drv\n", __func__);
        return err;
    }

    set_bit(BT_DRV_RUNNING, &bt_dev_p->flags);

    return err;

}


static void __exit brcm_bt_drv_exit(void) /* Destructor */
{
    destroy_workqueue(bt_dev_p->tx_wq);
    if (test_and_clear_bit(BT_DRV_RUNNING, &bt_dev_p->flags))
    {
        cdev_del(&bt_dev_p->c_dev);
        device_destroy(bt_dev_p->cl, dev);
        class_destroy(bt_dev_p->cl);
        unregister_chrdev_region(dev, 1);
        BT_DRV_DBG(V4L2_DBG_INIT, "Unregistering driver done");
    }

    if(bt_dev_p)
    {
        kfree(bt_dev_p);
        BT_DRV_DBG(V4L2_DBG_INIT, "Driver memory deallocated");
    }

    BT_DRV_DBG(V4L2_DBG_INIT, "BT_protocol driver exited");
}

module_init(brcm_bt_drv_init);
module_exit(brcm_bt_drv_exit);

module_param(bt_dbg_param, int, S_IRUGO);
MODULE_PARM_DESC(ldisc_dbg_param, \
               "Set to integer value from 1 to 31 for enabling/disabling" \
               " specific categories of logs");


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_VERSION(VERSION); /* defined in makefile */
MODULE_DESCRIPTION("Bluetooth driver for Bluedroid. \
 Integrates with Line discipline driver (Shared Transport)");



