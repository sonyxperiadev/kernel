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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
// Version 2.0.0 : using reg data file (2010/11/05)
// Version 2.0.1 : syntxt bug fix (2010/11/09)
// Version 2.0.2 : Save status cmd delay bug (2010/11/10)
// Version 2.0.3 : modify delay 10ms -> 50ms for clear hw calibration bit
//		: modify ZINITIX_TOTAL_NUMBER_OF_Y register (read only -> read/write )
//		: modify SUPPORTED FINGER NUM register (read only -> read/write )
// Version 2.0.4 : [20101116]
//	Modify Firmware Upgrade routine.
// Version 2.0.5 : [20101118]
//	add esd timer function & some bug fix.
//	you can select request_threaded_irq or request_irq, setting USE_THREADED_IRQ.
// Version 2.0.6 : [20101123]
//	add ABS_MT_WIDTH_MAJOR Report
// Version 2.0.7 : [20101201]
//	Modify zinitix_early_suspend() / zinitix_late_resume() routine.
// Version 2.0.8 : [20101216]
//	add using spin_lock option
// Version 2.0.9 : [20101216]
//	Test Version
// Version 2.0.10 : [20101217]
//	add USE_THREAD_METHOD option. if  USE_THREAD_METHOD = 0, you use workqueue
// Version 2.0.11 : [20101229]
//	add USE_UPDATE_SYSFS option for update firmware. && TOUCH_MODE == 1 mode.
// Version 2.0.13 : [20110125]
//	modify esd timer routine
// Version 2.0.14 : [20110217]
//	esd timer bug fix. (kernel panic)
//	sysfs bug fix.
// Version 2.0.15 : [20110315]
//	add power off delay ,250ms
// Version 2.0.16 : [20110316]
//	add upgrade method using isp
// Version 2.0.17 : [20110406]
//	change naming rule : sain -> zinitix
//    (add) pending interrupt skip
//	add isp upgrade mode
//	remove warning message when complile

// Version 3.0.2 : [20110711]
//   support bt4x3 series
// Version 3.0.3 : [20110720]
//   add raw data monitoring func.
//   add the h/w calibration skip option.
// Version 3.0.4 : [20110728]
//   use threaded irq
// Version 3.0.5 : [20110801]
//   fix some bugs.
// Version 3.0.6 : [20110802]
//   fix Bt4x3 isp upgrade bug.
//	add	USE_TS_MISC_DEVICE option  for showing info & upgrade
//	remove USE_UPDATE_SYSFS option
// Version 3.0.7 : [201108016]
//   merge USE_TS_MISC_DEVICE option  and USE_TEST_RAW_TH_DATA_MODE
//	fix work proceedure bug.
// Version 3.0.8 / Version 3.0.9 : [201108017]
//   add ioctl func. 
// Version 3.0.10 : [201108030]
//   support REAL_SUPPORTED_FINGER_NUM
// Version 3.0.11 : [201109014]
//   support zinitix apps.
// Version 3.0.12 : [201109015]
//   add disable touch event func.
// Version 3.0.13 : [201109015]
//   add USING_CHIP_SETTING option. : interrupt mask / button num / finger num
// Version 3.0.14 : [201109020]
//   support apps. above kernel version 2.6.35
// Version 3.0.15 : [201101004]
//   modify timing in firmware upgrade
//	retry when failt to upgrade firmware


#include <linux/module.h>
#include <linux/input.h>
#include <linux/i2c.h>		// I2C_M_NOSTART
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


#include <asm/io.h>
#include <linux/delay.h>
#include <mach/gpio.h>
#include <asm/uaccess.h>
// PSJ #include <linux/i2c/tsp_gpio.h>
// PSJ #include <linux/d1982/pmic.h>
// platform dependent
// -----------------------------
//#include <mach/regs-mem.h>
//#include <mach/regs-gpio.h>
//#include <mach/gpio-bank.h>
// -----------------------------


#include "zinitix_tsp_bt403_amazing.h"

#include <linux/time.h>		//KEY_LED_CONTROL
#include <linux/timer.h>
//#include <linux/mfd/pmic8058.h>

#if	BT4x2_Series
#include "zinitix_touch_firmware.h"
#include "zinitix_touch_reg_data.h"
#endif

#if	BT4x3_Above_Series
//#include "zinitix_touch_bt4x3_firmware.h"	//V75
//#include "zinitix_touch_bt4x3_firmware_R01.h"	//V76
//#include "ZinitixFW/zinitix_touch_bt4x3_firmware.h"	//V78_test
#include "ZinitixFW/zinitix_touch_bt4x3_firmware_R001.h"	//R21
#include "ZinitixFW/zinitix_touch_bt4x3_reg_data.h"
#endif

//#define SEC_DRIVER 
//#define KEY_LED_CONTROL

#define TSP_INT 121 //PSJ
#define TOUCH_EN 22 //PSJ

#define CONFIG_TREBON_HW01 1
#if 0
static struct regulator *touch_regulator = NULL;
#ifdef CONFIG_TREBON_HW01
static struct regulator *touch_io_regulator = NULL;
#endif
#endif

#define	ZINITIX_DEBUG		1

static	int	m_ts_debug_mode = ZINITIX_DEBUG;

#define	SYSTEM_MAX_X_RESOLUTION	319 //4000	//480
#define	SYSTEM_MAX_Y_RESOLUTION	479 //3900	//800

#ifdef CONFIG_TREBON_HW01
#define T_REGULATOR_TOUCH "D1980_LDO8"
#define T_REGULATOR_IO_TOUCH  "D1980_LDO19"
#endif

#if	RESET_CONTROL
#define	SYSTEM_RESET_PIN	S5PV210_GPG3(0)
#endif



#if	ZINITIX_ESD_TIMER_INTERVAL
static struct workqueue_struct *zinitix_tmr_workqueue;
#endif

#define	zinitix_debug_msg(fmt, args...)	if(m_ts_debug_mode) printk(KERN_INFO "[TSP][%-18s:%5d]" fmt, __FUNCTION__, __LINE__, ## args)

//-------------------------------------------------------
typedef	struct	
{
	u16	x;
	u16	y;
	u8	width;	
	u8	sub_status;
}_ts_zinitix_coord;

typedef	struct	
{
	u16	status;
#if (TOUCH_MODE == 1)
	u16	event_flag;
#else
	u8	finger_cnt;
	u8	time_stamp;
#endif	
	_ts_zinitix_coord	coord[MAX_SUPPORTED_FINGER_NUM];

}_ts_zinitix_point_info;


#define	TOUCH_V_FLIP	0x01
#define	TOUCH_H_FLIP	0x02
#define	TOUCH_XY_SWAP	0x04

typedef	struct
{
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
#if	BT4x3_Above_Series
	u16 max_y_node;
	u16 total_cal_n;
#endif
#endif	
}_ts_capa_info;

typedef	enum
{
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
}_ts_work_proceedure;


typedef struct
{
	struct input_dev *input_dev;
	struct task_struct *task;
	wait_queue_head_t	wait;
	struct work_struct  tmr_work;
	struct i2c_client *client;
	struct semaphore update_lock;    
	u32 i2c_dev_addr;
	_ts_capa_info	cap_info;
	char	phys[32];

	bool is_valid_event;
	_ts_zinitix_point_info touch_info;
	_ts_zinitix_point_info reported_touch_info;    	
	u16 icon_event_reg;	
	u16 event_type;
	u32 int_gpio_num;
	u32 irq;
	u8 button[MAX_SUPPORTED_BUTTON_NUM];

	u8 work_proceedure;	
	struct semaphore work_proceedure_lock;

#if	RESET_CONTROL
	int reset_gpio_num;
#endif

	u8	use_esd_timer;
#if	ZINITIX_ESD_TIMER_INTERVAL
	bool in_esd_timer;
	struct timer_list esd_timeout_tmr;		//for repeated card detecting work
	struct timer_list *p_esd_timeout_tmr;		//for repeated card detecting work
#endif

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
} zinitix_touch_dev;


#define ZINITIX_DRIVER_NAME        "Zinitix_tsp"

#if	TOUCH_USING_ISP_METHOD
#define ZINITIX_ISP_NAME        "zinitix_isp"	
struct i2c_client *m_isp_client = NULL;
#endif
#define FWUP_OFF 0
#define FWUP_ON 1
#define FWUP_NOW -1

static struct i2c_device_id zinitix_idtable[] = {
#if	TOUCH_USING_ISP_METHOD
	{ZINITIX_ISP_NAME, 0},
#endif
	{ZINITIX_DRIVER_NAME, 0},  	// i2c register.
	{ }   
};

int zinitixTsp_irq;

u32 BUTTON_MAPPING_KEY[MAX_SUPPORTED_BUTTON_NUM]={KEY_MENU, KEY_BACK};	//SEC_DRIVER

