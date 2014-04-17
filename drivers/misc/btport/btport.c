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
 *  btport.c
 *
 *  Bluetooth RFCOMM/tty to BtPort bridge driver for BRCM BT stack
 *  the BtPort's are connected to the SPP/DUN profile on BT stack side (aka
 *  rfcomm data pipes)
 *  the /dev/rfcommXX provide a tty/uart like interface to the user application
 *  BtPort0 is management port used to create dynamically a BtPortXX to
 *  rfcommXX mapping. where XX is rfcomm channel id.
 *  The driver runs in non-irq context!
 *
 *  Based on Linux 2.6 serial driver architecture
 *
 *  V 1.1    -- Compatible BlueZ's rfcomm device support
 *
 */
/*#define DEBUG
#define VERBOSE_DEBUG */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/device.h>
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

#define DRIVER_VERSION "v1.1"

#define BRCM_RFCOMM_TYPE	10
/* 0: linux kernel dynamically assigns major at register */
#define BRCM_BTPORT_MAJOR	0

/* use the rfcomm tty: 216, to avoid collision: 200 */
#ifdef CONFIG_BT_RFCOMM_TTY
#define BRCM_RFCOMM_MAJOR	200
#else
#define BRCM_RFCOMM_MAJOR	216
#endif
#define BRCM_RFCOMM_MINOR	0

#define BRCM_SERIAL_NAME	"BRCMPORT"
#define BRCM_BTPORT_NAME	"BtPort"
#define BRCM_BTPORT_FMT_NAME	"BtPort%d"


#define BRCM_MAX_UART_NR	255	/* Maximum ports */
#define BRCM_RFCOMM_DUN		0
#define BRCM_RFCOMM_SPP		1
#define BRCM_RFCOMM_RAW		2

/* if defined, the BtPort0 ioctl() dynamically create the device instead of
 * user space mkdev. use this as default */
#define BRCM_CREATE_BTPORT_DYN

/* #define BTPORT_ASYNC_SUPPORT */
/*
 *
 *   STRUCTS AND TYPEDEFS
 *
 */
#define BRCM_BUF_SIZE 4096
/* max size accepted by btport_write(): make it depend on page size
 * tty accepts only a max of 64Kbyte per write */
#define BTPORT_MAX_WRITE_SIZE (1*PAGE_SIZE)

struct btLinuxPortEvent {
	int eventCode;
	short modemControlReg;
};

#define SET_EVENT_RESULT 0
#define SET_EVENT_DATA 1

struct brcm_uart_port {
	struct uart_port rfcomm_port;
	spinlock_t lock;	/* protect queues and lists */
	char tx_data[BRCM_BUF_SIZE];
	int tx_data_head;
	int tx_data_tail;
	struct btLinuxPortEvent saved_event;
	wait_queue_head_t rx_wait_q;
	wait_queue_head_t port_wait_q;
	int flag;
	int set_event[2];	/* [0]:event_result, [1] event_data */
	int open_count;		/* may use atomic_t */
#ifdef BTPORT_ASYNC_SUPPORT
	struct fasync_struct *fasync;
#endif
	uint16_t line;
	uint8_t rfcomm_opened;
	bdaddr_t src;
	bdaddr_t dst;
};

struct btport_cdev {
	struct cdev	btPortCdev;
	struct class	*btPortClass;
	dev_t		btPortDevNum;
	int		nrDevices; /* holds the number of devices created! */
};
/*static int brcm_btport_major; */
/* TODO: use list */
static struct brcm_uart_port *brcm_rfcomm_ports[BRCM_MAX_UART_NR];
static struct uart_driver brcm_rfcomm_port_reg;
static struct btport_cdev gBrcmBtport;
static DEFINE_MUTEX(btport_mutex);

static int init_rfcomm_ports(struct brcm_uart_port **port, unsigned int i);
static void free_rfcomm_ports(struct brcm_uart_port **port);
#ifdef BTPORT_ASYNC_SUPPORT
static int btport_fasync(int fd, struct file *file, int on);
#endif

#define tx_circ_empty(circ) ((circ)->tx_data_head == (circ)->tx_data_tail)
#define tx_circ_chars_pending(circ) \
	(CIRC_CNT((circ)->tx_data_head, (circ)->tx_data_tail, BRCM_BUF_SIZE))
#define tx_circ_chars_free(circ) \
	(CIRC_SPACE((circ)->tx_data_head, (circ)->tx_data_tail, BRCM_BUF_SIZE))

/*
 * kick in tx from rfcomm to btport transmit (from userial_core)
 * called with up->rfcomm_port->lock taken
 */
static void send_tx_char(struct brcm_uart_port *up)
{

	struct circ_buf *xmit = &up->rfcomm_port.state->xmit;
	int remain_to_send, free_space;
	unsigned head;
	unsigned tail;

	head = up->tx_data_head;
	tail = up->tx_data_tail;

	remain_to_send = uart_circ_chars_pending(xmit);
	free_space = CIRC_SPACE(head, tail, BRCM_BUF_SIZE);

	pr_devel("tx_data_head: %d, tx_data_tail: %d, remain to send: %d, free: %d",
	     head, tail, remain_to_send, free_space);

	while (remain_to_send && free_space) {
		up->tx_data[head] = xmit->buf[xmit->tail];

		/* commit the item before incrementing the head */
		smp_wmb();
		head = (head + 1) & (BRCM_BUF_SIZE - 1);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		remain_to_send--;
		free_space--;
		pr_devel("char feed: %c, remain to send: %d, free space: %d",
		     xmit->buf[xmit->tail], remain_to_send, free_space);
	}
	up->tx_data_head = head;

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(&up->rfcomm_port);
	pr_devel("char feed: %c, remain to send: %d, free space: %d",
	     xmit->buf[xmit->tail], remain_to_send, free_space);
}

/*
 * set modem control signals from BtPort (bt stack) to rfcomm/tty
 * (DTE->DCE)
 *
 */
