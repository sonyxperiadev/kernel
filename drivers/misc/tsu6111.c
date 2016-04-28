/*
 * driver/misc/tsu6111.c - TSU6111 micro USB switch device driver
 *
 * Copyright (C) 2010 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Wonguk Jeong <wonguk.jeong@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/i2c-algo-bit.h>
#include <linux/i2c-algo-bit.h>
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/i2c/tsu6111.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include <linux/input.h>
#include <linux/power_supply.h>

/* TSU6111 I2C registers */
#define TSU6111_REG_DEVID		0x01
#define TSU6111_REG_CTRL		0x02
#define TSU6111_REG_INT1		0x03
#define TSU6111_REG_INT2		0x04
#define TSU6111_REG_INT1_MASK		0x05
#define TSU6111_REG_INT2_MASK		0x06
#define TSU6111_REG_ADC			0x07
#define TSU6111_REG_TIMING1		0x08
#define TSU6111_REG_TIMING2		0x09
#define TSU6111_REG_DEV_T1		0x0a
#define TSU6111_REG_DEV_T2		0x0b
#define TSU6111_REG_BTN1		0x0c
#define TSU6111_REG_BTN2		0x0d
#define TSU6111_REG_CK			0x0e
#define TSU6111_REG_CK_INT1		0x0f
#define TSU6111_REG_CK_INT2		0x10
#define TSU6111_REG_CK_INTMASK1		0x11
#define TSU6111_REG_CK_INTMASK2		0x12
#define TSU6111_REG_MANSW1		0x13
#define TSU6111_REG_MANSW2		0x14
#define TSU6111_REG_MANUAL_OVERRIDES1	0x1B
#define TSU6111_REG_RESERVED_1D		0x20

/* Control */
#define CON_SWITCH_OPEN		(1 << 4)
#define CON_RAW_DATA		(1 << 3)
#define CON_MANUAL_SW		(1 << 2)
#define CON_WAIT		(1 << 1)
#define CON_INT_MASK		(1 << 0)
#define CON_MASK		(CON_SWITCH_OPEN | CON_RAW_DATA | \
				CON_MANUAL_SW | CON_WAIT)

/* Device Type 1 */
#define DEV_USB_OTG		(1 << 7)
#define DEV_DEDICATED_CHG	(1 << 6)
#define DEV_USB_CHG		(1 << 5)
#define DEV_CAR_KIT		(1 << 4)
#define DEV_UART		(1 << 3)
#define DEV_USB			(1 << 2)
#define DEV_AUDIO_2		(1 << 1)
#define DEV_AUDIO_1		(1 << 0)

#define DEV_T1_USB_MASK		(DEV_USB_OTG | DEV_USB)
#define DEV_T1_UART_MASK	(DEV_UART)
#define DEV_T1_CHARGER_MASK	(DEV_DEDICATED_CHG | DEV_USB_CHG | DEV_CAR_KIT)

/* Device Type 2 */
#define DEV_AV			(1 << 6)
#define DEV_TTY			(1 << 5)
#define DEV_PPD			(1 << 4)
#define DEV_JIG_UART_OFF	(1 << 3)
#define DEV_JIG_UART_ON		(1 << 2)
#define DEV_JIG_USB_OFF		(1 << 1)
#define DEV_JIG_USB_ON		(1 << 0)

#define DEV_T2_USB_MASK		(DEV_JIG_USB_OFF | DEV_JIG_USB_ON)
#define DEV_T2_UART_MASK	(DEV_JIG_UART_OFF | DEV_JIG_UART_ON)
#define DEV_T2_JIG_MASK		(DEV_JIG_USB_OFF | DEV_JIG_USB_ON | \
				DEV_JIG_UART_OFF | DEV_JIG_UART_ON)

/*
 * Manual Switch
 * D- [7:5] / D+ [4:2]
 * 000: Open all / 001: USB / 010: AUDIO / 011: UART / 100: V_AUDIO
 */
#define SW_VAUDIO		((4 << 5) | (4 << 2) | (1 << 1) | (1 << 0))
#define SW_UART			((3 << 5) | (3 << 2))
#define SW_AUDIO		((2 << 5) | (2 << 2) | (1 << 1) | (1 << 0))
#define SW_DHOST		((1 << 5) | (1 << 2) | (1 << 1) | (1 << 0))
#define SW_AUTO			((0 << 5) | (0 << 2))
#define SW_USB_OPEN		(1 << 0)
#define SW_ALL_OPEN		(0)

