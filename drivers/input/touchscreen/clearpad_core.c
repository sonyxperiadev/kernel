/* linux/drivers/input/touchscreen/clearpad_core.c
 *
 * Copyright (C) 2010 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 - 2013 Sony Mobile Communications AB.
 *
 * Author: Courtney Cavin <courtney.cavin@sonyericsson.com>
 *         Yusuke Yoshimura <Yusuke.Yoshimura@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/clearpad.h>
#include <linux/input/evgen_helper.h>
#include <mach/gpio.h>
#include <linux/ctype.h>
#include <linux/firmware.h>
#include <linux/slab.h>
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#endif
#include <linux/sched.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#ifdef CONFIG_ARM
#include <asm/mach-types.h>
#endif

#define SYNAPTICS_CLEARPAD_VENDOR		0x1
#define SYNAPTICS_MAX_N_FINGERS			10
#define SYNAPTICS_FINGER_DATA_SIZE		5
#define SYNAPTICS_DEVICE_STATUS			0x13
#define SYNAPTICS_MAX_Z_VALUE			255
#define SYNAPTICS_MAX_W_VALUE			15
#define SYNAPTICS_PDT_START			0xEF
#define SYNAPTICS_CHARGER_CONTROL		0xF0
#define SYNAPTICS_SIZE_OF_FD			6
#define SYNAPTICS_PAGE_SELECT_OFFSET		0xFF
#define SYNAPTICS_SUPPORTED_PAGE_NUM		0x04
#define SYNAPTICS_MAX_INTERRUPT_SOURCE_COUNT	0x7
#define SYNAPTICS_STRING_LENGTH			128
#define SYNAPTICS_RETRY_NUM_OF_INITIAL_CHECK	2
#define SYNAPTICS_FINGER_OFF(n, x) \
	((((n) / 4) + !!(n % 4)) + SYNAPTICS_FINGER_DATA_SIZE * (x))
#define SYNAPTICS_REG_MAX \
	SYNAPTICS_FINGER_OFF(SYNAPTICS_MAX_N_FINGERS, SYNAPTICS_MAX_N_FINGERS)
#define SYNAPTICS_FINGER_STATE(buf, i) ((buf[i / 4] >> ((i % 4) * 2)) & 0x03)
#define HWTEST_SIZE_OF_COMMAND_PREFIX		2
#define HWTEST_SIZE_OF_ONE_DIMENSION		1
#define HWTEST_SIZE_OF_ONE_HIGH_RX		3
#define HWTEST_SIZE_OF_TX_TO_TX_SHORT(x)	(((x) + 7) / 8)
#define SYNAPTICS_WATCHDOG_POLL_DEFAULT_INTERVAL HZ

#define SYN_ADDRESS(th, func, type, addr) ((th)->pdt[func].base[type] + (addr))
#define SYN_PAGE(th, func) ((th)->pdt[func].page)
#define SYNSET(...)  __VA_ARGS__
#define SYNX(x) SYN_##x
#define SYNY(y) SYN_TYPE_##y
#define SYNF(x, y, a) SYNSET(SYNX(x), SYNY(y), a)

#define DEVICE_STATUS_UNCONFIGURED_RESET_OCCURRED	0x81
#define DEVICE_STATUS_UNCONFIGURED_DEVICE_FAILURE	0x83
#define DEVICE_STATUS_DEVICE_FAILURE			0x03
#define DEVICE_COMMAND_RESET				0x01
#define DEVICE_CONTROL_SLEEP_MODE			0x03
#define DEVICE_CONTROL_SLEEP_MODE_NORMAL_OPERATION	0x00
#define DEVICE_CONTROL_SLEEP_MODE_SENSOR_SLEEP		0x01
#define DEVICE_CONTROL_CONFIGURED			0x80
#define DEVICE_PEN_SUPPORTED				0x01
#define PEN_DETECT_INTERRUPT				0x01
#define PEN_DETECT_INT_ENABLE				0x01
#define XY_REPORTING_MODE				0x07
#define XY_HAS_LPWG					0x10
#define XY_REPORTING_MODE_REDUCED_REPORTING_MODE	0x01
#define XY_REPORTING_MODE_WAKEUP_GESTURE_MODE		0x04
#define XY_LPWG_STATUS_DOUBLE_TAP_DETECTED		0x01
#define XY_LPWG_STATUS_SWIPE_DETECTED			0x02
#define XY_LPWG_STATUS_TWO_SWIPE_DETECTED		0x04
#define FLASH_CONTROL_WRITE_FIRMWARE_BLOCK		0x02
#define FLASH_CONTROL_ERASE_ALL				0x03
#define FLASH_CONTROL_WRITE_CONFIGURATION_BLOCK		0x06
#define FLASH_CONTROL_ERASE_CONFIGURATION		0x07
#define FLASH_CONTROL_ENABLE_FLASH_PROGRAMMING		0x0f
#define FLASH_CONTROL_PROGRAM_ENABLED			0x80
#define ANALOG_CONTROL_NO_AUTO_CAL			0x10
#define ANALOG_CONTROL_CBC_CAPACITANCE			0x07
#define ANALOG_CONTROL_CBC_POLARITY			0x08
#define ANALOG_CONTROL_CBC_XMTR_CARRIER_SELECT		0x10
#define ANALOG_CONTROL_NO_SIGNALCLARITY			0x01
#define ANALOG_QUERY_NUM_OF_ELECTRODES			0x3F
#define ANALOG_COMMAND_GET_IMAGE			0x04
#define ANALOG_COMMAND_GET_REPORT			0x01
#define ANALOG_COMMAND_FORCE_CAL			0x02
#define ANALOG_COMMAND_FORCE_UPDATE			0x04

#define TOUCHCMD_CHARGER_MODE_START	"cmstart"
#define TOUCHCMD_CHARGER_MODE_END	"cmend"

#define LOGx(this, LEVEL, X, ...)				\
do {								\
	dev_dbg(&this->pdev->dev, LEVEL "%s: %d: " X,		\
		       __func__, __LINE__, ## __VA_ARGS__);	\
} while (0)
#define LOG_STAT(this, X, ...) LOGx(this, "stat: ", X, ## __VA_ARGS__)
#define LOG_EVENT(this, X, ...) LOGx(this, "event: ", X, ## __VA_ARGS__)
#define LOG_CHECK(this, X, ...) LOGx(this, "check: ", X, ## __VA_ARGS__)
#define LOG_VERBOSE(this, X, ...) LOGx(this, "verbose: ", X, ## __VA_ARGS__)

#define LOCK(this)			\
do {					\
	LOG_CHECK(this, "(will lock)\n");	\
	mutex_lock(&this->lock);	\
	LOG_CHECK(this, "LOCKED\n");	\
} while (0)
#define UNLOCK(this)			\
do {					\
	LOG_CHECK(this, "UNLOCK\n");	\
	mutex_unlock(&this->lock);	\
} while (0)

enum synaptics_state {
	SYN_STATE_INIT,
	SYN_STATE_RUNNING,
	SYN_STATE_FLASH_IMAGE_SET,
	SYN_STATE_FLASH_ENABLE,
	SYN_STATE_FLASH_PROGRAM,
	SYN_STATE_FLASH_DATA,
	SYN_STATE_FLASH_CONFIG,
	SYN_STATE_FLASH_DISABLE,
	SYN_STATE_WAIT_FOR_INT,
	SYN_STATE_IRQ_DISABLED,
	SYN_STATE_DISABLED,
};

static const char * const state_name[] = {
	[SYN_STATE_INIT]		= "init",
	[SYN_STATE_RUNNING]		= "running",
	[SYN_STATE_FLASH_IMAGE_SET]	= "flash image set",
	[SYN_STATE_FLASH_ENABLE]	= "flash enable",
	[SYN_STATE_FLASH_PROGRAM]	= "flash program",
	[SYN_STATE_FLASH_DATA]		= "flash data",
	[SYN_STATE_FLASH_CONFIG]	= "flash config",
	[SYN_STATE_FLASH_DISABLE]	= "flash disable",
	[SYN_STATE_WAIT_FOR_INT]	= "wait for interrupt",
	[SYN_STATE_IRQ_DISABLED]	= "irq disabled",
	[SYN_STATE_DISABLED]		= "disabled",
};

enum synaptics_task {
	SYN_TASK_NONE,
	SYN_TASK_NO_SUSPEND,
};

enum synaptics_chip {
	SYN_CHIP_3000	= 0x33,
	SYN_CHIP_3200	= 0x35,
	SYN_CHIP_3400	= 0x36,
};

enum synaptics_bootloader_id {
	SYN_BOOTLOADER_ID_0 = 0,
	SYN_BOOTLOADER_ID_1 = 1,
};

static const char * const task_name[] = {
	[SYN_TASK_NONE]		= "none",
	[SYN_TASK_NO_SUSPEND]	= "executing task",
};

enum synaptics_active {
	SYN_ACTIVE_POWER	= (1 << 0),
	SYN_STANDBY		= (1 << 1),
	SYN_STANDBY_AFTER_TASK	= (1 << 2),
};

enum synaptics_clearpad_function {
	SYN_F01_RMI,
	SYN_F05_ANALOG,
	SYN_F11_2D,
	SYN_F34_FLASH,
	SYN_F54_ANALOG,
	SYN_F55_SENSOR,
	SYN_N_FUNCTIONS,
};

static const u8 function_value[] = {
	[SYN_F01_RMI]		= 0x01,
	[SYN_F05_ANALOG]	= 0x05,
	[SYN_F11_2D]		= 0x11,
	[SYN_F34_FLASH]		= 0x34,
	[SYN_F54_ANALOG]	= 0x54,
	[SYN_F55_SENSOR]	= 0x55,
	[SYN_N_FUNCTIONS]	= 0x00,
};

enum synaptics_clearpad_reg_type {
	SYN_TYPE_DATA,
	SYN_TYPE_CTRL,
	SYN_TYPE_COMMAND,
	SYN_TYPE_QUERY,
	SYN_TYPE_END,
};

static const char * const synaptics_clearpad_flash_status[] = {
	[0] = "Success",
	[1] = "(Reserved)",
	[2] = "Flash Programming Not Enabled/Bad Command",
	[3] = "Invalid Block Number",
	[4] = "Block Not Erased",
	[5] = "Erase Key Incorrect",
	[6] = "Unknown Erase/Program Failure",
	[7] = "Device has been reset",
};

static const char * const synaptics_clearpad_flash_reason[] = {
	[4] = "Configuration CRC Failure",
	[5] = "Firmware CRC Failure",
	[6] = "CRC In Progress",
};

enum synaptics_clearpad_firmware {
	HEADER_SIZE			= 0x100,
	HEADER_VERSION_OFFSET		= 0x07,
	HEADER_FIRMWARE_SIZE_OFFSET	= 0x08,
	HEADER_FIRMWARE_SIZE_SIZE	= 4,
	HEADER_CONFIG_SIZE_OFFSET	= 0x0c,
	HEADER_CONFIG_SIZE_SIZE		= 4,
	HEADER_PRODUCT_ID_OFFSET	= 0x10,
	HEADER_PRODUCT_ID_SIZE		= 10,
	HEADER_CUSTOMER_FAMILY_OFFSET	= 0x1e,
	HEADER_FIRMWARE_REVISION_OFFSET	= 0x1f,
};

static const int synaptics_number_of_fingers[] = {
	[0] = 1,
	[1] = 2,
	[2] = 3,
	[3] = 4,
	[4] = 5,
	[5] = 10,
	[6] = 0,
	[7] = 0,
};

enum synaptics_device_serialization_queries {
	SIZE_OF_DATE_CODE	= 3,
	SIZE_OF_TESTER_ID	= 2,
	SIZE_OF_SERIAL_NUMBER	= 2,
};

enum synaptics_flush_commands {
	SYN_LOAD_START,
	SYN_LOAD_END,
	SYN_FORCE_FLUSH,
	SYN_CONFIG_FLUSH,
};

static const char * const flush_commands[] = {
	[SYN_LOAD_START]	= "load_start",
	[SYN_LOAD_END]		= "load_end",
	[SYN_FORCE_FLUSH]	= "force_flush",
	[SYN_CONFIG_FLUSH]	= "config_flush",
};

enum synaptics_flash_modes {
	SYN_FLASH_MODE_NORMAL = 0,
	SYN_FLASH_MODE_CONFIG = 1,
};

struct synaptics_device_info {
	u8 manufacturer_id;
	u8 product_properties;
	u8 customer_family;
	u8 firmware_revision;
	u8 date[SIZE_OF_DATE_CODE];
	u8 tester_id[SIZE_OF_TESTER_ID];
	u8 serial_number[SIZE_OF_SERIAL_NUMBER];
	u8 product_id[HEADER_PRODUCT_ID_SIZE];
};

struct synaptics_point {
	int id;
	int x;
	int y;
	int wx;
	int wy;
	int z;
	int tool;
};

enum synaptics_tool {
	SYN_TOOL_FINGER	= 0x01,
	SYN_TOOL_PEN	= 0x02,
};

struct synaptics_pointer {
	bool down;
	struct synaptics_funcarea *funcarea;
	struct synaptics_point cur;
};

struct synaptics_function_descriptor {
	u8 number;
	u8 int_count;
	u8 irq_mask;
	u8 page;
	u8 base[SYN_TYPE_END];
};

struct synaptics_flash_block {
	int blocks;
	int length;
	int pos;
	const u8 *data;
};

struct synaptics_flash_image {
	u8 *image;
	size_t size;
	size_t buffer_size; /* allocated buffer size */
	u8 format_version;
	int firmware_size;
	int config_size;
	u8 customer_family;
	u8 firmware_revision;
	u8 product_id[HEADER_PRODUCT_ID_SIZE];
	struct synaptics_flash_block data;
	struct synaptics_flash_block config;
};

struct synaptics_extents {
	int x_min, y_min;
	int x_max, y_max;
	int n_fingers;
};

enum hwtest_data_type {
	HWTEST_NULL,
	HWTEST_U8,
	HWTEST_S8,
	HWTEST_S16,
};

enum f05_command {
	HWTEST_BASELINE = 0x40,
	HWTEST_SIGNAL_DIFF = 0x80,
};

enum f54_command {
	F54_16_IMAGE_REPORT		= 2,
	F54_AUTOSCAN_REPORT		= 3,
	F54_HIGH_RESISTANCE_REPORT	= 4,
	F54_TX_TO_TX_SHORT_REPORT	= 5,
	F54_RX_TO_RX1_REPORT		= 7,
	F54_TX_TO_GROUND_REPORT		= 16,
	F54_RX_TO_RX2_REPORT		= 17,
	F54_RAW_CAPACITANCE_REPORT	= 19,
	F54_RAW_CAP_RX_COUPLING_REPORT	= 20,
};

