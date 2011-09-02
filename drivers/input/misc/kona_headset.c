/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file	drivers/input/misc/brcm_headset.c
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

/* This should be defined before kernel.h is included */
/* #define DEBUG */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#ifdef CONFIG_SWITCH
#include <linux/switch.h>
#endif
#include <mach/io_map.h>
#include <mach/kona_headset_pd.h>
#include <mach/rdb/brcm_rdb_aci.h>
#include <mach/rdb/brcm_rdb_auxmic.h>
#include <mach/rdb/brcm_rdb_audioh.h>
#include <mach/rdb/brcm_rdb_khub_clk_mgr_reg.h>

#include <plat/chal/chal_aci.h>


/* TODO:
 * 1. There is a issue now with the interrupts being issued from ACI on headset
 * insert/remove. ACI issues an interrupt for both Headset insert/remove and
 * Headset button press. But the driver uses interrupts issued by ACI to only
 * detect Button press. So when the headset is removed/inserted slowly the ACI
 * interrupt issued for the HS remove/insert is serviced before the interrupt
 * issued on the GPIO line.
 * As a work around till the actual fix is figured out, the delayed work queue
 * for the HS Button press is scheduled after a delay to make sure that even if
 * the HSB IRQ is serviced before the HS ISR, the GPIO status read in
 * delayed work queue before sending the event to the input sub-system reads
 * right. But still on slow HS insertion, 1 HS button interrupt is serviced.
 * 2. Add DEBOUNCE_TIME as a part of the platform data in the board file to be
 * accessed in the driver - to make it board specific and not driver generic */

/*
 * The gpio_set_debounce expects the debounce argument in micro seconds
 * Previously the kona implementation which is called from gpio_set_debounce
 * was expecting the argument as milliseconds which was incorrect. 
 * The commit 38598aea6cb57290ef6ed3518b75b9f398b7694f fixed it. Hence we have to 
 * change to the correct way of passing the time in microseconds resolution.
 */
#define DEBOUNCE_TIME	(64000)
#define KEY_PRESS_REF_TIME	msecs_to_jiffies(100)
#define KEY_DETECT_DELAY	msecs_to_jiffies(128)
#define ACI_S1  0
#define ACI_T1  0xFE
#define ACI_M1  0x500
#define ACI_MT1 0x400

struct mic_t {
	int hsirq;
	int hsbirq;
	int auxmic_base;
	int aci_base;
	CHAL_HANDLE aci_chal_hdl;
	struct kona_headset_pd *headset_pd;
#ifdef CONFIG_SWITCH
	struct switch_dev sdev;
#endif
	struct work_struct work;
	struct delayed_work input_work;
	struct input_dev *headset_button_idev;
};

static struct mic_t *mic_dev = NULL;

/*------------------------------------------------------------------------------
    Function name   : input_work_func
    Description     : Work function that will send the button press/release
		evetns to the input sub-system.
    Return type     : void
------------------------------------------------------------------------------*/
static void input_work_func(struct delayed_work *work)
{
	struct mic_t *p = container_of(work, struct mic_t, input_work);
	unsigned val;
	static int keystate = 0;

	val = gpio_get_value(irq_to_gpio(p->hsirq));
	val = (val ^ p->headset_pd->hs_default_state);
	/* Send the button press event to the input sub-system. Since
	 * the interrupt for the HS button is only issued on button press,
	 * check the status of the key (press/release) and then send the
	 * event fot the key release */
	if (keystate == 0) {
		if (val) {
			input_report_key(p->headset_button_idev, KEY_SEND, 1);
			input_sync(p->headset_button_idev);
		}
	}
	/*Check if the key is released */
	if (readl(p->aci_base + ACI_COMP_DOUT_OFFSET) &
	    ACI_COMP_DOUT_COMP1_DOUT_CMD_ONE) {
		keystate = 1;
		schedule_delayed_work(&(p->input_work), KEY_PRESS_REF_TIME);
	} else {
		keystate = 0;
		input_report_key(p->headset_button_idev, KEY_SEND, 0);
		input_sync(p->headset_button_idev);
	}
}

/*------------------------------------------------------------------------------
    Function name   : switch_work
    Description     : Work function that will set the state of the headset
		switch dev and enable/disable the HS button interrupt
    Return type     : void
------------------------------------------------------------------------------*/
/* TODO: Remove the prints in the Function once audio driver is implemented */
static void switch_work(struct work_struct *work)
{
	struct mic_t *p = container_of(work, struct mic_t, work);
	unsigned headset_state = gpio_get_value(irq_to_gpio(p->hsirq));

	headset_state = (headset_state ^ p->headset_pd->hs_default_state);

#ifdef CONFIG_SWITCH
	switch_set_state(&(p->sdev), headset_state);
#endif
	/* If the Headset is inserted, enable the ACI interrupt for HS button
	 * else disable it*/
	if (headset_state) {
		pr_info("Headset inserted\n");
		writel((ACI_DSP_INT_COMP1INT_EN_MASK |
			ACI_DSP_INT_COMP1INT_STS_MASK),
		       p->aci_base + ACI_INT_OFFSET);
		enable_irq(p->hsbirq);
	} else {
		pr_info("Headset removed\n");
		writel((~ACI_DSP_INT_COMP1INT_EN_MASK |
			ACI_DSP_INT_COMP1INT_STS_MASK),
		       p->aci_base + ACI_INT_OFFSET);
		disable_irq(p->hsbirq);
	}
}

