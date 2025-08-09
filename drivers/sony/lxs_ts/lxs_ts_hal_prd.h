/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * lxs_ts_hal_prd.h
 *
 * LXS touch raw-data debugging
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __LXS_TS_HAL_PRD_H
#define __LXS_TS_HAL_PRD_H

#include "lxs_ts.h"

#define __PRD_USE_BIG_ATTR

#define __PRD_SUPPORT_SD_TEST

//#define __PRD_TEST_FILE_OFF

#define __PRD_SUPPORT_SD_MIN_MAX

#define SELF_TEST_PATH_DEFAULT	"/sdcard/lxs_ts_self_test.txt"
#define SPEC_FILE_EXT_DEFAULT	"/sdcard/lxs_ts_spec.txt"
#define SPEC_FILE_INT_DEFAULT	"lxs_ts_spec.txt"

enum {
	PRD_SPEC_LINE_SIZE = (128<<10),
	/* */
	PRD_LOG_POOL_SIZE = (8<<10),	//custom for SOMC SW82907
	PRD_LOG_LINE_SIZE = (1<<10),
	/* */
	PRD_RAWBUF_SIZE	= (32<<10),
	/* */
	PRD_BUF_DUMMY		= 128,		//dummy size for avoiding memory overflow
};

enum {
	PRD_APP_INFO_SIZE	= 32,
	/* */
	PRD_M1_COL_SIZE	= 2,
	PRD_SHORT_COL_SZ	= 4,
};

enum {
	TIME_INFO_SKIP = 0,
	TIME_INFO_WRITE,
};

enum {
	PRD_CTRL_IDX_RAW = 0,
	PRD_CTRL_IDX_BASE,
	PRD_CTRL_IDX_DELTA,
	PRD_CTRL_IDX_LABEL,
	PRD_CTRL_IDX_DEBUG,
	PRD_CTRL_IDX_P_POS_RAW,
	PRD_CTRL_IDX_P_POS_BASE,
	PRD_CTRL_IDX_P_POS_DELTA,
	PRD_CTRL_IDX_P_POS_LABEL,
	PRD_CTRL_IDX_P_POS_DEBUG,
	PRD_CTRL_IDX_P_RAW,
	PRD_CTRL_IDX_P_BASE,
	PRD_CTRL_IDX_P_DELTA,
	PRD_CTRL_IDX_P_DEBUG,
	PRD_CTRL_IDX_SYSR,
	PRD_CTRL_IDX_SYSB,
	PRD_CTRL_IDX_SYSD,
	PRD_CTRL_IDX_P_POS_SYSR,
	PRD_CTRL_IDX_P_SYSR,
	PRD_CTRL_IDX_P_SYS_DEBUG,
	PRD_CTRL_IDX_MAX,
};

enum {
	PRD_CTRL_IDX_SD_OPEN_NODE = 0,
	PRD_CTRL_IDX_SD_SHORT_NODE,
	PRD_CTRL_IDX_SD_OPEN_RX_NODE,
	PRD_CTRL_IDX_SD_OPEN_TX_NODE,
	PRD_CTRL_IDX_SD_SHORT_RX_NODE,
	PRD_CTRL_IDX_SD_SHORT_TX_NODE,
	PRD_CTRL_IDX_SD_SHORT_CH_NODE,
	PRD_CTRL_IDX_SD_SHORT_MUX_NODE,
	PRD_CTRL_IDX_SD_U3_M2_RAW,
	PRD_CTRL_IDX_SD_U3_M1_RAW,
	PRD_CTRL_IDX_SD_U3_JITTER,
	PRD_CTRL_IDX_SD_U3_M1_JITTER,
	PRD_CTRL_IDX_SD_U3_M2_RAW_SELF,
	PRD_CTRL_IDX_SD_U3_JITTER_SELF,
	PRD_CTRL_IDX_SD_U3_P_JITTER,
	PRD_CTRL_IDX_SD_CALB,
	PRD_CTRL_IDX_SD_SYNC,
	PRD_CTRL_IDX_SD_IRQ,
	PRD_CTRL_IDX_SD_RST,
	PRD_CTRL_IDX_SD_MAX,
};

enum {
	PRD_CTRL_IDX_LPWG_SD_OPEN = 0,
	PRD_CTRL_IDX_LPWG_SD_GAP_X,
	PRD_CTRL_IDX_LPWG_SD_GAP_Y,
	PRD_CTRL_IDX_LPWG_SD_GAP_A,
	PRD_CTRL_IDX_LPWG_SD_SHORT_GND,
	PRD_CTRL_IDX_LPWG_SD_SHORT_TRX,
	PRD_CTRL_IDX_LPWG_SD_SHORT_VDD,
	PRD_CTRL_IDX_LPWG_SD_U0_M2_RAW,
	PRD_CTRL_IDX_LPWG_SD_U0_M1_RAW,
	PRD_CTRL_IDX_LPWG_SD_U0_JITTER,
	PRD_CTRL_IDX_LPWG_SD_U0_M1_JITTER,
	PRD_CTRL_IDX_LPWG_SD_U0_M2_RAW_SELF,
	PRD_CTRL_IDX_LPWG_SD_U0_JITTER_SELF,
	PRD_CTRL_IDX_LPWG_SD_U0_P_JITTER,
	PRD_CTRL_IDX_LPWG_SD_MAX,
};

