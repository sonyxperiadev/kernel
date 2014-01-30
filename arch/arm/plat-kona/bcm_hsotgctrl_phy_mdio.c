/*****************************************************************************
*  Copyright 2001 - 2012 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/pm_runtime.h>
#include <linux/io.h>
#include <mach/memory.h>
#include <mach/rdb/brcm_rdb_hsotg_ctrl.h>
#include <mach/rdb/brcm_rdb_khub_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#include <plat/pi_mgr.h>
#include <plat/clock.h>
#include <linux/usb/bcm_hsotgctrl_phy_mdio.h>
#include <linux/usb/bcm_hsotgctrl.h>
#include <linux/of.h>
#include <linux/of_platform.h>


#define USB_PHY_MDIO_ID 9
#define USB_PHY_MDIO0 0
#define MDIO_ACCESS_KEY 0x00A5A501
#define PHY_PM_DELAY_IN_MS 1

#define TX_CURRENT_MASK		0x07E0
#define TX_CURRENT_OFFSET	0x05
#define SQUELCH_MASK		0x01F0
#define SQUELCH_OFFSET		0x04

struct bcm_hsotgctrl_drv_data {
	struct device *dev;
	struct clk *otg_clk;
	struct clk *mdio_master_clk;
	void *hsotg_ctrl_base;
	void *chipregs_base;
	struct workqueue_struct *bcm_hsotgctrl_work_queue;
	struct delayed_work wakeup_work;
	int hsotgctrl_irq;
	bool irq_enabled;
	bool allow_suspend;
};

static struct bcm_hsotgctrl_drv_data *local_usb_mdio_handle;

static int TxCurrent_index;
static int RxSensitivity_index;
static int TxCurrent_table_size;
static int RxSensitivity_table_size;
static bool CTableSize;
static bool STableSize;

static int USB_MDIO[6] = {
	0x0018,			/* MDIO Register 0 initial value 0x0000 */
	0x0080,			/* MDIO Register 1 initial value 0x0000 */
	0x0000,			/* MDIO Register 2 initial value 0x0000 */
#if defined(CONFIG_MACH_HAWAII_GARNET)
	0x0400,			/* MDIO Register 3 initial value 0x0400 */
#else
	0x0000,			/* MDIO Register 3 initial value 0x0000 */
#endif
	0x0020,			/* MDIO Register 4 initial value 0x0020 */
	0x0000			/* MDIO Register 5 initial value 0x0000 */
};

struct Rx_Sensitivity_t {
	 int value;
	 char *value_string;
	 char *Rx_sensitivity_squelch;
};

static struct Rx_Sensitivity_t USB_Rx_Sensitivity[] = {
		{0,  "00", "102 mV / 67 mV"},		/* 0x00000 --0 */
		{1,  "01", "102 mV / 75 mV"},		/* 0x00001 --1 */
		{2,  "02", "102 mV / 47 mV"},		/* 0x00010 --2 */
		{3,  "03", "102 mV / 55 mV"},		/* 0x00011 --3 */
		{16, "10", "79 mV / 67 mV"},		/* 0x10000 --16 */
		{17, "11", "79 mV / 75 mV"},		/* 0x10001 --17 */
		{18, "12", "79 mV / 47 mV"},		/* 0x10010 --18 */
		{19, "13", "79 mV / 55 mV"},		/* 0x10011 --19 */
		{20, "14", "94 mV / 67 mV"},		/* 0x10100 --20 */
		{21, "15", "94 mV / 75 mV"},		/* 0x10101 --21 */
		{22, "16", "94 mV / 47 mV"},		/* 0x10110 --22 */
		{23, "17", "94 mV / 55 mV"},		/* 0x10111 --23 */
		{24, "18", "110 mV / 67 mV"},		/* 0x11000 --24 */
		{25, "19", "110 mV / 75 mV"},		/* 0x11001 --25 */
		{26, "1A", "110 mV / 47 mV"},		/* 0x11010 --26 */
		{27, "1B", "110 mV / 55 mV"},		/* 0x11011 --27 */
		{28, "1C", "118 mV / 67 mV"},		/* 0x11100 --28 */
		{29, "1D", "118 mV / 75 mV"},		/* 0x11101 --29 */
		{30, "1E", "118 mV / 47 mV"},		/* 0x11110 --30 */
		{31, "1F", "118 mV / 55 mV"},		/* 0x11111 --31 */
};