struct synaptics_clearpad {
	enum   synaptics_state state;
	enum   synaptics_task task;
	enum   synaptics_chip chip;
	struct input_dev *input;
	struct platform_device *pdev;
	struct clearpad_platform_data *pdata;
	struct clearpad_bus_data *bdata;
	struct mutex lock;
	struct synaptics_device_info device_info;
	struct synaptics_funcarea *funcarea;
	struct synaptics_pointer pointer[SYNAPTICS_MAX_N_FINGERS];
	struct synaptics_function_descriptor pdt[SYN_N_FUNCTIONS];
	struct synaptics_flash_image flash;
	struct synaptics_easy_wakeup_config easy_wakeup_config;
	struct evgen_block *evgen_blocks;
#ifdef CONFIG_TOUCHSCREEN_CLEARPAD_RMI_DEV
	struct platform_device *rmi_dev;
#endif
	bool fwdata_available;
	enum synaptics_flash_modes flash_mode;
	struct synaptics_extents extents;
	int active;
	int irq_mask;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
	char fwname[SYNAPTICS_STRING_LENGTH + 1];
	char result_info[SYNAPTICS_STRING_LENGTH + 1];
	wait_queue_head_t task_none_wq;
	bool flash_requested;
	bool charger_mode;
	u8 page_num;
	u8 reg_buf[SYNAPTICS_REG_MAX];
#ifdef CONFIG_DEBUG_FS
	struct dentry *debugfs;
#endif
	bool pen_enabled;
	unsigned long ew_timeout;
	struct delayed_work wd_poll_work;
	int wd_poll_t_jf;
	spinlock_t slock;
	bool dev_busy;
	bool irq_pending;
};

static void synaptics_funcarea_initialize(struct synaptics_clearpad *this);
static void synaptics_clearpad_reset_power(struct synaptics_clearpad *this);

static char *make_string(u8 *array, size_t size)
{
	static char string[SYNAPTICS_STRING_LENGTH + 1];

	memset(string, 0, SYNAPTICS_STRING_LENGTH + 1);
	size = (SYNAPTICS_STRING_LENGTH < size) ?
				SYNAPTICS_STRING_LENGTH : size;
	memcpy(string, array, size);

	return string;
}

static int regs_read(struct synaptics_clearpad *this, u8 reg, u8 *buf, int len)
{
	return this->bdata->read(this->pdev->dev.parent, reg, buf, len);
}

static int regs_write(struct synaptics_clearpad *this, u8 reg, const u8 *buf,
		u8 len)
{
	return this->bdata->write(this->pdev->dev.parent, reg, buf, len);
}

static inline int synaptics_clearpad_page_sel(struct synaptics_clearpad *this,
		u8 page_num)
{
	int rc = 0;

	if (this->page_num != page_num) {
		rc = regs_write(this, SYNAPTICS_PAGE_SELECT_OFFSET,
						&page_num, 1);
		if (!rc)
			this->page_num = page_num;
	}
	return rc;
}

static int synaptics_put(struct synaptics_clearpad *this,
		enum synaptics_clearpad_function func,
		enum synaptics_clearpad_reg_type type,
		u8 addr, u8 val)
{
	int rc;

	rc = synaptics_clearpad_page_sel(this, SYN_PAGE(this, func));
	if (!rc)
		rc = regs_write(this, SYN_ADDRESS(this, func, type, addr),
						&val, 1);
	return rc;
}

static int synaptics_put_bit(struct synaptics_clearpad *this,
		enum synaptics_clearpad_function func,
		enum synaptics_clearpad_reg_type type,
		u8 addr, u8 val, u8 mask)
{
	int rc;
	u8 buf;

	rc = synaptics_clearpad_page_sel(this, SYN_PAGE(this, func));
	if (rc)
		goto err_return;

	rc = regs_read(this, SYN_ADDRESS(this, func, type, addr), &buf, 1);
	if (rc)
		goto err_return;

	buf = (buf & ~mask) | val;
	rc = regs_write(this, SYN_ADDRESS(this, func, type, addr), &buf, 1);
err_return:
	return rc;
}

static int synaptics_write(struct synaptics_clearpad *this,
		enum synaptics_clearpad_function func,
		enum synaptics_clearpad_reg_type type,
		u8 addr, const u8 *buf, u8 len)
{
	int rc;

	rc = synaptics_clearpad_page_sel(this, SYN_PAGE(this, func));
	if (!rc)
		rc = regs_write(this, SYN_ADDRESS(this, func, type, addr),
						buf, len);
	return rc;
}

static int synaptics_read(struct synaptics_clearpad *this,
		enum synaptics_clearpad_function func,
		enum synaptics_clearpad_reg_type type,
		u8 addr, u8 *buf, u8 len)
{
	int rc;

	rc = synaptics_clearpad_page_sel(this, SYN_PAGE(this, func));
	if (!rc)
		rc = regs_read(this, SYN_ADDRESS(this, func, type, addr),
						buf, len);
	return rc;
}

static void synaptics_clearpad_set_irq(struct synaptics_clearpad *this,
					u8 user, bool enable)
{
	int mask;

	LOG_STAT(this, "user_mask 0x%02x user 0x%02x enable %d\n",
		 this->irq_mask, user, enable);

	if (enable)
		mask = this->irq_mask | user;
	else
		mask = this->irq_mask & ~user;

	if (mask && !this->irq_mask) {
		LOG_STAT(this, "enable IRQ (user_mask 0x%02x)\n", mask);
		enable_irq(this->pdata->irq);
	} else if (!mask && this->irq_mask) {
		LOG_STAT(this, "disable IRQ (user_mask 0x%02x)\n", mask);
		disable_irq_nosync(this->pdata->irq);
	} else
		LOG_STAT(this, "no change IRQ (%s)\n",
				mask ? "enable" : "disable");
	this->irq_mask = mask;
}

static int synaptics_clearpad_read_pdt(struct synaptics_clearpad *this)
{
	u8 addr = SYNAPTICS_PDT_START - 1;
	u8 irq_bit = 0;
	u8 page = 0;
	int i, j, k;
	int rc;

	memset(&this->pdt, 0, sizeof(*this->pdt) * SYN_N_FUNCTIONS);
	for (i = 0; i < SYN_N_FUNCTIONS && addr >= SYNAPTICS_SIZE_OF_FD - 1;) {
		struct synaptics_function_descriptor fdes;
		rc = regs_read(this, addr--, &fdes.number, 1);
		if (rc)
			break;
		rc = regs_read(this, addr--, &fdes.int_count, 1);
		if (rc)
			break;
		fdes.int_count &= SYNAPTICS_MAX_INTERRUPT_SOURCE_COUNT;
		for (fdes.irq_mask = 0, j = 0; j < fdes.int_count; j++)
			fdes.irq_mask |= (1 << irq_bit++);
		rc = regs_read(this, addr--, &fdes.base[SYN_TYPE_DATA], 1);
		if (rc)
			break;
		rc = regs_read(this, addr--, &fdes.base[SYN_TYPE_CTRL], 1);
		if (rc)
			break;
		rc = regs_read(this, addr--, &fdes.base[SYN_TYPE_COMMAND], 1);
		if (rc)
			break;
		rc = regs_read(this, addr--, &fdes.base[SYN_TYPE_QUERY], 1);
		if (rc)
			break;
		fdes.page = page;
		LOG_CHECK(this, "F%02x_IRQ_MASK = %02x\n",
			    fdes.number, fdes.irq_mask);
		LOG_CHECK(this, "F%02x_DATA = %02x\n",
			    fdes.number, fdes.base[SYN_TYPE_DATA]);
		LOG_CHECK(this, "F%02x_CTRL = %02x\n",
			    fdes.number, fdes.base[SYN_TYPE_CTRL]);
		LOG_CHECK(this, "F%02x_COMMAND = %02x\n",
			    fdes.number, fdes.base[SYN_TYPE_COMMAND]);
		LOG_CHECK(this, "F%02x_QUERY = %02x\n",
			    fdes.number, fdes.base[SYN_TYPE_QUERY]);
		LOG_CHECK(this, "F%02x Page = %02x\n",
			    fdes.number, fdes.page);
		if (!fdes.number) {
			if (page < SYNAPTICS_SUPPORTED_PAGE_NUM) {
				addr = SYNAPTICS_PDT_START - 1;
				rc = synaptics_clearpad_page_sel(this, ++page);
				if (rc)
					break;
				continue;
			}
			break;
		}
		for (k = 0; k < SYN_N_FUNCTIONS; k++) {
			if (function_value[k] == fdes.number) {
				if (!this->pdt[k].number)
					memcpy(&this->pdt[k], &fdes,
							sizeof(*this->pdt));
				i++;
				break;
			}
		}
	}
	return rc;
}

static int synaptics_clearpad_set_pen(struct synaptics_clearpad *this)
{
	int rc;
	u8 buf;

	rc = synaptics_read(this, SYNF(F11_2D, QUERY, 0x06), &buf, 1);
	if (rc)
		goto exit;
	if (buf & DEVICE_PEN_SUPPORTED) {
		rc = synaptics_put_bit(this, SYNF(F11_2D, CTRL, 0x0A),
			PEN_DETECT_INT_ENABLE,
			PEN_DETECT_INTERRUPT);
		if (rc)
			goto exit;

		if (this->pdt[SYN_F54_ANALOG].number
					== function_value[SYN_F54_ANALOG])
			rc = synaptics_put(this, SYNF(F54_ANALOG, COMMAND,
				0x00), ANALOG_COMMAND_FORCE_UPDATE);
	}
exit:
	return rc;
}

static int synaptics_clearpad_prepare_f11_2d(struct synaptics_clearpad *this)
{
	int rc;
	u8 buf[4];
	enum registers {
		REG_X_LSB,
		REG_X_MSB,
		REG_Y_LSB,
		REG_Y_MSB,
	};

	rc = synaptics_read(this, SYNF(F11_2D, CTRL, 0x06), buf, sizeof(buf));
	if (rc)
		goto exit;

	this->extents.x_min = 0;
	this->extents.y_min = 0;
	this->extents.x_max = (buf[REG_X_LSB] | (buf[REG_X_MSB] << 8));
	this->extents.y_max = (buf[REG_Y_LSB] | (buf[REG_Y_MSB] << 8));

	rc = synaptics_read(this, SYNF(F11_2D, QUERY, 0x01), buf, 1);
	if (rc)
		goto exit;

	this->extents.n_fingers = synaptics_number_of_fingers[buf[0] & 0x07];
	dev_info(&this->pdev->dev, "number of fingers=%d\n",
			this->extents.n_fingers);

	/* enable ABS event interrupts */
	rc = synaptics_put(this, SYNF(F01_RMI, CTRL, 0x01),
			this->pdt[SYN_F11_2D].irq_mask);
	if (rc)
		goto exit;

	if (this->chip == SYN_CHIP_3000) {
		/* set reduced reporting mode */
		rc = synaptics_put_bit(this, SYNF(F11_2D, CTRL, 0x00),
				XY_REPORTING_MODE_REDUCED_REPORTING_MODE,
				XY_REPORTING_MODE);
		if (rc)
			goto exit;
	}

	if (this->pen_enabled)
		rc = synaptics_clearpad_set_pen(this);

exit:
	return rc;
}

static void synaptics_firmware_reset(struct synaptics_clearpad *this)
{
	kfree(this->flash.image);
	memset(&this->flash, 0, sizeof(this->flash));
	this->fwdata_available = false;
	dev_info(&this->pdev->dev, "firmware image has been reset\n");
}

static int synaptics_clearpad_flash(struct synaptics_clearpad *this);

static int synaptics_clearpad_initialize(struct synaptics_clearpad *this)
{
	int rc;
	u8 type[2], buf[2];
	struct synaptics_device_info *info = &this->device_info;

	rc = synaptics_clearpad_read_pdt(this);
	if (rc)
		goto exit;

	/* set device configured bit */
	rc = synaptics_put_bit(this, SYNF(F01_RMI, CTRL, 0x00),
					DEVICE_CONTROL_CONFIGURED,
					DEVICE_CONTROL_CONFIGURED);
	if (rc)
		goto exit;

	/* read synaptics chip type */
	rc = synaptics_read(this, SYNF(F34_FLASH, QUERY, 0x00), type, 2);
	if (rc)
		goto exit;
	this->chip = type[SYN_BOOTLOADER_ID_1];

	/* read device configuration */
	rc = synaptics_read(this, SYNF(F01_RMI, QUERY, 0x00), (u8 *)info, 21);
	if (rc)
		goto exit;
	if (this->chip >= SYN_CHIP_3200) {
		rc = synaptics_read(this, SYNF(F34_FLASH, CTRL, 0x00), buf, 2);
		if (rc)
			goto exit;
		info->customer_family = buf[0];
		info->firmware_revision = buf[1];
	}

	if (this->state != SYN_STATE_RUNNING) {
		dev_info(&this->pdev->dev,
			"device mid %d, prop %d, family 0x%02x, rev 0x%02x\n",
			 info->manufacturer_id, info->product_properties,
			 info->customer_family, info->firmware_revision);
		dev_info(&this->pdev->dev,
			"bl %04d-%02d-%02d, tester %d, s/n %d, id '%s'\n",
			 2000 + info->date[0], info->date[1], info->date[2],
			 ((int)info->tester_id[0] << 8) + info->tester_id[1],
			 ((int)info->serial_number[0] << 8)
			 + info->serial_number[1],
			 make_string(info->product_id, HEADER_PRODUCT_ID_SIZE));
	}

	if (this->flash.image) {
		dev_info(&this->pdev->dev, "force firmware update\n");
		this->state = SYN_STATE_FLASH_IMAGE_SET;
		this->task = SYN_TASK_NO_SUSPEND;
		rc = synaptics_clearpad_flash(this);
		goto exit;
	}

	if (this->pdt[SYN_F11_2D].number == function_value[SYN_F11_2D]) {
		rc = synaptics_clearpad_prepare_f11_2d(this);
		if (rc)
			goto exit;
	}

	snprintf(this->result_info, sizeof(this->result_info),
		"%s, family 0x%02x, fw rev 0x%02x, (%s)\n",
		make_string(this->device_info.product_id,
			HEADER_PRODUCT_ID_SIZE),
		this->device_info.customer_family,
		this->device_info.firmware_revision,
		this->flash_requested ? "fw updated" : "no fw update");
	this->flash_requested = false;

	/* notify end of task */
	dev_info(&this->pdev->dev, "result: %s", this->result_info);
	wake_up_interruptible(&this->task_none_wq);
exit:
	return rc;
}