static void btlinux_handle_mctrl(struct brcm_uart_port *linux_port,
				 uint8_t signals, uint8_t values)
{
	uint16_t status = 0;

	pr_debug("[USERSPACE <-- KERNEL] -- signals %x, values %x", signals,
	     values);

	if (values & PORT_DTRDSR_ON) {
		pr_devel("BTPORT_DTRDSR_ON");
		status |= TIOCM_DSR;
	};
	if (values & PORT_CTSRTS_ON) {
		pr_devel("BTPORT_CTSRTS_ON");
		status |= TIOCM_CTS;
	};
	if (values & PORT_RING_ON) {
		pr_devel("BTPORT_RING_ON");
		status |= TIOCM_RI;
	};
	/* you may need to remap to BTLINUX_TIOCM_CD */
	/* if platform uses a different bit for CD ! */
	if (values & PORT_DCD_ON) {
		pr_devel("BTPORT_DCD_ON");
		status |= TIOCM_CD;
	};

	if (signals & MODEM_CNTRL_DTRDSR_MASK) {
		pr_devel("MODEM_CNTRL_DTRDSR_MASK, SET DTR %d",
				status & TIOCM_DSR);
		linux_port->rfcomm_port.icount.dsr++;
	}

	if (signals & MODEM_CNTRL_CTSRTS_MASK) {
		pr_devel("MODEM_CNTRL_CTSRTS_MASK, SET CTS %d",
				status & TIOCM_CTS);
		uart_handle_cts_change(&linux_port->rfcomm_port,
				       status & TIOCM_CTS);
	}

	if (signals & MODEM_CNTRL_RI_MASK) {
		pr_devel("MODEM_CNTRL_RI_MASK, SET RI %d", status & TIOCM_RI);
		linux_port->rfcomm_port.icount.rng++;
	}
	if (signals & MODEM_CNTRL_CD_MASK) {
		/* you may need to remap to BTLINUX_TIOCM_CD */
		/* if platform uses a different bit for CD ! */
		pr_devel("MODEM_CNTRL_CD_MASK, SET CD %d", status & TIOCM_CD);

		uart_handle_dcd_change(&linux_port->rfcomm_port,
				       status & TIOCM_CD);
	}
	wake_up_interruptible(&linux_port->rfcomm_port.state->
			port.delta_msr_wait);
}


/*
 * get rfcomm device info of all open devices
 */
static int rfcomm_get_dev_list(void __user *arg)
{
	struct rfcomm_dev_list_req *dl;
	struct rfcomm_dev_info *di;
	int n = 0, i = 0, size, err = 0;
	uint16_t dev_num;

	pr_debug("rfcomm_get_dev_list()");

	if (get_user(dev_num, (uint16_t __user *)arg))
		return -EFAULT;

	if (!dev_num || dev_num > (PAGE_SIZE * 4) / sizeof(*di))
		return -EINVAL;

	size = sizeof(*dl) + dev_num * sizeof(*di);

	dl = kzalloc(size, GFP_KERNEL);
	if (!dl)
		return -ENOMEM;

	di = dl->dev_info;

	if (mutex_lock_interruptible(&btport_mutex)) {
		err = -EINTR;
		goto err_out;
	}

	for (n = 1; n < BRCM_MAX_UART_NR; n++) {
		if (!(brcm_rfcomm_ports[n]))
			continue;
		(di + i)->id = brcm_rfcomm_ports[n]->line;
		(di + i)->flags = 0x1;
		(di + i)->state = brcm_rfcomm_ports[n]->rfcomm_opened;
		(di + i)->channel = brcm_rfcomm_ports[n]->line;
		bacpy(&(di + i)->src, &brcm_rfcomm_ports[n]->src);
		bacpy(&(di + i)->dst, &brcm_rfcomm_ports[n]->dst);

		i++;
		if (n >= dev_num)
			break;
	}
	dl->dev_num = i;
	size = sizeof(*dl) + i * sizeof(*di);

	mutex_unlock(&btport_mutex);

	if (size)
		err = copy_to_user(arg, dl, size);
err_out:
	kfree(dl);

	return err ? -EFAULT : 0;
}

/*
 * set rfcomm device info (bd, etc)
 */
static int rfcomm_set_di(struct brcm_uart_port *p_btport, void __user *arg)
{
	struct rfcomm_dev_info di;

	pr_debug("rfcomm_set_di()");

	if (copy_from_user(&di, arg, sizeof(struct rfcomm_dev_info))) {
		pr_err("rfcomm_set_di(): Failed getting user data");
		return -EFAULT;
	}

	if (mutex_lock_interruptible(&btport_mutex))
		return -EINTR;

	pr_devel("src: %02x:%02x:%02x:%02x:%02x:%02x",
	     di.src.b[0], di.src.b[1], di.src.b[2], di.src.b[3], di.src.b[4],
	     di.src.b[5]);
	bacpy(&p_btport->src, &di.src);

	pr_devel("dst: %02x:%02x:%02x:%02x:%02x:%02x",
	     di.dst.b[0], di.dst.b[1], di.dst.b[2], di.dst.b[3], di.dst.b[4],
	     di.dst.b[5]);
	bacpy(&p_btport->dst, &di.dst);

	mutex_unlock(&btport_mutex);

	return 0;
}

/*
 * set rfcomm device info (bd, etc)
 */
static int rfcomm_get_di(struct brcm_uart_port *p_btport, void __user *arg)
{
	struct rfcomm_dev_info di;

	pr_debug("rfcomm_get_di()");

	if (copy_from_user(&di, arg, sizeof(struct rfcomm_dev_info))) {
		pr_err("rfcomm_get_di(): Failed getting user data");
		return -EFAULT;
	}

	if (mutex_lock_interruptible(&btport_mutex))
		return -EINTR;

	pr_devel("src: %02x:%02x:%02x:%02x:%02x:%02x",
	     di.src.b[0], di.src.b[1], di.src.b[2], di.src.b[3], di.src.b[4],
	     di.src.b[5]);
	bacpy(&p_btport->src, &di.src);

	pr_devel("dst: %02x:%02x:%02x:%02x:%02x:%02x",
	     di.dst.b[0], di.dst.b[1], di.dst.b[2], di.dst.b[3], di.dst.b[4],
	     di.dst.b[5]);
	bacpy(&p_btport->dst, &di.dst);

	mutex_unlock(&btport_mutex);

	return 0;
}

/*
 *
 */
