/* drivers/input/touchscreen/max1187x.c
 *
 * Copyright (c)2013 Maxim Integrated Products, Inc.
 * Copyright (C) 2013-2014 Sony Mobile Communications AB.
 *
 * Driver Version: 3.3.2.2
 * Release Date: October 29, 2013
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <asm/byteorder.h>
#include <linux/async.h>
#include <linux/crc16.h>
#include <linux/delay.h>
#include <linux/errno.h>
#ifdef CONFIG_FB
#include <linux/fb.h>
#endif
#include <linux/firmware.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/evdt_helper.h>
#include <linux/input/max1187x.h>
#include <linux/input/max1187x_config.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/module.h>
#ifdef CONFIG_FB
#include <linux/notifier.h>
#endif
#include <linux/of.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/types.h>

#define NWORDS(a)    (sizeof(a) / sizeof(u16))
#define BYTE_SIZE(a) ((a) * sizeof(u16))
#define BYTEH(a)     ((a) >> 8)
#define BYTEL(a)     ((a) & 0xFF)
#define HI_NIBBLE(a) (((a) & 0xF0) >> 4)
#define LO_NIBBLE(a) ((a) & 0x0F)
#define DEBUG_FLAG(DEV, NAME, ...) ({		\
	bool debug_flag = false; \
	dev_dbg(DEV, NAME "%s", ##__VA_ARGS__, (debug_flag = true, "")); \
	debug_flag; \
})
#define INFO_BUFFER(DEV, BUF, LEN, FMT, FLN) ({			\
	int i, written;							\
	char debug_string[MXM_DEBUG_STRING_LEN_MAX];			\
	for (i = 0, written = 0; i < (LEN); i++) {			\
		written += snprintf(debug_string + written, FLN, FMT, BUF[i]); \
		if (written + FLN >= MXM_DEBUG_STRING_LEN_MAX) {	\
			dev_info(DEV, "%s", debug_string);		\
			written = 0;					\
		}							\
	}								\
	if (written > 0)						\
		dev_info(DEV, "%s", debug_string);			\
})
#define DEBUG_BUFFER(DEV, NAME, BUF, LEN, FMT, FLN) ({	\
	if (DEBUG_FLAG(DEV, NAME " (%d)", LEN))		\
		INFO_BUFFER(DEV, BUF, LEN, FMT, FLN);	\
})

#define MXM_WAKEUP_GESTURE "wakeup_gesture"

#define MXM_CMD_LEN_PACKET_MAX      9
#define MXM_CMD_LEN_MAX             (15 * MXM_CMD_LEN_PACKET_MAX)
#define MXM_CMD_LEN_PACKET_MIN      2
#define MXM_CMD_ID_AND_SIZE_LEN     2
#define MXM_CMD_ADD_AND_HDR_LEN     2
#define MXM_RPT_LEN_PACKET_MAX      245
#define MXM_RPT_LEN_MAX             1000
#define MXM_RPT_FIRST_PACKET        1
#define MXM_RPT_PKT_HDR_LEN         1
#define MXM_RPT_MAX_WORDS_PER_PKT   0xF4
#define MXM_ONE_PACKET_RPT          0x11
#define MXM_TOUCH_REPORT_MODE_EXT   0x02
#define MXM_REPORT_READERS_MAX      5
#define MXM_BYTES_LEN_IN_WORDS      2
#define MXM_BYTES_LEN_WR_BYTES      6
#define MXM_BYTES_LEN_WR_WORDS      8
#define MXM_DEBUG_STRING_LEN_MAX    60
#define MXM_PWR_DATA_WAKEUP_GEST    0x0102
#define MXM_TOUCH_COUNT_MAX         10
#define MXM_PRESSURE_Z_MIN_TO_SQRT  2
#define MXM_PRESSURE_SQRT_MAX       181
#define MXM_LCD_X_MIN               480
#define MXM_LCD_Y_MIN               240
#define MXM_LCD_SIZE_MAX            0x7FFF
#define MXM_NUM_SENSOR_MAX          40
#define MXM_IRQ_RESET_TIMEOUT       1000

/* Timings */
#define MXM_WAIT_MIN_US             1000
#define MXM_WAIT_MAX_US             2000
#define MXM_PWR_SET_WAIT_MS         100
#define MXM_CHIP_RESET_US           6000

/* Regulator */
#define MXM_LPM_UA_LOAD       2000
#define MXM_HPM_UA_LOAD       15000
#define MXM_VREG_MAX_UV       3000000

/* Bootloader */
#define MXM_BL_STATUS_ADDR_H     0x00
#define MXM_BL_STATUS_ADDR_L     0xFF
#define MXM_BL_DATA_ADDR_H       0x00
#define MXM_BL_DATA_ADDR_L       0xFE
#define MXM_BL_STATUS_READY_H    0xAB
#define MXM_BL_STATUS_READY_L    0xCC
#define MXM_BL_DATA_READY_H      0x00
#define MXM_BL_DATA_READY_L      0x3E
#define MXM_BL_RXTX_COMPLETE_H   0x54
#define MXM_BL_RXTX_COMPLETE_L   0x32
#define MXM_BL_ENTER_SEQ_L       0x7F00
#define MXM_BL_ENTER_SEQ_H1      0x0047
#define MXM_BL_ENTER_SEQ_H2      0x00C7
#define MXM_BL_ENTER_SEQ_H3      0x0007
#define MXM_BL_ENTER_RETRY       3
#define MXM_BL_ENTER_CONF_RETRY  5
#define MXM_BL_SET_BYTE_MODE_H   0x00
#define MXM_BL_SET_BYTE_MODE_L   0x0A
#define MXM_BL_ERASE_CONF_RETRY  10
#define MXM_BL_ERASE_DELAY_MS    60
#define MXM_BL_ERASE_FLASH_L     0x02
#define MXM_BL_ERASE_FLASH_H     0x00
#define MXM_BL_RD_STATUS_RETRY   3
#define MXM_BL_WR_FAST_FLASH_L   0xF0
#define MXM_BL_WR_FAST_FLASH_H   0x00
#define MXM_BL_WR_START_ADDR     0x00
#define MXM_BL_WR_DBUF0_ADDR     0x00
#define MXM_BL_WR_DBUF1_ADDR     0x40
#define MXM_BL_WR_BLK_SIZE       128
#define MXM_BL_WR_TX_SZ          130
#define MXM_BL_WR_MIN_US         10000
#define MXM_BL_WR_MAX_US         11000
#define MXM_BL_WR_STATUS_RETRY   100
#define MXM_BL_WR_DELAY_MS       200
#define MXM_BL_WR_CONF_RETRY     5
#define MXM_BL_GET_CRC_L         0x30
#define MXM_BL_GET_CRC_H         0x02
#define MXM_BL_CRC_GET_RETRY     5
#define MXM_BL_EXIT_SEQ_L        0x7F00
#define MXM_BL_EXIT_SEQ_H1       0x0040
#define MXM_BL_EXIT_SEQ_H2       0x00C0
#define MXM_BL_EXIT_SEQ_H3       0x0000
#define MXM_BL_EXIT_RETRY        3

/* Firmware Update */
#define MXM_FW_FILENAME_MAX   64
#define MXM_FW_RETRIES_MAX    5
#define MXM_FW_UPDATE_DEFAULT 0 /* flashing with dflt_cfg if fw is corrupted */
#define MXM_FW_UPDATE_FORCE   1 /* force flashing with dflt_cfg */

static const char * const fw_update_mode[] = {
	[MXM_FW_UPDATE_DEFAULT] = "default",
	[MXM_FW_UPDATE_FORCE] = "force",
};

enum maxim_coordinate_settings {
	MXM_REVERSE_X   = 0x0001,
	MXM_REVERSE_Y   = 0x0002,
	MXM_SWAP_XY     = 0x0004,
};

enum maxim_start_address {
	MXM_CMD_START_ADDR  = 0x0000,
	MXM_RPT_START_ADDR  = 0x000A,
};

enum maxim_command_size {
	MXM_ZERO_SIZE_CMD  = 0x0000,
	MXM_ONE_SIZE_CMD   = 0x0001,
};

enum maxim_command_id {
	MXM_CMD_ID_GET_CFG_INF         = 0x0002,
	MXM_CMD_ID_SET_TOUCH_RPT_MODE  = 0x0018,
	MXM_CMD_ID_SET_POWER_MODE      = 0x0020,
	MXM_CMD_ID_GET_FW_VERSION      = 0x0040,
	MXM_CMD_ID_SET_GLOVE_MODE      = 0x0083,
	MXM_CMD_ID_RESET_SYSTEM        = 0x00E9,
};

enum maxim_report_id {
	MXM_RPT_ID_CFG_INF           = 0x0102,
	MXM_RPT_ID_POWER_MODE        = 0x0121,
	MXM_RPT_ID_FW_VERSION        = 0x0140,
	MXM_RPT_ID_SYS_STATUS        = 0x01A0,
	MXM_RPT_ID_TOUCH_RAW_IMAGE   = 0x0800,
	MXM_RPT_ID_EXT_TOUCH_INFO    = 0x0802,
};

enum maxim_power_mode {
	MXM_PWR_SLEEP_MODE  = 0x0000,
	MXM_ACTIVE_MODE     = 0x0002,
	MXM_DRIVE_MODE      = 0x0004,
	MXM_SENSE_MODE      = 0x0005,
	MXM_WAKEUP_MODE     = 0x0006,
};

enum maxim_system_status {
	MXM_STATUS_FAIL_CMD     = 0x0001,
	MXM_STATUS_POWER_RESET  = 0x0008,
	MXM_STATUS_EXT_RESET    = 0x0010,
	MXM_STATUS_WD_RESET     = 0x0020,
	MXM_STATUS_SOFT_RESET   = 0x0040,
};

enum maxim_tool_type {
	MXM_TOOL_PEN       = 0x01,
	MXM_TOOL_FINGER    = 0x02,
	MXM_TOOL_GLOVE     = 0x03,
};

struct max1187x_packet_header {
	u16 total_num;
	u16 curr_num;
	u16 curr_size;
};

struct max1187x_touch_report_header {
	u16 header;
	u16 report_id;
	u16 report_size;
	u16 touch_count:4;
	u16 reserved0:12;
	u16 button0:1;
	u16 button1:1;
	u16 button2:1;
	u16 button3:1;
	u16 reserved1:12;
	u16 framecounter;
};

struct max1187x_system_status_report {
	u16 header;
	u16 report_id;
	u16 report_size;
	u16 value;
};

struct max1187x_touch_report_extended {
	u16 finger_id:4;
	u16 reserved0:4;
	u16 tool_type:4;
	u16 reserved1:4;
	u16 x:12;
	u16 reserved2:4;
	u16 y:12;
	u16 reserved3:4;
	u16 z;
	s16 xspeed;
	s16 yspeed;
	s8 xpixel;
	s8 ypixel;
	u16 area;
	u16 xmin;
	u16 xmax;
	u16 ymin;
	u16 ymax;
};

struct report_reader {
	u16 report_id;
	u16 reports_passed;
	struct semaphore sem;
	int status;
};

struct data {
	struct max1187x_pdata *pdata;
	struct i2c_client *client;
	struct input_dev *input_dev;
	char phys[32];
	struct device_node *evdt_node;
#ifdef CONFIG_FB
	struct notifier_block fb_notif;
	struct work_struct notify_resume;
	struct work_struct notify_suspend;
#endif
	bool is_suspended;
	bool pm_suspended;
	bool irq_on_suspend;
	struct regulator *vreg_touch_vdd;
	char *vdd_supply_name;
	wait_queue_head_t waitqueue_all;

	u16 chip_id;
	u16 config_id;

	struct mutex fw_mutex;
	struct mutex i2c_mutex;
	struct mutex report_mutex;
	struct semaphore report_sem;
	struct semaphore reset_sem;
	struct report_reader report_readers[MXM_REPORT_READERS_MAX];
	u8 report_readers_outstanding;

	u16 cmd_buf[MXM_CMD_LEN_MAX];
	u16 cmd_len;

	struct semaphore sema_rbcmd;
	wait_queue_head_t waitqueue_rbcmd;
	u8 rbcmd_waiting;
	u8 rbcmd_received;
	u16 rbcmd_report_id;
	u16 *rbcmd_rx_report;
	u16 *rbcmd_rx_report_len;

	u16 rx_report[MXM_RPT_LEN_MAX]; /* with header */
	u16 rx_report_len;
	u16 rx_packet[MXM_RPT_LEN_PACKET_MAX + 1]; /* with header */
	u32 irq_count;
	u16 framecounter;
	u16 list_finger_ids;
	u16 curr_finger_ids;
	u8 fw_update_mode;
	u8 sysfs_created;
	bool is_raw_mode;

	u16 button0:1;
	u16 button1:1;
	u16 button2:1;
	u16 button3:1;

	unsigned long ew_timeout;
};

#ifdef CONFIG_FB
static int fb_notifier_callback(struct notifier_block *self,
				unsigned long event, void *data);
static void notify_resume(struct work_struct *work);
static void notify_suspend(struct work_struct *work);
#endif

static int vreg_configure(struct data *ts, bool enable);

