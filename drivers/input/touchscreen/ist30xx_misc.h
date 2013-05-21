/*
 *  Copyright (C) 2010, Imagis Technology Co. Ltd. All Rights Reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#ifndef __IST30XX_MISC_H__
#define __IST30XX_MISC_H__


#define IST30XX_TUNES               (1)

#define IST30XX_SENSOR_ADDR         (0x40009000)
#define IST30XX_RAW_ADDR            (0x40100400)
#define IST30XX_FILTER_ADDR         (0x40100800)

#define IST30XX_RX_CNT_ADDR         (0x20000038)
#define IST30XX_CONFIG_ADDR         (0x20000040)

struct TSP_CH_INFO {
	u8	x;
	u8	y;
};
struct TSP_FRAME_BUF {
	u16 **raw;
	u16 **base;
	u16 **filter;
	u16	min_raw;
	u16	max_raw;
	u16	min_base;
	u16	max_base;
	u16	int_len;
	u16	mod_len;
};
struct TSP_DIRECTION {
	bool	txch_y;
	u8	swap_xy;
	u8	flip_x;
	u8	flip_y;
};
typedef struct _TSP_INFO {
	struct TSP_CH_INFO	mod;
	struct TSP_CH_INFO	intl;
	struct TSP_DIRECTION	dir;
	struct TSP_FRAME_BUF	buf;
	int			height;
	int			width;
} TSP_INFO;
typedef struct _TKEY_INFO {
	bool	enable;
	bool	tx_line;
	u8	axis_chnum;
	u8	ch_num[5];
} TKEY_INFO;

int ist30xx_enter_debug_mode(void);
int ist30xx_parse_tsp_node(u16 *raw_buf, u16 *base_buf);
int ist30xx_read_tsp_node(u16 *raw_buf, u16 *base_buf);

int ist30xx_init_misc_sysfs(void);

#endif  /*__IST30XX_MISC_H__*/
