/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
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
*   @file   pmu_bcm59038.h
*
*   @brief  This file includes memory map definitions for Broadcom PMU BCM59038.
*           Contains specific macros, typedefs, and defs specific for BCM59038.
*
****************************************************************************/

//******************************** History *************************************
//
// $Log:  $
// Initial Ver. 08-03-2009     Ismail Syed
//******************************************************************************

//******************************************************************************
//                          definition block
//******************************************************************************
#ifndef _PMU_BCM59038_H_
#define _PMU_BCM59038_H_

// PMU REGISTERS MAPPING --------------------------------------------------------

#define PMU_REG_I2CCNTRL       	0x00
			
#define PMU_REG_HOSTACT       	0x01
#define PMU_REG_SMPLSET       	0x02
#define PMU_REG_PMUGID        	0x03

#define PMU_REG_PLLCTRL       	0x0B
#define PMU_REG_PONKEYBDB     	0x0C
#define PMU_REG_ACDDB        	0x0D
#define PMU_REG_PONKEYBDB1      0x0E

#define PMU_REG_CMPCTRL1        0x10
#define PMU_REG_CMPCTRL2        0x11
#define PMU_REG_CMPCTRL3        0x12
#define PMU_REG_CMPCTRL4        0x13
#define PMU_REG_CMPCTRL5        0x14
#define PMU_REG_CMPCTRL6        0x15
#define PMU_REG_CMPCTRL7        0x16
#define PMU_REG_CMPCTRL8        0x17
#define PMU_REG_CMPCTRL9        0x18
#define PMU_REG_CMPCTRL10       0x19
#define PMU_REG_CMPCTRL11       0x1A
//Added for NTC control operation
#define PMU_REG_CMPCTRL12       0x1B

#define PMU_REG_RTCSC        	0x20
#define PMU_REG_RTCMN        	0x21
#define PMU_REG_RTCHR        	0x22
#define PMU_REG_RTCWD        	0x23
#define PMU_REG_RTCDT        	0x24
#define PMU_REG_RTCMT        	0x25
#define PMU_REG_RTCYR        	0x26
#define PMU_REG_RTCSC_A1        0x27
#define PMU_REG_RTCMN_A1        0x28
#define PMU_REG_RTCHR_A1        0x29
#define PMU_REG_RTCWD_A1        0x2A
#define PMU_REG_RTCDT_A1        0x2B
#define PMU_REG_RTCMT_A1        0x2C
#define PMU_REG_RTCYR_A1        0x2D
#define PMU_REG_RTC_EXSAVE      0x2E
#define PMU_REG_BBCCTRL        	0x2F
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

#define PMU_REG_INT1M        	0x3A
#define PMU_REG_INT2M        	0x3B
#define PMU_REG_INT3M        	0x3C
#define PMU_REG_INT4M        	0x3D
#define PMU_REG_INT5M        	0x3E
#define PMU_REG_INT6M        	0x3F
#define PMU_REG_INT7M        	0x40
#define PMU_REG_INT8M        	0x41
#define PMU_REG_INT9M        	0x42
#define PMU_REG_INT10M          0x43

#define PMU_REG_OTGCTRL1        0x44
#define PMU_REG_OTGCTRL2        0x45
#define PMU_REG_OTGCTRL3        0x46
#define PMU_REG_OTGCTRL4        0x47
#define PMU_REG_OTGCTRL5        0x48
#define PMU_REG_OTGCTRL6        0x49
#define PMU_REG_OTGCTRL7        0x4A
#define PMU_REG_OTGCTRL8        0x4B
#define PMU_REG_MBCCTRL1        0x4C
#define PMU_REG_MBCCTRL2        0x4D
#define PMU_REG_MBCCTRL3        0x4E
#define PMU_REG_MBCCTRL4        0x4F
#define PMU_REG_MBCCTRL5        0x50
#define PMU_REG_MBCCTRL6        0x51
#define PMU_REG_MBCCTRL7        0x52
#define PMU_REG_MBCCTRL8        0x53
#define PMU_REG_MBCCTRL9        0x54
#define PMU_REG_MBCCTRL10       0x55

#define PMU_REG_MBCCTRL11       0x56
#define PMU_REG_MBCCTRL12       0x57
#define PMU_REG_MBCCTRL13       0x58
#define PMU_REG_MBCCTRL14       0x59
#define PMU_REG_MBCCTRL15       0x5A
#define PMU_REG_MBCCTRL16       0x5B