// FW version
zinitix_touch_dev *global_touch_dev;
int IsDuringCall = 0;
static int IsdebugMatrix = 0;
static u8 IsfwUpdate = 0;
/* sys fs */
struct class *touch_class;
EXPORT_SYMBOL(touch_class);
struct device *firmware_dev;
EXPORT_SYMBOL(firmware_dev);
static ssize_t firmware_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t firmware_sw_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t tsp_StartCall_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t tsp_EndCall_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t tsp_xy_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t firmware_update(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t firmware_update_status(struct device *dev, struct device_attribute *attr, char *buf);
//static ssize_t firmware_store( struct device *dev, struct device_attribute *attr, const char *buf, size_t size);
static DEVICE_ATTR(firmware_ver	, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, firmware_show, NULL);
static DEVICE_ATTR(sw_ver	, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, firmware_sw_show, NULL);
static DEVICE_ATTR(tsp_StartCall	, S_IRUGO | S_IWUSR | S_IWGRP | S_IXOTH, tsp_StartCall_show, NULL);
static DEVICE_ATTR(tsp_EndCall	, S_IRUGO | S_IWUSR | S_IWGRP | S_IXOTH, tsp_EndCall_show, NULL);
static DEVICE_ATTR(debug_xy, S_IRUGO | S_IWUSR | S_IWGRP | S_IXOTH, tsp_xy_show, NULL);
static DEVICE_ATTR(fw_update, S_IRUGO | S_IWUSR | S_IWGRP | S_IXOTH, firmware_update, NULL);
static DEVICE_ATTR(fw_upstatus, S_IRUGO | S_IWUSR | S_IWGRP | S_IXOTH, firmware_update_status, NULL);
// define sub functions
//==================================================================
#if BT4x2_Series
inline s32 ts_write_cmd(struct i2c_client *client, u8 reg)
{
	s32 ret;
	ret = i2c_smbus_write_byte(client, reg);
	udelay(DELAY_FOR_POST_TRANSCATION);
	return ret;
}

inline s32 ts_write_reg(struct i2c_client *client, u8 reg, u16 value)
{
	s32 ret;
	ret = i2c_smbus_write_word_data(client, reg, value);
	udelay(DELAY_FOR_POST_TRANSCATION);
	return ret;
}

inline s32 ts_read_data(struct i2c_client *client, u8 reg, u8 *values, u16 length)
{
	s32 ret;
	if((ret = i2c_master_send(client , &reg , 1)) < 0)	return ret;	// select register
	udelay(DELAY_FOR_TRANSCATION);		// for setup tx transaction.
	if((ret = i2c_master_recv(client , values , length)) < 0)	return ret;
	udelay(DELAY_FOR_POST_TRANSCATION);
	return length;
}

inline s32 ts_write_data(struct i2c_client *client, u8 reg, u8 *values, u16 length)
{
	s32 ret;
	ret = i2c_smbus_write_i2c_block_data(client, reg, length, values);
	udelay(DELAY_FOR_POST_TRANSCATION);
	return ret;
}

inline s32 ts_read_raw_data(struct i2c_client *client, u8 reg, u8 *values, u16 length)
{
	s32 ret;
	if((ret = i2c_master_send(client , &reg , 1)) < 0)	return ret;	// select register
	mdelay(5);		// for setup tx transaction
	if((ret = i2c_master_recv(client , values , length)) < 0)	return ret;
	udelay(DELAY_FOR_POST_TRANSCATION);	
	return length;
}
#endif
#define TOUCH_ON 1
#define TOUCH_OFF 0


void touch_power_control(int on_off)
{
	int rc;
	
	if (on_off == TOUCH_ON)
	{
		rc = gpio_request(TOUCH_EN,"Touch_en");
		if (rc < 0)
		{
			printk("[TSP] touch_power_control unable to request GPIO pin");
			//printk(KERN_ERR "unable to request GPIO pin %d\n", TSP_INT_GPIO_PIN);
			return;
		}
		gpio_direction_output(TOUCH_EN,1);
		gpio_set_value(TOUCH_EN,1);
		gpio_free(TOUCH_EN);
	}
	else if(on_off == TOUCH_OFF)
	{
		gpio_request(TOUCH_EN,"Touch_en");
		gpio_direction_output(TOUCH_EN,0);
		gpio_set_value(TOUCH_EN,0);
		gpio_free(TOUCH_EN);
	}
}


#if 0 //PSJ
void touch_ctrl_regulator(int on_off)
{
	int ret=0;

	printk("[TSP] %s, %d\n", __func__, on_off );

	if(touch_regulator == NULL)
#ifdef CONFIG_TREBON_HW01
//PSJ		touch_regulator = regulator_get(NULL,T_REGULATOR_TOUCH);
#else
		touch_regulator = regulator_get(NULL,REGULATOR_TOUCH);
#endif
	if(on_off==TOUCH_ON)
	{
		regulator_set_voltage(touch_regulator,OUTPUT_3_3V,OUTPUT_3_3V);
		ret = regulator_enable(touch_regulator);
		if (ret) {
			printk(KERN_ERR "%s: regulator_touch enable failed (%d)\n",__func__, ret);
		}
	}
	else
	{
		ret = regulator_disable(touch_regulator);
		if (ret) {
			printk(KERN_ERR "%s: regulator_touch disable failed (%d)\n",__func__, ret);
		}        
	}
}

#ifdef CONFIG_TREBON_HW01
void touch_io_ctrl_regulator(int on_off)
{
	int ret=0,ret2=0;

	printk("[TSP] %s, %d\n", __func__, on_off );
	if(touch_io_regulator == NULL)
		touch_io_regulator = regulator_get(NULL,T_REGULATOR_IO_TOUCH);

	if(on_off==TOUCH_ON)
	{
		regulator_set_voltage(touch_io_regulator,OUTPUT_1_8V,OUTPUT_1_8V);
		ret = regulator_enable(touch_io_regulator);
		if (ret) {
			printk(KERN_ERR "%s: regulator_touch_io enable failed (%d)\n",__func__, ret);
		}
	}
	else
	{
		ret = regulator_disable(touch_io_regulator);
		if (ret) {
			printk(KERN_ERR "%s: regulator_touch_io disable failed (%d)\n",__func__, ret);
		}  
	}
}
#endif

EXPORT_SYMBOL(touch_ctrl_regulator);
#ifdef CONFIG_TREBON_HW01
EXPORT_SYMBOL(touch_io_ctrl_regulator);
#endif

#endif

#if BT4x3_Above_Series

inline s32 ts_read_data(struct i2c_client *client, u16 reg, u8 *values, u16 length)
{
	s32 ret;
	if((ret = i2c_master_send(client , (u8*)&reg , 2)) < 0)	
	{
		printk("[TSP] ts_read_data 1 : %d\n", ret);
		return ret;	// select register
	}
	udelay(DELAY_FOR_TRANSCATION);		// for setup tx transaction.
	if((ret = i2c_master_recv(client , values , length)) < 0)
	{
		printk("[TSP] ts_read_data 2: %d\n", ret);
		return ret;
	}
	udelay(DELAY_FOR_POST_TRANSCATION);
	return length;
}


inline s32 ts_write_data(struct i2c_client *client, u16 reg, u8 *values, u16 length)
{
	s32 ret;
	u8	pkt[4];
	pkt[0] = (reg)&0xff;
	pkt[1] = (reg >>8)&0xff;
	pkt[2] = values[0];
	pkt[3] = values[1];

	if((ret = i2c_master_send(client , pkt , length+2)) < 0)	
	{
		printk("[TSP] ts_write_data : %d\n", ret);

		return ret;	
	}
	udelay(DELAY_FOR_POST_TRANSCATION);
	return length;
}

inline s32 ts_write_reg(struct i2c_client *client, u16 reg, u16 value)
{
	if(ts_write_data(client, reg, (u8*)&value, 2) < 0)		return -1;
	return I2C_SUCCESS;
}

inline s32 ts_write_cmd(struct i2c_client *client, u16 reg)
{
	s32 ret;
	if((ret = i2c_master_send(client , (u8*)&reg , 2)) < 0)
	{
		printk("[TSP] ts_write_cmd : %d\n", ret);
		return ret;	
	}
	udelay(DELAY_FOR_POST_TRANSCATION);
	return I2C_SUCCESS;
}

inline s32 ts_read_raw_data(struct i2c_client *client, u16 reg, u8 *values, u16 length)
{
	s32 ret;
	if((ret = i2c_master_send(client , (u8*)&reg , 2)) < 0)	
	{
		printk("[TSP] ts_read_raw_data 1 : %d\n", ret);
		return ret;	// select register
	}
	udelay(200);		// for setup tx transaction.
	if((ret = i2c_master_recv(client , values , length)) < 0)
	{
		printk("[TSP] ts_read_raw_data 1 : %d\n", ret);

		return ret;
	}
	udelay(DELAY_FOR_POST_TRANSCATION);
	return length;
}

#endif

#if	TOUCH_USING_ISP_METHOD
inline s32 ts_read_firmware_data(struct i2c_client *client, char *addr, u8 *values, u16 length)
{
	s32 ret;
	if(addr != NULL)
	{
		if((ret = i2c_master_send(client , addr , 2)) < 0)	return ret;	// select register
		mdelay(1);		// for setup tx transaction.
	}
	if((ret = i2c_master_recv(client , values , length)) < 0)	return ret;
	udelay(DELAY_FOR_POST_TRANSCATION);
	return length;
}

inline s32 ts_write_firmware_data(struct i2c_client *client, u8 *values, u16 length)
{
	s32 ret;
	if((ret = i2c_master_send(client , values , length)) < 0)
	{

		return ret;	
	}
	udelay(DELAY_FOR_POST_TRANSCATION);
	return length;
}
#else
inline s32 ts_read_firmware_data(struct i2c_client *client, u8 reg, u8 *values, u16 length)
{
	s32 ret;
	if((ret = i2c_master_send(client , &reg , 1)) < 0)	return ret;	// select register
	mdelay(1);		// for setup tx transaction
	if((ret = i2c_master_recv(client , values , length)) < 0)	return ret;
	udelay(DELAY_FOR_POST_TRANSCATION);	
	return length;
}
#endif

//==================================================================


static int zinitix_touch_probe(struct i2c_client *client, const struct i2c_device_id *i2c_id);
static int zinitix_touch_remove(struct i2c_client *client);
bool ts_init_touch(zinitix_touch_dev* touch_dev);
static void	zinitix_clear_report_data(zinitix_touch_dev *touch_dev);



#if (TOUCH_MODE == 1)
static void	zinitix_report_data(zinitix_touch_dev *touch_dev, int id);
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void zinitix_early_suspend(struct early_suspend *h);
static void zinitix_late_resume(struct early_suspend *h);
#endif

#if	ZINITIX_ESD_TIMER_INTERVAL
static void ts_esd_timer_start(u16 sec, zinitix_touch_dev* touch_dev);
static void ts_esd_timer_stop(zinitix_touch_dev* touch_dev);
static void ts_esd_timer_init(zinitix_touch_dev* touch_dev);
static void ts_esd_timeout_handler(unsigned long data);
#endif


#if USE_TEST_RAW_TH_DATA_MODE
#if 0 //LINUX_VERSION_CODE < KERNEL_VERSION(2.6.36)	
static int ts_misc_fops_ioctl(struct inode *inode, struct file *filp,unsigned int cmd, unsigned long arg);
#else
static long ts_misc_fops_ioctl(struct file *filp,unsigned int cmd, unsigned long arg);
#endif
static int ts_misc_fops_open(struct inode *inode, struct file *filp);
static int ts_misc_fops_close(struct inode *inode, struct file *filp);

static struct file_operations ts_misc_fops = {
	.owner = THIS_MODULE,
	.open = ts_misc_fops_open,
	.release = ts_misc_fops_close,
#if 0//LINUX_VERSION_CODE < KERNEL_VERSION(2.6.36)	
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

#define TOUCH_IOCTL_BASE            		0xbc
#define TOUCH_IOCTL_GET_DEBUGMSG_STATE		_IOW(TOUCH_IOCTL_BASE,0,int)
#define TOUCH_IOCTL_SET_DEBUGMSG_STATE		_IOW(TOUCH_IOCTL_BASE,1,int)
#define TOUCH_IOCTL_GET_CHIP_REVISION		_IOW(TOUCH_IOCTL_BASE,2,int)
#define TOUCH_IOCTL_GET_FW_VERSION		_IOW(TOUCH_IOCTL_BASE,3,int)
#define TOUCH_IOCTL_GET_REG_DATA_VERSION	_IOW(TOUCH_IOCTL_BASE,4,int)
#define TOUCH_IOCTL_VARIFY_UPGRADE_SIZE		_IOW(TOUCH_IOCTL_BASE,5,int)
#define TOUCH_IOCTL_VARIFY_UPGRADE_DATA		_IOW(TOUCH_IOCTL_BASE,6,int)
#define TOUCH_IOCTL_START_UPGRADE		_IOW(TOUCH_IOCTL_BASE,7,int)
#define TOUCH_IOCTL_GET_X_NODE_NUM		_IOW(TOUCH_IOCTL_BASE,8,int)
#define TOUCH_IOCTL_GET_Y_NODE_NUM		_IOW(TOUCH_IOCTL_BASE,9,int)
#define TOUCH_IOCTL_GET_TOTAL_NODE_NUM		_IOW(TOUCH_IOCTL_BASE,10,int)
#define TOUCH_IOCTL_SET_RAW_DATA_MODE		_IOW(TOUCH_IOCTL_BASE,11,int)
#define TOUCH_IOCTL_GET_RAW_DATA		_IOW(TOUCH_IOCTL_BASE,12,int)
#define TOUCH_IOCTL_GET_X_RESOLUTION		_IOW(TOUCH_IOCTL_BASE,13,int)
#define TOUCH_IOCTL_GET_Y_RESOLUTION		_IOW(TOUCH_IOCTL_BASE,14,int)
#define TOUCH_IOCTL_HW_CALIBRAION		_IOW(TOUCH_IOCTL_BASE,15,int)
#define TOUCH_IOCTL_GET_REG			_IOW(TOUCH_IOCTL_BASE,16,int)
#define TOUCH_IOCTL_SET_REG			_IOW(TOUCH_IOCTL_BASE,17,int)
#define TOUCH_IOCTL_SEND_SAVE_STATUS		_IOW(TOUCH_IOCTL_BASE,18,int)
#define TOUCH_IOCTL_DONOT_TOUCH_EVENT		_IOW(TOUCH_IOCTL_BASE,19,int)


zinitix_touch_dev* misc_touch_dev;

#endif //USE_TEST_RAW_TH_DATA_MODE

#ifdef SEC_DRIVER 
int touch_power_onoff( int onoff );
void init_hw(void);
#endif

// id -> include/linux/i2c-id.h 
static struct i2c_driver zinitix_touch_driver = {
	.probe     = zinitix_touch_probe,
	.remove    = zinitix_touch_remove,
	.id_table  = zinitix_idtable,
	.driver    = {
		.name  = ZINITIX_DRIVER_NAME,
	},
};

#ifdef KEY_LED_CONTROL
static struct timer_list  g_led_timer;
static bool key_led_on = false;

static void vled_power_onoff(int onoff)
{
	if(onoff)
	{
		if(!key_led_on)
		{
			gpio_direction_output(TSP_KEY_LED, 1);
			key_led_on = true;
		}
	}
	else
	{
		gpio_direction_output(TSP_KEY_LED, 0);
		key_led_on = false;
	}
}

static void TouchKey_Led_TimerProc(void)
{
	vled_power_onoff(0);
	key_led_on = false;
}


static ssize_t touch_led_control(struct device *dev,
		struct device_attribute *attr, const char *buf,
		size_t size)
{
	unsigned char data;
	int int_data;
	int errnum;	
	printk("[TSP] touch_led_control start\n");

	if (sscanf(buf, "%c\n", &data) == 1) {
//		int_data = atoi(&data);
//		data = data *0x10;
		printk(KERN_DEBUG "[TSP] touch_led_control data: %d\n", data);
		if(data==50){	vled_power_onoff(1);}
		else if(data==48){	vled_power_onoff(0);}


	} else
		printk("[TSP] touch_led_control Error\n");

	return size;
}


#endif


#if	USE_TEST_RAW_TH_DATA_MODE
static bool ts_get_raw_data(zinitix_touch_dev* touch_dev)
{
	down(&touch_dev->raw_data_lock);
	if(touch_dev->raw_mode_flag == TOUCH_TEST_RAW_MODE)
	{
#if	BT4x2_Series			
		if(touch_dev->cap_info.total_node_num*2 + MAX_TEST_POINT_INFO*2 < 512)
		{
			if(ts_read_raw_data(touch_dev->client,ZINITIX_RAWDATA_REG,(char *)touch_dev->cur_data, (unsigned int)((unsigned int)touch_dev->cap_info.total_node_num*2 + MAX_TEST_POINT_INFO*2))<0)
			{
				printk(KERN_INFO "[TSP] error : read zinitix tc raw data\n");
				up(&touch_dev->raw_data_lock); 
				return false;
			}	
		}
		else
		{
			if(ts_read_raw_data(touch_dev->client,ZINITIX_RAWDATA_REG,(char *)&touch_dev->cur_data[0], 512)<0)
			{
				printk(KERN_INFO "[TSP] error : read zinitix tc raw data\n");
				up(&touch_dev->raw_data_lock); 
				return false;
			}			
			udelay(50);
			if(ts_read_raw_data(touch_dev->client,ZINITIX_EXTRA_RAWDATA_REG,(char *)&touch_dev->cur_data[256], (unsigned int)((unsigned int)touch_dev->cap_info.total_node_num*2 + MAX_TEST_POINT_INFO*2)-512)<0)
			{
				printk(KERN_INFO "[TSP] error : read zinitix tc raw data\n");
				up(&touch_dev->raw_data_lock); 
				return false;
			}	
		}
#endif

#if	BT4x3_Above_Series		
		if(ts_read_raw_data(touch_dev->client,ZINITIX_RAWDATA_REG,(char *)touch_dev->cur_data, (unsigned int)((unsigned int)touch_dev->cap_info.total_node_num*2 + MAX_TEST_POINT_INFO*2))<0)
		{
			printk(KERN_INFO "[TSP] error : read zinitix tc raw data\n");
			up(&touch_dev->raw_data_lock); 
			return false;
		}	
#endif			
		if(!zinitix_bit_test(touch_dev->cur_data[touch_dev->cap_info.total_node_num], BIT_ICON_EVENT) && 
				!zinitix_bit_test(touch_dev->cur_data[touch_dev->cap_info.total_node_num], BIT_PT_EXIST))		// no point, so update ref_data
		{
			memcpy((u8*)touch_dev->ref_data, (u8*)touch_dev->cur_data, touch_dev->cap_info.total_node_num*2);
		}
		touch_dev->update = 1;
		memcpy((u8*)(&touch_dev->touch_info), (u8*)&touch_dev->cur_data[touch_dev->cap_info.total_node_num], sizeof(_ts_zinitix_point_info));
		up(&touch_dev->raw_data_lock); 
		return true;		
	}
	else if(touch_dev->raw_mode_flag != TOUCH_NORMAL_MODE)
	{
		zinitix_debug_msg("read raw data\r\n");

#if	BT4x2_Series			
		if(touch_dev->raw_mode_flag == TOUCH_ZINITIX_CAL_N_MODE)
		{
			if(ts_read_raw_data(touch_dev->client,ZINITIX_RAWDATA_REG,(char *)touch_dev->cur_data, 
					(unsigned int)((unsigned int)161*2 + sizeof(_ts_zinitix_point_info)))<0)
			{
				printk(KERN_INFO "[TSP] error : read zinitix tc raw data\n");
				up(&touch_dev->raw_data_lock); 
				return false;
			}		
			misc_touch_dev->update = 1;
			memcpy((u8*)(&touch_dev->touch_info), (u8*)&touch_dev->cur_data[161], sizeof(_ts_zinitix_point_info));
			up(&touch_dev->raw_data_lock); 			
			return true;
		}
		if((unsigned int)((unsigned int)touch_dev->cap_info.total_node_num*2 + sizeof(_ts_zinitix_point_info)) < 512)
		{
			if(ts_read_raw_data(touch_dev->client,ZINITIX_RAWDATA_REG,(char *)touch_dev->cur_data, 
				(unsigned int)((unsigned int)touch_dev->cap_info.total_node_num*2 + sizeof(_ts_zinitix_point_info)))<0)
			{
				printk(KERN_INFO "[TSP] error : read zinitix tc raw data\n");
				up(&touch_dev->raw_data_lock); 
				return false;
			}	
		}
		else
		{			
			if(ts_read_raw_data(touch_dev->client,ZINITIX_RAWDATA_REG,(char *)&touch_dev->cur_data[0], 512)<0)
			{
				printk(KERN_INFO "[TSP] error : read zinitix tc raw data\n");
				up(&touch_dev->raw_data_lock); 
				return false;
			}
			udelay(50);
			if(ts_read_raw_data(touch_dev->client,ZINITIX_EXTRA_RAWDATA_REG,(char *)&touch_dev->cur_data[256], 
					(unsigned int)((unsigned int)touch_dev->cap_info.total_node_num*2 + sizeof(_ts_zinitix_point_info))-512)<0)
			{
				printk(KERN_INFO "[TSP] error : read zinitix tc raw data\n");
				up(&touch_dev->raw_data_lock); 
				return false;
			}				
		}
#endif		
#if	BT4x3_Above_Series			
		if(touch_dev->raw_mode_flag == TOUCH_ZINITIX_CAL_N_MODE)
		{			
			int total_cal_n = touch_dev->cap_info.total_cal_n;
			if(total_cal_n == 0) total_cal_n = MAX_TEST_RAW_DATA;

			if(ts_read_raw_data(touch_dev->client,ZINITIX_RAWDATA_REG,(char *)touch_dev->cur_data, 
				(unsigned int)((unsigned int)total_cal_n*2 + sizeof(_ts_zinitix_point_info)))<0)
			{
				printk(KERN_INFO "[TSP] error : read zinitix tc raw data\n");
				up(&touch_dev->raw_data_lock); 
				return false;
			}		
			misc_touch_dev->update = 1;
			memcpy((u8*)(&touch_dev->touch_info), (u8*)&touch_dev->cur_data[total_cal_n], sizeof(_ts_zinitix_point_info));
			up(&touch_dev->raw_data_lock); 			
			return true;
		}

		if(ts_read_raw_data(touch_dev->client,ZINITIX_RAWDATA_REG,(char *)touch_dev->cur_data,
				(unsigned int)((unsigned int)touch_dev->cap_info.total_node_num*2 + sizeof(_ts_zinitix_point_info)))<0)
		{
			printk(KERN_INFO "[TSP] error : read zinitix tc raw data\n");
			up(&touch_dev->raw_data_lock); 
			return false;
		}		
#endif		
		if(!zinitix_bit_test(touch_dev->cur_data[touch_dev->cap_info.total_node_num], BIT_ICON_EVENT) && 
				!zinitix_bit_test(touch_dev->cur_data[touch_dev->cap_info.total_node_num], BIT_PT_EXIST))		// no point, so update ref_data
		{
			memcpy((u8*)touch_dev->ref_data, (u8*)touch_dev->cur_data, touch_dev->cap_info.total_node_num*2);
		}
		touch_dev->update = 1;
		memcpy((u8*)(&touch_dev->touch_info), (u8*)&touch_dev->cur_data[touch_dev->cap_info.total_node_num], sizeof(_ts_zinitix_point_info));	
	}	
	up(&touch_dev->raw_data_lock); 
	return true;
}
#endif	


static bool ts_get_samples (zinitix_touch_dev* touch_dev)
{
	int i;	

	zinitix_debug_msg("ts_get_samples+\r\n");

	if (gpio_get_value(touch_dev->int_gpio_num))
	{
		//interrupt pin is high, not valid data.
		zinitix_debug_msg("woops... inturrpt pin is high\r\n");
		return false;
	}



#if	USE_TEST_RAW_TH_DATA_MODE
	if(touch_dev->raw_mode_flag != TOUCH_NORMAL_MODE)
	{
		if(ts_get_raw_data(touch_dev) == false)	return false;

#if (TOUCH_MODE == 0)		
		goto continue_check_point_data;
#endif		
	}		
#endif

#if (TOUCH_MODE == 1)

	memset(&touch_dev->touch_info, 0x0, sizeof(_ts_zinitix_point_info));	

	if (ts_read_data (touch_dev->client, ZINITIX_POINT_STATUS_REG, (u8*)(&touch_dev->touch_info), 4)< 0)
	{
		zinitix_debug_msg("error read point info using i2c.-\r\n");
		return false;
	}
	zinitix_debug_msg("status reg = 0x%x , event_flag = 0x%04x\r\n", touch_dev->touch_info.status, touch_dev->touch_info.event_flag);

	if(touch_dev->touch_info.status == 0x0)
	{
		zinitix_debug_msg("periodical esd repeated int occured\r\n");
		return true;
	}

	if(zinitix_bit_test(touch_dev->touch_info.status, BIT_ICON_EVENT))
	{
		udelay(20);
		if (ts_read_data (touch_dev->client, ZINITIX_ICON_STATUS_REG, (u8*)(&touch_dev->icon_event_reg), 2) < 0)
		{
			printk(KERN_INFO "[TSP] error read icon info using i2c.\n");
			return false;
		}
		return true;
	}

	if(!zinitix_bit_test(touch_dev->touch_info.status, BIT_PT_EXIST))
	{
		//ts_write_cmd(touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
		for(i=0; i < touch_dev->cap_info.multi_fingers; i++)
		{		
			if(zinitix_bit_test(touch_dev->reported_touch_info.coord[i].sub_status, SUB_BIT_EXIST))
			{				
				input_report_abs(touch_dev->input_dev, ABS_MT_TOUCH_MAJOR, 0);
				input_report_abs(touch_dev->input_dev, ABS_MT_WIDTH_MAJOR, 0);	
				input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_X, touch_dev->reported_touch_info.coord[i].x);
				input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_Y, touch_dev->reported_touch_info.coord[i].y);
				input_mt_sync(touch_dev->input_dev);	
				touch_dev->reported_touch_info.coord[i].sub_status = 0;
			}
		}	

            printk(KERN_INFO "x = %d, y = %d\n",touch_dev->reported_touch_info.coord[i].x, touch_dev->reported_touch_info.coord[i].y);
		input_sync(touch_dev->input_dev);
		return true;
	}	


	for(i=0; i < touch_dev->cap_info.multi_fingers; i++)
	{
		if(zinitix_bit_test(touch_dev->touch_info.event_flag, i))
		{			
			udelay(20);
			if (ts_read_data (touch_dev->client, ZINITIX_POINT_STATUS_REG+2+i, (u8*)(&touch_dev->touch_info.coord[i]), sizeof(_ts_zinitix_coord))< 0)
			{
				zinitix_debug_msg("error read point info using i2c.-\r\n");
				return false;
			}
			zinitix_bit_clr(touch_dev->touch_info.event_flag, i);
			if(touch_dev->touch_info.event_flag == 0)	
			{
				//ts_write_cmd(touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
				zinitix_report_data(touch_dev, i);
				return true;
			}
			else
				zinitix_report_data(touch_dev, i);
		}		
	}	


#else
	if (ts_read_data (touch_dev->client, ZINITIX_POINT_STATUS_REG, (u8*)(&touch_dev->touch_info), sizeof(_ts_zinitix_point_info))< 0)
	{
		zinitix_debug_msg("error read point info using i2c.-\r\n");
		return false;
	}

continue_check_point_data:	
	zinitix_debug_msg("status reg = 0x%x , point cnt = %d, time stamp = %d\r\n", touch_dev->touch_info.status, 
		touch_dev->touch_info.finger_cnt, touch_dev->touch_info.time_stamp);

	if(touch_dev->touch_info.status == 0x0 && touch_dev->touch_info.finger_cnt == 100)
	{
		zinitix_debug_msg("periodical esd repeated int occured\r\n");
		return true;
	}

	for(i=0; i< MAX_SUPPORTED_BUTTON_NUM; i++)	touch_dev->button[i] = ICON_BUTTON_UNCHANGE;

	if(zinitix_bit_test(touch_dev->touch_info.status, BIT_ICON_EVENT))
	{
		udelay(20);
		if (ts_read_data (touch_dev->client, ZINITIX_ICON_STATUS_REG, (u8*)(&touch_dev->icon_event_reg), 2) < 0)
		{
			printk(KERN_INFO "[TSP] error read icon info using i2c.\n");
			return false;
		}
	}
#endif		
	zinitix_debug_msg("ts_get_samples-\r\n");

	return true;
}



