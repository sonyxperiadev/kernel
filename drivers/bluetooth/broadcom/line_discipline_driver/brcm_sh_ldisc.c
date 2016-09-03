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
*
*  Filename:      brcm_sh_ldisc.c
*
*  Description:   Broadcom HCI Shared line discipline driver implementation
*
*******************************************************************************/

#include <linux/module.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/poll.h>
#include <linux/stat.h>
#include <linux/moduleparam.h>

#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/signal.h>
#include <linux/ioctl.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/semaphore.h>
#include <linux/version.h>

#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h>

#include "brcm_hci_uart.h"
#include "../include/v4l2_target.h"
#include "../include/fm.h"
#include "../include/ant.h"
#include "../include/v4l2_logs.h"

#include <linux/dirent.h>
#include <linux/ctype.h>
#include <linux/tty.h>

#include <linux/time.h>


/*****************************************************************************
**  Constants & Macros for dynamic logging
*****************************************************************************/
#ifndef BTLDISC_DEBUG
#define BTLDISC_DEBUG TRUE
#endif

/* set this module parameter to enable debug info */
int ldisc_dbg_param = 0;

#if BTLDISC_DEBUG
#define BT_LDISC_DBG(flag, fmt, arg...) \
        do { \
            if (ldisc_dbg_param & flag) \
                printk(KERN_DEBUG "(brcmldisc):%s  "fmt"\n" , \
                                           __func__,## arg); \
        } while(0)
#else
#define BT_LDISC_DBG(flag, fmt, arg...)
#endif

#define BT_LDISC_ERR(fmt, arg...)  printk(KERN_ERR "(brcmldisc)ERR:%s  "fmt"\n" , \
                                           __func__,## arg)

#if V4L2_SNOOP_ENABLE
/* parameter to enable HCI snooping */
int ldisc_snoop_enable_param = 0;
#endif

/*******************************************************************************
**  Constants & Macros
************************************************************************************/

#undef CONFIG_BT_HCIUART_BCSP
#undef CONFIG_BT_HCIUART_LL
#undef CONFIG_BT_HCIUART_H4


#define PROTO_ENTRY(type, name) name
const unsigned char *protocol_strngs[] = {
    PROTO_ENTRY(ST_BT, "Bluetooth"),
    PROTO_ENTRY(ST_FM, "FM"),
    PROTO_ENTRY(ST_GPS, "GPS"),
};
/** Bluetooth Address */
typedef struct {
    uint8_t address[6];
} __attribute__((packed))bt_bdaddr_t;

#define RESP_BUFF_SIZE 1024

/* Baudrate threshold to change UART clock rate to 48 MHz */
#define CLOCK_SET_BAUDRATE 3000000

/* Max length for FW patchfile name */
#define FW_PATCH_FILENAME_MAXLEN 80

#define BD_ADDR_SIZE 18
#define LPM_PARAM_SIZE 100

/* HCI HCI_V4L2 message type definitions */
#define HCI_V4L2_TYPE_COMMAND         1
#define HCI_V4L2_TYPE_ACL_DATA        2
#define HCI_V4L2_TYPE_SCO_DATA        3
#define HCI_V4L2_TYPE_EVENT           4
#define HCI_V4L2_TYPE_FM_CMD          8


/* Message event ID passed from stack to vendor lib */
#define MSG_STACK_TO_HC_HCI_ACL        0x2100 /* eq. BT_EVT_TO_LM_HCI_ACL */
#define MSG_STACK_TO_HC_HCI_SCO        0x2200 /* eq. BT_EVT_TO_LM_HCI_SCO */
#define MSG_STACK_TO_HC_HCI_CMD        0x2000 /* eq. BT_EVT_TO_LM_HCI_CMD */
#define MSG_FM_TO_HC_HCI_CMD           0x4000
#define MSG_HC_TO_FM_HCI_EVT           0x3000

/* struct to parse vendor params */
typedef int (conf_action_t)(char *p_conf_name, char *p_conf_value);

typedef struct {
    const char *conf_entry;
    conf_action_t *p_action;
    long param;
} conf_entry_t;

/*******************************************************************************
**  Local type definitions
*******************************************************************************/
spinlock_t  reg_lock;
static int is_print_reg_error = 1;
static unsigned long jiffi1, jiffi2;
struct mutex cmd_credit;

/* setting custom baudrate received from UIM */
struct ktermios ktermios;
static unsigned char bd_addr_array[6] = {0};
char bd_addr[BD_ADDR_SIZE] = {0,};
char fw_name[FW_PATCH_FILENAME_MAXLEN];

#if V4L2_SNOOP_ENABLE
/* enable/disable HCI snoop logging */
char snoop_enable[2] = {0, 0};
#endif

/* module parameters */
static char lpm_param[LPM_PARAM_SIZE];
static long int custom_baudrate = 0;
static int patchram_settlement_delay = 0;
static int ControllerAddrRead = 0;
static int LpmUseBluesleep = 0;
static enum sleep_type sleep = SLEEP_DEFAULT;

#if V4L2_SNOOP_ENABLE
/* HCI snoop and netlink socket related variables */
/* Variables for netlink sockets for hcisnoop */
#define NETLINK_USER 29

struct sock *nl_sk_hcisnoop = NULL;
static int hcisnoop_client_pid = 0;
static struct nlmsghdr *nlh;
static struct sk_buff *hcisnoop_skb_out;
#endif


/*******************************************************************************
**  Function forward-declarations and Function callback declarations
*******************************************************************************/

static struct hci_uart_proto *hup[HCI_UART_MAX_PROTO];

#define MAX_BRCM_DEVICES    3   /* Imagine 1 on each UART for now */

static struct platform_device *brcm_plt_devices[MAX_BRCM_DEVICES];


/*******************************************************************************
**  Function forward-declarations and Function callback declarations
*******************************************************************************/
/**
  * internal functions to read chip name and
 * download patchram file
 */
static long download_patchram(struct hci_uart*);
static struct platform_device *ldisc_get_plat_device(int id);
static int bcmbt_ldisc_remove(struct platform_device *pdev);

/********************************************************************/


/*****************************************************************************
**  Type definitions
*****************************************************************************/

/* parsing functions */
int parse_custom_baudrate(char *p_conf_name, char *p_conf_value)
{
    pr_info("%s = %s\n", p_conf_name, p_conf_value);
    sscanf(p_conf_value, "%ld", &custom_baudrate);
    return 0;
}

int parse_patchram_settlement_delay(char *p_conf_name, char *p_conf_value)
{
    pr_info("%s = %s\n", p_conf_name, p_conf_value);
    sscanf(p_conf_value, "%d", &patchram_settlement_delay);
    return 0;
}

int parse_LpmUseBluesleep(char *p_conf_name, char *p_conf_value)
{
    pr_info("%s = %s\n", p_conf_name, p_conf_value);
    sscanf(p_conf_value, "%d", &LpmUseBluesleep);
    if( LpmUseBluesleep )
        sleep = SLEEP_BLUESLEEP;
    BT_LDISC_DBG(V4L2_DBG_INIT, "%s sleep %d", __func__,sleep);
    return 0;
}

int parse_ControllerAddrRead(char *p_conf_name, char *p_conf_value)
{
    pr_info("%s = %s\n", p_conf_name, p_conf_value);
    sscanf(p_conf_value, "%d", &ControllerAddrRead);
    return 0;
}

int parse_ldisc_snoop_enable_param(char *p_conf_name, char *p_conf_value)
{
    pr_info("%s = %s\n", p_conf_name, p_conf_value);
    sscanf(p_conf_value, "%d", &ldisc_snoop_enable_param);
    return 0;
}

int parse_lpm_param(char *p_conf_name, char *p_conf_value)
{
    pr_info("%s = %s strlen = %d\n", p_conf_name, p_conf_value, strlen(p_conf_value));
    memset(lpm_param, 0, LPM_PARAM_SIZE);
    strncpy(lpm_param, p_conf_value, strlen(p_conf_value));
    pr_info("parse_lpm_param = %s\n", lpm_param);
    return 0;
}

int dbg_ldisc_drv(char *p_conf_name, char *p_conf_value)
{
    pr_info("%s = %s\n", p_conf_name, p_conf_value);
    sscanf(p_conf_value, "%d", &ldisc_dbg_param);
    return 0;
}

int dbg_bt_drv(char *p_conf_name, char *p_conf_value)
{
    pr_info("%s = %s\n", p_conf_name, p_conf_value);
    return 0;
}

int dbg_fm_drv(char *p_conf_name, char *p_conf_value)
{
    pr_info("%s = %s\n", p_conf_name, p_conf_value);
    return 0;
}

static const conf_entry_t conf_table[] = {
    {"custom_baudrate", parse_custom_baudrate, 0},
    {"patchram_settlement_delay", parse_patchram_settlement_delay, 0},
    {"LpmUseBluesleep", parse_LpmUseBluesleep, 0},
    {"ControllerAddrRead", parse_ControllerAddrRead, 0},
    {"ldisc_snoop_enable_param", parse_ldisc_snoop_enable_param, 0},
    {"lpm_param" , parse_lpm_param, 0},
    {"ldisc_dbg_param",dbg_ldisc_drv},
    {"bt_dbg_param",dbg_bt_drv},
    {"fm_dbg_param",dbg_fm_drv},
    {(const char *) NULL, NULL, 0}
};


#if V4L2_SNOOP_ENABLE
/* Function callback for netlink socket */
static void brcm_hcisnoop_recv_msg(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;

    if(skb != NULL)
    {
        nlh = (struct nlmsghdr *)skb->data;
        BT_LDISC_DBG(V4L2_DBG_TX,"hcisnoop: received read command from hcisnoop client: %s",\
                                                       (char *)nlmsg_data(nlh));
        hcisnoop_client_pid = nlh->nlmsg_pid; /*pid of sending process */
        BT_LDISC_DBG(V4L2_DBG_TX,"hcisnoop: accepting request from pid=%d",\
                                                           hcisnoop_client_pid);
    }
    else {
        BT_LDISC_ERR("skb is NULL");
    }

    return;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0)
struct netlink_kernel_cfg cfg = {
    .input = brcm_hcisnoop_recv_msg,
};
#endif

static int enable_snoop(void)
{
    int err;
    if(ldisc_snoop_enable_param)
    {
        /* check whether snoop is enabled */
        BT_LDISC_DBG(V4L2_DBG_TX, "ldisc_snoop_enable_param = %d",\
            ldisc_snoop_enable_param);

        /* close previously created socket */
        if(nl_sk_hcisnoop)
        {
            netlink_kernel_release(nl_sk_hcisnoop);
            nl_sk_hcisnoop = NULL;
        }
        /* start hci snoop to hcidump */
        /* Create socket for hcisnoop */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0)
        nl_sk_hcisnoop = netlink_kernel_create(&init_net, NETLINK_USER, 0,
                    brcm_hcisnoop_recv_msg, NULL, THIS_MODULE);
#else
        nl_sk_hcisnoop = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
#endif
        if (!nl_sk_hcisnoop)
        {
            BT_LDISC_ERR("Error creating netlink socket for HCI snoop");
            err = -10;
            return err;
        }
        return 0;
    }
    else {
        return 0;
    }
}

