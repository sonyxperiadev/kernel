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
*  @file   chal_smi.c
*
*  @brief  HERA SMI Controller cHAL source code file.
*
*  @note
*
****************************************************************************/

#include <plat/chal/chal_common.h>
#include <plat/chal/chal_smi.h>
#include <mach/rdb/brcm_rdb_smi.h>
#include <mach/rdb/brcm_rdb_util.h>

#define CHAL_SMI_WR_STROBE_MAX  127
#define CHAL_SMI_WR_PACE_MAX    127
#define CHAL_SMI_WR_HOLD_MAX    63
#define CHAL_SMI_WR_SETUP_MAX   63

#define CHAL_SMI_RD_STROBE_MAX  127
#define CHAL_SMI_RD_PACE_MAX    127
#define CHAL_SMI_RD_HOLD_MAX    63
#define CHAL_SMI_RD_SETUP_MAX   63

#define CHAL_SMI_BANK0          0
#define CHAL_SMI_BANK1          1
#define CHAL_SMI_BANK2          2
#define CHAL_SMI_BANK3          3

/*SET BIT FIELD USING RDB DEFINITIONs; VALUE IS 0 BASED  */
#define SMI_REG_FIELD_SET(r, f, d) \
	(((BRCM_REGTYPE(r))(d) << BRCM_FIELDSHIFT(r, f)) \
	& BRCM_FIELDMASK(r, f))

/*SETs REGISTER BITs Defined WITH MASK */
#define SMI_WRITE_REG_MASKED(b, r, m, d) \
	(BRCM_WRITE_REG(b, r, (BRCM_READ_REG(b, r) & (~m)) | d))

/*
 * Local Variables
 */
typedef struct {
	cBool init;
	cUInt32 baseAddr;
} CHAL_SMI_T, *pCHAL_SMI_T;

static CHAL_SMI_T smiDev[1];

/*
 *
 * Function Name: smi_calc_smil_len
 *
 * Description:   Calculates number of SMI bus transcations for programmed mode
 *
 */
