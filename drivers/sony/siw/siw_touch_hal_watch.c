/*
 * siw_touch_hal_watch.c - SiW touch hal driver for ABT
 *
 * Copyright (C) 2016 Silicon Works - http://www.siliconworks.co.kr
 * Copyright (C) 2018 Sony Mobile Communications Inc.
 * Author: Hyunho Kim <kimhh@siliconworks.co.kr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#include "siw_touch_cfg.h"

#if defined(__SIW_SUPPORT_WATCH)	//See siw_touch_cfg.h

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/atomic.h>
#include <linux/syscalls.h>
#include <linux/file.h>

#include "siw_touch.h"
#include "siw_touch_hal.h"
#include "siw_touch_bus.h"
#include "siw_touch_event.h"
#include "siw_touch_gpio.h"
#include "siw_touch_irq.h"
#include "siw_touch_sys.h"

#define __SUPPORT_WATCH_CTRL_ACCESS

#define __SUPPORT_WATCH_FONT_STS_CHK

/*
 * [Notice : __SUPPORT_WATCH_CTRL_ACCESS]
 * This chipset has watch function in its spec,
 * but its concept has been changed differently than before.
 * - The role for watch control including font download is mainly managed by MIPI.
 * - This watch layer supports 'config_fontdata'
 *   but this is just a kind of 'Plan B' for test or debugging
 *   when the MIPI control for font download doesn't work well.
 */
#if defined(CONFIG_TOUCHSCREEN_SIW_SW49408)
#undef	__SUPPORT_WATCH_CTRL_ACCESS
#undef	__SUPPORT_WATCH_FONT_STS_CHK
#endif

#define SIW_MAX_FONT_SIZE			(6<<10)

#define SIW_FONT_MAGIC_CODE			(0x3EEAFEEB)
#define SIW_FONT_MAGIC_CODE_SIZE	(4)

enum {
	EXT_WATCH_CFG_DEBUG = 'd',
};

enum {
	EXT_WATCH_LUT_NUM	= 7,
	EXT_WATCH_LUT_MAX	= 7,
};

enum {
	NOT_SUPPORT = 0,
	SUPPORT,
};

enum {
	UNBLOCKED = 0,
	BLOCKED,
};

enum {
	EXT_WATCH_OFF = 0,
	EXT_WATCH_ON,
};

enum {
	EXT_WATCH_RTC_START = 1,
	EXT_WATCH_RTC_STOP = 2,
};

enum {
	EXT_WATCH_SET_RTC_COUNT	= 305,
	EXT_WATCH_SET_RTC_ECNT	= 32764,
};

enum {
	FONT_MEM_CRC = (1<<8),
};

enum {
	FONT_EMPTY = 0,
	FONT_DOWNLOADING,
	FONT_READY,
};

enum {
	RTC_CLEAR = 0,
	RTC_RUN,
};

#define WATCH_TYPE(_watch)			((_watch->type) & 0xFFFF)
#define WATCH_TYPE_VAR(_watch)		(((_watch->type) >> 16) & 0xFF)

#define WATCH_FLAG(_watch)			(_watch->flag)

#define FONT_TYPE(_watch)			(_watch->font_type)

struct ext_watch_font_header {
	u32 magic_code;
	u8 width_num;
	u8 width_colon;
	u8 height;
	u8 font_id;
	u32 size;
};

__packed struct ext_watch_query_font_data {
	bool Font_supported;	/* 0:not supported, 1:supported */
	u8 max_font_x_size;		/* 1~9 number X max size. */
	u8 max_font_y_size;		/* 1~9 number Y max size. */
	u8 max_cln_x_size;		/* ":" X max size. (ex. 23:47) */
	u8 max_cln_y_size;		/* ":" Y max size. (ex. 23:47) */
};

struct ext_watch_query_font_position {	/* 0:not supported, 1:supported */
	bool vertical_position_supported;
	bool horizontal_position_supported;
};

struct ext_watch_query_font_time {	/* 0:not supported, 1:supported */
	bool h24_supported;
	bool ampm_supported;
};

__packed struct ext_watch_query_font_color {	/* 0:not supported, 1:supported */
	u8 max_num;		/* The number of LUT */
	bool lut_supported;
	bool alpha_supported;
	bool gradation_supported;
};

__packed struct ext_watch_query_font_effect {
	bool zero_supported;	/* 0:display off, 1:display on */
	u8 blink_type;			/* 0:blink disable, 1:500ms, 2:1sec, 3:2sec. */
};

struct ext_watch_config_font_effect_blink {
	u32	blink_type; /* 0:off, 1:125ms, 2:250ms, 3:500ms, 4:1s, 5:2sec. 6:4s, 7:8s */
	u32	bstartx; /* blink startx: watstartx <= bstartx <= bendx */
	u32	bendx; /* blink end: bendx <= watendx */
};

struct ext_watch_config_font_effect {
	u32	len;
	u32	watchon;				/* 0:watch off, 1:watch on */
	u32	h24_en;					/* 0:12 hour display, 1:24 hour display */
	u32	zero_disp;				/* 0:display off, 1:display on */
	u32	clock_disp_type;		/* 0:hour and min, 1:min and sec */
	u32	midnight_hour_zero_en;	/* 0: 12:00 mode, 1: 00:00 */
	struct ext_watch_config_font_effect_blink blink;	/*for blink effect */
};

struct ext_watch_config_font_lut {
	/* LUT */
	u32	rgb_blue;
	u32	rgb_green;
	u32	rgb_red;
};

struct ext_watch_config_font_prop {
	u32	len;
	u32	max_num;		/* The number of LUT */
	struct ext_watch_config_font_lut lut[EXT_WATCH_LUT_NUM];
};

struct ext_watch_config_font_pos {
	u32 len;
	u32	watstartx;
	u32	watendx;
	u32	watstarty;
	u32	watendy;
	/* relative position in watch area */
	u32	h1x_pos;	/* 1 ~ 9hour position */
	u32	h10x_pos;	/* 10, 20 hour position */
	u32	m1x_pos;	/* 1 ~ 9min position */
	u32	m10x_pos;	/* 10 ~ 50 min position */
	u32	clx_pos;	/* colon position */
};

struct ext_watch_config_time_sync {	/* to sync with AP's current time */
	u32	len;
	u32	rtc_cwhour;		/* for hour */
	u32	rtc_cwmin;		/* for min */
	u32	rtc_cwsec;		/* for sec */
	u32	rtc_cwmilli;	/* for millisecond */
};

__packed struct ext_watch_bits_ctrl {	/* 0x2D2 */
	u32	reserved0:1;
	u32	dispmode:1;	/* 0:U2, 1:AnyMode */
	u32	reserved1:5;
	u32	alpha:9;
};

__packed struct ext_watch_bits_area { 	/* 0x2D3, 0x2D4 */
	u32	watstart:12;
	u32	watend:12;
};

__packed struct ext_watch_bits_blink_area { /* 0x2D5 */
	u32 bstartx:12;
	u32 bendx:12;
};

__packed struct ext_watch_bits_lut {	/* 0x2D6 */
	u8 b;
	u8 g;
	u8 r;
	u8 rssvd0;
};

__packed struct ext_watch_bits_time {
	u32 hour:5;
	u32 min:6;
	u32 sec:6;
	u32 rsvd0:15;
};

__packed struct ext_watch_cfg_mode {	/* 36 bytes */
	struct ext_watch_bits_ctrl watch_ctrl;				/* 2 bytes */
	struct ext_watch_bits_area watch_area_x;			/* 3 bytes */
	struct ext_watch_bits_area watch_area_y;			/* 3 bytes */
	struct ext_watch_bits_blink_area blink_area;		/* 3 bytes */
	struct ext_watch_bits_lut __lut[EXT_WATCH_LUT_NUM];	/* 21 bytes */
	struct ext_watch_bits_lut *lut;
};

__packed struct ext_watch_cfg_time {	/* 36 bytes */
	u32 disp_waton;						/* 0xC10 watch display off:0, on:1*/
	struct ext_watch_bits_time rtc_sct;	/* 0x081 */
	u32 rtc_sctcnt;						/* 0x082 */
	u32 rtc_mit;						/* 0x083 */
	u32 rtc_capture;					/* 0x084 */
	struct ext_watch_bits_time rtc_ctst;	/* 0x087 */
	u32 rtc_ecnt;						/* 0x088 */
};

enum {
	POS_MAX_H10X	= (1<<9),
	POS_MAX_H1X		= (1<<9),
	POS_MAX_M10X	= (1<<9),
	POS_MAX_M1X		= (1<<9),
	POS_MAX_CLX		= (1<<9),
};

/* relative position in watch area */
__packed struct ext_watch_cfg_position {	/* 0xC11 */
	u32 h10x_pos:9;
	u32 h1x_pos:9;
	u32 reserved0:14;
	/* */
	u32 m10x_pos:9;
	u32 m1x_pos:9;
	u32 reserved1:14;
	/* */
	u32 clx_pos:9;
	u32 reserved2:23;
	/* */
	u32 zero_disp:1;
	u32 h24_en:1;
	u32 clock_disp_mode:1;
	u32 midnight_hour_zero_en:1;
	u32 reserved3:28;
	/* */
	u32 bhprd:3;
	u32 reserved4:29;
	/* */
	u32 num_width:8;
	u32 colon_width:8;
	u32 height:8;
	u32 font_id:8;
};

static inline int __check_pos_not_set(struct ext_watch_cfg_position *position)
{
	return !(position->m1x_pos);
}

__packed struct ext_watch_cfg_status {	/* 0x270*/
	u32 step:3;
	u32 en:1;
	u32 en_24:1;
	u32 zero_en:1;
	u32 disp_mode:1;
	u32 bhprd:3;
	u32 cur_hour:5;
	u32 cur_min:6;
	u32 cur_sec:6;
	u32 midnight_hour_zero_en:1;
	u32 reserved0:4;
};

#if defined(__SUPPORT_WATCH_CTRL_ACCESS)
struct ext_watch_cfg {
	u8 *font_data;
	u32 magic_code;
	u32 font_crc;
	struct ext_watch_cfg_mode mode;
	struct ext_watch_cfg_time time;
	struct ext_watch_cfg_position position;
};

struct watch_state_info {
	atomic_t font_status;
	atomic_t rtc_status;
	struct ext_watch_cfg_mode mode_r;
	struct ext_watch_cfg_status status_r;
	struct ext_watch_cfg_position position_r;
};
#else	/* __SUPPORT_WATCH_CTRL_ACCESS */
struct ext_watch_cfg {
	u8 *font_data;
	u32 magic_code;
	u32 font_crc;
	struct ext_watch_cfg_time time;
};

struct watch_state_info {
	atomic_t font_status;
	atomic_t rtc_status;
};
#endif	/* __SUPPORT_WATCH_CTRL_ACCESS */

struct watch_data {
	struct watch_state_info state;
	struct bin_attribute fontdata_attr;
	int font_written_size;
	int font_max_size;
	int font_type;
	int font_crc_offset;
	u32 font_magic_code;
	struct reset_area *win_fixed;
	/* for TOUCH_USE_FONT_BINARY only */
	int font_num;
	int font_idx;
	struct ext_watch_cfg ext_wdata;
	int flag;
#define _WATCH_FLAG_SKIP_GET_MODE		(1UL<<0)
#define _WATCH_FLAG_SKIP_SET_MODE		(1UL<<1)
#define _WATCH_FLAG_SKIP_LUT_UPDATE		(1UL<<2)
#define _WATCH_FLAG_SKIP_MEM_CTRL		(1UL<<3)
#define _WATCH_FLAG_SKIP_SET_TIME		(1UL<<4)
#define _WATCH_FLAG_SKIP_SET_POS		(1UL<<5)
#define _WATCH_FLAG_SKIP_SET_DISP		(1UL<<6)

	int type;
	/* */
	u32 dcst_offset;
	u32 dcst_data;
};

enum {
	WATCH_FLAG_SKIP_GET_MODE	= _WATCH_FLAG_SKIP_GET_MODE,
	WATCH_FLAG_SKIP_SET_MODE	= _WATCH_FLAG_SKIP_SET_MODE,
	WATCH_FLAG_SKIP_LUT_UPDATE	= _WATCH_FLAG_SKIP_LUT_UPDATE,
	WATCH_FLAG_SKIP_MEM_CTRL	= _WATCH_FLAG_SKIP_MEM_CTRL,
	WATCH_FLAG_SKIP_SET_TIME	= _WATCH_FLAG_SKIP_SET_TIME,
	WATCH_FLAG_SKIP_SET_POS		= _WATCH_FLAG_SKIP_SET_POS,
	WATCH_FLAG_SKIP_SET_DISP	= _WATCH_FLAG_SKIP_SET_DISP,
};

/* Disable all */
#define WATCH_FLAG_SKIP_FOR_DEFAULT		(0 |	\
										WATCH_FLAG_SKIP_GET_MODE |	\
										WATCH_FLAG_SKIP_SET_MODE |	\
										WATCH_FLAG_SKIP_LUT_UPDATE |	\
										WATCH_FLAG_SKIP_MEM_CTRL |	\
										WATCH_FLAG_SKIP_SET_TIME |	\
										WATCH_FLAG_SKIP_SET_POS |	\
										WATCH_FLAG_SKIP_SET_DISP |	\
										0)

enum {
	WATCH_TYPE_0 = 0,	/* LG4895 */
	WATCH_TYPE_1,		/* LG4946 */
};

#define SIW_WATCH_TAG 	"watch: "

