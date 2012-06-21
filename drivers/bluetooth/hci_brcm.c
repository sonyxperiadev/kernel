/****************************************************************************
*
*	Copyright (c) 1999-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*   Notwithstanding the above, under no circumstances may you combine this
*   software in any way with any other Broadcom software provided under a
*   license other than the GPL, without Broadcom's express prior written
*   consent.
*
****************************************************************************/

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
#include <linux/gpio.h>

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
#include "hci_uart.h"

#include <linux/broadcom/bcm_bzhw.h>

#if 0
#ifndef CONFIG_BT_HCIUART_DEBUG
#undef  BT_DBG
#define BT_DBG(A...)
#endif
#endif

/* HCIBRCM commands */
#define HCIBRCM_GO_TO_SLEEP_IND	0x30
#define HCIBRCM_GO_TO_SLEEP_ACK	0x31
#define HCIBRCM_WAKE_UP_IND	0x32
#define HCIBRCM_WAKE_UP_ACK	0x33

/* HCIBRCM receiver States */
#define HCIBRCM_W4_PACKET_TYPE	0
#define HCIBRCM_W4_EVENT_HDR	1
#define HCIBRCM_W4_ACL_HDR	2
#define HCIBRCM_W4_SCO_HDR	3
#define HCIBRCM_W4_DATA		4

#define TIMER_PERIOD 100	/* 100 ms */
#define HOST_CONTROLLER_IDLE_TSH 500	/* 4 s */

void assert_bt_wake(int bt_wake, struct pi_mgr_qos_node *lqos_node,
			struct tty_struct *tty)
{
	bcm_bzhw_assert_bt_wake(bt_wake, lqos_node, tty);
}

void deassert_bt_wake(int bt_wake, int host_wake)
{
	bcm_bzhw_deassert_bt_wake(bt_wake, host_wake);

}

/* HCIBRCM states */
enum hcibrcm_states_e {
	HCIBRCM_ASLEEP,
	HCIBRCM_ASLEEP_TO_AWAKE,
	HCIBRCM_AWAKE,
	HCIBRCM_AWAKE_TO_ASLEEP
};

struct hcibrcm_cmd {
	u8 cmd;
} __attribute__ ((packed));

struct brcm_struct {
	unsigned long rx_state;
	unsigned long rx_count;
	struct sk_buff *rx_skb;
	struct sk_buff_head txq;
	spinlock_t hcibrcm_lock;	/* HCIBRCM state lock   */
	unsigned long hcibrcm_state;	/* HCIBRCM power state  */
	unsigned short is_there_activity;
	unsigned short inactive_period;
	struct sk_buff_head tx_wait_q;	/* HCIBRCM wait queue   */
	int btwake_gpio;
	int hostwake_gpio;
	struct pi_mgr_qos_node *lqos_node;
};

static struct timer_list sleep_timer;
struct bcmbzhw_struct *hw_struct;

/**
 * timeout handler
*/
void sleep_timer_function(unsigned long data)
{
	struct brcm_struct *lbrcm = (struct brcm_struct *)data;
	if (lbrcm->is_there_activity) {
		lbrcm->inactive_period = 0;
		lbrcm->is_there_activity = 0;
	} else {
		/*if (lbrcm->hcibrcm_state != HCIBRCM_ASLEEP) {*/
		lbrcm->inactive_period += TIMER_PERIOD;
		if (lbrcm->inactive_period >= HOST_CONTROLLER_IDLE_TSH) {
			/* deassert BT_WAKE signal*/
			BT_DBG("Deassert wake signal, moves to ASLEEP");
			lbrcm->hcibrcm_state = HCIBRCM_ASLEEP;
			lbrcm->inactive_period = 0;
			deassert_bt_wake(lbrcm->btwake_gpio,
			lbrcm->hostwake_gpio);
		}
		/*}*/
	}
	if (lbrcm->hcibrcm_state != HCIBRCM_ASLEEP) {
		mod_timer(&sleep_timer,
			  jiffies + msecs_to_jiffies(TIMER_PERIOD));
	}
}



static void __brcm_bcm_serial_clock_on(struct hci_uart *hu)
{
	struct brcm_struct *brcm = hu->priv;
	bcm_bzhw_request_clock_on(brcm->lqos_node);
}

static void __brcm_bcm_serial_clock_request_off(struct hci_uart *hu)
{
	struct brcm_struct *brcm = hu->priv;
	bcm_bzhw_request_clock_off(brcm->lqos_node);
}


