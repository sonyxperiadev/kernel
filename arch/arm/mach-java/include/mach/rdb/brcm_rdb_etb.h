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

#ifndef __BRCM_RDB_ETB_H__
#define __BRCM_RDB_ETB_H__

#define ETB_RDP_OFFSET                                                    0x00000004
#define ETB_RDP_TYPE                                                      UInt32
#define ETB_RDP_RESERVED_MASK                                             0x00000000
#define    ETB_RDP_RDP_SHIFT                                              0
#define    ETB_RDP_RDP_MASK                                               0xFFFFFFFF

#define ETB_STS_OFFSET                                                    0x0000000C
#define ETB_STS_TYPE                                                      UInt32
#define ETB_STS_RESERVED_MASK                                             0xFFFFFFF0
#define    ETB_STS_FTEMPTY_SHIFT                                          3
#define    ETB_STS_FTEMPTY_MASK                                           0x00000008
#define    ETB_STS_ACQCOMP_SHIFT                                          2
#define    ETB_STS_ACQCOMP_MASK                                           0x00000004
#define    ETB_STS_TRIGGERED_SHIFT                                        1
#define    ETB_STS_TRIGGERED_MASK                                         0x00000002
#define    ETB_STS_FULL_SHIFT                                             0
#define    ETB_STS_FULL_MASK                                              0x00000001

#define ETB_RRD_OFFSET                                                    0x00000010
#define ETB_RRD_TYPE                                                      UInt32
#define ETB_RRD_RESERVED_MASK                                             0x00000000
#define    ETB_RRD_RRD_SHIFT                                              0
#define    ETB_RRD_RRD_MASK                                               0xFFFFFFFF

#define ETB_RRP_OFFSET                                                    0x00000014
#define ETB_RRP_TYPE                                                      UInt32
#define ETB_RRP_RESERVED_MASK                                             0xFFFC0000
#define    ETB_RRP_RRP_SHIFT                                              0
#define    ETB_RRP_RRP_MASK                                               0x0003FFFF

#define ETB_RWP_OFFSET                                                    0x00000018
#define ETB_RWP_TYPE                                                      UInt32
#define ETB_RWP_RESERVED_MASK                                             0xFFFC0000
#define    ETB_RWP_RWP_SHIFT                                              0
#define    ETB_RWP_RWP_MASK                                               0x0003FFFF

#define ETB_TRG_OFFSET                                                    0x0000001C
#define ETB_TRG_TYPE                                                      UInt32
#define ETB_TRG_RESERVED_MASK                                             0xFFFC0000
#define    ETB_TRG_TRG_SHIFT                                              0
#define    ETB_TRG_TRG_MASK                                               0x0003FFFF

#define ETB_CTL_OFFSET                                                    0x00000020
#define ETB_CTL_TYPE                                                      UInt32
#define ETB_CTL_RESERVED_MASK                                             0xFFFFFFFE
#define    ETB_CTL_TRACECAPTEN_SHIFT                                      0
#define    ETB_CTL_TRACECAPTEN_MASK                                       0x00000001

#define ETB_RWD_OFFSET                                                    0x00000024
#define ETB_RWD_TYPE                                                      UInt32
#define ETB_RWD_RESERVED_MASK                                             0x00000000
#define    ETB_RWD_RWD_SHIFT                                              0
#define    ETB_RWD_RWD_MASK                                               0xFFFFFFFF

#define ETB_FFSR_OFFSET                                                   0x00000300
#define ETB_FFSR_TYPE                                                     UInt32
#define ETB_FFSR_RESERVED_MASK                                            0xFFFFFFFC
#define    ETB_FFSR_FTSTOPPED_SHIFT                                       1
#define    ETB_FFSR_FTSTOPPED_MASK                                        0x00000002
#define    ETB_FFSR_FLINPROG_SHIFT                                        0
#define    ETB_FFSR_FLINPROG_MASK                                         0x00000001

