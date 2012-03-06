/*****************************************************************************
 * Copyright 2010-2012 Broadcom Corporation.  All rights reserved.
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2, available at
 * http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
 *
 * Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a
 * license other than the GPL, without Broadcom's express prior written
 * consent.
 *****************************************************************************/
/*
 *  brcm_rfcomm.c
 *
 *  Driver for Bluetooth RFCOMM ports
 *
 *  Based on Linux 2.6 serial driver architecture
 *
 *  V 1.0    -- Compatible BlueZ's rfcomm device support
 *
 */
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/module.h>
#include <linux/circ_buf.h>

#include <linux/capability.h>
#include <linux/skbuff.h>

#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h>
#include <net/bluetooth/rfcomm.h>

#include "btport.h"

#define DRIVER_VERSION "v1.0"

#define BRCM_RFCOMM_TYPE	10
#define BRCM_BTPORT_MAJOR	0

#define BRCM_RFCOMM_MAJOR	200
#define BRCM_RFCOMM_MINOR	104

#define BRCM_SERIAL_NAME	"BRCMPORT"
#define BRCM_BTPORT_NAME	"BtPort"

#define BRCM_UART_NR		255	/* Maximum ports */
#define BRCM_RFCOMM_DUN		0
#define BRCM_RFCOMM_SPP		1
#define BRCM_RFCOMM_RAW		2

#define _DEBUG_
#define _INFO_

