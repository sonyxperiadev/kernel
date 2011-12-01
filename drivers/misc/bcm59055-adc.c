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
#include <linux/completion.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif
#include <mach/irqs.h>
#include <linux/stringify.h>
#include <linux/broadcom/bcm59055-adc.h>
#include <linux/broadcom/bcmtypes.h>

struct bcm59055_saradc {
	struct bcm590xx *bcm59055;
	int mode;
	int rtm_dly_usec;
	bool rtm_ignored;
	bool rtm_overridden;
	struct mutex lock;
	struct completion rtm_req;
};

static struct bcm59055_saradc *bcm59055_saradc;

#define CLOCK_CYCLE_IN_USEC		32
#define CYCLE_TO_COMPLETE_I2C_RTM	23
#define DELTA_CYCLE_I2C_RTM		1250 /* 1250 cycles seems to cover a correctly working ADC */
#define CYCLE_TO_COMPLETE_ADCSYN_RTM	18
#define DELTA_CYCLE_ADCSYN_RTM		100 /* This one must have max timing for delay between two CP wakeup evetns */
#define INT_REG_READ_TIME_IN_USEC	63
/* 23 RTC clock cycle is taken by a RTM req without delay
 * taken in consideration.
*/
#define I2C_RTM_TIME			(usecs_to_jiffies((CYCLE_TO_COMPLETE_I2C_RTM + DELTA_CYCLE_I2C_RTM) * (CLOCK_CYCLE_IN_USEC)))
#define I2C_RTM_EXECUTION_TIMEOUT	((I2C_RTM_TIME) + usecs_to_jiffies(bcm59055_saradc->rtm_dly_usec + INT_REG_READ_TIME_IN_USEC))
#define ADCSYN_RTM_TIME			(usecs_to_jiffies((CYCLE_TO_COMPLETE_ADCSYN_RTM + DELTA_CYCLE_ADCSYN_RTM) * (CLOCK_CYCLE_IN_USEC)))
#define ADCSYN_RTM_EXECUTION_TIMEOUT	((ADCSYN_RTM_TIME) + usecs_to_jiffies(bcm59055_saradc->rtm_dly_usec + INT_REG_READ_TIME_IN_USEC))


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
	struct bcm590xx *bcm59055;
	u8 regVal;
	int ret;
	pr_info("inside %s\n", __func__);
	if(!bcm59055_saradc || !bcm59055_saradc->bcm59055)
		return -EPERM;
	mutex_lock(&bcm59055_saradc->lock);
	bcm59055 = bcm59055_saradc->bcm59055;

	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_ADCCTRL2);
	regVal |= BCM59055_ADCCTRL2_GSM_DEBOUNCE;
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_ADCCTRL2, regVal);
	mutex_unlock(&bcm59055_saradc->lock);
	return ret;
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
	int ret;
	struct bcm590xx *bcm59055;
	pr_info("inside %s\n", __func__);
	if(!bcm59055_saradc || !bcm59055_saradc->bcm59055)
		return -EPERM;
	mutex_lock(&bcm59055_saradc->lock);
	bcm59055 = bcm59055_saradc->bcm59055;

	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_ADCCTRL2);
	regVal &= ~BCM59055_ADCCTRL2_GSM_DEBOUNCE;
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_ADCCTRL2, regVal);
	mutex_unlock(&bcm59055_saradc->lock);
	return ret;
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
	int ret;
	struct bcm590xx *bcm59055;
	pr_info("inside %s\n", __func__);
	if(!bcm59055_saradc || !bcm59055_saradc->bcm59055)
		return -EPERM;
	mutex_lock(&bcm59055_saradc->lock);
	bcm59055 = bcm59055_saradc->bcm59055;

	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_CMPCTRL12);
	regVal |= BCM59055_CMPCTRL12_NTCON;
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_CMPCTRL12, regVal);
	mutex_unlock(&bcm59055_saradc->lock);
	return ret;
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
	int ret;
	struct bcm590xx *bcm59055;
	pr_info("inside %s\n", __func__);
	if(!bcm59055_saradc || !bcm59055_saradc->bcm59055)
		return -EPERM;
	mutex_lock(&bcm59055_saradc->lock);
	bcm59055 = bcm59055_saradc->bcm59055;

	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_CMPCTRL12);
	regVal &= ~BCM59055_CMPCTRL12_NTCON;
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_CMPCTRL12, regVal);
	mutex_unlock(&bcm59055_saradc->lock);
	return ret;
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
	struct bcm590xx *bcm59055;
	pr_info("inside %s: Channel %d\n", __func__, sel);
	if(!bcm59055_saradc || !bcm59055_saradc->bcm59055)
		return -EPERM;
	mutex_lock(&bcm59055_saradc->lock);
	bcm59055 = bcm59055_saradc->bcm59055;

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
		mutex_unlock(&bcm59055_saradc->lock);
		return -EPERM;
	}
	/* Read ADC data register according to the BCM59055_SARADC_SELECT */
	regVal = bcm590xx_reg_read(bcm59055, regD1);

	if (regVal & BCM59055_ADCDATA_INVALID) {
		pr_debug("bcm59055_saradc_read_data: Invalid data\n");
		mutex_unlock(&bcm59055_saradc->lock);
		return -EINVAL;	/* Current data is invalid user should call read again */
	}
	adcData = regVal;
	adcData <<= 8;
	regVal = bcm590xx_reg_read(bcm59055, regD2);
	adcData |= regVal;
	mutex_unlock(&bcm59055_saradc->lock);
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

