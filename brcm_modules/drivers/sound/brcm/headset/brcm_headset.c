/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
* 
* 	@file	drivers/sound/brcm/headset/brcm_headset.c
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
* 
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/input.h>
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/pci.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#ifdef CONFIG_SWITCH
#include <linux/switch.h>
#endif
#include <linux/delay.h>

#include <mach/irqs.h>
//#include <asm/irq.h>
#include <linux/irq.h>
#include <linux/version.h>
#include <linux/broadcom/bcm_major.h>
#include <plat/brcm_headset_pd.h>
#include <plat/syscfg.h>

#include <cfg_global.h>
#include "brcm_headset_hw.h"
#include <mach/gpio.h>

#define REF_TIME 800000000
#define KEY_PRESS_REF_TIME msecs_to_jiffies(100) 
#define KEY_PRESS_THRESHOLD 0x6000
#define MIC_PLUGIN_MASK	0x7f00
#define REG_ANACR12 0x088800b0

#ifdef CONFIG_SWITCH
static void switch_work(struct work_struct *);
#endif
static void input_work_func(struct work_struct *work);

struct h2w_switch_data {
#ifdef CONFIG_SWITCH
        struct switch_dev sdev;
#endif
        struct work_struct work;
};

struct mic_t
{
	int hsirq;
	int hsbirq;
	int hsbst;
	struct auxmic *pauxmic; 
	int headset_state;
	ktime_t hstime;
	ktime_t hsbtime;
        int hsmajor;
	struct h2w_switch_data switch_data;
	struct input_dev *headset_button_idev;
	struct delayed_work input_work;
	struct brcm_headset_pd	*headset_pd;
};

static struct mic_t mic;

extern int bcm_gpio_set_db_val(unsigned int gpio, unsigned int db_val);

static inline int check_delta(ktime_t a, ktime_t b)
{
        int ret = -1;
        ktime_t res = ktime_sub(a,b);
        if(res.tv.sec >= 0){
        /* HSB after HS event -- normal case*/
                if((res.tv.sec >= 1) || (res.tv.nsec > REF_TIME)){
                /* The delay is greater than 800 msec .. so fine*/
                        ret = 0;
                } else {
                /* Delay is < 800 msec so HSB must be spurious*/
                        ret = -1;
                }
        } else {
                /* HSB event before HS event
		 * 1) Could be HSB happened and then HS was removed
 		 *  2) Could be a spurious event on HS removal or insertion
 		 * In any case let us reject it*/
                ret = -1;
        }
        return ret;
}

static void input_work_func(struct work_struct *work)
{
	int delta;
	unsigned long val;
	static int key_status = 0;
	/* If the key_status is 0, send the event for a key press */
	 if(key_status == 0)
	 {
	     delta = check_delta(mic.hsbtime,mic.hstime);
	     if((mic.headset_state== 1) && (delta == 0)){
		 input_report_key(mic.headset_button_idev, KEY_BCM_HEADSET_BUTTON,1);
		 input_sync(mic.headset_button_idev);
	     }
	 }
	 /* Check if the value read from ANACR12 is greater than the
	  * threshold. If so, the key is still pressed and schedule the work
	  * queue till the value is less than the threshold */
	 val = readl(io_p2v(REG_ANACR12));
	 if (val >= KEY_PRESS_THRESHOLD)
	 {
	     key_status = 1;
	     schedule_delayed_work(&(mic.input_work), KEY_PRESS_REF_TIME);
	 }
	 /*Once the value read from ANACR12 is lesser than the threshold, send
	  * the event for a button release */
	 else
	 {
	     key_status = 0;
	     input_report_key(mic.headset_button_idev, KEY_BCM_HEADSET_BUTTON, 0);
             input_sync(mic.headset_button_idev);
	 }
}
/*------------------------------------------------------------------------------
    Function name   : hs_switchinit
    Description     : Register sysfs device for headset
        It uses class switch from kernel/common/driver/switch
    Return type     : int
------------------------------------------------------------------------------*/

#ifdef CONFIG_SWITCH
int hs_switchinit(struct mic_t *p)
{
    int result = 0;
    p->switch_data.sdev.name = "h2w";

    result = switch_dev_register(&p->switch_data.sdev);
    if (result < 0)
    {
        return result;
    }
    INIT_WORK(&p->switch_data.work,switch_work);
    return 0;
}
#endif