struct Tx_Current {
	 int value;
	 char *value_string;
	 char *driving_current;
};

static struct Tx_Current USB_Tx_Current[] = {
		{0, "0x00", "1.000 mA"},		/* 0x000000--0*/
		{1, "0x01", "50% of 1 mA"},		/* 0x000001--1 */
		{3, "0x03", "25% of 1 mA"},		/* 0x000011--3 */
		{7, "0x07", "12.5% of 1 mA"},	/* 0x000111--7 */
		{15, "0x0f", "no current"},		/* 0x001111--15 */
		{16, "0x10", "1.125 mA"},		/* 0x010000 --16*/
		{32, "0x20", "1.0625 mA"},		/* 0x100000 --32*/
		{48, "0x30", "1.1875 mA"}		/* 0x110000 --48*/
};



int bcm_hsotgctrl_phy_mdio_read(int mdio_num)
{
	int val;
	struct bcm_hsotgctrl_drv_data *bcm_hsotgctrl_handle
		= local_usb_mdio_handle;

	if (NULL == bcm_hsotgctrl_handle)
		return -ENODEV;

	if ((!bcm_hsotgctrl_handle->mdio_master_clk) ||
		  (!bcm_hsotgctrl_handle->dev))
		return -EIO;

	/* Enable mdio clk */
	val = clk_enable(bcm_hsotgctrl_handle->mdio_master_clk);
	if (val)
		return val;

	/* Program necessary values */
	val = (CHIPREG_MDIO_CTRL_ADDR_WRDATA_MDIO_SM_SEL_MASK
			| (USB_PHY_MDIO_ID <<
			CHIPREG_MDIO_CTRL_ADDR_WRDATA_MDIO_ID_SHIFT) |
			(USB_PHY_MDIO0 <<
			CHIPREG_MDIO_CTRL_ADDR_WRDATA_MDIO_REG_ADDR_SHIFT));
	writel(val, bcm_hsotgctrl_handle->chipregs_base
		+ CHIPREG_MDIO_CTRL_ADDR_WRDATA_OFFSET);
	mdelay(PHY_PM_DELAY_IN_MS);

	/* ******* MDIO REG N:: -->
	* Read to MDIO Num */
	val = (CHIPREG_MDIO_CTRL_ADDR_WRDATA_MDIO_SM_SEL_MASK |
			(USB_PHY_MDIO_ID <<
			CHIPREG_MDIO_CTRL_ADDR_WRDATA_MDIO_ID_SHIFT) |
			(mdio_num <<
			CHIPREG_MDIO_CTRL_ADDR_WRDATA_MDIO_REG_ADDR_SHIFT) |
			CHIPREG_MDIO_CTRL_ADDR_WRDATA_MDIO_READ_START_MASK);
	writel(val, bcm_hsotgctrl_handle->chipregs_base
		+ CHIPREG_MDIO_CTRL_ADDR_WRDATA_OFFSET);
	mdelay(PHY_PM_DELAY_IN_MS);
	val = 0x00;

	val = readl(bcm_hsotgctrl_handle->chipregs_base +
		CHIPREG_MDIO_RDDATA_OFFSET);

	/* Disable mdio clk */
	clk_disable(bcm_hsotgctrl_handle->mdio_master_clk);

	return val;
}

