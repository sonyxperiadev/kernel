/*****************************************************************************
*  Copyright 2001 - 2010 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/
/*============================================================================
*!
*! \file   chal_aci.c
*! \brief  ACI cHAL layer
*! \note
*!
*============================================================================
*/

/* ---- Include Files ------------------------------------------------------- */

#include <plat/chal/chal_types.h>
#include <plat/chal/chal_common.h>
#include <plat/chal/chal_aci.h>

#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_util.h>
#include <mach/rdb/brcm_rdb_aci.h>
#include <mach/rdb/brcm_rdb_auxmic.h>
#include <mach/rdb/brcm_rdb_audioh.h>

#include <stdarg.h>

#include <mach/hardware.h>
#include <mach/memory.h>
#include <linux/clk.h>
#include <linux/err.h>
/*==========================================================================
* Local Typedefs
*==========================================================================
*/

/*==========================================================================
* Local Definitions and Declarations
*==========================================================================
*/

/*
 * CONFIG_AUDIOH_REGS
 * Added the above macro to protect all AUDIOH register configuration.
 * Not sure why the MIC BIAS configuration of ACI should go and touch AUDIOH
 * registers as well. We tested Accessory detection, button press & release
 * without accessing AUDIOH registes and it looks good.
 * Until we understand the resason to do this, keeping AUDIOH accesses
 * protected  under undefined macro.
 */

#define ADC_FS_BIG_SMALL_B_LOW_RANGE 0	/*Sets ADC low scale: 0 = 1150 mV */
#define ADC_FS_BIG_SMALL_B_FULL_RANGE 1	/*Sets ADC full scale: 1= 2300 mV */

#define ACI_TX_INV_SETTING  ((~ACI_ACI_TX_INV_ACI_TX_INV_CMD_INVETED) & \
						ACI_ACI_TX_INV_ACI_TX_INV_MASK)

#define AUDIORX_VREF_PWRUP_POWERUP                              1
#define AUDIORX_VREF_PWRUP_POWERDOWN                            0

#define AUDIORX_VREF_FASTSETTLE_FAST                            1
#define AUDIORX_VREF_FASTSETTLE_NORMAL                          0

#define AUDIORX_VREF_POWERCYCLE_FAST                            1
#define AUDIORX_VREF_POWERCYCLE_NORMAL                          0

#define AUDIORX_BIAS_PWRUP_POWERUP                              1
#define AUDIORX_BIAS_PWRUP_POWERDOWN                            0

#define AUDIORX_VAUXMIC_2_10V                                   0
#define AUDIORX_VAUXMIC_2_19V                                   1
#define AUDIORX_VAUXMIC_2_29V                                   2
#define AUDIORX_VAUXMIC_2_40V                                   3

#define COMP2_TO_P_MIC_OUT                                      1
#define COMP2_TO_ACI_PULL                                       0

#define SW_MIC_DATAB_ACI_DATA                                   0
#define SW_MIC_DATAB_P_MIC_OUT                                  1

#define SW_ACI_OFF                                              0
#define SW_ACI_ON                                               1

#define ACI_BIAS_PWRDN_POWERUP                                  0
#define ACI_BIAS_PWRDN_POWERDOWN                                1

/*
 * Write ACI Command Register
 */
#define CHAL_ACI_WRITE_COMR(f, d)    do { \
cUInt16  reg; \
reg = (cUInt16)(BRCM_READ_REG(KONA_ACI_VA, ACI_COMR) & ~ACI_COMR_DBICMD_MASK); \
reg &= ~BRCM_FIELDMASK(ACI_COMR, f); \
reg |= (((BRCM_REGTYPE(ACI_COMR))d) << BRCM_FIELDSHIFT(ACI_COMR, f)) & \
					BRCM_FIELDMASK(ACI_COMR, f); \
BRCM_WRITE_REG(KONA_ACI_VA, ACI_COMR, reg); \
} while (0)

/*
 * ACI Block Register write delay
 */
#define HW_ACI_REGISTER_WRITE_SETTLE_DELAY  1	/* 3 cycles @ 32768 Hz = 92us */

/* The loop below will take around 200us */
#define HW_ACI_REGISTER_WRITE_SETTLE        do { \
cUInt32 i;  \
volatile cUInt32 j = 0;  \
	for (i = 0; i < 1000; i++)  {\
		j += BRCM_READ_REG_FIELD(KONA_ACI_VA, \
			ACI_COMP_DOUT, COMP1_DOUT); }  \
} while (0)

static void chal_aci_block_ctrl_arg(CHAL_HANDLE handle,
				    CHAL_ACI_block_action_t action,
				    CHAL_ACI_block_id_t id, va_list argp);
static cVoid chal_aci_init_aci(cUInt32 baseAddr);

/*=============================================================================
* Local Variables
*=============================================================================
*/
static volatile cUInt8 *base_addr;
static struct clk *audioh_apb_clk;

/*==========================================================================
* Function Declarations
*==========================================================================
*/

/**
* Function Name: CHAL_HANDLE chal_aci_init(cUInt32 baseAddr)
*
* Description:   Standard Init entry point for ACI Accessory driver which should
*                be the first function to call.
*
*Parameters :	baseAddr, mapped address of
*				this ACI Accessory instance. Not Used
*
* Return:
*             CHAL handle for this ACI Accessory instance
*
*/
CHAL_HANDLE chal_aci_init(void __iomem *baseAddr_NotUsed)
{
	base_addr = (cUInt8 *)KONA_ACI_VA;
	chal_aci_init_aci((cUInt32)base_addr);
	audioh_apb_clk = clk_get(NULL, "audioh_apb_clk");
	if (IS_ERR(audioh_apb_clk)) {
		pr_err("%s(): clk_get of audioh_apb_clk failed \r\n", __func__);
		audioh_apb_clk = NULL;
	}
	return (CHAL_HANDLE)base_addr;
}

/**
* Description:  De-Initialize CHAL ACI for the passed ACI Accessory instance
*
* Parameters:  handle (in) this ACI Accessory instance
*
* return none
*/
cVoid chal_aci_deinit(CHAL_HANDLE handle)
{
}