static void validate_fw(struct data *ts);
static int bootloader_enter(struct data *ts);
static int bootloader_exit(struct data *ts);
static int bootloader_get_crc(struct data *ts, u16 *crc16,
		u16 addr, u16 len, u16 delay);
static int bootloader_set_byte_mode(struct data *ts);
static int bootloader_erase_flash(struct data *ts);
static int bootloader_write_flash(struct data *ts, const u8 *image, u16 length);

static void propagate_report(struct data *ts, int status, u16 *report);
static int get_report(struct data *ts, u16 report_id, ulong timeout);
static void release_report(struct data *ts);
static int cmd_send_locked(struct data *ts, u16 *buf, u16 len);
static int cmd_send(struct data *ts, u16 *buf, u16 len);
static int rbcmd_send_receive(struct data *ts, u16 *cmd_buf,
		u16 cmd_len, u16 rpt_id,
		u16 *rpt_buf, u16 *rpt_len, u16 timeout);
static u16 max1187x_sqrt(u32 num);
static int reset_power(struct data *ts);
static int max1187x_set_glove_locked(struct data *ts, int enable);
static int max1187x_set_glove(struct data *ts, int enable);

/* I2C communication */
static int i2c_rx_bytes(struct data *ts, u8 *buf, u16 len)
{
	int ret;
	struct device *dev = &ts->client->dev;

	do {
		ret = i2c_master_recv(ts->client, (char *) buf, (int) len);
	} while (ret == -EAGAIN);
	if (ret < 0) {
		dev_err(dev, "I2C RX fail (%d)", ret);
		return ret;
	}

	len = ret;

	DEBUG_BUFFER(dev, "i2c_debug:rx",
		     buf, len, "0x%02x ", MXM_BYTES_LEN_WR_BYTES);

	return len;
}

static int i2c_rx_words(struct data *ts, u16 *buf, u16 len)
{
	int i, ret;
	struct device *dev = &ts->client->dev;

	do {
		ret = i2c_master_recv(ts->client,
			(char *) buf, (int) (len * MXM_BYTES_LEN_IN_WORDS));
	} while (ret == -EAGAIN);
	if (ret < 0) {
		dev_err(dev, "I2C RX fail (%d)", ret);
		return ret;
	}

	if (ret % 2) {
		dev_err(dev, "I2C words RX fail: odd number bytes (%d)", ret);
		return -EIO;
	}

	len = ret/2;

	for (i = 0; i < len; i++)
		buf[i] = cpu_to_le16(buf[i]);

	DEBUG_BUFFER(dev, "i2c_debug:rx",
		     buf, len, "0x%04x ", MXM_BYTES_LEN_WR_WORDS);

	return len;
}

static int i2c_tx_bytes(struct data *ts, u8 *buf, u16 len)
{
	int ret;
	struct device *dev = &ts->client->dev;

	do {
		ret = i2c_master_send(ts->client, (char *) buf, (int) len);
	} while (ret == -EAGAIN);
	if (ret < 0) {
		dev_err(dev, "I2C TX fail (%d)", ret);
		return ret;
	}

	len = ret;

	DEBUG_BUFFER(dev, "i2c_debug:tx",
		     buf, len, "0x%02x ", MXM_BYTES_LEN_WR_BYTES);

	return len;
}

static int i2c_tx_words(struct data *ts, u16 *buf, u16 len)
{
	int i, ret;
	struct device *dev = &ts->client->dev;

	for (i = 0; i < len; i++)
		buf[i] = cpu_to_le16(buf[i]);

	do {
		ret = i2c_master_send(ts->client,
			(char *) buf, (int) (len * MXM_BYTES_LEN_IN_WORDS));
	} while (ret == -EAGAIN);
	if (ret < 0) {
		dev_err(dev, "I2C TX fail (%d)", ret);
		return ret;
	}
	if (ret % 2) {
		dev_err(dev, "I2C words TX fail: odd number bytes (%d)", ret);
		return -EIO;
	}

	len = ret/2;

	DEBUG_BUFFER(dev, "i2c_debug:tx",
		     buf, len, "0x%04x ", MXM_BYTES_LEN_WR_WORDS);

	return len;
}

/* Read report */
static int read_mtp_report(struct data *ts, u16 *buf)
{
	int words = 1, words_tx, words_rx;
	int ret = 0, remainder = 0, offset = 0;
	u16 address = MXM_RPT_START_ADDR;
	struct device *dev = &ts->client->dev;

	/* read header, get size, read entire report */
	words_tx = i2c_tx_words(ts, &address, 1);
	if (words_tx != 1) {
		dev_err(dev, "Report RX fail: failed to set address");
		return -EIO;
	}

	if (!ts->is_raw_mode) {
		words_rx = i2c_rx_words(ts, buf, MXM_RPT_PKT_HDR_LEN);
		if (words_rx != MXM_RPT_PKT_HDR_LEN ||
			BYTEL(buf[0]) > MXM_RPT_LEN_PACKET_MAX) {
			ret = -EIO;
			dev_err(dev, "Report RX fail: received (%d) " \
					"expected (%d) words, " \
					"header (%04X)",
					words_rx, words, buf[0]);
			return ret;
		}

		words = BYTEL(buf[0]) + MXM_RPT_PKT_HDR_LEN;

		if (words > (u16) ts->pdata->i2c_words)
			words = (u16) ts->pdata->i2c_words;

		words_tx = i2c_tx_words(ts, &address, 1);
		if (words_tx != 1) {
			dev_err(dev, "Report RX fail:" \
				"failed to set address");
			return -EIO;
		}

		words_rx = i2c_rx_words(ts, &buf[offset], words);
		if (words_rx != words) {
			dev_err(dev, "Report RX fail 0x%X: " \
				"received (%d) expected (%d) words",
				address, words_rx, remainder);
			return -EIO;
		}
	} else {
		words_rx = i2c_rx_words(ts, buf, (u16) ts->pdata->i2c_words);
		if (words_rx != (u16) ts->pdata->i2c_words ||
			BYTEL(buf[0]) > MXM_RPT_LEN_PACKET_MAX) {
			ret = -EIO;
			dev_err(dev, "Report RX fail: received (%d) " \
				"expected (%d) words, header (%04X)",
				words_rx, words, buf[0]);
			return ret;
		}

		words = BYTEL(buf[0]) + MXM_RPT_PKT_HDR_LEN;
		remainder = words;

		if (remainder - (u16) ts->pdata->i2c_words > 0) {
			remainder -= (u16) ts->pdata->i2c_words;
			offset += (u16) ts->pdata->i2c_words;
			address += (u16) ts->pdata->i2c_words;
		}

		words_tx = i2c_tx_words(ts, &address, 1);
		if (words_tx != 1) {
			dev_err(dev, "Report RX fail: failed to set " \
				"address 0x%X", address);
			return -EIO;
		}

		words_rx = i2c_rx_words(ts, &buf[offset], remainder);
		if (words_rx != remainder) {
			dev_err(dev, "Report RX fail 0x%X: " \
				"received (%d) expected (%d) words",
					address, words_rx, remainder);
			return -EIO;
		}
	}

	return ret;
}

/* Send command */
static int send_mtp_command(struct data *ts, u16 *buf, u16 len)
{
	u16 tx_buf[MXM_CMD_LEN_PACKET_MAX + MXM_CMD_ADD_AND_HDR_LEN];
	u16 packets, last_packet, words_tx;
	struct max1187x_packet_header pkt;
	int i, ret = 0;
	u16 cmd_data_size = buf[1];

	/* check basics */
	if (len < MXM_CMD_LEN_PACKET_MIN || len > MXM_CMD_LEN_MAX ||
			(cmd_data_size + MXM_CMD_ID_AND_SIZE_LEN) != len) {
		dev_err(&ts->client->dev, "Command length is not valid");
		ret = -EINVAL;
		goto err_send_mtp_command;
	}

	/* packetize and send */
	packets = len / MXM_CMD_LEN_PACKET_MAX;
	if (len % MXM_CMD_LEN_PACKET_MAX)
		packets++;
	last_packet = packets - 1;
	pkt.total_num = packets << 12;
	tx_buf[0] = MXM_CMD_START_ADDR;

	for (i = 0; i < packets; i++) {
		pkt.curr_num = (i + 1) << 8;
		pkt.curr_size = (i == (last_packet)) ?
					len : MXM_CMD_LEN_PACKET_MAX;
		tx_buf[1] = pkt.total_num | pkt.curr_num | pkt.curr_size;
		memcpy(&tx_buf[2], &buf[i * MXM_CMD_LEN_PACKET_MAX],
						BYTE_SIZE(pkt.curr_size));
		words_tx = i2c_tx_words(ts, tx_buf,
				pkt.curr_size + MXM_CMD_ADD_AND_HDR_LEN);
		if (words_tx != (pkt.curr_size + MXM_CMD_ADD_AND_HDR_LEN)) {
			dev_err(&ts->client->dev, "Command TX fail: " \
			"transmitted (%d) expected (%d) words, packet (%d)",
			words_tx, pkt.curr_size + MXM_CMD_ADD_AND_HDR_LEN, i);
			ret = -EIO;
			goto err_send_mtp_command;
		}
		len -= MXM_CMD_LEN_PACKET_MAX;
	}

err_send_mtp_command:
	return ret;
}

/* Integer math operations */
u16 max1187x_sqrt(u32 num)
{
	u16 mask = 0x8000;
	u16 guess = 0;
	u32 prod = 0;

	if (num < MXM_PRESSURE_Z_MIN_TO_SQRT)
		return num;

	while (mask) {
		guess = guess ^ mask;
		prod = guess * guess;
		if (num < prod)
			guess = guess ^ mask;
		mask = mask>>1;
	}
	if (guess != 0xFFFF) {
		prod = guess * guess;
		if ((num - prod) > (prod + 2 * guess + 1 - num))
			guess++;
	}

	return guess;
}

static void report_buttons(struct data *ts,
			   struct max1187x_touch_report_header *header)
{
	struct max1187x_pdata *pdata = ts->pdata;

	if (!ts->input_dev->users)
		return;

	if (header->button0 != ts->button0) {
		input_report_key(ts->input_dev, pdata->button_code0,
				header->button0);
		input_sync(ts->input_dev);
		ts->button0 = header->button0;
	}
	if (header->button1 != ts->button1) {
		input_report_key(ts->input_dev, pdata->button_code1,
				header->button1);
		input_sync(ts->input_dev);
		ts->button1 = header->button1;
	}
	if (header->button2 != ts->button2) {
		input_report_key(ts->input_dev, pdata->button_code2,
				header->button2);
		input_sync(ts->input_dev);
		ts->button2 = header->button2;
	}
	if (header->button3 != ts->button3) {
		input_report_key(ts->input_dev, pdata->button_code3,
				header->button3);
		input_sync(ts->input_dev);
		ts->button3 = header->button3;
	}
}

static void report_down(struct data *ts,
			struct max1187x_touch_report_extended *e)
{
	struct max1187x_pdata *pdata = ts->pdata;
	struct device *dev = &ts->client->dev;
	struct input_dev *idev = ts->input_dev;
	u32 xcell = pdata->lcd_x / pdata->num_sensor_x;
	u32 ycell = pdata->lcd_y / pdata->num_sensor_y;
	u16 x = e->x;
	u16 y = e->y;
	u16 z = e->z;
	u16 raw_tool_type = e->tool_type;
	u16 tool_type;
	u16 id = e->finger_id;
	u16 idbit = 1 << id;
	s8 xpixel = e->xpixel;
	s8 ypixel = e->ypixel;
	u32 touch_major, touch_minor;
	s16 xsize, ysize, orientation;
	bool valid;

	if (pdata->coordinate_settings & MXM_SWAP_XY) {
		swap(x, y);
		swap(xpixel, ypixel);
	}
	if (pdata->coordinate_settings & MXM_REVERSE_X) {
		x = pdata->panel_margin_xl + pdata->lcd_x
			+ pdata->panel_margin_xh - 1 - x;
		xpixel = -xpixel;
	}
	if (pdata->coordinate_settings & MXM_REVERSE_Y) {
		y = pdata->panel_margin_yl + pdata->lcd_y
			+ pdata->panel_margin_yh - 1 - y;
		ypixel = -ypixel;
	}

	z = (MXM_PRESSURE_SQRT_MAX >> 2) + max1187x_sqrt(z);
	if (z > MXM_PRESSURE_SQRT_MAX)
		z = MXM_PRESSURE_SQRT_MAX;
	xsize = xpixel * (s16)xcell;
	ysize = ypixel * (s16)ycell;
	if (xsize < 0)
		xsize = -xsize;
	if (ysize < 0)
		ysize = -ysize;
	orientation = (xsize > ysize) ? 0 : 90;
	touch_major = (xsize > ysize) ? xsize : ysize;
	touch_minor = (xsize > ysize) ? ysize : xsize;

	if (raw_tool_type == MXM_TOOL_PEN) {
		if (ts->pdata->report_pen_as_finger)
			tool_type = MT_TOOL_FINGER;
		else
			tool_type = MT_TOOL_PEN;
	} else {
		if (raw_tool_type == MXM_TOOL_GLOVE) {
			if (ts->pdata->glove_enabled)
				z += MXM_PRESSURE_SQRT_MAX + 1;
			else
				return;
		}
		tool_type = MT_TOOL_FINGER;
	}
	valid = idev->users > 0;
	ts->curr_finger_ids |= idbit;

