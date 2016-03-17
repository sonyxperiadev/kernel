/*******************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/
/**
*
*   @file   bcm_auddrv_test.c
*
*   @brief	This file contains SysFS interface for audio driver test cases
*
****************************************************************************/

#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/dma-mapping.h>

#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm_params.h>
#include <sound/pcm.h>
#include <sound/rawmidi.h>
#include <sound/initval.h>

#include <linux/sysfs.h>

#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>

#include <mach/hardware.h>

#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include "chal_types.h"

#include "csl_caph.h"

#include "msconsts.h"
#include "shared.h"
#include "csl_aud_queue.h"
#include "csl_vpu.h"
#include "csl_arm2sp.h"

#include <mach/io_map.h>
#include "linux/gpio.h"
#include "linux/interrupt.h"
#include "mach/chip_pinmux.h"
#include "mach/pinmux.h"
#include "audio_trace.h"

#define IHF_ST_SUPPORTED
#define HEADSET_ST_SUPPORTED
#define DIGIMIC_SUPPORTED

#if defined(IHF_ST_SUPPORTED) | defined(HEADSET_ST_SUPPORTED)

/* BRCM audio */
#include "chal_caph_audioh.h"

/* BRCM PMU */
#include "linux/mfd/bcmpmu.h"
#include "linux/broadcom/bcmpmu_audio.h"

/* RDB access */
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_audioh.h>
#include <mach/rdb/brcm_rdb_khub_clk_mgr_reg.h>	/* For DigiMic test */
#include <mach/rdb/brcm_rdb_sclkcal.h>	/* For Sleep clock test */
#include <mach/rdb/brcm_rdb_bmdm_clk_mgr_reg.h>	/* For Sleep clock test */
#include <chal/chal_util.h>

/* HW Settling time */
#define AUDIO_SETTLING_TIME 15
#define DMIC_SETTLING_TIME 30

#define BB_TEST_DISABLE		0x00
#define BB_TEST_10_KOHM		0x01
#define BB_TEST_500_OHM		0x02
#define BB_TEST_500_OHM_10_KOHM	0x03

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
		aTrace(LOG_AUDIO_DRIVER, "GLUE_SELFTEST::" \
			"CHECKBIT_AND_ASSIGN_ERROR(%u):" \
			" AL:%u,  TV=0x%X, BTV=0x%X",\
			i, xAssertLevel, TestValue,\
			((TestValue) & (1 << i))); \
	if (xResultArray[i] == ST_PASS) {\
			if ((xAssertLevel == 1))  { \
				aTrace(LOG_AUDIO_DRIVER, "GLUE_SELFTEST::" \
					"CHECKBIT_AND_ASSIGN_ERROR:" \
					" High Check (%u)", i); \
		if (((TestValue) & (1 << i)) != 0) { \
			xResultArray[i]  =  xErrorCode; \
		  aTrace(LOG_AUDIO_DRIVER, "GLUE_SELFTEST::" \
				"CHECKBIT_AND_ASSIGN_ERROR:" \
				" High Assign err = %u", \
			  xErrorCode); \
		} \
	  } \
	  else { \
		aTrace(LOG_AUDIO_DRIVER, "GLUE_SELFTEST::" \
			"CHECKBIT_AND_ASSIGN_ERROR:" \
			" Low Check (%u)", i); \
		if (((TestValue) & (1 << i)) == 0) { \
			xResultArray[i] = xErrorCode; \
			aTrace(LOG_AUDIO_DRIVER, "GLUE_SELFTEST::" \
				"CHECKBIT_AND_ASSIGN_ERROR:" \
				" Low Assign err = %u", \
				xErrorCode); \
		} \
	  } \
	} \
	} \
	}

#endif

enum Selftest_Results_t {
	ST_PASS,	/* Test succeded */
	ST_FAIL,	/* Test Failed */
	ST_BUSY,
	ST_NOT_TESTED,
/* Audio */
	ST_SHORTED_GROUND,
	ST_SHORTED_POWER,
	ST_BAD_CONNECTION,
	ST_BAD_CONNECTION_OR_GROUND,
	ST_BAD_CONNECTION_OR_POWER,
	ST_SHORTED_GROUND_ACI,
	ST_SHORTED_GROUND_BIAS,
	ST_SHORTED_POWER_ACI,
	ST_SHORTED_POWER_BIAS,
	ST_RESULT_CNT
};

static const char * const Selftest_ResultString[] = {
	"PASS",			/* Test succeded */
	"FAIL",			/* Test Failed */
	"EBUSY",
	"NOT_TESTED",
/* Audio */
	"SHORTED_GROUND",
	"SHORTED_POWER",
	"BAD_CONNECTION",
	"BAD_CONNECTION_OR_GROUND",
	"BAD_CONNECTION_OR_POWER",
	"SHORTED_GROUND_ACI",
	"SHORTED_GROUND_BIAS",
	"SHORTED_POWER_ACI",
	"SHORTED_POWER_BIAS"
};

/*  Digmic selftest defines */
enum Selftest_Digimic_Setup_e {
	ST_DIGIMIC_ALL,
	ST_DIGIMIC_MIC1,
	ST_DIGIMIC_MIC2,
	ST_DIGIMIC_MIC3,
	ST_DIGIMIC_MIC4
};

enum ResultFormat_e {
	ST_RESULT_FORMAT_NORMAL = 0,
	ST_RESULT_FORMAT_DETAILED
};

struct SelftestDataStandard_t {
	enum ResultFormat_e rf;
};

struct SelftestDataDmic_t {
	enum ResultFormat_e rf;
	int mic;
};

/* Selftest User Space Commands */
struct SelftestData_t {
	struct SelftestDataDmic_t dmic;
	struct SelftestDataStandard_t hs;
	struct SelftestDataStandard_t ihf;
};

