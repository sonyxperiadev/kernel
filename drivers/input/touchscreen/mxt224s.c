/*
 *  Copyright (C) 2012, Samsung Electronics Co. Ltd. All Rights Reserved.
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

#define DEBUG
/*#define VERBOSE_DEBUG*/

#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
#define ATMEL_DEBUG
#define MXT_DEBUG
#endif

/*#define TOUCH_BOOSTER*/
#define CHECK_ANTITOUCH
#define SHAPETOUCH

#include <asm/unaligned.h>
#include <linux/delay.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/firmware.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/input/mxt224s.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#if defined(SEC_FAC_TSP)
#include <linux/list.h>
#endif

#if defined(TOUCH_BOOSTER)
#include <linux/mfd/dbx500-prcmu.h>
#endif

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timer.h>
#ifdef CONFIG_USB_SWITCHER
#include <linux/usb_switcher.h>
#include <linux/input/ab8505_micro_usb_iddet.h>
#endif

#define OBJ_TBL_ELEMENT_SIZE		6
#define OBJ_TBL_START_ADDR		7

#define CMD_RESET_OFFSET		0
#define CMD_BACKUP_OFFSET		1
#define CMD_CALIBRATE_OFFSET		2
#define CMD_REPORTATLL_OFFSET		3
#define CMD_DEBUG_CTRL_OFFSET		4
#define CMD_DIAGNOSTIC_OFFSET		5

#define MXT_T7_IDLE_ACQ_INT		0
#define MXT_T7_ACT_ACQ_INT		1

/* Slave addresses */
#define MXT_APP_LOW			0x4a
#define MXT_APP_HIGH			0x4b

#define MXT_BOOT_LOW			0x24
#define MXT_BOOT_HIGH			0x25

#define MXT_BOOT_VALUE			0xa5
#define MXT_BACKUP_VALUE		0x55

#define MXT_CHECKSUM_FIELD_SIZE		3

/* Bootloader mode status */
#define MXT_WAITING_BOOTLOAD_CMD	0xc0
#define MXT_WAITING_FRAME_DATA		0x80
#define MXT_FRAME_CRC_CHECK		0x02
#define MXT_FRAME_CRC_FAIL		0x03
#define MXT_FRAME_CRC_PASS		0x04
#define MXT_APP_CRC_FAIL		0x40
#define MXT_BOOT_STATUS_MASK		0x3f

/* Bootloader ID */
#define MXT_BOOT_EXTENDED_ID		0x20
#define MXT_BOOT_ID_MASK		0x1f

/* Command to unlock bootloader */
#define MXT_UNLOCK_CMD_MSB		0xaa
#define MXT_UNLOCK_CMD_LSB		0xdc

/* for command processor msg*/
#define MXT_CMD_PROC_RESET		0x80
#define MXT_CMD_PROC_OFL		0x40
#define MXT_CMD_PROC_SIGERR		0x20
#define MXT_CMD_PROC_CAL		0x10
#define MXT_CMD_PROC_CFGERR		0x08
#define MXT_CMD_PROC_COMSERR		0x04

/* for multi touchscreen msg*/
#define MXT_MT_DETECT			0x80
#define MXT_MT_PRESS			0x40
#define MXT_MT_RELEASE			0x20
#define MXT_MT_MOVE			0x10
#define MXT_MT_VECTOR			0x08
#define MXT_MT_AMP			0x04
#define MXT_MT_SUPPRESS			0x02
#define MXT_MT_UNGRIP			0x01

/* cmds on  diagnostic field*/
#define MXT_DIAG_PAGE_UP		0x01
#define MXT_DIAG_PAGE_DOWN		0x02
#define MXT_DIAG_DELTA_MODE		0x10
#define MXT_DIAG_REFERENCE_MODE		0x11
#define MXT_DIAG_CTE_MODE		0x31
#define MXT_DIAG_IDENTIFICATION_MODE	0x80
#define MXT_DIAG_TOCH_THRESHOLD_MODE	0xF4

#define MXT_DIAG_MODE_MASK		0xFC
#define MXT_DIAG_MODE			0
#define MXT_DIAG_PAGE			1
#define MXT_DIAG_DATA			2

/* Firmware name */
#define MXT_FW_NAME			"mXT1664S.fw"
#define MXT_MAX_FW_PATH			255
#define MXT_CONFIG_VERSION_LENGTH	30

/* Firmware version */
#define MXT_FIRM_VERSION		0x10
#define MXT_FIRM_BUILD			0xAA

/* Touchscreen configuration infomation */
#define MXT_FW_MAGIC			0x4D3C2B1A

#define MXT_BYTE_PER_NODE		2
#define MXT_NODE_PER_PAGE		64
#define MXT_PAGE_SIZE			128

#if defined(CHECK_ANTITOUCH)
#define MXT_T61_TIMER_ONESHOT		0
#define MXT_T61_TIMER_REPEAT		1
#define MXT_T61_TIMER_CMD_START		1
#define MXT_T61_TIMER_CMD_STOP		2
#endif

/* Spec offset */
#define REFERENCE_OFFSET		16384
#define DELTA_OFFSET			0

enum {
	IN_BUILT_IN = 0,
	IN_BOARD,
	IN_FS,
	IN_HEADER,
};

enum {
	NORMAL_CFG = 0,
	TA_CFG,
};

enum {
	NORMAL_MODE = 0,
	TA_MODE,
};

enum {
	FALSE = 0,
	TRUE,
};

enum ab8505_usb_link_status {
	USB_LINK_NOT_CONFIGURED_8505 = 0,
	USB_LINK_STD_HOST_NC_8505,
	USB_LINK_STD_HOST_C_NS_8505,
	USB_LINK_STD_HOST_C_S_8505,
	USB_LINK_CDP_8505,
	USB_LINK_RESERVED0_8505,
	USB_LINK_RESERVED1_8505,
	USB_LINK_DEDICATED_CHG_8505,
	USB_LINK_ACA_RID_A_8505,
	USB_LINK_ACA_RID_B_8505,
	USB_LINK_ACA_RID_C_NM_8505,
	USB_LINK_RESERVED2_8505,
	USB_LINK_RESERVED3_8505,
	USB_LINK_HM_IDGND_8505,
	USB_LINK_CHARGERPORT_NOT_OK_8505,
	USB_LINK_CHARGER_DM_HIGH_8505,
	USB_LINK_PHYEN_NO_VBUS_NO_IDGND_8505,
	USB_LINK_STD_UPSTREAM_NO_IDGNG_NO_VBUS_8505,
	USB_LINK_STD_UPSTREAM_8505,
	USB_LINK_CHARGER_SE1_8505,
	USB_LINK_CARKIT_CHGR_1_8505,
	USB_LINK_CARKIT_CHGR_2_8505,
	USB_LINK_ACA_DOCK_CHGR_8505,
	USB_LINK_SAMSUNG_BOOT_CBL_PHY_EN_8505,
	USB_LINK_SAMSUNG_BOOT_CBL_PHY_DISB_8505,
	USB_LINK_SAMSUNG_UART_CBL_PHY_EN_8505,
	USB_LINK_SAMSUNG_UART_CBL_PHY_DISB_8505,
	USB_LINK_MOTOROLA_FACTORY_CBL_PHY_EN_8505,
};

struct mxt_obj {
	u8 type;
	u16 start_addr;
	u8 size;
	u8 instances;
	u8 num_report_ids;
} __packed;

struct mxt_info_block {
	u8 family_id;
	u8 variant_id;
	u8 ver;
	u8 build;
	u8 matrix_xsize;
	u8 matrix_ysize;
	u8 obj_num;
};

struct mxt_finger_info {
	s16 x;
	s16 y;
#if defined(SHAPETOUCH)
	u16 component;
#endif
	u8 state;
};

struct mxt_message {
	u8 report_id;
	u8 data[7];
	u8 checksum;
};

#if defined(SEC_FAC_TSP)

#define TSP_CMD_STR_LEN		32
#define TSP_CMD_RESULT_STR_LEN	512
#define TSP_CMD_PARAM_NUM	8

enum {
	WAITING = 0,
	RUNNING,
	OK,
	FAIL,
	NOT_APPLICABLE,
};

struct tsp_cmd {
	struct list_head	list;
	const char		*cmd_name;
	void			(*cmd_func)(void *device_data);
};

struct sec_fac_tsp {
	struct device		*dummy_dev;
	struct list_head	cmd_list_head;
	u8			cmd_state;
	char			cmd[TSP_CMD_STR_LEN];
	int			cmd_param[TSP_CMD_PARAM_NUM];
	char			cmd_buff[TSP_CMD_RESULT_STR_LEN];
	char			cmd_result[TSP_CMD_RESULT_STR_LEN];
	struct mutex		cmd_lock;
	bool			cmd_is_running;
	u8			num_of_x;
	u8			num_of_y;
	u16			num_of_node;
	u16			*reference;
	s16			*delta;
	u16			reference_max;
	u16			reference_min;
	s16			delta_max;
	s16			delta_min;
};
#endif

struct mxt_fw_image {
	__le32	magic_code;
	__le32	hdr_len;
	__le32	cfg_len;
	__le32	fw_len;
	__le32	cfg_crc;
	u8	fw_ver;
	u8	build_ver;
	u8	data[0];
} __packed;

struct mxt_cfg_image {
	__le32	hdr_len;
	__le32	num_of_cfg;
	__le32	cfg_len;
	__le32	cfg_crc;
	u8	data[0];
} __packed;

struct mxt_cfg_data {
	u8 type;
	u8 instance;
	u8 size;
	u8 data[0];
} __packed;

struct mxt_fw_info {
	u8 fw_ver;
	u8 build_ver;
	u32 hdr_len;
	u32 cfg_len;
	u32 fw_len;
	u32 cfg_crc;
	const u8 *cfg_raw_data;	/* start address of configuration data */
	const u8 *fw_raw_data;	/* start address of firmware data */
	struct	mxt_data *data;
};

struct mxt_data {
	struct i2c_client		*client;
	struct i2c_client		*client_boot;
	struct input_dev		*input_dev;
	struct mutex			lock;
	const struct mxt_platform_data	*pdata;
	struct mxt_info_block		info;
	struct mxt_obj			*obj_tbl;
	struct mxt_report_id_map	*rid_map;
	struct mxt_fw_image		*fw_image;
	struct mxt_cfg_image		*cfg_image;
	struct mxt_cfg_image		*ta_cfg_image;
	struct delayed_work		wq;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend		early_suspend;
#endif
#ifdef CONFIG_USB_SWITCHER
	struct notifier_block		nb;
#endif
	struct timer_list		ta_status_timer;

	u8				max_report_id;
	u8				finger_report_id;
	u16				msg_proc;
	u16				msg_obj_size;
	u16				cmd_proc;

	u32				x_dropbit:2;
	u32				y_dropbit:2;
	u32				finger_mask;
	int				num_of_fingers;
	bool				enabled;
	u8				dev_mode;
	u8				usb_notify;
	u8				idle_cycle_time;
	u8				actv_cycle_time;
	u8				*finger_cnt;
#if defined(CHECK_ANTITOUCH)
	bool				autocal_enabled;
	bool				check_antitouch;
	bool				check_timer;
	u8                              check_after_wakeup;
#endif
#if defined(SHAPETOUCH)
	u16				sumsize;
#endif

#if defined(ATMEL_DEBUG)
	int				driver_paused;
	int				debug_enabled;
	u16				last_read_addr;
#endif
#if defined(MXT_DEBUG)
	bool				g_debug_switch;
#endif
#if defined(SEC_FAC_TSP)
	struct sec_fac_tsp		*fdata;
#endif
	struct mxt_finger_info		finger[];
};

struct mxt_report_id_map {
	u8 obj_type;
	u8 instance;
	int (*handler)(struct mxt_data *, struct mxt_message *);
};

struct msg_handler_struct {
	u8 obj_type;
	int (*handler)(struct mxt_data *, struct mxt_message *);
};

static int mxt_command_proc_msg_handler(struct mxt_data *,
					struct mxt_message *);
static int mxt_multi_touchscreen_msg_handler(struct mxt_data *,
					     struct mxt_message *);
static int mxt_touch_suppression_msg_handler(struct mxt_data *,
					     struct mxt_message *);
static int mxt_extra_touchscreen_msg_handler(struct mxt_data *,
					     struct mxt_message *);
static int mxt_timer_msg_handler(struct mxt_data *, struct mxt_message *);
static int mxt_noise_suppression_msg_handler(struct mxt_data *,
					     struct mxt_message *);

struct msg_handler_struct msg_handler[] = {
	{GEN_COMMANDPROCESSOR_T6, mxt_command_proc_msg_handler},
	{TOUCH_MULTITOUCHSCREEN_T9, mxt_multi_touchscreen_msg_handler},
	{TOUCH_KEYARRAY_T15, NULL},
	{SPT_GPIOPWM_T19, NULL},
	{TOUCH_PROXIMITY_T23, NULL},
	{SPT_SELFTEST_T25, NULL},
	{PROCI_TOUCHSUPPRESSION_T42, mxt_touch_suppression_msg_handler},
	{SPT_CTECONFIG_T46, NULL},
	{PROCI_SHIELDLESS_T56, NULL},
	{PROCI_EXTRATOUCHSCREENDATA_T57, mxt_extra_touchscreen_msg_handler},
	{SPT_TIMER_T61, mxt_timer_msg_handler},
	{PROCG_NOISESUPPRESSION_T62, mxt_noise_suppression_msg_handler},
};

#if defined(SEC_FAC_TSP)
static void fw_update(void *device_data);
static void get_fw_ver_bin(void *device_data);
static void get_fw_ver_ic(void *device_data);
static void get_config_ver(void *device_data);
static void get_threshold(void *device_data);
static void module_off_master(void *device_data);
static void module_on_master(void *device_data);
static void get_chip_vendor(void *device_data);
static void get_chip_name(void *device_data);
static void get_x_num(void *device_data);
static void get_y_num(void *device_data);
static void run_reference_read(void *device_data);
static void get_reference(void *device_data);
static void run_delta_read(void *device_data);
static void get_delta(void *device_data);
static void not_support_cmd(void *device_data);

static struct tsp_cmd tsp_cmds[] = {
	{TSP_CMD("fw_update", fw_update),},
	{TSP_CMD("get_fw_ver_bin", get_fw_ver_bin),},
	{TSP_CMD("get_fw_ver_ic", get_fw_ver_ic),},
	{TSP_CMD("get_config_ver", get_config_ver),},
	{TSP_CMD("get_threshold", get_threshold),},
	{TSP_CMD("module_off_master", module_off_master),},
	{TSP_CMD("module_on_master", module_on_master),},
	{TSP_CMD("module_off_slave", not_support_cmd),},
	{TSP_CMD("module_on_slave", not_support_cmd),},
	{TSP_CMD("get_chip_vendor", get_chip_vendor),},
	{TSP_CMD("get_chip_name", get_chip_name),},
	{TSP_CMD("get_x_num", get_x_num),},
	{TSP_CMD("get_y_num", get_y_num),},
	{TSP_CMD("run_reference_read", run_reference_read),},
	{TSP_CMD("get_reference", get_reference),},
	{TSP_CMD("run_delta_read", run_delta_read),},
	{TSP_CMD("get_delta", get_delta),},
	{TSP_CMD("not_support_cmd", not_support_cmd),},
};
#endif

#if defined(SEC_FAC_TSP)
struct device *sec_touchscreen_dev;
#endif

