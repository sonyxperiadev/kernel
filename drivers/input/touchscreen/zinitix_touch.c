/*
 *
 * Zinitix touch driver
 *
 * Copyright (C) 2009 Zinitix, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 */
/*
Version 2.0.0 : using reg data file (2010/11/05)
Version 2.0.1 : syntxt bug fix (2010/11/09)
Version 2.0.2 : Save status cmd delay bug (2010/11/10)
Version 2.0.3 : modify delay 10ms -> 50ms for clear hw calibration bit
	: modify ZINITIX_TOTAL_NUMBER_OF_Y register (read only -> read/write )
	: modify SUPPORTED FINGER NUM register (read only -> read/write )
Version 2.0.4 : [20101116]
	Modify Firmware Upgrade routine.
Version 2.0.5 : [20101118]
	add esd timer function & some bug fix.
	you can select request_threaded_irq or
	request_irq, setting USE_THREADED_IRQ.
Version 2.0.6 : [20101123]
	add ABS_MT_WIDTH_MAJOR Report
Version 2.0.7 : [20101201]
	Modify zinitix_early_suspend() / zinitix_late_resume() routine.
Version 2.0.8 : [20101216]
	add using spin_lock option
Version 2.0.9 : [20101216]
	Test Version
Version 2.0.10 : [20101217]
	add USE_THREAD_METHOD option.
	if USE_THREAD_METHOD = 0, you use workqueue
Version 2.0.11 : [20101229]
	add USE_UPDATE_SYSFS option for update firmware.
	&& TOUCH_MODE == 1 mode.
Version 2.0.13 : [20110125]
	modify esd timer routine
Version 2.0.14 : [20110217]
	esd timer bug fix. (kernel panic)
	sysfs bug fix.
Version 2.0.15 : [20110315]
	add power off delay ,250ms
Version 2.0.16 : [20110316]
	add upgrade method using isp
Version 2.0.17 : [20110406]
	change naming rule : sain -> zinitix
	(add) pending interrupt skip
	add isp upgrade mode
	remove warning message when complile
Version 3.0.2 : [20110711]
	support bt4x3 series
Version 3.0.3 : [20110720]
	add raw data monitoring func.
	add the h/w calibration skip option.
Version 3.0.4 : [20110728]
	fix using semaphore bug.
Version 3.0.5 : [20110801]
	fix some bugs.
Version 3.0.6 : [20110802]
	fix Bt4x3 isp upgrade bug.
	add USE_TS_MISC_DEVICE option for showing info & upgrade
	remove USE_UPDATE_SYSFS option
Version 3.0.7 : [201108016]
	merge USE_TS_MISC_DEVICE option and USE_TEST_RAW_TH_DATA_MODE
	fix work proceedure bug.
Version 3.0.8 / Version 3.0.9 : [201108017]
	add ioctl func.
Version 3.0.10 : [201108030]
	support REAL_SUPPORTED_FINGER_NUM
Version 3.0.11 : [201109014]
	support zinitix apps.
Version 3.0.12 : [201109015]
	add disable touch event func.
Version 3.0.13 : [201109015]
	add USING_CHIP_SETTING option :  interrupt mask/button num/finger num
Version 3.0.14 : [201109020]
	support apps. above kernel version 2.6.35
Version 3.0.15 : [201101004]
	modify timing in firmware upgrade retry when failt to upgrade firmware
Version 3.0.16 : [20111024]
	remove thread method.
	add resume/suspend function
	check code using checkpatch.pl
Version 3.0.19 : [20111124]
	modify isp timing : 2ms -> 1ms
Version 3.0.20 : [20120201]
	support icecream sandwich
	remove bt4x2 option
*/

#include <linux/module.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/hrtimer.h>
#include <linux/ioctl.h>
#include <linux/earlysuspend.h>
#include <linux/string.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/firmware.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <mach/gpio.h>
#include <linux/uaccess.h>
#include <linux/regulator/consumer.h>

#include "zinitix_touch.h"
#include "zinitix_touch_bt4x3_firmware.h"
#include "zinitix_touch_bt4x3_reg_data.h"

#define TSP_INT 86
#define TOUCH_EN 22 //PSJ

#define TOUCH_ON 1
#define TOUCH_OFF 0

static struct regulator *touch_regulator=NULL;

#define USE_THREADED_IRQ 1

#define	ZINITIX_DEBUG		0

static	int	m_ts_debug_mode = ZINITIX_DEBUG;

#define	SYSTEM_MAX_X_RESOLUTION	240
#define	SYSTEM_MAX_Y_RESOLUTION	320


/* interrupt pin number*/
#define GPIO_TOUCH_PIN_NUM	86
/* interrupt pin IRQ number */
#define GPIO_TOUCH_IRQ	0

#if USE_THREADED_IRQ
/* Do Nothing */
#else
static struct workqueue_struct *zinitix_workqueue;
#endif

static struct workqueue_struct *zinitix_tmr_workqueue;

#define	zinitix_debug_msg(fmt, args...)	\
	if (m_ts_debug_mode)	\
		printk(KERN_INFO "[%-18s:%5d]" fmt, \
		__func__, __LINE__, ## args);

struct _ts_zinitix_coord {
	u16	x;
	u16	y;
	u8	width;
	u8	sub_status;
};

struct _ts_zinitix_point_info {
	u16	status;
	u8	finger_cnt;
	u8	time_stamp;
	struct _ts_zinitix_coord	coord[MAX_SUPPORTED_FINGER_NUM];
};

#define	TOUCH_V_FLIP	0x01
#define	TOUCH_H_FLIP	0x02
#define	TOUCH_XY_SWAP	0x04

struct _ts_capa_info {
	u16 chip_revision;
	u16 chip_firmware_version;
	u16 chip_reg_data_version;
	u16 x_resolution;
	u16 y_resolution;
	u32 chip_fw_size;
	u32 MaxX;
	u32 MaxY;
	u32 MinX;
	u32 MinY;
	u32 Orientation;
	u8 gesture_support;
	u16 multi_fingers;
	u16 button_num;
	u16 chip_int_mask;
#if	USE_TEST_RAW_TH_DATA_MODE
	u16 x_node_num;
	u16 y_node_num;
	u16 total_node_num;
	u16 max_y_node;
	u16 total_cal_n;
#endif
};

enum _ts_work_proceedure {
	TS_NO_WORK = 0,
	TS_NORMAL_WORK,
	TS_ESD_TIMER_WORK,
	TS_IN_EALRY_SUSPEND,
	TS_IN_SUSPEND,
	TS_IN_RESUME,
	TS_IN_LATE_RESUME,
	TS_IN_UPGRADE,
	TS_REMOVE_WORK,
	TS_SET_MODE,
	TS_HW_CALIBRAION,
};

struct zinitix_touch_dev {
	struct input_dev *input_dev;
	struct task_struct *task;
	wait_queue_head_t	wait;
	struct work_struct	work;
	struct work_struct	tmr_work;
	struct i2c_client *client;
	struct semaphore update_lock;
	u32 i2c_dev_addr;
	struct _ts_capa_info	cap_info;
	char	phys[32];

	bool is_valid_event;
	struct _ts_zinitix_point_info touch_info;
	struct _ts_zinitix_point_info reported_touch_info;
	u16 icon_event_reg;
	u16 event_type;
	u32 int_gpio_num;
	u32 irq;
	u8 button[MAX_SUPPORTED_BUTTON_NUM];
	u8 work_proceedure;
	struct semaphore work_proceedure_lock;
	u8	use_esd_timer;

	bool in_esd_timer;
	struct timer_list esd_timeout_tmr;
	struct timer_list *p_esd_timeout_tmr;

#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif

#if	USE_TEST_RAW_TH_DATA_MODE
	struct semaphore	raw_data_lock;
	u16 raw_mode_flag;
	s16 ref_data[MAX_TEST_RAW_DATA];
	s16 cur_data[MAX_RAW_DATA];
	u8 update;
#endif
};


#define ZINITIX_DRIVER_NAME        "Zinitix_tsp"

#define ZINITIX_ISP_NAME	"zinitix_isp"
static struct i2c_client *m_isp_client;

static char IsfwUpdate[20]={0};

#define FW_DOWNLOADING "Downloading"
#define FW_DOWNLOAD_COMPLETE "Complete"
#define FW_DOWNLOAD_FAIL "FAIL"
#define FWUP_NOW -1

static struct i2c_device_id zinitix_idtable[] = {
	{ZINITIX_ISP_NAME, 0},
	{ZINITIX_DRIVER_NAME, 0},
	{ }
};

struct zinitix_touch_dev* g_touch_dev;

/* sys fs */
struct class *touch_class;
EXPORT_SYMBOL(touch_class);
struct device *firmware_dev;
EXPORT_SYMBOL(firmware_dev);
//static ssize_t firmware_show(struct device *dev, struct device_attribute *attr, char *buf);
//static ssize_t firmware_sw_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t zinitix_enter_testmode(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t zinitix_enter_normalmode(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t rawdata_pass_fail_zinitix(struct device *dev, struct device_attribute *attr, char *buf);

static ssize_t phone_firmware_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t part_firmware_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t threshold_firmware_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t config_firmware_show(struct device *dev, struct device_attribute *attr, char *buf);

//static ssize_t tsp_StartCall_show(struct device *dev, struct device_attribute *attr, char *buf);
//static ssize_t tsp_EndCall_show(struct device *dev, struct device_attribute *attr, char *buf);
//static ssize_t tsp_xy_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t firmware_update(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t firmware_update_status(struct device *dev, struct device_attribute *attr, char *buf);
//static ssize_t firmware_store( struct device *dev, struct device_attribute *attr, const char *buf, size_t size);

static DEVICE_ATTR(raw_enable, 0444, zinitix_enter_testmode, NULL) ;
static DEVICE_ATTR(raw_disable, 0444, zinitix_enter_normalmode, NULL) ;
static DEVICE_ATTR(raw_value, 0444, rawdata_pass_fail_zinitix, NULL) ;

//static DEVICE_ATTR(firmware_ver	, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, firmware_show, NULL);
//static DEVICE_ATTR(sw_ver	, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, firmware_sw_show, NULL);
static DEVICE_ATTR(phone_fw_ver, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, phone_firmware_show, NULL);
static DEVICE_ATTR(part_fw_ver, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, part_firmware_show, NULL);
static DEVICE_ATTR(threshold_fw_ver, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, threshold_firmware_show, NULL);
static DEVICE_ATTR(config_fw_ver, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, config_firmware_show, NULL);

//static DEVICE_ATTR(tsp_StartCall	, S_IRUGO | S_IWUSR | S_IWGRP | S_IXOTH, tsp_StartCall_show, NULL);
//static DEVICE_ATTR(tsp_EndCall	, S_IRUGO | S_IWUSR | S_IWGRP | S_IXOTH, tsp_EndCall_show, NULL);
//static DEVICE_ATTR(debug_xy, S_IRUGO | S_IWUSR | S_IWGRP | S_IXOTH, tsp_xy_show, NULL);
static DEVICE_ATTR(fw_update, 0777, firmware_update, NULL);
static DEVICE_ATTR(fw_upstatus, S_IRUGO | S_IWUSR | S_IWGRP | S_IXOTH, firmware_update_status, NULL);
//static DEVICE_ATTR(fw_update, S_IWUSR | S_IWGRP, NULL, firmware_update);

/* sys fs */

/*<= you must set key button mapping*/
u32 BUTTON_MAPPING_KEY[MAX_SUPPORTED_BUTTON_NUM] = {KEY_MENU, KEY_HOME, KEY_BACK};


void touch_delay(int time)
{
	mdelay(time*5);
}

/* define i2c sub functions*/
inline s32 ts_read_data(struct i2c_client *client,
	u16 reg, u8 *values, u16 length)
{
	s32 ret;
	/* select register*/
	ret = i2c_master_send(client , (u8 *)&reg , 2);
	zinitix_debug_msg("%s, %d, ret : %d\n", __func__, __LINE__, ret );	
	if (ret < 0)
		return ret;
	
	/* for setup tx transaction. */
	udelay(DELAY_FOR_TRANSCATION);
	ret = i2c_master_recv(client , values , length);
	zinitix_debug_msg("%s, %d, ret : %d\n", __func__, __LINE__, ret );		
	if (ret < 0)
		return ret;
	
	udelay(DELAY_FOR_POST_TRANSCATION);
	return length;
}

inline s32 ts_write_data(struct i2c_client *client,
	u16 reg, u8 *values, u16 length)
{
	s32 ret;
	u8	pkt[4];
	pkt[0] = (reg)&0xff;
	pkt[1] = (reg >> 8)&0xff;
	pkt[2] = values[0];
	pkt[3] = values[1];
	ret = i2c_master_send(client , pkt , length+2);
	if (ret < 0)
		return ret;
	udelay(DELAY_FOR_POST_TRANSCATION);
	return length;
}

inline s32 ts_write_reg(struct i2c_client *client, u16 reg, u16 value)
{
	if (ts_write_data(client, reg, (u8 *)&value, 2) < 0)
		return I2C_FAIL;
	return I2C_SUCCESS;
}

inline s32 ts_write_cmd(struct i2c_client *client, u16 reg)
{
	s32 ret;
	ret = i2c_master_send(client , (u8 *)&reg , 2);
	if (ret < 0)
		return I2C_FAIL;
	udelay(DELAY_FOR_POST_TRANSCATION);
	return I2C_SUCCESS;
}

inline s32 ts_read_raw_data(struct i2c_client *client,
		u16 reg, u8 *values, u16 length)
{
	s32 ret;
	/* select register */
	ret = i2c_master_send(client , (u8 *)&reg , 2);
	if (ret < 0)
		return ret;
	/* for setup tx transaction. */
	udelay(200);
	ret = i2c_master_recv(client , values , length);
	if (ret < 0)
		return ret;
	udelay(DELAY_FOR_POST_TRANSCATION);
	return length;
}



inline s32 ts_read_firmware_data(struct i2c_client *client,
	char *addr, u8 *values, u16 length)
{
	s32 ret;
	if (addr != NULL) {
		/* select register*/
		ret = i2c_master_send(client , addr , 2);
		if (ret < 0)
			return ret;
		/* for setup tx transaction. */
		touch_delay(1);
	}
	ret = i2c_master_recv(client , values , length);
	if (ret < 0)
		return ret;
	udelay(DELAY_FOR_POST_TRANSCATION);
	return length;
}

inline s32 ts_write_firmware_data(struct i2c_client *client,
	u8 *values, u16 length)
{
	s32 ret;
	ret = i2c_master_send(client , values , length);
	if (ret < 0)
		return ret;
	udelay(DELAY_FOR_POST_TRANSCATION);
	return length;
}

static int zinitix_touch_probe(struct i2c_client *client,
	const struct i2c_device_id *i2c_id);
static int zinitix_touch_remove(struct i2c_client *client);
static bool ts_init_touch(struct zinitix_touch_dev *touch_dev);
static void zinitix_clear_report_data(struct zinitix_touch_dev *touch_dev);
//static int zinitix_resume(struct i2c_client *client);
//static int zinitix_suspend(struct i2c_client *client, pm_message_t message);


#ifdef CONFIG_HAS_EARLYSUSPEND
static void zinitix_early_suspend(struct early_suspend *h);
static void zinitix_late_resume(struct early_suspend *h);
#endif


static void ts_esd_timer_start(u16 sec, struct zinitix_touch_dev *touch_dev);
static void ts_esd_timer_stop(struct zinitix_touch_dev *touch_dev);
static void ts_esd_timer_init(struct zinitix_touch_dev *touch_dev);
static void ts_esd_timeout_handler(unsigned long data);


#if USE_TEST_RAW_TH_DATA_MODE
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
static int ts_misc_fops_ioctl(struct inode *inode, struct file *filp,
	unsigned int cmd, unsigned long arg);
#else
static long ts_misc_fops_ioctl(struct file *filp,
	unsigned int cmd, unsigned long arg);
#endif
static int ts_misc_fops_open(struct inode *inode, struct file *filp);
static int ts_misc_fops_close(struct inode *inode, struct file *filp);

static const struct file_operations ts_misc_fops = {
	.owner = THIS_MODULE,
	.open = ts_misc_fops_open,
	.release = ts_misc_fops_close,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
	.ioctl = ts_misc_fops_ioctl,
#else
	.unlocked_ioctl = ts_misc_fops_ioctl,
#endif
};

static struct miscdevice touch_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "zinitix_touch_misc",
	.fops = &ts_misc_fops,
};

#define TOUCH_IOCTL_BASE	0xbc
#define TOUCH_IOCTL_GET_DEBUGMSG_STATE		_IOW(TOUCH_IOCTL_BASE, 0, int)
#define TOUCH_IOCTL_SET_DEBUGMSG_STATE		_IOW(TOUCH_IOCTL_BASE, 1, int)
#define TOUCH_IOCTL_GET_CHIP_REVISION		_IOW(TOUCH_IOCTL_BASE, 2, int)
#define TOUCH_IOCTL_GET_FW_VERSION		_IOW(TOUCH_IOCTL_BASE, 3, int)
#define TOUCH_IOCTL_GET_REG_DATA_VERSION	_IOW(TOUCH_IOCTL_BASE, 4, int)
#define TOUCH_IOCTL_VARIFY_UPGRADE_SIZE		_IOW(TOUCH_IOCTL_BASE, 5, int)
#define TOUCH_IOCTL_VARIFY_UPGRADE_DATA		_IOW(TOUCH_IOCTL_BASE, 6, int)
#define TOUCH_IOCTL_START_UPGRADE		_IOW(TOUCH_IOCTL_BASE, 7, int)
#define TOUCH_IOCTL_GET_X_NODE_NUM		_IOW(TOUCH_IOCTL_BASE, 8, int)
#define TOUCH_IOCTL_GET_Y_NODE_NUM		_IOW(TOUCH_IOCTL_BASE, 9, int)
#define TOUCH_IOCTL_GET_TOTAL_NODE_NUM		_IOW(TOUCH_IOCTL_BASE, 10, int)
#define TOUCH_IOCTL_SET_RAW_DATA_MODE		_IOW(TOUCH_IOCTL_BASE, 11, int)
#define TOUCH_IOCTL_GET_RAW_DATA		_IOW(TOUCH_IOCTL_BASE, 12, int)
#define TOUCH_IOCTL_GET_X_RESOLUTION		_IOW(TOUCH_IOCTL_BASE, 13, int)
#define TOUCH_IOCTL_GET_Y_RESOLUTION		_IOW(TOUCH_IOCTL_BASE, 14, int)
#define TOUCH_IOCTL_HW_CALIBRAION		_IOW(TOUCH_IOCTL_BASE, 15, int)
#define TOUCH_IOCTL_GET_REG			_IOW(TOUCH_IOCTL_BASE, 16, int)
#define TOUCH_IOCTL_SET_REG			_IOW(TOUCH_IOCTL_BASE, 17, int)
#define TOUCH_IOCTL_SEND_SAVE_STATUS		_IOW(TOUCH_IOCTL_BASE, 18, int)
#define TOUCH_IOCTL_DONOT_TOUCH_EVENT		_IOW(TOUCH_IOCTL_BASE, 19, int)