#define RAW_CTRL_INIT(_id, _name, _cmd, _data_fmt, _flag, _row, _col, _offset, _get_func, _mod_func, _prt_func)	\
		{	\
			.id = _id, .name = _name, .cmd = _cmd,	\
			.data_fmt = _data_fmt, .flag = _flag,\
			.row_size = _row, .col_size = _col,	\
			.offset = _offset, .get_raw_func = _get_func, .mod_raw_func = _mod_func, .prt_raw_func = _prt_func,	\
		}

#define SD_CTRL_OD_INIT(_id, _name, _cmd, _data_fmt, _flag, _row, _col, _offset, _odd_func)	\
		{	\
			.id = _id, .name = _name, .cmd = _cmd,	\
			.data_fmt = _data_fmt, .flag = _flag,\
			.row_size = _row, .col_size = _col,	\
			.offset = _offset, .odd_sd_func = _odd_func,	\
			.is_sd = 1, .is_os = 0,	\
		}

#define SD_CTRL_OS_INIT(_id, _name, _cmd, _data_fmt, _flag, _row, _col, _offset, _get_func, _mod_func, _prt_func, _cmp_func, _delay)	\
		{	\
			.id = _id, .name = _name, .cmd = _cmd,	\
			.data_fmt = _data_fmt, .flag = _flag,\
			.row_size = _row, .col_size = _col,	\
			.offset = _offset, .get_sd_func = _get_func, .mod_sd_func = _mod_func, .prt_sd_func = _prt_func, .cmp_sd_func = _cmp_func,	\
			.delay = _delay, .is_sd = 1, .is_os = 1, \
		}

#define SD_CTRL_INIT(_id, _name, _cmd, _data_fmt, _flag, _row, _col, _offset, _get_func, _mod_func, _prt_func, _cmp_func, _delay)	\
		{	\
			.id = _id, .name = _name, .cmd = _cmd,	\
			.data_fmt = _data_fmt, .flag = _flag,\
			.row_size = _row, .col_size = _col,	\
			.offset = _offset, .get_sd_func = _get_func, .mod_sd_func = _mod_func, .prt_sd_func = _prt_func, .cmp_sd_func = _cmp_func,	\
			.delay = _delay, .is_sd = 1, .is_os = 0,	\
		}

#define LPWG_SD_OS_INIT(_id, _name, _cmd, _data_fmt, _flag, _row, _col, _offset, _get_func, _mod_func, _prt_func, _cmp_func, _delay)	\
		{	\
			.id = _id, .name = _name, .cmd = _cmd,	\
			.data_fmt = _data_fmt, .flag = _flag,\
			.row_size = _row, .col_size = _col,	\
			.offset = _offset, .get_sd_func = _get_func, .mod_sd_func = _mod_func, .prt_sd_func = _prt_func, .cmp_sd_func = _cmp_func,	\
			.delay = _delay, .is_sd = 0, .is_os = 1, \
		}

#define LPWG_SD_CTRL_INIT(_id, _name, _cmd, _data_fmt, _flag, _row, _col, _offset, _get_func, _mod_func, _prt_func, _cmp_func, _delay)	\
		{	\
			.id = _id, .name = _name, .cmd = _cmd,	\
			.data_fmt = _data_fmt, .flag = _flag,\
			.row_size = _row, .col_size = _col,	\
			.offset = _offset, .get_sd_func = _get_func, .mod_sd_func = _mod_func, .prt_sd_func = _prt_func, .cmp_sd_func = _cmp_func,	\
			.delay = _delay, .is_sd = 0, .is_os = 0, \
		}

enum {
	PRD_RAW_FLAG_PEN		= BIT(0),
	/* */
	PRD_RAW_FLAG_TBL_RXTX	= BIT(4),	//keep table value : row_size, col_size
	PRD_RAW_FLAG_TBL_OFFSET	= BIT(5),	//keep table value : offset
	/* */
	PRD_RAW_FLAG_IGN_EDGE_T	= BIT(8),	//Ignore Top Edge
	PRD_RAW_FLAG_IGN_EDGE_B	= BIT(9),	//Ignore Bottom Edge
	/* */
	PRD_RAW_FLAG_DATA_TMP	= BIT(12),	//Final data is stored in tmp buffer
	PRD_RAW_FLAG_CMP16		= BIT(13),
	/* */
	PRD_RAW_FLAG_FW_SKIP	= BIT(16),
	PRD_RAW_FLAG_FW_NO_STS	= BIT(17),
	PRD_RAW_FLAG_RESP_LSB	= BIT(18),
	/* */
	PRD_RAW_FLAG_IGN_ROW_T	= BIT(24),	//Ignore ROW Top line
	PRD_RAW_FLAG_IGN_ROW_B	= BIT(25),	//Ignore ROW Bottom line
	PRD_RAW_FLAG_IGN_COL_R	= BIT(26),	//Ignore COL Right line
	PRD_RAW_FLAG_IGN_COL_L	= BIT(27),	//Ignore COL Left line
	/* */
	PRD_RAW_FLAG_TYPE_SYS	= BIT(31),
};

