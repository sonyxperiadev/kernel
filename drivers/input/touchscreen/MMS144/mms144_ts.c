/*
 * mms_ts.c - Touchscreen driver for Melfas MMS-series touch controllers
 *
 * Copyright (C) 2011 Google Inc.
 * Author: Dima Zavin <dima@android.com>
 *         Simon Wilson <simonwilson@google.com>
 *
 * ISP reflashing code based on original code from Melfas.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#define DEBUG
/* #define VERBOSE_DEBUG */
#define SEC_TSP_DEBUG

/* #define FORCE_FW_FLASH */
/* #define FORCE_FW_PASS */
/* #define ESD_DEBUG */

#define SEC_TSP_FACTORY_TEST
#define SEC_TSP_FW_UPDATE
#define TSP_BUF_SIZE 1024
#define FAIL -1
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/firmware.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <mach/gpio.h>
#include <linux/uaccess.h>

#include <linux/platform_data/mms144_ts.h>

#include <asm/unaligned.h>

#define MMS_HAS_TOUCH_KEY		1

#define MAX_FINGERS		10
#define MAX_WIDTH		30
#define MAX_PRESSURE		255
#define MAX_ANGLE		90
#define MIN_ANGLE		-90

/* Registers */
#define MMS_MODE_CONTROL	0x01
#define MMS_XYRES_HI		0x02
#define MMS_XRES_LO		0x03
#define MMS_YRES_LO		0x04

#define MMS_INPUT_EVENT_PKT_SZ	0x0F
#define MMS_INPUT_EVENT0	0x10
#define EVENT_SZ	8

#define MMS_CORE_VERSION	0xE1
#define MMS_TSP_REVISION	0xF0
#define MMS_HW_REVISION		0xF1
#define MMS_COMPAT_GROUP	0xF2
#define MMS_FW_VERSION		0xF4

enum {
	ISP_MODE_FLASH_ERASE = 0x59F3,
	ISP_MODE_FLASH_WRITE = 0x62CD,
	ISP_MODE_FLASH_READ = 0x6AC9,
};

/* each address addresses 4-byte words */
#define ISP_MAX_FW_SIZE		(0x1F00 * 4)
#define ISP_IC_INFO_ADDR	0x1F00
#define ISP_CAL_DATA_SIZE	256

#ifdef SEC_TSP_FW_UPDATE

#define WORD_SIZE		4

#define ISC_PKT_SIZE			1029
#define ISC_PKT_DATA_SIZE		1024
#define ISC_PKT_HEADER_SIZE		3
#define ISC_PKT_NUM				31

#define ISC_ENTER_ISC_CMD		0x5F
#define ISC_ENTER_ISC_DATA		0x01
#define ISC_CMD					0xAE
#define ISC_ENTER_UPDATE_DATA		0x55
#define ISC_ENTER_UPDATE_DATA_LEN	9
#define ISC_DATA_WRITE_SUB_CMD		0xF1
#define ISC_EXIT_ISC_SUB_CMD		0x0F
#define ISC_EXIT_ISC_SUB_CMD2		0xF0
#define ISC_CHECK_STATUS_CMD		0xAF
#define ISC_CONFIRM_CRC			0x03
#define ISC_DEFAULT_CRC			0xFFFF

#endif

#ifdef SEC_TSP_FACTORY_TEST
#define TX_NUM		26
#define RX_NUM		14
#define NODE_NUM	510	/* 30x17 */

/* self diagnostic */
#define ADDR_CH_NUM		0x0B
#define ADDR_UNIV_CMD	0xA0
#define CMD_ENTER_TEST	0x40
#define CMD_EXIT_TEST	0x4F
#define CMD_CM_DELTA	0x41
#define CMD_GET_DELTA	0x42
#define CMD_CM_ABS		0X43
#define CMD_GET_ABS		0X44
#define CMD_CM_JITTER	0X45
#define CMD_GET_JITTER	0X46
#define CMD_GET_INTEN	0x70
#define CMD_RESULT_SZ	0XAE
#define CMD_RESULT		0XAF

/* VSC(Vender Specific Command)  */
#define MMS_VSC_CMD			0xB0	/* vendor specific command */
#define MMS_VSC_MODE			0x1A	/* mode of vendor */

#define MMS_VSC_CMD_ENTER		0X01
#define MMS_VSC_CMD_CM_DELTA		0X02
#define MMS_VSC_CMD_CM_ABS		0X03
#define MMS_VSC_CMD_EXIT		0X05
#define MMS_VSC_CMD_INTENSITY		0X04
#define MMS_VSC_CMD_RAW			0X06
#define MMS_VSC_CMD_REFER		0X07

#define TSP_CMD_STR_LEN 32
#define TSP_CMD_RESULT_STR_LEN 512
#define TSP_CMD_PARAM_NUM 8
#endif /* SEC_TSP_FACTORY_TEST */

/* Touch booster */
#if defined(CONFIG_EXYNOS4_CPUFREQ) &&\
	defined(CONFIG_BUSFREQ_OPP)
#define TOUCH_BOOSTER			1
#define TOUCH_BOOSTER_OFF_TIME		100
#define TOUCH_BOOSTER_CHG_TIME		200
#else
#define TOUCH_BOOSTER			0
#endif

struct device *sec_touchscreen;
static struct device *bus_dev;

int touch_is_pressed;



/* Event types */
#define MMS_LOG_EVENT			0xD
#define MMS_NOTIFY_EVENT		0xE
#define MMS_ERROR_EVENT			0xF
#define MMS_TOUCH_KEY_EVENT		0x40



#define ISC_DL_MODE	1

/* 5.55" OCTA LCD */
#define FW_VERSION 0x12
#define BOOT_VERSION 0x5
#define CORE_VERSION 0x54
#define MAX_FW_PATH 255
#define TSP_FW_FILENAME "melfas_fw.bin"

#if ISC_DL_MODE	/* ISC_DL_MODE start */

/*
 *      Default configuration of ISC mode
 */
#define DEFAULT_SLAVE_ADDR	0x48

#define SECTION_NUM		3
#define SECTION_NAME_LEN	5

#define PAGE_HEADER		3
#define PAGE_DATA		1024
#define PAGE_TAIL		2
#define PACKET_SIZE		(PAGE_HEADER + PAGE_DATA + PAGE_TAIL)
#define TS_WRITE_REGS_LEN		1030

#define TIMEOUT_CNT		10
#define STRING_BUF_LEN		100


/* State Registers */
#define MIP_ADDR_INPUT_INFORMATION	0x01

#define ISC_ADDR_VERSION		0xE1
#define ISC_ADDR_SECTION_PAGE_INFO	0xE5

/* Config Update Commands */
#define ISC_CMD_ENTER_ISC		0x5F
#define ISC_CMD_ENTER_ISC_PARA1		0x01
#define ISC_CMD_UPDATE_MODE		0xAE
#define ISC_SUBCMD_ENTER_UPDATE		0x55
#define ISC_SUBCMD_DATA_WRITE		0XF1
#define ISC_SUBCMD_LEAVE_UPDATE_PARA1	0x0F
#define ISC_SUBCMD_LEAVE_UPDATE_PARA2	0xF0
#define ISC_CMD_CONFIRM_STATUS		0xAF

#define ISC_STATUS_UPDATE_MODE		0x01
#define ISC_STATUS_CRC_CHECK_SUCCESS	0x03

#define ISC_CHAR_2_BCD(num)	(((num/10)<<4) + (num%10))
#define ISC_MAX(x, y)		(((x) > (y)) ? (x) : (y))

static const char section_name[SECTION_NUM][SECTION_NAME_LEN] = {
	"BOOT", "CORE", "CONF"
};

static const unsigned char crc0_buf[31] = {
	0x1D, 0x2C, 0x05, 0x34, 0x95, 0xA4, 0x8D, 0xBC,
	0x59, 0x68, 0x41, 0x70, 0xD1, 0xE0, 0xC9, 0xF8,
	0x3F, 0x0E, 0x27, 0x16, 0xB7, 0x86, 0xAF, 0x9E,
	0x7B, 0x4A, 0x63, 0x52, 0xF3, 0xC2, 0xEB
};

static const unsigned char crc1_buf[31] = {
	0x1E, 0x9C, 0xDF, 0x5D, 0x76, 0xF4, 0xB7, 0x35,
	0x2A, 0xA8, 0xEB, 0x69, 0x42, 0xC0, 0x83, 0x01,
	0x04, 0x86, 0xC5, 0x47, 0x6C, 0xEE, 0xAD, 0x2F,
	0x30, 0xB2, 0xF1, 0x73, 0x58, 0xDA, 0x99
};

enum {
	ISC_NONE = -1,
	ISC_SUCCESS = 0,
	ISC_FILE_OPEN_ERROR,
	ISC_FILE_CLOSE_ERROR,
	ISC_FILE_FORMAT_ERROR,
	ISC_WRITE_BUFFER_ERROR,
	ISC_I2C_ERROR,
	ISC_UPDATE_MODE_ENTER_ERROR,
	ISC_CRC_ERROR,
	ISC_VALIDATION_ERROR,
	ISC_COMPATIVILITY_ERROR,
	ISC_UPDATE_SECTION_ERROR,
	ISC_SLAVE_ERASE_ERROR,
	ISC_SLAVE_DOWNLOAD_ERROR,
	ISC_DOWNLOAD_WHEN_SLAVE_IS_UPDATED_ERROR,
	ISC_INITIAL_PACKET_ERROR,
	ISC_NO_NEED_UPDATE_ERROR,
	ISC_LIMIT
};

enum {
	EC_NONE = -1,
	EC_DEPRECATED = 0,
	EC_BOOTLOADER_RUNNING = 1,
	EC_BOOT_ON_SUCCEEDED = 2,
	EC_ERASE_END_MARKER_ON_SLAVE_FINISHED = 3,
	EC_SLAVE_DOWNLOAD_STARTS = 4,
	EC_SLAVE_DOWNLOAD_FINISHED = 5,
	EC_2CHIP_HANDSHAKE_FAILED = 0x0E,
	EC_ESD_PATTERN_CHECKED = 0x0F,
	EC_LIMIT
};

enum {
	SEC_NONE = -1,
	SEC_BOOTLOADER = 0,
	SEC_CORE,
	SEC_CONFIG,
	SEC_LIMIT
};

struct tISCFWInfo_t {
	unsigned char version;
	unsigned char compatible_version;
	unsigned char start_addr;
	unsigned char end_addr;
	int bin_offset;
	u32 crc;
};

static struct tISCFWInfo_t mbin_info[SECTION_NUM];
static struct tISCFWInfo_t ts_info[SECTION_NUM];
static bool section_update_flag[SECTION_NUM];

const struct firmware *fw_mbin[SECTION_NUM];

static unsigned char g_wr_buf[1024 + 3 + 2];
#endif

enum fw_flash_mode {
	ISP_FLASH,
	ISC_FLASH,
};

enum {
	BUILT_IN = 0,
	UMS,
	REQ_FW,
};

struct tsp_callbacks {
	void (*inform_charger)(struct tsp_callbacks *tsp_cb, bool mode);
};

#ifdef CONFIG_LCD_FREQ_SWITCH
struct tsp_lcd_callbacks {
	void (*inform_lcd)(struct tsp_lcd_callbacks *tsp_cb, bool en);
};
#endif

struct mms_ts_info {
	struct i2c_client *client;
	struct input_dev *input_dev;
	char phys[32];
	int max_x;
	int max_y;
	bool invert_x;
	bool invert_y;
	u8 palm_flag;
	int irq;
	int (*power) (bool on);
	void (*input_event)(void *data);
	struct melfas_tsi_platform_data *pdata;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
#if TOUCH_BOOSTER
	struct delayed_work work_dvfs_off;
	struct delayed_work work_dvfs_chg;
	bool dvfs_lock_status;
	int cpufreq_level;
	struct mutex dvfs_lock;
#endif
	bool irq_check;
	/* protects the enabled flag */
	struct mutex lock;
	bool enabled;

	enum fw_flash_mode fw_flash_mode;
	void (*register_cb)(void *);
	struct tsp_callbacks callbacks;
#ifdef CONFIG_LCD_FREQ_SWITCH
	void (*register_lcd_cb)(void *);
	struct tsp_lcd_callbacks lcd_callback;
	bool tsp_lcdfreq_flag;
#endif
	bool ta_status;
	bool noise_mode;
	bool sleep_wakeup_ta_check;

#if defined(SEC_TSP_DEBUG)
	unsigned char finger_state[MAX_FINGERS];
#endif

#if defined(SEC_TSP_FW_UPDATE)
	u8 fw_update_state;
#endif
	u8 fw_ic_ver;
	u8 panel;
	u8 fw_core_ver;
#if defined(SEC_TSP_FACTORY_TEST)
	struct list_head cmd_list_head;
	u8 cmd_state;
	char cmd[TSP_CMD_STR_LEN];
	int cmd_param[TSP_CMD_PARAM_NUM];
	char cmd_result[TSP_CMD_RESULT_STR_LEN];
	struct mutex cmd_lock;
	bool cmd_is_running;

	unsigned int reference[NODE_NUM];
	unsigned int raw[NODE_NUM]; /* CM_ABS */
	unsigned int inspection[NODE_NUM];/* CM_DELTA */
	unsigned int intensity[NODE_NUM];
	bool ft_flag;
#endif				/* SEC_TSP_FACTORY_TEST */

	int (*lcd_type) (void);
};

struct mms_fw_image {
	__le32 hdr_len;
	__le32 data_len;
	__le32 fw_ver;
	__le32 hdr_ver;
	u8 data[0];
} __packed;

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mms_ts_early_suspend(struct early_suspend *h);
static void mms_ts_late_resume(struct early_suspend *h);
#endif
static int mms_ts_suspend(struct device *dev);
static int mms_ts_resume(struct device *dev);

#if defined(SEC_TSP_FACTORY_TEST)
#define TSP_CMD(name, func) .cmd_name = name, .cmd_func = func

struct tsp_cmd {
	struct list_head	list;
	const char	*cmd_name;
	void	(*cmd_func)(void *device_data);
};

static void fw_update(void *device_data);
static void get_fw_ver_bin(void *device_data);
static void get_fw_ver_ic(void *device_data);
static void get_config_ver(void *device_data);
static void get_threshold(void *device_data);
static void module_off_master(void *device_data);
static void module_on_master(void *device_data);
/*static void module_off_slave(void *device_data);
static void module_on_slave(void *device_data);*/
static void get_chip_vendor(void *device_data);
static void get_chip_name(void *device_data);
static void get_reference(void *device_data);
static void get_cm_abs(void *device_data);
static void get_cm_delta(void *device_data);
static void get_intensity(void *device_data);
static void get_x_num(void *device_data);
static void get_y_num(void *device_data);
static void run_reference_read(void *device_data);
static void run_cm_abs_read(void *device_data);
static void run_cm_delta_read(void *device_data);
static void run_intensity_read(void *device_data);
static void not_support_cmd(void *device_data);

struct tsp_cmd tsp_cmds[] = {
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
	{TSP_CMD("get_reference", get_reference),},
	{TSP_CMD("get_cm_abs", get_cm_abs),},
	{TSP_CMD("get_cm_delta", get_cm_delta),},
	{TSP_CMD("get_intensity", get_intensity),},
	{TSP_CMD("run_reference_read", run_reference_read),},
	{TSP_CMD("run_cm_abs_read", run_cm_abs_read),},
	{TSP_CMD("run_cm_delta_read", run_cm_delta_read),},
	{TSP_CMD("run_intensity_read", run_intensity_read),},
	{TSP_CMD("not_support_cmd", not_support_cmd),},
};
#endif

