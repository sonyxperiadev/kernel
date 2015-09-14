/* drivers/input/misc/cm36686.c - cm36686 optical sensors driver
 *
 * Copyright (C) 2013 Capella Microsystems Inc.
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

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/input/cm36686.h>
#include <asm/setup.h>
#include <linux/wakelock.h>
#include <linux/jiffies.h>

#include <linux/pinctrl/consumer.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>

#define D(x...) pr_info(x)

#define LIGHTSENSOR_IOCTL_MAGIC 'l'
#define LIGHTSENSOR_IOCTL_GET_ENABLED _IOR(LIGHTSENSOR_IOCTL_MAGIC, 1, int *)
#define LIGHTSENSOR_IOCTL_ENABLE _IOW(LIGHTSENSOR_IOCTL_MAGIC, 2, int *)

#define CAPELLA_CM3602_IOCTL_MAGIC 'c'
#define CAPELLA_CM3602_IOCTL_GET_ENABLED \
		_IOR(CAPELLA_CM3602_IOCTL_MAGIC, 1, int *)
#define CAPELLA_CM3602_IOCTL_ENABLE \
		_IOW(CAPELLA_CM3602_IOCTL_MAGIC, 2, int *)

#define LS_PWR_ON			(1 << 0)

#define I2C_RETRY_COUNT 10

#define NEAR_DELAY_TIME 		((100 * HZ) / 1000)

#define CONTROL_INT_ISR_REPORT		0x00
#define CONTROL_ALS			0x01
#define CONTROL_PS			0x02

static int record_init_fail = 0;
static void sensor_irq_do_work(struct work_struct *work);
static DECLARE_WORK(sensor_irq_work, sensor_irq_do_work);

struct cm36686_info {
	struct class *cm36686_class;
	struct device *ls_dev;
	struct device *ps_dev;

	struct input_dev *ls_input_dev;
	struct input_dev *ps_input_dev;

	struct i2c_client *i2c_client;
	struct workqueue_struct *lp_wq;

	int intr_pin;
	int als_enable;
	int ps_enable;
	int ps_irq_flag;

	uint16_t adc_table[ 10 ];
	uint16_t cali_table[10];
	int irq;

	int ls_calibrate;
	
	int (*power)(int, uint8_t); /* power to the chip */

	uint32_t als_kadc;
	uint32_t als_gadc;
	uint16_t golden_adc;

	struct wake_lock ps_wake_lock;
	int psensor_opened;
	int lightsensor_opened;
	uint8_t slave_addr;

	uint16_t ps_close_thd_set;
	uint16_t ps_away_thd_set;	
	int current_level;
	uint16_t current_adc;
	unsigned current_lux;
	unsigned IR_Ink;
    uint8_t inte_cancel_set;
	uint16_t ps_conf1_val;
	uint16_t ps_conf3_val;

	uint16_t ls_thd_high_offset;
	uint16_t ls_thd_low_offset;

	uint16_t ls_cmd;
	uint8_t record_clear_int_fail;

	unsigned vdd_power[ 3 ], i2c_power[ 3 ];
};
struct cm36686_info *lp_info;
int fLevel=-1;
static uint8_t ps_cancel_set;
static struct mutex als_enable_mutex, als_disable_mutex, als_get_adc_mutex;
static struct mutex ps_enable_mutex, ps_disable_mutex, ps_get_adc_mutex;
static struct mutex CM36686_control_mutex;
static int lightsensor_enable(struct cm36686_info *lpi);
static int lightsensor_disable(struct cm36686_info *lpi);
static int initial_cm36686(struct cm36686_info *lpi);
static void psensor_initial_cmd(struct cm36686_info *lpi);

int32_t als_kadc;

static int control_and_report(struct cm36686_info *lpi, uint8_t mode, uint16_t param);

static int I2C_RxData(uint16_t slaveAddr, uint8_t cmd, uint8_t *rxData, int length)
{
	uint8_t loop_i;
	int val;
	struct cm36686_info *lpi = lp_info;
		
	struct i2c_msg msgs[] = {
		{
		 .addr = slaveAddr,
		 .flags = 0,
		 .len = 1,
		 .buf = &cmd,
		 },
		{
		 .addr = slaveAddr,
		 .flags = I2C_M_RD,
		 .len = length,
		 .buf = rxData,
		 },		 
	};

	for (loop_i = 0; loop_i < I2C_RETRY_COUNT; loop_i++) {

		if (i2c_transfer(lp_info->i2c_client->adapter, msgs, 2) > 0)
			break;

		val = gpio_get_value(lpi->intr_pin);
		/*check intr GPIO when i2c error*/
		if (loop_i == 0 || loop_i == I2C_RETRY_COUNT -1)
			D("[PS][CM36686 error] %s, i2c err, slaveAddr 0x%x ISR gpio %d  = %d, record_init_fail %d \n",
				__func__, slaveAddr, lpi->intr_pin, val, record_init_fail);

		msleep(10);
	}
	if (loop_i >= I2C_RETRY_COUNT) {
		printk(KERN_ERR "[PS_ERR][CM36686 error] %s retry over %d\n",
			__func__, I2C_RETRY_COUNT);
		return -EIO;
	}

	return 0;
}

static int I2C_TxData(uint16_t slaveAddr, uint8_t *txData, int length)
{
	uint8_t loop_i;
	int val;
	struct cm36686_info *lpi = lp_info;
	struct i2c_msg msg[] = {
		{
		 .addr = slaveAddr,
		 .flags = 0,
		 .len = length,
		 .buf = txData,
		 },
	};

	for (loop_i = 0; loop_i < I2C_RETRY_COUNT; loop_i++) {
		if (i2c_transfer(lp_info->i2c_client->adapter, msg, 1) > 0)
			break;

		val = gpio_get_value(lpi->intr_pin);
		/*check intr GPIO when i2c error*/
		if (loop_i == 0 || loop_i == I2C_RETRY_COUNT -1)
			D("[PS][CM36686 error] %s, i2c err, slaveAddr 0x%x, value 0x%x, ISR gpio%d  = %d, record_init_fail %d\n",
				__func__, slaveAddr, txData[0], lpi->intr_pin, val, record_init_fail);

		msleep(10);
	}

	if (loop_i >= I2C_RETRY_COUNT) {
		printk(KERN_ERR "[ALS+PS_ERR][CM36686 error] %s retry over %d\n",
			__func__, I2C_RETRY_COUNT);
		return -EIO;
	}

	return 0;
}

static int _cm36686_I2C_Read_Word(uint16_t slaveAddr, uint8_t cmd, uint16_t *pdata)
{
	uint8_t buffer[2];
	int ret = 0;

	if (pdata == NULL)
		return -EFAULT;

	ret = I2C_RxData(slaveAddr, cmd, buffer, 2);

	if (ret < 0) {
		pr_err(
			"[ALS+PS_ERR][CM36686 error]%s: I2C_RxData fail [0x%x, 0x%x]\n",
			__func__, slaveAddr, cmd);
		return ret;
	}

	*pdata = (buffer[1]<<8)|buffer[0];

	return ret;
}

static int _cm36686_I2C_Write_Word(uint16_t SlaveAddress, uint8_t cmd, uint16_t data)
{
	char buffer[3];
	int ret = 0;

	buffer[0] = cmd;
	buffer[1] = (uint8_t)(data&0xff);
	buffer[2] = (uint8_t)((data&0xff00)>>8);	
	
	ret = I2C_TxData(SlaveAddress, buffer, 3);
	if (ret < 0) {
		pr_err("[ALS+PS_ERR][CM36686 error]%s: I2C_TxData fail\n", __func__);
		return -EIO;
	}

	return ret;
}

