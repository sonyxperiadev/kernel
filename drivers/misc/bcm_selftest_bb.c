/************************************************************************************************/
/*      											*/
/*  Copyright 2011  Broadcom Corporation							*/
/*      											*/
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU	*/
/*     General Public License version 2 (the GPL), available at 				*/
/*      											*/
/*          http://www.broadcom.com/licenses/GPLv2.php  					*/
/*      											*/
/*     with the following added to such license:						*/
/*      											*/
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.      	*/
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.   			*/
/*      											*/
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,	*/
/*     without Broadcom's express prior written consent.					*/
/*      											*/
/************************************************************************************************/

/* Tests supported in this file */
#define GPS_TEST_SUPPORTED      /* GPS GPIO control */  /* Implemented in LMP */
/* #define USB_ST_SUPPORTED */
/* #define ADC_ST_SUPPORTED */

/* Generel includes */
#include <linux/string.h>
#include <stdbool.h>
#include "linux/kernel.h"
#include "linux/delay.h"
#include "linux/init.h"
#include "linux/device.h"
#include "linux/fs.h"
#include "linux/proc_fs.h"
#include "linux/types.h"
#include <asm/uaccess.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <mach/hardware.h>

/* Generel Broadcom includes */

#include "linux/broadcom/bcm_selftest_bb.h"
#include "mach/bcm_selftest_bb_low.h"
/*#include "chipset_iomap.h"*/
/*#define ST_DBG(text,...) pr_debug (text"\n", ## __VA_ARGS__)*/
/*#define ST_MDBG(text,...) pr_debug (text"\n", ## __VA_ARGS__)*/
#define ST_DBG(text, ...) printk(KERN_INFO text"\n", ## __VA_ARGS__)
#define ST_MDBG(text, ...) printk(KERN_INFO text"\n", ## __VA_ARGS__)

#include <mach/io_map.h>

/* RDB access */
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_audioh.h>
#include <mach/rdb/brcm_rdb_khub_clk_mgr_reg.h> /* For DigiMic test */
#include <mach/rdb/brcm_rdb_sclkcal.h>  	/* For Sleep clock test */
#include <mach/rdb/brcm_rdb_bmdm_clk_mgr_reg.h> /* For Sleep clock test */
#define UNDER_LINUX
#include <mach/rdb/brcm_rdb_util.h>


/* Pinmux */
#include "mach/chip_pinmux.h"
#include "mach/pinmux.h"

/* PMU */
#include "linux/mfd/bcm590xx/core.h"
#include "linux/mfd/bcm590xx/bcm59055_A0.h"

/* GPIO */
#include "linux/gpio.h"
#include "linux/interrupt.h"


#ifdef GPS_TEST_SUPPORTED
/* GPS IO test variables */
static bool GPS_PABLANK_Setup_as_GPIO = false;
static struct pin_config StoredValue_GPS_PABLANK;
static bool GPS_TMARK_Setup_as_GPIO = false;
static struct pin_config StoredValue_GPS_TMARK;
#endif

/* PMU Selftest Driver */
static struct bcm590xx *bcm590xx_dev;


/**********************************************************/
/** Stuff that should be defined in header files - Begin **/
/**********************************************************/
/* GPIO defines */
#define GPIO_GPS_TMARK       97
#define GPIO_GPS_PABLANK     98

/* Missing Functions - where to find ?*/
#define assert(x)
/**********************************************************/
/** Stuff that should be defined in header files - End   **/
/**********************************************************/

/******************************************************/
/** Stuff that should be fined in header files - End **/
/******************************************************/

#ifdef GPS_TEST_SUPPORTED
/*---------------------------------------------------------------------------*/
/*! \brief ST_SELFTEST_control_gps_io.
 *  Self Test for gps control pins
 *
 *  \param name 	   - The logical name of the GenIO pin
 *  \param direction       - Defines if read or write operation shall be performed
 *  \param state	   - Pointer to data type that contains the pin state
 *
 *  \return     	   - The outcome of the self test procedure
 */