/* Create netlink socket to snoop Line discipline driver from user space */
int brcm_hci_snoop(struct hci_uart *hu, void* data, unsigned int count)
{
    int err=0;
    unsigned long flags;
    BT_LDISC_DBG(V4L2_DBG_TX , "brcm_hci_snoop");
    /* Snoop and send data to hcidump */
    if(hcisnoop_client_pid > 0)
    {
        spin_lock_irqsave(&hu->hcisnoop_lock, flags);
        BT_LDISC_DBG(V4L2_DBG_TX , "Routing packet to hcisnoop"\
            " client pid=%d data_len=%d", hcisnoop_client_pid, count);
        /* Also route packet to hcisnoop client */
        hcisnoop_skb_out = nlmsg_new(count, 0);

        if (!hcisnoop_skb_out)
        {
            BT_LDISC_ERR("hcisnoop: Failed to allocate new skb");
            spin_unlock_irqrestore(&hu->hcisnoop_lock, flags);
            return -1;
        }
        nlh = nlmsg_put(hcisnoop_skb_out, 0, 0, NLMSG_DONE, count, 0);
        NETLINK_CB(hcisnoop_skb_out).dst_group = 0; /* not in mcast group */
        memcpy(nlmsg_data(nlh), (char *) data, count);

        if(nl_sk_hcisnoop != NULL)
        {
            if ((err = nlmsg_unicast(nl_sk_hcisnoop, hcisnoop_skb_out,
                    hcisnoop_client_pid)) < 0)
            {
                BT_LDISC_ERR("Error errcode=%d while sending packet to pid = %d",
                    err, hcisnoop_client_pid);
                hcisnoop_client_pid = 0;
            }
            else
                BT_LDISC_DBG(V4L2_DBG_TX , "Forwarded hci packet"\
                                                         "to hcisnoop client");
        }
        else {
            BT_LDISC_ERR("nl_sk_hcisnoop == NULL");
        }
        spin_unlock_irqrestore(&hu->hcisnoop_lock, flags);
    }

    return 0;
}

#endif


/* Function to abstract tty write. This will help in hci snooping both read and write packets */
static int brcm_hci_write(struct hci_uart *hu, const unsigned char* data,
                                                             unsigned int count)
{
    int len = 0;

#if V4L2_SNOOP_ENABLE
    if(nl_sk_hcisnoop)
    {
        unsigned long flags = 0;

        hc_bt_hdr *hci_hdr;
        uint8_t *p;

        /* Snoop and send data to hcidump */
        /* create header for snooping */
        hci_hdr = (hc_bt_hdr *) hu->snoop_hdr_data;
        if(hci_hdr != NULL)
        {
            hci_hdr->offset = 0;
            hci_hdr->layer_specific = 0;
            hci_hdr->len = count - 1;
            BT_LDISC_DBG(V4L2_DBG_TX, "written values to hci_hdr");
        }
        else {
            BT_LDISC_ERR("hci_hdr==NULL");
            return -1;
        }

        /* write to tty->write for download patchram HCI commands */
        if(hu->protos_registered == LDISC_EMPTY)
        {
            BT_LDISC_DBG(V4L2_DBG_TX, "writing to tty->write and snoop tx during"\
                "download patchram");
            spin_lock_irqsave(&hu->hcisnoop_write_lock, flags);
            len = hu->tty->ops->write(hu->tty, data, count);
            spin_unlock_irqrestore(&hu->hcisnoop_write_lock, flags);
        }
        else
        {
            BT_LDISC_DBG(V4L2_DBG_TX,"snoop tx from registered driver");
        }

        switch(data[0])
        {
                /* Construct header for sent packet */
                case HCI_V4L2_TYPE_COMMAND:
                    BT_LDISC_DBG(V4L2_DBG_TX, "HCI_V4L2_TYPE_COMMAND");
                    hci_hdr->event = MSG_STACK_TO_HC_HCI_CMD;
                    break;
                case HCI_V4L2_TYPE_ACL_DATA:
                    BT_LDISC_DBG(V4L2_DBG_TX, "HCI_V4L2_TYPE_ACL_DATA");
                    hci_hdr->event = MSG_STACK_TO_HC_HCI_ACL;
                    break;
                case HCI_V4L2_TYPE_SCO_DATA:
                    BT_LDISC_DBG(V4L2_DBG_TX, "HCI_V4L2_TYPE_SCO_DATA");
                    hci_hdr->event = MSG_STACK_TO_HC_HCI_SCO;
                    break;
                case HCI_V4L2_TYPE_FM_CMD:
                    BT_LDISC_DBG(V4L2_DBG_TX, "HCI_V4L2_TYPE_FM_CMD");
                    hci_hdr->event = MSG_FM_TO_HC_HCI_CMD;
                    break;
                default:
                    BT_LDISC_ERR("Unknown event for received packet event "\
                        "= 0x%02X", data[0]);
                    return count;
                    break;
        }

        /* Hack to adjust layer_specific. Copy first 2 bytes of payload to layer_specific*/
        memcpy(&hci_hdr->layer_specific, (uint8_t *)data + 1, 2);

        /* Add HCI header */
        p = (uint8_t *)(hci_hdr + 1);
        /* Add payload */
        memcpy(p, ((unsigned char *)data) + 1, count-1);
        BT_LDISC_DBG(V4L2_DBG_TX, "Calling brcm_hci_snoop from brcm_hci_write");

        brcm_hci_snoop(hu, hci_hdr, sizeof(hc_bt_hdr) + count - 1);
    }
    else
#endif
    {
        len = hu->tty->ops->write(hu->tty, data, count);
    }
    return len;

}

/* parse and load vendor params */
static int parse_vendor_params(void)
{
    struct hci_uart *hu;
    conf_entry_t *p_entry;
    char *p;
    char *p_name, *p_value;
    hu_ref(&hu, 0);

    p = hu->vendor_params;
    pr_info("parse_vendor_params = %s", hu->vendor_params);
    while ((p_name = strsep(&p, " ")))
    {
        p_entry = (conf_entry_t *)conf_table;
        while (p_entry->conf_entry != NULL)
        {
            if (strncmp(p_entry->conf_entry, (const char *)p_name, \
                strlen(p_entry->conf_entry)) == 0)
            {
                p_value = strsep(&p_name, "=");
                p_entry->p_action(p_value, p_name);
                break;
            }
            p_entry++;
        }
    }
    return 0;
}

static ssize_t show_install(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct hci_uart *hu = dev_get_drvdata(dev);
    memcpy(buf, &hu->ldisc_install, sizeof(unsigned char));
    return 1;
}

static ssize_t store_install(struct device *dev,
        struct device_attribute *attr, char *buf,size_t size)
{
    unsigned long flags = 0;
    struct hci_uart *hu;
    hu_ref(&hu, 0);

    // Avoid race condition if all app (BT/FM/Ant) simultaneously try to write to install entry
    spin_lock_irqsave(&hu->err_lock, flags);

    /* When HCI command timeout or hardware error event occurs in an application,
    * BT/FM/Ant apps should set a value 0x02 to "install sysfs" entry. Ldisc will set
    * "bt_err" and "fm_err" for error indication. All applications which use V4L2 solution
    * should now restart and reset bt_err and fm_err after recovery.
    * E.g: If fm_err is already set, FM app concludes that BT app has already reported an error.
    * FM app only needs to restart.
    */
    if ((hu->ldisc_install == V4L2_STATUS_ON) && (buf[0] == V4L2_STATUS_ERR) \
        && ((hu->ldisc_bt_err == V4L2_ERR_FLAG_RESET) \
        || (hu->ldisc_fm_err == V4L2_ERR_FLAG_RESET)))
    {
        hu->ldisc_install = V4L2_STATUS_ERR;
        hu->ldisc_bt_err = V4L2_ERR_FLAG_SET;
        hu->ldisc_fm_err = V4L2_ERR_FLAG_SET;
        sysfs_notify(&hu->brcm_pdev->dev.kobj, NULL, "install");
        sysfs_notify(&hu->brcm_pdev->dev.kobj, NULL, "bt_err");
        sysfs_notify(&hu->brcm_pdev->dev.kobj, NULL, "fm_err");
        BT_LDISC_ERR("Error!: Userspace app has reported error. install = %c",\
            buf[0]);
    }
    spin_unlock_irqrestore(&hu->err_lock, flags);
    return size;
}

static ssize_t show_bt_err(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct hci_uart *hu = dev_get_drvdata(dev);
    memcpy(buf, &hu->ldisc_bt_err, sizeof(unsigned char));
    return 1;
}

static ssize_t store_bt_err(struct device *dev,
        struct device_attribute *attr, char *buf,size_t size)
{
    struct hci_uart *hu;
    hu_ref(&hu, 0);
    hu->ldisc_bt_err = buf[0];
    return size;
}

static ssize_t show_fm_err(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct hci_uart *hu = dev_get_drvdata(dev);
    memcpy(buf, &hu->ldisc_fm_err, sizeof(unsigned char));
    return 1;
}

static ssize_t store_fm_err(struct device *dev,
        struct device_attribute *attr, char *buf,size_t size)
{
    struct hci_uart *hu;
    hu_ref(&hu, 0);
    hu->ldisc_fm_err = buf[0];

    return size;
}

static ssize_t show_vendor_params(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct hci_uart *hu = dev_get_drvdata(dev);
    memcpy(buf, hu->vendor_params, VENDOR_PARAMS_LEN);
    return VENDOR_PARAMS_LEN;
}

static ssize_t store_vendor_params(struct device *dev,
        struct device_attribute *attr, char *buf,size_t size)
{
    struct hci_uart *hu;
    hu_ref(&hu, 0);
    memcpy(hu->vendor_params, buf, VENDOR_PARAMS_LEN);
    parse_vendor_params();

