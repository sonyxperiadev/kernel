/***************************************************************************
*
* Copyright 2004 - 2009 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*
****************************************************************************/
/**
*  @file   chal_dsi.c
*
*  @brief  HERA/RHEA DSI cHAL
*
*  @note
*
****************************************************************************/

#include <linux/string.h>

#include "plat/chal/chal_common.h"
#include "plat/chal/chal_dsi.h"
#include "mach/rdb/brcm_rdb_sysmap.h"
#include "mach/rdb/brcm_rdb_dsi1.h"

#include "mach/rdb/brcm_rdb_util.h"


/*
 * RDB diff from Rhea to Hawaii.
 * Reuse code!
 */
#define DSI1_CTRL_DISP_CRCC_SHIFT DSI1_CTRL_DIS_DISP_CRCC_SHIFT
#define DSI1_CTRL_DISP_CRCC_MASK DSI1_CTRL_DIS_DISP_CRCC_MASK
#define DSI1_CTRL_DISP_ECCC_SHIFT DSI1_CTRL_DIS_DISP_ECCC_SHIFT
#define DSI1_CTRL_DISP_ECCC_MASK DSI1_CTRL_DIS_DISP_ECCC_MASK
#define DSI1_CTRL_DSI_EN_SHIFT DSI1_CTRL_DSI1_EN_SHIFT
#define DSI1_CTRL_DSI_EN_MASK DSI1_CTRL_DSI1_EN_MASK

#define DSI1_TXPKT1_H_WC_CDFIFO_SHIFT DSI1_TXPKT1_H_BC_CMDFIFO_SHIFT
#define DSI1_TXPKT1_H_WC_CDFIFO_MASK DSI1_TXPKT1_H_BC_CMDFIFO_MASK
#define DSI1_TXPKT1_H_WC_PARAM_SHIFT DSI1_TXPKT1_H_BC_PARAM_SHIFT
#define DSI1_TXPKT1_H_WC_PARAM_MASK DSI1_TXPKT1_H_BC_PARAM_MASK
#define DSI1_TXPKT2_H_WC_CDFIFO_SHIFT DSI1_TXPKT2_H_BC_CMDFIFO_MASK
#define DSI1_TXPKT2_H_WC_CDFIFO_MASK DSI1_TXPKT2_H_BC_PARAM_SHIFT
#define DSI1_TXPKT2_H_WC_PARAM_SHIFT DSI1_TXPKT2_H_BC_PARAM_MASK
#define DSI1_TXPKT2_H_WC_PARAM_MASK DSI1_TXPKT2_H_DT_SHIFT

#define DSI1_RXPKT1_H_WC_PARAM_SHIFT DSI1_RXPKT1_H_BC_PARAM_SHIFT
#define DSI1_RXPKT1_H_WC_PARAM_MASK DSI1_RXPKT1_H_BC_PARAM_MASK
#define DSI1_TXPKT_PIXD_FIFO_PIXEL_SHIFT DSI1_TXPKT_PIXD_FIFO_WORD_SHIFT
#define DSI1_TXPKT_PIXD_FIFO_PIXEL_MASK DSI1_TXPKT_PIXD_FIFO_WORD_MASK
#define DSI1_LP_DLT6_LPX_SHIFT DSI1_LP_DLT6_LP_LPX_SHIFT
#define DSI1_LP_DLT6_LPX_MASK DSI1_LP_DLT6_LP_LPX_MASK
#define DSI1_PHY_TST2_PHYD0_TEST_HSDATA_SHIFT DSI1_PHY_TST2_PHYD0_HSDATA_SHIFT
#define DSI1_PHY_TST2_PHYD0_TEST_HSDATA_MASK DSI1_PHY_TST2_PHYD0_HSDATA_MASK

struct CHAL_DSI {
	cBool init;
	cUInt32 baseAddr;
	cInt32 dlCount;
	cBool clkContinuous;
};

/* NO OF DATA LINES SUPPORTED */
#define	DSI_DL_COUNT		    4
/* NO OF CONTROLLERs */
#define	DSI_DEV_COUNT		    1

/*
 * Local Variables
 */
static struct CHAL_DSI dsi_dev[DSI_DEV_COUNT];

/*
 * Local Functions
 */
/* SETs REGISTER BIT FIELD; VALUE IS 0 BASED */
#define	DSI_REG_FIELD_SET(r, f, d)	\
	(((BRCM_REGTYPE(r))(d) << BRCM_FIELDSHIFT(r, f)) &	\
	BRCM_FIELDMASK(r, f))

/*SETs REGISTER BITs Defined WITH MASK */
#define	DSI_REG_WRITE_MASKED(b, r, m, d)	\
	(BRCM_WRITE_REG(b, r, (BRCM_READ_REG(b, r) & (~m)) | d))

/* DSI COMMAND TYPE */
#define	CMND_CTRL_CMND_PKT	    0
#define	CMND_CTRL_CMND_PKTwBTA	    1
#define	CMND_CTRL_TRIG		    2
#define	CMND_CTRL_BTA		    3

/* DSI PACKET SOURCE */
#define	DSI_PKT_SRC_CMND_FIFO	    0
#define	DSI_PKT_SRC_DE0		    1
#define	DSI_PKT_SRC_DE1		    2
#define	DSI_PKT_SRC_INV		    3

#define	DSI_DT_VC_MASK		    0x000000C0
#define	DSI_DT_VC_MASK_SHIFT	    6
#define	DSI_DT_MASK		    0x0000003F

/* D-PHY Timing Record */
struct DSI_COUNTER {
	char *name;
	UInt32 timeBase;
	UInt32 mode;
	/* esc_clk LP counters are speced using this member */
	UInt32 time_lpx;
	UInt32 time_min1_ns;
	UInt32 time_min1_ui;
	UInt32 time_min2_ns;
	UInt32 time_min2_ui;
	UInt32 time_max_ns;
	UInt32 time_max_ui;
	UInt32 counter_min;
	UInt32 counter_max;
	UInt32 counter_step;
	UInt32 counter_offs;
	/* calculated value of the register */
	UInt32 counter;
	/* dbg */
	UInt32 period;
};

/*--- Counter Mode Flags */
/* record has MAX value set */
#define	DSI_C_HAS_MAX	     1
/* record MIN value is MAX of 2 values */
#define	DSI_C_MIN_MAX_OF_2   2

/*--- Counter timeBase Flags */
/* ESC2LPDT entry - must be first record */
#define	DSI_C_TIME_ESC2LPDT  0
/* counts in HS Bit Clk */
#define	DSI_C_TIME_HS	     1
/* counts in ESC CLKs */
#define	DSI_C_TIME_ESC	     2