static bool ts_read_coord (zinitix_touch_dev * hDevice)
{
	zinitix_touch_dev* touch_dev = (zinitix_touch_dev*)hDevice;
	//zinitix_debug_msg("ts_read_coord+\r\n");

	if(ts_get_samples(touch_dev)==false)
	{
		return false;
	}

	ts_write_cmd(touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
#if DELAY_FOR_SIGNAL_DELAY	
	udelay(DELAY_FOR_SIGNAL_DELAY);
#endif
	return true;
}

//
#if 0  //Comment out this function for compile warning
static void ts_power_control(zinitix_touch_dev *touch_dev, u8 ctl)
{
	if(ctl == POWER_OFF) //power off
	{
	}
	else if(ctl == POWER_ON)	//power on
	{
	}
	else if(ctl == RESET_LOW)	//reset pin low
	{
#if	RESET_CONTROL
		if(gpio_is_valid(touch_dev->reset_gpio_num))
		{
			printk(KERN_INFO "[TSP] reset pin low\r\n");
//			gpio_direction_output(touch_dev->reset_gpio_num, 0);
			gpio_set_value(touch_dev->reset_gpio_num, 0);			
		}
		else
			printk(KERN_INFO "[TSP] error : zinitix reset pin gpio is invalid\r\n");
#endif
	}
	else if(ctl == RESET_HIGH)	//reset pin high
	{
#if	RESET_CONTROL
		if(gpio_is_valid(touch_dev->reset_gpio_num))
		{
			printk(KERN_INFO "[TSP] reset pin high\r\n");
//			gpio_direction_output(touch_dev->reset_gpio_num, 1);
			gpio_set_value(touch_dev->reset_gpio_num, 1);
		}
		else
			printk(KERN_INFO "[TSP] error : zinitix reset pin gpio is invalid\r\n");
#endif
	}


}
#endif
static bool ts_mini_init_touch(zinitix_touch_dev * touch_dev)
{
	if(touch_dev == NULL)	
	{
		printk(KERN_INFO "[TSP] ts_mini_init_touch : error (touch_dev == NULL?)\r\n");
		return false;		
	}	

	ts_init_touch(touch_dev);

#if	ZINITIX_ESD_TIMER_INTERVAL	
	if(touch_dev->use_esd_timer)
	{
		ts_esd_timer_start(ZINITIX_CHECK_ESD_TIMER, touch_dev);
		zinitix_debug_msg("esd timer start\r\n");
	}
#endif		

	return true;
}


#if	ZINITIX_ESD_TIMER_INTERVAL

static void zinitix_touch_tmr_work(struct work_struct *work)
{
	zinitix_touch_dev *touch_dev = container_of(work, zinitix_touch_dev, tmr_work);	

	printk(KERN_INFO "[TSP] tmr queue work ++\r\n");
	if(touch_dev == NULL)
	{
		printk(KERN_INFO "[TSP] touch dev == NULL ?\r\n");
		goto fail_time_out_init;
	}	
	down(&touch_dev->work_proceedure_lock);	
	if(touch_dev->work_proceedure != TS_NO_WORK)
	{
		printk(KERN_INFO "[TSP] other process occupied (%d)\r\n", touch_dev->work_proceedure);
		up(&touch_dev->work_proceedure_lock);
		return;	
	}
	touch_dev->work_proceedure = TS_ESD_TIMER_WORK;	

	disable_irq(touch_dev->irq);
	printk(KERN_INFO "[TSP] error. timeout occured. maybe ts device dead. so reset & reinit.\r\n");
	mdelay(CHIP_POWER_OFF_DELAY);
//	ts_power_control(touch_dev, RESET_LOW); //reset pin low
//	ts_power_control (touch_dev, POWER_OFF);	//power off
#ifdef CONFIG_TREBON_HW01
//PSJ	touch_io_ctrl_regulator(0);	//SEC_DRIVER
#endif
//PSJ	touch_ctrl_regulator(0);	//SEC_DRIVER
	touch_power_control(0); //PSJ
	mdelay(1000);		
//	ts_power_control (touch_dev, POWER_ON);	//power on	
#ifdef CONFIG_TREBON_HW01	
//PSJ	touch_io_ctrl_regulator(1);	//SEC_DRIVER
#endif
//PSJ	touch_ctrl_regulator(1);	//SEC_DRIVER
	touch_power_control(1); //PSJ
//ts_power_control(touch_dev, RESET_HIGH);	//reset pin high
	mdelay(CHIP_ON_DELAY);
	zinitix_debug_msg("clear all reported points\r\n");
	zinitix_clear_report_data(touch_dev);
	if(ts_mini_init_touch(touch_dev) == false)
		goto fail_time_out_init;

	touch_dev->work_proceedure = TS_NO_WORK;
	enable_irq(touch_dev->irq);
	up(&touch_dev->work_proceedure_lock);
	printk(KERN_INFO "[TSP] tmr queue work ----\r\n");
	return;
fail_time_out_init:
	printk(KERN_INFO "[TSP] tmr work : restart error\r\n");	
	ts_esd_timer_start(ZINITIX_CHECK_ESD_TIMER, touch_dev);
	touch_dev->work_proceedure = TS_NO_WORK;
	enable_irq(touch_dev->irq);
	up(&touch_dev->work_proceedure_lock);
}

static void ts_esd_timer_start(u16 sec, zinitix_touch_dev* touch_dev)
{	
	zinitix_debug_msg(KERN_INFO "ts_esd_timer_start\r\n");	

	if(touch_dev->p_esd_timeout_tmr != NULL)	del_timer(touch_dev->p_esd_timeout_tmr);
	touch_dev->p_esd_timeout_tmr = NULL;

	init_timer(&(touch_dev->esd_timeout_tmr));
	touch_dev->esd_timeout_tmr.data = (unsigned long)(touch_dev);
	touch_dev->esd_timeout_tmr.function = ts_esd_timeout_handler;			
	touch_dev->esd_timeout_tmr.expires = jiffies + HZ*sec;	
	touch_dev->p_esd_timeout_tmr = &touch_dev->esd_timeout_tmr;
	add_timer(&touch_dev->esd_timeout_tmr);
}

static void ts_esd_timer_stop(zinitix_touch_dev* touch_dev)
{
	if(touch_dev->p_esd_timeout_tmr) del_timer(touch_dev->p_esd_timeout_tmr);
	touch_dev->p_esd_timeout_tmr = NULL;
}

//static void ts_esd_timer_modify(u16 sec, zinitix_touch_dev* touch_dev)
//{
//	mod_timer(&touch_dev->esd_timeout_tmr, jiffies + (HZ*sec));
//}

static void ts_esd_timer_init(zinitix_touch_dev* touch_dev)
{
	init_timer(&(touch_dev->esd_timeout_tmr));
	touch_dev->esd_timeout_tmr.data = (unsigned long)(touch_dev);
	touch_dev->esd_timeout_tmr.function = ts_esd_timeout_handler;		
	touch_dev->p_esd_timeout_tmr=NULL;
}

static void ts_esd_timeout_handler(unsigned long data)
{
	zinitix_touch_dev* touch_dev = (zinitix_touch_dev*)data;
	touch_dev->p_esd_timeout_tmr=NULL;		
	queue_work(zinitix_tmr_workqueue, &touch_dev->tmr_work);
}
#endif


bool ts_check_need_upgrade(u16 curVersion, u16 curRegVersion)
{
	u16	newVersion, phoneVersion,swVersion;

	newVersion = (u16) (m_firmware_data[0] | (m_firmware_data[1]<<8));
	if (ts_read_data(global_touch_dev->client, ZINITIX_DATA_VERSION_REG, (u8*)&phoneVersion, 2)<0);
	swVersion = (u16) (m_firmware_data[FIRMWARE_VERSION_POS+2]  | (m_firmware_data[FIRMWARE_VERSION_POS+3]<<8));

	//	printk(KERN_INFO "[TSP] cur Version = 0x%x, new Version = 0x%x\n", curVersion, newVersion);
	printk(KERN_INFO "[TSP] phone Version = %d, sw Version = %d\n", phoneVersion, swVersion);

	//	if(phoneVersion != swVersion) return true;
	//	else return false;
	if(phoneVersion < swVersion) return true;
	else return false;

	//	if(curVersion < newVersion)		return true;
	//	else if(curVersion > newVersion)	return false;

#if BT4x2_Series
	if(m_firmware_data[0x3FFE] == 0xff && m_firmware_data[0x3FFF] == 0xff)	return false;
	newVersion = (u16) (m_firmware_data[0x3FFE] | (m_firmware_data[0x3FFF]<<8));	

#endif	

#if BT4x3_Above_Series
	if(m_firmware_data[FIRMWARE_VERSION_POS+2] == 0xff && m_firmware_data[FIRMWARE_VERSION_POS+3] == 0xff)	return false;
	newVersion = (u16) (m_firmware_data[FIRMWARE_VERSION_POS+2] | (m_firmware_data[FIRMWARE_VERSION_POS+3]<<8));	// register data version
#endif	


	if(curRegVersion < newVersion)	return true;

	return false;
}


#define	TC_PAGE_SZ		64
#define	TC_SECTOR_SZ		8

u8 ts_upgrade_firmware(zinitix_touch_dev* touch_dev, const u8 *firmware_data, u32 size)
{
	u16 flash_addr;
	u8  * verify_data;
	int	retry_cnt = 0;
#if	(TOUCH_USING_ISP_METHOD==0 && BT4x2_Series == 1)		
	u32 i;
#endif	
	u8	i2c_buffer[TC_PAGE_SZ+2];



	verify_data = (u8*)kzalloc(size, GFP_KERNEL);
	if(verify_data == NULL)
	{
		printk(KERN_ERR "[TSP] cannot alloc verify buffer\n");
		return false;
	}

#if	(TOUCH_USING_ISP_METHOD==0 && BT4x2_Series == 1)
	do{
		printk(KERN_INFO "[TSP] reset command\n");
		if (ts_write_cmd(touch_dev->client, ZINITIX_SWRESET_CMD)!=I2C_SUCCESS)
		{
			printk(KERN_INFO "[TSP] failed to reset\n");
			goto fail_upgrade;
		}

#if USE_HW_CALIBRATION
		printk(KERN_INFO "[TSP] Erase Flash\n");
		if (ts_write_reg(touch_dev->client, ZINITIX_ERASE_FLASH, 0xaaaa)!=I2C_SUCCESS)
		{
			printk(KERN_INFO "[TSP] failed to erase flash\n");
			goto fail_upgrade;
		}		

		mdelay(500);
#else
		if (ts_write_reg(touch_dev->client, ZINITIX_TOUCH_MODE, 0x06)!=I2C_SUCCESS)
		{
			printk(KERN_INFO "[TSP] failed to erase flash\n");
			goto fail_upgrade;
		}		
		ts_write_cmd(touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
		mdelay(10);
		ts_write_cmd(touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
		mdelay(10);
#endif	

		printk(KERN_INFO "[TSP] writing firmware data\n");			

		for(flash_addr= 0; flash_addr< size; )
		{

			for(i=0; i< TC_PAGE_SZ/TC_SECTOR_SZ; i++)
			{
				zinitix_debug_msg("firmware write : addr = %04x, len = %d\n", flash_addr, TC_SECTOR_SZ);
				if(ts_write_data(touch_dev->client,ZINITIX_WRITE_FLASH, &firmware_data[flash_addr],TC_SECTOR_SZ)<0)
				{
					printk(KERN_INFO"[TSP] error : write zinitix tc firmare\n");
					goto fail_upgrade;
				}		
				flash_addr+= TC_SECTOR_SZ;	
				udelay(100);
			}
			mdelay(20);	
#if !USE_HW_CALIBRATION
			if(flash_addr >= CALIBRATION_AREA)	break;
#endif			
		}

		printk(KERN_INFO "[TSP] read firmware data\n");					
		for(flash_addr= 0; flash_addr< size; )
		{

			for(i=0; i< TC_PAGE_SZ/TC_SECTOR_SZ; i++)
			{
				zinitix_debug_msg( "firmware read : addr = %04x, len = %d\n", flash_addr, TC_SECTOR_SZ);
				if(ts_read_firmware_data(touch_dev->client,ZINITIX_READ_FLASH,&verify_data[flash_addr],TC_SECTOR_SZ)<0)
				{
					printk(KERN_INFO "[TSP] error : read zinitix tc firmare\n");
					goto fail_upgrade;
				}		
				flash_addr+= TC_SECTOR_SZ;			
			}
#if !USE_HW_CALIBRATION
			if(flash_addr >= CALIBRATION_AREA)
			{
				memcpy((u8*)&verify_data[CALIBRATION_AREA], (u8*)&firmware_data[CALIBRATION_AREA], size-CALIBRATION_AREA);
				break;				
			}
#endif						
		}
		// verify
		printk(KERN_INFO "[TSP] verify firmware data\n");
		if(memcmp((u8*)&firmware_data[0], (u8*)&verify_data[0], size) == 0)
		{
			printk(KERN_INFO "[TSP] upgrade finished\n");
			kfree(verify_data);		
	//		ts_power_control(touch_dev, RESET_LOW); //reset pin low
	//			ts_power_control(touch_dev, POWER_OFF); //power off
#ifdef CONFIG_TREBON_HW01
//PSJ			touch_io_ctrl_regulator(0);	//SEC_DRIVER
#endif
//PSJ			touch_ctrl_regulator(0);	//SEC_DRIVER
	touch_power_control(0); //PSJ
			mdelay(1000);
	//		ts_power_control(touch_dev, POWER_ON); //power on
#ifdef CONFIG_TREBON_HW01
//PSJ			touch_io_ctrl_regulator(1);	//SEC_DRIVER
#endif
//PSJ			touch_ctrl_regulator(1);	//SEC_DRIVER
	touch_power_control(1); //PSJ
	//		ts_power_control(touch_dev, RESET_HIGH);	//reset pin high	
			mdelay(CHIP_ON_DELAY);
			return true;
		}
		printk(KERN_INFO "[TSP] upgrade fail : so retry... (%d)\n", ++retry_cnt);

		if(retry_cnt >= ZINITIX_INIT_RETRY_CNT)
			goto fail_upgrade;

	}while(1);


#elif	(TOUCH_USING_ISP_METHOD==1)	// isp


	if(m_isp_client == NULL)
	{
		printk(KERN_ERR "[TSP] i2c client for isp is not register \r\n");
		return false;
	}

retry_isp_firmware_upgrade:

#if BT4x2_Series
	//must be reset pin low
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++
	ts_power_control(touch_dev, RESET_LOW);	//reset pin low
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++	
	mdelay(100);	
#endif	

#if BT4x3_Above_Series
//	ts_power_control(touch_dev, RESET_LOW); //reset pin low
//	ts_power_control(touch_dev, POWER_OFF); //power off
#ifdef CONFIG_TREBON_HW01
//PSJ	touch_io_ctrl_regulator(0);	//SEC_DRIVER
#endif
//PSJ	touch_ctrl_regulator(0);	//SEC_DRIVER
	touch_power_control(0); //PSJ
	mdelay(1000);
//	ts_power_control(touch_dev, POWER_ON); //power on
#ifdef CONFIG_TREBON_HW01
//PSJ	touch_io_ctrl_regulator(1);	//SEC_DRIVER
#endif
//PSJ	touch_ctrl_regulator(1);	//SEC_DRIVER
	touch_power_control(1); //PSJ
//	ts_power_control(touch_dev, RESET_HIGH);	//reset pin high
	udelay(1000);		// under 1ms
#endif
	printk(KERN_INFO "[TSP] write firmware data\n");

	for(flash_addr= 0; flash_addr< size; flash_addr+=TC_PAGE_SZ)
	{

#if !USE_HW_CALIBRATION
		if(flash_addr >= CALIBRATION_AREA*2)			break;
#endif								
//		printk(KERN_INFO "[TSP] .");
//		zinitix_debug_msg("firmware write : addr = %04x, len = %d\n", flash_addr, TC_PAGE_SZ);

		i2c_buffer[0] = (flash_addr>>8)&0xff;	//addr_h
		i2c_buffer[1] = (flash_addr)&0xff;	//addr_l
		memcpy(&i2c_buffer[2], &firmware_data[flash_addr], TC_PAGE_SZ);
//		printk( "[TSP] flash_addr = %x i2c_buffer = %x : %x\n",flash_addr, i2c_buffer[0],i2c_buffer[1]);

		if(ts_write_firmware_data(m_isp_client, i2c_buffer, TC_PAGE_SZ+2)<0)	
		{
			printk(KERN_INFO"[TSP] error : write zinitix tc firmare\n");
			goto fail_upgrade;
		}
		mdelay(20);	
	}
	mdelay(CHIP_POWER_OFF_AF_FZ_DELAY);

	printk(KERN_INFO "\r\n[TSP] read firmware data\n");						

#if BT4x3_Above_Series
	flash_addr = 0;
	i2c_buffer[0] = (flash_addr>>8)&0xff;	//addr_h
	i2c_buffer[1] = (flash_addr)&0xff;	//addr_l

#if !USE_HW_CALIBRATION
	size = CALIBRATION_AREA*2;
#endif	

	if(ts_read_firmware_data(m_isp_client, i2c_buffer,&verify_data[flash_addr], size)<0)
	{
		printk(KERN_INFO "[TSP] error : read zinitix tc firmare: addr = %04x, len = %d\n", flash_addr, size);
		goto fail_upgrade;
	}	
	if(memcmp((u8*)&firmware_data[flash_addr], (u8*)&verify_data[flash_addr], size) != 0)			
	{
		printk(KERN_INFO "[TSP] error : verify error : addr = %04x, len = %d\n", flash_addr, size);
		goto fail_upgrade;
	}

#else	//bt4x2

	for(flash_addr= 0; flash_addr< size; flash_addr+=TC_PAGE_SZ)
	{
#if !USE_HW_CALIBRATION
		if(flash_addr >= CALIBRATION_AREA*2)			break;
#endif	
		i2c_buffer[0] = (flash_addr>>8)&0xff;	//addr_h
		i2c_buffer[1] = (flash_addr)&0xff;	//addr_l
		zinitix_debug_msg("firmware read : addr = %04x, len = %d\n", flash_addr, TC_PAGE_SZ);
		if(ts_read_firmware_data(m_isp_client, i2c_buffer,&verify_data[flash_addr],TC_PAGE_SZ)<0)
		{
			printk(KERN_INFO "[TSP] error : read zinitix tc firmare: addr = %04x, len = %d\n", flash_addr, TC_PAGE_SZ);
			goto fail_upgrade;
		}				
		if(memcmp((u8*)&firmware_data[flash_addr], (u8*)&verify_data[flash_addr], TC_PAGE_SZ) != 0)			
		{
			printk(KERN_INFO "[TSP] error : verify error : addr = %04x, len = %d\n", flash_addr, TC_PAGE_SZ);
			goto fail_upgrade;
		}								
		udelay(500);

	}
#endif	
	// verify
	mdelay(CHIP_POWER_OFF_AF_FZ_DELAY);
	//ts_power_control(touch_dev, RESET_LOW); //reset pin low
	//	ts_power_control(touch_dev, POWER_OFF); //power off
#ifdef CONFIG_TREBON_HW01
//PSJ	touch_io_ctrl_regulator(0);	//SEC_DRIVER
#endif
//PSJ	touch_ctrl_regulator(0);	//SEC_DRIVER
	touch_power_control(0); //PSJ
	mdelay(1000);
	//	ts_power_control(touch_dev, POWER_ON); //power on
#ifdef CONFIG_TREBON_HW01
//PSJ	touch_io_ctrl_regulator(1);	//SEC_DRIVER
#endif
//PSJ	touch_ctrl_regulator(1);	//SEC_DRIVER
	touch_power_control(1); //PSJ
	//ts_power_control(touch_dev, RESET_HIGH);	//reset pin high
	mdelay(CHIP_ON_AF_FZ_DELAY);
	printk(KERN_INFO "[TSP] upgrade finished\n");
	kfree(verify_data);
	return true;

#endif

fail_upgrade:	
	mdelay(CHIP_POWER_OFF_AF_FZ_DELAY);
	//ts_power_control(touch_dev, RESET_LOW); //reset pin low
	//	ts_power_control(touch_dev, POWER_OFF); //power off
#ifdef CONFIG_TREBON_HW01
//PSJ	touch_io_ctrl_regulator(0);	//SEC_DRIVER
#endif
//PSJ	touch_ctrl_regulator(0);	//SEC_DRIVER
	touch_power_control(0); //PSJ
	mdelay(1000);
	//	ts_power_control(touch_dev, POWER_ON); //power on
#ifdef CONFIG_TREBON_HW01
//PSJ	touch_io_ctrl_regulator(1);	//SEC_DRIVER
#endif
//PSJ	touch_ctrl_regulator(1);	//SEC_DRIVER
	touch_power_control(1); //PSJ
	//ts_power_control(touch_dev, RESET_HIGH);	//reset pin high
	mdelay(CHIP_ON_AF_FZ_DELAY);

	printk(KERN_INFO "[TSP] upgrade fail : so retry... (%d)\n", ++retry_cnt);		
	if(retry_cnt >= ZINITIX_INIT_RETRY_CNT)		goto retry_isp_firmware_upgrade;	

	if(verify_data != NULL)		kfree(verify_data);
	printk(KERN_INFO "[TSP] upgrade fail..\n");
	return false;

}

int gv_initCheck = 0;

bool ts_init_touch(zinitix_touch_dev* touch_dev)
{
	u16	reg_val;
	int	i;
	u16 SetMaxX = SYSTEM_MAX_X_RESOLUTION; //Max Position range from 0x0002 to 0x1fff
	u16 SetMaxY = SYSTEM_MAX_Y_RESOLUTION; //Max Position range from 0x0002 to 0x1fff  
	u16 chip_revision;
	u16 chip_firmware_version;
	u16	chip_reg_data_version;
	u16	chip_eeprom_info;
	
	int	retry_cnt = 0;


	if(touch_dev == NULL)
	{
		printk(KERN_ERR "[TSP] error touch_dev == null?\r\n");
		return false;
	}


retry_init:

	zinitix_debug_msg("disable interrupt\r\n");

	for(i=0; i<ZINITIX_INIT_RETRY_CNT; i++)
	{
		if (ts_write_cmd(touch_dev->client, ZINITIX_SWRESET_CMD)==I2C_SUCCESS)	break;
		mdelay(10);
	}

	if(i==ZINITIX_INIT_RETRY_CNT)
	{
		printk(KERN_INFO "[TSP] fail to write interrupt register\r\n");
		goto fail_init;
	}

#if USING_CHIP_SETTING
	if(ts_read_data(touch_dev->client, ZINITIX_INT_ENABLE_FLAG, (u8*)&touch_dev->cap_info.chip_int_mask, 2) < 0)	goto fail_init;
	printk(KERN_INFO "[TSP] zinitix touch interrupt mask = %04x\r\n", touch_dev->cap_info.chip_int_mask);
	if(touch_dev->cap_info.chip_int_mask == 0 || touch_dev->cap_info.chip_int_mask == 0xffff)	goto fail_init;

#else	
	touch_dev->cap_info.button_num = SUPPORTED_BUTTON_NUM;	

	reg_val = 0;
	zinitix_bit_set(reg_val, BIT_PT_CNT_CHANGE);			
	zinitix_bit_set(reg_val, BIT_DOWN);
	zinitix_bit_set(reg_val, BIT_MOVE);
	zinitix_bit_set(reg_val, BIT_UP);		
	if(touch_dev->cap_info.button_num > 0)		zinitix_bit_set(reg_val, BIT_ICON_EVENT);
	printk("[TSP] reg_val = %x , button_num = %d",reg_val, touch_dev->cap_info.button_num);
	touch_dev->cap_info.chip_int_mask = reg_val;		
#endif

	if (ts_write_reg(touch_dev->client, ZINITIX_INT_ENABLE_FLAG, 0x0) !=I2C_SUCCESS)	goto fail_init;

	zinitix_debug_msg("send reset command\r\n");
	if (ts_write_cmd(touch_dev->client, ZINITIX_SWRESET_CMD)!=I2C_SUCCESS)	goto fail_init;

	/* get chip revision id */
	if (ts_read_data(touch_dev->client, ZINITIX_CHIP_REVISION, (u8*)&chip_revision, 2)<0)
	{
		printk(KERN_INFO "[TSP] fail to read chip revision\r\n");
		goto fail_init;
	}	
	printk(KERN_INFO "[TSP] zinitix touch chip revision id = %x\r\n", chip_revision);

	touch_dev->cap_info.chip_fw_size = 16*1024;

#if BT4x3_Above_Series	
	touch_dev->cap_info.chip_fw_size = 32*1024;
#endif

#if	USE_TEST_RAW_TH_DATA_MODE
	if (ts_read_data(touch_dev->client, ZINITIX_TOTAL_NUMBER_OF_X, (u8*)&touch_dev->cap_info.x_node_num, 2)<0) goto fail_init;
	printk(KERN_INFO "[TSP] zinitix touch chip x node num = %d\r\n", touch_dev->cap_info.x_node_num);
	if (ts_read_data(touch_dev->client, ZINITIX_TOTAL_NUMBER_OF_Y, (u8*)&touch_dev->cap_info.y_node_num, 2)<0) goto fail_init;
	printk(KERN_INFO "[TSP] zinitix touch chip y node num = %d\r\n", touch_dev->cap_info.y_node_num);

	touch_dev->cap_info.total_node_num = touch_dev->cap_info.x_node_num*touch_dev->cap_info.y_node_num;
	printk(KERN_INFO "[TSP] zinitix touch chip total node num = %d\r\n", touch_dev->cap_info.total_node_num);
#if BT4x3_Above_Series		
	if (ts_read_data(touch_dev->client, ZINITIX_MAX_Y_NUM, (u8*)&touch_dev->cap_info.max_y_node, 2)<0) goto fail_init;
	printk(KERN_INFO "[TSP] zinitix touch chip max y node num = %d\r\n", touch_dev->cap_info.max_y_node);	

	if (ts_read_data(touch_dev->client, ZINITIX_CAL_N_TOTAL_NUM, (u8*)&touch_dev->cap_info.total_cal_n, 2)<0) goto fail_init;
	printk(KERN_INFO "[TSP] zinitix touch chip total cal n data num = %d\r\n", touch_dev->cap_info.total_cal_n);	


#endif	
#endif


	/* get chip firmware version */
	if (ts_read_data(touch_dev->client, ZINITIX_FIRMWARE_VERSION, (u8*)&chip_firmware_version, 2)<0) goto fail_init;
	printk(KERN_INFO "[TSP] zinitix touch chip firmware version = %x\r\n", chip_firmware_version);

#if	0 //TOUCH_ONESHOT_UPGRADE PSJ
	chip_reg_data_version = 0xffff;

	if (ts_read_data(touch_dev->client, ZINITIX_DATA_VERSION_REG, (u8*)&chip_reg_data_version, 2)<0) goto fail_init;
	zinitix_debug_msg("touch reg data version = %d\r\n", chip_reg_data_version);

force_upgrade:	
	printk(KERN_INFO "work_proceedure = %d\n", touch_dev->work_proceedure);

	if((touch_dev->work_proceedure != TS_IN_RESUME) && (touch_dev->work_proceedure !=	TS_IN_EALRY_SUSPEND))
	{
		if(IsfwUpdate || (ts_check_need_upgrade(chip_firmware_version, chip_reg_data_version)==true) || (retry_cnt>=ZINITIX_INIT_RETRY_CNT))
		{
			printk(KERN_INFO "[TSP] start upgrade firmware\n");
			IsfwUpdate = FWUP_NOW;
			ts_upgrade_firmware(touch_dev, &m_firmware_data[2], touch_dev->cap_info.chip_fw_size);


			/* get chip revision id */
			if (ts_read_data(touch_dev->client, ZINITIX_CHIP_REVISION, (u8*)&chip_revision, 2)<0)
			{
				printk(KERN_INFO "[TSP] fail to read chip revision\r\n");
				goto fail_init;
			}		
			printk(KERN_INFO "[TSP] zinitix touch chip revision id = %x\r\n", chip_revision);

			/* get chip firmware version */
			if (ts_read_data(touch_dev->client, ZINITIX_FIRMWARE_VERSION, (u8*)&chip_firmware_version, 2)<0) goto fail_init;		
			printk(KERN_INFO "[TSP] zinitix touch chip renewed firmware version = %x\r\n", chip_firmware_version);

		}
	}
#endif

	if (ts_read_data(touch_dev->client, ZINITIX_DATA_VERSION_REG, (u8*)&chip_reg_data_version, 2)<0) goto fail_init;
	zinitix_debug_msg("touch reg data version = %d\r\n", chip_reg_data_version);

#if	0 //TOUCH_ONESHOT_UPGRADE
	if(chip_reg_data_version < m_reg_data[ZINITIX_DATA_VERSION_REG].reg_val)
	{
		zinitix_debug_msg("write new reg data( %d < %d)\r\n", chip_reg_data_version, m_reg_data[ZINITIX_DATA_VERSION_REG].reg_val);
		for(i=0; i < MAX_REG_COUNT; i++)
		{
			if(m_reg_data[i].valid == 1)
			{
				if(ts_write_reg(touch_dev->client, (u16)i, (u16)(m_reg_data[i].reg_val))!=I2C_SUCCESS) goto fail_init;
				if(i == ZINITIX_TOTAL_NUMBER_OF_X || i == ZINITIX_TOTAL_NUMBER_OF_Y)	mdelay(50);	//for clear hw calibration bit
				if(ts_read_data(touch_dev->client, (u16)i, (u8*)&stmp, 2)<0) goto fail_init;
				if(memcmp((char*)&m_reg_data[i].reg_val, (char*)&stmp, 2)!=0)		//if(m_reg_data[i].reg_val != stmp)
				printk(KERN_WARNING "[TSP] register data is different. (addr = 0x%02X , %d != %d)\r\n", i, m_reg_data[i].reg_val, stmp);					
			}
		}
		zinitix_debug_msg("done new reg data( %d < %d)\r\n", chip_reg_data_version, m_reg_data[ZINITIX_DATA_VERSION_REG].reg_val);		
		if (ts_write_cmd(touch_dev->client, ZINITIX_SAVE_STATUS_CMD)!=I2C_SUCCESS) goto fail_init;			
		mdelay(1000);	// for fusing eeprom
	}

#endif	

	if (ts_read_data(touch_dev->client, ZINITIX_EEPROM_INFO_REG, (u8*)&chip_eeprom_info, 2)<0) goto fail_init;
	zinitix_debug_msg("touch eeprom info = 0x%04X\r\n", chip_eeprom_info);

#if	USE_HW_CALIBRATION	
	if(zinitix_bit_test(chip_eeprom_info, 0))		// hw calibration bit
	{
		if(touch_dev->cap_info.chip_int_mask !=0)
		{
			if (ts_write_reg(touch_dev->client, ZINITIX_INT_ENABLE_FLAG, touch_dev->cap_info.chip_int_mask)!=I2C_SUCCESS)	goto fail_init;
		}
	// h/w calibration
		if (ts_write_reg(touch_dev->client, ZINITIX_TOUCH_MODE, 0x07)!=I2C_SUCCESS) goto fail_init;
		if (ts_write_cmd(touch_dev->client, ZINITIX_CALIBRATE_CMD)!=I2C_SUCCESS)	goto fail_init;
		if (ts_write_cmd(touch_dev->client, ZINITIX_SWRESET_CMD)!=I2C_SUCCESS)	goto fail_init;
		mdelay(1); 		
		ts_write_cmd(touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
		mdelay(100); 		
		ts_write_cmd(touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
		mdelay(100); 		
		ts_write_cmd(touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
		// wait for h/w calibration
		do{
			mdelay(1000); 
			if (ts_read_data(touch_dev->client, ZINITIX_EEPROM_INFO_REG, (u8*)&chip_eeprom_info, 2)<0) goto fail_init;
			zinitix_debug_msg("touch eeprom info = 0x%04X\r\n", chip_eeprom_info);
			if(!zinitix_bit_test(chip_eeprom_info, 0))	break;
		}while(1);

		if (ts_write_reg(touch_dev->client, ZINITIX_TOUCH_MODE, TOUCH_MODE)!=I2C_SUCCESS) goto fail_init;
		printk("[TSP] touch_dev->cap_info.chip_int_mask = %x",touch_dev->cap_info.chip_int_mask);
		mdelay(10);
		if (ts_write_cmd(touch_dev->client, ZINITIX_SWRESET_CMD)!=I2C_SUCCESS) goto fail_init;
#if BT4x3_Above_Series		
		mdelay(10);
		if (ts_write_cmd(touch_dev->client, ZINITIX_SAVE_CALIBRATION_CMD)!=I2C_SUCCESS) goto fail_init;
		mdelay(500);
#endif		

	//<----------------------------------------------------------
#if BT4x2_Series				
		if (ts_write_cmd(touch_dev->client, ZINITIX_SAVE_STATUS_CMD)!=I2C_SUCCESS) goto fail_init;
		mdelay(1000);	// for fusing eeprom
		if (ts_write_cmd(touch_dev->client, ZINITIX_SWRESET_CMD)!=I2C_SUCCESS) goto fail_init;				
#endif		
		//disable chip interrupt
		if (ts_write_reg(touch_dev->client, ZINITIX_INT_ENABLE_FLAG, 0)!=I2C_SUCCESS)	goto fail_init;

	}
#endif

	touch_dev->cap_info.chip_revision = (u16)chip_revision;
	touch_dev->cap_info.chip_firmware_version = (u16)chip_firmware_version;
	touch_dev->cap_info.chip_reg_data_version = (u16)chip_reg_data_version;


	/* initialize */	
	if (ts_write_reg(touch_dev->client, ZINITIX_X_RESOLUTION, (u16)(SetMaxX))!=I2C_SUCCESS) goto fail_init;
	if (ts_write_reg(touch_dev->client, ZINITIX_Y_RESOLUTION, (u16)(SetMaxY))!=I2C_SUCCESS) goto fail_init;

	if (ts_read_data(touch_dev->client, ZINITIX_X_RESOLUTION, (u8*)&touch_dev->cap_info.x_resolution, 2)<0) goto fail_init;
	zinitix_debug_msg("touch max x = %d\r\n", touch_dev->cap_info.x_resolution);
	if (ts_read_data(touch_dev->client, ZINITIX_Y_RESOLUTION, (u8*)&touch_dev->cap_info.y_resolution, 2)<0) goto fail_init;
	zinitix_debug_msg("touch max y = %d\r\n", touch_dev->cap_info.y_resolution);    

	touch_dev->cap_info.MinX = (u32)0;
	touch_dev->cap_info.MinY = (u32)0;
	touch_dev->cap_info.MaxX = (u32)touch_dev->cap_info.x_resolution;
	touch_dev->cap_info.MaxY = (u32)touch_dev->cap_info.y_resolution;

#if USING_CHIP_SETTING
	if (ts_read_data(touch_dev->client, ZINITIX_BUTTON_SUPPORTED_NUM, (u8*)&touch_dev->cap_info.button_num, 2)<0) goto fail_init;		
	zinitix_debug_msg("supported button num = %d\r\n", touch_dev->cap_info.button_num);
	if (ts_read_data(touch_dev->client, ZINITIX_SUPPORTED_FINGER_NUM, (u8*)&touch_dev->cap_info.multi_fingers, 2)<0) goto fail_init;
	zinitix_debug_msg("supported finger num = %d\r\n", touch_dev->cap_info.multi_fingers);		
#else	// driver setting

	if (ts_write_reg(touch_dev->client, ZINITIX_SUPPORTED_FINGER_NUM, (u16)MAX_SUPPORTED_FINGER_NUM)!=I2C_SUCCESS) goto fail_init;
	touch_dev->cap_info.multi_fingers = REAL_SUPPORTED_FINGER_NUM;
#endif

	zinitix_debug_msg("max supported finger num = %d, real supported finger num = %d\r\n", touch_dev->cap_info.multi_fingers, REAL_SUPPORTED_FINGER_NUM); 	

	touch_dev->cap_info.gesture_support = 0;

	zinitix_debug_msg("set other configuration\r\n");

#if	USE_TEST_RAW_TH_DATA_MODE
	if(touch_dev->raw_mode_flag != TOUCH_NORMAL_MODE)	// test mode
	{
		if (ts_write_reg(touch_dev->client, ZINITIX_TOUCH_MODE, touch_dev->raw_mode_flag)!=I2C_SUCCESS)	goto fail_init; 	
	}
	else
#endif
	{
		reg_val = TOUCH_MODE;
		if (ts_write_reg(touch_dev->client, ZINITIX_TOUCH_MODE, reg_val)!=I2C_SUCCESS)	goto fail_init;		
	}
	// soft calibration
	if (ts_write_cmd(touch_dev->client, ZINITIX_CALIBRATE_CMD)!=I2C_SUCCESS)		goto fail_init;
	if (ts_write_reg(touch_dev->client, ZINITIX_INT_ENABLE_FLAG, touch_dev->cap_info.chip_int_mask)!=I2C_SUCCESS)	goto fail_init;

	//---------------------------------------------------------------------
	// read garbage data
	for(i=0; i<10; i++)
	{
		ts_write_cmd(touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
		udelay(10);
	}

#if	USE_TEST_RAW_TH_DATA_MODE
	if(touch_dev->raw_mode_flag != TOUCH_NORMAL_MODE)	// test mode
	{
		if (ts_write_reg(touch_dev->client, ZINITIX_PERIODICAL_INTERRUPT_INTERVAL, ZINITIX_SCAN_RATE_HZ*ZINITIX_RAW_DATA_ESD_TIMER_INTERVAL)!=I2C_SUCCESS)
		printk(KERN_INFO "[zinitix_touch] Fail to set ZINITIX_RAW_DATA_ESD_TIMER_INTERVAL.\r\n");
	}
	else
#endif
	{
#if	ZINITIX_ESD_TIMER_INTERVAL	
		if (ts_write_reg(touch_dev->client, ZINITIX_PERIODICAL_INTERRUPT_INTERVAL, ZINITIX_SCAN_RATE_HZ*ZINITIX_ESD_TIMER_INTERVAL)!=I2C_SUCCESS)	goto fail_init;
		if (ts_read_data(touch_dev->client, ZINITIX_PERIODICAL_INTERRUPT_INTERVAL, (u8*)&reg_val, 2)<0) goto fail_init;
		zinitix_debug_msg("esd timer register = %d\r\n", reg_val); 
#endif
	}

	zinitix_debug_msg("successfully initialized\r\n");
	return true;

fail_init:

	if(retry_cnt++ <= ZINITIX_INIT_RETRY_CNT)
	{
		mdelay(CHIP_POWER_OFF_DELAY);
//		ts_power_control(touch_dev, RESET_LOW); //reset pin low
//		ts_power_control(touch_dev, POWER_OFF); //power off
#ifdef CONFIG_TREBON_HW01
//PSJ		touch_io_ctrl_regulator(0);	//SEC_DRIVER
#endif
//PSJ		touch_ctrl_regulator(0);	//SEC_DRIVER
	touch_power_control(0); //PSJ
		mdelay(1000);
//		ts_power_control(touch_dev, POWER_ON); //power on
#ifdef CONFIG_TREBON_HW01
//PSJ		touch_io_ctrl_regulator(1);	//SEC_DRIVER
#endif
//PSJ		touch_ctrl_regulator(1);	//SEC_DRIVER
	touch_power_control(1); //PSJ
//		ts_power_control(touch_dev, RESET_HIGH);	//reset pin high
		mdelay(CHIP_ON_DELAY);
		zinitix_debug_msg("retry to initiallize(retry cnt = %d)\r\n", retry_cnt);
		goto	retry_init;
	}
	else
	{
		touch_dev->cap_info.chip_fw_size = 16*1024;
#if BT4x3_Above_Series	
		touch_dev->cap_info.chip_fw_size = 32*1024;
#endif
		mdelay(CHIP_POWER_OFF_DELAY);
//		ts_power_control(touch_dev, RESET_LOW); //reset pin low
//		ts_power_control(touch_dev, POWER_OFF); //power off
#ifdef CONFIG_TREBON_HW01
//PSJ		touch_io_ctrl_regulator(0);	//SEC_DRIVER
#endif
//PSJ		touch_ctrl_regulator(0);	//SEC_DRIVER
	touch_power_control(0); //PSJ
		mdelay(1000);
//		ts_power_control(touch_dev, POWER_ON); //power on
#ifdef CONFIG_TREBON_HW01
//PSJ		touch_io_ctrl_regulator(1);	//SEC_DRIVER
#endif
//PSJ		touch_ctrl_regulator(1);	//SEC_DRIVER
	touch_power_control(1); //PSJ
//		ts_power_control(touch_dev, RESET_HIGH);	//reset pin high
		mdelay(CHIP_ON_DELAY);
		zinitix_debug_msg("retry to initiallize(retry cnt = %d)\r\n", retry_cnt);
#if 0 //TOUCH_FORCE_UPGRADE PSJ
		if(retry_cnt > 15) 
		{
			printk("[zinitix touch] failed to force upgrade\r\n");	 
			return false;
		}
		goto force_upgrade;
#endif
	}

	printk("[zinitix touch] failed to initiallize\r\n");
	gv_initCheck = 1;
	return false;

}


#if (TOUCH_MODE == 1)
static void	zinitix_report_data(zinitix_touch_dev *touch_dev, int id)
{
	int i;
	u32 x, y;
	u32 tmp;

	if(id >= touch_dev->cap_info.multi_fingers || id < 0)
	{
		return;
	}

	x = touch_dev->touch_info.coord[id].x;
	y = touch_dev->touch_info.coord[id].y;

	/* transformation from touch to screen orientation */
	if (touch_dev->cap_info.Orientation & TOUCH_V_FLIP)
	{
		y = touch_dev->cap_info.MaxY + touch_dev->cap_info.MinY - y;			               
	}
	if (touch_dev->cap_info.Orientation & TOUCH_H_FLIP)
	{
		x = touch_dev->cap_info.MaxX + touch_dev->cap_info.MinX - x;			               
	}
	if (touch_dev->cap_info.Orientation & TOUCH_XY_SWAP)
	{					
		zinitix_swap_v(x, y, tmp);
	}
	zinitix_debug_msg("x = %d, y = %d, w = %d\r\n", x, y, touch_dev->touch_info.coord[id].width);

	touch_dev->reported_touch_info.coord[id].x = x;
	touch_dev->reported_touch_info.coord[id].y = y;				
	touch_dev->reported_touch_info.coord[id].width = touch_dev->touch_info.coord[id].width;
	touch_dev->reported_touch_info.coord[id].sub_status = touch_dev->touch_info.coord[id].sub_status;


	for(i=0; i< touch_dev->cap_info.multi_fingers; i++)
	{
		if(zinitix_bit_test(touch_dev->reported_touch_info.coord[i].sub_status, SUB_BIT_EXIST)
				||zinitix_bit_test(touch_dev->reported_touch_info.coord[i].sub_status, SUB_BIT_DOWN)
				||zinitix_bit_test(touch_dev->reported_touch_info.coord[i].sub_status, SUB_BIT_MOVE))			
		{

			if(touch_dev->reported_touch_info.coord[i].width == 0)	touch_dev->reported_touch_info.coord[i].width = 5;
			input_report_abs(touch_dev->input_dev, ABS_MT_TOUCH_MAJOR, (u32)touch_dev->reported_touch_info.coord[i].width);
			input_report_abs(touch_dev->input_dev, ABS_MT_WIDTH_MAJOR, (u32)touch_dev->reported_touch_info.coord[i].width);					
			input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_X, touch_dev->reported_touch_info.coord[i].x);
			input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_Y, touch_dev->reported_touch_info.coord[i].y);
			input_mt_sync(touch_dev->input_dev);	
		}
		else if(zinitix_bit_test(touch_dev->reported_touch_info.coord[i].sub_status, SUB_BIT_UP))			
		{
			input_report_abs(touch_dev->input_dev, ABS_MT_TOUCH_MAJOR, 0);
			input_report_abs(touch_dev->input_dev, ABS_MT_WIDTH_MAJOR, 0);	
			input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_X, touch_dev->reported_touch_info.coord[i].x);
			input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_Y, touch_dev->reported_touch_info.coord[i].y);
			input_mt_sync(touch_dev->input_dev);	
			touch_dev->reported_touch_info.coord[i].sub_status = 0;
		}
		else
			touch_dev->reported_touch_info.coord[i].sub_status = 0;
	}

	input_sync(touch_dev->input_dev);
}
#endif	// TOUCH_MODE == 1

static void	zinitix_clear_report_data(zinitix_touch_dev *touch_dev)
{
	int i;
	u8	reported = 0;

	for(i=0; i < touch_dev->cap_info.multi_fingers; i++)
	{		
		if(zinitix_bit_test(touch_dev->reported_touch_info.coord[i].sub_status, SUB_BIT_EXIST))
		{				
			input_report_abs(touch_dev->input_dev, ABS_MT_TOUCH_MAJOR, 0);
			input_report_abs(touch_dev->input_dev, ABS_MT_WIDTH_MAJOR, 0);	
			input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_X, touch_dev->reported_touch_info.coord[i].x);
			input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_Y, touch_dev->reported_touch_info.coord[i].y);
			input_mt_sync(touch_dev->input_dev);	
			reported = 1;
		}
		touch_dev->reported_touch_info.coord[i].sub_status = 0;
	}	
	if(reported)	input_sync(touch_dev->input_dev);
}

static irqreturn_t ts_irq_thread(int irq, void *data)
{
	zinitix_touch_dev* touch_dev = (zinitix_touch_dev*)data;
	bool read_coord_continued;
	int i;
	u8 reported = false;
#if (TOUCH_MODE == 0)
	u32 x, y;
	u32 tmp;
#endif

	if(gpio_get_value(touch_dev->int_gpio_num))	// remove pending interrupt
	{
		zinitix_debug_msg("invalid interrupt occured +\r\n");
		return IRQ_HANDLED;
	}

	down(&touch_dev->work_proceedure_lock);
	if(touch_dev->work_proceedure != TS_NO_WORK)
	{
		printk(KERN_INFO "[TSP] invalid interrupt??? (%d)\n", touch_dev->work_proceedure);
		up(&touch_dev->work_proceedure_lock);
		return IRQ_HANDLED;
	}
	touch_dev->work_proceedure = TS_NORMAL_WORK;
	zinitix_debug_msg("zinitix_touch_thread : semaphore signalled\r\n");

#if	ZINITIX_ESD_TIMER_INTERVAL	
	if(touch_dev->use_esd_timer)
	{			
		ts_esd_timer_stop(touch_dev);	
		zinitix_debug_msg("esd timer stop\r\n");
	}
#endif				
	read_coord_continued = true;
	do
	{

		if (ts_read_coord(touch_dev)==false)
		{
			zinitix_debug_msg("couldn't read touch_dev sample\r\n");
			goto continue_read_samples;
		}
#if	USE_TEST_RAW_TH_DATA_MODE	
		if(touch_dev->raw_mode_flag == TOUCH_TEST_RAW_MODE)	goto continue_read_samples;
#endif		


		// invalid : maybe periodical repeated int.
		if(touch_dev->touch_info.status == 0x0)		goto continue_read_samples;
		reported = false;

		if(zinitix_bit_test(touch_dev->touch_info.status, BIT_ICON_EVENT))
		{

			for(i=0; i<touch_dev->cap_info.button_num; i++)
			{
				if(zinitix_bit_test(touch_dev->icon_event_reg, (BIT_O_ICON0_DOWN+i)))
				{
					touch_dev->button[i] = ICON_BUTTON_DOWN;
					input_report_key(touch_dev->input_dev, BUTTON_MAPPING_KEY[i], 1);					
					reported = true;						
					zinitix_debug_msg("button down = %d \r\n", i);
				}
			}

			for(i=0; i<touch_dev->cap_info.button_num; i++)
			{
				if(zinitix_bit_test(touch_dev->icon_event_reg, (BIT_O_ICON0_UP+i)))
				{
					touch_dev->button[i] = ICON_BUTTON_UP;	
					input_report_key(touch_dev->input_dev, BUTTON_MAPPING_KEY[i], 0);					
					reported = true;		
					zinitix_debug_msg("button up = %d \r\n", i);
				}
			}
		}

		// if button press or up event occured...
		if(reported == true)
		{
#ifdef KEY_LED_CONTROL
//			printk(TCH_KERN_DEBUG "[TSP] key_led_on:%d menu_button:%d back_button%d\n", key_led_on, BUTTON_MAPPING_KEY[0], BUTTON_MAPPING_KEY[1]);
//			if(!key_led_on) { 
//				vled_power_onoff(1);
//				mod_timer(&g_led_timer, (jiffies + HZ*2));
//				key_led_on = true;
//			}
#endif	

#if (TOUCH_MODE == 1)
			//input_sync(touch_dev->input_dev);
			for(i=0; i< touch_dev->cap_info.multi_fingers; i++)
			{

				if(zinitix_bit_test(touch_dev->reported_touch_info.coord[i].sub_status, SUB_BIT_EXIST))			
				{
					input_report_abs(touch_dev->input_dev, ABS_MT_TOUCH_MAJOR, 0);
					input_report_abs(touch_dev->input_dev, ABS_MT_WIDTH_MAJOR, 0);	
					input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_X, touch_dev->reported_touch_info.coord[i].x);
					input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_Y, touch_dev->reported_touch_info.coord[i].y);												
					input_mt_sync(touch_dev->input_dev);	
				}
				touch_dev->reported_touch_info.coord[i].sub_status = 0;
			}
			input_sync(touch_dev->input_dev);
			//goto continue_read_samples;
		}
#else				
			zinitix_debug_msg("%s : touch mode = 0\n", __func__);

			for(i=0; i< touch_dev->cap_info.multi_fingers; i++)
			{

				if(zinitix_bit_test(touch_dev->reported_touch_info.coord[i].sub_status, SUB_BIT_EXIST))			
				{

					//input_report_abs(touch_dev->input_dev,ABS_MT_TRACKING_ID,i);
					input_report_abs(touch_dev->input_dev, ABS_MT_TOUCH_MAJOR, 0);
					input_report_abs(touch_dev->input_dev, ABS_MT_WIDTH_MAJOR, 0);	
					input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_X, touch_dev->reported_touch_info.coord[i].x);
					input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_Y, touch_dev->reported_touch_info.coord[i].y);												
					input_mt_sync(touch_dev->input_dev);	
				}
			}
			memset(&touch_dev->reported_touch_info, 0x0, sizeof(_ts_zinitix_point_info));				
			input_sync(touch_dev->input_dev);
			udelay(100);				
			goto continue_read_samples;				
		}


		if (touch_dev->touch_info.finger_cnt > MAX_SUPPORTED_FINGER_NUM)
			touch_dev->touch_info.finger_cnt = MAX_SUPPORTED_FINGER_NUM;

		if(!zinitix_bit_test(touch_dev->touch_info.status, BIT_PT_EXIST))
		{

			for(i=0; i< touch_dev->cap_info.multi_fingers; i++)
			{
				if(zinitix_bit_test(touch_dev->reported_touch_info.coord[i].sub_status, SUB_BIT_EXIST))
				{
                                zinitix_debug_msg("TSP UP UP UP UP \n");
					//input_report_abs(touch_dev->input_dev,ABS_MT_TRACKING_ID,i);
					input_report_abs(touch_dev->input_dev, ABS_MT_TOUCH_MAJOR, 0);
					input_report_abs(touch_dev->input_dev, ABS_MT_WIDTH_MAJOR, 0);	
					input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_X, touch_dev->reported_touch_info.coord[i].x);
					input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_Y, touch_dev->reported_touch_info.coord[i].y);						
					input_mt_sync(touch_dev->input_dev);	
				}
			}				
			memset(&touch_dev->reported_touch_info, 0x0, sizeof(_ts_zinitix_point_info));					

                   input_report_key(touch_dev->input_dev, BTN_TOUCH, 0);  /* SKC */
			input_sync(touch_dev->input_dev);
			goto continue_read_samples;
		}


		for(i=0; i< touch_dev->cap_info.multi_fingers; i++)
		{

			if(zinitix_bit_test(touch_dev->touch_info.coord[i].sub_status, SUB_BIT_DOWN)
					|| zinitix_bit_test(touch_dev->touch_info.coord[i].sub_status, SUB_BIT_MOVE)
					|| zinitix_bit_test(touch_dev->touch_info.coord[i].sub_status, SUB_BIT_EXIST))				
			{
				x = touch_dev->touch_info.coord[i].x;
				y = touch_dev->touch_info.coord[i].y;

				/* transformation from touch to screen orientation */
				if (touch_dev->cap_info.Orientation & TOUCH_V_FLIP)
				{
					y = touch_dev->cap_info.MaxY + touch_dev->cap_info.MinY - y;			               
				}
				if (touch_dev->cap_info.Orientation & TOUCH_H_FLIP)
				{
					x = touch_dev->cap_info.MaxX + touch_dev->cap_info.MinX - x;			               
				}
				if (touch_dev->cap_info.Orientation & TOUCH_XY_SWAP)
				{					
					zinitix_swap_v(x, y, tmp);
				}
				if (x == 0) x = 1;
				if (y == 0) y = 1;
				touch_dev->touch_info.coord[i].x = x;
				touch_dev->touch_info.coord[i].y = y;

				zinitix_debug_msg("finger [%02d] x = %d, y = %d \r\n", i, x, y);
				if(IsdebugMatrix)
					zinitix_debug_msg("[TSP] finger [%02d] x = %d, y = %d \r\n", i, x, y);

				//input_report_abs(touch_dev->input_dev,ABS_MT_TRACKING_ID,i);
				if(touch_dev->touch_info.coord[i].width == 0)	touch_dev->touch_info.coord[i].width = 5;
				input_report_abs(touch_dev->input_dev, ABS_MT_TOUCH_MAJOR, (u32)touch_dev->touch_info.coord[i].width);
				input_report_abs(touch_dev->input_dev, ABS_MT_WIDTH_MAJOR, (u32)touch_dev->touch_info.coord[i].width);					
				input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_X, x);
				input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_Y, y);
				input_mt_sync(touch_dev->input_dev);	


			}
			else if(zinitix_bit_test(touch_dev->touch_info.coord[i].sub_status, SUB_BIT_UP))			
			{
				zinitix_debug_msg("finger [%02d] up \r\n", i);
				memset(&touch_dev->touch_info.coord[i], 0x0, sizeof(_ts_zinitix_coord));	
				//input_report_abs(touch_dev->input_dev, ABS_MT_TRACKING_ID,i);
				input_report_abs(touch_dev->input_dev, ABS_MT_TOUCH_MAJOR, 0);
				input_report_abs(touch_dev->input_dev, ABS_MT_WIDTH_MAJOR, 0);	
				input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_X, touch_dev->reported_touch_info.coord[i].x);
				input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_Y, touch_dev->reported_touch_info.coord[i].y);
				input_mt_sync(touch_dev->input_dev);

			}		

			else
				memset(&touch_dev->touch_info.coord[i], 0x0, sizeof(_ts_zinitix_coord));	


		}		        
		memcpy((char*)&touch_dev->reported_touch_info, (char*)&touch_dev->touch_info, sizeof(_ts_zinitix_point_info));	
             input_report_key(touch_dev->input_dev, BTN_TOUCH, 1);  /* SKC */
		input_sync(touch_dev->input_dev);