enum {
	PRD_DATA_FMT_MASK = 0x03,	/* 8bit or 16bit */
	PRD_DATA_FMT_S16	= BIT(0),
	PRD_DATA_FMT_U16	= BIT(1),
	PRD_DATA_FMT_PAD	= BIT(2),
	/* */
	PRD_DATA_FMT_DIM	= BIT(4),
	/* */
	PRD_DATA_FMT_OS_DATA	= BIT(8),
	PRD_DATA_FMT_SELF		= BIT(9),
	PRD_DATA_FMT_RXTX		= BIT(10),
	PRD_DATA_FMT_DATA		= BIT(11),
	/* */
	PRD_DATA_FMT_SYS_ROW	= BIT(12),
	PRD_DATA_FMT_SYS_COL	= BIT(13),
	PRD_DATA_FMT_SYS_PDATA	= BIT(14),
	PRD_DATA_FMT_SYS_PDBG	= BIT(15),
	/* */
	PRD_DATA_FMT_SELF_TX_FIRST	= BIT(16),	// for print_rxtx
	PRD_DATA_FMT_SELF_MISALIGN	= BIT(17),	// for print_raw
	PRD_DATA_FMT_SELF_REV_C		= BIT(18),
	PRD_DATA_FMT_SELF_REV_R	 	= BIT(19),
	/* */
	PRD_DATA_FMT_SELF_REV_DIR	= (PRD_DATA_FMT_SELF_REV_C|PRD_DATA_FMT_SELF_REV_R),
	PRD_DATA_FMT_SELF_REV_CHK	= (PRD_DATA_FMT_SELF_MISALIGN|PRD_DATA_FMT_SELF_REV_DIR),
};

struct lxs_prd_dim_reg {
	u32	type:8;
	u32	rev_rx:1;
	u32	rev_tx:1;
	u32	rev_ord:1;
	u32	rsvd:(24-3);
} __packed;

#define FMT_TYPE_SWAP_1		(PRD_DATA_FMT_SELF_MISALIGN)
#define FMT_TYPE_SWAP_2		(PRD_DATA_FMT_SELF_MISALIGN | PRD_DATA_FMT_SELF_REV_C | PRD_DATA_FMT_SELF_REV_R)
#define FMT_TYPE_SWAP_3		(PRD_DATA_FMT_SELF_MISALIGN | PRD_DATA_FMT_SELF_REV_R)
#define FMT_TYPE_SWAP_4		(PRD_DATA_FMT_SELF_MISALIGN | PRD_DATA_FMT_SELF_REV_C)
#define FMT_TYPE_FLIP_V		(PRD_DATA_FMT_SELF_REV_R)
#define FMT_TYPE_FLIP_H		(PRD_DATA_FMT_SELF_REV_C)
#define FMT_TYPE_FLIP_C		(PRD_DATA_FMT_SELF_REV_C | PRD_DATA_FMT_SELF_REV_R)

#define PRD_DIM_TBL(_type, _swap, _fv, _fh, _fmt)	\
	{	.type = _type, .swap = _swap, .flip_v = _fv, .flip_h = _fh,	.data_fmt = _fmt,	}

struct lxs_prd_dim_ctl {
	int type;
	int rev_rx;
	int rev_tx;
	int rev_ord;
	/* */
	int swap;			// on : 1 ~ , off : 0
	int flip_v;			// on : 1, off 0
	int flip_h;			// on : 1, off 0
	int data_fmt;
};

