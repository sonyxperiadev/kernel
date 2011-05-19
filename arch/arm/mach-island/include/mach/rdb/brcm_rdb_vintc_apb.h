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

#ifndef __BRCM_RDB_VINTC_APB_H__
#define __BRCM_RDB_VINTC_APB_H__

#define VINTC_APB_PERI2VPMINTPEND_OFFSET                                  0x00000000
#define VINTC_APB_PERI2VPMINTPEND_TYPE                                    UInt32
#define VINTC_APB_PERI2VPMINTPEND_RESERVED_MASK                           0xFFFF0000
#define    VINTC_APB_PERI2VPMINTPEND_DMA1CH7_2_VPM_SHIFT                  15
#define    VINTC_APB_PERI2VPMINTPEND_DMA1CH7_2_VPM_MASK                   0x00008000
#define    VINTC_APB_PERI2VPMINTPEND_DMA1CH6_2_VPM_SHIFT                  14
#define    VINTC_APB_PERI2VPMINTPEND_DMA1CH6_2_VPM_MASK                   0x00004000
#define    VINTC_APB_PERI2VPMINTPEND_DMA1CH5_2_VPM_SHIFT                  13
#define    VINTC_APB_PERI2VPMINTPEND_DMA1CH5_2_VPM_MASK                   0x00002000
#define    VINTC_APB_PERI2VPMINTPEND_DMA1CH4_2_VPM_SHIFT                  12
#define    VINTC_APB_PERI2VPMINTPEND_DMA1CH4_2_VPM_MASK                   0x00001000
#define    VINTC_APB_PERI2VPMINTPEND_DMA1CH3_2_VPM_SHIFT                  11
#define    VINTC_APB_PERI2VPMINTPEND_DMA1CH3_2_VPM_MASK                   0x00000800
#define    VINTC_APB_PERI2VPMINTPEND_DMA1CH2_2_VPM_SHIFT                  10
#define    VINTC_APB_PERI2VPMINTPEND_DMA1CH2_2_VPM_MASK                   0x00000400
#define    VINTC_APB_PERI2VPMINTPEND_DMA1CH1_2_VPM_SHIFT                  9
#define    VINTC_APB_PERI2VPMINTPEND_DMA1CH1_2_VPM_MASK                   0x00000200
#define    VINTC_APB_PERI2VPMINTPEND_DMA1CH0_2_VPM_SHIFT                  8
#define    VINTC_APB_PERI2VPMINTPEND_DMA1CH0_2_VPM_MASK                   0x00000100
#define    VINTC_APB_PERI2VPMINTPEND_DMA0CH7_2_VPM_SHIFT                  7
#define    VINTC_APB_PERI2VPMINTPEND_DMA0CH7_2_VPM_MASK                   0x00000080
#define    VINTC_APB_PERI2VPMINTPEND_DMA0CH6_2_VPM_SHIFT                  6
#define    VINTC_APB_PERI2VPMINTPEND_DMA0CH6_2_VPM_MASK                   0x00000040
#define    VINTC_APB_PERI2VPMINTPEND_DMA0CH5_2_VPM_SHIFT                  5
#define    VINTC_APB_PERI2VPMINTPEND_DMA0CH5_2_VPM_MASK                   0x00000020
#define    VINTC_APB_PERI2VPMINTPEND_DMA0CH4_2_VPM_SHIFT                  4
#define    VINTC_APB_PERI2VPMINTPEND_DMA0CH4_2_VPM_MASK                   0x00000010
#define    VINTC_APB_PERI2VPMINTPEND_DMA0CH3_2_VPM_SHIFT                  3
#define    VINTC_APB_PERI2VPMINTPEND_DMA0CH3_2_VPM_MASK                   0x00000008
#define    VINTC_APB_PERI2VPMINTPEND_DMA0CH2_2_VPM_SHIFT                  2
#define    VINTC_APB_PERI2VPMINTPEND_DMA0CH2_2_VPM_MASK                   0x00000004
#define    VINTC_APB_PERI2VPMINTPEND_DMA0CH1_2_VPM_SHIFT                  1
#define    VINTC_APB_PERI2VPMINTPEND_DMA0CH1_2_VPM_MASK                   0x00000002
#define    VINTC_APB_PERI2VPMINTPEND_DMA0CH0_2_VPM_SHIFT                  0
#define    VINTC_APB_PERI2VPMINTPEND_DMA0CH0_2_VPM_MASK                   0x00000001