/*------------------------------------------------------------------------------
    Function name   : hs_unregswitchdev
    Description     : Unregister input device for headset
    Return type     : int
------------------------------------------------------------------------------*/
int hs_unregswitchdev(struct mic_t *p)
{
#ifdef CONFIG_SWITCH
	cancel_work_sync(&p->work);
	switch_dev_unregister(&p->sdev);
#endif
	return 0;
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
	p->sdev.name = "h2w";

	result = switch_dev_register(&p->sdev);
	if (result < 0)
		return result;

	INIT_WORK(&(p->work), switch_work);
	return 0;
}
#endif

/*------------------------------------------------------------------------------
    Function name   : hs_unreginputdev
    Description     : unregister and free the input device for headset button
    Return type     : int
------------------------------------------------------------------------------*/
static int hs_unreginputdev(struct mic_t *p)
{
	cancel_delayed_work_sync(&p->input_work);
	input_unregister_device(p->headset_button_idev);
	input_free_device(p->headset_button_idev);
	return 0;
}

/*------------------------------------------------------------------------------
    Function name   : hs_inputdev
    Description     : Create and Register input device for headset button
    Return type     : int
------------------------------------------------------------------------------*/
static int hs_inputdev(struct mic_t *p)
{
	int result = 0;

	/* Allocate struct for input device */
	p->headset_button_idev = input_allocate_device();
	if ((p->headset_button_idev) == NULL) {
		pr_err("%s: Not enough memory\n", __func__);
		result = -EINVAL;
		goto inputdev_err;
	}

	/* specify key event type and value for it -
	 * Since we have only one button on headset,value KEY_SEND is sent */
	set_bit(EV_KEY, p->headset_button_idev->evbit);
	set_bit(KEY_SEND, p->headset_button_idev->keybit);
	p->headset_button_idev->name = "bcm_headset";
	p->headset_button_idev->phys = "headset/input0";
	p->headset_button_idev->id.bustype = BUS_HOST;
	p->headset_button_idev->id.vendor = 0x0001;
	p->headset_button_idev->id.product = 0x0100;

	/* Register input device for headset */
	result = input_register_device(p->headset_button_idev);
	if (result) {
		pr_err("%s: Failed to register device\n", __func__);
		hs_unreginputdev(p);
		goto inputdev_err;
	}

	INIT_DELAYED_WORK(&(p->input_work), input_work_func);

inputdev_err:
	return result;
}

/*------------------------------------------------------------------------------
    Function name   : hs_isr
    Description     : interrupt handler
    Return type     : irqreturn_t
------------------------------------------------------------------------------*/
irqreturn_t hs_isr(int irq, void *dev_id)
{
	struct mic_t *p = (struct mic_t *)dev_id;

	schedule_work(&(p->work));

	return IRQ_HANDLED;
}

/*------------------------------------------------------------------------------
    Function name   : hsb_isr
    Description     : interrupt handler
    Return type     : irqreturn_t
------------------------------------------------------------------------------*/
irqreturn_t hsb_isr(int irq, void *dev_id)
{
	struct mic_t *p = (struct mic_t *)dev_id;

	/* Enable the ACI interrupt bit and clear the status bit */
	writel((ACI_DSP_INT_COMP1INT_EN_MASK | ACI_DSP_INT_COMP1INT_STS_MASK),
	       p->aci_base + ACI_INT_OFFSET);

	/* The KEY_DETECT_DELAY is being used to overcome the problem where the HSB
	 * ISR is called before the HS ISR when the headset is removed or inserted.
	 * This makes sure that the GPIO status is set right before sending an
	 * event to the input sub-system */
	schedule_delayed_work(&(p->input_work), KEY_DETECT_DELAY);
	return IRQ_HANDLED;
}

