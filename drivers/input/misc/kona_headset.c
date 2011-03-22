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
#include <mach/rdb/brcm_rdb_sysmap_a9.h>
#include <mach/rdb/brcm_rdb_auxmic.h>
#include <mach/rdb/brcm_rdb_audioh.h>
#include <mach/rdb/brcm_rdb_khub_clk_mgr_reg.h>

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

#define DEBOUNCE_TIME	64
#define KEY_PRESS_REF_TIME	msecs_to_jiffies(100)
#define KEY_DETECT_DELAY	msecs_to_jiffies(128)

struct mic_t {
	int hsirq;
	int hsbirq;
	int auxmic_base;
	int aci_base;
	struct kona_headset_pd *headset_pd;
#ifdef CONFIG_SWITCH
	struct switch_dev sdev;
#endif
	struct work_struct work;
	struct delayed_work input_work;
	struct input_dev *headset_button_idev;
};

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

/*------------------------------------------------------------------------------
    Function name   : headset_default
    Description     : Hardware initialization sequence
    Return type     : int
------------------------------------------------------------------------------*/
static int headset_default(struct platform_device *pdev, struct mic_t *mic)
{
	int status = 0;
	unsigned hs_gpio;

	/* Initial settings for GPIO */
	/* Set the GPIO debounce */
	hs_gpio = irq_to_gpio(mic->hsirq);
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
	/* TODO: Remove the entire Audio CCU config policy settings and AUDIOH
	 * initialization sequence once they are being done as a part of PMU and
	 * audio driver settings*/
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
	ret = headset_default(pdev, mic);
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

static struct platform_driver headset_driver = {
	.probe = hs_probe,
	.remove = hs_remove,
	.driver = {
		   .name = "konaheadset",
		   .owner = THIS_MODULE,
		   },
};

/*------------------------------------------------------------------------------
    Function name   : kona_hs_module_init
    Description     : Initialize the driver
    Return type     : int
------------------------------------------------------------------------------*/
int __init kona_hs_module_init(void)
{
	return platform_driver_register(&headset_driver);
}

/*------------------------------------------------------------------------------
    Function name   : kona_hs_module_exit
    Description     : clean up
    Return type     : int
------------------------------------------------------------------------------*/
void __exit kona_hs_module_exit(void)
{
	return platform_driver_unregister(&headset_driver);
}

module_init(kona_hs_module_init);
module_exit(kona_hs_module_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Headset plug and button detection");
