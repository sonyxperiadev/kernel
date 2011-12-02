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

#ifndef __BRCM_RDB_WTI_H__
#define __BRCM_RDB_WTI_H__

#define WTI_DOGCR_OFFSET                                                  0x00000000
#define WTI_DOGCR_TYPE                                                    UInt32
#define WTI_DOGCR_RESERVED_MASK                                           0x00000E00
#define    WTI_DOGCR_DELAY_EN_SHIFT                                       31
#define    WTI_DOGCR_DELAY_EN_MASK                                        0x80000000
#define    WTI_DOGCR_DELAY_LD_SHIFT                                       16
#define    WTI_DOGCR_DELAY_LD_MASK                                        0x7FFF0000
#define    WTI_DOGCR_EN_SHIFT                                             15
#define    WTI_DOGCR_EN_MASK                                              0x00008000
#define    WTI_DOGCR_SRSTEN_SHIFT                                         14
#define    WTI_DOGCR_SRSTEN_MASK                                          0x00004000
#define    WTI_DOGCR_INTEN_SHIFT                                          13
#define    WTI_DOGCR_INTEN_MASK                                           0x00002000
#define    WTI_DOGCR_DSLPCEN_SHIFT                                        12
#define    WTI_DOGCR_DSLPCEN_MASK                                         0x00001000
#define    WTI_DOGCR_CLKS_SHIFT                                           8
#define    WTI_DOGCR_CLKS_MASK                                            0x00000100
#define    WTI_DOGCR_LD_SHIFT                                             0
#define    WTI_DOGCR_LD_MASK                                              0x000000FF

#define WTI_DOGTMR_OFFSET                                                 0x00000004
#define WTI_DOGTMR_TYPE                                                   UInt32
#define WTI_DOGTMR_RESERVED_MASK                                          0xFFFFFF00
#define    WTI_DOGTMR_CNT_SHIFT                                           0
#define    WTI_DOGTMR_CNT_MASK                                            0x000000FF

#define WTI_RTC_OFFSET                                                    0x00000008
#define WTI_RTC_TYPE                                                      UInt32
#define WTI_RTC_RESERVED_MASK                                             0x00000000
#define    WTI_RTC_COUNT_SHIFT                                            0
#define    WTI_RTC_COUNT_MASK                                             0xFFFFFFFF

#endif /* __BRCM_RDB_WTI_H__ */


