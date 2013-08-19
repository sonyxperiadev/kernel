/*****************************************************************************
* Copyright 2009-2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
 consent.
*****************************************************************************/

/*
 * Ambient Light and Proximity Sensor Driver
 *
 * This device controller is a slave on the I2C bus and is assigned an
 * address. This driver sets up the SOC as a I2C master and reads the slave
 * address to prox and als information.
 *
 * The driver uses the Linux input subsystem. User can access the touchscreen
 * data through the /dev/input/eventX node
 *
 */

#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/timer.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/isl290xx_common.h>
#include <linux/input.h>
#include <linux/miscdevice.h>
#include <linux/wakelock.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>

#define LOG_LEVEL_ERROR         (1U<<0)
#define LOG_LEVEL_INFO          (1U<<0)
#define LOG_LEVEL_DEBUG         (1U<<1)
#define LOG_LEVEL_FLOW          (1U<<2)

static int debug_mask = LOG_LEVEL_ERROR;
module_param_named(debug_mask, debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);
MODULE_PARM_DESC(debug,
		 "Debug level: Default: 0\n"
		 "\t\t\t  1: error info\n"
		 "\t\t\t  2: debug info\n" "\t\t\t  4: function flow\n");
#define pr_isl(debug_level_mask, fmt, args...) do {\
	if (debug_mask & LOG_LEVEL_##debug_level_mask)\
		pr_info("%s|%d: "fmt, KBUILD_BASENAME, __LINE__, ##args);\
} while (0)

#define ISL290XX_POLL_MODE 1
#define ISL290XX_USER_CALIBRATION 1
#define ISL290XX_INT_GPIO 89
#define ISL290XX_TAG        "[ISL290XX]"

#define ISL290XX_DEVICE_NAME		"isl290xx"
#define ISL290XX_DEVICE_ID                "isl290xx"
#define ISL290XX_ID_NAME_SIZE		10

#define ISL290XX_MAX_NUM_DEVICES		3
#define ISL290XX_MAX_DEVICE_REGS		10

#define ISL290XX_RESERVE_REG	0x0
#define ISL290XX_CFG_REG	0x1

#ifdef ISL29147_ENABLE
#define ISL290XX_PROX_EN_BIT    0x20 /*[5]*/
#define ISL290XX_PROX_SLP_BIT   0x1c /*[4:2]*/
#define ISL290XX_PROX_DR_BIT    0x03/*[1:0]*/
/*config 1*/
#define ISL290XX_CFG1_REG 0x02
#define ISL290XX_INT_ALG 0x80 /*[7]*/
#define ISL290XX_PROX_OFFSET 0x78 /*[6:3]*/
#define ISL290XX_ALS_EN_BIT     0x04 /*[2]*/
#define ISL290XX_ALS_RANGE_BIT  0x03 /*[1:0]*/
/*config 2*/
#define ISL290XX_CFG2_REG 0x03
#define ISL290XX_ALSIRCOMP 0x10
/*INT config*/
#define ISL290XX_INT_REG	0x4
#define ISL290XX_INT_PROX_BIT      0x80
#define ISL290XX_INT_PROX_PRST_BIT 0x60
#define ISL290XX_INT_ALS_BIT       0x08
#define ISL290XX_INT_ALS_PRST_BIT  0x06
#define ISL290XX_INT_CTRL_BIT      0x01
#define ISL290XX_INT_PWR_FAIL	0x10

#define ISL290XX_PROX_TH_L	0x5
#define ISL290XX_PROX_TH_H	0x6

#define ISL290XX_ALSIR_TH_L	0x7
#define ISL290XX_ALSIR_TH_HL	0x8
#define ISL290XX_ALSIR_TH_H	0x9

#define ISL290XX_PROX_DATA	0xa

#define ISL290XX_ALSIR_DATA_L	0xc
#define ISL290XX_ALSIR_DATA_H	0xb

#define ISL290XX_PROX_AMBIR	0x0d
#else
#define ISL290XX_PROX_EN_BIT    0x80
#define ISL290XX_PROX_SLP_BIT   0x70
#define ISL290XX_PROX_DR_BIT    0x08
#define ISL290XX_ALS_EN_BIT     0x04
#define ISL290XX_ALS_RANGE_BIT  0x02
#define ISL290XX_ALSIR_MODE_BIT 0x01
#define ISL290XX_SENSOR_ALL_OFF 0x00
#define ISL290XX_ALS_ON         (ISL290XX_ALS_EN_BIT | ISL290XX_ALS_RANGE_BIT)
#define ISL290XX_ALS_MASK       (ISL290XX_ALS_EN_BIT | ISL290XX_ALS_RANGE_BIT \
				| ISL290XX_ALSIR_MODE_BIT)
#define ISL290XX_PROX_MASK      (ISL290XX_PROX_EN_BIT | ISL290XX_PROX_SLP_BIT \
				| ISL290XX_PROX_DR_BIT)
#define ISL290XX_PROX_ON        (0xD0)

#define ISL290XX_INT_REG		0x2
#define ISL290XX_INT_PROX_BIT      0x80
#define ISL290XX_INT_PROX_PRST_BIT 0x60
#define ISL290XX_INT_ALS_BIT       0x08
#define ISL290XX_INT_ALS_PRST_BIT  0x06
#define ISL290XX_INT_CTRL_BIT      0x01
#define ISL290XX_INT_ALL_OFF       0x00
#define ISL290XX_ALS_INT_ON         (0x00)
#define ISL290XX_ALS_INT_MASK       (0x0E)
#define ISL290XX_PROX_INT_ON        (0x20)
#define ISL290XX_PROX_INT_MASK      (0xE0)

#define ISL290XX_PROX_TH_L	0x3
#define ISL290XX_PROX_TH_H	0x4

#define ISL290XX_ALSIR_TH_L	0x5
#define ISL290XX_ALSIR_TH_HL	0x6
#define ISL290XX_ALSIR_TH_H	0x7

#define ISL290XX_PROX_DATA	0x8

#define ISL290XX_ALSIR_DATA_L	0x9
#define ISL290XX_ALSIR_DATA_H	0xa
#endif
#define ISL290XX_TEST1		0x0e
#define ISL290XX_TEST2		0x0f

#define ISL290XX_HIGH_LUX_RANGE 2000
#define ISL290XX_LOW_LUX_RANGE 125
#define ISL290XX_LOW_TO_HIGH_COUNTS 1800
#define ISL290XX_HIGH_TO_LOW_COUNTS 500
#define	ISL290XX_MAX_LUX	65535000
#define ISL290XX_FILTER_DEPTH		3
#define THRES_LO_TO_HI_RATIO  (4/5)
#ifdef ISL290XX_USER_CALIBRATION
static int isl290xx_offset;
#endif

static const struct of_device_id isl290xx_of_match[] = {
	{.compatible = "bcm,isl290xx",},
	{},
}

MODULE_DEVICE_TABLE(of, isl290xx_of_match);

static int isl290xx_probe(struct i2c_client *clientp,
			  const struct i2c_device_id *idp);
static int isl290xx_remove(struct i2c_client *client);
static int isl290xx_open(struct inode *inode, struct file *file);
static int isl290xx_release(struct inode *inode, struct file *file);
static long isl290xx_ioctl(struct file *file, unsigned int cmd,
			   unsigned long arg);
static int isl290xx_read(struct file *file, char *buf, size_t count,
			 loff_t *ppos);
static int isl290xx_write(struct file *file, const char *buf, size_t count,
			  loff_t *ppos);
static loff_t isl290xx_llseek(struct file *file, loff_t offset, int orig);
#ifdef ISL290XX_POLL_MODE
static struct timer_list prox_poll_timer;
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
static void isl290xx_early_suspend(struct early_suspend *handler);
static void isl290xx_late_resume(struct early_suspend *handler);
#endif
static int prv_isl290xx_set_bit(u8 reg, u8 bit_mask, u8 value);
static int prv_isl290xx_get_lux(void);
static int prv_isl290xx_prox_poll(struct isl290xx_prox_info_s *prxp);
#ifdef ISL290XX_USER_CALIBRATION
static void prv_isl290xx_prox_cali(void);
#endif

#ifdef ISL29147_ENABLE
static int isl290xx_set_bits(u8 reg, u8 bit_mask, u8 value);
#endif
static void prv_isl290xx_work_func(struct work_struct *w);
static void prv_isl290xx_report_value(int mask);
static int prv_isl290xx_calc_distance(int value);
static int prv_isl290xx_ctrl_lp(int mask);
static void prv_isl290xx_reset(void);
static int light_on;
static int prox_on;

struct isl290xx_alsprox_data_s {
	struct input_dev *input_dev;
};

static struct isl290xx_alsprox_data_s *light;
static struct isl290xx_alsprox_data_s *proximity;
static dev_t isl290xx_device_number;
static int isl290xx_pls_irq_num;

struct class *isl290xx_class;