static int synaptics_flash_enable(struct synaptics_clearpad *this)
{
	int rc;
	u8 buf[2];

	if (!this->fwdata_available) {
		rc = -EINVAL;
		goto exit;
	}
	/* read bootloader id */
	rc = synaptics_read(this, SYNF(F34_FLASH, QUERY, 0x00),
			buf, sizeof(buf));
	LOG_CHECK(this, "rc=%d\n", rc);
	if (rc)
		goto exit;

	/* write bootloader id to block data */
	rc = synaptics_write(this, SYNF(F34_FLASH, DATA, 0x02),
			buf, sizeof(buf));
	LOG_CHECK(this, "rc=%d\n", rc);
	if (rc)
		goto exit;

	usleep_range(10000, 11000);

	/* issue a flash program enable */
	rc = synaptics_put(this, SYNF(F34_FLASH, DATA, 0x12),
			FLASH_CONTROL_ENABLE_FLASH_PROGRAMMING);
	LOG_CHECK(this, "rc=%d\n", rc);
	if (rc)
		goto exit;

	this->state = SYN_STATE_FLASH_ENABLE;
	msleep(100);

	synaptics_clearpad_set_irq(this,
			this->pdt[SYN_F34_FLASH].irq_mask, true);
exit:
	return rc;
}

static int synaptics_flash_program(struct synaptics_clearpad *this)
{
	int rc;
	u8 buf[2];

	if (!this->fwdata_available) {
		rc = -EINVAL;
		goto exit;
	}

	/* make sure that we are in programming mode and there are no issues */
	rc = synaptics_read(this, SYNF(F34_FLASH, DATA, 0x12), buf, 1);
	if (rc)
		goto exit;
	if (buf[0] != FLASH_CONTROL_PROGRAM_ENABLED) {
		dev_err(&this->pdev->dev,
		       "failed enabling flash (%s)\n",
		       synaptics_clearpad_flash_status[(buf[0]>>4)&7]);
		rc = -EIO;
		goto exit;
	}

	dev_info(&this->pdev->dev, "flashing enabled\n");

	/* PDT may have changed, re-read */
	rc = synaptics_clearpad_read_pdt(this);
	if (rc)
		goto exit;

	/* read bootloader id */
	rc = synaptics_read(this, SYNF(F34_FLASH, QUERY, 0x00), buf, 2);
	if (rc)
		goto exit;

	/* write bootloader id to block data */
	rc = synaptics_write(this, SYNF(F34_FLASH, DATA, 0x02), buf, 2);
	if (rc)
		goto exit;

	usleep_range(10000, 11000);

	if (this->flash_mode == SYN_FLASH_MODE_NORMAL)
		/* issue a firmware and configuration erase */
		rc = synaptics_put(this, SYNF(F34_FLASH, DATA, 0x12),
				   FLASH_CONTROL_ERASE_ALL);
	else
		/* issue a configuration erase */
		rc = synaptics_put(this, SYNF(F34_FLASH, DATA, 0x12),
				   FLASH_CONTROL_ERASE_CONFIGURATION);

	if (rc)
		goto exit;

	dev_info(&this->pdev->dev, "firmware erasing\n");

	this->state = SYN_STATE_FLASH_PROGRAM;
exit:
	return rc;
}


static int synaptics_flash_data(struct synaptics_clearpad *this)
{
	int rc, len;
	u8 buf;
	const u8 *data;
	struct synaptics_flash_image *f = &this->flash;

	if (!this->fwdata_available) {
		rc = -EINVAL;
		goto exit;
	}

	if (f->data.pos > 0)
		goto write_block_data;

	/* make sure that we are in programming mode and there are no issues */
	rc = synaptics_read(this, SYNF(F34_FLASH, DATA, 0x12), &buf, 1);
	if (rc)
		goto exit;
	if (buf != FLASH_CONTROL_PROGRAM_ENABLED) {
		dev_err(&this->pdev->dev,
				"failed flashing data (%s)\n",
				synaptics_clearpad_flash_status[(buf>>4)&7]);
		rc = -EIO;
		goto exit;
	}

	/* block # low byte */
	rc = synaptics_put(this, SYNF(F34_FLASH, DATA, 0x00),
			f->data.pos & 0xff);
	if (rc)
		goto exit;

	/* block # high byte */
	rc = synaptics_put(this, SYNF(F34_FLASH, DATA, 0x01),
			(f->data.pos >> 8) & 0xff);
	if (rc)
		goto exit;

write_block_data:
	data = f->data.data + f->data.pos * 16;
	len = f->data.length - f->data.pos * 16;
	if (len > 16)
		len = 16;

	/* write block data */
	rc = synaptics_write(this, SYNF(F34_FLASH, DATA, 0x02), data, len);
	if (rc)
		goto exit;

	/* issue a write data block command */
	rc = synaptics_put(this, SYNF(F34_FLASH, DATA, 0x12),
			FLASH_CONTROL_WRITE_FIRMWARE_BLOCK);
	if (rc)
		goto exit;

	if (f->data.pos % 100 == 0)
		dev_info(&this->pdev->dev,
		       "wrote %d blocks\n", f->data.pos);

	/* if we've reached the end of the data flashing */
	if (++f->data.pos == f->data.blocks) {
		dev_info(&this->pdev->dev,
				"data flash finished\n");
		this->state = SYN_STATE_FLASH_DATA;
	}
exit:
	return rc;
}

static int synaptics_flash_config(struct synaptics_clearpad *this)
{
	int rc, len;
	u8 buf;
	const u8 *data;
	struct synaptics_flash_image *f = &this->flash;

	if (!this->fwdata_available) {
		rc = -EINVAL;
		goto exit;
	}

	if (f->config.pos > 0)
		goto write_block_data;

	/* make sure that we are in programming mode and there are no issues */
	rc = synaptics_read(this, SYNF(F34_FLASH, DATA, 0x12), &buf, 1);
	if (rc)
		goto exit;
	if (buf != FLASH_CONTROL_PROGRAM_ENABLED) {
		dev_err(&this->pdev->dev,
				"failed flashing config (%s)\n",
				synaptics_clearpad_flash_status[(buf>>4)&7]);
		rc = -EIO;
		goto exit;
	}

	/* block # low byte */
	rc = synaptics_put(this, SYNF(F34_FLASH, DATA, 0x00),
			f->config.pos & 0xff);
	if (rc)
		goto exit;

	/* block # high byte */
	rc = synaptics_put(this, SYNF(F34_FLASH, DATA, 0x01),
			(f->config.pos >> 8) & 0xff);
	if (rc)
		goto exit;

write_block_data:
	data = f->config.data + f->config.pos * 16;
	len = f->config.length - f->config.pos * 16;
	if (len > 16)
		len = 16;

	/* write block data */
	rc = synaptics_write(this, SYNF(F34_FLASH, DATA, 0x02), data, len);
	if (rc)
		goto exit;

	/* issue a write configuration block command */
	rc = synaptics_put(this, SYNF(F34_FLASH, DATA, 0x12),
		FLASH_CONTROL_WRITE_CONFIGURATION_BLOCK);
	if (rc)
		goto exit;

	/* if we've reached the end of the configuration flashing */
	if (++f->config.pos == f->config.blocks) {
		dev_info(&this->pdev->dev,
				"configuration flash finished\n");
		this->state = SYN_STATE_FLASH_CONFIG;
	}
exit:
	return rc;
}


static int synaptics_flash_disable(struct synaptics_clearpad *this)
{
	int rc;
	u8 buf;

	if (!this->fwdata_available) {
		rc = -EINVAL;
		goto exit;
	}

	/* make sure that we are in programming mode and there are no issues */
	rc = synaptics_read(this, SYNF(F34_FLASH, DATA, 0x12), &buf, 1);
	if (rc)
		goto exit;
	if (buf != FLASH_CONTROL_PROGRAM_ENABLED) {
		dev_err(&this->pdev->dev,
				"failed flashing config (%s)\n",
				synaptics_clearpad_flash_status[(buf>>4)&7]);
		rc = -EIO;
		goto exit;
	}

	usleep_range(10000, 11000);

	/* send a reset to the device to complete the flash procedure */
	rc = synaptics_put(this, SYNF(F01_RMI, COMMAND, 0x00),
						DEVICE_COMMAND_RESET);
	if (rc)
		goto exit;

	dev_info(&this->pdev->dev,
			"flashing finished, resetting\n");
	this->state = SYN_STATE_FLASH_DISABLE;
	msleep(100);
exit:
	return rc;
}

static int synaptics_flash_verify(struct synaptics_clearpad *this)
{
	int rc;
	u8 buf;

	/* make sure that we are no longer in programming mode */
	rc = synaptics_read(this, SYNF(F01_RMI, DATA, 0x00), &buf, 1);
	LOG_CHECK(this, "rc=%d F01_RMI_DATA0=0x%x\n", rc, buf);
	if (rc)
		goto exit;
	if (buf & (1 << 6)) {
		dev_err(&this->pdev->dev,
				"failed disabling flash (%s)\n",
				synaptics_clearpad_flash_reason[buf & 0x0f]);
		rc = -EIO;
		goto exit;
	}

	this->state = SYN_STATE_INIT;
	this->task = SYN_TASK_NONE;

	dev_info(&this->pdev->dev,
			"device successfully flashed\n");

	synaptics_clearpad_set_irq(this,
			this->pdt[SYN_F34_FLASH].irq_mask, false);

	rc = synaptics_clearpad_initialize(this);
	LOG_CHECK(this, "rc=%d\n", rc);
	if (rc)
		goto exit;
	this->state = SYN_STATE_RUNNING;
	synaptics_funcarea_initialize(this);
exit:
	return rc;
}

static void synaptics_firmware_check(struct synaptics_clearpad *this)
{
	const u8 *data;
	struct synaptics_flash_image *f = &this->flash;

	data = this->flash.image;

	/* Set up data block info */
	f->data.length = le32_to_cpu(*(u32 *)(data + 8));
	f->data.blocks = (f->data.length / 16) + !!(f->data.length % 16);
	f->data.data = data + HEADER_SIZE;
	f->data.pos = 0;
	dev_info(&this->pdev->dev, "DATA: length=%d blocks=%d data=%p\n",
		 f->data.length, f->data.blocks, f->data.data);

	/* Set up configuration block info */
	f->config.length = le32_to_cpu(*(u32 *)(data + 12));
	f->config.blocks = (f->config.length / 16) + !!(f->config.length % 16);
	f->config.data = data + HEADER_SIZE + f->data.length;
	f->config.pos = 0;
	dev_info(&this->pdev->dev, "CONFIG: length=%d blocks=%d data=%p\n",
		 f->config.length, f->config.blocks, f->config.data);
}

static int synaptics_clearpad_flash(struct synaptics_clearpad *this)
{
	int rc = 0;

	switch (this->state) {
	case SYN_STATE_FLASH_IMAGE_SET:
		rc = synaptics_flash_enable(this);
		LOG_CHECK(this, "rc=%d\n", rc);
		break;
	case SYN_STATE_FLASH_ENABLE:
		rc = synaptics_flash_program(this);
		LOG_CHECK(this, "rc=%d\n", rc);
		break;
	case SYN_STATE_FLASH_PROGRAM:
		if (this->flash_mode == SYN_FLASH_MODE_NORMAL) {
			rc = synaptics_flash_data(this);
			LOG_CHECK(this, "rc=%d\n", rc);
		} else {
			rc = synaptics_flash_config(this);
			LOG_CHECK(this, "rc=%d\n", rc);
		}
		break;
	case SYN_STATE_FLASH_DATA:
		rc = synaptics_flash_config(this);
		LOG_CHECK(this, "rc=%d\n", rc);
		break;
	case SYN_STATE_FLASH_CONFIG:
		rc = synaptics_flash_disable(this);
		synaptics_firmware_reset(this);
		LOG_CHECK(this, "rc=%d\n", rc);
		break;
	case SYN_STATE_FLASH_DISABLE:
		rc = synaptics_flash_verify(this);
		LOG_CHECK(this, "rc=%d\n", rc);
		break;
	default:
		LOG_CHECK(this, "rc=%d\n", rc);
		break;
	}

	if (rc) {
		dev_err(&this->pdev->dev,
			"failed during flash (%s)\n", state_name[this->state]);
		this->state = SYN_STATE_DISABLED;
		synaptics_clearpad_set_irq(this,
				this->pdt[SYN_F34_FLASH].irq_mask, false);

		snprintf(this->result_info, SYNAPTICS_STRING_LENGTH,
			"%s, family 0x%02x, fw rev 0x%02x, failed fw update\n",
			make_string(this->device_info.product_id,
			HEADER_PRODUCT_ID_SIZE),
			 this->device_info.customer_family,
			 this->device_info.firmware_revision);
		this->flash_requested = false;
		synaptics_firmware_reset(this);

		/* check if standby was reserved */
		if (this->active & SYN_STANDBY_AFTER_TASK) {
			this->active &= ~SYN_STANDBY_AFTER_TASK;
			this->active |= SYN_STANDBY;

			LOG_STAT(this, "active: %x (task: %s)\n",
				 this->active, task_name[this->task]);
		}

		/* notify end of task */
		dev_info(&this->pdev->dev, "result: %s", this->result_info);
		wake_up_interruptible(&this->task_none_wq);
	}
	return rc;
}

static int synaptics_clearpad_charger_mode_update(struct synaptics_clearpad
					  *this, bool new_status, bool force)
{
	int rc = 0;
	u8 val;
	bool old_status;

	if (!force && this->charger_mode == new_status) {
		dev_info(&this->pdev->dev, "%s: Not change charger mode: %d\n",
			 __func__, this->charger_mode);
		goto exit;
	}

	rc = regs_read(this,
			SYNAPTICS_CHARGER_CONTROL,
			&val, 1);
	if (rc) {
		dev_err(&this->pdev->dev, "%s: read error %d\n",
			__func__, rc);
		goto exit;
	}
	old_status = val & 0x01;

	if (new_status)
		val |= 0x01;
	else
		val &= ~0x01;

	rc = synaptics_clearpad_page_sel(this, 0x00);
	if (rc)
		goto exit;

	rc = regs_write(this,
			SYNAPTICS_CHARGER_CONTROL,
			&val, 1);

	if (rc) {
		dev_err(&this->pdev->dev, "%s: write error %d\n",
			__func__, rc);
		goto exit;
	}

	this->charger_mode = new_status;
	dev_info(&this->pdev->dev, "%s: Set charger mode: %d => %d\n",
		 __func__, old_status, new_status);
exit:
	return rc;
}

static void synaptics_clearpad_wd_status_poll(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct synaptics_clearpad *this = container_of(dwork,
			struct synaptics_clearpad, wd_poll_work);
	int rc;

	dev_dbg(&this->pdev->dev, "%s\n", __func__);

	LOCK(this);
	if ((this->active & SYN_ACTIVE_POWER) && !this->flash_requested) {
		rc = synaptics_read(this, SYNF(F01_RMI, DATA, 0x00),
							(void *)&rc, 1);
		if (rc) {
			dev_err(&this->pdev->dev, "%s, rc = %d\n",
							__func__, rc);
			dev_info(&this->pdev->dev, "Resetting device\n");
			synaptics_clearpad_reset_power(this);
		}
		schedule_delayed_work(&this->wd_poll_work, this->wd_poll_t_jf);
	}
	UNLOCK(this);
}