#endif	// TOUCH_MODE == 1
continue_read_samples:

		//check_interrupt_pin, if high, enable int & wait signal
		//			if (gpio_get_value(touch_dev->int_gpio_num))
		if(1)
		{
			read_coord_continued = false;				
			if(touch_dev->work_proceedure == TS_NORMAL_WORK)
			{
#if	ZINITIX_ESD_TIMER_INTERVAL					
				if(touch_dev->use_esd_timer)
				{
					ts_esd_timer_start(ZINITIX_CHECK_ESD_TIMER, touch_dev);	
					zinitix_debug_msg("esd timer start\r\n");
				}
#endif							

				touch_dev->work_proceedure = TS_NO_WORK;
			}
			up(&touch_dev->work_proceedure_lock);
		} 
		else
		{
			zinitix_debug_msg("interrupt pin is still low, so continue read \r\n");
		}

	}while(read_coord_continued);

	return IRQ_HANDLED;

}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void zinitix_late_resume(struct early_suspend *h)
{

	zinitix_touch_dev * touch_dev;
	touch_dev = container_of(h, zinitix_touch_dev, early_suspend);
	printk(KERN_INFO "[TSP] resume++\r\n");
	if(touch_dev == NULL)	return;

	down(&touch_dev->work_proceedure_lock);
	if(touch_dev->work_proceedure != TS_IN_RESUME && touch_dev->work_proceedure != TS_IN_EALRY_SUSPEND)
	{
		up(&touch_dev->work_proceedure_lock);
		return;
	}	

	//	touch_power_onoff(1);
	//	mdelay(50);
	// for trebon tsp power on
#ifdef CONFIG_TREBON_HW01
//PSJ	touch_io_ctrl_regulator(1);	//SEC_DRIVER
#endif
//PSJ	touch_ctrl_regulator(1);	//SEC_DRIVER
	touch_power_control(1);
	mdelay(CHIP_ON_DELAY);

	ts_write_cmd(touch_dev->client, ZINITIX_WAKEUP_CMD);
	mdelay(10);	
	if(ts_mini_init_touch(touch_dev)==false)	goto fail_resume;
	touch_dev->work_proceedure = TS_NO_WORK;	
	up(&touch_dev->work_proceedure_lock);
	printk(KERN_INFO "[TSP] resume--\n");		
	return;	
fail_resume:
	printk(KERN_ERR "[TSP] failed to resume\n");
	touch_dev->work_proceedure = TS_NO_WORK;	
	up(&touch_dev->work_proceedure_lock);
	return;
}