/**
*   Control MIC line route
*
*   @param  route   Specifies if MIC line route
*
*   @return Void
*
*   @todo   <todo list>
*/
cVoid chal_aci_set_mic_route(CHAL_HANDLE handle, CHAL_ACI_mic_route_t route)
{
	/* Configure the MIC line route */

	switch (route) {
	case CHAL_ACI_MIC_ROUTE_TV:
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ECIDATA, ECIDATA,
				     ACI_ECIDATA_ECIDATA_CMD_OFF);
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ACI_CTRL, SW_MIC_DATAB,
				     SW_MIC_DATAB_P_MIC_OUT);
		/* P_MIC_DATA_IN connected to P_MIC_OUT */
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ADCIN_SEL,
				     ADCIN_SEL, COMP2_TO_P_MIC_OUT);
		/* COMP2 connected to P_MIC_OUT */
			/* ACI Pull-up OFF */
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
			ACI_ACI_CTRL, SW_ACI, SW_ACI_OFF);
			/* Power Up */
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ACI_CTRL,
			ACI_BIAS_PWRDN,
			ACI_BIAS_PWRDN_POWERUP);
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ADC_CTRL,
				     AUDIORX_BIAS_PWRUP,
				     AUDIORX_BIAS_PWRUP_POWERUP);
		break;
	case CHAL_ACI_MIC_ROUTE_MIC:
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
			ACI_ACI_CTRL, ACI_BIAS_PWRDN,
				     ACI_BIAS_PWRDN_POWERUP);
		/* Power Up (No affect on Rhea) */
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ADC_CTRL,
				     AUDIORX_BIAS_PWRUP,
				     AUDIORX_BIAS_PWRUP_POWERUP);
		/* Power Up (No affect on Rhea) */
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
				     ACI_ACI_CTRL, SW_MIC_DATAB,
				     SW_MIC_DATAB_P_MIC_OUT);
		/* P_MIC_DATA_IN connected to P_MIC_OUT */
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ECIDATA, ECIDATA,
				     ACI_ECIDATA_ECIDATA_CMD_OFF);
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
				     ACI_ADCIN_SEL,
				     ADCIN_SEL, COMP2_TO_P_MIC_OUT);
		/* COMP2 connected to P_MIC_OUT */
		/* ACI Pull-up OFF */
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
		ACI_ACI_CTRL,
		SW_ACI, SW_ACI_OFF);
		break;
	case CHAL_ACI_MIC_ROUTE_ACI_OPEN:
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
				     ACI_ACI_CTRL, ACI_BIAS_PWRDN,
				     ACI_BIAS_PWRDN_POWERUP);
		/* Power Up (No affect on Rhea) */
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
				     ACI_ADC_CTRL, AUDIORX_BIAS_PWRUP,
				     AUDIORX_BIAS_PWRUP_POWERUP);
		/* Power Up (No affect on Rhea) */
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ACI_CTRL,
				     SW_ACI, SW_ACI_ON);
		/* ACI Pull-up ON */
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ECIDATA,
				     ECIDATA, ACI_ECIDATA_ECIDATA_CMD_OFF);
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
				     ACI_ACI_CTRL, SW_MIC_DATAB,
				     SW_MIC_DATAB_ACI_DATA);
		/* P_MIC_DATA_IN connected to ACI data */
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
				     ACI_ADCIN_SEL, ADCIN_SEL,
				     COMP2_TO_ACI_PULL);
		/* COMP2 connected to P_MIC_OUT */
		chal_aci_disable_aci();
		break;
	case CHAL_ACI_MIC_ROUTE_ACI_CLOSED:
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
				     ACI_ACI_CTRL, ACI_BIAS_PWRDN,
				     ACI_BIAS_PWRDN_POWERUP);
		/* Power Up (No affect on Rhea) */
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
				     ACI_ADC_CTRL, AUDIORX_BIAS_PWRUP,
				     AUDIORX_BIAS_PWRUP_POWERUP);
		/* Power Up (No affect on Rhea) */
		/* ACI Pull-up ON */
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
			ACI_ACI_CTRL, SW_ACI, SW_ACI_ON);
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
				     ACI_ECIDATA, ECIDATA,
				     ACI_ECIDATA_ECIDATA_CMD_ON);
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
				     ACI_ACI_CTRL, SW_MIC_DATAB,
				     SW_MIC_DATAB_ACI_DATA);
		/* P_MIC_DATA_IN connected to ACI data */
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
				     ACI_ADCIN_SEL, ADCIN_SEL,
				     COMP2_TO_ACI_PULL);
		/* COMP2 connected to P_MIC_OUT */
		chal_aci_disable_aci();
		break;
	case CHAL_ACI_MIC_ROUTE_ALL_OFF:
#ifdef CONFIG_AUDIOH_REGS
		BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA, AUDIOH_AUDIORX_VMIC,
		     AUDIORX_VAUXMIC_CTRL,
		     AUDIORX_VAUXMIC_2_10V);
#endif
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_COMP_PWD, COMP1_PWD,
		     ACI_COMP_PWD_COMP1_PWD_CMD_POWER_DOWN);
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_COMP_PWD, COMP2_PWD,
		     ACI_COMP_PWD_COMP2_PWD_CMD_POWER_DOWN);
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ADC_PWD, ADC1_PWD,
		     ACI_ADC_PWD_ADC1_PWD_CMD_POWER_DOWN);
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ACI_CTRL, ACI_BIAS_PWRDN,
		     ACI_BIAS_PWRDN_POWERDOWN);
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ADCIN_SEL, ADCIN_SEL,
		     COMP2_TO_P_MIC_OUT);
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ADC_CTRL,
		     AUDIORX_BIAS_PWRUP,
		     AUDIORX_BIAS_PWRUP_POWERDOWN);
		BRCM_WRITE_REG_FIELD(KONA_AUXMIC_VA, AUXMIC_F_PWRDWN,
		     FORCE_PWR_DWN, 1);
		BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ADC_CTRL,
		     AUDIORX_VREF_PWRUP,
		     AUDIORX_VREF_PWRUP_POWERDOWN);

#ifdef CONFIG_AUDIOH_REGS
		BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA, AUDIOH_AUDIORX_VREF,
		     AUDIORX_VREF_FASTSETTLE,
		     AUDIORX_VREF_FASTSETTLE_NORMAL);
		BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA, AUDIOH_AUDIORX_VREF,
		     AUDIORX_VREF_POWERCYCLE,
		     AUDIORX_VREF_POWERCYCLE_NORMAL);
#endif
		break;
	default:
		CHAL_ASSERT(0);
	}
}

