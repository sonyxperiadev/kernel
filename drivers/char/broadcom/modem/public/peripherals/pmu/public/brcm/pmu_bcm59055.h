/****************************************************************************
*
*     Copyright (c) 2010 Broadcom Corporation
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
*   @file   pmu_bcm59055.h
*
*   @brief  This file includes memory map definitions for Broadcom PMU BCM59036.
*           Contains specific macros, typedefs, and defs specific for BCM59036.
*
****************************************************************************/

//******************************** History *************************************
//
// $Log:  $
// Initial Ver. 02-05-2010     Jim Chang
// 01-06-2011	Added ADP support Raja Ramaubramanian
//******************************************************************************

//******************************************************************************
//                          definition block
//******************************************************************************
#ifndef _PMU_BCM59055_H_
#define _PMU_BCM59055_H_

//#define BCM59055_B0       // enable for B0 PMU chip

// I2C addresses for BCM59036
#define PMU_BASE_ADDR   0x10    // write address 00010000
#define PMU_BASE_W      0x10    // write address 00010000
#define PMU_BASE_R      0x11    // read  address 00010001

// PMU REGISTERS MAPPING --------------------------------------------------------


// ADDMAP0, PAGE 0
// System configuration/Accessory
#define PMU_REG_I2CCNTRL        0x00    // 
#define PMU_REG_HOSTCTRL1       0x01    // 
#define PMU_REG_HOSTCTRL2       0x02    // 
#define PMU_REG_HOSTCTRL3       0x03    //
#define PMU_REG_PMUGID          0x04    //
#define PMU_REG_HOSTCTRL4       0x05    //
#define PMU_REG_WRLOCKKEY       0x07 
#define PMU_REG_PLLCTRL       	0x0B
#define PMU_REG_PONKEYCTRL1     0x0C
#define PMU_REG_PONKEYCTRL2     0x0D
#define PMU_REG_AUXCTRL         0x0E    //                                       
#define PMU_REG_PONKEYDBG       0x0F     

// Comparators
#define PMU_REG_CMPCTRL1        0x10    // 
#define PMU_REG_CMPCTRL2        0x11    // 
#define PMU_REG_CMPCTRL3        0x12    // 
#define PMU_REG_CMPCTRL11       0x1A    // 
#define PMU_REG_CMPCTRL12       0x1B    //                                        
#define PMU_REG_CMPCTRL13       0x1C    // 
                                        
// RTC                                        
#define PMU_REG_RTCSC           0x20    // R/W  real time clock seconds [5:0]
#define PMU_REG_RTCMN           0x21    // R/W  real time clock minutes [5:0]
#define PMU_REG_RTCHR           0x22    // R/W  real time clock hours [4:0]
#define PMU_REG_RTCWD           0x23    // R/W  real time clock weekday [2:0]
#define PMU_REG_RTCDT           0x24    // R/W  real time clock day [4:0]
#define PMU_REG_RTCMT           0x25    // R/W  real time clock month [3:0]
#define PMU_REG_RTCYR           0x26    // R/W  real time clock year [7:0]
#define PMU_REG_RTCSC_A1        0x27    // R/W  alarm clock 1 seconds [5:0]
#define PMU_REG_RTCMN_A1        0x28    // R/W  alarm clock 1 minutes [5:0]
#define PMU_REG_RTCHR_A1        0x29    // R/W  alarm clock 1 hours [4:0]
#define PMU_REG_RTCWD_A1        0x2A    // R/W  alarm clock 1 weekday [6:0]
#define PMU_REG_RTCDT_A1        0x2B    // R/W  alarm clock 1 day [4:0]
#define PMU_REG_RTCMT_A1        0x2C    // R/W  alarm clock 1 month [3:0]
#define PMU_REG_RTCYR_A1        0x2D    // R/W  alarm clock 1 year [7:0]
#define PMU_REG_RTCEXSAVE       0x2E
#define PMU_REG_BBCCTRL         0x2F    // R/W  backup battery charger control [7:0]

// Interrupts
#define PMU_REG_INT1        	0x30
#define PMU_REG_INT2        	0x31
#define PMU_REG_INT3        	0x32
#define PMU_REG_INT4        	0x33
#define PMU_REG_INT5        	0x34
#define PMU_REG_INT6        	0x35
#define PMU_REG_INT7        	0x36
#define PMU_REG_INT8        	0x37
#define PMU_REG_INT9        	0x38
#define PMU_REG_INT10			0x39
#define PMU_REG_INT11        	0x3A
#define PMU_REG_INT12        	0x3B
#define PMU_REG_INT13			0x3C
#define PMU_REG_INT14			0x3D

#define PMU_REG_INT1M        	0x40
#define PMU_REG_INT2M        	0x41
#define PMU_REG_INT3M        	0x42
#define PMU_REG_INT4M        	0x43
#define PMU_REG_INT5M        	0x44
#define PMU_REG_INT6M        	0x45
#define PMU_REG_INT7M        	0x46
#define PMU_REG_INT8M        	0x47
#define PMU_REG_INT9M        	0x48
#define PMU_REG_INT10M          0x49
#define PMU_REG_INT11M        	0x4A
#define PMU_REG_INT12M        	0x4B
#define PMU_REG_INT13M          0x4C                                 
#define PMU_REG_INT14M          0x4D

// Main battery control
#define PMU_REG_MBCCTRL1        0x50
#define PMU_REG_MBCCTRL2        0x51
#define PMU_REG_MBCCTRL3        0x52
#define PMU_REG_MBCCTRL4        0x53
#define PMU_REG_MBCCTRL5        0x54
#define PMU_REG_MBCCTRL6        0x55
#define PMU_REG_MBCCTRL7        0x56
#define PMU_REG_MBCCTRL8        0x57
#define PMU_REG_MBCCTRL9        0x58
#define PMU_REG_MBCCTRL10       0x59
#define PMU_REG_MBCCTRL11       0x5A
#define PMU_REG_MBCCTRL12       0x5B
#define PMU_REG_MBCCTRL13       0x5C
#define PMU_REG_MBCCTRL14       0x5D
#define PMU_REG_MBCCTRL22      0x65
#define PMU_REG_MBCCTRL23      0x66
#define PMU_REG_MBCCTRL24      0x67
#define PMU_REG_MBCCTRL25      0x68
#define PMU_REG_MBCCTRL26      0x69

// OTG
#define PMU_REG_OTGCTRL1        0x70
#define PMU_REG_OTGCTRL2        0x71
#define PMU_REG_OTGCTRL3        0x72
#define PMU_REG_OTGCTRL4        0x73
#define PMU_REG_OTGCTRL5        0x74
#define PMU_REG_OTGCTRL6        0x75
#define PMU_REG_OTGCTRL7        0x76
#define PMU_REG_OTGCTRL8        0x77
#define PMU_REG_OTGCTRL9        0x78
#define PMU_REG_OTGCTRL10       0x79
#define PMU_REG_OTGCTRL11       0x7A
#define PMU_REG_OTGCTRL12       0x7B
#define PMU_REG_BOOSTCTRL1      0x7C
#define PMU_REG_BOOSTCTRL2      0x7D
#define PMU_REG_BOOSTCTRL3      0x7E
#define PMU_REG_BOOSTCTRL4      0x7F

// ADC
#define PMU_REG_ADCCTRL1        0x80
#define PMU_REG_ADCCTRL2        0x81
#define PMU_REG_ADCCTRL3        0x82
#define PMU_REG_ADCCTRL4        0x83
#define PMU_REG_ADCCTRL5        0x84
#define PMU_REG_ADCCTRL6        0x85
#define PMU_REG_ADCCTRL7        0x86
#define PMU_REG_ADCCTRL8        0x87
#define PMU_REG_ADCCTRL9        0x88
#define PMU_REG_ADCCTRL10       0x89
#define PMU_REG_ADCCTRL11       0x8A
#define PMU_REG_ADCCTRL12       0x8B
#define PMU_REG_ADCCTRL13       0x8C
#define PMU_REG_ADCCTRL14       0x8D
#define PMU_REG_ADCCTRL15       0x8E
#define PMU_REG_ADCCTRL16       0x8F
#define PMU_REG_ADCCTRL17       0x90
#define PMU_REG_ADCCTRL18       0x91
#define PMU_REG_ADCCTRL19       0x92
#define PMU_REG_ADCCTRL20       0x93
#define PMU_REG_ADCCTRL21       0x94
#define PMU_REG_ADCCTRL22       0x95
#define PMU_REG_ADCCTRL23       0x96
#define PMU_REG_ADCCTRL24       0x97
#define PMU_REG_ADCCTRL25       0x98
#define PMU_REG_ADCCTRL26       0x99

//Power control
#define PMU_REG_RFOPMODCTRL     0xA0
#define PMU_REG_CAMOPMODCTRL    0xA1
#define PMU_REG_HV1OPMODCTRL    0xA2
#define PMU_REG_HV2OPMODCTRL    0xA3
#define PMU_REG_HV3OPMODCTRL    0xA4
#define PMU_REG_HV4OPMODCTRL    0xA5
#define PMU_REG_HV5OPMODCTRL    0xA6
#define PMU_REG_HV6OPMODCTRL    0xA7
#define PMU_REG_HV7OPMODCTRL    0xA8
#define PMU_REG_SIMOPMODCTRL    0xA9
#define PMU_REG_CSROPMODCTRL    0xAA
#define PMU_REG_IOSROPMODCTRL   0xAB
#define PMU_REG_SDSROPMODCTRL   0xAC
#define PMU_REG_RFLDOCTRL       0xB0
#define PMU_REG_CAMLDOCTRL      0xB1
#define PMU_REG_HVLDO1CTRL      0xB2
#define PMU_REG_HVLDO2CTRL      0xB3
#define PMU_REG_HVLDO3CTRL      0xB4
#define PMU_REG_HVLDO4CTRL      0xB5
#define PMU_REG_HVLDO5CTRL      0xB6
#define PMU_REG_HVLDO6CTRL      0xB7
#define PMU_REG_HVLDO7CTRL      0xB8
#define PMU_REG_SIMLDOCTRL      0xB9
#define PMU_REG_VIOPONGRPCTRL   0xBA

// Switching regulator
#define PMU_REG_CSRCTRL1        0xC0
#define PMU_REG_CSRCTRL2        0xC1
#define PMU_REG_CSRCTRL3        0xC2
#define PMU_REG_CSRCTRL4        0xC3
#define PMU_REG_CSRCTRL5        0xC4
#define PMU_REG_CSRCTRL6        0xC5
#define PMU_REG_CSRCTRL7        0xC6
#define PMU_REG_IOSRCTRL1       0xC8
#define PMU_REG_IOSRCTRL2       0xC9
#define PMU_REG_IOSRCTRL3       0xCA
#define PMU_REG_IOSRCTRL4       0xCB
#define PMU_REG_IOSRCTRL5       0xCC
#define PMU_REG_IOSRCTRL6       0xCD
#define PMU_REG_IOSRCTRL7       0xCE
#define PMU_REG_SDSRCTRL1       0xD0
#define PMU_REG_SDSRCTRL2       0xD1
#define PMU_REG_SDSRCTRL3       0xD2
#define PMU_REG_SDSRCTRL4       0xD3
#define PMU_REG_SDSRCTRL5       0xD4
#define PMU_REG_SDSRCTRL6       0xD5
#define PMU_REG_SDSRCTRL7       0xD5

// Environment Registers
#define PMU_REG_ENV1            0xE0    // R    environment monitor
#define PMU_REG_ENV2            0xE1    // R    environment monitor [5:0]
#define PMU_REG_ENV3            0xE2    // R    environment monitor [4:0]
#define PMU_REG_ENV4            0xE3    // R    environment monitor
#define PMU_REG_ENV5       		0xE4    // R    environment monitor
#define PMU_REG_ENV6       		0xE5    // R    environment monitor
#define PMU_REG_ENV7            0xE6    // R    environment monitor
#define PMU_REG_ENV8       		0xE7    // R    environment monitor
#define PMU_REG_ENV9       		0xE8    // R    environment monitor