int bcm59055_saradc_request_rtm(int ch_sel)
{
	return bcm59055_saradc_rtm_read(ch_sel, FALSE, ADCCTRL2_RTM_DELAY_468_75_USEC);
}

EXPORT_SYMBOL(bcm59055_saradc_request_rtm);

/******************************************************************************
 * *
 * * Function Name: bcm59055_saradc__rtm_read
 * *
 * * Description: Called to start Real Time Measurement either via I2C or ADC_SYNC.
 * * User for RTM data, must call this API even if they are waiting for ADC_SYNC
 * * to trigger the RTM convertion. So, that they get notified on RTM_DATA_RDY
 * * interrpt as well ADC driver will have the knowledge about ADC_SYNC triggered
 * * RTM conversion.
 * * Exception: If any RTM conversion is going on and that time ADC_SYNC trigger
 * * another RTM conversion, current request will be overridden, and -EIO will be
 * * returned. User must re-request after sometimes.
 * * Return:
 * ******************************************************************************/

int bcm59055_saradc_rtm_read(int ch_sel, bool adc_sync, u32 delay)
{
	u8 regVal;
	u16 adcData;
	int ret;
	unsigned long time_left;
	struct bcm590xx *bcm59055;
	pr_debug("inside %s\n", __func__);
	if(!bcm59055_saradc || !bcm59055_saradc->bcm59055)
		return -EPERM;
	bcm59055 = bcm59055_saradc->bcm59055;


	if ((ch_sel < 0) || (ch_sel >= ADC_NULL_CHANNEL)) {
		pr_info("%s: Wrong channel passed\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&bcm59055_saradc->lock);
	INIT_COMPLETION(bcm59055_saradc->rtm_req);
	/* set the delay */
	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_ADCCTRL2);
	regVal &= ~BCM59055_ADCCTRL2_RTM_DLY_MASK;	/* Clear the delay */
	regVal |= (delay & BCM59055_ADCCTRL2_RTM_DLY_MASK);
	bcm59055_saradc->rtm_dly_usec = delay * CLOCK_CYCLE_IN_USEC;
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_ADCCTRL2, regVal);
	if (ret) {
		mutex_unlock(&bcm59055_saradc->lock);
		pr_info("%s: Error writing RTM delay\n", __func__);
		return ret;
	}
	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_ADCCTRL1);
	regVal &= ~(BCM59055_ADCCTRL1_RTM_CH_MASK <<
				BCM59055_ADCCTRL1_RTM_CH_MASK_SHIFT);	/* Clear Channel Select */
	/* Unmask RTM conversion */
	regVal &= ~(BCM59055_ADCCTRL1_RTM_START | BCM59055_ADCCTRL1_RTM_ENABLE);
	regVal |= (ch_sel << BCM59055_ADCCTRL1_RTM_CH_MASK_SHIFT);
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_ADCCTRL1, regVal); /* set channel and enable RTM req */
	if (ret) {
		mutex_unlock(&bcm59055_saradc->lock);
		return ret;
	}
	/* Enable the interrupts */
	bcm590xx_enable_irq(bcm59055, BCM59055_IRQID_INT9_RTM_DATA_RDY);
	bcm590xx_enable_irq(bcm59055, BCM59055_IRQID_INT9_RTM_DURING_CON_MEAS);	/* Enable RTM WHILE CONT INT */
	bcm590xx_enable_irq(bcm59055, BCM59055_IRQID_INT9_RTM_IGNORE);		/* Enable RTM_IGNORE INT */
	bcm590xx_enable_irq(bcm59055, BCM59055_IRQID_INT9_RTM_OVERRIDDEN);	/* Enable RTM OVERRIDDEN INT */
	bcm590xx_enable_irq(bcm59055, BCM59055_IRQID_INT9_RTM_UPPER_BOUND);	/* Enable RTM UPPER BOUND RDY INT */
	if (!adc_sync) {
		regVal |= BCM59055_ADCCTRL1_RTM_START;
		ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_ADCCTRL1, regVal);	/* Start RTM Conversion */
		if (ret) {
			mutex_unlock(&bcm59055_saradc->lock);
			return ret;
		}
		time_left = wait_for_completion_timeout(&bcm59055_saradc->rtm_req, I2C_RTM_EXECUTION_TIMEOUT);
	} else
		time_left = wait_for_completion_timeout(&bcm59055_saradc->rtm_req, ADCSYN_RTM_EXECUTION_TIMEOUT);
	/* Disable the interrupts */
	bcm590xx_disable_irq(bcm59055, BCM59055_IRQID_INT9_RTM_DATA_RDY);       /* Disable RTM DATA RDY INT */
	bcm590xx_disable_irq(bcm59055, BCM59055_IRQID_INT9_RTM_DURING_CON_MEAS);        /* Enable RTM WHILE CONT INT */
	bcm590xx_disable_irq(bcm59055, BCM59055_IRQID_INT9_RTM_UPPER_BOUND);    /* Enable RTM UPPER BOUND RDY INT */
	bcm590xx_disable_irq(bcm59055, BCM59055_IRQID_INT9_RTM_OVERRIDDEN);
	bcm590xx_disable_irq(bcm59055, BCM59055_IRQID_INT9_RTM_IGNORE);
	if (time_left == 0) {
		pr_info("%s: RTM request timeout..No RTM READY interrupt\n", __func__);
		mutex_unlock(&bcm59055_saradc->lock);
		return -ETIMEDOUT;
	}
	if (bcm59055_saradc->rtm_ignored) {
		pr_info("%s: RTM request has been ignored due to UPPER bound\n", __func__);
		bcm59055_saradc->rtm_ignored = 0;
		mutex_unlock(&bcm59055_saradc->lock);
		return -EIO;
	}
	if (bcm59055_saradc->rtm_overridden) {
		pr_info("%s: RTM request has been overridden due to ADC_SYNC \
				triggered conversion\n", __func__);
		bcm59055_saradc->rtm_overridden = 0;
		mutex_unlock(&bcm59055_saradc->lock);
		return -EIO;
	}
	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_ADCCTRL26);
	adcData = regVal;
	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_ADCCTRL25);
	adcData |= (regVal << 8);
	mutex_unlock(&bcm59055_saradc->lock);
	return adcData;
}