static int get_ls_adc_value(uint16_t *als_step, bool resume)
{
	struct cm36686_info *lpi = lp_info;
	uint32_t tmpResult;
	int ret = 0;

	if (als_step == NULL)
		return -EFAULT;

	/* Read ALS data: */
	ret = _cm36686_I2C_Read_Word(lpi->slave_addr, ALS_DATA, als_step);
	if (ret < 0) {
		pr_err(
			"[LS][CM36686 error]%s: _cm36686_I2C_Read_Word fail\n",
			__func__);
		return -EIO;
	}
if( 0 )
  if (!lpi->ls_calibrate ) {
		tmpResult = (uint32_t)(*als_step) * lpi->als_gadc / lpi->als_kadc;
		if (tmpResult > 0xFFFF)
			*als_step = 0xFFFF;
		else
		  *als_step = tmpResult;  			
	}

	D("[LS][CM36686] %s: raw adc = 0x%X, ls_calibrate = %d\n",
		__func__, *als_step, lpi->ls_calibrate);

	return ret;
}

static int set_lsensor_range(uint16_t low_thd, uint16_t high_thd)
{
	int ret = 0;
	struct cm36686_info *lpi = lp_info;

	_cm36686_I2C_Write_Word(lpi->slave_addr, ALS_THDH, high_thd);
	_cm36686_I2C_Write_Word(lpi->slave_addr, ALS_THDL, low_thd);

	return ret;
}

static int get_ps_adc_value(uint16_t *data)
{
	int ret = 0;
	struct cm36686_info *lpi = lp_info;

	if (data == NULL)
		return -EFAULT;	

	ret = _cm36686_I2C_Read_Word(lpi->slave_addr, PS_DATA, data);
	
		
	if (ret < 0) {
		pr_err(
			"[PS][CM36686 error]%s: _cm36686_I2C_Read_Word fail\n",
			__func__);
		return -EIO;
	} else {
		pr_err(
			"[PS][CM36686 OK]%s: _cm36686_I2C_Read_Word OK 0x%04x\n",
			__func__, *data);
	}

	return ret;
}

static uint16_t mid_value(uint16_t value[], uint8_t size)
{
	int i = 0, j = 0;
	uint16_t temp = 0;

	if (size < 3)
		return 0;

	for (i = 0; i < (size - 1); i++)
		for (j = (i + 1); j < size; j++)
			if (value[i] > value[j]) {
				temp = value[i];
				value[i] = value[j];
				value[j] = temp;
			}
	return value[((size - 1) / 2)];
}

static int get_stable_ps_adc_value(uint16_t *ps_adc)
{
	uint16_t value[3] = {0, 0, 0}, mid_val = 0;
	int ret = 0;
	int i = 0;
	int wait_count = 0;
	struct cm36686_info *lpi = lp_info;

//	for (i = 0; i < 3; i++) {
		/*wait interrupt GPIO high*/
		while (gpio_get_value(lpi->intr_pin) == 0) {
			msleep(10);
			wait_count++;
			if (wait_count > 1) {
				pr_err("[PS_ERR][CM36686 error]%s: interrupt GPIO low,"
					" get_ps_adc_value\n", __func__);
				return -EIO;
			}
		}

		ret = get_ps_adc_value(&value[i]);
		if (ret < 0) {
			pr_err("[PS_ERR][CM36686 error]%s: get_ps_adc_value\n",
				__func__);
			return -EIO;
		}

//		if (wait_count < 60/10) {/*wait gpio less than 60ms*/
//			msleep(60 - (10*wait_count));
//		}
//		wait_count = 0;
//	}

if( 0 )
{
	/*D("Sta_ps: Before sort, value[0, 1, 2] = [0x%x, 0x%x, 0x%x]",
		value[0], value[1], value[2]);*/
	mid_val = mid_value(value, 3);
	D("Sta_ps: After sort, value[0, 1, 2] = [0x%x, 0x%x, 0x%x]\n",
		value[0], value[1], value[2]);
	//*ps_adc = (mid_val & 0xFF);

	*ps_adc	= mid_val;
 }

	*ps_adc	= value[0];

	return 0;
}

static void sensor_irq_do_work(struct work_struct *work)
{
	struct cm36686_info *lpi = lp_info;
	uint16_t intFlag;
  _cm36686_I2C_Read_Word(lpi->slave_addr, INT_FLAG, &intFlag);

  printk( "D[PS] Int, Flag(0x%x)\n", intFlag );

	control_and_report(lpi, CONTROL_INT_ISR_REPORT, intFlag);  
	  
	enable_irq(lpi->irq);
}

static irqreturn_t cm36686_irq_handler(int irq, void *data)
{
	struct cm36686_info *lpi = data;

	disable_irq_nosync(lpi->irq);
	queue_work(lpi->lp_wq, &sensor_irq_work);

	return IRQ_HANDLED;
}

static int als_power(int enable)
{
	struct cm36686_info *lpi = lp_info;

	if (lpi->power)
		lpi->power(LS_PWR_ON, 1);

	return 0;
}

static void ls_initial_cmd(struct cm36686_info *lpi)
{	
	/*must disable l-sensor interrupt befrore IST create*//*disable ALS func*/
	lpi->ls_cmd &= CM36686_ALS_INT_MASK;
  lpi->ls_cmd |= CM36686_ALS_SD;
  _cm36686_I2C_Write_Word(lpi->slave_addr, ALS_CONF, lpi->ls_cmd);  
}

static void psensor_initial_cmd(struct cm36686_info *lpi)
{
	/*must disable p-sensor interrupt befrore IST create*//*disable ALS func*/		
  lpi->ps_conf1_val |= CM36686_PS_SD;
  lpi->ps_conf1_val &= CM36686_PS_INT_MASK;  
  _cm36686_I2C_Write_Word(lpi->slave_addr, PS_CONF1, lpi->ps_conf1_val);   
  _cm36686_I2C_Write_Word(lpi->slave_addr, PS_CONF3, lpi->ps_conf3_val);
  //_cm36686_I2C_Write_Word(lpi->slave_addr, PS_THD, (lpi->ps_close_thd_set <<8)| lpi->ps_away_thd_set);
  _cm36686_I2C_Write_Word(lpi->slave_addr, PS_THDL,  lpi->ps_away_thd_set);
  _cm36686_I2C_Write_Word(lpi->slave_addr, PS_THDH,  lpi->ps_close_thd_set);
	D("[PS][CM36686] %s, finish\n", __func__);	
}

static int psensor_enable(struct cm36686_info *lpi)
{
	int ret = -EIO;
	
	mutex_lock(&ps_enable_mutex);
	D("[PS][CM36686] %s\n", __func__);

	if ( lpi->ps_enable ) {
		D("[PS][CM36686] %s: already enabled\n", __func__);
		ret = 0;
	} else
  	ret = control_and_report(lpi, CONTROL_PS, 1);
	
	mutex_unlock(&ps_enable_mutex);
	return ret;
}

static int psensor_disable(struct cm36686_info *lpi)
{
	int ret = -EIO;
	
	mutex_lock(&ps_disable_mutex);
	D("[PS][CM36686] %s\n", __func__);

	{

		uint16_t	u16ADC;

		int		iPinState;

		iPinState = gpio_get_value( lpi->intr_pin ), get_ps_adc_value( &u16ADC );

		printk( "[PS][ADC:Pin]=[%d:%d]\n", u16ADC, iPinState );

	}

	if ( lpi->ps_enable == 0 ) {
		D("[PS][CM36686] %s: already disabled\n", __func__);
		ret = 0;
	} else
  	ret = control_and_report(lpi, CONTROL_PS,0);
	
	mutex_unlock(&ps_disable_mutex);
	return ret;
}

static int psensor_open(struct inode *inode, struct file *file)
{
	struct cm36686_info *lpi = lp_info;

	D("[PS][CM36686] %s\n", __func__);

	if (lpi->psensor_opened)
		return -EBUSY;

	lpi->psensor_opened = 1;

	return 0;
}

static int psensor_release(struct inode *inode, struct file *file)
{
	struct cm36686_info *lpi = lp_info;

	D("[PS][CM36686] %s\n", __func__);

	lpi->psensor_opened = 0;

	return psensor_disable(lpi);
	//return 0;
}

static long psensor_ioctl(struct file *file, unsigned int cmd,
			unsigned long arg)
{
	int val;
	struct cm36686_info *lpi = lp_info;

	D("[PS][CM36686] %s cmd %d\n", __func__, _IOC_NR(cmd));

	switch (cmd) {
	case CAPELLA_CM3602_IOCTL_ENABLE:
		if (get_user(val, (unsigned long __user *)arg))
			return -EFAULT;
		if (val)
			return psensor_enable(lpi);
		else
			return psensor_disable(lpi);
		break;
	case CAPELLA_CM3602_IOCTL_GET_ENABLED:
		return put_user(lpi->ps_enable, (unsigned long __user *)arg);
		break;
	default:
		pr_err("[PS][CM36686 error]%s: invalid cmd %d\n",
			__func__, _IOC_NR(cmd));
		return -EINVAL;
	}
}

