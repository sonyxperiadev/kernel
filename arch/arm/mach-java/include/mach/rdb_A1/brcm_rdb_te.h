/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2013  Broadcom Corporation                                                        */
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
/*     Date     : Generated on 6/27/2013 16:58:22                                             */
/*     RDB file : //JAVA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_TE_H__
#define __BRCM_RDB_TE_H__

#define TE_0C_OFFSET                                                      0x00000000
#define TE_0C_TYPE                                                        UInt32
#define TE_0C_RESERVED_MASK                                               0xFFF80000
#define    TE_0C_HSLINE_SHIFT                                             3
#define    TE_0C_HSLINE_MASK                                              0x0007FFF8
#define    TE_0C_POL_SHIFT                                                2
#define    TE_0C_POL_MASK                                                 0x00000004
#define    TE_0C_MODE_SHIFT                                               1
#define    TE_0C_MODE_MASK                                                0x00000002
#define    TE_0C_TE_EN_SHIFT                                              0
#define    TE_0C_TE_EN_MASK                                               0x00000001

#define TE_0VSWIDTH_OFFSET                                                0x00000004
#define TE_0VSWIDTH_TYPE                                                  UInt32
#define TE_0VSWIDTH_RESERVED_MASK                                         0x00000000
#define    TE_0VSWIDTH_TE_VSWIDTH_SHIFT                                   0
#define    TE_0VSWIDTH_TE_VSWIDTH_MASK                                    0xFFFFFFFF

#define TE_1C_OFFSET                                                      0x00000008
#define TE_1C_TYPE                                                        UInt32
#define TE_1C_RESERVED_MASK                                               0xFFF80000
#define    TE_1C_HSLINE_SHIFT                                             3
#define    TE_1C_HSLINE_MASK                                              0x0007FFF8
#define    TE_1C_POL_SHIFT                                                2
#define    TE_1C_POL_MASK                                                 0x00000004
#define    TE_1C_MODE_SHIFT                                               1
#define    TE_1C_MODE_MASK                                                0x00000002
#define    TE_1C_TE_EN_SHIFT                                              0
#define    TE_1C_TE_EN_MASK                                               0x00000001

#define TE_1VSWIDTH_OFFSET                                                0x0000000C
#define TE_1VSWIDTH_TYPE                                                  UInt32
#define TE_1VSWIDTH_RESERVED_MASK                                         0x00000000
#define    TE_1VSWIDTH_TE_VSWIDTH_SHIFT                                   0
#define    TE_1VSWIDTH_TE_VSWIDTH_MASK                                    0xFFFFFFFF

#define TE_2C_OFFSET                                                      0x00000010
#define TE_2C_TYPE                                                        UInt32
#define TE_2C_RESERVED_MASK                                               0xFFF80000
#define    TE_2C_HSLINE_SHIFT                                             3
#define    TE_2C_HSLINE_MASK                                              0x0007FFF8
#define    TE_2C_POL_SHIFT                                                2
#define    TE_2C_POL_MASK                                                 0x00000004
#define    TE_2C_MODE_SHIFT                                               1
#define    TE_2C_MODE_MASK                                                0x00000002
#define    TE_2C_TE_EN_SHIFT                                              0
#define    TE_2C_TE_EN_MASK                                               0x00000001

#define TE_2VSWIDTH_OFFSET                                                0x00000014
#define TE_2VSWIDTH_TYPE                                                  UInt32
#define TE_2VSWIDTH_RESERVED_MASK                                         0x00000000
#define    TE_2VSWIDTH_TE_VSWIDTH_SHIFT                                   0
#define    TE_2VSWIDTH_TE_VSWIDTH_MASK                                    0xFFFFFFFF

#define TE_MUX_CTL_OFFSET                                                 0x00000018
#define TE_MUX_CTL_TYPE                                                   UInt32
#define TE_MUX_CTL_RESERVED_MASK                                          0xFFCCCCCC
#define    TE_MUX_CTL_SPI_TE_TRIG_SHIFT                                   20
#define    TE_MUX_CTL_SPI_TE_TRIG_MASK                                    0x00300000
#define    TE_MUX_CTL_SMI_TE_TRIG_SHIFT                                   16
#define    TE_MUX_CTL_SMI_TE_TRIG_MASK                                    0x00030000
#define    TE_MUX_CTL_DSI1_TE1_TRIG_SHIFT                                 12
#define    TE_MUX_CTL_DSI1_TE1_TRIG_MASK                                  0x00003000
#define    TE_MUX_CTL_DSI1_TE0_TRIG_SHIFT                                 8
#define    TE_MUX_CTL_DSI1_TE0_TRIG_MASK                                  0x00000300
#define    TE_MUX_CTL_DSI0_TE1_TRIG_SHIFT                                 4
#define    TE_MUX_CTL_DSI0_TE1_TRIG_MASK                                  0x00000030
#define    TE_MUX_CTL_DSI0_TE0_TRIG_SHIFT                                 0
#define    TE_MUX_CTL_DSI0_TE0_TRIG_MASK                                  0x00000003

#endif /* __BRCM_RDB_TE_H__ */


