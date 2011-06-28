/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/misc/bcm59055-adc.c
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

/*
*
*****************************************************************************
*
*  bcm59055-adc.c
*
*  PURPOSE:
*
*     This implements the driver for the SAR ADC on BCM59055 PMU chip.
*
*  NOTES:
*
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/power_supply.h>
#include <linux/workqueue.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif
#include <mach/irqs.h>
#include <linux/stringify.h>
#include <linux/broadcom/bcm59055-adc.h>


enum {
	SARADC_CONTINUOUS_MODE,
	SARADC_RTM_MODE,
};

struct bcm59055_saradc {
	struct bcm590xx *bcm59055;
	saradc_rtm_callback_handler handler;
	int channel;
	int mode;
	bool upper_bound;
	bool gsm_deb;
	bool ntc_block;
	struct mutex lock;
	void *param;
};

static struct bcm59055_saradc *bcm59055_saradc;

DECLARE_WAIT_QUEUE_HEAD(gAdcNoRTMQueue);

/* ---- Private Function Prototypes -------------------------------------- */

/* ---- Functions -------------------------------------------------------- */

/******************************************************************************
*
* Function Name: bcm59055_saradc_start_burst_mode
*
* Description: Called to start the ADC brust mode. This can be called when
				charger is inserted to perform the conversions wihtout any gap
* Return:
******************************************************************************/

int bcm59055_saradc_start_burst_mode(void)
{
	struct bcm590xx *bcm59055 = bcm59055_saradc->bcm59055;
	u8 regVal;
	pr_info("inside %s\n", __func__);

	if (bcm59055_saradc->gsm_deb) {
		pr_info("%s: GSM Brust mode is already on\n", __func__);
		return 0;
	}
	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_ADCCTRL2);
	regVal |= BCM59055_ADCCTRL2_GSM_DEBOUNCE;
	bcm59055_saradc->gsm_deb = 1;
	return bcm590xx_reg_write(bcm59055, BCM59055_REG_ADCCTRL2, regVal);
}

EXPORT_SYMBOL(bcm59055_saradc_start_burst_mode);

/******************************************************************************
*
* Function Name: bcm59055_saradc_stop_burst_mode
*
* Description: Called to stop the ADC brust mode
* Return:	   -EPERM if already stopped or 0 on success
******************************************************************************/

int bcm59055_saradc_stop_burst_mode(void)
{
	u8 regVal;
	struct bcm590xx *bcm59055 = bcm59055_saradc->bcm59055;
	pr_info("inside %s\n", __func__);
	if (!bcm59055_saradc->gsm_deb) {
		pr_info("%s: GSM Brust mode is already off\n", __func__);
		return 0;
	}
	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_ADCCTRL2);
	regVal &= ~BCM59055_ADCCTRL2_GSM_DEBOUNCE;
	bcm59055_saradc->gsm_deb = 0;
	return bcm590xx_reg_write(bcm59055, BCM59055_REG_ADCCTRL2, regVal);
}

EXPORT_SYMBOL(bcm59055_saradc_stop_burst_mode);

/******************************************************************************
*
* Function Name: bcm59055_saradc_enable_ntc_block
*
* Description: Called to start all conversion powered by NTC_BIAS_LDO
				Can be called when charger is inserted
				normally can be stopped as these conversions are not required
* Return:
******************************************************************************/

int bcm59055_saradc_enable_ntc_block(void)
{
	u8 regVal;
	struct bcm590xx *bcm59055 = bcm59055_saradc->bcm59055;
	pr_info("inside %s\n", __func__);
	if (bcm59055_saradc->ntc_block) {
		pr_info("%s: NTC Block is already enabled\n", __func__);
		return 0;
	}
	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_CMPCTRL12);
	regVal |= BCM59055_CMPCTRL12_NTCON;
	bcm59055_saradc->ntc_block = 1;
	return bcm590xx_reg_write(bcm59055, BCM59055_REG_CMPCTRL12, regVal);
}

EXPORT_SYMBOL(bcm59055_saradc_enable_ntc_block);