/* DSI Core Timing Registers */
enum DSI_TIMING_C {
	DSI_C_ESC2LP_RATIO = 0,
	DSI_C_HS_INIT,
	DSI_C_HS_WAKEUP,
	DSI_C_LP_WAKEUP,
	DSI_C_HS_CLK_PRE,
	DSI_C_HS_CLK_PREPARE,
	DSI_C_HS_CLK_ZERO,
	DSI_C_HS_CLK_POST,
	DSI_C_HS_CLK_TRAIL,
	DSI_C_HS_LPX,
	DSI_C_HS_PRE,
	DSI_C_HS_ZERO,
	DSI_C_HS_TRAIL,
	DSI_C_HS_EXIT,
	DSI_C_LPX,
	DSI_C_LP_TA_GO,
	DSI_C_LP_TA_SURE,
	DSI_C_LP_TA_GET,
	DSI_C_MAX,
};

/*
 *
 * Function Name: chal_dsi_init
 *
 * Description:   Initialize DSI Controller  and software interface
 *
 */
CHAL_HANDLE chal_dsi_init(cUInt32 baseAddr, pCHAL_DSI_INIT dsiInit)
{
	struct CHAL_DSI *pDev = NULL;
	cUInt32 i;

	if (dsiInit->dlCount > DSI_DL_COUNT) {
		chal_dprintf(CDBG_ERRO,
			     "ERROR: chal_dsi_init: DataLine Count\n");
		return (CHAL_HANDLE)NULL;
	}

	for (i = 0; i < DSI_DEV_COUNT; i++) {
		if (dsi_dev[i].init) {
			if (dsi_dev[i].baseAddr == baseAddr) {
				pDev = (struct CHAL_DSI *)&dsi_dev[i];
				break;
			}
		} else {
			pDev = (struct CHAL_DSI *)&dsi_dev[i];
			pDev->init = TRUE;
			pDev->baseAddr = baseAddr;
			pDev->dlCount = dsiInit->dlCount;
			pDev->clkContinuous = dsiInit->clkContinuous;
			break;
		}
	}

	if (pDev == NULL)
		chal_dprintf(CDBG_ERRO, "ERROR: chal_dsi_init: failed ...\n");

	return (CHAL_HANDLE)pDev;
}

/*
 *
 *  Function Name:  chal_dsi_phy_state
 *
 *  Description:    PHY State Control
 *                  Controls Clock & Data Line States
 *
 *  CLK lane cannot be directed to STOP state. It is normally in stop state
 *  if cont clock is not enabled.
 *  CLK lane will not transition to ULPS if cont clk is enabled.
 *  When ULPS is requested and cont clock was enabled it will be disabled
 *  at same time as ULPS is activated, and re-enabled upon ULPS exit
 *
 */
cVoid chal_dsi_phy_state(CHAL_HANDLE handle, CHAL_DSI_PHY_STATE_t state)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;
	cUInt32 regMask = 0;
	cUInt32 regVal = 0;

	regMask = DSI_REG_FIELD_SET(DSI1_PHYC, FORCE_TXSTOP_0, 1)
	    | DSI_REG_FIELD_SET(DSI1_PHYC, TXULPSCLK, 1)
	    | DSI_REG_FIELD_SET(DSI1_PHYC, TX_HSCLK_CONT, 1);

#if !defined(CONFIG_MACH_BCM_FPGA_E) && !defined(CONFIG_MACH_BCM_FPGA)
	switch (pDev->dlCount) {
	case 4:
		regMask |= DSI_REG_FIELD_SET(DSI1_PHYC, TXULPSESC_3, 1);
		regVal |= DSI_REG_FIELD_SET(DSI1_PHYC, TXULPSESC_3, 1);
	case 3:
		regMask |= DSI_REG_FIELD_SET(DSI1_PHYC, TXULPSESC_2, 1);
		regVal |= DSI_REG_FIELD_SET(DSI1_PHYC, TXULPSESC_2, 1);
	case 2:
		regMask |= DSI_REG_FIELD_SET(DSI1_PHYC, TXULPSESC_1, 1);
		regVal |= DSI_REG_FIELD_SET(DSI1_PHYC, TXULPSESC_1, 1);
	case 1:
	default:
		regMask |= DSI_REG_FIELD_SET(DSI1_PHYC, TXULPSESC_0, 1);
		regVal |= DSI_REG_FIELD_SET(DSI1_PHYC, TXULPSESC_0, 1);
	}
#endif

	switch (state) {
	case PHY_TXSTOP:
		regVal |= DSI_REG_FIELD_SET(DSI1_PHYC, FORCE_TXSTOP_0, 1);
		break;
	case PHY_ULPS:
		regVal |= DSI_REG_FIELD_SET(DSI1_PHYC, TXULPSCLK, 1);
		break;
	case PHY_CORE:
	default:
		if (pDev->clkContinuous)
			regVal |= DSI_REG_FIELD_SET(DSI1_PHYC, TX_HSCLK_CONT, 1);
		else
			regVal = 0;
		break;
	}

	DSI_REG_WRITE_MASKED(pDev->baseAddr, DSI1_PHYC, regMask, regVal);
}

/*
 *
 *  Function Name:  chal_dsi_set_afe_off
 *
 *  Description:    Power Down PHY AFE (Analog Front End)
 *
 */
cVoid chal_dsi_phy_afe_off(CHAL_HANDLE handle)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;
	cUInt32 afeVal = 0;
	cUInt32 afeMask = 0;

	afeMask = DSI1_PHY_AFEC0_PD_MASK | DSI1_PHY_AFEC0_PD_BG_MASK;

	afeVal = DSI_REG_FIELD_SET(DSI1_PHY_AFEC0, PD, 1) /* Pwr Down AFE */
	    | DSI_REG_FIELD_SET(DSI1_PHY_AFEC0, PD_BG, 1); /* Pwr Down BG */

	DSI_REG_WRITE_MASKED(pDev->baseAddr, DSI1_PHY_AFEC0, afeMask, afeVal);
}

/*
 *
 *  Function Name:  chal_dsi_phy_afe_on
 *
 *  Description:    Configure & Enable PHY-AFE  (Analog Front End)
 *
 */
cVoid chal_dsi_phy_afe_on(CHAL_HANDLE handle, pCHAL_DSI_AFE_CFG afeCfg)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;
	cUInt32 afeVal = 0;
	cUInt32 afeMask = 0;
	/*cUInt32         i; */

	afeMask = DSI1_PHY_AFEC0_CTATADJ_MASK
	    | DSI1_PHY_AFEC0_PTATADJ_MASK
	    | DSI1_PHY_AFEC0_DDRCLK_EN_MASK
	    | DSI1_PHY_AFEC0_PD_BG_MASK
	    | DSI1_PHY_AFEC0_PD_MASK
	    | DSI1_PHY_AFEC0_RESET_MASK
	    | DSI1_PHY_AFEC0_DDR2CLK_EN_MASK
	    | DSI1_PHY_AFEC0_IDR_DLANE3_MASK
	    | DSI1_PHY_AFEC0_IDR_DLANE2_MASK
	    | DSI1_PHY_AFEC0_IDR_DLANE1_MASK
	    | DSI1_PHY_AFEC0_IDR_DLANE0_MASK | DSI1_PHY_AFEC0_IDR_CLANE_MASK;

	afeVal = DSI_REG_FIELD_SET(DSI1_PHY_AFEC0, CTATADJ, afeCfg->afeCtaAdj)
	    | DSI_REG_FIELD_SET(DSI1_PHY_AFEC0, PTATADJ, afeCfg->afePtaAdj)
	    | DSI_REG_FIELD_SET(DSI1_PHY_AFEC0, IDR_CLANE, afeCfg->afeClkIdr)
	    | DSI_REG_FIELD_SET(DSI1_PHY_AFEC0, IDR_DLANE0, afeCfg->afeDlIdr)
	    | DSI_REG_FIELD_SET(DSI1_PHY_AFEC0, IDR_DLANE1, afeCfg->afeDlIdr)
	    | DSI_REG_FIELD_SET(DSI1_PHY_AFEC0, IDR_DLANE2, afeCfg->afeDlIdr)
	    | DSI_REG_FIELD_SET(DSI1_PHY_AFEC0, IDR_DLANE3, afeCfg->afeDlIdr);