static int headset_interface_init (struct mic_t *mic)
{
	if (mic == NULL)
		return -1;

/*
 * IMPORTANT
 * ---------
 * Configuring these AUDIOH MIC registers was required to get ACI interrupts
 * for button press. Really not sure about the connection.
 * But this logic was taken from the BLTS code and if this is not
 * done then we were not getting the ACI interrupts for button press.
 *
 * Looks like if the Audio driver init happens this is not required, in
 * case if Audio driver is not included in the build then this macro should
 * be included to get headset working.
 *
 * Ideally if a macro is used to control brcm audio driver inclusion that does
 * AUDIOH init, then we	 don't need another macro here, it can be something
 * like #ifndef CONFING_BRCM_AUDIOH
 *
 */
#ifdef CONFIG_HS_PERFORM_AUDIOH_SETTINGS 
	/* AUDIO settings */
	writel(AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_SEL_MIC1B_MIC2_MASK,
	       KONA_AUDIOH_VA + AUDIOH_AUDIORX_VRX1_OFFSET);
	writel(0x0, KONA_AUDIOH_VA + AUDIOH_AUDIORX_VRX2_OFFSET);
	writel((AUDIOH_AUDIORX_VREF_AUDIORX_VREF_POWERCYCLE_MASK |
		AUDIOH_AUDIORX_VREF_AUDIORX_VREF_FASTSETTLE_MASK),
	       KONA_AUDIOH_VA + AUDIOH_AUDIORX_VREF_OFFSET);
	writel((AUDIOH_AUDIORX_VMIC_AUDIORX_VMIC_CTRL_MASK |
		AUDIOH_AUDIORX_VMIC_AUDIORX_MIC_EN_MASK),
	       KONA_AUDIOH_VA + AUDIOH_AUDIORX_VMIC_OFFSET);
	writel(AUDIOH_AUDIORX_BIAS_AUDIORX_BIAS_PWRUP_MASK,
	       KONA_AUDIOH_VA + AUDIOH_AUDIORX_BIAS_OFFSET);
#endif
	/* AUXMIC Initialization */
	writel(AUXMIC_PRB_CYC_PROBE_CMD_128MS,
	       mic->auxmic_base + AUXMIC_PRB_CYC_OFFSET);
	writel(AUXMIC_MSR_DLY_MSR_DLY_CMD_32MS,
	       mic->auxmic_base + AUXMIC_MSR_DLY_OFFSET);
	writel(AUXMIC_MSR_INTVL_MSR_INTVL_CMD_128MS,
	       mic->auxmic_base + AUXMIC_MSR_INTVL_OFFSET);
	writel(AUXMIC_CMC_CONT_MSR_CTRL_CMD_PROB_CYC_INF,
	       mic->auxmic_base + AUXMIC_CMC_OFFSET);
	writel(AUXMIC_AUXEN_MICAUX_EN_MASK,
	       mic->auxmic_base + AUXMIC_AUXEN_OFFSET);
	writel((AUXMIC_F_PWRDWN_RESERVED_MASK &
		~(AUXMIC_F_PWRDWN_FORCE_PWR_DWN_MASK)),
	       mic->auxmic_base + AUXMIC_F_PWRDWN_OFFSET);

	/* ACI Initialization */
    	writel(ACI_COMP1MODE_COMP1MODE_CMD_INTEGRATE_AND_DUMP,
	       mic->aci_base + ACI_COMP1MODE_OFFSET);
    	writel(((ACI_S1_RESERVED_MASK & ACI_S1_S1_MASK) | ACI_S1),
	       mic->aci_base + ACI_S1_OFFSET);
    	writel(((ACI_T1_RESERVED_MASK &  ACI_T1_T1_MASK) | ACI_T1),
	       mic->aci_base + ACI_T1_OFFSET);
    	writel(((ACI_M1_RESERVED_MASK & ACI_M1_M1_MASK) | ACI_M1),
	       mic->aci_base + ACI_M1_OFFSET);
    	writel(((ACI_MT1_RESERVED_MASK & ACI_MT1_MT1_MASK) | ACI_MT1),
	       mic->aci_base + ACI_MT1_OFFSET);
 
    	writel(ACI_ACI_CTRL_SW_MIC_DATAB_MASK,
	       mic->aci_base + ACI_ACI_CTRL_OFFSET);
	writel((ACI_ADC_CTRL_AUDIORX_VREF_PWRUP_MASK |
		ACI_ADC_CTRL_AUDIORX_BIAS_PWRUP_MASK),
	       mic->aci_base + ACI_ADC_CTRL_OFFSET);
	writel(ACI_MIC_BIAS_MIC_AUX_BIAS_GND_CMD_IMPEDANCE_CONTROL_HI_Z,
	       mic->aci_base + ACI_MIC_BIAS_OFFSET);
	writel((ACI_DSP_COMPTH_SET_COMPTH1_SET_MASK <<
		ACI_DSP_COMPTH_SET_COMPTH1_SET_SHIFT),
	       mic->aci_base + ACI_COMPTH_SET_OFFSET);

	return 0;
}

/*------------------------------------------------------------------------------
    Function name   : headset_default
    Description     : Hardware initialization sequence
    Return type     : int
------------------------------------------------------------------------------*/
static int headset_default(struct mic_t *mic)
{
	int status = 0;
	unsigned hs_gpio;

	/* Initial settings for GPIO */
	hs_gpio = irq_to_gpio(mic->hsirq);

	/* Request the gpio 
	 * Note that this is an optional call for setting direction/debounce
	 * values. But set debounce will throw out warning messages if we 
	 * call gpio_set_debounce without calling gpio_request. 
	 * Note that it just throws out Warning messages and proceeds
	 * to auto request the same. We are adding this call here to 
	 * suppress the warning message.
	 */
	status = gpio_request (hs_gpio, "hs_detect");
	if (status < 0) {
		pr_err("%s: gpio request failed \r\n", __func__);
		return status;
	}

	/* Set the GPIO debounce */
	status = gpio_set_debounce(hs_gpio, DEBOUNCE_TIME);
	if (status < 0) {
		pr_err("%s: gpio set debounce failed\n", __func__);
		return status;
	}

	/* Set the GPIO direction input */
	status = gpio_direction_input(hs_gpio);
	if (status < 0) {
		pr_err("%s: gpio set direction input failed\n", __func__);
		return status;
	}

	/* Configure AUDIOH CCU for clock policy */
	/* 
	 * Remove the entire Audio CCU config policy settings and AUDIOH
	 * initialization sequence once they are being done as a part of PMU and
	 * audio driver settings
	 */
#ifdef CONFIG_HS_PERFORM_AUDIOH_SETTINGS 
	writel(0xa5a501, KONA_HUB_CLK_VA);
	writel(0x6060606,
	       KONA_HUB_CLK_VA + KHUB_CLK_MGR_REG_POLICY_FREQ_OFFSET);
	writel(0x7fffffff,
	       KONA_HUB_CLK_VA + KHUB_CLK_MGR_REG_POLICY0_MASK1_OFFSET);
	writel(0x7fffffff,
	       KONA_HUB_CLK_VA + KHUB_CLK_MGR_REG_POLICY1_MASK1_OFFSET);
	writel(0x7fffffff,
	       KONA_HUB_CLK_VA + KHUB_CLK_MGR_REG_POLICY2_MASK1_OFFSET);
	writel(0x7fffffff,
	       KONA_HUB_CLK_VA + KHUB_CLK_MGR_REG_POLICY3_MASK1_OFFSET);
	writel(0x7fffffff,
	       KONA_HUB_CLK_VA + KHUB_CLK_MGR_REG_POLICY0_MASK2_OFFSET);
	writel(0x7fffffff,
	       KONA_HUB_CLK_VA + KHUB_CLK_MGR_REG_POLICY1_MASK2_OFFSET);
	writel(0x7fffffff,
	       KONA_HUB_CLK_VA + KHUB_CLK_MGR_REG_POLICY2_MASK2_OFFSET);
	writel(0x7fffffff,
	       KONA_HUB_CLK_VA + KHUB_CLK_MGR_REG_POLICY3_MASK2_OFFSET);
	writel(0x3, KONA_HUB_CLK_VA + KHUB_CLK_MGR_REG_POLICY_CTL_OFFSET);
	writel(0x1ff, KONA_HUB_CLK_VA + KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET);
	writel(0x100A00, KONA_HUB_CLK_VA + 0x104);
	writel(0x40000100, KONA_HUB_CLK_VA + 0x124);
#endif
	headset_interface_init (mic);

	return status;
}

