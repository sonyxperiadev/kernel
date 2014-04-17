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

#ifndef __BRCM_RDB_RF_INTERFACE_BLOCK1_TOP_H__
#define __BRCM_RDB_RF_INTERFACE_BLOCK1_TOP_H__

#define RF_INTERFACE_BLOCK1_TOP_RXADCOFFSETI_OFFSET                       0x00000000
#define RF_INTERFACE_BLOCK1_TOP_RXADCOFFSETI_TYPE                         UInt32
#define RF_INTERFACE_BLOCK1_TOP_RXADCOFFSETI_RESERVED_MASK                0xC0000800
#define    RF_INTERFACE_BLOCK1_TOP_RXADCOFFSETI_DCOFFSETAVGI_SHIFT        12
#define    RF_INTERFACE_BLOCK1_TOP_RXADCOFFSETI_DCOFFSETAVGI_MASK         0x3FFFF000
#define    RF_INTERFACE_BLOCK1_TOP_RXADCOFFSETI_DCOFFSETESTI_SHIFT        0
#define    RF_INTERFACE_BLOCK1_TOP_RXADCOFFSETI_DCOFFSETESTI_MASK         0x000007FF

#define RF_INTERFACE_BLOCK1_TOP_RXADCOFFSETQ_OFFSET                       0x00000004
#define RF_INTERFACE_BLOCK1_TOP_RXADCOFFSETQ_TYPE                         UInt32
#define RF_INTERFACE_BLOCK1_TOP_RXADCOFFSETQ_RESERVED_MASK                0xC0000800
#define    RF_INTERFACE_BLOCK1_TOP_RXADCOFFSETQ_DCOFFSETAVGQ_SHIFT        12
#define    RF_INTERFACE_BLOCK1_TOP_RXADCOFFSETQ_DCOFFSETAVGQ_MASK         0x3FFFF000
#define    RF_INTERFACE_BLOCK1_TOP_RXADCOFFSETQ_DCOFFSETESTQ_SHIFT        0
#define    RF_INTERFACE_BLOCK1_TOP_RXADCOFFSETQ_DCOFFSETESTQ_MASK         0x000007FF

#define RF_INTERFACE_BLOCK1_TOP_RXADCMINCNTI_OFFSET                       0x00000008
#define RF_INTERFACE_BLOCK1_TOP_RXADCMINCNTI_TYPE                         UInt32
#define RF_INTERFACE_BLOCK1_TOP_RXADCMINCNTI_RESERVED_MASK                0xFFFE0000
#define    RF_INTERFACE_BLOCK1_TOP_RXADCMINCNTI_RXADCMINCNTI_SHIFT        0
#define    RF_INTERFACE_BLOCK1_TOP_RXADCMINCNTI_RXADCMINCNTI_MASK         0x0001FFFF

#define RF_INTERFACE_BLOCK1_TOP_RXADCMAXCNTI_OFFSET                       0x0000000C
#define RF_INTERFACE_BLOCK1_TOP_RXADCMAXCNTI_TYPE                         UInt32
#define RF_INTERFACE_BLOCK1_TOP_RXADCMAXCNTI_RESERVED_MASK                0xFFFE0000
#define    RF_INTERFACE_BLOCK1_TOP_RXADCMAXCNTI_RXADCMAXCNTI_SHIFT        0
#define    RF_INTERFACE_BLOCK1_TOP_RXADCMAXCNTI_RXADCMAXCNTI_MASK         0x0001FFFF

#define RF_INTERFACE_BLOCK1_TOP_RXADCMINCNTQ_OFFSET                       0x00000010
#define RF_INTERFACE_BLOCK1_TOP_RXADCMINCNTQ_TYPE                         UInt32
#define RF_INTERFACE_BLOCK1_TOP_RXADCMINCNTQ_RESERVED_MASK                0xFFFE0000
#define    RF_INTERFACE_BLOCK1_TOP_RXADCMINCNTQ_RXADCMINCNTQ_SHIFT        0
#define    RF_INTERFACE_BLOCK1_TOP_RXADCMINCNTQ_RXADCMINCNTQ_MASK         0x0001FFFF

#define RF_INTERFACE_BLOCK1_TOP_RXADCMAXCNTQ_OFFSET                       0x00000014
#define RF_INTERFACE_BLOCK1_TOP_RXADCMAXCNTQ_TYPE                         UInt32
#define RF_INTERFACE_BLOCK1_TOP_RXADCMAXCNTQ_RESERVED_MASK                0xFFFE0000
#define    RF_INTERFACE_BLOCK1_TOP_RXADCMAXCNTQ_RXADCMAXCNTQ_SHIFT        0
#define    RF_INTERFACE_BLOCK1_TOP_RXADCMAXCNTQ_RXADCMAXCNTQ_MASK         0x0001FFFF