	if (valid) {
		input_report_abs(idev, ABS_MT_TRACKING_ID, id);
		input_report_abs(idev, ABS_MT_TOOL_TYPE, tool_type);
		input_report_abs(idev, ABS_MT_POSITION_X, x);
		input_report_abs(idev, ABS_MT_POSITION_Y, y);
		if (pdata->pressure_enabled)
			input_report_abs(idev, ABS_MT_PRESSURE, z);
		if (pdata->orientation_enabled)
			input_report_abs(idev, ABS_MT_ORIENTATION, orientation);
		if (pdata->size_enabled) {
			input_report_abs(idev, ABS_MT_TOUCH_MAJOR, touch_major);
			input_report_abs(idev, ABS_MT_TOUCH_MINOR, touch_minor);
		}
		input_mt_sync(idev);
	}
	dev_dbg(dev, "event: %s%s%s %u: [XY %4d %4d ][PMmO %4d %4d %4d %3d ]",
		!(ts->list_finger_ids & (1 << id)) ? "DOWN" : "MOVE",
		valid ? " " : "#",
		raw_tool_type == MXM_TOOL_FINGER ? "Finger" :
		raw_tool_type == MXM_TOOL_PEN ? "Stylus" :
		raw_tool_type == MXM_TOOL_GLOVE ? "Glove" : "*Unknown*",
		id, x, y, z, touch_major, touch_minor, orientation);
}

static void report_up(struct data *ts, int id,
			struct max1187x_touch_report_extended *e)
{
	struct device *dev = &ts->client->dev;
	struct input_dev *idev = ts->input_dev;
	u16 raw_tool_type = e->tool_type;
	u16 idbit = 1 << id;
	bool valid;

	if (!(ts->list_finger_ids & idbit))
		return;

	valid = idev->users > 0;
	if (valid)
		input_mt_sync(idev);
	dev_dbg(dev, "event: UP%s%s %u\n",
		valid ? " " : "#",
		raw_tool_type == MXM_TOOL_FINGER ? "Finger" :
		raw_tool_type == MXM_TOOL_PEN ? "Stylus" :
		raw_tool_type == MXM_TOOL_GLOVE ? "Glove" : "*Unknown*",
		id);
	ts->list_finger_ids &= ~idbit;
}

static void invalidate_all_fingers(struct data *ts)
{
	struct device *dev = &ts->client->dev;

	dev_dbg(dev, "event: UP all\n");
	if (ts->input_dev->users) {
		input_mt_sync(ts->input_dev);
		input_sync(ts->input_dev);
	}
	ts->list_finger_ids = 0;
}

static void reinit_chip_settings(struct data *ts)
{
	u16 cmd_buf[] = {MXM_CMD_ID_SET_POWER_MODE,
			 MXM_ONE_SIZE_CMD,
			 MXM_ACTIVE_MODE};
	int ret;

	dev_dbg(&ts->client->dev, "%s: Enter\n", __func__);

	ret = cmd_send_locked(ts, cmd_buf, NWORDS(cmd_buf));
	if (ret)
		dev_err(&ts->client->dev, "Failed to set active mode");

	cmd_buf[0] = MXM_CMD_ID_SET_TOUCH_RPT_MODE;
	cmd_buf[1] = MXM_ONE_SIZE_CMD;
	cmd_buf[2] = MXM_TOUCH_REPORT_MODE_EXT;

	ret = cmd_send_locked(ts, cmd_buf, NWORDS(cmd_buf));
	if (ret)
		dev_err(&ts->client->dev, "Failed to set up touch report mode");

	max1187x_set_glove_locked(ts, ts->pdata->glove_enabled);

	dev_dbg(&ts->client->dev, "%s: Exit\n", __func__);
}

static void report_wakeup_gesture(struct data *ts,
				  struct max1187x_touch_report_header *header)
{
	struct device *dev = &ts->client->dev;
	u16 code = header->touch_count | (header->reserved0 << 4);

	dev_dbg(dev, "event: Received gesture: (0x%04X)\n", code);

	if (time_after(jiffies, ts->ew_timeout))
		ts->ew_timeout = jiffies + msecs_to_jiffies(
			ts->pdata->wakeup_gesture_timeout);
	else
		return;

	evdt_execute(ts->evdt_node, ts->input_dev, code);
}

static void process_report(struct data *ts, u16 *buf)
{
	u32 i;
	struct device *dev = &ts->client->dev;
	struct max1187x_touch_report_header *header;
	struct max1187x_system_status_report *status_report;
	struct max1187x_touch_report_extended *reporte;

	header = (struct max1187x_touch_report_header *) buf;

	if (BYTEH(header->header) != MXM_ONE_PACKET_RPT)
		goto end;

	if (device_may_wakeup(dev)) {
		if (header->report_id == MXM_RPT_ID_POWER_MODE
		    && ts->is_suspended) {
			report_wakeup_gesture(ts, header);
			goto end;
		}
	}

	if (header->report_id == MXM_RPT_ID_SYS_STATUS) {
		status_report = (struct max1187x_system_status_report *) buf;

		if (status_report->value & MXM_STATUS_EXT_RESET ||
		    status_report->value & MXM_STATUS_SOFT_RESET) {
			reinit_chip_settings(ts);
			if (status_report->value & MXM_STATUS_EXT_RESET)
				up(&ts->reset_sem);
		}
		goto end;
	}

	if (header->report_id != MXM_RPT_ID_EXT_TOUCH_INFO)
		goto end;

	if (ts->framecounter == header->framecounter) {
		dev_err(dev, "Same framecounter (%u) encountered  " \
			"at irq (%u)!\n", ts->framecounter, ts->irq_count);
		goto end;
	}
	ts->framecounter = header->framecounter;

	report_buttons(ts, header);

	if (header->touch_count > MXM_TOUCH_COUNT_MAX) {
		dev_err(dev, "Touch count (%u) out of bounds [0,10]!",
				header->touch_count);
		goto end;
	} else if (!header->touch_count) {
		invalidate_all_fingers(ts);
		goto end;
	}

	ts->curr_finger_ids = 0;
	reporte = (struct max1187x_touch_report_extended *)
		((u8 *)buf + sizeof(*header));
	for (i = 0; i < header->touch_count; i++, reporte++)
		report_down(ts, reporte);
	for (i = 0; i < MXM_TOUCH_COUNT_MAX; i++) {
		if (!(ts->curr_finger_ids & (1 << i)))
			report_up(ts, i, reporte);
	}
	if (ts->input_dev->users)
		input_sync(ts->input_dev);
	ts->list_finger_ids = ts->curr_finger_ids;
end:
	return;
}

static irqreturn_t irq_handler_soft(int irq, void *context)
{
	struct data *ts = (struct data *) context;
	int ret;

	dev_dbg(&ts->client->dev, "%s: Enter\n", __func__);

	mutex_lock(&ts->i2c_mutex);

	ret = read_mtp_report(ts, ts->rx_packet);
	if (!ret) {
		process_report(ts, ts->rx_packet);
		propagate_report(ts, 0, ts->rx_packet);
	} else {
		if (down_timeout(&ts->reset_sem,
		    msecs_to_jiffies(MXM_IRQ_RESET_TIMEOUT)) == 0)
			reset_power(ts);
		else
			dev_err(&ts->client->dev, "irq reset timeout\n");
	}

	mutex_unlock(&ts->i2c_mutex);

	dev_dbg(&ts->client->dev, "%s: Exit\n", __func__);
	return IRQ_HANDLED;
}

static irqreturn_t irq_handler_hard(int irq, void *context)
{
	struct data *ts = (struct data *) context;

	dev_dbg(&ts->client->dev, "%s: Enter\n", __func__);

	if (gpio_get_value(ts->pdata->gpio_tirq))
		goto irq_handler_hard_complete;

	ts->irq_count++;

	if (ts->pm_suspended) {
		ts->irq_on_suspend = true;
		dev_dbg(&ts->client->dev, "irq while suspended\n");
		goto irq_handler_hard_complete;
	}

	dev_dbg(&ts->client->dev, "%s: Exit\n", __func__);
	return IRQ_WAKE_THREAD;

irq_handler_hard_complete:
	dev_dbg(&ts->client->dev, "%s: Exit\n", __func__);
	return IRQ_HANDLED;
}

static ssize_t i2c_reset_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct data *ts = i2c_get_clientdata(client);
	int ret;

	ret = bootloader_exit(ts);
	if (ret) {
		dev_err(dev, "Failed to do i2c reset.");
		goto exit;
	}
	dev_info(dev, "i2c reset occured\n");
exit:
	return count;
}

static int reset_power(struct data *ts)
{
	int ret = -EINVAL;

	invalidate_all_fingers(ts);

	if (ts->pdata->gpio_reset) {
		ret = gpio_direction_output(ts->pdata->gpio_reset,
					    ts->pdata->reset_l2h ? 0 : 1);
		if (ret)
			goto exit;
	}
	usleep_range(MXM_WAIT_MIN_US, MXM_WAIT_MAX_US);
	if (ts->pdata->gpio_reset) {
		ret = gpio_direction_output(ts->pdata->gpio_reset,
					    ts->pdata->reset_l2h ? 1 : 0);
		if (ret)
			goto exit;
	}
	usleep_range(MXM_CHIP_RESET_US, MXM_CHIP_RESET_US + MXM_WAIT_MIN_US);

	dev_dbg(&ts->client->dev, "power on reset\n");
exit:
	if (ret) {
		dev_err(&ts->client->dev, "Failed to power on reset\n");
		up(&ts->reset_sem);
	}
	return ret;
}

static ssize_t power_on_reset_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct data *ts = i2c_get_clientdata(client);
	int ret;

	if (down_timeout(&ts->reset_sem,
	    msecs_to_jiffies(MXM_IRQ_RESET_TIMEOUT)) != 0) {
		dev_err(&ts->client->dev, "irq reset timeout\n");
		goto exit;
	}

	mutex_lock(&ts->i2c_mutex);
	ret = reset_power(ts);
	mutex_unlock(&ts->i2c_mutex);
	if (ret)
		goto exit;

	dev_info(dev, "hw reset occured\n");
exit:
	return count;
}

static int sreset(struct data *ts)
{
	u16 cmd_buf[] = {MXM_CMD_ID_RESET_SYSTEM, MXM_ZERO_SIZE_CMD};
	u16 rpt_buf[10], rpt_len;
	int ret;

	/* Report should be of length + 1 < 10 */
	rpt_len = sizeof(rpt_buf) / sizeof(u16);

	ret = rbcmd_send_receive(ts, cmd_buf, 2, MXM_RPT_ID_SYS_STATUS,
				 rpt_buf, &rpt_len, 3 * HZ);
	if (ret)
		dev_err(&ts->client->dev, "Failed to do soft reset.");
	return ret;
}

static ssize_t sreset_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct data *ts = i2c_get_clientdata(client);

	sreset(ts);

	return count;
}

static ssize_t fw_update_store(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct data *ts = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&ts->fw_mutex);
	if (sysfs_streq(buf, fw_update_mode[MXM_FW_UPDATE_DEFAULT])) {
		ts->fw_update_mode = MXM_FW_UPDATE_DEFAULT;
	} else if (sysfs_streq(buf, fw_update_mode[MXM_FW_UPDATE_FORCE])) {
		ts->fw_update_mode = MXM_FW_UPDATE_FORCE;
	} else {
		dev_err(dev, "Invalid argument: %s\n", buf);
		ret = -EINVAL;
		goto end;

	}
	dev_info(dev, "firmware update (%s)\n",
			fw_update_mode[ts->fw_update_mode]);
	validate_fw(ts);
	ret = count;

end:
	ts->fw_update_mode = MXM_FW_UPDATE_DEFAULT;
	mutex_unlock(&ts->fw_mutex);
	return ret;
}

static ssize_t irq_count_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct data *ts = i2c_get_clientdata(client);

	return snprintf(buf, PAGE_SIZE, "%u\n", ts->irq_count);
}

static ssize_t irq_count_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct data *ts = i2c_get_clientdata(client);

	ts->irq_count = 0;
	return count;
}

static ssize_t dflt_cfg_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct data *ts = i2c_get_clientdata(client);

	return snprintf(buf, PAGE_SIZE, "%u 0x%x 0x%x\n",
			ts->pdata->defaults_allow,
			ts->pdata->default_config_id,
			ts->pdata->default_chip_id);
}

static ssize_t dflt_cfg_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct data *ts = i2c_get_clientdata(client);

	(void) sscanf(buf, "%u 0x%x 0x%x", &ts->pdata->defaults_allow,
		&ts->pdata->default_config_id, &ts->pdata->default_chip_id);
	return count;
}

static ssize_t panel_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct data *ts = i2c_get_clientdata(client);

	return snprintf(buf, PAGE_SIZE, "%u %u %u %u %u %u\n",
			ts->pdata->panel_margin_xl, ts->pdata->panel_margin_xh,
			ts->pdata->panel_margin_yl, ts->pdata->panel_margin_yh,
			ts->pdata->lcd_x, ts->pdata->lcd_y);
}