static const struct file_operations psensor_fops = {
	.owner = THIS_MODULE,
	.open = psensor_open,
	.release = psensor_release,
	.unlocked_ioctl = psensor_ioctl
};

struct miscdevice psensor_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "proximity",
	.fops = &psensor_fops
};

void lightsensor_set_kvalue(struct cm36686_info *lpi)
{
	if (!lpi) {
		pr_err("[LS][CM36686 error]%s: ls_info is empty\n", __func__);
		return;
	}

	D("[LS][CM36686] %s: ALS calibrated als_kadc=0x%x\n",
			__func__, als_kadc);

	if (als_kadc >> 16 == ALS_CALIBRATED)
		lpi->als_kadc = als_kadc & 0xFFFF;
	else {
		lpi->als_kadc = 0;
		D("[LS][CM36686] %s: no ALS calibrated\n", __func__);
	}

	if (lpi->als_kadc && lpi->golden_adc > 0) {
		lpi->als_kadc = (lpi->als_kadc > 0 && lpi->als_kadc < 0x1000) ?
				lpi->als_kadc : lpi->golden_adc;
		lpi->als_gadc = lpi->golden_adc;
	} else {
		lpi->als_kadc = 1;
		lpi->als_gadc = 1;
	}
	D("[LS][CM36686] %s: als_kadc=0x%x, als_gadc=0x%x\n",
		__func__, lpi->als_kadc, lpi->als_gadc);
}


static int lightsensor_update_table(struct cm36686_info *lpi)
{
	uint32_t tmpData[10];
	int i;
	for (i = 0; i < 10; i++) {
		tmpData[i] = (uint32_t)(*(lpi->adc_table + i))
				* lpi->als_kadc / lpi->als_gadc ;
		if( tmpData[i] <= 0xFFFF ){
      lpi->cali_table[i] = (uint16_t) tmpData[i];		
    } else {
      lpi->cali_table[i] = 0xFFFF;    
    }         
		D("[LS][CM36686] %s: Calibrated adc_table: data[%d], %x\n",
			__func__, i, lpi->cali_table[i]);
	}

	return 0;
}


static int lightsensor_enable(struct cm36686_info *lpi)
{
	int ret = -EIO;
	
	mutex_lock(&als_enable_mutex);
	D("[LS][CM36686] %s\n", __func__);

	if (lpi->als_enable) {
		D("[LS][CM36686] %s: already enabled\n", __func__);
		ret = 0;
	} else
  	ret = control_and_report(lpi, CONTROL_ALS, 1);
	
	mutex_unlock(&als_enable_mutex);
	return ret;
}

static int lightsensor_disable(struct cm36686_info *lpi)
{
	int ret = -EIO;
	mutex_lock(&als_disable_mutex);
	D("[LS][CM36686] %s\n", __func__);

	if ( lpi->als_enable == 0 ) {
		D("[LS][CM36686] %s: already disabled\n", __func__);
		ret = 0;
	} else
    ret = control_and_report(lpi, CONTROL_ALS, 0);
	
	mutex_unlock(&als_disable_mutex);
	return ret;
}

static int lightsensor_open(struct inode *inode, struct file *file)
{
	struct cm36686_info *lpi = lp_info;
	int rc = 0;

	D("[LS][CM36686] %s\n", __func__);
	if (lpi->lightsensor_opened) {
		pr_err("[LS][CM36686 error]%s: already opened\n", __func__);
		rc = -EBUSY;
	}
	lpi->lightsensor_opened = 1;
	return rc;
}

static int lightsensor_release(struct inode *inode, struct file *file)
{
	struct cm36686_info *lpi = lp_info;

	D("[LS][CM36686] %s\n", __func__);
	lpi->lightsensor_opened = 0;
	return 0;
}

static long lightsensor_ioctl(struct file *file, unsigned int cmd,
		unsigned long arg)
{
	int rc, val;
	struct cm36686_info *lpi = lp_info;

	/*D("[CM36686] %s cmd %d\n", __func__, _IOC_NR(cmd));*/

	switch (cmd) {
	case LIGHTSENSOR_IOCTL_ENABLE:
		if (get_user(val, (unsigned long __user *)arg)) {
			rc = -EFAULT;
			break;
		}
		D("[LS][CM36686] %s LIGHTSENSOR_IOCTL_ENABLE, value = %d\n",
			__func__, val);
		rc = val ? lightsensor_enable(lpi) : lightsensor_disable(lpi);
		break;
	case LIGHTSENSOR_IOCTL_GET_ENABLED:
		val = lpi->als_enable;
		D("[LS][CM36686] %s LIGHTSENSOR_IOCTL_GET_ENABLED, enabled %d\n",
			__func__, val);
		rc = put_user(val, (unsigned long __user *)arg);
		break;
	default:
		pr_err("[LS][CM36686 error]%s: invalid cmd %d\n",
			__func__, _IOC_NR(cmd));
		rc = -EINVAL;
	}

	return rc;
}

static const struct file_operations lightsensor_fops = {
	.owner = THIS_MODULE,
	.open = lightsensor_open,
	.release = lightsensor_release,
	.unlocked_ioctl = lightsensor_ioctl
};

static struct miscdevice lightsensor_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "lightsensor",
	.fops = &lightsensor_fops
};

bool proximity_sensor_state = false;

bool enable_skip_function = false;

bool	proximity_status( void )
{

	return	enable_skip_function && proximity_sensor_state ? false : true;

}
EXPORT_SYMBOL( proximity_sensor_state );

static ssize_t ps_skip_event_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{

	return	sprintf( buf, "[AP:Driver]=[%d:%d], %s touch event\n", (int)enable_skip_function, (int)proximity_sensor_state, enable_skip_function && proximity_sensor_state ? "Skip" : "Bypass" );

}

static ssize_t ps_skip_event_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{

	unsigned	skip_event;

	sscanf( buf, "%d", &skip_event );

	enable_skip_function	= skip_event ? true : false;

	return	count;

}

static DEVICE_ATTR(ps_skip_event, 0644, ps_skip_event_show, ps_skip_event_store);

static ssize_t ps_adc_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{

	uint16_t value;
	int ret;
	struct cm36686_info *lpi = lp_info;
	int intr_val;

	intr_val = gpio_get_value(lpi->intr_pin);

	get_ps_adc_value(&value);

	ret = sprintf(buf, "DEC ADC[%d], ENABLE = %d, intr_pin = %d\n", value, lpi->ps_enable, intr_val);

	return ret;
}

static ssize_t ps_enable_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int ps_en;
	struct cm36686_info *lpi = lp_info;

	ps_en = -1;
	sscanf(buf, "%d", &ps_en);

	if (ps_en != 0 && ps_en != 1
		&& ps_en != 10 && ps_en != 13 && ps_en != 16)
		return -EINVAL;

	if (ps_en) {
		D("[PS][CM36686] %s: ps_en=%d\n",
			__func__, ps_en);
		psensor_enable(lpi);
	} else
		psensor_disable(lpi);

	D("[PS][CM36686] %s\n", __func__);

	return count;
}

static DEVICE_ATTR(ps_adc, 0644, ps_adc_show, ps_enable_store);

unsigned PS_cmd_test_value;
static ssize_t ps_parameters_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	int ret;
	struct cm36686_info *lpi = lp_info;

	ret = sprintf(buf, "PS_close_thd_set = 0x%x, PS_away_thd_set = 0x%x, PS_cmd_cmd:value = 0x%x\n",
		lpi->ps_close_thd_set, lpi->ps_away_thd_set, PS_cmd_test_value);

	return ret;
}

