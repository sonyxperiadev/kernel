/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license
* *other than the GPL, without Broadcom's express prior written consent.
* *****************************************************************************/

/* Tests supported in this file */
/*#define GPS_TEST_SUPPORTED*/
#define USB_ST_SUPPORTED
#define ADC_ST_SUPPORTED
#define SIM_SHORT_ST_SUPPORTED
#define SLEEPCLOCK_SUPPORTED
#define I2C_ST_SUPPORTED
#define BATRM_ST_SUPPORTED
#define IRQ_ST_SUPPORTED

/* Generel includes */
#include <linux/string.h>
#include <stdbool.h>
#include "linux/kernel.h"
#include "linux/delay.h"
#include "linux/init.h"
#include "linux/device.h"
#include "linux/err.h"
#include "linux/fs.h"
#include "linux/types.h"
#include "linux/miscdevice.h"
#include "linux/gpio.h"
#include "linux/interrupt.h"
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/module.h>

#include <mach/hardware.h>
#include <mach/io_map.h>
#include <mach/chip_pinmux.h>
#include <mach/pinmux.h>

/* PMU */
#include "linux/mfd/bcmpmu.h"

/* RDB access */
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_sclkcal.h>	/* For Sleep clock test */
#include <mach/rdb/brcm_rdb_bmdm_clk_mgr_reg.h>	/* For Sleep clock test */
#include <mach/rdb/brcm_rdb_simi.h>	/* For BARTM test */
#include <mach/rdb/brcm_rdb_slptimer.h>	/* For PMU_CLK32 test test */
#define UNDER_LINUX
#include <mach/rdb/brcm_rdb_util.h>

#define NO_DEBUG
#ifdef NO_DEBUG
#define ST_DBG(text, ...) pr_debug(text"\n", ## __VA_ARGS__)
#define ST_MDBG(text, ...) pr_debug(text"\n", ## __VA_ARGS__)
#else
#define ST_DBG(text, ...) printk(KERN_INFO text"\n", ## __VA_ARGS__)
#define ST_MDBG(text, ...) printk(KERN_INFO text"\n", ## __VA_ARGS__)
#endif

enum Selftest_Results_t {
	ST_PASS,		/* Test succeded */
	ST_FAIL,		/* Test Failed */
	ST_BUSY,
	ST_NOT_TESTED,
/* SIM */
	ST_SIM_SHORT_GROUND,
	ST_SIM_SHORT_POWER,
	ST_SIM_SHORT_INTERCONNECT
};

static const char *const Selftest_ResultString[] = {
	"PASS",			/* Test succeded */
	"FAIL",			/* Test Failed */
	"EBUSY",
	"NOT_TESTED",
/* SIM */
	"SIM_SHORT_GROUND",
	"SIM_SHORT_POWER",
	"SIM_SHORT_INTERCONNECT"
};

enum selftest_store_e {
	SELFTEST_USB,
	SELFTEST_ADC,
	SELFTEST_SC,
	SELFTEST_I2C,
	SELFTEST_BATRM,
	SELFTEST_IRQ,
	SELFTEST_SIM,
	SELFTEST_COUNT
};

/* GPS selftest defines */
enum Selftest_GPS_Pin_e {
	ST_GPS_TXP,
	ST_GPS_TIMESTAMP
};

enum Selftest_GPS_Pin_Action_e {
	ST_GPS_READ,
	ST_GPS_WRITE
};

enum Selftest_GPS_Pin_State_e {
	ST_GPS_HIGH,
	ST_GPS_LOW,
	ST_GPS_RELEASE
};

/********************************************************/
/** Usermode space communication **/
/********************************************************/
enum ResultFormat_e {
	ST_RESULT_FORMAT_NORMAL = 0,
	ST_RESULT_FORMAT_DETAILED
};

struct SelftestDataStandard_t {
	enum ResultFormat_e rf;
};

struct SelftestDataSim_t {
	enum ResultFormat_e rf;
	int slot;
};

/* Selftest User Space Commands */
struct SelftestData_t {
	struct SelftestDataStandard_t adc;
	struct SelftestDataStandard_t usb;
	struct SelftestDataStandard_t sc;
	struct SelftestDataStandard_t i2c;
	struct SelftestDataStandard_t batrm;
	struct SelftestDataStandard_t irq;
	struct SelftestDataSim_t sim;
};

static struct SelftestData_t SelftestData = {
	.adc = {
		.rf = ST_RESULT_FORMAT_DETAILED,
		},
	.usb = {
		.rf = ST_RESULT_FORMAT_DETAILED,
		},
	.sc = {
	       .rf = ST_RESULT_FORMAT_DETAILED,
	       },
	.i2c = {
		.rf = ST_RESULT_FORMAT_DETAILED,
		},
	.batrm = {
		  .rf = ST_RESULT_FORMAT_DETAILED,
		  },
	.irq = {
		.rf = ST_RESULT_FORMAT_DETAILED,
		},
	.sim = {
		.rf = ST_RESULT_FORMAT_DETAILED,
		}
};

struct SelftestUserCmdData_t {
	/* Input */
	uint testId;		/* SelftestCmds_e */
	enum ResultFormat_e ResultFormat;
	uint parm1;
	uint parm2;
	uint parm3;
	/* Output */
	uint subtestCount;
	uint subtestStatus[10];	/* Max 10 subtests supported */
	uint testStatus;
};

#ifdef GPS_TEST_SUPPORTED
/* GPS IO test variables */
static bool GPS_PABLANK_Setup_as_GPIO;	/*  = false */
static struct pin_config StoredValue_GPS_PABLANK;
static bool GPS_TMARK_Setup_as_GPIO;	/* = false */
static struct pin_config StoredValue_GPS_TMARK;
#endif

/* Sleep Clock Defines */
#define SLEEP_CLOCK_FREQUENCY_SPECIFIED 32768	/* 32KHz */
#define SLEEP_CLOCK_FREQUENCY_MAXIMUM (SLEEP_CLOCK_FREQUENCY_SPECIFIED + 2)
#define SLEEP_CLOCK_FREQUENCY_MINIMUM (SLEEP_CLOCK_FREQUENCY_SPECIFIED - 11)

/**********************************************************/
/** Internal data structure **/
/**********************************************************/
struct bcmpmu_selftest {
	struct bcmpmu *bcmpmu;
	struct platform_device *pdev;
};

static struct bcmpmu_selftest *bcmpmu_selftest;

static bool TestActive[SELFTEST_COUNT] = { false, false, false, false,
	false, false, false
};

/**********************************************************/
/** Stuff that should be defined in header files - Begin **/
/**********************************************************/
#ifdef SIM_SHORT_ST_SUPPORTED
/* SIM TEST defines */
#define ST_SIM_GPIO_CNT 8	/* Number of SIM interface pins */
#define SIM1_FIRST 0
#define SIM1_LAST  3
#define SIM2_FIRST 4
#define SIM2_LAST  7
#define MAX_SIM_INDEX (ST_SIM_GPIO_CNT-1)
#define SETTLING_TIME 5000
/* SIM: GPIO Defines */
#define ST_SIMRST       53
#define ST_SIMDAT       54
#define ST_SIMCLK       55
#define ST_SIMDET       56
#define ST_SIM2RST      85
#define ST_SIM2DAT      86
#define ST_SIM2CLK      87
#define ST_SIM2DET      88
#define ST_SIM2LDO_EN   99
#define ST_SIM2VDD2_SEL 95
/* SIM: PAD CTRL defines */

#define ST_SIMRST_PAD	PF_GPIO53
#define ST_SIMDAT_PAD	PF_GPIO54
#define ST_SIMCLK_PAD	PF_GPIO55
#define ST_SIMDET_PAD	PF_GPIO56
#define ST_SIM2RST_PAD	PF_GPIO85
#define ST_SIM2DAT_PAD	PF_GPIO86
#define ST_SIM2CLK_PAD	PF_GPIO87
#define ST_SIM2DET_PAD	PF_GPIO88

#define ST_SIMRST_PAD_NAME	PN_SIMRST
#define ST_SIMDAT_PAD_NAME	PN_SIMDAT
#define ST_SIMCLK_PAD_NAME	PN_SIMCLK
#define ST_SIMDET_PAD_NAME	PN_SIMDET
#define ST_SIM2RST_PAD_NAME	PN_SSPSYN
#define ST_SIM2DAT_PAD_NAME	PN_SSPDO
#define ST_SIM2CLK_PAD_NAME	PN_SSPCK
#define ST_SIM2DET_PAD_NAME	PN_SSPDI

#define SIMLDO_REGULATOR "sim_vcc"
#define SIM2LDO_REGULATOR "sim2_vcc"
#define ST_PAD_DRV_STRENGTH  1

#endif

/**********************************************************/
/** Stuff that should be defined in header files - End   **/
/**********************************************************/

/* Missing Functions - where to find ?*/
#define assert(x)