#define RF_INTERFACE_BLOCK1_TOP_RSSIACC0DATA_OFFSET                       0x00000018
#define RF_INTERFACE_BLOCK1_TOP_RSSIACC0DATA_TYPE                         UInt32
#define RF_INTERFACE_BLOCK1_TOP_RSSIACC0DATA_RESERVED_MASK                0xFFF80000
#define    RF_INTERFACE_BLOCK1_TOP_RSSIACC0DATA_RSSIACC0DATA_SHIFT        0
#define    RF_INTERFACE_BLOCK1_TOP_RSSIACC0DATA_RSSIACC0DATA_MASK         0x0007FFFF

#define RF_INTERFACE_BLOCK1_TOP_RSSIACC1DATA_OFFSET                       0x0000001C
#define RF_INTERFACE_BLOCK1_TOP_RSSIACC1DATA_TYPE                         UInt32
#define RF_INTERFACE_BLOCK1_TOP_RSSIACC1DATA_RESERVED_MASK                0xFFF80000
#define    RF_INTERFACE_BLOCK1_TOP_RSSIACC1DATA_RSSIACC1DATA_SHIFT        0
#define    RF_INTERFACE_BLOCK1_TOP_RSSIACC1DATA_RSSIACC1DATA_MASK         0x0007FFFF

#define RF_INTERFACE_BLOCK1_TOP_RSSIACC2DATA_OFFSET                       0x00000020
#define RF_INTERFACE_BLOCK1_TOP_RSSIACC2DATA_TYPE                         UInt32
#define RF_INTERFACE_BLOCK1_TOP_RSSIACC2DATA_RESERVED_MASK                0xFFF80000
#define    RF_INTERFACE_BLOCK1_TOP_RSSIACC2DATA_RSSIACC2DATA_SHIFT        0
#define    RF_INTERFACE_BLOCK1_TOP_RSSIACC2DATA_RSSIACC2DATA_MASK         0x0007FFFF

#define RF_INTERFACE_BLOCK1_TOP_RSSIACC3DATA_OFFSET                       0x00000024
#define RF_INTERFACE_BLOCK1_TOP_RSSIACC3DATA_TYPE                         UInt32
#define RF_INTERFACE_BLOCK1_TOP_RSSIACC3DATA_RESERVED_MASK                0xFFF80000
#define    RF_INTERFACE_BLOCK1_TOP_RSSIACC3DATA_RSSIACC3DATA_SHIFT        0
#define    RF_INTERFACE_BLOCK1_TOP_RSSIACC3DATA_RSSIACC3DATA_MASK         0x0007FFFF

#define RF_INTERFACE_BLOCK1_TOP_RXADCCFG_OFFSET                           0x00000028
#define RF_INTERFACE_BLOCK1_TOP_RXADCCFG_TYPE                             UInt32
#define RF_INTERFACE_BLOCK1_TOP_RXADCCFG_RESERVED_MASK                    0xFFFEAEEC
#define    RF_INTERFACE_BLOCK1_TOP_RXADCCFG_DEBUGDIRECT_SHIFT             16
#define    RF_INTERFACE_BLOCK1_TOP_RXADCCFG_DEBUGDIRECT_MASK              0x00010000
#define    RF_INTERFACE_BLOCK1_TOP_RXADCCFG_RXDDRSELECT_SHIFT             14
#define    RF_INTERFACE_BLOCK1_TOP_RXADCCFG_RXDDRSELECT_MASK              0x00004000
#define    RF_INTERFACE_BLOCK1_TOP_RXADCCFG_CX2CX4ON_SHIFT                12
#define    RF_INTERFACE_BLOCK1_TOP_RXADCCFG_CX2CX4ON_MASK                 0x00001000
#define    RF_INTERFACE_BLOCK1_TOP_RXADCCFG_RXADCFORMATSEL_SHIFT          8
#define    RF_INTERFACE_BLOCK1_TOP_RXADCCFG_RXADCFORMATSEL_MASK           0x00000100
#define    RF_INTERFACE_BLOCK1_TOP_RXADCCFG_RXADCSWAPEN_SHIFT             4
#define    RF_INTERFACE_BLOCK1_TOP_RXADCCFG_RXADCSWAPEN_MASK              0x00000010
#define    RF_INTERFACE_BLOCK1_TOP_RXADCCFG_RXCMFVPGAIN_SHIFT             0
#define    RF_INTERFACE_BLOCK1_TOP_RXADCCFG_RXCMFVPGAIN_MASK              0x00000003