int bcm_hsotgctrl_phy_mdio_write(int mdio_num, int value)
{
	int val;
	struct bcm_hsotgctrl_drv_data *bcm_hsotgctrl_handle
		= local_usb_mdio_handle;

	if (NULL == bcm_hsotgctrl_handle)
		return -ENODEV;

	if ((!bcm_hsotgctrl_handle->mdio_master_clk) ||
		  (!bcm_hsotgctrl_handle->dev))
		return -EIO;

	/* Enable mdio clk */
	val = clk_enable(bcm_hsotgctrl_handle->mdio_master_clk);
	if (val)
		return val;

	/* Program necessary values */
	val = (CHIPREG_MDIO_CTRL_ADDR_WRDATA_MDIO_SM_SEL_MASK
			| (USB_PHY_MDIO_ID <<
			CHIPREG_MDIO_CTRL_ADDR_WRDATA_MDIO_ID_SHIFT) |
			(USB_PHY_MDIO0 <<
			CHIPREG_MDIO_CTRL_ADDR_WRDATA_MDIO_REG_ADDR_SHIFT));
	writel(val, bcm_hsotgctrl_handle->chipregs_base
		+ CHIPREG_MDIO_CTRL_ADDR_WRDATA_OFFSET);
	mdelay(PHY_PM_DELAY_IN_MS);

	/* ******* MDIO REG 3:: -->
	* Write to MDIO register mdio_num with value
	*/
	val = (CHIPREG_MDIO_CTRL_ADDR_WRDATA_MDIO_SM_SEL_MASK |
			(USB_PHY_MDIO_ID <<
			CHIPREG_MDIO_CTRL_ADDR_WRDATA_MDIO_ID_SHIFT) |
			(mdio_num <<
			CHIPREG_MDIO_CTRL_ADDR_WRDATA_MDIO_REG_ADDR_SHIFT) |
			CHIPREG_MDIO_CTRL_ADDR_WRDATA_MDIO_WRITE_START_MASK |
			value);
	writel(val, bcm_hsotgctrl_handle->chipregs_base
		+ CHIPREG_MDIO_CTRL_ADDR_WRDATA_OFFSET);
	mdelay(PHY_PM_DELAY_IN_MS);

	val = (CHIPREG_MDIO_CTRL_ADDR_WRDATA_MDIO_SM_SEL_MASK |
			(USB_PHY_MDIO_ID <<
			CHIPREG_MDIO_CTRL_ADDR_WRDATA_MDIO_ID_SHIFT) |
			(mdio_num <<
			CHIPREG_MDIO_CTRL_ADDR_WRDATA_MDIO_REG_ADDR_SHIFT) |
			CHIPREG_MDIO_CTRL_ADDR_WRDATA_MDIO_READ_START_MASK);
	writel(val, bcm_hsotgctrl_handle->chipregs_base
		+ CHIPREG_MDIO_CTRL_ADDR_WRDATA_OFFSET);
	mdelay(PHY_PM_DELAY_IN_MS);

	/* Disable mdio clk */
	clk_disable(bcm_hsotgctrl_handle->mdio_master_clk);

	return 0;
}


int bcm_hsotgctrl_phy_mdio_initialization(void)
{
	int i;

	for (i = 0; i <= 5; i++)
		bcm_hsotgctrl_phy_mdio_write(i, USB_MDIO[i]);

	return 0;
}
EXPORT_SYMBOL_GPL(bcm_hsotgctrl_phy_mdio_initialization);


void bcm_hsotgctrl_mdio_register_write(int reg_num, int value)
{

	USB_MDIO[reg_num] = value;
	bcm_hsotgctrl_phy_mdio_write(reg_num, USB_MDIO[reg_num]);
	pr_info("USB PHY MDIO Register %d is now set to %x\n\n", reg_num,
	bcm_hsotgctrl_phy_mdio_read(reg_num));

}

static ssize_t
bcm_hsotg_mdio0_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	int val = bcm_hsotgctrl_phy_mdio_read(0);

	pr_info("USB PHY MDIO Register 0 ==%x\n", val);

	return scnprintf(buf, PAGE_SIZE, "USB PHY MDIO Register 0 ==%x\n", val);
}

static ssize_t
bcm_hsotg_mdio0_set(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	unsigned int val;
	int err;

	err = kstrtouint(buf, 0, &val);

	bcm_hsotgctrl_mdio_register_write(0, (int)val);

	pr_info("Set USB PHY MDIO Register 0 to %x\n", val);

	return count;
}