#if TOUCH_BOOSTER
static void change_dvfs_lock(struct work_struct *work)
{
	struct mms_ts_info *info = container_of(work,
				struct mms_ts_info, work_dvfs_chg.work);
	int ret;

	mutex_lock(&info->dvfs_lock);

	ret = dev_lock(bus_dev, sec_touchscreen, 267160); /* 266 Mhz setting */

	if (ret < 0)
		pr_err("%s: dev change bud lock failed(%d)\n",\
				__func__, __LINE__);
	else
		pr_info("[TSP] change_dvfs_lock");
	mutex_unlock(&info->dvfs_lock);
}
static void set_dvfs_off(struct work_struct *work)
{

	struct mms_ts_info *info = container_of(work,
				struct mms_ts_info, work_dvfs_off.work);
	int ret;

	mutex_lock(&info->dvfs_lock);

	ret = dev_unlock(bus_dev, sec_touchscreen);
	if (ret < 0)
			pr_err("%s: dev unlock failed(%d)\n",
			       __func__, __LINE__);

	exynos_cpufreq_lock_free(DVFS_LOCK_ID_TSP);
	info->dvfs_lock_status = false;
	pr_info("[TSP] DVFS Off!");
	mutex_unlock(&info->dvfs_lock);
	}

static void set_dvfs_lock(struct mms_ts_info *info, uint32_t on)
{
	int ret;

	mutex_lock(&info->dvfs_lock);
	if (info->cpufreq_level <= 0) {
		ret = exynos_cpufreq_get_level(800000, &info->cpufreq_level);
		if (ret < 0)
			pr_err("[TSP] exynos_cpufreq_get_level error");
		goto out;
	}
	if (on == 0) {
		if (info->dvfs_lock_status) {
			cancel_delayed_work(&info->work_dvfs_chg);
			schedule_delayed_work(&info->work_dvfs_off,
				msecs_to_jiffies(TOUCH_BOOSTER_OFF_TIME));
		}

	} else if (on == 1) {
		cancel_delayed_work(&info->work_dvfs_off);
		if (!info->dvfs_lock_status) {
			ret = dev_lock(bus_dev, sec_touchscreen, 400200);
			if (ret < 0) {
				pr_err("%s: dev lock failed(%d)\n",\
							__func__, __LINE__);
			}

			ret = exynos_cpufreq_lock(DVFS_LOCK_ID_TSP,
							info->cpufreq_level);
			if (ret < 0)
				pr_err("%s: cpu lock failed(%d)\n",\
							__func__, __LINE__);

			schedule_delayed_work(&info->work_dvfs_chg,
				msecs_to_jiffies(TOUCH_BOOSTER_CHG_TIME));

			info->dvfs_lock_status = true;
			pr_info("[TSP] DVFS On![%d]", info->cpufreq_level);
		}
	} else if (on == 2) {
		cancel_delayed_work(&info->work_dvfs_off);
		cancel_delayed_work(&info->work_dvfs_chg);
		schedule_work(&info->work_dvfs_off.work);
	}
out:
	mutex_unlock(&info->dvfs_lock);
}

#endif

static inline void mms_pwr_on_reset(struct mms_ts_info *info)
{
	struct i2c_adapter *adapter = to_i2c_adapter(info->client->dev.parent);

	if (!info->pdata->mux_fw_flash) {
		dev_info(&info->client->dev,
			 "missing platform data, can't do power-on-reset\n");
		return;
	}

	i2c_lock_adapter(adapter);
	info->pdata->mux_fw_flash(true);

	info->pdata->power(0);
//	gpio_direction_output(info->pdata->gpio_sda, 0);
//	gpio_direction_output(info->pdata->gpio_scl, 0);
//	gpio_direction_output(info->pdata->gpio_int, 0);
	msleep(50);
	info->pdata->power(1);
	msleep(200);

	info->pdata->mux_fw_flash(false);
	i2c_unlock_adapter(adapter);

	/* TODO: Seems long enough for the firmware to boot.
	 * Find the right value */
	msleep(250);
}
static void release_all_fingers(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;
	int i;

	dev_dbg(&client->dev, "[TSP] %s\n", __func__);

	for (i = 0; i < MAX_FINGERS; i++) {
#ifdef SEC_TSP_DEBUG
		if (info->finger_state[i] == 1)
			dev_notice(&client->dev, "finger %d up(force)\n", i);
#endif
		info->finger_state[i] = 0;
		input_mt_slot(info->input_dev, i);
		input_mt_report_slot_state(info->input_dev, MT_TOOL_FINGER,
					   false);
	}
	input_sync(info->input_dev);
#if TOUCH_BOOSTER
	set_dvfs_lock(info, 2);
	pr_info("[TSP] dvfs_lock free.\n ");
#endif
}

static void mms_set_noise_mode(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;

      return ; /* 20130913 TEMP */

	if (!(info->noise_mode && info->enabled))
		return;
	dev_notice(&client->dev, "%s\n", __func__);

	if (info->ta_status) {
		dev_notice(&client->dev, "noise_mode & TA connect!!!\n");
		i2c_smbus_write_byte_data(info->client, 0x30, 0x1);
	} else {
		dev_notice(&client->dev, "noise_mode & TA disconnect!!!\n");
		i2c_smbus_write_byte_data(info->client, 0x30, 0x2);
		info->noise_mode = 0;
	}
}

static void reset_mms_ts(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;

	if (info->enabled == false)
		return;

	dev_notice(&client->dev, "%s++\n", __func__);
	disable_irq_nosync(info->irq);
	info->enabled = false;
	touch_is_pressed = 0;

	release_all_fingers(info);

	mms_pwr_on_reset(info);

	info->enabled = true;
	if (info->ta_status) {
		dev_notice(&client->dev, "TA connect!!!\n");
		i2c_smbus_write_byte_data(info->client, 0x33, 0x1);
	} else {
		dev_notice(&client->dev, "TA disconnect!!!\n");
		i2c_smbus_write_byte_data(info->client, 0x33, 0x2);
	}
	mms_set_noise_mode(info);

	enable_irq(info->irq);

	dev_notice(&client->dev, "%s--\n", __func__);
}

static void melfas_ta_cb(struct tsp_callbacks *cb, bool ta_status)
{
	struct mms_ts_info *info =
			container_of(cb, struct mms_ts_info, callbacks);
	struct i2c_client *client = info->client;

	dev_notice(&client->dev, "%s\n", __func__);

	info->ta_status = ta_status;


      printk("%s  ,   %d \n",__func__, __LINE__);

	if (info->enabled) {
		if (info->ta_status) {
			dev_notice(&client->dev, "TA connect!!!\n");
			i2c_smbus_write_byte_data(info->client, 0x33, 0x1);
		} else {
			dev_notice(&client->dev, "TA disconnect!!!\n");
			i2c_smbus_write_byte_data(info->client, 0x33, 0x2);
		}
		mms_set_noise_mode(info);
	}

/*	if (!ta_status)
		mms_set_noise_mode(info);
*/
}

#ifdef CONFIG_LCD_FREQ_SWITCH
static void melfas_lcd_cb(struct tsp_lcd_callbacks *cb, bool en)
{
	struct mms_ts_info *info =
			container_of(cb, struct mms_ts_info, lcd_callback);

	if (info->enabled == false) {
		dev_err(&info->client->dev,
			"[TSP] do not excute %s.(touch off)\n", __func__);
		return;
	}

	if (info->fw_ic_ver < 0x21) {
		dev_err(&info->client->dev,
			"[TSP] Do not support firmware LCD framerate changing(ver = 0x%x)\n",
			info->fw_ic_ver);
		return;
	}

	if (en) {
		if (info->tsp_lcdfreq_flag == 0) {
			info->tsp_lcdfreq_flag = 1;
			dev_info(&info->client->dev, "[TSP] LCD framerate to 40 Hz\n");
			i2c_smbus_write_byte_data(info->client, 0x34, 0x1);
		}
	} else {
		if (info->tsp_lcdfreq_flag == 1) {
			info->tsp_lcdfreq_flag = 0;
			dev_info(&info->client->dev, "[TSP] LCD framreate to 60 Hz\n");
			i2c_smbus_write_byte_data(info->client, 0x34, 0x1);
		}
	}
}
#endif

static irqreturn_t mms_ts_interrupt(int irq, void *dev_id)
{
	struct mms_ts_info *info = dev_id;
	struct i2c_client *client = info->client;
	int ret;
	int i;
	int sz;

	u8 buf[MAX_FINGERS * EVENT_SZ] = { 0 };
	u8 reg = MMS_INPUT_EVENT0;
	struct i2c_msg msg[] = {
		{
			.addr = client->addr,
			.flags = 0,
			.buf = &reg,
			.len = 1,
		}, {
			.addr = client->addr,
			.flags = I2C_M_RD,
			.buf = buf,
		},
	};

	sz = i2c_smbus_read_byte_data(client, MMS_INPUT_EVENT_PKT_SZ);
	if (sz < 0) {
		dev_err(&client->dev, "%s bytes=%d\n", __func__, sz);
		for (i = 0; i < 50; i++) {
			sz = i2c_smbus_read_byte_data(client,
						      MMS_INPUT_EVENT_PKT_SZ);
			if (sz > 0)
				break;
		}

		if (i == 50) {
			dev_dbg(&client->dev, "i2c failed... reset!!\n");
			reset_mms_ts(info);
			goto out;
		}
	}

	if (sz == 0)
		goto out;

	if (sz > MAX_FINGERS * EVENT_SZ) {
		dev_err(&client->dev, "[TSP] abnormal data inputed.\n");
		goto out;
	}

	msg[1].len = sz;
	ret = i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg));
	if (ret != ARRAY_SIZE(msg)) {
		dev_err(&client->dev,
			"failed to read %d bytes of touch data (%d)\n",
			sz, ret);
		if (ret < 0) {
			dev_err(&client->dev,
				"%s bytes=%d\n", __func__, sz);
			for (i = 0; i < 5; i++) {
				msleep(20);
				ret = i2c_transfer(client->adapter,
					msg, ARRAY_SIZE(msg));
				if (ret > 0)
					break;
			}
			if (i == 5) {
				dev_dbg(&client->dev,
					"[TSP] i2c failed E2... reset!!\n");
				reset_mms_ts(info);
				goto out;
			}
		}
	}
#if defined(VERBOSE_DEBUG)
	print_hex_dump(KERN_DEBUG, "mms_ts raw: ",
		       DUMP_PREFIX_OFFSET, 32, 1, buf, sz, false);

#endif
	if (buf[0] == 0x0F) {	/* ESD */
		dev_dbg(&client->dev, "ESD DETECT.... reset!!\n");
		reset_mms_ts(info);
		goto out;
	}

	if (buf[0] == 0x0E) { /* NOISE MODE */
		dev_dbg(&client->dev, "[TSP] noise mode enter!!\n");
		info->noise_mode = 1;
		mms_set_noise_mode(info);
		goto out;
	}

	touch_is_pressed = 0;
	for (i = 0; i < sz; i += EVENT_SZ) {
		u8 *tmp = &buf[i];
#if MMS_HAS_TOUCH_KEY
		int key_code;
		int key_state;
#endif
		if (tmp[0] & MMS_TOUCH_KEY_EVENT) {
			switch (tmp[0] & 0xf) {
			case 1:
				key_code = KEY_MENU;
				break;
			case 2:
				key_code = KEY_BACK;
				break;
			default:
				dev_err(&client->dev, "unknown key type\n");
				goto out;
				break;
			}

			key_state = (tmp[0] & 0x80) ? 1 : 0;
			input_report_key(info->input_dev, key_code, key_state);

		} else {
		int id = (tmp[0] & 0xf) - 1;
		int x = tmp[2] | ((tmp[1] & 0xf) << 8);
		int y = tmp[3] | ((tmp[1] >> 4) << 8);
		int angle = 0;
		int palm = 0;
	
		angle = (tmp[5] >= 127) ? (-(256 - tmp[5])) : tmp[5];
		palm = (tmp[0] & 0x10) >> 4;

		if (info->invert_x) {
			x = info->max_x - x;
			if (x < 0)
				x = 0;
		}
		if (info->invert_y) {
			y = info->max_y - y;
			if (y < 0)
				y = 0;
		}
		if (palm) {
			if (info->palm_flag == 3) {
				info->palm_flag = 1;
			} else {
				info->palm_flag = 3;
				palm = 3;
			}
		} else {
			if (info->palm_flag == 2) {
				info->palm_flag = 0;
			} else {
				info->palm_flag = 2;
				palm = 2;
			}
		}
		if (id >= MAX_FINGERS) {
			dev_notice(&client->dev,
				"finger id error [%d]\n", id);
			reset_mms_ts(info);  /* 130913 to avoid lock up */
			goto out;
		}

		if (x == 0 && y == 0)
			continue;

		if ((tmp[0] & 0x80) == 0) {
			input_mt_slot(info->input_dev, id);
			input_mt_report_slot_state(info->input_dev,
						   MT_TOOL_FINGER, false);
#ifdef CONFIG_SAMSUNG_PRODUCT_SHIP
			if (info->finger_state[id] != 0) {
				info->finger_state[id] = 0;
#ifdef CONFIG_LCD_FREQ_SWITCH
				dev_notice(&client->dev,
					"R (%d) [%2d]",
				(info->tsp_lcdfreq_flag ? 40 : 60),
					id);
#else
				dev_notice(&client->dev,
					"R [%2d]", id);
#endif
			}
#else
			if (info->finger_state[id] != 0) {
				info->finger_state[id] = 0;
#ifdef CONFIG_LCD_FREQ_SWITCH
				dev_notice(&client->dev,
					"R(%d) [%2d],([%4d],[%3d]),S:%d W:%d",
				(info->tsp_lcdfreq_flag ? 40 : 60),
					id, x, y, tmp[4], tmp[5]);
#else
				dev_notice(&client->dev,
					"R [%2d],([%4d],[%3d]),S:%d W:%d",
					id, x, y, tmp[4], tmp[5]);
#endif
			}

#endif /*CONFIG_SAMSUNG_PRODUCT_SHIP */
			continue;
		}

		input_mt_slot(info->input_dev, id);
		input_mt_report_slot_state(info->input_dev,
					   MT_TOOL_FINGER, true);
		input_report_abs(info->input_dev,
			ABS_MT_POSITION_X, x);
		input_report_abs(info->input_dev,
			ABS_MT_POSITION_Y, y);
		input_report_abs(info->input_dev,
			ABS_MT_WIDTH_MAJOR, tmp[4]);
		input_report_abs(info->input_dev,
			ABS_MT_TOUCH_MAJOR, tmp[6]);
		input_report_abs(info->input_dev,
			ABS_MT_TOUCH_MINOR, tmp[7]);
		input_report_abs(info->input_dev,
			ABS_MT_ANGLE, angle);
		input_report_abs(info->input_dev,
			ABS_MT_PALM, palm);
#ifdef CONFIG_SAMSUNG_PRODUCT_SHIP
		if (info->finger_state[id] == 0) {
			info->finger_state[id] = 1;
#ifdef CONFIG_LCD_FREQ_SWITCH
			dev_notice(&client->dev,
				"P(%d) [%2d]",
				(info->tsp_lcdfreq_flag ? 40 : 60), id);

#else
			dev_notice(&client->dev,
				"P [%2d]", id);
#endif
		}
#else /* CONFIG_SAMSUNG_PRODUCT_SHIP */
		if (info->finger_state[id] == 0) {
			info->finger_state[id] = 1;
#ifdef CONFIG_LCD_FREQ_SWITCH
			dev_notice(&client->dev,
				"P(%d) [%2d],([%4d],[%3d]) w=%d, major=%d, minor=%d, angle=%d, palm=%d",
				(info->tsp_lcdfreq_flag ? 40 : 60),
				id, x, y, tmp[4], tmp[6],
				tmp[7], angle, palm);
#else
			dev_notice(&client->dev,
				"P [%2d],([%4d],[%3d]) w=%d, major=%d, minor=%d, angle=%d, palm=%d",
				id, x, y, tmp[4], tmp[6],
				tmp[7], angle, palm);
#endif
		}
#endif /* CONFIG_SAMSUNG_PRODUCT_SHIP */
		touch_is_pressed++;
	}
	}
	input_sync(info->input_dev);

#if TOUCH_BOOSTER
	set_dvfs_lock(info, !!touch_is_pressed);
#endif

out:
	return IRQ_HANDLED;
}

int get_tsp_status(void)
{
	return touch_is_pressed;
}
EXPORT_SYMBOL(get_tsp_status);


