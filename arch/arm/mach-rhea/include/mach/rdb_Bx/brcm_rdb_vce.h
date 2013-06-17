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

#ifndef __BRCM_RDB_VCE_H__
#define __BRCM_RDB_VCE_H__

#define VCE_DMEM_ACCESS_OFFSET                                            0x00000000
#define VCE_DMEM_ACCESS_TYPE                                              UInt32
#define VCE_DMEM_ACCESS_RESERVED_MASK                                     0x00000000
#define    VCE_DMEM_ACCESS_DMEM_DATA_SHIFT                                0
#define    VCE_DMEM_ACCESS_DMEM_DATA_MASK                                 0xFFFFFFFF

#define VCE_PMEM_ACCESS_OFFSET                                            0x00010000
#define VCE_PMEM_ACCESS_TYPE                                              UInt32
#define VCE_PMEM_ACCESS_RESERVED_MASK                                     0x00000000
#define    VCE_PMEM_ACCESS_PMEM_DATA_SHIFT                                0
#define    VCE_PMEM_ACCESS_PMEM_DATA_MASK                                 0xFFFFFFFF

#define VCE_GPRF_ACCESS_OFFSET                                            0x00020000
#define VCE_GPRF_ACCESS_TYPE                                              UInt32
#define VCE_GPRF_ACCESS_RESERVED_MASK                                     0x00000000
#define    VCE_GPRF_ACCESS_GPRF_DATA_SHIFT                                0
#define    VCE_GPRF_ACCESS_GPRF_DATA_MASK                                 0xFFFFFFFF

#define VCE_STATUS_OFFSET                                                 0x00040000
#define VCE_STATUS_TYPE                                                   UInt32
#define VCE_STATUS_RESERVED_MASK                                          0x7CE00000
#define    VCE_STATUS_VCE_INTERRUPT_POS_SHIFT                             31
#define    VCE_STATUS_VCE_INTERRUPT_POS_MASK                              0x80000000
#define    VCE_STATUS_VCE_NANOFLAG_POS_SHIFT                              25
#define    VCE_STATUS_VCE_NANOFLAG_POS_MASK                               0x02000000
#define    VCE_STATUS_VCE_RUNNING_POS_SHIFT                               24
#define    VCE_STATUS_VCE_RUNNING_POS_MASK                                0x01000000
#define    VCE_STATUS_VCE_REASON_POS_SHIFT                                16
#define    VCE_STATUS_VCE_REASON_POS_MASK                                 0x001F0000
#define    VCE_STATUS_VCE_BUSY_BITFIELD_SHIFT                             0
#define    VCE_STATUS_VCE_BUSY_BITFIELD_MASK                              0x0000FFFF

#define VCE_VERSION_OFFSET                                                0x00040004
#define VCE_VERSION_TYPE                                                  UInt32
#define VCE_VERSION_RESERVED_MASK                                         0x0000000F
#define    VCE_VERSION_SPEC_REVISION_SHIFT                                24
#define    VCE_VERSION_SPEC_REVISION_MASK                                 0xFF000000
#define    VCE_VERSION_SIM_SHIFT                                          20
#define    VCE_VERSION_SIM_MASK                                           0x00F00000
#define    VCE_VERSION_SUB_REVISION_SHIFT                                 4
#define    VCE_VERSION_SUB_REVISION_MASK                                  0x000FFFF0

#define VCE_PC_PF0_OFFSET                                                 0x00040008
#define VCE_PC_PF0_TYPE                                                   UInt32
#define VCE_PC_PF0_RESERVED_MASK                                          0x00000000
#define    VCE_PC_PF0_PC_PF0_SHIFT                                        0
#define    VCE_PC_PF0_PC_PF0_MASK                                         0xFFFFFFFF

#define VCE_PC_IF0_OFFSET                                                 0x0004000C
#define VCE_PC_IF0_TYPE                                                   UInt32
#define VCE_PC_IF0_RESERVED_MASK                                          0x00000000
#define    VCE_PC_IF0_PC_IF0_SHIFT                                        0
#define    VCE_PC_IF0_PC_IF0_MASK                                         0xFFFFFFFF

#define VCE_PC_RD0_OFFSET                                                 0x00040010
#define VCE_PC_RD0_TYPE                                                   UInt32
#define VCE_PC_RD0_RESERVED_MASK                                          0x00000000
#define    VCE_PC_RD0_PC_RD0_SHIFT                                        0
#define    VCE_PC_RD0_PC_RD0_MASK                                         0xFFFFFFFF

#define VCE_PC_EX0_OFFSET                                                 0x00040014
#define VCE_PC_EX0_TYPE                                                   UInt32
#define VCE_PC_EX0_RESERVED_MASK                                          0x00000000
#define    VCE_PC_EX0_PC_EX0_SHIFT                                        0
#define    VCE_PC_EX0_PC_EX0_MASK                                         0xFFFFFFFF