struct zinitix_touch_dev *misc_touch_dev;

#endif /*USE_TEST_RAW_TH_DATA_MODE */



/* id -> include/linux/i2c-id.h	*/
static struct i2c_driver zinitix_touch_driver = {
	.probe	= zinitix_touch_probe,
	.remove	= zinitix_touch_remove,
	//.suspend = zinitix_suspend,
	//.resume	= zinitix_resume,
	.id_table	= zinitix_idtable,
	.driver		= {
		.name	= ZINITIX_DRIVER_NAME,
	},
};

#if	USE_TEST_RAW_TH_DATA_MODE
static bool ts_get_raw_data(struct zinitix_touch_dev *touch_dev)
{
	u32 total_node = touch_dev->cap_info.total_node_num;
	u32 sz;
	u16 udata;
	down(&touch_dev->raw_data_lock);
	if (touch_dev->raw_mode_flag == TOUCH_TEST_RAW_MODE) {
		sz = (total_node*2 + MAX_TEST_POINT_INFO*2);


		if (ts_read_raw_data(touch_dev->client,
			ZINITIX_RAWDATA_REG,
			(char *)touch_dev->cur_data, sz) < 0) {
				printk(KERN_INFO "error : read zinitix tc raw data\n");
				up(&touch_dev->raw_data_lock);
				return false;
		}

		/* no point, so update ref_data*/
		udata = touch_dev->cur_data[total_node];

		if (!zinitix_bit_test(udata, BIT_ICON_EVENT)
			&& !zinitix_bit_test(udata, BIT_PT_EXIST))
			memcpy((u8 *)touch_dev->ref_data,
				(u8 *)touch_dev->cur_data, total_node*2);

		touch_dev->update = 1;
		memcpy((u8 *)(&touch_dev->touch_info),
			(u8 *)&touch_dev->cur_data[total_node],
			sizeof(struct _ts_zinitix_point_info));
		up(&touch_dev->raw_data_lock);
		return true;
	}  else if (touch_dev->raw_mode_flag != TOUCH_NORMAL_MODE) {
		zinitix_debug_msg("read raw data\r\n");
		sz = total_node*2 + sizeof(struct _ts_zinitix_point_info);


		if (touch_dev->raw_mode_flag == TOUCH_ZINITIX_CAL_N_MODE) {
			int total_cal_n = touch_dev->cap_info.total_cal_n;
			if (total_cal_n == 0)
				total_cal_n = 160;

			sz =
			(total_cal_n*2 + sizeof(struct _ts_zinitix_point_info));

			if (ts_read_raw_data(touch_dev->client,
				ZINITIX_RAWDATA_REG,
				(char *)touch_dev->cur_data, sz) < 0) {
				printk(KERN_INFO "error : read zinitix tc raw data\n");
				up(&touch_dev->raw_data_lock);
				return false;
			}
			misc_touch_dev->update = 1;
			memcpy((u8 *)(&touch_dev->touch_info),
				(u8 *)&touch_dev->cur_data[total_cal_n],
				sizeof(struct _ts_zinitix_point_info));
			up(&touch_dev->raw_data_lock);
			return true;
		}

		if (ts_read_raw_data(touch_dev->client,
			ZINITIX_RAWDATA_REG,
			(char *)touch_dev->cur_data, sz) < 0) {
			printk(KERN_INFO "error : read zinitix tc raw data\n");
			up(&touch_dev->raw_data_lock);
			return false;
		}

		udata = touch_dev->cur_data[total_node];
		if (!zinitix_bit_test(udata, BIT_ICON_EVENT)
			&& !zinitix_bit_test(udata, BIT_PT_EXIST))
			memcpy((u8 *)touch_dev->ref_data,
				(u8 *)touch_dev->cur_data, total_node*2);
		touch_dev->update = 1;
		memcpy((u8 *)(&touch_dev->touch_info),
			(u8 *)&touch_dev->cur_data[total_node],
			sizeof(struct _ts_zinitix_point_info));
	}
	up(&touch_dev->raw_data_lock);
	return true;
}
#endif


static bool ts_get_samples(struct zinitix_touch_dev *touch_dev)
{
	int i;

	zinitix_debug_msg("ts_get_samples+\r\n");

	if (gpio_get_value(touch_dev->int_gpio_num)) {
				/*interrupt pin is high, not valid data.*/
		zinitix_debug_msg("woops... inturrpt pin is high\r\n");
		return false;
	}

#if	USE_TEST_RAW_TH_DATA_MODE
	if (touch_dev->raw_mode_flag != TOUCH_NORMAL_MODE) {
		if (ts_get_raw_data(touch_dev) == false)
			return false;
		goto continue_check_point_data;
	}
#endif

	if (ts_read_data(touch_dev->client,
		ZINITIX_POINT_STATUS_REG,
		(u8 *)(&touch_dev->touch_info),
		sizeof(struct _ts_zinitix_point_info)) < 0) {
		zinitix_debug_msg("error read point info using i2c.-\n");
				return false;
	}

continue_check_point_data:
	zinitix_debug_msg("status = 0x%x , pt cnt = %d, t stamp = %d\n",
		touch_dev->touch_info.status,
		touch_dev->touch_info.finger_cnt,
		touch_dev->touch_info.time_stamp);

	if (touch_dev->touch_info.status == 0x0
		&& touch_dev->touch_info.finger_cnt == 100) {
		zinitix_debug_msg("periodical esd repeated int occured\r\n");
		return true;
	}

	for (i = 0; i < MAX_SUPPORTED_BUTTON_NUM; i++)
		touch_dev->button[i] = ICON_BUTTON_UNCHANGE;

	if (zinitix_bit_test(touch_dev->touch_info.status, BIT_ICON_EVENT)) {
		udelay(20);
		if (ts_read_data(touch_dev->client,
			ZINITIX_ICON_STATUS_REG,
			(u8 *)(&touch_dev->icon_event_reg), 2) < 0) {
			printk(KERN_INFO "error read icon info using i2c.\n");
					return false;
		}
	}
	zinitix_debug_msg("ts_get_samples-\r\n");

	return true;
}


static irqreturn_t ts_int_handler(int irq, void *dev)
{

#if USE_THREADED_IRQ	
      return IRQ_WAKE_THREAD;
#else
	struct zinitix_touch_dev *touch_dev = (struct zinitix_touch_dev *)dev;
	/* zinitix_debug_msg("interrupt occured +\r\n"); */
	/* remove pending interrupt */
		zinitix_debug_msg("ts_int_handler\r\n");	
	if (gpio_get_value(touch_dev->int_gpio_num)) {
		zinitix_debug_msg("invalid interrupt occured +\r\n");
			 return IRQ_HANDLED;
	}

	//disable_irq_nosync(irq);
	queue_work(zinitix_workqueue, &touch_dev->work);
	return IRQ_HANDLED;
#endif

}

