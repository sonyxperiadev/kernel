/* msm/clearpad/clearpad_core/clearpad_core.c
 *
 * Copyright (C) 2017 Sony Mobile Communications Inc.
 *
 * Author: Courtney Cavin <courtney.cavin@sonyericsson.com>
 *         Yusuke Yoshimura <Yusuke.Yoshimura@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/atomic.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/incell.h>
#include <linux/of_gpio.h>
#include <linux/ctype.h>
#include <linux/firmware.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/timer.h>
#ifdef CONFIG_FB
#include <linux/notifier.h>
#include <linux/fb.h>
#endif
#ifdef CONFIG_DRM_SDE_SPECIFIC_PANEL
#include <linux/drm_notify.h>
#endif
#ifdef CONFIG_ARM
#include <asm/mach-types.h>
#endif
#include "../include/clearpad.h"
#include "../include/evdt_helper.h"

#define SYN_CLEARPAD_VENDOR		0x1
#define SYN_MAX_N_FINGERS		10
#define SYN_DEVICE_STATUS		0x13
#define SYN_MAX_CTRL_VALUE		80
#define SYN_MAX_Z_VALUE			255
#define SYN_MAX_W_VALUE			15
#define SYN_PDT_START			0xEF
#define SYN_SIZE_OF_FD			6
#define SYN_PAGE_SELECT_OFFSET		0xFF
#define SYN_SUPPORTED_PAGE_NUM		0x04
#define SYN_MAX_INTERRUPT_SOURCE_COUNT	0x7
#define SYN_STRING_LENGTH		128
#define SYN_RETRY_NUM_OF_INITIAL_CHECK	2
#define SYN_RETRY_NUM_OF_PROBE		3
#define SYN_RETRY_NUM_OF_POST_PROBE	6
#define SYN_RETRY_NUM_OF_RECOVERY	3
#define SYN_RETRY_NUM_OF_RESET		5
#define SYN_RETRY_NUM			3
#define SYN_RETRY_DEFAULT_TIME_FOR_NOISE_DET	200
#define SYN_WAIT_TIME_OF_RESET		20
#define SYN_WAIT_TIME_AFTER_REGISTER_ACCESS	20
#define SYN_CALIBRATION_SETUP_TIME	210
#define SYN_CALIBRATION_WAIT		500
#define SYN_CALIBRATION_WAIT_MS		(15 * 1000)
#define SYN_CALIBRATION_EW_WAIT_MS	(10 * 1000)
#define SYN_CALIBRATION_BEFORE_HWRESET_WAIT	20
#define SYN_SINGLE_TRANSACTION_SIZE	8
#define SYN_FP_KEY_OFFSET		6
#define SYN_PAYLOAD_LENGTH		1
#define SYN_DEVICE_INFO_SIZE		5
#define SYN_DEVICE_BL_INFO_SIZE		2
#define SYN_DEVICE_BL70_READ_SIZE	2
#define SYN_DEVICE_BL60_READ_SIZE	4
#define SYN_DEVICE_BL_MAX_SIZE		4
#define SYN_STAMINA_MODE_SUPPORTED_MASK		0x80000000
#define SYN_STAMINA_REPROTRATE_SUPPORTED_MASK	0x00000001
#define SYN_STAMINA_DOZE_HOLDOFF_SUPPORTED_MASK	0x00000002
#define SYN_STAMINA_FORCE_DOZE_ACTIVE_MODE_SUPPORTED_MASK	0x00000004
#define SYN_WAIT_TIME_AFTER_CHANGE_REPORTRATE	34
#define HWTEST_SIZE_OF_COMMAND_PREFIX		2
#define HWTEST_SIZE_OF_ONE_DIMENSION		1
#define HWTEST_SIZE_OF_ONE_HIGH_RX		3
#define HWTEST_SIZE_OF_TX_TO_TX_SHORT(x)	(((x) + 7) / 8)
#define HWTEST_SIZE_OF_TRX_SHORT_2		7
#define HWTEST_SIZE_OF_TRX_SHORT_2_TAB		13
#define HWTEST_MAX_DIGITS			10
#define HWLOG_BUF_SIZE				(PAGE_SIZE * 10)
#define SYN_WAKEUP_GESTURE			"wakeup_gesture"
#define INDENT					"        "
#define FLASH_DATA_CONFIGURATION_AREA_SELECT_PERM	1
#define FLASH_DATA_CONFIGURATION_AREA_SELECT_SHIFT	13
#define FLASH_READ_CONFIGURATION_SIZE			21
#define FLASH_READ_FIRMWARE_INFO_SIZE_BL6X			4
#define FLASH_READ_FIRMWARE_INFO_SIZE_BL7X			5
#define LOCKDOWN_READ_SIZE		10
#define LOCKDOWN_DATA_OFFSET		4
#define LOCKDOWN_DATA_SIZE	(LOCKDOWN_READ_SIZE - LOCKDOWN_DATA_OFFSET)
#define LOCKDOWN_FAMILY_ID_MIN	0x0000
#define LOCKDOWN_FAMILY_ID_MAX	0xffff
#define ANALOG_TEST_NUM_OF_2D_RECEIVERS			12
#define ANALOG_TEST_NUM_OF_2D_TRANSMITTERS		13
#define ANALOG_TEST_SIZE_TO_2D_ELECTRODES_USED		14
#define FORCE_DOZE_ACTIVE_COMMAMD_GET			1
#define FORCE_DOZE_ACTIVE_COMMAMD_SET			2
#define FORCE_DOZE_ACTIVE_MODE_DEFAULT			0
#define FORCE_DOZE_ACTIVE_MODE_ACTIVE			1
#define FORCE_DOZE_ACTIVE_MODE_DOZE			2
#define SYN_STATUS_GET_ERROR				0

/*
 * Register Offset
 */

#define SYN_COVER_RECTANGLE_OFFSET	0x01
#define SYN_EXTERNAL_REPORT_RATE_OFFSET	0x06

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
/* only F12 can use SYNA (e.g. F12_2D_CTRL<regid>) */
#define SYNA(th, func, type, regid) \
	SYNSET(th, SYN_F_PAGE_ADDR(th, SYNFUNC(func), SYNTYPE(type), \
		((th)->pdt[SYNFUNC(func)].offset[SYNTYPE(type)][regid])))

#define DEBUG_COMMAND(C0, C1) (((int)C0 << 8) + (int)C1)
#define DEBUG_ONE_BYTE_HEX	2

#define BIT_DEF(name, mask, shift, ...) \
enum BIT_DEF_##name##_e { \
name##_MASK = mask, name##_SHIFT = shift, ## __VA_ARGS__ \
}
#define BIT_CLEAR(target, name) \
	({ target = ((target) & ~(name##_MASK)); })
#define BIT_SET(target, name, value) \
	({ BIT_CLEAR(target, name); target |= ((value) << name##_SHIFT); })
#define BIT_GET(target, name) \
	({ (target & name##_MASK) >> name##_SHIFT; })

/*
 * Register Bit Fields
 */

/* F01_RMI_CMD00: Device Command */
BIT_DEF(DEVICE_COMMAND_RESET,				0x01, 0);

/* F01_RMI_CTRL00: Device Control */
BIT_DEF(DEVICE_CONTROL_SLEEP_MODE,			0x03, 0,
	DEVICE_CONTROL_SLEEP_MODE_NORMAL_OPERATION	= 0x00,
	DEVICE_CONTROL_SLEEP_MODE_SENSOR_SLEEP		= 0x01,
	DEVICE_CONTROL_SLEEP_MODE_SWR			= 0x02);
BIT_DEF(DEVICE_CONTROL_NO_SLEEP,			0x04, 2);
BIT_DEF(DEVICE_CONTROL_CHARGER_CONNECTED,		0x20, 5);
BIT_DEF(DEVICE_CONTROL_REPORT_RATE,			0x40, 6);
BIT_DEF(DEVICE_CONTROL_CONFIGURED,			0x80, 7);

/* F01_RMI_CTRL01_00: Interrupt Enable 0 */
BIT_DEF(INTERRUPT_ENABLE_0,				0x3f, 0,
	INTERRUPT_ENABLE_0_ENABLE_ALL			= 0x3f,
	INTERRUPT_ENABLE_0_DISABLE_ALL			= 0x00);
BIT_DEF(INTERRUPT_ENABLE_0_FLASH,			0x01, 0);
BIT_DEF(INTERRUPT_ENABLE_0_STATUS,			0x02, 1);
BIT_DEF(INTERRUPT_ENABLE_0_ABS0,			0x04, 2);
BIT_DEF(INTERRUPT_ENALBE_0_ANALOG,			0x08, 3);
BIT_DEF(INTERRUPT_ENALBE_0_BUTTON,			0x10, 4);
BIT_DEF(INTERRUPT_ENABLE_0_SENSOR,			0x20, 5);
BIT_DEF(INTERRUPT_ENABLE_0_PRINT,			0x40, 6);

/* F01_RMI_CTRL18: Device Control 1 */
BIT_DEF(DEVICE_CONTROL_1_GSM_ENABLE,			0x01, 0);

/* F01_RMI_DATA00: Device Status */
BIT_DEF(DEVICE_STATUS_CODE,					0x0f, 0,
	DEVICE_STATUS_CODE_RESET_OCCURRED			= 0x01,
	DEVICE_STATUS_CODE_INVALID_CONFIGURATION		= 0x02,
	DEVICE_STATUS_CODE_DEVICE_FAILURE			= 0x03,
	DEVICE_STATUS_CODE_CONFIGURATION_CRC_FAILURE		= 0x04,
	DEVICE_STATUS_CODE_INVALID_PARTITION_TABLE_V2		= 0x04,
	DEVICE_STATUS_CODE_FIRMWARE_CRC_FAILURE			= 0x05,
	DEVICE_STATUS_CODE_CRC_IN_PROGRESS			= 0x06,
	DEVICE_STATUS_CODE_GUEST_CRC_FAILURE			= 0x07,
	DEVICE_STATUS_CODE_EXTERNAL_AFE_FAILURE			= 0x08,
	DEVICE_STATUS_CODE_CHEACKSUM_PROGRESS_V2		= 0x08,
	DEVICE_STATUS_CODE_DISPLAY_FAILURE			= 0x09,
	DEVICE_STATUS_CODE_FLASHCONFIG_CHEACKSUM_FAILD_V2	= 0x09,
	DEVICE_STATUS_CODE_CORECONFIG_CHEACKSUM_FAILD		= 0x0A,
	DEVICE_STATUS_CODE_COREFIRMWARE_CHEACKSUM_FAILD		= 0x0B,
	DEVICE_STATUS_CODE_GUESTCODE_CHEACKSUM_FAILD		= 0x0C,
	DEVICE_STATUS_CODE_DISPCINFIG_CHEACKSUM_FAILD		= 0x0D,
	DEVICE_STATUS_CODE_DEVICECINFIG_CHEACKSUM_FAILD		= 0x0E);
BIT_DEF(DEVICE_STATUS_FLASH_PROG,			0x40, 6);
BIT_DEF(DEVICE_STATUS_UNCONFIGURED,			0x80, 7);

/* F12_2D_QUERY00: General */
BIT_DEF(GENERAL_HAS_REGISTER_DESCRIPTOR,		0x01, 0);
/* F12_2D_QUERY10_00: Supported Object Types */
BIT_DEF(SUPPORTED_OBJECT_TYPES_HAS_GLOVED_FINGER,	0x20, 5);
BIT_DEF(SUPPORTED_OBJECT_TYPES_HAS_NARROW_OBJECT,	0x40, 6);
BIT_DEF(SUPPORTED_OBJECT_TYPES_HAS_HAND_EDGE,		0x80, 7);
/* F12_2D_QUERY10_01: Supported Object Types 2 */
BIT_DEF(SUPPORTED_OBJECT_TYPES_HAS_COVER,		0x01, 0);
BIT_DEF(SUPPORTED_OBJECT_TYPES_HAS_STYLUS,		0x02, 1);
BIT_DEF(SUPPORTED_OBJECT_TYPES_HAS_ERASER,		0x04, 2);
BIT_DEF(SUPPORTED_OBJECT_TYPES_HAS_SMALL_OBJECT,	0x08, 3);
/* F12_2D_CTRL20_01: Report Flags */
BIT_DEF(REPORT_FLAGS_REPORT_ALWAYS,			0x01, 0);
BIT_DEF(REPORT_FLAGS_REPORT_WAKEUP_GESTURE_ONLY,	0x02, 1);
BIT_DEF(REPORT_FLAGS_ENABLE_DRIBBLE,			0x04, 2);
/* F12_2D_CTRL23_00: Object Report Enable */
BIT_DEF(OBJECT_REPORT_ENABLE_FINGER,			0x01, 0);
BIT_DEF(OBJECT_REPORT_ENABLE_STYLUS,			0x02, 1);
BIT_DEF(OBJECT_REPORT_ENABLE_PALM,			0x04, 2);
BIT_DEF(OBJECT_REPORT_ENABLE_UNCLASSIFIED_OBJECT,	0x08, 3);
BIT_DEF(OBJECT_REPORT_ENABLE_HOVERING_FINGER,		0x10, 4);
BIT_DEF(OBJECT_REPORT_ENABLE_GLOVED_FINGER,		0x20, 5);
BIT_DEF(OBJECT_REPORT_ENABLE_NARROW_OBJECT,		0x40, 6);
BIT_DEF(OBJECT_REPORT_ENABLE_HAND_EDGE,			0x80, 7);
/* F12_2D_CTRL23_02: Report As Finger */
BIT_DEF(REPORT_AS_FINGER_STYLUS,			0x02, 1);
BIT_DEF(REPORT_AS_FINGER_PALM,				0x04, 2);
BIT_DEF(REPORT_AS_FINGER_UNCLASSIFIED_OBJECT,		0x08, 3);
BIT_DEF(REPORT_AS_FINGER_GLOVED_FINGER,			0x20, 5);
BIT_DEF(REPORT_AS_FINGER_NARROW_OBJECT,			0x40, 6);
BIT_DEF(REPORT_AS_FINGER_HAND_EDGE,			0x80, 7);
/* F12_2D_CTRL26: Feature Enable */
BIT_DEF(FEATURE_ENABLE_ENABLE_GLOVED_FINGER_DETECTION,	0x01, 0);
BIT_DEF(FEATURE_ENABLE_ENABLE_CLOSED_COVER_DETECTION,	0x02, 1);
/* F12_2D_CTRL27_00: Wakeup Gesture Enable */
BIT_DEF(WAKEUP_GESTURE_ENABLE_DOUBLE_TAP,		0x01, 0);
BIT_DEF(WAKEUP_GESTURE_ENABLE_SWIPE,			0x02, 1);
BIT_DEF(WAKEUP_GESTURE_ENABLE_TAP_AND_HOLD,		0x04, 2);
BIT_DEF(WAKEUP_GESTURE_ENABLE_CIRCLE,			0x08, 3);
BIT_DEF(WAKEUP_GESTURE_ENABLE_TRIANGLE,			0x10, 4);
BIT_DEF(WAKEUP_GESTURE_ENABLE_VEE,			0x20, 5);
BIT_DEF(WAKEUP_GESTURE_ENABLE_UNICODE,			0x40, 6);
/* F12_2D_CTRL33_00: Multi-Finger Moisture General */
BIT_DEF(ENABLE_MULTIFINGER_MOISTURE,			0x01, 0);

/* F34_FLASH_DATA00: Status */
BIT_DEF(STATUS_FLASH_STATUS,					0x1F, 0,
	STATUS_FLASH_STATUS_SUCCESS				= 0x00,
	STATUS_FLASH_STATUS_DEVICE_NOT_IN_BOOTLOADER_MODE	= 0x01,
	STATUS_FLASH_STATUS_INVALID_PARTITION			= 0x02,
	STATUS_FLASH_STATUS_INVALID_COMMAND			= 0x03,
	STATUS_FLASH_STATUS_INVALID_BLOCK_OFFSET		= 0x04,
	STATUS_FLASH_STATUS_INVALID_TRANSFER			= 0x05,
	STATUS_FLASH_STATUS_NOT_ERASED				= 0x06,
	STATUS_FLASH_STATUS_FLASH_PROGRAMMING_KEY_INCORRECT	= 0x07,
	STATUS_FLASH_STATUS_BAD_PARTITION_TABLE			= 0x08,
	STATUS_FLASH_STATUS_CHECKSUM_FAILED			= 0x09,
	STATUS_FLASH_STATUS_FLASH_HARDWARE_FAILURE		= 0x1F);
BIT_DEF(STATUS_DEVICE_CONFIG_STATUS,			0x60, 5);
BIT_DEF(STATUS_BL_MODE,					0x80, 7);
/* F34_FLASH_DATA02: Flash Control */
BIT_DEF(FLASH_CONTROL,					0x3F, 0,
	FLASH_CONTROL_WRITE_FIRMWARE_BLOCK		= 0x02,
	FLASH_CONTROL_ERASE_ALL				= 0x03,
	FLASH_CONTROL_READ_CONFIGURATION_BLOCK		= 0x05,
	FLASH_CONTROL_WRITE_CONFIGURATION_BLOCK		= 0x06,
	FLASH_CONTROL_ERASE_CONFIGURATION		= 0x07,
	FLASH_CONTROL_ERASE_DISPLAY_BLOCK		= 0x0A,
	FLASH_CONTROL_ENABLE_FLASH_PROGRAMMING		= 0x0F,
	FLASH_CONTROL_READ_LOCKDOWN_BLOCK		= 0x1B);
/* F34_FLASH_DATA03: Flash Status */
BIT_DEF(FLASH_STATUS_PROGRAM_ENABLED,			0x80, 7);

/* F51_CUSTOM_CTRL05.00: Cover */
BIT_DEF(COVER_ENABLE,					0x01, 0);
BIT_DEF(COVER_REPORT_FINGER,				0x02, 1);

/* F54_ANALOG_CMD00: Analog Command */
BIT_DEF(ANALOG_COMMAND_GET_REPORT,			0x01, 0);
BIT_DEF(ANALOG_COMMAND_FORCE_CALIBRATION,		0x02, 1);
BIT_DEF(ANALOG_COMMAND_FORCE_UPDATE,			0x04, 2);

/* F54_ANALOG_CTRL41: Multi Metric Noise Mitigation Control */
BIT_DEF(MULTIMETRIC_NOISE_CTRL_NO_SIGNAL_CLARITY,	0x01, 0);
/* F54_ANALOG_CTRL57: 0D CBC Settings */
BIT_DEF(CBC_SETTINGS_XMTR_CARRIER_SELECT,		0x10, 4);
/* F54_ANALOG_CTRL88: Analog Control 1 */
BIT_DEF(ANALOG_CONTROL_1_CBC_XMTR_CARRIER_SELECT,	0x20, 5);
/* F54_ANALOG_CTRL109: General Control */
BIT_DEF(BASELINE_CORRECTION_MODE,			0x03, 0);
/* F54_ANALOG_CTRL113: General Control */
BIT_DEF(DISABLE_HYBRID_BASELINE,			0x2F, 5);
/* F54_ANALOG_CTRL147: General Control */
BIT_DEF(DISABLE_HYBRID_CBC_AUTO_CORRECTION,		0x02, 1);
/* F54_ANALOG_CTRL149: Trans CBC 2 */
BIT_DEF(TRANS_CBC_2_TRANS_CBC_GLOBAL_CAP,		0x01, 0);
/* F54_ANALOG_CTRL188: Start Calibration or Production Test */
BIT_DEF(START_CAL_PROD_TEST_START_CALIBRATION,		0x01, 0);
BIT_DEF(START_CAL_PROD_TEST_START_IS_CALIBRATION,	0x02, 1);
BIT_DEF(START_CAL_PROD_TEST_SET_FREQUENCY,		0x0C, 2);
BIT_DEF(START_CAL_PROD_TEST_START_PROD_TEST,		0x10, 4);
BIT_DEF(START_CAL_PROD_TEST_START_SHORT_TEST_CAL,	0x60, 5);
/* F54_ANALOG_CTRL214: General Control */
BIT_DEF(ENABLE_HYBRID_CHARGER_NOISE_MITIGATION,		0x01, 0);

/* F54_ANALOG_DATA31: Calibration State */
BIT_DEF(CALIBRATION_STATE_IS_CALIBRATION_CRC,		0x01, 0);
BIT_DEF(CALIBRATION_STATE_CALIBRATION_CRC,		0x02, 1);

#define MAX_USLEEP_RANGE_IN_MS 20

#define LOGx(FUNC, this, X, ...)			\
	FUNC(&this->pdev->dev, "(%s:%d) " X,		\
		__func__, __LINE__, ## __VA_ARGS__)
#define LOGD(this, X, ...) LOGx(dev_dbg, this, X, ## __VA_ARGS__)
#define LOGI(this, X, ...) LOGx(dev_info, this, X, ## __VA_ARGS__)
#define LOGW(this, X, ...) LOGx(dev_warn, this, X, ## __VA_ARGS__)
#define LOGE(this, X, ...) LOGx(dev_err, this, X, ## __VA_ARGS__)
#define DEBUG_FLAG(this, NAME) ({					\
	bool debug_flag = false;					\
	dev_dbg(&this->pdev->dev, NAME " (%d)\n", (debug_flag = true));	\
	dev_info(&this->pdev->dev, "DEBUG_FLAG " NAME " = %s",		\
	debug_flag ? "true" : "false");					\
	debug_flag;							\
})

#define LOG_STAT(this, X, ...) LOGD(this, "stat: " X, ## __VA_ARGS__)
#define LOG_EVENT(this, X, ...) LOGD(this, "event: " X, ## __VA_ARGS__)
#define LOG_CHECK(this, X, ...) LOGD(this, "check: " X, ## __VA_ARGS__)
#define LOG_VERBOSE(this, X, ...) LOGD(this, "verbose: " X, ## __VA_ARGS__)

#define HWLOG(this, format, ...)				\
	clearpad_debug_hwtest_log(this, format, ## __VA_ARGS__)
#define HWLOGx(FUNC, this, format, ...)		\
({						\
	FUNC(this, format, ## __VA_ARGS__);	\
	HWLOG(this, format, ## __VA_ARGS__);	\
})
#define HWLOGD(this, format, ...) HWLOGx(LOGD, this, format, ## __VA_ARGS__)
#define HWLOGI(this, format, ...) HWLOGx(LOGI, this, format, ## __VA_ARGS__)
#define HWLOGE(this, format, ...) HWLOGx(LOGE, this, format, ## __VA_ARGS__)
#define HWLOGW(this, format, ...) HWLOGx(LOGW, this, format, ## __VA_ARGS__)

#define NAME_OF(NAMEARRAY, VALUE)					\
	(((VALUE) < 0 || ARRAY_SIZE(NAMEARRAY) <= (VALUE)) ||		\
	 (NAMEARRAY)[(VALUE)] == NULL ?	"(unknown)" : (NAMEARRAY)[(VALUE)])

#define LOCK(L)						\
({							\
	LOG_STAT(this, "(will lock) <" #L ">\n");	\
	mutex_lock(&(L)->lock);				\
	get_monotonic_boottime(&(L)->ts);		\
	(L)->owner_func = __func__;			\
	(L)->owner_line = __LINE__;			\
	LOG_STAT(this, "LOCKED <" #L ">\n");		\
})

#define TRYLOCK(L)							\
({									\
	int rc;								\
	LOG_STAT(this, "(try lock) <" #L ">\n");			\
	rc = mutex_trylock(&(L)->lock);					\
	if (rc) {							\
		get_monotonic_boottime(&(L)->ts);			\
		(L)->owner_func = __func__;				\
		(L)->owner_line = __LINE__;				\
	}								\
	LOG_STAT(this, "%s <" #L ">\n", rc ? "LOCKED" : "(no lock)");	\
	rc;								\
})

#define UNLOCK(L)				\
({						\
	LOG_STAT(this, "UNLOCK <" #L ">\n");	\
	get_monotonic_boottime(&(L)->ts);	\
	(L)->owner_func = __func__;		\
	(L)->owner_line = __LINE__;		\
	mutex_unlock(&(L)->lock);		\
})

#define IS_LOCKED(L)					\
({							\
	mutex_is_locked(&(L)->lock) ? true : false;	\
})


/*
 * Types
 */

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

enum clearpad_state_e {
	SYN_STATE_INIT,
	SYN_STATE_RUNNING,
	SYN_STATE_FLASH_IMAGE_SET,
	SYN_STATE_FLASH_ENABLE,
	SYN_STATE_FLASH_PROGRAM,
	SYN_STATE_FLASH_ERASE,
	SYN_STATE_FLASH_DATA,
	SYN_STATE_FLASH_DISP,
	SYN_STATE_FLASH_CONFIG,
	SYN_STATE_FLASH_DISABLE,
	SYN_STATE_DISABLED,
};

static const char * const clearpad_state_name[] = {
	[SYN_STATE_INIT]		= "init",
	[SYN_STATE_RUNNING]		= "running",
	[SYN_STATE_FLASH_IMAGE_SET]	= "flash image set",
	[SYN_STATE_FLASH_ENABLE]	= "flash enable",
	[SYN_STATE_FLASH_PROGRAM]	= "flash program",
	[SYN_STATE_FLASH_ERASE]		= "flash erase",
	[SYN_STATE_FLASH_DATA]		= "flash data",
	[SYN_STATE_FLASH_DISP]		= "flash disp",
	[SYN_STATE_FLASH_CONFIG]	= "flash config",
	[SYN_STATE_FLASH_DISABLE]	= "flash disable",
	[SYN_STATE_DISABLED]		= "disabled",
};

enum clearpad_chip_e {
	SYN_CHIP_3500	= 0x38,
	SYN_CHIP_3330	= 0x3A, /* Hybrid incell */
	SYN_CHIP_332U	= 0x40, /* Full incell */
	SYN_CHIP_4353	= 0x44,
};

static const char * const clearpad_chip_name[] = {
	[SYN_CHIP_3500]	= "S3500",
	[SYN_CHIP_3330]	= "S3330", /* Hybrid incell */
	[SYN_CHIP_332U]	= "S332U", /* Full incell */
	[SYN_CHIP_4353]	= "TD4353jdi",
};

enum clearpad_function_e {
	SYN_F01_RMI,
	SYN_F12_2D,
	SYN_F34_FLASH,
	SYN_F51_CUSTOM,
	SYN_F54_ANALOG,
	SYN_F55_SENSOR,
	SYN_N_FUNCTIONS,
};

static const u8 clearpad_function_value[] = {
	[SYN_F01_RMI]		= 0x01,
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

static const char * const clearpad_flash_status_name[] = {
	[STATUS_FLASH_STATUS_SUCCESS]
		= "Success",
	[STATUS_FLASH_STATUS_DEVICE_NOT_IN_BOOTLOADER_MODE]
		= "Device Not In Bootloader Mode",
	[STATUS_FLASH_STATUS_INVALID_PARTITION]
		= "Invalid Partition",
	[STATUS_FLASH_STATUS_INVALID_COMMAND]
		= "Invalid Command",
	[STATUS_FLASH_STATUS_INVALID_BLOCK_OFFSET]
		= "Invalid Block Offset",
	[STATUS_FLASH_STATUS_INVALID_TRANSFER]
		= "Invalid Transfer",
	[STATUS_FLASH_STATUS_NOT_ERASED]
		= "Not Erased",
	[STATUS_FLASH_STATUS_FLASH_PROGRAMMING_KEY_INCORRECT]
		= "Flash Programming Key Incorrect",
	[STATUS_FLASH_STATUS_BAD_PARTITION_TABLE]
		= "Bad Partition Table",
	[STATUS_FLASH_STATUS_CHECKSUM_FAILED]
		= "Checksum Failed",
	[STATUS_FLASH_STATUS_FLASH_HARDWARE_FAILURE]
		= "Flash Hardware Failure",
};

static const char * const clearpad_flash_reason_name[] = {
	[DEVICE_STATUS_CODE_CONFIGURATION_CRC_FAILURE]
				= "Configuration CRC Failure",
	[DEVICE_STATUS_CODE_FIRMWARE_CRC_FAILURE]
				= "Firmware CRC Failure",
	[DEVICE_STATUS_CODE_CRC_IN_PROGRESS]
				= "CRC In Progress",
};

enum clearpad_firmware_e {
	HEADER_SIZE			= 0x100,
	HEADER_VERSION_OFFSET		= 0x07,
	HEADER_DATA_LENGTH		= 0x08,
	HEADER_CONFIG_LENGTH		= 0x0C,
	HEADER_BL_LENGTH		= 0x24,
	HEADER_DISP_OFFSET		= 0x40,
	HEADER_PRODUCT_ID_SIZE		= 10,
};

enum clearpad_firmware_block_e {
	BLOCK_SIZE_16_BYTE		= 0x10,
	BLOCK_SIZE_256_BYTE		= 0x100,
};

enum clearpad_6_8_firmware_disp_block_e {
	DISP_CONFIG_BLOCK		= 0x20,
	DISP_CONFIG_LENGTH		= 0x2000,
};

enum firmware_configuration_const_e {
	CONFIG_CUSTOMER_FAMILY_OFFSET,
	CONFIG_FIRMWARE_REVISION_MAJOR_OFFSET,
	CONFIG_FIRMWARE_REVISION_MINOR_OFFSET,
	CONFIG_FIRMWARE_REVISION_EXTRA_OFFSET,
	CONFIG_FIRMWARE_INFO_SIZE,
};

enum clearpad_container_firmware_e {
	CON_TOP_START_ADDRESS_OFFSET	= 0xc,
	CON_CONTENT_LENGTH_OFFSET	= 0x18,
	CON_CONTENT_ADDRESS_OFFSET	= 0x1c,
	CON_START_ADDRESS_SIZE		= 0x4,
	CON_ID_OFFSET			= 0x4,
};

enum firmware_partition_id_const_e {
	PID_BOOTLOADER			= 1,
	PID_DEVICE_CONFIGURATION	= 2,
	PID_FLASH_CONFIGURATION		= 3,
	PID_MANUFACTURING_BLOCK		= 4,
	PID_GUEST_SERIALIZATION		= 5,
	PID_GLOBAL_PARAMETERS		= 6,
	PID_CORE_CODE			= 7,
	PID_CORE_CONFIGURATION		= 8,
	PID_DISPLAY_CONFIGURATION	= 10,
	PID_EXTERNAL_TOUCH_AFE_CONFIG	= 11,
};

enum firmware_flash_command_const_e {
	FLASH_CMD_IDLE,
	FLASH_CMD_ENTER_BOOTLOADER,
	FLASH_CMD_READ,
	FLASH_CMD_WRITE,
	FLASH_CMD_ERASE,
	FLASH_CMD_ERASE_APPLICATION,
	FLASH_CMD_SENSOR_ID,
};

enum firmware_container_id_const_e {
	CID_CORE_CODE_CONTAINER = 0x12,
	CID_CORE_CONFIGURATION_CONTAINER = 0x13,
};

enum firmware_bootloader_version_const_e {
	BV6	= 0x06,
	BV6_8	= 0x08,
	BV7	= 0x10,
};

static const int clearpad_bootloader_version_dec[] = {
	[BV6]	= 6,
	[BV7]	= 7,
};

enum clearpad_flash_command_e {
	SYN_FORCE_FLASH,
	SYN_DEFAULT_FLASH,
};

static const char * const clearpad_flash_command_name[] = {
	[SYN_FORCE_FLASH]	= "force",
	[SYN_DEFAULT_FLASH]	= "default",
};

enum clearpad_calibration_e {
	SYN_CALIBRATION_NORMAL,
	SYN_CALIBRATION_EW,
};

static const char * const clearpad_calibration_name[] = {
	[SYN_CALIBRATION_NORMAL] = "normal calibration",
	[SYN_CALIBRATION_EW] = "EW calibration",
};

struct clearpad_lock_t {
	struct mutex lock;
	struct timespec ts;
	const char *owner_func;
	int owner_line;
};

struct clearpad_post_probe_t {
	struct delayed_work work;
	bool start;
	bool done;
	int retry;
};

struct clearpad_thread_resume_t {
	struct work_struct work;
	struct workqueue_struct *work_queue;
};

struct clearpad_touchctrl_t {
	struct clearpad_lock_t session_lock;
	const char *session;
	int power_user; /* reference counter */
	bool will_powerdown; /* early powerdown callback has been called */
};

struct clearpad_interrupt_wait_t {
	const char *name;
	bool use;
	wait_queue_head_t wq;
	atomic_t done;
	int result;
};

struct clearpad_interrupt_t {
	u32 count;
	u32 wait_ms;
	struct clearpad_interrupt_wait_t for_reset;
	struct clearpad_interrupt_wait_t for_F34;
	struct clearpad_interrupt_wait_t for_F54;
	struct timespec hard_handler_ts;
	struct timespec threaded_handler_ts;
	struct timespec handle_first_event_ts;
};

enum clearpad_reset_e {
	SYN_HWRESET,
	SYN_SWRESET,
	SYN_FORCE_HWRESET,
	SYN_FORCE_SWRESET,
};

static const char * const clearpad_reset_name[] = {
	[SYN_HWRESET]	= "HW reset",
	[SYN_SWRESET]	= "SW reset",
	[SYN_FORCE_HWRESET]	= "Force HW reset",
	[SYN_FORCE_SWRESET]	= "Force SW reset",
};

struct clearpad_reset_t {
	enum clearpad_reset_e mode;
	struct delayed_work work;
	u32 delay_for_powerup_ms;
	int retry;
};

struct clearpad_device_info_t {
	u8 manufacturer_id;
	u8 product_properties;
	u8 customer_family;
	u8 firmware_revision_major;
	u8 firmware_revision_minor;
	u8 firmware_revision_extra;
	u8 analog_id;
	u8 product_id[HEADER_PRODUCT_ID_SIZE];
	u8 boot_loader_version_major;
	u8 boot_loader_version_minor;
	u32 bootloader_version;
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
	u32 length;
	int pos;
	const u8 *data;
	u16 transation_count;
	u16 remain_block;
	u16 payload_length;
	u16 block_size;
};

struct clearpad_flash_t {
	u8 format_version;
	int config_size;
	u8 customer_family;
	u8 firmware_revision_major;
	u8 firmware_revision_minor;
	u8 firmware_revision_extra;
	u8 analog_id;
	u8 product_id[HEADER_PRODUCT_ID_SIZE];
	struct clearpad_flash_block_t data;
	struct clearpad_flash_block_t config;
	struct clearpad_flash_block_t disp;
	const struct firmware *fw;
	const char *firmware_name;
	enum clearpad_flash_command_e command;
	bool enter_bootloader_mode;
	bool on_post_probe;
	unsigned long default_timeout_ms;
};

struct clearpad_extents_t {
	int preset_x_max, preset_y_max;
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
	u32 tag_x_max;
	u32 tag_y_max;
	u32 convert_window_size;
};

struct clearpad_wakeup_gesture_t {
	bool supported;
	bool enabled;
	unsigned long time_started;
	u32 timeout_delay;
	bool use_workaround_for_felica;
};

struct clearpad_watchdog_t {
	struct delayed_work work;
	int delay;
	bool enabled;
};

struct clearpad_noise_detect_t {
	spinlock_t slock;
	bool supported;
	bool enabled;
	u32 hard_handler_count;
	u32 threaded_handler_count;
	u32 irq_gpio_flags;
	int irq_gpio;
	int irq;
	bool first_irq;
	int retry_time_ms;
	struct timespec hard_handler_ts;
	struct timespec threaded_handler_ts;
};

struct clearpad_hwtest_t {
	struct clearpad_lock_t lock;
	char log_buf[HWLOG_BUF_SIZE];
	size_t log_size;
};

struct descriptor_t {
	u16 container_id;
	u8 major_version;
	u8 minor_version;
	u32 content_start_addr;
	u32 content_len;
};

struct clearpad_change_reportrate_t {
	bool supported;
	u8 mode;
};

struct clearpad_doze_holdoff_t {
	bool supported;
	u8 default_time;
	u8 glove_mode_time;
	u8 cover_mode_time;
	u8 force_doze_mode_time;
};

struct clearpad_doze_force_active_t {
	bool supported;
	u8 mode;
};

struct clearpad_stamina_mode_t {
	bool supported;
	bool enabled;
	struct clearpad_change_reportrate_t change_reportrate;
	struct clearpad_doze_holdoff_t doze_holdoff;
	struct clearpad_doze_force_active_t force_doze_active;
};

struct fb_t {
	bool unblank_done;
	bool unblank_early_done;
};

struct clearpad_reg_offset_t {
	bool updated;

	u8 f01_cmd00;
	u8 f01_ctrl00;
	u8 f01_ctrl01;
	u8 f01_ctrl05;
	u8 f01_ctrl18;
	u8 f01_data00;
	u8 f01_data01;
	u8 f01_query11;

	u8 f12_ctrl08;

	u8 f34_ctrl00;
	u8 f34_data00;
	u8 f34_data01;
	u8 f34_data02;
	u8 f34_data03;
	u8 f34_data04;
	u8 f34_data05;
	u8 f34_query00;
	u8 f34_query01;
	u8 f34_query03;

	u8 f51_ctrl05;
	u8 f51_ctrl30;

	u8 f54_cmd00;
	u8 f54_ctrl41;
	u8 f54_ctrl57;
	u8 f54_ctrl88;
	u8 f54_ctrl109;
	u8 f54_ctrl113;
	u8 f54_ctrl147;
	u8 f54_ctrl149;
	u8 f54_ctrl188;
	u8 f54_ctrl214;
	u8 f54_data00;
	u8 f54_data01;
	u8 f54_data02;
	u8 f54_data03;
	u8 f54_data31;
	u8 f54_query38;
};

struct clearpad_charger_only_t {
	unsigned long delay_ms;
};

struct clearpad_lockdown_area_t {
	u8 data[LOCKDOWN_DATA_SIZE];
	u8 somc_id;	/* 0x02 */
	u8 supplier_id;	/* 0x01 (Synaptics) */
	u16 chip_id;
	u16 family_id;
};

enum clearpad_hwtest_data_type_e {
	HWTEST_NULL,
	HWTEST_U8,
	HWTEST_S8,
	HWTEST_S16,
	HWTEST_U32,
};

enum clearpad_f54_command_e {
	F54_16_IMAGE_REPORT		= 2,
	F54_AUTOSCAN_REPORT		= 3,
	F54_HIGH_RESISTANCE_REPORT	= 4,
	F54_RAW_CAP_RX_COUPLING_REPORT	= 20,
	F54_SENSOR_SPEED_REPORT		= 22,
	F54_TRX_TO_TRX_SHORT_2_REPORT	= 26,
	F54_EWMODE_RAW_CAP_REPORT	= 38,
	F54_ABS_RAW_REPORT		= 63,
	F54_FULLINCELL_RAW_CAP_REPORT	= 83,
	F54_FULLINCELL_CAL_DATA_CHK_REPORT = 84,
	F54_FULLINCELL_SENSOR_SPEED_REPORT = 86,
	F54_TRX_TO_TRX_SHORT_RAW_IMAGE_REPORT = 100,
	F54_HIC_RX_SHORT_TEST_REPORT    = 125,
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

enum clearpad_force_sleep_e {
	FSMODE_OFF	= 0,	/* Normal suspend/resume */
	FSMODE_KEEP	= 1,	/* Sleep forever */
	FSMODE_ONESHOT	= 2,	/* Oneshot (will be FSMODE_OFF by resume) */
};

struct clearpad_t {
	enum   clearpad_state_e state;
	enum   clearpad_chip_e chip_id;
	struct input_dev *input;
	struct platform_device *pdev;
	struct clearpad_platform_data_t *pdata;
	struct clearpad_bus_data_t *bdata;
	struct clearpad_lock_t lock;
	struct clearpad_touchctrl_t touchctrl;
	struct clearpad_post_probe_t post_probe;
	struct clearpad_thread_resume_t thread_resume;
	struct clearpad_device_info_t device_info;
	struct clearpad_funcarea_t *funcarea;
	struct clearpad_pointer_t pointer[SYN_MAX_N_FINGERS];
	struct clearpad_function_descriptor_t pdt[SYN_N_FUNCTIONS];
	struct clearpad_flash_t flash;
	struct clearpad_wakeup_gesture_t wakeup_gesture;
	struct clearpad_charger_t charger;
	struct clearpad_pen_t pen;
	struct clearpad_glove_t glove;
	struct clearpad_cover_t cover;
	struct clearpad_extents_t extents;
	struct clearpad_reset_t reset;
	struct clearpad_noise_detect_t noise_det;
	struct clearpad_interrupt_t interrupt;
	struct clearpad_stamina_mode_t stamina;
	struct clearpad_reg_offset_t reg_offset;
	struct clearpad_charger_only_t charger_only;
	int irq;
	atomic_t irq_enabled;
	enum clearpad_force_sleep_e force_sleep;
#if defined (CONFIG_FB) \
&&         !defined(CONFIG_DRM_SDE_SPECIFIC_PANEL)
	struct notifier_block fb_notif;
#endif
#ifdef CONFIG_DRM_SDE_SPECIFIC_PANEL
	struct notifier_block drm_notif;
#endif
	char fwname[SYN_STRING_LENGTH + 1];
	char result_info[SYN_STRING_LENGTH + 1];
	bool flash_requested;
	struct clearpad_hwtest_t hwtest;
	u32 flip_config;
	u32 touch_pressure_enabled;
	u32 touch_size_enabled;
	u32 touch_orientation_enabled;
	bool calibration_supported;
	bool calibrate_on_fwflash;
	struct device_node *evdt_node;
	struct clearpad_watchdog_t watchdog;
	spinlock_t slock;
	bool dev_active;
	bool dev_busy;
	bool early_suspend; /* suspend mode has been set from early suspend */
	bool irq_pending;
	bool last_irq;
	struct fb_t wakeup;
	struct clearpad_lockdown_area_t lda;
	bool access_test_enabled;
	struct device virtdev;
};

/*
 * Function prototypes
 */

static void clearpad_update_chip_id(struct clearpad_t *this);
static void clearpad_post_probe_work(struct work_struct *work);
static void clearpad_thread_resume_work(struct work_struct *work);
static void clearpad_funcarea_initialize(struct clearpad_t *this);
static int clearpad_handle_if_first_event(struct clearpad_t *this);
static void clearpad_reset(struct clearpad_t *this,
			  enum clearpad_reset_e mode, const char *cause);
static void clearpad_funcarea_invalidate_all(struct clearpad_t *this);
static int clearpad_set_resume_mode(struct clearpad_t *this);
static int clearpad_set_suspend_mode(struct clearpad_t *this);
static int clearpad_read_lockdown_area(struct clearpad_t *this);
static unsigned int clearpad_get_lockdown_family_id(struct clearpad_t *this);
static int clearpad_process_irq(struct clearpad_t *this);
static int clearpad_flash(struct clearpad_t *this);
static bool clearpad_process_noise_det_irq(struct clearpad_t *this);
static void clearpad_touch_config_dt_for_chip_id(struct clearpad_t *this,
						int chip_id);
static int clearpad_driver_access_test(struct clearpad_t *this);
static int clearpad_debug_hwtest_log(struct clearpad_t *this,
				     const char *format, ...);
static void clearpad_debug_info(struct clearpad_t *this);
static int clearpad_hextou8(struct clearpad_t *this,
			     char **ptr, const char *guard,
			     u8 *value);
static int clearpad_debug_read_reg(struct clearpad_t *this,
						u8 page, u8 reg);
static int clearpad_debug_read_packet(struct clearpad_t *this,
					u8 page, u8 reg, u8 length);
static int clearpad_debug_write_reg(struct clearpad_t *this,
					u8 page, u8 reg, u8 value);
static int clearpad_debug_write_packet(struct clearpad_t *this,
		u8 page, u8 reg, u8 length, char *b, char *guard);
static void clearpad_analog_test(struct clearpad_t *this,
		u8 f_analog, u8 mode, u8 count);
static int clearpad_debug_get_calibration_result(struct clearpad_t *this,
						 u8 check_mode);
static int clearpad_debug_do_calibration(struct clearpad_t *this, int mode);
static void clearpad_debug_registers(struct clearpad_t *this);

/*
 * Functions
 */

static inline bool is_equal_cstring(const char *a, const char *b)
{
	size_t length = max(strnlen(a, PAGE_SIZE), strnlen(b, PAGE_SIZE));

	return strncmp(a, b, length) == 0;
}

static char *clearpad_s(u8 *array, size_t size)
{
	static char string[SYN_STRING_LENGTH + 1];

	memset(string, 0, SYN_STRING_LENGTH + 1);
	size = (SYN_STRING_LENGTH < size) ? SYN_STRING_LENGTH : size;
	memcpy(string, array, size);

	return string;
}

static void clearpad_set_delay(unsigned long ms)
{
	if (ms > 0)
		ms <= MAX_USLEEP_RANGE_IN_MS ?
		usleep_range(ms * 1000, (ms * 1000) + 1000) : msleep(ms);
}

static inline bool clearpad_is_valid_function(struct clearpad_t *this, int func)
{
	return (0 <= func && func < SYN_N_FUNCTIONS)
		&& (this->pdt[func].number == clearpad_function_value[func]);
}

static bool is_irq_enabled(struct clearpad_t *this)
{
	return atomic_read(&this->irq_enabled) == 1;
}

/* need LOCK(&this->lock) */
static void clearpad_set_irq(struct clearpad_t *this, bool enable)
{
	int rc = atomic_cmpxchg(&this->irq_enabled, !enable, enable);

	if (rc == enable) {
		dev_err(&this->pdev->dev, "%s: unbalanced, tried to set %d\n",
				__func__, enable);
		return;
	}
	if (enable) {
		enable_irq(this->irq);
		LOGI(this, "irq was enabled\n");
	} else {
		disable_irq_nosync(this->irq);
		LOGI(this, "irq was disabled\n");
	}
}

static int clearpad_set_noise_det_irq(struct clearpad_t *this, bool enable,
				      bool set_first_irq)
{
	int ret = 0;
	unsigned long flags;

	if (this->noise_det.supported && this->chip_id == SYN_CHIP_332U) {
		spin_lock_irqsave(&this->noise_det.slock, flags);
		if (enable && set_first_irq)
			this->noise_det.first_irq = true;
		if (enable && !this->noise_det.enabled) {
			enable_irq(this->noise_det.irq);
		} else if (!enable && this->noise_det.enabled) {
			disable_irq_nosync(this->noise_det.irq);
		} else {
			spin_unlock_irqrestore(&this->noise_det.slock, flags);
			ret = -1;
			goto end;
		}
		this->noise_det.enabled = enable;
		spin_unlock_irqrestore(&this->noise_det.slock, flags);
	} else {
		ret = -1;
	}
end:
	return ret;
}

/* need LOCK(&this->lock) */
static void clearpad_notify_interrupt(struct clearpad_t *this,
				      struct clearpad_interrupt_wait_t *wait,
				      int result)
{
	if (wait->use) {
		LOGI(this, "handled interrupt '%s' (rc=%d)\n",
		     wait->name, result);
		atomic_inc(&wait->done);
		wait->result = result;
		wake_up_all(&wait->wq);
	}
}

/* need LOCK(&this->lock) */
static void clearpad_prepare_for_interrupt(struct clearpad_t *this,
				struct clearpad_interrupt_wait_t *wait,
				const char *name)
{
	if (wait->use)
		LOGE(this, "already used '%s' (%s)\n", wait->name, name);

	wait->name = name;
	wait->use = true;
	atomic_set(&wait->done, 0);
}

/* need LOCK(&this->lock) */
static void clearpad_undo_prepared_interrupt(struct clearpad_t *this,
				struct clearpad_interrupt_wait_t *wait,
				const char *name)
{
	if (is_equal_cstring(wait->name, name)) {
		LOGE(this, "undo prepared interrupt %s\n", wait->name);
		wait->name = NULL;
		wait->use = false;
	}
}

/* NO need LOCK(&this->lock) */
static int clearpad_wait_for_interrupt(struct clearpad_t *this,
				       struct clearpad_interrupt_wait_t *wait,
				       unsigned long ms)
{
	int rc;
	int retry = 0;

	LOG_STAT(this, "(wait for interrupt) <%s>\n", wait->name);
retry:
	rc = wait_event_timeout(wait->wq,
				atomic_read(&wait->done), msecs_to_jiffies(ms));
	if (rc == 0) {
		LOGW(this, "timeout '%s' (retry %d)\n", wait->name, retry);
		rc = -ETIMEDOUT;
		if (retry++ < SYN_RETRY_NUM)
			goto retry;
	} else if (rc < 0) {
		LOGE(this, "failed to wait for '%s' (rc=%d done=%d)",
		     wait->name, rc, atomic_read(&wait->done));
	} else {
		LOG_STAT(this, "received interrupt <%s> (done=%d)\n",
			 wait->name, atomic_read(&wait->done));
		rc = 0;
	}

	LOCK(&this->lock);
	wait->name = NULL;
	wait->use = false;
	if (!rc)
		rc = wait->result;
	UNLOCK(&this->lock);

	return rc;
}

/*
 * touchctrl : porting for exported functions
 */

static int touchctrl_hwreset(struct clearpad_t *this, int mode)
{
	int rc = 0;
	int raw_mode = 0;
	int retry;

	LOGI(this, "execute '%s'\n", NAME_OF(clearpad_reset_name, mode));
	this->wakeup.unblank_done = false;
	this->wakeup.unblank_early_done = false;
	switch (mode) {
	case SYN_HWRESET:
	case SYN_FORCE_HWRESET:
		raw_mode = INCELL_TOUCH_RESET;
		break;
	default:
		LOGE(this, "unknown HW reset mode (%d)\n", mode);
		return -EINVAL;
	}

	for (retry = 0; retry < SYN_RETRY_NUM; retry++) {
		rc = incell_control_mode(raw_mode, INCELL_FORCE);
		if (rc != INCELL_EBUSY)
			break;
	}

	return rc;
}

static bool touchctrl_is_touch_powered(struct clearpad_t *this)
{
	incell_pw_status status = { false, false };
	int rc;

	rc = incell_get_power_status(&status);
	if (rc)
		LOGE(this, "failed to get power status\n");
	else
		LOGD(this, "power status (touch %s, display %s)\n",
		     status.touch_power ? "ON" : "OFF",
		     status.display_power ? "ON" : "OFF");

	return rc == 0 && status.touch_power;
}

static bool touchctrl_is_display_powered(struct clearpad_t *this)
{
	incell_pw_status status = { false, false };
	int rc;

	rc = incell_get_power_status(&status);
	if (rc)
		LOGE(this, "failed to get power status\n");
	else
		LOGD(this, "power status (touch %s, display %s)\n",
		     status.touch_power ? "ON" : "OFF",
		     status.display_power ? "ON" : "OFF");

	return rc == 0 && status.display_power;
}

/*
 * need LOCK(&this->lock)
 *
 * @return true : locked with specified condition
 *        false : not locked
 */
static bool touchctrl_lock_power(struct clearpad_t *this, const char *id,
				bool need_touch_power, bool need_display_power)
{
#if defined(CONFIG_DRM_SDE_SPECIFIC_PANEL) || defined(CONFIG_FB)
	struct clearpad_touchctrl_t *touchctrl = &this->touchctrl;
	incell_pw_status status = { false, false };
	int rc;
	bool result = false;

	LOG_STAT(this, "(will lock power) <%s>\n", id);

	if (touchctrl->power_user != 0) {
		rc = incell_get_power_status(&status);
		if (rc)
			LOGE(this, "failed to get power status (rc=%d)\n", rc);
		goto check_condition;
	}

	/* stop lock if it will be powered down */
	if (!this->wakeup_gesture.enabled && touchctrl->will_powerdown) {
		LOG_STAT(this, "(no lock) will be powered down <%s>\n", id);
		goto err_in_condition_of_powerdown;
	}

	/* the first user locks power */
	rc = incell_power_lock_ctrl(INCELL_DISPLAY_POWER_LOCK, &status);
	switch (rc) {
	case INCELL_OK:
		break;
	case INCELL_ALREADY_LOCKED:
		LOGW(this, "already locked <%s>\n", id);
		WARN_ON(rc == INCELL_ALREADY_LOCKED);
		break;
	default:
		LOGE(this, "failed to lock power(%d) <%s>\n", rc, id);
		goto err_in_power_lock_ctrl;
	}

check_condition:
	if ((need_touch_power && !status.touch_power) ||
	    (need_display_power && !status.display_power)) {
		LOG_STAT(this, "(no lock) power due to condition) <%s> "
			 "touch=%s(%s) display=%s(%s)\n", id,
			 status.touch_power ? "On" : "Off",
			 need_touch_power ? "required" : "optional",
			 status.display_power ? "On" : "Off",
			 need_display_power ? "required" : "optional");
		if (incell_power_lock_ctrl(INCELL_DISPLAY_POWER_UNLOCK,
								&status))
			LOGE(this, "failed to unlock power\n");
		rc = 0;
		goto err_in_locked_power_status;
	}
	result = true;
	touchctrl->power_user += 1;
	LOG_STAT(this, "LOCKED power <%s> (%d)\n", id, touchctrl->power_user);
err_in_locked_power_status:
err_in_power_lock_ctrl:
err_in_condition_of_powerdown:
	return result;
#else
	struct clearpad_touchctrl_t *touchctrl = &this->touchctrl;
	touchctrl->power_user += 1;
	return true;
#endif
}

/* need LOCK(&this->lock) */
static void touchctrl_unlock_power(struct clearpad_t *this, const char *id)
{
#if defined(CONFIG_DRM_SDE_SPECIFIC_PANEL) || defined(CONFIG_FB)
	struct clearpad_touchctrl_t *touchctrl = &this->touchctrl;
	incell_pw_status status = { false, false };
	int rc;
	unsigned long flags;

	LOG_STAT(this, "unlock power for touch <%s>\n", id);

	if (this->last_irq && is_equal_cstring(id, "irq_handler")) {
		LOGD(this, "last pending IRQ handled\n");
		this->last_irq = false;
		touchctrl->power_user -= 1;
	}

	touchctrl->power_user -= 1;
	if (touchctrl->power_user > 0) {
		LOG_STAT(this, "UNLOCKED power <%s> user(%d)\n",
			 id, touchctrl->power_user);
		goto unlocked;
	} else if (touchctrl->power_user < 0) {
		LOGE(this, "invalid unlock <%s> user(%d)\n",
		     id, touchctrl->power_user);
		touchctrl->power_user = 0;
	}

	/* suspend when the last user unlocks power */
	if (touchctrl->will_powerdown) {
		if (this->dev_active) {
			rc = clearpad_set_suspend_mode(this);
			if (rc)
				LOGE(this, "failed to suspend\n");

			if (this->watchdog.enabled)
				cancel_delayed_work(&this->watchdog.work);
			cancel_delayed_work(&this->reset.work);
		}
		/* check for pending IRQ on suspend mode */
		if (!this->dev_active) {
			spin_lock_irqsave(&this->slock, flags);
			if (unlikely(this->dev_busy || this->irq_pending)) {
				touchctrl->power_user += 1;
				this->last_irq = true;
				spin_unlock_irqrestore(&this->slock, flags);
				LOGD(this, "there is a pending IRQ, "
					   "will not yet Unlock Power\n");
				goto unlocked_with_pending_irq;
			}
			spin_unlock_irqrestore(&this->slock, flags);
		}
	}

	rc = incell_power_lock_ctrl(INCELL_DISPLAY_POWER_UNLOCK, &status);
	switch (rc) {
	case INCELL_OK:
		break;
	case INCELL_ALREADY_UNLOCKED:
		LOGW(this, "already unlocked <%s>\n", id);
		break;
	default:
		LOGE(this, "failed to unlock power(%d) <%s>\n", rc, id);
		goto err_in_power_lock_ctrl;
	}
	LOG_STAT(this, "UNLOCKED power <%s> touch=%s display=%s\n", id,
		 status.touch_power ? "On" : "Off",
		 status.display_power ? "On" : "Off");

err_in_power_lock_ctrl:
unlocked_with_pending_irq:
unlocked:
	return;
#else
	struct clearpad_touchctrl_t *touchctrl = &this->touchctrl;
	touchctrl->power_user -= 1;
	return;
#endif
}

/* Begin a session to use touch device requiring power supply */
static int clearpad_ctrl_session_begin(struct clearpad_t *this,
				       const char *session)
{
	struct clearpad_touchctrl_t *touchctrl = &this->touchctrl;
	int rc = 0;

	pm_stay_awake(&this->pdev->dev);
	LOCK(&touchctrl->session_lock);
	LOGI(this, "begin '%s' session\n", session);
	touchctrl->session = session;

	LOCK(&this->lock);
	/* keep touch power for this session */
	if (!touchctrl_lock_power(this, session, true, false)) {
		LOGE(this, "failed to lock power\n");
		rc = -EAGAIN;
		goto err_in_lock_power;
	}
	rc = clearpad_handle_if_first_event(this);
	if (rc < 0) {
		LOGE(this, "failed to handle first event\n");
		rc = -EBUSY;
		goto err_in_first_event_handling;
	} else {
		rc = 0;
	}
	UNLOCK(&this->lock);
	goto end;

err_in_first_event_handling:
	touchctrl_unlock_power(this, session);
err_in_lock_power:
	UNLOCK(&this->lock);
	touchctrl->session = NULL;
	UNLOCK(&touchctrl->session_lock);
	pm_relax(&this->pdev->dev);
end:
	return rc;
}

/* End a session to use touch device requiring power supply */
static void clearpad_ctrl_session_end(struct clearpad_t *this,
				     const char *session)
{
	struct clearpad_touchctrl_t *touchctrl = &this->touchctrl;

	if (touchctrl->session == NULL ||
	    !is_equal_cstring(touchctrl->session, session)) {
		LOGW(this, "try invalid session end '%s' (current '%s')",
		     session, touchctrl->session);
	}

	/* release power for this session */
	LOCK(&this->lock);
	touchctrl_unlock_power(this, session);
	UNLOCK(&this->lock);

	LOGI(this, "end '%s' session\n", session);
	touchctrl->session = NULL;
	UNLOCK(&touchctrl->session_lock);
	pm_relax(&this->pdev->dev);
}

/*
 * Basic bus access
 */

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

/*
 * Read/Write data of specific size
 */

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
		goto end;

	buf = (buf & ~mask) | val;
	rc = clearpad_put(this, addr, buf);
end:
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

static int clearpad_set_doze_holdoff_time(struct clearpad_t *this,
					  u8 holdoff_time)
{
	int rc = 0;
	u8 current_time = 0x00;

	if (!this->stamina.supported) {
		LOGI(this, "stamina mode is not supported\n");
		goto end;
	}
	if (!this->stamina.doze_holdoff.supported) {
		LOGI(this, "doze holdoff is not supported\n");
		goto end;
	}

	if (clearpad_get(
		SYNF(this, F01_RMI, CTRL, this->reg_offset.f01_ctrl05),
			&current_time)) {
		LOGE(this, "failed to get current Doze Holdoff\n");
		rc = -EINVAL;
		goto end;
	}
	if (current_time == holdoff_time) {
		HWLOGI(this, "new Doze Holdoff is same as current=0x%02x\n",
			current_time);
		goto end;
	}

	/* F01_RMI_CTRL05: Doze Holdoff */
	rc = clearpad_put(
			SYNF(this, F01_RMI, CTRL,
			     this->reg_offset.f01_ctrl05),
			holdoff_time);
	if (rc) {
		LOGE(this, "failed to set new Doze Holdoff=0x%02x\n",
			holdoff_time);
		goto end;
	}
	/* F54_ANALOG_CMD00: Analog Command */
	rc = clearpad_put(
			SYNF(this, F54_ANALOG, COMMAND,
				this->reg_offset.f54_cmd00),
			ANALOG_COMMAND_FORCE_UPDATE_MASK);
	if (rc) {
		LOGE(this, "failed to set force update\n");
		goto end;
	}
	HWLOGI(this, "changed Doze Holdoff=0x%02x\n", holdoff_time);

end:
	return rc;
}

/* need LOCK(&this->lock) */
static int clearpad_set_doze_holdoff(struct clearpad_t *this)
{
	int rc = 0;
	u8 holdoff_time = 0x00;
	u8 nosleep = 0x00;

	if (!this->stamina.supported) {
		LOGI(this, "stamina mode is not supported\n");
		goto end;
	}
	if (!this->stamina.doze_holdoff.supported) {
		LOGI(this, "doze holdoff is not supported\n");
		goto end;
	}

	LOGI(this, "glove mode: %s, cover mode/status: %s/%s\n",
		this->glove.enabled ? "enable" : "disable",
		this->cover.enabled ? "enable" : "disable",
		this->cover.status ? "CLOSE" : "OPEN");

	holdoff_time = this->stamina.doze_holdoff.default_time;
	if (this->glove.enabled)
		holdoff_time = this->stamina.doze_holdoff.glove_mode_time;
	if (this->cover.enabled && this->cover.status)
		holdoff_time = this->stamina.doze_holdoff.cover_mode_time;

	if (this->stamina.force_doze_active.supported) {
		switch (this->stamina.force_doze_active.mode) {
		case FORCE_DOZE_ACTIVE_MODE_DEFAULT:
			break;
		case FORCE_DOZE_ACTIVE_MODE_ACTIVE:
			nosleep = DEVICE_CONTROL_NO_SLEEP_MASK;
			break;
		case FORCE_DOZE_ACTIVE_MODE_DOZE:
			holdoff_time = this->stamina.doze_holdoff.
				force_doze_mode_time;
			break;
		default:
			LOGE(this, "invalid mode (%d)\n",
				this->stamina.force_doze_active.mode);
			break;
		}

		rc = clearpad_put_bit(SYNF(this, F01_RMI, CTRL,
				this->reg_offset.f01_ctrl00),
				nosleep, DEVICE_CONTROL_NO_SLEEP_MASK);
		if (rc) {
			LOGE(this, "failed to set no sleep\n");
			goto end;
		}
	}

	rc = clearpad_set_doze_holdoff_time(this, holdoff_time);
	if (rc)
		goto end;

end:
	return rc;
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
	clearpad_default_funcarea_array[0].original.x2 =
						this->extents.preset_x_max;
	clearpad_default_funcarea_array[0].original.y2 =
						this->extents.preset_y_max;
	clearpad_default_funcarea_array[0].extension.x2 =
						this->extents.preset_x_max;
	clearpad_default_funcarea_array[0].extension.y2 =
						this->extents.preset_y_max;
	return clearpad_default_funcarea_array;
}

static int clearpad_read_pdt(struct clearpad_t *this)
{
	struct clearpad_function_descriptor_t fdes;
	u8 addr = SYN_PDT_START - 1;
	u8 irq_bit = 0;
	u8 page = 0;
	int i, j, k;
	int rc;

	memset(&this->pdt, 0, sizeof(*this->pdt) * SYN_N_FUNCTIONS);
	for (i = 0; i < SYN_N_FUNCTIONS && addr >= SYN_SIZE_OF_FD - 1;) {
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
				HWLOGI(this, "F%02x page:0x%02x "
				       "base[DATA:0x%02x "
				       "CTRL:0x%02x COMMAND:0x%02x "
				       "QUERY:0x%02x] irq:0x%02x\n",
				       fdes.number,
				       fdes.page,
				       fdes.base[SYN_TYPE_DATA],
				       fdes.base[SYN_TYPE_CTRL],
				       fdes.base[SYN_TYPE_COMMAND],
				       fdes.base[SYN_TYPE_QUERY],
				       fdes.irq_mask);
				break;
			}
		}
	}

	return rc;
}

/*
 * Charger
 */

static int clearpad_set_charger(struct clearpad_t *this)
{
	int rc = 0;

	if (!this->charger.supported)
		goto end;

	if (this->charger.status)
		/* F01_RMI_CTRL00: Device Command */
		rc = clearpad_put_bit(
			SYNF(this, F01_RMI, CTRL, this->reg_offset.f01_ctrl00),
			DEVICE_CONTROL_CHARGER_CONNECTED_MASK,
			DEVICE_CONTROL_CHARGER_CONNECTED_MASK);
	else
		/* F01_RMI_CTRL00: Device Command */
		rc = clearpad_put_bit(
			SYNF(this, F01_RMI, CTRL, this->reg_offset.f01_ctrl00),
			0, DEVICE_CONTROL_CHARGER_CONNECTED_MASK);
	if (rc)
		LOGE(this, "failed to set charger status");
end:
	return rc;
}

/*
 * Pen
 */

static int clearpad_set_pen(struct clearpad_t *this)
{
	int rc = 0;

	if (!this->pen.supported)
		goto end;

	if (!clearpad_is_valid_function(this, SYN_F12_2D)) {
		LOGE(this, "F12 is required to set pen\n");
		rc = -EPERM;
		goto end;
	}

	/* F12_2D_CTRL23_00: Object Report Enable */
	rc = clearpad_put_bit(SYNA(this, F12_2D, CTRL, 23),
			      this->pen.enabled ?
			      OBJECT_REPORT_ENABLE_STYLUS_MASK : 0,
			      OBJECT_REPORT_ENABLE_STYLUS_MASK);
end:
	if (rc)
		LOGE(this, "failed to set pen");
	return rc;
}

/*
 * Glove finger register in touch ic
 */

static int clearpad_set_glove_finger_reg(struct clearpad_t *this, bool enable)
{
	int rc = 0;

	/* F12_2D_CTRL23_00: Object Report Enable */
	rc = clearpad_put_bit(SYNA(this, F12_2D, CTRL, 23),
			      enable ?
			      OBJECT_REPORT_ENABLE_GLOVED_FINGER_MASK : 0,
			      OBJECT_REPORT_ENABLE_GLOVED_FINGER_MASK);
	if (rc) {
		LOGE(this, "error in setting for object report enable\n");
		goto end;
	}
	if (this->chip_id == SYN_CHIP_3330 ||
	    this->chip_id == SYN_CHIP_332U) {
		/* F12_2D_CTRL26: Feature Enable */
		rc = clearpad_put_bit(SYNA(this, F12_2D, CTRL, 26),
		      enable ?
		      FEATURE_ENABLE_ENABLE_GLOVED_FINGER_DETECTION_MASK : 0,
		      FEATURE_ENABLE_ENABLE_GLOVED_FINGER_DETECTION_MASK);
		if (rc) {
			LOGE(this, "error in setting for feature enable\n");
			goto end;
		}
	}
end:
	if (rc)
		LOGE(this, "failed to set glove finger register in touch ic \n");
	return rc;
}

/*
 * Glove
 */

static int clearpad_set_glove_mode(struct clearpad_t *this, bool enable)
{
	int rc = 0;
	u8 buf;

	if (!this->glove.supported)
		goto end;

	if (!clearpad_is_valid_function(this, SYN_F12_2D)) {
		LOGE(this, "F12 is required to set glove mode\n");
		rc = -EPERM;
		goto end;
	}

	/* F12_2D_QUERY10: Supported Object Types */
	rc = clearpad_get(SYNA(this, F12_2D, QUERY, 10), &buf);
	if (rc) {
		LOGE(this, "failed to get supported types");
		goto end;
	}
	if (!BIT_GET(buf, SUPPORTED_OBJECT_TYPES_HAS_GLOVED_FINGER)) {
		LOGI(this, "glove mode is not supported\n");
		goto end;
	}

	/* Set glove finger register in touch ic */
	if (!this->cover.status) {
		rc = clearpad_set_glove_finger_reg(this, enable);
		if (rc)
			goto end;
		if (this->chip_id == SYN_CHIP_4353) {
			/* F54_ANALOG_CMD00: Analog Command */
			rc = clearpad_put(SYNF(this, F54_ANALOG, COMMAND,
					this->reg_offset.f54_cmd00),
				ANALOG_COMMAND_FORCE_UPDATE_MASK);
			if (rc) {
				LOGE(this, "failed to set force update\n");
				goto end;
			}
		}
		rc = clearpad_set_doze_holdoff(this);
		if (rc)
			LOGE(this, "failed to set Doze Holdoff\n");
	}
end:
	if (rc)
		LOGE(this, "failed to set glove mode");
	return rc;
}

/*
 * Smart Cover
 */

static int clearpad_set_cover_status(struct clearpad_t *this)
{
	int rc = 0;
	size_t buf_size = 3;
	u8 buf[buf_size];

	if (!clearpad_is_valid_function(this, SYN_F12_2D) &&
	    !clearpad_is_valid_function(this, SYN_F51_CUSTOM) &&
	    !clearpad_is_valid_function(this, SYN_F54_ANALOG)) {
		LOGE(this, "F12, F51 and F54 are required to set cover status");
		rc = -EPERM;
		goto end;
	}

	switch (this->chip_id) {
	case SYN_CHIP_3330:
	case SYN_CHIP_332U:
	case SYN_CHIP_4353:
		/* F12_2D_CTRL26: Feature Enable */
		rc = clearpad_put_bit(SYNA(this, F12_2D, CTRL, 26),
		      this->cover.status ?
		      FEATURE_ENABLE_ENABLE_CLOSED_COVER_DETECTION_MASK : 0,
		      FEATURE_ENABLE_ENABLE_CLOSED_COVER_DETECTION_MASK);
		break;
	case SYN_CHIP_3500:
		/* F51_CUSTOM_CTRL05.00: Cover */
		rc = clearpad_put(
			SYNF(this, F51_CUSTOM, CTRL,
				this->reg_offset.f51_ctrl05),
			this->cover.status ?
			COVER_ENABLE_MASK | COVER_REPORT_FINGER_MASK : 0x00);
		break;
	default:
		LOGE(this, "not supported chip id (0x%02x)\n", this->chip_id);
		rc = -EINVAL;
		break;
	}
	if (rc)
		goto end;

	/* Set glove finger register in touch ic */
	if (this->cover.status) {
		rc = clearpad_set_glove_finger_reg(this, true);
	} else {
		rc = clearpad_set_glove_finger_reg(this, this->glove.enabled);
	}
	if (rc)
		goto end;

	switch (this->chip_id) {
	case SYN_CHIP_3330:
	case SYN_CHIP_332U:
		/* Report Glove As Finger setting is not needed */
		break;
	case SYN_CHIP_3500:
	case SYN_CHIP_4353:
		/* F12_2D_CTRL23_02: Report As Finger */
		rc = clearpad_get_block(SYNA(this, F12_2D, CTRL, 23), buf,
					buf_size);
		if (rc)
			goto end;

		BIT_SET(buf[F12_2D_CTRL_REPORT_AS_FINGER],
			REPORT_AS_FINGER_GLOVED_FINGER,
			this->cover.status ? 1 : 0);
		rc = clearpad_put_block(SYNA(this, F12_2D, CTRL, 23), buf,
					buf_size);
		if (rc)
			goto end;
		break;
	default:
		LOGE(this, "not supported chip id (0x%02x)\n", this->chip_id);
		break;
	}

	/* F54_ANALOG_CMD00: Analog Command */
	rc = clearpad_put(
			SYNF(this, F54_ANALOG, COMMAND,
				this->reg_offset.f54_cmd00),
			ANALOG_COMMAND_FORCE_UPDATE_MASK);
	if (rc)
		goto end;

	rc = clearpad_set_doze_holdoff(this);
	if (rc)
		LOGE(this, "failed to set Doze Holdoff\n");
end:
	if (rc)
		LOGE(this, "failed to set cover status");

	return rc;
}

static int clearpad_set_cover_window(struct clearpad_t *this)
{
	int rc;
	u8 block_buf[8];

	if (this->chip_id == SYN_CHIP_3330 ||
	    this->chip_id == SYN_CHIP_332U ||
	    this->chip_id == SYN_CHIP_4353) {
		/* Cover window size register F12_2D_CTRL25 */
		if (!clearpad_is_valid_function(this, SYN_F12_2D)) {
			LOGE(this, "F12_2D is required to set cover window");
			rc = -EPERM;
			goto end;
		}

		block_buf[0] = this->cover.win_left;
		block_buf[1] = this->cover.win_left >> 8;
		block_buf[2] = this->cover.win_right;
		block_buf[3] = this->cover.win_right >> 8;
		block_buf[4] = this->cover.win_top;
		block_buf[5] = this->cover.win_top >> 8;
		block_buf[6] = this->cover.win_bottom;
		block_buf[7] = this->cover.win_bottom >> 8;

		/* F12_2D_CTRL25: Closed Xmin/Xmax/Ymin/Ymax */
		rc = clearpad_put_block(SYNA(this, F12_2D, CTRL, 25),
				block_buf, 8);
		if (rc)
			goto end;
	} else {
		/* Cover window size register F51_CUSTOM_CTRL05 */
		if (!clearpad_is_valid_function(this, SYN_F51_CUSTOM)) {
			LOGE(this, "F51 is required to set cover window");
			rc = -EPERM;
			goto end;
		}

		block_buf[0] = this->cover.win_left;
		block_buf[1] = this->cover.win_left >> 8;
		block_buf[2] = this->cover.win_top;
		block_buf[3] = this->cover.win_top >> 8;
		block_buf[4] = this->cover.win_right;
		block_buf[5] = this->cover.win_right >> 8;
		block_buf[6] = this->cover.win_bottom;
		block_buf[7] = this->cover.win_bottom >> 8;

		/* F51_CUSTOM_CTRL05.01: Cover Left/Top/Right/Bottom */
		rc = clearpad_put_block(
				SYNF(this, F51_CUSTOM, CTRL,
					this->reg_offset.f51_ctrl05 +
					SYN_COVER_RECTANGLE_OFFSET),
				block_buf, 8);
		if (rc)
			goto end;
	}

	rc = clearpad_set_cover_status(this);
end:
	if (rc)
		LOGE(this, "failed to set cover window");
	return rc;
}

/*
 * Stamina (Report rate)
 */

/* need LOCK(&this->lock) */
static int clearpad_change_report_rate(struct clearpad_t *this)
{
	int rc = 0;
	bool enable = false, report_status = false;
	u8 buf;

	if (!this) {
		LOGE(this, "failed get this data\n");
		rc = -EINVAL;
		goto exit;
	}

	if (!this->stamina.change_reportrate.supported)
		goto exit;

	if (this->stamina.change_reportrate.mode > 0)
		enable = true;
	else
		enable = false;

	switch (this->chip_id) {
	case SYN_CHIP_3330:
	case SYN_CHIP_332U:
	case SYN_CHIP_4353:
		rc = clearpad_get(SYNF(this, F01_RMI, CTRL,
			this->reg_offset.f01_ctrl00), &buf);
		if (rc) {
			LOGE(this, "failed to get status\n");
			rc = -EINVAL;
			goto exit;
		}
		if (BIT_GET(buf, DEVICE_CONTROL_REPORT_RATE))
			report_status = true;

		if (enable == report_status) {
			LOGI(this, "report rate is already %s\n",
			this->stamina.enabled ? "enabled" : "disabled");
			goto exit;
		}

		rc = clearpad_put_bit(
				SYNF(this, F01_RMI, CTRL,
					this->reg_offset.f01_ctrl00),
				enable ? DEVICE_CONTROL_REPORT_RATE_MASK : 0,
				DEVICE_CONTROL_REPORT_RATE_MASK);
		if (rc) {
			LOGE(this, "failed to set report rate\n");
			goto exit;
		}
		LOGI(this, "report rate %s\n",
			enable ? "enable" : "disable");

		clearpad_set_delay(SYN_WAIT_TIME_AFTER_CHANGE_REPORTRATE);

		/* F54_ANALOG_CMD00: Analog Command */
		rc = clearpad_put(SYNF(this, F54_ANALOG, COMMAND,
					  this->reg_offset.f54_cmd00),
					  ANALOG_COMMAND_FORCE_UPDATE_MASK);
		if (rc) {
			LOGE(this, "failed to set force update\n");
			goto exit;
		}

		break;
	case SYN_CHIP_3500:
		/* F51_CUSTOM_CTRL30.06[1:0]: External Report Rate Selection */
		rc = clearpad_put(
				SYNF(this, F51_CUSTOM, CTRL,
				this->reg_offset.f51_ctrl30
				+ SYN_EXTERNAL_REPORT_RATE_OFFSET),
				this->stamina.change_reportrate.mode);
		if (rc) {
			LOGE(this, "failed to set change report rate\n");
			goto exit;
		}

		break;
	default:
		LOGE(this, "not supported on chip id 0x0%2x\n", this->chip_id);
		goto exit;
	}

exit:
	return rc;
}

/*
 * Stamina
 */

/* need LOCK(&this->lock) */
static int clearpad_set_stamina_mode(struct clearpad_t *this)
{
	int rc = 0;

	if (!this->stamina.supported)
		goto end;

	rc = clearpad_change_report_rate(this);
	if (rc)
		LOGE(this, "failed to change report rate\n");

end:
	return rc;
}

/*
 * Wakeup gesture (EW)
 */

/* need LOCK(&this->lock) */
static int clearpad_enable_wakeup_gesture(struct clearpad_t *this)
{
	u8 buf[F12_2D_CTRL_RPT_REG_MAX];
	int rc = 0;

	if (!clearpad_is_valid_function(this, SYN_F12_2D)) {
		LOGE(this, "F12 is required to set wakeup gesture\n");
		rc = -EPERM;
		goto end;
	}

	if (this->chip_id == SYN_CHIP_3330 &&
	    !clearpad_is_valid_function(this, SYN_F54_ANALOG)) {
		LOGE(this, "F54 is required to set wakeup gesture\n");
		rc = -EPERM;
		goto end;
	}

	/* F01_RMI_CTRL00: Device Control */
	rc = clearpad_put_bit(
		SYNF(this, F01_RMI, CTRL, this->reg_offset.f01_ctrl00),
		DEVICE_CONTROL_CONFIGURED_MASK,
		DEVICE_CONTROL_CONFIGURED_MASK);
	if (rc) {
		LOGE(this, "failed to set device configured bit\n");
		goto end;
	}
	clearpad_set_delay(SYN_WAIT_TIME_AFTER_REGISTER_ACCESS);

	if (this->chip_id == SYN_CHIP_3330 &&
	    this->wakeup_gesture.use_workaround_for_felica) {
		/* F12_2D_CTRL33_00: Multi-Finger Moisture General */
		rc = clearpad_get_block(SYNA(this, F12_2D, CTRL, 33),
				buf, 1);
		if (rc) {
			LOGE(this, "failed to read enable multifinger "
				   "moisture register\n");
			goto end;
		}
		BIT_SET(buf[0], ENABLE_MULTIFINGER_MOISTURE, 0);
		rc = clearpad_put_block(SYNA(this, F12_2D, CTRL, 33),
				buf, 1);
		if (rc) {
			LOGE(this, "failed to set enable multifinger "
				   "moisture bit\n");
			goto end;
		}

		/* F54_ANALOG_CTRL113_00: General Control */
		rc = clearpad_put_bit(SYNF(this, F54_ANALOG, CTRL,
				this->reg_offset.f54_ctrl113),
				      DISABLE_HYBRID_BASELINE_MASK,
				      DISABLE_HYBRID_BASELINE_MASK);
		if (rc) {
			LOGE(this, "failed to set disable hybrid baseline\n");
			goto end;
		}
		/* F54_ANALOG_CTRL109_00: General Control */
		rc = clearpad_put(SYNF(this, F54_ANALOG, CTRL,
				this->reg_offset.f54_ctrl109),
				       BASELINE_CORRECTION_MODE_MASK);
		if (rc) {
			LOGE(this, "failed to set baseline correction mode\n");
			goto end;
		}
		/* F54_ANALOG_CTRL147_00: Disable Hybrid CBC Auto Correction */
		rc = clearpad_put_bit(SYNF(this, F54_ANALOG, CTRL,
			this->reg_offset.f54_ctrl147),
				DISABLE_HYBRID_CBC_AUTO_CORRECTION_MASK,
				DISABLE_HYBRID_CBC_AUTO_CORRECTION_MASK);
		if (rc) {
			LOGE(this, "failed to set Disable Hybrid CBC Auto  "
				   "Correction\n");
			goto end;
		}
		/* F54_ANALOG_CTRL214_00: General Control */
		rc = clearpad_put_bit(SYNF(this, F54_ANALOG, CTRL,
			this->reg_offset.f54_ctrl214), 0,
				ENABLE_HYBRID_CHARGER_NOISE_MITIGATION_MASK);
		if (rc) {
			LOGE(this, "failed to set enable hybrid charger noise "
				   "mitigation\n");
			goto end;
		}
		/* F54_ANALOG_CMD00: Analog Command */
		rc = clearpad_put(
			SYNF(this, F54_ANALOG, COMMAND,
				this->reg_offset.f54_cmd00),
			ANALOG_COMMAND_FORCE_UPDATE_MASK);
		if (rc) {
			LOGE(this, "failed to set force update\n");
			goto end;
		}
		clearpad_set_delay(50);
	}
	/* F12_2D_CTRL20_01: Report Flags */
	rc = clearpad_get_block(SYNA(this, F12_2D, CTRL, 20),
				buf, F12_2D_CTRL_RPT_REG_MAX);
	if (rc) {
		LOGE(this, "failed to read control report register\n");
		goto end;
	}
	BIT_SET(buf[F12_2D_CTRL_RPT_FLAG],
		REPORT_FLAGS_REPORT_WAKEUP_GESTURE_ONLY, 1);
	rc = clearpad_put_block(SYNA(this, F12_2D, CTRL, 20),
				buf, F12_2D_CTRL_RPT_REG_MAX);
	if (rc) {
		LOGE(this, "failed to enable wakeup gesture\n");
		goto end;
	}

	/* F12_2D_CTRL27_00: Wakeup Gesture Enable */
	rc = clearpad_put(SYNA(this, F12_2D, CTRL, 27),
			  WAKEUP_GESTURE_ENABLE_DOUBLE_TAP_MASK);
	if (rc) {
		LOGE(this, "failed to enable double tap gesture\n");
		goto end;
	}
end:
	return rc;
}

/* need LOCK(&this->lock) */
static int clearpad_disable_wakeup_gesture(struct clearpad_t *this)
{
	u8 buf[F12_2D_CTRL_RPT_REG_MAX];
	int rc = 0;

	if (!clearpad_is_valid_function(this, SYN_F12_2D)) {
		LOGE(this, "F12 is required to set wakeup gesture\n");
		rc = -EPERM;
		goto end;
	}

	switch (this->chip_id) {
	case SYN_CHIP_3330:
	case SYN_CHIP_332U:
		LOGD(this, "already disabled wakeup gesture by HW reset\n");
		break;
	case SYN_CHIP_3500:
		/* F12_2D_CTRL27_00: Wakeup Gesture Enable */
		rc = clearpad_put(SYNA(this, F12_2D, CTRL, 27), 0);
		if (rc) {
			LOGE(this, "failed to disable double tap gesture\n");
			goto end;
		}

		/* F12_2D_CTRL20_01: Report Flags */
		rc = clearpad_get_block(SYNA(this, F12_2D, CTRL, 20),
					buf, F12_2D_CTRL_RPT_REG_MAX);
		if (rc) {
			LOGE(this, "failed to read control report register\n");
			goto end;
		}
		BIT_CLEAR(buf[F12_2D_CTRL_RPT_FLAG],
			  REPORT_FLAGS_REPORT_WAKEUP_GESTURE_ONLY);
		rc = clearpad_put_block(SYNA(this, F12_2D, CTRL, 20),
					buf, F12_2D_CTRL_RPT_REG_MAX);
		if (rc) {
			LOGE(this, "failed to disable report gesture\n");
			goto end;
		}

		/* F54_ANALOG_CMD00: Analog Command */
		rc = clearpad_put(
			SYNF(this, F54_ANALOG, COMMAND,
				this->reg_offset.f54_cmd00),
			ANALOG_COMMAND_FORCE_CALIBRATION_MASK);
		if (rc)
			LOGE(this, "failed to force calibrate\n");
		break;
	default:
		LOGE(this, "not supported on chip id 0x0%2x\n", this->chip_id);
		break;
	}
end:
	return rc;
}

static int clearpad_set_feature_settings(struct clearpad_t *this)
{
	int rc;

	rc = clearpad_set_charger(this);
	if (rc)
		goto end;
	rc = clearpad_set_pen(this);
	if (rc)
		goto end;
	rc = clearpad_set_glove_mode(this, this->glove.enabled);
	if (rc)
		goto end;
	if (this->cover.enabled)
		rc = clearpad_set_cover_window(this);
	if (rc)
		goto end;
	if (this->stamina.enabled)
		rc = clearpad_set_stamina_mode(this);
	if (rc)
		goto end;
	rc = clearpad_set_doze_holdoff(this);
end:
	return rc;
}

static int clearpad_gen_offsets(u8 desc, u8 offset_from, u8 reg_array[],
	int reg_from, int array_size)
{
	const u8 unused_offset = 0xFF;
	int offset = offset_from;
	int bit, reg = reg_from;

	for (bit = 0; bit < 8 && reg < array_size; bit++)
		reg_array[reg++] = desc & (1 << bit) ? offset++ : unused_offset;

	if (reg > SYN_MAX_CTRL_VALUE) {
		WARN_ON(reg > SYN_MAX_CTRL_VALUE);
		offset = -1;
	};

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
	u8 size_presence;
	int offset = 0;
	int size_num_bytes;
	int rc, i;

	rc = clearpad_get(SYNI(this, func, SYN_TYPE_QUERY, query),
			  &size_presence);
	if (rc)
		goto end;

	BUG_ON(size_presence > max_size_presence);

	/* The size of the control register structure is currently unused,
	 * but we need to find out how many bytes are allocated for it.
	 */
	rc = clearpad_get(SYNI(this, func, SYN_TYPE_QUERY, query + 1), buffer);
	if (rc)
		goto end;

	size_num_bytes = buffer[0] == 0 ? max_size_bytes : 1;

	rc = clearpad_get_block(SYNI(this, func, SYN_TYPE_QUERY, query + 1),
			buffer, size_num_bytes + size_presence);
	if (rc)
		goto end;

	for (i = 0; i < size_presence; i++) {
		offset = clearpad_gen_offsets(buffer[i + size_num_bytes],
			offset, reg_array, i * 8, reg_array_size);
		if (offset < 0) {
			HWLOGW(this, "register id is out of range\n");
			goto end;
		}
	}
end:
	return rc;
}

static void clearpad_log_offsets(struct clearpad_t *this, const char *header,
		enum clearpad_function_e func,
		u8 reg_array[], int reg_array_size)
{
	int i;

	LOGD(this, "%s(%d)\n", header, func);
	for (i = 0; i < reg_array_size; i++)
		LOGD(this, "[%.2d]=0x%02hX\n", i, (u16)reg_array[i]);
}

static int clearpad_init_reg_offsets(struct clearpad_t *this,
		enum clearpad_function_e func)
{
	const u8 query_query_register_presence = 0x01;
	const u8 query_ctrl_register_presence = 0x04;
	const u8 query_data_register_presence = 0x07;
	u8 query_desc;
	int rc;

	rc = clearpad_get(SYNI(this, func, SYN_TYPE_QUERY, 0x00), &query_desc);
	if (rc)
		goto end;

	if (!(BIT_GET(query_desc, GENERAL_HAS_REGISTER_DESCRIPTOR))) {
		LOGE(this, "register descriptors not supported!\n");
		rc = -EINVAL;
		goto end;
	}

	rc = clearpad_query_regs(this, func, query_query_register_presence,
		this->pdt[func].offset[SYN_TYPE_QUERY],
		ARRAY_SIZE(this->pdt[func].offset[SYN_TYPE_QUERY]));
	if (rc)
		goto end;

	clearpad_log_offsets(this, "query offsets", func,
		this->pdt[func].offset[SYN_TYPE_QUERY],
		ARRAY_SIZE(this->pdt[func].offset[SYN_TYPE_QUERY]));

	rc = clearpad_query_regs(this, func, query_ctrl_register_presence,
		this->pdt[func].offset[SYN_TYPE_CTRL],
		ARRAY_SIZE(this->pdt[func].offset[SYN_TYPE_CTRL]));
	if (rc)
		goto end;

	clearpad_log_offsets(this, "control offsets", func,
		this->pdt[func].offset[SYN_TYPE_CTRL],
		ARRAY_SIZE(this->pdt[func].offset[SYN_TYPE_CTRL]));

	rc = clearpad_query_regs(this, func, query_data_register_presence,
		this->pdt[func].offset[SYN_TYPE_DATA],
		ARRAY_SIZE(this->pdt[func].offset[SYN_TYPE_DATA]));

	clearpad_log_offsets(this, "data offsets", func,
		this->pdt[func].offset[SYN_TYPE_DATA],
		ARRAY_SIZE(this->pdt[func].offset[SYN_TYPE_DATA]));
end:
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
		goto end;

	/* F12_2D_CTRL08_[00-03]: Maximum XY Coordinate */
	rc = clearpad_get_block(SYNA(this, F12_2D, CTRL, 8), buf, sizeof(buf));
	if (rc)
		goto end;
	this->extents.x_max = (buf[REG_X_LSB] | (buf[REG_X_MSB] << 8));
	this->extents.y_max = (buf[REG_Y_LSB] | (buf[REG_Y_MSB] << 8));

	WARN_ON(this->extents.preset_x_max != this->extents.x_max);
	WARN_ON(this->extents.preset_y_max != this->extents.y_max);

	/* F12_2D_CTRL23_01: Max Number Of Reported Objects */
	rc = clearpad_get_block(SYNA(this, F12_2D, CTRL, 23), buf, 2);
	if (rc)
		goto end;
	this->extents.n_fingers = buf[F12_2D_CTRL_MAX_OBJ_REPORT];

	/* F12_2D_CTRL28: Data Reporting Enable Mask */
	rc = clearpad_get(SYNA(this, F12_2D, CTRL, 28), buf);
	if (rc)
		goto end;

	this->extents.n_bytes_per_object = 0;

	for (i = 0; i < BITS_PER_BYTE; i++)
		this->extents.n_bytes_per_object += (buf[0] & BIT(i)) >> i;

	LOGI(this, "x_max=%d, y_max=%d, n_fingers=%d, n_bytes_per_object=%d\n",
		this->extents.x_max, this->extents.y_max,
		this->extents.n_fingers,
		this->extents.n_bytes_per_object);

	rc = clearpad_set_feature_settings(this);

end:
	return rc;
}

static void clearpad_firmware_reset(struct clearpad_t *this)
{
	this->flash.data.pos = 0;
	this->flash.config.pos = 0;
	if (this->flash.fw) {
		release_firmware(this->flash.fw);
		this->flash.fw = NULL;
	}
	HWLOGI(this, "firmware image has been reset\n");
}

static bool clearpad_status_check(u8 device_status)
{
	bool status_error = false;

	if ((BIT_GET(device_status, DEVICE_STATUS_CODE) ==
		DEVICE_STATUS_CODE_INVALID_PARTITION_TABLE_V2) ||
	    (BIT_GET(device_status, DEVICE_STATUS_CODE) ==
		DEVICE_STATUS_CODE_CHEACKSUM_PROGRESS_V2) ||
	    (BIT_GET(device_status, DEVICE_STATUS_CODE) ==
		DEVICE_STATUS_CODE_FLASHCONFIG_CHEACKSUM_FAILD_V2) ||
	    (BIT_GET(device_status, DEVICE_STATUS_CODE) ==
		DEVICE_STATUS_CODE_CORECONFIG_CHEACKSUM_FAILD) ||
	    (BIT_GET(device_status, DEVICE_STATUS_CODE) ==
		DEVICE_STATUS_CODE_COREFIRMWARE_CHEACKSUM_FAILD) ||
	    (BIT_GET(device_status, DEVICE_STATUS_CODE) ==
		DEVICE_STATUS_CODE_GUESTCODE_CHEACKSUM_FAILD) ||
	    (BIT_GET(device_status, DEVICE_STATUS_CODE) ==
		DEVICE_STATUS_CODE_DISPCINFIG_CHEACKSUM_FAILD) ||
	    (BIT_GET(device_status, DEVICE_STATUS_CODE) ==
		DEVICE_STATUS_CODE_DEVICECINFIG_CHEACKSUM_FAILD)) {
		status_error = true;
	}
	return status_error;
}

static int clearpad_initialize(struct clearpad_t *this)
{
	int rc;
	u8 fw_info[SYN_DEVICE_INFO_SIZE], bl_ver[SYN_DEVICE_BL_MAX_SIZE];
	u8 device_status, fw_status;
	struct clearpad_device_info_t *info = &this->device_info;
	u8 product_id[HEADER_PRODUCT_ID_SIZE];

	LOGI(this, "initialize device\n");

	/* read device product id */
	/* F01_RMI_QUERY11: Product ID Query */
	rc = clearpad_get_block(SYNF(this, F01_RMI, QUERY,
		this->reg_offset.f01_query11),
		product_id, HEADER_PRODUCT_ID_SIZE);
	if (rc)
		goto end;
	memcpy(info->product_id, product_id, HEADER_PRODUCT_ID_SIZE);

	clearpad_update_chip_id(this);

	/* read device status */
	/* F01_RMI_DATA00: Device Status */
	rc = clearpad_get(SYNF(this, F01_RMI, DATA,
		this->reg_offset.f01_data00),
		&device_status);
	if (rc)
		goto end;

	LOGI(this, "device status 0x%02x\n", device_status);

	switch (this->chip_id) {
	case SYN_CHIP_4353:
		if ((device_status == SYN_STATUS_GET_ERROR) ||
			clearpad_status_check(device_status)) {
			LOGE(this, "initialize status error reset\n");
			clearpad_reset(this, SYN_SWRESET, __func__);
		}
		/* Intentional no break */
	case SYN_CHIP_3500:
		/* F34_FLASH_QUERY00: Bootloader Revision */
		rc = clearpad_get_block(SYNF(this, F34_FLASH, QUERY,
				this->reg_offset.f34_query00),
				bl_ver, SYN_DEVICE_BL60_READ_SIZE);
		if (rc)
			goto end;

		/* Flash memory management, Version 1         */
		/* Bootloader Revision is stored as character */
		LOGI(this, "bl[2]:%c\n", bl_ver[2]);
		LOGI(this, "bl[3]:%c\n", bl_ver[3]);

		/* change character to decimal */
		bl_ver[0] = bl_ver[2] - 0x30;
		bl_ver[1] = bl_ver[3] - 0x30;
		LOGI(this, "bootloader revision %d.%d\n", bl_ver[1], bl_ver[0]);
		if (bl_ver[0] == BV6_8)
			info->bootloader_version = BV6_8;
		else
			info->bootloader_version = BV6;
		break;
	case SYN_CHIP_332U:
	case SYN_CHIP_3330:
		/* F34_FLASH_QUERY01: Bootloader Revision */
		rc = clearpad_get_block(SYNF(this, F34_FLASH, QUERY,
					this->reg_offset.f34_query01),
					bl_ver, SYN_DEVICE_BL70_READ_SIZE);
		if (rc)
			goto end;

		/* Flash memory management, Version 2       */
		/* Bootloader Revision is stored as decimal */
		LOGI(this, "bootloader revision %d.%03d\n", bl_ver[1], bl_ver[0]);
		info->bootloader_version = BV7;
		break;
	default:
		LOGE(this, "invalid chip id :0x%02x\n", this->chip_id);
		goto end;
	}

	if (((info->bootloader_version != BV6_8) &&
		 ((BIT_GET(device_status, DEVICE_STATUS_CODE) ==
		  DEVICE_STATUS_CODE_CONFIGURATION_CRC_FAILURE) ||
		 (BIT_GET(device_status, DEVICE_STATUS_CODE) ==
		  DEVICE_STATUS_CODE_FIRMWARE_CRC_FAILURE) ||
		 (BIT_GET(device_status, DEVICE_STATUS_CODE) ==
		  DEVICE_STATUS_CODE_CRC_IN_PROGRESS))) ||
		((info->bootloader_version == BV6_8) &&
		 (clearpad_status_check(device_status)))) {
		LOGE(this, "error device status force recovery \n");
		/* force firmware recovery */
		memset(fw_info, 0, sizeof(fw_info));
	} else {
		if (bl_ver[1] >= clearpad_bootloader_version_dec[BV7]) {
			/* F34_FLASH_DATA00: Status */
			rc = clearpad_get(SYNF(this, F34_FLASH, DATA,
					this->reg_offset.f34_data00),
				&fw_status);
			if (rc)
				goto end;

			LOGI(this, "FW status 0x%02x\n", fw_status);

			switch (BIT_GET(fw_status, STATUS_FLASH_STATUS)) {
			case STATUS_FLASH_STATUS_BAD_PARTITION_TABLE:
			case STATUS_FLASH_STATUS_CHECKSUM_FAILED:
				/* force firmware recovery */
				memset(fw_info, 0, sizeof(fw_info));
				break;
			default:
				break;
			}
			rc = clearpad_get_block(
				SYNF(this, F34_FLASH, CTRL,
				this->reg_offset.f34_ctrl00), fw_info,
				FLASH_READ_FIRMWARE_INFO_SIZE_BL7X);
			if (rc)
				goto end;

			info->customer_family = fw_info[0];
			info->firmware_revision_major = fw_info[1];
			info->firmware_revision_minor = fw_info[2];
			info->firmware_revision_extra = fw_info[3];
			info->analog_id = fw_info[4];
			info->boot_loader_version_minor = bl_ver[0];
			info->boot_loader_version_major = bl_ver[1];
		} else if (bl_ver[1] >= clearpad_bootloader_version_dec[BV6]) {
			rc = clearpad_get_block(SYNF(this, F34_FLASH, CTRL,
				this->reg_offset.f34_ctrl00),
				fw_info, FLASH_READ_FIRMWARE_INFO_SIZE_BL6X);
			if (rc)
				goto end;

			info->customer_family = fw_info[0];
			info->firmware_revision_major = fw_info[1];
			info->firmware_revision_minor = fw_info[2];
			info->firmware_revision_extra = fw_info[3];
			info->boot_loader_version_minor = bl_ver[0];
			info->boot_loader_version_major = bl_ver[1];
		}
	}

	/* overwrite default settings with actual chip settings */
	clearpad_touch_config_dt_for_chip_id(this, this->chip_id);

	if (this->state != SYN_STATE_RUNNING) {
		LOGI(this, "device mid %d, prop %d, family 0x%02x, "
		     "rev 0x%02x.%02x, extra 0x%02x, aid 0x%02x\n",
		     info->manufacturer_id, info->product_properties,
		     info->customer_family, info->firmware_revision_major,
		     info->firmware_revision_minor,
		     info->firmware_revision_extra,
		     info->analog_id);
		LOGI(this, "product id '%s'\n",
		     clearpad_s(info->product_id, HEADER_PRODUCT_ID_SIZE));
	}

	if (clearpad_is_valid_function(this, SYN_F12_2D)) {
		rc = clearpad_prepare_f12_2d(this);
		if (rc)
			goto end;
	}

	/* set device configured bit */
	/* F01_RMI_CTRL00: Device Control */
	rc = clearpad_put_bit(SYNF(this, F01_RMI, CTRL,
		this->reg_offset.f01_ctrl00),
			      DEVICE_CONTROL_CONFIGURED_MASK,
			      DEVICE_CONTROL_CONFIGURED_MASK);
	if (rc)
		goto end;

	clearpad_set_delay(SYN_WAIT_TIME_AFTER_REGISTER_ACCESS);

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
	LOGI(this, "result: %s", this->result_info);
end:
	/* inform running state */
	this->state = SYN_STATE_RUNNING;
	return rc;
}

/*
 * @return 1 : initialized
 *         0 : not initialized since interrupt.count != 0
 *         negative value : error
 * need LOCK(&this->lock)
 */
static int clearpad_initialize_if_first_event(struct clearpad_t *this,
					      u8 *interrupt_status,
					      u8 *device_status)
{
	struct timespec ts;
	int rc = 0;
	int retry;

	if (this->interrupt.count != 0)
		goto read_interrupt;

	get_monotonic_boottime(&ts);
	HWLOGI(this, "read first event (power=%s active=%s) @ %ld.%06ld\n",
	       touchctrl_is_touch_powered(this) ? "OK" : "NG",
	       this->dev_active ? "true" : "false",
	       ts.tv_sec, ts.tv_nsec);

	for (retry = 0; retry < SYN_RETRY_NUM; retry++) {
		rc = clearpad_set_page(this, 0);
		if (!rc)
			goto read_pdt;
		if (retry < SYN_RETRY_NUM - 1)
			clearpad_set_delay(
				this->reset.delay_for_powerup_ms);
	}
	HWLOGE(this, "failed to set page 0\n");
	goto err_in_set_page;

read_pdt:
	rc = clearpad_read_pdt(this);
	if (rc) {
		HWLOGE(this, "failed to read pdt\n");
		goto err_in_read_pdt;
	}

read_interrupt:
	/* F01_RMI_DATA01: Interrupt Status */
	rc = clearpad_get(SYNF(this, F01_RMI, DATA,
		this->reg_offset.f01_data01), interrupt_status);
	if (rc) {
		HWLOGE(this, "failed to read interrupt status\n");
		goto end;
	}
	/* F01_RMI_DATA00: Device Status */
	rc = clearpad_get(SYNF(this, F01_RMI, DATA,
		this->reg_offset.f01_data00), device_status);
	if (rc) {
		HWLOGE(this, "failed to read device status\n");
		goto end;
	}
	if (this->interrupt.count != 0)
		goto end;

	rc = clearpad_initialize(this);
	if (rc) {
		HWLOGE(this, "failed to initialize (rc=%d)\n", rc);
		goto end;
	}
	rc = 1;
end:
	return rc;

err_in_set_page:
err_in_read_pdt:
	/* this workaround will be replaced by HW reset */
	HWLOGE(this, "retry reading interrupt status as workaround\n");
	/* F01_RMI_DATA01: Interrupt Status */
	rc = clearpad_get(SYNF(this, F01_RMI, DATA,
		this->reg_offset.f01_data01), interrupt_status);
	if (rc)
		HWLOGE(this, "failed to read interrupt status\n");
	return -EIO;
}

/*
 * @return 1 : initialized
 *         0 : not initialized since interrupt.count != 0
 *         negative value : error
 * need LOCK(&this->lock)
 */
static int clearpad_handle_if_first_event(struct clearpad_t *this)
{
	int rc = 0;

	if (this->interrupt.count != 0)
		goto end;

	get_monotonic_boottime(&this->interrupt.handle_first_event_ts);

	HWLOGI(this, "first event (power=%s active=%s) @ %ld.%06ld\n",
	       touchctrl_is_touch_powered(this) ? "OK" : "NG",
	       this->dev_active ? "true" : "false",
	       this->interrupt.handle_first_event_ts.tv_sec,
	       this->interrupt.handle_first_event_ts.tv_nsec);

	rc = clearpad_process_irq(this);
	if (rc)
		HWLOGE(this, "failed to read interrupt, "
		       "but ignore (rc=%d)\n", rc);

	if (!this->post_probe.done)
		goto end;

	if (this->force_sleep == FSMODE_ONESHOT) {
		LOGI(this, "clear force sleep mode\n");
		this->force_sleep = FSMODE_OFF;
	}
	if (this->force_sleep != FSMODE_OFF) {
		HWLOGI(this, "force sleep mode\n");
		if (this->dev_active) {
			rc = clearpad_set_suspend_mode(this);
			if (rc)
				HWLOGE(this, "failed to force sleep mode\n");
		}
	} else if (!this->dev_active) {
		rc = clearpad_set_resume_mode(this);
		if (rc)
			HWLOGE(this, "failed to resume (rc=%d)\n", rc);
	}
	rc = rc < 0 ? rc : 1;
end:
	return rc;
}

void clearpad_parse_descriptor(struct clearpad_t *this, u32 container_addr)
{
	struct descriptor_t desc;
	struct clearpad_flash_t *f = &this->flash;
	const u8 *data = this->flash.fw->data;
	u8 *cc_content_addr;

	desc.container_id = (u16)le32_to_cpu(*(u32 *)
				(data + container_addr + CON_ID_OFFSET));
	LOGD(this, "container_addr=%d", container_addr);
	desc.content_len = le32_to_cpu(*(u32 *)(data
				+ container_addr + CON_CONTENT_LENGTH_OFFSET));
	desc.content_start_addr = le32_to_cpu(*(u32 *)
			(data + container_addr + CON_CONTENT_ADDRESS_OFFSET));
	LOGD(this,
	"container_id=%d, desc.content_len=%d, desc.content_start_addr=%d\n",
	 desc.container_id, desc.content_len, desc.content_start_addr);

	cc_content_addr = (u8 *)(data + desc.content_start_addr);
	/* Set up block info */
	switch (desc.container_id) {
	case CID_CORE_CODE_CONTAINER:
		f->data.length = desc.content_len;
		f->data.data = cc_content_addr;
		HWLOGD(this, "core code: data (length %u)\n", f->data.length);
		break;
	case CID_CORE_CONFIGURATION_CONTAINER:
		f->config.length = desc.content_len;
		f->config.data = cc_content_addr;
		f->customer_family = *cc_content_addr
		    + CONFIG_CUSTOMER_FAMILY_OFFSET;
		f->firmware_revision_major =
		    *(cc_content_addr + CONFIG_FIRMWARE_REVISION_MAJOR_OFFSET);
		f->firmware_revision_minor =
		    *(cc_content_addr + CONFIG_FIRMWARE_REVISION_MINOR_OFFSET);
		f->firmware_revision_extra =
		    *(cc_content_addr + CONFIG_FIRMWARE_REVISION_EXTRA_OFFSET);
		HWLOGD(this,
		      "core config: family 0x%02x, rev 0x%02x.%02x.0x%02x\n",
		      f->customer_family, f->firmware_revision_major,
		      f->firmware_revision_minor, f->firmware_revision_extra);
		break;
	default:
		HWLOGD(this, "core(container id %d)\n", desc.container_id);
		break;
	}
}

void clearpad_parse_container(struct clearpad_t *this)
{
	int i, j = 0;
	u8 cont_count;
	u32 offset, top_container_addr, container_addr, cont_len;
	const u8 *data = this->flash.fw->data;

	HWLOGI(this, "start parse\n");
	offset = le32_to_cpu(*(u32 *)(data + CON_TOP_START_ADDRESS_OFFSET));
	LOGD(this, "offset=%d\n", offset);

	cont_len = le32_to_cpu(*(u32 *)(data + offset +
					CON_CONTENT_LENGTH_OFFSET));
	cont_count = cont_len / CON_START_ADDRESS_SIZE;
	top_container_addr = le32_to_cpu(*(u32 *)(data + offset +
						CON_CONTENT_ADDRESS_OFFSET));
	LOGD(this, "cont_len=%d, top_container_addr=%d\n",
		 cont_len, top_container_addr);

	for (i = 0; i < cont_count; i++) {
		container_addr =
			le32_to_cpu(*(u32 *)(data + top_container_addr + j));
		j += CON_START_ADDRESS_SIZE;
		LOGD(this, "container_addr=%d, i=%d, j=%d\n",
				container_addr, i, j);
		clearpad_parse_descriptor(this, container_addr);
	}
}

static void clearpad_parse_firmware(struct clearpad_t *this)
{
	const u8 *data;
	struct clearpad_flash_t *f = &this->flash;
	struct clearpad_device_info_t *info = &this->device_info;
	u32 blsize = 0;
	u32 offset_dispconfig = 0;
	u32 block_size = BLOCK_SIZE_16_BYTE;

	data = this->flash.fw->data;
	LOGI(this, "data:%p\n", data);

	if (info->bootloader_version == BV6_8) {
		blsize = le32_to_cpu(*(u32 *)(data + HEADER_BL_LENGTH));
		offset_dispconfig = le32_to_cpu(*(u32 *)(data + HEADER_DISP_OFFSET));
		block_size = BLOCK_SIZE_256_BYTE;

		/* Set up display block info */
		f->disp.length = DISP_CONFIG_LENGTH;
		f->disp.blocks = DISP_CONFIG_BLOCK;
		f->disp.data = data + offset_dispconfig;
		f->disp.pos = 0;
		LOGI(this, "DISP: length=%x blocks=%x data=%p\n",
			f->disp.length, f->disp.blocks, f->disp.data);
	}

	/* Set up data block info */
	f->data.length = le32_to_cpu(*(u32 *)(data + HEADER_DATA_LENGTH));
	f->data.blocks = (f->data.length / block_size) + !!(f->data.length % block_size);
	f->data.data = data + HEADER_SIZE + blsize;;
	f->data.pos = 0;
	LOGI(this, "DATA: length=%x blocks=%x data=%p\n",
		 f->data.length, f->data.blocks, f->data.data);

	/* Set up configuration block info */
	f->config.length = le32_to_cpu(*(u32 *)(data + HEADER_CONFIG_LENGTH));
	f->config.blocks = (f->config.length / block_size) + !!(f->config.length % block_size);
	f->config.data = data + HEADER_SIZE + f->data.length + blsize;;
	f->config.pos = 0;
	LOGI(this, "CONFIG: length=%x blocks=%x data=%p\n",
		 f->config.length, f->config.blocks, f->config.data);

	/* family Rev major minor extra info */
	f->customer_family =
		*(data + f->data.length + HEADER_SIZE + CONFIG_CUSTOMER_FAMILY_OFFSET + blsize);
	f->firmware_revision_major =
		*(data + f->data.length + HEADER_SIZE + CONFIG_FIRMWARE_REVISION_MAJOR_OFFSET + blsize);
	f->firmware_revision_minor =
		*(data + f->data.length + HEADER_SIZE + CONFIG_FIRMWARE_REVISION_MINOR_OFFSET + blsize);
	f->firmware_revision_extra =
		*(data + f->data.length + HEADER_SIZE + CONFIG_FIRMWARE_REVISION_EXTRA_OFFSET + blsize);
	LOGI(this,
		"core config: family 0x%02x, rev 0x%02x.%02x.0x%02x\n",
		f->customer_family, f->firmware_revision_major,
		f->firmware_revision_minor, f->firmware_revision_extra);

}

static ssize_t clearpad_get_firmware(struct clearpad_t *this, char *filename)
{
	int rc;
	struct clearpad_flash_t *f = &this->flash;

	rc = request_firmware(&f->fw, filename, this->bdata->dev);
	if (rc || !f->fw) {
		HWLOGE(this, "fw request failed (rc=%d, fw=%p, name=%s)\n",
		       rc, f->fw, filename);
		rc = -EINVAL;
		goto end;
	}

	f->format_version = f->fw->data[HEADER_VERSION_OFFSET];
	if (f->format_version >= BV7) {
		clearpad_parse_container(this);
	} else if (f->format_version >= BV6) {
		clearpad_parse_firmware(this);
	} else {
		HWLOGE(this, "bootloader version not supported\n");
		rc = -ENODEV;
		goto end;
	}

	HWLOGD(this, "DATA: length=%d blocks=%d\n",
	       f->data.length, f->data.blocks);
	HWLOGD(this, "CONFIG: length=%d blocks=%d\n",
	       f->config.length, f->config.blocks);
	HWLOGI(this, "firmware image size=%zu\n", this->flash.fw->size);
end:
	return rc;
}

static int clearpad_judge_firmware_flash(struct clearpad_t *this,
		unsigned int id, enum clearpad_flash_command_e flash_cmd)
{
	unsigned int family_id;
	int rc = 0;
	char filename[SYN_STRING_LENGTH] = "";

	if (!this->device_info.customer_family)
		HWLOGI(this, "device is broken\n");
	else
		HWLOGI(this, "device is normal\n");

	if (flash_cmd != SYN_FORCE_FLASH) {
		rc = clearpad_read_lockdown_area(this);
		if (rc) {
			HWLOGE(this, "failed to get lockdown area\n");
			goto end;
		}
		family_id = clearpad_get_lockdown_family_id(this);
		if (family_id > LOCKDOWN_FAMILY_ID_MIN &&
		    family_id < LOCKDOWN_FAMILY_ID_MAX) {
			id = family_id;
			HWLOGI(this, "lockdown value is valid\n");
		} else if (this->device_info.customer_family > 0) {
			id = this->device_info.customer_family;
			HWLOGI(this,
				"use device id since lockdown was invalid\n");
		} else {
			HWLOGE(this, "failed to get valid id\n");
		}
	}

	snprintf(filename, SYN_STRING_LENGTH, this->flash.firmware_name, id);
	rc = clearpad_get_firmware(this, filename);
	if (rc) {
		HWLOGE(this, "failed to get firmware (module id 0x%02x)\n", id);
		goto end;
	}

	HWLOGI(this, "module id (device = 0x%02x, fw file = 0x%02x)\n",
		      id, this->flash.customer_family);
	if (id != this->flash.customer_family) {
		HWLOGW(this, "stop flashing, id is not matched with fw file\n");
		goto end;
	}

	HWLOGI(this, "old firmware 0x%02x revision 0x%02x.%02x\n",
		this->device_info.customer_family,
		this->device_info.firmware_revision_major,
		this->device_info.firmware_revision_minor);
	HWLOGI(this, "new firmware 0x%02x revision 0x%02x.%02x\n",
		this->flash.customer_family,
		this->flash.firmware_revision_major,
		this->flash.firmware_revision_minor);

	if (flash_cmd == SYN_FORCE_FLASH ||
	    (this->device_info.customer_family !=
	     this->flash.customer_family) ||
	    (this->device_info.firmware_revision_major !=
	     this->flash.firmware_revision_major) ||
	    (this->device_info.firmware_revision_minor !=
	     this->flash.firmware_revision_minor)) {
		HWLOGI(this, "update of firmware is necessary\n");
		rc = 1;
	} else {
		HWLOGI(this, "update of firmware is unnecessary\n");
	}
end:
	return rc;
}

/* for Bootloader v6.x */
static int clearpad_flash_enable_bl_v6_x(struct clearpad_t *this)
{
	int rc;
	u8 buf[SYN_DEVICE_BL_INFO_SIZE];

	/* read bootloader id */
	rc = clearpad_get_block(SYNF(this, F34_FLASH, QUERY,
			this->reg_offset.f34_query00),
			buf, sizeof(buf));
	LOG_CHECK(this, "rc=%d\n", rc);
	if (rc)
		goto end;

	/* write bootloader id to block data */
	rc = clearpad_put_block(SYNF(this, F34_FLASH, DATA,
				this->reg_offset.f34_data01),
				buf, sizeof(buf));
	LOG_CHECK(this, "rc=%d\n", rc);
	if (rc)
		goto end;

	clearpad_set_delay(10);

	/* issue a flash program enable */
	this->flash.enter_bootloader_mode = true;
	rc = clearpad_put(SYNF(this, F34_FLASH, DATA,
			this->reg_offset.f34_data02),
			FLASH_CONTROL_ENABLE_FLASH_PROGRAMMING);
	LOG_CHECK(this, "rc=%d\n", rc);
	if (rc)
		goto end;

	this->state = SYN_STATE_FLASH_ENABLE;
	clearpad_set_delay(100);

	clearpad_set_irq(this, true);

end:
	return rc;
}

static int clearpad_flash_program_bl_v6_x(struct clearpad_t *this)
{
	struct clearpad_flash_t *flash = &this->flash;
	struct clearpad_device_info_t *info = &this->device_info;
	int rc;
	u8 buf[SYN_DEVICE_BL_INFO_SIZE];
	u8 cmd;

	/* make sure that we are in programming mode and there are no issues */
	rc = clearpad_get(SYNF(this, F34_FLASH, DATA,
		this->reg_offset.f34_data03), buf);
	if (rc) {
		HWLOGE(this, "failed to get flash programming status\n");
		goto end;
	}
	if (!(BIT_GET(buf[0], STATUS_BL_MODE))) {
		HWLOGE(this, "failed enabling flash (%s)\n",
			      NAME_OF(clearpad_flash_status_name,
				      BIT_GET(buf[0], STATUS_FLASH_STATUS)));
		rc = -EIO;
		goto end;
	}

	HWLOGI(this, "flashing enabled\n");

	if (this->state == SYN_STATE_FLASH_ENABLE) {
		/* PDT may have changed, re-read */
		rc = clearpad_read_pdt(this);
		if (rc) {
			HWLOGE(this, "failed to read pdt\n");
			goto end;
		}
	}

	/* read bootloader id */
	rc = clearpad_get_block(SYNF(this, F34_FLASH, QUERY,
			this->reg_offset.f34_query00), buf, sizeof(buf));
	if (rc)
		goto end;

	/* write bootloader id to block data */
	rc = clearpad_put_block(SYNF(this, F34_FLASH, DATA,
		this->reg_offset.f34_data01), buf, sizeof(buf));
	if (rc)
		goto end;

	if (this->state == SYN_STATE_FLASH_ENABLE &&
	    (flash->command == SYN_FORCE_FLASH ||
	     flash->command == SYN_DEFAULT_FLASH)) {
		rc = clearpad_put(SYNF(this, F34_FLASH, DATA,
				this->reg_offset.f34_data02),
				FLASH_CONTROL_ERASE_ALL);
		if (rc)
			goto end;
		this->state = SYN_STATE_FLASH_PROGRAM;
	} else if ((this->state == SYN_STATE_FLASH_PROGRAM &&
		    (flash->command == SYN_FORCE_FLASH ||
		     flash->command == SYN_DEFAULT_FLASH))) {
		if (info->bootloader_version == BV6_8)
			cmd = FLASH_CONTROL_ERASE_DISPLAY_BLOCK;
		else
			cmd = FLASH_CONTROL_ERASE_CONFIGURATION;
		rc = clearpad_put(SYNF(this, F34_FLASH, DATA,
				this->reg_offset.f34_data02), cmd);
		if (rc)
			goto end;
		this->state = SYN_STATE_FLASH_ERASE;
	} else {
		HWLOGE(this, "invalid state(%s) for command(%s)\n",
			NAME_OF(clearpad_state_name, this->state),
			NAME_OF(clearpad_flash_command_name, flash->command));
		rc = -EINVAL;
		goto end;
	}

	HWLOGI(this, "firmware erasing\n");
end:
	return rc;
}

static int clearpad_flash_update_bl_v6_x(struct clearpad_t *this)
{
	struct clearpad_flash_t *flash = &this->flash;
	struct clearpad_device_info_t *info = &this->device_info;
	int rc, len;
	const u8 *data;
	u8 buf, pid;
	struct clearpad_flash_block_t *object;
	enum clearpad_state_e finish_state;
	u8 pos[2];
	u32 block_size = BLOCK_SIZE_16_BYTE;

	if (this->state == SYN_STATE_FLASH_ERASE &&
	    (flash->command == SYN_FORCE_FLASH ||
	     flash->command == SYN_DEFAULT_FLASH)) {
		object = &(&this->flash)->data;
		pid = PID_CORE_CODE;
		finish_state = SYN_STATE_FLASH_DATA;
	} else if ((this->state == SYN_STATE_FLASH_DATA &&
		    (flash->command == SYN_FORCE_FLASH ||
		     flash->command == SYN_DEFAULT_FLASH))) {
		object = &(&this->flash)->config;
		pid = PID_CORE_CONFIGURATION;
		if (info->bootloader_version == BV6_8)
			finish_state = SYN_STATE_FLASH_DISP;
		else
			finish_state = SYN_STATE_FLASH_CONFIG;
	} else if ((this->state == SYN_STATE_FLASH_DISP &&
		    (flash->command == SYN_FORCE_FLASH ||
		     flash->command == SYN_DEFAULT_FLASH))) {
		object = &(&this->flash)->disp;
		pid = PID_CORE_CONFIGURATION;
		finish_state = SYN_STATE_FLASH_CONFIG;
	} else {
		HWLOGE(this, "invalid state(%s) for command(%s)\n",
			NAME_OF(clearpad_state_name, this->state),
			NAME_OF(clearpad_flash_command_name, flash->command));
		rc = -EINVAL;
		goto end;
	}

	if (info->bootloader_version == BV6_8) {
		if (object->pos > 0 && pid == PID_CORE_CODE)
			goto write_block_data;
		else
			goto write_block_set;
	} else {
		if (object->pos > 0)
			goto write_block_data;
	}

	/* make sure that we are in programming mode and there are no issues */
	rc = clearpad_get(SYNF(this, F34_FLASH, DATA,
		this->reg_offset.f34_data03), &buf);
	if (rc) {
		HWLOGE(this, "failed to get flash programming status\n");
		goto end;
	}

	if (!(BIT_GET(buf, STATUS_BL_MODE))) {
		HWLOGE(this, "failed flashing in %s (%s)\n",
		     NAME_OF(clearpad_state_name, this->state),
		     NAME_OF(clearpad_flash_status_name,
			     BIT_GET(buf, STATUS_FLASH_STATUS)));
		rc = -EIO;
		goto end;
	}

	object->payload_length = SYN_PAYLOAD_LENGTH;

write_block_set:
	/* block # low and high byte */
	pos[0] = object->pos & 0xff;
	pos[1] = (object->pos >> 8) & 0xff;
	if (this->state == SYN_STATE_FLASH_DISP) {
		pos[1] &= 0x1f;
		pos[1] |= (unsigned char)(0x03 << 5);
	}
	rc = clearpad_put_block(SYNF(this, F34_FLASH, DATA,
			this->reg_offset.f34_data00), pos, 2);
	if (rc)
		goto end;

write_block_data:
	if (info->bootloader_version == BV6_8)
		block_size = BLOCK_SIZE_256_BYTE;
	data = object->data + object->pos * block_size;
	len = object->length - object->pos * block_size;
	if (len > block_size)
		len = block_size;

	/* write block data */
	rc = clearpad_put_block(SYNF(this, F34_FLASH, DATA,
		this->reg_offset.f34_data01), data, len);
	if (rc)
		goto end;
	/* issue a write data block command */
	if (pid == PID_CORE_CODE)
		rc = clearpad_put(SYNF(this, F34_FLASH, DATA,
				this->reg_offset.f34_data02),
				FLASH_CONTROL_WRITE_FIRMWARE_BLOCK);
	else
		rc = clearpad_put(SYNF(this, F34_FLASH, DATA,
				this->reg_offset.f34_data02),
				FLASH_CONTROL_WRITE_CONFIGURATION_BLOCK);
	if (rc)
		goto end;

	if (object->pos % 100 == 0)
		LOGI(this, "wrote %d blocks\n", object->pos);

	/* if we've reached the end of the data/configuration flashing */
	if (++object->pos == object->blocks) {
		HWLOGI(this, "flash finished on %s\n",
			      NAME_OF(clearpad_state_name, this->state));
		this->state = finish_state;
	}
end:
	return rc;
}

static int clearpad_flash_disable_bl_v6_x(struct clearpad_t *this)
{
	int rc;
	u8 buf;

	/* make sure that we are in programming mode and there are no issues */
	rc = clearpad_get(SYNF(this, F34_FLASH, DATA,
		this->reg_offset.f34_data03), &buf);
	if (rc)
		goto end;

	if (!(BIT_GET(buf, STATUS_BL_MODE))) {
		HWLOGE(this, "failed flashing config (%s)\n",
			      NAME_OF(clearpad_flash_status_name,
				      BIT_GET(buf, STATUS_FLASH_STATUS)));
		rc = -EIO;
		goto end;
	}

	clearpad_set_delay(SYN_WAIT_TIME_AFTER_REGISTER_ACCESS);

	/* send a reset to the device to complete the flash procedure */
	clearpad_reset(this, SYN_SWRESET, __func__);

	HWLOGI(this, "flashing finished, resetting\n");
	this->state = SYN_STATE_FLASH_DISABLE;
end:
	return rc;
}

/* need LOCK(&this->lock) */
static int clearpad_flash_bl_v6_x(struct clearpad_t *this)
{
	int rc = 0;

	switch (this->state) {
	case SYN_STATE_FLASH_IMAGE_SET:
		rc = clearpad_flash_enable_bl_v6_x(this);
		break;
	case SYN_STATE_FLASH_ENABLE:
	case SYN_STATE_FLASH_PROGRAM:
		rc = clearpad_flash_program_bl_v6_x(this);
		break;
	case SYN_STATE_FLASH_ERASE:
	case SYN_STATE_FLASH_DATA:
	case SYN_STATE_FLASH_DISP:
		rc = clearpad_flash_update_bl_v6_x(this);
		break;
	case SYN_STATE_FLASH_CONFIG:
		rc = clearpad_flash_disable_bl_v6_x(this);
		clearpad_firmware_reset(this);
		break;
	case SYN_STATE_FLASH_DISABLE:
		HWLOGE(this, "should be handled by clearpad_initialize\n");
		break;
	default:
		HWLOGE(this, "invalid state(%s)",
			      NAME_OF(clearpad_state_name, this->state));
		rc = -EINVAL;
		break;
	}
	return rc;
}

/* for Bootloader v7.x */
static int clearpad_flash_enable_bl_v7_x(struct clearpad_t *this)
{
	int rc;
	u8 buf[SYN_SINGLE_TRANSACTION_SIZE] = {
		PID_BOOTLOADER, 0x00, 0x00, 0x00, 0x00,
		FLASH_CMD_ENTER_BOOTLOADER};

	/* set device configured bit */
	/* F01_RMI_CTRL00: Device Control */
	rc = clearpad_put_bit(SYNF(this, F01_RMI, CTRL,
				  this->reg_offset.f01_ctrl00),
			      DEVICE_CONTROL_CONFIGURED_MASK,
			      DEVICE_CONTROL_CONFIGURED_MASK);
	if (rc) {
		HWLOGE(this, "failed to set device configured bit\n");
		goto end;
	}
	clearpad_set_delay(SYN_WAIT_TIME_AFTER_REGISTER_ACCESS);

	/* read flash program key */
	rc = clearpad_get_block(SYNF(this, F34_FLASH, QUERY,
			this->reg_offset.f34_query01),
			buf + SYN_FP_KEY_OFFSET, 2);
	if (rc) {
		HWLOGE(this, "failed to read flash program key\n");
		goto end;
	}

	/* issue command to enter bootloader mode with key*/
	this->flash.enter_bootloader_mode = true;
	rc = clearpad_put_block(SYNF(this, F34_FLASH, DATA,
			this->reg_offset.f34_data01),
			buf, sizeof(buf));

	if (rc) {
		HWLOGE(this, "failed to enter bootloader mode\n");
		goto end;
	}

	this->state = SYN_STATE_FLASH_ENABLE;

end:
	return rc;
}

static int clearpad_flash_program_bl_v7_x(struct clearpad_t *this)
{
	struct clearpad_flash_t *flash = &this->flash;
	int rc;
	u8 buf[SYN_SINGLE_TRANSACTION_SIZE] = {
		0x00, 0x00, 0x00, 0x00, 0x00, FLASH_CMD_ERASE};

	/* make sure that we are in programming mode and there are no issues */
	rc = clearpad_get(SYNF(this, F34_FLASH, DATA,
		this->reg_offset.f34_data00), buf);
	if (rc) {
		HWLOGE(this, "failed to get flash programming status\n");
		goto end;
	}

	if (!(BIT_GET(buf[0], STATUS_BL_MODE))) {
		HWLOGE(this, "failed enabling flash (%s)\n",
			      NAME_OF(clearpad_flash_status_name,
				      BIT_GET(buf[0], STATUS_FLASH_STATUS)));
		rc = -EIO;
		goto end;
	}

	HWLOGI(this, "flashing enabled\n");

	if (this->state == SYN_STATE_FLASH_ENABLE) {
		/* PDT may have changed, re-read */
		rc = clearpad_read_pdt(this);
		if (rc) {
			HWLOGE(this, "failed to read pdt\n");
			goto end;
		}
	}

	/* read flash program key */
	rc = clearpad_get_block(SYNF(this, F34_FLASH, QUERY,
				this->reg_offset.f34_query01),
				buf + SYN_FP_KEY_OFFSET, 2);
	if (rc) {
		HWLOGE(this, "failed to flash program key\n");
		goto end;
	}

	if (this->state == SYN_STATE_FLASH_ENABLE &&
	    (flash->command == SYN_FORCE_FLASH ||
	     flash->command == SYN_DEFAULT_FLASH)) {
		buf[0] = PID_CORE_CODE;
		this->state = SYN_STATE_FLASH_PROGRAM;
	} else if ((this->state == SYN_STATE_FLASH_PROGRAM &&
		    (flash->command == SYN_FORCE_FLASH ||
		     flash->command == SYN_DEFAULT_FLASH))) {
		buf[0] = PID_CORE_CONFIGURATION;
		this->state = SYN_STATE_FLASH_ERASE;
	} else {
		HWLOGE(this, "invalid state(%s) for command(%s)\n",
			NAME_OF(clearpad_state_name, this->state),
			NAME_OF(clearpad_flash_command_name, flash->command));
		rc = -EINVAL;
		goto end;
	}

	/* issue command to erase partition with key*/
	rc = clearpad_put_block(SYNF(this, F34_FLASH, DATA,
				this->reg_offset.f34_data01), buf, sizeof(buf));
	if (rc) {
		HWLOGE(this, "failed to erase partition with key\n");
		goto end;
	}

	HWLOGI(this, "firmware erasing\n");
end:
	return rc;
}

static int clearpad_flash_update_bl_v7_x(struct clearpad_t *this)
{
	struct clearpad_flash_t *flash = &this->flash;
	int rc, len;
	const u8 *data;
	u8 buf, pid;
	u8 block_buf[3];
	u16 block_num = 0;
	struct clearpad_flash_block_t *object;
	enum clearpad_state_e finish_state;

	if (this->state == SYN_STATE_FLASH_ERASE &&
	    (flash->command == SYN_FORCE_FLASH ||
	     flash->command == SYN_DEFAULT_FLASH)) {
		object = &(&this->flash)->data;
		pid = PID_CORE_CODE;
		finish_state = SYN_STATE_FLASH_DATA;
	} else if ((this->state == SYN_STATE_FLASH_DATA &&
		    (flash->command == SYN_FORCE_FLASH ||
		     flash->command == SYN_DEFAULT_FLASH))) {
		object = &(&this->flash)->config;
		pid = PID_CORE_CONFIGURATION;
		finish_state = SYN_STATE_FLASH_CONFIG;
	} else {
		HWLOGE(this, "invalid state(%s) for command(%s)\n",
			NAME_OF(clearpad_state_name, this->state),
			NAME_OF(clearpad_flash_command_name, flash->command));
		rc = -EINVAL;
		goto end;
	}

	if (object->pos > 0)
		goto write_block_data;

	/* make sure that we are in programming mode and there are no issues */
	rc = clearpad_get(SYNF(this, F34_FLASH, DATA,
		this->reg_offset.f34_data00), &buf);
	if (rc) {
		HWLOGE(this, "failed to get flash programming status\n");
		goto end;
	}

	if (!(BIT_GET(buf, STATUS_BL_MODE))) {
		HWLOGE(this, "failed flashing in %s (%s)\n",
		     NAME_OF(clearpad_state_name, this->state),
		     NAME_OF(clearpad_flash_status_name,
			     BIT_GET(buf, STATUS_FLASH_STATUS)));
		rc = -EIO;
		goto end;
	}

	object->payload_length = SYN_PAYLOAD_LENGTH;

	/* read block size */
	rc = clearpad_get_block(SYNF(this, F34_FLASH, QUERY,
				this->reg_offset.f34_query03),
				block_buf, 3);
	if (rc) {
		HWLOGE(this, "unable to read block size of fw\n");
		goto end;
	}

	object->block_size = ((block_buf[2] << 8) | block_buf[1]);

	object->remain_block = (object->length / object->block_size)
		% object->payload_length;
	object->transation_count = (object->length / object->block_size)
		/ object->payload_length;
	object->blocks = (object->length / object->block_size)
		+ !!(object->length % object->block_size);

	if (object->remain_block > 0)
		object->transation_count++;

	/* write partition id */
	rc = clearpad_put(SYNF(this, F34_FLASH, DATA,
		this->reg_offset.f34_data01), pid);
	if (rc) {
		HWLOGE(this, "unable to write partition id\n");
		goto end;
	}

	/* write block num */
	rc = clearpad_put_block(SYNF(this, F34_FLASH, DATA,
					this->reg_offset.f34_data02),
					(u8 *)&block_num, 2);
	if (rc) {
		HWLOGE(this, "unable to write block number\n");
		goto end;
	}

write_block_data:
	if ((object->pos == (object->transation_count - 1))
			&& (object->remain_block > 0))
		object->payload_length = object->remain_block;

	data = object->data + object->pos * object->block_size;
	len = object->length - object->pos * object->block_size;

	if (len > object->block_size)
		len = object->block_size;

	/* write payload length */
	rc = clearpad_put_block(SYNF(
		this, F34_FLASH, DATA, this->reg_offset.f34_data03),
		(u8 *)&(object->payload_length), 2);
	if (rc) {
		HWLOGE(this, "unable to write payload length\n");
		goto end;
	}

	/* write command */
	rc = clearpad_put(
		SYNF(this, F34_FLASH, DATA, this->reg_offset.f34_data04),
		FLASH_CMD_WRITE);
	if (rc) {
		HWLOGE(this, "unable to write flash cmd\n");
		goto end;
	}

	/* issue a write data/configuration block command with data */
	rc = clearpad_put_block(SYNF(this, F34_FLASH, DATA,
		this->reg_offset.f34_data05), data, len);
	if (rc) {
		HWLOGE(this, "unable to write data/configuration block cmd\n");
		goto end;
	}

	if (object->pos % 100 == 0)
		LOGI(this, "wrote %d blocks\n", object->pos);

	/* if we've reached the end of the data/configuration flashing */
	if (++object->pos == object->blocks) {
		HWLOGI(this, "flash finished on %s\n",
			      NAME_OF(clearpad_state_name, this->state));
		this->state = finish_state;
	}
end:
	return rc;
}

static int clearpad_flash_disable_bl_v7_x(struct clearpad_t *this)
{
	int rc;
	u8 buf;

	/* make sure that we are in programming mode and there are no issues */
	rc = clearpad_get(SYNF(this, F34_FLASH, DATA,
			this->reg_offset.f34_data00), &buf);
	if (rc)
		goto end;

	if (!(BIT_GET(buf, STATUS_BL_MODE))) {
		HWLOGE(this, "failed flashing config (%s)\n",
			      NAME_OF(clearpad_flash_status_name,
				      BIT_GET(buf, STATUS_FLASH_STATUS)));
		rc = -EIO;
		goto end;
	}

	clearpad_set_delay(SYN_WAIT_TIME_AFTER_REGISTER_ACCESS);

	/* send a reset to the device to complete the flash procedure */
	clearpad_reset(this, SYN_SWRESET, __func__);

	HWLOGI(this, "flashing finished, resetting\n");
	this->state = SYN_STATE_FLASH_DISABLE;
end:
	return rc;
}

/* need LOCK(&this->lock) */
static int clearpad_flash_bl_v7_x(struct clearpad_t *this)
{
	int rc = 0;

	switch (this->state) {
	case SYN_STATE_FLASH_IMAGE_SET:
		rc = clearpad_flash_enable_bl_v7_x(this);
		break;
	case SYN_STATE_FLASH_ENABLE:
	case SYN_STATE_FLASH_PROGRAM:
		rc = clearpad_flash_program_bl_v7_x(this);
		break;
	case SYN_STATE_FLASH_ERASE:
	case SYN_STATE_FLASH_DATA:
		rc = clearpad_flash_update_bl_v7_x(this);
		break;
	case SYN_STATE_FLASH_CONFIG:
		rc = clearpad_flash_disable_bl_v7_x(this);
		clearpad_firmware_reset(this);
		break;
	case SYN_STATE_FLASH_DISABLE:
		HWLOGE(this, "should be handled by clearpad_initialize\n");
		break;
	default:
		HWLOGE(this, "invalid state(%s)",
			      NAME_OF(clearpad_state_name, this->state));
		rc = -EINVAL;
		break;
	}
	return rc;
}

/* need LOCK(&this->lock) */
static int clearpad_flash(struct clearpad_t *this)
{
	int rc = 0;
	struct clearpad_device_info_t *info = &this->device_info;

	LOGD(this, "flash state=%s\n",
	     NAME_OF(clearpad_state_name, this->state));

	switch (info->bootloader_version) {
	case BV6:
	case BV6_8:
		rc = clearpad_flash_bl_v6_x(this);
		break;
	case BV7:
		rc = clearpad_flash_bl_v7_x(this);
		break;
	default:
		rc = -EINVAL;
		HWLOGE(this, "invalid bl version\n");
		break;
	}
	if (rc) {
		HWLOGE(this, "failed during flash (%s), rc = %d\n",
		     NAME_OF(clearpad_state_name, this->state), rc);

		snprintf(this->result_info, SYN_STRING_LENGTH,
			"%s, family 0x%02x, fw rev 0x%02x.%02x, "
			"extra 0x%02x, failed fw update\n",
			clearpad_s(this->device_info.product_id,
			HEADER_PRODUCT_ID_SIZE),
			this->device_info.customer_family,
			this->device_info.firmware_revision_major,
			this->device_info.firmware_revision_minor,
			this->device_info.firmware_revision_extra);
		this->flash_requested = false;
		clearpad_firmware_reset(this);
		HWLOGI(this, "result: %s", this->result_info);

		if (this->state != SYN_STATE_FLASH_IMAGE_SET)
			clearpad_notify_interrupt(this,
					&this->interrupt.for_reset, rc);
	}
	return rc;
}

/* need LOCK(&this->lock) */
bool clearpad_is_healthy(struct clearpad_t *this)
{
	u8 status = 0;
	int rc = 0;

	switch (this->chip_id) {
	case SYN_CHIP_332U:
		/* F01_RMI_CTRL00: Device Control */
		rc = clearpad_get(SYNF(this, F01_RMI, CTRL,
			this->reg_offset.f01_ctrl00), &status);
		if (rc) {
			LOGE(this, "rc=%d\n", rc);
			return false;
		}
		if (BIT_GET(status, DEVICE_CONTROL_CONFIGURED)) {
			LOGE(this, "status=0x%02x\n", status);
			return false;
		}
		/* fall-through */
	case SYN_CHIP_3330:
	case SYN_CHIP_3500:
	case SYN_CHIP_4353:
		/* F01_RMI_DATA00: Device Status */
		rc = clearpad_get(SYNF(this, F01_RMI, DATA,
			this->reg_offset.f01_data00), &status);
		if (rc) {
			LOGE(this, "rc=%d\n", rc);
			return false;
		}
		if (BIT_GET(status, DEVICE_STATUS_CODE)) {
			LOGE(this, "status=0x%02x\n", status);
			switch (BIT_GET(status, DEVICE_STATUS_CODE)) {
			case DEVICE_STATUS_CODE_DEVICE_FAILURE:
			case DEVICE_STATUS_CODE_CONFIGURATION_CRC_FAILURE:
			case DEVICE_STATUS_CODE_FIRMWARE_CRC_FAILURE:
			case DEVICE_STATUS_CODE_GUEST_CRC_FAILURE:
			case DEVICE_STATUS_CODE_EXTERNAL_AFE_FAILURE:
			case DEVICE_STATUS_CODE_DISPLAY_FAILURE:
			return false;
			default:
				return true;
			}
		}
		break;
	default:
		LOGE(this, "not supported chip id (0x%02x)\n", this->chip_id);
		rc = -EINVAL;
		break;
	}
	return true;
}

static void clearpad_watchdog_work(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct clearpad_watchdog_t *wd = (struct clearpad_watchdog_t *)dwork;
	struct clearpad_t *this = container_of(wd, struct clearpad_t, watchdog);
	bool healthy = true;
	int rc;

	LOGD(this, "start\n");

	LOCK(&this->lock);
	if (!this->dev_active || this->interrupt.count == 0) {
		LOGI(this, "avoid to access I2C before waiting %d ms delay\n",
				this->reset.delay_for_powerup_ms);

		if (!this->touchctrl.will_powerdown && !this->flash_requested)
			schedule_delayed_work(&this->watchdog.work,
					      this->watchdog.delay);

		goto not_ready_to_access_i2c;
	}

	if (!touchctrl_lock_power(this, __func__, true, false)) {
		LOG_STAT(this, "stop watchdog because of no lock power\n");
		UNLOCK(&this->lock);
		return;
	}

	if (this->dev_active && !this->flash_requested)
		healthy = clearpad_is_healthy(this);
	LOG_STAT(this, "%s (icount=%u)\n",
	     healthy ? "healthy" : "NEED HW RESET", this->interrupt.count);
	if (!healthy) {
		clearpad_debug_info(this);
		clearpad_reset(this, SYN_HWRESET, "Watchdog");
		UNLOCK(&this->lock);
		rc = clearpad_wait_for_interrupt(this,
					&this->interrupt.for_reset,
					this->interrupt.wait_ms);
		LOCK(&this->lock);
		if (rc)
			LOGE(this, "failed to get interrupt (rc=%d)\n", rc);
	}
	if (this->dev_active && !this->flash_requested)
		schedule_delayed_work(&this->watchdog.work,
				      this->watchdog.delay);
	touchctrl_unlock_power(this, __func__);
not_ready_to_access_i2c:
	UNLOCK(&this->lock);
}

/* need LOCK(&this->lock) */
static void clearpad_watchdog_update(struct clearpad_t *this)
{
	if (!this->watchdog.enabled)
		return;

	cancel_delayed_work(&this->watchdog.work);

	if (this->dev_active && !this->flash_requested)
		schedule_delayed_work(&this->watchdog.work,
				      this->watchdog.delay);
}

/* need LOCK(&this->lock) */
static int clearpad_set_resume_mode(struct clearpad_t *this)
{
	int rc = 0;
	u8 interrupt;
	u8 value;

	HWLOGI(this, "set resume mode\n");
	WARN_ON(this->dev_active && !this->early_suspend);

	if (this->post_probe.done)
		clearpad_funcarea_invalidate_all(this);

	/* F01_RMI_CTRL01.00: Interrupt Enable 0 */
	rc = clearpad_put(SYNF(this, F01_RMI, CTRL,
		this->reg_offset.f01_ctrl01),
		INTERRUPT_ENABLE_0_ENABLE_ALL);
	if (rc) {
		LOGE(this, "failed to set interrupt enable\n");
		goto end;
	}

	/* F01_RMI_CTRL00: Device Control */
	rc = clearpad_get(SYNF(this, F01_RMI, CTRL,
		this->reg_offset.f01_ctrl00), &value);
	if (rc) {
		LOGE(this, "failed to get sleep status\n");
		goto end;
	}
	if (BIT_GET(value, DEVICE_CONTROL_SLEEP_MODE)) {
		rc = clearpad_put_bit(SYNF(this, F01_RMI, CTRL,
			this->reg_offset.f01_ctrl00),
			DEVICE_CONTROL_SLEEP_MODE_NORMAL_OPERATION,
			DEVICE_CONTROL_SLEEP_MODE_MASK);
		if (rc) {
			LOGE(this, "failed to exit sleep mode\n");
			goto end;
		}
		clearpad_set_delay(this->charger_only.delay_ms);
	}

	this->early_suspend = false;

	if (this->wakeup_gesture.enabled) {
		rc = clearpad_disable_wakeup_gesture(this);
		if (rc) {
			LOGE(this, "failed to disable wakeup gesture\n");
			goto end;
		}
	}

	if (this->cover.enabled)
		rc = clearpad_set_cover_status(this);

	this->dev_active = true;

	if (clearpad_set_noise_det_irq(this, true, true))
		HWLOGI(this, "no noise_det irq change (enable)\n");
	else
		LOGI(this, "noise_det irq was enabled\n");

	if (!is_irq_enabled(this)) {
		/* F01_RMI_DATA01: Interrupt Status */
		rc = clearpad_get(SYNF(this, F01_RMI, DATA,
			this->reg_offset.f01_data01), &interrupt);
		if (rc) {
			LOGE(this, "failed to read interrupt status\n");
			goto end;
		}
		LOGI(this, "ignore interrupt 0x%02x\n", interrupt);
		clearpad_set_irq(this, true);
	}

end:
	LOGI(this, "set resume mode (rc=%d)\n", rc);
	return rc;
}

/* This function is called from touch backlight daemon context
 * while clearpad_set_suspend_mode() is called from EARLY POWERDOWN.
 *
 * need LOCK(&this->lock)
 */
static int clearpad_set_early_suspend_mode(struct clearpad_t *this)
{
	int rc = 0;

	HWLOGI(this, "set early suspend mode\n");

	if (this->force_sleep != FSMODE_KEEP) {
		/* F01_RMI_CTRL01.00: Interrupt Enable 0 */
		rc = clearpad_put(SYNF(this, F01_RMI, CTRL,
			this->reg_offset.f01_ctrl01),
				  INTERRUPT_ENABLE_0_DISABLE_ALL);
		if (rc) {
			LOGE(this, "failed to disable interrupt mode\n");
			goto end;
		}
	} else {
		/* F01_RMI_CTRL00: Device Control */
		rc = clearpad_put_bit(SYNF(this, F01_RMI, CTRL,
			this->reg_offset.f01_ctrl00),
			DEVICE_CONTROL_SLEEP_MODE_SENSOR_SLEEP,
			DEVICE_CONTROL_SLEEP_MODE_MASK);
		if (rc) {
			LOGE(this, "failed to exit normal mode\n");
			goto end;
		}
		clearpad_set_delay(this->charger_only.delay_ms);
	}
	clearpad_set_irq(this, false);
	this->early_suspend = true;

	HWLOGI(this, "enter sleep mode\n");
end:
	return rc;
}

/* need LOCK(&this->lock) */
static int clearpad_set_suspend_mode(struct clearpad_t *this)
{
	int rc = 0;

	if (this->dev_active)
		HWLOGI(this, "set suspend mode\n");
	else
		HWLOGI(this, "change suspend mode\n");

	if (this->force_sleep != FSMODE_KEEP &&
	    this->wakeup_gesture.enabled) {
		HWLOGI(this, "prepare for wakeup gesture mode");
		if (this->early_suspend) {
			/* F01_RMI_CTRL01.00: Interrupt Enable 0 */
			rc = clearpad_put(SYNF(this, F01_RMI, CTRL,
				this->reg_offset.f01_ctrl01),
				INTERRUPT_ENABLE_0_ENABLE_ALL);
			if (rc) {
				LOGE(this, "failed to exit sleep mode\n");
				goto end;
			}
		}

		rc = clearpad_enable_wakeup_gesture(this);
		if (rc) {
			LOGE(this, "failed to enable wakeup gesture\n");
			goto end;
		}

		this->wakeup_gesture.time_started = jiffies - 1;
		clearpad_set_delay(SYN_WAIT_TIME_AFTER_REGISTER_ACCESS);
		clearpad_set_irq(this, true);
		HWLOGI(this, "enter wakeup gesture mode\n");
	} else {
		if (!this->early_suspend) {
			if (this->force_sleep != FSMODE_KEEP) {
				/* F01_RMI_CTRL01.00: Interrupt Enable 0 */
				rc = clearpad_put(SYNF(this, F01_RMI, CTRL,
					this->reg_offset.f01_ctrl01),
					INTERRUPT_ENABLE_0_DISABLE_ALL);
				if (rc) {
					LOGE(this, "failed to set interrupt"
						   "disable\n");
					goto end;
				}
			} else {
				/* F01_RMI_CTRL00: Device Control */
				rc = clearpad_put_bit(
					SYNF(this, F01_RMI, CTRL,
					this->reg_offset.f01_ctrl00),
					DEVICE_CONTROL_SLEEP_MODE_SENSOR_SLEEP,
					DEVICE_CONTROL_SLEEP_MODE_MASK);
				if (rc) {
					LOGE(this, "failed to exit normal"
						   "mode\n");
					goto end;
				}
				clearpad_set_delay(this->charger_only.delay_ms);
			}
			clearpad_set_irq(this, false);
			HWLOGI(this, "enter sleep mode\n");
		} else {
			HWLOGI(this, "already in sleep mode\n");
		}
	}

	this->dev_active = false;

	if (clearpad_set_noise_det_irq(this, false, false))
		HWLOGI(this, "no noise_det irq change (disable)\n");
	else
		LOGI(this, "noise_det irq was disabled\n");

end:
	if (rc)
		LOGI(this, "set suspend mode (rc=%d)\n", rc);

	return rc;
}

/*
 * Reset
 */

static void clearpad_reset_work(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct clearpad_reset_t *reset
		= container_of(dwork, struct clearpad_reset_t, work);
	struct clearpad_t *this
		= container_of(reset, struct clearpad_t, reset);
	int retry = 0;
	int rc;
	unsigned long flags;

	LOGI(this, "start %s '%s'\n",
	     NAME_OF(clearpad_reset_name, reset->mode),
	     this->interrupt.for_reset.name);

	LOCK(&this->lock);
	if (!touchctrl_lock_power(this, __func__, true, false)) {
		LOG_STAT(this, "stop reset work because of no power\n");
		clearpad_notify_interrupt(this,
					  &this->interrupt.for_reset, -EPERM);
		goto err_in_lock_power;
	}

retry_reset:
	this->interrupt.count = 0; /* to detect first interrupt */
	spin_lock_irqsave(&this->slock, flags);
	this->dev_busy = false;
	this->irq_pending = false;
	spin_unlock_irqrestore(&this->slock, flags);
	switch (reset->mode) {
	case SYN_HWRESET:
	case SYN_FORCE_HWRESET:
		rc = touchctrl_hwreset(this, reset->mode);
		if (rc && retry++ < SYN_RETRY_NUM_OF_RESET)
			goto retry_reset;
		break;
	case SYN_SWRESET:
	case SYN_FORCE_SWRESET:
		/* F01_RMI_CMD00: Device Command */
		rc = clearpad_put(
				SYNF(this, F01_RMI, COMMAND,
					this->reg_offset.f01_cmd00),
				DEVICE_COMMAND_RESET_MASK);
		if (rc && retry++ < SYN_RETRY_NUM_OF_RESET)
			goto retry_reset;
		break;
	default:
		rc = -EINVAL;
		LOGE(this, "invalid mode %d\n", reset->mode);
		break;
	}
	LOGW(this, "state '%s' was interrupted by reset\n",
	     NAME_OF(clearpad_state_name, this->state));
	/* will be changed to RUNNING by clearpad_initialize */
	this->state = SYN_STATE_DISABLED;
	/* inform about this reset to running session */
	clearpad_notify_interrupt(this, &this->interrupt.for_F34, -EINTR);
	clearpad_notify_interrupt(this, &this->interrupt.for_F54, -EINTR);

	if ((rc == -EAGAIN || rc == -EBUSY) &&
	    reset->retry++ < SYN_RETRY_NUM_OF_RESET) {
		LOGI(this, "schedule reset for retry (rc=%d)\n", rc);
		schedule_delayed_work(&reset->work, 1 * HZ);
	} else if (rc) {
		LOGE(this, "failed to execute %s '%s'\n",
		     NAME_OF(clearpad_reset_name, reset->mode),
		     this->interrupt.for_reset.name);
		clearpad_notify_interrupt(this,
					  &this->interrupt.for_reset, rc);
	}
	if (this->watchdog.enabled)
		clearpad_watchdog_update(this);

	touchctrl_unlock_power(this, __func__);
	LOGI(this, "done\n");
err_in_lock_power:
	UNLOCK(&this->lock);
	return;
}

/*
 * request asynchronized reset
 * need LOCK(&this->lock) */
static void clearpad_reset(struct clearpad_t *this,
			  enum clearpad_reset_e mode, const char *cause)
{
	struct clearpad_reset_t *reset = &this->reset;

	if (!this->dev_active
	    && mode != SYN_FORCE_HWRESET && mode != SYN_FORCE_SWRESET) {
		HWLOGW(this, "could not execute %s because "
			"device is suspended and not force reset\n",
			NAME_OF(clearpad_reset_name, mode));
		goto err_in_device_mode;
	}

	if (atomic_read(&this->interrupt.for_reset.done) == 0 &&
	    this->interrupt.for_reset.name) {
		LOGI(this, "request %s '%s' continued to %s '%s'\n",
		     NAME_OF(clearpad_reset_name, mode), cause,
		     NAME_OF(clearpad_reset_name, reset->mode),
		     this->interrupt.for_reset.name);
	}

	if (this->watchdog.enabled)
		cancel_delayed_work(&this->watchdog.work);

	reset->mode = mode;
	this->interrupt.for_reset.name = cause;
	clearpad_prepare_for_interrupt(this, &this->interrupt.for_reset, cause);

	reset->retry = 0;
	LOGE(this, "schedule %s '%s'\n",
	     NAME_OF(clearpad_reset_name, mode), cause);
	if (!schedule_delayed_work(&reset->work, 0))
		LOGI(this, "already reset on queue\n");

err_in_device_mode:
	return;
}

static int clearpad_gpio_export(struct clearpad_t *this,
				struct device *dev, bool export)
{
	int rc = 0;

	if (export) {
		rc = gpio_export(this->pdata->irq_gpio, false);
		if (rc) {
			LOGE(this, "failed to export gpio (rc=%d)\n", rc);
		} else {
			rc = gpio_export_link(dev, "attn",
					this->pdata->irq_gpio);
			if (rc)
				LOGE(this, "failed to export (rc=%d)\n", rc);
		}
		if (this->noise_det.supported) {
			rc = gpio_export(this->noise_det.irq_gpio, false);
			if (rc) {
				LOGE(this, "failed to export noise_det gpio\n"
				     " (rc=%d)\n", rc);
			} else {
				rc = gpio_export_link(dev, "noise_det",
						this->noise_det.irq_gpio);
				if (rc)
					LOGE(this,
					     "failed to export noise_det link"
					     " (rc=%d)\n", rc);
			}
		}
	} else {
		gpio_unexport(this->pdata->irq_gpio);
		sysfs_remove_link(&dev->kobj, "attn");
		if (this->noise_det.supported) {
			gpio_unexport(this->noise_det.irq_gpio);
			sysfs_remove_link(&dev->kobj, "noise_det");
		}
	}

	return rc;
}

static void clearpad_funcarea_initialize(struct clearpad_t *this)
{
	struct clearpad_funcarea_t *funcarea;
	struct clearpad_area_t pointer_area;
	struct clearpad_button_data_t *button;
	struct clearpad_pointer_data_t *pointer_data;
	static const char const *func_name[] = {
		[SYN_FUNCAREA_INSENSIBLE] = "insensible",
		[SYN_FUNCAREA_POINTER] = "pointer",
		[SYN_FUNCAREA_BUTTON] = "button",
	};

	this->funcarea = clearpad_funcarea_get(this,
				this->device_info.customer_family,
				this->device_info.firmware_revision_major);
	funcarea = this->funcarea;

	if (funcarea == NULL) {
		LOGI(this, "no funcarea\n");
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

		LOGI(this, "funcarea '%s' [%d, %d, %d, %d] [%d, %d, %d, %d]\n",
		     NAME_OF(func_name, funcarea->func),
		     funcarea->original.x1, funcarea->original.y1,
		     funcarea->original.x2, funcarea->original.y2,
		     funcarea->extension.x1, funcarea->extension.y1,
		     funcarea->extension.x2, funcarea->extension.y2);
	}
}

static inline bool clearpad_funcarea_test(struct clearpad_area_t *area,
					   struct clearpad_point_t *point)
{
	return area->x1 <= point->x && point->x <= area->x2
		&& area->y1 <= point->y && point->y <= area->y2;
}

static struct clearpad_funcarea_t *
clearpad_funcarea_search(struct clearpad_t *this,
			  struct clearpad_pointer_t *pointer)
{
	struct clearpad_funcarea_t *funcarea = this->funcarea;

	if (funcarea == NULL)
		goto end;

	/* get new funcarea */
	for (; funcarea->func != SYN_FUNCAREA_END; funcarea++) {
		if (clearpad_funcarea_test(&funcarea->original,
						&pointer->cur))
			goto end;
		if (funcarea->func == SYN_FUNCAREA_POINTER
		    && clearpad_funcarea_test(&funcarea->extension,
						&pointer->cur))
			goto end;
	}
	funcarea = NULL;
end:
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
	struct clearpad_funcarea_t *new_funcarea;

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
	struct clearpad_area_t *extension;

	if (clearpad_is_valid_function(this, SYN_F12_2D))
		clearpad_parse_finger_n_f12(this, buf, finger, &new_point);
	else
		return;

	/* check finger state */
	if (new_point.tool == SYN_TOOL_FINGER ||
		(this->glove.enabled && new_point.tool == SYN_TOOL_GLOVE) ||
		(this->pen.enabled && (new_point.tool == SYN_TOOL_PEN))) {

		switch (this->flip_config) {
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
			if (clearpad_funcarea_test(&this->funcarea->original, &new_point)) {
				/* first touch event */
				pointer->down = true;
				pointer->cur = new_point;
				pointer->funcarea
					= clearpad_funcarea_search(this, pointer);
				LOG_VERBOSE(this, "validate pointer %d [func %d]\n",
					    new_point.id, pointer->funcarea
					    ? pointer->funcarea->func : -1);
			} else {
				LOGW(this, "invalidate pointer %d\n", new_point.id);
			}
		}
		if (pointer->funcarea) {
			extension = &pointer->funcarea->extension;

			if (clearpad_funcarea_test(extension, &new_point)) {
				pointer->cur = new_point;
				clearpad_funcarea_down(this, pointer);
			} else {
				pointer->down = false;
				clearpad_funcarea_out(this, pointer);
				LOGW(this, "invalidate pointer %d\n", new_point.id);
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
}

static int get_num_fingers_f12(struct clearpad_t *this,
	int *num_fingers)
{
	int rc, num;
	u16 val, mask;
	const int max_objects = this->extents.n_fingers;

	/* F12_2D_DATA15: Object Attention */
	rc = clearpad_get_block(SYNA(this, F12_2D, DATA, 15),
				(u8 *)&val, sizeof(val));
	if (rc)
		goto end;

	val = le16_to_cpu(val);

	for (num = 0, mask = 0x1; num < max_objects; num++, mask <<= 1)
		if (val < mask)
			break;

	*num_fingers = num;

	LOG_CHECK(this, "fingers=%d, 0x%04hX", num, val);
end:
	return rc;
}

static int clearpad_read_fingers_f12(struct clearpad_t *this)
{
	int rc, finger, num_fingers;
	u8 buf[this->extents.n_fingers * this->extents.n_bytes_per_object];

	memset(buf, 0, sizeof(buf));

	rc = get_num_fingers_f12(this, &num_fingers);
	if (rc)
		goto end;

	if (num_fingers > 0) {
		/* F12_2D_DATA01: Sensed Objects */
		rc = clearpad_get_block(SYNA(this, F12_2D, DATA, 1),
			buf, num_fingers * this->extents.n_bytes_per_object);
		if (rc)
			goto end;
	}

	for (finger = 0; finger < this->extents.n_fingers; finger++)
		clearpad_report_finger_n(this, buf, finger);
end:
	return rc;
}

static int clearpad_handle_gesture(struct clearpad_t *this)
{
	u8 wakeint = 0;
	int rc = -EIO;

	if (clearpad_is_valid_function(this, SYN_F12_2D))
		/* F12_2D_DATA04: Detected Gestures */
		rc = clearpad_get(SYNA(this, F12_2D, DATA, 0x04), &wakeint);
	if (rc)
		goto end;

	LOGI(this, "gesture: %d\n", wakeint);

	if (time_after(jiffies, this->wakeup_gesture.time_started))
		this->wakeup_gesture.time_started = jiffies +
		msecs_to_jiffies(this->wakeup_gesture.timeout_delay);
	else
		goto end;

	evdt_execute(this->evdt_node, this->input, wakeint);
end:
	return rc;
}

static int clearpad_process_F01_RMI(struct clearpad_t *this, u8 device_status)
{
	int rc = 0;

	HWLOGI(this, "device status 0x%02x (code=%d, unconfig=%d)\n",
	       device_status, BIT_GET(device_status, DEVICE_STATUS_CODE),
	       BIT_GET(device_status, DEVICE_STATUS_UNCONFIGURED));
	if (BIT_GET(device_status, DEVICE_STATUS_CODE)
	    == DEVICE_STATUS_CODE_RESET_OCCURRED &&
	    BIT_GET(device_status, DEVICE_STATUS_UNCONFIGURED)) {
		LOGI(this, "device reset\n");
		/* initialized already */
		WARN_ON(this->interrupt.count != 0);
		/* check result of flash disabling */
		WARN_ON(BIT_GET(device_status, DEVICE_STATUS_FLASH_PROG));
		HWLOGI(this, "clear glitch (irq_pending=%s) for reset\n",
		       this->irq_pending ? "true" : "false");

		if (this->force_sleep == FSMODE_ONESHOT) {
			LOGI(this, "clear force sleep mode\n");
			this->force_sleep = FSMODE_OFF;
		}
		if (this->force_sleep != FSMODE_OFF) {
			HWLOGI(this, "force sleep mode\n");
			if (this->dev_active) {
				rc = clearpad_set_suspend_mode(this);
				if (rc)
					HWLOGE(this, "failed to suspend "
						     "(rc=%d)\n", rc);
			}
		} else if (!this->dev_active) {
			rc = clearpad_set_resume_mode(this);
			if (rc)
				HWLOGE(this, "failed to resume (rc=%d)\n", rc);
		}
		clearpad_notify_interrupt(this, &this->interrupt.for_reset, 0);
	} else if (BIT_GET(device_status, DEVICE_STATUS_CODE)
		   == DEVICE_STATUS_CODE_DEVICE_FAILURE) {
		clearpad_reset(this, SYN_HWRESET, __func__);
	} else {
		LOGI(this, "unexpected device status=0x%02x\n", device_status);
	}
	return rc;
}

static int clearpad_process_F12_2D(struct clearpad_t *this, bool *waked)
{
	int rc = 0;
	u8 status;

	*waked = false;

	if (!this->post_probe.done) {
		LOGI(this, "ignore event before end of post probe\n");
		goto end;
	}

	if (this->wakeup_gesture.enabled && !this->dev_active &&
	    !this->last_irq) {
		rc = clearpad_handle_gesture(this);
		if (!rc)
			*waked = true;
		goto end;
	}

	/* F01_RMI_DATA00: Device Status */
	rc = clearpad_get(SYNF(this, F01_RMI, DATA,
		this->reg_offset.f01_data00), &status);
	LOG_CHECK(this, "rc=%d F01_RMI_DATA00=0x%x\n", rc, status);
	if (rc)
		goto end;

	if (BIT_GET(status, DEVICE_STATUS_CODE)
	    == DEVICE_STATUS_CODE_DEVICE_FAILURE) {
		LOGE(this, "found device failure\n");
		WARN_ON(1); /* instead of HWRESET */
		goto end;
	}

	rc = clearpad_read_fingers_f12(this);
	if (rc)
		goto end;
	if (this->input->users)
		input_sync(this->input);
end:
	return rc;
}

/* need LOCK(&this->lock) */
static int clearpad_process_irq(struct clearpad_t *this)
{
	int rc = 0;
	u8 interrupt_status = 0;
	u8 device_status = 0;
	bool waked = false;
	unsigned long flags;

	if (this->flash.enter_bootloader_mode) {
		clearpad_set_delay(this->reset.delay_for_powerup_ms);
		this->flash.enter_bootloader_mode = false;
		HWLOGI(this, "clear glitch (irq_pending=%s) "
		       "for entering bootloader mode\n",
		       this->irq_pending ? "true" : "false");
		spin_lock_irqsave(&this->slock, flags);
		this->irq_pending = false;
		spin_unlock_irqrestore(&this->slock, flags);
	}
	if (clearpad_initialize_if_first_event(this,
				&interrupt_status, &device_status) < 0) {
		LOGE(this, "failed to initialize for first event\n");
		rc = -EBUSY;
		goto no_valid_interrupt;
	}

	if (interrupt_status & this->pdt[SYN_F34_FLASH].irq_mask) {
		if (this->flash.fw)
			clearpad_flash(this);
		clearpad_notify_interrupt(this, &this->interrupt.for_F34, 0);
	} else if (interrupt_status & this->pdt[SYN_F01_RMI].irq_mask) {
		rc = clearpad_process_F01_RMI(this, device_status);
	} else if (interrupt_status & this->pdt[SYN_F54_ANALOG].irq_mask) {
		this->state = SYN_STATE_RUNNING;
		clearpad_notify_interrupt(this, &this->interrupt.for_F54, 0);
	} else if (interrupt_status & this->pdt[SYN_F12_2D].irq_mask) {
		rc = clearpad_process_F12_2D(this, &waked);
		if (!rc && waked) {
			/* will resume in next first event */
			this->interrupt.count = 0;
			goto no_valid_interrupt;
		}
	} else {
		LOGI(this, "no work, interrupt=[0x%02x]\n", interrupt_status);
	}

	if (this->interrupt.count == 0 && rc)
		goto no_valid_interrupt;

	this->interrupt.count += 1;
	if (this->interrupt.count == 0) {
		LOGI(this, "rewind interrupt.count\n");
		this->interrupt.count = 1;
	}

no_valid_interrupt:
	if (rc) {
		LOGE(this, "error (icount=%u rc=%d)\n",
		     this->interrupt.count, rc);
		WARN_ON(1); /* instead of HWRESET */
	}

	if (this->watchdog.enabled)
		clearpad_watchdog_update(this);

	return rc;
}

/* need LOCK(&this->lock) */
static bool clearpad_process_noise_det_irq(struct clearpad_t *this)
{
	bool retry = false;
	int rc = 0;

	if (!this->post_probe.done) {
		LOGW(this, "post_probe hasn't finished, not need to retry\n");
		goto not_done_post_probe;
	}

	if (this->touchctrl.will_powerdown) {
		LOGW(this, "will_powerdown, not need to retry\n");
		goto will_powerdown;
	}

	if (!this->dev_active || this->interrupt.count == 0) {
		LOGI(this, "not read I2C access, need to retry\n");
		retry = true;
		goto not_ready_to_access_i2c;
	}

	if (!touchctrl_lock_power(this, "noise_det_irq", true, false)) {
		HWLOGW(this, "power is already turned off, need to retry\n");
		retry = true;
		goto end;
	}

	/* F01_RMI_CTRL18: Device Control 1 */
	rc = clearpad_put_bit(
		SYNF(this, F01_RMI, CTRL, this->reg_offset.f01_ctrl18),
		DEVICE_CONTROL_1_GSM_ENABLE_MASK,
		DEVICE_CONTROL_1_GSM_ENABLE_MASK);
	if (rc) {
		LOGE(this, "failed to set noise reduction bit\n");
		retry = true;
		goto err_i2c_access;
	}

err_i2c_access:
	touchctrl_unlock_power(this, "noise_det_irq");

not_done_post_probe:
will_powerdown:
not_ready_to_access_i2c:
end:
	return retry;
}

static irqreturn_t clearpad_threaded_handler(int irq, void *dev_id)
{
	struct clearpad_t *this = dev_id;
	unsigned long flags;
	bool locked;

	get_monotonic_boottime(&this->interrupt.threaded_handler_ts);

	LOCK(&this->lock);
	locked = touchctrl_lock_power(this, "irq_handler", true, false);
	/* workaround to clear interrupt status */
	if (!locked)
		HWLOGW(this, "read interrupt status though no power lock\n");

	do {
		(void)clearpad_process_irq(this);

		spin_lock_irqsave(&this->slock, flags);
		if (likely(!this->irq_pending)) {
			this->dev_busy = false;
			spin_unlock_irqrestore(&this->slock, flags);
			break;
		}
		this->irq_pending = false;
		spin_unlock_irqrestore(&this->slock, flags);
		LOGD(this, "touch irq pending\n");
	} while (true);

	if (locked)
		touchctrl_unlock_power(this, "irq_handler");
	UNLOCK(&this->lock);

	return IRQ_HANDLED;
}

static irqreturn_t clearpad_hard_handler(int irq, void *dev_id)
{
	struct clearpad_t *this = dev_id;
	unsigned long flags;
	irqreturn_t ret;
	bool val;

	get_monotonic_boottime(&this->interrupt.hard_handler_ts);

	spin_lock_irqsave(&this->slock, flags);

	val = gpio_get_value(this->pdata->irq_gpio);
	if (val) {
		spin_unlock_irqrestore(&this->slock, flags);
		return IRQ_HANDLED;
	}

	if (unlikely(this->dev_busy)) {
		this->irq_pending = true;
		ret = IRQ_HANDLED;
	} else {
		this->dev_busy = true;
		ret = IRQ_WAKE_THREAD;
	}
	spin_unlock_irqrestore(&this->slock, flags);
	if (ret == IRQ_HANDLED)
		LOGD(this, "touch irq busy\n");
	return ret;
}

static irqreturn_t clearpad_noise_det_threaded_handler(int irq, void *dev_id)
{
	struct clearpad_t *this = dev_id;
	bool retry = false;

	get_monotonic_boottime(&this->noise_det.threaded_handler_ts);
	LOCK(&this->lock);

	retry = clearpad_process_noise_det_irq(this);

	this->noise_det.threaded_handler_count += 1;
	if (this->noise_det.threaded_handler_count == 0) {
		LOGI(this, "rewind noise_det.threaded_handler_count\n");
		this->noise_det.threaded_handler_count = 1;
	}
	UNLOCK(&this->lock);

	if (retry) {
		clearpad_set_delay(this->noise_det.retry_time_ms);
		LOGI(this, "Enable det_irq for retry hic=%u tic=%u@ "
		     "@ %ld.%06ld\n",
		     this->noise_det.hard_handler_count,
		     this->noise_det.threaded_handler_count,
		     this->noise_det.threaded_handler_ts.tv_sec,
		     this->noise_det.threaded_handler_ts.tv_nsec);
		if (clearpad_set_noise_det_irq(this, true, true))
			HWLOGE(this, "no noise_det irq change (enable)\n");
		else
			LOGI(this, "noise_det irq was enabled\n");
	} else {
		LOGI(this, "Success set fw_bit hic=%u tic=%u@ %ld.%06ld\n",
		     this->noise_det.hard_handler_count,
		     this->noise_det.threaded_handler_count,
		     this->noise_det.threaded_handler_ts.tv_sec,
		     this->noise_det.threaded_handler_ts.tv_nsec);
	}
	return IRQ_HANDLED;
}

static irqreturn_t clearpad_noise_det_hard_handler(int irq, void *dev_id)
{
	struct clearpad_t *this = dev_id;
	irqreturn_t ret;
	unsigned long flags;

	get_monotonic_boottime(&this->noise_det.hard_handler_ts);
	if (clearpad_set_noise_det_irq(this, false, false))
		HWLOGE(this, "no noise_det irq change(disable)\n");
	else
		LOGD(this, "noise_det irq was disabled\n");

	spin_lock_irqsave(&this->noise_det.slock, flags);
	if (this->noise_det.first_irq) {
		this->noise_det.first_irq = false;
		ret = IRQ_HANDLED;
	} else {
		ret = IRQ_WAKE_THREAD;
	}
	this->noise_det.hard_handler_count += 1;
	if (this->noise_det.hard_handler_count == 0) {
		LOGI(this, "rewind hard_handler_count.th_handler_count\n");
		this->noise_det.hard_handler_count = 1;
	}
	spin_unlock_irqrestore(&this->noise_det.slock, flags);

	if (ret == IRQ_HANDLED) {
		if (clearpad_set_noise_det_irq(this, true, false))
			HWLOGE(this, "no noise_det irq change"
			       " w/o 1st_irq(enable)\n");
		else
			LOGD(this, "noise_det irq was enabled w/o 1st_irq\n");
	} else {
		LOGD(this, "Wake up noise_det_threaded_handle hic=%u tic=%u "
		     "@ %ld.%06ld\n",
		     this->noise_det.hard_handler_count,
		     this->noise_det.threaded_handler_count,
		     this->noise_det.hard_handler_ts.tv_sec,
		     this->noise_det.hard_handler_ts.tv_nsec);
	}
	return ret;
}

static int clearpad_device_open(struct input_dev *dev)
{
	struct clearpad_t *this = input_get_drvdata(dev);
	int rc;

	LOG_STAT(this, "state=%s\n", NAME_OF(clearpad_state_name, this->state));

	switch (this->state) {
	case SYN_STATE_INIT:
		rc = 0;
		break;
	case SYN_STATE_DISABLED:
		rc = -ENODEV;
		break;
	case SYN_STATE_RUNNING:
		rc = this->post_probe.done ? 0 : -ENODEV;
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

	LOCK(&this->lock);
	LOG_STAT(this, "state=%s\n", NAME_OF(clearpad_state_name, this->state));

	UNLOCK(&this->lock);
}

/* need LOCK(&this->lock) */
static int clearpad_command_fwflash(struct clearpad_t *this)
{
	struct clearpad_device_info_t *info = &this->device_info;
	int rc;

	memset(this->result_info, 0, SYN_STRING_LENGTH);
	this->flash_requested = true;
	this->reg_offset.updated = false;

	/* prepare to waiting for end of flash */
	clearpad_prepare_for_interrupt(this, &this->interrupt.for_reset,
				       "wait for flashing");

	if (this->flash.fw) {
		HWLOGI(this, "start firmware flash\n");
		this->flash.analog_id = info->analog_id;
		this->state = SYN_STATE_FLASH_IMAGE_SET;
		rc = clearpad_flash(this);
		LOG_CHECK(this, "rc=%d\n", rc);
		if (rc) {
			clearpad_undo_prepared_interrupt(this,
			&this->interrupt.for_reset, "wait for flashing");
			goto error;
		}
	}

	/* wait for end of flash */
	UNLOCK(&this->lock);
	rc = clearpad_wait_for_interrupt(this, &this->interrupt.for_reset,
					 this->flash.default_timeout_ms);
	LOCK(&this->lock);
	if (rc) {
		LOGE(this, "failed to get interrupt (rc=%d)\n", rc);
		goto error;
	}

error:
	snprintf(this->result_info, SYN_STRING_LENGTH,
		 "%s, family 0x%02x, fw rev 0x%02x.%02x, extra 0x%02x, %s\n",
		 clearpad_s(this->device_info.product_id,
			    HEADER_PRODUCT_ID_SIZE),
		 this->device_info.customer_family,
		 this->device_info.firmware_revision_major,
		 this->device_info.firmware_revision_minor,
		 this->device_info.firmware_revision_extra,
		 rc == 0 ? "succeeded fw update" : "failed fw update");
	this->flash_requested = false;
	HWLOGI(this, "result: %s", this->result_info);

	if (this->watchdog.enabled)
		clearpad_watchdog_update(this);

	if (rc) {
		/* inform disabled state */
		this->state = SYN_STATE_DISABLED;
		clearpad_reset(this, SYN_SWRESET, __func__);
		UNLOCK(&this->lock);
		rc = clearpad_wait_for_interrupt(this,
					&this->interrupt.for_reset,
					this->flash.default_timeout_ms);
		LOCK(&this->lock);
		if (rc)
			LOGE(this, "failed to get interrupt (rc=%d)\n", rc);
	}
	return rc;
}

static ssize_t clearpad_state_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct clearpad_t *this = dev_get_drvdata(dev);
	struct clearpad_touchctrl_t *touchctrl = &this->touchctrl;

	if (!strncmp(attr->attr.name, __stringify(post_probe_start),
								PAGE_SIZE)) {
		snprintf(buf, PAGE_SIZE,
			"%d", this->post_probe.start);
		goto end;
	}

	if (!this->post_probe.done) {
		LOGW(this, "post_probe hasn't finished, please retry later\n");
		buf[0] = '\0';
		goto end;
	}

	if (!strncmp(attr->attr.name, __stringify(fwinfo), PAGE_SIZE)) {
		snprintf(buf, PAGE_SIZE,
			 "%s, family 0x%02x, fw rev 0x%02x.%02x, "
			 "extra 0x%02x, aid 0x%02x, state=%s, "
			 "active=%s, type=%s, icount=%u, session=%s, "
			 "power(touch=%s display=%s user=%d)\n",
			 clearpad_s(this->device_info.product_id,
				    HEADER_PRODUCT_ID_SIZE),
			 this->device_info.customer_family,
			 this->device_info.firmware_revision_major,
			 this->device_info.firmware_revision_minor,
			 this->device_info.firmware_revision_extra,
			 this->device_info.analog_id,
			 NAME_OF(clearpad_state_name, this->state),
			 this->dev_active ? "true" : "false",
			 NAME_OF(clearpad_chip_name, this->chip_id),
			 this->interrupt.count, touchctrl->session,
			 touchctrl_is_touch_powered(this) ? "OK" : "NG",
			 touchctrl_is_display_powered(this) ? "OK" : "NG",
			 touchctrl->power_user);
	} else if (!strncmp(attr->attr.name, __stringify(fwchip_id), PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%x", this->chip_id);
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
	else if (!strncmp(attr->attr.name, __stringify(fwstate), PAGE_SIZE))
		snprintf(buf, PAGE_SIZE, "%s",
			 NAME_OF(clearpad_state_name, this->state));
	else if (!strncmp(attr->attr.name, __stringify(wakeup_gesture),
		PAGE_SIZE))
		snprintf(buf, PAGE_SIZE, "%d",
			this->wakeup_gesture.enabled);
	else if (!strncmp(attr->attr.name, __stringify(pen), PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%d", this->pen.enabled);
	else if (!strncmp(attr->attr.name, __stringify(glove_mode), PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%d", this->glove.enabled);
	else if (!strncmp(attr->attr.name, __stringify(irq_enable), PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%d", this->force_sleep);
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
	else if (!strncmp(attr->attr.name, __stringify(stamina_mode),
								PAGE_SIZE))
		snprintf(buf, PAGE_SIZE,
			"%d", this->stamina.enabled);
	else
		snprintf(buf, PAGE_SIZE, "illegal sysfs file");
end:
	return strnlen(buf, PAGE_SIZE);
}

/*
 * Calibration
 */

/* need LOCK(&this->lock) */
static int clearpad_do_calibration(struct clearpad_t *this, int mode,
				   u8 calibration_crc)
{
	unsigned long timeout;
	int rc = 0;
	int reset_rc = 0;
	bool calibrate = false;
	u8 status, bit, need_bit;

	switch (mode) {
	case SYN_CALIBRATION_NORMAL:
		need_bit = CALIBRATION_STATE_CALIBRATION_CRC_MASK;
		calibrate = !!BIT_GET(calibration_crc,
				    CALIBRATION_STATE_CALIBRATION_CRC);
		bit = START_CAL_PROD_TEST_START_CALIBRATION_MASK;
		timeout = jiffies + msecs_to_jiffies(SYN_CALIBRATION_WAIT_MS);
		break;
	case SYN_CALIBRATION_EW:
		need_bit = CALIBRATION_STATE_IS_CALIBRATION_CRC_MASK;
		calibrate = !!BIT_GET(calibration_crc,
				    CALIBRATION_STATE_IS_CALIBRATION_CRC);
		if (!calibrate)
			break;
		bit = START_CAL_PROD_TEST_START_IS_CALIBRATION_MASK;
		timeout = jiffies +
			msecs_to_jiffies(SYN_CALIBRATION_EW_WAIT_MS);
		break;
	default:
		HWLOGI(this, "unknown mode %d\n", mode);
		rc = -EINVAL;
		goto end;
	}
	HWLOGI(this, "%s : %s\n", NAME_OF(clearpad_calibration_name, mode),
		     calibrate ? "Start" : "Skip");
	if (!calibrate)
		goto end;

	clearpad_set_delay(SYN_CALIBRATION_SETUP_TIME);
	/* start calibration */
	/* F54_ANALOG_CTRL188: Start Calibration or Production Test */
	rc = clearpad_put_bit(SYNF(this, F54_ANALOG, CTRL,
		this->reg_offset.f54_ctrl188), bit, bit);
	if (rc) {
		HWLOGE(this, "failed to start %s\n",
			      NAME_OF(clearpad_calibration_name, mode));
		goto end;
	}

	/* wait until end of calibration */
	do {
		clearpad_set_delay(SYN_CALIBRATION_WAIT);
		/* F54_ANALOG_CTRL188: Start Calibration or Production Test */
		rc = clearpad_get(SYNF(this, F54_ANALOG, CTRL,
			this->reg_offset.f54_ctrl188),
			&status);
		if (rc)
			HWLOGE(this, "failed to read status\n");
		if (!(status & bit))
			goto succeeded;
	} while (time_before(jiffies, timeout));
	HWLOGE(this, "calibration time out\n");
	rc = -EIO;
	goto end;

succeeded:
	rc = clearpad_get(SYNF(this, F54_ANALOG, DATA,
		this->reg_offset.f54_data31), &status);
	if (rc) {
		HWLOGE(this, "failed to get calibration status\n");
		goto end;
	}
	/* check calibration result */
	if (status & need_bit) {
		HWLOGE(this, "%s was failed, crc = %x\n",
		     NAME_OF(clearpad_calibration_name, mode), status);
		rc = -EIO;
	}
	HWLOGI(this, "%s : %s\n", NAME_OF(clearpad_calibration_name, mode),
		     status & need_bit ? "Failed" : "Succeed");

	clearpad_set_delay(20);

end:
	switch (mode) {
	case SYN_CALIBRATION_EW:
		if (!calibrate)
			break;
		clearpad_set_delay(SYN_CALIBRATION_BEFORE_HWRESET_WAIT);
		clearpad_reset(this, SYN_HWRESET, "Calibration(EW)");
		UNLOCK(&this->lock);
		reset_rc = clearpad_wait_for_interrupt(this,
					&this->interrupt.for_reset,
					this->interrupt.wait_ms);
		LOCK(&this->lock);
		if (reset_rc)
			LOGE(this, "failed to get interrupt (rc=%d)\n", rc);
		break;
	default:
		break;
	}

	return rc;
}

/* need LOCK(&this->lock) */
static int clearpad_calibrate_on_fwflash(struct clearpad_t *this,
					 bool fwflashed)
{
	struct clearpad_device_info_t *info = &this->device_info;
	int rc, i;
	u8 calibration_crc = 0;

	if (!clearpad_is_valid_function(this, SYN_F54_ANALOG)) {
		rc = -EIO;
		HWLOGE(this, "F54_ANALOG invalid\n");
		goto end;
	}

	HWLOGI(this, "old aid = %x, new aid = %x\n",
	     this->flash.analog_id, info->analog_id);
	/* check if needed */
	if (fwflashed && (this->flash.analog_id != info->analog_id)) {
		HWLOGW(this, "analog ID is not matching\n");
		BIT_SET(calibration_crc,
			CALIBRATION_STATE_CALIBRATION_CRC, 1);
		BIT_SET(calibration_crc,
			CALIBRATION_STATE_IS_CALIBRATION_CRC, 1);
	} else {
		rc = clearpad_get(SYNF(this, F54_ANALOG, DATA,
			this->reg_offset.f54_data31),
			&calibration_crc);
		if (rc) {
			HWLOGE(this, "failed to get calibration status\n");
			goto end;
		}
	}

	for (i = 0; i < 2; i++) {
		rc = clearpad_do_calibration(this, SYN_CALIBRATION_NORMAL,
					       calibration_crc);
		if (rc) {
			LOGE(this, "failed normal calibration\n");
			if (i > 0)
				goto end;
			clearpad_reset(this, SYN_SWRESET, __func__);
			UNLOCK(&this->lock);
			rc = clearpad_wait_for_interrupt(this,
					&this->interrupt.for_reset,
					this->interrupt.wait_ms);
			LOCK(&this->lock);
			if (rc)
				LOGE(this, "failed to get interrupt (rc=%d)\n",
									rc);
			continue;
		}
		break;
	}

	rc = clearpad_do_calibration(this, SYN_CALIBRATION_EW, calibration_crc);
	if (rc)
		LOGE(this, "failed ew calibration\n");

end:
	return rc;
}

/* need LOCK(&this->lock) */
static int clearpad_fwflash_core(struct clearpad_t *this,
				 enum clearpad_flash_command_e command,
				 unsigned int id)
{
	struct clearpad_flash_t *flash = &this->flash;
	int rc;
	bool do_update = false;

	if (!this->dev_active) {
		rc = -EINVAL;
		HWLOGW(this, "could not start fwflash because "
			"device is in suspended mode\n");
		goto err_in_device_mode;
	}

	if (this->flash.fw) {
		rc = -EAGAIN;
		HWLOGE(this, "previous fw update is ongoing\n");
		goto err_in_busy_check;
	}
	clearpad_firmware_reset(this);
	flash->command = command;

	rc = clearpad_judge_firmware_flash(this, id, command);
	if (rc < 0)
		goto err_in_judgement;

	do_update = (rc == 1) ? true : false;
	if (do_update) {
		/* enable irq inside flash_enable */
		rc = clearpad_command_fwflash(this);
		if (rc) {
			HWLOGE(this, "firmware flash was failed\n");
			goto err_in_fw_flash;
		}
	}

	if (this->calibration_supported && this->calibrate_on_fwflash) {
		rc = clearpad_calibrate_on_fwflash(this, do_update);
		if (rc) {
			HWLOGE(this, "calibrate fatal error\n");
			goto err_in_calibration;
		}
	}

err_in_calibration:
err_in_fw_flash:
err_in_judgement:
	clearpad_firmware_reset(this);
err_in_busy_check:
err_in_device_mode:
	return rc;
}

static ssize_t clearpad_fwflash_store(struct device *dev,
					       struct device_attribute *attr,
					       const char *buf, size_t size)
{
	struct clearpad_t *this = dev_get_drvdata(dev);
	const char **command = (const char **)clearpad_flash_command_name;
	const char *session = "fwflash store";
	int rc = 0;
	unsigned int id = 0;
	char request[SYN_STRING_LENGTH] = "";

	if (!this->post_probe.done) {
		LOGW(this, "post_probe hasn't finished, please retry later\n");
		goto err_in_check_post_probe;
	}

	HWLOGI(this, "flash command: %s\n", buf);
	if (sscanf(buf, "%7s %4x", request, &id) < 0) {
		HWLOGE(this, "%s sscanf failed ", buf);
		rc = -EINVAL;
		goto err_in_fw_flash_command_check;
	}
	HWLOGI(this, "request: %s\n", request);

	rc = clearpad_ctrl_session_begin(this, session);
	if (rc)
		goto err_in_ctrl_begin;

	LOCK(&this->lock);
	if (!strncmp(request, command[SYN_FORCE_FLASH], PAGE_SIZE)) {
		HWLOGI(this, "start force firmware flash, id=%x\n", id);
		rc = clearpad_fwflash_core(this, SYN_FORCE_FLASH, id);
	} else if (!strncmp(request, command[SYN_DEFAULT_FLASH], PAGE_SIZE)) {
		HWLOGI(this, "start firmware config & data\n");
		rc = clearpad_fwflash_core(this, SYN_DEFAULT_FLASH, 0);
	} else {
		HWLOGE(this, "illegal command\n");
		rc = -EINVAL;
	}
	UNLOCK(&this->lock);

	clearpad_ctrl_session_end(this, session);

err_in_ctrl_begin:
err_in_fw_flash_command_check:
err_in_check_post_probe:
	return rc ? rc : size;
}

static ssize_t clearpad_enabled_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct clearpad_t *this = dev_get_drvdata(dev);
	int rc = 0;

	LOGD(this, "start\n");

	if (!this->post_probe.done) {
		LOGW(this, "post_probe hasn't finished, please retry later\n");
		goto err_in_check_post_probe;
	}

	LOCK(&this->lock);

	if (sysfs_streq(buf, "1")) {
		rc = clearpad_gpio_export(this, &this->input->dev,
						false);
		if (rc)
			LOGE(this, "failed to unexport gpio\n");
		goto enable;
	} else if (sysfs_streq(buf, "0")) {
		devm_free_irq(&this->pdev->dev, this->irq, this);
		if (this->noise_det.supported)
			devm_free_irq(&this->pdev->dev,
				      this->noise_det.irq, this);
		rc = clearpad_gpio_export(this, &this->input->dev,
						true);
		if (rc) {
			LOGE(this, "failed to export gpio\n");
			goto enable;
		}
	}
	goto end;

enable:
	rc = devm_request_threaded_irq(&this->pdev->dev,
				this->irq,
				clearpad_hard_handler,
				clearpad_threaded_handler,
				IRQF_TRIGGER_LOW | IRQF_ONESHOT,
				this->pdev->dev.driver->name, this);
	if (rc)
		LOGE(this, "irq %d busy? <%d>\n", this->irq, rc);

	if (this->noise_det.supported) {
		rc = devm_request_threaded_irq(&this->pdev->dev,
					this->noise_det.irq,
					clearpad_noise_det_hard_handler,
					clearpad_noise_det_threaded_handler,
					IRQF_TRIGGER_RISING | IRQF_ONESHOT,
					"clearpad_noise_det", this);
		if (rc)
			LOGE(this, "noise_det irq %d busy? <%d>\n",
			       this->noise_det.irq, rc);
	}
end:
	UNLOCK(&this->lock);

err_in_check_post_probe:
	LOG_STAT(this, "state=%s\n", NAME_OF(clearpad_state_name, this->state));
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t clearpad_glove_enabled_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct clearpad_t *this = dev_get_drvdata(dev);
	const char *session = "glove enabled store";
	int rc = 0;
	bool lazy_update = false;

	LOGD(this, "start\n");

	if (!this->glove.supported) {
		LOGI(this, "glove mode is not supported");
		goto err_in_check_support;
	}

	if (!this->post_probe.done) {
		LOGI(this, "post_probe hasn't finished, will apply later\n");
		lazy_update = true;
		goto err_in_check_post_probe;
	}

	if (!this->dev_active || this->interrupt.count == 0) {
		LOGI(this, "avoid to access I2C before waiting %d ms delay\n",
				this->reset.delay_for_powerup_ms);
		lazy_update = true;
		goto not_ready_to_access_i2c;
	}

	rc = clearpad_ctrl_session_begin(this, session);
	if (rc) {
		LOGI(this, "not powered, will be applied later\n");
		lazy_update = true;
		goto err_in_session_begin;
	}

err_in_session_begin:
not_ready_to_access_i2c:
err_in_check_post_probe:
	LOCK(&this->lock);
	this->glove.enabled = sysfs_streq(buf, "0") ? false : true;
	if (!lazy_update) {
		rc = clearpad_set_glove_mode(this, this->glove.enabled);
		if (rc)
			LOGE(this, "failed to set glove for device\n");
	}
	LOGI(this, "glove mode: %s",
		    this->glove.enabled ? "ENABLE" : "DISABLE");
	UNLOCK(&this->lock);

	if (!lazy_update)
		clearpad_ctrl_session_end(this, session);

err_in_check_support:
	return size;
}

static ssize_t clearpad_pen_enabled_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct clearpad_t *this = dev_get_drvdata(dev);
	const char *session = "pen enabled store";
	int rc = 0;
	bool lazy_update = false;

	LOGD(this, "start\n");

	if (!this->pen.supported) {
		LOGI(this, "pen is not supported");
		goto err_in_check_support;
	}

	if (!this->post_probe.done) {
		LOGI(this, "post_probe hasn't finished, will apply later\n");
		lazy_update = true;
		goto err_in_check_post_probe;
	}

	if (!this->dev_active || this->interrupt.count == 0) {
		LOGI(this, "avoid to access I2C before waiting %d ms delay\n",
				this->reset.delay_for_powerup_ms);
		lazy_update = true;
		goto not_ready_to_access_i2c;
	}

	rc = clearpad_ctrl_session_begin(this, session);
	if (rc) {
		LOGI(this, "not powered, will be applied later\n");
		lazy_update = true;
		goto err_in_session_begin;
	}

err_in_session_begin:
not_ready_to_access_i2c:
err_in_check_post_probe:
	LOCK(&this->lock);
	this->pen.enabled = sysfs_streq(buf, "0") ? false : true;
	if (!lazy_update) {
		rc = clearpad_set_pen(this);
		if (rc)
			LOGE(this, "failed to set pen for device\n");
	}
	LOGI(this, "pen mode: %s", this->pen.enabled ? "ENABLE" : "DISABLE");
	UNLOCK(&this->lock);

	if (!lazy_update)
		clearpad_ctrl_session_end(this, session);

err_in_check_support:
	return size;
}

static ssize_t clearpad_wakeup_gesture_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	int rc = 0;
	const char *session = "wakeup gesture store";
	struct clearpad_t *this = dev_get_drvdata(dev);
	bool new = false;
	bool old;
	bool lazy_update = false;

	LOGD(this, "start\n");

	if (!this->wakeup_gesture.supported) {
		LOGI(this, "wakeup gesture is not supported");
		goto err_in_check_support;
	}

	if (!this->post_probe.done) {
		LOGI(this, "post_probe hasn't finished, will apply later\n");
		lazy_update = true;
		goto err_in_check_post_probe;
	}

	if (!this->dev_active || this->interrupt.count == 0) {
		LOGI(this, "avoid to access I2C before waiting %d ms delay\n",
				this->reset.delay_for_powerup_ms);
		lazy_update = true;
		goto not_ready_to_access_i2c;
	}

	rc = clearpad_ctrl_session_begin(this, session);
	if (rc) {
		LOGI(this, "not powered, will be applied later\n");
		lazy_update = true;
		goto err_in_session_begin;

	}

err_in_session_begin:
not_ready_to_access_i2c:
err_in_check_post_probe:
	LOCK(&this->lock);
	new = sysfs_streq(buf, "0") ? false : true;
	old = this->wakeup_gesture.enabled;
	this->wakeup_gesture.enabled = new;
	if (!lazy_update && !this->dev_active && old != new) {
		/* mode is changed in already suspended state */
		rc = clearpad_set_suspend_mode(this);
		if (rc)
			LOGE(this, "failed change suspend mode (rc=%d)\n", rc);

	}
	LOGI(this, "wakeup gesture: %s",
	     this->wakeup_gesture.enabled ? "ENABLE" : "DISABLE");

	UNLOCK(&this->lock);

	if (!lazy_update)
		clearpad_ctrl_session_end(this, session);

err_in_check_support:
	return size;
}

static ssize_t clearpad_force_sleep_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct clearpad_t *this = dev_get_drvdata(dev);
	const char *session = "force sleep store";
	int rc = 0;
	int value;
	enum clearpad_force_sleep_e old;
	bool lazy_update = false;

	LOGD(this, "start\n");

	if (!this->post_probe.done) {
		LOGI(this, "post_probe hasn't finished, will apply later\n");
		lazy_update = true;
		goto err_in_check_post_probe;
	}

	if (!this->dev_active || this->interrupt.count == 0) {
		LOGI(this, "avoid to access I2C before waiting %d ms delay\n",
				this->reset.delay_for_powerup_ms);
		lazy_update = true;
		goto not_ready_to_access_i2c;
	}

	rc = clearpad_ctrl_session_begin(this, session);
	if (rc) {
		LOGI(this, "not powered, will be applied later\n");
		lazy_update = true;
		goto err_in_session_begin;
	}

err_in_session_begin:
not_ready_to_access_i2c:
err_in_check_post_probe:
	LOCK(&this->lock);
	this->wakeup.unblank_done = false;
	this->wakeup.unblank_early_done = false;
	old = this->force_sleep;

	rc = kstrtoint(buf, 0, &value);
	if (rc) {
		LOGE(this, "failed to read force_sleep\n");
		goto err_in_value;
	}
	this->force_sleep = value;
	if (this->force_sleep < FSMODE_OFF ||
	    FSMODE_ONESHOT < this->force_sleep) {
		LOGE(this, "Invalid force_sleep mode\n");
		this->force_sleep = old;
		goto err_in_value;
	}

	if (old == FSMODE_ONESHOT && this->force_sleep == FSMODE_KEEP) {
		LOGW(this, "refuse KEEP mode from ONESHOT mode\n");
		this->force_sleep = old;
	}
	LOG_STAT(this, "force_sleep: %d\n", this->force_sleep);

	if (!lazy_update) {
		if (this->force_sleep == FSMODE_OFF &&
		    this->state == SYN_STATE_RUNNING &&
		    (!this->dev_active || this->early_suspend)) {
			rc = clearpad_set_resume_mode(this);
			if (rc)
				LOGE(this, "failed to set resume for device\n");
		} else if (this->force_sleep != FSMODE_OFF &&
			   this->state == SYN_STATE_RUNNING &&
			   this->dev_active) {
			rc = clearpad_set_early_suspend_mode(this);
			if (rc)
				LOGE(this, "failed to force sleep device\n");
		}
	}
err_in_value:
	UNLOCK(&this->lock);

	if (!lazy_update)
		clearpad_ctrl_session_end(this, session);

	return size;
}

static ssize_t clearpad_charger_status_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	int rc = 0;
	const char *session = "charger status store";
	struct clearpad_t *this = dev_get_drvdata(dev);
	bool lazy_update = false;

	LOGD(this, "start\n");

	if (!this->charger.supported) {
		LOGI(this, "charger mode is not supported\n");
		goto err_in_check_support;
	}

	if (!this->post_probe.done) {
		LOGI(this, "post_probe hasn't finished, will apply later\n");
		lazy_update = true;
		goto err_in_check_post_probe;
	}

	if (!this->dev_active || this->interrupt.count == 0) {
		LOGI(this, "avoid to access I2C before waiting %d ms delay\n",
				this->reset.delay_for_powerup_ms);
		lazy_update = true;
		goto not_ready_to_access_i2c;
	}

	rc = clearpad_ctrl_session_begin(this, session);
	if (rc) {
		LOGI(this, "not powered, will be applied later\n");
		lazy_update = true;
		goto err_in_session_begin;
	}

err_in_session_begin:
not_ready_to_access_i2c:
err_in_check_post_probe:
	LOCK(&this->lock);
	this->charger.status = sysfs_streq(buf, "0") ? false : true;
	if (!lazy_update) {
		rc = clearpad_set_charger(this);
		if (rc)
			LOGE(this, "failed to set charger for device\n");
	}
	LOGI(this, "charger status: %s\n",
	     this->charger.status ? "CONN" : "DISCONN");
	UNLOCK(&this->lock);

	clearpad_ctrl_session_end(this, session);

err_in_check_support:
	return size;
}

static ssize_t clearpad_cover_status_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	int rc = 0;
	const char *session = "cover status store";
	struct clearpad_t *this = dev_get_drvdata(dev);
	bool lazy_update = false;

	if (!this->cover.supported) {
		LOGI(this, "cover mode is not supported");
		goto err_in_check_support;
	}

	if (!this->post_probe.done) {
		LOGI(this, "post_probe hasn't finished, will apply later\n");
		lazy_update = true;
		goto err_in_check_post_probe;
	}

	if (!this->dev_active || this->interrupt.count == 0) {
		LOGI(this, "avoid to access I2C before waiting %d ms delay\n",
				this->reset.delay_for_powerup_ms);
		lazy_update = true;
		goto not_ready_to_access_i2c;
	}

	rc = clearpad_ctrl_session_begin(this, session);
	if (rc) {
		LOGI(this, "not powered, will be applied later\n");
		lazy_update = true;
		goto err_in_session_begin;
	}

err_in_session_begin:
not_ready_to_access_i2c:
err_in_check_post_probe:
	LOCK(&this->lock);
	this->cover.status = sysfs_streq(buf, "0") ? false : true;
	if (!lazy_update && this->cover.enabled) {
		rc = clearpad_set_cover_status(this);
		if (rc)
			LOGE(this, "failed to set cover status for device\n");
	}
	LOGI(this, "cover status: %s", this->cover.status ? "CLOSE" : "OPEN");
	UNLOCK(&this->lock);

	if (!lazy_update)
		clearpad_ctrl_session_end(this, session);

err_in_check_support:
	return size;
}

static ssize_t clearpad_cover_mode_enabled_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	int rc = 0;
	const char *session = "cover mode enabled store";
	struct clearpad_t *this = dev_get_drvdata(dev);
	bool lazy_update = false;

	if (!this->cover.supported) {
		LOGI(this, "cover mode is not supported");
		goto err_in_check_support;
	}

	if (!this->dev_active || this->interrupt.count == 0) {
		LOGI(this, "avoid to access I2C before waiting %d ms delay\n",
				this->reset.delay_for_powerup_ms);
		lazy_update = true;
		goto not_ready_to_access_i2c;
	}

	this->cover.enabled = sysfs_streq(buf, "0") ? false : true;

	if (!this->post_probe.done) {
		LOGI(this, "post_probe hasn't finished, will apply later\n");
		lazy_update = true;
		goto err_in_check_post_probe;
	}

	rc = clearpad_ctrl_session_begin(this, session);
	if (rc) {
		LOGI(this, "not powered, will be applied later\n");
		lazy_update = true;
		goto err_in_session_begin;
	}

err_in_session_begin:
not_ready_to_access_i2c:
err_in_check_post_probe:
	LOCK(&this->lock);
	this->cover.enabled = sysfs_streq(buf, "0") ? false : true;
	if (!lazy_update) {
		if (this->cover.enabled) {
			LOGI(this, "cover mode enabled\n");
		} else {
			this->cover.status = false;
			rc = clearpad_set_cover_status(this);
			if (rc)
				LOGE(this, "failed to set cover mode\n");
			LOGI(this, "cover mode disabled\n");
		}
	}
	UNLOCK(&this->lock);

	if (!lazy_update)
		clearpad_ctrl_session_end(this, session);

err_in_check_support:
	return size;
}

static ssize_t clearpad_cover_win_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	int win_size = 0;
	int win_size_org = 0;
	int rc = 0;
	const char *session = "cover win store";
	struct clearpad_t *this = dev_get_drvdata(dev);
	bool lazy_update = false;

	if (!this->cover.supported) {
		LOGI(this, "cover mode is not supported");
		goto err_in_check_support;
	}

	if (!this->post_probe.done) {
		LOGI(this, "post_probe hasn't finished, will apply later\n");
		lazy_update = true;
		goto err_in_check_post_probe;
	}

	if (!this->dev_active || this->interrupt.count == 0) {
		LOGI(this, "avoid to access I2C before waiting %d ms delay\n",
				this->reset.delay_for_powerup_ms);
		lazy_update = true;
		goto not_ready_to_access_i2c;
	}

	rc = clearpad_ctrl_session_begin(this, session);
	if (rc) {
		LOGI(this, "not powered, will be applied later\n");
		lazy_update = true;
		goto err_in_session_begin;
	}

err_in_session_begin:
not_ready_to_access_i2c:
err_in_check_post_probe:
	LOCK(&this->lock);
	if (kstrtoint(buf, 0, &win_size)) {
		LOGE(this, "failed to read %s", attr->attr.name);
		rc = -EINVAL;
		goto err_in_read_size;
	}
	win_size_org = win_size;

	if (!strncmp(attr->attr.name, "cover_win_top", PAGE_SIZE)) {
		if (this->cover.convert_window_size && this->cover.tag_y_max)
			win_size = win_size * (this->extents.preset_y_max + 1) /
							this->cover.tag_y_max;
		this->cover.win_top = win_size;
	} else if (!strncmp(attr->attr.name, "cover_win_bottom", PAGE_SIZE)) {
		if (this->cover.convert_window_size && this->cover.tag_y_max)
			win_size = win_size * (this->extents.preset_y_max + 1) /
							this->cover.tag_y_max;
		this->cover.win_bottom = win_size;
	} else if (!strncmp(attr->attr.name, "cover_win_right", PAGE_SIZE)) {
		if (this->cover.convert_window_size && this->cover.tag_x_max)
			win_size = win_size * (this->extents.preset_x_max + 1) /
							this->cover.tag_x_max;
		this->cover.win_right = win_size;
	} else if (!strncmp(attr->attr.name, "cover_win_left", PAGE_SIZE)) {
		if (this->cover.convert_window_size && this->cover.tag_x_max)
			win_size = win_size * (this->extents.preset_x_max + 1) /
							this->cover.tag_x_max;
		this->cover.win_left = win_size;
	}
	if (!lazy_update) {
		rc = clearpad_set_cover_window(this);
		if (rc)
			LOGE(this, "failed to set cover window for device\n");
	}
err_in_read_size:
	LOGI(this, "%s = %d (org %d) rc = %d\n",
	     attr->attr.name, win_size, win_size_org, rc);
	UNLOCK(&this->lock);

	if (!lazy_update)
		clearpad_ctrl_session_end(this, session);

err_in_check_support:
	return size;
}

static ssize_t clearpad_stamina_mode_enabled_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	int rc = 0;
	int value = 0;
	const char *session = "stamina mode enabled store";
	struct clearpad_t *this = dev_get_drvdata(dev);
	bool lazy_update = false;

	LOGD(this, "start\n");

	if (!this->stamina.supported) {
		LOGI(this, "stamina mode is not supported\n");
		goto err_in_check_support;
	}

	if (!this->post_probe.done) {
		LOGI(this, "post_probe hasn't finished, will apply later\n");
		lazy_update = true;
		goto err_in_check_post_probe;
	}

	if (!this->dev_active || this->interrupt.count == 0) {
		LOGI(this, "avoid to access I2C before waiting %d ms delay\n",
				this->reset.delay_for_powerup_ms);
		lazy_update = true;
		goto not_ready_to_access_i2c;
	}

	rc = clearpad_ctrl_session_begin(this, session);
	if (rc) {
		LOGI(this, "not powered, will be applied later\n");
		lazy_update = true;
		goto err_in_session_begin;
	}

err_in_session_begin:
err_in_check_post_probe:
not_ready_to_access_i2c:
	LOCK(&this->lock);
	if (kstrtoint(buf, 0, &value)) {
		LOGE(this, "failed to read %s", attr->attr.name);
		rc = -EINVAL;
		goto err_in_read_size;
	}
	switch (this->chip_id) {
	case SYN_CHIP_3330:
	case SYN_CHIP_332U:
	case SYN_CHIP_4353:
		if (value < 0 || 1 < value) {
			LOGE(this, "invalid stamina report rate mode %d\n",
					value);
			goto err_in_read_size;
		}
		if (value) {
			this->stamina.enabled = true;
			this->stamina.change_reportrate.mode = 1;
		} else {
			this->stamina.enabled = false;
			this->stamina.change_reportrate.mode = 0;
		}
		break;
	case SYN_CHIP_3500:
		if (value < 0 || 3 < value) {
			LOGE(this, "invalid stamina report rate mode %d\n",
					value);
			goto err_in_read_size;
		}
		if (value)
			this->stamina.enabled = true;
		else
			this->stamina.enabled = false;
		this->stamina.change_reportrate.mode = value;
		break;
	default:
		LOGE(this, "not supported on chip id 0x0%2x\n", this->chip_id);
		goto err_in_read_size;
	}
	if (!lazy_update) {
		rc = clearpad_set_stamina_mode(this);
		if (rc)
			LOGE(this, "failed to set stamina for device\n");
	}
err_in_read_size:
	LOGI(this, "stamina mode %s\n",
		this->stamina.enabled ? "enable" : "disable");
	LOGI(this, "change report rate mode %d\n",
		this->stamina.change_reportrate.mode);
	UNLOCK(&this->lock);

	if (!lazy_update)
		clearpad_ctrl_session_end(this, session);

err_in_check_support:
	return size;
}

static ssize_t clearpad_post_probe_start_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct clearpad_t *this = dev_get_drvdata(dev);

	LOGD(this, "start\n");

	LOCK(&this->lock);
	this->post_probe.start = sysfs_streq(buf, "0") ? false : true;
	if (this->post_probe.done) {
		HWLOGI(this, "already post probed, need reboot\n");
	} else {
		if (this->post_probe.start) {
			HWLOGI(this, "start post probe"
			       "with post_probe_start sysfs\n");
			schedule_delayed_work(&this->post_probe.work, 0);
		}
	}
	UNLOCK(&this->lock);

	return size;
}

static int clearpad_set_force_doze_active_mode(struct clearpad_t *this, u8 mode)
{
	int rc = 0;

	if (!this->stamina.force_doze_active.supported) {
		LOGI(this, "force doze/active mode is not supported\n");
		goto end;
	}
	if (mode > FORCE_DOZE_ACTIVE_MODE_DOZE) {
		LOGE(this, "invalid mode (%d)\n", mode);
		goto end;
	}
	this->stamina.force_doze_active.mode = mode;
	rc = clearpad_set_doze_holdoff(this);
	if (rc)
		LOGE(this, "failed to set Doze Holdoff\n");
end:
	return rc;
}

static void clearpad_get_force_doze_active_mode(struct clearpad_t *this)
{
	HWLOGI(this, "doze/active mode %d\n",
		this->stamina.force_doze_active.mode);
}

static ssize_t clearpad_debug_hwtest_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct clearpad_t *this = dev_get_drvdata(dev);
	struct clearpad_hwtest_t *hwt = &this->hwtest;
	ssize_t readable_size = min((ssize_t)hwt->log_size, (ssize_t)PAGE_SIZE - 1);

	LOCK(&hwt->lock);
	if (readable_size <= 0) {
		HWLOGE(this, "readable size <= 0\n");
		return 0;
	}
	memcpy(buf, &hwt->log_buf, readable_size);
	UNLOCK(&hwt->lock);

	return strnlen(buf, PAGE_SIZE);
}

static ssize_t clearpad_debug_hwtest_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct clearpad_t *this = dev_get_drvdata(dev);
	struct clearpad_hwtest_t *hwt = &this->hwtest;
	const char *session = "hwtest";
	int rc = 0;
	unsigned long arg;
	u8 page, reg, value, length, id, rw, mode;
	int ms;
	char *bhead = NULL;
	char *b;
	char *guard;

	/* print debug_info if no argument as default without lock */
	if (size < HWTEST_SIZE_OF_COMMAND_PREFIX) {
		clearpad_debug_info(this);
		goto end;
	}

	LOCK(&hwt->lock);
	b = bhead = devm_kzalloc(&this->pdev->dev, size + 1, GFP_KERNEL);
	if (!b) {
		rc = -ENOMEM;
		goto err_in_devm_kzalloc;
	}
	memcpy(b, buf, size);

	guard = b + size;
	while (guard > b) {
		if (isascii(guard[-1]) && isgraph(guard[-1]))
			break;
		guard[-1] = '\0';
		guard--; /* remove garbages */
	}


	/* init hwtest log_buf with command */
	hwt->log_size =
		scnprintf(hwt->log_buf, sizeof(hwt->log_buf), "%s\n", b);

	switch (DEBUG_COMMAND(b[0], b[1])) {
	case DEBUG_COMMAND('R', 'B'):
	case DEBUG_COMMAND('R', '0'):
		/* RB/R0[2:page][2:reg] */
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
	case DEBUG_COMMAND('W', 'B'):
	case DEBUG_COMMAND('W', '0'):
		/* WB/W0[2:page][2:reg][2:value] */
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
		rc = clearpad_debug_write_packet(this, page, reg,
						 length, b, guard);
		break;
	case DEBUG_COMMAND('A', '0'):
		/* A0[2:test type][2:count] */
		b += HWTEST_SIZE_OF_COMMAND_PREFIX;
		rc = kstrtoul(b, 16, &arg);
		if (rc)
			goto err_invalid_arg;
		if (!clearpad_is_valid_function(this, SYN_F54_ANALOG)) {
			HWLOGE(this, "F54 is not supported\n");
			goto err_invalid_arg;
		}
		value = arg;
		reg = arg >> 8;
		if (clearpad_ctrl_session_begin(this, session) == 0) {
			clearpad_analog_test(this, SYN_F54_ANALOG, reg, value);
			clearpad_ctrl_session_end(this, session);
		}
		break;
	case DEBUG_COMMAND('R', 'H'):
		/* RH/PO - HW reset */
		if (clearpad_ctrl_session_begin(this, session) == 0) {
			LOCK(&this->lock);
			clearpad_reset(this, SYN_HWRESET, "HW reset cmd");
			UNLOCK(&this->lock);
			rc = clearpad_wait_for_interrupt(this,
						&this->interrupt.for_reset,
						this->interrupt.wait_ms);
			if (rc)
				LOGE(this, "failed to get interrupt (rc=%d)\n",
									rc);
			clearpad_ctrl_session_end(this, session);
		}
		break;
	case DEBUG_COMMAND('P', '0'):
	case DEBUG_COMMAND('R', 'F'):
		/* RH/PO - HW reset */
		if (clearpad_ctrl_session_begin(this, session) == 0) {
			LOCK(&this->lock);
			clearpad_reset(this, SYN_FORCE_HWRESET, "Force HW reset cmd");
			UNLOCK(&this->lock);
			rc = clearpad_wait_for_interrupt(this,
						&this->interrupt.for_reset,
						this->interrupt.wait_ms);
			if (rc)
				LOGE(this, "failed to get interrupt (rc=%d)\n",
									rc);
			clearpad_ctrl_session_end(this, session);
		}
		break;
	case DEBUG_COMMAND('R', 'S'):
	case DEBUG_COMMAND('P', '1'):
		/* RS/P1 - SW reset */
		if (clearpad_ctrl_session_begin(this, session) == 0) {
			LOCK(&this->lock);
			clearpad_reset(this, SYN_SWRESET, "SW reset cmd");
			UNLOCK(&this->lock);
			rc = clearpad_wait_for_interrupt(this,
						&this->interrupt.for_reset,
						this->interrupt.wait_ms);
			if (rc)
				LOGE(this, "failed to get interrupt (rc=%d)\n",
									rc);
			clearpad_ctrl_session_end(this, session);
		}
		break;
	case DEBUG_COMMAND('F', 'D'):
		/* FD - FW default flash */
		b += HWTEST_SIZE_OF_COMMAND_PREFIX;
		HWLOGI(this, "start default firmware flash\n");
		if (clearpad_ctrl_session_begin(this, session) == 0) {
			LOCK(&this->lock);
			rc = clearpad_fwflash_core(this, SYN_DEFAULT_FLASH, 0);
			UNLOCK(&this->lock);
			clearpad_ctrl_session_end(this, session);
		}
		break;
	case DEBUG_COMMAND('F', 'F'):
		/* FF[2:id] - FW force flash */
		b += HWTEST_SIZE_OF_COMMAND_PREFIX;
		if (clearpad_hextou8(this, &b, guard, &id)) {
			HWLOGE(this, "need module id for force flash\n");
			goto err_invalid_arg;
		}
		HWLOGI(this, "start force firmware flash\n");
		if (clearpad_ctrl_session_begin(this, session) == 0) {
			LOCK(&this->lock);
			rc = clearpad_fwflash_core(this, SYN_FORCE_FLASH, id);
			UNLOCK(&this->lock);
			clearpad_ctrl_session_end(this, session);
		}
		break;
	case DEBUG_COMMAND('G', 'N'):
	case DEBUG_COMMAND('C', '0'):
		/* GN/C0 - get normal calibration result */
		if (clearpad_ctrl_session_begin(this, session) == 0) {
			rc = clearpad_debug_get_calibration_result(this,
				CALIBRATION_STATE_CALIBRATION_CRC_MASK);
			clearpad_ctrl_session_end(this, session);
		}
		break;
	case DEBUG_COMMAND('G', 'E'):
	case DEBUG_COMMAND('C', '1'):
		/* GE/C1 - get EW calibration result */
		if (clearpad_ctrl_session_begin(this, session) == 0) {
			rc = clearpad_debug_get_calibration_result(this,
				CALIBRATION_STATE_IS_CALIBRATION_CRC_MASK);
			clearpad_ctrl_session_end(this, session);
		}
		break;
	case DEBUG_COMMAND('C', 'N'):
	case DEBUG_COMMAND('C', '2'):
		/* CN/C2 - execute normal calibration */
		if (clearpad_ctrl_session_begin(this, session) == 0) {
			LOCK(&this->lock);
			rc = clearpad_debug_do_calibration(this,
				SYN_CALIBRATION_NORMAL);
			UNLOCK(&this->lock);
			clearpad_ctrl_session_end(this, session);
		}
		break;
	case DEBUG_COMMAND('C', 'E'):
	case DEBUG_COMMAND('C', '3'):
		/* CE/C3 - execute EW calibration */
		if (clearpad_ctrl_session_begin(this, session) == 0) {
			LOCK(&this->lock);
			rc = clearpad_debug_do_calibration(this,
				SYN_CALIBRATION_EW);
			UNLOCK(&this->lock);
			clearpad_ctrl_session_end(this, session);
		}
		break;
	case DEBUG_COMMAND('X', 'W'):
		/* XW[number:interval(ms)] - watchdog (0:stop, else:start) */
		b += HWTEST_SIZE_OF_COMMAND_PREFIX;
		b = skip_spaces(b);
		cancel_delayed_work_sync(&this->watchdog.work);
		if (kstrtoint(b, 0, &ms) || ms == 0) {
			this->watchdog.enabled = false;
			HWLOGI(this, "stop watchdog\n");
		} else {
			this->watchdog.delay = msecs_to_jiffies(ms);
			this->watchdog.enabled = true;
			HWLOGI(this, "start watchdog (interval ms=%d)\n", ms);
			schedule_delayed_work(&this->watchdog.work,
					      this->watchdog.delay);
		}
		break;
	case DEBUG_COMMAND('X', 'R'):
		/* XR - registers */
		clearpad_debug_registers(this);
		break;
	case DEBUG_COMMAND('X', 'X'):
		/* XX - debug info */
		clearpad_debug_info(this);
		break;
	case DEBUG_COMMAND('D', 'A'):
		b += HWTEST_SIZE_OF_COMMAND_PREFIX;
		rc = clearpad_hextou8(this, &b, guard, &rw);
		if (rc) {
			LOGE(this, "read/write parameter is invalid\n");
			goto err_invalid_arg;
		}
		if (rw == FORCE_DOZE_ACTIVE_COMMAMD_GET) {
			clearpad_get_force_doze_active_mode(this);
		} else if (rw == FORCE_DOZE_ACTIVE_COMMAMD_SET) {
			rc = clearpad_hextou8(this, &b, guard, &mode);
			if (rc) {
				LOGE(this, "mode parameter is invalid\n");
				goto err_invalid_arg;
			}
			rc = clearpad_set_force_doze_active_mode(this, mode);
		} else {
			LOGE(this, "Invalid parameters\n");
			goto err_invalid_arg;
		}
		break;
	default:
		goto err_invalid_arg;
	}
	goto end_free;

err_invalid_arg:
	HWLOGE(this, "illegal command\n");
	rc = -EINVAL;
end_free:
	devm_kfree(&this->pdev->dev, bhead);
err_in_devm_kzalloc:
	UNLOCK(&hwt->lock);
end:
	return size;
}

static ssize_t clearpad_debug_lda_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct clearpad_t *this = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE,
		"[lockdown area] somc id 0x%02x supplier id 0x%02x "
			"chip id 0x%04x family id 0x%04x\n",
			this->lda.somc_id, this->lda.supplier_id,
			this->lda.chip_id, this->lda.family_id);
	return strnlen(buf, PAGE_SIZE);
}

static struct device_attribute clearpad_sysfs_attrs[] = {
	__ATTR(fwinfo, S_IRUGO, clearpad_state_show, 0),
	__ATTR(fwchip_id, S_IRUGO, clearpad_state_show, 0),
	__ATTR(fwfamily, S_IRUGO, clearpad_state_show, 0),
	__ATTR(fwrevision, S_IRUGO, clearpad_state_show, 0),
	__ATTR(fwrevision_minor, S_IRUGO, clearpad_state_show, 0),
	__ATTR(fwrevision_extra, S_IRUGO, clearpad_state_show, 0),
	__ATTR(fwstate, S_IRUGO, clearpad_state_show, 0),
	__ATTR(fwflash, S_IWUSR, 0, clearpad_fwflash_store),
	__ATTR(enabled, S_IWUSR, 0, clearpad_enabled_store),
	__ATTR(pen, S_IRUGO | S_IWUSR, clearpad_state_show,
				clearpad_pen_enabled_store),
	__ATTR(glove_mode, S_IRUGO | S_IWUSR, clearpad_state_show,
				clearpad_glove_enabled_store),
	__ATTR(irq_enable, S_IRUGO | S_IWUSR, clearpad_state_show,
				clearpad_force_sleep_store),

	__ATTR(charger_status, S_IRUGO | S_IWUSR, clearpad_state_show,
				clearpad_charger_status_store),
	__ATTR(cover_status, S_IRUGO | S_IWUSR,
				clearpad_state_show,
				clearpad_cover_status_store),
	__ATTR(cover_mode, S_IRUGO | S_IWUSR,
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
				clearpad_cover_win_store),
	__ATTR(stamina_mode, S_IRUGO | S_IWUSR,
				clearpad_state_show,
				clearpad_stamina_mode_enabled_store),
	__ATTR(post_probe_start, S_IRUGO | S_IWUSR,
				clearpad_state_show,
				clearpad_post_probe_start_store),
	__ATTR(hwtest, S_IRUGO | S_IWUSR,
				clearpad_debug_hwtest_show,
				clearpad_debug_hwtest_store),
	__ATTR(lda, S_IRUGO, clearpad_debug_lda_show, 0),
	__ATTR_NULL
};

static struct device_attribute clearpad_wakeup_gesture_attr =
	__ATTR(wakeup_gesture, S_IRUGO | S_IWUSR,
				clearpad_state_show,
				clearpad_wakeup_gesture_store);

static int clearpad_create_sysfs_entries(struct clearpad_t *this,
				struct device_attribute *attrs)
{
	int i, rc = 0;

	for (i = 0; attrs[i].attr.name; i++) {
		rc = device_create_file(&this->input->dev, &attrs[i]);
		if (rc)
			goto err_in_create_file;
	}
	goto end;

err_in_create_file:
	for (i = i - 1 ; i >= 0; --i)
		device_remove_file(&this->input->dev, &attrs[i]);
end:
	return rc;
}

static void clearpad_remove_sysfs_entries(struct clearpad_t *this,
				struct device_attribute *attrs)
{
	int i;

	for (i = 0; attrs[i].attr.name; i++)
		device_remove_file(&this->input->dev, &attrs[i]);
}

static int clearpad_read_config_u8(struct device_node *devnode,
	char *conf_name, u8 *container)
{
	int rc = 0;
	u32 value;

	if (!of_property_read_u32(devnode, conf_name, &value))
		*container = (u8)value;
	else
		rc = -EINVAL;

	return rc;
}

static void clearpad_reg_offset_print(struct clearpad_t *this)
{
	/* F01_RMI */
	LOGD(this, "f01-cmd00:0x%02X\n", this->reg_offset.f01_cmd00);
	LOGD(this, "f01-ctrl00:0x%02X\n", this->reg_offset.f01_ctrl00);
	LOGD(this, "f01-ctrl01:0x%02X\n", this->reg_offset.f01_ctrl01);
	LOGD(this, "f01-ctrl05:0x%02X\n", this->reg_offset.f01_ctrl05);
	LOGD(this, "f01-ctrl18:0x%02X\n", this->reg_offset.f01_ctrl18);
	LOGD(this, "f01-data00:0x%02X\n", this->reg_offset.f01_data00);
	LOGD(this, "f01-data01:0x%02X\n", this->reg_offset.f01_data01);
	LOGD(this, "f01-query11:0x%02X\n", this->reg_offset.f01_query11);

	/* F02_2D */
	LOGD(this, "f12-ctrl08:0x%02X\n", this->reg_offset.f12_ctrl08);

	/* F34_FLASH */
	LOGD(this, "f34-ctrl00:0x%02X\n", this->reg_offset.f34_ctrl00);
	LOGD(this, "f34-data00:0x%02X\n", this->reg_offset.f34_data00);
	LOGD(this, "f34-data01:0x%02X\n", this->reg_offset.f34_data01);
	LOGD(this, "f34-data02:0x%02X\n", this->reg_offset.f34_data02);
	LOGD(this, "f34-data03:0x%02X\n", this->reg_offset.f34_data03);
	LOGD(this, "f34-data04:0x%02X\n", this->reg_offset.f34_data04);
	LOGD(this, "f34-data05:0x%02X\n", this->reg_offset.f34_data05);
	LOGD(this, "f34-query00:0x%02X\n", this->reg_offset.f34_query00);
	LOGD(this, "f34-query01:0x%02X\n", this->reg_offset.f34_query01);
	LOGD(this, "f34-query03:0x%02X\n", this->reg_offset.f34_query03);

	/* F51_CUSTOM */
	LOGD(this, "f51-ctrl05:0x%02X\n", this->reg_offset.f51_ctrl05);
	LOGD(this, "f51-ctrl30:0x%02X\n", this->reg_offset.f51_ctrl30);

	/* F54_ANALOG */
	LOGD(this, "f54-cmd00:0x%02X\n", this->reg_offset.f54_cmd00);
	LOGD(this, "f54-ctrl41:0x%02X\n", this->reg_offset.f54_ctrl41);
	LOGD(this, "f54-ctrl57:0x%02X\n", this->reg_offset.f54_ctrl57);
	LOGD(this, "f54-ctrl88:0x%02X\n", this->reg_offset.f54_ctrl88);
	LOGD(this, "f54-ctrl109:0x%02X\n", this->reg_offset.f54_ctrl109);
	LOGD(this, "f54-ctrl113:0x%02X\n", this->reg_offset.f54_ctrl113);
	LOGD(this, "f54-ctrl147:0x%02X\n", this->reg_offset.f54_ctrl147);
	LOGD(this, "f54-ctrl149:0x%02X\n", this->reg_offset.f54_ctrl149);
	LOGD(this, "f54-ctrl188:0x%02X\n", this->reg_offset.f54_ctrl188);
	LOGD(this, "f54-ctrl214:0x%02X\n", this->reg_offset.f54_ctrl214);
	LOGD(this, "f54-data00:0x%02X\n", this->reg_offset.f54_data00);
	LOGD(this, "f54-data01:0x%02X\n", this->reg_offset.f54_data01);
	LOGD(this, "f54-data02:0x%02X\n", this->reg_offset.f54_data02);
	LOGD(this, "f54-data03:0x%02X\n", this->reg_offset.f54_data03);
	LOGD(this, "f54-data31:0x%02X\n", this->reg_offset.f54_data31);
	LOGD(this, "f54-query38:0x%02X\n", this->reg_offset.f54_query38);
}

static void clearpad_reg_offset_dt(struct clearpad_t *this,
	 struct device_node *devnode)
{
	if (!devnode) {
		LOGW(this, "device node is invalid\n");
		return;
	}

	/* F01_RMI */
	clearpad_read_config_u8(devnode,
		"somc,clearpad-f01-rmi-cmd00", &this->reg_offset.f01_cmd00);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f01-rmi-ctrl00", &this->reg_offset.f01_ctrl00);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f01-rmi-ctrl01", &this->reg_offset.f01_ctrl01);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f01-rmi-ctrl05", &this->reg_offset.f01_ctrl05);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f01-rmi-ctrl18", &this->reg_offset.f01_ctrl18);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f01-rmi-data00", &this->reg_offset.f01_data00);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f01-rmi-data01", &this->reg_offset.f01_data01);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f01-rmi-query11", &this->reg_offset.f01_query11);

	/* F02_2D */
	clearpad_read_config_u8(devnode,
		"somc,clearpad-f12-2d-ctrl08", &this->reg_offset.f12_ctrl08);

	/* F34_FLASH */
	clearpad_read_config_u8(devnode,
		"somc,clearpad-f34-flash-ctrl00", &this->reg_offset.f34_ctrl00);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f34-flash-data00", &this->reg_offset.f34_data00);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f34-flash-data01", &this->reg_offset.f34_data01);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f34-flash-data02", &this->reg_offset.f34_data02);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f34-flash-data03", &this->reg_offset.f34_data03);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f34-flash-data04", &this->reg_offset.f34_data04);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f34-flash-data05", &this->reg_offset.f34_data05);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f34-flash-query00", &this->reg_offset.f34_query00);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f34-flash-query01", &this->reg_offset.f34_query01);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f34-flash-query03", &this->reg_offset.f34_query03);

	/* F51_CUSTOM */
	clearpad_read_config_u8(devnode,
		"somc,clearpad-f51-custom-ctrl05", &this->reg_offset.f51_ctrl05);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f51-custom-ctrl30", &this->reg_offset.f51_ctrl30);

	/* F54_ANALOG */
	clearpad_read_config_u8(devnode,
		"somc,clearpad-f54-analog-cmd00", &this->reg_offset.f54_cmd00);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f54-analog-ctrl41", &this->reg_offset.f54_ctrl41);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f54-analog-ctrl57", &this->reg_offset.f54_ctrl57);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f54-analog-ctrl88", &this->reg_offset.f54_ctrl88);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f54-analog-ctrl109", &this->reg_offset.f54_ctrl109);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f54-analog-ctrl113", &this->reg_offset.f54_ctrl113);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f54-analog-ctrl147", &this->reg_offset.f54_ctrl147);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f54-analog-ctrl149", &this->reg_offset.f54_ctrl149);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f54-analog-ctrl188", &this->reg_offset.f54_ctrl188);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f54-analog-ctrl214", &this->reg_offset.f54_ctrl214);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f54-analog-data00", &this->reg_offset.f54_data00);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f54-analog-data01", &this->reg_offset.f54_data01);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f54-analog-data02", &this->reg_offset.f54_data02);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f54-analog-data03", &this->reg_offset.f54_data03);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f54-analog-data31", &this->reg_offset.f54_data31);

	clearpad_read_config_u8(devnode,
		"somc,clearpad-f54-analog-query38", &this->reg_offset.f54_query38);
}

static void clearpad_reg_offset_config_dt(struct clearpad_t *this)
{
	struct device_node *devnode = this->bdata->of_node;

	clearpad_reg_offset_dt(this, devnode);
}

static void clearpad_reg_offset_config_dt_for_extra_id(
		struct clearpad_t *this,
		struct device_node *chip_node)
{
	struct device_node *extra_node = NULL;
	char extra_name[SYN_STRING_LENGTH];

	snprintf(extra_name, SYN_STRING_LENGTH,
		"EXTRA_0x%02X", this->device_info.firmware_revision_extra);

	extra_node = of_find_node_by_name(chip_node, extra_name);
	if (extra_node == NULL) {
		LOGE(this, "no settings for %s\n", extra_name);
		return;
	}

	LOGI(this, "read settings for %s\n", extra_name);

	clearpad_reg_offset_dt(this, extra_node);
	clearpad_reg_offset_print(this);

	this->reg_offset.updated = true;
}

static void clearpad_touch_config_dt_for_chip_id(struct clearpad_t *this,
						int chip_id)
{
	struct device_node *dev_node = this->bdata->of_node;
	struct device_node *chip_node = NULL;
	const char *chip_name;
	u32 value;

	if (chip_id)
		chip_name = NAME_OF(clearpad_chip_name, chip_id);
	else
		chip_name = "clearpad_default";
	if (chip_name == NULL) {
		LOGE(this, "unknown chip (0x%x)\n", chip_id);
		return;
	}
	chip_node = of_find_node_by_name(dev_node, chip_name);
	if (chip_node == NULL) {
		LOGE(this, "no settings for %s\n", chip_name);
		return;
	}

	LOGI(this, "read settings for %s\n", chip_name);

	if (of_property_read_u32(chip_node, "flash_default_timeout_ms", &value))
		LOGW(this, "no flash_default_timeout_ms\n");
	else
		this->flash.default_timeout_ms = (unsigned long)value;

	if (of_property_read_u32(chip_node, "calibrate_on_fwflash", &value))
		LOGW(this, "no calibrate_on_fwflash config\n");
	else
		this->calibrate_on_fwflash = value ? true : false;

	if (of_property_read_u32(chip_node, "calibration_supported", &value))
		LOGW(this, "no calibration_supported\n");
	else
		this->calibration_supported = value ? true : false;

	if (of_property_read_u32(chip_node, "hwreset_delay_for_powerup_ms",
				 &this->reset.delay_for_powerup_ms))
		LOGW(this, "no hwreset_delay_for_powerup_ms config\n");

	if (of_property_read_u32(chip_node, "interrupt_default_wait_ms",
				 &this->interrupt.wait_ms))
		LOGW(this, "no interrupt_default_wait_ms config\n");

	if (of_property_read_u32(chip_node, "charger_only_delay_ms", &value))
		LOGW(this, "no charger_only_delay_ms config\n");
	else
		this->charger_only.delay_ms = (unsigned long)value;

	if (!this->reg_offset.updated)
		clearpad_reg_offset_config_dt_for_extra_id(this, chip_node);
}

static int clearpad_touch_config_dt(struct clearpad_t *this)
{
	int rc = 0;
	struct device_node *devnode = this->bdata->of_node;
	u32 value;

	if (of_property_read_u32(devnode, "chip_id", &this->chip_id))
		LOGW(this, "no chip_id config\n");

	if (of_property_read_u32(devnode, "post_probe_start", &value))
		LOGW(this, "no post_probe_start config\n");
	else
		this->post_probe.start = value ? true : false;

	if (of_property_read_string(devnode, "synaptics,firmware_name",
		&this->flash.firmware_name))
		LOGW(this, "no firmware_name config\n");

	if (of_property_read_u32(devnode, "flash_on_post_probe", &value))
		LOGW(this, "no flash_on_post_probe config\n");
	else
		this->flash.on_post_probe = value ? true : false;

	if (of_property_read_u32(devnode, "flip_config", &this->flip_config))
		LOGW(this, "no flip_config config\n");

	if (of_property_read_u32(devnode, "charger_supported", &value))
		LOGW(this, "no charger_supported config\n");
	else
		this->charger.supported = value ? true : false;

	if (of_property_read_u32(devnode, "pen_supported", &value))
		LOGW(this, "no pen_supported config\n");
	else
		this->pen.supported = value ? true : false;

	if (of_property_read_u32(devnode, "glove_supported", &value))
		LOGW(this, "no glove_supported config\n");
	else
		this->glove.supported = value ? true : false;

	if (of_property_read_u32(devnode, "cover_supported", &value))
		LOGW(this, "no cover_supported config\n");
	else
		this->cover.supported = value ? true : false;

	if (of_property_read_u32(devnode, "cover_tag_x_max",
		&this->cover.tag_x_max))
		LOGW(this, "no cover_tag_x_max\n");

	if (of_property_read_u32(devnode, "cover_tag_y_max",
		&this->cover.tag_y_max))
		LOGW(this, "no cover_tag_y_max\n");

	if (of_property_read_u32(devnode, "convert_cover_win_size",
		&this->cover.convert_window_size))
		LOGW(this, "no convert_cover_win_size\n");

	if (of_property_read_u32(devnode, "touch_pressure_enabled",
		&this->touch_pressure_enabled))
		LOGW(this, "no touch_pressure_enabled\n");

	if (of_property_read_u32(devnode, "touch_size_enabled",
		&this->touch_size_enabled))
		LOGW(this, "no touch_size_enabled config\n");

	if (of_property_read_u32(devnode, "touch_orientation_enabled",
		&this->touch_orientation_enabled))
		LOGW(this, "no touch_orientation_enabled\n");

	if (of_property_read_u32(devnode, "preset_x_max",
		&this->extents.preset_x_max))
		LOGW(this, "no preset_x_max config\n");

	if (of_property_read_u32(devnode, "preset_y_max",
		&this->extents.preset_y_max))
		LOGW(this, "no preset_y_max config\n");

	if (of_property_read_u32(devnode, "preset_n_fingers",
		&this->extents.n_fingers))
		LOGW(this, "no preset_n_fingers config\n");

	if (of_property_read_u32(devnode, "wakeup_gesture_supported", &value))
		LOGW(this, "no wakeup_gesture_supported\n");
	else
		this->wakeup_gesture.supported = value ? true : false;

	if (of_property_read_u32(devnode, "wakeup_gesture_timeout",
		&this->wakeup_gesture.timeout_delay))
		LOGW(this, "no wakeup_gesture_timeout\n");

	if (of_property_read_u32(devnode,
		"wakeup_gesture_use_workaround_for_felica", &value))
		LOGW(this, "no wakeup_gesture_use_workaround_for_felica\n");
	else
		this->wakeup_gesture.use_workaround_for_felica =
							value ? true : false;

	if (of_property_read_u32(devnode, "watchdog_enabled", &value))
		LOGW(this, "no watchdog_enabled\n");
	else
		this->watchdog.enabled = value ? true : false;

	if (of_property_read_u32(devnode, "watchdog_delay_ms", &value)) {
		LOGW(this, "no watchdog_delay_ms, watchdog is disabled\n");
		this->watchdog.enabled = false;
	} else
		this->watchdog.delay = msecs_to_jiffies(value);

	if (of_find_property(devnode, "synaptics,irq_gpio_noise_det", NULL)) {
		this->noise_det.supported = true;
		this->noise_det.irq_gpio =
			of_get_named_gpio_flags(devnode,
				"synaptics,irq_gpio_noise_det", 0,
				&this->noise_det.irq_gpio_flags);
	} else {
		LOGI(this, "no synaptics,irq_gpio_noise_det config\n");
		this->noise_det.supported = false;
	}

	if (of_property_read_u32(devnode, "noise_det_retrytime_ms", &value)) {
		LOGI(this, "no noise_det_retrytime_ms and use default time\n");
		this->noise_det.retry_time_ms =
			SYN_RETRY_DEFAULT_TIME_FOR_NOISE_DET;
	} else {
		this->noise_det.retry_time_ms = value;
	}

	if (of_property_read_u32(devnode, "stamina_mode_supported", &value)) {
		LOGW(this, "no stamina_mode_supported config\n");
	} else {
		this->stamina.supported =
		(value & SYN_STAMINA_MODE_SUPPORTED_MASK) ? true : false;

		this->stamina.change_reportrate.supported =
		(value & SYN_STAMINA_REPROTRATE_SUPPORTED_MASK) ? true : false;

		this->stamina.doze_holdoff.supported =
		(value & SYN_STAMINA_DOZE_HOLDOFF_SUPPORTED_MASK)
			? true : false;

		this->stamina.force_doze_active.supported =
		(value & SYN_STAMINA_FORCE_DOZE_ACTIVE_MODE_SUPPORTED_MASK)
			? true : false;
	}

	if (of_property_read_u32(devnode, "doze_default_time", &value))
		LOGW(this, "no doze_default_time config\n");
	else
		this->stamina.doze_holdoff.default_time = (u8)value;

	if (of_property_read_u32(devnode, "doze_glove_mode_time", &value))
		LOGW(this, "no doze_glove_mode_time config\n");
	else
		this->stamina.doze_holdoff.glove_mode_time = (u8)value;

	if (of_property_read_u32(devnode, "doze_cover_mode_time", &value))
		LOGW(this, "no doze_cover_mode_time config\n");
	else
		this->stamina.doze_holdoff.cover_mode_time = (u8)value;

	if (of_property_read_u32(devnode, "doze_force_doze_mode_time", &value))
		LOGW(this, "no doze_force_doze_mode_time config\n");
	else
		this->stamina.doze_holdoff.force_doze_mode_time = (u8)value;

	if (of_property_read_u32(devnode, "access_test", &value))
		LOGW(this, "no access_test config\n");
	else
		this->access_test_enabled = value ? true : false;

	/* read default register offset params */
	clearpad_reg_offset_config_dt(this);

	/* set default chip id settings */
	clearpad_touch_config_dt_for_chip_id(this, 0);

	return rc;
}

static void clearpad_update_chip_id(struct clearpad_t *this)
{
	u8 *product_id = clearpad_s(this->device_info.product_id,
				    HEADER_PRODUCT_ID_SIZE);
	int id;

	for (id = 0; id < ARRAY_SIZE(clearpad_chip_name); id++) {
		if (clearpad_chip_name[id] == NULL)
			continue;
		if (is_equal_cstring(product_id, clearpad_chip_name[id])) {
			this->chip_id = id;
			goto update;
		}
	}
update:
	LOGI(this, "chip_id=0x%x\n", this->chip_id);
}

static int clearpad_input_init(struct clearpad_t *this)
{
	int rc = 0;

	if (this->input) {
		LOGI(this, "already input device has been allocated\n");
		goto end;
	}

	this->input = input_allocate_device();
	if (!this->input) {
		rc = -ENOMEM;
		goto end;
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

	LOGI(this, "touch area [%d, %d, %d, %d]\n",
	     this->extents.x_min, this->extents.y_min,
	     this->extents.preset_x_max, this->extents.preset_y_max);

	rc = input_register_device(this->input);
	if (rc) {
		LOGE(this, "failed to register device\n");
		input_set_drvdata(this->input, NULL);
		input_free_device(this->input);
		this->input = NULL;
		goto end;
	}

end:
	return rc;
}

static int clearpad_input_ev_init(struct clearpad_t *this)
{
	int rc = 0;

	if (this->evdt_node) {
		LOGI(this, "already input ev has been initialized\n");
		goto end;
	}

	if (this->wakeup_gesture.supported) {
		this->evdt_node = evdt_initialize(this->bdata->dev, this->input,
						SYN_WAKEUP_GESTURE);
		if (!this->evdt_node) {
			LOGE(this, "no wakeup_gesture dt\n");
			goto end;
		}

		rc = device_create_file(&this->input->dev,
					&clearpad_wakeup_gesture_attr);
		if (rc) {
			LOGE(this, "sysfs_create_file failed: %d\n", rc);
			this->evdt_node = NULL;
			goto end;
		}
		LOGI(this, "touch wakeup feature ok\n");
	}
end:
	return rc;
}

static int clearpad_pm_suspend(struct device *dev)
{
#ifdef CLEARPAD_WAKEUP_GESTURE
	struct clearpad_t *this = dev_get_drvdata(dev);

	HWLOGI(this, "pm suspend was called\n");

	if (device_may_wakeup(dev)) {
		enable_irq_wake(this->irq);
		HWLOGI(this, "enable irq wake");
	}

	HWLOGI(this, "pm suspend(active=%s)\n",
	       this->dev_active ? "true" : "false");
#endif
	return 0;
}

static int clearpad_pm_resume(struct device *dev)
{
#ifdef CLEARPAD_WAKEUP_GESTURE
	struct clearpad_t *this = dev_get_drvdata(dev);

	HWLOGI(this, "pm resume was called\n");
	if (device_may_wakeup(dev)) {
		disable_irq_wake(this->irq);
		HWLOGI(this, "disable irq wake");
	}

	HWLOGI(this, "pm resume(active=%s)\n",
	       this->dev_active ? "true" : "false");
#endif
	return 0;
}

/*
 * callback and display
 */

#if defined (CONFIG_FB) \
||         defined(CONFIG_DRM_SDE_SPECIFIC_PANEL)

/* need LOCK(&this->lock) */
static void clearpad_powerdown_core(struct clearpad_t *this, const char *id)
{
	struct clearpad_touchctrl_t *touchctrl = &this->touchctrl;
	struct timespec ts;
	bool already_powerdown;
	bool locked = false;

	get_monotonic_boottime(&ts);
	already_powerdown = this->touchctrl.will_powerdown;
	if (already_powerdown) {
		HWLOGI(this, "received %s again "
		       "(power=%s icount=%u) @ %ld.%06ld\n",
		       id, touchctrl_is_touch_powered(this) ? "OK" : "NG",
		       this->interrupt.count, ts.tv_sec, ts.tv_nsec);
		goto end;
	}
	HWLOGI(this, "%s (power=%s icount=%u) @ %ld.%06ld\n",
	       id, touchctrl_is_touch_powered(this) ? "OK" : "NG",
	       this->interrupt.count, ts.tv_sec, ts.tv_nsec);

	if (work_pending(&this->thread_resume.work))
		HWLOGI(this, "flushing pending thread_resume\n");
	UNLOCK(&this->lock);
	flush_workqueue(this->thread_resume.work_queue);
	LOCK(&this->lock);
	locked = touchctrl_lock_power(this, __func__, true, false);
	if (!locked)
		/* TODO consider this fatal error case */
		LOGE(this, "failed to lock power(user=%d), "
		     "might cause suspend error\n", touchctrl->power_user);

	this->touchctrl.will_powerdown = true;

	/* suspend mode is set if no user */
	if (locked)
		touchctrl_unlock_power(this, __func__);

end:
	return;
}

static void clearpad_cb_early_powerdown_handler(struct clearpad_t *this)
{
	LOCK(&this->lock);
	if (this->wakeup_gesture.enabled)
		goto end;

	clearpad_powerdown_core(this, "EARLY POWERDOWN");
end:
	this->wakeup.unblank_early_done = false;
	UNLOCK(&this->lock);
	return;
}

static void clearpad_cb_powerdown_handler(struct clearpad_t *this)
{
	LOCK(&this->lock);
	if (this->wakeup_gesture.enabled)
		clearpad_powerdown_core(this, "POWERDOWN");
	this->wakeup.unblank_done = false;
	UNLOCK(&this->lock);
	return;
}

static void clearpad_cb_early_unblank_handler(struct clearpad_t *this)
{
	struct timespec ts;
	unsigned long flags;

	if (this->wakeup.unblank_early_done)
		return;

	get_monotonic_boottime(&ts);
	LOCK(&this->lock);
	this->wakeup.unblank_early_done = true;

	if (this->touchctrl.will_powerdown && this->touchctrl.power_user == 0) {
		this->interrupt.count = 0;
		spin_lock_irqsave(&this->slock, flags);
		this->dev_busy = false;
		this->irq_pending = false;
		spin_unlock_irqrestore(&this->slock, flags);
	}

	this->touchctrl.will_powerdown = false;
	UNLOCK(&this->lock);
	HWLOGI(this, "EARLY UNBLANK @ %ld.%06ld\n", ts.tv_sec, ts.tv_nsec);
}

static void clearpad_cb_unblank_handler(struct clearpad_t *this)
{
	struct timespec ts;
	bool power = touchctrl_is_touch_powered(this);

	get_monotonic_boottime(&ts);
	HWLOGI(this, "UNBLANK (power=%s icount=%u active=%s) @ %ld.%06ld\n",
	       power ? "OK" : "NG",
	       this->interrupt.count,
	       this->dev_active ? "true" : "false",
	       ts.tv_sec, ts.tv_nsec);

	if (!power) {
		HWLOGI(this, "ignore UNBLANK event before power on\n");
		return;
	}

	if (this->wakeup.unblank_done)
		return;

	LOCK(&this->lock);
	this->wakeup.unblank_done = true;
	if (!this->post_probe.done) {
		HWLOGI(this, "ignore UNBLANK event before post probe\n");
		if (this->post_probe.start) {
			HWLOGI(this, "schedule post probe work for fail-safe\n");
			schedule_delayed_work(&this->post_probe.work, 0);
		}
		goto err_in_post_probe_done;
	}

	if (work_pending(&this->thread_resume.work))
		HWLOGI(this, "cb_unblank was called again before flushing\n");

	HWLOGI(this, "schedule for thread resume operation\n");
	queue_work(this->thread_resume.work_queue,
		   &this->thread_resume.work);

err_in_post_probe_done:
	UNLOCK(&this->lock);

	get_monotonic_boottime(&ts);
	HWLOGI(this, "end UNBLANK @ %ld.%06ld\n",
	       ts.tv_sec, ts.tv_nsec);
}
#endif

#if defined (CONFIG_FB) \
&&          !defined(CONFIG_DRM_SDE_SPECIFIC_PANEL)
static int clearpad_fb_notifier_callback(struct notifier_block *self,
				unsigned long event, void *data)
{
	struct fb_event *evdata = data;
	int blank;
	struct clearpad_t *this =
		container_of(self, struct clearpad_t, fb_notif);

	if (evdata && evdata->data) {
		if (event == FB_EARLY_EVENT_BLANK ||
		    event == FB_EXT_EARLY_EVENT_BLANK) {
			blank = *(int *)evdata->data;
			HWLOGI(this, "%s: %s\n",
				(event == FB_EARLY_EVENT_BLANK) ? "Early" :
					"ExtEarly",
				(blank == FB_BLANK_POWERDOWN) ? "Powerdown" :
				(blank == FB_BLANK_UNBLANK) ? "Unblank" :
				 "???");
			switch (blank) {
			case FB_BLANK_POWERDOWN:
				clearpad_cb_early_powerdown_handler(this);
				break;
			case FB_BLANK_UNBLANK:
				clearpad_cb_early_unblank_handler(this);
				break;
			default:
				break;
			}
		} else if (event == FB_EVENT_BLANK ||
			   event == FB_EXT_EVENT_BLANK) {
			blank = *(int *)evdata->data;
			HWLOGI(this, "%s: %s\n",
				(event == FB_EVENT_BLANK) ? "Blank" :
					"ExtBlank",
				(blank == FB_BLANK_POWERDOWN) ? "Powerdown" :
				(blank == FB_BLANK_UNBLANK) ? "Unblank" :
				 "???");
			switch (blank) {
			case FB_BLANK_POWERDOWN:
				clearpad_cb_powerdown_handler(this);
				break;
			case FB_BLANK_UNBLANK:
				clearpad_cb_unblank_handler(this);
			default:
				break;
			}
		}
	}
	return 0;
}

#endif

#ifdef CONFIG_DRM_SDE_SPECIFIC_PANEL
static int clearpad_drm_notifier_callback(struct notifier_block *self,
				unsigned long event, void *data)
{
	struct drm_ext_event *evdata = (struct drm_ext_event *)data;
	int blank;
	struct clearpad_t *this = container_of(self, struct clearpad_t, drm_notif);

	if (evdata && evdata->data) {
		if (event == DRM_EXT_EVENT_BEFORE_BLANK) {
			blank = *(int *)evdata->data;
			HWLOGI(this, "Before: %s\n",
				(blank == DRM_BLANK_POWERDOWN) ? "Powerdown" :
				(blank == DRM_BLANK_UNBLANK) ? "Unblank" :
				 "???");
			switch (blank) {
			case DRM_BLANK_POWERDOWN:
				clearpad_cb_early_powerdown_handler(this);
				if (clearpad_set_suspend_mode(this))
					LOGE(this, "failed to set suspend mode\n");
				if (this->watchdog.enabled)
					cancel_delayed_work(&this->watchdog.work);
				break;
			case DRM_BLANK_UNBLANK:
				clearpad_cb_early_unblank_handler(this);
				break;
			default:
				break;
			}
		} else if (event == DRM_EXT_EVENT_AFTER_BLANK) {
			blank = *(int *)evdata->data;
			HWLOGI(this, "After: %s\n",
				(blank == DRM_BLANK_POWERDOWN) ? "Powerdown" :
				(blank == DRM_BLANK_UNBLANK) ? "Unblank" :
				 "???");
			switch (blank) {
			case DRM_BLANK_POWERDOWN:
				clearpad_cb_powerdown_handler(this);
				break;
			case DRM_BLANK_UNBLANK:
				clearpad_cb_unblank_handler(this);
			default:
				break;
			}
		}
	}

	return 0;
}

#endif


/*
 * analog test
 */

static int clearpad_debug_hwtest_log(struct clearpad_t *this,
				     const char *format, ...)
{
	va_list args;
	struct clearpad_hwtest_t *hwt = &this->hwtest;
	int remain = sizeof(hwt->log_buf) - hwt->log_size;
	int length = 0;
	struct timespec ts;

	if (remain <= 1) {
		/* Rewind */
		get_monotonic_boottime(&ts);
		hwt->log_size =
			scnprintf(hwt->log_buf, sizeof(hwt->log_buf),
				  "Rewound @ %ld.%06ld\n",
				  ts.tv_sec, ts.tv_nsec);
		remain = sizeof(hwt->log_buf) - hwt->log_size;
		BUG_ON(remain <= 1);
	}

	va_start(args, format);
	length =  vscnprintf(hwt->log_buf + hwt->log_size, remain,
			     format, args);
	va_end(args);
	hwt->log_size += length;

	return length;
}

static void clearpad_analog_test_get_loop_count(struct clearpad_t *this,
		u8 mode, int num_tx, int num_rx, int *loop_count_i,
		int *loop_count_j, int *data_type)
{
	switch (mode) {
	case F54_16_IMAGE_REPORT:
	case F54_AUTOSCAN_REPORT:
	case F54_RAW_CAP_RX_COUPLING_REPORT:
	case F54_SENSOR_SPEED_REPORT:
	case F54_FULLINCELL_RAW_CAP_REPORT:
	case F54_FULLINCELL_CAL_DATA_CHK_REPORT:
	case F54_FULLINCELL_SENSOR_SPEED_REPORT:
	case F54_TRX_TO_TRX_SHORT_RAW_IMAGE_REPORT:
		*loop_count_i = num_tx;
		*loop_count_j = num_rx;
		*data_type = HWTEST_S16;
		break;
	case F54_HIGH_RESISTANCE_REPORT:
		*loop_count_i = HWTEST_SIZE_OF_ONE_DIMENSION;
		*loop_count_j = HWTEST_SIZE_OF_ONE_HIGH_RX;
		*data_type = HWTEST_S16;
		break;
	case F54_TRX_TO_TRX_SHORT_2_REPORT:
		if (this->chip_id == SYN_CHIP_3500)
			*loop_count_i = HWTEST_SIZE_OF_TRX_SHORT_2;
		else if (this->chip_id == SYN_CHIP_3330)
			*loop_count_i = HWTEST_SIZE_OF_TRX_SHORT_2 + 1;
		else
			*loop_count_i = HWTEST_SIZE_OF_TRX_SHORT_2_TAB;
		*loop_count_j = HWTEST_SIZE_OF_ONE_DIMENSION;
		*data_type = HWTEST_U8;
		break;
	case F54_ABS_RAW_REPORT:
		*loop_count_i = num_rx + num_tx;
		*loop_count_j = HWTEST_SIZE_OF_ONE_DIMENSION;
		*data_type = HWTEST_U32;
		break;
	case F54_EWMODE_RAW_CAP_REPORT:
		*loop_count_i = num_rx;
		*loop_count_j = HWTEST_SIZE_OF_ONE_DIMENSION;
		*data_type = HWTEST_U32;
		break;
	case F54_HIC_RX_SHORT_TEST_REPORT:
		*loop_count_i = num_rx;
		*loop_count_j = num_rx;
		*data_type = HWTEST_S16;
		break;
	default:
		*loop_count_i = *loop_count_j = *data_type = 0;
		break;
	}
	HWLOGI(this, "loop_count_i[%d], loop_count_j[%d], data_type[%d]\n",
	       *loop_count_i, *loop_count_j, *data_type);
}

static void clearpad_analog_test(struct clearpad_t *this,
		u8 f_analog, u8 mode, u8 count)
{
	int rc, i, j, k, len, num_tx = 0, num_rx = 0;
	int loop_count_i, loop_count_j, data_type, data_size;
	u8 buf[ANALOG_TEST_SIZE_TO_2D_ELECTRODES_USED], *data, *line, *pl, fw_rev_extra;
	const char delimeter[] = " | ";

	/* Handle unsupported test report */
	switch (this->chip_id) {
	case SYN_CHIP_3500:
		switch (mode) {
		case F54_16_IMAGE_REPORT:
		case F54_AUTOSCAN_REPORT:
		case F54_HIGH_RESISTANCE_REPORT:
		case F54_RAW_CAP_RX_COUPLING_REPORT:
		case F54_SENSOR_SPEED_REPORT:
		case F54_TRX_TO_TRX_SHORT_2_REPORT:
			break;
		default:
			HWLOGE(this, "analog test is not supported\n");
			goto end;
		}
		break;
	case SYN_CHIP_3330:
		switch (mode) {
		case F54_16_IMAGE_REPORT:
		case F54_AUTOSCAN_REPORT:
		case F54_HIGH_RESISTANCE_REPORT:
		case F54_RAW_CAP_RX_COUPLING_REPORT:
		case F54_SENSOR_SPEED_REPORT:
		case F54_TRX_TO_TRX_SHORT_2_REPORT:
		case F54_EWMODE_RAW_CAP_REPORT:
		case F54_ABS_RAW_REPORT:
		case F54_TRX_TO_TRX_SHORT_RAW_IMAGE_REPORT:
			break;
		default:
			HWLOGE(this, "analog test is not supported\n");
			goto end;
		}
		break;
	case SYN_CHIP_332U:
		switch (mode) {
		case F54_16_IMAGE_REPORT:
		case F54_FULLINCELL_RAW_CAP_REPORT:
		case F54_FULLINCELL_CAL_DATA_CHK_REPORT:
		case F54_FULLINCELL_SENSOR_SPEED_REPORT:
			break;
		default:
			HWLOGE(this, "analog test is not supported\n");
			goto end;
		}
		break;
	case SYN_CHIP_4353:
		switch (mode) {
		case F54_16_IMAGE_REPORT:
		case F54_RAW_CAP_RX_COUPLING_REPORT:
		case F54_SENSOR_SPEED_REPORT:
		case F54_ABS_RAW_REPORT:
		case F54_HIC_RX_SHORT_TEST_REPORT:
			break;
		default:
			HWLOGE(this, "mode %u is not supported\n", mode);
			goto end;
		}
		break;
	default:
		HWLOGE(this, "unsupported chip id\n");
		goto end;
	}

	memset(buf, 0, sizeof(buf));

	LOCK(&this->lock);

	rc = clearpad_get_block(SYNF(this, F12_2D, CTRL,
		this->reg_offset.f12_ctrl08),
				buf, ANALOG_TEST_SIZE_TO_2D_ELECTRODES_USED);
	if (rc)
		goto err_set_irq_xy;
	num_rx = buf[ANALOG_TEST_NUM_OF_2D_RECEIVERS];
	num_tx = buf[ANALOG_TEST_NUM_OF_2D_TRANSMITTERS];

	/* F01_RMI_CTRL00: Device Control */
	rc = clearpad_put_bit(SYNF(this, F01_RMI, CTRL,
		this->reg_offset.f01_ctrl00),
			      DEVICE_CONTROL_NO_SLEEP_MASK,
			      DEVICE_CONTROL_NO_SLEEP_MASK);
	if (rc)
		goto update_mode;

	/* Wait until sleep mode is completely changed to NO_SLEEP */
	clearpad_set_delay(100);

	/* F01_RMI_CTRL01: Interrupt Enable */
	rc = clearpad_put(SYNF(this, F01_RMI, CTRL,
		this->reg_offset.f01_ctrl01),
			this->pdt[f_analog].irq_mask);
	if (rc)
		goto unset_no_sleep_mode;

	fw_rev_extra = this->device_info.firmware_revision_extra;

	if (this->chip_id == SYN_CHIP_332U) {
		if (mode == F54_FULLINCELL_RAW_CAP_REPORT) {
			/* F54_ANALOG_CTRL188: Start Calibration or Prod Test */
			rc = clearpad_put_bit(
				SYNF(this, F54_ANALOG, CTRL,
				this->reg_offset.f54_ctrl188),
				0, START_CAL_PROD_TEST_SET_FREQUENCY_MASK);
			if (rc)
				goto err_set_irq_xy;
			/* F54_ANALOG_CTRL188: Start Calibration or Prod Test */
			rc = clearpad_put_bit(
				SYNF(this, F54_ANALOG, CTRL,
				this->reg_offset.f54_ctrl188),
				START_CAL_PROD_TEST_START_PROD_TEST_MASK,
				START_CAL_PROD_TEST_START_PROD_TEST_MASK);
			if (rc)
				goto err_set_irq_xy;
		}
	} else if (this->chip_id == SYN_CHIP_3500 ||
		   this->chip_id == SYN_CHIP_4353) {
		if (mode == F54_RAW_CAP_RX_COUPLING_REPORT ||
			mode == F54_HIGH_RESISTANCE_REPORT ||
			mode == F54_TRX_TO_TRX_SHORT_2_REPORT) {
			switch (fw_rev_extra) {
			case 0x01:
			case 0x02:
			case 0x03:
				/* F54_ANALOG_CTRL149: Trans CBC 2 */
				rc = clearpad_put_bit(
				SYNF(this, F54_ANALOG, CTRL,
				this->reg_offset.f54_ctrl149), 0,
				TRANS_CBC_2_TRANS_CBC_GLOBAL_CAP_MASK);
				if (rc)
					goto err_set_irq_xy;
				/* F54_ANALOG_CTRL88: Analog Control 1 */
				rc = clearpad_put_bit(
				SYNF(this, F54_ANALOG, CTRL,
				this->reg_offset.f54_ctrl88), 0,
				ANALOG_CONTROL_1_CBC_XMTR_CARRIER_SELECT_MASK);
				if (rc)
					goto err_set_irq_xy;
				/* F54_ANALOG_CTRL57: 0D CBC Settings */
				rc = clearpad_put_bit(
				SYNF(this, F54_ANALOG, CTRL,
				this->reg_offset.f54_ctrl57), 0,
				CBC_SETTINGS_XMTR_CARRIER_SELECT_MASK);
				if (rc)
					goto err_set_irq_xy;
				/* F54_ANALOG_CTRL41:
				   Multimetric Noise Control*/
				rc = clearpad_put_bit(
				SYNF(this, F54_ANALOG, CTRL,
				this->reg_offset.f54_ctrl41),
				MULTIMETRIC_NOISE_CTRL_NO_SIGNAL_CLARITY_MASK,
				MULTIMETRIC_NOISE_CTRL_NO_SIGNAL_CLARITY_MASK);
				if (rc)
					goto err_set_irq_xy;

				break;
			default:
				HWLOGE(this, "firmware is not supported\n");
				break;
			}
			/* F54_ANALOG_CMD00: Analog Command */
			rc = clearpad_put(
					SYNF(this, F54_ANALOG, COMMAND,
						this->reg_offset.f54_cmd00),
					ANALOG_COMMAND_FORCE_UPDATE_MASK);
			if (rc)
				goto err_set_irq_xy;
			rc = clearpad_get(SYNF(this, F54_ANALOG, COMMAND,
					this->reg_offset.f54_cmd00),
				buf);
			if (rc)
				goto err_set_irq_xy;
			for (i = 0;
			BIT_GET(buf[0], ANALOG_COMMAND_FORCE_UPDATE) != 0;
			i++) {
				clearpad_set_delay(
					SYN_WAIT_TIME_AFTER_REGISTER_ACCESS);
				rc = clearpad_get(
					SYNF(this, F54_ANALOG, COMMAND,
					this->reg_offset.f54_cmd00), buf);
				if (rc || i > 100)
					goto err_set_irq_xy;
				HWLOGI(this,
				       "force update flag = %x, loop = %d\n",
					buf[0], i);
			}
			/* F54_ANALOG_CMD00: Analog Command */
			rc = clearpad_put(
					SYNF(this, F54_ANALOG, COMMAND,
						this->reg_offset.f54_cmd00),
					ANALOG_COMMAND_FORCE_CALIBRATION_MASK);
			if (rc)
				goto err_set_irq_xy;
			rc = clearpad_get(SYNF(this, F54_ANALOG, COMMAND,
				this->reg_offset.f54_cmd00),
				buf);
			if (rc || BIT_GET(buf[0], ANALOG_COMMAND_GET_REPORT))
				goto err_set_irq_xy;
			for (i = 0;
			BIT_GET(buf[0], ANALOG_COMMAND_FORCE_CALIBRATION) != 0;
			i++) {
				clearpad_set_delay(
					SYN_WAIT_TIME_AFTER_REGISTER_ACCESS);
				rc = clearpad_get(
					SYNF(this, F54_ANALOG, COMMAND,
					this->reg_offset.f54_cmd00), buf);
				if (rc || i > 100)
					goto err_set_irq_xy;
				HWLOGI(this, "force cal flag = %x, loop = %d\n",
				       buf[0], i);
			}
		}
	}

	clearpad_analog_test_get_loop_count(this, mode, num_tx, num_rx,
				&loop_count_i, &loop_count_j, &data_type);
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
		HWLOGE(this, "unsupported command\n");
		goto err_set_irq_xy;
	}

	data = devm_kzalloc(&this->pdev->dev, loop_count_i * loop_count_j * data_size,
							   GFP_KERNEL);
	if (!data)
		goto err_set_irq_xy;
	len = HWTEST_MAX_DIGITS + 1;
	line = devm_kzalloc(&this->pdev->dev,
			    loop_count_j * (len + sizeof(delimeter)),
			    GFP_KERNEL);
	if (!line)
		goto err_kfree_data;

	for (k = 0; k < count; k++) {
		s64 min_val = UINT_MAX, max_val = INT_MIN;

		HWLOGI(this, "ANALOG: mode[%d], num[%d], rx[%d], tx[%d]",
		       mode, k, num_rx, num_tx);

		rc = clearpad_put(SYNF(this, F54_ANALOG, DATA,
			this->reg_offset.f54_data01), 0x00);
		if (rc)
			goto err_reset;
		rc = clearpad_put(SYNF(this, F54_ANALOG, DATA,
			this->reg_offset.f54_data02), 0x00);
		if (rc)
			goto err_reset;
		rc = clearpad_put(SYNF(this, F54_ANALOG, DATA,
			this->reg_offset.f54_data00), mode);
		if (rc)
			goto err_reset;

		clearpad_prepare_for_interrupt(this, &this->interrupt.for_F54,
				"F54_ANALOG_CMD00 for analog test");

		/* F54_ANALOG_CMD00: Analog Command */
		rc = clearpad_put(
				SYNF(this, F54_ANALOG, COMMAND,
					this->reg_offset.f54_cmd00),
				ANALOG_COMMAND_GET_REPORT_MASK);
		if (rc) {
			clearpad_undo_prepared_interrupt(this,
				&this->interrupt.for_F54,
				"F54_ANALOG_CMD00 for analog test");
			goto err_reset;
		}

		UNLOCK(&this->lock);
		rc = clearpad_wait_for_interrupt(this,
					&this->interrupt.for_F54,
					this->interrupt.wait_ms);
		LOCK(&this->lock);
		if (rc) {
			LOGE(this, "failed to get interrupt (rc=%d)\n", rc);
			goto err_reset;
		}

		clearpad_set_delay(20);
		rc = clearpad_get_block(
			SYNF(this, F54_ANALOG, DATA, this->reg_offset.f54_data03),
				data, loop_count_i * loop_count_j * data_size);
		if (rc)
			goto err_reset;
		for (i = 0; i < loop_count_i; i++) {
			pl = line;
			for (j = 0; j < loop_count_j; j++) {
				s64 val = 0;

				switch (data_type) {
				case HWTEST_U8:
					val = (u8)(*(data + (i * loop_count_j) + j));
					break;
				case HWTEST_S8:
					val = (s8)(*(data + (i * loop_count_j) + j));
					break;
				case HWTEST_S16:
					val = (s16)le16_to_cpup(
					(const u16 *)(data + ((i * loop_count_j) + j) * data_size));
					break;
				case HWTEST_U32:
					val = (u32)le32_to_cpup(
					(const u32 *)(data + ((i * loop_count_j) + j) * data_size));
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
			HWLOGI(this, "%s\n", line);
		}
		HWLOGI(this, "MIN = %06lld / MAX = %06lld\n", min_val, max_val);
		clearpad_set_delay(100);
	}

err_reset:
	devm_kfree(&this->pdev->dev, line);
	devm_kfree(&this->pdev->dev, data);
	clearpad_reset(this, SYN_FORCE_SWRESET, __func__);
	UNLOCK(&this->lock);
	rc = clearpad_wait_for_interrupt(this,
					 &this->interrupt.for_reset,
					 this->interrupt.wait_ms);
	LOCK(&this->lock);
	if (rc)
		LOGE(this, "failed to get interrupt (rc=%d)\n", rc);
	goto update_mode;

err_kfree_data:
	devm_kfree(&this->pdev->dev, data);
err_set_irq_xy:
	if (clearpad_is_valid_function(this, SYN_F12_2D))
		clearpad_put(SYNF(this, F01_RMI, CTRL,
			this->reg_offset.f01_ctrl01),
			this->pdt[SYN_F12_2D].irq_mask);
unset_no_sleep_mode:
	/* F01_RMI_CTRL00: Device Control */
	clearpad_put_bit(SYNF(this, F01_RMI, CTRL,
		this->reg_offset.f01_ctrl00),
		0, DEVICE_CONTROL_NO_SLEEP_MASK);
update_mode:
	UNLOCK(&this->lock);
end:
	return;
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
		goto end;
	}
	strlcpy(s, p, DEBUG_ONE_BYTE_HEX + 1);
	p += DEBUG_ONE_BYTE_HEX;
	rc = kstrtoul(s, 16, &v);
	if (rc) {
		p = NULL;
		goto end;
	}
	*value = (u8)v;
	*ptr = skip_spaces(p);
end:
	return rc;
}

static int clearpad_debug_read_reg(struct clearpad_t *this,
						u8 page, u8 reg)
{
	int rc;
	u8 value;

	LOCK(&this->lock);
	rc = clearpad_get(this, SYN_PAGE_ADDR(page, reg), &value);
	if (!rc)
		HWLOGI(this, "read page=0x%02x, addr=0x%02x, value=0x%02x\n",
		       page, reg, value);
	else
		HWLOGE(this, "error in reading single register\n");
	UNLOCK(&this->lock);
	return rc;
}

static int clearpad_debug_read_packet(struct clearpad_t *this,
					u8 page, u8 reg, u8 length)
{
	int rc, i;
	u8 *pkt;

	LOCK(&this->lock);
	pkt = devm_kzalloc(&this->pdev->dev, length, GFP_KERNEL);
	if (!pkt) {
		rc = -ENOMEM;
		goto end;
	}
	rc = clearpad_read_block(this, SYN_PAGE_ADDR(page, reg), pkt, length);
	if (rc > 0) {
		HWLOGI(this, "read page=0x%02x, addr=0x%02x\n", page, reg);
		for (i = 0; i < length; i++)
			HWLOGI(this, "index[%d]=0x%02x\n", i, pkt[i]);
		rc = 0;
	} else {
		HWLOGE(this, "error in reading packet register\n");
	}
	devm_kfree(&this->pdev->dev, pkt);
end:
	UNLOCK(&this->lock);
	return rc;
}

static int clearpad_debug_write_reg(struct clearpad_t *this,
					u8 page, u8 reg, u8 value)
{
	int rc;

	LOCK(&this->lock);
	rc = clearpad_put(this, SYN_PAGE_ADDR(page, reg), value);
	if (!rc)
		HWLOGI(this, "write page=0x%02x, addr=0x%02x, value=0x%02x\n",
		       page, reg, value);
	else
		HWLOGE(this, "error in writing to register\n");
	UNLOCK(&this->lock);
	return rc;
}

static int clearpad_debug_write_packet(struct clearpad_t *this,
		u8 page, u8 reg, u8 length, char *b, char *guard)
{
	u8 *pkt;
	u8 index, value;
	int rc, i;

	LOCK(&this->lock);
	pkt = devm_kzalloc(&this->pdev->dev, length, GFP_KERNEL);
	if (!pkt) {
		rc = -ENOMEM;
		goto end;
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
			HWLOGE(this, "invalid arguments > %s\n", b);
			rc = -EINVAL;
			goto err_free;
		}
		HWLOGI(this, "mod index[%d]=0x%02x(0x%02x)\n",
		       index, value, pkt[index]);
		pkt[index] = value;
	}

	/* write back packet*/
	rc = clearpad_put_block(this, SYN_PAGE_ADDR(page, reg), pkt, length);
	if (rc) {
		HWLOGE(this, "error in writing to pkt register\n");
		goto err_free;
	}

	HWLOGI(this, "write page=0x%02x, addr=0x%02x\n", page, reg);
	for (i = 0; i < length; i++)
		HWLOGI(this, "index[%d]=0x%02x\n", i, pkt[i]);
	rc = 0;

err_free:
	devm_kfree(&this->pdev->dev, pkt);
end:
	UNLOCK(&this->lock);
	return rc;
}

static int clearpad_debug_get_calibration_result(struct clearpad_t *this,
						 u8 check_mode)
{
	int rc = 0;
	u8 buf;

	if (!this->calibration_supported) {
		HWLOGE(this, "%s doesn't support calibration\n",
		       NAME_OF(clearpad_chip_name, this->chip_id));
		goto end;
	}

	if (!clearpad_is_valid_function(this, SYN_F54_ANALOG)) {
		rc = -EIO;
		HWLOGE(this, "F54_ANALOG invalid\n");
		goto end;
	}
	LOCK(&this->lock);
	rc = clearpad_get(SYNF(this, F54_ANALOG, DATA,
		this->reg_offset.f54_data31), &buf);
	UNLOCK(&this->lock);
	if (rc) {
		HWLOGE(this, "failed to get CRC\n");
		goto end;
	}

	HWLOGI(this, "CRC = 0x%02x\n", buf);

	if (buf & check_mode) {
		rc = -EIO;
		HWLOGE(this, "calibration is not successful\n");
	}
end:
	return rc;
}

/* need LOCK(&this->lock) */
static int clearpad_debug_do_calibration(struct clearpad_t *this, int mode)
{
	int rc = 0;

	if (!this->calibration_supported) {
		HWLOGE(this, "%s doesn't support calibration\n",
		       NAME_OF(clearpad_chip_name, this->chip_id));
		goto end;
	}

	if (!clearpad_is_valid_function(this, SYN_F54_ANALOG)) {
		rc = -EIO;
		HWLOGE(this, "F54_ANALOG invalid\n");
		goto end;
	}

	rc = clearpad_do_calibration(this, mode,
				     CALIBRATION_STATE_CALIBRATION_CRC_MASK |
				     CALIBRATION_STATE_IS_CALIBRATION_CRC_MASK);
end:
	return rc;
}

static int clearpad_debug_print_reg(const char *desc, struct clearpad_t *this,
			      u16 addr, u8 *buf, int length)
{
	int rc;
	int i;

	rc = clearpad_get_block(this, addr, buf, length);
	if (rc == 0)
		for (i = 0; i < length; i++)
			HWLOGI(this, "[0x%04x]/%02d 0x%02x %s\n",
				addr, i, buf[i], i == 0 ? desc : "");
	else
		HWLOGI(this, "[0x%04x] N/A %s\n", addr, desc);

	return rc;
}

static void clearpad_debug_registers(struct clearpad_t *this)
{
	u8 buf[80];

	LOCK(&this->lock);
	if (!touchctrl_lock_power(this, __func__, true, false)) {
		LOGI(this, "No register info due to no power\n");
		goto end;
	}

	if (!clearpad_is_valid_function(this, SYN_F34_FLASH))
		goto reg_F01_RMI;

	HWLOGI(this, "=== F34_FLASH ===\n");

	/* F34_FLASH_DATA00: Status */
	if (clearpad_debug_print_reg("F34_FLASH_DATA00: Status",
		     SYNF(this, F34_FLASH, DATA,
		     this->reg_offset.f34_data00), buf, 1) == 0) {
		HWLOGI(this, INDENT "Status = %d\n",
		       BIT_GET(buf[0], STATUS_FLASH_STATUS));
		HWLOGI(this, INDENT "Device Cfg Status = %d\n",
		       BIT_GET(buf[0], STATUS_DEVICE_CONFIG_STATUS));
		HWLOGI(this, INDENT "BL Mode = %d\n",
		       BIT_GET(buf[0], STATUS_BL_MODE));
	}

	/* F34_FLASH_QUERY01: Bootloader Revision */
	clearpad_debug_print_reg("F34_FLASH_QUERY01: Bootloader Revision",
		 SYNF(this, F34_FLASH, QUERY,
		 this->reg_offset.f34_query01), buf, 2);

	/* F34_FLASH_CTRL00: Customer Configuration ID */
	if (clearpad_debug_print_reg("F34_FLASH_CTRL00: Customer Config ID",
		     SYNF(this, F34_FLASH, CTRL,
		     this->reg_offset.f34_ctrl00), buf, 5) == 0)
		HWLOGI(this, INDENT "family=0x%02x rev=0x%02x.%02x "
		       "extra=0x%02x aid=0x%02x\n",
		       buf[0], buf[1], buf[2], buf[3], buf[4]);

reg_F01_RMI:
	if (!clearpad_is_valid_function(this, SYN_F01_RMI))
		goto reg_F12_2D;

	HWLOGI(this, "=== F01_RMI ===\n");

	/* F01_RMI_CTRL00: Device Control */
	if (clearpad_debug_print_reg("F01_RMI_CTRL00: Device Control",
		     SYNF(this, F01_RMI, CTRL,
		     this->reg_offset.f01_ctrl00), buf, 1) == 0) {
		HWLOGI(this, INDENT "Sleep Mode = %d\n",
		       BIT_GET(buf[0], DEVICE_CONTROL_SLEEP_MODE));
		HWLOGI(this, INDENT "No Sleep = %d\n",
		       BIT_GET(buf[0], DEVICE_CONTROL_NO_SLEEP));
		HWLOGI(this, INDENT "Charger Connected = %d\n",
		       BIT_GET(buf[0], DEVICE_CONTROL_CHARGER_CONNECTED));
		HWLOGI(this, INDENT "Report Rate = %d\n",
		       BIT_GET(buf[0], DEVICE_CONTROL_REPORT_RATE));
		HWLOGI(this, INDENT "Configured = %d\n",
		       BIT_GET(buf[0], DEVICE_CONTROL_CONFIGURED));
	}

	/* F01_RMI_CTRL01: Interrupt Enable 0 */
	clearpad_debug_print_reg("F01_RMI_CTRL01: Interrupt Enable 0",
		 SYNF(this, F01_RMI, CTRL,
		 this->reg_offset.f01_ctrl01), buf, 1);

	/* F01_RMI_CTRL18: Device Control 1 */
	clearpad_debug_print_reg("F01_RMI_CTRL18: Device Control 1",
		 SYNF(this, F01_RMI, CTRL,
		 this->reg_offset.f01_ctrl18), buf, 1);

	/* F01_RMI_CTRL05: Doze Holdoff */
	clearpad_debug_print_reg("F01_RMI_CTRL05: Doze Holdoff",
		 SYNF(this, F01_RMI, CTRL,
		 this->reg_offset.f01_ctrl05), buf, 1);

	/* F01_RMI_DATA00: Device Status */
	if (clearpad_debug_print_reg("F01_RMI_DATA00: Device Status",
		     SYNF(this, F01_RMI, DATA,
		     this->reg_offset.f01_data00), buf, 1) == 0) {
		HWLOGI(this, INDENT "Status Code = %d\n",
		       BIT_GET(buf[0], DEVICE_STATUS_CODE));
		HWLOGI(this, INDENT "Flash Prog = %d\n",
		       BIT_GET(buf[0], DEVICE_STATUS_FLASH_PROG));
		HWLOGI(this, INDENT "Unconfigured = %d\n",
		       BIT_GET(buf[0], DEVICE_STATUS_UNCONFIGURED));
	}

	/* F01_RMI_DATA01.00: Interrupt Status */
	clearpad_debug_print_reg("F01_RMI_DATA01.00: Interrupt Status",
		 SYNF(this, F01_RMI, DATA,
		 this->reg_offset.f01_data01), buf, 1);

reg_F12_2D:
	if (!clearpad_is_valid_function(this, SYN_F12_2D))
		goto reg_end;

	HWLOGI(this, "=== F12_2D ===\n");

	/* F12_2D_DATA15: Object Attention */
	clearpad_debug_print_reg("F12_2D_DATA15: Object Attention",
				 SYNA(this, F12_2D, DATA, 15), buf, 2);

	/* F12_2D_DATA01: Object Type and Status 0-9 */
	if (clearpad_debug_print_reg("F12_2D_DATA01: Obj Type and Status 0-9",
			SYNA(this, F12_2D, DATA, 01), buf, 8 * 10) == 0) {
		int i;
		u8 *b;

		for (i = 0; i < 10; i++) {
			b = &buf[i * 8];
			HWLOGI(this, INDENT "Object %2d: (x,y)=(%d,%d) "
			       "w=(%d,%d) z=%d t=%d\n",
			       i, b[1] + (b[2] << 8), b[3] + (b[4] << 8),
			       b[6], b[7], b[5], b[0]);
		}
	}

	/* F12_2D_QUERY00: General */
	clearpad_debug_print_reg("F12_2D_QUERY00: General",
		 SYNA(this, F12_2D, QUERY, 0), buf, 1);

	/* F12_2D_QUERY10: Supported Object Types */
	if (this->chip_id == SYN_CHIP_3330) {
		if (clearpad_debug_print_reg(
			"F12_2D_QUERY10: Supported Object Types",
			SYNA(this, F12_2D, QUERY, 10), buf, 1) == 0) {
			HWLOGI(this, INDENT "Gloved Finger = %d\n",
			       BIT_GET(buf[0],
			       SUPPORTED_OBJECT_TYPES_HAS_GLOVED_FINGER));
			HWLOGI(this, INDENT "Narrow Object = %d\n",
			       BIT_GET(buf[0],
			       SUPPORTED_OBJECT_TYPES_HAS_NARROW_OBJECT));
			HWLOGI(this, INDENT "Hand Edge = %d\n",
			       BIT_GET(buf[0],
			       SUPPORTED_OBJECT_TYPES_HAS_HAND_EDGE));
		}
	} else {
		if (clearpad_debug_print_reg(
			"F12_2D_QUERY10: Supported Object Types",
			SYNA(this, F12_2D, QUERY, 10), buf, 2) == 0) {
			HWLOGI(this, INDENT "Gloved Finger = %d\n",
			       BIT_GET(buf[0],
			       SUPPORTED_OBJECT_TYPES_HAS_GLOVED_FINGER));
			HWLOGI(this, INDENT "Cover = %d\n",
			       BIT_GET(buf[1],
			       SUPPORTED_OBJECT_TYPES_HAS_COVER));
			HWLOGI(this, INDENT "Stylus = %d\n",
			       BIT_GET(buf[1],
			       SUPPORTED_OBJECT_TYPES_HAS_STYLUS));
			HWLOGI(this, INDENT "Eraser = %d\n",
			       BIT_GET(buf[1],
			       SUPPORTED_OBJECT_TYPES_HAS_ERASER));
			HWLOGI(this, INDENT "Small Object = %d\n",
			       BIT_GET(buf[1],
			       SUPPORTED_OBJECT_TYPES_HAS_SMALL_OBJECT));
		}
	}

	/* F12_2D_CTRL20_01: Report Flags */
	if (clearpad_debug_print_reg("F12_2D_CTRL20_01: Report Flags",
	SYNA(this, F12_2D, CTRL, 20), buf, F12_2D_CTRL_RPT_REG_MAX) == 0) {
		HWLOGI(this, INDENT "Report Always = %d\n",
		       BIT_GET(buf[F12_2D_CTRL_RPT_FLAG],
			       REPORT_FLAGS_REPORT_ALWAYS));
		HWLOGI(this, INDENT "Report Wakeup Gesture Only = %d\n",
		       BIT_GET(buf[F12_2D_CTRL_RPT_FLAG],
			       REPORT_FLAGS_REPORT_WAKEUP_GESTURE_ONLY));
		HWLOGI(this, INDENT "Enable Dribble = %d\n",
		       BIT_GET(buf[F12_2D_CTRL_RPT_FLAG],
			       REPORT_FLAGS_ENABLE_DRIBBLE));
	}

	/* F12_2D_CTRL23_00: Object Report Enable */
	if (clearpad_debug_print_reg("F12_2D_CTRL23_00: Object Report Enable",
		     SYNA(this, F12_2D, CTRL, 23), buf, 1) == 0) {
		HWLOGI(this, INDENT "Finger = %d\n",
		       BIT_GET(buf[0], OBJECT_REPORT_ENABLE_FINGER));
		HWLOGI(this, INDENT "Stylus = %d\n",
		       BIT_GET(buf[0], OBJECT_REPORT_ENABLE_STYLUS));
		HWLOGI(this, INDENT "Palm = %d\n",
		       BIT_GET(buf[0], OBJECT_REPORT_ENABLE_PALM));
		HWLOGI(this, INDENT "Unclassified Object = %d\n",
		BIT_GET(buf[0], OBJECT_REPORT_ENABLE_UNCLASSIFIED_OBJECT));
		HWLOGI(this, INDENT "Hovering Finger = %d\n",
		       BIT_GET(buf[0], OBJECT_REPORT_ENABLE_HOVERING_FINGER));
		HWLOGI(this, INDENT "Gloved Finger = %d\n",
		       BIT_GET(buf[0], OBJECT_REPORT_ENABLE_GLOVED_FINGER));
		HWLOGI(this, INDENT "Narrow Object = %d\n",
		       BIT_GET(buf[0], OBJECT_REPORT_ENABLE_NARROW_OBJECT));
		HWLOGI(this, INDENT "Hand Edge = %d\n",
		       BIT_GET(buf[0], OBJECT_REPORT_ENABLE_HAND_EDGE));
	}

	/* F12_2D_CTRL23_02: Report As Finger */
	if (clearpad_debug_print_reg("F12_2D_CTRL23_02: Report As Finger",
		     SYNA(this, F12_2D, CTRL, 23), buf, 3) == 0) {
		HWLOGI(this, INDENT "Stylus = %d\n",
		       BIT_GET(buf[F12_2D_CTRL_REPORT_AS_FINGER],
			       REPORT_AS_FINGER_STYLUS));
		HWLOGI(this, INDENT "Palm = %d\n",
		       BIT_GET(buf[F12_2D_CTRL_REPORT_AS_FINGER],
			       REPORT_AS_FINGER_PALM));
		HWLOGI(this, INDENT "Unclassified Object = %d\n",
		       BIT_GET(buf[F12_2D_CTRL_REPORT_AS_FINGER],
			       REPORT_AS_FINGER_UNCLASSIFIED_OBJECT));
		HWLOGI(this, INDENT "Gloved Finger = %d\n",
		       BIT_GET(buf[F12_2D_CTRL_REPORT_AS_FINGER],
			       REPORT_AS_FINGER_GLOVED_FINGER));
		HWLOGI(this, INDENT "Narrow Object = %d\n",
		       BIT_GET(buf[F12_2D_CTRL_REPORT_AS_FINGER],
			       REPORT_AS_FINGER_NARROW_OBJECT));
		HWLOGI(this, INDENT "Hand Edge = %d\n",
		       BIT_GET(buf[F12_2D_CTRL_REPORT_AS_FINGER],
			       REPORT_AS_FINGER_HAND_EDGE));
	}

	if (this->chip_id == SYN_CHIP_3330 ||
	    this->chip_id == SYN_CHIP_332U) {
		/* F12_2D_CTRL25: Closed Xmin/Xmax/Ymin/Ymax */
		if (clearpad_debug_print_reg(
			     "F12_2D_CTRL25: Closed Xmin/Xmax/Ymin/Ymax",
			     SYNA(this, F12_2D, CTRL, 25), buf, 8) == 0) {
			HWLOGI(this, INDENT "Closed Cover Xmin = %d\n",
			       ((buf[1] << 8) | buf[0]));
			HWLOGI(this, INDENT "             Xmax = %d\n",
			       ((buf[3] << 8) | buf[2]));
			HWLOGI(this, INDENT "             Ymin = %d\n",
			       ((buf[5] << 8) | buf[4]));
			HWLOGI(this, INDENT "             Ymax = %d\n",
			       ((buf[7] << 8) | buf[6]));
		}
	}

	/* F12_2D_CTRL26: Feature Enable */
	if (clearpad_debug_print_reg("F12_2D_CTRL26: Feature Enable",
		     SYNA(this, F12_2D, CTRL, 26), buf, 1) == 0) {
		HWLOGI(this, INDENT "Glove Finger Detection = %d\n",
		BIT_GET(buf[0], FEATURE_ENABLE_ENABLE_GLOVED_FINGER_DETECTION));
		HWLOGI(this, INDENT "Closed Cover Detection = %d\n",
		BIT_GET(buf[0], FEATURE_ENABLE_ENABLE_CLOSED_COVER_DETECTION));
	}

	/* F12_2D_CTRL27_00: Wakeup Gesture Enable */
	if (clearpad_debug_print_reg("F12_2D_CTRL27_00: Wakeup Gesture Enable",
		     SYNA(this, F12_2D, CTRL, 27), buf, 1) == 0) {
		HWLOGI(this, INDENT "Double Tap = %d\n",
		       BIT_GET(buf[0], WAKEUP_GESTURE_ENABLE_DOUBLE_TAP));
		HWLOGI(this, INDENT "Swipe = %d\n",
		       BIT_GET(buf[0], WAKEUP_GESTURE_ENABLE_SWIPE));
		HWLOGI(this, INDENT "Tap And Hold = %d\n",
		       BIT_GET(buf[0], WAKEUP_GESTURE_ENABLE_TAP_AND_HOLD));
		HWLOGI(this, INDENT "Circle = %d\n",
		       BIT_GET(buf[0], WAKEUP_GESTURE_ENABLE_CIRCLE));
		HWLOGI(this, INDENT "Triangle = %d\n",
		       BIT_GET(buf[0], WAKEUP_GESTURE_ENABLE_TRIANGLE));
		HWLOGI(this, INDENT "Vee = %d\n",
		       BIT_GET(buf[0], WAKEUP_GESTURE_ENABLE_VEE));
		HWLOGI(this, INDENT "Unicode = %d\n",
		       BIT_GET(buf[0], WAKEUP_GESTURE_ENABLE_UNICODE));
	}

	/* F12_2D_CTRL33_00: Multi-Finger Moisture General */
	if (clearpad_debug_print_reg("F12_2D_CTRL33_00: Multi-Finger "
				     "Moisture General",
		     SYNA(this, F12_2D, CTRL, 33), buf, 1) == 0) {
		HWLOGI(this, INDENT "Enable Multifinger Moisture = %d\n",
		       BIT_GET(buf[0], ENABLE_MULTIFINGER_MOISTURE));
	}
	if (!clearpad_is_valid_function(this, SYN_F54_ANALOG))
		goto reg_end;

	HWLOGI(this, "=== F51_CUSTOM ===\n");

	if (this->chip_id == SYN_CHIP_3500) {
		/* F51_CUSTOM_CTRL30_06: Custom Report Rate */
		if (clearpad_debug_print_reg(
			"F51_CUSTOM_CTRL30_06: Custom Report Rate State",
			SYNF(this, F51_CUSTOM, CTRL,
			this->reg_offset.f51_ctrl30
			+ SYN_EXTERNAL_REPORT_RATE_OFFSET), buf, 1) == 0) {
			HWLOGI(this,
				INDENT "Custom Report Rate Mode = %d\n",
				*buf);
		}
	}

	HWLOGI(this, "=== F54_ANALOG ===\n");

	if (this->calibration_supported) {
		/* F54_ANALOG_DATA31: Calibration State */
		if (clearpad_debug_print_reg("F54_ANALOG_DATA31: Calibration "
					     "State",
			SYNF(this, F54_ANALOG, DATA,
			this->reg_offset.f54_data31), buf, 1) == 0) {
			HWLOGI(this, INDENT "Normal Calibration CRC = %d\n",
				BIT_GET(buf[0],
					CALIBRATION_STATE_CALIBRATION_CRC));
			HWLOGI(this, INDENT "Wake-up Gesture Calibration "
					    "CRC = %d\n",
				BIT_GET(buf[0],
					CALIBRATION_STATE_IS_CALIBRATION_CRC));
		}
	}

	/* F54_ANALOG_CMD00: Analog Command 0 */
	if (clearpad_debug_print_reg("F54_ANALOG_CMD00: Analog Command 0",
		     SYNF(this, F54_ANALOG, COMMAND,
		     this->reg_offset.f54_cmd00), buf, 1) == 0) {
		HWLOGI(this, INDENT "Force Cal = %d\n",
		       BIT_GET(buf[0], ANALOG_COMMAND_FORCE_CALIBRATION));
		HWLOGI(this, INDENT "Force Update = %d\n",
		       BIT_GET(buf[0], ANALOG_COMMAND_FORCE_UPDATE));
	}

	if (this->chip_id == SYN_CHIP_3330) {

		/* F54_ANALOG_CTRL109_00: General Control */
		if (clearpad_debug_print_reg("F54_ANALOG_CTRL109_00: General Control",
				SYNF(this, F54_ANALOG, CTRL,
				this->reg_offset.f54_ctrl109),
				buf, 1) == 0) {
			HWLOGI(this, INDENT "Baseline Correction Mode = %d\n",
				   BIT_GET(buf[0], BASELINE_CORRECTION_MODE));
		}

		/* F54_ANALOG_CTRL113_00: General Control */
		if (clearpad_debug_print_reg("F54_ANALOG_CTRL113_00: General Control",
				SYNF(this, F54_ANALOG, CTRL,
				this->reg_offset.f54_ctrl113),
				buf, 1) == 0) {
			HWLOGI(this, INDENT "Disable Hybrid Baseline = %d\n",
				   BIT_GET(buf[0], DISABLE_HYBRID_BASELINE));
		}

		/* F54_ANALOG_CTRL147_00: Disable Hybrid CBC Auto Correction */
		if (clearpad_debug_print_reg("F54_ANALOG_CTRL147_00: General Control",
				SYNF(this, F54_ANALOG, CTRL,
				this->reg_offset.f54_ctrl147),
				buf, 1) == 0) {
			HWLOGI(this, INDENT "Disable Hybrid CBC Auto Correction = %d\n",
				   BIT_GET(buf[0], DISABLE_HYBRID_CBC_AUTO_CORRECTION));
		}

		/* F54_ANALOG_CTRL214_00: General Control */
		if (clearpad_debug_print_reg("F54_ANALOG_CTRL214_00: General Control",
				SYNF(this, F54_ANALOG, CTRL,
				this->reg_offset.f54_ctrl214),
				buf, 1) == 0) {
			HWLOGI(this, INDENT "Enable Hybrid Charger noise "
						"Mitigation = %d\n",
				   BIT_GET(buf[0], ENABLE_HYBRID_CHARGER_NOISE_MITIGATION));
		}
	}

	clearpad_reg_offset_print(this);

reg_end:
	touchctrl_unlock_power(this, __func__);

end:
	UNLOCK(&this->lock);
	return;
}

static void clearpad_debug_info(struct clearpad_t *this)
{
	struct clearpad_touchctrl_t *touchctrl = &this->touchctrl;

	HWLOGI(this, "%s, family 0x%02x, fw rev 0x%02x.%02x, "
	       "extra 0x%02x, aid 0x%02x, state=%s, "
	       "active=%s, type=%s, irq=%s icount=%u\n",
	       clearpad_s(this->device_info.product_id, HEADER_PRODUCT_ID_SIZE),
	       this->device_info.customer_family,
	       this->device_info.firmware_revision_major,
	       this->device_info.firmware_revision_minor,
	       this->device_info.firmware_revision_extra,
	       this->device_info.analog_id,
	       NAME_OF(clearpad_state_name, this->state),
	       this->dev_active ? "true" : "false",
	       NAME_OF(clearpad_chip_name, this->chip_id),
	       is_irq_enabled(this) ? "enabled" : "disabled",
	       this->interrupt.count);

	/* clearpad_lockdown_area_t */
	HWLOGI(this, "[lockdown area] somc id 0x%02x supplier id 0x%02x "
	       "chip id 0x%04x family id 0x%04x\n",
	       this->lda.somc_id, this->lda.supplier_id,
	       this->lda.chip_id, this->lda.family_id);

	/* clearpad_post_probe_t */
	HWLOGI(this, "[post_probe] done=%s retry=%d\n",
	       this->post_probe.done ? "true" : "false",
	       this->post_probe.retry);

	/* clearpad_touchctrl_t */
	HWLOGI(this, "[power] touch=%s display=%s user=%d will_powerdown=%s\n",
	       touchctrl_is_touch_powered(this) ? "OK" : "NG",
	       touchctrl_is_display_powered(this) ? "OK" : "NG",
	       touchctrl->power_user,
	       touchctrl->will_powerdown ? "true" : "false");

	/* clearpad_interrupt_t */
	HWLOGI(this, "[interrupt] hard_handler @ %ld.%06ld "
	       "threaded_handler @ %ld.%06ld handle_first_event @ %ld.%06ld\n",
	       this->interrupt.hard_handler_ts.tv_sec,
	       this->interrupt.hard_handler_ts.tv_nsec,
	       this->interrupt.threaded_handler_ts.tv_sec,
	       this->interrupt.threaded_handler_ts.tv_nsec,
	       this->interrupt.handle_first_event_ts.tv_sec,
	       this->interrupt.handle_first_event_ts.tv_nsec);
	HWLOGI(this, INDENT "dev_busy=%s irq_pending=%s\n",
	       this->dev_busy ? "true" : "false",
	       this->irq_pending ? "true" : "false");
	HWLOGI(this, INDENT "reset(use=%s done=%s) "
	       "F34(use=%s done=%s) F54(use=%s done=%s)\n",
	       this->interrupt.for_reset.use ? "true" : "false",
	       atomic_read(&this->interrupt.for_reset.done) ? "true" : "false",
	       this->interrupt.for_F34.use ? "true" : "false",
	       atomic_read(&this->interrupt.for_F34.done) ? "true" : "false",
	       this->interrupt.for_F54.use ? "true" : "false",
	       atomic_read(&this->interrupt.for_F54.done) ? "true" : "false");

	/* clearpad_noise_detect_t */
	HWLOGI(this, "[noise_detect] supported=%s 1st_irq=%s"
	       " retry_time_ms=%d\n",
	       this->noise_det.supported ? "true" : "false",
	       this->noise_det.first_irq ? "true" : "false",
	       this->noise_det.retry_time_ms);
	HWLOGI(this, INDENT "enabled=%s hard_irq_c=%u threaded_irq_c=%u\n",
	       this->noise_det.enabled ? "true" : "false",
	       this->noise_det.hard_handler_count,
	       this->noise_det.threaded_handler_count);
	HWLOGI(this, INDENT "hard_handler @ %ld.%06ld "
	       "threaded_handler @ %ld.%06ld\n",
	       this->noise_det.hard_handler_ts.tv_sec,
	       this->noise_det.hard_handler_ts.tv_nsec,
	       this->noise_det.threaded_handler_ts.tv_sec,
	       this->noise_det.threaded_handler_ts.tv_nsec);

	/* locks */
	HWLOGI(this, "[lock] this->lock: %s owner(%s:%d @ %ld.%06ld)\n",
	       IS_LOCKED(&this->lock) ? "LOCKED" : "UNLOCKED",
	       this->lock.owner_func, this->lock.owner_line,
	       this->lock.ts.tv_sec, this->lock.ts.tv_nsec);
	HWLOGI(this, INDENT "session_lock: %s <%s> owner(%s:%d @ %ld.%06ld)\n",
	       IS_LOCKED(&touchctrl->session_lock)
	       ? "LOCKED" : "UNLOCKED", touchctrl->session,
	       touchctrl->session_lock.owner_func,
	       touchctrl->session_lock.owner_line,
	       touchctrl->session_lock.ts.tv_sec,
	       touchctrl->session_lock.ts.tv_nsec);
	HWLOGI(this, INDENT "hwtest.lock: %s owner(%s:%d @ %ld.%06ld)\n",
	       IS_LOCKED(&this->hwtest.lock) ? "LOCKED" : "UNLOCKED",
	       this->hwtest.lock.owner_func, this->hwtest.lock.owner_line,
	       this->hwtest.lock.ts.tv_sec, this->hwtest.lock.ts.tv_nsec);

	/* feature flags */
	HWLOGI(this, "[force_sleep] mode=%d\n", this->force_sleep);
	HWLOGI(this, "[charger] supported=%s status=%s\n",
	       this->charger.supported ? "true" : "false",
	       this->charger.status ? "true" : "false");
	HWLOGI(this, "[charger only] delay_ms=%lu\n",
	       this->charger_only.delay_ms);
	HWLOGI(this, "[stylus] supported=%s enabled=%s\n",
	       this->pen.supported ? "true" : "false",
	       this->pen.enabled ? "true" : "false");
	HWLOGI(this, "[glove] supported=%s enabled=%s\n",
	       this->glove.supported ? "true" : "false",
	       this->glove.enabled ? "true" : "false");
	HWLOGI(this, "[cover] supported=%s status=%s enabled=%s\n",
	       this->cover.supported ? "true" : "false",
	       this->cover.status ? "true" : "false",
	       this->cover.enabled ? "true" : "false");
	HWLOGI(this, INDENT "win top=%d bottom=%d right=%d left=%d\n",
	       this->cover.win_top, this->cover.win_bottom,
	       this->cover.win_right, this->cover.win_left);
	HWLOGI(this, INDENT "tag x_max=%u y_max=%u convert_window_size=%u\n",
	       this->cover.tag_x_max, this->cover.tag_y_max,
	       this->cover.convert_window_size);
	HWLOGI(this, "[wakeup_gesture] supported=%s enabled=%s\n",
	       this->wakeup_gesture.supported ? "true" : "false",
	       this->wakeup_gesture.enabled ? "true" : "false");
	HWLOGI(this, "[watchdog] enabled=%s delay=%d\n",
	       this->watchdog.enabled ? "true" : "false", this->watchdog.delay);
	HWLOGI(this, "[stamina_mode] supported=%s enabled=%s\n",
	       this->stamina.supported ? "true" : "false",
	       this->stamina.enabled ? "true" : "false");
	HWLOGI(this, INDENT "[change_reportrate] supported=%s mode=%d\n",
	       this->stamina.change_reportrate.supported ? "true" : "false",
	       this->stamina.change_reportrate.mode);
	HWLOGI(this, INDENT "[doze holdoff] supported=%s default_time=%u "
		"glove_mode_time=%u cover_mode_time=%u force_mode_time=%u\n",
	       this->stamina.doze_holdoff.supported ? "true" : "false",
	       this->stamina.doze_holdoff.default_time,
	       this->stamina.doze_holdoff.glove_mode_time,
	       this->stamina.doze_holdoff.cover_mode_time,
	       this->stamina.doze_holdoff.force_doze_mode_time);
	HWLOGI(this, INDENT"[force doze/active mode] supported=%s mode=%d\n",
	       this->stamina.force_doze_active.supported ? "true" : "false",
	       this->stamina.force_doze_active.mode);
	HWLOGI(this, "[early unblank] done=%s early_done=%s\n",
	       this->wakeup.unblank_done ? "true" : "false",
	       this->wakeup.unblank_early_done ? "true" : "false");
}

static int clearpad_read_lockdown_area(struct clearpad_t *this)
{
	int rc = 0;
	u8 data[LOCKDOWN_READ_SIZE];
	const int somcId_offset = 0x04;
	const int supplierId_offset = 0x05;
	const int chipId_upper_offset = 0x06;
	const int chipId_lower_offset = 0x07;
	const int familyId_upper_offset = 0x08;
	const int familyId_lower_offset = 0x09;

	if (this->chip_id != SYN_CHIP_4353) {
		HWLOGE(this, "not supported lockdown area\n");
		this->lda.family_id = LOCKDOWN_FAMILY_ID_MIN;
		goto end;
	}
	/* parse image file and PDT */
	rc = clearpad_read_pdt(this);
	if (rc) {
		HWLOGE(this, "failed to read pdt\n");
		goto end;
	}

	clearpad_prepare_for_interrupt(this, &this->interrupt.for_F34,
				       "F34_FLASH_DATA02 for read lockdown");
	rc = clearpad_put(SYNF(this, F34_FLASH, DATA,
			this->reg_offset.f34_data02),
			FLASH_CONTROL_READ_LOCKDOWN_BLOCK);
	if (rc) {
		HWLOGE(this, "unable to read lockdown\n");
		goto end;
	}
	UNLOCK(&this->lock);
	rc = clearpad_wait_for_interrupt(this, &this->interrupt.for_F34,
						this->interrupt.wait_ms);
	LOCK(&this->lock);
	if (rc) {
		HWLOGE(this, "wait for interrupt status failed %d\n", rc);
		goto end;
	}

	rc = clearpad_get_block(SYNF(this, F34_FLASH, DATA,
			this->reg_offset.f34_data01),
			data, LOCKDOWN_READ_SIZE);
	if (rc) {
		HWLOGE(this, "read data error\n");
		goto end;
	}

	memcpy(this->lda.data, &data[LOCKDOWN_DATA_OFFSET], LOCKDOWN_DATA_SIZE);
	HWLOGI(this, "lockdown data %02x %02x %02x %02x %02x %02x\n",
		this->lda.data[somcId_offset - LOCKDOWN_DATA_OFFSET],
		this->lda.data[supplierId_offset - LOCKDOWN_DATA_OFFSET],
		this->lda.data[chipId_upper_offset - LOCKDOWN_DATA_OFFSET],
		this->lda.data[chipId_lower_offset - LOCKDOWN_DATA_OFFSET],
		this->lda.data[familyId_upper_offset - LOCKDOWN_DATA_OFFSET],
		this->lda.data[familyId_lower_offset - LOCKDOWN_DATA_OFFSET]);

	this->lda.somc_id = data[somcId_offset];
	this->lda.supplier_id = data[supplierId_offset];
	this->lda.chip_id = data[chipId_upper_offset] << 8 |
			    data[chipId_lower_offset];
	this->lda.family_id = data[familyId_upper_offset] << 8 |
			      data[familyId_lower_offset];
	HWLOGI(this, "somc id 0x%02x supplier id 0x%02x "
	       "chip id 0x%04x family id 0x%04x\n",
	       this->lda.somc_id, this->lda.supplier_id,
	       this->lda.chip_id, this->lda.family_id);
end:
	return rc;
}

static unsigned int clearpad_get_lockdown_family_id(struct clearpad_t *this)
{
	HWLOGI(this, "module　id:0x%04x\n", this->lda.family_id);
	return this->lda.family_id;
}

static int clearpad_probe(struct platform_device *pdev)
{
	struct clearpad_data_t *cdata = pdev->dev.platform_data;
	struct clearpad_t *this;
	struct clearpad_touchctrl_t *touchctrl;
	struct kobject *parent;
	char *symlink_name;
	struct timespec ts;
	int rc;
	bool retry = false;
	struct platform_device *rmi_dev = NULL;
	char *path_name = "common_touch";

	this = devm_kzalloc(&pdev->dev, sizeof(struct clearpad_t), GFP_KERNEL);
	if (!this) {
		rc = -ENOMEM;
		retry = true;
		goto end;
	}
	/* Start logging for probe (no lock until end of probe) */
	get_monotonic_boottime(&ts);
	this->hwtest.log_size =
		scnprintf(this->hwtest.log_buf, sizeof(this->hwtest.log_buf),
			  "start probe @ %ld.%06ld\n", ts.tv_sec, ts.tv_nsec);

	this->state = SYN_STATE_INIT;
	this->wakeup.unblank_done = false;
	this->wakeup.unblank_early_done = false;
	mutex_init(&this->lock.lock);
	spin_lock_init(&this->slock);

	touchctrl = &this->touchctrl;
	mutex_init(&touchctrl->session_lock.lock);

	atomic_set(&this->interrupt.for_reset.done, 0);
	init_waitqueue_head(&this->interrupt.for_reset.wq);
	atomic_set(&this->interrupt.for_F34.done, 0);
	init_waitqueue_head(&this->interrupt.for_F34.wq);
	atomic_set(&this->interrupt.for_F54.done, 0);
	init_waitqueue_head(&this->interrupt.for_F54.wq);
	INIT_DELAYED_WORK(&this->reset.work, clearpad_reset_work);
	INIT_DELAYED_WORK(&this->post_probe.work, clearpad_post_probe_work);
	INIT_DELAYED_WORK(&this->watchdog.work, clearpad_watchdog_work);
	mutex_init(&this->hwtest.lock.lock);

	this->thread_resume.work_queue
		= create_workqueue("clearpad_thread_resume");
	if (!this->thread_resume.work_queue) {
		HWLOGE(this, "no create workqueue\n");
		rc = -EINVAL;
		goto err_work_queue;
	}
	INIT_WORK(&this->thread_resume.work, clearpad_thread_resume_work);

	dev_set_drvdata(&pdev->dev, this);
	/* LOGx is available after this */
	LOCK(&touchctrl->session_lock);
	this->pdev = pdev;
	this->pdata = cdata->pdata;
	if (!this->pdata) {
		HWLOGE(this, "no platform data\n");
		rc = -EINVAL;
		goto err_free;
	}

	this->bdata = cdata->bdata;
	if (!this->bdata) {
		HWLOGE(this, "no bus data\n");
		rc = -EINVAL;
		goto err_free;
	}

	if (this->bdata->of_node) {
		rc = clearpad_touch_config_dt(this);
		if (rc) {
			HWLOGE(this, "err in device tree\n");
			goto err_free;
		}
	}

	spin_lock_init(&this->noise_det.slock);

	if (this->access_test_enabled) {
		rc = clearpad_driver_access_test(this);
		if (rc) {
			goto err_free;
		}
	}

	if (!cdata->rmi_dev) {
		rmi_dev = platform_device_alloc(CLEARPAD_RMI_DEV_NAME, -1);
		if (!rmi_dev) {
			HWLOGE(this, "err in platform_device_alloc\n");
			rc = -ENOMEM;
			retry = true;
			goto err_free;
		}

		rmi_dev->dev.parent = &pdev->dev;
		rc = platform_device_add_data(rmi_dev, cdata,
						sizeof(struct clearpad_data_t));
		if (rc) {
			HWLOGE(this, "err in platform_device_add_data\n");
			goto err_device_put;
		}

		rc = platform_device_add(rmi_dev);
		if (rc) {
			HWLOGE(this, "err in platform_device_add\n");
			goto err_device_put;
		}

		if (!rmi_dev->dev.driver) {
			HWLOGE(this, "no rmi dev\n");
			rc = -ENODEV;
			goto err_device_put;
		}
		cdata->rmi_dev = rmi_dev;
	}

	device_init_wakeup(&this->pdev->dev, 1);

#if defined (CONFIG_FB) \
&&          !defined(CONFIG_DRM_SDE_SPECIFIC_PANEL)
	/* Execute post probe the first UNBLANK event
	   TODO : Must update after API update. */
	HWLOGI(this, "register fb callback\n");
	this->fb_notif.notifier_call = clearpad_fb_notifier_callback;
	rc = fb_register_client(&this->fb_notif);
	if (rc) {
		HWLOGE(this, "unable to register fb_notifier\n");
		goto err_in_fb_register_client;
	}
#endif
#ifdef CONFIG_DRM_SDE_SPECIFIC_PANEL
	/* Execute post probe the first UNBLANK event
	   TODO : Must update after API update. */
	HWLOGI(this, "register drm callback\n");
	this->drm_notif.notifier_call = clearpad_drm_notifier_callback;
	rc = drm_register_client(&this->drm_notif);
	if (rc) {
		HWLOGE(this, "unable to register drm_notifier\n");
		goto err_in_drm_register_client;
	}
#endif

	this->force_sleep = FSMODE_OFF;

	this->irq = gpio_to_irq(this->pdata->irq_gpio);
	rc = devm_request_threaded_irq(&this->pdev->dev,
				this->irq,
				clearpad_hard_handler,
				clearpad_threaded_handler,
				IRQF_TRIGGER_LOW | IRQF_ONESHOT,
				this->pdev->dev.driver->name, this);
	if (rc) {
		HWLOGE(this, "failed to request threaded irq %d (rc=%d)\n",
		       this->irq, rc);
		goto err_in_request_threaded_irq;
	}
	disable_irq_nosync(this->irq);
	atomic_set(&this->irq_enabled, 0);

	if (this->noise_det.supported) {
		this->noise_det.irq = gpio_to_irq(this->noise_det.irq_gpio);
		rc = devm_request_threaded_irq(&this->pdev->dev,
					this->noise_det.irq,
					clearpad_noise_det_hard_handler,
					clearpad_noise_det_threaded_handler,
					IRQF_TRIGGER_RISING | IRQF_ONESHOT,
					"clearpad_noise_det", this);
		if (rc) {
			HWLOGE(this, "failed to request threaded irq %d"
			       " (rc=%d)\n",
			       this->noise_det.irq, rc);
			goto err_in_request_threaded_irq_gpio_noise_det;
		}
		disable_irq_nosync(this->noise_det.irq);
	}

	HWLOGI(this, "input init\n");
	rc = clearpad_input_init(this);
	if (rc) {
		HWLOGE(this, "err in input init\n");
		goto err_in_input_init;
	}

	HWLOGI(this, "ev init\n");
	rc = clearpad_input_ev_init(this);
	if (rc) {
		HWLOGE(this, "err in ev init\n");
		goto err_in_ev_init;
	}

	/* sysfs */
	HWLOGI(this, "create sysfs\n");
	rc = clearpad_create_sysfs_entries(this, clearpad_sysfs_attrs);
	if (rc) {
		HWLOGE(this, "unable to create feature sysfs\n");
		goto err_in_create_sysfs_entries;
	}

	/* create symlink */
	parent = this->input->dev.kobj.parent;
	symlink_name = this->pdata->symlink_name ? : CLEARPAD_NAME;
	rc = sysfs_create_link(parent, &this->input->dev.kobj, symlink_name);
	if (rc) {
		HWLOGE(this, "sysfs_create_link error\n");
		goto err_in_create_link;
	}
	dev_set_name(&this->virtdev, "%s", path_name);
	rc = device_register(&this->virtdev);
	if (rc)
		HWLOGE(this, "device virtdev register error\n");
	dev_set_drvdata(&this->virtdev, this);
	rc = sysfs_create_link(&this->virtdev.kobj, &this->input->dev.kobj, "touch");
	if (rc)
		HWLOGE(this, "common sysfs link error\n");

	if (this->post_probe.start) {
		HWLOGI(this, "schedule post probe\n");
		schedule_delayed_work(&this->post_probe.work, 0);
	} else {
		HWLOGI(this, "post_probe_start sysfs is required"
		       "to start post probe\n");
	}
	UNLOCK(&touchctrl->session_lock);
	goto end_log;

err_in_create_link:
	clearpad_remove_sysfs_entries(this, clearpad_sysfs_attrs);
err_in_create_sysfs_entries:
err_in_ev_init:
err_in_input_init:
err_in_request_threaded_irq_gpio_noise_det:
err_in_request_threaded_irq:
#if defined (CONFIG_FB) \
&&          !defined(CONFIG_DRM_SDE_SPECIFIC_PANEL)
	fb_unregister_client(&this->fb_notif);
err_in_fb_register_client:
#endif
#ifdef CONFIG_DRM_SDE_SPECIFIC_PANEL
	drm_unregister_client(&this->drm_notif);
err_in_drm_register_client:
#endif
err_device_put:
	if (rmi_dev)
		platform_device_put(rmi_dev);
err_free:
	UNLOCK(&touchctrl->session_lock);
	dev_set_drvdata(&pdev->dev, NULL);
	destroy_workqueue(this->thread_resume.work_queue);
err_work_queue:
end_log:
	get_monotonic_boottime(&ts);
	HWLOGI(this, "end probe @ %ld.%06ld (rc=%d)\n",
	       ts.tv_sec, ts.tv_nsec, rc);
end:
	if (retry) {
		if (cdata->probe_retry < SYN_RETRY_NUM_OF_PROBE) {
			rc = -EPROBE_DEFER;
			cdata->probe_retry++;
		}
	}

	return rc;
}

static int clearpad_driver_access_test(struct clearpad_t *this)
{
	int rc = 0;
	int retry;
	u8 device_status = 0;

	for (retry = 0; retry < SYN_RETRY_NUM; retry++) {
		clearpad_set_delay(this->reset.delay_for_powerup_ms);
		rc = clearpad_set_page(this, 0);
		if (!rc)
			goto set_page_passed;
	}
	LOGE(this, "failed to set page 0\n");
	rc = -EINVAL;
	goto end;

set_page_passed:

	/* F01_RMI_DATA00: Device Status */
	rc = clearpad_get(SYNF(this, F01_RMI, DATA,
		this->reg_offset.f01_data00), &device_status);
	if (rc) {
		HWLOGE(this, "failed to read device status\n");
		rc = -EINVAL;
	}
end:
	return rc;
}

static void clearpad_post_probe_work(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct clearpad_post_probe_t *post_probe
		= container_of(dwork, struct clearpad_post_probe_t, work);
	struct clearpad_t *this
		= container_of(post_probe, struct clearpad_t, post_probe);
	const char *session = "post probe";
	struct timespec ts;

	int retry;
	bool do_reschedule = false;
	int rc;

	LOCK(&this->lock);
	if (this->post_probe.done) {
		HWLOGI(this, "already post probe has been done\n");
		UNLOCK(&this->lock);
		goto post_probe_done;
	}
	UNLOCK(&this->lock);

	get_monotonic_boottime(&ts);
	HWLOGI(this, "start post probe @ %ld.%06ld\n", ts.tv_sec, ts.tv_nsec);

	rc = clearpad_ctrl_session_begin(this, session);
	if (rc) {
		HWLOGE(this, "failed to begin post probe session\n");
		do_reschedule = true;
		goto err_in_ctrl_session_begin;
	}

	WARN_ON(!touchctrl_is_display_powered(this));

	LOCK(&this->lock);
	if (!this->dev_active) {
		rc = clearpad_set_resume_mode(this);
		if (rc)
			HWLOGE(this, "failed to set resume mode\n");
	}
	if (this->flash.on_post_probe) {
		this->state = SYN_STATE_DISABLED;
		HWLOGI(this, "ensure firmware\n");
		for (retry = 0; retry < SYN_RETRY_NUM_OF_RECOVERY; retry++) {
			rc = clearpad_fwflash_core(this, SYN_DEFAULT_FLASH, 0);
			if (rc == 0)
				break;
			HWLOGI(this, "retry fwflash (%d)\n", retry);
			clearpad_set_delay(50);
		}
		if (rc) {
			rc = 0;
			HWLOGW(this, "fw flash failed but continue init\n");
		}
	}
	if (this->chip_id == SYN_CHIP_3500) {
		/* Remove this part after firmware flash sequence released */
		/* @ Yoshino1.0 */
		HWLOGW(this, "WA for Yoshino BU\n");
		clearpad_reset(this, SYN_SWRESET, __func__);

		this->state = SYN_STATE_RUNNING;

		UNLOCK(&this->lock);

		rc = clearpad_wait_for_interrupt(this,
			&this->interrupt.for_reset, this->interrupt.wait_ms);
		if (rc)
			LOGE(this, "failed to get interrupt (rc=%d)\n", rc);
	} else {
		this->state = SYN_STATE_RUNNING;

		UNLOCK(&this->lock);
	}

	clearpad_ctrl_session_end(this, session);

err_in_ctrl_session_begin:
	get_monotonic_boottime(&ts);
	HWLOGI(this, "end post probe @ %ld.%06ld (rc=%d)\n",
	       ts.tv_sec, ts.tv_nsec, rc);

	if (do_reschedule) {
		this->post_probe.retry++;
		if (this->post_probe.retry <= SYN_RETRY_NUM_OF_POST_PROBE) {
			HWLOGI(this, "reschedule post probe (%d)\n",
			       this->post_probe.retry);
			schedule_delayed_work(&this->post_probe.work, 3 * HZ);
		} else {
			this->post_probe.retry = 0;
			HWLOGE(this, "stop post probe\n");
		}
	}
	if (!rc) {
		LOCK(&this->lock);
		this->post_probe.done = true;
		clearpad_init_reg_offsets(this, SYN_F12_2D);
		clearpad_set_feature_settings(this);
		if (this->force_sleep != FSMODE_OFF && this->dev_active) {
			rc = clearpad_set_suspend_mode(this);
			if (rc)
				HWLOGE(this, "failed to force sleep device\n");
		}
		UNLOCK(&this->lock);
	}

post_probe_done:
	return;
}

static void clearpad_thread_resume_work(struct work_struct *work)
{
	struct clearpad_thread_resume_t *thread_resume
		= container_of(work, struct clearpad_thread_resume_t, work);
	struct clearpad_t *this
		= container_of(thread_resume,
				struct clearpad_t, thread_resume);
	struct timespec ts;
	bool locked = false;
	int rc;

	get_monotonic_boottime(&ts);
	LOCK(&this->lock);
	if (this->dev_active) {
		HWLOGI(this, "device is already active\n");
		goto already_active;
	}

	if (this->touchctrl.will_powerdown) {
		HWLOGI(this, "not necessary to do thread_resume "
		       "(power=%s icount=%u) @ %ld.%06ld\n",
		       touchctrl_is_touch_powered(this) ? "OK" : "NG",
		       this->interrupt.count, ts.tv_sec, ts.tv_nsec);
		goto will_powerdown;
	}

	HWLOGI(this, "start thread_resume @ %ld.%06ld\n",
	       ts.tv_sec, ts.tv_nsec);

	locked = touchctrl_lock_power(this, "cb_unblank", true, false);
	if (!locked) {
		HWLOGW(this, "power is already turned off");
		goto end;
	}

	this->interrupt.count = 0;
	if (clearpad_handle_if_first_event(this) < 0)
		LOGE(this, "failed to handle first event\n");
		/* Workaround for Kagura sharp panel id 9  & Maple*/
		if (this->chip_id == SYN_CHIP_3500) {
			switch (this->device_info.customer_family) {
			case 0xd0:
			case 0xd1:
				HWLOGW(this, "Force Calibration for Maple\n");
				rc = clearpad_put(
					SYNF(this, F54_ANALOG, COMMAND,
						this->reg_offset.f54_cmd00),
					ANALOG_COMMAND_FORCE_CALIBRATION_MASK);
				if (rc)
					LOGE(this, "failed to force calibrate\n");
				break;
			default:
				break;
			}
		}

		touchctrl_unlock_power(this, "cb_unblank");

	get_monotonic_boottime(&ts);
	HWLOGI(this, "end thread_resume @ %ld.%06ld\n",
	       ts.tv_sec, ts.tv_nsec);
	goto end;

already_active:
will_powerdown:
end:
	UNLOCK(&this->lock);
	return;
}

static int clearpad_remove(struct platform_device *pdev)
{
	struct clearpad_data_t *cdata = pdev->dev.platform_data;
	struct clearpad_t *this = dev_get_drvdata(&pdev->dev);
	char *symlink_name = this->pdata->symlink_name ? : CLEARPAD_NAME;

	cancel_delayed_work_sync(&this->watchdog.work);
	cancel_delayed_work_sync(&this->reset.work);
	flush_workqueue(this->thread_resume.work_queue);
	destroy_workqueue(this->thread_resume.work_queue);
	device_init_wakeup(&this->pdev->dev, 0);
	devm_free_irq(&this->pdev->dev, this->irq, this);
	if (this->noise_det.supported)
		devm_free_irq(&this->pdev->dev, this->noise_det.irq, this);
	sysfs_remove_link(this->input->dev.kobj.parent, symlink_name);
	clearpad_remove_sysfs_entries(this, clearpad_sysfs_attrs);
#if defined (CONFIG_FB) \
&&          !defined(CONFIG_DRM_SDE_SPECIFIC_PANEL)
	fb_unregister_client(&this->fb_notif);
#endif
#ifdef CONFIG_DRM_SDE_SPECIFIC_PANEL
	drm_unregister_client(&this->drm_notif);
#endif
	input_unregister_device(this->input);
	platform_device_unregister(cdata->rmi_dev);
	dev_set_drvdata(&pdev->dev, NULL);
	return 0;
}

static const struct dev_pm_ops clearpad_pm = {
	.suspend = clearpad_pm_suspend,
	.resume = clearpad_pm_resume,
};

static struct platform_driver clearpad_driver = {
	.driver = {
		.name	= CLEARPAD_NAME,
		.owner	= THIS_MODULE,
		.pm	= &clearpad_pm,
	},
	.probe		= clearpad_probe,
	.remove		= clearpad_remove,
};

static int __init clearpad_init(void)
{
	return platform_driver_register(&clearpad_driver);
}

static void __exit clearpad_exit(void)
{
	platform_driver_unregister(&clearpad_driver);
}

late_initcall(clearpad_init);
module_exit(clearpad_exit);

MODULE_DESCRIPTION(CLEARPAD_NAME "ClearPad Driver");
MODULE_LICENSE("GPL v2");