static ssize_t btport_read(struct file *file, char __user *user_buffer,
			   size_t len, loff_t *ppos)
{
	struct brcm_uart_port *p_btport =
	    (struct brcm_uart_port *)file->private_data;
	struct uart_port *up;
	int i = 0;
	ssize_t count = 0;
	unsigned char *p;
	unsigned head;
	unsigned tail;

	if (p_btport == NULL) {
		pr_err("can't find device");
		return -ENODEV;
	}

	if (!p_btport->rfcomm_opened) {
		pr_err("port not opened");
		return -ENODEV;
	}

	up = &p_btport->rfcomm_port;
	spin_lock(&up->lock);
	head = p_btport->tx_data_head;
	tail = p_btport->tx_data_tail;

	count = CIRC_CNT(head, tail, BRCM_BUF_SIZE);
	if (count > len)
		count = len;
	/* dbg ("tx_data_head: %d, tx_data_tail: %d, count: %d ",
		head, tail, count); */
	spin_unlock(&up->lock);
	p = kmalloc(count, GFP_KERNEL);
	spin_lock(&up->lock);
	if (p) {
		for (i = 0; i < count; i++) {
			p[i] = p_btport->tx_data[tail];
			tail = (tail + 1) & (BRCM_BUF_SIZE - 1);
		}
		spin_unlock(&up->lock);

		if (copy_to_user(user_buffer, p, count)) {
			count = -EFAULT;
			kfree(p);
			goto out;
		}
		kfree(p);

		smp_wmb();
		spin_lock(&up->lock);
		p_btport->tx_data_tail =
		    (p_btport->tx_data_tail + count) & (BRCM_BUF_SIZE - 1);
		pr_devel("tx_data_head: %d, tx_data_tail: %d, count: %d ", head,
		    tail, count);
	} else {
		pr_warning("Failed to allocat buffer");
		count = -ENOMEM;
	}
	spin_unlock(&up->lock);
out:
	return count;
}

/*
 * write rfcomm data from BT stack to the tty/rfcommXX port
 * typically count is of BT MTU size which is for SPP/DUN 660-> 1696 bytes
 * tty locking should be good enough. so no need for lock!
 */
static ssize_t btport_write(struct file *file, const char __user *user_buffer,
			    size_t count, loff_t *ppos)
{
	struct brcm_uart_port *p_btport;
	struct tty_struct *tty;
	unsigned char *p;
	int i = 0;
	int access = 0;

	p_btport = (struct brcm_uart_port *)file->private_data;
	if (!p_btport || !p_btport->rfcomm_opened) {
		pr_err("btport_write() rfcomm/btport not opened");
		return -ENODEV;
	}

	/* check to verify that the incoming data is good */
	access = !access_ok(VERIFY_READ, user_buffer, count);
	if (access) {
		pr_err("buffer access verification failed");
		return -EFAULT;
	}
	if (!p_btport->rfcomm_port.state) {
		pr_err("port info cannot obtained");
		return -EINVAL;
	}

	tty = p_btport->rfcomm_port.state->port.tty;
	if (!tty) {
		pr_err("tty not obtained");
		return -EINVAL;
	}

	/* if nothing to send, skip the copy */
	if (!count)
		goto write_exit;

	/* limit to 64KiB which is the max size accepted by tty */
	if (count > BTPORT_MAX_WRITE_SIZE)
		count = BTPORT_MAX_WRITE_SIZE;
	pr_devel("kmalloc(%d, GFP_KERNEL);", count);
	p = kmalloc(count, GFP_KERNEL); /* would be better to use atomic! */
	if (p) {
		/*pr_devel("copy_from_user(p, user_buffer, %d);", count); */
		if (!copy_from_user(p, user_buffer, count)) {
			i = tty_insert_flip_string(tty, p, count);
			p_btport->rfcomm_port.icount.rx += i;
			count -= i;
		} else {
			i = -EFAULT;
		}
		kfree(p);
	}

write_exit:
	if (0 <= i)
		tty_flip_buffer_push(tty);
	/* let user know how much data we sent to transport  */
	return i;
}

/*
 *
 */
static unsigned int btport_poll(struct file *file, struct poll_table_struct *p_pt)
{
	struct brcm_uart_port *p_btport = file->private_data;
	struct uart_port *up;
	unsigned mask = 0;	/* always make this writable  */

	if (p_btport == NULL) {
		pr_err("error, can't find device");
		mask |= POLLHUP | POLLRDNORM;
		goto out;
	}

	if (!p_btport->rfcomm_port.state) {
		pr_debug("btport_poll() uart closed");
		mask |= POLLHUP | POLLRDNORM;
		goto out;
	}
	pr_devel("btport_poll(rfcomm_opened: %d), tx_data_head: %d, "
			"tx_data_tail: %d", p_btport->rfcomm_opened,
			p_btport->tx_data_head, p_btport->tx_data_tail);

	up = &p_btport->rfcomm_port;
	spin_lock(&up->lock);
	if (p_btport->tx_data_head == p_btport->tx_data_tail) {
		spin_unlock(&up->lock);
		poll_wait(file, &p_btport->rx_wait_q, p_pt);
		spin_lock(&up->lock);
	}

	if (p_btport->tx_data_head != p_btport->tx_data_tail)
		mask |= POLLIN | POLLRDNORM;
	spin_unlock(&up->lock);
out:
	return mask;
}

/*
 * opens a BtPortXX and allocates dynamically the corresponding
 * rfcommXX port
 * BtPort0 is handled espially. it is only used for managment
 */
static int btport_open(struct inode *inode, struct file *filp)
{
	struct brcm_uart_port *p_btport;
	int minor;
	int ret;

	minor = iminor(inode);

	pr_debug("btport_open(BtPort%d)", minor);

	if (minor >= BRCM_MAX_UART_NR)
		return -ENODEV;

	if (mutex_lock_interruptible(&btport_mutex))
		return -EINTR;
	 /* allow multiple BtPort0 open but not for BtPortXX/rfcommXX */
	if ((0 < minor) && brcm_rfcomm_ports[minor]) {
		ret = -EBUSY;
		goto out;
	}
	ret = init_rfcomm_ports(&brcm_rfcomm_ports[minor], minor);
	if (0 > ret) {
		pr_err("btport%d:init_rfcomm_ports() failed: %d",
				minor, ret);
		goto out;
	}
	p_btport = brcm_rfcomm_ports[minor];

	pr_devel("btport_open() p_btport %x", (unsigned int)p_btport);

	/* init the wait q  */
	init_waitqueue_head(&p_btport->rx_wait_q);

	pr_devel("wait queue initialized");

	p_btport->open_count++;
	p_btport->line = minor;

	p_btport->tx_data_head = 0;
	p_btport->tx_data_tail = 0;

	filp->private_data = p_btport;

	if (minor > 0) {
		pr_devel("Add rfcomm%d, up %x up->rfcomm_port %x",
		     p_btport->rfcomm_port.line, (int)p_btport,
		     (int)&p_btport->rfcomm_port);

		ret =
		    uart_add_one_port(&brcm_rfcomm_port_reg,
				      &p_btport->rfcomm_port);
		if (ret) {
			pr_err("uart_add_one_port : error %d", ret);
			free_rfcomm_ports(&brcm_rfcomm_ports[minor]);
			mutex_unlock(&btport_mutex);
			return -ENODEV;
		}
		init_waitqueue_head(&p_btport->port_wait_q);
	}

	pr_devel("### btport%d opened (open_count %d) ###", minor,
	    p_btport->open_count);
out:
	mutex_unlock(&btport_mutex);
	return 0;
}