static ssize_t panel_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct data *ts = i2c_get_clientdata(client);

	(void) sscanf(buf, "%u %u %u %u %u %u", &ts->pdata->panel_margin_xl,
		&ts->pdata->panel_margin_xh, &ts->pdata->panel_margin_yl,
		&ts->pdata->panel_margin_yh, &ts->pdata->lcd_x,
		&ts->pdata->lcd_y);
	return count;
}

static ssize_t fw_ver_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct data *ts = i2c_get_clientdata(client);

	int ret, count = 0;
	u16 cmd_buf[2];
	u16 rpt_buf[100], rpt_len;
	u16 cst_info_addr = 0;

	/* Both reports should be of length + 1 < 100 */
	rpt_len = sizeof(rpt_buf) / sizeof(u16);

	/* Read firmware version */
	cmd_buf[0] = MXM_CMD_ID_GET_FW_VERSION;
	cmd_buf[1] = MXM_ZERO_SIZE_CMD;

	ret = rbcmd_send_receive(ts, cmd_buf, 2, MXM_RPT_ID_FW_VERSION,
				 rpt_buf, &rpt_len, HZ/4);

	if (ret)
		goto err_fw_ver_show;

	ts->chip_id = BYTEH(rpt_buf[4]);
	count += snprintf(buf, PAGE_SIZE, "fw_ver (%u.%u.%u) " \
					"chip_id (0x%02X)\n",
					BYTEH(rpt_buf[3]),
					BYTEL(rpt_buf[3]),
					rpt_buf[5],
					ts->chip_id);

	/* Read touch configuration */
	cmd_buf[0] = MXM_CMD_ID_GET_CFG_INF;
	cmd_buf[1] = MXM_ZERO_SIZE_CMD;

	rpt_len = sizeof(rpt_buf) / sizeof(u16);
	ret = rbcmd_send_receive(ts, cmd_buf, 2, MXM_RPT_ID_CFG_INF,
				 rpt_buf, &rpt_len, HZ/4);

	if (ret) {
		dev_err(dev, "Failed to receive chip config\n");
		goto err_fw_ver_show;
	}

	ts->config_id = rpt_buf[3];

	count += snprintf(buf + count, PAGE_SIZE, "config_id (0x%04X) ",
					ts->config_id);
	switch (ts->chip_id) {
	case 0x55:
	case 0x57:
		cst_info_addr = 42;
		break;
	case 0x72:
	case 0x73:
	case 0x74:
	case 0x75:
	case 0x76:
	case 0x78:
		cst_info_addr = 58;
		break;
	default:
		break;
	}
	if (cst_info_addr != 0)
		count += snprintf(buf + count, PAGE_SIZE,
				  "customer_info[1:0] (0x%04X, 0x%04X)\n",
				  rpt_buf[cst_info_addr + 1],
				  rpt_buf[cst_info_addr]);

err_fw_ver_show:
	return count;
}

static ssize_t chip_id_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct data *ts = i2c_get_clientdata(client);

	return snprintf(buf, PAGE_SIZE, "0x%02X\n", ts->chip_id);
}

static ssize_t config_id_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct data *ts = i2c_get_clientdata(client);

	return snprintf(buf, PAGE_SIZE, "0x%04X\n", ts->config_id);
}

static ssize_t driver_ver_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "3.3.2.2: October 29, 2013\n");
}

static ssize_t command_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct data *ts = i2c_get_clientdata(client);
	u16 buffer[MXM_CMD_LEN_MAX];
	char scan_buf[5];
	int i, ret;

	count--; /* ignore carriage return */
	if (count % 4) {
		dev_err(dev, "words not properly defined");
		return -EINVAL;
	}
	scan_buf[4] = '\0';
	for (i = 0; i < count; i += 4) {
		memcpy(scan_buf, &buf[i], 4);
		if (sscanf(scan_buf, "%4hx", &buffer[i / 4]) != 1) {
			dev_err(dev, "bad word (%s)", scan_buf);
			return -EINVAL;
		}
	}
	ret = cmd_send(ts, buffer, count / 4);
	if (ret)
		dev_err(dev, "MTP command failed");
	return ++count;
}

static ssize_t report_read(struct file *file, struct kobject *kobj,
	struct bin_attribute *attr, char *buf, loff_t off, size_t count)
{
	struct i2c_client *client = kobj_to_i2c_client(kobj);
	struct data *ts = i2c_get_clientdata(client);
	int printed, i, offset = 0, payload;
	int full_packet;
	int num_term_char;

	if (get_report(ts, 0xFFFF, 0xFFFFFFFF))
		return 0;

	payload = ts->rx_report_len;
	full_packet = payload;
	num_term_char = 2; /* number of term char */
	if (count < (4 * full_packet + num_term_char))
		return -EIO;
	if (count > (4 * full_packet + num_term_char))
		count = 4 * full_packet + num_term_char;

	for (i = 1; i <= payload; i++) {
		printed = snprintf(&buf[offset], PAGE_SIZE, "%04X\n",
			ts->rx_report[i]);
		if (printed <= 0)
			return -EIO;
		offset += printed - 1;
	}
	snprintf(&buf[offset], PAGE_SIZE, ",\n");
	release_report(ts);

	return count;
}

static int max1187x_set_glove_locked(struct data *ts, int enable)
{
	u16 cmd_buf[] = {MXM_CMD_ID_SET_GLOVE_MODE,
			  MXM_ONE_SIZE_CMD,
			  enable};
	int ret;

	ret = cmd_send_locked(ts, cmd_buf, NWORDS(cmd_buf));
	if (ret)
		dev_err(&ts->client->dev, "Failed to set glove mode");

	return ret;
}

static int max1187x_set_glove(struct data *ts, int enable)
{
	int ret;

	mutex_lock(&ts->i2c_mutex);
	ret = max1187x_set_glove_locked(ts, enable);
	mutex_unlock(&ts->i2c_mutex);

	return ret;
}

static ssize_t glove_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct data *ts = i2c_get_clientdata(client);

	return snprintf(buf, PAGE_SIZE, "%u\n", ts->pdata->glove_enabled);
}

static ssize_t glove_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct data *ts = i2c_get_clientdata(client);
	int ret;

	if (sscanf(buf, "%u", &ts->pdata->glove_enabled) != 1) {
		dev_err(dev, "Invalid (%s)", buf);
		return -EINVAL;
	}

	ret = max1187x_set_glove(ts, ts->pdata->glove_enabled);

	return ret ? ret : strnlen(buf, PAGE_SIZE);
}

static struct device_attribute dev_attrs[] = {
	__ATTR(i2c_reset, S_IWUSR, NULL, i2c_reset_store),
	__ATTR(por, S_IWUSR, NULL, power_on_reset_store),
	__ATTR(sreset, S_IWUSR, NULL, sreset_store),
	__ATTR(fw_update, S_IWUSR, NULL, fw_update_store),
	__ATTR(irq_count, S_IRUGO | S_IWUSR, irq_count_show,
		irq_count_store),
	__ATTR(dflt_cfg, S_IRUGO | S_IWUSR, dflt_cfg_show, dflt_cfg_store),
	__ATTR(panel, S_IRUGO | S_IWUSR, panel_show, panel_store),
	__ATTR(fw_ver, S_IRUGO, fw_ver_show, NULL),
	__ATTR(chip_id, S_IRUGO, chip_id_show, NULL),
	__ATTR(config_id, S_IRUGO, config_id_show, NULL),
	__ATTR(driver_ver, S_IRUGO, driver_ver_show, NULL),
	__ATTR(command, S_IWUSR, NULL, command_store),
	__ATTR(glove, S_IRUGO | S_IWUSR, glove_show, glove_store)
};

static struct bin_attribute dev_attr_report = {
		.attr = {.name = "report", .mode = S_IRUGO},
		.read = report_read };

static int create_sysfs_entries(struct data *ts)
{
	int i, ret = 0;

	for (i = 0; i < ARRAY_SIZE(dev_attrs); i++) {
		ret = device_create_file(&ts->client->dev, &dev_attrs[i]);
		if (ret) {
			for (; i >= 0; --i) {
				device_remove_file(&ts->client->dev,
							&dev_attrs[i]);
				ts->sysfs_created--;
			}
			break;
		}
		ts->sysfs_created++;
	}
	return ret;
}

static void remove_sysfs_entries(struct data *ts)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(dev_attrs); i++)
		if (ts->sysfs_created && ts->sysfs_created--)
			device_remove_file(&ts->client->dev, &dev_attrs[i]);
}

/* Send command to chip */
static int cmd_send_locked(struct data *ts, u16 *buf, u16 len)
{
	int ret;

	memcpy(ts->cmd_buf, buf, len * sizeof(buf[0]));
	ts->cmd_len = len;

	ret = send_mtp_command(ts, ts->cmd_buf, ts->cmd_len);
	if (ts->cmd_buf[0] == MXM_CMD_ID_SET_TOUCH_RPT_MODE)
		ts->is_raw_mode = !ts->cmd_buf[2];
	if (ts->cmd_buf[0] == MXM_CMD_ID_SET_POWER_MODE &&
			(ts->cmd_buf[2] == MXM_DRIVE_MODE ||
			 ts->cmd_buf[2] == MXM_SENSE_MODE))
		ts->is_raw_mode = true;

	if (ret)
		dev_err(&ts->client->dev, "Failed to send command (ret=%d)\n",
									ret);

	return ret;
}

static int cmd_send(struct data *ts, u16 *buf, u16 len)
{
	int ret;

	mutex_lock(&ts->i2c_mutex);
	ret = cmd_send_locked(ts, buf, len);
	mutex_unlock(&ts->i2c_mutex);

	return ret;
}

/*
 * Send command to chip and expect a report with
 * id == rpt_id within timeout time.
 * timeout is measured in jiffies. 1s = HZ jiffies
 */
static int rbcmd_send_receive(struct data *ts, u16 *cmd_buf,
		u16 cmd_len, u16 rpt_id,
		u16 *rpt_buf, u16 *rpt_len, u16 timeout)
{
	int ret;

	dev_dbg(&ts->client->dev, "%s: Enter\n", __func__);
	ret = down_interruptible(&ts->sema_rbcmd);
	if (ret)
		goto err_rbcmd_send_receive_sema_rbcmd;

	ts->rbcmd_report_id = rpt_id;
	ts->rbcmd_rx_report = rpt_buf;
	ts->rbcmd_rx_report_len = rpt_len;
	ts->rbcmd_received = 0;
	ts->rbcmd_waiting = 1;

	ret = cmd_send(ts, cmd_buf, cmd_len);
	if (ret)
		goto err_rbcmd_send_receive_cmd_send;

	ret = wait_event_interruptible_timeout(ts->waitqueue_rbcmd,
			ts->rbcmd_received != 0, timeout);
	if (ret < 0 || !ts->rbcmd_received)
		goto err_rbcmd_send_receive_timeout;

	dev_dbg(&ts->client->dev, "%s: Received report_ID (0x%04X) "\
						"report_len (%d)\n", __func__,
						ts->rbcmd_report_id,
						*ts->rbcmd_rx_report_len);

	ts->rbcmd_waiting = 0;
	up(&ts->sema_rbcmd);

	dev_dbg(&ts->client->dev, "%s: Exit\n", __func__);
	return 0;

err_rbcmd_send_receive_timeout:
	dev_dbg(&ts->client->dev, "%s: Timed out waiting for "\
						"report_ID (0x%04X)\n",
						__func__, ts->rbcmd_report_id);
err_rbcmd_send_receive_cmd_send:
	ts->rbcmd_waiting = 0;
	up(&ts->sema_rbcmd);
err_rbcmd_send_receive_sema_rbcmd:
	dev_dbg(&ts->client->dev, "%s: Exit\n", __func__);
	return -ERESTARTSYS;
}

static int read_chip_data(struct data *ts)
{
	int ret;
	u16 loopcounter;
	u16 cmd_buf[2];
	u16 rpt_buf[100], rpt_len;

	/* Both reports should be of length + 1 < 100 */
	rpt_len = sizeof(rpt_buf) / sizeof(u16);

	/* Read firmware version */
	cmd_buf[0] = MXM_CMD_ID_GET_FW_VERSION;
	cmd_buf[1] = MXM_ZERO_SIZE_CMD;

	loopcounter = 0;
	ret = -1;
	while (loopcounter < MXM_FW_RETRIES_MAX && ret) {
		rpt_len = sizeof(rpt_buf) / sizeof(u16);
		ret = rbcmd_send_receive(ts, cmd_buf, 2,
				MXM_RPT_ID_FW_VERSION,
				rpt_buf, &rpt_len, HZ/4);
		loopcounter++;
	}

	if (ret) {
		dev_err(&ts->client->dev, "Failed to receive fw version\n");
		goto err_read_chip_data;
	}

	ts->chip_id = BYTEH(rpt_buf[4]);
	dev_info(&ts->client->dev, "(INIT): fw_ver (%u.%u.%u) " \
					"chip_id (0x%02X)\n",
					BYTEH(rpt_buf[3]),
					BYTEL(rpt_buf[3]),
					rpt_buf[5],
					ts->chip_id);

	/* Read touch configuration */
	cmd_buf[0] = MXM_CMD_ID_GET_CFG_INF;
	cmd_buf[1] = MXM_ZERO_SIZE_CMD;

	loopcounter = 0;
	ret = -1;
	while (loopcounter < MXM_FW_RETRIES_MAX && ret) {
		rpt_len = sizeof(rpt_buf) / sizeof(u16);
		ret = rbcmd_send_receive(ts, cmd_buf, 2,
				MXM_RPT_ID_CFG_INF,
				rpt_buf, &rpt_len, HZ/4);
		loopcounter++;
	}

	if (ret) {
		dev_err(&ts->client->dev, "Failed to receive chip config\n");
		goto err_read_chip_data;
	}

	ts->config_id = rpt_buf[3];

	dev_info(&ts->client->dev, "(INIT): config_id (0x%04X)\n",
					ts->config_id);

	return 0;

err_read_chip_data:
	return ret;
}