/******************************************************************************
*
* Function Name: bcm59055_saradc_disable_ntc_block
*
* Description: Called to block all conversion powered by NTC_BIAS_LDO
* Return:
******************************************************************************/

int bcm59055_saradc_disable_ntc_block(void)
{
	u8 regVal;
	struct bcm590xx *bcm59055 = bcm59055_saradc->bcm59055;
	pr_info("inside %s\n", __func__);
	if (!bcm59055_saradc->ntc_block) {
		pr_info("%s: NTC Block is already disabled\n", __func__);
		return 0;
	}
	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_CMPCTRL12);
	regVal &= ~BCM59055_CMPCTRL12_NTCON;
	bcm59055_saradc->ntc_block = 0;
	return bcm590xx_reg_write(bcm59055, BCM59055_REG_CMPCTRL12, regVal);
}

EXPORT_SYMBOL(bcm59055_saradc_disable_ntc_block);

/******************************************************************************
*
* Function Name: bcm59055_saradc_read_data
*
* Description: Called to read perticular ADC register
* Return:	   return 10-bit ADC data or ERROR
******************************************************************************/

int bcm59055_saradc_read_data(int sel)
{
	u16 adcData = 0;
	u8 regVal;
	int regD1, regD2;
	struct bcm590xx *bcm59055 = bcm59055_saradc->bcm59055;
	pr_debug("inside %s: Channel %d\n", __func__, sel);

	switch (sel) {
	case ADC_VMBAT_CHANNEL:
		regD1 = BCM59055_REG_ADCCTRL3;
		regD2 = BCM59055_REG_ADCCTRL4;
		break;
	case ADC_VBBAT_CHANNEL:
		regD1 = BCM59055_REG_ADCCTRL5;
		regD2 = BCM59055_REG_ADCCTRL6;
		break;
	case ADC_VWALL_CHANNEL:
		regD1 = BCM59055_REG_ADCCTRL7;
		regD2 = BCM59055_REG_ADCCTRL8;
		break;
	case ADC_VBUS_CHANNEL:
		regD1 = BCM59055_REG_ADCCTRL9;
		regD2 = BCM59055_REG_ADCCTRL10;
		break;
	case ADC_ID_CHANNEL:
		regD1 = BCM59055_REG_ADCCTRL11;
		regD2 = BCM59055_REG_ADCCTRL12;
		break;
	case ADC_NTC_CHANNEL:
		regD1 = BCM59055_REG_ADCCTRL13;
		regD2 = BCM59055_REG_ADCCTRL14;
		break;
	case ADC_BSI_CHANNEL:
		regD1 = BCM59055_REG_ADCCTRL15;
		regD2 = BCM59055_REG_ADCCTRL16;
		break;
	case ADC_BOM_CHANNEL:
		regD1 = BCM59055_REG_ADCCTRL17;
		regD2 = BCM59055_REG_ADCCTRL18;
		break;
	case ADC_32KTEMP_CHANNEL:
		regD1 = BCM59055_REG_ADCCTRL19;
		regD2 = BCM59055_REG_ADCCTRL20;
		break;
	case ADC_PATEMP_CHANNEL:
		regD1 = BCM59055_REG_ADCCTRL21;
		regD2 = BCM59055_REG_ADCCTRL22;
		break;
	case ADC_ALS_CHANNEL:
		regD1 = BCM59055_REG_ADCCTRL23;
		regD2 = BCM59055_REG_ADCCTRL24;
		break;
	default:
		return -EINVAL;
	}
	/* Read ADC data register according to the BCM59055_SARADC_SELECT */
	regVal = bcm590xx_reg_read(bcm59055, regD1);

	if (regVal & BCM59055_ADCDATA_INVALID) {
		pr_info("bcm59055_saradc_read_data: Invalid data\n");
		return -EFAULT;	/* Current data is invalid user should call read again */
	}
	adcData = regVal;
	adcData <<= 8;
	regVal = bcm590xx_reg_read(bcm59055, regD2);
	adcData |= regVal;

	return adcData;
}