static const struct lxs_prd_dim_ctl prd_dim_table[] = {
	PRD_DIM_TBL(1, 1, 0, 0, FMT_TYPE_SWAP_1),
	PRD_DIM_TBL(2, 2, 0, 0, FMT_TYPE_SWAP_2),
	PRD_DIM_TBL(3, 3, 0, 0, FMT_TYPE_SWAP_3),
	PRD_DIM_TBL(4, 4, 0, 0, FMT_TYPE_SWAP_4),
	PRD_DIM_TBL(5, 0, 0, 1, FMT_TYPE_FLIP_H),
	PRD_DIM_TBL(6, 0, 1, 0, FMT_TYPE_FLIP_V),
	PRD_DIM_TBL(7, 0, 1, 1, FMT_TYPE_FLIP_C),
	/* */
	PRD_DIM_TBL(11, 0, 1, 0, FMT_TYPE_FLIP_V),
	PRD_DIM_TBL(12, 0, 0, 1, FMT_TYPE_FLIP_H),
	PRD_DIM_TBL(13, 0, 1, 1, FMT_TYPE_FLIP_C),
	PRD_DIM_TBL(14, 0, 0, 0, 0),
	PRD_DIM_TBL(15, 1, 0, 0, FMT_TYPE_SWAP_1),
	PRD_DIM_TBL(16, 2, 0, 0, FMT_TYPE_SWAP_2),
	PRD_DIM_TBL(17, 4, 0, 0, FMT_TYPE_SWAP_4),
	/* */
	PRD_DIM_TBL(21, 0, 0, 1, FMT_TYPE_FLIP_H),
	PRD_DIM_TBL(22, 0, 1, 0, FMT_TYPE_FLIP_V),
	PRD_DIM_TBL(23, 0, 0, 0, 0),
	PRD_DIM_TBL(24, 0, 1, 1, FMT_TYPE_FLIP_C),
	PRD_DIM_TBL(25, 2, 0, 0, FMT_TYPE_SWAP_2),
	PRD_DIM_TBL(26, 1, 0, 0, FMT_TYPE_SWAP_1),
	PRD_DIM_TBL(27, 3, 0, 0, FMT_TYPE_SWAP_3),
	/* */
	PRD_DIM_TBL(31, 2, 0, 0, FMT_TYPE_SWAP_2),
	PRD_DIM_TBL(32, 1, 0, 0, FMT_TYPE_SWAP_1),
	PRD_DIM_TBL(33, 4, 0, 0, FMT_TYPE_SWAP_4),
	PRD_DIM_TBL(34, 3, 0, 0, FMT_TYPE_SWAP_3),
	PRD_DIM_TBL(35, 0, 1, 0, FMT_TYPE_FLIP_V),
	PRD_DIM_TBL(35, 0, 0, 1, FMT_TYPE_FLIP_H),
	PRD_DIM_TBL(36, 0, 0, 0, 0),
	/* */
	PRD_DIM_TBL(-1, 0, 0, 0, 0),	/* End mark */
};

/*
 * invalid node zone : 'col_s <= col <= col_e' & 'row_s <= row <= row_e'
 */
struct lxs_prd_empty_node {
	int col_s;
	int col_e;
	int row_s;
	int row_e;
};

struct lxs_prd_raw_pad {
	int row;
	int col;
};

struct lxs_prd_raw_ctrl {
	int id;
	char *name;
	int cmd;
	u32 data_fmt;
	u32 data_dim;	//TBD
	u32 flag;
	int row_size;
	int col_size;
	int row_act;	//actual row for print
	int col_act;	//actual col for print
	int data_size;
	u32 offset;
	/* */
	char *raw_buf;
	char *raw_tmp;
	char *prt_buf;
	void *priv;	/* TBD */
	/* */
	int (*get_raw_func)(void *prd_data, void *ctrl_data);
	int (*mod_raw_func)(void *prd_data, void *ctrl_data);
	int (*prt_raw_func)(void *prd_data, void *ctrl_data, int prt_size);
	/* */
	struct lxs_prd_raw_pad *pad;	//pad info
	int is_pad;
	/* */
	int is_dim;		//TBD
};

struct lxs_prd_sd_ctrl {
	int id;
	char *name;
	int cmd;
	u32 data_fmt;
	u32 data_dim;	//TBD
	u32 flag;
	int row_size;
	int col_size;
	int row_act;	//actual row for print
	int col_act;	//actual col for print
	int data_size;
	u32 offset;
	/* */
	char *raw_buf;
	char *raw_tmp;
	char *prt_buf;
	void *priv;	/* TBD */
	/* */
	int (*odd_sd_func)(void *prd_data, void *ctrl_data);	/* specific case */
	int (*get_sd_func)(void *prd_data, void *ctrl_data);
	int (*mod_sd_func)(void *prd_data, void *ctrl_data);
	int (*prt_sd_func)(void *prd_data, void *ctrl_data);
	int (*cmp_sd_func)(void *prd_data, void *ctrl_data);
	/* */
	int is_sd;	//1: sd, 0: lpwg_sd
	int is_os;	//1: open/short, 0: else
	int delay;
	/* */
	struct lxs_prd_raw_pad *pad;	//pad info
	int is_pad;
	/* */
	int is_dim;		//TBD
	/* */
	struct lxs_prd_empty_node *empty_node;
	/* */
	int cmp_min;
	int cmp_max;
	/* */
	int min;
	int max;
};

