/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
* more details.
*******************************************************************************/
/*******************************************************************************
*                                                                              *
*   File Name:    taos.c                                                       *
*   Description:   Linux device driver for Taos ambient light and              *
*   proximity sensors.                                                         *
*   Author:         John Koshi                                                 *
*   History:   09/16/2009 - Initial creation                                   *
*           10/09/2009 - Triton version                                        *
*           12/21/2009 - Probe/remove mode                                     *
*           02/07/2010 - Add proximity                                         *
*                                                                              *
********************************************************************************
*    Proprietary to Taos Inc., 1001 Klein Road #300, Plano, TX 75074           *
*******************************************************************************/

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/timer.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/i2c/taos_common.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <mach/gpio.h>
#include <linux/poll.h>
#include <linux/wakelock.h>
#include <linux/input.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#define I2C_RETRY_DELAY		5
#define I2C_RETRY_COUNTS	20
#define LOG_LEVEL_ERROR		(1U<<0)
#define LOG_LEVEL_INFO		(1U<<0)
#define LOG_LEVEL_DEBUG		(1U<<1)
#define LOG_LEVEL_FLOW		(1U<<2)

static int debug_mask = LOG_LEVEL_ERROR;
module_param_named(debug_mask, debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);
MODULE_PARM_DESC(debug,
		 "Debug level: Default: 0\n"
		 "\t\t\t  1: error info\n"
		 "\t\t\t  2: debug info\n" "\t\t\t  4: function flow\n");
