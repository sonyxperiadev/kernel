/* SPDX-License-Identifier: GPL-2.0-only WITH Linux-syscall-note */
/*
 * Copyright 2021 Sony Corporation
 */

#ifndef __LINUX_SONY_CAMERA_H
#define __LINUX_SONY_CAMERA_H

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#include <limits.h>
#endif
#include <linux/time.h>

#define SONY_CAMERA_MAX_I2C_DATA                   (256)
#define SONY_CAMERA_MAX_EEPROM_DATA                (8192)
#define SONY_CAMERA_MAX_TEMPERATURE_NUM_PER_CAMERA (3)

#define SONY_CAMERA_CMD_BASE              (0x10000)
#define SONY_CAMERA_CMD_POWER_UP          (SONY_CAMERA_CMD_BASE + 0)
#define SONY_CAMERA_CMD_POWER_DOWN        (SONY_CAMERA_CMD_BASE + 1)
#define SONY_CAMERA_CMD_I2C_READ          (SONY_CAMERA_CMD_BASE + 2)
#define SONY_CAMERA_CMD_I2C_WRITE         (SONY_CAMERA_CMD_BASE + 3)
#define SONY_CAMERA_CMD_I2C_READ_2        (SONY_CAMERA_CMD_BASE + 4)
#define SONY_CAMERA_CMD_I2C_WRITE_2       (SONY_CAMERA_CMD_BASE + 5)
#define SONY_CAMERA_CMD_SPI_READ          (SONY_CAMERA_CMD_BASE + 6)
#define SONY_CAMERA_CMD_SPI_WRITE         (SONY_CAMERA_CMD_BASE + 7)
#define SONY_CAMERA_CMD_SPI_READ_2        (SONY_CAMERA_CMD_BASE + 8)
#define SONY_CAMERA_CMD_SPI_WRITE_2       (SONY_CAMERA_CMD_BASE + 9)
#define SONY_CAMERA_CMD_DOWNLOAD_FW       (SONY_CAMERA_CMD_BASE + 10)
#define SONY_CAMERA_CMD_READ_FLASH        (SONY_CAMERA_CMD_BASE + 11)
#define SONY_CAMERA_CMD_WAIT_IDLE         (SONY_CAMERA_CMD_BASE + 12)
#define SONY_CAMERA_CMD_GET_EVENT         (SONY_CAMERA_CMD_BASE + 13)
#define SONY_CAMERA_CMD_SET_THERMAL       (SONY_CAMERA_CMD_BASE + 14)
#define SONY_CAMERA_CMD_CLEAR             (SONY_CAMERA_CMD_BASE + 15)
#define SONY_CAMERA_CMD_BUG_ON            (SONY_CAMERA_CMD_BASE + 16)
#define SONY_CAMERA_CMD_GET_POWER_STATE   (SONY_CAMERA_CMD_BASE + 17)
#define SONY_CAMERA_CMD_SET_POWER         (SONY_CAMERA_CMD_BASE + 18)
#define SONY_CAMERA_CMD_START_SOF_EVENT   (SONY_CAMERA_CMD_BASE + 19)
#define SONY_CAMERA_CMD_STOP_SOF_EVENT    (SONY_CAMERA_CMD_BASE + 20)

#define SONY_CAMERA_EVT_BASE           (0x20000)
#define SONY_CAMERA_EVT_SOF            (SONY_CAMERA_EVT_BASE + 0)
#define SONY_CAMERA_EVT_EXTERNAL       (SONY_CAMERA_EVT_BASE + 1)

enum sony_camera_sub_device_type {
	SONY_CAMERA_SUB_DEVICE_TYPE_SENSOR,
	SONY_CAMERA_SUB_DEVICE_TYPE_EXTERNAL_ISP,
	SONY_CAMERA_SUB_DEVICE_TYPE_FLASH,
	SONY_CAMERA_SUB_DEVICE_TYPE_OIS,
	SONY_CAMERA_SUB_DEVICE_TYPE_MAX,
};