/*------------------------------------------------------------------------------
    Function name   : hs_inputdev
    Description     : Create and Register input device for headset button
    Return type     : int
------------------------------------------------------------------------------*/
int  hs_inputdev(struct mic_t *p)
{
    int result = 0;
        // Allocate struct for input device
    p->headset_button_idev = input_allocate_device();
    if ((p->headset_button_idev) == NULL) {
        pr_err("headset button: Not enough memory\n");
    }
    // specify key event type and value for it
    // we have only one button on headset so only one possible
    // value KEY_SEND used here.

    set_bit(EV_KEY, p->headset_button_idev->evbit);
    set_bit(KEY_BCM_HEADSET_BUTTON, p->headset_button_idev->keybit);
    p->headset_button_idev->name = "bcm_headset";
    p->headset_button_idev->phys = "headset/input0";
    p->headset_button_idev->id.bustype = BUS_HOST;
    p->headset_button_idev->id.vendor = 0x0001;
    p->headset_button_idev->id.product = 0x0100;

    // Register input device for headset
    result = input_register_device(p->headset_button_idev);
    if (result) {
    pr_err("button.c: Failed to register device\n");
         return result;
    }
	INIT_DELAYED_WORK(&mic.input_work,input_work_func);
    return 0;
}

/*------------------------------------------------------------------------------
    Function name   : hs_unregsysfs
    Description     : Unregister sysfs and input device for headset
    Return type     : int
------------------------------------------------------------------------------*/

int hs_unregsysfs(struct mic_t *p)
{
    int result = 0;
#ifdef CONFIG_SWITCH
    result = cancel_work_sync(&p->switch_data.work);
    if (result != 0) {
                return result;
        }
    switch_dev_unregister(&p->switch_data.sdev);
#endif
    return 0;
}

int hs_unreginputdev(struct mic_t *p)
{
	input_unregister_device(p->headset_button_idev);
	return 0;
}


/*------------------------------------------------------------------------------
    Function name   : BrcmHeadsetIoctl
    Description     : communication method to/from the user space

    Return type     : int
------------------------------------------------------------------------------*/

static int hs_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
	case BCM_HEADSET_IOCTL_STATUS:
	return mic.headset_state;
    }
    	return 0;
}

/* File operations on HEADSET device */
static struct file_operations hs_fops = {
  ioctl:hs_ioctl,
};

#ifdef CONFIG_SWITCH
// Switch class work to update state of headset
static void switch_work(struct work_struct *work)
{
        struct h2w_switch_data *data =
                container_of(work, struct h2w_switch_data, work);
        switch_set_state(&data->sdev, mic.headset_state);
	if(mic.headset_state){
		 if(mic.hsbst){
			mic.hsbst = 0;
		 	enable_irq(mic.hsbirq);
		 }
	} else {
		if(!mic.hsbst){
		 	disable_irq(mic.hsbirq);
			mic.hsbst = 1;
		}
	}
}
#endif
/*------------------------------------------------------------------------------
    Function name   : hs_isr
    Description     : interrupt handler

    Return type     : irqreturn_t
------------------------------------------------------------------------------*/
irqreturn_t hs_isr(int irq, void *dev_id)
{
    struct mic_t *p = (struct mic_t *)dev_id;
	unsigned long int val = readl(io_p2v(REG_ANACR12));
	val = val & MIC_PLUGIN_MASK;
	/*If the value read from anacr12 is zero and still the ISR is invoked, then the interrupt is
	 * deemed illegal*/
	if(!val)
		return IRQ_NONE;
    p->hstime = ktime_get();
    if (p->headset_pd->check_hs_state)
    {
        p->headset_pd->check_hs_state(&p->headset_state);
        set_irq_type(mic.hsirq, (p->headset_state) ? IRQF_TRIGGER_RISING : IRQF_TRIGGER_FALLING);
    }
    else
    {
	     p->headset_state = (p->headset_state)?0:1;
        set_irq_type(mic.hsirq, (p->headset_state) ? IRQF_TRIGGER_FALLING : IRQF_TRIGGER_RISING);
    }
    schedule_work(&(p->switch_data.work));
    return IRQ_HANDLED; 

}

/*------------------------------------------------------------------------------
    Function name   : hs_buttonisr
    Description     : interrupt handler

    Return type     : irqreturn_t
------------------------------------------------------------------------------*/
irqreturn_t hs_buttonisr(int irq, void *dev_id)
{
    ktime_t r, temp;
    unsigned int val;
    /* Read the ANACR12 register value to check if the interrupt being
     * serviced by the ISR is spurious */
    val = readl(io_p2v(REG_ANACR12));
    temp = ktime_get();
    r = ktime_sub(temp,mic.hsbtime);
    if((r.tv.sec > 0) || (r.tv.nsec > REF_TIME)){
	mic.hsbtime = temp;
    } else {
	return IRQ_HANDLED;
    }
    /* If the value read from the ANACR12 register is greater than the
     * threshold, schedule the workqueue */
    if (val >= KEY_PRESS_THRESHOLD)
	schedule_delayed_work(&(mic.input_work) , 0);
    else
	pr_info("Headset Button press detected for a illegal interrupt\n");
    return IRQ_HANDLED;
}