static struct SelftestData_t SelftestData = {
	.dmic = {
		   .rf = ST_RESULT_FORMAT_DETAILED,
		   },
	.hs = {
		   .rf = ST_RESULT_FORMAT_DETAILED,
		   },
	.ihf = {
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

#ifdef DIGIMIC_SUPPORTED
/* Variable for DigiMic test */
static bool DigiMicInterruptReceived;
#endif

/**********************************************************/
/** Stuff that should be defined in header files - Begin **/
/**********************************************************/
#define ST_PN_DMIC1DQ  PN_GPIO34
#define ST_PN_DMIC1CLK PN_GPIO33

#define DIGMIC_VCC_REGULATOR "hv7ldo_uc"
/**********************************************************/
/** Stuff that should be defined in header files - End **/
/**********************************************************/

/************************/
/* Test Implementations */
/************************/
struct dac_ctrl_t {
	u32 AUDIOTX_TEST_EN;
	u32 AUDIOTX_BB_STI;
	u32 AUDIOTX_EP_DRV_STO;
};

static void st_audio_audiotx_set_dac_ctrl(CHAL_HANDLE audiohandle,
					  struct dac_ctrl_t writedata)
{
	u32 ctrl = 0;

	ctrl = chal_audio_audiotx_get_dac_ctrl(audiohandle);
	aTrace(LOG_AUDIO_DRIVER, "GLUE_SELFTEST::"
			"st_audio_audiotx_set_dac_ctrl():"
		" Pre-AUDIOH_DAC_CTRL: 0x%08X",
		ctrl);

	ctrl &= (~AUDIOH_DAC_CTRL_AUDIOTX_TEST_EN_MASK);
	ctrl &= (~AUDIOH_DAC_CTRL_AUDIOTX_BB_STI_MASK);

	ctrl |=
	    ((writedata.
	      AUDIOTX_TEST_EN)) & 3 << AUDIOH_DAC_CTRL_AUDIOTX_TEST_EN_SHIFT;
	ctrl |=
	    ((writedata.
	      AUDIOTX_BB_STI) & 3) << AUDIOH_DAC_CTRL_AUDIOTX_BB_STI_SHIFT;

	aTrace(LOG_AUDIO_DRIVER,
			"GLUE_SELFTEST::st_audio_audiotx_set_dac_ctrl(): "
	       "AUDIOH_DAC_CTRL: 0x%08X",
	       ctrl);
	chal_audio_audiotx_set_dac_ctrl(audiohandle, ctrl);

	{
		u32 val;
		val = BRCM_READ_REG(KONA_AUDIOH_VA, AUDIOH_DAC_CTRL);
		aTrace(LOG_AUDIO_DRIVER, "GLUE_SELFTEST::std_selftest_hs():"
		       " AUDIOH_DAC_CTRL: 0x%08X",
		       val);
	}
}
static void st_audio_audiotx_get_dac_ctrl(CHAL_HANDLE audiohandle,
					  struct dac_ctrl_t *readdata)
{

	u32 ctrl;

	ctrl = chal_audio_audiotx_get_dac_ctrl(audiohandle);
	readdata->AUDIOTX_TEST_EN = ctrl & 3;
	readdata->AUDIOTX_BB_STI = (ctrl >> 2) & 3;
	readdata->AUDIOTX_EP_DRV_STO = (ctrl >> 4) & 3;

}

enum selftest_store_e {
	SELFTEST_IHF,
	SELFTEST_HS,
	SELFTEST_DMIC,
	SELFTEST_COUNT
};

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
*  Notes:   This function is should be part of CLKMGR block.
*           Will be replaced with CLKMGR_hw_XXX	function once available
*
****************************************************************************/
static void AUDIOH_hw_clkInit(void)
{
	u32 regVal;

	/* Enable write access */
	regVal = (0x00A5A5 << KHUB_CLK_MGR_REG_WR_ACCESS_PASSWORD_SHIFT);
	regVal |= KHUB_CLK_MGR_REG_WR_ACCESS_CLKMGR_ACC_MASK;
	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_WR_ACCESS,
		       regVal);

	/* Set the frequency policy */
	regVal = (0x06 << KHUB_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_SHIFT);
	regVal |= (0x06 << KHUB_CLK_MGR_REG_POLICY_FREQ_POLICY1_FREQ_SHIFT);
	regVal |= (0x06 << KHUB_CLK_MGR_REG_POLICY_FREQ_POLICY2_FREQ_SHIFT);
	regVal |= (0x06 << KHUB_CLK_MGR_REG_POLICY_FREQ_POLICY3_FREQ_SHIFT);
	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_POLICY_FREQ,
		       regVal);

	/* Set the frequency policy */
	regVal = 0x7FFFFFFF;
	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_POLICY0_MASK1,
		       regVal);
	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_POLICY1_MASK1,
		       regVal);
	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_POLICY2_MASK1,
		       regVal);
	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_POLICY3_MASK1,
		       regVal);
	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_POLICY0_MASK2,
		       regVal);
	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_POLICY1_MASK2,
		       regVal);
	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_POLICY2_MASK2,
		       regVal);
	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_POLICY3_MASK2,
		       regVal);

	/* start the frequency policy */
	regVal =
	    (KHUB_CLK_MGR_REG_POLICY_CTL_GO_MASK |
	     KHUB_CLK_MGR_REG_POLICY_CTL_GO_AC_MASK);
	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_POLICY_CTL,
		       regVal);
}

static void AUDIOH_hw_setClk(void)
{
	u32 regVal;

	/* Enable all the AUDIOH clocks, 26M, 156M, 2p4M, 6p5M  */
	regVal = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_CLK_EN_MASK;
	regVal |=
	    KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_HW_SW_GATING_SEL_MASK;
	regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_CLK_EN_MASK;
	regVal |=
	    KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_HW_SW_GATING_SEL_MASK;
	regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_CLK_EN_MASK;
	regVal |=
	    KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_HW_SW_GATING_SEL_MASK;
	regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_CLK_EN_MASK;
	regVal |=
	    KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_HW_SW_GATING_SEL_MASK;
	regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_HYST_VAL_MASK;

	BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA, KHUB_CLK_MGR_REG_AUDIOH_CLKGATE,
		       regVal);

}

static struct dac_ctrl_t Stored_dac_ctrl_Value;
static u8 Stored_DacPower;
static struct pin_config DmicStoredValue[4];
static int regl_hv7ldo_orig_state;
static u8 StoredDMIC0Enable;
static u8 StoredDMIC1Enable;
static u32 StoredClkGate;
static u32 StoredPolicyFreq;
static u32 StoredPolicyMask[8];
static bool TestActive[SELFTEST_COUNT] = { false, false, false };

void st_audio_store_registers(enum selftest_store_e test)
{
	CHAL_HANDLE audiohandle;
	struct regulator *regl_hv7ldo = NULL;
	audiohandle = chal_audio_init(KONA_AUDIOH_VA, KONA_SDT_BASE_VA);

	if (TestActive[SELFTEST_DMIC] ||
	    TestActive[SELFTEST_IHF] || TestActive[SELFTEST_HS]) {
		/* Store clock settings  */
		StoredClkGate =
		    BRCM_READ_REG(KONA_HUB_CLK_BASE_VA,
				  KHUB_CLK_MGR_REG_AUDIOH_CLKGATE);
		StoredPolicyFreq =
		    BRCM_READ_REG(KONA_HUB_CLK_BASE_VA,
				  KHUB_CLK_MGR_REG_POLICY_FREQ);
		StoredPolicyMask[0] =
		    BRCM_READ_REG(KONA_HUB_CLK_BASE_VA,
				  KHUB_CLK_MGR_REG_POLICY0_MASK1);
		StoredPolicyMask[1] =
		    BRCM_READ_REG(KONA_HUB_CLK_BASE_VA,
				  KHUB_CLK_MGR_REG_POLICY1_MASK1);
		StoredPolicyMask[2] =
		    BRCM_READ_REG(KONA_HUB_CLK_BASE_VA,
				  KHUB_CLK_MGR_REG_POLICY2_MASK1);
		StoredPolicyMask[3] =
		    BRCM_READ_REG(KONA_HUB_CLK_BASE_VA,
				  KHUB_CLK_MGR_REG_POLICY3_MASK1);
		StoredPolicyMask[4] =
		    BRCM_READ_REG(KONA_HUB_CLK_BASE_VA,
				  KHUB_CLK_MGR_REG_POLICY0_MASK2);
		StoredPolicyMask[5] =
		    BRCM_READ_REG(KONA_HUB_CLK_BASE_VA,
				  KHUB_CLK_MGR_REG_POLICY1_MASK2);
		StoredPolicyMask[6] =
		    BRCM_READ_REG(KONA_HUB_CLK_BASE_VA,
				  KHUB_CLK_MGR_REG_POLICY2_MASK2);
		StoredPolicyMask[7] =
		    BRCM_READ_REG(KONA_HUB_CLK_BASE_VA,
				  KHUB_CLK_MGR_REG_POLICY3_MASK2);
	}

	TestActive[test] = true;
	switch (test) {
	case SELFTEST_DMIC:
		/* Store GPIO setting registers */
		DmicStoredValue[0].name = PN_DMIC0CLK;
		pinmux_get_pin_config(&DmicStoredValue[0]);
		DmicStoredValue[1].name = PN_DMIC0DQ;
		pinmux_get_pin_config(&DmicStoredValue[1]);
		DmicStoredValue[2].name = ST_PN_DMIC1CLK;
		pinmux_get_pin_config(&DmicStoredValue[2]);
		DmicStoredValue[3].name = ST_PN_DMIC1DQ;
		pinmux_get_pin_config(&DmicStoredValue[3]);
		/* Store PMU register Values */
		regl_hv7ldo = regulator_get(NULL, DIGMIC_VCC_REGULATOR);
		if (IS_ERR(regl_hv7ldo)) {
			printk(KERN_ERR "Cannot get <hv7ldo> regulator");
		} else {
			regl_hv7ldo_orig_state =
			    regulator_is_enabled(regl_hv7ldo);
			regulator_put(regl_hv7ldo);
		}
		AUDIOH_hw_clkInit();
		AUDIOH_hw_setClk();
		StoredDMIC0Enable =
		    chal_audio_vinpath_digi_mic_enable_read(audiohandle);
		StoredDMIC1Enable =
		    chal_audio_nvinpath_digi_mic_enable_read(audiohandle);
		break;
	case SELFTEST_IHF:
		bcmpmu_audio_ihf_selftest_backup(1);
		AUDIOH_hw_clkInit();
		AUDIOH_hw_setClk();
		/*      Store BB register Values */
		/* Register shared with HS */
		st_audio_audiotx_get_dac_ctrl(audiohandle,
					      &Stored_dac_ctrl_Value);
		Stored_DacPower = chal_audio_ihfpath_get_dac_pwr(audiohandle);
		break;
	case SELFTEST_HS:
		bcmpmu_audio_hs_selftest_backup(1);
		AUDIOH_hw_clkInit();
		AUDIOH_hw_setClk();
		/* Store BB register Values */
		/* Register shared with IHF */
		st_audio_audiotx_get_dac_ctrl(audiohandle,
					      &Stored_dac_ctrl_Value);
		break;
	default:
		break;
	}
}

