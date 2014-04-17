/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2012  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/*     Date     : Generated on 9/25/2012 19:15:19                                             */
/*     RDB file : //HAWAII/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_SPI_H__
#define __BRCM_RDB_SPI_H__

#define SPI_CS_OFFSET                                                     0x00000000
#define SPI_CS_TYPE                                                       UInt32
#define SPI_CS_RESERVED_MASK                                              0xFC000000
#define    SPI_CS_LEN_LONG_SHIFT                                          25
#define    SPI_CS_LEN_LONG_MASK                                           0x02000000
#define    SPI_CS_DMA_LEN_SHIFT                                           24
#define    SPI_CS_DMA_LEN_MASK                                            0x01000000
#define    SPI_CS_CSPOL2_SHIFT                                            23
#define    SPI_CS_CSPOL2_MASK                                             0x00800000
#define    SPI_CS_CSPOL1_SHIFT                                            22
#define    SPI_CS_CSPOL1_MASK                                             0x00400000
#define    SPI_CS_CSPOL0_SHIFT                                            21
#define    SPI_CS_CSPOL0_MASK                                             0x00200000
#define    SPI_CS_RXF_SHIFT                                               20
#define    SPI_CS_RXF_MASK                                                0x00100000
#define    SPI_CS_RXR_SHIFT                                               19
#define    SPI_CS_RXR_MASK                                                0x00080000
#define    SPI_CS_TXD_SHIFT                                               18
#define    SPI_CS_TXD_MASK                                                0x00040000
#define    SPI_CS_RXD_SHIFT                                               17
#define    SPI_CS_RXD_MASK                                                0x00020000
#define    SPI_CS_DONE_SHIFT                                              16
#define    SPI_CS_DONE_MASK                                               0x00010000
#define    SPI_CS_TE_EN_SHIFT                                             15
#define    SPI_CS_TE_EN_MASK                                              0x00008000
#define    SPI_CS_LMONO_SHIFT                                             14
#define    SPI_CS_LMONO_MASK                                              0x00004000
#define    SPI_CS_LEN_SHIFT                                               13
#define    SPI_CS_LEN_MASK                                                0x00002000
#define    SPI_CS_REN_SHIFT                                               12
#define    SPI_CS_REN_MASK                                                0x00001000
#define    SPI_CS_ADCS_SHIFT                                              11
#define    SPI_CS_ADCS_MASK                                               0x00000800
#define    SPI_CS_INTR_SHIFT                                              10
#define    SPI_CS_INTR_MASK                                               0x00000400
#define    SPI_CS_INTD_SHIFT                                              9
#define    SPI_CS_INTD_MASK                                               0x00000200
#define    SPI_CS_DMAEN_SHIFT                                             8
#define    SPI_CS_DMAEN_MASK                                              0x00000100
#define    SPI_CS_TA_SHIFT                                                7
#define    SPI_CS_TA_MASK                                                 0x00000080
#define    SPI_CS_CSPOL_SHIFT                                             6
#define    SPI_CS_CSPOL_MASK                                              0x00000040
#define    SPI_CS_CLEAR_SHIFT                                             4
#define    SPI_CS_CLEAR_MASK                                              0x00000030
#define    SPI_CS_CPOL_SHIFT                                              3
#define    SPI_CS_CPOL_MASK                                               0x00000008
#define    SPI_CS_CPHA_SHIFT                                              2
#define    SPI_CS_CPHA_MASK                                               0x00000004
#define    SPI_CS_CS_SHIFT                                                0
#define    SPI_CS_CS_MASK                                                 0x00000003

#define SPI_FIFO_OFFSET                                                   0x00000004
#define SPI_FIFO_TYPE                                                     UInt32
#define SPI_FIFO_RESERVED_MASK                                            0x00000000
#define    SPI_FIFO_DATA_SHIFT                                            0
#define    SPI_FIFO_DATA_MASK                                             0xFFFFFFFF

#define SPI_CLK_OFFSET                                                    0x00000008
#define SPI_CLK_TYPE                                                      UInt32
#define SPI_CLK_RESERVED_MASK                                             0xFFFF0000
#define    SPI_CLK_CDIV_SHIFT                                             0
#define    SPI_CLK_CDIV_MASK                                              0x0000FFFF

#define SPI_DLEN_OFFSET                                                   0x0000000C
#define SPI_DLEN_TYPE                                                     UInt32
#define SPI_DLEN_RESERVED_MASK                                            0xFFFF0000
#define    SPI_DLEN_LEN_SHIFT                                             0
#define    SPI_DLEN_LEN_MASK                                              0x0000FFFF

#define SPI_LTOH_OFFSET                                                   0x00000010
#define SPI_LTOH_TYPE                                                     UInt32
#define SPI_LTOH_RESERVED_MASK                                            0xFFFFFFF0
#define    SPI_LTOH_TOH_SHIFT                                             0
#define    SPI_LTOH_TOH_MASK                                              0x0000000F

#define SPI_DC_OFFSET                                                     0x00000014
#define SPI_DC_TYPE                                                       UInt32
#define SPI_DC_RESERVED_MASK                                              0x00000000
#define    SPI_DC_RPANIC_SHIFT                                            24
#define    SPI_DC_RPANIC_MASK                                             0xFF000000
#define    SPI_DC_RDREQ_SHIFT                                             16
#define    SPI_DC_RDREQ_MASK                                              0x00FF0000
#define    SPI_DC_TPANIC_SHIFT                                            8
#define    SPI_DC_TPANIC_MASK                                             0x0000FF00
#define    SPI_DC_TDREQ_SHIFT                                             0
#define    SPI_DC_TDREQ_MASK                                              0x000000FF

#endif /* __BRCM_RDB_SPI_H__ */