/**
*   Function used to control Accessory Block
*
*   @param  action  Specifies the action to be performed
*   @param  id      Specifies the hardware unit to control
*   @param  arg     Optional argument depending on the action
*
*   @return Void
*
*   @todo   <todo list>
*/
static void chal_aci_block_ctrl_arg(CHAL_HANDLE handle,
				    CHAL_ACI_block_action_t action,
				    CHAL_ACI_block_id_t id, va_list argp)
{
	unsigned long reg_val;
	unsigned long mask;

	switch (action) {
	case CHAL_ACI_BLOCK_ACTION_ENABLE:
		switch (id) {
	/* The action applies to both Comperators */
		case CHAL_ACI_BLOCK_COMP:
			BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_COMP_PWD,
				COMP2_PWD,
				ACI_COMP_PWD_COMP2_PWD_CMD_POWER_UP);
			/* Fall through */
		case CHAL_ACI_BLOCK_COMP1:
			BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_COMP_PWD,
				COMP1_PWD,
				ACI_COMP_PWD_COMP1_PWD_CMD_POWER_UP);
			break;
		case CHAL_ACI_BLOCK_COMP2:
			BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_COMP_PWD,
				COMP2_PWD,
				ACI_COMP_PWD_COMP2_PWD_CMD_POWER_UP);
			break;
		case CHAL_ACI_BLOCK_ADC:
		/* The action applies to both ADC's */
			/* Fall through */
		case CHAL_ACI_BLOCK_ADC1:
			BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ADC_PWD, ADC1_PWD,
					     ACI_ADC_PWD_ADC1_PWD_CMD_POWER_UP);
			break;
		case CHAL_ACI_BLOCK_ADC2:
			break;
		case CHAL_ACI_BLOCK_DIGITAL:
			BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ACI_CTRL,
				ACI_BIAS_PWRDN,
				ACI_BIAS_PWRDN_POWERUP);
			BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_MIC_BIAS,
				MIC_BIAS,
				ACI_MIC_BIAS_MIC_BIAS_CMD_PERIODIC_MEASUREMENT_DIS);
			break;
		default:
			CHAL_ASSERT(0);
		}
		break;

	case CHAL_ACI_BLOCK_ACTION_DISABLE:
		switch (id) {
		case CHAL_ACI_BLOCK_COMP:
	/* The action applies to both Comperators */
			BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_COMP_PWD,
			COMP2_PWD,
			ACI_COMP_PWD_COMP2_PWD_CMD_POWER_DOWN);
			/* Fall through */
		case CHAL_ACI_BLOCK_COMP1:
			BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_COMP_PWD,
				COMP1_PWD,
				ACI_COMP_PWD_COMP1_PWD_CMD_POWER_DOWN);
			break;
		case CHAL_ACI_BLOCK_COMP2:
			BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_COMP_PWD,
				COMP2_PWD,
				ACI_COMP_PWD_COMP2_PWD_CMD_POWER_DOWN);
			break;
		case CHAL_ACI_BLOCK_ADC:
	/* The action applies to both ADC's */
			/* Fall through */
		case CHAL_ACI_BLOCK_ADC1:
			BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
				ACI_ADC_PWD, ADC1_PWD,
				ACI_ADC_PWD_ADC1_PWD_CMD_POWER_DOWN);
			break;
		case CHAL_ACI_BLOCK_ADC2:
			break;
		case CHAL_ACI_BLOCK_DIGITAL:
			BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ACI_CTRL,
				ACI_BIAS_PWRDN,
				ACI_BIAS_PWRDN_POWERDOWN);
			BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_MIC_BIAS,
				MIC_BIAS,
				ACI_MIC_BIAS_MIC_BIAS_CMD_PERIODIC_MEASUREMENT_EN);
			break;
		default:
			CHAL_ASSERT(0);
		}
		break;

	case CHAL_ACI_BLOCK_ACTION_RESET_FILTER:
		switch (id) {
		case CHAL_ACI_BLOCK_COMP:
	/* The action applies to both Comperators */
			chal_aci_block_ctrl_arg(handle, action,
						CHAL_ACI_BLOCK_COMP2, argp);
			/* Fall through */
		case CHAL_ACI_BLOCK_COMP1:
			BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_COMP1RESET,
				COMP1RESET,
				ACI_COMP1RESET_COMP1RESET_CMD_RST_SIGNAL_BY_FW);
			BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_COMP1RESET,
				COMP1RESET,
				~ACI_COMP1RESET_COMP1RESET_CMD_RST_SIGNAL_BY_FW);
			break;
		case CHAL_ACI_BLOCK_COMP2:
		/* Missing COMP2 defines in RDB */
			BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_COMP2RESET,
				COMP2RESET,
				ACI_COMP1RESET_COMP1RESET_CMD_RST_SIGNAL_BY_FW);
		/* Missing COMP2 defines in RDB */
			BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
				ACI_COMP2RESET, COMP2RESET,
				~ACI_COMP1RESET_COMP1RESET_CMD_RST_SIGNAL_BY_FW);
			break;
		default:
			CHAL_ASSERT(0);
		}
		break;

	case CHAL_ACI_BLOCK_ACTION_CONFIGURE_FILTER:
		switch (id) {
		case CHAL_ACI_BLOCK_COMP:
		/* The action applies to both Comperators */
			chal_aci_block_ctrl_arg(handle, action,
						CHAL_ACI_BLOCK_COMP2, argp);
			/* Fall through */
		case CHAL_ACI_BLOCK_COMP1:{
				CHAL_ACI_filter_config_comp_t *comp_config =
				    (CHAL_ACI_filter_config_comp_t *)
				    va_arg(argp, void *);
				if (comp_config->mode ==
				    CHAL_ACI_FILTER_MODE_INTEGRATE) {
					BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
						ACI_COMP1MODE,
						COMP1MODE,
						ACI_COMP1MODE_COMP1MODE_CMD_INTEGRATE_AND_DUMP);
				} else {
					BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
					    ACI_COMP1MODE,
					    COMP1MODE,
					    ACI_COMP1MODE_COMP1MODE_CMD_DECIMATE_BY_M);
				}
				if (comp_config->reset_type ==
				    CHAL_ACI_FILTER_RESET_FIRMWARE) {
					BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
						ACI_COMP1RESET,
						COMP1RESET_SW,
						ACI_COMP1RESET_COMP1RESET_SW_CMD_FILTERS_CMP1OUT2_RST_BY_FW);
				} else {
					BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
						ACI_COMP1RESET,
						COMP1RESET_SW,
						ACI_COMP1RESET_COMP1RESET_SW_CMD_FILTERS_CMP1OUT2_RST_BY_COMP1);
				}
				BRCM_WRITE_REG(KONA_ACI_VA, ACI_S1,
					       comp_config->s);
				BRCM_WRITE_REG(KONA_ACI_VA, ACI_T1,
					       comp_config->t);
				BRCM_WRITE_REG(KONA_ACI_VA, ACI_M1,
					       comp_config->m);
				BRCM_WRITE_REG(KONA_ACI_VA, ACI_MT1,
					       comp_config->mt);
				chal_aci_block_ctrl(handle,
					CHAL_ACI_BLOCK_ACTION_RESET_FILTER,
					CHAL_ACI_BLOCK_COMP1);
				break;
			}
		case CHAL_ACI_BLOCK_COMP2:{
				CHAL_ACI_filter_config_comp_t *comp_config =
				    (CHAL_ACI_filter_config_comp_t *)
				    va_arg(argp, void *);
				if (comp_config->mode ==
				    CHAL_ACI_FILTER_MODE_INTEGRATE) {
					BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
						ACI_COMP2MODE,
						COMP2MODE,
						ACI_COMP2MODE_COMP2MODE_CMD_INTEGRATE_AND_DUMP);
				} else {
					BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
					   ACI_COMP2MODE,
					   COMP2MODE,
					   ACI_COMP2MODE_COMP2MODE_CMD_DECIMATE_BY_M);
				}
				if (comp_config->reset_type ==
				    CHAL_ACI_FILTER_RESET_FIRMWARE) {
					/* Missing COMP2 defines in RDB */
					BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
					ACI_COMP2RESET,
					COMP2RESET_SW,
					ACI_COMP1RESET_COMP1RESET_SW_CMD_FILTERS_CMP1OUT2_RST_BY_FW);
				} else {
					/* Missing COMP2 defines in RDB */
					BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
					ACI_COMP2RESET,
					COMP2RESET_SW,
					ACI_COMP1RESET_COMP1RESET_SW_CMD_FILTERS_CMP1OUT2_RST_BY_COMP1);
				}
				BRCM_WRITE_REG(KONA_ACI_VA, ACI_S2,
					       comp_config->s);
				BRCM_WRITE_REG(KONA_ACI_VA, ACI_T2,
					       comp_config->t);
				BRCM_WRITE_REG(KONA_ACI_VA, ACI_M2,
					       comp_config->m);
				BRCM_WRITE_REG(KONA_ACI_VA, ACI_MT2,
					       comp_config->mt);
				chal_aci_block_ctrl(handle,
					CHAL_ACI_BLOCK_ACTION_RESET_FILTER,
					CHAL_ACI_BLOCK_COMP2);
				break;
			}
		case CHAL_ACI_BLOCK_ADC:
		/* The action applies to both ADC's */
			chal_aci_block_ctrl_arg(handle, action,
						CHAL_ACI_BLOCK_ADC2, argp);
			/* Fall through */
		case CHAL_ACI_BLOCK_ADC1:{
				CHAL_ACI_filter_config_adc_t *adc_config =
				    (CHAL_ACI_filter_config_adc_t *)
				    va_arg(argp, void *);
				if (adc_config->chopping_enable) {
					BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
						ACI_ADC_PWD,
						ADC1_DIS_CHOP,
						ACI_ADC_PWD_ADC1_DIS_CHOP_CMD_LOADING);
				} else {
					BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
						ACI_ADC_PWD,
						ADC1_DIS_CHOP,
						ACI_ADC_PWD_ADC1_DIS_CHOP_CMD_NOT_LOADING);
				}
				BRCM_WRITE_REG(KONA_ACI_VA, ACI_N1,
					       adc_config->n);
				break;
			}
		case CHAL_ACI_BLOCK_ADC2:{
#ifdef ACI_ADC_PWD_ADC2_DIS_CHOP_SHIFT
				CHAL_ACI_filter_config_adc_t *adc_config =
				    (CHAL_ACI_filter_config_adc_t *)
				    va_arg(argp, void *);
				if (adc_config->chopping_enable) {
					BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
					    ACI_ADC_PWD,
					    ADC2_DIS_CHOP,
					    ACI_ADC_PWD_ADC2_DIS_CHOP_CMD_LOADING);
				} else {
					BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
					    ACI_ADC_PWD,
					    ADC2_DIS_CHOP,
					    ACI_ADC_PWD_ADC2_DIS_CHOP_CMD_NOT_LOADING);
				}
				BRCM_WRITE_REG(KONA_ACI_VA, ACI_N2,
					       adc_config->n);