static ssize_t ps_parameters_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{

	struct cm36686_info *lpi = lp_info;
	char *token[10];
	int i;

	printk(KERN_INFO "[PS][CM36686] %s\n", buf);
	for (i = 0; i < 3; i++)
		token[i] = strsep((char **)&buf, " ");

	lpi->ps_close_thd_set = simple_strtoul(token[0], NULL, 16);
	lpi->ps_away_thd_set = simple_strtoul(token[1], NULL, 16);	
	PS_cmd_test_value = simple_strtoul(token[2], NULL, 16);
	printk(KERN_INFO
		"[PS][CM36686]Set PS_close_thd_set = 0x%x, PS_away_thd_set = 0x%x, PS_cmd_cmd:value = 0x%x\n",
		lpi->ps_close_thd_set, lpi->ps_away_thd_set, PS_cmd_test_value);

	D("[PS][CM36686] %s\n", __func__);

	return count;
}

static DEVICE_ATTR(ps_parameters, 0644,
	ps_parameters_show, ps_parameters_store);


static ssize_t ps_conf_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cm36686_info *lpi = lp_info;
	return sprintf(buf, "PS_CONF1 = 0x%04x, PS_CONF3 = 0x%04x\n", lpi->ps_conf1_val, lpi->ps_conf3_val);
}
static ssize_t ps_conf_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int code1, code2;
	struct cm36686_info *lpi = lp_info;

	sscanf(buf, "0x%x 0x%x", &code1, &code2);

	D("[PS]%s: store value PS conf1 reg = 0x%04x PS conf3 reg = 0x%04x\n", __func__, code1, code2);

  lpi->ps_conf1_val = code1;
  lpi->ps_conf3_val = code2;

	_cm36686_I2C_Write_Word(lpi->slave_addr, PS_CONF3, lpi->ps_conf3_val );  
	_cm36686_I2C_Write_Word(lpi->slave_addr, PS_CONF1, lpi->ps_conf1_val );

	return count;
}
static DEVICE_ATTR(ps_conf, 0644, ps_conf_show, ps_conf_store);

static ssize_t ps_thd_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	int ret;
	struct cm36686_info *lpi = lp_info;
  ret = sprintf(buf, "[PS][CM36686]PS Hi/Low THD ps_close_thd_set = 0x%04x(%d), ps_away_thd_set = 0x%04x(%d)\n", lpi->ps_close_thd_set, lpi->ps_close_thd_set, lpi->ps_away_thd_set, lpi->ps_away_thd_set);
  return ret;	
}
static ssize_t ps_thd_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int code1, code2;
	struct cm36686_info *lpi = lp_info;

	sscanf(buf, "0x%x 0x%x", &code1, &code2);

	lpi->ps_close_thd_set = code1;	
	lpi->ps_away_thd_set = code2;
	
      _cm36686_I2C_Write_Word(lpi->slave_addr, PS_THDH, lpi->ps_close_thd_set );
	  _cm36686_I2C_Write_Word(lpi->slave_addr, PS_THDL, lpi->ps_away_thd_set );
	
	D("[PS][CM36686]%s: ps_close_thd_set = 0x%04x(%d), ps_away_thd_set = 0x%04x(%d)\n", __func__, lpi->ps_close_thd_set, lpi->ps_close_thd_set, lpi->ps_away_thd_set, lpi->ps_away_thd_set);
    
	return count;
}
static DEVICE_ATTR(ps_thd, 0644, ps_thd_show, ps_thd_store);

static ssize_t ps_canc_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	int ret = 0;
	struct cm36686_info *lpi = lp_info;

	ret = sprintf(buf, "[PS][CM36686]PS_CANC = 0x%04x(%d)\n", lpi->inte_cancel_set,lpi->inte_cancel_set);

	return ret;
}
static ssize_t ps_canc_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int code;
	struct cm36686_info *lpi = lp_info;

	sscanf(buf, "0x%x", &code);

	D("[PS][CM36686]PS_CANC: store value = 0x%04x(%d)\n", code,code);
	
	lpi->inte_cancel_set = code;	
	_cm36686_I2C_Write_Word(lpi->slave_addr, PS_CANC, lpi->inte_cancel_set );
	
	return count;
}
static DEVICE_ATTR(ps_canc, 0644, ps_canc_show, ps_canc_store);

static ssize_t ps_hw_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	int ret = 0;
	struct cm36686_info *lpi = lp_info;

	ret = sprintf(buf, "PS1: reg = 0x%x, PS3: reg = 0x%x, ps_close_thd_set = 0x%x, ps_away_thd_set = 0x%x\n",
		lpi->ps_conf1_val, lpi->ps_conf3_val, lpi->ps_close_thd_set, lpi->ps_away_thd_set);

	return ret;
}
static ssize_t ps_hw_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int code;
	
	sscanf(buf, "0x%x", &code);

	D("[PS]%s: store value = 0x%x\n", __func__, code);

	return count;
}
static DEVICE_ATTR(ps_hw, 0644, ps_hw_show, ps_hw_store);

static ssize_t ls_adc_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	int ret;
	struct cm36686_info *lpi = lp_info;

	D("[LS][CM36686] %s: ADC = 0x%04X, Level = %d \n",
		__func__, lpi->current_adc, lpi->current_level);
	ret = sprintf(buf, "ADC[0x%04X] => level %d\n",
		lpi->current_adc, lpi->current_level);

	return ret;
}

static DEVICE_ATTR(ls_adc, 0644, ls_adc_show, NULL);

static ssize_t ls_current_adc_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	
	struct cm36686_info	*lpi = lp_info;

	int	ret;

	uint16_t	ALS_conf, als_adc;

	mutex_lock( &als_enable_mutex );

	ret = _cm36686_I2C_Read_Word( lpi->slave_addr, ALS_CONF, &ALS_conf );

	ret = _cm36686_I2C_Read_Word( lpi->slave_addr, ALS_DATA, &als_adc );

	mutex_unlock( &als_enable_mutex );

	ret	= sprintf( buf, "ADC[0x%04x], ALS_CONF[0x%04x]\n", als_adc, ALS_conf );

	return	ret;

}

static DEVICE_ATTR(ls_current_adc, 0644, ls_current_adc_show, NULL);

static ssize_t ls_lux_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	int ret;
	struct cm36686_info *lpi = lp_info;

	printk( "Lux = %d\n", lpi->current_lux );

	ret	= sprintf( buf, "%d", lpi->current_lux );

	return ret;
}

static DEVICE_ATTR(ls_lux, 0644, ls_lux_show, NULL);

static ssize_t ls_ir_ink_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct cm36686_info *lpi = lp_info;
	unsigned value = 0;
	sscanf(buf, "%u", &value);

	lpi->IR_Ink = value;
	printk( "[IR_Ink] <= %u\n", lpi->IR_Ink );

	return count;
}

static ssize_t ls_ir_ink_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	int ret;
	struct cm36686_info *lpi = lp_info;

	printk( "[IR_Ink] <= %u\n", lpi->IR_Ink );

	ret	= sprintf( buf, "%d", lpi->IR_Ink );

	return ret;
}

static DEVICE_ATTR(ls_ir_ink, 0644, ls_ir_ink_show, ls_ir_ink_store );

static ssize_t ls_enable_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{

	int ret = 0;
	struct cm36686_info *lpi = lp_info;

	ret = sprintf(buf, "Light sensor Auto Enable = %d\n",
			lpi->als_enable);

	return ret;
}

static ssize_t ls_enable_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	int ret = 0;
	int ls_auto;
	struct cm36686_info *lpi = lp_info;

	ls_auto = -1;
	sscanf(buf, "%d", &ls_auto);

	if (ls_auto != 0 && ls_auto != 1 && ls_auto != 147)
		return -EINVAL;

	if (ls_auto) {
		lpi->ls_calibrate = (ls_auto == 147) ? 1 : 0;
		ret = lightsensor_enable(lpi);
	} else {
		lpi->ls_calibrate = 0;
		ret = lightsensor_disable(lpi);
	}

	D("[LS][CM36686] %s: lpi->als_enable = %d, lpi->ls_calibrate = %d, ls_auto=%d\n",
		__func__, lpi->als_enable, lpi->ls_calibrate, ls_auto);

	if (ret < 0)
		pr_err(
		"[LS][CM36686 error]%s: set auto light sensor fail\n",
		__func__);

	return count;
}