/****************************/
/* ST_SLEEP_CLOCK_FREQ_TEST */
/****************************/
#ifdef SLEEPCLOCK_SUPPORTED
static void std_selftest_sleepclk(struct SelftestUserCmdData_t *cmddata)
{
	bool isCalibrationStarted = false;
	bool isCalibrationFinished = false;
	bool CalibrateAgain = true;
	int CalibrateTries = 0;
	int i = 0;
	u32 calibratedClockFrequency = 0;
	u32 calibrationCounterFastRegister = 0;
	u32 calibrationCounterSlowRegister = 0;
	u32 originalCompareRegisterData = 0;
	u32 original32kClockRegisterData = 0;
	u32 originalSCLKCALClockRegisterData = 0;

	u8 result = ST_FAIL;

	ST_DBG("GLUE_SELFTEST::std_selftest_sleepclk");

	/* Save the original CMP value and set it to 50, */
	originalCompareRegisterData = BRCM_READ_REG(KONA_SCLKCAL_VA,
						    SCLKCAL_CACMP);
	original32kClockRegisterData =
	    BRCM_READ_REG(KONA_BMDM_CCU_VA, BMDM_CLK_MGR_REG_WCDMA_32K_CLKGATE);
	originalSCLKCALClockRegisterData =
	    BRCM_READ_REG(KONA_BMDM_CCU_VA, BMDM_CLK_MGR_REG_SCLKCAL_CLKGATE);

	ST_DBG
	    ("GLUE_SELFTEST::std_selftest_sleepclk() SCLKCAL_CACMP  =  0x%08X",
	     (unsigned int)originalCompareRegisterData);
	ST_DBG
	    ("GLUE_SELFTEST::std_selftest_sleepclk() SCLKCAL_CACTRL  =  0x%08X",
	     (unsigned int)BRCM_READ_REG(KONA_SCLKCAL_VA, SCLKCAL_CACTRL));

	ST_DBG
	    ("GLUE_SELFTEST::std_selftest_sleepclk() "
	     "WCDMA_32K_CLKGATE  =  0x%08X",
	     (unsigned int)BRCM_READ_REG(KONA_BMDM_CCU_VA,
					 BMDM_CLK_MGR_REG_WCDMA_32K_CLKGATE));
	ST_DBG
	    ("GLUE_SELFTEST::std_selftest_sleepclk() "
	     "SCLKCAL_CLKGATE    =  0x%08X",
	     (unsigned int)BRCM_READ_REG(KONA_BMDM_CCU_VA,
					 BMDM_CLK_MGR_REG_SCLKCAL_CLKGATE));
	ST_DBG
	    ("GLUE_SELFTEST::std_selftest_sleepclk() "
	     "ACTIVITY_MON1	  =  0x%08X",
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
		CalibrateAgain = false;
		CalibrateTries++;

		BRCM_WRITE_REG_FIELD(KONA_SCLKCAL_VA, SCLKCAL_CACMP, CACMP, 50);
		BRCM_WRITE_REG_FIELD(KONA_SCLKCAL_VA,
				     SCLKCAL_CACMP, MODE13MHZ, 0);

		/* Start the calibration */
		udelay(250); /* JIRA HWRHEA-1243 */
		BRCM_WRITE_REG_FIELD(KONA_SCLKCAL_VA, SCLKCAL_CACTRL, CAINIT,
				     1);

		ST_DBG
		    ("GLUE_SELFTEST::std_selftest_sleepclk() "
		     "WCDMA_32K_CLKGATE  =  0x%08X",
		     (unsigned int)BRCM_READ_REG(KONA_BMDM_CCU_VA,
					 BMDM_CLK_MGR_REG_WCDMA_32K_CLKGATE));
		ST_DBG
		    ("GLUE_SELFTEST::std_selftest_sleepclk() "
		     "SCLKCAL_CLKGATE    =  0x%08X",
		     (unsigned int)BRCM_READ_REG(KONA_BMDM_CCU_VA,
					 BMDM_CLK_MGR_REG_SCLKCAL_CLKGATE));
		ST_DBG
		    ("GLUE_SELFTEST::std_selftest_sleepclk() "
		     "ACTIVITY_MON1	  =  0x%08X",
		     (unsigned int)BRCM_READ_REG(KONA_BMDM_CCU_VA,
					 BMDM_CLK_MGR_REG_ACTIVITY_MON1));

		/* make sure the 9th bit is set to 1, which is the CASTAT
		   field of Calibration Control/Status Register */
		do {
			if (0 !=
			    (BRCM_READ_REG_FIELD
			     (KONA_SCLKCAL_VA, SCLKCAL_CACTRL, CASTAT))) {
				isCalibrationStarted = true;
			} else {
				usleep_range(2000, 2500);
				i++;
			}
		} while (!isCalibrationStarted && (i < 10));

		if (isCalibrationStarted) {
			ST_DBG("GLUE_SELFTEST::std_selftest_sleepclk()"
			       " isCalibrationStarted");
			msleep(45);
			i = 0;

			/*verify if calibration has finished, by checking
			   if the CASTAT field is reset to 0 */
			do {
				if (0 ==
				    (BRCM_READ_REG_FIELD
				     (KONA_SCLKCAL_VA, SCLKCAL_CACTRL,
				      CASTAT))) {
					isCalibrationFinished = true;
				} else {
					ST_DBG
					    ("GLUE_SELFTEST:: "
					     "Waiting for Calibration#%02u", i);
					usleep_range(5000, 5500);
					i++;
				}
			} while (!isCalibrationFinished && (i < 10));
		}

		if (isCalibrationFinished) {
			/* Read CAFR and CASR */
			calibrationCounterFastRegister =
			    BRCM_READ_REG(KONA_SCLKCAL_VA, SCLKCAL_CAFR);
			calibrationCounterSlowRegister =
			    BRCM_READ_REG(KONA_SCLKCAL_VA, SCLKCAL_CASR);
			ST_DBG
			    ("GLUE_SELFTEST::calibrationCounterFastRegister  ="
			     "  %u, calibrationCounterSlowRegister  =  %u.",
			     calibrationCounterFastRegister,
			     calibrationCounterSlowRegister);
			if ((calibrationCounterFastRegister == 1)
			    && (calibrationCounterSlowRegister == 1)
			    && (CalibrateTries < 10)) {
				ST_DBG("GLUE_SELFTEST:: Calibrate again...");
				CalibrateAgain = true;
				continue;
			}

			/* Calculate the frequency and check if it is in the
			   range. */
			calibratedClockFrequency =
			    ((13000000 / 12) * calibrationCounterSlowRegister) /
			  calibrationCounterFastRegister;
			ST_DBG("GLUE_SELFTEST::calibratedClockFrequency  =  %u",
			       calibratedClockFrequency);
			if ((calibratedClockFrequency >=
			     SLEEP_CLOCK_FREQUENCY_MINIMUM)
			    && (calibratedClockFrequency <=
				SLEEP_CLOCK_FREQUENCY_MAXIMUM)) {
				result = ST_PASS;
			}
		} else {
			calibrationCounterFastRegister =
			    BRCM_READ_REG(KONA_SCLKCAL_VA, SCLKCAL_CAFR);
			calibrationCounterSlowRegister =
			    BRCM_READ_REG(KONA_SCLKCAL_VA, SCLKCAL_CASR);
			ST_DBG
			    ("GLUE_SELFTEST:: Calibration failed: "
			     "calibrationCounterFastRegister  =  %u, "
			     "calibrationCounterSlowRegister  =  %u.",
			     calibrationCounterFastRegister,
			     calibrationCounterSlowRegister);
			if (calibrationCounterSlowRegister &&
			   calibrationCounterFastRegister) {
				/* Calculate the frequency and check
				   if it is in the range. */
				calibratedClockFrequency =
				    ((13000000 / 12) *
				     calibrationCounterSlowRegister) /
				     calibrationCounterFastRegister;
				ST_DBG("GLUE_SELFTEST::calClockFreq  =  %u",
				       calibratedClockFrequency);
			}
			if (CalibrateTries < 10) {
				ST_DBG("GLUE_SELFTEST:: Calibrate again...");
				CalibrateAgain = true;
				continue;
			}
		}

	}
	/*restore the original data of the register */
	BRCM_WRITE_REG(KONA_SCLKCAL_VA, SCLKCAL_CACMP,
		       originalCompareRegisterData);
	BRCM_WRITE_REG(KONA_BMDM_CCU_VA, BMDM_CLK_MGR_REG_WCDMA_32K_CLKGATE,
		       original32kClockRegisterData);
	BRCM_WRITE_REG(KONA_BMDM_CCU_VA, BMDM_CLK_MGR_REG_SCLKCAL_CLKGATE,
		       originalSCLKCALClockRegisterData);

	/* Create failure structure */
	if (ST_PASS == result) {
		cmddata->subtestCount = 0;
	} else {
		cmddata->subtestCount = 1;
		cmddata->subtestStatus[0] = ST_FAIL;
	}

	if (ST_PASS == result) {
		ST_DBG
		    ("GLUE_SELFTEST::std_selftest_sleepclk() returned "
		     "-----> ST_PASS");
	} else {
		ST_DBG
		    ("GLUE_SELFTEST::std_selftest_sleepclk() returned "
		     "-----> ST_FAILED");
	}
	cmddata->testStatus = result;
}
#endif

#ifdef BATRM_ST_SUPPORTED
/* Stuff thats should be in PMU driver header file */
#define PMU_HOSTCTRL3_SELF_TEST 0x20
#define PMU_HOSTCTRL3_BATRMTEST 0x40