#define pr_taos(debug_level_mask, fmt, args...) do {\
	if (debug_mask & LOG_LEVEL_##debug_level_mask)\
		pr_info("%s|%d: "fmt, KBUILD_BASENAME, __LINE__, ##args);\
} while (0)

#define TAOS_DEVICE_NAME                "taos"
#define TAOS_DEVICE_ID                  "tmd2771"
#define TAOS_ID_NAME_SIZE               10
#define TAOS_TRITON_CHIPIDVAL           0x00
#define TAOS_TRITON_MAXREGS             32
#define TAOS_DEVICE_ADDR1               0x29
#define TAOS_DEVICE_ADDR2               0x39
#define TAOS_DEVICE_ADDR3               0x49
#define TAOS_MAX_NUM_DEVICES            3
#define TAOS_MAX_DEVICE_REGS            32
#define I2C_MAX_ADAPTERS                1

/* TRITON register offsets */
#define TAOS_TRITON_CNTRL               0x00
#define TAOS_TRITON_ALS_TIME            0X01
#define TAOS_TRITON_PRX_TIME            0x02
#define TAOS_TRITON_WAIT_TIME           0x03
#define TAOS_TRITON_ALS_MINTHRESHLO     0X04
#define TAOS_TRITON_ALS_MINTHRESHHI     0X05
#define TAOS_TRITON_ALS_MAXTHRESHLO     0X06
#define TAOS_TRITON_ALS_MAXTHRESHHI     0X07
#define TAOS_TRITON_PRX_MINTHRESHLO     0X08
#define TAOS_TRITON_PRX_MINTHRESHHI     0X09
#define TAOS_TRITON_PRX_MAXTHRESHLO     0X0A
#define TAOS_TRITON_PRX_MAXTHRESHHI     0X0B
#define TAOS_TRITON_INTERRUPT           0x0C
#define TAOS_TRITON_PRX_CFG             0x0D
#define TAOS_TRITON_PRX_COUNT           0x0E
#define TAOS_TRITON_GAIN                0x0F
#define TAOS_TRITON_REVID               0x11
#define TAOS_TRITON_CHIPID              0x12
#define TAOS_TRITON_STATUS              0x13
#define TAOS_TRITON_ALS_CHAN0LO         0x14
#define TAOS_TRITON_ALS_CHAN0HI         0x15
#define TAOS_TRITON_ALS_CHAN1LO         0x16
#define TAOS_TRITON_ALS_CHAN1HI         0x17
#define TAOS_TRITON_PRX_LO              0x18
#define TAOS_TRITON_PRX_HI              0x19
#define TAOS_TRITON_TEST_STATUS         0x1F

/* Triton cmd reg masks */
#define TAOS_TRITON_CMD_REG             0X80
#define TAOS_TRITON_CMD_AUTO            0x10
#define TAOS_TRITON_CMD_BYTE_RW         0x00
#define TAOS_TRITON_CMD_WORD_BLK_RW     0x20
#define TAOS_TRITON_CMD_SPL_FN          0x60
#define TAOS_TRITON_CMD_PROX_INTCLR     0X05
#define TAOS_TRITON_CMD_ALS_INTCLR      0X06
#define TAOS_TRITON_CMD_PROXALS_INTCLR  0X07
#define TAOS_TRITON_CMD_TST_REG         0X08
#define TAOS_TRITON_CMD_USER_REG        0X09

/* Triton cntrl reg masks */
#define TAOS_TRITON_CNTL_PROX_INT_ENBL  0X20
#define TAOS_TRITON_CNTL_ALS_INT_ENBL   0X10
#define TAOS_TRITON_CNTL_WAIT_TMR_ENBL  0X08
#define TAOS_TRITON_CNTL_PROX_DET_ENBL  0X04
#define TAOS_TRITON_CNTL_ADC_ENBL       0x02
#define TAOS_TRITON_CNTL_PWRON          0x01

/* Triton status reg masks */
#define TAOS_TRITON_STATUS_ADCVALID     0x01
#define TAOS_TRITON_STATUS_PRXVALID     0x02
#define TAOS_TRITON_STATUS_ADCINTR      0x10
#define TAOS_TRITON_STATUS_PRXINTR      0x20

/* lux constants */
#define TAOS_MAX_LUX                    1000000
#define TAOS_SCALE_MILLILUX             2
#define TAOS_FILTER_DEPTH               3
#define CHIP_ID                         0x3d

struct tmd2771x_reg {
	const char *name;
	u8 reg;
} tmd2771x_regs[] = {
	{"ENABLE", TAOS_TRITON_CNTRL},
	{"ATIME", TAOS_TRITON_ALS_TIME},
	{"PTIME", TAOS_TRITON_PRX_TIME},
	{"WTIME", TAOS_TRITON_WAIT_TIME},
	{"AILTL", TAOS_TRITON_ALS_MINTHRESHLO},
	{"AILTH", TAOS_TRITON_ALS_MINTHRESHHI},
	{"AIHTL", TAOS_TRITON_ALS_MAXTHRESHLO},
	{"AIHTH", TAOS_TRITON_ALS_MAXTHRESHHI},
	{"PILTL", TAOS_TRITON_PRX_MINTHRESHLO},
	{"PILTH", TAOS_TRITON_PRX_MINTHRESHHI},
	{"PIHTL", TAOS_TRITON_PRX_MAXTHRESHLO},
	{"PIHTH", TAOS_TRITON_PRX_MAXTHRESHHI},
	{"PERS", TAOS_TRITON_INTERRUPT},
	{"CONFIG", TAOS_TRITON_PRX_CFG},
	{"PPCOUNT", TAOS_TRITON_PRX_COUNT},
	{"CONTROL", TAOS_TRITON_GAIN},
	{"ID", TAOS_TRITON_CHIPID},
	{"STATUS", TAOS_TRITON_STATUS},
	{"C0DATA", TAOS_TRITON_ALS_CHAN0LO},
	{"C0DATAH", TAOS_TRITON_ALS_CHAN0HI},
	{"C1DATA", TAOS_TRITON_ALS_CHAN1LO},
	{"C1DATAH", TAOS_TRITON_ALS_CHAN1HI},
	{"PDATA", TAOS_TRITON_PRX_LO},
	{"PDATAH", TAOS_TRITON_PRX_HI},
};

static const struct of_device_id tmd2771_of_match[] = {
	{.compatible = "bcm,tmd2771",},
	{},
}

MODULE_DEVICE_TABLE(of, tmd2771_of_match);

#ifdef TMD2771_USER_CALIBRATION
static int tmd2771_offset;
#endif

#define TAOS_INPUT_NAME_ALS  "TAOS_ALS_SENSOR"
#define TAOS_INPUT_NAME_PROX  "TAOS_PROX_SENSOR"
int isPsensorLocked;
int als_ps_int;
int als_ps_gpio_inr;

/* forward declarations */
static int taos_probe(struct i2c_client *clientp,
		      const struct i2c_device_id *idp);
static int taos_remove(struct i2c_client *client);
static int taos_open(struct inode *inode, struct file *file);
static int taos_release(struct inode *inode, struct file *file);
static long taos_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static int taos_read(struct file *file, char *buf, size_t count, loff_t *ppos);
static int taos_write(struct file *file, const char *buf, size_t count,
		      loff_t *ppos);
static loff_t taos_llseek(struct file *file, loff_t offset, int orig);
static int taos_get_lux(void);
static int taos_lux_filter(int raw_lux);
static int taos_device_name(unsigned char *bufp, char **device_name);
static int taos_prox_poll(struct taos_prox_info *prxp);
static void taos_prox_poll_timer_func(unsigned long param);
static void taos_prox_poll_timer_start(void);

static int taos_als_threshold_set(void);
static int taos_prox_threshold_set(void);
static int taos_als_get_data(void);
static int taos_interrupts_clear(void);
static int prox_calibrate(void);
static int taos_sensors_als_on(void);

DECLARE_WAIT_QUEUE_HEAD(waitqueue_read);
#define ALS_PROX_DEBUG
unsigned int ReadEnable;
struct ReadData {
	unsigned int data;
	unsigned int interrupt;
};
struct ReadData readdata[2];

/* first device number */
static dev_t taos_dev_number;

/* class structure for this device */
struct class *taos_class;

/* module device table */
static struct i2c_device_id taos_idtable[] = {
	{TAOS_DEVICE_ID, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, taos_idtable);

/*board and address info*/
struct i2c_board_info taos_board_info[] = {
	{I2C_BOARD_INFO(TAOS_DEVICE_ID, TAOS_DEVICE_ADDR2),},
};

unsigned short const taos_addr_list[2] = { TAOS_DEVICE_ADDR2, I2C_CLIENT_END };

/*client and device*/
struct i2c_client *my_clientp;
struct i2c_client *bad_clientp[TAOS_MAX_NUM_DEVICES];
static int device_found;

static char pro_buf[4];
static int mcount = 0x0;
static char als_buf[4];
static u16 status = 0x0;
static int ALS_ON = 0x0;
static int PROX_STATE = 0x0;
static int PROX_ON = 0x0;
static int prox_tmp_on = 1;

/* driver definition */
static struct i2c_driver taos_driver = {
	.driver = {
		   .owner = THIS_MODULE,
		   .name = TAOS_DEVICE_NAME,
		   .of_match_table = tmd2771_of_match,
		   },
	.id_table = taos_idtable,
	.probe = taos_probe,
	.remove = __devexit_p(taos_remove),
};

/* per-device data */
struct taos_data {
	struct i2c_client *client;
	struct cdev cdev;
	unsigned int addr;
	struct input_dev *input_dev_als;
	struct input_dev *input_dev_prox;
	struct work_struct work;
	struct wake_lock taos_wake_lock;
	char taos_id;
	char taos_name[TAOS_ID_NAME_SIZE];
	struct mutex update_lock;
	char valid;
	unsigned long last_updated;

} *taos_datap;

static const struct file_operations taos_fops = {
	.owner = THIS_MODULE,
	.open = taos_open,
	.release = taos_release,
	.read = taos_read,
	.write = taos_write,
	.llseek = taos_llseek,
	.unlocked_ioctl = taos_ioctl,
	/*.poll = taos_poll, */
};

/* device configuration */
struct taos_cfg *taos_cfgp;


/*prox info*/
struct taos_prox_info prox_cal_info[32];
struct taos_prox_info prox_cur_info;
struct taos_prox_info *prox_cur_infop = &prox_cur_info;
static u8 prox_history_hi = 0x0;
static u8 prox_history_lo = 0x0;
static struct timer_list prox_poll_timer;

static int device_released = 0x0;
static u16 sat_als = 0x0;
static u16 sat_prox = 0x0;

/* device reg init values */
static u8 taos_triton_reg_init[16] = {
	0x00, 0xFF, 0XFF, 0XFF, 0X00, 0X00, 0XFF, 0XFF, 0X00,
	0X00, 0XFF, 0XFF, 0X00, 0X00, 0X00, 0X00
};

/* lux time scale */
struct time_scale_factor {
	u16 numerator;
	u16 denominator;
	u16 saturation;
};
struct time_scale_factor TritonTime = { 1, 0, 0 };
struct time_scale_factor *lux_timep = &TritonTime;

/* gain table */
static u8 taos_triton_gain_table[] = { 1, 8, 16, 120 };

/* lux data */
struct lux_data {
	u16 ratio;
	u16 clear;
	u16 ir;
};
struct lux_data TritonFN_lux_data[] = {
	{9830, 8320, 15360},
	{12452, 10554, 22797},
	{14746, 6234, 11430},
	{17695, 3968, 6400},
	{0, 0, 0}
};
struct lux_data *lux_tablep = TritonFN_lux_data;
static int lux_history[TAOS_FILTER_DEPTH] = { -ENODATA, -ENODATA, -ENODATA };

static irqreturn_t taos_irq_handler(int irq, void *dev_id)
{
	schedule_work(&taos_datap->work);
	return IRQ_HANDLED;
}

static int taos_get_data(void)
{
	int ret = 0;
	ret = i2c_smbus_write_byte(taos_datap->client,
				   TAOS_TRITON_CMD_REG | 0x13);
	if (ret < 0) {
		pr_taos(ERROR, "TAOS: i2c_smbus_write_byte(1)failed\n");
		return ret;
	}
	status = i2c_smbus_read_byte(taos_datap->client);
	if (mutex_trylock(&taos_datap->update_lock) == 0) {
		pr_taos(INFO, "taos_get_data device isbusy\n");
		return -ERESTARTSYS;
	}
	if ((status & 0x20) == 0x20) {
		/*set_threshold();*/
		ret = taos_prox_threshold_set();
		if (ret >= 0)
			ReadEnable = 1;
	} else if ((status & 0x10) == 0x10) {
		ReadEnable = 1;
		taos_als_threshold_set();
		taos_als_get_data();
	}
	mutex_unlock(&taos_datap->update_lock);
	return ret;
}

static int taos_interrupts_clear(void)
{
	int ret = 0;
	ret = i2c_smbus_write_byte(taos_datap->client, (TAOS_TRITON_CMD_REG
							| TAOS_TRITON_CMD_SPL_FN
							| 0x07));
	if (ret < 0) {
		pr_taos(ERROR, "TAOS: i2c error in taos_interrupts_clear()\n");
		return ret;
	}
	return ret;
}

static void taos_work_func(struct work_struct *work)
{
	taos_get_data();
	taos_interrupts_clear();
}

static int taos_als_get_data(void)
{
	int ret = 0;
	u8 reg_val;
	int lux_val = 0;

	pr_taos(DEBUG, "trace; taos_als_get_data\n");
	ret = i2c_smbus_write_byte(taos_datap->client, TAOS_TRITON_CMD_REG
				   | TAOS_TRITON_CNTRL);
	if (ret < 0) {
		pr_taos(ERROR, "TAOS: i2c_smbus_write_byte fail in %s\n",
			__func__);
		return ret;
	}

	reg_val = i2c_smbus_read_byte(taos_datap->client);
	if ((reg_val & (TAOS_TRITON_CNTL_ADC_ENBL | TAOS_TRITON_CNTL_PWRON)) !=
	    (TAOS_TRITON_CNTL_ADC_ENBL | TAOS_TRITON_CNTL_PWRON)) {
		pr_taos(ERROR, "TAOS: taos_als_get_data  reg_val === %0x\n",
			reg_val);
		return -ENODATA;
	}

	ret = i2c_smbus_write_byte(taos_datap->client, TAOS_TRITON_CMD_REG |
				   TAOS_TRITON_STATUS);
	if (ret < 0) {
		pr_taos(ERROR, "i2c_smbus_write_byte failed status register\n");
		return ret;
	}

	reg_val = i2c_smbus_read_byte(taos_datap->client);
	if ((reg_val & TAOS_TRITON_STATUS_ADCVALID) !=
	    TAOS_TRITON_STATUS_ADCVALID) {
		pr_taos(ERROR, "TAOS: taos_als_get_data  reg_val === %0x\n",
			reg_val);
		return -ENODATA;
	}

	lux_val = taos_get_lux();
	if (lux_val < 0)
		pr_taos(ERROR, "TAOS: call to taos_get_lux() error %d\n",
			lux_val);

	/* lux_val = taos_lux_filter(lux_val); */
	input_report_abs(taos_datap->input_dev_als, ABS_MISC, lux_val);
	input_sync(taos_datap->input_dev_als);
	return ret;
}

static int taos_als_threshold_set(void)
{
	int i, ret = 0;
	u8 chdata[2];
	u16 ch0;
	u16 als_threshold_lo;
	u16 als_threshold_hi;

	for (i = 0; i < 2; i++) {
		chdata[i] = (i2c_smbus_read_byte_data(taos_datap->client,
						      (TAOS_TRITON_CMD_REG |
						       TAOS_TRITON_CMD_AUTO |
						       (TAOS_TRITON_ALS_CHAN0LO
							+ i))));
	}

	ch0 = chdata[0] + chdata[1] * 256;
	als_threshold_hi = (12 * ch0) / 10;
	if (als_threshold_hi >= 65535)
		als_threshold_hi = 65535;
	als_threshold_lo = (8 * ch0) / 10;
	als_buf[0] = als_threshold_lo & 0x0ff;
	als_buf[1] = als_threshold_lo >> 8;
	als_buf[2] = als_threshold_hi & 0x0ff;
	als_buf[3] = als_threshold_hi >> 8;

	pr_taos(DEBUG, "trace : taos_als_threshold_set\n");
	for (mcount = 0; mcount < 4; mcount++) {
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						(TAOS_TRITON_CMD_REG | 0x04) +
						mcount, als_buf[mcount]);
		if (ret < 0) {
			pr_taos(ERROR,
				"TAOS: i2c_smbus_write_byte_data failed in %s\n",
				__func__);
			return ret;
		}
	}

	return ret;
}

static int taos_prox_threshold_set(void)
{
	int i, ret = 0;
	u8 chdata[6];
	u16 proxdata = 0;
	u16 cleardata = 0;
	int data = 0;
	for (i = 0; i < 6; i++) {
		chdata[i] =
		    (i2c_smbus_read_byte_data
		     (taos_datap->client,
		      (TAOS_TRITON_CMD_REG | TAOS_TRITON_CMD_AUTO |
		       (TAOS_TRITON_ALS_CHAN0LO + i))));
	}
	cleardata = chdata[0] + chdata[1] * 256;
	proxdata = chdata[4] + chdata[5] * 256;

	pr_taos(DEBUG, "trace: taos_prox_threshold_set %d\n", proxdata);
	if (prox_tmp_on
		|| proxdata
		< (taos_cfgp->prox_threshold_lo - taos_cfgp->prox_win_sw)) {
		pro_buf[0] = 0x0;
		pro_buf[1] = 0x0;
		pro_buf[2] = taos_cfgp->prox_threshold_hi & 0x0ff;
		pro_buf[3] = taos_cfgp->prox_threshold_hi >> 8;
		data = 1;
		input_report_abs(taos_datap->input_dev_prox, ABS_DISTANCE,
				 data);
		input_sync(taos_datap->input_dev_prox);
		pr_taos(INFO, " proximity: no object detected\n");
	} else if (proxdata
		> (taos_cfgp->prox_threshold_hi + taos_cfgp->prox_win_sw)) {
		pr_taos(DEBUG, "====cleardata = %d sat_als = %d\n",
			cleardata, sat_als);
		if (cleardata > ((sat_als * 80) / 100))
			return -ENODATA;
		pro_buf[0] = taos_cfgp->prox_threshold_lo & 0x0ff;
		pro_buf[1] = taos_cfgp->prox_threshold_lo >> 8;
		pro_buf[2] = 0xff;
		pro_buf[3] = 0xff;
		data = 0;
		input_report_abs(taos_datap->input_dev_prox, ABS_DISTANCE,
				 data);
		input_sync(taos_datap->input_dev_prox);
		pr_taos(DEBUG, "proximity: object detected\n");
	}
	PROX_STATE = data;
	for (mcount = 0; mcount < 4; mcount++) {
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						(TAOS_TRITON_CMD_REG | 0x08) +
						mcount, pro_buf[mcount]);
		if (ret < 0)
			return ret;
	}
	prox_tmp_on = 0;
	return ret;
}