static DEVICE_ATTR(ls_auto, 0644,
	ls_enable_show, ls_enable_store);

static ssize_t ls_kadc_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct cm36686_info *lpi = lp_info;
	int ret;

	ret = sprintf(buf, "kadc = 0x%x",
			lpi->als_kadc);

	return ret;
}

static ssize_t ls_kadc_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct cm36686_info *lpi = lp_info;
	int kadc_temp = 0;

	sscanf(buf, "%d", &kadc_temp);

	mutex_lock(&als_get_adc_mutex);
  if(kadc_temp != 0) {
		lpi->als_kadc = kadc_temp;
		if(  lpi->als_gadc != 0){
  		if (lightsensor_update_table(lpi) < 0)
				printk(KERN_ERR "[LS][CM36686 error] %s: update ls table fail\n", __func__);
  	} else {
			printk(KERN_INFO "[LS]%s: als_gadc =0x%x wait to be set\n",
					__func__, lpi->als_gadc);
  	}		
	} else {
		printk(KERN_INFO "[LS]%s: als_kadc can't be set to zero\n",
				__func__);
	}
				
	mutex_unlock(&als_get_adc_mutex);
	return count;
}

static DEVICE_ATTR(ls_kadc, 0644, ls_kadc_show, ls_kadc_store);

static ssize_t ls_gadc_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct cm36686_info *lpi = lp_info;
	int ret;

	ret = sprintf(buf, "gadc = 0x%x\n", lpi->als_gadc);

	return ret;
}

static ssize_t ls_gadc_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct cm36686_info *lpi = lp_info;
	int gadc_temp = 0;

	sscanf(buf, "%d", &gadc_temp);
	
	mutex_lock(&als_get_adc_mutex);
  if(gadc_temp != 0) {
		lpi->als_gadc = gadc_temp;
		if(  lpi->als_kadc != 0){
  		if (lightsensor_update_table(lpi) < 0)
				printk(KERN_ERR "[LS][CM36686 error] %s: update ls table fail\n", __func__);
  	} else {
			printk(KERN_INFO "[LS]%s: als_kadc =0x%x wait to be set\n",
					__func__, lpi->als_kadc);
  	}		
	} else {
		printk(KERN_INFO "[LS]%s: als_gadc can't be set to zero\n",
				__func__);
	}
	mutex_unlock(&als_get_adc_mutex);
	return count;
}

static DEVICE_ATTR(ls_gadc, 0644, ls_gadc_show, ls_gadc_store);

static ssize_t ls_adc_table_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	unsigned length = 0;
	int i;

	for (i = 0; i < 10; i++) {
		length += sprintf(buf + length,
			"[CM36686]Get adc_table[%d] =  0x%x ; %d, Get cali_table[%d] =  0x%x ; %d, \n",
			i, *(lp_info->adc_table + i),
			*(lp_info->adc_table + i),
			i, *(lp_info->cali_table + i),
			*(lp_info->cali_table + i));
	}
	return length;
}

static ssize_t ls_adc_table_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{

	struct cm36686_info *lpi = lp_info;
	char *token[10];
	uint16_t tempdata[10];
	int i;

	printk(KERN_INFO "[LS][CM36686]%s\n", buf);
	for (i = 0; i < 10; i++) {
		token[i] = strsep((char **)&buf, " ");
		tempdata[i] = simple_strtoul(token[i], NULL, 16);
		if (tempdata[i] < 1 || tempdata[i] > 0xffff) {
			printk(KERN_ERR
			"[LS][CM36686 error] adc_table[%d] =  0x%x Err\n",
			i, tempdata[i]);
			return count;
		}
	}
	mutex_lock(&als_get_adc_mutex);
	for (i = 0; i < 10; i++) {
		lpi->adc_table[i] = tempdata[i];
		printk(KERN_INFO
		"[LS][CM36686]Set lpi->adc_table[%d] =  0x%x\n",
		i, *(lp_info->adc_table + i));
	}
	if (lightsensor_update_table(lpi) < 0)
		printk(KERN_ERR "[LS][CM36686 error] %s: update ls table fail\n",
		__func__);
	mutex_unlock(&als_get_adc_mutex);
	D("[LS][CM36686] %s\n", __func__);

	return count;
}

static DEVICE_ATTR(ls_adc_table, 0644,
	ls_adc_table_show, ls_adc_table_store);

static ssize_t ls_conf_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct cm36686_info *lpi = lp_info;
	return sprintf(buf, "ALS_CONF = %x\n", lpi->ls_cmd);
}
static ssize_t ls_conf_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct cm36686_info *lpi = lp_info;
	int value = 0;
	sscanf(buf, "0x%x", &value);

	lpi->ls_cmd = value;
	printk(KERN_INFO "[LS]set ALS_CONF = %x\n", lpi->ls_cmd);
	
	_cm36686_I2C_Write_Word(lpi->slave_addr, ALS_CONF, lpi->ls_cmd);
	return count;
}
static DEVICE_ATTR(ls_conf, 0644, ls_conf_show, ls_conf_store);

static ssize_t ls_fLevel_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "fLevel = %d\n", fLevel);
}
static ssize_t ls_fLevel_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct cm36686_info *lpi = lp_info;
	int value=0;
	sscanf(buf, "%d", &value);
	(value>=0)?(value=min(value,10)):(value=max(value,-1));
	fLevel=value;
	input_report_abs(lpi->ls_input_dev, ABS_MISC, fLevel);
	input_sync(lpi->ls_input_dev);
	printk(KERN_INFO "[LS]set fLevel = %d\n", fLevel);

	msleep(1000);
	fLevel=-1;
	return count;
}
static DEVICE_ATTR(ls_flevel, 0644, ls_fLevel_show, ls_fLevel_store);

static ssize_t ls_thd_offset_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	struct cm36686_info	*lpi = lp_info;

	int	ret;

	ret	= sprintf( buf, "Threshold Offset[High:Low]=[%d:%d]\n", lpi->ls_thd_high_offset, lpi->ls_thd_low_offset );
	
	return	ret;	

}
static ssize_t ls_thd_offset_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	
	struct cm36686_info	*lpi = lp_info;

	int	offset_high, offset_low;

	sscanf( buf, "%d %d", &offset_high, &offset_low );

	lpi->ls_thd_high_offset = offset_high, lpi->ls_thd_low_offset = offset_low;

	printk( "[LS][CM36686] Threshold Offset[High:Low]=[%d:%d]\n", offset_high, offset_low );

	return	count;

}
static DEVICE_ATTR(ls_thd_offset, 0644, ls_thd_offset_show, ls_thd_offset_store);

static int lightsensor_setup(struct cm36686_info *lpi)
{
	int ret;

	lpi->ls_input_dev = input_allocate_device();
	if (!lpi->ls_input_dev) {
		pr_err(
			"[LS][CM36686 error]%s: could not allocate ls input device\n",
			__func__);
		return -ENOMEM;
	}
	lpi->ls_input_dev->name = "cm36686-ls";
	set_bit(EV_ABS, lpi->ls_input_dev->evbit);
	input_set_abs_params(lpi->ls_input_dev, ABS_MISC, 0, 9, 0, 0);

	ret = input_register_device(lpi->ls_input_dev);
	if (ret < 0) {
		pr_err("[LS][CM36686 error]%s: can not register ls input device\n",
				__func__);
		goto err_free_ls_input_device;
	}

	ret = misc_register(&lightsensor_misc);
	if (ret < 0) {
		pr_err("[LS][CM36686 error]%s: can not register ls misc device\n",
				__func__);
		goto err_unregister_ls_input_device;
	}

	return ret;

err_unregister_ls_input_device:
	input_unregister_device(lpi->ls_input_dev);
err_free_ls_input_device:
	input_free_device(lpi->ls_input_dev);
	return ret;
}