EXPORT_SYMBOL(bcm59055_saradc_read_data);

/******************************************************************************
*
* Function Name: bcm59055_saradc_request_rtm
*
* Description: Called to start Real Time Measurement, callback to be registered
*	       Once conversion is done, callback will be called with ADC data.
* Return:
******************************************************************************/

int bcm59055_saradc_request_rtm(saradc_rtm_callback_handler cb_ptr,
				int ch_sel, void *data)
{
	u8 regVal;
	int ret;
	struct bcm590xx *bcm59055 = bcm59055_saradc->bcm59055;
	pr_info("inside %s\n", __func__);
	mutex_lock(&bcm59055_saradc->lock);

	if (bcm59055_saradc->mode == SARADC_RTM_MODE) {
		pr_info("%s: MODE is already in RTM\n", __func__);
		return -EPERM;
	}
	if (!cb_ptr) {
		pr_info("%s: No Callback pointer registered\n", __func__);
		return -EINVAL;
	}
	if ((ch_sel < 0) && (ch_sel >= ADC_NULL_CHANNEL)) {
		pr_info("%s: Wrong channel passed\n", __func__);
		return -EINVAL;
	}

	bcm59055_saradc->mode = SARADC_RTM_MODE;
	bcm59055_saradc->handler = cb_ptr;
	bcm59055_saradc->channel = ch_sel;
	bcm59055_saradc->param = data;

	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_ADCCTRL1);
	regVal &= ~(BCM59055_ADCCTRL1_RTM_CH_MASK <<
				BCM59055_ADCCTRL1_RTM_CH_MASK_SHIFT);	/* Clear Channel Select */
	regVal |=
	    (BCM59055_ADCCTRL1_RTM_ENABLE |
	     (ch_sel & BCM59055_ADCCTRL1_RTM_CH_MASK));
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_ADCCTRL1, regVal); /* set channel and enable RTM req */
	if (ret)
		return ret;
	regVal |= BCM59055_ADCCTRL1_RTM_START;

	bcm590xx_enable_irq(bcm59055, BCM59055_IRQID_INT9_RTM_DATA_RDY);	/* Enable RTM DATA RDY INT */
	bcm590xx_enable_irq(bcm59055, BCM59055_IRQID_INT9_RTM_DURING_CON_MEAS);	/* Enable RTM WHILE CONT INT */
	bcm590xx_enable_irq(bcm59055, BCM59055_IRQID_INT9_RTM_UPPER_BOUND);	/* Enable RTM UPPER BOUND RDY INT */
	return bcm590xx_reg_write(bcm59055, BCM59055_REG_ADCCTRL1, regVal);	/* Start RTM Conversion */
}

EXPORT_SYMBOL(bcm59055_saradc_request_rtm);

/******************************************************************************
*
* Function Name: bcm59055_saradc_set_rtm_delay
*
* Description: Called to set the RTM delay
* Return:	0 on Success or Error value
******************************************************************************/

int bcm59055_saradc_set_rtm_delay(int delay)
{
	u8 regVal;
	struct bcm590xx *bcm59055 = bcm59055_saradc->bcm59055;
	pr_info("inside %s\n", __func__);

	if ((delay > 0) && (delay >= ADCCTRL2_RTM_DELAY_MAX))
		return -EINVAL;

	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_ADCCTRL2);
	regVal &= ~BCM59055_ADCCTRL2_RTM_DLY_MASK;	/* Clear the delay */
	regVal |= delay;

	return bcm590xx_reg_write(bcm59055, BCM59055_REG_ADCCTRL2, regVal);
}

EXPORT_SYMBOL(bcm59055_saradc_set_rtm_delay);

/******************************************************************************
*
* Function Name: bcm59055_saradc_send_rtm_data
*
* Description: Send the RTM data back to caller
******************************************************************************/