static int hs_remove(struct platform_device *pdev)
{
	struct mic_t *mic;

	mic = platform_get_drvdata(pdev);

	free_irq(mic->hsirq, NULL);
	free_irq(mic->hsbirq, NULL);

	hs_unreginputdev(mic);
	hs_unregswitchdev(mic);

	kfree(mic);
	return 0;
}

static int __init hs_probe(struct platform_device *pdev)
{
	int ret = 0, val;
	struct resource *mem_resource;
	struct mic_t *mic;

	mic = kzalloc(sizeof(struct mic_t), GFP_KERNEL);
	if (!mic)
		return -ENOMEM;

	mic_dev = mic;

	if (pdev->dev.platform_data)
		mic->headset_pd = pdev->dev.platform_data;

	/* Initialize the switch dev for headset */
#ifdef CONFIG_SWITCH
	ret = hs_switchinit(mic);
	if (ret < 0)
		goto err2;
#endif

	/* Initialize a input device for the headset button */
	ret = hs_inputdev(mic);
	if (ret < 0) {
		hs_unregswitchdev(mic);
		goto err2;
	}

	/* Get the Headset and HS button IRQ */
	mic->hsirq = platform_get_irq(pdev, 0);
	if (!mic->hsirq) {
		ret = -EINVAL;
		goto err1;
	}
	pr_info("HS irq %d\n", mic->hsirq);

	mic->hsbirq = platform_get_irq(pdev, 1);
	if (!mic->hsbirq) {
		ret = -EINVAL;
		goto err1;
	}
	pr_info("HSB irq %d\n", mic->hsbirq);

	/* Get the base address for AUXMIC and ACI control registers */
	mem_resource = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mem_resource) {
		ret = -ENXIO;
		goto err1;
	}
	mic->auxmic_base = HW_IO_PHYS_TO_VIRT(mem_resource->start);

	mem_resource = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!mem_resource) {
		ret = -ENXIO;
		goto err1;
	}
	mic->aci_base = HW_IO_PHYS_TO_VIRT(mem_resource->start);

	/* Hardware initialization */
	ret = headset_default(mic);
	if (ret < 0)
		goto err1;

	/* Request the IRQ for Headset */
	ret =
	    request_irq(mic->hsirq, hs_isr,
			(IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING |
			 IRQF_NO_SUSPEND), "KonaHeadset", mic);
	if (ret < 0) {
		pr_err("%s() : Request irq failed for Headset\n", __func__);
		goto err1;
	}

	/* Request the IRQ for HS Button */
	ret =
	    request_irq(mic->hsbirq, hsb_isr, IRQF_NO_SUSPEND,
			"KonaHeadsetButton", mic);
	if (ret < 0) {
		pr_err("%s() : Request irq failed for Headset button\n",
		       __func__);
		/* Free the HS IRQ if the HS Button IRQ request fails */
		free_irq(mic->hsirq, NULL);
		goto err1;
	}

	/* Check the value on the GPIO line to ensure the HS is connected */
	val = gpio_get_value(irq_to_gpio(mic->hsirq));
	val = (val ^ mic->headset_pd->hs_default_state);
	if (val) {
		/* If the headset is not connected on boot, disable the HS
		 * button Interrupt */
		writel((~ACI_DSP_INT_COMP1INT_EN_MASK |
			ACI_DSP_INT_COMP1INT_STS_MASK),
		       mic->aci_base + ACI_INT_OFFSET);
	} else {
		/* If the headset is connected on boot, enable the HS Button
		 * interrupt */
		writel((ACI_DSP_INT_COMP1INT_EN_MASK |
			ACI_DSP_INT_COMP1INT_STS_MASK),
		       mic->aci_base + ACI_INT_OFFSET);
	}

	/* Perform CHAL initialization */
	mic->aci_chal_hdl = chal_aci_init(mic->aci_base);

	/* Store the mic structure data as private driver data for later use */
	platform_set_drvdata(pdev, mic);

	return ret;
err1:
	hs_unregswitchdev(mic);
	hs_unreginputdev(mic);
err2:
	kfree(mic);
	return ret;
}