/*
 * Builds and sends an HCIBRCM command packet.
 * These are very simple packets with only 1 cmd byte
 */
static int send_hcibrcm_cmd(u8 cmd, struct hci_uart *hu)
{
	int err = 0;
	struct sk_buff *skb = NULL;
	struct brcm_struct *brcm = hu->priv;
	struct hcibrcm_cmd *hcibrcm_packet;

	BT_DBG("hu %p cmd 0x%x", hu, cmd);

	/* allocate packet */
	skb = bt_skb_alloc(1, GFP_ATOMIC);
	if (!skb) {
		BT_ERR("cannot allocate memory for HCIBRCM packet");
		err = -ENOMEM;
		goto out;
	}

	/* prepare packet */
	hcibrcm_packet = (struct hcibrcm_cmd *)skb_put(skb, 1);
	hcibrcm_packet->cmd = cmd;
	skb->dev = (void *)hu->hdev;

	/* send packet */
	skb_queue_tail(&brcm->txq, skb);
out:
	return err;
}

/* Initialize protocol */
static int brcm_open(struct hci_uart *hu)
{
	struct brcm_struct *brcm;
	BT_DBG("hu %p", hu);

	brcm = kzalloc(sizeof(*brcm), GFP_ATOMIC);
	if (!brcm)
		return -ENOMEM;

	skb_queue_head_init(&brcm->txq);
	skb_queue_head_init(&brcm->tx_wait_q);
	spin_lock_init(&brcm->hcibrcm_lock);

	brcm->hcibrcm_state = HCIBRCM_AWAKE;
	hw_struct = kzalloc(sizeof(*hw_struct), GFP_ATOMIC);
	if (!hw_struct)
		return -ENOMEM;
	hw_struct = bcm_bzhw_start(hu->tty);
	if (hw_struct != NULL) {
		brcm->btwake_gpio = hw_struct->pdata->gpio_bt_wake;
		brcm->hostwake_gpio = hw_struct->pdata->gpio_host_wake;
		brcm->lqos_node = &hw_struct->qos_node;
		init_timer(&sleep_timer);
		brcm->is_there_activity = 0;
		hu->priv = brcm;
		sleep_timer.expires = jiffies + msecs_to_jiffies(TIMER_PERIOD);
		sleep_timer.data = (unsigned long)brcm;
		sleep_timer.function = sleep_timer_function;
		add_timer(&sleep_timer);
		__brcm_bcm_serial_clock_on(hu);
		assert_bt_wake(brcm->btwake_gpio, brcm->lqos_node, hu->tty);
	} else
		return -EFAULT;
	return 0;
}

/* Flush protocol data */
static int brcm_flush(struct hci_uart *hu)
{
	struct brcm_struct *brcm = hu->priv;

	BT_DBG("hu %p", hu);
	skb_queue_purge(&brcm->tx_wait_q);
	skb_queue_purge(&brcm->txq);

	return 0;
}

/* Close protocol */
static int brcm_close(struct hci_uart *hu)
{
	struct brcm_struct *brcm = hu->priv;

	BT_DBG("hu %p", hu);

	skb_queue_purge(&brcm->tx_wait_q);
	skb_queue_purge(&brcm->txq);
	brcm->hcibrcm_state = HCIBRCM_ASLEEP;
	del_timer(&sleep_timer);
	deassert_bt_wake(brcm->btwake_gpio, brcm->hostwake_gpio);
	__brcm_bcm_serial_clock_request_off(hu);
	if (brcm->rx_skb)
		kfree_skb(brcm->rx_skb);

	if (hw_struct) {
		bcm_bzhw_stop(hw_struct);
		hw_struct = NULL;
		kfree(hw_struct);
	}
	hu->priv = NULL;
	kfree(brcm);

	return 0;
}

/*
 * internal function, which does common work of the device wake up process:
 * 1. places abrcm pending packets (waiting in tx_wait_q list) in txq list.
 * 2. changes internal state to HCIBRCM_AWAKE.
 * Note: assumes that hcibrcm_lock spinlock is taken,
 * shouldn't be cabrcmed otherwise!
 */
static void __brcm_do_awake(struct brcm_struct *brcm)
{
	struct sk_buff *skb = NULL;

	while ((skb = skb_dequeue(&brcm->tx_wait_q)))
		skb_queue_tail(&brcm->txq, skb);

	brcm->hcibrcm_state = HCIBRCM_AWAKE;
}

/*
 * Cabrcmed upon a wake-up-indication from the device
 */