static struct i2c_device_id isl290xx_idt[] = {
	{ISL290XX_DEVICE_ID, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, isl290xx_idt);

struct i2c_client *isl290xx_my_clientp;
struct i2c_client *isl290xx_bad_clientp[ISL290XX_MAX_NUM_DEVICES];
static struct i2c_driver isl290xx_driver = {
	.driver = {
		   .owner = THIS_MODULE,
		   .name = "isl290xx",
		   .of_match_table = isl290xx_of_match,
		   },
	.id_table = isl290xx_idt,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = isl290xx_suspend,
	.resume	= isl290xx_resume,
#endif
	.probe = isl290xx_probe,
	.remove = __devexit_p(isl290xx_remove),
};


struct isl290xx_data_t {
	struct i2c_client *client;
	struct cdev cdev;
	unsigned int addr;
	char isl290xx_id;
	char isl290xx_name[ISL290XX_ID_NAME_SIZE];
	struct mutex update_lock;
	struct mutex lock;
	struct mutex prox_lock;
	struct wake_lock isl290xx_wake_lock;
#ifdef ISL290XX_POLL_MODE
	struct delayed_work prox_poll_work;
#endif
	unsigned long last_updated;
	struct work_struct isl290xx_work;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend	early_suspend;
#endif
	struct mutex proximity_calibrating;
} *isl290xx_data_tp;


static const struct file_operations ISL_fops = {
	.owner = THIS_MODULE,
	.open = isl290xx_open,
	.release = isl290xx_release,
	.read = isl290xx_read,
	.write = isl290xx_write,
	.llseek = isl290xx_llseek,
	.unlocked_ioctl = isl290xx_ioctl,
};

struct isl290xx_cfg_s *isl290xx_cfgp;

static u16 als_intr_threshold_hi_param;
static u16 als_intr_threshold_lo_param;
int g_isl290xx_lux;

struct isl290xx_prox_info_s isl290xx_prox_cal_info[20];
struct isl290xx_prox_info_s isl290xx_prox_cur_info;
struct isl290xx_prox_info_s *isl290xx_prox_cur_infop = &isl290xx_prox_cur_info;

static int device_released;
int isPsensorLocked;

#ifdef ISL290XX_POLL_MODE
static int prox_poll_count = 1;
#endif

struct isl290xx_reg {
	const char *name;
	u8 reg;

} isl290xx_regs[] = {
#ifdef ISL29147_ENABLE
	{"DEVICEID",	ISL290XX_RESERVE_REG},
	{"CONFIG0",	ISL290XX_CFG_REG},
	{"CONFIG1",	ISL290XX_CFG1_REG},
	{"CONFIG2",	ISL290XX_CFG2_REG},
#else
	{"CONFIGURE",	ISL290XX_CFG_REG},
#endif
	{"INTERRUPT",		ISL290XX_INT_REG},
	{"PROX_LT",		ISL290XX_PROX_TH_L},
	{"PROX_HT",		ISL290XX_PROX_TH_H},
	{"ALS_IR_TH1",		ISL290XX_ALSIR_TH_L},
	{"ALS_IR_TH2",		ISL290XX_ALSIR_TH_HL},
	{"ALS_IR_TH3",		ISL290XX_ALSIR_TH_H},
	{"PROX_DATA",		ISL290XX_PROX_DATA},
	{"ALS_IR_DT1",		ISL290XX_ALSIR_DATA_L},
	{"ALS_IR_DT2",		ISL290XX_ALSIR_DATA_H},
#ifdef ISL29147_ENABLE
	{"PROX_AMBIR",	ISL290XX_PROX_AMBIR},
	{"CONFIG3",	ISL290XX_TEST2},
#else
	{"TEST1",	ISL290XX_TEST1},
	{"TEST2",		ISL290XX_TEST2},
#endif
};

struct time_scale_factor {
	u16 numerator;
	u16 denominator;
	u16 saturation;
};
struct time_scale_factor ISL_TritonTime = { 1, 0, 0 };
struct time_scale_factor *ISL_lux_timep = &ISL_TritonTime;

static int lux_history[ISL290XX_FILTER_DEPTH] = {
		-ENODATA, -ENODATA, -ENODATA };

struct isl290xx_prox_data_t {
	u16 ratio;
	u16 hi;
	u16 lo;
};
struct isl290xx_prox_data_t isl290xx_prox_data[] = {
	{1, 50, 20},
	{3, 20, 16},
	{6, 18, 14},
	{10, 16, 16},
	{0, 0, 0}
};
struct isl290xx_prox_data_t *isl290xx_prox_tablep = isl290xx_prox_data;

static irqreturn_t isl290xx_interrupt(int irq, void *data)
{
	schedule_work(&isl290xx_data_tp->isl290xx_work);

	return IRQ_HANDLED;
}


static void prv_isl290xx_work_func(struct work_struct *w)
{

	int ret = 0;
	u16 status = 0;
	status =
	    i2c_smbus_read_byte_data(isl290xx_data_tp->client,
				     ISL290XX_INT_REG);
	if (status < 0)
		goto read_reg_fail;

	if (((status & ISL290XX_INT_ALS_BIT) != 0)) {
		status = status & (~ISL290XX_INT_ALS_BIT);
		g_isl290xx_lux = prv_isl290xx_get_lux();
		prv_isl290xx_report_value(0);
	}

	if ((status & ISL290XX_INT_PROX_BIT) != 0) {
		status = status & (~ISL290XX_INT_PROX_BIT);
		pr_debug("isl290xx: data %d, hi=%d, lo=%d\n",
			 isl290xx_prox_cur_infop->prox_data,
			 isl290xx_cfgp->prox_threshold_hi,
			 isl290xx_cfgp->prox_threshold_lo);
		ret = prv_isl290xx_prox_poll(isl290xx_prox_cur_infop);
		if (ret < 0)
			pr_isl(ERROR, "get prox poll failed\n");
		if (isl290xx_prox_cur_infop->prox_data >
		    isl290xx_cfgp->prox_threshold_hi) {
			isl290xx_prox_cur_infop->prox_event = 1;
#ifndef ISL290XX_POLL_MODE
		} else if (isl290xx_prox_cur_infop->prox_data <
			   isl290xx_cfgp->prox_threshold_lo) {
			isl290xx_prox_cur_infop->prox_event = 0;
#else
			prox_poll_count = 1;
			mod_timer(&prox_poll_timer, jiffies + HZ / 1000);
#endif
		}
		prv_isl290xx_report_value(1);
	}

read_reg_fail:
	ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
		ISL290XX_INT_REG,
		status&(~ISL290XX_INT_PROX_BIT | ~ISL290XX_INT_ALS_BIT));
	if (ret  < 0)
		pr_isl(ERROR,
		       "ISL290XX:failed in clear als interrupt\n");
	ret = gpio_get_value(isl290xx_pls_irq_num);
	if (ret == 0) {
		pr_isl(ERROR, "clear intrrupts failed\n");
		i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_INT_REG,
			status&(~ISL290XX_INT_PROX_BIT
			| ~ISL290XX_INT_ALS_BIT));
	}
	return;
}

static int prv_isl290xx_calc_distance(int value)
{
	int temp = 0;
	if (isl290xx_prox_cur_infop->prox_event == 1)
		temp = 0;
	else if (isl290xx_prox_cur_infop->prox_event == 0)
		temp = 5;
	return temp;
}

static void prv_isl290xx_report_value(int mask)
{
	struct isl290xx_prox_info_s *val = isl290xx_prox_cur_infop;
	int lux_val = g_isl290xx_lux;
	int dist;
	mutex_lock(&isl290xx_data_tp->prox_lock);
	if (mask == 0) {
		input_report_abs(light->input_dev, ABS_MISC, lux_val);
		input_sync(light->input_dev);
	}

	if (mask == 1) {
		dist = prv_isl290xx_calc_distance(val->prox_data);
		input_report_abs(proximity->input_dev, ABS_DISTANCE, dist);
		input_sync(proximity->input_dev);
	}
	mutex_unlock(&isl290xx_data_tp->prox_lock);
}

static int __init isl290xx_init(void)
{
	int ret = 0;
	ret = alloc_chrdev_region(&isl290xx_device_number, 0,
		ISL290XX_MAX_NUM_DEVICES,
		ISL290XX_DEVICE_NAME);
	if (ret  < 0) {
		pr_isl(ERROR, "alloc region err\n");
		return ret;
	}
	isl290xx_class = class_create(THIS_MODULE, ISL290XX_DEVICE_NAME);
	isl290xx_data_tp = kmalloc(sizeof(struct isl290xx_data_t), GFP_KERNEL);
	if (!isl290xx_data_tp) {
		pr_isl(ERROR, "kmalloc for struct isl290xx_data_t\n");
		return -ENOMEM;
	}
	memset(isl290xx_data_tp, 0, sizeof(struct isl290xx_data_t));
	cdev_init(&isl290xx_data_tp->cdev, &ISL_fops);
	isl290xx_data_tp->cdev.owner = THIS_MODULE;
	ret = cdev_add(&isl290xx_data_tp->cdev, isl290xx_device_number, 1);
	if (ret < 0) {
		pr_isl(ERROR, "cdev_add() failed\n");
		return ret;
	}
	ret = i2c_add_driver(&isl290xx_driver);
	if (ret) {
		pr_isl(ERROR, "i2c_add_driver() failed %d\n", ret);
		return ret;
	}
	return ret;
}

static void __exit isl290xx_exit(void)
{
	i2c_del_driver(&isl290xx_driver);
	unregister_chrdev_region(isl290xx_device_number,
				 ISL290XX_MAX_NUM_DEVICES);
	device_destroy(isl290xx_class, MKDEV(MAJOR(isl290xx_device_number), 0));
	cdev_del(&isl290xx_data_tp->cdev);
	class_destroy(isl290xx_class);
	mutex_destroy(&isl290xx_data_tp->proximity_calibrating);
	mutex_destroy(&isl290xx_data_tp->update_lock);
	mutex_destroy(&isl290xx_data_tp->lock);
	mutex_destroy(&isl290xx_data_tp->prox_lock);
	kfree(isl290xx_data_tp);
}