/*
 * Note that there is a __refdata added to the headset_driver platform driver
 * structure. What is the meaning for it and why its required.
 *
 * The probe function: 
 * From the platform driver documentation its advisable to keep the probe
 * function of a driver in the __init section if the device is NOT hot pluggable. 
 * Note that in headset case even though the headset is hot pluggable, the driver 
 * is not. That is a new node will not be created and the probe will not be called 
 * again. So it makes sense to keep the hs_probe in __init section so as to 
 * reduce the driver's run time foot print.
 * 
 * The Warning message:
 * But since the functions address (reference) is stored in a structure that
 * will be available even after init (in case of remove, suspend etc) there
 * is a Warning message from the compiler
 *
 * The __refdata keyword can be used to suppress this warning message. Tells the
 * compiler not to throw out this warning. And in this scenario even though
 * we store the function pointer from __init section to the platform driver
 * structure that lives after __init, we wont be referring the probe function
 * in the life time until headset_driver lives, so its OK to suppress.
 */
static struct platform_driver __refdata headset_driver = {
	.probe = hs_probe,
	.remove = hs_remove,
	.driver = {
		   .name = "konaheadset",
		   .owner = THIS_MODULE,
		   },
};

/* 
 * sysfs interface implementation to export Head set type 
 * ------------------------------------------------------
 * Using this interface the type of Head set plugged in is exposed
 * to the user space in /sys/hs_type/hs_type_read
 *
 * There are 3 different values that will be displayed by this function
 *  1 - Headphone
 *  2 - Open Cable 
 *  3 - Headset
 *
 * From the user space when some one reads this value using 'cat' command
 * Ex:- cat /sys/hs_type/hs_type_read
 *
 * The function hs_type_detect_func will be invoked. The actual accessory
 * detection algorithm is implemented here.
 * 
 * Why is the detection alogorithm implemented here and not in the 
 * work queue that informs accessory insertion/removal through switch class
 * interface?
 *
 * There are two reasons
 * 1) Wanted to keep the actual accessory detection logic seperate from 
 *    insertion/removal detection to avoid unnecessary complexity. Things
 *    like when we are executing accessory detection logic in work queue
 *    a removal happens etc can be avoided.
 *
 * 2) Standard Android user space code just want to know insertion/removal
 *    of headset and is not worried about the different types. So adding
 *    this algorithm there would include unnecessary processing logic
 */

static struct kobject *hs_type_kobj;

enum hs_type {
	HEADPHONE = 1,
	OPEN_CABLE,
	HEADSET,
	/* If more HS types are required to be added
	 * add here, not below HS_TYPE_MAX
	 */
	HS_TYPE_MAX,
	HEADPHONE_DETECTED,
	OPEN_CABLE_DETECTED,
	HEADSET_DETECTED
};


/* 
 * API controllers
 */
 /*
  * USE_SHARED_MIC_BIAS_CONTROL should be defined if driver
  * should use the shared MIC Bias control function
  */
#define USE_SHARED_MIC_BIAS_CONTROL

/* 
 * Hera AO COMP1 don't support interrupt on both rising and falling edgeds.
 * The plan is to support both edges on Rhea. COMP2 will be used until this 
 * chips is in place. 
 */
#define COMP2_USED_FOR_HOOK_DETECTION

/*
 * After configuring the ADC, it takes different 'time' for the 
 * ADC to settle depending on the HS type. The time outs are 
 * in milli seconds
 */

#define DET_PLUG_INSERTION_SETTLE       250     /* Plug insertion setteling time */
#define DET_PLUG_CONNECTED_SETTLE       80      /* in HAL_ACC_DET_PLUG_CONNECTED */
#define DET_HEADPHONE_SETTLE            5       /* in HAL_ACC_DET_HEADPHONE */
#define DET_OPEN_CABLE_SETTLE           20      /* in HAL_ACC_DET_OPEN_CABLE */
#define DET_BASIC_CARKIT_SETTLE         20      /* in HAL_ACC_DET_BASIC_CARKIT */
#define FINAL_HEADSET_SETTLE            40      /* Headset mode */
#define FINAL_OPENCABLE_SETTLE          40      /* Open Cable mode */

#ifdef COMP2_USED_FOR_HOOK_DETECTION
#define CHAL_ACI_HOOK_DETECTION_BLOCK           CHAL_ACI_BLOCK_COMP2
#define ACC_CHIPSET_HOOK_DETECTION_INTERRUPT    ACC_CHIPSET_INTERRUPT_COMP2
#else
#define CHAL_ACI_HOOK_DETECTION_BLOCK           CHAL_ACI_BLOCK_COMP1
#define ACC_CHIPSET_HOOK_DETECTION_INTERRUPT    ACC_CHIPSET_INTERRUPT_COMP1
#endif

/*
 * Button/Hook Filter configuration
 */
#define ACC_HW_COMP1_FILTER_WIDTH   1024    /* = 1024 / (Filter block frequencey) = 1024 / 32768 => 31ms */


/* 
 * Accessory Detecting voltage
 *
 * Voltage defined in mv 
 */
#define HEADPHONE_DETECT_LEVEL_MIN      0
#define HEADPHONE_DETECT_LEVEL_MAX      40
#define HEADPHONE_DETECT_LEVEL2_MIN     91
#define HEADPHONE_DETECT_LEVEL2_MAX     599

#define OPENCABLE_DETECT_LEVEL_MIN      1900
#define OPENCABLE_DETECT_LEVEL_MAX      5000

#define BASIC_HEADSET_DETECT_LEVEL_MIN  0
#define BASIC_HEADSET_DETECT_LEVEL_MAX  200

/* Accessory Hardware configuration support variables */
/* ADC */
static const CHAL_ACI_filter_config_adc_t aci_filter_adc_config = {0, 0x0B};