static int psensor_setup(struct cm36686_info *lpi)
{
	int ret;

	lpi->ps_input_dev = input_allocate_device();
	if (!lpi->ps_input_dev) {
		pr_err(
			"[PS][CM36686 error]%s: could not allocate ps input device\n",
			__func__);
		return -ENOMEM;
	}
	lpi->ps_input_dev->name = "cm36686-ps";
	set_bit(EV_ABS, lpi->ps_input_dev->evbit);
	input_set_abs_params(lpi->ps_input_dev, ABS_DISTANCE, 0, 1, 0, 0);

	ret = input_register_device(lpi->ps_input_dev);
	if (ret < 0) {
		pr_err(
			"[PS][CM36686 error]%s: could not register ps input device\n",
			__func__);
		goto err_free_ps_input_device;
	}

	ret = misc_register(&psensor_misc);
	if (ret < 0) {
		pr_err(
			"[PS][CM36686 error]%s: could not register ps misc device\n",
			__func__);
		goto err_unregister_ps_input_device;
	}

	return ret;

err_unregister_ps_input_device:
	input_unregister_device(lpi->ps_input_dev);
err_free_ps_input_device:
	input_free_device(lpi->ps_input_dev);
	return ret;
}


static int initial_cm36686(struct cm36686_info *lpi)
{
	int val, ret;
	uint16_t idReg;

	val = gpio_get_value(lpi->intr_pin);
	D("[PS][CM36686] %s, INTERRUPT GPIO val = %d\n", __func__, val);

	ret = _cm36686_I2C_Read_Word(lpi->slave_addr, ID_REG, &idReg);

	if ((ret < 0) || (idReg != 0x0186)) {
  		if (record_init_fail == 0)
  			record_init_fail = 1;
  		return -ENOMEM;/*If devices without cm36686 chip and did not probe driver*/	
  }
  
	return 0;
}

static int cm36686_setup(struct cm36686_info *lpi)
{
	int ret = 0;

	als_power(1);
	msleep(5);
	ret = gpio_request(lpi->intr_pin, "gpio_cm36686_intr");
	if (ret < 0) {
		pr_err("[PS][CM36686 error]%s: gpio %d request failed (%d)\n",
			__func__, lpi->intr_pin, ret);
		return ret;
	}

	ret = gpio_direction_input(lpi->intr_pin);
	if (ret < 0) {
		pr_err(
			"[PS][CM36686 error]%s: fail to set gpio %d as input (%d)\n",
			__func__, lpi->intr_pin, ret);
		goto fail_free_intr_pin;
	}


	ret = initial_cm36686(lpi);
	if (ret < 0) {
		pr_err(
			"[PS_ERR][CM36686 error]%s: fail to initial cm36686 (%d)\n",
			__func__, ret);
		goto fail_free_intr_pin;
	}
	
	/*Default disable P sensor and L sensor*/
  ls_initial_cmd(lpi);
	psensor_initial_cmd(lpi);

	ret = request_any_context_irq(lpi->irq,
			cm36686_irq_handler,
			IRQF_TRIGGER_LOW,
			"cm36686",
			lpi);
	if (ret < 0) {
		pr_err(
			"[PS][CM36686 error]%s: req_irq(%d) fail for gpio %d (%d)\n",
			__func__, lpi->irq,
			lpi->intr_pin, ret);
		goto fail_free_intr_pin;
	}

	enable_irq_wake(lpi->irq);

	return ret;

fail_free_intr_pin:
	gpio_free(lpi->intr_pin);
	return ret;
}

static bool pin_control( struct cm36686_info *lpi )
{

	struct device		*dev = &lpi->i2c_client->dev;

	struct pinctrl		*ts_pinctrl;

	struct pinctrl_state	*gpio_state_active, *gpio_state_sleep;

	int			ret;

	/* Get pinctrl if target uses pinctrl */
	ts_pinctrl	= devm_pinctrl_get( dev );

	if( IS_ERR_OR_NULL( ts_pinctrl ) )
	{

		printk( "PLS : Target does not use pinctrl\n" );

		return	false;

	}

	gpio_state_active	= pinctrl_lookup_state( ts_pinctrl, "pls_active" );

	if( IS_ERR_OR_NULL( gpio_state_active ) )
	{

		printk( "PLS : Can not get ts default pinstate( pls_active )\n" );

		return	false;

	}

	ret	= pinctrl_select_state( ts_pinctrl, gpio_state_active );

	if( ret )
	{

		printk( "PLS : can not set pins( pls_active )\n" );

		return	false;

	}

	gpio_state_sleep	= pinctrl_lookup_state( ts_pinctrl, "pls_suspend" );

	if( IS_ERR_OR_NULL( gpio_state_sleep ) )
	{

		printk( "PLS : Can not get ts default pinstate( pls_suspend )\n" );

		return	false;

	}

	ret	= pinctrl_select_state( ts_pinctrl, gpio_state_sleep );

	if( ret )
	{

		printk( "PLS : can not set pins( pls_suspend )\n" );

		return	false;

	}

	return	true;

}

static bool get_property_u16_array( struct device_node *np, const char *name, u16 *array, unsigned count )
{

	const __be32	*values;

	unsigned	loop;

	int		size;

	values	= of_get_property( np, name, &size );

	if( count < size / sizeof( unsigned ) )
		return	false;

	for( loop = 0 ; loop < count ; ++loop )
		*( array + loop )	= be32_to_cpup( values++ );

	return	true;

}

static bool get_property_unsigned_array( struct device_node *np, const char *name, unsigned *array, unsigned count )
{

	const __be32	*values;

	unsigned	loop;

	int		size;

	values	= of_get_property( np, name, &size );

	if( count < size / sizeof( unsigned ) )
		return	false;

	for( loop = 0 ; loop < count ; ++loop )
		*( array + loop )	= be32_to_cpup( values++ );

	return	true;

}

static unsigned	get_property( struct device_node *np, const char *name )
{

	const __be32	*values;

	int		size;

	values	= of_get_property( np, name, &size );

	return	be32_to_cpup( values );

}

static bool device_tree( struct cm36686_info *lpi )
{

	struct device_node *np = lpi->i2c_client->dev.of_node;

	lpi->intr_pin	= of_get_named_gpio( np, "intr", 0 );

	lpi->irq	= gpio_to_irq( lpi->intr_pin );

	if( !get_property_u16_array( np, "cm36286,levels", lpi->adc_table, 10 ) )
		return	false;

	if( !get_property_unsigned_array( np, "cm36286,vdd_power", lpi->vdd_power, 3 ) )
		return	false;

	if( !get_property_unsigned_array( np, "cm36286,i2c_power", lpi->i2c_power, 3 ) )
		return	false;

	lpi->slave_addr		= get_property( np, "cm36286,slave_addr" );

	lpi->ps_away_thd_set	= get_property( np, "cm36286,ps_away_thd_set" );

	lpi->ps_close_thd_set	= get_property( np, "cm36286,ps_close_thd_set" );

	lpi->ps_conf1_val	= get_property( np, "cm36286,ps_conf1_val" );

	lpi->ps_conf3_val	= get_property( np, "cm36286,ps_conf3_val" );

	lpi->ls_cmd		= get_property( np, "cm36286,ls_cmd" );

	lpi->ls_thd_high_offset	= get_property( np, "cm36286,ls_thd_high_offset" );

	lpi->ls_thd_low_offset	= get_property( np, "cm36286,ls_thd_low_offset" );

	return	true;

}

bool	enable_power_source( struct cm36686_info *lpi )
{

	struct	power_source_data
	{

		char	*name;

		unsigned	min_voltage, max_voltage, load_current;

	};

	struct power_source_data	power_source[] =
	{

		{ "vdd", lpi->vdd_power[ 0 ], lpi->vdd_power[ 1 ], lpi->vdd_power[ 2 ] },
		{ "vcc_i2c", lpi->i2c_power[ 0 ], lpi->i2c_power[ 1 ], lpi->i2c_power[ 2 ] }

	};

	struct device		*dev = &lpi->i2c_client->dev;

	struct regulator	*power;

	unsigned	loop;

	for( loop = 0 ; loop < sizeof( power_source ) / sizeof( *power_source ) ; ++loop )
	{ // enable loop

		struct power_source_data	*source = power_source + loop;

		power	= regulator_get( dev, source->name );

		if( IS_ERR( power ) )
		{ // Failed

			printk( "PLS : Failed to get %s regulator\n", source->name );

			return	false;

		} // Failed

		if( regulator_count_voltages( power ) > 0 )
		{ // Set voltage & current

			if( regulator_set_voltage( power, source->min_voltage, source->max_voltage ) )
			{ // Failed

				printk( "PLS : reg set %s vtg failed\n", source->name );

				return	false;

			} // Failed

			if( regulator_set_optimum_mode( power, source->load_current ) < 0 )
			{ // Failed

				printk( "PLS : Regulator %s set_opt failed\n", source->name );

				return	false;

			} // Failed

		} // Set voltage & current

		if( regulator_enable( power ) )
		{ // Failed

			printk( "PLS : Regulator %s enable failed\n", source->name );

			return	false;

		} // Failed

	} // enable loop

	return	true;

}