static ssize_t als_enable_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int enable;
	int ret;

	ret = kstrtoint(buf, 0, &enable);
	if (ret)
		return ret;
	if (enable) {
		if (light_on)
			pr_isl(INFO,
			       "light already on.\n");
		ret = prv_isl290xx_ctrl_lp(0x10);
		if (ret >= 0) {
			light_on = 1;
			pr_isl(INFO, "lux=%d\n",
			       g_isl290xx_lux);
		}
	} else {
		if (light_on == 0)
			pr_isl(INFO,
			       "light already off\n");

		ret = prv_isl290xx_ctrl_lp(0x20);
		if (ret >= 0) {
			light_on = 0;
			pr_isl(INFO, "als off\n");
		}
	}

	return count;
}

static DEVICE_ATTR(als_enable, 0644, NULL, als_enable_store);

static ssize_t prox_enable_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{

	int enable;
	int ret;

	ret = kstrtoint(buf, 0, &enable);
	if (ret)
		return ret;
	if (enable) {
		if (prox_on)
			pr_isl(INFO,
			       "ISL290XX_IOCTL_PROX_ON: prox already on.\n");

		isl290xx_prox_cur_infop->prox_event = 0;
		isl290xx_prox_cur_infop->prox_clear = 0;
		isl290xx_prox_cur_infop->prox_data = 0;
		ret = prv_isl290xx_ctrl_lp(0x01);
		if (ret >= 0) {
			prox_on = 1;
			if (isPsensorLocked == 0) {
				wake_lock\
				(&isl290xx_data_tp->isl290xx_wake_lock);
				isPsensorLocked = 1;
			}
			pr_isl(INFO, "ISL290XX_IOCTL_PROX_ON\n");
		}
	} else {
		if (prox_on == 0) {
			pr_isl(INFO,
			       "ISL290XX_IOCTL_PROX_OFF: prox already off.\n");
		}
		ret = prv_isl290xx_ctrl_lp(0x02);
		if (ret >= 0) {
			prox_on = 0;
			if (isPsensorLocked == 1) {
				wake_unlock
				(&isl290xx_data_tp->isl290xx_wake_lock);
				isPsensorLocked = 0;
			}
			pr_isl(INFO, "ISL290XX_IOCTL_PROX_OFF\n");
		}
	}
	return count;
}

static DEVICE_ATTR(prox_enable, 0644, NULL, prox_enable_store);

#ifdef ISL290XX_USER_CALIBRATION
static ssize_t prox_cali_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	int enable;
	int ret;
	ret = kstrtoint(buf, 0, &enable);
	if (ret)
		return ret;
	if (enable)
		prv_isl290xx_prox_cali();
	return count;
}
static DEVICE_ATTR(prox_cali, 0644, NULL, prox_cali_store);

static ssize_t isl290xx_get_offset(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%d\n", isl290xx_offset);
}

static ssize_t isl290xx_set_offset(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	int x;
	int hi;
	int lo;
	int i;
	int err = -EINVAL;
	err = sscanf(buf, "%d", &x);
	if (err != 1) {
		pr_err("invalid parameter number: %d\n", err);
		return err;
	}
	isl290xx_offset = x;
	hi = isl290xx_cfgp->prox_threshold_hi_def + isl290xx_offset
			- isl290xx_cfgp->prox_offset;
	lo = isl290xx_cfgp->prox_threshold_lo_def + isl290xx_offset
			- isl290xx_cfgp->prox_offset;
	pr_isl(INFO,
		"isl290xx_offset=%d,prox_offset=%d, hi =%d, lo=%d\n",
		isl290xx_offset, isl290xx_cfgp->prox_offset, hi, lo);
	if (hi > 0xfe || lo > 0xfe ||
		(isl290xx_offset-isl290xx_cfgp->prox_offset) < 0) {
		isl290xx_offset = 0;
		pr_isl(ERROR, "isl290xx sw cali failed\n");
	}
	else {
		if (lo > isl290xx_offset) {
			isl290xx_cfgp->prox_threshold_hi = hi;
			isl290xx_cfgp->prox_threshold_lo = lo;
		} else {
			for (i = 0; i < 100; i++) {
				hi++;
				lo++;
				if (lo > isl290xx_offset) {
					isl290xx_cfgp->prox_threshold_hi
						= hi + 1;
					isl290xx_cfgp->prox_threshold_lo
						= lo + 1;
					break;
				}
			}
		}
	}
	return count;
}
static DEVICE_ATTR(offset,  S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	isl290xx_get_offset, isl290xx_set_offset);
#endif

static ssize_t isl290xx_registers_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)

{
	unsigned i, n, reg_count;
	u8 value;
	reg_count = sizeof(isl290xx_regs) / sizeof(isl290xx_regs[0]);
	for (i = 0, n = 0; i < reg_count; i++) {
		value = i2c_smbus_read_byte_data(isl290xx_data_tp->client,
				isl290xx_regs[i].reg);
		n += scnprintf(buf + n, PAGE_SIZE - n,
			"%-20s = 0x%02X\n",
			isl290xx_regs[i].name,
			value);
	}
	return n;
}

static ssize_t isl290xx_registers_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{

	unsigned i, reg_count, value;
	int ret;
	char name[30];
	if (count >= 30)
		return -EFAULT;

	if (sscanf(buf, "%30s %x", name, &value) != 2) {
		pr_isl(ERROR, "input invalid\n");
		return -EFAULT;
	}
	reg_count = sizeof(isl290xx_regs) / sizeof(isl290xx_regs[0]);
	for (i = 0; i < reg_count; i++) {
		if (!strcmp(name, isl290xx_regs[i].name)) {
			switch (isl290xx_regs[i].reg) {
			case ISL290XX_PROX_TH_L:
				isl290xx_cfgp->prox_threshold_lo = value;
			break;
			case ISL290XX_PROX_TH_H:
				isl290xx_cfgp->prox_threshold_hi = value;
			break;

			default:
			break;
			}
			ret = i2c_smbus_write_byte_data(
				isl290xx_data_tp->client, isl290xx_regs[i].reg,
			value);

			if (ret) {
				pr_isl(ERROR,
					"Failed to write register %s\n",
					name);
				return -EFAULT;
			}
			return count;
		}
	}
	pr_isl(INFO, "no such register %s\n", name);
	return -EFAULT;
}

static DEVICE_ATTR(registers, 0644, isl290xx_registers_show,
			isl290xx_registers_store);

static struct attribute *isl290xx_ctrl_attr[] = {
	&dev_attr_als_enable.attr,
	&dev_attr_prox_enable.attr,
#ifdef ISL290XX_USER_CALIBRATION
	&dev_attr_prox_cali.attr,
	&dev_attr_offset.attr,
#endif
	&dev_attr_registers.attr,
	NULL,
};

static struct attribute_group isl290xx_ctrl_attr_grp = {
	.attrs = isl290xx_ctrl_attr,
};


#ifdef ISL290XX_POLL_MODE
void isl290xx_poll_timer_func(unsigned long data)
{
	struct isl290xx_data_t *dd;
	long delay = 2;
	dd = (struct isl290xx_data_t *)data;
	schedule_delayed_work(&dd->prox_poll_work, HZ / 1000);
	mod_timer(&prox_poll_timer, jiffies + delay);
	prox_poll_count++;
}

static void isl290xx_prox_poll_work_f(struct work_struct *work)
{
	int ret = 0;
	ret = prv_isl290xx_prox_poll(isl290xx_prox_cur_infop);
	if (ret < 0)
		pr_isl(ERROR, "get prox poll failed\n");
	if (isl290xx_prox_cur_infop->prox_data >
		isl290xx_cfgp->prox_threshold_hi) {
		isl290xx_prox_cur_infop->prox_event = 1;
	} else if (isl290xx_prox_cur_infop->prox_data <
			isl290xx_cfgp->prox_threshold_lo) {
		isl290xx_prox_cur_infop->prox_event = 0;
		if (prox_poll_count == 10)
			del_timer(&prox_poll_timer);
	}
	prv_isl290xx_report_value(1);
}
#endif

static int isl290xx_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int ret = 0;
#if 0
	cancel_work_sync(&isl290xx_data_tp->isl290xx_work);
#ifdef ISL290XX_POLL_MODE
	del_timer_sync(&prox_poll_timer);
	cancel_delayed_work_sync(&isl290xx_data_tp->prox_poll_work);
#endif
#endif
	return ret;
}