cUInt32 chal_smi_calc_smil_len(cUInt32 bytes, /* FIFO transfer size in bytes */
			       CHAL_SMI_COL_MODE_T in_col_mode,
			       CHAL_SMI_BUS_WIDTH_T bus_width)
{
	cUInt32 smilL = 0;

	switch (in_col_mode) {
	case CHAL_SMI_CM_16B_RGB565:
		switch (bus_width) {
		case CHAL_SMI_BUSW_08B:
			smilL = bytes;
			break;
		case CHAL_SMI_BUSW_09B:
		case CHAL_SMI_BUSW_16B:
		case CHAL_SMI_BUSW_18B:
			smilL = bytes >> 1;
			break;
		default:
			break;
		}
		break;
	case CHAL_SMI_CM_32B_RGB888:
		switch (bus_width) {
		case CHAL_SMI_BUSW_08B:
			smilL = (bytes * 3) >> 2;
			break;
		case CHAL_SMI_BUSW_09B:
			smilL = bytes >> 1;
			break;
/*              case CHAL_SMI_BUSW_16B :  smilL = (bytes*3)>>2; break; */
		case CHAL_SMI_BUSW_16B:
			smilL = (bytes * 3) >> 3;
			break;
		case CHAL_SMI_BUSW_18B:
			smilL = bytes;
			break;
		default:
			break;
		}
		break;
	/* case CHAL_SMI_CM_NAND: */
	case CHAL_SMI_CM_NONE:
		/* ACCORDING TO THIS IMPLEMENTATION DIRECT MODE MEMORY 32-BIT */
		/* WORD ORGANIZATION IS  */
		/*     8-bit bus     33333333 22222222 11111111 00000000 */
		/*     9-bit bus     xxxxxxx1 11111111 xxxxxxx0 00000000 */
		/*    16-bit bus     11111111 11111111 00000000 00000000 */
		/*    18-bit bus     xxxxxxxx xxxxxx00 00000000 00000000 */
		switch (bus_width) {
		case CHAL_SMI_BUSW_08B:
			smilL = bytes;
			break;
		case CHAL_SMI_BUSW_09B:
			smilL = bytes >> 1;
			break;
		case CHAL_SMI_BUSW_16B:
			smilL = bytes >> 1;
			break;
		case CHAL_SMI_BUSW_18B:
			smilL = bytes >> 2;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return smilL;
}

/*
 *
 * Function Name: chal_smi_init
 *
 * Description:   Initialize LCDC Parallel (Z80/M68 & DBI) Controller
 *                and software interface
 *
 */
CHAL_HANDLE chal_smi_init(cUInt32 baseAddr)
{
	pCHAL_SMI_T pDev;

	chal_dprintf(CDBG_INFO, "chal_smi_init\n");

	pDev = (pCHAL_SMI_T)&smiDev[0];

	if (!pDev->init) {
		pDev->baseAddr = baseAddr;
		pDev->init = TRUE;
	}

	return (CHAL_HANDLE)pDev;
}

/*
 *
 * Function Name: chal_smi_setup_mode
 *
 * Description:   Setup SMI Mode
 *                Will Disable DIRECT & FIFO MODE
 *
 */
void chal_smi_setup_mode(CHAL_HANDLE handle, cUInt32 bank, pCHAL_SMI_MODE mode)
{
	pCHAL_SMI_T pDev = (pCHAL_SMI_T)handle;

	cUInt32 valueRd = 0;
	cUInt32 maskRd = 0;
	cUInt32 valueWr = 0;
	cUInt32 maskWr = 0;

	/* disable Programmed mode interface */
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, SMI_CS, ENABLE, 0);
	/* disable Direct mode interface */
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, SMI_DCS, EANBLE, 0);

	maskRd |= SMI_DSR0_MODE68_MASK | SMI_DSR0_FSETUP_MASK;
	if (mode->m68)
		valueRd |= SMI_DSR0_MODE68_MASK;
	if (mode->setupFirstTrasferOnly)
		valueRd |= SMI_DSR0_FSETUP_MASK;

	maskWr |= SMI_DSW0_WFORMAT_MASK | SMI_DSW0_WSWAP_MASK;
	if (mode->swap)
		valueWr |= SMI_DSW0_WSWAP_MASK;
	if (mode->inPixelMode == CHAL_SMI_CM_32B_RGB888)
		valueWr |= SMI_DSW0_WFORMAT_MASK;

	switch (bank) {
	case CHAL_SMI_BANK0:
		SMI_WRITE_REG_MASKED(pDev->baseAddr, SMI_DSR0, maskRd, valueRd);
		SMI_WRITE_REG_MASKED(pDev->baseAddr, SMI_DSW0, maskWr, valueWr);
		break;
	case CHAL_SMI_BANK1:
		SMI_WRITE_REG_MASKED(pDev->baseAddr, SMI_DSR1, maskRd, valueRd);
		SMI_WRITE_REG_MASKED(pDev->baseAddr, SMI_DSW1, maskWr, valueWr);
		break;
	case CHAL_SMI_BANK2:
		SMI_WRITE_REG_MASKED(pDev->baseAddr, SMI_DSR2, maskRd, valueRd);
		SMI_WRITE_REG_MASKED(pDev->baseAddr, SMI_DSW2, maskWr, valueWr);
		break;
	case CHAL_SMI_BANK3:
		SMI_WRITE_REG_MASKED(pDev->baseAddr, SMI_DSR3, maskRd, valueRd);
		SMI_WRITE_REG_MASKED(pDev->baseAddr, SMI_DSW3, maskWr, valueWr);
		break;
	default:
		break;
	}
}				/* chal_smi_setup_mode */

/*
 *
 * Function Name: chal_smi_setup_int
 *
 * Description:   Enable | Disable SMI PROG MODE Interrupts
 *                intmask - interrupt events to deal with
 *                intVal  - 0=DIS 1=ENA
 */
void chal_smi_setup_int(CHAL_HANDLE handle, cUInt32 intMask, cUInt32 intVal)
{
	pCHAL_SMI_T pDev = (pCHAL_SMI_T)handle;

	SMI_WRITE_REG_MASKED(pDev->baseAddr, SMI_CS, intMask, intVal);
}