#if ISC_DL_MODE
static int mms100_i2c_read(struct i2c_client *client,
		u16 addr, u16 length, u8 *value)
{
	struct i2c_adapter *adapter = client->adapter;
	struct i2c_msg msg;
	int ret = -1;

	msg.addr = client->addr;
	msg.flags = 0x00;
	msg.len = 1;
	msg.buf = (u8 *) &addr;

	ret = i2c_transfer(adapter, &msg, 1);

	if (ret >= 0) {
		msg.addr = client->addr;
		msg.flags = I2C_M_RD;
		msg.len = length;
		msg.buf = (u8 *) value;

		ret = i2c_transfer(adapter, &msg, 1);
	}

	if (ret < 0)
		pr_err("[TSP] : read error : [%d]", ret);

	return ret;
}


static int mms100_reset(struct mms_ts_info *info)
{
	info->pdata->power(false);
	msleep(30);
	info->pdata->power(true);
	msleep(300);

	return ISC_SUCCESS;
}
/*
static int mms100_check_operating_mode(struct i2c_client *_client,
		const int _error_code)
{
	int ret;
	unsigned char rd_buf = 0x00;
	unsigned char count = 0;

	if(_client == NULL)
		pr_err("[TSP ISC] _client is null");

	ret = mms100_i2c_read(_client, ISC_ADDR_VERSION, 1, &rd_buf);

	if (ret<0) {
		pr_info("[TSP ISC] %s,%d: i2c read fail[%d]\n",
			__func__, __LINE__, ret);
		return _error_code;
	}

	return ISC_SUCCESS;
}
*/
static int mms100_get_version_info(struct i2c_client *_client)
{
	int i, ret;
	unsigned char rd_buf[8];

	/* config version brust read (core, private, public) */
	ret = mms100_i2c_read(_client, ISC_ADDR_VERSION, 3, rd_buf);
	if (ret < 0) {
		pr_info("[TSP ISC] %s,%d: i2c read fail[%d]\n",
			__func__, __LINE__, ret);
		return ISC_I2C_ERROR;
	}

      printk("~~~~~~~~~FW version is %x\n", rd_buf[2]);

	for (i = 0; i < SECTION_NUM; i++)
		ts_info[i].version = rd_buf[i];

	ts_info[SEC_CORE].compatible_version =
		ts_info[SEC_BOOTLOADER].version;
	ts_info[SEC_CONFIG].compatible_version =
		ts_info[SEC_CORE].version;

	ret = mms100_i2c_read(_client, ISC_ADDR_SECTION_PAGE_INFO, 8, rd_buf);
	if (ret < 0) {
		pr_info("[TSP ISC] %s,%d: i2c read fail[%d]\n",
			__func__, __LINE__, ret);
		return ISC_I2C_ERROR;
	}

	for (i = 0; i < SECTION_NUM; i++) {
		ts_info[i].start_addr = rd_buf[i];
		ts_info[i].end_addr = rd_buf[i + SECTION_NUM + 1];
	}

	for (i = 0; i < SECTION_NUM; i++) {
		pr_info("TS : Section(%d) version: 0x%02X\n",
			i, ts_info[i].version);
		pr_info("TS : Section(%d) Start Address: 0x%02X\n",
			i, ts_info[i].start_addr);
		pr_info("TS : Section(%d) End Address: 0x%02X\n",
			i, ts_info[i].end_addr);
		pr_info("TS : Section(%d) Compatibility: 0x%02X\n",
			i, ts_info[i].compatible_version);
	}

	return ISC_SUCCESS;
}

static int mms100_seek_section_info(void)
{
	int i;
	char str_buf[STRING_BUF_LEN];
	char name_buf[SECTION_NAME_LEN];
	int version;
	int page_num;
	int offset[SECTION_NUM] = {0x03f0, 0x53f0 - 0x0400, 0x7bf0 - 0x5400};
	const unsigned char *buf;
	int next_ptr;
	char *bin_header = "[Binary]";
	int cnt = strlen(bin_header);
	int j;

	for (i = 0; i < SECTION_NUM; i++) {
		if (fw_mbin[i] == NULL) {
			buf = NULL;
			pr_info("[TSP ISC] fw_mbin[%d]->data is NULL", i);
		} else {
			buf = fw_mbin[i]->data;
		}

		if (buf == NULL) {
			mbin_info[i].version = ts_info[i].version;
			mbin_info[i].compatible_version =
				ts_info[i].compatible_version;
			mbin_info[i].start_addr = ts_info[i].start_addr;
			mbin_info[i].end_addr = ts_info[i].end_addr;
		} else {
			next_ptr = 0;

			do {
				sscanf(buf + next_ptr, "%99s", str_buf);
				next_ptr += strlen(str_buf) + 1;
			} while (!strstr(str_buf, "SECTION_NAME"));

			sscanf(buf + next_ptr, "%99s%4s", str_buf, name_buf);

			if (strncmp(section_name[i], name_buf,
				SECTION_NAME_LEN))
				return ISC_FILE_FORMAT_ERROR;

			do {
				sscanf(buf + next_ptr, "%99s", str_buf);
				next_ptr += strlen(str_buf) + 1;
			} while (!strstr(str_buf, "SECTION_VERSION"));

			sscanf(buf + next_ptr, "%99s%d", str_buf, &version);
			mbin_info[i].version = ISC_CHAR_2_BCD(version);

			do {
				sscanf(buf + next_ptr, "%99s", str_buf);
				next_ptr += strlen(str_buf) + 1;
			} while (!strstr(str_buf, "START_PAGE_ADDR"));

			sscanf(buf + next_ptr, "%99s%d", str_buf, &page_num);
			mbin_info[i].start_addr = page_num;

			do {
				sscanf(buf + next_ptr, "%99s", str_buf);
				next_ptr += strlen(str_buf) + 1;
			} while (!strstr(str_buf, "END_PAGE_ADDR"));

			sscanf(buf + next_ptr, "%99s%d", str_buf, &page_num);
			mbin_info[i].end_addr = page_num;

			do {
				sscanf(buf + next_ptr, "%99s", str_buf);
				next_ptr += strlen(str_buf) + 1;
			} while (!strstr(str_buf, "COMPATIBLE_VERSION"));

			sscanf(buf + next_ptr, "%99s%d", str_buf, &version);
			mbin_info[i].compatible_version =
				ISC_CHAR_2_BCD(version);
#if 0
			do {
				sscanf(buf + next_ptr, "%s", str_buf);
				next_ptr += strlen(str_buf) + 1;
			} while (!strstr(str_buf, "[Binary]"));
#else

			for (j = 0; j < cnt;) {
				if (*(buf + next_ptr) == bin_header[j])
					j++;
				else
					j = 0;

				next_ptr++;
			}

			next_ptr++;

			mbin_info[i].bin_offset = next_ptr;

			mbin_info[i].crc =
				get_unaligned_le32(buf + next_ptr + offset[i]);
#endif
			if (mbin_info[i].version == 0xFF)
				return ISC_FILE_FORMAT_ERROR;
		}
	}

	for (i = 0; i < SECTION_NUM; i++) {
		pr_info("[TSP ISC] MBin : Section(%d) Version: 0x%02X\n",
			i, mbin_info[i].version);
		pr_info("[TSP ISC] MBin : Section(%d) Start Address: 0x%02X\n",
			i, mbin_info[i].start_addr);
		pr_info("[TSP ISC] MBin : Section(%d) End Address: 0x%02X\n",
			i, mbin_info[i].end_addr);
		pr_info("[TSP ISC] MBin : Section(%d) Compatibility: 0x%02X\n",
			i, mbin_info[i].compatible_version);
		pr_info("[TSP ISC] MBin : Section(%d) CRC: 0x%02X\n",
			i, mbin_info[i].crc);
	}

	return ISC_SUCCESS;
}

static int mms100_compare_version_info(struct i2c_client *_client,
		bool force_config, bool force_all)
{
	int i, ret;
	unsigned char expected_compatibility[SECTION_NUM];

	if (mms100_get_version_info(_client) != ISC_SUCCESS)
		return ISC_I2C_ERROR;

	ret = mms100_seek_section_info();
	if (ret)
		return ret;

	/* Check update areas , 0 : bootloader 1: core 2: private 3: public */
	for (i = 0; i < SECTION_NUM; i++) {
		if ((mbin_info[i].version == 0) ||
			(mbin_info[i].version != ts_info[i].version)) {
			section_update_flag[i] = true;
			pr_info("[TSP ISC] [%d] section will be updated!", i);
		}
	}
	section_update_flag[0] = false;
	section_update_flag[1] = false;
	if (force_config)
		section_update_flag[2] = true;
	if (force_all) {
		section_update_flag[0] = true;
		section_update_flag[1] = true;
		section_update_flag[2] = true;
	}
	pr_info("[TSP ISC] [%d] [%d] [%d]", section_update_flag[0],
		section_update_flag[1], section_update_flag[2]);

	if (section_update_flag[SEC_BOOTLOADER]) {
		expected_compatibility[SEC_CORE] =
		mbin_info[SEC_BOOTLOADER].version;
	} else {
		expected_compatibility[SEC_CORE] =
		ts_info[SEC_BOOTLOADER].version;
	}

	if (section_update_flag[SEC_CORE]) {
		expected_compatibility[SEC_CONFIG] =
		mbin_info[SEC_CORE].version;
	} else {
		expected_compatibility[SEC_CONFIG] =
		ts_info[SEC_CORE].version;
	}

	for (i = SEC_CORE; i < SEC_CONFIG; i++) {
		if (section_update_flag[i]) {
			pr_info("[TSP ISC] section_update_flag(%d), 0x%02x, 0x%02x\n",
				i, expected_compatibility[i],
				mbin_info[i].compatible_version);

			if (expected_compatibility[i] !=
				mbin_info[i].compatible_version)
				return ISC_COMPATIVILITY_ERROR;
		} else {
			pr_info("[TSP ISC] !section_update_flag(%d), 0x%02x, 0x%02x\n",
				i, expected_compatibility[i],
				ts_info[i].compatible_version);
			if (expected_compatibility[i] !=
				ts_info[i].compatible_version)
				return ISC_COMPATIVILITY_ERROR;
		}
	}
	return ISC_SUCCESS;
}

static int mms100_enter_ISC_mode(struct i2c_client *_client)
{
	int ret;
	unsigned char wr_buf[2];

	pr_info("[TSP ISC] %s\n", __func__);

	wr_buf[0] = ISC_CMD_ENTER_ISC;
	wr_buf[1] = ISC_CMD_ENTER_ISC_PARA1;

	ret = i2c_master_send(_client, wr_buf, 2);

	if (ret < 0) {
		pr_info("[TSP ISC] %s,%d: i2c write fail[%d]\n",
			__func__, __LINE__, ret);
		return ISC_I2C_ERROR;
	}

	msleep(150);

	return ISC_SUCCESS;
}

static int mms100_enter_config_update(struct i2c_client *_client)
{
	int ret;
	unsigned char wr_buf[10] = {0,};
	unsigned char rd_buf;

	wr_buf[0] = ISC_CMD_UPDATE_MODE;
	wr_buf[1] = ISC_SUBCMD_ENTER_UPDATE;

	ret = i2c_master_send(_client, wr_buf, 10);

	if (ret < 0) {
		pr_info("[TSP ISC] %s,%d: i2c write fail[%d]\n",
			__func__, __LINE__, ret);
		return ISC_I2C_ERROR;
	}

	ret = mms100_i2c_read(_client, ISC_CMD_CONFIRM_STATUS, 1, &rd_buf);
	if (ret < 0) {
		pr_info("[TSP ISC] %s,%d: i2c read fail[%d]\n",
			__func__, __LINE__, ret);
		return ISC_I2C_ERROR;
	}

	if (rd_buf != ISC_STATUS_UPDATE_MODE)
		return ISC_UPDATE_MODE_ENTER_ERROR;

	pr_info("[TSP ISC]End mms100_enter_config_update()\n");

	return ISC_SUCCESS;
}

static int mms100_ISC_clear_page(struct i2c_client *_client,
		unsigned char _page_addr)
{
	int ret;
	unsigned char rd_buf;

	memset(&g_wr_buf[3], 0xFF, PAGE_DATA);

	g_wr_buf[0] = ISC_CMD_UPDATE_MODE;	/* command */
	g_wr_buf[1] = ISC_SUBCMD_DATA_WRITE;	/* sub_command */
	g_wr_buf[2] = _page_addr;

	g_wr_buf[PAGE_HEADER + PAGE_DATA] = crc0_buf[_page_addr];
	g_wr_buf[PAGE_HEADER + PAGE_DATA + 1] = crc1_buf[_page_addr];

	ret = i2c_master_send(_client, g_wr_buf, PACKET_SIZE);

	if (ret < 0) {
		pr_info("[TSP ISC] %s,%d: i2c write fail[%d]\n",
			__func__, __LINE__, ret);
		return ISC_I2C_ERROR;
	}

	ret = mms100_i2c_read(_client, ISC_CMD_CONFIRM_STATUS, 1, &rd_buf);

	if (ret < 0) {
		pr_info("[TSP ISC] %s,%d: i2c read fail[%d]\n",
			__func__, __LINE__, ret);
		return ISC_I2C_ERROR;
	}

	if (rd_buf != ISC_STATUS_CRC_CHECK_SUCCESS)
		return ISC_UPDATE_MODE_ENTER_ERROR;

	pr_info("[TSP ISC]End mms100_ISC_clear_page()\n");
	return ISC_SUCCESS;

}

static int mms100_ISC_clear_validate_markers(struct i2c_client *_client)
{
	int ret_msg;
	int i, j;
	bool is_matched_address;

	for (i = SEC_CORE; i <= SEC_CONFIG; i++) {
		if (section_update_flag[i]) {
			if (ts_info[i].end_addr <= 30 &&
				ts_info[i].end_addr > 0) {
				ret_msg = mms100_ISC_clear_page(_client,
					ts_info[i].end_addr);

				if (ret_msg != ISC_SUCCESS)
					return ret_msg;
			}
		}
	}

	for (i = SEC_CORE; i <= SEC_CONFIG; i++) {
		if (section_update_flag[i]) {
			is_matched_address = false;
			for (j = SEC_CORE; j <= SEC_CONFIG; j++) {
				if (mbin_info[i].end_addr ==
					ts_info[i].end_addr) {
					is_matched_address = true;
					break;
				}
			}

			if (!is_matched_address) {
				if (mbin_info[i].end_addr <= 30 &&
					mbin_info[i].end_addr > 0) {
					ret_msg = mms100_ISC_clear_page(_client,
						mbin_info[i].end_addr);

				if (ret_msg != ISC_SUCCESS)
					return ret_msg;
				}
			}
		}
	}

	usleep_range(10000, 15000);

	return ISC_SUCCESS;
}

static void mms100_calc_crc(unsigned char *crc,
		int page_addr, unsigned char *ptr_fw)
{
	int	i, j;

	unsigned char  ucData;

	unsigned short SeedValue;
	unsigned short CRC_check_buf;
	unsigned short CRC_send_buf;
	unsigned short IN_data;
	unsigned short XOR_bit_1;
	unsigned short XOR_bit_2;
	unsigned short XOR_bit_3;

	CRC_check_buf = 0xFFFF;
	SeedValue = (unsigned short)page_addr;

	for (i = 7; i >= 0; i--) {
		IN_data = (SeedValue >> i) & 0x01;
		XOR_bit_1 = (CRC_check_buf & 0x0001) ^ IN_data;
		XOR_bit_2 = XOR_bit_1^(CRC_check_buf >> 11 & 0x01);
		XOR_bit_3 = XOR_bit_1^(CRC_check_buf >> 4 & 0x01);
		CRC_send_buf = (XOR_bit_1 << 4) | (CRC_check_buf >> 12 & 0x0F);
		CRC_send_buf =
			(CRC_send_buf << 7) | (XOR_bit_2 << 6) |
			(CRC_check_buf >> 5 & 0x3F);
		CRC_send_buf =
			(CRC_send_buf << 4) | (XOR_bit_3 << 3) |
			(CRC_check_buf >> 1 & 0x0007);
		CRC_check_buf = CRC_send_buf;
	}

	for (j = 0; j < 1024; j++) {
		ucData = ptr_fw[j];

		for (i = 7; i >= 0; i--) {
			IN_data = (ucData >> i) & 0x0001;
			XOR_bit_1 = (CRC_check_buf & 0x0001) ^ IN_data;
			XOR_bit_2 = XOR_bit_1^(CRC_check_buf >> 11 & 0x01);
			XOR_bit_3 = XOR_bit_1^(CRC_check_buf >> 4 & 0x01);
			CRC_send_buf =
				(XOR_bit_1 << 4) | (CRC_check_buf >> 12 & 0x0F);
			CRC_send_buf =
				(CRC_send_buf << 7) | (XOR_bit_2 << 6) |
				(CRC_check_buf >> 5 & 0x3F);
			CRC_send_buf =
				(CRC_send_buf << 4) | (XOR_bit_3 << 3) |
				(CRC_check_buf >> 1 & 0x0007);
			CRC_check_buf = CRC_send_buf;
		}
	}

	crc[0] = (unsigned char)((CRC_check_buf >> 8) & 0xFF);
	crc[1] = (unsigned char)((CRC_check_buf	>> 0) & 0xFF);
}

