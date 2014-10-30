/* linux/drivers/input/touchscreen/clearpad_core.c
 *
 * Copyright (C) 2010 Sony Mobile Communications Inc.
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
#include <linux/input/evdt_helper.h>
#include <mach/gpio.h>
#include <linux/ctype.h>
#include <linux/firmware.h>
#include <linux/slab.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#endif
#include <linux/sched.h>
#ifdef CONFIG_ARM
#include <asm/mach-types.h>
#endif

#define SYN_CLEARPAD_VENDOR		0x1
#define SYN_MAX_N_FINGERS		10
#define SYN_FINGER_DATA_SIZE		5
#define SYN_DEVICE_STATUS		0x13
#define SYN_MAX_CTRL_VALUE		32
#define SYN_MAX_Z_VALUE			255
/* Todo: Consider F11 */
#define SYN_MAX_W_VALUE			15
#define SYN_PDT_START			0xEF
#define SYN_SIZE_OF_FD			6
#define SYN_PAGE_SELECT_OFFSET		0xFF
#define SYN_SUPPORTED_PAGE_NUM		0x04
#define SYN_MAX_INTERRUPT_SOURCE_COUNT	0x7
#define SYN_STRING_LENGTH		128
#define SYN_RETRY_NUM_OF_INITIAL_CHECK	2
#define SYN_RETRY_NUM_OF_PROBE_CHECK	3
#define SYN_RETRY_NUM_OF_RESET 5
#define SYN_PCA_ACCESS_MAX_READ_SIZE	15
#define SYN_PCA_ACCESS_MAX_WRITE_SIZE	16
#define SYN_FINGER_OFF(n, x, s) \
	((((n) / 4) + !!(n % 4)) + (s) * (x))
#define SYN_REG_MAX \
	SYN_FINGER_OFF(SYN_MAX_N_FINGERS, SYN_MAX_N_FINGERS, \
	SYN_FINGER_DATA_SIZE)
#define SYN_FINGER_STATE(buf, i) ((buf[i / 4] >> ((i % 4) * 2)) & 0x03)
#define HWTEST_SIZE_OF_COMMAND_PREFIX		2
#define HWTEST_SIZE_OF_ONE_DIMENSION		1
#define HWTEST_SIZE_OF_ONE_HIGH_RX		3
#define HWTEST_SIZE_OF_TX_TO_TX_SHORT(x)	(((x) + 7) / 8)
#define HWTEST_SIZE_OF_TRX_SHORT_2		7
#define HWTEST_SIZE_OF_TRX_SHORT_2_TAB		13
#define HWTEST_MAX_DIGITS			10
#define SYN_WATCHDOG_POLL_DEFAULT_INTERVAL	HZ
#define SYN_WAKEUP_GESTURE			"wakeup_gesture"

#define SYN_PAGE_ADDR(page, addr) ((page) << 8 | (addr))
#define SYN_F_ADDR(th, func, type, reg) ((th)->pdt[func].base[type] + (reg))
#define SYN_F_PAGE(th, func) ((th)->pdt[func].page)
#define SYN_F_PAGE_ADDR(th, func, type, reg) \
	SYN_PAGE_ADDR(SYN_F_PAGE(th, func), SYN_F_ADDR(th, func, type, reg))
#define SYNSET(...)  __VA_ARGS__
#define SYNI(th, func, type, reg) \
	SYNSET(th, SYN_F_PAGE_ADDR(th, func, type, reg))
#define SYNFUNC(x) SYN_##x
#define SYNTYPE(y) SYN_TYPE_##y
#define SYNF(th, func, type, reg) \
	SYNI(th, SYNFUNC(func), SYNTYPE(type), reg)