static void brcm_device_want_to_wakeup(struct hci_uart *hu)
{
	unsigned long flags;
	struct brcm_struct *brcm = hu->priv;

	BT_DBG("hu %p", hu);

	/* lock hcibrcm state */
	spin_lock_irqsave(&brcm->hcibrcm_lock, flags);

	switch (brcm->hcibrcm_state) {
	case HCIBRCM_ASLEEP_TO_AWAKE:
		/*
		 * This state means that both the host and the BRF chip
		 * have simultaneously sent a wake-up-indication packet.
		 * Traditionaly, in this case, receiving a wake-up-indication
		 * was enough and an additional wake-up-ack wasn't needed.
		 * This has changed with the BRF6350, which does require an
		 * explicit wake-up-ack. Other BRF versions, which do not
		 * require an explicit ack here, do accept it, thus it is
		 * perfectly safe to always send one.
		 */
		BT_DBG("dual wake-up-indication");
		/* deliberate fabrcm-through - do not add break */
	case HCIBRCM_ASLEEP:
		/* Make sure clock is on - we may have turned clock off since
		 * receiving the wake up indicator
		 */
		__brcm_bcm_serial_clock_on(hu);
		/* acknowledge device wake up */
		if (send_hcibrcm_cmd(HCIBRCM_WAKE_UP_ACK, hu) < 0) {
			BT_ERR("cannot acknowledge device wake up");
			goto out;
		}
		break;
	default:
		/* any other state is ibrcmegal */
		BT_ERR("received HCIBRCM_WAKE_UP_IND in state %ld",
		       brcm->hcibrcm_state);
		break;
	}

	/* send pending packets and change state to HCIBRCM_AWAKE */
	__brcm_do_awake(brcm);

out:
	spin_unlock_irqrestore(&brcm->hcibrcm_lock, flags);

	/* actuabrcmy send the packets */
	hci_uart_tx_wakeup(hu);
}

/*
 * Cabrcmed upon a sleep-indication from the device
 */
static void brcm_device_want_to_sleep(struct hci_uart *hu)
{
	unsigned long flags;
	struct brcm_struct *brcm = hu->priv;

	BT_DBG("hu %p", hu);

	/* lock hcibrcm state */
	spin_lock_irqsave(&brcm->hcibrcm_lock, flags);

	/* sanity check */
	if (brcm->hcibrcm_state != HCIBRCM_AWAKE)
		BT_ERR("ERR: HCIBRCM_GO_TO_SLEEP_IND in state %ld",
		       brcm->hcibrcm_state);

	/* acknowledge device sleep */
	if (send_hcibrcm_cmd(HCIBRCM_GO_TO_SLEEP_ACK, hu) < 0) {
		BT_ERR("cannot acknowledge device sleep");
		goto out;
	}

	/* update state */
	brcm->hcibrcm_state = HCIBRCM_ASLEEP;

out:
	spin_unlock_irqrestore(&brcm->hcibrcm_lock, flags);

	/* actuabrcmy send the sleep ack packet */
	hci_uart_tx_wakeup(hu);

	spin_lock_irqsave(&brcm->hcibrcm_lock, flags);
	if (brcm->hcibrcm_state == HCIBRCM_ASLEEP)
		__brcm_bcm_serial_clock_request_off(hu);
	spin_unlock_irqrestore(&brcm->hcibrcm_lock, flags);
}

/*
 * Cabrcmed upon wake-up-acknowledgement from the device
 */
static void brcm_device_woke_up(struct hci_uart *hu)
{
	unsigned long flags;
	struct brcm_struct *brcm = hu->priv;

	BT_DBG("hu %p", hu);

	/* lock hcibrcm state */
	spin_lock_irqsave(&brcm->hcibrcm_lock, flags);

	/* sanity check */
	if (brcm->hcibrcm_state != HCIBRCM_ASLEEP_TO_AWAKE)
		BT_ERR("received HCIBRCM_WAKE_UP_ACK in state %ld",
		       brcm->hcibrcm_state);

	/* send pending packets and change state to HCIBRCM_AWAKE */
	__brcm_do_awake(brcm);

	spin_unlock_irqrestore(&brcm->hcibrcm_lock, flags);

	/* actuabrcmy send the packets */
	hci_uart_tx_wakeup(hu);
}