static int mms100_update_section_data(struct i2c_client *_client)
{
	int i, j, ret;
	unsigned char rd_buf;
	unsigned char crc[2];
	const unsigned char *ptr_fw;
#if 0
	char str_buf[STRING_BUF_LEN];
#endif
	int page_addr;

	for (i = 0; i < SECTION_NUM; i++) {
		if (section_update_flag[i]) {
			pr_info("[TSP ISC] section data i2c flash : [%d]", i);

			ptr_fw = fw_mbin[i]->data;
#if 0
			do {
				sscanf(ptr_fw, "%s", str_buf);
				ptr_fw += strlen(str_buf) + 1;
/*
	pr_info("[TSP ISC] Section[%d] %s", i, str_buf );
*/
			} while (!strstr(str_buf, "[Binary]"));
			ptr_fw += 1;
#else
			ptr_fw += mbin_info[i].bin_offset;
#endif
			for (page_addr = mbin_info[i].start_addr;
				page_addr <= mbin_info[i].end_addr;
				page_addr++) {
				if (page_addr - mbin_info[i].start_addr > 0)
					ptr_fw += 1024;

				g_wr_buf[0] = ISC_CMD_UPDATE_MODE;
				g_wr_buf[1] = ISC_SUBCMD_DATA_WRITE;
				g_wr_buf[2] = (unsigned char)page_addr;

				for (j = 0; j < 1024; j += 4) {
					g_wr_buf[3+j] = ptr_fw[j+3];
					g_wr_buf[3+j+1] = ptr_fw[j+2];
					g_wr_buf[3+j+2] = ptr_fw[j+1];
					g_wr_buf[3+j+3] = ptr_fw[j+0];
				}

				mms100_calc_crc(crc, page_addr, &g_wr_buf[3]);

				g_wr_buf[1027] = crc[0];
				g_wr_buf[1028] = crc[1];
/*
	pr_info("[TSP ISC] [%d] DATA %02X %02X %02X %02X   CRC %02X %02X ",
		page_addr, g_wr_buf[3], g_wr_buf[4],
		g_wr_buf[5], g_wr_buf[6] , crc[0] , crc[1] );
*/
				ret = i2c_master_send(_client,
					g_wr_buf, PACKET_SIZE);
				if (ret < 0) {
					pr_info("[TSP ISC] %s,%d: i2c write fail[%d]\n",
						__func__, __LINE__, ret);
					return ISC_I2C_ERROR;
				}

				ret = mms100_i2c_read(_client,
					ISC_CMD_CONFIRM_STATUS, 1, &rd_buf);
				if (ret < 0) {
					pr_info("[TSP ISC] %s,%d: i2c read fail[%d]\n",
						__func__, __LINE__, ret);
					return ISC_I2C_ERROR;
				}

				if (rd_buf != ISC_STATUS_CRC_CHECK_SUCCESS)
					return ISC_CRC_ERROR;

				section_update_flag[i] = false;
			}
		}
	}

	return ISC_SUCCESS;
}

static int mms100_open_mbinary(struct mms_ts_info *info)
{
	struct i2c_client *_client = info->client;
	int ret = 0;

	ret += request_firmware(&(fw_mbin[0]),\
			"tsp_melfas/gd2/BOOT.fw", &_client->dev);
	ret += request_firmware(&(fw_mbin[1]),\
			"tsp_melfas/gd2/CORE.fw", &_client->dev);
	ret += request_firmware(&(fw_mbin[2]),\
			"tsp_melfas/gd2/CONF.fw", &_client->dev);
	if (!ret)
		return ISC_SUCCESS;
	else {
		pr_info("[TSP ISC] request_firmware fail");
		return ret;
	}
}

static int mms100_close_mbinary(void)
{
	int i;

	for (i = 0; i < SECTION_NUM; i++) {
		if (fw_mbin[i] != NULL)
			release_firmware(fw_mbin[i]);
	}
	return ISC_SUCCESS;
}

int mms100_ISC_download_mbinary(struct mms_ts_info *info,
		bool force_config, bool force_all)
{
	struct i2c_client *_client = info->client;
	int ret_msg = ISC_NONE;

	pr_info("[TSP ISC] %s\n", __func__);

	mms100_reset(info);

/*	ret_msg = mms100_check_operating_mode(_client, EC_BOOT_ON_SUCCEEDED);
	if (ret_msg != ISC_SUCCESS)
		goto ISC_ERROR_HANDLE;
*/
	ret_msg = mms100_open_mbinary(info);
	if (ret_msg != ISC_SUCCESS)
		goto ISC_ERROR_HANDLE;

	/*Config version Check*/
	ret_msg = mms100_compare_version_info(_client, force_config, force_all);
	if (ret_msg != ISC_SUCCESS)
		goto ISC_ERROR_HANDLE;

	ret_msg = mms100_enter_ISC_mode(_client);
	if (ret_msg != ISC_SUCCESS)
		goto ISC_ERROR_HANDLE;

	ret_msg = mms100_enter_config_update(_client);
	if (ret_msg != ISC_SUCCESS)
		goto ISC_ERROR_HANDLE;

	ret_msg = mms100_ISC_clear_validate_markers(_client);
	if (ret_msg != ISC_SUCCESS)
		goto ISC_ERROR_HANDLE;

	pr_info("[TSP ISC]mms100_update_section_data start");

	ret_msg = mms100_update_section_data(_client);
	if (ret_msg != ISC_SUCCESS)
		goto ISC_ERROR_HANDLE;

	pr_info("[TSP ISC]mms100_update_section_data end");

/*	mms100_reset(info); */

	pr_info("[TSP ISC]FIRMWARE_UPDATE_FINISHED!!!\n");

	ret_msg = ISC_SUCCESS;

ISC_ERROR_HANDLE:
	if (ret_msg != ISC_SUCCESS)
		pr_info("[TSP ISC]ISC_ERROR_CODE: %d\n", ret_msg);

	mms100_reset(info);
	mms100_close_mbinary();

	return ret_msg;
}
#endif	/* ISC_DL_MODE start */

static void hw_reboot(struct mms_ts_info *info, bool bootloader)
{
	info->pdata->power(0);
	gpio_direction_output(info->pdata->gpio_sda, bootloader ? 0 : 1);
	gpio_direction_output(info->pdata->gpio_scl, bootloader ? 0 : 1);
	gpio_direction_output(info->pdata->gpio_int, 0);
	msleep(30);
	info->pdata->power(1);
	msleep(30);

	if (bootloader) {
		gpio_set_value(info->pdata->gpio_scl, 0);
		gpio_set_value(info->pdata->gpio_sda, 1);
	} else {
		gpio_set_value(info->pdata->gpio_int, 1);
		gpio_direction_input(info->pdata->gpio_int);
		gpio_direction_input(info->pdata->gpio_scl);
		gpio_direction_input(info->pdata->gpio_sda);
	}
	msleep(40);
}

static inline void hw_reboot_bootloader(struct mms_ts_info *info)
{
	hw_reboot(info, true);
}

static inline void hw_reboot_normal(struct mms_ts_info *info)
{
	hw_reboot(info, false);
}

static void isp_toggle_clk(struct mms_ts_info *info, int start_lvl, int end_lvl,
			   int hold_us)
{
	gpio_set_value(info->pdata->gpio_scl, start_lvl);
	udelay(hold_us);
	gpio_set_value(info->pdata->gpio_scl, end_lvl);
	udelay(hold_us);
}

/* 1 <= cnt <= 32 bits to write */
static void isp_send_bits(struct mms_ts_info *info, u32 data, int cnt)
{
	gpio_direction_output(info->pdata->gpio_int, 0);
	gpio_direction_output(info->pdata->gpio_scl, 0);
	gpio_direction_output(info->pdata->gpio_sda, 0);

	/* clock out the bits, msb first */
	while (cnt--) {
		gpio_set_value(info->pdata->gpio_sda, (data >> cnt) & 1);
		udelay(3);
		isp_toggle_clk(info, 1, 0, 3);
	}
}

/* 1 <= cnt <= 32 bits to read */
static u32 isp_recv_bits(struct mms_ts_info *info, int cnt)
{
	u32 data = 0;

	gpio_direction_output(info->pdata->gpio_int, 0);
	gpio_direction_output(info->pdata->gpio_scl, 0);
	gpio_set_value(info->pdata->gpio_sda, 0);
	gpio_direction_input(info->pdata->gpio_sda);

	/* clock in the bits, msb first */
	while (cnt--) {
		isp_toggle_clk(info, 0, 1, 1);
		data = (data << 1) | (!!gpio_get_value(info->pdata->gpio_sda));
	}

	gpio_direction_output(info->pdata->gpio_sda, 0);
	return data;
}

static void isp_enter_mode(struct mms_ts_info *info, u32 mode)
{
	int cnt;
	unsigned long flags;

	local_irq_save(flags);
	gpio_direction_output(info->pdata->gpio_int, 0);
	gpio_direction_output(info->pdata->gpio_scl, 0);
	gpio_direction_output(info->pdata->gpio_sda, 1);

	mode &= 0xffff;
	for (cnt = 15; cnt >= 0; cnt--) {
		gpio_set_value(info->pdata->gpio_int, (mode >> cnt) & 1);
		udelay(3);
		isp_toggle_clk(info, 1, 0, 3);
	}

	gpio_set_value(info->pdata->gpio_int, 0);
	local_irq_restore(flags);
}

static void isp_exit_mode(struct mms_ts_info *info)
{
	int i;
	unsigned long flags;

	local_irq_save(flags);
	gpio_direction_output(info->pdata->gpio_int, 0);
	udelay(3);

	for (i = 0; i < 10; i++)
		isp_toggle_clk(info, 1, 0, 3);
	local_irq_restore(flags);
}

static void flash_set_address(struct mms_ts_info *info, u16 addr)
{
	/* Only 13 bits of addr are valid.
	 * The addr is in bits 13:1 of cmd */
	isp_send_bits(info, (u32) (addr & 0x1fff) << 1, 18);
}

static void flash_erase(struct mms_ts_info *info)
{
	isp_enter_mode(info, ISP_MODE_FLASH_ERASE);

	gpio_direction_output(info->pdata->gpio_int, 0);
	gpio_direction_output(info->pdata->gpio_scl, 0);
	gpio_direction_output(info->pdata->gpio_sda, 1);

	/* 4 clock cycles with different timings for the erase to
	 * get processed, clk is already 0 from above */
	udelay(7);
	isp_toggle_clk(info, 1, 0, 3);
	udelay(7);
	isp_toggle_clk(info, 1, 0, 3);
	usleep_range(25000, 35000);
	isp_toggle_clk(info, 1, 0, 3);
	usleep_range(150, 200);
	isp_toggle_clk(info, 1, 0, 3);

	gpio_set_value(info->pdata->gpio_sda, 0);

	isp_exit_mode(info);
}

static u32 flash_readl(struct mms_ts_info *info, u16 addr)
{
	int i;
	u32 val;
	unsigned long flags;

	local_irq_save(flags);
	isp_enter_mode(info, ISP_MODE_FLASH_READ);
	flash_set_address(info, addr);

	gpio_direction_output(info->pdata->gpio_scl, 0);
	gpio_direction_output(info->pdata->gpio_sda, 0);
	udelay(40);

	/* data load cycle */
	for (i = 0; i < 6; i++)
		isp_toggle_clk(info, 1, 0, 10);

	val = isp_recv_bits(info, 32);
	isp_exit_mode(info);
	local_irq_restore(flags);

	return val;
}

static void flash_writel(struct mms_ts_info *info, u16 addr, u32 val)
{
	unsigned long flags;

	local_irq_save(flags);
	isp_enter_mode(info, ISP_MODE_FLASH_WRITE);
	flash_set_address(info, addr);
	isp_send_bits(info, val, 32);

	gpio_direction_output(info->pdata->gpio_sda, 1);
	/* 6 clock cycles with different timings for the data to get written
	 * into flash */
	isp_toggle_clk(info, 0, 1, 3);
	isp_toggle_clk(info, 0, 1, 3);
	isp_toggle_clk(info, 0, 1, 6);
	isp_toggle_clk(info, 0, 1, 12);
	isp_toggle_clk(info, 0, 1, 3);
	isp_toggle_clk(info, 0, 1, 3);

	isp_toggle_clk(info, 1, 0, 1);

	gpio_direction_output(info->pdata->gpio_sda, 0);
	isp_exit_mode(info);
	local_irq_restore(flags);
	usleep_range(300, 400);
}

static bool flash_is_erased(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;
	u32 val;
	u16 addr;

	for (addr = 0; addr < (ISP_MAX_FW_SIZE / 4); addr++) {
		udelay(40);
		val = flash_readl(info, addr);

		if (val != 0xffffffff) {
			dev_dbg(&client->dev,
				"addr 0x%x not erased: 0x%08x != 0xffffffff\n",
				addr, val);
			return false;
		}
	}
	return true;
}

static int fw_write_image(struct mms_ts_info *info, const u8 * data, size_t len)
{
	struct i2c_client *client = info->client;
	u16 addr = 0;

	for (addr = 0; addr < (len / 4); addr++, data += 4) {
		u32 val = get_unaligned_le32(data);
		u32 verify_val;
		int retries = 3;

		while (retries--) {
			flash_writel(info, addr, val);
			verify_val = flash_readl(info, addr);
			if (val == verify_val)
				break;
			dev_err(&client->dev,
				"mismatch @ addr 0x%x: 0x%x != 0x%x\n",
				addr, verify_val, val);
			continue;
		}
		if (retries < 0)
			return -ENXIO;
	}

	return 0;
}

static int fw_download(struct mms_ts_info *info, const u8 * data, size_t len)
{
	struct i2c_client *client = info->client;
	u32 val;
	int ret = 0;
	int i;
	u32 *buf = kzalloc(ISP_CAL_DATA_SIZE * 4, GFP_KERNEL);
	if (!buf) {
		dev_err(&info->client->dev, "%s: failed to allocate memory\n",
			__func__);
		return -ENOMEM;
	}

	if (len % 4) {
		dev_err(&client->dev,
			"fw image size (%d) must be a multiple of 4 bytes\n",
			len);
		kfree(buf);
		return -EINVAL;
	} else if (len > ISP_MAX_FW_SIZE) {
		dev_err(&client->dev,
			"fw image is too big, %d > %d\n", len, ISP_MAX_FW_SIZE);
		kfree(buf);
		return -EINVAL;
	}

	dev_info(&client->dev, "fw download start\n");

	info->pdata->power(0);
	gpio_direction_output(info->pdata->gpio_sda, 0);
	gpio_direction_output(info->pdata->gpio_scl, 0);
	gpio_direction_output(info->pdata->gpio_int, 0);

	hw_reboot_bootloader(info);

	dev_info(&client->dev, "calibration data backup\n");
	for (i = 0; i < ISP_CAL_DATA_SIZE; i++)
		buf[i] = flash_readl(info, ISP_IC_INFO_ADDR);

	val = flash_readl(info, ISP_IC_INFO_ADDR);
	dev_info(&client->dev, "IC info: 0x%02x (%x)\n", val & 0xff, val);

	dev_info(&client->dev, "fw erase...\n");
	flash_erase(info);
	if (!flash_is_erased(info)) {
		ret = -ENXIO;
		goto err;
	}

	dev_info(&client->dev, "fw write...\n");
	/* XXX: what does this do?! */
	flash_writel(info, ISP_IC_INFO_ADDR, 0xffffff00 | (val & 0xff));
	usleep_range(1000, 1500);
	ret = fw_write_image(info, data, len);
	if (ret)
		goto err;
	usleep_range(1000, 1500);

	dev_info(&client->dev, "restoring data\n");
	for (i = 0; i < ISP_CAL_DATA_SIZE; i++)
		flash_writel(info, ISP_IC_INFO_ADDR, buf[i]);
	kfree(buf);

	dev_info(&client->dev, "fw download done...\n");
	hw_reboot_normal(info);
	msleep(200);
	return 0;

err:
	dev_err(&client->dev, "fw download failed...\n");
	kfree(buf);
	hw_reboot_normal(info);
	return ret;
}