/*
 *
 * Function Name: chal_smi_setup_rd_timing
 *
 * Description:   Set-up RD Timing
 *                Will Disable DIRECT & FIFO MODE
 *
 */
void chal_smi_setup_rd_timing(CHAL_HANDLE handle,
			      cUInt32 bank, pCHAL_SMI_TIMING timing)
{
	pCHAL_SMI_T pDev = (pCHAL_SMI_T)handle;
	cUInt32 mask = 0;
	cUInt32 value = 0;

	mask = SMI_DSR0_RDREQ_MASK /* for now, not an option, hardcoded to 0 */
	    | SMI_DSR0_RWIDTH_MASK
	    | SMI_DSR0_RPACEALL_MASK
	    | SMI_DSR0_RHOLD_MASK
	    | SMI_DSR0_RPACE_MASK
	    | SMI_DSR0_RSTROBE_MASK | SMI_DSR0_RSETUP_MASK;
	/*SMI_DSR0_MODE68_MASK    // handled from setup */
	/*SMI_DSR0_FSETUP_MASK    // handled from setup */

	if (timing->paceAll)
		value |= SMI_DSR0_RPACEALL_MASK;

	value |= (timing->busWidth << SMI_DSR0_RWIDTH_SHIFT);
	value |= (timing->setup << SMI_DSR0_RSETUP_SHIFT);
	value |= (timing->hold << SMI_DSR0_RHOLD_SHIFT);
	value |= (timing->pace << SMI_DSR0_RPACE_SHIFT);
	value |= (timing->strobe << SMI_DSR0_RSTROBE_SHIFT);

	/* disable Programmed mode interface */
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, SMI_CS, ENABLE, 0);
	/* disable Direct mode interface */
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, SMI_DCS, EANBLE, 0);

	switch (bank) {
	case CHAL_SMI_BANK0:
		SMI_WRITE_REG_MASKED(pDev->baseAddr, SMI_DSR0, mask, value);
		break;
	case CHAL_SMI_BANK1:
		SMI_WRITE_REG_MASKED(pDev->baseAddr, SMI_DSR1, mask, value);
		break;
	case CHAL_SMI_BANK2:
		SMI_WRITE_REG_MASKED(pDev->baseAddr, SMI_DSR2, mask, value);
		break;
	case CHAL_SMI_BANK3:
		SMI_WRITE_REG_MASKED(pDev->baseAddr, SMI_DSR3, mask, value);
		break;
	default:
		break;
	}
}				/* chal_smi_setup_rd_timing */

/*
 *
 * Function Name: chal_smi_setup_wr_timing
 *
 * Description:   Set-up WR Timing
 *                Will Disable DIRECT & FIFO MODE
 *
 */