// Page select register ADDMAP0, PAGE0
#define PMU_REG_PAGESEL         0xFF

// PMU/PID  ADDMAP0, PAGE 1
#define PMU_REG_PMUMODE         0xEF

// ADDMAP1, PAGE 0
// PWMLED
#define PMU_REG_PWMLEDCTRL1     0x00    
#define PMU_REG_PWMLEDCTRL2     0x01  
#define PMU_REG_PWMLEDCTRL3     0x02  
#define PMU_REG_PWMLEDCTRL4     0x03  
#define PMU_REG_PWMLEDCTRL5     0x04  
#define PMU_REG_PWMLEDCTRL6     0x05  
#define PMU_REG_PWMLEDCTRL7     0x06  
#define PMU_REG_PWMLEDCTRL8     0x07  
#define PMU_REG_PWMLEDCTRL9     0x08  
#define PMU_REG_PWMLEDCTRL10    0x09    
#define PMU_REG_PWMLEDCTRL11    0x0A
#define PMU_REG_PWMLEDCTRL12    0x0B
#define PMU_REG_PWMLEDCTRL13    0x0C
#define PMU_REG_PWMLEDCTRL14    0x0D
#define PMU_REG_PWMLEDCTRL15    0x0E
#define PMU_REG_PWMLEDCTRL16    0x0F
#define PMU_REG_PWMLEDCTRL17    0x10
#define PMU_REG_PWMLEDCTRL18    0x11
#define PMU_REG_PWMLEDCTRL19    0x12
#define PMU_REG_PWMLEDCTRL20    0x13
#define PMU_REG_PWMLEDCTRL21    0x14

// DBI section, ADDMAP1, PAGE 0
#define PMU_REG_DBICTRL         0x21  

// ADP section, ADDMAP1, PAGE 0
#define PMU_REG_OTGSTS1     	0x70
#define PMU_REG_OTGSTS2         0x71
#define PMU_REG_OTGSTS3     	0x72
#define PMU_REG_OTGSTS4     	0x73
#define PMU_REG_OTGSTS5         0x74
#define PMU_REG_OTGSTS6      	0x75

// Audio section, ADDMAP1, PAGE 0
#define PMU_REG_IHFTOP        	0x80
#define PMU_REG_IHFBIASCLK      0x81
#define PMU_REG_IHFLDO        	0x82
#define PMU_REG_IHFPOP        	0x83
#define PMU_REG_IHFRCCALI       0x84
#define PMU_REG_IHFADCI        	0x85
#define PMU_REG_IHFSSP        	0x86
#define PMU_REG_IHFHSAUDTST     0x87
#define PMU_REG_IHFANARAMP      0x88
#define PMU_REG_IHFPGA1        	0x89
#define PMU_REG_IHFPGA2        	0x8A
#define PMU_REG_IHFLOOPFILTER   0x8B
#define PMU_REG_IHFCOMPPD       0x8C
#define PMU_REG_IHFDRIVER       0x8D
#define PMU_REG_IHFFBAMP        0x8E
#define PMU_REG_IHFSTIN        	0x8F
#define PMU_REG_IHFSTO        	0x90
#define PMU_REG_IHFSCDADCO      0x91
#define PMU_REG_IHFRCCALO       0x92
#define PMU_REG_IHFRCCALRAW1O   0x93
#define PMU_REG_IHFRCCALRAW2O   0x94
#define PMU_REG_IHFMISC			0x95

#define PMU_REG_HSCP1			0x98
#define PMU_REG_HSCP2			0x99
#define PMU_REG_HSCP3			0x9A
#define PMU_REG_HSDRV        	0x9B
#define PMU_REG_HSLDO        	0x9C
#define PMU_REG_HSLF        	0x9D
#define PMU_REG_HSPGA1        	0x9E
#define PMU_REG_HSPGA2        	0x9F
#define PMU_REG_HSPGA3        	0xA0
#define PMU_REG_HSRC        	0xA1
#define PMU_REG_HSPUP1        	0xA2
#define PMU_REG_HSPUP2        	0xA3
#define PMU_REG_HSIST        	0xA4
#define PMU_REG_HSOC1        	0xA5
#define PMU_REG_HSOC2        	0xA6
#define PMU_REG_HSOC3        	0xA7
#define PMU_REG_HSOC4        	0xA8
#define PMU_REG_HSOUT1        	0xA9
#define PMU_REG_HSOUT2        	0xAA
#define PMU_REG_HSOUT3        	0xAB
#define PMU_REG_HSOUT4        	0xAC
#define PMU_REG_HSOUT5        	0xAD
#define PMU_REG_HSOUT6        	0xAE
#define PMU_REG_HSOUT7        	0xAF

// FUEL GAUGE, ADDMAP1, PAGE 0
#define PMU_REG_FGCTRL1         0xC0    
#define PMU_REG_FGCTRL2         0xC1    
#define PMU_REG_FGOPMODCTRL     0xC2     
#define PMU_REG_FGOCICCTRL1     0xC3                                            
#define PMU_REG_FGOFFSET_TRIM   0xC4    //  
#define PMU_REG_FGGAIN_TRIM     0xC5    //  
#define PMU_REG_FGACCM1         0xC6    // 
#define PMU_REG_FGACCM2         0xC7    // 
#define PMU_REG_FGACCM3         0xC8    // 
#define PMU_REG_FGACCM4         0xC9    // 
#define PMU_REG_FGCNT1          0xCA    // 
#define PMU_REG_FGCNT2          0xCB    // 
#define PMU_REG_FGSLEEPCNT1     0xCC    // 
#define PMU_REG_FGSLEEPCNT2     0xCD    // 
#define PMU_REG_FGSMPL1         0xCE    // FG normal sample output before offset/gain calibration [15:8]
#define PMU_REG_FGSMPL2         0xCF    // FG normal sample output before offset/gain calibration [7:0]
#define PMU_REG_FGSMPL3         0xD0    // FG normal sample output after offset/gain calibration [15:8]
#define PMU_REG_FGSMPL4         0xD1    // FG normal sample output after offset/gain calibration [7:0]
#define PMU_REG_FGSMPL5         0xD2    // FG fast sample output before offset/gain calibration [15:8]
#define PMU_REG_FGSMPL6         0xD3    // FG fast sample output before offset/gain calibrationfuel [7:0]                                        
#define PMU_REG_FGOFFSET1       0xD4    // 
#define PMU_REG_FGOFFSET2       0xD5    //
#define PMU_REG_FGBATCAL1       0xD6    // 
#define PMU_REG_FGBATCAL2       0xD7    //
#define PMU_REG_FGGNRL1         0xD8    // 
#define PMU_REG_FGGNRL2         0xD9    
#define PMU_REG_FGGNRL3         0xDA    
#define PMU_REG_FGTRIMGN1_1     0xDB
#define PMU_REG_FGTRIMGN1_2     0xDC    
#define PMU_REG_FGTRIMGN2_1     0xDD    
#define PMU_REG_FGTRIMGN2_2     0xDE

// PMUID ADDMAP1, PAGE 0
#define PMU_REG_PMUID           0xF7
#define PMU_REG_PMUREV          0xF8   


#define PMU_REG_TOTAL_PER_DEV		(PMU_REG_PAGESEL + 1)  //0xFF + 1                                 
#define PMU_REG_TOTAL			415	// There are some gaps in ADDMAP1	
#define PMU_REG_LDO_SR_TOTAL	10
	                                        
/* Revisit:
old 035 stuff)


#define PMU_REG_MSPWRGRP		0x9E    // R/W  MSLDO power-up grouping control [5:0]
#define PMU_REG_AXPWRGRP		0x9F    // R/W  AXLDO power-up grouping control [5:0]
#define PMU_REG_LVPWRGRP		0xA0    // R/W  LVLDO power-up grouping control [5:0]
#define PMU_REG_IOPWRGRP		0xA1    // R/W  IOLDO power-up grouping control [2:0]
#define PMU_REG_SRPWRGRP		0xA2    // R/W  SR power-up grouping control [5:0]
#define PMU_REG_APWRGRP			0xAA    // R/W  ALDO power-up grouping control [5:0]
#define PMU_REG_RFPWRGRP		0xAB    // R/W  RFLDO power-up grouping control [5:0]
#define PMU_REG_LCSIMPWRGRP		0xAC    // R/W  LC and SIMLDO power-up grouping control [5:0]
#define PMU_REG_HCPWRGRP		0xAD    // R/W  HCLDO power-up grouping control [5:0]


#define PMU_REG_PDCMPSYN        0xE4    // R    presence detection/comparator status [7:0]
#define PMU_REG_PDCMPSYN1       0xE5    // R    presence detection/comparator status [7:0]
#define PMU_REG_PDCMPSYN2       0xE6    // R    presence detection/comparator status [7:0]
#define PMU_REG_PDCMPSYN3       0xE7    // R    presence detection/comparator status [7:0]
#define PMU_REG_PDCMPSYN4       0xE8    // R    presence detection/comparator status [7:0]
#define PMU_REG_PDCMPSYN5       0xE9    // R    presence detection/comparator status [7:0]
#define PMU_REG_PDCMPSYN6       0xEA    // R    presence detection/comparator status [7:0]

#define PMU_REG_TOTAL			(PMU_REG_PDCMPSYN6+1)		// 0xEA registers for BCM59036 A0
#define PMU_REG_LDO_SR_TOTAL	10
#define PMU_REG_LDO_SR_START_INDEX	PMU_REG_ALDOCTRL

*/

//Audio bitmaps
#define PMU_HSPUP2_PWRUP				0x40
#define PMU_HSDRV_SC_DISSC				0x10
#define PMU_HSDRV_SC_CURRENT_MASK		0x03
#define PMU_HS_GAIN_L					0x40
#define PMU_HS_GAIN_R					0x80
#define PMU_HS_GAIN_MASK				0x3F
#define PMU_HS_IDDQ_PWRDWN				0x10
#define PMU_PLL_EN						0x02
#define PMU_PLL_AUDIO_EN				0x20
#define PMU_HS_HSPGA3_ACINADJ           0x20
#define PMU_HS_HSPGA3_ENACCPL           0x04
#define PMU_HS_HSPGA3_PULLDNSJ          0x02
#define PMU_HS_HSCP3_I_CP_CG_I2C        0x08
#define PMU_HS_HSCP3_I_CP_CG_SEL        0x04

#define PMU_IHF_IDDQ					0x01  // global IHF power down control in IHFTOP
#define PMU_IHF_LDOPUP					0x01
#define PMU_IHF_POPPUP					0x08
#define PMU_IHF_AUTOSEQ					0x40
#define PMU_IHF_BYPASS					0x60
#define PMU_IHF_GAIN_MASK				0x3F
#define PMU_HS_SC_EDISABLE				0x04
#define PMU_IHF_SC_EDISABLE				0x08

// Register bitmap defines
#define PMU_I2CCNTRL_I2CHSEN			0x80
#define PMU_I2CCNTRL_I2CRDBLOCKEN		0x40
#define PMU_I2CCNTRL_I2C_HIGHSPEED		0x20
#define PMU_I2CCNTRL_I2CFILTEN			0x10
#define PMU_I2CCNTRL_I2CSLAVEID			0x0C
#define PMU_I2CCNTRL_FIFOFULL_R			0x02
#define PMU_I2CCNTRL_I2CRDBLOCK			0x01

