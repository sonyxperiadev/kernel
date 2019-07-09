/*
 * Atmel maXTouch Touchscreen driver
 *
 * Copyright (C) 2010 Samsung Electronics Co.Ltd
 * Copyright (C) 2013 LG Electronics Co.Ltd
 * Copyright (C) 2017, 2018 Sony Mobile Communications Inc.
 * Author: <WX-BSP-TS@lge.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#ifndef __LINUX_ATMEL_MXT_TS_H__
#define __LINUX_ATMEL_MXT_TS_H__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/notifier.h>
#include <linux/drm_notify.h>
#include <linux/input/mt.h>
//#include <linux/sysdev.h>
#include <linux/device.h>
#include <linux/firmware.h>
#if defined(CONFIG_DRM_SDE_SPECIFIC_PANEL)
#include <linux/incell.h>
#endif

#define LGE_TOUCH_NAME	"lge_touch"
#define MXT_DEVICE_NAME	"touch_dev"

#define SIDE_KEY_DEVICE	1
#if SIDE_KEY_DEVICE
#define SIDE_KEY_DEVICE_NAME	"sidekey_dev"
#endif

#define T100_AREA_REPLACE_AMPLITUDE
#ifdef T100_AREA_REPLACE_AMPLITUDE
#define T100_AREA_W_H_IN_2BYTES
#endif

#define MAX_ATTR_SIZE	11 * 1024 - 545
#define MXT_FACTORY

#define REF_OFFSET_VALUE	16384
#define REF_MIN_VALUE	(19744 - REF_OFFSET_VALUE)
#define REF_MAX_VALUE	(28884 - REF_OFFSET_VALUE)

#define NODE_PER_PAGE	64
#define DATA_PER_NODE	2

#define MS_TO_NS(x)	(x * 1E6L)

/* Firmware params */
#define MXT_FW_MAGIC	0x4D3C2B1A
#define MXT_FW_RETRY_NUM	3

/* Diagnostic command defines  */
#define MXT_DIAG_PAGE_UP	0x01
#define MXT_DIAG_PAGE_DOWN	0x02
#define MXT_DIAG_DELTA_MODE	0x10
#define MXT_DIAG_REFERENCE_MODE	0x11
#define MXT_DIAG_CTE_MODE	0x31
#define MXT_DIAG_IDENTIFICATION_MODE	0x80
#define MXT_DIAG_TOCH_THRESHOLD_MODE	0xF4
#define MXT_DIAG_MODE_MASK	0xFC
#define MXT_DIAGNOSTIC_MODE	0
#define MXT_DIAGNOSTIC_PAGE	1
#define MXT_CONFIG_VERSION_LENGTH	30

/* Configuration file */
#define MXT_CFG_MAGIC	"OBP_RAW V1"

/* Registers */
#define MXT_OBJECT_NUM_MAX	200
#define MXT_OBJECT_START	0x07
#define MXT_OBJECT_SIZE	6
#define MXT_INFO_CHECKSUM_SIZE	3
#define MXT_MAX_BLOCK_WRITE	256

/* Object types */
#define MXT_DEBUG_DIAGNOSTIC_T37	37
#define MXT_GEN_MESSAGE_T5	5
#define MXT_GEN_COMMAND_T6	6
#define MXT_GEN_POWER_T7	7
#define MXT_GEN_ACQUIRE_T8	8
#define MXT_TOUCH_KEYARRAY_T15	15
#define MXT_TOUCH_PROXIMITY_T23	23
#define MXT_TOUCH_MULTITOUCHSCREEN_T100	100
#define MXT_PROCI_GRIPSUPPRESSION_T40	40
#define MXT_PROCI_TOUCHSUPPRESSION_T42	42
#define MXT_PROCI_STYLUS_T47	47
#define MXT_PROCI_ADAPTIVETHRESHOLD_T55	55
#define MXT_PROCI_SHIELDLESS_T56	56
#define MXT_PROCI_EXTRATOUCHSCREENDATA_T57	57
#define MXT_PROCI_LENSBENDING_T65	65
#define MXT_PROCI_PALMGESTUREPROCESSOR_T69	69
#define MXT_PROCG_NOISESUPPRESSION_T72	72
#define MXT_GLOVEDETECTION_T78	78
#define MXT_RETRANSMISSIONCOMPENSATION_T80	80
#define MXT_PROCI_GESTUREPROCESSOR_T84	84
#define MXT_PROCI_TOUCH_SEQUENCE_LOGGER_T93	93
#define MXT_PROCI_SCHNOISESUPPRESSION_T103	103
#define MXT_SPT_COMMSCONFIG_T18	18
#define MXT_SPT_GPIOPWM_T19	19
#define MXT_SPT_SELFTEST_T25	25
#define MXT_SPT_USERDATA_T38	38
#define MXT_SPT_MESSAGECOUNT_T44	44
#define MXT_SPT_CTECONFIG_T46	46
#define MXT_SPT_TIMER_T61	61
#define MXT_SPT_GOLDENREFERENCES_T66	66
#define MXT_SPT_DYNAMICCONFIGURATIONCONTROLLER_T70	70
#define MXT_SPT_DYNAMICCONFIGURATIONCONTAINER_T71	71
#define MXT_SPT_SELFCAPCBCRCONFIG_T102	102
#define MXT_SPT_AUXTOUCHCONFIG_T104	104
#define MXT_SPT_TOUCHSCREENHOVER_T101	101
#define MXT_SPT_DRIVENPLATEHOVERCONFIG_T105	105
#define MXT_PROCI_ONETOUCH_T24	24
#define MXT_SPT_PROTOTYPE_T35	35
#define MXT_SPT_SELFCAPGLOBALCONFIG_T109	109

