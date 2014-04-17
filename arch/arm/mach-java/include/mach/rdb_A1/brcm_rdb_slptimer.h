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

#ifndef __BRCM_RDB_SLPTIMER_H__
#define __BRCM_RDB_SLPTIMER_H__

#define SLPTIMER_SMTDCLR_OFFSET                                           0x00000000
#define SLPTIMER_SMTDCLR_TYPE                                             UInt32
#define SLPTIMER_SMTDCLR_RESERVED_MASK                                    0xFFFFE000
#define    SLPTIMER_SMTDCLR_VAL_SHIFT                                     0
#define    SLPTIMER_SMTDCLR_VAL_MASK                                      0x00001FFF

#define SLPTIMER_SMTDCR_OFFSET                                            0x00000004
#define SLPTIMER_SMTDCR_TYPE                                              UInt32
#define SLPTIMER_SMTDCR_RESERVED_MASK                                     0xFFFFE000
#define    SLPTIMER_SMTDCR_VAL_SHIFT                                      0
#define    SLPTIMER_SMTDCR_VAL_MASK                                       0x00001FFF

#define SLPTIMER_SMTCR_OFFSET                                             0x00000008
#define SLPTIMER_SMTCR_TYPE                                               UInt32
#define SLPTIMER_SMTCR_RESERVED_MASK                                      0xFFFFFFF8
#define    SLPTIMER_SMTCR_APB_READ_SEL_SHIFT                              2
#define    SLPTIMER_SMTCR_APB_READ_SEL_MASK                               0x00000004
#define    SLPTIMER_SMTCR_CLKS_SHIFT                                      1
#define    SLPTIMER_SMTCR_CLKS_MASK                                       0x00000002
#define    SLPTIMER_SMTCR_TMREN_SHIFT                                     0
#define    SLPTIMER_SMTCR_TMREN_MASK                                      0x00000001

#define SLPTIMER_SMTCMPR_OFFSET                                           0x0000000C
#define SLPTIMER_SMTCMPR_TYPE                                             UInt32
#define SLPTIMER_SMTCMPR_RESERVED_MASK                                    0x00000000
#define    SLPTIMER_SMTCMPR_VAL_SHIFT                                     0
#define    SLPTIMER_SMTCMPR_VAL_MASK                                      0xFFFFFFFF

#define SLPTIMER_SMTSTR_OFFSET                                            0x00000010
#define SLPTIMER_SMTSTR_TYPE                                              UInt32
#define SLPTIMER_SMTSTR_RESERVED_MASK                                     0x00000000
#define    SLPTIMER_SMTSTR_VAL_SHIFT                                      0
#define    SLPTIMER_SMTSTR_VAL_MASK                                       0xFFFFFFFF

#endif /* __BRCM_RDB_SLPTIMER_H__ */


