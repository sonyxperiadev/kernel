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
**  Name:           brcm_hci_uart.h
**
**  Description:    This is the header file for.Bluetooth HCI UART driver
**
*****************************************************************************/

#ifndef BRCM_HCI_UART_H
#define BRCM_HCI_UART_H

#include <linux/platform_device.h>
#include <linux/firmware.h>
#include "../include/brcm_ldisc_sh.h"
#include "../include/v4l2_target.h"

#define TRUE 1
#define FALSE 0

/*****************************************************************************
**  Constants & Macros
*****************************************************************************/

#ifndef N_BRCM_HCI
#define N_BRCM_HCI      25
#endif

/* Ioctls */
#define HCIUARTSETPROTO     _IOW('U', 200, int)
#define HCIUARTGETPROTO     _IOR('U', 201, int)
#define HCIUARTGETDEVICE    _IOR('U', 202, int)

/* UART protocols */
#define HCI_UART_MAX_PROTO      5

/* #define HCI_UART_H4    0*/
#define HCI_UART_BCSP           1
#define HCI_UART_3WIRE          2
#define HCI_UART_H4DS           3
#define HCI_UART_LL             4
#define HCI_UART_BRCM           0

/* HCI_UART flag bits */
#define HCI_UART_PROTO_SET      0

/* TX states  */
#define HCI_UART_SENDING        1
#define HCI_UART_TX_WAKEUP      2

#define LOCAL_NAME_BUFFER_LEN  32
#define UART_DEV_NAME_LEN      32
#define VENDOR_PARAMS_LEN 300

/* time in msec to wait for
 * line discipline to be installed
 */
#define LDISC_TIME             1500
#define CMD_RESP_TIME          800
#define CMD_WR_TIME            5000
#define POR_RETRY_COUNT        5


/* states of protocol list */
#define LDISC_NOTEMPTY          1
#define LDISC_EMPTY             0

/*
 * possible st_states
 */
#define LDISC_INITIALIZING      1
#define LDISC_REG_IN_PROGRESS   2
#define LDISC_REG_PENDING       3
#define LDISC_WAITING_FOR_RESP  4

/* HCI pkt event and command codes */
#define HCI_FM_PKT           0x08

/* Max size of HCI packet */
#define V4L2_HCI_PKT_MAX_SIZE 1050

/* time for workaround in msec to wait for closed tty */
#define TTY_CLOSE_TIME          20000

struct brcm_struct {
    unsigned long rx_state;
    unsigned long rx_count;
    struct sk_buff *rx_skb;
    struct sk_buff_head txq;
    spinlock_t hcibrcm_lock;          /* HCIBRCM state lock */
    unsigned long hcibrcm_state;      /* HCIBRCM power state */
    unsigned short is_there_activity;
    unsigned short inactive_period;
    struct sk_buff_head tx_wait_q;    /* HCIBRCM wait queue */

    void* hu;
    char resp_buffer[30];
};


typedef struct hci_uart_proto hci_uart_proto;
typedef struct hci_uart hci_uart;

struct hci_uart_proto {
    unsigned int id;
    int (*open)(struct hci_uart *hu);
    int (*close)(struct hci_uart *hu);
    int (*flush)(struct hci_uart *hu);
    int (*recv)(struct hci_uart *hu, void *data, int len);
    int (*recv_int)(struct hci_uart *hu, void *data, int len);
    int (*enqueue)(struct hci_uart *hu, struct sk_buff *skb);
    struct sk_buff *(*dequeue)(struct hci_uart *hu);
};

struct hci_uart {
    struct tty_struct *tty;
    struct hci_dev *hdev;
    unsigned long flags;

    struct hci_uart_proto *proto;
    struct brcm_struct *priv;
    void *hdr_data;

#if V4L2_SNOOP_ENABLE
    void *snoop_hdr_data;
#endif

    struct sk_buff        *tx_skb;
    unsigned long sh_ldisc_state;
    unsigned long        tx_state;
    spinlock_t        rx_lock;
    struct sh_proto_s *list[PROTO_SH_MAX];
    bool is_registered[PROTO_SH_MAX];
    unsigned char    protos_registered;
    spinlock_t lock;

    struct completion cmd_rcvd, *ldisc_installed, ldisc_patchram_complete,
                uim_baudrate_set_complete;
    char resp_buffer[30];
    struct platform_device *brcm_pdev;
    const struct firmware *fw_entry;

    unsigned char ldisc_install;
    unsigned char ldisc_bt_err;
    unsigned char ldisc_fm_err;
    spinlock_t err_lock;

    //vendor params as comma separated string , read from bt_vendor.conf
    char vendor_params[VENDOR_PARAMS_LEN];

#if V4L2_SNOOP_ENABLE
    spinlock_t hcisnoop_lock;
    spinlock_t hcisnoop_write_lock;
#endif
    struct completion *tty_close_complete;
};

typedef struct {
    unsigned short          event;
    unsigned short          len;
    unsigned short          offset;
    unsigned short          layer_specific;
} hc_bt_hdr;


/*****************************************************************************
**  Functions
*****************************************************************************/
int brcm_init(void);
int brcm_deinit(void);

/*****************************************************************************
**  Functions
*****************************************************************************/

int brcm_hci_uart_register_proto(struct hci_uart_proto *p);
int brcm_hci_uart_unregister_proto(struct hci_uart_proto *p);
int brcm_hci_uart_tx_wakeup(struct hci_uart *hu);
void brcm_hci_uart_route_frame(enum proto_type protoid,
                                      struct hci_uart *hu, struct sk_buff *skb);
void brcm_hci_process_frametype( register int frame_type,
                               struct hci_uart*hu,struct sk_buff *skb,int count);

/**
 * sh_ldisc_write -
 * internal write function, passed onto protocol drivers
 * via the write function ptr of protocol struct
 */
long brcm_sh_ldisc_write(struct sk_buff *);

/**
 * sh_ldisc_init -
 * internal init function, passed onto protocol drivers
 * via the init function ptr of protocol struct
 */
/* ask for reference from KIM */
void hu_ref(struct hci_uart **, int);
long brcm_sh_ldisc_start(struct hci_uart *hu);
long brcm_sh_ldisc_stop(struct hci_uart *hu);

#ifdef CONFIG_BT_HCIUART_H4
int h4_init(void);
int h4_deinit(void);
#endif

#ifdef CONFIG_BT_HCIUART_BCSP
int bcsp_init(void);
int bcsp_deinit(void);
#endif

#ifdef CONFIG_BT_HCIUART_LL
int ll_init(void);
int ll_deinit(void);
#endif

#ifdef CONFIG_BT_HCIUART_BRCM
int brcm_init(void);
int brcm_deinit(void);
#endif

#endif