static void synaptics_clearpad_wd_update(struct synaptics_clearpad *this,
					 bool sync)
{
	if (sync)
		cancel_delayed_work_sync(&this->wd_poll_work);
	else
		cancel_delayed_work(&this->wd_poll_work);

	LOCK(this);
	if ((this->active & SYN_ACTIVE_POWER) && !this->flash_requested)
		schedule_delayed_work(&this->wd_poll_work, this->wd_poll_t_jf);
	UNLOCK(this);
}

static int synaptics_clearpad_set_power(struct synaptics_clearpad *this)
{
	int rc = 0;
	int active;
	bool should_wake;
	u8 irq;
	int users;

	LOCK(this);
	active = this->active;
	users = this->input ? this->input->users : 0;

	LOG_STAT(this, "powered %d, users %d, standby %d\n",
		 !!(active & SYN_ACTIVE_POWER),
		 users,
		 !!(active & SYN_STANDBY));

	dev_info(&this->pdev->dev, "%s: state=%s\n", __func__,
		 state_name[this->state]);
	should_wake = !(active & SYN_STANDBY);

	if (should_wake && !(active & SYN_ACTIVE_POWER)) {

		dev_info(&this->pdev->dev, "power ON\n");

		if (this->pdata->vreg_suspend) {
			rc = this->pdata->vreg_suspend(0);
			if (rc)
				goto err_unlock;
		}
		usleep_range(10000, 11000);

		synaptics_clearpad_set_irq(this,
				this->pdt[SYN_F01_RMI].irq_mask, true);
		rc = synaptics_read(this, SYNF(F01_RMI, DATA, 0x01), &irq, 1);
		if (rc) {
			dev_err(&this->pdev->dev,
					"%s, rc = %d, Resetting device\n",
					__func__, rc);
			synaptics_clearpad_reset_power(this);
		}

		if (this->pdt[SYN_F11_2D].number
				== function_value[SYN_F11_2D]) {
			rc = synaptics_put_bit(this, SYNF(F11_2D, CTRL, 0x00),
				XY_REPORTING_MODE_REDUCED_REPORTING_MODE,
				XY_REPORTING_MODE);
			if (rc)
				goto err_unlock;
		}

		rc = synaptics_put_bit(this, SYNF(F01_RMI, CTRL, 0x00),
			DEVICE_CONTROL_SLEEP_MODE_NORMAL_OPERATION,
			DEVICE_CONTROL_SLEEP_MODE);
		if (rc) {
			dev_err(&this->pdev->dev,
				"failed to exit sleep mode\n");
			goto err_report_mode;
		}

		usleep_range(10000, 11000);
		this->active |= SYN_ACTIVE_POWER;

	} else if (!should_wake && (active & SYN_ACTIVE_POWER)) {

		dev_info(&this->pdev->dev, "power OFF\n");

		if (this->easy_wakeup_config.gesture_enable) {
			rc = synaptics_put_bit(this, SYNF(F11_2D, CTRL, 0x00),
				XY_REPORTING_MODE_WAKEUP_GESTURE_MODE,
				XY_REPORTING_MODE);
			if (rc)
				goto err_unlock;

			this->ew_timeout = jiffies - 1;
			usleep_range(10000, 11000);
			LOG_CHECK(this, "enter doze mode\n");
			synaptics_clearpad_set_irq(this,
					this->pdt[SYN_F01_RMI].irq_mask, true);
			synaptics_read(this, SYNF(F01_RMI, DATA, 0x01),
					&irq, 1);

		} else {
			rc = synaptics_put_bit(this, SYNF(F01_RMI, CTRL, 0x00),
				DEVICE_CONTROL_SLEEP_MODE_SENSOR_SLEEP,
				DEVICE_CONTROL_SLEEP_MODE);
			if (rc) {
				dev_err(&this->pdev->dev,
					"failed to enter sleep mode\n");
				goto err_unlock;
			}
			usleep_range(10000, 11000);
			LOG_CHECK(this, "enter sleep mode\n");
			synaptics_clearpad_set_irq(this,
					this->pdt[SYN_F01_RMI].irq_mask, false);
		}

		this->active &= ~SYN_ACTIVE_POWER;

		if (this->pdata->vreg_suspend) {
			rc = this->pdata->vreg_suspend(1);
			if (rc)
				goto err_unlock;
		}
	} else {
		dev_info(&this->pdev->dev, "no change (%d)\n", should_wake);
	}

	goto err_unlock;

err_report_mode:
	if (this->pdt[SYN_F11_2D].number == function_value[SYN_F11_2D])
		synaptics_put_bit(this, SYNF(F11_2D, CTRL, 0x00),
			XY_REPORTING_MODE_REDUCED_REPORTING_MODE,
			XY_REPORTING_MODE);
err_unlock:
	UNLOCK(this);

	if (this->pdata->watchdog_enable)
		synaptics_clearpad_wd_update(this, true);

	return rc;
}

static void synaptics_clearpad_reset_power(struct synaptics_clearpad *this)
{
	int rc;

	if (this->pdata->vreg_reset) {
		rc = this->pdata->vreg_reset(this->pdev->dev.parent);
		dev_info(&this->pdev->dev, "power on reset\n");
		if (rc)
			dev_err(&this->pdev->dev, "vreg reset failed\n");
		else
			msleep(300);
	}
	this->page_num = 0;
}

static void synaptics_funcarea_initialize(struct synaptics_clearpad *this)
{
	struct synaptics_funcarea *funcarea;
	struct synaptics_area pointer_area;
	struct synaptics_button_data *button;
	struct synaptics_pointer_data *pointer_data;
	const char *func_name[] = {
		[SYN_FUNCAREA_INSENSIBLE] = "insensible",
		[SYN_FUNCAREA_POINTER] = "pointer",
		[SYN_FUNCAREA_BUTTON] = "button",
	};

	this->funcarea = this->pdata->funcarea_get(
				this->device_info.customer_family,
				this->device_info.firmware_revision);
	funcarea = this->funcarea;

	if (funcarea == NULL) {
		dev_info(&this->pdev->dev, "no funcarea\n");
		return;
	}

	for (; funcarea->func != SYN_FUNCAREA_END; funcarea++) {
		switch (funcarea->func) {
		case SYN_FUNCAREA_POINTER:
			pointer_area = (struct synaptics_area)
							funcarea->original;
			pointer_data = (struct synaptics_pointer_data *)
							funcarea->data;
			if (pointer_data) {
				pointer_area.x1 -= pointer_data->offset_x;
				pointer_area.x2 -= pointer_data->offset_x;
				pointer_area.y1 -= pointer_data->offset_y;
				pointer_area.y2 -= pointer_data->offset_y;
			}
			input_mt_init_slots(this->input,
						SYNAPTICS_MAX_N_FINGERS);
			input_set_abs_params(this->input, ABS_MT_POSITION_X,
					     pointer_area.x1,
					     pointer_area.x2, 0, 0);
			input_set_abs_params(this->input, ABS_MT_POSITION_Y,
					     pointer_area.y1,
					     pointer_area.y2, 0, 0);
			input_set_abs_params(this->input, ABS_MT_PRESSURE,
					0, SYNAPTICS_MAX_Z_VALUE, 0, 0);
			input_set_abs_params(this->input, ABS_MT_TOUCH_MAJOR,
					0, SYNAPTICS_MAX_W_VALUE + 1, 0, 0);
			input_set_abs_params(this->input, ABS_MT_TOUCH_MINOR,
					0, SYNAPTICS_MAX_W_VALUE + 1, 0, 0);
			input_set_abs_params(this->input, ABS_MT_ORIENTATION,
					-1, 1, 0, 0);
			if (this->pen_enabled)
				input_set_abs_params(this->input,
					ABS_MT_TOOL_TYPE, 0, MT_TOOL_MAX, 0, 0);
			break;
		case SYN_FUNCAREA_BUTTON:
			button = (struct synaptics_button_data *)funcarea->data;
			input_set_capability(this->input, EV_KEY, button->code);
			break;
		default:
			continue;
		}

		dev_info(&this->pdev->dev,
			 "funcarea '%s' [%d, %d, %d, %d] [%d, %d, %d, %d]\n",
			 func_name[funcarea->func],
			 funcarea->original.x1, funcarea->original.y1,
			 funcarea->original.x2, funcarea->original.y2,
			 funcarea->extension.x1, funcarea->extension.y1,
			 funcarea->extension.x2, funcarea->extension.y2);
	}
}

static inline bool synaptics_funcarea_test(struct synaptics_area *area,
					   struct synaptics_point *point)
{
	return (area->x1 <= point->x && point->x <= area->x2
		&& area->y1 <= point->y && point->y <= area->y2);
}

static struct synaptics_funcarea *
synaptics_funcarea_search(struct synaptics_clearpad *this,
			  struct synaptics_pointer *pointer)
{
	struct synaptics_funcarea *funcarea = this->funcarea;

	if (funcarea == NULL)
		goto exit;

	/* get new funcarea */
	for ( ; funcarea->func != SYN_FUNCAREA_END; funcarea++) {
		if (synaptics_funcarea_test(&funcarea->original,
						&pointer->cur))
			goto exit;
		if (funcarea->func == SYN_FUNCAREA_POINTER
		    && synaptics_funcarea_test(&funcarea->extension,
						&pointer->cur))
			goto exit;
	}
	funcarea = NULL;
exit:
	return funcarea;
}

static void synaptics_funcarea_crop(struct synaptics_area *area,
		struct synaptics_point *point)
{

	if (point->x < area->x1)
		point->x = area->x1;
	else if (area->x2 < point->x)
		point->x = area->x2;

	if (point->y < area->y1)
		point->y = area->y1;
	else if (area->y2 < point->y)
		point->y = area->y2;

}

static void synaptics_funcarea_down(struct synaptics_clearpad *this,
				    struct synaptics_pointer *pointer)
{
	int touch_major, touch_minor;
	struct synaptics_button_data *button;
	struct synaptics_pointer_data *pointer_data;
	struct synaptics_point *cur = &pointer->cur;

	switch (pointer->funcarea->func) {
	case SYN_FUNCAREA_INSENSIBLE:
		LOG_EVENT(this, "insensible\n");
		pointer->down = false;
		break;
	case SYN_FUNCAREA_POINTER:
		synaptics_funcarea_crop(&pointer->funcarea->original, cur);
		pointer_data = (struct synaptics_pointer_data *)
					pointer->funcarea->data;
		if (pointer_data) {
			cur->x -= pointer_data->offset_x;
			cur->y -= pointer_data->offset_y;
		}
		if (cur->tool == SYN_TOOL_FINGER)
			cur->tool = MT_TOOL_FINGER;
		else
			cur->tool = MT_TOOL_PEN;
		LOG_EVENT(this, "pt[%d]: (x,y)=(%d,%d) w=(%d,%d) z=%d t=%d\n",
		cur->id, cur->x, cur->y, cur->wx, cur->wy, cur->z, cur->tool);
		touch_major = max(cur->wx, cur->wy) + 1;
		touch_minor = min(cur->wx, cur->wy) + 1;
		input_mt_slot(this->input, cur->id);
		input_mt_report_slot_state(this->input, cur->tool, true);
		input_report_abs(this->input, ABS_MT_POSITION_X, cur->x);
		input_report_abs(this->input, ABS_MT_POSITION_Y, cur->y);
		input_report_abs(this->input, ABS_MT_PRESSURE, cur->z);
		input_report_abs(this->input, ABS_MT_TOUCH_MAJOR, touch_major);
		input_report_abs(this->input, ABS_MT_TOUCH_MINOR, touch_minor);
		input_report_abs(this->input, ABS_MT_ORIENTATION,
				 (cur->wx > cur->wy));
		break;
	case SYN_FUNCAREA_BUTTON:
		LOG_EVENT(this, "button\n");
		button = (struct synaptics_button_data *)
					pointer->funcarea->data;
		if (button)
			button->down = true;
		break;
	default:
		break;
	}
}

static void synaptics_funcarea_up(struct synaptics_clearpad *this,
				  struct synaptics_pointer *pointer)
{
	struct synaptics_button_data *button;

	switch (pointer->funcarea->func) {
	case SYN_FUNCAREA_INSENSIBLE:
		LOG_EVENT(this, "insensible up\n");
		break;
	case SYN_FUNCAREA_POINTER:
		LOG_EVENT(this, "pointer up\n");
		input_mt_slot(this->input, pointer->cur.id);
		input_mt_report_slot_state(this->input,
				pointer->cur.tool, false);
		break;
	case SYN_FUNCAREA_BUTTON:
		LOG_EVENT(this, "button up\n");
		button = (struct synaptics_button_data *)
					pointer->funcarea->data;
		if (button)
			button->down = false;
		break;
	default:
		break;
	}
	pointer->funcarea = NULL;
}

static void synaptics_funcarea_out(struct synaptics_clearpad *this,
				  struct synaptics_pointer *pointer)
{
	struct synaptics_funcarea *new_funcarea ;

	synaptics_funcarea_up(this, pointer);

	new_funcarea = synaptics_funcarea_search(this, pointer);
	if (new_funcarea == NULL)
		return;

	switch (new_funcarea->func) {
	case SYN_FUNCAREA_INSENSIBLE:
		pointer->down = false;
		break;
	default:
		break;
	}
}

static void synaptics_report_button(struct synaptics_clearpad *this,
		struct synaptics_button_data *button)
{
	if (button->down) {
		if (!button->down_report) {
			button->down_report = true;
			input_report_key(this->input, button->code, 1);
			LOG_EVENT(this, "key(%d): down\n", button->code);
		}
	} else {
		if (button->down_report) {
			button->down_report = false;
			input_report_key(this->input, button->code, 0);
			LOG_EVENT(this, "key(%d): up\n", button->code);
		}
	}
}

static void
synaptics_funcarea_report_extra_events(struct synaptics_clearpad *this)
{
	struct synaptics_funcarea *funcarea = this->funcarea;
	struct synaptics_button_data *button;

	if (funcarea == NULL)
		return;

	for (; funcarea->func != SYN_FUNCAREA_END; funcarea++) {
		if (funcarea->func == SYN_FUNCAREA_BUTTON) {
			button = (struct synaptics_button_data *)funcarea->data;
			if (button)
				synaptics_report_button(this, button);
		}
	}
}

static void synaptics_funcarea_invalidate_all(struct synaptics_clearpad *this)
{
	struct synaptics_pointer *pointer;
	int i;

	for (i = 0; i < this->extents.n_fingers; ++i) {
		pointer = &this->pointer[i];
		if (pointer->down) {
			pointer->down = false;
			LOG_VERBOSE(this, "invalidate pointer %d\n", i);
		}
		if (pointer->funcarea)
			synaptics_funcarea_up(this, pointer);
	}
	synaptics_funcarea_report_extra_events(this);
	input_sync(this->input);
}