#define ETB_FFCR_OFFSET                                                   0x00000304
#define ETB_FFCR_TYPE                                                     UInt32
#define ETB_FFCR_RESERVED_MASK                                            0xFFFFC88C
#define    ETB_FFCR_STOPTRIG_SHIFT                                        13
#define    ETB_FFCR_STOPTRIG_MASK                                         0x00002000
#define    ETB_FFCR_STOPFL_SHIFT                                          12
#define    ETB_FFCR_STOPFL_MASK                                           0x00001000
#define    ETB_FFCR_TRIGFL_SHIFT                                          10
#define    ETB_FFCR_TRIGFL_MASK                                           0x00000400
#define    ETB_FFCR_TRIGEVT_SHIFT                                         9
#define    ETB_FFCR_TRIGEVT_MASK                                          0x00000200
#define    ETB_FFCR_TRIGIN_SHIFT                                          8
#define    ETB_FFCR_TRIGIN_MASK                                           0x00000100
#define    ETB_FFCR_FONMAN_SHIFT                                          6
#define    ETB_FFCR_FONMAN_MASK                                           0x00000040
#define    ETB_FFCR_FONTRIG_SHIFT                                         5
#define    ETB_FFCR_FONTRIG_MASK                                          0x00000020
#define    ETB_FFCR_FONFLIN_SHIFT                                         4
#define    ETB_FFCR_FONFLIN_MASK                                          0x00000010
#define    ETB_FFCR_ENFCONT_SHIFT                                         1
#define    ETB_FFCR_ENFCONT_MASK                                          0x00000002
#define    ETB_FFCR_ENFTC_SHIFT                                           0
#define    ETB_FFCR_ENFTC_MASK                                            0x00000001

#define ETB_ITMISCOP0_OFFSET                                              0x00000EE0
#define ETB_ITMISCOP0_TYPE                                                UInt32
#define ETB_ITMISCOP0_RESERVED_MASK                                       0xFFFFFFFC
#define    ETB_ITMISCOP0_FULL_SHIFT                                       1
#define    ETB_ITMISCOP0_FULL_MASK                                        0x00000002
#define    ETB_ITMISCOP0_ACQCOMP_SHIFT                                    0
#define    ETB_ITMISCOP0_ACQCOMP_MASK                                     0x00000001

#define ETB_ITTRFLINACK_OFFSET                                            0x00000EE4
#define ETB_ITTRFLINACK_TYPE                                              UInt32
#define ETB_ITTRFLINACK_RESERVED_MASK                                     0xFFFFFFFC
#define    ETB_ITTRFLINACK_FLUSHINACK_SHIFT                               1
#define    ETB_ITTRFLINACK_FLUSHINACK_MASK                                0x00000002
#define    ETB_ITTRFLINACK_TRIGINACK_SHIFT                                0
#define    ETB_ITTRFLINACK_TRIGINACK_MASK                                 0x00000001

#define ETB_ITTRFLIN_OFFSET                                               0x00000EE8
#define ETB_ITTRFLIN_TYPE                                                 UInt32
#define ETB_ITTRFLIN_RESERVED_MASK                                        0xFFFFFFFC
#define    ETB_ITTRFLIN_FLUSHIN_SHIFT                                     1
#define    ETB_ITTRFLIN_FLUSHIN_MASK                                      0x00000002
#define    ETB_ITTRFLIN_TRIGIN_SHIFT                                      0
#define    ETB_ITTRFLIN_TRIGIN_MASK                                       0x00000001

#define ETB_ITATBDATA0_OFFSET                                             0x00000EEC
#define ETB_ITATBDATA0_TYPE                                               UInt32
#define ETB_ITATBDATA0_RESERVED_MASK                                      0xFFFFFFE0
#define    ETB_ITATBDATA0_ATDATA_31_SHIFT                                 4
#define    ETB_ITATBDATA0_ATDATA_31_MASK                                  0x00000010
#define    ETB_ITATBDATA0_ATDATA_23_SHIFT                                 3
#define    ETB_ITATBDATA0_ATDATA_23_MASK                                  0x00000008
#define    ETB_ITATBDATA0_ATDATA_15_SHIFT                                 2
#define    ETB_ITATBDATA0_ATDATA_15_MASK                                  0x00000004
#define    ETB_ITATBDATA0_ATDATA_7_SHIFT                                  1
#define    ETB_ITATBDATA0_ATDATA_7_MASK                                   0x00000002
#define    ETB_ITATBDATA0_ATDATA_0_SHIFT                                  0
#define    ETB_ITATBDATA0_ATDATA_0_MASK                                   0x00000001

#define ETB_ITATBCTR2_OFFSET                                              0x00000EF0
#define ETB_ITATBCTR2_TYPE                                                UInt32
#define ETB_ITATBCTR2_RESERVED_MASK                                       0xFFFFFFFC
#define    ETB_ITATBCTR2_AFVALIDS_SHIFT                                   1
#define    ETB_ITATBCTR2_AFVALIDS_MASK                                    0x00000002
#define    ETB_ITATBCTR2_ATREADYS_SHIFT                                   0
#define    ETB_ITATBCTR2_ATREADYS_MASK                                    0x00000001