#ifdef _INFO_
#define info(fmt, arg...) \
	printk(KERN_INFO "%s-%s: " fmt "\n", BRCM_SERIAL_NAME ,\
			__func__, ## arg)
#else
#define info(fmt, arg...)
#endif

#ifdef _DEBUG_
#define dbg(fmt, arg...) \
	printk(KERN_DEBUG "%s-%s: " fmt "\n", BRCM_SERIAL_NAME ,\
			__func__, ## arg)
#else
#define dbg(fmt, arg...)
#endif

#define err(format, arg...) \
	printk(KERN_ERR "%s-%s: " format "\n" , BRCM_SERIAL_NAME ,\
			__func__, ## arg)

#define warn(format, arg...) \
	printk(KERN_WARNING "%s-%s: " format "\n" , BRCM_SERIAL_NAME ,\
			__func__, ## arg)

/****************************************************************************
 *
 *   STRUCTS AND TYPEDEFS
 *
 ****************************************************************************/
#define BRCM_BUF_SIZE 4096

struct BTLINUXPORTEvent {
	int eventCode;
	union {
		short modemControlReg;
	} u;
};

struct brcm_uart_port {
	struct uart_port rfcomm_port;
	uint16_t line;
	uint8_t port_added;
	uint8_t rfcomm_opened;
	uint8_t open_count;
	int btport_opened;
	spinlock_t rx_lock;
	char rx_data[BRCM_BUF_SIZE];
	int rx_data_head;
	int rx_data_tail;
	char rx_full;
	char tx_data[BRCM_BUF_SIZE];
	int tx_data_head;
	int tx_data_tail;
	spinlock_t tx_lock;
	struct BTLINUXPORTEvent saved_event;
	wait_queue_head_t port_wait_q;
	int event_result;
	int flag;
	wait_queue_head_t rx_wait_q;
	uint8_t wake_timeout;
	int event_data;
	int modem_control;
	struct fasync_struct *fasync;
	struct tasklet_struct tx_task;
	bdaddr_t src;
	bdaddr_t dst;
};
int brcm_btport_major;
static struct brcm_uart_port *brcm_rfcomm_ports;
static struct uart_driver brcm_rfcomm_port_reg;
static int btport_fasync(int fd, struct file *file, int on);

#define tx_circ_empty(circ) ((circ)->tx_data_head == (circ)->tx_data_tail)
#define tx_circ_chars_pending(circ) \
	(CIRC_CNT((circ)->tx_data_head, (circ)->tx_data_tail, BRCM_BUF_SIZE))
#define tx_circ_chars_free(circ) \
	(CIRC_SPACE((circ)->tx_data_head, (circ)->tx_data_tail, BRCM_BUF_SIZE))

/****************************************************************************
 *
 ****************************************************************************/
static void send_tx_char(struct brcm_uart_port *up)
{

	struct circ_buf *xmit = &up->rfcomm_port.state->xmit;
	int remain_to_send, free_space;
	unsigned long flags;
	unsigned head;
	unsigned tail;

	spin_lock_irqsave(&up->tx_lock, flags);
	head = up->tx_data_head;
	tail = up->tx_data_tail;

	remain_to_send = uart_circ_chars_pending(xmit);
	free_space = CIRC_SPACE(head, tail, BRCM_BUF_SIZE);

	info("tx_data_head: %d, tx_data_tail: %d, remain to send: %d, free: %d",
	     head, tail, remain_to_send, free_space);

	while (remain_to_send && free_space) {
		up->tx_data[up->tx_data_head] = xmit->buf[xmit->tail];

		/* commit the item before incrementing the head */
		smp_wmb();
		up->tx_data_head = (up->tx_data_head + 1) & (BRCM_BUF_SIZE - 1);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		remain_to_send--;
		free_space--;
		info("char feed: %c, remain to send: %d, free space: %d",
		     xmit->buf[xmit->tail], remain_to_send, free_space);
	}
	spin_unlock_irqrestore(&up->tx_lock, flags);

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(&up->rfcomm_port);

	info("char feed: %c, remain to send: %d, free space: %d",
	     xmit->buf[xmit->tail], remain_to_send, free_space);
}

/****************************************************************************
 *
 *
 *
 ****************************************************************************/
static void btlinux_handle_mctrl(struct brcm_uart_port *linux_port,
				 uint8_t signals, uint8_t values)
{
	uint16_t status = 0;

	info("[USERSPACE <-- KERNEL] -- signals %x, values %x", signals,
	     values);

	if (values & PORT_DTRDSR_ON) {
		info("BTPORT_DTRDSR_ON");
		status |= TIOCM_DSR;
	};
	if (values & PORT_CTSRTS_ON) {
		info("BTPORT_CTSRTS_ON");
		status |= TIOCM_CTS;
	};
	if (values & PORT_RING_ON) {
		info("BTPORT_RING_ON");
		status |= TIOCM_RI;
	};
	/* you may need to remap to BTLINUX_TIOCM_CD */
	/* if platform uses a different bit for CD ! */
	if (values & PORT_DCD_ON) {
		info("BTPORT_DCD_ON");
		status |= TIOCM_CD;
	};

	if (signals & MODEM_CNTRL_DTRDSR_MASK) {
		info("MODEM_CNTRL_DTRDSR_MASK, SET DTR %d", status & TIOCM_DSR);
		linux_port->rfcomm_port.icount.dsr++;
	}

	if (signals & MODEM_CNTRL_CTSRTS_MASK) {
		info("MODEM_CNTRL_CTSRTS_MASK, SET CTS %d", status & TIOCM_CTS);
		uart_handle_cts_change(&linux_port->rfcomm_port,
				       status & TIOCM_CTS);
	}

	if (signals & MODEM_CNTRL_RI_MASK) {
		info("MODEM_CNTRL_RI_MASK, SET RI %d", status & TIOCM_RI);
		linux_port->rfcomm_port.icount.rng++;
	}
	if (signals & MODEM_CNTRL_CD_MASK) {
		/* you may need to remap to BTLINUX_TIOCM_CD */
		/* if platform uses a different bit for CD ! */
		info("MODEM_CNTRL_CD_MASK, SET CD %d", status & TIOCM_CD);

		uart_handle_dcd_change(&linux_port->rfcomm_port,
				       status & TIOCM_CD);

	}
}

static DEFINE_RWLOCK(rfcomm_dev_lock);

/****************************************************************************
 *
 ****************************************************************************/
static int rfcomm_get_dev_list(void __user *arg)
{
	struct rfcomm_dev_list_req *dl;
	struct rfcomm_dev_info *di;
	int n = 0, i = 0, size, err = 0;
	uint16_t dev_num;

	BT_DBG("");

	if (get_user(dev_num, (uint16_t __user *)arg))
		return -EFAULT;

	if (!dev_num || dev_num > (PAGE_SIZE * 4) / sizeof(*di))
		return -EINVAL;

	size = sizeof(*dl) + dev_num * sizeof(*di);

	dl = kmalloc(size, GFP_KERNEL);
	if (!dl)
		return -ENOMEM;

	di = dl->dev_info;

	read_lock_bh(&rfcomm_dev_lock);

	for (n = 1; n < BRCM_UART_NR; n++) {
		if (!(brcm_rfcomm_ports + n)->port_added)
			continue;
		(di + i)->id = (brcm_rfcomm_ports + n)->line;
		(di + i)->flags = (brcm_rfcomm_ports + n)->port_added;
		(di + i)->state = (brcm_rfcomm_ports + n)->rfcomm_opened;
		(di + i)->channel = (brcm_rfcomm_ports + n)->line;
		bacpy(&(di + i)->src, &(brcm_rfcomm_ports + n)->src);
		bacpy(&(di + i)->dst, &(brcm_rfcomm_ports + n)->dst);

		i++;
		if (n >= dev_num)
			break;
	}

	read_unlock_bh(&rfcomm_dev_lock);

	dl->dev_num = i;
	size = sizeof(*dl) + i * sizeof(*di);

	if (size)
		err = copy_to_user(arg, dl, size);
	kfree(dl);

	return err ? -EFAULT : 0;
}

/****************************************************************************
 *
 ****************************************************************************/
static int rfcomm_set_di(struct brcm_uart_port *p_btport, void __user *arg)
{
	struct rfcomm_dev_info di;

	BT_DBG("");

	read_lock_bh(&rfcomm_dev_lock);

	if (copy_from_user(&di, (void *)arg, sizeof(struct rfcomm_dev_info))) {
		err("Failed getting data from user.");
		return -EFAULT;
	}

	info("src: %02x:%02x:%02x:%02x:%02x:%02x",
	     di.src.b[0], di.src.b[1], di.src.b[2], di.src.b[3], di.src.b[4],
	     di.src.b[5]);
	bacpy(&p_btport->src, &di.src);

	info("dst: %02x:%02x:%02x:%02x:%02x:%02x",
	     di.dst.b[0], di.dst.b[1], di.dst.b[2], di.dst.b[3], di.dst.b[4],
	     di.dst.b[5]);
	bacpy(&p_btport->dst, &di.dst);

	read_unlock_bh(&rfcomm_dev_lock);

	return 0;
}

/****************************************************************************
 *
 ****************************************************************************/
static ssize_t btport_read(struct file *file, char __user *user_buffer,
			   size_t len, loff_t *ppos)
{
	struct brcm_uart_port *p_btport =
	    (struct brcm_uart_port *)file->private_data;
	int i = 0, count = 0;
	unsigned long flags;
	unsigned char *p;
	unsigned head;
	unsigned tail;

	if (p_btport == NULL) {
		err("can't find device");
		return -ENODEV;
	}

	if (!p_btport->rfcomm_opened) {
		err("port not opened");
		return -ENODEV;
	}

	if (!p_btport->btport_opened)
		return -EINVAL;

	spin_lock_irqsave(&p_btport->tx_lock, flags);

	head = p_btport->tx_data_head;
	tail = p_btport->tx_data_tail;

	count = CIRC_CNT(head, tail, BRCM_BUF_SIZE);
	if (count > len)
		count = len;

	/* dbg ("tx_data_head: %d, tx_data_tail: %d, count: %d ",
		head, tail, count); */

	p = kmalloc(count, GFP_ATOMIC);

	if (p) {
		for (i = 0; i < count; i++) {
			p[i] = p_btport->tx_data[tail];
			tail = (tail + 1) & (BRCM_BUF_SIZE - 1);
		}

		if (copy_to_user(user_buffer, p, count)) {
			count = -EFAULT;
			kfree(p);
			goto err_out;
		}
		kfree(p);

		smp_wmb();
		p_btport->tx_data_tail =
		    (p_btport->tx_data_tail + count) & (BRCM_BUF_SIZE - 1);
		dbg("tx_data_head: %d, tx_data_tail: %d, count: %d ", head,
		    tail, count);
		if (p_btport->rx_data_tail != p_btport->rx_data_head)
			p_btport->rx_full = 0;

	} else {
		info("Failed to allocat buffer");
		count = -ENOMEM;
	}
err_out:
	spin_unlock_irqrestore(&p_btport->tx_lock, flags);

	return count;
}

/****************************************************************************
 *
 ****************************************************************************/
static ssize_t btport_write(struct file *file, const char __user *user_buffer,
			    size_t count, loff_t *ppos)
{
	struct brcm_uart_port *p_btport;
	struct tty_struct *tty;
	int i = 0;
	int access = 0;
	unsigned long flags;

	p_btport = (struct brcm_uart_port *)file->private_data;
	if (p_btport == NULL) {
		err("can't find device");
		return -ENODEV;
	}

	if (!p_btport->rfcomm_opened) {
		err("rfcomm port not opened");
		return -ENODEV;
	}

	if (!p_btport->btport_opened) {
		err("btport port not opened");
		return -EINVAL;
	}

	/* check to verify that the incomign data is good */
	access = !access_ok(VERIFY_READ, (void *)user_buffer, count);
	if (access) {
		err("buffer access verification failed");
		return 0;
	}
	if (!p_btport->rfcomm_port.state) {
		err("port info cannot obtained");
		return -EINVAL;
	}

	tty = p_btport->rfcomm_port.state->port.tty;
	if (!tty) {
		err("tty not obtained");
		return -EINVAL;
	}

	spin_lock_irqsave(&p_btport->rx_lock, flags);

	if (count) {
		unsigned char *p;

		info("kmalloc(%d, GFP_ATOMIC);", count);
		p = kmalloc(count, GFP_ATOMIC);
		if (p) {
			info("copy_from_user(p, user_buffer, %d);", count);
			if (!copy_from_user(p, user_buffer, count)) {
				while (count) {
					i = 0;

					if (tty_buffer_request_room(tty, 1) ==
					    0) {
						err("Flip buffer overflows");
						goto write_exit;
					}
					if (tty_insert_flip_char
					    (tty, p[i], TTY_NORMAL)
					    == 0)
						err("flip buffer insert error");
					i++;
					p_btport->rfcomm_port.icount.rx++;
					count--;

				}
			} else {
				i = -EFAULT;
			}
			kfree(p);
		}
	}

write_exit:
	spin_unlock_irqrestore(&p_btport->rx_lock, flags);

	if (0 <= i) {
		if (p_btport->rx_data_tail != p_btport->rx_data_head)
			p_btport->rx_full = 0;

		tty->low_latency = 1;
		tty->icanon = 0;

		tty_flip_buffer_push(tty);
	}
	/* let user know how much data we sent to transport  */
	return i;
}

/****************************************************************************
 *
 ****************************************************************************/
unsigned int btport_poll(struct file *file, struct poll_table_struct *p_pt)
{
	struct brcm_uart_port *p_btport = file->private_data;
	unsigned mask = 0;	/* always make this writable  */
	struct circ_buf *xmit;

	if (p_btport == NULL) {
		err("error, can't find device");
		return -ENODEV;
	}

	if (!p_btport->btport_opened) {
		mask |= POLLHUP | POLLRDNORM;
		return mask;
	}

	if (!p_btport->rfcomm_port.state)
		return mask;

	xmit = &p_btport->rfcomm_port.state->xmit;
	if (p_btport->tx_data_head == p_btport->tx_data_tail)
		poll_wait(file, &p_btport->rx_wait_q, p_pt);

	/* only set READ mask if data is queued from HCI */
	if (p_btport->tx_data_head != p_btport->tx_data_tail)
		mask |= POLLIN | POLLRDNORM;

	return mask;
}

/****************************************************************************
 *
 ****************************************************************************/
static int btport_open(struct inode *inode, struct file *filp)
{
	struct brcm_uart_port *p_btport;
	int minor;
	int ret;

	minor = iminor(inode);

	info("btport%d ", minor);

	if (minor >= BRCM_UART_NR)
		return -ENODEV;

	p_btport = (brcm_rfcomm_ports + minor);

	dbg("p_btport %x", (unsigned int)p_btport);

	if (minor > 0) {
		if (p_btport->open_count) {
			err("btport%d already opened.", minor);
			return -EBUSY;
		}
	}

	/* init the wait q  */
	init_waitqueue_head(&p_btport->rx_wait_q);

	dbg("wait queue initialized");

	p_btport->open_count++;
	p_btport->btport_opened = TRUE;
	p_btport->line = minor;

	p_btport->rx_data_head = 0;
	p_btport->rx_data_tail = 0;
	p_btport->rx_full = 0;

	p_btport->tx_data_head = 0;
	p_btport->tx_data_tail = 0;

	filp->private_data = p_btport;

	if (minor > 0) {
		if (!p_btport->port_added) {
			info("Add rfcomm%d, up %x up->rfcomm_port %x",
			     p_btport->rfcomm_port.line, (int)p_btport,
			     (int)&p_btport->rfcomm_port);

			ret =
			    uart_add_one_port(&brcm_rfcomm_port_reg,
					      &p_btport->rfcomm_port);
			if (ret) {
				err("uart_add_one_port : error %d", ret);
				return -ENODEV;
			}
			init_waitqueue_head(&p_btport->port_wait_q);
			p_btport->port_added = 1;
		} else
			info("rfcomm%d port already added! up %x"
					"up->rfcomm_port %x",
					p_btport->rfcomm_port.line,
					(int)p_btport,
					(int)&p_btport->rfcomm_port);
	}

	dbg("### btport%d opened (open_count %d) ###", minor,
	    p_btport->open_count);

	return 0;
}

/****************************************************************************
 *
 ****************************************************************************/
static int btport_release(struct inode *inode, struct file *filp)
{
	struct brcm_uart_port *p_btport =
	    (struct brcm_uart_port *)filp->private_data;
	int minor;

	minor = iminor(inode);

	if (p_btport == NULL) {
		err("can't find device");
		return -ENODEV;
	}

	if (minor > 0) {
		if (p_btport->port_added) {
			dbg("unregister rfcomm port");
			uart_remove_one_port(&brcm_rfcomm_port_reg,
					     &p_btport->rfcomm_port);
			p_btport->port_added = 0;
		}
	}
	if (p_btport->fasync)
		btport_fasync(-1, filp, 0);

	p_btport->open_count--;
	p_btport->btport_opened = FALSE;

	dbg("open_count %d", p_btport->open_count);

	p_btport->wake_timeout = TRUE;

	/* ensure there is no process hanging on poll / select  */
	wake_up_interruptible(&p_btport->rx_wait_q);

	info("### close btport%d : done ! ###", p_btport->line);

	return 0;
}

/****************************************************************************
 *
 ****************************************************************************/
static int btport_fasync(int fd, struct file *file, int on)
{
	int retval;
	struct brcm_uart_port *p_btport = file->private_data;

	dbg("fd %d, on %d", fd, on);

	if (p_btport == NULL) {
		err("error, can't find device");
		return -ENODEV;
	}

	retval = fasync_helper(fd, file, on, &p_btport->fasync);
	return retval < 0 ? retval : 0;
}

/****************************************************************************
 *
 ****************************************************************************/
static long btport_ioctl(struct file *file, unsigned int cmd,
						unsigned long arg)
{
	struct brcm_uart_port *p_btport;
	long retval = 0;
	uint8_t signals;
	uint8_t values;

	/* dbg ("cmd %x", cmd); */

	p_btport = (struct brcm_uart_port *)file->private_data;
	if (p_btport == NULL) {
		err("can't find device");
		return -ENODEV;
	}

	info("Received command %d, RFCOMMGETDEVLIST = %d", cmd,
	     RFCOMMGETDEVLIST);
	info("Received command %d, RFCOMMCREATEDEV = %d", cmd, RFCOMMCREATEDEV);

	/* Compatible with blueZ socket first */
	switch (cmd) {

	case RFCOMMCREATEDEV:
		info("RFCOMMCREATEDEV");
		return rfcomm_set_di(p_btport, (void __user *)arg);
		break;

	case RFCOMMGETDEVLIST:
		info("RFCOMMGETDEVLIST");
		return rfcomm_get_dev_list((void __user *)arg);
		break;

	}

	if (!p_btport->rfcomm_opened) {
		err("port not opened");
		return -ENODEV;
	}

	if (!p_btport->btport_opened)
		return -EINVAL;

	switch (cmd) {

	case IOCTL_BTPORT_GET_EVENT:
		info("IOCTL_BTPORT_GET_EVENT");
		if (copy_to_user
		    ((void *)arg, &p_btport->saved_event,
		     sizeof(p_btport->saved_event))) {
			err("copy_to_user failed");
			retval = -EFAULT;
			goto err_out;
		}
		break;

	case IOCTL_BTPORT_SET_EVENT_RESULT:
		info("IOCTL_BTPORT_SET_EVENT_RESULT");
		p_btport->event_result = 0;
		p_btport->flag = 1;
		if (copy_from_user
		    (&p_btport->event_result, (void *)arg, sizeof(int))) {
			retval = -EFAULT;
			err("IOCTL_BTPORT_SET_EVENT_RESULT failed 1st param");
			wake_up(&p_btport->port_wait_q);
			goto err_out;
		}
		if (copy_from_user
		    (&p_btport->event_data, ((char *)arg) + sizeof(int),
		     sizeof(int))) {
			retval = -EFAULT;
			err("IOCTL_BTPORT_SET_EVENT_RESULT failed  2nd pararm");
			wake_up(&p_btport->port_wait_q);
			goto err_out;
		}
		info("fetched event result %d, ev data %x",
		     p_btport->event_result, p_btport->event_data);
		wake_up(&p_btport->port_wait_q);
		info("signal wait queue");
		break;

	case IOCTL_BTPORT_HANDLE_MCTRL:
		info("IOCTL_BTPORT_HANDLE_MCTRL");
		if (copy_from_user(&signals, (void *)arg, sizeof(uint8_t))) {
			retval = -EFAULT;
			err("IOCTL_BTPORT_HANDLE_MCTRL failed 1st param");
			goto err_out;
		}
		if (copy_from_user
		    (&values, ((char *)arg) + sizeof(uint8_t),
		     sizeof(uint8_t))) {
			retval = -EFAULT;
			err("IOCTL_BTPORT_HANDLE_MCTRL failed 2nd param");
			goto err_out;
		}

		btlinux_handle_mctrl(p_btport, signals, values);

		break;

	case RFCOMMGETDEVINFO:
		info("RFCOMMGETDEVINFO");
		break;
	}
err_out:
	dbg("BTLINUXPORT btport_ioctl returning %ld", retval);
	return retval;
}

/*****************************************************************************/
static const struct file_operations brcm_btport_fops = {
	.owner = THIS_MODULE,
	.read = btport_read,
	.write = btport_write,
	.poll = btport_poll,
	.open = btport_open,
	.release = btport_release,
	.fasync = btport_fasync,
	.compat_ioctl = btport_ioctl,
};

/****************************************************************************
  ****************************************************************************
  ****************************************************************************
  * RFCOMM
  ****************************************************************************
  ****************************************************************************
  ****************************************************************************/
static unsigned int brcm_rfcomm_tx_empty(struct uart_port *uport)
{
	struct circ_buf *xmit = &uport->state->xmit;

	if (uart_circ_empty(xmit)) {
		info("tx buffer empty");
		return TIOCSER_TEMT;
	}
	return 0;
}

/****************************************************************************
 *
 ****************************************************************************/
static void brcm_rfcomm_set_mctrl(struct uart_port *uport, unsigned int mctrl)
{
	struct brcm_uart_port *p = (struct brcm_uart_port *)uport->private_data;

	if (p) {
		info("mctrl: %x, p: %x, p->flag: %x", mctrl, (unsigned int)p,
		     p->flag);

		if (!p) {
			err("null uart");
			return;
		}

		if (!p->rfcomm_opened) {
			err("rfcomm port is not opened");
			return;
		}

		if (!p->btport_opened) {
			err("btport is not opened");
			return;
		}
		p->saved_event.eventCode = BTLINUX_PORT_MODEM_CONTROL;
		p->saved_event.u.modemControlReg = 0;

		if (mctrl & TIOCM_DTR) {
			/* info (" DTR SET"); */
			p->saved_event.u.modemControlReg |=
			    MODEM_CNTRL_DTRDSR_MASK;
		} else {
			/* info(" DTR CLR"); */
		}

		if (mctrl & TIOCM_RTS) {
			/* info(" RTS SET"); */
			p->saved_event.u.modemControlReg |=
			    MODEM_CNTRL_CTSRTS_MASK;
		} else {
			/* info(" RTS CLR"); */
		}

		/* remap TIOCM_CD depending on ttySAx CD output if platform is
		 *  using different TIOCM, eg. OUT1 */
		if (mctrl & BTLINUX_TIOCM_CD) {
			/* info(" CD SET"); */
			p->saved_event.u.modemControlReg |= MODEM_CNTRL_CD_MASK;
		} else {
			/* info(" CD CLR"); */
		}
		if (mctrl & TIOCM_RI) {
			/* info(" RI SET"); */
			p->saved_event.u.modemControlReg |= MODEM_CNTRL_RI_MASK;
		} else {
			/* info(" RI CLR"); */
		}

		if (p->fasync) {
			dbg("notify application using fasync (fd %d)",
			    p->fasync->fa_fd);

			/* notify application */
			kill_fasync(&p->fasync, SIGUSR1, POLL_MSG);

			/* sync wait */
			wait_event(p->port_wait_q, p->flag != 0);
			dbg("wait done");
		} else {
			dbg("No fasync requested.");
		}
	}
}

/****************************************************************************
 *
 ****************************************************************************/
static unsigned int brcm_rfcomm_get_mctrl(struct uart_port *uport)
{
	unsigned int ret = 0;
	struct brcm_uart_port *p = (struct brcm_uart_port *)uport->private_data;

	if (p) {
		if (!p->rfcomm_opened) {
			info("port is not opened");
			return -EINVAL;
		}

		/* notify application */
		if (p->fasync) {
			p->saved_event.eventCode = BTLINUX_PORT_MODEM_STATUS;
			kill_fasync(&p->fasync, SIGUSR1, POLL_MSG);
			/* sync wait */
			wait_event(p->port_wait_q, p->flag != 0);

			p->flag = 0;
			if (p->event_result == 0) {
				err("Bad return from PORT_GetModemStatus");
				return -EINVAL;
			}
			/* info("fetched modem status ""
				"(from userspace stack) 0x%x",
			   up->event_data); */

			if (p->event_data & PORT_DTRDSR_ON)
				ret |= TIOCM_DSR;

			if (p->event_data & PORT_CTSRTS_ON)
				ret |= TIOCM_CTS;

			if (p->event_data & PORT_RING_ON)
				ret |= TIOCM_RI;

			/* you may need to remap to BTLINUX_TIOCM_CD */
			/* if platform uses a different bit for CD ! */
			if (p->event_data & PORT_DCD_ON)
				ret |= TIOCM_CD;
		} else {
			if (p->saved_event.u.
			    modemControlReg & MODEM_CNTRL_DTRDSR_MASK)
				ret |= TIOCM_DSR;
			if (p->saved_event.u.
			    modemControlReg & MODEM_CNTRL_CTSRTS_MASK)
				ret |= TIOCM_CTS;
			if (p->saved_event.u.
			    modemControlReg & MODEM_CNTRL_CD_MASK)
				ret |= TIOCM_CD;
			if (p->saved_event.u.
			    modemControlReg & MODEM_CNTRL_RI_MASK)
				ret |= TIOCM_RI;
			/* info("Default modem status 0x%x", ret); */
		}
	}
	return ret;
}

/****************************************************************************
 *
 ****************************************************************************/
static void brcm_rfcomm_stop_tx(struct uart_port *uport)
{
	dbg("not implemented");
}

/****************************************************************************
 *
 ****************************************************************************/
static void brcm_rfcomm_start_tx(struct uart_port *uport)
{
	struct brcm_uart_port *p = (struct brcm_uart_port *)uport->private_data;

	dbg(" ");
	if (p) {
		struct circ_buf *xmit = &p->rfcomm_port.state->xmit;
		int count;

		if (!p->rfcomm_opened) {
			err("port is not opened");
			return;
		}

		if (!p->btport_opened) {
			err("bt port is not opened");
			brcm_rfcomm_stop_tx(uport);
			return;
		}

		count = uart_circ_chars_pending(xmit);
		dbg("line %d - buf size = %d", p->line, count);

		if (uport->x_char) {
			info("x_char is transmitted");
			uport->icount.tx++;
			uport->x_char = 0;
			return;
		}

		if (uart_circ_empty(xmit) || uart_tx_stopped(uport)) {
			info("tx buffer empty or tx stopped");
			brcm_rfcomm_stop_tx(uport);
			return;
		}

		send_tx_char(p);

		/* wake up application waiting in a poll */
		wake_up_interruptible(&p->rx_wait_q);
	}
}

/****************************************************************************
 *
 ****************************************************************************/
static void brcm_rfcomm_stop_rx(struct uart_port *uport)
{
	dbg("not implemented");
}

/****************************************************************************
 *
 ****************************************************************************/
static void brcm_rfcomm_enable_ms(struct uart_port *uport)
{
	dbg("not implemented");
}

/****************************************************************************
 *
 ****************************************************************************/
static void brcm_rfcomm_break_ctl(struct uart_port *uport, int break_state)
{
	dbg("not implemented");
}

/****************************************************************************
 *
 ****************************************************************************/
static int brcm_rfcomm_startup(struct uart_port *uport)
{
	struct brcm_uart_port *p = (struct brcm_uart_port *)uport->private_data;
	struct tty_struct *tty;

	if (p) {
		dbg("uart_port: %x, line: %d", (unsigned int)p,
		    p->rfcomm_port.line);

		p->rfcomm_opened = 1;
		p->saved_event.eventCode = BTLINUX_PORT_OPENED;

		tty = uport->state->port.tty;
		if (!tty) {
			err("Failed to get tty pointer");
			return -EINVAL;
		}

		p->saved_event.u.modemControlReg |= MODEM_CNTRL_DTRDSR_MASK;
		p->saved_event.u.modemControlReg |= MODEM_CNTRL_CTSRTS_MASK;
		p->saved_event.u.modemControlReg |= MODEM_CNTRL_CD_MASK;
		p->saved_event.u.modemControlReg |= MODEM_CNTRL_RI_MASK;

		dbg("tty magic: %d, index: %d, name: %s", tty->magic,
		    tty->index, tty->name);
		dbg("rfcomm_opened: %d", p->rfcomm_opened);
		uart_write_wakeup(&p->rfcomm_port);
	}
	return 0;
}

/****************************************************************************
 *
 ****************************************************************************/
static void brcm_rfcomm_shutdown(struct uart_port *uport)
{
	struct tty_struct *tty;
	struct brcm_uart_port *p = (struct brcm_uart_port *)uport->private_data;

	tty = uport->state->port.tty;

	if (tty)
		info("%s is closed", tty->name);

	p->rfcomm_opened = 0;
}

/****************************************************************************
 *
 ****************************************************************************/
static void brcm_rfcomm_set_termios(struct uart_port *uport,
				    struct ktermios *termios,
				    struct ktermios *old)
{
	struct tty_struct *tty;

	dbg(" ");

	tty = uport->state->port.tty;

	if (tty)
		memcpy(tty->termios, termios, sizeof(*termios));
}

/****************************************************************************
 *
 ****************************************************************************/
static void brcm_rfcomm_pm(struct uart_port *uport, unsigned int state,
			   unsigned int oldstate)
{
	dbg("not implemented");
}

/****************************************************************************
 *
 ****************************************************************************/
static const char *brcm_rfcomm_type(struct uart_port *uport)
{
	if (uport->line == BRCM_RFCOMM_DUN)
		return "brcm DUN";
	if (uport->line == BRCM_RFCOMM_SPP)
		return "brcm SPP";

	return "brcm RFCOMM";
}

/****************************************************************************
 *
 ****************************************************************************/
static void brcm_rfcomm_release_port(struct uart_port *uport)
{
	dbg("not implemented");
}

/****************************************************************************
 *
 ****************************************************************************/
static void brcm_rfcomm_config_port(struct uart_port *uport, int flags)
{
	struct brcm_uart_port *p = uport->private_data;

	dbg(" ");

	p->rfcomm_port.type = BRCM_RFCOMM_TYPE;

}

/****************************************************************************
 *
 ****************************************************************************/
static int brcm_rfcomm_verify_port(struct uart_port *uport,
				   struct serial_struct *ser)
{
	dbg("not implemented");
	return 0;
}

/****************************************************************************
 *
 ****************************************************************************/
static int brcm_rfcomm_ioctl(struct uart_port *uport, unsigned int cmd,
			     unsigned long arg)
{
	struct tty_struct *tty;
	void __user *argp = (void __user *)arg;

	tty = uport->state->port.tty;

	if (!tty) {
		dbg("Failed to get tty pointer");
		return 0;
	}

	switch (cmd) {
	case TCGETS:
		if (copy_to_user(argp, tty->termios, sizeof(struct termios)))
			return -EFAULT;
		break;

	case TCSETS:
		if (copy_from_user(tty->termios, argp, sizeof(struct termios)))
			return -EFAULT;
		break;

	case TIOCSBRK:
		err(" IOCTL(TIOCSBRK) -- unhandled");
		break;

	case TIOCCBRK:
		err(" IOCTL (TIOCCBRK) -- unhandled");
		break;

	default:
		return -ENOIOCTLCMD;
	}
	return 0;

}

/* ****************************************************************************/
static struct uart_driver brcm_rfcomm_port_reg = {
	.owner = THIS_MODULE,
	.driver_name = BRCM_SERIAL_NAME,
#ifdef CONFIG_DEVFS_FS
	.dev_name = "rfcomm%d",
#else
	.dev_name = "rfcomm",
#endif
	.major = BRCM_RFCOMM_MAJOR,
	.minor = BRCM_RFCOMM_MINOR,
	.nr = BRCM_UART_NR,
	.cons = NULL,
};

static struct uart_ops brcm_rfcomm_pops = {
	.tx_empty = brcm_rfcomm_tx_empty,
	.set_mctrl = brcm_rfcomm_set_mctrl,
	.get_mctrl = brcm_rfcomm_get_mctrl,
	.stop_tx = brcm_rfcomm_stop_tx,
	.start_tx = brcm_rfcomm_start_tx,
	.stop_rx = brcm_rfcomm_stop_rx,
	.enable_ms = brcm_rfcomm_enable_ms,
	.break_ctl = brcm_rfcomm_break_ctl,
	.startup = brcm_rfcomm_startup,
	.shutdown = brcm_rfcomm_shutdown,
	.set_termios = brcm_rfcomm_set_termios,
	.pm = brcm_rfcomm_pm,
	.type = brcm_rfcomm_type,
	.release_port = brcm_rfcomm_release_port,
	.config_port = brcm_rfcomm_config_port,
	.verify_port = brcm_rfcomm_verify_port,
	.ioctl = brcm_rfcomm_ioctl,
};

/****************************************************************************
 *
 ****************************************************************************/
static int __init brcm_init_rfcomm_ports(struct uart_driver *drv)
{
	int i, ret = 0;

	/* /dev/rfcomm0 = rfcomm(chanel15) */
	brcm_rfcomm_ports = (struct brcm_uart_port *)
	    kmalloc((BRCM_UART_NR * sizeof(struct brcm_uart_port)), GFP_KERNEL);

	if (brcm_rfcomm_ports != NULL) {
		for (i = 0; i < BRCM_UART_NR; i++) {
			struct brcm_uart_port *p = (brcm_rfcomm_ports + i);
			memset(p, 0, sizeof(*p));
			p->rfcomm_port.line = i;
			p->rfcomm_port.ops = &brcm_rfcomm_pops;
			p->rfcomm_port.iobase = 0x1;
			p->rfcomm_port.mapbase = 'C';	/*  0x45; */
			p->rfcomm_port.membase = (uint8_t *)"K"; /* 0x55; */
			p->rfcomm_port.flags |= UPF_BOOT_AUTOCONF;
			p->rfcomm_port.private_data = p;
			p->saved_event.u.modemControlReg = 0;
			spin_lock_init(&p->rx_lock);
			spin_lock_init(&p->tx_lock);
		}
	}
	return ret;
}

/****************************************************************************
 *
 ****************************************************************************/
static int __init brcm_bt_init(void)
{
	int ret;

	info("Loading BRCM rfcomm driver %s", DRIVER_VERSION);

	/* Register BtPort as char device */
	brcm_btport_major = register_chrdev(BRCM_BTPORT_MAJOR, BRCM_BTPORT_NAME,
					    &brcm_btport_fops);

	info("Registered btport chrdev, major number returned = %d",
	     brcm_btport_major);

	if (brcm_btport_major < 0) {
		err("unable to get major number");
		return brcm_btport_major;
	}

	ret = uart_register_driver(&brcm_rfcomm_port_reg);

	if (ret) {
		err("Failed to register rfcomm driver %d", ret);
		unregister_chrdev(brcm_btport_major, BRCM_BTPORT_NAME);
		return ret;
	}

	ret = brcm_init_rfcomm_ports(&brcm_rfcomm_port_reg);

	info("BRCM rfcomm initialized, OK");

	return ret;
}

/****************************************************************************
 *
 ****************************************************************************/
static void __exit brcm_bt_exit(void)
{
	int i;

	info("Unloading BRCM rfcomm driver %s", DRIVER_VERSION);

	for (i = 0; i < BRCM_UART_NR; i++) {
		if ((brcm_rfcomm_ports + i)->port_added) {
			uart_remove_one_port(&brcm_rfcomm_port_reg,
					     &(brcm_rfcomm_ports +
					       i)->rfcomm_port);
			(brcm_rfcomm_ports + i)->port_added = 0;
		}
	}
	uart_unregister_driver(&brcm_rfcomm_port_reg);
	unregister_chrdev(brcm_btport_major, BRCM_BTPORT_NAME);

	kfree(brcm_rfcomm_ports);

	info("BRCM rfcomm uninitialized OK");
}

module_init(brcm_bt_init);
module_exit(brcm_bt_exit);

/* Module information */
MODULE_AUTHOR("Satyajit Roy <roys@broadcom.com>"
		"-Chinda Keodouangsy <chindak@broadcom.com>");
MODULE_DESCRIPTION("BRCM RFCOMM port driver");
MODULE_LICENSE("GPL");