/* COMP1 */
static const CHAL_ACI_filter_config_comp_t aci_comp1_config_default = {
	CHAL_ACI_FILTER_MODE_INTEGRATE,
	CHAL_ACI_FILTER_RESET_FIRMWARE,
	0,       /* = S */
	254,     /* = T */
	1280,    /* = M = 1280 / 32768 => 39ms */
	ACC_HW_COMP1_FILTER_WIDTH /* = MT */
};

/* COMP2 */
static const CHAL_ACI_filter_config_comp_t aci_comp2_config_default = {
	CHAL_ACI_FILTER_MODE_INTEGRATE,
	CHAL_ACI_FILTER_RESET_FIRMWARE,
	0,     /* = S  */
	254,   /* = T  */
	1792,  /* = M  */
	1616   /* = MT */
};

/* MIC bias */
static CHAL_ACI_micbias_config_t aci_mic_bias = {
	CHAL_ACI_MIC_BIAS_OFF,
	CHAL_ACI_MIC_BIAS_2_1V,
	CHAL_ACI_MIC_BIAS_PRB_CYC_256MS,
	CHAL_ACI_MIC_BIAS_MSR_DLY_4MS,
	CHAL_ACI_MIC_BIAS_MSR_INTVL_64MS,
	CHAL_ACI_MIC_BIAS_1_MEASUREMENT
};

/* Vref */
static CHAL_ACI_vref_config_t aci_vref_config = {CHAL_ACI_VREF_OFF};

/* Low level functions called to detect the accessory type */

/* TODO: mic bias control function to be implemented */

static int aci_inactivate_detection_hw (void)
{
	if (mic_dev == NULL) {
		pr_err("aci_adc_config: invalid mic_dev handle \r\n");
		return  -EFAULT;
	}

	if (mic_dev->aci_chal_hdl == NULL) {
		pr_err("aci_adc_config: Invalid CHAL handle \r\n");
		return -EFAULT;
	}

	/* TODO: Clear & disable Hook interrupts */

	/* Configure MIC pin bias in powerdown state */
	chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
		CHAL_ACI_BLOCK_ACTION_MIC_POWERDOWN_HIZ_IMPEDANCE,
		CHAL_ACI_BLOCK_GENERIC,TRUE );

#ifdef GLUE_ACCESSORY_USE_SHARED_MIC_BIAS_CONTROL
	/* TODO: implementation not done yet */
#else
        /* Powerdown generic detection block*/
	aci_mic_bias.mode = CHAL_ACI_MIC_BIAS_OFF;
        chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
		CHAL_ACI_BLOCK_ACTION_MIC_BIAS,
		CHAL_ACI_BLOCK_GENERIC, &aci_mic_bias);
#endif

	/* Powerdown ADC1&2 */
	chal_aci_block_ctrl(mic_dev->aci_chal_hdl, 
		CHAL_ACI_BLOCK_ACTION_DISABLE,
		CHAL_ACI_BLOCK_ADC );

	/* Powerdown COMP1&2 */
	chal_aci_block_ctrl(mic_dev->aci_chal_hdl, 
		CHAL_ACI_BLOCK_ACTION_DISABLE,
		CHAL_ACI_BLOCK_COMP );

	chal_aci_block_ctrl(mic_dev->aci_chal_hdl, 
		CHAL_ACI_BLOCK_ACTION_RESET_FILTER,
		CHAL_ACI_BLOCK_COMP );

	/* 
	 * Powerdown digital block (controlled by Measurement Enable signal)
	 */
	 chal_aci_block_ctrl(mic_dev->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_DISABLE,
	 	CHAL_ACI_BLOCK_DIGITAL );

	 /* Powerdown Block Vref */
	 aci_vref_config.mode = CHAL_ACI_VREF_OFF;
	 chal_aci_block_ctrl(mic_dev->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_VREF,
	 	CHAL_ACI_BLOCK_GENERIC, &aci_vref_config);

	chal_aci_set_mic_route (mic_dev->aci_chal_hdl, CHAL_ACI_MIC_ROUTE_ALL_OFF);
	
	return 0;
}