// Register bitmap defines
#define PMU_HOSTCTRL1_SYS_WDT_EN		0x01
#define PMU_HOSTCTRL1_SYS_WDT_CLR		0x02
#define PMU_HOSTCTRL1_SW_SHDWN 	    	0x04  // shutdown PMU; self clear and write protected
#define PMU_HOSTCTRL1_BLK_MBWV_SHDWN	0x08
#define PMU_HOSTCTRL1_CLK32K_ONOFF     	0x10
#define PMU_HOSTCTRL1_STATMUX       	0x20
#if defined(BCM59055_B0)
#define PMU_HOSTCTRL1_MBPD_R_DB       	0x40  // B0 only
#endif
#define PMU_HOSTCTRL2_SYS_WDT_TIME_MASK	0x7F

#if defined(BCM59055_B0)
#define PMU_CMPCTRL3_MBWV_DEB_SEL_MASK 	0x30   // B0 only
#endif

#define PMU_PONKEYCTRL1_KEY_PAD_LOCK	0x40
#define PMU_PONKEYCTRL1_PRESS_DEB_MASK	0x03
#define PMU_PONKEYCTRL1_RELEASE_DEB_MASK    0x38
#define PMU_PONKEYCTRL2_VLD_TMR_MASK	0x1F

#define PMU_CMPCTRL1_MBWVS_MASK			0x07   // MBWV mask 
#define PMU_CMPCTRL1_MBUVS_MASK			0xC0   // MBUV mask
#define PMU_CMPCTRL2_VBUSCLPSCS_MASK	0x03
#define PMU_CMPCTRL2_FGCVS_MASK 		0x18
#define PMU_CMPCTRL2_BBATUVS_MASK 		0x60
#define PMU_CMPCTRL11_BAT_DET_EN       	0x01
#define PMU_CMPCTRL11_MBOV_HOST_EN		0x04
#define PMU_CMPCTRL12_NTCON 		    0x01
#define PMU_CMPCTRL12_NTC_SYNC_MODE		0x02
#define PMU_CMPCTRL12_NTC_EN_PM0   		0x10                                              
#define PMU_CMPCTRL12_NTC_EN_PM1		0x20
#define PMU_CMPCTRL12_NTC_EN_PM2		0x40
#define PMU_CMPCTRL12_NTC_EN_PM3		0x80
#define PMU_CMPCTRL13_BSI_ON		    0x01
#define PMU_CMPCTRL13_BSI_SYNC_MODE		0x02
#define PMU_CMPCTRL13_BSI_EN_PM0   		0x10                                              
#define PMU_CMPCTRL13_BSI_EN_PM1		0x20
#define PMU_CMPCTRL13_BSI_EN_PM2		0x40
#define PMU_CMPCTRL13_BSI_EN_PM3		0x80

#define PMU_RTC_CORE_RTC_EXSAVE2	0x04
#define PMU_RTC_CORE_RTC_EXSAVE3	0x08
#define PMU_RTC_CORE_RTC_EXSAVE4_MASK	0xF0

//BBCCTRL SECTION
#define	PMU_BBCCTRL_HOSTEN				0x01
#define	PMU_BBCCTRL_RES_SEL				0x06
#define	PMU_BBCCTRL_CUR_SEL				0x38
#define	PMU_BBCCTRL_VOL_SEL				0xC0

// INT1        
#define PMU_INT1_POK_PRESS       0x01
#define PMU_INT1_POK_RELEASE     0x02
#define PMU_INT1_POK_WAKEUP      0x04
#define PMU_INT1_POK_BIT_VLD     0x08
#define PMU_INT1_POK_SHDWN       0x10
#define PMU_INT1_AUX_INS         0x20
#define PMU_INT1_AUX_RM          0x40
#define PMU_INT1_GBAT_IN         0x80

//INT2         
#define PMU_INT2_CHGINS         0x01
#define PMU_INT2_CHGRM          0x02
#define PMU_INT2_CHGOV_DIS      0x04
#if defined(BCM59055_B0)
#define PMU_INT2_WAC_CC_REDUCED 0x08  // B0 chip
#endif
#define PMU_INT2_USBINS         0x10
#define PMU_INT2_USBRM          0x20
#define PMU_INT2_UBPD_CHP_DIS   0x40
#if defined(BCM59055_B0)
#define PMU_INT2_USB_CC_REDUCED 0x80  // B0 chip
#endif

//INT3          
#define PMU_INT3_WAC_REV_POL    0x01
#define PMU_INT3_USB_REV_POL    0x02
#define PMU_INT3_MAIN_CV_LOOP   0x04
#define PMU_INT3_CV_TMP_EXP     0x08
#define PMU_INT3_CGPD_CHG_F     0x10
#define PMU_INT3_UBPD_CHG_F     0x20
#define PMU_INT3_CHGERRDIS      0x40
#define PMU_INT3_ACP7_INT       0x80

//INT4          
#define PMU_INT4_VBUS_VLD_F     0x01
#define PMU_INT4_VA_SESS_VLD_F  0x02
#define PMU_INT4_VB_SESS_END_F  0x04
#define PMU_INT4_OTG_A          0x08
#define PMU_INT4_VBUS_VLD_R     0x10
#define PMU_INT4_VA_SESS_VLD_R  0x20
#define PMU_INT4_VB_SESS_VLD_R  0x40
#define PMU_INT4_OTG_B          0x80

// INT5         
#define PMU_INT5_ID_CHG       0x01
#define PMU_INT5_CHGDET_LATCH 0x02
#define PMU_INT5_CHGDET_TO    0x04
#define PMU_INT5_RID_C_TO_FLT 0x08
#define PMU_INT5_ADP_CHANGE   0x10
#define PMU_INT5_ADP_SNS_END  0x20
#define PMU_INT5_RESUME_VBUS  0x40
#define PMU_INT5_RESUME_VWALL 0x80

// INT6         
#define PMU_INT6_BATINS    0x01
#define PMU_INT6_BATRM     0x02
#define PMU_INT6_MBTEMPLOW 0x04
#define PMU_INT6_MBTEMPHI  0x08
#define PMU_INT6_CHGOV     0x10
#define PMU_INT6_USBOV     0x20
#define PMU_INT6_MBOV_DIS  0x40
#define PMU_INT6_USBOV_DIS 0x80

// INT7         
#define PMU_INT7_CHG_SW_TMR_EXP 0x01
#define PMU_INT7_CHG_HW_TMR_EXP 0x02
#define PMU_INT7_VBUSLOWBND     0x04
#define PMU_INT7_MBC_TF         0x08
#define PMU_INT7_MBWV_R_10S_WAIT 0x10
#define PMU_INT7_MBOV           0x20
#define PMU_INT7_BBLOW          0x40
#define PMU_INT7_FGC            0x80

// INT8         
#define PMU_INT8_RTC_ALARM       0x01	
#define PMU_INT8_RTC_SEC         0x02	
#define PMU_INT8_RTC_MIN         0x04	
#define PMU_INT8_RTCADJ          0x08
#define PMU_INT8_AUD_HSAB_SC     0x10
#define PMU_INT8_AUD_IHFD_SC     0x20	
#define PMU_INT8_VBOVRI          0x40	
#define PMU_INT8_VBOVRV          0x80

// INT9         
#define PMU_INT9_RTM_DATA_RDY    0x01	
#define PMU_INT9_RTM_DURING_CONT 0x02	
#define PMU_INT9_RTM_UPPER_BOUND 0x04	
#define PMU_INT9_RTM_IGNORE      0x08
#define PMU_INT9_RTM_OVERRIDDEN  0x10	
#define PMU_INT9_XTAL_FAIL       0x20	
#define PMU_INT9_ADP_PROB        0x40	
#define PMU_INT9_RESERVED_BIT7 0x80

// INT10        
#define PMU_INT10_CSROVRI       0x01
#define PMU_INT10_IOSROVRI      0x02
#define	PMU_INT10_SDSROVRI	    0x04
#define	PMU_INT10_CAMLDOOVRI	0x08
#define	PMU_INT10_RFLDOOVRI	    0x10
#define	PMU_INT10_HVLDO1OVRI	0x20
#define	PMU_INT10_HVLDO2OVRI	0x40
#define	PMU_INT10_HVLDO3OVRI	0x80

// INT11        
#define PMU_INT11_HVLDO4OVRI    0x01
#define PMU_INT11_HVLDO5OVRI    0x02
#define	PMU_INT11_HVLDO6OVRI    0x04
#define	PMU_INT11_HVLDO7OVRI    0x08
#define	PMU_INT11_SIMLDOOVRI	0x10
#define	PMU_INT11_CAMLDO_SHD    0x20
#define	PMU_INT11_RFLDO_SHD 	0x40
#define	PMU_INT11_HVLDO1_SHD	0x80

// INT12
#define	PMU_INT12_HVLDO2_SHD   0x01
#define PMU_INT12_HVLDO3_SHD   0x02
#define PMU_INT12_HVLDO4_SHD   0x04
#define	PMU_INT12_HVLDO5_SHD   0x08
#define	PMU_INT12_HVLDO6_SHD   0x10
#define	PMU_INT12_HVLDO7_SHD   0x20
#define	PMU_INT12_SIMLDO_SHD   0x40
#define	PMU_INT12_RESERVED_BIT7 0x80

// INT13        
#define PMU_INT13_DBI_NOPINT   0x01

// INT14
#define PMU_INT14_DBI_ACCINT   0x01
#define PMU_INT14_DBI_DREC     0x02
#define	PMU_INT14_DBI_DSENT    0x04
#define	PMU_INT14_DBI_SPDSET   0x08
#define	PMU_INT14_DBI_COMERR   0x10
#define	PMU_INT14_DBI_FRAERR   0x20
#define	PMU_INT14_DBI_RESERR   0x40
#define	PMU_INT14_DBI_COLL     0x80

// MBCCTRL bit fields
#define PMU_MBCCTRL1_SW_EXP_SEL_MASK  0x03
#define PMU_MBCCTRL1_SW_TMR_EN   0x04
#define PMU_MBCCTRL1_SW_TMR_CLR   0x10
#define PMU_MBCCTRL2_BTEMPSWH_EN  0x01
#define PMU_MBCCTRL2_BTEMPSWL_EN  0x02
#define PMU_MBCCTRL2_BTEMPHWH_EN  0x04
#define PMU_MBCCTRL2_BTEMPHWL_EN  0x08

#define PMU_MBCCTRL3_WAC_HOSTEN  0x01  
#define PMU_MBCCTRL3_USB_HOSTEN  0x02  
#define PMU_MBCCTRL3_ADP_PRI     0x04
#define PMU_MBCCTRL3_MBOV_EN     0x08
#define PMU_MBCCTRL4_SW_CTRL_SWITCH  0x01
#define PMU_MBCCTRL4_SW_EOC  0x02
#define PMU_MBCCTRL4_TF_CTRL   0x04
#define PMU_MBCCTRL4_TF_SW_EN  0x08
#define PMU_MBCCTRL5_BC11_EN 0x01
#define PMU_MBCCTRL5_BCDLDO_AON  0x02
#define PMU_MBCCTRL5_USB_DET_LDO_EN 0x04
#define PMU_MBCCTRL5_DIS_RID_FLOAT  0x08
#define PMU_MBCCTRL5_CHG_TYP_MASK 0x30
#define PMU_MBCCTRL5_CHG_TYP_NOT_ACT 0x0
#define PMU_MBCCTRL5_CHG_TYP_SDP 0x10
#define PMU_MBCCTRL5_CHG_TYP_CDP 0x20
#define PMU_MBCCTRL5_CHG_TYP_DCP 0x30
#define PMU_MBCCTRL6_VFLOATMAX_LOCK 0x10
#define PMU_MBCCTRL6_VFLOATMAX_MASK 0x0F
#define PMU_MBCCTRL6_VFLOATMAX_420MV 0x08
#define PMU_MBCCTRL6_VFLOATMAX_430MV 0x0C
#define PMU_MBCCTRL7_VFLOAT_MASK    0x0F 
#define PMU_MBCCTRL7_VFLOAT_390MV   0x01
#define PMU_MBCCTRL7_VFLOAT_400MV   0x02
#define PMU_MBCCTRL7_VFLOAT_405MV   0x03
#define PMU_MBCCTRL7_VFLOAT_410MV   0x04
#define PMU_MBCCTRL7_VFLOAT_415MV   0x06
#define PMU_MBCCTRL7_VFLOAT_420MV   0x08
#define PMU_MBCCTRL8_ICCMAX_LOCK 0x08
#define PMU_MBCCTRL8_ICCMAX_MASK 0x07
#define PMU_MBCCTRL8_ICCMAX_900MA 0x07
#define PMU_MBCCTRL9_SWUP  0x01
#define PMU_MBCCTRL9_DBP   0x02
#define PMU_MBCCTRL10_USB_FC_CC_MASK  0x07
#define PMU_MBCCTRL11_WAC_FC_CC_MASK  0x07
#define PMU_MBCCTRL24_TF_EN_ATE  0x20