void std_selftest_control_gps_io(struct SelftestDevData_t *dev,
				 struct SelftestUserCmdData_t *cmddata)
{
	u8 result  = ST_SELFTEST_FAILED; /* set to default */
	struct pin_config  GPIOSetup;
	int ret;
	/* Input */
	u8 name      = cmddata->parm1;
	u8 direction = cmddata->parm2;
	u8 *state    = (u8 *)&cmddata->parm3;

	ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () called -- ST_SELFTEST_GPS_READ/WRITE %x %x %x", name, direction, *state);

	GPIOSetup.func          = PF_GPIO;
	GPIOSetup.reg.val       = 0;
	GPIOSetup.reg.b.drv_sth = DRIVE_STRENGTH_2MA;

	switch (name) {
	case ST_SELFTEST_GPS_TXP:
		if (GPS_PABLANK_Setup_as_GPIO == false) {
			ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () GPS_PABLANK_Setup_as_GPIO");
			StoredValue_GPS_PABLANK.name = PN_GPS_PABLANK;
			pinmux_get_pin_config(&StoredValue_GPS_PABLANK);
			pinmux_set_pin_config(&GPIOSetup);
			ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () 0x%08X",
				   StoredValue_GPS_PABLANK.reg.val);
			ret = gpio_request(GPIO_GPS_PABLANK, "GPS PABLANK");
			if (ret < 0) {
				ST_DBG("GLUE_SELFTEST::gpio %u request failed",
				       GPIO_GPS_PABLANK);
/*				  dev_err(&pdev->dev, "Unable to request GPIO pin %d\n", GPIO_GPS_PABLANK);*/
				result = ST_SELFTEST_FAILED;
			}
			GPS_PABLANK_Setup_as_GPIO = true;
		}
		switch (direction) {
		case ST_SELFTEST_GPS_WRITE:{
			ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () GPS_PABLANK - Write");
			switch (*state) {
			case ST_SELFTEST_GPS_HIGH:
				 gpio_direction_output(GPIO_GPS_PABLANK, 1);
				 gpio_set_value(GPIO_GPS_PABLANK, 1);
				 break;
			case ST_SELFTEST_GPS_LOW:
				 gpio_direction_output(GPIO_GPS_PABLANK, 0);
				 gpio_set_value(GPIO_GPS_PABLANK, 0);
				 break;
			case ST_SELFTEST_GPS_RELEASE:
			default:
			/* assume release state, defined in ISI */
			if (GPS_PABLANK_Setup_as_GPIO == true) {
				ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () GPS_PABLANK - Restore");
				pinmux_set_pin_config(&StoredValue_GPS_PABLANK);
				gpio_free(GPIO_GPS_PABLANK);
				GPS_PABLANK_Setup_as_GPIO = false;
			}
			break;
			}
			result = ST_SELFTEST_OK;
			break;
			}
		case ST_SELFTEST_GPS_READ:
			 gpio_direction_input(GPIO_GPS_PABLANK);
			 ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () GPS_PABLANK - Read");
			 if (gpio_get_value(GPIO_GPS_PABLANK) == 1) {
			     ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () >>>High");
			     *state = ST_SELFTEST_GPS_HIGH;
			 } else {
			     ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () >>>Low");
			     *state = ST_SELFTEST_GPS_LOW;
			 }
			 result = ST_SELFTEST_OK;
			 break;
		default:
			assert(false);
			break;
		}
		break;
	case ST_SELFTEST_GPS_TIMESTAMP:
		if (GPS_TMARK_Setup_as_GPIO == false) {
			ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () GPS_TMARK_Setup_as_GPIO");
			StoredValue_GPS_TMARK.name = PN_GPS_TMARK;
			pinmux_get_pin_config(&StoredValue_GPS_TMARK);
			pinmux_set_pin_config(&GPIOSetup);
			ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () 0x%08X",
				   StoredValue_GPS_TMARK.reg.val);
			ret = gpio_request(GPIO_GPS_TMARK, "GPS TMARK");
			if (ret < 0) {
				ST_DBG("GLUE_SELFTEST::gpio %u request failed",
				       GPIO_GPS_PABLANK);
/*			  dev_err(&pdev->dev, "Unable to request GPIO pin %d\n", GPIO_GPS_PABLANK);*/
				result = ST_SELFTEST_FAILED;
			}
			GPS_TMARK_Setup_as_GPIO = true;
		}
		switch (direction) {
		case ST_SELFTEST_GPS_WRITE:{
			 ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () GPS_TMARK - Write");
			switch (*state) {
			case ST_SELFTEST_GPS_HIGH:
				 gpio_direction_output(GPIO_GPS_TMARK, 1);
				 gpio_set_value(GPIO_GPS_TMARK, 1);
				 break;
			case ST_SELFTEST_GPS_LOW:
				 gpio_direction_output(GPIO_GPS_TMARK, 0);
				 gpio_set_value(GPIO_GPS_TMARK, 0);
				 break;
			case ST_SELFTEST_GPS_RELEASE:
			default:
				 /* assume release state, defined in ISI */
				 if (GPS_TMARK_Setup_as_GPIO == true) {
					ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () GPS_TMARK - Restore");
					pinmux_set_pin_config(&StoredValue_GPS_TMARK);
					gpio_free(GPIO_GPS_TMARK);
					GPS_TMARK_Setup_as_GPIO = false;
				 }
				 break;
			 }
			 result = ST_SELFTEST_OK;
			 break;
		}

		case ST_SELFTEST_GPS_READ:
			 ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () GPS_TMARK - Read");
			 gpio_direction_input(GPIO_GPS_TMARK);
			if (gpio_get_value(GPIO_GPS_TMARK) == 1) {
				 ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () >>>High");
				*state = ST_SELFTEST_GPS_HIGH;
			} else {
				ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () >>>Low");
				*state = ST_SELFTEST_GPS_LOW;
			}
			result = ST_SELFTEST_OK;
			break;
		default:
			assert(false);
			break;
		}
		break;
	default:
		result = ST_SELFTEST_NOT_SUPPORTED;
		break;
	}

	if (ST_SELFTEST_OK == result) {
		ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () returned -----> ST_SELFTEST_OK");
	} else {
		ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () returned -----> ST_SELFTEST_FAILED");
	}

	cmddata->testStatus = result;
}
#endif