EXPORT_SYMBOL(bcm59055_saradc_rtm_read);

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
	int ret;
	struct bcm590xx *bcm59055;
	pr_info("inside %s\n", __func__);
	if(!bcm59055_saradc || !bcm59055_saradc->bcm59055)
		return -EPERM;
	mutex_lock(&bcm59055_saradc->lock);
	bcm59055 = bcm59055_saradc->bcm59055;

	if ((delay < ADCCTRL2_RTM_DELAY_0_USEC) ||
			(delay >= ADCCTRL2_RTM_DELAY_MAX))
		return -EINVAL;

	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_ADCCTRL2);
	regVal &= ~BCM59055_ADCCTRL2_RTM_DLY_MASK;	/* Clear the delay */
	regVal |= (delay & BCM59055_ADCCTRL2_RTM_DLY_MASK);
	bcm59055_saradc->rtm_dly_usec = delay * CLOCK_CYCLE_IN_USEC;
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_ADCCTRL2, regVal);
	mutex_unlock(&bcm59055_saradc->lock);
	return ret;
}

EXPORT_SYMBOL(bcm59055_saradc_set_rtm_delay);

/******************************************************************************
*
* Function Name: bcm59055_saradc_isr
*
* Description: Interrupt Handler
******************************************************************************/

static void bcm59055_saradc_isr(int intr, void *data)
{
	struct bcm59055_saradc *saradc = data;

	switch (intr) {
	case BCM59055_IRQID_INT9_RTM_DATA_RDY:
		pr_debug("%s: RTM_DATA_RDY INT\n", __func__);
		complete(&saradc->rtm_req);
		break;
	case BCM59055_IRQID_INT9_RTM_DURING_CON_MEAS:
		pr_debug("%s: BCM59055_IRQID_INT9_RTM_DURING_CON_MEAS\n", __func__);
		break;
	case BCM59055_IRQID_INT9_RTM_UPPER_BOUND:
		pr_debug("%s: BCM59055_IRQID_INT9_RTM_UPPER_BOUND\n",
			__func__);
		break;
	case BCM59055_IRQID_INT9_RTM_IGNORE:
		pr_debug("%s: BCM59055_IRQID_INT9_RTM_IGNORE\n", __func__);
		/* In case current RTM request is the given after BCM59055_IRQID_INT9_RTM_UPPER_BOUND
		 * request would be ignored and we need to notify the user of the request
		 * for the same.
		*/
		saradc->rtm_ignored = 1;
		complete(&saradc->rtm_req);
		break;
	case BCM59055_IRQID_INT9_RTM_OVERRIDDEN:
		pr_debug("%s: BCM59055_IRQID_INT9_RTM_OVERRIDDEN\n",
			__func__);
		/* This INT will come if I2C RTM conversion has been overridden by ADC_SYNC
		 * triggered conversion
		*/
		saradc->rtm_overridden = 1;
		complete(&saradc->rtm_req);
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
	init_completion(&saradc->rtm_req);
	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_ADCCTRL1);
	printk("%s: ADCCTRL1 = 0%x\n", __func__, regVal);
	regVal &= BCM59055_ADCCTRL1_RTM_DISABLE;	/* Set RTM_mask=0. No RTM request will be taken */
	regVal |= ADCCTRL1_RESET_COUNT_7;	/* Set Max Reset Count to 7 */
	bcm590xx_reg_write(bcm59055, BCM59055_REG_ADCCTRL1, regVal);

	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_ADCCTRL2);
	/* get current RTM delay */
	saradc->rtm_dly_usec = (regVal & BCM59055_ADCCTRL2_RTM_DLY_MASK) * CLOCK_CYCLE_IN_USEC;
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

	mutex_lock(&saradc->lock);
	saradc->rtm_ignored = 0;
	saradc->rtm_overridden = 0;
	mutex_unlock(&saradc->lock);
	saradc->bcm59055 = bcm59055;
	bcm59055_saradc = saradc;
	return 0;
}

static int __devexit bcm59055_saradc_remove(struct platform_device *pdev)
{
	struct bcm59055_saradc *saradc = platform_get_drvdata(pdev);
	struct bcm590xx *bcm59055;
	if (saradc) {
		bcm59055 = saradc->bcm59055;
		bcm590xx_free_irq(bcm59055, BCM59055_IRQID_INT9_RTM_DATA_RDY);
		bcm590xx_free_irq(bcm59055, BCM59055_IRQID_INT9_RTM_DURING_CON_MEAS);
		bcm590xx_free_irq(bcm59055, BCM59055_IRQID_INT9_RTM_UPPER_BOUND);
		bcm590xx_free_irq(bcm59055, BCM59055_IRQID_INT9_RTM_IGNORE);
		bcm590xx_free_irq(bcm59055, BCM59055_IRQID_INT9_RTM_OVERRIDDEN);

		kfree(saradc);
	}
	bcm59055_saradc = NULL;
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
