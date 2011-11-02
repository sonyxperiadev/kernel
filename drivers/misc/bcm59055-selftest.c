/************************************************************************************************/
/*      											*/
/*  Copyright 2011  Broadcom Corporation							*/
/*      											*/
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU	*/
/*     General Public License version 2 (the GPL), available at 				*/
/*      											*/
/*          http://www.broadcom.com/licenses/GPLv2.php  					*/
/*      											*/
/*     with the following added to such license:						*/
/*      											*/
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.      	*/
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.   			*/
/*      											*/
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,	*/
/*     without Broadcom's express prior written consent.					*/
/*      											*/
/************************************************************************************************/

/* Tests supported in this file */
#define GPS_TEST_SUPPORTED      /* GPS GPIO control */  /* Implemented in LMP */
/* #define USB_ST_SUPPORTED */
/* #define ADC_ST_SUPPORTED */
#define SLEEPCLOCK_SUPPORTED	/* Implemented in LMP */ /* Test OK */
#define DIGIMIC_SUPPORTED   	/* Implemented in LMP */ /* Test OK */
#define HEADSET_ST_SUPPORTED	/* Implemented in LMP */ /* Test Fails */
#define IHF_ST_SUPPORTED	/* Implemented in LMP */ /* Test OK */
#define PMU_ST_SUPPORTED	/* Implemented in LMP */ /* Test OK */

/* Use RDB calls used as CHAL calls are not available */
#define USE_AUDIOH_RDB_DMIC
#define USE_AUDIOH_RDB_IHF
#define USE_AUDIOH_RDB_HS

/* Generel includes */
#include <linux/string.h>
#include <stdbool.h>
#include "linux/kernel.h"
#include "linux/delay.h"
#include "linux/init.h"
#include "linux/device.h"
#include "linux/err.h"
#include "linux/fs.h"
#include "linux/proc_fs.h"
#include "linux/types.h"
#include <asm/uaccess.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <mach/hardware.h>

/* Generel Broadcom includes */

/* PMU */
#include "linux/mfd/bcm590xx/core.h"
#include "linux/broadcom/bcm59055-adc.h"
#include "linux/broadcom/bcm59055-audio.h"
#include "linux/broadcom/bcm59055-selftest.h"
/*#include "chipset_iomap.h"*/
/*#define ST_DBG(text,...) pr_debug (text"\n", ## __VA_ARGS__)*/
/*#define ST_MDBG(text,...) pr_debug (text"\n", ## __VA_ARGS__)*/
#define ST_DBG(text, ...) printk(KERN_INFO text"\n", ## __VA_ARGS__)
#define ST_MDBG(text, ...) printk(KERN_INFO text"\n", ## __VA_ARGS__)

#include <mach/io_map.h>

/* RDB access */
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_audioh.h>
#include <mach/rdb/brcm_rdb_khub_clk_mgr_reg.h> /* For DigiMic test */
#include <mach/rdb/brcm_rdb_sclkcal.h>  	/* For Sleep clock test */
#include <mach/rdb/brcm_rdb_bmdm_clk_mgr_reg.h> /* For Sleep clock test */
#include <mach/rdb/brcm_rdb_simi.h>		/* For BARTM test */
#include <mach/rdb/brcm_rdb_slptimer.h>		/* For PMU_CLK32 test test */
#define UNDER_LINUX
#include <mach/rdb/brcm_rdb_util.h>


/* Pinmux */
#include "mach/chip_pinmux.h"
#include "mach/pinmux.h"


/* GPIO */
#include "linux/gpio.h"
#include "linux/interrupt.h"

#ifdef GPS_TEST_SUPPORTED
/* GPS IO test variables */
static bool GPS_PABLANK_Setup_as_GPIO = false;
static struct pin_config StoredValue_GPS_PABLANK;
static bool GPS_TMARK_Setup_as_GPIO = false;
static struct pin_config StoredValue_GPS_TMARK;
#endif

/* PMU Selftest Driver */
static struct bcm590xx *bcm590xx_dev;


/* DIGIMIC*/
#define ST_GPIO_DMIC0DQ  GPIO_DMIC0DQ
#define ST_GPIO_DMIC1DQ  GPIO34
#define ST_GPIO_DMIC0CLK GPIO_DMIC0CLK
#define ST_GPIO_DMIC1CLK GPIO33
#define ST_PN_DMIC1DQ  PN_GPIO34
#define ST_PN_DMIC1CLK PN_GPIO33

#define AUDIO_SETTLING_TIME 15
#define DMIC_SETTLING_TIME 30


/* Audio BB Test enable defines */
#define BB_TEST_DISABLE 	0x00
#define BB_TEST_10_KOHM 	0x01
#define BB_TEST_500_OHM 	0x02
#define BB_TEST_500_OHM_10_KOHM	0x03


/* Sleep Clock Defines */
#define SLEEP_CLOCK_FREQUENCY_SPECIFIED 32768 /*32KHz*/
#define SLEEP_CLOCK_FREQUENCY_MAXIMUM (SLEEP_CLOCK_FREQUENCY_SPECIFIED + 2)
#define SLEEP_CLOCK_FREQUENCY_MINIMUM (SLEEP_CLOCK_FREQUENCY_SPECIFIED - 11)


#ifdef DIGIMIC_SUPPORTED
/* Variable for DigiMic test */
static bool DigiMicInterruptReceived;
#endif

/* Helper Macroes */
#define IHF_NUMBER_OF_SUBTESTS1 2
#define IHF_NUMBER_OF_SUBTESTS2 2
#define HA_NUMBER_OF_SUBTESTS 4
#define CHECKBIT_AND_ASSIGN_ERROR(xAssertLevel, xChecks, \
xReadValue, xResultArray, xErrorCode) \
  { \
  int i; \
  u8 TestValue; \
  TestValue = xReadValue; \
  for (i = 0 ; i < (xChecks) ; i++) { \
	ST_MDBG("GLUE_SELFTEST::CHECKBIT_AND_ASSIGN_ERROR(%u):" \
		" AL:%u,  TV=0x%X, BTV=0x%X",\
	 i, xAssertLevel, TestValue,\
	 ((TestValue) & (1 << i))); \
	if (xResultArray[i] == ST_SELFTEST_OK) {\
	  if ((xAssertLevel == 1))  { \
		ST_MDBG("GLUE_SELFTEST::CHECKBIT_AND_ASSIGN_ERROR:" \
			" High Check (%u)", i); \
		if (((TestValue) & (1 << i)) != 0) { \
		  xResultArray[i]  =  xErrorCode; \
		  ST_MDBG("GLUE_SELFTEST::CHECKBIT_AND_ASSIGN_ERROR:" \
			  " High Assign err = %u", \
		  xErrorCode); \
		} \
	  } \
	  else { \
		ST_MDBG("GLUE_SELFTEST::CHECKBIT_AND_ASSIGN_ERROR:" \
			" Low Check (%u)", i); \
		if (((TestValue) & (1 << i)) == 0) { \
			xResultArray[i] = xErrorCode; \
			ST_MDBG("GLUE_SELFTEST::CHECKBIT_AND_ASSIGN_ERROR:" \
				" Low Assign err = %u", \
				xErrorCode); \
		} \
	  } \
	} \
  } \
  }


/**********************************************************/
/** Stuff that should be defined in header files - Begin **/
/**********************************************************/
/* GPIO defines */
#define GPIO_DMIC0DQ		124
#define GPIO33  		 33
#define GPIO_DMIC0CLK   	123
#define GPIO34  		 34

#define GPIO_GPS_TMARK       97
#define GPIO_GPS_PABLANK     98
/**********************************************************/
/** Stuff that should be defined in header files - End   **/
/**********************************************************/

/* Missing Functions - where to find ?*/
#define assert(x)

/****************************/
/* ST_SLEEP_CLOCK_FREQ_TEST */
/****************************/
#ifdef SLEEPCLOCK_SUPPORTED
static void std_selftest_sleepclk(struct SelftestDevData_t *dev,
			   struct SelftestUserCmdData_t *cmddata)
{
	bool isCalibrationStarted  =  false;
	bool isCalibrationFinished  =  false;
	bool CalibrateAgain  =  true;
	int CalibrateTries = 0;
	int i  =  0;
	u32 calibratedClockFrequency  =  0;
	u32 calibrationCounterFastRegister  =  0;
	u32 calibrationCounterSlowRegister  =  0;
	u32 originalCompareRegisterData  =  0;
	u32 original32kClockRegisterData  =  0;
	u32 originalSCLKCALClockRegisterData  =  0;

	u8 result  =  ST_SELFTEST_FAILED;

	ST_DBG("GLUE_SELFTEST::std_selftest_sleepclk");

	/*Save the original CMP value and set it to 50,*/
	originalCompareRegisterData  =  BRCM_READ_REG(KONA_SCLKCAL_VA,
						    SCLKCAL_CACMP);
	original32kClockRegisterData  =  BRCM_READ_REG(KONA_BMDM_CCU_VA,
						     BMDM_CLK_MGR_REG_WCDMA_32K_CLKGATE);
	originalSCLKCALClockRegisterData  =  BRCM_READ_REG(KONA_BMDM_CCU_VA,
							 BMDM_CLK_MGR_REG_SCLKCAL_CLKGATE);

	ST_DBG("GLUE_SELFTEST::std_selftest_sleepclk() SCLKCAL_CACMP  =  0x%08X",
	       (unsigned int)originalCompareRegisterData);
	ST_DBG("GLUE_SELFTEST::std_selftest_sleepclk() SCLKCAL_CACTRL  =  0x%08X",
	       (unsigned int)BRCM_READ_REG(KONA_SCLKCAL_VA, SCLKCAL_CACTRL));

	ST_DBG("GLUE_SELFTEST::std_selftest_sleepclk() WCDMA_32K_CLKGATE  =  0x%08X",
	       (unsigned int)BRCM_READ_REG(KONA_BMDM_CCU_VA,
					    BMDM_CLK_MGR_REG_WCDMA_32K_CLKGATE));
	ST_DBG("GLUE_SELFTEST::std_selftest_sleepclk() SCLKCAL_CLKGATE    =  0x%08X",
	       (unsigned int)BRCM_READ_REG(KONA_BMDM_CCU_VA,
					    BMDM_CLK_MGR_REG_SCLKCAL_CLKGATE));
	ST_DBG("GLUE_SELFTEST::std_selftest_sleepclk() ACTIVITY_MON1	  =  0x%08X",
	       (unsigned int)BRCM_READ_REG(KONA_BMDM_CCU_VA,
					    BMDM_CLK_MGR_REG_ACTIVITY_MON1));

	/* Enable Clocks */
	BRCM_WRITE_REG_FIELD(KONA_BMDM_CCU_VA,
			     BMDM_CLK_MGR_REG_SCLKCAL_CLKGATE,
			     SCLKCAL_CLK_EN, 1);
	BRCM_WRITE_REG_FIELD(KONA_BMDM_CCU_VA,
			     BMDM_CLK_MGR_REG_WCDMA_32K_CLKGATE,
			     WCDMA_32K_CLK_EN, 1);