/*    if( !afeCfg->afeBandGapOn ) */
/*        afeVal |= DSI_REG_FIELD_SET( DSI1_PHY_AFEC0, PD_BG, 1 ); */
/* */
/*    if( afeCfg->afeDs2xClkEna ) */
/*        afeVal |= DSI_REG_FIELD_SET( DSI1_PHY_AFEC0, DDRCLK_EN, 1 ); */

	afeVal |= DSI_REG_FIELD_SET(DSI1_PHY_AFEC0, PD_BG, 0);
	afeVal |= DSI_REG_FIELD_SET(DSI1_PHY_AFEC0, PD, 0);
	/* for now, enable all clock outputs */
	afeVal |= DSI_REG_FIELD_SET(DSI1_PHY_AFEC0, DDRCLK_EN, 1);
	afeVal |= DSI_REG_FIELD_SET(DSI1_PHY_AFEC0, DDR2CLK_EN, 1);

	afeVal |= DSI_REG_FIELD_SET(DSI1_PHY_AFEC0, RESET, 1);

	/* PWR-UP & Reset */
	DSI_REG_WRITE_MASKED(pDev->baseAddr, DSI1_PHY_AFEC0, afeMask, afeVal);

	/*for ( i=0; i<100; i++ ) {} */
	CHAL_DELAY_MS(2);

	/* remove reset */
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_PHY_AFEC0, RESET, 0);
}

/*
 *
 * Function Name:  chalDsiTimingDivAndRoundUp
 *
 * Description:    DSI Timing Counters - Divide & RoundUp Utility
 *                 Checks for Counter Value Overflow
 */
static Boolean chalDsiTimingDivAndRoundUp(struct DSI_COUNTER *pDsiC,
					  cUInt32 i,	/* DSI counter index */
					  cUInt32 dividend,
					  cUInt32 divisor)
{
	cUInt32 counter;
	cUInt32 counter_remaindor;

	counter = dividend / divisor;
	counter_remaindor = dividend % divisor;

	if (counter_remaindor)
		counter++;

	if ((counter % pDsiC[i].counter_step) != 0)
		counter += pDsiC[i].counter_step;

	counter = counter & (~(pDsiC[i].counter_step - 1));

	counter -= pDsiC[i].counter_offs;

	pDsiC[i].counter = counter;

	if (counter > pDsiC[i].counter_max) {
		chal_dprintf(CDBG_ERRO,
			     "[cHAL DSI] chalDsiTimingDivAndRoundUp: "
			     "%s counter value overflow\n\r", pDsiC[i].name);
		return FALSE;
	} else {
		return TRUE;
	}
}

/*
 *
 * Function Name:  chal_dsi_set_timing
 *
 * Description:    Calculate Values Of DSI Timing Counters
 *
 *                 <in> escClk_MHz      ESC CLK after divider
 *                 <in> hsBitRate_Mbps  HS Bit Rate ( eq to DSI PLL/dsiPllDiv )
 *                 <in> lpBitRate_Mbps  LP Bit Rate, Max 10 Mbps
 *
 */