static int __init taos_init(void)
{
	int ret = 0;
	struct device *temp_dev;

	ret = alloc_chrdev_region(&taos_dev_number, 0,
				  TAOS_MAX_NUM_DEVICES, TAOS_DEVICE_NAME);
	if (ret < 0) {
		pr_taos(ERROR,
			"TAOS: alloc_chrdev_region() failed in taos_init()\n");
		goto err_alloc_chrdev;
	}

	taos_class = class_create(THIS_MODULE, TAOS_DEVICE_NAME);
	if (IS_ERR(taos_class)) {
		pr_taos(ERROR, "fail to create class taos\n");
		goto err_create_class;
	}

	taos_datap = kzalloc(sizeof(struct taos_data), GFP_KERNEL);
	if (!taos_datap)
		goto err_kzalloc_taos_datap;
	memset(taos_datap, 0, sizeof(struct taos_data));
	cdev_init(&taos_datap->cdev, &taos_fops);
	taos_datap->cdev.owner = THIS_MODULE;
	ret = cdev_add(&taos_datap->cdev, taos_dev_number, 1);
	if (ret < 0) {
		pr_taos(ERROR, "TAOS: cdev_add() failed in taos_init()\n");
		goto err_cdev_add;
	}
	wake_lock_init(&taos_datap->taos_wake_lock, WAKE_LOCK_SUSPEND,
		       "taos-wake-lock");
	temp_dev = device_create(taos_class, NULL,
				 MKDEV(MAJOR(taos_dev_number), 0), &taos_driver,
				 "taos");
	if (IS_ERR(temp_dev)) {
		pr_taos(ERROR, "cannot create taos dev\n");
		goto err_create_taos_dev;
	}

	ret = i2c_add_driver(&taos_driver);
	if (ret < 0) {
		pr_taos(ERROR,
			"TAOS: i2c_add_driver() failed in taos_init()\n");
		goto err_create_taos_dev;
	}
	return 0;

err_create_taos_dev:
	cdev_del(&taos_datap->cdev);
err_cdev_add:
	kfree(taos_datap);
	taos_datap = NULL;
err_kzalloc_taos_datap:
	class_destroy(taos_class);
err_create_class:
	unregister_chrdev_region(taos_dev_number, TAOS_MAX_NUM_DEVICES);
err_alloc_chrdev:
	return ret;
}

static void __exit taos_exit(void)
{
	if (my_clientp)
		i2c_unregister_device(my_clientp);
	i2c_del_driver(&taos_driver);
	device_destroy(taos_class, MKDEV(MAJOR(taos_dev_number), 0));
	cdev_del(&taos_datap->cdev);
	class_destroy(taos_class);
	unregister_chrdev_region(taos_dev_number, TAOS_MAX_NUM_DEVICES);
	disable_irq(als_ps_int);
	kfree(taos_datap);
}

static int __attribute__ ((unused)) als_calibrate(struct taos_cfg *taos_cfg,
						  struct taos_data *taos_data)
{
	int itime = 0;
	int ret = 0;
	u8 reg_cntrl = 0, reg_val = 0;
	itime = (((taos_cfgp->als_time / 50) * 18) - 1);
	itime = (~itime);
	ret = i2c_smbus_write_byte_data(taos_datap->client, TAOS_TRITON_CMD_REG
					| TAOS_TRITON_ALS_TIME, itime);
	if (ret < 0) {
		pr_taos(ERROR,
			"TAOS: i2c_smbus_write_byte_data failed in %s\n",
			__func__);
		return ret;
	}
	ret = i2c_smbus_write_byte(taos_datap->client, TAOS_TRITON_CMD_REG
				   | TAOS_TRITON_CNTRL);
	if (ret < 0) {
		pr_taos(ERROR,
			"TAOS:i2c_smbus_write_byte failed als_calibrate\n");
		return ret;
	}

	reg_val = i2c_smbus_read_byte(taos_datap->client);
	reg_cntrl =
	    reg_val | (TAOS_TRITON_CNTL_ADC_ENBL | TAOS_TRITON_CNTL_PWRON);
	ret = i2c_smbus_write_byte_data(taos_datap->client, TAOS_TRITON_CMD_REG
					| TAOS_TRITON_CNTRL, reg_cntrl);
	if (ret < 0) {
		pr_taos(ERROR,
			"TAOS: i2c_smbus_write_byte_data failed in als_on\n");
		ret = i2c_smbus_write_byte(taos_datap->client,
					   TAOS_TRITON_CMD_REG |
					   TAOS_TRITON_GAIN);
		if (ret < 0)
			pr_taos(ERROR, "failed in ioctl als_on\n");
		return ret;
	}

	reg_val = i2c_smbus_read_byte(taos_datap->client);
	reg_val = reg_val & 0xFC;
	reg_val = reg_val | (taos_cfgp->gain & 0x03);
	ret = i2c_smbus_write_byte_data(taos_datap->client,
					TAOS_TRITON_CMD_REG
					| TAOS_TRITON_GAIN, reg_val);
	if (ret < 0) {
		pr_taos(ERROR, "TAOS: failed in ioctl als_on\n");
		return ret;
	}
	mdelay(500);

	ret = i2c_smbus_write_byte(taos_datap->client, TAOS_TRITON_CMD_REG |
				   TAOS_TRITON_STATUS);
	if (ret < 0) {
		pr_taos(ERROR,
			"TAOS: i2c_smbus_write_byte failed in als_calibrate\n");
		return ret;
	}

	reg_val = i2c_smbus_read_byte(taos_datap->client);
	if ((reg_val & 0x01) != 0x01)
		return -ENODATA;
	ret = taos_als_get_data();
	return ret;
}