static void synaptics_report_finger_n(struct synaptics_clearpad *this,
				      int finger)
{
	enum registers {
		REG_X_MSB,
		REG_Y_MSB,
		REG_XY_LSB,
		REG_XY_W,
		REG_Z,
	};
	struct synaptics_pointer *pointer = &this->pointer[finger];
	u8 *buf = this->reg_buf;
	int flip_config = 0;
	int tool;

	/* check finger state */
	tool = SYNAPTICS_FINGER_STATE(buf, finger);
	if (tool == SYN_TOOL_FINGER ||
		(this->pen_enabled && (tool == SYN_TOOL_PEN))) {
		struct synaptics_point new_point;

		buf += SYNAPTICS_FINGER_OFF(this->extents.n_fingers, finger);
		new_point.id = finger;
		new_point.x = ((buf[REG_X_MSB] << 4)
			       | ((buf[REG_XY_LSB] & 0x0f)));
		new_point.y = ((buf[REG_Y_MSB] << 4)
			       | ((buf[REG_XY_LSB] & 0xf0) >> 4));
		new_point.wx = (buf[REG_XY_W] & 0x0f);
		new_point.wy = ((buf[REG_XY_W] >> 4) & 0x0f);
		new_point.z = buf[REG_Z];
		new_point.tool = tool;

		flip_config = this->pdata->flip_config_get(
				this->device_info.customer_family,
				this->device_info.firmware_revision);

		switch (flip_config) {
		case SYN_FLIP_X:
			new_point.x = this->extents.x_max - new_point.x;
			break;
		case SYN_FLIP_Y:
			new_point.y = this->extents.y_max - new_point.y;
			break;
		case SYN_FLIP_XY:
			new_point.x = this->extents.x_max - new_point.x;
			new_point.y = this->extents.y_max - new_point.y;
			break;
		case SYN_FLIP_NONE:
		default:
			break;
		}

		LOG_VERBOSE(this, "pt[%d]: (x,y)=(%d,%d) w=(%d,%d) z=%d t=%d\n",
			    new_point.id, new_point.x, new_point.y,
			    new_point.wx, new_point.wy, new_point.z,
			    new_point.tool);

		if (!pointer->down) {
			/* first touch event */
			pointer->down = true;
			pointer->cur = new_point;
			pointer->funcarea
				= synaptics_funcarea_search(this, pointer);
			LOG_VERBOSE(this, "validate pointer %d [func %d]\n",
				    new_point.id, pointer->funcarea
				    ? pointer->funcarea->func : -1);
		}
		if (pointer->funcarea) {
			struct synaptics_area *extension
				= &pointer->funcarea->extension;

			if (synaptics_funcarea_test(extension, &new_point)) {
				pointer->cur = new_point;
				synaptics_funcarea_down(this, pointer);
			} else {
				synaptics_funcarea_out(this, pointer);
			}
		}
	} else {
		if (pointer->down) {
			pointer->down = false;
			LOG_VERBOSE(this, "invalidate pointer %d\n", finger);
		}
		if (pointer->funcarea)
			synaptics_funcarea_up(this, pointer);
	}
}

static int synaptics_clearpad_read_fingers(struct synaptics_clearpad *this)
{
	int rc, i, size;
	u8 *buf = this->reg_buf;

	/* read status and first finger */
	memset(buf, 0, SYNAPTICS_REG_MAX);
	size = SYNAPTICS_FINGER_OFF(this->extents.n_fingers, 1);
	rc = synaptics_read(this, SYNF(F11_2D, DATA, 0x00), buf, size);
	if (rc)
		goto exit;
	for (i = this->extents.n_fingers - 1 ; i > 0; i--) {
		if (SYNAPTICS_FINGER_STATE(buf, i)) {
			/* read remained fingers */
			rc = synaptics_read(this,
				SYNF(F11_2D, DATA, 0x00) + size, buf + size,
				SYNAPTICS_FINGER_DATA_SIZE * i);
			break;
		}
	}
exit:
	return rc;
}

static int synaptics_clearpad_handle_gesture(struct synaptics_clearpad *this)
{
	u8 wakeint;
	int rc;

	rc = synaptics_read(this, SYNF(F11_2D, DATA,
			this->easy_wakeup_config.large_panel ? 0x39 : 0x43),
			&wakeint, 1);
	if (rc)
		goto exit;

	dev_info(&this->pdev->dev, "Gesture %d", wakeint);

	if (time_after(jiffies, this->ew_timeout))
		this->ew_timeout = jiffies + msecs_to_jiffies(
			this->easy_wakeup_config.timeout_delay);
	else
		goto exit;

	switch (wakeint) {
	case XY_LPWG_STATUS_DOUBLE_TAP_DETECTED:
		rc = evgen_execute(this->input, this->evgen_blocks,
					"double_tap");
		break;
	case XY_LPWG_STATUS_SWIPE_DETECTED:
		rc = evgen_execute(this->input, this->evgen_blocks,
					"single_swipe");
		break;
	case XY_LPWG_STATUS_TWO_SWIPE_DETECTED:
		rc = evgen_execute(this->input, this->evgen_blocks,
					"two_swipe");
		break;
	default:
		rc = -EINVAL;
		break;
	}
exit:
	return rc;
}

static void synaptics_clearpad_process_irq(struct synaptics_clearpad *this)
{
	int rc, i;
	u8 status;
	u8 interrupt;

	LOCK(this);
	if (!(this->active & SYN_ACTIVE_POWER)) {
		if (this->easy_wakeup_config.gesture_enable &&
			this->pdata->vreg_suspend) {
			rc = this->pdata->vreg_suspend(0);
			if (rc)
				goto unlock;
			usleep_range(10000, 11000);
		} else {
			dev_dbg(&this->pdev->dev,
				"No set vdd, gesture: %d\n",
				this->easy_wakeup_config.gesture_enable);
			rc = 0;
			goto unlock;
		}
	}

	rc = synaptics_read(this, SYNF(F01_RMI, DATA, 0x01), &interrupt, 1);
	LOG_CHECK(this, "rc=%d F01_RMI_DATA1=0x%x\n", rc, interrupt);
	if (rc)
		goto err_bus;

	if (interrupt & this->pdt[SYN_F34_FLASH].irq_mask) {
		synaptics_clearpad_flash(this);
		goto unlock;
	}

	if (interrupt & this->pdt[SYN_F01_RMI].irq_mask) {
		for (i = 0; i < SYNAPTICS_RETRY_NUM_OF_INITIAL_CHECK; i++) {
			rc = synaptics_read(this, SYNF(F01_RMI, DATA, 0x00),
					&status, 1);
			if (rc)
				goto err_bus;
			dev_info(&this->pdev->dev, "status = 0x%02x\n", status);
			if (DEVICE_STATUS_UNCONFIGURED_RESET_OCCURRED
								== status) {
				dev_info(&this->pdev->dev, "device reset\n");
				if (this->state == SYN_STATE_FLASH_DISABLE) {
					synaptics_clearpad_flash(this);
				} else {
					synaptics_clearpad_initialize(this);
					this->state = SYN_STATE_RUNNING;
					synaptics_clearpad_charger_mode_update(
					       this, this->charger_mode, true);
				}
				break;
			} else if ((DEVICE_STATUS_DEVICE_FAILURE == status) ||
			(DEVICE_STATUS_UNCONFIGURED_DEVICE_FAILURE == status)) {
				synaptics_clearpad_reset_power(this);
				goto unlock;
			} else {
				dev_info(&this->pdev->dev,
						"check fail: retry = %d\n", i);
				msleep(100);
			}
		}
		goto unlock;
	}

	if (interrupt & this->pdt[SYN_F05_ANALOG].irq_mask) {
		this->state = SYN_STATE_RUNNING;
		wake_up_interruptible(&this->task_none_wq);
		goto unlock;
	}

	if (interrupt & this->pdt[SYN_F54_ANALOG].irq_mask) {
		this->state = SYN_STATE_RUNNING;
		wake_up_interruptible(&this->task_none_wq);
		goto unlock;
	}

	if (interrupt & this->pdt[SYN_F11_2D].irq_mask) {
		if (this->easy_wakeup_config.gesture_enable
		    && !(this->active & SYN_ACTIVE_POWER)) {
			if (synaptics_clearpad_handle_gesture(this) == 0)
				goto unlock; /* gesture handled */
			else
				goto err_bus;
		}

		rc = synaptics_read(this, SYNF(F01_RMI, DATA, 0x00),
				&status, 1);
		LOG_CHECK(this, "rc=%d F01_RMI_DATA00=0x%x\n", rc, status);
		if (rc)
			goto err_bus;
		if ((DEVICE_STATUS_DEVICE_FAILURE == status) ||
		(DEVICE_STATUS_UNCONFIGURED_DEVICE_FAILURE == status)) {
			synaptics_clearpad_reset_power(this);
			goto unlock;
		}

		if (this->chip == SYN_CHIP_3000) {
			rc = synaptics_read(this, SYNF(F11_2D, DATA, 0x35),
					&status, 1);
			LOG_CHECK(this, "rc=%d F11_2D_DATA09=0x%x\n", rc,
					status);
			if (rc)
				goto err_bus;
		}

		rc = synaptics_clearpad_read_fingers(this);
		if (rc)
			goto err_bus;

		for (i = 0; i < this->extents.n_fingers; ++i)
			synaptics_report_finger_n(this, i);

		synaptics_funcarea_report_extra_events(this);
		input_sync(this->input);

		goto unlock;
	}

	dev_info(&this->pdev->dev, "no work, interrupt=[0x%02x]\n", interrupt);
	goto unlock;

err_bus:
	dev_err(&this->pdev->dev, "read error\n");
unlock:
	UNLOCK(this);

	if (!rc && this->pdata->watchdog_enable)
		synaptics_clearpad_wd_update(this, false);
}

static irqreturn_t synaptics_clearpad_threaded_handler(int irq, void *dev_id)
{
	struct device *dev = dev_id;
	struct synaptics_clearpad *this = dev_get_drvdata(dev);
	unsigned long flags;

	do {
		synaptics_clearpad_process_irq(this);

		spin_lock_irqsave(&this->slock, flags);
		if (likely(!this->irq_pending)) {
			this->dev_busy = false;
			spin_unlock_irqrestore(&this->slock, flags);
			break;
		}
		this->irq_pending = false;
		dev_info(&this->pdev->dev, "Touch irq pending\n");
		spin_unlock_irqrestore(&this->slock, flags);

	} while (true);
	return IRQ_HANDLED;
}

static irqreturn_t synaptics_clearpad_hard_handler(int irq, void *dev_id)
{
	struct device *dev = dev_id;
	struct synaptics_clearpad *this = dev_get_drvdata(dev);
	unsigned long flags;
	irqreturn_t ret;

	spin_lock_irqsave(&this->slock, flags);
	if (unlikely(this->dev_busy)) {
		this->irq_pending = true;
		dev_info(&this->pdev->dev, "Wake on touch\n");
		ret = IRQ_HANDLED;
	} else {
		this->dev_busy = true;
		ret = IRQ_WAKE_THREAD;
	}
	spin_unlock_irqrestore(&this->slock, flags);
	return ret;
}

static int synaptics_clearpad_device_open(struct input_dev *dev)
{
	struct synaptics_clearpad *this = input_get_drvdata(dev);
	int rc;

	LOG_STAT(this, "state=%s\n", state_name[this->state]);

	switch (this->state) {
	case SYN_STATE_INIT:
		rc = 0;
		break;
	case SYN_STATE_DISABLED:
		rc = -ENODEV;
		break;
	case SYN_STATE_RUNNING:
		rc = synaptics_clearpad_set_power(this);
		break;
	default:
		rc = -EBUSY;
		break;
	}

	return rc;
}

static void synaptics_clearpad_device_close(struct input_dev *dev)
{
	struct synaptics_clearpad *this = input_get_drvdata(dev);

	LOG_STAT(this, "state=%s\n", state_name[this->state]);

	(void)synaptics_clearpad_set_power(this);
}

static int synaptics_clearpad_command_open(struct synaptics_clearpad *this,
		size_t image_size)
{
	int rc = 0;
	LOCK(this);
	/* allocate image buffer */
	this->flash.image = kmalloc(image_size, GFP_KERNEL);
	if (this->flash.image == NULL) {
		dev_err(&this->pdev->dev,
		       "buffer allocation error (%d bytes)\n", image_size);
		rc = -ENOMEM;
	} else {
		this->flash.buffer_size = image_size;
		dev_info(&this->pdev->dev,
			"prepared buffer size=%d\n", this->flash.buffer_size);
	}
	UNLOCK(this);
	return rc;
}

static ssize_t synaptics_clearpad_fwdata_write(struct file *file,
		struct kobject *kobj,
		struct bin_attribute *bin_attr,
		char *buf, loff_t pos, size_t size)
{
	int rc;
	struct device *dev = container_of(kobj, struct device, kobj);
	struct synaptics_clearpad *this = dev_get_drvdata(dev);

	if (!this->flash.image) {
		size_t image_size;

		if (size < HEADER_SIZE) {
			dev_err(&this->pdev->dev, "invalid firmware size");
			size = -EINVAL;
			goto exit;
		}
		this->flash.format_version = buf[HEADER_VERSION_OFFSET];
		memcpy(&(this->flash.firmware_size),
				&buf[HEADER_FIRMWARE_SIZE_OFFSET],
				HEADER_FIRMWARE_SIZE_SIZE);
		memcpy(&(this->flash.config_size),
				&buf[HEADER_CONFIG_SIZE_OFFSET],
				HEADER_CONFIG_SIZE_SIZE);
		memcpy(this->flash.product_id,
				&buf[HEADER_PRODUCT_ID_OFFSET],
				HEADER_PRODUCT_ID_SIZE);
		this->flash.customer_family =
				buf[HEADER_CUSTOMER_FAMILY_OFFSET];
		this->flash.firmware_revision =
				buf[HEADER_FIRMWARE_REVISION_OFFSET];
		image_size = this->flash.firmware_size
				+ this->flash.config_size
				+ HEADER_SIZE;
		dev_info(&this->pdev->dev,
				"firmware_size=%d\n",
				this->flash.firmware_size);
		dev_info(&this->pdev->dev,
				"config_size=%d\n",
				this->flash.config_size);
		dev_info(&this->pdev->dev,
				"image_size=%d\n", image_size);
		rc = synaptics_clearpad_command_open(this, image_size);
		if (rc) {
			size = -EINVAL;
			goto exit;
		}
	}

	if (this->flash.size + size > this->flash.buffer_size) {
		dev_err(&this->pdev->dev,
		       "firmware buffer is too small\n");
		size = -ENOMEM;
		goto exit;
	}

	LOCK(this);
	memcpy(this->flash.image + this->flash.size, buf, size);
	this->flash.size += size;
	dev_info(&this->pdev->dev,
		"got %d bytes, total %d bytes\n", size, this->flash.size);
	UNLOCK(this);
exit:
	return size;
}

static struct bin_attribute synaptics_clearpad_fwdata = {
	.attr = {
		.name = "fwdata",
		.mode = 0600,
	},
	.size = 4096,
	.write = synaptics_clearpad_fwdata_write
};