#define READ_FW_FROM_HEADER 1
static u8 fw_latest[] = {0x11, 0xaa};	/* version, build_version */
u8 fw_data[] = {
#if READ_FW_FROM_HEADER
	#include "mxt224s__APP_v1_1_AA_.h"
#endif
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mxt_early_suspend(struct early_suspend *);
static void mxt_late_resume(struct early_suspend *);
#endif

#ifdef CONFIG_USB_SWITCHER
extern int micro_usb_register_usb_notifier(struct notifier_block *nb);
extern int use_ab8505_iddet;
#endif

static int mxt_read_mem(struct mxt_data *data, u16 reg, u8 len, u8 *buf)
{
	struct i2c_client *client = data->client;
	u16 le_reg = cpu_to_le16(reg);
	struct i2c_msg msg[] = {
		{
			.addr = client->addr,
			.flags = 0,
			.len = 2,
			.buf = (u8 *)&le_reg,
		},
		{
			.addr = client->addr,
			.flags = I2C_M_RD,
			.len = len,
			.buf = buf,
		},
	};
	int ret;

	ret = i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg));
	if (ret != ARRAY_SIZE(msg)) {
		dev_err(&client->dev, "fail to %s 0x%X[%d] (%d)\n", __func__,
			le_reg, len, ret);
		return -EIO;
	}

	return ret;
}

static int mxt_write_mem(struct mxt_data *data, u16 reg, u8 len, const u8 *buf)
{
	struct i2c_client *client = data->client;
	u8 tmp[len + 2];
	int ret;

	put_unaligned_le16(cpu_to_le16(reg), tmp);
	memcpy(tmp + 2, buf, len);

	ret = i2c_master_send(client, tmp, ARRAY_SIZE(tmp));
	if (ret != ARRAY_SIZE(tmp)) {
		dev_err(&client->dev, "fail to %s 0x%X[%d] (%d)\n", __func__,
			cpu_to_le16(reg), len, ret);
		return -EIO;
	}

	return ret;
}

static int mxt_fw_write(struct mxt_data *data,
				const u8 *frame_data, unsigned int frame_size)
{
	struct i2c_client *client = data->client;
	struct i2c_client *client_boot = data->client_boot;
	int ret;

	ret = i2c_master_send(client_boot, frame_data, frame_size);
	if (ret != frame_size) {
		dev_err(&client->dev, "%s: i2c send failed\n", __func__);
		return -EIO;
	}

	return 0;
}

struct mxt_obj *mxt_get_obj(struct mxt_data *data, u8 type)
{
	struct i2c_client *client = data->client;
	struct mxt_obj *obj;
	int i;

	if (!data->obj_tbl) {
		dev_err(&client->dev, "no obj table\n");
		return NULL;
	}

	if (!data->info.obj_num) {
		dev_err(&client->dev, "no objcect table entyr\n");
		return NULL;
	}

	for (i = 0; i < data->info.obj_num; i++) {
		obj = data->obj_tbl + i;
		if (obj->type == type)
			return obj;
	}
	dev_err(&client->dev, "invalid obj type (%d)\n", type);

	return NULL;
}

int mxt_read_obj(struct mxt_data *data, u8 type, u8 offset, u8 *val)
{
	struct mxt_obj *obj;
	u16 reg;

	obj = mxt_get_obj(data, type);
	if (!obj)
		return -EINVAL;

	reg = obj->start_addr;

	return mxt_read_mem(data, reg + offset, 1, val);
}

static int mxt_write_obj(struct mxt_data *data,
				 u8 type, u8 offset, u8 val)
{
	struct mxt_obj *obj;
	u16 reg;

	obj = mxt_get_obj(data, type);
	if (!obj)
		return -EINVAL;

	reg = obj->start_addr;
	return mxt_write_mem(data, reg + offset, 1, &val);
}

static int (*mxt_get_handler(struct mxt_data *data, u8 obj_type))
			    (struct mxt_data *, struct mxt_message *)
{
	struct i2c_client *client = data->client;
	int i;

	for (i = 0; i < ARRAY_SIZE(msg_handler); i++) {
		if (obj_type == msg_handler[i].obj_type)
			return msg_handler[i].handler;
	}
	dev_err(&client->dev, "no handler for obj %d\n", obj_type);
	return NULL;
}

static int mxt_calibrate_chip(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	int ret;
	u8 buf = 1;

	/* send calibration command */
	ret = mxt_write_mem(data, data->cmd_proc + CMD_CALIBRATE_OFFSET, 1,
			    &buf);
	if (ret < 0) {
		dev_err(&client->dev, "fail to send calibration cmd\n");
		return ret;
	}

	dev_info(&client->dev, "cmd: calibration\n");

	return 0;
}

static struct mxt_cfg_data *mxt_get_cfg(struct mxt_data *data, u8 type, u8 mode)
{
	struct i2c_client *client = data->client;
	struct mxt_cfg_image *cfg_image;
	struct mxt_cfg_data *cfg;
	u32 offset;
	int i;

	switch (mode) {

	case TA_MODE:
		cfg_image = data->ta_cfg_image;
		break;
	case NORMAL_MODE:
		cfg_image = data->cfg_image;
		break;
	default:
		dev_err(&client->dev, "%s: invalid mode (%d)\n", __func__,
			mode);
	}

	offset = cfg_image->hdr_len;
	for (i = 0; i < cfg_image->num_of_cfg; i++) {
		cfg = (struct mxt_cfg_data *)((u8 *)cfg_image + offset);
		if (cfg->type == type)
			return cfg;
		offset += sizeof(struct mxt_cfg_data) + cfg->size;
	}

	dev_err(&client->dev, "%s: no cfg type %d on %d\n", __func__, type,
		 mode);
	return NULL;
}

#if defined(CHECK_ANTITOUCH)
/*
 * set the timer on T61 object
 * cmd: timer command
 * mode: timer mode
 * period: setting time in ms
 */
static int mxt_t61_timer_set(struct mxt_data *data, u8 cmd, u8 mode, u16 period)
{
	struct i2c_client *client = data->client;
	struct mxt_obj *obj;
	int ret;
	u8 buf[] = {3, cmd, mode, period & 0xFF, (period >> 8) & 0xFF};

	print_hex_dump(KERN_INFO, "T61 msg:", DUMP_PREFIX_NONE, 16, 1, buf,
		       ARRAY_SIZE(buf), false);

	obj = mxt_get_obj(data, SPT_TIMER_T61);
	if (!obj) {
		dev_err(&client->dev, "%s: fail to write obj T%d\n",
			__func__, SPT_TIMER_T61);
		return -EINVAL;
	}

	ret = mxt_write_mem(data, obj->start_addr, ARRAY_SIZE(buf), buf);
	if (ret < 0) {
		dev_err(&client->dev, "%s: fail to write T%d (%d)\n", __func__,
			obj->type, ret);
		return ret;
	}

	dev_info(&data->client->dev, "%s: timer %sabled %d\n",
		 __func__, (period > 0) ? "en" : "dis", period);

	return 0;
}

static int mxt_t8_autocal_set(struct mxt_data *data, u8 time)
{
	struct i2c_client *client = data->client;
	struct mxt_obj *obj;
	int ret;

	obj = mxt_get_obj(data, GEN_ACQUISITIONCONFIG_T8);
	if (!obj) {
		dev_err(&client->dev, "%s: fail to write obj T%d\n",
			__func__, GEN_ACQUISITIONCONFIG_T8);
		return -EINVAL;
	}

	ret = mxt_write_mem(data, obj->start_addr + 4, 1, &time);
	if (ret < 0) {
		dev_err(&client->dev, "%s: fail to write T%d (%d)\n", __func__,
			obj->type, ret);
		return ret;
	}

	data->autocal_enabled = (time > 0) ? true : false;
	dev_info(&client->dev, "autocal %sabled %d\n",
		 (time > 0) ? "en" : "dis", time);

	return 0;

}
#endif	/* CHECK_ANTITOUCH */

static int mxt_command_proc_msg_handler(struct mxt_data *data,
					struct mxt_message *msg)
{
	struct i2c_client *client = data->client;
	char tag[] = "T6";

	if (msg->data[0] & MXT_CMD_PROC_RESET)
		dev_info(&client->dev, "%s: reset is on going\n", tag);

	if (msg->data[0] & MXT_CMD_PROC_OFL)
		dev_err(&client->dev, "%s: overflow detected\n", tag);

	if (msg->data[0] & MXT_CMD_PROC_SIGERR)
		dev_err(&client->dev, "%s: signal error\n", tag);

	if (msg->data[0] & MXT_CMD_PROC_CAL)
		dev_info(&client->dev, "%s: calibration is on going\n", tag);

	if (msg->data[0] & MXT_CMD_PROC_CFGERR)
		dev_err(&client->dev, "%s: config error\n", tag);

	if (msg->data[0] & MXT_CMD_PROC_COMSERR)
		dev_err(&client->dev, "%s: i2c checksum error\n", tag);

	if (msg->data[0] == 0x00) {
		dev_info(&client->dev, "%s: normal mode\n", tag);
		#if defined(CHECK_ANTITOUCH)
			data->check_timer = 0;
			data->check_antitouch = 0;
			data->check_after_wakeup = 1;
		#endif /*CHECK_ANTITOUCH*/
	}

	return 0;
}

static int mxt_multi_touchscreen_msg_handler(struct mxt_data *data,
					     struct mxt_message *msg)
{
	struct i2c_client *client = data->client;
	char tag[] = "T9";
	int id;
	u8 status;

	id = msg->report_id - data->finger_report_id;

	/* If not a touch event, return */
	if (id < 0 || id >= data->num_of_fingers) {
		dev_err(&client->dev, "%s: invalid finger id %d\n", tag, id);
		return -EINVAL;
	}

	status = msg->data[0];

	if (status & MXT_MT_RELEASE) {
		input_mt_slot(data->input_dev, id);
		input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER,
					   false);
#if defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
		dev_info(&client->dev, "%4s[%d]:\n", "up", id);
#else
		dev_info(&client->dev, "%4s[%d]: %3d, %3d\n", "up",
			 id, data->finger[id].x, data->finger[id].y);
#endif
		data->finger[id].x = 0;
		data->finger[id].y = 0;

		if (data->finger[id].state) {
			if (*data->finger_cnt > 0)
				(*data->finger_cnt)--;
		}
		data->finger[id].state = 0;

#if defined(TOUCH_BOOSTER)
		if (*data->finger_cnt == 0) {
			prcmu_qos_update_requirement(PRCMU_QOS_APE_OPP,
				(char *)client->name,
				PRCMU_QOS_DEFAULT_VALUE);
			prcmu_qos_update_requirement(PRCMU_QOS_DDR_OPP,
				(char *)client->name,
				PRCMU_QOS_DEFAULT_VALUE);
			prcmu_qos_update_requirement(PRCMU_QOS_ARM_KHZ,
				(char *)client->name,
				PRCMU_QOS_DEFAULT_VALUE);
		}
#endif

	} else if ((status & MXT_MT_DETECT) &&
			(status & (MXT_MT_PRESS | MXT_MT_MOVE))) {
		int x, y, w, z;

#if defined(SHAPETOUCH)
		int c;
#endif
		x = ((msg->data[1] << 4) | (msg->data[3] >> 4)) >>
		    data->x_dropbit;
		y = ((msg->data[2] << 4) | (msg->data[3] & 0xF)) >>
		    data->y_dropbit;
		w = msg->data[4];
		z = msg->data[5];

#if defined(SHAPETOUCH)
		c = msg->data[6];
#endif

#if !defined(VERBOSE_DEBUG)
		if (status & MXT_MT_PRESS) {
#endif

#if defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
			dev_info(&client->dev, "%4s[%d]:\n", "down", id);
#elif defined(SHAPETOUCH)
			dev_info(&client->dev, "%4s[%d]: %3d, %3d (%3d,%3d,%3d,%3d)\n",
				"down", id, x, y, w, z, c, data->sumsize);
#else
			dev_info(&client->dev, "%4s[%d]: %3d, %3d (%3d,%3d)\n",
				"down", id, x, y, w, z);
#endif

#if defined(TOUCH_BOOSTER)
			if (*data->finger_cnt == 0) {
				prcmu_qos_update_requirement(PRCMU_QOS_APE_OPP,
					(char *)client->name,
					PRCMU_QOS_APE_OPP_MAX);
				prcmu_qos_update_requirement(PRCMU_QOS_DDR_OPP,
					(char *)client->name,
					PRCMU_QOS_DDR_OPP_MAX);
				prcmu_qos_update_requirement(PRCMU_QOS_ARM_KHZ,
					(char *)client->name, 800000);
			}
#endif

			if (!data->finger[id].state)
				(*data->finger_cnt)++;

			data->finger[id].state = 1;

#if defined(CHECK_ANTITOUCH)
			dev_info(&client->dev, "data->finger_cnt = %d\n",
				*data->finger_cnt);
			if (data->check_after_wakeup) {
				if (*data->finger_cnt > 1) {
					dev_info(&client->dev, "data->check_after_wakeup!!!!!!!!!!!!!!\r\n");
					mxt_calibrate_chip(data);
					mxt_t61_timer_set(data, MXT_T61_TIMER_CMD_START,
						MXT_T61_TIMER_ONESHOT, 2000);

					data->check_after_wakeup = 0;
				} else {
					mxt_t61_timer_set(data, MXT_T61_TIMER_CMD_START,
						MXT_T61_TIMER_ONESHOT, 800);
					data->check_after_wakeup = 2;
				}
			}

#endif
#if !defined(VERBOSE_DEBUG)
		}
#endif

		data->finger[id].x = x;
		data->finger[id].y = y;

		input_mt_slot(data->input_dev, id);
		input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER,
						true);
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, w);
		input_report_abs(data->input_dev, ABS_MT_PRESSURE, z);
		input_report_abs(data->input_dev, ABS_MT_POSITION_X, x);
		input_report_abs(data->input_dev, ABS_MT_POSITION_Y, y);
#if defined(SHAPETOUCH)
		input_report_abs(data->input_dev, ABS_MT_COMPONENT, c);
		input_report_abs(data->input_dev, ABS_MT_SUMSIZE, data->sumsize);
#endif

	} else if (status & MXT_MT_SUPPRESS) {
		int mt_val;
		input_mt_slot(data->input_dev, id);
		mt_val = input_mt_get_value(&data->input_dev->
					    mt[data->input_dev->slot],
					    ABS_MT_TRACKING_ID);
		if (mt_val > 0) {
			input_mt_report_slot_state(data->input_dev,
						   MT_TOOL_FINGER, false);
#if defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
		dev_info(&client->dev, "%4s[%d]:\n", "up/s", id);
#else
		dev_info(&client->dev, "%4s[%d]: %3d, %3d\n", "up/s",
			 id, data->finger[id].x, data->finger[id].y);
#endif
		}
	} else {
		/* ignore changed amplitude and vector messsage */
		if (!((status & MXT_MT_DETECT) &&
			(status & MXT_MT_VECTOR || status & MXT_MT_AMP))) {

			dev_err(&client->dev, "unknown state: %d, 0x%x\n",
				id, status);
		}
		goto out;
	}
#if defined(ATMEL_DEBUG)
	if (likely(!data->driver_paused))
#endif
		input_sync(data->input_dev);

out:
	return 0;
}

static int mxt_touch_suppression_msg_handler(struct mxt_data *data,
					     struct mxt_message *msg)
{
	struct i2c_client *client = data->client;
	char tag[] = "T42";

	if (msg->data[0] & 0x01) {
		dev_info(&client->dev, "%s: palm touch detected\n", tag);
#if defined(CHECK_ANTITOUCH)
	if (data->check_after_wakeup) {
		dev_info(&client->dev, "PALM data->check_after_wakeup PALM  \n");
		mxt_t61_timer_set(data, MXT_T61_TIMER_CMD_START,
			MXT_T61_TIMER_ONESHOT, 2000);
		mxt_calibrate_chip(data);
		data->check_after_wakeup = 0;
	}
#endif
	} else
		dev_info(&client->dev, "%s: palm touch released\n", tag);

	return 0;
}