	while (CalibrateAgain) {
		CalibrateAgain  =  false;
		CalibrateTries++;

		BRCM_WRITE_REG_FIELD(KONA_SCLKCAL_VA,
				     SCLKCAL_CACMP, CACMP, 50);
		BRCM_WRITE_REG_FIELD(KONA_SCLKCAL_VA,
				     SCLKCAL_CACMP, MODE13MHZ, 0);

		/*Start the calibration*/
		BRCM_WRITE_REG_FIELD(KONA_SCLKCAL_VA, SCLKCAL_CACTRL, CAINIT, 1);

		ST_DBG("GLUE_SELFTEST::std_selftest_sleepclk() WCDMA_32K_CLKGATE  =  0x%08X",
		       (unsigned int)BRCM_READ_REG(KONA_BMDM_CCU_VA,
						    BMDM_CLK_MGR_REG_WCDMA_32K_CLKGATE));
		ST_DBG("GLUE_SELFTEST::std_selftest_sleepclk() SCLKCAL_CLKGATE    =  0x%08X",
		       (unsigned int)BRCM_READ_REG(KONA_BMDM_CCU_VA,
						    BMDM_CLK_MGR_REG_SCLKCAL_CLKGATE));
		ST_DBG("GLUE_SELFTEST::std_selftest_sleepclk() ACTIVITY_MON1	  =  0x%08X",
		       (unsigned int)BRCM_READ_REG(KONA_BMDM_CCU_VA,
						    BMDM_CLK_MGR_REG_ACTIVITY_MON1));

		/*make sure the 9th bit is set to 1, which is the CASTAT field of Calibration Control/Status Register*/
		do {
			if (0 != (BRCM_READ_REG_FIELD(KONA_SCLKCAL_VA, SCLKCAL_CACTRL, CASTAT))) {
				isCalibrationStarted  =  true;
			} else {
				mdelay(2);
				i++;
			}
		} while (!isCalibrationStarted && (i  <  10));

		if (isCalibrationStarted) {
			ST_DBG("GLUE_SELFTEST::std_selftest_sleepclk() isCalibrationStarted");
			mdelay(45);
			i  =  0;

			/*verify if calibration has finished, by checking if the CASTAT field is reset to 0*/
			do {
				if (0 == (BRCM_READ_REG_FIELD(KONA_SCLKCAL_VA, SCLKCAL_CACTRL, CASTAT))) {
					isCalibrationFinished  =  true;
				} else {
					ST_DBG("GLUE_SELFTEST:: Waiting for Calibration#%02u", i);
					mdelay(5);
					i++;
				}
			} while (!isCalibrationFinished && (i  <  10));
		}

		if (isCalibrationFinished) {
			/*Read CAFR and CASR*/
			calibrationCounterFastRegister  =  BRCM_READ_REG(KONA_SCLKCAL_VA, SCLKCAL_CAFR);
			calibrationCounterSlowRegister  =  BRCM_READ_REG(KONA_SCLKCAL_VA, SCLKCAL_CASR);
			ST_DBG("GLUE_SELFTEST::calibrationCounterFastRegister  =  %u, calibrationCounterSlowRegister  =  %u.", calibrationCounterFastRegister, calibrationCounterSlowRegister);
			if ((calibrationCounterFastRegister == 1) && (calibrationCounterSlowRegister == 1) && (CalibrateTries < 10)) {
				ST_DBG("GLUE_SELFTEST:: Calibrate again...");
				CalibrateAgain  =  true;
				continue;
			}

			/*Calculate the frequency and check if it is in the range.*/
			calibratedClockFrequency  =  13000000.0 / 12 * calibrationCounterSlowRegister / calibrationCounterFastRegister;
			ST_DBG("GLUE_SELFTEST::calibratedClockFrequency  =  %u", calibratedClockFrequency);
			if ((calibratedClockFrequency >= SLEEP_CLOCK_FREQUENCY_MINIMUM) && (calibratedClockFrequency <= SLEEP_CLOCK_FREQUENCY_MAXIMUM)) {
				result = ST_SELFTEST_OK;
			}
		} else {
			calibrationCounterFastRegister  =  BRCM_READ_REG(KONA_SCLKCAL_VA, SCLKCAL_CAFR);
			calibrationCounterSlowRegister  =  BRCM_READ_REG(KONA_SCLKCAL_VA, SCLKCAL_CASR);
			ST_DBG("GLUE_SELFTEST:: Calibration failed: calibrationCounterFastRegister  =  %u, calibrationCounterSlowRegister  =  %u.", calibrationCounterFastRegister, calibrationCounterSlowRegister);
			if (CalibrateTries < 10) {
				ST_DBG("GLUE_SELFTEST:: Calibrate again...");
				CalibrateAgain  =  true;
				continue;
			}
		}

	}
	/*restore the original data of the register*/
	BRCM_WRITE_REG(KONA_SCLKCAL_VA, SCLKCAL_CACMP, originalCompareRegisterData);
	BRCM_WRITE_REG(KONA_BMDM_CCU_VA, BMDM_CLK_MGR_REG_WCDMA_32K_CLKGATE, original32kClockRegisterData);
	BRCM_WRITE_REG(KONA_BMDM_CCU_VA, BMDM_CLK_MGR_REG_SCLKCAL_CLKGATE, originalSCLKCALClockRegisterData);

	/* Create failure structure */
	if (ST_SELFTEST_OK == result) {
		cmddata->subtestCount  =  0;
	} else {
		cmddata->subtestCount  =  1;
		cmddata->subtestStatus[0]  =  ST_SELFTEST_FAILED;
	}

	if (ST_SELFTEST_OK == result) {
		ST_DBG("GLUE_SELFTEST::std_selftest_sleepclk() returned -----> ST_SELFTEST_OK");
	} else {
		ST_DBG("GLUE_SELFTEST::std_selftest_sleepclk() returned -----> ST_SELFTEST_FAILED");
	}
	cmddata->testStatus  =  result;
}
#endif

/*******************/
/* ST_DIGIMIC_TEST */
/*******************/
#ifdef DIGIMIC_SUPPORTED

#define  WRITE_REG32(reg, value)	(*((volatile u32 *) (reg))  =  (u32) (value))
/****************************************************************************
*
* NAME:  AUDIOH_hw_clkInit
*
*
*  Description:  Initializes the clock manager settings for APB13
*
*
*  Parameters:  None
*
*  Returns: None
*
*  Notes:   This function is should be part of CLKMGR block. Will be replaced with CLKMGR_hw_XXX
*   			function once available
*
****************************************************************************/
void AUDIOH_hw_clkInit(void)
{
	u32      regVal;

	/* Enable write access */
	regVal  =  (0x00A5A5  <<  KHUB_CLK_MGR_REG_WR_ACCESS_PASSWORD_SHIFT);
	regVal |=  KHUB_CLK_MGR_REG_WR_ACCESS_CLKMGR_ACC_MASK;
	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_WR_ACCESS, regVal);

	/* Set the frequency policy */
	regVal  =  (0x06  <<  KHUB_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_SHIFT);
	regVal |=  (0x06  <<  KHUB_CLK_MGR_REG_POLICY_FREQ_POLICY1_FREQ_SHIFT);
	regVal |=  (0x06  <<  KHUB_CLK_MGR_REG_POLICY_FREQ_POLICY2_FREQ_SHIFT);
	regVal |=  (0x06  <<  KHUB_CLK_MGR_REG_POLICY_FREQ_POLICY3_FREQ_SHIFT);
	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_POLICY_FREQ, regVal);

	/* Set the frequency policy */
	regVal  =  0x7FFFFFFF;
	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_POLICY0_MASK1, regVal);
	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_POLICY1_MASK1, regVal);
	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_POLICY2_MASK1, regVal);
	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_POLICY3_MASK1, regVal);
	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_POLICY0_MASK2, regVal);
	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_POLICY1_MASK2, regVal);
	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_POLICY2_MASK2, regVal);
	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_POLICY3_MASK2, regVal);

	/* start the frequency policy */
	regVal  =  (KHUB_CLK_MGR_REG_POLICY_CTL_GO_MASK | KHUB_CLK_MGR_REG_POLICY_CTL_GO_AC_MASK);
	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_POLICY_CTL, regVal);
}

static void AUDIOH_hw_setClk(u32 enable)
{
	u32      regVal;

	if (enable) {
		/* Enable all the AUDIOH clocks, 26M, 156M, 2p4M, 6p5M  */
		regVal  =  KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_CLK_EN_MASK;
		regVal |=  KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_HW_SW_GATING_SEL_MASK;
		regVal |=  KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_CLK_EN_MASK;
		regVal |=  KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_HW_SW_GATING_SEL_MASK;
		regVal |=  KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_CLK_EN_MASK;
		regVal |=  KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_HW_SW_GATING_SEL_MASK;
		regVal |=  KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_CLK_EN_MASK;
		regVal |=  KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_HW_SW_GATING_SEL_MASK;
		regVal |=  KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_HYST_VAL_MASK;

		BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_AUDIOH_CLKGATE, regVal);
	} else {
		/* Disable all the AUDIOH clocks, 26M, 156M, 2p4M, 6p5M  */
		regVal  =  KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_HW_SW_GATING_SEL_MASK;
		regVal &=  ~KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_CLK_EN_MASK;
		regVal |=  KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_HW_SW_GATING_SEL_MASK;
		regVal &=  ~KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_CLK_EN_MASK;
		regVal |=  KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_HW_SW_GATING_SEL_MASK;
		regVal &=  ~KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_CLK_EN_MASK;
		regVal |=  KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_HW_SW_GATING_SEL_MASK;
		regVal &=  ~KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_CLK_EN_MASK;
		regVal |=  KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_HYST_VAL_MASK;

		BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_AUDIOH_CLKGATE, regVal);
	}
}

/* Clock control functions */
#ifdef USE_AUDIOH_RDB_DMIC
static void ST_AUDIOH_hw_DMIC_Enable(int dmic)
#else
static void ST_AUDIOH_hw_DMIC_Enable(CHAL_HANDLE audiohandle, int dmic)
#endif
{
#ifdef USE_AUDIOH_RDB_DMIC
	volatile u32      regVal;

	regVal  =  BRCM_READ_REG(KONA_AUDIOH_VA, AUDIOH_ADC_CTL);

	ST_DBG("GLUE_SELFTEST::AUDIOH_hw_DMIC_Enable()  Enable interface DMIC%u(RDB)", dmic);
	switch( dmic ) {
	case 0:
		regVal |=  (AUDIOH_ADC_CTL_DMIC1_EN_MASK);
		break;
	case 1:
		regVal |=  (AUDIOH_ADC_CTL_DMIC2_EN_MASK);
		break;
	case 2:
		regVal |=  (AUDIOH_ADC_CTL_DMIC3_EN_MASK);
		break;
	case 3:
		regVal |=  (AUDIOH_ADC_CTL_DMIC4_EN_MASK);
		break;
	}
	BRCM_WRITE_REG(KONA_AUDIOH_VA, AUDIOH_ADC_CTL, regVal);
#else
	/* Enable DMIC paths */
	ST_DBG("GLUE_SELFTEST::AUDIOH_hw_DMIC_Enable()  Enable interface DMIC%u(RDB)", dmic);
	switch( dmic ) {
	case 0:
		chal_audio_vinpath_digi_mic_enable(audiohandle, CHAL_AUDIO_CHANNEL_LEFT);
		break;
	case 1:
		chal_audio_vinpath_digi_mic_enable(audiohandle, CHAL_AUDIO_CHANNEL_RIGHT);
		break;
	case 2:
		chal_audio_nvinpath_digi_mic_enable(audiohandle, CHAL_AUDIO_CHANNEL_LEFT);
		break;
	case 3:
		chal_audio_nvinpath_digi_mic_enable(audiohandle, CHAL_AUDIO_CHANNEL_RIGHT);
		break;
	}
#endif
	/* Setup Clock */
	AUDIOH_hw_clkInit();
	AUDIOH_hw_setClk(1);
}

#ifdef USE_AUDIOH_RDB_DMIC
static void ST_AUDIOH_hw_DMIC_Disable(void)
#else
static void ST_AUDIOH_hw_DMIC_Disable(CHAL_HANDLE audiohandle)
#endif
{
	volatile u32      regVal;

	/* Disable all DMIC paths */
	regVal  =  BRCM_READ_REG(KONA_AUDIOH_VA, AUDIOH_ADC_CTL);

	regVal &= ~(AUDIOH_ADC_CTL_DMIC1_EN_MASK|AUDIOH_ADC_CTL_DMIC2_EN_MASK|AUDIOH_ADC_CTL_DMIC3_EN_MASK|AUDIOH_ADC_CTL_DMIC4_EN_MASK);

	BRCM_WRITE_REG(KONA_AUDIOH_VA, AUDIOH_ADC_CTL, regVal);
}

static irqreturn_t GPIO_DigiMicSelftestEventFunction(int irq, void *dev_id)
{
	/* Receive GPIO event */
	DigiMicInterruptReceived  =  true;

	ST_DBG("GLUE_SELFTEST::GPIO_DigiMicSelftestEventFunction(%u)  ", irq);

	disable_irq_nosync(irq);

	return IRQ_HANDLED;
}

#define DIGIMIC_SUBTESTS_PER_MIC 1
#define MAX_DIGIMIC_IF_COUNT 2
#define MAX_DIGIMIC_COUNT 4
#define DIGIMIC_NUMBER_OF_SUBTESTS 4
#define DIGIMIC_COMM_TEST 0 /* First digimic test */