static ssize_t
bcm_hsotg_mdio1_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	int val = bcm_hsotgctrl_phy_mdio_read(1);

	pr_info("USB PHY MDIO Register 1 ==%x\n", val);

	return scnprintf(buf, PAGE_SIZE, "USB PHY MDIO Register 1 ==%x\n", val);
}

static ssize_t
bcm_hsotg_mdio1_set(struct device *dev, struct device_attribute *attr,
				  const char *buf, size_t count)
{
	unsigned int val;
	int err;

	err = kstrtouint(buf, 0, &val);

	bcm_hsotgctrl_mdio_register_write(1, (int)val);

	pr_info("Set USB PHY MDIO Register 1 to %x\n", val);

	return count;
}

static ssize_t
bcm_hsotg_mdio2_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	int val = bcm_hsotgctrl_phy_mdio_read(2);

	pr_info("USB PHY MDIO Register 2 ==%x\n", val);

	return scnprintf(buf, PAGE_SIZE, "USB PHY MDIO Register 2 ==%x\n", val);
}

static ssize_t
bcm_hsotg_mdio2_set(struct device *dev, struct device_attribute *attr,
				 const char *buf, size_t count)
{
	unsigned int val;
	int err;

	err = kstrtouint(buf, 0, &val);

	bcm_hsotgctrl_mdio_register_write(2, (int)val);

	pr_info("Set USB PHY MDIO Register 2 to %x\n", val);

	return count;
}

static ssize_t
bcm_hsotg_mdio3_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	int val = bcm_hsotgctrl_phy_mdio_read(3);

	pr_info("USB PHY MDIO Register 3 ==%x\n", val);

	return scnprintf(buf, PAGE_SIZE, "USB PHY MDIO Register 3 ==%x\n", val);
}

static ssize_t
bcm_hsotg_mdio3_set(struct device *dev, struct device_attribute *attr,
				 const char *buf, size_t count)
{
	unsigned int val;
	int err;

	err = kstrtouint(buf, 0, &val);

	bcm_hsotgctrl_mdio_register_write(3, (int)val);

	pr_info("Set USB PHY MDIO Register 3 to %x\n", val);

	return count;
}

static ssize_t
bcm_hsotg_mdio4_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	int val = bcm_hsotgctrl_phy_mdio_read(4);

	pr_info("USB PHY MDIO Register 45 ==%x\n", val);

	return scnprintf(buf, PAGE_SIZE, "USB PHY MDIO Register 4 ==%x\n", val);
}

static ssize_t
bcm_hsotg_mdio4_set(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	unsigned int val;
	int err;

	err = kstrtouint(buf, 0, &val);

	bcm_hsotgctrl_mdio_register_write(4, (int)val);

	pr_info("Set USB PHY MDIO Register 4 to %x\n", val);

	return count;
}

static ssize_t
bcm_hsotg_mdio5_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	int val = bcm_hsotgctrl_phy_mdio_read(5);

	pr_info("USB PHY MDIO Register 5 ==%x\n", val);

	return scnprintf(buf, PAGE_SIZE, "USB PHY MDIO Register 5 ==%x\n", val);
}

static ssize_t
bcm_hsotg_mdio5_set(struct device *dev, struct device_attribute *attr,
				 const char *buf, size_t count)
{
	unsigned int val;
	int err;

	err = kstrtouint(buf, 0, &val);

	bcm_hsotgctrl_mdio_register_write(5, (int)val);

	pr_info("Set USB PHY MDIO Register 5 to %x\n", val);

	return count;
}

static ssize_t
bcm_hsotg_TxCurrent_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	int val = (bcm_hsotgctrl_phy_mdio_read(3)
		&TX_CURRENT_MASK)>>TX_CURRENT_OFFSET;
	int i;

	for (i = 0; i < TxCurrent_table_size; i++) {
		if ((USB_Tx_Current[i].value) == val) {

			pr_info("The USB Tx Current is ==%x The TX Driving_current is %s\n\n",
				val, USB_Tx_Current[i].driving_current);
			return sprintf(buf,
				"The USB Tx Current is %x --------%s\n",
				val, USB_Tx_Current[i].driving_current);
		}
	}
	pr_info("The USB Tx Current is ==%x\n", val);
	pr_info("The current is undefined or invalid\n");

	return sprintf(buf,
		"The USB Tx Current is invalid, the value is ==%x\n", val);

}