static void std_selftest_batrm(struct SelftestUserCmdData_t *cmddata)
{
	bool result = false;
	int i = 0;
	u32 batrm_n = 0;
	unsigned int new_register_value = 0, old_register_value = 0;
	unsigned int readback;
	unsigned int StoredRegValue32[1];

	ST_DBG("GLUE_SELFTEST::std_selftest_batrm() called.");

	bcmpmu_selftest->bcmpmu->read_dev(bcmpmu_selftest->bcmpmu,
					  PMU_REG_HOSTCTRL3,
					  &old_register_value, PMU_BITMASK_ALL);

	new_register_value = old_register_value;

	StoredRegValue32[0] = BRCM_READ_REG(KONA_SIMI_VA, SIMI_DESDCR);

	/*enable batrm selftest mode */
	new_register_value |= PMU_HOSTCTRL3_SELF_TEST;
	bcmpmu_selftest->bcmpmu->write_dev_drct(bcmpmu_selftest->bcmpmu, 0,
						0x07, 0x38, PMU_BITMASK_ALL);
	bcmpmu_selftest->bcmpmu->write_dev(bcmpmu_selftest->bcmpmu,
					   PMU_REG_HOSTCTRL3,
					   new_register_value,
					   bcmpmu_selftest->bcmpmu->
					   regmap[PMU_REG_HOSTCTRL3].mask);

	usleep_range(1000, 1500);
	/*set the output data bit to high */
	new_register_value |= PMU_HOSTCTRL3_BATRMTEST;
	bcmpmu_selftest->bcmpmu->write_dev(bcmpmu_selftest->bcmpmu,
					   PMU_REG_HOSTCTRL3,
					   new_register_value,
					   bcmpmu_selftest->bcmpmu->
					   regmap[PMU_REG_HOSTCTRL3].mask);
	bcmpmu_selftest->bcmpmu->read_dev(bcmpmu_selftest->bcmpmu,
					  PMU_REG_HOSTCTRL3, &readback,
					  PMU_BITMASK_ALL);

	ST_DBG("GLUE_SELFTEST:: High test: PMU_REG_HOSTCTRL3  =  0x%X",
	       readback);

	/* SETUP SIM */
	/* Enable ESD clocks */
	BRCM_WRITE_REG_FIELD(KONA_SIMI_VA, SIMI_DESDCR, SIM_ESD_EN, 1);
	/* Disable ESD triggered by BATRM */
	BRCM_WRITE_REG_FIELD(KONA_SIMI_VA, SIMI_DESDCR, SIM_BATRM_ESD_EN, 0);

	/*get the state of the BAT_RM bit, make sure it is high */
	for (i = 0; i < 5; i++) {
		/* Read via SIM interface */
		batrm_n = BRCM_READ_REG_FIELD(KONA_SIMI_VA,
					      SIMI_DESDISR, BATRM_N);
		ST_DBG("GLUE_SELFTEST:: High test: batrm_n  =  %u", batrm_n);
		if (0 != batrm_n)
			break;
		usleep_range(5000, 5500);
	}

	if (0 != batrm_n) {
		/*set the output data bit to low */
		new_register_value &= ~PMU_HOSTCTRL3_BATRMTEST;
		bcmpmu_selftest->bcmpmu->write_dev(bcmpmu_selftest->bcmpmu,
						   PMU_REG_HOSTCTRL3,
						   new_register_value,
						   bcmpmu_selftest->bcmpmu->
						   regmap[PMU_REG_HOSTCTRL3].
						   mask);
		bcmpmu_selftest->bcmpmu->read_dev(bcmpmu_selftest->bcmpmu,
						  PMU_REG_HOSTCTRL3, &readback,
						  PMU_BITMASK_ALL);

		ST_DBG("GLUE_SELFTEST:: Low test: PMU_REG_HOSTCTRL3  =  0x%X",
		       readback);
		usleep_range(10000, 10500);
		/* Read via SIM interface */
		batrm_n = BRCM_READ_REG_FIELD(KONA_SIMI_VA,
					      SIMI_DESDISR, BATRM_N);

		ST_DBG("GLUE_SELFTEST:: Low test: batrm_n = %u", batrm_n);
		result = (batrm_n == 0) ? true : false;
	}

	BRCM_WRITE_REG(KONA_SIMI_VA, SIMI_DESDCR, StoredRegValue32[0]);
	bcmpmu_selftest->bcmpmu->write_dev_drct(bcmpmu_selftest->bcmpmu, 0,
						0x07, 0x38, PMU_BITMASK_ALL);
	bcmpmu_selftest->bcmpmu->write_dev(bcmpmu_selftest->bcmpmu,
					   PMU_REG_HOSTCTRL3,
					   old_register_value,
					   bcmpmu_selftest->bcmpmu->
					   regmap[PMU_REG_HOSTCTRL3].mask);

	ST_DBG("GLUE_SELFTEST::std_selftest_batrm() returned %u.", result);

	if (result) {
		ST_DBG
		    ("GLUE_SELFTEST::std_selftest_batrm() Connection is good");
		cmddata->testStatus = ST_PASS;
		cmddata->subtestCount = 0;
	} else {
		ST_DBG("GLUE_SELFTEST::std_selftest_batrm() Connection is bad");
		cmddata->testStatus = ST_FAIL;
		cmddata->subtestCount = 1;
		cmddata->subtestStatus[0] = ST_FAIL;
	}
}
#endif

#ifdef IRQ_ST_SUPPORTED
static void std_selftest_irq(struct SelftestUserCmdData_t *cmddata)
{
	int result = false;
	int reading;
	int i = 0;
	struct bcmpmu_adc_req ReqData;
	do {
		ReqData.sig = PMU_ADC_VMBATT;
		ReqData.tm = PMU_ADC_TM_RTM_SW;
		ReqData.flags = PMU_ADC_RAW_ONLY;
		bcmpmu_selftest->bcmpmu->adc_req(bcmpmu_selftest->bcmpmu,
						 &ReqData);
		reading = ReqData.raw;
		if (reading < 0)
			msleep(20);

	} while ((reading < 0) && (i++ < 5) && (reading != -EIO));
	if (reading > 0 || reading == -EIO)
		result = true;

	if (result) {
		ST_DBG("GLUE_SELFTEST::std_selftest_irq() Connection is good");
		cmddata->testStatus = ST_PASS;
		cmddata->subtestCount = 0;
	} else {
		ST_DBG("GLUE_SELFTEST::std_selftest_irq() Connection is bad");
		cmddata->testStatus = ST_FAIL;
		cmddata->subtestCount = 1;
		cmddata->subtestStatus[0] = ST_FAIL;
	}
}
#endif

#ifdef I2C_ST_SUPPORTED
static void std_selftest_i2c(struct SelftestUserCmdData_t *cmddata)
{
	/* Use DBI interrupt mask for testing */
	unsigned int originalRegisterData = 0;
	unsigned int valueToWrite = 0xAA;
	unsigned int valueToRead = 0;
	bool testResult = false;
	int rc;
	ST_DBG("GLUE_SELFTEST::std_selftest_i2c() called.");

	bcmpmu_selftest->bcmpmu->read_dev(bcmpmu_selftest->bcmpmu,
					  PMU_REG_INT14, &originalRegisterData,
					  PMU_BITMASK_ALL);

	rc = bcmpmu_selftest->bcmpmu->write_dev(bcmpmu_selftest->bcmpmu,
						PMU_REG_INT14, valueToWrite,
						bcmpmu_selftest->bcmpmu->
						regmap[PMU_REG_INT14].mask);
	if (rc >= 0) {
		ST_DBG("GLUE_SELFTEST::std_selftest_i2c() Value Written");
		rc = bcmpmu_selftest->bcmpmu->read_dev(bcmpmu_selftest->bcmpmu,
						       PMU_REG_INT14,
						       &valueToRead,
						       PMU_BITMASK_ALL);
		if (rc >= 0) {
			ST_DBG
			    ("GLUE_SELFTEST::std_selftest_i2c() "
			     "Value Read Back");
			testResult =
			    (valueToWrite == valueToRead) ? true : false;
		}
	}
	bcmpmu_selftest->bcmpmu->write_dev(bcmpmu_selftest->bcmpmu,
					   PMU_REG_INT14, originalRegisterData,
					   bcmpmu_selftest->bcmpmu->
					   regmap[PMU_REG_INT14].mask);

	ST_DBG("GLUE_SELFTEST::std_selftest_i2c() returned testResult = %u.",
	       testResult);

	if (testResult) {
		ST_DBG("GLUE_SELFTEST::std_selftest_i2c() Connection is good");
		cmddata->testStatus = ST_PASS;
		cmddata->subtestCount = 0;
	} else {
		ST_DBG("GLUE_SELFTEST::std_selftest_i2c() Connection is bad");
		cmddata->testStatus = ST_FAIL;
		cmddata->subtestCount = 1;
		cmddata->subtestStatus[0] = ST_FAIL;
	}
}
#endif