static void zinitix_early_suspend(struct early_suspend *h)
{
	zinitix_touch_dev * touch_dev;
	touch_dev = container_of(h, zinitix_touch_dev, early_suspend);
	if(touch_dev == NULL)	return;

#if	ZINITIX_ESD_TIMER_INTERVAL	
	flush_work(&touch_dev->tmr_work);
#endif

	down(&touch_dev->work_proceedure_lock);
	if(touch_dev->work_proceedure != TS_NO_WORK)
	{
		up(&touch_dev->work_proceedure_lock);
		return;
	}	
	touch_dev->work_proceedure = TS_IN_EALRY_SUSPEND;

	printk(KERN_INFO "[TSP] early suspend++\n");
	zinitix_debug_msg("clear all reported points\r\n");
	zinitix_clear_report_data(touch_dev);

#if	ZINITIX_ESD_TIMER_INTERVAL	
	if(touch_dev->use_esd_timer)
	{			
		ts_write_reg(touch_dev->client, ZINITIX_PERIODICAL_INTERRUPT_INTERVAL, 0);
		ts_esd_timer_stop(touch_dev);
		printk(KERN_INFO "[TSP] ts_esd_timer_stop\n");			
	}
#endif		

#if !(USING_CHIP_SETTING)
	ts_write_reg(touch_dev->client, ZINITIX_INT_ENABLE_FLAG, 0x0);
#endif
	udelay(100);
	ts_write_cmd(touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
	if (ts_write_cmd(touch_dev->client, ZINITIX_SLEEP_CMD)!=I2C_SUCCESS)
	{
		printk(KERN_ERR "[TSP] failed to enter into sleep mode\n");
		up(&touch_dev->work_proceedure_lock);
//		touch_power_onoff(0);	
//		mdelay(50);
#ifdef CONFIG_TREBON_HW01
//PSJ		touch_io_ctrl_regulator(0);	//SEC_DRIVER
#endif
//PSJ		touch_ctrl_regulator(0);	//SEC_DRIVER
	touch_power_control(0); //PSJ
		mdelay(CHIP_POWER_OFF_DELAY);		
		return;
	}	

//	vled_power_onoff(0);
//	touch_power_onoff(0);	
//	mdelay(50);
#ifdef CONFIG_TREBON_HW01
//PSJ	touch_io_ctrl_regulator(0);	//SEC_DRIVER
#endif
//PSJ	touch_ctrl_regulator(0);	//SEC_DRIVER
	touch_power_control(0); //PSJ
	mdelay(CHIP_POWER_OFF_DELAY);

	printk(KERN_INFO "[TSP] early suspend--\n");
	up(&touch_dev->work_proceedure_lock);
	return;
}

#endif	// CONFIG_HAS_EARLYSUSPEND



#if	USE_TEST_RAW_TH_DATA_MODE

static ssize_t zinitix_get_test_raw_data(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i,j;
	int k = 0;

	int written_bytes = 0 ;  /* & error check */

	int16_t raw_data[X_RAW_DATA][Y_RAW_DATA]={{0,},};
	int16_t diff_data[X_RAW_DATA][Y_RAW_DATA]={{0,},};


	down(&misc_touch_dev->raw_data_lock);

	for(j = 0 ; j < Y_RAW_DATA; j++)		
	{
		k = j;
		for(i = 0 ; i < X_RAW_DATA ; i++)
		{
			raw_data[i][j] =(int)(misc_touch_dev->ref_data[k]&0xffff);
			diff_data[i][j] =(int)(((s16)(misc_touch_dev->cur_data[k]-misc_touch_dev->ref_data[k]))&0xffff);
			k+=Y_RAW_DATA;
		}
	}

	up(&misc_touch_dev->raw_data_lock);	


	for(i = 0 ; i < X_RAW_DATA ; i++)
	{
		printk("[TSP]");
		for(j = 0 ; j < Y_RAW_DATA ; j++ )
			printk(" %5d", raw_data[i][j]);
		printk("\n");
	}


	for(i = 0 ; i < X_RAW_DATA ; i++)
	{
		printk("[TSP]");
		for(j = 0 ; j < Y_RAW_DATA ; j++ )
			printk(" %5d", diff_data[i][j]);
		printk("\n");
	}

	for(j = 0 ; j < Y_RAW_DATA ; j++)
	{
		for (i = 0; i < X_RAW_DATA ; i++)
			written_bytes += sprintf(buf+written_bytes, "%d %d\n", raw_data[i][j], diff_data[i][j]);
	}

	if (written_bytes > 0)
		return written_bytes;

	return sprintf(buf, "-1");
}


ssize_t zinitix_set_testmode(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned char value = 0;

	printk(KERN_INFO "[zinitix_touch] zinitix_set_testmode, buf = %d\r\n", *buf);

	if(misc_touch_dev == NULL)
	{
		zinitix_debug_msg("device NULL : NULL\n");
		return 0;
	}

	sscanf(buf, "%c", &value);

	if(value !=	TOUCH_TEST_RAW_MODE && value !=	TOUCH_NORMAL_MODE)
	{
		printk(KERN_WARNING "[zinitix ts] test mode setting value error. you must set %d[=normal] or %d[=raw mode]\r\n", TOUCH_NORMAL_MODE, TOUCH_TEST_RAW_MODE);
		return 1;
	}

	down(&misc_touch_dev->raw_data_lock);	
	misc_touch_dev->raw_mode_flag = value;

	printk(KERN_INFO "[zinitix_touch] zinitix_set_testmode, touchkey_testmode = %d\r\n", misc_touch_dev->raw_mode_flag);

	if(misc_touch_dev->raw_mode_flag == TOUCH_NORMAL_MODE) // 테스트 모드 해제시
	{	
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
	}
	else // 테스트 모드 진입시
	{
		printk(KERN_INFO "[zinitix_touch] TEST Mode Enter\r\n");

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
		memset(&misc_touch_dev->reported_touch_info, 0x0, sizeof(_ts_zinitix_point_info));
		memset(&misc_touch_dev->touch_info, 0x0, sizeof(_ts_zinitix_point_info));
	}
	up(&misc_touch_dev->raw_data_lock);
	return 1;             

}
static ssize_t zinitix_tkey_reference(struct device *dev, struct device_attribute *attr, char *buf)
{
	zinitix_touch_dev *touch_dev = global_touch_dev;    


	printk(KERN_INFO "[zinitix_touch] %s\r\n", __FUNCTION__);

	if(touch_dev == NULL)
		zinitix_debug_msg("device NULL : NULL\n");

	printk(KERN_INFO "[zinitix_touch] %s :  menu(%d), back(%d)\r\n", __FUNCTION__, touch_dev->cur_data[ZINITIX_MENU_KEY], touch_dev->cur_data[ZINITIX_BACK_KEY]);

	return sprintf(buf, "%d %d", touch_dev->cur_data[ZINITIX_MENU_KEY], touch_dev->cur_data[ZINITIX_BACK_KEY]);    
}

static ssize_t zinitix_tkey_data(struct device *dev, struct device_attribute *attr, char *buf)
{
	zinitix_touch_dev *touch_dev = global_touch_dev;    
//	u16 menukey, backkey;

	printk(KERN_INFO "[zinitix_touch] %s\r\n", __FUNCTION__);

	if(touch_dev == NULL)
		zinitix_debug_msg("device NULL : NULL\n");

	printk(KERN_INFO "[zinitix_touch] %s :  menu(%d)(%d), back(%d)(%d)\r\n", __FUNCTION__, 
								touch_dev->ref_data[ZINITIX_MENU_KEY], touch_dev->cur_data[ZINITIX_MENU_KEY], 
								touch_dev->ref_data[ZINITIX_BACK_KEY],touch_dev->cur_data[ZINITIX_BACK_KEY]);  

	return sprintf(buf, "%d\n%d\n%d\n%d\n", touch_dev->ref_data[ZINITIX_MENU_KEY], touch_dev->cur_data[ZINITIX_MENU_KEY], 
								touch_dev->ref_data[ZINITIX_BACK_KEY],touch_dev->cur_data[ZINITIX_BACK_KEY]);    
}
static int16_t zinitix_raw_data[X_RAW_DATA][Y_RAW_DATA] = {{0,},};
static ssize_t zinitix_touch_reference(struct device *dev, struct device_attribute *attr, char *buf)
{
	int time_out = 0;
	int i, j;
	int interrup_detecting;
	int written_bytes = 0;	/* error check */

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
				return snprintf(buf, 2, "0");

			if (raw_data[i][j] > CAL_MAX_NUM)
				return snprintf(buf, 2, "0");
		}
	}

	// TOUCH KEY ndata
	for (j = 0; j < misc_touch_dev->cap_info.y_node_num; j++) {
		if ((j == 1) || (j == 8)) {
			if (raw_data[misc_touch_dev->cap_info.x_node_num-1][j]
			< CAL_MIN_NUM)
				return snprintf(buf, 2, "0");

			if (raw_data[misc_touch_dev->cap_info.x_node_num-1][j]
			> CAL_MAX_NUM)
				return snprintf(buf, 2, "0");
		}
	}
	//return snprintf(buf, 2, "1");
	for (i = 0; i < misc_touch_dev->cap_info.x_node_num; i++) {
		for (j = 0; j < misc_touch_dev->cap_info.y_node_num; j++) {
			written_bytes += sprintf(buf+written_bytes, "%d,",raw_data[i][j]);
		}
	}
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