static bool synaptics_clearpad_check_task(struct synaptics_clearpad *this,
			   enum synaptics_state *state)
{
	bool rc = false;

	LOCK(this);
	*state = this->state;
	if (*state == SYN_STATE_RUNNING || *state == SYN_STATE_DISABLED)
		rc = true;

	UNLOCK(this);

	return rc;
}

static int synaptics_clearpad_command_fw_load_start(
					struct synaptics_clearpad *this)
{
	int rc;

	LOCK(this);
	if (this->flash.image == NULL) {
		synaptics_firmware_reset(this);
		rc = sysfs_create_bin_file(&this->input->dev.kobj,
				&synaptics_clearpad_fwdata);
		if (rc) {
			dev_err(&this->pdev->dev,
					"failed to create fwdata\n");
		}
	} else {
		dev_err(&this->pdev->dev,
				"flash.image already exists\n");
		synaptics_firmware_reset(this);
		rc = -EINVAL;
	}
	UNLOCK(this);

	return rc;
}

static int synaptics_clearpad_command_fw_flash(struct synaptics_clearpad *this,
					enum synaptics_flash_modes flash_mode)
{
	enum   synaptics_state state;
	int rc;

	LOCK(this);
	if (!this->fwdata_available) {
		dev_err(&this->pdev->dev,
				"fwdata_available is not ready yet\n");
		rc = -EINVAL;
		UNLOCK(this);
		goto error;
	}
	UNLOCK(this);
	if (wait_event_interruptible(this->task_none_wq,
			synaptics_clearpad_check_task(this, &state))) {
		rc = -ERESTARTSYS;
		goto error;
	}

	LOCK(this);
	memset(this->result_info, 0, SYNAPTICS_STRING_LENGTH);
	this->flash_requested = true;
	this->flash_mode = flash_mode;

	synaptics_firmware_check(this);

	if (this->active & SYN_STANDBY) {
		/* wake up during flashing */
		this->active &= ~SYN_STANDBY;
		this->active |= SYN_STANDBY_AFTER_TASK;
		LOG_STAT(this, "active: %x (task: %s)\n",
			 this->active, task_name[this->task]);
	}
	UNLOCK(this);

	/* wake up */
	rc = synaptics_clearpad_set_power(this);
	if (rc)
		goto error;

	LOCK(this);
	rc = synaptics_clearpad_initialize(this);
	LOG_CHECK(this, "rc=%d\n", rc);
	UNLOCK(this);
	if (rc)
		goto error;

	/* wait for end of flash */
	if (wait_event_interruptible(this->task_none_wq,
			synaptics_clearpad_check_task(this, &state))) {
		rc = -ERESTARTSYS;
		goto error;
	}

	LOCK(this);
	/* check if standby was reserved */
	if (this->active & SYN_STANDBY_AFTER_TASK) {
		this->active &= ~SYN_STANDBY_AFTER_TASK;
		this->active |= SYN_STANDBY;
		LOG_STAT(this, "active: %x (task: %s)\n",
			 this->active, task_name[this->task]);
	}
	UNLOCK(this);

	/* restore previous state */
	rc = synaptics_clearpad_set_power(this);
	if (!rc)
		goto exit;

error:
	snprintf(this->result_info, SYNAPTICS_STRING_LENGTH,
		"%s, family 0x%02x, fw rev 0x%02x, failed fw update\n",
		make_string(this->device_info.product_id,
					HEADER_PRODUCT_ID_SIZE),
		 this->device_info.customer_family,
		 this->device_info.firmware_revision);
	LOCK(this);
	this->flash_requested = false;
	synaptics_firmware_reset(this);
	dev_info(&this->pdev->dev, "result: %s", this->result_info);
	UNLOCK(this);

	if (this->pdata->watchdog_enable)
		synaptics_clearpad_wd_update(this, true);
exit:
	return rc;
}

static int synaptics_clearpad_command_fw_load_end(
					struct synaptics_clearpad *this)
{
	int rc;

	LOCK(this);
	if (!this->flash.image) {
		dev_err(&this->pdev->dev,
				"loading firmware is not started yet\n");
		rc = -EINVAL;
	} else if (this->flash.size == this->flash.buffer_size) {
		this->fwdata_available = true;
		rc = 0;
	} else {
		dev_err(&this->pdev->dev,
				"loading firmware is not finished yet\n");
		synaptics_firmware_reset(this);
		rc = -EINVAL;
	}
	sysfs_remove_bin_file(&this->input->dev.kobj,
			&synaptics_clearpad_fwdata);
	UNLOCK(this);

	return rc;
}