static int mxt_extra_touchscreen_msg_handler(struct mxt_data *data,
					     struct mxt_message *msg)
{
	struct i2c_client *client = data->client;
	char tag[] = "T57";
	u16 tch_area, atch_area;

#if defined(SHAPETOUCH)
	data->sumsize = (msg->data[1] << 8) | msg->data[0];
#endif

#if defined(CHECK_ANTITOUCH)
	tch_area = (msg->data[3] << 8) | msg->data[2];
	atch_area = (msg->data[5] << 8) | msg->data[4];

	if (atch_area)
		dev_info(&client->dev, "%s: tch=%d, atch=%d\n", tag,
			tch_area, atch_area);

	if (atch_area > 1 && data->check_antitouch == 0) {
		mxt_t8_autocal_set(data, 3);
		data->check_antitouch = 1;
		mxt_t61_timer_set(data, MXT_T61_TIMER_CMD_START,
				  MXT_T61_TIMER_ONESHOT, 1200);
	}
	if ((tch_area == 0) && (atch_area == 0) && (*data->finger_cnt > 0) && (data->check_after_wakeup)) {
		dev_info(&client->dev, "tch_area==%d, atch_area==%d  !!!!!!\n", tch_area, atch_area);
		mxt_calibrate_chip(data);
		data->check_after_wakeup = 0;
	}

#else
	print_hex_dump(KERN_INFO, "T57 msg:", DUMP_PREFIX_NONE, 16, 1, &msg,
		       sizeof(struct mxt_message), false);
#endif /* defined(CHECK_ANTITOUCH) */
	return 0;
}

static int mxt_timer_msg_handler(struct mxt_data *data,
				 struct mxt_message *msg)
{
	struct i2c_client *client = data->client;
	char tag[] = "T61";

#if defined(CHECK_ANTITOUCH)
	dev_info(&client->dev, "%s: %d: datas: %#x, %#x, %#x\n", tag,
		 data->check_antitouch, msg->data[0], msg->data[1],
		 msg->data[2]);

	if ((msg->data[0] & 0xa0) == 0xa0) {
		data->check_antitouch = 0;
		data->check_timer = 0;
		mxt_t8_autocal_set(data, 0);

		if (data->check_after_wakeup == 2)
			data->check_after_wakeup = 0;
	}
#else
	print_hex_dump(KERN_INFO, "T57 msg:", DUMP_PREFIX_NONE, 16, 1, &msg,
		       sizeof(struct mxt_message), false);
#endif /* defined(CHECK_ANTITOUCH) */
	return 0;
}

static int mxt_noise_suppression_msg_handler(struct mxt_data *data,
					     struct mxt_message *msg)
{
	struct i2c_client *client = data->client;
	char tag[] = "T62";
/*
	dev_err(&client->dev, "%s: not prepared.\n", tag);
	print_hex_dump(KERN_ERR, "T62 msg:", DUMP_PREFIX_NONE, 16, 1, &msg,
		       sizeof(struct mxt_message), false);
*/
	return 0;
}

static irqreturn_t mxt_irq_thread(int irq, void *dev_id)
{
	struct mxt_data *data = dev_id;
	struct i2c_client *client = data->client;
	struct mxt_message msg;
	u8 report_id;
	int ret;

	do {
		ret = mxt_read_mem(data, data->msg_proc,
				   sizeof(struct mxt_message), (u8 *)&msg);
		if (ret < 0) {
			dev_err(&client->dev, "fail to read msg (%d)\n", ret);
			goto out;
		}

		printk("msg report_id=%d,%d,%d,%d\n", msg.report_id, msg.data[0], msg.data[1], msg.data[2]);

#if defined(ATMEL_DEBUG)
		if (data->debug_enabled)
			print_hex_dump(KERN_INFO, "MXT MSG:", DUMP_PREFIX_NONE,
					16, 1, &msg, sizeof(struct mxt_message),
					false);
#endif
		report_id = msg.report_id;

		/* ignore invalid interrupt */
		if (report_id == RESERVED_T0 || report_id == RESERVED_T255)
			continue;

		if (report_id < 1 || report_id > data->max_report_id) {
			dev_err(&client->dev, "invalid report id (%d)",
				report_id);
			goto out;
		}

		if (!data->rid_map[report_id].handler) {
			dev_err(&client->dev, "no hander for type %d (id=%d)",
				data->rid_map[report_id].obj_type, report_id);
			print_hex_dump(KERN_ERR, "MXT MSG:", DUMP_PREFIX_OFFSET,
					16, 1, (u8 *)&msg,
					sizeof(struct mxt_message), false);
			goto out;
		}
		if (data->rid_map[report_id].handler)
			data->rid_map[report_id].handler(data, &msg);

	} while (!gpio_get_value(data->pdata->gpio_read_done));

out:
	return IRQ_HANDLED;
}

static int mxt_get_obj_tbl(struct mxt_data *data, u8 force)
{
	struct i2c_client *client = data->client;
	int ret;
	int i;
	u8 type_cnt;

	if (!data->info.obj_num) {
		dev_err(&client->dev, "cat't get obj table. "
			"(the number of obj is %d)\n", data->info.obj_num);
		return -EINVAL;
	}

	if (force)
		kfree(data->obj_tbl);

	data->obj_tbl = kzalloc(data->info.obj_num * sizeof(struct mxt_obj),
				GFP_KERNEL);
	if (!data->obj_tbl) {
		dev_err(&client->dev, "failed to allocate mem for obj tbl\n");
		return -ENOMEM;
	}

	ret = mxt_read_mem(data, OBJ_TBL_START_ADDR,
			   data->info.obj_num * sizeof(struct mxt_obj),
			   (u8 *)data->obj_tbl);
	if (ret < 0) {
		dev_err(&client->dev, "fail to read obj table (%d)\n", ret);
		goto err;
	}

	data->max_report_id = 0;
	type_cnt = 0;

	for (i = 0; i < data->info.obj_num; i++) {
		data->obj_tbl[i].start_addr =
			le16_to_cpu(data->obj_tbl[i].start_addr);

		/* size and instance values are smaller than atual value */
		data->obj_tbl[i].size += 1;
		data->obj_tbl[i].instances += 1;
		data->max_report_id += data->obj_tbl[i].num_report_ids *
				       (data->obj_tbl[i].instances);

		switch (data->obj_tbl[i].type) {

		case GEN_MESSAGEPROCESSOR_T5:
			data->msg_proc = data->obj_tbl[i].start_addr;
			data->msg_obj_size = data->obj_tbl[i].size;
#ifdef DEBUG
			dev_info(&client->dev, "message obj addr: 0x%X, size: "
				"%d\n", data->msg_proc, data->msg_obj_size);
#endif
			break;

		case GEN_COMMANDPROCESSOR_T6:
			data->cmd_proc = data->obj_tbl[i].start_addr;
			break;

		case TOUCH_MULTITOUCHSCREEN_T9:
			data->finger_report_id = type_cnt + 1;
#ifdef DEBUG
			dev_info(&client->dev, "iinger report id: %d\n",
				data->finger_report_id);
#endif
			break;

		default:
			break;
		}

		if (data->obj_tbl[i].num_report_ids) {
			type_cnt += data->obj_tbl[i].num_report_ids *
				    (data->obj_tbl[i].instances);
		}
	}

	dev_info(&client->dev, "the number of obj is %d\n", data->info.obj_num);

#ifdef DEBUG
	dev_info(&client->dev, "the objs in IC.\n");
	for (i = 0; i < data->info.obj_num; i++) {
		dev_info(&client->dev, "obj_T%2d: addr=0x%x, size=%d, "
			"instance=%d, report id=%d\n", data->obj_tbl[i].type,
			data->obj_tbl[i].start_addr, data->obj_tbl[i].size,
			data->obj_tbl[i].instances,
			data->obj_tbl[i].num_report_ids);
	}
#endif
	return 0;

err:
	kfree(data->obj_tbl);
	return ret;
}

static int mxt_get_rid_map(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	int i, j;
	int id;

	data->rid_map = kzalloc((data->max_report_id + 1) *
				sizeof(*data->rid_map),	GFP_KERNEL);

	if (!data->rid_map) {
		dev_err(&client->dev, "failed to allocate mem for rid\n");
		return -ENOMEM;
	}

	data->rid_map[0].instance = 0;
	data->rid_map[0].obj_type = 0;
	id = 1;

	for (i = 0; i < data->info.obj_num; i++) {
		if (data->obj_tbl[i].num_report_ids == 0)
			continue;

		for (j = 1; j <= data->obj_tbl[i].instances; j++) {
			int sid = id;
			while (id < (sid + data->obj_tbl[i].num_report_ids)) {
				data->rid_map[id].instance = j;
				data->rid_map[id].obj_type =
							data->obj_tbl[i].type;
				data->rid_map[id].handler =
							mxt_get_handler(data,
							data->obj_tbl[i].type);
				id++;
			}
		}
	}
#ifdef VERBOSE_DEBUG
	for (i = 0; i <= data->max_report_id; i++) {
		dev_err(&client->dev, "report id[%2d]: T%d, %p\n", i,
			data->rid_map[i].obj_type, data->rid_map[i].handler);
	}
#endif
	return 0;
}

static int mxt_read_info_crc(struct mxt_data *data, u32 *crc_pointer)
{
	struct i2c_client *client = data->client;
	u16 crc_address;
	u8 msg[MXT_CHECKSUM_FIELD_SIZE];
	int ret;

	/* Read Info block CRC address */
	crc_address = OBJ_TBL_START_ADDR +
			data->info.obj_num * OBJ_TBL_ELEMENT_SIZE;

	ret = mxt_read_mem(data, crc_address, ARRAY_SIZE(msg), msg);
	if (ret < 0) {
		dev_err(&client->dev, "fail to read checksum field (%d)\n",
			ret);
		return ret;
	}

	*crc_pointer = msg[0] | (msg[1] << 8) | (msg[2] << 16);

	return 0;
}

static u32 crc24(u32 crc, u8 byte1, u8 byte2)
{
	static const u32 crcpoly = 0x80001B;
	u32 res;
	u16 data_word;

	data_word = (((u16)byte2) << 8) | byte1;
	res = (crc << 1) ^ (u32)data_word;

	if (res & 0x1000000)
		res ^= crcpoly;

	return res;
}

static int mxt_calculate_infoblock_crc(struct mxt_data *data, u32 *crc_pointer)
{
	struct i2c_client *client = data->client;
	u8 *buf;
	int ret;
	int i;
	u32 crc = 0;
	u32 buf_size = OBJ_TBL_START_ADDR +
		       data->info.obj_num * OBJ_TBL_ELEMENT_SIZE;

	buf = kzalloc(buf_size, GFP_KERNEL);
	if (!buf) {
		dev_err(&client->dev, "failed to allocate mem for infoblock\n");
		return -ENOMEM;
	}

	ret = mxt_read_mem(data, 0, buf_size, buf);
	if (ret < 0) {
		dev_err(&client->dev, "fail to read infoblock (%d)\n", ret);
		goto err;
	}

	for (i = 0; i < buf_size - 1; i += 2)
		crc = crc24(crc, buf[i], buf[i + 1]);

	*crc_pointer = crc24(crc, buf[i], 0) & 0x00FFFFFF;

	ret = 0;

err:
	kfree(buf);
	return ret;
}

static int mxt_get_bootloader_version(struct mxt_data *data, u8 val)
{
	struct i2c_client *client = data->client;
	struct i2c_client *client_boot = data->client_boot;
	u8 buf[3];
	int ret;

	if (val & MXT_BOOT_EXTENDED_ID) {
		ret = i2c_master_recv(client_boot, buf, sizeof(buf));
		if (ret != sizeof(buf)) {
			dev_err(&client->dev, "%s: i2c recv failed (%d)\n",
				__func__, ret);
			return -EIO;
		}
		dev_info(&client->dev, "bootloader id:%d, version:%d", buf[1],
			 buf[2]);
	} else {
		dev_info(&client->dev, "bootloader id:%d",
			 val & MXT_BOOT_ID_MASK);
	}
	return 0;
}

static int mxt_check_bootloader(struct mxt_data *data, unsigned int state)
{
	struct i2c_client *client = data->client;
	struct i2c_client *client_boot = data->client_boot;
	int ret;
	u8 val;

recheck:
	ret = i2c_master_recv(client_boot, &val, sizeof(val));
	if (ret != sizeof(val)) {
		dev_err(&client->dev, "%s: i2c recv failed (%d)\n", __func__,
			ret);
		return ret;
	}

	switch (state) {
	case MXT_WAITING_BOOTLOAD_CMD:
		ret = mxt_get_bootloader_version(data, val);
		if (ret < 0) {
			dev_err(&client->dev, "fail to get bootloader version."
				" (%d)\n", ret);
			return ret;
		}
		val &= ~MXT_BOOT_STATUS_MASK;
		break;
	case MXT_WAITING_FRAME_DATA:
	case MXT_APP_CRC_FAIL:
		val &= ~MXT_BOOT_STATUS_MASK;
		break;
	case MXT_FRAME_CRC_PASS:
		if (val == MXT_FRAME_CRC_CHECK)
			goto recheck;
		if (val == MXT_FRAME_CRC_FAIL) {
			dev_err(&client->dev, "bootloader CRC fail\n");
			return -EINVAL;
		}
		break;
	default:
		return -EINVAL;
	}

	if (val != state) {
		dev_err(&client->dev, "invalid bootloader mode (val=0x%x, "
			"state=0x%x)\n", val, state);
		return -EINVAL;
	}

	return 0;
}

static int mxt_unlock_bootloader(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	struct i2c_client *client_boot = data->client_boot;
	u8 buf[] = {MXT_UNLOCK_CMD_LSB, MXT_UNLOCK_CMD_MSB};
	int ret;

	ret = i2c_master_send(client_boot, buf, ARRAY_SIZE(buf));
	if (ret != ARRAY_SIZE(buf)) {
		dev_err(&client->dev, "%s: i2c send failed\n", __func__);
		return -EIO;
	}

	return 0;
}

static int mxt_enter_bootloader(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	int ret;

	/* Change to the bootloader mode */
	ret = mxt_write_obj(data, GEN_COMMANDPROCESSOR_T6, CMD_RESET_OFFSET,
			    MXT_BOOT_VALUE);
	if (ret < 0) {
		dev_err(&client->dev, "%s: fail to write obj 0x%x:%d\n",
			__func__, CMD_RESET_OFFSET, MXT_BOOT_VALUE);
		return ret;
	}
	msleep(MXT_1664S_SW_RESET_TIME);

	return 0;
}

static int mxt_probe_bootloader(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	struct i2c_client *client_boot = data->client_boot;
	u8 val;

	if (i2c_master_recv(client_boot, &val, 1) != 1) {
		dev_err(&client->dev, "%s: i2c recv failed\n", __func__);
		return -EIO;
	}

	if (val & (~MXT_BOOT_STATUS_MASK)) {
		if (val & MXT_APP_CRC_FAIL)
			dev_err(&client->dev, "Application CRC failure\n");
		else
			dev_err(&client->dev, "Device in bootloader mode\n");
	} else {
		dev_err(&client->dev, "%s: Unknow status\n", __func__);
		return -EIO;
	}
	return 0;
}

static int mxt_get_id_info(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	int ret;

	/* Read IC information */
	ret = mxt_read_mem(data, 0X00, sizeof(struct mxt_info_block),
				(u8 *)&data->info);
	if (ret < 0) {
		dev_err(&client->dev, "fail to read ID info (%d)\n", ret);
		return ret;
	}

	print_hex_dump(KERN_INFO, "MXT_id info: ", DUMP_PREFIX_OFFSET, 16, 1,
		       &data->info.family_id, sizeof(struct mxt_info_block),
		       false);

	return 0;
}

static int load_fw_in_built_in(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	int ret;

	ret = 0;
	dev_info(&client->dev, "%s\n", __func__);

	return ret;
}