/* Interrupt 1 */
#define INT_DETACH		(1 << 1)
#define INT_ATTACH		(1 << 0)

#define	ADC_GND			0x00
#define	ADC_MHL			0x01
#define	ADC_DOCK_VOL_DN		0x0a
#define	ADC_DOCK_VOL_UP		0x0b
#define	ADC_CEA936ATYPE1_CHG	0x17
#define	ADC_JIG_USB_OFF		0x18
#define	ADC_JIG_USB_ON		0x19
#define	ADC_DESKDOCK		0x1a
#define	ADC_CEA936ATYPE2_CHG	0x1b
#define	ADC_JIG_UART_OFF	0x1c
#define	ADC_JIG_UART_ON		0x1d
#define	ADC_CARDOCK		0x1d
#define ADC_OPEN		0x1f

struct tsu6111_usbsw {
	struct i2c_client		*client;
	struct tsu6111_platform_data	*pdata;
	int				dev1;
	int				dev2;
	int				mansw;

	struct input_dev	*input;
	int			previous_key;

	struct delayed_work	init_work;
	struct delayed_work	detect_work;
	struct mutex		mutex;
};

enum {
	DOCK_KEY_NONE			= 0,
	DOCK_KEY_VOL_UP_PRESSED,
	DOCK_KEY_VOL_UP_RELEASED,
	DOCK_KEY_VOL_DOWN_PRESSED,
	DOCK_KEY_VOL_DOWN_RELEASED,
};

extern int bcmpmu_check_vbus();

static struct tsu6111_usbsw *local_usbsw=NULL;


static int tsu6111_write_reg(struct i2c_client *client,        u8 reg, u8 data)
{
       int ret;
       u8 buf[2];
       struct i2c_msg msg[1];

	buf[0] = reg;
	buf[1] = data;

	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = buf;

       ret = i2c_transfer(client->adapter, msg, 1);
       if (ret != 1) {
               printk("\n [tsu6111] i2c Write Failed (ret=%d) \n", ret);
               return -1;
       }

	return ret;
}

static int tsu6111_read_reg(struct i2c_client *client, u8 reg, u8 *data)
{
       int ret;
       u8 buf[1];
       struct i2c_msg msg[2];

	buf[0] = reg;

        msg[0].addr = client->addr;
        msg[0].flags = 0;
        msg[0].len = 1;
        msg[0].buf = buf;

        msg[1].addr = client->addr;
        msg[1].flags = I2C_M_RD;
        msg[1].len = 1;
        msg[1].buf = buf;

       ret = i2c_transfer(client->adapter, msg, 2);
       if (ret != 2) {
               printk("\n [tsu6111] i2c Read Failed (ret=%d) \n", ret);
               return -1;
       }
       *data = buf[0];

	return 0;
}

static int tsu6111_read_word_reg(struct i2c_client *client, u8 reg, int *data)
{
       int ret;
       u8 buf[1];
	   u8 data1,data2;
       struct i2c_msg msg[2];

	buf[0] = reg;

        msg[0].addr = client->addr;
        msg[0].flags = 0;
        msg[0].len = 1;
        msg[0].buf = buf;

        msg[1].addr = client->addr;
        msg[1].flags = I2C_M_RD;
        msg[1].len = 1;
        msg[1].buf = buf;

       ret = i2c_transfer(client->adapter, msg, 2);
       if (ret != 2) {
               printk("\n [tsu6111] i2c Read Failed (ret=%d) \n", ret);
               return -1;
       }

	data1 = buf[0];

	buf[0] = reg+1;

        msg[0].addr = client->addr;
        msg[0].flags = 0;
        msg[0].len = 1;
        msg[0].buf = buf;

        msg[1].addr = client->addr;
        msg[1].flags = I2C_M_RD;
        msg[1].len = 1;
        msg[1].buf = buf;

       ret = i2c_transfer(client->adapter, msg, 2);
       if (ret != 2) {
               printk("\n [tsu6111] i2c Read Failed (ret=%d) \n", ret);
               return -1;
       }

	data2 = buf[0];

	*data = (data2<<8) | data1;


	return 0;
}
void musb_vbus_changed(int state)
{
	if( !local_usbsw ){
		printk("failed to allocate driver data\n");
		return 0;
	}

	struct tsu6111_usbsw *usbsw = local_usbsw;
	struct tsu6111_platform_data *pdata = usbsw->pdata;

	if(usbsw->dev2 & DEV_AV){
		printk("%s:dock vbus is %s\n",__func__,(state?"enabled":"disabled"));

		if(state){
			if(bcmpmu_check_vbus())
				if (pdata->charger_cb)
					pdata->charger_cb(TSU6111_ATTACHED);
		}else{
			if (pdata->charger_cb)
				pdata->charger_cb(TSU6111_DETACHED);
		}
	}

}
EXPORT_SYMBOL(musb_vbus_changed);