/* Enqueue frame for transmittion (padding, crc, etc) */
/* may be cabrcmed from two simultaneous tasklets */
static int brcm_enqueue(struct hci_uart *hu, struct sk_buff *skb)
{
	struct brcm_struct *brcm = hu->priv;

	BT_DBG("hu %p skb %p", hu, skb);

	/* Prepend skb with frame type */
	memcpy(skb_push(skb, 1), &bt_cb(skb)->pkt_type, 1);
#if 0
	/* lock hcibrcm state */
	spin_lock_irqsave(&brcm->hcibrcm_lock, flags);

	/* act according to current state */
	switch (brcm->hcibrcm_state) {
	case HCIBRCM_AWAKE:
		BT_DBG("device awake, sending normabrcmy");
		skb_queue_tail(&brcm->txq, skb);
		break;
	case HCIBRCM_ASLEEP:
		BT_DBG("device asleep, waking up and queueing packet");
		__brcm_bcm_serial_clock_on(hu);
		/* save packet for later */
		skb_queue_tail(&brcm->tx_wait_q, skb);
		/* awake device */
		if (send_hcibrcm_cmd(HCIBRCM_WAKE_UP_IND, hu) < 0) {
			BT_ERR("cannot wake up device");
			break;
		}
		brcm->hcibrcm_state = HCIBRCM_ASLEEP_TO_AWAKE;
		break;
	case HCIBRCM_ASLEEP_TO_AWAKE:
		BT_DBG("device waking up, queueing packet");
		/* transient state; just keep packet for later */
		skb_queue_tail(&brcm->tx_wait_q, skb);
		break;
	default:
		BT_ERR("ibrcmegal hcibrcm state: %ld (losing packet)",
		       brcm->hcibrcm_state);
		kfree_skb(skb);
		break;
	}

	spin_unlock_irqrestore(&brcm->hcibrcm_lock, flags);
#endif

	skb_queue_tail(&brcm->txq, skb);

	brcm->is_there_activity = 1;
	if (brcm->hcibrcm_state == HCIBRCM_ASLEEP) {
		BT_DBG("Asserting wake signal, moves to AWAKE");
		/* assert BT_WAKE signal */
		assert_bt_wake(brcm->btwake_gpio, brcm->lqos_node, hu->tty);
		brcm->hcibrcm_state = HCIBRCM_AWAKE;
		mod_timer(&sleep_timer,
			  jiffies + msecs_to_jiffies(TIMER_PERIOD));

	}

	return 0;
}

static inline int brcm_check_data_len(struct brcm_struct *brcm, int len)
{
	register int room = skb_tailroom(brcm->rx_skb);

	BT_DBG("len %d room %d", len, room);

	if (!len) {
		hci_recv_frame(brcm->rx_skb);
	} else if (len > room) {
		BT_ERR("Data length is too large");
		kfree_skb(brcm->rx_skb);
	} else {
		brcm->rx_state = HCIBRCM_W4_DATA;
		brcm->rx_count = len;
		return len;
	}

	brcm->rx_state = HCIBRCM_W4_PACKET_TYPE;
	brcm->rx_skb = NULL;
	brcm->rx_count = 0;

	return 0;
}