static int prox_calibrate(void)
{
	int i = 0;
	int ret = 0;
	u8 reg_cntrl = 0, reg_val = 0;
	int prox_sum = 0, prox_mean = 0, prox_max = 0;
	ret = i2c_smbus_write_byte_data(taos_datap->client,
					TAOS_TRITON_CMD_REG | 0x01,
					taos_cfgp->prox_int_time);
	if (ret < 0) {
		pr_taos(ERROR,
			"TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
		return ret;
	}

	ret = i2c_smbus_write_byte_data(taos_datap->client,
					(TAOS_TRITON_CMD_REG | 0x02),
					taos_cfgp->prox_adc_time);
	if (ret < 0) {
		pr_taos(ERROR,
			"TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
		return ret;
	}
	ret = i2c_smbus_write_byte_data(taos_datap->client,
					(TAOS_TRITON_CMD_REG | 0x03),
					taos_cfgp->prox_wait_time);
	if (ret < 0) {
		pr_taos(ERROR,
			"TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
		return ret;
	}

	ret = i2c_smbus_write_byte_data(taos_datap->client, TAOS_TRITON_CMD_REG
					| 0x0D, taos_cfgp->prox_config);
	if (ret < 0) {
		pr_taos(ERROR,
			"TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
		return ret;
	}

	ret = i2c_smbus_write_byte_data(taos_datap->client,
					TAOS_TRITON_CMD_REG | 0x0E,
					taos_cfgp->prox_pulse_cnt);
	if (ret < 0) {
		pr_taos(ERROR,
			"TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
		return ret;
	}

	ret = i2c_smbus_write_byte_data(taos_datap->client,
					TAOS_TRITON_CMD_REG | 0x0F,
					taos_cfgp->prox_gain);
	if (ret < 0) {
		pr_taos(ERROR,
			"TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
		return ret;
	}
	reg_cntrl = reg_val | (TAOS_TRITON_CNTL_PROX_DET_ENBL |
			       TAOS_TRITON_CNTL_PWRON |
			       TAOS_TRITON_CNTL_ADC_ENBL);
	ret =
	    i2c_smbus_write_byte_data(taos_datap->client,
				      TAOS_TRITON_CMD_REG | TAOS_TRITON_CNTRL,
				      reg_cntrl);
	if (ret < 0) {
		pr_taos(ERROR,
			"TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
		return ret;
	}
	prox_sum = 0;
	prox_max = 0;
	for (i = 0; i < 32; i++) {
		ret = taos_prox_poll(&prox_cal_info[i]);
		if (ret < 0) {
			pr_taos(ERROR,
				"TAOS: call to prox_poll failed in ioctl prox_calibrate\n");
			return ret;
		}
		prox_sum += prox_cal_info[i].prox_data;
		if (prox_cal_info[i].prox_data > prox_max)
			prox_max = prox_cal_info[i].prox_data;
		msleep(100);
	}
	prox_mean = prox_sum >> 5;
	taos_cfgp->prox_threshold_hi =
	    ((((prox_max - prox_mean) * 200) + 50) / 100) + prox_mean;
	taos_cfgp->prox_threshold_lo =
	    ((((prox_max - prox_mean) * 170) + 50) / 100) + prox_mean;
	if (taos_cfgp->prox_threshold_hi > 600) {
		taos_cfgp->prox_threshold_hi = 600;
		taos_cfgp->prox_threshold_lo = 550;
	}
	if (taos_cfgp->prox_threshold_lo < 100) {
		taos_cfgp->prox_threshold_hi = 200;
		taos_cfgp->prox_threshold_lo = 150;
	}
#ifdef TMD2771_USER_CALIBRATION
	tmd2771_offset = prox_mean;
	pr_taos(INFO, "prox calibrate tmd2771_offset=%d\n", tmd2771_offset);
#endif
	pr_taos(DEBUG, "----------- taos_cfgp->prox_threshold_hi = %d\n",
		taos_cfgp->prox_threshold_hi);
	pr_taos(DEBUG, "----------- taos_cfgp->prox_threshold_lo = %d\n",
		taos_cfgp->prox_threshold_lo);
	ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG |
						TAOS_TRITON_CNTRL, 0x00);
	if (ret < 0)
		pr_taos(ERROR,
			"Power Off FAIL in prox_calibrate ret=%d\n", ret);
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static int __taos_late_resume(struct i2c_client *client)
{
	u8 reg_val;
	int ret = -1;

	pr_taos(DEBUG, "taos late resume\n");

	ret = i2c_smbus_write_byte(taos_datap->client,
					TAOS_TRITON_CMD_REG |
					TAOS_TRITON_CNTRL);
	if (ret < 0) {
		pr_taos(ERROR,
		"TAOS: i2c_smbus_write_byte failed in ioctl als_calibrate\n");
		return ret;
	}

	reg_val = i2c_smbus_read_byte(taos_datap->client);
	if (((reg_val & TAOS_TRITON_CNTL_PROX_DET_ENBL) == 0x0)
		&& (ALS_ON == 1))
		taos_sensors_als_on();

	if (PROX_ON == 1 && ALS_ON == 1)
		reg_val = 0x3f;
	else if (ALS_ON == 1)
		reg_val = 0x13;
	else if (PROX_ON == 1)
		reg_val = 0x2d;
	pr_taos(DEBUG, "TAOS: enabled reg in resume=0x%x\n", reg_val);
	ret = i2c_smbus_write_byte(taos_datap->client, TAOS_TRITON_CMD_REG
					| TAOS_TRITON_CNTRL);
	if (ret < 0) {
		pr_taos(ERROR,
		"TAOS: i2c_smbus_write_byte failed in taos_resume\n");
		return ret;
	}

	ret = i2c_smbus_write_byte_data(taos_datap->client,
				TAOS_TRITON_CMD_REG | TAOS_TRITON_CNTRL,
				reg_val);

	if (ret < 0) {
		pr_taos(ERROR,
		"TAOS: i2c_smbus_write_byte_data failed in ioctl als_off\n");
		return ret;
	}

	return ret;
}

static int __taos_early_suspend(struct i2c_client *client, pm_message_t mesg)
{
	u8 reg_val = 0, reg_cntrl = 0;
	int ret = -1;

	pr_taos(DEBUG, "taos enter suspend\n");
	if (isPsensorLocked)
		return -1;

	ret = i2c_smbus_write_byte(taos_datap->client, TAOS_TRITON_CMD_REG
				   | TAOS_TRITON_CNTRL);
	if (ret < 0) {
		pr_taos(ERROR,
			"TAOS: i2c_smbus_write_byte failed in taos_resume\n");

		return ret;
	}
	reg_val = i2c_smbus_read_byte(taos_datap->client);
	if (~(reg_val & TAOS_TRITON_CNTL_PROX_DET_ENBL)) {
		reg_cntrl = reg_val & (~TAOS_TRITON_CNTL_PWRON);
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG |
						TAOS_TRITON_CNTRL, reg_cntrl);
		if (ret < 0) {
			pr_taos(ERROR,
				"TAOS: i2c_smbus_write_byte_data failed in taos_suspend\n");

			return ret;
		}
	}

	return ret;
}

static void taos_early_suspend(struct early_suspend *h)
{
	pm_message_t mesg = {.event = PM_EVENT_SUSPEND, };
	__taos_early_suspend(taos_datap->client, mesg);
}

static void taos_late_resume(struct early_suspend *h)
{
	__taos_late_resume(taos_datap->client);
}

static struct early_suspend taos_early_suspend_desc = {
	.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN,
	.suspend = taos_early_suspend,
	.resume = taos_late_resume,
};
#endif

static int taos_i2c_read(struct i2c_client *client, u8 reg, u8 *val)
{
	int err = -EIO;

	/* select register to write */
	err = i2c_smbus_write_byte(client, (TAOS_TRITON_CMD_REG | reg));
	if (err < 0) {
		pr_taos(ERROR, "failed to write cmd register 0x%x, ret: %d\n",
			reg, err);

		goto smbus_write_err;
	}

	/* read the data */
	err = i2c_smbus_read_byte(client);
	if (err >= 0) {
		*val = (u8) err;
	} else {
		pr_taos(ERROR, "failed to read register: 0x%x,ret:%d\n",
			reg, err);

		goto smbus_write_err;
	}

smbus_write_err:
	return err;
}
#ifdef TMD2771_USER_CALIBRATION
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
		prox_calibrate();
	return count;
}
static DEVICE_ATTR(prox_cali, 0644, NULL, prox_cali_store);

static ssize_t tmd2771_get_offset(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%d\n", tmd2771_offset);
}

static ssize_t tmd2771_set_offset(struct device *dev,
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
	tmd2771_offset = x;
	hi = taos_cfgp->prox_threshold_hi_def + tmd2771_offset
			- taos_cfgp->prox_offset;
	lo = taos_cfgp->prox_threshold_lo_def + tmd2771_offset
			- taos_cfgp->prox_offset;
	pr_taos(INFO,
		"tmd2771_offset=%d,prox_offset=%d, hi =%d, lo=%d\n",
		tmd2771_offset, taos_cfgp->prox_offset, hi, lo);
	if (hi > 0xfffe || lo > 0xfffe ||
		(tmd2771_offset-taos_cfgp->prox_offset) < 0) {
		tmd2771_offset = 0;
		pr_taos(ERROR, "isl290xx sw cali failed\n");
	} else {
		if (lo > tmd2771_offset) {
			taos_cfgp->prox_threshold_hi = hi;
			taos_cfgp->prox_threshold_lo = lo;
		} else {
			for (i = 0; i < 100; i++) {
				hi++;
				lo++;
				if (lo > tmd2771_offset) {
					taos_cfgp->prox_threshold_hi
						= hi + 1;
					taos_cfgp->prox_threshold_lo
						= lo + 1;
					break;
				}
			}
		}
	}
	return count;
}
static DEVICE_ATTR(offset,  S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	tmd2771_get_offset, tmd2771_set_offset);
#endif
static ssize_t taos_power_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "\nprox: %s, light: %s\n\n",
		       PROX_ON ? "on" : "off", ALS_ON ? "on" : "off");
}

static ssize_t taos_state_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "\nproximity:%s detect object\n\n",
		       PROX_STATE ? "no" : " ");
}



static ssize_t taos_reg_show(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	unsigned i, n, reg_count;
	u8 value;
	reg_count = sizeof(tmd2771x_regs) / sizeof(tmd2771x_regs[0]);
	for (i = 0, n = 0; i < reg_count; i++) {
		taos_i2c_read(taos_datap->client, tmd2771x_regs[i].reg, &value);
		n += scnprintf(buf + n, PAGE_SIZE - n,
			"%-20s = 0x%02X\n",
			tmd2771x_regs[i].name,
			value);
	}
	return n;
}


static ssize_t taos_reg_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	unsigned i, reg_count, value;
	int ret;
	u8 prox_lo;
	u8 prox_hi;
	char name[30];
	prox_lo = 0;
	prox_hi = 0;
	if (count >= 30)
		return -EFAULT;
	if (sscanf(buf, "%30s %x", name, &value) != 2) {
		pr_err("input invalid\n");
		return -EFAULT;
	}
	reg_count = sizeof(tmd2771x_regs) / sizeof(tmd2771x_regs[0]);
	for (i = 0; i < reg_count; i++) {
		if (!strcmp(name, tmd2771x_regs[i].name)) {
			switch (tmd2771x_regs[i].reg) {
			case TAOS_TRITON_PRX_MINTHRESHLO:
				taos_i2c_read(taos_datap->client,
					TAOS_TRITON_PRX_MINTHRESHHI,
					&prox_hi);
				taos_cfgp->prox_threshold_lo = value
					+ (prox_hi << 8);
				break;
			case TAOS_TRITON_PRX_MINTHRESHHI:
				taos_i2c_read(taos_datap->client,
					TAOS_TRITON_PRX_MINTHRESHLO, &prox_lo);
				taos_cfgp->prox_threshold_lo = (value << 8)
					+ prox_lo;
				break;
			case TAOS_TRITON_PRX_MAXTHRESHLO:
				taos_i2c_read(taos_datap->client,
					TAOS_TRITON_PRX_MAXTHRESHHI,
					&prox_hi);
				taos_cfgp->prox_threshold_hi = value
					+ (prox_hi << 8);
				break;
			case TAOS_TRITON_PRX_MAXTHRESHHI:
				taos_i2c_read(taos_datap->client,
					TAOS_TRITON_PRX_MAXTHRESHLO,
					&prox_lo);
				taos_cfgp->prox_threshold_hi = (value << 8)
					+ prox_lo;
				break;
			default:
				break;
			}
			ret = i2c_smbus_write_byte_data(taos_datap->client,
					tmd2771x_regs[i].reg
					| TAOS_TRITON_CMD_REG, value);
			if (ret) {
				pr_taos(ERROR,
					"Failed to write register %s\n",
					name);
				return -EFAULT;
			}
			return count;
		}
	}
	pr_taos(INFO, "no such register %s\n", name);
	return -EFAULT;
}

static ssize_t prox_enable_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{

	int enable;
	int ret;
	u8 reg_cntrl;

	ret = kstrtoint(buf, 0, &enable);
	if (ret)
		return ret;
	if (enable) {
		if (PROX_ON) {
			pr_taos(INFO, "prox_enable: prox already on.\n");
			return 0;
		}
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG | 0x01,
						taos_cfgp->prox_int_time);
		if (ret < 0)
			return ret;
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG | 0x02,
						taos_cfgp->prox_adc_time);
		if (ret < 0)
			return ret;
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG | 0x03,
						taos_cfgp->prox_wait_time);
		if (ret < 0)
			return ret;
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG | 0x0C,
						taos_cfgp->prox_intr_filter);
		if (ret < 0)
			return ret;
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG | 0x0D,
						taos_cfgp->prox_config);
		if (ret < 0)
			return ret;
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG | 0x0E,
						taos_cfgp->prox_pulse_cnt);
		if (ret < 0)
			return ret;
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG | 0x0F,
						taos_cfgp->prox_gain);
		if (ret < 0)
			return ret;

		/* enable the irq of als when prox on */
		reg_cntrl = TAOS_TRITON_CNTL_PROX_DET_ENBL |
		    TAOS_TRITON_CNTL_PWRON |
		    TAOS_TRITON_CNTL_PROX_INT_ENBL |
		    TAOS_TRITON_CNTL_ADC_ENBL |
		    TAOS_TRITON_CNTL_WAIT_TMR_ENBL |
		    TAOS_TRITON_CNTL_ALS_INT_ENBL;

		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG |
						TAOS_TRITON_CNTRL, reg_cntrl);
		if (ret < 0)
			return ret;
		PROX_ON = 1;
	} else {
		if (PROX_ON == 0) {
			pr_taos(INFO,
				"prox_on: prox already off.\n");
			return 0;
		}
		ret = i2c_smbus_write_byte_data(taos_datap->client,
					TAOS_TRITON_CMD_REG |
					TAOS_TRITON_CNTRL, 0x00);
		if (ret < 0)
			return ret;
		if (ALS_ON == 1)
			taos_sensors_als_on();
		PROX_ON = 0;
	}
	return count;
}

static DEVICE_ATTR(prox_enable, 0644, NULL, prox_enable_store);
static DEVICE_ATTR(taos_power, 0444, taos_power_show, NULL);
static DEVICE_ATTR(taos_state, 0444, taos_state_show, NULL);
static DEVICE_ATTR(taos_reg, 0644, taos_reg_show, taos_reg_store);

static struct attribute *taos_prox_attr[] = {
	&dev_attr_taos_power.attr,
	&dev_attr_taos_state.attr,
	&dev_attr_taos_reg.attr,
#ifdef TMD2771_USER_CALIBRATION
	&dev_attr_prox_cali.attr,
	&dev_attr_offset.attr,
#endif
	&dev_attr_prox_enable.attr,
	NULL,
};

static struct attribute_group taos_prox_attr_grp = {
	.attrs = taos_prox_attr,
};