#define RF_INTERFACE_BLOCK1_TOP_RXAGCCFG_OFFSET                           0x0000002C
#define RF_INTERFACE_BLOCK1_TOP_RXAGCCFG_TYPE                             UInt32
#define RF_INTERFACE_BLOCK1_TOP_RXAGCCFG_RESERVED_MASK                    0xFF0000EE
#define    RF_INTERFACE_BLOCK1_TOP_RXAGCCFG_RXAGCMAXTHRES_SHIFT           16
#define    RF_INTERFACE_BLOCK1_TOP_RXAGCCFG_RXAGCMAXTHRES_MASK            0x00FF0000
#define    RF_INTERFACE_BLOCK1_TOP_RXAGCCFG_RXAGCMINTHRES_SHIFT           8
#define    RF_INTERFACE_BLOCK1_TOP_RXAGCCFG_RXAGCMINTHRES_MASK            0x0000FF00
#define    RF_INTERFACE_BLOCK1_TOP_RXAGCCFG_RXAGCDATASEL_SHIFT            4
#define    RF_INTERFACE_BLOCK1_TOP_RXAGCCFG_RXAGCDATASEL_MASK             0x00000010
#define    RF_INTERFACE_BLOCK1_TOP_RXAGCCFG_RXAGCEN_SHIFT                 0
#define    RF_INTERFACE_BLOCK1_TOP_RXAGCCFG_RXAGCEN_MASK                  0x00000001

#define RF_INTERFACE_BLOCK1_TOP_RSSICFG_OFFSET                            0x00000030
#define RF_INTERFACE_BLOCK1_TOP_RSSICFG_TYPE                              UInt32
#define RF_INTERFACE_BLOCK1_TOP_RSSICFG_RESERVED_MASK                     0xFFF0EEEE
#define    RF_INTERFACE_BLOCK1_TOP_RSSICFG_RSSIDATAPOLL_SHIFT             16
#define    RF_INTERFACE_BLOCK1_TOP_RSSICFG_RSSIDATAPOLL_MASK              0x000F0000
#define    RF_INTERFACE_BLOCK1_TOP_RSSICFG_RSSIACCLENG_SHIFT              12
#define    RF_INTERFACE_BLOCK1_TOP_RSSICFG_RSSIACCLENG_MASK               0x00001000
#define    RF_INTERFACE_BLOCK1_TOP_RSSICFG_RSSIDECNUM_SHIFT               8
#define    RF_INTERFACE_BLOCK1_TOP_RSSICFG_RSSIDECNUM_MASK                0x00000100
#define    RF_INTERFACE_BLOCK1_TOP_RSSICFG_RSSIDATASEL_SHIFT              4
#define    RF_INTERFACE_BLOCK1_TOP_RSSICFG_RSSIDATASEL_MASK               0x00000010
#define    RF_INTERFACE_BLOCK1_TOP_RSSICFG_RSSIEN_SHIFT                   0
#define    RF_INTERFACE_BLOCK1_TOP_RSSICFG_RSSIEN_MASK                    0x00000001

#define RF_INTERFACE_BLOCK1_TOP_DCCFG1_OFFSET                             0x00000034
#define RF_INTERFACE_BLOCK1_TOP_DCCFG1_TYPE                               UInt32
#define RF_INTERFACE_BLOCK1_TOP_DCCFG1_RESERVED_MASK                      0xFFFFF0EE
#define    RF_INTERFACE_BLOCK1_TOP_DCCFG1_DCOFFSETAVGBW_SHIFT             8
#define    RF_INTERFACE_BLOCK1_TOP_DCCFG1_DCOFFSETAVGBW_MASK              0x00000F00
#define    RF_INTERFACE_BLOCK1_TOP_DCCFG1_DCOFFSETMODE_SHIFT              4
#define    RF_INTERFACE_BLOCK1_TOP_DCCFG1_DCOFFSETMODE_MASK               0x00000010
#define    RF_INTERFACE_BLOCK1_TOP_DCCFG1_DCOFFSETEN_SHIFT                0
#define    RF_INTERFACE_BLOCK1_TOP_DCCFG1_DCOFFSETEN_MASK                 0x00000001