#ifdef SIM_SHORT_ST_SUPPORTED
#define ST_SIM1 0
#define ST_SIM2 1
static bool PowerOnSIM(int slot)
{
	struct regulator *regl_sim = NULL;
	switch (slot) {
	case ST_SIM1:
		{
			ST_DBG("GLUE_SELFTEST::PowerOnSIM(%u) SIM1#1", slot);
			regl_sim = regulator_get(NULL, SIMLDO_REGULATOR);
			if (IS_ERR(regl_sim)) {
				printk(KERN_INFO
				       "%s: regulator_get failed(0x%X)\n",
				       __func__, (unsigned int)regl_sim);
				ST_DBG
				    ("GLUE_SELFTEST::PowerOnSIM(%u) "
				     "SIM1 Power failed", slot);
				return false;
			} else {
				/* Enable SIM1 power via PMU driver / PMU HAL */
				regulator_enable(regl_sim);
				ST_DBG
				    ("GLUE_SELFTEST::PowerOnSIM(%u) "
				     "SIM1 supported", slot);
				usleep_range(10000, 10500);

				regulator_put(regl_sim);
				return true;
			}

		}
	case ST_SIM2:
		ST_DBG("GLUE_SELFTEST::PowerOnSIM(%u) SIM2#1", slot);
		regl_sim = regulator_get(NULL, SIM2LDO_REGULATOR);
		if (IS_ERR(regl_sim)) {
			printk(KERN_INFO "%s: regulator_get failed(0x%X)\n",
			       __func__, (unsigned int)regl_sim);
			ST_DBG
			    ("GLUE_SELFTEST::PowerOnSIM(%u) SIM2 Power failed",
			     slot);
			return false;
		} else {
			/* Enable SIM1 power via PMU driver / PMU HAL  */
			regulator_enable(regl_sim);
			ST_DBG("GLUE_SELFTEST::PowerOnSIM(%u) SIM2 supported",
			       slot);
			usleep_range(10000, 10500);

			regulator_put(regl_sim);
			return true;
		}
		return true;
	default:
		break;

	}
	ST_DBG("GLUE_SELFTEST::PowerOnSIM(%u) SIM not tested", slot);
	return false;
}

