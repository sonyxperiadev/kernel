// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Microchip Technology
 */
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/usb.h>
#include <linux/crc32.h>
#include <linux/signal.h>
#include <linux/slab.h>
#include <linux/if_vlan.h>
#include <linux/uaccess.h>
#include <linux/linkmode.h>
#include <linux/list.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/mdio.h>
#include <linux/phy.h>
#include <net/ip6_checksum.h>
#include <net/vxlan.h>
#include <linux/interrupt.h>
#include <linux/irqdomain.h>
#include <linux/irq.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/microchipphy.h>
#include <linux/phy_fixed.h>
#include <linux/of_mdio.h>
#include <linux/of_net.h>
#include <linux/string.h>
#include "lan78xx.h"

#define DRIVER_AUTHOR	"WOOJUNG HUH <woojung.huh@microchip.com>"
#define DRIVER_DESC	"LAN78XX USB 3.0 Gigabit Ethernet Devices"
#define DRIVER_NAME	"lan78xx"

#define TX_TIMEOUT_JIFFIES		(5 * HZ)
#define THROTTLE_JIFFIES		(HZ / 8)
#define UNLINK_TIMEOUT_MS		3

#define RX_MAX_QUEUE_MEMORY		(60 * 1518)

#define SS_USB_PKT_SIZE			(1024)
#define HS_USB_PKT_SIZE			(512)
#define FS_USB_PKT_SIZE			(64)

#define MAX_RX_FIFO_SIZE		(12 * 1024)
#define MAX_TX_FIFO_SIZE		(12 * 1024)

#define FLOW_THRESHOLD(n)		((((n) + 511) / 512) & 0x7F)
#define FLOW_CTRL_THRESHOLD(on, off)	((FLOW_THRESHOLD(on)  << 0) | \
					 (FLOW_THRESHOLD(off) << 8))

/* Flow control turned on when Rx FIFO level rises above this level (bytes) */
#define FLOW_ON_SS			9216
#define FLOW_ON_HS			8704

/* Flow control turned off when Rx FIFO level falls below this level (bytes) */
#define FLOW_OFF_SS			4096
#define FLOW_OFF_HS			1024

#define DEFAULT_BURST_CAP_SIZE		(MAX_TX_FIFO_SIZE)
#define DEFAULT_BULK_IN_DELAY		(0x0800)
#define MAX_SINGLE_PACKET_SIZE		(9000)
#define DEFAULT_TX_CSUM_ENABLE		(true)
#define DEFAULT_RX_CSUM_ENABLE		(true)
#define DEFAULT_TSO_CSUM_ENABLE		(true)
#define DEFAULT_VLAN_FILTER_ENABLE	(true)
#define DEFAULT_VLAN_RX_OFFLOAD		(true)
#define TX_OVERHEAD			(8)
#define TX_ALIGNMENT			(4)
#define RXW_PADDING			2

#define LAN78XX_USB_VENDOR_ID		(0x0424)
#define LAN7800_USB_PRODUCT_ID		(0x7800)
#define LAN7850_USB_PRODUCT_ID		(0x7850)
#define LAN7801_USB_PRODUCT_ID		(0x7801)
#define LAN78XX_EEPROM_MAGIC		(0x78A5)
#define LAN78XX_OTP_MAGIC		(0x78F3)
#define AT29M2AF_USB_VENDOR_ID		(0x07C9)
#define AT29M2AF_USB_PRODUCT_ID	(0x0012)

#define	MII_READ			1
#define	MII_WRITE			0

#define EEPROM_INDICATOR		(0xA5)
#define EEPROM_MAC_OFFSET		(0x01)
#define MAX_EEPROM_SIZE			512
#define OTP_INDICATOR_1			(0xF3)
#define OTP_INDICATOR_2			(0xF7)

#define WAKE_ALL			(WAKE_PHY | WAKE_UCAST | \
					 WAKE_MCAST | WAKE_BCAST | \
					 WAKE_ARP | WAKE_MAGIC)

#define TX_URB_NUM			10
#define TX_SS_URB_NUM			TX_URB_NUM
#define TX_HS_URB_NUM			TX_URB_NUM
#define TX_FS_URB_NUM			TX_URB_NUM

/* USB related defines */
#define BULK_IN_PIPE			1
#define BULK_OUT_PIPE			2

/* default autosuspend delay (mSec)*/
#define DEFAULT_AUTOSUSPEND_DELAY	(10 * 1000)

/* statistic update interval (mSec) */
#define STAT_UPDATE_TIMER		(1 * 1000)

/* time to wait for MAC or FCT to stop (jiffies) */
#define HW_DISABLE_TIMEOUT		(HZ / 10)

/* time to wait between polling MAC or FCT state (ms) */
#define HW_DISABLE_DELAY_MS		1

/* defines interrupts from interrupt EP */
#define MAX_INT_EP			(32)
#define INT_EP_INTEP			(31)
#define INT_EP_OTP_WR_DONE		(28)
#define INT_EP_EEE_TX_LPI_START		(26)
#define INT_EP_EEE_TX_LPI_STOP		(25)
#define INT_EP_EEE_RX_LPI		(24)
#define INT_EP_MAC_RESET_TIMEOUT	(23)
#define INT_EP_RDFO			(22)
#define INT_EP_TXE			(21)
#define INT_EP_USB_STATUS		(20)
#define INT_EP_TX_DIS			(19)
#define INT_EP_RX_DIS			(18)
#define INT_EP_PHY			(17)
#define INT_EP_DP			(16)
#define INT_EP_MAC_ERR			(15)
#define INT_EP_TDFU			(14)
#define INT_EP_TDFO			(13)
#define INT_EP_UTX			(12)
#define INT_EP_GPIO_11			(11)
#define INT_EP_GPIO_10			(10)
#define INT_EP_GPIO_9			(9)
#define INT_EP_GPIO_8			(8)
#define INT_EP_GPIO_7			(7)
#define INT_EP_GPIO_6			(6)
#define INT_EP_GPIO_5			(5)
#define INT_EP_GPIO_4			(4)
#define INT_EP_GPIO_3			(3)
#define INT_EP_GPIO_2			(2)
#define INT_EP_GPIO_1			(1)
#define INT_EP_GPIO_0			(0)

#define USB_CTRL_GET_TIMEOUT_LOCAL 250
#define USB_CTRL_SET_TIMEOUT_LOCAL 250

static const char lan78xx_gstrings[][ETH_GSTRING_LEN] = {
	"RX FCS Errors",
	"RX Alignment Errors",
	"Rx Fragment Errors",
	"RX Jabber Errors",
	"RX Undersize Frame Errors",
	"RX Oversize Frame Errors",
	"RX Dropped Frames",
	"RX Unicast Byte Count",
	"RX Broadcast Byte Count",
	"RX Multicast Byte Count",
	"RX Unicast Frames",
	"RX Broadcast Frames",
	"RX Multicast Frames",
	"RX Pause Frames",
	"RX 64 Byte Frames",
	"RX 65 - 127 Byte Frames",
	"RX 128 - 255 Byte Frames",
	"RX 256 - 511 Bytes Frames",
	"RX 512 - 1023 Byte Frames",
	"RX 1024 - 1518 Byte Frames",
	"RX Greater 1518 Byte Frames",
	"EEE RX LPI Transitions",
	"EEE RX LPI Time",
	"TX FCS Errors",
	"TX Excess Deferral Errors",
	"TX Carrier Errors",
	"TX Bad Byte Count",
	"TX Single Collisions",
	"TX Multiple Collisions",
	"TX Excessive Collision",
	"TX Late Collisions",
	"TX Unicast Byte Count",
	"TX Broadcast Byte Count",
	"TX Multicast Byte Count",
	"TX Unicast Frames",
	"TX Broadcast Frames",
	"TX Multicast Frames",
	"TX Pause Frames",
	"TX 64 Byte Frames",
	"TX 65 - 127 Byte Frames",
	"TX 128 - 255 Byte Frames",
	"TX 256 - 511 Bytes Frames",
	"TX 512 - 1023 Byte Frames",
	"TX 1024 - 1518 Byte Frames",
	"TX Greater 1518 Byte Frames",
	"EEE TX LPI Transitions",
	"EEE TX LPI Time",
};

struct lan78xx_statstage {
	u32 rx_fcs_errors;
	u32 rx_alignment_errors;
	u32 rx_fragment_errors;
	u32 rx_jabber_errors;
	u32 rx_undersize_frame_errors;
	u32 rx_oversize_frame_errors;
	u32 rx_dropped_frames;
	u32 rx_unicast_byte_count;
	u32 rx_broadcast_byte_count;
	u32 rx_multicast_byte_count;
	u32 rx_unicast_frames;
	u32 rx_broadcast_frames;
	u32 rx_multicast_frames;
	u32 rx_pause_frames;
	u32 rx_64_byte_frames;
	u32 rx_65_127_byte_frames;
	u32 rx_128_255_byte_frames;
	u32 rx_256_511_bytes_frames;
	u32 rx_512_1023_byte_frames;
	u32 rx_1024_1518_byte_frames;
	u32 rx_greater_1518_byte_frames;
	u32 eee_rx_lpi_transitions;
	u32 eee_rx_lpi_time;
	u32 tx_fcs_errors;
	u32 tx_excess_deferral_errors;
	u32 tx_carrier_errors;
	u32 tx_bad_byte_count;
	u32 tx_single_collisions;
	u32 tx_multiple_collisions;
	u32 tx_excessive_collision;
	u32 tx_late_collisions;
	u32 tx_unicast_byte_count;
	u32 tx_broadcast_byte_count;
	u32 tx_multicast_byte_count;
	u32 tx_unicast_frames;
	u32 tx_broadcast_frames;
	u32 tx_multicast_frames;
	u32 tx_pause_frames;
	u32 tx_64_byte_frames;
	u32 tx_65_127_byte_frames;
	u32 tx_128_255_byte_frames;
	u32 tx_256_511_bytes_frames;
	u32 tx_512_1023_byte_frames;
	u32 tx_1024_1518_byte_frames;
	u32 tx_greater_1518_byte_frames;
	u32 eee_tx_lpi_transitions;
	u32 eee_tx_lpi_time;
};

struct lan78xx_statstage64 {
	u64 rx_fcs_errors;
	u64 rx_alignment_errors;
	u64 rx_fragment_errors;
	u64 rx_jabber_errors;
	u64 rx_undersize_frame_errors;
	u64 rx_oversize_frame_errors;
	u64 rx_dropped_frames;
	u64 rx_unicast_byte_count;
	u64 rx_broadcast_byte_count;
	u64 rx_multicast_byte_count;
	u64 rx_unicast_frames;
	u64 rx_broadcast_frames;
	u64 rx_multicast_frames;
	u64 rx_pause_frames;
	u64 rx_64_byte_frames;
	u64 rx_65_127_byte_frames;
	u64 rx_128_255_byte_frames;
	u64 rx_256_511_bytes_frames;
	u64 rx_512_1023_byte_frames;
	u64 rx_1024_1518_byte_frames;
	u64 rx_greater_1518_byte_frames;
	u64 eee_rx_lpi_transitions;
	u64 eee_rx_lpi_time;
	u64 tx_fcs_errors;
	u64 tx_excess_deferral_errors;
	u64 tx_carrier_errors;
	u64 tx_bad_byte_count;
	u64 tx_single_collisions;
	u64 tx_multiple_collisions;
	u64 tx_excessive_collision;
	u64 tx_late_collisions;
	u64 tx_unicast_byte_count;
	u64 tx_broadcast_byte_count;
	u64 tx_multicast_byte_count;
	u64 tx_unicast_frames;
	u64 tx_broadcast_frames;
	u64 tx_multicast_frames;
	u64 tx_pause_frames;
	u64 tx_64_byte_frames;
	u64 tx_65_127_byte_frames;
	u64 tx_128_255_byte_frames;
	u64 tx_256_511_bytes_frames;
	u64 tx_512_1023_byte_frames;
	u64 tx_1024_1518_byte_frames;
	u64 tx_greater_1518_byte_frames;
	u64 eee_tx_lpi_transitions;
	u64 eee_tx_lpi_time;
};

static u32 lan78xx_regs[] = {
	ID_REV,
	INT_STS,
	HW_CFG,
	PMT_CTL,
	E2P_CMD,
	E2P_DATA,
	USB_STATUS,
	VLAN_TYPE,
	MAC_CR,
	MAC_RX,
	MAC_TX,
	FLOW,
	ERR_STS,
	MII_ACC,
	MII_DATA,
	EEE_TX_LPI_REQ_DLY,
	EEE_TW_TX_SYS,
	EEE_TX_LPI_REM_DLY,
	WUCSR
};

#define PHY_REG_SIZE (32 * sizeof(u32))

struct lan78xx_net;

struct lan78xx_priv {
	struct lan78xx_net *dev;
	u32 rfe_ctl;
	u32 mchash_table[DP_SEL_VHF_HASH_LEN]; /* multicast hash table */
	u32 pfilter_table[NUM_OF_MAF][2]; /* perfect filter table */
	u32 vlan_table[DP_SEL_VHF_VLAN_LEN];
	struct mutex dataport_mutex; /* for dataport access */
	spinlock_t rfe_ctl_lock; /* for rfe register access */
	struct work_struct set_multicast;
	struct work_struct set_vlan;
	u32 wol;
};

enum skb_state {
	illegal = 0,
	tx_start,
	tx_done,
	rx_start,
	rx_done,
	rx_cleanup,
	unlink_start
};

struct skb_data {		/* skb->cb is one of these */
	struct urb *urb;
	struct lan78xx_net *dev;
	enum skb_state state;
	size_t length;
	int num_of_packet;
};

struct usb_context {
	struct usb_ctrlrequest req;
	struct lan78xx_net *dev;
};

#define EVENT_TX_HALT			0
#define EVENT_RX_HALT			1
#define EVENT_RX_MEMORY			2
#define EVENT_STS_SPLIT			3
#define EVENT_LINK_RESET		4
#define EVENT_RX_PAUSED			5
#define EVENT_DEV_WAKING		6
#define EVENT_DEV_ASLEEP		7
#define EVENT_DEV_OPEN			8
#define EVENT_STAT_UPDATE		9
#define EVENT_DEV_DISCONNECT		10

struct statstage {
	struct mutex			access_lock;	/* for stats access */
	struct lan78xx_statstage	saved;
	struct lan78xx_statstage	rollover_count;
	struct lan78xx_statstage	rollover_max;
	struct lan78xx_statstage64	curr_stat;
};

struct irq_domain_data {
	struct irq_domain	*irqdomain;
	unsigned int		phyirq;
	struct irq_chip		*irqchip;
	irq_flow_handler_t	irq_handler;
	u32			irqenable;
	struct mutex		irq_lock;		/* for irq bus access */
};

struct lan78xx_net {
	struct net_device	*net;
	struct usb_device	*udev;
	struct usb_interface	*intf;
	void			*driver_priv;

	int			rx_qlen;
	int			tx_qlen;
	struct sk_buff_head	rxq;
	struct sk_buff_head	txq;
	struct sk_buff_head	done;
	struct sk_buff_head	txq_pend;

	struct tasklet_struct	bh;
	struct delayed_work	wq;

	int			msg_enable;

	struct urb		*urb_intr;
	struct usb_anchor	deferred;

	struct mutex		dev_mutex; /* serialise open/stop wrt suspend/resume */
	struct mutex		mdiobus_mutex; /* for MDIO bus access */
	struct mutex		ctrl_mutex; /* serialize all EP0 vendor register ops */
	unsigned int		pipe_in, pipe_out, pipe_intr;

	u32			hard_mtu;	/* count any extra framing */
	size_t			rx_urb_size;	/* size for rx urbs */

	unsigned long		flags;

	wait_queue_head_t	*wait;
	unsigned char		suspend_count;

	unsigned int		maxpacket;
	struct timer_list	stat_monitor;

	unsigned long		data[5];

	int			link_on;
	u8			mdix_ctrl;

	u32			chipid;
	u32			chiprev;
	struct mii_bus		*mdiobus;
	phy_interface_t		interface;

	int			fc_autoneg;
	u8			fc_request_control;

	int			delta;
	struct statstage	stats;

	struct irq_domain_data	domain_data;
};

/* define external phy id */
#define	PHY_LAN8835			(0x0007C130)
#define	PHY_KSZ9031RNX			(0x00221620)

/* use ethtool to change the level for any given device */
static int msg_level = -1;
module_param(msg_level, int, 0);
MODULE_PARM_DESC(msg_level, "Override default message level");

/* Maximum number of retries for register operations that timeout */
#define LAN78XX_USB_RETRIES 5
#define LAN78XX_MDIO_TIMEOUT		HZ
#define LAN78XX_MDIO_STALL_MS		USB_CTRL_GET_TIMEOUT

