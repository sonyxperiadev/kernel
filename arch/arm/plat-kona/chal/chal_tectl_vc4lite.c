/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*             @file     arch/arm/plat-kona/chal/chal_tectl_vc4lite.c
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

#include "plat/chal/chal_types.h"
#include "plat/chal/chal_common.h"
#include "plat/chal/chal_tectl_vc4lite.h"
#include "mach/rdb/brcm_rdb_te.h"
#include "mach/rdb/brcm_rdb_util.h"
#include "mach/hardware.h"

#define TECTL_ADDR   (KONA_MM_TECTL_VA)

/* SET REGISTER BIT FIELD; VALUE IS 0 BASED */
#define TECTL_REG_FIELD_SET(r, f, d) \
	(((BRCM_REGTYPE(r))(d) << BRCM_FIELDSHIFT(r, f)) \
	& BRCM_FIELDMASK(r, f))

/* SET REGISTER BITS WITH MASK */
#define TECTL_WRITE_REG_MASKED(b, r, m, d) \
	(BRCM_WRITE_REG(b, r, (BRCM_READ_REG(b, r) & (~m)) | d))

/*
 * Local Variables
 */

/*
 *
 * Function Name:  chal_te_set_mux_out
 *
 * Description:    TECTL MUX Output Configuration
 *                 Route Selected Chip TE Input To LCD Controller TE input
 *
 */
cInt32 chal_te_set_mux_out(cUInt32 teIn, cUInt32 teOut)
{
	cUInt32 mask;
	cUInt32 value = (cUInt32) teIn;

	switch (teOut) {
	case TE_VC4L_OUT_SPI:
		mask = TE_MUX_CTL_SPI_TE_TRIG_MASK;
		value <<= TE_MUX_CTL_SPI_TE_TRIG_SHIFT;
		break;
	case TE_VC4L_OUT_SMI:
		mask = TE_MUX_CTL_SMI_TE_TRIG_MASK;
		value <<= TE_MUX_CTL_SMI_TE_TRIG_SHIFT;
		break;
	case TE_VC4L_OUT_DSI0_TE0:
		mask = TE_MUX_CTL_DSI0_TE0_TRIG_MASK;
		value <<= TE_MUX_CTL_DSI0_TE0_TRIG_SHIFT;
		break;
	case TE_VC4L_OUT_DSI0_TE1:
		mask = TE_MUX_CTL_DSI0_TE1_TRIG_MASK;
		value <<= TE_MUX_CTL_DSI0_TE1_TRIG_SHIFT;
		break;
	case TE_VC4L_OUT_DSI1_TE0:
		mask = TE_MUX_CTL_DSI1_TE0_TRIG_MASK;
		value <<= TE_MUX_CTL_DSI1_TE0_TRIG_SHIFT;
		break;
	case TE_VC4L_OUT_DSI1_TE1:
		mask = TE_MUX_CTL_DSI1_TE1_TRIG_MASK;
		value <<= TE_MUX_CTL_DSI1_TE1_TRIG_SHIFT;
		break;
	default:
		chal_dprintf(CDBG_ERRO, "chal_te_set_mux_out: "
			     "Invalid MUX Output[%d]\n", (int)teOut);
		return -1;
	}

	TECTL_WRITE_REG_MASKED(TECTL_ADDR, TE_MUX_CTL, mask, value);
	return 0;
}

/*
 *
 * Function Name:  chal_te_cfg_input
 *
 * Description:    Configure & Enable TE Input
 *
 */
cInt32 chal_te_cfg_input(UInt32 teIn, pCHAL_TE_CFG teCfg)
{

#define TE_VSWIDTH_MAX   (TE_0VSWIDTH_TE_VSWIDTH_MASK >> \
	TE_0VSWIDTH_TE_VSWIDTH_SHIFT)
#define TE_HSLINE_MAX    (TE_0C_HSLINE_MASK >> TE_0C_HSLINE_SHIFT)

	cUInt32 te_ctrl_reg_val = 0;
	cUInt32 te_ctrl_reg_mask = 0;

	te_ctrl_reg_mask |= TE_0C_MODE_MASK
	    | TE_0C_HSLINE_MASK | TE_0C_POL_MASK | TE_0C_TE_EN_MASK;

	if (teCfg->sync_pol == TE_VC4L_ACT_POL_HI)
		te_ctrl_reg_val |= TE_0C_POL_MASK;

	if (teCfg->te_mode == TE_VC4L_MODE_VSYNC_HSYNC) {
		te_ctrl_reg_val |= TE_0C_MODE_MASK;

		if (teCfg->vsync_width > TE_VSWIDTH_MAX) {
			chal_dprintf(CDBG_ERRO, "chal_te_cfg_input: "
				     "VSYNC Width Value[0x%08X] Overflow, Max[0x%08X]\n",
				     (unsigned int)teCfg->vsync_width,
				     TE_VSWIDTH_MAX);
			return -1;
		}
		if (teCfg->hsync_line > TE_HSLINE_MAX) {
			chal_dprintf(CDBG_ERRO, "chal_te_cfg_input: "
				     "HSYNC Line Value[0x%08X] Overflow, Max[0x%08X]\n",
				     (unsigned int)teCfg->hsync_line,
				     TE_HSLINE_MAX);
			return -1;
		}

		te_ctrl_reg_val |= (teCfg->hsync_line << TE_0C_HSLINE_SHIFT);
	}

	te_ctrl_reg_val |= TE_0C_TE_EN_MASK;

	switch (teIn) {
	case TE_VC4L_IN_0:
		BRCM_WRITE_REG(TECTL_ADDR, TE_0VSWIDTH, teCfg->vsync_width);
		BRCM_WRITE_REG(TECTL_ADDR, TE_0C, te_ctrl_reg_val);
		break;
	case TE_VC4L_IN_1:
		BRCM_WRITE_REG(TECTL_ADDR, TE_1VSWIDTH, teCfg->vsync_width);
		BRCM_WRITE_REG(TECTL_ADDR, TE_1C, te_ctrl_reg_val);
		break;
	case TE_VC4L_IN_2:
		BRCM_WRITE_REG(TECTL_ADDR, TE_2VSWIDTH, teCfg->vsync_width);
		BRCM_WRITE_REG(TECTL_ADDR, TE_2C, te_ctrl_reg_val);
		break;
	default:
		chal_dprintf(CDBG_ERRO, "chal_te_cfg_input: "
			     "Invalid TE Input[%d]\n", (int)teIn);
		return -1;
	}

	return 0;
}