/*
 * Closes BtPortXX and removes corresponding rfcommXX uart
 */
static int btport_release(struct inode *inode, struct file *filp)
{
	struct brcm_uart_port *p_btport =
	    (struct brcm_uart_port *)filp->private_data;
	int minor;

	/* already closed/freed/never opened, silently return */
	if (!p_btport) {
		pr_info("btport_release(): btport already closed");
		return 0;
	}
	minor = iminor(inode);

	if (mutex_lock_interruptible(&btport_mutex))
		return -ERESTARTSYS;

	/* BtPortXX are only single instance, check if already freed.
	 * check for null brcm_rfcomm_ports[minor], there is a minimal
	 * race condition before mutex call */
	if ((minor > 0) && brcm_rfcomm_ports[minor]) {
		pr_devel("btport_release() unregister rfcomm port (minor:%d)",
				minor);
		uart_remove_one_port(&brcm_rfcomm_port_reg,
					 &p_btport->rfcomm_port);
	}
#ifdef BTPORT_ASYNC_SUPPORT
	if (p_btport->fasync)
		btport_fasync(-1, filp, 0);
#endif
	if (p_btport->open_count)
		p_btport->open_count--;
	pr_devel("btport_release() open_count %d", p_btport->open_count);

	/* ensure there is no process hanging on poll / select  */
	wake_up_interruptible(&p_btport->rx_wait_q);

	pr_devel("### btport_release() btport%d : done ! ###", p_btport->line);

#ifdef BRCM_CREATE_BTPORT_DYN
	/* open_count should always be 0 for XX > 0
	 * BtPort0 is freed in brcm_bt_exit() */
	if ((minor > 0) && brcm_rfcomm_ports[minor] &&
			(0 == p_btport->open_count)) {
		device_destroy(gBrcmBtport.btPortClass,
				MKDEV(MAJOR(gBrcmBtport.btPortDevNum),
				MINOR(gBrcmBtport.btPortDevNum)+
				minor));
		gBrcmBtport.nrDevices--;
	}
	/* BtPort0 should only be freed if open_count reached 0! */
	if (0 == p_btport->open_count)
#else
	if (minor > 0)
#endif
		free_rfcomm_ports(&brcm_rfcomm_ports[minor]);
	mutex_unlock(&btport_mutex);
	return 0;
}

#ifdef BTPORT_ASYNC_SUPPORT
/*
 * async signalling
 */
static int btport_fasync(int fd, struct file *file, int on)
{
	int retval;
	struct brcm_uart_port *p_btport = file->private_data;

	pr_devel("btport_fasync(fd %d, on %d)", fd, on);

	if (p_btport == NULL) {
		pr_err("error, can't find device");
		return -ENODEV;
	}

	retval = fasync_helper(fd, file, on, &p_btport->fasync);
	return retval < 0 ? retval : 0;
}
#endif

static long create_btport_rfcomm(struct brcm_uart_port *p_btport,
				void __user *arg)
{
	struct rfcomm_dev_info di;
	long	rc = 0;
#ifdef BRCM_CREATE_BTPORT_DYN
	struct device *dev;
#endif
	if (copy_from_user(&di, arg, sizeof(struct rfcomm_dev_info))) {
		pr_err("create_btport_rfcomm(): Failed getting user data");
		return -EFAULT;
	}
	if (mutex_lock_interruptible(&btport_mutex))
		return -EINTR;
	pr_debug("create_btport_rfcomm(di.id:%d, di.channel, %d)", di.id,
			di.channel);
#ifdef BRCM_CREATE_BTPORT_DYN
	if ((0 == di.channel) || (di.channel >= BRCM_MAX_UART_NR) ||
			(brcm_rfcomm_ports[di.channel])) {
		pr_warning("create_btport_rfcomm() invalid params, already "
				"allocated, channel: %d (max: %d)",
				di.channel, BRCM_MAX_UART_NR);
		rc = brcm_rfcomm_ports[di.channel] ? -EEXIST : -EINVAL;
		goto out;
	}
	dev = device_create(gBrcmBtport.btPortClass, NULL,
			MKDEV(MAJOR(gBrcmBtport.btPortDevNum),
			MINOR(gBrcmBtport.btPortDevNum)+di.channel),
			NULL, BRCM_BTPORT_FMT_NAME, di.channel);
	if (IS_ERR(dev)) {
		pr_err("create_btport_rfcomm(): Failed to create dev %s%d",
				BRCM_BTPORT_NAME, di.channel);
		rc = PTR_ERR(dev);
	}
	if (rc >= 0)
		gBrcmBtport.nrDevices++;
	pr_devel("create_btport_rfcomm() rc: %ld, nrDevices: %d", rc,
			gBrcmBtport.nrDevices);
out:
#endif
	mutex_unlock(&btport_mutex);
	if (rc >= 0)
		return 0;
	else
		return (long)rc;
}

/*
 * BtPortXX ioctl handler
 * BtPort0 managment port: creation of new BtPortXX device
 * BtPortXX flow control handling from BT stack side
 */