static void std_selftest_sim(struct SelftestUserCmdData_t *cmddata)
{
	struct pin_config StoredSIMPMuxGpioValue[ST_SIM_GPIO_CNT];
	int StoredPowerValue[2] = { 0, 0 };
	struct pin_config GPIOSetup;
	bool TestSIM1, TestSIM2;
	static int ST_GPIOList[ST_SIM_GPIO_CNT] = {
		ST_SIMCLK, ST_SIMDAT, ST_SIMDET, ST_SIMRST,
		ST_SIM2CLK, ST_SIM2DAT, ST_SIM2DET, ST_SIM2RST
	};
	static enum PIN_NAME ST_PMUXListn[ST_SIM_GPIO_CNT] = {
		ST_SIMCLK_PAD_NAME, ST_SIMDAT_PAD_NAME, ST_SIMDET_PAD_NAME,
		ST_SIMRST_PAD_NAME,
		ST_SIM2CLK_PAD_NAME, ST_SIM2DAT_PAD_NAME, ST_SIM2DET_PAD_NAME,
		ST_SIM2RST_PAD_NAME
	};
	static enum PIN_FUNC ST_PMUXListf[ST_SIM_GPIO_CNT] = {
		ST_SIMCLK_PAD, ST_SIMDAT_PAD, ST_SIMDET_PAD, ST_SIMRST_PAD,
		ST_SIM2CLK_PAD, ST_SIM2DAT_PAD, ST_SIM2DET_PAD, ST_SIM2RST_PAD
	};
	static int ST_GPIOList_Power[ST_SIM_GPIO_CNT] = {
		ST_SIMCLK, 0, 0, ST_SIMRST,
		ST_SIM2CLK, 0, 0, ST_SIM2RST
	};
	static int ST_GPIOList_Inter[ST_SIM_GPIO_CNT][ST_SIM_GPIO_CNT] = {
		{0, ST_SIMDAT, ST_SIMDET, ST_SIMRST, 0, 0, 0, 0},
		{ST_SIMCLK, 0, ST_SIMDET, ST_SIMRST, 0, 0, 0, 0},
		{ST_SIMCLK, ST_SIMDAT, 0, ST_SIMRST, 0, 0, 0, 0},
		{ST_SIMCLK, ST_SIMDAT, ST_SIMDET, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, ST_SIM2DAT, ST_SIM2DET, ST_SIM2RST},
		{0, 0, 0, 0, ST_SIM2CLK, 0, ST_SIM2DET, ST_SIM2RST},
		{0, 0, 0, 0, ST_SIM2CLK, ST_SIM2DAT, 0, ST_SIM2RST},
		{0, 0, 0, 0, ST_SIM2CLK, ST_SIM2DAT, ST_SIM2DET, 0}
	};
	static int ST_GPIOList_Inter_High[ST_SIM_GPIO_CNT][ST_SIM_GPIO_CNT] = {
		{0, 0, 0, ST_SIMRST, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{ST_SIMCLK, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, ST_SIM2RST},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, ST_SIM2CLK, 0, 0, 0}
	};
	int Status_List[ST_SIM_GPIO_CNT] = {
		ST_PASS, ST_PASS, ST_PASS,
		ST_PASS, ST_PASS, ST_PASS,
		ST_PASS, ST_PASS
	};
	int st;
	int FirstPin = 0, LastPin = 0;

	ST_DBG("GLUE_SELFTEST::hal_selftest_sim()");

	switch (cmddata->parm1) {
	default:
	case 0:
		TestSIM1 = PowerOnSIM(ST_SIM1);
		TestSIM2 = PowerOnSIM(ST_SIM2);
		break;
	case 1:
		TestSIM1 = PowerOnSIM(ST_SIM1);
		TestSIM2 = false;
		break;
	case 2:
		TestSIM1 = false;
		TestSIM2 = PowerOnSIM(ST_SIM2);
		break;
	}

	if (TestSIM1 && TestSIM2) {
		FirstPin = SIM1_FIRST;	/* SIM 1 */
		LastPin = SIM2_LAST;	/* SIM 2 */
	} else {
		if (TestSIM1) {
			FirstPin = SIM1_FIRST;	/* SIM 1 */
			LastPin = SIM1_LAST;
		}
		if (TestSIM2) {
			FirstPin = SIM2_FIRST;	/* SIM 2 */
			LastPin = SIM2_LAST;
		}
	}

	/* Store settings and setup as GPIO */
	for (st = FirstPin; st <= LastPin; st++) {
		StoredSIMPMuxGpioValue[st].name = ST_PMUXListn[st];
		pinmux_get_pin_config(&StoredSIMPMuxGpioValue[st]);
	}
	if (TestSIM1 != 0) {
		struct regulator *regl_sim = NULL;
		regl_sim = regulator_get(NULL, SIMLDO_REGULATOR);
		if (IS_ERR(regl_sim))
			printk(KERN_INFO "%s: regulator_get failed#1(0x%X)\n",
			       __func__, (unsigned int)regl_sim);
		else {
			StoredPowerValue[ST_SIM1] =
			    regulator_is_enabled(regl_sim);
			regulator_put(regl_sim);
		}
	}
	if (TestSIM2 != 0) {
		struct regulator *regl_sim = NULL;
		regl_sim = regulator_get(NULL, SIM2LDO_REGULATOR);
		if (IS_ERR(regl_sim))
			printk(KERN_INFO "%s: regulator_get failed#1(0x%X)\n",
			       __func__, (unsigned int)regl_sim);
		else {
			StoredPowerValue[ST_SIM2] =
			    regulator_is_enabled(regl_sim);
			regulator_put(regl_sim);
		}
	}

	/* Setup pins */
	GPIOSetup.reg.val = 0;
	GPIOSetup.reg.b.drv_sth = ST_PAD_DRV_STRENGTH;

	if (TestSIM1 != 0) {
		/* Setup SIM 1 */
		ST_DBG("GLUE_SELFTEST::hal_selftest_sim() Initialize SIM1");
		for (st = SIM1_FIRST; st <= SIM1_LAST; st++) {
			ST_DBG
			    ("GLUE_SELFTEST::hal_selftest_sim() "
			     "Init GPIO%u(%u,%u)",
			     ST_GPIOList[st], ST_PMUXListn[st],
			     ST_PMUXListf[st]);
			GPIOSetup.name = ST_PMUXListn[st];
			GPIOSetup.func = ST_PMUXListf[st];
			pinmux_set_pin_config(&GPIOSetup);
			gpio_request(ST_GPIOList[st], "SIM1_GPIO");
		}
		ST_DBG
		    ("GLUE_SELFTEST::hal_selftest_sim() "
		     "SIM1 Initialized to GPIO");
	}

	/* Setup SIM 2 */
	if (TestSIM2 != 0) {
		ST_DBG("GLUE_SELFTEST::hal_selftest_sim() Initialize SIM2");
		for (st = SIM2_FIRST; st <= SIM2_LAST; st++) {
			ST_DBG
			    ("GLUE_SELFTEST::hal_selftest_sim() "
			     "Init GPIO%u(%u,%u)",
			     ST_GPIOList[st], ST_PMUXListn[st],
			     ST_PMUXListf[st]);
			GPIOSetup.name = ST_PMUXListn[st];
			GPIOSetup.func = ST_PMUXListf[st];
			pinmux_set_pin_config(&GPIOSetup);
			gpio_request(ST_GPIOList[st], "SIM2_GPIO");
		}
		ST_DBG
		    ("GLUE_SELFTEST::hal_selftest_sim() "
		     "SIM2 Initialized to GPIO");
	}

	if (TestSIM1 || TestSIM2) {

		/* Test for  grounding error */
		for (st = FirstPin; st <= LastPin; st++) {
			GPIOSetup.name = ST_PMUXListn[st];
			pinmux_get_pin_config(&GPIOSetup);
			GPIOSetup.reg.b.pull_up = 1;
			GPIOSetup.reg.b.pull_dn = 0;
			pinmux_set_pin_config(&GPIOSetup);
			gpio_direction_input(ST_GPIOList[st]);
		}
		usleep_range(SETTLING_TIME, SETTLING_TIME+500);
		for (st = FirstPin; st <= LastPin; st++) {
			ST_DBG
			    ("GLUE_SELFTEST::hal_selftest_sim() "
			     "Ground test[%u]", st);
			if (gpio_get_value(ST_GPIOList[st]) == 0) {
				/*  Shorted to ground */
				ST_DBG
				    ("GLUE_SELFTEST::hal_selftest_sim() "
				     "Ground test[%u](%u) = 0 -> Error",
				     st, ST_GPIOList[st]);

				if (Status_List[st] == ST_PASS) {
					ST_DBG("GLUE_SELFTEST::"
					       "hal_selftest_sim() "
					       "Ground test[%u] = "
					       "ST_SIM_SHORT_GROUND", st);
					Status_List[st] = ST_SIM_SHORT_GROUND;
				}
			}
		}

		/* Test for power error */
		for (st = FirstPin; st <= LastPin; st++) {
			GPIOSetup.name = ST_PMUXListn[st];
			pinmux_get_pin_config(&GPIOSetup);
			GPIOSetup.reg.b.pull_up = 0;
			GPIOSetup.reg.b.pull_dn = 1;
			pinmux_set_pin_config(&GPIOSetup);
			gpio_direction_input(ST_GPIOList[st]);
		}
		usleep_range(SETTLING_TIME, SETTLING_TIME+500);
		for (st = FirstPin; st <= LastPin; st++) {
			if (ST_GPIOList_Power[st] == 0) {
				/* Pin cannot be power tested as
				   external pullup is applied */
				continue;
			}

			ST_DBG
			    ("GLUE_SELFTEST::hal_selftest_sim() Power test[%u]",
			     st);
			if (gpio_get_value(ST_GPIOList_Power[st]) == 1) {
				/*  Shorted to Power */
				ST_DBG
				    ("GLUE_SELFTEST::hal_selftest_sim() "
				     "Power test[%u](%u) = 1 -> Error",
				     st, ST_GPIOList_Power[st]);
				if (Status_List[st] == ST_PASS) {
					ST_DBG("GLUE_SELFTEST::"
					       "hal_selftest_sim()"
					       "Power test[%u] = "
					       "ST_SIM_SHORT_POWER", st);
					Status_List[st] = ST_SIM_SHORT_POWER;
				}
			}
		}

		/* Test for Interconnect errors - Low */
		for (st = FirstPin; st <= LastPin; st++) {
			GPIOSetup.name = ST_PMUXListn[st];
			pinmux_get_pin_config(&GPIOSetup);
			GPIOSetup.reg.b.pull_up = 1;
			GPIOSetup.reg.b.pull_dn = 0;
			pinmux_set_pin_config(&GPIOSetup);
			gpio_direction_input(ST_GPIOList[st]);

		}
		for (st = FirstPin; st <= LastPin; st++) {
			int i;
			GPIOSetup.name = ST_PMUXListn[st];
			pinmux_get_pin_config(&GPIOSetup);
			GPIOSetup.reg.b.pull_up = 1;
			GPIOSetup.reg.b.pull_dn = 0;
			pinmux_set_pin_config(&GPIOSetup);
			gpio_direction_output(ST_GPIOList[st], 0);
			gpio_set_value(ST_GPIOList[st], 0);

		    usleep_range(SETTLING_TIME, SETTLING_TIME+500);
			for (i = 0; i <= MAX_SIM_INDEX; i++) {
				if (ST_GPIOList_Inter[st][i] == 0) {
					/* This connection can not be tested due
					   to external pullup or is placed of
					   different sim slot */
					continue;
				}
				if ((Status_List[i] != ST_PASS)
				    && (Status_List[i] !=
					ST_SIM_SHORT_INTERCONNECT))
					/* This pin failed in High/Low test */
					continue;
				ST_DBG
				    ("GLUE_SELFTEST::hal_selftest_sim() "
				     "Interconnect test - Low[%u->%i](%u) ",
				     st, i, ST_GPIOList_Inter[st][i]);
				if (gpio_get_value(ST_GPIOList_Inter[st][i]) ==
				    0) {
					/* Interconnect problem found */
					ST_DBG("GLUE_SELFTEST::"
					       "hal_selftest_sim() "
					       "Interconnect test - "
					       "Low[%u->%u]= Low -> Error",
					       st, i);
					if (Status_List[st] == ST_PASS) {
						Status_List[st] =
						    ST_SIM_SHORT_INTERCONNECT;
						ST_DBG("GLUE_SELFTEST::"
						       "hal_selftest_sim() "
						       "Interconnect test - "
						       "Low[%u->%u] = ST_"
						       "SELFTEST_SIM_SHORT_"
						       "INTERCONNECT", st, i);
					}
				}
			}
			gpio_direction_input(ST_GPIOList[st]);
		}

		/* Test for Interconnect errors - High  */
		for (st = FirstPin; st <= LastPin; st++) {
			GPIOSetup.name = ST_PMUXListn[st];
			pinmux_get_pin_config(&GPIOSetup);
			GPIOSetup.reg.b.pull_up = 0;
			GPIOSetup.reg.b.pull_dn = 1;
			pinmux_set_pin_config(&GPIOSetup);
			gpio_direction_input(ST_GPIOList[st]);

		}
		for (st = FirstPin; st <= LastPin; st++) {
			int i;
			GPIOSetup.name = ST_PMUXListn[st];
			pinmux_get_pin_config(&GPIOSetup);
			GPIOSetup.reg.b.pull_up = 0;
			GPIOSetup.reg.b.pull_dn = 1;
			pinmux_set_pin_config(&GPIOSetup);
			gpio_direction_output(ST_GPIOList[st], 1);
			gpio_set_value(ST_GPIOList[st], 1);

		    usleep_range(SETTLING_TIME, SETTLING_TIME+500);
			for (i = 0; i <= MAX_SIM_INDEX; i++) {
				if (ST_GPIOList_Inter_High[st][i] == 0) {
					/* This connection can not be tested due
					   to external pullup or is placed of
					   different sim slot */
					continue;
				}
				if ((Status_List[i] != ST_PASS)
				    && (Status_List[i] !=
					ST_SIM_SHORT_INTERCONNECT)) {
					/* This pin failed in High/Low test */
					continue;
				}

				ST_DBG
				    ("GLUE_SELFTEST::hal_selftest_sim() "
				     "Interconnect test - High[%u->%i](%u)",
				     st, i, ST_GPIOList_Inter[st][i]);
				if (gpio_get_value
				    (ST_GPIOList_Inter_High[st][i]) == 1) {
					/* Interconnect problem found */
					ST_DBG("GLUE_SELFTEST::"
					       "hal_selftest_sim() "
					       "Interconnect test - "
					       "High[%u->%u] = High -> Error",
					       st, i);
					if (Status_List[st] == ST_PASS) {
						Status_List[st] =
						    ST_SIM_SHORT_INTERCONNECT;
						ST_DBG("GLUE_SELFTEST::"
						       "hal_selftest_sim() "
						       "Interconnect test - "
						       "High[%u->%u] = ST_"
						       "SELFTEST_SIM_SHORT_"
						       "INTERCONNECT", st, i);
					}
				}
			}
			gpio_direction_input(ST_GPIOList[st]);
		}

		/* Restore settings */
		for (st = FirstPin; st <= LastPin; st++)
			pinmux_set_pin_config(&StoredSIMPMuxGpioValue[st]);

		if (TestSIM1 != 0) {
			struct regulator *regl_sim = NULL;
			regl_sim = regulator_get(NULL, SIMLDO_REGULATOR);
			if (IS_ERR(regl_sim))
				printk(KERN_INFO
				       "%s: regulator_get failed(0x%X)\n",
				       __func__, (unsigned int)regl_sim);
			else {
				if (StoredPowerValue[ST_SIM1])
					regulator_enable(regl_sim);
				else
					regulator_disable(regl_sim);
				regulator_put(regl_sim);
			}
		}
		if (TestSIM2 != 0) {
			struct regulator *regl_sim = NULL;
			regl_sim = regulator_get(NULL, SIM2LDO_REGULATOR);
			if (IS_ERR(regl_sim))
				printk(KERN_INFO
				       "%s: regulator_get failed(0x%X)\n",
				       __func__, (unsigned int)regl_sim);
			else {
				if (StoredPowerValue[ST_SIM2])
					regulator_enable(regl_sim);
				else
					regulator_disable(regl_sim);
				regulator_put(regl_sim);
			}
		}

		/* Copy status to return structure */
		if (Status_List[0] != ST_PASS
		    || Status_List[1] != ST_PASS
		    || Status_List[2] != ST_PASS
		    || Status_List[3] != ST_PASS
		    || Status_List[4] != ST_PASS
		    || Status_List[5] != ST_PASS
		    || Status_List[6] != ST_PASS || Status_List[7] != ST_PASS) {
			ST_DBG
			    ("GLUE_SELFTEST::std_selftest_irq() "
			     "Connection is bad");
			cmddata->testStatus = ST_FAIL;
			cmddata->subtestCount = 8;
			for (st = SIM1_FIRST; st <= SIM2_LAST; st++)
				cmddata->subtestStatus[st] = Status_List[st];
		} else {
			ST_DBG
			    ("GLUE_SELFTEST::std_selftest_irq()"
			     " Connection is good");
			cmddata->testStatus = ST_PASS;
			cmddata->subtestCount = 0;
		}

	}
}