cBool chal_dsi_set_timing(CHAL_HANDLE handle,
			  void *phy_timing,
			  CHAL_DSI_CLK_SEL_t coreClkSel,
			  cUInt32 escClk_MHz,
			  cUInt32 hsBitRate_Mbps, cUInt32 lpBitRate_Mbps)
{
	Boolean res = FALSE;

	cUInt32 scaled_time_min;
	cUInt32 scaled_time_min1;
	cUInt32 scaled_time_min2;
	cUInt32 scaled_time_max;
	cUInt32 scaled_period;
	cUInt32 scaled_ui_ns;
	cUInt32 scaled_escClk_ns;
	cUInt32 lp_clk_khz;
	cUInt32 i;
	struct DSI_COUNTER *pDsiC;
	struct CHAL_DSI *pDev;
	cUInt32 counter_offs;
	cUInt32 counter_step;
	cUInt32 lp_lpx_ns;

	pDev = (struct CHAL_DSI *)handle;
	pDsiC = (struct DSI_COUNTER *)phy_timing;
	scaled_ui_ns = (1000 * 1000) / hsBitRate_Mbps;

	scaled_escClk_ns = (1000 * 1000) / escClk_MHz;

	/* figure step & offset for HS counters */
	if (coreClkSel == CHAL_DSI_BIT_CLK_DIV_BY_8) {
		counter_offs = 8;
		counter_step = 8;
	} else if (coreClkSel == CHAL_DSI_BIT_CLK_DIV_BY_4) {
		counter_offs = 4;
		counter_step = 4;
	} else {
		counter_offs = 2;
		counter_step = 2;
	}

	/* init offset & step for HS counters */
	for (i = 1; i < DSI_C_MAX; i++) {
		/* Period_units [ns] */
		if (pDsiC[i].timeBase & DSI_C_TIME_HS) {
			pDsiC[i].counter_offs = counter_offs;
			pDsiC[i].counter_step = counter_step;
		}
	}

	/* LP clk (LP Symbol Data Rate) = esc_clk / esc2lp_ratio */
	/* calculate esc2lp_ratio */
	if (!chalDsiTimingDivAndRoundUp(pDsiC, DSI_C_ESC2LP_RATIO,
					escClk_MHz, lpBitRate_Mbps * 2)) {
		return FALSE;
	}
	/* actual lp clock */
	lp_clk_khz = 1000 * escClk_MHz
	    / (pDsiC[DSI_C_ESC2LP_RATIO].counter
	       + pDsiC[DSI_C_ESC2LP_RATIO].counter_offs);

	/* lp_esc_clk == lp_data_clock */
	lp_lpx_ns = (1000 * 1000 / lp_clk_khz);
	/* set LP LPX to be equal to LP bit rate */

	/* set time_min_ns for LP esc_clk counters */
	pDsiC[DSI_C_LPX].time_min1_ns = pDsiC[DSI_C_LPX].time_lpx * lp_lpx_ns;
	pDsiC[DSI_C_LP_TA_GO].time_min1_ns =
	    pDsiC[DSI_C_LP_TA_GO].time_lpx * lp_lpx_ns;
	pDsiC[DSI_C_LP_TA_SURE].time_min1_ns =
	    pDsiC[DSI_C_LP_TA_SURE].time_lpx * lp_lpx_ns;
	pDsiC[DSI_C_LP_TA_GET].time_min1_ns =
	    pDsiC[DSI_C_LP_TA_GET].time_lpx * lp_lpx_ns;

	/* start from 1, skip [0]=esc2lp_ratio */
	for (i = 1; i < DSI_C_MAX; i++) {
		/* Period_min1 [ns] */
		scaled_time_min1 = pDsiC[i].time_min1_ns * 1000
		    + pDsiC[i].time_min1_ui * scaled_ui_ns;

		/* Period_min2 [ns] */
		if (pDsiC[i].mode & DSI_C_MIN_MAX_OF_2)
			scaled_time_min2 = pDsiC[i].time_min2_ns * 1000
			    + pDsiC[i].time_min2_ui * scaled_ui_ns;
		else
			scaled_time_min2 = 0;

		/* Period_min [ns] = max(min1, min2) */
		if (scaled_time_min1 >= scaled_time_min2)
			scaled_time_min = scaled_time_min1;
		else
			scaled_time_min = scaled_time_min2;

		/* Period_max [ns] */
		if (pDsiC[i].mode & DSI_C_HAS_MAX)
			scaled_time_max = pDsiC[i].time_max_ns * 1000
			    + pDsiC[i].time_max_ui * scaled_ui_ns;
		else
			scaled_time_max = 0;

		/* Period_units [ns] */
		if (pDsiC[i].timeBase & DSI_C_TIME_HS)
			scaled_period = scaled_ui_ns;
		else if (pDsiC[i].timeBase & DSI_C_TIME_ESC)
			scaled_period = scaled_escClk_ns;
		else
			scaled_period = 0;

		pDsiC[i].period = scaled_period;

		if (scaled_period != 0) {
			res =
			    chalDsiTimingDivAndRoundUp(pDsiC, i,
						       scaled_time_min,
						       scaled_period);
			if (!res)
				return res;

			if (pDsiC[i].mode & DSI_C_HAS_MAX) {
				if ((pDsiC[i].counter * scaled_period) >
				    scaled_time_max) {
					chal_dprintf(CDBG_ERRO,
						     "[cHAL DSI] chal_dsi_set_timing: "
						     "%s violates MAX D-PHY Spec allowed value\n\r",
						     pDsiC[i].name);
					return FALSE;
				}
			}
		}
	}

#if 0
	for (i = 0; i < DSI_C_MAX; i++) {
		if (pDsiC[i].timeBase == DSI_C_TIME_ESC2LPDT) {
			chal_dprintf(CDBG_ERRO,
				     "[cHAL DSI] chal_dsi_set_timing: "
				     "%14s %7d => LP clk %u[Mhz]\n\r",
				     pDsiC[i].name, pDsiC[i].counter,
				     escClk_MHz / (pDsiC[i].counter +
						   pDsiC[i].counter_offs));
		} else {
			chal_dprintf(CDBG_ERRO,
				     "[cHAL DSI] chal_dsi_set_timing: "
				     "%14s %7d => %u[ns]\n\r", pDsiC[i].name,
				     pDsiC[i].counter,
				     (pDsiC[i].counter + pDsiC[i].counter_offs)
				     * pDsiC[i].period / 1000);
		}
	}

	chal_dprintf(CDBG_ERRO, "\r\n[cHAL DSI] chal_dsi_set_timing: "
		     "HS_DATA_RATE %u[Mbps]\r\n", hsBitRate_Mbps);

	chal_dprintf(CDBG_ERRO, "[cHAL DSI] chal_dsi_set_timing: "
		     "LP_DATA_RATE %u[kbps]\n\r", lp_clk_khz / 2);
#endif
	/* set ESC 2 LPDT ratio */
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_PHYC,
			     ESC_CLK_LPDT, pDsiC[DSI_C_ESC2LP_RATIO].counter);

	/* HS_DLT5  INIT */
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_HS_DLT5,
			     HS_INIT, pDsiC[DSI_C_HS_INIT].counter);

	/* HS_CLT2  ULPS WakeUp */
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_HS_CLT2,
			     HS_WUP, pDsiC[DSI_C_HS_WAKEUP].counter);
	/* LP_DLT7  ULPS WakeUp */
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_LP_DLT7,
			     LP_WUP, pDsiC[DSI_C_LP_WAKEUP].counter);

	/* HS CLK - HS_CLT0 reg */
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_HS_CLT0,
			     HS_CZERO, pDsiC[DSI_C_HS_CLK_ZERO].counter);
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_HS_CLT0,
			     HS_CPRE, pDsiC[DSI_C_HS_CLK_PRE].counter);
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_HS_CLT0,
			     HS_CPREP, pDsiC[DSI_C_HS_CLK_PREPARE].counter);

	/* HS CLK - HS_CLT1 reg */
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_HS_CLT1,
			     HS_CTRAIL, pDsiC[DSI_C_HS_CLK_TRAIL].counter);
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_HS_CLT1,
			     HS_CPOST, pDsiC[DSI_C_HS_CLK_POST].counter);

	/* HS DATA HS_DLT3 REG */
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_HS_DLT3,
			     HS_EXIT, pDsiC[DSI_C_HS_EXIT].counter);
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_HS_DLT3,
			     HS_ZERO, pDsiC[DSI_C_HS_ZERO].counter);
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_HS_DLT3,
			     HS_PRE, pDsiC[DSI_C_HS_PRE].counter);

	/* HS DATA HS_DLT4 REG */
	/* !!! HS_ANLAT, new in HERA/RHEA, for now init to 0 (DEF) */
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_HS_DLT4, HS_ANLAT, 0);
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_HS_DLT4,
			     HS_TRAIL, pDsiC[DSI_C_HS_TRAIL].counter);
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_HS_DLT4,
			     HS_LPX, pDsiC[DSI_C_HS_LPX].counter);

	/* LP_DLT6 REG */
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_LP_DLT6,
			     TA_GET, pDsiC[DSI_C_LP_TA_GET].counter);
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_LP_DLT6,
			     TA_SURE, pDsiC[DSI_C_LP_TA_SURE].counter);
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_LP_DLT6,
			     TA_GO, pDsiC[DSI_C_LP_TA_GO].counter);
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_LP_DLT6,
			     LPX, pDsiC[DSI_C_LPX].counter);

	return TRUE;
}

/*
 *
 *  Function Name:  chal_dsi_off
 *
 *  Description:    Disable DSI core
 *
 */