#ifdef TMD2771_USER_CALIBRATION
static void tmd2771_prox_boot_cali(void)
{
	int hi;
	int lo;
	int i;
	pr_taos(INFO, "prox. auto calibration in progress\n");
	prox_calibrate();
	hi =  taos_cfgp->prox_threshold_hi_def + tmd2771_offset
			- taos_cfgp->prox_offset;
	lo = taos_cfgp->prox_threshold_lo_def + tmd2771_offset
			- taos_cfgp->prox_offset;
	pr_taos(INFO, "tmd2771_offset=%d,prox_offset=%d, hi =%d, lo=%d\n",
		tmd2771_offset, taos_cfgp->prox_offset, hi, lo);

	if (hi > 0xfffe || lo > 0xfffe ||
		(tmd2771_offset-taos_cfgp->prox_offset) < 0) {
		tmd2771_offset = 0;
		pr_taos(ERROR, "tmd2771 boot cali failed\n");
	} else {
		if (lo > tmd2771_offset) {
			taos_cfgp->prox_threshold_hi = hi;
			taos_cfgp->prox_threshold_lo = lo;
		} else {
			for (i = 0; i < 60; i++) {
				hi += 2;
				lo += 2;
				if (lo > tmd2771_offset) {
					taos_cfgp->prox_threshold_hi
						= hi + 1;
					taos_cfgp->prox_threshold_lo
						= lo + 1;
					break;
				}
			}
		}
	}
}
#endif
static int taos_probe(struct i2c_client *clientp,
		      const struct i2c_device_id *idp)
{
	int ret = 0;
	int i = 0;
	int chip_id;
	unsigned char buf[TAOS_MAX_DEVICE_REGS];
	char *device_name;
	struct tmd2771_platform_data *pdata = NULL;
	struct device_node *np;
	u32 val = 0;
#ifdef TMD2771_USER_CALIBRATION
	tmd2771_offset = 0;
#endif
	if (!i2c_check_functionality(clientp->adapter,
				     I2C_FUNC_SMBUS_BYTE_DATA)) {
		pr_taos(ERROR, "i2c smbus byte data functions unsupported\n");
		ret = -EOPNOTSUPP;
		/*goto err_i2c_check_function; */
	}

	if (!i2c_check_functionality(clientp->adapter,
				     I2C_FUNC_SMBUS_WORD_DATA)) {
		pr_taos(ERROR, "i2c smbus word data functions unsupported\n");
	}

	if (!i2c_check_functionality
	    (clientp->adapter, I2C_FUNC_SMBUS_BLOCK_DATA)) {
		pr_taos(ERROR,
			"TAOS: taos_probe() - i2c smbus block data functions unsupported\n");
	}
	for (i = 0; i < I2C_RETRY_COUNTS; i++) {
		chip_id = i2c_smbus_read_byte_data(clientp,
					(TAOS_TRITON_CMD_REG |
					(TAOS_TRITON_CNTRL + 0x12)));
		if (chip_id < 0) {
			pr_taos(ERROR, "get chip id fail with tries %d\n",
				i);
			msleep_interruptible(I2C_RETRY_DELAY);
			if (i == (I2C_RETRY_DELAY-1)) {
				ret = -ENODEV;
				goto err_i2c_check_function;
			}
		} else {
			pr_taos(INFO,
				"get chip id sucessfully with tries %d", i);
			break;
		}
	}
	taos_datap->client = clientp;
	i2c_set_clientdata(clientp, taos_datap);
	INIT_WORK(&(taos_datap->work), taos_work_func);
	mutex_init(&taos_datap->update_lock);

	taos_datap->input_dev_als = input_allocate_device();
	if (!taos_datap->input_dev_als) {
		pr_taos(ERROR, "fail to alloc input device for taos light\n");
		ret = -ENOMEM;
		goto err_alloc_input_taos_light;
	}

	taos_datap->input_dev_prox = input_allocate_device();
	if (!taos_datap->input_dev_als) {
		pr_taos(ERROR, "fail to alloc input device for taos proxim\n");
		ret = -ENOMEM;
		goto err_alloc_input_taos_prox;
	}

	taos_datap->input_dev_als->name = TAOS_INPUT_NAME_ALS;
	taos_datap->input_dev_als->id.bustype = BUS_I2C;

	taos_datap->input_dev_prox->name = TAOS_INPUT_NAME_PROX;
	taos_datap->input_dev_prox->id.bustype = BUS_I2C;
	set_bit(EV_ABS, taos_datap->input_dev_als->evbit);
	set_bit(EV_ABS, taos_datap->input_dev_prox->evbit);
	input_set_abs_params(taos_datap->input_dev_als, ABS_MISC, 0, 255, 0, 1);
	input_set_abs_params(taos_datap->input_dev_prox,
			     ABS_DISTANCE, 0, 1000, 0, 0);
	ret = input_register_device(taos_datap->input_dev_als);
	if (0 != ret) {
		pr_taos(ERROR, "fail to register input for taos als\n");
		ret = -ENOMEM;
		goto err_register_input_als;
	}
	ret = input_register_device(taos_datap->input_dev_prox);
	if (0 != ret) {
		pr_taos(ERROR, "fail to register input for taos prox\n");
		ret = -ENOMEM;
		goto err_register_input_prox;
	}

	for (i = 0; i < TAOS_MAX_DEVICE_REGS; i++) {
		ret = i2c_smbus_write_byte(clientp, TAOS_TRITON_CMD_REG |
					   (TAOS_TRITON_CNTRL + i));
		if (ret < 0) {
			pr_taos(ERROR,
				"write control reg failed in taos_probe()\n");
			goto err_register_input_prox;
		}
		buf[i] = i2c_smbus_read_byte(clientp);
	}
	ret = taos_device_name(buf, &device_name);
	if (ret == 0) {
		pr_taos(ERROR,
			"TAOS: chip id that was read found mismatched by taos_device_name()\n");
		ret = -ENODEV;
		goto err_register_input_prox;
	}
	if (strcmp(device_name, TAOS_DEVICE_ID)) {
		pr_taos(ERROR,
			"TAOS: chip id that was read does not match expected id in taos_probe()\n");
		ret = -ENODEV;
		goto err_register_input_prox;
	}

	device_found = 1;
	ret = i2c_smbus_write_byte(clientp, TAOS_TRITON_CMD_REG |
				   TAOS_TRITON_CNTRL);
	if (ret < 0) {
		pr_taos(ERROR,
			"TAOS: i2c_smbus_write_byte() to control reg failed in taos_probe()\n");
		ret = -EIO;
		goto err_register_input_prox;
	}
	strlcpy(clientp->name, TAOS_DEVICE_ID, I2C_NAME_SIZE);
	strlcpy(taos_datap->taos_name, TAOS_DEVICE_ID, TAOS_ID_NAME_SIZE);
	taos_datap->valid = 0;
	taos_cfgp = kzalloc(sizeof(struct taos_cfg), GFP_KERNEL);
	if (!taos_cfgp) {
		pr_taos(ERROR,
			"TAOS: kmalloc for struct taos_cfg failed in taos_probe()\n");
		ret = -ENOMEM;
		goto err_kzalloc_taos_cfgp;
	}
	ret = i2c_smbus_write_byte_data(taos_datap->client,
					TAOS_TRITON_CMD_REG | 0x00, 0x00);
	if (ret < 0) {
		pr_taos(ERROR, "TAOS: failed in power down\n");
		ret = -EIO;
		goto err_write_ctr_reg;
	}
	if (clientp->dev.platform_data)
		pdata = clientp->dev.platform_data;
	else {
		if (taos_datap->client->irq)
			val = taos_datap->client->irq;
		else {
			np = taos_datap->client->dev.of_node;
			ret = of_property_read_u32(np,
				"calibrate_target_param", &val);
			if (ret)
				goto err_read;
			taos_cfgp->calibrate_target = val;
			ret = of_property_read_u32(np,
				"als_time_param", &val);
			if (ret)
				goto err_read;
			taos_cfgp->als_time = val;
			ret = of_property_read_u32(np,
				"scale_factor_param", &val);
			if (ret)
				goto err_read;
			taos_cfgp->scale_factor = val;
			ret = of_property_read_u32(np,
				"gain_trim_param", &val);
			if (ret)
				goto err_read;
			taos_cfgp->gain_trim = val;
			ret = of_property_read_u32(np,
				"filter_history_param", &val);
			if (ret)
				goto err_read;
			taos_cfgp->filter_history = val;
			ret = of_property_read_u32(np,
				"filter_count_param", &val);
			if (ret)
				goto err_read;
			taos_cfgp->filter_count = val;
			ret = of_property_read_u32(np,
				"gain_param", &val);
			if (ret)
				goto err_read;
			taos_cfgp->gain = val;
			ret = of_property_read_u32(np,
				"prox_threshold_hi_param", &val);
			if (ret)
				goto err_read;
			taos_cfgp->prox_threshold_hi = val;
#ifdef TMD2771_USER_CALIBRATION
			taos_cfgp->prox_threshold_hi_def = val;
#endif
			ret = of_property_read_u32(np,
				"prox_threshold_lo_param", &val);
			if (ret)
				goto err_read;
			taos_cfgp->prox_threshold_lo = val;
#ifdef TMD2771_USER_CALIBRATION
			taos_cfgp->prox_threshold_lo_def = val;
#endif
			ret = of_property_read_u32(np,
				"als_threshold_hi_param", &val);
			if (ret)
				goto err_read;
			taos_cfgp->als_threshold_hi = val;
			ret = of_property_read_u32(np,
				"als_threshold_lo_param", &val);
			if (ret)
				goto err_read;
			taos_cfgp->als_threshold_lo = val;
			ret = of_property_read_u32(np,
				"prox_int_time_param", &val);
			if (ret)
				goto err_read;
			taos_cfgp->prox_int_time = val;
			ret = of_property_read_u32(np,
				"prox_adc_time_param", &val);
			if (ret)
				goto err_read;
			taos_cfgp->prox_adc_time = val;
			ret = of_property_read_u32(np,
				"prox_wait_time_param", &val);
			if (ret)
				goto err_read;
			taos_cfgp->prox_wait_time = val;
			ret = of_property_read_u32(np,
				"prox_intr_filter_param", &val);
			if (ret)
				goto err_read;
			taos_cfgp->prox_intr_filter = val;
			ret = of_property_read_u32(np,
				"prox_config_param", &val);
			if (ret)
				goto err_read;
			taos_cfgp->prox_config = val;
			ret = of_property_read_u32(np,
				"prox_pulse_cnt_param", &val);
			if (ret)
				goto err_read;
			taos_cfgp->prox_pulse_cnt = val;
			ret = of_property_read_u32(np,
				"prox_gain_param", &val);
			if (ret)
				goto err_read;
			taos_cfgp->prox_gain = val;
			ret = of_property_read_u32(np,
				"prox_win_sw", &val);
			if (ret)
				goto err_read;
			taos_cfgp->prox_win_sw = val;
			ret = of_property_read_u32(np,
				"gpio-irq-pin", &val);
			if (ret)
				goto err_read;
#ifdef TMD2771_USER_CALIBRATION
		ret = of_property_read_u32(np,
			"prox_offset_param", &val);
		if (ret)
			taos_cfgp->prox_offset = 0;
		else
			taos_cfgp->prox_offset = val;
		ret = of_property_read_u32(np,
			"prox_boot_cali", &val);
		if (ret)
			taos_cfgp->prox_boot_cali = 0;
		else
			taos_cfgp->prox_boot_cali = val;
#endif
		}
		als_ps_int = gpio_to_irq(val);
		als_ps_gpio_inr = val;
	}
	sat_als = (256 - taos_cfgp->prox_int_time) << 10;
	sat_prox = (256 - taos_cfgp->prox_adc_time) << 10;
	ret = gpio_request(als_ps_gpio_inr, "ALS_PS_INT");
	if (ret < 0) {
		pr_taos(ERROR, "failed to request GPIO:%d,ERRNO:%d\n",
			(int)als_ps_gpio_inr, ret);
		goto err_gpio_request;
	}
	gpio_direction_input(als_ps_gpio_inr);

	ret = request_threaded_irq(als_ps_int, NULL, &taos_irq_handler,
				   IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
				   "taos_irq", taos_datap);
	if (ret != 0) {
		pr_taos(ERROR, "request irq fail for taos\n");
		goto err_request_irq;
	}
#ifdef CONFIG_HAS_EARLYSUSPEND
	register_early_suspend(&taos_early_suspend_desc);
#endif

	ret = sysfs_create_group(&clientp->dev.kobj, &taos_prox_attr_grp);
	if (0 != ret)
		pr_taos(ERROR, "sysfs create debug node fail, ret: %d\n", ret);

	pr_taos(INFO, "device tmd27713 probe scuccess, chip id: 0x%x\n",
		buf[0x12]);
	return 0;
err_read:
	free_irq(als_ps_int, taos_datap);
err_request_irq:
	gpio_free(als_ps_gpio_inr);
err_gpio_request:
err_write_ctr_reg:
	kfree(taos_cfgp);
	taos_cfgp = NULL;
err_kzalloc_taos_cfgp:
	input_unregister_device(taos_datap->input_dev_prox);
err_register_input_prox:
	input_unregister_device(taos_datap->input_dev_als);
err_register_input_als:
	input_free_device(taos_datap->input_dev_prox);
err_alloc_input_taos_prox:
	input_free_device(taos_datap->input_dev_als);
err_alloc_input_taos_light:
err_i2c_check_function:
	return ret;
}

