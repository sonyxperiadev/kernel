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
/*     Date     : Generated on 2/10/2012 15:44:22                                             */
/*     RDB file : //RHEA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_DSP_RF_GPO_H__
#define __BRCM_RDB_DSP_RF_GPO_H__

#define DSP_RF_GPO_GPSPI_R_OFFSET                                         0x00000A36
#define DSP_RF_GPO_GPSPI_R_TYPE                                           UInt16
#define DSP_RF_GPO_GPSPI_R_RESERVED_MASK                                  0x0000FFF8
#define    DSP_RF_GPO_GPSPI_R_SPI_MUX_SHIFT                               0
#define    DSP_RF_GPO_GPSPI_R_SPI_MUX_MASK                                0x00000007

#define DSP_RF_GPO_GPPCR_R_OFFSET                                         0x00000A38
#define DSP_RF_GPO_GPPCR_R_TYPE                                           UInt16
#define DSP_RF_GPO_GPPCR_R_RESERVED_MASK                                  0x0000FFFC
#define    DSP_RF_GPO_GPPCR_R_PAGE_SHIFT                                  0
#define    DSP_RF_GPO_GPPCR_R_PAGE_MASK                                   0x00000003

#define DSP_RF_GPO_GPDSCR_R_OFFSET                                        0x00000A3A
#define DSP_RF_GPO_GPDSCR_R_TYPE                                          UInt16
#define DSP_RF_GPO_GPDSCR_R_RESERVED_MASK                                 0x00000000
#define    DSP_RF_GPO_GPDSCR_R_STATE_SHIFT                                0
#define    DSP_RF_GPO_GPDSCR_R_STATE_MASK                                 0x0000FFFF

#define DSP_RF_GPO_GPOCR_R_OFFSET                                         0x00000A3C
#define DSP_RF_GPO_GPOCR_R_TYPE                                           UInt16
#define DSP_RF_GPO_GPOCR_R_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_GPOCR_R_OC_SHIFT                                    0
#define    DSP_RF_GPO_GPOCR_R_OC_MASK                                     0x0000FFFF

#define DSP_RF_GPO_GPWSR_R_OFFSET                                         0x00000A3E
#define DSP_RF_GPO_GPWSR_R_TYPE                                           UInt16
#define DSP_RF_GPO_GPWSR_R_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_GPWSR_R_SEL_SHIFT                                   0
#define    DSP_RF_GPO_GPWSR_R_SEL_MASK                                    0x0000FFFF

#define DSP_RF_GPO_TGDR_R0_OFFSET                                         0x00000A40
#define DSP_RF_GPO_TGDR_R0_TYPE                                           UInt16
#define DSP_RF_GPO_TGDR_R0_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_TGDR_R0_DEL_SHIFT                                   0
#define    DSP_RF_GPO_TGDR_R0_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_TGDR_R1_OFFSET                                         0x00000A42
#define DSP_RF_GPO_TGDR_R1_TYPE                                           UInt16
#define DSP_RF_GPO_TGDR_R1_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_TGDR_R1_DEL_SHIFT                                   0
#define    DSP_RF_GPO_TGDR_R1_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_TGDR_R2_OFFSET                                         0x00000A44
#define DSP_RF_GPO_TGDR_R2_TYPE                                           UInt16
#define DSP_RF_GPO_TGDR_R2_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_TGDR_R2_DEL_SHIFT                                   0
#define    DSP_RF_GPO_TGDR_R2_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_TGDR_R3_OFFSET                                         0x00000A46
#define DSP_RF_GPO_TGDR_R3_TYPE                                           UInt16
#define DSP_RF_GPO_TGDR_R3_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_TGDR_R3_DEL_SHIFT                                   0
#define    DSP_RF_GPO_TGDR_R3_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_TGDR_R4_OFFSET                                         0x00000A48
#define DSP_RF_GPO_TGDR_R4_TYPE                                           UInt16
#define DSP_RF_GPO_TGDR_R4_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_TGDR_R4_DEL_SHIFT                                   0
#define    DSP_RF_GPO_TGDR_R4_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_TGDR_R5_OFFSET                                         0x00000A4A
#define DSP_RF_GPO_TGDR_R5_TYPE                                           UInt16
#define DSP_RF_GPO_TGDR_R5_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_TGDR_R5_DEL_SHIFT                                   0
#define    DSP_RF_GPO_TGDR_R5_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_TGDR_R6_OFFSET                                         0x00000A4C
#define DSP_RF_GPO_TGDR_R6_TYPE                                           UInt16
#define DSP_RF_GPO_TGDR_R6_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_TGDR_R6_DEL_SHIFT                                   0
#define    DSP_RF_GPO_TGDR_R6_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_TGDR_R7_OFFSET                                         0x00000A4E
#define DSP_RF_GPO_TGDR_R7_TYPE                                           UInt16
#define DSP_RF_GPO_TGDR_R7_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_TGDR_R7_DEL_SHIFT                                   0
#define    DSP_RF_GPO_TGDR_R7_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_TGPR_R0_OFFSET                                         0x00000A50
#define DSP_RF_GPO_TGPR_R0_TYPE                                           UInt16
#define DSP_RF_GPO_TGPR_R0_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_TGPR_R0_TGP_SHIFT                                   0
#define    DSP_RF_GPO_TGPR_R0_TGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_TGPR_R1_OFFSET                                         0x00000A52
#define DSP_RF_GPO_TGPR_R1_TYPE                                           UInt16
#define DSP_RF_GPO_TGPR_R1_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_TGPR_R1_TGP_SHIFT                                   0
#define    DSP_RF_GPO_TGPR_R1_TGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_TGPR_R2_OFFSET                                         0x00000A54
#define DSP_RF_GPO_TGPR_R2_TYPE                                           UInt16
#define DSP_RF_GPO_TGPR_R2_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_TGPR_R2_TGP_SHIFT                                   0
#define    DSP_RF_GPO_TGPR_R2_TGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_TGPR_R3_OFFSET                                         0x00000A56
#define DSP_RF_GPO_TGPR_R3_TYPE                                           UInt16
#define DSP_RF_GPO_TGPR_R3_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_TGPR_R3_TGP_SHIFT                                   0
#define    DSP_RF_GPO_TGPR_R3_TGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_TGPR_R4_OFFSET                                         0x00000A58
#define DSP_RF_GPO_TGPR_R4_TYPE                                           UInt16
#define DSP_RF_GPO_TGPR_R4_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_TGPR_R4_TGP_SHIFT                                   0
#define    DSP_RF_GPO_TGPR_R4_TGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_TGPR_R5_OFFSET                                         0x00000A5A
#define DSP_RF_GPO_TGPR_R5_TYPE                                           UInt16
#define DSP_RF_GPO_TGPR_R5_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_TGPR_R5_TGP_SHIFT                                   0
#define    DSP_RF_GPO_TGPR_R5_TGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_TGPR_R6_OFFSET                                         0x00000A5C
#define DSP_RF_GPO_TGPR_R6_TYPE                                           UInt16
#define DSP_RF_GPO_TGPR_R6_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_TGPR_R6_TGP_SHIFT                                   0
#define    DSP_RF_GPO_TGPR_R6_TGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_TGPR_R7_OFFSET                                         0x00000A5E
#define DSP_RF_GPO_TGPR_R7_TYPE                                           UInt16
#define DSP_RF_GPO_TGPR_R7_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_TGPR_R7_TGP_SHIFT                                   0
#define    DSP_RF_GPO_TGPR_R7_TGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_RGDR_R0_OFFSET                                         0x00000A60
#define DSP_RF_GPO_RGDR_R0_TYPE                                           UInt16
#define DSP_RF_GPO_RGDR_R0_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_RGDR_R0_DEL_SHIFT                                   0
#define    DSP_RF_GPO_RGDR_R0_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_RGDR_R1_OFFSET                                         0x00000A62
#define DSP_RF_GPO_RGDR_R1_TYPE                                           UInt16
#define DSP_RF_GPO_RGDR_R1_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_RGDR_R1_DEL_SHIFT                                   0
#define    DSP_RF_GPO_RGDR_R1_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_RGDR_R2_OFFSET                                         0x00000A64
#define DSP_RF_GPO_RGDR_R2_TYPE                                           UInt16
#define DSP_RF_GPO_RGDR_R2_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_RGDR_R2_DEL_SHIFT                                   0
#define    DSP_RF_GPO_RGDR_R2_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_RGDR_R3_OFFSET                                         0x00000A66
#define DSP_RF_GPO_RGDR_R3_TYPE                                           UInt16
#define DSP_RF_GPO_RGDR_R3_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_RGDR_R3_DEL_SHIFT                                   0
#define    DSP_RF_GPO_RGDR_R3_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_RGDR_R4_OFFSET                                         0x00000A68
#define DSP_RF_GPO_RGDR_R4_TYPE                                           UInt16
#define DSP_RF_GPO_RGDR_R4_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_RGDR_R4_DEL_SHIFT                                   0
#define    DSP_RF_GPO_RGDR_R4_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_RGDR_R5_OFFSET                                         0x00000A6A
#define DSP_RF_GPO_RGDR_R5_TYPE                                           UInt16
#define DSP_RF_GPO_RGDR_R5_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_RGDR_R5_DEL_SHIFT                                   0
#define    DSP_RF_GPO_RGDR_R5_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_RGDR_R6_OFFSET                                         0x00000A6C
#define DSP_RF_GPO_RGDR_R6_TYPE                                           UInt16
#define DSP_RF_GPO_RGDR_R6_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_RGDR_R6_DEL_SHIFT                                   0
#define    DSP_RF_GPO_RGDR_R6_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_RGDR_R7_OFFSET                                         0x00000A6E
#define DSP_RF_GPO_RGDR_R7_TYPE                                           UInt16
#define DSP_RF_GPO_RGDR_R7_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_RGDR_R7_DEL_SHIFT                                   0
#define    DSP_RF_GPO_RGDR_R7_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_RGPR_R0_OFFSET                                         0x00000A70
#define DSP_RF_GPO_RGPR_R0_TYPE                                           UInt16
#define DSP_RF_GPO_RGPR_R0_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_RGPR_R0_RGP_SHIFT                                   0
#define    DSP_RF_GPO_RGPR_R0_RGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_RGPR_R1_OFFSET                                         0x00000A72
#define DSP_RF_GPO_RGPR_R1_TYPE                                           UInt16
#define DSP_RF_GPO_RGPR_R1_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_RGPR_R1_RGP_SHIFT                                   0
#define    DSP_RF_GPO_RGPR_R1_RGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_RGPR_R2_OFFSET                                         0x00000A74
#define DSP_RF_GPO_RGPR_R2_TYPE                                           UInt16
#define DSP_RF_GPO_RGPR_R2_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_RGPR_R2_RGP_SHIFT                                   0
#define    DSP_RF_GPO_RGPR_R2_RGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_RGPR_R3_OFFSET                                         0x00000A76
#define DSP_RF_GPO_RGPR_R3_TYPE                                           UInt16
#define DSP_RF_GPO_RGPR_R3_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_RGPR_R3_RGP_SHIFT                                   0
#define    DSP_RF_GPO_RGPR_R3_RGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_RGPR_R4_OFFSET                                         0x00000A78
#define DSP_RF_GPO_RGPR_R4_TYPE                                           UInt16
#define DSP_RF_GPO_RGPR_R4_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_RGPR_R4_RGP_SHIFT                                   0
#define    DSP_RF_GPO_RGPR_R4_RGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_RGPR_R5_OFFSET                                         0x00000A7A
#define DSP_RF_GPO_RGPR_R5_TYPE                                           UInt16
#define DSP_RF_GPO_RGPR_R5_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_RGPR_R5_RGP_SHIFT                                   0
#define    DSP_RF_GPO_RGPR_R5_RGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_RGPR_R6_OFFSET                                         0x00000A7C
#define DSP_RF_GPO_RGPR_R6_TYPE                                           UInt16
#define DSP_RF_GPO_RGPR_R6_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_RGPR_R6_RGP_SHIFT                                   0
#define    DSP_RF_GPO_RGPR_R6_RGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_RGPR_R7_OFFSET                                         0x00000A7E
#define DSP_RF_GPO_RGPR_R7_TYPE                                           UInt16
#define DSP_RF_GPO_RGPR_R7_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_RGPR_R7_RGP_SHIFT                                   0
#define    DSP_RF_GPO_RGPR_R7_RGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_TGDR_R8_OFFSET                                         0x00000AA0
#define DSP_RF_GPO_TGDR_R8_TYPE                                           UInt16
#define DSP_RF_GPO_TGDR_R8_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_TGDR_R8_DEL_SHIFT                                   0
#define    DSP_RF_GPO_TGDR_R8_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_TGDR_R9_OFFSET                                         0x00000AA2
#define DSP_RF_GPO_TGDR_R9_TYPE                                           UInt16
#define DSP_RF_GPO_TGDR_R9_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_TGDR_R9_DEL_SHIFT                                   0
#define    DSP_RF_GPO_TGDR_R9_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_TGDR_R10_OFFSET                                        0x00000AA4
#define DSP_RF_GPO_TGDR_R10_TYPE                                          UInt16
#define DSP_RF_GPO_TGDR_R10_RESERVED_MASK                                 0x0000F000
#define    DSP_RF_GPO_TGDR_R10_DEL_SHIFT                                  0
#define    DSP_RF_GPO_TGDR_R10_DEL_MASK                                   0x00000FFF

#define DSP_RF_GPO_TGDR_R11_OFFSET                                        0x00000AA6
#define DSP_RF_GPO_TGDR_R11_TYPE                                          UInt16
#define DSP_RF_GPO_TGDR_R11_RESERVED_MASK                                 0x0000F000
#define    DSP_RF_GPO_TGDR_R11_DEL_SHIFT                                  0
#define    DSP_RF_GPO_TGDR_R11_DEL_MASK                                   0x00000FFF

#define DSP_RF_GPO_TGDR_R12_OFFSET                                        0x00000AA8
#define DSP_RF_GPO_TGDR_R12_TYPE                                          UInt16
#define DSP_RF_GPO_TGDR_R12_RESERVED_MASK                                 0x0000F000
#define    DSP_RF_GPO_TGDR_R12_DEL_SHIFT                                  0
#define    DSP_RF_GPO_TGDR_R12_DEL_MASK                                   0x00000FFF

#define DSP_RF_GPO_TGDR_R13_OFFSET                                        0x00000AAA
#define DSP_RF_GPO_TGDR_R13_TYPE                                          UInt16
#define DSP_RF_GPO_TGDR_R13_RESERVED_MASK                                 0x0000F000
#define    DSP_RF_GPO_TGDR_R13_DEL_SHIFT                                  0
#define    DSP_RF_GPO_TGDR_R13_DEL_MASK                                   0x00000FFF

#define DSP_RF_GPO_TGDR_R14_OFFSET                                        0x00000AAC
#define DSP_RF_GPO_TGDR_R14_TYPE                                          UInt16
#define DSP_RF_GPO_TGDR_R14_RESERVED_MASK                                 0x0000F000
#define    DSP_RF_GPO_TGDR_R14_DEL_SHIFT                                  0
#define    DSP_RF_GPO_TGDR_R14_DEL_MASK                                   0x00000FFF

#define DSP_RF_GPO_TGDR_R15_OFFSET                                        0x00000AAE
#define DSP_RF_GPO_TGDR_R15_TYPE                                          UInt16
#define DSP_RF_GPO_TGDR_R15_RESERVED_MASK                                 0x0000F000
#define    DSP_RF_GPO_TGDR_R15_DEL_SHIFT                                  0
#define    DSP_RF_GPO_TGDR_R15_DEL_MASK                                   0x00000FFF

#define DSP_RF_GPO_TGPR_R8_OFFSET                                         0x00000AB0
#define DSP_RF_GPO_TGPR_R8_TYPE                                           UInt16
#define DSP_RF_GPO_TGPR_R8_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_TGPR_R8_TGP_SHIFT                                   0
#define    DSP_RF_GPO_TGPR_R8_TGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_TGPR_R9_OFFSET                                         0x00000AB2
#define DSP_RF_GPO_TGPR_R9_TYPE                                           UInt16
#define DSP_RF_GPO_TGPR_R9_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_TGPR_R9_TGP_SHIFT                                   0
#define    DSP_RF_GPO_TGPR_R9_TGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_TGPR_R10_OFFSET                                        0x00000AB4
#define DSP_RF_GPO_TGPR_R10_TYPE                                          UInt16
#define DSP_RF_GPO_TGPR_R10_RESERVED_MASK                                 0x00000000
#define    DSP_RF_GPO_TGPR_R10_TGP_SHIFT                                  0
#define    DSP_RF_GPO_TGPR_R10_TGP_MASK                                   0x0000FFFF

#define DSP_RF_GPO_TGPR_R11_OFFSET                                        0x00000AB6
#define DSP_RF_GPO_TGPR_R11_TYPE                                          UInt16
#define DSP_RF_GPO_TGPR_R11_RESERVED_MASK                                 0x00000000
#define    DSP_RF_GPO_TGPR_R11_TGP_SHIFT                                  0
#define    DSP_RF_GPO_TGPR_R11_TGP_MASK                                   0x0000FFFF

#define DSP_RF_GPO_TGPR_R12_OFFSET                                        0x00000AB8
#define DSP_RF_GPO_TGPR_R12_TYPE                                          UInt16
#define DSP_RF_GPO_TGPR_R12_RESERVED_MASK                                 0x00000000
#define    DSP_RF_GPO_TGPR_R12_TGP_SHIFT                                  0
#define    DSP_RF_GPO_TGPR_R12_TGP_MASK                                   0x0000FFFF

#define DSP_RF_GPO_TGPR_R13_OFFSET                                        0x00000ABA
#define DSP_RF_GPO_TGPR_R13_TYPE                                          UInt16
#define DSP_RF_GPO_TGPR_R13_RESERVED_MASK                                 0x00000000
#define    DSP_RF_GPO_TGPR_R13_TGP_SHIFT                                  0
#define    DSP_RF_GPO_TGPR_R13_TGP_MASK                                   0x0000FFFF

#define DSP_RF_GPO_TGPR_R14_OFFSET                                        0x00000ABC
#define DSP_RF_GPO_TGPR_R14_TYPE                                          UInt16
#define DSP_RF_GPO_TGPR_R14_RESERVED_MASK                                 0x00000000
#define    DSP_RF_GPO_TGPR_R14_TGP_SHIFT                                  0
#define    DSP_RF_GPO_TGPR_R14_TGP_MASK                                   0x0000FFFF

#define DSP_RF_GPO_TGPR_R15_OFFSET                                        0x00000ABE
#define DSP_RF_GPO_TGPR_R15_TYPE                                          UInt16
#define DSP_RF_GPO_TGPR_R15_RESERVED_MASK                                 0x00000000
#define    DSP_RF_GPO_TGPR_R15_TGP_SHIFT                                  0
#define    DSP_RF_GPO_TGPR_R15_TGP_MASK                                   0x0000FFFF

#define DSP_RF_GPO_RGDR_R8_OFFSET                                         0x00000AC0
#define DSP_RF_GPO_RGDR_R8_TYPE                                           UInt16
#define DSP_RF_GPO_RGDR_R8_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_RGDR_R8_DEL_SHIFT                                   0
#define    DSP_RF_GPO_RGDR_R8_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_RGDR_R9_OFFSET                                         0x00000AC2
#define DSP_RF_GPO_RGDR_R9_TYPE                                           UInt16
#define DSP_RF_GPO_RGDR_R9_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_RGDR_R9_DEL_SHIFT                                   0
#define    DSP_RF_GPO_RGDR_R9_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_RGDR_R10_OFFSET                                        0x00000AC4
#define DSP_RF_GPO_RGDR_R10_TYPE                                          UInt16
#define DSP_RF_GPO_RGDR_R10_RESERVED_MASK                                 0x0000F000
#define    DSP_RF_GPO_RGDR_R10_DEL_SHIFT                                  0
#define    DSP_RF_GPO_RGDR_R10_DEL_MASK                                   0x00000FFF

#define DSP_RF_GPO_RGDR_R11_OFFSET                                        0x00000AC6
#define DSP_RF_GPO_RGDR_R11_TYPE                                          UInt16
#define DSP_RF_GPO_RGDR_R11_RESERVED_MASK                                 0x0000F000
#define    DSP_RF_GPO_RGDR_R11_DEL_SHIFT                                  0
#define    DSP_RF_GPO_RGDR_R11_DEL_MASK                                   0x00000FFF

#define DSP_RF_GPO_RGDR_R12_OFFSET                                        0x00000AC8
#define DSP_RF_GPO_RGDR_R12_TYPE                                          UInt16
#define DSP_RF_GPO_RGDR_R12_RESERVED_MASK                                 0x0000F000
#define    DSP_RF_GPO_RGDR_R12_DEL_SHIFT                                  0
#define    DSP_RF_GPO_RGDR_R12_DEL_MASK                                   0x00000FFF

#define DSP_RF_GPO_RGDR_R13_OFFSET                                        0x00000ACA
#define DSP_RF_GPO_RGDR_R13_TYPE                                          UInt16
#define DSP_RF_GPO_RGDR_R13_RESERVED_MASK                                 0x0000F000
#define    DSP_RF_GPO_RGDR_R13_DEL_SHIFT                                  0
#define    DSP_RF_GPO_RGDR_R13_DEL_MASK                                   0x00000FFF

#define DSP_RF_GPO_RGDR_R14_OFFSET                                        0x00000ACC
#define DSP_RF_GPO_RGDR_R14_TYPE                                          UInt16
#define DSP_RF_GPO_RGDR_R14_RESERVED_MASK                                 0x0000F000
#define    DSP_RF_GPO_RGDR_R14_DEL_SHIFT                                  0
#define    DSP_RF_GPO_RGDR_R14_DEL_MASK                                   0x00000FFF

#define DSP_RF_GPO_RGDR_R15_OFFSET                                        0x00000ACE
#define DSP_RF_GPO_RGDR_R15_TYPE                                          UInt16
#define DSP_RF_GPO_RGDR_R15_RESERVED_MASK                                 0x0000F000
#define    DSP_RF_GPO_RGDR_R15_DEL_SHIFT                                  0
#define    DSP_RF_GPO_RGDR_R15_DEL_MASK                                   0x00000FFF

#define DSP_RF_GPO_RGPR_R8_OFFSET                                         0x00000AD0
#define DSP_RF_GPO_RGPR_R8_TYPE                                           UInt16
#define DSP_RF_GPO_RGPR_R8_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_RGPR_R8_RGP_SHIFT                                   0
#define    DSP_RF_GPO_RGPR_R8_RGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_RGPR_R9_OFFSET                                         0x00000AD2
#define DSP_RF_GPO_RGPR_R9_TYPE                                           UInt16
#define DSP_RF_GPO_RGPR_R9_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_RGPR_R9_RGP_SHIFT                                   0
#define    DSP_RF_GPO_RGPR_R9_RGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_RGPR_R10_OFFSET                                        0x00000AD4
#define DSP_RF_GPO_RGPR_R10_TYPE                                          UInt16
#define DSP_RF_GPO_RGPR_R10_RESERVED_MASK                                 0x00000000
#define    DSP_RF_GPO_RGPR_R10_RGP_SHIFT                                  0
#define    DSP_RF_GPO_RGPR_R10_RGP_MASK                                   0x0000FFFF

#define DSP_RF_GPO_RGPR_R11_OFFSET                                        0x00000AD6
#define DSP_RF_GPO_RGPR_R11_TYPE                                          UInt16
#define DSP_RF_GPO_RGPR_R11_RESERVED_MASK                                 0x00000000
#define    DSP_RF_GPO_RGPR_R11_RGP_SHIFT                                  0
#define    DSP_RF_GPO_RGPR_R11_RGP_MASK                                   0x0000FFFF

#define DSP_RF_GPO_RGPR_R12_OFFSET                                        0x00000AD8
#define DSP_RF_GPO_RGPR_R12_TYPE                                          UInt16
#define DSP_RF_GPO_RGPR_R12_RESERVED_MASK                                 0x00000000
#define    DSP_RF_GPO_RGPR_R12_RGP_SHIFT                                  0
#define    DSP_RF_GPO_RGPR_R12_RGP_MASK                                   0x0000FFFF

#define DSP_RF_GPO_RGPR_R13_OFFSET                                        0x00000ADA
#define DSP_RF_GPO_RGPR_R13_TYPE                                          UInt16
#define DSP_RF_GPO_RGPR_R13_RESERVED_MASK                                 0x00000000
#define    DSP_RF_GPO_RGPR_R13_RGP_SHIFT                                  0
#define    DSP_RF_GPO_RGPR_R13_RGP_MASK                                   0x0000FFFF

#define DSP_RF_GPO_RGPR_R14_OFFSET                                        0x00000ADC
#define DSP_RF_GPO_RGPR_R14_TYPE                                          UInt16
#define DSP_RF_GPO_RGPR_R14_RESERVED_MASK                                 0x00000000
#define    DSP_RF_GPO_RGPR_R14_RGP_SHIFT                                  0
#define    DSP_RF_GPO_RGPR_R14_RGP_MASK                                   0x0000FFFF

#define DSP_RF_GPO_RGPR_R15_OFFSET                                        0x00000ADE
#define DSP_RF_GPO_RGPR_R15_TYPE                                          UInt16
#define DSP_RF_GPO_RGPR_R15_RESERVED_MASK                                 0x00000000
#define    DSP_RF_GPO_RGPR_R15_RGP_SHIFT                                  0
#define    DSP_RF_GPO_RGPR_R15_RGP_MASK                                   0x0000FFFF

#define DSP_RF_GPO_GPWR_R_OFFSET                                          0x00000AFC
#define DSP_RF_GPO_GPWR_R_TYPE                                            UInt16
#define DSP_RF_GPO_GPWR_R_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_GPWR_R_WGP_SHIFT                                    0
#define    DSP_RF_GPO_GPWR_R_WGP_MASK                                     0x0000FFFF

#define DSP_RF_GPO_GPSR_R_OFFSET                                          0x00000AFE
#define DSP_RF_GPO_GPSR_R_TYPE                                            UInt16
#define DSP_RF_GPO_GPSR_R_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_GPSR_R_GPS_SHIFT                                    0
#define    DSP_RF_GPO_GPSR_R_GPS_MASK                                     0x0000FFFF

#define DSP_RF_GPO_GPSPI_OFFSET                                           0x0000E51B
#define DSP_RF_GPO_GPSPI_TYPE                                             UInt16
#define DSP_RF_GPO_GPSPI_RESERVED_MASK                                    0x0000FFF8
#define    DSP_RF_GPO_GPSPI_SPI_MUX_SHIFT                                 0
#define    DSP_RF_GPO_GPSPI_SPI_MUX_MASK                                  0x00000007

#define DSP_RF_GPO_GPPCR_OFFSET                                           0x0000E51C
#define DSP_RF_GPO_GPPCR_TYPE                                             UInt16
#define DSP_RF_GPO_GPPCR_RESERVED_MASK                                    0x0000FFFC
#define    DSP_RF_GPO_GPPCR_PAGE_SHIFT                                    0
#define    DSP_RF_GPO_GPPCR_PAGE_MASK                                     0x00000003

#define DSP_RF_GPO_GPDSCR_OFFSET                                          0x0000E51D
#define DSP_RF_GPO_GPDSCR_TYPE                                            UInt16
#define DSP_RF_GPO_GPDSCR_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_GPDSCR_STATE_SHIFT                                  0
#define    DSP_RF_GPO_GPDSCR_STATE_MASK                                   0x0000FFFF

#define DSP_RF_GPO_GPOCR_OFFSET                                           0x0000E51E
#define DSP_RF_GPO_GPOCR_TYPE                                             UInt16
#define DSP_RF_GPO_GPOCR_RESERVED_MASK                                    0x00000000
#define    DSP_RF_GPO_GPOCR_OC_SHIFT                                      0
#define    DSP_RF_GPO_GPOCR_OC_MASK                                       0x0000FFFF

#define DSP_RF_GPO_GPWSR_OFFSET                                           0x0000E51F
#define DSP_RF_GPO_GPWSR_TYPE                                             UInt16
#define DSP_RF_GPO_GPWSR_RESERVED_MASK                                    0x00000000
#define    DSP_RF_GPO_GPWSR_SEL_SHIFT                                     0
#define    DSP_RF_GPO_GPWSR_SEL_MASK                                      0x0000FFFF

#define DSP_RF_GPO_TGDR_0_OFFSET                                          0x0000E520
#define DSP_RF_GPO_TGDR_0_TYPE                                            UInt16
#define DSP_RF_GPO_TGDR_0_RESERVED_MASK                                   0x0000F000
#define    DSP_RF_GPO_TGDR_0_DEL_SHIFT                                    0
#define    DSP_RF_GPO_TGDR_0_DEL_MASK                                     0x00000FFF

#define DSP_RF_GPO_TGDR_1_OFFSET                                          0x0000E521
#define DSP_RF_GPO_TGDR_1_TYPE                                            UInt16
#define DSP_RF_GPO_TGDR_1_RESERVED_MASK                                   0x0000F000
#define    DSP_RF_GPO_TGDR_1_DEL_SHIFT                                    0
#define    DSP_RF_GPO_TGDR_1_DEL_MASK                                     0x00000FFF

#define DSP_RF_GPO_TGDR_2_OFFSET                                          0x0000E522
#define DSP_RF_GPO_TGDR_2_TYPE                                            UInt16
#define DSP_RF_GPO_TGDR_2_RESERVED_MASK                                   0x0000F000
#define    DSP_RF_GPO_TGDR_2_DEL_SHIFT                                    0
#define    DSP_RF_GPO_TGDR_2_DEL_MASK                                     0x00000FFF

#define DSP_RF_GPO_TGDR_3_OFFSET                                          0x0000E523
#define DSP_RF_GPO_TGDR_3_TYPE                                            UInt16
#define DSP_RF_GPO_TGDR_3_RESERVED_MASK                                   0x0000F000
#define    DSP_RF_GPO_TGDR_3_DEL_SHIFT                                    0
#define    DSP_RF_GPO_TGDR_3_DEL_MASK                                     0x00000FFF

#define DSP_RF_GPO_TGDR_4_OFFSET                                          0x0000E524
#define DSP_RF_GPO_TGDR_4_TYPE                                            UInt16
#define DSP_RF_GPO_TGDR_4_RESERVED_MASK                                   0x0000F000
#define    DSP_RF_GPO_TGDR_4_DEL_SHIFT                                    0
#define    DSP_RF_GPO_TGDR_4_DEL_MASK                                     0x00000FFF

#define DSP_RF_GPO_TGDR_5_OFFSET                                          0x0000E525
#define DSP_RF_GPO_TGDR_5_TYPE                                            UInt16
#define DSP_RF_GPO_TGDR_5_RESERVED_MASK                                   0x0000F000
#define    DSP_RF_GPO_TGDR_5_DEL_SHIFT                                    0
#define    DSP_RF_GPO_TGDR_5_DEL_MASK                                     0x00000FFF

#define DSP_RF_GPO_TGDR_6_OFFSET                                          0x0000E526
#define DSP_RF_GPO_TGDR_6_TYPE                                            UInt16
#define DSP_RF_GPO_TGDR_6_RESERVED_MASK                                   0x0000F000
#define    DSP_RF_GPO_TGDR_6_DEL_SHIFT                                    0
#define    DSP_RF_GPO_TGDR_6_DEL_MASK                                     0x00000FFF

#define DSP_RF_GPO_TGDR_7_OFFSET                                          0x0000E527
#define DSP_RF_GPO_TGDR_7_TYPE                                            UInt16
#define DSP_RF_GPO_TGDR_7_RESERVED_MASK                                   0x0000F000
#define    DSP_RF_GPO_TGDR_7_DEL_SHIFT                                    0
#define    DSP_RF_GPO_TGDR_7_DEL_MASK                                     0x00000FFF

#define DSP_RF_GPO_TGPR_0_OFFSET                                          0x0000E528
#define DSP_RF_GPO_TGPR_0_TYPE                                            UInt16
#define DSP_RF_GPO_TGPR_0_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_TGPR_0_TGP_SHIFT                                    0
#define    DSP_RF_GPO_TGPR_0_TGP_MASK                                     0x0000FFFF

#define DSP_RF_GPO_TGPR_1_OFFSET                                          0x0000E529
#define DSP_RF_GPO_TGPR_1_TYPE                                            UInt16
#define DSP_RF_GPO_TGPR_1_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_TGPR_1_TGP_SHIFT                                    0
#define    DSP_RF_GPO_TGPR_1_TGP_MASK                                     0x0000FFFF

#define DSP_RF_GPO_TGPR_2_OFFSET                                          0x0000E52A
#define DSP_RF_GPO_TGPR_2_TYPE                                            UInt16
#define DSP_RF_GPO_TGPR_2_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_TGPR_2_TGP_SHIFT                                    0
#define    DSP_RF_GPO_TGPR_2_TGP_MASK                                     0x0000FFFF

#define DSP_RF_GPO_TGPR_3_OFFSET                                          0x0000E52B
#define DSP_RF_GPO_TGPR_3_TYPE                                            UInt16
#define DSP_RF_GPO_TGPR_3_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_TGPR_3_TGP_SHIFT                                    0
#define    DSP_RF_GPO_TGPR_3_TGP_MASK                                     0x0000FFFF

#define DSP_RF_GPO_TGPR_4_OFFSET                                          0x0000E52C
#define DSP_RF_GPO_TGPR_4_TYPE                                            UInt16
#define DSP_RF_GPO_TGPR_4_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_TGPR_4_TGP_SHIFT                                    0
#define    DSP_RF_GPO_TGPR_4_TGP_MASK                                     0x0000FFFF

#define DSP_RF_GPO_TGPR_5_OFFSET                                          0x0000E52D
#define DSP_RF_GPO_TGPR_5_TYPE                                            UInt16
#define DSP_RF_GPO_TGPR_5_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_TGPR_5_TGP_SHIFT                                    0
#define    DSP_RF_GPO_TGPR_5_TGP_MASK                                     0x0000FFFF

#define DSP_RF_GPO_TGPR_6_OFFSET                                          0x0000E52E
#define DSP_RF_GPO_TGPR_6_TYPE                                            UInt16
#define DSP_RF_GPO_TGPR_6_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_TGPR_6_TGP_SHIFT                                    0
#define    DSP_RF_GPO_TGPR_6_TGP_MASK                                     0x0000FFFF

#define DSP_RF_GPO_TGPR_7_OFFSET                                          0x0000E52F
#define DSP_RF_GPO_TGPR_7_TYPE                                            UInt16
#define DSP_RF_GPO_TGPR_7_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_TGPR_7_TGP_SHIFT                                    0
#define    DSP_RF_GPO_TGPR_7_TGP_MASK                                     0x0000FFFF

#define DSP_RF_GPO_RGDR_0_OFFSET                                          0x0000E530
#define DSP_RF_GPO_RGDR_0_TYPE                                            UInt16
#define DSP_RF_GPO_RGDR_0_RESERVED_MASK                                   0x0000F000
#define    DSP_RF_GPO_RGDR_0_DEL_SHIFT                                    0
#define    DSP_RF_GPO_RGDR_0_DEL_MASK                                     0x00000FFF

#define DSP_RF_GPO_RGDR_1_OFFSET                                          0x0000E531
#define DSP_RF_GPO_RGDR_1_TYPE                                            UInt16
#define DSP_RF_GPO_RGDR_1_RESERVED_MASK                                   0x0000F000
#define    DSP_RF_GPO_RGDR_1_DEL_SHIFT                                    0
#define    DSP_RF_GPO_RGDR_1_DEL_MASK                                     0x00000FFF

#define DSP_RF_GPO_RGDR_2_OFFSET                                          0x0000E532
#define DSP_RF_GPO_RGDR_2_TYPE                                            UInt16
#define DSP_RF_GPO_RGDR_2_RESERVED_MASK                                   0x0000F000
#define    DSP_RF_GPO_RGDR_2_DEL_SHIFT                                    0
#define    DSP_RF_GPO_RGDR_2_DEL_MASK                                     0x00000FFF

#define DSP_RF_GPO_RGDR_3_OFFSET                                          0x0000E533
#define DSP_RF_GPO_RGDR_3_TYPE                                            UInt16
#define DSP_RF_GPO_RGDR_3_RESERVED_MASK                                   0x0000F000
#define    DSP_RF_GPO_RGDR_3_DEL_SHIFT                                    0
#define    DSP_RF_GPO_RGDR_3_DEL_MASK                                     0x00000FFF

#define DSP_RF_GPO_RGDR_4_OFFSET                                          0x0000E534
#define DSP_RF_GPO_RGDR_4_TYPE                                            UInt16
#define DSP_RF_GPO_RGDR_4_RESERVED_MASK                                   0x0000F000
#define    DSP_RF_GPO_RGDR_4_DEL_SHIFT                                    0
#define    DSP_RF_GPO_RGDR_4_DEL_MASK                                     0x00000FFF

#define DSP_RF_GPO_RGDR_5_OFFSET                                          0x0000E535
#define DSP_RF_GPO_RGDR_5_TYPE                                            UInt16
#define DSP_RF_GPO_RGDR_5_RESERVED_MASK                                   0x0000F000
#define    DSP_RF_GPO_RGDR_5_DEL_SHIFT                                    0
#define    DSP_RF_GPO_RGDR_5_DEL_MASK                                     0x00000FFF

#define DSP_RF_GPO_RGDR_6_OFFSET                                          0x0000E536
#define DSP_RF_GPO_RGDR_6_TYPE                                            UInt16
#define DSP_RF_GPO_RGDR_6_RESERVED_MASK                                   0x0000F000
#define    DSP_RF_GPO_RGDR_6_DEL_SHIFT                                    0
#define    DSP_RF_GPO_RGDR_6_DEL_MASK                                     0x00000FFF

#define DSP_RF_GPO_RGDR_7_OFFSET                                          0x0000E537
#define DSP_RF_GPO_RGDR_7_TYPE                                            UInt16
#define DSP_RF_GPO_RGDR_7_RESERVED_MASK                                   0x0000F000
#define    DSP_RF_GPO_RGDR_7_DEL_SHIFT                                    0
#define    DSP_RF_GPO_RGDR_7_DEL_MASK                                     0x00000FFF

#define DSP_RF_GPO_RGPR_0_OFFSET                                          0x0000E538
#define DSP_RF_GPO_RGPR_0_TYPE                                            UInt16
#define DSP_RF_GPO_RGPR_0_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_RGPR_0_RGP_SHIFT                                    0
#define    DSP_RF_GPO_RGPR_0_RGP_MASK                                     0x0000FFFF

#define DSP_RF_GPO_RGPR_1_OFFSET                                          0x0000E539
#define DSP_RF_GPO_RGPR_1_TYPE                                            UInt16
#define DSP_RF_GPO_RGPR_1_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_RGPR_1_RGP_SHIFT                                    0
#define    DSP_RF_GPO_RGPR_1_RGP_MASK                                     0x0000FFFF

#define DSP_RF_GPO_RGPR_2_OFFSET                                          0x0000E53A
#define DSP_RF_GPO_RGPR_2_TYPE                                            UInt16
#define DSP_RF_GPO_RGPR_2_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_RGPR_2_RGP_SHIFT                                    0
#define    DSP_RF_GPO_RGPR_2_RGP_MASK                                     0x0000FFFF

#define DSP_RF_GPO_RGPR_3_OFFSET                                          0x0000E53B
#define DSP_RF_GPO_RGPR_3_TYPE                                            UInt16
#define DSP_RF_GPO_RGPR_3_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_RGPR_3_RGP_SHIFT                                    0
#define    DSP_RF_GPO_RGPR_3_RGP_MASK                                     0x0000FFFF

#define DSP_RF_GPO_RGPR_4_OFFSET                                          0x0000E53C
#define DSP_RF_GPO_RGPR_4_TYPE                                            UInt16
#define DSP_RF_GPO_RGPR_4_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_RGPR_4_RGP_SHIFT                                    0
#define    DSP_RF_GPO_RGPR_4_RGP_MASK                                     0x0000FFFF

#define DSP_RF_GPO_RGPR_5_OFFSET                                          0x0000E53D
#define DSP_RF_GPO_RGPR_5_TYPE                                            UInt16
#define DSP_RF_GPO_RGPR_5_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_RGPR_5_RGP_SHIFT                                    0
#define    DSP_RF_GPO_RGPR_5_RGP_MASK                                     0x0000FFFF

#define DSP_RF_GPO_RGPR_6_OFFSET                                          0x0000E53E
#define DSP_RF_GPO_RGPR_6_TYPE                                            UInt16
#define DSP_RF_GPO_RGPR_6_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_RGPR_6_RGP_SHIFT                                    0
#define    DSP_RF_GPO_RGPR_6_RGP_MASK                                     0x0000FFFF

#define DSP_RF_GPO_RGPR_7_OFFSET                                          0x0000E53F
#define DSP_RF_GPO_RGPR_7_TYPE                                            UInt16
#define DSP_RF_GPO_RGPR_7_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_RGPR_7_RGP_SHIFT                                    0
#define    DSP_RF_GPO_RGPR_7_RGP_MASK                                     0x0000FFFF

#define DSP_RF_GPO_TGDR_8_OFFSET                                          0x0000E550
#define DSP_RF_GPO_TGDR_8_TYPE                                            UInt16
#define DSP_RF_GPO_TGDR_8_RESERVED_MASK                                   0x0000F000
#define    DSP_RF_GPO_TGDR_8_DEL_SHIFT                                    0
#define    DSP_RF_GPO_TGDR_8_DEL_MASK                                     0x00000FFF

#define DSP_RF_GPO_TGDR_9_OFFSET                                          0x0000E551
#define DSP_RF_GPO_TGDR_9_TYPE                                            UInt16
#define DSP_RF_GPO_TGDR_9_RESERVED_MASK                                   0x0000F000
#define    DSP_RF_GPO_TGDR_9_DEL_SHIFT                                    0
#define    DSP_RF_GPO_TGDR_9_DEL_MASK                                     0x00000FFF

#define DSP_RF_GPO_TGDR_10_OFFSET                                         0x0000E552
#define DSP_RF_GPO_TGDR_10_TYPE                                           UInt16
#define DSP_RF_GPO_TGDR_10_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_TGDR_10_DEL_SHIFT                                   0
#define    DSP_RF_GPO_TGDR_10_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_TGDR_11_OFFSET                                         0x0000E553
#define DSP_RF_GPO_TGDR_11_TYPE                                           UInt16
#define DSP_RF_GPO_TGDR_11_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_TGDR_11_DEL_SHIFT                                   0
#define    DSP_RF_GPO_TGDR_11_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_TGDR_12_OFFSET                                         0x0000E554
#define DSP_RF_GPO_TGDR_12_TYPE                                           UInt16
#define DSP_RF_GPO_TGDR_12_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_TGDR_12_DEL_SHIFT                                   0
#define    DSP_RF_GPO_TGDR_12_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_TGDR_13_OFFSET                                         0x0000E555
#define DSP_RF_GPO_TGDR_13_TYPE                                           UInt16
#define DSP_RF_GPO_TGDR_13_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_TGDR_13_DEL_SHIFT                                   0
#define    DSP_RF_GPO_TGDR_13_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_TGDR_14_OFFSET                                         0x0000E556
#define DSP_RF_GPO_TGDR_14_TYPE                                           UInt16
#define DSP_RF_GPO_TGDR_14_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_TGDR_14_DEL_SHIFT                                   0
#define    DSP_RF_GPO_TGDR_14_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_TGDR_15_OFFSET                                         0x0000E557
#define DSP_RF_GPO_TGDR_15_TYPE                                           UInt16
#define DSP_RF_GPO_TGDR_15_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_TGDR_15_DEL_SHIFT                                   0
#define    DSP_RF_GPO_TGDR_15_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_TGPR_8_OFFSET                                          0x0000E558
#define DSP_RF_GPO_TGPR_8_TYPE                                            UInt16
#define DSP_RF_GPO_TGPR_8_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_TGPR_8_TGP_SHIFT                                    0
#define    DSP_RF_GPO_TGPR_8_TGP_MASK                                     0x0000FFFF

#define DSP_RF_GPO_TGPR_9_OFFSET                                          0x0000E559
#define DSP_RF_GPO_TGPR_9_TYPE                                            UInt16
#define DSP_RF_GPO_TGPR_9_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_TGPR_9_TGP_SHIFT                                    0
#define    DSP_RF_GPO_TGPR_9_TGP_MASK                                     0x0000FFFF

#define DSP_RF_GPO_TGPR_10_OFFSET                                         0x0000E55A
#define DSP_RF_GPO_TGPR_10_TYPE                                           UInt16
#define DSP_RF_GPO_TGPR_10_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_TGPR_10_TGP_SHIFT                                   0
#define    DSP_RF_GPO_TGPR_10_TGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_TGPR_11_OFFSET                                         0x0000E55B
#define DSP_RF_GPO_TGPR_11_TYPE                                           UInt16
#define DSP_RF_GPO_TGPR_11_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_TGPR_11_TGP_SHIFT                                   0
#define    DSP_RF_GPO_TGPR_11_TGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_TGPR_12_OFFSET                                         0x0000E55C
#define DSP_RF_GPO_TGPR_12_TYPE                                           UInt16
#define DSP_RF_GPO_TGPR_12_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_TGPR_12_TGP_SHIFT                                   0
#define    DSP_RF_GPO_TGPR_12_TGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_TGPR_13_OFFSET                                         0x0000E55D
#define DSP_RF_GPO_TGPR_13_TYPE                                           UInt16
#define DSP_RF_GPO_TGPR_13_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_TGPR_13_TGP_SHIFT                                   0
#define    DSP_RF_GPO_TGPR_13_TGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_TGPR_14_OFFSET                                         0x0000E55E
#define DSP_RF_GPO_TGPR_14_TYPE                                           UInt16
#define DSP_RF_GPO_TGPR_14_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_TGPR_14_TGP_SHIFT                                   0
#define    DSP_RF_GPO_TGPR_14_TGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_TGPR_15_OFFSET                                         0x0000E55F
#define DSP_RF_GPO_TGPR_15_TYPE                                           UInt16
#define DSP_RF_GPO_TGPR_15_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_TGPR_15_TGP_SHIFT                                   0
#define    DSP_RF_GPO_TGPR_15_TGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_RGDR_8_OFFSET                                          0x0000E560
#define DSP_RF_GPO_RGDR_8_TYPE                                            UInt16
#define DSP_RF_GPO_RGDR_8_RESERVED_MASK                                   0x0000F000
#define    DSP_RF_GPO_RGDR_8_DEL_SHIFT                                    0
#define    DSP_RF_GPO_RGDR_8_DEL_MASK                                     0x00000FFF

#define DSP_RF_GPO_RGDR_9_OFFSET                                          0x0000E561
#define DSP_RF_GPO_RGDR_9_TYPE                                            UInt16
#define DSP_RF_GPO_RGDR_9_RESERVED_MASK                                   0x0000F000
#define    DSP_RF_GPO_RGDR_9_DEL_SHIFT                                    0
#define    DSP_RF_GPO_RGDR_9_DEL_MASK                                     0x00000FFF

#define DSP_RF_GPO_RGDR_10_OFFSET                                         0x0000E562
#define DSP_RF_GPO_RGDR_10_TYPE                                           UInt16
#define DSP_RF_GPO_RGDR_10_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_RGDR_10_DEL_SHIFT                                   0
#define    DSP_RF_GPO_RGDR_10_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_RGDR_11_OFFSET                                         0x0000E563
#define DSP_RF_GPO_RGDR_11_TYPE                                           UInt16
#define DSP_RF_GPO_RGDR_11_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_RGDR_11_DEL_SHIFT                                   0
#define    DSP_RF_GPO_RGDR_11_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_RGDR_12_OFFSET                                         0x0000E564
#define DSP_RF_GPO_RGDR_12_TYPE                                           UInt16
#define DSP_RF_GPO_RGDR_12_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_RGDR_12_DEL_SHIFT                                   0
#define    DSP_RF_GPO_RGDR_12_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_RGDR_13_OFFSET                                         0x0000E565
#define DSP_RF_GPO_RGDR_13_TYPE                                           UInt16
#define DSP_RF_GPO_RGDR_13_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_RGDR_13_DEL_SHIFT                                   0
#define    DSP_RF_GPO_RGDR_13_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_RGDR_14_OFFSET                                         0x0000E566
#define DSP_RF_GPO_RGDR_14_TYPE                                           UInt16
#define DSP_RF_GPO_RGDR_14_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_RGDR_14_DEL_SHIFT                                   0
#define    DSP_RF_GPO_RGDR_14_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_RGDR_15_OFFSET                                         0x0000E567
#define DSP_RF_GPO_RGDR_15_TYPE                                           UInt16
#define DSP_RF_GPO_RGDR_15_RESERVED_MASK                                  0x0000F000
#define    DSP_RF_GPO_RGDR_15_DEL_SHIFT                                   0
#define    DSP_RF_GPO_RGDR_15_DEL_MASK                                    0x00000FFF

#define DSP_RF_GPO_RGPR_8_OFFSET                                          0x0000E568
#define DSP_RF_GPO_RGPR_8_TYPE                                            UInt16
#define DSP_RF_GPO_RGPR_8_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_RGPR_8_RGP_SHIFT                                    0
#define    DSP_RF_GPO_RGPR_8_RGP_MASK                                     0x0000FFFF

#define DSP_RF_GPO_RGPR_9_OFFSET                                          0x0000E569
#define DSP_RF_GPO_RGPR_9_TYPE                                            UInt16
#define DSP_RF_GPO_RGPR_9_RESERVED_MASK                                   0x00000000
#define    DSP_RF_GPO_RGPR_9_RGP_SHIFT                                    0
#define    DSP_RF_GPO_RGPR_9_RGP_MASK                                     0x0000FFFF

#define DSP_RF_GPO_RGPR_10_OFFSET                                         0x0000E56A
#define DSP_RF_GPO_RGPR_10_TYPE                                           UInt16
#define DSP_RF_GPO_RGPR_10_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_RGPR_10_RGP_SHIFT                                   0
#define    DSP_RF_GPO_RGPR_10_RGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_RGPR_11_OFFSET                                         0x0000E56B
#define DSP_RF_GPO_RGPR_11_TYPE                                           UInt16
#define DSP_RF_GPO_RGPR_11_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_RGPR_11_RGP_SHIFT                                   0
#define    DSP_RF_GPO_RGPR_11_RGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_RGPR_12_OFFSET                                         0x0000E56C
#define DSP_RF_GPO_RGPR_12_TYPE                                           UInt16
#define DSP_RF_GPO_RGPR_12_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_RGPR_12_RGP_SHIFT                                   0
#define    DSP_RF_GPO_RGPR_12_RGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_RGPR_13_OFFSET                                         0x0000E56D
#define DSP_RF_GPO_RGPR_13_TYPE                                           UInt16
#define DSP_RF_GPO_RGPR_13_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_RGPR_13_RGP_SHIFT                                   0
#define    DSP_RF_GPO_RGPR_13_RGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_RGPR_14_OFFSET                                         0x0000E56E
#define DSP_RF_GPO_RGPR_14_TYPE                                           UInt16
#define DSP_RF_GPO_RGPR_14_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_RGPR_14_RGP_SHIFT                                   0
#define    DSP_RF_GPO_RGPR_14_RGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_RGPR_15_OFFSET                                         0x0000E56F
#define DSP_RF_GPO_RGPR_15_TYPE                                           UInt16
#define DSP_RF_GPO_RGPR_15_RESERVED_MASK                                  0x00000000
#define    DSP_RF_GPO_RGPR_15_RGP_SHIFT                                   0
#define    DSP_RF_GPO_RGPR_15_RGP_MASK                                    0x0000FFFF

#define DSP_RF_GPO_GPWR_OFFSET                                            0x0000E57E
#define DSP_RF_GPO_GPWR_TYPE                                              UInt16
#define DSP_RF_GPO_GPWR_RESERVED_MASK                                     0x00000000
#define    DSP_RF_GPO_GPWR_WGP_SHIFT                                      0
#define    DSP_RF_GPO_GPWR_WGP_MASK                                       0x0000FFFF

#define DSP_RF_GPO_GPSR_OFFSET                                            0x0000E57F
#define DSP_RF_GPO_GPSR_TYPE                                              UInt16
#define DSP_RF_GPO_GPSR_RESERVED_MASK                                     0x00000000
#define    DSP_RF_GPO_GPSR_GPS_SHIFT                                      0
#define    DSP_RF_GPO_GPSR_GPS_MASK                                       0x0000FFFF

#endif /* __BRCM_RDB_DSP_RF_GPO_H__ */