static void std_selftest_digimic(struct SelftestDevData_t *dev, struct SelftestUserCmdData_t *cmddata)
{
	int Mic, MicIf;
	int ret;
	int i;
	bool TestMic[MAX_DIGIMIC_COUNT];
	/*  int CLK_CONNECTION[MAX_DIGIMIC_IF_COUNT]  =  { ST_GPIO_DMIC0CLK, ST_GPIO_DMIC1CLK };*/
	int DQ_CONNECTION[MAX_DIGIMIC_IF_COUNT]   =  { ST_GPIO_DMIC0DQ, ST_GPIO_DMIC1DQ };
	enum PIN_NAME PMUX_CLK_CONNECTION[MAX_DIGIMIC_IF_COUNT]   =  { PN_DMIC0CLK, ST_PN_DMIC1CLK };
	enum PIN_NAME PMUX_DQ_CONNECTION[MAX_DIGIMIC_IF_COUNT]   =  { PN_DMIC0DQ, ST_PN_DMIC1DQ };
	enum PIN_FUNC PMUX_DMIC_MODE_CLK[MAX_DIGIMIC_IF_COUNT]  =  { PF_DMIC0CLK, PF_DMIC1CLK };
	enum PIN_FUNC PMUX_DMIC_MODE_GPIO[MAX_DIGIMIC_IF_COUNT]  =  { PF_GPIO123, PF_GPIO33 };

/*    enum PIN_FUNC PMUX_DMIC_MODE_DATA[MAX_DIGIMIC_IF_COUNT]  =  { PF_DMIC0DQ, PF_DMIC1DQ };*/
	int MIC_IF[MAX_DIGIMIC_COUNT]  =  { 0, 0, 1, 1 };
	struct pin_config StoredValue[4];
	struct pin_config PIN_DMIC_Setup;
	struct pin_config PIN_GPIO_Setup;
	u8   StoredRegValue8[1];
#ifdef USE_AUDIOH_RDB_DMIC
	u32  StoredRegValue32[1];
#else
	CHAL_HANDLE audiohandle;
	u8 StoredDMIC0Enable;
	u8 StoredDMIC1Enable;
#endif
	u8 Status[DIGIMIC_NUMBER_OF_SUBTESTS] = { ST_SELFTEST_OK, ST_SELFTEST_OK };

#if 1
	AUDIOH_hw_clkInit();
	AUDIOH_hw_setClk(1);
#endif

	/* Pins: DMIC0CLK(GPIO123) Reg: PAD_CTRL - DMIC0CLK*/
	/*  	 DMIC0DQ(GPIO124)  Reg: PAD_CTRL - DMIC0DQ*/
	/* Registers: AUDIOH_ADC_CTL */
	switch(cmddata->parm1) {
	default:
	case ST_SELFTEST_DIGIMIC_NONE:
		TestMic[0] = false;
		TestMic[1] = false;
	case ST_SELFTEST_DIGIMIC_1MIC:
		TestMic[0] = true;
		TestMic[1] = false;
	case ST_SELFTEST_DIGIMIC_2MICS: 
		TestMic[0] = true;
		TestMic[1] = true;
	}
	/* Pins: DMIC1CLK(GPIO33)  Reg: PAD_CTRL - GPIO33 */
	/*  	 DMIC1DQ(GPIO34)   Reg: PAD_CTRL - GPIO34*/
	/* Registers: AUDIOH_ADC_CTL */
	switch(cmddata->parm2) {
	default:
	case ST_SELFTEST_DIGIMIC_NONE:
		TestMic[2] = false;
		TestMic[3] = false;
	case ST_SELFTEST_DIGIMIC_1MIC:
		TestMic[2] = true;
		TestMic[3] = false;
	case ST_SELFTEST_DIGIMIC_2MICS:
		TestMic[2] = true;
		TestMic[3] = true;
	}

	/* Store PMU register Values */
	StoredRegValue8[0]  =  bcm590xx_reg_read(dev->bcm_5900xx_pmu_dev, BCM59055_REG_HV7OPMODCTRL);
	/* Store GPIO setting registers */
	StoredValue[0].name  =  PN_DMIC0CLK;
	pinmux_get_pin_config(&StoredValue[0]);
	StoredValue[0].name  =  PN_DMIC0DQ;
	pinmux_get_pin_config(&StoredValue[1]);
	StoredValue[0].name  =  PN_GPIO33;
	pinmux_get_pin_config(&StoredValue[2]);
	StoredValue[0].name  =  PN_GPIO34;
	pinmux_get_pin_config(&StoredValue[3]);

#ifdef USE_AUDIOH_RDB_DMIC
	/* Store DMIC setting registers */
	StoredRegValue32[0]  =  BRCM_READ_REG(KONA_AUDIOH_VA, AUDIOH_ADC_CTL);
#else
	audiohandle  =  chal_audio_init(KONA_AUDIOH_VA, KONA_SDT_BASE_VA);
	StoredDMIC0Enable  =  chal_audio_vinpath_digi_mic_enable_read(audiohandle);
	StoredDMIC1Enable  =  chal_audio_nvinpath_digi_mic_enable_read(audiohandle);
#endif

	/* Actual test */
	for (Mic = 0 ; Mic < MAX_DIGIMIC_COUNT ; Mic++) {

		if (!TestMic[Mic]) {
			Status[Mic*DIGIMIC_SUBTESTS_PER_MIC]    =  ST_SELFTEST_NOT_TESTED;
			ST_DBG("GLUE_SELFTEST::std_selftest_digimic()  Mic#%u Not Tested", Mic);
			continue;
		}
		MicIf = MIC_IF[Mic];

		ST_DBG("GLUE_SELFTEST::std_selftest_digimic()  Mic#%u", Mic);

		/* Subtest 3(6) - Connection test */
		ST_DBG("GLUE_SELFTEST::std_selftest_digimic()  Connection Test (%u) DQ = %u ", Mic, DQ_CONNECTION[MicIf]);
		/*0. Power on microphone HVLDO7 (on PMU HVLDO7PMODCTRL) */
		bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev, BCM59055_REG_HV7OPMODCTRL, 0x00);	/* Turn on HVLDO7 in all cases b00000000 = 0x00 */
		/*1.	 Setup DMIC0CLK as Mic clock output*/
		PIN_GPIO_Setup.name  =  PMUX_DQ_CONNECTION[MicIf];
		PIN_GPIO_Setup.func  =  PMUX_DMIC_MODE_GPIO[MicIf];
		PIN_GPIO_Setup.reg.val  =  0;
		PIN_GPIO_Setup.reg.b.drv_sth  =  DRIVE_STRENGTH_8MA;
		pinmux_set_pin_config(&PIN_GPIO_Setup);
		ST_DBG("GLUE_SELFTEST:: DQ Setup 0x%X", PIN_GPIO_Setup.reg.val);

		PIN_DMIC_Setup.name  =  PMUX_CLK_CONNECTION[MicIf];
		PIN_DMIC_Setup.func  =  PMUX_DMIC_MODE_CLK[MicIf];
		PIN_DMIC_Setup.reg.val  =  0;
		PIN_DMIC_Setup.reg.b.drv_sth  =  DRIVE_STRENGTH_8MA;
		pinmux_set_pin_config(&PIN_DMIC_Setup);
		ST_DBG("GLUE_SELFTEST:: CLK Setup 0x%X", PIN_DMIC_Setup.reg.val);

		/* Enable Audio Clocks */
#ifdef USE_AUDIOH_RDB_DMIC
		ST_AUDIOH_hw_DMIC_Enable(Mic);
#else
		ST_AUDIOH_hw_DMIC_Enable(audiohandle, Mic);
#endif

		mdelay(DMIC_SETTLING_TIME);

		/*2.	 Setup DMICXDQ  input  */
		/*3.	 Setup DMICXDQ to interrupt on rising edge*/

		ret  =  gpio_request(DQ_CONNECTION[MicIf], "DMICDQ");
		if (ret  <  0) {
			ST_DBG("GLUE_SELFTEST::gpio %u request failed", DQ_CONNECTION[MicIf]);
			cmddata->testStatus  =  ST_SELFTEST_FAILED;
			return;
		}
		DigiMicInterruptReceived  =  false;
		ST_DBG("GLUE_SELFTEST::std_selftest_digimic()  Set interrupt Mode(Input/Rising)");
		gpio_direction_input(DQ_CONNECTION[MicIf]);
		ret  =  request_irq(gpio_to_irq(DQ_CONNECTION[MicIf]),
				  GPIO_DigiMicSelftestEventFunction,
				  IRQF_TRIGGER_RISING,
				  "Digmimic Data",
				  0);
		if (ret  <  0) {
			ST_DBG("GLUE_SELFTEST::gpio irq %u request failed", DQ_CONNECTION[MicIf]);
			cmddata->testStatus  =  ST_SELFTEST_FAILED;
			gpio_free(DQ_CONNECTION[MicIf]);
			return;
		}
		/*4.	 Wait for interrupt for 10ms			  */
		for (i = 0; i < 10 ; i++) {
			ST_DBG("GLUE_SELFTEST::std_selftest_digimic()  Interrrupt wait loop %u, Value = %u", i, gpio_get_value(DQ_CONNECTION[MicIf]));
			mdelay(2);
			if (DigiMicInterruptReceived == true)
				break;
		}
		ST_DBG("GLUE_SELFTEST::std_selftest_digimic()  Disable Interrupt");
		disable_irq(gpio_to_irq(DQ_CONNECTION[MicIf]));
		free_irq(gpio_to_irq(DQ_CONNECTION[MicIf]), 0);
		gpio_free(DQ_CONNECTION[MicIf]);
		if (DigiMicInterruptReceived == true) {
			/* b.    Interrupt received  = > Continue to 5.  */
			ST_DBG("GLUE_SELFTEST::std_selftest_digimic() Interrupt Received");
			Status[(Mic*DIGIMIC_SUBTESTS_PER_MIC)+DIGIMIC_COMM_TEST]    =  ST_SELFTEST_OK;
		} else {
			Status[(Mic*DIGIMIC_SUBTESTS_PER_MIC)+DIGIMIC_COMM_TEST]    =  ST_SELFTEST_BAD_CONNECTION;
			/* a.    Timeout  = > No connection - Test failed*/
		}
		ST_DBG("GLUE_SELFTEST::std_selftest_digimic()  3.1 Status[%u]  =  %u ", (Mic*DIGIMIC_SUBTESTS_PER_MIC)+DIGIMIC_COMM_TEST, Status[(Mic*DIGIMIC_SUBTESTS_PER_MIC)+DIGIMIC_COMM_TEST]);

		if (Status[(Mic*DIGIMIC_SUBTESTS_PER_MIC)+DIGIMIC_COMM_TEST] == ST_SELFTEST_OK) {
			/*5.	 Setup DMICXDQ to interrupt on falling edge*/
			DigiMicInterruptReceived  =  false;
			ret  =  gpio_request(DQ_CONNECTION[MicIf], "DMICDQ");
			if (ret  <  0) {
				ST_DBG("GLUE_SELFTEST::gpio %u request failed", DQ_CONNECTION[MicIf]);
				cmddata->testStatus  =  ST_SELFTEST_FAILED;
				return;
			}
			gpio_direction_input(DQ_CONNECTION[MicIf]);
			DigiMicInterruptReceived  =  false;
			ST_DBG("GLUE_SELFTEST::std_selftest_digimic()  Set interrupt Mode(Input/Falling)");
			ret  =  request_irq(gpio_to_irq(DQ_CONNECTION[MicIf]),
					  GPIO_DigiMicSelftestEventFunction,
					  IRQF_TRIGGER_FALLING,
					  "Digmimic Data",
					  0);
			if (ret  <  0) {
				ST_DBG("GLUE_SELFTEST::gpio irq %u request failed", DQ_CONNECTION[MicIf]);
				cmddata->testStatus  =  ST_SELFTEST_FAILED;
				gpio_free(DQ_CONNECTION[MicIf]);
				return;
			}
			/*6.	 Wait for interrupt for 10ms			  */
			for (i = 0; i < 10 ; i++) {
				ST_DBG("GLUE_SELFTEST::std_selftest_digimic()  Interrrupt wait loop %u, Value = %u", i, gpio_get_value(DQ_CONNECTION[MicIf]));
				mdelay(2);
				if (DigiMicInterruptReceived == true)
					break;
			}
			ST_DBG("GLUE_SELFTEST::std_selftest_digimic()  Disable Interrupt");
			disable_irq(gpio_to_irq(DQ_CONNECTION[MicIf]));
			free_irq(gpio_to_irq(DQ_CONNECTION[MicIf]), 0);
			gpio_free(DQ_CONNECTION[MicIf]);
			if (DigiMicInterruptReceived == true) {
				/* b.    Interrupt received  = > ST_SELFTEST_OK */
				ST_DBG("GLUE_SELFTEST::std_selftest_digimic() Interrupt Received");
				Status[(Mic*DIGIMIC_SUBTESTS_PER_MIC)+DIGIMIC_COMM_TEST]    =  ST_SELFTEST_OK;
			} else {
				/* a.    Timeout  = > No connection - Test failed */
				Status[(Mic*DIGIMIC_SUBTESTS_PER_MIC)+DIGIMIC_COMM_TEST]    =  ST_SELFTEST_BAD_CONNECTION;
			}
			ST_DBG("GLUE_SELFTEST::std_selftest_digimic()  3.2 Status[%u]  =  %u", (Mic*DIGIMIC_SUBTESTS_PER_MIC)+DIGIMIC_COMM_TEST, Status[(Mic*DIGIMIC_SUBTESTS_PER_MIC)+DIGIMIC_COMM_TEST]);
		}
		ST_AUDIOH_hw_DMIC_Disable();
	} /* Mic */

	/* Restore PMU register values */
	bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev, BCM59055_REG_HV7OPMODCTRL, (u16)StoredRegValue8[0]);