#define PMU_REG_A1OPMODCTRL     0x60
#define PMU_REG_A2OPMODCTRL     0x61
#define PMU_REG_A3OPMODCTRL     0x62
#define PMU_REG_A4OPMODCTRL     0x63
#define PMU_REG_A5OPMODCTRL     0x64
#define PMU_REG_A6OPMODCTRL     0x65
#define PMU_REG_A7OPMODCTRL     0x66
#define PMU_REG_A8OPMODCTRL     0x67
#define PMU_REG_D1OPMODCTRL     0x68
#define PMU_REG_D2OPMODCTRL     0x69
#define PMU_REG_D3OPMODCTRL     0x6A
#define PMU_REG_D4OPMODCTRL     0x6B
#define PMU_REG_SDOPMODCTRL     0x6C
#define PMU_REG_LVOPMODCTRL     0x6D
#define PMU_REG_SIMOPMODCTRL    0x6E
#define PMU_REG_CSROPMODCTRL    0x6F
#define PMU_REG_IOSROPMODCTRL   0x70
#define PMU_REG_ALDO1CTRL       0x71
#define PMU_REG_ALDO2CTRL       0x72
#define PMU_REG_ALDO3CTRL       0x73
#define PMU_REG_ALDO4CTRL       0x74
#define PMU_REG_ALDO5CTRL       0x75
#define PMU_REG_ALDO6CTRL       0x76
#define PMU_REG_ALDO7CTRL       0x77
#define PMU_REG_ALDO8CTRL       0x78
#define PMU_REG_DLDO1CTRL       0x79
#define PMU_REG_DLDO2CTRL       0x7A
#define PMU_REG_DLDO3CTRL       0x7B
#define PMU_REG_DLDO4CTRL       0x7C
#define PMU_REG_SDLDOCTRL       0x7D
#define PMU_REG_LVLDOCTRL       0x7E
#define PMU_REG_SIMLDOCTRL      0x7F
#define PMU_REG_CSRCTRL        	0x80
#define PMU_REG_IOSRCTRL        0x81
#define PMU_REG_CSRCTRL1        0x82
#define PMU_REG_CSRCTRL2        0x83
#define PMU_REG_CSRCTRL3        0x84
#define PMU_REG_CSRCTRL4        0x85
#define PMU_REG_CSRCTRL5        0x86
#define PMU_REG_CSRCTRL6        0x87
#define PMU_REG_CSRCTRL7        0x88

#define PMU_REG_IOSRCTRL1       0x8A
#define PMU_REG_IOSRCTRL2       0x8B
#define PMU_REG_IOSRCTRL3       0x8C
#define PMU_REG_IOSRCTRL4       0x8D

#define PMU_REG_ADCCTRL1        0x90
#define PMU_REG_ADCCTRL2        0x91
#define PMU_REG_ADCCTRL3        0x92
#define PMU_REG_ADCCTRL4        0x93
#define PMU_REG_ADCCTRL5        0x94
#define PMU_REG_ADCCTRL6        0x95
#define PMU_REG_ADCCTRL7        0x96
#define PMU_REG_ADCCTRL8        0x97
#define PMU_REG_ADCCTRL9        0x98
#define PMU_REG_ADCCTRL10       0x99
#define PMU_REG_ADCCTRL11       0x9A
#define PMU_REG_ADCCTRL12       0x9B
#define PMU_REG_ADCCTRL13       0x9C
#define PMU_REG_ADCCTRL14       0x9D
#define PMU_REG_ADCCTRL15       0x9E
#define PMU_REG_ADCCTRL16       0x9F
#define PMU_REG_ADCCTRL17       0xA0
#define PMU_REG_ADCCTRL18       0xA1
#define PMU_REG_ADCCTRL19       0xA2
#define PMU_REG_ADCCTRL20       0xA3
#define PMU_REG_ADCCTRL21       0xA4
#define PMU_REG_ADCCTRL22       0xA5
#define PMU_REG_ADCCTRL23       0xA6
#define PMU_REG_ADCCTRL24       0xA7
#define PMU_REG_ADCCTRL25       0xA8
#define PMU_REG_ADCCTRL26       0xA9
#define PMU_REG_ADCCTRL27       0xAA
#define PMU_REG_ADCCTRL28       0xAB
#define PMU_REG_ADCCTRL29       0xAC

#define PMU_REG_IHFTOP        	0xB0
#define PMU_REG_IHFBIASCLK      0xB1
#define PMU_REG_IHFLDO        	0xB2
#define PMU_REG_IHFPOP        	0xB3
#define PMU_REG_IHFRCCALI       0xB4
#define PMU_REG_IHFADCI        	0xB5
#define PMU_REG_IHFSSP        	0xB6
#define PMU_REG_IHFSSPTST       0xB7
#define PMU_REG_IHFANARAMP      0xB8
#define PMU_REG_IHFPGA1        	0xB9
#define PMU_REG_IHFPGA2        	0xBA
#define PMU_REG_IHFLOOPFILTER   0xBB
#define PMU_REG_IHFCOMPPD       0xBC
#define PMU_REG_IHFDRIVER       0xBD
#define PMU_REG_IHFFBAMP        0xBE
#define PMU_REG_IHFSTIN        	0xBF
#define PMU_REG_IHFSTO        	0xC0
#define PMU_REG_IHFSCDADCO      0xC1
#define PMU_REG_IHFRCCALO       0xC2
#define PMU_REG_IHFRCCALRAW1O   0xC3
#define PMU_REG_IHFRCCALRAW2O   0xC4
#define PMU_REG_IHFMISC			0xC5