static int cm36686_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret = 0;
	struct cm36686_info *lpi;

	D("[ALS+PS][CM36686] %s\n", __func__);


	lpi = kzalloc(sizeof(struct cm36686_info), GFP_KERNEL);
	if (!lpi)
		return -ENOMEM;

	/*D("[CM36686] %s: client->irq = %d\n", __func__, client->irq);*/

	lpi->i2c_client = client;

	i2c_set_clientdata(client, lpi);

	if( !pin_control( lpi ) )
		return	-1;

	if( !device_tree( lpi ) )
		return	-1;

	if( !enable_power_source( lpi ) )
		return	-1;

	ps_cancel_set = lpi->inte_cancel_set;
	
	lpi->record_clear_int_fail=0;
	
	D("[PS][CM36686] %s: ls_cmd 0x%x\n",
		__func__, lpi->ls_cmd);

	if( lpi->ls_cmd == 0 )
		lpi->ls_cmd	= CM36686_ALS_IT_160ms | CM36686_ALS_PERS_2;

	lp_info = lpi;

	mutex_init(&CM36686_control_mutex);

	mutex_init(&als_enable_mutex);
	mutex_init(&als_disable_mutex);
	mutex_init(&als_get_adc_mutex);

	ret = lightsensor_setup(lpi);
	if (ret < 0) {
		pr_err("[LS][CM36686 error]%s: lightsensor_setup error!!\n",
			__func__);
		goto err_lightsensor_setup;
	}

	mutex_init(&ps_enable_mutex);
	mutex_init(&ps_disable_mutex);
	mutex_init(&ps_get_adc_mutex);

	ret = psensor_setup(lpi);
	if (ret < 0) {
		pr_err("[PS][CM36686 error]%s: psensor_setup error!!\n",
			__func__);
		goto err_psensor_setup;
	}

  //SET LUX STEP FACTOR HERE
  // if adc raw value one step = 5/100 = 1/20 = 0.05 lux
  // the following will set the factor 0.05 = 1/20
  // and lpi->golden_adc = 1;  
  // set als_kadc = (ALS_CALIBRATED <<16) | 20;

  als_kadc = (ALS_CALIBRATED <<16) | 20;
  lpi->golden_adc = 1;

  //ls calibrate always set to 1 
  lpi->ls_calibrate = 1;

	lightsensor_set_kvalue(lpi);
	ret = lightsensor_update_table(lpi);
	if (ret < 0) {
		pr_err("[LS][CM36686 error]%s: update ls table fail\n",
			__func__);
		goto err_lightsensor_update_table;
	}

	lpi->lp_wq = create_singlethread_workqueue("cm36686_wq");
	if (!lpi->lp_wq) {
		pr_err("[PS][CM36686 error]%s: can't create workqueue\n", __func__);
		ret = -ENOMEM;
		goto err_create_singlethread_workqueue;
	}
	wake_lock_init(&(lpi->ps_wake_lock), WAKE_LOCK_SUSPEND, "proximity");

	ret = cm36686_setup(lpi);
	if (ret < 0) {
		pr_err("[PS_ERR][CM36686 error]%s: cm36686_setup error!\n", __func__);
		goto err_cm36686_setup;
	}
	lpi->cm36686_class = class_create(THIS_MODULE, "capella_sensors");
	if (IS_ERR(lpi->cm36686_class)) {
		ret = PTR_ERR(lpi->cm36686_class);
		lpi->cm36686_class = NULL;
		goto err_create_class;
	}

	lpi->ls_dev = device_create(lpi->cm36686_class,
				NULL, 0, "%s", "lightsensor");
	if (unlikely(IS_ERR(lpi->ls_dev))) {
		ret = PTR_ERR(lpi->ls_dev);
		lpi->ls_dev = NULL;
		goto err_create_ls_device;
	}

	/* register the attributes */
	ret = device_create_file(lpi->ls_dev, &dev_attr_ls_adc);
	if (ret)
		goto err_create_ls_device_file;

	/* register the attributes */
	ret = device_create_file(lpi->ls_dev, &dev_attr_ls_current_adc);
	if (ret)
		goto err_create_ls_device_file;

	/* register the attributes */
	ret = device_create_file(lpi->ls_dev, &dev_attr_ls_auto);
	if (ret)
		goto err_create_ls_device_file;

	/* register the attributes */
	ret = device_create_file(lpi->ls_dev, &dev_attr_ls_kadc);
	if (ret)
		goto err_create_ls_device_file;

	ret = device_create_file(lpi->ls_dev, &dev_attr_ls_gadc);
	if (ret)
		goto err_create_ls_device_file;

	ret = device_create_file(lpi->ls_dev, &dev_attr_ls_adc_table);
	if (ret)
		goto err_create_ls_device_file;

	ret = device_create_file(lpi->ls_dev, &dev_attr_ls_conf);
	if (ret)
		goto err_create_ls_device_file;

	ret = device_create_file(lpi->ls_dev, &dev_attr_ls_flevel);
	if (ret)
		goto err_create_ls_device_file;

	ret = device_create_file(lpi->ls_dev, &dev_attr_ls_thd_offset);
	if (ret)
		goto err_create_ls_device_file;

	ret = device_create_file(lpi->ls_dev, &dev_attr_ls_lux);
	if (ret)
		goto err_create_ls_device_file;

	ret = device_create_file(lpi->ls_dev, &dev_attr_ls_ir_ink);
	if (ret)
		goto err_create_ls_device_file;

	lpi->ps_dev = device_create(lpi->cm36686_class,
				NULL, 0, "%s", "proximity");
	if (unlikely(IS_ERR(lpi->ps_dev))) {
		ret = PTR_ERR(lpi->ps_dev);
		lpi->ps_dev = NULL;
		goto err_create_ls_device_file;
	}

	/* register the attributes */
	ret = device_create_file(lpi->ps_dev, &dev_attr_ps_skip_event);
	if (ret)
		goto err_create_ps_device;

	ret = device_create_file(lpi->ps_dev, &dev_attr_ps_adc);
	if (ret)
		goto err_create_ps_device;

	ret = device_create_file(lpi->ps_dev,
		&dev_attr_ps_parameters);
	if (ret)
		goto err_create_ps_device;

	/* register the attributes */
	ret = device_create_file(lpi->ps_dev, &dev_attr_ps_conf);
	if (ret)
		goto err_create_ps_device;

	/* register the attributes */
	ret = device_create_file(lpi->ps_dev, &dev_attr_ps_thd);
	if (ret)
		goto err_create_ps_device;
		
	/* register the attributes */
	ret = device_create_file(lpi->ps_dev, &dev_attr_ps_canc);
	if (ret)
		goto err_create_ps_device;
		
	ret = device_create_file(lpi->ps_dev, &dev_attr_ps_hw);
	if (ret)
		goto err_create_ps_device;

	D("[PS][CM36686] %s: Probe success!\n", __func__);

	return ret;

err_create_ps_device:
	device_unregister(lpi->ps_dev);
err_create_ls_device_file:
	device_unregister(lpi->ls_dev);
err_create_ls_device:
	class_destroy(lpi->cm36686_class);
err_create_class:
err_cm36686_setup:
	destroy_workqueue(lpi->lp_wq);
	wake_lock_destroy(&(lpi->ps_wake_lock));

	input_unregister_device(lpi->ls_input_dev);
	input_free_device(lpi->ls_input_dev);
	input_unregister_device(lpi->ps_input_dev);
	input_free_device(lpi->ps_input_dev);
