/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/
/**
*
*  haptics.c
*
*  PURPOSE:
*
*     This file contains the haptics driver routines.
*
*  NOTES:
*
*****************************************************************************/

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/timer.h>
#include <linux/broadcom/bcm_haptics.h>
#include <linux/broadcom/amxr.h>
#include <linux/broadcom/amxr_port.h>
#include "../staging/android/timed_output.h"

static int debug_level = 0;

/* Timer data structures */
static struct timer_list g_haptics_timer;		/* timer to control length of vibration */
static unsigned long g_haptics_off_jiffies = 0;		/* jiffies of the timer expiration; 0 if timer is not set */
static spinlock_t g_haptics_lock;		/* lock for updating g_haptics_off_jiffies */
static atomic_t g_haptics_flag = ATOMIC_INIT(0);		/* 1 for vibrating, 0 otherwise */

/* Audio Mixer data structures */
#define HALAUDIO_PORT_HZ	48000
#define AMXR_HAPTICS_FRAMESZ_16BITS	240	/* 48 kHz, 5ms */

static AMXR_HDL g_amixer_fd;

static int16_t *amxr_getsrc(int bytes, void *privdata);

static AMXR_PORT_CB g_amxr_callback =
{
	.getsrc = amxr_getsrc,
};

static AMXR_PORT_ID g_hal_port_id;
static AMXR_PORT_ID g_ept_port_d;

