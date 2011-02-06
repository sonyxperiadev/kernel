/*******************************************************************************************
 * Copyright 2010 Broadcom Corporation.  All rights reserved.
 *
 * Unless you and Broadcom execute a separate written software license agreement
 * governing use of this software, this software is licensed to you under the
 * terms of the GNU General Public License version 2, available at
 * http://www.gnu.org/copyleft/gpl.html (the "GPL").
 *
 * Notwithstanding the above, under no circumstances may you combine this software
 * in any way with any other Broadcom software provided under a license other than
 * the GPL, without Broadcom's express prior written consent.
 * *******************************************************************************************/

/***************************************************************************
**
*
*   @file   xscript_regulator.c
*
*   @brief  This test module is for executing and testing the kernel code from drivers/regulator.
*                 This module is driven by a user space program through calls to the ioctl
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
#include <asm/uaccess.h>
#include <linux/errno.h>
#include <linux/interrupt.h>

//REGULATOR Driver framework API's
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include "xscript_regulator.h"


int test_regulator_get(char *);
int test_regulator_is_enabled();
int test_regulator_enable();
int test_regulator_set_voltage(int);
int test_regulator_get_voltage();
int test_regulator_disable();
int test_regulator_put();


static struct regulator* gRegulator = NULL;
//regulator-test major number
unsigned int Major=REGULATORMAJOR;


//DMA-TEST file ops
static int regulator_ioctl(struct inode *, struct file *, unsigned int,unsigned long);
static int regulator_open(struct inode *, struct file *);
static int regulator_close(struct inode *, struct file *);


/*
 * File operations struct, to use operations find the
 * correct file descriptor
 */
static struct file_operations regulator_fops = {
        open : regulator_open,
        release: regulator_close,
        ioctl: regulator_ioctl,
};

static int regulator_open(struct inode *ino, struct file *f) {
        return 0;
}

static int regulator_close(struct inode *ino, struct file *f) {

        return 0;
}


/*
 * regulator_ioctl:
 *      a user space program can drive the test functions
 *      through a call to ioctl once the correct file
 *      descriptor has been attained
 */
static int regulator_ioctl(struct inode *ino, struct file *f,
                        unsigned int cmd, unsigned long l)
{
	int 				rc=0;
	tmod_interface_t	tif;
	char *consumer=NULL;
	int 				MicroVolts=0;


	
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
	
				   consumer = tif.in_data;
					pr_info("consumer in kernel %s   %s \n",consumer,tif.in_data);
		   }
	
		   /*
			 * Setup inparms_0 as needed
	 		*/
			if (tif.in_len_0 > 0)
			{
				MicroVolts= *tif.in_data_0;
				pr_info("MicroVolts  %d   %d \n",MicroVolts,*tif.in_data_0);
			}
	
		   /*
			* Use a switch statement to determine which function
			* to call, based on the cmd flag that is specified
			* in user space. Pass in inparms or outparms as
			* needed
			*
			*/
		   switch(cmd)
		   {
	
				  case BCM_REGULATOR_GET: rc= test_regulator_get(consumer); break;
				  case BCM_REGULATOR_IS_ENABLED: rc=test_regulator_is_enabled(); break;
				  case BCM_REGULATOR_ENABLE: rc=test_regulator_enable();break;
				  case BCM_REGULATOR_SET_VOLTAGE: rc=test_regulator_set_voltage(MicroVolts);break;
				  case BCM_REGULATOR_GET_VOLTAGE: rc=test_regulator_get_voltage();break;
				  case BCM_REGULATOR_DISABLE: rc=test_regulator_disable();break;
				  case BCM_REGULATOR_PUT: rc=test_regulator_put();break;
				  default:
						   pr_info("regulator-test: Mismatching ioctl command\n");
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
	//printk("Kernel output parameteris %d",*outparms);
	/*
	 * setup the rest of tif pointer for returning to
	 * to user space, using copy_to_user if needed
	 */
	
    /* copy tif structure into l so that can be used by user program */
    if(copy_to_user((void*)l, &tif, sizeof(tif)))
    {
        pr_info("regulator-test: Unsuccessful copy_to_user of tif\n");
        rc = -EFAULT;
    }
	
	
  
	return rc;

}



//Get the regulator device
int test_regulator_get(char *inConsumer)
{
	
	gRegulator = regulator_get(NULL,inConsumer);
    if ( gRegulator > 0 )
    {
        return 0;
    }
    else
    {
		return -1;
    }	

}
//Check the regulator os enabled
// Returns enabled if return value >0 
int test_regulator_is_enabled()
{
	if(gRegulator)
	{
		return(regulator_is_enabled(gRegulator));
	}
	else
	{
		return -1;
	}
}

//Return 0 for success or <0 for failure
int test_regulator_enable()
{
	if(gRegulator)
	{
		return(regulator_enable(gRegulator));
	}
	else
	{
		return -1;
	} 
	
	
}

//Return 0 for success or <0 for failure
int test_regulator_set_voltage(int MicroVolts)
{
	
	if(gRegulator)
	{
		return(regulator_set_voltage(gRegulator,MicroVolts,MicroVolts));
	}
	else
	{
		return -1;
	} 
}

//Return voltage in Micro volts
int test_regulator_get_voltage()
{
	if(gRegulator)
	{
		return(regulator_get_voltage(gRegulator));
	}
	else
	{
		return -1;
	} 

	
}

//Return 0 for success or <0 for failure
int test_regulator_disable()
{
	if(gRegulator)
	{
		return(regulator_disable(gRegulator));
	}
	else
	{
		return -1;
	} 	 
	
	
}

//Return 0 for success.
int test_regulator_put()
{
	if(gRegulator)
	{ 
		regulator_put(gRegulator); //returns void
		gRegulator=NULL;
		return 0;
	}
	else
	{
		return -1;
	} 	 
	
	
}



/*
 * regulator_init_module
 *      set the owner of regulator_fops, register the module
 *      as a char device, and perform any necessary
 *      initialization
 */
static int regulator_init_module(void)
{
	int rc;

	regulator_fops.owner = THIS_MODULE;

    pr_info("regulator-test: *** Register device %s **\n", TEST_DEVICE_NAME);

	rc = register_chrdev(Major, TEST_DEVICE_NAME, &regulator_fops);
    if (rc < 0)
   	{
    	pr_info("regulator-test: Failed to register device.\n");
        return rc;
    }

    if(Major == 0)
    Major = rc;
	
	/* call any other init functions you might use here */
	pr_info("regulator-test: Registration success.\n");
   
	return 0;
}

/*
 * regulator_exit_module
 *      unregister the device and any necessary
 *      operations to close devices
 */ 
static void regulator_exit_module(void)
{

	/* free any pointers still allocated, using kfree*/
	
	unregister_chrdev(Major, TEST_DEVICE_NAME);
}

/* specify what that init is run when the module is first
loaded and that exit is run when it is removed */

MODULE_AUTHOR("Brodcom");
MODULE_DESCRIPTION(TEST_DEVICE_NAME);
MODULE_LICENSE("GPL");
module_init(regulator_init_module)
module_exit(regulator_exit_module)