#ifdef USB_ST_SUPPORTED
static u8 hal_selftest_usb_charger_latch_ok = 0;
static u8 hal_selftest_usb_charger_called = 0;

void std_selftest_usb_charger(struct SelftestDevData_t *dev,
			      struct SelftestUserCmdData_t *cmddata)
{
	u8 mbc5_orig, mbc5_bcd_aon, i;
	u8 status;

	if (!hal_selftest_usb_charger_called) {
#ifdef NEED_TO_SIGNUP_FOR_EVENT_IN_LMP
		status = HAL_EM_PMU_RegisterEventCB (PMU_DRV_CHGDET_LATCH, &hal_selftest_usb_charger_latch_cb);
#endif
		ST_DBG("GLUE_SELFTEST::hal_selftest_usb_charger() Status: %d", status);
		hal_selftest_usb_charger_called = 1;
	}

	hal_selftest_usb_charger_latch_ok = 0;

	/* Set PMU into BCDLDO Always on in MBCCTRL5 [1] */
	mbc5_orig = bcm590xx_reg_read(dev->bcm_5900xx_pmu_dev, BCM59055_REG_MBCCTRL5);

	if (mbc5_orig & BCM59055_REG_MBCCTRL5_USB_DET_LDO_EN) {
		/* LDO is already on. */
		mbc5_bcd_aon = mbc5_orig & ~(BCM59055_REG_MBCCTRL5_USB_DET_LDO_EN);
		ST_DBG("GLUE_SELFTEST::hal_selftest_usb_charger(); USB_DET_LDO is already on, orig = %x aon = %x", mbc5_orig, mbc5_bcd_aon);
		bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev, BCM59055_REG_MBCCTRL5, mbc5_bcd_aon);
		mdelay(5);
	}

	mbc5_bcd_aon = mbc5_orig | BCM59055_REG_MBCCTRL5_BCDLDO_AON | BCM59055_REG_MBCCTRL5_USB_DET_LDO_EN | BCM59055_REG_MBCCTRL5_BC11_EN;
	bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev, BCM59055_REG_MBCCTRL5, mbc5_bcd_aon);

	ST_DBG("GLUE_SELFTEST::hal_selftest_usb_charger() MBCCTRL5 original %x, aon %x", mbc5_orig, mbc5_bcd_aon);

	for (i = 0; i < 200; i++) {
		/* wait for CHGDET_LATCH status */
		mdelay(2);
		if (hal_selftest_usb_charger_latch_ok)
			break;
	}
	/* Restore MBCCTRL5 */
	mdelay(25);

	bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev, BCM59055_REG_MBCCTRL5, mbc5_orig);
	/* IF chp_typ == 0x01, test is working correctly. chp_typ is located in bits 4 and 5.*/

	if (hal_selftest_usb_charger_latch_ok) {
		ST_DBG("GLUE_SELFTEST::hal_selftest_usb_charger() Connection is good");
			cmddata->subtestCount = 0;
			cmddata->subtestStatus[i] = ST_SELFTEST_OK;
	} else {
		cmddata->subtestCount = 1;
		cmddata->subtestStatus[i] = ST_SELFTEST_FAILED;
	}
}
#endif