#define PMU_REG_HSCP1			0xC8
#define PMU_REG_HSCP2			0xC9
#define PMU_REG_HSCP3			0xCA
#define PMU_REG_HSDRV        	0xCB
#define PMU_REG_HSLDO        	0xCC
#define PMU_REG_HSLF        	0xCD
#define PMU_REG_HSPGA1        	0xCE
#define PMU_REG_HSPGA2        	0xCF
#define PMU_REG_HSPGA3        	0xD0
#define PMU_REG_HSRC        	0xD1
#define PMU_REG_HSPUP1        	0xD2
#define PMU_REG_HSPUP2        	0xD3
#define PMU_REG_HSIST        	0xD4
#define PMU_REG_HSOC1        	0xD5
#define PMU_REG_HSOC2        	0xD6
#define PMU_REG_HSOC3        	0xD7
#define PMU_REG_HSOC4        	0xD8
#define PMU_REG_HSOUT1        	0xD9
#define PMU_REG_HSOUT2        	0xDA
#define PMU_REG_HSOUT3        	0xDB
#define PMU_REG_HSOUT4        	0xDC
#define PMU_REG_HSOUT5        	0xDD
#define PMU_REG_HSOUT6        	0xDE
#define PMU_REG_HSOUT7        	0xDF

#define PMU_REG_ENV1        	0xF0
#define PMU_REG_ENV2        	0xF1
#define PMU_REG_ENV3        	0xF2
#define PMU_REG_ENV4        	0xF3
#define PMU_REG_ENV5        	0xF4
#define PMU_REG_ENV6        	0xF5

#define PMU_REG_PDCMPSYN0       0xF7
#define PMU_REG_PDCMPSYN1       0xF8
#define PMU_REG_PDCMPSYN2       0xF9
#define PMU_REG_PDCMPSYN3       0xFA
#define PMU_REG_PDCMPSYN4       0xFB
#define PMU_REG_PDCMPSYN5       0xFC

#define PMU_REG_PAGESEL0       	0xFF
#define PMU_REG_PAGESEL1        0xFF

#define PMU_REG_TOTAL			(PMU_REG_PAGESEL1 + 1)

// Register bitmap defines
#define PMU_I2CCNTRL_I2CHSEN			0x80
#define PMU_I2CCNTRL_I2CRDBLOCKEN		0x40
#define PMU_I2CCNTRL_I2C_HIGHSPEED		0x20
#define PMU_I2CCNTRL_I2CFILTEN			0x10
#define PMU_I2CCNTRL_I2CSLAVEID			0x0C
#define PMU_I2CCNTRL_FIFOFULL_R			0x02
#define PMU_I2CCNTRL_I2CRDBLOCK			0x01


#define PMU_HOSTACT_WDT_CLR				0x01
#define PMU_HOSTACT_WDT_ON_R			0x02
#define PMU_HOSTACT_HOSTDICOFF			0x04
#define PMU_HOSTACT_WDT_TIME			0x18


#define PMU_SMPLSET_SMPL_ON				0x01
#define PMU_SMPLSET_SMPL_TIME			0x0E

#define PMU_PMUGID_R					0xFF

#define	PMU_PONKEYBDB_BHOLD				0x07
#define	PMU_PONKEYBDB_BRF				0x38
#define	PMU_PONKEYBDB_KEYLOCK			0x40

#define	PMU_ACDDB_INS_DB				0x03
#define	PMU_ACDDB_RM_DB					0x0C
#define	PMU_ACDDB_PONKEYDEL				0x70

#define	PMU_PONKLEYBDB1_OFF_HOLD		0x07

#define	PMU_CMPCTRL1_LOWBATCVS			0x30

#define	PMU_CMPCTRL2_MAINT_TH			0x0C	//MBMCVS

#define	PMU_CMPCTRL10_DIE_WARM_TH		0x0C

#define	PMU_BBCCTRL_HOSTEN				0x01
#define	PMU_BBCCTRL_RES_SEL				0x06
#define	PMU_BBCCTRL_CUR_SEL				0x38
#define	PMU_BBCCTRL_VOL_SEL				0xC0

#define PMU_INT1_PONKEYBR			    0x01
#define PMU_INT1_PONKEYBF       		0x02
#define PMU_INT1_PONKEYBH       		0x04
#define PMU_INT1_SMPL	        		0x08
#define PMU_INT1_RTCA1          		0x10
#define PMU_INT1_RTC1S		    		0x20
#define PMU_INT1_RTC60S         		0x40
#define PMU_INT1_RTCADJ         		0x80

#define PMU_INT2_CHGINS			        0x01
#define PMU_INT2_CHGRM          		0x02
#define PMU_INT2_CHGOV         			0x04
#define PMU_INT2_EOC            		0x08
#define PMU_INT2_USBINS         		0x10
#define PMU_INT2_USBRM          		0x20
#define PMU_INT2_USBOV         			0x40
#define PMU_INT2_MBCCHGERR      		0x80

#define PMU_INT3_ACDINS			        0x01
#define PMU_INT3_ACDRM          		0x02
#define PMU_INT3_WALL_RESUME   			0x04
#define PMU_INT3_RTM_DATA_RDY      		0x08
#define PMU_INT3_RTM_WHILE_CONT    		0x10
#define PMU_INT3_RTM_UPPER_BOUND   		0x20
#define PMU_INT3_RTM_IGNORE     		0x40
#define PMU_INT3_RTM_OVERRIDDEN   		0x80

#define PMU_INT4_VBUS_VALID_F	        0x01
#define PMU_INT4_SES_A_VALID_F     		0x02
#define PMU_INT4_SES_B_END_F    		0x04
#define PMU_INT4_ID_INSERT        		0x08
#define PMU_INT4_VBUS_VALID_R      		0x10
#define PMU_INT4_SES_A_VALID_R     		0x20
#define PMU_INT4_SES_B_END_R    		0x40
#define PMU_INT4_ID_REMOVE      		0x80