unsigned int musb_get_charger_type(void)
{
	if( !local_usbsw ){
		printk("failed to allocate driver data\n");
		return POWER_SUPPLY_TYPE_BATTERY;
	}

	struct i2c_client *client = local_usbsw->client;
	int value = 0;

	tsu6111_read_word_reg(client, TSU6111_REG_DEV_T1, &value);
	printk("%s chrgr type 0x%x\n", __func__, value);

	if(value & DEV_USB)
	{
		return POWER_SUPPLY_TYPE_USB_CDP;
	}
	else if(value & DEV_T1_CHARGER_MASK || (value >> 8) & DEV_AV)
	{
		return POWER_SUPPLY_TYPE_USB_DCP;
	}
	else if(1 == bcmpmu_check_vbus())
	{
		printk("%s chrgr type : 3rd party charger - only vbus!!!\n", __func__);
		return POWER_SUPPLY_TYPE_USB_DCP;
	}

	return POWER_SUPPLY_TYPE_BATTERY;
}
EXPORT_SYMBOL(musb_get_charger_type);

u16 tsu6111_get_chrgr_type(void)
{
	if( !local_usbsw ){
		printk("failed to allocate driver data\n");
		return 0;
        }

	struct i2c_client *client = local_usbsw->client;
	int value = 0;

	tsu6111_read_word_reg(client, TSU6111_REG_DEV_T1, &value);
	printk("%s chrgr type 0x%x\n", __func__, value);
	return value;
}
EXPORT_SYMBOL(tsu6111_get_chrgr_type);

/* for external charger detection  apart from PMU/BB*/
int bcm_ext_bc_status(void)
{
	return tsu6111_get_chrgr_type();
}
EXPORT_SYMBOL(bcm_ext_bc_status);

enum bcmpmu_chrgr_type_t
get_ext_charger_type(struct bcmpmu_accy *paccy, unsigned int bc_status)
{
	enum bcmpmu_chrgr_type_t type;

	if (bc_status) {
		if (bc_status &
			(JIG_BC_STS_UART_MSK |
				JIG_BC_STS_DCP_MSK))
			type = PMU_CHRGR_TYPE_DCP;
		else if (bc_status & JIG_BC_STS_SDP_MSK)
			type = PMU_CHRGR_TYPE_SDP;
		else if (bc_status & JIG_BC_STS_CDP_MSK)
			type = PMU_CHRGR_TYPE_CDP;
		else
			type = PMU_CHRGR_TYPE_NONE;
	} else
		type = PMU_CHRGR_TYPE_NONE;

	return type;
}
EXPORT_SYMBOL(get_ext_charger_type);

static void tsu6111_sw_reset(struct tsu6111_usbsw *usbsw)
{
	struct i2c_client *client = usbsw->client;

	disable_irq(client->irq);

	/*Hold SCL&SDA Low more than 30ms*/
	gpio_direction_output(GPIO_USB_I2C_SDA,0);
	gpio_direction_output(GPIO_USB_I2C_SCL,0);
	msleep(31);

	/*Make SCL&SDA High again*/
	gpio_direction_output(GPIO_USB_I2C_SDA,1);
	gpio_direction_output(GPIO_USB_I2C_SCL,1);

	/*Write SOME Init register value again*/
	enable_irq(client->irq);


}

static ssize_t tsu6111_show_timing2(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct tsu6111_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	unsigned char value = 0;

	tsu6111_read_reg(client,TSU6111_REG_TIMING2,&value);

	return snprintf(buf, 30, "TIMING2: %02x\n", value);
}