static int mms_flash_boot(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	int ret = 0;
	int retries = 3;
	const u8 *buff = 0;
	long fsize = 0;
	const struct firmware *tsp_fw = NULL;

	dev_info(&client->dev, "%s\n", __func__);

	ret = request_firmware(&tsp_fw,
		"tsp_melfas/note/mms_boot.fw", &(client->dev));
	if (ret) {
		dev_err(&client->dev, "request firmware error!!\n");
		return 1;
	}

	fsize = tsp_fw->size;
	buff = kzalloc((size_t)fsize, GFP_KERNEL);
	if (!buff) {
		dev_err(&client->dev, "fail to alloc buffer for fw\n");
		info->cmd_state = 3;
		release_firmware(tsp_fw);
		return 1;
	}

	memcpy((void *)buff, tsp_fw->data, fsize);
	release_firmware(tsp_fw);

	while (retries--) {
		i2c_lock_adapter(adapter);
		info->pdata->mux_fw_flash(true);

		ret = fw_download(info, (const u8 *)buff,
				(const size_t)fsize);

		info->pdata->mux_fw_flash(false);
		i2c_unlock_adapter(adapter);

		if (ret < 0)
			dev_err(&client->dev, "retry flashing\n");
		else
			break;
	}
	kfree(buff);
	return ret;
}

#if defined(SEC_TSP_ISC_FW_UPDATE)
static u16 gen_crc(u8 data, u16 pre_crc)
{
	u16 crc;
	u16 cur;
	u16 temp;
	u16 bit_1;
	u16 bit_2;
	int i;

	crc = pre_crc;
	for (i = 7; i >= 0; i--) {
		cur = ((data >> i) & 0x01) ^ (crc & 0x0001);
		bit_1 = cur ^ (crc >> 11 & 0x01);
		bit_2 = cur ^ (crc >> 4 & 0x01);
		temp = (cur << 4) | (crc >> 12 & 0x0F);
		temp = (temp << 7) | (bit_1 << 6) | (crc >> 5 & 0x3F);
		temp = (temp << 4) | (bit_2 << 3) | (crc >> 1 & 0x0007);
		crc = temp;
	}
	return crc;
}

static int isc_fw_download(struct mms_ts_info *info, const u8 * data,
			   size_t len)
{
	u8 *buff;
	u16 crc_buf;
	int src_idx;
	int dest_idx;
	int ret;
	int i, j;

	buff = kzalloc(ISC_PKT_SIZE, GFP_KERNEL);
	if (!buff) {
		dev_err(&info->client->dev, "%s: failed to allocate memory\n",
			__func__);
		ret = -1;
		goto err_mem_alloc;
	}

	/* enterring ISC mode */
	*buff = ISC_ENTER_ISC_DATA;
	ret = i2c_smbus_write_byte_data(info->client,
		ISC_ENTER_ISC_CMD, *buff);
	if (ret < 0) {
		dev_err(&info->client->dev,
			"fail to enter ISC mode(err=%d)\n", ret);
		goto fail_to_isc_enter;
	}
	usleep_range(10000, 20000);
	dev_info(&info->client->dev, "Enter ISC mode\n");

	/*enter ISC update mode */
	*buff = ISC_ENTER_UPDATE_DATA;
	ret = i2c_smbus_write_i2c_block_data(info->client,
					     ISC_CMD,
					     ISC_ENTER_UPDATE_DATA_LEN, buff);
	if (ret < 0) {
		dev_err(&info->client->dev,
			"fail to enter ISC update mode(err=%d)\n", ret);
		goto fail_to_isc_update;
	}
	dev_info(&info->client->dev, "Enter ISC update mode\n");

	/* firmware write */
	*buff = ISC_CMD;
	*(buff + 1) = ISC_DATA_WRITE_SUB_CMD;
	for (i = 0; i < ISC_PKT_NUM; i++) {
		*(buff + 2) = i;
		crc_buf = gen_crc(*(buff + 2), ISC_DEFAULT_CRC);

		for (j = 0; j < ISC_PKT_DATA_SIZE; j++) {
			dest_idx = ISC_PKT_HEADER_SIZE + j;
			src_idx = i * ISC_PKT_DATA_SIZE +
			    ((int)(j / WORD_SIZE)) * WORD_SIZE -
			    (j % WORD_SIZE) + 3;
			*(buff + dest_idx) = *(data + src_idx);
			crc_buf = gen_crc(*(buff + dest_idx), crc_buf);
		}

		*(buff + ISC_PKT_DATA_SIZE + ISC_PKT_HEADER_SIZE + 1) =
		    crc_buf & 0xFF;
		*(buff + ISC_PKT_DATA_SIZE + ISC_PKT_HEADER_SIZE) =
		    crc_buf >> 8 & 0xFF;

		ret = i2c_master_send(info->client, buff, ISC_PKT_SIZE);
		if (ret < 0) {
			dev_err(&info->client->dev,
				"fail to firmware writing on packet %d.(%d)\n",
				i, ret);
			goto fail_to_fw_write;
		}
		usleep_range(1, 5);

		/* confirm CRC */
		ret = i2c_smbus_read_byte_data(info->client,
					       ISC_CHECK_STATUS_CMD);
		if (ret == ISC_CONFIRM_CRC) {
			dev_info(&info->client->dev,
				 "updating %dth firmware data packet.\n", i);
		} else {
			dev_err(&info->client->dev,
				"fail to firmware update on %dth (%X).\n",
				i, ret);
			ret = -1;
			goto fail_to_confirm_crc;
		}
	}

	ret = 0;

fail_to_confirm_crc:
fail_to_fw_write:
	/* exit ISC mode */
	*buff = ISC_EXIT_ISC_SUB_CMD;
	*(buff + 1) = ISC_EXIT_ISC_SUB_CMD2;
	i2c_smbus_write_i2c_block_data(info->client, ISC_CMD, 2, buff);
	usleep_range(10000, 20000);
fail_to_isc_update:
	hw_reboot_normal(info);
fail_to_isc_enter:
	kfree(buff);
err_mem_alloc:
	return ret;
}
#endif /* SEC_TSP_ISC_FW_UPDATE */

static unsigned char get_fw_version(struct mms_ts_info *info, u8 area)
{
	struct i2c_client *client = info->client;
	struct i2c_adapter *adapter = client->adapter;
	struct i2c_msg msg;
	u8 reg = MMS_CORE_VERSION;
	int ret;
	unsigned char buf[4];

	msg.addr = client->addr;
	msg.flags = 0x00;
	msg.len = 1;
	msg.buf = &reg;

	ret = i2c_transfer(adapter, &msg, 1);

	if (ret >= 0) {
		msg.addr = client->addr;
		msg.flags = I2C_M_RD;
		msg.len = 4;
		msg.buf = buf;

		ret = i2c_transfer(adapter, &msg, 1);
	}
	if (ret < 0) {
		pr_err("[TSP] : read error : [%d]", ret);
		return ret;
	}

	if (area == SEC_BOOTLOADER)
		return buf[0];
	else if (area == SEC_CORE)
		return buf[1];
	else if (area == SEC_CONFIG)
		return buf[2];
	else
		return 0;
}

static int get_panel_version(struct mms_ts_info *info)
{
	int ret;
	int retries = 3;

	/* this seems to fail sometimes after a reset.. retry a few times */
	do {
		ret = i2c_smbus_read_byte_data(info->client, MMS_COMPAT_GROUP);
	} while (ret < 0 && retries-- > 0);

	return ret;
}

/*
static int mms_ts_enable(struct mms_ts_info *info, int wakeupcmd)
{
	mutex_lock(&info->lock);
	if (info->enabled)
		goto out;

	if (wakeupcmd == 1) {
		i2c_smbus_write_byte_data(info->client, 0, 0);
		usleep_range(3000, 5000);
	}
	info->enabled = true;
	enable_irq(info->irq);
out:
	mutex_unlock(&info->lock);
	return 0;
}

static int mms_ts_disable(struct mms_ts_info *info, int sleepcmd)
{
	mutex_lock(&info->lock);
	if (!info->enabled)
		goto out;
	disable_irq_nosync(info->irq);
	if (sleepcmd == 1) {
		i2c_smbus_write_byte_data(info->client, MMS_MODE_CONTROL, 0);
		usleep_range(10000, 12000);
	}
	info->enabled = false;
	touch_is_pressed = 0;
out:
	mutex_unlock(&info->lock);
	return 0;
}
*/

static int check_fw_status(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;
	int ret;
	int size;
	u32 pre_crc;
	u32 test_crc;
	u8 cmd, val;
	int cnt = 30;
	int gpio = info->pdata->gpio_int;

	cmd = 0xA0;
	val = 0x02;
	ret = i2c_smbus_write_byte_data(client, cmd, val);
	if (ret)
		goto out;

	size = i2c_smbus_read_byte_data(client, 0xAE);
	if (size != 4) {
		dev_err(&client->dev,
			"universial cmd size error. got %d, expected 4\n",
			size);
		goto out;
	}

	cmd = 0xAF;
	ret = i2c_smbus_read_i2c_block_data(client, cmd, size, (u8 *)&pre_crc);
	if (ret != size)
		goto out;

	dev_info(&client->dev, "pre_calced crc val : 0x%X", pre_crc);

	val = 0x4E;
	ret = i2c_smbus_write_byte_data(client, cmd, val);
	if (ret)
		goto out;

	do {
		usleep_range(1000, 1500);
	} while (cnt-- && gpio_get_value(gpio));

	if (cnt < 0)
		dev_info(&client->dev, "intr timeout, forcing to read\n");

	size = i2c_smbus_read_byte_data(client, 0xAE);
	if (size != 4) {
		dev_err(&client->dev,
			"universial cmd size error. got %d, expected 4\n",
			size);
		goto out;
	}

	cmd = 0xAF;
	ret = i2c_smbus_read_i2c_block_data(client, cmd, size, (u8 *)&test_crc);
	if (ret != size)
		goto out;

	dev_info(&client->dev, "calced crc val : 0x%X", pre_crc);

	return (pre_crc != test_crc);

out:
	dev_err(&client->dev, "failed to read crc data\n");
	ret = -EIO;
	return ret;
}

static int mms_ts_fw_load(struct mms_ts_info *info, bool force_config)
{

	struct i2c_client *client = info->client;
	int ret = 0;
	int bin_ver = FW_VERSION;
	int retries = 4;
	int fw_check = 0;
	bool error = false;
	u8 ver;

	info->fw_ic_ver = get_fw_version(info, SEC_CONFIG);
	dev_info(&client->dev,
		 "[TSP]fw version 0x%02x !!!!\n",
		 info->fw_ic_ver);

	if (!force_config) {
		if ((info->fw_ic_ver >= bin_ver) &&
			(info->fw_ic_ver != 0xff)) {
			dev_info(&client->dev,
				"fw version update does not need\n");
			goto done;
		}
	}

	while (retries--) {
		if (error) {
			dev_err(&client->dev, "retry flashing\n");
			ret = mms_flash_boot(info);
			if (ret != 0) {
				dev_err(&client->dev, "mms_flash_boot fail\n");
				continue;
			}
			error = false;
		}

		if (retries == 3)
			ret = mms100_ISC_download_mbinary(info,
				force_config, false);
		else
			ret = mms100_ISC_download_mbinary(info,
				force_config, true);
		if (ret != 0) {
			dev_err(&client->dev, "mms100_ISC_download_mbinary fail\n");
			error = true;
			continue;
		}
		fw_check = check_fw_status(info);
		if (fw_check != 0) {
			dev_err(&client->dev, "check_fw_status fail\n");
			error = true;
			continue;
		}
		if (retries != 3) {
			ver = get_fw_version(info, SEC_BOOTLOADER);
			if (ver != BOOT_VERSION) {
				dev_err(&client->dev, "boot version read fail\n");
				error = true;
				continue;
			}
			ver = get_fw_version(info, SEC_CORE);
			if (ver != CORE_VERSION) {
				dev_err(&client->dev, "core version read fail\n");
				error = true;
				continue;
			}
		}
		info->fw_ic_ver = get_fw_version(info, SEC_CONFIG);
		if (info->fw_ic_ver != bin_ver) {
			dev_err(&client->dev, "fw version read fail\n");
			error = true;
			continue;
		}
		pr_err("[TSP] mms100_ISC_download_mbinary success");
		break;
	}
	if (error) {
		pr_err("[TSP] fw update fail");
		ret = 1;
	} else {
		dev_info(&client->dev,
			 "[TSP]fw version 0x%02x !!!!\n",
			 info->fw_ic_ver);
	}
done:
	return ret;
}

#ifdef SEC_TSP_FACTORY_TEST
static void set_cmd_result(struct mms_ts_info *info, char *buff, int len)
{
	strncat(info->cmd_result, buff, len);
}

static int get_data(struct mms_ts_info *info, u8 addr, u8 size, u8 *array)
{
	struct i2c_client *client = info->client;
	struct i2c_adapter *adapter = client->adapter;
	struct i2c_msg msg;
	u8 reg = addr;
	unsigned char buf[size];
	int ret;

	msg.addr = client->addr;
	msg.flags = 0x00;
	msg.len = 1;
	msg.buf = &reg;

	ret = i2c_transfer(adapter, &msg, 1);

	if (ret >= 0) {
		msg.addr = client->addr;
		msg.flags = I2C_M_RD;
		msg.len = size;
		msg.buf = buf;

		ret = i2c_transfer(adapter, &msg, 1);
	}
	if (ret < 0) {
		pr_err("[TSP] : read error : [%d]", ret);
		return ret;
	}

	memcpy(array, &buf, size);
	return size;
}

static void get_intensity_data(struct mms_ts_info *info)
{
	u8 w_buf[4];
	u8 r_buf;
	u8 read_buffer[60] = {0};
	int i, j;
	int ret;
	u16 max_value = 0, min_value = 0;
	u16 raw_data;
	char buff[TSP_CMD_STR_LEN] = {0};

	disable_irq(info->irq);

	w_buf[0] = ADDR_UNIV_CMD;
	w_buf[1] = CMD_GET_INTEN;
	w_buf[2] = 0xFF;
	for (i = 0; i < RX_NUM; i++) {
		w_buf[3] = i;

		ret = i2c_smbus_write_i2c_block_data(info->client,
			w_buf[0], 3, &w_buf[1]);
		if (ret < 0)
			goto err_i2c;
		usleep_range(1, 5);

		ret = i2c_smbus_read_i2c_block_data(info->client,
			CMD_RESULT_SZ, 1, &r_buf);
		if (ret < 0)
			goto err_i2c;

		ret = get_data(info, CMD_RESULT, r_buf, read_buffer);
		if (ret < 0)
			goto err_i2c;

		for (j = 0; j < r_buf/2; j++) {
			raw_data = read_buffer[2*j] | (read_buffer[2*j+1] << 8);
			if (raw_data > 32767)
				raw_data = 0;
			if (i == 0 && j == 0) {
				max_value = min_value = raw_data;
			} else {
				max_value = max(max_value, raw_data);
				min_value = min(min_value, raw_data);
			}
			info->intensity[i * TX_NUM + j] = raw_data;
			dev_dbg(&info->client->dev,
				"[TSP] intensity[%d][%d] = %d\n", j, i,
				info->intensity[i * TX_NUM + j]);
		}
	}

	snprintf(buff, sizeof(buff), "%d,%d", min_value, max_value);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));

	enable_irq(info->irq);

	return;