// OTG section
#define PMU_OTGCTRL1_VBUS_PULSE   0x01
#define PMU_OTGCTRL1_PD_SRP       0x02
#define PMU_OTGCTRL1_OFFVBUSB     0x04
#define PMU_OTGCTRL1_EN_ID_IN     0x08
#define PMU_OTGCTRL1_EN_SESS_VLD  0x10
#define PMU_OTGCTRL1_VBUS_RBUS    0x20
#define PMU_OTGCTRL1_OTG_SHDN     0x40
#define PMU_OTGCTRL1_EN_SRP_COMPS 0x80
#define PMU_OTGCTRL2_ADP_COMP_DB_MASK 0x03
#if defined(BCM59055_B0)
#define PMU_OTGCTRL2_VBUS_RBUS_SEL 0x04      // B0 chip
#define PMU_OTGCTRL2_ADP_PRB_CYCLE_TIME 0x08 // B0 chip
#define PMU_OTGCTRL2_ADP_SNS       0x10      // B0 chip
#define PMU_OTGCTRL2_VBS_ADP_COMP_DB_MASK 0x03
#endif

#define PMU_OTGSTS1_ADP_PRB_COMP  0x01
#define PMU_OTGSTS1_ADP_SNS_COMP  0x02
#define PMU_OTGSTS1_ADP_DSCHG_COMP  0x04
#define PMU_OTGSTS1_ADP_ATTACH_DET  0x08
#define PMU_OTGSTS1_ADP_SNS_DET     0x10

#define PMU_OTGCTRL4_ID_REF1_2MSB_MASK 0x03
#define PMU_OTGCTRL4_ID_REF2_6LSB_MASK 0xFC
#define PMU_OTGCTRL5_ID_REF2_4MSB_MASK 0x0F
#define PMU_OTGCTRL5_ID_REF3_4LSB_MASK 0xF0
#define PMU_OTGCTRL6_ID_REF3_6MSB_MASK 0x3F
#define PMU_OTGCTRL6_ID_REF4_2LSB_MASK 0xC0
#define PMU_OTGCTRL8_ID_REF_SEL_MASK   0x60
#define PMU_OTGCTRL9_ADP_THR_TIME_REF_MASK 0x3F
#define PMU_OTGCTRL9_TPROBE_MAX_MSB_MASK 0xC0
#define PMU_OTGCTRL11_ADP_PRB 0x01
#if !defined(BCM59055_B0)
#define PMU_OTGCTRL11_ADP_SNS 0x02
#define PMU_OTGCTRL11_ADP_ONE_SHOT 0x04
#else
#define PMU_OTGCTRL11_ADP_PRB_MODE_MASK 0x06
#define PMU_OTGCTRL11_ADP_CALIBRATE 0x02
#define PMU_OTGCTRL11_ADP_ONE_SHOT 0x04
#endif
#define PMU_OTGCTRL11_ADP_THR_RATIO_MASK 0xF8
#define PMU_OTGCTRL11_ADP_THR_CTRL 0x10
#define PMU_OTGCTRL12_EN_ADP_PRB_COMP 0x01
#define PMU_OTGCTRL12_EN_ADP_SNS_COMP 0x02
#define PMU_OTGCTRL12_EN_ADP_SNS_AON  0x04
#define PMU_OTGCTRL12_EN_ADP_SHUTDOWNB 0x08
#define PMU_OTGCTRL12_EN_ADP_PRB_REG_RST 0x10
#if !defined(BCM59055_B0)
#define PMU_OTGCTRL12_EN_ADP_COMP_METHOD 0x20
#define PMU_OTGCTRL12_EN_ADP_THR_CTRL 0x40
#else
#define PMU_OTGCTRL12_EN_ADP_COMP_METHOD 0x20

#define PMU_OTGCTRL12_EN_ADP_COMP_METHOD_MASK 0x60
#define PMU_OTGCTRL12_EN_ADP_THR_CTRL    0x80
#endif

// ADC SECTION
#define PMU_ADCCTRL1_MAX_RESET_COUNT_MASK 0x03
#define PMU_ADCCTRL1_RTM_START 0x04
#define PMU_ADCCTRL1_RTM_MASK 0x08
#define PMU_ADCCTRL1_RTM_SELECT_MASK 0xF0
#define PMU_ADCCTRL2_RTM_DELAY_MASK 0x1F
#define PMU_ADCCTRL2_GSM_DEBOUNCE 0x20
#define PMU_ADCCTRLX_ADC_DATA_BIT9_8_MASK 0x03
#define PMU_ADCCTRLX_ADC_DATA_INVALID 0x04

// LDO SECTION
#define PMU_XLDOCTRL_VOUT_MASK 0x07  //for RF,SIM,CAM,HV1-7

// switcher SECTION
#define PMU_XSRCTRLX_VOUT_MASK 0x1F  //for CSR,IOSR and SDSR

// DBI section
#define PMU_DBICTRL_DBIEN    0x01

// PWMLED SECTION
#define PMU_PWMLEDCTRL1_HWSW_CTRL 0x01
#define PMU_PWMLEDCTRL1_PWMLED_PD 0x02
#define PMU_PWMLEDCTRL1_REPEAT 0x04
#define PMU_PWMLEDCTRL1_VMBAT_DRIVER 0x08
#define PMU_PWMLEDCTRL1_MODE_SELECT  0x10
#define PMU_PWMLEDCTRL1_PWM_FREQ_MASK 0x60
#define PMU_PWMLEDCTRL1_PWM_ERR_INDICATION 0x80
#define PMU_PWMLEDCTRL6_LEN_WAIT_LO_MSB_MASK 0x07
#define PMU_PWMLEDCTRL8_LEN_WAIT_HI_MSB_MASK 0x07
#define PMU_PWMLEDCTRLX_LEN_RAMP_MSB_MASK 0x07
#define PMU_PWMLEDCTRLX_SLOPE_RAMP_MASK 0x18

//FG SECTION
#define PMU_FGCTRL1_FGHOSTEN	0x01
#define PMU_FGCTRL2_FGTRIM      0x01
#define PMU_FGCTRL2_FGCAL       0x02
#define PMU_FGCTRL2_LONGCAL     0x04
#define PMU_FGCTRL2_FG1PTCAL    0x08
#define PMU_FGCTRL2_FGRESET     0x10
#define PMU_FGCTRL2_FGFRZREAD   0x20
#define PMU_FGCTRL2_FGFRZSMPL   0x40
#define PMU_FGCTRL2_FGFORCECAL  0x80

#define PMU_FGOPMODCTRL_FGSYNCMODE  0x10
#define PMU_FGOPMODCTRL_FGMODON     0x20

#define PMU_FGCICCTRL1_FGFRZSMPL    0x04
#define PMU_FGACCM1_FGACCM_MASK     0x03
#define PMU_FGACCM1_FGRDVALID       0x80
#define PMU_FGBATCAL2_FGVMBAT9_8_MASK 0x03

#define PMU_FGOFFSET_MASK	 	    0xFFFF
#define PMU_FGCOUNT_MASK            0xFFFF
#define PMU_FGSMPL2_MASK			0x3F  // Revisit
#define PMU_FGSMPL_MASK_SIGN		0x8000
#define PMU_FGSMPL_VALUE			0x7FFF
#define PMU_FGOFFSET_MASK_SIGN		0x8000
#define PMU_FGOFFSET_VALUE			0x7FFF
#define PMU_FGSMPL_MULTIPLIER		976/1000    
#define PMU_FGACCM_VALUE_MASK		0x1FFFFFF


#define PMU_PAGESEL_MASK 0x03

//Revisit
#define CHRG_WDT_MSB					0x10	// CHWD MSB


// ENV1
#define PMU_ENV1_MBWV          0x01    
#define PMU_ENV1_MBWV_DELTA    0x02
#define PMU_ENV1_ERC           0x04
#define PMU_ENV1_MBMCb          0x08
#define PMU_ENV1_MBOV           0x10
#define PMU_ENV1_MBUV           0x20
#define PMU_ENV1_BSIWV          0x40
#define PMU_ENV1_BBLOWb         0x80  // low backup battery level detected (debounce)

// ENV2
#define PMU_ENV2_CGPD            0x01
#define PMU_ENV2_UBPD            0x02
#define PMU_ENV2_UBPD_USBDET     0x04
#define PMU_ENV2_UBPD_INT        0x08
#define PMU_ENV2_CGPD_PRI        0x10    
#define PMU_ENV2_UBPD_PRI        0x20
#define PMU_ENV2_WAC_VALID       0x40
#define PMU_ENV2_USB_VALID       0x80

// ENV3
#define PMU_ENV3_CGPD_CHG       0x01
#define PMU_ENV3_UBPD_CHG       0x02
#define PMU_ENV3_CGMBC          0x04
#define PMU_ENV3_UBMBC          0x08
#define PMU_ENV3_CHGOV          0x10
#define PMU_ENV3_USBOV          0x20
#define PMU_ENV3_ACP7_DET       0x40
#define PMU_ENV3_USB_DISABLE    0x80

// ENV4
#define PMU_ENV4_VBUS_VALID     0x01
#define PMU_ENV4_A_SESS_VALID   0x02
#define PMU_ENV4_B_SESS_END     0x04
#define PMU_ENV4_ID_IN_MASK     0x38
#define PMU_ENV4_OTG_ID         0x40

// ENV5
#define PMU_ENV5_MBPD           0x01
#define PMU_ENV5_MBTEMPLOW      0x02
#define PMU_ENV5_MBTEMPHI       0x04
#define PMU_ENV5_PONKEYB        0x08
#define PMU_ENV5_ACD            0x10
#define PMU_ENV5_CHIP_TOOHOT    0x20
#define PMU_ENV5_PONCTRL        0x40

// ENV6
#define PMU_ENV6_MBCERROR       0x01    
#define PMU_ENV6_OFFVBUSb       0x02    // voffbus status
#define PMU_ENV6_OTGSHDNb       0x04    // otgblock status
#define PMU_ENV6_ADPSHDNb       0x08

// ENV7
#define PMU_ENV7_WAC_RVP        0x01
#define PMU_ENV7_USB_RVP        0x02
#define PMU_ENV7_MBC_UIb        0x04
#define PMU_ENV7_MBC_CV         0x08
#define PMU_ENV7_TRUE_TF        0x10
#define PMU_ENV7_STAT1          0x40
#define PMU_ENV7_STAT2          0x80

// ENV8
#define PMU_ENV8_CGPD_WAKE      0x01
#define PMU_ENV8_UBPD_WAKE      0x02
#define PMU_ENV8_NTC_ALARM_WAKE 0x04
#define PMU_ENV8_AUXON_WAKE     0x08
#define PMU_ENV8_POK_WAKE       0x10
#define PMU_ENV8_GBAT_PLUGIN    0x20
#define PMU_ENV8_DIS_CHG_SHDN   0x40
#define PMU_ENV8_DIS_CHG_XTAL_FAIL 0x80