void chal_smi_setup_wr_timing(CHAL_HANDLE handle,
			      cUInt32 bank, pCHAL_SMI_TIMING timing)
{
	pCHAL_SMI_T pDev = (pCHAL_SMI_T)handle;
	cUInt32 mask = 0;
	cUInt32 value = 0;
	CHAL_SMI_TIMING_T calcT;

	calcT = *timing;

	if (calcT.setup > CHAL_SMI_RD_SETUP_MAX) {
		chal_dprintf(CDBG_ERRO, "chal_smi_wr: SETUP cnt OVF, "
			     "using MAX[%d]\n", CHAL_SMI_RD_SETUP_MAX);
		calcT.setup = CHAL_SMI_RD_SETUP_MAX;
	}
	if (calcT.hold > CHAL_SMI_RD_HOLD_MAX) {
		chal_dprintf(CDBG_ERRO, "chal_smi_wr: HOLD cnt OVF, "
			     "using MAX[%d]\n", CHAL_SMI_RD_HOLD_MAX);
		calcT.hold = CHAL_SMI_RD_HOLD_MAX;
	}
	if (calcT.pace > CHAL_SMI_RD_PACE_MAX) {
		chal_dprintf(CDBG_ERRO, "chal_smi_wr: PACE cnt OVF, "
			     "using MAX[%d]\n", CHAL_SMI_RD_PACE_MAX);
		calcT.pace = CHAL_SMI_RD_PACE_MAX;
	}
	if (calcT.strobe > CHAL_SMI_RD_STROBE_MAX) {
		chal_dprintf(CDBG_ERRO, "chal_smi_wr: STROBE cnt OVF, "
			     "using MAX[%d]\n", CHAL_SMI_RD_STROBE_MAX);
		calcT.strobe = CHAL_SMI_RD_STROBE_MAX;
	}

	mask = SMI_DSW0_WDREQ_MASK /* for now, not an option, hardcoded to 0 */
	    | SMI_DSW0_WWIDTH_MASK
	    | SMI_DSW0_WPACEALL_MASK
	    | SMI_DSW0_WSETUP_MASK
	    | SMI_DSW0_WHOLD_MASK | SMI_DSW0_WPACE_MASK | SMI_DSW0_WSTROBE_MASK;
	/* SMI_DSW0_WSWAP_MASK handled from setup */
	/* SMI_DSW0_WFORMAT_MASK handled from setup */

	if (timing->paceAll)
		value |= SMI_DSW0_WPACEALL_MASK;

	value |= (timing->busWidth << SMI_DSW0_WWIDTH_SHIFT);
	value |= (calcT.setup << SMI_DSW0_WSETUP_SHIFT);
	value |= (calcT.hold << SMI_DSW0_WHOLD_SHIFT);
	value |= (calcT.pace << SMI_DSW0_WPACE_SHIFT);
	value |= (calcT.strobe << SMI_DSW0_WSTROBE_SHIFT);

	/* disable Programmed mode interface */
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, SMI_CS, ENABLE, 0);
	/* disable Direct mode interface */
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, SMI_DCS, EANBLE, 0);

	switch (bank) {
	case CHAL_SMI_BANK0:
		SMI_WRITE_REG_MASKED(pDev->baseAddr, SMI_DSW0, mask, value);
		break;
	case CHAL_SMI_BANK1:
		SMI_WRITE_REG_MASKED(pDev->baseAddr, SMI_DSW1, mask, value);
		break;
	case CHAL_SMI_BANK2:
		SMI_WRITE_REG_MASKED(pDev->baseAddr, SMI_DSW2, mask, value);
		break;
	case CHAL_SMI_BANK3:
		SMI_WRITE_REG_MASKED(pDev->baseAddr, SMI_DSW3, mask, value);
		break;
	default:
		break;
	}
}				/* chal_smi_setup_wr_timing */

/*
 *
 * Function Name: chal_smi_dir_write_acc
 *
 * Description:   Direct Mode Write Access
 *
 */
void chal_smi_dir_write_acc(CHAL_HANDLE handle,
			    cUInt8 bank, cUInt8 addr, cUInt32 data)
{
	pCHAL_SMI_T pDev = (pCHAL_SMI_T)handle;

/*    chal_dprintf (CDBG_INFO, "chal_smi_dir_write_acc: " */
/*        "BANK[%02d] ADDR[0x%08X] DATA[0x%08X]\n",  bank, addr, data ); */

	/* enable direct mode & clear DONE status if set */
	BRCM_WRITE_REG(pDev->baseAddr,
		       SMI_DCS, SMI_DCS_EANBLE_MASK | SMI_DCS_DONE_MASK);

	BRCM_WRITE_REG(pDev->baseAddr, SMI_DA,
		       (cUInt32)addr | ((cUInt32)(bank)) << SMI_DA_WRITE_SHIFT);

	BRCM_WRITE_REG_FIELD(pDev->baseAddr, SMI_DD, DD, data);

	BRCM_WRITE_REG(pDev->baseAddr,
		       SMI_DCS,
		       SMI_DCS_EANBLE_MASK | SMI_DCS_WRITE_MASK |
		       SMI_DCS_START_MASK);
}

/*
 *
 * Function Name: chal_smi_dir_read_acc
 *
 * Description:   Direct Mode Read Access
 *
 */