static bool ts_read_coord(struct zinitix_touch_dev *hDevice)
{
	struct zinitix_touch_dev *touch_dev =
		(struct zinitix_touch_dev *)hDevice;
	if (ts_get_samples(touch_dev) == false)
		return false;

	ts_write_cmd(touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
	udelay(DELAY_FOR_SIGNAL_DELAY);
	return true;
}

void touch_ctrl_regulator(unsigned char on_off)
{
	int ret;
	
		if(touch_regulator == NULL)
		{
			zinitix_debug_msg(" %s, %d \n", __func__, __LINE__ );
			touch_regulator = regulator_get(NULL, "hv10"); 
			zinitix_debug_msg(" %s, %d \n", __func__, __LINE__ );			
			if(IS_ERR(touch_regulator)){
				zinitix_debug_msg("can not get VTOUCH_3.3V\n");
			}	
			//ret = regulator_set_voltage(touch_regulator,3300000,3300000);	
			//ret = regulator_enable(touch_regulator);			
		}	

	if(on_off==1)
	{
		zinitix_debug_msg(" %s, %d Touch On\n", __func__, __LINE__ );	
		
		//if(!regulator_is_enabled(touch_regulator))
		{
			zinitix_debug_msg(" %s, %d \n", __func__, __LINE__ );	
			ret = regulator_set_voltage(touch_regulator,3300000,3300000);
			zinitix_debug_msg("regulator_set_voltage ret = %d \n", ret);       			
			ret = regulator_enable(touch_regulator);
			zinitix_debug_msg("regulator_enable ret = %d \n", ret);       			
		} 

	}
	else
	{
		zinitix_debug_msg("%s, %d TOUCH Off\n", __func__, __LINE__ );	
	
		//if(regulator_is_enabled(touch_regulator))
		{
			ret = regulator_disable(touch_regulator);
			zinitix_debug_msg("regulator_disable ret = %d \n", ret);			
		}

	}

}
EXPORT_SYMBOL(touch_ctrl_regulator);

static void ts_power_control(struct zinitix_touch_dev *touch_dev, u8 ctl)
{
	if (ctl == POWER_OFF)
		touch_ctrl_regulator(0);
	else if (ctl == POWER_ON)
		touch_ctrl_regulator(1);

}

static bool ts_mini_init_touch(struct zinitix_touch_dev *touch_dev)
{
	if (touch_dev == NULL) {
		printk(KERN_INFO "ts_mini_init_touch : error (touch_dev == NULL?)\r\n");
		return false;
	}

	ts_init_touch(touch_dev);

	if (touch_dev->use_esd_timer) {
		ts_esd_timer_start(ZINITIX_CHECK_ESD_TIMER, touch_dev);
		zinitix_debug_msg("esd timer start\r\n");
	}

	return true;
}


static void zinitix_touch_tmr_work(struct work_struct *work)
{
	struct zinitix_touch_dev *touch_dev =
		container_of(work, struct zinitix_touch_dev, tmr_work);

	printk(KERN_INFO "tmr queue work ++\r\n");
	if (touch_dev == NULL) {
		printk(KERN_INFO "touch dev == NULL ?\r\n");
		goto fail_time_out_init;
	}
	down(&touch_dev->work_proceedure_lock);
	if (touch_dev->work_proceedure != TS_NO_WORK) {
		printk(KERN_INFO "other process occupied (%d)\r\n",
			touch_dev->work_proceedure);
		up(&touch_dev->work_proceedure_lock);
		return;
	}
	touch_dev->work_proceedure = TS_ESD_TIMER_WORK;

	//disable_irq(touch_dev->irq);
	printk(KERN_INFO "error. timeout occured. maybe ts device dead. so reset & reinit.\r\n");
	touch_delay(CHIP_POWER_OFF_DELAY);
	ts_power_control(touch_dev, POWER_OFF);
	touch_delay(CHIP_POWER_OFF_DELAY);
	ts_power_control(touch_dev, POWER_ON);
	touch_delay(CHIP_ON_DELAY);

	zinitix_debug_msg("clear all reported points\r\n");
	zinitix_clear_report_data(touch_dev);
	if (ts_mini_init_touch(touch_dev) == false)
		goto fail_time_out_init;

	touch_dev->work_proceedure = TS_NO_WORK;
	//enable_irq(touch_dev->irq);
	up(&touch_dev->work_proceedure_lock);
	printk(KERN_INFO "tmr queue work ----\r\n");
	return;
fail_time_out_init:
	printk(KERN_INFO "tmr work : restart error\r\n");
	ts_esd_timer_start(ZINITIX_CHECK_ESD_TIMER, touch_dev);
	touch_dev->work_proceedure = TS_NO_WORK;
	//enable_irq(touch_dev->irq);
	up(&touch_dev->work_proceedure_lock);
}

static void ts_esd_timer_start(u16 sec, struct zinitix_touch_dev *touch_dev)
{
	if (touch_dev->p_esd_timeout_tmr != NULL)
		del_timer(touch_dev->p_esd_timeout_tmr);
	touch_dev->p_esd_timeout_tmr = NULL;

	init_timer(&(touch_dev->esd_timeout_tmr));
	touch_dev->esd_timeout_tmr.data = (unsigned long)(touch_dev);
	touch_dev->esd_timeout_tmr.function = ts_esd_timeout_handler;
	touch_dev->esd_timeout_tmr.expires = jiffies + HZ*sec;
	touch_dev->p_esd_timeout_tmr = &touch_dev->esd_timeout_tmr;
	add_timer(&touch_dev->esd_timeout_tmr);
}

static void ts_esd_timer_stop(struct zinitix_touch_dev *touch_dev)
{
	if (touch_dev->p_esd_timeout_tmr)
		del_timer(touch_dev->p_esd_timeout_tmr);
	touch_dev->p_esd_timeout_tmr = NULL;
}

static void ts_esd_timer_init(struct zinitix_touch_dev *touch_dev)
{
	init_timer(&(touch_dev->esd_timeout_tmr));
	touch_dev->esd_timeout_tmr.data = (unsigned long)(touch_dev);
	touch_dev->esd_timeout_tmr.function = ts_esd_timeout_handler;
	touch_dev->p_esd_timeout_tmr = NULL;
}

static void ts_esd_timeout_handler(unsigned long data)
{
	struct zinitix_touch_dev *touch_dev = (struct zinitix_touch_dev *)data;
	touch_dev->p_esd_timeout_tmr = NULL;
	queue_work(zinitix_tmr_workqueue, &touch_dev->tmr_work);
}

#if	TOUCH_ONESHOT_UPGRADE
static bool ts_check_need_upgrade(u16 curVersion, u16 curRegVersion)
{
	u16	newVersion;
	newVersion = (u16) (m_firmware_data[0] | (m_firmware_data[1]<<8));

	printk(KERN_INFO "cur Version = 0x%x, new Version = 0x%x\n",
		curVersion, newVersion);

	if (curVersion < newVersion)
		return true;
	else if (curVersion > newVersion)
		return false;



	if (m_firmware_data[FIRMWARE_VERSION_POS+2] == 0xff
		&& m_firmware_data[FIRMWARE_VERSION_POS+3] == 0xff)
		return false;
	newVersion = (u16) (m_firmware_data[FIRMWARE_VERSION_POS+2]
		| (m_firmware_data[FIRMWARE_VERSION_POS+3]<<8));


	if (curRegVersion < newVersion)
		return true;

	return false;
}
#endif

#define	TC_PAGE_SZ		64
#define	TC_SECTOR_SZ		8

static u8 ts_upgrade_firmware(struct zinitix_touch_dev *touch_dev,
	const u8 *firmware_data, u32 size)
{
	u16 flash_addr;
	u8 *verify_data;
	int retry_cnt = 0;
	u8 i2c_buffer[TC_PAGE_SZ+2];

	if (m_isp_client == NULL) {
		printk(KERN_ERR "i2c client for isp is not register \r\n");
		return false;
	}

	verify_data = kzalloc(size, GFP_KERNEL);
	if (verify_data == NULL) {
		printk(KERN_ERR "cannot alloc verify buffer\n");
		return false;
	}

retry_isp_firmware_upgrade:
	ts_power_control(touch_dev, POWER_OFF);
	touch_delay(CHIP_POWER_OFF_AF_FZ_DELAY);
	ts_power_control(touch_dev, POWER_ON);
	touch_delay(1);		/* under 4ms*/

	printk(KERN_INFO "write firmware data\n");

	for (flash_addr = 0; flash_addr < size; flash_addr += TC_PAGE_SZ) {

#if !USE_HW_CALIBRATION
		if (flash_addr >= CALIBRATION_AREA*2)
			break;
#endif
		printk(KERN_INFO ".");
		zinitix_debug_msg("firmware write : addr = %04x, len = %d\n",
			flash_addr, TC_PAGE_SZ);

		i2c_buffer[0] = (flash_addr>>8)&0xff;	/*addr_h*/
		i2c_buffer[1] = (flash_addr)&0xff;	/*addr_l*/
		memcpy(&i2c_buffer[2], &firmware_data[flash_addr], TC_PAGE_SZ);

		if (ts_write_firmware_data(m_isp_client,
			i2c_buffer, TC_PAGE_SZ+2) < 0) {
			printk(KERN_INFO"error : write zinitix tc firmare\n");
			goto fail_upgrade;
		}
		touch_delay(20);
	}
	touch_delay(CHIP_POWER_OFF_AF_FZ_DELAY);

	printk(KERN_INFO "\r\nread firmware data\n");


	flash_addr = 0;
	i2c_buffer[0] = (flash_addr>>8)&0xff;	/*addr_h*/
	i2c_buffer[1] = (flash_addr)&0xff;	/*addr_l*/

#if !USE_HW_CALIBRATION
	size = CALIBRATION_AREA*2;
#endif
	if (ts_read_firmware_data(m_isp_client,
		i2c_buffer, &verify_data[flash_addr], size) < 0) {
		printk(KERN_INFO "error : read zinitix tc firmare: addr = %04x, len = %d\n",
			flash_addr, size);
		goto fail_upgrade;
	}
	if (memcmp((u8 *)&firmware_data[flash_addr],
		(u8 *)&verify_data[flash_addr], size) != 0) {
		printk(KERN_INFO "error : verify error : addr = %04x, len = %d\n",
			flash_addr, size);
		goto fail_upgrade;
	}

	touch_delay(CHIP_POWER_OFF_AF_FZ_DELAY);
	ts_power_control(touch_dev, POWER_OFF);
	touch_delay(CHIP_POWER_OFF_AF_FZ_DELAY);
	ts_power_control(touch_dev, POWER_ON);
	touch_delay(CHIP_ON_AF_FZ_DELAY);
	printk(KERN_INFO "upgrade finished\n");
	kfree(verify_data);
	return true;

fail_upgrade:

	touch_delay(CHIP_POWER_OFF_AF_FZ_DELAY);
	ts_power_control(touch_dev, POWER_OFF);
	touch_delay(CHIP_POWER_OFF_AF_FZ_DELAY);
	ts_power_control(touch_dev, POWER_ON);
	touch_delay(CHIP_ON_AF_FZ_DELAY);

	printk(KERN_INFO "upgrade fail : so retry... (%d)\n", ++retry_cnt);
	if (retry_cnt <= ZINITIX_INIT_RETRY_CNT)
		goto retry_isp_firmware_upgrade;

	if (verify_data != NULL)
		kfree(verify_data);

	printk(KERN_INFO "upgrade fail..\n");
	return false;

}


static bool ts_init_touch(struct zinitix_touch_dev *touch_dev)
{
	u16 reg_val;
	int i;
	u16 SetMaxX = SYSTEM_MAX_X_RESOLUTION;
	u16 SetMaxY = SYSTEM_MAX_Y_RESOLUTION;
	u16 chip_revision;
	u16 chip_firmware_version;
	u16 chip_reg_data_version;
	u16 chip_eeprom_info;
#if USE_CHECKSUM	
	u16 chip_check_reg0, chip_check_reg1, chip_check_reg2, chip_check_reg3;
	u8 checksum_err;
#endif	
#if	TOUCH_ONESHOT_UPGRADE
	//	s16 stmp;
#endif
	int retry_cnt = 0;

	if (touch_dev == NULL) {
		printk(KERN_ERR "error touch_dev == null?\r\n");
		return false;
	}


retry_init:
	zinitix_debug_msg("check checksum\r\n");
#if USE_CHECKSUM
	checksum_err = 0;

	for (i = 0; i < ZINITIX_INIT_RETRY_CNT; i++) {
		if (ts_read_data(touch_dev->client, 
			ZINITIX_CHECK_REG0,
			(u8 *)&chip_check_reg0, 2) < 0) {
				touch_delay(50);
				continue;
			}
		if (ts_read_data(touch_dev->client, 
			ZINITIX_CHECK_REG1,
			(u8 *)&chip_check_reg1, 2) < 0){
				touch_delay(50);
				continue;
			}
		if (ts_read_data(touch_dev->client, 
			ZINITIX_CHECK_REG2,
			(u8 *)&chip_check_reg2, 2) < 0){
				touch_delay(50);
				continue;
			}
		if (ts_read_data(touch_dev->client, 
			ZINITIX_CHECK_REG3,
			(u8 *)&chip_check_reg3, 2) < 0){
				touch_delay(50);
				continue;
			}
		if( (chip_check_reg0==chip_check_reg2) && (chip_check_reg1==chip_check_reg3) )		
			break;
		else {
			checksum_err = 1;
			break;
		}
	}
	
	if (i == ZINITIX_INIT_RETRY_CNT || checksum_err) {
		printk(KERN_INFO "fail to check firmware data\r\n");
		if(checksum_err == 1 && retry_cnt < ZINITIX_INIT_RETRY_CNT)
			retry_cnt = ZINITIX_INIT_RETRY_CNT;
		goto fail_init;
	}
#endif

	if (ts_write_cmd(touch_dev->client,
		ZINITIX_SWRESET_CMD) != I2C_SUCCESS) {
		printk(KERN_INFO "fail to write reset command\r\n");
		goto fail_init;
	}

#if USING_CHIP_SETTING
	if (ts_read_data(touch_dev->client,
		ZINITIX_INT_ENABLE_FLAG,
		(u8 *)&touch_dev->cap_info.chip_int_mask, 2) < 0)
		goto fail_init;
	printk(KERN_INFO "zinitix touch interrupt mask = %04x\r\n",
		touch_dev->cap_info.chip_int_mask);
	if (touch_dev->cap_info.chip_int_mask == 0
		|| touch_dev->cap_info.chip_int_mask == 0xffff)
		goto fail_init;
#else
	touch_dev->cap_info.button_num = SUPPORTED_BUTTON_NUM;

	reg_val = 0;
	zinitix_bit_set(reg_val, BIT_PT_CNT_CHANGE);
	zinitix_bit_set(reg_val, BIT_DOWN);
	zinitix_bit_set(reg_val, BIT_MOVE);
	zinitix_bit_set(reg_val, BIT_UP);
	if (touch_dev->cap_info.button_num > 0)
		zinitix_bit_set(reg_val, BIT_ICON_EVENT);
	touch_dev->cap_info.chip_int_mask = reg_val;
#endif


	if (ts_write_reg(touch_dev->client,
		ZINITIX_INT_ENABLE_FLAG, 0x0) != I2C_SUCCESS)
		goto fail_init;

	zinitix_debug_msg("send reset command\r\n");
	if (ts_write_cmd(touch_dev->client,
		ZINITIX_SWRESET_CMD) != I2C_SUCCESS)
		goto fail_init;

	/* get chip revision id */
	if (ts_read_data(touch_dev->client,
		ZINITIX_CHIP_REVISION,
		(u8 *)&chip_revision, 2) < 0) {
		printk(KERN_INFO "fail to read chip revision\r\n");
		goto fail_init;
	}
	printk(KERN_INFO "zinitix touch chip revision id = %x\r\n",
		chip_revision);

	touch_dev->cap_info.chip_fw_size = 32*1024;

#if	USE_TEST_RAW_TH_DATA_MODE
	if (ts_read_data(touch_dev->client,
		ZINITIX_TOTAL_NUMBER_OF_X,
		(u8 *)&touch_dev->cap_info.x_node_num, 2) < 0)
		goto fail_init;
	printk(KERN_INFO "zinitix touch chip x node num = %d\r\n",
		touch_dev->cap_info.x_node_num);
	if (ts_read_data(touch_dev->client,
		ZINITIX_TOTAL_NUMBER_OF_Y,
		(u8 *)&touch_dev->cap_info.y_node_num, 2) < 0)
		goto fail_init;
	printk(KERN_INFO "zinitix touch chip y node num = %d\r\n",
		touch_dev->cap_info.y_node_num);

	touch_dev->cap_info.total_node_num =
		touch_dev->cap_info.x_node_num*touch_dev->cap_info.y_node_num;
	printk(KERN_INFO "zinitix touch chip total node num = %d\r\n",
		touch_dev->cap_info.total_node_num);

	if (ts_read_data(touch_dev->client,
		ZINITIX_MAX_Y_NUM,
		(u8 *)&touch_dev->cap_info.max_y_node, 2) < 0)
		goto fail_init;

	printk(KERN_INFO "zinitix touch chip max y node num = %d\r\n",
		touch_dev->cap_info.max_y_node);

	if (ts_read_data(touch_dev->client,
		ZINITIX_CAL_N_TOTAL_NUM,
		(u8 *)&touch_dev->cap_info.total_cal_n, 2) < 0)
		goto fail_init;
	printk(KERN_INFO "zinitix touch chip total cal n data num = %d\r\n",
		touch_dev->cap_info.total_cal_n);


#endif


	/* get chip firmware version */
	if (ts_read_data(touch_dev->client,
		ZINITIX_FIRMWARE_VERSION,
		(u8 *)&chip_firmware_version, 2) < 0)
		goto fail_init;
	printk(KERN_INFO "zinitix touch chip firmware version = %x\r\n",
		chip_firmware_version);

#if	TOUCH_ONESHOT_UPGRADE
	chip_reg_data_version = 0xffff;

	if (ts_read_data(touch_dev->client,
		ZINITIX_DATA_VERSION_REG,
		(u8 *)&chip_reg_data_version, 2) < 0)
		goto fail_init;
	zinitix_debug_msg("touch reg data version = %d\r\n",
		chip_reg_data_version);

	if (ts_check_need_upgrade(chip_firmware_version,
		chip_reg_data_version) == true) {
		printk(KERN_INFO "start upgrade firmware\n");

		ts_upgrade_firmware(touch_dev, &m_firmware_data[2],
			touch_dev->cap_info.chip_fw_size);

		/* get chip revision id */
		if (ts_read_data(touch_dev->client,
			ZINITIX_CHIP_REVISION,
			(u8 *)&chip_revision, 2) < 0) {
			printk(KERN_INFO "fail to read chip revision\r\n");
			goto fail_init;
		}
		printk(KERN_INFO "zinitix touch chip revision id = %x\r\n",
			chip_revision);

		/* get chip firmware version */
		if (ts_read_data(touch_dev->client,
			ZINITIX_FIRMWARE_VERSION,
			(u8 *)&chip_firmware_version, 2) < 0)
			goto fail_init;
		printk(KERN_INFO "zinitix touch chip renewed firmware version = %x\r\n",
			chip_firmware_version);

	}
#endif

	if (ts_read_data(touch_dev->client,
		ZINITIX_DATA_VERSION_REG,
		(u8 *)&chip_reg_data_version, 2) < 0)
		goto fail_init;
	zinitix_debug_msg("touch reg data version = %d\r\n",
		chip_reg_data_version);

#if 0//	TOUCH_ONESHOT_UPGRADE
	if (chip_reg_data_version
		< m_reg_data[ZINITIX_DATA_VERSION_REG].reg_val) {
		zinitix_debug_msg("write new reg data( %d < %d)\r\n",
			chip_reg_data_version,
			m_reg_data[ZINITIX_DATA_VERSION_REG].reg_val);
		for (i = 0; i < MAX_REG_COUNT; i++) {
			if (m_reg_data[i].valid == 1) {
				if (ts_write_reg(touch_dev->client,
					(u16)i,
					(u16)(m_reg_data[i].reg_val))
					!= I2C_SUCCESS)
					goto fail_init;
				if (i == ZINITIX_TOTAL_NUMBER_OF_X
					|| i == ZINITIX_TOTAL_NUMBER_OF_Y)
					touch_delay(50);
				if (ts_read_data(touch_dev->client,
					(u16)i, (u8 *)&stmp, 2) < 0)
					goto fail_init;
				if (memcmp((char *)&m_reg_data[i].reg_val,
					(char *)&stmp, 2) < 0)
					printk(KERN_WARNING "register data is different. (addr = 0x%02X , %d != %d)\r\n",
						i, m_reg_data[i].reg_val, stmp);
			}
		}
		zinitix_debug_msg("done new reg data( %d < %d)\r\n",
			chip_reg_data_version,
			m_reg_data[ZINITIX_DATA_VERSION_REG].reg_val);
		if (ts_write_cmd(touch_dev->client,
			ZINITIX_SAVE_STATUS_CMD) != I2C_SUCCESS)
			goto fail_init;
		touch_delay(1000);	/* for fusing eeprom */
	}

#endif

	if (ts_read_data(touch_dev->client,
		ZINITIX_EEPROM_INFO_REG,
		(u8 *)&chip_eeprom_info, 2) < 0)
		goto fail_init;
	zinitix_debug_msg("touch eeprom info = 0x%04X\r\n", chip_eeprom_info);

#if	USE_HW_CALIBRATION
	if (zinitix_bit_test(chip_eeprom_info, 0)) { /* hw calibration bit*/

		if (touch_dev->cap_info.chip_int_mask != 0)
			if (ts_write_reg(touch_dev->client,
				ZINITIX_INT_ENABLE_FLAG,
				touch_dev->cap_info.chip_int_mask)
				!= I2C_SUCCESS)
				goto fail_init;
			
		 /* h/w calibration */
		 touch_delay(100);
		if (ts_write_reg(touch_dev->client,
			ZINITIX_TOUCH_MODE, 0x07) != I2C_SUCCESS)
			goto fail_init;
		touch_delay(250);
		if (ts_write_cmd(touch_dev->client,
			ZINITIX_CALIBRATE_CMD) != I2C_SUCCESS)
			goto fail_init;
		if (ts_write_cmd(touch_dev->client,
			ZINITIX_SWRESET_CMD) != I2C_SUCCESS)
			goto fail_init;
		touch_delay(10);

		/* wait for h/w calibration*/
		do {
			ts_write_cmd(touch_dev->client,
					ZINITIX_CLEAR_INT_STATUS_CMD);
			touch_delay(1000);
			if (ts_read_data(touch_dev->client,
				ZINITIX_EEPROM_INFO_REG,
				(u8 *)&chip_eeprom_info, 2) < 0)
				goto fail_init;
			zinitix_debug_msg("touch eeprom info = 0x%04X\r\n",
				chip_eeprom_info);
			if (!zinitix_bit_test(chip_eeprom_info, 0))
				break;
		} while (1);

		if (ts_write_reg(touch_dev->client,
			ZINITIX_TOUCH_MODE, TOUCH_MODE) != I2C_SUCCESS)
			goto fail_init;


		touch_delay(10);
		if (ts_write_cmd(touch_dev->client,
			ZINITIX_SWRESET_CMD) != I2C_SUCCESS)
			goto fail_init;

		touch_delay(10);
		if (ts_write_cmd(touch_dev->client,
			ZINITIX_SAVE_CALIBRATION_CMD) != I2C_SUCCESS)
			goto fail_init;
		touch_delay(500);


		/* disable chip interrupt */
		if (ts_write_reg(touch_dev->client,
			ZINITIX_INT_ENABLE_FLAG, 0) != I2C_SUCCESS)
			goto fail_init;

	}
#endif

	touch_dev->cap_info.chip_revision = (u16)chip_revision;
	touch_dev->cap_info.chip_firmware_version = (u16)chip_firmware_version;
	touch_dev->cap_info.chip_reg_data_version = (u16)chip_reg_data_version;


	/* initialize */
	if (ts_write_reg(touch_dev->client,
		ZINITIX_X_RESOLUTION,
		(u16)(SetMaxX)) != I2C_SUCCESS)
		goto fail_init;

	if (ts_write_reg(touch_dev->client,
		ZINITIX_Y_RESOLUTION,
		(u16)(SetMaxY)) != I2C_SUCCESS)
		goto fail_init;

	if (ts_read_data(touch_dev->client,
		ZINITIX_X_RESOLUTION,
		(u8 *)&touch_dev->cap_info.x_resolution, 2) < 0)
		goto fail_init;
	zinitix_debug_msg("touch max x = %d\r\n",
		touch_dev->cap_info.x_resolution);
	if (ts_read_data(touch_dev->client,
		ZINITIX_Y_RESOLUTION,
		(u8 *)&touch_dev->cap_info.y_resolution, 2) < 0)
		goto fail_init;
	zinitix_debug_msg("touch max y = %d\r\n",
		touch_dev->cap_info.y_resolution);

	touch_dev->cap_info.MinX = (u32)0;
	touch_dev->cap_info.MinY = (u32)0;
	touch_dev->cap_info.MaxX = (u32)touch_dev->cap_info.x_resolution;
	touch_dev->cap_info.MaxY = (u32)touch_dev->cap_info.y_resolution;

#if USING_CHIP_SETTING
	if (ts_read_data(touch_dev->client,
		ZINITIX_BUTTON_SUPPORTED_NUM,
		(u8 *)&touch_dev->cap_info.button_num, 2) < 0)
		goto fail_init;
	zinitix_debug_msg("supported button num = %d\r\n",
		touch_dev->cap_info.button_num);
	if (ts_read_data(touch_dev->client,
		ZINITIX_SUPPORTED_FINGER_NUM,
		(u8 *)&touch_dev->cap_info.multi_fingers, 2) < 0)
		goto fail_init;
	zinitix_debug_msg("supported finger num = %d\r\n",
		touch_dev->cap_info.multi_fingers);
#else
	if (ts_write_reg(touch_dev->client,
		ZINITIX_BUTTON_SUPPORTED_NUM,
		(u16)touch_dev->cap_info.button_num) != I2C_SUCCESS)
		goto fail_init;

	if (ts_write_reg(touch_dev->client,
		ZINITIX_SUPPORTED_FINGER_NUM,
		(u16)MAX_SUPPORTED_FINGER_NUM) != I2C_SUCCESS)
		goto fail_init;
	touch_dev->cap_info.multi_fingers = REAL_SUPPORTED_FINGER_NUM;
#endif

	zinitix_debug_msg("max supported finger num = %d, \
		real supported finger num = %d\r\n",
		touch_dev->cap_info.multi_fingers, REAL_SUPPORTED_FINGER_NUM);
	touch_dev->cap_info.gesture_support = 0;
	zinitix_debug_msg("set other configuration\r\n");

#if	USE_TEST_RAW_TH_DATA_MODE
	if (touch_dev->raw_mode_flag != TOUCH_NORMAL_MODE) {	/* Test Mode */
		if (ts_write_reg(touch_dev->client,
			ZINITIX_TOUCH_MODE,
			touch_dev->raw_mode_flag) != I2C_SUCCESS)
			goto fail_init;
	} else
#endif
{
	reg_val = TOUCH_MODE;
	if (ts_write_reg(touch_dev->client,
		ZINITIX_TOUCH_MODE,
		reg_val) != I2C_SUCCESS)
		goto fail_init;
}
	/* soft calibration */
	if (ts_write_cmd(touch_dev->client,
		ZINITIX_CALIBRATE_CMD) != I2C_SUCCESS)
		goto fail_init;
	if (ts_write_reg(touch_dev->client,
		ZINITIX_INT_ENABLE_FLAG,
		touch_dev->cap_info.chip_int_mask) != I2C_SUCCESS)
		goto fail_init;

	/* read garbage data */
	for (i = 0; i < 10; i++)	{
		ts_write_cmd(touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
		udelay(10);
	}

#if	USE_TEST_RAW_TH_DATA_MODE
	if (touch_dev->raw_mode_flag != TOUCH_NORMAL_MODE) { /* Test Mode */
		if (ts_write_reg(touch_dev->client,
			ZINITIX_PERIODICAL_INTERRUPT_INTERVAL,
			ZINITIX_SCAN_RATE_HZ
			*ZINITIX_RAW_DATA_ESD_TIMER_INTERVAL) != I2C_SUCCESS)
			printk(KERN_INFO "[zinitix_touch] Fail to set ZINITIX_RAW_DATA_ESD_TIMER_INTERVAL.\r\n");
	} else
#endif
{
	if (ZINITIX_ESD_TIMER_INTERVAL) {
		if (ts_write_reg(touch_dev->client,
			ZINITIX_PERIODICAL_INTERRUPT_INTERVAL,
			ZINITIX_SCAN_RATE_HZ
			*ZINITIX_ESD_TIMER_INTERVAL) != I2C_SUCCESS)
			goto fail_init;
		if (ts_read_data(touch_dev->client,
			ZINITIX_PERIODICAL_INTERRUPT_INTERVAL,
			(u8 *)&reg_val, 2) < 0)
			goto fail_init;
		zinitix_debug_msg("esd timer register = %d\r\n", reg_val);
	}
}

	zinitix_debug_msg("successfully initialized\r\n");
	return true;

fail_init:

	if (++retry_cnt <= ZINITIX_INIT_RETRY_CNT) {
		ts_power_control(touch_dev, POWER_OFF);
		touch_delay(CHIP_POWER_OFF_DELAY);
		ts_power_control(touch_dev, POWER_ON);
		touch_delay(CHIP_ON_DELAY);
		zinitix_debug_msg("retry to initiallize(retry cnt = %d)\r\n",
			retry_cnt);
		goto	retry_init;
	} else if(retry_cnt == ZINITIX_INIT_RETRY_CNT+1){
		touch_dev->cap_info.chip_fw_size = 32*1024;

		zinitix_debug_msg("retry to initiallize(retry cnt = %d)\r\n", retry_cnt);
#if TOUCH_FORCE_UPGRADE
		if( ts_upgrade_firmware(touch_dev, &m_firmware_data[2], 
			touch_dev->cap_info.chip_fw_size) == false) {
			printk(KERN_INFO "Upgrade Fail\n");
			return false;
		}
		touch_delay(100);
		// hw calibration and make checksum			
		if (ts_write_reg(touch_dev->client,
			ZINITIX_TOUCH_MODE, 0x07) != I2C_SUCCESS)
			goto fail_hwcal_init;
		touch_delay(250);
		if (ts_write_cmd(touch_dev->client, 
			ZINITIX_CALIBRATE_CMD) != I2C_SUCCESS)
			goto fail_hwcal_init;
		if (ts_write_cmd(touch_dev->client, 
			ZINITIX_SWRESET_CMD) != I2C_SUCCESS)
			goto fail_hwcal_init;
		touch_delay(10);
		/* wait for h/w calibration*/
		do {
			ts_write_cmd(touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
			touch_delay(1000);
			if (ts_read_data(touch_dev->client, 
				ZINITIX_EEPROM_INFO_REG, (u8 *)&chip_eeprom_info, 2) < 0)
				goto fail_hwcal_init;
			zinitix_debug_msg("touch eeprom info = 0x%04X\r\n", chip_eeprom_info);
			if (!zinitix_bit_test(chip_eeprom_info, 0))
				break;
		} while (1);

		if (ts_write_reg(touch_dev->client, 
			ZINITIX_TOUCH_MODE, TOUCH_MODE) != I2C_SUCCESS)
			goto fail_hwcal_init;
		touch_delay(10);
		if (ts_write_cmd(touch_dev->client, 
			ZINITIX_SWRESET_CMD) != I2C_SUCCESS)
			goto fail_hwcal_init;
		touch_delay(10);
		if (ts_write_cmd(touch_dev->client, 
			ZINITIX_SAVE_CALIBRATION_CMD) != I2C_SUCCESS)
			goto fail_hwcal_init;
		touch_delay(1000);	//for fuzing
fail_hwcal_init:		
		// to read new information
		goto retry_init;
#endif	  
	}
	printk("[zinitix touch] failed to initiallize\r\n");
	return false;
}

static void	zinitix_clear_report_data(struct zinitix_touch_dev *touch_dev)
{
	int i;
	u8 reported = 0;
	u8 sub_status;

	for (i = 0; i < touch_dev->cap_info.multi_fingers; i++) {
		sub_status = touch_dev->reported_touch_info.coord[i].sub_status;
		if (zinitix_bit_test(sub_status, SUB_BIT_EXIST)) {
#ifdef ABS_MT_TRACKING_ID
			input_report_abs(touch_dev->input_dev,
				ABS_MT_TRACKING_ID, i);
#endif
			
			input_report_abs(touch_dev->input_dev,
				ABS_MT_TOUCH_MAJOR, 0);
			input_report_abs(touch_dev->input_dev,
				ABS_MT_WIDTH_MAJOR, 0);
			input_report_abs(touch_dev->input_dev,
				ABS_MT_POSITION_X,
				touch_dev->reported_touch_info.coord[i].x);
			input_report_abs(touch_dev->input_dev,
				ABS_MT_POSITION_Y,
				touch_dev->reported_touch_info.coord[i].y);
			input_mt_sync(touch_dev->input_dev);
			reported = 1;
		}
		touch_dev->reported_touch_info.coord[i].sub_status = 0;
	}
	if (reported)
	{
		input_report_key(touch_dev->input_dev, BTN_TOUCH, 0);  /* ice cream */	
		input_sync(touch_dev->input_dev);
	}
}

#if USE_THREADED_IRQ
static irqreturn_t zinitix_touch_work(int irq, void *data)
{
	struct zinitix_touch_dev* touch_dev = (struct zinitix_touch_dev*)data;
	int i;
	u8 reported = false;
	u8 sub_status;
	u32 x, y;
#if (TOUCH_MODE == 0)
	u32 w;
	u32 tmp;
#endif

	//PSJ zinitix_debug_msg("zinitix_touch_thread : semaphore signalled\r\n");

#if	ZINITIX_ESD_TIMER_INTERVAL
	if (touch_dev->use_esd_timer) {
		ts_esd_timer_stop(touch_dev);
		zinitix_debug_msg("esd timer stop\r\n");
	}
#endif
	down(&touch_dev->work_proceedure_lock);
	if (touch_dev->work_proceedure != TS_NO_WORK) {
		zinitix_debug_msg("zinitix_touch_thread : \
			[warning] other process occupied..\r\n");
#if DELAY_FOR_SIGNAL_DELAY
		udelay(DELAY_FOR_SIGNAL_DELAY);
#endif
		if (!gpio_get_value(touch_dev->int_gpio_num)) {
			ts_write_cmd(touch_dev->client,	ZINITIX_CLEAR_INT_STATUS_CMD);
#if DELAY_FOR_SIGNAL_DELAY
			udelay(DELAY_FOR_SIGNAL_DELAY);
#endif
		}
		goto continue_read_samples;
	}
	touch_dev->work_proceedure = TS_NORMAL_WORK;
	if (ts_read_coord(touch_dev) == false) {
		zinitix_debug_msg("couldn't read touch_dev sample\r\n");
		goto continue_read_samples;
	}

#if	USE_TEST_RAW_TH_DATA_MODE
	if (touch_dev->raw_mode_flag == TOUCH_TEST_RAW_MODE)
		goto continue_read_samples;
#endif

	/* invalid : maybe periodical repeated int. */
	if (touch_dev->touch_info.status == 0x0)
		goto continue_read_samples;

	reported = false;

	if (zinitix_bit_test(touch_dev->touch_info.status, BIT_ICON_EVENT)) {
		for (i = 0; i < touch_dev->cap_info.button_num; i++) {
			if (zinitix_bit_test(touch_dev->icon_event_reg,	(BIT_O_ICON0_DOWN+i))) {
				touch_dev->button[i] = ICON_BUTTON_DOWN;
				input_report_key(touch_dev->input_dev,	BUTTON_MAPPING_KEY[i], 1);
				reported = true;
				//zinitix_debug_msg("button down = %d \r\n", i);
			}
		}

		for (i = 0; i < touch_dev->cap_info.button_num; i++) {
			if (zinitix_bit_test(touch_dev->icon_event_reg,	(BIT_O_ICON0_UP+i))) {
				touch_dev->button[i] = ICON_BUTTON_UP;
				input_report_key(touch_dev->input_dev,	BUTTON_MAPPING_KEY[i], 0);
				reported = true;
				//zinitix_debug_msg("button up = %d \r\n", i);
			}
		}
	}

	/* if button press or up event occured... */
	if (reported == true) {
#if (TOUCH_MODE == 1)
		/* input_sync(touch_dev->input_dev); */
		for (i = 0; i < touch_dev->cap_info.multi_fingers; i++) {
			sub_status = touch_dev->reported_touch_info.coord[i].sub_status;
			x = touch_dev->reported_touch_info.coord[i].x;
			y = touch_dev->reported_touch_info.coord[i].y;
			if (zinitix_bit_test(sub_status, SUB_BIT_EXIST)) {
				input_report_abs(touch_dev->input_dev,	ABS_MT_TOUCH_MAJOR, 0);
				input_report_abs(touch_dev->input_dev,	ABS_MT_WIDTH_MAJOR, 0);
				input_report_abs(touch_dev->input_dev,	ABS_MT_POSITION_X, x);
				input_report_abs(touch_dev->input_dev,	ABS_MT_POSITION_Y, y);
				input_mt_sync(touch_dev->input_dev);
			}
			touch_dev->reported_touch_info.coord[i].sub_status = 0;
		}
		input_sync(touch_dev->input_dev);
		/*goto continue_read_samples;*/
	}
#else
		for (i = 0; i < touch_dev->cap_info.multi_fingers; i++)	{
			sub_status = touch_dev->reported_touch_info.coord[i].sub_status;
			x = touch_dev->reported_touch_info.coord[i].x;
			y = touch_dev->reported_touch_info.coord[i].y;
			if (zinitix_bit_test(sub_status, SUB_BIT_EXIST)) {
				zinitix_debug_msg("finger [%02d] up \r\n", i);
				/*input_report_abs(touch_dev->input_dev, ABS_MT_TRACKING_ID,i);*/
				input_report_abs(touch_dev->input_dev,	ABS_MT_TOUCH_MAJOR, 0);
				input_report_abs(touch_dev->input_dev,	ABS_MT_WIDTH_MAJOR, 0);
				input_report_abs(touch_dev->input_dev,	ABS_MT_POSITION_X, x);
				input_report_abs(touch_dev->input_dev,	ABS_MT_POSITION_Y, y);
				input_mt_sync(touch_dev->input_dev);
			}
		}
		memset(&touch_dev->reported_touch_info,	0x0, sizeof(struct _ts_zinitix_point_info));
		input_sync(touch_dev->input_dev);
		udelay(100);
		goto continue_read_samples;
	}
	if (touch_dev->touch_info.finger_cnt > MAX_SUPPORTED_FINGER_NUM)
		touch_dev->touch_info.finger_cnt = MAX_SUPPORTED_FINGER_NUM;

	if (!zinitix_bit_test(touch_dev->touch_info.status, BIT_PT_EXIST)) {
		for (i = 0; i < touch_dev->cap_info.multi_fingers; i++) {
			sub_status = touch_dev->reported_touch_info.coord[i].sub_status;
			x = touch_dev->reported_touch_info.coord[i].x;
			y = touch_dev->reported_touch_info.coord[i].y;
			if (zinitix_bit_test(sub_status, SUB_BIT_EXIST)) {
				zinitix_debug_msg("finger [%02d] up, x = %d, y = %d \r\n", i, x, y);
				/*input_report_abs(touch_dev->input_dev, ABS_MT_TRACKING_ID,i);*/
				input_report_abs(touch_dev->input_dev,	ABS_MT_TOUCH_MAJOR, 0);
				input_report_abs(touch_dev->input_dev,	ABS_MT_WIDTH_MAJOR, 0);
				input_report_abs(touch_dev->input_dev,	ABS_MT_POSITION_X, x);
				input_report_abs(touch_dev->input_dev,	ABS_MT_POSITION_Y, y);
				input_mt_sync(touch_dev->input_dev);
			}
		}
		memset(&touch_dev->reported_touch_info,	0x0, sizeof(struct _ts_zinitix_point_info));
            input_report_key(touch_dev->input_dev, BTN_TOUCH, 0);  /* SKC */
		input_sync(touch_dev->input_dev);
		goto continue_read_samples;
	}

	for (i = 0; i < touch_dev->cap_info.multi_fingers; i++)	{
		sub_status = touch_dev->touch_info.coord[i].sub_status;

		if (zinitix_bit_test(sub_status, SUB_BIT_DOWN)|| zinitix_bit_test(sub_status, SUB_BIT_MOVE) || zinitix_bit_test(sub_status, SUB_BIT_EXIST)) {
			;
		} else if (zinitix_bit_test(sub_status, SUB_BIT_UP)) {
			x = touch_dev->reported_touch_info.coord[i].x;
			y = touch_dev->reported_touch_info.coord[i].y;
                   zinitix_debug_msg("finger [%02d] up, x = %d, y = %d \r\n", i, x, y);

			memset(&touch_dev->touch_info.coord[i],	0x0, sizeof(struct _ts_zinitix_coord));
			/*input_report_abs(touch_dev->input_dev, ABS_MT_TRACKING_ID,i);*/
			input_report_abs(touch_dev->input_dev,	ABS_MT_TOUCH_MAJOR, 0);
			input_report_abs(touch_dev->input_dev,	ABS_MT_WIDTH_MAJOR, 0);
			input_report_abs(touch_dev->input_dev,	ABS_MT_POSITION_X, x);
			input_report_abs(touch_dev->input_dev,	ABS_MT_POSITION_Y, y);
			input_mt_sync(touch_dev->input_dev);
		}

	}

	for (i = 0; i < touch_dev->cap_info.multi_fingers; i++)	{
		sub_status = touch_dev->touch_info.coord[i].sub_status;

		if (zinitix_bit_test(sub_status, SUB_BIT_DOWN)
			|| zinitix_bit_test(sub_status, SUB_BIT_MOVE)
			|| zinitix_bit_test(sub_status, SUB_BIT_EXIST)) {

			x = touch_dev->touch_info.coord[i].x;
			y = touch_dev->touch_info.coord[i].y;
			w = touch_dev->touch_info.coord[i].width;

			 /* transformation from touch to screen orientation */
			if (touch_dev->cap_info.Orientation & TOUCH_V_FLIP)
				y = touch_dev->cap_info.MaxY + touch_dev->cap_info.MinY - y;
			if (touch_dev->cap_info.Orientation & TOUCH_H_FLIP)
				x = touch_dev->cap_info.MaxX + touch_dev->cap_info.MinX - x;
			if (touch_dev->cap_info.Orientation & TOUCH_XY_SWAP)
				zinitix_swap_v(x, y, tmp);
			touch_dev->touch_info.coord[i].x = x;
			touch_dev->touch_info.coord[i].y = y;
			//PSJ zinitix_debug_msg("finger [%02d] x = %d, y = %d \r\n",	i, x, y);
			if(zinitix_bit_test(sub_status, SUB_BIT_DOWN))
			{
                        zinitix_debug_msg("finger [%02d] down, x = %d, y = %d \r\n",	i, x, y);
			}

			/*input_report_abs(touch_dev->input_dev,
				ABS_MT_TRACKING_ID,i); */
			if (w == 0)
				w = 5;
			input_report_abs(touch_dev->input_dev,	ABS_MT_TOUCH_MAJOR, (u32)w);
			input_report_abs(touch_dev->input_dev,	ABS_MT_WIDTH_MAJOR, (u32)w);
			input_report_abs(touch_dev->input_dev,	ABS_MT_POSITION_X, x);
			input_report_abs(touch_dev->input_dev,	ABS_MT_POSITION_Y, y);
                   input_report_key(touch_dev->input_dev, BTN_TOUCH, 1);  /* SKC */
			input_mt_sync(touch_dev->input_dev);
		} else if (zinitix_bit_test(sub_status, SUB_BIT_UP)) {
			;
		} else
			memset(&touch_dev->touch_info.coord[i],	0x0, sizeof(struct _ts_zinitix_coord));

	}
       input_report_key(touch_dev->input_dev, BTN_TOUCH, 1);  /* SKC */
	memcpy((char *)&touch_dev->reported_touch_info,	(char *)&touch_dev->touch_info,	sizeof(struct _ts_zinitix_point_info));
	input_sync(touch_dev->input_dev);

#endif	/* TOUCH_MODE == 1 */

continue_read_samples:

	/* check_interrupt_pin, if high, enable int & wait signal */
	if (touch_dev->work_proceedure == TS_NORMAL_WORK) {
#if	ZINITIX_ESD_TIMER_INTERVAL
		if (touch_dev->use_esd_timer) {
			ts_esd_timer_start(ZINITIX_CHECK_ESD_TIMER, touch_dev);
			zinitix_debug_msg("esd tmr start\n");
		}
#endif
		touch_dev->work_proceedure = TS_NO_WORK;
	}
	up(&touch_dev->work_proceedure_lock);
	return IRQ_HANDLED;
}
#else
static void zinitix_touch_work(struct work_struct *work)
{
	int i;
	u8 reported = false;
	u8 sub_status;
	u32 x, y;
	u32 w;
	u32 tmp;

	struct zinitix_touch_dev *touch_dev = container_of(work, struct zinitix_touch_dev, work);

	zinitix_debug_msg("zinitix_touch_thread : semaphore signalled\r\n");

	if (touch_dev->use_esd_timer) {
		ts_esd_timer_stop(touch_dev);
		zinitix_debug_msg("esd timer stop\r\n");
	}

	down(&touch_dev->work_proceedure_lock);
	if (touch_dev->work_proceedure != TS_NO_WORK) {
		zinitix_debug_msg("zinitix_touch_thread : \
			[warning] other process occupied..\r\n");
		udelay(DELAY_FOR_SIGNAL_DELAY);
		if (!gpio_get_value(touch_dev->int_gpio_num)) {
			ts_write_cmd(touch_dev->client,
				ZINITIX_CLEAR_INT_STATUS_CMD);
			udelay(DELAY_FOR_SIGNAL_DELAY);
		}
		goto continue_read_samples;
	}
	touch_dev->work_proceedure = TS_NORMAL_WORK;
	if (ts_read_coord(touch_dev) == false) {
		zinitix_debug_msg("couldn't read touch_dev sample\r\n");
		goto continue_read_samples;
	}

#if	USE_TEST_RAW_TH_DATA_MODE
	if (touch_dev->raw_mode_flag == TOUCH_TEST_RAW_MODE)
		goto continue_read_samples;
#endif

	/* invalid : maybe periodical repeated int. */
	if (touch_dev->touch_info.status == 0x0)
		goto continue_read_samples;

	reported = false;

	if (zinitix_bit_test(touch_dev->touch_info.status, BIT_ICON_EVENT)) {
		for (i = 0; i < touch_dev->cap_info.button_num; i++) {
			if (zinitix_bit_test(touch_dev->icon_event_reg,
				(BIT_O_ICON0_DOWN+i))) {
				touch_dev->button[i] = ICON_BUTTON_DOWN;
				input_report_key(touch_dev->input_dev,
					BUTTON_MAPPING_KEY[i], 1);
				reported = true;
				zinitix_debug_msg("button down = %d \r\n", i);
			}
		}

		for (i = 0; i < touch_dev->cap_info.button_num; i++) {
			if (zinitix_bit_test(touch_dev->icon_event_reg,
				(BIT_O_ICON0_UP+i))) {
				touch_dev->button[i] = ICON_BUTTON_UP;
				input_report_key(touch_dev->input_dev,
					BUTTON_MAPPING_KEY[i], 0);
				reported = true;
				zinitix_debug_msg("button up = %d \r\n", i);
			}
		}
	}

	/* if button press or up event occured... */
	if (reported == true) {
		for (i = 0; i < touch_dev->cap_info.multi_fingers; i++)	{
			sub_status =
			touch_dev->reported_touch_info.coord[i].sub_status;
			x = touch_dev->reported_touch_info.coord[i].x;
			y = touch_dev->reported_touch_info.coord[i].y;
			if (zinitix_bit_test(sub_status, SUB_BIT_EXIST)) {
				zinitix_debug_msg("finger [%02d] up \r\n", i);
#ifdef ABS_MT_TRACKING_ID
				input_report_abs(touch_dev->input_dev,
					ABS_MT_TRACKING_ID,i);
#endif
				input_report_abs(touch_dev->input_dev,
					ABS_MT_TOUCH_MAJOR, 0);
				input_report_abs(touch_dev->input_dev,
					ABS_MT_WIDTH_MAJOR, 0);
				input_report_abs(touch_dev->input_dev,
					ABS_MT_POSITION_X, x);
				input_report_abs(touch_dev->input_dev,
					ABS_MT_POSITION_Y, y);
				input_mt_sync(touch_dev->input_dev);
			}
		}
		memset(&touch_dev->reported_touch_info,
			0x0, sizeof(struct _ts_zinitix_point_info));
		input_report_key(touch_dev->input_dev, BTN_TOUCH, 0);  /* ice cream */		
		input_sync(touch_dev->input_dev);
		udelay(100);
		goto continue_read_samples;
	}
	if (touch_dev->touch_info.finger_cnt > MAX_SUPPORTED_FINGER_NUM)
		touch_dev->touch_info.finger_cnt = MAX_SUPPORTED_FINGER_NUM;

	if (!zinitix_bit_test(touch_dev->touch_info.status, BIT_PT_EXIST)) {
		for (i = 0; i < touch_dev->cap_info.multi_fingers; i++) {
			sub_status =
			touch_dev->reported_touch_info.coord[i].sub_status;
			x = touch_dev->reported_touch_info.coord[i].x;
			y = touch_dev->reported_touch_info.coord[i].y;
			if (zinitix_bit_test(sub_status, SUB_BIT_EXIST)) {
				zinitix_debug_msg("finger [%02d] up \r\n", i);
#ifdef ABS_MT_TRACKING_ID
				input_report_abs(touch_dev->input_dev,
					ABS_MT_TRACKING_ID,i);
#endif
				input_report_abs(touch_dev->input_dev,
				ABS_MT_TOUCH_MAJOR, 0);
				input_report_abs(touch_dev->input_dev,
					ABS_MT_WIDTH_MAJOR, 0);
				input_report_abs(touch_dev->input_dev,
					ABS_MT_POSITION_X, x);
				input_report_abs(touch_dev->input_dev,
					ABS_MT_POSITION_Y, y);
				input_mt_sync(touch_dev->input_dev);
			}
		}
		memset(&touch_dev->reported_touch_info,
			0x0, sizeof(struct _ts_zinitix_point_info));
		input_report_key(touch_dev->input_dev, BTN_TOUCH, 0);  /* ice cream */		
		input_sync(touch_dev->input_dev);
		goto continue_read_samples;
	}

	for (i = 0; i < touch_dev->cap_info.multi_fingers; i++)	{
		sub_status = touch_dev->touch_info.coord[i].sub_status;

		if (zinitix_bit_test(sub_status, SUB_BIT_DOWN)
			|| zinitix_bit_test(sub_status, SUB_BIT_MOVE)
			|| zinitix_bit_test(sub_status, SUB_BIT_EXIST)) {
			;
		} else if (zinitix_bit_test(sub_status, SUB_BIT_UP)) {
			zinitix_debug_msg("finger [%02d] up \r\n", i);
			x = touch_dev->reported_touch_info.coord[i].x;
			y = touch_dev->reported_touch_info.coord[i].y;

			memset(&touch_dev->touch_info.coord[i],
				0x0, sizeof(struct _ts_zinitix_coord));
#ifdef ABS_MT_TRACKING_ID			
			input_report_abs(touch_dev->input_dev,
				ABS_MT_TRACKING_ID,i);
#endif
			input_report_abs(touch_dev->input_dev,
				ABS_MT_TOUCH_MAJOR, 0);
			input_report_abs(touch_dev->input_dev,
				ABS_MT_WIDTH_MAJOR, 0);
			input_report_abs(touch_dev->input_dev,
				ABS_MT_POSITION_X, x);
			input_report_abs(touch_dev->input_dev,
				ABS_MT_POSITION_Y, y);
			input_mt_sync(touch_dev->input_dev);
		}

	}

	for (i = 0; i < touch_dev->cap_info.multi_fingers; i++)	{
		sub_status = touch_dev->touch_info.coord[i].sub_status;

		if (zinitix_bit_test(sub_status, SUB_BIT_DOWN)
			|| zinitix_bit_test(sub_status, SUB_BIT_MOVE)
			|| zinitix_bit_test(sub_status, SUB_BIT_EXIST)) {

			x = touch_dev->touch_info.coord[i].x;
			y = touch_dev->touch_info.coord[i].y;
			w = touch_dev->touch_info.coord[i].width;

			 /* transformation from touch to screen orientation */
			if (touch_dev->cap_info.Orientation & TOUCH_V_FLIP)
				y = touch_dev->cap_info.MaxY
					+ touch_dev->cap_info.MinY - y;
			if (touch_dev->cap_info.Orientation & TOUCH_H_FLIP)
				x = touch_dev->cap_info.MaxX
					+ touch_dev->cap_info.MinX - x;
			if (touch_dev->cap_info.Orientation & TOUCH_XY_SWAP)
				zinitix_swap_v(x, y, tmp);
			touch_dev->touch_info.coord[i].x = x;
			touch_dev->touch_info.coord[i].y = y;
			zinitix_debug_msg("finger [%02d] x = %d, y = %d \r\n",
				i, x, y);


			if (w == 0)
				w = 5;
#ifdef ABS_MT_TRACKING_ID
			input_report_abs(touch_dev->input_dev,
				ABS_MT_TRACKING_ID,i);
#endif
			input_report_abs(touch_dev->input_dev,
				ABS_MT_TOUCH_MAJOR, (u32)w);
			input_report_abs(touch_dev->input_dev,
				ABS_MT_WIDTH_MAJOR, (u32)w);
			input_report_abs(touch_dev->input_dev,
				ABS_MT_POSITION_X, x);
			input_report_abs(touch_dev->input_dev,
				ABS_MT_POSITION_Y, y);
			input_mt_sync(touch_dev->input_dev);
		} else if (zinitix_bit_test(sub_status, SUB_BIT_UP)) {
			;
		} else
			memset(&touch_dev->touch_info.coord[i],
				0x0, sizeof(struct _ts_zinitix_coord));

	}
	memcpy((char *)&touch_dev->reported_touch_info,
		(char *)&touch_dev->touch_info,
		sizeof(struct _ts_zinitix_point_info));
	input_report_key(touch_dev->input_dev, BTN_TOUCH, 1);  /* ice cream */	
	input_sync(touch_dev->input_dev);

continue_read_samples:

	/* check_interrupt_pin, if high, enable int & wait signal */
	if (touch_dev->work_proceedure == TS_NORMAL_WORK) {
		if (touch_dev->use_esd_timer) {
			ts_esd_timer_start(ZINITIX_CHECK_ESD_TIMER, touch_dev);
			zinitix_debug_msg("esd tmr start\n");
		}
		touch_dev->work_proceedure = TS_NO_WORK;
	}
	up(&touch_dev->work_proceedure_lock);
	//enable_irq(touch_dev->irq);
}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void zinitix_late_resume(struct early_suspend *h)
{

	struct zinitix_touch_dev *touch_dev;
	touch_dev = container_of(h, struct zinitix_touch_dev, early_suspend);
	printk(KERN_INFO "late resume++\r\n");

	if (touch_dev == NULL)
		return;

	down(&touch_dev->work_proceedure_lock);
	if (touch_dev->work_proceedure != TS_IN_RESUME
		&& touch_dev->work_proceedure != TS_IN_EALRY_SUSPEND) {
		printk(KERN_INFO"zinitix_late_resume : invalid work proceedure (%d)\r\n",
			touch_dev->work_proceedure);
		up(&touch_dev->work_proceedure_lock);
		return;
	}

      ts_power_control(touch_dev, POWER_ON);
	msleep(CHIP_ON_DELAY);
    
	ts_write_cmd(touch_dev->client, ZINITIX_WAKEUP_CMD);
	msleep(10);
	if (ts_mini_init_touch(touch_dev) == false)
		goto fail_resume;

    #if USE_THREADED_IRQ
	enable_irq(touch_dev->irq);
    #else
	enable_irq(touch_dev->irq);
    #endif
	touch_dev->work_proceedure = TS_NO_WORK;
	up(&touch_dev->work_proceedure_lock);
	printk(KERN_INFO "late resume--\n");
	return;
fail_resume:
	printk(KERN_ERR "failed to resume\n");
    #if USE_THREADED_IRQ
	enable_irq(touch_dev->irq);
    #else
	enable_irq(touch_dev->irq);
    #endif
	touch_dev->work_proceedure = TS_NO_WORK;
	up(&touch_dev->work_proceedure_lock);
	return;
}


static void zinitix_early_suspend(struct early_suspend *h)
{
	struct zinitix_touch_dev *touch_dev =
		container_of(h, struct zinitix_touch_dev, early_suspend);

	if (touch_dev == NULL)
		return;

	flush_work(&touch_dev->tmr_work);
	flush_work(&touch_dev->work);

	down(&touch_dev->work_proceedure_lock);
	if (touch_dev->work_proceedure != TS_NO_WORK) {
		printk(KERN_INFO"zinitix_early_suspend : invalid work proceedure (%d)\r\n",
			touch_dev->work_proceedure);
		up(&touch_dev->work_proceedure_lock);
		return;
	}
	touch_dev->work_proceedure = TS_IN_EALRY_SUSPEND;

	printk(KERN_INFO "early suspend++\n");
	zinitix_debug_msg("clear all reported points\r\n");
	zinitix_clear_report_data(touch_dev);

	if (touch_dev->use_esd_timer) {
		ts_write_reg(touch_dev->client,
			ZINITIX_PERIODICAL_INTERRUPT_INTERVAL, 0);
		ts_esd_timer_stop(touch_dev);
		printk(KERN_INFO " ts_esd_timer_stop\n");
	}

      #if USE_THREADED_IRQ
	disable_irq(touch_dev->irq);
      #else
	disable_irq(touch_dev->irq);
      #endif
#if !(USING_CHIP_SETTING)
	ts_write_reg(touch_dev->client, ZINITIX_INT_ENABLE_FLAG, 0x0);
#endif
	udelay(100);
	ts_write_cmd(touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
	if (ts_write_cmd(touch_dev->client, ZINITIX_SLEEP_CMD) != I2C_SUCCESS) {
		printk(KERN_ERR "failed to enter into sleep mode\n");
		up(&touch_dev->work_proceedure_lock);
		return;
	}

      ts_power_control(touch_dev, POWER_OFF);
	msleep(CHIP_POWER_OFF_DELAY);
       
	printk(KERN_INFO "early suspend--\n");
	up(&touch_dev->work_proceedure_lock);
	return;
}

#endif	/* CONFIG_HAS_EARLYSUSPEND */


#if 0
static int zinitix_resume(struct i2c_client *client)
{
	struct zinitix_touch_dev *touch_dev = i2c_get_clientdata(client);

	zinitix_debug_msg("resume++\n");
	down(&touch_dev->work_proceedure_lock);
	if (touch_dev->work_proceedure != TS_IN_SUSPEND) {
		printk(KERN_INFO"zinitix_resume : invalid work proceedure (%d)\r\n",
			touch_dev->work_proceedure);
		up(&touch_dev->work_proceedure_lock);
		return 0;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	touch_dev->work_proceedure = TS_IN_RESUME;
#else
	touch_dev->work_proceedure = TS_NO_WORK;
#endif

	ts_power_control(touch_dev, POWER_ON);
	touch_delay(CHIP_ON_DELAY);

	zinitix_debug_msg("resume--\n");
	up(&touch_dev->work_proceedure_lock);
	return 0;

}

static int zinitix_suspend(struct i2c_client *client, pm_message_t message)
{
	struct zinitix_touch_dev *touch_dev = i2c_get_clientdata(client);

	zinitix_debug_msg("suspend++\n");
	down(&touch_dev->work_proceedure_lock);
	if (touch_dev->work_proceedure != TS_NO_WORK
		&& touch_dev->work_proceedure != TS_IN_EALRY_SUSPEND) {
		printk(KERN_INFO"zinitix_suspend : invalid work proceedure (%d)\r\n",
			touch_dev->work_proceedure);
		up(&touch_dev->work_proceedure_lock);
		return 0;
	}

	ts_power_control(touch_dev, POWER_OFF);
	touch_delay(CHIP_POWER_OFF_DELAY);

	touch_dev->work_proceedure = TS_IN_SUSPEND;
	zinitix_debug_msg("suspend--\n");
	up(&touch_dev->work_proceedure_lock);
	return 0;
}
#endif

#if	USE_TEST_RAW_TH_DATA_MODE
static ssize_t zinitix_get_test_raw_data(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int total_n_num;
	if (misc_touch_dev == NULL) {
		zinitix_debug_msg("device NULL : NULL\n");
		return 0;
	}
	down(&misc_touch_dev->raw_data_lock);
	total_n_num = misc_touch_dev->cap_info.total_node_num;
	if (zinitix_bit_test(
		misc_touch_dev->cur_data[total_n_num], BIT_PT_EXIST)) {
		/*x_lsb*/
		buf[20] =
			(misc_touch_dev->cur_data[total_n_num+1]&0xff);
		/*x_msb*/
		buf[21] =
			((misc_touch_dev->cur_data[total_n_num+1]>>8)&0xff);
		/*y_lsb*/
		buf[22] =
			(misc_touch_dev->cur_data[total_n_num+2]&0xff);
		/*y_msb*/
		buf[23] =
			((misc_touch_dev->cur_data[total_n_num+2]
			>>8)&0xff);
	} else {
		buf[20] = 0; /*x_lsb*/
		buf[21] = 0; /*x_msb*/
		buf[22] = 0; /*y_lsb*/
		buf[23] = 0; /*y_msb*/
	}

	/*lsb*/
	buf[0] =
		(char)(misc_touch_dev->ref_data[22]&0xff);
	/*msb*/
	buf[1] =
		(char)((misc_touch_dev->ref_data[22]>>8)&0xff);
	/*delta lsb*/
	buf[2] =
		(char)(((s16)(misc_touch_dev->cur_data[22]
		- misc_touch_dev->ref_data[22]))&0xff);
	/*delta msb*/
	buf[3] =
		(char)((((s16)(misc_touch_dev->cur_data[22]
		- misc_touch_dev->ref_data[22]))>>8)&0xff);
	/*
	buf[4] =
		(char)(misc_touch_dev->ref_data[51]&0xff);
	buf[5] =
		(char)((misc_touch_dev->ref_data[51]>>8)&0xff);
	buf[6] =
		(char)(((s16)(misc_touch_dev->cur_data[51]
		-misc_touch_dev->ref_data[51]))&0xff);
	buf[7] =
		(char)((((s16)(misc_touch_dev->cur_data[51]
		-misc_touch_dev->ref_data[51]))>>8)&0xff);

	buf[8] =
		(char)(misc_touch_dev->ref_data[102]&0xff);
	buf[9] =
		(char)((misc_touch_dev->ref_data[102]>>8)&0xff);
	buf[10] =
		(char)(((s16)(misc_touch_dev->cur_data[102]
		-misc_touch_dev->ref_data[102]))&0xff);
	buf[11] =
		(char)((((s16)(misc_touch_dev->cur_data[102]
		-misc_touch_dev->ref_data[102]))>>8)&0xff);

	buf[12] =
		(char)(misc_touch_dev->ref_data[169]&0xff);
	buf[13] =
		(char)((misc_touch_dev->ref_data[169]>>8)&0xff);
	buf[14] =
		(char)(((s16)(misc_touch_dev->cur_data[169]
		-misc_touch_dev->ref_data[169]))&0xff);
	buf[15] =
		(char)((((s16)(misc_touch_dev->cur_data[169]
		-misc_touch_dev->ref_data[169]))>>8)&0xff);

	buf[16] =
		(char)(misc_touch_dev->ref_data[178]&0xff);
	buf[17] =
		(char)((misc_touch_dev->ref_data[178]>>8)&0xff);
	buf[18] =
		(char)(((s16)(misc_touch_dev->cur_data[178]
		-misc_touch_dev->ref_data[178]))&0xff);
	buf[19] =
		(char)((((s16)(misc_touch_dev->cur_data[178]
		- misc_touch_dev->ref_data[178]))>>8)&0xff);
	*/
	up(&misc_touch_dev->raw_data_lock);


	return 24;
}


ssize_t zinitix_set_testmode(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned char value = 0;

	printk(KERN_INFO "[zinitix_touch] zinitix_set_testmode, buf = %d\r\n",
		*buf);

	if (misc_touch_dev == NULL) {
		zinitix_debug_msg("device NULL : NULL\n");
		return 0;
	}

	sscanf(buf, "%c", &value);

	if (value != TOUCH_TEST_RAW_MODE && value != TOUCH_NORMAL_MODE) {
		printk(KERN_WARNING "[zinitix ts] test mode setting value error. you must set %d[=normal] or %d[=raw mode]\r\n",
			TOUCH_NORMAL_MODE, TOUCH_TEST_RAW_MODE);
		return 1;
	}

	down(&misc_touch_dev->raw_data_lock);
	misc_touch_dev->raw_mode_flag = value;

	printk(KERN_INFO "[zinitix_touch] zinitix_set_testmode, touchkey_testmode = %d\r\n",
		misc_touch_dev->raw_mode_flag);

	if (misc_touch_dev->raw_mode_flag == TOUCH_NORMAL_MODE) {
		/* enter into normal mode */
		printk(KERN_INFO "[zinitix_touch] TEST Mode Exit\r\n");

		if (ts_write_reg(misc_touch_dev->client,
			ZINITIX_PERIODICAL_INTERRUPT_INTERVAL,
			ZINITIX_SCAN_RATE_HZ
			*ZINITIX_ESD_TIMER_INTERVAL) != I2C_SUCCESS)
			printk(KERN_INFO "[zinitix_touch] Fail to set ZINITIX_PERIODICAL_INTERRUPT_INTERVAL.\r\n");

		if (ts_write_reg(misc_touch_dev->client,
			ZINITIX_TOUCH_MODE, TOUCH_MODE) != I2C_SUCCESS)
			printk(KERN_INFO "[zinitix_touch] Fail to set touch mode %d.\r\n",
				TOUCH_MODE);

		/* clear garbage data */
		ts_write_cmd(misc_touch_dev->client,
			ZINITIX_CLEAR_INT_STATUS_CMD);
		touch_delay(100);
		ts_write_cmd(misc_touch_dev->client,
			ZINITIX_CLEAR_INT_STATUS_CMD);
	} else {
		/* enter into test mode */
		printk(KERN_INFO "[zinitix_touch] TEST Mode Enter\r\n");

		if (ts_write_reg(misc_touch_dev->client,
			ZINITIX_PERIODICAL_INTERRUPT_INTERVAL,
			ZINITIX_SCAN_RATE_HZ
			*ZINITIX_RAW_DATA_ESD_TIMER_INTERVAL) != I2C_SUCCESS)
			printk(KERN_INFO "[zinitix_touch] Fail to set ZINITIX_RAW_DATA_ESD_TIMER_INTERVAL.\r\n");

		if (ts_write_reg(misc_touch_dev->client,
			ZINITIX_TOUCH_MODE, TOUCH_TEST_RAW_MODE) != I2C_SUCCESS)
			printk(KERN_INFO "[zinitix_touch] Fail to set touch mode %d.\r\n",
				TOUCH_TEST_RAW_MODE);
		ts_write_cmd(misc_touch_dev->client,
			ZINITIX_CLEAR_INT_STATUS_CMD);
		/* clear garbage data */
		touch_delay(100);
		ts_write_cmd(misc_touch_dev->client,
			ZINITIX_CLEAR_INT_STATUS_CMD);
		memset(&misc_touch_dev->reported_touch_info,
			0x0, sizeof(struct _ts_zinitix_point_info));
		memset(&misc_touch_dev->touch_info,
			0x0, sizeof(struct _ts_zinitix_point_info));
	}
	up(&misc_touch_dev->raw_data_lock);
	return 1;

}

static DEVICE_ATTR(get_touch_raw_data,
	S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH,
	zinitix_get_test_raw_data,
	zinitix_set_testmode);


static int ts_misc_fops_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int ts_misc_fops_close(struct inode *inode, struct file *filp)
{
	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
static int ts_misc_fops_ioctl(struct inode *inode,
	struct file *filp, unsigned int cmd,
	unsigned long arg)
#else
static long ts_misc_fops_ioctl(struct file *filp,
	unsigned int cmd, unsigned long arg)
#endif
{
	void __user *argp = (void __user *)arg;
	struct _raw_ioctl raw_ioctl;
	u8 *u8Data;
	int i, j;
	int ret = 0;
	size_t sz = 0;
	u16 version;
	u8 div_node;
	int total_cal_n;

	u16 mode;
	u16 chip_eeprom_info;
	struct _reg_ioctl reg_ioctl;
	u16 val;
	int nval = 0;

	if (misc_touch_dev == NULL)
		return -1;

	/* zinitix_debug_msg("cmd = %d, argp = 0x%x\n", cmd, (int)argp); */

	switch (cmd) {

	case TOUCH_IOCTL_GET_DEBUGMSG_STATE:
		ret = m_ts_debug_mode;
		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
		break;

	case TOUCH_IOCTL_SET_DEBUGMSG_STATE:
		if (copy_from_user(&nval, argp, 4)) {
			printk(KERN_INFO "[zinitix_touch] error : copy_from_user\n");
			return -1;
		}
		if (nval)
			printk(KERN_INFO "[zinitix_touch] on debug mode (%d)\n",
				nval);
		else
			printk(KERN_INFO "[zinitix_touch] off debug mode (%d)\n",
				nval);
		m_ts_debug_mode = nval;
		break;

	case TOUCH_IOCTL_GET_CHIP_REVISION:
		ret = misc_touch_dev->cap_info.chip_revision;
		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
		break;

	case TOUCH_IOCTL_GET_FW_VERSION:
		ret = misc_touch_dev->cap_info.chip_firmware_version;
		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
		break;

	case TOUCH_IOCTL_GET_REG_DATA_VERSION:
		ret = misc_touch_dev->cap_info.chip_reg_data_version;
		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
		break;

	case TOUCH_IOCTL_VARIFY_UPGRADE_SIZE:
		if (copy_from_user(&sz, argp, sizeof(size_t)))
			return -1;

		printk(KERN_INFO "firmware size = %d\r\n", sz);
		if (misc_touch_dev->cap_info.chip_fw_size != sz) {
			printk(KERN_INFO "firmware size error\r\n");
			return -1;
		}
		break;

	case TOUCH_IOCTL_VARIFY_UPGRADE_DATA:
		if (copy_from_user(&m_firmware_data[2],
			argp, misc_touch_dev->cap_info.chip_fw_size))
			return -1;

		version =
			(u16)(((u16)m_firmware_data[FIRMWARE_VERSION_POS+1]<<8)
			|(u16)m_firmware_data[FIRMWARE_VERSION_POS]);

		printk(KERN_INFO "firmware version = %x\r\n", version);

		if (copy_to_user(argp, &version, sizeof(version)))
			return -1;
		break;

	case TOUCH_IOCTL_START_UPGRADE:
		disable_irq(misc_touch_dev->irq);
		down(&misc_touch_dev->work_proceedure_lock);
		misc_touch_dev->work_proceedure = TS_IN_UPGRADE;

		if (misc_touch_dev->use_esd_timer)
			ts_esd_timer_stop(misc_touch_dev);
		zinitix_debug_msg("clear all reported points\r\n");
		zinitix_clear_report_data(misc_touch_dev);

		printk(KERN_INFO "start upgrade firmware\n");
		if (ts_upgrade_firmware(misc_touch_dev,
			&m_firmware_data[2],
			misc_touch_dev->cap_info.chip_fw_size) == false) {
			enable_irq(misc_touch_dev->irq);
			misc_touch_dev->work_proceedure = TS_NO_WORK;
			up(&misc_touch_dev->work_proceedure_lock);
			return -1;
		}

		if (ts_init_touch(misc_touch_dev) == false) {
			enable_irq(misc_touch_dev->irq);
			misc_touch_dev->work_proceedure = TS_NO_WORK;
			up(&misc_touch_dev->work_proceedure_lock);
			return -1;
		}


		if (misc_touch_dev->use_esd_timer) {
			ts_esd_timer_start(ZINITIX_CHECK_ESD_TIMER,
				misc_touch_dev);
			zinitix_debug_msg("esd timer start\r\n");
		}

		enable_irq(misc_touch_dev->irq);
		misc_touch_dev->work_proceedure = TS_NO_WORK;
		up(&misc_touch_dev->work_proceedure_lock);
		break;

	case TOUCH_IOCTL_GET_X_RESOLUTION:
		ret = misc_touch_dev->cap_info.x_resolution;
		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
		break;

	case TOUCH_IOCTL_GET_Y_RESOLUTION:
		ret = misc_touch_dev->cap_info.y_resolution;
		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
		break;

	case TOUCH_IOCTL_GET_X_NODE_NUM:
		ret = misc_touch_dev->cap_info.x_node_num;
		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
		break;

	case TOUCH_IOCTL_GET_Y_NODE_NUM:
		ret = misc_touch_dev->cap_info.y_node_num;
		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
		break;

	case TOUCH_IOCTL_GET_TOTAL_NODE_NUM:
		ret = misc_touch_dev->cap_info.total_node_num;
		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
		break;

	case TOUCH_IOCTL_HW_CALIBRAION:
		ret = -1;
		disable_irq(misc_touch_dev->irq);
		down(&misc_touch_dev->work_proceedure_lock);
		if (misc_touch_dev->work_proceedure != TS_NO_WORK) {
			printk(KERN_INFO"other process occupied.. (%d)\r\n",
				misc_touch_dev->work_proceedure);
			up(&misc_touch_dev->work_proceedure_lock);
			return -1;
		}
		misc_touch_dev->work_proceedure = TS_HW_CALIBRAION;

		touch_delay(100);
		/* h/w calibration */
		if (ts_write_reg(misc_touch_dev->client,
			ZINITIX_TOUCH_MODE, 0x07) != I2C_SUCCESS)
			goto fail_hw_cal;
		touch_delay(250);
		if (ts_write_cmd(misc_touch_dev->client,
			ZINITIX_CALIBRATE_CMD) != I2C_SUCCESS)
			goto fail_hw_cal;
		if (ts_write_cmd(misc_touch_dev->client,
			ZINITIX_SWRESET_CMD) != I2C_SUCCESS)
			goto fail_hw_cal;
		touch_delay(10);
		ts_write_cmd(misc_touch_dev->client,
			ZINITIX_CLEAR_INT_STATUS_CMD);
		/* wait for h/w calibration */
		do {
			touch_delay(1000);
			if (ts_read_data(misc_touch_dev->client,
				ZINITIX_EEPROM_INFO_REG,
				(u8 *)&chip_eeprom_info, 2) < 0)
				goto fail_hw_cal;
			zinitix_debug_msg("touch eeprom info = 0x%04X\r\n",
				chip_eeprom_info);
			if (!zinitix_bit_test(chip_eeprom_info, 0))
				break;
		} while (1);


		touch_delay(10);
		if (ts_write_cmd(misc_touch_dev->client,
			ZINITIX_SAVE_CALIBRATION_CMD) != I2C_SUCCESS)
			goto fail_hw_cal;
		touch_delay(500);

		ret = 0;
fail_hw_cal:
		if (misc_touch_dev->raw_mode_flag == TOUCH_NORMAL_MODE)
			mode = TOUCH_MODE;
		else
			mode = misc_touch_dev->raw_mode_flag;
		if (ts_write_reg(misc_touch_dev->client,
			ZINITIX_TOUCH_MODE, mode) != I2C_SUCCESS) {
			printk(KERN_INFO "fail to set touch mode %d.\n",
				mode);
			goto fail_hw_cal2;
		}

		if (ts_write_cmd(misc_touch_dev->client,
			ZINITIX_SWRESET_CMD) != I2C_SUCCESS)
			goto fail_hw_cal2;

		enable_irq(misc_touch_dev->irq);
		misc_touch_dev->work_proceedure = TS_NO_WORK;
		up(&misc_touch_dev->work_proceedure_lock);
		return ret;
fail_hw_cal2:
		enable_irq(misc_touch_dev->irq);
		misc_touch_dev->work_proceedure = TS_NO_WORK;
		up(&misc_touch_dev->work_proceedure_lock);
		return -1;

	case TOUCH_IOCTL_SET_RAW_DATA_MODE:
		if (misc_touch_dev == NULL) {
			zinitix_debug_msg("misc device NULL?\n");
			return -1;
		}

		down(&misc_touch_dev->work_proceedure_lock);
		if (misc_touch_dev->work_proceedure != TS_NO_WORK) {
			printk(KERN_INFO"other process occupied.. (%d)\r\n",
				misc_touch_dev->work_proceedure);
			up(&misc_touch_dev->work_proceedure_lock);
			return -1;
		}
		misc_touch_dev->work_proceedure = TS_SET_MODE;

		if (copy_from_user(&nval, argp, 4)) {
			printk(KERN_INFO "[zinitix_touch] error : copy_from_user\r\n");
			misc_touch_dev->work_proceedure = TS_NO_WORK;
			return -1;
		}

		zinitix_debug_msg("[zinitix_touch] touchkey_testmode = %d\r\n",
			misc_touch_dev->raw_mode_flag);

		if (nval == TOUCH_NORMAL_MODE &&
			misc_touch_dev->raw_mode_flag != TOUCH_NORMAL_MODE) {
			/* enter into normal mode */
			misc_touch_dev->raw_mode_flag = nval;
			printk(KERN_INFO "[zinitix_touch] raw data mode exit\r\n");

			if (ts_write_reg(misc_touch_dev->client,
				ZINITIX_PERIODICAL_INTERRUPT_INTERVAL,
				ZINITIX_SCAN_RATE_HZ*ZINITIX_ESD_TIMER_INTERVAL)
				!= I2C_SUCCESS)
				printk(KERN_INFO "[zinitix_touch] Fail to set ZINITIX_PERIODICAL_INTERRUPT_INTERVAL.\n");

			if (ts_write_reg(misc_touch_dev->client,
				ZINITIX_TOUCH_MODE, TOUCH_MODE) != I2C_SUCCESS)
				printk(KERN_INFO "[zinitix_touch] fail to set TOUCH_MODE.\r\n");

			/* clear garbage data */
			ts_write_cmd(misc_touch_dev->client,
				ZINITIX_CLEAR_INT_STATUS_CMD);
			touch_delay(100);
			ts_write_cmd(misc_touch_dev->client,
				ZINITIX_CLEAR_INT_STATUS_CMD);
		} else if (nval != TOUCH_NORMAL_MODE) {
			/* enter into test mode*/
			misc_touch_dev->raw_mode_flag = nval;
			printk(KERN_INFO "[zinitix_touch] raw data mode enter\r\n");

			if (ts_write_reg(misc_touch_dev->client,
				ZINITIX_PERIODICAL_INTERRUPT_INTERVAL,
				ZINITIX_SCAN_RATE_HZ
				*ZINITIX_RAW_DATA_ESD_TIMER_INTERVAL)
				!= I2C_SUCCESS)
				printk(KERN_INFO "[zinitix_touch] Fail to set ZINITIX_RAW_DATA_ESD_TIMER_INTERVAL.\n");

			if (ts_write_reg(misc_touch_dev->client,
				ZINITIX_TOUCH_MODE,
				misc_touch_dev->raw_mode_flag) != I2C_SUCCESS)
				printk(KERN_INFO "[zinitix_touch] raw data mode : Fail to set TOUCH_MODE %d.\n",
					misc_touch_dev->raw_mode_flag);

			ts_write_cmd(misc_touch_dev->client,
				ZINITIX_CLEAR_INT_STATUS_CMD);
			/* clear garbage data */
			touch_delay(100);
			ts_write_cmd(misc_touch_dev->client,
				ZINITIX_CLEAR_INT_STATUS_CMD);
		}

		misc_touch_dev->work_proceedure = TS_NO_WORK;
		up(&misc_touch_dev->work_proceedure_lock);
		return 0;

	case TOUCH_IOCTL_GET_REG:
		if (misc_touch_dev == NULL) {
			zinitix_debug_msg("misc device NULL?\n");
			return -1;
		}
		down(&misc_touch_dev->work_proceedure_lock);
		if (misc_touch_dev->work_proceedure != TS_NO_WORK) {
			printk(KERN_INFO "other process occupied.. (%d)\n",
				misc_touch_dev->work_proceedure);
			up(&misc_touch_dev->work_proceedure_lock);
			return -1;
		}

		misc_touch_dev->work_proceedure = TS_SET_MODE;

		if (copy_from_user(&reg_ioctl,
			argp, sizeof(struct _reg_ioctl))) {
			misc_touch_dev->work_proceedure = TS_NO_WORK;
			up(&misc_touch_dev->work_proceedure_lock);
			printk(KERN_INFO "[zinitix_touch] error : copy_from_user\n");
			return -1;
		}

		if (ts_read_data(misc_touch_dev->client,
			reg_ioctl.addr, (u8 *)&val, 2) < 0)
			ret = -1;

		nval = (int)val;

		if (copy_to_user(reg_ioctl.val, (u8 *)&nval, 4)) {
			misc_touch_dev->work_proceedure = TS_NO_WORK;
			up(&misc_touch_dev->work_proceedure_lock);
			printk(KERN_INFO "[zinitix_touch] error : copy_to_user\n");
			return -1;
		}

		zinitix_debug_msg("read : reg addr = 0x%x, val = 0x%x\n",
			reg_ioctl.addr, nval);

		misc_touch_dev->work_proceedure = TS_NO_WORK;
		up(&misc_touch_dev->work_proceedure_lock);
		return ret;

	case TOUCH_IOCTL_SET_REG:

		if (misc_touch_dev == NULL) {
			zinitix_debug_msg("misc device NULL?\n");
			return -1;
		}
		down(&misc_touch_dev->work_proceedure_lock);
		if (misc_touch_dev->work_proceedure != TS_NO_WORK) {
			printk(KERN_INFO "other process occupied.. (%d)\n",
				misc_touch_dev->work_proceedure);
			up(&misc_touch_dev->work_proceedure_lock);
			return -1;
		}

		misc_touch_dev->work_proceedure = TS_SET_MODE;
		if (copy_from_user(&reg_ioctl,
				argp, sizeof(struct _reg_ioctl))) {
			misc_touch_dev->work_proceedure = TS_NO_WORK;
			up(&misc_touch_dev->work_proceedure_lock);
			printk(KERN_INFO "[zinitix_touch] error : copy_from_user\n");
			return -1;
		}

		if (copy_from_user(&val, reg_ioctl.val, 4)) {
			misc_touch_dev->work_proceedure = TS_NO_WORK;
			up(&misc_touch_dev->work_proceedure_lock);
			printk(KERN_INFO "[zinitix_touch] error : copy_from_user\n");
			return -1;
		}

		if (ts_write_reg(misc_touch_dev->client,
			reg_ioctl.addr, val) != I2C_SUCCESS)
			ret = -1;

		zinitix_debug_msg("write : reg addr = 0x%x, val = 0x%x\r\n",
			reg_ioctl.addr, val);
		misc_touch_dev->work_proceedure = TS_NO_WORK;
		up(&misc_touch_dev->work_proceedure_lock);
		return ret;

	case TOUCH_IOCTL_DONOT_TOUCH_EVENT:

		if (misc_touch_dev == NULL) {
			zinitix_debug_msg("misc device NULL?\n");
			return -1;
		}
		down(&misc_touch_dev->work_proceedure_lock);
		if (misc_touch_dev->work_proceedure != TS_NO_WORK) {
			printk(KERN_INFO"other process occupied.. (%d)\r\n",
				misc_touch_dev->work_proceedure);
			up(&misc_touch_dev->work_proceedure_lock);
			return -1;
		}

		misc_touch_dev->work_proceedure = TS_SET_MODE;
		if (ts_write_reg(misc_touch_dev->client,
			ZINITIX_INT_ENABLE_FLAG, 0) != I2C_SUCCESS)
			ret = -1;
		zinitix_debug_msg("write : reg addr = 0x%x, val = 0x0\r\n",
			ZINITIX_INT_ENABLE_FLAG);

		misc_touch_dev->work_proceedure = TS_NO_WORK;
		up(&misc_touch_dev->work_proceedure_lock);
		return ret;

	case TOUCH_IOCTL_SEND_SAVE_STATUS:
		if (misc_touch_dev == NULL) {
			zinitix_debug_msg("misc device NULL?\n");
			return -1;
		}
		down(&misc_touch_dev->work_proceedure_lock);
		if (misc_touch_dev->work_proceedure != TS_NO_WORK) {
			printk(KERN_INFO"other process occupied.. (%d)\r\n",
				misc_touch_dev->work_proceedure);
			up(&misc_touch_dev->work_proceedure_lock);
			return -1;
		}
		misc_touch_dev->work_proceedure = TS_SET_MODE;
		ret = 0;
		if (ts_write_cmd(misc_touch_dev->client,
			ZINITIX_SAVE_STATUS_CMD) != I2C_SUCCESS)
			ret = -1;
		touch_delay(1000);	/* for fusing eeprom */

		misc_touch_dev->work_proceedure = TS_NO_WORK;
		up(&misc_touch_dev->work_proceedure_lock);
		return ret;

	case TOUCH_IOCTL_GET_RAW_DATA:
		if (misc_touch_dev == NULL) {
			zinitix_debug_msg("misc device NULL?\n");
			return -1;
		}

		if (misc_touch_dev->raw_mode_flag == TOUCH_NORMAL_MODE)
			return -1;

		down(&misc_touch_dev->raw_data_lock);
		if (misc_touch_dev->update == 0) {
			up(&misc_touch_dev->raw_data_lock);
			return -2;
		}

		if (copy_from_user(&raw_ioctl,
			argp, sizeof(raw_ioctl))) {
			up(&misc_touch_dev->raw_data_lock);
			printk(KERN_INFO "[zinitix_touch] error : copy_from_user\r\n");
			return -1;
		}

		misc_touch_dev->update = 0;

		u8Data = (u8 *)&misc_touch_dev->cur_data[0];
		if (misc_touch_dev->raw_mode_flag == TOUCH_ZINITIX_CAL_N_MODE) {


			j = 0;
			total_cal_n =
				misc_touch_dev->cap_info.total_cal_n;
			if (total_cal_n == 0)
				total_cal_n = 160;
			div_node =
				(u8)misc_touch_dev->cap_info.max_y_node;
			if (div_node == 0)
				div_node = 16;
			u8Data = (u8 *)&misc_touch_dev->cur_data[0];
			for (i = 0; i < total_cal_n*2; i++) {
				if (i%div_node <
					misc_touch_dev->cap_info.y_node_num) {
					misc_touch_dev->ref_data[j] =
						(u16)u8Data[i];
					j++;
				}
			}

			u8Data = (u8 *)&misc_touch_dev->ref_data[0];
		}

		if (copy_to_user(raw_ioctl.buf, (u8 *)u8Data,
			raw_ioctl.sz)) {
			up(&misc_touch_dev->raw_data_lock);
			return -1;
		}

		up(&misc_touch_dev->raw_data_lock);
		return 0;

	default:
		break;
	}
	return 0;
}

#endif /* USE_TEST_RAW_TH_DATA_MODE */


static int zinitix_touch_probe(struct i2c_client *client,
		const struct i2c_device_id *i2c_id)
{
	int ret = 0;
	struct zinitix_touch_dev *touch_dev = NULL;
	int i;

	zinitix_debug_msg("zinitix_touch_probe+\r\n");


	zinitix_debug_msg("Above BT4x3 Driver\r\n");

	printk(KERN_INFO "[zinitix touch] driver version = %s\r\n",
		TS_DRVIER_VERSION);

	
#ifdef ABS_MT_TRACKING_ID
	printk(KERN_INFO "[zinitix touch] use tracking id\r\n");
#endif

	if (strcmp(client->name, ZINITIX_ISP_NAME) == 0) {
		printk(KERN_INFO "isp client probe \r\n");
		m_isp_client = client;
		return 0;
	}


	if(touch_regulator == NULL)
	{
		zinitix_debug_msg(" %s, %d \n", __func__, __LINE__ );
		touch_regulator = regulator_get(NULL, "hv10"); 
		zinitix_debug_msg(" %s, %d \n", __func__, __LINE__ );			
		if(IS_ERR(touch_regulator)){
			zinitix_debug_msg("can not get VTOUCH_3.3V\n");
		}	
		}

	ts_power_control(touch_dev, POWER_ON);

	zinitix_debug_msg("i2c check function \r\n");
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_ERR "error : not compatible i2c function \r\n");
		ret = -ENODEV;
		goto err_check_functionality;
	}

	zinitix_debug_msg("touch data alloc \r\n");
	touch_dev = kzalloc(sizeof(struct zinitix_touch_dev), GFP_KERNEL);
	if (!touch_dev) {
		printk(KERN_ERR "unabled to allocate touch data \r\n");
		ret = -ENOMEM;
		goto err_alloc_dev_data;
	}
	touch_dev->client = client;
	i2c_set_clientdata(client, touch_dev);

      g_touch_dev = touch_dev;


      #if USE_THREADED_IRQ
      #else
	INIT_WORK(&touch_dev->work, zinitix_touch_work);
      #endif

      #if USE_THREADED_IRQ
      #else
	zinitix_debug_msg("touch workqueue create \r\n");
	zinitix_workqueue = create_singlethread_workqueue("zinitix_workqueue");
	if (!zinitix_workqueue) {
		printk(KERN_ERR "unabled to create touch thread \r\n");
		ret = -1;
		goto err_kthread_create_failed;
	}
      #endif


	zinitix_debug_msg("allocate input device \r\n");
	touch_dev->input_dev = input_allocate_device();
	if (touch_dev->input_dev == 0) {
		printk(KERN_ERR "unabled to allocate input device \r\n");
		ret = -ENOMEM;
		goto err_input_allocate_device;
	}

	/* initialize zinitix touch ic */
	touch_dev->int_gpio_num = GPIO_TOUCH_PIN_NUM;	/*	for upgrade */

	memset(&touch_dev->reported_touch_info,
		0x0, sizeof(struct _ts_zinitix_point_info));

#if	USE_TEST_RAW_TH_DATA_MODE
	/*	not test mode */
	touch_dev->raw_mode_flag = TOUCH_NORMAL_MODE;
#endif
	if(ts_init_touch(touch_dev) == false) {
		goto err_input_register_device;
	}
	touch_dev->use_esd_timer = 0;

	INIT_WORK(&touch_dev->tmr_work, zinitix_touch_tmr_work);
	zinitix_tmr_workqueue =
		create_singlethread_workqueue("zinitix_tmr_workqueue");
	if (!zinitix_tmr_workqueue) {
		printk(KERN_ERR "unabled to create touch tmr work queue \r\n");
		ret = -1;
		goto err_kthread_create_failed;
	}
	
	if (ZINITIX_ESD_TIMER_INTERVAL) {
		touch_dev->use_esd_timer = 1;
		ts_esd_timer_init(touch_dev);
		ts_esd_timer_start(ZINITIX_CHECK_ESD_TIMER, touch_dev);
		printk(KERN_INFO " ts_esd_timer_start\n");
	}


	sprintf(touch_dev->phys, "input(ts)");
	touch_dev->input_dev->name = ZINITIX_DRIVER_NAME;
	touch_dev->input_dev->id.bustype = BUS_I2C;
	touch_dev->input_dev->id.vendor = 0x0001;
	touch_dev->input_dev->phys = touch_dev->phys;
	touch_dev->input_dev->id.product = 0x0002;
	touch_dev->input_dev->id.version = 0x0100;

	set_bit(EV_SYN, touch_dev->input_dev->evbit);
	set_bit(EV_KEY, touch_dev->input_dev->evbit);
	set_bit(BTN_TOUCH, touch_dev->input_dev->keybit);
	set_bit(EV_ABS, touch_dev->input_dev->evbit);
	set_bit(INPUT_PROP_DIRECT, touch_dev->input_dev->propbit);

	for (i = 0; i < MAX_SUPPORTED_BUTTON_NUM; i++)
		set_bit(BUTTON_MAPPING_KEY[i], touch_dev->input_dev->keybit);

	if (touch_dev->cap_info.Orientation & TOUCH_XY_SWAP) {
		input_set_abs_params(touch_dev->input_dev, ABS_MT_POSITION_Y,
			touch_dev->cap_info.MinX,
			touch_dev->cap_info.MaxX + ABS_PT_OFFSET,
			0, 0);
		input_set_abs_params(touch_dev->input_dev, ABS_MT_POSITION_X,
			touch_dev->cap_info.MinY,
			touch_dev->cap_info.MaxY + ABS_PT_OFFSET,
			0, 0);
	} else {
		input_set_abs_params(touch_dev->input_dev, ABS_MT_POSITION_X,
			touch_dev->cap_info.MinX,
			touch_dev->cap_info.MaxX + ABS_PT_OFFSET,
			0, 0);
		input_set_abs_params(touch_dev->input_dev, ABS_MT_POSITION_Y,
			touch_dev->cap_info.MinY,
			touch_dev->cap_info.MaxY + ABS_PT_OFFSET,
			0, 0);
	}

	input_set_abs_params(touch_dev->input_dev, ABS_TOOL_WIDTH,
		0, 255, 0, 0);
	input_set_abs_params(touch_dev->input_dev, ABS_MT_TOUCH_MAJOR,
		0, 255, 0, 0);
	input_set_abs_params(touch_dev->input_dev, ABS_MT_WIDTH_MAJOR,
		0, 255, 0, 0);

#ifdef ABS_MT_TRACKING_ID
	input_set_abs_params(touch_dev->input_dev, ABS_MT_TRACKING_ID, 
		0, touch_dev->cap_info.multi_fingers, 0, 0);
#endif			
	

	zinitix_debug_msg("register %s input device \r\n",
		touch_dev->input_dev->name);
	ret = input_register_device(touch_dev->input_dev);
	if (ret) {
		printk(KERN_ERR "unable to register %s input device\r\n",
			touch_dev->input_dev->name);
		goto err_input_register_device;
	}

	/* configure touchscreen interrupt gpio */
	ret = gpio_request(GPIO_TOUCH_PIN_NUM, "zinitix_irq_gpio");
	if (ret) {
		printk(KERN_ERR "unable to request gpio.(%s)\r\n",
			touch_dev->input_dev->name);
		goto err_request_irq;
	}

	ret = gpio_direction_input(GPIO_TOUCH_PIN_NUM);

	touch_dev->int_gpio_num = GPIO_TOUCH_PIN_NUM;

#ifdef	GPIO_TOUCH_IRQ
	touch_dev->irq = GPIO_TOUCH_IRQ;
#else
	touch_dev->irq = gpio_to_irq(touch_dev->int_gpio_num);
	if (touch_dev->irq < 0)
		printk(KERN_INFO "error. gpio_to_irq(..) function is not \
			supported? you should define GPIO_TOUCH_IRQ.\r\n");
#endif
	zinitix_debug_msg("request irq (irq = %d, pin = %d) \r\n",
		touch_dev->client->irq, touch_dev->int_gpio_num);

	touch_dev->work_proceedure = TS_NO_WORK;
	sema_init(&touch_dev->work_proceedure_lock, 1);

	touch_dev->irq = touch_dev->client->irq;

	if (touch_dev->client->irq) {

        #if USE_THREADED_IRQ
        ret = request_threaded_irq(touch_dev->irq, ts_int_handler, zinitix_touch_work, IRQF_TRIGGER_FALLING | IRQF_ONESHOT , ZINITIX_DRIVER_NAME, touch_dev);
        #else
		ret = request_irq(touch_dev->client->irq, ts_int_handler, IRQF_TRIGGER_FALLING, ZINITIX_DRIVER_NAME, touch_dev);
        #endif

		if (ret) {
			printk(KERN_ERR "unable to register irq.(%s)\r\n",
				touch_dev->input_dev->name);
			goto err_request_irq;
		}
	}
	dev_info(&client->dev, "zinitix touch probe.\r\n");

#ifdef CONFIG_HAS_EARLYSUSPEND
	touch_dev->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	touch_dev->early_suspend.suspend = zinitix_early_suspend;
	touch_dev->early_suspend.resume = zinitix_late_resume;
	register_early_suspend(&touch_dev->early_suspend);
#endif

#if	USE_TEST_RAW_TH_DATA_MODE
	sema_init(&touch_dev->raw_data_lock, 1);

	misc_touch_dev = touch_dev;

	ret = misc_register(&touch_misc_device);
	if (ret)
		zinitix_debug_msg("Fail to register touch misc device.\n");

	if (device_create_file(touch_misc_device.this_device,
		&dev_attr_get_touch_raw_data) < 0)
		printk(KERN_INFO "Failed to create device file(%s)!\n",
			dev_attr_get_touch_raw_data.attr.name);

#endif


      /* sys fs */
      touch_class = class_create(THIS_MODULE, "touch");
	if (IS_ERR(touch_class))
		pr_err("[TSP] Failed to create class(touch)!\n");
	firmware_dev = device_create(touch_class, NULL, 0, NULL, "firmware");
	if (IS_ERR(firmware_dev))
		pr_err("[TSP] Failed to create device(firmware)!\n");
	if (device_create_file(firmware_dev, &dev_attr_phone_fw_ver) < 0)
		pr_err("[TSP] Failed to create device file(%s)!\n", dev_attr_phone_fw_ver.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_part_fw_ver) < 0)
		pr_err("[TSP] Failed to create device file(%s)!\n", dev_attr_part_fw_ver.attr.name);
      if (device_create_file(firmware_dev, &dev_attr_threshold_fw_ver) < 0)
		pr_err("[TSP] Failed to create device file(%s)!\n", dev_attr_threshold_fw_ver.attr.name);
      if (device_create_file(firmware_dev, &dev_attr_config_fw_ver) < 0)
		pr_err("[TSP] Failed to create device file(%s)!\n", dev_attr_config_fw_ver.attr.name);
	//if (device_create_file(firmware_dev, &dev_attr_tsp_StartCall) < 0)
		//pr_err("[TSP] Failed to create device file(%s)!\n", dev_attr_tsp_StartCall.attr.name);
	//if (device_create_file(firmware_dev, &dev_attr_tsp_EndCall) < 0)
		//pr_err("[TSP] Failed to create device file(%s)!\n", dev_attr_tsp_EndCall.attr.name);
	//if (device_create_file(firmware_dev, &dev_attr_debug_xy) < 0)
		//pr_err("[TSP] Failed to create device file(%s)!\n", dev_attr_debug_xy.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_fw_update) < 0)
		pr_err("[TSP] Failed to create device file(%s)!\n", dev_attr_fw_update.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_fw_upstatus) < 0)
		pr_err("[TSP] Failed to create device file(%s)!\n", dev_attr_fw_upstatus.attr.name);	
      if (device_create_file(firmware_dev, &dev_attr_raw_enable) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_raw_enable.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_raw_disable) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_raw_disable.attr.name);
      if (device_create_file(firmware_dev, &dev_attr_raw_value) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_raw_value.attr.name);
      /* sys fs */


	return 0;

err_request_irq:
	input_unregister_device(touch_dev->input_dev);
err_input_register_device:
	input_free_device(touch_dev->input_dev);
err_kthread_create_failed:
err_input_allocate_device:
	kfree(touch_dev);
err_alloc_dev_data:
err_check_functionality:

	return ret;
}


