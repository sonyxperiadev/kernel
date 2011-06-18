/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   include/linux/broadcom/bcm59055-adc.h
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
* bcm59055-adc.h
*
* PURPOSE:
*
*
*
* NOTES:
*
* ****************************************************************************/

#ifndef __BCM59055_ADC_H__
#define __BCM59055_ADC_H__

enum {
	ADC_VMBAT_CHANNEL = 0,
	ADC_VBBAT_CHANNEL,
	ADC_VWALL_CHANNEL,
	ADC_VBUS_CHANNEL,
	ADC_ID_CHANNEL,
	ADC_NTC_CHANNEL,
	ADC_BSI_CHANNEL,
	ADC_BOM_CHANNEL,
	ADC_32KTEMP_CHANNEL,
	ADC_PATEMP_CHANNEL,
	ADC_ALS_CHANNEL,
	ADC_BSI_CAL_L_CHANNEL,
	ADC_NTC_CAL_L_CHANNEL,
	ADC_NTC_CAL_H_CHANNEL,
	ADC_BSI_CAL_H_CHANNEL,
	ADC_NULL_CHANNEL
};

enum {
	ADCCTRL1_RESET_COUNT_1 = 0,
	ADCCTRL1_RESET_COUNT_3,
	ADCCTRL1_RESET_COUNT_5,
	ADCCTRL1_RESET_COUNT_7
};


enum {
	ADCCTRL2_RTM_DELAY_0_USEC = 0,
	ADCCTRL2_RTM_DELAY_31_25_USEC,
	ADCCTRL2_RTM_DELAY_62_50_USEC,
	ADCCTRL2_RTM_DELAY_93_75_USEC,

	ADCCTRL2_RTM_DELAY_125_USEC,
	ADCCTRL2_RTM_DELAY_156_25_USEC,
	ADCCTRL2_RTM_DELAY_187_50_USEC,
	ADCCTRL2_RTM_DELAY_218_75_USEC,

	ADCCTRL2_RTM_DELAY_250_USEC,
	ADCCTRL2_RTM_DELAY_281_25_USEC,
	ADCCTRL2_RTM_DELAY_312_50_USEC,
	ADCCTRL2_RTM_DELAY_343_75_USEC,

	ADCCTRL2_RTM_DELAY_375_USEC,
	ADCCTRL2_RTM_DELAY_406_25_USEC,
	ADCCTRL2_RTM_DELAY_437_50_USEC,
	ADCCTRL2_RTM_DELAY_468_75_USEC,

	ADCCTRL2_RTM_DELAY_500_USEC,
	ADCCTRL2_RTM_DELAY_531_25_USEC,
	ADCCTRL2_RTM_DELAY_562_50_USEC,
	ADCCTRL2_RTM_DELAY_593_75_USEC,

	ADCCTRL2_RTM_DELAY_625_USEC,
	ADCCTRL2_RTM_DELAY_656_25_USEC,
	ADCCTRL2_RTM_DELAY_687_50_USEC,
	ADCCTRL2_RTM_DELAY_718_75_USEC,

	ADCCTRL2_RTM_DELAY_750_USEC,
	ADCCTRL2_RTM_DELAY_781_25_USEC,
	ADCCTRL2_RTM_DELAY_812_50_USEC,
	ADCCTRL2_RTM_DELAY_843_75_USEC,

	ADCCTRL2_RTM_DELAY_875_USEC,
	ADCCTRL2_RTM_DELAY_906_25_USEC,
	ADCCTRL2_RTM_DELAY_937_50_USEC,
	ADCCTRL2_RTM_DELAY_968_75_USEC,
	ADCCTRL2_RTM_DELAY_MAX,
};

#define BCM59055_ADCCTRL1_RTM_ENABLE		(1 << 3)
#define BCM59055_ADCCTRL1_RTM_DISABLE		~(BCM59055_ADCCTRL1_RTM_ENABLE)
#define BCM59055_ADCCTRL1_RTM_START			(1 << 2)
#define BCM59055_ADCCTRL1_RTM_CH_MASK_SHIFT	0x4
#define BCM59055_ADCCTRL1_RTM_CH_MASK		0xF


#define BCM59055_ADCCTRL2_GSM_DEBOUNCE		(1 << 5)

#define BCM59055_ADCCTRL2_RTM_DLY_MASK		0x1F

#define BCM59055_ADCDATA_INVALID			(1 << 2)

#define BCM59055_CMPCTRL12_NTCON			0x1

typedef void (*saradc_rtm_callback_handler) (void *param, u32 selection,
					     u16 data);

extern int bcm59055_saradc_start_burst_mode(void);
extern int bcm59055_saradc_stop_burst_mode(void);
extern int bcm59055_saradc_enable_ntc_block(void);
extern int bcm59055_saradc_disable_ntc_block(void);
extern int bcm59055_saradc_read_data(int sel);
extern int bcm59055_saradc_request_rtm(saradc_rtm_callback_handler handler, int ch_sel, void *arg);
extern int bcm59055_saradc_set_rtm_delay(int delay);
#endif