#endif

#ifdef GPS_TEST_SUPPORTED
/*---------------------------------------------------------------------------*/
/*! \brief ST_control_gps_io.
*  Self Test for gps control pins
*
* \param name		- The logical name of the GenIO pin
* \param direction	- Defines if read or write operation shall be performed
* \param state		- Pointer to data type that contains the pin state
*
* \return		- The outcome of the self test procedure
*/
static void std_selftest_control_gps_io(struct SelftestUserCmdData_t *cmddata)
{
	u8 result = ST_FAIL;	/* set to default */
	struct pin_config GPIOSetup;
	int ret;
	/* Input */
	u8 name = cmddata->parm1;
	u8 direction = cmddata->parm2;
	u8 *state = (u8 *) &cmddata->parm3;
	unsigned gpio;
	enum PIN_FUNC PF_gpio;

	ST_DBG
	    ("GLUE_SELFTEST::std_selftest_control_gps_io () "
	     "called -- ST_GPS_READ/WRITE %x %x %x", name, direction, *state);

	GPIOSetup.reg.val = 0;
	GPIOSetup.reg.b.drv_sth = DRIVE_STRENGTH_2MA;

	switch (name) {
	case ST_GPS_TXP:
		pinmux_find_gpio(PN_GPS_PABLANK, &gpio, PF_gpio);
		if (GPS_PABLANK_Setup_as_GPIO == false) {
			ST_DBG
			    ("GLUE_SELFTEST::std_selftest_control_gps_io ()"
			     " GPS_PABLANK_Setup_as_GPIO");
			StoredValue_GPS_PABLANK.name = PN_GPS_PABLANK;
			pinmux_get_pin_config(&StoredValue_GPS_PABLANK);
			GPIOSetup.name = PN_GPS_PABLANK;
			GPIOSetup.func = PF_gpio;
			pinmux_set_pin_config(&GPIOSetup);
			ST_DBG
			    ("GLUE_SELFTEST::std_selftest_control_gps_io ()"
			     " 0x%08X", StoredValue_GPS_PABLANK.reg.val);
			ret = gpio_request(gpio, "GPS PABLANK");
			if (ret < 0) {
				ST_DBG("GLUE_SELFTEST::gpio %u request failed",
				       GPIO_GPS_PABLANK);
				result = ST_FAIL;
			}
			GPS_PABLANK_Setup_as_GPIO = true;
		}
		switch (direction) {
		case ST_GPS_WRITE:{
				switch (*state) {
				case ST_GPS_HIGH:
					gpio_direction_output(gpio, 1);
					gpio_set_value(gpio, 1);
					ST_DBG("GLUE_SELFTEST::"
					       "std_selftest_control_gps_io ()"
					       " GPS_PABLANK - Write - High");
					break;
				case ST_GPS_LOW:
					gpio_direction_output(gpio, 0);
					gpio_set_value(gpio, 0);
					ST_DBG("GLUE_SELFTEST::"
					       "std_selftest_control_gps_io ()"
					       " GPS_PABLANK - Write - Low");
					break;
				case ST_GPS_RELEASE:
				default:
					/* assume release state,
					   defined in ISI */
					if (GPS_PABLANK_Setup_as_GPIO == true) {
						ST_DBG("GLUE_SELFTEST::"
						       "std_selftest_control"
						       "_gps_io () GPS_PABLANK"
						       " - Restore");
						pinmux_set_pin_config
						    (&StoredValue_GPS_PABLANK);
						gpio_free(gpio);
						GPS_PABLANK_Setup_as_GPIO =
						    false;
					}
					break;
				}
				result = ST_PASS;
				break;
			}
		case ST_GPS_READ:
			gpio_direction_input(gpio);
			ST_DBG
			    ("GLUE_SELFTEST::std_selftest_control_gps_io ()"
			     " GPS_PABLANK - Read");
			if (gpio_get_value(gpio) == 1) {
				ST_DBG("GLUE_SELFTEST::"
				       "std_selftest_control_gps_io ()>>>High");
				*state = ST_GPS_HIGH;
			} else {
				ST_DBG("GLUE_SELFTEST::"
				       "std_selftest_control_gps_io ()>>>Low");
				*state = ST_GPS_LOW;
			}
			result = ST_PASS;
			break;
		default:
			assert(false);
			break;
		}
		break;
	case ST_GPS_TIMESTAMP:
		pinmux_find_gpio(PN_GPS_TMARK, &gpio, PF_gpio);
		if (GPS_TMARK_Setup_as_GPIO == false) {
			ST_DBG
			    ("GLUE_SELFTEST::std_selftest_control_gps_io ()"
			     " GPS_TMARK_Setup_as_GPIO");
			StoredValue_GPS_TMARK.name = PN_GPS_TMARK;
			pinmux_get_pin_config(&StoredValue_GPS_TMARK);
			GPIOSetup.name = PN_GPS_TMARK;
			GPIOSetup.func = PF_gpio;
			pinmux_set_pin_config(&GPIOSetup);
			ST_DBG
			    ("GLUE_SELFTEST::std_selftest_control_gps_io ()"
			     " 0x%08X", StoredValue_GPS_TMARK.reg.val);
			ret = gpio_request(gpio, "GPS TMARK");
			if (ret < 0) {
				ST_DBG("GLUE_SELFTEST::gpio %u request failed",
				       GPIO_GPS_PABLANK);
				result = ST_FAIL;
			}
			GPS_TMARK_Setup_as_GPIO = true;
		}
		switch (direction) {
		case ST_GPS_WRITE:{
				switch (*state) {
				case ST_GPS_HIGH:
					gpio_direction_output(gpio, 1);
					gpio_set_value(gpio, 1);
					ST_DBG("GLUE_SELFTEST::"
					       "std_selftest_control_gps_io ()"
					       " GPS_TMARK - Write - High");
					break;
				case ST_GPS_LOW:
					gpio_direction_output(gpio, 0);
					gpio_set_value(gpio, 0);
					ST_DBG("GLUE_SELFTEST::"
					       "std_selftest_control_gps_io ()"
					       " GPS_TMARK - Write - Low");
					break;
				case ST_GPS_RELEASE:
				default:
					/* assume release state,
					   defined in ISI */
					if (GPS_TMARK_Setup_as_GPIO == true) {
						ST_DBG("GLUE_SELFTEST::"
						       "std_selftest_control_"
						       "gps_io () GPS_TMARK"
						       " - Restore");
						pinmux_set_pin_config
						    (&StoredValue_GPS_TMARK);
						gpio_free(gpio);
						GPS_TMARK_Setup_as_GPIO = false;
					}
					break;
				}
				result = ST_PASS;
				break;
			}

		case ST_GPS_READ:
			ST_DBG
			    ("GLUE_SELFTEST::std_selftest_control_gps_io ()"
			     " GPS_TMARK - Read");
			gpio_direction_input(gpio);
			if (gpio_get_value(gpio) == 1) {
				ST_DBG("GLUE_SELFTEST::"
				       "std_selftest_control_gps_io () >>>High");
				*state = ST_GPS_HIGH;
			} else {
				ST_DBG("GLUE_SELFTEST::"
				       "std_selftest_control_gps_io () >>>Low");
				*state = ST_GPS_LOW;
			}
			result = ST_PASS;
			break;
		default:
			assert(false);
			break;
		}
		break;
	default:
		result = ST_NOT_TESTED;
		break;
	}

	if (ST_PASS == result) {
		ST_DBG
		    ("GLUE_SELFTEST::std_selftest_control_gps_io () returned "
		     "-----> ST_PASS");
	} else {
		ST_DBG
		    ("GLUE_SELFTEST::std_selftest_control_gps_io () returned "
		     "-----> ST_FAIL");
	}

	cmddata->testStatus = result;
}
#endif

#ifdef USB_ST_SUPPORTED
static u8 hal_selftest_usb_charger_latch_ok;	/* = 0; */
static u8 hal_selftest_usb_charger_called;	/* = 0; */