static ssize_t
bcm_hsotg_TxCurrent_Table_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int i;

	pr_info("USB TX Current settings and Driving Current\n");
	pr_info("============================================\n");

	for (i = 0; i < TxCurrent_table_size; i++)
		pr_info("%s -------- %s\n", USB_Tx_Current[i].value_string,
			USB_Tx_Current[i].driving_current);
	pr_info("============================================\n");

	return 0;
}

static ssize_t
bcm_hsotg_Current_Table(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	char tempbuf[80] = {0};

	if (CTableSize) {
		CTableSize = false;
		return sprintf(buf, "%d\n", TxCurrent_table_size);
	} else {

			sprintf(tempbuf, "%s -------- %s\n",
			USB_Tx_Current[TxCurrent_index].value_string,
			USB_Tx_Current[TxCurrent_index].driving_current);

			TxCurrent_index++;
			if (TxCurrent_index >= TxCurrent_table_size) {
				TxCurrent_index = 0;
				CTableSize = true;
			}
			return sprintf(buf, "%s\n", (char *)tempbuf);
		}
}


static ssize_t
bcm_hsotg_TxCurrent_set(struct device *dev, struct device_attribute *attr,
				 const char *buf, size_t count)
{
	int TxCurrent;
	int mdio3;
	unsigned int val;
	int i, err;

	err = kstrtouint(buf, 0, &val);

	if (val > 0x3F)
		goto error_Set_Tx_Current;

	TxCurrent = (int)val;

	mdio3 = bcm_hsotgctrl_phy_mdio_read(3);
	mdio3 &= ~(TX_CURRENT_MASK);
	mdio3 |= TxCurrent<<TX_CURRENT_OFFSET;

	bcm_hsotgctrl_phy_mdio_write(3, USB_MDIO[3]);
	for (i = 0; i < TxCurrent_table_size; i++) {
		if ((USB_Tx_Current[i].value) == TxCurrent) {
			pr_info("The USB TxCurrent now is ==%x The driving_current is %s\n\n",
				TxCurrent, USB_Tx_Current[i].driving_current);
			pr_info("The USB Tx Current setting is accepted\n");
			USB_MDIO[3] = mdio3;
			return count;
		}
	}
error_Set_Tx_Current:
	pr_info("The USB_Tx_Current is ==%x\n", val);
	pr_info("The current is undefined or invalid\n");
	pr_info("The USB Tx Current setting is rejected!!!\n");

	return count;

}

static ssize_t
bcm_hsotg_Squelch_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	int val = (bcm_hsotgctrl_phy_mdio_read(4)
		&SQUELCH_MASK)>>SQUELCH_OFFSET;
	int i;

	for (i = 0; i < RxSensitivity_table_size; i++) {
		if ((USB_Rx_Sensitivity[i].value) == val) {
			pr_info("USB USB_Rx_Sensitivity setting is\n ");
			pr_info("%s -------- %s\n",
				USB_Rx_Sensitivity[i].value_string,
				USB_Rx_Sensitivity[i].Rx_sensitivity_squelch);
			return sprintf(buf, "         %s -------- %s\n",
				USB_Rx_Sensitivity[i].value_string,
				USB_Rx_Sensitivity[i].Rx_sensitivity_squelch);
		}
	}
	pr_info("The USB USB_Rx_Sensitivity is ==%x\n", val);
	pr_info("The current is undefined or invalid\n");

	return sprintf(buf,
		"The USB USB_Rx_Sensitivity is invalid, the value is ==%x\n",
		val);

}

static ssize_t
bcm_hsotg_Squelch_Table_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	int i;

	pr_info("USB RxSensitivity settings and Voltage\n");
	pr_info("============================================\n");
	for (i = 0; i < RxSensitivity_table_size; i++)
		pr_info("%s -------- %s\n", USB_Rx_Sensitivity[i].value_string,
			USB_Rx_Sensitivity[i].Rx_sensitivity_squelch);
	pr_info("============================================\n");

	return 0;
}