void st_audio_restore_registers(enum selftest_store_e test)
{
	CHAL_HANDLE audiohandle;
	struct regulator *regl_hv7ldo = NULL;
	audiohandle = chal_audio_init(KONA_AUDIOH_VA, KONA_SDT_BASE_VA);

	switch (test) {
	case SELFTEST_DMIC:
		/* Restore GPIO setting registers */
		aTrace(LOG_AUDIO_DRIVER,
				"GLUE_SELFTEST:: 1:pinmux_set_pin_config(%u)",
		       DmicStoredValue[0].name);
		pinmux_set_pin_config(&DmicStoredValue[0]);
		aTrace(LOG_AUDIO_DRIVER,
				"GLUE_SELFTEST:: 2:pinmux_set_pin_config(%u)",
		       DmicStoredValue[1].name);
		pinmux_set_pin_config(&DmicStoredValue[1]);
		aTrace(LOG_AUDIO_DRIVER,
				"GLUE_SELFTEST:: 3:pinmux_set_pin_config(%u)",
		       DmicStoredValue[2].name);
		pinmux_set_pin_config(&DmicStoredValue[2]);
		aTrace(LOG_AUDIO_DRIVER,
				"GLUE_SELFTEST:: 4:pinmux_set_pin_config(%u)",
		       DmicStoredValue[3].name);
		pinmux_set_pin_config(&DmicStoredValue[3]);
		StoredDMIC0Enable =
		    chal_audio_vinpath_digi_mic_enable_read(audiohandle);
		StoredDMIC1Enable =
		    chal_audio_nvinpath_digi_mic_enable_read(audiohandle);
		regl_hv7ldo = regulator_get(NULL, DIGMIC_VCC_REGULATOR);
		if (IS_ERR(regl_hv7ldo)) {
			printk(KERN_ERR "Cannot get <hv7ldo> regulator");
		} else {
			if (regl_hv7ldo_orig_state == 0)
				regulator_disable(regl_hv7ldo);
			regulator_put(regl_hv7ldo);
		}
		break;
	case SELFTEST_IHF:
		bcmpmu_audio_ihf_selftest_backup(0);
		/* Restore BB register values */
		st_audio_audiotx_set_dac_ctrl(audiohandle,
					      Stored_dac_ctrl_Value);
		chal_audio_ihfpath_set_dac_pwr(audiohandle, Stored_DacPower);
		break;
	case SELFTEST_HS:
		bcmpmu_audio_hs_selftest_backup(0);
		/* Restore BB register values */
		st_audio_audiotx_set_dac_ctrl(audiohandle,
					      Stored_dac_ctrl_Value);
		break;
	default:
		break;
	}
	if (!TestActive[SELFTEST_DMIC] &&
	    !TestActive[SELFTEST_IHF] && !TestActive[SELFTEST_HS]) {
		/* Retore clock settings  */
		BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA,
			       KHUB_CLK_MGR_REG_AUDIOH_CLKGATE, StoredClkGate);
		BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA,
			       KHUB_CLK_MGR_REG_POLICY_FREQ, StoredPolicyFreq);
		BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA,
			       KHUB_CLK_MGR_REG_POLICY0_MASK1,
			       StoredPolicyMask[0]);
		BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA,
			       KHUB_CLK_MGR_REG_POLICY1_MASK1,
			       StoredPolicyMask[1]);
		BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA,
			       KHUB_CLK_MGR_REG_POLICY2_MASK1,
			       StoredPolicyMask[2]);
		BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA,
			       KHUB_CLK_MGR_REG_POLICY3_MASK1,
			       StoredPolicyMask[3]);
		BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA,
			       KHUB_CLK_MGR_REG_POLICY0_MASK2,
			       StoredPolicyMask[4]);
		BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA,
			       KHUB_CLK_MGR_REG_POLICY1_MASK2,
			       StoredPolicyMask[5]);
		BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA,
			       KHUB_CLK_MGR_REG_POLICY2_MASK2,
			       StoredPolicyMask[6]);
		BRCM_WRITE_REG(KONA_HUB_CLK_BASE_VA,
			       KHUB_CLK_MGR_REG_POLICY3_MASK2,
			       StoredPolicyMask[7]);
	}
	TestActive[test] = false;
}