    // enable/disable snoop
    if(enable_snoop()!=0)
    {
        BT_LDISC_ERR("Unable to enable HCI snoop in ldisc");
    }

    return size;
}

static ssize_t store_bdaddr(struct device *dev,
        struct device_attribute *attr, char *buf,size_t size)
{
    sprintf(bd_addr, "%s\n", buf);

    pr_info("store_bdaddr  %s  size %d",bd_addr,size);
    return size;
}

/* UIM will read firmware patch filename.
    From one of the following
    1. "FwPatchFileName" entry is present in bt_vendor.conf
                                 OR
    2. Read chip name through HCI command.and search through the directory
        to find the exact match.
    UIM passes the filename to ldisc as sysfs entry */
static ssize_t store_fw_patchfile(struct device *dev,
        struct device_attribute *attr, char *buf,size_t size)
{
    sprintf(fw_name, "%s",buf);
    BT_LDISC_DBG(V4L2_DBG_INIT,"store_fw_patchfile  %s size %d ",fw_name,size);
    return size;
}

#if V4L2_SNOOP_ENABLE
static ssize_t show_snoop_enable(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%s", snoop_enable);
}

static ssize_t store_snoop_enable(struct device *dev,
        struct device_attribute *attr, char *buf,size_t size)
{
    memcpy(&snoop_enable, buf, 1);
    if(!ldisc_snoop_enable_param) {
        // enable HCI snoop in line discipline driver
        if(!strcmp(snoop_enable,"1")) {
            if(nl_sk_hcisnoop)
            {
                netlink_kernel_release(nl_sk_hcisnoop);
                nl_sk_hcisnoop = NULL;
            }
            /* start hci snoop to hcidump */
            /* Create socket for hcisnoop */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0)
            nl_sk_hcisnoop = netlink_kernel_create(&init_net, NETLINK_USER, 0,
                        brcm_hcisnoop_recv_msg, NULL, THIS_MODULE);
#else
            nl_sk_hcisnoop = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
#endif
            if (!nl_sk_hcisnoop)
            {
                BT_LDISC_ERR("Error creating netlink socket for HCI snoop");
            }
            BT_LDISC_DBG(V4L2_DBG_TX, "enabling HCI snoop");
        }
        else {
            /* stop hci snoop to hcidump */
            if(nl_sk_hcisnoop)
                netlink_kernel_release(nl_sk_hcisnoop);
            nl_sk_hcisnoop = NULL;
            BT_LDISC_DBG(V4L2_DBG_TX, "disabling HCI snoop");
        }
    }

    return size;
}
#endif

/* structures specific for sysfs entries */
static struct kobj_attribute ldisc_bdaddr =
__ATTR(bdaddr, 0666, NULL,(void *)store_bdaddr);

/* structures specific for sysfs entries */
static struct kobj_attribute ldisc_install =
__ATTR(install, 0666, (void *)show_install, (void *)store_install);

/* structures specific for sysfs entries */
static struct kobj_attribute ldisc_vendor_params =
__ATTR(vendor_params, 0666, (void *)show_vendor_params, (void *)store_vendor_params);

/* structures specific for sysfs entries */
static struct kobj_attribute ldisc_bt_err =
__ATTR(bt_err, 0666, (void *)show_bt_err, (void *)store_bt_err);

/* structures specific for sysfs entries */
static struct kobj_attribute ldisc_fm_err =
__ATTR(fm_err, 0666, (void *)show_fm_err, (void *)store_fm_err);

/* structures specific for sysfs entries */
static struct kobj_attribute ldisc_fw_patchfile =
__ATTR(fw_patchfile, 0666, NULL, (void *)store_fw_patchfile);

#if V4L2_SNOOP_ENABLE
/* structures specific for sysfs entries */
static struct kobj_attribute ldisc_snoop_enable =
__ATTR(snoop_enable, 0666, (void *)show_snoop_enable, (void *)store_snoop_enable);
#endif

static struct attribute *uim_attrs[] = {
    &ldisc_install.attr,
    &ldisc_bdaddr.attr,
    &ldisc_fw_patchfile.attr,
#if V4L2_SNOOP_ENABLE
    &ldisc_snoop_enable.attr,
#endif
    &ldisc_vendor_params.attr,
    &ldisc_bt_err.attr,
    &ldisc_fm_err.attr,
    NULL,
};

static struct attribute_group uim_attr_grp = {
    .attrs = uim_attrs,
};


static void add_channel_to_table(struct hci_uart*hu,
        struct sh_proto_s *new_proto)
{
    BT_LDISC_DBG(V4L2_DBG_INIT, "id %d\n", new_proto->type);

    /* list now has the channel id as index itself */
    hu->list[new_proto->type] = new_proto;
    hu->list[new_proto->type]->priv_data = new_proto->priv_data;
    hu->is_registered[new_proto->type] = true;
}

static void remove_channel_from_table(struct hci_uart*hu,
        enum proto_type proto)
{
    BT_LDISC_DBG(V4L2_DBG_INIT, "id %d\n", proto);
    hu->list[proto] = NULL;
    hu->is_registered[proto] = false;
}

/* to signal completion of line discipline installation
 */
void sh_ldisc_complete(void *sh_data)
{
    struct hci_uart*sh_gdata = (struct hci_uart*)sh_data;
    complete(&sh_gdata->ldisc_installed);
}


/*******************************************************************************
**  Functions
*******************************************************************************/

/*******************************************************************************
**
** Function - brcm_hci_uart_register_proto()
**
** Description - Used by the lower layer to register UART protocol
**             which can be H4, LL, BCSP etc
**
** Returns - 0 if success; errno otherwise
**
*******************************************************************************/
int brcm_hci_uart_register_proto(struct hci_uart_proto *p)
{
    if (p->id >= HCI_UART_MAX_PROTO)
        return -EINVAL;

    if (hup[p->id])
        return -EEXIST;

    hup[p->id] = p;
    BT_LDISC_DBG(V4L2_DBG_INIT, "%p", p);

    return 0;
}

/*******************************************************************************
**
** Function - brcm_hci_uart_unregister_proto()
**
** Description - Used by the lower layer to unregister UART protocol
**             which can be H4, LL, BCSP etc
**
** Returns - 0 if success; errno otherwise
**
*******************************************************************************/
int brcm_hci_uart_unregister_proto(struct hci_uart_proto *p)
{
    if (p->id >= HCI_UART_MAX_PROTO)
        return -EINVAL;

    if (!hup[p->id])
        return -EINVAL;

    hup[p->id] = NULL;

    return 0;
}

/*******************************************************************************
**
** Function - brcm_hci_uart_route_frame()
**
** Description - push the skb received to relevant upper layer
**             protocol stacks, which could be BT, FM or GPS etc
**
** Returns - 0 if success; errno otherwise
**
*******************************************************************************/
void brcm_hci_uart_route_frame(enum proto_type protoid, struct hci_uart *hu,
                    struct sk_buff *skb)
{
    BT_LDISC_DBG(V4L2_DBG_RX, " (prot:%d) ", protoid);

#if V4L2_SNOOP_ENABLE
    if(nl_sk_hcisnoop)
    {
        if (!(hu->is_registered[PROTO_SH_ANT] || hu->is_registered[PROTO_SH_FM])
           || (skb->data)[11] != 0x03 || (skb->data)[12] != 0x0c)
        {
            /* forward to hcisnoop */
            BT_LDISC_DBG(V4L2_DBG_RX, "capturing snoop skb->len=%d", skb->len);
            brcm_hci_snoop(hu, skb->data, skb->len);

            /* Remove hci header used for HCI snoop, before sending to fmdrv */
            if(protoid == PROTO_SH_FM) {
                skb_pull(skb, sizeof(hc_bt_hdr));
            }
        }
    }
#endif

    /* Remove hci header used for HCI snoop, before sending to btdrv */
    if(protoid == PROTO_SH_BT) {
        char type = sh_ldisc_cb(skb)->pkt_type;
        skb_pull(skb, sizeof(hc_bt_hdr));
        memcpy(skb_push(skb, sizeof(char)),&type, sizeof(char));
    }

    if(mutex_is_locked(&cmd_credit))
    {
        if(protoid == PROTO_SH_BT)
        {
            if((skb->data[1]==0x0e && skb->data[3]==0x01) ||  // command_complete evt with hci_credit=1
               (skb->data[1]==0x0f && skb->data[4]==0x01))    // command_status evt with hci_credit=1
            {
                complete_all(&hu->cmd_rcvd);
            }
        }
        else if(protoid == PROTO_SH_FM || protoid == PROTO_SH_ANT)
        {
            if((skb->data[0]==0x0e && skb->data[2]==0x01) ||  // command_complete evt with hci_credit=1
               (skb->data[0]==0x0f && skb->data[3]==0x01))    // command_status evt with hci_credit=1
            {
                complete_all(&hu->cmd_rcvd);
            }
        }
    }

    if (unlikely
            (hu == NULL || skb == NULL
                || hu->list[protoid] == NULL))
    {
        BT_LDISC_ERR("protocol %d not registered, no data to send?",
                            protoid);
        if (hu != NULL && skb != NULL)
            kfree_skb(skb);

        return;
    }
  /* this cannot fail. This shouldn't take long. Should be just skb_queue_tail for the
    *   protocol stack driver
    */
    if (likely(hu->list[protoid]->recv != NULL))
    {
        if (unlikely
        (hu->list[protoid]->recv
        (hu->list[protoid]->priv_data, skb)
        != 0))
        {
            BT_LDISC_ERR(" proto stack %d's ->recv failed", protoid);
            kfree_skb(skb);
            return;
        }
    }
    else
    {
        BT_LDISC_ERR(" proto stack %d's ->recv null", protoid);
        kfree_skb(skb);
    }
    return;
}

/*******************************************************************************
**
** Function - brcm_hci_uart_get_proto()
**
** Description - Function to get the registered UART Protocol
**             which can be H4, LL, BCSP etc
**
** Returns - Pointer to struct hci_uart_proto
**
*******************************************************************************/
static struct hci_uart_proto *brcm_hci_uart_get_proto(unsigned int id)
{
    if (id >= HCI_UART_MAX_PROTO)
        return NULL;

    return hup[id];
}