static ssize_t tsu6111_set_timing2(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct tsu6111_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	unsigned char value;

	value = (unsigned char) simple_strtoul(buf, NULL,16);
	printk(KERN_ALERT "input value = %x\n", value);

	tsu6111_write_reg(client,TSU6111_REG_TIMING2,value);

	return count;
}
static ssize_t tsu6111_show_timing1(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct tsu6111_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	unsigned char value = 0;

	tsu6111_read_reg(client,TSU6111_REG_TIMING1,&value);

	return snprintf(buf, 30, "TIMING1: %02x\n", value);
}


static ssize_t tsu6111_set_timing1(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct tsu6111_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	unsigned char value;

	value = (unsigned char) simple_strtoul(buf, NULL,16);
	printk(KERN_ALERT "input value = %x\n", value);

	tsu6111_write_reg(client,TSU6111_REG_TIMING1,value);

	return count;
}

static ssize_t tsu6111_show_control(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct tsu6111_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	unsigned char value = 0;

	tsu6111_read_reg(client,TSU6111_REG_CTRL,&value);

	return snprintf(buf, 30, "CONTROL: %02x\n", value);
}


static ssize_t tsu6111_set_control(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct tsu6111_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	unsigned char value;

	value = (unsigned char) simple_strtoul(buf, NULL,16);
	printk(KERN_ALERT "input value = %x\n", value);

	tsu6111_write_reg(client,TSU6111_REG_CTRL,value);

	return count;
}

static ssize_t tsu6111_reg_dump(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct tsu6111_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	unsigned char value = 0, i = 0;
	unsigned char count = 0;

	for (i=0; i <= 0x15; i++){
		tsu6111_read_reg(client,i,&value);
		count+=snprintf(&buf[count],20, "reg[%02x]=%02x\n",i,value);
	}
	printk(KERN_ALERT "count = %d\n", count);
	return count;
}

static ssize_t tsu6111_show_device_type(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct tsu6111_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	unsigned char value = 0;

	/* Checkout out i2c reads */
	tsu6111_read_reg(client,TSU6111_REG_DEVID,&value);
	printk(KERN_ALERT "device id = %x\n", value);

	tsu6111_read_reg(client,TSU6111_REG_DEV_T1,&value);
	printk(KERN_ALERT "device t1 = %x\n", value);

	tsu6111_read_reg(client,TSU6111_REG_DEV_T2,&value);
	printk(KERN_ALERT "device t2 = %x\n", value);

	return snprintf(buf, 30, "DEVICE_TYPE: %02x\n", value);
}

static ssize_t tsu6111_show_manualsw(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tsu6111_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	unsigned char value = 0;

	tsu6111_read_reg(client,TSU6111_REG_MANSW1,&value);
	printk(KERN_ALERT "manual sw1 = %x\n", value);

	if (value == SW_VAUDIO)
		return snprintf(buf, 15, "VAUDIO\n");
	else if (value == SW_UART)
		return snprintf(buf, 15, "UART\n");
	else if (value == SW_AUDIO)
		return snprintf(buf, 15, "AUDIO\n");
	else if (value == SW_DHOST)
		return snprintf(buf, 15, "DHOST\n");
	else if (value == SW_AUTO)
		return snprintf(buf, 15, "AUTO\n");
	else
		return snprintf(buf, 15, "%x", value);
}

static ssize_t tsu6111_set_manualsw(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct tsu6111_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	unsigned char value = 0;
	unsigned int path = 0;
	int ret;

	tsu6111_read_reg(client,TSU6111_REG_CTRL,&value);

	if ((value & ~CON_MANUAL_SW) !=
			(CON_SWITCH_OPEN | CON_RAW_DATA | CON_WAIT))
		return 0;

	if (!strncmp(buf, "VAUDIO", 6)) {
		path = SW_VAUDIO;
		value &= ~CON_MANUAL_SW;
	} else if (!strncmp(buf, "UART", 4)) {
		path = SW_UART;
		value &= ~CON_MANUAL_SW;
	} else if (!strncmp(buf, "AUDIO", 5)) {
		path = SW_AUDIO;
		value &= ~CON_MANUAL_SW;
	} else if (!strncmp(buf, "DHOST", 5)) {
		path = SW_DHOST;
		value &= ~CON_MANUAL_SW;
	} else if (!strncmp(buf, "AUTO", 4)) {
		path = SW_AUTO;
		value |= CON_MANUAL_SW;
	} else {
		dev_err(dev, "Wrong command\n");
		return 0;
	}

	usbsw->mansw = path;

	ret = tsu6111_write_reg(client, TSU6111_REG_MANSW1, path);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	ret = tsu6111_write_reg(client, TSU6111_REG_CTRL, value);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	return count;
}