/******************/
/*  IHF TEST  */
/******************/
#ifdef IHF_ST_SUPPORTED
static void std_selftest_ihf(struct SelftestUserCmdData_t *cmddata)
{
	u8 ResultArray1[IHF_NUMBER_OF_SUBTESTS1] = { ST_PASS,
		ST_PASS
	};
	u8 ResultArray2[IHF_NUMBER_OF_SUBTESTS2] = { ST_PASS,
		ST_PASS
	};
	u8 ReadValue;		/* Value read from PMU */
	struct dac_ctrl_t Dac_Ctrl;
	CHAL_HANDLE audiohandle;

	aTrace(LOG_AUDIO_DRIVER, "GLUE_SELFTEST::std_selftest_ihf() called.");

	audiohandle = chal_audio_init(KONA_AUDIOH_VA, KONA_SDT_BASE_VA);
	st_audio_store_registers(SELFTEST_IHF);
	st_audio_audiotx_get_dac_ctrl(audiohandle, &Dac_Ctrl);

	/* Subtest 1 + 2 - IHF DAC */
	/* PMU Input test */
	/* 1.   Enable test mode (driving buffer enabled) */
	/* (i_hs_enst[1:0]  =  '11') on PMU */
	bcmpmu_audio_ihf_testmode(PMU_TEST_READ_AND_ENABLE);
	/*2.     Disable Output (i_hs_enst  =  '0') on BB */
	/* AUDIOTX_TEST_EN[1:0]  =  '00' */
	aTrace(LOG_AUDIO_DRIVER,
			"GLUE_SELFTEST::std_selftest_ihf() Disable Output");
	Dac_Ctrl.AUDIOTX_TEST_EN = BB_TEST_DISABLE;
	st_audio_audiotx_set_dac_ctrl(audiohandle, Dac_Ctrl);
	/*2a. Disable BB output drivers (High-Z) */
	aTrace
	    (LOG_AUDIO_DRIVER, "GLUE_SELFTEST::std_selftest_ihf()"
	     "Disable BB output drivers (High-Z)");
	chal_audio_ihfpath_set_dac_pwr(audiohandle, 0);

	aTrace(LOG_AUDIO_DRIVER, "GLUE_SELFTEST::std_selftest_ihf()  Test 1.1");
	/*3.     Set Output (i_hs_ist  =  '1') on PMU    */

	bcmpmu_audio_ihf_selftest_stimulus_input(0x02);
	/*4.     Check result (o_hst_ist[3:0]). */
	mdelay(AUDIO_SETTLING_TIME);
	bcmpmu_audio_ihf_selftest_result(&ReadValue);
	/* a.    Bit High  = > Check passed     */
	/* b.    Bit Low  = > Shorted to Ground */
	CHECKBIT_AND_ASSIGN_ERROR(0, IHF_NUMBER_OF_SUBTESTS1,
				  ReadValue,
				  ResultArray1, ST_SHORTED_GROUND);
	aTrace(LOG_AUDIO_DRIVER,
			"GLUE_SELFTEST::std_selftest_ihf()"
			"Test1 readvalue  =  0x%X",
	       ReadValue);

	aTrace(LOG_AUDIO_DRIVER, "GLUE_SELFTEST::std_selftest_ihf()  Test 1.2");
	/*5.     Set Output (i_hs_ist  =  '0') on PMU */
	bcmpmu_audio_ihf_selftest_stimulus_input(0x00);
	/*6.     Check result (o_hst_ist[3:0]).    */
	mdelay(AUDIO_SETTLING_TIME);
	bcmpmu_audio_ihf_selftest_result(&ReadValue);
	/* a.    Bit Low  = > Check passed        */
	/* b.    Bit High  = > Shorted to Power */
	CHECKBIT_AND_ASSIGN_ERROR(1, IHF_NUMBER_OF_SUBTESTS1, ReadValue,
				  ResultArray1, ST_SHORTED_POWER);
	aTrace(LOG_AUDIO_DRIVER, "GLUE_SELFTEST::std_selftest_ihf()"
			"Test2 readvalue  =  0x%X",
	       ReadValue);

	/* BB output test: */
	/*1.    Enable Output (Test_en[0:1]  =  '11') on BB */
	Dac_Ctrl.AUDIOTX_TEST_EN = BB_TEST_500_OHM;
	st_audio_audiotx_set_dac_ctrl(audiohandle, Dac_Ctrl);
	/*2.     Enable test mode (Short or Open test) */
	/*  (i_IHFselftest_en[1:0]  =  '10') */
	bcmpmu_audio_ihf_testmode(PMU_TEST_READ_AND_DISABLE);
	aTrace(LOG_AUDIO_DRIVER, "GLUE_SELFTEST::std_selftest_ihf()  Test 1.3");
	/*3.     Set Output (i_BB_sti  =  '11') */
	Dac_Ctrl.AUDIOTX_BB_STI = 0x03;
	st_audio_audiotx_set_dac_ctrl(audiohandle, Dac_Ctrl);
	/*3a.   Set (i_IHFsti  =  '0x') */
	bcmpmu_audio_ihf_selftest_stimulus_input(0x00);
	/*4.     Check result (o_IHFsti). */
	mdelay(AUDIO_SETTLING_TIME);
	bcmpmu_audio_ihf_selftest_result(&ReadValue);
	/* a.    Bit High  = > Check passed */
	/* b.    Bit Low  = > Shorted to Ground */
	CHECKBIT_AND_ASSIGN_ERROR(0, IHF_NUMBER_OF_SUBTESTS1,
				  ReadValue, ResultArray1,
				  ST_BAD_CONNECTION_OR_GROUND);

	aTrace(LOG_AUDIO_DRIVER, "GLUE_SELFTEST::std_selftest_ihf()  Test 1.4");
	/*5.    Set Output (i_BB_sti  =  '00') */
	Dac_Ctrl.AUDIOTX_BB_STI = 0x00;
	st_audio_audiotx_set_dac_ctrl(audiohandle, Dac_Ctrl);
	/*5a.   Set (i_IHFsti  =  '0x') */
	bcmpmu_audio_ihf_selftest_stimulus_input(0x00);
	/*6.     Check result (o_IHFsti). */
	mdelay(AUDIO_SETTLING_TIME);
	bcmpmu_audio_ihf_selftest_result(&ReadValue);
	/* a.    Bit Low  = > Check passed */
	/* b.    Bit High  = > Shorted to Supply */
	CHECKBIT_AND_ASSIGN_ERROR(1, IHF_NUMBER_OF_SUBTESTS1, ReadValue,
				  ResultArray1,
				  ST_BAD_CONNECTION_OR_POWER);

	/* Subtest 3 + 4 - IHF Output  */
	/*PMU Output Pins Test: */
	aTrace(LOG_AUDIO_DRIVER, "GLUE_SELFTEST::std_selftest_ihf()  Test 2.1");
	/*1.     Enable by setting i_IHFselftest_en[0] to 1. */
	bcmpmu_audio_ihf_testmode(PMU_TEST_ENABLE_NO_READ);
	/*2.     i_IHFsto[1:0]  =  '0x' */
	bcmpmu_audio_ihf_selftest_stimulus_output(0x00);
	/* Check */
	mdelay(AUDIO_SETTLING_TIME);
	bcmpmu_audio_ihf_selftest_result(&ReadValue);
	/* a.   Check if  o_IHFsto[1:0] == '00' */
	/*   = > No pull-up resistor from the output pins to Supply */
	/* b.   Check if  o_IHFsto[1:0] < >'00' */
	/*   = > Either or both output pins are short to Supply */
	CHECKBIT_AND_ASSIGN_ERROR(1, IHF_NUMBER_OF_SUBTESTS2,
				  ReadValue, ResultArray2,
				  ST_BAD_CONNECTION_OR_POWER);

	aTrace(LOG_AUDIO_DRIVER, "GLUE_SELFTEST::std_selftest_ihf()  Test 2.2");
	/*3.     i_IHFsto[1:0]  =  '1x' */
	bcmpmu_audio_ihf_selftest_stimulus_output(0x02);
	/* Check */
	mdelay(AUDIO_SETTLING_TIME);
	bcmpmu_audio_ihf_selftest_result(&ReadValue);
	/* a.    Check o_IHFsto[1:0] = '00' */
	/*   = > No pull-down resistor from the output pins to ground */
	/* b.    Check o_IHFsto[1:0] < >'00' */
	/*   = > Either or both output pins are short to ground */
	CHECKBIT_AND_ASSIGN_ERROR(1, IHF_NUMBER_OF_SUBTESTS2,
				  ReadValue, ResultArray2,
				  ST_BAD_CONNECTION_OR_GROUND);

	st_audio_restore_registers(SELFTEST_IHF);
	/* Fill out failures status code structure */
	/* Subtest 1 + 2 - IHF DAC */
	{
		int i;
		for (i = 0; i < IHF_NUMBER_OF_SUBTESTS1; i++) {
			cmddata->subtestStatus[i] =
			    ResultArray1[IHF_NUMBER_OF_SUBTESTS1 - 1 - i];
			aTrace
			    (LOG_AUDIO_DRIVER,
			     "GLUE_SELFTEST::std_selftest_ihf()"
			     "ResultArray1[%u]  =  %u",
			     IHF_NUMBER_OF_SUBTESTS1 - 1 - i,
			     ResultArray1[IHF_NUMBER_OF_SUBTESTS1 - 1 - i]);
		}
	}
	/* Subtest 3 + 4 - IHF Output  */
	{
		int i;
		for (i = 0; i < IHF_NUMBER_OF_SUBTESTS2; i++) {
			cmddata->subtestStatus[i + IHF_NUMBER_OF_SUBTESTS1] =
			    ResultArray2[IHF_NUMBER_OF_SUBTESTS2 - 1 - i];
			aTrace
			    (LOG_AUDIO_DRIVER,
			     "GLUE_SELFTEST::std_selftest_ihf()  "
			     "ResultArray2[%u]  =  %u",
			     IHF_NUMBER_OF_SUBTESTS2 - 1 - i,
			     ResultArray2[IHF_NUMBER_OF_SUBTESTS2 - 1 - i]);
		}
	}

	/* Find return code */
	if (ResultArray1[0] || ResultArray1[1] ||
	    ResultArray2[0] || ResultArray2[1]) {
		cmddata->subtestCount =
		    IHF_NUMBER_OF_SUBTESTS1 + IHF_NUMBER_OF_SUBTESTS2;
		cmddata->testStatus = ST_FAIL;
	} else {
		cmddata->subtestCount = 0;
		cmddata->testStatus = ST_PASS;
	}

}
#endif /* IHF_ST_SUPPORTED */