static int __devexit taos_remove(struct i2c_client *client)
{
	int ret = 0;

	sysfs_remove_group(&client->dev.kobj, &taos_prox_attr_grp);
	free_irq(als_ps_int, taos_datap);
	gpio_free(als_ps_gpio_inr);
	kfree(taos_cfgp);
	taos_cfgp = NULL;
	input_unregister_device(taos_datap->input_dev_prox);
	input_unregister_device(taos_datap->input_dev_als);
	input_free_device(taos_datap->input_dev_prox);
	input_free_device(taos_datap->input_dev_als);
	return ret;
}

static int taos_open(struct inode *inode, struct file *file)
{
	struct taos_data *taos_datap;
	int ret = 0;
	device_released = 0;
	pr_taos(DEBUG, "TAOS:********** taos_open **********\n");
	taos_datap = container_of(inode->i_cdev, struct taos_data, cdev);
	if (strcmp(taos_datap->taos_name, TAOS_DEVICE_ID) != 0) {
		pr_taos(ERROR,
			"device name incorrect during taos_open(),get %s\n",
			taos_datap->taos_name);
		ret = -ENODEV;
	}

	memset(readdata, 0, sizeof(struct ReadData) * 2);
	disable_irq(als_ps_int);
	enable_irq(als_ps_int);
	return ret;
}

static int taos_release(struct inode *inode, struct file *file)
{
	struct taos_data *taos_datap;
	int ret = 0;
	device_released = 1;
	PROX_ON = 0;
	prox_history_hi = 0;
	prox_history_lo = 0;
	taos_datap = container_of(inode->i_cdev, struct taos_data, cdev);
	if (strcmp(taos_datap->taos_name, TAOS_DEVICE_ID) != 0) {
		pr_taos(ERROR, "TAOS: device name incorrect during get %s\n",
			taos_datap->taos_name);
		ret = -ENODEV;
	}
	return ret;
}

static int taos_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	unsigned long flags;
	int realmax;
	int err;
	if ((!ReadEnable) && (file->f_flags & O_NONBLOCK))
		return -EAGAIN;
	local_save_flags(flags);
	local_irq_disable();
	realmax = 0;
	if (mutex_trylock(&taos_datap->update_lock) == 0) {
		pr_taos(ERROR, "taos_get_data device isbusy\n");
		return -ERESTARTSYS;
	}
	if (ReadEnable > 0) {
		if (sizeof(struct ReadData) * 2 < count)
			realmax = sizeof(struct ReadData) * 2;
		else
			realmax = count;
		err = copy_to_user(buf, readdata, realmax);
		if (err)
			return -EAGAIN;
		ReadEnable = 0;
	}
	mutex_unlock(&taos_datap->update_lock);
	memset(readdata, 0, sizeof(struct ReadData) * 2);
	local_irq_restore(flags);
	return realmax;
}

static int taos_write(struct file *file, const char *buf, size_t count,
		      loff_t *ppos)
{
	struct taos_data *taos_datap;
	u8 i = 0, xfrd = 0, reg = 0;
	u8 my_buf[TAOS_MAX_DEVICE_REGS];
	int ret = 0;
	if ((*ppos < 0) || (*ppos >= TAOS_MAX_DEVICE_REGS)
	    || ((*ppos + count) > TAOS_MAX_DEVICE_REGS)) {
		pr_taos(ERROR, "reg limit check failed in taos_write()\n");
		return -EINVAL;
	}
	reg = (u8)*ppos;
	ret = copy_from_user(my_buf, buf, count);
	if (ret) {
		pr_taos(ERROR, "copy_to_user failed in taos_write()\n");
		return -ENODATA;
	}
	taos_datap =
	    container_of(file->f_dentry->d_inode->i_cdev, struct taos_data,
			 cdev);
	while (xfrd < count) {
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG | reg,
						my_buf[i++]);
		if (ret < 0) {
			pr_taos(ERROR,
				"TAOS: i2c_smbus_write_byte_data failed in taos_write()\n");
			return ret;
		}
		reg++;
		xfrd++;
	}
	return (int)xfrd;
}

static loff_t taos_llseek(struct file *file, loff_t offset, int orig)
{
	int ret = 0;
	loff_t new_pos = 0;
	if ((offset >= TAOS_MAX_DEVICE_REGS) || (orig < 0) || (orig > 1)) {
		pr_taos(ERROR,
			"TAOS: offset param limit or origin limit check failed in taos_llseek()\n");
		return -EINVAL;
	}
	switch (orig) {
	case 0:
		new_pos = offset;
		break;
	case 1:
		new_pos = file->f_pos + offset;
		break;
	}
	if ((new_pos < 0) || (new_pos >= TAOS_MAX_DEVICE_REGS) || (ret < 0)) {
		pr_taos(ERROR,
			"TAOS: new offset limit or origin limit check failed in taos_llseek()\n");
		return -EINVAL;
	}
	file->f_pos = new_pos;
	return new_pos;
}

static int taos_sensors_als_on(void)
{
	int ret = 0, i = 0;
	u8 itime = 0, reg_val = 0, reg_cntrl = 0;
	for (i = 0; i < TAOS_FILTER_DEPTH; i++)
		lux_history[i] = -ENODATA;
	ret = i2c_smbus_write_byte(taos_datap->client,
				   TAOS_TRITON_CMD_REG | TAOS_TRITON_CMD_SPL_FN
				   | TAOS_TRITON_CMD_ALS_INTCLR);
	if (ret < 0) {
		pr_taos(ERROR,
			"TAOS: i2c_smbus_write_byte failed in ioctl als_on\n");
		return ret;
	}
	itime = (((taos_cfgp->als_time / 50) * 18) - 1);
	itime = (~itime);
	ret = i2c_smbus_write_byte_data(taos_datap->client,
					TAOS_TRITON_CMD_REG |
					TAOS_TRITON_ALS_TIME, itime);
	if (ret < 0) {
		pr_taos(ERROR,
			"TAOS: i2c_smbus_write_byte_data failed in ioctl als_on\n");
		return ret;
	}
	ret = i2c_smbus_write_byte_data(taos_datap->client,
					TAOS_TRITON_CMD_REG |
					TAOS_TRITON_INTERRUPT,
					taos_cfgp->prox_intr_filter);
	if (ret < 0) {
		pr_taos(ERROR,
			"TAOS: i2c_smbus_write_byte_data failed in ioctl als_on\n");
		return ret;
	}
	ret = i2c_smbus_write_byte(taos_datap->client,
				   TAOS_TRITON_CMD_REG | TAOS_TRITON_GAIN);
	if (ret < 0) {
		pr_taos(ERROR,
			"TAOS: i2c_smbus_write_byte failed in ioctl als_on\n");
		return ret;
	}
	reg_val = i2c_smbus_read_byte(taos_datap->client);
	reg_val = reg_val & 0xFC;
	reg_val = reg_val | (taos_cfgp->gain & 0x03);
	ret = i2c_smbus_write_byte_data(taos_datap->client,
					TAOS_TRITON_CMD_REG |
					TAOS_TRITON_GAIN, reg_val);
	if (ret < 0) {
		pr_taos(ERROR,
			"TAOS: i2c_smbus_write_byte_data failed in ioctl als_on\n");
		return ret;
	}
	reg_cntrl = (TAOS_TRITON_CNTL_ADC_ENBL | TAOS_TRITON_CNTL_PWRON |
		     TAOS_TRITON_CNTL_ALS_INT_ENBL);
	ret = i2c_smbus_write_byte_data(taos_datap->client,
					TAOS_TRITON_CMD_REG |
					TAOS_TRITON_CNTRL, reg_cntrl);
	if (ret < 0) {
		pr_taos(ERROR,
			"TAOS: i2c_smbus_write_byte_data failed in ioctl als_on\n");
		return ret;
	}
	taos_als_threshold_set();
	pr_taos(DEBUG, " taos_als_threshold_set succesful\n");
	return ret;
}