cVoid chal_dsi_off(CHAL_HANDLE handle)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;

	/*DSI_PHYC_PHY_CLANE_EN   = 0; */
	/*DSI_PHYC_PHY_DLANE0_EN  = 0; */
	BRCM_WRITE_REG(pDev->baseAddr, DSI1_PHYC, 0);

	/* DSI_CTRL_DSI_EN = 0 */
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_CTRL, DSI_EN, 0);
}

/*
 *
 *  Function Name:  chal_dsi_te_mode
 *
 *  Description:    Set TE SYNC mode
 *
 */
cVoid chal_dsi_te_mode(CHAL_HANDLE handle, CHAL_DSI_TE_MODE_t teMode)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;

	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_CTRL, TE_TRIGC, teMode);
}

/*
 *
 *  Function Name:  chal_dsi_on
 *
 *  Description:
 *
 */
cVoid chal_dsi_on(CHAL_HANDLE handle, pCHAL_DSI_MODE dsiMode)
{

	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;
	cUInt32 mask = 0;
	cUInt32 ctrl = 0;

	/* DSI-CTRL  Configure Mode & enable the core */
	mask = DSI_REG_FIELD_SET(DSI1_CTRL, DISP_CRCC, 1)
	    | DSI_REG_FIELD_SET(DSI1_CTRL, DISP_ECCC, 1)
	    | DSI_REG_FIELD_SET(DSI1_CTRL, RX_LPDT_EOT_EN, 1)
	    | DSI_REG_FIELD_SET(DSI1_CTRL, LPDT_EOT_EN, 1)
	    | DSI_REG_FIELD_SET(DSI1_CTRL, HSDT_EOT_EN, 1)
	    | DSI_REG_FIELD_SET(DSI1_CTRL, DSI_EN, 1)
	    | DSI_REG_FIELD_SET(DSI1_CTRL, CAL_BYTE_EN, 1)
	    | DSI1_CTRL_HS_CLKC_MASK;

	ctrl = dsiMode->clkSel << DSI1_CTRL_HS_CLKC_SHIFT;

	if (dsiMode->clkSel != CHAL_DSI_BIT_CLK_DIV_BY_8)
		ctrl |= DSI_REG_FIELD_SET(DSI1_CTRL, CAL_BYTE_EN, 1);

	if (dsiMode->enaRxCrc)
		ctrl |= DSI_REG_FIELD_SET(DSI1_CTRL, DISP_CRCC, 1);
	if (dsiMode->enaRxEcc)
		ctrl |= DSI_REG_FIELD_SET(DSI1_CTRL, DISP_ECCC, 1);

	if (dsiMode->enaHsTxEotPkt)
		ctrl |= DSI_REG_FIELD_SET(DSI1_CTRL, HSDT_EOT_EN, 1);

	if (dsiMode->enaLpTxEotPkt)
		ctrl |= DSI_REG_FIELD_SET(DSI1_CTRL, LPDT_EOT_EN, 1);

	if (dsiMode->enaLpRxEotPkt)
		ctrl |= DSI_REG_FIELD_SET(DSI1_CTRL, RX_LPDT_EOT_EN, 1);

	ctrl |= DSI_REG_FIELD_SET(DSI1_CTRL, DSI_EN, 1);

	DSI_REG_WRITE_MASKED(pDev->baseAddr, DSI1_CTRL, mask, ctrl);

	/* PHY-C  Configure & Enable D-PHY Interface */
	mask = 0;
	ctrl = 0;

	mask = DSI1_PHYC_TX_HSCLK_CONT_MASK | DSI1_PHYC_PHY_CLANE_EN_MASK;
	switch (pDev->dlCount) {
	case 4:
		mask |= DSI1_PHYC_PHY_DLANE3_EN_MASK;
		ctrl |= DSI_REG_FIELD_SET(DSI1_PHYC, PHY_DLANE3_EN, 1);
	case 3:
		mask |= DSI1_PHYC_PHY_DLANE2_EN_MASK;
		ctrl |= DSI_REG_FIELD_SET(DSI1_PHYC, PHY_DLANE2_EN, 1);
	case 2:
		mask |= DSI1_PHYC_PHY_DLANE1_EN_MASK;
		ctrl |= DSI_REG_FIELD_SET(DSI1_PHYC, PHY_DLANE1_EN, 1);
	case 1:
	default:
		mask |= DSI1_PHYC_PHY_DLANE0_EN_MASK;
		ctrl |= DSI_REG_FIELD_SET(DSI1_PHYC, PHY_DLANE0_EN, 1);
	}

	if (dsiMode->enaContClock)
		ctrl |= DSI_REG_FIELD_SET(DSI1_PHYC, TX_HSCLK_CONT, 1);

	ctrl |= DSI_REG_FIELD_SET(DSI1_PHYC, PHY_CLANE_EN, 1);

	DSI_REG_WRITE_MASKED(pDev->baseAddr, DSI1_PHYC, mask, ctrl);
}

/*
 *
 *  Function Name:  chal_dsi_get_ena_int
 *
 *  Description:    Get enabled DSI Interrupts
 *
 */
cUInt32 chal_dsi_get_ena_int(CHAL_HANDLE handle)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;

	return BRCM_READ_REG(pDev->baseAddr, DSI1_INT_EN);
}

/*
 *
 *  Function Name:  chal_dsi_ena_int
 *
 *  Description:    Enable(b'1b) | Disable(b'0') DSI Interrupts
 *
 */
cVoid chal_dsi_ena_int(CHAL_HANDLE handle, cUInt32 intMask)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;

	BRCM_WRITE_REG(pDev->baseAddr, DSI1_INT_EN, intMask);
}

/*
 *
 *  Function Name:  chal_dsi_get_int
 *
 *  Description:    Get Pending Interrupts
 *
 */
cUInt32 chal_dsi_get_int(CHAL_HANDLE handle)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;

	return BRCM_READ_REG(pDev->baseAddr, DSI1_INT_STAT);
}

/*
 *
 *  Function Name:  chal_dsi_clr_int
 *
 *  Description:    Clear Pending Interrupt
 *
 */
cVoid chal_dsi_clr_int(CHAL_HANDLE handle, cUInt32 intMask)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;

	BRCM_WRITE_REG(pDev->baseAddr, DSI1_INT_STAT, intMask);
}

/*
 *
 *  Function Name:  chal_dsi_clr_fifo
 *
 *  Description:    Clear Sleleceted DSI FIFOs
 *
 */
cVoid chal_dsi_clr_fifo(CHAL_HANDLE handle, cUInt32 fifoMask)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;

	DSI_REG_WRITE_MASKED(pDev->baseAddr, DSI1_CTRL, fifoMask, fifoMask);
}

/*
 *
 *  Function Name:  chal_dsi_get_status
 *
 *  Description:    Int Status is collection of Int event (set until cleared)
 *                  and status (real time state) flags
 *
 */
cUInt32 chal_dsi_get_status(CHAL_HANDLE handle)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;

	return BRCM_READ_REG(pDev->baseAddr, DSI1_STAT);
}