#ifdef USE_AUDIOH_RDB_DMIC
	/* Store DMIC setting registers */
	StoredRegValue32[0]  =  BRCM_READ_REG(KONA_AUDIOH_VA, AUDIOH_ADC_CTL);
#else
	chal_audio_vinpath_digi_mic_enable(audiohandle, (u16)StoredDMIC0Enable);
	chal_audio_nvinpath_digi_mic_enable(audiohandle, (u16)StoredDMIC1Enable);
#endif
	/* Restore GPIO setting registers */
	pinmux_set_pin_config(&StoredValue[0]);
	pinmux_set_pin_config(&StoredValue[1]);
	pinmux_set_pin_config(&StoredValue[2]);
	pinmux_set_pin_config(&StoredValue[3]);

	/* Fill out failures status code structure */
	for (i  =  0; i  <  DIGIMIC_NUMBER_OF_SUBTESTS; i++) {
		cmddata->subtestStatus[i] = Status[i];
		ST_DBG("GLUE_SELFTEST::std_selftest_digimic()  Status[%u]  =  %u", i, Status[i]);
	}
	/* Find return code */
	if (((Status[0] != 0) && (Status[0] != ST_SELFTEST_NOT_TESTED)) || ((Status[1] != 0) && (Status[1] != ST_SELFTEST_NOT_TESTED))) {
		cmddata->subtestCount = DIGIMIC_NUMBER_OF_SUBTESTS;
		cmddata->testStatus = ST_SELFTEST_FAILED;
	} else {
		cmddata->subtestCount = 0;
		cmddata->testStatus = ST_SELFTEST_OK;
	}

}
#endif

#if !defined(USE_AUDIOH_RDB_HS) && !defined(USE_AUDIOH_RDB_IHF)
typedef struct {
	u32 AUDIOTX_TEST_EN;
	u32 AUDIOTX_BB_STI;
	u32 AUDIOTX_EP_DRV_STO;

} dac_ctrl_t;

static void st_audio_audiotx_set_dac_ctrl(CHAL_HANDLE audiohandle,
					  dac_ctrl_t *writedata)
{
	u32 ctrl  =  0;

	ctrl |=  (writedata->AUDIOTX_TEST_EN) & 3;
	ctrl |=  ((writedata->AUDIOTX_BB_STI) & 3) << 2;

	chal_audio_audiotx_set_dac_ctrl(audiohandle, ctrl);

}
void st_audio_audiotx_get_dac_ctrl(CHAL_HANDLE audiohandle,
				   dac_ctrl_t *readdata)
{

	u32 ctrl;

	ctrl  =  chal_audio_audiotx_get_dac_ctrl(audiohandle);
	readdata->AUDIOTX_TEST_EN        =  ctrl & 3;
	readdata->AUDIOTX_BB_STI         =  (ctrl >> 2) & 3;
	readdata->AUDIOTX_EP_DRV_STO     =  (ctrl >> 4) & 3;

}
#endif

#ifdef HEADSET_ST_SUPPORTED
static void std_selftest_headset(struct SelftestDevData_t *dev,
			  struct SelftestUserCmdData_t *cmddata)
{
	u8  ResultArray[HA_NUMBER_OF_SUBTESTS]  =  {ST_SELFTEST_OK,
		ST_SELFTEST_OK,
		ST_SELFTEST_OK,
		ST_SELFTEST_OK};
	u8  ReadValue; /* Value read from PMU */
	u8  ReadValue2;	/* Value read from PMU */
	u8  StoredRegValue8[7];
#ifdef USE_AUDIOH_RDB_HS
	u32 StoredRegValue32[1];
#else
	dac_ctrl_t Stored_dac_ctrl_Value;
	dac_ctrl_t Dac_Ctrl;
	CHAL_HANDLE audiohandle;
#endif
	ST_DBG("GLUE_SELFTEST::std_selftest_headset() called.");
#if 1
	AUDIOH_hw_clkInit();
	AUDIOH_hw_setClk(1);
#endif

	/* Store PMU register Values */
	StoredRegValue8[0]  =  bcm590xx_reg_read(dev->bcm_5900xx_pmu_dev,
					       BCM59055_REG_HSPGA1);
	StoredRegValue8[1]  =  bcm590xx_reg_read(dev->bcm_5900xx_pmu_dev,
					       BCM59055_REG_HSPGA2);
	StoredRegValue8[2]  =  bcm590xx_reg_read(dev->bcm_5900xx_pmu_dev,
					       BCM59055_REG_HSPGA3);
	StoredRegValue8[3]  =  bcm590xx_reg_read(dev->bcm_5900xx_pmu_dev,
					       BCM59055_REG_HSIST);
	StoredRegValue8[4]  =  bcm590xx_reg_read(dev->bcm_5900xx_pmu_dev,
					       BCM59055_REG_HSPUP1);
	StoredRegValue8[5]  =  bcm590xx_reg_read(dev->bcm_5900xx_pmu_dev,
					       BCM59055_REG_HSPUP2);
	StoredRegValue8[6]  =  bcm590xx_reg_read(dev->bcm_5900xx_pmu_dev,
					       BCM59055_REG_PLLCTRL);
	/* Store BB register Values */
#ifdef USE_AUDIOH_RDB_HS
	StoredRegValue32[0]  =  BRCM_READ_REG(KONA_AUDIOH_VA, AUDIOH_DAC_CTRL);
#else
	audiohandle  =  chal_audio_init(KONA_AUDIOH_VA, KONA_SDT_BASE_VA);
	st_audio_audiotx_get_dac_ctrl(audiohandle, &Stored_dac_ctrl_Value);
	st_audio_audiotx_get_dac_ctrl(audiohandle, &Dac_Ctrl);
#endif

	/**************************/
	/* Subtest 1 + 2 + 3 + 4  */
	/**************************/
	ReadValue  =  bcm590xx_reg_read(dev->bcm_5900xx_pmu_dev, BCM59055_REG_PMUID);
	ReadValue2  =  bcm590xx_reg_read(dev->bcm_5900xx_pmu_dev, BCM59055_REG_PMUID2);
	ST_DBG("GLUE_SELFTEST::std_selftest_headset()  PMU_ID = 0x%X, PMU_ID2 = 0x%X",
		   ReadValue, ReadValue2);
	ST_DBG("GLUE_SELFTEST::std_selftest_headset()  RA[0] = %u,RA[1] = %u,RA[2] = %u,RA[3] = %u",
		   ResultArray[0], ResultArray[1], ResultArray[2], ResultArray[3]);

	ST_DBG("GLUE_SELFTEST::std_selftest_headset() Dump before setup");
	/* Setup */
	
	bcm59055_hs_set_input_mode( 0, PMU_HS_DIFFERENTIAL_DC_COUPLED );
	bcm59055_hs_set_gain(PMU_AUDIO_HS_BOTH,0);
	bcm59055_audio_init();
	bcm59055_hs_power(true);

	ST_DBG("GLUE_SELFTEST::std_selftest_headset() Dump after setup ");
	/* PMU Input test */
	/**********/
	/* TEST 1 */
	/**********/
	ST_DBG("GLUE_SELFTEST::std_selftest_headset()  INPUT TEST");
	ST_DBG("GLUE_SELFTEST::TEST 1");
	/* 1.   Enable test mode (driving buffer enabled) (i_hs_enst[1:0]  =  '11') on PMU */

	bcm59055_audio_hs_testmode(PMU_TEST_READ_AND_ENABLE);

	/*2.	 Disable Output (i_hs_enst  =  '0') on BB*/ /* AUDIOTX_TEST_EN[1:0]  =  '00' */
#ifdef USE_AUDIOH_RDB_HS
	BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA, AUDIOH_DAC_CTRL,
			      AUDIOTX_TEST_EN, BB_TEST_10_KOHM);
	BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA, AUDIOH_DAC_CTRL,
			      AUDIOTX_BB_STI, 0x00);
#else
	Dac_Ctrl.AUDIOTX_TEST_EN  =  BB_TEST_10_KOHM;
	Dac_Ctrl.AUDIOTX_BB_STI  =  0x00;
	st_audio_audiotx_set_dac_ctrl(audiohandle, &Dac_Ctrl);
#endif
	/*3.	 Set Output (i_hs_ist  =  '1') on PMU    */
	bcm59055_audio_hs_selftest_stimulus(0x01);

	ST_DBG("GLUE_SELFTEST::std_selftest_headset() Dump before test");
	/*4.	 Check result (o_hst_ist[3:0]). */
	ST_DBG("GLUE_SELFTEST::std_selftest_headset() Dump after test ");
	mdelay(AUDIO_SETTLING_TIME);
	bcm59055_audio_hs_selftest_result(&ReadValue);

	/* a.    Bit High  = > Check passed   	*/
	/* b.    Bit Low  = > Shorted to Ground*/
	CHECKBIT_AND_ASSIGN_ERROR(0, HA_NUMBER_OF_SUBTESTS,
				   ReadValue, ResultArray, ST_SELFTEST_SHORTED_GROUND);
	ST_DBG("GLUE_SELFTEST::std_selftest_headset()  Test1 readvalue  =  0x%X -> 0x%X",
	       ReadValue, (ReadValue>>PMU_HSOUT1_OFFSET_O_HS_IST));
	ST_DBG("GLUE_SELFTEST::std_selftest_headset()  RA[0] = %u,RA[1] = %u,RA[2] = %u,RA[3] = %u",
	       ResultArray[0], ResultArray[1], ResultArray[2], ResultArray[3]);

	/**********/
	/* TEST 2 */
	/**********/
	ST_DBG("GLUE_SELFTEST::TEST 2");
#ifdef USE_AUDIOH_RDB_HS
	BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA, AUDIOH_DAC_CTRL, AUDIOTX_BB_STI, 0x03);
#else
	Dac_Ctrl.AUDIOTX_BB_STI  =  0x03;
	st_audio_audiotx_set_dac_ctrl(audiohandle, &Dac_Ctrl);
#endif
	/*5.	 Set Output (i_hs_ist  =  '0') on PMU*/
	bcm59055_audio_hs_selftest_stimulus(0x00);
	/*6.	 Check result (o_hst_ist[3:0]).    */
	mdelay(AUDIO_SETTLING_TIME);
	bcm59055_audio_hs_selftest_result(&ReadValue);
	/* a.    Bit Low  = > Check passed	*/
	/* b.    Bit High  = > Shorted to Power */
	CHECKBIT_AND_ASSIGN_ERROR(1, HA_NUMBER_OF_SUBTESTS,
				  ReadValue,
				  ResultArray, ST_SELFTEST_SHORTED_POWER);
	ST_DBG("GLUE_SELFTEST::std_selftest_headset()  Test2 readvalue  =  0x%X -> 0x%X",
	       ReadValue, (ReadValue>>PMU_HSOUT1_OFFSET_O_HS_IST));
	ST_DBG("GLUE_SELFTEST::std_selftest_headset()  RA[0] = %u,RA[1] = %u,RA[2] = %u,RA[3] = %u",
	       ResultArray[0], ResultArray[1], ResultArray[2], ResultArray[3]);


	/* BB Output Test */
	/**********/
	/* TEST 3 */
	/**********/
	ST_DBG("GLUE_SELFTEST::std_selftest_headset()  OUTPUT TEST");
	ST_DBG("GLUE_SELFTEST::TEST 3");
	/*1.	 Enable test mode (driving buffer disabled) (i_hs_enst[1:0]  =  '10') on PMU */
	bcm59055_audio_hs_testmode(PMU_TEST_READ_AND_DISABLE);