static ssize_t
bcm_hsotg_Sensitivity_Table(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	char tempbuf[80] = {0};

	if (STableSize) {
		STableSize = false;
		return sprintf(buf, "%d\n", RxSensitivity_table_size);
	} else {
			sprintf(tempbuf, "%s -------- %s\n",
			USB_Rx_Sensitivity[RxSensitivity_index].value_string,
		USB_Rx_Sensitivity[RxSensitivity_index].Rx_sensitivity_squelch);

			RxSensitivity_index++;
			if (RxSensitivity_index >=
				RxSensitivity_table_size) {
				RxSensitivity_index = 0;
				STableSize = true;
			}
			return sprintf(buf, "%s\n", (char *)tempbuf);
		}
}

static ssize_t
bcm_hsotg_Squelch_set(struct device *dev, struct device_attribute *attr,
				 const char *buf, size_t count)
{
	unsigned int squelch;
	int mdio4;
	unsigned int val;
	int err;
	int i;

	err = kstrtouint(buf, 0, &val);

	if (val > 0x1F)
		goto error_Set_Squelch;

	squelch = (int) val;

	mdio4 = bcm_hsotgctrl_phy_mdio_read(4);
	mdio4 &= ~(SQUELCH_MASK);
	mdio4 |= squelch<<SQUELCH_OFFSET;

	USB_MDIO[4] = mdio4;
	bcm_hsotgctrl_phy_mdio_write(4, USB_MDIO[4]);

	for (i = 0; i < RxSensitivity_table_size; i++) {
		if ((USB_Rx_Sensitivity[i].value) == val) {
			pr_info("The USB_Rx_Sensitivity now is ==%x\n", val);
			pr_info("The USB_Rx_Sensitivity and Squelch  is %s\n",
				USB_Rx_Sensitivity[i].Rx_sensitivity_squelch);
			pr_info("The USB Tx Current setting is accepted\n");
			USB_MDIO[4] = mdio4;
			return count;
		}
	}

error_Set_Squelch:
	pr_info("The USB_Squelch is ==%x\n", val);
	pr_info("The squelch is undefined or invalid\n");
	pr_info("The USB squelch setting is rejected!!!\n");

	return count;


}

static DEVICE_ATTR(USB_MDIO0, S_IRUGO | S_IWUSR, bcm_hsotg_mdio0_show,
	bcm_hsotg_mdio0_set);
static DEVICE_ATTR(USB_MDIO1, S_IRUGO | S_IWUSR, bcm_hsotg_mdio1_show,
	bcm_hsotg_mdio1_set);
static DEVICE_ATTR(USB_MDIO2, S_IRUGO | S_IWUSR, bcm_hsotg_mdio2_show,
	bcm_hsotg_mdio2_set);
static DEVICE_ATTR(USB_MDIO3, S_IRUGO | S_IWUSR, bcm_hsotg_mdio3_show,
	bcm_hsotg_mdio3_set);
static DEVICE_ATTR(USB_MDIO4, S_IRUGO | S_IWUSR, bcm_hsotg_mdio4_show,
	bcm_hsotg_mdio4_set);
static DEVICE_ATTR(USB_MDIO5, S_IRUGO | S_IWUSR, bcm_hsotg_mdio5_show,
	bcm_hsotg_mdio5_set);
static DEVICE_ATTR(TxCurrent, S_IRUGO | S_IWUSR, bcm_hsotg_TxCurrent_show,
		bcm_hsotg_TxCurrent_set);
static DEVICE_ATTR(Squelch, S_IRUGO | S_IWUSR, bcm_hsotg_Squelch_show,
		bcm_hsotg_Squelch_set);
static DEVICE_ATTR(TxCurrent_table, S_IRUGO | S_IWUSR,
	bcm_hsotg_TxCurrent_Table_show, NULL);
static DEVICE_ATTR(Squelch_table, S_IRUGO | S_IWUSR,
	bcm_hsotg_Squelch_Table_show, NULL);