static int load_fw_in_header(struct mxt_data *data)
{
	struct i2c_client *client = data->client;

	if (ARRAY_SIZE(fw_data) == 0) {
		dev_err(&client->dev, "no fw in header\n");
		return -EINVAL;
	}
	dev_info(&client->dev, "fw size in header is %d\n",
		ARRAY_SIZE(fw_data));

	data->fw_image = kzalloc(ARRAY_SIZE(fw_data), GFP_KERNEL);
	if (!data->fw_image) {
		dev_err(&client->dev, "fail to alloc mem for fw image\n");
		return -ENOMEM;
	}

	memcpy(data->fw_image, fw_data, ARRAY_SIZE(fw_data));

	data->fw_image->hdr_len = sizeof(struct mxt_fw_image);
	data->fw_image->fw_len = ARRAY_SIZE(fw_data) - data->fw_image->hdr_len;

#ifdef DEBUG
	dev_err(&client->dev, "fw_image header info: "
		"0x%X:0x%X:0x%X:0x%X:0x%X:0x%X:0x%X\n",
		data->fw_image->magic_code, data->fw_image->hdr_len,
		data->fw_image->cfg_len, data->fw_image->fw_len,
		data->fw_image->cfg_crc, data->fw_image->fw_ver,
		data->fw_image->build_ver);

	dev_err(&client->dev, "fw_image word data: 0x%X:0x%X:0x%X:0x%X\n",
		data->fw_image->data[0], data->fw_image->data[1],
		data->fw_image->data[2], data->fw_image->data[3]);
#endif

	dev_info(&client->dev, "fw in header is successfully loaded\n");

	return 0;
}

static int load_fw_in_fs(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	int ret;

	ret = 0;
	dev_info(&client->dev, "%s\n", __func__);

	return ret;
}

static int mxt_load_fw(struct mxt_data *data, u8 in)
{
	struct i2c_client *client = data->client;
	int ret;

	switch (in) {
	case IN_BUILT_IN:
		ret = load_fw_in_built_in(data);
		break;
	case IN_HEADER:
		ret = load_fw_in_header(data);
		break;
	case IN_FS:
		ret = load_fw_in_fs(data);
		break;
	default:
		dev_err(&client->dev, "invalid fw data position (%d)\n", in);
		return -EINVAL;
	}
	if (ret < 0) {
		dev_err(&client->dev, "fail to load fw data in %d (%d)\n",
			in, ret);
		return ret;
	}
	return 0;
}

static int mxt_flash_fw(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	unsigned int frame_size;
	unsigned int pos;
	int ret;

	ret = mxt_check_bootloader(data, MXT_WAITING_BOOTLOAD_CMD);
	if (ret < 0) {
		/*may still be unlocked from previous update attempt */
		ret = mxt_check_bootloader(data, MXT_WAITING_FRAME_DATA);
		if (ret < 0) {
			dev_err(&client->dev, "fail to check bootloader. "
				"wating_frame_data state (%d)\n", ret);
			return ret;
		}
	} else {
		dev_info(&client->dev, "unlocking bootloader\n");
		/* Unlock bootloader */
		ret = mxt_unlock_bootloader(data);
		if (ret < 0) {
			dev_err(&client->dev, "fail to unlocking bootloader."
				"(%d)\n", ret);
			return ret;
		}
	}
	pos = 0;
	while (pos < data->fw_image->fw_len) {
		ret = mxt_check_bootloader(data, MXT_WAITING_FRAME_DATA);
		if (ret < 0) {
			dev_err(&client->dev, "fail to check bootloader dufing "
				"fw flashing, wating_frame_data state (%d)\n",
				ret);
			return ret;
		}

		frame_size = ((*(data->fw_image->data + pos) << 8) |
			      *(data->fw_image->data + pos + 1));

		/*
		* We should add 2 at frame size as the the firmware data is not
		* included the CRC bytes.
		*/
		frame_size += 2;

		/* Write one frame to device */
		ret = mxt_fw_write(data, data->fw_image->data + pos,
				   frame_size);
		if (ret < 0) {
			dev_err(&client->dev, "fail to write fw data frame (%d)"
				"\n", ret);
			return ret;
		}

		ret = mxt_check_bootloader(data, MXT_FRAME_CRC_PASS);
		if (ret < 0) {
			dev_err(&client->dev, "fail to check bootloader dufing "
				"fw flashing, frame_crc state (%d)\n", ret);
			return ret;
		}

		pos += frame_size;

		dev_info(&client->dev, "fw is updated %d bytes(/%d bytes)\n",
				pos, data->fw_image->fw_len);

		msleep(20);
	}

	msleep(MXT_1664S_FW_RESET_TIME);
	dev_info(&client->dev, "Sucess updating firmware\n");

	return 0;
}

static int load_config_in_built_in(struct mxt_data *data, u8 type)
{
	struct i2c_client *client = data->client;
	int ret;

	ret = 0;
	dev_info(&client->dev, "%s\n", __func__);

	return ret;
}

static int load_config_in_board(struct mxt_data *data, u8 type)
{
	struct i2c_client *client = data->client;
	struct mxt_config_struct *cur_cfg;
	struct mxt_cfg_image *cur_cfg_image;
	struct mxt_cfg_data *cfg;
	int ret;
	int cfg_len;
	int i;
	u32 offset;
	u32 max_cfg_len;
	u32 size;

	switch (type) {
	case NORMAL_CFG:
		cur_cfg = data->pdata->config;
		break;
	case TA_CFG:
		cur_cfg = data->pdata->ta_config;
		break;
	default:
		dev_err(&client->dev, "invalid config type %d\n", type);
		return -EINVAL;
	}

	dev_info(&client->dev, "loading %s config data from board\n",
		(type) ? "ta" : "normal");

	cfg_len = 0;
	max_cfg_len = 0;

	for (i = 0; cur_cfg[i].data[0] != RESERVED_T255; i++) {
		size = sizeof(struct mxt_cfg_data) + cur_cfg[i].size;
		cfg_len += size;
		max_cfg_len = (max_cfg_len < size) ? size : max_cfg_len;
	}

	cur_cfg_image = kzalloc(sizeof(struct mxt_cfg_image) + cfg_len,
				GFP_KERNEL);
	if (!cur_cfg_image) {
		dev_err(&client->dev, "fail to alloc mem for cfg image\n");
		return -ENOMEM;
	}

	cfg = kzalloc(max_cfg_len, GFP_KERNEL);
	if (!cfg) {
		dev_err(&client->dev, "fail to alloc mem for cfg buf\n");
		ret = -ENOMEM;
		goto err_alloc_cfg;
	}

	cur_cfg_image->hdr_len = sizeof(struct mxt_cfg_image);
	cur_cfg_image->num_of_cfg = i;
	cur_cfg_image->cfg_len = cfg_len;
	cur_cfg_image->cfg_crc = 0;

	dev_info(&client->dev, "loaded config info: hdr_len=%d, num_of_cfg=%d,"
		" cfg_len=%d, cfg_crc=%d\n", cur_cfg_image->hdr_len,
		cur_cfg_image->num_of_cfg, cur_cfg_image->cfg_len,
		cur_cfg_image->cfg_crc);

	offset = cur_cfg_image->hdr_len;
	for (i = 0; i < cur_cfg_image->num_of_cfg; i++) {
		cfg->type = cur_cfg[i].data[0];
		cfg->instance = cur_cfg[i].instance;
		cfg->size = cur_cfg[i].size;

		memcpy(cfg->data, &cur_cfg[i].data[1], cfg->size);
		memcpy((struct mxt_cfg_image *)((u8 *)cur_cfg_image + offset),
			cfg, sizeof(struct mxt_cfg_data) + cfg->size);

		offset += sizeof(struct mxt_cfg_data) + cfg->size;
	}
	kfree(cfg);

#ifdef VERBOSE_DEBUG
	offset = cur_cfg_image->hdr_len;
	for (i = 0; i < cur_cfg_image->num_of_cfg; i++) {
		cfg = (struct mxt_cfg_data *)((u8 *)cur_cfg_image + offset);

		print_hex_dump(KERN_INFO, "MXT cfg: ", DUMP_PREFIX_OFFSET, 16,
			       1, cfg, sizeof(struct mxt_cfg_data) + cfg->size,
			       false);
		offset += sizeof(struct mxt_cfg_data) + cfg->size;
	}
#endif

	if (type == NORMAL_CFG)
		data->cfg_image = cur_cfg_image;
	else if (type == TA_CFG)
		data->ta_cfg_image = cur_cfg_image;

	dev_info(&client->dev, "%s config data in board file is successfully"
		"loaded.\n", (type) ? "ta" : "normal");

	return 0;

err_alloc_cfg:
	kfree(cur_cfg_image);
	cur_cfg_image = NULL;
	return ret;
}

static int load_config_in_fs(struct mxt_data *data, u8 type)
{
	struct i2c_client *client = data->client;
	int ret;

	ret = 0;
	dev_info(&client->dev, "%s\n", __func__);

	return ret;
}

static int mxt_load_general_config(struct mxt_data *data, u8 in)
{
	struct i2c_client *client = data->client;
	int ret;

	/* load general config */
	switch (in) {
	case IN_BUILT_IN:
		ret = load_config_in_built_in(data, NORMAL_CFG);
		break;
	case IN_BOARD:
		ret = load_config_in_board(data, NORMAL_CFG);
		break;
	case IN_FS:
		ret = load_config_in_fs(data, NORMAL_CFG);
		break;
	default:
		dev_err(&client->dev, "invalid config data position (%d)\n",
			in);
		return -EINVAL;
	}
	if (ret < 0) {
		dev_err(&client->dev, "fail to load config data in %d (%d)\n",
			in, ret);
		return ret;
	}

	return 0;
}

static int mxt_load_ta_config(struct mxt_data *data, u8 in)
{
	struct i2c_client *client = data->client;
	int ret;

	/* load ta mode config */
	switch (in) {
	case IN_BUILT_IN:
		ret = load_config_in_built_in(data, TA_CFG);
		break;
	case IN_BOARD:
		ret = load_config_in_board(data, TA_CFG);
		break;
	case IN_FS:
		ret = load_config_in_fs(data, TA_CFG);
		break;
	default:
		dev_err(&client->dev, "invalid config data position (%d)\n",
			in);
		return -EINVAL;
	}
	if (ret < 0) {
		dev_err(&client->dev, "fail to load config data in %d (%d)\n",
			in, ret);
		return ret;
	}

	return 0;
}

static int mxt_write_config(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	struct mxt_obj *obj;
	struct mxt_cfg_data *cfg;
	int ret;
	int index;
	u16 addr;
	u8 val;
	int i;

	val = 0;

	/* Write config info */
	for (index = 0; index < data->cfg_image->cfg_len;) {

		if (index + sizeof(struct mxt_cfg_data) >=
			data->cfg_image->cfg_len) {
			dev_err(&client->dev, "index(%d) of cfg_data exceeded "
				"total size(%d)!!\n",
				index + sizeof(struct mxt_cfg_data),
				data->cfg_image->cfg_len);
			return -EINVAL;
		}

		/* Get the info about each obj */
		cfg = (struct mxt_cfg_data *)(&data->cfg_image->data[index]);

#ifdef VERBOSE_DEBUG
		print_hex_dump(KERN_ERR, "MXT cfg: ", DUMP_PREFIX_OFFSET, 32, 1,
			       cfg, sizeof(struct mxt_cfg_data) + cfg->size,
			       false);
#endif

		index += sizeof(struct mxt_cfg_data) + cfg->size;

		if (index > data->cfg_image->cfg_len) {
			dev_err(&client->dev, "index(%d) of cfg exceeded "
				"total size(%d) in T%d obj!!\n", index,
				data->cfg_image->cfg_len, cfg->type);
			return -EINVAL;
		}

		obj = mxt_get_obj(data, cfg->type);
		if (!obj) {
			dev_err(&client->dev, "T%d is invalid obj type\n",
				cfg->type);
			return -EINVAL;
		}

		/* Check and compare the size, instance of each obj */
		if (cfg->size > obj->size) {
			dev_err(&client->dev, "T%d obj length exceeded "
				"(%d, %d)\n", cfg->type, cfg->size,
				obj->size);
			return -EINVAL;
		}

		if (cfg->instance >= obj->instances) {
			dev_err(&client->dev, "T%d obj instances exceeded "
				"(%d, %d)\n", cfg->type, cfg->instance,
				obj->instances);
			return -EINVAL;
		}

		dev_dbg(&client->dev, "writing config for obj_T%d len=%d "
			"instance=%d (%d/%d)\n", cfg->type, obj->size,
			cfg->instance, index, data->cfg_image->cfg_len);

		addr = obj->start_addr + obj->size * cfg->instance;

		/* Write register values of each obj */
		ret = mxt_write_mem(data, addr, cfg->size, cfg->data);
		if (ret < 0) {
			dev_err(&client->dev, "%s: fail to write T%d (%d)\n",
				__func__, obj->type, ret);
			return ret;
		}

		/*
		 * If firmware is upgraded, new bytes may be added to end of
		 * objects. It is generally forward compatible to zero these
		 * bytes - previous behaviour will be retained. However
		 * this does invalidate the CRC and will force a config
		 * download every time until the configuration is updated.
		 */

		if (cfg->size < obj->size) {
			dev_err(&client->dev, "warn: zeroing %d byte(s) in "
				"T%d\n", obj->size - cfg->size, cfg->type);

			for (i = cfg->size + 1; i < obj->size; i++) {
				ret = mxt_write_mem(data, addr + i, 1, &val);
				if (ret < 0)
					return ret;
			}
		}

	}
	dev_info(&client->dev, "the config data is successfully writed.\n");

	return 0;
}

static int mxt_set_drop_bit(struct mxt_data *data)
{
	struct mxt_cfg_data *cfg;
	int index;

	for (index = 0; index < data->cfg_image->cfg_len;) {

		/* Get the info about each obj */
		cfg = (struct mxt_cfg_data *)(&data->cfg_image->data[index]);
		if (cfg->type == TOUCH_MULTITOUCHSCREEN_T9) {
			/* Are x and y inverted? */
			if (cfg->data[9] & 0x1) {
				data->x_dropbit = (!(cfg->data[21] & 0xC)) << 1;
				data->y_dropbit = (!(cfg->data[19] & 0xC)) << 1;
			} else {
				data->x_dropbit = (!(cfg->data[19] & 0xC)) << 1;
				data->y_dropbit = (!(cfg->data[21] & 0xC)) << 1;
			}
			return 0;
		}
		index += sizeof(struct mxt_cfg_data) + cfg->size;
	}
	return -EINVAL;
}

static int mxt_backup(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	int ret;
	u8 buf = MXT_BACKUP_VALUE;

	ret = mxt_write_mem(data, data->cmd_proc + CMD_BACKUP_OFFSET, 1, &buf);
	if (ret < 0) {
		dev_err(&client->dev, "fail to send backup cmd\n");
		return ret;
	}

	dev_info(&client->dev, "cmd: backup\n");

	return 0;
}

static int mxt_reset(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	int ret;
	u8 buf = 0xff;

	ret = mxt_write_mem(data, data->cmd_proc + CMD_RESET_OFFSET, 1, &buf);
	if (ret < 0) {
		dev_err(&client->dev, "fail to send reset cmd\n");
		return ret;
	}

	dev_info(&client->dev, "cmd: reset\n");

	return 0;
}