// ENV9
#define PMU_ENV9_MBUV_SHDN  0x01
#define PMU_ENV9_THSD_SHDN  0x02
#define PMU_ENV9_MBWV_SHDN  0x04
#define PMU_ENV9_SYS_WDT_EXP_SHDN  0x08
#define PMU_ENV9_SW_SHDN    0x10
#define PMU_ENV9_LONG_POK_SHDN  0x20
#define PMU_ENV9_SR_OVRI_SHDN   0x40
#define PMU_ENV9_XTAL_FAIL_SHDN 0x80

// ENV10
#define PMU_ENV10_ADP_PRB_COMP  0x01
#define PMU_ENV10_ADP_SNS_COMP  0x02
#define PMU_ENV10_ADP_DSCH_COMP 0x04
#define PMU_ENV10_ADP_ATTACH_DET 0x08
#define PMU_ENV10_ADP_SNS_DET    0x10

// ENV11
#define PMU_ENV11_TPROBE_INIT_LSB_MASK    0xFF

// ENV12
#define PMU_ENV12_TPROBE_RISE1_LSB_MASK    0xFF

// ENV13
#define PMU_ENV13_TPROBE_RISE2_LSB_MASK    0xFF

// ENV14
#define PMU_ENV14_TPROBE_INIT_MSB_MASK  0x03
#define PMU_ENV14_TPROBE_RISE1_MSB_MASK 0x0C
#define PMU_ENV14_TPROBE_RISE2_MSB_MASK 0x30

// ENV15
#define PMU_ENV15_ADP_THR_TIME_MASK 0x3F
#define PMU_ENV15_ADP_DSCHG_FAIL    0x40
#define PMU_ENV15_ADP_CHG_FAIL      0x80


// SIMLDOCTRL
#define PMU_SIMLDOCTRL_VSELMASK     0x07
#define PMU_SIMLDOCTRL_1V8          0x01
#define PMU_SIMLDOCTRL_2V5          0x02
#define PMU_SIMLDOCTRL_3V0          0x06
#define PMU_SIMLDOCTRL_3V3          0x07
#define PMU_SIMLDOCTRL_SIMOFF_EN    0x40

// PWM and LED
#define PMU_LEDON_MASK	    0x03
#define PMU_LEDON			0x01 //0x03
#define PMU_PWMON			0x02
#define PMU_SYSCLK_MASK	    0x0C
#define PMU_SYSCLK_4		0x00
#define PMU_SYSCLK_16		0x04
#define PMU_SYSCLK_64		0x08
#define PMU_SYSCLK_512		0x0C
#define PMU_PWMLED_PDN      0x40
#define PMU_PWMLED1		    0x02
#define PMU_PWMLED2		    0x01

// PONKEY bit fields
#define PMU_PONKEYBDB_ONHOLD_MASK    0x07
#define PMU_PONKEYBDB_PONKEYBRF_MASK 0x38
#define PMU_PONKEYBDB_KEYLOCK        0x40    // bit value for keylock bit
#define PMU_ACDDB_PONKEYBDEL_MASK    0x70    // shutdown delay mask
#define PMU_ACDDB_PONKEYBDEL_MIN     0x30 // 2 sec min delay for keylock
#define PMU_PONKEYBDB1_OFFHOLD_MASK  0x07    // bit value for keylock bit

// AUXONb bit fields
#define PMU_AUXCTRL_DEBOUNCE_MASK    0x03

#define MAX_NUMBER_LDO		10

//******************************************************************************
// Typedefs
//******************************************************************************

// PMU Interrupts IDs  ---------------------------------------------------------------
typedef enum {
    PMU_IRQID_INT1_POK_PRESS,       
    PMU_IRQID_INT1_POK_RELEASE,     
    PMU_1RQID_INT1_POK_WAKEUP,      
    PMU_1RQID_INT1_POK_BIT_VLD,     
    PMU_1RQID_INT1_POK_SHDWN,       
    PMU_1RQID_INT1_AUX_INS,         
    PMU_1RQID_INT1_AUX_RM,          
    PMU_1RQID_INT1_GBAT_IN,         
     
    PMU_IRQID_INT2_CHGINS,         
    PMU_IRQID_INT2_CHGRM,          
    PMU_IRQID_INT2_CHGOV_DIS,   
#if !defined(BCM59055_B0)
    PMU_IRQID_INT2_RESERVED_BIT3,      
#else
    PMU_IRQID_INT2_WAC_CC_REDUCED,
#endif
    PMU_IRQID_INT2_USBINS,         
    PMU_IRQID_INT2_USBRM,          
    PMU_IRQID_INT2_UBPD_CHP_DIS,        
#if !defined(BCM59055_B0)
    PMU_IRQID_INT2_RESERVED_BIT7,       
#else
    PMU_IRQID_INT2_USB_CC_REDUCED,
#endif
    PMU_IRQID_INT3_WAC_REV_POL,    
    PMU_IRQID_INT3_USB_REV_POL,    
    PMU_IRQID_INT3_MBC_CV_LOOP,    
    PMU_IRQID_INT3_CV_TMR_EXP,    
    PMU_IRQID_INT3_CGPD_CHG_F,     
    PMU_IRQID_INT3_UBPD_CHG_F,     
    PMU_IRQID_INT3_CHGERRDIS,      
    PMU_IRQID_INT3_ACP7_INT,       

    PMU_IRQID_INT4_VBUS_VLD_F,    
    PMU_IRQID_INT4_VA_SESS_VLD_F, 
    PMU_IRQID_INT4_VB_SESS_END_F, 
    PMU_IRQID_INT4_OTG_A,      // previously ID insert
    PMU_IRQID_INT4_VBUS_VLD_R,    
    PMU_IRQID_INT4_VA_SESS_VLD_R, 
    PMU_IRQID_INT4_VB_SESS_END_R, 
    PMU_IRQID_INT4_OTG_B,    // previously ID remove    

    PMU_IRQID_INT5_ID_CHG,       
    PMU_IRQID_INT5_CHGDET_LATCH, 
    PMU_IRQID_INT5_CHGDET_TO,    
    PMU_IRQID_INT5_RID_C_TO_FLT, 
    PMU_IRQID_INT5_ADP_CHANGE,   
    PMU_IRQID_INT5_ADP_SNS_END,  
    PMU_IRQID_INT5_RESUME_VBUS,  
    PMU_IRQID_INT5_RESUME_VWALL, 

    PMU_IRQID_INT6_BATINS,    
    PMU_IRQID_INT6_BATRM,     
    PMU_IRQID_INT6_MBTEMPLOW, 
    PMU_IRQID_INT6_MBTEMPHI,  
    PMU_IRQID_INT6_CHGOV,     
    PMU_IRQID_INT6_USBOV,     
    PMU_IRQID_INT6_MBOV_DIS,  
    PMU_IRQID_INT6_USBOV_DIS, 

    PMU_IRQID_INT7_CHG_SW_TMR_EXP, 
    PMU_IRQID_INT7_CHG_HW_TMR_EXP, 
    PMU_IRQID_INT7_VBUSLOWBND,     
    PMU_IRQID_INT7_MBC_TF,         
    PMU_IRQID_INT7_MBWV_R_10S_WAIT,
    PMU_IRQID_INT7_MBOV,           
    PMU_IRQID_INT7_BBLOW,          
    PMU_IRQID_INT7_FGC,  

    PMU_IRQID_INT8_RTC_ALARM,         
    PMU_IRQID_INT8_RTC_SEC,           
    PMU_IRQID_INT8_RTC_MIN,           
    PMU_IRQID_INT8_RTCADJ,          
    PMU_IRQID_INT8_AUD_HSAB_SC,     
    PMU_IRQID_INT8_AUD_IHFD_SC,       
    PMU_IRQID_INT8_VBOVRI,            
    PMU_IRQID_INT8_VBOVRV,          

    PMU_IRQID_INT9_RTM_DATA_RDY,     
    PMU_IRQID_INT9_RTM_DURING_CONT,  
    PMU_IRQID_INT9_RTM_UPPER_BOUND,  
    PMU_IRQID_INT9_RTM_IGNORE,      
    PMU_IRQID_INT9_RTM_OVERRIDDEN,   
    PMU_IRQID_INT9_XTAL_FAIL,    
    PMU_IRQID_INT9_ADP_PROB,    
    PMU_IRQID_INT9_ADP_PROB_ERR, // ADP PRobe error

    PMU_IRQID_INT10_CSROVRI,      
    PMU_IRQID_INT10_IOSROVRI,     
    PMU_IRQID_INT10_SDSROVRI,	    
    PMU_IRQID_INT10_CAMLDOOVRI,	
    PMU_IRQID_INT10_RFLDOOVRI,	
    PMU_IRQID_INT10_HVLDO1OVRI,	
    PMU_IRQID_INT10_HVLDO2OVRI,	
    PMU_IRQID_INT10_HVLDO3OVRI,	

    PMU_IRQID_INT11_HVLDO4OVRI,   
    PMU_IRQID_INT11_HVLDO5OVRI,   
    PMU_IRQID_INT11_HVLDO6OVRI,
    PMU_IRQID_INT11_HVLDO7OVRI,
    PMU_IRQID_INT11_SIMLDOOVRI,	
    PMU_IRQID_INT11_CAMLDO_SHD,   
    PMU_IRQID_INT11_RFLDO_SHD, 	
    PMU_IRQID_INT11_HVLDO1_SHD,

    PMU_IRQID_INT12_HVLDO2_SHD,	
    PMU_IRQID_INT12_HVLDO3_SHD,   
    PMU_IRQID_INT12_HVLDO4_SHD,   
    PMU_IRQID_INT12_HVLDO5_SHD,   
    PMU_IRQID_INT12_HVLDO6_SHD,
    PMU_IRQID_INT12_HVLDO7_SHD,
    PMU_IRQID_INT12_SIMLDO_SHD,   
    PMU_IRQID_INT12_RESERVED_BIT7,

    PMU_IRQID_INT13_DBI_NOPINT,
    PMU_IRQID_INT13_DBI_RESERVED_BIT1,
    PMU_IRQID_INT13_DBI_RESERVED_BIT2,
    PMU_IRQID_INT13_DBI_RESERVED_BIT3,
    PMU_IRQID_INT13_DBI_RESERVED_BIT4,
    PMU_IRQID_INT13_DBI_RESERVED_BIT5,
    PMU_IRQID_INT13_DBI_RESERVED_BIT6,
    PMU_IRQID_INT13_DBI_RESERVED_BIT7,

    PMU_IRQID_INT14_DBI_ACCINT,  
    PMU_IRQID_INT14_DBI_DREC,  
    PMU_IRQID_INT14_DBI_DSENT,   
    PMU_IRQID_INT14_DBI_SPDSET,  
    PMU_IRQID_INT14_DBI_COMERR,
    PMU_IRQID_INT14_DBI_FRAERR,
    PMU_IRQID_INT14_DBI_RESERR, 
    PMU_IRQID_INT14_DBI_COLL, 
    
    PMU_TOTAL_IRQ
} PMU_InterruptId_t;


// PMU revison ID
typedef enum {
    PMU_REVISION_ID_A0,               // Rev A0
    PMU_REVISION_ID_B0,				  // Rev B0
    PMU_REVISION_ID_C0,               // Rev C0
    PMU_REVISION_ID_UN_READ = 0xFF
} PMU_revisionID_t;

//CON_TYPE related
typedef enum {
    PMU_USB_NONE,   
    PMU_USB_SDP,    // USB standard downstream port e.g. PC
    PMU_USB_CDP,    // USB charging downstream port
    PMU_USB_DCP,    // USB dedicated charging port
    PMU_USB_SDP_LIMITED // like SDP, but with limited or no charging capability, e.g. cell phone
} USB_Connector_Type_t;

//adaptor priority related
typedef enum {
    ADAPTOR_PRI_USB,                // usb has priority
    ADAPTOR_PRI_WALL                // wall has priority
} Adaptor_Pri_t;