static DEVICE_ATTR(CurrentTable, S_IRUGO | S_IWUSR,
	bcm_hsotg_Current_Table, NULL);
static DEVICE_ATTR(SensitivityTable, S_IRUGO | S_IWUSR,
	bcm_hsotg_Sensitivity_Table, NULL);

static struct attribute *bcm_hsotgctrl_phy_mdio_attrs[] = {
	&dev_attr_USB_MDIO0.attr,
	&dev_attr_USB_MDIO1.attr,
	&dev_attr_USB_MDIO2.attr,
	&dev_attr_USB_MDIO3.attr,
	&dev_attr_USB_MDIO4.attr,
	&dev_attr_USB_MDIO5.attr,
	&dev_attr_TxCurrent.attr,
	&dev_attr_Squelch.attr,
	&dev_attr_TxCurrent_table.attr,
	&dev_attr_Squelch_table.attr,
	&dev_attr_CurrentTable.attr,
	&dev_attr_SensitivityTable.attr,
	NULL
};

static const struct attribute_group bcm_hsotgctrl_phy_mdio_attrs_group = {
	.attrs = bcm_hsotgctrl_phy_mdio_attrs,
};

static int bcm_hsotgctrl_phy_mdio_probe(struct platform_device *pdev)
{
	int error = 0;
	struct bcm_hsotgctrl_drv_data *hsotgctrl_drvdata;
	struct bcm_hsotgctrl_platform_data *plat_data = NULL;

	if (pdev->dev.platform_data)
		plat_data = pdev->dev.platform_data;
	else if (pdev->dev.of_node) {
		int val;
		const char *rstring;
		struct resource *resource;

		plat_data = kzalloc(sizeof(*plat_data), GFP_KERNEL);
		if (!plat_data) {
			dev_err(&pdev->dev,
				"%s: memory allocation failed.", __func__);
			error = -ENOMEM;
			goto err_ret;
		}
		resource = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		if (resource && resource->start) {
			plat_data->hsotgctrl_virtual_mem_base =
				HW_IO_PHYS_TO_VIRT(resource->start);
		} else {
			pr_info("Invalid hsotgctrl_virtual_mem_base from DT\n");
			error = -ENXIO;
			goto err_read;
		}

		error = of_property_read_u32(pdev->dev.of_node,
					     "chipreg-virtual-mem-base", &val);
		if (error) {
			dev_err(&pdev->dev,
				"chipreg-virtual-mem-base read failed %d\n",
				error);
			goto err_read;
		}
		plat_data->chipreg_virtual_mem_base = HW_IO_PHYS_TO_VIRT(val);

		resource = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
		if (resource && resource->start) {
			plat_data->irq = resource->start;
		} else {
			pr_info("Invalid irq from DT\n");
			error = -ENXIO;
			goto err_read;
		}

		error = of_property_read_string(pdev->dev.of_node,
						"usb-ahb-clk-name", &rstring);
		if (error) {
			dev_err(&pdev->dev,
				"usb-ahb-clk-name read failed %d\n", error);
			goto err_read;
		}
		plat_data->usb_ahb_clk_name = (char *)rstring;

		error = of_property_read_string(pdev->dev.of_node,
						"mdio-mstr-clk-name", &rstring);
		if (error) {
			dev_err(&pdev->dev,
				"mdio-mstr-clk-name read failed %d\n", error);
			goto err_read;
		}
		plat_data->mdio_mstr_clk_name = (char *)rstring;
	}

	if (plat_data == NULL) {
		dev_err(&pdev->dev, "platform_data failed\n");
		return -ENODEV;
	}

	hsotgctrl_drvdata = kzalloc(sizeof(*hsotgctrl_drvdata), GFP_KERNEL);
	if (!hsotgctrl_drvdata) {
		dev_warn(&pdev->dev, "Memory allocation failed\n");
		return -ENOMEM;
	}

	local_usb_mdio_handle = hsotgctrl_drvdata;

	hsotgctrl_drvdata->hsotg_ctrl_base =
		(void *)plat_data->hsotgctrl_virtual_mem_base;
	if (!hsotgctrl_drvdata->hsotg_ctrl_base) {
		dev_warn(&pdev->dev, "No vaddr for HSOTGCTRL!\n");
		kfree(hsotgctrl_drvdata);
		return -ENOMEM;
	}

	hsotgctrl_drvdata->chipregs_base =
		(void *)plat_data->chipreg_virtual_mem_base;
	if (!hsotgctrl_drvdata->chipregs_base) {
		dev_warn(&pdev->dev, "No vaddr for CHIPREG!\n");
		kfree(hsotgctrl_drvdata);
		return -ENOMEM;
	}

	hsotgctrl_drvdata->dev = &pdev->dev;

	hsotgctrl_drvdata->mdio_master_clk = clk_get(NULL,
		plat_data->mdio_mstr_clk_name);

	if (IS_ERR(hsotgctrl_drvdata->mdio_master_clk)) {
		error = PTR_ERR(hsotgctrl_drvdata->mdio_master_clk);
		dev_warn(&pdev->dev, "MDIO Mst clk alloc failed\n");
		kfree(hsotgctrl_drvdata);
		return error;
	}

	platform_set_drvdata(pdev, hsotgctrl_drvdata);

	error = sysfs_create_group(&pdev->dev.kobj,
		&bcm_hsotgctrl_phy_mdio_attrs_group);

	if (error) {
		dev_warn(&pdev->dev, "Failed to create HOST file\n");
		goto Error_bcm_hsotgctrl_phy_mdio_probe;
	}

	TxCurrent_index = 0;
	RxSensitivity_index = 0;
	TxCurrent_table_size = ARRAY_SIZE(USB_Tx_Current);
	RxSensitivity_table_size = ARRAY_SIZE(USB_Rx_Sensitivity);

	CTableSize = true;
	STableSize = true;
	return 0;

Error_bcm_hsotgctrl_phy_mdio_probe:
	clk_put(hsotgctrl_drvdata->mdio_master_clk);
	kfree(hsotgctrl_drvdata);
err_read:
	if (pdev->dev.of_node)
		kfree(plat_data);
err_ret:
	pr_err("%s probe failed\n", __func__);
	return error;
}