/*------------------------------------------------------------------------------
    Function name   : BrcmHeadsetModuleInit
    Description     : Initialize the driver

    Return type     : int
------------------------------------------------------------------------------*/


static int hs_remove(struct platform_device *pdev)
{
        return 0;
}


static int __init hs_probe(struct platform_device *pdev)
{
    int result;
    mic.hsmajor = 0;
    mic.headset_state = 0;
    mic.hsbtime.tv.sec = 0;
    mic.hsbtime.tv.nsec = 0;
    mic.headset_pd = NULL;
#ifdef CONFIG_SWITCH
    result = hs_switchinit(&mic);
    if (result < 0) {
        return result;
    }
#endif
    result = hs_inputdev(&mic);
    if (result < 0) {
        goto err;
    }
    result = register_chrdev(mic.hsmajor, "BrcmHeadset", &hs_fops);
    if(result < 0)
    {
	goto err1;
    }
    else if(result > 0 && (mic.hsmajor == 0))    /* this is for dynamic major */
    {
        mic.hsmajor = result;
    }

    /* check if platform data is defined for a particular board variant */
    if (pdev->dev.platform_data)
    {
         mic.headset_pd = pdev->dev.platform_data;
         if (mic.headset_pd->hsgpio == NULL)
         {
             mic.hsirq = mic.headset_pd->hsirq;
         }
         else
         {
             bcm_gpio_set_db_val(mic.headset_pd->hsgpio, 0xF);
             if (gpio_request(mic.headset_pd->hsgpio, "headset detect") < 0)
		       {
	              pr_info("Could not reserve headset signal GPIO!\n");
			       goto err2;
		       }
            gpio_direction_input(mic.headset_pd->hsgpio);
            mic.hsirq = gpio_to_irq(mic.headset_pd->hsgpio);
         }
       mic.hsbirq = mic.headset_pd->hsbirq;
    }
    else
    {
     	mic.hsirq = platform_get_irq(pdev, 0);
     	mic.hsbirq = platform_get_irq(pdev, 1);
    }
    pr_info("HS irq %d\n",mic.hsirq);
    pr_info("HSB irq %d\n",mic.hsbirq);
    result = request_irq(mic.hsbirq, hs_buttonisr, (void *) NULL, "BrcmHeadsetButton",  (void *) NULL);
    mic.hsbst = 1; /* Disabled */
    disable_irq(mic.hsbirq);
    if(result < 0)
    {
	goto err2;
    }
    result = request_irq(mic.hsirq, hs_isr,(IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING), "BrcmHeadset",  &mic);
    if(result < 0)
    {
	free_irq(mic.hsbirq, &mic);
	goto err2;
    }

    pr_info("BrcmHeadset: module inserted >>>> . Major number is = %d\n",mic.hsmajor);
    
    /* Set the ANACR2 bit for mic power down */
    board_sysconfig(SYSCFG_HEADSET, SYSCFG_INIT);

    board_sysconfig(SYSCFG_AUXMIC, SYSCFG_INIT);
    
    /*Fix the audio path is wrong when headset already plugged in the device  then boot device case.*/
    if (mic.headset_pd->hsgpio != NULL)
    {
	     hs_isr(mic.hsirq,&mic);
    }
    return 0;
err2:   unregister_chrdev(mic.hsmajor,"BrcmHeadset");
err1:   hs_unreginputdev(&mic);
err:    hs_unregsysfs(&mic);
	return result;
}
static struct platform_driver headset_driver = {
        .probe = hs_probe,
        .remove = hs_remove,
        .driver = {
                   .name = "bcmheadset",
                   .owner = THIS_MODULE,
                   },
};


/*------------------------------------------------------------------------------
    Function name   : BrcmHeadsetModuleExit
    Description     : clean up

    Return type     : int
------------------------------------------------------------------------------*/

int __init BrcmHeadsetModuleInit(void)
{

	return platform_driver_register(&headset_driver);
}
void __exit BrcmHeadsetModuleExit(void)
{
    // Cancel work of event notification to UI and unregister switch dev.
#ifdef CONFIG_SWITCH	
    	cancel_work_sync(&mic.switch_data.work);
    	switch_dev_unregister(&mic.switch_data.sdev);
#endif
    	free_irq(mic.hsirq, &mic.switch_data);
    	free_irq(mic.hsbirq, NULL);
	input_unregister_device(mic.headset_button_idev);	
	unregister_chrdev(mic.hsmajor,"BrcmHeadset");
	return platform_driver_unregister(&headset_driver);
}

module_init(BrcmHeadsetModuleInit);
module_exit(BrcmHeadsetModuleExit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Headset plug and button detection");