#define SYNOFF(th, func, type, sym) \
	((th)->pdt[SYNFUNC(func)].offset[SYNTYPE(type)][func##_##type##_##sym])
#define SYNS(th, func, type, sym) \
	SYNSET(th, SYN_F_PAGE_ADDR(th, SYNFUNC(func), SYNTYPE(type), \
				   SYNOFF(th, func, type, sym)))

#ifdef CONFIG_DEBUG_FS
#define DEBUG_COMMAND(C0, C1) (((int)C0 << 8) + (int)C1)
#define DEBUG_ONE_BYTE_HEX	2
#endif

#define DEVICE_STATUS_UNCONFIGURED_RESET_OCCURRED	0x81
#define DEVICE_STATUS_UNCONFIGURED_DEVICE_FAILURE	0x83
#define DEVICE_STATUS_DEVICE_FAILURE			0x03
#define DEVICE_STATUS_CONFIGURATION_CRC_FAILURE		0x04
#define DEVICE_STATUS_FIRMWARE_CRC_FAILURE		0x05
#define DEVICE_STATUS_CRC_IN_PROGRESS			0x06
#define DEVICE_COMMAND_RESET				0x01
#define DEVICE_COMMAND_REZERO				0x01
#define DEVICE_CONTROL_SLEEP_MODE			0x03
#define DEVICE_CONTROL_SLEEP_MODE_NORMAL_OPERATION	0x00
#define DEVICE_CONTROL_SLEEP_MODE_SENSOR_SLEEP		0x01
#define DEVICE_CONTROL_SLEEP_MODE_NO_SLEEP		0x04
#define DEVICE_CONTROL_CHARGER_CONNECTED		0x20
#define DEVICE_CONTROL_CHARGER_NOT_CONNECTED		0x00
#define DEVICE_CONTROL_CONFIGURED			0x80
#define DEVICE_PEN_SUPPORTED				0x01
#define PEN_DETECT_F11_INTERRUPT			0x01
#define PEN_DETECT_F12_INTERRUPT			0x02
#define PEN_DETECT_INT_ENABLE				0x01
#define PEN_DETECT_INT_DISABLE				0x00
#define GLOVE_MODE_ENABLE				0x01
#define GLOVE_MODE_DISABLE				0x02
#define GLOVE_MODE_SUPPORT				0x20
#define GLOVE_MODE_BIT_ENABLE				0x20
#define GLOVE_MODE_BIT_DISABLE				0x00
#define GLOVE_MODE_BIT_MASK				0x20
#define EW_DOUBLE_TAP_ENABLE				0x01
#define EW_DOUBLE_TAP_DISABLE				0x00
#define EW_RPT_GESTURE_ENABLE				0x02
#define EW_RPT_GESTURE_DISABLE				0x00
#define XY_REPORTING_MODE				0x07
#define XY_HAS_LPWG					0x10
#define XY_REPORTING_MODE_REDUCED_REPORTING_MODE	0x01
#define XY_REPORTING_MODE_WAKEUP_GESTURE_MODE		0x04
#define REPORT_GLOVE_AS_FINGER				0x20
#define FLASH_CONTROL_WRITE_FIRMWARE_BLOCK		0x02
#define FLASH_CONTROL_ERASE_ALL				0x03
#define FLASH_CONTROL_READ_CONFIGURATION_BLOCK		0x05
#define FLASH_CONTROL_WRITE_CONFIGURATION_BLOCK		0x06
#define FLASH_CONTROL_ERASE_CONFIGURATION		0x07
#define FLASH_CONTROL_ENABLE_FLASH_PROGRAMMING		0x0f
#define FLASH_CONTROL_PROGRAM_ENABLED			0x80
#define FLASH_DATA_CONFIGURATION_AREA			0x01
#define ANALOG_CONTROL_NO_AUTO_CAL			0x10
#define ANALOG_CONTROL_CBC_CAPACITANCE			0x07
#define ANALOG_CONTROL_CBC_POLARITY			0x08
#define ANALOG_CONTROL_CBC_XMTR_CARRIER_SELECT		0x10
#define ANALOG_CONTROL_CBC_XMTR_CARRIER_SELECT_2	0x20
#define ANALOG_CONTROL_TRANS_CBC_GLOBAL_CAP		0x01
#define ANALOG_CONTROL_NO_SIGNALCLARITY			0x01
#define ANALOG_QUERY_NUM_OF_ELECTRODES			0x3F
#define ANALOG_COMMAND_GET_IMAGE			0x04
#define ANALOG_COMMAND_GET_REPORT			0x01
#define ANALOG_COMMAND_FORCE_CAL			0x02
#define ANALOG_COMMAND_FORCE_UPDATE			0x04

#define CLEARPAD_VDD "touch_vdd"
#define CLEARPAD_VIO "touch_vio"

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

enum clearpad_state_e {
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

static const char * const clearpad_state_name[] = {
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

enum clearpad_task_e {
	SYN_TASK_NONE,
	SYN_TASK_NO_SUSPEND,
};

enum clearpad_chip_e {
	SYN_CHIP_3000	= 0x33,
	SYN_CHIP_3200	= 0x35,
	SYN_CHIP_3400	= 0x36,
	SYN_CHIP_3500	= 0x38,
	SYN_CHIP_7300	= 0x37,
	SYN_CHIP_7500	= 0x39,
};

static const char * const clearpad_task_name[] = {
	[SYN_TASK_NONE]		= "none",
	[SYN_TASK_NO_SUSPEND]	= "executing task",
};

enum clearpad_active_e {
	SYN_ACTIVE_POWER	= (1 << 0),
	SYN_STANDBY		= (1 << 1),
	SYN_STANDBY_AFTER_TASK	= (1 << 2),
};

enum clearpad_function_e {
	SYN_F01_RMI,
	SYN_F05_ANALOG,
	SYN_F11_2D,
	SYN_F12_2D,
	SYN_F34_FLASH,
	SYN_F51_CUSTOM,
	SYN_F54_ANALOG,
	SYN_F55_SENSOR,
	SYN_N_FUNCTIONS,
};

static const u8 clearpad_function_value[] = {
	[SYN_F01_RMI]		= 0x01,
	[SYN_F05_ANALOG]	= 0x05,
	[SYN_F11_2D]		= 0x11,
	[SYN_F12_2D]		= 0x12,
	[SYN_F34_FLASH]		= 0x34,
	[SYN_F51_CUSTOM]	= 0x51,
	[SYN_F54_ANALOG]	= 0x54,
	[SYN_F55_SENSOR]	= 0x55,
	[SYN_N_FUNCTIONS]	= 0x00,
};

enum clearpad_reg_type_e {
	SYN_TYPE_DATA,
	SYN_TYPE_CTRL,
	SYN_TYPE_COMMAND,
	SYN_TYPE_QUERY,
	SYN_TYPE_END,
};

static const char * const clearpad_flash_status[] = {
	[0] = "Success",
	[1] = "(Reserved)",
	[2] = "Flash Programming Not Enabled/Bad Command",
	[3] = "Invalid Block Number",
	[4] = "Block Not Erased",
	[5] = "Erase Key Incorrect",
	[6] = "Unknown Erase/Program Failure",
	[7] = "Device has been reset",
};

static const char * const clearpad_flash_reason[] = {
	[4] = "Configuration CRC Failure",
	[5] = "Firmware CRC Failure",
	[6] = "CRC In Progress",
};

enum clearpad_firmware_e {
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

static const int clearpad_number_of_fingers[] = {
	[0] = 1,
	[1] = 2,
	[2] = 3,
	[3] = 4,
	[4] = 5,
	[5] = 10,
	[6] = 0,
	[7] = 0,
};

enum clearpad_device_serialization_queries_e {
	SIZE_OF_DATE_CODE	= 3,
	SIZE_OF_TESTER_ID	= 2,
	SIZE_OF_SERIAL_NUMBER	= 2,
};

enum clearpad_flush_command_e {
	SYN_LOAD_START,
	SYN_LOAD_END,
	SYN_FORCE_FLUSH,
	SYN_CONFIG_FLUSH,
};

static const char * const clearpad_flush_commands[] = {
	[SYN_LOAD_START]	= "load_start",
	[SYN_LOAD_END]		= "load_end",
	[SYN_FORCE_FLUSH]	= "force_flush",
	[SYN_CONFIG_FLUSH]	= "config_flush",
};

enum clearpad_flash_mode_e {
	SYN_FLASH_MODE_NORMAL = 0,
	SYN_FLASH_MODE_CONFIG = 1,
};

struct clearpad_device_info_t {
	u8 manufacturer_id;
	u8 product_properties;
	u8 customer_family;
	u8 firmware_revision_major;
	u8 firmware_revision_minor;
	u8 firmware_revision_extra;
	u8 date[SIZE_OF_DATE_CODE];
	u8 tester_id[SIZE_OF_TESTER_ID];
	u8 serial_number[SIZE_OF_SERIAL_NUMBER];
	u8 product_id[HEADER_PRODUCT_ID_SIZE];
};

struct clearpad_point_t {
	int id;
	int x;
	int y;
	int wx;
	int wy;
	int z;
	int tool;
};

enum clearpad_tool_e {
	SYN_TOOL_FINGER	= 0x01,
	SYN_TOOL_PEN	= 0x02,
	SYN_TOOL_GLOVE	= 0x03,
};

enum clearpad_tools_type_f11_e {
	SYN_F11_TOOL_TYPE_NOOBJ,
	SYN_F11_TOOL_TYPE_FINGER,
	SYN_F11_TOOL_TYPE_PEN,
	SYN_F11_TOOL_TYPE_GLOVE,
};

static const int clearpad_tool_type_f11[] = {
	[SYN_F11_TOOL_TYPE_NOOBJ]	= 0x00,
	[SYN_F11_TOOL_TYPE_FINGER]	= SYN_TOOL_FINGER,
	[SYN_F11_TOOL_TYPE_PEN]		= SYN_TOOL_PEN,
	[SYN_F11_TOOL_TYPE_GLOVE]	= SYN_TOOL_GLOVE,
};

enum clearpad_tools_type_f12_e {
	SYN_F12_TOOL_TYPE_NOOBJ,
	SYN_F12_TOOL_TYPE_FINGER,
	SYN_F12_TOOL_TYPE_PEN,
	SYN_F12_TOOL_TYPE_PALM,
	SYN_F12_TOOL_TYPE_UNCLASS,
	SYN_F12_TOOL_TYPE_RESERVE,
	SYN_F12_TOOL_TYPE_GLOVE,
	SYN_F12_TOOL_TYPE_NARROW,
	SYN_F12_TOOL_TYPE_HANDEDGE,
};

static const int clearpad_tool_type_f12[] = {
	[SYN_F12_TOOL_TYPE_NOOBJ]	= 0x00,
	[SYN_F12_TOOL_TYPE_FINGER]	= SYN_TOOL_FINGER,
	[SYN_F12_TOOL_TYPE_PEN]		= SYN_TOOL_PEN,
	[SYN_F12_TOOL_TYPE_PALM]	= 0x00,
	[SYN_F12_TOOL_TYPE_UNCLASS]	= 0x00,
	[SYN_F12_TOOL_TYPE_RESERVE]	= 0x00,
	[SYN_F12_TOOL_TYPE_GLOVE]	= SYN_TOOL_GLOVE,
	[SYN_F12_TOOL_TYPE_NARROW]	= 0x00,
	[SYN_F12_TOOL_TYPE_HANDEDGE]	= 0x00,
};

struct clearpad_pointer_t {
	bool down;
	struct clearpad_funcarea_t *funcarea;
	struct clearpad_point_t cur;
};

struct clearpad_function_descriptor_t {
	u8 number;
	u8 int_count;
	u8 irq_mask;
	u8 page;
	u8 base[SYN_TYPE_END];
	u8 offset[SYN_TYPE_END][SYN_MAX_CTRL_VALUE];
};

struct clearpad_flash_block_t {
	int blocks;
	int length;
	int pos;
	const u8 *data;
};

struct clearpad_flash_image_t {
	u8 *image;
	size_t size;
	size_t buffer_size; /* allocated buffer size */
	u8 format_version;
	int firmware_size;
	int config_size;
	u8 customer_family;
	u8 firmware_revision_major;
	u8 firmware_revision_minor;
	u8 firmware_revision_extra;
	u8 product_id[HEADER_PRODUCT_ID_SIZE];
	struct clearpad_flash_block_t data;
	struct clearpad_flash_block_t config;
};

struct clearpad_extents_t {
	int x_min, y_min;
	int x_max, y_max;
	int n_fingers;
	int n_bytes_per_object;
};

struct clearpad_charger_t {
	bool supported;
	bool status;
};

struct clearpad_pen_t {
	bool supported;
	bool enabled;
};

struct clearpad_glove_t {
	bool supported;
	bool enabled;
};

struct clearpad_cover_t {
	bool supported;
	bool status;
	bool enabled;
	int win_top;
	int win_bottom;
	int win_right;
	int win_left;
};

struct clearpad_wakeup_gesture_t {
	bool supported;
	bool enabled;
	bool lpm_disabled;
	bool large_panel;
	unsigned long time_started;
	u32 timeout_delay;
};

enum clearpad_hwtest_data_type_e {
	HWTEST_NULL,
	HWTEST_U8,
	HWTEST_S8,
	HWTEST_S16,
	HWTEST_U32,
};

enum clearpad_f05_command_e {
	HWTEST_BASELINE = 0x40,
	HWTEST_SIGNAL_DIFF = 0x80,
};

enum clearpad_f54_command_e {
	F54_16_IMAGE_REPORT		= 2,
	F54_AUTOSCAN_REPORT		= 3,
	F54_HIGH_RESISTANCE_REPORT	= 4,
	F54_TX_TO_TX_SHORT_REPORT	= 5,
	F54_RX_TO_RX1_REPORT		= 7,
	F54_TX_TO_GROUND_REPORT		= 16,
	F54_RX_TO_RX2_REPORT		= 17,
	F54_RAW_CAPACITANCE_REPORT	= 19,
	F54_RAW_CAP_RX_COUPLING_REPORT	= 20,
	F54_SENSOR_SPEED_REPORT		= 22,
	F54_TRX_TO_TRX_SHORT_2_REPORT	= 26,
	F54_ABS_RAW_REPORT		= 38,
	F54_TRX_TO_TRX_SHORT_REPORT	= 43,
};

enum clearpad_f12_2d_ctrl_registers_id_e {
	F12_2D_CTRL_SENSOR_TUNING	= 8,
	F12_2D_CTRL_FINGER_TUNING	= 9,
	F12_2D_CTRL_OBJECT_TRACKING	= 10,
	F12_2D_CTRL_POSITION_TUNING	= 11,
	F12_2D_CTRL_CLASSIFIER		= 15,
	F12_2D_CTRL_RPT_CONTROL		= 20,
	F12_2D_CTRL_RPT_FILTER_PARAMS	= 22,
	F12_2D_CTRL_OBJECT_RPT_ENABLE	= 23,
	F12_2D_CTRL_N_BYTES_PER_OBJECT	= 28,
	F12_2D_CTRL_REG_MAX
};

enum clearpad_f12_2d_reporting_control_e {
	F12_2D_CTRL_MOTION_SUPP_X	= 0,
	F12_2D_CTRL_MOTION_SUPP_Y,
	F12_2D_CTRL_RPT_FLAG,
	F12_2D_CTRL_HOVER_RPT,
	F12_2D_CTRL_RPT_REG_MAX
};

enum clearpad_f12_2d_object_report_enable_e {
	F12_2D_CTRL_OBJ_REPORT_ENABLE	= 0,
	F12_2D_CTRL_MAX_OBJ_REPORT,
	F12_2D_CTRL_REPORT_AS_FINGER
};

enum clearpad_f12_2d_data_registers_id_e {
	F12_2D_DATA_SENSED_OBJECTS	= 1,
	F12_2D_DATA_OBJ_ATTENTION	= 15,
	F12_2D_DATA_REG_MAX
};

struct clearpad_t {
	enum   clearpad_state_e state;
	enum   clearpad_task_e task;
	enum   clearpad_chip_e chip_id;
	struct regulator *vreg_touch_vdd;
	struct regulator *vreg_touch_vio;
	struct input_dev *input;
	struct platform_device *pdev;
	struct clearpad_platform_data_t *pdata;
	struct clearpad_bus_data_t *bdata;
	struct mutex lock;
	struct clearpad_device_info_t device_info;
	struct clearpad_funcarea_t *funcarea;
	struct clearpad_pointer_t pointer[SYN_MAX_N_FINGERS];
	struct clearpad_function_descriptor_t pdt[SYN_N_FUNCTIONS];
	struct clearpad_flash_image_t flash;
	struct clearpad_wakeup_gesture_t wakeup_gesture;
	struct clearpad_charger_t charger;
	struct clearpad_pen_t pen;
	struct clearpad_glove_t glove;
	struct clearpad_cover_t cover;
	bool fwdata_available;
	bool pca_busy;
	enum clearpad_flash_mode_e flash_mode;
	struct clearpad_extents_t extents;
	int active;
	int irq;
	int irq_mask;

	int screen_status;

	char fwname[SYN_STRING_LENGTH + 1];
	char result_info[SYN_STRING_LENGTH + 1];
	wait_queue_head_t task_none_wq;
	bool flash_requested;
	u8 page_num;
	u8 reg_buf[SYN_REG_MAX];
#ifdef CONFIG_DEBUG_FS
	struct dentry *debugfs;
#endif
	u32 gpio_reset;
	u32 reset_l2h;
	u32 num_sensor_rx;
	u32 num_sensor_tx;
	u32 touch_pressure_enabled;
	u32 touch_size_enabled;
	u32 touch_orientation_enabled;
	struct device_node *evdt_node;
	struct delayed_work wd_poll_work;
	int wd_poll_t_jf;
	spinlock_t slock;
	bool dev_busy;
	bool irq_pending;
	u8 default_reporting_mode;
	u32 por_delay_after;
	u32 reset_count;
	const char *reset_cause;
};

static void clearpad_funcarea_initialize(struct clearpad_t *this);
static void clearpad_reset_power(struct clearpad_t *this,
					   const char *cause);
static void clearpad_resume(struct device *dev);
static void clearpad_suspend(struct device *dev);

static char *clearpad_s(u8 *array, size_t size)
{
	static char string[SYN_STRING_LENGTH + 1];

	memset(string, 0, SYN_STRING_LENGTH + 1);
	size = (SYN_STRING_LENGTH < size) ? SYN_STRING_LENGTH : size;
	memcpy(string, array, size);

	return string;
}

static inline bool clearpad_is_valid_function(struct clearpad_t *this, int func)
{
	return (0 <= func && func < SYN_N_FUNCTIONS)
		&& (this->pdt[func].number == clearpad_function_value[func]);
}

/* Basic bus access */

static int clearpad_set_page(struct clearpad_t *this, u8 page)
{
	return this->bdata->set_page(this->pdev->dev.parent, page);
}

static int clearpad_read(struct clearpad_t *this, u16 addr,
			 u8 *buf, int len)
{
	return this->bdata->read(this->pdev->dev.parent, addr, buf, len);
}

static int clearpad_write(struct clearpad_t *this, u16 addr,
			  const u8 *buf, u8 len)
{
	return this->bdata->write(this->pdev->dev.parent, addr, buf, len);
}

static int clearpad_read_block(struct clearpad_t *this, u16 addr,
		u8 *buf, int len)
{
	return this->bdata->read_block(this->pdev->dev.parent,
				       addr, buf, len);
}

static int clearpad_write_block(struct clearpad_t *this, u16 addr,
		const u8 *buf, int len)
{
	return this->bdata->write_block(this->pdev->dev.parent,
					addr, buf, len);
}

/* Read/Write data of specific size */

static int clearpad_get(struct clearpad_t *this, u16 addr, u8 *val)
{
	int rc = clearpad_read(this, addr, val, 1);
	return  rc == 1 ? 0 : (rc < 0 ? rc : -EIO);
}

static int clearpad_put(struct clearpad_t *this, u16 addr, u8 val)
{
	int rc = clearpad_write(this, addr, &val, 1);
	return  rc == 1 ? 0 : (rc < 0 ? rc : -EIO);
}

static int clearpad_put_bit(struct clearpad_t *this, u16 addr, u8 val, u8 mask)
{
	int rc;
	u8 buf;

	rc = clearpad_get(this, addr, &buf);
	if (rc)
		goto err_return;

	buf = (buf & ~mask) | val;
	rc = clearpad_put(this, addr, buf);
err_return:
	return rc;
}

static int clearpad_get_block(struct clearpad_t *this, u16 addr,
			      u8 *buf, int len)
{
	int rc;

	rc = clearpad_read_block(this, addr, buf, len);
	return  rc == len ? 0 : (rc < 0 ? rc : -EIO);
}

static int clearpad_put_block(struct clearpad_t *this, u16 addr,
			      const u8 *buf, int len)
{
	int rc;

	rc = clearpad_write_block(this, addr, buf, len);
	return  rc == len ? 0 : (rc < 0 ? rc : -EIO);
}

static struct clearpad_funcarea_t clearpad_default_funcarea_array[] = {
	{
		{ 0, 0, 0, 0}, { 0, 0, 0, 0},
		SYN_FUNCAREA_POINTER, NULL
	},
	{ .func = SYN_FUNCAREA_END }
};

static struct clearpad_funcarea_t *clearpad_funcarea_get(
		struct clearpad_t *this, u8 module_id, u8 rev)
{
	clearpad_default_funcarea_array[0].original.x2 = this->extents.x_max;
	clearpad_default_funcarea_array[0].original.y2 = this->extents.y_max;
	clearpad_default_funcarea_array[0].extension.x2 = this->extents.x_max;
	clearpad_default_funcarea_array[0].extension.y2 = this->extents.y_max;
	return clearpad_default_funcarea_array;
}

static int clearpad_flip_config_get(u8 module_id, u8 rev)
{
	return SYN_FLIP_NONE;
}

static void clearpad_set_irq(struct clearpad_t *this,
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
		enable_irq(this->irq);
	} else if (!mask && this->irq_mask) {
		LOG_STAT(this, "disable IRQ (user_mask 0x%02x)\n", mask);
		disable_irq_nosync(this->irq);
	} else {
		LOG_STAT(this, "no change IRQ (%s)\n",
				mask ? "enable" : "disable");
	}
	this->irq_mask = mask;
}

static int clearpad_read_pdt(struct clearpad_t *this)
{
	u8 addr = SYN_PDT_START - 1;
	u8 irq_bit = 0;
	u8 page = 0;
	int i, j, k;
	int rc;

	memset(&this->pdt, 0, sizeof(*this->pdt) * SYN_N_FUNCTIONS);
	for (i = 0; i < SYN_N_FUNCTIONS && addr >= SYN_SIZE_OF_FD - 1;) {
		struct clearpad_function_descriptor_t fdes;
		rc = clearpad_get(this, SYN_PAGE_ADDR(page, addr--),
				  &fdes.number);
		if (rc)
			break;
		rc = clearpad_get(this, SYN_PAGE_ADDR(page, addr--),
				  &fdes.int_count);
		if (rc)
			break;
		fdes.int_count &= SYN_MAX_INTERRUPT_SOURCE_COUNT;
		for (fdes.irq_mask = 0, j = 0; j < fdes.int_count; j++)
			fdes.irq_mask |= (1 << irq_bit++);
		rc = clearpad_get(this, SYN_PAGE_ADDR(page, addr--),
				  &fdes.base[SYN_TYPE_DATA]);
		if (rc)
			break;
		rc = clearpad_get(this, SYN_PAGE_ADDR(page, addr--),
				  &fdes.base[SYN_TYPE_CTRL]);
		if (rc)
			break;
		rc = clearpad_get(this, SYN_PAGE_ADDR(page, addr--),
				  &fdes.base[SYN_TYPE_COMMAND]);
		if (rc)
			break;
		rc = clearpad_get(this, SYN_PAGE_ADDR(page, addr--),
				  &fdes.base[SYN_TYPE_QUERY]);
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
			if (page < SYN_SUPPORTED_PAGE_NUM) {
				addr = SYN_PDT_START - 1;
				page += 1;
				continue;
			}
			break;
		}
		for (k = 0; k < SYN_N_FUNCTIONS; k++) {
			if (clearpad_function_value[k] == fdes.number) {
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

static int clearpad_set_charger(struct clearpad_t *this)
{
	int rc = 0;

	if (!this->charger.supported)
		goto exit;

	if (this->charger.status)
		rc = clearpad_put_bit(SYNF(this, F01_RMI, CTRL, 0x00),
			DEVICE_CONTROL_CHARGER_CONNECTED,
			DEVICE_CONTROL_CHARGER_CONNECTED);
	else
		rc = clearpad_put_bit(SYNF(this, F01_RMI, CTRL, 0x00),
			DEVICE_CONTROL_CHARGER_NOT_CONNECTED,
			DEVICE_CONTROL_CHARGER_CONNECTED);
	if (rc)
		dev_err(&this->pdev->dev, "failed to set charger status");
exit:
	return rc;
}

static int clearpad_set_pen(struct clearpad_t *this)
{
	int rc = 0;
	u8 buf;

	if (!this->pen.supported)
		goto exit;

	if (clearpad_is_valid_function(this, SYN_F11_2D)) {
		rc = clearpad_get(SYNF(this, F11_2D, QUERY, 0x06), &buf);
		if (rc)
			goto exit;
		if (buf & DEVICE_PEN_SUPPORTED) {
			rc = clearpad_put_bit(SYNF(this, F11_2D, CTRL, 0x0A),
				this->pen.enabled ?
				PEN_DETECT_INT_ENABLE :
				PEN_DETECT_INT_DISABLE,
				PEN_DETECT_F11_INTERRUPT);
			if (rc)
				goto exit;

			if (clearpad_is_valid_function(this, SYN_F54_ANALOG))
				rc = clearpad_put(
					SYNF(this, F54_ANALOG, COMMAND, 0x00),
					ANALOG_COMMAND_FORCE_UPDATE);
		} else {
			dev_warn(&this->pdev->dev, "pen is not supported\n");
		}
	} else if (clearpad_is_valid_function(this, SYN_F12_2D)) {
		rc = clearpad_put_bit(SYNF(this, F12_2D, CTRL,
					(this->chip_id == SYN_CHIP_3500 ||
					 this->chip_id == SYN_CHIP_7500) ?
					0x09 : 0x07),
				this->pen.enabled ?
				PEN_DETECT_F12_INTERRUPT :
				PEN_DETECT_INT_DISABLE,
				PEN_DETECT_F12_INTERRUPT);
	} else {
		dev_warn(&this->pdev->dev, "pen is not supported\n");
	}
exit:
	return rc;
}

static int clearpad_set_glove_mode(struct clearpad_t *this,
						bool enable)
{
	int rc = 0;
	u8 buf;

	if (!this->glove.supported)
		goto exit;

	if (clearpad_is_valid_function(this, SYN_F12_2D)) {
		rc = clearpad_get(SYNF(this, F12_2D, QUERY, 0x0A), &buf);
		if (rc) {
			dev_err(&this->pdev->dev,
				"failed to get support types");
			goto exit;
		}

		if (!(buf & GLOVE_MODE_SUPPORT)) {
			dev_warn(&this->pdev->dev,
				 "glove mode is not supported\n");
			goto exit;
		}

		rc = clearpad_put_bit(SYNF(this, F12_2D, CTRL, 0x09),
					enable ?
					GLOVE_MODE_BIT_ENABLE :
					GLOVE_MODE_BIT_DISABLE,
					GLOVE_MODE_BIT_MASK);
	} else if (clearpad_is_valid_function(this, SYN_F51_CUSTOM))
		rc = clearpad_put(SYNF(this, F51_CUSTOM,
					CTRL, 0x03),
					enable ?
					GLOVE_MODE_ENABLE :
					GLOVE_MODE_DISABLE);
	else {
		dev_warn(&this->pdev->dev, "glove mode is not supported\n");
		goto exit;
	}

	if (rc)
		dev_err(&this->pdev->dev, "failed to set glove mode");
exit:
	return rc;
}

static int clearpad_set_cover_status(struct clearpad_t *this)
{
	int rc = 0;
	u8 bufstr[3] = {0};

	rc = clearpad_set_glove_mode(this,
			this->cover.status ? true : this->glove.enabled);
	if (rc)
		goto exit;

	rc = clearpad_get_block(SYNF(this, F12_2D, CTRL, 0x09), bufstr, 3);
	if (rc)
		goto exit;

	if (this->cover.status)
		bufstr[F12_2D_CTRL_REPORT_AS_FINGER] |=
					REPORT_GLOVE_AS_FINGER;
	else
		bufstr[F12_2D_CTRL_REPORT_AS_FINGER] &=
					~REPORT_GLOVE_AS_FINGER;

	rc = clearpad_put_block(SYNF(this, F12_2D, CTRL, 0x09), bufstr, 3);
	if (rc)
		goto exit;

	rc = clearpad_put(SYNF(this, F51_CUSTOM, CTRL, 0x00),
			this->cover.status ? 0x03 : 0x00);
	if (rc)
		goto exit;

	rc = clearpad_put_bit(SYNF(this, F54_ANALOG, COMMAND, 0x00),
			ANALOG_COMMAND_FORCE_UPDATE,
			ANALOG_COMMAND_FORCE_UPDATE);
exit:
	if (rc)
		dev_err(&this->pdev->dev, "failed to set cover status");

	return rc;
}

static int clearpad_set_cover_window(struct clearpad_t *this)
{
	int rc;

	rc = clearpad_put(SYNF(this, F51_CUSTOM,
			CTRL, 0x03), this->cover.win_top);
	if (rc)
		goto exit;
	rc = clearpad_put(SYNF(this, F51_CUSTOM,
			CTRL, 0x04), this->cover.win_top >> 8);
	if (rc)
		goto exit;
	rc = clearpad_put(SYNF(this, F51_CUSTOM,
			CTRL, 0x07), this->cover.win_bottom);
	if (rc)
		goto exit;
	rc = clearpad_put(SYNF(this, F51_CUSTOM,
			CTRL, 0x08), this->cover.win_bottom >> 8);
	if (rc)
		goto exit;
	rc = clearpad_put(SYNF(this, F51_CUSTOM,
			CTRL, 0x05), this->cover.win_right);
	if (rc)
		goto exit;
	rc = clearpad_put(SYNF(this, F51_CUSTOM,
			CTRL, 0x06), this->cover.win_right >> 8);
	if (rc)
		goto exit;
	rc = clearpad_put(SYNF(this, F51_CUSTOM,
			CTRL, 0x01), this->cover.win_left);
	if (rc)
		goto exit;
	rc = clearpad_put(SYNF(this, F51_CUSTOM,
			CTRL, 0x02), this->cover.win_left >> 8);
	if (rc)
		dev_err(&this->pdev->dev, "failed to set cover window");
exit:
	return rc;
}

static int clearpad_set_wakeup_gesture(struct clearpad_t *this, int enable)
{
	int rc = 0;

	rc = clearpad_put(SYNF(this, F12_2D, CTRL, 0x0C),
		enable ? EW_DOUBLE_TAP_ENABLE : EW_DOUBLE_TAP_DISABLE);
	if (rc)
		dev_err(&this->pdev->dev, "failed to set wakeup gesture");
	return rc;
}

static ssize_t clearpad_pca_show(struct device *dev,
					      struct device_attribute *attr,
					      char *buf)
{
	struct clearpad_t *this = dev_get_drvdata(dev);
	int rc, i, block_count, null_pos;
	u8 tmp_buf[2];
	union {
		unsigned short block_num;
		unsigned char data[2];
	} block_num;
	unsigned int block_size;

	LOCK(this);
	if (this->pca_busy) {
		dev_err(&this->pdev->dev, "pca is still busy\n");
		rc = -EBUSY;
		UNLOCK(this);
		goto exit;
	}
	this->pca_busy = true;

	/* get block size */
	if (this->chip_id == SYN_CHIP_3400 ||
		this->chip_id == SYN_CHIP_3500 ||
		this->chip_id == SYN_CHIP_7500) {
		rc = clearpad_get_block(SYNF(this, F34_FLASH, QUERY, 0x02),
				    tmp_buf, 2);
	} else {
		rc = clearpad_get(SYNF(this, F34_FLASH, QUERY, 0x03),
				  &tmp_buf[0]);
		if (rc)
			goto err_unlock;
		rc = clearpad_get(SYNF(this, F34_FLASH, QUERY, 0x04),
				  &tmp_buf[1]);
	}
	if (rc)
		goto err_unlock;

	block_size = ((tmp_buf[1] << 8) | tmp_buf[0]);

	/* calc need block num with round up */
	block_count = ((SYN_PCA_ACCESS_MAX_READ_SIZE + 1) +
		       (block_size - 1)) / block_size;
	null_pos = block_size -
	    (SYN_PCA_ACCESS_MAX_READ_SIZE % block_size);

	block_num.block_num = 0;
	for (i = 0; i < block_count; i++) {
		block_num.data[1] &= 0x1F;
		block_num.data[1] |= (FLASH_DATA_CONFIGURATION_AREA << 5);

		/* write block number */
		rc = clearpad_put_block(SYNF(this, F34_FLASH, DATA, 0x00),
				     block_num.data, 2);
		if (rc)
			break;

		usleep_range(10000, 11000);

		/* issue read configuration block command */
		rc = clearpad_put(SYNF(this, F34_FLASH, DATA,
				(this->chip_id == SYN_CHIP_3400 ||
				this->chip_id == SYN_CHIP_3500  ||
				this->chip_id == SYN_CHIP_7500) ? 0x02 : 0x12),
				FLASH_CONTROL_READ_CONFIGURATION_BLOCK);
		if (rc)
			break;

		msleep(100);

		/* data read */
		rc = clearpad_get_block(SYNF(this, F34_FLASH, DATA,
				(this->chip_id == SYN_CHIP_3400 ||
				 this->chip_id == SYN_CHIP_3500 ||
				 this->chip_id == SYN_CHIP_7500) ? 0x01 : 0x02),
				buf, block_size);
		if (rc)
			break;

		usleep_range(10000, 11000);

		buf += block_size;
		block_num.block_num++;
	}
	buf -= null_pos;
	dev_info(&this->pdev->dev, "read size = %d",
			 (i * block_size) - null_pos);
err_unlock:
	if (rc)
		dev_err(&this->pdev->dev, "failed to read from touch device");
	this->pca_busy = false;
	UNLOCK(this);
exit:
	return rc ? rc : ((i * block_size) - null_pos);
}


static int clearpad_set_feature_settings(struct clearpad_t *this)
{
	int rc;

	rc = clearpad_set_charger(this);
	if (rc)
		goto exit;
	rc = clearpad_set_pen(this);
	if (rc)
		goto exit;
	rc = clearpad_set_glove_mode(this, this->glove.enabled);
	if (rc)
		goto exit;
	if (this->cover.enabled)
		rc = clearpad_set_cover_window(this);
exit:
	return rc;
}

static int clearpad_prepare_f11_2d(struct clearpad_t *this)
{
	int rc;
	u8 buf[4];
	u8 report_mode;
	enum registers {
		REG_X_LSB,
		REG_X_MSB,
		REG_Y_LSB,
		REG_Y_MSB,
	};

	rc = clearpad_get_block(SYNF(this, F11_2D, CTRL, 0x06),
				buf, sizeof(buf));
	if (rc)
		goto exit;

	this->extents.x_min = 0;
	this->extents.y_min = 0;
	this->extents.x_max = (buf[REG_X_LSB] | (buf[REG_X_MSB] << 8));
	this->extents.y_max = (buf[REG_Y_LSB] | (buf[REG_Y_MSB] << 8));

	rc = clearpad_get(SYNF(this, F11_2D, QUERY, 0x01), buf);
	if (rc)
		goto exit;

	this->extents.n_fingers = clearpad_number_of_fingers[buf[0] & 0x07];
	dev_info(&this->pdev->dev, "number of fingers=%d\n",
			this->extents.n_fingers);

	/* enable ABS event interrupts */
	rc = clearpad_put(SYNF(this, F01_RMI, CTRL, 0x01),
			this->pdt[SYN_F11_2D].irq_mask);
	if (rc)
		goto exit;

	if (this->extents.n_bytes_per_object == 0)
		this->extents.n_bytes_per_object = SYN_FINGER_DATA_SIZE;

	if (this->chip_id == SYN_CHIP_3000) {
		/* set reduced reporting mode */
		this->default_reporting_mode =
				XY_REPORTING_MODE_REDUCED_REPORTING_MODE;
		rc = clearpad_put_bit(SYNF(this, F11_2D, CTRL, 0x00),
				XY_REPORTING_MODE_REDUCED_REPORTING_MODE,
				XY_REPORTING_MODE);
		if (rc)
			goto exit;
	} else {
		rc = clearpad_get(SYNF(this, F11_2D, CTRL, 0x00), &report_mode);
		if (rc)
			goto exit;
		this->default_reporting_mode = report_mode & XY_REPORTING_MODE;
	}

	rc = clearpad_set_feature_settings(this);

exit:
	return rc;
}

static u8 clearpad_gen_offsets(u8 desc, u8 offset_from, u8 reg_array[],
	int reg_from, int array_size)
{
	const u8 unused_offset = 0xFF;
	u8 offset = offset_from;
	int bit, reg = reg_from;

	for (bit = 0; bit < 8 && reg < array_size; bit++)
		reg_array[reg++] = desc & (1 << bit) ? offset++ : unused_offset;

	return offset;
}

static int clearpad_query_regs(struct clearpad_t *this,
	enum clearpad_function_e func,
	u8 query, u8 reg_array[], int reg_array_size)
{
	/* register presence descriptors: maximum 32 bytes */
	const int max_size_presence = 32;
	/* size of register structure: 1 or 3 bytes */
	const int max_size_bytes = 3;

	u8 buffer[max_size_bytes + max_size_presence];
	u8 size_presence, offset = 0;
	int size_num_bytes;
	int rc, i;

	rc = clearpad_get(SYNI(this, func, SYN_TYPE_QUERY, query),
			  &size_presence);
	if (rc)
		goto err_ret;

	BUG_ON(size_presence > max_size_presence);

	/* The size of the control register structure is currently unused,
	 * but we need to find out how many bytes are allocated for it.
	 */
	rc = clearpad_get(SYNI(this, func, SYN_TYPE_QUERY, query + 1), buffer);
	if (rc)
		goto err_ret;

	size_num_bytes = buffer[0] == 0 ? max_size_bytes : 1;

	rc = clearpad_get_block(SYNI(this, func, SYN_TYPE_QUERY, query + 1),
			buffer, size_num_bytes + size_presence);
	if (rc)
		goto err_ret;

	for (i = 0; i < size_presence; i++)
		offset = clearpad_gen_offsets(buffer[i + size_num_bytes],
			offset, reg_array, i * 8, reg_array_size);
err_ret:
	return rc;
}

static void clearpad_log_offsets(struct device *dev, const char *header,
		enum clearpad_function_e func,
		u8 reg_array[], int reg_array_size)
{
	int i;

	dev_dbg(dev, "%s(%d)\n", header, func);
	for (i = 0; i < reg_array_size; i++)
		dev_dbg(dev, "[%.2d]=0x%02hX\n", i, (u16)reg_array[i]);
}

static int clearpad_init_reg_offsets(struct clearpad_t *this,
		enum clearpad_function_e func)
{
	const u8 query_ctrl_register_presence = 0x04;
	const u8 query_data_register_presence = 0x07;
	u8 query_desc;
	int rc;

	rc = clearpad_get(SYNI(this, func, SYN_TYPE_QUERY, 0x00), &query_desc);
	if (rc)
		goto err_ret;

	if (!(query_desc & 0x01)) {
		dev_err(&this->pdev->dev,
			"register descriptors not supported!\n");
		rc = -EINVAL;
		goto err_ret;
	}

	rc = clearpad_query_regs(this, func, query_ctrl_register_presence,
		this->pdt[func].offset[SYN_TYPE_CTRL],
		ARRAY_SIZE(this->pdt[func].offset[SYN_TYPE_CTRL]));
	if (rc)
		goto err_ret;

	clearpad_log_offsets(&this->pdev->dev, "control offsets", func,
		this->pdt[func].offset[SYN_TYPE_CTRL],
		ARRAY_SIZE(this->pdt[func].offset[SYN_TYPE_CTRL]));

	rc = clearpad_query_regs(this, func, query_data_register_presence,
		this->pdt[func].offset[SYN_TYPE_DATA],
		ARRAY_SIZE(this->pdt[func].offset[SYN_TYPE_DATA]));

	clearpad_log_offsets(&this->pdev->dev, "data offsets", func,
		this->pdt[func].offset[SYN_TYPE_DATA],
		ARRAY_SIZE(this->pdt[func].offset[SYN_TYPE_DATA]));
err_ret:
	return rc;
}

static int clearpad_prepare_f12_2d(struct clearpad_t *this)
{
	int rc;
	int i;
	u8 buf[4];
	enum registers {
		REG_X_LSB,
		REG_X_MSB,
		REG_Y_LSB,
		REG_Y_MSB,
	};

	rc = clearpad_init_reg_offsets(this, SYN_F12_2D);
	if (rc)
		goto err_ret;

	rc = clearpad_get_block(SYNS(this, F12_2D, CTRL, SENSOR_TUNING),
				buf, sizeof(buf));
	if (rc)
		goto err_ret;
	this->extents.x_max = (buf[REG_X_LSB] | (buf[REG_X_MSB] << 8));
	this->extents.y_max = (buf[REG_Y_LSB] | (buf[REG_Y_MSB] << 8));

	rc = clearpad_get_block(SYNS(this, F12_2D, CTRL, OBJECT_RPT_ENABLE),
		buf, 2);
	if (rc)
		goto err_ret;
	this->extents.n_fingers = buf[1];

	rc = clearpad_get(SYNS(this, F12_2D, CTRL, N_BYTES_PER_OBJECT), buf);
	if (rc)
		goto err_ret;

	this->extents.n_bytes_per_object = 0;

	for (i = 0; i < BITS_PER_BYTE; i++)
		this->extents.n_bytes_per_object += (buf[0] & BIT(i)) >> i;

	dev_info(&this->pdev->dev,
		"x_max=%d, y_max=%d, n_fingers=%d, n_bytes_per_object=%d\n",
		this->extents.x_max, this->extents.y_max,
		this->extents.n_fingers,
		this->extents.n_bytes_per_object);

	rc = clearpad_set_feature_settings(this);

err_ret:
	return rc;
}

static void clearpad_firmware_reset(struct clearpad_t *this)
{
	kfree(this->flash.image);
	memset(&this->flash, 0, sizeof(this->flash));
	this->fwdata_available = false;
	dev_info(&this->pdev->dev, "firmware image has been reset\n");
}

static int clearpad_flash(struct clearpad_t *this);

static int clearpad_initialize(struct clearpad_t *this)
{
	int rc;
	u8 buf[4];
	struct clearpad_device_info_t *info = &this->device_info;

	rc = clearpad_read_pdt(this);
	if (rc)
		goto exit;

	/* set device configured bit */
	rc = clearpad_put_bit(SYNF(this, F01_RMI, CTRL, 0x00),
					DEVICE_CONTROL_CONFIGURED,
					DEVICE_CONTROL_CONFIGURED);
	if (rc)
		goto exit;

	/* read device configuration */
	rc = clearpad_get_block(SYNF(this, F01_RMI, QUERY, 0x00),
				(u8 *)info, 21);
	if (rc)
		goto exit;
	if (this->chip_id >= SYN_CHIP_3200) {
		/* read device status */
		rc = clearpad_get(SYNF(this, F01_RMI, DATA, 0x00), buf);
		if (rc)
			goto exit;

		switch (buf[0] & 0x0f) {
		case DEVICE_STATUS_CONFIGURATION_CRC_FAILURE:
		case DEVICE_STATUS_FIRMWARE_CRC_FAILURE:
		case DEVICE_STATUS_CRC_IN_PROGRESS:
			memset(buf, 0, sizeof(buf));
			break;
		default:
			rc = clearpad_get_block(
				SYNF(this, F34_FLASH, CTRL, 0x00), buf, 4);
			if (rc)
				goto exit;
			break;
		}
		info->customer_family = buf[0];
		info->firmware_revision_major = buf[1];
		info->firmware_revision_minor = buf[2];
		info->firmware_revision_extra = buf[3];
	}

	if (this->state != SYN_STATE_RUNNING) {
		dev_info(&this->pdev->dev,
			"device mid %d, prop %d, family 0x%02x, " \
			"rev 0x%02x.%02x, extra 0x%02x\n",
			info->manufacturer_id, info->product_properties,
			info->customer_family, info->firmware_revision_major,
			info->firmware_revision_minor,
			info->firmware_revision_extra);
		dev_info(&this->pdev->dev,
			"bl %04d-%02d-%02d, tester %d, s/n %d, id '%s'\n",
			 2000 + info->date[0], info->date[1], info->date[2],
			 ((int)info->tester_id[0] << 8) + info->tester_id[1],
			 ((int)info->serial_number[0] << 8)
			 + info->serial_number[1],
			 clearpad_s(info->product_id, HEADER_PRODUCT_ID_SIZE));
	}

	if (this->flash.image) {
		dev_info(&this->pdev->dev, "force firmware update\n");
		this->state = SYN_STATE_FLASH_IMAGE_SET;
		this->task = SYN_TASK_NO_SUSPEND;
		rc = clearpad_flash(this);
		goto exit;
	}

	if (clearpad_is_valid_function(this, SYN_F11_2D)) {
		rc = clearpad_prepare_f11_2d(this);
		if (rc)
			goto exit;
	}

	if (clearpad_is_valid_function(this, SYN_F12_2D)) {
		rc = clearpad_prepare_f12_2d(this);
		if (rc)
			goto exit;
	}

	snprintf(this->result_info, sizeof(this->result_info),
		"%s, family 0x%02x, fw rev 0x%02x.%02x, extra 0x%02x, (%s)\n",
		clearpad_s(this->device_info.product_id,
			HEADER_PRODUCT_ID_SIZE),
		this->device_info.customer_family,
		this->device_info.firmware_revision_major,
		this->device_info.firmware_revision_minor,
		this->device_info.firmware_revision_extra,
		this->flash_requested ? "fw updated" : "no fw update");
	this->flash_requested = false;

	/* notify end of task */
	dev_info(&this->pdev->dev, "result: %s", this->result_info);
	wake_up_interruptible(&this->task_none_wq);
exit:
	return rc;
}

static int clearpad_flash_enable(struct clearpad_t *this)
{
	int rc;
	u8 buf[2];

	if (!this->fwdata_available) {
		rc = -EINVAL;
		goto exit;
	}
	/* read bootloader id */
	rc = clearpad_get_block(SYNF(this, F34_FLASH, QUERY, 0x00),
			buf, sizeof(buf));
	LOG_CHECK(this, "rc=%d\n", rc);
	if (rc)
		goto exit;

	/* write bootloader id to block data */
	rc = clearpad_put_block(SYNF(this, F34_FLASH, DATA,
				(this->chip_id == SYN_CHIP_3400 ||
				 this->chip_id == SYN_CHIP_3500 ||
				 this->chip_id == SYN_CHIP_7500) ? 0x01 : 0x02),
				buf, sizeof(buf));
	LOG_CHECK(this, "rc=%d\n", rc);
	if (rc)
		goto exit;

	usleep_range(10000, 11000);

	/* issue a flash program enable */
	rc = clearpad_put(SYNF(this, F34_FLASH, DATA,
			(this->chip_id == SYN_CHIP_3400 ||
			 this->chip_id == SYN_CHIP_3500 ||
			 this->chip_id == SYN_CHIP_7500) ? 0x02 : 0x12),
			FLASH_CONTROL_ENABLE_FLASH_PROGRAMMING);
	LOG_CHECK(this, "rc=%d\n", rc);
	if (rc)
		goto exit;

	this->state = SYN_STATE_FLASH_ENABLE;
	msleep(100);

	clearpad_set_irq(this, this->pdt[SYN_F34_FLASH].irq_mask, true);
exit:
	return rc;
}

static int clearpad_flash_program(struct clearpad_t *this)
{
	int rc;
	u8 buf[2];

	if (!this->fwdata_available) {
		rc = -EINVAL;
		goto exit;
	}

	/* make sure that we are in programming mode and there are no issues */
	rc = clearpad_get(SYNF(this, F34_FLASH, DATA,
		(this->chip_id == SYN_CHIP_3400 ||
		 this->chip_id == SYN_CHIP_3500 ||
		 this->chip_id == SYN_CHIP_7500) ? 0x03 : 0x12), buf);
	if (rc)
		goto exit;
	if (buf[0] != FLASH_CONTROL_PROGRAM_ENABLED) {
		dev_err(&this->pdev->dev,
		       "failed enabling flash (%s)\n",
			(this->chip_id == SYN_CHIP_3400 ||
			 this->chip_id == SYN_CHIP_3500 ||
			 this->chip_id == SYN_CHIP_7500) ?
			clearpad_flash_status[buf[0] & 7] :
			clearpad_flash_status[(buf[0] >> 4) & 7]);
		rc = -EIO;
		goto exit;
	}

	dev_info(&this->pdev->dev, "flashing enabled\n");

	/* PDT may have changed, re-read */
	rc = clearpad_read_pdt(this);
	if (rc)
		goto exit;

	/* read bootloader id */
	rc = clearpad_get_block(SYNF(this, F34_FLASH, QUERY, 0x00), buf, 2);
	if (rc)
		goto exit;

	/* write bootloader id to block data */
	rc = clearpad_put_block(SYNF(this, F34_FLASH, DATA,
		(this->chip_id == SYN_CHIP_3400 ||
		 this->chip_id == SYN_CHIP_3500 ||
		 this->chip_id == SYN_CHIP_7500) ? 0x01 : 0x02), buf, 2);
	if (rc)
		goto exit;

	usleep_range(10000, 11000);

	if (this->flash_mode == SYN_FLASH_MODE_NORMAL)
		/* issue a firmware and configuration erase */
		rc = clearpad_put(SYNF(this, F34_FLASH, DATA,
				(this->chip_id == SYN_CHIP_3400 ||
				 this->chip_id == SYN_CHIP_3500 ||
				 this->chip_id == SYN_CHIP_7500) ? 0x02 : 0x12),
				FLASH_CONTROL_ERASE_ALL);
	else
		/* issue a configuration erase */
		rc = clearpad_put(SYNF(this, F34_FLASH, DATA,
				(this->chip_id == SYN_CHIP_3400 ||
				 this->chip_id == SYN_CHIP_3500 ||
				 this->chip_id == SYN_CHIP_7500) ? 0x02 : 0x12),
				FLASH_CONTROL_ERASE_CONFIGURATION);

	if (rc)
		goto exit;

	dev_info(&this->pdev->dev, "firmware erasing\n");

	this->state = SYN_STATE_FLASH_PROGRAM;
exit:
	return rc;
}

static int clearpad_flash_data(struct clearpad_t *this)
{
	int rc, len;
	u8 buf;
	const u8 *data;
	struct clearpad_flash_image_t *f = &this->flash;
	u8 pos[2];

	if (!this->fwdata_available) {
		rc = -EINVAL;
		goto exit;
	}

	if (f->data.pos > 0)
		goto write_block_data;

	/* make sure that we are in programming mode and there are no issues */
	rc = clearpad_get(SYNF(this, F34_FLASH, DATA,
		(this->chip_id == SYN_CHIP_3400 ||
		 this->chip_id == SYN_CHIP_3500 ||
		 this->chip_id == SYN_CHIP_7500) ? 0x03 : 0x12), &buf);
	if (rc)
		goto exit;
	if (buf != FLASH_CONTROL_PROGRAM_ENABLED) {
		dev_err(&this->pdev->dev,
				"failed flashing data (%s)\n",
				(this->chip_id == SYN_CHIP_3400 ||
				 this->chip_id == SYN_CHIP_3500 ||
				 this->chip_id == SYN_CHIP_7500) ?
				clearpad_flash_status[buf & 7] :
				clearpad_flash_status[(buf >> 4) & 7]);
		rc = -EIO;
		goto exit;
	}

	/* block # low and high byte */
	pos[0] = f->data.pos & 0xff;
	pos[1] = (f->data.pos >> 8) & 0xff;
	rc = clearpad_put_block(SYNF(this, F34_FLASH, DATA, 0x00), pos, 2);
	if (rc)
		goto exit;

write_block_data:
	data = f->data.data + f->data.pos * 16;
	len = f->data.length - f->data.pos * 16;
	if (len > 16)
		len = 16;

	/* write block data */
	rc = clearpad_put_block(SYNF(this, F34_FLASH, DATA,
		(this->chip_id == SYN_CHIP_3400 ||
		 this->chip_id == SYN_CHIP_3500 ||
		 this->chip_id == SYN_CHIP_7500) ? 0x01 : 0x02), data, len);
	if (rc)
		goto exit;

	/* issue a write data block command */
	rc = clearpad_put(SYNF(this, F34_FLASH, DATA,
			(this->chip_id == SYN_CHIP_3400 ||
			 this->chip_id == SYN_CHIP_3500 ||
			 this->chip_id == SYN_CHIP_7500) ? 0x02 : 0x12),
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

static int clearpad_flash_config(struct clearpad_t *this)
{
	int rc, len;
	u8 buf;
	const u8 *data;
	struct clearpad_flash_image_t *f = &this->flash;
	u8 pos[2];

	if (!this->fwdata_available) {
		rc = -EINVAL;
		goto exit;
	}

	if (f->config.pos > 0)
		goto write_block_data;

	/* make sure that we are in programming mode and there are no issues */
	rc = clearpad_get(SYNF(this, F34_FLASH, DATA,
		(this->chip_id == SYN_CHIP_3400 ||
		 this->chip_id == SYN_CHIP_3500 ||
		 this->chip_id == SYN_CHIP_7500) ? 0x03 : 0x12), &buf);
	if (rc)
		goto exit;
	if (buf != FLASH_CONTROL_PROGRAM_ENABLED) {
		dev_err(&this->pdev->dev,
				"failed flashing config (%s)\n",
				(this->chip_id == SYN_CHIP_3400 ||
				 this->chip_id == SYN_CHIP_3500 ||
				 this->chip_id == SYN_CHIP_7500) ?
				clearpad_flash_status[buf & 7] :
				clearpad_flash_status[(buf >> 4) & 7]);
		rc = -EIO;
		goto exit;
	}

	/* block # low and high byte */
	pos[0] = f->config.pos & 0xff;
	pos[1] = (f->config.pos >> 8) & 0xff;
	rc = clearpad_put_block(SYNF(this, F34_FLASH, DATA, 0x00), pos, 2);
	if (rc)
		goto exit;

write_block_data:
	data = f->config.data + f->config.pos * 16;
	len = f->config.length - f->config.pos * 16;
	if (len > 16)
		len = 16;

	/* write block data */
	rc = clearpad_put_block(SYNF(this, F34_FLASH, DATA,
		(this->chip_id == SYN_CHIP_3400 ||
		 this->chip_id == SYN_CHIP_3500 ||
		 this->chip_id == SYN_CHIP_7500) ? 0x01 : 0x02), data, len);
	if (rc)
		goto exit;

	/* issue a write configuration block command */
	rc = clearpad_put(SYNF(this, F34_FLASH, DATA,
			(this->chip_id == SYN_CHIP_3400 ||
			 this->chip_id == SYN_CHIP_3500 ||
			 this->chip_id == SYN_CHIP_7500) ? 0x02 : 0x12),
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

static int clearpad_flash_disable(struct clearpad_t *this)
{
	int rc;
	u8 buf;

	if (!this->fwdata_available) {
		rc = -EINVAL;
		goto exit;
	}

	/* make sure that we are in programming mode and there are no issues */
	rc = clearpad_get(SYNF(this, F34_FLASH, DATA,
		(this->chip_id == SYN_CHIP_3400 ||
		 this->chip_id == SYN_CHIP_3500 ||
		 this->chip_id == SYN_CHIP_7500) ? 0x03 : 0x12), &buf);
	if (rc)
		goto exit;
	if (buf != FLASH_CONTROL_PROGRAM_ENABLED) {
		dev_err(&this->pdev->dev,
				"failed flashing config (%s)\n",
				(this->chip_id == SYN_CHIP_3400 ||
				 this->chip_id == SYN_CHIP_3500 ||
				 this->chip_id == SYN_CHIP_7500) ?
				clearpad_flash_status[buf & 7] :
				clearpad_flash_status[(buf >> 4) & 7]);
		rc = -EIO;
		goto exit;
	}

	usleep_range(10000, 11000);

	/* send a reset to the device to complete the flash procedure */
	rc = clearpad_put(SYNF(this, F01_RMI, COMMAND, 0x00),
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

static int clearpad_flash_verify(struct clearpad_t *this)
{
	int rc;
	u8 buf;

	/* make sure that we are no longer in programming mode */
	rc = clearpad_get(SYNF(this, F01_RMI, DATA, 0x00), &buf);
	LOG_CHECK(this, "rc=%d F01_RMI_DATA0=0x%x\n", rc, buf);
	if (rc)
		goto exit;
	if (buf & (1 << 6)) {
		dev_err(&this->pdev->dev,
				"failed disabling flash (%s)\n",
				clearpad_flash_reason[buf & 0x0f]);
		rc = -EIO;
		goto exit;
	}

	this->state = SYN_STATE_INIT;
	this->task = SYN_TASK_NONE;

	dev_info(&this->pdev->dev,
			"device successfully flashed\n");

	clearpad_set_irq(this, this->pdt[SYN_F34_FLASH].irq_mask, false);

	rc = clearpad_initialize(this);
	LOG_CHECK(this, "rc=%d\n", rc);
	if (rc)
		goto exit;
	this->state = SYN_STATE_RUNNING;
	clearpad_funcarea_initialize(this);
exit:
	return rc;
}

static void clearpad_firmware_check(struct clearpad_t *this)
{
	const u8 *data;
	struct clearpad_flash_image_t *f = &this->flash;

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

static int clearpad_flash(struct clearpad_t *this)
{
	int rc = 0;

	switch (this->state) {
	case SYN_STATE_FLASH_IMAGE_SET:
		rc = clearpad_flash_enable(this);
		LOG_CHECK(this, "rc=%d\n", rc);
		break;
	case SYN_STATE_FLASH_ENABLE:
		rc = clearpad_flash_program(this);
		LOG_CHECK(this, "rc=%d\n", rc);
		break;
	case SYN_STATE_FLASH_PROGRAM:
		if (this->flash_mode == SYN_FLASH_MODE_NORMAL) {
			rc = clearpad_flash_data(this);
			LOG_CHECK(this, "rc=%d\n", rc);
		} else {
			rc = clearpad_flash_config(this);
			LOG_CHECK(this, "rc=%d\n", rc);
		}
		break;
	case SYN_STATE_FLASH_DATA:
		rc = clearpad_flash_config(this);
		LOG_CHECK(this, "rc=%d\n", rc);
		break;
	case SYN_STATE_FLASH_CONFIG:
		rc = clearpad_flash_disable(this);
		clearpad_firmware_reset(this);
		LOG_CHECK(this, "rc=%d\n", rc);
		break;
	case SYN_STATE_FLASH_DISABLE:
		rc = clearpad_flash_verify(this);
		LOG_CHECK(this, "rc=%d\n", rc);
		break;
	default:
		LOG_CHECK(this, "rc=%d\n", rc);
		break;
	}

	if (rc) {
		dev_err(&this->pdev->dev, "failed during flash (%s)\n",
			clearpad_state_name[this->state]);
		this->state = SYN_STATE_DISABLED;
		clearpad_set_irq(this,
				 this->pdt[SYN_F34_FLASH].irq_mask, false);

		snprintf(this->result_info, SYN_STRING_LENGTH,
			"%s, family 0x%02x, fw rev 0x%02x.%02x, " \
			"extra 0x%02x, failed fw update\n",
			clearpad_s(this->device_info.product_id,
			HEADER_PRODUCT_ID_SIZE),
			this->device_info.customer_family,
			this->device_info.firmware_revision_major,
			this->device_info.firmware_revision_minor,
			this->device_info.firmware_revision_extra);
		this->flash_requested = false;
		clearpad_firmware_reset(this);

		/* check if standby was reserved */
		if (this->active & SYN_STANDBY_AFTER_TASK) {
			this->active &= ~SYN_STANDBY_AFTER_TASK;
			this->active |= SYN_STANDBY;

			LOG_STAT(this, "active: %x (task: %s)\n",
				 this->active, clearpad_task_name[this->task]);
		}

		/* notify end of task */
		dev_info(&this->pdev->dev, "result: %s", this->result_info);
		wake_up_interruptible(&this->task_none_wq);
	}
	return rc;
}

static void clearpad_wd_status_poll(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct clearpad_t *this = container_of(dwork,
			struct clearpad_t, wd_poll_work);
	int rc;

	dev_dbg(&this->pdev->dev, "%s\n", __func__);

	LOCK(this);
	if ((this->active & SYN_ACTIVE_POWER) && !this->flash_requested) {
		rc = clearpad_get(SYNF(this, F01_RMI, DATA, 0x00), (void *)&rc);
		if (rc) {
			dev_err(&this->pdev->dev, "%s, rc = %d\n",
							__func__, rc);
			dev_info(&this->pdev->dev, "Resetting device\n");
			clearpad_reset_power(this, __func__);
		} else {
			this->reset_count = 0;
		}
		schedule_delayed_work(&this->wd_poll_work, this->wd_poll_t_jf);
	}
	UNLOCK(this);
}

static void clearpad_wd_update(struct clearpad_t *this, bool sync)
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

static int clearpad_regulator_handler(struct regulator *regulator,
				struct device *dev, const char *func_str,
				const char *reg_str, int sw)
{
	int rc, enabled;

	if (IS_ERR_OR_NULL(regulator)) {
		rc = regulator ? PTR_ERR(regulator) : -EINVAL;
		dev_err(dev,
			"%s: regulator '%s' invalid",
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

static int clearpad_vreg_suspend(struct clearpad_t *this, int enable)
{
	int rc = 0;

	if (IS_ERR(this->vreg_touch_vdd)) {
		dev_err(&this->pdev->dev,
				"%s: vreg_touch_vdd is not initialized\n",
				__func__);
		rc = -ENODEV;
		goto exit;
	}

	if (enable)
		rc = regulator_set_optimum_mode(this->vreg_touch_vdd, 1000);
	else
		rc = regulator_set_optimum_mode(this->vreg_touch_vdd, 15000);

	if (rc < 0) {
		dev_err(&this->pdev->dev,
				"%s: vdd: set mode (%s) failed, rc=%d\n",
				__func__, (enable ? "LPM" : "HPM"), rc);
		goto exit;
	} else {
		dev_info(&this->pdev->dev,
				"%s: vdd: set mode (%s) ok, new mode=%d\n",
				__func__, (enable ? "LPM" : "HPM"), rc);
		rc = 0;
	}
exit:
	return rc;
}

static int clearpad_vreg_configure(struct clearpad_t *this, int enable)
{
	int rc = 0;
	struct device *dev = this->pdev->dev.parent;

	if (enable) {
		this->vreg_touch_vdd = regulator_get(dev, CLEARPAD_VDD);
		if (IS_ERR(this->vreg_touch_vdd)) {
			dev_err(dev, "%s: get vdd failed\n", __func__);
			rc = -ENODEV;
			goto err_ret;
		}
		rc = regulator_set_voltage(this->vreg_touch_vdd,
						3000000, 3000000);
		if (rc) {
			dev_err(dev, "%s: set voltage vdd failed, rc=%d\n",
								__func__, rc);
			goto err_put_vdd;
		}
		rc = clearpad_regulator_handler(this->vreg_touch_vdd, dev,
						__func__, CLEARPAD_VDD, 1);
		if (rc)
			goto err_put_vdd;
		rc = clearpad_vreg_suspend(this, 0);
		if (rc) {
			dev_err(dev, "%s: set vdd mode failed, rc=%d\n",
				__func__, rc);
			goto err_disable_vdd;
		}
		this->vreg_touch_vio = regulator_get(dev, CLEARPAD_VIO);
		if (!IS_ERR(this->vreg_touch_vio)) {
			rc = clearpad_regulator_handler(this->vreg_touch_vio,
					dev, __func__, CLEARPAD_VIO, 1);
			if (rc)
				goto err_put_vio;
		}
	} else {
		if (!IS_ERR(this->vreg_touch_vio)) {
			clearpad_regulator_handler(this->vreg_touch_vio,
					dev, __func__, CLEARPAD_VIO, 0);
			regulator_put(this->vreg_touch_vio);
		}
		if (!IS_ERR(this->vreg_touch_vdd)) {
			rc = regulator_set_voltage(this->vreg_touch_vdd,
					0, 3000000);
			if (rc)
				dev_err(dev,
					"%s: set voltage vdd failed, rc=%d\n",
					__func__, rc);
			clearpad_regulator_handler(this->vreg_touch_vdd,
					dev, __func__, CLEARPAD_VDD, 0);
			regulator_put(this->vreg_touch_vdd);
		}
	}
	return rc;
err_put_vio:
	regulator_put(this->vreg_touch_vio);
err_disable_vdd:
	clearpad_regulator_handler(this->vreg_touch_vdd, dev, __func__,
					CLEARPAD_VDD, 0);
err_put_vdd:
	regulator_put(this->vreg_touch_vdd);
err_ret:
	return rc;
}

static int clearpad_gpio_configure(struct clearpad_t *this, int enable)
{
	int rc = 0;

	if (enable) {
		rc = gpio_request(this->pdata->irq_gpio, CLEARPAD_NAME);
		if (rc) {
			dev_err(&this->pdev->dev,
					"%s: gpio_requeset failed, rc=%d\n",
					__func__, rc);
			return rc;
		}

		if (this->gpio_reset) {
			rc = gpio_request(this->gpio_reset, CLEARPAD_NAME);
			if (rc) {
				dev_err(&this->pdev->dev,
					"%s: gpio_request(%d) failed, rc=%d\n",
					__func__, this->gpio_reset, rc);
				gpio_free(this->pdata->irq_gpio);
				return rc;
			}

			rc = gpio_direction_output(this->gpio_reset,
						this->reset_l2h ? 1 : 0);
			if (rc) {
				dev_err(&this->pdev->dev,
				"%s: gpio_direction_output(%d) failed, rc=%d\n",
					__func__, this->gpio_reset, rc);
				gpio_free(this->pdata->irq_gpio);
				gpio_free(this->gpio_reset);
				return rc;
			}
		}
	} else {
		gpio_free(this->pdata->irq_gpio);
		if (this->gpio_reset) {
			gpio_direction_output(this->gpio_reset,
						this->reset_l2h ? 0 : 1);
			gpio_free(this->gpio_reset);
		}
	}

	return rc;
}

static int clearpad_reset_power_core(struct clearpad_t *this)
{
	int rc = 0;

	rc = clearpad_vreg_configure(this, 0);
	if (rc) {
		dev_err(&this->pdev->dev,
			"%s: Vreg disable configure error\n",
			__func__);
		goto exit;
	}

	switch (this->chip_id) {
	case SYN_CHIP_3000:
	case SYN_CHIP_3200:
	case SYN_CHIP_7300:
	case SYN_CHIP_7500:
		usleep_range(10000, 11000);
		break;

	case SYN_CHIP_3400:
		msleep(1000);
		break;

	case SYN_CHIP_3500:
		rc = clearpad_gpio_configure(this, 0);
		if (rc) {
			dev_err(&this->pdev->dev,
				"%s: GPIO disable configure error\n",
				__func__);
			clearpad_vreg_configure(this, 1);
			goto exit;
		}
		msleep(400);
		rc = clearpad_gpio_configure(this, 1);
		if (rc) {
			dev_err(&this->pdev->dev,
				"%s: GPIO enable configure error\n",
				__func__);
			clearpad_vreg_configure(this, 1);
			goto exit;
		}
		break;

	default:
		dev_err(&this->pdev->dev, "Unknown chip id:0x%x\n",
			this->chip_id);
		break;
	}

	rc = clearpad_vreg_configure(this, 1);
	if (rc)
		dev_err(&this->pdev->dev,
			"%s: Vreg enable configure error\n",
			__func__);
exit:
	return rc;
}

static int clearpad_set_normal_mode(struct clearpad_t *this)
{
	int rc = 0;
	u8 irq, buf[4] = {0};

	dev_dbg(&this->pdev->dev, "%s\n", __func__);

	if (this->wakeup_gesture.enabled) {
		if (!this->wakeup_gesture.lpm_disabled) {
			rc = clearpad_vreg_suspend(this, 0);
			if (rc)
				goto exit;
			usleep_range(10000, 11000);

			clearpad_set_irq(this, this->pdt[SYN_F01_RMI].irq_mask,
									true);
		}
		rc = clearpad_get(SYNF(this, F01_RMI, DATA, 0x01), &irq);
		if (rc) {
			dev_err(&this->pdev->dev,
			"failed to read interrupt status\n");
			goto exit;
		}
		if (clearpad_is_valid_function(this, SYN_F11_2D)) {
			rc = clearpad_put_bit(SYNF(this, F11_2D, CTRL, 0x00),
				this->default_reporting_mode,
				XY_REPORTING_MODE);
			if (rc) {
				dev_err(&this->pdev->dev,
				"failed to exit wakeup gesture mode\n");
				goto exit;
			}
		} else if (clearpad_is_valid_function(this, SYN_F12_2D)) {
			rc = clearpad_get_block(SYNF(this, F12_2D, CTRL, 0x07),
						buf, F12_2D_CTRL_RPT_REG_MAX);
			if (rc) {
				dev_err(&this->pdev->dev,
				"failed to read control report register\n");
				goto exit;
			}
			buf[F12_2D_CTRL_RPT_FLAG] = EW_RPT_GESTURE_DISABLE;
			rc = clearpad_put_block(SYNF(this, F12_2D, CTRL, 0x07),
						buf, F12_2D_CTRL_RPT_REG_MAX);
			if (rc) {
				dev_err(&this->pdev->dev,
				"failed to disable report gesture\n");
				goto exit;
			}
			rc = clearpad_set_wakeup_gesture(this, 0);
			if (rc) {
				dev_err(&this->pdev->dev,
				"failed to exit wakeup gesture mode\n");
				goto exit;
			}
			if (this->chip_id == SYN_CHIP_3500 ||
			    this->chip_id == SYN_CHIP_7500) {
				rc = clearpad_put_bit(SYNF(this, F54_ANALOG,
					COMMAND, 0x00),
					ANALOG_COMMAND_FORCE_CAL,
					ANALOG_COMMAND_FORCE_CAL);
				if (rc) {
					dev_err(&this->pdev->dev,
					"failed to force calibrate\n");
					goto exit;
				}
			}
		}
	} else {
		rc = clearpad_vreg_suspend(this, 0);
		if (rc)
			goto exit;
		usleep_range(10000, 11000);
		clearpad_set_irq(this, this->pdt[SYN_F01_RMI].irq_mask,
								true);
		rc = clearpad_get(SYNF(this, F01_RMI, DATA, 0x01), &irq);
		if (rc) {
			dev_err(&this->pdev->dev,
				"failed to read interrupt status\n");
			goto exit;
		}
		rc = clearpad_put_bit(SYNF(this, F01_RMI, CTRL, 0x00),
			DEVICE_CONTROL_SLEEP_MODE_NORMAL_OPERATION,
			DEVICE_CONTROL_SLEEP_MODE);
		if (rc) {
			dev_err(&this->pdev->dev,
				"failed to exit sleep mode\n");
			goto exit;
		}
		usleep_range(10000, 11000);
	}

	if (clearpad_is_valid_function(this, SYN_F51_CUSTOM)
	    && this->cover.enabled)
		rc = clearpad_set_cover_status(this);

	this->active |= SYN_ACTIVE_POWER;
	dev_info(&this->pdev->dev, "normal mode OK\n");
exit:
	return rc;
}

static int clearpad_set_suspend_mode(struct clearpad_t *this)
{
	int rc = 0;
	u8 buf[4] = {0};

	dev_dbg(&this->pdev->dev, "%s\n", __func__);

	if (this->wakeup_gesture.enabled) {
		if (clearpad_is_valid_function(this, SYN_F11_2D)) {
			rc = clearpad_put_bit(SYNF(this, F11_2D, CTRL, 0x00),
				XY_REPORTING_MODE_WAKEUP_GESTURE_MODE,
				XY_REPORTING_MODE);
		} else if (clearpad_is_valid_function(this, SYN_F12_2D)) {
			rc = clearpad_get_block(SYNF(this, F12_2D, CTRL, 0x07),
						buf, F12_2D_CTRL_RPT_REG_MAX);
			if (rc) {
				dev_err(&this->pdev->dev,
				"failed to read control report register\n");
				goto exit;
			}
			buf[F12_2D_CTRL_RPT_FLAG] = EW_RPT_GESTURE_ENABLE;
			rc = clearpad_put_block(SYNF(this, F12_2D, CTRL, 0x07),
						buf, F12_2D_CTRL_RPT_REG_MAX);
			if (!rc)
				rc = clearpad_set_wakeup_gesture(this, 1);
		}
		if (rc) {
			dev_err(&this->pdev->dev,
				"failed to enter wake-up gesture mode\n");
			goto exit;
		}

		this->wakeup_gesture.time_started = jiffies - 1;
		usleep_range(10000, 11000);
		LOG_CHECK(this, "enter doze mode\n");
		if (!this->wakeup_gesture.lpm_disabled) {
			rc = clearpad_vreg_suspend(this, 1);
			if (rc)
				goto exit;
		}
	} else {
		rc = clearpad_put_bit(SYNF(this, F01_RMI, CTRL, 0x00),
			DEVICE_CONTROL_SLEEP_MODE_SENSOR_SLEEP,
			DEVICE_CONTROL_SLEEP_MODE);
		if (rc) {
			dev_err(&this->pdev->dev,
				"failed to exit normal mode\n");
			goto exit;
		}
		usleep_range(10000, 11000);
		clearpad_set_irq(this, this->pdt[SYN_F01_RMI].irq_mask, false);
		LOG_CHECK(this, "enter sleep mode\n");
		rc = clearpad_vreg_suspend(this, 1);
		if (rc)
			goto exit;
	}

	this->active &= ~SYN_ACTIVE_POWER;
	dev_info(&this->pdev->dev, "suspend mode OK\n");
exit:
	return rc;
}

static int clearpad_set_power(struct clearpad_t *this)
{
	int rc = 0;
	int active;
	bool should_wake;

	LOCK(this);
	active = this->active;

	LOG_STAT(this, "powered %d, users %d, standby %d\n",
		 !!(active & SYN_ACTIVE_POWER),
		 this->input->users,
		 !!(active & SYN_STANDBY));

	dev_info(&this->pdev->dev, "%s: state=%s\n", __func__,
		 clearpad_state_name[this->state]);
	should_wake = !(active & SYN_STANDBY);

	if (should_wake && !(active & SYN_ACTIVE_POWER))
		rc = clearpad_set_normal_mode(this);
	else if (!should_wake && (active & SYN_ACTIVE_POWER))
		rc = clearpad_set_suspend_mode(this);
	else
		dev_info(&this->pdev->dev, "no change (%d)\n", should_wake);

	if (rc)
		clearpad_reset_power(this, __func__);
	else
		this->reset_count = 0;
	UNLOCK(this);

	if (this->pdata->watchdog_enable)
		clearpad_wd_update(this, true);

	return rc;
}

static void clearpad_reset_power(struct clearpad_t *this, const char *cause)
{
	unsigned long flags;

	if (this->flash_requested)
		return;

	if (cause && cause == this->reset_cause) {
		if (this->reset_count >= SYN_RETRY_NUM_OF_RESET) {
			dev_info(&this->pdev->dev, "ignore reset request\n");
			return;
		} else {
			this->reset_count++;
		}
	} else {
		this->reset_cause = cause;
		if (!cause) {
			cause = "force";
			this->reset_count = 0;
		} else {
			this->reset_count = 1;
		}
	}

	dev_info(&this->pdev->dev, "power on reset (%s)\n", cause);

	spin_lock_irqsave(&this->slock, flags);
	this->dev_busy = false;
	this->irq_pending = false;
	spin_unlock_irqrestore(&this->slock, flags);

	if (clearpad_reset_power_core(this)) {
		dev_err(&this->pdev->dev, "vreg reset failed\n");
	} else {
		msleep(this->por_delay_after);
		clearpad_set_page(this, 0);
		clearpad_set_irq(this, this->pdt[SYN_F01_RMI].irq_mask, true);
	}
}

static int clearpad_gpio_export(struct clearpad_t *this,
				struct device *dev, bool export)
{
	int rc = 0;

	if (export) {
		rc = gpio_export(this->pdata->irq_gpio, false);
		if (rc) {
			dev_err(dev, "%s: Failed to export gpio, rc=%d\n",
				__func__, rc);
		} else {
			rc = gpio_export_link(dev, "attn",
					this->pdata->irq_gpio);
			if (rc)
				dev_err(dev, "%s: Failed to export, rc=%d\n",
				__func__, rc);
		}
	} else {
		gpio_unexport(this->pdata->irq_gpio);
		sysfs_remove_link(&dev->kobj, "attn");
	}

	return rc;
}

static void clearpad_funcarea_initialize(struct clearpad_t *this)
{
	struct clearpad_funcarea_t *funcarea;
	struct clearpad_area_t pointer_area;
	struct clearpad_button_data_t *button;
	struct clearpad_pointer_data_t *pointer_data;
	const char *func_name[] = {
		[SYN_FUNCAREA_INSENSIBLE] = "insensible",
		[SYN_FUNCAREA_POINTER] = "pointer",
		[SYN_FUNCAREA_BUTTON] = "button",
	};

	this->funcarea = clearpad_funcarea_get(this,
				this->device_info.customer_family,
				this->device_info.firmware_revision_major);
	funcarea = this->funcarea;

	if (funcarea == NULL) {
		dev_info(&this->pdev->dev, "no funcarea\n");
		return;
	}

	for (; funcarea->func != SYN_FUNCAREA_END; funcarea++) {
		switch (funcarea->func) {
		case SYN_FUNCAREA_POINTER:
			pointer_area = (struct clearpad_area_t)
							funcarea->original;
			pointer_data = (struct clearpad_pointer_data_t *)
							funcarea->data;
			if (pointer_data) {
				pointer_area.x1 -= pointer_data->offset_x;
				pointer_area.x2 -= pointer_data->offset_x;
				pointer_area.y1 -= pointer_data->offset_y;
				pointer_area.y2 -= pointer_data->offset_y;
			}
			input_set_abs_params(this->input, ABS_MT_TRACKING_ID,
					0, this->extents.n_fingers, 0, 0);
			input_set_abs_params(this->input, ABS_MT_POSITION_X,
					     pointer_area.x1,
					     pointer_area.x2, 0, 0);
			input_set_abs_params(this->input, ABS_MT_POSITION_Y,
					     pointer_area.y1,
					     pointer_area.y2, 0, 0);
			if (this->touch_pressure_enabled)
				input_set_abs_params(this->input,
					ABS_MT_PRESSURE, 0,
					SYN_MAX_Z_VALUE, 0, 0);
			if (this->touch_size_enabled) {
				input_set_abs_params(this->input,
					ABS_MT_TOUCH_MAJOR, 0,
					SYN_MAX_W_VALUE + 1, 0, 0);
				input_set_abs_params(this->input,
					ABS_MT_TOUCH_MINOR, 0,
					SYN_MAX_W_VALUE + 1, 0, 0);
			}
			if (this->touch_orientation_enabled)
				input_set_abs_params(this->input,
					ABS_MT_ORIENTATION, -1, 1, 0, 0);
			input_set_abs_params(this->input, ABS_MT_TOOL_TYPE,
					0, this->pen.enabled ? MT_TOOL_PEN :
							MT_TOOL_FINGER, 0, 0);
			break;
		case SYN_FUNCAREA_BUTTON:
			button =
				(struct clearpad_button_data_t *)funcarea->data;
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

static inline bool clearpad_funcarea_test(struct clearpad_area_t *area,
					   struct clearpad_point_t *point)
{
	return (area->x1 <= point->x && point->x <= area->x2
		&& area->y1 <= point->y && point->y <= area->y2);
}

static struct clearpad_funcarea_t *
clearpad_funcarea_search(struct clearpad_t *this,
			  struct clearpad_pointer_t *pointer)
{
	struct clearpad_funcarea_t *funcarea = this->funcarea;

	if (funcarea == NULL)
		goto exit;

	/* get new funcarea */
	for ( ; funcarea->func != SYN_FUNCAREA_END; funcarea++) {
		if (clearpad_funcarea_test(&funcarea->original,
						&pointer->cur))
			goto exit;
		if (funcarea->func == SYN_FUNCAREA_POINTER
		    && clearpad_funcarea_test(&funcarea->extension,
						&pointer->cur))
			goto exit;
	}
	funcarea = NULL;
exit:
	return funcarea;
}

static void clearpad_funcarea_crop(struct clearpad_area_t *area,
		struct clearpad_point_t *point)
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

static void clearpad_funcarea_down(struct clearpad_t *this,
				    struct clearpad_pointer_t *pointer)
{
	int touch_major, touch_minor;
	struct clearpad_button_data_t *button;
	struct clearpad_pointer_data_t *pointer_data;
	struct clearpad_point_t *cur = &pointer->cur;
	struct input_dev *idev = this->input;
	bool valid;

	switch (pointer->funcarea->func) {
	case SYN_FUNCAREA_INSENSIBLE:
		LOG_EVENT(this, "insensible\n");
		pointer->down = false;
		break;
	case SYN_FUNCAREA_POINTER:
		clearpad_funcarea_crop(&pointer->funcarea->original, cur);
		pointer_data = (struct clearpad_pointer_data_t *)
					pointer->funcarea->data;
		if (pointer_data) {
			cur->x -= pointer_data->offset_x;
			cur->y -= pointer_data->offset_y;
		}
		if (cur->tool == SYN_TOOL_PEN) {
			cur->tool = MT_TOOL_PEN;
		} else {
			/* shift range if glove event */
			if (cur->tool == SYN_TOOL_GLOVE) {
				if (cur->z > 0)
					cur->z += SYN_MAX_Z_VALUE + 1;
				else
					cur->z = 0;
			} else {
				if (cur->z > SYN_MAX_Z_VALUE)
					cur->z = SYN_MAX_Z_VALUE;
			}
			cur->tool = MT_TOOL_FINGER;
		}
		valid = idev->users > 0;
		LOG_EVENT(this, "%s[%d]: (x,y)=(%d,%d) w=(%d,%d) z=%d t=%d\n",
			  valid ? "pt" : "unused pt", cur->id, cur->x, cur->y,
			  cur->wx, cur->wy, cur->z, cur->tool);
		if (!valid)
			break;
		touch_major = max(cur->wx, cur->wy) + 1;
		touch_minor = min(cur->wx, cur->wy) + 1;
		input_report_abs(idev, ABS_MT_TRACKING_ID, cur->id);
		input_report_abs(idev, ABS_MT_TOOL_TYPE, cur->tool);
		input_report_abs(idev, ABS_MT_POSITION_X, cur->x);
		input_report_abs(idev, ABS_MT_POSITION_Y, cur->y);
		if (this->touch_pressure_enabled)
			input_report_abs(idev, ABS_MT_PRESSURE, cur->z);
		if (this->touch_size_enabled) {
			input_report_abs(idev, ABS_MT_TOUCH_MAJOR, touch_major);
			input_report_abs(idev, ABS_MT_TOUCH_MINOR, touch_minor);
		}
		if (this->touch_orientation_enabled)
			input_report_abs(idev, ABS_MT_ORIENTATION,
				 (cur->wx > cur->wy));
		input_mt_sync(idev);
		break;
	case SYN_FUNCAREA_BUTTON:
		LOG_EVENT(this, "button\n");
		button = (struct clearpad_button_data_t *)
					pointer->funcarea->data;
		if (button)
			button->down = true;
		break;
	default:
		break;
	}
}

static void clearpad_funcarea_up(struct clearpad_t *this,
				  struct clearpad_pointer_t *pointer)
{
	struct clearpad_button_data_t *button;
	struct input_dev *idev = this->input;
	bool valid;

	switch (pointer->funcarea->func) {
	case SYN_FUNCAREA_INSENSIBLE:
		LOG_EVENT(this, "insensible up\n");
		break;
	case SYN_FUNCAREA_POINTER:
		valid = idev->users > 0;
		LOG_EVENT(this, "%s up\n", valid ? "pt" : "unused pt");
		if (!valid)
			break;
		input_mt_sync(idev);
		break;
	case SYN_FUNCAREA_BUTTON:
		LOG_EVENT(this, "button up\n");
		button = (struct clearpad_button_data_t *)
					pointer->funcarea->data;
		if (button)
			button->down = false;
		break;
	default:
		break;
	}
	pointer->funcarea = NULL;
}

static void clearpad_funcarea_out(struct clearpad_t *this,
				  struct clearpad_pointer_t *pointer)
{
	struct clearpad_funcarea_t *new_funcarea ;

	clearpad_funcarea_up(this, pointer);

	new_funcarea = clearpad_funcarea_search(this, pointer);
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

static void clearpad_report_button(struct clearpad_t *this,
		struct clearpad_button_data_t *button)
{
	bool valid = this->input->users > 0;

	if (button->down) {
		if (!button->down_report) {
			button->down_report = true;
			if (valid)
				input_report_key(this->input, button->code, 1);
			LOG_EVENT(this, "%s(%d): down\n",
				  valid ? "key" : "unused key", button->code);
		}
	} else {
		if (button->down_report) {
			button->down_report = false;
			if (valid)
				input_report_key(this->input, button->code, 0);
			LOG_EVENT(this, "%s(%d): up\n",
				  valid ? "key" : "unused key", button->code);
		}
	}
}

static void
clearpad_funcarea_report_extra_events(struct clearpad_t *this)
{
	struct clearpad_funcarea_t *funcarea = this->funcarea;
	struct clearpad_button_data_t *button;

	if (funcarea == NULL)
		return;

	for (; funcarea->func != SYN_FUNCAREA_END; funcarea++) {
		if (funcarea->func == SYN_FUNCAREA_BUTTON) {
			button =
				(struct clearpad_button_data_t *)funcarea->data;
			if (button)
				clearpad_report_button(this, button);
		}
	}
}

static void clearpad_funcarea_invalidate_all(struct clearpad_t *this)
{
	struct clearpad_pointer_t *pointer;
	int i;

	for (i = 0; i < this->extents.n_fingers; ++i) {
		pointer = &this->pointer[i];
		if (pointer->down) {
			pointer->down = false;
			LOG_VERBOSE(this, "invalidate pointer %d\n", i);
		}
		if (pointer->funcarea)
			clearpad_funcarea_up(this, pointer);
	}
	clearpad_funcarea_report_extra_events(this);
	if (this->input->users)
		input_sync(this->input);
}

static void clearpad_parse_finger_n_f11(struct clearpad_t *this,
		const u8 *buf, int finger, struct clearpad_point_t *new_point)
{
	enum registers {
		REG_X_MSB,
		REG_Y_MSB,
		REG_XY_LSB,
		REG_XY_W,
		REG_Z,
	};
	new_point->tool = SYN_FINGER_STATE(buf, finger);
	new_point->tool = clearpad_tool_type_f11[new_point->tool];
	buf += SYN_FINGER_OFF(this->extents.n_fingers, finger,
		this->extents.n_bytes_per_object);
	new_point->id = finger;
	new_point->x = (buf[REG_X_MSB] << 4) | ((buf[REG_XY_LSB] & 0x0f));
	new_point->y = (buf[REG_Y_MSB] << 4) | ((buf[REG_XY_LSB] & 0xf0) >> 4);
	new_point->wx = (buf[REG_XY_W] & 0x0f);
	new_point->wy = ((buf[REG_XY_W] >> 4) & 0x0f);
	new_point->z = buf[REG_Z];
}

static void clearpad_parse_finger_n_f12(struct clearpad_t *this,
		const u8 *buf, int finger, struct clearpad_point_t *new_point)
{
	enum registers {
		REG_TYPE_STATUS,
		REG_X_LSB,
		REG_X_MSB,
		REG_Y_LSB,
		REG_Y_MSB,
		REG_Z,
		REG_WX,
		REG_WY,
	};
	buf += this->extents.n_bytes_per_object * finger;
	new_point->tool = buf[REG_TYPE_STATUS];
	new_point->tool = clearpad_tool_type_f12[new_point->tool];
	new_point->id = finger;
	new_point->x = (buf[REG_X_MSB] << 8) | buf[REG_X_LSB];
	new_point->y = (buf[REG_Y_MSB] << 8) | buf[REG_Y_LSB];
	new_point->wx = buf[REG_WX];
	new_point->wy = buf[REG_WY];
	new_point->z = buf[REG_Z];
}

static void clearpad_report_finger_n(struct clearpad_t *this,
		const u8 *buf, int finger)
{
	struct clearpad_pointer_t *pointer = &this->pointer[finger];
	struct clearpad_point_t new_point;
	int flip_config = 0;

	if (clearpad_is_valid_function(this, SYN_F11_2D))
		clearpad_parse_finger_n_f11(this, buf, finger, &new_point);
	else if (clearpad_is_valid_function(this, SYN_F12_2D))
		clearpad_parse_finger_n_f12(this, buf, finger, &new_point);
	else
		goto err_ret;

	/* check finger state */
	if (new_point.tool == SYN_TOOL_FINGER ||
		(this->glove.enabled && new_point.tool == SYN_TOOL_GLOVE) ||
		(this->pen.enabled && (new_point.tool == SYN_TOOL_PEN))) {

		flip_config = clearpad_flip_config_get(
				this->device_info.customer_family,
				this->device_info.firmware_revision_major);

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
				= clearpad_funcarea_search(this, pointer);
			LOG_VERBOSE(this, "validate pointer %d [func %d]\n",
				    new_point.id, pointer->funcarea
				    ? pointer->funcarea->func : -1);
		}
		if (pointer->funcarea) {
			struct clearpad_area_t *extension
				= &pointer->funcarea->extension;

			if (clearpad_funcarea_test(extension, &new_point)) {
				pointer->cur = new_point;
				clearpad_funcarea_down(this, pointer);
			} else {
				clearpad_funcarea_out(this, pointer);
			}
		}
	} else {
		if (pointer->down) {
			pointer->down = false;
			LOG_VERBOSE(this, "invalidate pointer %d\n", finger);
		}
		if (pointer->funcarea)
			clearpad_funcarea_up(this, pointer);
	}
err_ret:
	return;
}

static int clearpad_read_fingers_f11(struct clearpad_t *this)
{
	int rc, i, size;
	u8 *buf = this->reg_buf;

	/* read status and first finger */
	memset(buf, 0, SYN_REG_MAX);
	size = SYN_FINGER_OFF(this->extents.n_fingers, 1,
		this->extents.n_bytes_per_object);
	rc = clearpad_get_block(SYNF(this, F11_2D, DATA, 0x00), buf, size);
	if (rc)
		goto exit;
	for (i = this->extents.n_fingers - 1 ; i > 0; i--) {
		if (SYN_FINGER_STATE(buf, i)) {
			/* read remained fingers */
			rc = clearpad_get_block(
				SYNF(this, F11_2D, DATA, 0x00) + size,
				buf + size,
				this->extents.n_bytes_per_object * i);
			break;
		}
	}
exit:
	return rc;
}

static int get_num_fingers_f12(struct clearpad_t *this,
	int *num_fingers)
{
	int rc, num;
	u16 val, mask;
	const int max_objects = this->extents.n_fingers;

	rc = clearpad_get_block(SYNS(this, F12_2D, DATA, OBJ_ATTENTION),
		(u8 *)&val, sizeof(val));
	if (rc)
		goto error;

	val = le16_to_cpu(val);

	for (num = 0, mask = 0x1; num < max_objects; num++, mask <<= 1)
		if (val < mask)
			break;

	*num_fingers = num;

	dev_dbg(&this->pdev->dev, "fingers=%d, 0x%04hX", num, val);
error:
	return rc;
}

static int clearpad_read_fingers_f12(struct clearpad_t *this)
{
	int rc, finger, num_fingers;
	u8 buf[this->extents.n_fingers * this->extents.n_bytes_per_object];

	memset(buf, 0, sizeof(buf));

	rc = get_num_fingers_f12(this, &num_fingers);
	if (rc)
		goto err_ret;

	if (num_fingers > 0) {
		rc = clearpad_get_block(
			SYNS(this, F12_2D, DATA, SENSED_OBJECTS),
			buf, num_fingers * this->extents.n_bytes_per_object);
		if (rc)
			goto err_ret;
	}

	for (finger = 0; finger < this->extents.n_fingers; finger++)
		clearpad_report_finger_n(this, buf, finger);
err_ret:
	return rc;
}

static int clearpad_handle_gesture(struct clearpad_t *this)
{
	u8 wakeint = 0;
	int rc = -EIO;

	if (clearpad_is_valid_function(this, SYN_F11_2D)) {
		rc = clearpad_get_block(SYNF(this, F11_2D, DATA,
		this->wakeup_gesture.large_panel ? 0x36 : 0x43),
							&wakeint, 1);
	} else if (clearpad_is_valid_function(this, SYN_F12_2D)) {
		rc = clearpad_get(SYNF(this, F12_2D, DATA, 0x02), &wakeint);
	}
	if (rc)
		goto exit;

	dev_info(&this->pdev->dev, "Gesture %d", wakeint);

	if (time_after(jiffies, this->wakeup_gesture.time_started))
		this->wakeup_gesture.time_started = jiffies +
		msecs_to_jiffies(this->wakeup_gesture.timeout_delay);
	else
		goto exit;

	evdt_execute(this->evdt_node, this->input, wakeint);
exit:
	return rc;
}

static int clearpad_process_F01_RMI(struct clearpad_t *this)
{
	int rc, i;
	u8 status;

	for (i = 0; i < SYN_RETRY_NUM_OF_INITIAL_CHECK; i++) {
		rc = clearpad_get(SYNF(this, F01_RMI, DATA, 0x00), &status);
		if (rc)
			goto exit;
		dev_info(&this->pdev->dev, "status = 0x%02x\n", status);

		if (DEVICE_STATUS_UNCONFIGURED_RESET_OCCURRED == status) {
			dev_info(&this->pdev->dev, "device reset\n");
			if (this->state == SYN_STATE_FLASH_DISABLE) {
				rc = clearpad_flash(this);
				if (rc)
					goto exit;
			} else {
				rc = clearpad_initialize(this);
				if (rc)
					goto exit;
				this->state = SYN_STATE_RUNNING;
			}

			if (this->active &
			    (SYN_STANDBY | SYN_STANDBY_AFTER_TASK))
				rc = clearpad_set_suspend_mode(this);
			else
				rc = clearpad_set_normal_mode(this);

			goto exit;
		} else if ((DEVICE_STATUS_DEVICE_FAILURE == status) ||
			(DEVICE_STATUS_UNCONFIGURED_DEVICE_FAILURE == status)) {
			clearpad_reset_power(this, NULL);
			goto exit;
		} else {
			dev_info(&this->pdev->dev,
					"check fail: retry = %d\n", i);
			msleep(100);
		}
	}
exit:
	return rc;
}

static int clearpad_process_F11_2D(struct clearpad_t *this)
{
	int rc, i;
	u8 status;

	if (this->wakeup_gesture.enabled &&
	    !(this->active & SYN_ACTIVE_POWER)) {
		rc = clearpad_handle_gesture(this);
		goto exit;
	}

	rc = clearpad_get(SYNF(this, F01_RMI, DATA, 0x00), &status);
	LOG_CHECK(this, "rc=%d F01_RMI_DATA00=0x%x\n", rc, status);
	if (rc)
		goto exit;

	if ((DEVICE_STATUS_DEVICE_FAILURE == status) ||
	    (DEVICE_STATUS_UNCONFIGURED_DEVICE_FAILURE == status)) {
		clearpad_reset_power(this, NULL);
		goto exit;
	}

	if (this->chip_id == SYN_CHIP_3000) {
		rc = clearpad_get(SYNF(this, F11_2D, DATA, 0x35), &status);
		LOG_CHECK(this, "rc=%d F11_2D_DATA09=0x%x\n", rc, status);
		if (rc)
			goto exit;
	}

	rc = clearpad_read_fingers_f11(this);
	if (rc)
		goto exit;

	for (i = 0; i < this->extents.n_fingers; ++i)
		clearpad_report_finger_n(this, this->reg_buf, i);

	clearpad_funcarea_report_extra_events(this);
	if (this->input->users)
		input_sync(this->input);
exit:
	return rc;
}

static int clearpad_process_F12_2D(struct clearpad_t *this)
{
	int rc;
	u8 status;

	if (this->wakeup_gesture.enabled &&
	    !(this->active & SYN_ACTIVE_POWER)) {
		rc = clearpad_handle_gesture(this);
		goto exit;
	}

	rc = clearpad_get(SYNF(this, F01_RMI, DATA, 0x00), &status);
	LOG_CHECK(this, "rc=%d F01_RMI_DATA00=0x%x\n", rc, status);
	if (rc)
		goto exit;

	if (DEVICE_STATUS_DEVICE_FAILURE == status) {
		clearpad_reset_power(this, NULL);
		goto exit;
	}

	rc = clearpad_read_fingers_f12(this);
	if (rc)
		goto exit;
	if (this->input->users)
		input_sync(this->input);
exit:
	return rc;
}

static int clearpad_process_irq(struct clearpad_t *this)
{
	int rc;
	u8 interrupt;

	LOCK(this);
	if (!this->wakeup_gesture.lpm_disabled) {
		if (!(this->active & SYN_ACTIVE_POWER)) {
			rc = clearpad_vreg_suspend(this, 0);
			if (rc)
				goto unlock;
			usleep_range(10000, 11000);
		}
	}

	rc = clearpad_get(SYNF(this, F01_RMI, DATA, 0x01), &interrupt);
	LOG_CHECK(this, "rc=%d F01_RMI_DATA1=0x%x\n", rc, interrupt);
	if (rc)
		goto unlock;

	if (interrupt & this->pdt[SYN_F34_FLASH].irq_mask) {
		rc = clearpad_flash(this);
		goto unlock;
	}

	if (interrupt & this->pdt[SYN_F01_RMI].irq_mask) {
		rc = clearpad_process_F01_RMI(this);
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
		rc = clearpad_process_F11_2D(this);
		goto unlock;
	}

	if (interrupt & this->pdt[SYN_F12_2D].irq_mask) {
		rc = clearpad_process_F12_2D(this);
		goto unlock;
	}

	rc = 0;

	dev_info(&this->pdev->dev, "no work, interrupt=[0x%02x]\n", interrupt);
unlock:
	if (rc) {
		dev_err(&this->pdev->dev, "%s: error %d\n", __func__, rc);
		clearpad_reset_power(this, __func__);
	} else {
		this->reset_count = 0;
	}

	UNLOCK(this);

	if (this->pdata->watchdog_enable)
		clearpad_wd_update(this, false);
	return rc;
}

static irqreturn_t clearpad_threaded_handler(int irq, void *dev_id)
{
	struct device *dev = dev_id;
	struct clearpad_t *this = dev_get_drvdata(dev);
	unsigned long flags;

	do {
		(void)clearpad_process_irq(this);

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

static irqreturn_t clearpad_hard_handler(int irq, void *dev_id)
{
	struct device *dev = dev_id;
	struct clearpad_t *this = dev_get_drvdata(dev);
	unsigned long flags;
	irqreturn_t ret;

	spin_lock_irqsave(&this->slock, flags);
	if (unlikely(this->dev_busy)) {
		this->irq_pending = true;
		dev_info(&this->pdev->dev, "Touch irq busy\n");
		ret = IRQ_HANDLED;
	} else {
		this->dev_busy = true;
		ret = IRQ_WAKE_THREAD;
	}
	spin_unlock_irqrestore(&this->slock, flags);
	return ret;
}

static int clearpad_device_open(struct input_dev *dev)
{
	struct clearpad_t *this = input_get_drvdata(dev);
	int rc;

	LOG_STAT(this, "state=%s\n", clearpad_state_name[this->state]);

	switch (this->state) {
	case SYN_STATE_INIT:
		rc = 0;
		break;
	case SYN_STATE_DISABLED:
		rc = -ENODEV;
		break;
	case SYN_STATE_RUNNING:
		rc = clearpad_set_power(this);
		break;
	default:
		rc = -EBUSY;
		break;
	}

	return rc;
}

static void clearpad_device_close(struct input_dev *dev)
{
	struct clearpad_t *this = input_get_drvdata(dev);

	LOG_STAT(this, "state=%s\n", clearpad_state_name[this->state]);

	(void)clearpad_set_power(this);
}

static int clearpad_command_open(struct clearpad_t *this,
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

static ssize_t clearpad_fwdata_write(struct file *file,
		struct kobject *kobj,
		struct bin_attribute *bin_attr,
		char *buf, loff_t pos, size_t size)
{
	int rc;
	struct device *dev = container_of(kobj, struct device, kobj);
	struct clearpad_t *this = dev_get_drvdata(dev);

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
		this->flash.firmware_revision_major =
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
		rc = clearpad_command_open(this, image_size);
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

static struct bin_attribute clearpad_fwdata = {
	.attr = {
		.name = "fwdata",
		.mode = 0600,
	},
	.size = 4096,
	.write = clearpad_fwdata_write
};

static bool clearpad_check_task(struct clearpad_t *this,
			   enum clearpad_state_e *state)
{
	bool rc = false;

	LOCK(this);
	*state = this->state;
	if (*state == SYN_STATE_RUNNING || *state == SYN_STATE_DISABLED)
		rc = true;

	UNLOCK(this);

	return rc;
}

static int clearpad_command_fw_load_start(
					struct clearpad_t *this)
{
	int rc;

	LOCK(this);
	if (this->flash.image == NULL) {
		clearpad_firmware_reset(this);
		rc = sysfs_create_bin_file(&this->input->dev.kobj,
				&clearpad_fwdata);
		if (rc) {
			dev_err(&this->pdev->dev,
					"failed to create fwdata\n");
		}
	} else {
		dev_err(&this->pdev->dev,
				"flash.image already exists\n");
		clearpad_firmware_reset(this);
		rc = -EINVAL;
	}
	UNLOCK(this);

	return rc;
}

static int clearpad_command_fw_flash(struct clearpad_t *this,
					enum clearpad_flash_mode_e flash_mode)
{
	enum clearpad_state_e state;
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
			clearpad_check_task(this, &state))) {
		rc = -ERESTARTSYS;
		goto error;
	}

	clearpad_set_irq(this, this->pdt[SYN_F01_RMI].irq_mask, false);
	synchronize_irq(this->irq);

	LOCK(this);
	memset(this->result_info, 0, SYN_STRING_LENGTH);
	this->flash_requested = true;
	this->flash_mode = flash_mode;

	clearpad_firmware_check(this);

	if (this->active & SYN_STANDBY) {
		/* wake up during flashing */
		this->active &= ~SYN_STANDBY;
		this->active |= SYN_STANDBY_AFTER_TASK;
		LOG_STAT(this, "active: %x (task: %s)\n",
			 this->active, clearpad_task_name[this->task]);
	}
	UNLOCK(this);

	/* wake up */
	rc = clearpad_set_power(this);
	if (rc)
		goto error;

	LOCK(this);
	rc = clearpad_initialize(this);
	LOG_CHECK(this, "rc=%d\n", rc);
	UNLOCK(this);
	if (rc)
		goto error;

	/* wait for end of flash */
	if (wait_event_interruptible(this->task_none_wq,
			clearpad_check_task(this, &state))) {
		rc = -ERESTARTSYS;
		goto error;
	}

	LOCK(this);
	/* check if standby was reserved */
	if (this->active & SYN_STANDBY_AFTER_TASK) {
		this->active &= ~SYN_STANDBY_AFTER_TASK;
		this->active |= SYN_STANDBY;
		LOG_STAT(this, "active: %x (task: %s)\n",
			 this->active, clearpad_task_name[this->task]);
	}
	UNLOCK(this);

	clearpad_set_irq(this, this->pdt[SYN_F01_RMI].irq_mask, true);

	/* restore previous state */
	rc = clearpad_set_power(this);
	if (!rc)
		goto exit;

error:
	clearpad_set_irq(this, this->pdt[SYN_F01_RMI].irq_mask, true);
	snprintf(this->result_info, SYN_STRING_LENGTH,
		"%s, family 0x%02x, fw rev 0x%02x.%02x, extra 0x%02x, " \
		"failed fw update\n",
		clearpad_s(this->device_info.product_id,
					HEADER_PRODUCT_ID_SIZE),
		this->device_info.customer_family,
		this->device_info.firmware_revision_major,
		this->device_info.firmware_revision_minor,
		this->device_info.firmware_revision_extra);
	LOCK(this);
	this->flash_requested = false;
	clearpad_firmware_reset(this);
	dev_info(&this->pdev->dev, "result: %s", this->result_info);
	UNLOCK(this);

	if (this->pdata->watchdog_enable)
		clearpad_wd_update(this, true);
exit:
	return rc;
}

static int clearpad_command_fw_load_end(
					struct clearpad_t *this)
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
		clearpad_firmware_reset(this);
		rc = -EINVAL;
	}
	sysfs_remove_bin_file(&this->input->dev.kobj,
			&clearpad_fwdata);
	UNLOCK(this);

	return rc;
}

static ssize_t clearpad_state_show(struct device *dev,
					      struct device_attribute *attr,
					      char *buf)
{
	struct clearpad_t *this = dev_get_drvdata(dev);

	if (!strncmp(attr->attr.name, __stringify(fwinfo), PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%s, family 0x%02x, fw rev 0x%02x.%02x, extra 0x%02x," \
			" task=%s, state=%s\n",
			clearpad_s(this->device_info.product_id,
				HEADER_PRODUCT_ID_SIZE),
			this->device_info.customer_family,
			this->device_info.firmware_revision_major,
			this->device_info.firmware_revision_minor,
			this->device_info.firmware_revision_extra,
			clearpad_task_name[this->task],
			clearpad_state_name[this->state]);
	else if (!strncmp(attr->attr.name, __stringify(fwfamily), PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%x", this->device_info.customer_family);
	else if (!strncmp(attr->attr.name, __stringify(fwrevision), PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%x", this->device_info.firmware_revision_major);
	else if (!strncmp(attr->attr.name,
			__stringify(fwrevision_minor), PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%x", this->device_info.firmware_revision_minor);
	else if (!strncmp(attr->attr.name,
			__stringify(fwrevision_extra), PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%x", this->device_info.firmware_revision_extra);
	else if (!strncmp(attr->attr.name, __stringify(fwtask), PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%s", clearpad_task_name[this->task]);
	else if (!strncmp(attr->attr.name, __stringify(fwstate), PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%s", clearpad_state_name[this->state]);
	else if (!strncmp(attr->attr.name, __stringify(wakeup_gesture),
		PAGE_SIZE))
		snprintf(buf, PAGE_SIZE, "%d",
			this->wakeup_gesture.enabled);
	else if (!strncmp(attr->attr.name, __stringify(pen), PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%d", this->pen.enabled);
	else if (!strncmp(attr->attr.name, __stringify(glove), PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%d", this->glove.enabled);
	else if (!strncmp(attr->attr.name, __stringify(screen_status),
								PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%d", this->screen_status);
	else if (!strncmp(attr->attr.name, __stringify(charger_status),
								PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%d", this->charger.status);
	else if (!strncmp(attr->attr.name, __stringify(cover_status),
								PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%d", this->cover.status);
	else if (!strncmp(attr->attr.name, __stringify(cover_mode_enabled),
								PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%d", this->cover.enabled);
	else if (!strncmp(attr->attr.name, __stringify(cover_win_top),
								PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%d", this->cover.win_top);
	else if (!strncmp(attr->attr.name, __stringify(cover_win_bottom),
								PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%d", this->cover.win_bottom);
	else if (!strncmp(attr->attr.name, __stringify(cover_win_right),
								PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%d", this->cover.win_right);
	else if (!strncmp(attr->attr.name, __stringify(cover_win_left),
								PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%d", this->cover.win_left);
	else
		snprintf(buf, PAGE_SIZE, "illegal sysfs file");
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t clearpad_fwflush_store(struct device *dev,
					       struct device_attribute *attr,
					       const char *buf, size_t size)
{
	struct clearpad_t *this = dev_get_drvdata(dev);
	const char **command = (const char **)clearpad_flush_commands;
	int rc;

	dev_info(&this->pdev->dev, "flush command: %s\n", buf);

	if (!strncmp(buf, command[SYN_LOAD_START], PAGE_SIZE)) {
		rc = clearpad_command_fw_load_start(this);
	} else if (!strncmp(buf, command[SYN_LOAD_END], PAGE_SIZE)) {
		rc = clearpad_command_fw_load_end(this);
	} else if (!strncmp(buf, command[SYN_FORCE_FLUSH], PAGE_SIZE)) {
		dev_info(&this->pdev->dev, "start firmware flash\n");
		rc = clearpad_command_fw_flash(this, SYN_FLASH_MODE_NORMAL);
	} else if (!strncmp(buf, command[SYN_CONFIG_FLUSH], PAGE_SIZE)) {
		dev_info(&this->pdev->dev, "start firmware config\n");
		rc = clearpad_command_fw_flash(this, SYN_FLASH_MODE_CONFIG);
	} else {
		dev_err(&this->pdev->dev, "illegal command\n");
		rc = -EINVAL;
	}
	if (rc)
		dev_err(&this->pdev->dev, "%s failed\n", __func__);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t clearpad_enabled_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct clearpad_t *this = dev_get_drvdata(dev);
	int rc;

	dev_dbg(&this->pdev->dev, "%s: start\n", __func__);

	LOCK(this);

	if (sysfs_streq(buf, "1") &&
		(this->state == SYN_STATE_IRQ_DISABLED)) {
		rc = clearpad_gpio_export(this, &this->input->dev,
						false);
		if (rc)
			dev_err(&this->pdev->dev,
				"gpio could not be unexported\n");
		goto enable;
	} else if (sysfs_streq(buf, "0") &&
			(this->state == SYN_STATE_RUNNING)) {
		free_irq(this->irq, &this->pdev->dev);
		rc = clearpad_gpio_export(this, &this->input->dev,
						true);
		if (rc) {
			dev_err(&this->pdev->dev,
				"failed gpio export\n");
			goto enable;
		}
		this->state = SYN_STATE_IRQ_DISABLED;
	} else {
		/* No changes */
		dev_info(&this->pdev->dev, "%s: No changes, state=%s\n",
			 __func__, clearpad_state_name[this->state]);
	}
	goto end;

enable:
	rc = request_threaded_irq(this->irq,
				clearpad_hard_handler,
				clearpad_threaded_handler,
				IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
				this->pdev->dev.driver->name,
				&this->pdev->dev);
	if (rc)
		dev_err(&this->pdev->dev,
			"irq %d busy? <%d>\n",
			this->irq, rc);
	else
		this->state = SYN_STATE_RUNNING;

end:
	LOG_STAT(this, "state=%s\n", clearpad_state_name[this->state]);
	UNLOCK(this);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t clearpad_glove_enabled_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct clearpad_t *this = dev_get_drvdata(dev);
	int rc = 0;

	dev_dbg(&this->pdev->dev, "%s: start\n", __func__);

	LOCK(this);

	if (!this->glove.supported) {
		dev_info(&this->pdev->dev, "Glove mode is not supported");
		goto exit;
	}

	this->glove.enabled = sysfs_streq(buf, "0") ? false : true;

	if (!this->cover.status) {
		rc = clearpad_set_glove_mode(this, this->glove.enabled);
		if (rc)
			goto exit;
	}

	dev_info(&this->pdev->dev, "glove mode: %s",
			this->glove.enabled ? "ENABLE" : "DISABLE");
exit:
	UNLOCK(this);

	return rc ? rc : strnlen(buf, PAGE_SIZE);
}

static ssize_t clearpad_pen_enabled_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct clearpad_t *this = dev_get_drvdata(dev);
	int rc = 0;

	dev_dbg(&this->pdev->dev, "%s: start\n", __func__);

	LOCK(this);

	if (!this->pen.supported) {
		dev_info(&this->pdev->dev, "Pen is not supported");
		goto exit;
	}

	this->pen.enabled = sysfs_streq(buf, "0") ? false : true;

	rc = clearpad_set_pen(this);
	if (rc)
		goto exit;

	dev_info(&this->pdev->dev, "pen mode: %s",
			this->pen.enabled ? "ENABLE" : "DISABLE");
exit:
	UNLOCK(this);

	return rc ? rc : strnlen(buf, PAGE_SIZE);
}

static ssize_t clearpad_pca_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct clearpad_t *this = dev_get_drvdata(dev);
	int rc, i, block_count;
	u8 tmp_buf[2] = {0};
	union {
		unsigned short block_num;
		unsigned char data[2];
	} block_num;
	unsigned int block_size;

	LOCK(this);
	if (this->pca_busy) {
		dev_err(&this->pdev->dev,
				"pca is still busy\n");
		rc = -EBUSY;
		UNLOCK(this);
		goto exit;
	}
	this->pca_busy = true;

	if (size > SYN_PCA_ACCESS_MAX_WRITE_SIZE) {
		rc = -EINVAL;
		dev_err(&this->pdev->dev,
		       "Input data size is large (size = %d)\n", size);
		goto err_unlock;
	}

	/* get block size */
	if (this->chip_id == SYN_CHIP_3400 ||
		this->chip_id == SYN_CHIP_3500 ||
		this->chip_id == SYN_CHIP_7500) {
		rc = clearpad_get_block(SYNF(this, F34_FLASH, QUERY, 0x02),
				    tmp_buf, 2);
	} else {
		rc = clearpad_get(SYNF(this, F34_FLASH, QUERY, 0x03),
				  &tmp_buf[0]);
		if (rc)
			goto err_unlock;
		rc = clearpad_get(SYNF(this, F34_FLASH, QUERY, 0x04),
				  &tmp_buf[1]);
	}
	if (rc)
		goto err_unlock;

	block_size = ((tmp_buf[1] << 8) | tmp_buf[0]);

	/* calc need block num with round up */
	block_count = (size + (block_size - 1)) / block_size;
	if (size % block_size) {
		rc = -EINVAL;
		dev_err(&this->pdev->dev,
		       "Writed data size is not multiples of block_size" \
			"(size = %d, block_size = %d)\n", size, block_size);
		goto err_unlock;
	}

	/* change to bootloader mode start */
	/* read bootloader id */
	rc = clearpad_get_block(SYNF(this, F34_FLASH, QUERY, 0x00), tmp_buf, 2);
	if (rc)
		goto err_unlock;

	/* write bootloader id to block data */
	rc = clearpad_put_block(SYNF(this, F34_FLASH, DATA,
				(this->chip_id == SYN_CHIP_3400 ||
				 this->chip_id == SYN_CHIP_3500 ||
				 this->chip_id == SYN_CHIP_7500) ? 0x01 : 0x02),
				tmp_buf, 2);
	if (rc)
		goto err_unlock;

	usleep_range(10000, 11000);

	/* issue a flash program enable */
	rc = clearpad_put(SYNF(this, F34_FLASH, DATA,
			(this->chip_id == SYN_CHIP_3400 ||
			 this->chip_id == SYN_CHIP_3500 ||
			 this->chip_id == SYN_CHIP_7500) ? 0x02 : 0x12),
			FLASH_CONTROL_ENABLE_FLASH_PROGRAMMING);
	if (rc)
		goto err_unlock;

	msleep(100);

	/* PDT have changed, re-read. when success, driver can reset.*/
	rc = clearpad_read_pdt(this);
	if (rc)
		goto err_unlock;

	 /* make sure that we are in programming mode and there are no issues */
	rc = clearpad_get(SYNF(this, F34_FLASH, DATA,
		(this->chip_id == SYN_CHIP_3400 ||
		 this->chip_id == SYN_CHIP_3500 ||
		 this->chip_id == SYN_CHIP_7500) ? 0x03 : 0x12), tmp_buf);
	if (rc)
		goto err_unlock;

	if (tmp_buf[0] != FLASH_CONTROL_PROGRAM_ENABLED) {
		dev_err(&this->pdev->dev,
		       "failed enabling flash (%s)\n",
			(this->chip_id == SYN_CHIP_3400 ||
			 this->chip_id == SYN_CHIP_3500 ||
			 this->chip_id == SYN_CHIP_7500) ?
			clearpad_flash_status[tmp_buf[0] & 7] :
			clearpad_flash_status[(tmp_buf[0] >> 4) & 7]);
		rc = -EIO;
		goto err_unlock;
	}
	/* change to bootloader mode end */

	/* check write block position */
	block_num.block_num = 0;
	for (i = 0; i < block_count; i++) {
		block_num.data[1] &= 0x1F;
		block_num.data[1] |= (FLASH_DATA_CONFIGURATION_AREA << 5);

		/* write block number */
		rc = clearpad_put_block(SYNF(this, F34_FLASH, DATA, 0x00),
				     block_num.data, 2);
		if (rc)
			break;

		/* write block data */
		rc = clearpad_put_block(SYNF(this, F34_FLASH, DATA,
			(this->chip_id == SYN_CHIP_3400 ||
			 this->chip_id == SYN_CHIP_3500 ||
			 this->chip_id == SYN_CHIP_7500) ? 0x01 : 0x02),
				     buf, block_size);
		if (rc)
			break;

		usleep_range(10000, 11000);

		/* issue a write configuration block command */
		rc = clearpad_put(SYNF(this, F34_FLASH, DATA,
				(this->chip_id == SYN_CHIP_3400 ||
				 this->chip_id == SYN_CHIP_3500 ||
				 this->chip_id == SYN_CHIP_7500) ? 0x02 : 0x12),
				FLASH_CONTROL_WRITE_CONFIGURATION_BLOCK);
		if (rc)
			break;

		msleep(100);

		buf += block_size;
		block_num.block_num++;
	}
	dev_info(&this->pdev->dev, "write size = %d", (i * block_size));

	usleep_range(10000, 11000);

	/* send a reset to the device to complete the flash procedure */
	clearpad_put(SYNF(this, F01_RMI, COMMAND, 0x00),
						DEVICE_COMMAND_RESET);
	msleep(100);

err_unlock:
	if (rc)
		dev_err(&this->pdev->dev, "failed to write to touch device");
	this->pca_busy = false;
	UNLOCK(this);
exit:
	return rc ? rc : (i * block_size);
}

static ssize_t clearpad_wakeup_gesture_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	int rc = 0;
	struct clearpad_t *this = dev_get_drvdata(dev);

	dev_dbg(&this->pdev->dev, "%s: start\n", __func__);

	LOCK(this);

	if (!this->wakeup_gesture.supported) {
		dev_info(&this->pdev->dev, "Wakeup gesture is not supported");
		goto exit;
	}

	this->wakeup_gesture.enabled = sysfs_streq(buf, "0") ? false : true;

	device_init_wakeup(&this->pdev->dev,
			this->wakeup_gesture.enabled ? 1 : 0);

	dev_info(&this->pdev->dev, "wakeup gesture: %s",
			this->wakeup_gesture.enabled ? "ENABLE" : "DISABLE");
exit:
	UNLOCK(this);

	return rc ? rc : size;
}

static ssize_t clearpad_screen_status_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct clearpad_t *this = dev_get_drvdata(dev);

	dev_dbg(&this->pdev->dev, "%s: start\n", __func__);

	LOCK(this);

	sscanf(buf, "%d", &this->screen_status);
	dev_dbg(&this->pdev->dev, "%s: screen_status = %d\n", __func__,
				this->screen_status);

	if (this->screen_status) {
		if (!(this->active & SYN_ACTIVE_POWER))
			clearpad_resume(&this->pdev->dev);
	} else {
		if (this->active & SYN_ACTIVE_POWER)
			clearpad_suspend(&this->pdev->dev);
	}

	UNLOCK(this);

	clearpad_set_power(this);

	return strnlen(buf, PAGE_SIZE);
}

static ssize_t clearpad_charger_status_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	int rc = 0;
	struct clearpad_t *this = dev_get_drvdata(dev);

	dev_dbg(&this->pdev->dev, "%s: start\n", __func__);

	LOCK(this);

	if (!this->charger.supported) {
		dev_info(&this->pdev->dev, "Charger mode is not supported");
		goto exit;
	}

	this->charger.status = sysfs_streq(buf, "0") ? false : true;

	rc = clearpad_set_charger(this);
	if (rc)
		goto exit;

	dev_info(&this->pdev->dev, "charger status: %s",
			 this->charger.status ? "CONN" : "DISCONN");
exit:
	UNLOCK(this);

	return rc ? rc : size;
}

static ssize_t clearpad_cover_status_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	int rc = 0;
	struct clearpad_t *this = dev_get_drvdata(dev);

	LOCK(this);

	if (!this->cover.supported) {
		dev_info(&this->pdev->dev, "Cover mode is not supported");
		goto exit;
	}

	if (!clearpad_is_valid_function(this, SYN_F51_CUSTOM)) {
		dev_err(&this->pdev->dev, "%s: FW Not supported", __func__);
		rc = -EPERM;
		goto exit;
	}

	this->cover.status = sysfs_streq(buf, "0") ? false : true;

	if (this->cover.enabled) {
		rc = clearpad_set_cover_status(this);
		if (rc)
			goto exit;
	}

	dev_info(&this->pdev->dev, "cover status: %s",
			this->cover.status ? "CLOSE" : "OPEN");
exit:
	UNLOCK(this);

	return rc ? rc : size;
}

static ssize_t clearpad_cover_mode_enabled_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	int rc = 0;
	struct clearpad_t *this = dev_get_drvdata(dev);

	LOCK(this);

	if (!this->cover.supported) {
		dev_info(&this->pdev->dev, "Cover mode is not supported");
		goto exit;
	}

	if (!clearpad_is_valid_function(this, SYN_F51_CUSTOM)) {
		dev_err(&this->pdev->dev, "%s: FW Not supported", __func__);
		rc = -EPERM;
		goto exit;
	}

	this->cover.enabled = sysfs_streq(buf, "0") ? false : true;

	if (this->cover.enabled) {
		dev_info(&this->pdev->dev, "Cover mode Enabled\n");
	} else {
		this->cover.status = false;
		rc = clearpad_set_cover_status(this);
		dev_info(&this->pdev->dev, "Cover mode Disabled\n");
	}
	if (rc)
		dev_err(&this->pdev->dev, "%s failed\n", __func__);
exit:
	UNLOCK(this);

	return rc ? rc : size;
}

static ssize_t clearpad_cover_win_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	int win_size = 0;
	int rc = 0;
	struct clearpad_t *this = dev_get_drvdata(dev);

	LOCK(this);

	if (!this->cover.supported) {
		dev_info(&this->pdev->dev, "Cover mode is not supported");
		goto exit;
	}

	if (!clearpad_is_valid_function(this, SYN_F51_CUSTOM)) {
		dev_err(&this->pdev->dev, "%s: FW Not supported", __func__);
		rc = -EPERM;
		goto exit;
	}

	if (sscanf(buf, "%d", &win_size) != 1) {
		dev_err(&this->pdev->dev, "%s: %s sscanf failed ",
						__func__, attr->attr.name);
		rc = -EINVAL;
		goto exit;
	}

	dev_info(&this->pdev->dev, "%s: %s = %d\n", __func__,
					attr->attr.name, win_size);

	if (!strncmp(attr->attr.name, "cover_win_top", PAGE_SIZE))
		this->cover.win_top = win_size;
	else if (!strncmp(attr->attr.name, "cover_win_bottom", PAGE_SIZE))
		this->cover.win_bottom = win_size;
	else if (!strncmp(attr->attr.name, "cover_win_right", PAGE_SIZE))
		this->cover.win_right = win_size;
	else if (!strncmp(attr->attr.name, "cover_win_left", PAGE_SIZE))
		this->cover.win_left = win_size;
	rc = clearpad_set_cover_window(this);
exit:
	UNLOCK(this);

	return rc ? rc : size;
}

static struct device_attribute clearpad_sysfs_attrs[] = {
	__ATTR(fwinfo, S_IRUGO, clearpad_state_show, 0),
	__ATTR(fwfamily, S_IRUGO, clearpad_state_show, 0),
	__ATTR(fwrevision, S_IRUGO, clearpad_state_show, 0),
	__ATTR(fwrevision_minor, S_IRUGO, clearpad_state_show, 0),
	__ATTR(fwrevision_extra, S_IRUGO, clearpad_state_show, 0),
	__ATTR(fwtask, S_IRUGO, clearpad_state_show, 0),
	__ATTR(fwstate, S_IRUGO, clearpad_state_show, 0),
	__ATTR(fwflush, S_IWUSR, 0, clearpad_fwflush_store),
	__ATTR(enabled, S_IWUSR, 0, clearpad_enabled_store),
	__ATTR(pen, S_IRUGO | S_IWUSR, clearpad_state_show,
				clearpad_pen_enabled_store),
	__ATTR(glove, S_IRUGO | S_IWUSR, clearpad_state_show,
				clearpad_glove_enabled_store),
	__ATTR(screen_status, S_IRUGO | S_IWUSR, clearpad_state_show,
				clearpad_screen_status_store),
	__ATTR(charger_status, S_IRUGO | S_IWUSR, clearpad_state_show,
				clearpad_charger_status_store),
	__ATTR(pca, S_IRUGO | S_IWUSR, clearpad_pca_show,
				clearpad_pca_store),
	__ATTR(cover_status, S_IRUGO | S_IWUSR,
				clearpad_state_show,
				clearpad_cover_status_store),
	__ATTR(cover_mode_enabled, S_IRUGO | S_IWUSR,
				clearpad_state_show,
				clearpad_cover_mode_enabled_store),
	__ATTR(cover_win_top, S_IRUGO | S_IWUSR,
				clearpad_state_show,
				clearpad_cover_win_store),
	__ATTR(cover_win_bottom, S_IRUGO | S_IWUSR,
				clearpad_state_show,
				clearpad_cover_win_store),
	__ATTR(cover_win_right, S_IRUGO | S_IWUSR,
				clearpad_state_show,
				clearpad_cover_win_store),
	__ATTR(cover_win_left, S_IRUGO | S_IWUSR,
				clearpad_state_show,
				clearpad_cover_win_store)
};

static struct device_attribute clearpad_wakeup_gesture_attr =
	__ATTR(wakeup_gesture, S_IRUGO | S_IWUSR,
				clearpad_state_show,
				clearpad_wakeup_gesture_store);

static int create_sysfs_entries(struct clearpad_t *this)
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

static void remove_sysfs_entries(struct clearpad_t *this)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(clearpad_sysfs_attrs); i++)
		device_remove_file(&this->input->dev, &clearpad_sysfs_attrs[i]);
}

static int clearpad_touch_config_dt(struct clearpad_t *this)
{
	int rc = 0;
	struct device_node *devnode = this->bdata->of_node;

	if (of_property_read_u32(devnode, "chip_id", &this->chip_id)) {
		dev_err(&this->pdev->dev, "no chip_id config\n");
		rc = -EINVAL;
		goto exit;
	}

	if (of_property_read_u32(devnode, "gpio_reset",
		&this->gpio_reset))
		dev_warn(&this->pdev->dev, "no gpio_reset config\n");

	if (of_property_read_u32(devnode, "reset_l2h",
		&this->reset_l2h))
		dev_warn(&this->pdev->dev, "no reset_l2h config\n");

	if (of_property_read_u32(devnode, "num_sensor_rx",
		&this->num_sensor_rx))
		dev_warn(&this->pdev->dev, "no num_sensor_rx config\n");

	if (of_property_read_u32(devnode, "num_sensor_tx",
		&this->num_sensor_tx))
		dev_warn(&this->pdev->dev, "no num_sensor_tx config\n");

	if (of_property_read_u32(devnode, "charger_supported",
		(u32 *)&this->charger.supported))
		dev_warn(&this->pdev->dev, "no charger_supported config\n");

	if (of_property_read_u32(devnode, "pen_supported",
		(u32 *)&this->pen.supported))
		dev_warn(&this->pdev->dev, "no pen_supported config\n");

	if (of_property_read_u32(devnode, "glove_supported",
		(u32 *)&this->glove.supported))
		dev_warn(&this->pdev->dev, "no glove_supported config\n");

	if (of_property_read_u32(devnode, "cover_supported",
		(u32 *)&this->cover.supported))
		dev_warn(&this->pdev->dev, "no cover_supported config\n");

	if (of_property_read_u32(devnode, "touch_pressure_enabled",
		&this->touch_pressure_enabled))
		dev_warn(&this->pdev->dev, "no touch_pressure_enabled " \
			 "config\n");

	if (of_property_read_u32(devnode, "touch_size_enabled",
		&this->touch_size_enabled))
		dev_warn(&this->pdev->dev, "no touch_size_enabled config\n");

	if (of_property_read_u32(devnode, "touch_orientation_enabled",
		&this->touch_orientation_enabled))
		dev_warn(&this->pdev->dev, "no touch_orientation_enabled " \
			 "config\n");

	if (of_property_read_u32(devnode, "preset_x_max", &this->extents.x_max))
		dev_warn(&this->pdev->dev, "no preset_x_max config\n");

	if (of_property_read_u32(devnode, "preset_y_max", &this->extents.y_max))
		dev_warn(&this->pdev->dev, "no preset_y_max config\n");

	if (of_property_read_u32(devnode, "preset_n_fingers",
		&this->extents.n_fingers))
		dev_warn(&this->pdev->dev, "no preset_n_fingers config\n");

	if (of_property_read_u32(devnode, "por_delay_after",
		&this->por_delay_after))
		dev_warn(&this->pdev->dev, "no por_delay_after config\n");

	if (of_property_read_u32(devnode, "wakeup_gesture_supported",
		(u32 *)&this->wakeup_gesture.supported))
		dev_warn(&this->pdev->dev, "no wakeup_gesture_supported\n");

	if (of_property_read_u32(devnode, "wakeup_gesture_large_panel",
		(u32 *)&this->wakeup_gesture.large_panel))
		dev_warn(&this->pdev->dev, "no wakeup_gesture_large_panel\n");

	if (of_property_read_u32(devnode, "wakeup_gesture_lpm_disabled",
		(u32 *)&this->wakeup_gesture.lpm_disabled))
		dev_warn(&this->pdev->dev, "no wakeup_gesture_lpm_disabled\n");

	if (of_property_read_u32(devnode, "wakeup_gesture_timeout",
		&this->wakeup_gesture.timeout_delay))
		dev_warn(&this->pdev->dev, "no wakeup_gesture_timeout\n");

exit:
	return rc;
}

static int clearpad_input_init(struct clearpad_t *this)
{
	int rc;

	this->input = input_allocate_device();
	if (!this->input) {
		rc = -ENOMEM;
		goto exit;
	}

	input_set_drvdata(this->input, this);

	this->input->open = clearpad_device_open;
	this->input->close = clearpad_device_close;
	this->input->name = CLEARPAD_NAME;
	this->input->id.vendor = SYN_CLEARPAD_VENDOR;
	this->input->id.product = 1;
	this->input->id.version = 1;
	this->input->id.bustype = this->bdata->bustype;

	clearpad_funcarea_initialize(this);

	set_bit(EV_ABS, this->input->evbit);

	set_bit(ABS_MT_TRACKING_ID, this->input->absbit);
	set_bit(ABS_MT_TOOL_TYPE, this->input->absbit);

	dev_info(&this->pdev->dev, "Touch area [%d, %d, %d, %d]\n",
		 this->extents.x_min, this->extents.y_min,
		 this->extents.x_max, this->extents.y_max);

	rc = input_register_device(this->input);
	if (rc) {
		dev_err(&this->pdev->dev,
		       "failed to register device\n");
		input_set_drvdata(this->input, NULL);
		input_free_device(this->input);
		goto exit;
	}

exit:
	return rc;
}

static void clearpad_input_ev_init(struct clearpad_t *this)
{
	int rc = 0;

	if (this->wakeup_gesture.supported) {
		this->evdt_node = evdt_initialize(this->bdata->dev, this->input,
						SYN_WAKEUP_GESTURE);
		if (!this->evdt_node) {
			dev_err(&this->pdev->dev, "no wakeup_gesture dt\n");
		} else {
			rc = device_create_file(&this->input->dev,
					&clearpad_wakeup_gesture_attr);
			if (rc)
				dev_err(&this->pdev->dev,
					"sysfs_create_file failed: %d\n", rc);

			dev_info(&this->pdev->dev, "Touch Wakeup Feature OK\n");
			device_init_wakeup(&this->pdev->dev, 0);
		}
	}
}

static void clearpad_suspend(struct device *dev)
{
	struct clearpad_t *this = dev_get_drvdata(dev);
	bool go_suspend;

	go_suspend = (this->task != SYN_TASK_NO_SUSPEND);
	if (go_suspend)
		this->active |= SYN_STANDBY;
	else
		this->active |= SYN_STANDBY_AFTER_TASK;

	LOG_STAT(this, "active: %x (task: %s)\n",
		 this->active, clearpad_task_name[this->task]);
}

static void clearpad_resume(struct device *dev)
{
	struct clearpad_t *this = dev_get_drvdata(dev);
	bool go_resume;

	go_resume = !!(this->active & (SYN_STANDBY | SYN_STANDBY_AFTER_TASK));
	if (go_resume)
		this->active &= ~(SYN_STANDBY | SYN_STANDBY_AFTER_TASK);

	LOG_STAT(this, "active: %x (task: %s)\n",
		 this->active, clearpad_task_name[this->task]);

	clearpad_funcarea_invalidate_all(this);
}

static int clearpad_pm_suspend(struct device *dev)
{
	struct clearpad_t *this = dev_get_drvdata(dev);
	unsigned long flags;
	int rc = 0;

	spin_lock_irqsave(&this->slock, flags);
	if (unlikely(this->dev_busy)) {
		dev_info(dev, "Busy to suspend\n");
		spin_unlock_irqrestore(&this->slock, flags);
		return -EBUSY;
	}
	this->dev_busy = true;
	spin_unlock_irqrestore(&this->slock, flags);

	if (this->active & SYN_ACTIVE_POWER) {
		clearpad_suspend(&this->pdev->dev);
		rc = clearpad_set_power(this);
		if (rc) {
			if (this->reset_count >= SYN_RETRY_NUM_OF_RESET)
				rc = 0; /* stop retry of recovery */
			else
				return rc;
		}
	}

	if (device_may_wakeup(dev)) {
		enable_irq_wake(this->irq);
		dev_info(&this->pdev->dev, "enable irq wake");
	}
	return 0;
}

static int clearpad_pm_resume(struct device *dev)
{
	struct clearpad_t *this = dev_get_drvdata(dev);
	unsigned long flags;
	bool irq_pending;
	int rc = 0;

	if (device_may_wakeup(dev)) {
		disable_irq_wake(this->irq);
		dev_info(&this->pdev->dev, "disable irq wake");
	}

	spin_lock_irqsave(&this->slock, flags);
	irq_pending = this->irq_pending;
	this->irq_pending = false;
	this->dev_busy = false;
	spin_unlock_irqrestore(&this->slock, flags);

	if (unlikely(irq_pending)) {
		dev_dbg(&this->pdev->dev, "Process pending IRQ\n");
		rc = clearpad_process_irq(this);
	}

	return 0;
}

static int clearpad_pm_suspend_noirq(struct device *dev)
{
	struct clearpad_t *this = dev_get_drvdata(dev);
	if (this->irq_pending && device_may_wakeup(dev)) {
		dev_info(&this->pdev->dev, "Need to resume\n");
		return -EBUSY;
	}
	return 0;
}

#ifdef CONFIG_DEBUG_FS
static int clearpad_get_num_tx_physical(struct clearpad_t *this, int num_tx)
{
	int i, rc;
	u8 *buf;
	int num_tx_physical = -1;

	buf = kzalloc(num_tx, GFP_KERNEL);
	if (!buf)
		goto exit;
	rc = clearpad_get_block(SYNF(this, F55_SENSOR, CTRL, 0x2),
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

static void clearpad_analog_test_get_loop_count(struct clearpad_t *this,
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
		case F54_SENSOR_SPEED_REPORT:
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
		case F54_TRX_TO_TRX_SHORT_REPORT:
			*loop_count_i = num_rx + num_tx;
			*loop_count_j = HWTEST_SIZE_OF_ONE_DIMENSION;
			*data_type = HWTEST_S16;
			break;
		case F54_TRX_TO_TRX_SHORT_2_REPORT:
			*loop_count_i = (this->chip_id == SYN_CHIP_3500) ?
						HWTEST_SIZE_OF_TRX_SHORT_2 :
						HWTEST_SIZE_OF_TRX_SHORT_2_TAB;
			*loop_count_j = HWTEST_SIZE_OF_ONE_DIMENSION;
			*data_type = HWTEST_U8;
			break;
		case F54_ABS_RAW_REPORT:
			*loop_count_i = num_rx + num_tx;
			*loop_count_j = HWTEST_SIZE_OF_ONE_DIMENSION;
			*data_type = HWTEST_U32;
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

static void clearpad_analog_test(struct clearpad_t *this,
		u8 f_analog, u8 mode, u8 count)
{
	int rc, i, j, k, len, num_tx = 0, num_rx = 0;
	int loop_count_i, loop_count_j, data_type, data_size;
	u8 buf[14], *data, *line, *pl, fw_rev_extra;
	enum clearpad_state_e state;
	const char delimeter[] = " | ";

	memset(buf, 0, sizeof(buf));

	LOCK(this);
	this->task = SYN_TASK_NO_SUSPEND;
	if (this->active & SYN_STANDBY) {
		this->active &= ~SYN_STANDBY;
		this->active |= SYN_STANDBY_AFTER_TASK;
		LOG_STAT(this, "active: %x (task: %s)\n",
			 this->active, clearpad_task_name[this->task]);
	}
	UNLOCK(this);
	rc = clearpad_set_power(this);
	if (rc)
		goto err_retrun;

	rc = clearpad_put_bit(SYNF(this, F01_RMI, CTRL, 0x00),
			DEVICE_CONTROL_SLEEP_MODE_NO_SLEEP,
			DEVICE_CONTROL_SLEEP_MODE_NO_SLEEP);
	if (rc)
		goto set_power;

	/* Wait until sleep mode is completely changed to NO_SLEEP */
	msleep(20);

	rc = clearpad_put(SYNF(this, F01_RMI, CTRL, 0x01),
			this->pdt[f_analog].irq_mask);
	if (rc)
		goto unset_no_sleep_mode;

	if (f_analog == SYN_F05_ANALOG) {
		rc = clearpad_get_block(SYNF(this, F05_ANALOG, QUERY, 0x00),
				buf, 2);
		if (rc)
			goto err_set_irq_xy;
		num_rx = buf[0];
		num_tx = buf[1];
	} else if (f_analog == SYN_F54_ANALOG) {
		if (this->num_sensor_rx && this->num_sensor_tx) {
			num_rx = this->num_sensor_rx;
			num_tx = this->num_sensor_tx;
		} else {
			dev_info(&this->pdev->dev,
				 "Analog Test not supported\n");
			goto err_set_irq_xy;
		}
	}

	if (f_analog == SYN_F05_ANALOG) {
		num_rx &= ANALOG_QUERY_NUM_OF_ELECTRODES;
		num_tx &= ANALOG_QUERY_NUM_OF_ELECTRODES;
		rc = clearpad_put(SYNF(this, F05_ANALOG, CTRL, 0x00),
				ANALOG_CONTROL_NO_AUTO_CAL);
		if (rc)
			goto err_set_irq_xy;
	} else if (f_analog == SYN_F54_ANALOG &&
			mode != F54_16_IMAGE_REPORT &&
			mode != F54_AUTOSCAN_REPORT &&
			mode != F54_SENSOR_SPEED_REPORT &&
			mode != F54_TRX_TO_TRX_SHORT_2_REPORT &&
			mode != F54_ABS_RAW_REPORT) {
		fw_rev_extra = this->device_info.firmware_revision_extra;
		switch (this->chip_id) {
		case SYN_CHIP_3500:
		case SYN_CHIP_7500:
			switch (fw_rev_extra) {
			case 0x00:
				rc = clearpad_get(
				SYNF(this, F54_ANALOG, QUERY, 0x1E), buf);
				if (rc)
					goto err_set_irq_xy;
				if (buf[0] != 0) {
					rc = clearpad_put_bit(
					SYNF(this, F54_ANALOG, CTRL, 0x38), 0,
					ANALOG_CONTROL_TRANS_CBC_GLOBAL_CAP);
					if (rc)
						goto err_set_irq_xy;
				}
				rc = clearpad_put_bit(
				SYNF(this, F54_ANALOG, CTRL, 0x19), 0,
				ANALOG_CONTROL_CBC_XMTR_CARRIER_SELECT_2);
				if (rc)
					goto err_set_irq_xy;
				break;
			case 0x01:
			case 0x02:
				rc = clearpad_put_bit(
					SYNF(this, F54_ANALOG, CTRL,
					fw_rev_extra == 0x01 ? 0x33 : 0x32), 0,
					ANALOG_CONTROL_TRANS_CBC_GLOBAL_CAP);
				if (rc)
					goto err_set_irq_xy;
				rc = clearpad_put_bit(
				SYNF(this, F54_ANALOG, CTRL, 0x15), 0,
				ANALOG_CONTROL_CBC_XMTR_CARRIER_SELECT_2);
				if (rc)
					goto err_set_irq_xy;
				break;
			default:
				break;
			}
			/* fall through */
		case SYN_CHIP_3400:
			switch (fw_rev_extra) {
			case 0x00:
				rc = clearpad_put_bit(
					SYNF(this, F54_ANALOG, CTRL, 0x17), 0,
					ANALOG_CONTROL_CBC_XMTR_CARRIER_SELECT);
				if (rc)
					goto err_set_irq_xy;
				break;
			default:
				break;
			}
			rc = clearpad_put_bit(
				SYNF(this, F54_ANALOG, CTRL, 0x14),
				ANALOG_CONTROL_NO_SIGNALCLARITY,
				ANALOG_CONTROL_NO_SIGNALCLARITY);
			if (rc)
				goto err_set_irq_xy;
			break;
		default:
			rc = clearpad_put_bit(
				SYNF(this, F54_ANALOG, CTRL, 0x08), 0,
				ANALOG_CONTROL_CBC_CAPACITANCE);
			if (rc)
				goto err_set_irq_xy;
			rc = clearpad_put_bit(
				SYNF(this, F54_ANALOG, CTRL, 0x08), 0,
				ANALOG_CONTROL_CBC_POLARITY);
			if (rc)
				goto err_set_irq_xy;
			rc = clearpad_put_bit(
				SYNF(this, F54_ANALOG, CTRL, 0x08), 0,
				ANALOG_CONTROL_CBC_XMTR_CARRIER_SELECT);
			if (rc)
				goto err_set_irq_xy;
			rc = clearpad_put_bit(
				SYNF(this, F54_ANALOG, CTRL, 0x51),
				ANALOG_CONTROL_NO_SIGNALCLARITY,
				ANALOG_CONTROL_NO_SIGNALCLARITY);
			if (rc)
				goto err_set_irq_xy;
			break;
		}
		rc = clearpad_put_bit(SYNF(this, F54_ANALOG, COMMAND, 0x00),
				ANALOG_COMMAND_FORCE_UPDATE,
				ANALOG_COMMAND_FORCE_UPDATE);
		if (rc)
			goto err_set_irq_xy;
		rc = clearpad_get(SYNF(this, F54_ANALOG, COMMAND, 0x00), buf);
		if (rc)
			goto err_set_irq_xy;
		for (i = 0 ; (buf[0] & ANALOG_COMMAND_FORCE_UPDATE) != 0 ;
				i++) {
			usleep_range(10000, 11000);
			rc = clearpad_get(
				SYNF(this, F54_ANALOG, COMMAND, 0x00), buf);
			if (rc || i > 100)
				goto err_set_irq_xy;
			dev_info(&this->pdev->dev,
				"Force update flag = %x, loop = %d\n",
				buf[0], i);
		}
		rc = clearpad_put_bit(SYNF(this, F54_ANALOG, COMMAND, 0x00),
				ANALOG_COMMAND_FORCE_CAL,
				ANALOG_COMMAND_FORCE_CAL);
		if (rc)
			goto err_set_irq_xy;
		rc = clearpad_get(SYNF(this, F54_ANALOG, COMMAND, 0x00), buf);
		if (rc || buf[0] & ANALOG_COMMAND_GET_REPORT)
			goto err_set_irq_xy;
		for (i = 0 ; (buf[0] & ANALOG_COMMAND_FORCE_CAL) != 0 ; i++) {
			usleep_range(10000, 11000);
			rc = clearpad_get(
				SYNF(this, F54_ANALOG, COMMAND, 0x00), buf);
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
		data_size = sizeof(u8);
		break;
	case HWTEST_S16:
		data_size = sizeof(s16);
		break;
	case HWTEST_U32:
		data_size = sizeof(u32);
		break;
	default:
		dev_err(&this->pdev->dev, "unsupported command\n");
		goto err_set_irq_xy;
	}

	data = kmalloc(num_rx * data_size, GFP_KERNEL);
	if (!data)
		goto err_set_irq_xy;
	len = HWTEST_MAX_DIGITS + 1;
	line = kmalloc(num_rx * (len + sizeof(delimeter)), GFP_KERNEL);
	if (!line)
		goto err_kfree_data;

	for (k = 0; k < count; k++) {
		s64 min_val = UINT_MAX, max_val = INT_MIN;
		dev_info(&this->pdev->dev,
				"ANALOG: mode[%d], num[%d], rx[%d], tx[%d]",
				mode, k, num_rx, num_tx);
		LOCK(this);
		this->state = SYN_STATE_WAIT_FOR_INT;
		UNLOCK(this);

		if (f_analog == SYN_F05_ANALOG) {
			rc = clearpad_put(
				SYNF(this, F05_ANALOG, DATA, 0x01), mode);
			if (rc)
				goto err_reset;
			rc = clearpad_put(
					SYNF(this, F05_ANALOG, COMMAND, 0x00),
					ANALOG_COMMAND_GET_IMAGE);
			if (rc)
				goto err_reset;
		} else if (f_analog == SYN_F54_ANALOG) {
			rc = clearpad_put(
				SYNF(this, F54_ANALOG, DATA, 0x00), mode);
			if (rc)
				goto err_reset;
			rc = clearpad_put(
					SYNF(this, F54_ANALOG, COMMAND, 0x00),
					ANALOG_COMMAND_GET_REPORT);
			if (rc)
				goto err_reset;
		}
		if (wait_event_interruptible(this->task_none_wq,
					clearpad_check_task(this, &state)))
			goto err_reset;

		if (f_analog == SYN_F54_ANALOG) {
			rc = clearpad_put(SYNF(this, F54_ANALOG, DATA, 0x01),
					0x00);
			if (rc)
				goto err_reset;
			rc = clearpad_put(SYNF(this, F54_ANALOG, DATA, 0x02),
					0x00);
			if (rc)
				goto err_reset;
		}
		for (i = 0; i < loop_count_i; i++) {
			if (f_analog == SYN_F05_ANALOG) {
				rc = clearpad_put(
					SYNF(this, F05_ANALOG, DATA, 0x01),
					mode + i);
				if (rc)
					goto err_reset;
				rc = clearpad_get_block(
					SYNF(this, F05_ANALOG, DATA, 0x02),
					data, loop_count_j * 2);
				if (rc)
					goto err_reset;
			} else if (f_analog == SYN_F54_ANALOG) {
				for (j = 0; j < loop_count_j * data_size; j++) {
					rc = clearpad_get(
					SYNF(this, F54_ANALOG, DATA, 0x03),
						data + j);
					if (rc)
						goto err_reset;
				}
			}
			pl = line;
			for (j = 0; j < loop_count_j; j++) {
				s64 val = 0;
				switch (data_type) {
				case HWTEST_U8:
					val = (u8)(*(data + j));
					break;
				case HWTEST_S8:
					val = (s8)(*(data + j));
					break;
				case HWTEST_S16:
					val = (s16)le16_to_cpup(
					(const u16 *)(data + j * data_size));
					break;
				case HWTEST_U32:
					val = (u32)le32_to_cpup(
					(const u32 *)(data + j * data_size));
					break;
				default:
					break;
				}
				if (val >= max_val)
					max_val = val;
				if (val <= min_val)
					min_val = val;
				pl += snprintf(pl, len, "%6lld", val);
				if ((j + 1) % loop_count_j)
					pl += snprintf(pl, sizeof(delimeter),
								delimeter);
			}
			dev_info(&this->pdev->dev, "%s\n", line);
		}
		dev_info(&this->pdev->dev,
			"MIN = %06lld / MAX = %06lld\n", min_val, max_val);
		msleep(100);
	}

err_reset:
	kfree(line);
	kfree(data);
	LOCK(this);
	this->state = SYN_STATE_WAIT_FOR_INT;
	UNLOCK(this);
	clearpad_put(SYNF(this, F01_RMI, COMMAND, 0x00), DEVICE_COMMAND_RESET);
	wait_event_interruptible(this->task_none_wq,
				clearpad_check_task(this, &state));
	goto set_power;

err_kfree_data:
	kfree(data);
err_set_irq_xy:
	if (clearpad_is_valid_function(this, SYN_F11_2D))
		clearpad_put(SYNF(this, F01_RMI, CTRL, 0x01),
				this->pdt[SYN_F11_2D].irq_mask);
	else if (clearpad_is_valid_function(this, SYN_F12_2D))
		clearpad_put(SYNF(this, F01_RMI, CTRL, 0x01),
				this->pdt[SYN_F12_2D].irq_mask);
unset_no_sleep_mode:
	clearpad_put_bit(SYNF(this, F01_RMI, CTRL, 0x00), 0,
				DEVICE_CONTROL_SLEEP_MODE_NO_SLEEP);
set_power:
	LOCK(this);
	this->task = SYN_TASK_NONE;
	if (this->active & SYN_STANDBY_AFTER_TASK) {
		this->active &= ~SYN_STANDBY_AFTER_TASK;
		this->active |= SYN_STANDBY;
		LOG_STAT(this, "active: %x (task: %s)\n",
			 this->active, clearpad_task_name[this->task]);
	}
	UNLOCK(this);
	clearpad_set_power(this);
err_retrun:
	return;
}

static ssize_t clearpad_debug_hwtest_open(struct inode *inode,
		struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static int clearpad_hextou8(struct clearpad_t *this,
			     char **ptr, const char *guard,
			     u8 *value)
{
	int rc = 0;
	char *p = *ptr;
	char s[DEBUG_ONE_BYTE_HEX + 1];
	unsigned long v;

	p = skip_spaces(p);
	if (p + DEBUG_ONE_BYTE_HEX > guard) {
		rc = -EINVAL;
		goto exit;
	}
	strlcpy(s, p, DEBUG_ONE_BYTE_HEX + 1);
	p += DEBUG_ONE_BYTE_HEX;
	rc = kstrtoul(s, 16, &v);
	if (rc) {
		p = NULL;
		goto exit;
	}
	*value = (u8)v;
	*ptr = p;
exit:
	return rc;
}

static int clearpad_debug_read_reg(struct clearpad_t *this,
						u8 page, u8 reg)
{
	int rc;
	u8 value;

	LOCK(this);
	rc = clearpad_get(this, SYN_PAGE_ADDR(page, reg), &value);
	if (!rc)
		dev_info(&this->pdev->dev,
			 "read page=0x%02x, addr=0x%02x, value=0x%02x\n",
			 page, reg, value);
	else
		dev_err(&this->pdev->dev, "Error in reading single register\n");
	UNLOCK(this);
	return rc;
}

static int clearpad_debug_read_packet(struct clearpad_t *this,
					u8 page, u8 reg, u8 length)
{
	int rc, i;
	u8 *pkt;

	LOCK(this);
	pkt = kzalloc(length, GFP_KERNEL);
	if (!pkt) {
		rc = -ENOMEM;
		goto exit;
	}
	rc = clearpad_read_block(this, SYN_PAGE_ADDR(page, reg), pkt, length);
	if (rc > 0) {
		dev_info(&this->pdev->dev,
			 "read page=0x%02x, addr=0x%02x\n", page, reg);
		for (i = 0; i < length; i++)
			dev_info(&this->pdev->dev,
				 "index[%d]=0x%02x\n", i, pkt[i]);
		rc = 0;
	} else {
		dev_err(&this->pdev->dev, "Error in reading packet register\n");
	}
	kzfree(pkt);
exit:
	UNLOCK(this);
	return rc;
}

static int clearpad_debug_write_reg(struct clearpad_t *this,
					u8 page, u8 reg, u8 value)
{
	int rc;

	LOCK(this);
	rc = clearpad_put(this, SYN_PAGE_ADDR(page, reg), value);
	if (!rc)
		dev_info(&this->pdev->dev,
			 "write page=0x%02x, addr=0x%02x, value=0x%02x\n",
			 page, reg, value);
	else
		dev_err(&this->pdev->dev, "Error in writing to register\n");
	UNLOCK(this);
	return rc;
}

static int clearpad_debug_write_packet(struct clearpad_t *this,
		u8 page, u8 reg, u8 length, char *b, char *guard)
{
	u8 *pkt;
	u8 index, value;
	int rc, i;

	LOCK(this);
	pkt = kzalloc(length, GFP_KERNEL);
	if (!pkt) {
		rc = -ENOMEM;
		goto exit;
	}

	/* read modified part of packet */
	rc = clearpad_read_block(this, SYN_PAGE_ADDR(page, reg), pkt, length);
	if (rc < 0)
		goto err_free;

	/* modify indexed data */
	while (b != NULL && *(b + 1) != '\0') {
		if (clearpad_hextou8(this, &b, guard, &index) ||
		    clearpad_hextou8(this, &b, guard, &value) ||
		    index >= length) {
			dev_err(&this->pdev->dev, "invalid arguments\n");
			rc = -EINVAL;
			goto err_free;
		}
		dev_info(&this->pdev->dev, "mod index[%d]=0x%02x(0x%02x)\n",
			index, value, pkt[index]);
		pkt[index] = value;
	}

	/* write back packet*/
	rc = clearpad_put_block(this, SYN_PAGE_ADDR(page, reg), pkt, length);
	if (rc) {
		dev_err(&this->pdev->dev, "Error in writing to pkt register\n");
		goto err_free;
	}

	dev_info(&this->pdev->dev,
		 "write page=0x%02x, addr=0x%02x\n", page, reg);
	for (i = 0; i < length; i++)
		dev_info(&this->pdev->dev, "index[%d]=0x%02x\n", i, pkt[i]);
	rc = 0;

err_free:
	kzfree(pkt);
exit:
	UNLOCK(this);
	return rc;
}

static ssize_t clearpad_debug_hwtest_write(struct file *file,
		const char __user *buf, size_t count, loff_t *pos)
{
	struct clearpad_t *this = (struct clearpad_t *)file->private_data;
	int rc;
	unsigned long arg;
	u8 page, reg, value, length;
	char *bhead = NULL;
	char *b;
	char *guard;
	long l;

	if (count <= HWTEST_SIZE_OF_COMMAND_PREFIX)
		goto err_invalid_arg;

	l = strnlen_user(buf, count - 1);
	b = bhead = kzalloc(l, GFP_KERNEL);
	if (!b) {
		rc = -ENOMEM;
		goto exit;
	}
	rc = strncpy_from_user(b, buf, l);
	if (!rc)
		goto err_free;

	guard = b + l;

	switch (DEBUG_COMMAND(b[0], b[1])) {
	case DEBUG_COMMAND('R', '0'):
		/* R0[2:page][2:reg] */
		b += HWTEST_SIZE_OF_COMMAND_PREFIX;
		if (clearpad_hextou8(this, &b, guard, &page) ||
		    clearpad_hextou8(this, &b, guard, &reg))
			goto err_invalid_arg;
		rc = clearpad_debug_read_reg(this, page, reg);
		break;
	case DEBUG_COMMAND('R', 'P'):
		/* RP[2:page][2:reg] [2:length] */
		b += HWTEST_SIZE_OF_COMMAND_PREFIX;
		if (clearpad_hextou8(this, &b, guard, &page) ||
		    clearpad_hextou8(this, &b, guard, &reg) ||
		    clearpad_hextou8(this, &b, guard, &length))
			goto err_invalid_arg;
		rc = clearpad_debug_read_packet(this, page, reg, length);
		break;
	case DEBUG_COMMAND('W', '0'):
		/* W0[2:page][2:reg][2:value] */
		b += HWTEST_SIZE_OF_COMMAND_PREFIX;
		if (clearpad_hextou8(this, &b, guard, &page) ||
		    clearpad_hextou8(this, &b, guard, &reg) ||
		    clearpad_hextou8(this, &b, guard, &value))
			goto err_invalid_arg;
		rc = clearpad_debug_write_reg(this, page, reg, value);
		break;
	case DEBUG_COMMAND('W', 'P'):
		/* WP[2:page][2:reg] [2:length] [2:index][2:value] ... */
		b += HWTEST_SIZE_OF_COMMAND_PREFIX;
		if (clearpad_hextou8(this, &b, guard, &page) ||
		    clearpad_hextou8(this, &b, guard, &reg) ||
		    clearpad_hextou8(this, &b, guard, &length))
			goto err_invalid_arg;
		rc = clearpad_debug_write_packet(this, page, reg, length,
						 b, guard);
		break;
	case DEBUG_COMMAND('S', '0'):
		/* S000[2:count] */
		rc = kstrtoul(&b[HWTEST_SIZE_OF_COMMAND_PREFIX], 16, &arg);
		if (rc)
			goto err_invalid_arg;
		if (clearpad_is_valid_function(this, SYN_F05_ANALOG)) {
			clearpad_analog_test(this, SYN_F05_ANALOG,
					HWTEST_SIGNAL_DIFF, arg);
			break;
		} else {
			goto err_invalid_arg;
		}
	case DEBUG_COMMAND('B', '0'):
		/* B000[2:count] */
		rc = kstrtoul(&b[HWTEST_SIZE_OF_COMMAND_PREFIX], 16, &arg);
		if (rc)
			goto err_invalid_arg;
		if (clearpad_is_valid_function(this, SYN_F05_ANALOG)) {
			clearpad_analog_test(this, SYN_F05_ANALOG,
					HWTEST_BASELINE, arg);
			break;
		} else {
			goto err_invalid_arg;
		}
	case DEBUG_COMMAND('A', '0'):
		/* A0[2:test type][2:count] */
		rc = kstrtoul(&b[HWTEST_SIZE_OF_COMMAND_PREFIX], 16, &arg);
		if (rc)
			goto err_invalid_arg;
		if (clearpad_is_valid_function(this, SYN_F54_ANALOG)) {
			value = arg;
			reg = arg >> 8;
			clearpad_analog_test(this, SYN_F54_ANALOG, reg, value);
			break;
		} else {
			goto err_invalid_arg;
		}
	case DEBUG_COMMAND('P', '0'):
		/* P0 */
		LOCK(this);
		clearpad_reset_power(this, NULL);
		UNLOCK(this);
		break;
	default:
		goto err_invalid_arg;
		break;
	}
	goto exit;

err_invalid_arg:
	dev_err(&this->pdev->dev, "illegal command\n");
	rc = -EINVAL;
err_free:
	kzfree(bhead);
exit:
	return rc ? rc : count;
}

static const struct file_operations clearpad_debug_hwtest_fops = {
	.owner = THIS_MODULE,
	.open = clearpad_debug_hwtest_open,
	.write = clearpad_debug_hwtest_write,
};

static void clearpad_debug_init(struct clearpad_t *this)
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
				&clearpad_debug_hwtest_fops);
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
	struct clearpad_data_t *cdata = pdev->dev.platform_data;
	struct clearpad_t *this;
	struct kobject *parent;
	char *symlink_name;
	int rc;
	bool retry = false;
#ifdef CONFIG_TOUCHSCREEN_CLEARPAD_RMI_DEV
	struct platform_device *rmi_dev;
#endif

	this = kzalloc(sizeof(struct clearpad_t), GFP_KERNEL);
	if (!this) {
		dev_err(&pdev->dev, "no memory available\n");
		rc = -ENOMEM;
		retry = true;
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

	if (this->bdata->of_node) {
		rc = clearpad_touch_config_dt(this);
		if (rc)
			goto err_free;
	}

#ifdef CONFIG_TOUCHSCREEN_CLEARPAD_RMI_DEV
	if (!cdata->rmi_dev) {
		rmi_dev = platform_device_alloc(CLEARPAD_RMI_DEV_NAME, -1);
		if (!rmi_dev) {
			rc = -ENOMEM;
			goto err_free;
		}

		rmi_dev->dev.parent = &pdev->dev;
		rc = platform_device_add_data(rmi_dev, cdata,
						sizeof(struct clearpad_data_t));
		if (rc)
			goto err_device_put;

		rc = platform_device_add(rmi_dev);
		if (rc)
			goto err_device_put;

		if (!rmi_dev->dev.driver) {
			rc = -ENODEV;
			goto err_device_del;
		}
		cdata->rmi_dev = rmi_dev;
	}
#endif

	rc = clearpad_vreg_configure(this, 1);
	if (rc) {
		dev_err(&this->pdev->dev, "failed vreg init\n");
		goto err_device_del;
	}

	rc = clearpad_gpio_configure(this, 1);
	if (rc) {
		dev_err(&this->pdev->dev, "failed gpio init\n");
		goto err_vreg_teardown;
	}

	msleep(400);

	if (this->pdata->watchdog_enable) {
		this->wd_poll_t_jf = this->pdata->watchdog_poll_t_ms ?
			msecs_to_jiffies(this->pdata->watchdog_poll_t_ms) :
			SYN_WATCHDOG_POLL_DEFAULT_INTERVAL;
		INIT_DELAYED_WORK(&this->wd_poll_work, clearpad_wd_status_poll);
	}

	LOCK(this);
	rc = clearpad_initialize(this);
	UNLOCK(this);
	if (rc) {
		dev_err(&this->pdev->dev, "failed clearpad initialization\n");
		retry = true;
		goto err_gpio_teardown;
	}

	rc = clearpad_input_init(this);
	if (rc)
		goto err_gpio_teardown;

	clearpad_input_ev_init(this);

	this->state = SYN_STATE_RUNNING;

	/* sysfs */
	rc = create_sysfs_entries(this);
	if (rc)
		goto err_input_device;

#ifdef CONFIG_DEBUG_FS
	/* debugfs */
	clearpad_debug_init(this);
#endif

	/* create symlink */
	parent = this->input->dev.kobj.parent;
	symlink_name = this->pdata->symlink_name ? : CLEARPAD_NAME;
	rc = sysfs_create_link(parent, &this->input->dev.kobj, symlink_name);
	if (rc) {
		dev_err(&this->pdev->dev, "sysfs_create_link error\n");
		goto err_sysfs_remove_group;
	}

	this->irq = gpio_to_irq(this->pdata->irq_gpio);
	rc = request_threaded_irq(this->irq,
				clearpad_hard_handler,
				clearpad_threaded_handler,
				IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
				this->pdev->dev.driver->name,
				&this->pdev->dev);
	if (rc) {
		dev_err(&this->pdev->dev, "irq %d busy?\n", this->irq);
		goto err_sysfs_remove_link;
	}
	disable_irq_nosync(this->irq);

	rc = clearpad_set_power(this);
	if (rc) {
		retry = true;
		goto err_irq;
	}

	if (this->chip_id == SYN_CHIP_3200 || this->chip_id == SYN_CHIP_7300) {
		rc = clearpad_put_bit(SYNF(this, F11_2D, COMMAND, 0x00),
				DEVICE_COMMAND_REZERO, DEVICE_COMMAND_REZERO);
		if (rc)
			goto err_irq;
	}
	if (this->chip_id == SYN_CHIP_3500) {
		rc = clearpad_put(SYNF(this, F01_RMI, COMMAND, 0x00),
				   DEVICE_COMMAND_RESET);
		if (rc)
			goto err_irq;
	}

	goto exit;

err_irq:
	free_irq(this->irq, &this->pdev->dev);
err_sysfs_remove_link:
	sysfs_remove_link(parent, symlink_name);
err_sysfs_remove_group:
#ifdef CONFIG_DEBUG_FS
	debugfs_remove_recursive(this->debugfs);
#endif
	remove_sysfs_entries(this);
err_input_device:
	input_unregister_device(this->input);
err_gpio_teardown:
	clearpad_gpio_configure(this, 0);
err_vreg_teardown:
	clearpad_vreg_configure(this, 0);
err_device_del:
#ifdef CONFIG_TOUCHSCREEN_CLEARPAD_RMI_DEV
	if (!cdata->rmi_dev)
		platform_device_del(rmi_dev);
err_device_put:
	if (!cdata->rmi_dev)
		platform_device_put(rmi_dev);
#endif
err_free:
	dev_set_drvdata(&pdev->dev, NULL);
	kfree(this);
exit:
	if (retry) {
		if (cdata->probe_retry < SYN_RETRY_NUM_OF_PROBE_CHECK) {
			rc = -EPROBE_DEFER;
			cdata->probe_retry++;
			msleep(50);
		}
	}
	return rc;
}

static int __devexit clearpad_remove(struct platform_device *pdev)
{
#ifdef CONFIG_TOUCHSCREEN_CLEARPAD_RMI_DEV
	struct clearpad_data_t *cdata = pdev->dev.platform_data;
#endif
	struct clearpad_t *this = dev_get_drvdata(&pdev->dev);
	char *symlink_name = this->pdata->symlink_name ? : CLEARPAD_NAME;

	cancel_delayed_work_sync(&this->wd_poll_work);
	device_init_wakeup(&this->pdev->dev, 0);
	free_irq(this->irq, &this->pdev->dev);
	sysfs_remove_link(this->input->dev.kobj.parent, symlink_name);
#ifdef CONFIG_DEBUG_FS
	debugfs_remove_recursive(this->debugfs);
#endif
	remove_sysfs_entries(this);
	input_unregister_device(this->input);
	clearpad_gpio_configure(this, 0);
	clearpad_vreg_configure(this, 0);
#ifdef CONFIG_TOUCHSCREEN_CLEARPAD_RMI_DEV
	platform_device_put(cdata->rmi_dev);
	cdata->rmi_dev = NULL;
#endif
	dev_set_drvdata(&pdev->dev, NULL);
	kfree(this);

	return 0;
}

static const struct dev_pm_ops clearpad_pm = {
	.suspend = clearpad_pm_suspend,
	.resume = clearpad_pm_resume,
	.suspend_noirq = clearpad_pm_suspend_noirq,
};

static struct platform_driver clearpad_driver = {
	.driver = {
		.name	= CLEARPAD_NAME,
		.owner	= THIS_MODULE,
		.pm	= &clearpad_pm,
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