#define t_watch_info(_dev, fmt, args...)	\
		__t_dev_info(_dev, SIW_WATCH_TAG fmt, ##args)

#define t_watch_err(_dev, fmt, args...)	\
		__t_dev_err(_dev, SIW_WATCH_TAG fmt, ##args)

#define t_watch_warn(_dev, fmt, args...)	\
		__t_dev_warn(_dev, SIW_WATCH_TAG fmt, ##args)

#define t_watch_dbg(_dev, fmt, args...)	\
		t_dev_dbg_watch(_dev, SIW_WATCH_TAG fmt, ##args)

#define siw_watch_snprintf(_buf, _buf_max, _size, _fmt, _args...) \
		__siw_snprintf(_buf, _buf_max, _size, _fmt, ##_args)


#define TS_MODULE "[watch]"

enum {
	FONT_TEMP_LOG_SZ = 256,
};

enum {
	WATCH_MAX_RW_SIZE = (SIW_TOUCH_MAX_BUF_SIZE>>1),
};


// LOOK UP TABLE for CRC16 generation
// Polynomial X^16+X^15+X^2+1
static const u16 ext_watch_crc16lut[] = {
	0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
	0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
	0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
	0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
	0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
	0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
	0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
	0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
	0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
	0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
	0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
	0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
	0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
	0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
	0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
	0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
	0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
	0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
	0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
	0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
	0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
	0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
	0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
	0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
	0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
	0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
	0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
	0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
	0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
	0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
	0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
	0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
};

#define __ret_val_blocked(_val)		(-EPERM)
//#define	__ret_val_blocked(_val)		(_val)

static int ext_quirk_check(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	unsigned long test_bit;

	test_bit = touch_test_quirks(ts, CHIP_QUIRK_NOT_SUPPORT_WATCH);
	if (test_bit)
		t_dev_dbg_base(dev, "%s watch control not supported\n",
			touch_chip_name(ts));

	return !!test_bit;
}

#if defined(__SUPPORT_WATCH_CTRL_ACCESS)

#if defined(CONFIG_TOUCHSCREEN_SIW_SW49407)
#define __SIW_WATCH_MODE_DCST
#endif

enum {
	DCST_RANGE_8BIT = 1,	// 32-bit : data[0:7] + rsvd[31:08]
	DCST_RANGE_16BIT,		// 32-bit x 2
	DCST_RANGE_24BIT,		// 32-bit x 3
	/* */
	DCST_ZERO_CHECK = (1<<16),
};

#if defined(__SIW_WATCH_MODE_DCST)
static int ext_watch_read_dcst(struct device *dev, u32 addr, u32 *value, int range)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
//	struct siw_ts *ts = chip->ts;
//	struct siw_hal_reg *reg = chip->reg;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	u32 data[4];
	u32 _value = 0;
	int i;
	int ret;

	if (!watch->dcst_offset || !watch->dcst_data) {
		t_watch_err(dev, "wrong dcst setup: o %Xh, d %Xh\n",
			watch->dcst_offset, watch->dcst_data);
		return -EFAULT;
	}

	ret = siw_hal_write_value(dev, watch->dcst_offset, addr);
	if (ret < 0) {
		t_watch_err(dev, "dcst: read offset err[%04Xh, 0x%X], %d\n",
				watch->dcst_offset, addr, ret);
		goto out;
	}

	ret = siw_hal_reg_read(dev, watch->dcst_data, data, sizeof(data[0]) * range);
	if (ret < 0) {
		t_watch_err(dev, "dcst: read data err[%04Xh], %d\n",
				watch->dcst_data, ret);
		goto out;
	}

	for (i = 0; i < range; i++) {
		_value |= (data[i] & 0xFF)<<(i<<3);
	}

	if (value != NULL)
		*value = _value;

out:
	return ret;
}

static int ext_watch_write_dcst(struct device *dev, u32 addr, u32 value, int range)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
//	struct siw_ts *ts = chip->ts;
//	struct siw_hal_reg *reg = chip->reg;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	u32 data[4];
	u32 _value = value;
	int i;
	int ret = 0;

	if (!watch->dcst_offset || !watch->dcst_data) {
		t_watch_err(dev, "wrong dcst setup: o %Xh, d %Xh\n",
			watch->dcst_offset, watch->dcst_data);
		return -EFAULT;
	}

	for (i = 0; i < range; i++) {
		data[i] = _value & 0xFF;
		_value >>= 8;
	}

	ret = siw_hal_write_value(dev, watch->dcst_offset, addr);
	if (ret < 0) {
		t_watch_err(dev, "dcst: write offset err[%04Xh, %Xh], %d\n",
				watch->dcst_offset, addr, ret);
		goto out;
	}

	ret = siw_hal_reg_write(dev, watch->dcst_data, data, sizeof(data[0]) * range);
	if (ret < 0) {
		t_watch_err(dev, "dcst: write data err[%04Xh, %Xh], %d\n",
				watch->dcst_data, value, ret);
		goto out;
	}

out:
	return ret;
}

#define WATCH_MODE_RD_STR			"get mode(dcst): "
#define WATCH_MODE_WR_STR			"set mode(dcst): "
#define WATCH_MODE_LUT_REG_GAP		3

static int ext_watch_do_mode_read(struct device *dev, u32 addr, u32 *value, int range)
{
	return ext_watch_read_dcst(dev, addr, value, range);
}

static int ext_watch_do_mode_write(struct device *dev, u32 addr, u32 value, int range)
{
	return ext_watch_write_dcst(dev, addr, value, range);
}
#else	/* __SIW_WATCH_MODE_DCST */
#define WATCH_MODE_RD_STR			"get mode: "
#define WATCH_MODE_WR_STR			"set mode: "
#define WATCH_MODE_LUT_REG_GAP		1

static int ext_watch_do_mode_read(struct device *dev, u32 addr, u32 *value, int range)
{
	return siw_hal_read_value(dev, addr, value);
}

static int ext_watch_do_mode_write(struct device *dev, u32 addr, u32 value, int range)
{
	return siw_hal_write_value(dev, addr, value);
}
#endif	/* __SIW_WATCH_MODE_DCST */

static int ext_watch_mode_read(struct device *dev, u32 addr, u32 *value,
			const char *name, int range)
{
	u32 _value;
	int zero_check = !!(range & DCST_ZERO_CHECK);
	int ret = 0;

	range &= 0xFF;

	if (zero_check & !value) {
		if (name != NULL)
			t_watch_warn(dev, WATCH_MODE_RD_STR "%s skip\n", name);
		return 0;
	}

	ret = ext_watch_do_mode_read(dev, addr, &_value, range);
	if (ret < 0) {
		goto out;
	}

	if (name != NULL) {
		switch (range) {
		case DCST_RANGE_24BIT:
			t_watch_info(dev, WATCH_MODE_RD_STR "%s %06Xh\n", name, _value);
			break;
		case DCST_RANGE_16BIT:
			t_watch_info(dev, WATCH_MODE_RD_STR "%s %04Xh\n", name, _value);
			break;
		default:
			t_watch_info(dev, WATCH_MODE_RD_STR "%s %02Xh\n", name, _value);
			break;
		}
	}

	if (value != NULL)
		*value = _value;

out:
	return ret;
}

static int ext_watch_mode_write(struct device *dev, u32 addr, u32 value,
			const char *name, int range)
{
	int zero_check = !!(range & DCST_ZERO_CHECK);

	range &= 0xFF;

	if (zero_check & !value) {
		if (name != NULL)
			t_watch_warn(dev, WATCH_MODE_WR_STR "%s skip\n", name);
		return 0;
	}

	if (name != NULL) {
		switch (range) {
		case DCST_RANGE_24BIT:
			t_watch_info(dev, WATCH_MODE_WR_STR "%s %06Xh\n", name, value);
			break;
		case DCST_RANGE_16BIT:
			t_watch_info(dev, WATCH_MODE_WR_STR "%s %04Xh\n", name, value);
			break;
		default:
			t_watch_info(dev, WATCH_MODE_WR_STR "%s %02Xh\n", name, value);
			break;
		}
	}

	return ext_watch_do_mode_write(dev, addr, value, range);
}

static int ext_watch_rtc_start(struct device *dev, u8 start)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_hal_reg *reg = chip->reg;
	u32 rtc_ctrl;
	int ret = 0;

	rtc_ctrl = (start == EXT_WATCH_RTC_START) ?
				EXT_WATCH_RTC_START :
				EXT_WATCH_RTC_STOP;

	ret = siw_hal_write_value(dev,
				reg->ext_watch_rtc_run,
				rtc_ctrl);
	if (ret < 0) {
		t_watch_err(dev, "failed to run rtc, %d\n", ret);
		goto out;
	}

	t_watch_info(dev, "rtc %s\n",
		(rtc_ctrl == EXT_WATCH_RTC_START) ? "on" : "off");

out:
	return ret;
}

static int ext_watch_mem_ctrl(struct device *dev, int on_off, const char *msg)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
//	struct siw_ts *ts = chip->ts;
	struct siw_hal_reg *reg = chip->reg;
	struct watch_data *watch = (struct watch_data *)chip->watch;
//	u32 value = !!on_off;
	int ret;

	if (WATCH_FLAG(watch) & WATCH_FLAG_SKIP_MEM_CTRL) {
		return 0;
	}

	ret = siw_hal_write_value(dev,
				reg->ext_watch_mem_ctrl,
		 		!!on_off);
	if (ret < 0) {
		return ret;
	}
	t_watch_info(dev, "%s, watch memory clk %s\n",
		msg, (on_off) ? "on" : "off");
	return 0;
}

static void __store_log_buf(struct device *dev, char *buf,
				char *log, int *buflen, int loglen)
{
	if (buf) {
		memcpy(&buf[*buflen], log, loglen);
		*buflen += loglen;
	} else {
		t_watch_info(dev, "%s", log);
	}
}

static int ext_watch_get_mode(struct device *dev, char *buf, int *len)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
//	struct siw_ts *ts = chip->ts;
	struct siw_hal_reg *reg = chip->reg;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	struct ext_watch_cfg_mode *mode_r = &watch->state.mode_r;
	struct ext_watch_bits_lut *lut;
	char log[FONT_TEMP_LOG_SZ] = {0, };
	int loglen = 0;
	int buflen = 0;
	u32 val;
	int i;
	int ret = 0;

	if (WATCH_FLAG(watch) & WATCH_FLAG_SKIP_GET_MODE) {
		return 0;
	}

	if (len)
		buflen = *len;

	ret = ext_watch_mode_read(dev, reg->ext_watch_ctrl, &val,
				"watch_ctrl", DCST_RANGE_16BIT);
	if (ret < 0) {
		goto out;
	}
	memcpy((void *)&mode_r->watch_ctrl, (void *)&val, sizeof(mode_r->watch_ctrl));
	loglen = siw_watch_snprintf(log, FONT_TEMP_LOG_SZ, 0,
					"watch_ctrl[%04Xh] dispmode %d, alpha %d\n",
					reg->ext_watch_ctrl,
					mode_r->watch_ctrl.dispmode,
					mode_r->watch_ctrl.alpha);

	__store_log_buf(dev, buf, log, &buflen, loglen);

	ret = ext_watch_mode_read(dev, reg->ext_watch_area_x, &val,
				"watch_area_x", DCST_RANGE_24BIT);
	if (ret < 0) {
		goto out;
	}
	memcpy((void *)&mode_r->watch_area_x, (void *)&val, sizeof(mode_r->watch_area_x));
	loglen = siw_watch_snprintf(log, FONT_TEMP_LOG_SZ, 0,
					"watch_area_x[%04Xh] sx %d, ex %d\n",
					reg->ext_watch_area_x,
					mode_r->watch_area_x.watstart,
					mode_r->watch_area_x.watend);

	__store_log_buf(dev, buf, log, &buflen, loglen);

	ret = ext_watch_mode_read(dev, reg->ext_watch_area_y, &val,
				"watch_area_y", DCST_RANGE_24BIT);
	if (ret < 0) {
		goto out;
	}
	t_watch_dbg(dev, "get mode: watch_area_y %Xh\n", val);
	memcpy((void *)&mode_r->watch_area_y, (void *)&val, sizeof(mode_r->watch_area_y));
	loglen = siw_watch_snprintf(log, FONT_TEMP_LOG_SZ, 0,
					"watch_area_y[%04Xh] sy %d, ey %d\n",
					reg->ext_watch_area_y,
					mode_r->watch_area_y.watstart,
					mode_r->watch_area_y.watend);

	__store_log_buf(dev, buf, log, &buflen, loglen);

	ret = ext_watch_mode_read(dev, reg->ext_watch_blink_area, &val,
				"blink_area", DCST_RANGE_24BIT);
	if (ret < 0) {
		goto out;
	}
	memcpy((void *)&mode_r->blink_area, (void *)&val, sizeof(mode_r->blink_area));
	loglen = siw_watch_snprintf(log, FONT_TEMP_LOG_SZ, 0,
					"blink_area[%04Xh] bsx %d, bex %d\n",
					reg->ext_watch_blink_area,
					mode_r->blink_area.bstartx,
					mode_r->blink_area.bendx);

	__store_log_buf(dev, buf, log, &buflen, loglen);

	mode_r->lut = mode_r->__lut;
	lut = mode_r->lut;
	for (i = 0; i < EXT_WATCH_LUT_NUM; i++) {
		ret = ext_watch_mode_read(dev,
				reg->ext_watch_lut + (WATCH_MODE_LUT_REG_GAP*i), &val,
				"lut", DCST_RANGE_24BIT);
		if (ret < 0) {
			goto out;
		}
		memcpy((void *)lut, (void *)&val, sizeof(*lut));
		loglen = siw_watch_snprintf(log, FONT_TEMP_LOG_SZ, 0,
					"LUT[%d][%04Xh] b %02X, g %02X, r %02X\n",
					i, reg->ext_watch_blink_area + i,
					lut->b, lut->g, lut->r);

		__store_log_buf(dev, buf, log, &buflen, loglen);

		lut++;
	}

	if (len)
		*len = buflen;

	return 0;

out:
	return ret;
}