static int lan78xx_read_reg(struct lan78xx_net *dev, u32 index, u32 *data)
{
	u32 *buf;
	int ret;
	int retries;
	int attempts;

	if (test_bit(EVENT_DEV_DISCONNECT, &dev->flags))
		return -ENODEV;

	buf = kmalloc(sizeof(u32), GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	/* Retry on timeout to handle transient USB issues */
	for (retries = 0; retries < LAN78XX_USB_RETRIES; retries++) {
		unsigned long attempt_start = jiffies;
		unsigned int elapsed_ms;

		mutex_lock(&dev->ctrl_mutex);
		ret = usb_control_msg(dev->udev, usb_rcvctrlpipe(dev->udev, 0),
				      USB_VENDOR_REQUEST_READ_REGISTER,
				      USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
				      0, index, buf, 4, USB_CTRL_GET_TIMEOUT_LOCAL);
		mutex_unlock(&dev->ctrl_mutex);
		elapsed_ms = jiffies_to_msecs(jiffies - attempt_start);
		if (likely(ret >= 0)) {
			le32_to_cpus(buf);
			*data = *buf;
			if (retries)
				netdev_dbg(dev->net,
					   "register read 0x%08x recovered after %d retries, value 0x%08x\n",
					   index, retries, *data);
			break;
		} else if (ret == -ENODEV) {
			netdev_dbg(dev->net,
				   "register read 0x%08x stopped because the USB device disappeared\n",
				   index);
			break;
		}
		if (ret == -ETIMEDOUT && net_ratelimit())
			netdev_warn(dev->net,
				    "register read 0x%08x timed out after %u ms on attempt %d/%d\n",
				    index, elapsed_ms, retries + 1,
				    LAN78XX_USB_RETRIES);
		netdev_dbg(dev->net,
			   "register read 0x%08x attempt %d/%d failed after %u ms: %pe\n",
			   index, retries + 1, LAN78XX_USB_RETRIES,
			   elapsed_ms, ERR_PTR(ret));
		/* Brief delay before retry (exponential backoff) */
		if (retries < LAN78XX_USB_RETRIES - 1)
			usleep_range(100 << retries, 200 << retries);
	}

	attempts = min(retries + 1, LAN78XX_USB_RETRIES);
	if (unlikely(ret < 0) && net_ratelimit()) {
		netdev_warn(dev->net,
			    "Failed to read register index 0x%08x after %d attempts. ret = %pe",
			    index, attempts, ERR_PTR(ret));
	}
	kfree(buf);
	return ret < 0 ? ret : 0;
}

static int lan78xx_write_reg(struct lan78xx_net *dev, u32 index, u32 data)
{
	u32 *buf;
	int ret = 0;
	int retries;
	int attempts;

	if (test_bit(EVENT_DEV_DISCONNECT, &dev->flags))
		return -ENODEV;

	buf = kmalloc(sizeof(u32), GFP_KERNEL);
	if (!buf)
		return -ENOMEM;
	*buf = data;
	cpu_to_le32s(buf);

	/* Retry on timeout to handle transient USB issues */
	for (retries = 0; retries < LAN78XX_USB_RETRIES; retries++) {
		unsigned long attempt_start = jiffies;
		unsigned int elapsed_ms;

		mutex_lock(&dev->ctrl_mutex);
		ret = usb_control_msg(dev->udev, usb_sndctrlpipe(dev->udev, 0),
				      USB_VENDOR_REQUEST_WRITE_REGISTER,
				      USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
				      0, index, buf, 4, USB_CTRL_SET_TIMEOUT_LOCAL);
		mutex_unlock(&dev->ctrl_mutex);
		elapsed_ms = jiffies_to_msecs(jiffies - attempt_start);

		if (likely(ret >= 0)) {
			if (retries)
				netdev_dbg(dev->net,
					   "register write 0x%08x=0x%08x recovered after %d retries\n",
					   index, data, retries);
			break;
		} else if (ret == -ENODEV) {
			netdev_dbg(dev->net,
				   "register write 0x%08x stopped because the USB device disappeared\n",
				   index);
			break;
		}
		if (ret == -ETIMEDOUT && net_ratelimit())
			netdev_warn(dev->net,
				    "register write 0x%08x=0x%08x timed out after %u ms on attempt %d/%d\n",
				    index, data, elapsed_ms, retries + 1,
				    LAN78XX_USB_RETRIES);
		netdev_dbg(dev->net,
			   "register write 0x%08x=0x%08x attempt %d/%d failed after %u ms: %pe\n",
			   index, data, retries + 1, LAN78XX_USB_RETRIES,
			   elapsed_ms, ERR_PTR(ret));
		/* Brief delay before retry (exponential backoff) */
		if (retries < LAN78XX_USB_RETRIES - 1)
			usleep_range(100 << retries, 200 << retries);
	}
	attempts = min(retries + 1, LAN78XX_USB_RETRIES);
	if (unlikely(ret < 0) && net_ratelimit()) {
		netdev_warn(dev->net,
			    "Failed to write register index 0x%08x after %d attempts. ret = %pe",
			    index, attempts, ERR_PTR(ret));
	}

	kfree(buf);

	return ret < 0 ? ret : 0;
}

static int lan78xx_update_reg(struct lan78xx_net *dev, u32 reg, u32 mask,
			      u32 data)
{
	int ret = 0;
	u32 buf;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	ret = lan78xx_read_reg(dev, reg, &buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	buf &= ~mask;
	buf |= (mask & data);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return lan78xx_write_reg(dev, reg, buf);
}

static int lan78xx_read_stats(struct lan78xx_net *dev,
			      struct lan78xx_statstage *data)
{
	int ret = 0;
	int i;
	struct lan78xx_statstage *stats;
	u32 *src;
	u32 *dst;

	netdev_dbg(dev->net, "reading hardware statistics block\n");

	stats = kmalloc(sizeof(*stats), GFP_KERNEL);
	if (!stats)
		return -ENOMEM;
	mutex_lock(&dev->ctrl_mutex);
	ret = usb_control_msg(dev->udev,
			      usb_rcvctrlpipe(dev->udev, 0),
			      USB_VENDOR_REQUEST_GET_STATS,
			      USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			      0,
			      0,
			      (void *)stats,
			      sizeof(*stats),
			      USB_CTRL_GET_TIMEOUT_LOCAL);
	mutex_unlock(&dev->ctrl_mutex);
	if (likely(ret >= 0)) {
		src = (u32 *)stats;
		dst = (u32 *)data;
		for (i = 0; i < sizeof(*stats) / sizeof(u32); i++) {
			le32_to_cpus(&src[i]);
			dst[i] = src[i];
		}
	} else {
		netdev_warn(dev->net,
			    "Failed to read stat ret = %d", ret);
	}

	kfree(stats);
	return ret;
}

#define check_counter_rollover(struct1, dev_stats, member)		\
	do {								\
		if ((struct1)->member < (dev_stats).saved.member)	\
			(dev_stats).rollover_count.member++;		\
	} while (0)

static void lan78xx_check_stat_rollover(struct lan78xx_net *dev,
					struct lan78xx_statstage *stats)
{
	pr_debug(" SOFTING  %d ", __LINE__);
	check_counter_rollover(stats, dev->stats, rx_fcs_errors);
	check_counter_rollover(stats, dev->stats, rx_alignment_errors);
	check_counter_rollover(stats, dev->stats, rx_fragment_errors);
	check_counter_rollover(stats, dev->stats, rx_jabber_errors);
	check_counter_rollover(stats, dev->stats, rx_undersize_frame_errors);
	check_counter_rollover(stats, dev->stats, rx_oversize_frame_errors);
	check_counter_rollover(stats, dev->stats, rx_dropped_frames);
	check_counter_rollover(stats, dev->stats, rx_unicast_byte_count);
	check_counter_rollover(stats, dev->stats, rx_broadcast_byte_count);
	check_counter_rollover(stats, dev->stats, rx_multicast_byte_count);
	check_counter_rollover(stats, dev->stats, rx_unicast_frames);
	check_counter_rollover(stats, dev->stats, rx_broadcast_frames);
	check_counter_rollover(stats, dev->stats, rx_multicast_frames);
	check_counter_rollover(stats, dev->stats, rx_pause_frames);
	check_counter_rollover(stats, dev->stats, rx_64_byte_frames);
	check_counter_rollover(stats, dev->stats, rx_65_127_byte_frames);
	check_counter_rollover(stats, dev->stats, rx_128_255_byte_frames);
	check_counter_rollover(stats, dev->stats, rx_256_511_bytes_frames);
	check_counter_rollover(stats, dev->stats, rx_512_1023_byte_frames);
	check_counter_rollover(stats, dev->stats, rx_1024_1518_byte_frames);
	check_counter_rollover(stats, dev->stats, rx_greater_1518_byte_frames);
	check_counter_rollover(stats, dev->stats, eee_rx_lpi_transitions);
	check_counter_rollover(stats, dev->stats, eee_rx_lpi_time);
	check_counter_rollover(stats, dev->stats, tx_fcs_errors);
	check_counter_rollover(stats, dev->stats, tx_excess_deferral_errors);
	check_counter_rollover(stats, dev->stats, tx_carrier_errors);
	check_counter_rollover(stats, dev->stats, tx_bad_byte_count);
	check_counter_rollover(stats, dev->stats, tx_single_collisions);
	check_counter_rollover(stats, dev->stats, tx_multiple_collisions);
	check_counter_rollover(stats, dev->stats, tx_excessive_collision);
	check_counter_rollover(stats, dev->stats, tx_late_collisions);
	check_counter_rollover(stats, dev->stats, tx_unicast_byte_count);
	check_counter_rollover(stats, dev->stats, tx_broadcast_byte_count);
	check_counter_rollover(stats, dev->stats, tx_multicast_byte_count);
	check_counter_rollover(stats, dev->stats, tx_unicast_frames);
	check_counter_rollover(stats, dev->stats, tx_broadcast_frames);
	check_counter_rollover(stats, dev->stats, tx_multicast_frames);
	check_counter_rollover(stats, dev->stats, tx_pause_frames);
	check_counter_rollover(stats, dev->stats, tx_64_byte_frames);
	check_counter_rollover(stats, dev->stats, tx_65_127_byte_frames);
	check_counter_rollover(stats, dev->stats, tx_128_255_byte_frames);
	check_counter_rollover(stats, dev->stats, tx_256_511_bytes_frames);
	check_counter_rollover(stats, dev->stats, tx_512_1023_byte_frames);
	check_counter_rollover(stats, dev->stats, tx_1024_1518_byte_frames);
	check_counter_rollover(stats, dev->stats, tx_greater_1518_byte_frames);
	check_counter_rollover(stats, dev->stats, eee_tx_lpi_transitions);
	check_counter_rollover(stats, dev->stats, eee_tx_lpi_time);

	pr_debug(" SOFTING  %d ", __LINE__);
	memcpy(&dev->stats.saved, stats, sizeof(struct lan78xx_statstage));
}

static void lan78xx_update_stats(struct lan78xx_net *dev)
{
	u32 *p, *count, *max;
	u64 *data;
	int i;
	struct lan78xx_statstage lan78xx_stats;

	pr_debug(" SOFTING  %d ", __LINE__);
	if (usb_autopm_get_interface(dev->intf) < 0)
		return;

	pr_debug(" SOFTING  %d ", __LINE__);
	p = (u32 *)&lan78xx_stats;
	count = (u32 *)&dev->stats.rollover_count;
	max = (u32 *)&dev->stats.rollover_max;
	data = (u64 *)&dev->stats.curr_stat;

	pr_debug(" SOFTING  %d ", __LINE__);
	mutex_lock(&dev->stats.access_lock);

	pr_debug(" SOFTING  %d ", __LINE__);
	if (lan78xx_read_stats(dev, &lan78xx_stats) > 0)
		lan78xx_check_stat_rollover(dev, &lan78xx_stats);

	pr_debug(" SOFTING  %d ", __LINE__);
	for (i = 0; i < (sizeof(lan78xx_stats) / (sizeof(u32))); i++)
		data[i] = (u64)p[i] + ((u64)count[i] * ((u64)max[i] + 1));

	pr_debug(" SOFTING  %d ", __LINE__);
	mutex_unlock(&dev->stats.access_lock);

	usb_autopm_put_interface(dev->intf);
}

/* Loop until the read is completed with timeout called with mdiobus_mutex held */
static int lan78xx_mdiobus_wait_not_busy(struct lan78xx_net *dev)
{
	unsigned long start_time = jiffies;
	u32 val;
	int ret = 0;

	do {
		ret = lan78xx_read_reg(dev, MII_ACC, &val);
		if (unlikely(ret < 0)) {
			netdev_info(dev->net,
				    "MII_ACC busy check failed after %u ms: %pe\n",
				    jiffies_to_msecs(jiffies - start_time),
				    ERR_PTR(ret));
			return -EIO;
		}
		if (!(val & MII_ACC_MII_BUSY_))
			return 0;
	} while (!time_after(jiffies, start_time + LAN78XX_MDIO_TIMEOUT));

	netdev_warn(dev->net,
		    "MII_ACC stayed busy for %u ms, last value 0x%08x\n",
		    jiffies_to_msecs(jiffies - start_time), val);
	return -EIO;
}

static inline u32 mii_access(int id, int index, int read)
{
	u32 ret = 0;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	ret = ((u32)id << MII_ACC_PHY_ADDR_SHIFT_) & MII_ACC_PHY_ADDR_MASK_;
	ret |= ((u32)index << MII_ACC_MIIRINDA_SHIFT_) & MII_ACC_MIIRINDA_MASK_;
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (read)
		ret |= MII_ACC_MII_READ_;
	else
		ret |= MII_ACC_MII_WRITE_;
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	ret |= MII_ACC_MII_BUSY_;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return ret;
}

static int lan78xx_wait_eeprom(struct lan78xx_net *dev)
{
	unsigned long start_time = jiffies;
	u32 val;
	int ret = 0;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	do {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		ret = lan78xx_read_reg(dev, E2P_CMD, &val);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (unlikely(ret < 0))
			return -EIO;

		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (!(val & E2P_CMD_EPC_BUSY_) ||
		    (val & E2P_CMD_EPC_TIMEOUT_))
			break;
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		usleep_range(40, 100);
	} while (!time_after(jiffies, start_time + HZ));

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (val & (E2P_CMD_EPC_TIMEOUT_ | E2P_CMD_EPC_BUSY_)) {
		netdev_warn(dev->net, "EEPROM read operation timeout");
		return -EIO;
	}

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return 0;
}

static int lan78xx_eeprom_confirm_not_busy(struct lan78xx_net *dev)
{
	unsigned long start_time = jiffies;
	u32 val;
	int ret = 0;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	do {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		ret = lan78xx_read_reg(dev, E2P_CMD, &val);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (unlikely(ret < 0))
			return -EIO;
		pr_debug(" SOFTING  %d %d", __LINE__, ret);

		if (!(val & E2P_CMD_EPC_BUSY_))
			return 0;

		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		usleep_range(40, 100);
	} while (!time_after(jiffies, start_time + HZ));

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	netdev_warn(dev->net, "EEPROM is busy");
	return -EIO;
}

static int lan78xx_read_raw_eeprom(struct lan78xx_net *dev, u32 offset,
				   u32 length, u8 *data)
{
	u32 val;
	u32 saved;
	int i, ret=0;
	int retval;

	/* depends on chip, some EEPROM pins are muxed with LED function.
	 * disable & restore LED function to access EEPROM.
	 */
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	ret = lan78xx_read_reg(dev, HW_CFG, &val);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	saved = val;
	if (dev->chipid == ID_REV_CHIP_ID_7800_) {
		val &= ~(HW_CFG_LED1_EN_ | HW_CFG_LED0_EN_);
		ret = lan78xx_write_reg(dev, HW_CFG, val);
	}

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	retval = lan78xx_eeprom_confirm_not_busy(dev);
	pr_debug(" SOFTING  %d %d", __LINE__, retval);
	if (retval)
		return retval;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	for (i = 0; i < length; i++) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		val = E2P_CMD_EPC_BUSY_ | E2P_CMD_EPC_CMD_READ_;
		val |= (offset & E2P_CMD_EPC_ADDR_MASK_);
		ret = lan78xx_write_reg(dev, E2P_CMD, val);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (unlikely(ret < 0)) {
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			retval = -EIO;
			goto exit;
		}

		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		retval = lan78xx_wait_eeprom(dev);
		pr_debug(" SOFTING  %d %d", __LINE__, retval);
		if (retval < 0)
			goto exit;

		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		ret = lan78xx_read_reg(dev, E2P_DATA, &val);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (unlikely(ret < 0)) {
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			retval = -EIO;
			goto exit;
		}

		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		data[i] = val & 0xFF;
		offset++;
	}

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	retval = 0;
exit:
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (dev->chipid == ID_REV_CHIP_ID_7800_)
		ret = lan78xx_write_reg(dev, HW_CFG, saved);

	pr_debug(" SOFTING  %d %d", __LINE__, retval);
	return retval;
}

static int lan78xx_read_eeprom(struct lan78xx_net *dev, u32 offset,
			       u32 length, u8 *data)
{
	u8 sig;
	int ret=0;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	ret = lan78xx_read_raw_eeprom(dev, 0, 1, &sig);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if ((ret == 0) && (sig == EEPROM_INDICATOR))
		ret = lan78xx_read_raw_eeprom(dev, offset, length, data);
	else
		ret = -EINVAL;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return ret;
}

static int lan78xx_write_raw_eeprom(struct lan78xx_net *dev, u32 offset,
				    u32 length, u8 *data)
{
	u32 val;
	u32 saved;
	int i, ret=0;
	int retval;

	/* depends on chip, some EEPROM pins are muxed with LED function.
	 * disable & restore LED function to access EEPROM.
	 */
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	ret = lan78xx_read_reg(dev, HW_CFG, &val);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	saved = val;
	if (dev->chipid == ID_REV_CHIP_ID_7800_) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		val &= ~(HW_CFG_LED1_EN_ | HW_CFG_LED0_EN_);
		ret = lan78xx_write_reg(dev, HW_CFG, val);
	}

	retval = lan78xx_eeprom_confirm_not_busy(dev);
	pr_debug(" SOFTING  %d %d", __LINE__, retval);
	if (retval)
		goto exit;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	/* Issue write/erase enable command */
	val = E2P_CMD_EPC_BUSY_ | E2P_CMD_EPC_CMD_EWEN_;
	ret = lan78xx_write_reg(dev, E2P_CMD, val);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (unlikely(ret < 0)) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		retval = -EIO;
		goto exit;
	}

	retval = lan78xx_wait_eeprom(dev);
	pr_debug(" SOFTING  %d %d", __LINE__, retval);
	if (retval < 0)
		goto exit;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	for (i = 0; i < length; i++) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		/* Fill data register */
		val = data[i];
		ret = lan78xx_write_reg(dev, E2P_DATA, val);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0) {
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			retval = -EIO;
			goto exit;
		}

		/* Send "write" command */
		val = E2P_CMD_EPC_BUSY_ | E2P_CMD_EPC_CMD_WRITE_;
		val |= (offset & E2P_CMD_EPC_ADDR_MASK_);
		ret = lan78xx_write_reg(dev, E2P_CMD, val);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0) {
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			retval = -EIO;
			goto exit;
		}

		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		retval = lan78xx_wait_eeprom(dev);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (retval < 0)
			goto exit;

		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		offset++;
	}

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	retval = 0;
exit:
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (dev->chipid == ID_REV_CHIP_ID_7800_)
		ret = lan78xx_write_reg(dev, HW_CFG, saved);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return retval;
}

static int lan78xx_read_raw_otp(struct lan78xx_net *dev, u32 offset,
				u32 length, u8 *data)
{
	int i;
	u32 buf;
	unsigned long timeout;

	pr_debug(" SOFTING  %d ", __LINE__);
	lan78xx_read_reg(dev, OTP_PWR_DN, &buf);

	if (buf & OTP_PWR_DN_PWRDN_N_) {
		pr_debug(" SOFTING  %d ", __LINE__);
		/* clear it and wait to be cleared */
		lan78xx_write_reg(dev, OTP_PWR_DN, 0);

		pr_debug(" SOFTING  %d ", __LINE__);
		timeout = jiffies + HZ;
		do {
			usleep_range(1, 10);
			pr_debug(" SOFTING  %d ", __LINE__);
			lan78xx_read_reg(dev, OTP_PWR_DN, &buf);
			pr_debug(" SOFTING  %d ", __LINE__);
			if (time_after(jiffies, timeout)) {
				pr_debug(" SOFTING  %d ", __LINE__);
				netdev_warn(dev->net,
					    "timeout on OTP_PWR_DN");
				return -EIO;
			}
		} while (buf & OTP_PWR_DN_PWRDN_N_);
	}

	for (i = 0; i < length; i++) {
		pr_debug(" SOFTING  %d ", __LINE__);
		lan78xx_write_reg(dev, OTP_ADDR1,
				  ((offset + i) >> 8) & OTP_ADDR1_15_11);
		lan78xx_write_reg(dev, OTP_ADDR2,
				  ((offset + i) & OTP_ADDR2_10_3));

		lan78xx_write_reg(dev, OTP_FUNC_CMD, OTP_FUNC_CMD_READ_);
		lan78xx_write_reg(dev, OTP_CMD_GO, OTP_CMD_GO_GO_);

		timeout = jiffies + HZ;
		do {
			pr_debug(" SOFTING  %d ", __LINE__);
			udelay(1);
			lan78xx_read_reg(dev, OTP_STATUS, &buf);
			pr_debug(" SOFTING  %d ", __LINE__);
			if (time_after(jiffies, timeout)) {
				netdev_warn(dev->net,
					    "timeout on OTP_STATUS");
				return -EIO;
			}
		} while (buf & OTP_STATUS_BUSY_);

		lan78xx_read_reg(dev, OTP_RD_DATA, &buf);

		data[i] = (u8)(buf & 0xFF);
	}

	pr_debug(" SOFTING  %d ", __LINE__);
	return 0;
}

static int lan78xx_write_raw_otp(struct lan78xx_net *dev, u32 offset,
				 u32 length, u8 *data)
{
	int i;
	u32 buf;
	unsigned long timeout;

	pr_debug(" SOFTING  %d ", __LINE__);
	lan78xx_read_reg(dev, OTP_PWR_DN, &buf);

	pr_debug(" SOFTING  %d ", __LINE__);
	if (buf & OTP_PWR_DN_PWRDN_N_) {
		/* clear it and wait to be cleared */
		lan78xx_write_reg(dev, OTP_PWR_DN, 0);

		pr_debug(" SOFTING  %d ", __LINE__);
		timeout = jiffies + HZ;
		do {
			pr_debug(" SOFTING  %d ", __LINE__);
			udelay(1);
			lan78xx_read_reg(dev, OTP_PWR_DN, &buf);
			if (time_after(jiffies, timeout)) {
				pr_debug(" SOFTING  %d ", __LINE__);
				netdev_warn(dev->net,
					    "timeout on OTP_PWR_DN completion");
				return -EIO;
			}
		} while (buf & OTP_PWR_DN_PWRDN_N_);
	}

	pr_debug(" SOFTING  %d ", __LINE__);
	/* set to BYTE program mode */
	lan78xx_write_reg(dev, OTP_PRGM_MODE, OTP_PRGM_MODE_BYTE_);

	pr_debug(" SOFTING  %d ", __LINE__);
	for (i = 0; i < length; i++) {
		pr_debug(" SOFTING  %d ", __LINE__);
		lan78xx_write_reg(dev, OTP_ADDR1,
				  ((offset + i) >> 8) & OTP_ADDR1_15_11);
		lan78xx_write_reg(dev, OTP_ADDR2,
				  ((offset + i) & OTP_ADDR2_10_3));
		lan78xx_write_reg(dev, OTP_PRGM_DATA, data[i]);
		lan78xx_write_reg(dev, OTP_TST_CMD, OTP_TST_CMD_PRGVRFY_);
		lan78xx_write_reg(dev, OTP_CMD_GO, OTP_CMD_GO_GO_);

		timeout = jiffies + HZ;
		do {
			pr_debug(" SOFTING  %d ", __LINE__);
			udelay(1);
			lan78xx_read_reg(dev, OTP_STATUS, &buf);
			if (time_after(jiffies, timeout)) {
				pr_debug(" SOFTING  %d ", __LINE__);
				netdev_warn(dev->net,
					    "Timeout on OTP_STATUS completion");
				return -EIO;
			}
		} while (buf & OTP_STATUS_BUSY_);
	}

	pr_debug(" SOFTING  %d ", __LINE__);
	return 0;
}

static int lan78xx_read_otp(struct lan78xx_net *dev, u32 offset,
			    u32 length, u8 *data)
{
	u8 sig;
	int ret=0;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	ret = lan78xx_read_raw_otp(dev, 0, 1, &sig);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret == 0) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (sig == OTP_INDICATOR_2)
			offset += 0x100;
		else if (sig != OTP_INDICATOR_1)
			ret = -EINVAL;
		if (!ret)
			ret = lan78xx_read_raw_otp(dev, offset, length, data);
	}

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return ret;
}

static int lan78xx_dataport_wait_not_busy(struct lan78xx_net *dev)
{
	unsigned long start_time = jiffies;
	u32 last_dp_sel = 0;
	int i, ret = 0;

	for (i = 0; i < 100; i++) {
		u32 dp_sel;
		ret = lan78xx_read_reg(dev, DP_SEL, &dp_sel);
		if (unlikely(ret < 0)) {
			netdev_warn(dev->net,
				    "dataport RAM readiness poll failed after %d poll(s), %u ms: %pe\n",
				    i + 1,
				    jiffies_to_msecs(jiffies - start_time),
				    ERR_PTR(ret));
			return -EIO;
		}

		last_dp_sel = dp_sel;

		if (dp_sel & DP_SEL_DPRDY_) {
			if (i)
				netdev_dbg(dev->net,
					   "dataport RAM window became ready after %d poll(s), %u ms, DP_SEL 0x%08x\n",
					   i + 1,
					   jiffies_to_msecs(jiffies - start_time),
					   dp_sel);
			return 0;
		}
		usleep_range(40, 100);
	}
	netdev_warn(dev->net,
		    "%s timed out after %d poll(s), %u ms, last DP_SEL 0x%08x\n",
		    __func__, i, jiffies_to_msecs(jiffies - start_time),
		    last_dp_sel);
	return -EIO;
}

static int lan78xx_dataport_write(struct lan78xx_net *dev, u32 ram_select,
				  u32 addr, u32 length, u32 *buf)
{
	struct lan78xx_priv *pdata = (struct lan78xx_priv *)(dev->data[0]);
	u32 dp_sel;
	int i, ret = 0;

	netdev_dbg(dev->net,
		   "writing dataport RAM select 0x%08x, start 0x%08x, %u word(s)\n",
		   ram_select, addr, length);

	if (usb_autopm_get_interface(dev->intf) < 0) {
		netdev_dbg(dev->net,
			   "skipping dataport write because USB runtime PM did not resume the interface\n");
		return 0;
	}

	mutex_lock(&pdata->dataport_mutex);
	ret = lan78xx_dataport_wait_not_busy(dev);
	if (ret < 0)
		goto done;
	ret = lan78xx_read_reg(dev, DP_SEL, &dp_sel);
	if (ret < 0) {
		netdev_warn(dev->net,
			    "failed to read DP_SEL before dataport write: %pe\n",
			    ERR_PTR(ret));
		goto done;
	}
	dp_sel &= ~DP_SEL_RSEL_MASK_;
	dp_sel |= ram_select;
	ret = lan78xx_write_reg(dev, DP_SEL, dp_sel);
	if (ret < 0) {
		netdev_warn(dev->net,
			    "failed to select dataport RAM 0x%08x: %pe\n",
			    ram_select, ERR_PTR(ret));
		goto done;
	}
	for (i = 0; i < length; i++) {
		ret = lan78xx_write_reg(dev, DP_ADDR, addr + i);
		if (ret < 0) {
			netdev_warn(dev->net,
				    "failed to write dataport address 0x%08x at word %d/%u: %pe\n",
				    addr + i, i + 1, length, ERR_PTR(ret));
			goto done;
		}

		ret = lan78xx_write_reg(dev, DP_DATA, buf[i]);
		if (ret < 0) {
			netdev_warn(dev->net,
				    "failed to write dataport data at word %d/%u: %pe\n",
				    i + 1, length, ERR_PTR(ret));
			goto done;
		}

		ret = lan78xx_write_reg(dev, DP_CMD, DP_CMD_WRITE_);
		if (ret < 0) {
			netdev_warn(dev->net,
				    "failed to issue dataport write command at word %d/%u: %pe\n",
				    i + 1, length, ERR_PTR(ret));
			goto done;
		}

		ret = lan78xx_dataport_wait_not_busy(dev);
		if (ret < 0)
			goto done;
	}
done:
	mutex_unlock(&pdata->dataport_mutex);
	usb_autopm_put_interface(dev->intf);
	return ret;
}

static void lan78xx_set_addr_filter(struct lan78xx_priv *pdata,
				    int index, u8 addr[ETH_ALEN])
{
	u32 temp;

	pr_debug(" SOFTING  %d", __LINE__);
	if ((pdata) && (index > 0) && (index < NUM_OF_MAF)) {
		pr_debug(" SOFTING  %d", __LINE__);
		temp = addr[3];
		temp = addr[2] | (temp << 8);
		temp = addr[1] | (temp << 8);
		temp = addr[0] | (temp << 8);
		pdata->pfilter_table[index][1] = temp;
		temp = addr[5];
		temp = addr[4] | (temp << 8);
		temp |= MAF_HI_VALID_ | MAF_HI_TYPE_DST_;
		pdata->pfilter_table[index][0] = temp;
	}
	pr_debug(" SOFTING  %d ", __LINE__);
}

/* returns hash bit number for given MAC address */
static inline u32 lan78xx_hash(char addr[ETH_ALEN])
{
	pr_debug(" SOFTING  %d ", __LINE__);
	return (ether_crc(ETH_ALEN, addr) >> 23) & 0x1ff;
}

static void lan78xx_deferred_multicast_write(struct work_struct *param)
{
	struct lan78xx_priv *pdata =
			container_of(param, struct lan78xx_priv, set_multicast);
	struct lan78xx_net *dev = pdata->dev;
	int i;

	pr_debug(" SOFTING  %d ", __LINE__);
	netif_dbg(dev, drv, dev->net, "deferred multicast write 0x%08x\n",
		  pdata->rfe_ctl);

	pr_debug(" SOFTING  %d ", __LINE__);
	lan78xx_dataport_write(dev, DP_SEL_RSEL_VLAN_DA_, DP_SEL_VHF_VLAN_LEN,
			       DP_SEL_VHF_HASH_LEN, pdata->mchash_table);

	pr_debug(" SOFTING  %d ", __LINE__);
	for (i = 1; i < NUM_OF_MAF; i++) {
		pr_debug(" SOFTING  %d ", __LINE__);
		lan78xx_write_reg(dev, MAF_HI(i), 0);
		lan78xx_write_reg(dev, MAF_LO(i),
				  pdata->pfilter_table[i][1]);
		lan78xx_write_reg(dev, MAF_HI(i),
				  pdata->pfilter_table[i][0]);
	}

	pr_debug(" SOFTING  %d ", __LINE__);
	lan78xx_write_reg(dev, RFE_CTL, pdata->rfe_ctl);
}