err_create_singlethread_workqueue:
err_lightsensor_update_table:
	misc_deregister(&psensor_misc);
err_psensor_setup:
	mutex_destroy(&CM36686_control_mutex);
	mutex_destroy(&ps_enable_mutex);
	mutex_destroy(&ps_disable_mutex);
	mutex_destroy(&ps_get_adc_mutex);
	misc_deregister(&lightsensor_misc);
err_lightsensor_setup:
	mutex_destroy(&als_enable_mutex);
	mutex_destroy(&als_disable_mutex);
	mutex_destroy(&als_get_adc_mutex);

	kfree(lpi);
	return ret;
}

static int control_and_report( struct cm36686_info *lpi, uint8_t mode, uint16_t param ) {
	int ret=0;
	uint16_t adc_value = 0;
	uint16_t ps_data = 0;
	int level = 0, i, val;

	int	ls_high_thd, ls_low_thd;
	
  mutex_lock(&CM36686_control_mutex);
   if( mode == CONTROL_ALS ){
    if(param){
      lpi->ls_cmd &= CM36686_ALS_SD_MASK;      
    } else {
      lpi->ls_cmd |= CM36686_ALS_SD;
    }
    _cm36686_I2C_Write_Word(lpi->slave_addr, ALS_CONF, lpi->ls_cmd);
    lpi->als_enable=param;
  } else if( mode == CONTROL_PS ){
    if(param){ 
      lpi->ps_conf1_val &= CM36686_PS_SD_MASK;
      lpi->ps_conf1_val |= CM36686_PS_INT_IN_AND_OUT;      
    } else {
      lpi->ps_conf1_val |= CM36686_PS_SD;
      lpi->ps_conf1_val &= CM36686_PS_INT_MASK;
    }
    _cm36686_I2C_Write_Word(lpi->slave_addr, PS_CONF1, lpi->ps_conf1_val);    
    lpi->ps_enable=param;  
  }
  if((mode == CONTROL_ALS)||(mode == CONTROL_PS)){  
    if( param==1 ){
		  msleep(100);  
    }
  }
     	
  if(lpi->als_enable){
    if( mode == CONTROL_ALS ||
      ( mode == CONTROL_INT_ISR_REPORT && 
      ((param&INT_FLAG_ALS_IF_L)||(param&INT_FLAG_ALS_IF_H)))){
    
    	  lpi->ls_cmd &= CM36686_ALS_INT_MASK;
    	  ret = _cm36686_I2C_Write_Word(lpi->slave_addr, ALS_CONF, lpi->ls_cmd);  
      
        get_ls_adc_value(&adc_value, 0);

if( 0 )
{
        if( lpi->ls_calibrate ) {
        	for (i = 0; i < 10; i++) {
      	  	if (adc_value <= (*(lpi->cali_table + i))) {
      		  	level = i;
      			  if (*(lpi->cali_table + i))
      				  break;
      		  }
      		  if ( i == 9) {/*avoid  i = 10, because 'cali_table' of size is 10 */
      			  level = i;
      			  break;
      		  }
      	  }
        } else {
      	  for (i = 0; i < 10; i++) {
      		  if (adc_value <= (*(lpi->adc_table + i))) {
      			  level = i;
      			  if (*(lpi->adc_table + i))
      				  break;
      		  }
      		  if ( i == 9) {/*avoid  i = 10, because 'cali_table' of size is 10 */
      			  level = i;
      			  break;
      		  }
      	  }
    	  }
    
    	  ret = set_lsensor_range(((i == 0) || (adc_value == 0)) ? 0 :
    		   	*(lpi->cali_table + (i - 1)) + 1,
    		    *(lpi->cali_table + i));
}

	ls_high_thd = adc_value + lpi->ls_thd_high_offset, ls_low_thd = adc_value > lpi->ls_thd_low_offset ? adc_value - lpi->ls_thd_low_offset : 0;

	if( ls_low_thd <= 0x10 )
		ls_low_thd	= 1;

	set_lsensor_range( ls_low_thd, ls_high_thd );
    	  
        lpi->ls_cmd |= CM36686_ALS_INT_EN;
    	  
        ret = _cm36686_I2C_Write_Word(lpi->slave_addr, ALS_CONF, lpi->ls_cmd);  
    	  
//    		if ((i == 0) || (adc_value == 0))
//    			D("[LS][CM36686] %s: ADC=0x%03X, Level=%d, l_thd equal 0, h_thd = 0x%x \n",
//    				__func__, adc_value, level, *(lpi->cali_table + i));
//    		else
//    			D("[LS][CM36686] %s: ADC=0x%03X, Level=%d, l_thd = 0x%x, h_thd = 0x%x \n",
//    				__func__, adc_value, level, *(lpi->cali_table + (i - 1)) + 1, *(lpi->cali_table + i));
    		lpi->current_level = level;
    		lpi->current_adc = adc_value;

		{

			static unsigned	Lux_step[] = { 8, 4, 2, 1 };

			unsigned	IR_Ink = lpi->IR_Ink, ADC = adc_value, Step = Lux_step[ lpi->ls_cmd >> 6 & 0x3 ];
			
			unsigned	Lux = ( IR_Ink * ADC * Step + 5000 ) / 1000 / 100;

			static unsigned	Count = 0;

			Lux	= Lux <= 7 ? 0 : Lux;

			lpi->current_lux	= Lux;

			if( !Count )
				printk( "[LS][CM36686] [L][A:I:S]=[%d][0x%x:%d:%d]\n", Lux, adc_value, IR_Ink, Step );

			Count	= ( Count + 1 ) % 3;

			input_report_abs( lpi->ls_input_dev, ABS_MISC, Lux );

			input_sync( lpi->ls_input_dev );

		}

    }
  }

#define PS_CLOSE 1
#define PS_AWAY  (1<<1)
#define PS_CLOSE_AND_AWAY PS_CLOSE+PS_AWAY
   if(lpi->ps_enable){
    int ps_status = 0;

    if( mode == CONTROL_PS )
      ps_status = PS_CLOSE_AND_AWAY;   
    else if(mode == CONTROL_INT_ISR_REPORT ){  
      if ( param & INT_FLAG_PS_IF_CLOSE )
        ps_status |= PS_CLOSE;      
      if ( param & INT_FLAG_PS_IF_AWAY )
        ps_status |= PS_AWAY;
    }

printk( "D[PS] S(%d)\n", ps_status );

    if (ps_status!=0){
      switch(ps_status){
        case PS_CLOSE_AND_AWAY:
		  get_stable_ps_adc_value(&ps_data);
          val = (ps_data >= lpi->ps_close_thd_set) ? 1 : 0;

	  D( "[PS][CM36686] ADC = %d\n", ps_data );

          break;
        case PS_AWAY:
          val = 0;
		  D("[PS][CM36686] proximity detected object away\n");

proximity_sensor_state	= false;

		  break;
        case PS_CLOSE:
          val = 1;
		  D("[PS][CM36686] proximity detected object close\n");

proximity_sensor_state	= true;

          break;
        };

      wake_lock_timeout( &lpi->ps_wake_lock, HZ / 2 );

      input_report_abs(lpi->ps_input_dev, ABS_DISTANCE, val);      
      input_sync(lpi->ps_input_dev);

      D( "[PS][CM36686] State = %d\n", val );

    }
  }

  mutex_unlock(&CM36686_control_mutex);
  return ret;
}

static struct of_device_id cm36286_match_table[] = {
	{ .compatible = "capella,cm36286",},
	{ },
};

static const struct i2c_device_id cm36686_i2c_id[] = {
	{CM36686_I2C_NAME, 0},
	{}
};

static struct i2c_driver cm36686_driver = {
	.id_table = cm36686_i2c_id,
	.probe = cm36686_probe,
	.driver = {
		.name = CM36686_I2C_NAME,
		.owner = THIS_MODULE,
		.of_match_table = cm36286_match_table,
	},
};

static int __init cm36686_init(void)
{
	return i2c_add_driver(&cm36686_driver);
}

static void __exit cm36686_exit(void)
{
	i2c_del_driver(&cm36686_driver);
}

module_init(cm36686_init);
module_exit(cm36686_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CM36686 Driver");