static int zinitix_touch_remove(struct i2c_client *client)
{
	struct zinitix_touch_dev *touch_dev = i2c_get_clientdata(client);

	zinitix_debug_msg("zinitix_touch_remove+ \r\n");
	down(&touch_dev->work_proceedure_lock);
	if (touch_dev->work_proceedure != TS_NO_WORK)
		flush_work(&touch_dev->work);

	touch_dev->work_proceedure = TS_REMOVE_WORK;

	if (touch_dev->use_esd_timer != 0) {
		flush_work(&touch_dev->tmr_work);
		ts_write_reg(touch_dev->client,
			ZINITIX_PERIODICAL_INTERRUPT_INTERVAL, 0);
		ts_esd_timer_stop(touch_dev);
		zinitix_debug_msg(KERN_INFO " ts_esd_timer_stop\n");
		destroy_workqueue(zinitix_tmr_workqueue);
	}

	if (touch_dev->irq)
		free_irq(touch_dev->irq, touch_dev);

#if USE_TEST_RAW_TH_DATA_MODE
	device_remove_file(touch_misc_device.this_device,
		&dev_attr_get_touch_raw_data);
	misc_deregister(&touch_misc_device);
#endif


#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&touch_dev->early_suspend);
#endif

       #if USE_THREADED_IRQ
      /* Do Nothing */
      #else
	destroy_workqueue(zinitix_workqueue);
      #endif

	if (gpio_is_valid(touch_dev->int_gpio_num) != 0)
		gpio_free(touch_dev->int_gpio_num);

	input_unregister_device(touch_dev->input_dev);
	input_free_device(touch_dev->input_dev);
	up(&touch_dev->work_proceedure_lock);
	kfree(touch_dev);

	return 0;
}