//OTG role
typedef enum {
    OTG_A_DEV,        // otg A device
    OTG_B_DEV         // otg B device or no cable
} OTG_Role_t;

typedef enum {
	PMU_OTG_ID_GND = 0,		///< Grounded
	PMU_OTG_ID_B,           ///< Legacy float
    PMU_OTG_ID_RESERVE1,	///< Id float
	PMU_OTG_ID_RID_A,		///< 
	PMU_OTG_ID_RID_B,		///< 
	PMU_OTG_ID_RID_C,		///<    
    PMU_OTG_ID_RESERVE2,
    PMU_OTG_ID_FLOAT        ///< Rid_float
} PMU_OTG_Id_t;

// OTG stat
typedef enum {
    OTG_BOOST_STAT,        // otg boost regulator status
    OTG_BLOCK_STAT         // otg block status
} OTG_Stat_t;

// OTG monitor status
typedef struct{
    UInt16 TprobeInit;
    UInt16 TprobeRise1;	
    UInt16 TprobeRise2;
} ADP_Tprobe_t;

// Enumeration_Status
typedef enum {
    PRE_ENUM,       // before enumeration: for cable disconnect, or USB cable connected but enumeration has
                    // not started.
    ENUM_STARTED,   // enumeration has started
    ENUM_DONE       // enumeration has finished (and cable is still connected)
} Enumeration_Status_t;


// Charger watchdog timer
typedef enum {
	PMU_MBCCTRL1_CHWD_TIMER_4S,
	PMU_MBCCTRL1_CHWD_TIMER_8S,
	PMU_MBCCTRL1_CHWD_TIMER_16S,
	PMU_MBCCTRL1_CHWD_TIMER_32S
}	Charger_WD_Timer_t;

// LED Typedef enums ------------------------------------------------------------
// Revisit,  Peter Feng's team takes care of this
typedef enum{
    PMULED_Cyc_0p4s = 0x00,             ///< Add comments here
    PMULED_Cyc_1s   = 0x01,
    PMULED_Cyc_1p2s = 0x02,
    PMULED_Cyc_2s   = 0x03,
    PMULED_Cyc_2p6s = 0x04,
    PMULED_Cyc_4s   = 0x05,
    PMULED_Cyc_6s   = 0x06,
    PMULED_Cyc_8s   = 0x07
} PMULedCycle_t;

typedef enum{
    PMULED_Patt_On50msOff                   = 0x00,      ///< Add comments here
    PMULED_Patt_On100msOff                  = 0x08,
    PMULED_Patt_On200msOff                  = 0x10,
    PMULED_Patt_On500msOff                  = 0x18,
    PMULED_Patt_On50msOff50msOn50msOff      = 0x20,
    PMULED_Patt_On100msOff100msOn100msOff   = 0x28,
    PMULED_Patt_On200msOff200msOn200msOff   = 0x30,
    PMULED_Patt_On                          = 0x38
} PMULedPattern_t;

typedef enum
{
    PMULED_ID1 = PMU_REG_PWMLEDCTRL1,
    PMULED_ID2 = PMU_REG_PWMLEDCTRL6,
    PMULED_ID_MAX
} PMULedID_t;

//--- for PWM
typedef enum{
    PMUPWM_ID1 = PMU_REG_PWMLEDCTRL1,
    PMUPWM_ID2 = PMU_REG_PWMLEDCTRL6
} PMUPwmID_t;


//use operation character '|' to select muti-operation mode like
//PMULED_Oper_Save | PMULED_Oper_Standby | PMULED_Oper_Active
typedef UInt8   PMULedOperMode_t;
#define PMULED_Oper_Off     0x00
#define PMULED_Oper_Charger 0x40
#define PMULED_Oper_Active  0x80

// Used for BBCCharge maintenance
#define BBC_STATE_ON    1
#define BBC_STATE_OFF   0
#define DEFAULT_BBC_INIT_ON_DURATION   4 * 60 * 60 * TICKS_ONE_SECOND  // 4 hours
#define DEFAULT_BBC_ON_DURATION     1 * 60 * 60 * TICKS_ONE_SECOND  // 1 hour
#define DEFAULT_BBC_OFF_DURATION    2 * 60 * 60 * TICKS_ONE_SECOND  // 2 hours


/*
 * broadcast event handler definition
 */
//typedef void (*HAL_BroadcastEvent_handler_t)(HAL_EM_PMU_Event_en_t eventID, void *data1, void *data2);							
//typedef void (*HAL_BroadcastEvent_handler_t)(void *eventID);
typedef void (*HAL_BroadcastIdChangEvent_handler_t)(PMU_OTG_Id_t prevID, PMU_OTG_Id_t currID);

// System watchdog control
typedef enum{
    PMU_SYS_WDT_CTRL_EN = 0,
    PMU_SYS_WDT_CTRL_SHDN = 1
} PMU_SYS_WDT_CTRL_t;

// Main battery charger EOC current
typedef enum{
    PMU_EOCS_50MA,
    PMU_EOCS_60MA,
    PMU_EOCS_70MA,
    PMU_EOCS_80MA,
    PMU_EOCS_90MA,
    PMU_EOCS_100MA,
    PMU_EOCS_110MA,
    PMU_EOCS_120MA,
    PMU_EOCS_130MA,
    PMU_EOCS_140MA,
    PMU_EOCS_150MA,
    PMU_EOCS_160MA,
    PMU_EOCS_170MA,
    PMU_EOCS_180MA,
    PMU_EOCS_190MA,
    PMU_EOCS_200MA
} PMU_EOCS_t;

// Maintenance charge voltage level
typedef enum{
    PMU_MBMCVS_3P95,
    PMU_MBMCVS_4P00,
    PMU_MBMCVS_4P05,
    PMU_MBMCVS_4P10
} PMU_MBMCVS_t;

// charging current
typedef enum{
    PMU_CC_100MA,
    PMU_CC_350MA,
    PMU_CC_500MA,
    PMU_CC_600MA,
    PMU_CC_800MA,
    PMU_CC_900MA
} PMU_CC_t;

// charging voltage
typedef enum{
    PMU_CV_3600MV,
    PMU_CV_3900MV,
    PMU_CV_4000MV,
    PMU_CV_4050MV,
    PMU_CV_4100MV,
    PMU_CV_4125MV,
    PMU_CV_4150MV,
    PMU_CV_4175MV,
    PMU_CV_4200MV,
    PMU_CV_4225MV,
    PMU_CV_4250MV,
    PMU_CV_4275MV,
    PMU_CV_4300MV,
    PMU_CV_4325MV,
    PMU_CV_4350MV,
    PMU_CV_4375MV
} PMU_CV_t;

// Backup battery charger related voltage, current, and resistor controls
typedef enum{
    PMU_BBCVS_2P5,
    PMU_BBCVS_3P0,
    PMU_BBCVS_3P3,
    PMU_BBCVS_NoChange = 0xff
} PMU_BBCVS_t;

typedef enum{
    PMU_BBCRS_1P2K,
    PMU_BBCRS_2P4K,
    PMU_BBCRS_3P6K,
    PMU_BBCRS_4P8K,
    PMU_BBCRS_NoChange = 0xff
} PMU_BBCRS_t;

typedef enum{
    PMU_BBCCS_10UA = 0,
    PMU_BBCCS_20UA = 1,
    PMU_BBCCS_50UA = 2,
    PMU_BBCCS_100UA = 3,
    PMU_BBCCS_200UA = 4,
    PMU_BBCCS_400UA = 5,
    PMU_BBCCS_NoChange = 0xff
} PMU_BBCCS_t;


typedef enum {
	EOC_DC_0_PERCENT = 0,
	EOC_DC_3_PERCENT,
	EOC_DC_6_PERCENT,
	EOC_DC_9_PERCENT,
	EOC_DC_12_PERCENT,
	EOC_DC_15_PERCENT,
	EOC_DC_18_PERCENT,
	EOC_DC_21_PERCENT,
	EOC_DC_24_PERCENT,
	EOC_DC_27_PERCENT,
	EOC_DC_30_PERCENT,
	EOC_DC_33_PERCENT,
	EOC_DC_36_PERCENT,
	EOC_DC_39_PERCENT,
	EOC_DC_42_PERCENT,
	EOC_DC_45_PERCENT,
	EOC_DC_48_PERCENT,
	EOC_DC_51_PERCENT,
	EOC_DC_54_PERCENT,
	EOC_DC_57_PERCENT,
	EOC_DC_60_PERCENT,
	EOC_DC_63_PERCENT,
	EOC_DC_66_PERCENT,
	EOC_DC_69_PERCENT,
	EOC_DC_72_PERCENT,
	EOC_DC_76_PERCENT,
	EOC_DC_80_PERCENT,
	EOC_DC_84_PERCENT,
	EOC_DC_88_PERCENT,
	EOC_DC_92_PERCENT,
	EOC_DC_96_PERCENT,
	EOC_DC_100_PERCENT
} PMU_EOC_DC_t;


typedef enum {
	PMU_NO_CHARGER_IS_INUSE = 0,		///< No charger is in use
	PMU_USB_IS_INUSE,					///< USB charger/cable is in use
	PMU_WALL_IS_INUSE					///< WALL charger is in use
} PMU_ChargerInUse_t;


typedef enum {
	PMU_RFLDOCTRL = 0,
	PMU_CAMLDOCTRL,
	PMU_HVLDO1CTRL,
	PMU_HVLDO2CTRL,
	PMU_HVLDO3CTRL,
	PMU_HVLDO4CTRL,
	PMU_HVLDO5CTRL,
	PMU_HVLDO6CTRL,
	PMU_HVLDO7CTRL,
	PMU_SIMLDOCTRL
} PMU_LDO_t;


static UInt16 *pVoltAvgBeforeChargerOn;
static UInt16 *pBattPercentageLevel;
static UInt16 *pBatteryLevelNumber;
static UInt16 *pRunningVoltAvg;
static UInt16 *pCurrentBattLevel;

//*****************************************************************************
/**
*	PMU_UserPONKEYLock
*   @param setToOn (in) 		
*
*******************************************************************************/
void PMU_UserPONKEYLock(UInt8 setToOn);
//*****************************************************************************
/**
*	PMU_DRV_BBCCtrl
*   @param pmuBBCVSVal (in) 		
*   @param pmuBBCCSVal (in)
*   @param pmuBBCRSVal (in)
*
*******************************************************************************/
void PMU_DRV_BBCCtrl(PMU_BBCVS_t pmuBBCVSVal, PMU_BBCCS_t pmuBBCCSVal, PMU_BBCRS_t pmuBBCRSVal);
//*****************************************************************************
/**
*	PMU_DRV_BBCTimerReconfig
*   @param onHrInterval (in) 		
*   @param onMinInterval (in) 
*   @param offHrInterval (in) 
*   @param offMinInterval (in) 
*
*******************************************************************************/
void PMU_DRV_BBCTimerReconfig(UInt8 onHrInterval, UInt8 onMinInterval, UInt8 offHrInterval, UInt8 offMinInterval);


// Get revision ID
PMU_revisionID_t PMU_DRV_GetRevisionID( void );

// OTG functions
void PMU_DRV_UpdateOTGStat( OTG_Stat_t OTG_Stat, Boolean enable );
void PMU_DRV_HWCtrlOTG( Boolean enable );
//*****************************************************************************
/**
*	@param enable (in) 		
*
*******************************************************************************/
void PMU_DRV_CtrlOTGBlock( Boolean enable );
//*****************************************************************************
/**
*	@param enable (in) 		
*
*******************************************************************************/
void PMU_DRV_CtrlVBusBoost( Boolean enable );
//*****************************************************************************
/**
*	@param enable (in) 		
*
*******************************************************************************/
void PMU_DRV_CtrlVBusPulse( Boolean enable );
//*****************************************************************************
/**
*	PMU_DRV_CtrlSRPPullDown
*   @param enable (in) 		
*
*******************************************************************************/
void PMU_DRV_CtrlSRPPullDown( Boolean enable );
//*****************************************************************************
/**
*	PMU_DRV_CheckOtgState 		
*
*******************************************************************************/
void PMU_DRV_CheckOtgState( void );
//*******************************************************************************
/**
* PMU_DRV_GetVbusAndIDStat
* @return
*******************************************************************************/
UInt8 PMU_DRV_GetVbusAndIDStat(void);