static void lan78xx_set_multicast(struct net_device *netdev)
{
	struct lan78xx_net *dev = netdev_priv(netdev);
	struct lan78xx_priv *pdata = (struct lan78xx_priv *)(dev->data[0]);
	unsigned long flags;
	int i;

	pr_debug(" SOFTING  %d ", __LINE__);
	spin_lock_irqsave(&pdata->rfe_ctl_lock, flags);

	pdata->rfe_ctl &= ~(RFE_CTL_UCAST_EN_ | RFE_CTL_MCAST_EN_ |
			    RFE_CTL_DA_PERFECT_ | RFE_CTL_MCAST_HASH_);

	pr_debug(" SOFTING  %d ", __LINE__);
	for (i = 0; i < DP_SEL_VHF_HASH_LEN; i++)
		pdata->mchash_table[i] = 0;

	pr_debug(" SOFTING  %d ", __LINE__);
	/* pfilter_table[0] has own HW address */
	for (i = 1; i < NUM_OF_MAF; i++) {
		pr_debug(" SOFTING  %d ", __LINE__);
		pdata->pfilter_table[i][0] = 0;
		pdata->pfilter_table[i][1] = 0;
	}

	pr_debug(" SOFTING  %d ", __LINE__);
	pdata->rfe_ctl |= RFE_CTL_BCAST_EN_;

	if (dev->net->flags & IFF_PROMISC) {
		pr_debug(" SOFTING  %d ", __LINE__);
		netif_dbg(dev, drv, dev->net, "promiscuous mode enabled");
		pdata->rfe_ctl |= RFE_CTL_MCAST_EN_ | RFE_CTL_UCAST_EN_;
	} else {
		pr_debug(" SOFTING  %d ", __LINE__);
		if (dev->net->flags & IFF_ALLMULTI) {
			pr_debug(" SOFTING  %d ", __LINE__);
			netif_dbg(dev, drv, dev->net,
				  "receive all multicast enabled");
			pdata->rfe_ctl |= RFE_CTL_MCAST_EN_;
		}
	}

	pr_debug(" SOFTING  %d ", __LINE__);
	if (netdev_mc_count(dev->net)) {
		struct netdev_hw_addr *ha;
		int i;

		pr_debug(" SOFTING  %d ", __LINE__);
		netif_dbg(dev, drv, dev->net, "receive multicast hash filter");

		pdata->rfe_ctl |= RFE_CTL_DA_PERFECT_;

		i = 1;
		netdev_for_each_mc_addr(ha, netdev) {
			/* set first 32 into Perfect Filter */
			if (i < 33) {
				lan78xx_set_addr_filter(pdata, i, ha->addr);
			} else {
				u32 bitnum = lan78xx_hash(ha->addr);

				pdata->mchash_table[bitnum / 32] |=
							(1 << (bitnum % 32));
				pdata->rfe_ctl |= RFE_CTL_MCAST_HASH_;
			}
			i++;
		}
		pr_debug(" SOFTING  %d ", __LINE__);
	}

	pr_debug(" SOFTING  %d ", __LINE__);
	spin_unlock_irqrestore(&pdata->rfe_ctl_lock, flags);

	pr_debug(" SOFTING  %d ", __LINE__);
	/* defer register writes to a sleepable context */
	schedule_work(&pdata->set_multicast);
}

/**
 * lan78xx_configure_flowcontrol - Set MAC and FIFO flow control configuration
 * @dev: pointer to the LAN78xx device structure
 * @tx_pause: enable transmission of pause frames
 * @rx_pause: enable reception of pause frames
 *
 * This function configures the LAN78xx flow control settings by writing
 * to the FLOW and FCT_FLOW registers. The pause time is set to the
 * maximum allowed value (65535 quanta). FIFO thresholds are selected
 * based on USB speed.
 *
 * Return: 0 on success or a negative error code on failure.
 */
static int lan78xx_configure_flowcontrol(struct lan78xx_net *dev,
					 bool tx_pause, bool rx_pause)
{
	/* Use maximum pause time: 65535 quanta (512-bit times) */
	const u32 pause_time_quanta = 65535;
	u32 fct_flow = 0;
	u32 flow = 0;
	int ret;

	/* Prepare MAC flow control bits */
	if (tx_pause)
		flow |= FLOW_CR_TX_FCEN_ | pause_time_quanta;

	if (rx_pause)
		flow |= FLOW_CR_RX_FCEN_;

	/* Select RX FIFO thresholds based on USB speed */
	switch (dev->udev->speed) {
	case USB_SPEED_SUPER:
		fct_flow = FLOW_CTRL_THRESHOLD(FLOW_ON_SS, FLOW_OFF_SS);
		break;
	case USB_SPEED_HIGH:
		fct_flow = FLOW_CTRL_THRESHOLD(FLOW_ON_HS, FLOW_OFF_HS);
		break;
	default:
		netdev_warn(dev->net, "Unsupported USB speed: %d\n",
			    dev->udev->speed);
		return -EINVAL;
	}

	/* Step 1: Write FIFO thresholds before enabling pause frames */
	ret = lan78xx_write_reg(dev, FCT_FLOW, fct_flow);
	if (ret < 0)
		return ret;

	/* Step 2: Enable MAC pause functionality */
	return lan78xx_write_reg(dev, FLOW, flow);
}

static int lan78xx_update_flowcontrol(struct lan78xx_net *dev, u8 duplex,
				      u16 lcladv, u16 rmtadv)
{
	u8 cap;

	if (dev->fc_autoneg)
		cap = mii_resolve_flowctrl_fdx(lcladv, rmtadv);
	else
		cap = dev->fc_request_control;

	netif_dbg(dev, link, dev->net, "rx pause %s, tx pause %s",
		  (cap & FLOW_CTRL_RX ? "enabled" : "disabled"),
		  (cap & FLOW_CTRL_TX ? "enabled" : "disabled"));

	return lan78xx_configure_flowcontrol(dev,
					     cap & FLOW_CTRL_TX,
					     cap & FLOW_CTRL_RX);
}

static int lan78xx_mac_reset(struct lan78xx_net *dev)
{
	unsigned long start_time = jiffies;
	u32 val;
	int ret=0;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	mutex_lock(&dev->mdiobus_mutex);

	/* Resetting the device while there is activity on the MDIO
	 * bus can result in the MAC interface locking up and not
	 * completing register access transactions.
	 */
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	ret = lan78xx_mdiobus_wait_not_busy(dev);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		goto done;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	ret = lan78xx_read_reg(dev, MAC_CR, &val);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		goto done;

	val |= MAC_CR_RST_;
	ret = lan78xx_write_reg(dev, MAC_CR, val);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		goto done;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	/* Wait for the reset to complete before allowing any further
	 * MAC register accesses otherwise the MAC may lock up.
	 */
	do {
		ret = lan78xx_read_reg(dev, MAC_CR, &val);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			goto done;

		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (!(val & MAC_CR_RST_)) {
			ret = 0;
			goto done;
		}
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
	} while (!time_after(jiffies, start_time + HZ));

	ret = -ETIMEDOUT;
done:
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	mutex_unlock(&dev->mdiobus_mutex);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return ret;
}

/**
 * lan78xx_phy_int_ack - Acknowledge PHY interrupt
 * @dev: pointer to the LAN78xx device structure
 *
 * This function acknowledges the PHY interrupt by setting the
 * INT_STS_PHY_INT_ bit in the interrupt status register (INT_STS).
 *
 * Return: 0 on success or a negative error code on failure.
 */
static int lan78xx_phy_int_ack(struct lan78xx_net *dev)
{
	return lan78xx_write_reg(dev, INT_STS, INT_STS_PHY_INT_);
}

/**
 * lan78xx_configure_usb - Configure USB link power settings
 * @dev: pointer to the LAN78xx device structure
 * @speed: negotiated Ethernet link speed (in Mbps)
 *
 * This function configures U1/U2 link power management for SuperSpeed
 * USB devices based on the current Ethernet link speed. It uses the
 * USB_CFG1 register to enable or disable U1 and U2 low-power states.
 *
 * Note: Only LAN7800 and LAN7801 support SuperSpeed (USB 3.x).
 *       LAN7850 is a High-Speed-only (USB 2.0) device and is skipped.
 *
 * Return: 0 on success or a negative error code on failure.
 */
static int lan78xx_configure_usb(struct lan78xx_net *dev, int speed)
{
	u32 mask, val;
	int ret;

	/* Only configure USB settings for SuperSpeed devices */
	if (dev->udev->speed != USB_SPEED_SUPER)
		return 0;

	/* LAN7850 does not support USB 3.x */
	if (dev->chipid == ID_REV_CHIP_ID_7850_) {
		netdev_warn_once(dev->net, "Unexpected SuperSpeed for LAN7850 (USB 2.0 only)\n");
		return 0;
	}

	switch (speed) {
	case SPEED_1000:
		/* Disable U2, enable U1 */
		ret = lan78xx_update_reg(dev, USB_CFG1,
					 USB_CFG1_DEV_U2_INIT_EN_, 0);
		if (ret < 0)
			return ret;

		return lan78xx_update_reg(dev, USB_CFG1,
					  USB_CFG1_DEV_U1_INIT_EN_,
					  USB_CFG1_DEV_U1_INIT_EN_);

	case SPEED_100:
	case SPEED_10:
		/* Enable both U1 and U2 */
		mask = USB_CFG1_DEV_U1_INIT_EN_ | USB_CFG1_DEV_U2_INIT_EN_;
		val = mask;
		return lan78xx_update_reg(dev, USB_CFG1, mask, val);

	default:
		netdev_warn(dev->net, "Unsupported link speed: %d\n", speed);
		return -EINVAL;
	}
}

static int lan78xx_link_reset(struct lan78xx_net *dev)
{
	struct phy_device *phydev = dev->net->phydev;
	struct ethtool_link_ksettings ecmd;
	int ladv, radv, ret = 0, link;

	pr_debug(" SOFTING LINK RESET %d %d", __LINE__, ret);
	
	/* If PHY hasn't been started yet (interface not opened), start it now
	 * to enable link detection during boot-time initialization.
	 * This is safe here because we're in work queue context.
	 */
	if (phydev) {
		switch (phydev->state) {
		case PHY_DOWN:
		case PHY_READY:
		case PHY_HALTED:
			if (net_ratelimit()) {
				netdev_info(dev->net,
					    "Link reset: starting PHY for boot-time link detection (state: %d)\n",
					    phydev->state);
			}
			phy_start(phydev);
			/* Give PHY a moment to initialize before reading status */
			msleep(100);
			break;
		default:
			if (net_ratelimit()) {
				netdev_dbg(dev->net,
					   "Link reset: PHY already active (state: %d)\n",
					   phydev->state);
			}
			break;
		}
	}
	
	/* clear LAN78xx interrupt status - continue even if this fails
	 * to avoid getting stuck in an interrupt storm
	 */
	ret = lan78xx_phy_int_ack(dev);
	if (unlikely(ret < 0)) {
		netdev_warn(dev->net,
			    "Failed to ack PHY interrupt: %pe - continuing anyway\n",
			    ERR_PTR(ret));
		/* Don't return here - continue with link status check */
	}

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	
	/* Safety check: ensure PHY is valid before accessing */
	if (!phydev) {
		netdev_warn(dev->net, "Link reset: PHY not available\n");
		return -ENODEV;
	}
	
	mutex_lock(&phydev->lock);
	phy_read_status(phydev);
	link = phydev->link;
	mutex_unlock(&phydev->lock);

	/* If PHY status read failed due to timeouts, be conservative:
	 * - If we think we have a link but PHY says no link, trust the PHY
	 * - If we think we have no link but PHY says link, trust the PHY
	 * - This allows proper recovery when cable is connected
	 */
	if (unlikely(!phydev->link && dev->link_on)) {
		netdev_dbg(dev->net, "PHY reports no link, forcing carrier off\n");
		link = 0;
	} else if (unlikely(phydev->link && !dev->link_on)) {
		netdev_dbg(dev->net, "PHY reports link up, enabling carrier\n");
		link = 1;
	}

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (!link && dev->link_on) {
		dev->link_on = false;
		netif_carrier_off(dev->net);

		/* reset MAC */
		ret = lan78xx_mac_reset(dev);
		if (ret < 0)
			return ret;

		del_timer(&dev->stat_monitor);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		
		/* Schedule periodic link check to recover from NO-CARRIER state */
		mod_timer(&dev->stat_monitor, jiffies + STAT_UPDATE_TIMER);
	} else if (link && !dev->link_on) {
		dev->link_on = true;
		netif_carrier_on(dev->net);

		phy_ethtool_ksettings_get(phydev, &ecmd);

		ret = lan78xx_configure_usb(dev, ecmd.base.speed);
		if (ret < 0)
			return ret;

		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		ladv = phy_read(phydev, MII_ADVERTISE);
		pr_debug(" SOFTING  %d %d", __LINE__, ladv);
		if (ladv < 0)
			return ladv;

		radv = phy_read(phydev, MII_LPA);
		pr_debug(" SOFTING  %d %d", __LINE__, radv);
		if (radv < 0)
			return radv;

		netif_dbg(dev, link, dev->net,
			  "speed: %u duplex: %d anadv: 0x%04x anlpa: 0x%04x",
			  ecmd.base.speed, ecmd.base.duplex, ladv, radv);

		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		ret = lan78xx_update_flowcontrol(dev, ecmd.base.duplex, ladv,
						 radv);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			return ret;

		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (!timer_pending(&dev->stat_monitor)) {
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			dev->delta = 1;
			mod_timer(&dev->stat_monitor,
				  jiffies + STAT_UPDATE_TIMER);
		}

		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		tasklet_schedule(&dev->bh);
	}

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return 0;
}

/* some work can't be done in tasklets, so we use keventd
 *
 * NOTE:  annoying asymmetry:  if it's active, schedule_work() fails,
 * but tasklet_schedule() doesn't.	hope the failure is rare.
 */
static void lan78xx_defer_kevent(struct lan78xx_net *dev, int work)
{
	pr_debug(" SOFTING  %d ", __LINE__);
	set_bit(work, &dev->flags);
	
	/* For critical events like LINK_RESET, try to wake up the USB interface
	 * to ensure the event is not silently dropped due to autosuspend
	 */
	if (work == EVENT_LINK_RESET) {
		usb_autopm_get_interface_async(dev->intf);
	}
	
	if (!schedule_delayed_work(&dev->wq, 0)) {
		/* Work queue is busy, but the flag is set so the event will be
		 * processed when the work queue runs. This is normal for frequently
		 * scheduled events like EVENT_LINK_RESET and EVENT_STAT_UPDATE.
		 * The stat monitor timer (runs every 1s) ensures the work queue
		 * will process all flags regularly.
		 */
		if (work != EVENT_LINK_RESET && work != EVENT_STAT_UPDATE) {
			/* Only log for less frequent events that might indicate a real issue */
			if (net_ratelimit()) {
				netdev_warn(dev->net, "kevent %d work queue busy (flag set, will be processed)\n", work);
			}
		} else if (net_ratelimit()) {
			/* Debug level for frequently scheduled events */
			netdev_dbg(dev->net, "kevent %d work queue busy (flag set, will be processed)\n", work);
		}
	}
}

static void lan78xx_status(struct lan78xx_net *dev, struct urb *urb)
{
	u32 intdata;

	pr_debug(" SOFTING  %d ", __LINE__);
	if (urb->actual_length != 4) {
		pr_debug(" SOFTING  %d ", __LINE__);
		netdev_warn(dev->net,
			    "unexpected urb length %d", urb->actual_length);
		return;
	}

	pr_debug(" SOFTING  %d ", __LINE__);
	intdata = get_unaligned_le32(urb->transfer_buffer);

	pr_debug(" SOFTING  %d ", __LINE__);
	if (intdata & INT_ENP_PHY_INT) {
		pr_debug(" SOFTING  %d ", __LINE__);
		netif_dbg(dev, link, dev->net, "PHY INTR: 0x%08x\n", intdata);
		lan78xx_defer_kevent(dev, EVENT_LINK_RESET);

		pr_debug(" SOFTING  %d ", __LINE__);
		if (dev->domain_data.phyirq > 0) {
			pr_debug(" SOFTING  %d ", __LINE__);
			local_irq_disable();
			generic_handle_irq(dev->domain_data.phyirq);
			local_irq_enable();
		}
	} else {
		pr_debug(" SOFTING  %d ", __LINE__);
		netdev_warn(dev->net,
			    "unexpected interrupt: 0x%08x\n", intdata);
	}
}

static int lan78xx_ethtool_get_eeprom_len(struct net_device *netdev)
{
	pr_debug(" SOFTING  %d ", __LINE__);
	return MAX_EEPROM_SIZE;
}

static int lan78xx_ethtool_get_eeprom(struct net_device *netdev,
				      struct ethtool_eeprom *ee, u8 *data)
{
	struct lan78xx_net *dev = netdev_priv(netdev);
	int ret;

	ret = usb_autopm_get_interface(dev->intf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret)
		return ret;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	ee->magic = LAN78XX_EEPROM_MAGIC;

	ret = lan78xx_read_raw_eeprom(dev, ee->offset, ee->len, data);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);

	usb_autopm_put_interface(dev->intf);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return ret;
}

static int lan78xx_ethtool_set_eeprom(struct net_device *netdev,
				      struct ethtool_eeprom *ee, u8 *data)
{
	struct lan78xx_net *dev = netdev_priv(netdev);
	int ret;

	ret = usb_autopm_get_interface(dev->intf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret)
		return ret;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	/* Invalid EEPROM_INDICATOR at offset zero will result in a failure
	 * to load data from EEPROM
	 */
	if (ee->magic == LAN78XX_EEPROM_MAGIC)
		ret = lan78xx_write_raw_eeprom(dev, ee->offset, ee->len, data);
	else if ((ee->magic == LAN78XX_OTP_MAGIC) &&
		 (ee->offset == 0) &&
		 (ee->len == 512) &&
		 (data[0] == OTP_INDICATOR_1))
		ret = lan78xx_write_raw_otp(dev, ee->offset, ee->len, data);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	usb_autopm_put_interface(dev->intf);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return ret;
}

static void lan78xx_get_strings(struct net_device *netdev, u32 stringset,
				u8 *data)
{
	pr_debug(" SOFTING  %d ", __LINE__);
	if (stringset == ETH_SS_STATS)
		memcpy(data, lan78xx_gstrings, sizeof(lan78xx_gstrings));
}

static int lan78xx_get_sset_count(struct net_device *netdev, int sset)
{
	pr_debug(" SOFTING  %d ", __LINE__);
	if (sset == ETH_SS_STATS)
		return ARRAY_SIZE(lan78xx_gstrings);
	else
		return -EOPNOTSUPP;
}

static void lan78xx_get_stats(struct net_device *netdev,
			      struct ethtool_stats *stats, u64 *data)
{
	struct lan78xx_net *dev = netdev_priv(netdev);

	pr_debug(" SOFTING  %d ", __LINE__);
	lan78xx_update_stats(dev);

	mutex_lock(&dev->stats.access_lock);
	memcpy(data, &dev->stats.curr_stat, sizeof(dev->stats.curr_stat));
	mutex_unlock(&dev->stats.access_lock);
}

static void lan78xx_get_wol(struct net_device *netdev,
			    struct ethtool_wolinfo *wol)
{
	struct lan78xx_net *dev = netdev_priv(netdev);
	int ret=0;
	u32 buf;
	struct lan78xx_priv *pdata = (struct lan78xx_priv *)(dev->data[0]);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (usb_autopm_get_interface(dev->intf) < 0)
		return;

	ret = lan78xx_read_reg(dev, USB_CFG0, &buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (unlikely(ret < 0)) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		wol->supported = 0;
		wol->wolopts = 0;
	} else {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (buf & USB_CFG_RMT_WKP_) {
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			wol->supported = WAKE_ALL;
			wol->wolopts = pdata->wol;
		} else {
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			wol->supported = 0;
			wol->wolopts = 0;
		}
	}

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	usb_autopm_put_interface(dev->intf);
}

static int lan78xx_set_wol(struct net_device *netdev,
			   struct ethtool_wolinfo *wol)
{
	struct lan78xx_net *dev = netdev_priv(netdev);
	struct lan78xx_priv *pdata = (struct lan78xx_priv *)(dev->data[0]);
	int ret;