static long btport_ioctl(struct file *file, unsigned int cmd,
						unsigned long arg)
{
	struct brcm_uart_port *p_btport;
	struct uart_port *up;
	struct btLinuxPortEvent saved_event;
	long retval = 0;
	uint8_t mctrl[2]; /* [0]: signals, [1]: values */

	/* dbg ("cmd %x", cmd); */

	p_btport = (struct brcm_uart_port *)file->private_data;
	if (p_btport == NULL) {
		pr_err("can't find device");
		return -ENODEV;
	}
	pr_devel("Received command %d, RFCOMMGETDEVLIST = %d", cmd,
			RFCOMMGETDEVLIST);
	pr_devel("Received command %d, RFCOMMCREATEDEV = %d", cmd,
			RFCOMMCREATEDEV);

	/* Compatible with blueZ socket first */
	switch (cmd) {

	case RFCOMMCREATEDEV:
		pr_devel("RFCOMMCREATEDEV");
		if (p_btport->line == 0)
			return create_btport_rfcomm(p_btport,
					(void __user *)arg);
		return rfcomm_set_di(p_btport, (void __user *)arg);
		break;

	case RFCOMMGETDEVLIST:
		pr_devel("RFCOMMGETDEVLIST");
		return rfcomm_get_dev_list((void __user *)arg);
		break;

	case RFCOMMGETDEVINFO:
		pr_devel("RFCOMMGETDEVINFO");
		return rfcomm_get_di(p_btport, (void __user *)arg);
		break;
	}
	spin_lock(&p_btport->lock);
	if (!p_btport->rfcomm_opened) {
		spin_unlock(&p_btport->lock);
		pr_debug("port not opened");
		return -ENODEV;
	}
	spin_unlock(&p_btport->lock);

	up = &p_btport->rfcomm_port;

	switch (cmd) {

	case IOCTL_BTPORT_GET_EVENT:
		pr_devel("IOCTL_BTPORT_GET_EVENT");
		/* prevent interruption from uart driver side */
		spin_lock(&up->lock);
		saved_event.eventCode = p_btport->saved_event.eventCode;
		saved_event.modemControlReg =
				p_btport->saved_event.modemControlReg;
		spin_unlock(&up->lock);
		if (copy_to_user
		    ((void __user *)arg, &saved_event, sizeof(saved_event))) {
			pr_err("copy_to_user failed");
			retval = -EFAULT;
			goto err_out;
		}
		break;

	case IOCTL_BTPORT_SET_EVENT_RESULT:
		pr_devel("IOCTL_BTPORT_SET_EVENT_RESULT");
		spin_lock(&up->lock);
		p_btport->set_event[SET_EVENT_RESULT] = 0;
		p_btport->set_event[SET_EVENT_DATA] = 0;
		p_btport->flag = 1;
		spin_unlock(&up->lock);
		if (copy_from_user
		    (&p_btport->set_event, (void __user *)arg,
				    sizeof(p_btport->set_event))) {
			retval = -EFAULT;
			pr_err("IOCTL_BTPORT_SET_EVENT_RESULT failed");
			wake_up_interruptible(&p_btport->port_wait_q);
			goto err_out;
		}
		pr_devel("fetched event result %d, ev data 0x%x",
				p_btport->set_event[SET_EVENT_RESULT],
				p_btport->set_event[SET_EVENT_DATA]);
		wake_up_interruptible(&p_btport->port_wait_q);
		pr_devel("signal wait queue");
		break;

	case IOCTL_BTPORT_HANDLE_MCTRL:
		pr_devel("IOCTL_BTPORT_HANDLE_MCTRL");
		if (copy_from_user(&mctrl, (void __user *)arg,
				sizeof(mctrl))) {
			retval = -EFAULT;
			pr_err("IOCTL_BTPORT_HANDLE_MCTRL failed 1st param");
			goto err_out;
		}
		btlinux_handle_mctrl(p_btport, mctrl[0], mctrl[1]);
		break;
	}
err_out:
	pr_devel("BTLINUXPORT btport_ioctl returning %ld", retval);
	return retval;
}

/**/
static const struct file_operations brcm_btport_fops = {
	.owner = THIS_MODULE,
	.read = btport_read,
	.write = btport_write,
	.poll = btport_poll,
	.open = btport_open,
	.release = btport_release,
#ifdef BTPORT_ASYNC_SUPPORT
	.fasync = btport_fasync,
#else
	.fasync = NULL,
#endif
	.unlocked_ioctl = btport_ioctl,
};

/*
 *
 * RFCOMM/uart related ops
 *
 */
static unsigned int brcm_rfcomm_tx_empty(struct uart_port *uport)
{
	struct circ_buf *xmit = &uport->state->xmit;

	if (uart_circ_empty(xmit)) {
		pr_devel("tx buffer empty");
		return TIOCSER_TEMT;
	}
	return 0;
}

/*
 * set modem signals rfcommXX -> BtPortXX, signal it to BT stack
 *  called with uport->locked and irq disabled!
 */
static void brcm_rfcomm_set_mctrl(struct uart_port *uport, unsigned int mctrl)
{
	struct brcm_uart_port *p = (struct brcm_uart_port *)uport->private_data;

	if (!p) {
		pr_warning("brcm_rfcomm_set_mctrl() rfcomm/btport not opened");
		return;
	}

	pr_debug("brcm_rfcomm_set_mctrl(mctrl: 0x%x), p: %x, p->flag: %x",
			mctrl, (unsigned int)p, p->flag);
	p->saved_event.eventCode = BTLINUX_PORT_MODEM_CONTROL;
	p->saved_event.modemControlReg = 0;

	if (mctrl & TIOCM_DTR) {
		/* pr_devel(" DTR SET"); */
		p->saved_event.modemControlReg |=
		    MODEM_CNTRL_DTRDSR_MASK;
	} else {
		/* pr_devel(" DTR CLR"); */
	}

	if (mctrl & TIOCM_RTS) {
		/* pr_devel(" RTS SET"); */
		p->saved_event.modemControlReg |=
		    MODEM_CNTRL_CTSRTS_MASK;
	} else {
		/* pr_devel(" RTS CLR"); */
	}

	/* TIOCM_CD force it on all the time to avoid dealock from rfcomm/tty
	 * side if CLOCAL is cleared! */
	p->saved_event.modemControlReg |= MODEM_CNTRL_CD_MASK;

	if (mctrl & TIOCM_RI) {
		/* pr_devel(" RI SET"); */
		p->saved_event.modemControlReg |= MODEM_CNTRL_RI_MASK;
	} else {
		/* pr_devel(" RI CLR"); */
	}

#ifdef BTPORT_ASYNC_SUPPORT
	if (p->fasync) {
		pr_devel("notify application using fasync (fd %d)",
		    p->fasync->fa_fd);

		/* notify application */
		kill_fasync(&p->fasync, SIGUSR1, POLL_MSG);

		/* sync wait */
		wait_event_interruptible(p->port_wait_q, p->flag != 0);
		pr_devel("wait done");
	} else {
		pr_debug("brcm_rfcomm_set_mctrl() No fasync requested.");
	}
#endif
}

/*
 * read modem signals from BtPortXX (Bt stack side) by SIGUSR1 mechanism
 * called with uport->locked and irq disabled!
 */