#define PMU_INT5_ID_CHANGE		        0x01
#define PMU_INT5_CHGDET_DONE       		0x02
#define PMU_INT5_CHGDET_TIMEOUT 		0x04
#define PMU_INT5_RID_C_TO_FLOAT    		0x08
#define PMU_INT5_VBUS_LOWBOUND    		0x10
#define PMU_INT5_RESUME_VBUS      		0x20
#define PMU_INT5_AUD_HSAB_SHTCKT		0x40
#define PMU_INT5_AUD_IHFD_SHTCKT  		0x80

#define PMU_INT6_BBLOW			        0x01
#define PMU_INT6_MBC_TF          		0x02
#define PMU_INT6_CHGERR_DIS    			0x04
#define PMU_INT6_CHG_WDT_EXP       		0x08
#define PMU_INT6_IOSR_OVRI         		0x10
#define PMU_INT6_CSR_OVRI          		0x20
#define PMU_INT6_IOSR_OVRV     			0x40
#define PMU_INT6_CSR_OVROV	    		0x80

#define PMU_INT7_MBTEMP_FAULT	        0x01
#define PMU_INT7_MBTEMP_LOW        		0x02
#define PMU_INT7_MBTEMP_HIGH   			0x04
#define PMU_INT7_MBOV            		0x08
#define PMU_INT7_BATINS         		0x10
#define PMU_INT7_BATRM          		0x20
#define PMU_INT7_LOWBAT       			0x40
#define PMU_INT7_VERYLOWBAT      		0x80

//ENV1
#define	PMU_ENV1_UBMBC					0x01
#define	PMU_ENV1_P_CGPD					0x02
#define	PMU_ENV1_P_UBPD					0x04
#define	PMU_ENV1_MBWV					0x08
#define	PMU_ENV1_CHGOV					0x10
#define	PMU_ENV1_LOWBAT					0x20
#define	PMU_ENV1_BBATUVB				0x40
#define	PMU_ENV1_CGMBC					0x80

// ENV2
#define PMU_ENV2_PONKEYB           		0x01
#define PMU_ENV2_ACD            		0x02
#define PMU_ENV2_CHIP_TOOHOT       		0x08
#define PMU_ENV2_USBOV            		0x10
#define PMU_ENV2_P_UBPD_USBDET     		0x20
#define PMU_ENV2_TH_FOLD           		0x40
#define PMU_ENV2_MB_ERC            		0x80

//ENV3
#define PMU_ENV3_MBPD					0x01
#define PMU_ENV3_MBTEMPLOW				0x02
#define PMU_ENV3_MBTEMPHIGH				0x04
#define PMU_ENV3_VERYLOWBAT				0x40
#define PMU_ENV3_P_BAT_RM				0x80

//ENV4
#define PMU_ENV4_VBUS_VLD				0x01
#define PMU_ENV4_A_SESS_VLD				0x02
#define PMU_ENV4_B_SESS_END				0x04
#define PMU_ENV4_ID_IN					0x38
#define PMU_ENV4_OTGID					0x40

//ENV5
#define PMU_ENV5_P_CGPD_ONOFF			0x01
#define PMU_ENV5_P_UBPD_ONOFF			0x02
#define PMU_ENV5_EOC					0x04
#define PMU_ENV5_MBUV					0x08
#define PMU_ENV5_MB_MAINT_V				0x10
#define PMU_ENV5_MBOV					0x20
#define PMU_ENV5_P_CGPD_CHG				0x40
#define PMU_ENV5_P_UBPD_CHG				0x80

//ENV6
#define PMU_ENV6_MBC_ERR				0x01
#define PMU_ENV6_OFFVBUSB				0x02
#define PMU_ENV6_OTG_SHUTDOWN			0x04

//OTGCTRL1
#define PMU_OTGCTRL1_EN_VB_PULSE 		0x01
#define PMU_OTGCTRL1_EN_PD_SRP   		0x02
#define PMU_OTGCTRL1_OFFVBUSb			0x04
#define PMU_OTGCTRL1_IDIN_EN			0x08
#define PMU_OTGCTRL1_VA_SESS_EN			0x10
#define PMU_OTGCTRL1_EN_VBUS_RBUS		0x20
#define PMU_OTGCTRL1_OTG_SHUTDOWN		0x40

//OTGCTRL2
#define PMU_OTGCTRL2_VBUS_DEBOUNCE		0x03

//OTGCTRL3
#define	PMU_OTGCTRL3_ID_REF1_LSB		0xFF

//OTGCTRL4
#define	PMU_OTGCTRL4_ID_REF1_MSB		0x03
#define	PMU_OTGCTRL4_ID_REF2_LSB		0xFC

//OTGCTRL5
#define	PMU_OTGCTRL5_ID_REF2_MSB		0x0F
#define	PMU_OTGCTRL5_ID_REF3_LSB		0xF0

//OTGCTRL6
#define	PMU_OTGCTRL6_ID_REF3_MSB		0x3F
#define	PMU_OTGCTRL6_ID_REF4_LSB		0xC0

//OTGCTRL7
#define	PMU_OTGCTRL7_ID_REF4_MSB		0xFF