static ssize_t zinitix_touch_ndata_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i,j,written_bytes=0;
	printk("[TSP] %s\n",__func__);	
	
	for (i = 0; i < misc_touch_dev->cap_info.x_node_num-1; i++) {
		for (j = 0; j < misc_touch_dev->cap_info.y_node_num; j++) {
			written_bytes += sprintf(buf+written_bytes, "%d,",zinitix_raw_data[i][j]);
		}
	}
	if(written_bytes > 0)
		return written_bytes;  
	return -1;
}
static ssize_t zinitix_delta(struct device *dev, struct device_attribute *attr, char *buf)
{
	zinitix_touch_dev *touch_dev = global_touch_dev;    
	int written_bytes = 0, i;

	printk(KERN_INFO "[zinitix_touch] %s\r\n", __FUNCTION__);

	if(touch_dev == NULL)
		zinitix_debug_msg("device NULL : NULL\n");

	for(i = 0 ; i < MAX_TEST_RAW_DATA; i++)
	{
		written_bytes += sprintf(buf+written_bytes, "%d,", touch_dev->cur_data[i] - touch_dev->ref_data[i]) ;
	}

	if(written_bytes > 0)
		return written_bytes;    
	return -1;
}

static ssize_t zinitix_scantime(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i, j;
	int written_bytes = 0;	/* error check */

	u8 raw_data[FULL_X_DATA][FULL_Y_DATA] = {{0,},}; // get the full_data, 1 byte
	u8 scantime_data[X_RAW_DATA][Y_RAW_DATA] = {{0,},};

	printk(KERN_INFO "[zinitix_touch] scantime_zinitix( )\r\n");
	disable_irq(misc_touch_dev->client->irq);

	if (ts_write_reg(misc_touch_dev->client,
			ZINITIX_TOUCH_MODE, 8) != I2C_SUCCESS) {
		printk(KERN_INFO "[zinitix_touch] ZINITIX_TOUCH_MODE fail\r\n");
		goto fail_read_raw;
	}
	msleep(100);
	ts_write_cmd(misc_touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);

	//	ts_read_data(misc_touch_dev->client, ZINITIX_CAL_N_TOTAL_NUM, (u8*)&misc_touch_dev->cap_info.total_cal_n, 2);
	if (ts_read_raw_data(misc_touch_dev->client,
			ZINITIX_RAWDATA_REG, (char *)raw_data, SCANTIME_RAWDATA) < 0) {
		printk(KERN_INFO "[TSP] error : read zinitix tc Cal N raw data\n");
		goto fail_read_raw;
	}
	msleep(10);
	ts_write_cmd(misc_touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
	msleep(10);

	if (ts_write_reg(misc_touch_dev->client,
			ZINITIX_TOUCH_MODE, TOUCH_MODE) != I2C_SUCCESS) {
		printk(KERN_INFO "[zinitix_touch] ZINITIX_TOUCH_MODE fail\r\n");
		goto fail_read_raw;
	}
	ts_write_cmd(misc_touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
	msleep(10);
	enable_irq(misc_touch_dev->client->irq);

#if 0
	for (i = 0; i < 20/* misc_touch_dev->cap_info.x_node_num*/; i++) { 
		printk(KERN_CONT "[TSP]"); 
		for (j = 0; j < 16/*misc_touch_dev->cap_info.y_node_num*/; j++) {
			//scantime_data[i][j] = raw_data[i][j];
			printk(KERN_CONT " %d", raw_data[i][j]);
		}
		printk(KERN_CONT "\n");
	}
#endif // #if 0. This is only for test to check raw_data

	for (i = 0; i < misc_touch_dev->cap_info.x_node_num; i++) {
		printk(KERN_CONT "[TSP]");
		for (j = 0; j < misc_touch_dev->cap_info.y_node_num; j++) {
			scantime_data[i][j] = raw_data[i][j];
			printk(KERN_CONT " %d", scantime_data[i][j]);
		}
		printk(KERN_CONT "\n");
	}

	for (i = 0; i < misc_touch_dev->cap_info.x_node_num; i++) {
		for (j = 0; j < misc_touch_dev->cap_info.y_node_num; j++) {
//			snprintf(buf+written_bytes, 8, "%d,",
//			scantime_data[i][j]);
//			written_bytes += 4;
			written_bytes +=sprintf(buf+written_bytes, "%d,",scantime_data[i][j]);

		}
	}

	if(written_bytes > 0)
		return written_bytes;    

	return snprintf(buf, 2, "-1");

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

static ssize_t zinitix_enter_testmode(struct device *dev, struct device_attribute *attr, char *buf)
{
	zinitix_touch_dev *touch_dev = global_touch_dev;    

	printk(KERN_INFO "[zinitix_touch] %s 111 name : %s\r\n", __FUNCTION__, touch_dev->client->name);

	if(touch_dev == NULL)
	zinitix_debug_msg("device NULL : NULL\n");

	down(&touch_dev->raw_data_lock);	
	touch_dev->raw_mode_flag = TOUCH_TEST_RAW_MODE;

	if (ts_write_reg(touch_dev->client, ZINITIX_PERIODICAL_INTERRUPT_INTERVAL, ZINITIX_SCAN_RATE_HZ*ZINITIX_RAW_DATA_ESD_TIMER_INTERVAL)!=I2C_SUCCESS)
		printk(KERN_INFO "[zinitix_touch] Fail to set ZINITIX_RAW_DATA_ESD_TIMER_INTERVAL.\r\n");

	if (ts_write_reg(touch_dev->client, ZINITIX_TOUCH_MODE, TOUCH_TEST_RAW_MODE)!=I2C_SUCCESS)
	{
		printk(KERN_INFO "[zinitix_touch] TEST Mode : Fail to set ZINITX_TOUCH_MODE %d.\r\n", TOUCH_TEST_RAW_MODE);
	}

	ts_write_cmd(touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
	// clear garbage data
	mdelay(100);
	ts_write_cmd(touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);		
	memset(&touch_dev->reported_touch_info, 0x0, sizeof(_ts_zinitix_point_info));
	memset(&touch_dev->touch_info, 0x0, sizeof(_ts_zinitix_point_info));

	up(&touch_dev->raw_data_lock);
	return 1;             
}

static ssize_t zinitix_enter_scanmode(struct device *dev, struct device_attribute *attr, char *buf)
{
	zinitix_touch_dev *touch_dev = global_touch_dev;    

	printk(KERN_INFO "[zinitix_touch] %s 111 name : %s\r\n", __FUNCTION__, touch_dev->client->name);

	if(touch_dev == NULL)
		zinitix_debug_msg("device NULL : NULL\n");

	down(&touch_dev->raw_data_lock);	
	touch_dev->raw_mode_flag = TOUCH_ZINITIX_CAL_N_MODE;

	if (ts_write_reg(touch_dev->client, ZINITIX_PERIODICAL_INTERRUPT_INTERVAL, ZINITIX_SCAN_RATE_HZ*ZINITIX_RAW_DATA_ESD_TIMER_INTERVAL)!=I2C_SUCCESS)
		printk(KERN_INFO "[zinitix_touch] Fail to set ZINITIX_RAW_DATA_ESD_TIMER_INTERVAL.\r\n");

	if (ts_write_reg(touch_dev->client, ZINITIX_TOUCH_MODE, TOUCH_ZINITIX_CAL_N_MODE)!=I2C_SUCCESS)
	{
		printk(KERN_INFO "[zinitix_touch] TEST Mode : Fail to set ZINITX_TOUCH_MODE %d.\r\n", TOUCH_ZINITIX_CAL_N_MODE);
	}

	ts_write_cmd(touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
	// clear garbage data
	mdelay(100);
	ts_write_cmd(touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);		
	memset(&touch_dev->reported_touch_info, 0x0, sizeof(_ts_zinitix_point_info));
	memset(&touch_dev->touch_info, 0x0, sizeof(_ts_zinitix_point_info));

	up(&touch_dev->raw_data_lock);
	return 1;             
}

static ssize_t zinitix_enter_normalmode(struct device *dev, struct device_attribute *attr, char *buf)
{
	zinitix_touch_dev *touch_dev = global_touch_dev;    

	printk(KERN_INFO "[zinitix_touch] %s\r\n", __FUNCTION__);

	if(touch_dev == NULL)
		zinitix_debug_msg("device NULL : NULL\n");

	down(&touch_dev->raw_data_lock);	
	touch_dev->raw_mode_flag = TOUCH_NORMAL_MODE;

	printk(KERN_INFO "[zinitix_touch] TEST Mode Exit\r\n");

	if (ts_write_reg(touch_dev->client, ZINITIX_PERIODICAL_INTERRUPT_INTERVAL, ZINITIX_SCAN_RATE_HZ*ZINITIX_ESD_TIMER_INTERVAL)!=I2C_SUCCESS)
		printk(KERN_INFO "[zinitix_touch] Fail to set ZINITIX_PERIODICAL_INTERRUPT_INTERVAL.\r\n");

	if (ts_write_reg(touch_dev->client, ZINITIX_TOUCH_MODE, TOUCH_MODE)!=I2C_SUCCESS)
	{
		printk(KERN_INFO "[zinitix_touch] Fail to set ZINITX_TOUCH_MODE %d.\r\n", TOUCH_MODE);
	}
	// clear garbage data
	ts_write_cmd(touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
	mdelay(100);
	ts_write_cmd(touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);	

	up(&touch_dev->raw_data_lock);
	return 1;                
}
static ssize_t zinitix_count_node(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk("[TSP] %s\n",__func__);
	return sprintf(buf, "%d,%d\n", X_RAW_DATA,Y_RAW_DATA);
}
static ssize_t zinitix_chip_info(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk("[TSP] %s\n",__func__);
	return sprintf(buf, "%s,%s\n", CHIP_INFO,CHIP_NAME);
}
static ssize_t zinitix_menu_sensitivity_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret = 0;
	u16 menu_sensitivity;

	ret = ts_read_data(misc_touch_dev->client, 0x00A8, (u8*)&menu_sensitivity, 2);

	if (ret<0)
		return sprintf(buf, "%s\n",  "fail to read menu_sensitivity.");	
	else
		return sprintf(buf, "%d\n",  menu_sensitivity);
}

static ssize_t zinitix_back_sensitivity_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret = 0;
	u16 back_sensitivity;

	ret = ts_read_data(misc_touch_dev->client, 0x00A9, (u8*)&back_sensitivity, 2);

	if (ret<0)
		return sprintf(buf, "%s\n",  "fail to read back_sensitivity.");	
	else
		return sprintf(buf, "%d\n",  back_sensitivity);
}
static DEVICE_ATTR(get_touch_test_raw_data, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, zinitix_get_test_raw_data, zinitix_set_testmode);
static DEVICE_ATTR(tkey_reference, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, zinitix_tkey_reference, NULL);
static DEVICE_ATTR(touch_reference, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, zinitix_touch_reference, NULL);
static DEVICE_ATTR(delta, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, zinitix_delta, NULL);
static DEVICE_ATTR(scantime, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, zinitix_scantime, NULL);
static DEVICE_ATTR(enter_testmode, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, zinitix_enter_testmode, NULL);
static DEVICE_ATTR(enter_scanmode, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, zinitix_enter_scanmode, NULL);
static DEVICE_ATTR(enter_normalmode, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, zinitix_enter_normalmode, NULL);
static DEVICE_ATTR(count_node, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, zinitix_count_node, NULL);	
static DEVICE_ATTR(chip_info, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, zinitix_chip_info, NULL);	
static DEVICE_ATTR(menu_sensitivity, S_IRUGO | S_IWUSR | S_IWGRP, zinitix_menu_sensitivity_show, NULL);
static DEVICE_ATTR(back_sensitivity, S_IRUGO | S_IWUSR | S_IWGRP, zinitix_back_sensitivity_show, NULL);
static DEVICE_ATTR(touch_ndata, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, zinitix_touch_ndata_show, NULL);
static DEVICE_ATTR(tkey_data, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, zinitix_tkey_data, NULL);
#ifdef KEY_LED_CONTROL
static DEVICE_ATTR(brightness, S_IRUGO | S_IWUSR | S_IWGRP, NULL,  touch_led_control);
#endif

static int ts_misc_fops_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int ts_misc_fops_close(struct inode *inode, struct file *filp)
{
	return 0;
}

#if 0//LINUX_VERSION_CODE < KERNEL_VERSION(2.6.36)
static int ts_misc_fops_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#else
static long ts_misc_fops_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
	void __user *argp = (void __user *)arg;


	if(misc_touch_dev==NULL)
	{
		return -1;
	}

	//zinitix_debug_msg("cmd = %d, argp = 0x%x\n", cmd, (int)argp);	

	switch(cmd)
	{

	case TOUCH_IOCTL_GET_DEBUGMSG_STATE:
	{
		int ret = 0;
		ret = m_ts_debug_mode;

		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
	}
	break;
	case TOUCH_IOCTL_SET_DEBUGMSG_STATE:
	{
		int value = 0;			
		if(copy_from_user(&value, argp, 4))
		{
			printk(KERN_INFO "[zinitix_touch] error : copy_from_user\r\n");
			return -1;	
		}
		if(value)
			printk(KERN_INFO "[zinitix_touch] turn on debug mode (%d)\r\n", value);
		else
			printk(KERN_INFO "[zinitix_touch] turn off debug mode (%d)\r\n", value);
		m_ts_debug_mode = value;
	}
	break;
	case TOUCH_IOCTL_GET_CHIP_REVISION:
	{
		int ret = 0;
		ret = misc_touch_dev->cap_info.chip_revision;

		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
	}
	break;

	case TOUCH_IOCTL_GET_FW_VERSION:
	{
		int ret = 0;
		ret = misc_touch_dev->cap_info.chip_firmware_version;

		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
	}
	break;
	case TOUCH_IOCTL_GET_REG_DATA_VERSION:
	{
		int ret = 0;
		ret = misc_touch_dev->cap_info.chip_reg_data_version;

		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
	}
	break;
	case TOUCH_IOCTL_VARIFY_UPGRADE_SIZE:
	{
		size_t 	sz = 0;						
		if(copy_from_user(&sz, argp, sizeof(size_t)))	return -1;

		printk(KERN_INFO "[TSP] firmware size = %d\r\n", sz);
		if(misc_touch_dev->cap_info.chip_fw_size != sz)
		{
			printk(KERN_INFO "[TSP] firmware size error\r\n");
			return -1;
		}

	}
	break;		

	case TOUCH_IOCTL_VARIFY_UPGRADE_DATA:
	{
	//	int ret = 0;
		u16	version;
	//	unsigned int	version_pos;
		if(copy_from_user(&m_firmware_data[2], argp, misc_touch_dev->cap_info.chip_fw_size))	return -1;
#if	BT4x2_Series
//		version_pos = (unsigned int)((((unsigned int)m_firmware_data[4]<<8)&0xff00)|(m_firmware_data[5]&0xff));			
//		version = (u16)(((u16)m_firmware_data[version_pos+2+2]<<8)|(u16)m_firmware_data[version_pos+2+1]);
#endif
#if	BT4x3_Above_Series 			
		version = (u16)(((u16)m_firmware_data[FIRMWARE_VERSION_POS+1]<<8)|(u16)m_firmware_data[FIRMWARE_VERSION_POS]);			
#endif
		printk(KERN_INFO "[TSP] firmware version = %x\r\n", version);

			if (copy_to_user(argp, &version, sizeof(version)))			return -1;
	}
	break;		
	case TOUCH_IOCTL_START_UPGRADE:
	{
	//	int ret = 0;			

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

		//------------------------------------------------------------
		printk(KERN_INFO "[TSP] start upgrade firmware\n");			
		if(ts_upgrade_firmware(misc_touch_dev, &m_firmware_data[2], misc_touch_dev->cap_info.chip_fw_size)==false)
		{
			enable_irq(misc_touch_dev->irq);
			misc_touch_dev->work_proceedure = TS_NO_WORK;
			up(&misc_touch_dev->work_proceedure_lock);																	
			return -1;
		}

		//------------------------------------------------------------			

		if(ts_init_touch(misc_touch_dev)==false)
		{
			enable_irq(misc_touch_dev->irq);
			misc_touch_dev->work_proceedure = TS_NO_WORK;
			up(&misc_touch_dev->work_proceedure_lock);										
			return -1;
		}

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

	}
	break;		

	case TOUCH_IOCTL_GET_X_RESOLUTION:
	{
		int ret = 0;
		ret = misc_touch_dev->cap_info.x_resolution;

		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
	}
	break;

	case TOUCH_IOCTL_GET_Y_RESOLUTION:
	{
		int ret = 0;
		ret = misc_touch_dev->cap_info.y_resolution;

		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
	}
	break;

	case TOUCH_IOCTL_GET_X_NODE_NUM:
	{
		int ret = 0;
		ret = misc_touch_dev->cap_info.x_node_num;

		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
		}
	break;

	case TOUCH_IOCTL_GET_Y_NODE_NUM:
	{
		int ret = 0;
		ret = misc_touch_dev->cap_info.y_node_num;

		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
	}
	break;

	case TOUCH_IOCTL_GET_TOTAL_NODE_NUM:
	{
		int ret = 0;
		ret = misc_touch_dev->cap_info.total_node_num;

		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
	}
	break;

	case TOUCH_IOCTL_HW_CALIBRAION:
	{
	//	int value = 0;
		u16 mode;
		u16 chip_eeprom_info;
		int ret = -1;

		if(misc_touch_dev == NULL)	zinitix_debug_msg("misc device NULL?\n");
		disable_irq(misc_touch_dev->irq);
		down(&misc_touch_dev->work_proceedure_lock);	
		if(misc_touch_dev->work_proceedure != TS_NO_WORK)
		{
			printk(KERN_INFO"[TSP] other process occupied.. (%d)\r\n", misc_touch_dev->work_proceedure);
			up(&misc_touch_dev->work_proceedure_lock);
			return -1;
		}
		misc_touch_dev->work_proceedure = TS_HW_CALIBRAION;


		// h/w calibration
		if (ts_write_reg(misc_touch_dev->client, ZINITIX_TOUCH_MODE, 0x07)!=I2C_SUCCESS) goto fail_hw_cal;
		if (ts_write_cmd(misc_touch_dev->client, ZINITIX_CALIBRATE_CMD)!=I2C_SUCCESS)	goto fail_hw_cal;
		if (ts_write_cmd(misc_touch_dev->client, ZINITIX_SWRESET_CMD)!=I2C_SUCCESS)	goto fail_hw_cal;
		mdelay(1); 		
		ts_write_cmd(misc_touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
		// wait for h/w calibration
		do{
			mdelay(1000); 
			if (ts_read_data(misc_touch_dev->client, ZINITIX_EEPROM_INFO_REG, (u8*)&chip_eeprom_info, 2)<0) goto fail_hw_cal;
			zinitix_debug_msg("touch eeprom info = 0x%04X\r\n", chip_eeprom_info);
			if(!zinitix_bit_test(chip_eeprom_info, 0))	break;
		}while(1);

#if BT4x3_Above_Series		
		mdelay(10);
		if (ts_write_cmd(misc_touch_dev->client, ZINITIX_SAVE_CALIBRATION_CMD)!=I2C_SUCCESS) goto fail_hw_cal;
		mdelay(500);
#endif		
		ret = 0;
fail_hw_cal:
		if(misc_touch_dev->raw_mode_flag == TOUCH_NORMAL_MODE)			
			mode = TOUCH_MODE;			
		else
			mode = misc_touch_dev->raw_mode_flag;

		if (ts_write_reg(misc_touch_dev->client, ZINITIX_TOUCH_MODE, mode)!=I2C_SUCCESS)
		{
			printk(KERN_INFO "[zinitix_touch] Fail to set ZINITX_TOUCH_MODE %d.\r\n", mode);
			goto fail_hw_cal2;
		}

		if (ts_write_cmd(misc_touch_dev->client, ZINITIX_SWRESET_CMD)!=I2C_SUCCESS) goto fail_hw_cal2;

#if BT4x2_Series				
		if (ts_write_cmd(misc_touch_dev->client, ZINITIX_SAVE_STATUS_CMD)!=I2C_SUCCESS) goto fail_hw_cal2;
		mdelay(1000);	// for fusing eeprom
		if (ts_write_cmd(misc_touch_dev->client, ZINITIX_SWRESET_CMD)!=I2C_SUCCESS) goto fail_hw_cal2;				
#endif			
		enable_irq(misc_touch_dev->irq);
		misc_touch_dev->work_proceedure = TS_NO_WORK;
		up(&misc_touch_dev->work_proceedure_lock);
		return ret;		
fail_hw_cal2:  
		enable_irq(misc_touch_dev->irq);
		misc_touch_dev->work_proceedure = TS_NO_WORK;
		up(&misc_touch_dev->work_proceedure_lock);
		return -1;
	}
	break;

	case TOUCH_IOCTL_SET_RAW_DATA_MODE:
	{

		int value = 0;
		if(misc_touch_dev == NULL)
		{
			zinitix_debug_msg("misc device NULL?\n");
			return -1;
		}

		down(&misc_touch_dev->work_proceedure_lock);	
		if(misc_touch_dev->work_proceedure != TS_NO_WORK)
		{
			printk(KERN_INFO"[TSP] other process occupied.. (%d)\r\n", misc_touch_dev->work_proceedure);
			up(&misc_touch_dev->work_proceedure_lock);
			return -1;
		}
		misc_touch_dev->work_proceedure = TS_SET_MODE;

		if(copy_from_user(&value, argp, 4))
		{				
			printk(KERN_INFO "[zinitix_touch] error : copy_from_user\r\n");
			misc_touch_dev->work_proceedure = TS_NO_WORK;
			return -1;	
		}
		/*		
		if(value != TOUCH_ZINITIX_BASELINED_RAW_MODE &&  value != TOUCH_ZINITIX_PROCESSED_RAW_MODE 
		&& value != TOUCH_ZINITIX_CAL_N_MODE && value != TOUCH_NORMAL_MODE)
		{
		printk(KERN_WARNING "[zinitix ts] test mode setting value error. you must set %d[=normal] or %d[=raw mode]\r\n", TOUCH_NORMAL_MODE, TOUCH_ZINITIX_BASELINED_RAW_MODE);
		misc_touch_dev->work_proceedure = TS_NO_WORK;
		up(&misc_touch_dev->work_proceedure_lock);
		return 0;
		}
		*/



		zinitix_debug_msg("[zinitix_touch] zinitix_set_testmode, touchkey_testmode = %d\r\n", misc_touch_dev->raw_mode_flag);

		if(value == TOUCH_NORMAL_MODE && misc_touch_dev->raw_mode_flag != TOUCH_NORMAL_MODE) // 테스트 모드 해제시
		{	
			misc_touch_dev->raw_mode_flag = value;
			printk(KERN_INFO "[zinitix_touch] raw data mode exit\r\n");

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
		}
		else if(value != TOUCH_NORMAL_MODE)// 테스트 모드 진입시
		{
			misc_touch_dev->raw_mode_flag = value;
			printk(KERN_INFO "[zinitix_touch] raw data mode enter\r\n");

			if (ts_write_reg(misc_touch_dev->client, ZINITIX_PERIODICAL_INTERRUPT_INTERVAL, ZINITIX_SCAN_RATE_HZ*ZINITIX_RAW_DATA_ESD_TIMER_INTERVAL)!=I2C_SUCCESS)
				printk(KERN_INFO "[zinitix_touch] Fail to set ZINITIX_RAW_DATA_ESD_TIMER_INTERVAL.\r\n");

			if (ts_write_reg(misc_touch_dev->client, ZINITIX_TOUCH_MODE, misc_touch_dev->raw_mode_flag)!=I2C_SUCCESS)
			{
				printk(KERN_INFO "[zinitix_touch] raw data mode : Fail to set ZINITX_TOUCH_MODE %d.\r\n", misc_touch_dev->raw_mode_flag);
			}
			ts_write_cmd(misc_touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);
			// clear garbage data
			mdelay(100);
			ts_write_cmd(misc_touch_dev->client, ZINITIX_CLEAR_INT_STATUS_CMD);		
			//memset(&misc_touch_dev->reported_touch_info, 0x0, sizeof(_ts_zinitix_point_info));
			//memset(&misc_touch_dev->touch_info, 0x0, sizeof(_ts_zinitix_point_info));
		}

		misc_touch_dev->work_proceedure = TS_NO_WORK;
		up(&misc_touch_dev->work_proceedure_lock);
		return 0;		  

	}
	break;
	case TOUCH_IOCTL_GET_REG:
	{
		_reg_ioctl reg_ioctl;		
		u16	val;	
		int 	ret =0;
		int	nval;

		if(misc_touch_dev == NULL)
		{
			zinitix_debug_msg("misc device NULL?\n");
			return -1;
		}
		down(&misc_touch_dev->work_proceedure_lock);	
		if(misc_touch_dev->work_proceedure != TS_NO_WORK)
		{
			printk(KERN_INFO"[TSP] other process occupied.. (%d)\r\n", misc_touch_dev->work_proceedure);
			up(&misc_touch_dev->work_proceedure_lock);
			return -1;
		}

		misc_touch_dev->work_proceedure = TS_SET_MODE;

		if(copy_from_user(&reg_ioctl, argp, sizeof(_reg_ioctl)))
		{
			misc_touch_dev->work_proceedure = TS_NO_WORK;
			up(&misc_touch_dev->work_proceedure_lock);
			printk(KERN_INFO "[zinitix_touch] error : copy_from_user\r\n");
			return -1;	
		}

		if (ts_read_data(misc_touch_dev->client, reg_ioctl.addr, (u8*)&val, 2)<0) ret = -1;

		nval = (int)val;

		if (copy_to_user(reg_ioctl.val, (u8*)&nval, 4))
		{
			misc_touch_dev->work_proceedure = TS_NO_WORK;
			up(&misc_touch_dev->work_proceedure_lock);
			printk(KERN_INFO "[zinitix_touch] error : copy_to_user\r\n");
			return -1;
		}

		zinitix_debug_msg("read register : register address = 0x%x, value = 0x%x\r\n", reg_ioctl.addr, nval);

		misc_touch_dev->work_proceedure = TS_NO_WORK;
		up(&misc_touch_dev->work_proceedure_lock); 
		return ret; 
	}	
	case TOUCH_IOCTL_SET_REG:
	{
		_reg_ioctl reg_ioctl;					
		int ret =0;
		int	val;

		if(misc_touch_dev == NULL)
		{
			zinitix_debug_msg("misc device NULL?\n");
			return -1;
		}
		down(&misc_touch_dev->work_proceedure_lock);	
		if(misc_touch_dev->work_proceedure != TS_NO_WORK)
		{
			printk(KERN_INFO"[TSP] other process occupied.. (%d)\r\n", misc_touch_dev->work_proceedure);
			up(&misc_touch_dev->work_proceedure_lock);
			return -1;
		}

		misc_touch_dev->work_proceedure = TS_SET_MODE;

		if(copy_from_user(&reg_ioctl, argp, sizeof(_reg_ioctl)))
		{
			misc_touch_dev->work_proceedure = TS_NO_WORK;
			up(&misc_touch_dev->work_proceedure_lock);
			printk(KERN_INFO "[zinitix_touch] error : copy_from_user\r\n");
			return -1;	
		}			

		if(copy_from_user(&val, reg_ioctl.val, 4))
		{
			misc_touch_dev->work_proceedure = TS_NO_WORK;
			up(&misc_touch_dev->work_proceedure_lock);
			printk(KERN_INFO "[zinitix_touch] error : copy_from_user\r\n");
			return -1;	
		}

		if (ts_write_reg(misc_touch_dev->client, reg_ioctl.addr, val)!=I2C_SUCCESS)	
			ret = -1;			

		zinitix_debug_msg("write register : register address = 0x%x, value = 0x%x\r\n", reg_ioctl.addr, val);

		misc_touch_dev->work_proceedure = TS_NO_WORK;
		up(&misc_touch_dev->work_proceedure_lock); 
		return ret; 
	}	
	case TOUCH_IOCTL_DONOT_TOUCH_EVENT:
	{			
		int ret =0;

		if(misc_touch_dev == NULL)
		{
			zinitix_debug_msg("misc device NULL?\n");
			return -1;
		}
		down(&misc_touch_dev->work_proceedure_lock);	
		if(misc_touch_dev->work_proceedure != TS_NO_WORK)
		{
			printk(KERN_INFO"[TSP] other process occupied.. (%d)\r\n", misc_touch_dev->work_proceedure);
			up(&misc_touch_dev->work_proceedure_lock);
			return -1;
		}

		misc_touch_dev->work_proceedure = TS_SET_MODE;			
		if (ts_write_reg(misc_touch_dev->client, ZINITIX_INT_ENABLE_FLAG, 0)!=I2C_SUCCESS)	ret = -1;			

		zinitix_debug_msg("write register : register address = 0x%x, value = 0x0\r\n", ZINITIX_INT_ENABLE_FLAG);

		misc_touch_dev->work_proceedure = TS_NO_WORK;
		up(&misc_touch_dev->work_proceedure_lock); 
		return ret; 
	}	
	break;			
	case TOUCH_IOCTL_SEND_SAVE_STATUS:
	{				
		int ret =0;

		if(misc_touch_dev == NULL)
		{
			zinitix_debug_msg("misc device NULL?\n");
			return -1;
		}
		down(&misc_touch_dev->work_proceedure_lock);	
		if(misc_touch_dev->work_proceedure != TS_NO_WORK)
		{
			printk(KERN_INFO"[TSP] other process occupied.. (%d)\r\n", misc_touch_dev->work_proceedure);
			up(&misc_touch_dev->work_proceedure_lock);
			return -1;
		}
		misc_touch_dev->work_proceedure = TS_SET_MODE;
		ret = 0;
		if (ts_write_cmd(misc_touch_dev->client, ZINITIX_SAVE_STATUS_CMD)!=I2C_SUCCESS) ret =-1;
		mdelay(1000);	// for fusing eeprom						

		misc_touch_dev->work_proceedure = TS_NO_WORK;
		up(&misc_touch_dev->work_proceedure_lock); 
		return ret; 
	}	
	case TOUCH_IOCTL_GET_RAW_DATA:
	{
		_raw_ioctl raw_ioctl;
		u8 *u8Data;
		int i, j, total_cal_n;
		u8 div_node;

		if(misc_touch_dev == NULL)
		{
			zinitix_debug_msg("misc device NULL?\n");
			return -1;
		}

		if(misc_touch_dev->raw_mode_flag == TOUCH_NORMAL_MODE)	return -1;

		down(&misc_touch_dev->raw_data_lock);				
		if(misc_touch_dev->update==0)
		{
			up(&misc_touch_dev->raw_data_lock);
			return -2;
		}

		if(copy_from_user(&raw_ioctl, argp, sizeof(raw_ioctl)))
		{				
			up(&misc_touch_dev->raw_data_lock);
			printk(KERN_INFO "[zinitix_touch] error : copy_from_user\r\n");
			return -1;	
		}

		misc_touch_dev->update = 0;

		u8Data = (u8*)&misc_touch_dev->cur_data[0];
		if(misc_touch_dev->raw_mode_flag == TOUCH_ZINITIX_CAL_N_MODE)
		{
#if BT4x2_Series				
			j = 0;		
			u8Data = (u8*)&misc_touch_dev->cur_data[1];
			for(i=0; i<160; i++)
			{
				if((i*2)%16 < misc_touch_dev->cap_info.y_node_num)
				{
					misc_touch_dev->ref_data[j*2] = misc_touch_dev->cur_data[0] - (u16)u8Data[i*2];
					misc_touch_dev->ref_data[j*2+1] = misc_touch_dev->cur_data[0] - (u16)u8Data[i*2+1];
					j++;
				}			
			}
#endif				

#if BT4x3_Above_Series				
			j = 0;		
			total_cal_n = misc_touch_dev->cap_info.total_cal_n;
			if(total_cal_n == 0) total_cal_n = MAX_TEST_RAW_DATA;

			div_node = (u8)misc_touch_dev->cap_info.max_y_node;
			if(div_node == 0)	div_node = X_RAW_DATA;
			u8Data = (u8*)&misc_touch_dev->cur_data[0];
			for(i=0; i<total_cal_n; i++)
			{
				if((i*2)%div_node < misc_touch_dev->cap_info.y_node_num)
				{
					misc_touch_dev->ref_data[j*2] = (u16)u8Data[i*2];
					misc_touch_dev->ref_data[j*2+1] = (u16)u8Data[i*2+1];
					j++;
				}			
			}
#endif
			u8Data = (u8*)&misc_touch_dev->ref_data[0];
		}

		if (copy_to_user(raw_ioctl.buf, (u8*)u8Data, raw_ioctl.sz))
		{
			up(&misc_touch_dev->raw_data_lock); 
			return -1;
		}			

		up(&misc_touch_dev->raw_data_lock); 

		return 0; 
	}	

	default:
		break;
	}
	return 0;

}

#endif //USE_TEST_RAW_TH_DATA_MODE


static int zinitix_touch_probe(struct i2c_client *client, const struct i2c_device_id *i2c_id)
{
	int ret;
	zinitix_touch_dev* touch_dev;
	int i;

#if defined (CONFIG_TOUCHSCREEN_ATMEL_I2C)
	extern int mxt_probe_ok;

	if(mxt_probe_ok == 1)
	{
		zinitix_debug_msg("probe return+\r\n");
		return -ENODEV;
	}
#endif


#if	BT4x2_Series  	
	zinitix_debug_msg("BT4x2 Driver\r\n");		
#endif

#if	BT4x3_Above_Series  	
	zinitix_debug_msg("Above BT4x3 Driver\r\n");		
#endif	
	printk(KERN_INFO "[zinitix touch] driver version = %s\r\n", TS_DRVIER_VERSION);	

#if	TOUCH_USING_ISP_METHOD	
	if(strcmp(client->name, ZINITIX_ISP_NAME) == 0)
	{
		printk(KERN_INFO "[TSP] isp client probe \r\n");
		m_isp_client = client;
		return 0;
	}
#endif
#if 0
	init_hw();
#endif
	zinitix_debug_msg("zinitix_touch_probe+\r\n");
#ifdef CONFIG_TREBON_HW01
//PSJ	touch_io_ctrl_regulator(0);	//SEC_DRIVER
#endif
//PSJ	touch_ctrl_regulator(0);	//SEC_DRIVER
	touch_power_control(0); //PSJ
	mdelay(1000);
	//		ts_power_control(touch_dev, POWER_ON); //power on
#ifdef CONFIG_TREBON_HW01
//PSJ	touch_io_ctrl_regulator(1);	//SEC_DRIVER
#endif
//PSJ	touch_ctrl_regulator(1);	//SEC_DRIVER
	touch_power_control(1); //PSJ
	//		ts_power_control(touch_dev, RESET_HIGH);	//reset pin high
	mdelay(CHIP_ON_DELAY);
	mdelay(500);

	zinitix_debug_msg("i2c check function \r\n");	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_ERR "[TSP] error : not compatible i2c function \r\n");
		ret = -ENODEV;
		goto err_check_functionality;
	}

	zinitix_debug_msg("touch data alloc \r\n");	
	touch_dev = kzalloc(sizeof(zinitix_touch_dev), GFP_KERNEL);
	if (!touch_dev) {
		printk(KERN_ERR "[TSP] unabled to allocate touch data \r\n");	
		ret = -ENOMEM;
		goto err_alloc_dev_data;
	}
	touch_dev->client = client;
	i2c_set_clientdata(client, touch_dev);

	global_touch_dev = touch_dev;

#if	RESET_CONTROL
	if(gpio_request(SYSTEM_RESET_PIN, "zinitix_reset_pin"))
	{
		printk(KERN_ERR "[TSP] error : could not obtain gpio for reset pin\r\n");
		touch_dev->reset_gpio_num = -ENODEV;
	}
	else
	{		
		// output
		gpio_direction_output(SYSTEM_RESET_PIN, 1);
		touch_dev->reset_gpio_num = SYSTEM_RESET_PIN;
		//		ts_power_control(touch_dev, RESET_HIGH);
	}

	mdelay(CHIP_ON_DELAY);
#endif

	zinitix_debug_msg("allocate input device \r\n");
	touch_dev->input_dev = input_allocate_device();
	if (touch_dev->input_dev == 0) {
		printk(KERN_ERR "[TSP] unabled to allocate input device \r\n");
		ret = -ENOMEM;
		goto err_input_allocate_device;
	}

	//initialize zinitix touch ic
	touch_dev->int_gpio_num = client->irq;	// for upgrade	
	zinitixTsp_irq=client->irq;

	memset(&touch_dev->reported_touch_info, 0x0, sizeof(_ts_zinitix_point_info));

#if	USE_TEST_RAW_TH_DATA_MODE
	touch_dev->raw_mode_flag = TOUCH_NORMAL_MODE;			// not test mode
#endif	

	ts_init_touch(touch_dev);	

	touch_dev->use_esd_timer = 0;

#if	ZINITIX_ESD_TIMER_INTERVAL	
	INIT_WORK(&touch_dev->tmr_work, zinitix_touch_tmr_work);

	zinitix_tmr_workqueue = create_singlethread_workqueue("zinitix_tmr_workqueue");
	if (!zinitix_tmr_workqueue)
	{
		printk(KERN_ERR "unabled to create touch tmr work queue \r\n");
		ret = -1;
		goto err_kthread_create_failed;
	}

	touch_dev->use_esd_timer = 1;
	ts_esd_timer_init(touch_dev);	
	ts_esd_timer_start(ZINITIX_CHECK_ESD_TIMER, touch_dev);	
	printk(KERN_INFO "[TSP] ts_esd_timer_start\n");	
#endif	

	touch_class = class_create(THIS_MODULE, "touch");
	if (IS_ERR(touch_class))
		pr_err("[TSP] Failed to create class(touch)!\n");
	firmware_dev = device_create(touch_class, NULL, 0, NULL, "firmware");
	if (IS_ERR(firmware_dev))
		pr_err("[TSP] Failed to create device(firmware)!\n");
	if (device_create_file(firmware_dev, &dev_attr_firmware_ver) < 0)
		pr_err("[TSP] Failed to create device file(%s)!\n", dev_attr_firmware_ver.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_sw_ver) < 0)
		pr_err("[TSP] Failed to create device file(%s)!\n", dev_attr_sw_ver.attr.name);  
	if (device_create_file(firmware_dev, &dev_attr_tsp_StartCall) < 0)
		pr_err("[TSP] Failed to create device file(%s)!\n", dev_attr_tsp_StartCall.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_tsp_EndCall) < 0)
		pr_err("[TSP] Failed to create device file(%s)!\n", dev_attr_tsp_EndCall.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_debug_xy) < 0)
		pr_err("[TSP] Failed to create device file(%s)!\n", dev_attr_debug_xy.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_fw_update) < 0)
		pr_err("[TSP] Failed to create device file(%s)!\n", dev_attr_fw_update.attr.name);
	if (device_create_file(firmware_dev, &dev_attr_fw_upstatus) < 0)
		pr_err("[TSP] Failed to create device file(%s)!\n", dev_attr_fw_upstatus.attr.name);	  
	sprintf(touch_dev->phys, "input(ts)");
	touch_dev->input_dev->name = ZINITIX_DRIVER_NAME;
	//touch_dev->input_dev->phys = "zinitix_touch/input0";	// <- for compatability
	touch_dev->input_dev->id.bustype = BUS_I2C;
	touch_dev->input_dev->id.vendor = 0x0001;
	touch_dev->input_dev->phys = touch_dev->phys;
	touch_dev->input_dev->id.product = 0x0002;
	touch_dev->input_dev->id.version = 0x0100;
	//touch_dev->input_dev->dev.parent = &client->dev;

	set_bit(EV_SYN, touch_dev->input_dev->evbit);
	set_bit(EV_KEY, touch_dev->input_dev->evbit);
	set_bit(BTN_TOUCH, touch_dev->input_dev->keybit);
	set_bit(EV_ABS, touch_dev->input_dev->evbit);
      set_bit(INPUT_PROP_DIRECT, touch_dev->input_dev->propbit);

	for(i=0; i< MAX_SUPPORTED_BUTTON_NUM; i++)	
		set_bit(BUTTON_MAPPING_KEY[i], touch_dev->input_dev->keybit);


	if (touch_dev->cap_info.Orientation & TOUCH_XY_SWAP)
	{
		input_set_abs_params(touch_dev->input_dev, ABS_MT_POSITION_Y, touch_dev->cap_info.MinX, touch_dev->cap_info.MaxX + ABS_PT_OFFSET, 0, 0);		
		input_set_abs_params(touch_dev->input_dev, ABS_MT_POSITION_X, touch_dev->cap_info.MinY, touch_dev->cap_info.MaxY + ABS_PT_OFFSET, 0, 0);
	}
	else
	{
		input_set_abs_params(touch_dev->input_dev, ABS_MT_POSITION_X, touch_dev->cap_info.MinX, touch_dev->cap_info.MaxX + ABS_PT_OFFSET, 0, 0);		
		input_set_abs_params(touch_dev->input_dev, ABS_MT_POSITION_Y, touch_dev->cap_info.MinY, touch_dev->cap_info.MaxY + ABS_PT_OFFSET, 0, 0);
	}

	input_set_abs_params(touch_dev->input_dev, ABS_TOOL_WIDTH, 0, 255, 0, 0);				
	input_set_abs_params(touch_dev->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(touch_dev->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);

	zinitix_debug_msg("register %s input device \r\n", touch_dev->input_dev->name);
	ret = input_register_device(touch_dev->input_dev);
	if(ret) {
		printk(KERN_ERR "[TSP] unable to register %s input device\r\n", touch_dev->input_dev->name);
		goto err_input_register_device;
	}

	/* configure touchscreen interrupt gpio */
	ret = gpio_request(TSP_INT, "zinitix_irq_gpio");
	if (ret) {
		printk(KERN_ERR "[TSP] unable to request gpio.(%s)\r\n", touch_dev->input_dev->name);
		goto err_request_irq;
	}

	ret = gpio_direction_input(irq_to_gpio(zinitixTsp_irq));

	touch_dev->int_gpio_num = TSP_INT;

#ifdef	GPIO_TOUCH_IRQ
	touch_dev->irq = GPIO_TOUCH_IRQ;
#else
	touch_dev->irq = gpio_to_irq(touch_dev->int_gpio_num);
	if(touch_dev->irq < 0)
	{
		printk(KERN_INFO "[TSP] error. gpio_to_irq(..) function is not supported? you should define GPIO_TOUCH_IRQ.\r\n");		
	}
#endif
	zinitix_debug_msg("request irq (irq = %d, pin = %d) \r\n", touch_dev->irq, touch_dev->int_gpio_num);

	touch_dev->work_proceedure = TS_NO_WORK;
	sema_init(&touch_dev->work_proceedure_lock, 1); 


	ret = request_threaded_irq(touch_dev->irq, NULL, ts_irq_thread,	IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "zinitix_ts", touch_dev);
	if (ret) {
		dev_err(&client->dev, "failed to request irq %d\n",		touch_dev->irq);
		goto err_request_irq;
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

	// Fops Device
	ret = misc_register(&touch_misc_device);
	if (ret) {
		zinitix_debug_msg("Fail to register touch misc device.\n");
	}

	if(device_create_file(touch_misc_device.this_device, &dev_attr_get_touch_test_raw_data)<0)		//sys/class/misc/touch_misc_fops/....
		printk("[TSP] Failed to create device file(%s)!\n", dev_attr_get_touch_test_raw_data.attr.name);	
	if(device_create_file(touch_misc_device.this_device, &dev_attr_tkey_reference)<0)	
		printk("[TSP] Failed to create device file(%s)!\n", dev_attr_tkey_reference.attr.name);	
	if(device_create_file(touch_misc_device.this_device, &dev_attr_touch_reference)<0)	
		printk("[TSP] Failed to create device file(%s)!\n", dev_attr_touch_reference.attr.name);	
	if(device_create_file(touch_misc_device.this_device, &dev_attr_delta)<0)	
		printk("[TSP] Failed to create device file(%s)!\n", dev_attr_delta.attr.name);	
	if(device_create_file(touch_misc_device.this_device, &dev_attr_scantime)<0)	
		printk("[TSP] Failed to create device file(%s)!\n", dev_attr_scantime.attr.name);	   
	if(device_create_file(touch_misc_device.this_device, &dev_attr_enter_testmode)<0)	
		printk("[TSP] Failed to create device file(%s)!\n", dev_attr_enter_testmode.attr.name);
	if(device_create_file(touch_misc_device.this_device, &dev_attr_enter_scanmode)<0)	
		printk("[TSP] Failed to create device file(%s)!\n", dev_attr_enter_scanmode.attr.name);        
	if(device_create_file(touch_misc_device.this_device, &dev_attr_enter_normalmode)<0)	
		printk("[TSP] Failed to create device file(%s)!\n", dev_attr_enter_normalmode.attr.name);
	if (device_create_file(touch_misc_device.this_device, &dev_attr_count_node) < 0)
		printk("[TSP] Failed to create device file(%s)!\n", dev_attr_count_node.attr.name);
	if (device_create_file(touch_misc_device.this_device, &dev_attr_chip_info) < 0)
		printk("[TSP] Failed to create device file(%s)!\n", dev_attr_chip_info.attr.name);
	if (device_create_file(touch_misc_device.this_device, &dev_attr_menu_sensitivity) < 0)
		printk("[TSP] Failed to create device file(%s)!\n", dev_attr_menu_sensitivity.attr.name);
	if (device_create_file(touch_misc_device.this_device, &dev_attr_back_sensitivity) < 0)
		printk("[TSP] Failed to create device file(%s)!\n", dev_attr_back_sensitivity.attr.name);
	if(device_create_file(touch_misc_device.this_device, &dev_attr_touch_ndata)<0)	
		printk("[TSP] Failed to create device file(%s)!\n", dev_attr_touch_ndata.attr.name);	
	if(device_create_file(touch_misc_device.this_device, &dev_attr_tkey_data)<0)	
		printk("[TSP] Failed to create device file(%s)!\n", dev_attr_tkey_data.attr.name);	
#endif	
#ifdef KEY_LED_CONTROL
//	init_timer(&g_led_timer);
//	g_led_timer.function = TouchKey_Led_TimerProc;

	if (device_create_file(touch_misc_device.this_device,  &dev_attr_brightness) < 0) {
		printk("[TSP] %s device_create_file fail dev_attr_touch_update\n",__FUNCTION__);
		pr_err("[TSP] Failed to create device file(%s)!\n",dev_attr_brightness.attr.name);
	}
#endif
return 0;

err_request_irq:
	input_unregister_device(touch_dev->input_dev);
err_input_register_device:
	input_free_device(touch_dev->input_dev);
//err_kthread_create_failed:	
err_input_allocate_device:	
	kfree(touch_dev);
err_alloc_dev_data:	
err_check_functionality:
//	touch_power_onoff(0);		//SEC_DRIVER
#ifdef CONFIG_TREBON_HW01
//PSJ	touch_io_ctrl_regulator(0);	//SEC_DRIVER
#endif
//PSJ	touch_ctrl_regulator(0);	//SEC_DRIVER
	touch_power_control(0); //PSJ
	mdelay(CHIP_POWER_OFF_DELAY);

	return ret;
}


static int zinitix_touch_remove(struct i2c_client *client)
{
	zinitix_touch_dev *touch_dev = i2c_get_clientdata(client);

	zinitix_debug_msg("zinitix_touch_remove+ \r\n");	
	down(&touch_dev->work_proceedure_lock);	
	touch_dev->work_proceedure = TS_REMOVE_WORK;

#if	ZINITIX_ESD_TIMER_INTERVAL	
	if(touch_dev->use_esd_timer)
	{	
		flush_work(&touch_dev->tmr_work);		
		ts_write_reg(touch_dev->client, ZINITIX_PERIODICAL_INTERRUPT_INTERVAL, 0);
		ts_esd_timer_stop(touch_dev);			
		zinitix_debug_msg(KERN_INFO " ts_esd_timer_stop\n");			
		destroy_workqueue(zinitix_tmr_workqueue);
	}
#endif
	if (touch_dev->irq) {
		free_irq(touch_dev->irq, touch_dev);
	}

#if USE_TEST_RAW_TH_DATA_MODE
	device_remove_file(touch_misc_device.this_device, &dev_attr_get_touch_test_raw_data);
	misc_deregister(&touch_misc_device);
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&touch_dev->early_suspend);
#endif

	if(gpio_is_valid(touch_dev->int_gpio_num))
		gpio_free(touch_dev->int_gpio_num);

#if	RESET_CONTROL
	if(gpio_is_valid(touch_dev->reset_gpio_num))
		gpio_free(touch_dev->reset_gpio_num);
#endif
	input_unregister_device(touch_dev->input_dev);
	input_free_device(touch_dev->input_dev);
	up(&touch_dev->work_proceedure_lock);	
	kfree(touch_dev);

	return 0;
}