static unsigned int brcm_rfcomm_get_mctrl(struct uart_port *uport)
{
	unsigned int ret = 0;
	struct brcm_uart_port *p = (struct brcm_uart_port *)uport->private_data;

	if (!p || !p->rfcomm_opened) {
		pr_warning("brcm_rfcomm_get_mctrl() rfcomm/port is not opened");
		return -EINVAL;
	}

#ifdef BTPORT_ASYNC_SUPPORT
	pr_debug("brcm_rfcomm_get_mctrl(line: %d), p->fasync: 0x%p", p->line,
			p->fasync);
#else
	pr_debug("brcm_rfcomm_get_mctrl(line: %d), NO fasync", p->line);
#endif
	p->saved_event.eventCode = BTLINUX_PORT_MODEM_STATUS;
#ifdef BTPORT_ASYNC_SUPPORT
	/* notify application. this can not really work because irq is disabled
	 * and lock is taken. */
	if (p->fasync) {
		kill_fasync(&p->fasync, SIGUSR1, POLL_MSG);
		/* sync wait interruptible to avoid hanging if process is
		 * killed */
		if (wait_event_interruptible(p->port_wait_q, p->flag != 0)) {
			ret = -EINTR;
			goto out;
		}

		p->flag = 0;
		if (p->set_event[SET_EVENT_RESULT] == 0) {
			pr_warning("brcm_rfcomm_get_mctrl() bad return from "
					"PORT_GetModemStatus");
			ret = -EINVAL;
			goto out;
		}
		/* pr_devel("fetched modem status ""
			"(from userspace stack) 0x%x",
		   up->set_event[SET_EVENT_DATA]); */

		if (p->set_event[SET_EVENT_DATA] & PORT_DTRDSR_ON)
			ret |= TIOCM_DSR;

		if (p->set_event[SET_EVENT_DATA] & PORT_CTSRTS_ON)
			ret |= TIOCM_CTS;

		if (p->set_event[SET_EVENT_DATA] & PORT_RING_ON)
			ret |= TIOCM_RI;

		/* you may need to remap to BTLINUX_TIOCM_CD */
		/* if platform uses a different bit for CD ! */
		if (p->set_event[SET_EVENT_DATA] & PORT_DCD_ON)
			ret |= TIOCM_CD;
	} else {
		if (p->saved_event.
		    modemControlReg & MODEM_CNTRL_DTRDSR_MASK)
			ret |= TIOCM_DSR;
		if (p->saved_event.
		    modemControlReg & MODEM_CNTRL_CTSRTS_MASK)
			ret |= TIOCM_CTS;
		if (p->saved_event.
		    modemControlReg & MODEM_CNTRL_CD_MASK)
			ret |= TIOCM_CD;
		if (p->saved_event.
		    modemControlReg & MODEM_CNTRL_RI_MASK)
			ret |= TIOCM_RI;
		/* pr_devel("Default modem status 0x%x", ret); */
	}
out:
#else
	if (p->saved_event.
	    modemControlReg & MODEM_CNTRL_DTRDSR_MASK)
		ret |= TIOCM_DSR;
	if (p->saved_event.
	    modemControlReg & MODEM_CNTRL_CTSRTS_MASK)
		ret |= TIOCM_CTS;
	if (p->saved_event.
	    modemControlReg & MODEM_CNTRL_CD_MASK)
		ret |= TIOCM_CD;
	if (p->saved_event.
	    modemControlReg & MODEM_CNTRL_RI_MASK)
		ret |= TIOCM_RI;
	/* pr_devel("Default modem status 0x%x", ret); */
#endif
	pr_debug("brcm_rfcomm_get_mctrl() ret: 0x%x", ret);
	return ret;
}

static void brcm_rfcomm_stop_tx(struct uart_port *uport)
{
	pr_debug("brcm_rfcomm_stop_tx() not implemented");
}

/*
 * triggered by a rfcommXX write
 * trigger a read to BtPortXX side, called with uport->lock and irq disabled
 */
static void brcm_rfcomm_start_tx(struct uart_port *uport)
{
	struct brcm_uart_port *p = (struct brcm_uart_port *)uport->private_data;
	struct circ_buf *xmit;
	int count;

	if (!p || !p->rfcomm_opened) {
		pr_err("brcm_rfcomm_start_tx(() rfcomm/btport is not opened");
		return;

	}
	pr_devel("brcm_rfcomm_start_tx()");

	xmit = &p->rfcomm_port.state->xmit;

	count = uart_circ_chars_pending(xmit);
	pr_devel("line %d - buf size = %d", p->line, count);

	if (uport->x_char) {
		pr_devel("x_char is transmitted");
		uport->icount.tx++;
		uport->x_char = 0;
		return;
	}

	if (uart_circ_empty(xmit) || uart_tx_stopped(uport)) {
		pr_devel("tx buffer empty or tx stopped");
		brcm_rfcomm_stop_tx(uport);
		return;
	}

	send_tx_char(p);

	/* wake up application waiting in a poll */
	wake_up_interruptible(&p->rx_wait_q);
}

static void brcm_rfcomm_stop_rx(struct uart_port *uport)
{
	pr_debug("brcm_rfcomm_stop_rx() not implemented");
}

static void brcm_rfcomm_enable_ms(struct uart_port *uport)
{
	pr_debug("brcm_rfcomm_enable_ms() not implemented");
}

static void brcm_rfcomm_break_ctl(struct uart_port *uport, int break_state)
{
	pr_debug("brcm_rfcomm_break_ctl() not implemented");
}

/*
 * user app opens rfcommXX
 * initialisse modem signals
 */
static int brcm_rfcomm_startup(struct uart_port *uport)
{
	struct brcm_uart_port *p =
			(struct brcm_uart_port *)uport->private_data;
	struct tty_struct *tty;

	if (!p)
		return -ENODEV;

	pr_debug("brcm_rfcomm_startup(): uart_port: %x, line: %d",
			(unsigned int)p,
		    p->rfcomm_port.line);

	tty = uport->state->port.tty;
	if (!tty) {
		pr_err("Failed to get tty pointer");
		return -EINVAL;
	}

	spin_lock(&p->lock);
	p->rfcomm_opened = 1;
	spin_unlock(&p->lock);

	/* use low latency for performance as driver is not running in irq
	 * context */
	tty->low_latency = 1;
	tty->icanon = 0;

	/* this DCE view. CD an input on rfcommXX! */
	p->saved_event.eventCode = BTLINUX_PORT_OPENED;
	p->saved_event.modemControlReg |= MODEM_CNTRL_DTRDSR_MASK;
	p->saved_event.modemControlReg |= MODEM_CNTRL_CTSRTS_MASK;
	p->saved_event.modemControlReg |= MODEM_CNTRL_CD_MASK;

	pr_devel("tty magic: %d, index: %d, name: %s", tty->magic,
	    tty->index, tty->name);
	pr_devel("rfcomm_opened: %d", p->rfcomm_opened);
	/* uart_write_wakeup(&p->rfcomm_port); */
	tty_wakeup(tty);

	return 0;
}