/******************/
/*  HEADSET TEST  */
/******************/
#ifdef HEADSET_ST_SUPPORTED
static void std_selftest_hs(struct SelftestUserCmdData_t *cmddata)
{
	u8 ResultArray[HA_NUMBER_OF_SUBTESTS] = { ST_PASS,
		ST_PASS,
		ST_PASS,
		ST_PASS
	};
	u8 ReadValue;		/* Value read from PMU */
	struct dac_ctrl_t Dac_Ctrl;
	CHAL_HANDLE audiohandle;
	aTrace(LOG_AUDIO_DRIVER,
			"GLUE_SELFTEST::std_selftest_headset() called.");

	audiohandle = chal_audio_init(KONA_AUDIOH_VA, KONA_SDT_BASE_VA);
	st_audio_store_registers(SELFTEST_HS);
	st_audio_audiotx_get_dac_ctrl(audiohandle, &Dac_Ctrl);

	/**************************/
	/* Subtest 1 + 2 + 3 + 4  */
	/**************************/
	aTrace
	    (LOG_AUDIO_DRIVER, "GLUE_SELFTEST::std_selftest_hs()  "
	     "RA[0] = %u,RA[1] = %u,RA[2] = %u,RA[3] = %u",
	     ResultArray[0], ResultArray[1], ResultArray[2], ResultArray[3]);

	/* Setup */

	bcmpmu_hs_set_input_mode(0, PMU_HS_DIFFERENTIAL_DC_COUPLED);
	bcmpmu_hs_set_gain(PMU_AUDIO_HS_BOTH, 0);
	bcmpmu_audio_init();
	bcmpmu_hs_power(true);

	/* PMU Input test */
	/**********/
	/* TEST 1 */
	/**********/
	aTrace(LOG_AUDIO_DRIVER,
			"GLUE_SELFTEST::std_selftest_hs()  INPUT TEST");
	aTrace(LOG_AUDIO_DRIVER, "GLUE_SELFTEST::TEST 1");
	/* 1.   Enable test mode (driving buffer enabled)
	   (i_hs_enst[1:0]  =  '11') on PMU */

	bcmpmu_audio_hs_testmode(PMU_TEST_READ_AND_ENABLE);

	/*2.     Disable Output (i_hs_enst  =  '0') on BB */
	/* AUDIOTX_TEST_EN[1:0]  =  '00' */
	Dac_Ctrl.AUDIOTX_TEST_EN = BB_TEST_10_KOHM;
	Dac_Ctrl.AUDIOTX_BB_STI = 0x00;
	st_audio_audiotx_set_dac_ctrl(audiohandle, Dac_Ctrl);

	/*3.     Set Output (i_hs_ist  =  '1') on PMU    */
	bcmpmu_audio_hs_selftest_stimulus(0x01);

	/*4.     Check result (o_hst_ist[3:0]). */
	mdelay(AUDIO_SETTLING_TIME);
	bcmpmu_audio_hs_selftest_result(&ReadValue);

	/* a.    Bit High  = > Check passed     */
	/* b.    Bit Low  = > Shorted to Ground */
	CHECKBIT_AND_ASSIGN_ERROR(0, HA_NUMBER_OF_SUBTESTS,
				  ReadValue, ResultArray,
				  ST_SHORTED_GROUND);
	aTrace(LOG_AUDIO_DRIVER, "GLUE_SELFTEST::std_selftest_hs()"
			"Test1 readvalue  =  0x%X",
	       ReadValue);
	aTrace
	    (LOG_AUDIO_DRIVER, "GLUE_SELFTEST::std_selftest_hs()  "
	     "RA[0] = %u,RA[1] = %u,RA[2] = %u,RA[3] = %u",
	     ResultArray[0], ResultArray[1], ResultArray[2], ResultArray[3]);

	/**********/
	/* TEST 2 */
	/**********/
	aTrace(LOG_AUDIO_DRIVER, "GLUE_SELFTEST::TEST 2");
	BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA, AUDIOH_DAC_CTRL, AUDIOTX_BB_STI,
			     0x03);

	/*5.     Set Output (i_hs_ist  =  '0') on PMU */
	bcmpmu_audio_hs_selftest_stimulus(0x00);
	/*6.     Check result (o_hst_ist[3:0]).    */
	mdelay(AUDIO_SETTLING_TIME);
	bcmpmu_audio_hs_selftest_result(&ReadValue);
	/* a.    Bit Low  = > Check passed      */
	/* b.    Bit High  = > Shorted to Power */
	CHECKBIT_AND_ASSIGN_ERROR(1, HA_NUMBER_OF_SUBTESTS,
				  ReadValue,
				  ResultArray, ST_SHORTED_POWER);
	aTrace(LOG_AUDIO_DRIVER,
			"GLUE_SELFTEST::std_selftest_hs()"
			"Test2 readvalue  =  0x%X",
	       ReadValue);
	aTrace
	    (LOG_AUDIO_DRIVER, "GLUE_SELFTEST::std_selftest_hs()  "
	     "RA[0] = %u,RA[1] = %u,RA[2] = %u,RA[3] = %u",
	     ResultArray[0], ResultArray[1], ResultArray[2], ResultArray[3]);

	/* BB Output Test */
	/**********/
	/* TEST 3 */
	/**********/
	aTrace(LOG_AUDIO_DRIVER,
			"GLUE_SELFTEST::std_selftest_hs()  OUTPUT TEST");
	aTrace(LOG_AUDIO_DRIVER, "GLUE_SELFTEST::TEST 3");
	/*1.     Enable test mode (driving buffer disabled)
	  (i_hs_enst[1:0]  =  '10') on PMU */
	bcmpmu_audio_hs_testmode(PMU_TEST_READ_AND_DISABLE);
	/*2.     Enable Output (i_hs_enst  =  '1') on BB */
	/* AUDIOTX_TEST_EN[1:0]  =  '11' */
	Dac_Ctrl.AUDIOTX_TEST_EN = BB_TEST_500_OHM;
	/*3.     Set Output (i_hs_ist  =  '1') on BB */
	/* AUDIOTX_BB_STI[1:0]  =  '11' */
	Dac_Ctrl.AUDIOTX_BB_STI = 0x03;
	st_audio_audiotx_set_dac_ctrl(audiohandle, Dac_Ctrl);

	/*4.     Check result (o_hst_ist[3:0]).   */
	mdelay(AUDIO_SETTLING_TIME);
	bcmpmu_audio_hs_selftest_result(&ReadValue);
	/* a.    Bit High  = > Check passed                               */
	/* b.    Bit Low  = > Shorted to Ground or Not connected */
	CHECKBIT_AND_ASSIGN_ERROR(0, HA_NUMBER_OF_SUBTESTS,
				  ReadValue, ResultArray,
				  ST_BAD_CONNECTION_OR_GROUND);
	aTrace(LOG_AUDIO_DRIVER,
			"GLUE_SELFTEST::std_selftest_hs()"
			"Test3 readvalue  =  0x%X",
	       ReadValue);
	aTrace
	    (LOG_AUDIO_DRIVER, "GLUE_SELFTEST::std_selftest_hs()  "
	     "RA[0] = %u,RA[1] = %u,RA[2] = %u,RA[3] = %u",
	     ResultArray[0], ResultArray[1], ResultArray[2], ResultArray[3]);

	/**********/
	/* TEST 4 */
	/**********/
	aTrace(LOG_AUDIO_DRIVER, "GLUE_SELFTEST::TEST 4");
	/*5.     Set Output (i_hs_ist  =  '0') on BB */
	/* AUDIOTX_BB_STI[1:0]  =  '00' */
	Dac_Ctrl.AUDIOTX_BB_STI = 0x00;
	st_audio_audiotx_set_dac_ctrl(audiohandle, Dac_Ctrl);

	/*6.     Check result (o_hst_ist[3:0]).   */
	mdelay(AUDIO_SETTLING_TIME);
	bcmpmu_audio_hs_selftest_result(&ReadValue);

	/* a.    Bit Low  = > Check passed */
	/* b.    Bit High  = > Shorted to Power or Not connected   */
	CHECKBIT_AND_ASSIGN_ERROR(1, HA_NUMBER_OF_SUBTESTS,
				  ReadValue,
				  ResultArray,
				  ST_BAD_CONNECTION_OR_POWER);
	aTrace(LOG_AUDIO_DRIVER,
			"GLUE_SELFTEST::std_selftest_hs()"
			"Test4 readvalue  =  0x%X",
	       ReadValue);
	aTrace
	    (LOG_AUDIO_DRIVER, "GLUE_SELFTEST::std_selftest_hs()  "
	     "RA[0] = %u,RA[1] = %u,RA[2] = %u,RA[3] = %u",
	     ResultArray[0], ResultArray[1], ResultArray[2], ResultArray[3]);

	st_audio_restore_registers(SELFTEST_HS);

	/* Fill out failures status code structure */
	{
		int i;
		for (i = 0; i < HA_NUMBER_OF_SUBTESTS; i++) {
			cmddata->subtestStatus[i] =
			    ResultArray[HA_NUMBER_OF_SUBTESTS - 1 - i];
			aTrace
			    (LOG_AUDIO_DRIVER,
			     "GLUE_SELFTEST::std_selftest_headset()  "
			     "ResultArray[%u]  =  %u",
			     HA_NUMBER_OF_SUBTESTS - 1 - i,
			     ResultArray[HA_NUMBER_OF_SUBTESTS - 1 - i]);
		}
	}

	/* Find return code */
	if (ResultArray[0] || ResultArray[1] ||
	    ResultArray[2] || ResultArray[3]) {
		cmddata->subtestCount = HA_NUMBER_OF_SUBTESTS;
		cmddata->testStatus = ST_FAIL;
	} else {
		cmddata->subtestCount = 0;
		cmddata->testStatus = ST_PASS;
	}
}
#endif