	ret = usb_autopm_get_interface(dev->intf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (wol->wolopts & ~WAKE_ALL)
		return -EINVAL;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	pdata->wol = wol->wolopts;

	device_set_wakeup_enable(&dev->udev->dev, (bool)wol->wolopts);

	phy_ethtool_set_wol(netdev->phydev, wol);

	usb_autopm_put_interface(dev->intf);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return ret;
}

static int lan78xx_get_eee(struct net_device *net, struct ethtool_eee *edata)
{
	struct lan78xx_net *dev = netdev_priv(net);
	struct phy_device *phydev = net->phydev;
	int ret;
	u32 buf;

	ret = usb_autopm_get_interface(dev->intf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	ret = phy_ethtool_get_eee(phydev, edata);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		goto exit;

	ret = lan78xx_read_reg(dev, MAC_CR, &buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (buf & MAC_CR_EEE_EN_) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		edata->eee_enabled = true;
		edata->eee_active = !!(edata->advertised &
				       edata->lp_advertised);
		edata->tx_lpi_enabled = true;
		/* EEE_TX_LPI_REQ_DLY & tx_lpi_timer are same uSec unit */
		ret = lan78xx_read_reg(dev, EEE_TX_LPI_REQ_DLY, &buf);
		edata->tx_lpi_timer = buf;
	} else {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		edata->eee_enabled = false;
		edata->eee_active = false;
		edata->tx_lpi_enabled = false;
		edata->tx_lpi_timer = 0;
	}

	ret = 0;
exit:
	usb_autopm_put_interface(dev->intf);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return ret;
}

static int lan78xx_set_eee(struct net_device *net, struct ethtool_eee *edata)
{
	struct lan78xx_net *dev = netdev_priv(net);
	int ret;
	u32 buf;

	ret = usb_autopm_get_interface(dev->intf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (edata->eee_enabled) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		ret = lan78xx_read_reg(dev, MAC_CR, &buf);
		buf |= MAC_CR_EEE_EN_;
		ret = lan78xx_write_reg(dev, MAC_CR, buf);

		phy_ethtool_set_eee(net->phydev, edata);

		buf = (u32)edata->tx_lpi_timer;
		ret = lan78xx_write_reg(dev, EEE_TX_LPI_REQ_DLY, buf);
	} else {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		ret = lan78xx_read_reg(dev, MAC_CR, &buf);
		buf &= ~MAC_CR_EEE_EN_;
		ret = lan78xx_write_reg(dev, MAC_CR, buf);
	}

	usb_autopm_put_interface(dev->intf);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return 0;
}

static u32 lan78xx_get_link(struct net_device *net)
{
	u32 link;

	mutex_lock(&net->phydev->lock);
	phy_read_status(net->phydev);
	link = net->phydev->link;
	mutex_unlock(&net->phydev->lock);

	pr_debug(" SOFTING  %d %d", __LINE__, link);
	return link;
}

static void lan78xx_get_drvinfo(struct net_device *net,
				struct ethtool_drvinfo *info)
{
	struct lan78xx_net *dev = netdev_priv(net);

	pr_debug(" SOFTING  %d ", __LINE__);
	strncpy(info->driver, DRIVER_NAME, sizeof(info->driver));
	usb_make_path(dev->udev, info->bus_info, sizeof(info->bus_info));
}

static u32 lan78xx_get_msglevel(struct net_device *net)
{
	struct lan78xx_net *dev = netdev_priv(net);
	pr_debug(" SOFTING  %d %d", __LINE__,dev->msg_enable);

	return dev->msg_enable;
}

static void lan78xx_set_msglevel(struct net_device *net, u32 level)
{
	struct lan78xx_net *dev = netdev_priv(net);
	pr_debug(" SOFTING  %d ", __LINE__);

	dev->msg_enable = level;
}

static int lan78xx_get_link_ksettings(struct net_device *net,
				      struct ethtool_link_ksettings *cmd)
{
	struct lan78xx_net *dev = netdev_priv(net);
	struct phy_device *phydev = net->phydev;
	int ret;

	ret = usb_autopm_get_interface(dev->intf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	phy_ethtool_ksettings_get(phydev, cmd);

	usb_autopm_put_interface(dev->intf);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return ret;
}

static int lan78xx_set_link_ksettings(struct net_device *net,
				      const struct ethtool_link_ksettings *cmd)
{
	struct lan78xx_net *dev = netdev_priv(net);
	struct phy_device *phydev = net->phydev;
	int ret = 0;
	int temp;

	ret = usb_autopm_get_interface(dev->intf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	/* change speed & duplex */
	ret = phy_ethtool_ksettings_set(phydev, cmd);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (!cmd->base.autoneg) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		/* force link down */
		temp = phy_read(phydev, MII_BMCR);
		phy_write(phydev, MII_BMCR, temp | BMCR_LOOPBACK);
		mdelay(1);
		phy_write(phydev, MII_BMCR, temp);
	}

	usb_autopm_put_interface(dev->intf);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return ret;
}

static void lan78xx_get_pause(struct net_device *net,
			      struct ethtool_pauseparam *pause)
{
	struct lan78xx_net *dev = netdev_priv(net);
	struct phy_device *phydev = net->phydev;
	struct ethtool_link_ksettings ecmd;

	phy_ethtool_ksettings_get(phydev, &ecmd);

	pause->autoneg = dev->fc_autoneg;

	if (dev->fc_request_control & FLOW_CTRL_TX)
		pause->tx_pause = 1;

	if (dev->fc_request_control & FLOW_CTRL_RX)
		pause->rx_pause = 1;
	pr_debug(" SOFTING  %d ", __LINE__);
}

static int lan78xx_set_pause(struct net_device *net,
			     struct ethtool_pauseparam *pause)
{
	struct lan78xx_net *dev = netdev_priv(net);
	struct phy_device *phydev = net->phydev;
	struct ethtool_link_ksettings ecmd;
	int ret=0;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	phy_ethtool_ksettings_get(phydev, &ecmd);

	if (pause->autoneg && !ecmd.base.autoneg) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		ret = -EINVAL;
		goto exit;
	}

	dev->fc_request_control = 0;
	if (pause->rx_pause)
		dev->fc_request_control |= FLOW_CTRL_RX;

	if (pause->tx_pause)
		dev->fc_request_control |= FLOW_CTRL_TX;

	if (ecmd.base.autoneg) {
		__ETHTOOL_DECLARE_LINK_MODE_MASK(fc) = { 0, };
		u32 mii_adv;

		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		linkmode_clear_bit(ETHTOOL_LINK_MODE_Pause_BIT,
				   ecmd.link_modes.advertising);
		linkmode_clear_bit(ETHTOOL_LINK_MODE_Asym_Pause_BIT,
				   ecmd.link_modes.advertising);
		mii_adv = (u32)mii_advertise_flowctrl(dev->fc_request_control);
		mii_adv_to_linkmode_adv_t(fc, mii_adv);
		linkmode_or(ecmd.link_modes.advertising, fc,
			    ecmd.link_modes.advertising);

		phy_ethtool_ksettings_set(phydev, &ecmd);
	}

	dev->fc_autoneg = pause->autoneg;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	ret = 0;
exit:
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return ret;
}

static int lan78xx_get_regs_len(struct net_device *netdev)
{
	if (!netdev->phydev)
		return (sizeof(lan78xx_regs));
	else
		return (sizeof(lan78xx_regs) + PHY_REG_SIZE);
	pr_debug(" SOFTING  %d ", __LINE__);
}

static void
lan78xx_get_regs(struct net_device *netdev, struct ethtool_regs *regs,
		 void *buf)
{
	u32 *data = buf;
	int i, j;
	struct lan78xx_net *dev = netdev_priv(netdev);

	pr_debug(" SOFTING  %d ", __LINE__);
	/* Read Device/MAC registers */
	for (i = 0; i < ARRAY_SIZE(lan78xx_regs); i++)
		lan78xx_read_reg(dev, lan78xx_regs[i], &data[i]);

	pr_debug(" SOFTING  %d ", __LINE__);
	if (!netdev->phydev)
		return;

	pr_debug(" SOFTING  %d ", __LINE__);
	/* Read PHY registers */
	for (j = 0; j < 32; i++, j++)
		data[i] = phy_read(netdev->phydev, j);
	pr_debug(" SOFTING  %d ", __LINE__);
}

static const struct ethtool_ops lan78xx_ethtool_ops = {
	.get_link	= lan78xx_get_link,
	.nway_reset	= phy_ethtool_nway_reset,
	.get_drvinfo	= lan78xx_get_drvinfo,
	.get_msglevel	= lan78xx_get_msglevel,
	.set_msglevel	= lan78xx_set_msglevel,
	.get_eeprom_len = lan78xx_ethtool_get_eeprom_len,
	.get_eeprom	= lan78xx_ethtool_get_eeprom,
	.set_eeprom	= lan78xx_ethtool_set_eeprom,
	.get_ethtool_stats = lan78xx_get_stats,
	.get_sset_count = lan78xx_get_sset_count,
	.get_strings	= lan78xx_get_strings,
	.get_wol	= lan78xx_get_wol,
	.set_wol	= lan78xx_set_wol,
	.get_ts_info	= ethtool_op_get_ts_info,
	.get_eee	= lan78xx_get_eee,
	.set_eee	= lan78xx_set_eee,
	.get_pauseparam	= lan78xx_get_pause,
	.set_pauseparam	= lan78xx_set_pause,
	.get_link_ksettings = lan78xx_get_link_ksettings,
	.set_link_ksettings = lan78xx_set_link_ksettings,
	.get_regs_len	= lan78xx_get_regs_len,
	.get_regs	= lan78xx_get_regs,
};

static void lan78xx_init_mac_address(struct lan78xx_net *dev)
{
	u32 addr_lo, addr_hi;
	u8 addr[6];

	lan78xx_read_reg(dev, RX_ADDRL, &addr_lo);
	lan78xx_read_reg(dev, RX_ADDRH, &addr_hi);

	addr[0] = addr_lo & 0xFF;
	addr[1] = (addr_lo >> 8) & 0xFF;
	addr[2] = (addr_lo >> 16) & 0xFF;
	addr[3] = (addr_lo >> 24) & 0xFF;
	addr[4] = addr_hi & 0xFF;
	addr[5] = (addr_hi >> 8) & 0xFF;

	pr_debug(" SOFTING  %d ", __LINE__);
	if (!is_valid_ether_addr(addr)) {
		pr_debug(" SOFTING  %d ", __LINE__);
		if (!eth_platform_get_mac_address(&dev->udev->dev, addr)) {
			pr_debug(" SOFTING  %d ", __LINE__);
			/* valid address present in Device Tree */
			netif_dbg(dev, ifup, dev->net,
				  "MAC address read from Device Tree");
		} else if (((lan78xx_read_eeprom(dev, EEPROM_MAC_OFFSET,
						 ETH_ALEN, addr) == 0) ||
			    (lan78xx_read_otp(dev, EEPROM_MAC_OFFSET,
					      ETH_ALEN, addr) == 0)) &&
			   is_valid_ether_addr(addr)) {
			pr_debug(" SOFTING  %d ", __LINE__);
			/* eeprom values are valid so use them */
			netif_dbg(dev, ifup, dev->net,
				  "MAC address read from EEPROM");
		} else {
			pr_debug(" SOFTING  %d ", __LINE__);
			/* generate random MAC */
			eth_random_addr(addr);
			netif_dbg(dev, ifup, dev->net,
				  "MAC address set to random addr");
		}

		pr_debug(" SOFTING  %d ", __LINE__);
		addr_lo = addr[0] | (addr[1] << 8) |
			  (addr[2] << 16) | (addr[3] << 24);
		addr_hi = addr[4] | (addr[5] << 8);

		lan78xx_write_reg(dev, RX_ADDRL, addr_lo);
		lan78xx_write_reg(dev, RX_ADDRH, addr_hi);
	}

	pr_debug(" SOFTING  %d ", __LINE__);
	lan78xx_write_reg(dev, MAF_LO(0), addr_lo);
	lan78xx_write_reg(dev, MAF_HI(0), addr_hi | MAF_HI_VALID_);

	pr_debug(" SOFTING  %d ", __LINE__);
	ether_addr_copy(dev->net->dev_addr, addr);
}

/* MDIO read and write wrappers for phylib */
static int lan78xx_mdiobus_read(struct mii_bus *bus, int phy_id, int idx)
{
	struct lan78xx_net *dev = bus->priv;
	unsigned long start_time = jiffies;
	unsigned long phase_start;
	unsigned int prewait_ms = 0;
	unsigned int issue_ms = 0;
	unsigned int complete_ms = 0;
	unsigned int data_ms = 0;
	u32 val, addr;
	int ret = 0;

	ret = usb_autopm_get_interface(dev->intf);
	if (ret < 0)
		return ret;
	mutex_lock(&dev->mdiobus_mutex);

	/* confirm MII not busy */
	phase_start = jiffies;
	ret = lan78xx_mdiobus_wait_not_busy(dev);
	prewait_ms = jiffies_to_msecs(jiffies - phase_start);
	if (ret < 0)
		goto done;
	/* set the address, index & direction (read from PHY) */
	addr = mii_access(phy_id, idx, MII_READ);
	phase_start = jiffies;
	ret = lan78xx_write_reg(dev, MII_ACC, addr);
	issue_ms = jiffies_to_msecs(jiffies - phase_start);
	if (ret < 0)
		goto done;

	phase_start = jiffies;
	ret = lan78xx_mdiobus_wait_not_busy(dev);
	complete_ms = jiffies_to_msecs(jiffies - phase_start);
	if (ret < 0)
		goto done;
	phase_start = jiffies;
	ret = lan78xx_read_reg(dev, MII_DATA, &val);
	data_ms = jiffies_to_msecs(jiffies - phase_start);
	if (ret < 0)
		goto done;

	ret = (int)(val & 0xFFFF);

done:
	if (time_after(jiffies,
		       start_time + msecs_to_jiffies(LAN78XX_MDIO_STALL_MS)) ||
	    ret < 0) {
		unsigned int elapsed_ms = jiffies_to_msecs(jiffies - start_time);

		if (ret < 0)
			netdev_warn(dev->net,
				    "MDIO read phy %d reg 0x%02x failed after %u ms: %pe\n",
				    phy_id, idx, elapsed_ms, ERR_PTR(ret));
		else
			netdev_warn(dev->net,
				    "MDIO read phy %d reg 0x%02x took %u ms, value 0x%04x\n",
				    phy_id, idx, elapsed_ms, ret);
		netdev_info(dev->net,
			    "MDIO read phy %d reg 0x%02x phase times: prewait %u ms, issue %u ms, completion %u ms, data %u ms\n",
			    phy_id, idx, prewait_ms, issue_ms, complete_ms,
			    data_ms);
	} else {
		netdev_dbg(dev->net,
			   "MDIO read phy %d reg 0x%02x returned 0x%04x in %u ms\n",
			   phy_id, idx, ret,
			   jiffies_to_msecs(jiffies - start_time));
	}

	mutex_unlock(&dev->mdiobus_mutex);
	usb_autopm_put_interface(dev->intf);
	return ret;
}

static int lan78xx_mdiobus_write(struct mii_bus *bus, int phy_id, int idx,
				 u16 regval)
{
	struct lan78xx_net *dev = bus->priv;
	unsigned long start_time = jiffies;
	u32 val, addr;
	int ret;

	ret = usb_autopm_get_interface(dev->intf);
	if (ret < 0)
		return ret;

	mutex_lock(&dev->mdiobus_mutex);

	/* confirm MII not busy */
	ret = lan78xx_mdiobus_wait_not_busy(dev);
	if (ret < 0)
		goto done;

	val = (u32)regval;
	ret = lan78xx_write_reg(dev, MII_DATA, val);
	if (ret < 0)
		goto done;

	/* set the address, index & direction (write to PHY) */
	addr = mii_access(phy_id, idx, MII_WRITE);
	ret = lan78xx_write_reg(dev, MII_ACC, addr);
	if (ret < 0)
		goto done;

	ret = lan78xx_mdiobus_wait_not_busy(dev);
	if (ret < 0)
		goto done;

done:
	if (time_after(jiffies,
		       start_time + msecs_to_jiffies(LAN78XX_MDIO_STALL_MS)) ||
	    ret < 0) {
		unsigned int elapsed_ms = jiffies_to_msecs(jiffies - start_time);

		if (ret < 0)
			netdev_warn(dev->net,
				    "MDIO write phy %d reg 0x%02x value 0x%04x failed after %u ms: %pe\n",
				    phy_id, idx, regval, elapsed_ms,
				    ERR_PTR(ret));
		else
			netdev_warn(dev->net,
				    "MDIO write phy %d reg 0x%02x value 0x%04x took %u ms\n",
				    phy_id, idx, regval, elapsed_ms);
	} else {
		netdev_dbg(dev->net,
			   "MDIO write phy %d reg 0x%02x value 0x%04x completed in %u ms\n",
			   phy_id, idx, regval,
			   jiffies_to_msecs(jiffies - start_time));
	}

	mutex_unlock(&dev->mdiobus_mutex);
	usb_autopm_put_interface(dev->intf);
	return ret;
}

static int lan78xx_mdio_init(struct lan78xx_net *dev)
{
	struct device_node *node;
	int ret=0;

	dev->mdiobus = mdiobus_alloc();
	if (!dev->mdiobus) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		netdev_err(dev->net, "can't allocate MDIO bus\n");
		return -ENOMEM;
	}

	dev->mdiobus->priv = (void *)dev;
	dev->mdiobus->read = lan78xx_mdiobus_read;
	dev->mdiobus->write = lan78xx_mdiobus_write;
	dev->mdiobus->name = "lan78xx-mdiobus";
	dev->mdiobus->parent = &dev->udev->dev;

	snprintf(dev->mdiobus->id, MII_BUS_ID_SIZE, "usb-%03d:%03d",
		 dev->udev->bus->busnum, dev->udev->devnum);

	switch (dev->chipid) {
	case ID_REV_CHIP_ID_7800_:
	case ID_REV_CHIP_ID_7850_:
		/* set to internal PHY id */
		dev->mdiobus->phy_mask = ~(1 << 1);
		break;
	case ID_REV_CHIP_ID_7801_:
		/* scan thru PHYAD[2..0] */
		dev->mdiobus->phy_mask = ~(0xFF);
		break;
	}

	node = of_get_child_by_name(dev->udev->dev.of_node, "mdio");
	ret = of_mdiobus_register(dev->mdiobus, node);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	of_node_put(node);
	if (ret) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		netdev_err(dev->net, "can't register MDIO bus\n");
		goto exit1;
	}

	netdev_dbg(dev->net, "registered mdiobus bus %s\n", dev->mdiobus->id);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return 0;
exit1:
	mdiobus_free(dev->mdiobus);
	return ret;
}

static void lan78xx_remove_mdio(struct lan78xx_net *dev)
{
	pr_debug(" SOFTING  %d ", __LINE__);
	mdiobus_unregister(dev->mdiobus);
	mdiobus_free(dev->mdiobus);
}

static void lan78xx_link_status_change(struct net_device *net)
{
	struct lan78xx_net *dev = netdev_priv(net);
	struct phy_device *phydev = net->phydev;
	struct ethtool_eee eee;
	u32 data;
	int ret;

	ret = lan78xx_read_reg(dev, MAC_CR, &data);
	if (ret < 0)
		return;

	/* Check EEE status using ethtool interface */
	if (phy_ethtool_get_eee(phydev, &eee) == 0 && eee.tx_lpi_enabled)
		data |=  MAC_CR_EEE_EN_;
	else
		data &= ~MAC_CR_EEE_EN_;
	lan78xx_write_reg(dev, MAC_CR, data);

	phy_print_status(phydev);
}

static int irq_map(struct irq_domain *d, unsigned int irq,
		   irq_hw_number_t hwirq)
{
	struct irq_domain_data *data = d->host_data;
	pr_debug(" SOFTING  %d ", __LINE__);

	irq_set_chip_data(irq, data);
	irq_set_chip_and_handler(irq, data->irqchip, data->irq_handler);
	irq_set_noprobe(irq);

	return 0;
}

static void irq_unmap(struct irq_domain *d, unsigned int irq)
{
	pr_debug(" SOFTING  %d ", __LINE__);
	irq_set_chip_and_handler(irq, NULL, NULL);
	irq_set_chip_data(irq, NULL);
}

static const struct irq_domain_ops chip_domain_ops = {
	.map	= irq_map,
	.unmap	= irq_unmap,
};

static void lan78xx_irq_mask(struct irq_data *irqd)
{
	struct irq_domain_data *data = irq_data_get_irq_chip_data(irqd);
	pr_debug(" SOFTING  %d ", __LINE__);

	data->irqenable &= ~BIT(irqd_to_hwirq(irqd));
}

static void lan78xx_irq_unmask(struct irq_data *irqd)
{
	struct irq_domain_data *data = irq_data_get_irq_chip_data(irqd);
	pr_debug(" SOFTING  %d ", __LINE__);

	data->irqenable |= BIT(irqd_to_hwirq(irqd));
}

static void lan78xx_irq_bus_lock(struct irq_data *irqd)
{
	struct irq_domain_data *data = irq_data_get_irq_chip_data(irqd);
	pr_debug(" SOFTING  %d ", __LINE__);

	mutex_lock(&data->irq_lock);
}

static void lan78xx_irq_bus_sync_unlock(struct irq_data *irqd)
{
	struct irq_domain_data *data = irq_data_get_irq_chip_data(irqd);
	struct lan78xx_net *dev =
			container_of(data, struct lan78xx_net, domain_data);
	u32 buf;

	pr_debug(" SOFTING  %d ", __LINE__);
	/* call register access here because irq_bus_lock & irq_bus_sync_unlock
	 * are only two callbacks executed in non-atomic contex.
	 */
	lan78xx_read_reg(dev, INT_EP_CTL, &buf);
	if (buf != data->irqenable)
		lan78xx_write_reg(dev, INT_EP_CTL, data->irqenable);

	pr_debug(" SOFTING  %d ", __LINE__);
	mutex_unlock(&data->irq_lock);
}

static struct irq_chip lan78xx_irqchip = {
	.name			= "lan78xx-irqs",
	.irq_mask		= lan78xx_irq_mask,
	.irq_unmask		= lan78xx_irq_unmask,
	.irq_bus_lock		= lan78xx_irq_bus_lock,
	.irq_bus_sync_unlock	= lan78xx_irq_bus_sync_unlock,
};

static int lan78xx_setup_irq_domain(struct lan78xx_net *dev)
{
	struct device_node *of_node;
	struct irq_domain *irqdomain;
	unsigned int irqmap = 0;
	u32 buf;
	int ret = 0;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	of_node = dev->udev->dev.parent->of_node;

	mutex_init(&dev->domain_data.irq_lock);

	lan78xx_read_reg(dev, INT_EP_CTL, &buf);
	dev->domain_data.irqenable = buf;

	dev->domain_data.irqchip = &lan78xx_irqchip;
	dev->domain_data.irq_handler = handle_simple_irq;

	irqdomain = irq_domain_add_simple(of_node, MAX_INT_EP, 0,
					  &chip_domain_ops, &dev->domain_data);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (irqdomain) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		/* create mapping for PHY interrupt */
		irqmap = irq_create_mapping(irqdomain, INT_EP_PHY);
		if (!irqmap) {
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			irq_domain_remove(irqdomain);

			irqdomain = NULL;
			ret = -EINVAL;
		}
	} else {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		ret = -EINVAL;
	}

	dev->domain_data.irqdomain = irqdomain;
	dev->domain_data.phyirq = irqmap;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return ret;
}

static void lan78xx_remove_irq_domain(struct lan78xx_net *dev)
{
	if (dev->domain_data.phyirq > 0) {
		pr_debug(" SOFTING  %d ", __LINE__);
		irq_dispose_mapping(dev->domain_data.phyirq);

		if (dev->domain_data.irqdomain)
			irq_domain_remove(dev->domain_data.irqdomain);
	}
	dev->domain_data.phyirq = 0;
	dev->domain_data.irqdomain = NULL;
}


/**
 * lan78xx_register_fixed_phy() - Register a fallback fixed PHY
 * @dev: LAN78xx device
 *
 * Registers a fixed PHY with 1 Gbps full duplex. This is used in special cases
 * like EVB-KSZ9897-1, where LAN7801 acts as a USB-to-Ethernet interface to a
 * switch without a visible PHY.
 *
 * Return: pointer to the registered fixed PHY, or ERR_PTR() on error.
 */
static struct phy_device *lan78xx_register_fixed_phy(struct lan78xx_net *dev)
{
	struct fixed_phy_status fphy_status = {
		.link = 1,
		.speed = SPEED_1000,
		.duplex = DUPLEX_FULL,
	};

	netdev_info(dev->net,
		    "No PHY found on LAN7801 – registering fixed PHY (e.g. EVB-KSZ9897-1)\n");

	return fixed_phy_register(PHY_POLL, &fphy_status, NULL);
}

/**
 * lan78xx_get_phy() - Probe or register PHY device and set interface mode
 * @dev: LAN78xx device structure
 *
 * This function attempts to find a PHY on the MDIO bus. If no PHY is found
 * and the chip is LAN7801, it registers a fixed PHY as fallback. It also
 * sets dev->interface based on chip ID and detected PHY type.
 *
 * Return: a valid PHY device pointer, or ERR_PTR() on failure.
 */
static struct phy_device *lan78xx_get_phy(struct lan78xx_net *dev)
{
	struct phy_device *phydev;

	/* Attempt to locate a PHY on the MDIO bus */
	phydev = phy_find_first(dev->mdiobus);

	switch (dev->chipid) {
	case ID_REV_CHIP_ID_7801_:
		if (phydev) {
			/* External RGMII PHY detected */
			dev->interface = PHY_INTERFACE_MODE_RGMII_ID;
			phydev->is_internal = false;

			if (!phydev->drv)
				netdev_warn(dev->net,
					    "PHY driver not found – assuming RGMII delays are on PCB or strapped for the PHY\n");

			return phydev;
		}

		dev->interface = PHY_INTERFACE_MODE_RGMII;
		/* No PHY found – fallback to fixed PHY (e.g. KSZ switch board) */
		return lan78xx_register_fixed_phy(dev);

	case ID_REV_CHIP_ID_7800_:
	case ID_REV_CHIP_ID_7850_:
		if (!phydev)
			return ERR_PTR(-ENODEV);

		/* These use internal GMII-connected PHY */
		dev->interface = PHY_INTERFACE_MODE_GMII;
		phydev->is_internal = true;
		return phydev;

	default:
		netdev_err(dev->net, "Unknown CHIP ID: 0x%08x\n", dev->chipid);
		return ERR_PTR(-ENODEV);
	}
}

/**
 * lan78xx_mac_prepare_for_phy() - Preconfigure MAC-side interface settings
 * @dev: LAN78xx device
 *
 * Configure MAC-side registers according to dev->interface, which should be
 * set by lan78xx_get_phy().
 *
 * Return: 0 on success or a negative error code.
 */
static int lan78xx_mac_prepare_for_phy(struct lan78xx_net *dev)
{
	int ret;

	switch (dev->interface) {
	case PHY_INTERFACE_MODE_RGMII:
		/* Enable MAC-side TX clock delay */
		ret = lan78xx_write_reg(dev, MAC_RGMII_ID,
					MAC_RGMII_ID_TXC_DELAY_EN_);
		if (ret < 0)
			return ret;

		ret = lan78xx_write_reg(dev, RGMII_TX_BYP_DLL, 0x3D00);
		if (ret < 0)
			return ret;

		ret = lan78xx_update_reg(dev, HW_CFG,
					 HW_CFG_CLK125_EN_ | HW_CFG_REFCLK25_EN_,
					 HW_CFG_CLK125_EN_ | HW_CFG_REFCLK25_EN_);
		if (ret < 0)
			return ret;

		break;

	case PHY_INTERFACE_MODE_RGMII_ID:
		/* Disable MAC-side TXC delay, PHY provides it */
		ret = lan78xx_write_reg(dev, MAC_RGMII_ID, 0);
		if (ret < 0)
			return ret;

		break;

	case PHY_INTERFACE_MODE_GMII:
		/* No MAC-specific configuration required */
		break;

	default:
		netdev_warn(dev->net, "Unsupported interface mode: %d\n",
			    dev->interface);
		break;
	}

	return 0;
}