static int ext_watch_get_position(struct device *dev, char *buf, int *len)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct siw_hal_reg *reg = chip->reg;
	struct touch_xfer_msg *xfer = ts->xfer;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	struct ext_watch_cfg_position *position_r = &watch->state.position_r;
	struct ext_watch_cfg_status *status_r = &watch->state.status_r;
	char log[FONT_TEMP_LOG_SZ] = {0, 0};
	int loglen = 0;
	int buflen = 0;
	int ret = 0;

#if 0
	if (!buf) {
		t_watch_err(dev, "get position: NULL buffer");
		ret = -EINVAL;
		goto out;
	}
#endif

	if (len)
		buflen = *len;

	siw_hal_xfer_init(dev, xfer);

	siw_hal_xfer_add_rx(xfer,
			reg->ext_watch_position_r,
			(void *)position_r, sizeof(u32)*3);

	siw_hal_xfer_add_rx(xfer,
			reg->ext_watch_state,
			(void *)status_r, sizeof(u32));

	ret = siw_hal_xfer_msg(dev, xfer);
	if (ret < 0) {
		goto out;
	}

	loglen = siw_watch_snprintf(log, FONT_TEMP_LOG_SZ, 0,
					"position[%04Xh ~ %04Xh] h10x %d, h1x %d, m10x %d, m1x %d, clx %d\n",
					reg->ext_watch_position_r,
					reg->ext_watch_position_r + 3,
					position_r->h10x_pos, position_r->h1x_pos,
		 			position_r->m10x_pos, position_r->m1x_pos,
		 			position_r->clx_pos);
	__store_log_buf(dev, buf, log, &buflen, loglen);

	position_r->zero_disp = status_r->zero_en;
	position_r->h24_en = status_r->en_24;
	position_r->clock_disp_mode = status_r->disp_mode;
	position_r->midnight_hour_zero_en = status_r->midnight_hour_zero_en;
	position_r->bhprd = status_r->bhprd;

	loglen = siw_watch_snprintf(log, FONT_TEMP_LOG_SZ, 0,
					"state[%04Xh] zero disp %d, 24H mode %d, clock mode %d\n",
					reg->ext_watch_state,
					position_r->zero_disp, position_r->h24_en,
					position_r->clock_disp_mode);

	__store_log_buf(dev, buf, log, &buflen, loglen);

	loglen = siw_watch_snprintf(log, FONT_TEMP_LOG_SZ, 0,
					"state[%04Xh] midnight mode %d, blink period %d\n",
					reg->ext_watch_state,
					position_r->midnight_hour_zero_en,
					position_r->bhprd);

	__store_log_buf(dev, buf, log, &buflen, loglen);

	loglen = siw_watch_snprintf(log, FONT_TEMP_LOG_SZ, 0,
					"state[%04Xh] step %d, enable %d\n",
					reg->ext_watch_state,
		 			status_r->step, status_r->en);

	__store_log_buf(dev, buf, log, &buflen, loglen);

	if (len)
		*len = buflen;

	return 0;

out:
	t_watch_err(dev, "get position: failed, %d\n", ret);
	return ret;
}

static int ext_watch_get_curr_time(struct device *dev, char *buf, int *len)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct siw_hal_reg *reg = chip->reg;
	struct touch_xfer_msg *xfer = ts->xfer;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	struct ext_watch_bits_time *rtc_ctst = &watch->ext_wdata.time.rtc_ctst;
	struct ext_watch_cfg_status status = {0, 0};
	char log[FONT_TEMP_LOG_SZ] = {0, 0};
	int loglen = 0;
	int buflen = 0;
	int ret = 0;

#if 0
	if (!buf) {
		t_watch_err(dev, "get curr time: NULL buffer");
		ret = -EINVAL;
		goto out;
	}
#endif

	if (len)
		buflen = *len;

	siw_hal_xfer_init(dev, xfer);

	siw_hal_xfer_add_rx(xfer,
			reg->ext_watch_state,
			(void *)&status, sizeof(u32));

	siw_hal_xfer_add_rx(xfer,
			reg->ext_watch_rtc_ctst,
			(void *)rtc_ctst, sizeof(u32));

	ret = siw_hal_xfer_msg(dev, xfer);
	if (ret < 0) {
		goto out;
	}

	loglen = siw_watch_snprintf(log, FONT_TEMP_LOG_SZ, 0,
					"state[%04Xh, %04Xh] display %02d:%02d:%02d, rtc %02d:%02d:%02d\n",
					reg->ext_watch_state, reg->ext_watch_rtc_ctst,
					status.cur_hour, status.cur_min, status.cur_sec,
	 				rtc_ctst->hour, rtc_ctst->min, rtc_ctst->sec);

	__store_log_buf(dev, buf, log, &buflen, loglen);

	if (len)
		*len = buflen;

	return 0;

out:
	t_watch_err(dev, "get curr time: failed, %d\n", ret);
	return ret;
}

static int ext_watch_get_dic_st(struct device *dev, char *buf, int *len)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
//	struct siw_ts *ts = chip->ts;
	struct siw_hal_reg *reg = chip->reg;
	u32 dic_status = 0;
	u32 watch_status = 0;
	char log[FONT_TEMP_LOG_SZ] = {0, };
	int loglen = 0;
	int buflen = 0;
	int ret = 0;

	if (len)
		buflen = *len;

	ret = siw_hal_read_value(dev,
				reg->sys_dispmode_status,
				&dic_status);
	if (ret < 0) {
		goto out;
	}

	ret = siw_hal_read_value(dev,
				reg->ext_watch_display_status,
				&watch_status);
	if (ret < 0) {
		goto out;
	}

	if (dic_status == 2) {
		loglen = siw_watch_snprintf(log, FONT_TEMP_LOG_SZ, 0,
						"display mode U2, watch display %s\n",
						(watch_status & 1) ? "on" : "off");
	} else {
		loglen = siw_watch_snprintf(log, FONT_TEMP_LOG_SZ, 0,
						"display mode U%d\n",
						dic_status);
	}

	__store_log_buf(dev, buf, log, &buflen, loglen);

	if (len)
		*len = buflen;

	return 0;

out:
	t_watch_err(dev, "get dic st: failed, %d\n", ret);

	return ret;
}

static int ext_watch_set_mode(struct device *dev, int log)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
//	struct siw_ts *ts = chip->ts;
	struct siw_hal_reg *reg = chip->reg;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	struct ext_watch_cfg_mode *mode = &watch->ext_wdata.mode;
	struct ext_watch_cfg_position *position = &watch->ext_wdata.position;
//	int watch_type = WATCH_TYPE(watch);
	u32 val = 0;
	int i;
	int ret = 0;

	if (WATCH_FLAG(watch) & WATCH_FLAG_SKIP_SET_MODE) {
		/* SKIP : all handled by MIPI, no effect */
		return 0;
	}

	if (__check_pos_not_set(position)) {
		t_watch_info(dev, "set mode: skip\n");
		return 0;
	}

	mode->watch_ctrl.alpha = 1;
//	mode->watch_ctrl.alpha = !!position->bhprd;

	val = 0;
	memcpy((void *)&val, (void *)&mode->watch_ctrl, sizeof(mode->watch_ctrl));
	ret = ext_watch_mode_write(dev, reg->ext_watch_ctrl, val,
				(log) ? "watch_ctrl" : NULL, DCST_RANGE_16BIT);
	if (ret < 0) {
		goto out;
	}

	val = 0;
	memcpy((void *)&val, (void *)&mode->watch_area_x, sizeof(mode->watch_area_x));
	ret = ext_watch_mode_write(dev, reg->ext_watch_area_x, val,
				(log) ? "watch_area_x" : NULL, DCST_ZERO_CHECK | DCST_RANGE_24BIT);
	if (ret < 0)
		goto out;

	val = 0;
	memcpy((void *)&val, (void *)&mode->watch_area_y, sizeof(mode->watch_area_y));
	ret = ext_watch_mode_write(dev, reg->ext_watch_area_y, val,
				(log) ? "watch_area_y" : NULL, DCST_ZERO_CHECK | DCST_RANGE_24BIT);
	if (ret < 0)
		goto out;

	val = 0;
	memcpy((void *)&val, (void *)&mode->blink_area, sizeof(mode->blink_area));
	ret = ext_watch_mode_write(dev, reg->ext_watch_blink_area, val,
				(log) ? "blink_area" : NULL, DCST_ZERO_CHECK | DCST_RANGE_24BIT);
	if (ret < 0)
		goto out;

	if ((WATCH_FLAG(watch) & WATCH_FLAG_SKIP_LUT_UPDATE)) {
		goto skip_lut;
	}

	if (mode->lut == NULL) {
		goto skip_lut;
	}

	val = 0;
	for (i = 0; i < EXT_WATCH_LUT_NUM; i++) {
		memcpy((void *)&val, (void *)&mode->lut[i], sizeof(mode->lut[0])-sizeof(u8));
		ret = ext_watch_mode_write(dev,
				reg->ext_watch_lut + (WATCH_MODE_LUT_REG_GAP*i), val,
				(log) ? "lut" : NULL, DCST_RANGE_24BIT);
		if (ret < 0)
			goto out;
	}

skip_lut:
	t_watch_info(dev, WATCH_MODE_WR_STR "done\n");

	return 0;

out:
	t_watch_err(dev, WATCH_MODE_WR_STR "failed, %d\n", ret);
	return ret;

}

static int ext_watch_set_curr_time(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct siw_hal_reg *reg = chip->reg;
	struct touch_xfer_msg *xfer = ts->xfer;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	struct ext_watch_cfg_time *time = &watch->ext_wdata.time;
//	struct ext_watch_cfg_status status = {0, 0};
	u32 rtc_ctrl = 0;
	u32 rtc_sctcnt = 0;
	u32 rtc_count = EXT_WATCH_SET_RTC_COUNT;		 /* 30.5 us */
	int ret = 0;

	if (WATCH_FLAG(watch) & WATCH_FLAG_SKIP_SET_TIME) {
		return 0;
	}

	time->rtc_ecnt = EXT_WATCH_SET_RTC_ECNT;
	rtc_sctcnt = (int)((time->rtc_sctcnt * rtc_count) / 10);
	rtc_ctrl = time->rtc_ecnt & 0xFFFF;

	ret = ext_watch_rtc_start(dev, EXT_WATCH_RTC_STOP);
	if (ret < 0) {
		goto out;
	}

	siw_hal_xfer_init(dev, xfer);

	siw_hal_xfer_add_tx(xfer,
			reg->ext_watch_rtc_sct,
			(void *)&time->rtc_sct, sizeof(u32));

	siw_hal_xfer_add_tx(xfer,
			reg->ext_watch_rtc_sctcnt,
			(void *)&rtc_sctcnt, sizeof(u32));

	siw_hal_xfer_add_tx(xfer,
			reg->ext_watch_rtc_ecnt,
			(void *)&rtc_ctrl, sizeof(u32));

	ret = siw_hal_xfer_msg(dev, xfer);
	if (ret < 0) {
		goto out;
	}

	t_watch_info(dev, "set time : %02d:%02d:%02d, clk[%d Hz], cnt %d\n",
		time->rtc_sct.hour,
		time->rtc_sct.min,
		time->rtc_sct.sec,
		time->rtc_ecnt,
		time->rtc_sctcnt);

	ret = ext_watch_rtc_start(dev, EXT_WATCH_RTC_START);
	if (ret < 0) {
		goto out;
	}

	atomic_set(&watch->state.rtc_status, RTC_RUN);

	return 0;

out:
	t_watch_err(dev, "set curr time, failed, %d\n", ret);
	return ret;
}

static int ext_watch_set_position(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
//	struct siw_ts *ts = chip->ts;
	struct siw_hal_reg *reg = chip->reg;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	struct ext_watch_cfg_position *position = &watch->ext_wdata.position;
	u8 *ptr = (u8 *)position;
	int i;
	int ret = 0;

	if (WATCH_FLAG(watch) & WATCH_FLAG_SKIP_SET_POS) {
		return 0;
	}

#if 0
	if (chip->lcd_mode != LCD_MODE_U3) {
		t_watch_info(dev, "lcd mode is %s, skip\n",
			siw_lcd_driving_mode_str(chip->lcd_mode));
		return 0;
	}
#endif

	if (__check_pos_not_set(position)) {
		t_watch_info(dev, "set position: skip\n");
		return 0;
	}

#if 1
	/*
	 * Single access for wide compatibility
	 * LG4946 can't support burst access
	 */
	for (i = 0; i < 5; i++) {
		ret = siw_hal_reg_write(dev,
				reg->ext_watch_position + i,
				(void *)&ptr[i<<2], sizeof(u32));
		if (ret < 0) {
			break;
		}
	}
#else
	ret = siw_hal_reg_write(dev,
				reg->ext_watch_position,
				(void *)position, sizeof(u32)*5);
#endif
	if (ret < 0) {
		goto out;
	}

	t_watch_info(dev, "set position: done\n");

	return 0;

out:
	t_watch_err(dev, "set position: failed, %d\n", ret);
	return ret;
}