static int mxt_transit_mode(struct mxt_data *data, u8 mode)
{
	struct i2c_client *client = data->client;
	struct mxt_cfg_image *ta_cfg_image, *cfg_image;
	struct mxt_cfg_data *cfg;
	struct mxt_obj *obj;
	u32 offset;
	int ret;
	int i;
	u16 addr;

	switch (mode) {

	case TA_MODE:
		ta_cfg_image = data->ta_cfg_image;

		offset = ta_cfg_image->hdr_len;
		for (i = 0; i < ta_cfg_image->num_of_cfg; i++) {
			cfg = (struct mxt_cfg_data *)
			      ((u8 *)ta_cfg_image + offset);

			obj = mxt_get_obj(data, cfg->type);
			if (!obj) {
				dev_err(&client->dev, "%s: T%d is invalid obj "
					"type\n", __func__, cfg->type);
				return -EINVAL;
			}

			addr = obj->start_addr + obj->size * cfg->instance;

			ret = mxt_write_mem(data, addr, cfg->size, cfg->data);
			if (ret < 0) {
				dev_err(&client->dev, "%s: fail to write T%d "
					"(%d)\n", __func__, obj->type, ret);
				return ret;
			}
			offset += sizeof(struct mxt_cfg_data) + cfg->size;
		}
		dev_err(&client->dev, "%s: transit to %s mode\n", __func__,
			"TA");
		break;

	case NORMAL_MODE:
		ta_cfg_image = data->ta_cfg_image;
		cfg_image = data->cfg_image;

		offset = ta_cfg_image->hdr_len;
		for (i = 0; i < ta_cfg_image->num_of_cfg; i++) {
			struct mxt_cfg_data *__cfg;
			cfg = (struct mxt_cfg_data *)
			      ((u8 *)ta_cfg_image + offset);
			__cfg = mxt_get_cfg(data, cfg->type, NORMAL_MODE);
			if (!__cfg) {
				dev_err(&client->dev, "%s: no T%d config\n",
					__func__, cfg->type);
				return -EINVAL;
			}

			obj = mxt_get_obj(data, __cfg->type);
			if (!obj) {
				dev_err(&client->dev, "%s: T%d is invalid obj "
					"type\n", __func__, __cfg->type);
				return -EINVAL;
			}

			addr = obj->start_addr + obj->size * __cfg->instance;

			ret = mxt_write_mem(data, addr, __cfg->size,
					    __cfg->data);
			if (ret < 0) {
				dev_err(&client->dev, "%s: fail to write T%d "
					"(%d)\n", __func__, obj->type, ret);
				return ret;
			}

			offset += sizeof(struct mxt_cfg_data) + cfg->size;
		}
		dev_err(&client->dev, "%s: transit to %s mode\n", __func__,
			"NORMAL");
		break;

	default:
		dev_err(&client->dev, "%s: invalid mode (%d)\n", __func__,
			mode);
		return -EINVAL;
	}

	return 0;
}

static void mxt_mode_transit_worker(struct work_struct *work)
{
	struct mxt_data *data = container_of(work, struct mxt_data, wq.work);
	struct i2c_client *client = data->client;
	int cnt = 0;
	int ret;
	u8 mode = data->dev_mode;

	while (!data->enabled) {
		usleep_range(100000, 200000);

		dev_err(&client->dev, "%s: wiating for device (%d)\n",
			__func__, cnt++);
		if (cnt > 30) {
			dev_err(&client->dev, "%s: transit (%s) waiting timeout"
				"\n", __func__, (mode) ? "TA" : "NORMAL");
			return;
		}
	}

	disable_irq(client->irq);
	ret = mxt_transit_mode(data, mode);
	enable_irq(client->irq);

	if (ret < 0) {
		dev_err(&client->dev, "%s: fail to transit %s mode\n",
		__func__, (mode) ? "TA" : "NORMAL");
		return;
	}

	dev_info(&client->dev, "%s: finish mode transit to %s mode\n", __func__,
		(mode) ? "TA" : "NORMAL");
}

#ifdef CONFIG_USB_SWITCHER
int mxt_usb_switch_notify(struct notifier_block *nb, unsigned long val,
			  void *dev)
{
	struct mxt_data *data = container_of(nb, struct mxt_data, nb);
	struct i2c_client *client = data->client;

	data->usb_notify = TRUE;

	if (use_ab8505_iddet) {

		if (val == USB_LINK_NOT_CONFIGURED_8505) {

			if (data->dev_mode == NORMAL_MODE) {
				dev_err(&client->dev, "%s: already NORMAL state"
					" (0x%lx)\n", __func__, val);
				goto out;
			}

			data->dev_mode = NORMAL_MODE;

		} else {
			switch (val & 0x1F) {
			case	USB_LINK_STD_HOST_NC_8505:
			case	USB_LINK_STD_HOST_C_NS_8505:
			case	USB_LINK_STD_HOST_C_S_8505:
			case	USB_LINK_CDP_8505:
			case	USB_LINK_DEDICATED_CHG_8505:
			case	USB_LINK_CARKIT_CHGR_1_8505:
			case	USB_LINK_CARKIT_CHGR_2_8505:
				if (data->dev_mode == TA_MODE) {
					dev_err(&client->dev, "%s: already TA"
						" state (0x%lx)\n", __func__,
						val);
					goto out;
				}

				data->dev_mode = TA_MODE;
				break;
			default:
				dev_err(&client->dev, "%s: no mode (%d) change"
					" (0x%lx)\n", __func__, val);
				return 0;
			}
		}
	} else {
		if ((val & (EXTERNAL_USB | EXTERNAL_DEDICATED_CHARGER |
		    EXTERNAL_USB_CHARGER | EXTERNAL_CAR_KIT |
		    EXTERNAL_PHONE_POWERED_DEVICE)) && (val &
		    (USB_SWITCH_CONNECTION_EVENT |
		    USB_SWITCH_DRIVER_STARTED))) {

			if (data->dev_mode == TA_MODE) {
				dev_err(&client->dev, "%s: already TA state "
					"(0x%lx)\n", __func__, val);
				goto out;
			}

			data->dev_mode = TA_MODE;

		} else if (val & USB_SWITCH_DISCONNECTION_EVENT) {

			if (data->dev_mode == NORMAL_MODE) {
				dev_err(&client->dev, "%s: already NORMAL state"
					" (0x%lx)\n", __func__, val);
				goto out;
			}

			data->dev_mode = NORMAL_MODE;

		} else {
			dev_err(&client->dev, "%s: no mode (%d) change (0x%lx)"
				"\n", __func__, data->dev_mode, val);
			return 0;
		}
	}

	schedule_delayed_work(&data->wq, msecs_to_jiffies(100));
out:
	return 0;
}
#endif

static int mxt_initialize(struct mxt_data *data, bool probe_time)
{
	struct i2c_client *client = data->client;
	u8 dummy_buf[3] = {0,};
	u32 read_info_crc, calc_info_crc;
	u16 addr, i;
	int ret;

	bool fw_empty = false;

	if (!probe_time)
		goto force_fw_update;

	/* read id informations*/
	ret = mxt_get_id_info(data);
	if (ret < 0) {
		dev_err(&client->dev, "fail to get id info (%d)", ret);

		/* need to check IC is in boot mode */
		ret = mxt_probe_bootloader(data);
		if (ret) {
			dev_err(&client->dev,
				"Failed to verify bootloader's status\n");
			goto out;
		}

		dev_info(&client->dev, "firmware flashing from boot-mode\n");
		fw_empty = true;
	}

	ret = mxt_get_obj_tbl(data, false);
	if (ret < 0)
		dev_err(&client->dev, "fail to get obj tbl (%d)", ret);

force_fw_update:
/*
	ret = mxt_load_fw(data, IN_HEADER);
	if (ret < 0) {
		dev_err(&client->dev, "fail to load fw (%d)", ret);
		return ret;
	}

	dev_err(&client->dev, "fw_empty=%d\n", fw_empty);

	if (!probe_time || fw_empty || fw_latest[0] != data->info.ver ||
	    fw_latest[1] != data->info.build) {

*/
	if (0) {
		if (!fw_empty) {
			ret = mxt_enter_bootloader(data);
			if (ret < 0) {
				dev_err(&client->dev, "fail to enter bootloader"
					" (%d)\n", ret);
				return ret;
			}
		}

		ret = mxt_flash_fw(data);
		if (ret < 0) {
			dev_err(&client->dev, "fail flash firmware (%d)\n",
				ret);
			return ret;
		}

		ret = mxt_get_id_info(data);
		if (ret < 0) {
			dev_err(&client->dev, "fail to get id info (%d)",
				ret);
			return ret;
		}
		ret = mxt_get_obj_tbl(data, true);
		if (ret < 0) {
			dev_err(&client->dev, "fail to get obj tbl (%d)", ret);
			return ret;
			/*
			 * to do
			 * impossible to fw flash?
			 */
		}
	}

	ret = mxt_get_rid_map(data);
	if (ret < 0) {
		dev_err(&client->dev, "fail to get rid map (%d)", ret);
		return ret;
	}

	ret = mxt_read_info_crc(data, &read_info_crc);
	if (ret < 0) {
		dev_err(&client->dev, "fail to read info_crc (%d)", ret);
		goto out;
	}

	ret = mxt_calculate_infoblock_crc(data, &calc_info_crc);
	if (ret < 0) {
		dev_err(&client->dev, "fail to calculate infoblock crc (%d)",
			ret);
		goto out;
	}


	if (read_info_crc != calc_info_crc) {
		dev_err(&data->client->dev, "Infomation CRC error :"
			"[CRC 0x%06X!=0x%06X]\n", read_info_crc, calc_info_crc);
		ret = -EFAULT;
		goto out;
	}

	ret = mxt_load_general_config(data, IN_BOARD);
	if (ret < 0) {
		dev_err(&client->dev, "fail to load general config (%d)", ret);
		return ret;
	}

	/*initialize, config 0 */
	for (i = 0; i < 417; i++) {
		addr = 142+i;
		mxt_write_mem(data, addr, 1, &dummy_buf[0]);
	}
	/*initialize*/

	ret = mxt_write_config(data);
	if (ret < 0) {
		dev_err(&client->dev, "fail to write config (%d)", ret);
		return ret;
	}

	ret = mxt_set_drop_bit(data);
	if (ret < 0) {
		dev_err(&client->dev, "fail to set drop bit (%d)", ret);
		return ret;
	}

	ret = mxt_load_ta_config(data, IN_BOARD);
	if (ret < 0) {
		dev_err(&client->dev, "fail to load ta config (%d)", ret);
		return ret;
	}

	ret = mxt_backup(data);
	if (ret < 0) {
		dev_err(&client->dev, "fail to back-up (%d)", ret);
		return ret;
	}

	msleep(100);

	ret = mxt_reset(data);
	if (ret < 0) {
		dev_err(&client->dev, "fail to reset (%d)", ret);
		return ret;
	}

	msleep(100);

	do {
	/*dummy read*/
	 mxt_read_mem(data, data->msg_proc, 3 /*size*/, dummy_buf);
	printk("%s,dummy_buf=%d, %d %d\n", __func__, dummy_buf[0], dummy_buf[1], dummy_buf[2]);
	msleep(5);
	} while (dummy_buf[0] != 0xFF);

	return 0;

out:
	dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
	return ret;
}

static void mxt_uninitialize(struct mxt_data *data)
{
	kfree(data->obj_tbl);
	kfree(data->rid_map);
	kfree(data->fw_image);
	kfree(data->cfg_image);
	kfree(data->ta_cfg_image);
}

#if defined(CONFIG_PM) || defined(CONFIG_HAS_EARLYSUSPEND)
static int mxt_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mxt_data *data = i2c_get_clientdata(client);
	int i;

	mutex_lock(&data->lock);

	if (!data->enabled) {
		dev_info(&client->dev, "%s: already disabled\n", __func__);
		goto out;
	}

	data->enabled = false;
	disable_irq(client->irq);

	/* power-off*/
	data->pdata->power_con(false);

	/* release all touch. the finger state check rouinte is not needed. */
	for (i = 0; i < data->num_of_fingers; i++) {
		int mt_val;
		input_mt_slot(data->input_dev, i);

		mt_val = input_mt_get_value(&data->input_dev->
					    mt[data->input_dev->slot],
					    ABS_MT_TRACKING_ID);
		if (mt_val != -1) {
#if defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
			dev_info(&client->dev, "%4s[%d]:\n", "up/f", i);
#else
			dev_info(&client->dev, "%4s[%d]: %3d, %3d\n", "up/f",
				 i, data->finger[i].x, data->finger[i].y);
#endif
			data->finger[i].x = 0;
			data->finger[i].y = 0;
			data->finger[i].state = 0;
		}

		input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER,
					   false);
	}
	input_sync(data->input_dev);

	*data->finger_cnt = 0;
	data->usb_notify = FALSE;

out:
	mutex_unlock(&data->lock);
	dev_info(&client->dev, "%s\n", __func__);
	return 0;
}

static int mxt_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mxt_data *data = i2c_get_clientdata(client);
	u8 dummy_buf[3] = {0,};

	mutex_lock(&data->lock);

	if (data->enabled) {
		dev_info(&client->dev, "%s: already enabled\n", __func__);
		goto out;
	}

	/* power-on*/
	data->pdata->power_con(true);
	msleep(100);

/* to do : check
	1. TA state check and do someting
	2. reset error conditions
	3. calibration ??
*/
	data->check_after_wakeup = 1;
	data->enabled = true;

	if (data->usb_notify == TRUE)
		data->usb_notify = FALSE;
	else if (data->usb_notify == FALSE && data->dev_mode == TA_MODE)
		schedule_delayed_work(&data->wq, msecs_to_jiffies(100));

	if (!gpio_get_value(data->pdata->gpio_read_done)) {
		printk("[TSP] interrupt value is low --> dummy read\n");
		do {
		/*dummy read*/
		 mxt_read_mem(data, data->msg_proc, 3 /*size*/, dummy_buf);
		printk("%s,dummy_buf=%d, %d %d\n", __func__, dummy_buf[0], dummy_buf[1], dummy_buf[2]);
		msleep(5);


		} while (dummy_buf[0] != 0xFF);
	}

	enable_irq(client->irq);

out:
	mutex_unlock(&data->lock);
	dev_info(&client->dev, "%s\n", __func__);
	return 0;
}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mxt_early_suspend(struct early_suspend *h)
{
	struct mxt_data *data = container_of(h, struct mxt_data, early_suspend);
	mxt_suspend(&data->client->dev);
}

static void mxt_late_resume(struct early_suspend *h)
{
	struct mxt_data *data = container_of(h, struct mxt_data, early_suspend);
	mxt_resume(&data->client->dev);
}
#endif

#if defined(ATMEL_DEBUG)
/* Functions for mem_access interface */
static int mxt_read_block(struct i2c_client *client, u16 addr, u16 length,
			  u8 *value)
{
	struct i2c_adapter *adapter = client->adapter;
	struct i2c_msg msg[2];
	__le16	le_addr;
	struct mxt_data *data;

	data = i2c_get_clientdata(client);

	if (!data)
		return -ENXIO;

	if ((data->last_read_addr == addr) && (addr == data->msg_proc)) {
		if  (i2c_master_recv(client, value, length) == length) {
			if (data->debug_enabled)
				print_hex_dump(KERN_INFO, "MXT RX:",
					       DUMP_PREFIX_NONE, 16, 1,
					       value, length, false);
			return 0;
		} else
			return -EIO;
	} else {
		data->last_read_addr = addr;
	}

	le_addr = cpu_to_le16(addr);
	msg[0].addr  = client->addr;
	msg[0].flags = 0x00;
	msg[0].len   = 2;
	msg[0].buf   = (u8 *)&le_addr;

	msg[1].addr  = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len   = length;
	msg[1].buf   = (u8 *) value;
	if (i2c_transfer(adapter, msg, 2) == 2) {
		if (data->debug_enabled) {
			print_hex_dump(KERN_INFO, "MXT TX:", DUMP_PREFIX_NONE,
				       16, 1, msg[0].buf, msg[0].len, false);
			print_hex_dump(KERN_INFO, "MXT RX:", DUMP_PREFIX_NONE,
				       16, 1, msg[1].buf, msg[1].len, false);
		}
		return 0;
	} else
		return -EIO;
}