#define VINTC_APB_PERI2VPMINTMASK_OFFSET                                  0x00000004
#define VINTC_APB_PERI2VPMINTMASK_TYPE                                    UInt32
#define VINTC_APB_PERI2VPMINTMASK_RESERVED_MASK                           0xFFFF0000
#define    VINTC_APB_PERI2VPMINTMASK_DMA1CH7_2_VPM_MASK_SHIFT             15
#define    VINTC_APB_PERI2VPMINTMASK_DMA1CH7_2_VPM_MASK_MASK              0x00008000
#define    VINTC_APB_PERI2VPMINTMASK_DMA1CH6_2_VPM_MASK_SHIFT             14
#define    VINTC_APB_PERI2VPMINTMASK_DMA1CH6_2_VPM_MASK_MASK              0x00004000
#define    VINTC_APB_PERI2VPMINTMASK_DMA1CH5_2_VPM_MASK_SHIFT             13
#define    VINTC_APB_PERI2VPMINTMASK_DMA1CH5_2_VPM_MASK_MASK              0x00002000
#define    VINTC_APB_PERI2VPMINTMASK_DMA1CH4_2_VPM_MASK_SHIFT             12
#define    VINTC_APB_PERI2VPMINTMASK_DMA1CH4_2_VPM_MASK_MASK              0x00001000
#define    VINTC_APB_PERI2VPMINTMASK_DMA1CH3_2_VPM_MASK_SHIFT             11
#define    VINTC_APB_PERI2VPMINTMASK_DMA1CH3_2_VPM_MASK_MASK              0x00000800
#define    VINTC_APB_PERI2VPMINTMASK_DMA1CH2_2_VPM_MASK_SHIFT             10
#define    VINTC_APB_PERI2VPMINTMASK_DMA1CH2_2_VPM_MASK_MASK              0x00000400
#define    VINTC_APB_PERI2VPMINTMASK_DMA1CH1_2_VPM_MASK_SHIFT             9
#define    VINTC_APB_PERI2VPMINTMASK_DMA1CH1_2_VPM_MASK_MASK              0x00000200
#define    VINTC_APB_PERI2VPMINTMASK_DMA1CH0_2_VPM_MASK_SHIFT             8
#define    VINTC_APB_PERI2VPMINTMASK_DMA1CH0_2_VPM_MASK_MASK              0x00000100
#define    VINTC_APB_PERI2VPMINTMASK_DMA0CH7_2_VPM_MASK_SHIFT             7
#define    VINTC_APB_PERI2VPMINTMASK_DMA0CH7_2_VPM_MASK_MASK              0x00000080
#define    VINTC_APB_PERI2VPMINTMASK_DMA0CH6_2_VPM_MASK_SHIFT             6
#define    VINTC_APB_PERI2VPMINTMASK_DMA0CH6_2_VPM_MASK_MASK              0x00000040
#define    VINTC_APB_PERI2VPMINTMASK_DMA0CH5_2_VPM_MASK_SHIFT             5
#define    VINTC_APB_PERI2VPMINTMASK_DMA0CH5_2_VPM_MASK_MASK              0x00000020
#define    VINTC_APB_PERI2VPMINTMASK_DMA0CH4_2_VPM_MASK_SHIFT             4
#define    VINTC_APB_PERI2VPMINTMASK_DMA0CH4_2_VPM_MASK_MASK              0x00000010
#define    VINTC_APB_PERI2VPMINTMASK_DMA0CH3_2_VPM_MASK_SHIFT             3
#define    VINTC_APB_PERI2VPMINTMASK_DMA0CH3_2_VPM_MASK_MASK              0x00000008
#define    VINTC_APB_PERI2VPMINTMASK_DMA0CH2_2_VPM_MASK_SHIFT             2
#define    VINTC_APB_PERI2VPMINTMASK_DMA0CH2_2_VPM_MASK_MASK              0x00000004
#define    VINTC_APB_PERI2VPMINTMASK_DMA0CH1_2_VPM_MASK_SHIFT             1
#define    VINTC_APB_PERI2VPMINTMASK_DMA0CH1_2_VPM_MASK_MASK              0x00000002
#define    VINTC_APB_PERI2VPMINTMASK_DMA0CH0_2_VPM_MASK_SHIFT             0
#define    VINTC_APB_PERI2VPMINTMASK_DMA0CH0_2_VPM_MASK_MASK              0x00000001