static int __used __ext_watch_display_onoff_type_1(struct device *dev, u32 data)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
//	struct siw_ts *ts = chip->ts;
	struct siw_hal_reg *reg = chip->reg;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	int waton = data;
	int ret = 0;

	if (atomic_read(&watch->state.rtc_status) != RTC_RUN) {
		ret = ext_watch_set_curr_time(dev);
		if (ret < 0) {
			goto out;
		}
	}

	switch (chip->lcd_mode) {
	case LCD_MODE_U2:
	case LCD_MODE_U2_UNBLANK:
		if (waton) {
			ret = siw_hal_write_value(dev,
						reg->ext_watch_dcs_ctrl,
						waton);
			if (ret < 0) {
				goto out;
			}

			t_watch_info(dev, "display[%04Xh] : %s for U2\n",
				reg->ext_watch_dcs_ctrl,
				waton ? "on" : "off");
		}
		break;
	}

out:
	return ret;
}

static int __ext_watch_display_onoff(struct device *dev, u32 data)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
//	struct siw_ts *ts = chip->ts;
	struct siw_hal_reg *reg = chip->reg;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	struct ext_watch_cfg_time *time = &watch->ext_wdata.time;
	int watch_type = WATCH_TYPE(watch);
	int ret = 0;

	if (WATCH_FLAG(watch) & WATCH_FLAG_SKIP_SET_DISP) {
		return 0;
	}

	if (data == ~0) {
		data = time->disp_waton;
	}

	switch (watch_type) {
	case WATCH_TYPE_1:
		ret = __ext_watch_display_onoff_type_1(dev, data );
		if (ret < 0) {
			goto out;
		}
		break;
	}

	ret = siw_hal_write_value(dev,
				reg->ext_watch_display_on,
				data);
	if (ret < 0) {
		goto out;
	}

	t_watch_info(dev, "display[%04Xh] : %s\n",
		reg->ext_watch_display_on,
		data ? "on" : "off");

	return 0;

out:
	t_watch_err(dev, "failed to control display on/off(%d), %d\n",
	 	time->disp_waton, ret);
	return ret;
}

static int ext_watch_display_onoff(struct device *dev)
{
	return __ext_watch_display_onoff(dev, ~0);
}

static void ext_watch_display_off(struct device *dev)
{
	__ext_watch_display_onoff(dev, 0);
}

static int ext_watch_set_cfg(struct device *dev, char log)
{
	int ret = 0;

	ret = ext_watch_set_mode(dev, log);
	if (ret < 0) {
		goto out;
	}

	ret = ext_watch_set_position(dev);
	if (ret < 0) {
		goto out;
	}

out:
	return ret;
}
#else	/* __SUPPORT_WATCH_CTRL_ACCESS */
static int __used ext_watch_rtc_start(struct device *dev, u8 start)
{
	return 0;
}

static int __used ext_watch_mem_ctrl(struct device *dev, int on_off, const char *msg)
{
	return 0;
}

static int __used ext_watch_get_curr_time(struct device *dev, char *buf, int *len)
{
	return 0;
}

static int __used ext_watch_display_onoff(struct device *dev)
{
	return 0;
}

static void __used ext_watch_display_off(struct device *dev)
{

}

static int __used ext_watch_set_cfg(struct device *dev, char log)
{
	return 0;
}
#endif	/* __SUPPORT_WATCH_CTRL_ACCESS */

static u16 ext_watch_cal_crc16(const u16 *data, u32 size, u16 init_val)
{
	const u16 *crc16lut = ext_watch_crc16lut;
	u16 crc_sum = 0;
	u8 temp = 0;
	int i;

	crc_sum = init_val;
	for (i = 0; i < size; i += 2) {
		temp = (data[i]>>8) & 0xFF;
		crc_sum = (crc_sum<<8) ^ crc16lut[((crc_sum>>8)&0xFF)^temp];

		temp = data[i]&0xFF;
		crc_sum = (crc_sum<<8) ^ crc16lut[((crc_sum>>8)&0xFF)^temp];
	}

	return crc_sum;
}

static u32 ext_watch_font_crc_cal(char *data, u32 size)
{
	u32 crc_value = 0;

	/*CRC Calculation*/
	crc_value = ext_watch_cal_crc16((const u16*)&data[0], size>>1, 0);
	crc_value |= (ext_watch_cal_crc16((const u16*)&data[2], size>>1, 0) << 16);

	return crc_value & 0x3FFFFFFF;
}

#if defined(__SUPPORT_WATCH_FONT_STS_CHK)
static int ext_watch_do_chk_font_status(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_hal_reg *reg = chip->reg;
	u32 status = 0;
	int ret = 0;

	ret = siw_hal_read_value(dev,
				reg->tc_status,
				&status);
	if (ret < 0) {
		return ret;
	}

	if (!(status & FONT_MEM_CRC)) {
		t_watch_err(dev, "font crc fail [tc_status %08Xh]\n", status);
		return 1;
	}

	t_watch_info(dev, "font crc ok\n");
	return 0;
}

static int ext_watch_chk_font_status(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	int ret = 0;

	if (siw_touch_boot_mode_check(dev) >= MINIOS_MFTS_FOLDER) {
		return -EBUSY;
	}

	switch (atomic_read(&watch->state.font_status)) {
	case FONT_EMPTY :
		return -EBUSY;

	case FONT_DOWNLOADING :
		mod_delayed_work(ts->wq, &chip->font_download_work,
			msecs_to_jiffies(20));
		return 0;
	}

	ret = ext_watch_do_chk_font_status(dev);
	if (ret == 1) {
		mod_delayed_work(ts->wq, &chip->font_download_work,
			msecs_to_jiffies(20));
		goto out;
	}

out:
	return ret;
}
#else	/* __SUPPORT_WATCH_FONT_STS_CHK */
static int ext_watch_chk_font_status(struct device *dev)
{
	return 0;
}

#define ext_watch_do_chk_font_status	ext_watch_chk_font_status
#endif	/* __SUPPORT_WATCH_FONT_STS_CHK */

enum {
	FONT_TYPE_0 = 0,
	FONT_TYPE_1,
};

#define FONT_DN_WORK_MSG	"font dn work(%d): "