/* Writes a block of bytes (max 256) to given address in mXT chip. */
int mxt_write_block(struct i2c_client *client, u16 addr, u16 length,
		    u8 *value)
{
	int i;
	struct {
		__le16	le_addr;
		u8	data[256];

	} i2c_block_transfer;

	struct mxt_data *data;

	if (length > 256)
		return -EINVAL;

	data = i2c_get_clientdata(client);
	if (data != NULL)
		data->last_read_addr = -1;

	for (i = 0; i < length; i++)
		i2c_block_transfer.data[i] = *value++;

	i2c_block_transfer.le_addr = cpu_to_le16(addr);

	i = i2c_master_send(client, (u8 *)&i2c_block_transfer, length + 2);

	if (i == (length + 2)) {
		if (data->debug_enabled)
			print_hex_dump(KERN_INFO, "MXT TX:", DUMP_PREFIX_NONE,
				16, 1, &i2c_block_transfer, length+2, false);
		return length;
	} else
		return -EIO;
}

static ssize_t mem_access_read(struct file *filp, struct kobject *kobj,
	struct bin_attribute *bin_attr, char *buf, loff_t off, size_t count)
{
	struct i2c_client *client = to_i2c_client(container_of(kobj,
						  struct device, kobj));
	int ret = 0;

	dev_info(&client->dev, "%s: p=%p, off=%lli, c=%zi\n", __func__, buf,
		 off, count);

	if (off >= 32768)
		return -EIO;

	if (off + count > 32768)
		count = 32768 - off;

	if (count > 256)
		count = 256;

	if (count > 0)	{
		ret = mxt_read_block(client, off, count, buf);
		if (ret < 0)
			dev_err(&client->dev, "%s: fail to mxt_read_block (%d)"
				"\n", __func__, ret);
	}

	return ret >= 0 ? count : ret;
}

static ssize_t mem_access_write(struct file *filp, struct kobject *kobj,
	struct bin_attribute *bin_attr, char *buf, loff_t off, size_t count)
{
	struct i2c_client *client = to_i2c_client(container_of(kobj,
						  struct device, kobj));
	int ret = 0;

	dev_info(&client->dev, "%s: p=%p, off=%lli, c=%zi\n", __func__, buf,
		 off, count);

	if (off >= 32768)
		return -EIO;

	if (off + count > 32768)
		count = 32768 - off;

	if (count > 256)
		count = 256;

	if (count > 0) {
		ret = mxt_write_block(client, off, count, buf);
		if (ret < 0)
			dev_err(&client->dev, "%s: fail to mxt_write_block (%d)"
				"\n", __func__, ret);
	}

	return ret >= 0 ? count : 0;
}

static ssize_t pause_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;

	int count = 0;

	count += sprintf(buf + count, "%d", data->driver_paused);
	count += sprintf(buf + count, "\n");

	dev_info(&client->dev, "%s: %s\n", __func__,
		(data->driver_paused) ? "paused" : "unpaused");

	return count;
}

static ssize_t pause_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int val = 0;
	int ret;

	ret = sscanf(buf, "%u", &val);
	if (ret != 1) {
		dev_err(&client->dev, "%s: wrong size %d (%d)\n",
			__func__, val, ret);
		return -EINVAL;
	}

	if (val < 0 || val >= 2) {
		dev_err(&client->dev, "%s: not proper switch value %d\n",
			__func__, val);
		return -EINVAL;
	}

	data->driver_paused = val;
	dev_info(&client->dev, "%s: %s\n", __func__,
		(data->driver_paused) ? "paused" : "unpaused");

	return count;
}

static ssize_t debug_enable_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int count = 0;

	count += sprintf(buf + count, "%d", data->debug_enabled);
	count += sprintf(buf + count, "\n");

	dev_info(&client->dev, "%s: %s\n", __func__,
		(data->debug_enabled) ? "debug enabled" : "debug disabled");

	return count;
}

static ssize_t debug_enable_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int val = 0;
	int ret;

	ret = sscanf(buf, "%u", &val);
	if (ret != 1) {
		dev_err(&client->dev, "%s: wrong size %d (%d)\n",
			__func__, val, ret);
		return -EINVAL;
	}

	if (val < 0 || val >= 2) {
		dev_err(&client->dev, "%s: not proper switch value %d\n",
			__func__, val);
		return -EINVAL;
	}

	data->debug_enabled = val;
	dev_info(&client->dev, "%s: %s\n", __func__,
		(data->debug_enabled) ? "debug enabled" : "debug disabled");

	return count;
}

static ssize_t command_calibrate_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t count)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	int ret;

	ret = mxt_calibrate_chip(data);

	return (ret < 0) ? ret : count;
}

static ssize_t command_reset_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	int ret;

	ret = mxt_reset(data);

	return (ret < 0) ? ret : count;
}

static ssize_t command_backup_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	int ret;

	ret = mxt_backup(data);

	return (ret < 0) ? ret : count;
}

/*
 * to do : change sysfs file permission
 */
/* sysfs files for libmaxtouch interface */
static DEVICE_ATTR(pause_driver, S_IRUGO | S_IWUGO, pause_show, pause_store);
static DEVICE_ATTR(debug_enable, S_IRUGO | S_IWUGO, debug_enable_show,
		   debug_enable_store);
static DEVICE_ATTR(command_calibrate, S_IRUGO | S_IWUGO, NULL,
		   command_calibrate_store);
static DEVICE_ATTR(command_reset, S_IRUGO | S_IWUGO, NULL,
		   command_reset_store);
static DEVICE_ATTR(command_backup, S_IRUGO | S_IWUGO, NULL,
		   command_backup_store);

static struct attribute *libmaxtouch_attributes[] = {
	&dev_attr_pause_driver.attr,
	&dev_attr_debug_enable.attr,
	&dev_attr_command_calibrate.attr,
	&dev_attr_command_reset.attr,
	&dev_attr_command_backup.attr,
	NULL,
};

static struct attribute_group libmaxtouch_attr_group = {
	.attrs = libmaxtouch_attributes,
};

struct bin_attribute mem_access_attr;
#endif

#if defined(MXT_DEBUG)
static ssize_t mxt_dgb_switch_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;

	data->g_debug_switch = !data->g_debug_switch;

	dev_info(&client->dev, "%s: debug switch is toggled (%d)\n", __func__,
		data->g_debug_switch);

	return count;
}

static ssize_t mxt_obj_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int ret;
	unsigned int type;
	unsigned int offset;
	unsigned int val;

	sscanf(buf, "%u%u%u", &type, &offset, &val);
	dev_info(&client->dev, "%s: write obj T%2d:%d, 0x%x\n",
		__func__, (u8)type, (u8)offset, (u8)val);

	ret = mxt_write_obj(data, (u8)type, (u8)offset, (u8)val);
	if (ret < 0) {
		dev_err(&client->dev, "%s: fail to write obj %d:%d, 0x%x\n",
			__func__, (u8)type, (u8)offset, (u8)val);
		goto out;
	}

	val = 0xff;
	ret = mxt_read_obj(data, (u8)type, (u8)offset, (u8 *)&val);
	if (ret < 0) {
		dev_err(&client->dev, "%s: fail to read obj 0x%x:%d, 0x%x\n",
			__func__, (u8)type, (u8)offset, (u8)val);
		goto out;
	}

	dev_info(&client->dev, "%s: write obj T%2d:%d, 0x%x\n",
		__func__, (u8)type, (u8)offset, (u8)val);
out:
	return count;
}

static ssize_t mxt_obj_show(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	struct mxt_obj *obj;
	int ret;
	unsigned int type;
	u8 *buffer;

	sscanf(buf, "%u", &type);

	obj = mxt_get_obj(data, (u8)type);
	if (!obj) {
		dev_err(&client->dev, "%s: T%d is invalid obj type\n",
			__func__, (u8)type);
		goto out;
	}

	buffer = kzalloc(obj->size, GFP_KERNEL);
	if (!buffer) {
		dev_err(&client->dev, "%s: fail to alloc buffer\n", __func__);
		goto out;
	}

	ret = mxt_read_mem(data, obj->start_addr, obj->size, buffer);
	if (ret < 0) {
		dev_err(&client->dev, "%s: fail to read T%d (%d)\n",
			__func__, (u8)type, ret);
		goto out;
	}

	dev_info(&client->dev, "%s: obj T%d is\n", __func__, (u8)type);
	print_hex_dump(KERN_INFO, "mxt obj:", DUMP_PREFIX_OFFSET, 16, 1, buffer,
			obj->size, false);
out:
	return count;
}

/*
 * to do : change sysfs file permission
 */
static DEVICE_ATTR(obj_show, S_IRWXU | S_IRWXG | S_IRWXO, NULL, mxt_obj_show);
static DEVICE_ATTR(obj_write, S_IRWXU | S_IRWXG | S_IRWXO, NULL, mxt_obj_store);
static DEVICE_ATTR(dbg_switch, S_IRWXU | S_IRWXG | S_IRWXO, NULL,
							mxt_dgb_switch_store);

static struct attribute *mxt_attrs[] = {
	&dev_attr_obj_show.attr,
	&dev_attr_obj_write.attr,
	&dev_attr_dbg_switch.attr,
	NULL,
};

static const struct attribute_group mxt_attr_group = {
	.attrs = mxt_attrs,
};
#endif

#if defined(SEC_FAC_TSP)
static void set_cmd_result(struct mxt_data *data, char *buff, int len)
{
	struct sec_fac_tsp *fdata = data->fdata;
	strncat(fdata->cmd_result, buff, len);
}

static ssize_t cmd_store(struct device *dev, struct device_attribute *devattr,
			 const char *buf, size_t count)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	struct sec_fac_tsp *fdata = data->fdata;
	struct i2c_client *client = data->client;
	char *cur, *start, *end;
	char buff[TSP_CMD_STR_LEN] = {0, };
	int len, i;
	struct tsp_cmd *tsp_cmd_ptr = NULL;
	char delim = ',';
	bool cmd_found = false;
	int ret;
	int param_cnt = 0;

	if (fdata->cmd_is_running == true) {
		dev_err(&client->dev, "%s: other cmd is running.\n", __func__);
		goto err_out;
	}

	/* check lock  */
	mutex_lock(&fdata->cmd_lock);
	fdata->cmd_is_running = true;
	mutex_unlock(&fdata->cmd_lock);

	fdata->cmd_state = RUNNING;

	for (i = 0; i < ARRAY_SIZE(fdata->cmd_param); i++)
		fdata->cmd_param[i] = 0;

	len = (int)count;
	if (*(buf + len - 1) == '\n')
		len--;
	memset(fdata->cmd, 0x00, ARRAY_SIZE(fdata->cmd));
	memcpy(fdata->cmd, buf, len);

	cur = strchr(buf, (int)delim);
	if (cur)
		memcpy(buff, buf, cur - buf);
	else
		memcpy(buff, buf, len);

	/* find command */
	list_for_each_entry(tsp_cmd_ptr, &fdata->cmd_list_head, list) {
		if (!strcmp(buff, tsp_cmd_ptr->cmd_name)) {
			cmd_found = true;
			break;
		}
	}

	/* set not_support_cmd */
	if (!cmd_found) {
		list_for_each_entry(tsp_cmd_ptr, &fdata->cmd_list_head, list) {
			if (!strcmp("not_support_cmd", tsp_cmd_ptr->cmd_name))
				break;
		}
	}

	/* parsing TSP standard tset parameter */
	if (cur && cmd_found) {
		cur++;
		start = cur;
		do {
			if (*cur == delim || cur - buf == len) {
				end = cur;
				memset(buff, 0x00, ARRAY_SIZE(buff));
				memcpy(buff, start, end - start);
				*(buff + strlen(buff)) = '\0';

				ret = kstrtoint(buff, 10,
						&fdata->cmd_param[param_cnt]);
				if (ret < 0) {
					dev_info(&client->dev, "%s: fail to get"
						 " %dth param (%d)\n", __func__,
						 param_cnt, ret);
					goto err_out;
				}
				start = cur + 1;
				memset(buff, 0x00, ARRAY_SIZE(buff));
				param_cnt++;
			}
			cur++;
		} while (cur - buf <= len);
	}

	dev_info(&client->dev, "%s: cmd = %s\n", __func__,
		 tsp_cmd_ptr->cmd_name);
	for (i = 0; i < param_cnt; i++)
		dev_info(&client->dev, "cmd param %d= %d\n", i,
							fdata->cmd_param[i]);

	tsp_cmd_ptr->cmd_func(data);

err_out:
	return count;
}

static ssize_t cmd_status_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	struct sec_fac_tsp *fdata = data->fdata;
	struct i2c_client *client = data->client;
	char buff[16];

	dev_info(&client->dev, "%s: check status:%d\n", __func__,
							fdata->cmd_state);

	switch (fdata->cmd_state) {
	case WAITING:
		sprintf(buff, "%s", TOSTRING(WAITING));
		break;
	case RUNNING:
		sprintf(buff, "%s", TOSTRING(RUNNING));
		break;
	case OK:
		sprintf(buff, "%s", TOSTRING(OK));
		break;
	case FAIL:
		sprintf(buff, "%s", TOSTRING(FAIL));
		break;
	case NOT_APPLICABLE:
		sprintf(buff, "%s", TOSTRING(NOT_APPLICABLE));
		break;
	default:
		sprintf(buff, "%s", TOSTRING(NOT_APPLICABLE));
		break;
	}

	return sprintf(buf, "%s\n", buff);
}

static ssize_t cmd_result_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	struct sec_fac_tsp *fdata = data->fdata;
	struct i2c_client *client = data->client;

	dev_info(&client->dev, "%s: \"%s\"(%d)\n", __func__,
				fdata->cmd_result, strlen(fdata->cmd_result));

	mutex_lock(&fdata->cmd_lock);
	fdata->cmd_is_running = false;
	mutex_unlock(&fdata->cmd_lock);

	fdata->cmd_state = WAITING;

	return sprintf(buf, "%s", fdata->cmd_result);
}

static void set_default_result(struct mxt_data *data)
{
	struct sec_fac_tsp *fdata = data->fdata;
	char delim = ':';

	memset(fdata->cmd_result, 0x00, ARRAY_SIZE(fdata->cmd_result));
	memset(fdata->cmd_buff, 0x00, ARRAY_SIZE(fdata->cmd_buff));
	memcpy(fdata->cmd_result, fdata->cmd, strlen(fdata->cmd));
	strncat(fdata->cmd_result, &delim, 1);
}


static void not_support_cmd(void *device_data)
{
	struct mxt_data *data = (struct  mxt_data *)device_data;
	struct sec_fac_tsp *fdata = data->fdata;
	struct i2c_client *client = data->client;

	set_default_result(data);

	sprintf(fdata->cmd_buff, "%s", "NA");
	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));
	fdata->cmd_state = NOT_APPLICABLE;
	dev_info(&client->dev, "%s: \"%s\"(%d)\n", __func__,
				fdata->cmd_buff, strlen(fdata->cmd_buff));
	return;
}

static void fw_update(void *device_data)
{
	struct mxt_data *data = (struct mxt_data *)device_data;
	struct sec_fac_tsp *fdata = data->fdata;
	struct i2c_client *client = data->client;
	int ret;

	set_default_result(data);

	if (!data->enabled) {
		dev_err(&client->dev, "%s: device is not enabled\n", __func__);
		goto err;
	}

	disable_irq(client->irq);
	ret = mxt_initialize(data, false);
	enable_irq(client->irq);
	if (ret < 0) {
		dev_err(&client->dev, "%s: fail to upate\n", __func__);
		goto err;
	}

	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));

	fdata->cmd_state = OK;
	dev_info(&client->dev, "%s: \"%s\"\n", __func__, fdata->cmd_buff);

	return;

err:
	sprintf(fdata->cmd_buff, "%s", "NG");
	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));
	fdata->cmd_state = FAIL;

	return ;
}