struct lxs_prd_data {
	void *ts;
	struct device *dev;
	struct mutex lock;
	/* */
	int chip_type;
	int panel_type;
	int file_wr_off;
	int file_rd_off;
	int dbg_mask;
	int test_type;
	/* */
	int self;
	int row;
	int col;
	int row_adj;
	int col_adj;
	int ch;
	int raw_base;
	int sys_base;
	int sys_cmd;
	u32 sys_dbg_addr;
	u32 sys_pdata_rx;
	u32 sys_pdata_tx;
	u32 sys_pdebug_rx;
	u32 sys_pdebug_tx;
	/* */
	int raw_lookup_table[PRD_CTRL_IDX_MAX];
	int sd_lookup_table[PRD_CTRL_IDX_SD_MAX];
	int lpwg_sd_lookup_table[PRD_CTRL_IDX_LPWG_SD_MAX];
	int num_raw_ctrl;
	int num_sd_ctrl;
	int num_lpwg_sd_ctrl;
	const struct lxs_prd_raw_ctrl *tbl_raw;
	const struct lxs_prd_sd_ctrl *tbl_sd;
	const struct lxs_prd_sd_ctrl *tbl_lpwg_sd;
	struct lxs_prd_raw_ctrl *raw_ctrl;
	struct lxs_prd_sd_ctrl *sd_ctrl;
	struct lxs_prd_sd_ctrl *lpwg_sd_ctrl;
	struct lxs_prd_sd_ctrl sd_run;
	/* */
	struct lxs_prd_raw_ctrl *orig_raw;
	struct lxs_prd_raw_ctrl conv_raw;
	struct lxs_prd_raw_ctrl conv_app;
	struct lxs_prd_sd_ctrl *orig_sd;
	struct lxs_prd_sd_ctrl conv_sd;
	/* */
	struct lxs_prd_dim_ctl raw_dim_ctl;
	struct lxs_prd_dim_ctl sys_dim_ctl;
	struct lxs_prd_dim_ctl sd_dim_ctl;
	/* */
	struct lxs_prd_dim_reg raw_dim_reg;
	struct lxs_prd_dim_reg sys_dim_reg;
	/* */
	char spec_line[PRD_SPEC_LINE_SIZE + PRD_BUF_DUMMY];
	char log_line[PRD_LOG_LINE_SIZE + PRD_BUF_DUMMY];
	char log_pool[PRD_LOG_POOL_SIZE + PRD_BUF_DUMMY];
	char log_bin[PRD_LOG_POOL_SIZE + PRD_BUF_DUMMY];
	char raw_buf[PRD_RAWBUF_SIZE + PRD_BUF_DUMMY];
	char raw_tmp[PRD_RAWBUF_SIZE + PRD_BUF_DUMMY];
	char raw_app[PRD_RAWBUF_SIZE + PRD_BUF_DUMMY];
	char raw_c[128<<1];
	char raw_r[128<<1];
	/* */
#if 0
	u16 cmp_lower_array[PRD_RAWBUF_SIZE + PRD_BUF_DUMMY];
	u16 cmp_upper_array[PRD_RAWBUF_SIZE + PRD_BUF_DUMMY];
	char cmp_lower_str[64];
	char cmp_upper_str[64];
#endif
	u16 cmp_lower;
	u16 cmp_upper;
	/* */
	int app_read;
	int app_mode;
	u32 app_reg_addr;
	u32 app_reg_size;
	struct lxs_prd_raw_ctrl *app_raw_ctrl;
	/* */
	int enable_swap_app;
	int enable_swap_raw;
	int enable_swap_sys;
	int enable_swap_sd;
	/* */
	int sysfs_flag;
	/* */
	int sysfs_done;
};

enum {
	CMD_TEST_EXIT = 0,
	CMD_TEST_ENTER,
};

/* AIT Algorithm Engine HandShake CMD */
enum {
	PRD_CMD_NONE = 0,
	PRD_CMD_RAW,
	PRD_CMD_BASE,
	PRD_CMD_DELTA,
	PRD_CMD_LABEL,
	PRD_CMD_FILTERED_DELTA,
	PRD_CMD_RESERVED,
	PRD_CMD_DEBUG,
	/* */
	PRD_CMD_RAW_S = 8,
	PRD_CMD_BASE_S,
	PRD_CMD_DELTA_S,
	PRD_CMD_LABEL_S,
	PRD_CMD_DEBUG_S,
	/* */
	PRD_CMD_P_POS_RAW = 13,
	PRD_CMD_P_POS_BASE,
	PRD_CMD_P_POS_DELTA,
	PRD_CMD_P_POS_LABEL,
	PRD_CMD_P_POS_DEBUG,
	/* */
	PRD_CMD_P_RAW = 18,
	PRD_CMD_P_BASE,
	PRD_CMD_P_DELTA,
	PRD_CMD_P_DEBUG,
	/* */
	PRD_CMD_SYSR = 100,
	PRD_CMD_SYSB,
	PRD_CMD_SYSD,
	/* */
	PRD_CMD_P_POS_SYSR = 210,
	PRD_CMD_P_SYSR = 215,
	PRD_CMD_P_SYS_DEBUG = 217,
	/* */
	PRD_CMD_DONT_USE = 0xEE,
	PRD_CMD_WAIT = 0xFF,
};

/* AIT Algorithm Engine HandShake Status */
enum {
	RS_READY	= 0xA0,
	RS_NONE	= 0x05,
	RS_LOG		= 0x77,
	RS_IMAGE	= 0xAA
};

enum {
	PRD_TIME_STR_SZ = 64,
};

enum {
	REPORT_END_RS_NG = 0x05,
	REPORT_END_RS_OK = 0xAA,
};