#define t_watch_info_font_dn_x(_dev, fmt, _idx, args...)	\
		t_watch_info(_dev, FONT_DN_WORK_MSG fmt, _idx, ##args)

#define t_watch_info_font_dn_0(_dev, fmt, args...)	\
		t_watch_info_font_dn_x(_dev, fmt, 0, ##args)
#define t_watch_info_font_dn_1(_dev, fmt, args...)	\
		t_watch_info_font_dn_x(_dev, fmt, 1, ##args)

#define t_watch_err_font_dn_x(_dev, fmt, _idx, args...)	\
		t_watch_err(_dev, FONT_DN_WORK_MSG fmt, _idx, ##args)

#define t_watch_err_font_dn_0(_dev, fmt, args...)	\
		t_watch_err_font_dn_x(_dev, fmt, 0, ##args)
#define t_watch_err_font_dn_1(_dev, fmt, args...)	\
		t_watch_err_font_dn_x(_dev, fmt, 1, ##args)

static int ext_watch_font_dn_type_0(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_hal_reg *reg = chip->reg;
//	struct siw_ts *ts = chip->ts;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	struct ext_watch_cfg *ext_wdata = &watch->ext_wdata;
	struct ext_watch_font_header __font_hdr;
	struct ext_watch_font_header *font_hdr = NULL;
	u8 *font_data;
	int font_size;
	int curr_size;
	u32 offset = 0;
	u32 crc_addr = 0;
	u32 crc_file = 0;
	u32 font_crc_check = 0;
	u32 magic_addr = 0;
	u32 font_magic_check = 0;
	int ret = 0;

	t_watch_info_font_dn_0(dev, "begins\n");

	memcpy((void *)&__font_hdr.width_num, (void *)watch->ext_wdata.font_data,
		sizeof(__font_hdr) - sizeof(__font_hdr.magic_code));

	font_hdr = (struct ext_watch_font_header *)&__font_hdr;

	magic_addr = font_hdr->size;

	t_watch_info_font_dn_0(dev,
		"magic addr %08Xh\n",
		magic_addr);

	if (magic_addr > watch->font_written_size) {
		t_watch_err_font_dn_0(dev, "wrong magic addr, %08Xh\n", magic_addr);
		goto out;
	}

	memcpy((void *)&font_hdr->magic_code,
		(void *)&ext_wdata->font_data[magic_addr], sizeof(u32));

	if (font_hdr->magic_code != watch->font_magic_code) {
		t_watch_err_font_dn_0(dev, "wrong magic code: %08Xh (%08Xh)\n",
			font_hdr->magic_code, watch->font_magic_code);
		goto out;
	}
	ext_wdata->magic_code = watch->font_magic_code;

	t_watch_info_font_dn_0(dev,
		"id %d, size %d\n",
		font_hdr->font_id, font_hdr->size);

	t_watch_info_font_dn_0(dev,
		"width_num %d, width_colon %d, height %d\n",
		 font_hdr->width_num, font_hdr->width_colon, font_hdr->height);

//	crc_addr = font_hdr->size + SIW_FONT_MAGIC_CODE_SIZE;
	crc_addr = font_hdr->size + watch->font_crc_offset;

	memcpy((void *)&crc_file, (void *)&ext_wdata->font_data[crc_addr], sizeof(u32));
	t_watch_info_font_dn_0(dev,
		"input crc %08Xh[addr %08Xh]\n",
		crc_file, crc_addr);

	ext_wdata->font_crc =
		ext_watch_font_crc_cal(ext_wdata->font_data, crc_addr);
	t_watch_info_font_dn_0(dev,
		"result crc %08Xh\n",
		ext_wdata->font_crc);

	memcpy((void *)&ext_wdata->font_data[crc_addr],
		 (void *)&ext_wdata->font_crc, sizeof(u32));

	magic_addr >>= 2;
	crc_addr >>= 2;
	offset = 0;
	font_data = ext_wdata->font_data;
	font_size = watch->font_written_size;
	while (font_size) {
		curr_size = min(font_size, WATCH_MAX_RW_SIZE);

		/* write font */
		ret = siw_hal_write_value(dev,
					reg->ext_watch_font_offset,
			 		offset);
		if (ret < 0) {
			 goto out;
		}

		ret = siw_hal_reg_write(dev,
					reg->ext_watch_font_addr,
			 		(void *)font_data, curr_size);
		if (ret < 0) {
			goto out;
		}

		/* dump font */
		ret = siw_hal_write_value(dev,
					reg->ext_watch_font_offset,
			 		magic_addr);
		if (ret < 0) {
			goto out;
		}

		ret = siw_hal_read_value(dev,
					reg->ext_watch_font_addr,
			 		&font_magic_check);
		if (ret < 0) {
			goto out;
		}

		t_watch_info_font_dn_0(dev,
			"font magic return check : %Xh\n",
			font_magic_check);

		ret = siw_hal_write_value(dev,
					reg->ext_watch_font_offset,
			 		crc_addr);
		if (ret < 0) {
			goto out;
		}

		ret = siw_hal_read_value(dev,
					reg->ext_watch_font_addr,
			 		&font_crc_check);
		if (ret < 0) {
			goto out;
		}

		t_watch_info_font_dn_0(dev,
			"font crc return check : %Xh\n",
			font_crc_check);


		font_size -= curr_size;
		font_data += curr_size;
		offset += (curr_size>>2);
	}

#if defined(__SUPPORT_WATCH_FONT_STS_CHK)
	ret = siw_hal_write_value(dev,
				reg->ext_watch_font_crc, 1);
	if (ret < 0) {
		goto out;
	}
#endif

	atomic_set(&watch->state.font_status, FONT_READY);

	if (chip->lcd_mode == LCD_MODE_U2) {
		ret = ext_watch_mem_ctrl(dev, 0, "U2 mode");
		if (ret < 0) {
			goto out;
		}
	}

	t_watch_info_font_dn_0(dev,
		"done(%d)\n",
		watch->font_written_size);

	return 0;

out:
	atomic_set(&watch->state.font_status, FONT_EMPTY);

	t_watch_err_font_dn_0(dev, "failed, %d\n", ret);

	return ret;
}

static int ext_watch_font_dn_type_1(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_hal_reg *reg = chip->reg;
//	struct siw_ts *ts = chip->ts;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	struct ext_watch_cfg *ext_wdata = &watch->ext_wdata;
	struct ext_watch_font_header *font_hdr = NULL;
	u8 *font_data;
	int font_size;
	int curr_size;
	u32 offset = 0;
	u32 crc_addr = 0;
	u32 crc_file = 0;
	int ret = 0;

	t_watch_info_font_dn_1(dev, "begins\n");

	font_hdr = (struct ext_watch_font_header *)watch->ext_wdata.font_data;

	if (font_hdr->magic_code != watch->font_magic_code) {
		t_watch_err_font_dn_1(dev, "wrong magic code: %08Xh (%08Xh)\n",
			font_hdr->magic_code, watch->font_magic_code);
		goto out;
	}
	ext_wdata->magic_code = watch->font_magic_code;

	t_watch_info_font_dn_1(dev,
		"id %d, size %d\n",
		font_hdr->font_id, font_hdr->size);

	t_watch_info_font_dn_1(dev,
		"width_num %d, width_colon %d, height %d\n",
		font_hdr->width_num, font_hdr->width_colon, font_hdr->height);

//	crc_addr = font_hdr->size + sizeof(struct ext_watch_font_header);
	crc_addr = font_hdr->size + watch->font_crc_offset;

	memcpy((void *)&crc_file, (void *)&ext_wdata->font_data[crc_addr], sizeof(u32));
	t_watch_info_font_dn_1(dev,
		"input crc %08Xh[addr %08Xh]\n",
		crc_file, crc_addr);

	ext_wdata->font_crc =
		ext_watch_font_crc_cal(ext_wdata->font_data, crc_addr);
	t_watch_info_font_dn_1(dev,
		"result crc %08Xh\n",
		ext_wdata->font_crc);

	if (watch->font_written_size == crc_addr) {
		watch->font_written_size += sizeof(ext_wdata->font_crc);
	}

	memcpy((void *)&ext_wdata->font_data[crc_addr],
		 (void *)&ext_wdata->font_crc, sizeof(u32));

	offset = 0;
	font_data = ext_wdata->font_data;
	font_size = watch->font_written_size;
	while (font_size) {
		curr_size = min(font_size, WATCH_MAX_RW_SIZE);

		/* write font */
		ret = siw_hal_write_value(dev,
					reg->ext_watch_font_offset,
			 		offset);
		if (ret < 0) {
			 goto out;
		}

		ret = siw_hal_reg_write(dev,
					reg->ext_watch_font_addr,
			 		(void *)font_data, curr_size);
		if (ret < 0) {
			goto out;
		}

		font_size -= curr_size;
		font_data += curr_size;
		offset += (curr_size>>2);
	}

#if defined(__SUPPORT_WATCH_FONT_STS_CHK)
	ret = siw_hal_write_value(dev,
				reg->ext_watch_font_crc, 1);
	if (ret < 0) {
		goto out;
	}
#endif

	atomic_set(&watch->state.font_status, FONT_READY);

	if (chip->lcd_mode == LCD_MODE_U2) {
		ret = ext_watch_mem_ctrl(dev, 0, "U2 mode");
		if (ret < 0) {
			goto out;
		}
	}

	t_watch_info_font_dn_1(dev,
		"done(%d)\n",
		watch->font_written_size);

	return 0;

out:
	atomic_set(&watch->state.font_status, FONT_EMPTY);

	t_watch_err_font_dn_1(dev, "failed, %d\n", ret);

	return ret;
}

static void ext_watch_font_download(struct work_struct *font_download_work)
{
	struct siw_touch_chip *chip =
			container_of(to_delayed_work(font_download_work),
				 struct siw_touch_chip, font_download_work);
	struct siw_ts *ts = chip->ts;
	struct device *dev = chip->dev;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	int font_type = FONT_TYPE(watch);
//	int watch_type = WATCH_TYPE(watch);
//	int watch_type_var = WATCH_TYPE_VAR(watch);
	int ret = 0;

	if (atomic_read(&watch->state.font_status) == FONT_EMPTY) {
		t_watch_err(dev, "font dn work: data not downloaded\n");
		return;
	}

	if (atomic_read(&ts->state.sleep) == IC_DEEP_SLEEP) {
		atomic_set(&watch->state.font_status, FONT_DOWNLOADING);

		t_watch_info(dev, "font dn work: IC deep sleep\n");
		return;
	}

	mutex_lock(&ts->lock);
	mutex_lock(&ts->reset_lock);

	if (chip->lcd_mode == LCD_MODE_U2) {
		watch->ext_wdata.time.disp_waton = 0;
		ret = ext_watch_display_onoff(dev);
		if (ret < 0) {
			goto out;
		}

		ret = ext_watch_mem_ctrl(dev, 1, "U2 mode");
		if (ret < 0) {
			goto out;
		}
	}

	switch (font_type) {
	case 1:
		ext_watch_font_dn_type_1(dev);
		break;
	default:
		ext_watch_font_dn_type_0(dev);
		break;
	}

	touch_msleep(5);

	ext_watch_do_chk_font_status(dev);

out:
	mutex_unlock(&ts->reset_lock);
	mutex_unlock(&ts->lock);
}

static int ext_watch_font_dump(struct device *dev, u8 *buf)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct siw_hal_reg *reg = chip->reg;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	u8 *font_data;
	int font_size;
	int curr_size;
	u32 offset = 0;
	int ret = 0;

	mutex_lock(&ts->lock);

	t_watch_info(dev, "font dump: begins\n");

	if (chip->lcd_mode == LCD_MODE_U2) {
		ret = ext_watch_mem_ctrl(dev, 1, "font dump: U2 mode");
		if (ret < 0) {
			goto out;
		}
	}

	font_data = buf;
	font_size = watch->font_written_size;
	while (font_size) {
		curr_size = min(font_size, WATCH_MAX_RW_SIZE);

		ret = siw_hal_write_value(dev,
					reg->ext_watch_font_offset,
					offset);
		if (ret < 0) {
			 goto out;
		}

		ret = siw_hal_reg_write(dev,
					reg->ext_watch_font_addr,
			 		(void *)font_data, curr_size);
		if (ret < 0) {
			goto out;
		}

		font_size -= curr_size;
		font_data += curr_size;
		offset += (curr_size>>2);
	}

	if (chip->lcd_mode == LCD_MODE_U2) {
		ret = ext_watch_mem_ctrl(dev, 0, "font dump: U2 mode");
		if (ret < 0) {
			goto out;
		}
	}

	t_watch_info(dev, "font dump: done(%d)\n",
			watch->font_written_size);

	mutex_unlock(&ts->lock);

	return 0;

out:
	t_watch_err(dev, "font dump: failed, %d\n", ret);

	mutex_unlock(&ts->lock);

	return ret;
}

static int ext_watch_fontdata_check(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct watch_data *watch = chip->watch;
	struct siw_touch_font_bin *font_bin = NULL;
	u8 *font_buf = NULL;
	u32 font_size = 0;

	watch->font_num = 0;
	watch->font_idx = 0;

	if (!(touch_flags(ts) & TOUCH_USE_FONT_BINARY)) {
		return 0;
	}

	t_watch_info(dev, "font_check: binary\n");

	font_bin = touch_font_bin(ts);
	if (font_bin == NULL) {
		t_watch_err(dev, "font_check: empty font info\n");
		return -ENOMEM;
	}

	while (1) {
		font_buf = font_bin->font_data;
		font_size = font_bin->font_size;

		if ((font_buf == NULL) || !font_size) {
			break;
		}

		watch->font_num++;

		font_bin++;
	}

	t_watch_info(dev,
		"font_check: %d binary detected\n",
		watch->font_num);

	return 0;
}

static int ext_watch_fontdata_read(struct device *dev,
			const char *name, u8 *buf, u32 max_size)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct watch_data *watch = chip->watch;
	struct siw_touch_font_bin *font_bin = NULL;
	struct file *filp = NULL;
	u8 *font_buf = NULL;
	u32 font_size = 0;
	int font_idx = 0;
	loff_t fsize;
	int rd_size;
	int ret = 0;

	if (touch_flags(ts) & TOUCH_USE_FONT_BINARY) {
		t_watch_info(dev, "font_preload: font info - binary\n");

		if (!watch->font_num) {
			t_watch_err(dev, "font_preload: no valid font info\n");
			return -EFAULT;
		}

		font_idx = watch->font_idx;
		if (font_idx >= watch->font_num) {
			t_watch_err(dev, "font_preload: invalid font index: %d > %d\n",
				font_idx, watch->font_num - 1);
			return -EFAULT;
		}

		font_bin = touch_font_bin(ts);
		if (font_bin == NULL) {
			t_watch_err(dev, "font_preload: empty font info\n");
			return -ENOMEM;
		}

		font_buf = font_bin[font_idx].font_data;
		font_size = font_bin[font_idx].font_size;

		if ((font_buf == NULL) || !font_size) {
			t_watch_err(dev, "font_preload: invalid font info\n");
			return -ENOMEM;
		}

		if (font_size > max_size) {
			t_watch_err(dev, "font_preload: size overflow, %d > %d\n",
				font_size, max_size);
			return -EOVERFLOW;
		}

		memcpy(buf, font_buf, font_size);
		return font_size;
	}

	t_watch_info(dev, "font_preload: font info - file\n");

	if (!name) {
		t_watch_info(dev, "font_preload: watch_font_image not defined\n");
		return -EFAULT;
	}

	filp = filp_open(name, O_RDONLY, 0);
	if (IS_ERR(filp)) {
		t_watch_err(dev, "font_preload: font_image open failed\n");
		return (int)PTR_ERR(filp);
	}

	fsize = vfs_llseek(filp, 0, SEEK_END);
	if ((u32)fsize > max_size) {
		t_watch_err(dev, "font_preload: size overflow, %d > %d\n",
			(int)fsize, max_size);
		ret = -EOVERFLOW;
		goto out;
	}

	rd_size = kernel_read(filp, 0, (char *)buf, (unsigned long)fsize);
	if (rd_size != (int)fsize) {
		t_watch_err(dev, "font_preload: failed to read[%d], %d\n",
			(int)fsize, rd_size);
		memset(buf, 0, max_size);
		ret = (rd_size < 0) ? rd_size : -EFAULT;
		goto out;
	}

	ret = rd_size;

out:
	filp_close(filp, 0);
	return ret;
}

static int ext_watch_fontdata_preload(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct watch_data *watch = chip->watch;
	u8 *buf;
	int size = 0;

	if (!watch->ext_wdata.font_data) {
		t_watch_info(dev, "font_preload: no font data buffer! check!\n");
		return 0;
	}
	buf = watch->ext_wdata.font_data;

	size = ext_watch_fontdata_read(dev,
				ts->watch_font_image,
				buf,
				watch->font_max_size);
	if (size < 0) {
		return size;
	}

	watch->font_written_size = (u32)size;

	atomic_set(&watch->state.font_status, FONT_DOWNLOADING);

	if (touch_flags(ts) & TOUCH_USE_FONT_BINARY) {
		t_watch_info(dev, "font image preload done (%d)\n",
			size);
	} else {
		t_watch_info(dev, "font image preload done (%d, %s)\n",
			size, ts->watch_font_image);
	}
	t_watch_info(dev,
			" %02X %02X %02X %02X"
			" %02X %02X %02X %02X"
			" ...\n",
			buf[0], buf[1], buf[2], buf[3],
			buf[4], buf[5], buf[6], buf[7]);

	return 0;
}


#if defined(__SUPPORT_WATCH_CTRL_ACCESS)
static ssize_t store_ext_watch_rtc_onoff(struct device *dev,
		 			const char *buf, size_t count)
{
	struct siw_ts *ts = to_touch_core(dev);
	u8 buf_val, value = 0;
	u8 zero = '0';
	int ret = 0;

	buf_val = buf[0];
	value = !(!buf_val || (buf_val == zero));

	mutex_lock(&ts->lock);
	ret = ext_watch_rtc_start(dev, value);
	mutex_unlock(&ts->lock);

	return count;
}

static ssize_t show_ext_watch_block_cfg(struct device *dev, char *buf)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	int ret = 0;

	ret = snprintf(buf, PAGE_SIZE, "block_watch_cfg : %d\n",
				atomic_read(&chip->block_watch_cfg));

	return ret;
}

static ssize_t store_ext_watch_block_cfg(struct device *dev,
		 const char *buf, size_t count)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	u8 buf_val, buf_val_org, value;

	buf_val = buf[0];
	buf_val_org = buf_val;
	if (buf_val >= '0')
		buf_val -= '0';
	if (buf_val > BLOCKED) {
		t_watch_err(dev, "invalid param, %d\n", buf_val_org);
		goto out;
	};

	value = buf_val;

	atomic_set(&chip->block_watch_cfg, value);

	t_watch_info(dev, "%s\n", (value) ? "BLOCKED" : "UNBLOCKED");

out:
	return count;
}

static ssize_t store_ext_watch_font_onoff(struct device *dev,
					const char *buf, size_t count)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	u8 buf_val, value;
	int ret = 0;

	if (atomic_read(&chip->block_watch_cfg) == BLOCKED) {
		t_watch_err(dev, "store font onoff blocked\n");
		return __ret_val_blocked(count);
	}

	mutex_lock(&ts->lock);

	buf_val = buf[0];
	if ((count == 2 ) && (buf_val == EXT_WATCH_CFG_DEBUG)) {
		buf_val = 1;
	} else if ((buf_val >= '0') && (buf_val <= '9')) {
		buf_val -= '0';
	}

	value = !!buf_val;

	ret = ext_watch_set_cfg(dev, 1);
	if (ret < 0) {
		t_watch_err(dev, "failed to set watch cfg, %d\n", ret);
		goto out;
	}

	watch->ext_wdata.time.disp_waton = value;

	ret = ext_watch_display_onoff(dev);
	if (ret < 0) {
		t_watch_err(dev,
			"failed to set watch display control, %d\n", ret);
		goto out;
	}

	t_watch_info(dev,
		"Power %s(%d)\n", (value) ? "ON" : "OFF", value);

	ret = count;

out:
	mutex_unlock(&ts->lock);
	return ret;
}

static ssize_t store_ext_watch_config_font_effect(struct device *dev,
		 			const char *buf, size_t count)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
//	struct siw_ts *ts = chip->ts;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	struct ext_watch_cfg_mode *mode = &watch->ext_wdata.mode;
	struct ext_watch_cfg_time *time = &watch->ext_wdata.time;
	struct ext_watch_cfg_position *position = &watch->ext_wdata.position;
	struct ext_watch_config_font_effect cfg = { 0, };
	char period[16];
	int blink_type, blink_max, blink_boundary, blink_unit;
	int watch_type = WATCH_TYPE(watch);
	int watch_type_var = WATCH_TYPE_VAR(watch);

	if (atomic_read(&chip->block_watch_cfg) == BLOCKED) {
		t_watch_err(dev, "store font effect blocked\n");
		return __ret_val_blocked(count);
	}

	//for test using echo command
	if ((count == 2) && (buf[0] == EXT_WATCH_CFG_DEBUG)) {
		memset((char *)&cfg, 0, sizeof(cfg));

		cfg.midnight_hour_zero_en = 0;
		cfg.blink.blink_type = 3;

		switch (watch_type) {
		case WATCH_TYPE_1:
			cfg.blink.bstartx = 696;
			cfg.blink.bendx = 744;
			break;
		default:
			/*
			 * blink position is handled by MIPI.
			 */
			switch (watch_type_var) {
			case 1:	//SW49407
				cfg.blink.bstartx = 1092 + 132;
				cfg.blink.bendx = 1092 + 156;
				break;
			}
			break;
		}
	} else {
		memcpy((char *)&cfg, buf, sizeof(cfg));
	}

	blink_type = cfg.blink.blink_type;

	/*
	 * 4:1s, 5:2s, 6:4s, 7:8s
	 * 1:125ms, 2:250ms, 3:500ms
	 */
	blink_max = 7;
	blink_boundary = 4;
	blink_unit = 125;

	if (blink_type > blink_max)
		blink_type = blink_max;

	if (blink_type >= blink_boundary) {
		snprintf(period, sizeof(period), "%d s",
			1<<(blink_type - blink_boundary));
	} else if (blink_type) {
		snprintf(period, sizeof(period), "%d ms",
			(1<<(blink_type - 1))*blink_unit);
	} else {
		snprintf(period, sizeof(period), "off");
	}

	position->h24_en = cfg.h24_en;
	position->zero_disp = cfg.zero_disp;
	position->clock_disp_mode = cfg.clock_disp_type;
	position->midnight_hour_zero_en = cfg.midnight_hour_zero_en;
	position->bhprd = blink_type;

	mode->blink_area.bstartx = cfg.blink.bstartx;
	mode->blink_area.bendx = cfg.blink.bendx;

	t_watch_info(dev,
		"24h mode %s, zero dispaly %s, %s type, %s mode\n",
		(cfg.h24_en) ? "on" : "off",
		(cfg.zero_disp) ? "on" : "off",
		(cfg.clock_disp_type) ? "MM:SS" : "HH:MM",
		(cfg.midnight_hour_zero_en) ? "00:00" : "12:00");

	switch (watch_type) {
	case WATCH_TYPE_1:
		t_watch_info(dev,
			"blink area [%d , %d] period %s\n",
			cfg.blink.bstartx, cfg.blink.bendx, period);
		break;
	case WATCH_TYPE_0:
		time->disp_waton = cfg.watchon;
		t_watch_info(dev,
			"blink area [%d , %d] period %s, watch display %s\n",
			cfg.blink.bstartx, cfg.blink.bendx, period,
			(time->disp_waton) ? "on" : "off");
		break;
	}

	return count;
}