static void std_selftest_usb_event_notif_callback(struct bcmpmu *pmu_handle,
						  unsigned char event,
						  void *param1, void *data)
{
	struct bcmpmu_selftest *bcmpmu_selftest =
	    (struct bcmpmu_selftest *)data;

	if (!data) {
		dev_info(bcmpmu_selftest->bcmpmu->dev,
			 "ERROR: xceiver data not passed by PMU callback\n");
		return;
	}

	switch (event) {
	case BCMPMU_USB_EVENT_CHGDET_LATCH:
		ST_DBG
		    ("GLUE_SELFTEST::std_selftest_usb_event_notif_callback::"
		     "BCMPMU_USB_EVENT_CHGDET_LATCH");
		hal_selftest_usb_charger_latch_ok = 1;
		break;
	default:
		break;
	}
}
static void std_selftest_usb_charger(struct SelftestUserCmdData_t *cmddata)
{
	unsigned int mbc5_cdet_orig, mbc5_usb_det_ldo_en, i;
	u8 status = 0;

	if (!hal_selftest_usb_charger_called) {
		hal_selftest_usb_charger_latch_ok = 0;
		if (bcmpmu_selftest->bcmpmu->register_usb_callback) {
			/* Register callback functions for PMU events */
			status =
			    bcmpmu_selftest->bcmpmu->
			    register_usb_callback(bcmpmu_selftest->bcmpmu,
					  std_selftest_usb_event_notif_callback,
						  (void *)bcmpmu_selftest);
		}
		ST_DBG("GLUE_SELFTEST::hal_selftest_usb_charger() Status: %d",
		       status);
		hal_selftest_usb_charger_called = 1;
	}

	/* Set PMU into BCDLDO Always on in MBCCTRL5 [1] */
	bcmpmu_selftest->bcmpmu->read_dev(bcmpmu_selftest->bcmpmu,
					  PMU_REG_MBCCTRL5_CHARGE_DET,
					  &mbc5_cdet_orig,
					  bcmpmu_selftest->bcmpmu->
					  regmap[PMU_REG_MBCCTRL5_CHARGE_DET].
					  mask);
	bcmpmu_selftest->bcmpmu->read_dev(bcmpmu_selftest->bcmpmu,
					  PMU_REG_MBCCTRL5_USB_DET_LDO_EN,
					  &mbc5_usb_det_ldo_en,
					  PMU_BITMASK_ALL);
	mbc5_usb_det_ldo_en =
	    mbc5_usb_det_ldo_en >> bcmpmu_selftest->bcmpmu->
	    regmap[PMU_REG_MBCCTRL5_USB_DET_LDO_EN].shift;
	{
		unsigned int val;
		bcmpmu_selftest->bcmpmu->read_dev_drct(bcmpmu_selftest->bcmpmu,
						       0, 0x54, &val,
						       PMU_BITMASK_ALL);
		ST_DBG("GLUE_SELFTEST::hal_selftest_usb_charger()"
		       " MBCCTRL5 val: %u", val);
	}

	if (mbc5_usb_det_ldo_en) {
		/* LDO is already on. */
		ST_DBG
		    ("GLUE_SELFTEST::hal_selftest_usb_charger(); "
		     "USB_DET_LDO is already on, orig = %x aon = %x",
		     mbc5_cdet_orig, 0);
		bcmpmu_selftest->bcmpmu->write_dev(bcmpmu_selftest->bcmpmu,
					   PMU_REG_MBCCTRL5_USB_DET_LDO_EN,
					   0,
					   bcmpmu_selftest->bcmpmu->
					   regmap
					   [PMU_REG_MBCCTRL5_USB_DET_LDO_EN].
					   mask);
		usleep_range(5000, 5500);
	}

	bcmpmu_selftest->bcmpmu->write_dev(bcmpmu_selftest->bcmpmu,
					   PMU_REG_MBCCTRL5_CHARGE_DET,
					   0x7 << bcmpmu_selftest->bcmpmu->
					   regmap[PMU_REG_MBCCTRL5_CHARGE_DET].
					   shift,
					   bcmpmu_selftest->bcmpmu->
					   regmap[PMU_REG_MBCCTRL5_CHARGE_DET].
					   mask);

	{
		unsigned int val;
		bcmpmu_selftest->bcmpmu->read_dev_drct(bcmpmu_selftest->bcmpmu,
						       0, 0x54, &val,
						       PMU_BITMASK_ALL);
		ST_DBG("GLUE_SELFTEST::hal_selftest_usb_charger()"
		       " MBCCTRL5 val: %u", val);
	}

	ST_DBG("GLUE_SELFTEST::hal_selftest_usb_charger() "
	       "MBCCTRL5 original %x, aon %x", mbc5_cdet_orig, 1);

	for (i = 0; i < 200; i++) {
		/* wait for CHGDET_LATCH status */
		usleep_range(2000, 2500);
		if (hal_selftest_usb_charger_latch_ok)
			break;
	}
	msleep(25);

	/* Restore MBCCTRL5 */
	bcmpmu_selftest->bcmpmu->write_dev(bcmpmu_selftest->bcmpmu,
					   PMU_REG_MBCCTRL5_CHARGE_DET,
					   mbc5_cdet_orig,
					   bcmpmu_selftest->bcmpmu->
					   regmap[PMU_REG_MBCCTRL5_CHARGE_DET].
					   mask);
	/* IF chp_typ == 0x01, test is working correctly.
	   chp_typ is located in bits 4 and 5. */

	if (hal_selftest_usb_charger_latch_ok) {
		ST_DBG
		    ("GLUE_SELFTEST::hal_selftest_usb_charger() "
		     "Connection is good");
		cmddata->testStatus = ST_PASS;
		cmddata->subtestCount = 0;
	} else {
		ST_DBG
		    ("GLUE_SELFTEST::hal_selftest_usb_charger() "
		     "Connection is bad");
		cmddata->testStatus = ST_FAIL;
		cmddata->subtestCount = 1;
		cmddata->subtestStatus[0] = ST_FAIL;
	}
}
#endif

#ifdef ADC_ST_SUPPORTED
#define TEST_DELAY 10
static void std_selftest_adc(struct SelftestUserCmdData_t *cmddata)
{
	int result = false;
	int reading;
	int i = 0, ret;
	struct bcmpmu_adc_req ReqData;
	ST_DBG("GLUE_SELFTEST::std_selftest_adc() Start");
	do {
		ReqData.sig = PMU_ADC_VMBATT;
		ReqData.tm = PMU_ADC_TM_RTM_SW_TEST;
		ReqData.flags = PMU_ADC_RAW_ONLY;
		ret =
		    bcmpmu_selftest->bcmpmu->adc_req(bcmpmu_selftest->bcmpmu,
						     &ReqData);
		reading = ReqData.raw;
		if (reading < 0)
			msleep(20);
		ST_DBG("GLUE_SELFTEST::std_selftest_adc() Test loop(%u,%i)", i,
		       ret);
	} while ((reading < 0) && (i++ < 5) && (reading != -EIO));
	if (reading > 0 || reading == -EIO)
		result = true;

	if (result) {
		ST_DBG("GLUE_SELFTEST::std_selftest_adc() Connection is good");
		cmddata->testStatus = ST_PASS;
		cmddata->subtestCount = 0;
	} else {
		ST_DBG("GLUE_SELFTEST::std_selftest_adc() Connection is bad");
		cmddata->testStatus = ST_FAIL;
		cmddata->subtestCount = 1;
		cmddata->subtestStatus[0] = ST_FAIL;
	}
}
#endif

/********************************************************************/
/* Command Handling helpers */
/********************************************************************/

static ssize_t FormatTestResult(struct SelftestUserCmdData_t cmddata, char *buf)
{
	int i;
	char *cbuf;
	cbuf = buf;

	/* Normal result */
	sprintf(cbuf, "%s\n", Selftest_ResultString[cmddata.testStatus]);
	/* Detailed result */
	if ((cmddata.ResultFormat == ST_RESULT_FORMAT_DETAILED) &&
	    (cmddata.testStatus == ST_FAIL) && (cmddata.subtestCount != 0)) {
		cbuf += strlen(cbuf);
		sprintf(cbuf, "%i\n", cmddata.subtestCount);
		cbuf += strlen(cbuf);
		for (i = 0; i < cmddata.subtestCount; i++) {
			sprintf(cbuf, "%s\n",
				Selftest_ResultString[cmddata.
						      subtestStatus[i]]);
			cbuf += strlen(cbuf);
		}
	}
	return strlen(buf);
}