static int aci_hw_config(int hst)
{
	int time_to_settle = 0;

	if (mic_dev == NULL) {
		pr_err("aci_adc_config: invalid mic_dev handle \r\n");
		return  -EFAULT;
	}

	if (mic_dev->aci_chal_hdl == NULL) {
		pr_err("aci_adc_config: Invalid CHAL handle \r\n");
		return -EFAULT;
	}

	switch (hst) {
	case HEADPHONE:

		pr_debug("aci_hw_config: Configuring for headphone \r\n");
		/* Setup MIC bias */
		aci_mic_bias.mode = CHAL_ACI_MIC_BIAS_ON;
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_MIC_BIAS,
			CHAL_ACI_BLOCK_GENERIC, &aci_mic_bias);

		/* TODO: Setup the Interrupt Source, is this required ??? */

		/* Configure MIC Line route */
		chal_aci_set_mic_route (mic_dev->aci_chal_hdl, CHAL_ACI_MIC_ROUTE_MIC);

		/* Power up Digital block */
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_ENABLE, 
			CHAL_ACI_BLOCK_DIGITAL);

		/* Power up the ADC */
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_ENABLE, 
			CHAL_ACI_BLOCK_ADC);

		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_ADC_RANGE,
			CHAL_ACI_BLOCK_ADC, CHAL_ACI_BLOCK_ADC_LOW_VOLTAGE);

		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, 
			CHAL_ACI_BLOCK_ACTION_CONFIGURE_FILTER, CHAL_ACI_BLOCK_ADC,
			&aci_filter_adc_config);

		time_to_settle = DET_PLUG_CONNECTED_SETTLE;
		break;

	case OPEN_CABLE:

		pr_debug("aci_hw_config: Configuring for open cable \r\n");
		/* Configure the MIC line route */
		chal_aci_set_mic_route (mic_dev->aci_chal_hdl, CHAL_ACI_MIC_ROUTE_ACI_OPEN);

		/* Powerup ADC */
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_ENABLE,
			CHAL_ACI_BLOCK_ADC);

		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_ADC_RANGE,
			CHAL_ACI_BLOCK_ADC, CHAL_ACI_BLOCK_ADC_HIGH_VOLTAGE);

		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, 
			CHAL_ACI_BLOCK_ACTION_CONFIGURE_FILTER, CHAL_ACI_BLOCK_ADC,
			&aci_filter_adc_config);

		/* Reset COMP2 */
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
			CHAL_ACI_BLOCK_ACTION_CONFIGURE_FILTER, CHAL_ACI_BLOCK_COMP2,
			&aci_comp2_config_default);

		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
			CHAL_ACI_BLOCK_ACTION_COMP_THRESHOLD, CHAL_ACI_BLOCK_COMP2,
			1900);

		time_to_settle = DET_OPEN_CABLE_SETTLE;
		break;

	case HEADSET:

		pr_debug("aci_hw_config: Configuring for HEADSET \r\n");
		/* Configure the MIC line route */
		chal_aci_set_mic_route (mic_dev->aci_chal_hdl, CHAL_ACI_MIC_ROUTE_MIC);

		/* Turn OFF MIC Bias */
		aci_mic_bias.mode = CHAL_ACI_MIC_BIAS_GND;
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_MIC_BIAS,
			CHAL_ACI_BLOCK_GENERIC, &aci_mic_bias);

		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, 
			CHAL_ACI_BLOCK_ACTION_MIC_POWERDOWN_HIZ_IMPEDANCE,
			CHAL_ACI_BLOCK_GENERIC,
			0);

		/* TODO: Find out whether COMP2_USED_FOR_HOOK_DETECTION should
		 * be defined or not ???
		 */
#ifndef COMP2_USED_FOR_HOOK_DETECTION
		/* Power down COMP2 */
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_DISABLE,
			CHAL_ACI_BLOCK_COMP2);

		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
			CHAL_ACI_BLOCK_ACTION_RESET_FILTER, CHAL_ACI_BLOCK_COMP2);
#endif
		/* Configure COMP1 (0.6V) */
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_ENABLE,
			CHAL_ACI_HOOK_DETECTION_BLOCK);

		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, 
			CHAL_ACI_BLOCK_ACTION_CONFIGURE_FILTER, CHAL_ACI_HOOK_DETECTION_BLOCK, 
			&aci_comp1_config_default);

		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
			CHAL_ACI_BLOCK_ACTION_COMP_THRESHOLD, CHAL_ACI_HOOK_DETECTION_BLOCK,
			600);

		time_to_settle = DET_BASIC_CARKIT_SETTLE;
		break;

	case HEADPHONE_DETECTED:
		pr_debug("aci_hw_config: Head phone detected, powering down the accessory HW and configuring the mic line \r\n");
		/* Configure the MIC line route */
		chal_aci_set_mic_route (mic_dev->aci_chal_hdl, CHAL_ACI_MIC_ROUTE_MIC);

		/* Powerdown Accessory detection Hardware */
		aci_inactivate_detection_hw ();

		break;

	case OPEN_CABLE_DETECTED:
		pr_debug("aci_hw_config: open cable detected, powering down the accessory HW and configuring the mic line \r\n");
		/* Configure the MIC line route */
		chal_aci_set_mic_route (mic_dev->aci_chal_hdl, CHAL_ACI_MIC_ROUTE_MIC);

		/* Powerdown Accessory detection Hardware */
		aci_inactivate_detection_hw ();
		time_to_settle =  FINAL_OPENCABLE_SETTLE;

		break;

	case HEADSET_DETECTED:
		pr_debug("aci_hw_config: headset detected, powering down the accessory HW and configuring the mic line \r\n");
		/* Configure the MIC line route */
		chal_aci_set_mic_route (mic_dev->aci_chal_hdl, CHAL_ACI_MIC_ROUTE_MIC);

		/* Powerdown Accessory detection Hardware */
		aci_inactivate_detection_hw ();
		time_to_settle =  FINAL_HEADSET_SETTLE;
	
		break;
	default:
		/* TODO: Logically what does this mean ???? */
		break;
	}

	/* 
	 * Wait till the ADC settles, the timings varies for different types
	 * of headset. 
	 */
	/* TODO:
	 * There is a settling time after which a call back is
	 * invoked, does this call back trigger any other HW config or
	 * its just a notification to the upper layer ????
	 */
	if (time_to_settle != 0)
		msleep(time_to_settle);

	return 0;
}