#endif
				break;
			}
		default:
			CHAL_ASSERT(0);
		}
		break;

	case CHAL_ACI_BLOCK_ACTION_COMP_THRESHOLD:{
			cUInt32 threshold_voltage = va_arg(argp, cUInt32);
			/* = (voltage * NumberOfSteps) / Reference Voltage; */
			cUInt32 threshold = ((threshold_voltage * 255) / 2500);
			switch (id) {
			case CHAL_ACI_BLOCK_COMP:
			/* The action applies to both Comperators */
				chal_aci_block_ctrl_arg(handle, action,
							CHAL_ACI_BLOCK_COMP2,
							argp);
				/* Fall through */
			case CHAL_ACI_BLOCK_COMP1:
				BRCM_WRITE_REG(KONA_ACI_VA, ACI_COMP1TH,
					       threshold);
				BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
				   ACI_COMPTH_SET,
				   COMPTH1_SET,
				   ACI_COMPTH_SET_COMPTH1_SET_CMD_LOADING);
				BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
				     ACI_COMPTH_SET,
				     COMPTH1_SET,
				     ACI_COMPTH_SET_COMPTH1_SET_CMD_NOT_LOADING);
				chal_aci_block_ctrl(handle,
					    CHAL_ACI_BLOCK_ACTION_RESET_FILTER,
					    CHAL_ACI_BLOCK_COMP1);
				break;
			case CHAL_ACI_BLOCK_COMP2:
				BRCM_WRITE_REG(KONA_ACI_VA, ACI_COMP2TH,
					       threshold);
				BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
				     ACI_COMPTH_SET,
				     COMPTH2_SET,
				     ACI_COMPTH_SET_COMPTH2_SET_CMD_LOADING);
				BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
				     ACI_COMPTH_SET,
				     COMPTH2_SET,
				     ACI_COMPTH_SET_COMPTH2_SET_CMD_NOT_LOADING);
				chal_aci_block_ctrl(handle,
				    CHAL_ACI_BLOCK_ACTION_RESET_FILTER,
				    CHAL_ACI_BLOCK_COMP2);
				break;
			default:
				CHAL_ASSERT(0);
			}
			break;
		}

	case CHAL_ACI_BLOCK_ACTION_ADC_RANGE:{
			CHAL_ACI_range_config_adc_t range =
			    (CHAL_ACI_range_config_adc_t) va_arg(argp, int);
			CHAL_ASSERT(id == CHAL_ACI_BLOCK_ADC);
			switch (range) {
			case CHAL_ACI_BLOCK_ADC_OFF:
				chal_aci_block_ctrl(handle,
				    CHAL_ACI_BLOCK_ACTION_DISABLE,
				    CHAL_ACI_BLOCK_ADC1);
				chal_aci_block_ctrl(handle,
				    CHAL_ACI_BLOCK_ACTION_DISABLE,
				    CHAL_ACI_BLOCK_ADC2);
				break;
			case CHAL_ACI_BLOCK_ADC_LOW_VOLTAGE:
				/* ADC can measure low voltages
				*(0 - 0.3125v) with hig resolution */
				chal_aci_block_ctrl(handle,
				    CHAL_ACI_BLOCK_ACTION_ENABLE,
				    CHAL_ACI_BLOCK_ADC1);
				chal_aci_block_ctrl(handle,
				    CHAL_ACI_BLOCK_ACTION_DISABLE,
				    CHAL_ACI_BLOCK_ADC2);
					/* Sets ADC low scale: 1 = 1150 mV */
				BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
					ACI_ADC_CTRL,
					ADC_FS_BIG_SMALL_B,
					ADC_FS_BIG_SMALL_B_LOW_RANGE);
					/* 1 = enable adaptive step size */
				BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
					ACI_ADC_CTRL,
					ADC_ADAPTIVE_EN, 1);
				break;
			case CHAL_ACI_BLOCK_ADC_FULL_RANGE:
			case CHAL_ACI_BLOCK_ADC_HIGH_VOLTAGE:
				/* ADC can measure full range voltages
					* (0 - 2.5v) with low resolution */
				chal_aci_block_ctrl(handle,
					CHAL_ACI_BLOCK_ACTION_ENABLE,
					CHAL_ACI_BLOCK_ADC1);
				chal_aci_block_ctrl(handle,
					CHAL_ACI_BLOCK_ACTION_DISABLE,
					CHAL_ACI_BLOCK_ADC2);
				/* Sets ADC full scale: 0= 2300 mV */
				BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
					ACI_ADC_CTRL,
					ADC_FS_BIG_SMALL_B,
					ADC_FS_BIG_SMALL_B_FULL_RANGE);
				/* 1 = enable adaptive step size */
				BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
					ACI_ADC_CTRL,
					ADC_ADAPTIVE_EN, 1);
				break;
			default:
				CHAL_ASSERT(0);
			}
			break;
		}
	case CHAL_ACI_BLOCK_ACTION_INTERRUPT_ENABLE:
/*
 * Note the defn of status bit in the rdb, writing 1 clears the
 * interrupt. So we should mask them to be zero and not touch the
 * status bits. Writing status bits as is would ack an interrupt so
 * just mask and take only the upper nibble and zero the lower nibble.
 *
 * The existing CHAL code was retainging the status bits also as it
 * is. This was causing the interrupts to be cleared un-intentionally.
 * So re-implementing this part as needed.
 */
		reg_val = BRCM_READ_REG(KONA_ACI_VA, ACI_INT) & 0x70;

		switch (id) {
		case CHAL_ACI_BLOCK_COMP:
	/* The action applies to all Comperators */
			mask = ACI_INT_INV_COMP2INT_EN_MASK |
			    ACI_INT_COMP2INT_EN_MASK |
				ACI_INT_COMP1INT_EN_MASK;
			break;
		case CHAL_ACI_BLOCK_COMP1:
			mask = ACI_INT_COMP1INT_EN_MASK;
			break;
		case CHAL_ACI_BLOCK_COMP2:
			mask = ACI_INT_COMP2INT_EN_MASK;
			break;
		case CHAL_ACI_BLOCK_COMP2_INV:
			mask = ACI_INT_INV_COMP2INT_EN_MASK;
			break;
		default:
			CHAL_ASSERT(0);
		}
		reg_val = reg_val | mask;
		BRCM_WRITE_REG(KONA_ACI_VA, ACI_INT, reg_val);
		break;
	case CHAL_ACI_BLOCK_ACTION_INTERRUPT_DISABLE:

/*
 * Note the defn of status bit in the rdb, writing 1 clears the
 * interrupt. So we should mask them to be zero and not touch the
 * status bits. Writing status bits as is would ack an interrupt so
 * just mask and take only the upper nibble and zero the lower nibble.
 */
		reg_val = BRCM_READ_REG(KONA_ACI_VA, ACI_INT) & 0x70;

		switch (id) {
		case CHAL_ACI_BLOCK_COMP:
		/* The action applies to all Comperators */
			mask = ACI_INT_INV_COMP2INT_EN_MASK |
			    ACI_INT_COMP2INT_EN_MASK |
				ACI_INT_COMP1INT_EN_MASK;
			break;
		case CHAL_ACI_BLOCK_COMP1:
			mask = ACI_INT_COMP1INT_EN_MASK;
			break;
		case CHAL_ACI_BLOCK_COMP2:
			mask = ACI_INT_COMP2INT_EN_MASK;
			break;
		case CHAL_ACI_BLOCK_COMP2_INV:
			mask = ACI_INT_INV_COMP2INT_EN_MASK;
			break;
		default:
			CHAL_ASSERT(0);
		}
		reg_val = reg_val & ~mask;
		BRCM_WRITE_REG(KONA_ACI_VA, ACI_INT, reg_val);
		break;
	case CHAL_ACI_BLOCK_ACTION_INTERRUPT_ACKNOWLEDGE:
/*
 * Note the defn of status bit in the rdb, writing 1 clears the
 * interrupt. So we should mask them to be zero and not touch the
 * status bits. Writing status bits as is would ack an interrupt so
 * just mask and take only the upper nibble and zero the lower nibble.
 */
		reg_val = BRCM_READ_REG(KONA_ACI_VA, ACI_INT) & 0x70;
		switch (id) {
		case CHAL_ACI_BLOCK_COMP:
		/* The action applies to all Comperators */
			mask = ACI_INT_INV_COMP2INT_STS_MASK |
			    ACI_INT_COMP2INT_STS_MASK |
			    ACI_INT_COMP1INT_STS_MASK;
			break;
		case CHAL_ACI_BLOCK_COMP1:
			mask = ACI_INT_COMP1INT_STS_MASK;
			break;
		case CHAL_ACI_BLOCK_COMP2:
			mask = ACI_INT_COMP2INT_STS_MASK;
			break;
		case CHAL_ACI_BLOCK_COMP2_INV:
			mask = ACI_INT_INV_COMP2INT_STS_MASK;
			break;
		default:
			CHAL_ASSERT(0);
		}
		reg_val = reg_val | mask;
		BRCM_WRITE_REG(KONA_ACI_VA, ACI_INT, reg_val);
		break;
	case CHAL_ACI_BLOCK_ACTION_MIC_BIAS:{
			CHAL_ACI_micbias_config_t *bias_config =
			    (CHAL_ACI_micbias_config_t *) va_arg(argp, void *);

			switch (bias_config->mode) {
			case CHAL_ACI_MIC_BIAS_ON:
				/* Powerup generic detection
				*block (including bias) */
				BRCM_WRITE_REG_FIELD(KONA_AUXMIC_VA,
						     AUXMIC_F_PWRDWN,
						     FORCE_PWR_DWN, 0);

				/* Configure Continuous measurement mode */
				BRCM_WRITE_REG_FIELD(KONA_AUXMIC_VA, AUXMIC_CMC,
					CONT_MSR_CTRL, 1);
				BRCM_WRITE_REG_FIELD(KONA_ACI_VA,
					ACI_MIC_BIAS,
					MIC_BIAS,
					ACI_MIC_BIAS_MIC_BIAS_CMD_PERIODIC_MEASUREMENT_DIS);

				/* Set Bias Voltage */
				if (bias_config->voltage ==
					CHAL_ACI_MIC_BIAS_0_45V) {
					/* disable, bias = 0.45V */
#ifdef CONFIG_AUDIOH_REGS
					BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA,
					   AUDIOH_AUDIORX_VMIC,
					   AUDIORX_VAUXMIC_CTRL,
					   AUDIORX_VAUXMIC_2_10V);
#endif
					BRCM_WRITE_REG_FIELD(KONA_AUXMIC_VA,
					     AUXMIC_AUXEN,
					     MICAUX_EN, 0);
				} else if (bias_config->voltage ==
						CHAL_ACI_MIC_BIAS_2_5V) {
					/* enable, bias = 2.5V */
#ifdef CONFIG_AUDIOH_REGS
					BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA,
					     AUDIOH_AUDIORX_VMIC,
					     AUDIORX_VAUXMIC_CTRL,
					     AUDIORX_VAUXMIC_2_40V);
#endif
					BRCM_WRITE_REG_FIELD(KONA_AUXMIC_VA,
					     AUXMIC_AUXEN,
					     MICAUX_EN, 1);
				} else {	/* enable, bias = 2.1V */
#ifdef CONFIG_AUDIOH_REGS
					BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA,
					     AUDIOH_AUDIORX_VMIC,
					     AUDIORX_VAUXMIC_CTRL,
					     AUDIORX_VAUXMIC_2_10V);
#endif
					BRCM_WRITE_REG_FIELD(KONA_AUXMIC_VA,
					     AUXMIC_AUXEN,
					     MICAUX_EN, 1);
				}
				break;
			case CHAL_ACI_MIC_BIAS_DISCONTINUOUS:
				/* Set Bias Voltage */
				if (bias_config->voltage ==
						CHAL_ACI_MIC_BIAS_0_45V) {
						/* disable, bias = 0.45V */
#ifdef CONFIG_AUDIOH_REGS
					BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA,
					     AUDIOH_AUDIORX_VMIC,
					     AUDIORX_VAUXMIC_CTRL,
					     AUDIORX_VAUXMIC_2_10V);
#endif
					BRCM_WRITE_REG_FIELD(KONA_AUXMIC_VA,
					     AUXMIC_AUXEN,
					     MICAUX_EN, 0);
				} else if (bias_config->voltage ==
						CHAL_ACI_MIC_BIAS_2_5V) {
						/* enable, bias = 2.5V */
#ifdef CONFIG_AUDIOH_REGS
					BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA,
					     AUDIOH_AUDIORX_VMIC,
					     AUDIORX_VAUXMIC_CTRL,
					     AUDIORX_VAUXMIC_2_40V);
#endif
					BRCM_WRITE_REG_FIELD(KONA_AUXMIC_VA,
					     AUXMIC_AUXEN,
					     MICAUX_EN, 1);
				} else {	/* enable, bias = 2.1V */
#ifdef CONFIG_AUDIOH_REGS
					BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA,
					     AUDIOH_AUDIORX_VMIC,
					     AUDIORX_VAUXMIC_CTRL,
					     AUDIORX_VAUXMIC_2_10V);