/*
 *
 *  Function Name:  chal_dsi_clr_status
 *
 *  Description:    Int Status is collection of Int event (set until cleared)
 *                  and status (real time state) flags
 *                  1 - clears event bits
 */
cVoid chal_dsi_clr_status(CHAL_HANDLE handle, cUInt32 statMask)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;

	BRCM_WRITE_REG(pDev->baseAddr, DSI1_STAT, statMask);
}

/*
 *
 *  Function Name:  chal_dsi_tx_bta
 *
 *  Description:    Send Bus Turn Around
 *
 */
cVoid chal_dsi_tx_bta(CHAL_HANDLE handle, cUInt8 txEng)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;
	cUInt32 pktc = 0;

	pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_CTRL, CMND_CTRL_BTA);
	pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_REPEAT, 1);
	pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_MODE, 1);
	pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_TX_TIME,
				  CHAL_DSI_CMND_WHEN_BEST_EFFORT);
	pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, DISPLAY_NO,
				  DSI_PKT_SRC_CMND_FIFO);
	pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_EN, 1);

	if (txEng)
		BRCM_WRITE_REG(pDev->baseAddr, DSI1_TXPKT2_C, pktc);
	else
		BRCM_WRITE_REG(pDev->baseAddr, DSI1_TXPKT1_C, pktc);

}

/*
 *
 *  Function Name:  chal_dsi_tx_trig
 *
 *  Description:    Send TRIGGER Message
 *
 */
cVoid chal_dsi_tx_trig(CHAL_HANDLE handle, cUInt8 txEng, cUInt8 trig)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;
	cUInt32 pktc = 0;

	pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_CTRL, CMND_CTRL_TRIG);
	pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_REPEAT, 1);
	pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_MODE, 1);	/* LowPower */

	pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_TX_TIME,
				  CHAL_DSI_CMND_WHEN_BEST_EFFORT);
	pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, DISPLAY_NO,
				  DSI_PKT_SRC_CMND_FIFO);
	pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_EN, 1);
	pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, TRIG_CMD, trig);

	if (txEng)
		BRCM_WRITE_REG(pDev->baseAddr, DSI1_TXPKT2_C, pktc);
	else
		BRCM_WRITE_REG(pDev->baseAddr, DSI1_TXPKT1_C, pktc);

}

/*
 *
 *  Function Name:  chal_dsi_tx_short
 *
 *  Description:    Configure packet send interface for sending SHORT packet.
 *                   o SHORT PACKET Data is read from input PACKET BUFFFER.
 *
 */
CHAL_DSI_RES_t chal_dsi_tx_short(CHAL_HANDLE handle,
				 cUInt8 txEng, pCHAL_DSI_TX_CFG txCfg)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;
	cUInt32 pktc = 0;
	cUInt32 pkth = 0;
	cUInt32 dsi_DT = 0;

	if (txCfg->msgLen > 2)
		return CHAL_DSI_MSG_SIZE;

	if (txCfg->isLP)
		pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_MODE, 1);

	if (txCfg->endWithBta)
		pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_CTRL,
					  CMND_CTRL_CMND_PKTwBTA);
	else
		pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_CTRL,
					  CMND_CTRL_CMND_PKT);

	pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_REPEAT, 1);
	pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_TX_TIME, txCfg->vmWhen);
	pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, DISPLAY_NO,
				  DSI_PKT_SRC_CMND_FIFO);

	if (txCfg->start)
		pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_EN, 1);

	if (txCfg->isTe)
		pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_TE_EN, 1);

	dsi_DT = ((txCfg->vc & 0x00000003) << 6) | txCfg->dsiCmnd;

	/* SHORT MSG IS FIXED IN SIZE (always 2  parms) */

	if (txCfg->msgLen >= 1)
		pkth |= (UInt32)txCfg->msg[0];
	if (txCfg->msgLen == 2)
		pkth |= ((UInt32)txCfg->msg[1] << 8);

	pkth = DSI_REG_FIELD_SET(DSI1_TXPKT1_H, WC_PARAM, pkth);

	pkth |= dsi_DT;

	if (txEng)
		BRCM_WRITE_REG(pDev->baseAddr, DSI1_TXPKT2_H, pkth);
	else
		BRCM_WRITE_REG(pDev->baseAddr, DSI1_TXPKT1_H, pkth);

	if (txEng)
		BRCM_WRITE_REG(pDev->baseAddr, DSI1_TXPKT2_C, pktc);
	else
		BRCM_WRITE_REG(pDev->baseAddr, DSI1_TXPKT1_C, pktc);

	return CHAL_DSI_OK;
}

/*
 *
 *  Function Name:  chal_dsi_tx_long
 *
 *  Description:    Configure packet send interface for sending long
 *                  packets using CMND or CMND & PIXEL FIFO.
 *                  Caller is responsible for handling FIFOs content, input
 *                  PACKET BUFFER in not used by the call.
 *
 */
CHAL_DSI_RES_t chal_dsi_tx_long(CHAL_HANDLE handle,
				cUInt8 txEng, pCHAL_DSI_TX_CFG txCfg)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;
	cUInt32 pktc = 0;
	cUInt32 pkth = 0;
	cUInt32 dsi_DT = 0;
	cUInt32 pfifo_count_b;

	if (txCfg->msgLenCFifo > CHAL_DSI_CMND_FIFO_SIZE_B)
		return CHAL_DSI_MSG_SIZE;

	pfifo_count_b = txCfg->msgLen - txCfg->msgLenCFifo;

	if ((pfifo_count_b > CHAL_DSI_PIXEL_FIFO_SIZE_B)
	    || (pfifo_count_b % 4 != 0)) {
		return CHAL_DSI_MSG_SIZE;
	}

	if (txCfg->isLP)
		pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_MODE, 1);
	/* long */
	pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_TYPE, 1);

	if (txCfg->endWithBta)
		pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_CTRL,
					  CMND_CTRL_CMND_PKTwBTA);
	else
		pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_CTRL,
					  CMND_CTRL_CMND_PKT);

	pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_REPEAT, txCfg->repeat);
	pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_TX_TIME, txCfg->vmWhen);

	if (txCfg->msgLen == txCfg->msgLenCFifo)
		pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, DISPLAY_NO,
					  DSI_PKT_SRC_CMND_FIFO);
	else if (txCfg->dispEngine == 1)
		pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, DISPLAY_NO,
					  DSI_PKT_SRC_DE1);
	else
		pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, DISPLAY_NO,
					  DSI_PKT_SRC_DE0);

	if (txCfg->isTe)
		pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_TE_EN, 1);

	if (txCfg->start)
		pktc |= DSI_REG_FIELD_SET(DSI1_TXPKT1_C, CMD_EN, 1);

	dsi_DT = ((txCfg->vc & 0x00000003) << 6) | txCfg->dsiCmnd;

	pkth = DSI_REG_FIELD_SET(DSI1_TXPKT1_H, WC_CDFIFO, txCfg->msgLenCFifo)
	    | DSI_REG_FIELD_SET(DSI1_TXPKT1_H, WC_PARAM, txCfg->msgLen);

	pkth |= dsi_DT;

	if (txEng)
		BRCM_WRITE_REG(pDev->baseAddr, DSI1_TXPKT2_H, pkth);
	else
		BRCM_WRITE_REG(pDev->baseAddr, DSI1_TXPKT1_H, pkth);