//*******************************************************************************
/**
* PMU_DRV_SetSWUp
* 
*******************************************************************************/
void PMU_DRV_SetSWUp( void );
//*****************************************************************************
/**
*	PMU_DRV_CtrlDetectLDO
*   @param start (in) 		
*
*******************************************************************************/
void PMU_DRV_CtrlDetectLDO( Boolean start );
//*******************************************************************************
/**
* PMU_DRV_IsDBP
* @return
*******************************************************************************/
Boolean PMU_DRV_IsDBP( void );
//*******************************************************************************
/**
* PMU_DRV_IsDetectLDO_On
* @return
*******************************************************************************/
Boolean PMU_DRV_IsDetectLDO_On( void );
//*******************************************************************************
/**
* PMU_DRV_IsBatteryWeak
* @return
*******************************************************************************/
Boolean PMU_DRV_IsBatteryWeak( void );

//*****************************************************************************
/**
*	PMU_GetUSBRCCurrentinMA
*   @param current (in) 		
*
*******************************************************************************/
void PMU_GetUSBRCCurrentinMA(UInt16* current);
//*****************************************************************************
/**
*	PMU_SetUSBRCPreEnumCurrent 		
*
*******************************************************************************/
void PMU_SetUSBRCPreEnumCurrent(void);
//*****************************************************************************
/**
*	PMU_SetUSBRCEnumCurrent 		
*
*******************************************************************************/
void PMU_SetUSBRCEnumCurrent(void);
//*****************************************************************************
/**
*	PMU_GetUSBRCEnumCurrent 		
*   @param value (in)
*
*******************************************************************************/
void PMU_GetUSBRCEnumCurrent(UInt8* value);
//*****************************************************************************
/**
*	PMU_GetUSBRCEnumCurrentinMA 		
*   @param current (in)
*
*******************************************************************************/
void PMU_GetUSBRCEnumCurrentinMA(UInt16* current);
void PMU_DRV_SetEnumStarted( void );
void PMU_DRV_ClearUSBOverICount( void );
//*****************************************************************************
/**
*	PMU_SetChargingCurrent 		
*   @param isUSBCharger (in)
*   @param current (in)
*
*******************************************************************************/
void PMU_SetChargingCurrent(UInt8 isUSBCharger, UInt8 current);
//*****************************************************************************
/**
*	PMU_GetChargingCurrent 
*   @param isUSBCharger (in)		
*   @param value (in)
*
*******************************************************************************/
void PMU_GetChargingCurrent(UInt8 isUSBCharger, UInt8* value);
//*****************************************************************************
/**
*	PMU_SetChargingVoltage 		
*   @param isUSBCharger (in)
*   @param voltage (in)
*
*******************************************************************************/
void PMU_SetChargingVoltage(UInt8 isUSBCharger, UInt8 voltage);
//*****************************************************************************
/**
*	PMU_GetChargingVoltage 
*   @param isUSBCharger (in)		
*   @param value (in)
*
*******************************************************************************/
void PMU_GetChargingVoltage(UInt8 isUSBCharger, UInt8* value);
//*****************************************************************************
/**
*	PMU_MaxChargingVoltage 
*   @param voltage (in)
*
*******************************************************************************/
void PMU_MaxChargingVoltage(UInt8 voltage);
//*****************************************************************************
/**
*	PMU_MaxChargingCurrent 
*   @param current (in)
*
*******************************************************************************/
void PMU_MaxChargingCurrent(UInt8 isUSBCharger, UInt8 current);

void PMU_EnableRapidCharging(UInt8 isUSBCharger, UInt8 enable);
//*****************************************************************************
/**
*	PMU_EnableCharging 
*   @param isUSBCharger (in)		
*   @param enable (in)
*
*******************************************************************************/
void PMU_EnableCharging(UInt8 isUSBCharger, UInt8 enable);
//*****************************************************************************
/**
*	PMU_SetEndOfChargeCurrent 
*   @param current (in)
*
*******************************************************************************/
void PMU_SetEndOfChargeCurrent(UInt8 current);
//*****************************************************************************
/**
*	PMU_GetEndOfChargeCurrent 
*   @param value (in)
*
*******************************************************************************/
void PMU_GetEndOfChargeCurrent(UInt8* value);
//*****************************************************************************
/**
*PMU_DRV_ClearChargerWatchdog
*
*******************************************************************************/
void PMU_DRV_ClearChargerWatchdog(void);
//*****************************************************************************
/**
*PMU_DRV_ClearWatchdog
*
*******************************************************************************/
void PMU_ClearWatchdog(void);

//*****************************************************************************
/**
*PMU_PetPmuWdt
*
*******************************************************************************/
void PMU_PetPmuWdt (void);

//*****************************************************************************
/**
*	PMU_EnableWatchdog 
*   @param enable (in)
*
*******************************************************************************/
void PMU_EnableWatchdog( Boolean enable );

//*****************************************************************************
/**
*	PMU_DRV_CtrlWatchdog 
*   @param enable (in)
*
*******************************************************************************/
void PMU_DRV_CtrlWatchdog(Boolean enable);

//*****************************************************************************
/**
*	PMU_EnableBBC 
*   @param enable (in)
*
*******************************************************************************/
void PMU_EnableBBC(UInt8 enable);
void PMU_GetBBCParams(void);
//*****************************************************************************
/**
*	PMU_SetOnkeyDebounceTime 
*   @param isHold (in)		
*   @param debounceTime (in)
*
*******************************************************************************/
void PMU_SetOnkeyDebounceTime(UInt8 isHold, UInt8 debounceTime);
//*****************************************************************************
/**
*	PMU_GetOnkeyDebounceTime 
*   @param isHold (in)		
*   @param debounceTime (in)
*
*******************************************************************************/
void PMU_GetOnkeyDebounceTime(UInt8 isHold, UInt8 *debounceTime);
//*****************************************************************************
/**
*	PMU_IsNTCBlockOn 
*   @param isNTCOn (in)		
*
*******************************************************************************/
void PMU_IsNTCBlockOn(UInt8 *isNTCOn);
//*****************************************************************************
/**
*	PMU_DRV_CtrlChargerWatchdog 
*   @param enable (in)		
*   @param interval (in)
*
*******************************************************************************/
void PMU_DRV_CtrlChargerWatchdog(Boolean enable, Charger_WD_Timer_t interval);
//*****************************************************************************
/**
*	PMU_DRV_ReconfigSysWDT 
*   @param interval (in)		
*
*******************************************************************************/
void PMU_DRV_ReconfigSysWDT(UInt8 interval);
//*****************************************************************************
/**
*	@param pmuLDO (in) 	
*	@param voltage (in) 	
*
*******************************************************************************/
void PMU_SetLDOVoltage(PMU_LDO_t pmuLDO, UInt8 voltage);
//*****************************************************************************
/**
*	@param pmuLDO (in) 	
*	@param voltage (in) 	
*
*******************************************************************************/
void PMU_GetLDOVoltage(PMU_LDO_t pmuLDO, UInt8 *voltage);
//*****************************************************************************
/**
*	PMU_SetCoreSwitcherVoltage 
*   @param isNormalMode (in)		
*   @param voltage (in)
*
*******************************************************************************/
void PMU_SetCoreSwitcherVoltage(UInt8 isNormalMode, UInt8 voltage);
//*****************************************************************************
/**
*	PMU_GetCSRVoltage 
*   @param isNormalMode (in)		
*   @param voltage (in)
*
*******************************************************************************/
void PMU_GetCSRVoltage(UInt8 isNormalMode, UInt8 *voltage);
//*****************************************************************************
/**
*	PMU_SetLDOMode 
*   @param pmuLDO (in)		
*   @param mode (in)
*
*******************************************************************************/
void PMU_SetLDOMode(PMU_LDO_t pmuLDO, UInt8 mode);
//*****************************************************************************
/**
*	PMU_GetLDOMode 
*   @param pmuLDO (in)		
*   @param mode (in)
*
*******************************************************************************/
void PMU_GetLDOMode(PMU_LDO_t pmuLDO, UInt8 *mode);
//*****************************************************************************
/**
*	PMU_isSIMInitialized 
*   @param isSIMReady (in)		
*
*******************************************************************************/
void PMU_isSIMInitialized(UInt8 *isSIMReady);
//*****************************************************************************
/**
*	PMU_SetChargingAdaptorPriority 
*   @param isUsbCharger (in)		
*
*******************************************************************************/
void PMU_SetChargingAdaptorPriority(UInt8 isUsbCharger);
//*****************************************************************************
/**
*	PMU_GetChargingAdaptorPriority 
*   @param value (in)		
*
*******************************************************************************/
void PMU_GetChargingAdaptorPriority(UInt8* value);
//*****************************************************************************
/**
*	PMU_GetBBCLowThreshold 
*   @param bbcThreshold (in)		
*
*******************************************************************************/
void PMU_GetBBCLowThreshold(UInt8* bbcThreshold);
//*****************************************************************************
/**
*	PMU_DRV_CheckChargingState 
*
*******************************************************************************/
void PMU_DRV_CheckChargingState( void );
//*****************************************************************************
/**
*	PMU_DRV_SetUSBChargerType 
*   @param usbChargerType (in)		
*
*******************************************************************************/
void PMU_DRV_SetUSBChargerType(USB_Connector_Type_t usbChargerType);

// ADP
// ADP
//*****************************************************************************
/**
*	PMU_DRV_CtrlADPProbe 
*	Start or stop ADP probing one shot or continious. Also used for forced calibration 
*
*	Called by: Higher layers such as USB Stack
*	Calls: I2C driver (for writing to ADP registers)
*
*	Restriction: ??
*
*	@param Boolean adpStart (in) 
*	@param Boolean adpOneShot (in) 
*	@param Boolean adpCalibration (in)
*	@return void
*
*	@note Clients should call this routine through HAL layer wrapper 
*	(EM_PMU_ACTION_CTRL_ADP_PROBE)
*	adpStart: 
*		TRUE: Start the Probe
*		FALSE: Stop the Probe 
*	adpOneShot: ADP Probe mode;
*		TRUE: ADP ONE SHOT (it is a single probing cycle when ADP_PRB is set 
*			that compares and generates interrupt if attach or detach is detected)
*		FALSE: Continious
*	forceCalibration: Force the ADP Calibration before doing oneshot or continious probing
*		TRUE: Force the calibration(use this flag if Ref Probe is not done earlier or ADP registers are reset)
*		FALSE: Don't force the calibration(normal usage; assumes Ref Probe is done earlier) 
*	This call also enables OTG and ADP Block if they are not enabled previously
*******************************************************************************/
void PMU_DRV_CtrlADPProbe(Boolean adpStart, Boolean adpOneShot, Boolean forceCalibration);

//*****************************************************************************
/**
*	PMU_DRV_CtrlADPSense 
*	Start or stop ADP sensing; ADP sensing is to find out if remote device is doing ADP probing
*
*	Called by: Higher layers such as USB Stack
*	Calls: I2C driver (for writing to ADP registers)
*
*	Restriction: ??
*
*   @param   Boolean adpSnsStart
*   @param   Boolean adpSnsAON
*   @return  void
*	@note Clients should call this routine through HAL layer wrapper 
*	(EM_PMU_ACTION_CTRL_ADP_SENSE)
*	adpStart: 
*		TRUE: Start the Sensing
*		FALSE: Stop the Sensing 
*	adpSnsAON: ADP Probe mode;
*		TRUE: Perform continious ADP sense unitil directed to stop
*		FALSE: Perform ADP sense for only 5 sec (A0 chip) or 3.2 sec (B0 chip)
*	If we disable ADP sense, it does not disable ADP block or comparators
*	This call also enables OTG and ADP Block if they are not enabled previously
*******************************************************************************/

