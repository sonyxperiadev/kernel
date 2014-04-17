/*
 * Device driver for monitoring ambient light intensity in (lux)
 * proximity detection (prox), Gesture, and Beam functionality within the
 * AMS-TAOS TMG399X family of devices.
 *
 * Copyright (c) 2013, AMS-TAOS USA, Inc.
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __TMG399X_H
#define __TMG399X_H

#include <linux/types.h>
#include <linux/timer.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

/* Max number of segments allowable in LUX table */
#define TMG399X_MAX_LUX_TABLE_SIZE		9
#define MAX_DEFAULT_TABLE_BYTES (sizeof(int) * TMG399X_MAX_LUX_TABLE_SIZE)

/* Default LUX and Color coefficients */

#define D_Factor	241
#define R_Coef		144
#define G_Coef		1000
#define B_Coef		400
#define CT_Coef		(3972)
#define CT_Offset	(1672)

#define D_Factor1	1375
#define R_Coef1		-5
#define G_Coef1		1000
#define B_Coef1		452
#define CT_Coef1	5111
#define CT_Offset1	547

/* Default Beam configuration */
#define IRBEAM_CFG	0x29
#define IRBEAM_CARR	0x00
#define IRBEAM_NS	0x0A
#define IRBEAM_ISD	0x31
#define IRBEAM_NP	0x0A
#define IRBEAM_IPD	0x32
#define IRBEAM_DIV	0x32
#define IRBEAM_MAX_NUM_HOPS	15

/* Proximity threshold definition */
#define	PROX_THRESH_LOW		50
#define	PROX_THRESH_HIGH	80

struct device;

#define TMG399X_CMD_IRBEAM_INT_CLR	0xE3
#define TMG399X_CMD_PROX_INT_CLR	0xE5
#define TMG399X_CMD_ALS_INT_CLR		0xE6
#define TMG399X_CMD_NON_GES_INT_CLR	0xE7

#define PRX_PERSIST(p) (((p) & 0xf) << 4)
#define ALS_PERSIST(p) (((p) & 0xf) << 0)
#define PRX_PULSE_CNT(p) (((p-1) & 0x3f) << 0)
#define GES_PULSE_CNT(p) (((p-1) & 0x3f) << 0)

#define INT2OFFSET(offset, value) do {\
				if (value < 0)\
					offset = 128 - value; \
				else \
					offset = value; \
				} while (0)

#define I2C_ADDR_OFFSET	0x80

enum tmg399x_regs {
	TMG399X_CONTROL,
	TMG399X_ALS_TIME,
	TMG399X_RESV_1,
	TMG399X_WAIT_TIME,
	TMG399X_ALS_MINTHRESHLO,
	TMG399X_ALS_MINTHRESHHI,
	TMG399X_ALS_MAXTHRESHLO,
	TMG399X_ALS_MAXTHRESHHI,
	TMG399X_RESV_2,
	TMG399X_PRX_MINTHRESHLO,
	TMG399X_RESV_3,
	TMG399X_PRX_MAXTHRESHHI,
	TMG399X_PERSISTENCE,
	TMG399X_CONFIG_1,
	TMG399X_PRX_PULSE,
	TMG399X_GAIN,

	TMG399X_CONFIG_2,
	TMG399X_REVID,
	TMG399X_CHIPID,
	TMG399X_STATUS,
	TMG399X_CLR_CHANLO,
	TMG399X_CLR_CHANHI,
	TMG399X_RED_CHANLO,
	TMG399X_RED_CHANHI,
	TMG399X_GRN_CHANLO,
	TMG399X_GRN_CHANHI,
	TMG399X_BLU_CHANLO,
	TMG399X_BLU_CHANHI,
	TMG399X_PRX_CHAN,
	TMG399X_PRX_OFFSET_NE,
	TMG399X_PRX_OFFSET_SW,
	TMG399X_CONFIG_3,