/*******************************************************************************
**
** Function - brcm_hci_uart_set_proto()
**
** Description - Function to register the UART Protocol
**
** Returns - 0 if success; errno otherwise
**
*******************************************************************************/
static int brcm_hci_uart_set_proto(struct hci_uart *hu, int id)
{
    struct hci_uart_proto *p;
    int err;

    p = brcm_hci_uart_get_proto(id);
    if (!p)
        return -EPROTONOSUPPORT;

    err = p->open(hu);
    if (err)
        return err;

    hu->proto = p;
    BT_LDISC_DBG(V4L2_DBG_INIT, "%p", p);
    return 0;
}

/*******************************************************************************
**
** Function - brcm_hci_uart_set_proto()
**
** Description - Called upon TX completion, prints packet type for debugging
**               purposes
**
** Returns - void
**
*******************************************************************************/
static inline void brcm_hci_uart_tx_complete(struct hci_uart *hu,
                                                                   int pkt_type)
{
    /* Update HCI stat counters */
    switch (pkt_type)
    {
        case HCI_COMMAND_PKT:
            BT_LDISC_DBG(V4L2_DBG_TX, "HCI command packet txed");
            break;

        case HCI_ACLDATA_PKT:
            BT_LDISC_DBG(V4L2_DBG_TX, "HCI ACL DATA packet txed");
            break;

        case HCI_SCODATA_PKT:
            BT_LDISC_DBG(V4L2_DBG_TX ,"HCI SCO DATA packet txed");
            break;
    }
}

/*******************************************************************************
**
** Function - brcm_hci_uart_dequeue()
**
** Description - Function to dequeue SKB from the protocol queue
**
** Returns - Pointer to struct sk_buff
**
*******************************************************************************/
static inline struct sk_buff *brcm_hci_uart_dequeue(struct hci_uart *hu)
{
    struct sk_buff *skb = hu->tx_skb;

    if (!skb)
        skb = hu->proto->dequeue(hu);
    else
        hu->tx_skb = NULL;

    return skb;
}

/*******************************************************************************
**
** Function - brcm_hci_uart_tx_wakeup()
**
** Description - Function to dequeue data and write data
**               to UART driver
**
** Returns - Pointer to struct sk_buff
**
*******************************************************************************/
int brcm_hci_uart_tx_wakeup(struct hci_uart *hu)
{
    struct tty_struct *tty = hu->tty;
    struct sk_buff *skb;
    unsigned long lock_flags;
    if (test_and_set_bit(HCI_UART_SENDING, &hu->tx_state))
    {
        set_bit(HCI_UART_TX_WAKEUP, &hu->tx_state);
        return 0;
    }

    BT_LDISC_DBG(V4L2_DBG_TX, "hci_uart_tx_wakeup");
    brcm_btsleep_wake(sleep);

    do{
        clear_bit(HCI_UART_TX_WAKEUP, &hu->tx_state);

        while ((skb = brcm_hci_uart_dequeue(hu))) {
            int len;
            spin_lock_irqsave(&hu->lock, lock_flags);

            len = tty->ops->write(tty, skb->data, skb->len);

            skb_pull(skb, len);
            if (skb->len)
            {
                hu->tx_skb = skb;
                set_bit(HCI_UART_TX_WAKEUP, &hu->tx_state);
                spin_unlock_irqrestore(&hu->lock, lock_flags);
                break;
            }

            brcm_hci_uart_tx_complete(hu, sh_ldisc_cb(skb)->pkt_type);
            kfree_skb(skb);
            spin_unlock_irqrestore(&hu->lock, lock_flags);
        }
    }while(test_bit(HCI_UART_TX_WAKEUP, &hu->tx_state));

    clear_bit(HCI_UART_SENDING, &hu->tx_state);
    return 0;
}


/*******************************************************************************
**
** Function - brcm_sh_ldisc_lpm_disable()
**
** Description - disable LPM before turning OFF the chip.
**
** Returns - 0 if success; errno otherwise
**
*******************************************************************************/