#endif
					BRCM_WRITE_REG_FIELD(KONA_AUXMIC_VA,
					     AUXMIC_AUXEN,
					     MICAUX_EN, 1);
				}
				BRCM_WRITE_REG(KONA_AUXMIC_VA, AUXMIC_PRB_CYC,
					bias_config->probe_cycle);
				BRCM_WRITE_REG(KONA_AUXMIC_VA, AUXMIC_MSR_DLY,
					 bias_config->measure_delay);
				BRCM_WRITE_REG(KONA_AUXMIC_VA, AUXMIC_MSR_INTVL,
					 bias_config->measure_interval);
				BRCM_WRITE_REG_FIELD(KONA_AUXMIC_VA, AUXMIC_MIC,
					 MSR_INTVL_CTRL, 0);
				BRCM_WRITE_REG_FIELD(KONA_AUXMIC_VA, AUXMIC_CMC,
					 CONT_MSR_CTRL, 0);
				BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_MIC_BIAS,
				    MIC_BIAS,
				    ACI_MIC_BIAS_MIC_BIAS_CMD_PERIODIC_MEASUREMENT_EN);
				/* Powerup generic detection
				*block (including bias) */
				BRCM_WRITE_REG_FIELD(KONA_AUXMIC_VA,
				     AUXMIC_F_PWRDWN,
				     FORCE_PWR_DWN, 0);
				break;
			case CHAL_ACI_MIC_BIAS_OFF:
				/* MIC Bias cannot turned full
				* off in this state.MIC is also
				*the supply for other internal
				blocks */
				/* Powerup generic detection
				*block (including bias) */
				BRCM_WRITE_REG_FIELD(KONA_AUXMIC_VA,
				     AUXMIC_F_PWRDWN,
				     FORCE_PWR_DWN, 0);
				/* Configure Continuous measurement mode */
				BRCM_WRITE_REG_FIELD(KONA_AUXMIC_VA, AUXMIC_CMC,
						     CONT_MSR_CTRL, 1);
				BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_MIC_BIAS,
				   MIC_BIAS,
				   ACI_MIC_BIAS_MIC_BIAS_CMD_PERIODIC_MEASUREMENT_DIS);
				break;
			case CHAL_ACI_MIC_BIAS_GND:
				/* MIC Bias can not turned full off
				*in this state. MIC is also the supply
				*for other internal blocks */
				/* Configure Lo-Z (GND) impedance
				*when MIC bias is powered down */
				BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_MIC_BIAS,
				     MIC_AUX_BIAS_GND,
				     ACI_MIC_BIAS_MIC_AUX_BIAS_GND_CMD_IMPEDANCE_CONTROL_LO_Z);
				/* Powerdown generic detection
				*block (including bias) */
				BRCM_WRITE_REG_FIELD(KONA_AUXMIC_VA,
				    AUXMIC_F_PWRDWN,
				    FORCE_PWR_DWN, 1);
				/* Configure Continuous measurement mode */
				BRCM_WRITE_REG_FIELD(KONA_AUXMIC_VA, AUXMIC_CMC,
						     CONT_MSR_CTRL, 1);
				BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_MIC_BIAS,
				     MIC_BIAS,
				     ACI_MIC_BIAS_MIC_BIAS_CMD_PERIODIC_MEASUREMENT_DIS);
				break;
			case CHAL_ACI_MIC_BIAS_HIZ:
				/* MIC Bias can not turned full off
				*in this state.MIC is also the supply
				*for other internal blocks */
				/* Configure Hi-Z impedance
				*when MIC bias is powered down */
				BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_MIC_BIAS,
				    MIC_AUX_BIAS_GND,
				    ACI_MIC_BIAS_MIC_AUX_BIAS_GND_CMD_IMPEDANCE_CONTROL_HI_Z);
				/* Powerdown generic detection
				*block (including bias) */
				BRCM_WRITE_REG_FIELD(KONA_AUXMIC_VA,
						     AUXMIC_F_PWRDWN,
						     FORCE_PWR_DWN, 1);
				/* Configure Continuous measurement mode */
				BRCM_WRITE_REG_FIELD(KONA_AUXMIC_VA, AUXMIC_CMC,
				    CONT_MSR_CTRL, 1);
				BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_MIC_BIAS,
				    MIC_BIAS,
				    ACI_MIC_BIAS_MIC_BIAS_CMD_PERIODIC_MEASUREMENT_DIS);
				break;
			}
			break;
		}
	case CHAL_ACI_BLOCK_ACTION_MIC_POWERDOWN_HIZ_IMPEDANCE:{
			Boolean hiz = (Boolean) va_arg(argp, int);
			CHAL_ASSERT(id == CHAL_ACI_BLOCK_GENERIC);
			if (hiz == TRUE) {
				/* Configure Hi-Z (21K) impedance
				* when MIC bias is powered down */
				BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_MIC_BIAS,
				   MIC_AUX_BIAS_GND,
				   ACI_MIC_BIAS_MIC_AUX_BIAS_GND_CMD_IMPEDANCE_CONTROL_HI_Z);
			} else {
				/* Configure Lo-Z (GND) impedance
				* when MIC bias is powered down */
				BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_MIC_BIAS,
					MIC_AUX_BIAS_GND,
					ACI_MIC_BIAS_MIC_AUX_BIAS_GND_CMD_IMPEDANCE_CONTROL_LO_Z);
			}
			break;
		}
	case CHAL_ACI_BLOCK_ACTION_VREF:{
			CHAL_ACI_vref_config_t *vref_config =
			    (CHAL_ACI_vref_config_t *) va_arg(argp, void *);
			switch (vref_config->mode) {
			case CHAL_ACI_VREF_FAST_ON:
				/* Vref Powerup */
#ifdef CONFIG_AUDIOH_REGS
				BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA,
				     AUDIOH_AUDIORX_VREF,
				     AUDIORX_VREF_FASTSETTLE,
				     AUDIORX_VREF_FASTSETTLE_FAST);
				BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA,
				    AUDIOH_AUDIORX_VREF,
				    AUDIORX_VREF_POWERCYCLE,
				    AUDIORX_VREF_POWERCYCLE_FAST);
				BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA,
				    AUDIOH_AUDIORX_VREF,
				    AUDIORX_VREF_PWRUP,
				    AUDIORX_VREF_PWRUP_POWERUP);
#endif
				BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ADC_CTRL,
				   AUDIORX_VREF_PWRUP,
				   AUDIORX_VREF_PWRUP_POWERUP);

				/* Feedback from ASIC Team - We need
				 * keep fast_settle high for
				 * 10mS once you power up VREF_PWRUP.
				 * For normal operation,
				 * fast_settle should be low
				 */
				if (audioh_apb_clk != NULL) {
					int err = clk_enable(audioh_apb_clk);
					if (err) {
						pr_err("clk_enable(audioh_apb_clk) failed - %d", err);
						return;
					}
					BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA,
					    AUDIOH_AUDIORX_VREF,
					    AUDIORX_VREF_FASTSETTLE,
					    AUDIORX_VREF_FASTSETTLE_FAST);
					usleep_range(10000, 10500);
					BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA,
					    AUDIOH_AUDIORX_VREF,
					    AUDIORX_VREF_FASTSETTLE,
					    AUDIORX_VREF_FASTSETTLE_NORMAL);
					clk_disable(audioh_apb_clk);
				}
				break;
			case CHAL_ACI_VREF_ON:
				/* Vref Powerup */
#ifdef CONFIG_AUDIOH_REGS
				BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA,
				     AUDIOH_AUDIORX_VREF,
				     AUDIORX_VREF_POWERCYCLE,
				     AUDIORX_VREF_POWERCYCLE_FAST);
				BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA,
				     AUDIOH_AUDIORX_VREF,
				     AUDIORX_VREF_FASTSETTLE,
				     AUDIORX_VREF_FASTSETTLE_NORMAL);
				BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA,
				    AUDIOH_AUDIORX_VREF,
				    AUDIORX_VREF_PWRUP,
				    AUDIORX_VREF_PWRUP_POWERUP);
#endif
				BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ADC_CTRL,
				     AUDIORX_VREF_PWRUP,
				     AUDIORX_VREF_PWRUP_POWERUP);
				break;
			case CHAL_ACI_VREF_OFF:
				/* Vref Poweroff */
#ifdef CONFIG_AUDIOH_REGS
				BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA,
				     AUDIOH_AUDIORX_VREF,
				     AUDIORX_VREF_POWERCYCLE,
				     AUDIORX_VREF_POWERCYCLE_NORMAL);
				BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA,
				     AUDIOH_AUDIORX_VREF,
				     AUDIORX_VREF_FASTSETTLE,
				     AUDIORX_VREF_FASTSETTLE_NORMAL);
				BRCM_WRITE_REG_FIELD(KONA_AUDIOH_VA,
				    AUDIOH_AUDIORX_VREF,
				    AUDIORX_VREF_PWRUP,
				    AUDIORX_VREF_PWRUP_POWERDOWN);
#endif
				BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ADC_CTRL,
				   AUDIORX_VREF_PWRUP,
				   AUDIORX_VREF_PWRUP_POWERDOWN);
				break;
			default:
				CHAL_ASSERT(0);
			}
			break;
		}
	default:
		CHAL_ASSERT(0);
	}
}

cVoid chal_aci_block_ctrl(CHAL_HANDLE handle, CHAL_ACI_block_action_t action,
			  CHAL_ACI_block_id_t id, ...)
{
	va_list argp;
	va_start(argp, id);
	chal_aci_block_ctrl_arg(handle, action, id, argp);
	va_end(argp);
}