/* Not for ATMEL S540 */
#define MXT_GEN_DATASOURCE_T53	53
#define MXT_TOUCH_MULTI_T9	9
#define MXT_TOUCH_PROXKEY_T52	52
#define MXT_PROCI_GRIPFACE_T20	20
#define MXT_PROCG_NOISE_T22	22
#define MXT_PROCI_TWOTOUCH_T27	27
#define MXT_PROCI_PALM_T41	41
#define MXT_PROCG_NOISESUPPRESSION_T48	48
#define MXT_SPT_CTECONFIG_T28	28
#define MXT_SPT_DIGITIZER_T43	43
#define MXT_SPT_NOISESUPPRESSION_T48	48
#define MXT_PROCI_ACTIVE_STYLUS_T63	63
#define MXT_RESERVED_T255	255

/* MXT_GEN_MESSAGE_T5 object */
#define MXT_RPTID_NOMSG		0xff

/* MXT_GEN_COMMAND_T6 field */
#define MXT_COMMAND_RESET	0
#define MXT_COMMAND_BACKUPNV	1
#define MXT_COMMAND_CALIBRATE	2
#define MXT_COMMAND_REPORTALL	3
#define MXT_COMMAND_DIAGNOSTIC	5
#define MXT_COMMAND_SELF_REFERENCE	248

/* Define for T6 status byte */
#define MXT_T6_STATUS_RESET	(1 << 7)
#define MXT_T6_STATUS_OFL	(1 << 6)
#define MXT_T6_STATUS_SIGERR	(1 << 5)
#define MXT_T6_STATUS_CAL	(1 << 4)
#define MXT_T6_STATUS_CFGERR	(1 << 3)
#define MXT_T6_STATUS_COMSERR	(1 << 2)

#define MXT_POWER_CFG_RUN	0
#define MXT_POWER_CFG_DEEPSLEEP	1

/* MXT_GEN_COMMAND_T8 field */
#define MXT_T8_TCHAUTOCAL	4
#define MXT_T8_ATCHCALST	6
#define MXT_T8_ATCHCALSTHR	7
#define MXT_T8_ATCHFRCCALTHR	8
#define MXT_T8_ATCHFRCCALRATIO	9

#define MXT_GRIP_REJECTION_BORDER_NUM 4
#define MXT_MAX_FINGER_IN_FIRMWARE 16