void PMU_DRV_CtrlADPSense(Boolean adpStart, Boolean adpSnsAON);

//*****************************************************************************
/**
*	PMU_DRV_CtrlADPBlock 
*	Enable or disable ADP block 
*
*	Called by: Higher layers such as USB Stack
*	Calls: I2C driver (for writing to ADP registers)
*
*	Restriction: ??
*
*   @param   Boolean enable
*   @return  void
*
*	@note Clients should call this routine through HAL layer wrapper 
*	(EM_PMU_ACTION_CTRL_ADP_BLOCK)
*	adpStart: 
*		TRUE: Enable ADP Block
*		FALSE: Disable ADP Block
*	Enables or Disables ADP/Sense comparators and ADP shutdown block
*	
*******************************************************************************/

void PMU_DRV_CtrlADPBlock(Boolean enable);


//*****************************************************************************
/**
*	PMU_DRV_GetADPProbeStatus 
*   	Return ADP probing status (whether it's attached or detached)
*
*	Called by: Higher layers such as USB Stack
*	Calls: None
*
*	@param void
*	@return  Boolean: TRUE: attached; FALSE: detached
*	@note Clients should call this routine through HAL layer wrapper 
*	(EM_PMU_ACTION_IS_ADP_ATTACHED)
*   
******************************************************************************/
Boolean PMU_DRV_GetADPProbeStatus(void);

//*****************************************************************************
/**
*	PMU_DRV_GetADPSenseStatus 
*   	Return ADP Sensing status (whether remote device is doing ADP probling or not)
*
*	Called by: Higher layers such as USB Stack
*	Calls: None
*
*	@param void
*	@return Boolean: TRUE: remote is doing ADP probing; FALSE: remote is not
*	@note Clients should call this routine through HAL layer wrapper 
*	(EM_PMU_ACTION_IS_REMOTE_ADP_PROBING)
*   
******************************************************************************/

Boolean PMU_DRV_GetADPSenseStatus(void);

//*****************************************************************************
/**
*	PMU_DRV_GetADPSenseSetting 
*   	Return ADP Sense setting status in the device
*
*	Called by: Higher layers such as USB Stack
*	Calls: I2C driver (for Reading ADP registers); blocking read
*
*	@param void
*	@return Boolean: TRUE: ADP Sense is started; FALSE: ADP Sense is stopped
*	@note Clients should call this routine through HAL layer wrapper 
*	(EM_PMU_ACTION_GET_ADP_SENSE_SETTING)
*		This API & the subsequent HAL action can be used by the client for direct control
*   
******************************************************************************/
Boolean PMU_DRV_GetADPSenseSetting(void);

//*****************************************************************************
/**
*	PMU_DRV_GetADPProbeSetting 
*   	Return ADP Probe setting status in the device
*
*	Called by: Higher layers such as USB Stack
*	Calls: I2C driver (for Reading ADP registers); blocking read
*
*	@param void
*	@return Boolean: TRUE: ADP Probe is started; FALSE: ADP Probe is stopped
*	@note Clients should call this routine through HAL layer wrapper 
*	(EM_PMU_ACTION_GET_ADP_PROBE_SETTING)
*		This API & the subsequent HAL action can be used by the client for direct control
*   
******************************************************************************/
Boolean PMU_DRV_GetADPProbeSetting(void);

//*****************************************************************************
/**
*	PMU_DRV_GetADPProbeMeasurement 
*   	Return ADP Probe measurement register values
*
*	Called by: Higher layers such as USB Stack
*	Calls:I2C driver (for Reading ADP registers); blocking read to Address map1 register
*
*	@param void
*	@return ADP_Tprobe_t: Structure containing TProbe Init/Rise1/Rise2 register values
*	@note Clients should call this routine through HAL layer wrapper 
*	(EM_PMU_ACTION_GET_ADP_PROBE_MEASUREMENT_REGISTER_VALUES)
*		This API & the subsequent HAL action can be used by the client for direct control
*   
******************************************************************************/

ADP_Tprobe_t PMU_DRV_GetADPProbeMeasurement(void);

//*****************************************************************************
/**
*	PMU_DRV_ResetADPRegs 
*   	Reset ADP registers
*
*	Called by: Higher layers such as USB Stack
*	Calls: Boolean
*
*	@param void
*	@return void
*	@note Clients should call this routine through HAL layer wrapper 
*	(EM_PMU_ACTION_CTRL_RESET_ADP_REGS)
*		This API & the subsequent HAL action can be used by the client for direct control
*   
******************************************************************************/
void PMU_DRV_ResetADPRegs(void);

//*****************************************************************************
/**
*	PMU_DRV_SetTPROBE_MAX 
*   	Set TProbemax value
*
*	Called by: Higher layers such as USB Stack
*	Calls: I2C driver (for Reading & writing to ADP registers); blocking read & non-blocking write
*
*	@param UInt16 Tprobe_max (MSB of TPROBE_MAX. 0x32Khz clock cycles to 
*		512x32Khz clock cycles (16ms) in 0.5x32Khz clock cycles steps)
*	@return void
*	@note Clients should call this routine through HAL layer wrapper 
*	(EM_PMU_ACTION_CTRL_SET_TPROBE_MAX)
*	This API & the subsequent HAL action can be used by the client for direct control
*   
******************************************************************************/
void PMU_DRV_SetTPROBE_MAX(UInt16 Tprobe_max);

//*****************************************************************************
/**
*	PMU_DRV_SetADPCompMethod 
*   	Set Probe comparison method
*
*	Called by: Higher layers such as USB Stack
*	Calls: I2C driver (for Reading & writing to ADP registers); blocking read & non-blocking write
*
*	@param Uint method
*                                  0/3 - TPRPBE_RISE2 compared with TPROBE_INT "OR"
*				      TPROBE_RISE2 compared with TPROBE_RISE1
*                                  1             - TPRPBE_RISE2 compared with TPROBE_RISE1
*                                  2             - TPRPBE_RISE2 compared with TPROBE_INT
*	@return void
*	@note Clients should call this routine through HAL layer wrapper 
*	(EM_PMU_ACTION_CTRL_SET_ADP_COMP_METHOD)
*	This API & the subsequent HAL action can be used by the client for direct control
*   
******************************************************************************/
void PMU_DRV_SetADPCompMethod(UInt16 method);

//*****************************************************************************
/**
*
*   Select VBUS and ADP comps debounce time  
*
*	@param Uint comp_db
*                                  0 - Not used
*                                  1 - 0.4ms
*                                  2 - 1.4ms
                                    3 - 2.7ms
*   @return  void
*	@note Clients should call this routine through HAL layer wrapper 
*	(EM_PMU_ACTION_CTRL_SELECT_ADP_COMP_DB)
*	This API & the subsequent HAL action can be used by the client for direct control
******************************************************************************/
void PMU_DRV_SelectADPCompDb(UInt16 comp_db);

//*****************************************************************************
/**
*	PMU_DRV_ADPRefProbe 
*	Start ADP Reference probe; 
*	
*	Called by: Higher layers such as USB Stack
*	Calls: I2C driver (for writing to ADP registers)
*
*	Restriction: ??
*
*	@param Boolean postEvent
*		TRUE: Post completion event to the client via HAL if they are registered
*		FALSE: Don't post completion event; just start the reference probe 	
*	@return void
**	@note Clients should call this routine through HAL layer wrapper 
*	(EM_PMU_ACTION_CTRL_ADP_REF_PROBE)
*	Reference probe does not generate interrupt; 
*	so the implementation sets the timer for TPROBE_RISE2 value
*	to reach its level.
*	On Reference probe completion (on timer expirty), post PMU_DRV_ADP_REF_PROBE 
*	event if postEvent is TRUE
*******************************************************************************/
void PMU_DRV_ADPRefProbe(Boolean postEvent);

//*****************************************************************************
/**
*	PMU_DRV_ADPSenseProbe 
*	Sense followed by Probe 
*
*	Called by: Higher layers such as USB Stack
*	Calls: I2C driver (for writing to ADP registers)
*
*	Restriction: ??
*
*	@param void
*	@return void
*
*	@note Clients should call this routine through HAL layer wrapper 
*	(EM_PMU_ACTION_CTRL_ADP_SENSE_PROBE)
*	Clients gets notified through PMU_DRV_ADP_CHANGE event
*	1) Do Ref probe if not done already
*	2) Do sense
*	3) on the sense interrupt, either sense again, or, probe continious
*	
*******************************************************************************/
void PMU_DRV_ADPSenseProbe(void);

//*****************************************************************************
/**
*	PMU_DRV_ADPStopProbe 
*	Stop the ADP probe 
*
*	Called by: Higher layers such as USB Stack
*	Calls: I2C driver (for writing to ADP registers)
*
*	Restriction: ??
*
*	@param void	
*	@return void
*
*	@note Clients should call this routine through HAL layer wrapper 
*	(EM_PMU_ACTION_CTRL_ADP_STOP_PROBE)
*	No specfic event posted. This API is guaranteed to stop the probing
*	Assert in implementation by reading it back to see if probing stopped.
*	
*	
*******************************************************************************/
void PMU_DRV_ADPStopProbe(void);

//*****************************************************************************
/**
*	PMU_DRV_ADPSetProbeCycle
*	Sets the ADP probe cycle length to short or long
*
*	Called by: Higher layers such as USB Stack
*	Calls: I2C driver (for writing to ADP registers)
*
*	Restriction: ??
*
*	@param Boolean shortCycle	
*		TRUE set Probing to  short Cycle 
*		FALSE set probing to long cycle ((default setting when powers up) 
*	@return void
*
*	@note Clients should call this routine through HAL layer wrapper 
*	(EM_PMU_ACTION_CTRL_ADP_SET_PROBE_CYCLE)
*	No specfic event posted. This API is guaranteed to set the probing cycle
*******************************************************************************/
void PMU_DRV_ADPSetProbeCycle(Boolean shortCycle);


//*****************************************************************************
/**
*	PMU_DRV_GetRTCAdjustmentNeededBootValue 
*   @return
*
*******************************************************************************/
Boolean PMU_DRV_GetRTCAdjustmentNeededBootValue(void);

Boolean PMU_IsUsbPlugIn(void);

//int PMU_DRV_BroadcastEventRegister(HAL_BroadcastEvent_handler_t callback);
//int PMU_DRV_BroadcastPrivateEventRegister(HAL_BroadcastEvent_handler_t callback);
int PMU_DRV_BroadcastIdChangeEventRegister(HAL_BroadcastIdChangEvent_handler_t callback);
int PMU_DRV_GetPMUSysparmCBRegister(UInt8 (*callback)(UInt16 index));
int PMU_DRV_GetPMUUsbRCCurrentCBRegister(UInt8 (*callback)(void));
int PMU_DRV_GetLDOSettingCBRegister(UInt32  (*callback)(void));
void PMU_SetOnkeyValidityTimerTime(UInt8 shutdownDelayTime);
void PMU_GetOnkeyValidityTimerTime(UInt8 *shutdownDelayTime);
void PMU_SetAUXDebounceTime(UInt8 debounceTime);
void PMU_GetAUXDebounceTime(UInt8 *debounceTime);
Boolean PMU_DRV_IsBusPowered(void);
PMU_ChargerInUse_t PMU_DRV_GetChargerInUse ( void );
void PMU_SetPWM(
    PMUPwmID_t  PwmId,
    UInt8       DutyCycle,//DutyCycle%
    Boolean     StatusOn
    );
void PMU_DRV_ReadFuelGaugeSample( UInt8 type, short *fgSmpl );
#endif