static DEVICE_ATTR(control, S_IRUGO | S_IWUSR, tsu6111_show_control, tsu6111_set_control);
static DEVICE_ATTR(timing1, S_IRUGO | S_IWUSR, tsu6111_show_timing1, tsu6111_set_timing1);
static DEVICE_ATTR(timing2, S_IRUGO | S_IWUSR, tsu6111_show_timing2, tsu6111_set_timing2);
static DEVICE_ATTR(device_type, S_IRUGO, tsu6111_show_device_type, NULL);
static DEVICE_ATTR(switch, S_IRUGO | S_IWUSR,
		tsu6111_show_manualsw, tsu6111_set_manualsw);
static DEVICE_ATTR(regdump, S_IRUGO, tsu6111_reg_dump, NULL);

static struct attribute *tsu6111_attributes[] = {
	&dev_attr_control.attr,
	&dev_attr_timing1.attr,
	&dev_attr_timing2.attr,
	&dev_attr_device_type.attr,
	&dev_attr_switch.attr,
	&dev_attr_regdump.attr,
	NULL
};

static const struct attribute_group tsu6111_group = {
	.attrs = tsu6111_attributes,
};

void tsu6111_manual_switching(int path)
{
	struct i2c_client *client = local_usbsw->client;
	u8 value, data = 0;
	int ret;

	tsu6111_read_reg(client,TSU6111_REG_CTRL,&value);
	if (value < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, value);

	if ((value & ~CON_MANUAL_SW) !=
			(CON_SWITCH_OPEN | CON_RAW_DATA | CON_WAIT))
		return;

	if (path == SWITCH_PORT_VAUDIO) {
		data = SW_VAUDIO;
		value &= ~CON_MANUAL_SW;
	} else if (path ==  SWITCH_PORT_UART) {
		data = SW_UART;
		value &= ~CON_MANUAL_SW;
	} else if (path ==  SWITCH_PORT_AUDIO) {
		data = SW_AUDIO;
		value &= ~CON_MANUAL_SW;
	} else if (path ==  SWITCH_PORT_USB) {
		data = SW_DHOST;
		value &= ~CON_MANUAL_SW;
	} else if (path ==  SWITCH_PORT_AUTO) {
		data = SW_AUTO;
		value |= CON_MANUAL_SW;
	} else if (path ==  SWITCH_PORT_USB_OPEN) {
		data = SW_USB_OPEN;
		value &= ~CON_MANUAL_SW;
	} else if (path ==  SWITCH_PORT_ALL_OPEN) {
		data = SW_ALL_OPEN;
		value &= ~CON_MANUAL_SW;
	} else {
		pr_info("%s: wrong path (%d)\n", __func__, path);
		return;
	}

	local_usbsw->mansw = data;

	/* path for FTM sleep */
	if (path ==  SWITCH_PORT_ALL_OPEN) {
		ret = tsu6111_write_reg(client,
					TSU6111_REG_MANUAL_OVERRIDES1, 0x0a);
		if (ret < 0)
			dev_err(&client->dev, "%s: err %d\n", __func__, ret);

		ret = tsu6111_write_reg(client,
						TSU6111_REG_MANSW1, data);
		if (ret < 0)
			dev_err(&client->dev, "%s: err %d\n", __func__, ret);

		ret = tsu6111_write_reg(client,
						TSU6111_REG_MANSW2, data);
		if (ret < 0)
			dev_err(&client->dev, "%s: err %d\n", __func__, ret);

		ret = tsu6111_write_reg(client,
						TSU6111_REG_CTRL, value);
		if (ret < 0)
			dev_err(&client->dev, "%s: err %d\n", __func__, ret);
	} else {
		ret = tsu6111_write_reg(client,
						TSU6111_REG_MANSW1, data);
		if (ret < 0)
			dev_err(&client->dev, "%s: err %d\n", __func__, ret);

		ret = tsu6111_write_reg(client,
						TSU6111_REG_CTRL, value);
		if (ret < 0)
			dev_err(&client->dev, "%s: err %d\n", __func__, ret);
	}

}
EXPORT_SYMBOL(tsu6111_manual_switching);