#define VINTC_APB_PERI2VPMINTSTATUS_OFFSET                                0x00000008
#define VINTC_APB_PERI2VPMINTSTATUS_TYPE                                  UInt32
#define VINTC_APB_PERI2VPMINTSTATUS_RESERVED_MASK                         0x7FFF0000
#define    VINTC_APB_PERI2VPMINTSTATUS_TEST_ENB_REG_SHIFT                 31
#define    VINTC_APB_PERI2VPMINTSTATUS_TEST_ENB_REG_MASK                  0x80000000
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA1CH7_2_VPM_SHIFT                15
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA1CH7_2_VPM_MASK                 0x00008000
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA1CH6_2_VPM_SHIFT                14
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA1CH6_2_VPM_MASK                 0x00004000
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA1CH5_2_VPM_SHIFT                13
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA1CH5_2_VPM_MASK                 0x00002000
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA1CH4_2_VPM_SHIFT                12
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA1CH4_2_VPM_MASK                 0x00001000
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA1CH3_2_VPM_SHIFT                11
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA1CH3_2_VPM_MASK                 0x00000800
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA1CH2_2_VPM_SHIFT                10
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA1CH2_2_VPM_MASK                 0x00000400
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA1CH1_2_VPM_SHIFT                9
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA1CH1_2_VPM_MASK                 0x00000200
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA1CH0_2_VPM_SHIFT                8
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA1CH0_2_VPM_MASK                 0x00000100
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA0CH7_2_VPM_SHIFT                7
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA0CH7_2_VPM_MASK                 0x00000080
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA0CH6_2_VPM_SHIFT                6
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA0CH6_2_VPM_MASK                 0x00000040
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA0CH5_2_VPM_SHIFT                5
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA0CH5_2_VPM_MASK                 0x00000020
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA0CH4_2_VPM_SHIFT                4
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA0CH4_2_VPM_MASK                 0x00000010
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA0CH3_2_VPM_SHIFT                3
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA0CH3_2_VPM_MASK                 0x00000008
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA0CH2_2_VPM_SHIFT                2
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA0CH2_2_VPM_MASK                 0x00000004
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA0CH1_2_VPM_SHIFT                1
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA0CH1_2_VPM_MASK                 0x00000002
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA0CH0_2_VPM_SHIFT                0
#define    VINTC_APB_PERI2VPMINTSTATUS_DMA0CH0_2_VPM_MASK                 0x00000001

#define VINTC_APB_HOST2VPMINT_OFFSET                                      0x0000000C
#define VINTC_APB_HOST2VPMINT_TYPE                                        UInt32
#define VINTC_APB_HOST2VPMINT_RESERVED_MASK                               0xFFFFFFFE
#define    VINTC_APB_HOST2VPMINT_INTERRUPT_2_VPM_SHIFT                    0
#define    VINTC_APB_HOST2VPMINT_INTERRUPT_2_VPM_MASK                     0x00000001

#define VINTC_APB_VPM2HOSTINT_OFFSET                                      0x00000010
#define VINTC_APB_VPM2HOSTINT_TYPE                                        UInt32
#define VINTC_APB_VPM2HOSTINT_RESERVED_MASK                               0xFFFFFFFE
#define    VINTC_APB_VPM2HOSTINT_VPM_INTERRUPT_2_ARM_SHIFT                0
#define    VINTC_APB_VPM2HOSTINT_VPM_INTERRUPT_2_ARM_MASK                 0x00000001

#define VINTC_APB_VPMBOOTEN_OFFSET                                        0x00000014
#define VINTC_APB_VPMBOOTEN_TYPE                                          UInt32
#define VINTC_APB_VPMBOOTEN_RESERVED_MASK                                 0xFFFFFFFE
#define    VINTC_APB_VPMBOOTEN_VPM_BOOT_EN_SHIFT                          0
#define    VINTC_APB_VPMBOOTEN_VPM_BOOT_EN_MASK                           0x00000001

#define VINTC_APB_VPMBOOTVECTOR_OFFSET                                    0x00000018
#define VINTC_APB_VPMBOOTVECTOR_TYPE                                      UInt32
#define VINTC_APB_VPMBOOTVECTOR_RESERVED_MASK                             0x00000000
#define    VINTC_APB_VPMBOOTVECTOR_VPM_BOOT_EN_SHIFT                      0
#define    VINTC_APB_VPMBOOTVECTOR_VPM_BOOT_EN_MASK                       0xFFFFFFFF

#define VINTC_APB_VPMRESET_OFFSET                                         0x0000001C
#define VINTC_APB_VPMRESET_TYPE                                           UInt32
#define VINTC_APB_VPMRESET_RESERVED_MASK                                  0xFFFFFFFC
#define    VINTC_APB_VPMRESET_SPARE1_SHIFT                                1
#define    VINTC_APB_VPMRESET_SPARE1_MASK                                 0x00000002
#define    VINTC_APB_VPMRESET_SPARE2_SHIFT                                0
#define    VINTC_APB_VPMRESET_SPARE2_MASK                                 0x00000001

#define VINTC_APB_VPMEXTERNALWAIT_OFFSET                                  0x00000020
#define VINTC_APB_VPMEXTERNALWAIT_TYPE                                    UInt32
#define VINTC_APB_VPMEXTERNALWAIT_RESERVED_MASK                           0xFFFFFFFE
#define    VINTC_APB_VPMEXTERNALWAIT_EXTERNAL_WAIT_SHIFT                  0
#define    VINTC_APB_VPMEXTERNALWAIT_EXTERNAL_WAIT_MASK                   0x00000001

#define VINTC_APB_VPMSPARE_OFFSET                                         0x00000024
#define VINTC_APB_VPMSPARE_TYPE                                           UInt32
#define VINTC_APB_VPMSPARE_RESERVED_MASK                                  0x00000000
#define    VINTC_APB_VPMSPARE_VPM_SPARE_REG_SHIFT                         0
#define    VINTC_APB_VPMSPARE_VPM_SPARE_REG_MASK                          0xFFFFFFFF

#endif /* __BRCM_RDB_VINTC_APB_H__ */