static int isl290xx_resume(struct i2c_client *client)
{

	int ret = 0;
#if 0
#ifdef ISL290XX_POLL_MODE
	mod_timer(&prox_poll_timer, jiffies + HZ/1000);
#endif
#endif
	return ret;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void isl290xx_early_suspend(struct early_suspend *handler)
{
	struct isl290xx_data_t *isl290xx_data;

	isl290xx_data = container_of(handler, struct isl290xx_data_t,
						early_suspend);
	isl290xx_suspend(isl290xx_data->client, PMSG_SUSPEND);
}

static void isl290xx_late_resume(struct early_suspend *handler)
{
	struct isl290xx_data_t *isl290xx_data;

	isl290xx_data = container_of(handler, struct isl290xx_data_t, early_suspend);
	isl290xx_resume(isl290xx_data->client);
}
#endif

static int isl290xx_probe(struct i2c_client *clientp,
			  const struct i2c_device_id *idp)
{
	int ret;
	int i;
	struct device_node *np;
	unsigned char buf[ISL290XX_MAX_DEVICE_REGS];
	u32 val;
#ifdef ISL290XX_USER_CALIBRATION
	isl290xx_offset = 0;
#endif
	val = 0;
        light_on = 0;
        prox_on = 0;
	isPsensorLocked = 0;
	als_intr_threshold_hi_param = 0;
	als_intr_threshold_lo_param = 0x0fff;
	g_isl290xx_lux = 0;
	device_released = 0;
	ret = 0;
	i = 0;
	pr_info("isl290xx_probe+\n");
	if (!i2c_check_functionality
	    (clientp->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		pr_isl(ERROR,
		       "i2c smbus byte data functions unsupported\n");
		return - EOPNOTSUPP;
	}
	if (!i2c_check_functionality
	    (clientp->adapter, I2C_FUNC_SMBUS_WORD_DATA)) {
		pr_isl(ERROR,
		       "i2c smbus word data functions unsupported\n");
	}
	if (!i2c_check_functionality
	    (clientp->adapter, I2C_FUNC_SMBUS_BLOCK_DATA)) {
		pr_isl(ERROR,
		       "i2c smbus block data functions unsupported\n");
	}
	isl290xx_data_tp->client = clientp;
	i2c_set_clientdata(clientp, isl290xx_data_tp);

	for (i = 1; i < ISL290XX_MAX_DEVICE_REGS; i++)
		buf[i] = i2c_smbus_read_byte_data(isl290xx_data_tp->client, i);
	wake_lock_init(&isl290xx_data_tp->isl290xx_wake_lock,
			WAKE_LOCK_SUSPEND, "isl290xx-wake-lock");

	device_create(isl290xx_class, NULL,
			MKDEV(MAJOR(isl290xx_device_number), 0),
			&isl290xx_driver,
			"isl290xx");
	ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
		ISL290XX_ALSIR_TH_L, 0);
	if (ret < 0) {
		pr_isl(ERROR,
		       "control reg failed in isl290xx_probe()\n");
		return ret;
	}
	ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
		ISL290XX_ALSIR_TH_HL, 0);
	if (ret < 0) {
		pr_isl(ERROR,
		       "control reg failed in isl290xx_probe()\n");
		return ret;
	}
	ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_ALSIR_TH_H, 0);
	if (ret < 0) {
		pr_isl(ERROR,
		       " control reg failed in isl290xx_probe()\n");
		return ret;
	}
#ifdef ISL29147_ENABLE
	ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
		ISL290XX_CFG_REG,
		0);
	if (ret < 0) {
		pr_isl(ERROR,
		       "control reg failed in isl290xx_probe()\n");
		return ret;
	}
	ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
		ISL290XX_CFG1_REG, 0);
	if (ret < 0) {
		pr_isl(ERROR, "control reg failed in isl290xx_probe()\n");
		return ret;
	}

	ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_INT_REG, 0x10);
	if (ret  < 0) {
		pr_isl(ERROR,
		       "control reg failed in isl290xx_probe()\n");
		return ret;
	}

#else
	ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
		ISL290XX_CFG_REG,
		ISL290XX_SENSOR_ALL_OFF);
	if (ret < 0) {
		pr_isl(ERROR,
		       "control reg failed in isl290xx_probe()\n");
		return ret;
	}
	ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
		ISL290XX_INT_REG,
		ISL290XX_INT_ALL_OFF);
	if (ret  < 0) {
		pr_isl(ERROR,
		       "control reg failed in isl290xx_probe()\n");
		return ret;
	}
#endif
	INIT_WORK(&isl290xx_data_tp->isl290xx_work, prv_isl290xx_work_func);
	mutex_init(&isl290xx_data_tp->proximity_calibrating);
	if (!clientp->dev.platform_data) {
		np = isl290xx_data_tp->client->dev.of_node;
		ret = of_property_read_u32(np, "gpio-irq-pin", &val);
		clientp->irq = val;
		isl290xx_pls_irq_num = val;
	}
	pr_isl(INFO, "ISL290XX use gpio %d\n", clientp->irq);
	ret = request_threaded_irq(gpio_to_irq(clientp->irq),
			NULL, &isl290xx_interrupt,
			IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
			isl290xx_data_tp->isl290xx_name,
			isl290xx_prox_cur_infop);
	if (ret) {
		pr_isl(INFO, "ISL290XX request interrupt failed\n");
		return ret;
	}

	strlcpy(clientp->name, ISL290XX_DEVICE_ID, I2C_NAME_SIZE);
	strlcpy(isl290xx_data_tp->isl290xx_name, ISL290XX_DEVICE_ID,
		ISL290XX_ID_NAME_SIZE);

	mutex_init(&isl290xx_data_tp->update_lock);
	mutex_init(&isl290xx_data_tp->lock);
	mutex_init(&isl290xx_data_tp->prox_lock);
	isl290xx_cfgp =
	     kmalloc(sizeof(struct isl290xx_cfg_s), GFP_KERNEL);
	if (!isl290xx_cfgp) {
		pr_isl(ERROR,
		       "kmalloc for struct isl290xx_cfg_s failed)\n");
		return - ENOMEM;
	}
	if (!clientp->dev.platform_data) {
		ret = of_property_read_u32(np,
			"prox_threshold_hi_param", &val);
		isl290xx_cfgp->prox_threshold_hi = val;
		isl290xx_cfgp->prox_threshold_hi_def = val;
		ret = of_property_read_u32(np,
			"prox_threshold_lo_param", &val);
		isl290xx_cfgp->prox_threshold_lo = val;
		isl290xx_cfgp->prox_threshold_lo_def = val;
		ret = of_property_read_u32(np,
			"aps_config_param", &val);
		isl290xx_cfgp->aps_config = val;
#ifdef ISL29147_ENABLE
		ret = of_property_read_u32(np,
			"aps_config1_param", &val);
		isl290xx_cfgp->aps_config1 = val;
		ret = of_property_read_u32(np,
			"aps_config2_param", &val);
		isl290xx_cfgp->aps_config2 = val;
#endif
		ret = of_property_read_u32(np,
			"aps_intr_param", &val);
		isl290xx_cfgp->aps_intr = val;
#ifdef ISL290XX_USER_CALIBRATION
		ret = of_property_read_u32(np,
			"prox_offset_param", &val);
		isl290xx_cfgp->prox_offset = val;
		ret = of_property_read_u32(np,
			"prox_boot_cali", &val);
		isl290xx_cfgp->prox_boot_cali = val;
#endif
	}
	light = kzalloc(sizeof(struct isl290xx_alsprox_data_s), GFP_KERNEL);
	if (!light) {
		ret = -ENOMEM;
		goto exit_alloc_data_failed;
	}
	proximity = kzalloc(sizeof(struct isl290xx_alsprox_data_s), GFP_KERNEL);
	if (!proximity) {
		ret = -ENOMEM;
		goto exit_alloc_data_failed;
	}

	light->input_dev = input_allocate_device();
	if (!light->input_dev) {
		ret = -ENOMEM;
		pr_isl(ERROR,
		       "Failed to allocate light input device\n");
		goto exit_input_dev_alloc_failed;
	}
	proximity->input_dev = input_allocate_device();
	if (!proximity->input_dev) {
		ret = -ENOMEM;
		pr_isl(ERROR,
		       "Failed to allocate prox input device\n");
		goto exit_input_dev_alloc_failed;
	}

	/* lux */
	set_bit(EV_ABS, light->input_dev->evbit);
	input_set_abs_params(light->input_dev, ABS_MISC, 0, 65535, 0, 0);
	light->input_dev->name = "lightsensor";
	/* prox */
	set_bit(EV_ABS, proximity->input_dev->evbit);
	input_set_abs_params(proximity->input_dev, ABS_DISTANCE, 0, 65535, 0,
			     0);
	proximity->input_dev->name = "proximity";
	proximity->input_dev->dev.parent = &isl290xx_data_tp->client->dev;
	ret = input_register_device(light->input_dev);
	if (ret) {
		pr_isl(ERROR, "Unable to register input device: %s\n",
		       light->input_dev->name);
		goto exit_input_register_device_failed;
	}
	ret = input_register_device(proximity->input_dev);
	if (ret) {
		pr_isl(ERROR, "Unable to register input device: %s\n",
		       proximity->input_dev->name);
		goto exit_input_register_device_failed;
	}
#ifdef ISL290XX_POLL_MODE
	INIT_DELAYED_WORK(&isl290xx_data_tp->prox_poll_work,
				isl290xx_prox_poll_work_f);
	setup_timer(&prox_poll_timer, isl290xx_poll_timer_func,
				(long)isl290xx_data_tp);
#endif
	ret = sysfs_create_group(&clientp->dev.kobj, &isl290xx_ctrl_attr_grp);
	if (0 != ret) {
		goto exit_input_register_device_failed;
                pr_isl(ERROR, "sysfs create debug node fail, ret: %d\n", ret);
	}
#ifdef CONFIG_HAS_EARLYSUSPEND
	isl290xx_data_tp->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	isl290xx_data_tp->early_suspend.suspend = isl290xx_early_suspend;
	isl290xx_data_tp->early_suspend.resume = isl290xx_late_resume;
	register_early_suspend(&isl290xx_data_tp->early_suspend);