static void tsu6111_reg_init(struct tsu6111_usbsw *usbsw)
{
	struct i2c_client *client = usbsw->client;
	unsigned int ctrl = CON_MASK;
	int ret;
	u8 val;

	pr_info("tsu6111_reg_init is called\n");
#if 1 //defined(CONFIG_MACH_HAWAII_SS_LOGAN_REV03) || defined(CONFIG_MACH_HAWAII_SS_LOGANDS_REV01)
	tsu6111_read_reg(client, TSU6111_REG_CTRL, &val);

	ctrl = val & (~0x1);

	ret = tsu6111_write_reg(client, TSU6111_REG_CTRL, ctrl);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);
#else
	tsu6111_write_reg(client,TSU6111_REG_INT1_MASK,0xfc);
	tsu6111_write_reg(client,TSU6111_REG_INT2_MASK,0xff);

	/* ADC Detect Time: 500ms */
	ret = tsu6111_write_reg(client, TSU6111_REG_TIMING1, /*0x6*/0x0);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	/* switching wait = 110ms */
	ret = tsu6111_write_reg(client, TSU6111_REG_TIMING2, /*0x5*/0x0);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	tsu6111_read_reg(client,TSU6111_REG_MANSW1,&val);
	usbsw->mansw = val;

	if (usbsw->mansw < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, usbsw->mansw);

	if (usbsw->mansw){
		ctrl &= ~CON_MANUAL_SW;	/* Manual Switching Mode */
		printk(KERN_ALERT
			"%s Manual switching mode enabled\n", __func__);
	}

	ret = tsu6111_write_reg(client, TSU6111_REG_CTRL, ctrl);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	ret = tsu6111_write_reg(client, TSU6111_REG_RESERVED_1D, 0x04);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	ret = tsu6111_read_reg(client,TSU6111_REG_DEVID,&val);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	dev_info(&client->dev, " tsu6111_reg_init dev ID: 0x%x\n", val);
#endif
}