static ssize_t synaptics_clearpad_state_show(struct device *dev,
					      struct device_attribute *attr,
					      char *buf)
{
	struct synaptics_clearpad *this = dev_get_drvdata(dev);

	if (!strncmp(attr->attr.name, __stringify(fwinfo), PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%s, family 0x%02x, fw rev 0x%02x, task=%s, state=%s\n",
			make_string(this->device_info.product_id,
				HEADER_PRODUCT_ID_SIZE),
			this->device_info.customer_family,
			this->device_info.firmware_revision,
			task_name[this->task], state_name[this->state]);
	else if (!strncmp(attr->attr.name, __stringify(fwfamily), PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%x", this->device_info.customer_family);
	else if (!strncmp(attr->attr.name, __stringify(fwrevision), PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%x", this->device_info.firmware_revision);
	else if (!strncmp(attr->attr.name, __stringify(fwtask), PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%s", task_name[this->task]);
	else if (!strncmp(attr->attr.name, __stringify(fwstate), PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%s", state_name[this->state]);
	else if (!strncmp(attr->attr.name, __stringify(wakeup_gesture),
		PAGE_SIZE))
		snprintf(buf, PAGE_SIZE, "%d",
			this->easy_wakeup_config.gesture_enable);
	else
		snprintf(buf, PAGE_SIZE, "illegal sysfs file");
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t synaptics_clearpad_fwflush_store(struct device *dev,
					       struct device_attribute *attr,
					       const char *buf, size_t size)
{
	struct synaptics_clearpad *this = dev_get_drvdata(dev);
	int rc;

	dev_info(&this->pdev->dev, "flush command: %s\n", buf);

	if (!strncmp(buf, flush_commands[SYN_LOAD_START], PAGE_SIZE)) {
		rc = synaptics_clearpad_command_fw_load_start(this);
	} else if (!strncmp(buf, flush_commands[SYN_LOAD_END], PAGE_SIZE)) {
		rc = synaptics_clearpad_command_fw_load_end(this);
	} else if (!strncmp(buf, flush_commands[SYN_FORCE_FLUSH], PAGE_SIZE)) {
		dev_info(&this->pdev->dev, "start firmware flash\n");
		rc = synaptics_clearpad_command_fw_flash(this,
							 SYN_FLASH_MODE_NORMAL);
	} else if (!strncmp(buf, flush_commands[SYN_CONFIG_FLUSH], PAGE_SIZE)) {
		dev_info(&this->pdev->dev, "start firmware config\n");
		rc = synaptics_clearpad_command_fw_flash(this,
							 SYN_FLASH_MODE_CONFIG);
	} else {
		dev_err(&this->pdev->dev, "illegal command\n");
		rc = -EINVAL;
	}
	if (rc)
		dev_err(&this->pdev->dev, "%s failed\n", __func__);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t synaptics_clearpad_touchcmd_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct synaptics_clearpad *this = dev_get_drvdata(dev);

	dev_dbg(&this->pdev->dev, "%s: start\n", __func__);

	LOCK(this);

	if (sysfs_streq(buf, TOUCHCMD_CHARGER_MODE_START)) {
		synaptics_clearpad_charger_mode_update(this, true, false);
	} else if (sysfs_streq(buf, TOUCHCMD_CHARGER_MODE_END)) {
		synaptics_clearpad_charger_mode_update(this, false, false);
	} else {
		/* not supported command */
		dev_err(&this->pdev->dev, "%s: cmd not supported\n", __func__);
	}

	UNLOCK(this);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t synaptics_clearpad_enabled_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct synaptics_clearpad *this = dev_get_drvdata(dev);
	int rc;

	dev_dbg(&this->pdev->dev, "%s: start\n", __func__);

	LOCK(this);

	if (sysfs_streq(buf, "1") &&
		(this->state == SYN_STATE_IRQ_DISABLED)) {
		if (this->pdata->gpio_export) {
			rc = this->pdata->gpio_export(&this->input->dev,
							false);
			if (rc)
				dev_err(&this->pdev->dev,
					"gpio could not be unexported\n");
		}
		goto enable;
	} else if (sysfs_streq(buf, "0") &&
			(this->state == SYN_STATE_RUNNING)) {
		free_irq(this->pdata->irq, &this->pdev->dev);
		if (this->pdata->gpio_export) {
			rc = this->pdata->gpio_export(&this->input->dev,
							true);
			if (rc) {
				dev_err(&this->pdev->dev,
					"failed gpio export\n");
				goto enable;
			}
		}
		this->state = SYN_STATE_IRQ_DISABLED;
	} else {
		/* No changes */
		dev_info(&this->pdev->dev, "%s: No changes, state=%s\n",
			 __func__, state_name[this->state]);
	}
	goto end;

enable:
	rc = request_threaded_irq(this->pdata->irq,
				synaptics_clearpad_hard_handler,
				synaptics_clearpad_threaded_handler,
				IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
				this->pdev->dev.driver->name,
				&this->pdev->dev);
	if (rc) {
		dev_err(&this->pdev->dev,
			"irq %d busy? <%d>\n",
			this->pdata->irq, rc);

	} else {
		this->state = SYN_STATE_RUNNING;
	}
end:
	LOG_STAT(this, "state=%s\n", state_name[this->state]);
	UNLOCK(this);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t synaptics_clearpad_wakeup_gesture_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct synaptics_clearpad *this = dev_get_drvdata(dev);

	dev_dbg(&this->pdev->dev, "%s: start\n", __func__);

	LOCK(this);

	if (sysfs_streq(buf, "1")) {
		this->easy_wakeup_config.gesture_enable = true;
		device_init_wakeup(&this->pdev->dev, 1);
	} else if (sysfs_streq(buf, "0")) {
		this->easy_wakeup_config.gesture_enable = false;
		device_init_wakeup(&this->pdev->dev, 0);
	} else {
		dev_err(&this->pdev->dev,
			"illegal command in %s, buf is: %s\n",
			buf, __func__);
	}

	UNLOCK(this);
	return strnlen(buf, PAGE_SIZE);
}

static struct device_attribute clearpad_sysfs_attrs[] = {
	__ATTR(fwinfo, S_IRUGO, synaptics_clearpad_state_show, 0),
	__ATTR(fwfamily, S_IRUGO, synaptics_clearpad_state_show, 0),
	__ATTR(fwrevision, S_IRUGO, synaptics_clearpad_state_show, 0),
	__ATTR(fwtask, S_IRUGO, synaptics_clearpad_state_show, 0),
	__ATTR(fwstate, S_IRUGO, synaptics_clearpad_state_show, 0),
	__ATTR(fwflush, S_IWUSR, 0, synaptics_clearpad_fwflush_store),
	__ATTR(touchcmd, S_IWUSR, 0, synaptics_clearpad_touchcmd_store),
	__ATTR(enabled, S_IWUSR, 0, synaptics_clearpad_enabled_store),
};

static struct device_attribute clearpad_wakeup_gesture_attr =
	__ATTR(wakeup_gesture, S_IRUGO | S_IWUSR,
				synaptics_clearpad_state_show,
				synaptics_clearpad_wakeup_gesture_store);

static int create_sysfs_entries(struct synaptics_clearpad *this)
{
	int i, rc = 0;

	for (i = 0; i < ARRAY_SIZE(clearpad_sysfs_attrs); i++) {
		rc = device_create_file(&this->input->dev,
				&clearpad_sysfs_attrs[i]);
		if (rc) {
			for (; i >= 0; --i)
				device_remove_file(&this->input->dev,
						   &clearpad_sysfs_attrs[i]);
			break;
		}
	}
	return rc;
}

static void remove_sysfs_entries(struct synaptics_clearpad *this)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(clearpad_sysfs_attrs); i++)
		device_remove_file(&this->input->dev, &clearpad_sysfs_attrs[i]);
}

static int synaptics_clearpad_input_init(struct synaptics_clearpad *this)
{
	int rc;

	this->input = input_allocate_device();
	if (!this->input) {
		rc = -ENOMEM;
		goto exit;
	}

	input_set_drvdata(this->input, this);

	this->input->open = synaptics_clearpad_device_open;
	this->input->close = synaptics_clearpad_device_close;
	this->input->name = CLEARPAD_NAME;
	this->input->id.vendor = SYNAPTICS_CLEARPAD_VENDOR;
	this->input->id.product = 1;
	this->input->id.version = 1;
	this->input->id.bustype = this->bdata->bustype;
	set_bit(EV_ABS, this->input->evbit);

	set_bit(ABS_MT_TRACKING_ID, this->input->absbit);
	set_bit(ABS_MT_ORIENTATION, this->input->absbit);
	set_bit(ABS_MT_PRESSURE, this->input->absbit);
	set_bit(ABS_MT_TOUCH_MAJOR, this->input->absbit);
	set_bit(ABS_MT_TOUCH_MINOR, this->input->absbit);
	if (this->pen_enabled)
		set_bit(ABS_MT_TOOL_TYPE, this->input->absbit);

	dev_info(&this->pdev->dev, "Touch area [%d, %d, %d, %d]\n",
		 this->extents.x_min, this->extents.y_min,
		 this->extents.x_max, this->extents.y_max);

	synaptics_funcarea_initialize(this);

	if (this->pdata->evgen_block_get)
		this->evgen_blocks = this->pdata->evgen_block_get(
			this->device_info.customer_family,
			this->device_info.firmware_revision);
	dev_info(&this->pdev->dev, "evgen_blocks is %s\n",
		 this->evgen_blocks ? "used" : "null");
	evgen_initialize(this->input, this->evgen_blocks);

	rc = input_register_device(this->input);
	if (rc) {
		dev_err(&this->pdev->dev,
		       "failed to register device\n");
		input_set_drvdata(this->input, NULL);
		input_free_device(this->input);
		goto exit;
	}

	if (this->evgen_blocks) {
		rc = device_create_file(&this->input->dev,
				&clearpad_wakeup_gesture_attr);
		if (rc)
			dev_err(&this->pdev->dev,
				"sysfs_create_file failed: %d\n", rc);
	}
exit:
	return rc;
}

static int synaptics_clearpad_suspend(struct device *dev)
{
	struct synaptics_clearpad *this = dev_get_drvdata(dev);
	int rc = 0;
	bool go_suspend;

	LOCK(this);
	go_suspend = (this->task != SYN_TASK_NO_SUSPEND);
	if (go_suspend)
		this->active |= SYN_STANDBY;
	else
		this->active |= SYN_STANDBY_AFTER_TASK;

	LOG_STAT(this, "active: %x (task: %s)\n",
		 this->active, task_name[this->task]);
	UNLOCK(this);

	rc = synaptics_clearpad_set_power(this);
	return rc;
}

static int synaptics_clearpad_resume(struct device *dev)
{
	struct synaptics_clearpad *this = dev_get_drvdata(dev);
	int rc = 0;
	bool go_resume;

	LOCK(this);
	go_resume = !!(this->active & (SYN_STANDBY | SYN_STANDBY_AFTER_TASK));
	if (go_resume)
		this->active &= ~(SYN_STANDBY | SYN_STANDBY_AFTER_TASK);

	LOG_STAT(this, "active: %x (task: %s)\n",
		 this->active, task_name[this->task]);

	synaptics_funcarea_invalidate_all(this);
	UNLOCK(this);

	rc = synaptics_clearpad_set_power(this);
	return rc;
}

static int synaptics_clearpad_pm_suspend(struct device *dev)
{
	struct synaptics_clearpad *this = dev_get_drvdata(dev);
	unsigned long flags;

	spin_lock_irqsave(&this->slock, flags);
	if (unlikely(this->dev_busy)) {
		dev_info(dev, "Busy to suspend\n");
		spin_unlock_irqrestore(&this->slock, flags);
		return -EBUSY;
	}
	this->dev_busy = true;
	spin_unlock_irqrestore(&this->slock, flags);

#ifndef CONFIG_HAS_EARLYSUSPEND
	rc = synaptics_clearpad_suspend(&this->pdev->dev);
	if (rc)
		return rc;
#endif
	if (device_may_wakeup(dev)) {
		enable_irq_wake(this->pdata->irq);
		dev_info(&this->pdev->dev, "enable irq wake");
	}
	return 0;
}

static int synaptics_clearpad_pm_resume(struct device *dev)
{
	struct synaptics_clearpad *this = dev_get_drvdata(dev);
	unsigned long flags;
	bool irq_pending;

	if (device_may_wakeup(dev)) {
		disable_irq_wake(this->pdata->irq);
		dev_info(&this->pdev->dev, "disable irq wake");
	}
#ifndef CONFIG_HAS_EARLYSUSPEND
	rc = synaptics_clearpad_resume(&this->pdev->dev);
	if (rc)
		return rc;
#endif
	spin_lock_irqsave(&this->slock, flags);
	irq_pending = this->irq_pending;
	this->dev_busy = false;
	spin_unlock_irqrestore(&this->slock, flags);

	if (unlikely(irq_pending)) {
		dev_dbg(&this->pdev->dev, "Process pending IRQ\n");
		synaptics_clearpad_process_irq(this);
	}
	return 0;
}

static int synaptics_clearpad_pm_suspend_noirq(struct device *dev)
{
	struct synaptics_clearpad *this = dev_get_drvdata(dev);
	if (this->irq_pending && device_may_wakeup(dev)) {
		dev_info(&this->pdev->dev, "Need to resume\n");
		return -EBUSY;
	}
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void synaptics_clearpad_early_suspend(struct early_suspend *handler)
{
	struct synaptics_clearpad *this =
	container_of(handler, struct synaptics_clearpad, early_suspend);

	dev_info(&this->pdev->dev, "early suspend\n");
	synaptics_clearpad_suspend(&this->pdev->dev);
}

static void synaptics_clearpad_late_resume(struct early_suspend *handler)
{
	struct synaptics_clearpad *this =
	container_of(handler, struct synaptics_clearpad, early_suspend);

	dev_info(&this->pdev->dev, "late resume\n");
	synaptics_clearpad_resume(&this->pdev->dev);
}
#endif
#ifdef CONFIG_DEBUG_FS
static int clearpad_get_num_tx_physical(struct synaptics_clearpad *this,
		int num_tx)
{
	int i, rc;
	u8 *buf;
	int num_tx_physical = -1;

	buf = kzalloc(num_tx, GFP_KERNEL);
	if (!buf)
		goto exit;
	rc = synaptics_read(this, SYNF(F55_SENSOR, CTRL, 0x2),
			buf, num_tx);
	if (rc)
		goto error_free;
	for (i = 0 ; i < num_tx ; i++) {
		if (num_tx_physical < buf[i])
			num_tx_physical = buf[i];
		dev_dbg(&this->pdev->dev, "buf[%d], read data[%x], max = %d\n",
			i, buf[i], num_tx_physical);
	}
	num_tx_physical++;
error_free:
	kfree(buf);
exit:
	return num_tx_physical;
}

static void clearpad_analog_test_get_loop_count(struct synaptics_clearpad *this,
		u8 f_analog, u8 mode, int num_tx, int num_rx,
		int *loop_count_i, int *loop_count_j, int *data_type)
{
	if (f_analog == SYN_F05_ANALOG) {
		switch (mode) {
		case HWTEST_BASELINE:
			*loop_count_i = num_tx;
			*loop_count_j = num_rx;
			*data_type = HWTEST_S16;
			break;
		case HWTEST_SIGNAL_DIFF:
			*loop_count_i = num_tx;
			*loop_count_j = num_rx;
			*data_type = HWTEST_S8;
			break;
		default:
			*loop_count_i = *loop_count_j = *data_type = 0;
			break;
		}
	} else if (f_analog == SYN_F54_ANALOG) {
		switch (mode) {
		case F54_16_IMAGE_REPORT:
		case F54_AUTOSCAN_REPORT:
		case F54_RAW_CAPACITANCE_REPORT:
		case F54_RAW_CAP_RX_COUPLING_REPORT:
			*loop_count_i = num_tx;
			*loop_count_j = num_rx;
			*data_type = HWTEST_S16;
			break;
		case F54_HIGH_RESISTANCE_REPORT:
			*loop_count_i = HWTEST_SIZE_OF_ONE_DIMENSION;
			*loop_count_j = HWTEST_SIZE_OF_ONE_HIGH_RX;
			*data_type = HWTEST_S16;
			break;
		case F54_TX_TO_TX_SHORT_REPORT:
		case F54_TX_TO_GROUND_REPORT:
			num_tx = clearpad_get_num_tx_physical(this, num_tx);
			if (num_tx > 0) {
				*loop_count_i = HWTEST_SIZE_OF_ONE_DIMENSION;
				*loop_count_j =
					HWTEST_SIZE_OF_TX_TO_TX_SHORT(num_tx);
				*data_type = HWTEST_U8;
			} else {
				*loop_count_i = *loop_count_j = *data_type = 0;
			}
			break;
		case F54_RX_TO_RX1_REPORT:
			*loop_count_i = min(num_tx, num_rx);
			*loop_count_j = num_rx;
			*data_type = HWTEST_S16;
			break;
		case F54_RX_TO_RX2_REPORT:
			*loop_count_i = num_rx > num_tx ? num_rx - num_tx : 0;
			*loop_count_j = num_rx;
			*data_type = HWTEST_S16;
			break;
		default:
			*loop_count_i = *loop_count_j = *data_type = 0;
			break;
		}
	} else {
		*loop_count_i = *loop_count_j = *data_type = 0;
	}
	dev_info(&this->pdev->dev,
			"loop_count_i[%d], loop_count_j[%d], data_type[%d]\n",
			*loop_count_i, *loop_count_j, *data_type);
}

static void synaptics_clearpad_analog_test(struct synaptics_clearpad *this,
		u8 f_analog, u8 mode, u8 count)
{
	int rc, i, j, k, num_tx, num_rx;
	int loop_count_i, loop_count_j, data_type, data_size;
	u8 buf[2], *data, *line, *pl;
	enum synaptics_state state;
	const char str1[] = " | ";
	const char str2[] = "\n";

	memset(buf, 0, sizeof(buf));

	LOCK(this);
	this->task = SYN_TASK_NO_SUSPEND;
	if (this->active & SYN_STANDBY) {
		this->active &= ~SYN_STANDBY;
		this->active |= SYN_STANDBY_AFTER_TASK;
		LOG_STAT(this, "active: %x (task: %s)\n",
			 this->active, task_name[this->task]);
	}
	UNLOCK(this);
	rc = synaptics_clearpad_set_power(this);
	if (rc)
		goto err_retrun;

	rc = synaptics_put(this, SYNF(F01_RMI, CTRL, 0x01),
			this->pdt[f_analog].irq_mask);
	if (rc)
		goto set_power;

	if (f_analog == SYN_F05_ANALOG) {
		rc = synaptics_read(this, SYNF(F05_ANALOG, QUERY, 0x00),
				buf, 2);
		if (rc)
			goto err_set_irq_xy;
	} else if (f_analog == SYN_F54_ANALOG) {
		if (this->pdt[SYN_F11_2D].number
				== function_value[SYN_F11_2D]) {
			rc = synaptics_read(this, SYNF(F11_2D, CTRL, 0x34),
					buf, 2);
			if (rc)
				goto err_set_irq_xy;
		}
	}
	num_rx = buf[0];
	num_tx = buf[1];

	if (f_analog == SYN_F05_ANALOG) {
		num_rx &= ANALOG_QUERY_NUM_OF_ELECTRODES;
		num_tx &= ANALOG_QUERY_NUM_OF_ELECTRODES;
		rc = synaptics_put(this, SYNF(F05_ANALOG, CTRL, 0x00),
				ANALOG_CONTROL_NO_AUTO_CAL);
		if (rc)
			goto err_set_irq_xy;
	} else if (f_analog == SYN_F54_ANALOG &&
			mode != F54_16_IMAGE_REPORT &&
			mode != F54_AUTOSCAN_REPORT) {
		rc = synaptics_put_bit(this, SYNF(F54_ANALOG, CTRL, 0x08),
				0, ANALOG_CONTROL_CBC_CAPACITANCE);
		if (rc)
			goto err_set_irq_xy;
		rc = synaptics_put_bit(this, SYNF(F54_ANALOG, CTRL, 0x08),
				0, ANALOG_CONTROL_CBC_POLARITY);
		if (rc)
			goto err_set_irq_xy;
		rc = synaptics_put_bit(this, SYNF(F54_ANALOG, CTRL, 0x08),
				0, ANALOG_CONTROL_CBC_XMTR_CARRIER_SELECT);
		if (rc)
			goto err_set_irq_xy;
		rc = synaptics_put_bit(this, SYNF(F54_ANALOG, CTRL, 0x51),
				ANALOG_CONTROL_NO_SIGNALCLARITY,
				ANALOG_CONTROL_NO_SIGNALCLARITY);
		if (rc)
			goto err_set_irq_xy;
		rc = synaptics_put_bit(this, SYNF(F54_ANALOG, COMMAND, 0x00),
				ANALOG_COMMAND_FORCE_UPDATE,
				ANALOG_COMMAND_FORCE_UPDATE);
		if (rc)
			goto err_set_irq_xy;
		rc = synaptics_read(this,
				SYNF(F54_ANALOG, COMMAND, 0x00), buf, 1);
		if (rc)
			goto err_set_irq_xy;
		for (i = 0 ; (buf[0] & ANALOG_COMMAND_FORCE_UPDATE) != 0 ; i++) {
			usleep_range(10000, 11000);
			rc = synaptics_read(this,
					SYNF(F54_ANALOG, COMMAND, 0x00), buf, 1);
			if (rc || i > 100)
				goto err_set_irq_xy;
			dev_info(&this->pdev->dev,
				 "Force update flag = %x, loop = %d\n", buf[0], i);
		}
		rc = synaptics_put_bit(this, SYNF(F54_ANALOG, COMMAND, 0x00),
				ANALOG_COMMAND_FORCE_CAL,
				ANALOG_COMMAND_FORCE_CAL);
		if (rc)
			goto err_set_irq_xy;
		rc = synaptics_read(this,
				SYNF(F54_ANALOG, COMMAND, 0x00), buf, 1);
		if (rc || buf[0] & ANALOG_COMMAND_GET_REPORT)
			goto err_set_irq_xy;
		for (i = 0 ; (buf[0] & ANALOG_COMMAND_FORCE_CAL) != 0 ; i++) {
			usleep_range(10000, 11000);
			rc = synaptics_read(this,
					SYNF(F54_ANALOG, COMMAND, 0x00), buf, 1);
			if (rc || i > 100)
				goto err_set_irq_xy;
			dev_info(&this->pdev->dev,
				"Force cal flag = %x, loop = %d\n", buf[0], i);
		}
	}

	clearpad_analog_test_get_loop_count(this, f_analog, mode, num_tx,
			num_rx, &loop_count_i, &loop_count_j, &data_type);
	switch (data_type) {
	case HWTEST_U8:
	case HWTEST_S8:
		data_size = 1;
		break;
	case HWTEST_S16:
		data_size = 2;
		break;
	default:
		dev_err(&this->pdev->dev, "unsupported command\n");
		goto err_set_irq_xy;
	}

	data = kmalloc(num_rx * 2, GFP_KERNEL);
	if (!data)
		goto err_set_irq_xy;
	line = kmalloc(num_rx * 9 + 2, GFP_KERNEL);
	if (!line)
		goto err_kfree_data;

	for (k = 0; k < count; k++) {
		s16 min_val = SHRT_MAX, max_val = SHRT_MIN;
		dev_info(&this->pdev->dev,
				"ANALOG: mode[%d], num[%d], rx[%d], tx[%d]",
				mode, k, num_rx, num_tx);
		LOCK(this);
		this->state = SYN_STATE_WAIT_FOR_INT;
		UNLOCK(this);

		if (f_analog == SYN_F05_ANALOG) {
			rc = synaptics_put(this,
					SYNF(F05_ANALOG, DATA, 0x01), mode);
			if (rc)
				goto err_reset;
			rc = synaptics_put(this,
					SYNF(F05_ANALOG, COMMAND, 0x00),
					ANALOG_COMMAND_GET_IMAGE);
			if (rc)
				goto err_reset;
		} else if (f_analog == SYN_F54_ANALOG) {
			rc = synaptics_put(this,
					SYNF(F54_ANALOG, DATA, 0x00), mode);
			if (rc)
				goto err_reset;
			rc = synaptics_put(this,
					SYNF(F54_ANALOG, COMMAND, 0x00),
					ANALOG_COMMAND_GET_REPORT);
			if (rc)
				goto err_reset;
		}
		if (wait_event_interruptible(this->task_none_wq,
					synaptics_clearpad_check_task(this,
						&state)))
			goto err_reset;

		if (f_analog == SYN_F54_ANALOG) {
			rc = synaptics_put(this, SYNF(F54_ANALOG, DATA, 0x01),
					0x00);
			if (rc)
				goto err_reset;
			rc = synaptics_put(this, SYNF(F54_ANALOG, DATA, 0x02),
					0x00);
			if (rc)
				goto err_reset;
		}
		for (i = 0; i < loop_count_i; i++) {
			if (f_analog == SYN_F05_ANALOG) {
				rc = synaptics_put(this,
						SYNF(F05_ANALOG, DATA, 0x01),
						mode + i);
				if (rc)
					goto err_reset;
				rc = synaptics_read(this,
						SYNF(F05_ANALOG, DATA, 0x02),
						data, loop_count_j * 2);
				if (rc)
					goto err_reset;
			} else if (f_analog == SYN_F54_ANALOG) {
				for (j = 0; j < loop_count_j * data_size; j++) {
					rc = synaptics_read(this,
						SYNF(F54_ANALOG, DATA, 0x03),
						data + j, 1);
					if (rc)
						goto err_reset;
				}
			}
			pl = line;
			for (j = 0; j < loop_count_j; j++) {
				s16 val = 0;
				switch (data_type) {
				case HWTEST_U8:
					val = (u8)(*(data + j));
					break;
				case HWTEST_S8:
					val = (s8)(*(data + j));
					break;
				case HWTEST_S16:
					val = (s16)le16_to_cpup(
						(const u16 *)(data + j * 2));
					break;
				default:
					break;
				}
				if (val >= max_val)
					max_val = val;
				if (val <= min_val)
					min_val = val;
				pl += snprintf(pl, 6 + 1, "%6d", val);
				if ((j + 1) % loop_count_j)
					pl += snprintf(pl, sizeof(str1), str1);
				else
					pl += snprintf(pl, sizeof(str2), str2);
			}
			dev_info(&this->pdev->dev, "%s", line);
		}
		dev_info(&this->pdev->dev,
				"MIN = %06d / MAX = %06d\n", min_val, max_val);
		msleep(100);
	}

err_reset:
	kfree(line);
	kfree(data);
	LOCK(this);
	this->state = SYN_STATE_WAIT_FOR_INT;
	UNLOCK(this);
	synaptics_put(this, SYNF(F01_RMI, COMMAND, 0x00), DEVICE_COMMAND_RESET);
	wait_event_interruptible(this->task_none_wq,
				synaptics_clearpad_check_task(this, &state));
	goto set_power;

err_kfree_data:
	kfree(data);
err_set_irq_xy:
	if (this->pdt[SYN_F11_2D].number == function_value[SYN_F11_2D])
		synaptics_put(this, SYNF(F01_RMI, CTRL, 0x01),
				this->pdt[SYN_F11_2D].irq_mask);
set_power:
	LOCK(this);
	this->task = SYN_TASK_NONE;
	if (this->active & SYN_STANDBY_AFTER_TASK) {
		this->active &= ~SYN_STANDBY_AFTER_TASK;
		this->active |= SYN_STANDBY;
		LOG_STAT(this, "active: %x (task: %s)\n",
			 this->active, task_name[this->task]);
	}
	UNLOCK(this);
	synaptics_clearpad_set_power(this);
err_retrun:
	return;
}

static ssize_t synaptics_clearpad_debug_hwtest_open(struct inode *inode,
		struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t synaptics_clearpad_debug_hwtest_write(struct file *file,
		const char __user *buf, size_t count, loff_t *pos)
{
	struct synaptics_clearpad *this =
		(struct synaptics_clearpad *)file->private_data;
	int rc;
	unsigned long arg = 0;
	u8 reg, value, page;
	ssize_t retval;

	if (strnlen(buf, PAGE_SIZE) <= HWTEST_SIZE_OF_COMMAND_PREFIX)
		goto err_invalid_arg;

	rc = kstrtoul_from_user(buf + HWTEST_SIZE_OF_COMMAND_PREFIX,
				count - HWTEST_SIZE_OF_COMMAND_PREFIX,
				16, &arg);
	if (rc < 0)
		goto err_invalid_arg;

	switch (buf[0]) {
	case 'R':
		page = arg >> 8;
		reg = arg;
		LOCK(this);
		rc = synaptics_clearpad_page_sel(this, page);
		if (rc) {
			UNLOCK(this);
			goto err_return;
		}
		rc = regs_read(this, reg, &value, 1);
		if (!rc)
			dev_info(&this->pdev->dev,
				"read page=0x%02x, addr=0x%02x, value=0x%02x\n",
				page, reg, value);
		UNLOCK(this);
		break;
	case 'W':
		page = arg >> 16;
		reg = arg >> 8;
		value = arg;
		LOCK(this);
		rc = synaptics_clearpad_page_sel(this, page);
		if (rc) {
			UNLOCK(this);
			goto err_return;
		}
		rc = regs_write(this, reg, &value, 1);
		if (!rc)
			dev_info(&this->pdev->dev,
				"write page=0x%02x, addr=0x%02x, value=0x%02x\n",
				page, reg, value);
		UNLOCK(this);
		break;
	case 'S':
		if (this->pdt[SYN_F05_ANALOG].number ==
				function_value[SYN_F05_ANALOG]) {
			synaptics_clearpad_analog_test(this, SYN_F05_ANALOG,
					HWTEST_SIGNAL_DIFF, arg);
			break;
		} else {
			goto err_invalid_arg;
		}
	case 'B':
		if (this->pdt[SYN_F05_ANALOG].number ==
				function_value[SYN_F05_ANALOG]) {
			synaptics_clearpad_analog_test(this, SYN_F05_ANALOG,
					HWTEST_BASELINE, arg);
			break;
		} else {
			goto err_invalid_arg;
		}
	case 'A':
		if (this->pdt[SYN_F54_ANALOG].number ==
				function_value[SYN_F54_ANALOG]) {
			value = arg;
			reg = arg >> 8;
			synaptics_clearpad_analog_test(this,
					SYN_F54_ANALOG, reg, value);
			break;
		} else {
			goto err_invalid_arg;
		}
	case 'P':
		LOCK(this);
		synaptics_clearpad_reset_power(this);
		UNLOCK(this);
		break;
	default:
		break;
	}
	retval = strnlen(buf, PAGE_SIZE);
	goto exit;

err_invalid_arg:
	dev_err(&this->pdev->dev, "illegal command\n");
err_return:
	retval = -EINVAL;
exit:
	return retval;
}

static const struct file_operations synaptics_clearpad_debug_hwtest_fops = {
	.owner = THIS_MODULE,
	.open = synaptics_clearpad_debug_hwtest_open,
	.write = synaptics_clearpad_debug_hwtest_write,
};

static void synaptics_clearpad_debug_init(struct synaptics_clearpad *this)
{
	struct dentry *dent = NULL;

	dent = debugfs_create_dir("clearpad", 0);
	if (!dent || IS_ERR(dent)) {
		dev_err(&this->pdev->dev,
			"%s: debugfs_create_dir error: dent=0x%x\n",
			__func__, (unsigned)dent);
		goto exit;
	}

	this->debugfs = dent;

	dent = debugfs_create_file("hwtest", 0600, this->debugfs,
				(void *)this,
				&synaptics_clearpad_debug_hwtest_fops);
	if (!dent || IS_ERR(dent)) {
		dev_err(&this->pdev->dev,
			"%s: debugfs_create_file error: dent=0x%x\n",
			__func__, (unsigned)dent);
		goto error;
	}

	goto exit;

error:
	debugfs_remove_recursive(this->debugfs);
	this->debugfs = NULL;
exit:
	return;
}
#endif /* CONFIG_DEBUG_FS */

static int __devinit clearpad_probe(struct platform_device *pdev)
{
	struct clearpad_data *cdata = pdev->dev.platform_data;
	struct synaptics_clearpad *this;
	struct kobject *parent;
	char *symlink_name;
	int rc;

	this = kzalloc(sizeof(struct synaptics_clearpad), GFP_KERNEL);
	if (!this) {
		rc = -ENOMEM;
		goto exit;
	}

	mutex_init(&this->lock);
	spin_lock_init(&this->slock);
	init_waitqueue_head(&this->task_none_wq);

	dev_set_drvdata(&pdev->dev, this);
	this->pdev = pdev;
	this->pdata = cdata->pdata;
	if (!this->pdata) {
		dev_err(&this->pdev->dev, "no platform data\n");
		rc = -EINVAL;
		goto err_free;
	}
	this->bdata = cdata->bdata;
	if (!this->bdata) {
		dev_err(&this->pdev->dev, "no bus data\n");
		rc = -EINVAL;
		goto err_free;
	}
	memcpy(&this->easy_wakeup_config, this->pdata->easy_wakeup_config,
					sizeof(this->easy_wakeup_config));
#ifdef CONFIG_TOUCHSCREEN_CLEARPAD_RMI_DEV
	this->rmi_dev = platform_device_alloc(CLEARPAD_RMI_DEV_NAME, -1);
	if (!this->rmi_dev) {
		rc = -ENOMEM;
		goto err_free;
	}

	this->rmi_dev->dev.parent = &pdev->dev;
	rc = platform_device_add_data(this->rmi_dev, cdata,
					sizeof(struct clearpad_data));
	if (rc)
		goto err_device_put;

	rc = platform_device_add(this->rmi_dev);
	if (rc)
		goto err_device_put;

	if (!this->rmi_dev->dev.driver) {
		rc = -ENODEV;
		goto err_device_del;
	}
#endif

	if (this->pdata->vreg_configure) {
		rc = this->pdata->vreg_configure(this->pdev->dev.parent, 1);
		if (rc) {
			dev_err(&this->pdev->dev,
			       "failed vreg init\n");
			goto err_device_del;
		}
	}

	if (this->pdata->gpio_configure) {
		rc = this->pdata->gpio_configure(1);
		if (rc) {
			dev_err(&this->pdev->dev,
			       "failed gpio init\n");
			goto err_vreg_teardown;
		}
	}

	msleep(400);

	if (this->pdata->watchdog_enable) {
		this->wd_poll_t_jf = this->pdata->watchdog_poll_t_ms ?
			msecs_to_jiffies(this->pdata->watchdog_poll_t_ms) :
			SYNAPTICS_WATCHDOG_POLL_DEFAULT_INTERVAL;
		INIT_DELAYED_WORK(&this->wd_poll_work,
				synaptics_clearpad_wd_status_poll);
	}

	LOCK(this);
	rc = synaptics_clearpad_initialize(this);
	LOG_CHECK(this, "rc=%d\n", rc);
	UNLOCK(this);
	if (rc)
		goto err_gpio_teardown;

	rc = synaptics_clearpad_input_init(this);
	if (rc)
		goto err_gpio_teardown;

	this->state = SYN_STATE_RUNNING;

#ifdef CONFIG_HAS_EARLYSUSPEND
	this->early_suspend.suspend = synaptics_clearpad_early_suspend;
	this->early_suspend.resume = synaptics_clearpad_late_resume;
	register_early_suspend(&this->early_suspend);
#endif

	/* sysfs */
	rc = create_sysfs_entries(this);
	if (rc)
		goto err_unregister_early_suspend;

#ifdef CONFIG_DEBUG_FS
	/* debugfs */
	synaptics_clearpad_debug_init(this);
#endif

	/* create symlink */
	parent = this->input->dev.kobj.parent;
	symlink_name = this->pdata->symlink_name ? : CLEARPAD_NAME;
	rc = sysfs_create_link(parent, &this->input->dev.kobj, symlink_name);
	if (rc) {
		dev_err(&this->pdev->dev, "sysfs_create_link error\n");
		goto err_sysfs_remove_group;
	}

	rc = request_threaded_irq(this->pdata->irq,
				synaptics_clearpad_hard_handler,
				synaptics_clearpad_threaded_handler,
				IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
				this->pdev->dev.driver->name,
				&this->pdev->dev);
	if (rc) {
		dev_err(&this->pdev->dev,
		       "irq %d busy?\n", this->pdata->irq);
		UNLOCK(this);
		goto err_sysfs_remove_link;
	}
	disable_irq_nosync(this->pdata->irq);

	rc = synaptics_clearpad_set_power(this);
	if (rc)
		goto err_irq;

	goto exit;

err_irq:
	free_irq(this->pdata->irq, &this->pdev->dev);
err_sysfs_remove_link:
	sysfs_remove_link(parent, symlink_name);
err_sysfs_remove_group:
#ifdef CONFIG_DEBUG_FS
	debugfs_remove_recursive(this->debugfs);
#endif
	remove_sysfs_entries(this);
err_unregister_early_suspend:
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&this->early_suspend);
#endif
	input_unregister_device(this->input);
err_gpio_teardown:
	if (this->pdata->gpio_configure)
		this->pdata->gpio_configure(0);
err_vreg_teardown:
	if (this->pdata->vreg_configure)
		this->pdata->vreg_configure(this->pdev->dev.parent, 0);
err_device_del:
	platform_device_del(this->rmi_dev);
err_device_put:
#ifdef CONFIG_TOUCHSCREEN_CLEARPAD_RMI_DEV
	platform_device_put(this->rmi_dev);
#endif
err_free:
	dev_set_drvdata(&pdev->dev, NULL);
	kfree(this);
exit:
	return rc;
}

static int __devexit clearpad_remove(struct platform_device *pdev)
{
	struct synaptics_clearpad *this = dev_get_drvdata(&pdev->dev);
	char *symlink_name = this->pdata->symlink_name ? : CLEARPAD_NAME;

	cancel_delayed_work_sync(&this->wd_poll_work);
	device_init_wakeup(&this->pdev->dev, 0);
	free_irq(this->pdata->irq, &this->pdev->dev);
	sysfs_remove_link(this->input->dev.kobj.parent, symlink_name);
#ifdef CONFIG_DEBUG_FS
	debugfs_remove_recursive(this->debugfs);
#endif
	remove_sysfs_entries(this);
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&this->early_suspend);
#endif
	input_unregister_device(this->input);
	if (this->pdata->gpio_configure)
		this->pdata->gpio_configure(0);
	if (this->pdata->vreg_configure)
		this->pdata->vreg_configure(this->pdev->dev.parent, 0);
#ifdef CONFIG_TOUCHSCREEN_CLEARPAD_RMI_DEV
	platform_device_put(this->rmi_dev);
#endif
	dev_set_drvdata(&pdev->dev, NULL);
	kfree(this);

	return 0;
}


static const struct dev_pm_ops synaptics_clearpad_pm = {
	.suspend = synaptics_clearpad_pm_suspend,
	.resume = synaptics_clearpad_pm_resume,
	.suspend_noirq = synaptics_clearpad_pm_suspend_noirq,
};

static struct platform_driver clearpad_driver = {
	.driver = {
		.name	= CLEARPAD_NAME,
		.owner	= THIS_MODULE,
		.pm	= &synaptics_clearpad_pm,
	},
	.probe		= clearpad_probe,
	.remove		= __devexit_p(clearpad_remove),
};

static int __init clearpad_init(void)
{
	return platform_driver_register(&clearpad_driver);
}

static void __exit clearpad_exit(void)
{
	platform_driver_unregister(&clearpad_driver);
}

module_init(clearpad_init);
module_exit(clearpad_exit);

MODULE_DESCRIPTION(CLEARPAD_NAME "ClearPad Driver");
MODULE_LICENSE("GPL v2");