#ifdef USE_AUDIOH_RDB_HS
	/*2.	 Enable Output (i_hs_enst  =  '1') on BB */
	/* AUDIOTX_TEST_EN[1:0]  =  '11' */
	BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA, AUDIOH_DAC_CTRL,
			     AUDIOTX_TEST_EN, BB_TEST_500_OHM);
	/*3.	 Set Output (i_hs_ist  =  '1') on BB */
	/* AUDIOTX_BB_STI[1:0]  =  '11' */
	BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA, AUDIOH_DAC_CTRL,
			     AUDIOTX_BB_STI ,  0x03);
#else
	/*2.	 Enable Output (i_hs_enst  =  '1') on BB */
	/* AUDIOTX_TEST_EN[1:0]  =  '11' */
	Dac_Ctrl.AUDIOTX_TEST_EN  =  BB_TEST_500_OHM;
	/*3.	 Set Output (i_hs_ist  =  '1') on BB */
	/* AUDIOTX_BB_STI[1:0]  =  '11' */
	Dac_Ctrl.AUDIOTX_BB_STI  =  0x03;
	st_audio_audiotx_set_dac_ctrl(audiohandle, &Dac_Ctrl);
#endif
	/*4.	 Check result (o_hst_ist[3:0]).   */
	mdelay(AUDIO_SETTLING_TIME);
	bcm59055_audio_hs_selftest_result(&ReadValue);
	/* a.    Bit High  = > Check passed   				  */
	/* b.    Bit Low  = > Shorted to Ground or Not connected*/
	CHECKBIT_AND_ASSIGN_ERROR(0, HA_NUMBER_OF_SUBTESTS,
				   ReadValue, ResultArray,
				   ST_SELFTEST_BAD_CONNECTION_OR_GROUND);
	ST_DBG("GLUE_SELFTEST::std_selftest_headset()  Test3 readvalue  =  0x%X -> 0x%X",
		   ReadValue, (ReadValue>>PMU_HSOUT1_OFFSET_O_HS_IST));
	ST_DBG("GLUE_SELFTEST::std_selftest_headset()  RA[0] = %u,RA[1] = %u,RA[2] = %u,RA[3] = %u",
		   ResultArray[0], ResultArray[1], ResultArray[2], ResultArray[3]);

	/**********/
	/* TEST 4 */
	/**********/
	ST_DBG("GLUE_SELFTEST::TEST 4");
	/*5.	 Set Output (i_hs_ist  =  '0') on BB */ /* AUDIOTX_BB_STI[1:0]  =  '00' */
#ifdef USE_AUDIOH_RDB_HS
	BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA,
			     AUDIOH_DAC_CTRL, AUDIOTX_BB_STI ,  0x00);
#else
	Dac_Ctrl.AUDIOTX_BB_STI  =  0x00;
	st_audio_audiotx_set_dac_ctrl(audiohandle, &Dac_Ctrl);
#endif
	/*6.	 Check result (o_hst_ist[3:0]).   */
	mdelay(AUDIO_SETTLING_TIME);
	bcm59055_audio_hs_selftest_result(&ReadValue);

	/* a.    Bit Low  = > Check passed						 */
	/* b.    Bit High  = > Shorted to Power or Not connected   */
	CHECKBIT_AND_ASSIGN_ERROR(1, HA_NUMBER_OF_SUBTESTS,
				  ReadValue,
				  ResultArray,
				  ST_SELFTEST_BAD_CONNECTION_OR_POWER);
	ST_DBG("GLUE_SELFTEST::std_selftest_headset()  Test4 readvalue  =  0x%X -> 0x%X",
		   ReadValue, (ReadValue>>PMU_HSOUT1_OFFSET_O_HS_IST));
	ST_DBG("GLUE_SELFTEST::std_selftest_headset()  RA[0] = %u,RA[1] = %u,RA[2] = %u,RA[3] = %u",
		   ResultArray[0], ResultArray[1], ResultArray[2], ResultArray[3]);

	/* Restore PMU register values */
	bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev, BCM59055_REG_HSPGA1,
			   StoredRegValue8[0]);
	bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev, BCM59055_REG_HSPGA2,
			   StoredRegValue8[1]);
	bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev, BCM59055_REG_HSPGA3,
			   StoredRegValue8[2]);
	bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev, BCM59055_REG_HSIST,
			   StoredRegValue8[3]);
	bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev, BCM59055_REG_HSPUP1,
			   StoredRegValue8[4]);
	bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev, BCM59055_REG_HSPUP2,
			   StoredRegValue8[5]);
	bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev, BCM59055_REG_PLLCTRL,
			   StoredRegValue8[6]);
	/* Restore BB register values */

#ifdef USE_AUDIOH_RDB_HS
	BRCM_WRITE_REG(KONA_AUDIOH_VA, AUDIOH_DAC_CTRL, StoredRegValue32[0]);
#else
	st_audio_audiotx_set_dac_ctrl(audiohandle, &Stored_dac_ctrl_Value);
#endif

	/* Fill out failures status code structure */
	{
		int i;
		for (i  =  0; i  <  HA_NUMBER_OF_SUBTESTS; i++) {
			cmddata->subtestStatus[i]  =  ResultArray[HA_NUMBER_OF_SUBTESTS-1-i];
			ST_DBG("GLUE_SELFTEST::std_selftest_headset()  ResultArray[%u]  =  %u", HA_NUMBER_OF_SUBTESTS-1-i, ResultArray[HA_NUMBER_OF_SUBTESTS-1-i]);
		}
	}

	/* Find return code */
	if (ResultArray[0] || ResultArray[1] || ResultArray[2] || ResultArray[3]) {
		cmddata->subtestCount  =  HA_NUMBER_OF_SUBTESTS;
		cmddata->testStatus  =  ST_SELFTEST_FAILED;
	} else {
		cmddata->subtestCount  =  0;
		cmddata->testStatus  =  ST_SELFTEST_OK;
	}
}
#endif

#ifdef IHF_ST_SUPPORTED
static void std_selftest_ihf(struct SelftestDevData_t *dev, struct SelftestUserCmdData_t *cmddata)
{
	u8  ResultArray1[IHF_NUMBER_OF_SUBTESTS1] = {ST_SELFTEST_OK, ST_SELFTEST_OK};
	u8  ResultArray2[IHF_NUMBER_OF_SUBTESTS2] = {ST_SELFTEST_OK, ST_SELFTEST_OK};
	u8  ReadValue; /* Value read from PMU */
	u8  StoredRegValue8[2];
#ifdef USE_AUDIOH_RDB_IHF
	u32 StoredRegValue32[2];
#else
	dac_ctrl_t Stored_dac_ctrl_Value;
	dac_ctrl_t Dac_Ctrl;
	u8 Stored_DacPower;
	CHAL_HANDLE audiohandle;
#endif

	ST_DBG("GLUE_SELFTEST::std_selftest_ihf() called.");
#if 1
	AUDIOH_hw_clkInit();
	AUDIOH_hw_setClk(1);
#endif

	/* Store PMU register Values */
	StoredRegValue8[0]  =  bcm590xx_reg_read(dev->bcm_5900xx_pmu_dev,
					       BCM59055_REG_IHFSTIN);
	StoredRegValue8[1]  =  bcm590xx_reg_read(dev->bcm_5900xx_pmu_dev,
					       BCM59055_REG_IHFSTO);

	/* Store BB register Values */
#ifdef USE_AUDIOH_RDB_IHF
	ST_DBG("GLUE_SELFTEST::std_selftest_ihf() TEST-0");
	StoredRegValue32[0]  =  BRCM_READ_REG(KONA_AUDIOH_VA, AUDIOH_DAC_CTRL);
	ST_DBG("GLUE_SELFTEST::std_selftest_ihf() TEST-1");
	StoredRegValue32[1]  =  BRCM_READ_REG(KONA_AUDIOH_VA, AUDIOH_IHF_PWR);
	ST_DBG("GLUE_SELFTEST::std_selftest_ihf() TEST-2");
#else
	audiohandle  =  chal_audio_init(KONA_AUDIOH_VA, KONA_SDT_BASE_VA);
	st_audio_audiotx_get_dac_ctrl(audiohandle, &Stored_dac_ctrl_Value);
	Stored_DacPower  =  chal_audio_ihfpath_get_dac_pwr(audiohandle);
#endif
	/* Subtest 1 + 2 - IHF DAC */
	/* PMU Input test */
	ST_DBG("GLUE_SELFTEST::std_selftest_ihf() Init tests");
	/* 1.   Enable test mode (driving buffer enabled) (i_hs_enst[1:0]  =  '11') on PMU */
	ST_DBG("GLUE_SELFTEST::std_selftest_ihf() Enable test mode");
	bcm59055_audio_ihf_testmode(PMU_TEST_READ_AND_ENABLE);

	/*2.	 Disable Output (i_hs_enst  =  '0') on BB*/ /* AUDIOTX_TEST_EN[1:0]  =  '00' */
	ST_DBG("GLUE_SELFTEST::std_selftest_ihf() Disable Output");
#ifdef USE_AUDIOH_RDB_IHF
	BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA, AUDIOH_DAC_CTRL,
			     AUDIOTX_TEST_EN, BB_TEST_DISABLE);
#else
	Dac_Ctrl.AUDIOTX_TEST_EN  =  BB_TEST_DISABLE;
	st_audio_audiotx_set_dac_ctrl(audiohandle, &Dac_Ctrl);
#endif
	/*2a. Disable BB output drivers (High-Z) */
	ST_DBG("GLUE_SELFTEST::std_selftest_ihf() Disable BB output drivers (High-Z)");
#ifdef USE_AUDIOH_RDB_IHF
	BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA, AUDIOH_IHF_PWR,
			     AUDIOTX_IHF_DACR_PD, 1);
	BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA, AUDIOH_IHF_PWR,
			     AUDIOTX_IHF_DACL_PD, 1);
#else
	chal_audio_ihfpath_set_dac_pwr(audiohandle, 0);
#endif

	ST_DBG("GLUE_SELFTEST::std_selftest_ihf()  Test 1.1");

	/*3.	 Set Output (i_hs_ist  =  '1') on PMU    */
	bcm59055_audio_ihf_selftest_stimulus_input(0x02);

	/*4.	 Check result (o_hst_ist[3:0]). */
	mdelay(AUDIO_SETTLING_TIME);
	bcm59055_audio_ihf_selftest_result(&ReadValue);

	/* a.    Bit High  = > Check passed   	*/
	/* b.    Bit Low  = > Shorted to Ground*/
	CHECKBIT_AND_ASSIGN_ERROR(0, IHF_NUMBER_OF_SUBTESTS1,
				  ReadValue,
				  ResultArray1, ST_SELFTEST_SHORTED_GROUND);
	ST_DBG("GLUE_SELFTEST::std_selftest_ihf()  Test1 readvalue  =  0x%X",
	       ReadValue);

	ST_DBG("GLUE_SELFTEST::std_selftest_ihf()  Test 1.2");
	/*5.	 Set Output (i_hs_ist  =  '0') on PMU*/
	bcm59055_audio_ihf_selftest_stimulus_input(0x00);

	/*6.	 Check result (o_hst_ist[3:0]).    */
	mdelay(AUDIO_SETTLING_TIME);
	bcm59055_audio_ihf_selftest_result(&ReadValue);

	/* a.    Bit Low  = > Check passed	  */
	/* b.    Bit High  = > Shorted to Power */
	CHECKBIT_AND_ASSIGN_ERROR(1, IHF_NUMBER_OF_SUBTESTS1, ReadValue, ResultArray1, ST_SELFTEST_SHORTED_POWER);
	ST_DBG("GLUE_SELFTEST::std_selftest_ihf()  Test2 readvalue  =  0x%X",
	       ReadValue);

	/* BB output test: */
	/*1.	Enable Output (Test_en[0:1]  =  '11') on BB */
#ifdef USE_AUDIOH_RDB_IHF
	BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA, AUDIOH_DAC_CTRL,
			     AUDIOTX_TEST_EN, BB_TEST_500_OHM);
#else
	Dac_Ctrl.AUDIOTX_TEST_EN  =  BB_TEST_500_OHM;
	st_audio_audiotx_set_dac_ctrl(audiohandle, &Dac_Ctrl);