static int __devinit zinitix_touch_init(void)
{
	printk("[TSP] zinitix_touch_init ! \n");
	return i2c_add_driver(&zinitix_touch_driver);    
}

static void __exit zinitix_touch_exit(void)
{
	i2c_del_driver(&zinitix_touch_driver);
}

static ssize_t firmware_sw_show(struct device *dev, struct device_attribute *attr, char *buf)
{
//	int ret;
	u16	sw_version;
//	u16 chip_reg_data_version;

	printk("[TSP] %s\n",__func__);

	//sw_version =   (u16)(((u16)m_firmware_data[FIRMWARE_VERSION_POS+1]<<8)|(u16)m_firmware_data[FIRMWARE_VERSION_POS]);			
	sw_version = (u16) (m_firmware_data[FIRMWARE_VERSION_POS+2]  | (m_firmware_data[FIRMWARE_VERSION_POS+3]<<8));
	printk("[TSP] phone ver firmware=%x\n", sw_version);
	return sprintf(buf, "%d\n", sw_version);
}

static ssize_t firmware_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	u16 phone_version;
	printk("[TSP] %s\n",__func__);

	/* get chip firmware version */
	//phone_version = global_touch_dev->cap_info.chip_firmware_version;
	if (ts_read_data(global_touch_dev->client, ZINITIX_DATA_VERSION_REG, (u8*)&phone_version, 2)<0);
	printk(KERN_INFO "zinitix touch chip firmware version = %x\r\n", phone_version);

	return sprintf(buf, "%d\n", phone_version);
}