static long taos_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct taos_data *taos_datap;
	u8 itime;
	int prox_sum = 0, prox_mean = 0, prox_max = 0;
	int lux_val = 0, ret = 0, i = 0, tmp = 0;
	u8 reg_val = 0, reg_cntrl = 0;
	int ret_check = 0;
	int ret_m = 0;
	u8 reg_val_temp = 0;
	itime = 0;
	taos_datap = container_of(file->f_dentry->d_inode->i_cdev,
				  struct taos_data, cdev);
	switch (cmd) {
	case TAOS_IOCTL_SENSOR_CHECK:
		reg_val_temp = 0;
		ret = i2c_smbus_write_byte(taos_datap->client,
					   TAOS_TRITON_CMD_REG |
					   TAOS_TRITON_CNTRL);
		if (ret < 0) {
			pr_taos(ERROR, "TAOS_IOCTL_SENSOR_CHECK failed\n");
			return ret;
		}
		reg_val_temp = i2c_smbus_read_byte(taos_datap->client);
		pr_taos(INFO, "TAOS_IOCTL_SENSOR_CHECK,prox_adc_time,%d~\n",
			reg_val_temp);
		if ((reg_val_temp & 0xFF) == 0xF)
			return -ENODATA;
		break;

	case TAOS_IOCTL_SENSOR_CONFIG:
		ret = copy_from_user(taos_cfgp, (struct taos_cfg *)arg,
				     sizeof(struct taos_cfg));
		if (ret) {
			pr_taos(ERROR,
				"TAOS: copy_from_user failed in ioctl config_set\n");
			return -ENODATA;
		}
		break;

	case TAOS_IOCTL_SENSOR_ON:
		ret = 0;
		reg_val = 0;
		ret_m = 0;
		for (i = 0; i < TAOS_FILTER_DEPTH; i++)
			lux_history[i] = -ENODATA;
		ret = i2c_smbus_write_byte(taos_datap->client,
					   TAOS_TRITON_CMD_REG |
					   TAOS_TRITON_CMD_SPL_FN |
					   TAOS_TRITON_CMD_ALS_INTCLR);
		if (ret < 0) {
			pr_taos(ERROR,
				"TAOS: i2c_smbus_write_byte failed in ioctl als_on\n");
			return ret;
		}

		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG |
						TAOS_TRITON_ALS_TIME,
						taos_cfgp->prox_int_time);
		if (ret < 0) {
			pr_taos(ERROR,
				"TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
			return ret;
		}

		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG |
						TAOS_TRITON_PRX_TIME,
						taos_cfgp->prox_adc_time);
		if (ret < 0) {
			pr_taos(ERROR,
				"TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
			return ret;
		}

		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG |
						TAOS_TRITON_WAIT_TIME,
						taos_cfgp->prox_wait_time);
		if (ret < 0) {
			pr_taos(ERROR,
				"TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
			return ret;
		}

		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG |
						TAOS_TRITON_INTERRUPT,
						taos_cfgp->prox_intr_filter);
		if (ret < 0) {
			pr_taos(ERROR,
				"TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
			return ret;
		}

		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG |
						TAOS_TRITON_PRX_CFG,
						taos_cfgp->prox_config);
		if (ret < 0) {
			pr_taos(ERROR,
				"TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
			return ret;
		}

		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG |
						TAOS_TRITON_PRX_COUNT,
						taos_cfgp->prox_pulse_cnt);
		if (ret < 0) {
			pr_taos(ERROR,
				"TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
			return ret;
		}
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG |
						TAOS_TRITON_GAIN,
						taos_cfgp->prox_gain);
		if (ret < 0) {
			pr_taos(ERROR,
				"TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
			return ret;
		}
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG |
						TAOS_TRITON_CNTRL, 0xF);
		if (ret < 0) {
			pr_taos(ERROR,
				"TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
			return ret;
		}
		break;

	case TAOS_IOCTL_SENSOR_OFF:
		ret = 0;
		reg_val = 0;
		ret_check = 0;
		ret_m = 0;
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG |
						0x00, 0x00);
		if (ret < 0) {
			pr_taos(ERROR,
				"TAOS: i2c_smbus_write_byte_data failed in ioctl prox_off\n");
			return ret;
		}
		break;

	case TAOS_IOCTL_ALS_ON:
		pr_taos(DEBUG, "ioctl: TAOS_IOCTL_ALS_ON\n");
		if (PROX_ON == 1) {
			ALS_ON = 1;
			return ret;
		}

		ret = i2c_smbus_write_byte(taos_datap->client,
					   TAOS_TRITON_CMD_REG |
					   TAOS_TRITON_CNTRL);
		if (ret < 0) {
			pr_taos(ERROR,
				"TAOS: i2c_smbus_write_byte failed in ioctl als_calibrate\n");
			return ret;
		}
		reg_val = i2c_smbus_read_byte(taos_datap->client);
		if ((reg_val & TAOS_TRITON_CNTL_PROX_DET_ENBL) == 0x0)
			taos_sensors_als_on();
		ALS_ON = 1;
		break;

	case TAOS_IOCTL_ALS_OFF:
		pr_taos(DEBUG, "ioctl:TAOS_IOCTL_ALS_OFF\n");
		if (PROX_ON == 1) {
			ALS_ON = 0;
			return ret;
		}

		for (i = 0; i < TAOS_FILTER_DEPTH; i++)
			lux_history[i] = -ENODATA;
		ret = i2c_smbus_write_byte(taos_datap->client,
					   TAOS_TRITON_CMD_REG |
					   TAOS_TRITON_CNTRL);
		if (ret < 0) {
			pr_taos(ERROR,
				"TAOS: i2c_smbus_write_byte failed in ioctl als_calibrate\n");
			return ret;
		}
		reg_val = i2c_smbus_read_byte(taos_datap->client);
		if ((reg_val & TAOS_TRITON_CNTL_PROX_DET_ENBL) == 0x0) {
			ret = i2c_smbus_write_byte_data(taos_datap->client,
							TAOS_TRITON_CMD_REG |
							TAOS_TRITON_CNTRL,
							0x00);
			if (ret < 0) {
				pr_taos(ERROR,
					"TAOS: i2c_smbus_write_byte_data failed in ioctl als_off\n");
				return ret;
			}
			cancel_work_sync(&taos_datap->work);
		}
		ALS_ON = 0;
		break;

	case TAOS_IOCTL_ALS_DATA:
		ret = i2c_smbus_write_byte(taos_datap->client,
					   TAOS_TRITON_CMD_REG |
					   TAOS_TRITON_CNTRL);
		if (ret < 0) {
			pr_taos(ERROR,
				"TAOS: i2c_smbus_write_byte failed in ioctl als_data\n");
			return ret;
		}
		reg_val = i2c_smbus_read_byte(taos_datap->client);
		if ((reg_val &
		     (TAOS_TRITON_CNTL_ADC_ENBL | TAOS_TRITON_CNTL_PWRON)) !=
		    (TAOS_TRITON_CNTL_ADC_ENBL | TAOS_TRITON_CNTL_PWRON))
			return -ENODATA;
		ret = i2c_smbus_write_byte(taos_datap->client,
					   TAOS_TRITON_CMD_REG |
					   TAOS_TRITON_STATUS);
		if (ret < 0) {
			pr_taos(ERROR,
				"TAOS: i2c_smbus_write_byte failed in ioctl als_data\n");
			return ret;
		}
		reg_val = i2c_smbus_read_byte(taos_datap->client);
		if ((reg_val & TAOS_TRITON_STATUS_ADCVALID) !=
		    TAOS_TRITON_STATUS_ADCVALID)
			return -ENODATA;
		lux_val = taos_get_lux();
		if (lux_val < 0)
			pr_taos(ERROR,
				"TAOS: call to taos_get_lux() returned error %d in ioctl als_data\n",
				lux_val);
		lux_val = taos_lux_filter(lux_val);
		break;

	case TAOS_IOCTL_ALS_CALIBRATE:
		itime = (((taos_cfgp->als_time / 50) * 18) - 1);
		itime = (~itime);
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG |
						TAOS_TRITON_ALS_TIME, itime);
		if (ret < 0)
			return ret;
		ret = i2c_smbus_write_byte(taos_datap->client,
					   TAOS_TRITON_CMD_REG |
					   TAOS_TRITON_CNTRL);
		if (ret < 0)
			return ret;
		reg_val = i2c_smbus_read_byte(taos_datap->client);
		reg_cntrl = reg_val | (TAOS_TRITON_CNTL_ADC_ENBL |
				       TAOS_TRITON_CNTL_PWRON);
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG |
						TAOS_TRITON_CNTRL, reg_cntrl);
		if (ret < 0)
			return ret;
		ret = i2c_smbus_write_byte(taos_datap->client,
					   TAOS_TRITON_CMD_REG |
					   TAOS_TRITON_GAIN);
		if (ret < 0)
			return ret;
		reg_val = i2c_smbus_read_byte(taos_datap->client);
		reg_val = reg_val & 0xFC;
		reg_val = reg_val | (taos_cfgp->gain & 0x03);
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG |
						TAOS_TRITON_GAIN, reg_val);
		if (ret < 0)
			return ret;
		mdelay(500);
		ret = i2c_smbus_write_byte(taos_datap->client,
					   TAOS_TRITON_CMD_REG |
					   TAOS_TRITON_STATUS);
		if (ret < 0)
			return ret;
		reg_val = i2c_smbus_read_byte(taos_datap->client);
		if ((reg_val & 0x01) != 0x01)
			return -ENODATA;
		taos_als_get_data();
		break;

	case TAOS_IOCTL_CONFIG_GET:
		pr_taos(DEBUG, "ioctl: TAOS_IOCTL_CONFIG_GET\n");
#ifdef TMD2771_USER_CALIBRATION
		if (taos_cfgp->prox_boot_cali)
			tmd2771_prox_boot_cali();