/******************/
/*  DIGIMIC TEST  */
/******************/

#ifdef DIGIMIC_SUPPORTED
/* Clock control functions */
static void ST_AUDIOH_hw_DMIC_Enable(CHAL_HANDLE audiohandle, int dmic)
{
	/* Enable DMIC paths */
	aTrace
	    (LOG_AUDIO_DRIVER, "GLUE_SELFTEST::AUDIOH_hw_DMIC_Enable()  "
	     "Enable interface DMIC%u(RDB)",
	     dmic);
	switch (dmic) {
	case 0:
		chal_audio_vinpath_digi_mic_enable(audiohandle,
						   CHAL_AUDIO_CHANNEL_LEFT);
		break;
	case 1:
		chal_audio_vinpath_digi_mic_enable(audiohandle,
						   CHAL_AUDIO_CHANNEL_RIGHT);
		break;
	case 2:
		chal_audio_nvinpath_digi_mic_enable(audiohandle,
						    CHAL_AUDIO_CHANNEL_LEFT);
		break;
	case 3:
		chal_audio_nvinpath_digi_mic_enable(audiohandle,
						    CHAL_AUDIO_CHANNEL_RIGHT);
		break;
	}
	/* Setup Clock */
}

static void ST_AUDIOH_hw_DMIC_Disable(CHAL_HANDLE audiohandle)
{
	u32 regVal;

	/* Disable all DMIC paths */
	regVal = BRCM_READ_REG(KONA_AUDIOH_VA, AUDIOH_ADC_CTL);

	regVal &=
	    ~(AUDIOH_ADC_CTL_DMIC1_EN_MASK | AUDIOH_ADC_CTL_DMIC2_EN_MASK |
	      AUDIOH_ADC_CTL_DMIC3_EN_MASK | AUDIOH_ADC_CTL_DMIC4_EN_MASK);

	BRCM_WRITE_REG(KONA_AUDIOH_VA, AUDIOH_ADC_CTL, regVal);
}

static irqreturn_t GPIO_DigiMicSelftestEventFunction(int irq, void *dev_id)
{
	/* Receive GPIO event */
	DigiMicInterruptReceived = true;

	aTrace(LOG_AUDIO_DRIVER,
			"GLUE_SELFTEST::GPIO_DigiMicSelftestEventFunction(%u) "
			, irq);

	disable_irq_nosync(irq);

	return IRQ_HANDLED;
}