static void bcm59055_saradc_send_rtm_data(struct bcm590xx *bcm59055, int ch_sel)
{
	u16 regVal;
	u16 adcData;

	pr_info("inside %s\n", __func__);
	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_ADCCTRL26);

	adcData = regVal;
	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_ADCCTRL25);

	adcData |= (regVal << 8);

	if (bcm59055_saradc->handler)
		bcm59055_saradc->handler(bcm59055_saradc->param, ch_sel,
					 adcData);
}

/******************************************************************************
*
* Function Name: bcm59055_saradc_isr
*
* Description: Interrupt Handler
******************************************************************************/

static void bcm59055_saradc_isr(int intr, void *data)
{
	struct bcm59055_saradc *saradc = data;
	struct bcm590xx *bcm59055 = saradc->bcm59055;

	switch (intr) {
	case BCM59055_IRQID_INT9_RTM_DATA_RDY:
		saradc->mode = SARADC_CONTINUOUS_MODE;
		pr_info("%s: RTM_DATA_RDY INT\n", __func__);

		bcm59055_saradc_send_rtm_data(bcm59055, saradc->channel);
		saradc->channel = ADC_NULL_CHANNEL;
		bcm590xx_disable_irq(bcm59055, BCM59055_IRQID_INT9_RTM_DATA_RDY);	/* Disable RTM DATA RDY INT */
		bcm590xx_disable_irq(bcm59055, BCM59055_IRQID_INT9_RTM_DURING_CON_MEAS);	/* Enable RTM WHILE CONT INT */
		bcm590xx_disable_irq(bcm59055, BCM59055_IRQID_INT9_RTM_UPPER_BOUND);	/* Enable RTM UPPER BOUND RDY INT */
		mutex_unlock(&saradc->lock);
		/* wait 40ms to complete one continuous conversion
		 * before taking any more RTM request
		*/
		if (saradc->upper_bound) {
			interruptible_sleep_on_timeout(&gAdcNoRTMQueue, 40 * HZ / 1000);
			saradc->upper_bound = 0;
		}
		break;
	case BCM59055_IRQID_INT9_RTM_DURING_CON_MEAS:
		pr_info("%s: BCM59055_IRQID_INT9_RTM_DURING_CON_MEAS\n", __func__);
		break;
	case BCM59055_IRQID_INT9_RTM_UPPER_BOUND:
		pr_info("%s: BCM59055_IRQID_INT9_RTM_UPPER_BOUND\n",
			__func__);
		saradc->upper_bound = 1;
		break;
	case BCM59055_IRQID_INT9_RTM_IGNORE:
		pr_info("%s: BCM59055_IRQID_INT9_RTM_IGNORE\n", __func__);
		/* This INT shouldn't be coming, taken care in driver code */
		break;
	case BCM59055_IRQID_INT9_RTM_OVERRIDDEN:
		pr_info("%s: BCM59055_IRQID_INT9_RTM_OVERRIDDEN\n",
			__func__);
		/* This INT shouldn't be coming, taken care in driver code */
		break;
	default:
		return;
	}
}