	TMG399X_GES_ENTH,
	TMG399X_GES_EXTH,
	TMG399X_GES_CFG_1,
	TMG399X_GES_CFG_2,
	TMG399X_GES_OFFSET_N,
	TMG399X_GES_OFFSET_S,
	TMG399X_GES_PULSE,
	TMG399X_GES_OFFSET_W,
	TMG399X_GES_RESV,
	TMG399X_GES_OFFSET_E,
	TMG399X_GES_CFG_3,
	TMG399X_GES_CFG_4,
	TMG399X_RESV_4,
	TMG399X_RESV_5,
	TMG399X_GES_FLVL,
	TMG399X_GES_STAT,

	TMG399X_REG_MAX,
};

enum tmg399x_gesfifo_regs {
	TMG399X_GES_NFIFO = 0x7C,
	TMG399X_GES_SFIFO,
	TMG399X_GES_WFIFO,
	TMG399X_GES_EFIFO,
};

enum tmg399x_irbeam_regs {
	TMG399X_IRBEAM_CFG = 0x20,
	TMG399X_IRBEAM_CARR,
	TMG399X_IRBEAM_NS,
	TMG399X_IRBEAM_ISD,
	TMG399X_IRBEAM_NP,
	TMG399X_IRBEAM_IPD,
	TMG399X_IRBEAM_DIV,
	TMG399X_IRBEAM_LEN,
	TMG399X_IRBEAM_STAT,
};

enum tmg399x_remote_regs {
	TMG399X_REMOTE_CFG = 0x20,
	TMG399X_REMOTE_CARR,
	TMG399X_REMOTE_NS,
	TMG399X_REMOTE_DLY2T,
	TMG399X_REMOTE_NCP,
	TMG399X_REMOTE_CPOFF,
	TMG399X_REMOTE_DIV,
	TMG399X_REMOTE_LEN,
	TMG399X_REMOTE_STAT,
	TMG399X_REMOTE_SLEN,
};

enum tmg399x_pwr_state {
	POWER_ON,
	POWER_OFF,
	POWER_STANDBY,
};

enum tmg399x_en_reg {
	TMG399X_EN_PWR_ON   = (1 << 0),
	TMG399X_EN_ALS      = (1 << 1),
	TMG399X_EN_PRX      = (1 << 2),
	TMG399X_EN_WAIT     = (1 << 3),
	TMG399X_EN_ALS_IRQ  = (1 << 4),
	TMG399X_EN_PRX_IRQ  = (1 << 5),
	TMG399X_EN_GES      = (1 << 6),
	TMG399X_EN_BEAM     = (1 << 7),
};

enum tmg399x_cfgl_reg {
	WLONG          = (1 << 1),
};

enum tmg399x_ppulse_reg {
	PPLEN_4US      = (0 << 6),
	PPLEN_8US      = (1 << 6),
	PPLEN_16US     = (2 << 6),
	PPLEN_32US     = (3 << 6),
};

enum tmg399x_ctrl_reg {
	AGAIN_1        = (0 << 0),
	AGAIN_4        = (1 << 0),
	AGAIN_16       = (2 << 0),
	AGAIN_64       = (3 << 0),
	PGAIN_1        = (0 << 2),
	PGAIN_2        = (1 << 2),
	PGAIN_4        = (2 << 2),
	PGAIN_8        = (3 << 2),
	PDRIVE_100MA   = (0 << 6),
	PDRIVE_50MA    = (1 << 6),
	PDRIVE_25MA    = (2 << 6),
	PDRIVE_12MA    = (3 << 6),
};

enum tmg399x_cfg2_reg {
	LEDBOOST_100   = (0 << 4),
	LEDBOOST_150   = (1 << 4),
	LEDBOOST_200   = (2 << 4),
	LEDBOOST_300   = (3 << 4),
	CPSIEN         = (1 << 6),
	PSIEN          = (1 << 7),
};

