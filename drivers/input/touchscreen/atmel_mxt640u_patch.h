/*
 * Atmel maXTouch Touchscreen patch driver
 *
 * Copyright (C) 2013 Atmel Corporation
 * Copyright (C) 2013 LG Electronics, Inc.
 * Copyright (C) 2017, 2018 Sony Mobile Communications Inc.
 * Author: <WX-BSP-TS@lge.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include "atmel_mxt640u.h"

#ifndef __LINUX_ATMEL_PATCH_TS_H__
#define __LINUX_ATMEL_PATCH_TS_H__

#define TIME_WRAP_AROUND(x, y)		(((y)>(x)) ? (y)-(x) : (0-(x))+(y))

#define MXT_PATCH_MAGIC		0x52296416
#define MXT_PATCH_VERSION		1
#define MXT_PATCH_MAX_STAGE		255
#define MXT_PATCH_MAX_TLINE		255
#define MXT_PATCH_MAX_TRIG		255
#define MXT_PATCH_MAX_ITEM		255
#define MXT_PATCH_MAX_TYPE		255
#define MXT_PATCH_MAX_CON		255
#define MXT_PATCH_MAX_EVENT		255
#define MXT_PATCH_MAX_MSG_SIZE		10
#define MXT_PATCH_T71_DATA_MAX		200
#define MXT_PATCH_T71_PTN_OPT		1
#define MXT_PATCH_T71_PTN_CAL		2
#define MXT_PATCH_LOCK_CHECK		1
#define MXT_PATCH_SUPP_CHECK		1
#define MXT_PATCH_T9STATUS_CHK		1
#define MXT_PATCH_START_FROM_NORMAL	1
#define MXT_PATCH_STAGE_RESET		1
#define MXT_PATCH_USER_DATA		1
#define MXT_PATCH_USER_DATA_MAX	32
#define MXT_PATCH_TRIGGER_MOD		1

#define MXT_XML_CON_NONE		"0"
#define MXT_XML_CON_EQUAL		"="
#define MXT_XML_CON_BELOW		"<"
#define MXT_XML_CON_ABOVE		">"
#define MXT_XML_CON_PLUS		"+"
#define MXT_XML_CON_MINUS		"-"
#define MXT_XML_CON_MUL		"*"
#define MXT_XML_CON_DIV		"/"
#define MXT_XML_CON_MASK		"&"
#define MXT_XML_SRC_NONE		"0"
#define MXT_XML_SRC_CHRG		"TA"
#define MXT_XML_SRC_FCNT		"FCNT"
#define MXT_XML_SRC_AREA		"AREA"
#define MXT_XML_SRC_AMP		"AMP"
#define MXT_XML_SRC_SUM		"SUM"
#define MXT_XML_SRC_TCH		"TCH"
#define MXT_XML_SRC_ATCH		"ATCH"
#define MXT_XML_SRC_KCNT		"KCNT"
#define MXT_XML_SRC_KVAL		"KVAL"
#define MXT_XML_SRC_T9STATUS		"T9STATUS"
#define MXT_XML_SRC_USER1		"USR1"
#define MXT_XML_SRC_USER2 		"USR2"
#define MXT_XML_SRC_USER3		"USR3"
#define MXT_XML_SRC_USER4 		"USR4"
#define MXT_XML_SRC_USER5 		"USR5"
#define MXT_XML_ACT_NONE		"0"
#define MXT_XML_ACT_CAL		"CAL"
#define MXT_XML_ACT_EXTMR		"EXTMR"

#define MXT_SUPPRESS_MSG_MASK		(1 << 1)
#define MXT_AMPLITUDE_MSG_MASK		(1 << 2)
#define MXT_VECTOR_MSG_MASK		(1 << 3)
#define MXT_MOVE_MSG_MASK		(1 << 4)
#define MXT_RELEASE_MSG_MASK		(1 << 5)
#define MXT_PRESS_MSG_MASK		(1 << 6)
#define MXT_DETECT_MSG_MASK		(1 << 7)

#define MXT_MSG_T15_STATUS		0x00
#define MXT_MSG_T15_KEYSTATE		0x01
#define MXT_MSGB_T15_DETECT		0x80


/* Message type of T100 object */
#define MXT_T100_SCREEN_MSG_FIRST_RPT_ID	0
#define MXT_T100_SCREEN_MSG_SECOND_RPT_ID	1
#define MXT_T100_SCREEN_MESSAGE_NUM_RPT_ID	2