err_i2c:
	dev_err(&info->client->dev, "%s: fail to i2c (cmd=%d)\n",
		__func__, MMS_VSC_CMD_INTENSITY);
}

static void get_raw_data(struct mms_ts_info *info, u8 cmd)
{
	u8 w_buf[4];
	u8 r_buf = 0;
	u8 read_buffer[60] = {0};
	int ret;
	int i, j;
	int max_value = 0, min_value = 0;
	int raw_data;
	int retry;
	char buff[TSP_CMD_STR_LEN] = {0};
	int gpio = info->pdata->gpio_int;

	disable_irq(info->irq);

	ret = i2c_smbus_write_byte_data(info->client,
		ADDR_UNIV_CMD, CMD_ENTER_TEST);
	if (ret < 0)
		goto err_i2c;

	/* event type check */
	retry = 1;
	while (retry) {
		while (gpio_get_value(gpio))
			udelay(100);

		ret = i2c_smbus_read_i2c_block_data(info->client,
			0x0F, 1, &r_buf);
		if (ret < 0)
			goto err_i2c;

		ret = i2c_smbus_read_i2c_block_data(info->client,
			0x10, 1, &r_buf);
		if (ret < 0)
			goto err_i2c;

		dev_info(&info->client->dev, "event type = 0x%x\n", r_buf);
		if (r_buf == 0x0C)
			retry = 0;
	}

	w_buf[0] = ADDR_UNIV_CMD;
	if (cmd == MMS_VSC_CMD_CM_DELTA)
		w_buf[1] = CMD_CM_DELTA;
	else
		w_buf[1] = CMD_CM_ABS;
	ret = i2c_smbus_write_i2c_block_data(info->client,
		 w_buf[0], 1, &w_buf[1]);
	if (ret < 0)
		goto err_i2c;
	while (gpio_get_value(gpio))
		udelay(100);

	ret = i2c_smbus_read_i2c_block_data(info->client,
		CMD_RESULT_SZ, 1, &r_buf);
	if (ret < 0)
		goto err_i2c;
	ret = i2c_smbus_read_i2c_block_data(info->client,
		CMD_RESULT, 1, &r_buf);
	if (ret < 0)
		goto err_i2c;

	if (r_buf == 1)
		dev_info(&info->client->dev, "PASS\n");
	else
		dev_info(&info->client->dev, "FAIL\n");

	if (cmd == MMS_VSC_CMD_CM_DELTA)
		w_buf[1] = CMD_GET_DELTA;
	else
		w_buf[1] = CMD_GET_ABS;
	w_buf[2] = 0xFF;

	for (i = 0; i < RX_NUM; i++) {
		w_buf[3] = i;

		ret = i2c_smbus_write_i2c_block_data(info->client,
			w_buf[0], 3, &w_buf[1]);
		if (ret < 0)
			goto err_i2c;

		while (gpio_get_value(gpio))
			udelay(100);

		ret = i2c_smbus_read_i2c_block_data(info->client,
			CMD_RESULT_SZ, 1, &r_buf);
		if (ret < 0)
			goto err_i2c;

		ret = get_data(info, CMD_RESULT, r_buf, read_buffer);
		if (ret < 0)
			goto err_i2c;

		for (j = 0; j < TX_NUM; j++) {
			raw_data = read_buffer[2*j] | (read_buffer[2*j+1] << 8);
			if (i == 0 && j == 0) {
				max_value = min_value = raw_data;
			} else {
				max_value = max(max_value, raw_data);
				min_value = min(min_value, raw_data);
			}

			if (cmd == MMS_VSC_CMD_CM_DELTA) {
				info->inspection[i * TX_NUM + j] =
					raw_data;
				dev_dbg(&info->client->dev,
					"[TSP] delta[%d][%d] = %d\n", j, i,
					info->inspection[i * TX_NUM + j]);
			} else if (cmd == MMS_VSC_CMD_CM_ABS) {
				info->raw[i * TX_NUM + j] =
					raw_data;
				dev_dbg(&info->client->dev,
					"[TSP] raw[%d][%d] = %d\n", j, i,
					info->raw[i * TX_NUM + j]);
			} else if (cmd == MMS_VSC_CMD_REFER) {
				info->reference[i * TX_NUM + j] =
					raw_data;
				dev_dbg(&info->client->dev,
					"[TSP] reference[%d][%d] = %d\n", j, i,
					info->reference[i * TX_NUM + j]);
			}
		}
	}

	ret = i2c_smbus_write_byte_data(info->client,
		ADDR_UNIV_CMD, CMD_EXIT_TEST);

	snprintf(buff, sizeof(buff), "%d,%d", min_value, max_value);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));

	touch_is_pressed = 0;
	release_all_fingers(info);

	mms_pwr_on_reset(info);
	info->enabled = true;


      printk("%s  ,   %d \n",__func__, __LINE__);
	if (info->ta_status) {
		dev_notice(&info->client->dev, "TA connect!!!\n");
		i2c_smbus_write_byte_data(info->client, 0x33, 0x1);
	} else {
		dev_notice(&info->client->dev, "TA disconnect!!!\n");
		i2c_smbus_write_byte_data(info->client, 0x33, 0x2);
	}
	mms_set_noise_mode(info);

	enable_irq(info->irq);

	return;

err_i2c:
	dev_err(&info->client->dev, "%s: fail to i2c (cmd=%d)\n",
		__func__, cmd);
}

static void get_raw_data_all(struct mms_ts_info *info, u8 cmd)
{
	u8 w_buf[6];
	u8 read_buffer[2];	/* 52 */
	int ret;
	int i, j;
	u32 max_value = 0, min_value = 0;
	u32 raw_data;
	char buff[TSP_CMD_STR_LEN] = {0};
	int gpio = info->pdata->gpio_int;

/*      gpio = msm_irq_to_gpio(info->irq); */
	disable_irq(info->irq);

	w_buf[0] = MMS_VSC_CMD;	/* vendor specific command id */
	w_buf[1] = MMS_VSC_MODE;	/* mode of vendor */
	w_buf[2] = 0;		/* tx line */
	w_buf[3] = 0;		/* rx line */
	w_buf[4] = 0;		/* reserved */
	w_buf[5] = 0;		/* sub command */

	if (cmd == MMS_VSC_CMD_EXIT) {
		w_buf[5] = MMS_VSC_CMD_EXIT;	/* exit test mode */

		ret = i2c_smbus_write_i2c_block_data(info->client,
						     w_buf[0], 5, &w_buf[1]);
		if (ret < 0)
			goto err_i2c;
		enable_irq(info->irq);
		msleep(200);
		return;
	}

	/* MMS_VSC_CMD_CM_DELTA or MMS_VSC_CMD_CM_ABS
	 * this two mode need to enter the test mode
	 * exit command must be followed by testing.
	 */
	if (cmd == MMS_VSC_CMD_CM_DELTA || cmd == MMS_VSC_CMD_CM_ABS) {
		/* enter the debug mode */
		w_buf[2] = 0x0;	/* tx */
		w_buf[3] = 0x0;	/* rx */
		w_buf[5] = MMS_VSC_CMD_ENTER;

		ret = i2c_smbus_write_i2c_block_data(info->client,
						     w_buf[0], 5, &w_buf[1]);
		if (ret < 0)
			goto err_i2c;

		/* wating for the interrupt */
		while (gpio_get_value(gpio))
			udelay(100);
	}

	for (i = 0; i < RX_NUM; i++) {
		for (j = 0; j < TX_NUM; j++) {

			w_buf[2] = j;	/* tx */
			w_buf[3] = i;	/* rx */
			w_buf[5] = cmd;

			ret = i2c_smbus_write_i2c_block_data(info->client,
					w_buf[0], 5, &w_buf[1]);
			if (ret < 0)
				goto err_i2c;

			usleep_range(1, 5);

			ret = i2c_smbus_read_i2c_block_data(info->client, 0xBF,
							    2, read_buffer);
			if (ret < 0)
				goto err_i2c;

			raw_data = ((u16) read_buffer[1] << 8) | read_buffer[0];
			if (i == 0 && j == 0) {
				max_value = min_value = raw_data;
			} else {
				max_value = max(max_value, raw_data);
				min_value = min(min_value, raw_data);
			}

			if (cmd == MMS_VSC_CMD_INTENSITY) {
				info->intensity[i * TX_NUM + j] =
					raw_data;
				dev_dbg(&info->client->dev,
					"[TSP] intensity[%d][%d] = %d\n", j, i,
					info->intensity[i * TX_NUM + j]);
			} else if (cmd == MMS_VSC_CMD_CM_DELTA) {
				info->inspection[i * TX_NUM + j] =
					raw_data;
				dev_dbg(&info->client->dev,
					"[TSP] delta[%d][%d] = %d\n", j, i,
					info->inspection[i * TX_NUM + j]);
			} else if (cmd == MMS_VSC_CMD_CM_ABS) {
				info->raw[i * TX_NUM + j] =
					raw_data;
				dev_dbg(&info->client->dev,
					"[TSP] raw[%d][%d] = %d\n", j, i,
					info->raw[i * TX_NUM + j]);
			} else if (cmd == MMS_VSC_CMD_REFER) {
				info->reference[i * TX_NUM + j] =
					raw_data;
				dev_dbg(&info->client->dev,
					"[TSP] reference[%d][%d] = %d\n", j, i,
					info->reference[i * TX_NUM + j]);
			}
		}

	}

	snprintf(buff, sizeof(buff), "%d,%d", min_value, max_value);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));

	enable_irq(info->irq);

	return;

err_i2c:
	dev_err(&info->client->dev, "%s: fail to i2c (cmd=%d)\n",
		__func__, cmd);
}

static ssize_t show_close_tsp_test(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct mms_ts_info *info = dev_get_drvdata(dev);

	get_raw_data_all(info, MMS_VSC_CMD_EXIT);
	info->ft_flag = 0;

	return snprintf(buf, TSP_BUF_SIZE, "%u\n", 0);
}

static void set_default_result(struct mms_ts_info *info)
{
	char delim = ':';

	memset(info->cmd_result, 0x00, ARRAY_SIZE(info->cmd_result));
	memcpy(info->cmd_result, info->cmd, strlen(info->cmd));
	strncat(info->cmd_result, &delim, 1);
}

static int check_rx_tx_num(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[TSP_CMD_STR_LEN] = {0};
	int node;

	if (info->cmd_param[0] < 0 ||
			info->cmd_param[0] >= TX_NUM  ||
			info->cmd_param[1] < 0 ||
			info->cmd_param[1] >= RX_NUM) {
		snprintf(buff, sizeof(buff) , "%s", "NG");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = 3;

		dev_info(&info->client->dev, "%s: parameter error: %u,%u\n",
				__func__, info->cmd_param[0],
				info->cmd_param[1]);
		node = -1;
		return node;
}
	node = info->cmd_param[1] * TX_NUM + info->cmd_param[0];
	dev_info(&info->client->dev, "%s: node = %d\n", __func__,
			node);
	return node;

}

static void not_support_cmd(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buff[16] = {0};

	set_default_result(info);
	sprintf(buff, "%s", "NA");
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = 4;
	dev_info(&info->client->dev, "%s: \"%s(%d)\"\n", __func__,
				buff, strnlen(buff, sizeof(buff)));
	return;
}

static int mms_ts_core_fw_load(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	int ret;
	u8 fw_bin_ver = FW_VERSION;
	int retries = 3;
	const u8 *buff = 0;
	long fsize = 0;
	const struct firmware *tsp_fw = NULL;
	bool error = false;
	u8 ver;

	dev_info(&client->dev, "Entered REQ_FW\n");
	ret = request_firmware(&tsp_fw,
		"tsp_melfas/gd2/melfas.fw", &(client->dev));
	if (ret) {
		dev_err(&client->dev, "request firmware error!!\n");
		return 1;
	}
	dev_info(&client->dev,
		"fw_bin_ver = 0x%02x\n", fw_bin_ver);

	fsize = tsp_fw->size;
	buff = kzalloc((size_t)fsize, GFP_KERNEL);
	if (!buff) {
		dev_err(&client->dev, "fail to alloc buffer for fw\n");
		info->cmd_state = 3;
		release_firmware(tsp_fw);
		return 1;
	}

	memcpy((void *)buff, tsp_fw->data, fsize);
	release_firmware(tsp_fw);

	while (retries--) {
		if (error) {
			dev_err(&client->dev, "retry flashing\n");
			error = false;
		}
		i2c_lock_adapter(adapter);
		info->pdata->mux_fw_flash(true);

		ret = fw_download(info, (const u8 *)buff,
				(const size_t)fsize);

		info->pdata->mux_fw_flash(false);
		i2c_unlock_adapter(adapter);

		if (ret < 0) {
			dev_err(&client->dev, "fw_download fail\n");
			error = true;
			continue;
		}

		ver = get_fw_version(info, SEC_BOOTLOADER);
		if (ver != BOOT_VERSION) {
			dev_err(&client->dev, "boot version read fail\n");
			error = true;
			continue;
		}
		ver = get_fw_version(info, SEC_CORE);
		if (ver != CORE_VERSION) {
			dev_err(&client->dev, "core version read fail\n");
			error = true;
			continue;
		}
		info->fw_ic_ver = get_fw_version(info, SEC_CONFIG);
		if (info->fw_ic_ver != fw_bin_ver) {
			dev_err(&client->dev, "config version read fail\n");
			error = true;
			continue;
		}

		dev_info(&client->dev,
		  "ISP fw update done. ver = 0x%02x\n", info->fw_ic_ver);
		break;
	}
	if (error) {
		dev_err(&client->dev, "ERROR : fw update fail\n");
		kfree(buff);
		return 1;
	} else {
		dev_info(&client->dev, "fw update success\n");
		kfree(buff);
		return 0;
	}
}