/**
 * lan78xx_configure_leds_from_dt() - Configure LED enables based on DT
 * @dev: LAN78xx device
 * @phydev: PHY device (must be valid)
 *
 * Reads "microchip,led-modes" property from the PHY's DT node and enables
 * the corresponding number of LEDs by writing to HW_CFG.
 *
 * This helper preserves the original logic, enabling up to 4 LEDs.
 * If the property is not present, this function does nothing.
 *
 * Return: 0 on success or a negative error code.
 */
static int lan78xx_configure_leds_from_dt(struct lan78xx_net *dev,
					  struct phy_device *phydev)
{
	struct device_node *np = phydev->mdio.dev.of_node;
	u32 reg;
	int len, ret;

	if (!np)
		return 0;

	len = of_property_count_elems_of_size(np, "microchip,led-modes",
					      sizeof(u32));
	if (len < 0)
		return 0;

	ret = lan78xx_read_reg(dev, HW_CFG, &reg);
	if (ret < 0)
		return ret;

	reg &= ~(HW_CFG_LED0_EN_ | HW_CFG_LED1_EN_ |
		 HW_CFG_LED2_EN_ | HW_CFG_LED3_EN_);

	reg |= (len > 0) * HW_CFG_LED0_EN_ |
	       (len > 1) * HW_CFG_LED1_EN_ |
	       (len > 2) * HW_CFG_LED2_EN_ |
	       (len > 3) * HW_CFG_LED3_EN_;

	return lan78xx_write_reg(dev, HW_CFG, reg);
}

static int lan78xx_phy_init(struct lan78xx_net *dev)
{
	__ETHTOOL_DECLARE_LINK_MODE_MASK(fc) = { 0, };
	int ret=0;
	u32 mii_adv;
	struct phy_device *phydev;

	phydev = lan78xx_get_phy(dev);
	if (IS_ERR(phydev))
		return PTR_ERR(phydev);

	ret = lan78xx_mac_prepare_for_phy(dev);
	if (ret < 0)
		goto free_phy;

	/* if phyirq is not set, use polling mode in phylib */
	if (dev->domain_data.phyirq > 0) {
		phydev->irq = dev->domain_data.phyirq;
		netdev_dbg(dev->net, "Using PHY interrupt mode, irq = %d\n", phydev->irq);
	} else {
		phydev->irq = PHY_POLL;
		netdev_dbg(dev->net, "Using PHY polling mode\n");
	}
	phydev->irq = PHY_POLL;

	/* set to AUTOMDIX */
	phydev->mdix = ETH_TP_MDI_AUTO;

	ret = phy_connect_direct(dev->net, phydev,
				 lan78xx_link_status_change,
				 dev->interface);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		netdev_err(dev->net, "can't attach PHY to %s\n",
			   dev->mdiobus->id);
		if (dev->chipid == ID_REV_CHIP_ID_7801_) {
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			if (phy_is_pseudo_fixed_link(phydev)) {
				pr_debug(" SOFTING  %d %d", __LINE__, ret);
				fixed_phy_unregister(phydev);
			} else {
				pr_debug(" SOFTING  %d %d", __LINE__, ret);
				phy_unregister_fixup_for_uid(PHY_KSZ9031RNX,
							     0xfffffff0);
				phy_unregister_fixup_for_uid(PHY_LAN8835,
							     0xfffffff0);
			}
		}
		return -EIO;
	}

	/* MAC doesn't support 1000T Half */
	phy_remove_link_mode(phydev, ETHTOOL_LINK_MODE_1000baseT_Half_BIT);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	/* support both flow controls */
	dev->fc_request_control = (FLOW_CTRL_RX | FLOW_CTRL_TX);
	linkmode_clear_bit(ETHTOOL_LINK_MODE_Pause_BIT,
			   phydev->advertising);
	linkmode_clear_bit(ETHTOOL_LINK_MODE_Asym_Pause_BIT,
			   phydev->advertising);
	mii_adv = (u32)mii_advertise_flowctrl(dev->fc_request_control);
	mii_adv_to_linkmode_adv_t(fc, mii_adv);
	linkmode_or(phydev->advertising, fc, phydev->advertising);

	ret = lan78xx_configure_leds_from_dt(dev, phydev);
	if (ret)
		goto free_phy;

	genphy_config_aneg(phydev);

	dev->fc_autoneg = phydev->autoneg;

	return 0;

free_phy:
	if (phy_is_pseudo_fixed_link(phydev)) {
		fixed_phy_unregister(phydev);
		phy_device_free(phydev);
	}

	return ret;
}

static int lan78xx_set_rx_max_frame_length(struct lan78xx_net *dev, int size)
{
	u32 buf;
	bool rxenabled;

	pr_debug(" SOFTING  %d", __LINE__);
	lan78xx_read_reg(dev, MAC_RX, &buf);

	rxenabled = ((buf & MAC_RX_RXEN_) != 0);

	pr_debug(" SOFTING  %d", __LINE__);
	if (rxenabled) {
		pr_debug(" SOFTING  %d", __LINE__);
		buf &= ~MAC_RX_RXEN_;
		lan78xx_write_reg(dev, MAC_RX, buf);
	}

	pr_debug(" SOFTING  %d", __LINE__);
	/* add 4 to size for FCS */
	buf &= ~MAC_RX_MAX_SIZE_MASK_;
	buf |= (((size + 4) << MAC_RX_MAX_SIZE_SHIFT_) & MAC_RX_MAX_SIZE_MASK_);

	lan78xx_write_reg(dev, MAC_RX, buf);

	pr_debug(" SOFTING  %d", __LINE__);
	if (rxenabled) {
		pr_debug(" SOFTING  %d", __LINE__);
		buf |= MAC_RX_RXEN_;
		lan78xx_write_reg(dev, MAC_RX, buf);
	}

	pr_debug(" SOFTING  %d", __LINE__);
	return 0;
}

static int unlink_urbs(struct lan78xx_net *dev, struct sk_buff_head *q)
{
	struct sk_buff *skb;
	unsigned long flags;
	int count = 0;

	spin_lock_irqsave(&q->lock, flags);
	while (!skb_queue_empty(q)) {
		struct skb_data	*entry;
		struct urb *urb;
		int ret;

		pr_debug(" SOFTING  %d", __LINE__);
		skb_queue_walk(q, skb) {
			entry = (struct skb_data *)skb->cb;
			if (entry->state != unlink_start)
				goto found;
		}
		break;
found:
		entry->state = unlink_start;
		urb = entry->urb;

		/* Get reference count of the URB to avoid it to be
		 * freed during usb_unlink_urb, which may trigger
		 * use-after-free problem inside usb_unlink_urb since
		 * usb_unlink_urb is always racing with .complete
		 * handler(include defer_bh).
		 */
		usb_get_urb(urb);
		spin_unlock_irqrestore(&q->lock, flags);
		/* during some PM-driven resume scenarios,
		 * these (async) unlinks complete immediately
		 */
		ret = usb_unlink_urb(urb);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret != -EINPROGRESS && ret != 0)
			netdev_dbg(dev->net, "unlink urb err, %d\n", ret);
		else
			count++;
		usb_put_urb(urb);
		spin_lock_irqsave(&q->lock, flags);
	}
	spin_unlock_irqrestore(&q->lock, flags);
	return count;
}

static int lan78xx_change_mtu(struct net_device *netdev, int new_mtu)
{
	struct lan78xx_net *dev = netdev_priv(netdev);
	int ll_mtu = new_mtu + netdev->hard_header_len;
	int old_hard_mtu = dev->hard_mtu;
	int old_rx_urb_size = dev->rx_urb_size;
	int ret=0;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	/* no second zero-length packet read wanted after mtu-sized packets */
	if ((ll_mtu % dev->maxpacket) == 0)
		return -EDOM;

	ret = usb_autopm_get_interface(dev->intf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	lan78xx_set_rx_max_frame_length(dev, new_mtu + VLAN_ETH_HLEN);

	netdev->mtu = new_mtu;

	dev->hard_mtu = netdev->mtu + netdev->hard_header_len;
	if (dev->rx_urb_size == old_hard_mtu) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		dev->rx_urb_size = dev->hard_mtu;
		if (dev->rx_urb_size > old_rx_urb_size) {
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			if (netif_running(dev->net)) {
				pr_debug(" SOFTING  %d %d", __LINE__, ret);
				unlink_urbs(dev, &dev->rxq);
				tasklet_schedule(&dev->bh);
			}
		}
	}

	usb_autopm_put_interface(dev->intf);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return 0;
}

static int lan78xx_set_mac_addr(struct net_device *netdev, void *p)
{
	struct lan78xx_net *dev = netdev_priv(netdev);
	struct sockaddr *addr = p;
	u32 addr_lo, addr_hi;

	pr_debug(" SOFTING  %d ", __LINE__);
	if (netif_running(netdev))
		return -EBUSY;

	pr_debug(" SOFTING  %d ", __LINE__);
	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	pr_debug(" SOFTING  %d ", __LINE__);
	ether_addr_copy(netdev->dev_addr, addr->sa_data);

	pr_debug(" SOFTING  %d ", __LINE__);
	addr_lo = netdev->dev_addr[0] |
		  netdev->dev_addr[1] << 8 |
		  netdev->dev_addr[2] << 16 |
		  netdev->dev_addr[3] << 24;
	addr_hi = netdev->dev_addr[4] |
		  netdev->dev_addr[5] << 8;

	lan78xx_write_reg(dev, RX_ADDRL, addr_lo);
	lan78xx_write_reg(dev, RX_ADDRH, addr_hi);

	pr_debug(" SOFTING  %d ", __LINE__);
	/* Added to support MAC address changes */
	lan78xx_write_reg(dev, MAF_LO(0), addr_lo);
	lan78xx_write_reg(dev, MAF_HI(0), addr_hi | MAF_HI_VALID_);

	pr_debug(" SOFTING  %d ", __LINE__);
	return 0;
}

/* Enable or disable Rx checksum offload engine */
static int lan78xx_set_features(struct net_device *netdev,
				netdev_features_t features)
{
	struct lan78xx_net *dev = netdev_priv(netdev);
	struct lan78xx_priv *pdata = (struct lan78xx_priv *)(dev->data[0]);
	unsigned long flags;

	pr_debug(" SOFTING  %d ", __LINE__);
	spin_lock_irqsave(&pdata->rfe_ctl_lock, flags);

	if (features & NETIF_F_RXCSUM) {
		pr_debug(" SOFTING  %d ", __LINE__);
		pdata->rfe_ctl |= RFE_CTL_TCPUDP_COE_ | RFE_CTL_IP_COE_;
		pdata->rfe_ctl |= RFE_CTL_ICMP_COE_ | RFE_CTL_IGMP_COE_;
	} else {
		pr_debug(" SOFTING  %d ", __LINE__);
		pdata->rfe_ctl &= ~(RFE_CTL_TCPUDP_COE_ | RFE_CTL_IP_COE_);
		pdata->rfe_ctl &= ~(RFE_CTL_ICMP_COE_ | RFE_CTL_IGMP_COE_);
	}

	if (features & NETIF_F_HW_VLAN_CTAG_RX)
		pdata->rfe_ctl |= RFE_CTL_VLAN_STRIP_;
	else
		pdata->rfe_ctl &= ~RFE_CTL_VLAN_STRIP_;

	pr_debug(" SOFTING  %d ", __LINE__);
	if (features & NETIF_F_HW_VLAN_CTAG_FILTER)
		pdata->rfe_ctl |= RFE_CTL_VLAN_FILTER_;
	else
		pdata->rfe_ctl &= ~RFE_CTL_VLAN_FILTER_;

	pr_debug(" SOFTING  %d ", __LINE__);
	spin_unlock_irqrestore(&pdata->rfe_ctl_lock, flags);

	lan78xx_write_reg(dev, RFE_CTL, pdata->rfe_ctl);

	pr_debug(" SOFTING  %d ", __LINE__);
	return 0;
}

static void lan78xx_deferred_vlan_write(struct work_struct *param)
{
	struct lan78xx_priv *pdata =
			container_of(param, struct lan78xx_priv, set_vlan);
	struct lan78xx_net *dev = pdata->dev;

	pr_debug(" SOFTING  %d ", __LINE__);
	lan78xx_dataport_write(dev, DP_SEL_RSEL_VLAN_DA_, 0,
			       DP_SEL_VHF_VLAN_LEN, pdata->vlan_table);
}

static int lan78xx_vlan_rx_add_vid(struct net_device *netdev,
				   __be16 proto, u16 vid)
{
	struct lan78xx_net *dev = netdev_priv(netdev);
	struct lan78xx_priv *pdata = (struct lan78xx_priv *)(dev->data[0]);
	u16 vid_bit_index;
	u16 vid_dword_index;

	pr_debug(" SOFTING  %d ", __LINE__);
	vid_dword_index = (vid >> 5) & 0x7F;
	vid_bit_index = vid & 0x1F;

	pdata->vlan_table[vid_dword_index] |= (1 << vid_bit_index);

	/* defer register writes to a sleepable context */
	schedule_work(&pdata->set_vlan);

	pr_debug(" SOFTING  %d ", __LINE__);
	return 0;
}

static int lan78xx_vlan_rx_kill_vid(struct net_device *netdev,
				    __be16 proto, u16 vid)
{
	struct lan78xx_net *dev = netdev_priv(netdev);
	struct lan78xx_priv *pdata = (struct lan78xx_priv *)(dev->data[0]);
	u16 vid_bit_index;
	u16 vid_dword_index;

	vid_dword_index = (vid >> 5) & 0x7F;
	vid_bit_index = vid & 0x1F;

	pdata->vlan_table[vid_dword_index] &= ~(1 << vid_bit_index);

	/* defer register writes to a sleepable context */
	schedule_work(&pdata->set_vlan);

	pr_debug(" SOFTING  %d ", __LINE__);
	return 0;
}