#endif
	pr_info("isl probe ---\n");
	return ret;
exit_input_register_device_failed:
	sysfs_remove_group(&clientp->dev.kobj, &isl290xx_ctrl_attr_grp);
	if (light->input_dev)
		input_free_device(light->input_dev);
	if (proximity->input_dev)
		input_free_device(proximity->input_dev);
exit_input_dev_alloc_failed:
exit_alloc_data_failed:
	kfree(light);
	kfree(proximity);
	return ret;
}

static int __devexit isl290xx_remove(struct i2c_client *client)
{
	int ret = 0;
	sysfs_remove_group(&client->dev.kobj, &isl290xx_ctrl_attr_grp);
#ifdef ISL290XX_POLL_MODE
	del_timer(&prox_poll_timer);
#endif
	return ret;
}

static int isl290xx_open(struct inode *inode, struct file *file)
{
	struct isl290xx_data_t *isl290xx_data_tp;
	int ret = 0;
	device_released = 0;
	isl290xx_data_tp =
	    container_of(inode->i_cdev, struct isl290xx_data_t, cdev);
	ret = strcmp(isl290xx_data_tp->isl290xx_name, ISL290XX_DEVICE_ID);
	if (ret != 0) {
		pr_isl(ERROR,
		       "device name incorrect, get %s\n",
		       isl290xx_data_tp->isl290xx_name);
		ret = -ENODEV;
	}
	return ret;
}

static int isl290xx_release(struct inode *inode, struct file *file)
{
	struct isl290xx_data_t *isl290xx_data_tp;
	int ret = 0;

	device_released = 1;
	prox_on = 0;
	isl290xx_data_tp =
	    container_of(inode->i_cdev, struct isl290xx_data_t, cdev);
	ret = strcmp(isl290xx_data_tp->isl290xx_name, ISL290XX_DEVICE_ID);
	if (ret != 0) {
		pr_isl(ERROR,
		       "device name incorrect, get %s\n",
		       isl290xx_data_tp->isl290xx_name);
		ret = -ENODEV;
	}
	return ret;
}

static int isl290xx_read(struct file *file, char *buf, size_t count,
			 loff_t *ppos)
{
	struct isl290xx_data_t *isl290xx_data_tp;
	u8 i = 0, xfrd = 0, reg = 0;
	u8 my_buf[ISL290XX_MAX_DEVICE_REGS];
	int ret = 0;

	if ((*ppos < 0) || (*ppos >= ISL290XX_MAX_DEVICE_REGS)
	    || (count > ISL290XX_MAX_DEVICE_REGS)) {
		pr_isl(ERROR,
		       "reg limit check failed in isl290xx_read()\n");
		return - EINVAL;
	}
	reg = (u8)*ppos;
	isl290xx_data_tp =
	    container_of(file->f_dentry->d_inode->i_cdev,
			 struct isl290xx_data_t, cdev);
	while (xfrd < count) {
		my_buf[i++] =
		    i2c_smbus_read_byte_data(isl290xx_data_tp->client, reg);
		reg++;
		xfrd++;
	}
	ret = copy_to_user(buf, my_buf, xfrd);
	if (ret) {
		pr_isl(ERROR, "copy_to_user failed in isl290xx_read()\n");
		return - ENODATA;
	}
	return (int)xfrd;
}

static int isl290xx_write(struct file *file, const char *buf, size_t count,
			  loff_t *ppos)
{
	struct isl290xx_data_t *isl290xx_data_tp;
	u8 i = 0, xfrd = 0, reg = 0;
	u8 my_buf[ISL290XX_MAX_DEVICE_REGS];
	int ret = 0;

	if ((*ppos < 0) || (*ppos >= ISL290XX_MAX_DEVICE_REGS)
	    || ((*ppos + count) > ISL290XX_MAX_DEVICE_REGS)) {
		pr_isl(ERROR,
		       "reg limit check failed in isl290xx_write()\n");
		return - EINVAL;
	}
	reg = (u8)*ppos;
	ret = copy_from_user(my_buf, buf, count);
	if (ret) {
		pr_isl(ERROR, "ISL: copy_to_user failed in isl290xx_write()\n");
		return - ENODATA;
	}
	isl290xx_data_tp =
	    container_of(file->f_dentry->d_inode->i_cdev,
			 struct isl290xx_data_t, cdev);
	while (xfrd < count) {
		ret = i2c_smbus_write_byte_data(
		isl290xx_data_tp->client, reg, my_buf[i++]);
		if (ret  < 0) {
			pr_isl(ERROR,
			       "i2c_smbus_write_byte_data failed\n");
			return ret;
		}
		reg++;
		xfrd++;
	}
	return (int)xfrd;
}

static loff_t isl290xx_llseek(struct file *file, loff_t offset, int orig)
{
	int ret = 0;
	loff_t new_pos = 0;

	if ((offset >= ISL290XX_MAX_DEVICE_REGS) ||
		(orig < 0) || (orig > 1)) {
		pr_isl(ERROR,
		       "offset param limit or origin limit check failed\n");
		return - EINVAL;
	}
	switch (orig) {
	case 0:
		new_pos = offset;
		break;
	case 1:
		new_pos = file->f_pos + offset;
		break;
	default:
		return - EINVAL;
		break;
	}
	if ((new_pos < 0) || (new_pos >= ISL290XX_MAX_DEVICE_REGS)
		|| (ret < 0)) {
		pr_isl(ERROR,
		       "new offset limit or origin limit check failed\n");
		return - EINVAL;
	}
	file->f_pos = new_pos;
	return new_pos;
}
#ifdef ISL29147_ENABLE
static void prv_isl290xx_reset(void)
{
	int ret = 0;
	ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
		ISL290XX_TEST1, 0x38);
	if (ret < 0)
		pr_isl(ERROR, "write test2 register failed in reset\n");
	ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
		ISL290XX_TEST1, 0x00);
	if (ret < 0)
		pr_isl(ERROR, "s/w reset error\n");
}
#else
static void prv_isl290xx_reset(void)
{
	int ret = 0;
	ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
		ISL290XX_CFG_REG, 0x00);
	if (ret < 0)
		pr_isl(ERROR, "write cfg register failed in reset");
	ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
		ISL290XX_TEST2, 0x29);
	if (ret < 0)
		pr_isl(ERROR, "write test2 register failed in reset");
	ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
		ISL290XX_TEST1, 0x00);
	if (ret < 0)
		pr_isl(ERROR, "write test1 register failed in reset");
	ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
		ISL290XX_TEST2, 0x00);
	if (ret < 0)
		pr_isl(ERROR, "write test2 register failed in reset");

}
#endif
static int prv_isl290xx_set_bit(u8 reg, u8 bit_mask, u8 value)
{
	int ret = 0;
	u8 reg_val;
	reg_val = i2c_smbus_read_byte_data(isl290xx_data_tp->client, reg);
	if (reg_val < 0) {
			pr_isl(ERROR,
				"[isl290xx] prv_isl290xx_set_bit err%d\n",
				__LINE__);
			return reg_val;
	} else {
		if (value)
			reg_val |= (u8)bit_mask;
		else
			reg_val &= (u8)~bit_mask;
	}
	ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			reg, reg_val);
	if (ret < 0) {
		pr_isl(ERROR,
			"[isl290xx] prv_isl290xx_set_bit err%d\n",
			__LINE__);
		return ret;
	}
	return ret;
}