#ifdef ADC_ST_SUPPORTED
void std_selftest_adc(struct SelftestDevData_t *dev,
		      struct SelftestUserCmdData_t *cmddata)
{
    ST_DBG("GLUE_SELFTEST::std_selftest_adc () called.");

    /*...........*/
    /*....TBD....*/
    /*...........*/

}
#endif


/********************************************************************/
/* Command Handling */
/********************************************************************/

static void SelftestHandleCommand(enum SelftestUSCmds_e cmd,
				  struct SelftestDevData_t *dev,
				  struct SelftestUserCmdData_t *cmddata)
{
	ST_DBG("SelftestHandleCommand::Cmd:%u", cmddata->testId);
	/* Handle Userspace Commands */
	switch (cmd) {
	#ifdef GPS_TEST_SUPPORTED
	case ST_SUSC_GPS_TEST:
		std_selftest_control_gps_io(dev, cmddata);
		break;
	#endif
	#ifdef USB_ST_SUPPORTED
	case ST_SUSC_USB_ST:
		std_selftest_usb_charger(dev, cmddata);
		break;
	#endif
	#ifdef ADC_ST_SUPPORTED
	case ST_SUSC_ADC_ST:
		std_selftest_adc(dev, cmddata);
		break;
	#endif
	default:
		SelftestHandleCommand_Low(cmd, dev, cmddata);
		break;
	}
}

/********************************************************************/
/* Selftest file interface */
/********************************************************************/
/***********************PMU PROC DEBUG Interface*********************/
static int selftest_open(struct inode *inode, struct file *file)
{
	pr_debug("%s\n", __func__);
	file->private_data = PDE(inode)->data;

	return 0;
}