static void lan78xx_init_ltm(struct lan78xx_net *dev)
{
	int ret;
	u32 buf;
	u32 regs[6] = { 0 };

	ret = lan78xx_read_reg(dev, USB_CFG1, &buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (buf & USB_CFG1_LTM_ENABLE_) {
		u8 temp[2];
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		/* Get values from EEPROM first */
		if (lan78xx_read_eeprom(dev, 0x3F, 2, temp) == 0) {
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			if (temp[0] == 24) {
				ret = lan78xx_read_raw_eeprom(dev,
							      temp[1] * 2,
							      24,
							      (u8 *)regs);
				pr_debug(" SOFTING  %d %d", __LINE__, ret);
				if (ret < 0)
					return;
			}
		} else if (lan78xx_read_otp(dev, 0x3F, 2, temp) == 0) {
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			if (temp[0] == 24) {
				ret = lan78xx_read_raw_otp(dev,
							   temp[1] * 2,
							   24,
							   (u8 *)regs);
				pr_debug(" SOFTING  %d %d", __LINE__, ret);
				if (ret < 0)
					return;
			}
		}
	}

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	lan78xx_write_reg(dev, LTM_BELT_IDLE0, regs[0]);
	lan78xx_write_reg(dev, LTM_BELT_IDLE1, regs[1]);
	lan78xx_write_reg(dev, LTM_BELT_ACT0, regs[2]);
	lan78xx_write_reg(dev, LTM_BELT_ACT1, regs[3]);
	lan78xx_write_reg(dev, LTM_INACTIVE0, regs[4]);
	lan78xx_write_reg(dev, LTM_INACTIVE1, regs[5]);
}

static int lan78xx_start_hw(struct lan78xx_net *dev, u32 reg, u32 hw_enable)
{
	return lan78xx_update_reg(dev, reg, hw_enable, hw_enable);
}

static int lan78xx_stop_hw(struct lan78xx_net *dev, u32 reg, u32 hw_enabled,
			   u32 hw_disabled)
{
	unsigned long timeout;
	bool stopped = true;
	int ret;
	u32 buf;

	/* Stop the h/w block (if not already stopped) */

	ret = lan78xx_read_reg(dev, reg, &buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (buf & hw_enabled) {
		buf &= ~hw_enabled;

		ret = lan78xx_write_reg(dev, reg, buf);
		if (ret < 0)
			return ret;

		stopped = false;
		timeout = jiffies + HW_DISABLE_TIMEOUT;
		do  {
			ret = lan78xx_read_reg(dev, reg, &buf);
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			if (ret < 0)
				return ret;

			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			if (buf & hw_disabled)
				stopped = true;
			else
				msleep(HW_DISABLE_DELAY_MS);
		} while (!stopped && !time_after(jiffies, timeout));
	}

	ret = stopped ? 0 : -ETIMEDOUT;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return ret;
}

static int lan78xx_flush_fifo(struct lan78xx_net *dev, u32 reg, u32 fifo_flush)
{
	return lan78xx_update_reg(dev, reg, fifo_flush, fifo_flush);
}

static int lan78xx_start_tx_path(struct lan78xx_net *dev)
{
	int ret=0;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	netif_dbg(dev, drv, dev->net, "start tx path");

	/* Start the MAC transmitter */

	ret = lan78xx_start_hw(dev, MAC_TX, MAC_TX_TXEN_);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	/* Start the Tx FIFO */

	ret = lan78xx_start_hw(dev, FCT_TX_CTL, FCT_TX_CTL_EN_);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return 0;
}

static int lan78xx_stop_tx_path(struct lan78xx_net *dev)
{
	int ret;

	netif_dbg(dev, drv, dev->net, "stop tx path");

	/* Stop the Tx FIFO */

	ret = lan78xx_stop_hw(dev, FCT_TX_CTL, FCT_TX_CTL_EN_, FCT_TX_CTL_DIS_);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	/* Stop the MAC transmitter */

	ret = lan78xx_stop_hw(dev, MAC_TX, MAC_TX_TXEN_, MAC_TX_TXD_);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return 0;
}

/* The caller must ensure the Tx path is stopped before calling
 * lan78xx_flush_tx_fifo().
 */
static int lan78xx_flush_tx_fifo(struct lan78xx_net *dev)
{
	return lan78xx_flush_fifo(dev, FCT_TX_CTL, FCT_TX_CTL_RST_);
}

static int lan78xx_start_rx_path(struct lan78xx_net *dev)
{
	int ret=0;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	netif_dbg(dev, drv, dev->net, "start rx path");

	/* Start the Rx FIFO */

	ret = lan78xx_start_hw(dev, FCT_RX_CTL, FCT_RX_CTL_EN_);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	/* Start the MAC receiver*/

	ret = lan78xx_start_hw(dev, MAC_RX, MAC_RX_RXEN_);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return 0;
}

static int lan78xx_stop_rx_path(struct lan78xx_net *dev)
{
	int ret;

	netif_dbg(dev, drv, dev->net, "stop rx path");

	/* Stop the MAC receiver */

	ret = lan78xx_stop_hw(dev, MAC_RX, MAC_RX_RXEN_, MAC_RX_RXD_);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	/* Stop the Rx FIFO */

	ret = lan78xx_stop_hw(dev, FCT_RX_CTL, FCT_RX_CTL_EN_, FCT_RX_CTL_DIS_);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	return 0;
}

/* The caller must ensure the Rx path is stopped before calling
 * lan78xx_flush_rx_fifo().
 */
static int lan78xx_flush_rx_fifo(struct lan78xx_net *dev)
{
	return lan78xx_flush_fifo(dev, FCT_RX_CTL, FCT_RX_CTL_RST_);
}

static int lan78xx_reset(struct lan78xx_net *dev)
{
	struct lan78xx_priv *pdata = (struct lan78xx_priv *)(dev->data[0]);
	unsigned long timeout;
	int ret;
	u32 buf;
	u8 sig;

	ret = lan78xx_read_reg(dev, HW_CFG, &buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	buf |= HW_CFG_LRST_;

	ret = lan78xx_write_reg(dev, HW_CFG, buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	timeout = jiffies + HZ;
	do {
		mdelay(1);
		ret = lan78xx_read_reg(dev, HW_CFG, &buf);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			return ret;

		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (time_after(jiffies, timeout)) {
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			netdev_warn(dev->net,
				    "timeout on completion of LiteReset");
			ret = -ETIMEDOUT;
			return ret;
		}
	} while (buf & HW_CFG_LRST_);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	lan78xx_init_mac_address(dev);

	/* save DEVID for later usage */
	ret = lan78xx_read_reg(dev, ID_REV, &buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	dev->chipid = (buf & ID_REV_CHIP_ID_MASK_) >> 16;
	dev->chiprev = buf & ID_REV_CHIP_REV_MASK_;

	/* Respond to the IN token with a NAK */
	ret = lan78xx_read_reg(dev, USB_CFG0, &buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	buf |= USB_CFG_BIR_;

	ret = lan78xx_write_reg(dev, USB_CFG0, buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	/* Init LTM */
	lan78xx_init_ltm(dev);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);

	if (dev->udev->speed == USB_SPEED_SUPER) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		buf = DEFAULT_BURST_CAP_SIZE / SS_USB_PKT_SIZE;
		dev->rx_urb_size = DEFAULT_BURST_CAP_SIZE;
		dev->rx_qlen = 4;
		dev->tx_qlen = 4;
	} else if (dev->udev->speed == USB_SPEED_HIGH) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		buf = DEFAULT_BURST_CAP_SIZE / HS_USB_PKT_SIZE;
		dev->rx_urb_size = DEFAULT_BURST_CAP_SIZE;
		dev->rx_qlen = RX_MAX_QUEUE_MEMORY / dev->rx_urb_size;
		dev->tx_qlen = RX_MAX_QUEUE_MEMORY / dev->hard_mtu;
	} else {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		buf = DEFAULT_BURST_CAP_SIZE / FS_USB_PKT_SIZE;
		dev->rx_urb_size = DEFAULT_BURST_CAP_SIZE;
		dev->rx_qlen = 4;
		dev->tx_qlen = 4;
	}

	ret = lan78xx_write_reg(dev, BURST_CAP, buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	ret = lan78xx_write_reg(dev, BULK_IN_DLY, DEFAULT_BULK_IN_DELAY);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	ret = lan78xx_read_reg(dev, HW_CFG, &buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	buf |= HW_CFG_MEF_;

	ret = lan78xx_write_reg(dev, HW_CFG, buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	ret = lan78xx_read_reg(dev, USB_CFG0, &buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	buf |= USB_CFG_BCE_;

	ret = lan78xx_write_reg(dev, USB_CFG0, buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	/* set FIFO sizes */
	buf = (MAX_RX_FIFO_SIZE - 512) / 512;

	ret = lan78xx_write_reg(dev, FCT_RX_FIFO_END, buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	buf = (MAX_TX_FIFO_SIZE - 512) / 512;

	ret = lan78xx_write_reg(dev, FCT_TX_FIFO_END, buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	ret = lan78xx_write_reg(dev, INT_STS, INT_STS_CLEAR_ALL_);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	ret = lan78xx_write_reg(dev, FLOW, 0);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	ret = lan78xx_write_reg(dev, FCT_FLOW, 0);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	/* Don't need rfe_ctl_lock during initialisation */
	ret = lan78xx_read_reg(dev, RFE_CTL, &pdata->rfe_ctl);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	pdata->rfe_ctl |= RFE_CTL_BCAST_EN_ | RFE_CTL_DA_PERFECT_;

	ret = lan78xx_write_reg(dev, RFE_CTL, pdata->rfe_ctl);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	/* Enable or disable checksum offload engines */
	ret = lan78xx_set_features(dev->net, dev->net->features);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	lan78xx_set_multicast(dev->net);

	/* reset PHY */
	ret = lan78xx_read_reg(dev, PMT_CTL, &buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	buf |= PMT_CTL_PHY_RST_;

	ret = lan78xx_write_reg(dev, PMT_CTL, buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	timeout = jiffies + HZ;
	do {
		mdelay(1);
		ret = lan78xx_read_reg(dev, PMT_CTL, &buf);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			return ret;

		if (time_after(jiffies, timeout)) {
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			netdev_warn(dev->net, "timeout waiting for PHY Reset");
			ret = -ETIMEDOUT;
			return ret;
		}
	} while ((buf & PMT_CTL_PHY_RST_) || !(buf & PMT_CTL_READY_));

	ret = lan78xx_read_reg(dev, MAC_CR, &buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	/* LAN7801 only has RGMII mode */
	if (dev->chipid == ID_REV_CHIP_ID_7801_)
		buf &= ~MAC_CR_GMII_EN_;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (dev->chipid == ID_REV_CHIP_ID_7800_) {
		ret = lan78xx_read_raw_eeprom(dev, 0, 1, &sig);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (!ret && sig != EEPROM_INDICATOR) {
			/* Implies there is no external eeprom. Set mac speed */
			netdev_info(dev->net, "No External EEPROM. Setting MAC Speed\n");
			buf |= MAC_CR_AUTO_DUPLEX_ | MAC_CR_AUTO_SPEED_;
		}
	}
	ret = lan78xx_write_reg(dev, MAC_CR, buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	ret = lan78xx_set_rx_max_frame_length(dev,
					      dev->net->mtu + VLAN_ETH_HLEN);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return ret;
}

static void lan78xx_init_stats(struct lan78xx_net *dev)
{
	u32 *p;
	int i;

	pr_debug(" SOFTING  %d ", __LINE__);
	/* initialize for stats update
	 * some counters are 20bits and some are 32bits
	 */
	p = (u32 *)&dev->stats.rollover_max;
	for (i = 0; i < (sizeof(dev->stats.rollover_max) / (sizeof(u32))); i++)
		p[i] = 0xFFFFF;

	pr_debug(" SOFTING  %d ", __LINE__);
	dev->stats.rollover_max.rx_unicast_byte_count = 0xFFFFFFFF;
	dev->stats.rollover_max.rx_broadcast_byte_count = 0xFFFFFFFF;
	dev->stats.rollover_max.rx_multicast_byte_count = 0xFFFFFFFF;
	dev->stats.rollover_max.eee_rx_lpi_transitions = 0xFFFFFFFF;
	dev->stats.rollover_max.eee_rx_lpi_time = 0xFFFFFFFF;
	dev->stats.rollover_max.tx_unicast_byte_count = 0xFFFFFFFF;
	dev->stats.rollover_max.tx_broadcast_byte_count = 0xFFFFFFFF;
	dev->stats.rollover_max.tx_multicast_byte_count = 0xFFFFFFFF;
	dev->stats.rollover_max.eee_tx_lpi_transitions = 0xFFFFFFFF;
	dev->stats.rollover_max.eee_tx_lpi_time = 0xFFFFFFFF;

	set_bit(EVENT_STAT_UPDATE, &dev->flags);
	pr_debug(" SOFTING  %d ", __LINE__);
}

static int lan78xx_open(struct net_device *net)
{
	struct lan78xx_net *dev = netdev_priv(net);
	int ret=0;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	netif_dbg(dev, ifup, dev->net, "open device");

	ret = usb_autopm_get_interface(dev->intf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	mutex_lock(&dev->dev_mutex);

	phy_start(net->phydev);

	/* Force initial link status check */
	lan78xx_defer_kevent(dev, EVENT_LINK_RESET);

	netif_dbg(dev, ifup, dev->net, "phy initialised successfully");

	/* for Link Check */
	if (dev->urb_intr) {
		ret = usb_submit_urb(dev->urb_intr, GFP_KERNEL);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0) {
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			netif_err(dev, ifup, dev->net,
				  "intr submit %d\n", ret);
			goto done;
		}
	}

	ret = lan78xx_flush_rx_fifo(dev);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		goto done;
	ret = lan78xx_flush_tx_fifo(dev);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		goto done;

	ret = lan78xx_start_tx_path(dev);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		goto done;
	ret = lan78xx_start_rx_path(dev);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		goto done;

	lan78xx_init_stats(dev);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	set_bit(EVENT_DEV_OPEN, &dev->flags);

	netif_start_queue(net);

	dev->link_on = false;

	lan78xx_defer_kevent(dev, EVENT_LINK_RESET);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
done:
	mutex_unlock(&dev->dev_mutex);

	usb_autopm_put_interface(dev->intf);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return ret;
}

static void lan78xx_terminate_urbs(struct lan78xx_net *dev)
{
	DECLARE_WAIT_QUEUE_HEAD_ONSTACK(unlink_wakeup);
	DECLARE_WAITQUEUE(wait, current);
	int temp;

	pr_debug(" SOFTING  %d ", __LINE__);
	/* ensure there are no more active urbs */
	add_wait_queue(&unlink_wakeup, &wait);
	set_current_state(TASK_UNINTERRUPTIBLE);
	dev->wait = &unlink_wakeup;
	temp = unlink_urbs(dev, &dev->txq) + unlink_urbs(dev, &dev->rxq);

	pr_debug(" SOFTING  %d ", __LINE__);
	/* maybe wait for deletions to finish. */
	while (!skb_queue_empty(&dev->rxq) ||
	       !skb_queue_empty(&dev->txq)) {
		pr_debug(" SOFTING  %d ", __LINE__);
		schedule_timeout(msecs_to_jiffies(UNLINK_TIMEOUT_MS));
		set_current_state(TASK_UNINTERRUPTIBLE);
		netif_dbg(dev, ifdown, dev->net,
			  "waited for %d urb completions", temp);
	}
	pr_debug(" SOFTING  %d ", __LINE__);
	set_current_state(TASK_RUNNING);
	dev->wait = NULL;
	remove_wait_queue(&unlink_wakeup, &wait);

	pr_debug(" SOFTING  %d ", __LINE__);
	while (!skb_queue_empty(&dev->done)) {
		struct skb_data *entry;
		struct sk_buff *skb;

	pr_debug(" SOFTING  %d ", __LINE__);
		skb = skb_dequeue(&dev->done);
		entry = (struct skb_data *)(skb->cb);
		usb_free_urb(entry->urb);
		dev_kfree_skb(skb);
	}
}

static int lan78xx_stop(struct net_device *net)
{
	struct lan78xx_net *dev = netdev_priv(net);

	pr_debug(" SOFTING  %d ", __LINE__);
	netif_dbg(dev, ifup, dev->net, "stop device");

	mutex_lock(&dev->dev_mutex);

	pr_debug(" SOFTING  %d ", __LINE__);
	if (timer_pending(&dev->stat_monitor))
		del_timer_sync(&dev->stat_monitor);

	clear_bit(EVENT_DEV_OPEN, &dev->flags);
	netif_stop_queue(net);
	tasklet_kill(&dev->bh);

	pr_debug(" SOFTING  %d ", __LINE__);
	lan78xx_terminate_urbs(dev);

	pr_debug(" SOFTING  %d ", __LINE__);
	netif_info(dev, ifdown, dev->net,
		   "stop stats: rx/tx %lu/%lu, errs %lu/%lu\n",
		   net->stats.rx_packets, net->stats.tx_packets,
		   net->stats.rx_errors, net->stats.tx_errors);

	pr_debug(" SOFTING  %d ", __LINE__);
	/* ignore errors that occur stopping the Tx and Rx data paths */
	lan78xx_stop_tx_path(dev);
	lan78xx_stop_rx_path(dev);

	pr_debug(" SOFTING  %d ", __LINE__);
	if (net->phydev) {
	pr_debug(" SOFTING  %d ", __LINE__);
		phy_stop(net->phydev);
	}

	pr_debug(" SOFTING  %d ", __LINE__);
	usb_kill_urb(dev->urb_intr);

	/* deferred work (task, timer, softirq) must also stop.
	 * can't flush_scheduled_work() until we drop rtnl (later),
	 * else workers could deadlock; so make workers a NOP.
	 */
	clear_bit(EVENT_TX_HALT, &dev->flags);
	clear_bit(EVENT_RX_HALT, &dev->flags);
	clear_bit(EVENT_LINK_RESET, &dev->flags);
	clear_bit(EVENT_STAT_UPDATE, &dev->flags);

	cancel_delayed_work_sync(&dev->wq);

	usb_autopm_put_interface(dev->intf);

	mutex_unlock(&dev->dev_mutex);

	pr_debug(" SOFTING  %d ", __LINE__);
	return 0;
}

static struct sk_buff *lan78xx_tx_prep(struct lan78xx_net *dev,
				       struct sk_buff *skb, gfp_t flags)
{
	u32 tx_cmd_a, tx_cmd_b;
	void *ptr;

	if (skb_cow_head(skb, TX_OVERHEAD)) {
		dev_kfree_skb_any(skb);
		return NULL;
	}

	if (skb_linearize(skb)) {
		dev_kfree_skb_any(skb);
		return NULL;
	}

	tx_cmd_a = (u32)(skb->len & TX_CMD_A_LEN_MASK_) | TX_CMD_A_FCS_;

	if (skb->ip_summed == CHECKSUM_PARTIAL)
		tx_cmd_a |= TX_CMD_A_IPE_ | TX_CMD_A_TPE_;

	tx_cmd_b = 0;
	if (skb_is_gso(skb)) {
		u16 mss = max(skb_shinfo(skb)->gso_size, TX_CMD_B_MSS_MIN_);

		tx_cmd_b = (mss << TX_CMD_B_MSS_SHIFT_) & TX_CMD_B_MSS_MASK_;

		tx_cmd_a |= TX_CMD_A_LSO_;
	}

	if (skb_vlan_tag_present(skb)) {
		tx_cmd_a |= TX_CMD_A_IVTG_;
		tx_cmd_b |= skb_vlan_tag_get(skb) & TX_CMD_B_VTAG_MASK_;
	}

	ptr = skb_push(skb, 8);
	put_unaligned_le32(tx_cmd_a, ptr);
	put_unaligned_le32(tx_cmd_b, ptr + 4);

	return skb;
}

static enum skb_state defer_bh(struct lan78xx_net *dev, struct sk_buff *skb,
			       struct sk_buff_head *list, enum skb_state state)
{
	unsigned long flags;
	enum skb_state old_state;
	struct skb_data *entry = (struct skb_data *)skb->cb;

	spin_lock_irqsave(&list->lock, flags);
	old_state = entry->state;
	entry->state = state;

	__skb_unlink(skb, list);
	spin_unlock(&list->lock);
	spin_lock(&dev->done.lock);

	__skb_queue_tail(&dev->done, skb);
	if (skb_queue_len(&dev->done) == 1)
		tasklet_schedule(&dev->bh);
	spin_unlock_irqrestore(&dev->done.lock, flags);

	return old_state;
}

static void tx_complete(struct urb *urb)
{
	struct sk_buff *skb = (struct sk_buff *)urb->context;
	struct skb_data *entry = (struct skb_data *)skb->cb;
	struct lan78xx_net *dev = entry->dev;

	if (urb->status == 0) {
		dev->net->stats.tx_packets += entry->num_of_packet;
		dev->net->stats.tx_bytes += entry->length;
	} else {
		dev->net->stats.tx_errors++;

		switch (urb->status) {
		case -EPIPE:
			lan78xx_defer_kevent(dev, EVENT_TX_HALT);
			break;

		/* software-driven interface shutdown */
		case -ECONNRESET:
		case -ESHUTDOWN:
			netif_dbg(dev, tx_err, dev->net,
				  "tx err interface gone %d\n",
				  entry->urb->status);
			break;

		case -EPROTO:
		case -ETIME:
		case -EILSEQ:
			netif_stop_queue(dev->net);
			netif_dbg(dev, tx_err, dev->net,
				  "tx err queue stopped %d\n",
				  entry->urb->status);
			break;
		default:
			netif_dbg(dev, tx_err, dev->net,
				  "unknown tx err %d\n",
				  entry->urb->status);
			break;
		}
	}

	usb_autopm_put_interface_async(dev->intf);

	defer_bh(dev, skb, &dev->txq, tx_done);
}

static void lan78xx_queue_skb(struct sk_buff_head *list,
			      struct sk_buff *newsk, enum skb_state state)
{
	struct skb_data *entry = (struct skb_data *)newsk->cb;

	__skb_queue_tail(list, newsk);
	entry->state = state;
}

static netdev_tx_t
lan78xx_start_xmit(struct sk_buff *skb, struct net_device *net)
{
	struct lan78xx_net *dev = netdev_priv(net);
	struct sk_buff *skb2 = NULL;

	if (test_bit(EVENT_DEV_ASLEEP, &dev->flags))
		schedule_delayed_work(&dev->wq, 0);

	if (skb) {
		skb_tx_timestamp(skb);
		skb2 = lan78xx_tx_prep(dev, skb, GFP_ATOMIC);
	}

	if (skb2) {
		skb_queue_tail(&dev->txq_pend, skb2);

		/* throttle TX patch at slower than SUPER SPEED USB */
		if ((dev->udev->speed < USB_SPEED_SUPER) &&
		    (skb_queue_len(&dev->txq_pend) > 10))
			netif_stop_queue(net);
	} else {
		netif_dbg(dev, tx_err, dev->net,
			  "lan78xx_tx_prep return NULL\n");
		dev->net->stats.tx_errors++;
		dev->net->stats.tx_dropped++;
	}

	tasklet_schedule(&dev->bh);

	return NETDEV_TX_OK;
}

static int lan78xx_bind(struct lan78xx_net *dev, struct usb_interface *intf)
{
	struct lan78xx_priv *pdata = NULL;
	int ret=0;
	int i;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	dev->data[0] = (unsigned long)kzalloc(sizeof(*pdata), GFP_KERNEL);

	pdata = (struct lan78xx_priv *)(dev->data[0]);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (!pdata) {
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
		netdev_warn(dev->net, "Unable to allocate lan78xx_priv");
		return -ENOMEM;
	}

	pdata->dev = dev;

	spin_lock_init(&pdata->rfe_ctl_lock);
	mutex_init(&pdata->dataport_mutex);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	INIT_WORK(&pdata->set_multicast, lan78xx_deferred_multicast_write);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	for (i = 0; i < DP_SEL_VHF_VLAN_LEN; i++)
		pdata->vlan_table[i] = 0;

	INIT_WORK(&pdata->set_vlan, lan78xx_deferred_vlan_write);

	dev->net->features = 0;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (DEFAULT_TX_CSUM_ENABLE)
		dev->net->features |= NETIF_F_HW_CSUM;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (DEFAULT_RX_CSUM_ENABLE)
		dev->net->features |= NETIF_F_RXCSUM;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (DEFAULT_TSO_CSUM_ENABLE)
		dev->net->features |= NETIF_F_TSO | NETIF_F_TSO6 | NETIF_F_SG;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (DEFAULT_VLAN_RX_OFFLOAD)
		dev->net->features |= NETIF_F_HW_VLAN_CTAG_RX;

	if (DEFAULT_VLAN_FILTER_ENABLE)
		dev->net->features |= NETIF_F_HW_VLAN_CTAG_FILTER;

	dev->net->hw_features = dev->net->features;

	ret = lan78xx_setup_irq_domain(dev);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0) {
		netdev_warn(dev->net,
			    "lan78xx_setup_irq_domain() failed : %d", ret);
		goto out1;
	}

	dev->net->hard_header_len += TX_OVERHEAD;
	dev->hard_mtu = dev->net->mtu + dev->net->hard_header_len;

	/* Init all registers */
	ret = lan78xx_reset(dev);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret) {
		netdev_warn(dev->net, "Registers INIT FAILED....");
		goto out2;
	}

	ret = lan78xx_mdio_init(dev);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret) {
		netdev_warn(dev->net, "MDIO INIT FAILED.....");
		goto out2;
	}

	dev->net->flags |= IFF_MULTICAST;

	pdata->wol = WAKE_MAGIC;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return ret;

out2:
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	lan78xx_remove_irq_domain(dev);

out1:
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	netdev_warn(dev->net, "Bind routine FAILED");
	cancel_work_sync(&pdata->set_multicast);
	cancel_work_sync(&pdata->set_vlan);
	kfree(pdata);
	return ret;
}

static void lan78xx_unbind(struct lan78xx_net *dev, struct usb_interface *intf)
{
	struct lan78xx_priv *pdata = (struct lan78xx_priv *)(dev->data[0]);

	pr_debug(" SOFTING  %d", __LINE__);
	lan78xx_remove_irq_domain(dev);

	lan78xx_remove_mdio(dev);

	if (pdata) {
		pr_debug(" SOFTING  %d", __LINE__);
		cancel_work_sync(&pdata->set_multicast);
		cancel_work_sync(&pdata->set_vlan);
		netif_dbg(dev, ifdown, dev->net, "free pdata");
		kfree(pdata);
		pdata = NULL;
		dev->data[0] = 0;
	}
}

static void lan78xx_rx_csum_offload(struct lan78xx_net *dev,
				    struct sk_buff *skb,
				    u32 rx_cmd_a, u32 rx_cmd_b)
{
	/* HW Checksum offload appears to be flawed if used when not stripping
	 * VLAN headers. Drop back to S/W checksums under these conditions.
	 */
	if (!(dev->net->features & NETIF_F_RXCSUM) ||
	    unlikely(rx_cmd_a & RX_CMD_A_ICSM_) ||
	    ((rx_cmd_a & RX_CMD_A_FVTG_) &&
	     !(dev->net->features & NETIF_F_HW_VLAN_CTAG_RX))) {
		skb->ip_summed = CHECKSUM_NONE;
	} else {
		skb->csum = ntohs((u16)(rx_cmd_b >> RX_CMD_B_CSUM_SHIFT_));
		skb->ip_summed = CHECKSUM_COMPLETE;
	}
}

static void lan78xx_rx_vlan_offload(struct lan78xx_net *dev,
				    struct sk_buff *skb,
				    u32 rx_cmd_a, u32 rx_cmd_b)
{
	if ((dev->net->features & NETIF_F_HW_VLAN_CTAG_RX) &&
	    (rx_cmd_a & RX_CMD_A_FVTG_))
		__vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q),
				       (rx_cmd_b & 0xffff));
}

static void lan78xx_skb_return(struct lan78xx_net *dev, struct sk_buff *skb)
{
	int status;

	dev->net->stats.rx_packets++;
	dev->net->stats.rx_bytes += skb->len;

	skb->protocol = eth_type_trans(skb, dev->net);

	netif_dbg(dev, rx_status, dev->net, "< rx, len %zu, type 0x%x\n",
		  skb->len + sizeof(struct ethhdr), skb->protocol);
	memset(skb->cb, 0, sizeof(struct skb_data));

	if (skb_defer_rx_timestamp(skb))
		return;

	status = netif_rx(skb);
	if (status != NET_RX_SUCCESS)
		netif_dbg(dev, rx_err, dev->net,
			  "netif_rx status %d\n", status);
}

static int lan78xx_rx(struct lan78xx_net *dev, struct sk_buff *skb)
{
	if (skb->len < dev->net->hard_header_len)
		return 0;

	while (skb->len > 0) {
		u32 rx_cmd_a, rx_cmd_b, align_count, size;
		u16 rx_cmd_c;
		struct sk_buff *skb2;
		unsigned char *packet;

		rx_cmd_a = get_unaligned_le32(skb->data);
		skb_pull(skb, sizeof(rx_cmd_a));

		rx_cmd_b = get_unaligned_le32(skb->data);
		skb_pull(skb, sizeof(rx_cmd_b));

		rx_cmd_c = get_unaligned_le16(skb->data);
		skb_pull(skb, sizeof(rx_cmd_c));

		packet = skb->data;

		/* get the packet length */
		size = (rx_cmd_a & RX_CMD_A_LEN_MASK_);
		align_count = (4 - ((size + RXW_PADDING) % 4)) % 4;

		if (unlikely(size > skb->len)) {
			netif_dbg(dev, rx_err, dev->net,
				  "size err rx_cmd_a=0x%08x\n",
				  rx_cmd_a);
			return 0;
		}

		if (unlikely(rx_cmd_a & RX_CMD_A_RED_)) {
			netif_dbg(dev, rx_err, dev->net,
				  "Error rx_cmd_a=0x%08x", rx_cmd_a);
		} else {
			/* last frame in this batch */
			if (skb->len == size) {
				if (unlikely(size < ETH_FCS_LEN)) {
					netif_dbg(dev, rx_err, dev->net,
						  "size err rx_cmd_a=0x%08x\n",
						  rx_cmd_a);
					return 0;
				}

				lan78xx_rx_csum_offload(dev, skb,
							rx_cmd_a, rx_cmd_b);
				lan78xx_rx_vlan_offload(dev, skb,
							rx_cmd_a, rx_cmd_b);

				skb_trim(skb, skb->len - 4); /* remove fcs */
				skb->truesize = size + sizeof(struct sk_buff);

				return 1;
			}

			skb2 = skb_clone(skb, GFP_ATOMIC);
			if (unlikely(!skb2)) {
				netdev_warn(dev->net, "Error allocating skb");
				return 0;
			}

			if (unlikely(size < ETH_FCS_LEN)) {
				netif_dbg(dev, rx_err, dev->net,
					  "size err rx_cmd_a=0x%08x\n",
					  rx_cmd_a);
				dev_kfree_skb_any(skb2);
				return 0;
			}

			skb2->len = size;
			skb2->data = packet;
			skb_set_tail_pointer(skb2, size);

			lan78xx_rx_csum_offload(dev, skb2, rx_cmd_a, rx_cmd_b);
			lan78xx_rx_vlan_offload(dev, skb2, rx_cmd_a, rx_cmd_b);

			skb_trim(skb2, skb2->len - 4); /* remove fcs */
			skb2->truesize = size + sizeof(struct sk_buff);

			lan78xx_skb_return(dev, skb2);
		}

		skb_pull(skb, size);

		/* padding bytes before the next frame starts */
		if (skb->len)
			skb_pull(skb, align_count);
	}

	return 1;
}

static inline void rx_process(struct lan78xx_net *dev, struct sk_buff *skb)
{
	if (!lan78xx_rx(dev, skb)) {
		dev->net->stats.rx_errors++;
		goto done;
	}

	if (skb->len) {
		lan78xx_skb_return(dev, skb);
		return;
	}

	netif_dbg(dev, rx_err, dev->net, "drop\n");
	dev->net->stats.rx_errors++;
done:
	skb_queue_tail(&dev->done, skb);
}

static void rx_complete(struct urb *urb);

static int rx_submit(struct lan78xx_net *dev, struct urb *urb, gfp_t flags)
{
	struct sk_buff *skb;
	struct skb_data *entry;
	unsigned long lockflags;
	size_t size = dev->rx_urb_size;
	int ret = 0;

	skb = netdev_alloc_skb_ip_align(dev->net, size);
	if (!skb) {
		usb_free_urb(urb);
		return -ENOMEM;
	}

	entry = (struct skb_data *)skb->cb;
	entry->urb = urb;
	entry->dev = dev;
	entry->length = 0;

	usb_fill_bulk_urb(urb, dev->udev, dev->pipe_in,
			  skb->data, size, rx_complete, skb);

	spin_lock_irqsave(&dev->rxq.lock, lockflags);

	if (netif_device_present(dev->net) &&
	    netif_running(dev->net) &&
	    !test_bit(EVENT_RX_HALT, &dev->flags) &&
	    !test_bit(EVENT_DEV_ASLEEP, &dev->flags)) {
		ret = usb_submit_urb(urb, GFP_ATOMIC);
		switch (ret) {
		case 0:
			lan78xx_queue_skb(&dev->rxq, skb, rx_start);
			break;
		case -EPIPE:
			lan78xx_defer_kevent(dev, EVENT_RX_HALT);
			break;
		case -ENODEV:
		case -ENOENT:
			netif_dbg(dev, ifdown, dev->net, "device gone\n");
			netif_device_detach(dev->net);
			break;
		case -EHOSTUNREACH:
			ret = -ENOLINK;
			break;
		default:
			netif_dbg(dev, rx_err, dev->net,
				  "rx submit, %d\n", ret);
			tasklet_schedule(&dev->bh);
		}
	} else {
		netif_dbg(dev, ifdown, dev->net, "rx: stopped\n");
		ret = -ENOLINK;
	}
	spin_unlock_irqrestore(&dev->rxq.lock, lockflags);
	if (ret) {
		dev_kfree_skb_any(skb);
		usb_free_urb(urb);
	}
	return ret;
}

static void rx_complete(struct urb *urb)
{
	struct sk_buff	*skb = (struct sk_buff *)urb->context;
	struct skb_data	*entry = (struct skb_data *)skb->cb;
	struct lan78xx_net *dev = entry->dev;
	int urb_status = urb->status;
	enum skb_state state;

	skb_put(skb, urb->actual_length);
	state = rx_done;
	entry->urb = NULL;

	switch (urb_status) {
	case 0:
		if (skb->len < dev->net->hard_header_len) {
			state = rx_cleanup;
			dev->net->stats.rx_errors++;
			dev->net->stats.rx_length_errors++;
			netif_dbg(dev, rx_err, dev->net,
				  "rx length %d\n", skb->len);
		}
		usb_mark_last_busy(dev->udev);
		break;
	case -EPIPE:
		dev->net->stats.rx_errors++;
		lan78xx_defer_kevent(dev, EVENT_RX_HALT);
		fallthrough;
	case -ECONNRESET:				/* async unlink */
	case -ESHUTDOWN:				/* hardware gone */
		netif_dbg(dev, ifdown, dev->net,
			  "rx shutdown, code %d\n", urb_status);
		state = rx_cleanup;
		entry->urb = urb;
		urb = NULL;
		break;
	case -EPROTO:
	case -ETIME:
	case -EILSEQ:
		dev->net->stats.rx_errors++;
		state = rx_cleanup;
		entry->urb = urb;
		urb = NULL;
		break;

	/* data overrun ... flush fifo? */
	case -EOVERFLOW:
		dev->net->stats.rx_over_errors++;
		fallthrough;

	default:
		state = rx_cleanup;
		dev->net->stats.rx_errors++;
		netif_dbg(dev, rx_err, dev->net, "rx status %d\n", urb_status);
		break;
	}

	state = defer_bh(dev, skb, &dev->rxq, state);

	if (urb) {
		if (netif_running(dev->net) &&
		    !test_bit(EVENT_RX_HALT, &dev->flags) &&
		    state != unlink_start) {
			rx_submit(dev, urb, GFP_ATOMIC);
			return;
		}
		usb_free_urb(urb);
	}
	netif_dbg(dev, rx_err, dev->net, "no read resubmitted\n");
}

static void lan78xx_tx_bh(struct lan78xx_net *dev)
{
	int length;
	struct urb *urb = NULL;
	struct skb_data *entry;
	unsigned long flags;
	struct sk_buff_head *tqp = &dev->txq_pend;
	struct sk_buff *skb, *skb2;
	int ret=0;
	int count, pos;
	int skb_totallen, pkt_cnt;

	skb_totallen = 0;
	pkt_cnt = 0;
	count = 0;
	length = 0;
	spin_lock_irqsave(&tqp->lock, flags);
	skb_queue_walk(tqp, skb) {
		if (skb_is_gso(skb)) {
			if (!skb_queue_is_first(tqp, skb)) {
				/* handle previous packets first */
				break;
			}
			count = 1;
			length = skb->len - TX_OVERHEAD;
			__skb_unlink(skb, tqp);
			spin_unlock_irqrestore(&tqp->lock, flags);
			goto gso_skb;
		}

		if ((skb_totallen + skb->len) > MAX_SINGLE_PACKET_SIZE)
			break;
		skb_totallen = skb->len + roundup(skb_totallen, sizeof(u32));
		pkt_cnt++;
	}
	spin_unlock_irqrestore(&tqp->lock, flags);

	/* copy to a single skb */
	skb = alloc_skb(skb_totallen, GFP_ATOMIC);
	if (!skb)
		goto drop;

	skb_put(skb, skb_totallen);

	for (count = pos = 0; count < pkt_cnt; count++) {
		skb2 = skb_dequeue(tqp);
		if (skb2) {
			length += (skb2->len - TX_OVERHEAD);
			memcpy(skb->data + pos, skb2->data, skb2->len);
			pos += roundup(skb2->len, sizeof(u32));
			dev_kfree_skb(skb2);
		}
	}

gso_skb:
	urb = usb_alloc_urb(0, GFP_ATOMIC);
	if (!urb)
		goto drop;

	entry = (struct skb_data *)skb->cb;
	entry->urb = urb;
	entry->dev = dev;
	entry->length = length;
	entry->num_of_packet = count;

	spin_lock_irqsave(&dev->txq.lock, flags);
	ret = usb_autopm_get_interface_async(dev->intf);
	if (ret < 0) {
		spin_unlock_irqrestore(&dev->txq.lock, flags);
		goto drop;
	}

	usb_fill_bulk_urb(urb, dev->udev, dev->pipe_out,
			  skb->data, skb->len, tx_complete, skb);

	if (length % dev->maxpacket == 0) {
		/* send USB_ZERO_PACKET */
		urb->transfer_flags |= URB_ZERO_PACKET;
	}

#ifdef CONFIG_PM
	/* if this triggers the device is still a sleep */
	if (test_bit(EVENT_DEV_ASLEEP, &dev->flags)) {
		/* transmission will be done in resume */
		usb_anchor_urb(urb, &dev->deferred);
		/* no use to process more packets */
		netif_stop_queue(dev->net);
		usb_put_urb(urb);
		spin_unlock_irqrestore(&dev->txq.lock, flags);
		netdev_dbg(dev->net, "Delaying transmission for resumption\n");
		return;
	}
#endif

	ret = usb_submit_urb(urb, GFP_ATOMIC);
	switch (ret) {
	case 0:
		netif_trans_update(dev->net);
		lan78xx_queue_skb(&dev->txq, skb, tx_start);
		if (skb_queue_len(&dev->txq) >= dev->tx_qlen)
			netif_stop_queue(dev->net);
		break;
	case -EPIPE:
		netif_stop_queue(dev->net);
		lan78xx_defer_kevent(dev, EVENT_TX_HALT);
		usb_autopm_put_interface_async(dev->intf);
		break;
	case -ENODEV:
	case -ENOENT:
		netif_dbg(dev, tx_err, dev->net,
			  "tx: submit urb err %d (disconnected?)", ret);
		netif_device_detach(dev->net);
		break;
	default:
		usb_autopm_put_interface_async(dev->intf);
		netif_dbg(dev, tx_err, dev->net,
			  "tx: submit urb err %d\n", ret);
		break;
	}

	spin_unlock_irqrestore(&dev->txq.lock, flags);

	if (ret) {
		netif_dbg(dev, tx_err, dev->net, "drop, code %d\n", ret);
drop:
		dev->net->stats.tx_dropped++;
		if (skb)
			dev_kfree_skb_any(skb);
		usb_free_urb(urb);
	} else {
		netif_dbg(dev, tx_queued, dev->net,
			  "> tx, len %d, type 0x%x\n", length, skb->protocol);
	}
}

static void lan78xx_rx_bh(struct lan78xx_net *dev)
{
	struct urb *urb;
	int i;

	if (skb_queue_len(&dev->rxq) < dev->rx_qlen) {
		for (i = 0; i < 10; i++) {
			if (skb_queue_len(&dev->rxq) >= dev->rx_qlen)
				break;
			urb = usb_alloc_urb(0, GFP_ATOMIC);
			if (urb)
				if (rx_submit(dev, urb, GFP_ATOMIC) == -ENOLINK)
					return;
		}

		if (skb_queue_len(&dev->rxq) < dev->rx_qlen)
			tasklet_schedule(&dev->bh);
	}
	if (skb_queue_len(&dev->txq) < dev->tx_qlen)
		netif_wake_queue(dev->net);
}

static void lan78xx_bh(struct tasklet_struct *t)
{
	struct lan78xx_net *dev = from_tasklet(dev, t, bh);
	struct sk_buff *skb;
	struct skb_data *entry;

	while ((skb = skb_dequeue(&dev->done))) {
		entry = (struct skb_data *)(skb->cb);
		switch (entry->state) {
		case rx_done:
			entry->state = rx_cleanup;
			rx_process(dev, skb);
			continue;
		case tx_done:
			usb_free_urb(entry->urb);
			dev_kfree_skb(skb);
			continue;
		case rx_cleanup:
			usb_free_urb(entry->urb);
			dev_kfree_skb(skb);
			continue;
		default:
			netdev_dbg(dev->net, "skb state %d\n", entry->state);
			return;
		}
	}

	if (netif_device_present(dev->net) && netif_running(dev->net)) {
		/* reset update timer delta */
		if (timer_pending(&dev->stat_monitor) && (dev->delta != 1)) {
			dev->delta = 1;
			mod_timer(&dev->stat_monitor,
				  jiffies + STAT_UPDATE_TIMER);
		}

		if (!skb_queue_empty(&dev->txq_pend))
			lan78xx_tx_bh(dev);

		if (!test_bit(EVENT_RX_HALT, &dev->flags))
			lan78xx_rx_bh(dev);
	}
}

static void lan78xx_delayedwork(struct work_struct *work)
{
	int status;
	struct lan78xx_net *dev;
	int pm_ret;

	pr_debug(" SOFTING  %d", __LINE__);
	dev = container_of(work, struct lan78xx_net, wq.work);

	if (test_bit(EVENT_DEV_DISCONNECT, &dev->flags))
		return;

	pm_ret = usb_autopm_get_interface(dev->intf);
	if (pm_ret < 0) {
		/* USB interface is not ready (suspended/autosuspend).
		 * For critical events like LINK_RESET, retry after a delay
		 * instead of silently dropping the event.
		 */
		if (test_bit(EVENT_LINK_RESET, &dev->flags)) {
			if (net_ratelimit()) {
				netdev_warn(dev->net,
					   "USB PM get failed (%d) for link check, retrying after 100ms\n",
					   pm_ret);
			}
			/* Retry after 100ms to allow USB to wake up */
			/* Ensure the flag stays set so we retry */
			if (!schedule_delayed_work(&dev->wq, msecs_to_jiffies(100))) {
				/* Work already queued, also ensure stat monitor will retry */
				if (!timer_pending(&dev->stat_monitor)) {
					mod_timer(&dev->stat_monitor, jiffies + msecs_to_jiffies(500));
				}
			}
		}
		return;
	}

	if (test_bit(EVENT_TX_HALT, &dev->flags)) {
		unlink_urbs(dev, &dev->txq);

		status = usb_clear_halt(dev->udev, dev->pipe_out);
		if (status < 0 &&
		    status != -EPIPE &&
		    status != -ESHUTDOWN) {
			pr_debug(" SOFTING  %d", __LINE__);
			if (netif_msg_tx_err(dev))
				netdev_err(dev->net,
					   "can't clear tx halt, status %d\n",
					   status);
		} else {
			clear_bit(EVENT_TX_HALT, &dev->flags);
			pr_debug(" SOFTING  %d", __LINE__);
			if (status != -ESHUTDOWN)
				netif_wake_queue(dev->net);
		}
	}

	pr_debug(" SOFTING  %d", __LINE__);
	if (test_bit(EVENT_RX_HALT, &dev->flags)) {
		pr_debug(" SOFTING  %d", __LINE__);
		unlink_urbs(dev, &dev->rxq);
		status = usb_clear_halt(dev->udev, dev->pipe_in);
		if (status < 0 &&
		    status != -EPIPE &&
		    status != -ESHUTDOWN) {
			pr_debug(" SOFTING  %d", __LINE__);
			if (netif_msg_rx_err(dev))
				netdev_err(dev->net,
					   "can't clear rx halt, status %d\n",
					   status);
			pr_debug(" SOFTING  %d", __LINE__);
		} else {
			pr_debug(" SOFTING  %d", __LINE__);
			clear_bit(EVENT_RX_HALT, &dev->flags);
			/* Restart RX processing after successful halt clear */
			if (netif_device_present(dev->net) && netif_running(dev->net))
				tasklet_schedule(&dev->bh);
		}
	}

	pr_debug(" SOFTING  %d", __LINE__);
	if (test_bit(EVENT_LINK_RESET, &dev->flags)) {
		int ret = 0;

		clear_bit(EVENT_LINK_RESET, &dev->flags);
		if (lan78xx_link_reset(dev) < 0) {
			pr_debug(" SOFTING  %d", __LINE__);
			netdev_info(dev->net, "link reset failed (%d)\n",
				    ret);
		}
		/* Release the async PM reference we took in lan78xx_defer_kevent */
		usb_autopm_put_interface_async(dev->intf);
	}

	if (test_bit(EVENT_STAT_UPDATE, &dev->flags)) {
		pr_debug(" SOFTING  %d", __LINE__);
		lan78xx_update_stats(dev);

		clear_bit(EVENT_STAT_UPDATE, &dev->flags);

		mod_timer(&dev->stat_monitor,
			  jiffies + (STAT_UPDATE_TIMER * dev->delta));

		dev->delta = min((dev->delta * 2), 50);
	}

	pr_debug(" SOFTING  %d", __LINE__);
	usb_autopm_put_interface(dev->intf);
}

static void intr_complete(struct urb *urb)
{
	struct lan78xx_net *dev = urb->context;
	int status = urb->status;

	pr_debug(" SOFTING  %d", __LINE__);
	switch (status) {
	/* success */
	case 0:
		pr_debug(" SOFTING  %d", __LINE__);
		lan78xx_status(dev, urb);
		break;

	/* software-driven interface shutdown */
	case -ENOENT:			/* urb killed */
	case -ENODEV:			/* hardware gone */
	case -ESHUTDOWN:		/* hardware gone */
		pr_debug(" SOFTING  %d", __LINE__);
		netif_dbg(dev, ifdown, dev->net,
			  "intr shutdown, code %d\n", status);
		return;

	/* NOTE:  not throttling like RX/TX, since this endpoint
	 * already polls infrequently
	 */
	default:
		pr_debug(" SOFTING  %d", __LINE__);
		netdev_dbg(dev->net, "intr status %d\n", status);
		break;
	}

	if (!netif_device_present(dev->net) ||
	    !netif_running(dev->net)) {
		pr_debug(" SOFTING  %d", __LINE__);
		netdev_warn(dev->net, "not submitting new status URB");
		return;
	}

	memset(urb->transfer_buffer, 0, urb->transfer_buffer_length);
	status = usb_submit_urb(urb, GFP_ATOMIC);

	switch (status) {
	case  0:
		pr_debug(" SOFTING  %d", __LINE__);
		break;
	case -ENODEV:
	case -ENOENT:
		pr_debug(" SOFTING  %d", __LINE__);
		netif_dbg(dev, timer, dev->net,
			  "intr resubmit %d (disconnect?)", status);
		netif_device_detach(dev->net);
		break;
	default:
		pr_debug(" SOFTING  %d", __LINE__);
		netif_err(dev, timer, dev->net,
			  "intr resubmit --> %d\n", status);
		break;
	}
}

static void lan78xx_disconnect(struct usb_interface *intf)
{
	struct lan78xx_net *dev;
	struct usb_device *udev;
	struct net_device *net;
	struct phy_device *phydev;

	pr_debug(" SOFTING  %d", __LINE__);
	dev = usb_get_intfdata(intf);
	usb_set_intfdata(intf, NULL);
	if (!dev)
		return;

	pr_debug(" SOFTING  %d", __LINE__);
	set_bit(EVENT_DEV_DISCONNECT, &dev->flags);

	udev = interface_to_usbdev(intf);
	net = dev->net;

	unregister_netdev(net);

	/* Ensure proper cleanup order */
	if (timer_pending(&dev->stat_monitor))
		del_timer_sync(&dev->stat_monitor);
	set_bit(EVENT_DEV_DISCONNECT, &dev->flags);
	cancel_delayed_work_sync(&dev->wq);
	
	/* Stop PHY to prevent further link events */
	if (net->phydev)
		phy_stop(net->phydev);

	phydev = net->phydev;

	phy_unregister_fixup_for_uid(PHY_KSZ9031RNX, 0xfffffff0);
	phy_unregister_fixup_for_uid(PHY_LAN8835, 0xfffffff0);

	phy_disconnect(net->phydev);

	if (phy_is_pseudo_fixed_link(phydev)) {
		fixed_phy_unregister(phydev);
		phy_device_free(phydev);
	}

	usb_scuttle_anchored_urbs(&dev->deferred);

	if (timer_pending(&dev->stat_monitor))
		del_timer_sync(&dev->stat_monitor);

	lan78xx_unbind(dev, intf);

	pr_debug(" SOFTING  %d", __LINE__);
	usb_kill_urb(dev->urb_intr);
	usb_free_urb(dev->urb_intr);

	free_netdev(net);
	usb_put_dev(udev);
	pr_debug(" SOFTING  %d", __LINE__);
}

static void lan78xx_tx_timeout(struct net_device *net, unsigned int txqueue)
{
	struct lan78xx_net *dev = netdev_priv(net);

	unlink_urbs(dev, &dev->txq);
	tasklet_schedule(&dev->bh);
}

static netdev_features_t lan78xx_features_check(struct sk_buff *skb,
						struct net_device *netdev,
						netdev_features_t features)
{
	pr_debug(" SOFTING  %d", __LINE__);
	if (skb->len + TX_OVERHEAD > MAX_SINGLE_PACKET_SIZE)
		features &= ~NETIF_F_GSO_MASK;

	features = vlan_features_check(skb, features);
	features = vxlan_features_check(skb, features);

	return features;
}

static const struct net_device_ops lan78xx_netdev_ops = {
	.ndo_open		= lan78xx_open,
	.ndo_stop		= lan78xx_stop,
	.ndo_start_xmit		= lan78xx_start_xmit,
	.ndo_tx_timeout		= lan78xx_tx_timeout,
	.ndo_change_mtu		= lan78xx_change_mtu,
	.ndo_set_mac_address	= lan78xx_set_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_do_ioctl		= phy_do_ioctl_running,
	.ndo_set_rx_mode	= lan78xx_set_multicast,
	.ndo_set_features	= lan78xx_set_features,
	.ndo_vlan_rx_add_vid	= lan78xx_vlan_rx_add_vid,
	.ndo_vlan_rx_kill_vid	= lan78xx_vlan_rx_kill_vid,
	.ndo_features_check	= lan78xx_features_check,
};

static void lan78xx_stat_monitor(struct timer_list *t)
{
	struct lan78xx_net *dev = from_timer(dev, t, stat_monitor);
	
	pr_debug(" SOFTING  %d", __LINE__);

	lan78xx_defer_kevent(dev, EVENT_STAT_UPDATE);
	
	/* Also trigger link check to recover from NO-CARRIER state */
	if (!dev->link_on) {
		if (net_ratelimit()) {
			netdev_dbg(dev->net, "Stat monitor: link down, triggering link check\n");
		}
		lan78xx_defer_kevent(dev, EVENT_LINK_RESET);
		/* Ensure stat monitor will fire again if link doesn't come up */
		if (!timer_pending(&dev->stat_monitor)) {
			dev->delta = 1;
			mod_timer(&dev->stat_monitor, jiffies + STAT_UPDATE_TIMER);
		}
	}
}

static int lan78xx_probe(struct usb_interface *intf,
			 const struct usb_device_id *id)
{
	struct usb_host_endpoint *ep_blkin, *ep_blkout, *ep_intr;
	struct lan78xx_net *dev;
	struct net_device *netdev;
	struct usb_device *udev;
	int ret;
	unsigned int maxp;
	unsigned int period;
	u8 *buf = NULL;

	pr_debug(" SOFTING  %d", __LINE__);
	udev = interface_to_usbdev(intf);
	udev = usb_get_dev(udev);

	netdev = alloc_etherdev(sizeof(struct lan78xx_net));
	if (!netdev) {
		pr_debug(" SOFTING  %d", __LINE__);
		dev_err(&intf->dev, "Error: OOM\n");
		ret = -ENOMEM;
		goto out1;
	}

	/* netdev_printk() needs this */
	SET_NETDEV_DEV(netdev, &intf->dev);

	dev = netdev_priv(netdev);
	dev->udev = udev;
	dev->intf = intf;
	dev->net = netdev;
	dev->msg_enable = netif_msg_init(msg_level, NETIF_MSG_DRV
					| NETIF_MSG_PROBE | NETIF_MSG_LINK);

	skb_queue_head_init(&dev->rxq);
	skb_queue_head_init(&dev->txq);
	skb_queue_head_init(&dev->done);
	skb_queue_head_init(&dev->txq_pend);
	mutex_init(&dev->mdiobus_mutex);
	mutex_init(&dev->dev_mutex);
	mutex_init(&dev->ctrl_mutex);

	tasklet_setup(&dev->bh, lan78xx_bh);
	INIT_DELAYED_WORK(&dev->wq, lan78xx_delayedwork);
	init_usb_anchor(&dev->deferred);

	netdev->netdev_ops = &lan78xx_netdev_ops;
	netdev->watchdog_timeo = TX_TIMEOUT_JIFFIES;
	netdev->ethtool_ops = &lan78xx_ethtool_ops;

	dev->delta = 1;
	timer_setup(&dev->stat_monitor, lan78xx_stat_monitor, 0);

	mutex_init(&dev->stats.access_lock);

	if (intf->cur_altsetting->desc.bNumEndpoints < 3) {
		pr_debug(" SOFTING  %d", __LINE__);
		ret = -ENODEV;
		goto out2;
	}

	dev->pipe_in = usb_rcvbulkpipe(udev, BULK_IN_PIPE);
	ep_blkin = usb_pipe_endpoint(udev, dev->pipe_in);
	if (!ep_blkin || !usb_endpoint_is_bulk_in(&ep_blkin->desc)) {
		pr_debug(" SOFTING  %d", __LINE__);
		ret = -ENODEV;
		goto out2;
	}

	dev->pipe_out = usb_sndbulkpipe(udev, BULK_OUT_PIPE);
	ep_blkout = usb_pipe_endpoint(udev, dev->pipe_out);
	if (!ep_blkout || !usb_endpoint_is_bulk_out(&ep_blkout->desc)) {
		pr_debug(" SOFTING  %d", __LINE__);
		ret = -ENODEV;
		goto out2;
	}

	ep_intr = &intf->cur_altsetting->endpoint[2];
	if (!usb_endpoint_is_int_in(&ep_intr->desc)) {
		pr_debug(" SOFTING  %d", __LINE__);
		ret = -ENODEV;
		goto out2;
	}

	dev->pipe_intr = usb_rcvintpipe(dev->udev,
					usb_endpoint_num(&ep_intr->desc));

	ret = lan78xx_bind(dev, intf);
	if (ret < 0) {
		pr_debug(" SOFTING  %d", __LINE__);
		goto out2;
	}

	if (netdev->mtu > (dev->hard_mtu - netdev->hard_header_len))
		netdev->mtu = dev->hard_mtu - netdev->hard_header_len;

	/* MTU range: 68 - 9000 */
	netdev->max_mtu = MAX_SINGLE_PACKET_SIZE;
	netif_set_gso_max_size(netdev, MAX_SINGLE_PACKET_SIZE - MAX_HEADER);

	period = ep_intr->desc.bInterval;
	maxp = usb_maxpacket(dev->udev, dev->pipe_intr, 0);
	buf = kmalloc(maxp, GFP_KERNEL);
	if (buf) {
		pr_debug(" SOFTING  %d", __LINE__);
		dev->urb_intr = usb_alloc_urb(0, GFP_KERNEL);
		if (!dev->urb_intr) {
			pr_debug(" SOFTING  %d", __LINE__);
			ret = -ENOMEM;
			kfree(buf);
			goto out3;
		} else {
			pr_debug(" SOFTING  %d", __LINE__);
			usb_fill_int_urb(dev->urb_intr, dev->udev,
					 dev->pipe_intr, buf, maxp,
					 intr_complete, dev, period);
			dev->urb_intr->transfer_flags |= URB_FREE_BUFFER;
		}
	}

	dev->maxpacket = usb_maxpacket(dev->udev, dev->pipe_out, 1);

	/* Reject broken descriptors. */
	if (dev->maxpacket == 0) {
		pr_debug(" SOFTING  %d", __LINE__);
		ret = -ENODEV;
		goto out4;
	}

	/* driver requires remote-wakeup capability during autosuspend. */
	intf->needs_remote_wakeup = 1;
	
	/* Enable autosuspend for better power management */
	usb_enable_autosuspend(udev);

	ret = lan78xx_phy_init(dev);
	if (ret < 0) {
		pr_debug(" SOFTING  %d", __LINE__);
		goto out4;
	}

	/* Initialize carrier state - start with carrier off until link is detected */
	netif_carrier_off(netdev);

	/* Set interface name based on USB port number for Android/LXC container usage */
	/* Port 5 (2-1.5) -> eth_obd, Port 4 (2-1.4) -> eth_lan, Port 3 (2-1.3) -> eth_brr */
	if (udev->portnum == 5) {
		strscpy(netdev->name, "eth_obd", IFNAMSIZ);
		netif_info(dev, probe, netdev, "Setting interface name to eth_obd (USB port %u)\n",
			   udev->portnum);
	} else if (udev->portnum == 4) {
		strscpy(netdev->name, "eth_lan", IFNAMSIZ);
		netif_info(dev, probe, netdev, "Setting interface name to eth_lan (USB port %u)\n",
			   udev->portnum);
	} else if (udev->portnum == 3) {
		strscpy(netdev->name, "eth_brr", IFNAMSIZ);
		netif_info(dev, probe, netdev, "Setting interface name to eth_brr (USB port %u)\n",
			   udev->portnum);
	} else {
		netif_info(dev, probe, netdev, "Using default interface name (USB port %u)\n",
			   udev->portnum);
	}

	ret = register_netdev(netdev);
	if (ret != 0) {
		pr_debug(" SOFTING  %d", __LINE__);
		netif_err(dev, probe, netdev, "couldn't register the device\n");
		goto out5;
	}

	usb_set_intfdata(intf, dev);

	/* Configure eth_obd (port 5) to 100Mbps, full duplex, autoneg on */
	if (udev->portnum == 5 && netdev->phydev) {
		struct ethtool_link_ksettings ecmd;
		int link_ret;

		/* Get current settings */
		phy_ethtool_ksettings_get(netdev->phydev, &ecmd);
		/* Set to 100Mbps, full duplex, autoneg on */
		ecmd.base.speed = SPEED_100;
		ecmd.base.duplex = DUPLEX_FULL;
		ecmd.base.autoneg = AUTONEG_ENABLE;
		/* Clear all advertising bits, then set only 100Mbps full duplex */
		linkmode_zero(ecmd.link_modes.advertising);
		linkmode_set_bit(ETHTOOL_LINK_MODE_100baseT_Full_BIT,
				 ecmd.link_modes.advertising);
		linkmode_set_bit(ETHTOOL_LINK_MODE_Autoneg_BIT,
				 ecmd.link_modes.advertising);

		link_ret = phy_ethtool_ksettings_set(netdev->phydev, &ecmd);
		if (link_ret == 0) {
			netif_info(dev, probe, netdev,
				   "Configured eth_obd: 100Mbps, full duplex, autoneg on\n");
		} else {
			netif_warn(dev, probe, netdev,
				   "Failed to configure link settings: %d\n", link_ret);
		}
	}

	ret = device_set_wakeup_enable(&udev->dev, true);

	 /* Default delay of 2sec has more overhead than advantage.
	  * Set to 10sec as default.
	  */
	pm_runtime_set_autosuspend_delay(&udev->dev,
					 DEFAULT_AUTOSUSPEND_DELAY);

	/* During boot, the interface might not be opened immediately.
	 * Start stat monitor timer to ensure link checks happen even if
	 * interface isn't opened yet. This helps recover from boot-time
	 * initialization race conditions where interface stays DOWN.
	 */
	if (!timer_pending(&dev->stat_monitor)) {
		dev->delta = 1;
		/* Start first check after 2 seconds to allow USB/PHY to stabilize */
		mod_timer(&dev->stat_monitor, jiffies + msecs_to_jiffies(2000));
		if (net_ratelimit()) {
			netdev_info(netdev,
				    "Device registered, starting initial link check in 2s\n");
		}
	}

	pr_debug(" SOFTING  %d", __LINE__);
	return 0;

out5:
	pr_debug(" SOFTING  %d", __LINE__);
	phy_disconnect(netdev->phydev);
out4:
	pr_debug(" SOFTING  %d", __LINE__);
	usb_free_urb(dev->urb_intr);
out3:
	pr_debug(" SOFTING  %d", __LINE__);
	lan78xx_unbind(dev, intf);
out2:
	pr_debug(" SOFTING  %d", __LINE__);
	free_netdev(netdev);
out1:
	pr_debug(" SOFTING  %d", __LINE__);
	usb_put_dev(udev);

	pr_debug(" SOFTING  %d", __LINE__);
	return ret;
}

static u16 lan78xx_wakeframe_crc16(const u8 *buf, int len)
{
	const u16 crc16poly = 0x8005;
	int i;
	u16 bit, crc, msb;
	u8 data;

	crc = 0xFFFF;
	for (i = 0; i < len; i++) {
		data = *buf++;
		for (bit = 0; bit < 8; bit++) {
			msb = crc >> 15;
			crc <<= 1;

			if (msb ^ (u16)(data & 1)) {
				crc ^= crc16poly;
				crc |= (u16)0x0001U;
			}
			data >>= 1;
		}
	}

	return crc;
}

static int lan78xx_set_auto_suspend(struct lan78xx_net *dev)
{
	u32 buf;
	int ret;

	ret = lan78xx_stop_tx_path(dev);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	ret = lan78xx_stop_rx_path(dev);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	/* auto suspend (selective suspend) */

	ret = lan78xx_write_reg(dev, WUCSR, 0);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;
	ret = lan78xx_write_reg(dev, WUCSR2, 0);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;
	ret = lan78xx_write_reg(dev, WK_SRC, 0xFFF1FF1FUL);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	/* set goodframe wakeup */

	ret = lan78xx_read_reg(dev, WUCSR, &buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	buf |= WUCSR_RFE_WAKE_EN_;
	buf |= WUCSR_STORE_WAKE_;

	ret = lan78xx_write_reg(dev, WUCSR, buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	ret = lan78xx_read_reg(dev, PMT_CTL, &buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	buf &= ~PMT_CTL_RES_CLR_WKP_EN_;
	buf |= PMT_CTL_RES_CLR_WKP_STS_;
	buf |= PMT_CTL_PHY_WAKE_EN_;
	buf |= PMT_CTL_WOL_EN_;
	buf &= ~PMT_CTL_SUS_MODE_MASK_;
	buf |= PMT_CTL_SUS_MODE_3_;

	ret = lan78xx_write_reg(dev, PMT_CTL, buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	ret = lan78xx_read_reg(dev, PMT_CTL, &buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	buf |= PMT_CTL_WUPS_MASK_;

	ret = lan78xx_write_reg(dev, PMT_CTL, buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	ret = lan78xx_start_rx_path(dev);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);

	return ret;
}

static int lan78xx_set_suspend(struct lan78xx_net *dev, u32 wol)
{
	const u8 ipv4_multicast[3] = { 0x01, 0x00, 0x5E };
	const u8 ipv6_multicast[3] = { 0x33, 0x33 };
	const u8 arp_type[2] = { 0x08, 0x06 };
	u32 temp_pmt_ctl;
	int mask_index;
	u32 temp_wucsr;
	u32 buf;
	u16 crc;
	int ret;

	ret = lan78xx_stop_tx_path(dev);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;
	ret = lan78xx_stop_rx_path(dev);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	ret = lan78xx_write_reg(dev, WUCSR, 0);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;
	ret = lan78xx_write_reg(dev, WUCSR2, 0);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;
	ret = lan78xx_write_reg(dev, WK_SRC, 0xFFF1FF1FUL);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	temp_wucsr = 0;

	temp_pmt_ctl = 0;

	ret = lan78xx_read_reg(dev, PMT_CTL, &temp_pmt_ctl);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	temp_pmt_ctl &= ~PMT_CTL_RES_CLR_WKP_EN_;
	temp_pmt_ctl |= PMT_CTL_RES_CLR_WKP_STS_;

	for (mask_index = 0; mask_index < NUM_OF_WUF_CFG; mask_index++) {
		ret = lan78xx_write_reg(dev, WUF_CFG(mask_index), 0);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			return ret;
	}

	mask_index = 0;
	if (wol & WAKE_PHY) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		temp_pmt_ctl |= PMT_CTL_PHY_WAKE_EN_;

		temp_pmt_ctl |= PMT_CTL_WOL_EN_;
		temp_pmt_ctl &= ~PMT_CTL_SUS_MODE_MASK_;
		temp_pmt_ctl |= PMT_CTL_SUS_MODE_0_;
	}
	if (wol & WAKE_MAGIC) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		temp_wucsr |= WUCSR_MPEN_;

		temp_pmt_ctl |= PMT_CTL_WOL_EN_;
		temp_pmt_ctl &= ~PMT_CTL_SUS_MODE_MASK_;
		temp_pmt_ctl |= PMT_CTL_SUS_MODE_3_;
	}
	if (wol & WAKE_BCAST) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		temp_wucsr |= WUCSR_BCST_EN_;

		temp_pmt_ctl |= PMT_CTL_WOL_EN_;
		temp_pmt_ctl &= ~PMT_CTL_SUS_MODE_MASK_;
		temp_pmt_ctl |= PMT_CTL_SUS_MODE_0_;
	}
	if (wol & WAKE_MCAST) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		temp_wucsr |= WUCSR_WAKE_EN_;

		/* set WUF_CFG & WUF_MASK for IPv4 Multicast */
		crc = lan78xx_wakeframe_crc16(ipv4_multicast, 3);
		ret = lan78xx_write_reg(dev, WUF_CFG(mask_index),
					WUF_CFGX_EN_ |
					WUF_CFGX_TYPE_MCAST_ |
					(0 << WUF_CFGX_OFFSET_SHIFT_) |
					(crc & WUF_CFGX_CRC16_MASK_));
		if (ret < 0)
			return ret;

		ret = lan78xx_write_reg(dev, WUF_MASK0(mask_index), 7);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			return ret;
		ret = lan78xx_write_reg(dev, WUF_MASK1(mask_index), 0);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			return ret;
		ret = lan78xx_write_reg(dev, WUF_MASK2(mask_index), 0);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			return ret;
		ret = lan78xx_write_reg(dev, WUF_MASK3(mask_index), 0);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			return ret;

		mask_index++;

		/* for IPv6 Multicast */
		crc = lan78xx_wakeframe_crc16(ipv6_multicast, 2);
		ret = lan78xx_write_reg(dev, WUF_CFG(mask_index),
					WUF_CFGX_EN_ |
					WUF_CFGX_TYPE_MCAST_ |
					(0 << WUF_CFGX_OFFSET_SHIFT_) |
					(crc & WUF_CFGX_CRC16_MASK_));
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			return ret;

		ret = lan78xx_write_reg(dev, WUF_MASK0(mask_index), 3);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			return ret;
		ret = lan78xx_write_reg(dev, WUF_MASK1(mask_index), 0);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			return ret;
		ret = lan78xx_write_reg(dev, WUF_MASK2(mask_index), 0);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			return ret;
		ret = lan78xx_write_reg(dev, WUF_MASK3(mask_index), 0);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			return ret;

		mask_index++;

		temp_pmt_ctl |= PMT_CTL_WOL_EN_;
		temp_pmt_ctl &= ~PMT_CTL_SUS_MODE_MASK_;
		temp_pmt_ctl |= PMT_CTL_SUS_MODE_0_;
	}
	if (wol & WAKE_UCAST) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		temp_wucsr |= WUCSR_PFDA_EN_;

		temp_pmt_ctl |= PMT_CTL_WOL_EN_;
		temp_pmt_ctl &= ~PMT_CTL_SUS_MODE_MASK_;
		temp_pmt_ctl |= PMT_CTL_SUS_MODE_0_;
	}
	if (wol & WAKE_ARP) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		temp_wucsr |= WUCSR_WAKE_EN_;

		/* set WUF_CFG & WUF_MASK
		 * for packettype (offset 12,13) = ARP (0x0806)
		 */
		crc = lan78xx_wakeframe_crc16(arp_type, 2);
		ret = lan78xx_write_reg(dev, WUF_CFG(mask_index),
					WUF_CFGX_EN_ |
					WUF_CFGX_TYPE_ALL_ |
					(0 << WUF_CFGX_OFFSET_SHIFT_) |
					(crc & WUF_CFGX_CRC16_MASK_));
		if (ret < 0)
			return ret;

		ret = lan78xx_write_reg(dev, WUF_MASK0(mask_index), 0x3000);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			return ret;
		ret = lan78xx_write_reg(dev, WUF_MASK1(mask_index), 0);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			return ret;
		ret = lan78xx_write_reg(dev, WUF_MASK2(mask_index), 0);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			return ret;
		ret = lan78xx_write_reg(dev, WUF_MASK3(mask_index), 0);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			return ret;

		mask_index++;

		temp_pmt_ctl |= PMT_CTL_WOL_EN_;
		temp_pmt_ctl &= ~PMT_CTL_SUS_MODE_MASK_;
		temp_pmt_ctl |= PMT_CTL_SUS_MODE_0_;
	}

	ret = lan78xx_write_reg(dev, WUCSR, temp_wucsr);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	/* when multiple WOL bits are set */
	if (hweight_long((unsigned long)wol) > 1) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		temp_pmt_ctl |= PMT_CTL_WOL_EN_;
		temp_pmt_ctl &= ~PMT_CTL_SUS_MODE_MASK_;
		temp_pmt_ctl |= PMT_CTL_SUS_MODE_0_;
	}
	ret = lan78xx_write_reg(dev, PMT_CTL, temp_pmt_ctl);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	/* clear WUPS */
	ret = lan78xx_read_reg(dev, PMT_CTL, &buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	buf |= PMT_CTL_WUPS_MASK_;

	ret = lan78xx_write_reg(dev, PMT_CTL, buf);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	ret = lan78xx_start_rx_path(dev);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return ret;
}

static int lan78xx_suspend(struct usb_interface *intf, pm_message_t message)
{
	struct lan78xx_net *dev = usb_get_intfdata(intf);
	bool dev_open;
	int ret=0;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	mutex_lock(&dev->dev_mutex);

	netif_dbg(dev, ifdown, dev->net,
		  "suspending: pm event %#x", message.event);

	dev_open = test_bit(EVENT_DEV_OPEN, &dev->flags);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (dev_open) {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		spin_lock_irq(&dev->txq.lock);
		/* don't autosuspend while transmitting */
		if ((skb_queue_len(&dev->txq) ||
		     skb_queue_len(&dev->txq_pend)) &&
		    PMSG_IS_AUTO(message)) {
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			spin_unlock_irq(&dev->txq.lock);
			ret = -EBUSY;
			goto out;
		} else {
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			set_bit(EVENT_DEV_ASLEEP, &dev->flags);
			spin_unlock_irq(&dev->txq.lock);
		}

		/* stop RX */
		ret = lan78xx_stop_rx_path(dev);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			goto out;

		ret = lan78xx_flush_rx_fifo(dev);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			goto out;

		/* stop Tx */
		ret = lan78xx_stop_tx_path(dev);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			goto out;

		/* empty out the Rx and Tx queues */
		netif_device_detach(dev->net);
		lan78xx_terminate_urbs(dev);
		usb_kill_urb(dev->urb_intr);

		/* reattach */
		netif_device_attach(dev->net);

		del_timer(&dev->stat_monitor);

		if (PMSG_IS_AUTO(message)) {
			ret = lan78xx_set_auto_suspend(dev);
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			if (ret < 0)
				goto out;
		} else {
			struct lan78xx_priv *pdata;
			pr_debug(" SOFTING  %d %d", __LINE__, ret);

			pdata = (struct lan78xx_priv *)(dev->data[0]);
			netif_carrier_off(dev->net);
			ret = lan78xx_set_suspend(dev, pdata->wol);
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			if (ret < 0)
				goto out;
		}
	} else {
		/* Interface is down; don't allow WOL and PHY
		 * events to wake up the host
		 */
		u32 buf;
		pr_debug(" SOFTING  %d %d", __LINE__, ret);

		set_bit(EVENT_DEV_ASLEEP, &dev->flags);

		ret = lan78xx_write_reg(dev, WUCSR, 0);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			goto out;
		ret = lan78xx_write_reg(dev, WUCSR2, 0);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			goto out;

		ret = lan78xx_read_reg(dev, PMT_CTL, &buf);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			goto out;

		buf &= ~PMT_CTL_RES_CLR_WKP_EN_;
		buf |= PMT_CTL_RES_CLR_WKP_STS_;
		buf &= ~PMT_CTL_SUS_MODE_MASK_;
		buf |= PMT_CTL_SUS_MODE_3_;

		ret = lan78xx_write_reg(dev, PMT_CTL, buf);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			goto out;

		ret = lan78xx_read_reg(dev, PMT_CTL, &buf);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			goto out;

		buf |= PMT_CTL_WUPS_MASK_;

		ret = lan78xx_write_reg(dev, PMT_CTL, buf);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			goto out;
	}

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	ret = 0;
out:
	mutex_unlock(&dev->dev_mutex);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return ret;
}

static bool lan78xx_submit_deferred_urbs(struct lan78xx_net *dev)
{
	bool pipe_halted = false;
	struct urb *urb;

	pr_debug(" SOFTING  %d", __LINE__);
	while ((urb = usb_get_from_anchor(&dev->deferred))) {
		struct sk_buff *skb = urb->context;
		int ret=0;

		if (!netif_device_present(dev->net) ||
		    !netif_carrier_ok(dev->net) ||
		    pipe_halted) {
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			usb_free_urb(urb);
			dev_kfree_skb(skb);
			continue;
		}

		ret = usb_submit_urb(urb, GFP_ATOMIC);

		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret == 0) {
			netif_trans_update(dev->net);
			lan78xx_queue_skb(&dev->txq, skb, tx_start);
		} else {
			usb_free_urb(urb);
			dev_kfree_skb(skb);

			if (ret == -EPIPE) {
				pr_debug(" SOFTING  %d %d", __LINE__, ret);
				netif_stop_queue(dev->net);
				pipe_halted = true;
			} else if (ret == -ENODEV) {
				pr_debug(" SOFTING  %d %d", __LINE__, ret);
				netif_device_detach(dev->net);
			}
		}
	}

	pr_debug(" SOFTING  %d", __LINE__);
	return pipe_halted;
}

static int lan78xx_resume(struct usb_interface *intf)
{
	struct lan78xx_net *dev = usb_get_intfdata(intf);
	bool dev_open;
	int ret=0;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	mutex_lock(&dev->dev_mutex);

	netif_dbg(dev, ifup, dev->net, "resuming device");

	dev_open = test_bit(EVENT_DEV_OPEN, &dev->flags);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (dev_open) {
		bool pipe_halted = false;
		pr_debug(" SOFTING  %d %d", __LINE__, ret);

		ret = lan78xx_flush_tx_fifo(dev);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			goto out;

		if (dev->urb_intr) {
			int ret = usb_submit_urb(dev->urb_intr, GFP_KERNEL);
			pr_debug(" SOFTING  %d %d", __LINE__, ret);

			if (ret < 0) {
				if (ret == -ENODEV) {
					netif_device_detach(dev->net);
					pr_debug(" SOFTING  %d %d", __LINE__, ret);
				}
				pr_debug(" SOFTING  %d %d", __LINE__, ret);

				netdev_warn(dev->net, "Failed to submit intr URB");
			}
		}

		spin_lock_irq(&dev->txq.lock);

		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (netif_device_present(dev->net)) {
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			pipe_halted = lan78xx_submit_deferred_urbs(dev);

			if (pipe_halted)
				lan78xx_defer_kevent(dev, EVENT_TX_HALT);
		}

		clear_bit(EVENT_DEV_ASLEEP, &dev->flags);

		spin_unlock_irq(&dev->txq.lock);

		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (!pipe_halted &&
		    netif_device_present(dev->net) &&
		    (skb_queue_len(&dev->txq) < dev->tx_qlen))
			netif_start_queue(dev->net);

		ret = lan78xx_start_tx_path(dev);
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		if (ret < 0)
			goto out;

		tasklet_schedule(&dev->bh);

		if (!timer_pending(&dev->stat_monitor)) {
			pr_debug(" SOFTING  %d %d", __LINE__, ret);
			dev->delta = 1;
			mod_timer(&dev->stat_monitor,
				  jiffies + STAT_UPDATE_TIMER);
		}

	} else {
		pr_debug(" SOFTING  %d %d", __LINE__, ret);
		clear_bit(EVENT_DEV_ASLEEP, &dev->flags);
	}

	ret = lan78xx_write_reg(dev, WUCSR2, 0);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		goto out;
	ret = lan78xx_write_reg(dev, WUCSR, 0);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		goto out;
	ret = lan78xx_write_reg(dev, WK_SRC, 0xFFF1FF1FUL);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		goto out;

	ret = lan78xx_write_reg(dev, WUCSR2, WUCSR2_NS_RCD_ |
					     WUCSR2_ARP_RCD_ |
					     WUCSR2_IPV6_TCPSYN_RCD_ |
					     WUCSR2_IPV4_TCPSYN_RCD_);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		goto out;

	ret = lan78xx_write_reg(dev, WUCSR, WUCSR_EEE_TX_WAKE_ |
					    WUCSR_EEE_RX_WAKE_ |
					    WUCSR_PFDA_FR_ |
					    WUCSR_RFE_WAKE_FR_ |
					    WUCSR_WUFR_ |
					    WUCSR_MPR_ |
					    WUCSR_BCST_FR_);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		goto out;

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	ret = 0;
out:
	mutex_unlock(&dev->dev_mutex);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return ret;
}

static int lan78xx_reset_resume(struct usb_interface *intf)
{
	struct lan78xx_net *dev = usb_get_intfdata(intf);
	int ret;

	netif_dbg(dev, ifup, dev->net, "(reset) resuming device");

	ret = lan78xx_reset(dev);
	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	if (ret < 0)
		return ret;

	phy_start(dev->net->phydev);

	/* Force link status check after reset resume */
	lan78xx_defer_kevent(dev, EVENT_LINK_RESET);

	ret = lan78xx_resume(intf);

	pr_debug(" SOFTING  %d %d", __LINE__, ret);
	return ret;
}

static const struct usb_device_id products[] = {
	{
	/* LAN7800 USB Gigabit Ethernet Device */
	USB_DEVICE(LAN78XX_USB_VENDOR_ID, LAN7800_USB_PRODUCT_ID),
	},
	{
	/* LAN7850 USB Gigabit Ethernet Device */
	USB_DEVICE(LAN78XX_USB_VENDOR_ID, LAN7850_USB_PRODUCT_ID),
	},
	{
	/* LAN7801 USB Gigabit Ethernet Device */
	USB_DEVICE(LAN78XX_USB_VENDOR_ID, LAN7801_USB_PRODUCT_ID),
	},
	{
	/* ATM2-AF USB Gigabit Ethernet Device */
	USB_DEVICE(AT29M2AF_USB_VENDOR_ID, AT29M2AF_USB_PRODUCT_ID),
	},
	{},
};
MODULE_DEVICE_TABLE(usb, products);

static struct usb_driver lan78xx_driver = {
	.name			= DRIVER_NAME,
	.id_table		= products,
	.probe			= lan78xx_probe,
	.disconnect		= lan78xx_disconnect,
	.suspend		= lan78xx_suspend,
	.resume			= lan78xx_resume,
	.reset_resume		= lan78xx_reset_resume,
	.supports_autosuspend	= 1,
	.disable_hub_initiated_lpm = 1,
};

module_usb_driver(lan78xx_driver);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