static int device_fw_load(struct data *ts, const struct firmware *fw,
	u16 fw_index)
{
	struct device *dev = &ts->client->dev;
	u16 filesize, file_codesize, loopcounter;
	u16 file_crc16_1, file_crc16_2, local_crc16;
	int chip_crc16_1 = -1, chip_crc16_2 = -1, ret;

	filesize = fw->size;
	file_codesize = fw->size;

	file_crc16_1 = crc16(0, fw->data, file_codesize);

	loopcounter = 0;
	do {
		ret = bootloader_enter(ts);
		if (!ret)
			ret = bootloader_get_crc(ts, &local_crc16,
				0, file_codesize, MXM_BL_WR_DELAY_MS);
		if (!ret)
			chip_crc16_1 = local_crc16;
		ret = bootloader_exit(ts);
		loopcounter++;
	} while (loopcounter < MXM_FW_RETRIES_MAX && chip_crc16_1 == -1);

	dev_info(dev, "(INIT): file_crc16_1 = 0x%04x, chip_crc16_1 = 0x%04x\n",
			file_crc16_1, chip_crc16_1);

	if (ts->fw_update_mode == MXM_FW_UPDATE_FORCE ||
	    file_crc16_1 != chip_crc16_1) {
		loopcounter = 0;
		file_crc16_2 = crc16(0, fw->data, filesize);

		while (loopcounter < MXM_FW_RETRIES_MAX && file_crc16_2
				!= chip_crc16_2) {
			dev_info(dev, "(INIT): Reprogramming chip." \
					"Attempt %d", loopcounter+1);
			ret = bootloader_enter(ts);
			if (!ret)
				ret = bootloader_erase_flash(ts);
			if (!ret)
				ret = bootloader_set_byte_mode(ts);
			if (!ret)
				ret = bootloader_write_flash(ts, fw->data,
					filesize);
			if (!ret)
				ret = bootloader_get_crc(ts, &local_crc16,
					0, filesize, MXM_BL_WR_DELAY_MS);
			if (!ret)
				chip_crc16_2 = local_crc16;
			dev_info(dev, "(INIT): file_crc16_2 = 0x%04x, " \
					"chip_crc16_2 = 0x%04x\n",
					file_crc16_2, chip_crc16_2);
			ret = bootloader_exit(ts);
			loopcounter++;
		}

		if (file_crc16_2 != chip_crc16_2)
			return -EAGAIN;
	}

	loopcounter = 0;
	do {
		ret = bootloader_exit(ts);
		loopcounter++;
	} while (loopcounter < MXM_FW_RETRIES_MAX && ret);

	if (ret)
		return -EIO;

	return 0;
}

static void validate_fw(struct data *ts)
{
	struct device *dev = &ts->client->dev;
	const struct firmware *fw;
	u16 config_id, chip_id;
	int i, ret;
	u16 cmd_buf[3];
	char tmp[MXM_FW_FILENAME_MAX];
	char filename[MXM_FW_FILENAME_MAX];

	if (ts->fw_update_mode == MXM_FW_UPDATE_FORCE) {
		if (ts->chip_id != ts->pdata->default_chip_id
		    || ts->config_id != ts->pdata->default_config_id) {
			dev_warn(dev, "*** Overwrites chip/config id " \
				 "[0x%02x 0x%04x] => [0x%02x 0x%04x]",
				 ts->chip_id, ts->config_id,
				 ts->pdata->default_chip_id,
				 ts->pdata->default_config_id);
		}
		ts->chip_id = 0;
		ts->config_id = 0;
		goto set_id;
	}

	ret = read_chip_data(ts);
	if (ret && !ts->pdata->defaults_allow) {
		dev_err(dev, "Firmware is not responsive " \
				"and default update is disabled\n");
		return;
	}

set_id:
	if (ts->chip_id)
		chip_id = ts->chip_id;
	else
		chip_id = ts->pdata->default_chip_id;

	if (ts->config_id)
		config_id = ts->config_id;
	else
		config_id = ts->pdata->default_config_id;

	/* fw_name should have "%" for chip_id, and "%%" for config_id */
	snprintf(tmp, MXM_FW_FILENAME_MAX, ts->pdata->fw_name, chip_id);
	snprintf(filename, MXM_FW_FILENAME_MAX, tmp, config_id);
	dev_info(dev, "(INIT): Firmware file (%s)", filename);

	ret = request_firmware(&fw, filename, &ts->client->dev);
	if (ret || fw == NULL) {
		dev_err(dev, "firmware request failed (ret = %d, fwptr = %p)",
			ret, fw);
		return;
	}

	mutex_lock(&ts->i2c_mutex);
	disable_irq(ts->client->irq);
	if (device_fw_load(ts, fw, i)) {
		release_firmware(fw);
		dev_err(dev, "firmware download failed");
		enable_irq(ts->client->irq);
		mutex_unlock(&ts->i2c_mutex);
		return;
	}

	release_firmware(fw);
	dev_info(dev, "(INIT): firmware okay\n");
	enable_irq(ts->client->irq);
	mutex_unlock(&ts->i2c_mutex);
	ret = read_chip_data(ts);

	cmd_buf[0] = MXM_CMD_ID_SET_TOUCH_RPT_MODE;
	cmd_buf[1] = MXM_ONE_SIZE_CMD;
	cmd_buf[2] = MXM_TOUCH_REPORT_MODE_EXT;
	ret = cmd_send(ts, cmd_buf, NWORDS(cmd_buf));
	if (ret) {
		dev_err(dev, "Failed to set up touch report mode");
		return;
	}

	max1187x_set_glove(ts, ts->pdata->glove_enabled);
}

static int regulator_handler(struct regulator *regulator,
				struct device *dev,
				const char *func_str,
				const char *reg_str,
				int sw)
{
	int rc, enabled;

	if (IS_ERR_OR_NULL(regulator)) {
		rc = regulator ? PTR_ERR(regulator) : -EINVAL;
		dev_err(dev, "%s: regulator '%s' invalid",
			func_str ? func_str : "?",
			reg_str ? reg_str : "?");
		goto exit;
	}

	if (sw)
		rc = regulator_enable(regulator);
	else
		rc = regulator_disable(regulator);
	if (rc) {
		enabled = regulator_is_enabled(regulator);
		dev_warn(dev,
			"%s: regulator '%s' status is %d",
			func_str ? func_str : "?",
			reg_str ? reg_str : "?",
			enabled);
		if ((!enabled && !sw) || (enabled > 0 && sw))
			rc = 0;
	}
exit:
	return rc;
}

static int vreg_suspend(struct data *ts, bool enable)
{
	struct device *dev = &ts->client->dev;
	int rc = 0;

	if (IS_ERR(ts->vreg_touch_vdd)) {
		dev_err(dev, "vreg_touch_vdd is not initialized\n");
		rc = -ENODEV;
		goto exit;
	}

	if (enable)
		rc = regulator_set_optimum_mode(ts->vreg_touch_vdd,
							MXM_LPM_UA_LOAD);
	else
		rc = regulator_set_optimum_mode(ts->vreg_touch_vdd,
							MXM_HPM_UA_LOAD);

	if (rc < 0) {
		dev_err(dev, "%s: vdd: set mode (%s) failed, rc=%d\n",
				__func__, (enable ? "LPM" : "HPM"), rc);
		goto exit;
	} else {
		dev_dbg(dev, "%s: vdd: set mode (%s) ok, new mode=%d\n",
			__func__, (enable ? "LPM" : "HPM"), rc);
		rc = 0;
	}
exit:
	return rc;
}

static int vreg_configure(struct data *ts, bool enable)
{
	struct device *dev = &ts->client->dev;
	int rc = 0;

	if (enable) {
		ts->vreg_touch_vdd = regulator_get(dev,
						ts->pdata->vdd_supply_name);
		if (IS_ERR(ts->vreg_touch_vdd)) {
			dev_err(dev, "%s: get vdd failed\n", __func__);
			rc = -ENODEV;
			goto err_ret;
		}
		rc = regulator_set_voltage(ts->vreg_touch_vdd,
					MXM_VREG_MAX_UV, MXM_VREG_MAX_UV);
		if (rc) {
			dev_err(dev, "%s: set voltage vdd failed, rc=%d\n",
								__func__, rc);
			goto err_put_vdd;
		}
		rc = regulator_handler(ts->vreg_touch_vdd, dev,
				__func__, ts->pdata->vdd_supply_name, 1);
		if (rc)
			goto err_put_vdd;
		rc = vreg_suspend(ts, false);
		if (rc) {
			dev_err(dev, "%s: set vdd mode failed, rc=%d\n",
							__func__, rc);
			goto err_disable_vdd;
		}
		dev_dbg(dev, "%s: set touch_vdd ON\n", __func__);
	} else {
		if (!IS_ERR(ts->vreg_touch_vdd)) {
			rc = regulator_set_voltage(ts->vreg_touch_vdd,
					0, MXM_VREG_MAX_UV);
			if (rc)
				dev_err(dev, "%s: set voltage vdd failed, " \
						"rc=%d\n", __func__, rc);
			regulator_handler(ts->vreg_touch_vdd,
				dev, __func__, ts->pdata->vdd_supply_name, 0);
			regulator_put(ts->vreg_touch_vdd);
			dev_dbg(dev, "%s: set touch_vdd OFF\n", __func__);
		}
	}
	return rc;
err_disable_vdd:
	regulator_handler(ts->vreg_touch_vdd, dev, __func__,
					ts->pdata->vdd_supply_name, 0);
err_put_vdd:
	regulator_put(ts->vreg_touch_vdd);
err_ret:
	return rc;
}

/* #ifdef CONFIG_OF */
static struct max1187x_pdata *max1187x_get_platdata_dt(struct device *dev)
{
	struct max1187x_pdata *pdata = NULL;
	struct device_node *devnode = dev->of_node;