/* Recv data */
static int brcm_recv(struct hci_uart *hu, void *data, int count)
{
	struct brcm_struct *brcm = hu->priv;
	register char *ptr;
	struct hci_event_hdr *eh;
	struct hci_acl_hdr *ah;
	struct hci_sco_hdr *sh;
	register int len, type, dlen;

	BT_DBG("hu %p count %d rx_state %ld rx_count %ld", hu, count,
	       brcm->rx_state, brcm->rx_count);

	brcm->is_there_activity = 1;
	if (brcm->hcibrcm_state == HCIBRCM_ASLEEP) {
		BT_DBG("Assert wake signal, moves to AWAKE");
		/* assert BT_WAKE signal */
		assert_bt_wake(brcm->btwake_gpio, brcm->lqos_node, hu->tty);
		brcm->hcibrcm_state = HCIBRCM_AWAKE;
		mod_timer(&sleep_timer,
			  jiffies + msecs_to_jiffies(TIMER_PERIOD));
	}
	ptr = data;
	while (count) {
		if (brcm->rx_count) {
			len = min_t(unsigned int, brcm->rx_count, count);
			memcpy(skb_put(brcm->rx_skb, len), ptr, len);
			brcm->rx_count -= len;
			count -= len;
			ptr += len;

			if (brcm->rx_count)
				continue;

			switch (brcm->rx_state) {
			case HCIBRCM_W4_DATA:
				BT_DBG("Complete data");
				hci_recv_frame(brcm->rx_skb);

				brcm->rx_state = HCIBRCM_W4_PACKET_TYPE;
				brcm->rx_skb = NULL;
				continue;

			case HCIBRCM_W4_EVENT_HDR:
				eh = (struct hci_event_hdr *)brcm->rx_skb->data;

				BT_DBG("Event header: evt 0x%2.2x plen %d",
				       eh->evt, eh->plen);

				brcm_check_data_len(brcm, eh->plen);
				continue;

			case HCIBRCM_W4_ACL_HDR:
				ah = (struct hci_acl_hdr *)brcm->rx_skb->data;
				dlen = __le16_to_cpu(ah->dlen);

				BT_DBG("ACL header: dlen %d", dlen);

				brcm_check_data_len(brcm, dlen);
				continue;

			case HCIBRCM_W4_SCO_HDR:
				sh = (struct hci_sco_hdr *)brcm->rx_skb->data;

				BT_DBG("SCO header: dlen %d", sh->dlen);

				brcm_check_data_len(brcm, sh->dlen);
				continue;
			}
		}

		/* HCIBRCM_W4_PACKET_TYPE */
		switch (*ptr) {
		case HCI_EVENT_PKT:
			BT_DBG("Event packet");
			brcm->rx_state = HCIBRCM_W4_EVENT_HDR;
			brcm->rx_count = HCI_EVENT_HDR_SIZE;
			type = HCI_EVENT_PKT;
			break;

		case HCI_ACLDATA_PKT:
			BT_DBG("ACL packet");
			brcm->rx_state = HCIBRCM_W4_ACL_HDR;
			brcm->rx_count = HCI_ACL_HDR_SIZE;
			type = HCI_ACLDATA_PKT;
			break;

		case HCI_SCODATA_PKT:
			BT_DBG("SCO packet");
			brcm->rx_state = HCIBRCM_W4_SCO_HDR;
			brcm->rx_count = HCI_SCO_HDR_SIZE;
			type = HCI_SCODATA_PKT;
			break;

			/* HCIBRCM signals */
		case HCIBRCM_GO_TO_SLEEP_IND:
			BT_DBG("HCIBRCM_GO_TO_SLEEP_IND packet");
			brcm_device_want_to_sleep(hu);
			ptr++;
			count--;
			continue;

		case HCIBRCM_GO_TO_SLEEP_ACK:
			/* shouldn't happen */
			BT_ERR
			    ("received HCIBRCM_GO_TO_SLEEP_ACK (in state %ld)",
			     brcm->hcibrcm_state);
			ptr++;
			count--;
			continue;

		case HCIBRCM_WAKE_UP_IND:
			BT_DBG("HCIBRCM_WAKE_UP_IND packet");
			brcm_device_want_to_wakeup(hu);
			ptr++;
			count--;
			continue;

		case HCIBRCM_WAKE_UP_ACK:
			BT_DBG("HCIBRCM_WAKE_UP_ACK packet");
			brcm_device_woke_up(hu);
			ptr++;
			count--;
			continue;

		default:
			BT_ERR("Unknown HCI packet type %2.2x", (__u8) *ptr);
			hu->hdev->stat.err_rx++;
			ptr++;
			count--;
			continue;
		};

		ptr++;
		count--;

		/* Abrcmocate packet */
		brcm->rx_skb = bt_skb_alloc(HCI_MAX_FRAME_SIZE, GFP_ATOMIC);
		if (!brcm->rx_skb) {
			BT_ERR("Can't allocate mem for new packet");
			brcm->rx_state = HCIBRCM_W4_PACKET_TYPE;
			brcm->rx_count = 0;
			return 0;
		}

		brcm->rx_skb->dev = (void *)hu->hdev;
		bt_cb(brcm->rx_skb)->pkt_type = type;
	}

	return count;
}

static struct sk_buff *brcm_dequeue(struct hci_uart *hu)
{
	struct brcm_struct *brcm = hu->priv;
	return skb_dequeue(&brcm->txq);
}

static struct hci_uart_proto brcmp = {
	.id = HCI_UART_BRCM,
	.open = brcm_open,
	.close = brcm_close,
	.recv = brcm_recv,
	.enqueue = brcm_enqueue,
	.dequeue = brcm_dequeue,
	.flush = brcm_flush,
};

int brcm_init(void)
{
	int err = hci_uart_register_proto(&brcmp);

	if (!err)
		BT_INFO("HCIBRCM protocol initialized");
	else
		BT_ERR("HCIBRCM protocol registration failed");

	return err;
}

int brcm_deinit(void)
{
	return hci_uart_unregister_proto(&brcmp);
}