#ifdef ISL29147_ENABLE
static int isl290xx_set_bits(u8 reg, u8 bit_mask, u8 value)
{
	int ret;
	u8 init_val;
	u8 new_val;
	ret = 0;
	init_val = i2c_smbus_read_byte_data(isl290xx_data_tp->client, reg);
	if (ret < 0) {
		pr_err("[isl290xx]: fail to read reg 0x%x\n", reg);
		return ret;
	}
	new_val = (bit_mask&value) | ((~bit_mask)&init_val);
	ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			reg, new_val);
	if (ret < 0) {
		pr_err("[isl290xx]: fail to write reg 0x%x\n", reg);
		return ret;
	}
	return ret;

}
#endif
/*prv_isl290xx_ctrl_lp, mask values' indication*/
/*10 : light on*/
/*01 : prox on*/
/*20 : light off*/
/*02 : prox off*/
#ifdef ISL29147_ENABLE
static int prv_isl290xx_ctrl_lp(int mask)
{
	u8 ret = 0, reg_val = 0;
	mutex_lock(&isl290xx_data_tp->update_lock);
	if (mask == 0x10) {	/*10 : light on */
		pr_isl(INFO, "light on\n");
		prv_isl290xx_reset();
		input_report_abs(light->input_dev, ABS_MISC, -1);
		input_sync(light->input_dev);
		ret = isl290xx_set_bits(ISL290XX_INT_REG,
			ISL290XX_INT_ALS_PRST_BIT,
			isl290xx_cfgp->aps_intr);
		if (ret < 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_INT_REG,
			ISL290XX_INT_CTRL_BIT, 0);
		if (ret < 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_INT_REG,
			ISL290XX_INT_ALS_BIT, 0);
		if (ret < 0)
			goto out;

		reg_val = als_intr_threshold_lo_param & 0x00FF;
		ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_ALSIR_TH_L, reg_val);
		if (ret < 0) {
			pr_isl(ERROR,
			       "[isl290xx] prv_isl290xx_ctrl_lp i2c err%d\n",
			       __LINE__);

			goto out;
		}
		reg_val =
		    ((als_intr_threshold_lo_param >> 8) & 0x000F) |
		    ((als_intr_threshold_hi_param << 4) & 0x00F0);
		ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_ALSIR_TH_HL, reg_val);
		if (ret < 0) {
			pr_isl(ERROR,
			       "[isl290xx] prv_isl290xx_ctrl_lp i2c err%d\n",
			       __LINE__);

			goto out;
		}
		reg_val = (als_intr_threshold_hi_param >> 4) & 0x00FF;
		ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_ALSIR_TH_H, reg_val);
		if (ret < 0) {
			pr_isl(ERROR,
			       "[isl290xx] prv_isl290xx_ctrl_lp i2c err%d\n",
			       __LINE__);

			goto out;
		}
		ret = isl290xx_set_bits(ISL290XX_CFG1_REG,
			ISL290XX_ALS_RANGE_BIT,
			isl290xx_cfgp->aps_config1&ISL290XX_ALS_RANGE_BIT);
		if (ret < 0)
			goto out;
		ret = isl290xx_set_bits(ISL290XX_CFG1_REG,
			ISL290XX_INT_ALG,
			isl290xx_cfgp->aps_config1&ISL290XX_INT_ALG);
		if (ret < 0)
			goto out;
		ret = isl290xx_set_bits(ISL290XX_CFG1_REG,
			ISL290XX_ALS_EN_BIT, 0x4);
		if (ret < 0)
			goto out;
		mdelay(5);
		g_isl290xx_lux = prv_isl290xx_get_lux();
		prv_isl290xx_report_value(0);

	}
	if (mask == 0x01) {	/*01 : prox on */
		pr_isl(INFO, "prox on\n");
		input_report_abs(proximity->input_dev, ABS_DISTANCE, -1);
		input_sync(proximity->input_dev);
		ret = isl290xx_set_bits(ISL290XX_INT_REG,
			ISL290XX_INT_PROX_PRST_BIT,
			isl290xx_cfgp->aps_intr&ISL290XX_INT_PROX_PRST_BIT);
		if (ret < 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_INT_REG,
			ISL290XX_INT_PROX_BIT, 0);
		if (ret < 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_INT_REG,
				ISL290XX_INT_ALS_BIT, 0);
		if (ret < 0)
			pr_isl(ERROR,
				"isl290xx: cannot write unused bit\n");
		reg_val = isl290xx_cfgp->prox_threshold_lo & 0x00FF;
		ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_PROX_TH_L, reg_val);
		if (ret < 0) {
			pr_isl(ERROR,
			       "[isl290xx] prv_isl290xx_ctrl_lp i2c err%d\n",
			       __LINE__);
			goto out;
		}
		reg_val = isl290xx_cfgp->prox_threshold_hi & 0x00FF;
		ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_PROX_TH_H, reg_val);
		if (ret < 0) {
			pr_isl(ERROR,
			       "[isl290xx] prv_isl290xx_ctrl_lp i2c err%d\n",
			       __LINE__);
			goto out;
		}
		ret = isl290xx_set_bits(ISL290XX_CFG_REG,
			ISL290XX_PROX_SLP_BIT,
			isl290xx_cfgp->aps_config&ISL290XX_PROX_SLP_BIT);
		if (ret < 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_CFG_REG,
			ISL290XX_PROX_DR_BIT,
			isl290xx_cfgp->aps_config&ISL290XX_PROX_DR_BIT);
		if (ret < 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_CFG_REG,
			ISL290XX_PROX_EN_BIT, 1);
		if (ret < 0)
			goto out;
		ret = prv_isl290xx_prox_poll(isl290xx_prox_cur_infop);
		mdelay(5);
		if (ret < 0)
			pr_isl(ERROR, "get prox poll failed\n");
		if (isl290xx_prox_cur_infop->prox_data >
			isl290xx_cfgp->prox_threshold_hi) {
			isl290xx_prox_cur_infop->prox_event = 1;
		} else if (isl290xx_prox_cur_infop->prox_data <
			isl290xx_cfgp->prox_threshold_lo)
			isl290xx_prox_cur_infop->prox_event = 0;
		prv_isl290xx_report_value(1);
	}
	if (mask == 0x20) {	/*20 : light off */
		reg_val = 0xff;
		ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_ALSIR_TH_L, reg_val);
		if (ret < 0) {
			pr_isl(ERROR,
			       "[isl290xx] prv_isl290xx_ctrl_lp i2c err%d\n",
			       __LINE__);
			goto out;
		}
		reg_val = 0x0f;
		ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_ALSIR_TH_HL, reg_val);
		if (ret < 0) {
			pr_isl(ERROR,
			       "i2c_smbus_write_byte_data failed\n");
			goto out;
		}
		reg_val = 0x0;
		ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_ALSIR_TH_H, reg_val);
		if (ret < 0) {
			pr_isl(ERROR,
			       "i2c_smbus_write_byte_data failed\n");
			goto out;
		}
		reg_val = i2c_smbus_read_byte_data(isl290xx_data_tp->client,
			ISL290XX_INT_REG);
		if (reg_val < 0) {
			pr_isl(ERROR,
			       "i2c_smbus_read_byte_data failed\n");
			mutex_unlock(&isl290xx_data_tp->update_lock);
			return reg_val;
		}
		reg_val = (reg_val & (~ISL290XX_ALS_EN_BIT));
		ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_CFG1_REG, reg_val);
		if (ret < 0) {
			pr_isl(ERROR,
			       "i2c_smbus_write_byte_data failed\n");
			goto out;
		}
	}
	if (mask == 0x02) {	/*02 : prox off */
		ret = prv_isl290xx_set_bit(ISL290XX_CFG_REG,
				ISL290XX_PROX_EN_BIT, 0);
		if (ret != 0)
			goto out;
		ret = isl290xx_set_bits(ISL290XX_CFG_REG,
				ISL290XX_PROX_DR_BIT, 0);
		if (ret != 0)
			goto out;
	}
out:
	mutex_unlock(&isl290xx_data_tp->update_lock);
	return ret;
}