//OTGCTRL8
#define	PMU_OTGCTRL8_IDIN_SEL			0x1F
#define	PMU_OTGCTRL8_IDREF_SEL			0x60

//Audio bitmaps
#define PMU_HSPUP2_PWRUP				0x40
#define PMU_HSDRV_SC_ENABLE				0x10
#define PMU_HSDRV_SC_CURRENT_MASK		0x03
#define PMU_HS_GAIN_L					0x40
#define PMU_HS_GAIN_R					0x80
#define PMU_HS_GAIN_MASK				0x3F
#define PMU_HS_IDDQ_PWRDWN				0x10
#define PMU_PLL_EN						0x02

#define PMU_IHF_IDDQ					0x01
#define PMU_IHF_LDOPUP					0x01
#define PMU_IHF_POPPUP					0x08
#define PMU_IHF_AUTOSEQ					0x40
#define PMU_IHF_BYPASS					0x60
#define PMU_IHF_GAIN_MASK				0x3F

// SIMLDOCTRL
#define PMU_SIMLDOCTRL_SIMMASK   		0x03
#define PMU_SIMLDOCTRL_SIM_3V3    		0x03  
#define PMU_SIMLDOCTRL_SIM_3V0   		0x02
#define PMU_SIMLDOCTRL_SIM_2V5    		0x01
#define PMU_SIMLDOCTRL_SIM_1V8    		0x00

// PONKEY bit fields
#define PMU_PONKEYBDB_ONHOLD_MASK    	0x07
#define PMU_PONKEYBDB_PONKEYBRF_MASK 	0x38
#define PMU_PONKEYBDB_KEYLOCK        	0x40    // bit value for keylock bit
#define PMU_ACDDB_PONKEYBDEL_MASK    	0x70    // shutdown delay mask
#define PMU_ACDDB_PONKEYBDEL_MIN     	0x30 	 // 2 sec min delay for keylock
#define PMU_PONKEYBDB1_OFFHOLD_MASK  	0x07    // bit value for keylock bit

// SAR ADC bit fields
#define	PMU_ADCCTRL1_MAX_RTM_MASK		0x3		// Max Reset Count within 20ms time period
#define	PMU_ADCCTRL1_RTM_START			0x4		// Start RTM conversion
#define	PMU_ADCCTRL1_MASK_RTM			0x8		// Disable RTM conversion
#define	PMU_ADCCTRL1_RTM_CH_MASK		0xF0	// RTM Channel select mask

#define	PMU_ADCCTRL2_RTM_DLY_MASK		0x1F	// Time delay after conversion start
#define	PMU_ADCCTRL2_GSM_DEB_ENABLE 	0x20	// Enable GSM burst mode conversion

#define	PMU_ADC_MSB_DATA_MASK			0x3		// high 2 bits of the ADC value
#define	PMU_ADC_DATA_INVALID			0x4		// If set, ADC data is invalid


// HOSTACT
#define PMU_HOSTACT_WDT_CLR     		0x01
#define PMU_HOSTACT_WDT_ON      		0x02
#define PMU_HOSTACT_HOSTDICOFF  		0x04


//For the charging related....
// MBCCTRL bit fields
#define PMU_MBCCTRL2_VCHGRRC    0x04 //0x80    // wall charger rapid charge enable
#define PMU_MBCCTRL2_VUBGRRC    0x08 // USB charger rapid charge enable

#define PMU_MBCCTRL2_WMBCHOSTEN  0x01 //0x40    // WALL charger enable
#define PMU_MBCCTRL2_UMBCHOSTEN  0x02    // USB charger enable

#define PMU_MBCCTRL3_MAINTENANCE		0x06 // 4.1V for USB CV, and 3.6V for USB TC

#define PMU_MBCCTRL3_RC2_MASK   0x0F    // mask for wall RC2
#define PMU_MBCCTRL4_RC1_MASK   0x1F    // mask for wall RC1
#define PMU_MBCCTRL4_MBCWRC1_4  0x10    // wall charger RC1 high bit (bit 4)
#define PMU_MBCCTRL4_MBCWRC1_LOW_MASK 0x0F    // wall charger RC1 low bits (bit 3:0)
#define PMU_MBCCTRL4_MBCWTC1_4  0x20    // wall charger TC1 high bit (bit 4)                                        
#define PMU_MBCCTRL4_MBCUTC1_4  0x40    // usb charger TC1 high bit (bit 4)
// Constant Voltage for USB charger in BCM59038
#define PMU_MBCCTRL5_MAINTENANCE		0x06		// 4.1V for USB CV, and 3.6V for USB TC

#define PMU_MBCCTRL5_RC2_MASK   0x0F    // mask for usb RC2                                
#define PMU_MBCCTRL6_RC1_MASK   0x1F    // mask for usb RC1
#define PMU_MBCCTRL6_MBCURC1_4  0x10    // usb charger RC1 high bit (bit 4)
#define PMU_MBCCTRL6_MBCURC1_LOW_MASK 0x0F    // usb charger RC1 low bits (bit 3:0)
#define PMU_MBCCTRL11_MBCWTC1_LOW_MASK 0x0F    // usb charger RC1 low bits (bit 3:0)
#define PMU_MBCCTRL11_MBCUTC1_LOW_MASK 0xF0    // usb charger RC1 low bits (bit 3:0)
                                            