#define ETB_ITATBCTR1_OFFSET                                              0x00000EF4
#define ETB_ITATBCTR1_TYPE                                                UInt32
#define ETB_ITATBCTR1_RESERVED_MASK                                       0xFFFFFF80
#define    ETB_ITATBCTR1_ITATIDS_SHIFT                                    0
#define    ETB_ITATBCTR1_ITATIDS_MASK                                     0x0000007F

#define ETB_ITATBCTR0_OFFSET                                              0x00000EF8
#define ETB_ITATBCTR0_TYPE                                                UInt32
#define ETB_ITATBCTR0_RESERVED_MASK                                       0xFFFFFCFC
#define    ETB_ITATBCTR0_ATBYTES_SHIFT                                    8
#define    ETB_ITATBCTR0_ATBYTES_MASK                                     0x00000300
#define    ETB_ITATBCTR0_AFREADY_SHIFT                                    1
#define    ETB_ITATBCTR0_AFREADY_MASK                                     0x00000002
#define    ETB_ITATBCTR0_ATVALID_SHIFT                                    0
#define    ETB_ITATBCTR0_ATVALID_MASK                                     0x00000001

#define ETB_ICTRL_OFFSET                                                  0x00000F00
#define ETB_ICTRL_TYPE                                                    UInt32
#define ETB_ICTRL_RESERVED_MASK                                           0xFFFFFFFE
#define    ETB_ICTRL_INTEG_EN_SHIFT                                       0
#define    ETB_ICTRL_INTEG_EN_MASK                                        0x00000001

#define ETB_CLAIMSET_OFFSET                                               0x00000FA0
#define ETB_CLAIMSET_TYPE                                                 UInt32
#define ETB_CLAIMSET_RESERVED_MASK                                        0xFFFFFFF0
#define    ETB_CLAIMSET_CLAIM_TAGS_SET_SHIFT                              0
#define    ETB_CLAIMSET_CLAIM_TAGS_SET_MASK                               0x0000000F

#define ETB_CLAIMCLR_OFFSET                                               0x00000FA4
#define ETB_CLAIMCLR_TYPE                                                 UInt32
#define ETB_CLAIMCLR_RESERVED_MASK                                        0xFFFFFFF0
#define    ETB_CLAIMCLR_CLAIM_TAGS_CLR_SHIFT                              0
#define    ETB_CLAIMCLR_CLAIM_TAGS_CLR_MASK                               0x0000000F

#define ETB_LOCKACCESS_OFFSET                                             0x00000FB0
#define ETB_LOCKACCESS_TYPE                                               UInt32
#define ETB_LOCKACCESS_RESERVED_MASK                                      0x00000000
#define    ETB_LOCKACCESS_LOCK_ACCESS_SHIFT                               0
#define    ETB_LOCKACCESS_LOCK_ACCESS_MASK                                0xFFFFFFFF

#define ETB_LOCKSTATUS_OFFSET                                             0x00000FB4
#define ETB_LOCKSTATUS_TYPE                                               UInt32
#define ETB_LOCKSTATUS_RESERVED_MASK                                      0xFFFFFFF8
#define    ETB_LOCKSTATUS_ACCESS_32BIT_SHIFT                              2
#define    ETB_LOCKSTATUS_ACCESS_32BIT_MASK                               0x00000004
#define    ETB_LOCKSTATUS_LOCKED_SHIFT                                    1
#define    ETB_LOCKSTATUS_LOCKED_MASK                                     0x00000002
#define    ETB_LOCKSTATUS_LOCK_IMPLEMENTED_SHIFT                          0
#define    ETB_LOCKSTATUS_LOCK_IMPLEMENTED_MASK                           0x00000001

#define ETB_AUTHSTATUS_OFFSET                                             0x00000FB8
#define ETB_AUTHSTATUS_TYPE                                               UInt32
#define ETB_AUTHSTATUS_RESERVED_MASK                                      0xFFFFFF00
#define    ETB_AUTHSTATUS_SEC_NON_DBG_SHIFT                               6
#define    ETB_AUTHSTATUS_SEC_NON_DBG_MASK                                0x000000C0
#define    ETB_AUTHSTATUS_SEC_INV_DBG_SHIFT                               4
#define    ETB_AUTHSTATUS_SEC_INV_DBG_MASK                                0x00000030
#define    ETB_AUTHSTATUS_OPEN_NON_DBG_SHIFT                              2
#define    ETB_AUTHSTATUS_OPEN_NON_DBG_MASK                               0x0000000C
#define    ETB_AUTHSTATUS_OPEN_INV_DBG_SHIFT                              0
#define    ETB_AUTHSTATUS_OPEN_INV_DBG_MASK                               0x00000003