enum tmg399x_status {
	TMG399X_ST_ALS_VALID  = (1 << 0),
	TMG399X_ST_PRX_VALID  = (1 << 1),
	TMG399X_ST_GES_IRQ    = (1 << 2),
	TMG399X_ST_BEAM_IRQ   = (1 << 3),
	TMG399X_ST_ALS_IRQ    = (1 << 4),
	TMG399X_ST_PRX_IRQ    = (1 << 5),
	TMG399X_ST_PRX_SAT    = (1 << 6),
	TMG399X_ST_CP_SAT     = (1 << 7),
};

enum tmg399x_cfg3_reg {
	PMASK_E        = (1 << 0),
	PMASK_W        = (1 << 1),
	PMASK_S        = (1 << 2),
	PMASK_N        = (1 << 3),
	SAI            = (1 << 4),
	PCMP           = (1 << 5),
};

enum tmg399x_ges_cfg1_reg {
	GEXPERS_1      = (0 << 0),
	GEXPERS_2      = (1 << 0),
	GEXPERS_4      = (2 << 0),
	GEXPERS_7      = (3 << 0),
	GEXMSK_E       = (1 << 2),
	GEXMSK_W       = (1 << 3),
	GEXMSK_S       = (1 << 4),
	GEXMSK_N       = (1 << 5),
	GEXMSK_ALL     = (0xF << 2),
	FIFOTH_1       = (0 << 6),
	FIFOTH_4       = (1 << 6),
	FIFOTH_8       = (2 << 6),
	FIFOTH_16      = (3 << 6),
};

enum tmg399x_ges_cfg2_reg {
	GWTIME_0       = (0 << 0),
	GWTIME_3       = (1 << 0),
	GWTIME_6       = (2 << 0),
	GWTIME_8       = (3 << 0),
	GWTIME_14      = (4 << 0),
	GWTIME_22      = (5 << 0),
	GWTIME_30      = (6 << 0),
	GWTIME_39      = (7 << 0),
	GLDRIVE_100    = (0 << 3),
	GLDRIVE_50     = (1 << 3),
	GLDRIVE_25     = (2 << 3),
	GLDRIVE_12     = (3 << 3),
	GGAIN_1        = (0 << 5),
	GGAIN_2        = (1 << 5),
	GGAIN_4        = (2 << 5),
	GGAIN_8        = (3 << 5),
};

enum tmg399x_gpulse_reg {
	GPLEN_4US      = (0 << 6),
	GPLEN_8US      = (1 << 6),
	GPLEN_16US     = (2 << 6),
	GPLEN_32US     = (3 << 6),
};

enum tmg399x_ges_cfg3_reg {
	GBOTH_PAIR     = (0 << 0),
	GONLY_NS       = (1 << 0),
	GONLY_WE       = (2 << 0),
};

enum tmg399x_ges_cfg4 {
	TMG399X_GES_MODE     = (1 << 0),
	TMG399X_GES_EN_IRQ   = (1 << 1),
	TMG399X_GES_INT_CLR  = (1 << 2),
};

enum tmg399x_ges_status {
	TMG399X_GES_VALID     = (1 << 0),
	TMG399X_GES_FOV       = (1 << 1),
};

enum {
	TMG399X_ALS_GAIN_MASK = (3 << 0),
	TMG399X_PRX_GAIN_MASK = (3 << 2),
	TMG399X_LDRIVE_MASK   = (3 << 6),
	TMG399X_ALS_AGL_MASK  = (1 << 2),
	TMG399X_ALS_AGL_BOOST = 2,
	TMG399X_ATIME_PER_100 = 278,
	SCALE_SHIFT = 11,
	RATIO_SHIFT = 10,
	MAX_ALS_VALUE = 0xffff,
	MIN_ALS_VALUE = 10,
	GAIN_SWITCH_LEVEL = 100,
	TMG399X_GES_ST_MASK   = (3 << 0),
};

enum tmg399x_caloffsetstates {
	START_CALOFF,
	CHECK_PROX_NE,
	CHECK_PROX_SW,
	CHECK_NSWE_ZERO,
	CALOFF_OK
};