static int __ext_watch_chk_font_pos(struct device *dev,
				struct ext_watch_config_font_pos *cfg,
				struct reset_area *watch_win)
{
#if 1
	if (watch_win) {
		if ((cfg->watstartx < watch_win->x1) || (cfg->watendx > watch_win->x2) ||
			(cfg->watstarty < watch_win->y1) || (cfg->watendy > watch_win->y2)) {
			t_watch_err(dev, "check pos: invalid win: %d %d %d %d - %d %d %d %d\n",
				watch_win->x1, watch_win->x2, watch_win->y1, watch_win->y2,
				cfg->watstartx, cfg->watendx, cfg->watstarty, cfg->watendy);
			return -EINVAL;
		}
	}
#endif

	return 0;
}

static const struct reset_area watch_win_test = {
	.x1 = 420,
	.y1 = 0,
	.x2 = 1020,
	.y2 = 680,
};

static const struct ext_watch_config_font_pos watch_pos_test_1 = {
	.watstartx	= 464,
	.watendx	= 976,
	.watstarty	= 244,
	.watendy	= 428,
	.h1x_pos	= 114,
	.h10x_pos	= 0,
	.m1x_pos	= 398,
	.m10x_pos	= 284,
	.clx_pos	= 232,
};

static const struct ext_watch_config_font_pos watch_pos_test_0 = {
	.watstartx	= 522,
	.watendx	= 630,
	.watstarty	= 9,
	.watendy	= 59,
	.h1x_pos	= 16,
	.h10x_pos	= 0,
	.m1x_pos	= 64,
	.m10x_pos	= 48,
	.clx_pos	= (32+4),
};

static ssize_t store_ext_watch_config_font_position(struct device *dev,
					const char *buf, size_t count)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	struct ext_watch_cfg_mode *mode = &watch->ext_wdata.mode;
	struct ext_watch_cfg_position *position = &watch->ext_wdata.position;
	struct ext_watch_config_font_pos cfg = {0, };
	struct reset_area *watch_win = NULL;
	int watch_type = WATCH_TYPE(watch);
	int watch_type_var = WATCH_TYPE_VAR(watch);
	int ret = 0;

	if (atomic_read(&chip->block_watch_cfg) == BLOCKED) {
		t_watch_err(dev, "store font position blocked\n");
		return __ret_val_blocked(count);
	}

	if (watch->win_fixed != NULL) {
		watch_win = (struct reset_area *)watch->win_fixed;
	} else {
		watch_win = pdata_watch_win(ts->pdata);
		if (!watch_win) {
			watch_win = (struct reset_area *)&watch_win_test;
		}
	}

	//for test using echo command
	if ((count == 2) && (buf[0] == EXT_WATCH_CFG_DEBUG)) {
		void *pos_dbg = NULL;

		switch (watch_type) {
		case WATCH_TYPE_1:
			pos_dbg = (void *)&watch_pos_test_1;
			break;
		default:
			pos_dbg = (void *)&watch_pos_test_0;

			switch (watch_type_var) {
			case 1:		//SW49407
				cfg.watstartx = 1092;	// <= 1152
				cfg.watendx = 1092 + 288;
				cfg.watstarty = 0;
				cfg.watendy = 160;
				cfg.h10x_pos = 0;
				cfg.h1x_pos = 66;
				cfg.clx_pos = 132;
				cfg.m10x_pos = 156;
				cfg.m1x_pos = 222;
				pos_dbg = &cfg;
				break;
			}
			break;
		}
		if (pos_dbg) {
			memcpy((void *)&cfg, pos_dbg, sizeof(cfg));
		} else {
			memset((char *)&cfg, 0, sizeof(cfg));
		}
	} else {
		memcpy((char *)&cfg, buf, sizeof(cfg));
	}

	ret = __ext_watch_chk_font_pos(dev, &cfg, watch_win);
	if (ret < 0) {
		goto out;
	}

	mode->watch_area_x.watstart = cfg.watstartx;
	mode->watch_area_x.watend = cfg.watendx;
	mode->watch_area_y.watstart = cfg.watstarty;
	mode->watch_area_y.watend = cfg.watendy;

	position->h10x_pos = cfg.h10x_pos;
	position->h1x_pos = cfg.h1x_pos;
	position->m10x_pos = cfg.m10x_pos;
	position->m1x_pos = cfg.m1x_pos;
	position->clx_pos = cfg.clx_pos;

	t_watch_info(dev,
			"watch area: sx %d, ex %d, sy %d, ey %d\n",
			mode->watch_area_x.watstart,
			mode->watch_area_x.watend,
			mode->watch_area_y.watstart,
			mode->watch_area_y.watend);
	t_watch_info(dev,
			"watch position: h10x %d, h1x %d, m10x %d, m1x %d, c1x %d\n",
			position->h10x_pos,
			position->h1x_pos,
			position->m10x_pos,
			position->m1x_pos,
			position->clx_pos);

	if (ret < 0)
		goto out;

	/* Transfer pos data via notifier */
	siw_touch_blocking_notifier_call(LCD_EVENT_TOUCH_WATCH_POS_UPDATE, (void*)(&cfg));

	ret = count;

out:
	return ret;
}

static const struct ext_watch_config_font_prop watch_prop_test_lg4946 = {
	.max_num = EXT_WATCH_LUT_NUM,
	.lut = {
		[0] = { 0x01, 0x01, 0x01 },
		[1] = { 0x36, 0x36, 0x36 },
		[2] = { 0x4F, 0x4F, 0x4F },
		[3] = { 0x73, 0x73, 0x73 },
		[4] = { 0x8E, 0x8E, 0x8E },
		[5] = { 0xA2, 0xA2, 0xA2 },
		[6] = { 0xC0, 0xC0, 0xC0 },
	},
};

static const struct ext_watch_config_font_prop watch_prop_test = {
	.max_num = EXT_WATCH_LUT_NUM,
	.lut = {
		[0] = { 0xFF, 0x00, 0x00 },	/* blue */
		[1] = { 0x00, 0xFF, 0x00 },	/* green */
		[2] = { 0xFF, 0xFF, 0x00 },	/* cyan */
		[3] = { 0x00, 0x00, 0xFF },	/* red */
		[4] = { 0xFF, 0x00, 0xFF },	/* magenda */
		[5] = { 0x00, 0xFF, 0xFF },	/* yellow */
		[6] = { 0xFF, 0xFF, 0xFF },	/* white */
	},
};

static ssize_t store_ext_watch_config_font_property(struct device *dev,
 					const char *buf, size_t count)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	struct ext_watch_cfg_mode *mode = &watch->ext_wdata.mode;
	struct ext_watch_bits_lut *lut = NULL;
	struct ext_watch_config_font_prop cfg = {0, };
	struct ext_watch_config_font_lut *lut_src = NULL;
	void *prop_src = NULL;
	char log[FONT_TEMP_LOG_SZ] = {0, };
	int loglen = 0;
	int idx = 0;
//	int ret = 0;

	if (atomic_read(&chip->block_watch_cfg) == BLOCKED) {
		t_watch_err(dev, "store font property blocked\n");
		return __ret_val_blocked(count);
	}

	//for test using echo command
	if ((count == 2) && (buf[0] == EXT_WATCH_CFG_DEBUG)) {
		switch (touch_chip_type(ts)) {
		case CHIP_LG4946:
			prop_src = (void *)&watch_prop_test_lg4946;
			break;
		default:
			prop_src = (void *)&watch_prop_test;
			break;
		}
	} else {
		prop_src = (void *)buf;
	}
	memcpy((char *)&cfg, prop_src, sizeof(cfg));

	if (WATCH_FLAG(watch) & WATCH_FLAG_SKIP_LUT_UPDATE) {
		/* SKIP : lut setup is handled by MIPI, no effect */
		t_watch_info(dev, "skip font property\n");
		goto out;
	}

	if (!cfg.max_num || (cfg.max_num > EXT_WATCH_LUT_NUM)) {
		t_watch_err(dev, "invalid lut count, %d\n", cfg.max_num);
		goto out;
	}

	loglen += siw_watch_snprintf(log, FONT_TEMP_LOG_SZ, loglen,
				"lut[%d] ", cfg.max_num);

	mode->lut = mode->__lut;
	lut = mode->lut;
	lut_src = cfg.lut;
	for (idx = 0; idx < (int)cfg.max_num; idx++) {
		lut->b = lut_src->rgb_blue;
		lut->g = lut_src->rgb_green;
		lut->r = lut_src->rgb_red;

		loglen += siw_watch_snprintf(log, FONT_TEMP_LOG_SZ, loglen,
					"%d:%02X%02X%02X ",
					idx + 1,
					lut_src->rgb_blue,
					lut_src->rgb_green,
					lut_src->rgb_red);

		lut++;
		lut_src++;
	}

	t_watch_info(dev, "%s\n", log);

out:
	/* Transfer LUT data via notifier */
	siw_touch_blocking_notifier_call(LCD_EVENT_TOUCH_WATCH_LUT_UPDATE, (void*)(&cfg));

	return count;
}


static ssize_t store_ext_watch_config_time_sync(struct device *dev,
					const char *buf, size_t count)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	struct ext_watch_cfg_time *time = &watch->ext_wdata.time;
	struct ext_watch_config_time_sync cfg = {0, };
	int ret = 0;

	if (atomic_read(&chip->block_watch_cfg) == BLOCKED) {
		t_watch_err(dev, "store time sync blocked\n");
		return __ret_val_blocked(count);
	}

	mutex_lock(&ts->lock);

	t_watch_info(dev, "time sync\n");

	//for test using echo command
	if ((count == 2) && (buf[0] == EXT_WATCH_CFG_DEBUG)) {
		struct timespec my_time;
		struct tm my_date;

		my_time = current_kernel_time();
		time_to_tm(my_time.tv_sec,
				sys_tz.tz_minuteswest * 60 * (-1),
				&my_date);

		t_watch_info(dev, "get system time\n");

		memset((char *)&cfg, 0, sizeof(cfg));

		cfg.rtc_cwhour = my_date.tm_hour;
		cfg.rtc_cwmin = my_date.tm_min;
		cfg.rtc_cwsec = my_date.tm_sec;
		cfg.rtc_cwmilli = my_time.tv_nsec/1000000;
	} else {
		memcpy((char *)&cfg, buf, sizeof(cfg));
	}

	time->rtc_sct.hour = cfg.rtc_cwhour;
	time->rtc_sct.min = cfg.rtc_cwmin;
	time->rtc_sct.sec = cfg.rtc_cwsec;
	time->rtc_sctcnt = cfg.rtc_cwmilli;

	ret = ext_watch_set_curr_time(dev);
	if (ret < 0) {
		goto out;
	}

	ret = ext_watch_get_curr_time(dev, NULL, NULL);
	if (ret < 0) {
		goto out;
	}

	ret = ext_watch_display_onoff(dev);
	if (ret < 0) {
		goto out;
	}

	ret = count;

