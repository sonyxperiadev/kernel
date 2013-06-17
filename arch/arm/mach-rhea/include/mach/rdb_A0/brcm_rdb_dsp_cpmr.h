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
/*     Date     : Generated on 5/17/2011 0:56:25                                             */
/*     RDB file : //RHEA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_DSP_CPMR_H__
#define __BRCM_RDB_DSP_CPMR_H__

#define DSP_CPMR_PMSS_MEM_CFG_OFFSET                                      0x00000000
#define DSP_CPMR_PMSS_MEM_CFG_TYPE                                        UInt32
#define DSP_CPMR_PMSS_MEM_CFG_RESERVED_MASK                               0x00000000
#define    DSP_CPMR_PMSS_MEM_CFG_RESERVED_31TO6_SHIFT                     6
#define    DSP_CPMR_PMSS_MEM_CFG_RESERVED_31TO6_MASK                      0xFFFFFFC0
#define    DSP_CPMR_PMSS_MEM_CFG_CACHE_SIZE_SHIFT                         3
#define    DSP_CPMR_PMSS_MEM_CFG_CACHE_SIZE_MASK                          0x00000038
#define    DSP_CPMR_PMSS_MEM_CFG_TCM_SIZE_SHIFT                           0
#define    DSP_CPMR_PMSS_MEM_CFG_TCM_SIZE_MASK                            0x00000007

#define DSP_CPMR_PMSS_CIR_OFFSET                                          0x00000004
#define DSP_CPMR_PMSS_CIR_TYPE                                            UInt32
#define DSP_CPMR_PMSS_CIR_RESERVED_MASK                                   0x00000000
#define    DSP_CPMR_PMSS_CIR_RESERVED_31TO1_SHIFT                         1
#define    DSP_CPMR_PMSS_CIR_RESERVED_31TO1_MASK                          0xFFFFFFFE
#define    DSP_CPMR_PMSS_CIR_CACHE_INVALID_SHIFT                          0
#define    DSP_CPMR_PMSS_CIR_CACHE_INVALID_MASK                           0x00000001

#define DSP_CPMR_PMSS_PCR_OFFSET                                          0x00000008
#define DSP_CPMR_PMSS_PCR_TYPE                                            UInt32
#define DSP_CPMR_PMSS_PCR_RESERVED_MASK                                   0x00000000
#define    DSP_CPMR_PMSS_PCR_RESERVED_31TO7_SHIFT                         7
#define    DSP_CPMR_PMSS_PCR_RESERVED_31TO7_MASK                          0xFFFFFF80
#define    DSP_CPMR_PMSS_PCR_TCM_DISABLE_SHIFT                            6
#define    DSP_CPMR_PMSS_PCR_TCM_DISABLE_MASK                             0x00000040
#define    DSP_CPMR_PMSS_PCR_RESERVED_5_SHIFT                             5
#define    DSP_CPMR_PMSS_PCR_RESERVED_5_MASK                              0x00000020
#define    DSP_CPMR_PMSS_PCR_PRE_FETCH_SIZE_SHIFT                         3
#define    DSP_CPMR_PMSS_PCR_PRE_FETCH_SIZE_MASK                          0x00000018
#define    DSP_CPMR_PMSS_PCR_PRE_FETCH_EN_SHIFT                           2
#define    DSP_CPMR_PMSS_PCR_PRE_FETCH_EN_MASK                            0x00000004
#define    DSP_CPMR_PMSS_PCR_CACHE_LOCK_SHIFT                             1
#define    DSP_CPMR_PMSS_PCR_CACHE_LOCK_MASK                              0x00000002
#define    DSP_CPMR_PMSS_PCR_CACHE_EN_SHIFT                               0
#define    DSP_CPMR_PMSS_PCR_CACHE_EN_MASK                                0x00000001

#define DSP_CPMR_PMSS_PMBA_OFFSET                                         0x0000000C
#define DSP_CPMR_PMSS_PMBA_TYPE                                           UInt32
#define DSP_CPMR_PMSS_PMBA_RESERVED_MASK                                  0x00000000
#define    DSP_CPMR_PMSS_PMBA_PTCM_BASE_ADDR_SHIFT                        0
#define    DSP_CPMR_PMSS_PMBA_PTCM_BASE_ADDR_MASK                         0xFFFFFFFF

#define DSP_CPMR_DMSS_CON_CFG_OFFSET                                      0x00000020
#define DSP_CPMR_DMSS_CON_CFG_TYPE                                        UInt32
#define DSP_CPMR_DMSS_CON_CFG_RESERVED_MASK                               0x00000000
#define    DSP_CPMR_DMSS_CON_CFG_Z_PAGE_31TO16_SHIFT                      16
#define    DSP_CPMR_DMSS_CON_CFG_Z_PAGE_31TO16_MASK                       0xFFFF0000
#define    DSP_CPMR_DMSS_CON_CFG_RESERVED_15TO4_SHIFT                     4
#define    DSP_CPMR_DMSS_CON_CFG_RESERVED_15TO4_MASK                      0x0000FFF0
#define    DSP_CPMR_DMSS_CON_CFG_BIG_ENDIAN_SHIFT                         3
#define    DSP_CPMR_DMSS_CON_CFG_BIG_ENDIAN_MASK                          0x00000008
#define    DSP_CPMR_DMSS_CON_CFG_XYZ_SHIFT                                0
#define    DSP_CPMR_DMSS_CON_CFG_XYZ_MASK                                 0x00000007

#define DSP_CPMR_DMSS_DMBE_OFFSET                                         0x00000024
#define DSP_CPMR_DMSS_DMBE_TYPE                                           UInt32
#define DSP_CPMR_DMSS_DMBE_RESERVED_MASK                                  0x00000000
#define    DSP_CPMR_DMSS_DMBE_RESERVED_31TO2_SHIFT                        2
#define    DSP_CPMR_DMSS_DMBE_RESERVED_31TO2_MASK                         0xFFFFFFFC
#define    DSP_CPMR_DMSS_DMBE_EXT_DATA_PORT_EMPTY_SHIFT                   1
#define    DSP_CPMR_DMSS_DMBE_EXT_DATA_PORT_EMPTY_MASK                    0x00000002
#define    DSP_CPMR_DMSS_DMBE_WRITE_BUFFER_EMPTY_SHIFT                    0
#define    DSP_CPMR_DMSS_DMBE_WRITE_BUFFER_EMPTY_MASK                     0x00000001

#define DSP_CPMR_DMSS_MEM_CFG_OFFSET                                      0x00000028
#define DSP_CPMR_DMSS_MEM_CFG_TYPE                                        UInt32
#define DSP_CPMR_DMSS_MEM_CFG_RESERVED_MASK                               0x00000000
#define    DSP_CPMR_DMSS_MEM_CFG_RESERVED_31TO8_SHIFT                     8
#define    DSP_CPMR_DMSS_MEM_CFG_RESERVED_31TO8_MASK                      0xFFFFFF00
#define    DSP_CPMR_DMSS_MEM_CFG_Z_SIZE_SHIFT                             5
#define    DSP_CPMR_DMSS_MEM_CFG_Z_SIZE_MASK                              0x000000E0
#define    DSP_CPMR_DMSS_MEM_CFG_TCM_SIZE_SHIFT                           2
#define    DSP_CPMR_DMSS_MEM_CFG_TCM_SIZE_MASK                            0x0000001C
#define    DSP_CPMR_DMSS_MEM_CFG_BLOCK_NUMBER_SHIFT                       0
#define    DSP_CPMR_DMSS_MEM_CFG_BLOCK_NUMBER_MASK                        0x00000003

#define DSP_CPMR_DMSS_DMBA_OFFSET                                         0x0000002C
#define DSP_CPMR_DMSS_DMBA_TYPE                                           UInt32
#define DSP_CPMR_DMSS_DMBA_RESERVED_MASK                                  0x00000000
#define    DSP_CPMR_DMSS_DMBA_DTCM_BASE_ADDR_SHIFT                        0
#define    DSP_CPMR_DMSS_DMBA_DTCM_BASE_ADDR_MASK                         0xFFFFFFFF

#endif /* __BRCM_RDB_DSP_CPMR_H__ */