/*
 * rfcommXX is beging closed.
 */
static void brcm_rfcomm_shutdown(struct uart_port *uport)
{
	struct brcm_uart_port *p;

	if (!uport || !uport->private_data) {
		pr_info("brcm_rfcomm_shutdown() btport/rfcomm already down?");
		return;
	}
	p = (struct brcm_uart_port *)uport->private_data;
	spin_lock(&p->lock);
	pr_debug("brcm_rfcomm_shutdown(brcm_uart_port->line: %hd, "
			"rfcomm_opened: %d)", p->line,
			p->rfcomm_opened);
	if (!p->rfcomm_opened)
		pr_info("brcm_rfcomm_shutdown() rfcomm line %hd already"
				" closed", p->line);
	else
		p->rfcomm_opened = 0;
	spin_unlock(&p->lock);
}

/*
 *
 */
static void brcm_rfcomm_set_termios(struct uart_port *uport,
				    struct ktermios *termios,
				    struct ktermios *old)
{
	struct tty_struct *tty;

	tty = uport->state->port.tty;

	if (tty) {
		if (mutex_lock_interruptible(&tty->termios_mutex))
			return;
		pr_devel("brcm_rfcomm_set_termios(line: %d)",
			((struct brcm_uart_port *)uport->private_data)->line);

		memcpy(tty->termios, termios, sizeof(*termios));
		mutex_unlock(&tty->termios_mutex);
	}
}

/*
 * as long open, no sleep.
 */
static void brcm_rfcomm_pm(struct uart_port *uport, unsigned int state,
			   unsigned int oldstate)
{
	pr_debug("brcm_rfcomm_pm() not implemented");
}

static const char *brcm_rfcomm_type(struct uart_port *uport)
{
	if (uport->line == BRCM_RFCOMM_DUN)
		return "brcm DUN";
	if (uport->line == BRCM_RFCOMM_SPP)
		return "brcm SPP";

	return "brcm RFCOMM";
}


static void brcm_rfcomm_release_port(struct uart_port *uport)
{
	pr_debug("brcm_rfcomm_release_port() not implemented, uport: x%p",
			uport);
	if (uport)
		pr_debug("brcm_rfcomm_release_port(line: %d)", uport->line);
}


static void brcm_rfcomm_config_port(struct uart_port *uport, int flags)
{
	struct brcm_uart_port *p = uport->private_data;

	spin_lock(&p->lock);
	p->rfcomm_port.type = BRCM_RFCOMM_TYPE;
	spin_unlock(&p->lock);
	pr_debug("brcm_rfcomm_config_port(type: %d), flags: 0x%x",
			p->rfcomm_port.type, flags);
}


static int brcm_rfcomm_verify_port(struct uart_port *uport,
				   struct serial_struct *ser)
{
	struct tty_struct *tty = uport->state->port.tty;
	pr_debug("brcm_rfcomm_verify_port(): type: %d, line: %d, port: %d, "
			"flags: %d", ser->type, ser->line, ser->port,
			ser->flags);
	if (tty)
		pr_debug("tty: closing: %c, port_count: %d, port_blocked_open:"
				" %d", tty->closing, tty->port->count,
				tty->port->blocked_open);
	return 0;
}

/*
 * minimal uart/tty ioctl support
 */
static int brcm_rfcomm_ioctl(struct uart_port *uport, unsigned int cmd,
			     unsigned long arg)
{
	struct tty_struct *tty;
	int ret = 0;
	void __user *argp = (void __user *)arg; /* see tty_ioctl.c */

	tty = uport->state->port.tty;

	if (!tty) {
		pr_warning("brcm_rfcomm_ioctl():Failed to get tty pointer");
		return 0;
	}

	switch (cmd) {
	case TCGETS:
		if (mutex_lock_interruptible(&tty->termios_mutex))  {
			ret = -EINTR;
			break;
		}
		if (copy_to_user(argp, tty->termios, sizeof(struct termios))) {
			pr_warning("brcm_rfcomm_ioctl() TCGETS failed");
			ret = -EFAULT;
		}
		mutex_unlock(&tty->termios_mutex);
		break;

	case TCSETS:
		if (mutex_lock_interruptible(&tty->termios_mutex)) {
			ret = -EINTR;
			break;
		}
		if (copy_from_user(tty->termios, argp,
					sizeof(struct termios))) {
			pr_warning("brcm_rfcomm_ioctl() TCGETS failed");
			ret = -EFAULT;
		}
		mutex_unlock(&tty->termios_mutex);
		break;

	case TIOCSBRK:
		pr_warning("brcm_rfcomm_ioctl(TIOCSBRK) unhandled");
		break;

	case TIOCCBRK:
		pr_warning("brcm_rfcomm_ioctl(TIOCCBRK) unhandled");
		break;

	default:
		return -ENOIOCTLCMD;
	}
	return ret;

}

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
	.nr = BRCM_MAX_UART_NR,
	.cons = NULL,
};