/******************************************************************************
*
* Function Name: bcm59055_saradc_probe
*
* Description: Called to perform module initialization when the module is loaded.
*s
******************************************************************************/
static int __devinit bcm59055_saradc_probe(struct platform_device *pdev)
{
	struct bcm59055_saradc *saradc;
	struct bcm590xx *bcm59055 = dev_get_drvdata(pdev->dev.parent);
	u8 regVal;

	pr_info("BCM59055 SARADC Driver 1.0\n");
	/* Allocate Memory */
	saradc = kzalloc(sizeof(struct bcm59055_saradc), GFP_KERNEL);
	if (!saradc) {
		pr_info("%s: Memory can not be allocated!!\n",
			__func__);
		return -ENOMEM;
	}
	mutex_init(&saradc->lock);
	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_ADCCTRL1);
	printk("%s: ADCCTRL1 = 0%x\n", __func__, regVal);
	regVal &= BCM59055_ADCCTRL1_RTM_DISABLE;	/* Set RTM_mask=0. No RTM request will be taken */
	regVal |= ADCCTRL1_RESET_COUNT_7;	/* Set Max Reset Count to 7 */
	bcm590xx_reg_write(bcm59055, BCM59055_REG_ADCCTRL1, regVal);

	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_ADCCTRL2);
	printk("%s: ADCCTRL2 = 0%x\n", __func__, regVal);
	regVal &= ~BCM59055_ADCCTRL2_GSM_DEBOUNCE;	/* Set GSM_DEBOUNCE as 0 to stop ADC being always ON */
	bcm590xx_reg_write(bcm59055, BCM59055_REG_ADCCTRL2, regVal);

	/* Request for IRQs */
	bcm590xx_request_irq(bcm59055, BCM59055_IRQID_INT9_RTM_DATA_RDY, false,
			     bcm59055_saradc_isr, saradc);
	bcm590xx_request_irq(bcm59055, BCM59055_IRQID_INT9_RTM_DURING_CON_MEAS, false,
			     bcm59055_saradc_isr, saradc);
	bcm590xx_request_irq(bcm59055, BCM59055_IRQID_INT9_RTM_UPPER_BOUND, false,
			     bcm59055_saradc_isr, saradc);
	bcm590xx_request_irq(bcm59055, BCM59055_IRQID_INT9_RTM_IGNORE, false,
			     bcm59055_saradc_isr, saradc);
	bcm590xx_request_irq(bcm59055, BCM59055_IRQID_INT9_RTM_OVERRIDDEN, false,
			     bcm59055_saradc_isr, saradc);

	/* Check if NTC is enabled */
	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_CMPCTRL12);
	if (regVal & BCM59055_CMPCTRL12_NTCON)
		saradc->ntc_block = 1;
	else
		saradc->ntc_block = 0;
	saradc->bcm59055 = bcm59055;
	saradc->mode = SARADC_CONTINUOUS_MODE;	/* Continuous mode always */
	saradc->upper_bound = 0;
	saradc->gsm_deb = 0;
	bcm59055_saradc = saradc;
	return 0;
}

static int __devexit bcm59055_saradc_remove(struct platform_device *pdev)
{
	struct bcm59055_saradc *saradc = platform_get_drvdata(pdev);
	struct bcm590xx *bcm59055 = saradc->bcm59055;
	if (saradc) {
		bcm590xx_free_irq(bcm59055, BCM59055_IRQID_INT9_RTM_DATA_RDY);
		bcm590xx_free_irq(bcm59055, BCM59055_IRQID_INT9_RTM_DURING_CON_MEAS);
		bcm590xx_free_irq(bcm59055, BCM59055_IRQID_INT9_RTM_UPPER_BOUND);
		bcm590xx_free_irq(bcm59055, BCM59055_IRQID_INT9_RTM_IGNORE);
		bcm590xx_free_irq(bcm59055, BCM59055_IRQID_INT9_RTM_OVERRIDDEN);

		kfree(saradc);
	}
	return 0;
}

struct platform_driver saradc_driver = {
	.probe = bcm59055_saradc_probe,
	.remove = __devexit_p(bcm59055_saradc_remove),
	.driver = {
		   .name = "bcm590xx-saradc",
		   }
};

/****************************************************************************
*
*  bcm59055_saradc_init
*
*     Called to perform module initialization when the module is loaded
*
***************************************************************************/

static int __init bcm59055_saradc_init(void)
{
	platform_driver_register(&saradc_driver);
	/* initialize semaphore for ADC access control */
	return 0;
}				/* bcm59055_saradc_init */

/****************************************************************************
*
*  bcm59055_saradc_exit
*
*       Called to perform module cleanup when the module is unloaded.
*
***************************************************************************/

static void __exit bcm59055_saradc_exit(void)
{
	platform_driver_unregister(&saradc_driver);

}				/* bcm59055_saradc_exit */

subsys_initcall(bcm59055_saradc_init);
module_exit(bcm59055_saradc_exit);

MODULE_AUTHOR("TKG");
MODULE_DESCRIPTION("BCM59055 ADC Driver");