#define RF_INTERFACE_BLOCK1_TOP_DCCFG2_OFFSET                             0x00000038
#define RF_INTERFACE_BLOCK1_TOP_DCCFG2_TYPE                               UInt32
#define RF_INTERFACE_BLOCK1_TOP_DCCFG2_RESERVED_MASK                      0xFFFFFF0E
#define    RF_INTERFACE_BLOCK1_TOP_DCCFG2_DCOFFSETGAIN_SHIFT              4
#define    RF_INTERFACE_BLOCK1_TOP_DCCFG2_DCOFFSETGAIN_MASK               0x000000F0
#define    RF_INTERFACE_BLOCK1_TOP_DCCFG2_DCOFFSETLOOP_SHIFT              0
#define    RF_INTERFACE_BLOCK1_TOP_DCCFG2_DCOFFSETLOOP_MASK               0x00000001

#define RF_INTERFACE_BLOCK1_TOP_DCCFG3_OFFSET                             0x0000003C
#define RF_INTERFACE_BLOCK1_TOP_DCCFG3_TYPE                               UInt32
#define RF_INTERFACE_BLOCK1_TOP_DCCFG3_RESERVED_MASK                      0xFF800800
#define    RF_INTERFACE_BLOCK1_TOP_DCCFG3_DCOFFSETINITQ_SHIFT             12
#define    RF_INTERFACE_BLOCK1_TOP_DCCFG3_DCOFFSETINITQ_MASK              0x007FF000
#define    RF_INTERFACE_BLOCK1_TOP_DCCFG3_DCOFFSETINITI_SHIFT             0
#define    RF_INTERFACE_BLOCK1_TOP_DCCFG3_DCOFFSETINITI_MASK              0x000007FF

#define RF_INTERFACE_BLOCK1_TOP_GSMD_OFFSET                               0x00000040
#define RF_INTERFACE_BLOCK1_TOP_GSMD_TYPE                                 UInt32
#define RF_INTERFACE_BLOCK1_TOP_GSMD_RESERVED_MASK                        0x000088EE
#define    RF_INTERFACE_BLOCK1_TOP_GSMD_GSMDPOWER_SHIFT                   16
#define    RF_INTERFACE_BLOCK1_TOP_GSMD_GSMDPOWER_MASK                    0xFFFF0000
#define    RF_INTERFACE_BLOCK1_TOP_GSMD_GSMDIIR2BW_SHIFT                  12
#define    RF_INTERFACE_BLOCK1_TOP_GSMD_GSMDIIR2BW_MASK                   0x00007000
#define    RF_INTERFACE_BLOCK1_TOP_GSMD_GSMDIIR1BW_SHIFT                  8
#define    RF_INTERFACE_BLOCK1_TOP_GSMD_GSMDIIR1BW_MASK                   0x00000700
#define    RF_INTERFACE_BLOCK1_TOP_GSMD_GSMDDATASEL_SHIFT                 4
#define    RF_INTERFACE_BLOCK1_TOP_GSMD_GSMDDATASEL_MASK                  0x00000010
#define    RF_INTERFACE_BLOCK1_TOP_GSMD_GSMDEN_SHIFT                      0
#define    RF_INTERFACE_BLOCK1_TOP_GSMD_GSMDEN_MASK                       0x00000001

#define RF_INTERFACE_BLOCK1_TOP_SIGDELTA_OFFSET                           0x00000044
#define RF_INTERFACE_BLOCK1_TOP_SIGDELTA_TYPE                             UInt32
#define RF_INTERFACE_BLOCK1_TOP_SIGDELTA_RESERVED_MASK                    0xFFFFE8EE
#define    RF_INTERFACE_BLOCK1_TOP_SIGDELTA_CMF_BYPASS_TO_FNG_SHIFT       12
#define    RF_INTERFACE_BLOCK1_TOP_SIGDELTA_CMF_BYPASS_TO_FNG_MASK        0x00001000
#define    RF_INTERFACE_BLOCK1_TOP_SIGDELTA_SIGDELTADECPOS_SHIFT          8
#define    RF_INTERFACE_BLOCK1_TOP_SIGDELTA_SIGDELTADECPOS_MASK           0x00000700
#define    RF_INTERFACE_BLOCK1_TOP_SIGDELTA_SELDEBUGPORTIQ_SHIFT          4
#define    RF_INTERFACE_BLOCK1_TOP_SIGDELTA_SELDEBUGPORTIQ_MASK           0x00000010
#define    RF_INTERFACE_BLOCK1_TOP_SIGDELTA_SIGDELTACLKEN_SHIFT           0
#define    RF_INTERFACE_BLOCK1_TOP_SIGDELTA_SIGDELTACLKEN_MASK            0x00000001

#endif /* __BRCM_RDB_RF_INTERFACE_BLOCK1_TOP_H__ */