int selftest_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}
static long selftest_unlocked_ioctl(struct file *filp, unsigned int cmd,
				     unsigned long arg)
{
	struct SelftestUserCmdData_t cmddata;
	struct SelftestDevData_t dev;
	ST_DBG("Selftest command Received");
	ST_DBG("Selftest copy_from_user");
	dev.bcm_5900xx_pmu_dev = bcm590xx_dev;
	if (copy_from_user((void *)&cmddata, (void*)arg,
		sizeof(SelftestUserCmdData_t))) {
	ST_DBG("Selftest copy_from_user Failed");
	return -EFAULT;
	}

	cmddata.testId = cmd;
	SelftestHandleCommand((enum SelftestUSCmds_e)cmd, &dev, &cmddata);

	ST_DBG("Selftest copy_to_user");
	if (copy_to_user ((void *)arg, (void*)&cmddata, sizeof(SelftestUserCmdData_t))) {
		ST_DBG("Selftest copy_to_user Failed");
		return -EFAULT;
	}
	return 0;
}

#define MAX_USER_INPUT_LEN 10
static ssize_t selftest_write(struct file *file, const char __user *buffer,
	size_t len, loff_t *offset)
{
	struct SelftestUserCmdData_t cmddata;
	struct SelftestDevData_t dev;

	if (len > MAX_USER_INPUT_LEN)
		len = MAX_USER_INPUT_LEN;

	dev.bcm_5900xx_pmu_dev = bcm590xx_dev;
	cmddata.testId = buffer[0]-'a';
	switch (cmddata.testId) {
	case ST_SUSC_DIGIMIC:
		cmddata.parm1 = 1; /* Enable DMIC 1 Test */
		cmddata.parm2 = 1; /* Enable DMIC 2 Test */
		break;
	case ST_SUSC_GPS_TEST:
		cmddata.parm1 = 0;
		cmddata.parm2 = 0;
		cmddata.parm3 = 0;
		break;
	default:
		break;
	}
	SelftestHandleCommand(cmddata.testId, &dev, &cmddata);
	if (cmddata.testStatus == 0) {
		ST_DBG("Selftest OK");
	} else {
		ST_DBG("Selftest Failed (%u)", cmddata.testStatus);
	}


	*offset += len;
	return len;
}

static const struct file_operations selftest_ops = {
	.open = selftest_open,
	.unlocked_ioctl = selftest_unlocked_ioctl,
	.write = selftest_write,
	.release = selftest_release,
	.owner = THIS_MODULE,
};

/********************************************************************/
/* Selftest Drivers Stuff */
/********************************************************************/

/******************************************************************************
*
* Function Name: selftest_pmu_probe
*
* Desc: Called to perform module initialization when the module is loaded.
*
******************************************************************************/
static int __devinit selftest_pmu_probe(struct platform_device *pdev)
{
  bcm590xx_dev = dev_get_drvdata(pdev->dev.parent);

  ST_DBG("Selftest CSAPI driver probed");

  /* Register proc interface */
	proc_create_data("selftest", S_IRWXUGO, NULL,
		   &selftest_ops, NULL/*private data*/);

  return 0;
}

static int __devexit selftest_pmu_remove(struct platform_device *pdev)
{
	return 0;
}

struct platform_driver selftestpmu_driver = {
	.probe = selftest_pmu_probe,
	.remove = __devexit_p(selftest_pmu_remove),
	.driver = {
		   .name = "bcm_selftest_bb",
		   }
};

/****************************************************************************
*
*  selftest_pmu_init
*
*     Called to perform module initialization when the module is loaded
*
***************************************************************************/

static int __init selftest_pmu_init(void)
{
	/* Register driver */
	platform_driver_register(&selftestpmu_driver);

	return 0;
}

/****************************************************************************
*
*  selftest_pmu_exit
*
*       Called to perform module cleanup when the module is unloaded.
*
***************************************************************************/

static void __exit selftest_pmu_exit(void)
{
	platform_driver_unregister(&selftestpmu_driver);

}

subsys_initcall(selftest_pmu_init);
module_exit(selftest_pmu_exit);

MODULE_AUTHOR("PHERAGER");
MODULE_DESCRIPTION("BCM SELFTEST BB");