enum {
	REPORT_OFF = 0,
	REPORT_RAW,
	REPORT_BASE,
	REPORT_DELTA,
	REPORT_LABEL,
	REPORT_DEBUG_BUG,
	/* */
	REPORT_P_POS_RAW,
	REPORT_P_POS_BASE,
	REPORT_P_POS_DELTA,
	REPORT_P_POS_LABEL,
	REPORT_P_POS_DEBUG,
	/* */
	REPORT_P_RAW,
	REPORT_P_BASE,
	REPORT_P_DELTA,
	/* */
	REPORT_SYSR,
	REPORT_SYSB,
	REPORT_SYSD,
	REPORT_P_POS_SYSR,
	REPORT_P_SYSR,
	REPORT_P_SYS_DEBUG,
	/* */
	REPORT_MAX,
};

static const char *prd_app_mode_str[] = {
	[REPORT_OFF]			= "OFF",
	[REPORT_RAW]			= "RAW",
	[REPORT_BASE]			= "BASE",
	[REPORT_LABEL]			= "LABEL",
	[REPORT_DELTA]			= "DELTA",
	[REPORT_DEBUG_BUG]		= "DEBUG_BUF",
	/* */
	[REPORT_P_POS_RAW]		= "P_POS_RAW",
	[REPORT_P_POS_BASE]		= "P_POS_BASE",
	[REPORT_P_POS_DELTA]	= "P_POS_DELTA",
	[REPORT_P_POS_LABEL]	= "P_POS_LABEL",
	[REPORT_P_POS_DEBUG]	= "P_POS_DEBUG",
	/* */
	[REPORT_P_RAW]			= "RAW_P",
	[REPORT_P_BASE]			= "BASE_P",
	[REPORT_P_DELTA]		= "DELTA_P",
	/* */
	[REPORT_SYSR]			= "SYSR",
	[REPORT_SYSB]			= "SYSB",
	[REPORT_SYSD]			= "SYSD",
	[REPORT_P_POS_SYSR]		= "SYSR_P_POS",
	[REPORT_P_SYSR]			= "SYSR_P",
	[REPORT_P_SYS_DEBUG]	= "SYS_P_DEBUG",
};

static const int prd_app_mode_ctrl_id[] = {
	[REPORT_OFF]			= -1,
	[REPORT_RAW]			= PRD_CTRL_IDX_RAW,
	[REPORT_BASE]			= PRD_CTRL_IDX_BASE,
	[REPORT_DELTA]			= PRD_CTRL_IDX_DELTA,
	[REPORT_LABEL]			= PRD_CTRL_IDX_LABEL,
	[REPORT_DEBUG_BUG]		= PRD_CTRL_IDX_DEBUG,
	/* */
	[REPORT_P_POS_RAW]		= PRD_CTRL_IDX_P_POS_RAW,
	[REPORT_P_POS_BASE]		= PRD_CTRL_IDX_P_POS_BASE,
	[REPORT_P_POS_DELTA]	= PRD_CTRL_IDX_P_POS_DELTA,
	[REPORT_P_POS_LABEL]	= PRD_CTRL_IDX_P_POS_LABEL,
	[REPORT_P_POS_DEBUG]	= PRD_CTRL_IDX_P_POS_DEBUG,
	/* */
	[REPORT_P_RAW]			= PRD_CTRL_IDX_P_RAW,
	[REPORT_P_BASE]			= PRD_CTRL_IDX_P_BASE,
	[REPORT_P_DELTA]		= PRD_CTRL_IDX_P_DELTA,
	/* */
	[REPORT_SYSR]			= PRD_CTRL_IDX_SYSR,
	[REPORT_SYSB]			= PRD_CTRL_IDX_SYSB,
	[REPORT_SYSD]			= PRD_CTRL_IDX_SYSD,
	[REPORT_P_POS_SYSR]		= PRD_CTRL_IDX_P_POS_SYSR,
	[REPORT_P_SYSR]			= PRD_CTRL_IDX_P_SYSR,
	[REPORT_P_SYS_DEBUG]	= PRD_CTRL_IDX_P_SYS_DEBUG,
};

enum {
	PRD_DBG_FLAG_PATTERN_FW		= BIT(0),
	/* */
	PRD_DBG_FLAG_RAW_CMP		= BIT(7),
	/* */
	PRD_DBG_FLAG_RAW_LOG_ON		= BIT(16),
	/* */
	PRD_DBG_FLAG_DISABLE		= BIT(30),
};

/* Flexible digit width */
#define DIGIT_RANGE_BASE		4

#define PRD_RAWDATA_MAX			(32767)
#define PRD_RAWDATA_MIN			(-32768)

#define RAWDATA_MAX_DIGIT6		(999999)
#define RAWDATA_MIN_DIGIT6		(-99999)

#define RAWDATA_MAX_DIGIT5		(99999)
#define RAWDATA_MIN_DIGIT5		(-9999)

#define RAWDATA_MAX_DIGIT4		(9999)
#define RAWDATA_MIN_DIGIT4		(-999)