#define PMU_MBCCTRL7_EOCS_MASK  0x0F    // charger EOC field

#define PMU_MBCCTRL8_MBCFAULTCNT_MASK  0x0F    // mbc fault count mask
#define PMU_MBCCTRL8_PAUSECHARGE  0x20    // pause charge
//#define PMU_MBCCTRL8_VUBGRRC      0x40    // usb charger rapid charge enable
#define PMU_MBCCTRL8_TTRST        0x80    // total charge timer reset

//#define PMU_MBCCTRL9_MAINTCHRG   0x01    // maintenance charge enable
#define PMU_MBCCTRL2_MAINTCHRG   0x40    // maintenance charge enable
#define PMU_MBCCTRL9_CON_TYP     0x02    // connector type
#define PMU_MBCCTRL9_SP_TYP      0x04    // single connector type
#define PMU_MBCCTRL9_ADP_PRI     0x08    // adaptor priority bit
                                         
#define PMU_CMPCTRL12_NTCON   0x01 //PMU_MBCCTRL10_NTCON       0x01    // NTC LDO enable
//#define PMU_MBCCTRL10_BAT_DET_EN  0x02    // battery presence detection enable
#define PMU_CMPCTRL11_NTC_BDET_EN  0x01    // battery presence detection enable
//#define PMU_MBCCTRL10_BTEMP_EN    0x04    // battery temp detect enable
#define PMU_CMPCTRL11_NTC_BTEMP_EN    0x02    // battery temp detect enable
//#define PMU_MBCCTRL10_SYS_TYP     0x08    // system type bit: determine charger behavior when no battery
#define PMU_MBCCTRL9_SYS_TYP     0x01    // system type bit: determine charger behavior when no battery
#define PMU_MBCCTRL10_TC2CVCCSEL  0x10    // TC2 CV and CC select

// LOWBATCVS bit fields
#define PMU_LOWBATCVS_LOWBATCVS_MASK 0x03 // low main battery comparator threshold selector mask
#define PMU_LOWBATCVS_MBMCVS_MASK 0x0C // main battery maintenance charge comparator threshold selector mask
#define PMU_LOWBATCVS_MBMCVS_POS  2     // MBMCVS field starts at bit 2

// OTGCTRL bit fields
#define PMU_OTGCTRL1_EN_VB_PULSE 0x01    // Vbus pulsing enable/disable
#define PMU_OTGCTRL1_EN_PD_SRP   0x02    // active pull down control for SRP
#define PMU_OTGCTRL1_SHDN_EN     0x04    // control mapping GPIO1 pin to OTGSHD pin
#define PMU_OTGCTRL1_REDUCE_ID_CURR  0x08 // controls whether to reduce ID sense current
#define PMU_OTGCTRL1_OTG_SHUTDOWN 0x40   // USB OTG block enable/disable
#define PMU_OTGCTRL1_VBUSEN      0x80    // Vbus switching regulator enable/disable
#define PMU_OTGCTRL2_VBUSDB_MASK 0x03    // mask for Vbus comparator output debounce
#define PMU_OTGCTRL2_IDDB_MASK   0x0C    // mask for ID_IN comparator output debounce
#define PMU_OTGCTRL2_HWVSSW      0x10    // select whether hardware or software controls OTG
#define PMU_OTGCTRL2_BB_OTG_RST  0x20    // baseband resets OTG block: write 0 followed by 1
#define PMU_OTGCTRL2_VA_SESS_EN  0x40    // enable VA_SESS_VLD comparator
#define PMU_BOOSTCTRL4_EN_VBUS_RBUS  0x08 // Vbus 70k impedance enable/disable                                   
#define PMU_BOOSTCTRL4_EN_ID_COMP 0x10 // ID comparator enable/disable
#define PMU_DBGOTG1_VBUSBOOST    0x04    // reflects Vbus boost regulator status (on or off)
#define PMU_DBGOTG1_OTGBLOCK     0x08    // reflects OTG block status (enabled or disabled)

// BBCCTRL bit fields
#define PMU_BBCCTRL_BBCHOSTEN   0x01    // backup battery charger enable
#define PMU_BBCCTRL_BBCLOWIEN   0x40    // low charging current enable
#define PMU_BBCCTRL_BBCCS_MASK  0x4C    // mask for BBCCS field
#define PMU_BBCCTRL_BBCVS_MASK  0x30    // mask for BBCVS field
#define PMU_BBCCTRL_BBCRS_MASK  0x82    // mask for BBCRS field
#define PMU_BBCCTRL_BBCRS_P5K   0x00    // bit value for 1K resistor                                        
#define PMU_BBCCTRL_BBCRS_1K    0x02    // bit value for 1K resistor                                       
#define PMU_BBCCTRL_BBCRS_2K    0x80    // bit value for 1K resistor
#define PMU_BBCCTRL_BBCRS_4K    0x82    // bit value for 1K resistor
#define PMU_BBLOWDB_BBLOWCVS_MASK 0x03  // mask for BBLOWCVS field
/////////////// To be checked.


//******************************************************************************
// Typedefs 
//******************************************************************************