/**
*   This function will read the value on a specified output and return the
*   value.
*
*   @param  id      Specifies the hardware unit to read
*   @param  output  Specifies the hardware unit output type
*
*   @return Value read, the exact type depends on the id selected
*
*   @todo   <todo list>
*/
cInt32 chal_aci_block_read(CHAL_HANDLE handle, CHAL_ACI_block_id_t id,
			   CHAL_ACI_block_output_id_t output)
{
	switch (id) {
	case CHAL_ACI_BLOCK_COMP1:
		switch (output) {
		case CHAL_ACI_BLOCK_COMP_RAW:
			return BRCM_READ_REG_FIELD
			    (KONA_ACI_VA, ACI_COMP_DOUT,
			     COMP1_DOUT) ? CHAL_ACI_BLOCK_COMP_LINE_LOW :
			    CHAL_ACI_BLOCK_COMP_LINE_HIGH;
		case CHAL_ACI_BLOCK_COMP_FILTER1:
			return BRCM_READ_REG_FIELD
			    (KONA_ACI_VA, ACI_COMP_DOUT,
			     COMP1_DOUT1) ? CHAL_ACI_BLOCK_COMP_LINE_LOW :
			    CHAL_ACI_BLOCK_COMP_LINE_HIGH;
		case CHAL_ACI_BLOCK_COMP_FILTER2:
			return BRCM_READ_REG_FIELD
			    (KONA_ACI_VA, ACI_COMP_DOUT,
			     COMP1_DOUT2) ? CHAL_ACI_BLOCK_COMP_LINE_LOW :
			    CHAL_ACI_BLOCK_COMP_LINE_HIGH;
		case CHAL_ACI_BLOCK_COMP_INTERRUPT:
			return BRCM_READ_REG_FIELD
			    (KONA_ACI_VA, ACI_INT,
			     COMP1INT_STS) ? CHAL_ACI_BLOCK_COMP_LINE_HIGH :
			    CHAL_ACI_BLOCK_COMP_LINE_LOW;
		default:
			CHAL_ASSERT(0);
		}
		break;
	case CHAL_ACI_BLOCK_COMP2:
		switch (output) {
		case CHAL_ACI_BLOCK_COMP_RAW:
			return BRCM_READ_REG_FIELD
			    (KONA_ACI_VA, ACI_COMP_DOUT,
			     COMP2_DOUT) ? CHAL_ACI_BLOCK_COMP_LINE_LOW :
			    CHAL_ACI_BLOCK_COMP_LINE_HIGH;
		case CHAL_ACI_BLOCK_COMP_FILTER1:
			return BRCM_READ_REG_FIELD
			    (KONA_ACI_VA, ACI_COMP_DOUT,
			     COMP2_DOUT1) ? CHAL_ACI_BLOCK_COMP_LINE_LOW :
			    CHAL_ACI_BLOCK_COMP_LINE_HIGH;
		case CHAL_ACI_BLOCK_COMP_FILTER2:
			return BRCM_READ_REG_FIELD
			    (KONA_ACI_VA, ACI_COMP_DOUT,
			     COMP2_DOUT2) ? CHAL_ACI_BLOCK_COMP_LINE_LOW :
			    CHAL_ACI_BLOCK_COMP_LINE_HIGH;
		case CHAL_ACI_BLOCK_COMP_INTERRUPT:
			return BRCM_READ_REG_FIELD
			    (KONA_ACI_VA, ACI_INT,
			     COMP2INT_STS) ? CHAL_ACI_BLOCK_COMP_LINE_HIGH :
			    CHAL_ACI_BLOCK_COMP_LINE_LOW;
		default:
			CHAL_ASSERT(0);
		}
		break;
	case CHAL_ACI_BLOCK_COMP2_INV:
		if (output == CHAL_ACI_BLOCK_COMP_INTERRUPT) {
			return BRCM_READ_REG_FIELD
			    (KONA_ACI_VA, ACI_INT,
			     INV_COMP2INT_STS) ?
			    CHAL_ACI_BLOCK_COMP_LINE_HIGH :
			    CHAL_ACI_BLOCK_COMP_LINE_LOW;
		} else {
		}
		break;
	case CHAL_ACI_BLOCK_ADC:{
			cUInt32 value;
			cInt32 ret = -1;

			if (output == CHAL_ACI_BLOCK_ADC_RAW) {
				value =
				    BRCM_READ_REG(KONA_ACI_VA, ACI_ADC1_DOUT);
				if (value >=
						(ACI_ADC1_DOUT_ADC1_DOUT_MASK &
						(~0x3))) {
						/* ADC overflow */
					return -1;
				}
			} else {
				value =
				    BRCM_READ_REG(KONA_ACI_VA, ACI_ADC1_DOUTF);
				if (value >=
					(ACI_ADC1_DOUTF_ADC1_DOUTF_MASK &
					(~0x3))) {
					/* ADC overflow */
					return -1;
				}
			}

			/* Convert to mV */
			if (BRCM_READ_REG_FIELD(KONA_ACI_VA, ACI_ADC_CTRL,
					ADC_FS_BIG_SMALL_B) ==
					ADC_FS_BIG_SMALL_B_LOW_RANGE) {
					/*  ADC full scale: 1150 mV */
				/* = Ref Voltage * Reading / Steps */
				ret = ((1150UL * (value)) / 0x3FC);
			} else {	/* ADC full scale: 2300 mV */
				/* = Ref Voltage * Reading / Steps */
				ret = ((2300UL * (value)) / 0x3FC);
			}
			return ret;
		}
	default:
		CHAL_ASSERT(0);
	}
	return -1;
}

/* -------------------------------------------------------------------------- */
/*
 *
 * Accessory ACI Hardware abstraction
 *
 *
 */
/* -------------------------------------------------------------------------- */

/**
*   This function will initialize the ACI block
*
*   @param  Void
*
*   @return Void
*
*   @todo   <todo list>
*/
static cVoid chal_aci_init_aci(cUInt32 baseAddr)
{
	/* Disable all interrupts from ACI block */
	BRCM_WRITE_REG(KONA_ACI_VA, ACI_IMR, ACI_IMR_ACIIMR_MASK);
	HW_ACI_REGISTER_WRITE_SETTLE;

	/* Clear all pending interrupts in ACI block */
	BRCM_WRITE_REG(KONA_ACI_VA, ACI_IIDR, ACI_IIDR_ACIIIDR_MASK);
	HW_ACI_REGISTER_WRITE_SETTLE;

	BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ACI_TX_INV, ACI_TX_INV,
			     ACI_TX_INV_SETTING);

	if (ACI_TX_INV_SETTING == ACI_ACI_TX_INV_ACI_TX_INV_CMD_INVETED)
		CHAL_ACI_WRITE_COMR(DBIDOS, ACI_COMR_DBIDOS_CMD_TXUP);
	else
		CHAL_ACI_WRITE_COMR(DBIDOS, ACI_COMR_DBIDOS_CMD_TXDOWN);

	HW_ACI_REGISTER_WRITE_SETTLE;
}

/**
*   This function will enbale the ACI block
*
*   @param  Void
*
*   @return Void
*
*   @todo   <todo list>
*/
cVoid chal_aci_enable_aci(void)
{
	BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ACI_TX_INV, ACI_TX_INV,
			     ACI_TX_INV_SETTING);
	CHAL_ACI_WRITE_COMR(DBIDOC, ACI_COMR_DBIDOC_CMD_INUSE);
	HW_ACI_REGISTER_WRITE_SETTLE;
}

/**
*   This function will disable the ACI block
*
*   @param  Void
*
*   @return Void
*
*   @todo   <todo list>
*/
cVoid chal_aci_disable_aci(void)
{
	BRCM_WRITE_REG_FIELD(KONA_ACI_VA, ACI_ACI_TX_INV, ACI_TX_INV,
			     ACI_TX_INV_SETTING);
	chal_aci_tx_output_high(TRUE);
	CHAL_ACI_WRITE_COMR(DBIDOC, ACI_COMR_DBIDOC_CMD_DISABLED);
	HW_ACI_REGISTER_WRITE_SETTLE;
}

/**
*   This function will check if the ACI block is enabled
*
*   @param  Void
*
*   @return Void
*
*   @todo   <todo list>
*/
cBool chal_aci_is_aci_enabled(void)
{
	return (BRCM_READ_REG_FIELD(KONA_ACI_VA, ACI_COMR, DBIDOC) ==
		ACI_COMR_DBIDOC_CMD_INUSE ? TRUE : FALSE);
}

/**
*   This function will configure the RX input select
*
*   @param  input_type  Selects the input type
*
*   @return Void
*
*   @todo   <todo list>
*/
cVoid chal_aci_rx_input_set(CHAL_ACI_RX_INPUT_TYPE_t input_type)
{
	if (input_type == CHAL_ACI_RX_DIRECT)
		CHAL_ACI_WRITE_COMR(DBICON, ACI_COMR_DBICON_CMD_DIRECT);
	else
		CHAL_ACI_WRITE_COMR(DBICON, ACI_COMR_DBICON_CMD_VOTE);

	HW_ACI_REGISTER_WRITE_SETTLE;
}