int brcm_sh_ldisc_lpm_disable(struct hci_uart *hu)
{
    const char hci_lpm_disable_cmd[] = {0x01,0x27,0xfc,0x0c,0x00,0x00,0x00,0x00,\
                                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

    /* Perform LPM disable  */
    brcm_hci_write(hu, hci_lpm_disable_cmd, 16);
    if (!wait_for_completion_timeout
          (&hu->cmd_rcvd, msecs_to_jiffies(CMD_RESP_TIME))) {
            BT_LDISC_ERR(" waiting for set LPM disable. Command response timed out");
            return -ETIMEDOUT;
    }

    BT_LDISC_DBG(V4L2_DBG_INIT, "LPM disabled");
    return 0;
}

void resetErrFlags(struct sh_proto_s *new_proto)
{
    struct hci_uart *hu;
    hu_ref(&hu, 0);

    // reset error flags as protocol driver has initiated registration
    switch(new_proto->type) {
        case PROTO_SH_BT:
            hu->ldisc_bt_err = V4L2_ERR_FLAG_RESET;
            BT_LDISC_DBG(V4L2_DBG_OPEN, "resetting bt_err");
            sysfs_notify(&hu->brcm_pdev->dev.kobj, NULL, "bt_err");
            break;
        case PROTO_SH_FM:
            hu->ldisc_fm_err = V4L2_ERR_FLAG_RESET;
            sysfs_notify(&hu->brcm_pdev->dev.kobj, NULL, "fm_err");
            BT_LDISC_DBG(V4L2_DBG_OPEN, "resetting fm_err");
            break;
        default:
            BT_LDISC_ERR("Unknown proto id");
    }

}

/*******************************************************************************
**
** Function - brcm_sh_ldisc_register()
**
** Description - Register protocol
**              called from upper layer protocol stack drivers (BT or FM)
**
** Returns - 0 if success; errno otherwise
**
*******************************************************************************/
long brcm_sh_ldisc_register(struct sh_proto_s *new_proto)
{
    long err = 0;
    unsigned long flags, diff;
    struct hci_uart *hu;

    hu_ref(&hu, 0);
    BT_LDISC_DBG(V4L2_DBG_OPEN, "%p",hu);

    if(new_proto->type != 0)
    {
        BT_LDISC_DBG(V4L2_DBG_OPEN, "(%d) ", new_proto->type);
    }
    spin_lock_irqsave(&reg_lock, flags);
    if (hu == NULL || /*hu->priv == NULL ||*/
                new_proto == NULL || new_proto->recv == NULL)
    {
        spin_unlock_irqrestore(&reg_lock, flags);
        if (is_print_reg_error)
        {
            BT_LDISC_DBG(V4L2_DBG_OPEN,"%d) ", new_proto->type);
            pr_err("hu/new_proto/recv or reg_complete_cb not ready");
            jiffi1 = jiffies;
            is_print_reg_error = 0;
        }
        else
        {
            jiffi2 = jiffies;
            diff = (long)jiffi2 - (long)jiffi1;
            if ( ((diff *1000)/HZ) >= 1000)
                is_print_reg_error = 1;
        }
        return -1;
    }

    /* check if received proto is a valid proto */
    if (new_proto->type < PROTO_SH_BT || new_proto->type >= PROTO_SH_MAX)
    {
        spin_unlock_irqrestore(&reg_lock, flags);
        pr_err("protocol %d not supported", new_proto->type);
        return -EPROTONOSUPPORT;
    }

    /* check if protocol already registered */
    if (hu->list[new_proto->type] != NULL)
    {
        spin_unlock_irqrestore(&reg_lock, flags);
        BT_LDISC_DBG(V4L2_DBG_OPEN, "protocol %d already registered", new_proto->type);
        return -EALREADY;
    }
    if (test_bit(LDISC_REG_IN_PROGRESS, &hu->sh_ldisc_state)) {
        BT_LDISC_DBG(V4L2_DBG_OPEN, " LDISC_REG_IN_PROGRESS:%d ", new_proto->type);
        /* fw download in progress */

        add_channel_to_table(hu, new_proto);
        hu->protos_registered++;
        new_proto->write = brcm_sh_ldisc_write;

        set_bit(LDISC_REG_PENDING, &hu->sh_ldisc_state);
        spin_unlock_irqrestore(&reg_lock, flags);
        return -EINPROGRESS;
    } else if (hu->protos_registered == LDISC_EMPTY) {
        BT_LDISC_DBG(V4L2_DBG_OPEN, " chnl_id list empty :%d ", new_proto->type);
        set_bit(LDISC_REG_IN_PROGRESS, &hu->sh_ldisc_state);

        /* release lock previously held - re-locked below */
        spin_unlock_irqrestore(&reg_lock, flags);

        err = brcm_sh_ldisc_start(hu);
        BT_LDISC_DBG(V4L2_DBG_OPEN, "brcm_sh_ldisc_start response = %ld", err);
        if (err != 0) {
            clear_bit(LDISC_REG_IN_PROGRESS, &hu->sh_ldisc_state);
            if ((hu->protos_registered != LDISC_EMPTY) &&
                (test_bit(LDISC_REG_PENDING, &hu->sh_ldisc_state))) {
                pr_err(" ldisc registration failed ");
            }
            return -EINVAL;
        }

        BT_LDISC_DBG(V4L2_DBG_OPEN, "clearing flag LDISC_REG_IN_PROGRESS");
        clear_bit(LDISC_REG_IN_PROGRESS, &hu->sh_ldisc_state);

        BT_LDISC_DBG(V4L2_DBG_OPEN,"clearing flag LDISC_REG_PENDING");
        clear_bit(LDISC_REG_PENDING, &hu->sh_ldisc_state);

        /* check for already registered once more, since the above check is old */
        BT_LDISC_DBG(V4L2_DBG_OPEN, "checking already registerd proto");
        if (hu->is_registered[new_proto->type] == true) {
            pr_err(" proto %d already registered ",
                   new_proto->type);
            return -EALREADY;
        }

        spin_lock_irqsave(&reg_lock, flags);

        add_channel_to_table(hu, new_proto);
        hu->protos_registered++;
        BT_LDISC_DBG(V4L2_DBG_OPEN, "Changed hu->protos_registered = %d",
                                                      hu->protos_registered);
        new_proto->write = brcm_sh_ldisc_write;
        spin_unlock_irqrestore(&reg_lock, flags);
        resetErrFlags(new_proto);

        BT_LDISC_DBG(V4L2_DBG_OPEN, "exiting %s with err = %ld", __func__, err);
        return err;
    }
    /* if firmware patchram is already downloaded & new protocol driver registers */
    else {
        add_channel_to_table(hu, new_proto);
        hu->protos_registered++;
        BT_LDISC_DBG(V4L2_DBG_OPEN, "Changed hu->protos_registered = %d",
                                                      hu->protos_registered);
        new_proto->write = brcm_sh_ldisc_write;
        spin_unlock_irqrestore(&reg_lock, flags);
        resetErrFlags(new_proto);
        return err;
    }

    BT_LDISC_DBG(V4L2_DBG_OPEN, "done (%d) ", new_proto->type);
    return err;
}
EXPORT_SYMBOL(brcm_sh_ldisc_register);


/*******************************************************************************
**
** Function - brcm_sh_ldisc_unregister()
**
** Description - UnRegister protocol
**              called from upper layer protocol stack drivers (BT or FM)
**
** Returns - 0 if success; errno otherwise
**
*******************************************************************************/
long brcm_sh_ldisc_unregister(enum proto_type type)
{
    long err = 0;
    unsigned long flags;
    struct hci_uart *hu;

    BT_LDISC_DBG(V4L2_DBG_CLOSE,"%d ", type);

    if (type < PROTO_SH_BT || type >= PROTO_SH_MAX)
    {
        pr_err(" protocol %d not supported", type);
        return -EPROTONOSUPPORT;
    }

    spin_lock_irqsave(&reg_lock, flags);
    hu_ref(&hu, 0);
    BT_LDISC_DBG(V4L2_DBG_CLOSE, "%p ", hu);

     if (hu == NULL)
    {
        spin_unlock_irqrestore(&reg_lock, flags);
        pr_err(" protocol %d not registered", type);
        return -EPROTONOSUPPORT;
    }

    if(hu->protos_registered > 0)
        hu->protos_registered--;
    BT_LDISC_DBG(V4L2_DBG_CLOSE, "Changed hu->protos_registered = %d",hu->protos_registered);
    remove_channel_from_table(hu,type);
    spin_unlock_irqrestore(&reg_lock, flags);

    /* Power OFF chip only if no protos are registered.
       Send notification to UIM to power OFF chip */
    if ((hu->protos_registered == 0) &&
        (!test_bit(LDISC_REG_PENDING, &hu->sh_ldisc_state)) &&
        (!test_bit(LDISC_REG_IN_PROGRESS, &hu->sh_ldisc_state))) {

        BT_LDISC_DBG(V4L2_DBG_CLOSE," all chnl_ids unregistered ");

        /* stop traffic on tty */
        if (hu->tty){
            BT_LDISC_DBG(V4L2_DBG_CLOSE," calling tty_ldisc_flush ");
            tty_ldisc_flush(hu->tty);
            BT_LDISC_DBG(V4L2_DBG_CLOSE," calling stop_tty ");
            stop_tty(hu->tty);
        }

        /* all chnl_ids now unregistered */
        brcm_sh_ldisc_stop(hu);
    }

    return err;
}
EXPORT_SYMBOL(brcm_sh_ldisc_unregister);



/*****************************************************************************
* Function to encode baudrate from int to char sequence
*****************************************************************************/
void BRCM_encode_baud_rate(uint baud_rate, unsigned char *encoded_baud)
{
    if(baud_rate == 0 || encoded_baud == NULL) {
        pr_err("Baudrate not supported!");
        return;
    }

    encoded_baud[3] = (unsigned char)(baud_rate >> 24);
    encoded_baud[2] = (unsigned char)(baud_rate >> 16);
    encoded_baud[1] = (unsigned char)(baud_rate >> 8);
    encoded_baud[0] = (unsigned char)(baud_rate & 0xFF);
}

typedef struct {
    int baud_rate;
    int termios_value;
} tBaudRates;

tBaudRates baud_rates[] = {
    { 115200, B115200 },
    { 230400, B230400 },
    { 460800, B460800 },
    { 500000, B500000 },
    { 576000, B576000 },
    { 921600, B921600 },
    { 1000000, B1000000 },
    { 1152000, B1152000 },
    { 1500000, B1500000 },
    { 2000000, B2000000 },
    { 2500000, B2500000 },
    { 3000000, B3000000 },
	{ 4000000, B4000000 },
};

/*****************************************************************************
* Function to lookup baudrate
*****************************************************************************/
int
lookup_baudrate(long int baud_rate)
{
    unsigned int i;

    for (i = 0; i < (sizeof(baud_rates) / sizeof(tBaudRates)); i++) {
        if (baud_rates[i].baud_rate == baud_rate) {
            return i;
        }
    }

    return(-1);
}


/*****************************************************************************
* Function to encode baudrate from int to char sequence
*****************************************************************************/
void
BRCM_encode_bd_address( unsigned char  *bd_addrr)
{
    if(bd_addrr == NULL) {
        pr_info("%s : BD addr not supported!", __func__);
        return;
    }

    bd_addrr[0] = bd_addr_array[5];
    bd_addrr[1] = bd_addr_array[4];
    bd_addrr[2] = bd_addr_array[3];
    bd_addrr[3] = bd_addr_array[2];
    bd_addrr[4] = bd_addr_array[1];
    bd_addrr[5] = bd_addr_array[0];
}

/*****************************************************************************
* Function to read BD ADDR from bluetooth chip.
*****************************************************************************/
static long read_bd_addr(struct hci_uart *hu)
{
    long err = 0;
    long len =0;
    int i = 0;

    struct tty_struct *tty;
    const char hci_read_bdaddr[] = { 0x01, 0x09, 0x10, 0x00 };
    tty = hu->tty;

    init_completion(&hu->cmd_rcvd);

    len = brcm_hci_write(hu, hci_read_bdaddr, 4);

    if(!wait_for_completion_timeout
            (&hu->cmd_rcvd, msecs_to_jiffies(CMD_RESP_TIME))) {
            pr_err(" waiting for READ_BD_ADDR command response - timed out ");
            return -ETIMEDOUT;
        }

    BT_LDISC_DBG(V4L2_DBG_INIT, "read_bd_addr, received wait_completion");
    for (i=0;i<=5;i++){
        bd_addr_array[i] = (unsigned char)hu->priv->resp_buffer[7+i];
    }
    return err;
}

int str2bd(char *str, bt_bdaddr_t *addr)
{
    int32_t i = 0;
    for (i = 0; i < 6; i++)
    {
        addr->address[i] = (uint8_t)simple_strtoul(str, &str, 16);
        str++;
    }
    return 0;
}


int str2arr(char *str, uint8_t *addr,int len)
{
    int32_t i = 0;
    for (i = 0; i < len; i++)
    {
      addr[i] = (uint8_t)simple_strtoul(str, &str, 16);
       str++;
    }
    return 0;
}

/*****************************************************************************
* Function to download firmware patchfile to bluetooth chip.
*****************************************************************************/
static long download_patchram(struct hci_uart *hu)
{
    long err = 0;
    long len =0;
    struct tty_struct *tty = hu->tty;
    uint8_t hci_writesleepmode_cmd[35] = {0};
    unsigned char *ptr;

    const unsigned char hci_download_minidriver[] = { 0x01, 0x2e, 0xfc, 0x00 };
    const unsigned char hci_reset_cmd[] = {0x01, 0x03, 0x0C, 0x00};
    unsigned char hci_update_baud_rate[] = { 0x01, 0x18, 0xFC, 0x06,0x00, \
                                               0x00,0x00,0x00,0x00,0x00 };
    unsigned char hci_update_bd_addr[] = { 0x01, 0x01, 0xFC, 0x06,0x00, \
                                               0x00,0x00,0x00,0x00,0x00 };
    const char hci_uartclockset_cmd[] = {0x01, 0x45, 0xFC, 0x01, 0x01};

    unsigned char *buf = NULL;
    struct ktermios ktermios;
	if (!(buf = kmalloc(RESP_BUFF_SIZE, GFP_KERNEL))) {
        BT_LDISC_ERR("Unable to allocate memory for buf");
        err = -ENOMEM;
        goto error_state;
    }

    if (unlikely(hu == NULL || hu->tty == NULL)) {
        err = -EINVAL;
        goto error_state;
    }
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)
    memcpy(&ktermios, tty->termios, sizeof(ktermios));
#else
    memcpy(&ktermios, &(tty->termios), sizeof(ktermios));
#endif
    ptr = NULL;

    BT_LDISC_DBG(V4L2_DBG_INIT, "tty = %p hu = %p", tty,hu);

    /* request_firmware searches for patchram file. only in /vendor/firmware OR /etc/firmware */
    BT_LDISC_DBG(V4L2_DBG_INIT, "firmware patchram file: %s", fw_name);
    err = request_firmware(&hu->fw_entry, fw_name,
                 &hu->brcm_pdev->dev);

    /* patchram download failure */
    if ((unlikely((err != 0) || (hu->fw_entry->data == NULL) ||
         (hu->fw_entry->size == 0)))) {
         BT_LDISC_ERR("************* ERROR !!! Unable to download patchram \
            OR file %s not found **************", fw_name);
    }
    else {
        ptr = (void *)hu->fw_entry->data;
        len = hu->fw_entry->size;

        BT_LDISC_DBG(V4L2_DBG_INIT, " with header patchram data ptr %p, \
            len %ld ",ptr,len);

        /* write command for hci_download_minidriver. Perform this before patchram download */
        BT_LDISC_DBG(V4L2_DBG_INIT, "writing hci_download_minidriver");
        init_completion(&hu->cmd_rcvd);

        brcm_hci_write(hu, hci_download_minidriver, 4);

        /* delay before patchram download */
        msleep(50);

        /* start downloading firmware */
        do {
            buf[0] = 0x01;
            memcpy(buf+1, ptr, 3);
            ptr += 3;
            /* buf[3] holds the len of data to send */
            memcpy(buf+4, ptr, buf[3]);
            len -= buf[3] + 3;

            init_completion(&hu->cmd_rcvd);
            brcm_hci_write(hu, buf, buf[3] + 4);

            ptr += buf[3];

            if (!wait_for_completion_timeout
              (&hu->cmd_rcvd, msecs_to_jiffies(CMD_RESP_TIME))) {
                BT_LDISC_ERR(" waiting for download patchram command response \
                    - timed out ");
                return -ETIMEDOUT;
            }
        } while(len>0);

        BT_LDISC_DBG(V4L2_DBG_INIT, "fw patchram download complete");

        /* settlement delay */
        BT_LDISC_DBG(V4L2_DBG_INIT, "patchram_settlement_delay = %d", \
            patchram_settlement_delay);
        msleep(patchram_settlement_delay);

        /* firmware patchram download complete */
        release_firmware(hu->fw_entry);

        /* set baud rate to default */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)
        memcpy(&ktermios, tty->termios, sizeof(ktermios));
#else
        memcpy(&ktermios, &(tty->termios), sizeof(ktermios));
#endif
        BT_LDISC_DBG(V4L2_DBG_INIT, "before setting baudrate = %d\n",
                                     (speed_t)(ktermios.c_cflag & CBAUD));
        ktermios.c_cflag = (ktermios.c_cflag & ~CBAUD) | (B115200 & CBAUD);
        tty_set_termios(tty, &ktermios);

        msleep(20);

        BT_LDISC_DBG(V4L2_DBG_INIT, "after setting baudrate = %d\n",
                                           (speed_t)(ktermios.c_cflag & CBAUD));
    }

    /* Perform HCI Reset */
    init_completion(&hu->cmd_rcvd);
    BT_LDISC_DBG(V4L2_DBG_INIT, "Performing HCI Reset");
    brcm_hci_write(hu, hci_reset_cmd, 4);

    if (!wait_for_completion_timeout
          (&hu->cmd_rcvd, msecs_to_jiffies(CMD_RESP_TIME))) {
            pr_err(" waiting for HCI Reset command response - timed out ");
            err = -ETIMEDOUT;
            goto error_state;
    }
    BT_LDISC_DBG(V4L2_DBG_INIT, "%s HCI Reset complete", __func__);

    /* set UART clock rate to 48 MHz */
    if( custom_baudrate > CLOCK_SET_BAUDRATE){
        init_completion(&hu->cmd_rcvd);
        BT_LDISC_DBG(V4L2_DBG_INIT, "Baudrate > %ld UART clock set to 48 MHz",
                                                  (unsigned long)CLOCK_SET_BAUDRATE);
        brcm_hci_write(hu, hci_uartclockset_cmd, 5);

        if (!wait_for_completion_timeout
               (&hu->cmd_rcvd, msecs_to_jiffies(CMD_RESP_TIME))) {
            BT_LDISC_ERR(" waiting for UART clock set command response \
                - timed out");
            err = -ETIMEDOUT;
            goto error_state;
        }
    }

    /* set baud rate back to custom baudrate */
    init_completion(&hu->cmd_rcvd);
    BT_LDISC_DBG(V4L2_DBG_INIT,"set baud rate back to %ld", custom_baudrate);

    BRCM_encode_baud_rate(custom_baudrate, &hci_update_baud_rate[6]);

    brcm_hci_write(hu, hci_update_baud_rate, 10);

    if (!wait_for_completion_timeout
          (&hu->cmd_rcvd, msecs_to_jiffies(CMD_RESP_TIME))) {
            pr_err(" waiting for set baud rate back to %ld command response \
                - timed out", custom_baudrate);
            err = -ETIMEDOUT;
            goto error_state;
    }

    ktermios.c_cflag = (ktermios.c_cflag & ~CBAUD) |
        (baud_rates[lookup_baudrate(custom_baudrate)].termios_value & CBAUD);
    tty_set_termios(tty, &ktermios);
    BT_LDISC_DBG(V4L2_DBG_INIT, "after setting baudrate = %d\n",
                                          (speed_t)(ktermios.c_cflag & CBAUD));

    if(ControllerAddrRead){
        /*Read controller address and set*/
        pr_info("%s Read controller address and set", __func__);
        err = read_bd_addr(hu);
        if (err != 0) {
            pr_err("ldisc: failed to read local name for patchram");
            goto error_state;
        }
    }
    else
        str2bd(bd_addr,(bt_bdaddr_t*)bd_addr_array);
    BT_LDISC_DBG(V4L2_DBG_INIT, "BD ADDRESS going to  set is "\
        "%02X:%02X:%02X:%02X:%02X:%02X",
         bd_addr_array[0], bd_addr_array[1], bd_addr_array[2],
         bd_addr_array[3], bd_addr_array[4], bd_addr_array[5]);

    /* set BD address */
    init_completion(&hu->cmd_rcvd);
    BRCM_encode_bd_address(&hci_update_bd_addr[4]);

    brcm_hci_write(hu, hci_update_bd_addr, 10);
    if (!wait_for_completion_timeout
          (&hu->cmd_rcvd, msecs_to_jiffies(CMD_RESP_TIME))) {
            pr_err(" waiting for set bd addr command response \
                - timed out");
            err = -ETIMEDOUT;
            goto error_state;
    }
   BT_LDISC_DBG(V4L2_DBG_INIT, "BD ADDRESS set to "\
        "%02X:%02X:%02X:%02X:%02X:%02X",
        bd_addr_array[0], bd_addr_array[1], bd_addr_array[2],
        bd_addr_array[3], bd_addr_array[4], bd_addr_array[5]);

    /* Enable/Disable LPM should be configurable based on the module param */
    init_completion(&hu->cmd_rcvd);
    BT_LDISC_DBG(V4L2_DBG_INIT, "lpm param %s", lpm_param);
    str2arr(lpm_param, (uint8_t*) hci_writesleepmode_cmd, 16);

     BT_LDISC_DBG(V4L2_DBG_INIT,"LPM PARAM set to "\
        "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:"\
        "%02X:%02X:%02X:%02X:%02X:%02X:%02X",
     hci_writesleepmode_cmd[0], hci_writesleepmode_cmd[1], hci_writesleepmode_cmd[2],
     hci_writesleepmode_cmd[3],hci_writesleepmode_cmd[4], hci_writesleepmode_cmd[5],
     hci_writesleepmode_cmd[6],hci_writesleepmode_cmd[7], hci_writesleepmode_cmd[8],
     hci_writesleepmode_cmd[9],hci_writesleepmode_cmd[10], hci_writesleepmode_cmd[11],
     hci_writesleepmode_cmd[12],hci_writesleepmode_cmd[13],hci_writesleepmode_cmd[14],
     hci_writesleepmode_cmd[15]);

    brcm_hci_write(hu, hci_writesleepmode_cmd, 16);
    if (!wait_for_completion_timeout
          (&hu->cmd_rcvd, msecs_to_jiffies(CMD_RESP_TIME))) {
            pr_err(" waiting for set LPM command response timed out");
            err = -ETIMEDOUT;
            goto error_state;
    }

    err = 0;