// PMU Interrupts IDs  ---------------------------------------------------------------
typedef enum {
    PMU_IRQID_INT1_PONKEYBR,      // R&C  on key rising
    PMU_IRQID_INT1_PONKEYBF,      // R&C  on key falling
    PMU_IRQID_INT1_PONKEYBH,      // R&C  on key pressed > hold debounce time
    PMU_IRQID_INT1_SMPL,          // R&C  one minute elapsed
    PMU_IRQID_INT1_RTCA1,         // R&C  alarm 1
	PMU_IRQID_INT1_RTC1S,         // R&C  SMPL
    PMU_IRQID_INT1_RTC60S,        // R&C  real-time clock needs adjustment
    PMU_IRQID_INT1_RTCADJ,        // R&C  one second elapsed

    PMU_IRQID_INT2_CHGINS,        // R&C  wall charger inserted (high clears CHGRM)
    PMU_IRQID_INT2_CHGRM,         // R&C  wall charger removed (high clears CHGINS)
    PMU_IRQID_INT2_CHGOV,         // R&C  wall charger error (V too high)
    PMU_IRQID_INT2_EOC,           // R&C  wall/usb charging done
    PMU_IRQID_INT2_USBINS,        // R&C  usb charger inserted (high clears USBRM)
    PMU_IRQID_INT2_USBRM,         // R&C  wall charger removed (high clears USBINS)
    PMU_IRQID_INT2_USBOV,         // R&C  usb charger error (V too high)
    PMU_IRQID_INT2_MBCCHGERR,     // R&C  main battery charge error (over time)

    PMU_IRQID_INT3_ACDINS,        // R&C  accessory inserted (high clears ACDRM)
    PMU_IRQID_INT3_ACDRM,         // R&C  accessory removed (high clears ACDINS)
    PMU_IRQID_INT3_RESUME_VWALL,  // R&C  Resume Wall charging
    PMU_IRQID_INT3_RTM_DATA_RDY,  // R&C  Real time measurement data ready interrupt
    PMU_IRQID_INT3_RTM_INTR_CONT, // R&C  RTM request while continuous measurement is running
    PMU_IRQID_INT3_RTM_MAX_REQ,   // R&C  RTM request reached maximum limit
    PMU_IRQID_INT3_RTM_IGNORE,    // R&C  RTM request ignored
    PMU_IRQID_INT3_RTM_OVERRIDE,  // R&C  RTM rquest over-ridden

    PMU_IRQID_INT4_VBUSVALID_F,   // R&C  Vbus valid falling edge
    PMU_IRQID_INT4_A_SESSVALID_F, // R&C  A session valid falling edge
    PMU_IRQID_INT4_B_SESSEND_F,   // R&C  B session end falling edge
    PMU_IRQID_INT4_ID_INSRT,      // R&C  ID inserted
    PMU_IRQID_INT4_VBUSVALID_R,   // R&C  Vbus valid rising edge
    PMU_IRQID_INT4_A_SESSVALID_R, // R&C  A session valid rising edge
    PMU_IRQID_INT4_B_SESSEND_R,   // R&C  B session end rising edge
    PMU_IRQID_INT4_ID_RMV,        // R&C  ID removed

    PMU_IRQID_INT5_ID_CHANGE,  	   // R&C ID change interrupt 
    PMU_IRQID_INT5_CHGDET_SAMPLED, // R&C BC1.1 charger detect sampled
    PMU_IRQID_INT5_CHGDET_TIMEOUT, // R&C BC1.1 charger detect time out
    PMU_IRQID_INT5_RID_C2F,   	   // R&C ID change from RID_C to Float
    PMU_IRQID_INT5_VBUS_LOWBOUND,  // R&C VBUS trip low bound
    PMU_IRQID_INT5_RESUME_VBUS,    // R&C resume VBUS interrupt
    PMU_IRQID_INT5_AUD_HS_SHORT,   // R&C headset short circuit
    PMU_IRQID_INT5_AUD_D_SHORT,    // R&C class D short circuit
 
    PMU_IRQID_INT6_BBLOW,   	   // R&C Backup battery low
    PMU_IRQID_INT6_MBC_TF,   	   // R&C Main battery thermal foldback
    PMU_IRQID_INT6_CHGERR_DIS,     // R&C charger error disappear
    PMU_IRQID_INT6_CHGWDT_EXP,     // R&C charger watchdog expired
    PMU_IRQID_INT6_CGPD_FALLING,   // R&C wall charger CGPD falling
    PMU_IRQID_INT6_UBPD_FALLING,   // R&C USB charger UBPD falling
    PMU_IRQID_INT6_RESERVED1,      // R&C reserved
    PMU_IRQID_INT6_RESERVED2,      // R&C reserved

    PMU_IRQID_INT7_MBTEMP_FAULT,   // R&C main battery temperature fault
    PMU_IRQID_INT7_MBTEMP_LOW,     // R&C main battery temperature low
    PMU_IRQID_INT7_MBTEMP_HIGH,    // R&C main battery temperature high
    PMU_IRQID_INT7_MBOV,  		   // R&C main batteyr over voltage
    PMU_IRQID_INT7_BATINS,   	   // R&C battery insertion
    PMU_IRQID_INT7_BATRM,  		   // R&C battery removal
    PMU_IRQID_INT7_LOWBAT,  	   // R&C low battery interrupt
    PMU_IRQID_INT7_VERY_LOWBAT,    // R&C very low battery interrupt

    PMU_TOTAL_IRQ
} PMU_InterruptId_t;
	