enum sony_camera_i2c_freq_mode {
	SONY_CAMERA_I2C_FREQ_MODE_STAND,
	SONY_CAMERA_I2C_FREQ_MODE_FAST,
	SONY_CAMERA_I2C_FREQ_MODE_CUSTOM,
	SONY_CAMERA_I2C_FREQ_MODE_FAST_PLUS,
	SONY_CAMERA_I2C_FREQ_MODE_MAX,
};

enum sony_camera_i2c_type {
	SONY_CAMERA_I2C_TYPE_INVALID,
	SONY_CAMERA_I2C_TYPE_BYTE,
	SONY_CAMERA_I2C_TYPE_WORD,
	SONY_CAMERA_I2C_TYPE_3B,
	SONY_CAMERA_I2C_TYPE_MAX,
};

enum sony_camera_cmd {
	SONY_CAM_VDIG    =  0,
	SONY_CAM_VIO     =  1,
	SONY_CAM_VANA    =  2,
	SONY_CAM_VAF     =  3,
	SONY_CAM_VAF2    =  4,
	SONY_CAM_CLK     =  5,
	SONY_CAM_VMDR    =  6,
	SONY_GPIO_RESET  =  7,
	SONY_I2C_WRITE   =  8,
	SONY_CAM_GYRO    =  9,
	SONY_MIPI_SWITCH = 10,
	SONY_CAM_OIS     = 11,
	SONY_CAM_OIS2    = 12,
	SONY_GPIO_VANA   = 13,
	SONY_CAM_CLK2    = 14,
	SONY_CAM_CLK3    = 15,
	SONY_GPIO_RESET2 = 16,
	SONY_SPI_SET     = 17,
	SONY_CAM_VDIG2   = 20,
	SONY_CAM_VDIG3   = 21,
	SONY_CAM_VDIG4   = 22,
	SONY_CAM_VANA2   = 30,
	SONY_PMIC_CS0    = 40,
	SONY_PMIC_CS1    = 41,
	SONY_PMIC_VIN    = 42,
	SONY_PMIC_VDD0   = 43,
	SONY_PMIC_VDD1   = 44,
	SONY_CAM_ISP1    = 50,
	SONY_CAM_ISP2    = 51,
	SONY_CAM_ISP3    = 52,
	SONY_CAM_ISP4    = 53,
	SONY_CAM_ISP5    = 54,
	SONY_CAM_EXTIO   = 60,
	SONY_PROBE_WAIT  = 97,
	SONY_PROBE_WAIT2 = 98,
	EXIT             = 99,
};

struct sony_camera_i2c_data {
	uint32_t	slave_addr;
	uint32_t	addr;
	uint32_t	addr_type;
	uint32_t	len;
	uint8_t		data[SONY_CAMERA_MAX_I2C_DATA];
};

struct sony_camera_spi_data {
	uint32_t	header_size;
	void		*header;
	uint32_t	payload_size;
	void		*payload;
};

struct sony_camera_write_flash_data {
	uint32_t	addr;
	uint32_t	size;
	void		*data;
};

struct sony_camera_nor_flash_data {
	uint32_t	addr;
	uint32_t	size;
	void		*buf;
};

struct sony_camera_eeprom_data {
	uint8_t		data[SONY_CAMERA_MAX_EEPROM_DATA];
};

struct sony_camera_event_sof_data {
	uint32_t	      sof_count;
#ifdef __KERNEL__
	struct timespec64 mono_timestamp;
#else
	struct timespec mono_timestamp;
#endif
};

struct sony_camera_event_data {
	uint32_t	type;
	struct	sony_camera_event_sof_data sof_data;
};

struct sony_camera_thermal_data {
	int32_t	temperature[SONY_CAMERA_MAX_TEMPERATURE_NUM_PER_CAMERA];
};

struct sony_camera_power_cmd {
	uint32_t	cmd;
	uint32_t	arg;
};

static void sony_camera_log(uint8_t log_type, const char *func,
	const int line, const char *fmt, ...);

#endif /* __LINUX_SONY_CAMERA_H */