#define ETB_DEVID_OFFSET                                                  0x00000FC8
#define ETB_DEVID_TYPE                                                    UInt32
#define ETB_DEVID_RESERVED_MASK                                           0x00000000
#define    ETB_DEVID_DEVID_SHIFT                                          0
#define    ETB_DEVID_DEVID_MASK                                           0xFFFFFFFF

#define ETB_DEVTYPE_OFFSET                                                0x00000FCC
#define ETB_DEVTYPE_TYPE                                                  UInt32
#define ETB_DEVTYPE_RESERVED_MASK                                         0xFFFFFF00
#define    ETB_DEVTYPE_SUB_TYPE_SHIFT                                     4
#define    ETB_DEVTYPE_SUB_TYPE_MASK                                      0x000000F0
#define    ETB_DEVTYPE_MAIN_CLASS_SHIFT                                   0
#define    ETB_DEVTYPE_MAIN_CLASS_MASK                                    0x0000000F

#define ETB_PERID4_OFFSET                                                 0x00000FD0
#define ETB_PERID4_TYPE                                                   UInt32
#define ETB_PERID4_RESERVED_MASK                                          0xFFFFFF00
#define    ETB_PERID4_PID4_SHIFT                                          0
#define    ETB_PERID4_PID4_MASK                                           0x000000FF

#define ETB_PERID0_OFFSET                                                 0x00000FE0
#define ETB_PERID0_TYPE                                                   UInt32
#define ETB_PERID0_RESERVED_MASK                                          0xFFFFFF00
#define    ETB_PERID0_PID0_SHIFT                                          0
#define    ETB_PERID0_PID0_MASK                                           0x000000FF

#define ETB_PERID1_OFFSET                                                 0x00000FE4
#define ETB_PERID1_TYPE                                                   UInt32
#define ETB_PERID1_RESERVED_MASK                                          0xFFFFFF00
#define    ETB_PERID1_PID1_SHIFT                                          0
#define    ETB_PERID1_PID1_MASK                                           0x000000FF

#define ETB_PERID2_OFFSET                                                 0x00000FE8
#define ETB_PERID2_TYPE                                                   UInt32
#define ETB_PERID2_RESERVED_MASK                                          0xFFFFFF00
#define    ETB_PERID2_PID2_SHIFT                                          0
#define    ETB_PERID2_PID2_MASK                                           0x000000FF

#define ETB_PERID3_OFFSET                                                 0x00000FEC
#define ETB_PERID3_TYPE                                                   UInt32
#define ETB_PERID3_RESERVED_MASK                                          0xFFFFFF00
#define    ETB_PERID3_PID3_SHIFT                                          0
#define    ETB_PERID3_PID3_MASK                                           0x000000FF

#define ETB_COMPID0_OFFSET                                                0x00000FF0
#define ETB_COMPID0_TYPE                                                  UInt32
#define ETB_COMPID0_RESERVED_MASK                                         0xFFFFFF00
#define    ETB_COMPID0_CID0_SHIFT                                         0
#define    ETB_COMPID0_CID0_MASK                                          0x000000FF

#define ETB_COMPID1_OFFSET                                                0x00000FF4
#define ETB_COMPID1_TYPE                                                  UInt32
#define ETB_COMPID1_RESERVED_MASK                                         0xFFFFFF00
#define    ETB_COMPID1_CID1_SHIFT                                         0
#define    ETB_COMPID1_CID1_MASK                                          0x000000FF

#define ETB_COMPID2_OFFSET                                                0x00000FF8
#define ETB_COMPID2_TYPE                                                  UInt32
#define ETB_COMPID2_RESERVED_MASK                                         0xFFFFFF00
#define    ETB_COMPID2_CID2_SHIFT                                         0
#define    ETB_COMPID2_CID2_MASK                                          0x000000FF

#define ETB_COMPID3_OFFSET                                                0x00000FFC
#define ETB_COMPID3_TYPE                                                  UInt32
#define ETB_COMPID3_RESERVED_MASK                                         0xFFFFFF00
#define    ETB_COMPID3_CID3_SHIFT                                         0
#define    ETB_COMPID3_CID3_MASK                                          0x000000FF

#endif /* __BRCM_RDB_ETB_H__ */