static int aci_hw_read(int hst)
{
	int status = -1;
	int mic_level;

	if (mic_dev == NULL) {
		pr_err("aci_adc_config: invalid mic_dev handle \r\n");
		return  -EFAULT;
	}

	if (mic_dev->aci_chal_hdl == NULL) {
		pr_err("aci_adc_config: Invalid CHAL handle \r\n");
		return -EFAULT;
	}

	switch (hst) {
	case HEADPHONE:
		mic_level = chal_aci_block_read(mic_dev->aci_chal_hdl,
			CHAL_ACI_BLOCK_ADC, CHAL_ACI_BLOCK_ADC_RAW);

		if (( mic_level >= HEADPHONE_DETECT_LEVEL_MIN && 
		      mic_level <= HEADPHONE_DETECT_LEVEL_MAX)
		    ||
		    (mic_level >= HEADPHONE_DETECT_LEVEL2_MIN && 
		     mic_level <= HEADPHONE_DETECT_LEVEL2_MAX))
			status = HEADPHONE;		
		break;
	case OPEN_CABLE:
		pr_debug("aci_hw_read: Reading back for Open Cable \r\n");
		mic_level = chal_aci_block_read(mic_dev->aci_chal_hdl,
			CHAL_ACI_BLOCK_ADC, CHAL_ACI_BLOCK_ADC_RAW);

 		if (mic_level >= OPENCABLE_DETECT_LEVEL_MIN && 
		    mic_level <= OPENCABLE_DETECT_LEVEL_MAX)
			status = OPEN_CABLE;
		break;
	case HEADSET:
		pr_debug("aci_hw_read: Reading back for Headset \r\n");
		mic_level = chal_aci_block_read(mic_dev->aci_chal_hdl,
			CHAL_ACI_BLOCK_ADC, CHAL_ACI_BLOCK_ADC_RAW);

		if (mic_level >= BASIC_HEADSET_DETECT_LEVEL_MIN && 
		    mic_level <= BASIC_HEADSET_DETECT_LEVEL_MAX)
			status = HEADSET;
		break;
	default:
		/* TODO: Logically what does this mean ???? */
		break;
	}

	pr_debug("status is %d mic_level is %d \r\n", status, mic_level);
	return status;
}

/* Function that is invoked when the user tries to read from sysfs interface */
static ssize_t hs_type_detect_func (struct device *dev, 
	struct device_attribute *attr, char *buf)
{
	int i;

	if (mic_dev == NULL) {
		pr_err("mic_dev is empty \r\n");
		return 0;
	}

	/* 
	 * The ACI interrupt should be disabled until the detection process is
	 * over 
	 */
	disable_irq(mic_dev->hsbirq);


	/* 
	 * Configure the ACI block of ADC for detecting a particular type of
	 * accessory
	 */
	for (i=HEADPHONE;i<HS_TYPE_MAX;i++) {
		/* Configure the ADC to check a given HS type */
		aci_hw_config(i);

		/* 
		 * Now, read back to check whether the given accessory is
		 * present. If yes, then break the loop. If not, continue
		 */
		if (aci_hw_read(i) == i) {

/* 
 * CONFIG_ACI_PWRDN_AFTER_CHECK
 * ---------------------
 * DO NOT Define this macro. 
 * With the argument passed to aci_hw_config, it internally calls
 * aci_inactivate_detection function which turns off the COMP1,2 ADC1,2 and
 * also the Digital block. This leads to button presses not getting detected.
 * So we need to narrow down on what block is needed for button press and
 * should not be turning it OFF once a accessory type is detected. Until we
 * find that lets keep all the blocks powerd ON, after accessory detection.
 */
#ifdef CONFIG_ACI_PWRDN_AFTER_CHECK
			/* 
			 * Reconfigure the MIC BIAS and 
			 * Power down the ADCs that are powered up by aci_hw_config 
			 */
			aci_hw_config(i+HS_TYPE_MAX);
#endif

			/* 
			 * Re-initialize the HW regs as done during init.
			 * So, if some register settings were changed
			 * un-intentionally this will put it back to the
			 * normal state
			 */
			headset_interface_init (mic_dev);

			sprintf(buf,"%d",i);
			printk("%s\r\n", buf);
			break;
		} 

	} /* end of loop to check the devices */

	/* Re-enable the ACI(button) interrupts */
	enable_irq(mic_dev->hsbirq);

	/* TODO: If none of the 3 types are detected, what should we do ???? */
	return 0;
}

static DEVICE_ATTR(hs_type_read, 0666, hs_type_detect_func, NULL);

static struct attribute *hs_type_attrs[] = {
	&dev_attr_hs_type_read.attr,
	NULL,
};

static struct attribute_group hs_type_attr_group = {
	.attrs = hs_type_attrs,
};

/*------------------------------------------------------------------------------
    Function name   : kona_hs_module_init
    Description     : Initialize the driver
    Return type     : int
------------------------------------------------------------------------------*/
int __init kona_hs_module_init(void)
{
	int ret;

	/* First register the platform driver */
 	ret = platform_driver_register(&headset_driver);
	if (ret) {
		pr_err ("kona_hs_module_init: platform driver registration failed \r\n");
		return ret;
	}

	/* Now add the sysfs interface for the hs type */
	hs_type_kobj = kobject_create_and_add("hs_type", NULL);
	if (!hs_type_kobj) {
		pr_err ("kona_hs_module_init: unable to add the kobject	\r\n");
		return -ENOMEM;
	}

	return sysfs_create_group(hs_type_kobj, &hs_type_attr_group);
}

/*------------------------------------------------------------------------------
    Function name   : kona_hs_module_exit
    Description     : clean up
    Return type     : int
------------------------------------------------------------------------------*/
void __exit kona_hs_module_exit(void)
{
	sysfs_remove_group(hs_type_kobj, &hs_type_attr_group);
	return platform_driver_unregister(&headset_driver);
}

module_init(kona_hs_module_init);
module_exit(kona_hs_module_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Headset plug and button detection");