out:
	mutex_unlock(&ts->lock);
	return ret;
}

static ssize_t show_ext_watch_query_font_data(struct device *dev, char *buf)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
//	struct siw_ts *ts = chip->ts;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	struct ext_watch_query_font_data query;
	ssize_t size = sizeof(struct ext_watch_query_font_data);
	int watch_type = WATCH_TYPE(watch);

	query.Font_supported = SUPPORT;

	/*
	 * Not fixed, depends on F/W spec.
	 */
	switch (watch_type) {
	case WATCH_TYPE_1:
		query.max_font_x_size = 255;
		query.max_font_y_size = 184;
		query.max_cln_x_size = 255;
		query.max_cln_y_size = 48;
		break;
	default:
		query.max_font_x_size = 160;
		query.max_font_y_size = 64;
		query.max_cln_x_size = 160;
		query.max_cln_y_size = 24;
		break;
	};

	memcpy(buf, (void *)&query, size);

	return size;
}

static ssize_t show_ext_watch_query_font_position(struct device *dev, char *buf)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
//	struct siw_ts *ts = chip->ts;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	struct ext_watch_cfg_mode *mode = &watch->ext_wdata.mode;
	struct ext_watch_cfg_position *position = &watch->ext_wdata.position;
	struct ext_watch_query_font_position query;
	ssize_t size = sizeof(struct ext_watch_query_font_position);
	int watch_type = WATCH_TYPE(watch);

	switch (watch_type) {
	case WATCH_TYPE_1:
		query.vertical_position_supported = SUPPORT;
		break;
	default:
		query.vertical_position_supported = NOT_SUPPORT;
		break;
	}
	query.horizontal_position_supported = SUPPORT;

	memcpy(buf, (void *)&query, size);

	t_watch_info(dev,
			"watch area: sx %d, ex %d, sy %d, ey %d\n",
			mode->watch_area_x.watstart,
			mode->watch_area_x.watend,
			mode->watch_area_y.watstart,
			mode->watch_area_y.watend);

	t_watch_info(dev,
			"blink area: sx %d, ex %d\n",
			mode->blink_area.bstartx,
			mode->blink_area.bendx);

	t_watch_info(dev,
			"watch pos: h10x %d, h1x %d, m10x %d, m1x %d, c1x %d\n",
			position->h10x_pos,
			position->h1x_pos,
			position->m10x_pos,
			position->m1x_pos,
			position->clx_pos);

	return size;
}

static ssize_t show_ext_watch_query_font_time(struct device *dev, char *buf)
{
	struct ext_watch_query_font_time query;
	ssize_t size = sizeof(struct ext_watch_query_font_time);

	query.h24_supported = SUPPORT;
	query.ampm_supported = NOT_SUPPORT;

	t_watch_info(dev, "query font time: %2d %2d\n",
		query.h24_supported, query.ampm_supported);

	memcpy(buf, (void *)&query, size);

	return size;
}

static ssize_t show_ext_watch_query_font_color(struct device *dev, char *buf)
{
	struct ext_watch_query_font_color query;
	ssize_t size = sizeof(struct ext_watch_query_font_color);

	query.max_num = EXT_WATCH_LUT_NUM;
	query.lut_supported = SUPPORT;
	query.alpha_supported = SUPPORT;
	query.gradation_supported = SUPPORT;

	memcpy(buf, (void *)&query, size);

	return size;
}

static ssize_t show_ext_watch_query_font_effect(struct device *dev, char *buf)
{
	struct ext_watch_query_font_effect query;
	ssize_t size = sizeof(struct ext_watch_query_font_effect);

	query.zero_supported = SUPPORT;
	query.blink_type = 2;

	memcpy(buf, (void *)&query, size);

	return size;
}

static ssize_t show_ext_watch_curr_time(struct device *dev, char *buf)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	int len = 0;

	if (chip == NULL)
		return 0;

	ext_watch_get_curr_time(dev, buf, &len);

	return len;
}


static ssize_t show_ext_watch_setting(struct device *dev, char *buf)
{
	int len = 0;

	t_watch_dbg(dev, "show watch setting\n");

	ext_watch_get_mode(dev, buf, &len);
	ext_watch_get_position(dev, buf, &len);
	ext_watch_get_curr_time(dev, buf, &len);
	ext_watch_get_dic_st(dev, buf, &len);

	return len;
}
#endif	/* __SUPPORT_WATCH_CTRL_ACCESS */

static ssize_t store_ext_watch_ctrl(struct device *dev,
				const char *buf, size_t count)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	char command[6] = {0};
	u32 value = 0;
	int ret = 0;

	if (sscanf(buf, "%5s %X", command, &value) <= 0) {
		return count;
	}

	if (!strcmp(command, "p")) {
		watch->font_idx = value;
		ret = ext_watch_fontdata_preload(dev);
		if (ret >= 0) {
			mod_delayed_work(ts->wq, &chip->font_download_work,
				msecs_to_jiffies(20));
		}
	} else if (!strcmp(command, "d")) {
		mod_delayed_work(ts->wq, &chip->font_download_work,
			msecs_to_jiffies(20));
	}

	return count;
}


#define SIW_WATCH_ATTR(_name, _show, _store)	\
		TOUCH_ATTR(_name, _show, _store)

#define _SIW_WATCH_ATTR_T(_name)	\
		touch_attr_##_name

static SIW_WATCH_ATTR(control, NULL,
					store_ext_watch_ctrl);

#if defined(__SUPPORT_WATCH_CTRL_ACCESS)
static SIW_WATCH_ATTR(rtc_onoff, NULL, store_ext_watch_rtc_onoff);
static SIW_WATCH_ATTR(block_cfg, show_ext_watch_block_cfg,
					store_ext_watch_block_cfg);
static SIW_WATCH_ATTR(config_fontonoff, NULL,
					store_ext_watch_font_onoff);
static SIW_WATCH_ATTR(config_fonteffect, NULL,
					store_ext_watch_config_font_effect);
static SIW_WATCH_ATTR(config_fontposition, NULL,
					store_ext_watch_config_font_position);
static SIW_WATCH_ATTR(config_fontproperty, NULL,
					store_ext_watch_config_font_property);
static SIW_WATCH_ATTR(config_timesync, NULL,
					store_ext_watch_config_time_sync);
static SIW_WATCH_ATTR(current_time, show_ext_watch_curr_time, NULL);
static SIW_WATCH_ATTR(query_fontdata,
					show_ext_watch_query_font_data, NULL);
static SIW_WATCH_ATTR(query_fontposition,
					show_ext_watch_query_font_position, NULL);
static SIW_WATCH_ATTR(query_timesync,
					show_ext_watch_query_font_time, NULL);
static SIW_WATCH_ATTR(query_fontcolor,
					show_ext_watch_query_font_color, NULL);
static SIW_WATCH_ATTR(query_fonteffect,
					show_ext_watch_query_font_effect, NULL);
static SIW_WATCH_ATTR(get_cfg, show_ext_watch_setting, NULL);

static struct attribute *ext_watch_attribute_list[] = {
	&_SIW_WATCH_ATTR_T(control).attr,
	&_SIW_WATCH_ATTR_T(rtc_onoff).attr,
	&_SIW_WATCH_ATTR_T(block_cfg).attr,
	&_SIW_WATCH_ATTR_T(config_fontonoff).attr,
	&_SIW_WATCH_ATTR_T(config_fonteffect).attr,
	&_SIW_WATCH_ATTR_T(config_fontproperty).attr,
	&_SIW_WATCH_ATTR_T(config_fontposition).attr,
	&_SIW_WATCH_ATTR_T(config_timesync).attr,
	&_SIW_WATCH_ATTR_T(current_time).attr,
	&_SIW_WATCH_ATTR_T(query_fontdata).attr,
	&_SIW_WATCH_ATTR_T(query_fontposition).attr,
	&_SIW_WATCH_ATTR_T(query_timesync).attr,
	&_SIW_WATCH_ATTR_T(query_fontcolor).attr,
	&_SIW_WATCH_ATTR_T(query_fonteffect).attr,
	&_SIW_WATCH_ATTR_T(get_cfg).attr,
	NULL,
};
#else	/* __SUPPORT_WATCH_CTRL_ACCESS */

static struct attribute *ext_watch_attribute_list[] = {
	 &_SIW_WATCH_ATTR_T(control).attr,
	 NULL,
};
#endif	/* __SUPPORT_WATCH_CTRL_ACCESS */

static const struct attribute_group ext_watch_attribute_group = {
	 .attrs = ext_watch_attribute_list,
};

static ssize_t ext_watch_attr_show(struct kobject *kobj,
		struct attribute *attr, char *buf)
{
	struct siw_touch_chip *chip =
		container_of(kobj, struct siw_touch_chip, kobj);
	struct siw_touch_attribute *priv =
		container_of(attr, struct siw_touch_attribute, attr);
//	struct siw_ts *ts = chip->ts;
	ssize_t ret = 0;

	if (priv->show)
		ret = priv->show(chip->dev, buf);

	return ret;
}

static ssize_t ext_watch_attr_store(struct kobject *kobj,
		struct attribute *attr, const char *buf, size_t count)
{
	struct siw_touch_chip *chip =
		container_of(kobj, struct siw_touch_chip, kobj);
	struct siw_touch_attribute *priv =
		container_of(attr, struct siw_touch_attribute, attr);
//	struct siw_ts *ts = chip->ts;
	ssize_t ret = 0;

	if (priv->store)
		ret = priv->store(chip->dev, buf, count);

	return ret;
}

static const struct sysfs_ops ext_watch_sysfs_ops = {
	.show	= ext_watch_attr_show,
	.store	= ext_watch_attr_store,
};

static struct kobj_type ext_watch_kobj_type = {
	.sysfs_ops = &ext_watch_sysfs_ops,
};

static ssize_t ext_watch_access_read(struct file *filp, struct kobject *kobj,
					struct bin_attribute *bin_attr,
					char *buf, loff_t off, size_t count)
{
	struct siw_touch_chip *chip =
		container_of(kobj, struct siw_touch_chip, kobj);
	struct siw_ts *ts = chip->ts;
	struct device *dev = chip->dev;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	int ret = -EFAULT;

	if (off == 0) {
		if (atomic_read(&ts->state.sleep) == IC_DEEP_SLEEP) {
			t_watch_info(dev, "IC deep sleep. cannot read\n");
		} else {
			while (1) {
				if (atomic_read(&watch->state.font_status) != FONT_DOWNLOADING) {
					break;
				}

				t_watch_info(dev, "font downloading\n");
				touch_msleep(10);
			}
			ext_watch_font_dump(dev, watch->ext_wdata.font_data);
		}
	}

	t_watch_info(dev, "font(r): %Xh, %Xh\n", (int)off, (int)count);

	if ((off + count) > watch->font_max_size) {
		t_watch_err(dev, "access_read: size overflow : offset[%d] size[%d]\n",
			 (int)off, (int)count);
		ret = -EOVERFLOW;
	} else {
		memcpy(buf, &watch->ext_wdata.font_data[off], count);
		ret = count;
	}

	return (ssize_t)ret;
}

static ssize_t ext_watch_access_write(struct file *filp, struct kobject *kobj,
					struct bin_attribute *bin_attr,
					char *buf, loff_t off, size_t count)
{
	struct siw_touch_chip *chip =
		container_of(kobj, struct siw_touch_chip, kobj);
	struct siw_ts *ts = chip->ts;
	struct device *dev = chip->dev;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	int ret = -EFAULT;

	if (atomic_read(&chip->block_watch_cfg) == BLOCKED) {
		t_watch_warn(dev, "access_write: blocked\n");
		return count;
	}

	if (buf == NULL) {
		t_watch_err(dev, "access_write: NULL buf\n");
		goto out;
	}

	if (watch->ext_wdata.font_data == NULL) {
		t_watch_err(dev, "access_write: NULL font_data\n");
		goto out;
	}

	if ((off + count) > watch->font_max_size)	 {
		t_watch_err(dev, "access_write: size overflow : offset[%d] size[%d]\n",
			(int)off, (int)count);
		atomic_set(&watch->state.font_status, FONT_EMPTY);
		ret = -EOVERFLOW;
		goto out;
	}

	t_watch_info(dev, "font(w): %Xh, %Xh\n", (int)off, (int)count);

	watch->font_written_size = off + count;

	memcpy(&watch->ext_wdata.font_data[off], buf, count);

	atomic_set(&watch->state.font_status, FONT_DOWNLOADING);

	mod_delayed_work(ts->wq, &chip->font_download_work,
		msecs_to_jiffies(20));

	ret = count;

 out:
	 return ret;
}

static int ext_watch_fontdata_attr_init(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
//	struct siw_ts *ts = chip->ts;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	struct bin_attribute *fontdata_attr = &watch->fontdata_attr;
	u8 *buf;
	int size = watch->font_max_size;
	int ret = 0;

	watch->font_written_size = 0;
	buf = touch_kzalloc(dev, size, GFP_KERNEL);
	if (!buf) {
		t_watch_err(dev, "failed to allocate font buffer[%Xh]\n", size);
		ret = -ENOMEM;
		goto out;
	}
	watch->ext_wdata.font_data = buf;

	t_dev_dbg_base(dev, "font buffer[%Xh] allocated\n", size);

	sysfs_bin_attr_init(fontdata_attr);
	fontdata_attr->attr.name = "config_fontdata";
	fontdata_attr->attr.mode = S_IWUSR | S_IRUSR;
	fontdata_attr->read = ext_watch_access_read;
	fontdata_attr->write = ext_watch_access_write;
	fontdata_attr->size = size;

	ret = sysfs_create_bin_file(&chip->kobj, fontdata_attr);
	if (ret < 0) {
		t_watch_err(dev, "failed to create %s, %d\n",
		 		fontdata_attr->attr.name, ret);
		goto out_bin;
	}

	t_dev_dbg_base(dev, "file[%s] file created\n",
			fontdata_attr->attr.name);

	return 0;

out_bin:
	touch_kfree(dev, buf);

out:

	return ret;
}