/* Log */
#define LOGx(FUNC, level, X, ...)						\
({										\
	if (*log_level >= level)						\
		FUNC("[Solomon](%s:%d)" X, __func__, __LINE__, ##__VA_ARGS__);	\
})

enum log_level_list;

#define LOGE(X, ...) LOGx(pr_err, LEVEL_DEFAULT, X, ## __VA_ARGS__)
#define LOGW(X, ...) LOGx(pr_warn, LEVEL_DEFAULT, X, ## __VA_ARGS__)
#define LOGN(X, ...) LOGx(pr_notice, LEVEL_DEFAULT, X, ## __VA_ARGS__)
#define LOGI(X, ...) LOGx(pr_notice, LEVEL_DEBUG, X, ## __VA_ARGS__)
#define LOGD(X, ...) LOGx(pr_notice, LEVEL_ALL, X, ## __VA_ARGS__)

#define TOUCH_PATCH_INFO_MSG(fmt, args...)	pr_info("[Solomon Patch] " fmt, ##args)

/* MXT_TOUCH_MULTI_T9 field */
#define MXT_T9_ORIENT	9
#define MXT_T9_RANGE	18

/* MXT_TOUCH_MULTI_T9 status */
#define MXT_T9_UNGRIP	(1 << 0)
#define MXT_T9_SUPPRESS	(1 << 1)
#define MXT_T9_AMP	(1 << 2)
#define MXT_T9_VECTOR	(1 << 3)
#define MXT_T9_MOVE	(1 << 4)
#define MXT_T9_RELEASE	(1 << 5)
#define MXT_T9_PRESS	(1 << 6)
#define MXT_T9_DETECT	(1 << 7)

/* MXT_TOUCH_MULTI_T9 orient */
#define MXT_T9_ORIENT_SWITCH	(1 << 0)

/* MXT_SPT_COMMSCONFIG_T18 */
#define MXT_COMMS_CTRL		0
#define MXT_COMMS_CMD		1
#define MXT_COMMS_RETRIGEN	(1 << 6)

/* Define for MXT_GEN_COMMAND_T6 */
#define MXT_BOOT_VALUE	0xa5
#define MXT_RESET_VALUE	0x01
#define MXT_BACKUP_VALUE	0x55
#define MXT_STOP_DYNAMIC_CONFIG	0x33

/* Define for MXT_PROCI_TOUCHSUPPRESSION_T42 */
#define MXT_T42_MSG_TCHSUP	(1 << 0)

#define UDF_MESSAGE_COMMAND	50
#define MAX_POINT_SIZE_FOR_LPWG	10
#define MAX_T37_MSG_SIZE	9

/* T47 Stylus */
#define MXT_TOUCH_MAJOR_T47_STYLUS	1

/* T72 NOISE SUPPRESSION */
#define MXT_T72_VNOI	0x42	/* Very noisy state Touch & No Touch Byte: 66 ~ 75 */

/* T93 Touch Sequense Processor */
#define MXT_T93_TAP_COUNT	1
#define MXT_T93_HIGH_X	4
#define MXT_T93_LOW_X	3
#define MXT_T93_HIGH_Y	6
#define MXT_T93_LOW_Y	5
#define MXT_T93_FAIL_REASON	2
#define MXT_T93_FIRST_TOUCH	0x01
#define MXT_T93_SECOND_TOUCH	0x02
#define MXT_T93_DOUBLE_TAP_OK	0x00


/* T100 Multiple Touch Touchscreen */
#define MXT_T100_CTRL	0
#define MXT_T100_CFG1	1
#define MXT_T100_TCHAUX	3
#define MXT_T100_XRANGE	13
#define MXT_T100_YRANGE	24

#define MXT_T100_CFG_SWITCHXY	(1 << 5)

#define MXT_T100_TCHAUX_VECT	(1 << 0)
#define MXT_T100_TCHAUX_AMPL	(1 << 1)
#define MXT_T100_TCHAUX_AREA	(1 << 2)
#define MXT_T100_TCHAUX_RESV	(1 << 3)
#define MXT_T100_TCHAUX_PEAK	(1 << 4)

#define MXT_T100_DETECT	(1 << 7)
#define MXT_T100_FRIST_ID_SUPPRESSION	(1 << 6)
#define MXT_T100_TYPE_MASK	0x70
#define MXT_T100_TYPE_FINGER	0x10
#define MXT_T100_TYPE_STYLUS	0x20
#define MXT_T100_TYPE_GLOVE	0x50
#define MXT_T100_TYPE_PALM	0x60
#define MXT_T100_TYPE_SIDEKEY	0x70
#define MXT_T100_STATUS_MASK	0x0F
#define MXT_T100_NOEVENT	0x00
#define MXT_T100_PRESS		0x04
#define MXT_T100_RELEASE	0x05
#define MXT_T100_MOVE		0x01
#define MXT_T100_SUPPRESSION	0x03

#define MXT_T100_JUMPLIMIT	0x2B

#ifdef T100_AREA_W_H_IN_2BYTES
#define MXT_T100_AREA_MASK	0x3F
#define MXT_T100_WIDTH_MSB_MASK	0xC0
#define MXT_T100_WIDTH_LSB_MASK	0xE0
#define MXT_T100_HEIGHT_MASK	0x1F
#endif

/* Self Capacitance Global Configuration */
#define MXT_T109_CMD	3
#define MXT_T109_CMD_TUNE	1
#define MXT_T109_CMD_STORE_RAM	4

/* Delay times */
#define MXT_BACKUP_TIME		20	/* msec */
#define MXT_RESET_TIME		200	/* msec */
#define MXT_RESET_TIMEOUT	3000	/* msec */
#define MXT_CRC_TIMEOUT		1000	/* msec */
#define MXT_FW_RESET_TIME	1000	/* msec */
#define MXT_FW_CHG_TIMEOUT	300	/* msec */
#define MXT_WAKEUP_TIME		25	/* msec */
#define MXT_REGULATOR_DELAY	150	/* msec */
#define MXT_POWERON_DELAY	100	/* msec */
#define MXT_SELFTEST_TIME	3000	/* msec */
#define MXT_WAITED_UDF_TIME 200 /* msec */

/* Command to unlock bootloader */
#define MXT_UNLOCK_CMD_MSB	0xaa
#define MXT_UNLOCK_CMD_LSB	0xdc

/* Bootloader mode status */
#define MXT_WAITING_BOOTLOAD_CMD	0xc0	/* valid 7 6 bit only */
#define MXT_WAITING_FRAME_DATA	0x80	/* valid 7 6 bit only */
#define MXT_FRAME_CRC_CHECK	0x02
#define MXT_FRAME_CRC_FAIL	0x03
#define MXT_FRAME_CRC_PASS	0x04
#define MXT_APP_CRC_FAIL	0x40	/* valid 7 8 bit only */
#define MXT_BOOT_STATUS_MASK	0x3f
#define MXT_BOOT_EXTENDED_ID	(1 << 5)
#define MXT_BOOT_ID_MASK	0x1f

/* Touchscreen absolute values */
#define MXT_MAX_AREA	25

#define MXT_PIXELS_PER_MM	20

#define MXT_STATE_INACTIVE	0
#define MXT_STATE_RELEASE	1
#define MXT_STATE_PRESS	2
#define MXT_STATE_MOVE	3

#define MXT_MAX_KEY	0
#define MXT_MAX_FINGER	10
#define MXT_MAX_FW_PATH	30
#define MXT_DISALEEVT_VALUE	0x33

#define TOUCHEVENTFILTER	0

#if 1
#define CHARGER_PLUGGED	0
#define CHARGER_UNplugged	1
#define DEEP_SLEEP_WAKEUP	2

#define CHARGER_KNOCKON_SLEEP	3
#define CHARGER_KNOCKON_WAKEUP	4
#define NOCHARGER_KNOCKON_SLEEP	5
#define NOCHARGER_KNOCKON_WAKEUP	6

#define PATCH_EVENT_PAIR_NUM	4
#if 1 //For sensing test
#define SELF_CAP_ON_NOISE_RECOVER	11
#define SELF_CAP_OFF_NOISE_SUPPRESSION	12

#define HIGH_TEMP_SET	13
#define HIGH_TEMP_UNSET 	14

/*battery temp*/
#define HIGH_TEMP_SET_LEVEL	50
#define LOW_TEMP_SET_LEVEL	-5

#define HIGH_TEMP_MODE	1
#define NORMAL_TEMP_MODE	0
#define LOW_TEMP_MODE		-1

#define LOW_TEMP_SET	17
#define LOW_TEMP_UNSET	18

#define LOW_TEMP_KNOCKON_SET	19
#define LOW_TEMP_KNOCKON_UNSET	20
#define HIGH_TEMP_KNOCKON_SET	21
#define HIGH_TEMP_KNOCKON_UNSET	22

#if 1
#define PATCH_EVENT_AAT	15
#define CHARGER_PLUGGED_AAT	16
#endif
#else
#define PATCH_EVENT_AAT	11
#define CHARGER_PLUGGED_AAT	12
#endif
#else
/* patch Event */
#define CHARGER_PLUGGED	0
#define CHARGER_UNplugged	1
#define CHARGER_KNOCKON_SLEEP	2
#define CHARGER_KNOCKON_WAKEUP	3
#define NOCHARGER_KNOCKON_SLEEP	4
#define NOCHARGER_KNOCKON_WAKEUP	5
#define DEEP_SLEEP_WAKEUP	6
#endif

#define MXT_INFOMATION_BLOCK_SIZE	7
#define MXT_OBJECT_TABLE_ELEMENT_SIZE	6
#define MXT_OBJECT_TABLE_START_ADDRESS	7

#define PATCH_EVENT_KNOCKON	(0x01 << 1)
#define PATCH_EVENT_TA	(0x01 << 2)
#define POWERLOCK_FW_UP	(0x01 << 1)
#define POWERLOCK_SYSFS	(0x01 << 2)

#define SELF_DIAGNOSTIC_FILE_PATH_FACTORY	"/data/touch/touch_self_test.txt"
#define SELF_DIAGNOSTIC_FILE_PATH_NORMAL	"/sdcard/touch_self_test.txt"
#define RAWDATA_FILE_PATH_FACTOR	"/data/touch/touch_rawdata.txt"
#define RAWDATA_FILE_PATH_NORMAL	"/sdcard/touch_rawdata.txt"
#define DELTA_FILE_PATH_FACTORY	"/data/touch/touch_delta.txt"
#define DELTA_FILE_PATH_NORMAL	"/sdcard/touch_delta.txt"

#define MAX_LOG_FILE_SIZE	(10 * 1024 * 1024) /* 10 M byte */
#define MAX_LOG_FILE_COUNT	4

#define SELF_DIAGNOSTIC_STATUS_COMPLETE	0
#define SELF_DIAGNOSTIC_STATUS_RUNNING	1

/* Patch event code */
#define PATCH_EVENT_CODE_GRIP_SUPPRESSSION_PORTRAIT	((u8)0)
#define PATCH_EVENT_CODE_GRIP_SUPPRESSSION_LANDSCAPE	((u8)1)
#define PATCH_EVENT_CODE_SIDETOUCH_ENABLE	((u8)2)
#define PATCH_EVENT_CODE_SIDETOUCH_ONLY_RIGHT	((u8)3)
#define PATCH_EVENT_CODE_SIDETOUCH_ONLY_LEFT	((u8)4)
#define PATCH_EVENT_CODE_SIDETOUCH_DISABLE	((u8)5)
#define PATCH_EVENT_CODE_STAMINA_ENABLE	((u8)7)
#define PATCH_EVENT_CODE_STAMINA_DISABLE	((u8)8)
#define PATCH_EVENT_CODE_AOD_ENABLE	((u8)9)
#define PATCH_EVENT_CODE_AOD_DISABLE	((u8)10)
#define PATCH_EVENT_CODE_COVER_ENABLE	((u8)11)
#define PATCH_EVENT_CODE_COVER_DISABLE	((u8)12)
#define PATCH_EVENT_CODE_GLOVE_ENABLE	((u8)13)
#define PATCH_EVENT_CODE_GLOVE_DISABLE	((u8)14)

/* Sidetouch status */
#define SIDE_KEY_ENABLE	0
#define SIDE_KEY_ONLY_RIGHT	1
#define SIDE_KEY_ONLY_LEFT	2
#define SIDE_KEY_DISABLE	3
#define SIDE_KEY_ENABLE_CHARGER	4
#define SIDE_KEY_DISABLE_CHARGER	5

/* Feature status */
#define STATUS_ENABLE	1
#define STATUS_DISABLE	0

/* AoD mode */
#define AOD_MODE_DOUBLE_TAP	2

#define BASIC_FAMILY_ID	0xA6
#define FAMILY_ID_START_LOCATION	0
#define FAMILY_ID_SIZE	1
#define FINGER_PRESS	1
#define FINGER_RELEASE	0
#define MAX_PRESSURE	255
#define TAP_WAIT_TIME	3000
#define U8_BIT_SHIFT	8

#define GLOVE_MODE_FINGER_LIMIT	(u8)2

#define MXT_ACCESS_TEST_NUM    3
#define MXT_CHARGE_OUT_TIMEOUT 20

#define SIDE_KEY_MINOR_MOVE	100

/* Current SOD status */
#define SOD_MODE_ON	1
#define SOD_MODE_OFF	0

/* MXT_GEN_POWER_T7 field */
struct t7_config {
	u8 idle;
	u8 active;
} __packed;

struct t9_range {
	u16 x;
	u16 y;
} __packed;

enum {
	POWER_OFF = 0,
	POWER_ON,
	POWER_SLEEP,
	POWER_WAKE
};

enum {
	FINGER_INACTIVE,
	FINGER_RELEASED,
	FINGER_PRESSED,
	FINGER_MOVED
};

enum {
	TIME_EX_INIT_TIME,
	TIME_EX_FIRST_INT_TIME,
	TIME_EX_PREV_PRESS_TIME,
	TIME_EX_CURR_PRESS_TIME,
	TIME_EX_FIRST_GHOST_DETECT_TIME,
	TIME_EX_SECOND_GHOST_DETECT_TIME,
	TIME_EX_CURR_INT_TIME,
	TIME_EX_WATER_INT_TIME,
	TIME_EX_DELTA_INT_TIME,
	TIME_EX_ANTI_INT_TIME,
	TIME_EX_INVALID_AMPLITUDE_DETECT_INIT_TIME,
	TIME_EX_GHOST_PATTERN_DETECT_INIT_TIME,
	TIME_EX_PROFILE_MAX,
};

enum {
	NEED_TO_OUT = 101,
	NEED_TO_INIT,
};

enum {
	ANTI_FIRST_LIMIT,
	ANTI_SECOND_LIMIT,
	ANTI_TIME_LIMIT,
	FIRST_INT_TIME,
	ANTI_AREA_MONITOR,
	ANTI_AREA_LIMIT,
	FIRST_GHOST_TIME,
	SECOND_GHOST_TIME,
	BENDING_LVL,
	ANTI_AREA_NO_FINGER,
	ANTI_AREA_CNT,
	ANTI_AREA_ONE_FINGER,
	ANTI_AREA_MONITOR2,
	GHOST_VALUE_MAX,
};

enum {
	JITTER_VALUE,
	GHOST_DETECT_LIMIT_TIME,
	GHOST_DETECT_X,
	LIMIT_OF_Z,
	LIMIT_OF_COUNT,
	CAL_OF_TIME,
	PALM_CAL_TIME,
	GHOST_CARD_DETECTION_VALUE_MAX,
};

enum {
	GHOST_MARGIN_TO_EDGE_X,
	GHOST_MARGIN_TO_EDGE_Y,
	GHOST_INVALID_TOLERANCE_COUNT,
	GHOST_DETECT_MONITOR_DURATION_LIMIT,
	GHOST_PEN_VALID_MIN_AMPLITUDE,
	GHOST_PEN_VALID_MAX_AMPLITUDE,
	GHOST_VALID_AMPLITUDE_DETECT_TIME,
	GHOST_VALID_PEN_AMPLITUDE_DELTA_THRESHOLD,
	GHOST_VALID_FINGER_AMPLITUDE_DELTA_THRESHOLD,
	GHOST_MAX_AMPLITUDE,
	GHOST_INVALID_AMPLITUDE_VALUE_MAX,
};

enum {
	GHOST_PATTERN_MARGIN_TO_EDGE_X,
	GHOST_PATTERN_MARGIN_TO_EDGE_Y,
	GHOST_VALID_PATTERN_DETECT_TIME,
	GHOST_PATTERN_DETECTION_VALUE_MAX,
};

enum {
	LPWG_READ = 1,
	LPWG_ENABLE,
	LPWG_LCD_X,
	LPWG_LCD_Y,
	LPWG_ACTIVE_AREA_X1,
	LPWG_ACTIVE_AREA_X2,
	LPWG_ACTIVE_AREA_Y1,
	LPWG_ACTIVE_AREA_Y2,
	LPWG_TAP_COUNT,
	LPWG_REPLY,
	LPWG_LENGTH_BETWEEN_TAP,
	LPWG_EARLY_MODE,
	LPWG_DOUBLE_TAP_CHECK,
	LPWG_UPDATE_ALL,
};

enum {
	LPWG_NONE = 0,
	LPWG_DOUBLE_TAP,
	LPWG_MULTI_TAP,
};

enum {
	PM_RESUME = 0,
	PM_SUSPEND,
	PM_SUSPEND_IRQ,
};

typedef enum error_type {
	NO_ERROR = 0,
	ERROR,
	IGNORE_EVENT,
	IGNORE_EVENT_BUT_SAVE_IT,
} err_t;

enum log_level_list {
	LEVEL_DEFAULT,
	LEVEL_DEBUG,
	LEVEL_ALL,
};

struct point {
	int x;
	int y;
};

enum {
	INCOMING_CALL_IDLE,
	INCOMING_CALL_RINGING,
	INCOMING_CALL_OFFHOOK,
};

enum {
	SOD_POWER_ON = 0,
	SOD_POWER_OFF_SKIP,
	SOD_POWER_OFF,
};

struct tci_abs {
	int x;
	int y;
};

struct mxt_driver_data {
	struct mutex lpwg_lock;
	struct mutex i2c_suspend_lock;
	struct mutex irq_lock;
	struct mutex dev_lock;

	bool is_probing;
	bool is_resuming;
	bool selftest_enable;
	bool selftest_show;
	struct mxt_data *mxt_data;
	struct workqueue_struct *touch_wq;

	bool touch_irq_mask;
	bool touch_irq_wake_mask;
	unsigned char touched_finger_count;
	unsigned char patchevent_mask;
	unsigned char power_block_mask;

	int temp_mode;
	int touch_current_temp;
	int touch_previous_temp_mode;

	struct timeval t_ex_debug[TIME_EX_PROFILE_MAX];
	u8 resume_flag;
	u16 retry_chk;
	u8 ime_status_value;

#ifdef MXT_FACTORY
	bool factorymode;
#endif
	int usb_type;
	int factory_boot;

	struct tci_abs tci_press[MAX_POINT_SIZE_FOR_LPWG];
	struct tci_abs tci_report[MAX_POINT_SIZE_FOR_LPWG];

	struct workqueue_struct* touch_multi_tap_wq;
};

/* The platform data for the Atmel maXTouch touchscreen driver */
struct mxt_platform_data {
	size_t config_array_size;
	u8 numtouch;	/* Number of touches to report	*/
	int max_x;	/* The default reported X range   */
	int max_y;	/* The default reported Y range   */
	bool i2c_pull_up;
	int t15_num_keys;
	unsigned int t15_keystate[MXT_MAX_KEY];
	unsigned int t15_key_array_x[MXT_MAX_KEY];
	unsigned int t15_key_array_y[MXT_MAX_KEY];
	unsigned int t15_extra_keystate[MXT_MAX_KEY];
	unsigned int t15_extra_key_array_x[MXT_MAX_KEY];
	unsigned int t15_extra_key_array_y[MXT_MAX_KEY];
	unsigned int t15_keymap[MXT_MAX_KEY];
	unsigned long gpio_avdd;
	unsigned long gpio_dvdd;
	unsigned long gpio_vddio;
	unsigned long gpio_reset;
	unsigned long gpio_int;
	unsigned long gpio_id;
	unsigned char panel_on;
	const char *fw_name;
	const char *fw_name_ogs;
	const char *fw_name_gf2;
	const char *fw_name_lgd;
	const char *fw_name_laibao;
	const char *extra_fw_name_gf2;
	char knock_on_type;
	unsigned int lcd_x;
	unsigned int lcd_y;
	int global_access_pixel;
	u8 use_mfts;
	u8 fw_ver[3];
	u8 product[10];
	unsigned int ref_reg_weight_val;
	unsigned int butt_check_enable;
	struct regulator *avdd;
#ifdef CONFIG_TOUCHSCREEN_ATMEL_LPWG_DEBUG_REASON
	int use_debug_reason;
	int realtime_use_debug_reason;
#endif
	int lpwg_test;
};

struct mxt_finger {
	u16 x;
	u16 y;
	u16 w;
	u16 z;
	u16 component;
	u8 state;
	u8 type;
	u8 event;
	u16 mcount;
};

struct mxt_info {
	u8 family_id;
	u8 variant_id;
	u8 version;
	u8 build;
	u8 matrix_xsize;
	u8 matrix_ysize;
	u8 object_num;
};

struct mxt_object {
	u8 type;
	u16 start_address;
	u8 size_minus_one;
	u8 instances_minus_one;
	u8 num_report_ids;
} __packed;

struct mxt_raw_data {
	u8 num_xnode;
	u8 num_ynode;
	u16 num_nodes;
	u16 *reference;
	s16 *delta;
};

/**
 * struct mxt_fw_image - Represents a firmware file.
 * @ magic_code: Identifier of file type.
 * @ hdr_len: Size of file header (struct mxt_fw_image).
 * @ cfg_len: Size of configuration data.
 * @ fw_len: Size of firmware data.
 * @ cfg_crc: CRC of configuration settings.
 * @ bin_ver: Version of binary firmware.
 * @ build_ver: Build version of firmware.
 * @ data: Configuration data followed by firmware image.
 */
struct mxt_fw_image {
	__le32 magic_code;
	__le32 hdr_len;
	__le32 cfg_len;
	__le32 fw_len;
	__le32 cfg_crc;
	u8 bin_ver;
	u8 build_ver;
	u8 extra_info[32];
	u8 data[0];
} __packed;

/**
 * struct mxt_cfg_data - Represents a configuration data item.
 * @ type: Type of object.
 * @ instance: Instance number of object.
 * @ size: Size of object.
 * @ register_val: Series of register values for object.
 */
struct mxt_cfg_data {
	u8 type;
	u8 instance;
	u8 size;
	u8 register_val[0];
} __packed;

struct mxt_fw_info {
	u8 bin_ver;
	u8 build_ver;
	u32 hdr_len;
	u32 cfg_len;
	u32 fw_len;
	u32 cfg_crc;
	const u8 *cfg_raw_data;	/* start address of configuration data */
	const u8 *fw_raw_data;	/* start address of firmware data */
	struct mxt_data *data;
};

/*Reference Check*/
struct mxt_channel_size {
	u8 start_x;
	u8 start_y;
	u8 size_x;
	u8 size_y;
};

struct mxt_ref_limit {
	u8 ref_chk_using;
	u8 ref_x_all_err_line;
	u8 ref_y_all_err_line;
	u8 xline_max_err_cnt;
	u8 yline_max_err_cnt;
	u8 err_weight;
	u8 ref_rng_limit;
	u8 ref_diff_max;
	u8 ref_err_cnt;
	s8 y_line_dif[14];
	s8 butt_dif[3];
};

struct mxt_patch {
	u8* patch;
	u16* stage_addr;
	u16* tline_addr;
	u16* trigger_addr;
	u16* event_addr;
	u16* src_item;
	u16* check_cnt;
	u16 period;
	u8 stage_cnt;
	u8 tline_cnt;
	u8 trigger_cnt;
	u8 event_cnt;
	u8 option;
	u8 debug;
	u8 timer_id;
	u8 cur_stage;
	u8 cur_stage_opt;
	u8 run_stage;
	u8 start;
	u8 finger_cnt;
	u8 start_stage;
	u8 skip_test;
	u8 cal_flag;
	u32 date;
	u32 stage_timestamp;
};

struct mxt_message {
	u8 reportid;
	u8 message[8];
};

struct mxt_reportid {
	u8 type;
	u8 index;
};

struct t_data {
	u16 id;
	u16 x_position;
	u16 y_position;
	u16 width_major;
	u16 width_minor;
	u16 width_orientation;
	u16 pressure;
	u16 area;
	u8 status;
	u8 touch_major;
	u8 touch_minor;
	int tool;
	bool is_pen;
	bool is_palm;
	bool skip_report;
#if SIDE_KEY_DEVICE
	bool is_sidekey;
#endif
};

struct b_data {
	u16 key_code;
	u16 state;
};

struct touch_data {
	u8 total_num;
	u8 prev_total_num;
	u8 touch_count;
	u8 state;
	u8 palm;
	u8 prev_palm;
	struct t_data curr_data[MXT_MAX_FINGER];
	struct t_data prev_data[MXT_MAX_FINGER];
	struct b_data curr_button;
	struct b_data prev_button;
};

struct quickcover_size {
	int x_max;
	int y_max;
	int x_min;
	int y_min;
};

struct mxt_anti_info {
	bool insensitive_th;
	bool sensitive_th;
	bool anti_report;
	bool autocal;
	bool pen;
	int pen_id;
	int curr_ths;
	u16 anti_area;
	u16 touch_area;
	u16 inter_area;
	int fcnt0_msg_cnt;
};

/* Struct related to patch event */

struct atmel_side_key {
	bool supported;
	int status;
};

struct atmel_glove_mode {
	bool supported;
	int status;
};

struct atmel_cover_mode {
	bool supported;
	int status;
	int mode;
};

struct atmel_stamina_mode {
	bool supported;
	int status;
};

struct atmel_aod_mode {
	bool supported;
	int status;
};

struct atmel_sod_mode {
	bool supported;
	int status;
	int pre_status;
	int disp_status;
};

struct mxt_watchdog {
	struct delayed_work work;
	bool supported;
	int delay;
};

/* Each client has this additional data */
struct mxt_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
#if SIDE_KEY_DEVICE
	struct input_dev *input_dev_touch;
	struct input_dev *input_dev_sidekey;
#endif
	char phys[64];		/* device physical location */
	struct mxt_platform_data *pdata;
	struct mxt_object *object_table;
	struct mxt_object *global_object;
	struct mxt_reportid *reportids;
	struct mxt_info *info;
	struct mxt_fw_info fw_info;
	struct mxt_anti_info *anti;
	void *raw_info_block;
	unsigned int irq;
	unsigned int max_x;
	unsigned int max_y;
	bool in_bootloader;
	u16 mem_size;
	u8 t100_aux_ampl;
	u8 t100_aux_area;
	u8 t100_aux_vect;
	u8 t100_aux_resv;
#ifdef T100_AREA_REPLACE_AMPLITUDE
	u8 T100_palm_threshold;
#endif

	struct bin_attribute mem_access_attr;
	struct bin_attribute rawdata_attr;
	u8 *prd_rawdata_data;
	struct bin_attribute delta_attr;
	u8 *prd_delta_data;
	bool debug_enabled;
	bool t57_debug_enabled;
	u8 max_reportid;
	u32 config_crc;
	u32 info_crc;
	u8 bootloader_addr;
	struct t7_config t7_cfg;
	u8 *msg_buf;
	u8 t6_status;
	u8 t6_status_reset_irq;
	bool update_input;
	u8 last_message_count;
	u8 num_touchids;
	unsigned long t15_keystatus;
	bool use_regulator;
	u8 regulator_status;

	struct notifier_block drm_notif;
	struct kobject lge_touch_kobj;
	struct device virtdev;
	/* Cached parameters from object table */
	u16 T5_address;
	u8 T5_msg_size;
	u8 T6_reportid;
	u16 T6_address;
	u16 T7_address;
	u16 T8_address;
	u16 T9_address;
	u8 T9_reportid_min;
	u8 T9_reportid_max;
	u8 T15_reportid_min;
	u8 T15_reportid_max;
	u16 T18_address;

	u8 T24_reportid;
	u8 T35_reportid;
	u8 T25_reportid;
	u16 T25_address;
	u16 T42_address;
	u8 T42_reportid_min;
	u8 T42_reportid_max;

	u16 T44_address;

	u16 T46_address;
	u16 T47_address;
	u8 T48_reportid;
	u16 T56_address;
	u16 T65_address;
	u16 T71_address;
	u16 T72_address;

	u16 T93_address;
	u8 T93_reportid;
	u8 g_tap_cnt;

	u8 panel_type;

	u16 T100_address;
	u8 T100_reportid_min;
	u8 T100_reportid_max;

	u16 T109_address;
	u8 T109_reportid;

	u16 T80_address;
	u8 T80_reportid;

	struct mxt_channel_size channel_size;
	struct mxt_ref_limit ref_limit;
	u8 ref_chk;

	/* for fw update in bootloader */
	struct completion bl_completion;

	/* for reset handling */
	struct completion reset_completion;

	/* for reset handling */
	struct completion crc_completion;

	/* Auto touch test */
	struct completion t25_completion;

	/* Enable reporting of input events */
	bool enable_reporting;

	struct touch_data ts_data;

	/* Indicates whether device is in suspend */
	bool suspended;
	struct mxt_raw_data *rawdata;
	bool mxt_knock_on_enable;
	bool self_test_result;
	u8 self_test_status[4];
	u8 self_test_result_status;
	u16 reported_keycode;
	struct mxt_patch patch;
	struct mxt_finger fingers[MXT_MAX_FINGER];
	struct mxt_watchdog watchdog;
	bool button_lock;
	struct delayed_work work_button_lock;
	struct delayed_work work_palm_unlock;
	struct delayed_work work_deepsleep;
	struct delayed_work work_delay_cal;
	u8 work_deepsleep_enabled;
	u8 mfts_enable;
	u8 charging_mode; /*Charger mode in patch*/
	u8 palm;
	u16 anti_touch_area;
#if 0 /* remove unused variable */
	int **full_cap;
#endif
	/* qwindow_size */
	//u8 is_lpwg_report_enable;
	u8 mxt_multi_tap_enable;
	u8 lpwg_mode;
	u8 knock_on_mode;
	/* ATMEL SELF REFERENCE CHECK FOR E8 */

	u8 self_ref_chk[2];

	struct quickcover_size *qwindow_size;
	bool quickcover_enabled;

	struct hrtimer multi_tap_timer;
	struct work_struct multi_tap_work;

	/* T71 tmp using before firmware change */
	u8 t71_diff_using;
	u8 t71_diff_val[32];

	int incoming_call;
	int error;

	bool is_knockONonly;
	bool is_knockCodeDelay;
	bool mxt_mode_changed;
	bool delayed_cal;

	int sensor;
	int screen;
	int qcover;

	int stylus_in_a_row_cnt;        // Count of continuing stylus data
	int stylus_in_a_row_cnt_thr;    // if stylus_in_a_row_cnt_thr < sylus_in_a_row_cnt valut, notify on global valiant.
	int x_zitter;
	int y_zitter;

	u8 pm_state;
	u8 pen_support;

	int self_cap;
	u32 mfts_lpwg;

	struct pinctrl *ts_pinctrl;
	struct pinctrl_state *ts_pinset_state_active;
	struct pinctrl_state *ts_pinset_state_suspend;

	struct atmel_side_key side_key;
	struct atmel_glove_mode glove_mode;
	struct atmel_cover_mode cover_mode;
	struct atmel_stamina_mode stamina_mode;
	struct atmel_aod_mode aod_mode;
	struct atmel_sod_mode sod_mode;

	bool after_work;
	bool charge_out;
	bool first_unblank;
	bool wireless_charge;
	bool landscape;

	u8 register_value;

	u8 touch_type;

	enum log_level_list log_level;

#if SIDE_KEY_DEVICE
	int side_key_storage[MXT_MAX_FINGER];
#endif
	bool report_flag[MXT_MAX_FINGER_IN_FIRMWARE];
	struct mxt_driver_data	*mxt_drv_data;
};

int mxt_initialize_t9_input_device(struct mxt_data *data);
int mxt_initialize_t100_input_device(struct mxt_data *data);
int mxt_request_firmware_work(const struct firmware *fw,void *context);
int mxt_write_mem(struct mxt_data *data, u16 reg, u16 len, const u8 *buf);
int mxt_read_mem(struct mxt_data *data, u16 reg, u16 len, void *buf);
int mxt_write_object(struct mxt_data *data, u8 type, u8 offset, u8 val);
struct mxt_object *mxt_get_object(struct mxt_data *data, u8 type);
int mxt_read_object(struct mxt_data *data, u8 type, u8 offset, u8 *val);
int mxt_update_firmware(struct mxt_data *data, const struct firmware *fw);
int mxt_get_reference_chk(struct mxt_data *data);

#endif /* __LINUX_ATMEL_MXT_TS_H__ */