#define DIGIMIC_SUBTESTS_PER_MIC 1
#define MAX_DIGIMIC_IF_COUNT 2
#define MAX_DIGIMIC_COUNT 4
#define DIGIMIC_NUMBER_OF_SUBTESTS 4
#define DIGIMIC_COMM_TEST 0	/* First digimic test */
static void std_selftest_dmic(struct SelftestUserCmdData_t *cmddata)
{
	int Mic, MicIf;
	int ret;
	int i;
	struct regulator *regl_hv7ldo = NULL;
	bool TestMic[MAX_DIGIMIC_COUNT] = { false, false, false, false };
	enum PIN_NAME PMUX_CLK_CONNECTION[MAX_DIGIMIC_IF_COUNT] = {
		PN_DMIC0CLK, ST_PN_DMIC1CLK
		};
	enum PIN_NAME PMUX_DQ_CONNECTION[MAX_DIGIMIC_IF_COUNT] = {
		PN_DMIC0DQ, ST_PN_DMIC1DQ
		};
	enum PIN_FUNC PMUX_DMIC_CLK_MODE_CLK[MAX_DIGIMIC_IF_COUNT] = {
		PF_DMIC0CLK, PF_DMIC1CLK
		};
	enum PIN_FUNC PMUX_DMIC_DQ_MODE_GPIO[MAX_DIGIMIC_IF_COUNT];
	unsigned DQ_CONNECTION[MAX_DIGIMIC_IF_COUNT];
	int MIC_IF[MAX_DIGIMIC_COUNT] = { 0, 0, 1, 1 };
	struct pin_config PIN_DMIC_Setup;
	struct pin_config PIN_GPIO_Setup;
	CHAL_HANDLE audiohandle;
	unsigned find_gpio;
	enum PIN_FUNC find_PF_gpio;
	u8 Status[DIGIMIC_NUMBER_OF_SUBTESTS] = { ST_PASS,
		ST_PASS
	};

	st_audio_store_registers(SELFTEST_DMIC);

	for (i = 0; i < MAX_DIGIMIC_IF_COUNT; i++) {
		pinmux_find_gpio(PMUX_DQ_CONNECTION[i], &find_gpio,
				 &find_PF_gpio);
		aTrace
		    (LOG_AUDIO_DRIVER, "GLUE_SELFTEST::std_selftest_digimic() "
		     "PMUX_DQ_CONNECTION[%u]=%u, gpio=%u, gpio_func=%u ",
		     i, PMUX_DQ_CONNECTION[i], find_gpio, find_PF_gpio);
		DQ_CONNECTION[i] = find_gpio;
		PMUX_DMIC_DQ_MODE_GPIO[i] = find_PF_gpio;
	}

	switch (cmddata->parm1) {
	default:
	case ST_DIGIMIC_ALL:
		TestMic[0] = true;
		TestMic[1] = true;
		TestMic[2] = true;
		TestMic[3] = true;
		break;
	case ST_DIGIMIC_MIC1:
		TestMic[0] = true;
		break;
	case ST_DIGIMIC_MIC2:
		TestMic[1] = true;
		break;
	case ST_DIGIMIC_MIC3:
		TestMic[2] = true;
		break;
	case ST_DIGIMIC_MIC4:
		TestMic[3] = true;
		break;
	}

	audiohandle = chal_audio_init(KONA_AUDIOH_VA, KONA_SDT_BASE_VA);

	/* Actual test */
	for (Mic = 0; Mic < MAX_DIGIMIC_COUNT; Mic++) {

		if (!TestMic[Mic]) {
			Status[Mic * DIGIMIC_SUBTESTS_PER_MIC] =
			    ST_NOT_TESTED;
			aTrace
			    (LOG_AUDIO_DRIVER,
			     "GLUE_SELFTEST::std_selftest_digimic()"
			     "Mic#%u Not Tested",
			     Mic);
			continue;
		}
		MicIf = MIC_IF[Mic];

		aTrace(LOG_AUDIO_DRIVER,
				"GLUE_SELFTEST::std_selftest_digimic()"
				"Mic#%u", Mic);

		/* Subtest 3(6) - Connection test */
		aTrace
		    (LOG_AUDIO_DRIVER, "GLUE_SELFTEST::std_selftest_digimic()"
		     "Connection Test (%u) DQ = %u ",
		     Mic, DQ_CONNECTION[MicIf]);
		/*0. Power on microphone HVLDO7 (on PMU HVLDO7PMODCTRL) */
		/* powerOnDigitalMic(true); */
		regl_hv7ldo = regulator_get(NULL, DIGMIC_VCC_REGULATOR);
		if (IS_ERR(regl_hv7ldo)) {
			aError("%s: regulator_get failed(0x%X)\n",
			       __func__, (unsigned int)regl_hv7ldo);
			aError("Cannot get <hv7ldo> regulator");
		} else {
			regulator_enable(regl_hv7ldo);
			regulator_put(regl_hv7ldo);
		}
		/*1.     Setup DMIC0CLK as Mic clock output */
		PIN_GPIO_Setup.name = PMUX_DQ_CONNECTION[MicIf];
		PIN_GPIO_Setup.func = PMUX_DMIC_DQ_MODE_GPIO[MicIf];
		PIN_GPIO_Setup.reg.val = 0;
		PIN_GPIO_Setup.reg.b.drv_sth = DRIVE_STRENGTH_8MA;
		pinmux_set_pin_config(&PIN_GPIO_Setup);
		aTrace(LOG_AUDIO_DRIVER,
				"GLUE_SELFTEST:: DQ Setup 0x%X",
				PIN_GPIO_Setup.reg.val);

		PIN_DMIC_Setup.name = PMUX_CLK_CONNECTION[MicIf];
		PIN_DMIC_Setup.func = PMUX_DMIC_CLK_MODE_CLK[MicIf];
		PIN_DMIC_Setup.reg.val = 0;
		PIN_DMIC_Setup.reg.b.drv_sth = DRIVE_STRENGTH_8MA;
		pinmux_set_pin_config(&PIN_DMIC_Setup);
		aTrace(LOG_AUDIO_DRIVER, "GLUE_SELFTEST:: CLK Setup 0x%X",
		       PIN_DMIC_Setup.reg.val);

		/* Enable Audio Clocks */
		ST_AUDIOH_hw_DMIC_Enable(audiohandle, Mic);

		mdelay(DMIC_SETTLING_TIME);

		/*2.     Setup DMICXDQ  input  */
		/*3.     Setup DMICXDQ to interrupt on rising edge */

		ret = gpio_request(DQ_CONNECTION[MicIf], "DMICDQ");
		if (ret < 0) {
			aTrace(LOG_AUDIO_DRIVER,
					"GLUE_SELFTEST::gpio %u request failed",
			       DQ_CONNECTION[MicIf]);
			cmddata->testStatus = ST_FAIL;
			return;
		}
		DigiMicInterruptReceived = false;
		aTrace
		    (LOG_AUDIO_DRIVER, "GLUE_SELFTEST::std_selftest_digimic()  "
		     "Set interrupt Mode(Input/Rising)");
		gpio_direction_input(DQ_CONNECTION[MicIf]);
		ret = request_irq(gpio_to_irq(DQ_CONNECTION[MicIf]),
				  GPIO_DigiMicSelftestEventFunction,
				  IRQF_TRIGGER_RISING, "Digmimic Data", 0);
		if (ret < 0) {
			aTrace(LOG_AUDIO_DRIVER,
				"GLUE_SELFTEST::gpio irq %u request failed",
			       DQ_CONNECTION[MicIf]);
			cmddata->testStatus = ST_FAIL;
			gpio_free(DQ_CONNECTION[MicIf]);
			return;
		}
		/*4.     Wait for interrupt for 10ms                      */
		for (i = 0; i < 10; i++) {
			aTrace
			    (LOG_AUDIO_DRIVER,
			     "GLUE_SELFTEST::std_selftest_digimic()  "
			     "Interrrupt wait loop %u, Value = %u",
			     i, gpio_get_value(DQ_CONNECTION[MicIf]));
			mdelay(2);
			if (DigiMicInterruptReceived == true)
				break;
		}
		aTrace
		    (LOG_AUDIO_DRIVER,
		     "GLUE_SELFTEST::std_selftest_digimic()  "
		     "Disable Interrupt");
		disable_irq(gpio_to_irq(DQ_CONNECTION[MicIf]));
		free_irq(gpio_to_irq(DQ_CONNECTION[MicIf]), 0);
		gpio_free(DQ_CONNECTION[MicIf]);
		if (DigiMicInterruptReceived == true) {
			/* b.    Interrupt received  = > Continue to 5.  */
			aTrace
			    (LOG_AUDIO_DRIVER,
			     "GLUE_SELFTEST::std_selftest_digimic() "
			     "Interrupt Received");
			Status[(Mic * DIGIMIC_SUBTESTS_PER_MIC) +
			       DIGIMIC_COMM_TEST] = ST_PASS;
		} else {
			Status[(Mic * DIGIMIC_SUBTESTS_PER_MIC) +
			       DIGIMIC_COMM_TEST] = ST_BAD_CONNECTION;
			/* a.    Timeout  = > No connection - Test failed */
		}
		aTrace
		    (LOG_AUDIO_DRIVER,
		     "GLUE_SELFTEST::std_selftest_digimic()  "
		     "3.1 Status[%u]  =  %u ",
		     (Mic * DIGIMIC_SUBTESTS_PER_MIC) + DIGIMIC_COMM_TEST,
		     Status[(Mic * DIGIMIC_SUBTESTS_PER_MIC) +
			    DIGIMIC_COMM_TEST]);

		if (Status[(Mic * DIGIMIC_SUBTESTS_PER_MIC) + DIGIMIC_COMM_TEST]
		    == ST_PASS) {
			/*5.     Setup DMICXDQ to interrupt on falling edge */
			DigiMicInterruptReceived = false;
			ret = gpio_request(DQ_CONNECTION[MicIf], "DMICDQ");
			if (ret < 0) {
				aTrace(LOG_AUDIO_DRIVER,
						"GLUE_SELFTEST::gpio %u request failed",
				       DQ_CONNECTION[MicIf]);
				cmddata->testStatus = ST_FAIL;
				return;
			}
			gpio_direction_input(DQ_CONNECTION[MicIf]);
			DigiMicInterruptReceived = false;
			aTrace
			    (LOG_AUDIO_DRIVER ,
			     "GLUE_SELFTEST::std_selftest_digimic()  "
			     "Set interrupt Mode(Input/Falling)");
			ret =
			    request_irq(gpio_to_irq(DQ_CONNECTION[MicIf]),
					GPIO_DigiMicSelftestEventFunction,
					IRQF_TRIGGER_FALLING, "Digmimic Data",
					0);
			if (ret < 0) {
				aTrace(LOG_AUDIO_DRIVER, "GLUE_SELFTEST::"
				       "gpio irq %u request failed",
				     DQ_CONNECTION[MicIf]);
				cmddata->testStatus = ST_FAIL;
				gpio_free(DQ_CONNECTION[MicIf]);
				return;
			}
			/*6.     Wait for interrupt for 10ms  */
			for (i = 0; i < 10; i++) {
				aTrace
				    (LOG_AUDIO_DRIVER,
				     "GLUE_SELFTEST::std_selftest_digimic()  "
				     "Interrrupt wait loop %u, Value = %u",
				     i, gpio_get_value(DQ_CONNECTION[MicIf]));
				mdelay(2);
				if (DigiMicInterruptReceived == true)
					break;
			}
			aTrace
			    (LOG_AUDIO_DRIVER,
			     "GLUE_SELFTEST::std_selftest_digimic()  "
			     "Disable Interrupt");
			disable_irq(gpio_to_irq(DQ_CONNECTION[MicIf]));
			free_irq(gpio_to_irq(DQ_CONNECTION[MicIf]), 0);
			gpio_free(DQ_CONNECTION[MicIf]);
			if (DigiMicInterruptReceived == true) {
				/* b.    Interrupt received
				   = > ST_PASS */
				aTrace
				    (LOG_AUDIO_DRIVER,
				     "GLUE_SELFTEST::std_selftest_digimic() "
				     "Interrupt Received");
				Status[(Mic * DIGIMIC_SUBTESTS_PER_MIC) +
				       DIGIMIC_COMM_TEST] = ST_PASS;
			} else {
				/* a.    Timeout
				   = > No connection - Test failed */
				Status[(Mic * DIGIMIC_SUBTESTS_PER_MIC) +
				       DIGIMIC_COMM_TEST] =
				    ST_BAD_CONNECTION;
			}
			aTrace(LOG_AUDIO_DRIVER,
					"GLUE_SELFTEST::std_selftest_digimic()"
			     "3.2 Status[%u]  =  %u",
			     (Mic * DIGIMIC_SUBTESTS_PER_MIC) +
			     DIGIMIC_COMM_TEST,
			     Status[(Mic * DIGIMIC_SUBTESTS_PER_MIC) +
				    DIGIMIC_COMM_TEST]);
		}
		ST_AUDIOH_hw_DMIC_Disable(audiohandle);
	}			/* Mic */

	/* Restore register values */

	st_audio_restore_registers(SELFTEST_DMIC);

	/* Fill out failures status code structure */
	for (i = 0; i < DIGIMIC_NUMBER_OF_SUBTESTS; i++) {
		cmddata->subtestStatus[i] = Status[i];
		aTrace
		    (LOG_AUDIO_DRIVER,
		     "GLUE_SELFTEST::std_selftest_digimic()  Status[%u] = %u",
		     i, Status[i]);
	}
	/* Find return code */
	if (((Status[0] != 0) && (Status[0] != ST_NOT_TESTED)) ||
	    ((Status[1] != 0) && (Status[1] != ST_NOT_TESTED)) ||
	    ((Status[2] != 0) && (Status[2] != ST_NOT_TESTED)) ||
	    ((Status[3] != 0) && (Status[3] != ST_NOT_TESTED))) {
		cmddata->subtestCount = DIGIMIC_NUMBER_OF_SUBTESTS;
		cmddata->testStatus = ST_FAIL;
	} else {
		cmddata->subtestCount = 0;
		cmddata->testStatus = ST_PASS;
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
	    (cmddata.testStatus == ST_FAIL) &&
	    (cmddata.subtestCount != 0)) {
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

/*******************/
/* Sysfs Interface */
/*******************/

#ifdef DIGIMIC_SUPPORTED
static ssize_t show_selftest_dmic(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct SelftestUserCmdData_t cmddata;

	cmddata.ResultFormat = SelftestData.dmic.rf;
	cmddata.parm1 = SelftestData.dmic.mic;

	if (!TestActive[SELFTEST_DMIC])
		std_selftest_dmic(&cmddata);
	else
		cmddata.testStatus = ST_BUSY;

	return FormatTestResult(cmddata, buf);
}
static ssize_t store_selftest_dmic(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	int format, mic;
	int parms;

	parms = sscanf(buf, "%d %d", &format, &mic);
	if ((parms == 0) || (parms > 2))
		return -EINVAL;
	if (parms >= 1)
		SelftestData.dmic.rf = format;
	if (parms >= 2)
		SelftestData.dmic.mic = mic;
	return strnlen(buf, count);
}
#endif
#ifdef IHF_ST_SUPPORTED
static ssize_t show_selftest_ihf(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct SelftestUserCmdData_t cmddata;

	cmddata.ResultFormat = SelftestData.ihf.rf;

	if (!TestActive[SELFTEST_IHF])
		if (TestActive[SELFTEST_HS])
			cmddata.testStatus = ST_BUSY;
		else
			std_selftest_ihf(&cmddata);
	else
		cmddata.testStatus = ST_BUSY;

	return FormatTestResult(cmddata, buf);
}
static ssize_t store_selftest_ihf(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	int format;

	if ((sscanf(buf, "%d", &format)) != 1)
		return -EINVAL;

	SelftestData.ihf.rf = format;

	return strnlen(buf, count);
}
#endif
#ifdef HEADSET_ST_SUPPORTED
static ssize_t show_selftest_hs(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct SelftestUserCmdData_t cmddata;

	cmddata.ResultFormat = SelftestData.hs.rf;

	if (!TestActive[SELFTEST_HS])
		if (TestActive[SELFTEST_IHF])
			cmddata.testStatus = ST_BUSY;
		else
			std_selftest_hs(&cmddata);
	else
		cmddata.testStatus = ST_BUSY;

	return FormatTestResult(cmddata, buf);
}
static ssize_t store_selftest_hs(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	int format;

	if ((sscanf(buf, "%d", &format)) != 1)
		return -EINVAL;

	SelftestData.hs.rf = format;

	return strnlen(buf, count);
}
#endif

#ifdef DIGIMIC_SUPPORTED
static DEVICE_ATTR(selftest_dmic, S_IRUGO | S_IWUSR, show_selftest_dmic,
		   store_selftest_dmic);
#endif
#ifdef IHF_ST_SUPPORTED
static DEVICE_ATTR(selftest_ihf, S_IRUGO | S_IWUSR, show_selftest_ihf,
		   store_selftest_ihf);
#endif
#ifdef HEADSET_ST_SUPPORTED
static DEVICE_ATTR(selftest_hs, S_IRUGO | S_IWUSR, show_selftest_hs,
		   store_selftest_hs);
#endif

int BrcmCreateAuddrv_selftestSysFs(struct snd_card *card)
{
	int ret = 0;
	/*create sysfs file for Aud Driver selftest control */
#ifdef DIGIMIC_SUPPORTED
	ret = snd_add_device_sysfs_file(SNDRV_DEVICE_TYPE_CONTROL, card, -1,
					&dev_attr_selftest_dmic);
#endif
#ifdef IHF_ST_SUPPORTED
	ret = snd_add_device_sysfs_file(SNDRV_DEVICE_TYPE_CONTROL, card, -1,
					&dev_attr_selftest_ihf);
#endif
#ifdef HEADSET_ST_SUPPORTED
	ret = snd_add_device_sysfs_file(SNDRV_DEVICE_TYPE_CONTROL, card, -1,
					&dev_attr_selftest_hs);
#endif
	return ret;
}

/************************************/
/* Selftest Boot Excution interface */
/************************************/
int BrcmAudioSelftest(void)
{
	struct SelftestUserCmdData_t cmddata;

	std_selftest_ihf(&cmddata);

	if (cmddata.testStatus == ST_PASS) {

		std_selftest_hs(&cmddata);
		if (cmddata.testStatus == ST_PASS)
			return 0;
	}
	return -EIO;
}