static void ext_watch_fontdata_attr_free(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct watch_data *watch = (struct watch_data *)chip->watch;
	struct bin_attribute *fontdata_attr = &watch->fontdata_attr;
	int size = (int)fontdata_attr->size;

	t_dev_dbg_base(dev, "file[%s] file released\n",
			fontdata_attr->attr.name);

	sysfs_remove_bin_file(&chip->kobj, fontdata_attr);

	t_dev_dbg_base(dev, "font buffer[%Xh] released\n", size);

	if (watch->ext_wdata.font_data) {
		touch_kfree(dev, watch->ext_wdata.font_data);
		watch->ext_wdata.font_data = NULL;
	}
	watch->font_written_size = 0;
}

static int ext_watch_init(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct watch_data *watch = (struct watch_data *)chip->watch;
	int ret = 0;

	if (ext_quirk_check(dev)) {
		return 0;
	}

	if (siw_touch_boot_mode_check(dev) >= MINIOS_MFTS_FOLDER){
		return -EBUSY;
	}

	if (!watch) {
		t_watch_err(dev, "NULL watch\n");
		return -ENOMEM;
	}

	if (atomic_read(&watch->state.font_status) != FONT_READY){
		return -EBUSY;
	}

	t_watch_info(dev, "init\n");

	ret = ext_watch_set_cfg(dev, 0);
	if (ret < 0) {
		t_watch_err(dev, "failed to set watch cfg, %d\n", ret);
		goto out;
	}

	ret = ext_watch_display_onoff(dev);
	if (ret < 0) {
		t_watch_err(dev, "failed to set watch display control, %d\n", ret);
		goto out;
	}

out:
	 return ret;
}

struct siw_watch_type_table {
	int chip_type;
	int watch_type;
	int flag;
	/* */
	struct reset_area *win_fixed;	/* fixed 2nd screen window */
	/* */
	int font_type;
	int font_max_size;
	int font_crc_offset;
	u32 font_magic_code;
	/* */
	u32 dcst_offset;
	u32 dcst_data;
};

#define WATCH_FLAG_SKIP_FOR_LG4895		(0 |	\
										WATCH_FLAG_SKIP_GET_MODE |	\
										WATCH_FLAG_SKIP_SET_MODE |	\
										WATCH_FLAG_SKIP_LUT_UPDATE |	\
										0)

#define WATCH_FLAG_SKIP_FOR_SW49407		(0 |	\
										WATCH_FLAG_SKIP_MEM_CTRL |	\
										0)

static const struct reset_area watch_win_fixed_lg4895 = {
	.x1 = 200,
	.y1 = 0,
	.x2 = 720,
	.y2 = 80,
};

static const struct reset_area watch_win_fixed_sw49407 = {
	.x1 = 400,
	.y1 = 0,
	.x2 = 1440,
	.y2 = 160,
};

static const struct siw_watch_type_table watch_type_tables[] = {
	/* WATCH_TYPE_0 group */
	{	.chip_type = CHIP_LG4895,
		.watch_type = WATCH_TYPE_0,
		.flag = WATCH_FLAG_SKIP_FOR_LG4895,
		.win_fixed = (struct reset_area *)&watch_win_fixed_lg4895,
		.font_type = FONT_TYPE_0,
		.font_max_size = SIW_MAX_FONT_SIZE,
		.font_crc_offset = 4,
		.font_magic_code = SIW_FONT_MAGIC_CODE,
	},
	{	.chip_type = CHIP_SW49407,
		.watch_type = (WATCH_TYPE_0 | (1<<16)),
		.flag = WATCH_FLAG_SKIP_FOR_SW49407,
		.win_fixed = (struct reset_area *)&watch_win_fixed_sw49407,
		.font_type = FONT_TYPE_1,
		.font_max_size = ((45<<10) + 962),		//45.94KB
		.font_crc_offset = 16,
		.font_magic_code = SIW_FONT_MAGIC_CODE,
		.dcst_offset = 0x085,	//dcst_reg_oft
		.dcst_data = 0xFDB,		//dcst_reg_data
	},
	/* WATCH_TYPE_1 group */
	{	.chip_type = CHIP_LG4946,
		.watch_type = WATCH_TYPE_1,
		.flag = 0,
		.font_type = FONT_TYPE_1,
		.font_max_size = (87<<10),
		.font_crc_offset = 12,
		.font_magic_code = SIW_FONT_MAGIC_CODE,
	},
	{	.chip_type = CHIP_SW49406,
		.watch_type = WATCH_TYPE_1,
		.flag = 0,
		.font_type = FONT_TYPE_1,
		.font_max_size = (87<<10),
		.font_crc_offset = 12,
		.font_magic_code = SIW_FONT_MAGIC_CODE,
	},
#if 0
	{	.chip_type = CHIP_SW49408,
		.watch_type = (WATCH_TYPE_1 | (1<<16)),
		.flag = 0,
		.font_type = FONT_TYPE_1,
		.font_max_size = (87<<10),
		.font_crc_offset = 16,
		.font_magic_code = SIW_FONT_MAGIC_CODE,
	},
#endif
	/* */
	{ CHIP_NONE, 0, },	//End mark
};

static int ext_watch_scan_type(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct watch_data *watch = (struct watch_data *)chip->watch;
	struct siw_watch_type_table *table = (struct siw_watch_type_table *)watch_type_tables;

	while (1) {
		if (table->chip_type == CHIP_NONE) {
			break;
		}

		if (touch_chip_type(ts) == table->chip_type) {
			watch->type = table->watch_type;
			watch->flag = table->flag;

			watch->win_fixed = table->win_fixed;

			watch->font_type = table->font_type;
			watch->font_max_size = table->font_max_size;
			watch->font_crc_offset = table->font_crc_offset;
			watch->font_magic_code = table->font_magic_code;

			watch->dcst_offset = table->dcst_offset;
			watch->dcst_data = table->dcst_data;

			t_dev_dbg_base(dev, "[%s] watch table: %d(%d), %Xh, %d, %Xh, %Xh\n",
					touch_chip_name(ts),
					WATCH_TYPE(watch), WATCH_TYPE_VAR(watch),
					watch->flag,
					watch->font_type,
					watch->font_max_size,
					watch->font_magic_code);

			return 0;
		}

		table++;
	}

	watch->font_max_size = SIW_MAX_FONT_SIZE;
	watch->flag = WATCH_FLAG_SKIP_FOR_DEFAULT;
	watch->type = WATCH_TYPE_0;
	watch->font_type = FONT_TYPE_0;
	watch->font_max_size = (1<<10);
	watch->font_magic_code = SIW_FONT_MAGIC_CODE;

	t_watch_err(dev,
			"%s watch table not detected\n",
			touch_chip_name(ts));

	return -EFAULT;
//	return 0;
}

static int ext_watch_create_sysfs(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct device *idev = &ts->input->dev;
	//Caution : not ts->kobj
	struct kobject *kobj = &chip->kobj;
	struct kobject *parent;
	struct watch_data *watch;
	char *name = NULL;
	int ret = 0;

	watch = touch_kzalloc(dev, sizeof(*watch), GFP_KERNEL);
	if (!watch) {
		t_dev_err(dev, "failed to allocate memory for watch data\n");
		ret = -ENOMEM;
		goto out;
	}
	chip->watch = watch;
	t_dev_dbg_base(dev, "watch_data allocated\n");

	ret = ext_watch_scan_type(dev);
	if (ret < 0) {
		goto out_scan;
	}

	name = touch_ext_watch_name(ts);

	if (touch_flags(ts) & TOUCH_USE_VIRT_DIR_WATCH) {
		/*
		 * /sys/devices/virtual/
		 */
		parent = idev->kobj.parent->parent;
	} else {
		/*
		 * /sys/devices/virtual/input/
		 */
		parent = idev->kobj.parent;
	}
	ret = kobject_init_and_add(kobj, &ext_watch_kobj_type,
			parent, "%s", name);
	if (ret < 0) {
		t_watch_err(dev, "failed to create sysfs kobj\n");
		goto out_kobj;
	}

	ret = sysfs_create_group(kobj, &ext_watch_attribute_group);
	if (ret < 0) {
		t_watch_err(dev, "failed to create sysfs group\n");
		goto out_sys;
	}

	ret = ext_watch_fontdata_attr_init(dev);
	if (ret < 0) {
		t_watch_err(dev, "failed to create fontdata attr\n");
		goto out_attr;
	}

	INIT_DELAYED_WORK(&chip->font_download_work, ext_watch_font_download);

	ret = ext_watch_fontdata_check(dev);
	if (ret < 0) {
		goto out_font;
	}

	ret = ext_watch_fontdata_preload(dev);
	if (ret < 0) {
		/* Just skip */
	//	goto out_font;
	}

	t_dev_dbg_base(dev, "%s watch sysfs registered\n",
			touch_chip_name(ts));

	return 0;

out_font:
	ext_watch_fontdata_attr_free(dev);

out_attr:
	sysfs_remove_group(kobj, &ext_watch_attribute_group);

out_sys:
	kobject_del(kobj);

out_kobj:

out_scan:
	touch_kfree(dev, watch);
	chip->watch = NULL;

out:

	return ret;
}

static void ext_watch_remove_sysfs(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
//	struct device *idev = &ts->input->dev;
	//Caution : not ts->kobj
	struct kobject *kobj = &chip->kobj;

	if (chip->watch == NULL) {
		return;
	}

	cancel_delayed_work(&chip->font_download_work);

	ext_watch_fontdata_attr_free(dev);

	sysfs_remove_group(kobj, &ext_watch_attribute_group);

	kobject_del(kobj);

	touch_kfree(dev, chip->watch);
	chip->watch = NULL;

	t_dev_dbg_base(dev, "%s watch sysfs unregistered\n",
			touch_chip_name(ts));
}

int siw_hal_watch_sysfs(struct device *dev, int on_off)
{
	if (ext_quirk_check(dev)) {
		return 0;
	}

	if (on_off == DRIVER_INIT) {
		return ext_watch_create_sysfs(dev);
	}

	ext_watch_remove_sysfs(dev);
	return 0;
}

int siw_hal_watch_init(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);

	if (ext_quirk_check(dev)) {
		return 0;
	}

	if (chip->watch == NULL) {
		t_watch_err(dev, "NULL watch_data\n");
		return -ENOMEM;
	}

	return ext_watch_init(dev);
}

int siw_hal_watch_get_curr_time(struct device *dev, char *buf, int *len)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);

	if (ext_quirk_check(dev)) {
		return 0;
	}

	if (chip->watch == NULL) {
		t_watch_err(dev, "NULL watch_data\n");
		return -ENOMEM;
	}

	return ext_watch_get_curr_time(dev, buf, len);
}

void siw_hal_watch_display_off(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);

	if (ext_quirk_check(dev)) {
		return;
	}

	if (chip->watch == NULL) {
		t_watch_err(dev, "NULL watch_data\n");
		return;
	}

	ext_watch_display_off(dev);
}

int siw_hal_watch_chk_font_status(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);

	if (ext_quirk_check(dev)) {
		return 0;
	}

	if (chip->watch == NULL) {
		t_watch_err(dev, "NULL watch_data\n");
		return -ENOMEM;
	}

	ext_watch_init(dev);

	return ext_watch_chk_font_status(dev);
}

int siw_hal_watch_is_disp_waton(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct watch_data *watch = (struct watch_data *)chip->watch;

	return (!watch) ? 0 :
		watch->ext_wdata.time.disp_waton;
}

int siw_hal_watch_is_rtc_run(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct watch_data *watch = (struct watch_data *)chip->watch;

	return (!watch) ? 0 :
		(atomic_read(&watch->state.rtc_status) == RTC_RUN);
}

void siw_hal_watch_set_rtc_run(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct watch_data *watch = (struct watch_data *)chip->watch;

	if (watch)
		atomic_set(&watch->state.rtc_status, RTC_RUN);
}

void siw_hal_watch_set_rtc_clear(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct watch_data *watch = (struct watch_data *)chip->watch;

	if (watch)
		atomic_set(&watch->state.rtc_status, RTC_CLEAR);
}

void siw_hal_watch_rtc_on(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct watch_data *watch = (struct watch_data *)chip->watch;
	int watch_type;
	int rtc_on = 0;

	if (watch == NULL)
		return;

	watch_type = WATCH_TYPE(watch);

	switch (watch_type) {
	case WATCH_TYPE_1:
		rtc_on = 1;
		break;
	default:
		break;
	}

	if (rtc_on) {
		if (atomic_read(&watch->state.rtc_status) == RTC_CLEAR) {
			ext_watch_rtc_start(dev, EXT_WATCH_RTC_START);
		}
	}
}

void siw_hal_watch_set_font_empty(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct watch_data *watch = (struct watch_data *)chip->watch;

	if (watch) {
		atomic_set(&watch->state.font_status, FONT_EMPTY);
	}
}

void siw_hal_watch_set_cfg_blocked(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);

	atomic_set(&chip->block_watch_cfg, BLOCKED);
}

#endif	/* __SIW_SUPPORT_WATCH */


