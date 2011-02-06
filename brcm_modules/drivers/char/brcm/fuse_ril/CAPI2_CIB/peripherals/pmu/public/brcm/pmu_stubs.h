/****************************************************************************
*
*     Copyright (c) 2007 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           5300 California Avenue
*           Irvine, California 92617
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
/**
*
*   @file   pmu_stubs.h
*   @brief  Header file for no_pmu build. Every typdef/macros here are dummy.  
*
****************************************************************************/
#ifndef __PMU_STUBS_H__
#define __PMU_STUBS_H__

//******************************************************************************
//                          Register map (dummy registers)
//******************************************************************************

#define PMU_REG_ID      0x00    // R
#define PMU_REG_INT1    0x01    // R&C
#define PMU_REG_INT2    0x02    // R&C
#define PMU_REG_TOTAL	PMU_REG_INT2+1


//******************************************************************************
// Typedefs 
//******************************************************************************
// Dummy IRQ IDs
typedef enum {
    PMU_IRQID_INT1LOWBAT,
    PMU_IRQID_INT1SECOND,
    PMU_IRQID_INT1MINUTE,
    PMU_IRQID_INT1ALARM,
    PMU_IRQID_INT1ONKEYR,
    PMU_IRQID_INT1ONKEYF,
    PMU_IRQID_INT1ONKEY1S,
    PMU_IRQID_INT1THS,

    PMU_IRQID_INT2REC1R,
    PMU_IRQID_INT2REC1F,
    PMU_IRQID_INT2REC2LF,
    PMU_IRQID_INT2REC2LR,
    PMU_IRQID_INT2REC2HF,
    PMU_IRQID_INT2REC2HR,
    PMU_IRQID_INT2VMAX,
    PMU_IRQID_INT2CHGWD,

    PMU_IRQID_INT3SIMUV,
    PMU_IRQID_INT3INSERT,
    PMU_IRQID_INT3EXTRACT,
    PMU_IRQID_INT3MUTE,
    PMU_IRQID_INT3EARLY,
    PMU_IRQID_INT3SIMRDY,
    PMU_IRQID_INT3CHGINS,
    PMU_IRQID_INT3CHGRM,
    
    PMU_IRQID_INT4CHGRES,
    PMU_IRQID_INT4THLIMON,
    PMU_IRQID_INT4THLIMOFF,
    PMU_IRQID_INT4BATFUL,
    PMU_IRQID_INT4BATTMFLT,
    PMU_IRQID_INT4BATTMOK,
    PMU_IRQID_INT4UCHGRM,
    PMU_IRQID_INT4UCHGINS,
    PMU_TOTAL_IRQ
} PMU_InterruptId_t;

// LDO typedef for API
typedef enum {
    PMU_LDO_D1C  = 0x16,
    PMU_LDO_D2C  = 0x17,
    PMU_LDO_D3C  = 0x18,
    PMU_LDO_HCC  = 0x19,
    PMU_LDO_IOC  = 0x1A,
    PMU_LDO_LPC  = 0x1B,
    PMU_LDO_RF1C = 0x1C,
    PMU_LDO_RF2C = 0x1D 
} PMU_LDO_t;

//--- for PWM
typedef enum{
    PMUPWM_ID1,
    PMUPWM_ID2
} PMUPwmID_t;

#define PMU_KEYLIGHT    	PMUPWM_ID1
#define PMU_LCD_BACKLIGHT	PMUPWM_ID2

typedef enum {
	PMU_LDO1P2VOLT = 0x03,
    PMU_LDO1P3VOLT = 0x04,
	PMU_LDO1P4VOLT = 0x05,
	PMU_LDO1P5VOLT = 0x06,
	PMU_LDO1P6VOLT = 0x07,
    PMU_LDO1P7VOLT = 0x08,
	PMU_LDO1P8VOLT = 0x09,
	PMU_LDO1P9VOLT = 0x0a,
    PMU_LDO2P0VOLT = 0x0b,
	PMU_LDO2P1VOLT = 0x0c,
	PMU_LDO2P2VOLT = 0x0d,
    PMU_LDO2P3VOLT = 0x0e,
	PMU_LDO2P4VOLT = 0x0f,
	PMU_LDO2P5VOLT = 0x10,
    PMU_LDO2P6VOLT = 0x11,
	PMU_LDO2P7VOLT = 0x12,
	PMU_LDO2P8VOLT = 0x13,
    PMU_LDO2P9VOLT = 0x14,
	PMU_LDO3P0VOLT = 0x15,
	PMU_LDO3P1VOLT = 0x16,
	PMU_LDO3P2VOLT = 0x17,
	PMU_LDO3P3VOLT = 0x18
} PMU_LDOVolt_t;

typedef enum {
	PMU_GPO_ACTIVE_LOW  = 0,
	PMU_GPO_LED1_OUTPUT,
	PMU_GPO_LED2_OUTPUT,
	PMU_GPO_PWM1_OUTPUT,
	PMU_GPO_N_PWM1_OUTPUT,
	PMU_GPO_PWM2_OUTPUT,
	PMU_GPO_N_PWM2_OUTPUT,
	PMU_GPO_HIGH_IMPEDANCE
} PMU_GPOutput_t;

typedef enum {
	PMU_GPO1 = 1,
	PMU_GPO2 = 2,
	PMU_GPO3 = 3
} PMU_GPOpin_t;

#define OPMODE0		0
#define OPMODE1		1
#define OPMODE2		2
#define OPMODE3		3
#define OPMODE4		4
#define OPMODE5		5
#define OPMODE6		6
#define OPMODE7		7

#endif // #ifndef __PMU_STUBS_H__