static int bcm_hsotgctrl_phy_mdio_remove(struct platform_device *pdev)
{
	struct bcm_hsotgctrl_drv_data *hsotgctrl_drvdata
		= platform_get_drvdata(pdev);

	sysfs_remove_group(&pdev->dev.kobj,
		&bcm_hsotgctrl_phy_mdio_attrs_group);

	pm_runtime_disable(&pdev->dev);
	clk_put(hsotgctrl_drvdata->mdio_master_clk);
	local_usb_mdio_handle = NULL;
	kfree(hsotgctrl_drvdata);

	return 0;
}

static const struct of_device_id usb_phy_of_match[] = {
	{.compatible = "bcm,usb-phy",},
	{},
}
MODULE_DEVICE_TABLE(of, usb_phy_of_match);

static struct platform_driver bcm_hsotgctrl_phy_mdio_driver = {
	.driver = {
		   .name = "bcm_hsotgctrl_phy_mdio",
		   .owner = THIS_MODULE,
		.of_match_table = usb_phy_of_match,
	},
	.probe = bcm_hsotgctrl_phy_mdio_probe,
	.remove = bcm_hsotgctrl_phy_mdio_remove,
};

static int __init bcm_hsotgctrl_usb_phy_mdio_init(void)
{
	pr_info("Broadcom USB HSOTGCTRL USB Phy MDIO Driver\n");

	return platform_driver_register(&bcm_hsotgctrl_phy_mdio_driver);
}
postcore_initcall(bcm_hsotgctrl_usb_phy_mdio_init);

static void __exit bcm_hsotgctrl_phy_mdio_exit(void)
{
	platform_driver_unregister(&bcm_hsotgctrl_phy_mdio_driver);
}
module_exit(bcm_hsotgctrl_phy_mdio_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("USB HSOTGCTRL PHY MDIO driver");
MODULE_LICENSE("GPL");
