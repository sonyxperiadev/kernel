/******************************************************************************
 * isl290xx_common.h - Linux kernel module for intersil ambient light sensor
 *		and proximity sensor
 *
 *
 ********************************************************/

#ifndef __ISL290XX_COMMON_H__
#define __ISL290XX_COMMON_H__

#define ISL290XX_IOCTL_MAGIC	0XCF
#define ISL290XX_IOCTL_ALS_ON	_IO(ISL290XX_IOCTL_MAGIC, 1)
#define ISL290XX_IOCTL_ALS_OFF	_IO(ISL290XX_IOCTL_MAGIC, 2)
#define ISL290XX_IOCTL_ALS_DATA	_IOR(ISL290XX_IOCTL_MAGIC, 3, short)
#define ISL290XX_IOCTL_ALS_CALIBRATE	_IO(ISL290XX_IOCTL_MAGIC, 4)
#define ISL290XX_IOCTL_CONFIG_GET	_IOR(ISL290XX_IOCTL_MAGIC, 5, \
					struct isl290xx_cfg_s)
#define ISL290XX_IOCTL_CONFIG_SET	_IOW(ISL290XX_IOCTL_MAGIC, \
					6, struct isl290xx_cfg_s)
#define ISL290XX_IOCTL_PROX_ON	_IO(ISL290XX_IOCTL_MAGIC, 7)
#define ISL290XX_IOCTL_PROX_OFF		_IO(ISL290XX_IOCTL_MAGIC, 8)
#define ISL290XX_IOCTL_PROX_DATA	_IOR(ISL290XX_IOCTL_MAGIC, \
					9, struct isl290xx_prox_info_s)
#define ISL290XX_IOCTL_PROX_EVENT	_IO(ISL290XX_IOCTL_MAGIC, 10)
#define ISL290XX_IOCTL_PROX_CALIBRATE	_IO(ISL290XX_IOCTL_MAGIC, 11)
#define ISL290XX_IOCTL_PROX_GET_ENABLED	_IOR(ISL290XX_IOCTL_MAGIC, \
					12, int*)
#define ISL290XX_IOCTL_ALS_GET_ENABLED	_IOR(ISL290XX_IOCTL_MAGIC, \
					13, int*)

#define u32 unsigned int
#define u16 unsigned short
#define u8  unsigned char

struct isl290xx_cfg_s {
	u8 prox_threshold_hi;
	u8 prox_threshold_lo;
	u8 prox_threshold_hi_def;
	u8 prox_threshold_lo_def;
	u8 aps_config;
	u8 aps_config1;
	u8 aps_config2;
	u8 aps_intr;
	u8 prox_offset;
	u8 prox_boot_cali;
	u8 scale_factor;
};

struct isl290xx_prox_info_s {
	u8 prox_clear;
	u8 prox_data;
	int prox_event;
};

#endif