static ssize_t tsp_StartCall_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk("%s, %d\n", __func__, IsDuringCall);

	IsDuringCall = 1;

	return sprintf(buf, "%d", IsDuringCall);
}

static ssize_t tsp_EndCall_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk("%s, %d\n", __func__, IsDuringCall);

	IsDuringCall = 0;

	return sprintf(buf, "%d", IsDuringCall);
}

int Is_DuringCall(void)
{
	return IsDuringCall;
}
static ssize_t tsp_xy_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk("%s, %d\n", __func__, IsdebugMatrix);

	if(IsdebugMatrix)
		IsdebugMatrix = 0;
	else
		IsdebugMatrix = 1;

	return sprintf(buf, "%d", IsdebugMatrix);
}
static ssize_t firmware_update(struct device *dev, struct device_attribute *attr, char *buf)
{
//	int ret = 0;	
	printk("[TSP] %s\n",__func__);
	IsfwUpdate = FWUP_ON;

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
	if(ts_init_touch(misc_touch_dev)==false)
	{
		enable_irq(misc_touch_dev->irq);
		misc_touch_dev->work_proceedure = TS_NO_WORK;
		up(&misc_touch_dev->work_proceedure_lock);										
		return -1;
	}

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
	IsfwUpdate = FWUP_OFF;
	printk("\n[TSP] success firmware_update\n");
	return 0;

}
static ssize_t firmware_update_status(struct device *dev, struct device_attribute *attr, char *buf)
{
//	int ret;
	printk("[TSP] %s\n",__func__);

	return sprintf(buf, "%d\n", IsfwUpdate);
}
#ifdef SEC_DRIVER 

// configurate things related to hw such as gpio, regulator levels, ...
void init_hw(void)
{
	int err = gpio_request(TSP_EN, "TSP_EN");
	int err1 = gpio_request(TSP_KEY_LED, "TSP_KEY_LED");
//	gpio_tlmm_config(GPIO_CFG(TSP_SCL, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA),GPIO_CFG_ENABLE);
//	gpio_tlmm_config(GPIO_CFG(TSP_SDA, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA),GPIO_CFG_ENABLE);
//	gpio_tlmm_config(GPIO_CFG(TSP_INT, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_16MA),GPIO_CFG_ENABLE);
//	gpio_tlmm_config(GPIO_CFG(TSP_EN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_16MA),GPIO_CFG_ENABLE);

//	gpio_set_value( TSP_SCL , 1 );
//	gpio_set_value( TSP_SDA , 1 );
	gpio_direction_output( TSP_EN , 1 ); 
#ifdef CONFIG_TREBON_HW01
//PSJ	touch_io_ctrl_regulator( 1 );
#endif
//PSJ	touch_ctrl_regulator( 1 );
	touch_power_control(1); //PSJ

	msleep(500);
}

int touch_power_onoff( int onoff )
{
	gpio_direction_output( TSP_EN , onoff ); 
	return 0;
}

EXPORT_SYMBOL(touch_power_onoff);
//EXPORT_SYMBOL(touchkey_avaliable);
#endif


module_init(zinitix_touch_init);
module_exit(zinitix_touch_exit);

MODULE_DESCRIPTION("touch-screen device driver using i2c interface");
MODULE_AUTHOR("sohnet <swjang@zinitix.com>");
MODULE_LICENSE("GPL");