static void tsu6111_detect_dev(struct tsu6111_usbsw *usbsw, u8 intr)
{
	int device_type;
	unsigned char val1, val2;
	struct tsu6111_platform_data *pdata = usbsw->pdata;
	struct i2c_client *client = usbsw->client;

	msleep(50);
	tsu6111_read_word_reg(client, TSU6111_REG_DEV_T1,&device_type);
	if (device_type < 0) {
		dev_err(&client->dev, "%s: err %d\n", __func__, device_type);
		return;
	}
	val1 = device_type & 0xff;
	val2 = device_type >> 8;

	dev_info(&client->dev, "dev1: 0x%x, dev2: 0x%x\n", val1, val2);

	if( (intr == INT_ATTACH) && device_type ==0x0){
		if(bcmpmu_check_vbus()){
			dev_info(&client->dev, "forced VBUS charger\n");
			val1 = DEV_T1_CHARGER_MASK;
			dev_info(&client->dev, "dev1: 0x%x, dev2: 0x%x\n", val1, val2);
		}
	}

	/* Attached */
	if (val1 || val2) {
		int ret;
		/* USB */
		if (val1 & DEV_USB || val2 & DEV_T2_USB_MASK) {
			dev_info(&client->dev, "usb connect\n");

			if (pdata->usb_cb)
				pdata->usb_cb(TSU6111_ATTACHED);
			if (usbsw->mansw) {
				ret = tsu6111_write_reg(client,
				TSU6111_REG_MANSW1, usbsw->mansw);

				if (ret < 0)
					dev_err(&client->dev,
						"%s: err %d\n", __func__, ret);
			}
		/* UART */
		} else if (val1 & DEV_T1_UART_MASK || val2 & DEV_T2_UART_MASK) {

			dev_info(&client->dev, "uart connect\n");
			if (pdata->uart_cb)
				pdata->uart_cb(TSU6111_ATTACHED);

			if (usbsw->mansw) {
				ret = tsu6111_write_reg(client,
					TSU6111_REG_MANSW1, SW_UART);

				if (ret < 0)
					dev_err(&client->dev,
						"%s: err %d\n", __func__, ret);
			}
		/* CHARGER */
		} else if (val1 & DEV_T1_CHARGER_MASK) {
			dev_info(&client->dev, "charger connect\n");
			if (pdata->charger_cb)
				pdata->charger_cb(TSU6111_ATTACHED);
		/* for SAMSUNG OTG */
		} else if (val1 & DEV_USB_OTG) {
			dev_info(&client->dev, "otg connect\n");
		/* JIG */
		} else if (val2 & DEV_T2_JIG_MASK) {
			dev_info(&client->dev, "jig connect\n");

			if (pdata->jig_cb)
				pdata->jig_cb(TSU6111_ATTACHED);
		/* Desk Dock */
		} else if( val2 & DEV_AV){
			dev_info(&client->dev, "dock connect\n");
			if(bcmpmu_check_vbus())
				if (pdata->charger_cb)
					pdata->charger_cb(TSU6111_ATTACHED);
			}


	/* Detached */
	} else {
		/* USB */
		if (usbsw->dev1 & DEV_USB ||
				usbsw->dev2 & DEV_T2_USB_MASK) {
			if (pdata->usb_cb)
				pdata->usb_cb(TSU6111_DETACHED);
		/* UART */
		} else if (usbsw->dev1 & DEV_T1_UART_MASK ||
				usbsw->dev2 & DEV_T2_UART_MASK) {

			tsu6111_sw_reset(usbsw);
			tsu6111_reg_init(usbsw);

			if (pdata->uart_cb)
				pdata->uart_cb(TSU6111_DETACHED);
		/* CHARGER */
		} else if (usbsw->dev1 & DEV_T1_CHARGER_MASK) {
			if (pdata->charger_cb)
				pdata->charger_cb(TSU6111_DETACHED);

			tsu6111_sw_reset(usbsw);
			tsu6111_reg_init(usbsw);
		/* for SAMSUNG OTG */
		} else if (usbsw->dev1 & DEV_USB_OTG) {
			tsu6111_write_reg(client,
						TSU6111_REG_CTRL, 0x1E);
		/* JIG */
		} else if (usbsw->dev2 & DEV_T2_JIG_MASK) {
			if (pdata->jig_cb)
				pdata->jig_cb(TSU6111_DETACHED);
		/* Desk Dock */
		} else if (usbsw->dev2 & DEV_AV) {
			if (pdata->charger_cb)
				pdata->charger_cb(TSU6111_DETACHED);

			tsu6111_sw_reset(usbsw);
			tsu6111_reg_init(usbsw);
		}
		else if( intr == INT_DETACH ) {
            if (pdata->charger_cb)
                pdata->charger_cb(TSU6111_DETACHED);

            tsu6111_sw_reset(usbsw);
            tsu6111_reg_init(usbsw);
        }
	}
	usbsw->dev1 = val1;
	usbsw->dev2 = val2;
}

static irqreturn_t tsu6111_irq_thread(int irq, void *data)
{
	struct tsu6111_usbsw *usbsw = data;

	/* device detection */
	mutex_lock(&usbsw->mutex);
	schedule_delayed_work(&usbsw->detect_work,msecs_to_jiffies(10));
	mutex_unlock(&usbsw->mutex);

	return IRQ_HANDLED;
}

static int tsu6111_irq_init(struct tsu6111_usbsw *usbsw)
{
	struct i2c_client *client = usbsw->client;
	int ret;

	if (client->irq) {

		ret = request_threaded_irq(client->irq, NULL,
			tsu6111_irq_thread, IRQF_TRIGGER_FALLING |IRQF_ONESHOT |IRQF_NO_SUSPEND,
			"tsu6111 micro USB", usbsw);
		if (ret) {
			dev_err(&client->dev, "failed to reqeust IRQ\n");
			return ret;
		}
	}

	return 0;
}