static void fw_update(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	int ret = 0;
	int ver = 0, fw_bin_ver = FW_VERSION;
	int retries = 5;
	const u8 *buff = 0;
	mm_segment_t old_fs = {0};
	struct file *fp = NULL;
	long fsize = 0, nread = 0;
	const struct firmware *tsp_fw = NULL;
	char fw_path[MAX_FW_PATH+1];
	char result[16] = {0};

	set_default_result(info);
	dev_info(&client->dev,
		"fw_ic_ver = 0x%02x, fw_bin_ver = 0x%02x\n",
		info->fw_ic_ver, fw_bin_ver);

	if (info->cmd_param[0] == 0) {
		if (info->fw_core_ver > CORE_VERSION) {
			dev_info(&client->dev,
				"fw update does not need\n");
			info->cmd_state = 2;
			goto do_not_need_update;
		} else if (info->fw_core_ver == CORE_VERSION) {
			if (info->fw_ic_ver >= fw_bin_ver) {
				dev_info(&client->dev,
					"fw update does not need\n");
				info->cmd_state = 2;
				goto do_not_need_update;
			}
		} else { /* core < CORE_VERSION */
			dev_info(&client->dev,
				"fw update excute(core:0x%x)\n",
				info->fw_core_ver);
		}
	}

	switch (info->cmd_param[0]) {
	case BUILT_IN:
		dev_info(&client->dev, "built in fw is loaded!!\n");
		disable_irq(info->irq);
#if 0
		ret = mms100_ISC_download_mbinary(info, true, false);
#else
		ret = mms_ts_core_fw_load(info);
#endif
		ver = get_fw_version(info, SEC_CONFIG);
		info->fw_ic_ver = ver;
		if (ret == 0) {
			pr_err("[TSP] fw update success");
			info->cmd_state = 2;
		} else {
			pr_err("[TSP] fw update fail[%d]",
						ret);
			info->cmd_state = 3;
		}
		enable_irq(info->irq);
		return;
		break;

	case UMS:
		old_fs = get_fs();
		set_fs(get_ds());

		snprintf(fw_path, MAX_FW_PATH, "/sdcard/%s", TSP_FW_FILENAME);
		fp = filp_open(fw_path, O_RDONLY, 0);
		if (IS_ERR(fp)) {
			dev_err(&client->dev,
				"file %s open error:%d\n", fw_path, (s32)fp);
			info->cmd_state = 3;
			goto err_open;
		}

		fsize = fp->f_path.dentry->d_inode->i_size;

		buff = kzalloc((size_t)fsize, GFP_KERNEL);
		if (!buff) {
			dev_err(&client->dev, "fail to alloc buffer for fw\n");
			info->cmd_state = 3;
			goto err_alloc;
		}

		nread = vfs_read(fp, (char __user *)buff, fsize, &fp->f_pos);
		if (nread != fsize) {
			/*dev_err("fail to read file %s (nread = %d)\n",
					fw_path, nread);*/
			info->cmd_state = 3;
			goto err_fw_size;
		}

		filp_close(fp, current->files);
		set_fs(old_fs);
		dev_info(&client->dev, "ums fw is loaded!!\n");
		break;

	case REQ_FW:
		dev_info(&client->dev, "Entered REQ_FW case\n");
		ret = request_firmware(&tsp_fw, TSP_FW_FILENAME,
					&(client->dev));
		if (ret) {
			dev_err(&client->dev, "request firmware error!!\n");
			goto not_support;
		}

		fsize = tsp_fw->size;
		buff = kzalloc((size_t)fsize, GFP_KERNEL);
		if (!buff) {
			dev_err(&client->dev, "fail to alloc buffer for fw\n");
			info->cmd_state = 3;
			release_firmware(tsp_fw);
			goto not_support;
		}

		memcpy((void *)buff, tsp_fw->data, fsize);
		release_firmware(tsp_fw);
		break;

	default:
		dev_err(&client->dev, "invalid fw file type!!\n");
		goto not_support;
	}

	disable_irq(info->irq);
	while (retries--) {
		i2c_lock_adapter(adapter);
		info->pdata->mux_fw_flash(true);

		ret = fw_download(info, (const u8 *)buff,
				(const size_t)fsize);

		info->pdata->mux_fw_flash(false);
		i2c_unlock_adapter(adapter);

		if (ret < 0) {
			dev_err(&client->dev, "retry flashing\n");
			continue;
		}

		ver = get_fw_version(info, SEC_CONFIG);
		info->fw_ic_ver = ver;

		if (info->cmd_param[0] == 1 || info->cmd_param[0] == 2) {
			dev_info(&client->dev,
			  "fw update done. ver = 0x%02x\n", ver);
			info->cmd_state = 2;
			snprintf(result, sizeof(result) , "%s", "OK");
			set_cmd_result(info, result,
					strnlen(result, sizeof(result)));
			enable_irq(info->irq);
			kfree(buff);
			return;
		} else if (ver == fw_bin_ver) {
			dev_info(&client->dev,
			  "fw update done. ver = 0x%02x\n", ver);
			info->cmd_state = 2;
			snprintf(result, sizeof(result) , "%s", "OK");
			set_cmd_result(info, result,
					strnlen(result, sizeof(result)));
			enable_irq(info->irq);
			return;
		} else {
			dev_err(&client->dev,
					"ERROR : fw version is still wrong (0x%x != 0x%x)\n",
					ver, fw_bin_ver);
		}
		dev_err(&client->dev, "retry flashing\n");
	}

if (fp != NULL) {
err_fw_size:
	kfree(buff);
err_alloc:
	filp_close(fp, NULL);
err_open:
	set_fs(old_fs);
}
not_support:
do_not_need_update:
	snprintf(result, sizeof(result) , "%s", "NG");
	set_cmd_result(info, result, strnlen(result, sizeof(result)));
	return;
}

static void get_fw_ver_bin(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[16] = {0};

	set_default_result(info);
	snprintf(buff, sizeof(buff), "ME0054%02x", FW_VERSION);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = 2;
	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__,
			buff, strnlen(buff, sizeof(buff)));
}

static void get_fw_ver_ic(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[16] = {0};

	set_default_result(info);

	if (info->enabled) {
		info->fw_core_ver = get_fw_version(info, SEC_CORE);
		info->fw_ic_ver = get_fw_version(info, SEC_CONFIG);
	}
	snprintf(buff, sizeof(buff), "ME00%02x%02x",
		info->fw_core_ver, info->fw_ic_ver);

	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = 2;
	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__,
			buff, strnlen(buff, sizeof(buff)));
}

static void get_config_ver(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[20] = {0};

	set_default_result(info);

	snprintf(buff, sizeof(buff), "GN120_Me_0523");

	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = 2;
	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__,
			buff, strnlen(buff, sizeof(buff)));
}

static void get_threshold(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[16] = {0};
	int threshold;

	set_default_result(info);

	threshold = i2c_smbus_read_byte_data(info->client, 0x05);
	if (threshold < 0) {
		snprintf(buff, sizeof(buff), "%s", "NG");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = 3;
		return;
}
	snprintf(buff, sizeof(buff), "%d", threshold);

	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = 2;
	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__,
			buff, strnlen(buff, sizeof(buff)));
}

static void module_off_master(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[3] = {0};

	mms_ts_suspend(&info->client->dev);

	if (info->pdata->is_vdd_on() == 0)
		snprintf(buff, sizeof(buff), "%s", "OK");
	else
		snprintf(buff, sizeof(buff), "%s", "NG");

	set_default_result(info);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));

	if (strncmp(buff, "OK", 2) == 0)
		info->cmd_state = 2;
	else
		info->cmd_state = 3;

	info->cmd_is_running = false;

	dev_info(&info->client->dev, "%s: %s\n", __func__, buff);
}

static void module_on_master(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[3] = {0};

	mms_ts_resume(&info->client->dev);

	if (info->pdata->is_vdd_on() == 1)
		snprintf(buff, sizeof(buff), "%s", "OK");
	else
		snprintf(buff, sizeof(buff), "%s", "NG");

	set_default_result(info);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));

	if (strncmp(buff, "OK", 2) == 0)
		info->cmd_state = 2;
	else
		info->cmd_state = 3;

	info->cmd_is_running = false;

	dev_info(&info->client->dev, "%s: %s\n", __func__, buff);

}
/*
static void module_off_slave(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	not_support_cmd(info);
}

static void module_on_slave(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	not_support_cmd(info);
}
*/
static void get_chip_vendor(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[16] = {0};

	set_default_result(info);

	snprintf(buff, sizeof(buff), "%s", "MELFAS");
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = 2;
	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__,
			buff, strnlen(buff, sizeof(buff)));
}

static void get_chip_name(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[16] = {0};

	set_default_result(info);

	snprintf(buff, sizeof(buff), "%s", "MMS152");
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = 2;
	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__,
			buff, strnlen(buff, sizeof(buff)));
}

static void get_reference(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[16] = {0};
	unsigned int val;
	int node;

	set_default_result(info);
	node = check_rx_tx_num(info);

	if (node < 0)
		return;

	val = info->reference[node];
	snprintf(buff, sizeof(buff), "%u", val);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));

	info->cmd_state = 2;

	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__,
			buff, strnlen(buff, sizeof(buff)));

}

static void get_cm_abs(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[16] = {0};
	unsigned int val;
	int node;

	set_default_result(info);
	node = check_rx_tx_num(info);

	if (node < 0)
		return;

	val = info->raw[node];
	snprintf(buff, sizeof(buff), "%u", val);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = 2;

	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__, buff,
			strnlen(buff, sizeof(buff)));
}

static void get_cm_delta(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[16] = {0};
	unsigned int val;
	int node;

	set_default_result(info);
	node = check_rx_tx_num(info);

	if (node < 0)
		return;

	val = info->inspection[node];
	snprintf(buff, sizeof(buff), "%u", val);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = 2;

	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__, buff,
			strnlen(buff, sizeof(buff)));
}

static void get_intensity(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[16] = {0};
	unsigned int val;
	int node;

	set_default_result(info);
	node = check_rx_tx_num(info);

	if (node < 0)
		return;

	val = info->intensity[node];

	snprintf(buff, sizeof(buff), "%u", val);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = 2;

	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__, buff,
			strnlen(buff, sizeof(buff)));
}

static void get_x_num(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[16] = {0};
	int val;
	u8 r_buf[2];
	int ret;

	set_default_result(info);

	ret = i2c_smbus_read_i2c_block_data(info->client,
		ADDR_CH_NUM, 2, r_buf);
	val = r_buf[0];
	if (ret < 0) {
		snprintf(buff, sizeof(buff), "%s", "NG");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = 3;

			dev_info(&info->client->dev,
			"%s: fail to read num of x (%d).\n",
			__func__, val);
		return;
	}

	snprintf(buff, sizeof(buff), "%u", val);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = 2;

	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__, buff,
			strnlen(buff, sizeof(buff)));
}

static void get_y_num(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[16] = {0};
	int val;
	u8 r_buf[2];
	int ret;

	set_default_result(info);

	ret = i2c_smbus_read_i2c_block_data(info->client,
		ADDR_CH_NUM, 2, r_buf);
	val = r_buf[1];
	if (ret < 0) {
		snprintf(buff, sizeof(buff), "%s", "NG");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = 3;
			
		dev_info(&info->client->dev,
			"%s: fail to read num of x (%d).\n",
			__func__, val);
		return;
	}

	snprintf(buff, sizeof(buff), "%u", val);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = 2;

	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__, buff,
			strnlen(buff, sizeof(buff)));
}

static void run_reference_read(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	set_default_result(info);
	get_raw_data(info, MMS_VSC_CMD_REFER);
	info->cmd_state = 2;

/*	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__); */
}

static void run_cm_abs_read(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	set_default_result(info);
	get_raw_data(info, MMS_VSC_CMD_CM_ABS);
	info->cmd_state = 2;

/*	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__); */
}

static void run_cm_delta_read(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	set_default_result(info);
	get_raw_data(info, MMS_VSC_CMD_CM_DELTA);
	info->cmd_state = 2;

/*	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__); */
}

static void run_intensity_read(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	set_default_result(info);
	get_intensity_data(info);
	info->cmd_state = 2;

/*	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__); */
}

static ssize_t store_cmd(struct device *dev, struct device_attribute
				  *devattr, const char *buf, size_t count)
{
	struct mms_ts_info *info = dev_get_drvdata(dev);
	struct i2c_client *client = info->client;

	char *cur, *start, *end;
	char buff[TSP_CMD_STR_LEN] = {0};
	int len, i;
	struct tsp_cmd *tsp_cmd_ptr = NULL;
	char delim = ',';
	bool cmd_found = false;
	int param_cnt = 0;
	int ret;

	if (info->cmd_is_running == true) {
		dev_err(&info->client->dev, "tsp_cmd: other cmd is running.\n");
		goto err_out;
	}


	/* check lock  */
	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = true;
	mutex_unlock(&info->cmd_lock);

	info->cmd_state = 1;

	for (i = 0; i < ARRAY_SIZE(info->cmd_param); i++)
		info->cmd_param[i] = 0;

	len = (int)count;
	if (*(buf + len - 1) == '\n')
		len--;
	memset(info->cmd, 0x00, ARRAY_SIZE(info->cmd));
	memcpy(info->cmd, buf, len);

	cur = strchr(buf, (int)delim);
	if (cur)
		memcpy(buff, buf, cur - buf);
	else
		memcpy(buff, buf, len);

	/* find command */
	list_for_each_entry(tsp_cmd_ptr, &info->cmd_list_head, list) {
		if (!strcmp(buff, tsp_cmd_ptr->cmd_name)) {
			cmd_found = true;
			break;
		}
	}

	/* set not_support_cmd */
	if (!cmd_found) {
		list_for_each_entry(tsp_cmd_ptr, &info->cmd_list_head, list) {
			if (!strcmp("not_support_cmd", tsp_cmd_ptr->cmd_name))
				break;
		}
	}

	/* parsing parameters */
	if (cur && cmd_found) {
		cur++;
		start = cur;
		memset(buff, 0x00, ARRAY_SIZE(buff));
		do {
			if (*cur == delim || cur - buf == len) {
				end = cur;
				memcpy(buff, start, end - start);
				*(buff + strlen(buff)) = '\0';
				ret = kstrtoint(buff, 10,\
						info->cmd_param + param_cnt);
				start = cur + 1;
				memset(buff, 0x00, ARRAY_SIZE(buff));
				param_cnt++;
			}
			cur++;
		} while (cur - buf <= len);
	}

	dev_info(&client->dev, "cmd = %s\n", tsp_cmd_ptr->cmd_name);
	for (i = 0; i < param_cnt; i++)
		dev_info(&client->dev, "cmd param %d= %d\n", i,
							info->cmd_param[i]);

	tsp_cmd_ptr->cmd_func(info);


err_out:
	return count;
}

static ssize_t show_cmd_status(struct device *dev,
		struct device_attribute *devattr, char *buf)
{
	struct mms_ts_info *info = dev_get_drvdata(dev);
	char buff[16] = {0};

	dev_info(&info->client->dev, "tsp cmd: status:%d\n",
			info->cmd_state);

	if (info->cmd_state == 0)
		snprintf(buff, sizeof(buff), "WAITING");

	else if (info->cmd_state == 1)
		snprintf(buff, sizeof(buff), "RUNNING");

	else if (info->cmd_state == 2)
		snprintf(buff, sizeof(buff), "OK");

	else if (info->cmd_state == 3)
		snprintf(buff, sizeof(buff), "FAIL");

	else if (info->cmd_state == 4)
		snprintf(buff, sizeof(buff), "NOT_APPLICABLE");

	return snprintf(buf, TSP_BUF_SIZE, "%s\n", buff);
}

static ssize_t show_cmd_result(struct device *dev, struct device_attribute
				    *devattr, char *buf)
{
	struct mms_ts_info *info = dev_get_drvdata(dev);

	dev_info(&info->client->dev, "tsp cmd: result: %s\n", info->cmd_result);

	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);

	info->cmd_state = 0;

	return snprintf(buf, TSP_BUF_SIZE, "%s\n", info->cmd_result);
}

#ifdef ESD_DEBUG

static bool intensity_log_flag;

static u32 get_raw_data_one(struct mms_ts_info *info, u16 rx_idx, u16 tx_idx,
			    u8 cmd)
{
	u8 w_buf[6];
	u8 read_buffer[2];
	int ret;
	u32 raw_data;

	w_buf[0] = MMS_VSC_CMD;	/* vendor specific command id */
	w_buf[1] = MMS_VSC_MODE;	/* mode of vendor */
	w_buf[2] = 0;		/* tx line */
	w_buf[3] = 0;		/* rx line */
	w_buf[4] = 0;		/* reserved */
	w_buf[5] = 0;		/* sub command */

	if (cmd != MMS_VSC_CMD_INTENSITY && cmd != MMS_VSC_CMD_RAW &&
	    cmd != MMS_VSC_CMD_REFER) {
		dev_err(&info->client->dev, "%s: not profer command(cmd=%d)\n",
			__func__, cmd);
		return FAIL;
	}

	w_buf[2] = tx_idx;	/* tx */
	w_buf[3] = rx_idx;	/* rx */
	w_buf[5] = cmd;		/* sub command */

	ret = i2c_smbus_write_i2c_block_data(info->client, w_buf[0], 5,
					     &w_buf[1]);
	if (ret < 0)
		goto err_i2c;

	ret = i2c_smbus_read_i2c_block_data(info->client, 0xBF, 2, read_buffer);
	if (ret < 0)
		goto err_i2c;

	raw_data = ((u16) read_buffer[1] << 8) | read_buffer[0];
	if (cmd == MMS_VSC_CMD_REFER)
		raw_data = raw_data >> 4;

	return raw_data;

err_i2c:
	dev_err(&info->client->dev, "%s: fail to i2c (cmd=%d)\n",
		__func__, cmd);
	return FAIL;
}

static ssize_t show_intensity_logging_on(struct device *dev,
		struct device_attribute *devattr, char *buf)
{
	struct mms_ts_info *info = dev_get_drvdata(dev);
	struct i2c_client *client = info->client;
	struct file *fp;
	char log_data[160] = { 0, };
	char buff[16] = { 0, };
	mm_segment_t old_fs;
	long nwrite;
	u32 val;
	int i, y, c;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

#define MELFAS_DEBUG_LOG_PATH "/sdcard/melfas_log"