/* Event Types of T100 object */
#define MXT_T100_DETECT_MSG_MASK			7

/* Tool types of T100 object */
#define MXT_T100_TYPE_RESERVED				0
#define MXT_T100_TYPE_PATCH_FINGER			1
#define MXT_T100_TYPE_PASSIVE_STYLUS		2
#define MXT_T100_TYPE_ACTIVE_STYLUS			3
#define MXT_T100_TYPE_HOVERING_FINGER		4
#define MXT_T100_TYPE_GLOVE_TOUCH			5
#define MXT_T100_TYPE_LARGE_TOUCH			6 //20131227

enum{
	MXT_PATCH_CON_NONE = 0,
	MXT_PATCH_CON_EQUAL,
	MXT_PATCH_CON_BELOW,
	MXT_PATCH_CON_ABOVE,
	MXT_PATCH_CON_PLUS,
	MXT_PATCH_CON_MINUS,
	MXT_PATCH_CON_MUL,
	MXT_PATCH_CON_DIV,
	MXT_PATCH_CON_MASK,
	//...
	MXT_PATCH_CON_END
};

enum {
	MXT_PATCH_ITEM_NONE = 0,
	MXT_PATCH_ITEM_CHARG,
	MXT_PATCH_ITEM_FCNT,
	MXT_PATCH_ITEM_AREA,
	MXT_PATCH_ITEM_AMP,
	MXT_PATCH_ITEM_SUM,
	MXT_PATCH_ITEM_TCH,
	MXT_PATCH_ITEM_ATCH,
	MXT_PATCH_ITEM_KCNT,
	MXT_PATCH_ITEM_KVAL,
	MXT_PATCH_ITEM_T9STATUS,
	MXT_PATCH_ITEM_USER1,
	MXT_PATCH_ITEM_USER2,
	MXT_PATCH_ITEM_USER3,
	MXT_PATCH_ITEM_USER4,
	MXT_PATCH_ITEM_USER5,
	MXT_PATCH_ITEM_USER6,
	//...
	MXT_PATCH_ITEM_END
};

enum {
	MXT_PATCH_ACTION_NONE = 0,
	MXT_PATCH_ACTION_CAL,
	MXT_PATCH_ACTION_EXTEND_TIMER,
	MXT_PATCH_ACTION_GOTO_STAGE,
	MXT_PATCH_ACTION_CHANGE_START,
	//...
	MXT_PATCH_ACTION_END
};

struct patch_header{ // 32b
	u32	magic;
	u32	size;
	u32	date;
	u16	version;
	u8	option;
	u8	debug;
	u8	timer_id;
	u8	stage_cnt;
	u8	trigger_cnt;
	u8	event_cnt;
	u8	reserved[12];
};

struct stage_def{	// 8b
	u8	stage_id;
	u8	option;
	u16	stage_period;
	u8	cfg_cnt;
	u8	test_cnt;
	u16	reset_period;
};

struct stage_cfg{	// 4b
	u8	obj_type;
	u8	option;
	u8	offset;
	u8	val;
};

struct test_line{	// 12b
	u8	test_id;
	u8	item_cnt;
	u8	cfg_cnt;
	u8	act_id;
	u16	act_val;
	u16	option;
	u16	check_cnt;
	u8	reserved[2];
};

struct action_cfg{	// 4b
	u8	obj_type;
	u8	option;
	u8	offset;
	u8	val;
};

struct item_val{	// 4b
	u8	val_id;
	u8	val_eq;
	u16	val;
};

struct test_item{	// 8b
	u8	src_id;
	u8	cond;
	u8	reserved[2];
	struct item_val ival;
};

// Message Trigger
struct trigger{		// 12b
	u8	tid;
	u8	option;
	u8	object;
	u8	index;
	u8	match_cnt;
	u8	cfg_cnt;
	u8	reserved[3];
	u8	act_id;
	u16	act_val;
};