error_state:
    if (buf != NULL) kfree(buf);
    return err;
}


/**
 * brcm_sh_ldisc_stop - called  on the last un-registration */

long brcm_sh_ldisc_stop(struct hci_uart *hu)
{
    long err = 0;

    INIT_COMPLETION(hu->ldisc_installed);

    brcm_btsleep_stop(sleep);
    hu->ldisc_install = V4L2_STATUS_OFF;

    /* send uninstall notification to UIM */
    sysfs_notify(&hu->brcm_pdev->dev.kobj, NULL, "install");

    /* wait for ldisc to be un-installed */
    err = wait_for_completion_timeout(&hu->ldisc_installed,
            msecs_to_jiffies(LDISC_TIME));
    if (!err) {     /* timeout */
        BT_LDISC_ERR(" timed out waiting for ldisc to be un-installed");
        return -ETIMEDOUT;
    }

    return err;
}

/**
 *  This involves  reading the firmware version from chip,
 *    forming the fw file name
 *  based on the chip version, requesting the fw, parsing it
 *  and perform download(send/recv).
 */
long brcm_sh_ldisc_start(struct hci_uart *hu)
{
    long err = 0;
    long retry = POR_RETRY_COUNT;
    long cl_err = 0;

    struct tty_struct *tty = hu->tty;

    BT_LDISC_DBG(V4L2_DBG_INIT, " %p",tty);

    do {
        brcm_btsleep_start(sleep);
        INIT_COMPLETION(hu->ldisc_installed);
        /* send notification to UIM */
        hu->ldisc_install = V4L2_STATUS_ON;
        BT_LDISC_DBG(V4L2_DBG_INIT, "ldisc_install = %c",\
            hu->ldisc_install);
        sysfs_notify(&hu->brcm_pdev->dev.kobj,
            NULL, "install");

        /* wait for ldisc to be installed */
        err = wait_for_completion_timeout(&hu->ldisc_installed,
                msecs_to_jiffies(LDISC_TIME));
        if (!err) { /* timeout */
            pr_err("line disc installation timed out ");
            INIT_COMPLETION(hu->tty_close_complete);
            err = brcm_sh_ldisc_stop(hu);
            cl_err = wait_for_completion_timeout(&hu->tty_close_complete,
                    msecs_to_jiffies(TTY_CLOSE_TIME));
            if (!cl_err) { /* timeout */
                pr_err("tty close timed out");
                break;
            }
            continue;
        } else {
            /* ldisc installed now */
            BT_LDISC_DBG(V4L2_DBG_INIT, " line discipline installed ");
            err = download_patchram(hu);
            if (err != 0) {
                pr_err("patchram download failed");
                INIT_COMPLETION(hu->tty_close_complete);
                brcm_sh_ldisc_stop(hu);
                cl_err = wait_for_completion_timeout(&hu->tty_close_complete,
                        msecs_to_jiffies(TTY_CLOSE_TIME));
                if (!cl_err) { /* timeout */
                    pr_err("tty close timed out");
                    break;
                }
                continue;
            } else {/* on success don't retry */
                BT_LDISC_DBG(V4L2_DBG_INIT, "patchram downloaded successfully");
                // initialize lock for err flags
                spin_lock_init(&hu->err_lock);
                break;
            }
        }
    } while (retry--);

    return err;
}