static void get_fw_ver_bin(void *device_data)
{
	struct mxt_data *data = (struct mxt_data *)device_data;
	struct sec_fac_tsp *fdata = data->fdata;
	struct i2c_client *client = data->client;

	set_default_result(data);

	if (!fw_latest[0]) {
		dev_err(&client->dev, "no readed fw ver\n");
		goto err;
	}

	sprintf(fdata->cmd_buff, "AT00%04x", fw_latest[0]);
	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));

	fdata->cmd_state = OK;
	dev_info(&client->dev, "%s: \"%s\"\n", __func__, fdata->cmd_buff);

	return;

err:
	sprintf(fdata->cmd_buff, "%s", "NG");
	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));
	fdata->cmd_state = FAIL;

	return ;
}

static void get_fw_ver_ic(void *device_data)
{
	struct mxt_data *data = (struct mxt_data *)device_data;
	struct sec_fac_tsp *fdata = data->fdata;
	struct i2c_client *client = data->client;
	int ret;
	u8 buf;

	set_default_result(data);

	if (!data->enabled) {
		dev_err(&client->dev, "%s: device is not enabled\n", __func__);
		goto err;
	}

	ret = mxt_read_mem(data, 0X02, sizeof(u8), (u8 *)&buf);
	if (ret < 0) {
		dev_err(&client->dev, "fail to read fw ver of IC (%d)\n", ret);
		goto err;
	}

	/*sprintf(fdata->cmd_buff, "%#X", buf);*/
	sprintf(fdata->cmd_buff, "AT00%04x", buf);
	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));
	fdata->cmd_state = OK;

	dev_info(&client->dev, "%s: \"%s\"\n", __func__, fdata->cmd_buff);

	return;

err:
	sprintf(fdata->cmd_buff, "%s", "NG");
	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));
	fdata->cmd_state = FAIL;

	return ;
}

static void get_config_ver(void *device_data)
{
	struct mxt_data *data = (struct mxt_data *)device_data;
	struct sec_fac_tsp *fdata = data->fdata;
	struct i2c_client *client = data->client;

	set_default_result(data);

	if (!data->enabled) {
		dev_err(&client->dev, "%s: device is not enabled\n", __func__);
		goto err;
	}

	sprintf(fdata->cmd_buff, "%s", data->pdata->config_ver);

	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));
	fdata->cmd_state = OK;

	dev_info(&client->dev, "%s: \"%s\"\n", __func__, fdata->cmd_buff);

	return;

err:
	sprintf(fdata->cmd_buff, "%s", "NG");
	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));
	fdata->cmd_state = FAIL;

	return ;
}

static void get_threshold(void *device_data)
{
	struct mxt_data *data = (struct mxt_data *)device_data;
	struct sec_fac_tsp *fdata = data->fdata;
	struct i2c_client *client = data->client;
	int ret;
	u8 buf;

	set_default_result(data);

	if (!data->enabled) {
		dev_err(&client->dev, "%s: device is not enabled\n", __func__);
		goto err;
	}

	ret = mxt_read_obj(data, TOUCH_MULTITOUCHSCREEN_T9, 7, &buf);
	if (ret < 0) {
		dev_err(&client->dev, "fail to read threshold (%d)\n", ret);
		goto err;
	}

	sprintf(fdata->cmd_buff, "%d", buf);
	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));
	fdata->cmd_state = OK;

	dev_info(&client->dev, "%s: \"%s\"\n", __func__, fdata->cmd_buff);

	return;

err:
	sprintf(fdata->cmd_buff, "%s", "NG");
	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));
	fdata->cmd_state = FAIL;

	return ;
}

static void module_off_master(void *device_data)
{
	struct mxt_data *data = (struct mxt_data *)device_data;
	struct sec_fac_tsp *fdata = data->fdata;
	struct i2c_client *client = data->client;

	set_default_result(data);

	mutex_lock(&data->lock);
	data->enabled = false;
	disable_irq(client->irq);
	data->pdata->power_con(false);
	mutex_unlock(&data->lock);

	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));
	fdata->cmd_state = OK;
	dev_info(&client->dev, "%s: \"%s\"\n", __func__, fdata->cmd_buff);

	return;
}

static void module_on_master(void *device_data)
{
	struct mxt_data *data = (struct mxt_data *)device_data;
	struct sec_fac_tsp *fdata = data->fdata;
	struct i2c_client *client = data->client;

	set_default_result(data);

	mutex_lock(&data->lock);
	data->pdata->power_con(true);
	data->enabled = true;
	enable_irq(client->irq);
	mutex_unlock(&data->lock);

	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));
	fdata->cmd_state = OK;
	dev_info(&client->dev, "%s: \"%s\"\n", __func__, fdata->cmd_buff);

	return;
}

static void get_chip_vendor(void *device_data)
{
	struct mxt_data *data = (struct mxt_data *)device_data;
	struct sec_fac_tsp *fdata = data->fdata;
	struct i2c_client *client = data->client;

	set_default_result(data);

	sprintf(fdata->cmd_buff, "%s", "ATMEL");
	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));
	fdata->cmd_state = OK;

	dev_info(&client->dev, "%s: \"%s\"\n", __func__, fdata->cmd_buff);

	return;
}

static void get_chip_name(void *device_data)
{
	struct mxt_data *data = (struct mxt_data *)device_data;
	struct sec_fac_tsp *fdata = data->fdata;
	struct i2c_client *client = data->client;

	set_default_result(data);

	sprintf(fdata->cmd_buff, "%s", "MXT224S");
	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));
	fdata->cmd_state = OK;

	dev_info(&client->dev, "%s: \"%s\"\n", __func__, fdata->cmd_buff);

	return;
}

static void get_x_num(void *device_data)
{
	struct mxt_data *data = (struct mxt_data *)device_data;
	struct sec_fac_tsp *fdata = data->fdata;
	struct i2c_client *client = data->client;

	set_default_result(data);

	sprintf(fdata->cmd_buff, "%d", data->fdata->num_of_x);
	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));
	fdata->cmd_state = OK;

	dev_info(&client->dev, "%s: \"%s\"\n", __func__, fdata->cmd_buff);

	return;
}

static void get_y_num(void *device_data)
{
	struct mxt_data *data = (struct mxt_data *)device_data;
	struct sec_fac_tsp *fdata = data->fdata;
	struct i2c_client *client = data->client;

	set_default_result(data);

	sprintf(fdata->cmd_buff, "%d", data->fdata->num_of_y);
	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));
	fdata->cmd_state = OK;

	dev_info(&client->dev, "%s: \"%s\"\n", __func__, fdata->cmd_buff);

	return;
}

static int mxt_set_diagnostic_mode(struct mxt_data *data, u8 mode)
{
	struct i2c_client *client = data->client;
	int ret;

	ret = mxt_write_obj(data, GEN_COMMANDPROCESSOR_T6,
			    CMD_DIAGNOSTIC_OFFSET, mode);
	if (ret < 0) {
		dev_err(&client->dev, "%s: fail to enter diagnostic %#x mode "
			"(%d)\n", __func__, mode, ret);
		goto err;
	}

	/* confirm mode */
	if (mode & MXT_DIAG_MODE_MASK) {
		u8 mode_backup = mode;
		short cnt = 10;
		do {
			ret = mxt_read_obj(data, DEBUG_DIAGNOSTIC_T37,
				MXT_DIAG_MODE, &mode);
			if (ret < 0) {
				dev_err(&client->dev, "failed to enter %#x mode"
					" (%d)\n", mode_backup, ret);
				goto err;
			}
			msleep(20);
			if (cnt-- < 0) {
				dev_err(&client->dev, "failed to enter %#x mode"
					" (count underflow)\n", mode_backup);
				ret = -EAGAIN;
				goto err;
			}

		} while (mode != mode_backup);
		dev_info(&client->dev, "current dianostic mode is %#x\n",
			 mode);
	}

	return 0;
err:
	return ret;
}

static int mxt_read_all_diagnostic_data(struct mxt_data *data, u8 mode)
{
	struct i2c_client *client = data->client;
	struct sec_fac_tsp *fdata = data->fdata;
	struct mxt_obj *obj;
	u16 *buf, *raw_data_buf;
	int min, max;
	int buf_size;
	int i;
	int ret;
	int offset;

	switch (mode) {
	case MXT_DIAG_REFERENCE_MODE:
		offset = -REFERENCE_OFFSET;
		raw_data_buf = fdata->reference;
		break;
	case MXT_DIAG_DELTA_MODE:
		offset = DELTA_OFFSET;
		raw_data_buf = fdata->delta;
		break;
	default:
		dev_err(&client->dev, "%s: invalid mode %#x\n", __func__, mode);
		return -EINVAL;
	}

	buf_size = ((int)(fdata->num_of_x * data->info.matrix_ysize /
		   MXT_NODE_PER_PAGE) + 1) * MXT_PAGE_SIZE;

	buf = kzalloc(buf_size, GFP_KERNEL);
	if (!buf) {
		dev_err(&client->dev, "%s: fail to alloc buf\n", __func__);
		return -ENOMEM;
	}

	/* to make the Page Num to 0 */
	ret = mxt_set_diagnostic_mode(data, MXT_DIAG_CTE_MODE);
	if (ret < 0) {
		dev_err(&client->dev, "%s: fail to set mode %#x (%d)\n",
			__func__, MXT_DIAG_CTE_MODE, ret);
		goto out;
	}
	/* change the debug mode */
	ret = mxt_set_diagnostic_mode(data, mode);
	if (ret < 0) {
		dev_err(&client->dev, "%s: fail to set mode %#x (%d)\n",
			__func__, mode, ret);
		goto out;
	}

	/* get object info for diagnostic */
	obj = mxt_get_obj(data, DEBUG_DIAGNOSTIC_T37);
	if (!obj) {
		dev_err(&client->dev, "%s: fail to get obj\n", __func__);
		ret = -EINVAL;
		goto out;
	}

	/* initialize min, max value */
	max = 0x1 << (sizeof(int) * 8 - 1);
	min = ~max;

	/* read raw data*/
	for (i = 0; i < buf_size / MXT_PAGE_SIZE; i++) {
		u8 page;
		int next_page = i + 1;
		int cnt = 30;
		u8 *__buf = (u8 *)buf;

		ret = mxt_read_mem(data, obj->start_addr + MXT_DIAG_DATA,
				   MXT_PAGE_SIZE, __buf + MXT_PAGE_SIZE * i);

		if (next_page == buf_size / MXT_PAGE_SIZE)
			break;

		/* page up */
		ret = mxt_set_diagnostic_mode(data, MXT_DIAG_PAGE_UP);
		if (ret < 0) {
			dev_err(&client->dev, "%s: fail to set mode %#x (%d)\n",
				__func__, MXT_DIAG_PAGE_UP, ret);
			goto out;
		}
		do {
			ret = mxt_read_mem(data, obj->start_addr +
					   MXT_DIAG_PAGE, 1, &page);
			if (ret < 0) {
				dev_err(&client->dev, "%s: fail to read cur "
					"page (%d)\n", __func__, ret);
				goto out;
			}

			if (cnt-- < 0) {
				dev_err(&client->dev, "%s: fail to page up "
					"cnt underflow\n", __func__);
				ret = -EAGAIN;
				goto out;
			}

			usleep_range(1000, 1100);

		} while (page != next_page);
	}

	for (i = 0; i < buf_size / MXT_BYTE_PER_NODE; i++) {
		int x, y, src_idx, dest_idx;
		x = i / data->info.matrix_ysize;
		y = i % data->info.matrix_ysize;
		src_idx = x * data->info.matrix_ysize + y;
		dest_idx = x * fdata->num_of_y + y;

		if (y >= fdata->num_of_y ||  x >= fdata->num_of_x)
			continue;

		/*
		 * to do : if dual-X is enabled, reference data of bottom
			   tx line must be doubled. But now the dual-X is not
			   enabled in TA mode.
		 */
		buf[src_idx] += offset;
		raw_data_buf[dest_idx] = buf[src_idx];

		max = (max < (s16)buf[src_idx]) ? (s16)buf[src_idx] : max;
		min = (min > (s16)buf[src_idx]) ? (s16)buf[src_idx] : min;
	}

	switch (mode) {
	case MXT_DIAG_REFERENCE_MODE:
		fdata->reference_min = min;
		fdata->reference_max = max;
		break;
	case MXT_DIAG_DELTA_MODE:
		fdata->delta_min = min;
		fdata->delta_max = max;
		break;
	}

	ret = 0;
out:
	kfree(buf);
	return ret;
}

static void run_reference_read(void *device_data)
{
	struct mxt_data *data = (struct mxt_data *)device_data;
	struct sec_fac_tsp *fdata = data->fdata;
	struct i2c_client *client = data->client;
	int ret;

	set_default_result(data);

	/* enter mode */
	ret = mxt_read_all_diagnostic_data(data, MXT_DIAG_REFERENCE_MODE);
	if (ret < 0) {
		dev_info(&client->dev, "%s: fail to read reference (%d)\n",
			 __func__, ret);
		goto err;
	}

	sprintf(fdata->cmd_buff, "%d, %d", fdata->reference_min,
		fdata->reference_max);

	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));
	fdata->cmd_state = OK;

	dev_info(&client->dev, "%s: \"%s\"\n", __func__, fdata->cmd_buff);

	return;

err:
	sprintf(fdata->cmd_buff, "%s", "NG");
	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));
	fdata->cmd_state = FAIL;

	return ;
}

static void get_reference(void *device_data)
{
	struct mxt_data *data = (struct mxt_data *)device_data;
	struct sec_fac_tsp *fdata = data->fdata;
	struct i2c_client *client = data->client;
	int val;

	set_default_result(data);


	val = fdata->reference[fdata->cmd_param[0] * fdata->num_of_y +
			       fdata->cmd_param[1]];

	sprintf(fdata->cmd_buff, "%d", val);
	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));
	fdata->cmd_state = OK;

	dev_info(&client->dev, "%s: \"%s\"\n", __func__, fdata->cmd_buff);

	return;
}

static void run_delta_read(void *device_data)
{
	struct mxt_data *data = (struct mxt_data *)device_data;
	struct sec_fac_tsp *fdata = data->fdata;
	struct i2c_client *client = data->client;
	int ret;

	set_default_result(data);

	/* enter mode */
	ret = mxt_read_all_diagnostic_data(data, MXT_DIAG_DELTA_MODE);
	if (ret < 0) {
		dev_info(&client->dev, "%s: fail to read delta (%d)\n",
			 __func__, ret);
		goto err;
	}

	sprintf(fdata->cmd_buff, "%d, %d", fdata->delta_min,
		fdata->delta_max);

	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));
	fdata->cmd_state = OK;

	dev_info(&client->dev, "%s: \"%s\"\n", __func__, fdata->cmd_buff);

	return;

err:
	sprintf(fdata->cmd_buff, "%s", "NG");
	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));
	fdata->cmd_state = FAIL;

	return ;
}

static void get_delta(void *device_data)
{
	struct mxt_data *data = (struct mxt_data *)device_data;
	struct sec_fac_tsp *fdata = data->fdata;
	struct i2c_client *client = data->client;
	int val;

	set_default_result(data);

	val = fdata->delta[fdata->cmd_param[0] * fdata->num_of_y +
			   fdata->cmd_param[1]];

	sprintf(fdata->cmd_buff, "%d", val);
	set_cmd_result(data, fdata->cmd_buff, strlen(fdata->cmd_buff));
	fdata->cmd_state = OK;

	dev_info(&client->dev, "%s: \"%s\"\n", __func__, fdata->cmd_buff);

	return;
}