#define VCE_CONTROL_OFFSET                                                0x00040020
#define VCE_CONTROL_TYPE                                                  UInt32
#define VCE_CONTROL_RESERVED_MASK                                         0xFFFFFFFC
#define    VCE_CONTROL_RUN_BIT_CMD_SHIFT                                  0
#define    VCE_CONTROL_RUN_BIT_CMD_MASK                                   0x00000003

#define VCE_SEMA_CLEAR_OFFSET                                             0x00040024
#define VCE_SEMA_CLEAR_TYPE                                               UInt32
#define VCE_SEMA_CLEAR_RESERVED_MASK                                      0x7DFFFF00
#define    VCE_SEMA_CLEAR_CLR_INT_REQ_SHIFT                               31
#define    VCE_SEMA_CLEAR_CLR_INT_REQ_MASK                                0x80000000
#define    VCE_SEMA_CLEAR_CLR_NANO_FLAG_SHIFT                             25
#define    VCE_SEMA_CLEAR_CLR_NANO_FLAG_MASK                              0x02000000
#define    VCE_SEMA_CLEAR_CLR_USER_SEMA_SHIFT                             1
#define    VCE_SEMA_CLEAR_CLR_USER_SEMA_MASK                              0x000000FE
#define    VCE_SEMA_CLEAR_CLR_BKPT_SEMA_SHIFT                             0
#define    VCE_SEMA_CLEAR_CLR_BKPT_SEMA_MASK                              0x00000001

#define VCE_SEMA_SET_OFFSET                                               0x00040028
#define VCE_SEMA_SET_TYPE                                                 UInt32
#define VCE_SEMA_SET_RESERVED_MASK                                        0xFDFFFF00
#define    VCE_SEMA_SET_SET_NANO_FLAG_SHIFT                               25
#define    VCE_SEMA_SET_SET_NANO_FLAG_MASK                                0x02000000
#define    VCE_SEMA_SET_SET_USER_SEMA_SHIFT                               1
#define    VCE_SEMA_SET_SET_USER_SEMA_MASK                                0x000000FE
#define    VCE_SEMA_SET_SET_BKPT_SEMA_SHIFT                               0
#define    VCE_SEMA_SET_SET_BKPT_SEMA_MASK                                0x00000001

#define VCE_BAD_ADDR_OFFSET                                               0x00040030
#define VCE_BAD_ADDR_TYPE                                                 UInt32
#define VCE_BAD_ADDR_RESERVED_MASK                                        0x00000000
#define    VCE_BAD_ADDR_OUT_OF_RANGE_SHIFT                                31
#define    VCE_BAD_ADDR_OUT_OF_RANGE_MASK                                 0x80000000
#define    VCE_BAD_ADDR_INSTR_ERR_SHIFT                                   30
#define    VCE_BAD_ADDR_INSTR_ERR_MASK                                    0x40000000
#define    VCE_BAD_ADDR_BANG_IN_ERR_SHIFT                                 29
#define    VCE_BAD_ADDR_BANG_IN_ERR_MASK                                  0x20000000
#define    VCE_BAD_ADDR_BANG_OUT_ERR_SHIFT                                28
#define    VCE_BAD_ADDR_BANG_OUT_ERR_MASK                                 0x10000000
#define    VCE_BAD_ADDR_DMA_IN_ERR_SHIFT                                  27
#define    VCE_BAD_ADDR_DMA_IN_ERR_MASK                                   0x08000000
#define    VCE_BAD_ADDR_DMA_OUT_ERR_SHIFT                                 26
#define    VCE_BAD_ADDR_DMA_OUT_ERR_MASK                                  0x04000000
#define    VCE_BAD_ADDR_DMAINIT_ERR_SHIFT                                 25
#define    VCE_BAD_ADDR_DMAINIT_ERR_MASK                                  0x02000000
#define    VCE_BAD_ADDR_SD_ACCESS_ERR_SHIFT                               24
#define    VCE_BAD_ADDR_SD_ACCESS_ERR_MASK                                0x01000000
#define    VCE_BAD_ADDR_DMEM_ADDR_SHIFT                                   0
#define    VCE_BAD_ADDR_DMEM_ADDR_MASK                                    0x00FFFFFF

#define VCE_PC_ERR_OFFSET                                                 0x00040034
#define VCE_PC_ERR_TYPE                                                   UInt32
#define VCE_PC_ERR_RESERVED_MASK                                          0x00000000
#define    VCE_PC_ERR_PC_VCE_ERR_SHIFT                                    0
#define    VCE_PC_ERR_PC_VCE_ERR_MASK                                     0xFFFFFFFF

#endif /* __BRCM_RDB_VCE_H__ */