/**
*   This function will configure the Bus Holde time
*
*   @param  bus_hold    Selects the Bus Holde time
*
*   @return Void
*
*   @todo   <todo list>
*/
cVoid chal_aci_bus_hold_set(CHAL_ACI_BUS_HOLD_TIME_t bus_hold)
{
	if (bus_hold == CHAL_ACI_BUS_HOLD_2MS)
		CHAL_ACI_WRITE_COMR(BUSHT, ACI_COMR_BUSHT_CMD_BUSHOLDTIME_2MS);
	else
		CHAL_ACI_WRITE_COMR(BUSHT, ACI_COMR_BUSHT_CMD_BUSHOLDTIME_10MS);

	HW_ACI_REGISTER_WRITE_SETTLE;
}

/**
*   This function will configure the Start Pulse Length
*
*   @param  start_pulse_length  Selects the Start Pulse Length
*
*   @return Void
*
*   @todo   <todo list>
*/
cVoid chal_aci_start_pulse_set(cUInt8 start_pulse_length)
{
	if (start_pulse_length <= ACI_SPLR_ACISPLR_MASK)
		BRCM_WRITE_REG(KONA_ACI_VA, ACI_SPLR, start_pulse_length);
		HW_ACI_REGISTER_WRITE_SETTLE;

}

/**
*   This function will configure the use of the RxDaR register
*
*   @param  use Selects the use
*
*   @return Void
*
*   @todo   <todo list>
*/
cVoid chal_aci_rxdar_use(CHAL_ACI_RXDAR_USE_t use)
{
	if (use == CHAL_ACI_RXDAR_NORMAL)
		CHAL_ACI_WRITE_COMR(DBIDIC, ACI_COMR_DBIDIC_CMD_NORMAL);
	else
		CHAL_ACI_WRITE_COMR(DBIDIC, ACI_COMR_DBIDIC_CMD_DATAREG);

	HW_ACI_REGISTER_WRITE_SETTLE;
}

/**
*   This function will configure the RxDaR source
*
*   @param  source  Selects the source
*
*   @return Void
*
*   @todo   <todo list>
*/
cVoid chal_aci_rxdar_source(CHAL_ACI_RXDAR_SOURCE_t source)
{
	if (source == CHAL_ACI_RXDAR_SPEED)
		CHAL_ACI_WRITE_COMR(DBIDIS, ACI_COMR_DBIDIS_CMD_SPEEDREG);
	else
		CHAL_ACI_WRITE_COMR(DBIDIS, ACI_COMR_DBIDIS_CMD_RXMSB);

	HW_ACI_REGISTER_WRITE_SETTLE;
}

/**
*   This function will force Tx Output
*
*   @param  hight   Selects a High or Low output on Tx pin
*
*   @return Void
*
*   @todo   <todo list>
*/
cVoid chal_aci_tx_output_high(cBool high)
{
	if (high)
		CHAL_ACI_WRITE_COMR(DBIDOS, 0);
	else
		CHAL_ACI_WRITE_COMR(DBIDOS, 1);

	HW_ACI_REGISTER_WRITE_SETTLE;
}

/**
*   This function will read the Interrupt status register
*
*   @param  Void
*
*   @return The data read from Interrupt ID register
*
*   @todo   <todo list>
*/
CHAL_ACI_INTERRUPT_MASK_t chal_aci_interrupt_read(void)
{
	return (CHAL_ACI_INTERRUPT_MASK_t) (BRCM_READ_REG
					    (KONA_ACI_VA, ACI_IIDR));
}

/**
*   This function will read Rx input
*
*   @param  Void
*
*   @return The data read from Rx Data register
*
*   @todo   <todo list>
*/
cUInt16 chal_aci_rx_read(void)
{
	return (cUInt16) (BRCM_READ_REG(KONA_ACI_VA, ACI_RXDAR));
}

/**
*   This function will write the Tx output register
*
*   @param  data    Data for Output register
*
*   @return Void
*
*   @todo   <todo list>
*/
cVoid chal_aci_tx_write(cUInt16 data)
{
	BRCM_WRITE_REG(KONA_ACI_VA, ACI_TXDAR, data);
	HW_ACI_REGISTER_WRITE_SETTLE;
}

/**
*   This function will write the Command register
*
*   @param  command Comamnd for the register
*
*   @return Void
*
*   @todo   <todo list>
*/
cVoid chal_aci_command(CHAL_ACI_COMMAND_t command)
{
	if (command <= CHAL_ACI_CMD_MAX) {
		cUInt16 reg;
		reg = (cUInt16) BRCM_READ_REG(KONA_ACI_VA, ACI_COMR);
		reg &= ~ACI_COMR_DBICMD_MASK;
		reg |= (command & ACI_COMR_DBICMD_MASK);
		BRCM_WRITE_REG(KONA_ACI_VA, ACI_COMR, reg);
		HW_ACI_REGISTER_WRITE_SETTLE;
	}
}

/**
*   This function will enable ACI block interrupts given by mask
*
*   @param  mask    The interrupt mask to configure
*
*   @return Void
*
*   @todo   <todo list>
*/
cVoid chal_aci_interrupt_enable(CHAL_ACI_INTERRUPT_SELECT_t mask)
{
	cUInt16 reg;

	reg = (cUInt16) BRCM_READ_REG(KONA_ACI_VA, ACI_IMR);
	reg &= ~mask;
	BRCM_WRITE_REG(KONA_ACI_VA, ACI_IMR, reg);
	HW_ACI_REGISTER_WRITE_SETTLE;
}

/**
*   This function will disable ACI block interrupts given by mask
*
*   @param  mask    The interrupt mask to configure
*
*   @return Void
*
*   @todo   <todo list>
*/
cVoid chal_aci_interrupt_disable(CHAL_ACI_INTERRUPT_SELECT_t mask)
{
	cUInt16 reg;

	reg = (cUInt16) BRCM_READ_REG(KONA_ACI_VA, ACI_IMR);
	reg |= mask;
	BRCM_WRITE_REG(KONA_ACI_VA, ACI_IMR, reg);
	HW_ACI_REGISTER_WRITE_SETTLE;
}

/**
*   This function will acknowledge ACI block interrupts given by mask
*
*   @param  mask    The interrupt mask to configure
*
*   @return Void
*
*   @todo   <todo list>
*/
cVoid chal_aci_interrupt_acknowledge(CHAL_ACI_INTERRUPT_SELECT_t mask)
{
	BRCM_WRITE_REG(KONA_ACI_VA, ACI_IIDR, mask);
	HW_ACI_REGISTER_WRITE_SETTLE;
}

/***************************************************************************
*   This function will power on the ACI part for AUX MIC for audio
*
*   @param  Void
*
*   @return Void
*
*   @todo   <todo list>
*/
cVoid chal_aci_powerup_auxmic(void)
{
	/* 0x3500E0D0 = 0x20 */
	BRCM_WRITE_REG(KONA_ACI_VA, ACI_ACI_CTRL, 0x00000020);
	/* 0x35000E0D4 = 0xC0 */
	BRCM_WRITE_REG(KONA_ACI_VA, ACI_ADC_CTRL, 0x000000C0);
	/* 0x35000E028 = 0x0 */
	BRCM_WRITE_REG(KONA_AUXMIC_VA, AUXMIC_F_PWRDWN, 0x00000000);
	/* 0x35000E00C = 0x1 */
	BRCM_WRITE_REG(KONA_AUXMIC_VA, AUXMIC_CMC, 0x00000001);
	/* 0x35000E0C4 = 0x0 */
	BRCM_WRITE_REG(KONA_ACI_VA, ACI_MIC_BIAS, 0x00000000);
	/* 0x35000E014 = 0x1 */
	BRCM_WRITE_REG(KONA_AUXMIC_VA, AUXMIC_AUXEN, 0x00000001);
}

/** @} */