static const struct uart_ops brcm_rfcomm_pops = {
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

/* *port maybe NULL already, no need check as underlying code accepts
 NULL obj */
static void free_rfcomm_ports(struct brcm_uart_port **port)
{
	kfree(*port);
	*port = NULL;
}

static int init_rfcomm_ports(struct brcm_uart_port **port, unsigned int i)
{
	int ret = -ENOMEM;
	struct brcm_uart_port *p;

	if (*port) {
		pr_info("init_rfcomm_ports(port%d) already allocated", i);
		return i;
	}
	p = (struct brcm_uart_port *)
		kzalloc((sizeof(struct brcm_uart_port)),
				GFP_KERNEL);
	if (!p) {
		*port = p;
		return ret;
	}

	/* BtPort0 does not have any uart associated as used for management */
	if (0 == i)
		goto done;
	p->rfcomm_port.line = i;
	p->rfcomm_port.ops = &brcm_rfcomm_pops;
	p->rfcomm_port.iobase = 0x1;
	p->rfcomm_port.mapbase = 'C';	/*  0x45; */
	p->rfcomm_port.membase = (uint8_t __iomem *)'K'; /* 0x55; */
	p->rfcomm_port.flags |= UPF_BOOT_AUTOCONF;
	p->rfcomm_port.private_data = p;
	p->saved_event.modemControlReg = 0;
done:
	spin_lock_init(&p->lock);
	ret = (int)i;
	*port = p;
	return ret;
}

/*
 * register uart driver and preset rfcomm btport structure
 */
static int brcm_init_rfcomm_ports(struct uart_driver *drv,
		struct brcm_uart_port **pp_ports)
{
	int ret = 0;

	pr_debug("brcm_init_rfcomm_ports() register uart drv");
	ret = uart_register_driver(drv);

	if (ret) {
		pr_err("Failed to register rfcomm driver ret: %d for nr:%d",
				ret, drv->nr);
		goto out;
	}
	memset(pp_ports, 0, sizeof(*pp_ports)*BRCM_MAX_UART_NR);
out:
	return ret;
}

/*
 * create BtPort device. BtPort0 managemen port is allocated
 */
static int __init brcm_bt_init(void)
{
	int ret = 0;
	struct device *dev;

	pr_info("Loading BRCM rfcomm driver %s", DRIVER_VERSION);
	memset(&gBrcmBtport, 0, sizeof(gBrcmBtport));
	/* Allocate a major number dynamically */
	ret = alloc_chrdev_region(&gBrcmBtport.btPortDevNum, 0,
				BRCM_MAX_UART_NR, BRCM_BTPORT_NAME);
	if (ret < 0) {
		pr_err("brcm_bt_init() unable to get major number: %d", ret);
		return ret;
	}

	cdev_init(&gBrcmBtport.btPortCdev, &brcm_btport_fops);
	gBrcmBtport.btPortCdev.owner = THIS_MODULE;

	ret = cdev_add(&gBrcmBtport.btPortCdev, gBrcmBtport.btPortDevNum,
				BRCM_MAX_UART_NR);
	if (ret < 0) {
		pr_err("BtPort: cdev_add failed: %d", ret);
		goto out_unregister;
	}

	/* Now that we've added the device, create a class, so that udev will
	 * make the /dev entry */
	gBrcmBtport.btPortClass = class_create(THIS_MODULE, BRCM_BTPORT_NAME);
	if (IS_ERR(gBrcmBtport.btPortClass)) {
		pr_err("BtPort: Unable to create class");
		ret = -1;
		goto out_cdev_del;
	}

	/* create n startup devices. BtPort0 can be used to add more */
	dev = device_create(gBrcmBtport.btPortClass, NULL,
				gBrcmBtport.btPortDevNum,
				NULL, BRCM_BTPORT_NAME "0");
	gBrcmBtport.nrDevices = 1;

	ret = brcm_init_rfcomm_ports(&brcm_rfcomm_port_reg,
			&brcm_rfcomm_ports[0]);
	if (ret)
		goto out_rfc_failed;
#ifndef BRCM_CREATE_BTPORT_DYN
	/* always allocate BtPort0 for control purposes */
	ret = init_rfcomm_ports(&brcm_rfcomm_ports[0], 0);
	if (0 > ret) {
		pr_err("BtPort%d:init_rfcomm_ports() failed: %d", 0, ret);
		uart_unregister_driver(&brcm_rfcomm_port_reg);
		goto out_rfc_failed;
	}
#endif
	pr_devel("BRCM rfcomm initialized, OK, nrDevices %d",
			gBrcmBtport.nrDevices);
	goto done;

out_rfc_failed:
	pr_warn("BRCM BtPort/rfcomm initialisation failed");
	device_destroy(gBrcmBtport.btPortClass, gBrcmBtport.btPortDevNum);
	gBrcmBtport.nrDevices = 0;
	class_destroy(gBrcmBtport.btPortClass);

out_cdev_del:
	cdev_del(&gBrcmBtport.btPortCdev);

out_unregister:
	unregister_chrdev_region(gBrcmBtport.btPortDevNum, BRCM_MAX_UART_NR);

done:
	return ret;
}

/*
 * free all rfcommX and BtPortX (X>0)
 */
static int free_btport_rfcomm_ports(struct uart_driver *drv,
		struct brcm_uart_port **pp_ports)
{
	int ret = 0, i;

	pr_debug("brcm_free_rfcomm_ports() de-register uart drv");
	for (i = 1; i < BRCM_MAX_UART_NR; i++) {
		if (*(pp_ports+i)) {
			uart_remove_one_port(drv,
					     &(((*pp_ports+i))
						->rfcomm_port));
			free_rfcomm_ports(pp_ports+i);
#ifdef BRCM_CREATE_BTPORT_DYN
			device_destroy(gBrcmBtport.btPortClass,
					MKDEV(MAJOR(gBrcmBtport.btPortDevNum),
					MINOR(gBrcmBtport.btPortDevNum)+
					i));
			gBrcmBtport.nrDevices--;
#endif
		}
	}
	uart_unregister_driver(drv);
	free_rfcomm_ports(pp_ports);	/* BtPort0 */
	return ret;
}

/*
 * free all allocated buffer and remove all devices BtPortXX and rfcommXX from
 * /dev/
 */
static void __exit brcm_bt_exit(void)
{
	pr_info("Unloading BRCM rfcomm driver %s, nrDevices: %d",
			DRIVER_VERSION, gBrcmBtport.nrDevices);

	free_btport_rfcomm_ports(&brcm_rfcomm_port_reg,
				&brcm_rfcomm_ports[0]);
	/* only BtPort0 should be defined! */
	if (gBrcmBtport.nrDevices > 1)
		pr_warning("BtPort nrDevices should be 1");
	pr_debug("Dev nr: %d, MAJOR(%d), MINOR(%d),",
			gBrcmBtport.nrDevices,
			MAJOR(gBrcmBtport.btPortDevNum),
			MINOR(gBrcmBtport.btPortDevNum));
	device_destroy(gBrcmBtport.btPortClass,
			MKDEV(MAJOR(gBrcmBtport.btPortDevNum),
			MINOR(gBrcmBtport.btPortDevNum)));
	class_destroy(gBrcmBtport.btPortClass);

	cdev_del(&gBrcmBtport.btPortCdev);

	unregister_chrdev_region(gBrcmBtport.btPortDevNum, BRCM_MAX_UART_NR);
	pr_devel("BRCM rfcomm uninitialized OK");
}

module_init(brcm_bt_init);
module_exit(brcm_bt_exit);

/* Module information */
MODULE_AUTHOR("R Lendenmann <rlendenmann@broadcom.com>"
		"-Chinda Keodouangsy <chindak@broadcom.com>");
MODULE_DESCRIPTION("BRCM RFCOMM port driver");
MODULE_LICENSE("GPL");