enum tmg399x_caloffsetdirections {
	DIR_NONE,
	DIR_UP,
	DIR_DOWN
};

enum tmg399x_datatypes {
	PROX_DATA,
	GES_DATA,
	RGBC_DATA
};

struct tmg399x_als_info {
	u32 cpl;
	u32 saturation;
	u16 clear_raw;
	u16 red_raw;
	u16 green_raw;
	u16 blue_raw;
	u16 lux;
	u16 cct;
	s16 ir;
};

struct tmg399x_prox_info {
	int raw;
	int detected;
};

struct tmg399x_parameters {
	u8 gpio_irq;
	u8 als_time;
	u8 als_gain;
	u8 wait_time;
	u8 prox_th_min;
	u8 prox_th_max;
	u8 persist;
	u8 als_prox_cfg1;
	u8 prox_pulse;
	u8 prox_gain;
	u8 ldrive;
	u8 als_prox_cfg2;
	s8 prox_offset_ne;
	s8 prox_offset_sw;
	u8 als_prox_cfg3;

	u8 ges_entry_th;
	u8 ges_exit_th;
	u8 ges_cfg1;
	u8 ges_cfg2;
	s8 ges_offset_n;
	s8 ges_offset_s;
	u8 ges_pulse;
	s8 ges_offset_w;
	s8 ges_offset_e;
	u8 ges_dimension;
};

struct tmg399x_ges_nswe {
	u8 north;
	u8 south;
	u8 west;
	u8 east;
};

struct tmg399x_beam_settings {
	u8 beam_cfg;
	u8 beam_carr;
	u8 beam_ns;
	u8 beam_isd;
	u8 beam_np;
	u8 beam_ipd;
	u8 beam_div;
};

struct tmg399x_chip {
	struct mutex lock;
	struct i2c_client *client;
	struct work_struct irq_work;
	struct work_struct beam_work;
	struct tmg399x_prox_info prx_inf;
	struct tmg399x_als_info als_inf;
	struct tmg399x_parameters params;
	struct tmg399x_beam_settings beam_settings;
	struct tmg399x_beam_settings beam_hop[15];
	struct tmg399x_i2c_platform_data *pdata;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend suspend_desc;
#endif
	u8 shadow[48];
	struct input_dev *p_idev;
	struct input_dev *a_idev;
	struct timer_list rgbc_timer;
	int in_suspend;
	int wake_irq;
	int irq_pending;
	bool unpowered;
	bool als_enabled;
	bool wait_enabled;
	bool prx_enabled;
	bool beam_enabled;
	bool ges_enabled;
	bool rgbc_poll_flag;
	struct tmg399x_ges_nswe ges_raw_data[32];
	struct lux_segment *segment;
	int segment_num;
	int seg_num_max;
	bool als_gain_auto;
	u8 device_index;
	u8 pre_beam_xmit_state;
	u8 bc_symbol_table[128];
	u16 bc_nibbles;
	u16 hop_count;
	u8 hop_next_slot;
	u8 hop_index;
};

struct lux_segment {
	int d_factor;
	int r_coef;
	int g_coef;
	int b_coef;
	int ct_coef;
	int ct_offset;
};

struct tmg399x_i2c_platform_data {
	/* The following callback for power events received and handled by
	   the driver.  Currently only for SUSPEND and RESUME */
	int (*platform_power)(struct device *dev, enum tmg399x_pwr_state state);
	int (*platform_init)(void);
	void (*platform_teardown)(struct device *dev);
	char const *prox_name;
	char const *als_name;
	struct tmg399x_parameters parameters;
	struct tmg399x_beam_settings beam_settings;
	struct tmg3993_beam_settings *beam_hop[15];
	bool proximity_can_wake;
	bool als_can_wake;
	struct lux_segment *segment;
	int segment_num;
};


#endif /* __TMG399X_H */