#endif
	/*2.	 Enable test mode (Short or Open test) (i_IHFselftest_en[1:0]  =  '10') */
	bcm59055_audio_ihf_testmode(PMU_TEST_READ_AND_DISABLE);

	ST_DBG("GLUE_SELFTEST::std_selftest_ihf()  Test 1.3");
	/*3.	 Set Output (i_BB_sti  =  '11')*/
#ifdef USE_AUDIOH_RDB_IHF
	BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA, AUDIOH_DAC_CTRL, AUDIOTX_BB_STI, 0x03);
#else
	Dac_Ctrl.AUDIOTX_BB_STI  =  0x03;
	st_audio_audiotx_set_dac_ctrl(audiohandle, &Dac_Ctrl);
#endif
	/*3a.   Set (i_IHFsti  =  '0x')*/
	bcm59055_audio_ihf_selftest_stimulus_input(0x00);

	/*4.	 Check result (o_IHFsti). */
	mdelay(AUDIO_SETTLING_TIME);
	bcm59055_audio_ihf_selftest_result(&ReadValue);

	/* a.    Bit High  = > Check passed*/
	/* b.    Bit Low  = > Shorted to Ground*/
	CHECKBIT_AND_ASSIGN_ERROR(0, IHF_NUMBER_OF_SUBTESTS1,
				  ReadValue, ResultArray1, ST_SELFTEST_BAD_CONNECTION_OR_GROUND);

	ST_DBG("GLUE_SELFTEST::std_selftest_ihf()  Test 1.4");
	/*5.	Set Output (i_BB_sti  =  '00')*/
#ifdef USE_AUDIOH_RDB_IHF
	BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA, AUDIOH_DAC_CTRL,
			     AUDIOTX_BB_STI, 0x00);
#else
	Dac_Ctrl.AUDIOTX_BB_STI  =  0x00;
	st_audio_audiotx_set_dac_ctrl(audiohandle, &Dac_Ctrl);
#endif
	/*5a.   Set (i_IHFsti  =  '0x')*/
	bcm59055_audio_ihf_selftest_stimulus_input(0x00);

	/*6.	 Check result (o_IHFsti). */
	mdelay(AUDIO_SETTLING_TIME);
	bcm59055_audio_ihf_selftest_result(&ReadValue);

	/* a.    Bit Low  = > Check passed*/
	/* b.    Bit High  = > Shorted to Supply*/
	CHECKBIT_AND_ASSIGN_ERROR(1, IHF_NUMBER_OF_SUBTESTS1, ReadValue, ResultArray1, ST_SELFTEST_BAD_CONNECTION_OR_POWER);

	/* Subtest 3 + 4 - IHF Output  */
	/*PMU Output Pins Test: */
	ST_DBG("GLUE_SELFTEST::std_selftest_ihf()  Test 2.1");
	/*1.	 Enable by setting i_IHFselftest_en[0] to 1. */
	bcm59055_audio_ihf_testmode(PMU_TEST_ENABLE_NO_READ);

	/*2.	 i_IHFsto[1:0]  =  '0x'*/
	bcm59055_audio_ihf_selftest_stimulus_output(0x00);

	/* Check */
	mdelay(AUDIO_SETTLING_TIME);
	bcm59055_audio_ihf_selftest_result(&ReadValue);

	/* a.   Check if  o_IHFsto[1:0] == '00'  = > No pull-up resistor from the output pins to Supply*/
	/* b.   Check if  o_IHFsto[1:0] < >'00'  = > Either or both output pins are short to Supply*/
	CHECKBIT_AND_ASSIGN_ERROR(1, IHF_NUMBER_OF_SUBTESTS2,
				  ReadValue, ResultArray2, ST_SELFTEST_BAD_CONNECTION_OR_POWER);

	ST_DBG("GLUE_SELFTEST::std_selftest_ihf()  Test 2.2");
	/*3.	 i_IHFsto[1:0]  =  '1x'*/
	bcm59055_audio_ihf_selftest_stimulus_output(0x02);

	/* Check */
	mdelay(AUDIO_SETTLING_TIME);
	bcm59055_audio_ihf_selftest_result(&ReadValue);

	/* a.    Check o_IHFsto[1:0] = '00'  = > No pull-down resistor from the output pins to ground*/
	/* b.    Check o_IHFsto[1:0] < >'00'  = > Either or both output pins are short to ground*/
	CHECKBIT_AND_ASSIGN_ERROR(1, IHF_NUMBER_OF_SUBTESTS2,
				  ReadValue, ResultArray2, ST_SELFTEST_BAD_CONNECTION_OR_GROUND);

	/* Restore PMU register values */
	bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev,
			   BCM59055_REG_IHFSTIN, StoredRegValue8[0]);
	bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev,
			   BCM59055_REG_IHFSTO,  StoredRegValue8[1]);
	/* Restore BB register values */
#ifdef USE_AUDIOH_RDB_IHF
	BRCM_WRITE_REG(KONA_AUDIOH_VA, AUDIOH_DAC_CTRL, StoredRegValue32[0]);
	BRCM_WRITE_REG(KONA_AUDIOH_VA, AUDIOH_IHF_PWR, StoredRegValue32[1]);
#else
	st_audio_audiotx_set_dac_ctrl(audiohandle, &Stored_dac_ctrl_Value);
	chal_audio_ihfpath_set_dac_pwr(audiohandle, Stored_DacPower);
#endif

	/* Fill out failures status code structure */
	/* Subtest 1 + 2 - IHF DAC */
	{
		int i;
		for (i  =  0; i  <  IHF_NUMBER_OF_SUBTESTS1; i++) {
			cmddata->subtestStatus[i]  =  ResultArray1[IHF_NUMBER_OF_SUBTESTS1-1-i];
			ST_DBG("GLUE_SELFTEST::std_selftest_ihf()  ResultArray1[%u]  =  %u", IHF_NUMBER_OF_SUBTESTS1-1-i, ResultArray1[IHF_NUMBER_OF_SUBTESTS1-1-i]);
		}
	}
	/* Subtest 3 + 4 - IHF Output  */
	{
		int i;
		for (i  =  0; i  <  IHF_NUMBER_OF_SUBTESTS2; i++) {
			cmddata->subtestStatus[i+IHF_NUMBER_OF_SUBTESTS1]  =  ResultArray2[IHF_NUMBER_OF_SUBTESTS2-1-i];
			ST_DBG("GLUE_SELFTEST::std_selftest_ihf()  ResultArray2[%u]  =  %u", IHF_NUMBER_OF_SUBTESTS2-1-i, ResultArray2[IHF_NUMBER_OF_SUBTESTS2-1-i]);
		}
	}

	/* Find return code */
	if (ResultArray1[0] || ResultArray1[1] ||
	    ResultArray2[0] || ResultArray2[1]) {
		cmddata->subtestCount  =  IHF_NUMBER_OF_SUBTESTS1+IHF_NUMBER_OF_SUBTESTS2;
		cmddata->testStatus  =  ST_SELFTEST_FAILED;
	} else {
		cmddata->subtestCount  =  0;
		cmddata->testStatus  =  ST_SELFTEST_OK;
	}

}
#endif


#ifdef PMU_ST_SUPPORTED
/* Stuff thats should be in PMU driver header file */
	#define PMU_HOSTCTRL3_SELF_TEST 0x20
	#define PMU_HOSTCTRL3_BATRMTEST 0x40

/* Internal structures */
enum PMU_Pin_Index {
	PMU_PIN_SCL_SDA,
	PMU_PIN_PMU_INT,
	PMU_PIN_BATRM_INT,
	PMU_PIN_CLK32,
	PMU_PIN_MAX
};

typedef bool (*Pin_Func_t)(struct SelftestDevData_t *dev);

typedef struct {
	int index;
	bool isCovered;
	bool result;
	Pin_Func_t testRoutine;
} Pin_Status;

/* Forwarded functions */
static bool PMU_Pin_SCL_SDA(struct SelftestDevData_t *dev);	/* Subtest 1 */
static bool PMU_Pin_PMU_INT(struct SelftestDevData_t *dev);	/* Subtest 2 */
static bool PMU_Pin_BATRM_INT(struct SelftestDevData_t *dev);	/* Subtest 3 */
static bool PMU_Pin_CLK32(struct SelftestDevData_t *dev);	/* Subtest 4 */

static bool PMU_Pin_SCL_SDA(struct SelftestDevData_t *dev)
{
	/* Use DBI interrupt mask for testing */
	u32 testingRegisterID  =  BCM59055_REG_INT14MSK;
	u8 originalRegisterData  =  0;
	u8 valueToWrite  =  0xAA;
	u8 valueToRead  =  0;
	bool testResult  =  false;
	ST_DBG("GLUE_SELFTEST::PMU_Pin_SCL_SDA() called.");

	originalRegisterData  =  bcm590xx_reg_read(dev->bcm_5900xx_pmu_dev,
						 testingRegisterID);
	bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev, testingRegisterID,
			   valueToWrite);
	valueToRead  =  bcm590xx_reg_read(dev->bcm_5900xx_pmu_dev,
					testingRegisterID);
	testResult = (valueToWrite == valueToRead) ? true : false;
	bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev, testingRegisterID,
			   originalRegisterData);
	ST_DBG("GLUE_SELFTEST::PMU_Pin_SCL_SDA() returned testResult = %u.", testResult);
	return testResult;

}

/* PMU interrupt test */
static bool PMU_Pin_PMU_INT(struct SelftestDevData_t *dev)
{
	int reading;
	int i = 0;
	do {
		reading  =  bcm59055_saradc_request_rtm (0); /* VBAT */
		/*printk (KERN_INFO "%s: reading %d", reading);*/
		if (reading  <  0) {
			mdelay (20);
		}
	} while ((reading  <  0) && (i++ < 5) && (reading != -EIO));
	if (reading > 0 || reading == -EIO) {
		return true;
	}
	return false;
}


static bool PMU_Pin_BATRM_INT(struct SelftestDevData_t *dev)
{
	bool result  =  false;
	int i  =  0;
	u32 batrm_n  =  0;
	u8 new_register_value  =  0, old_register_value  =  0;
	u8 readback;
	u32 StoredRegValue32[1];

	ST_DBG("GLUE_SELFTEST::PMU_Pin_BATRM_INT() called.");

	old_register_value  =  bcm590xx_reg_read(dev->bcm_5900xx_pmu_dev,
					       BCM59055_REG_HOSTCTRL3);
	new_register_value  =  old_register_value;

	StoredRegValue32[0]  =  BRCM_READ_REG(KONA_SIMI_VA, SIMI_DESDCR);

	/*enable batrm selftest mode*/
	new_register_value |=  PMU_HOSTCTRL3_SELF_TEST;
	bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev, BCM590XX_REG_ENCODE(0x07,
									BCM590XX_SLAVE1_I2C_ADDRESS), 0x38);
	bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev, BCM59055_REG_HOSTCTRL3,
			   new_register_value);
	mdelay(1);
	/*set the output data bit to high*/
	new_register_value |=  PMU_HOSTCTRL3_BATRMTEST;

	bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev, BCM59055_REG_HOSTCTRL3,
			   new_register_value);

	readback  =  bcm590xx_reg_read(dev->bcm_5900xx_pmu_dev,
				     BCM59055_REG_HOSTCTRL3);
	ST_DBG("GLUE_SELFTEST:: High test: BCM59055_REG_HOSTCTRL3  =  0x%X",
	       readback);


	/* SETUP SIM */
	/* Enable ESD clocks */
	BRCM_WRITE_REG_FIELD(KONA_SIMI_VA, SIMI_DESDCR, SIM_ESD_EN, 1);
	/* Disable ESD triggered by BATRM */
	BRCM_WRITE_REG_FIELD(KONA_SIMI_VA, SIMI_DESDCR, SIM_BATRM_ESD_EN, 0);


	/*get the state of the BAT_RM bit, make sure it is high*/
	for (i  =  0; i  <  5; i++) {
		/* Read via SIM interface */
		batrm_n  =  BRCM_READ_REG_FIELD(KONA_SIMI_VA,
					      SIMI_DESDISR,
					      BATRM_N);
		ST_DBG("GLUE_SELFTEST:: High test: batrm_n  =  %u", batrm_n);
		if (0 != batrm_n) {
			break;
		}
		mdelay(5);
	}

	if (0 != batrm_n) {
		/*set the output data bit to low*/
		new_register_value &=  ~PMU_HOSTCTRL3_BATRMTEST;
		bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev,
				   BCM59055_REG_HOSTCTRL3, new_register_value);
		readback  =  bcm590xx_reg_read(dev->bcm_5900xx_pmu_dev,
					     BCM59055_REG_HOSTCTRL3);
		ST_DBG("GLUE_SELFTEST:: Low test: BCM59055_REG_HOSTCTRL3  =  0x%X",
		       readback);
		mdelay(10);
		/* Read via SIM interface */
		batrm_n  =  BRCM_READ_REG_FIELD(KONA_SIMI_VA,
					      SIMI_DESDISR,
					      BATRM_N);

		ST_DBG("GLUE_SELFTEST:: Low test: batrm_n = %u", batrm_n);
		result = (batrm_n == 0) ? true : false;
	}

	BRCM_WRITE_REG(KONA_SIMI_VA, SIMI_DESDCR, StoredRegValue32[0]);
	bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev, BCM590XX_REG_ENCODE(0x07,
									BCM590XX_SLAVE1_I2C_ADDRESS), 0x38);
	bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev,
			   BCM59055_REG_HOSTCTRL3,
			   old_register_value);

	ST_DBG("GLUE_SELFTEST::PMU_Pin_BATRM_INT() returned %u.", result);
	return result;
}

