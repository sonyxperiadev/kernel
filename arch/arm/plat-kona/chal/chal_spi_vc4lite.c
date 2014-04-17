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
*  @file   chal_spivc4l.c
*
*  @brief  HERA MM SPI Controller cHAL source code file.
*
*  @note   LOSSI, Bidirectional SDA Data Line
*
****************************************************************************/
#include <plat/chal/chal_common.h>
#include <plat/chal/chal_spi_vc4lite.h>

#if (defined(_RHEA_) || defined(_SAMOA_))
#include <mach/rdb/brcm_rdb_spi.h>
#elif (defined(_HERA_))
#include <mach/rdb/brcm_rdb_spi_vc4l.h>
#endif

#include <mach/rdb/brcm_rdb_util.h>

/* Max DMA transfer length */
#define SPI_VC4L_DMAL_MAX 0x0000FFFF

/* SET REGISTER BIT FIELD; VALUE IS 0 BASED  */
#define SPI_REG_FIELD_SET(r, f, d) \
	(((BRCM_REGTYPE(r))(d) << BRCM_FIELDSHIFT(r, f)) \
	& BRCM_FIELDMASK(r, f))

/* SET REGISTER BITS WITH MASK */
#define SPI_WRITE_REG_MASKED(b, r, m, d) \
	(BRCM_WRITE_REG(b, r, (BRCM_READ_REG(b, r) & (~m)) | d))

/*
 * Local Variables
 */
struct CHAL_SPIVC4L {
	cBool init;
	cUInt32 baseAddr;
};

static struct CHAL_SPIVC4L spiDev[1];

/*
 *
 * Function Name: chal_spivc4l_init
 *
 * Description:   Initialize SPI Controller cHAL Interface
 *
 *
 */
CHAL_HANDLE chal_spivc4l_init(cUInt32 baseAddr)
{
	struct CHAL_SPIVC4L *pDev;

	chal_dprintf(CDBG_INFO, "chal_spivc4l_init\n");

	pDev = (struct CHAL_SPIVC4L *)&spiDev[0];

	if (!pDev->init) {
		pDev->baseAddr = baseAddr;
		pDev->init = TRUE;
	}

	return (CHAL_HANDLE) pDev;
}

/*
 *
 * Function Name: chal_spivc4l_set_clk_div
 *
 * Description:   Set Clock Divider
 *
 *                Clock Divider =>  SCLK = Core Clock / CDIV
 *                If CDIV is set to 0, the divisor is 65536.
 *                The divisor must be a power of 2. Odd numbers rounded down.
 *
 */
cInt32 chal_spivc4l_set_clk_div(CHAL_HANDLE handle, cUInt32 clk_div)
{
	struct CHAL_SPIVC4L *pDev = (struct CHAL_SPIVC4L *)handle;

	if (clk_div > 0xFFFE) {
		chal_dprintf(CDBG_ERRO, "chal_spivc4l_set_clk_div: "
			     "ERR ClkDiv Oveflow [0x%08X], MAX[0xFFFE]\n",
			     clk_div);
		return -1;
	}

	if ((clk_div % 2) != 0) {
		chal_dprintf(CDBG_ERRO, "chal_spivc4l_set_clk_div: "
			     "ERR ClkDiv[0x%08X] Must Be Even Number\n",
			     clk_div);
		return -1;
	}
	BRCM_WRITE_REG(pDev->baseAddr, SPI_CLK, clk_div);

	return 0;
}				/* chal_spivc4l_set_clk_div */

/*
 *
 * Function Name: chal_spivc4l_wr_fifo
 *
 * Description:   Write to FIFO
 *
 */
void chal_spivc4l_wr_fifo(CHAL_HANDLE handle, cUInt32 data)
{
	struct CHAL_SPIVC4L *pDev = (struct CHAL_SPIVC4L *)handle;

	BRCM_WRITE_REG(pDev->baseAddr, SPI_FIFO, data);
}

/*
 *
 * Function Name: chal_spivc4l_rd_fifo
 *
 * Description:   Read from FIFO
 *
 */
cUInt32 chal_spivc4l_rd_fifo(CHAL_HANDLE handle)
{
	struct CHAL_SPIVC4L *pDev = (struct CHAL_SPIVC4L *)handle;

	return BRCM_READ_REG(pDev->baseAddr, SPI_FIFO);
}

/*
 *
 * Function Name: chal_spivc4l_get_dma_addr
 *
 * Description:   Get FIFO address
 *
 */
cUInt32 chal_spivc4l_get_dma_addr(CHAL_HANDLE handle)
{
	struct CHAL_SPIVC4L *pDev = (struct CHAL_SPIVC4L *)handle;

	return BRCM_REGADDR(pDev->baseAddr, SPI_FIFO);
}

/*
 *
 * Function Name: chal_spivc4l_set_dma_len
 *
 * Description:   Set SPI's DMALEN register
 *
 */
cInt32 chal_spivc4l_set_dma_len(CHAL_HANDLE handle, cUInt32 dma_len)
{
	struct CHAL_SPIVC4L *pDev = (struct CHAL_SPIVC4L *)handle;

	if (dma_len <= SPI_VC4L_DMAL_MAX) {
		BRCM_WRITE_REG(pDev->baseAddr, SPI_DLEN, dma_len);
		return 0;
	} else {
		return -1;
	}
}

/*
 *
 *  Function Name:  chal_spivc4l_rwm_cs
 *
 *  Description:    R-W-M SPI's Control-Status Register
 *
 */
void chal_spivc4l_rwm_cs(CHAL_HANDLE handle, cUInt32 mask, cUInt32 val)
{
	struct CHAL_SPIVC4L *pDev = (struct CHAL_SPIVC4L *)handle;

	SPI_WRITE_REG_MASKED(pDev->baseAddr, SPI_CS, mask, val);
}

/*
 *
 *  Function Name:  chal_spivc4l_set_cs
 *
 *  Description:    Set SPI's Control-Status Register
 *
 */
void chal_spivc4l_set_cs(CHAL_HANDLE handle, cUInt32 val)
{
	struct CHAL_SPIVC4L *pDev = (struct CHAL_SPIVC4L *)handle;

	BRCM_WRITE_REG(pDev->baseAddr, SPI_CS, val);
}

/*
 *
 *  Function Name:  chal_spivc4l_get_cs
 *
 *  Description:    Returns Content Of SPI's Control-Status Register
 *
 */
cUInt32 chal_spivc4l_get_cs(CHAL_HANDLE handle)
{
	struct CHAL_SPIVC4L *pDev = (struct CHAL_SPIVC4L *)handle;

	return BRCM_READ_REG(pDev->baseAddr, SPI_CS);
}