/*  chal_dprintf ( CDBG_ERRO, "[cHAL DSI] %s: PKTC[0x%08X] PKTH[0x%08X]\n", */
/*      __FUNCTION__, pktc  , pktc  ); */

	if (txEng)
		BRCM_WRITE_REG(pDev->baseAddr, DSI1_TXPKT2_C, pktc);
	else
		BRCM_WRITE_REG(pDev->baseAddr, DSI1_TXPKT1_C, pktc);

	return CHAL_DSI_OK;
}

/*
 *
 *  Function Name:  chal_dsi_tx_start
 *
 *  Description:    ENABLE | DISABLE  Command Interface
 *
 */
cVoid chal_dsi_tx_start(CHAL_HANDLE handle, cUInt8 txEng, cBool start)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;

	if (start)
		if (txEng)
			BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_TXPKT2_C,
					     CMD_EN, 1);
		else
			BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_TXPKT1_C,
					     CMD_EN, 1);
	else if (txEng)
		BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_TXPKT2_C, CMD_EN, 0);
	else
		BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_TXPKT1_C, CMD_EN, 0);
}

/*
 *
 *  Function Name:  chal_dsi_wr_cfifo
 *
 *  Description:    Write to CMND FIFO
 *                  o Caller reponsible for taking care of not overflowing the
 *                    FIFO if function called more than once
 *
 */
CHAL_DSI_RES_t chal_dsi_wr_cfifo(CHAL_HANDLE handle,
				 cUInt8 *pBuff, UInt32 byte_count)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;

	if (byte_count > CHAL_DSI_CMND_FIFO_SIZE_B)
		return CHAL_DSI_MSG_SIZE;

	while (byte_count--)
		BRCM_WRITE_REG(pDev->baseAddr, DSI1_TXPKT_CMD_FIFO, *pBuff++);

	return CHAL_DSI_OK;
}

/*
 *
 *  Function Name:  chal_dsi_wr_pfifo_be
 *
 *  Description:    Write to PIXEL FIFO
 *                  o No of bytes to be written MUST BE multiple of 4s
 *                  o DISP1 ENA & INPUT 'COLOR MODE == BE' MUST be set
 *                    prior to call
 *                  o BE setting has nothing to do with SYSTEM mem endianes
 *                  o To be used for sending command packets > CMND FIFO
 *                  o Caller reponsible for taking care of not overflowing the
 *                    FIFO if function called more than once
 */
CHAL_DSI_RES_t chal_dsi_wr_pfifo_be(CHAL_HANDLE handle,
				    cUInt8 *pBuff, UInt32 byte_count)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;
	UInt32 word_count;
	UInt32 word;

	if (byte_count > CHAL_DSI_PIXEL_FIFO_SIZE_B)
		return CHAL_DSI_MSG_SIZE;

	if (byte_count % 4)
		return CHAL_DSI_MSG_SIZE;

	word_count = byte_count >> 2;
	while (word_count--) {
		word =
		    pBuff[0] << 24 | pBuff[1] << 16 | pBuff[2] << 8 | pBuff[3];
		BRCM_WRITE_REG(pDev->baseAddr, DSI1_TXPKT_PIXD_FIFO, word);
		pBuff += 4;
	}

	return CHAL_DSI_OK;
}

/*
 *
 *  Function Name:  chal_dsi_read_reply
 *
 *  Description:
 *
 */

CHAL_DSI_RES_t chal_dsi_read_reply(CHAL_HANDLE handle,
	cUInt32 event,	/* DSI core event flags */
	pCHAL_DSI_REPLY reply)
{
#define	DSI_DT_P2H_SH_ACK_ERR_RPT	(0x02)
#define	DSI_DT_P2H_SH_GEN_RD_1B	    (0x11)
#define	DSI_DT_P2H_DCS_SH_RD_1B	    (0x21)

	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;
	cUInt32 rxStat;
	cUInt32 pkth;
	cUInt32 size;
	cUInt32 i;
	cUInt32 replyDT;

	reply->type = 0;

	if ((event & DSI1_STAT_RX1_PKT_MASK)
	    || (event & DSI1_STAT_PHY_RXTRIG_MASK)) {
		pkth = BRCM_READ_REG(pDev->baseAddr, DSI1_RXPKT1_H);

		if (event & DSI1_STAT_PHY_RXTRIG_MASK) {
			reply->type = RX_TYPE_TRIG;
			reply->trigger = pkth & DSI1_RXPKT1_H_DT_LP_CMD_MASK;
		} else {
			rxStat = 0;
			replyDT = pkth & DSI1_RXPKT1_H_DT_LP_CMD_MASK;

			/* either DATA REPLY or ERROR STAT */
			if ((replyDT & 0x3F) == DSI_DT_P2H_SH_ACK_ERR_RPT) {
				/* ERROR STAT */
				reply->type |= RX_TYPE_ERR_REPLY;
				reply->errReportDt = replyDT;

				if (pkth & DSI1_RXPKT1_H_DET_ERR_MASK)
					rxStat |= ERR_RX_MULTI_BIT;
				if (pkth & DSI1_RXPKT1_H_ECC_ERR_MASK)
					rxStat |= ERR_RX_ECC;
				if (pkth & DSI1_RXPKT1_H_COR_ERR_MASK)
					rxStat |= ERR_RX_CORRECTABLE;
				if (pkth & DSI1_RXPKT1_H_INCOMP_PKT_MASK)
					rxStat |= ERR_RX_PKT_INCOMPLETE;

				reply->errReportRxStat = rxStat;

				if ((rxStat == 0)
				    || (rxStat == ERR_RX_CORRECTABLE))
					reply->errReportRxStat |= ERR_RX_OK;

				/* ALWAYS SHORT - NO CRC */
				reply->errReport =
				    (pkth & DSI1_RXPKT1_H_WC_PARAM_MASK) >>
				    DSI1_RXPKT1_H_WC_PARAM_SHIFT;
			} else {
				/* DATA REPLY */
				reply->type = RX_TYPE_READ_REPLY;
#if 0
				reply->readReplyDt =
					pkth & DSI_RX1_PKTH_DT_LP_CMD_MASK;
#endif
				reply->readReplyDt = replyDT;

				if (pkth & DSI1_RXPKT1_H_CRC_ERR_MASK)
					rxStat |= ERR_RX_CRC;
				if (pkth & DSI1_RXPKT1_H_DET_ERR_MASK)
					rxStat |= ERR_RX_MULTI_BIT;
				if (pkth & DSI1_RXPKT1_H_ECC_ERR_MASK)
					rxStat |= ERR_RX_ECC;
				if (pkth & DSI1_RXPKT1_H_COR_ERR_MASK)
					rxStat |= ERR_RX_CORRECTABLE;
				if (pkth & DSI1_RXPKT1_H_INCOMP_PKT_MASK)
					rxStat |= ERR_RX_PKT_INCOMPLETE;

				reply->readReplyRxStat = rxStat;
				if ((rxStat == 0)
				    || (rxStat == ERR_RX_CORRECTABLE))
					reply->readReplyRxStat |= ERR_RX_OK;

				if (pkth & DSI1_RXPKT1_H_PKT_TYPE_MASK) {
					/* LONG, MAX 8 bytes of parms */
					size =
					    (pkth & DSI1_RXPKT1_H_WC_PARAM_MASK)
					    >> 8;
					for (i = 0; i < size; i++) {
						reply->pReadReply[i] =
						    BRCM_READ_REG(pDev->
							baseAddr,
							DSI1_RXPKT_FIFO);

					}
					reply->readReplySize = size;
				} else {
					/* SHORT  PKT - MAX 2 BYTEs, NO CRC */
					reply->pReadReply[0] =
					    (pkth & DSI1_RXPKT1_H_WC_PARAM_MASK)
					    >> 8;
					reply->pReadReply[1] =
					    (pkth & DSI1_RXPKT1_H_WC_PARAM_MASK)
					    >> 16;
					if (((reply->readReplyDt & 0x3F) ==
					     DSI_DT_P2H_SH_GEN_RD_1B)
					    || ((reply->readReplyDt & 0x3F) ==
						DSI_DT_P2H_DCS_SH_RD_1B)) {
						reply->readReplySize = 1;
					} else {
						reply->readReplySize = 2;
					}
				}
			}
		}
	}

	if (event & DSI1_STAT_RX2_PKT_MASK) {
		/* we can only get ERROR STAT here */
		reply->type |= RX_TYPE_ERR_REPLY;

		pkth = BRCM_READ_REG(pDev->baseAddr, DSI1_RXPKT2_H);

		rxStat = 0;

		if (pkth & DSI1_RXPKT2_H_DET_ERR_MASK)
			rxStat |= ERR_RX_MULTI_BIT;
		if (pkth & DSI1_RXPKT2_H_ECC_ERR_MASK)
			rxStat |= ERR_RX_ECC;
		if (pkth & DSI1_RXPKT2_H_COR_ERR_MASK)
			rxStat |= ERR_RX_CORRECTABLE;
		if (pkth & DSI1_RXPKT2_H_INCOMP_PKT_MASK)
			rxStat |= ERR_RX_PKT_INCOMPLETE;

		reply->errReportRxStat = rxStat;

		if ((rxStat == 0) || (rxStat == ERR_RX_CORRECTABLE))
			reply->errReportRxStat |= ERR_RX_OK;

		reply->errReportDt = pkth & DSI1_RXPKT2_H_DT_MASK;
		/* ALWAYS SHORT - NO CRC */
		reply->errReport = (pkth & DSI1_RXPKT2_H_PARAM_MASK) >>
		    DSI1_RXPKT2_H_PARAM_SHIFT;
	}

	if (reply->type == 0)
		return CHAL_DSI_RX_NO_PKT;
	else
		return CHAL_DSI_OK;
}