void chal_smi_dir_read_acc(CHAL_HANDLE handle, cUInt8 bank, cUInt8 addr)
{
	pCHAL_SMI_T pDev = (pCHAL_SMI_T)handle;

	/* enable direct mode & clear DONE status if set */
	BRCM_WRITE_REG(pDev->baseAddr,
		       SMI_DCS, SMI_DCS_EANBLE_MASK | SMI_DCS_DONE_MASK);

	BRCM_WRITE_REG(pDev->baseAddr, SMI_DA,
		       (cUInt32)addr | ((cUInt32)(bank)) << SMI_DA_WRITE_SHIFT);

	BRCM_WRITE_REG(pDev->baseAddr,
		       SMI_DCS, SMI_DCS_EANBLE_MASK | SMI_DCS_START_MASK);
}

/*
 *
 * Function Name: chal_smi_dir_read_data
 *
 * Description:   Direct Mode Read Data
 *
 */
cUInt32 chal_smi_dir_read_data(CHAL_HANDLE handle)
{
	pCHAL_SMI_T pDev = (pCHAL_SMI_T)handle;

	return BRCM_READ_REG(pDev->baseAddr, SMI_DD);
}

/*
 *
 * Function Name: chal_smi_fifo_done
 *
 * Description:   Clear FIFOs & FIFO Done Flag, Disable FIFO mode (TE & INTs)
 *                Disable DMA
 *
 */
cVoid chal_smi_fifo_done(CHAL_HANDLE handle)
{
	pCHAL_SMI_T pDev = (pCHAL_SMI_T)handle;
	cUInt32 mask;
	cUInt32 value;

	mask = SMI_CS_TEEN_MASK	/* DIS TE  */
	    | SMI_CS_START_MASK	/* CLR START */
	    | SMI_CS_ENABLE_MASK	/* DIS Prog Mode */
	    | SMI_CS_CLEAR_MASK	/* CLR FIFOs */
	    | SMI_CS_INTR_MASK	/* DIS Int on RX */
	    | SMI_CS_INTT_MASK	/* DIS Int on TX */
	    | SMI_CS_INTD_MASK	/* DIS Int on DONE */
	    | SMI_CS_DONE_MASK;	/* CLR DONE */

	value = SMI_CS_CLEAR_MASK | SMI_CS_DONE_MASK;

	/* DIS DMA mode if set */
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, SMI_DC, DMAEN, 0);
	SMI_WRITE_REG_MASKED(pDev->baseAddr, SMI_CS, mask, value);
}

/*
 *
 * Function Name: chal_smi_fifo_acc
 *
 * Description:   Setup FIFO mode access, Read | Write
 *
 */
void chal_smi_fifo_acc(CHAL_HANDLE handle, pCHAL_FIFO_ACC fifoMode)
{
	pCHAL_SMI_T pDev = (pCHAL_SMI_T)handle;
	cUInt32 mask;
	cUInt32 value;

	/* disable programmed mode interface */
	BRCM_WRITE_REG_FIELD(pDev->baseAddr, SMI_CS, ENABLE, 0);

	BRCM_WRITE_REG(pDev->baseAddr, SMI_L, fifoMode->smilL);
	BRCM_WRITE_REG(pDev->baseAddr, SMI_A,
		       (cUInt32)fifoMode->addr | ((cUInt32)(fifoMode->bank))
		       << SMI_A_DEVICE_SHIFT);

	if (fifoMode->isDma)
		BRCM_WRITE_REG_FIELD(pDev->baseAddr, SMI_DC, DMAEN, 1);
	else
		BRCM_WRITE_REG_FIELD(pDev->baseAddr, SMI_DC, DMAEN, 0);

	mask = SMI_CS_PXLDAT_MASK | SMI_CS_PVMODE_MASK | SMI_CS_TEEN_MASK |
			SMI_CS_WRITE_MASK | SMI_CS_START_MASK |
			SMI_CS_ENABLE_MASK | SMI_CS_TEEN_MASK |
			SMI_CS_CLEAR_MASK; /* clear FIFOs */
	mask |= SMI_CS_PVMODE_MASK; /* PV MODE for now hardcode to 0=DISABLE */
	mask |= SMI_CS_PAD_MASK; /* PADDING for now hardcode to 0 bytes */

	value = 0;

	if (fifoMode->isPixelFormat != 0)
		value |= SMI_CS_PXLDAT_MASK;
	if (fifoMode->isWrite != 0)
		value |= SMI_CS_WRITE_MASK;
	if (fifoMode->isTE != 0)
		value |= SMI_CS_TEEN_MASK;
/*  if( fifoMode->isPvMode      != 0 ) value |= SMI_CS_PVMODE_MASK; */
/*  value |= (fifoMode->padCount << SMI_CS_PAD_SHIFT);     */
	value |= (0 << SMI_CS_PAD_SHIFT);

	value |= SMI_CS_CLEAR_MASK;	/* clear  FIFOs */
	value |= SMI_CS_START_MASK;	/* start  PROGRAMMED mode */
	value |= SMI_CS_ENABLE_MASK;	/* enable PROGRAMMED mode */

	SMI_WRITE_REG_MASKED(pDev->baseAddr, SMI_CS, mask, value);
}