static int mxt_sec_fac_init(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	struct sec_fac_tsp *fdata;
	int ret;
	int i;

	fdata = kzalloc(sizeof(struct sec_fac_tsp), GFP_KERNEL);
	if (!fdata) {
		dev_err(&client->dev, "fail to alloc fdata\n");
		return -ENOMEM;
	}

	ret = mxt_read_obj(data, TOUCH_MULTITOUCHSCREEN_T9, 3,
			   &fdata->num_of_x);
	if (ret < 0) {
		dev_err(&client->dev, "%s: fail to read num of x (%d)\n",
			__func__, ret);
		goto err_get_num_of_x;
	}

	ret = mxt_read_obj(data, TOUCH_MULTITOUCHSCREEN_T9, 4,
			   &fdata->num_of_y);
	if (ret < 0) {
		dev_err(&client->dev, "%s: fail to read num of y (%d)\n",
			__func__, ret);
		goto err_get_num_of_y;
	}

	fdata->num_of_node = fdata->num_of_x * fdata->num_of_y;
#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
	dev_info(&client->dev, "%s: x=%d, y=%d, total=%d\n", __func__,
		fdata->num_of_x, fdata->num_of_y, fdata->num_of_node);
#endif

	fdata->reference = kzalloc(fdata->num_of_node * sizeof(u16),
				   GFP_KERNEL);
	if (!fdata->reference) {
		dev_err(&client->dev, "fail to alloc reference of fdata\n");
		ret = -ENOMEM;
		goto err_alloc_reference;
	}

	fdata->delta = kzalloc(fdata->num_of_node * sizeof(u16), GFP_KERNEL);
	if (!fdata->delta) {
		dev_err(&client->dev, "fail to alloc delta of fdata\n");
		ret = -ENOMEM;
		goto err_alloc_delta;
	}

	mutex_init(&fdata->cmd_lock);
	fdata->cmd_is_running = false;

	INIT_LIST_HEAD(&fdata->cmd_list_head);
	for (i = 0; i < ARRAY_SIZE(tsp_cmds); i++)
		list_add_tail(&tsp_cmds[i].list, &fdata->cmd_list_head);

	data->fdata = fdata;

	return 0;

err_alloc_delta:
	kfree(fdata->reference);
err_get_num_of_y:
err_get_num_of_x:
err_alloc_reference:
	kfree(fdata);
	return ret;
}

static void destroy_sec_fac(struct mxt_data *data)
{
	struct sec_fac_tsp *fdata = data->fdata;

	kfree(fdata->delta);
	kfree(fdata->reference);
	kfree(fdata);
}

static DEVICE_ATTR(cmd, S_IWUSR | S_IWGRP, NULL, cmd_store);
static DEVICE_ATTR(cmd_status, S_IRUGO, cmd_status_show, NULL);
static DEVICE_ATTR(cmd_result, S_IRUGO, cmd_result_show, NULL);

static struct attribute *fac_attributes[] = {
	&dev_attr_cmd.attr,
	&dev_attr_cmd_status.attr,
	&dev_attr_cmd_result.attr,
	NULL,
};

static struct attribute_group fac_attr_group = {
	.attrs = fac_attributes,
};
#endif

static int mxt_init_interface(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	int ret;

#if defined(ATMEL_DEBUG)
	ret = sysfs_create_group(&client->dev.kobj, &libmaxtouch_attr_group);
	if (ret) {
		dev_err(&client->dev,
			"failed to create libmaxtouch sysfs group (%d)\n", ret);
		return (ret < 0) ? ret : -ret;
	}

	sysfs_bin_attr_init(&mem_access_attr);
	mem_access_attr.attr.name = "mem_access";
	mem_access_attr.attr.mode = S_IRUGO | S_IWUGO;
	mem_access_attr.read = mem_access_read;
	mem_access_attr.write = mem_access_write;
	mem_access_attr.size = 65535;

	ret = sysfs_create_bin_file(&client->dev.kobj, &mem_access_attr);
	if (ret < 0) {
		dev_err(&client->dev, "failed to create device file (%s)\n",
			mem_access_attr.attr.name);
		goto err_create_mem_access;
	}
#endif

#if defined(MXT_DEBUG)
	ret = sysfs_create_group(&client->dev.kobj, &mxt_attr_group);
	if (ret) {
		dev_err(&client->dev,
			"failed to create mxt_attr_group sysfs group (%d)\n",
			ret);
		goto err_create_mxt_attr;
	}
#endif

#if defined(SEC_FAC_TSP)
	data->fdata->dummy_dev = device_create(sec_class, NULL, (dev_t)NULL,
					       data, "tsp");
	if (IS_ERR(data->fdata->dummy_dev)) {
		dev_err(&client->dev, "Failed to create fac tsp dummy dev\n");
		ret = -ENODEV;
		data->fdata->dummy_dev = NULL;
		goto err_create_sec_class_dev;
	}

	ret = sysfs_create_group(&data->fdata->dummy_dev->kobj,
				 &fac_attr_group);
	if (ret) {
		dev_err(&client->dev, "%s: failed to create fac_attr_group "
			"(%d)\n", __func__, ret);
		ret = (ret > 0) ? -ret : ret;
		goto err_create_fac_attr_group;
	}
#endif

	return 0;

#if defined(SEC_FAC_TSP)
	sysfs_remove_group(&data->fdata->dummy_dev->kobj, &fac_attr_group);
err_create_fac_attr_group:
	device_destroy(sec_class, (dev_t)NULL);
err_create_sec_class_dev:
#endif
#if defined(MXT_DEBUG)
	sysfs_remove_group(&client->dev.kobj, &mxt_attr_group);
err_create_mxt_attr:
#endif
#if defined(ATMEL_DEBUG)
	sysfs_remove_bin_file(&client->dev.kobj, &mem_access_attr);
err_create_mem_access:
	sysfs_remove_group(&client->dev.kobj, &libmaxtouch_attr_group);
#endif
	return (ret < 0) ? ret : -ret;
}

static void mxt_check_default_ta_status(unsigned long __data)
{
	struct mxt_data *data = (struct mxt_data *)__data;

	dev_err(&data->client->dev, "initial_cable_status=0x%x\n",
		*data->pdata->initial_cable_status);
	if (*data->pdata->initial_cable_status != 0) {
		data->dev_mode = TA_MODE;
		schedule_delayed_work(&data->wq, msecs_to_jiffies(100));
	}
}


#if defined(SEC_FAC_TSP)

static ssize_t fac_read_firm_version_phone(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	int ret;

	ret = sprintf(buf, "AT00%04x", fw_latest[0]);

	return ret;
}

static ssize_t fac_read_firm_version_panel(struct device *dev,
				      struct device_attribute *attr, char *buf)

{
	struct mxt_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int ret;
	u8 read_buf;

	set_default_result(data);

	ret = mxt_read_mem(data, 0X02, sizeof(u8), (u8 *)&read_buf);
	if (ret < 0) {
		dev_err(&client->dev, "fail to read fw ver of IC (%d)\n", ret);
	}
	ret = sprintf(buf, "AT00%04x", fw_latest[0]); /* temp */

/*	ret = sprintf(buf, "AT00%04x", read_buf); */

	return ret;
}



static DEVICE_ATTR(tsp_firm_version_phone, S_IRUGO, fac_read_firm_version_phone, NULL);
static DEVICE_ATTR(tsp_firm_version_panel, S_IRUGO, fac_read_firm_version_panel, NULL);


static struct attribute *fac_tsp_attributes[] = {
	&dev_attr_tsp_firm_version_phone.attr,
	&dev_attr_tsp_firm_version_panel.attr,
	NULL,
};

static struct attribute_group fac_tsp_attr_group = {
	.attrs = fac_tsp_attributes,
};

#endif


static int __devinit mxt_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	const struct mxt_platform_data *pdata = client->dev.platform_data;
	struct mxt_data *data;
	struct input_dev *input_dev;
	u16 boot_address;
	int ret;
	extern unsigned int lcd_type;
/*
	if (!lcd_type) {
		dev_err(&client->dev, "module is off.(%d)\n", lcd_type);
		return -ENODEV;
	}
*/
	if (!pdata) {
		dev_err(&client->dev, "missing platform data\n");
		return -EINVAL;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "not compatible i2c function\n");
		return -ENODEV;
	}

	data = kzalloc(sizeof(struct mxt_data) +
			pdata->num_of_fingers * sizeof(struct mxt_finger_info),
			GFP_KERNEL);
	if (!data) {
		dev_err(&client->dev, "fail to alloc device data\n");
		return -ENOMEM;
	}

	data->pdata = pdata;
	if (data->pdata->exit_flag) {
		dev_err(&client->dev, "exit flag is setted(%d)\n",
			data->pdata->exit_flag);
		ret = -ENODEV;
		goto exit_flag_set;
	}

	input_dev = input_allocate_device();
	if (!input_dev) {
		dev_err(&client->dev, "input device allocation failed\n");
		ret = -ENOMEM;
		goto err_alloc_input_dev;
	}

	data->client = client;
	data->input_dev = input_dev;

	data->num_of_fingers = pdata->num_of_fingers;
	data->finger_cnt = pdata->touchscreen_is_pressed;

	input_dev->name = "sec_touchscreen";
	input_dev->id.bustype = BUS_I2C;

	input_dev->dev.parent = &client->dev;


	set_bit(EV_ABS, input_dev->evbit);
	set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
	input_mt_init_slots(input_dev, data->num_of_fingers);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, pdata->min_x,
			     pdata->max_x, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, pdata->min_y,
			     pdata->max_y, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, pdata->min_w,
			     pdata->max_w, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, pdata->min_z,
			     pdata->max_z, 0, 0);
#if defined(SHAPETOUCH)
	input_set_abs_params(input_dev, ABS_MT_COMPONENT, 0, 255, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_SUMSIZE, 0, 11 * 19, 0, 0);
#endif

	input_set_drvdata(input_dev, data);
	/* regist input device */
	ret = input_register_device(input_dev);
	if (ret) {
		dev_err(&client->dev, "failed to register input dev (%d)\n",
			ret);
		goto err_reg_input_dev;
	}

	i2c_set_clientdata(client, data);

	/* regist dummy device for boot_address */
	if (data->pdata->boot_address) {
		boot_address = data->pdata->boot_address;
	} else {
		if (client->addr == MXT_APP_LOW)
			boot_address = MXT_BOOT_LOW;
		else
			boot_address = MXT_BOOT_HIGH;
	}
	data->client_boot = i2c_new_dummy(client->adapter, boot_address);
	if (!data->client_boot) {
		dev_err(&client->dev, "fail to get dummy i2c client 0x%X(%d)\n",
			boot_address, ret);
		ret = -ENOMEM;
		goto err_get_dummy_client;
	}

	mutex_init(&data->lock);

	ret = data->pdata->setup_power(&client->dev, true);
	if (ret < 0) {
		dev_err(&client->dev, "fail to setup power(%d)\n", ret);
		goto err_setup_power;
	}
	data->pdata->power_con(true);
	msleep(100);

#if defined(TOUCH_BOOSTER)
	prcmu_qos_add_requirement(PRCMU_QOS_APE_OPP, (char *)client->name,
				  PRCMU_QOS_DEFAULT_VALUE);
	prcmu_qos_add_requirement(PRCMU_QOS_DDR_OPP, (char *)client->name,
				  PRCMU_QOS_DEFAULT_VALUE);
	prcmu_qos_add_requirement(PRCMU_QOS_ARM_KHZ, (char *)client->name,
				  PRCMU_QOS_DEFAULT_VALUE);
	dev_info(&client->dev, "add_prcmu_qos is added\n");
#endif

	ret = request_threaded_irq(client->irq, NULL, mxt_irq_thread,
		IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "mxt_ts", data);
	if (ret < 0) {
		dev_err(&client->dev, "fail to request irq (%d)\n", ret);
		goto err_request_irq;
	}
	disable_irq(client->irq);

	ret = mxt_initialize(data, true);
	if (ret < 0) {
		dev_err(&client->dev, "fail to initialzie IC (%d)\n", ret);
		goto err_initialize;
	}

	enable_irq(client->irq);

	data->enabled = true;

	data->dev_mode = NORMAL_MODE;
	INIT_DELAYED_WORK(&data->wq, mxt_mode_transit_worker);

#ifdef CONFIG_USB_SWITCHER
	data->nb.notifier_call = mxt_usb_switch_notify;

	if (use_ab8505_iddet)
		micro_usb_register_usb_notifier(&data->nb);
	else
		usb_switch_register_notify(&data->nb);
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	data->early_suspend.suspend = mxt_early_suspend;
	data->early_suspend.resume = mxt_late_resume;
	register_early_suspend(&data->early_suspend);
#endif

#if defined(MXT_DEBUG)
	data->g_debug_switch = false;
#endif

#if defined(SEC_FAC_TSP)
	ret = mxt_sec_fac_init(data);
	if (ret < 0) {
		dev_err(&client->dev, "fail to init fdatea (%d)\n", ret);
		goto err_init_fdata;
	}
#endif
	ret = mxt_init_interface(data);
	if (ret < 0) {
		dev_err(&client->dev, "fail to init interface (%d)\n", ret);
		goto err_init_interface;
	}

	init_timer(&(data->ta_status_timer));
	data->ta_status_timer.expires = get_jiffies_64() + msecs_to_jiffies(5 * MSEC_PER_SEC);
	data->ta_status_timer.data = (unsigned long) data;
	data->ta_status_timer.function = mxt_check_default_ta_status;
	add_timer(&(data->ta_status_timer));

#if defined(SEC_FAC_TSP)

	sec_touchscreen_dev	= device_create(sec_class, NULL, 0, data, "sec_touchscreen");
	if (IS_ERR(sec_touchscreen_dev)) {
		dev_err(&client->dev, "Failed to create fac tsp temp dev\n");
	}

	ret = sysfs_create_group(&sec_touchscreen_dev->kobj, &fac_tsp_attr_group);
	if (ret)
		dev_err(&client->dev, "%s: failed to create fac_tsp_attr_group (%d)\n", __func__, ret);

#endif



	dev_info(&client->dev, "successfully probed.\n");

	return 0;

err_init_interface:
#if defined(SEC_FAC_TSP)
	destroy_sec_fac(data);
err_init_fdata:
#endif
err_initialize:
	mxt_uninitialize(data);
	free_irq(client->irq, data);
err_request_irq:
/*	data->pdata->setup_power(&client->dev, false);*/
err_setup_power:
	i2c_unregister_device(data->client_boot);
err_get_dummy_client:
	input_unregister_device(input_dev);
err_reg_input_dev:
	input_free_device(input_dev);
	input_dev = NULL;
err_alloc_input_dev:
exit_flag_set:
	kfree(data);
	return ret;
}

static int __devexit mxt_remove(struct i2c_client *client)
{
	struct mxt_data *data = i2c_get_clientdata(client);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&data->early_suspend);
#endif
	free_irq(client->irq, data);
	gpio_free(data->pdata->gpio_read_done);
	data->pdata->power_con(false);
	input_unregister_device(data->input_dev);
	i2c_unregister_device(data->client_boot);
	mxt_uninitialize(data);
#if defined(SEC_FAC_TSP)
	destroy_sec_fac(data);
#endif
	kfree(data);

	return 0;
}

static struct i2c_device_id mxt_idtable[] = {
	{MXT_DEV_NAME, 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, mxt_idtable);

#if defined(CONFIG_PM) && !defined(CONFIG_HAS_EARLYSUSPEND)
static const struct dev_pm_ops mxt_pm_ops = {
	.suspend = mxt_suspend,
	.resume = mxt_resume,
};
#endif

static struct i2c_driver mxt_i2c_driver = {
	.id_table = mxt_idtable,
	.probe = mxt_probe,
	.remove = __devexit_p(mxt_remove),
	.driver = {
		.owner	= THIS_MODULE,
		.name	= MXT_DEV_NAME,
#if defined(CONFIG_PM) && !defined(CONFIG_HAS_EARLYSUSPEND)
		.pm	= &mxt_pm_ops,
#endif
	},
};

static int __init mxt_init(void)
{
	return i2c_add_driver(&mxt_i2c_driver);
}

static void __exit mxt_exit(void)
{
	i2c_del_driver(&mxt_i2c_driver);
}
module_init(mxt_init);
module_exit(mxt_exit);

MODULE_DESCRIPTION("Atmel MaXTouch driver");
MODULE_LICENSE("GPL");