	if (!devnode)
		return NULL;

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		dev_err(dev, "Failed to allocate memory for pdata\n");
		return NULL;
	}

	/* Parse touch_vdd_supply_name */
	if (of_property_read_string(devnode, "touch_vdd-supply_name",
				(const char **)&pdata->vdd_supply_name)) {
		dev_err(dev, "Failed to get property: touch_vdd-supply_name\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse gpio_tirq */
	if (of_property_read_u32(devnode, "gpio_tirq", &pdata->gpio_tirq)) {
		dev_err(dev, "Failed to get property: gpio_tirq\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse gpio_reset */
	if (of_property_read_u32(devnode, "gpio_reset", &pdata->gpio_reset))
		dev_info(dev, "unused gpio_reset should be set to zero\n");

	/* Parse reset_l2h (Low to Hi) */
	if (of_property_read_u32(devnode, "reset_l2h", &pdata->reset_l2h))
		dev_info(dev, "unused reset_l2h should be set to zero\n");

	/* Parse enable_resume_por */
	if (of_property_read_u32(devnode, "enable_resume_por",
		&pdata->enable_resume_por))
		dev_info(dev, "unused enable_resume_por should be set to zero\n");

	/* Parse defaults_allow */
	if (of_property_read_u32(devnode, "defaults_allow",
		&pdata->defaults_allow)) {
		dev_err(dev, "Failed to get property: defaults_allow\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse default_config_id */
	if (of_property_read_u32(devnode, "default_config_id",
		&pdata->default_config_id)) {
		dev_err(dev, "Failed to get property: default_config_id\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse default_chip_id */
	if (of_property_read_u32(devnode, "default_chip_id",
		&pdata->default_chip_id)) {
		dev_err(dev, "Failed to get property: default_chip_id\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse touch_vdd_supply_name */
	if (of_property_read_string(devnode, "fw_name",
				(const char **)&pdata->fw_name)) {
		dev_err(dev, "Failed to get property: fw_name\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse i2c_words */
	if (of_property_read_u32(devnode, "i2c_words", &pdata->i2c_words)) {
		dev_err(dev, "Failed to get property: i2c_words\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse coordinate_settings */
	if (of_property_read_u32(devnode, "coordinate_settings",
		&pdata->coordinate_settings)) {
		dev_err(dev, "Failed to get property: coordinate_settings\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse panel_margin_xl */
	if (of_property_read_u32(devnode, "panel_margin_xl",
		&pdata->panel_margin_xl)) {
		dev_err(dev, "Failed to get property: panel_margin_xl\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse lcd_x */
	if (of_property_read_u32(devnode, "lcd_x", &pdata->lcd_x)) {
		dev_err(dev, "Failed to get property: lcd_x\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse panel_margin_xh */
	if (of_property_read_u32(devnode, "panel_margin_xh",
		&pdata->panel_margin_xh)) {
		dev_err(dev, "Failed to get property: panel_margin_xh\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse panel_margin_yl */
	if (of_property_read_u32(devnode, "panel_margin_yl",
		&pdata->panel_margin_yl)) {
		dev_err(dev, "Failed to get property: panel_margin_yl\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse lcd_y */
	if (of_property_read_u32(devnode, "lcd_y", &pdata->lcd_y)) {
		dev_err(dev, "Failed to get property: lcd_y\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse panel_margin_yh */
	if (of_property_read_u32(devnode, "panel_margin_yh",
		&pdata->panel_margin_yh)) {
		dev_err(dev, "Failed to get property: panel_margin_yh\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse row_count */
	if (of_property_read_u32(devnode, "num_sensor_x",
		&pdata->num_sensor_x)) {
		dev_err(dev, "Failed to get property: num_sensor_x\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse num_sensor_y */
	if (of_property_read_u32(devnode, "num_sensor_y",
		&pdata->num_sensor_y)) {
		dev_err(dev, "Failed to get property: num_sensor_y\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse button_code0 */
	if (of_property_read_u32(devnode, "button_code0",
		&pdata->button_code0)) {
		dev_err(dev, "Failed to get property: button_code0\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse button_code1 */
	if (of_property_read_u32(devnode, "button_code1",
		&pdata->button_code1)) {
		dev_err(dev, "Failed to get property: button_code1\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse button_code2 */
	if (of_property_read_u32(devnode, "button_code2",
		&pdata->button_code2)) {
		dev_err(dev, "Failed to get property: button_code2\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse button_code3 */
	if (of_property_read_u32(devnode, "button_code3",
		&pdata->button_code3)) {
		dev_err(dev, "Failed to get property: button_code3\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse touch_pressure_enabled */
	if (of_property_read_u32(devnode, "touch_pressure_enabled",
		&pdata->pressure_enabled)) {
		dev_err(dev, "Failed to get property: touch_pressure_enabled\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse touch_size_enabled */
	if (of_property_read_u32(devnode, "touch_size_enabled",
		&pdata->size_enabled)) {
		dev_err(dev, "Failed to get property: touch_size_enabled\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse touch_orientation_enabled */
	if (of_property_read_u32(devnode, "touch_orientation_enabled",
		&pdata->orientation_enabled)) {
		dev_err(dev, "Failed to get property: touch_orientation_enabled\n");
		goto err_max1187x_get_platdata_dt;
	}

	/* Parse glove_enabled */
	if (of_property_read_u32(devnode, "glove_enabled",
		&pdata->glove_enabled)) {
		dev_warn(dev, "no glove_enabled config\n");
	}

	/* Parse report_pen_as_finger */
	if (of_property_read_u32(devnode, "report_pen_as_finger",
		&pdata->report_pen_as_finger)) {
		dev_warn(dev, "no report_pen_as_finger config\n");
	}

	/* Parse wakeup_gesture_support */
	if (of_property_read_u32(devnode, "wakeup_gesture_support",
		&pdata->wakeup_gesture_support)) {
		dev_warn(dev, "Failed to get property: wakeup_gesture_support\n");
	}

	/* Parse wakeup_gesture_timeout */
	if (of_property_read_u32(devnode, "wakeup_gesture_timeout",
		&pdata->wakeup_gesture_timeout)) {
		dev_warn(dev, "Failed to get property: wakeup_gesture_timeout\n");
	}

	return pdata;

err_max1187x_get_platdata_dt:
	devm_kfree(dev, pdata);
	return NULL;
}

static int validate_pdata(struct device *dev, struct max1187x_pdata *pdata)
{
	if (!pdata) {
		dev_err(dev, "Platform data not found!\n");
		goto err_validate_pdata;
	}

	if (!pdata->gpio_tirq) {
		dev_err(dev, "gpio_tirq (%u) not defined!\n", pdata->gpio_tirq);
		goto err_validate_pdata;
	}

	if (pdata->lcd_x < MXM_LCD_X_MIN || pdata->lcd_x > MXM_LCD_SIZE_MAX) {
		dev_err(dev, "lcd_x (%u) out of range!\n", pdata->lcd_x);
		goto err_validate_pdata;
	}

	if (pdata->lcd_y < MXM_LCD_Y_MIN || pdata->lcd_y > MXM_LCD_SIZE_MAX) {
		dev_err(dev, "lcd_y (%u) out of range!\n", pdata->lcd_y);
		goto err_validate_pdata;
	}

	if (!pdata->num_sensor_x || pdata->num_sensor_x > MXM_NUM_SENSOR_MAX) {
		dev_err(dev, "num_sensor_x (%u) out of range!\n",
				pdata->num_sensor_x);
		goto err_validate_pdata;
	}

	if (!pdata->num_sensor_y || pdata->num_sensor_y > MXM_NUM_SENSOR_MAX) {
		dev_err(dev, "num_sensor_y (%u) out of range!\n",
				pdata->num_sensor_y);
		goto err_validate_pdata;
	}

	return 0;

err_validate_pdata:
	return -ENXIO;
}

static int max1187x_chip_init(struct data *ts, bool enable)
{
	int  ret;

	if (enable) {
		ret = gpio_request(ts->pdata->gpio_tirq, "max1187x_tirq");
		if (ret) {
			dev_err(&ts->client->dev,
				"GPIO request failed for max1187x_tirq (%d)\n",
				ts->pdata->gpio_tirq);
			return -EIO;
		}
		ret = gpio_direction_input(ts->pdata->gpio_tirq);
		if (ret) {
			dev_err(&ts->client->dev,
				"GPIO set input direction failed for " \
				"max1187x_tirq (%d)\n", ts->pdata->gpio_tirq);
			gpio_free(ts->pdata->gpio_tirq);
			return -EIO;
		}
	} else {
		gpio_free(ts->pdata->gpio_tirq);
	}

	return 0;
}

static void max1187x_gpio_init(struct data *ts, bool enable)
{
	int  ret;

	if (!ts->pdata->gpio_reset)
		return;

	if (enable) {
		ret = gpio_request(ts->pdata->gpio_reset,
							"max1187x_gpio_reset");
		if (ret) {
			dev_err(&ts->client->dev,
				"GPIO request failed for gpio reset (%d)\n",
				ts->pdata->gpio_reset);
			return;
		}
		ret = gpio_direction_output(ts->pdata->gpio_reset,
					    ts->pdata->reset_l2h ? 1 : 0);
		if (ret) {
			dev_err(&ts->client->dev,
				"GPIO set output direction failed for " \
				"max1187x_gpio_reset (%d)\n",
				ts->pdata->gpio_reset);
			gpio_free(ts->pdata->gpio_reset);
		}
	} else {
		ret = gpio_direction_output(ts->pdata->gpio_reset,
					    ts->pdata->reset_l2h ? 0 : 1);
		if (ret)
			dev_err(&ts->client->dev,
				"GPIO set output direction failed for " \
				"max1187x_gpio_reset (%d)\n",
				ts->pdata->gpio_reset);
		usleep_range(MXM_WAIT_MIN_US, MXM_WAIT_MAX_US);
	}
}

static int probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct data *ts;
	struct max1187x_pdata *pdata;
	struct kobject *parent;
	int ret;

	dev_info(dev, "(INIT): Start");

	/* if I2C functionality is not present we are done */
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(dev, "I2C core driver does not support  " \
						"I2C functionality");
		ret = -ENXIO;
		goto err_device_init;
	}
	dev_info(dev, "(INIT): I2C functionality OK");

	/* allocate control block; nothing more to do if we can't */
	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (!ts) {
		dev_err(dev, "Failed to allocate control block memory");
		ret = -ENOMEM;
		goto err_device_init;
	}

	/* Get platform data */
	pdata = dev_get_platdata(dev);
	/* If pdata is missing, try to get pdata from device tree (dts) */
	if (!pdata)
		pdata = max1187x_get_platdata_dt(dev);

	/* Validate if pdata values are okay */
	ret = validate_pdata(dev, pdata);
	if (ret < 0)
		goto err_device_init_pdata;
	dev_info(dev, "(INIT): Platform data OK");

	ts->pdata = pdata;
	ts->client = client;
	i2c_set_clientdata(client, ts);
	mutex_init(&ts->fw_mutex);
	mutex_init(&ts->i2c_mutex);
	mutex_init(&ts->report_mutex);
	sema_init(&ts->report_sem, 1);
	sema_init(&ts->sema_rbcmd, 1);
	sema_init(&ts->reset_sem, 1);
	ts->button0 = 0;
	ts->button1 = 0;
	ts->button2 = 0;
	ts->button3 = 0;

	init_waitqueue_head(&ts->waitqueue_all);
	init_waitqueue_head(&ts->waitqueue_rbcmd);

	dev_info(dev, "(INIT): Memory allocation OK");

	ret = vreg_configure(ts, true);
	if (ret < 0) {
			dev_err(dev, "Failed to configure VREG");
			goto err_device_init_vreg;
	}
	dev_info(&ts->client->dev, "(INIT): VREG OK");
	msleep(MXM_PWR_SET_WAIT_MS);

	/* Initialize GPIO pins */
	if (max1187x_chip_init(ts, true) < 0) {
		ret = -EIO;
		goto err_device_init_gpio;
	}
	dev_info(dev, "(INIT): chip init OK");

	max1187x_gpio_init(ts, true);

	/* allocate and register touch device */
	ts->input_dev = input_allocate_device();
	if (!ts->input_dev) {
		dev_err(dev, "Failed to allocate touch input device");
		ret = -ENOMEM;
		goto err_device_init_inputdev;
	}
	snprintf(ts->phys, sizeof(ts->phys), "%s/input0",
			dev_name(dev));
	ts->input_dev->name = MAX1187X_TOUCH;
	ts->input_dev->phys = ts->phys;
	ts->input_dev->id.bustype = BUS_I2C;
	__set_bit(EV_SYN, ts->input_dev->evbit);
	__set_bit(EV_ABS, ts->input_dev->evbit);
	__set_bit(EV_KEY, ts->input_dev->evbit);

	input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID,
			0, MXM_TOUCH_COUNT_MAX, 0, 0);
	ts->list_finger_ids = 0;
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X,
		ts->pdata->panel_margin_xl,
		ts->pdata->panel_margin_xl + ts->pdata->lcd_x - 1, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y,
		ts->pdata->panel_margin_yl,
		ts->pdata->panel_margin_yl + ts->pdata->lcd_y - 1, 0, 0);
	if (ts->pdata->pressure_enabled)
		input_set_abs_params(ts->input_dev, ABS_MT_PRESSURE,
				0, MXM_PRESSURE_SQRT_MAX, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOOL_TYPE,
			0, ts->pdata->report_pen_as_finger ? MT_TOOL_FINGER :
			MT_TOOL_PEN, 0, 0);
	if (ts->pdata->size_enabled) {
		input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR,
				0, ts->pdata->lcd_x + ts->pdata->lcd_y, 0, 0);
		input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MINOR,
				0, ts->pdata->lcd_x + ts->pdata->lcd_y, 0, 0);
	}
	if (ts->pdata->orientation_enabled)
		input_set_abs_params(ts->input_dev, ABS_MT_ORIENTATION,
				-90, 90, 0, 0);
	if (ts->pdata->button_code0 != KEY_RESERVED)
		set_bit(pdata->button_code0, ts->input_dev->keybit);
	if (ts->pdata->button_code1 != KEY_RESERVED)
		set_bit(pdata->button_code1, ts->input_dev->keybit);
	if (ts->pdata->button_code2 != KEY_RESERVED)
		set_bit(pdata->button_code2, ts->input_dev->keybit);
	if (ts->pdata->button_code3 != KEY_RESERVED)
		set_bit(pdata->button_code3, ts->input_dev->keybit);

	ret = input_register_device(ts->input_dev);
	if (ret) {
		dev_err(dev, "Failed to register touch input device");
		ret = -EPERM;
		input_free_device(ts->input_dev);
		goto err_device_init_inputdev;
	}
	dev_info(dev, "(INIT): Input touch device OK");

	if (ts->pdata->wakeup_gesture_support) {
		ts->evdt_node = evdt_initialize(dev, ts->input_dev,
							MXM_WAKEUP_GESTURE);
		if (!ts->evdt_node) {
			pr_info("No wakeup_gesture dt\n");
		} else {
			dev_info(dev, "(INIT): Touch Wakeup Feature OK\n");
			device_init_wakeup(&client->dev, 1);
			device_set_wakeup_enable(&client->dev, false);
		}
	}

	/* Setup IRQ and handler */
	ret = request_threaded_irq(client->irq,
			irq_handler_hard, irq_handler_soft,
			IRQF_TRIGGER_FALLING | IRQF_ONESHOT, client->name, ts);
	if (ret) {
		dev_err(dev, "Failed to setup IRQ handler");
		ret = -EIO;
		goto err_device_init_irq;
	}
	dev_info(&ts->client->dev, "(INIT): IRQ handler OK");

	/* reset chip to clear any missed reports */
	sreset(ts);

	/* collect controller ID and configuration ID data from firmware   */
	ret = read_chip_data(ts);
	if (ret)
		dev_warn(dev, "No firmware response (%d)", ret);

	/* configure suspend/resume */
#ifdef CONFIG_FB
	ts->fb_notif.notifier_call = fb_notifier_callback;
	ret = fb_register_client(&ts->fb_notif);
	if (ret) {
		dev_err(dev, "Unable to register fb_notifier");
	} else {
		INIT_WORK(&ts->notify_resume, notify_resume);
		INIT_WORK(&ts->notify_suspend, notify_suspend);
	}
#endif
	ts->is_suspended = false;
	ts->pm_suspended = false;
	ts->irq_on_suspend = false;

	dev_info(dev, "(INIT): suspend/resume registration OK");

	/* set up debug interface */
	ret = create_sysfs_entries(ts);
	if (ret) {
		dev_err(dev, "failed to create sysfs file");
		goto err_device_init_irq;
	}

	if (device_create_bin_file(&client->dev, &dev_attr_report) < 0) {
		dev_err(dev, "failed to create sysfs file [report]");
		goto err_device_init_sysfs_remove_group;
	}
	ts->sysfs_created++;

	/* create symlink */
	parent = ts->input_dev->dev.kobj.parent;
	ret = sysfs_create_link(parent, &client->dev.kobj, MAX1187X_NAME);
	if (ret)
		dev_err(dev, "sysfs_create_link error\n");

	dev_info(dev, "(INIT): Done\n");
	return 0;

err_device_init_sysfs_remove_group:
	remove_sysfs_entries(ts);
err_device_init_irq:
#ifdef CONFIG_FB
	fb_unregister_client(&ts->fb_notif);
#endif
	input_unregister_device(ts->input_dev);
err_device_init_inputdev:
	max1187x_chip_init(ts, false);
	max1187x_gpio_init(ts, false);
err_device_init_gpio:
	vreg_configure(ts, false);
err_device_init_vreg:
err_device_init_pdata:
	kzfree(ts);
err_device_init:
	return ret;
}

static void shutdown(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct data *ts = i2c_get_clientdata(client);

	if (ts == NULL)
		return;

	propagate_report(ts, -1, NULL);

	if (ts->pdata->wakeup_gesture_support)
		device_init_wakeup(&client->dev, 0);

	remove_sysfs_entries(ts);

	if (ts->sysfs_created && ts->sysfs_created--)
		device_remove_bin_file(&client->dev, &dev_attr_report);

#ifdef CONFIG_FB
	if (fb_unregister_client(&ts->fb_notif))
		dev_err(dev, "Error occurred while unregistering fb_notifier.");
	cancel_work_sync(&ts->notify_resume);
	cancel_work_sync(&ts->notify_suspend);
#endif

	if (client->irq)
		free_irq(client->irq, ts);

	sysfs_remove_link(ts->input_dev->dev.kobj.parent,
						MAX1187X_NAME);
	input_unregister_device(ts->input_dev);

	(void) max1187x_chip_init(ts, false);
	max1187x_gpio_init(ts, false);
	vreg_configure(ts, false);
	kzfree(ts);
}

/* Commands */
static void process_rbcmd(struct data *ts)
{
	dev_dbg(&ts->client->dev, "%s: Enter\n", __func__);
	if (!ts->rbcmd_waiting)
		goto exit;
	if (ts->rbcmd_report_id != ts->rx_report[1])
		goto exit;
	if (ts->rx_report_len > *ts->rbcmd_rx_report_len) {
		dev_dbg(&ts->client->dev, "%s: Report length mismatch: "\
						"received (%d) "
						"expected (%d) words, "
						"header (0x%04X)\n",
						__func__, ts->rx_report_len,
						*ts->rbcmd_rx_report_len,
						ts->rbcmd_report_id);
		goto exit;
	}

	ts->rbcmd_received = 1;
	memcpy(ts->rbcmd_rx_report, ts->rx_report, (ts->rx_report_len + 1)<<1);
	*ts->rbcmd_rx_report_len = ts->rx_report_len;
	wake_up_interruptible(&ts->waitqueue_rbcmd);

exit:
	dev_dbg(&ts->client->dev, "%s: Exit\n", __func__);
	return;
}

static int combine_multipacketreport(struct data *ts, u16 *report)
{
	u16 packet_header = report[0];
	u8 packet_seq_num = BYTEH(packet_header);
	u8 packet_size = BYTEL(packet_header);
	u16 total_packets, this_packet_num, offset;
	static u16 packet_seq_combined;

	if (packet_seq_num == MXM_ONE_PACKET_RPT) {
		memcpy(ts->rx_report, report, (packet_size + 1) << 1);
		ts->rx_report_len = packet_size;
		packet_seq_combined = 1;
		return 0;
	}

	total_packets = HI_NIBBLE(packet_seq_num);
	this_packet_num = LO_NIBBLE(packet_seq_num);

	if (this_packet_num == 1) {
		if (report[1] == MXM_RPT_ID_TOUCH_RAW_IMAGE) {
			ts->rx_report_len = report[2] + 2;
			packet_seq_combined = 1;
			memcpy(ts->rx_report, report, (packet_size + 1) << 1);
			return -EAGAIN;
		} else {
			return -EIO;
		}
	} else if (this_packet_num == packet_seq_combined + 1) {
		packet_seq_combined++;
		offset = (this_packet_num - 1) * MXM_RPT_MAX_WORDS_PER_PKT + 1;
		memcpy(ts->rx_report + offset, report + 1, packet_size << 1);
		if (total_packets == this_packet_num)
			return 0;
		else
			return -EIO;
	}
	return -EIO;
}

static void propagate_report(struct data *ts, int status, u16 *report)
{
	int i, ret;

	down(&ts->report_sem);
	mutex_lock(&ts->report_mutex);

	if (report) {
		ret = combine_multipacketreport(ts, report);
		if (ret) {
			up(&ts->report_sem);
			mutex_unlock(&ts->report_mutex);
			return;
		}
	}
	process_rbcmd(ts);

	for (i = 0; i < MXM_REPORT_READERS_MAX; i++) {
		if (!status) {
			if (ts->report_readers[i].report_id == 0xFFFF
				|| (ts->rx_report[1]
				&& ts->report_readers[i].report_id
				== ts->rx_report[1])) {
				up(&ts->report_readers[i].sem);
				ts->report_readers[i].reports_passed++;
				ts->report_readers_outstanding++;
			}
		} else {
			if (ts->report_readers[i].report_id) {
				ts->report_readers[i].status = status;
				up(&ts->report_readers[i].sem);
			}
		}
	}
	if (!ts->report_readers_outstanding)
		up(&ts->report_sem);
	mutex_unlock(&ts->report_mutex);
}

static int get_report(struct data *ts, u16 report_id, ulong timeout)
{
	int i, ret, status;

	mutex_lock(&ts->report_mutex);
	for (i = 0; i < MXM_REPORT_READERS_MAX; i++)
		if (!ts->report_readers[i].report_id)
			break;
	if (i == MXM_REPORT_READERS_MAX) {
		mutex_unlock(&ts->report_mutex);
		dev_err(&ts->client->dev, "maximum readers reached");
		return -EBUSY;
	}
	ts->report_readers[i].report_id = report_id;
	sema_init(&ts->report_readers[i].sem, 1);
	down(&ts->report_readers[i].sem);
	ts->report_readers[i].status = 0;
	ts->report_readers[i].reports_passed = 0;
	mutex_unlock(&ts->report_mutex);

	if (timeout == 0xFFFFFFFF)
		ret = down_interruptible(&ts->report_readers[i].sem);
	else
		ret = down_timeout(&ts->report_readers[i].sem,
			(timeout * HZ) / 1000);

	mutex_lock(&ts->report_mutex);
	if (ret && ts->report_readers[i].reports_passed > 0)
		if (--ts->report_readers_outstanding == 0)
			up(&ts->report_sem);
	status = ts->report_readers[i].status;
	ts->report_readers[i].report_id = 0;
	mutex_unlock(&ts->report_mutex);

	return (!status) ? ret : status;
}

static void release_report(struct data *ts)
{
	mutex_lock(&ts->report_mutex);
	if (--ts->report_readers_outstanding == 0)
		up(&ts->report_sem);
	mutex_unlock(&ts->report_mutex);
}

static void set_suspend_mode(struct data *ts)
{
	u16 cmd_buf[] = {MXM_CMD_ID_SET_POWER_MODE,
			 MXM_ONE_SIZE_CMD,
			 MXM_PWR_SLEEP_MODE};
	int ret;

	dev_info(&ts->client->dev, "%s\n", __func__);

	if (down_timeout(&ts->reset_sem,
	    msecs_to_jiffies(MXM_IRQ_RESET_TIMEOUT)) != 0) {
		dev_err(&ts->client->dev, "irq reset timeout\n");
		return;
	}

	ts->is_suspended = true;

	if (device_may_wakeup(&ts->client->dev)) {
		cmd_buf[2] = MXM_WAKEUP_MODE;
		ts->ew_timeout = jiffies - 1;
	}

	ret = cmd_send(ts, cmd_buf, NWORDS(cmd_buf));
	if (ret)
		dev_err(&ts->client->dev, "Failed to set sleep mode");

	if (cmd_buf[2] != MXM_WAKEUP_MODE) {
		usleep_range(MXM_WAIT_MIN_US, MXM_WAIT_MAX_US);
		vreg_suspend(ts, true);
	}

	up(&ts->reset_sem);

	dev_dbg(&ts->client->dev, "%s: Exit\n", __func__);
	return;
}

static void set_resume_mode(struct data *ts)
{
	dev_info(&ts->client->dev, "%s\n", __func__);

	if (down_timeout(&ts->reset_sem,
	    msecs_to_jiffies(MXM_IRQ_RESET_TIMEOUT)) != 0) {
		dev_err(&ts->client->dev, "irq reset timeout\n");
		return;
	}

	vreg_suspend(ts, false);
	usleep_range(MXM_WAIT_MIN_US, MXM_WAIT_MAX_US);

	if (ts->pdata->enable_resume_por) {
		disable_irq(ts->client->irq);
		reset_power(ts);
		enable_irq(ts->client->irq);
	} else {
		mutex_lock(&ts->i2c_mutex);
		reinit_chip_settings(ts);
		mutex_unlock(&ts->i2c_mutex);
	}

	ts->is_suspended = false;

	if (!ts->pdata->enable_resume_por)
		up(&ts->reset_sem);

	dev_dbg(&ts->client->dev, "%s: Exit\n", __func__);

	return;
}

#ifdef CONFIG_FB
static void notify_resume(struct work_struct *work)
{
	struct data *ts  = container_of(work, struct data, notify_resume);

	if (ts->is_suspended)
		set_resume_mode(ts);
}

static void notify_suspend(struct work_struct *work)
{
	struct data *ts  = container_of(work, struct data, notify_suspend);

	if (!ts->is_suspended)
		set_suspend_mode(ts);
}

static int fb_notifier_callback(struct notifier_block *self,
				unsigned long event, void *data)
{
	struct fb_event *evdata = data;
	int *blank;
	struct data *ts = container_of(self, struct data, fb_notif);

	if (evdata && evdata->data && event == FB_EVENT_BLANK && ts &&
			ts->client) {
		blank = evdata->data;
		if (*blank != FB_BLANK_UNBLANK) {
			dev_dbg(&ts->client->dev, "FB_BLANK_BLANKED\n");
			cancel_work_sync(&ts->notify_resume);
			cancel_work_sync(&ts->notify_suspend);
			schedule_work(&ts->notify_suspend);
		} else if (*blank == FB_BLANK_UNBLANK) {
			dev_dbg(&ts->client->dev, "FB_BLANK_UNBLANK\n");
			cancel_work_sync(&ts->notify_suspend);
			cancel_work_sync(&ts->notify_resume);
			schedule_work(&ts->notify_resume);
		}
	}
	return 0;
}
#endif

static int suspend_noirq(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct data *ts = i2c_get_clientdata(client);

	dev_dbg(&ts->client->dev, "%s: Enter\n", __func__);

	if (ts->irq_on_suspend && device_may_wakeup(&client->dev)) {
		dev_warn(&ts->client->dev, "Need to resume\n");
		return -EBUSY;
	}

	dev_dbg(&ts->client->dev, "%s: Exit\n", __func__);

	return 0;
}

static int suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct data *ts = i2c_get_clientdata(client);

	dev_dbg(&ts->client->dev, "%s: Enter\n", __func__);

	if (!ts->is_suspended)
		set_suspend_mode(ts);

	disable_irq(client->irq);

	if (device_may_wakeup(&client->dev)) {
		enable_irq_wake(client->irq);
		dev_dbg(&ts->client->dev, "enable irq wake\n");
	}

	ts->irq_on_suspend = false;
	ts->pm_suspended = true;

	dev_dbg(&ts->client->dev, "%s: Exit\n", __func__);

	return 0;
}

static int resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct data *ts = i2c_get_clientdata(client);
	int ret;

	dev_dbg(&ts->client->dev, "%s: Enter\n", __func__);

	if (device_may_wakeup(&client->dev)) {
		disable_irq_wake(client->irq);
		dev_dbg(&ts->client->dev, "disable irq wake\n");
	}

	if (ts->irq_on_suspend) {
		ts->irq_on_suspend = false;
		dev_dbg(&ts->client->dev, "process irq from suspend\n");
		ret = read_mtp_report(ts, ts->rx_packet);
		if (!ret)
			process_report(ts, ts->rx_packet);
	}

	enable_irq(client->irq);

	ts->pm_suspended = false;

	dev_dbg(&ts->client->dev, "%s: Exit\n", __func__);

	return 0;
}

static const struct dev_pm_ops max1187x_pm_ops = {
	.resume = resume,
	.suspend = suspend,
	.suspend_noirq = suspend_noirq,
};

static int bootloader_read_status_reg(struct data *ts, const u8 byteL,
	const u8 byteH)
{
	u8 buffer[] = { MXM_BL_STATUS_ADDR_L, MXM_BL_STATUS_ADDR_H }, i;

	for (i = 0; i < MXM_BL_RD_STATUS_RETRY; i++) {
		if (i2c_tx_bytes(ts, buffer, 2) != 2) {
			dev_err(&ts->client->dev, "TX fail");
			return -EIO;
		}
		if (i2c_rx_bytes(ts, buffer, 2) != 2) {
			dev_err(&ts->client->dev, "RX fail");
			return -EIO;
		}
		if (buffer[0] == byteL && buffer[1] == byteH)
			break;
	}
	if (i == MXM_BL_RD_STATUS_RETRY) {
		dev_err(&ts->client->dev, "Unexpected status => %02X%02X vs" \
			"%02X%02X", buffer[0], buffer[1], byteL, byteH);
		return -EIO;
	}

	return 0;
}

static int bootloader_write_status_reg(struct data *ts, const u8 byteL,
	const u8 byteH)
{
	u8 buffer[] = { MXM_BL_STATUS_ADDR_L, MXM_BL_STATUS_ADDR_H,
							byteL, byteH };

	if (i2c_tx_bytes(ts, buffer, 4) != 4) {
		dev_err(&ts->client->dev, "TX fail");
		return -EIO;
	}
	return 0;
}

static int bootloader_rxtx_complete(struct data *ts)
{
	return bootloader_write_status_reg(ts, MXM_BL_RXTX_COMPLETE_L,
				MXM_BL_RXTX_COMPLETE_H);
}

static int bootloader_read_data_reg(struct data *ts, u8 *byteL, u8 *byteH)
{
	u8 buffer[] = { MXM_BL_DATA_ADDR_L, MXM_BL_DATA_ADDR_H, 0x00, 0x00 };

	if (i2c_tx_bytes(ts, buffer, 2) != 2) {
		dev_err(&ts->client->dev, "TX fail");
		return -EIO;
	}
	if (i2c_rx_bytes(ts, buffer, 4) != 4) {
		dev_err(&ts->client->dev, "RX fail");
		return -EIO;
	}
	if (buffer[2] != MXM_BL_STATUS_READY_L &&
		buffer[3] != MXM_BL_STATUS_READY_H) {
		dev_err(&ts->client->dev, "Status is not ready");
		return -EIO;
	}

	*byteL = buffer[0];
	*byteH = buffer[1];
	return bootloader_rxtx_complete(ts);
}

static int bootloader_write_data_reg(struct data *ts, const u8 byteL,
	const u8 byteH)
{
	u8 buffer[6] = { MXM_BL_DATA_ADDR_L, MXM_BL_DATA_ADDR_H, byteL, byteH,
			MXM_BL_RXTX_COMPLETE_L, MXM_BL_RXTX_COMPLETE_H };

	if (bootloader_read_status_reg(ts, MXM_BL_STATUS_READY_L,
		MXM_BL_STATUS_READY_H) < 0) {
		dev_err(&ts->client->dev, "read status register fail");
		return -EIO;
	}
	if (i2c_tx_bytes(ts, buffer, 6) != 6) {
		dev_err(&ts->client->dev, "TX fail");
		return -EIO;
	}
	return 0;
}

static int bootloader_rxtx(struct data *ts, u8 *byteL, u8 *byteH,
	const int tx)
{
	if (tx > 0) {
		if (bootloader_write_data_reg(ts, *byteL, *byteH) < 0) {
			dev_err(&ts->client->dev, "write data register fail");
			return -EIO;
		}
		return 0;
	}

	if (bootloader_read_data_reg(ts, byteL, byteH) < 0) {
		dev_err(&ts->client->dev, "read data register fail");
		return -EIO;
	}
	return 0;
}

static int bootloader_get_cmd_conf(struct data *ts, int retries)
{
	u8 byteL, byteH;

	do {
		if (bootloader_read_data_reg(ts, &byteL, &byteH) >= 0) {
			if (byteH == MXM_BL_DATA_READY_H &&
				byteL == MXM_BL_DATA_READY_L)
				return 0;
		}
		retries--;
	} while (retries > 0);

	return -EIO;
}

static int bootloader_write_buffer(struct data *ts, u8 *buffer, int size)
{
	u8 byteH = MXM_BL_WR_START_ADDR;
	int k;

	for (k = 0; k < size; k++) {
		if (bootloader_rxtx(ts, &buffer[k], &byteH, 1) < 0) {
			dev_err(&ts->client->dev, "bootloader RX-TX fail");
			return -EIO;
		}
	}
	return 0;
}

static int bootloader_enter(struct data *ts)
{
	int i;
	u16 enter[3][2] = { { MXM_BL_ENTER_SEQ_L, MXM_BL_ENTER_SEQ_H1 },
			    { MXM_BL_ENTER_SEQ_L, MXM_BL_ENTER_SEQ_H2 },
			    { MXM_BL_ENTER_SEQ_L, MXM_BL_ENTER_SEQ_H3 }
			  };

	for (i = 0; i < MXM_BL_ENTER_RETRY; i++) {
		if (i2c_tx_words(ts, enter[i], 2) != 2) {
			dev_err(&ts->client->dev, "Failed to enter bootloader");
			return -EIO;
		}
	}

	if (bootloader_get_cmd_conf(ts, MXM_BL_ENTER_CONF_RETRY) < 0) {
		dev_err(&ts->client->dev, "Failed to enter bootloader mode");
		return -EIO;
	}
	return 0;
}

static int bootloader_exit(struct data *ts)
{
	int i;
	u16 exit[3][2] = { { MXM_BL_EXIT_SEQ_L, MXM_BL_EXIT_SEQ_H1 },
			   { MXM_BL_EXIT_SEQ_L, MXM_BL_EXIT_SEQ_H2 },
			   { MXM_BL_EXIT_SEQ_L, MXM_BL_EXIT_SEQ_H3 }
			 };

	for (i = 0; i < MXM_BL_EXIT_RETRY; i++) {
		if (i2c_tx_words(ts, exit[i], 2) != 2) {
			dev_err(&ts->client->dev, "Failed to exit bootloader");
			return -EIO;
		}
	}

	return 0;
}

static int bootloader_get_crc(struct data *ts, u16 *crc16,
		u16 addr, u16 len, u16 delay)
{
	u8 crc_command[] = { MXM_BL_GET_CRC_L, MXM_BL_GET_CRC_H,
			     BYTEL(addr), BYTEH(addr), BYTEL(len), BYTEH(len)
			   };
	u8 byteL = 0, byteH = 0;
	u16 rx_crc16 = 0;

	if (bootloader_write_buffer(ts, crc_command, 6) < 0) {
		dev_err(&ts->client->dev, "write buffer fail");
		return -EIO;
	}
	msleep(delay);

	/* reads low 8bits (crcL) */
	if (bootloader_rxtx(ts, &byteL, &byteH, 0) < 0) {
		dev_err(&ts->client->dev,
			"Failed to read low byte of crc response!");
		return -EIO;
	}
	rx_crc16 = (u16) byteL;

	/* reads high 8bits (crcH) */
	if (bootloader_rxtx(ts, &byteL, &byteH, 0) < 0) {
		dev_err(&ts->client->dev,
			"Failed to read high byte of crc response!");
		return -EIO;
	}
	rx_crc16 = (u16)(byteL << 8) | rx_crc16;

	if (bootloader_get_cmd_conf(ts, MXM_BL_CRC_GET_RETRY) < 0) {
		dev_err(&ts->client->dev, "CRC get failed!");
		return -EIO;
	}
	*crc16 = rx_crc16;

	return 0;
}

static int bootloader_set_byte_mode(struct data *ts)
{
	u8 buffer[2] = {MXM_BL_SET_BYTE_MODE_L, MXM_BL_SET_BYTE_MODE_H};

	if (bootloader_write_buffer(ts, buffer, 2) < 0) {
		dev_err(&ts->client->dev, "write buffer fail");
		return -EIO;
	}
	if (bootloader_get_cmd_conf(ts, 10) < 0) {
		dev_err(&ts->client->dev, "command confirm fail");
		return -EIO;
	}
	return 0;
}

static int bootloader_erase_flash(struct data *ts)
{
	u8 byteL = MXM_BL_ERASE_FLASH_L, byteH = MXM_BL_ERASE_FLASH_H;
	int i, verify = 0;

	if (bootloader_rxtx(ts, &byteL, &byteH, 1) < 0) {
		dev_err(&ts->client->dev, "bootloader RX-TX fail");
		return -EIO;
	}

	for (i = 0; i < MXM_BL_ERASE_CONF_RETRY; i++) {
		msleep(MXM_BL_ERASE_DELAY_MS);

		if (bootloader_get_cmd_conf(ts, 0) < 0)
			continue;

		verify = 1;
		break;
	}

	if (verify != 1) {
		dev_err(&ts->client->dev, "Flash Erase failed");
		return -EIO;
	}

	return 0;
}

static int bootloader_write_flash(struct data *ts, const u8 *image, u16 length)
{
	struct device *dev = &ts->client->dev;
	u8 buffer[MXM_BL_WR_TX_SZ];
	u8 length_L = length & 0xFF;
	u8 length_H = (length >> 8) & 0xFF;
	u8 command[] = { MXM_BL_WR_FAST_FLASH_L, MXM_BL_WR_FAST_FLASH_H,
			length_H, length_L, MXM_BL_WR_START_ADDR };
	u16 blocks_of_128bytes;
	int i, j;

	if (bootloader_write_buffer(ts, command, 5) < 0) {
		dev_err(dev, "write buffer fail");
		return -EIO;
	}

	blocks_of_128bytes = length >> 7;

	for (i = 0; i < blocks_of_128bytes; i++) {
		for (j = 0; j < MXM_BL_WR_STATUS_RETRY; j++) {
			usleep_range(MXM_WAIT_MIN_US, MXM_WAIT_MAX_US);
			if (!bootloader_read_status_reg(ts,
				MXM_BL_STATUS_READY_L, MXM_BL_STATUS_READY_H))
				break;
		}
		if (j == MXM_BL_WR_STATUS_RETRY) {
			dev_err(dev, "Failed to read Status register!");
			return -EIO;
		}

		buffer[0] = (!(i % 2)) ? MXM_BL_WR_DBUF0_ADDR :
					 MXM_BL_WR_DBUF1_ADDR;
		buffer[1] = 0x00;
		memcpy(buffer + 2, image + i * MXM_BL_WR_BLK_SIZE,
						MXM_BL_WR_BLK_SIZE);

		if (i2c_tx_bytes(ts, buffer, MXM_BL_WR_TX_SZ) !=
						MXM_BL_WR_TX_SZ) {
			dev_err(dev, "Failed to write data (%d)", i);
			return -EIO;
		}
		if (bootloader_rxtx_complete(ts) < 0) {
			dev_err(dev, "Transfer failure (%d)", i);
			return -EIO;
		}
	}

	usleep_range(MXM_BL_WR_MIN_US, MXM_BL_WR_MAX_US);
	if (bootloader_get_cmd_conf(ts, MXM_BL_WR_CONF_RETRY) < 0) {
		dev_err(dev, "Flash programming failed");
		return -EIO;
	}
	return 0;
}

/****************************************
 *
 * Standard Driver Structures/Functions
 *
 ****************************************/
static const struct i2c_device_id id[] = { { MAX1187X_NAME, 0 }, { } };

MODULE_DEVICE_TABLE(i2c, id);

static struct of_device_id max1187x_dt_match[] = {
	{ .compatible = "maxim,max1187x_tsc" },	{ } };

static struct i2c_driver driver = {
		.probe = probe,
		.shutdown = shutdown,
		.id_table = id,
		.driver = {
			.name = MAX1187X_NAME,
			.owner	= THIS_MODULE,
			.of_match_table = max1187x_dt_match,
			.pm = &max1187x_pm_ops,
		},
};

#ifndef MODULE
void __devinit max1187x_init_async(void *unused, async_cookie_t cookie)
{
	int rc;

	rc = i2c_add_driver(&driver);
	if (rc != 0)
		pr_err("Maxim I2C registration failed rc = %d\n", rc);
}
#endif

static int __devinit max1187x_init(void)
{
#ifdef MODULE
	return i2c_add_driver(&driver);
#else
	async_schedule(max1187x_init_async, NULL);
	return 0;
#endif
}

static void __exit max1187x_exit(void)
{
	i2c_del_driver(&driver);
}

module_init(max1187x_init);
module_exit(max1187x_exit);

MODULE_AUTHOR("Maxim Integrated Products, Inc.");
MODULE_DESCRIPTION("MAX1187X Touchscreen Driver");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("3.3.2");