#else
static int prv_isl290xx_ctrl_lp(int mask)
{
	u8 ret = 0, reg_val = 0;
	mutex_lock(&isl290xx_data_tp->update_lock);
	if (mask == 0x10) {	/*10 : light on */
		pr_isl(INFO, "light on\n");
		prv_isl290xx_reset();
		input_report_abs(light->input_dev, ABS_MISC, -1);
		input_sync(light->input_dev);
		ret = prv_isl290xx_set_bit(ISL290XX_INT_REG,
			ISL290XX_INT_ALS_PRST_BIT&0x02,
			isl290xx_cfgp->aps_intr&0x02);
		if (ret < 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_INT_REG,
			ISL290XX_INT_ALS_PRST_BIT&0x04,
			isl290xx_cfgp->aps_intr&0x04);
		if (ret < 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_INT_REG,
			ISL290XX_INT_CTRL_BIT, 0);
		if (ret < 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_INT_REG,
			ISL290XX_INT_ALS_BIT, 0);
		if (ret < 0)
			goto out;

		reg_val = als_intr_threshold_lo_param & 0x00FF;
		ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_ALSIR_TH_L, reg_val);
		if (ret < 0) {
			pr_isl(ERROR,
			       "[isl290xx] prv_isl290xx_ctrl_lp i2c err%d\n",
			       __LINE__);

			goto out;
		}
		reg_val =
		    ((als_intr_threshold_lo_param >> 8) & 0x000F) |
		    ((als_intr_threshold_hi_param << 4) & 0x00F0);
		ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_ALSIR_TH_HL, reg_val);
		if (ret < 0) {
			pr_isl(ERROR,
			       "[isl290xx] prv_isl290xx_ctrl_lp i2c err%d\n",
			       __LINE__);

			goto out;
		}
		reg_val = (als_intr_threshold_hi_param >> 4) & 0x00FF;
		ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_ALSIR_TH_H, reg_val);
		if (ret < 0) {
			pr_isl(ERROR,
			       "[isl290xx] prv_isl290xx_ctrl_lp i2c err%d\n",
			       __LINE__);

			goto out;
		}
		ret = prv_isl290xx_set_bit(ISL290XX_CFG_REG,
			ISL290XX_ALS_RANGE_BIT,
			isl290xx_cfgp->aps_config&0x02);
		if (ret < 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_CFG_REG,
			ISL290XX_ALSIR_MODE_BIT,
			isl290xx_cfgp->aps_config&0x01);
		if (ret < 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_CFG_REG,
			ISL290XX_ALS_EN_BIT, 1);
		if (ret < 0)
			goto out;
		mdelay(5);
		g_isl290xx_lux = prv_isl290xx_get_lux();
		prv_isl290xx_report_value(0);

	}
	if (mask == 0x01) {	/*01 : prox on */
		pr_isl(INFO, "prox on\n");
		input_report_abs(proximity->input_dev, ABS_DISTANCE, -1);
		input_sync(proximity->input_dev);
		ret = prv_isl290xx_set_bit(ISL290XX_INT_REG,
			ISL290XX_INT_PROX_PRST_BIT&0x20,
			isl290xx_cfgp->aps_intr&0x20);
		if (ret < 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_INT_REG,
			ISL290XX_INT_PROX_PRST_BIT&0x40,
			isl290xx_cfgp->aps_intr&0x40);
		if (ret < 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_INT_REG,
			ISL290XX_INT_PROX_BIT, 0);
		if (ret < 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_INT_REG, 0x10, 0);
		if (ret < 0)
			pr_isl(ERROR,
				"isl290xx: cannot write unused bit\n");
		reg_val = isl290xx_cfgp->prox_threshold_lo & 0x00FF;
		ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_PROX_TH_L, reg_val);
		if (ret < 0) {
			pr_isl(ERROR,
			       "[isl290xx] prv_isl290xx_ctrl_lp i2c err%d\n",
			       __LINE__);
			goto out;
		}
		reg_val = isl290xx_cfgp->prox_threshold_hi & 0x00FF;
		ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_PROX_TH_H, reg_val);
		if (ret < 0) {
			pr_isl(ERROR,
			       "[isl290xx] prv_isl290xx_ctrl_lp i2c err%d\n",
			       __LINE__);
			goto out;
		}
		ret = prv_isl290xx_set_bit(ISL290XX_CFG_REG,
			ISL290XX_PROX_SLP_BIT&0x40,
			isl290xx_cfgp->aps_config&0x40);
		if (ret < 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_CFG_REG,
			ISL290XX_PROX_SLP_BIT&0x20,
			isl290xx_cfgp->aps_config&0x20);
		if (ret < 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_CFG_REG,
			ISL290XX_PROX_SLP_BIT&0x10,
			isl290xx_cfgp->aps_config&0x10);
		if (ret < 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_CFG_REG,
			ISL290XX_PROX_DR_BIT,
			isl290xx_cfgp->aps_config&0x08);
		if (ret < 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_CFG_REG,
			ISL290XX_INT_PROX_BIT, 1);
		if (ret < 0)
			goto out;
		ret = prv_isl290xx_prox_poll(isl290xx_prox_cur_infop);
		mdelay(5);
		if (ret < 0)
			pr_isl(ERROR, "get prox poll failed\n");
		if (isl290xx_prox_cur_infop->prox_data >
			isl290xx_cfgp->prox_threshold_hi) {
			isl290xx_prox_cur_infop->prox_event = 1;
		} else if (isl290xx_prox_cur_infop->prox_data <
			isl290xx_cfgp->prox_threshold_lo)
			isl290xx_prox_cur_infop->prox_event = 0;
		prv_isl290xx_report_value(1);
	}
	if (mask == 0x20) {	/*20 : light off */
		reg_val = 0xff;
		ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_ALSIR_TH_L, reg_val);
		if (ret < 0) {
			pr_isl(ERROR,
			       "[isl290xx] prv_isl290xx_ctrl_lp i2c err%d\n",
			       __LINE__);
			goto out;
		}
		reg_val = 0x0f;
		ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_ALSIR_TH_HL, reg_val);
		if (ret < 0) {
			pr_isl(ERROR,
			       "i2c_smbus_write_byte_data failed\n");
			goto out;
		}
		reg_val = 0x0;
		ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_ALSIR_TH_H, reg_val);
		if (ret < 0) {
			pr_isl(ERROR,
			       "i2c_smbus_write_byte_data failed\n");
			goto out;
		}
		reg_val = i2c_smbus_read_byte_data(isl290xx_data_tp->client,
			ISL290XX_INT_REG);
		if (reg_val < 0) {
			pr_isl(ERROR,
			       "i2c_smbus_read_byte_data failed\n");
			mutex_unlock(&isl290xx_data_tp->update_lock);
			return reg_val;
		}
		reg_val = (reg_val & (~ISL290XX_ALS_INT_MASK));
		ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_INT_REG, reg_val);
		if (ret < 0) {
			pr_isl(ERROR,
			       "i2c_smbus_write_byte_data failed\n");
			goto out;
		}
		reg_val = i2c_smbus_read_byte_data(isl290xx_data_tp->client,
			ISL290XX_CFG_REG);
		if (reg_val < 0) {
			pr_isl(ERROR,
			       "i2c_smbus_read_byte_data failed\n");
			mutex_unlock(&isl290xx_data_tp->update_lock);
			return reg_val;
		}
		reg_val = (reg_val & (~ISL290XX_ALS_MASK));
		ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_CFG_REG, reg_val);
		if (ret < 0) {
			pr_isl(ERROR,
			       "i2c_smbus_write_byte_data failed\n");
			goto out;
		}
	}
	if (mask == 0x02) {	/*02 : prox off */
		ret = prv_isl290xx_set_bit(ISL290XX_CFG_REG,
				ISL290XX_PROX_EN_BIT, 0);
		if (ret != 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_CFG_REG,
				ISL290XX_PROX_SLP_BIT&0x40, 0);
		if (ret != 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_CFG_REG,
				ISL290XX_PROX_SLP_BIT&0x20, 0);
		if (ret != 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_CFG_REG,
				ISL290XX_PROX_SLP_BIT&0x10, 0);
		if (ret != 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_CFG_REG,
				ISL290XX_PROX_DR_BIT, 0);
		if (ret != 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_CFG_REG,
				ISL290XX_INT_PROX_BIT, 0);
		if (ret != 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_CFG_REG,
				ISL290XX_INT_PROX_PRST_BIT&0x4, 0);
		if (ret != 0)
			goto out;
		ret = prv_isl290xx_set_bit(ISL290XX_CFG_REG,
				ISL290XX_INT_PROX_PRST_BIT&0x2, 0);
		if (ret != 0)
			goto out;
	}
out:
	mutex_unlock(&isl290xx_data_tp->update_lock);
	return ret;
}
#endif

#ifdef ISL290XX_USER_CALIBRATION
static void prv_isl290xx_prox_cali(void)
{
	int prox_sum = 0, prox_mean = 0, prox_max = 0;
	int ret = 0, i = 0, prox_status = 0;
	int valid = 0;
	if (!prox_on) {
		prv_isl290xx_ctrl_lp(0x01);
		prox_status = 1;
	}
	mutex_lock(&isl290xx_data_tp->proximity_calibrating);
	prox_sum = 0;
	prox_max = 0;
	for (i = 0; i < 20; i++) {
		ret = prv_isl290xx_prox_poll(&isl290xx_prox_cal_info[i]);
		if (ret < 0)
			pr_isl(ERROR,
				"call to prox_poll failed\n");
		else {
			valid++;
			pr_isl(INFO,
				"prox calibrate poll prox[%d] = %d\n",
				i, isl290xx_prox_cal_info[i].prox_data);
			prox_sum += isl290xx_prox_cal_info[i].prox_data;
			if (isl290xx_prox_cal_info[i].prox_data > prox_max)
				prox_max = isl290xx_prox_cal_info[i].prox_data;
		}
		mdelay(100);
	}
	if (valid)
		prox_mean = prox_sum / valid;
	isl290xx_offset = prox_mean;
	mutex_unlock(&isl290xx_data_tp->proximity_calibrating);
	if (prox_status)
		prv_isl290xx_ctrl_lp(0x02);
	pr_isl(ERROR,
		"ISL290XX: valid samples:%d, delta:%d\n",
		valid, prox_mean);
}

static void prv_isl290xx_prox_boot_cali(void)
{
	int hi;
	int lo;
	int i;

	pr_isl(INFO, "prox. auto calibration in progress\n");
	prv_isl290xx_prox_cali();
	hi = isl290xx_cfgp->prox_threshold_hi_def + isl290xx_offset
			- isl290xx_cfgp->prox_offset;
	lo = isl290xx_cfgp->prox_threshold_lo_def + isl290xx_offset
			- isl290xx_cfgp->prox_offset;
	pr_isl(INFO,
		"isl290xx_offset=%d,prox_offset=%d, hi =%d, lo=%d\n",
		isl290xx_offset, isl290xx_cfgp->prox_offset, hi, lo);

	if (hi > 0xfe || lo > 0xfe ||
		(isl290xx_offset-isl290xx_cfgp->prox_offset) < 0) {
		isl290xx_offset = 0;
		pr_isl(ERROR, "isl290xx boot cali failed\n");
	} else {
		if (lo > isl290xx_offset) {
			isl290xx_cfgp->prox_threshold_hi = hi;
			isl290xx_cfgp->prox_threshold_lo = lo;
		} else {
			for (i = 0; i < 60; i++) {
				hi += 2;
				lo += 2;
				if (lo > isl290xx_offset) {
					isl290xx_cfgp->prox_threshold_hi
						= hi + 1;
					isl290xx_cfgp->prox_threshold_lo
						= lo + 1;
					break;
				}
			}
		}
	}
}
#endif

