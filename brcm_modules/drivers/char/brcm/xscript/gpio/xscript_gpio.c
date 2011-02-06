/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

//***************************************************************************
/**
*
*   @file   xscript_gpio.c
*
*   @brief  This test module is for executing and testing the kernel code from drivers/gpio. 
* 		  This module is driven by a user space program through calls to the ioctl
*
*
****************************************************************************/
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/pci.h>
#include <linux/sysdev.h>
#include <asm/uaccess.h>
#include <linux/errno.h>
#include <mach/hardware.h>

//Include GPIOLIB API's 
#include <linux/gpio.h>

#include "xscript_gpio.h"

//GPIO pins status
#define INPUT 1
#define OUTPUT 0

static int Major = GPIOMAJOR;
//Function prototype's
static int gpio_ioctl(struct inode *, struct file *, unsigned int,
			unsigned long);
static int gpio_open(struct inode *, struct file *);
static int gpio_close(struct inode *, struct file *);

extern int bcm_gpio_pull_up_down_enable(unsigned int gpio, bool enable);
extern int bcm_gpio_pull_up(unsigned int gpio, bool enable);


//Kernel module API's for user space test cases.
int test_gpio_get_value(int);
int test_gpio_request(int);
int test_gpio_free(int);
int test_gpio_direction_input(int);
int test_gpio_direction_output(int);
int test_gpio_set_value(int,int);
int test_gpio_direction_is_output(int);
int test_bcm_gpio_pull_up_down_enable(int,bool);
int test_bcm_gpio_pull_up(int,bool);
/*
 * File operations struct, to use operations find the
 * correct file descriptor
 */
static struct file_operations gpio_fops =
{
	open : gpio_open,
	release: gpio_close,
	ioctl: gpio_ioctl,
};

static int gpio_open(struct inode *ino, struct file *f)
{
	return 0;
}

static int gpio_close(struct inode *ino, struct file *f)
{
	return 0;
}

/*
 * gpio_ioctl:
 *      a user space program can drive the test functions
 *      through a call to ioctl once the correct file
 *      descriptor has been attained
 */
static int gpio_ioctl(struct inode *ino, struct file *f,
			unsigned int cmd, unsigned long l)
{
	int			rc;
	tmod_interface_t	tif;
	int			*inparms;
	int			*inparms_0;
	inparms = NULL;
	inparms_0 = NULL;
	rc = 0;

	/*
	 * the following calls are used to setup the
	 * parameters that might need to be passed
	 * between user and kernel space, using the tif
	 * pointer that is passed in as the last
	 * parameter to the ioctl
	 */
	if (copy_from_user(&tif, (void *)l, sizeof(tif)) )
	{
		/* Bad address */
		return(-EFAULT);
	}

	/*
	 * Setup inparms and outparms as needed
	 */
	if (tif.in_len > 0)
	{
		inparms = tif.in_data;               
	}

	/*
	 * Setup inparms_0 as needed
	 */
	if (tif.in_len_0 > 0)
	{
		inparms_0 = tif.in_data_0;               
	}

	/*
	 * Use a switch statement to determine which function
	 * to call, based on the cmd flag that is specified
	 * in user space. Pass in inparms or outparms as
	 * needed
	 */
	switch(cmd)
	{
		case GPIO_GET_VALUE:	rc=test_gpio_get_value(*inparms); break;
		case GPIO_SET_VALUE:	rc=test_gpio_set_value(*inparms,*inparms_0); break;
		case GPIO_REQUEST:	rc=test_gpio_request(*inparms); break;
		case GPIO_FREE:		rc=test_gpio_free(*inparms); break;
		case GPIO_DIR_OUTPUT:	rc=test_gpio_direction_output(*inparms); break;
		case GPIO_DIR_INPUT:	rc=test_gpio_direction_input(*inparms); break;
		case GPIO_IS_OUTPUT:	rc=test_gpio_direction_is_output(*inparms); break;
		case GPIO_PULL_DOWN_UP_EN:  rc=test_bcm_gpio_pull_up_down_enable(*inparms,*inparms_0);break;		
		case GPIO_PULL_DOWN_UP:  rc=test_bcm_gpio_pull_up(*inparms,*inparms_0);break;
		default:
			pr_info("gpio-test: Mismatching ioctl command\n");
			break;
	}

	/*
	 * copy in the test return code, the reason we
	 * this is so that in user space we can tell the
	 * difference between an error in one of our test
	 * calls or an error in the ioctl function
	 */
	tif.out_rc = rc;
	rc = 0;
	/*
	 * setup the rest of tif pointer for returning to
	 * to user space, using copy_to_user if needed
	 */
	
	/* copy tif structure into l so that can be used by user program */
	if(copy_to_user((void*)l, &tif, sizeof(tif)) )
	{
		pr_info("gpio-test: Unsuccessful copy_to_user of tif\n");
		rc = -EFAULT;
	}
	

	return rc;
}