static int __devinit zinitix_touch_init(void)
{

	m_isp_client = NULL;
	return i2c_add_driver(&zinitix_touch_driver);
}

static void __exit zinitix_touch_exit(void)
{
	i2c_del_driver(&zinitix_touch_driver);
}

static ssize_t phone_firmware_show(struct device *dev, struct device_attribute *attr, char *buf)
{   
    int HW_ver;
    int phone_ver;
    int ic_ver;

    zinitix_debug_msg("[TSP] %s\n",__func__);
    HW_ver = g_touch_dev->cap_info.chip_revision;
    ic_ver = g_touch_dev->cap_info.chip_reg_data_version;
    phone_ver = (u16) (m_firmware_data[FIRMWARE_VERSION_POS+2] | (m_firmware_data[FIRMWARE_VERSION_POS+3]<<8));

    //zinitix_debug_msg("chip_revision : %d, HW_ver = %d\n", g_touch_dev->cap_info.chip_revision, HW_ver);  // 0xb , 11
    //zinitix_debug_msg("chip_firmware_version : %d, phone_ver = %d\n", g_touch_dev->cap_info.chip_firmware_version, phone_ver);  //0x102, 258
    //zinitix_debug_msg("phone(m_firmware_data[0]) = %d\n", m_firmware_data[0]); 

    // sprintf(buf, "0%d\n", (HW_ver*100000) + (phone_ver*1000)+(ic_ver*10));
    sprintf(buf, "%02x\n", phone_ver);

    return sprintf(buf, "%s", buf );
}

