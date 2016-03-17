/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/plat-kona/include/plat/spi_kona.h
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

/*
 * Broadcom KONA SSPI based SPI master controller
 */
#ifndef __ARCH_PLAT_SPI_KONA_H__
#define __ARCH_PLAT_SPI_KONA_H__

struct spi_kona_platform_data {
	u8 enable_dma;
	u8 cs_line;
	u8 mode;
};
#endif /* __ARCH_PLAT_SPI_KONA_H__ */