static void tsu6111_init_detect(struct work_struct *work)
{
	struct tsu6111_usbsw *usbsw = container_of(work,
			struct tsu6111_usbsw, init_work.work);
	int ret;

	dev_info(&usbsw->client->dev, "%s\n", __func__);

	ret = tsu6111_irq_init(usbsw);
	if (ret)
		dev_info(&usbsw->client->dev,
				"failed to enable  irq init %s\n", __func__);
}
static void tsu6111_detect_work(struct work_struct *work)
{
	struct tsu6111_usbsw *usbsw = container_of(work,
			struct tsu6111_usbsw, detect_work.work);
	struct i2c_client *client = usbsw->client;
	int intr;
	u8 int1,int2;

	dev_info(&usbsw->client->dev, "%s\n", __func__);

	/* read and clear interrupt status bits */
	tsu6111_read_reg(client, TSU6111_REG_INT1,&int1);
	tsu6111_read_reg(client, TSU6111_REG_INT2,&int2);
	intr = (int2<<8) | int1;

	dev_info(&usbsw->client->dev,"[TSU6111] %s: intr=0x%x, int1 = 0x%X, int2=0x%x \n",__func__,intr,int1, int2);

	if (intr < 0) {
		msleep(100);
		dev_err(&client->dev, "%s: err %d\n", __func__, intr);
		tsu6111_read_word_reg(client, TSU6111_REG_INT1,&intr);
		if (intr < 0)
			dev_err(&client->dev,
				"%s: err at read %d\n", __func__, intr);
		tsu6111_reg_init(usbsw);
		return;
	} else if (intr == 0) {
		/* interrupt was fired, but no status bits were set,
		so device was reset. In this case, the registers were
		reset to defaults so they need to be reinitialised. */
		tsu6111_reg_init(usbsw);
		pr_err("tsu6111_irq_thread+: no status\n");
	}

	mutex_lock(&usbsw->mutex);
	tsu6111_detect_dev(usbsw,int1);
	mutex_unlock(&usbsw->mutex);

}


static int tsu6111_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct tsu6111_usbsw *usbsw;
	int ret = 0;

	pr_info("tsu6111_probe\n");

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -EIO;

	usbsw = kzalloc(sizeof(struct tsu6111_usbsw), GFP_KERNEL);
	if (!usbsw) {
		dev_err(&client->dev, "failed to allocate driver data\n");
		kfree(usbsw);
		return -ENOMEM;
	}

	usbsw->client = client;
	usbsw->pdata = client->dev.platform_data;
	if (!usbsw->pdata)
		goto fail1;

	i2c_set_clientdata(client, usbsw);
	mutex_init(&usbsw->mutex);

	local_usbsw = usbsw;


	tsu6111_reg_init(usbsw);

	ret = sysfs_create_group(&client->dev.kobj, &tsu6111_group);
	if (ret) {
		dev_err(&client->dev,
				"failed to create tsu6111 attribute group\n");
		goto fail2;
	}

	/* initial cable detection */
	INIT_DELAYED_WORK(&usbsw->init_work, tsu6111_init_detect);
	INIT_DELAYED_WORK(&usbsw->detect_work, tsu6111_detect_work);
	schedule_delayed_work(&usbsw->init_work, msecs_to_jiffies(5));
	schedule_delayed_work(&usbsw->detect_work, msecs_to_jiffies(10));

	pr_info("tsu6111_probe end.\n");
	return 0;

fail2:
	if (client->irq)
		free_irq(client->irq, usbsw);
fail1:
	mutex_destroy(&usbsw->mutex);
	i2c_set_clientdata(client, NULL);
	kfree(usbsw);
	pr_info("tsu6111_probe failed .\n");
	return ret;
}

static int tsu6111_remove(struct i2c_client *client)
{
	struct tsu6111_usbsw *usbsw = i2c_get_clientdata(client);

	cancel_delayed_work(&usbsw->detect_work);
	cancel_delayed_work(&usbsw->init_work);
	if (client->irq) {
		free_irq(client->irq, usbsw);
	}
	mutex_destroy(&usbsw->mutex);
	i2c_set_clientdata(client, NULL);

	sysfs_remove_group(&client->dev.kobj, &tsu6111_group);
	kfree(usbsw);
	return 0;
}

static int tsu6111_resume(struct i2c_client *client)
{
	return 0;
}


static const struct i2c_device_id tsu6111_id[] = {
	{"tsu6111", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, tsu6111_id);

static struct i2c_driver tsu6111_i2c_driver = {
	.driver = {
		.name = "tsu6111",
	},
	.probe = tsu6111_probe,
	.remove = tsu6111_remove,
	.resume = tsu6111_resume,
	.id_table = tsu6111_id,
};

static int __init tsu6111_init(void)
{
	pr_info("tsu6111_init:tsu6111\n");
	return i2c_add_driver(&tsu6111_i2c_driver);
}
module_init(tsu6111_init);

static void __exit tsu6111_exit(void)
{
	i2c_del_driver(&tsu6111_i2c_driver);
}
module_exit(tsu6111_exit);

MODULE_AUTHOR("Minkyu Kang <mk7.kang@samsung.com>");
MODULE_DESCRIPTION("TSU6111 USB Switch driver");
MODULE_LICENSE("GPL");