static ssize_t part_firmware_show(struct device *dev, struct device_attribute *attr, char *buf)
{   
    int HW_ver;
    int phone_ver;
    int ic_ver;

    zinitix_debug_msg("[TSP] %s\n",__func__);


    HW_ver = g_touch_dev->cap_info.chip_revision;
    ic_ver = g_touch_dev->cap_info.chip_reg_data_version;
    phone_ver = (u16) (m_firmware_data[FIRMWARE_VERSION_POS+2] | (m_firmware_data[FIRMWARE_VERSION_POS+3]<<8));

    //zinitix_debug_msg("chip_revision : %d, HW_ver = %d\n", g_touch_dev->cap_info.chip_revision, HW_ver);  // 0xb , 11
    //zinitix_debug_msg("chip_firmware_version : %d, phone_ver = %d\n", g_touch_dev->cap_info.chip_firmware_version, phone_ver);  //0x102, 258
    //zinitix_debug_msg("phone(m_firmware_data[0]) = %d\n", m_firmware_data[0]); 

    // sprintf(buf, "0%d\n", (HW_ver*100000) + (phone_ver*1000)+(ic_ver*10));
    sprintf(buf, "%02x\n",ic_ver);

    return sprintf(buf, "%s", buf );
}

static ssize_t threshold_firmware_show(struct device *dev, struct device_attribute *attr, char *buf)
{   
    int HW_ver;
    int phone_ver;
    int ic_ver;

    zinitix_debug_msg("[TSP] %s\n",__func__);

    HW_ver = g_touch_dev->cap_info.chip_revision;
    ic_ver = g_touch_dev->cap_info.chip_reg_data_version;
    phone_ver = (u16) (m_firmware_data[FIRMWARE_VERSION_POS+2] | (m_firmware_data[FIRMWARE_VERSION_POS+3]<<8));

    //zinitix_debug_msg("chip_revision : %d, HW_ver = %d\n", g_touch_dev->cap_info.chip_revision, HW_ver);  // 0xb , 11
    //zinitix_debug_msg("chip_firmware_version : %d, phone_ver = %d\n", g_touch_dev->cap_info.chip_firmware_version, phone_ver);  //0x102, 258
    //zinitix_debug_msg("phone(m_firmware_data[0]) = %d\n", m_firmware_data[0]); 

    // sprintf(buf, "0%d\n", (HW_ver*100000) + (phone_ver*1000)+(ic_ver*10));
    sprintf(buf, "%02x\n", 0x300);

    return sprintf(buf, "%s", buf );
}