	dev_info(&client->dev, "%s: start.\n", __func__);
	fp = filp_open(MELFAS_DEBUG_LOG_PATH, O_RDWR | O_CREAT,
		       S_IRWXU | S_IRWXG | S_IRWXO);
	if (IS_ERR(fp)) {
		dev_err(&client->dev, "%s: fail to open log file\n", __func__);
		goto open_err;
	}

	intensity_log_flag = 1;
	do {
		for (y = 0; y < 3; y++) {
			/* for tx chanel 0~2 */
			memset(log_data, 0x00, 160);

			snprintf(buff, 16, "%1u: ", y);
			strncat(log_data, buff, strnlen(buff, 16));

			for (i = 0; i < RX_NUM; i++) {
				val = get_raw_data_one(info, i, y,
						       MMS_VSC_CMD_INTENSITY);
				snprintf(buff, 16, "%5u, ", val);
				strncat(log_data, buff, strnlen(buff, 16));
			}
			memset(buff, '\n', 2);
			c = (y == 2) ? 2 : 1;
			strncat(log_data, buff, c);
			nwrite = vfs_write(fp, (const char __user *)log_data,
					   strnlen(log_data, 160), &fp->f_pos);
		}
		usleep_range(3000, 5000);
	} while (intensity_log_flag);

	filp_close(fp, current->files);
	set_fs(old_fs);

	return 0;

open_err:
	set_fs(old_fs);
	return FAIL;
}

static ssize_t show_intensity_logging_off(struct device *dev,
		struct device_attribute *devattr, char *buf)
{
	struct mms_ts_info *info = dev_get_drvdata(dev);
	intensity_log_flag = 0;
	usleep_range(10000, 12000);
	get_raw_data_all(info, MMS_VSC_CMD_EXIT);
	return 0;
}

#endif

static DEVICE_ATTR(close_tsp_test, S_IRUGO, show_close_tsp_test, NULL);
static DEVICE_ATTR(cmd, S_IWUSR | S_IWGRP, NULL, store_cmd);
static DEVICE_ATTR(cmd_status, S_IRUGO, show_cmd_status, NULL);
static DEVICE_ATTR(cmd_result, S_IRUGO, show_cmd_result, NULL);
#ifdef ESD_DEBUG
static DEVICE_ATTR(intensity_logging_on, S_IRUGO, show_intensity_logging_on,
		   NULL);
static DEVICE_ATTR(intensity_logging_off, S_IRUGO, show_intensity_logging_off,
		   NULL);
#endif

static struct attribute *sec_touch_facotry_attributes[] = {
	&dev_attr_close_tsp_test.attr,
	&dev_attr_cmd.attr,
	&dev_attr_cmd_status.attr,
	&dev_attr_cmd_result.attr,
#ifdef ESD_DEBUG
	&dev_attr_intensity_logging_on.attr,
	&dev_attr_intensity_logging_off.attr,
#endif
	NULL,
};

static struct attribute_group sec_touch_factory_attr_group = {
	.attrs = sec_touch_facotry_attributes,
};
#endif /* SEC_TSP_FACTORY_TEST */

static int mms_ts_fw_check(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;
	int ret;
	u8 ver;
	char buf[4] = { 0, };

	ret = i2c_master_recv(client, buf, 1);
	if (ret < 0) {		/* tsp connect check */
		pr_err("%s: i2c fail...[%d], addr[%d]\n",
			   __func__, ret, info->client->addr);
		pr_err("%s: tsp driver unload\n", __func__);
		return ret;
	}

	ver = get_fw_version(info, SEC_BOOTLOADER);
	dev_err(&client->dev, "boot version : %d\n", ver);
	if (ver != BOOT_VERSION) {
		dev_err(&client->dev, "boot version must be 0x%x\n",
			BOOT_VERSION);
		dev_err(&client->dev, "excute core firmware update\n");
		ret = mms_ts_core_fw_load(info);
		if (ret) {
			dev_err(&client->dev,
				"failed to initialize (%d)\n",
				ret);
			return ret;
		}
	}

	info->fw_core_ver = get_fw_version(info, SEC_CORE);
	dev_info(&client->dev, "core version : 0x%02x\n",
		info->fw_core_ver);

	if ((info->fw_core_ver < CORE_VERSION) ||
		(info->fw_core_ver == 0xff)) {
		dev_err(&client->dev, "core version must be 0x%x\n",
			CORE_VERSION);
		dev_err(&client->dev, "excute core firmware update\n");
		ret = mms_ts_core_fw_load(info);
		if (ret) {
			dev_err(&client->dev,
				"failed to initialize (%d)\n",
				ret);
			return ret;
		}
		info->fw_core_ver =
			get_fw_version(info, SEC_CORE);
	}

	info->fw_ic_ver = get_fw_version(info, SEC_CONFIG);
	dev_info(&client->dev, "config version : 0x%02x\n",
		info->fw_ic_ver);
	if ((info->fw_ic_ver != FW_VERSION) &&
		(info->fw_core_ver == CORE_VERSION)) {
		dev_err(&client->dev, "firmware update\n");
		dev_err(&client->dev, "ic:0x%x, bin:0x%x\n",
			info->fw_ic_ver, FW_VERSION);
		ret = mms_ts_fw_load(info, false);
		if (ret) {
			dev_err(&client->dev,
				"failed to initialize (%d)\n",
				ret);
			return ret;
		}
	}
	return 0;
}

static int  mms_ts_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct mms_ts_info *info;
	struct input_dev *input_dev;
	int ret = 0;
	int boot_ver = 0;

      printk("mms_ts_probe !!!!!!!!!!!!!!! \n");
#ifdef SEC_TSP_FACTORY_TEST
	int i;
	struct device *fac_dev_ts;
#endif
	touch_is_pressed = 0;

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -EIO;

	info = kzalloc(sizeof(struct mms_ts_info), GFP_KERNEL);
	if (!info) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	input_dev = input_allocate_device();
	if (!input_dev) {
		dev_err(&client->dev, "Failed to allocate memory for input device\n");
		ret = -ENOMEM;
		goto err_input_alloc;
	}

	info->client = client;
	info->input_dev = input_dev;
	info->pdata = client->dev.platform_data;
	if (NULL == info->pdata) {
		pr_err("failed to get platform data\n");
		goto err_config;
	}
	info->irq = -1;
	mutex_init(&info->lock);

	if (info->pdata) {
		info->max_x = info->pdata->max_x;
		info->max_y = info->pdata->max_y;
		info->invert_x = info->pdata->invert_x;
		info->invert_y = info->pdata->invert_y;
		info->lcd_type = info->pdata->lcd_type;
		info->input_event = info->pdata->input_event;
		info->register_cb = info->pdata->register_cb;
#ifdef CONFIG_LCD_FREQ_SWITCH
		info->register_lcd_cb = info->pdata->register_lcd_cb;
#endif
	} else {
		info->max_x = 480;
		info->max_y = 800;
	}

	i2c_set_clientdata(client, info);

	info->pdata->power(true);
	msleep(250);

      mms100_get_version_info(client);

	if (info->pdata->mux_fw_flash == NULL) {
		dev_err(&client->dev,
			"fw cannot be updated, missing platform data\n");
		goto err_config;
	}

	boot_ver = get_fw_version(info, SEC_BOOTLOADER);
	info->fw_core_ver = get_fw_version(info, SEC_CORE);
	info->fw_ic_ver = get_fw_version(info, SEC_CONFIG);

	dev_info(&client->dev, "boot : %d core : 0x%02x config : 0x%02x",
			boot_ver, info->fw_core_ver, info->fw_ic_ver);

#if 0
	ret = mms_ts_fw_check(info);
	if (ret) {
		dev_err(&client->dev,
			"failed to initialize (%d)\n", ret);
		goto err_config;
	}
#endif

	info->callbacks.inform_charger = melfas_ta_cb;
	if (info->register_cb)
		info->register_cb(&info->callbacks);
#ifdef CONFIG_LCD_FREQ_SWITCH
	info->lcd_callback.inform_lcd = melfas_lcd_cb;
	if (info->register_lcd_cb)
		info->register_lcd_cb(&info->lcd_callback);
	info->tsp_lcdfreq_flag = 0;
#endif

	snprintf(info->phys, sizeof(info->phys),
		 "%s/input0", dev_name(&client->dev));
	input_dev->name = "sec_touchscreen";
	input_dev->phys = info->phys;
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;

      	input_mt_init_slots(input_dev, MAX_FINGERS,0);

	__set_bit(EV_ABS, input_dev->evbit);
	__set_bit(INPUT_PROP_DIRECT, input_dev->propbit);

	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR,
				0, MAX_PRESSURE, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MINOR,
				0, MAX_PRESSURE, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X,
				0, (info->max_x)-1, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y,
				0, (info->max_y)-1, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR,
				0, MAX_WIDTH, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_ANGLE,
				MIN_ANGLE, MAX_ANGLE, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PALM,
				0, 1, 0, 0);
	input_set_drvdata(input_dev, info);

#if MMS_HAS_TOUCH_KEY
	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(KEY_MENU, input_dev->keybit);
	__set_bit(KEY_BACK, input_dev->keybit);
#endif

	ret = input_register_device(input_dev);
	if (ret) {
		dev_err(&client->dev, "failed to register input dev (%d)\n",
			ret);
		goto err_reg_input_dev;
	}

#if TOUCH_BOOSTER
	mutex_init(&info->dvfs_lock);
	INIT_DELAYED_WORK(&info->work_dvfs_off, set_dvfs_off);
	INIT_DELAYED_WORK(&info->work_dvfs_chg, change_dvfs_lock);
	bus_dev = dev_get("exynos-busfreq");
	info->cpufreq_level = -1;
	info->dvfs_lock_status = false;
#endif

	info->enabled = true;
	info->irq_check = true;

	ret = request_threaded_irq(client->irq, NULL, mms_ts_interrupt,
				   IRQF_TRIGGER_FALLING  | IRQF_ONESHOT,
				   MELFAS_TS_NAME, info);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to register interrupt\n");
		goto err_req_irq;
	}
	info->irq = client->irq;

#ifdef CONFIG_HAS_EARLYSUSPEND
	info->early_suspend.level = EARLY_SUSPEND_LEVEL_STOP_DRAWING;
	info->early_suspend.suspend = mms_ts_early_suspend;
	info->early_suspend.resume = mms_ts_late_resume;
	register_early_suspend(&info->early_suspend);
#endif

	sec_touchscreen = device_create(sec_class,
					NULL, 0, info, "sec_touchscreen");
	if (IS_ERR(sec_touchscreen)) {
		dev_err(&client->dev,
			"Failed to create device for the sysfs1\n");
		ret = -ENODEV;
	}

#ifdef SEC_TSP_FACTORY_TEST
		INIT_LIST_HEAD(&info->cmd_list_head);
		for (i = 0; i < ARRAY_SIZE(tsp_cmds); i++)
			list_add_tail(&tsp_cmds[i].list, &info->cmd_list_head);

		mutex_init(&info->cmd_lock);
		info->cmd_is_running = false;

	fac_dev_ts = device_create(sec_class,
			NULL, 0, info, "tsp");
	if (IS_ERR(fac_dev_ts))
		dev_err(&client->dev, "Failed to create device for the sysfs\n");

	ret = sysfs_create_group(&fac_dev_ts->kobj,
				 &sec_touch_factory_attr_group);
	if (ret)
		dev_err(&client->dev, "Failed to create sysfs group\n");
#endif
	return 0;

err_req_irq:
	input_unregister_device(input_dev);
err_reg_input_dev:
err_config:
	input_free_device(input_dev);
	/*input_dev = NULL;*/
err_input_alloc:
	kfree(info);
err_alloc:
	return ret;

}

static int mms_ts_remove(struct i2c_client *client)
{
	struct mms_ts_info *info = i2c_get_clientdata(client);

	unregister_early_suspend(&info->early_suspend);
	if (info->irq >= 0)
		free_irq(info->irq, info);
	input_unregister_device(info->input_dev);
	kfree(info);

	return 0;
}

#if defined(CONFIG_PM) || defined(CONFIG_HAS_EARLYSUSPEND)
static int mms_ts_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mms_ts_info *info = i2c_get_clientdata(client);

	if (!info->enabled)
		return 0;

      printk("%s called \n", __func__);

	dev_notice(&info->client->dev, "%s: users=%d\n", __func__,
		   info->input_dev->users);

	disable_irq(info->irq);
	info->enabled = false;
	touch_is_pressed = 0;
#ifdef CONFIG_LCD_FREQ_SWITCH
	info->tsp_lcdfreq_flag = 0;
#endif
	release_all_fingers(info);
	info->pdata->power(0);   
	info->sleep_wakeup_ta_check = info->ta_status;
	/* This delay needs to prevent unstable POR by
	rapid frequently pressing of PWR key. */
	msleep(50);
	return 0;
}

static int mms_ts_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mms_ts_info *info = i2c_get_clientdata(client);

       printk("%s called \n", __func__);

	if (info->enabled)
		return 0;

	dev_notice(&info->client->dev, "%s: users=%d\n", __func__,
		   info->input_dev->users);
	info->pdata->power(1); 
	msleep(120); 

	if (info->ta_status) {
		dev_notice(&client->dev, "TA connect!!!\n");
		i2c_smbus_write_byte_data(info->client, 0x33, 0x1);
	} else {
		dev_notice(&client->dev, "TA disconnect!!!\n");
		i2c_smbus_write_byte_data(info->client, 0x33, 0x2);
	}


	info->enabled = true;
	mms_set_noise_mode(info);
#if 0
	if (info->fw_ic_ver >= 0x21) {
		if ((info->ta_status == 1) &&
			(info->sleep_wakeup_ta_check == 0)) {
			dev_notice(&client->dev,
				"TA connect!!! %s\n", __func__);
			i2c_smbus_write_byte_data(info->client, 0x32, 0x1);
		}
	}
#endif
	/* Because irq_type by EXT_INTxCON register is changed to low_level
	 *  after wakeup, irq_type set to falling edge interrupt again.
	 */
	enable_irq(info->irq);
	return 0;
}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mms_ts_early_suspend(struct early_suspend *h)
{
	struct mms_ts_info *info;
      printk("%s called \n", __func__);
     // return ; /* 20130913 TEMP */
	info = container_of(h, struct mms_ts_info, early_suspend);
	mms_ts_suspend(&info->client->dev);

}

static void mms_ts_late_resume(struct early_suspend *h)
{
	struct mms_ts_info *info;
      printk("%s called \n", __func__);
      //return ; /* 20130913 TEMP */

	info = container_of(h, struct mms_ts_info, early_suspend);
	mms_ts_resume(&info->client->dev);
}
#endif

#if defined(CONFIG_PM) && !defined(CONFIG_HAS_EARLYSUSPEND)
static const struct dev_pm_ops mms_ts_pm_ops = {
	.suspend = mms_ts_suspend,
	.resume = mms_ts_resume,
#ifdef CONFIG_HIBERNATION
	.freeze = mms_ts_suspend,
	.thaw = mms_ts_resume,
	.restore = mms_ts_resume,
#endif
};
#endif

static const struct i2c_device_id mms_ts_id[] = {
	{MELFAS_TS_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, mms_ts_id);

static struct i2c_driver mms_ts_driver = {
	.probe = mms_ts_probe,
	.remove = mms_ts_remove,
	.driver = {
		   .name = MELFAS_TS_NAME,
#if defined(CONFIG_PM) && !defined(CONFIG_HAS_EARLYSUSPEND)
		   .pm = &mms_ts_pm_ops,
#endif
		   },
	.id_table = mms_ts_id,
};

static int __init mms_ts_init(void)
{

	return i2c_add_driver(&mms_ts_driver);

}

static void __exit mms_ts_exit(void)
{
	i2c_del_driver(&mms_ts_driver);
}

module_init(mms_ts_init);
module_exit(mms_ts_exit);

/* Module information */
MODULE_DESCRIPTION("Touchscreen driver for Melfas MMS-series controllers");
MODULE_LICENSE("GPL");