/*******************************************************************************
**
** Function - brcm_sh_ldisc_write()
**
** Description - Function to write to the shared line discipline driver
**              called from upper layer protocol stack drivers (BT or FM)
**              via the write function pointer
**
** Returns - 0 if success; errno otherwise
**
*******************************************************************************/
long brcm_sh_ldisc_write(struct sk_buff *skb)
{
    enum proto_type protoid = PROTO_SH_MAX;
    long len;
    struct brcm_struct *brcm;
    char ptr[6];

    struct hci_uart *hu;
    hu_ref(&hu,0);

    brcm = hu->priv;


    BT_LDISC_DBG(V4L2_DBG_TX, "%p",hu);

    if (unlikely(skb == NULL))
    {
        pr_err("data unavailable to perform write");
        return -1;
    }

    if (unlikely(hu == NULL || hu->tty == NULL))
    {
        pr_err("tty unavailable to perform write");
        return -1;
    }

    switch (sh_ldisc_cb(skb)->pkt_type)
    {
        case HCI_COMMAND_PKT:
        case HCI_ACLDATA_PKT:
        case HCI_SCODATA_PKT:
            protoid = PROTO_SH_BT;
            break;
#ifdef V4L2_ANT
       case ANT_PKT:
            protoid = PROTO_SH_ANT;
            break;
#endif
        case FM_CH8_PKT:
            protoid = PROTO_SH_FM;
            break;
    }

    if (unlikely(hu->list[protoid] == NULL))
    {
        pr_err(" protocol %d not registered, and writing? ",
                            protoid);
        return -1;
    }

    len = skb->len;

    if ((hu->is_registered[PROTO_SH_ANT] || hu->is_registered[PROTO_SH_FM])
            && (skb->data)[1] == 0x03 && (skb->data)[2] == 0x0c)
    {
        if (likely(hu->list[PROTO_SH_BT]->recv != NULL))
        {
            brcm->rx_skb = alloc_skb(HCI_MAX_FRAME_SIZE, GFP_ATOMIC);
            if(brcm->rx_skb)
                skb_reserve(brcm->rx_skb,8);

            brcm->rx_skb->dev = (void *) hu->hdev;
            sh_ldisc_cb(brcm->rx_skb)->pkt_type = HCI_EVENT_PKT;

            ptr[0] = 0x0e;
            ptr[1] = 0x04;
            ptr[2] = 0x01;
            ptr[3] = 0x03;
            ptr[4] = 0x0c;
            ptr[5] = 0x00;

            memcpy(skb_put(brcm->rx_skb, 6), ptr, 6);

            brcm_hci_process_frametype(HCI_EVENT_PKT,hu,brcm->rx_skb, 6);
            brcm_hci_uart_route_frame(PROTO_SH_BT, hu, brcm->rx_skb);
        }
    }
    else
    {
        if (hu->protos_registered > 1 &&
           (sh_ldisc_cb(skb)->pkt_type == HCI_COMMAND_PKT ||
            sh_ldisc_cb(skb)->pkt_type == FM_CH8_PKT ||
            sh_ldisc_cb(skb)->pkt_type == ANT_PKT))
        {
            mutex_lock(&cmd_credit);
            init_completion(&hu->cmd_rcvd);

            hu->proto->enqueue(hu, skb);

            /* forward to snoop */
#if V4L2_SNOOP_ENABLE
            if(nl_sk_hcisnoop)
                brcm_hci_write(hu, skb->data, skb->len);
#endif
            brcm_hci_uart_tx_wakeup(hu);
            if (!wait_for_completion_timeout(&hu->cmd_rcvd, msecs_to_jiffies(5000))) {
                pr_err(" waiting for command response - timed out");
                return 0;
            }
            mutex_unlock(&cmd_credit);
        }
        else
        {
            hu->proto->enqueue(hu, skb);

            /* forward to snoop */
#if V4L2_SNOOP_ENABLE
            if(nl_sk_hcisnoop)
                brcm_hci_write(hu, skb->data, skb->len);
#endif
            brcm_hci_uart_tx_wakeup(hu);
        }
    }

    /* return number of bytes written */
    return len;
}

/*****************************************************************************
**   Shared line discipline driver APIs
*****************************************************************************/
/*******************************************************************************
**
** Function - hci_uart_tty_open
**
** Description - Called when line discipline changed to HCI_UART.
**
** Arguments - tty    pointer to tty info structure
**
** Returns - 0 if success, otherwise error code
**
*******************************************************************************/
static int brcm_hci_uart_tty_open(struct tty_struct *tty)
{
    struct hci_uart *hu;
    unsigned long flags;
#if V4L2_SNOOP_ENABLE
    hc_bt_hdr *snoop_hci_hdr;
    hc_bt_hdr *snoop_hci_recv_hdr;
#endif
    hu_ref(&hu, 0);
    BT_LDISC_DBG(V4L2_DBG_INIT, "tty open %p hu %p", tty,hu);

    /* don't do an wakeup for now */
    clear_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);

    tty->disc_data = hu;
    hu->tty = tty;
    BT_LDISC_DBG(V4L2_DBG_INIT, "tty open %p hu %p", tty,hu);
    tty->receive_room = N_TTY_BUF_SIZE;

    spin_lock_irqsave(&reg_lock, flags);
    spin_unlock_irqrestore(&reg_lock, flags);

    spin_lock_init(&hu->rx_lock);
    spin_lock_init(&hu->lock);

#if V4L2_SNOOP_ENABLE
    spin_lock_init(&hu->hcisnoop_lock);
    spin_lock_init(&hu->hcisnoop_write_lock);
#endif

    /* Flush any pending characters in the driver and line discipline. */
    if (tty->ldisc->ops->flush_buffer)
        tty->ldisc->ops->flush_buffer(tty);
    tty_driver_flush_buffer(tty);

#if V4L2_SNOOP_ENABLE
    /* used to create header in snoop */
    snoop_hci_hdr = kzalloc(V4L2_HCI_PKT_MAX_SIZE, GFP_ATOMIC);
    if (!snoop_hci_hdr) {
        BT_LDISC_ERR("could not allocate memory to snoop_hci_hdr");
        return -ENOMEM;
    }
    else {
        hu->snoop_hdr_data = snoop_hci_hdr;
    }

    /* used to create header for snoop during internal command response */
    snoop_hci_recv_hdr = kzalloc(V4L2_HCI_PKT_MAX_SIZE, GFP_ATOMIC);
    if (!snoop_hci_recv_hdr) {
        BT_LDISC_ERR("could not allocate memory to snoop_hci_hdr");
        return -ENOMEM;
    }
    else {
        hu->hdr_data = snoop_hci_recv_hdr;
    }
#endif

   /* installation of N_BRCM_HCI ldisc complete */
    sh_ldisc_complete(hu);

    return 0;
}

/*******************************************************************************
**
** Function - hci_uart_tty_close
**
** Description - Called when the line discipline is changed to something
**              else, the tty is closed, or the tty detects a hangup.
**
** Arguments - tty    pointer to associated tty instance data
**
** Returns - void
**
*******************************************************************************/
static void brcm_hci_uart_tty_close(struct tty_struct *tty)
{
    struct hci_uart *hu = (void *)tty->disc_data; /* assign tty instance to hci_uart */
    int i;

    BT_LDISC_DBG(V4L2_DBG_INIT, "tty= %p", tty);

    /* Detach from the tty */
    tty->disc_data = NULL;

    tty_ldisc_flush(tty);

    if (tty->ldisc->ops->flush_buffer)
        tty->ldisc->ops->flush_buffer(tty);

    tty_driver_flush_buffer(tty);

#if V4L2_SNOOP_ENABLE
    /* release memory allocated for snooping */
    if(hu->snoop_hdr_data){
        kfree(hu->snoop_hdr_data);
        hu->snoop_hdr_data = NULL;
    }
    if(hu->hdr_data){
        kfree(hu->hdr_data);
        hu->hdr_data = NULL;
    }
#endif

    if (hu)
    {
        if (test_and_clear_bit(HCI_UART_PROTO_SET, &hu->flags))
        {
            hu->proto->close(hu);
            hu->protos_registered =0;
            BT_LDISC_DBG(V4L2_DBG_CLOSE, "brcm_hci_uart_tty_close protos_registered = %d",
                                     hu->protos_registered);
            for (i = PROTO_SH_BT; i < PROTO_SH_MAX; i++)
            {
                if (hu->list[i] != NULL)
                {
                    BT_LDISC_ERR("%d not un-registered, unregistering now", i);
                    remove_channel_from_table(hu,i);
                }
            }
        }
    }
    sh_ldisc_complete(hu);
    hu->tty = 0;
    complete(&hu->tty_close_complete);
    BT_LDISC_DBG(V4L2_DBG_INIT, "tty close exit");
}

/*******************************************************************************
**
** Function - hci_uart_tty_wakeup
**
** Description - Callback for transmit wakeup. Called when low level
**              device driver can accept more send data.
**
** Arguments - tty    pointer to associated tty instance data
**
** Returns - void
**
*******************************************************************************/
static void brcm_hci_uart_tty_wakeup(struct tty_struct *tty)
{
    struct hci_uart *hu = (void *)tty->disc_data; /* assign tty instance to hci_uart */

    if (!hu)
        return;

    if (tty != hu->tty)
        return;

    clear_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);
}

/*******************************************************************************
**
** Function - hci_uart_tty_receive
**
** Description - Called by tty low level driver when receive data is available.
**
* Arguments - tty          pointer to tty isntance data
*             data         pointer to received data
*             flags        pointer to flags for data
*             count        count of received data in bytes
** Returns - void
**
*******************************************************************************/
static void brcm_hci_uart_tty_receive(struct tty_struct *tty,
                                         const u8 *data, char *flags, int count)
{
    struct hci_uart *hu = (void *)tty->disc_data; /* assign tty instance to hci_uart */
    unsigned long lock_flags;

    if (!hu || tty != hu->tty)
        return;

    if (!test_bit(HCI_UART_PROTO_SET, &hu->flags))
        return;

    spin_lock_irqsave(&hu->rx_lock, lock_flags);

    hu->proto->recv(hu, (void *) data, count);

    spin_unlock_irqrestore(&hu->rx_lock, lock_flags);

}

/*******************************************************************************
**
** Function - hci_uart_tty_ioctl
**
** Description - Process IOCTL system call for the tty device.
**
** Arguments - tty  pointer to tty instance data
**    file          pointer to open file object for device
**    cmd           IOCTL command code
**    arg           argument for IOCTL call (cmd dependent)
*** Returns - 0 if success, otherwise error code
**
*******************************************************************************/
static int brcm_hci_uart_tty_ioctl(struct tty_struct *tty,
                        struct file * file, unsigned int cmd, unsigned long arg)
{
    struct hci_uart *hu = (void *)tty->disc_data; /* assign tty instance to hci_uart */
    int err = 0;

    BT_LDISC_DBG(V4L2_DBG_INIT,"cmd %d", cmd);

    /* Verify the status of the device */
    if (!hu)
        return -EBADF;

