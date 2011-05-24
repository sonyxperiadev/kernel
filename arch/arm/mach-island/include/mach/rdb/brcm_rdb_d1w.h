/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
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
/*     Date     : Generated on 4/25/2011 11:6:8                                             */
/*     RDB file : /projects/BIGISLAND/revA0                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_D1W_H__
#define __BRCM_RDB_D1W_H__

#define D1W_DATA_IN_REG_OFFSET                                            0x00000000
#define D1W_DATA_IN_REG_TYPE                                              UInt32
#define D1W_DATA_IN_REG_RESERVED_MASK                                     0xFFFFFF00
#define    D1W_DATA_IN_REG_UNION_SHIFT                                    0
#define    D1W_DATA_IN_REG_UNION_MASK                                     0x000000FF

#define D1W_DATA_OUT_REG_OFFSET                                           0x00000004
#define D1W_DATA_OUT_REG_TYPE                                             UInt32
#define D1W_DATA_OUT_REG_RESERVED_MASK                                    0xFFFFFF00
#define    D1W_DATA_OUT_REG_UNION_SHIFT                                   0
#define    D1W_DATA_OUT_REG_UNION_MASK                                    0x000000FF

#define D1W_ADDRESS_REG_OFFSET                                            0x00000008
#define D1W_ADDRESS_REG_TYPE                                              UInt32
#define D1W_ADDRESS_REG_RESERVED_MASK                                     0xFFFFFFF8
#define    D1W_ADDRESS_REG_DIN_SHIFT                                      0
#define    D1W_ADDRESS_REG_DIN_MASK                                       0x00000007
#define       D1W_ADDRESS_REG_DIN_CMD_ADDR_OW_REG_CONTROL                 0x00000005
#define       D1W_ADDRESS_REG_DIN_CMD_ADDR_OW_REG_CLOCK                   0x00000004
#define       D1W_ADDRESS_REG_DIN_CMD_ADDR_OW_REG_INTERRUPT_ENB           0x00000003
#define       D1W_ADDRESS_REG_DIN_CMD_ADDR_OW_REG_INTERRUPT               0x00000002
#define       D1W_ADDRESS_REG_DIN_CMD_ADDR_OW_REG_TRANSFER                0x00000001
#define       D1W_ADDRESS_REG_DIN_CMD_ADDR_OW_REG_COMMAND                 0x00000000

#define D1W_CONTROL_REG_OFFSET                                            0x0000000C
#define D1W_CONTROL_REG_TYPE                                              UInt32
#define D1W_CONTROL_REG_RESERVED_MASK                                     0xFFFFFFFC
#define    D1W_CONTROL_REG_ADS_SHIFT                                      1
#define    D1W_CONTROL_REG_ADS_MASK                                       0x00000002
#define    D1W_CONTROL_REG_RDWR_SHIFT                                     0
#define    D1W_CONTROL_REG_RDWR_MASK                                      0x00000001

#endif /* __BRCM_RDB_D1W_H__ */