static int16_t g_data_on[AMXR_HAPTICS_FRAMESZ_16BITS] = {
0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

static int16_t g_data_off[AMXR_HAPTICS_FRAMESZ_16BITS] = {
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

/****************************************************************************
*
*   Audio Mixer getsrc callback function
*
****************************************************************************/
static int16_t *amxr_getsrc(int bytes, void *privdata)
{
	int flag;
	if(debug_level >= 2)
		printk(KERN_DEBUG "%s entry\n", __FUNCTION__);
	flag = atomic_read(&g_haptics_flag);
	if(debug_level >= 2)
		printk(KERN_DEBUG "%s flag=%d\n", __FUNCTION__, flag);
	return flag ? g_data_on : g_data_off;
}

/****************************************************************************
*
*   timed_output get_time callback function
*
****************************************************************************/
static int haptics_get_time(struct timed_output_dev *dev)
{
	int ret = 0;
	if(debug_level >= 1)
		printk(KERN_DEBUG "%s entry\n", __FUNCTION__);
	spin_lock_bh(&g_haptics_lock);
	if(g_haptics_off_jiffies && g_haptics_off_jiffies > jiffies)
		ret = (g_haptics_off_jiffies - jiffies) * 1000 / HZ;
	spin_unlock_bh(&g_haptics_lock);
	if(debug_level >= 1)
		printk(KERN_DEBUG "%s ret=%d\n", __FUNCTION__, ret);
	return ret;
}

/****************************************************************************
*
*   timed_output enable callback function
*
****************************************************************************/
static void haptics_enable(struct timed_output_dev *dev, int msec)
{
if(debug_level >= 1)
		printk(KERN_DEBUG "%s entry\n", __FUNCTION__);
		
	atomic_set(&g_haptics_flag, 1);
	
	spin_lock_bh(&g_haptics_lock);
	g_haptics_off_jiffies = jiffies + msec * HZ / 1000;
	spin_unlock_bh(&g_haptics_lock);
	if(debug_level >= 1)
		printk(KERN_DEBUG "haptics_enable msec=%d haptics_off_jiffies=%lu\n", msec, g_haptics_off_jiffies);

	mod_timer(&g_haptics_timer, g_haptics_off_jiffies);
	if(debug_level >= 1)
		printk(KERN_DEBUG "%s exit\n", __FUNCTION__);
}

/****************************************************************************
*
*   timed_output driver interface
*
****************************************************************************/
static struct timed_output_dev haptics_dev = {
	.name = "vibrator",
	.get_time = haptics_get_time,
	.enable = haptics_enable,
};


/****************************************************************************
*
*   timer callback function
*
****************************************************************************/
static void on_haptics_timer(unsigned long data)
{
	if(debug_level >= 2)
		printk(KERN_DEBUG "%s entry\n", __FUNCTION__);

	atomic_set(&g_haptics_flag, 0);

	spin_lock_bh(&g_haptics_lock);
	g_haptics_off_jiffies = 0;
	spin_unlock_bh(&g_haptics_lock);
	if(debug_level >= 2)
		printk(KERN_DEBUG "%s exit\n", __FUNCTION__);
}

/****************************************************************************
*
*   Function called to initialize haptics driver.
*
****************************************************************************/

static int __init haptics_probe(struct platform_device *pdev)
{
	int rc, rc2;
	struct bcm_haptics_data *driver_data = NULL;

	if(debug_level >= 1)
		printk(KERN_DEBUG "%s: entry\n", __FUNCTION__);

	if(!pdev || !pdev->dev.platform_data)
	{
		printk(KERN_ERR "%s: plaftorm data is not set\n", __FUNCTION__);
		return -ENXIO;
	}
	driver_data = (struct bcm_haptics_data *)pdev->dev.platform_data;

	/* init spinlock */
	spin_lock_init(&g_haptics_lock);

	/* init timer */
	init_timer(&g_haptics_timer);
	g_haptics_timer.function = on_haptics_timer;

	if(debug_level >= 1)
		printk(KERN_DEBUG "%s: halaudio_port_name=%s ept_port_name=%s\n", __FUNCTION__, driver_data->halaudio_port_name, driver_data->ept_port_name);

	/* register timed_output device */
	rc = timed_output_dev_register(&haptics_dev);
	if(rc < 0) {
		printk(KERN_ERR "%s: failed to register timed_output device. rc=%i\n", __FUNCTION__, rc);
		goto do_return;
	}

	/* register audio mixer */
	rc = g_amixer_fd = amxrAllocateClient();
	if(g_amixer_fd < 0) {
			printk(KERN_ERR "%s: failed to allocate Audio Mixer client. rc=%i\n", __FUNCTION__, rc);
			goto do_timed_output_unregister;
	}

	/* create EPT virtual port */
	rc = amxrCreatePort(driver_data->ept_port_name, 
							&g_amxr_callback, NULL /* privdata */,
							0, 0, 0, /* dst */
							HALAUDIO_PORT_HZ, 1 /* mono */, AMXR_HAPTICS_FRAMESZ_16BITS * 2, /* size in bytes */ /* src */
							&g_ept_port_d);
	if(rc) {
		printk(KERN_ERR "%s: failed to create EPT port. rc=%i\n", __FUNCTION__, rc);
		goto do_free_amxr_client;
	}

	/* connect port with endpoint */	
	rc = amxrQueryPortByName(g_amixer_fd, driver_data->halaudio_port_name, &g_hal_port_id);
	if(rc) {
		printk(KERN_ERR "%s: failed to get port ID for %s. rc=%i\n", __FUNCTION__, driver_data->halaudio_port_name, rc);
		goto do_remove_amxr_port;
	}

	rc = amxrConnect(g_amixer_fd, g_ept_port_d, g_hal_port_id, AMXR_CONNECT_MONO2MONO);
	if(rc) {
		printk(KERN_ERR "%s: failed to connect Audio Mixer port. rc=%i\n", __FUNCTION__, rc);
		goto do_remove_amxr_port;
	}

	if(debug_level >= 1)
		printk(KERN_DEBUG "%s OK\n", __FUNCTION__);

	return 0;

do_remove_amxr_port:
	rc2 = amxrRemovePort(g_ept_port_d);
	if(rc2)
		printk(KERN_ERR "%s: failed to remove Audio Mixer Port. rc=%i\n", __FUNCTION__, rc2);

do_free_amxr_client:
	rc2 = amxrFreeClient(g_amixer_fd);
	if(rc2)
		printk(KERN_ERR "%s: failed to free Audio Mixer client. rc=%i\n", __FUNCTION__, rc2);

do_timed_output_unregister:
	timed_output_dev_unregister(&haptics_dev);

do_return:
	return rc;
}

/****************************************************************************
*
*   Function called to remove haptics driver.
*
****************************************************************************/

static int haptics_remove(struct platform_device *pdev)
{
	int rc;

	if(debug_level >= 1)
		printk(KERN_DEBUG "%s\n", __FUNCTION__);
	
	rc = amxrDisconnect(g_amixer_fd, g_ept_port_d, g_hal_port_id);
	if(rc)
		printk(KERN_ERR "%s: failed to disconnect Audio Mixer port. rc=%i\n", __FUNCTION__, rc);

	rc = amxrRemovePort(g_ept_port_d);
	if(rc)
		printk(KERN_ERR "%s: failed to remove Audio Mixer port. rc=%i\n", __FUNCTION__, rc);

	rc = amxrFreeClient(g_amixer_fd);
	if(rc)
		printk(KERN_ERR "%s: failed to free Audio Mixer client rc=%i\n", __FUNCTION__, rc);

	timed_output_dev_unregister(&haptics_dev);
	return 0;
}

/****************************************************************************
*
*   Data structure which defines the haptics driver
*
****************************************************************************/

static struct platform_driver haptics_driver =
{
	.probe	= haptics_probe,
	.remove	= haptics_remove,
	.driver	= 
	{
		.name	= BCM_HAPTICS_DRIVER_NAME,
	},
};

/***************************************************************************
*
*  Driver initialization called when module loaded by kernel
*
*  @return  
*     0              Success
*     -ve            Error code
****************************************************************************/

static int __init haptics_init(void)
{
	int rc = platform_driver_register(&haptics_driver);
	if(debug_level >= 1)
		printk(KERN_DEBUG "%s rc=%i\n", __FUNCTION__, rc);
	return rc;
}

/***************************************************************************
*
*  Driver destructor routine.
*
****************************************************************************/

static void __exit haptics_exit(void)
{
	if(debug_level >= 1)
		printk(KERN_DEBUG "%s\n", __FUNCTION__);
	platform_driver_unregister(&haptics_driver);
}

module_init(haptics_init);
module_exit(haptics_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Haptics Driver");
MODULE_LICENSE("GPL");

module_param(debug_level, int, 0);
MODULE_PARM_DESC(debug_level, "Debug level: 0, 1, or 2");
