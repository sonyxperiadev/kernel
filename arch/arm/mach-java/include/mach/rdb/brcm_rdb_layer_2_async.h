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
/*     Date     : Generated on 3/4/2013 11:52:5                                             */
/*     RDB file : //JAVA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_LAYER_2_ASYNC_H__
#define __BRCM_RDB_LAYER_2_ASYNC_H__

#define LAYER_2_ASYNC_ISTAT_OFFSET                                        0x00000000
#define LAYER_2_ASYNC_ISTAT_TYPE                                          UInt32
#define LAYER_2_ASYNC_ISTAT_RESERVED_MASK                                 0xFFFF0000
#define    LAYER_2_ASYNC_ISTAT_ISTAT_SHIFT                                0
#define    LAYER_2_ASYNC_ISTAT_ISTAT_MASK                                 0x0000FFFF

#define LAYER_2_ASYNC_IRSTAT_OFFSET                                       0x00000004
#define LAYER_2_ASYNC_IRSTAT_TYPE                                         UInt32
#define LAYER_2_ASYNC_IRSTAT_RESERVED_MASK                                0xFFFF0000
#define    LAYER_2_ASYNC_IRSTAT_IRSTAT_SHIFT                              0
#define    LAYER_2_ASYNC_IRSTAT_IRSTAT_MASK                               0x0000FFFF

#define LAYER_2_ASYNC_IENSET_OFFSET                                       0x00000008
#define LAYER_2_ASYNC_IENSET_TYPE                                         UInt32
#define LAYER_2_ASYNC_IENSET_RESERVED_MASK                                0xFFFF0000
#define    LAYER_2_ASYNC_IENSET_IENSET_SHIFT                              0
#define    LAYER_2_ASYNC_IENSET_IENSET_MASK                               0x0000FFFF

#define LAYER_2_ASYNC_IENCLR_OFFSET                                       0x0000000C
#define LAYER_2_ASYNC_IENCLR_TYPE                                         UInt32
#define LAYER_2_ASYNC_IENCLR_RESERVED_MASK                                0xFFFF0000
#define    LAYER_2_ASYNC_IENCLR_IENCLR_SHIFT                              0
#define    LAYER_2_ASYNC_IENCLR_IENCLR_MASK                               0x0000FFFF

#define LAYER_2_ASYNC_IRESET_OFFSET                                       0x00000014
#define LAYER_2_ASYNC_IRESET_TYPE                                         UInt32
#define LAYER_2_ASYNC_IRESET_RESERVED_MASK                                0xFFFF0000
#define    LAYER_2_ASYNC_IRESET_IRESET_SHIFT                              0
#define    LAYER_2_ASYNC_IRESET_IRESET_MASK                               0x0000FFFF

#define LAYER_2_ASYNC_L2TOL1SOFTINT_OFFSET                                0x00000018
#define LAYER_2_ASYNC_L2TOL1SOFTINT_TYPE                                  UInt32
#define LAYER_2_ASYNC_L2TOL1SOFTINT_RESERVED_MASK                         0xFFFFFFE0
#define    LAYER_2_ASYNC_L2TOL1SOFTINT_L2TOL1SOFTINT_SHIFT                0
#define    LAYER_2_ASYNC_L2TOL1SOFTINT_L2TOL1SOFTINT_MASK                 0x0000001F

#define LAYER_2_ASYNC_POLARITY_OFFSET                                     0x00000020
#define LAYER_2_ASYNC_POLARITY_TYPE                                       UInt32
#define LAYER_2_ASYNC_POLARITY_RESERVED_MASK                              0xFFFFFFF9
#define    LAYER_2_ASYNC_POLARITY_LEGACY_OSC_POWER_DOWN_SHIFT             2
#define    LAYER_2_ASYNC_POLARITY_LEGACY_OSC_POWER_DOWN_MASK              0x00000004
#define    LAYER_2_ASYNC_POLARITY_POLARITY_SHIFT                          1
#define    LAYER_2_ASYNC_POLARITY_POLARITY_MASK                           0x00000002

#define LAYER_2_ASYNC_STARTTIMER_OFFSET                                   0x00000024
#define LAYER_2_ASYNC_STARTTIMER_TYPE                                     UInt32
#define LAYER_2_ASYNC_STARTTIMER_RESERVED_MASK                            0xFFFFFFFC
#define    LAYER_2_ASYNC_STARTTIMER_END_OF_COUNT_SHIFT                    1
#define    LAYER_2_ASYNC_STARTTIMER_END_OF_COUNT_MASK                     0x00000002
#define    LAYER_2_ASYNC_STARTTIMER_STARTTIMER_SHIFT                      0
#define    LAYER_2_ASYNC_STARTTIMER_STARTTIMER_MASK                       0x00000001

#define LAYER_2_ASYNC_MAXTIMERCOUNT_OFFSET                                0x00000028
#define LAYER_2_ASYNC_MAXTIMERCOUNT_TYPE                                  UInt32
#define LAYER_2_ASYNC_MAXTIMERCOUNT_RESERVED_MASK                         0xFFFFC000
#define    LAYER_2_ASYNC_MAXTIMERCOUNT_MAXTIMERCOUNT_SHIFT                0
#define    LAYER_2_ASYNC_MAXTIMERCOUNT_MAXTIMERCOUNT_MASK                 0x00003FFF

#define LAYER_2_ASYNC_SFN_CNT_OFFSET                                      0x0000002C
#define LAYER_2_ASYNC_SFN_CNT_TYPE                                        UInt32
#define LAYER_2_ASYNC_SFN_CNT_RESERVED_MASK                               0xFFFFF000
#define    LAYER_2_ASYNC_SFN_CNT_SFN_CNT_SHIFT                            0
#define    LAYER_2_ASYNC_SFN_CNT_SFN_CNT_MASK                             0x00000FFF

#define LAYER_2_ASYNC_CAP_MTIMER_OFFSET                                   0x00000030
#define LAYER_2_ASYNC_CAP_MTIMER_TYPE                                     UInt32
#define LAYER_2_ASYNC_CAP_MTIMER_RESERVED_MASK                            0xFFFFFFFE
#define    LAYER_2_ASYNC_CAP_MTIMER_CAP_MTIMER_SHIFT                      0
#define    LAYER_2_ASYNC_CAP_MTIMER_CAP_MTIMER_MASK                       0x00000001

#define LAYER_2_ASYNC_SAMPLE_CNT_OFFSET                                   0x00000034
#define LAYER_2_ASYNC_SAMPLE_CNT_TYPE                                     UInt32
#define LAYER_2_ASYNC_SAMPLE_CNT_RESERVED_MASK                            0xFFFFFFFE
#define    LAYER_2_ASYNC_SAMPLE_CNT_SAMPLE_CNT_SHIFT                      0
#define    LAYER_2_ASYNC_SAMPLE_CNT_SAMPLE_CNT_MASK                       0x00000001

#define LAYER_2_ASYNC_CAP_FRAME_CNT_OFFSET                                0x00000038
#define LAYER_2_ASYNC_CAP_FRAME_CNT_TYPE                                  UInt32
#define LAYER_2_ASYNC_CAP_FRAME_CNT_RESERVED_MASK                         0xFFFF0000
#define    LAYER_2_ASYNC_CAP_FRAME_CNT_FRAME_CAP_SHIFT                    4
#define    LAYER_2_ASYNC_CAP_FRAME_CNT_FRAME_CAP_MASK                     0x0000FFF0
#define    LAYER_2_ASYNC_CAP_FRAME_CNT_SLOT_CAP_RH_E_SHIFT                0
#define    LAYER_2_ASYNC_CAP_FRAME_CNT_SLOT_CAP_RH_E_MASK                 0x0000000F

#define LAYER_2_ASYNC_CAP_SAMPLE_CNT_OFFSET                               0x0000003C
#define LAYER_2_ASYNC_CAP_SAMPLE_CNT_TYPE                                 UInt32
#define LAYER_2_ASYNC_CAP_SAMPLE_CNT_RESERVED_MASK                        0xFFFFC000
#define    LAYER_2_ASYNC_CAP_SAMPLE_CNT_CAP_SAMPLE_CNT_SHIFT              0
#define    LAYER_2_ASYNC_CAP_SAMPLE_CNT_CAP_SAMPLE_CNT_MASK               0x00003FFF

#define LAYER_2_ASYNC_GP_REGISTER1_OFFSET                                 0x00000040
#define LAYER_2_ASYNC_GP_REGISTER1_TYPE                                   UInt32
#define LAYER_2_ASYNC_GP_REGISTER1_RESERVED_MASK                          0xFFFF0000
#define    LAYER_2_ASYNC_GP_REGISTER1_GP_REGISTER1_SHIFT                  0
#define    LAYER_2_ASYNC_GP_REGISTER1_GP_REGISTER1_MASK                   0x0000FFFF

#define LAYER_2_ASYNC_GP_REGISTER2_OFFSET                                 0x00000044
#define LAYER_2_ASYNC_GP_REGISTER2_TYPE                                   UInt32
#define LAYER_2_ASYNC_GP_REGISTER2_RESERVED_MASK                          0xFFFF0000
#define    LAYER_2_ASYNC_GP_REGISTER2_GP_REGISTER2_SHIFT                  0
#define    LAYER_2_ASYNC_GP_REGISTER2_GP_REGISTER2_MASK                   0x0000FFFF

#define LAYER_2_ASYNC_AFCPDM_GSM_OFFSET                                   0x00000048
#define LAYER_2_ASYNC_AFCPDM_GSM_TYPE                                     UInt32
#define LAYER_2_ASYNC_AFCPDM_GSM_RESERVED_MASK                            0xFFFFF000
#define    LAYER_2_ASYNC_AFCPDM_GSM_AFCPDM_GSM_SHIFT                      0
#define    LAYER_2_ASYNC_AFCPDM_GSM_AFCPDM_GSM_MASK                       0x00000FFF

#define LAYER_2_ASYNC_AFCPDM_WCDMA_OFFSET                                 0x0000004C
#define LAYER_2_ASYNC_AFCPDM_WCDMA_TYPE                                   UInt32
#define LAYER_2_ASYNC_AFCPDM_WCDMA_RESERVED_MASK                          0xFFFFF000
#define    LAYER_2_ASYNC_AFCPDM_WCDMA_AFCPDM_WCDMA_SHIFT                  0
#define    LAYER_2_ASYNC_AFCPDM_WCDMA_AFCPDM_WCDMA_MASK                   0x00000FFF

#define LAYER_2_ASYNC_AFC_OEN_OFFSET                                      0x00000050
#define LAYER_2_ASYNC_AFC_OEN_TYPE                                        UInt32
#define LAYER_2_ASYNC_AFC_OEN_RESERVED_MASK                               0xFFFFFF06
#define    LAYER_2_ASYNC_AFC_OEN_AFCPDM_CNT_OEN_SHIFT                     4
#define    LAYER_2_ASYNC_AFC_OEN_AFCPDM_CNT_OEN_MASK                      0x000000F0
#define    LAYER_2_ASYNC_AFC_OEN_AFCPDM_SEL_SHIFT                         3
#define    LAYER_2_ASYNC_AFC_OEN_AFCPDM_SEL_MASK                          0x00000008
#define    LAYER_2_ASYNC_AFC_OEN_AFCPDM_OEN_SHIFT                         0
#define    LAYER_2_ASYNC_AFC_OEN_AFCPDM_OEN_MASK                          0x00000001

#define LAYER_2_ASYNC_REQ_L1_OFFSET                                       0x00000060
#define LAYER_2_ASYNC_REQ_L1_TYPE                                         UInt32
#define LAYER_2_ASYNC_REQ_L1_RESERVED_MASK                                0xFFFFFFFE
#define    LAYER_2_ASYNC_REQ_L1_REQ_L1_SHIFT                              0
#define    LAYER_2_ASYNC_REQ_L1_REQ_L1_MASK                               0x00000001

#define LAYER_2_ASYNC_REQ_L2_OFFSET                                       0x00000064
#define LAYER_2_ASYNC_REQ_L2_TYPE                                         UInt32
#define LAYER_2_ASYNC_REQ_L2_RESERVED_MASK                                0xFFFFFFFE
#define    LAYER_2_ASYNC_REQ_L2_REQ_L2_SHIFT                              0
#define    LAYER_2_ASYNC_REQ_L2_REQ_L2_MASK                               0x00000001

#define LAYER_2_ASYNC_GRANT_L1_L2_OFFSET                                  0x00000068
#define LAYER_2_ASYNC_GRANT_L1_L2_TYPE                                    UInt32
#define LAYER_2_ASYNC_GRANT_L1_L2_RESERVED_MASK                           0xFFFFFFF0
#define    LAYER_2_ASYNC_GRANT_L1_L2_OSC_POWER_DOWN_SHIFT                 3
#define    LAYER_2_ASYNC_GRANT_L1_L2_OSC_POWER_DOWN_MASK                  0x00000008
#define    LAYER_2_ASYNC_GRANT_L1_L2_SLEEP_MODE_SHIFT                     2
#define    LAYER_2_ASYNC_GRANT_L1_L2_SLEEP_MODE_MASK                      0x00000004
#define    LAYER_2_ASYNC_GRANT_L1_L2_GRANT_L1_L2_SHIFT                    0
#define    LAYER_2_ASYNC_GRANT_L1_L2_GRANT_L1_L2_MASK                     0x00000003

#define LAYER_2_ASYNC_CLOCKON_OFFSET                                      0x0000006C
#define LAYER_2_ASYNC_CLOCKON_TYPE                                        UInt32
#define LAYER_2_ASYNC_CLOCKON_RESERVED_MASK                               0xFFFFFFC0
#define    LAYER_2_ASYNC_CLOCKON_FORCE_MODEM_SLEEP_SHIFT                  5
#define    LAYER_2_ASYNC_CLOCKON_FORCE_MODEM_SLEEP_MASK                   0x00000020
#define    LAYER_2_ASYNC_CLOCKON_ENABLE_SLEEP_ON_ASYNC_SHIFT              4
#define    LAYER_2_ASYNC_CLOCKON_ENABLE_SLEEP_ON_ASYNC_MASK               0x00000010
#define    LAYER_2_ASYNC_CLOCKON_SLOWCLOCKON_SHIFT                        3
#define    LAYER_2_ASYNC_CLOCKON_SLOWCLOCKON_MASK                         0x00000008
#define    LAYER_2_ASYNC_CLOCKON_CLOCKSWITCHING_SHIFT                     2
#define    LAYER_2_ASYNC_CLOCKON_CLOCKSWITCHING_MASK                      0x00000004
#define    LAYER_2_ASYNC_CLOCKON_L2R_PLLFCLK_ON_SHIFT                     1
#define    LAYER_2_ASYNC_CLOCKON_L2R_PLLFCLK_ON_MASK                      0x00000002
#define    LAYER_2_ASYNC_CLOCKON_L2R_FCLK_ON_SHIFT                        0
#define    LAYER_2_ASYNC_CLOCKON_L2R_FCLK_ON_MASK                         0x00000001

#define LAYER_2_ASYNC_SOFTRESET_OFFSET                                    0x00000078
#define LAYER_2_ASYNC_SOFTRESET_TYPE                                      UInt32
#define LAYER_2_ASYNC_SOFTRESET_RESERVED_MASK                             0xFFFFFFFE
#define    LAYER_2_ASYNC_SOFTRESET_SOFTRESET_SHIFT                        0
#define    LAYER_2_ASYNC_SOFTRESET_SOFTRESET_MASK                         0x00000001

#endif /* __BRCM_RDB_LAYER_2_ASYNC_H__ */