static bool PMU_Pin_CLK32(struct SelftestDevData_t *dev)
{
	u32 oldValue  =  0, newValue  =  0;
	int i  =  0;
	ST_DBG("GLUE_SELFTEST::PMU_Pin_CLK32() called.");
	/*get the current down counter value, only the first 9 bits are used.*/
	oldValue = BRCM_READ_REG(KONA_SLPTIMER_VA, SLPTIMER_SMTSTR);
	for (i  =  0; i  <  5; i++) {
		mdelay(1);
		/*get the new current down counter value, check if changed*/
		newValue  = BRCM_READ_REG(KONA_SLPTIMER_VA, SLPTIMER_SMTSTR);
		ST_DBG("GLUE_SELFTEST::PMU_Pin_CLK32::old_value  =  %u, new_value  =  %u.", oldValue, newValue);
		if (oldValue != newValue) {
			ST_DBG("GLUE_SELFTEST::PMU_Pin_CLK32() returned true.");
			return true;
		}
	}

	ST_DBG("GLUE_SELFTEST::PMU_Pin_CLK32() returned false.");
	return false;
}


static void std_selftest_pmu(struct SelftestDevData_t *dev, struct SelftestUserCmdData_t *cmddata)
{
	Pin_Status Selftest_PMU_Pin_Coverage[PMU_PIN_MAX] =	{
		{ PMU_PIN_SCL_SDA,    true,  false,   PMU_Pin_SCL_SDA },
		{ PMU_PIN_PMU_INT,    true,  false,   PMU_Pin_PMU_INT},
		{ PMU_PIN_BATRM_INT,  true,  false,   PMU_Pin_BATRM_INT},
		{ PMU_PIN_CLK32,      true,  false,   PMU_Pin_CLK32}
	};

	int i  =  0;
	bool overallResult  =  true;

	ST_DBG("GLUE_SELFTEST::hal_selftest_pmu() called.");

	for (i  =  0; i  <  PMU_PIN_MAX; i++) {
		if (Selftest_PMU_Pin_Coverage[i].isCovered == true) {
			if ((*(Selftest_PMU_Pin_Coverage[i].testRoutine))(dev) == true) {
				Selftest_PMU_Pin_Coverage[i].result  =  true;
			} else {
				ST_DBG("GLUE_SELFTEST::hal_selftest_pmu()::failed test case index  =  %d", i);
				if (overallResult == true) {
					overallResult  =  false;
				}
			}
		}
	}

	cmddata->subtestCount  =  0;
	if (overallResult != true) {
		cmddata->subtestCount  =  PMU_PIN_MAX;
		for (i  =  0; i  <  PMU_PIN_MAX; i++) {
			if ((Selftest_PMU_Pin_Coverage[i].isCovered == true)) {
				cmddata->subtestStatus[i]  =  (Selftest_PMU_Pin_Coverage[i].result == true ? ST_SELFTEST_OK : ST_SELFTEST_FAILED);
			}
		}
	}

	if (!overallResult) {
		ST_DBG("GLUE_SELFTEST::hal_selftest_pmu() returned ----------> ST_SELFTEST_FAILED");
		cmddata->testStatus  =  ST_SELFTEST_FAILED;
		return;
	}

	ST_DBG("GLUE_SELFTEST::hal_selftest_pmu() returned ----------> ST_SELFTEST_OK");
	cmddata->testStatus  =  ST_SELFTEST_OK;
}
#else
static void std_selftest_pmu(struct SelftestDevData_t *dev, struct SelftestUserCmdData_t *cmddata)
{
	ST_DBG("GLUE_SELFTEST::std_selftest_pmu() called.");

	/*...........*/
	/*....TBD....*/
	/*...........*/

}
#endif


#ifdef GPS_TEST_SUPPORTED
/*---------------------------------------------------------------------------*/
/*! \brief ST_SELFTEST_control_gps_io.
 *  Self Test for gps control pins
 *
 *  \param name 	   - The logical name of the GenIO pin
 *  \param direction       - Defines if read or write operation shall be performed
 *  \param state	   - Pointer to data type that contains the pin state
 *
 *  \return     	   - The outcome of the self test procedure
 */
static void std_selftest_control_gps_io(struct SelftestDevData_t *dev,
				 struct SelftestUserCmdData_t *cmddata)
{
	u8 result  = ST_SELFTEST_FAILED; /* set to default */
	struct pin_config  GPIOSetup;
	int ret;
	/* Input */
	u8 name      = cmddata->parm1;
	u8 direction = cmddata->parm2;
	u8 *state    = (u8 *)&cmddata->parm3;

	ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () called -- ST_SELFTEST_GPS_READ/WRITE %x %x %x", name, direction, *state);

	GPIOSetup.reg.val       = 0;
	GPIOSetup.reg.b.drv_sth = DRIVE_STRENGTH_2MA;

	switch (name) {
	case ST_SELFTEST_GPS_TXP:
		if (GPS_PABLANK_Setup_as_GPIO == false) {
			ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () GPS_PABLANK_Setup_as_GPIO");
			StoredValue_GPS_PABLANK.name = PN_GPS_PABLANK;
			pinmux_get_pin_config(&StoredValue_GPS_PABLANK);
			GPIOSetup.name = PN_GPS_PABLANK;
			GPIOSetup.func = PF_GPIO98;
			pinmux_set_pin_config(&GPIOSetup);
			ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () 0x%08X",
				   StoredValue_GPS_PABLANK.reg.val);
			ret = gpio_request(GPIO_GPS_PABLANK, "GPS PABLANK");
			if (ret < 0) {
				ST_DBG("GLUE_SELFTEST::gpio %u request failed",
				       GPIO_GPS_PABLANK);
/*				  dev_err(&pdev->dev, "Unable to request GPIO pin %d\n", GPIO_GPS_PABLANK);*/
				result = ST_SELFTEST_FAILED;
			}
			GPS_PABLANK_Setup_as_GPIO = true;
		}
		switch (direction) {
		case ST_SELFTEST_GPS_WRITE:{
			switch (*state) {
			case ST_SELFTEST_GPS_HIGH:
				 gpio_direction_output(GPIO_GPS_PABLANK, 1);
				 gpio_set_value(GPIO_GPS_PABLANK, 1);
				 ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () GPS_PABLANK - Write - High");
				 break;
			case ST_SELFTEST_GPS_LOW:
				 gpio_direction_output(GPIO_GPS_PABLANK, 0);
				 gpio_set_value(GPIO_GPS_PABLANK, 0);
				 ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () GPS_PABLANK - Write - Low");
				 break;
			case ST_SELFTEST_GPS_RELEASE:
			default:
			/* assume release state, defined in ISI */
			if (GPS_PABLANK_Setup_as_GPIO == true) {
				ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () GPS_PABLANK - Restore");
				pinmux_set_pin_config(&StoredValue_GPS_PABLANK);
				gpio_free(GPIO_GPS_PABLANK);
				GPS_PABLANK_Setup_as_GPIO = false;
			}
			break;
			}
			result = ST_SELFTEST_OK;
			break;
			}
		case ST_SELFTEST_GPS_READ:
			 gpio_direction_input(GPIO_GPS_PABLANK);
			 ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () GPS_PABLANK - Read");
			 if (gpio_get_value(GPIO_GPS_PABLANK) == 1) {
			     ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () >>>High");
			     *state = ST_SELFTEST_GPS_HIGH;
			 } else {
			     ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () >>>Low");
			     *state = ST_SELFTEST_GPS_LOW;
			 }
			 result = ST_SELFTEST_OK;
			 break;
		default:
			assert(false);
			break;
		}
		break;
	case ST_SELFTEST_GPS_TIMESTAMP:
		if (GPS_TMARK_Setup_as_GPIO == false) {
			ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () GPS_TMARK_Setup_as_GPIO");
			StoredValue_GPS_TMARK.name = PN_GPS_TMARK;
			pinmux_get_pin_config(&StoredValue_GPS_TMARK);
			GPIOSetup.name = PN_GPS_TMARK;
			GPIOSetup.func = PF_GPIO97;
			pinmux_set_pin_config(&GPIOSetup);
			ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () 0x%08X",
				   StoredValue_GPS_TMARK.reg.val);
			ret = gpio_request(GPIO_GPS_TMARK, "GPS TMARK");
			if (ret < 0) {
				ST_DBG("GLUE_SELFTEST::gpio %u request failed",
				       GPIO_GPS_PABLANK);
/*			  dev_err(&pdev->dev, "Unable to request GPIO pin %d\n", GPIO_GPS_PABLANK);*/
				result = ST_SELFTEST_FAILED;
			}
			GPS_TMARK_Setup_as_GPIO = true;
		}
		switch (direction) {
		case ST_SELFTEST_GPS_WRITE:{
			switch (*state) {
			case ST_SELFTEST_GPS_HIGH:
				 gpio_direction_output(GPIO_GPS_TMARK, 1);
				 gpio_set_value(GPIO_GPS_TMARK, 1);
				 ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () GPS_TMARK - Write - High");
				 break;
			case ST_SELFTEST_GPS_LOW:
				 gpio_direction_output(GPIO_GPS_TMARK, 0);
				 gpio_set_value(GPIO_GPS_TMARK, 0);
				 ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () GPS_TMARK - Write - Low");
				 break;
			case ST_SELFTEST_GPS_RELEASE:
			default:
				 /* assume release state, defined in ISI */
				 if (GPS_TMARK_Setup_as_GPIO == true) {
					ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () GPS_TMARK - Restore");
					pinmux_set_pin_config(&StoredValue_GPS_TMARK);
					gpio_free(GPIO_GPS_TMARK);
					GPS_TMARK_Setup_as_GPIO = false;
				 }
				 break;
			 }
			 result = ST_SELFTEST_OK;
			 break;
		}

		case ST_SELFTEST_GPS_READ:
			 ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () GPS_TMARK - Read");
			 gpio_direction_input(GPIO_GPS_TMARK);
			if (gpio_get_value(GPIO_GPS_TMARK) == 1) {
				 ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () >>>High");
				*state = ST_SELFTEST_GPS_HIGH;
			} else {
				ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () >>>Low");
				*state = ST_SELFTEST_GPS_LOW;
			}
			result = ST_SELFTEST_OK;
			break;
		default:
			assert(false);
			break;
		}
		break;
	default:
		result = ST_SELFTEST_NOT_SUPPORTED;
		break;
	}

	if (ST_SELFTEST_OK == result) {
		ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () returned -----> ST_SELFTEST_OK");
	} else {
		ST_DBG("GLUE_SELFTEST::std_selftest_control_gps_io () returned -----> ST_SELFTEST_FAILED");
	}

	cmddata->testStatus = result;
}
#endif

#ifdef USB_ST_SUPPORTED
static u8 hal_selftest_usb_charger_latch_ok = 0;
static u8 hal_selftest_usb_charger_called = 0;

static void std_selftest_usb_charger(struct SelftestDevData_t *dev,
			      struct SelftestUserCmdData_t *cmddata)
{
	u8 mbc5_orig, mbc5_bcd_aon, i;
	u8 status;

	if (!hal_selftest_usb_charger_called) {
#ifdef NEED_TO_SIGNUP_FOR_EVENT_IN_LMP
		status = HAL_EM_PMU_RegisterEventCB (PMU_DRV_CHGDET_LATCH, &hal_selftest_usb_charger_latch_cb);
#endif
		ST_DBG("GLUE_SELFTEST::hal_selftest_usb_charger() Status: %d", status);
		hal_selftest_usb_charger_called = 1;
	}