struct match{		//8b
	u8	offset;
	u8	cond;
	u16	mask;
	u8	reserved[2];
	u16	val;
};

struct trigger_cfg{	// 4b
	u8	obj_type;
	u8	reserved;
	u8	offset;
	u8	val;
};

// Event
struct user_event{	// 8b
	u8	eid;
	u8	option;
	u8	cfg_cnt;
	u8	reserved[5];
};

struct event_cfg{	// 4b
	u8	obj_type;
	u8	reserved;
	u8	offset;
	u8	val;
};

struct test_src{
	int	charger;
	int	finger_cnt;
	int	area;
	int area2;
	int	amp;
	int	sum_size;
	int	tch_ch;
	int	atch_ch;
	int	key_cnt;
	int	key_val;
	int	status;
	int	user1;
	int	user2;
	int	user3;
	int	user4;
	int	user5;
	int	user6;
};

struct touch_pos{
	u8	tcount[MXT_MAX_FINGER];
	u16	initx[MXT_MAX_FINGER];
	u16	inity[MXT_MAX_FINGER];
	u16	oldx[MXT_MAX_FINGER];
	u16	oldy[MXT_MAX_FINGER];
	u8	locked_id;
	u8	moved_cnt;
	u8	option;
	u8	cal_enable;
	u8	reset_cnt;
	u8	distance;
	u8	maxdiff;
	u8	locked_cnt;
	u8	jitter;
	u8	area;
	u8	amp;
	u8	sum_size_t57;
	u8	tch_count_t57;
	u8	atch_count_t57;
	u8	amp_2finger_min;
	u8	area_2finger_min;
	u8	sum_size_t57_2finger_min;
	u8	tch_count_t57_2finger_min;
	u8	atch_count_t57_2finger_min;
	u8	amp_2finger_max;
	u8	area_2finger_max;
	u8	sum_size_t57_2finger_max;
	u8	tch_count_t57_2finger_max;
	u8	atch_count_t57_2finger_max;
	u8	amp_3finger_min;
	u8	area_3finger_min;
	u8	sum_size_t57_3finger_min;
	u8	tch_count_t57_3finger_min;
	u8	atch_count_t57_3finger_min;
	u8	amp_3finger_max;
	u8	area_3finger_max;
	u8	sum_size_t57_3finger_max;
	u8	tch_count_t57_3finger_max;
	u8	atch_count_t57_3finger_max;
	u8	amp_mfinger_min;
	u8	area_mfinger_min;
	u8	sum_size_t57_mfinger_min;
	u8	tch_count_t57_mfinger_min;
	u8	atch_count_t57_mfinger_min;
	u8	amp_mfinger_max;
	u8	area_mfinger_max;
	u8	sum_size_t57_mfinger_max;
	u8	tch_count_t57_mfinger_max;
	u8	atch_count_t57_mfinger_max;
	u8	xlo_limit;
	u16	xhi_limit;
	u8	ylo_limit;
	u16	yhi_limit;
};

struct touch_supp{
	u32	old_time;
	u8	repeat_cnt;
	u8	time_gap;
	u8	repeat_max;
};

#if 1
#define __mxt_patch_debug(_data, ...)	if (data->patch.debug) \
	TOUCH_PATCH_INFO_MSG(__VA_ARGS__);

#define __mxt_patch_ddebug(_data, ...)	if (data->patch.debug > 1) \
	TOUCH_PATCH_INFO_MSG( __VA_ARGS__);
#else
#define __mxt_patch_debug(_data, ...)	if (data->patch.debug) \
	dev_info(&(_data)->client->dev, __VA_ARGS__);

#define __mxt_patch_ddebug(_data, ...)	if (data->patch.debug > 1) \
	dev_info(&(_data)->client->dev, __VA_ARGS__);
#endif

/* Function Define */
int mxt_patch_init(struct mxt_data *data, u8* ppatch);
int mxt_patch_event(struct mxt_data *data, u8 event_id);
void mxt_patch_message(struct mxt_data *data, struct mxt_message *message);
void mxt_patch_dump_source(struct mxt_data *data, bool do_action);
int mxt_patch_run_stage(struct mxt_data *data);

#endif // __LINUX_ATMEL_PATCH_TS_H__