#define LXS_PRD_TAG 		"prd: "
#define LXS_PRD_TAG_ERR 	"prd(E): "
#define LXS_PRD_TAG_WARN	"prd(W): "
#define LXS_PRD_TAG_DBG		"prd(D): "

#define t_prd_info(_prd, fmt, args...)	\
		__t_dev_info(_prd->dev, LXS_PRD_TAG fmt, ##args)

#define t_prd_err(_prd, fmt, args...)	\
		__t_dev_err(_prd->dev, LXS_PRD_TAG_ERR fmt, ##args)

#define t_prd_warn(_prd, fmt, args...)	\
		__t_dev_warn(_prd->dev, LXS_PRD_TAG_WARN fmt, ##args)

#define t_prd_dbg(condition, _prd, fmt, args...)	\
		do {	\
			if (unlikely(t_prd_dbg_mask & (condition)))	\
				__t_dev_info(_prd->dev, LXS_PRD_TAG_DBG fmt, ##args);	\
		} while (0)

#define t_prd_dbg_base(_prd, fmt, args...)	\
		t_prd_dbg(DBG_BASE, _prd, fmt, ##args)

#define t_prd_dbg_trace(_prd, fmt, args...)	\
		t_prd_dbg(DBG_TRACE, _prd, fmt, ##args)

#define lxs_prd_buf_snprintf(_prt_buf, _prt_size, _fmt, _args...) \
		__lxs_snprintf(_prt_buf, PRD_LOG_POOL_SIZE, _prt_size, _fmt, ##_args);

#define lxs_prd_log_buf_snprintf(_log_buf, _prt_size, _fmt, _args...) \
		__lxs_snprintf(_log_buf, PRD_LOG_LINE_SIZE, _prt_size, _fmt, ##_args);

#define lxs_prd_log_info(_prd, _buf, _size)	\
		do {	\
			if (t_prd_dbg_flag & PRD_DBG_FLAG_RAW_LOG_ON) {	\
				if (_size)	t_prd_info(_prd, "%s\n", _buf);	\
			}	\
		} while (0)

#if 1	/* SOMC */
#define SD_RESULT_PASS	1
#define SD_RESULT_FAIL	0

#define SD_RESULT_STR_PASS	"1"
#define SD_RESULT_STR_FAIL	"0"
#else
#define SD_RESULT_PASS	0
#define SD_RESULT_FAIL	1

#define SD_RESULT_STR_PASS	"pass"
#define SD_RESULT_STR_FAIL	"fail"
#endif

#define lxs_snprintf_sd_result(_prt_buf, _prt_size, _item, _ret) \
		lxs_snprintf(_prt_buf, _prt_size, "%s:%s ", _item, (_ret != SD_RESULT_PASS) ? SD_RESULT_STR_FAIL : SD_RESULT_STR_PASS)

#define lxs_prd_log_flush(_prd, _log_buf, _prt_buf, _log_size, _prt_size)	\
		({	int __prt_size = 0;	\
			lxs_prd_log_info(_prd, _log_buf, _log_size);	\
			__prt_size = lxs_prd_buf_snprintf(_prt_buf, _prt_size, "%s\n", _log_buf);	\
			__prt_size;	\
		})

#define lxs_prd_log_end(_prd, _log_buf, _log_size)	\
		({	int __prt_size = 0;	\
			lxs_prd_log_info(_prd, _log_buf, _log_size);	\
			__prt_size = lxs_prd_log_buf_snprintf(_log_buf, _log_size, "\n");	\
			__prt_size;	\
		})

#define APP_REG_FLAG_LOG	BIT(0)
#define APP_REG_FLAG_WR		BIT(4)

#define TS_ATTR_PRD(_name, _show, _store)	\
		TS_ATTR(_name, _show, _store)

/*
 * App I/F : binary stream
 */
#define TS_ATTR_PRD_APP(_name, _idx)	\
static ssize_t prd_show_app_##_name(struct device *dev,	char *buf)	\
{	\
	return prd_show_app_common(dev, buf, REPORT_##_idx);	\
}	\
static TS_ATTR(prd_app_##_name, prd_show_app_##_name, NULL)

/*
 * Standard - PAGE_SIZE(4KB)
 */
#define TS_ATTR_PRD_STD(_name, _idx, _chk)	\
static ssize_t prd_show_##_name(struct device *dev, char *buf)	\
{	\
	return min(prd_show_common(dev, buf, PRD_CTRL_IDX_##_idx), (ssize_t)PAGE_SIZE);	\
}	\
static TS_ATTR(_name, prd_show_##_name, NULL)

#define PRD_BIN_ATTR_SIZE	(PRD_LOG_POOL_SIZE + PRD_BUF_DUMMY)

#define TS_ATTR_PRD_BIN(_name, _idx, _chk)	\
static ssize_t prd_show_##_name(struct device *dev, char *buf)	\
{	\
	return prd_show_common(dev, buf, PRD_CTRL_IDX_##_idx);	\
}	\
static ssize_t prd_show_bin_##_name(struct file *filp, struct kobject *kobj,	\
			struct bin_attribute *bin_attr, \
			char *buf, loff_t off, size_t count)	\
{	\
	struct lxs_ts *ts = container_of(kobj, struct lxs_ts, kobj);	\
	struct lxs_prd_data *prd = (struct lxs_prd_data *)ts->prd;	\
	char *log_bin = prd->log_bin;	\
	int prd_buf_size = bin_attr->size;	\
	if (off == 0) { \
		memset(log_bin, 0, prd_buf_size);	\
		prd_show_##_name(ts->dev, log_bin); \
	}	\
	if ((off + count) > prd_buf_size) { \
		memset(buf, 0, count);	\
	} else {	\
		memcpy(buf, &log_bin[off], count);	\
	}	\
	return (ssize_t)count;	\
}	\
static TS_BIN_ATTR(_name, prd_show_bin_##_name, NULL, PRD_BIN_ATTR_SIZE)

#if defined(__PRD_USE_BIG_ATTR)
#define TS_ATTR_PRD_RAW		TS_ATTR_PRD_BIN
#else
#define TS_ATTR_PRD_RAW		TS_ATTR_PRD_STD
#endif

/*---------------------------------------------------------------------------*/

#define RUN_DATA_WORD_LEN	10

enum {
	PRD_SD_SUB_IDX_NP_RAW = 0,
	PRD_SD_SUB_IDX_NP_RAW_SELF,
	PRD_SD_SUB_IDX_NP_JITTER,
	PRD_SD_SUB_IDX_NP_JITTER_SELF,
	PRD_SD_SUB_IDX_NP_CALB,
	PRD_SD_SUB_IDX_NP_SYNC,
	PRD_SD_SUB_IDX_NP_IRQ,
	PRD_SD_SUB_IDX_NP_RST,
	/* */
	PRD_SD_SUB_IDX_LP_OPEN_CMP,
	PRD_SD_SUB_IDX_LP_SHORT_CMP,
	PRD_SD_SUB_IDX_LP_OPEN,
	PRD_SD_SUB_IDX_LP_GAP_X,
	PRD_SD_SUB_IDX_LP_GAP_Y,
	PRD_SD_SUB_IDX_LP_GAP_A,
	PRD_SD_SUB_IDX_LP_SHORT_GND,
	PRD_SD_SUB_IDX_LP_SHORT_TRX,
	PRD_SD_SUB_IDX_LP_SHORT_VDD,
	PRD_SD_SUB_IDX_LP_RAW,
	PRD_SD_SUB_IDX_LP_RAW_SELF,
	PRD_SD_SUB_IDX_LP_JITTER,
	PRD_SD_SUB_IDX_LP_JITTER_SELF,
};

#define TS_ATTR_PRD_SD(_np, _name, _idx, _is_raw)	\
static ssize_t prd_show_##_name(struct device *dev, char *buf)	\
{	\
	return prd_show_sd_sub_common(dev, buf, NULL, (_is_raw) ? PAGE_SIZE : 0, PRD_SD_SUB_IDX_##_idx, _np);	\
}	\
static TS_ATTR_PRD(_name, prd_show_##_name, NULL)

#define TS_ATTR_PRD_SD_OD(_name, _idx)	TS_ATTR_PRD_SD(BIT(1)|1, _name, _idx, 0)
#define TS_ATTR_PRD_SD_NP_MIN(_name, _idx, _is_raw)	TS_ATTR_PRD_SD(BIT(2)|1, _name, _idx, _is_raw)
#define TS_ATTR_PRD_SD_NP_MAX(_name, _idx, _is_raw)	TS_ATTR_PRD_SD(BIT(3)|1, _name, _idx, _is_raw)
#define TS_ATTR_PRD_SD_NP_MMD(_name, _idx, _is_raw)	TS_ATTR_PRD_SD(BIT(4)|1, _name, _idx, _is_raw)
#define TS_ATTR_PRD_SD_NP(_name, _idx, _is_raw)	TS_ATTR_PRD_SD(1, _name, _idx, _is_raw)
#define TS_ATTR_PRD_SD_LP_MIN(_name, _idx, _is_raw)	TS_ATTR_PRD_SD(BIT(2), _name, _idx, _is_raw)
#define TS_ATTR_PRD_SD_LP_MAX(_name, _idx, _is_raw)	TS_ATTR_PRD_SD(BIT(3), _name, _idx, _is_raw)
#define TS_ATTR_PRD_SD_LP_MMD(_name, _idx, _is_raw)	TS_ATTR_PRD_SD(BIT(4), _name, _idx, _is_raw)
#define TS_ATTR_PRD_SD_LP_CMP(_name, _idx, _is_raw)	TS_ATTR_PRD_SD(BIT(1), _name, _idx, _is_raw)
#define TS_ATTR_PRD_SD_LP(_name, _idx, _is_raw)	TS_ATTR_PRD_SD(0, _name, _idx, _is_raw)

#endif /* __LXS_TS_HAL_PRD_H */