/*
 *
 * Function Name: chal_smi_fifo_wr
 *
 * Description:   Write to FIFO
 *
 */
void chal_smi_fifo_wr(CHAL_HANDLE handle, cUInt32 data)
{
	pCHAL_SMI_T pDev = (pCHAL_SMI_T)handle;

	BRCM_WRITE_REG(pDev->baseAddr, SMI_D, data);
}

/*
 *
 * Function Name: chal_smi_fifo_rd
 *
 * Description:   Read from FIFO
 *
 */
cUInt32 chal_smi_fifo_rd(CHAL_HANDLE handle)
{
	pCHAL_SMI_T pDev = (pCHAL_SMI_T)handle;

	return BRCM_READ_REG(pDev->baseAddr, SMI_D);
}

/*
 *
 * Function Name: chal_smi_get_dma_addr
 *
 * Description:   Get FIFO address
 *
 */
cUInt32 chal_smi_get_dma_addr(CHAL_HANDLE handle)
{
	pCHAL_SMI_T pDev = (pCHAL_SMI_T)handle;

	return BRCM_REGADDR(pDev->baseAddr, SMI_D);
}

/*
 *
 * Function Name: chal_smi_set_dma
 *
 * Description:   Enable | Disable SMI DMA Interface
 *
 */
void chal_smi_set_dma(CHAL_HANDLE handle, cBool enable)
{
	pCHAL_SMI_T pDev = (pCHAL_SMI_T)handle;

	if (enable)
		BRCM_WRITE_REG_FIELD(pDev->baseAddr, SMI_DC, DMAEN, (cUInt32)1);
	else
		BRCM_WRITE_REG_FIELD(pDev->baseAddr, SMI_DC, DMAEN, (cUInt32)0);
}

/*
 *
 * Function Name: chal_smi_set_te_enable
 *
 * Description:   Enable|Disable SMI Tearing SYNC
 *
 *
 */
cVoid chal_smi_set_te_enable(CHAL_HANDLE handle, cBool enable)
{
	pCHAL_SMI_T pDev = (pCHAL_SMI_T)handle;

	if (enable)
		BRCM_WRITE_REG_FIELD(pDev->baseAddr, SMI_CS, TEEN, (cUInt32)1);
	else
		BRCM_WRITE_REG_FIELD(pDev->baseAddr, SMI_CS, TEEN, (cUInt32)0);
}

/*
 *
 *  Function Name:  chal_smi_rd_status_dir
 *
 *  Description:    Returns Content Of Direct Control Status Register
 *                  SMI Direct Control:  WRITE/DONE/START/ENABLE
 *
 */
cUInt32 chal_smi_rd_status_dir(CHAL_HANDLE handle)
{
	pCHAL_SMI_T pDev = (pCHAL_SMI_T)handle;

	return BRCM_READ_REG(pDev->baseAddr, SMI_DCS);
}

/*
 *
 *  Function Name:  chal_smi_rd_status_fifo
 *
 *  Description:    Returns Content Of Status Register (PROG mode)
 *                  SMI Control/Status Register
 *
 */
cUInt32 chal_smi_rd_status_fifo(CHAL_HANDLE handle)
{
	pCHAL_SMI_T pDev = (pCHAL_SMI_T)handle;

	return BRCM_READ_REG(pDev->baseAddr, SMI_CS);
}