static long isl290xx_ioctl(struct file *file, unsigned int cmd,
			   unsigned long arg)
{
	int prox_sum = 0, prox_mean = 0, prox_max = 0;
	int ret = 0, i = 0;
	u8 reg_val = 0;
	u16 ratio;
	struct isl290xx_prox_data_t *prox_pt;
	switch (cmd) {
	case ISL290XX_IOCTL_ALS_ON:
		if (light_on) {
			pr_isl(INFO,
			       "ISL290XX_IOCTL_ALS_ON: light already on.\n");
			break;
		}
		ret = prv_isl290xx_ctrl_lp(0x10);
		if (ret >= 0) {
			light_on = 1;
			pr_isl(INFO, "ISL290XX_IOCTL_ALS_ON,lux=%d\n",
			       g_isl290xx_lux);
		}
		return ret;
		break;
	case ISL290XX_IOCTL_ALS_OFF:
		if (light_on == 0) {
			pr_isl(INFO,
			       "ISL290XX_IOCTL_ALS_OFF: light already off.\n");
			break;
		}

		for (i = 0; i < ISL290XX_FILTER_DEPTH; i++)
			lux_history[i] = -ENODATA;
		ret = prv_isl290xx_ctrl_lp(0x20);
		if (ret >= 0) {
			light_on = 0;
			pr_isl(INFO, "ISL290XX_IOCTL_ALS_OFF\n");
		}
		return ret;
		break;
	case ISL290XX_IOCTL_ALS_CALIBRATE:

		break;

	case ISL290XX_IOCTL_CONFIG_GET:
#ifdef ISL290XX_USER_CALIBRATION
		if (isl290xx_cfgp->prox_boot_cali)
			prv_isl290xx_prox_boot_cali();
#endif
		ret =
		    copy_to_user((struct isl290xx_cfg_s *)arg, isl290xx_cfgp,
				 sizeof(struct isl290xx_cfg_s));
		if (ret) {
			pr_isl(ERROR,
			       "copy_to_user failed in ioctl config_get\n");
			return - ENODATA;
		}
		return ret;
		break;
	case ISL290XX_IOCTL_CONFIG_SET:
		ret =
		    copy_from_user(isl290xx_cfgp, (struct isl290xx_cfg_s *)arg,
				   sizeof(struct isl290xx_cfg_s));
		if (ret) {
			pr_isl(ERROR,
			       "copy_from_user failed in ioctl config_set\n");
			return - ENODATA;
		}
		return ret;
		break;
	case ISL290XX_IOCTL_PROX_ON:
		if (prox_on) {
			pr_isl(INFO,
			       "ISL290XX_IOCTL_PROX_ON: prox already on.\n");
			break;
		}

		isl290xx_prox_cur_infop->prox_event = 0;
		isl290xx_prox_cur_infop->prox_clear = 0;
		isl290xx_prox_cur_infop->prox_data = 0;
		ret = prv_isl290xx_ctrl_lp(0x01);
		if (ret >= 0) {
			prox_on = 1;
			pr_isl(INFO, "ISL290XX_IOCTL_PROX_ON\n");
		}
		return ret;
		break;
	case ISL290XX_IOCTL_PROX_OFF:
		if (prox_on == 0) {
			pr_isl(INFO,
			       "ISL290XX_IOCTL_PROX_OFF: prox already off.\n");
			break;
		}

		ret = prv_isl290xx_ctrl_lp(0x02);
		if (ret >= 0) {
			prox_on = 0;
			pr_isl(INFO, "ISL290XX_IOCTL_PROX_OFF\n");
		}
		return ret;
		break;
	case ISL290XX_IOCTL_PROX_DATA:
		ret =
		    copy_to_user((struct isl290xx_prox_info_s *)arg,
				 isl290xx_prox_cur_infop,
				 sizeof(struct isl290xx_prox_info_s));
		if (ret) {
			pr_isl(ERROR,
			       "ISL: copy_to_user failed in ioctl prox_data\n");
			return - ENODATA;
		}
		return ret;
		break;
	case ISL290XX_IOCTL_PROX_EVENT:
		return isl290xx_prox_cur_infop->prox_event;
		break;

	case ISL290XX_IOCTL_PROX_GET_ENABLED:
		return put_user(prox_on, (unsigned long __user *)arg);
		break;
	case ISL290XX_IOCTL_ALS_GET_ENABLED:
		return put_user(light_on, (unsigned long __user *)arg);
		break;
	case ISL290XX_IOCTL_PROX_CALIBRATE:
		if (!prox_on) {
			pr_isl(ERROR,
			       "ioctl prox_calibrate was called before\n");
			return - EPERM;
		}
		mutex_lock(&isl290xx_data_tp->proximity_calibrating);
		prox_sum = 0;
		prox_max = 0;
		for (i = 0; i < 20; i++) {
			ret = prv_isl290xx_prox_poll(
				&isl290xx_prox_cal_info[i]);
			if (ret  < 0) {
				pr_isl(ERROR,
				       "call to prox_poll failed\n");
				mutex_unlock(&isl290xx_data_tp->
					     proximity_calibrating);
				return ret;
			}
			pr_isl(INFO,
			       "prox calibrate poll prox[%d] = %d\n", i,
			       isl290xx_prox_cal_info[i].prox_data);
			prox_sum += isl290xx_prox_cal_info[i].prox_data;
			if (isl290xx_prox_cal_info[i].prox_data > prox_max)
				prox_max = isl290xx_prox_cal_info[i].prox_data;
			mdelay(100);
		}
		prox_mean = prox_sum / 20;
		ratio = 10 * prox_mean / 0xff;
		for (prox_pt = isl290xx_prox_tablep;
		     prox_pt->ratio && prox_pt->ratio <= ratio; prox_pt++)
			;
		if (!prox_pt->ratio)
			return - 1;
		isl290xx_cfgp->prox_threshold_hi =
		    (prox_mean * prox_pt->hi) / 10;
		if (isl290xx_cfgp->prox_threshold_hi <= ((0xff * 3) / 100))
			isl290xx_cfgp->prox_threshold_hi =
			    ((0xff * 8) / 100);
		isl290xx_cfgp->prox_threshold_lo =
		    isl290xx_cfgp->prox_threshold_hi * THRES_LO_TO_HI_RATIO;
		reg_val = 0;
		ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_PROX_TH_L, reg_val);
		if (ret < 0) {
			pr_isl(ERROR,
			       "i2c_smbus_write_byte_data failed\n");
			return ret;
		}
		reg_val = isl290xx_cfgp->prox_threshold_hi & 0x00FF;
		ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_PROX_TH_H, reg_val);
		if (ret < 0) {
			pr_isl(ERROR,
			       "i2c_smbus_write_byte_data failed\n");
			return ret;
		}
		mutex_unlock(&isl290xx_data_tp->proximity_calibrating);
		break;

	default:
		return - EINVAL;
		break;
	}
	return ret;
}

static int prv_isl290xx_get_lux(void)
{
	u16 raw_clear;
	u16 reg_val;
	unsigned int zone_size;
	unsigned int switch_range;
	u32 lux = 0;
	int ret = 0;
	u8 chdata[2];
	int i = 0;
	raw_clear = 0;
	reg_val = 0;
	zone_size = 1;
	switch_range = 0;
	mutex_lock(&isl290xx_data_tp->lock);
	for (i = 0; i < 2; i++)
#ifdef ISL29147_ENABLE
		chdata[i] =
		    i2c_smbus_read_byte_data(isl290xx_data_tp->client,
					     ISL290XX_ALSIR_DATA_H + i);

	raw_clear = ((chdata[0]&0xf) << 8) + chdata[1];
#else
		chdata[i] =
		    i2c_smbus_read_byte_data(isl290xx_data_tp->client,
					     ISL290XX_ALSIR_DATA_L + i);

	raw_clear = ((chdata[1] & 0xF) << 8) | chdata[0];
#endif
	/*zone_size = raw_clear >> 3;*/
	als_intr_threshold_lo_param = ((raw_clear > zone_size) ? \
						raw_clear-zone_size : 0);
	if (raw_clear <= zone_size)
		als_intr_threshold_hi_param = 0;
	else if (raw_clear < 0xfff && raw_clear > zone_size) {
		als_intr_threshold_hi_param = raw_clear + zone_size;
		als_intr_threshold_hi_param =
				((als_intr_threshold_hi_param > 0xffe) ? \
				0xffe : als_intr_threshold_hi_param);
	} else
		als_intr_threshold_hi_param = 0xffe;
	reg_val = als_intr_threshold_lo_param & 0x00FF;
	ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_ALSIR_TH_L, reg_val);
	if (ret < 0) {
		pr_isl(ERROR,
		       "i2c_smbus_write_byte_data failed\n");
		mutex_unlock(&isl290xx_data_tp->lock);
		return ret;
	}
	reg_val =
	    ((als_intr_threshold_lo_param >> 8) & 0x000F) |
	    ((als_intr_threshold_hi_param << 4) & 0x00F0);
	ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
		ISL290XX_ALSIR_TH_HL, reg_val);
	if (ret < 0) {
		pr_isl(ERROR,
		       "i2c_smbus_write_byte_data failed\n");
		mutex_unlock(&isl290xx_data_tp->lock);
		return ret;
	}
	reg_val = (als_intr_threshold_hi_param >> 4) & 0x00FF;
	ret = i2c_smbus_write_byte_data(isl290xx_data_tp->client,
			ISL290XX_ALSIR_TH_H, reg_val);
	if (ret < 0) {
		pr_isl(ERROR,
		       "i2c_smbus_write_byte_data failed\n");
		mutex_unlock(&isl290xx_data_tp->lock);
		return ret;
	}

	 /*enable als*/
	lux = raw_clear;
	mutex_unlock(&isl290xx_data_tp->lock);
	return lux;
}

static int prv_isl290xx_prox_poll(struct isl290xx_prox_info_s *prxp)
{
	u8 prox_data;

	prox_data =
	    i2c_smbus_read_byte_data(isl290xx_data_tp->client,
				     ISL290XX_PROX_DATA);
	prxp->prox_data = prox_data;

	return prox_data;
}

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("isl Ambient Light and Proximity Sensor Driver");
MODULE_LICENSE("GPL");

module_init(isl290xx_init);
module_exit(isl290xx_exit);