/*
 *  gpio_get_value
 *      Get the value of GIOx status
 *      Input : GIO pin
 *      Output: Status of GIOx pin status.
 */
int test_gpio_get_value(int giox )
{
	return(gpio_get_value((unsigned)giox));
}

/*
 * gpio_request
 *      Request the GIOx
 *      Input : GIO pin
 *      Output: Return 0 for success or fail.
 */
int test_gpio_request(int giox)
{
	return(gpio_request((unsigned)giox,"GPIO_TEST"));
}

/*
 * gpio_free
 *      Free the GIOx
 *      Input : GIO pin
 *      Output: Return 0 for success or fail.
 */
int test_gpio_free(int giox )
{
	gpio_free((unsigned)giox);
	return 0;
}

/*
 * Set the gpio direction output
 *      Set the direction of GIOx pin to output
 *      Input : GIO pin
 *      Output: Return 0 for success or fail.
 */
int test_gpio_direction_output(int giox )
{
	return(gpio_direction_output((unsigned)giox, OUTPUT));
}

/*
 * Set the gpio direction input
 *      Set the direction of GIOx pin to input
 *      Input : GIO pin
 *      Output: Return 0 for success or fail.
 */
int test_gpio_direction_input(int giox )
{
	return(gpio_direction_input((unsigned)giox));
}

/*
 * Test GIOx pin directon is OUTPUT
 *      Check the direction of GIOx pin is output
 *      Input : GIO pin
 *      Output: Return 0 for success or fail.
 */
int test_gpio_direction_is_output(int giox )
{
/*
 * TODO: gpio_direction_is_output() is no longer supported
 *       Remove this fucntion.
 */
//	return(gpio_direction_is_output((unsigned)giox));
	return -EPERM;
}

/*
 *    gpio_set_value
 *      Set the value of GIOx status
 *      Input : GIO pin
 *      Output: Return 0 for success or fail.
 */
int test_gpio_set_value(int giox,int val)
{
	gpio_set_value((unsigned)giox,val);
	return 0;
}

/*
 *   bcm_gpio_pull_up	    
 *      The GIOx pin pull-up/pull-down  
 *      Input : GIO pin  and pull-up/pull-down
 *      Output: Return 0 for success or fail.
 */
int test_bcm_gpio_pull_up(int giox , bool enable)
{	
	return(bcm_gpio_pull_up((unsigned)giox,enable));
}
/*
 *   bcm_gpio_pull_up_down_enable
 *      Enable/Disable the GIOx pin pull-up/pull-down type
 *      Input : GIO pin  and enable/disable
 *      Output: Return 0 for success or fail.
 */
int test_bcm_gpio_pull_up_down_enable(int giox , bool enable)
{
	        return(bcm_gpio_pull_up_down_enable((unsigned)giox,enable));
}

/*
 * gpio_init_module
 *      set the owner of tbase_fops, register the module
 *      as a char device, and perform any necessary
 *      initialization
 */
static int gpio_init_module(void)
{
	int rc;
	gpio_fops.owner = THIS_MODULE;

	pr_info("gpio-test: *** Register device %s **\n", DEVICE_NAME);

	rc = register_chrdev(Major, DEVICE_NAME, &gpio_fops);
	if (rc < 0)
	{
		pr_info("gpio-test: Failed to register device.\n");
		return rc;
	}

	if(Major == 0)
		Major = rc;

	
	/* call any other init functions you might use here */
	pr_info("gpio-test: Registration success.\n");
	return 0;
}

/*
 * gpio_exit_module
 *      unregister the device and any necessary
 *      operations to close devices
 */ 
static void gpio_exit_module(void)
{
	/* free any pointers still allocated, using kfree*/
	unregister_chrdev(Major, DEVICE_NAME);
}

/* specify what that init is run when the module is first
 * loaded and that exit is run when it is removed
 */

module_init(gpio_init_module)
module_exit(gpio_exit_module)
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("GPIO Kernel test module");