typedef enum{
	PMU_PAGESEL_0,
	PMU_PAGESEL_1
}PMU_pageSel_t;

//!!!!!!!!!!!!! STOP !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//!!!!!!!!!!!!! CHEKCK !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !! ALL the ENUMs and the values after initial bringup !!!

//CON_TYPE related
typedef enum {
    USB_WALL_ONECON,   // type 1 or 2: coming thru one connector
    STANDARD_USB_WALL_TWOCON,      // type 3: coming thru two connectors - dual connector
    //NON_MINI_USB,                  // type 1, 2, or 3
    MINI_USB_ONECON                // type 4: coming thru one connector
} Connector_Type_t;

//adaptor priority related
typedef enum {
    ADAPTOR_PRI_USB,                // usb has priority
    ADAPTOR_PRI_WALL                // wall has priority
} Adaptor_Pri_t;

//single connector type (when Con_type is 1)
typedef enum {
    SINGLE_CONN_USB_ADAPTOR,                // single connector, usb adaptor plugged in
    SINGLE_CONN_WALL_ADAPTOR                // single connector, wall adaptor plugged in
} Single_Conn_Type_t;

//sys_typ System type related
typedef enum {
    NORMAL_CHARGING_NO_BATT,             // normal charging without battery
    STOP_CHARGING_NO_BATT                // stop charging and go to PWRUP without battery
} System_Type_t;

//OTG role
typedef enum {
    OTG_A_DEV,        // otg A device
    OTG_B_DEV         // otg B device or no cable
} OTG_Role_t;

// OTG stat
typedef enum {
    OTG_BOOST_STAT,        // otg boost regulator status
    OTG_BLOCK_STAT         // otg block status
} OTG_Stat_t;

// Enumeration_Status 
typedef enum {
    PRE_ENUM,       // before enumeration: for cable disconnect, or USB cable connected but enumeration has
                    // not started.
    ENUM_STARTED,   // enumeration has started
    ENUM_DONE       // enumeration has finished (and cable is still connected)
} Enumeration_Status_t;

// Used for BBCCharge maintenance
#define BBC_STATE_ON    1
#define BBC_STATE_OFF   0
#define DEFAULT_BBC_INIT_ON_DURATION   4 * 60 * 60 * TICKS_ONE_SECOND  // 4 hours
#define DEFAULT_BBC_ON_DURATION     1 * 60 * 60 * TICKS_ONE_SECOND  // 1 hour
#define DEFAULT_BBC_OFF_DURATION    2 * 60 * 60 * TICKS_ONE_SECOND  // 2 hours

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

// Backup battery charger related voltage, current, and resistor controls
typedef enum{
    PMU_BBCVS_2P5,
    PMU_BBCVS_3P0,
    PMU_BBCVS_3P3,
    PMU_BBCVS_NoChange = 0xff
} PMU_BBCVS_t;

typedef enum{
    PMU_BBCRS_P5K,
    PMU_BBCRS_1K,
    PMU_BBCRS_2K,
    PMU_BBCRS_4K,
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


// Charger Watchdog Timer: total charge timer
typedef enum
{
	PMU_WDT_3HOUR = 0,
	PMU_WDT_4HOUR,
	PMU_WDT_5HOUR,
	PMU_WDT_6HOUR,
	PMU_WDT_7HOUR,
	PMU_WDT_8HOUR,
	PMU_WDT_9HOUR,
	PMU_WDT_DISABLE
}PMU_WDT_t;

typedef enum {
	PMU_NO_CHARGER_IS_INUSE = 0,		///< No charger is in use
	PMU_USB_IS_INUSE,					///< USB charger/cable is in use
	PMU_WALL_IS_INUSE					///< WALL charger is in use
} PMU_ChargerInUse_t;

// PMU CSR Vout 
typedef enum
{
	CSRVOUT_0P70	= 0x00,
	CSRVOUT_0P80	= 0x01,
	CSRVOUT_0P86 	= 0x02,
	CSRVOUT_0P88 	= 0x03,
	CSRVOUT_0P90	= 0x04,
	CSRVOUT_0P92 	= 0x05,
	CSRVOUT_0P94	= 0x06,
	CSRVOUT_0P96	= 0x07,
	CSRVOUT_0P98	= 0x08,
	CSRVOUT_1P00 	= 0x09,
	CSRVOUT_1P02 	= 0x0A,
	CSRVOUT_1P04 	= 0x0B,
	CSRVOUT_1P06	= 0x0C,
	CSRVOUT_1P08 	= 0x0D,
	CSRVOUT_1P10 	= 0x0E,
	CSRVOUT_1P12 	= 0x0F,
	CSRVOUT_1P14 	= 0x10,
	CSRVOUT_1P16 	= 0x11,
	CSRVOUT_1P18 	= 0x12,
	CSRVOUT_1P20 	= 0x13,
	CSRVOUT_1P22	= 0x14,
	CSRVOUT_1P24	= 0x15,
	CSRVOUT_1P26	= 0x16,
	CSRVOUT_1P28	= 0x17,
	CSRVOUT_1P30	= 0x18,
	CSRVOUT_1P32	= 0x19,
	CSRVOUT_1P34	= 0x1A
} PMU_CSRVOut_en_t;




#endif

