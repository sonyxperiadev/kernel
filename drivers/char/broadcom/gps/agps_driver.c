/****************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
******************************************************************************/

/***************************************************************************
**
*
*   @file   gps_driver.c
*
*   @brief  This driver is used for turn on/off AGPS CNTIN clock
*
*
****************************************************************************/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>

#include <linux/clk.h>

#include <linux/broadcom/bcm_major.h>

#define GPS_KERNEL_MODULE_NAME  "bcm_gps"

#define GPS_LOG_LEVEL	KERN_ERR

#define GPS_KERNEL_TRACE_ON
#ifdef GPS_KERNEL_TRACE_ON
#define GPS_TRACE(str) printk str
#else
#define GPS_TRACE(str) {}
#endif

#define GPS_CMD_CNTIN_CLK_ON	0
#define GPS_CMD_CNTIN_CLK_OFF	1

/**
 *  file ops
 */
static int gps_kernel_open(struct inode *inode, struct file *filp);
static int gps_kernel_read(struct file *filep, char __user *buf,
			   size_t size, loff_t *off);
static long gps_kernel_ioctl(struct file *filp, unsigned int cmd,
			     unsigned long arg);
static int gps_kernel_release(struct inode *inode, struct file *filp);

static struct class *gps_class;
static struct clk *gGpsCntinClk;	/* = NULL; */
static unsigned char gIsCntinClkOn;	/* = 0; */

/*Platform device data*/
/*
typedef struct _PlatformDevData_t {
	int init;
} PlatformDevData_t;
*/
static const struct file_operations sFileOperations = {
	.owner = THIS_MODULE,
	.open = gps_kernel_open,
	.read = gps_kernel_read,
	.write = NULL,
	.unlocked_ioctl = gps_kernel_ioctl,
	.poll = NULL,
	.mmap = NULL,
	.release = gps_kernel_release
};

/*************************************************************
**
 *  Called by Linux I/O system to handle open() call.
 *  @param  (in)    not used
 *  @param  (io)    file pointer
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by struct file_operations 'open' member.
 */

static int gps_kernel_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	if (gGpsCntinClk == NULL) {
		gGpsCntinClk = clk_get(NULL, "dig_ch3_clk");
		if (IS_ERR(gGpsCntinClk)) {
			ret = PTR_ERR(gGpsCntinClk);
			gGpsCntinClk = NULL;
		}
	}
	GPS_TRACE((GPS_LOG_LEVEL "gps_kernel_open gGpsCntinClk=0x%x ret=%d\n",
		   (unsigned int)gGpsCntinClk, ret));
	return ret;
}

static int gps_kernel_read(struct file *filep, char __user *buf,
			   size_t size, loff_t *off)
{
	GPS_TRACE((GPS_LOG_LEVEL "gps_kernel_read\n"));
	return 0;
}

static long gps_kernel_ioctl(struct file *filp, unsigned int cmd,
			     unsigned long arg)
{
	GPS_TRACE((GPS_LOG_LEVEL "gps_kernel_ioctl cmd=%d IsOn=%d\n",
		   cmd, gIsCntinClkOn));

	switch (cmd) {
	case GPS_CMD_CNTIN_CLK_ON:
		if (gGpsCntinClk && (gIsCntinClkOn == 0)) {
			clk_enable(gGpsCntinClk);
			gIsCntinClkOn = 1;
			GPS_TRACE((GPS_LOG_LEVEL
				   "gps_kernel_ioctl clk_enable()\n"));
		}
		break;
	case GPS_CMD_CNTIN_CLK_OFF:
		if (gGpsCntinClk && (gIsCntinClkOn == 1)) {
			clk_disable(gGpsCntinClk);
			gIsCntinClkOn = 0;
			GPS_TRACE((GPS_LOG_LEVEL
				   "gps_kernel_ioctl clk_disable()\n"));
		}
		break;
	default:
		break;
	}
	return 0;
}

static int gps_kernel_release(struct inode *inode, struct file *filp)
{
	GPS_TRACE((GPS_LOG_LEVEL
		   "gps_kernel_release gGpsCntinClk=0x%x IsOn=%d\n",
		   (unsigned int)gGpsCntinClk, gIsCntinClkOn));
	if (gGpsCntinClk) {
		if (gIsCntinClkOn) {
			clk_disable(gGpsCntinClk);
			gIsCntinClkOn = 0;
		}
		clk_put(gGpsCntinClk);
		gGpsCntinClk = NULL;
	}
	return 0;
}

/***************************************************************************
**
 *  Called by Linux I/O system to initialize module.
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by module_init macro
 */
static int __init gps_kernel_module_init(void)
{
	int err = 1;
	struct device *myDevice;
	dev_t myDev;

	GPS_TRACE((GPS_LOG_LEVEL "enter gps_kernel_module_init()\n"));

	/*drive driver process: */
	if (register_chrdev(BCM_GPS_MAJOR, GPS_KERNEL_MODULE_NAME,
			    &sFileOperations) < 0) {
		GPS_TRACE((GPS_LOG_LEVEL "register_chrdev failed\n"));
		return -1;
	}

	gps_class = class_create(THIS_MODULE, "bcm_gps");
	if (IS_ERR(gps_class))
		return PTR_ERR(gps_class);
	myDev = MKDEV(BCM_GPS_MAJOR, 0);

	GPS_TRACE((GPS_LOG_LEVEL "mydev = %d\n", myDev));

	myDevice = device_create(gps_class, NULL, myDev, NULL, "bcm_gps");

	err = PTR_ERR(myDevice);
	if (IS_ERR(myDevice)) {
		GPS_TRACE((GPS_LOG_LEVEL "device create failed\n"));
		return -1;
	}

	GPS_TRACE((GPS_LOG_LEVEL "exit sucessfuly gps_kernel_module_init()\n"));
	return 0;
}

/***************************************************************************
**
 *  Called by Linux I/O system to exit module.
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by module_exit macro
 **/
static void __exit gps_kernel_module_exit(void)
{
	GPS_TRACE((GPS_LOG_LEVEL "gps_kernel_module_exit()\n"));
	unregister_chrdev(BCM_GPS_MAJOR, GPS_KERNEL_MODULE_NAME);
}

/**
 *  export module init and export functions
 **/
module_init(gps_kernel_module_init);
module_exit(gps_kernel_module_exit);
MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPS Driver");