static ssize_t config_firmware_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int HW_ver;
    int phone_ver;
    int ic_ver;

    zinitix_debug_msg("[TSP] %s\n",__func__);
    HW_ver = g_touch_dev->cap_info.chip_revision;
    ic_ver = g_touch_dev->cap_info.chip_reg_data_version;
    phone_ver = (u16) (m_firmware_data[FIRMWARE_VERSION_POS+2] | (m_firmware_data[FIRMWARE_VERSION_POS+3]<<8));

    //zinitix_debug_msg("chip_revision : %d, HW_ver = %d\n", g_touch_dev->cap_info.chip_revision, HW_ver);  // 0xb , 11
    //zinitix_debug_msg("chip_firmware_version : %d, phone_ver = %d\n", g_touch_dev->cap_info.chip_firmware_version, phone_ver);  //0x102, 258
    //zinitix_debug_msg("phone(m_firmware_data[0]) = %d\n", m_firmware_data[0]); 

    // sprintf(buf, "0%d\n", (HW_ver*100000) + (phone_ver*1000)+(ic_ver*10));
    sprintf(buf, "%02x\n", HW_ver);

    return sprintf(buf, "%s", buf );
}

static ssize_t firmware_update(struct device *dev, struct device_attribute *attr, char *buf)
{
    int i;
    u16 chip_eeprom_info;
   
	zinitix_debug_msg("[TSP] %s\n",__func__);
	sprintf(IsfwUpdate,"%s\n",FW_DOWNLOADING);


	disable_irq(misc_touch_dev->irq);				
	down(&misc_touch_dev->work_proceedure_lock);	
	misc_touch_dev->work_proceedure = TS_IN_UPGRADE;
#if	ZINITIX_ESD_TIMER_INTERVAL	
	if(misc_touch_dev->use_esd_timer)					
	{				
		ts_esd_timer_stop(misc_touch_dev);
	}
#endif

	zinitix_debug_msg("clear all reported points\r\n");
	zinitix_clear_report_data(misc_touch_dev);

	misc_touch_dev->cap_info.chip_fw_size = 32*1024;


		for(i=0; i < 5; i++) {
			zinitix_debug_msg("[zinitix touch] manual upgrade count = %d\r\n", i);	 
			if( ts_upgrade_firmware(misc_touch_dev, &m_firmware_data[2], 
				misc_touch_dev->cap_info.chip_fw_size) == true)
				break;
		}
		if(i >= 5) 
		{
			zinitix_debug_msg("[zinitix touch] failed to manual upgrade\r\n");	 
			return sprintf(buf, "%d", -1 );
			//return sprintf(buf, "%s", "fail!!\n" );
		} else {
			// hw calibration and make checksum
			zinitix_debug_msg("HW calibration 1 !!! \r\n");
			if (ts_write_reg(misc_touch_dev->client, ZINITIX_TOUCH_MODE, 0x07) != I2C_SUCCESS)
				goto fail_hwcal_init;
                   zinitix_debug_msg("HW calibration 2 !!! \r\n");
			if (ts_write_cmd(misc_touch_dev->client, ZINITIX_CALIBRATE_CMD) != I2C_SUCCESS)
				goto fail_hwcal_init;
                   zinitix_debug_msg("HW calibration 3 !!! \r\n");
			if (ts_write_cmd(misc_touch_dev->client, ZINITIX_SWRESET_CMD) != I2C_SUCCESS)
				goto fail_hwcal_init;
			mdelay(100);
			ts_write_cmd(misc_touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
			mdelay(100);
			ts_write_cmd(misc_touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
			mdelay(100);
			ts_write_cmd(misc_touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
			mdelay(100);		
			/* wait for h/w calibration*/
			do {
				mdelay(1000);
				if (ts_read_data(misc_touch_dev->client, ZINITIX_EEPROM_INFO_REG, (u8 *)&chip_eeprom_info, 2) < 0)
					goto fail_hwcal_init;
				zinitix_debug_msg("touch eeprom info = 0x%04X\r\n", chip_eeprom_info);
				if (!zinitix_bit_test(chip_eeprom_info, 0))
					break;
			} while (1);
                    zinitix_debug_msg("HW calibration 4 !!! \r\n");
			if (ts_write_reg(misc_touch_dev->client, ZINITIX_TOUCH_MODE, TOUCH_MODE) != I2C_SUCCESS)
				goto fail_hwcal_init;
			mdelay(10);
                    zinitix_debug_msg("HW calibration 5 !!! \r\n");
			if (ts_write_cmd(misc_touch_dev->client, ZINITIX_SWRESET_CMD) != I2C_SUCCESS)
				goto fail_hwcal_init;

			mdelay(10);
                   zinitix_debug_msg("HW calibration 6 !!! \r\n");
			if (ts_write_cmd(misc_touch_dev->client, ZINITIX_SAVE_CALIBRATION_CMD) != I2C_SUCCESS)
				goto fail_hwcal_init;
			mdelay(1000);
		}

	if(ts_init_touch(misc_touch_dev)==false)
	{
		enable_irq(misc_touch_dev->irq);
		misc_touch_dev->work_proceedure = TS_NO_WORK;
		up(&misc_touch_dev->work_proceedure_lock);
             zinitix_debug_msg("\n[TSP] fail firmware_update\n");
		return sprintf(buf, "%d", -1 );
		//return sprintf(buf, "%s", "fail!!\n" );
	}

      enable_irq(misc_touch_dev->irq);
	misc_touch_dev->work_proceedure = TS_NO_WORK;
	up(&misc_touch_dev->work_proceedure_lock);	
	sprintf(IsfwUpdate,"%s\n",FW_DOWNLOAD_COMPLETE);
	zinitix_debug_msg("\n[TSP] success firmware_update\n");
      return sprintf(buf, "%d", 1 );
      //return sprintf(buf, "%s", "success!!\n" );

fail_hwcal_init:
#if	ZINITIX_ESD_TIMER_INTERVAL	
	if(misc_touch_dev->use_esd_timer)
	{
		ts_esd_timer_start(ZINITIX_CHECK_ESD_TIMER, misc_touch_dev);
		zinitix_debug_msg("esd timer start\r\n");
	}
#endif				
	enable_irq(misc_touch_dev->irq);
	misc_touch_dev->work_proceedure = TS_NO_WORK;
	up(&misc_touch_dev->work_proceedure_lock);	
	sprintf(IsfwUpdate,"%s\n",FW_DOWNLOAD_FAIL);
	zinitix_debug_msg("\n[TSP] fail firmware_update\n");
      return sprintf(buf, "%d", -1 );
      //return sprintf(buf, "%s", "fail!!\n" );

}
static ssize_t firmware_update_status(struct device *dev, struct device_attribute *attr, char *buf)
{
	zinitix_debug_msg("[TSP] %s\n",__func__);

	return sprintf(buf, "%s\n", IsfwUpdate);
}

static int16_t zinitix_raw_data[X_RAW_DATA][Y_RAW_DATA] = {{0}};

static ssize_t zinitix_enter_testmode(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk(KERN_INFO "[zinitix_touch] %s 111 name : %s\r\n", __FUNCTION__, misc_touch_dev->client->name);

	if(misc_touch_dev == NULL)
	{
		zinitix_debug_msg("device NULL : NULL\n");
		return snprintf(buf, 2, "-1");
	}

	down(&misc_touch_dev->raw_data_lock);	
	misc_touch_dev->raw_mode_flag = TOUCH_TEST_RAW_MODE;

	if (ts_write_reg(misc_touch_dev->client, ZINITIX_PERIODICAL_INTERRUPT_INTERVAL, ZINITIX_SCAN_RATE_HZ*ZINITIX_RAW_DATA_ESD_TIMER_INTERVAL)!=I2C_SUCCESS)
		printk(KERN_INFO "[zinitix_touch] Fail to set ZINITIX_RAW_DATA_ESD_TIMER_INTERVAL.\r\n");
	
	if (ts_write_reg(misc_touch_dev->client, ZINITIX_TOUCH_MODE, TOUCH_TEST_RAW_MODE)!=I2C_SUCCESS)
	{
		printk(KERN_INFO "[zinitix_touch] TEST Mode : Fail to set ZINITX_TOUCH_MODE %d.\r\n", TOUCH_TEST_RAW_MODE);
	}

	ts_write_cmd(misc_touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
		// clear garbage data
	mdelay(100);
	ts_write_cmd(misc_touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);		
	memset(&misc_touch_dev->reported_touch_info, 0x0, sizeof(struct _ts_zinitix_point_info));
	memset(&misc_touch_dev->touch_info, 0x0, sizeof(struct _ts_zinitix_point_info));

	up(&misc_touch_dev->raw_data_lock);
	return 1;
}

static ssize_t zinitix_enter_normalmode(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk(KERN_INFO "[zinitix_touch] %s\r\n", __FUNCTION__);

	if(misc_touch_dev == NULL)
{
		zinitix_debug_msg("device NULL : NULL\n");
		return 0;
	}

	down(&misc_touch_dev->raw_data_lock);	
	misc_touch_dev->raw_mode_flag = TOUCH_NORMAL_MODE;

	printk(KERN_INFO "[zinitix_touch] TEST Mode Exit\r\n");

	if (ts_write_reg(misc_touch_dev->client, ZINITIX_PERIODICAL_INTERRUPT_INTERVAL, ZINITIX_SCAN_RATE_HZ*ZINITIX_ESD_TIMER_INTERVAL)!=I2C_SUCCESS)
		printk(KERN_INFO "[zinitix_touch] Fail to set ZINITIX_PERIODICAL_INTERRUPT_INTERVAL.\r\n");
	
	if (ts_write_reg(misc_touch_dev->client, ZINITIX_TOUCH_MODE, TOUCH_MODE)!=I2C_SUCCESS)
	{	
		printk(KERN_INFO "[zinitix_touch] Fail to set ZINITX_TOUCH_MODE %d.\r\n", TOUCH_MODE);
		}
		// clear garbage data
	ts_write_cmd(misc_touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
	mdelay(100);
	ts_write_cmd(misc_touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);	

	up(&misc_touch_dev->raw_data_lock);
	return 1;
}

static ssize_t rawdata_pass_fail_zinitix(struct device *dev, struct device_attribute *attr, char *buf)
{
	int time_out = 0;
	int i,j;
	int interrup_detecting;
	int written_bytes = 0;	/* error check */

	int phone_ver;
	int ic_ver;


	int16_t raw_data[X_RAW_DATA][Y_RAW_DATA] = {{0,},}; // 2 byte

	printk(KERN_INFO "[zinitix_touch] zinitix_read_raw_data( )\r\n");
	disable_irq(misc_touch_dev->client->irq);

	if (ts_write_reg(misc_touch_dev->client,
		ZINITIX_TOUCH_MODE, 7) != I2C_SUCCESS) {
		printk(KERN_INFO "[zinitix_touch] ZINITIX_TOUCH_MODE fail\r\n");
		goto fail_read_raw; 
	}
	udelay(10);
	
	for (i = 0; i < INT_WAIT_TIME; i++) {
		ts_write_cmd(misc_touch_dev->client,
		ZINITIX_CLEAR_INT_STATUS_CMD);
		time_out = 0;
		interrup_detecting = 1;

		while (gpio_get_value(misc_touch_dev->int_gpio_num)) {
			msleep(1);
			
			if (time_out++ > 20) {
				printk(KERN_INFO "[zinitix_touch] interrupt disable timed out\r\n");
				interrup_detecting = 0;
				break;
			}
		}
		if (i == INT_WAIT_TIME) {
			printk(KERN_INFO "[zinitix_touch] interrupt disable timed out\r\n");
			goto fail_read_raw;  
		}

		if ((interrup_detecting == 1) && (time_out < 20)) {
			if (ts_read_raw_data(misc_touch_dev->client,
					ZINITIX_RAWDATA_REG, (char *)raw_data,
					MAX_TEST_RAW_DATA*2) < 0) {
				printk(KERN_INFO "[zinitix_touch] ZINITIX_RAWDATA_REG fail\r\n");
				goto fail_read_raw;
			}
			memcpy(zinitix_raw_data, raw_data, MAX_TEST_RAW_DATA*2);
#ifdef ZINITIX_N_TEST
			for (j = 0; j < (MAX_TEST_RAW_DATA*2); j++) {
				printk(KERN_INFO "[zinitix_touch] temp_buff[%d]"
				"= %5d\r\n", j, zinitix_raw_data[j]);
			}
#endif
			break;
		}
	}

	ts_write_cmd(misc_touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
	udelay(10);
	
	if (ts_write_reg(misc_touch_dev->client,
			ZINITIX_TOUCH_MODE, TOUCH_MODE) != I2C_SUCCESS) {
		printk(KERN_INFO "[zinitix_touch] ZINITIX_TOUCH_MODE fail\r\n");
		goto fail_read_raw;  
	}
	ts_write_cmd(misc_touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
	udelay(10);
	
	enable_irq(misc_touch_dev->client->irq);

	for (i = 0; i < misc_touch_dev->cap_info.x_node_num; i++) {
		printk(KERN_CONT "[TSP]");
		for (j = 0; j < misc_touch_dev->cap_info.y_node_num; j++)
			printk(KERN_CONT " %5d", raw_data[i][j]);
		printk(KERN_CONT "\n");
	}

	// TSP ndata
	for (i = 0; i < misc_touch_dev->cap_info.x_node_num-1; i++) {
		for (j = 0; j < misc_touch_dev->cap_info.y_node_num; j++) {
			if (raw_data[i][j] < CAL_MIN_NUM)
	{
                        printk("[zinitix1] : returned by -> raw_data[%d][%d] %5d \n", i, j, raw_data[i][j]);
				return snprintf(buf, 2, "0"); // fail
			}

			if (raw_data[i][j] > CAL_MAX_NUM)
		{
                         printk("[zinitix2] : returned by -> raw_data[%d][%d] %5d \n", i, j, raw_data[i][j]);
				return snprintf(buf, 2, "0"); // fail
			}
		}
	}

	// TOUCH KEY ndata
	for (j = 0; j < misc_touch_dev->cap_info.y_node_num; j++) {
		if ((j == 1) || (j == 4) || (j == 7)) {
			if (raw_data[misc_touch_dev->cap_info.x_node_num-1][j] <  CAL_MIN_NUM)
	{
                          printk("[zinitix3] : returned by -> raw_data[%d][%d] %5d \n", misc_touch_dev->cap_info.x_node_num-1, j, raw_data[misc_touch_dev->cap_info.x_node_num-1][j]);
				return snprintf(buf, 2, "0"); // fail
			}

			if (raw_data[misc_touch_dev->cap_info.x_node_num-1][j] > CAL_MAX_NUM)
		{
                          printk("[zinitix4] : returned by -> raw_data[%d][%d] %5d \n", misc_touch_dev->cap_info.x_node_num-1, j, raw_data[misc_touch_dev->cap_info.x_node_num-1][j]);
				return snprintf(buf, 2, "0"); // fail
			}
		}
	}

      
      ic_ver = g_touch_dev->cap_info.chip_reg_data_version;
      phone_ver = (u16) (m_firmware_data[FIRMWARE_VERSION_POS+2] | (m_firmware_data[FIRMWARE_VERSION_POS+3]<<8));

	zinitix_debug_msg("phone_ver = %d, ic_ver = %d\n",phone_ver,ic_ver);

	if(phone_ver != ic_ver)
		return snprintf(buf, 2, "0"); // fail


	return snprintf(buf, 2, "1"); // success

	for (i = 0; i < misc_touch_dev->cap_info.x_node_num; i++) {
		for (j = 0; j < misc_touch_dev->cap_info.y_node_num; j++) {
			written_bytes += sprintf(buf+written_bytes, "%d,",raw_data[i][j]);
		}
	}

      printk("[zinitix_touch] written_bytes = %d \n",written_bytes);
	if(written_bytes > 0)
		return written_bytes;  
fail_read_raw:
	if (ts_write_reg(misc_touch_dev->client,
			ZINITIX_TOUCH_MODE, TOUCH_MODE) != I2C_SUCCESS) {
		printk(KERN_INFO "[zinitix_touch] ZINITIX_TOUCH_MODE fail\r\n");
		goto fail_read_raw;
	}
	ts_write_cmd(misc_touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
	udelay(10);
	
	enable_irq(misc_touch_dev->client->irq);
	return snprintf(buf, 2, "-1");
 }

module_init(zinitix_touch_init);
module_exit(zinitix_touch_exit);

MODULE_DESCRIPTION("zinitix touch-screen device driver using i2c interface");
MODULE_AUTHOR("sohnet <seonwoong.jang@zinitix.com>");
MODULE_LICENSE("GPL");