/********************************************************************/
/* Selftest Sysfs file interface */
/********************************************************************/
#ifdef USB_ST_SUPPORTED
static ssize_t show_selftest_usb(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct SelftestUserCmdData_t cmddata;

	cmddata.ResultFormat = SelftestData.usb.rf;

	if (!TestActive[SELFTEST_USB]) {
		TestActive[SELFTEST_USB] = true;
		std_selftest_usb_charger(&cmddata);
		TestActive[SELFTEST_USB] = false;
	} else {
		cmddata.testStatus = ST_BUSY;
		cmddata.subtestCount = 0;
	}

	return FormatTestResult(cmddata, buf);
}
static ssize_t store_selftest_usb(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	int format;

	if ((sscanf(buf, "%d", &format)) != 1)
		return -EINVAL;

	SelftestData.usb.rf = format;

	return strnlen(buf, count);
}
#endif
#ifdef ADC_ST_SUPPORTED
static ssize_t show_selftest_adcif(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct SelftestUserCmdData_t cmddata;

	cmddata.ResultFormat = SelftestData.adc.rf;

	if (!TestActive[SELFTEST_ADC]) {
		TestActive[SELFTEST_ADC] = true;
		std_selftest_adc(&cmddata);
		TestActive[SELFTEST_ADC] = false;
	} else {
		cmddata.testStatus = ST_BUSY;
		cmddata.subtestCount = 0;
	}

	return FormatTestResult(cmddata, buf);
}
static ssize_t store_selftest_adcif(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	int format;

	if ((sscanf(buf, "%d", &format)) != 1)
		return -EINVAL;

	SelftestData.adc.rf = format;

	return strnlen(buf, count);
}
#endif
#ifdef I2C_ST_SUPPORTED
static ssize_t show_selftest_i2c(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct SelftestUserCmdData_t cmddata;

	cmddata.ResultFormat = SelftestData.i2c.rf;

	if (!TestActive[SELFTEST_I2C]) {
		TestActive[SELFTEST_I2C] = true;
		std_selftest_i2c(&cmddata);
		TestActive[SELFTEST_I2C] = false;
	} else {
		cmddata.testStatus = ST_BUSY;
		cmddata.subtestCount = 0;
	}

	return FormatTestResult(cmddata, buf);
}
static ssize_t store_selftest_i2c(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	int format;

	if ((sscanf(buf, "%d", &format)) != 1)
		return -EINVAL;

	SelftestData.sc.rf = format;

	return strnlen(buf, count);
}
#endif
#ifdef IRQ_ST_SUPPORTED
static ssize_t show_selftest_irq(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct SelftestUserCmdData_t cmddata;

	cmddata.ResultFormat = SelftestData.irq.rf;

	if (!TestActive[SELFTEST_IRQ]) {
		TestActive[SELFTEST_IRQ] = true;
		std_selftest_irq(&cmddata);
		TestActive[SELFTEST_IRQ] = false;
	} else {
		cmddata.testStatus = ST_BUSY;
		cmddata.subtestCount = 0;
	}

	return FormatTestResult(cmddata, buf);
}
static ssize_t store_selftest_irq(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	int format;

	if ((sscanf(buf, "%d", &format)) != 1)
		return -EINVAL;

	SelftestData.irq.rf = format;

	return strnlen(buf, count);
}
#endif
#ifdef BATRM_ST_SUPPORTED
static ssize_t show_selftest_batrm(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct SelftestUserCmdData_t cmddata;

	cmddata.ResultFormat = SelftestData.batrm.rf;

	if (!TestActive[SELFTEST_BATRM]) {
		TestActive[SELFTEST_BATRM] = true;
		std_selftest_batrm(&cmddata);
		TestActive[SELFTEST_BATRM] = false;
	} else {
		cmddata.testStatus = ST_BUSY;
		cmddata.subtestCount = 0;
	}
	return FormatTestResult(cmddata, buf);
}
static ssize_t store_selftest_batrm(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	int format;

	if ((sscanf(buf, "%d", &format)) != 1)
		return -EINVAL;

	SelftestData.batrm.rf = format;

	return strnlen(buf, count);
}
#endif
#ifdef SLEEPCLOCK_SUPPORTED
static ssize_t show_selftest_sc(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct SelftestUserCmdData_t cmddata;

	cmddata.ResultFormat = SelftestData.sc.rf;

	if (!TestActive[SELFTEST_SC]) {
		TestActive[SELFTEST_SC] = true;
		std_selftest_sleepclk(&cmddata);
		TestActive[SELFTEST_SC] = false;
	} else {
		cmddata.testStatus = ST_BUSY;
		cmddata.subtestCount = 0;
	}

	return FormatTestResult(cmddata, buf);
}
static ssize_t store_selftest_sc(struct device *dev,
				 struct device_attribute *attr, const char *buf,
				 size_t count)
{
	int format;

	if ((sscanf(buf, "%d", &format)) != 1)
		return -EINVAL;

	SelftestData.sc.rf = format;

	return strnlen(buf, count);
}
#endif
#ifdef SIM_SHORT_ST_SUPPORTED
static ssize_t show_selftest_sim(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct SelftestUserCmdData_t cmddata;

	cmddata.ResultFormat = SelftestData.sim.rf;
	cmddata.parm1 = SelftestData.sim.slot;

	if (!TestActive[SELFTEST_SIM]) {
		TestActive[SELFTEST_SIM] = true;
		std_selftest_sim(&cmddata);
		TestActive[SELFTEST_SIM] = false;
	} else {
		cmddata.testStatus = ST_BUSY;
		cmddata.subtestCount = 0;
	}

	return FormatTestResult(cmddata, buf);
}
static ssize_t store_selftest_sim(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	int format, slot;
	int parms;

	parms = sscanf(buf, "%d %d", &format, &slot);
	if ((parms == 0) || (parms > 2))
		return -EINVAL;
	if (parms >= 1)
		SelftestData.sim.rf = format;
	if (parms >= 2)
		SelftestData.sim.slot = slot;
	return strnlen(buf, count);
}
#endif
/* PMU */
#ifdef USB_ST_SUPPORTED
static DEVICE_ATTR(selftest_usb, S_IRUGO | S_IWUSR, show_selftest_usb,
		   store_selftest_usb);
#endif
#ifdef ADC_ST_SUPPORTED
static DEVICE_ATTR(selftest_adcif, S_IRUGO | S_IWUSR, show_selftest_adcif,
		   store_selftest_adcif);
#endif
#ifdef I2C_ST_SUPPORTED
static DEVICE_ATTR(selftest_i2c, S_IRUGO | S_IWUSR, show_selftest_i2c,
		   store_selftest_i2c);
#endif
#ifdef IRQ_ST_SUPPORTED
static DEVICE_ATTR(selftest_irq, S_IRUGO | S_IWUSR, show_selftest_irq,
		   store_selftest_irq);
#endif
#ifdef BATRM_ST_SUPPORTED
static DEVICE_ATTR(selftest_batrm, S_IRUGO | S_IWUSR, show_selftest_batrm,
		   store_selftest_batrm);
#endif
#ifdef SLEEPCLOCK_SUPPORTED
static DEVICE_ATTR(selftest_sc, S_IRUGO | S_IWUSR, show_selftest_sc,
		   store_selftest_sc);
#endif
/* SIM */
#ifdef SIM_SHORT_ST_SUPPORTED
static DEVICE_ATTR(selftest_sim, S_IRUGO | S_IWUSR, show_selftest_sim,
		   store_selftest_sim);
#endif
#define BCMPMU_ST_DRIVER_NAME "bcmpmu_selftest"

static struct attribute *bcmpmu_selftest_attrs[] = {
#ifdef USB_ST_SUPPORTED
	&dev_attr_selftest_usb.attr,
#endif
#ifdef ADC_ST_SUPPORTED
	&dev_attr_selftest_adcif.attr,
#endif
#ifdef I2C_ST_SUPPORTED
	&dev_attr_selftest_i2c.attr,
#endif
#ifdef IRQ_ST_SUPPORTED
	&dev_attr_selftest_irq.attr,
#endif
#ifdef BATRM_ST_SUPPORTED
	&dev_attr_selftest_batrm.attr,
#endif
#ifdef SLEEPCLOCK_SUPPORTED
	&dev_attr_selftest_sc.attr,
#endif
#ifdef SIM_SHORT_ST_SUPPORTED
	&dev_attr_selftest_sim.attr,
#endif
	NULL
};

static const struct attribute_group bcmpmu_selftest_attr_group = {
	.attrs = bcmpmu_selftest_attrs,
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
	int ret;

	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_selftest *pselftest;

	ST_DBG("bcmpmu_selftest: probe called\n");

	/* Static data setup */
	pselftest = kzalloc(sizeof(struct bcmpmu_selftest), GFP_KERNEL);
	if (pselftest == NULL) {
		printk("bcmpmu_selftest: failed to alloc mem.\n");
		return -ENOMEM;
	}
	pselftest->bcmpmu = bcmpmu;
	pselftest->pdev = pdev;
	bcmpmu_selftest = (void *)pselftest;

	ret =
	    sysfs_create_group(&bcmpmu_selftest->pdev->dev.kobj,
			       &bcmpmu_selftest_attr_group);

	return ret;
}

static int __devexit selftest_pmu_remove(struct platform_device *pdev)
{
	sysfs_remove_group(&bcmpmu_selftest->pdev->dev.kobj,
			   &bcmpmu_selftest_attr_group);
	kfree(bcmpmu_selftest);
	return 0;
}

static struct platform_driver bcmpmu_selftest_driver = {
	.probe = selftest_pmu_probe,
	.remove = __devexit_p(selftest_pmu_remove),
	.driver = {
		   .name = BCMPMU_ST_DRIVER_NAME,
		   }
};

/****************************************************************************
*
*  selftest_pmu_init
*
*     Called to perform module initialization when the module is loaded
*
***************************************************************************/

static int __init selftest_pmu_init(void)
{
	int ret;

	ST_DBG("bcmpmu_selftest: selftest_pmu_init called\n");
	/* Register driver */
	ret = platform_driver_register(&bcmpmu_selftest_driver);

	return ret;
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
	platform_driver_unregister(&bcmpmu_selftest_driver);
}

module_init(selftest_pmu_init);
module_exit(selftest_pmu_exit);

MODULE_AUTHOR("PHERAGER");
MODULE_DESCRIPTION("BCMPMU_SELFTEST");
MODULE_LICENSE("GPL");
