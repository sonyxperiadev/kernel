/*
 * Copyright (C) 2010 - 2017 Novatek, Inc.
 *
 * $Revision: 22429 $
 * $Date: 2018-01-30 19:42:59 +0800 (Tue, 30 Jan 2018) $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */
#ifndef _LINUX_NVT_TOUCH_H
#define _LINUX_NVT_TOUCH_H

#include <linux/i2c.h>
#include <linux/input.h>
#include "smx3_touch_log.h"
#include "nt36xxx_mem_map.h"

#define NVT_DEBUG 0

/*---For SMx3 use start---*/
extern int32_t smx3_probe_log_proc_init(void);
extern int32_t Init_BootLoader(void);
extern int32_t Resume_PD(void);
extern int32_t nvt_read_project_info(void);
/*---Flash ID (Project info) read--- */
#define NVT_PROJECT_INFO_READ 1
#define PROJECT_INFO_LEN 8
#define PROJECT_INFO_LEN_VALID 5
#define PROJECT_INFO_ADDR 0x1E000

/*---Label to check different module source */
#define TP_MODULE_PID_SM13	0x01
#define TP_MODULE_PID_SM23	0x02
#define TP_MODULE_VE_TR		0x01
#define TP_MODULE_VE_TM		0x02
#define TP_MODULE_CG_KM		0x01
#define TP_MODULE_CG_TR		0x02
#define TP_MODULE_CG_TM		0x03
#define TP_MODULE_CG_08		0x01
#define TP_MODULE_CG_10		0x02
#define TP_MODULE_CG_BLACK	0x01
#define TP_MODULE_DEFAULT	0xFF

#define TP_SOURCE_TR_M08	0x01 /* SM13 Final module start */
#define TP_SOURCE_TR_M10	0x02
#define TP_SOURCE_TR_S08	0x03 /* SM13 Final module end */
#define TP_SOURCE_TM		0x11 /* SM23 Final module */
#define TP_SOURCE_UNKNOWN	0xFF

#define TP_READ_LCDID_PIN 55
#define TP_DRIVER_VER 0x0001
#define TP_DISABLE_BTN 1

/* for Feature Integration */
#define NVT_DOZE_MODE_FINC 1
#define NVT_IRQ_CONTROL 1

#define NVT_LIBSYSINFO 1

/*---For SMx3 use end---*/

//---GPIO number---
#define NVTTOUCH_RST_PIN 980
#define NVTTOUCH_INT_PIN 45

//---INT trigger mode---
//#define IRQ_TYPE_EDGE_RISING 1
//#define IRQ_TYPE_EDGE_FALLING 2
#define INT_TRIGGER_TYPE IRQ_TYPE_EDGE_FALLING


//---I2C driver info.---
#define NVT_I2C_NAME "NVT-ts"
#define I2C_BLDR_Address 0x01
#define I2C_FW_Address 0x01
#define I2C_HW_Address 0x62

//---Input device info.---
#define NVT_TS_NAME "NVTCapacitiveTouchScreen"


//---Touch info.---
#define TOUCH_DEFAULT_MAX_WIDTH 1080
#define TOUCH_DEFAULT_MAX_HEIGHT 2520
#define TOUCH_MAX_FINGER_NUM 10
#define TOUCH_FORCE_NUM 1000

//---Customerized func.---
#define NVT_TOUCH_PROC 1
#define NVT_TOUCH_EXT_PROC 1
#define NVT_TOUCH_MP 1
#define MT_PROTOCOL_B 1

#define BOOT_UPDATE_FIRMWARE 1
#define BOOT_UPDATE_FIRMWARE_NAME "novatek_ts_fw.bin"

/* for DP */
#define BOOT_UPDATE_FIRMWARE_NAME_TR_M08 "SM13_KM_08_PID5403.bin"
#define BOOT_UPDATE_FIRMWARE_NAME_TR_M10 "SM13_KM_08_PID5403.bin"
#define BOOT_UPDATE_FIRMWARE_NAME_TR_S08 "SM13_KM_08_PID5403.bin"
#define BOOT_UPDATE_FIRMWARE_NAME_TM "SM23_TM_08_PID5404.bin"

struct nvt_ts_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct work_struct nvt_work;
	struct work_struct nvt_fwu_work;
	uint16_t addr;
	int8_t phys[32];
#if defined(CONFIG_FB)
	struct notifier_block fb_notif;
#endif
	uint8_t fw_ver;
	uint8_t x_num;
	uint8_t y_num;
	uint16_t abs_x_max;
	uint16_t abs_y_max;
	uint8_t max_touch_num;
	uint8_t max_button_num;
	uint32_t int_trigger_type;
	int32_t irq_gpio;
	uint32_t irq_flags;
	int32_t reset_gpio;
	uint32_t reset_flags;
	struct mutex lock;
#if NVT_IRQ_CONTROL
	struct mutex irq_lock;
#endif
	const struct nvt_ts_mem_map *mmap;
	uint8_t carrier_system;
	uint16_t nvt_pid;
/*---For SMx3 use start---*/
	struct kobject *touchscreen_link;
	uint8_t mb_hw_id;
	uint8_t tp_source;
	int lcd_id;
#if NVT_PROJECT_INFO_READ
	uint8_t project_info[PROJECT_INFO_LEN];
#endif
#if BOOT_UPDATE_FIRMWARE
	uint8_t sw_fw_ver;
#endif
/*---For SMx3 use end---*/
};

#if NVT_TOUCH_PROC
struct nvt_flash_data{
	rwlock_t lock;
	struct i2c_client *client;
};
#endif

typedef enum {
	RESET_STATE_INIT = 0xA0,// IC reset
	RESET_STATE_REK,		// ReK baseline
	RESET_STATE_REK_FINISH,	// baseline is ready
	RESET_STATE_NORMAL_RUN,	// normal run
	RESET_STATE_MAX  = 0xAF
} RST_COMPLETE_STATE;

typedef enum {
	EVENT_MAP_HOST_CMD                      = 0x50,
	EVENT_MAP_HANDSHAKING_or_SUB_CMD_BYTE   = 0x51,
	EVENT_MAP_RESET_COMPLETE                = 0x60,
	EVENT_MAP_FWINFO                        = 0x78,
	EVENT_MAP_PROJECTID                     = 0x9A,
} I2C_EVENT_MAP;

//---extern structures---
extern struct nvt_ts_data *ts;

//---extern functions---
extern int32_t CTP_I2C_READ(struct i2c_client *client, uint16_t address, uint8_t *buf, uint16_t len);
extern int32_t CTP_I2C_WRITE(struct i2c_client *client, uint16_t address, uint8_t *buf, uint16_t len);
extern void nvt_bootloader_reset(void);
extern void nvt_sw_reset_idle(void);
extern int32_t nvt_check_fw_reset_state(RST_COMPLETE_STATE check_reset_state);
extern int32_t nvt_get_fw_info(void);
extern int32_t nvt_clear_fw_status(void);
extern int32_t nvt_check_fw_status(void);
extern void nvt_stop_crc_reboot(void);
#endif /* _LINUX_NVT_TOUCH_H */