    switch (cmd)
        {
        case HCIUARTSETPROTO:
            BT_LDISC_DBG(V4L2_DBG_INIT, "SETPROTO %lu hu %p", arg, hu);
            if (!test_and_set_bit(HCI_UART_PROTO_SET, &hu->flags))
            {
                err = brcm_hci_uart_set_proto(hu, arg);
                if (err) {
                    pr_err("error set proto");
                    clear_bit(HCI_UART_PROTO_SET, &hu->flags);
                    return err;
                }
            } else
                return -EBUSY;
            break;

        case HCIUARTGETPROTO:
            BT_LDISC_DBG(V4L2_DBG_INIT, "GETPROTO");
            if (test_bit(HCI_UART_PROTO_SET, &hu->flags))
                return hu->proto->id;
            return -EUNATCH;

        case HCIUARTGETDEVICE:
            BT_LDISC_DBG(V4L2_DBG_INIT,"GETDEVICE");
            if (test_bit(HCI_UART_PROTO_SET, &hu->flags))
                return hu->hdev->id;
            return -EUNATCH;

        default:
            err = n_tty_ioctl_helper(tty, file, cmd, arg);
            break;
    }

    return err;
}

/*******************************************************************************
**
** Function - brcm_hci_uart_tty_read
**
** Description - Read interface for sh_ldisc driver. Not supported
**
*******************************************************************************/
static ssize_t brcm_hci_uart_tty_read(struct tty_struct *tty,
       struct file *file, unsigned char __user *buf, size_t nr)
{
    BT_LDISC_DBG(V4L2_DBG_RX, "%s", __func__);

return 0;
}

/*******************************************************************************
**
** Function - brcm_hci_uart_tty_read
**
** Description - Read interface for sh_ldisc driver. Not supported
**
*******************************************************************************/
static ssize_t brcm_hci_uart_tty_write(struct tty_struct *tty,
                     struct file *file, const unsigned char *data, size_t count)
{
    BT_LDISC_DBG(V4L2_DBG_TX, "%s", __func__);

    return 0;
}

/*******************************************************************************
**
** Function - brcm_hci_uart_tty_read
**
** Description - Read interface for sh_ldisc driver. Not supported
**
*******************************************************************************/
static unsigned int brcm_hci_uart_tty_poll(struct tty_struct *tty,
                    struct file *filp, poll_table *wait)
{
    BT_LDISC_DBG(V4L2_DBG_RX, "%s", __func__);

    return 0;
}
static void brcm_hci_uart_tty_set_termios(struct tty_struct *tty,
                                                   struct ktermios *new_termios)
{
    struct ktermios *newktermios;
    BT_LDISC_DBG(V4L2_DBG_INIT, "new_termios->c_ispeed = %d, "\
        "new_termios->c_ospeed = %d",
        new_termios->c_ispeed, new_termios->c_ospeed);
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)
    newktermios =  tty->termios;
#else
    newktermios =  &(tty->termios);
#endif
    newktermios->c_ispeed= tty_termios_input_baud_rate(newktermios);
    newktermios->c_ospeed = tty_termios_baud_rate(newktermios);
}

/*****************************************************************************
**   Module INIT interface
*****************************************************************************/
static int brcm_hci_uart_init( void )

{
    static struct tty_ldisc_ops hci_uart_ldisc;
    int err;

    BT_LDISC_DBG(V4L2_DBG_INIT, "HCI BRCM UART driver ver %s", VERSION);

    /* Register the tty discipline */

    memset(&hci_uart_ldisc, 0, sizeof (hci_uart_ldisc));
    hci_uart_ldisc.magic        = TTY_LDISC_MAGIC;
    hci_uart_ldisc.name     = "n_brcm_hci";
    hci_uart_ldisc.open     = brcm_hci_uart_tty_open;
    hci_uart_ldisc.close        = brcm_hci_uart_tty_close;
    hci_uart_ldisc.read     = brcm_hci_uart_tty_read;
    hci_uart_ldisc.write        = brcm_hci_uart_tty_write;
    hci_uart_ldisc.ioctl        = brcm_hci_uart_tty_ioctl;
    hci_uart_ldisc.poll     = brcm_hci_uart_tty_poll;
    hci_uart_ldisc.receive_buf  = brcm_hci_uart_tty_receive;
    hci_uart_ldisc.write_wakeup = brcm_hci_uart_tty_wakeup;
    hci_uart_ldisc.owner        = THIS_MODULE;
    hci_uart_ldisc.set_termios = brcm_hci_uart_tty_set_termios;

    if ((err = tty_register_ldisc(N_BRCM_HCI, &hci_uart_ldisc)))
    {
        pr_err("HCI line discipline registration failed. (%d)", err);
        return err;
    }

#ifdef CONFIG_BT_HCIUART_H4
    h4_init();
#endif
#ifdef CONFIG_BT_HCIUART_BCSP
    bcsp_init();
#endif
#ifdef CONFIG_BT_HCIUART_LL
    ll_init();
#endif
#ifdef CONFIG_BT_HCIUART_BRCM
    brcm_init();
#endif

    /* initialize register lock spinlock */
    spin_lock_init(&reg_lock);

    return 0;
}


/*****************************************************************************
**   Module EXIT interface
*****************************************************************************/
static void brcm_hci_uart_exit(struct hci_uart* hu)
{
    int err = 0;
    BT_LDISC_DBG(V4L2_DBG_INIT, "%s", __func__);

#ifdef CONFIG_BT_HCIUART_H4
    h4_deinit();
#endif
#ifdef CONFIG_BT_HCIUART_BCSP
    bcsp_deinit();
#endif
#ifdef CONFIG_BT_HCIUART_LL
    ll_deinit();
#endif
#ifdef CONFIG_BT_HCIUART_BRCM
    brcm_deinit();
#endif

    kfree_skb(hu->tx_skb);
    /* Release tty registration of line discipline */
    if ((err = tty_unregister_ldisc(N_BRCM_HCI)))
        BT_LDISC_ERR("Can't unregister HCI line discipline (%d)", err);
}

/**
 * ldisc_get_plat_device -
 *  function which returns the reference to the platform device
 *  requested by id. As of now only 1 such device exists (id=0)
 *  the context requesting for reference can get the id to be
 *  requested by a. The protocol driver which is registering or
 *  b. the tty device which is opened.
 */
static struct platform_device *ldisc_get_plat_device(int id)
{
    return brcm_plt_devices[id];
}


/**
 * hu_ref - reference the core's data
 *  This references the per-ST platform device in the arch/xx/
 *  board-xx.c file.
 *  This would enable multiple such platform devices to exist
 *  on a given platform
 */
void hu_ref(struct hci_uart**priv, int id)
{
    struct platform_device  *pdev;
    struct hci_uart *hu;
    /* get hu reference from platform device */
    pdev = ldisc_get_plat_device(id);
    if (!pdev) {
        *priv = NULL;
        return;
    }
    hu = dev_get_drvdata(&pdev->dev);
    *priv = hu;
}

/**************************************************************
* bcmbt_ldisc_probe -
* Used for checking whether specified device hardware exists.
***************************************************************/
static int bcmbt_ldisc_probe(struct platform_device *pdev)
{

    int rc;
    struct hci_uart *hu;
    printk("%s\n",  __func__);
    BT_LDISC_DBG(V4L2_DBG_INIT, "%s", __func__);
    mutex_init(&cmd_credit);

    if ((pdev->id != -1) && (pdev->id < MAX_BRCM_DEVICES)) {
        /* multiple devices could exist */
        brcm_plt_devices[pdev->id] = pdev;
    } else {
        /* platform's sure about existence of 1 device */
        brcm_plt_devices[0] = pdev;
    }

    hu = kzalloc(sizeof(struct hci_uart), GFP_ATOMIC);
    memset(hu, 0, sizeof(struct hci_uart));
    if (!hu) {
        pr_err("no mem to allocate");
        return -ENOMEM;
    }
    dev_set_drvdata(&pdev->dev, hu);

    BT_LDISC_DBG(V4L2_DBG_INIT, "%s calling brcm_hci_uart_init ", __func__);

    /* register the tty line discipline driver */
    rc = brcm_hci_uart_init();
    if (rc) {
        pr_err("%s: brcm_hci_uart_init failed\n", __func__);
        return -1;
    }

    /* get reference of pdev for request_firmware
     */
    hu->brcm_pdev = pdev;
    init_completion(&hu->cmd_rcvd);
    init_completion(&hu->ldisc_installed);
    init_completion(&hu->tty_close_complete);

    rc = sysfs_create_group(&pdev->dev.kobj, &uim_attr_grp);
    if (rc) {
        pr_err("failed to create sysfs entries");
        return rc;
    }

    return 0;
}

static int bcmbt_ldisc_remove(struct platform_device *pdev)
{
    struct hci_uart* hu;
    BT_LDISC_DBG(V4L2_DBG_INIT, "bcmbt_ldisc_remove() unloading ");
    mutex_destroy(&cmd_credit);
#if V4L2_SNOOP_ENABLE
    if(ldisc_snoop_enable_param)
    {
        /* stop hci snoop to hcidump */
        if(nl_sk_hcisnoop)
            netlink_kernel_release(nl_sk_hcisnoop);
    }
#endif
    hu = dev_get_drvdata(&pdev->dev);
    sysfs_remove_group(&pdev->dev.kobj, &uim_attr_grp);
    brcm_hci_uart_exit(hu);
    kfree(hu);
    hu  = NULL;
    return 0;
}

static const struct of_device_id bcmbt_ldisc_dt_ids[] = {
    { .compatible = "bcmbt_ldisc"},
    {}
};

/* Sys_fs entry bcm_ldisc. The Line discipline driver sets this to 1 when bluedroid performs open on BT
  * protocol driver or application performs open on FM v4l2 driver */
/* Note: This entry is used in bt_hci_bdroid.c (Android source). Also present in
 *  brcm_sh_ldisc.c (v4l2_drivers) and board specific file (android kernel source) */
static struct platform_driver bcmbt_ldisc_platform_driver = {
    .probe = bcmbt_ldisc_probe,
    .remove = bcmbt_ldisc_remove,
    .driver = {
           .name = "bcm_ldisc",
           .owner = THIS_MODULE,
           .of_match_table = bcmbt_ldisc_dt_ids,
           },
};

module_platform_driver(bcmbt_ldisc_platform_driver);


/*****************************************************************************
**   Module Details
*****************************************************************************/

MODULE_PARM_DESC(reset, "Send HCI reset command on initialization");

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Line Discipline Driver for Shared Transport" \
                                                      "over UART ver ");
MODULE_VERSION(VERSION); /* defined in makefile */
MODULE_LICENSE("GPL");
MODULE_ALIAS_LDISC(N_BRCM_HCI);