	hal_selftest_usb_charger_latch_ok = 0;

	/* Set PMU into BCDLDO Always on in MBCCTRL5 [1] */
	mbc5_orig = bcm590xx_reg_read(dev->bcm_5900xx_pmu_dev, BCM59055_REG_MBCCTRL5);

	if (mbc5_orig & BCM59055_REG_MBCCTRL5_USB_DET_LDO_EN) {
		/* LDO is already on. */
		mbc5_bcd_aon = mbc5_orig & ~(BCM59055_REG_MBCCTRL5_USB_DET_LDO_EN);
		ST_DBG("GLUE_SELFTEST::hal_selftest_usb_charger(); USB_DET_LDO is already on, orig = %x aon = %x", mbc5_orig, mbc5_bcd_aon);
		bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev, BCM59055_REG_MBCCTRL5, mbc5_bcd_aon);
		mdelay(5);
	}

	mbc5_bcd_aon = mbc5_orig | BCM59055_REG_MBCCTRL5_BCDLDO_AON | BCM59055_REG_MBCCTRL5_USB_DET_LDO_EN | BCM59055_REG_MBCCTRL5_BC11_EN;
	bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev, BCM59055_REG_MBCCTRL5, mbc5_bcd_aon);

	ST_DBG("GLUE_SELFTEST::hal_selftest_usb_charger() MBCCTRL5 original %x, aon %x", mbc5_orig, mbc5_bcd_aon);

	for (i = 0; i < 200; i++) {
		/* wait for CHGDET_LATCH status */
		mdelay(2);
		if (hal_selftest_usb_charger_latch_ok)
			break;
	}
	/* Restore MBCCTRL5 */
	mdelay(25);

	bcm590xx_reg_write(dev->bcm_5900xx_pmu_dev, BCM59055_REG_MBCCTRL5, mbc5_orig);
	/* IF chp_typ == 0x01, test is working correctly. chp_typ is located in bits 4 and 5.*/

	if (hal_selftest_usb_charger_latch_ok) {
		ST_DBG("GLUE_SELFTEST::hal_selftest_usb_charger() Connection is good");
			cmddata->subtestCount = 0;
			cmddata->subtestStatus[i] = ST_SELFTEST_OK;
	} else {
		cmddata->subtestCount = 1;
		cmddata->subtestStatus[i] = ST_SELFTEST_FAILED;
	}
}
#endif

#ifdef ADC_ST_SUPPORTED
static void std_selftest_adc(struct SelftestDevData_t *dev,
		      struct SelftestUserCmdData_t *cmddata)
{
    ST_DBG("GLUE_SELFTEST::std_selftest_adc () called.");

    /*...........*/
    /*....TBD....*/
    /*...........*/

}
#endif


/********************************************************************/
/* Command Handling */
/********************************************************************/

static void SelftestHandleCommand(enum SelftestUSCmds_e cmd,
				  struct SelftestDevData_t *dev,
				  struct SelftestUserCmdData_t *cmddata)
{
	ST_DBG("SelftestHandleCommand::Cmd:%u", cmddata->testId);
	/* Handle Userspace Commands */
	switch (cmd) {
	#ifdef GPS_TEST_SUPPORTED
	case ST_SUSC_GPS_TEST:
		std_selftest_control_gps_io(dev, cmddata);
		break;
	#endif
	#ifdef USB_ST_SUPPORTED
	case ST_SUSC_USB_ST:
		std_selftest_usb_charger(dev, cmddata);
		break;
	#endif
	#ifdef ADC_ST_SUPPORTED
	case ST_SUSC_ADC_ST:
		std_selftest_adc(dev, cmddata);
		break;
	#endif
#ifdef DIGIMIC_SUPPORTED
	case ST_SUSC_DIGIMIC:
		ST_DBG("bcm_selftest_bb.c::SelftestHandleCommand::ST_SUSC_DIGIMIC");
		std_selftest_digimic(dev, cmddata);
		break;
#endif
#ifdef SLEEPCLOCK_SUPPORTED
	case ST_SUSC_SLEEPCLOCK:
		std_selftest_sleepclk(dev, cmddata);
		break;
#endif
#ifdef HEADSET_ST_SUPPORTED
	case ST_SUSC_HEADSET_ST:
		std_selftest_headset(dev, cmddata);
		break;
#endif
#ifdef IHF_ST_SUPPORTED
	case ST_SUSC_IHF_ST:
		std_selftest_ihf(dev, cmddata);
		break;
#endif
#ifdef PMU_ST_SUPPORTED
	case ST_SUSC_PMU_ST:
		std_selftest_pmu(dev, cmddata);
		break;
#endif
	default:
		cmddata->testStatus  =  ST_SELFTEST_NOT_SUPPORTED;
		ST_DBG("Command not supported");
		break;
	}
}

/********************************************************************/
/* Selftest Sysfs file interface */
/********************************************************************/
static SelftestUserCmdData_t cmddata;
static ssize_t show_TestResult (struct device_driver *d, char * buf)
{
	sprintf(buf, "%i %i %i %i %i %i %i %i %i %i %i %i %i",
		cmddata.testId, 
		cmddata.testStatus,
		cmddata.subtestCount,
		cmddata.subtestStatus[0], cmddata.subtestStatus[1],
		cmddata.subtestStatus[2], cmddata.subtestStatus[3],
		cmddata.subtestStatus[4], cmddata.subtestStatus[5],
		cmddata.subtestStatus[6], cmddata.subtestStatus[7],
		cmddata.subtestStatus[8], cmddata.subtestStatus[9] );
	return strlen(buf); 
}

static ssize_t show_TestStart (struct device_driver *d, char * buf)
{
	sprintf(buf,"%i %i %i %i)", cmddata.testId, 
			cmddata.parm1, cmddata.parm2, cmddata.parm3);
	return  strlen(buf);; 
}
static ssize_t store_TestStart (struct device_driver *d, const char * buf, size_t count)
{
	int TestId, Parm1, Parm2, Parm3;
	struct SelftestDevData_t dev;

	ST_DBG("store_TestStart - Message received");

	dev.bcm_5900xx_pmu_dev = bcm590xx_dev;

	if ( (sscanf(buf, "%i %i %i %i", &TestId, &Parm1, &Parm2, &Parm3 )) != 4)
		return -EINVAL;

	cmddata.testId = TestId;
	cmddata.parm1  = Parm1;
	cmddata.parm2  = Parm2;
	cmddata.parm3  = Parm3;

	ST_DBG("store_TestStart (%i, %i, %i, %i)", TestId, Parm1, Parm2, Parm3 );
	SelftestHandleCommand((enum SelftestUSCmds_e)cmddata.testId, &dev, &cmddata);
	
	return strnlen(buf, count);
}

static DRIVER_ATTR(TestResult, S_IRUGO, show_TestResult, NULL);
static DRIVER_ATTR(TestStart, S_IRUGO | S_IWUSR, show_TestStart, store_TestStart);


#define DRIVER_NAME "bcm59055-selftest"


/********************************************************************/
/* Selftest Procfs file interface */
/********************************************************************/
/***********************PMU PROC DEBUG Interface*********************/
static int selftest_open(struct inode *inode, struct file *file)
{
	pr_debug("%s\n", __func__);
	file->private_data = PDE(inode)->data;

	return 0;
}

int selftest_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}
static long selftest_unlocked_ioctl(struct file *filp, unsigned int cmd,
				     unsigned long arg)
{
	struct SelftestUserCmdData_t cmddata;
	struct SelftestDevData_t dev;
	ST_DBG("Selftest command Received");
	ST_DBG("Selftest copy_from_user");
	dev.bcm_5900xx_pmu_dev = bcm590xx_dev;
	if (copy_from_user((void *)&cmddata, (void*)arg,
		sizeof(SelftestUserCmdData_t))) {
	ST_DBG("Selftest copy_from_user Failed");
	return -EFAULT;
	}

	cmddata.testId = cmd;
	SelftestHandleCommand((enum SelftestUSCmds_e)cmd, &dev, &cmddata);

	ST_DBG("Selftest copy_to_user");
	if (copy_to_user ((void *)arg, (void*)&cmddata, sizeof(SelftestUserCmdData_t))) {
		ST_DBG("Selftest copy_to_user Failed");
		return -EFAULT;
	}
	return 0;
}

#define MAX_USER_INPUT_LEN 10
static ssize_t selftest_write(struct file *file, const char __user *buffer,
	size_t len, loff_t *offset)
{
	struct SelftestUserCmdData_t cmddata;
	struct SelftestDevData_t dev;

	if (len > MAX_USER_INPUT_LEN)
		len = MAX_USER_INPUT_LEN;

	dev.bcm_5900xx_pmu_dev = bcm590xx_dev;
	cmddata.testId = buffer[0]-'a';
	switch (cmddata.testId) {
	case ST_SUSC_DIGIMIC:
		cmddata.parm1 = 1; /* Enable DMIC 1 Test */
		cmddata.parm2 = 1; /* Enable DMIC 2 Test */
		break;
	case ST_SUSC_GPS_TEST:
		cmddata.parm1 = 0;
		cmddata.parm2 = 0;
		cmddata.parm3 = 0;
		break;
	default:
		break;
	}
	SelftestHandleCommand(cmddata.testId, &dev, &cmddata);
	if (cmddata.testStatus == 0) {
		ST_DBG("Selftest OK");
	} else {
		ST_DBG("Selftest Failed (%u)", cmddata.testStatus);
	}


	*offset += len;
	return len;
}

static const struct file_operations selftest_ops = {
	.open = selftest_open,
	.unlocked_ioctl = selftest_unlocked_ioctl,
	.write = selftest_write,
	.release = selftest_release,
	.owner = THIS_MODULE,
};

/********************************************************************/
/* Selftest Drivers Stuff */
/********************************************************************/

/******************************************************************************
*
* Function Name: selftest_pmu_probe
*
* Desc: Called to perform module initialization when the module is loaded.
*
******************************************************************************/
static int __devinit selftest_pmu_probe(struct platform_device *pdev)
{
	bcm590xx_dev = dev_get_drvdata(pdev->dev.parent);

	ST_DBG("Selftest CSAPI driver probed");

	/* Register proc interface */
	proc_create_data("selftest", S_IRWXUGO, NULL,
		   &selftest_ops, NULL/*private data*/);

	return 0;
}

static int __devexit selftest_pmu_remove(struct platform_device *pdev)
{
	return 0;
}

struct platform_driver selftestpmu_driver = {
	.probe = selftest_pmu_probe,
	.remove = __devexit_p(selftest_pmu_remove),
	.driver = {
		   .name = DRIVER_NAME,
		   }
};

#if 0
struct file_operations selftest_fops = {
	.owner 		= THIS_MODULE,
	.write 		= selftest_write,
	.unlocked_ioctl = selftest_unlocked_ioctl,
	.open 		= selftest_open,
	.release 	= selftest_release,
};

struct miscdevice selftestpmu_miscdevice = {
	.minor =    MISC_DYNAMIC_MINOR,
	.name =     DRIVER_NAME,
	.fops =     &selftest_fops
};
#endif


/****************************************************************************
*
*  selftest_pmu_init
*
*     Called to perform module initialization when the module is loaded
*
***************************************************************************/

static int __init selftest_pmu_init(void)
{
	/* Register driver */
	platform_driver_register(&selftestpmu_driver);
#if 0
	misc_register(&selftestpmu_miscdevice);
#endif
	/* Register sysfs interface */
	driver_create_file(&selftestpmu_driver.driver, &driver_attr_TestStart);
	driver_create_file(&selftestpmu_driver.driver, &driver_attr_TestResult);

	return 0;
}

/****************************************************************************
*
*  selftest_pmu_exit
*
*       Called to perform module cleanup when the module is unloaded.
*
***************************************************************************/

static void __exit selftest_pmu_exit(void)
{
	platform_driver_unregister(&selftestpmu_driver);

	driver_remove_file(&selftestpmu_driver.driver, &driver_attr_TestStart);
	driver_remove_file(&selftestpmu_driver.driver, &driver_attr_TestResult);

}


module_init(selftest_pmu_init);
module_exit(selftest_pmu_exit);

MODULE_AUTHOR("PHERAGER");
MODULE_DESCRIPTION("BCM SELFTEST BB");