#endif
		ret = copy_to_user((struct taos_cfg *)arg, taos_cfgp,
				   sizeof(struct taos_cfg));
		if (ret)
			return -ENODATA;
		break;

	case TAOS_IOCTL_CONFIG_SET:
		ret = copy_from_user(taos_cfgp, (struct taos_cfg *)arg,
				     sizeof(struct taos_cfg));
		if (ret)
			return -ENODATA;
		if (taos_cfgp->als_time < 50)
			taos_cfgp->als_time = 50;
		if (taos_cfgp->als_time > 650)
			taos_cfgp->als_time = 650;
		tmp = (taos_cfgp->als_time + 25) / 50;
		taos_cfgp->als_time = tmp * 50;
		sat_als = (256 - taos_cfgp->prox_int_time) << 10;
		sat_prox = (256 - taos_cfgp->prox_adc_time) << 10;
		break;

	case TAOS_IOCTL_PROX_ON:
		pr_taos(DEBUG, "ioctl: TAOS_IOCTL_PROX_ON\n");
		PROX_ON = 1;
		if (isPsensorLocked == 0) {
			wake_lock(&taos_datap->taos_wake_lock);
			isPsensorLocked = 1;
		}
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG | 0x01,
						taos_cfgp->prox_int_time);
		if (ret < 0)
			return ret;
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG | 0x02,
						taos_cfgp->prox_adc_time);
		if (ret < 0)
			return ret;
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG | 0x03,
						taos_cfgp->prox_wait_time);
		if (ret < 0)
			return ret;
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG | 0x0C,
						taos_cfgp->prox_intr_filter);
		if (ret < 0)
			return ret;
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG | 0x0D,
						taos_cfgp->prox_config);
		if (ret < 0)
			return ret;
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG | 0x0E,
						taos_cfgp->prox_pulse_cnt);
		if (ret < 0)
			return ret;
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG | 0x0F,
						taos_cfgp->prox_gain);
		if (ret < 0)
			return ret;

		/* enable the irq of als when prox on */
		reg_cntrl = TAOS_TRITON_CNTL_PROX_DET_ENBL |
		    TAOS_TRITON_CNTL_PWRON |
		    TAOS_TRITON_CNTL_PROX_INT_ENBL |
		    TAOS_TRITON_CNTL_ADC_ENBL |
		    TAOS_TRITON_CNTL_WAIT_TMR_ENBL |
		    TAOS_TRITON_CNTL_ALS_INT_ENBL;

		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG |
						TAOS_TRITON_CNTRL, reg_cntrl);
		if (ret < 0)
			return ret;
		taos_prox_threshold_set();
		break;

	case TAOS_IOCTL_PROX_OFF:
		pr_taos(DEBUG, "ioctl: TAOS_IOCTL_PROX_OFF\n");
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG |
						TAOS_TRITON_CNTRL, 0x00);
		if (ret < 0)
			return ret;
		if (ALS_ON == 1)
			taos_sensors_als_on();
		else
			cancel_work_sync(&taos_datap->work);
		PROX_ON = 0;
		if (isPsensorLocked == 1) {
			wake_unlock(&taos_datap->taos_wake_lock);
			isPsensorLocked = 0;
		}
		break;

	case TAOS_IOCTL_PROX_DATA:
		ret = taos_prox_poll(prox_cur_infop);
		if (ret < 0)
			return ret;
		ret = copy_to_user((struct taos_prox_info *)arg, prox_cur_infop,
				   sizeof(struct taos_prox_info));
		if (ret)
			return -ENODATA;
		break;

	case TAOS_IOCTL_PROX_EVENT:
		ret = taos_prox_poll(prox_cur_infop);
		if (ret < 0)
			return ret;
		return prox_cur_infop->prox_event;

	case TAOS_IOCTL_PROX_CALIBRATE:
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG | 0x01,
						taos_cfgp->prox_int_time);
		if (ret < 0)
			return ret;
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG | 0x02,
						taos_cfgp->prox_adc_time);
		if (ret < 0)
			return ret;
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG | 0x03,
						taos_cfgp->prox_wait_time);
		if (ret < 0)
			return ret;
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG | 0x0D,
						taos_cfgp->prox_config);
		if (ret < 0)
			return ret;
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG | 0x0E,
						taos_cfgp->prox_pulse_cnt);
		if (ret < 0)
			return ret;
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG | 0x0F,
						taos_cfgp->prox_gain);
		if (ret < 0)
			return ret;
		reg_cntrl = reg_val | (TAOS_TRITON_CNTL_PROX_DET_ENBL |
				       TAOS_TRITON_CNTL_PWRON |
				       TAOS_TRITON_CNTL_ADC_ENBL);
		ret = i2c_smbus_write_byte_data(taos_datap->client,
						TAOS_TRITON_CMD_REG |
						TAOS_TRITON_CNTRL, reg_cntrl);
		if (ret < 0)
			return ret;
		prox_sum = 0;
		prox_max = 0;
		for (i = 0; i < 32; i++) {
			ret = taos_prox_poll(&prox_cal_info[i]);
			if (ret < 0)
				return ret;
			prox_sum += prox_cal_info[i].prox_data;
			if (prox_cal_info[i].prox_data > prox_max)
				prox_max = prox_cal_info[i].prox_data;
			mdelay(100);
		}
		prox_mean = prox_sum >> 5;
		taos_cfgp->prox_threshold_hi =
		    ((((prox_max - prox_mean) * 200) + 50) / 100) + prox_mean;
		taos_cfgp->prox_threshold_lo =
		    ((((prox_max - prox_mean) * 170) + 50) / 100) + prox_mean;
		pr_taos(DEBUG, "TAOS:----- taos_cfgp->prox_threshold_hi = %d\n",
			taos_cfgp->prox_threshold_hi);
		pr_taos(DEBUG, "TAOS------ taos_cfgp->prox_threshold_lo = %d\n",
			taos_cfgp->prox_threshold_lo);
		for (i = 0; i < sizeof(taos_triton_reg_init); i++) {
			if (i != 11) {
				ret =
				    i2c_smbus_write_byte_data(
						taos_datap->client,
						TAOS_TRITON_CMD_REG
						|(TAOS_TRITON_CNTRL + i),
						taos_triton_reg_init[i]);
				if (ret < 0)
					return ret;
			}
		}
		break;

	default:
		return -EINVAL;
		break;
	}
	return ret;
}

/* read/calculate lux value */
static int taos_get_lux(void)
{
	u32 raw_clear = 0, raw_ir = 0, raw_lux = 0;
	u32 lux = 0;
	u32 ratio = 0;
	u8 dev_gain = 0;
	struct lux_data *p;
	int ret = 0;
	u8 chdata[4];
	u16 Tint;
	int tmp = 0, i = 0;

	for (i = 0; i < 4; i++) {
		ret = i2c_smbus_write_byte(taos_datap->client,
					   TAOS_TRITON_CMD_REG |
					   (TAOS_TRITON_ALS_CHAN0LO + i));
		if (ret < 0)
			return ret;
		chdata[i] = i2c_smbus_read_byte(taos_datap->client);
		pr_taos(DEBUG, "ch(%d),data=%d\n", i, chdata[i]);
	}
	pr_taos(DEBUG, "ch0=%d\n", chdata[0] + chdata[1] * 256);
	pr_taos(DEBUG, "ch1=%d\n", chdata[2] + chdata[3] * 256);

	tmp = (taos_cfgp->als_time + 25) / 50;
	TritonTime.numerator = 1;
	TritonTime.denominator = tmp;

	tmp = 300 * taos_cfgp->als_time;
	if (tmp > 65535)
		tmp = 65535;
	TritonTime.saturation = tmp;
	raw_clear = chdata[1];
	raw_clear <<= 8;
	raw_clear |= chdata[0];
	raw_ir = chdata[3];
	raw_ir <<= 8;
	raw_ir |= chdata[2];

	if (raw_ir > raw_clear) {
		raw_lux = raw_ir;
		raw_ir = raw_clear;
		raw_clear = raw_lux;
	}

	dev_gain = taos_triton_gain_table[taos_cfgp->gain & 0x3];
	if (raw_clear >= lux_timep->saturation)
		return TAOS_MAX_LUX;
	if (raw_ir >= lux_timep->saturation)
		return TAOS_MAX_LUX;
	if (raw_clear == 0)
		return 0;
	if (dev_gain == 0 || dev_gain > 127) {
		pr_taos(ERROR, "dev_gain = 0 or > 127 in taos_get_lux()\n");
		return -1;
	}
	if (lux_timep->denominator == 0) {
		pr_taos(ERROR, "error: lux_timep->denominator = 0\n");
		return -1;
	}

	raw_clear *= taos_cfgp->scale_factor * 500;
	raw_ir *= taos_cfgp->scale_factor * 100;

	ratio = (raw_ir << 15) / raw_clear;

	for (p = lux_tablep; p->ratio && p->ratio < ratio; p++)
		;
	if (!p->ratio)
		return 0;

	Tint = taos_cfgp->als_time;
	raw_clear =
	    ((raw_clear * 400 + (dev_gain >> 1)) / dev_gain +
	     (Tint >> 1)) / Tint;
	raw_ir =
	    ((raw_ir * 400 + (dev_gain >> 1)) / dev_gain + (Tint >> 1)) / Tint;
	lux = ((raw_clear * (p->clear)) - (raw_ir * (p->ir)));
	lux = 4 * (lux + 32000) / 64000;
	if (lux > TAOS_MAX_LUX)
		lux = TAOS_MAX_LUX;

	return lux;

	/*
	   lux = ((raw_clear * (p->clear)) - (raw_ir * (p->ir)));
	   lux =
	   ((lux +
	   (lux_timep->denominator >> 1)) / lux_timep->denominator) *
	   lux_timep->numerator;
	   lux = (lux + (dev_gain >> 1)) / dev_gain;
	   lux >>= TAOS_SCALE_MILLILUX;
	   //printk(KERN_INFO "------taos_get_lux--lux==%d\n", lux);
	   if (lux > TAOS_MAX_LUX)
	   lux = TAOS_MAX_LUX;
	   return (lux) * taos_cfgp->filter_count;
	 */

}

static int taos_lux_filter(int lux)
{
	static u8 middle[] = { 1, 0, 2, 0, 0, 2, 0, 1 };
	int index;

	lux_history[2] = lux_history[1];
	lux_history[1] = lux_history[0];
	lux_history[0] = lux;

	if (lux_history[2] < 0) {
		if (lux_history[1] > 0)
			return lux_history[1];
		else
			return lux_history[0];
	}
	index = 0;
	if (lux_history[0] > lux_history[1])
		index += 4;
	if (lux_history[1] > lux_history[2])
		index += 2;
	if (lux_history[0] > lux_history[2])
		index++;
	return lux_history[middle[index]];
}

/* verify device */
static int taos_device_name(unsigned char *bufp, char **device_name)
{
	if ((bufp[0x12] & 0xff) != 0x29)
		return 0;
	if (bufp[0x10] | bufp[0x1a] | bufp[0x1b] | bufp[0x1c] | bufp[0x1d] |
	    bufp[0x1e])
		return 0;
	if (bufp[0x13] & 0x0c)
		return 0;
	*device_name = "tmd2771";
	return 1;
}

static int taos_prox_poll(struct taos_prox_info *prxp)
{
	int i = 0, ret = 0;
	u8 chdata[6];
	for (i = 0; i < 6; i++) {
		chdata[i] =
		    (i2c_smbus_read_byte_data(taos_datap->client,
					      (TAOS_TRITON_CMD_REG |
					       TAOS_TRITON_CMD_AUTO |
					       (TAOS_TRITON_ALS_CHAN0LO + i))));
	}
	prxp->prox_clear = chdata[1];
	prxp->prox_clear <<= 8;
	prxp->prox_clear |= chdata[0];
	if (prxp->prox_clear > ((sat_als * 80) / 100))
		return -ENODATA;
	prxp->prox_data = chdata[5];
	prxp->prox_data <<= 8;
	prxp->prox_data |= chdata[4];
	return ret;
}

static void taos_prox_poll_timer_func(unsigned long param)
{
	int ret = 0;
	if (!device_released) {
		ret = taos_prox_poll(prox_cur_infop);
		if (ret < 0) {
			pr_taos(ERROR,
				"TAOS: call to prox_poll failed in taos_prox_poll_timer_func()\n");
			return;
		}
		taos_prox_poll_timer_start();
	}
	return;
}

static void taos_prox_poll_timer_start(void)
{
	init_timer(&prox_poll_timer);
	prox_poll_timer.expires = jiffies + (HZ / 10);
	prox_poll_timer.function = taos_prox_poll_timer_func;
	add_timer(&prox_poll_timer);
	return;
}

MODULE_AUTHOR("John Koshi - Surya Software");
MODULE_DESCRIPTION("TAOS ambient light and proximity sensor driver");
MODULE_LICENSE("GPL");

module_init(taos_init);
module_exit(taos_exit);