/*
 *
 *  Function Name:  chal_dsi_de1_get_dma_address
 *
 *  Description:
 *
 */
UInt32 chal_dsi_de1_get_dma_address(CHAL_HANDLE handle)
{
	struct CHAL_DSI *dsiH = (struct CHAL_DSI *)handle;

	return BRCM_REGADDR(dsiH->baseAddr, DSI1_TXPKT_PIXD_FIFO);
}

/*
 *
 *  Function Name:  chal_dsi_de1_set_dma_thresh
 *
 *  Description:
 *
 */
cVoid chal_dsi_de1_set_dma_thresh(CHAL_HANDLE handle, cUInt32 thresh)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;

	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_DISP1_CTRL, DMA_THRESH,
			     thresh);
}

/*
 *
 *  Function Name:  chal_dsi_de1_set_wc
 *
 *  Description:
 *
 *
 */
cVoid chal_dsi_de1_set_wc(CHAL_HANDLE handle, cUInt32 wc)
{
	/* NA to HERA/RHEA */
}

/*
 *
 *  Function Name:  chal_dsi_de1_set_cm
 *
 *  Description:
 *
 */
cVoid chal_dsi_de1_set_cm(CHAL_HANDLE handle, CHAL_DSI_DE1_COL_MOD_t cm)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;

	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_DISP1_CTRL, PFORMAT, cm);
}

/*
 *
 *  Function Name:  chal_dsi_de1_enable
 *
 *  Description:    Ena | Dis Color Engine 1
 *
 */
cVoid chal_dsi_de1_enable(CHAL_HANDLE handle, cBool ena)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;

	if (ena)
		BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_DISP1_CTRL, EN, 1);
	else
		BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_DISP1_CTRL, EN, 0);
}


/*
 *
 *  Function Name:  chal_dsi_de0_set_cm
 *
 *  Description:
 *
 */
cVoid chal_dsi_de0_set_cm(CHAL_HANDLE handle, CHAL_DSI_DE0_COL_MOD_t cm)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;

	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_DISP0_CTRL, PFORMAT, cm);
}

/*
 *
 *  Function Name:  chal_dsi_de0_set_pix_clk_div
 *
 *  Description:    Set pixel clock divider
 *
 */
cVoid chal_dsi_de0_set_pix_clk_div(CHAL_HANDLE handle, cUInt32 div)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;

	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_DISP0_CTRL, PIX_CLK_DIV, div);
}

/*
 *
 *  Function Name:  chal_dsi_de1_enable
 *
 *  Description:    Ena | Dis Color Engine 0
 *
 */
cVoid chal_dsi_de0_enable(CHAL_HANDLE handle, cBool ena)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;

	if (ena)
		BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_DISP0_CTRL, EN, 1);
	else
		BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_DISP0_CTRL, EN, 0);
}

/*
 *
 *  Function Name:  chal_dsi_de0_set_mode
 *
 *  Description:
 *
 */
cVoid chal_dsi_de0_set_mode(CHAL_HANDLE handle, CHAL_DSI_DE0_MODE_t mode)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;

	BRCM_WRITE_REG_FIELD(pDev->baseAddr, DSI1_DISP0_CTRL, MODE, mode);
}

/*
 *
 *  Function Name:  chal_dsi_de0_st_end
 *
 *  Description:
 *
 */
cVoid chal_dsi_de0_st_end(CHAL_HANDLE handle, cBool ena)
{
	struct CHAL_DSI *pDev = (struct CHAL_DSI *)handle;

	if (ena)
		BRCM_WRITE_REG_FIELD(pDev->baseAddr,
					DSI1_DISP0_CTRL, ST_END, 1);
	else
		BRCM_WRITE_REG_FIELD(pDev->baseAddr,
					DSI1_DISP0_CTRL, ST_END, 0);
}

