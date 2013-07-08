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

#ifndef __BRCM_RDB_AXITP_ONE_H__
#define __BRCM_RDB_AXITP_ONE_H__

#define AXITP_ONE_ATM_CONFIG_OFFSET                                       0x00000000
#define AXITP_ONE_ATM_CONFIG_TYPE                                         UInt32
#define AXITP_ONE_ATM_CONFIG_RESERVED_MASK                                0x00000000
#define    AXITP_ONE_ATM_CONFIG_BUSY_ID_SHIFT                             19
#define    AXITP_ONE_ATM_CONFIG_BUSY_ID_MASK                              0xFFF80000
#define    AXITP_ONE_ATM_CONFIG_OUTS_THRESH_SHIFT                         16
#define    AXITP_ONE_ATM_CONFIG_OUTS_THRESH_MASK                          0x00070000
#define    AXITP_ONE_ATM_CONFIG_OUTS_FILTERED_SHIFT                       15
#define    AXITP_ONE_ATM_CONFIG_OUTS_FILTERED_MASK                        0x00008000
#define    AXITP_ONE_ATM_CONFIG_OUTS_EN_SHIFT                             14
#define    AXITP_ONE_ATM_CONFIG_OUTS_EN_MASK                              0x00004000
#define    AXITP_ONE_ATM_CONFIG_ALL_CYCLES_EN_SHIFT                       13
#define    AXITP_ONE_ATM_CONFIG_ALL_CYCLES_EN_MASK                        0x00002000
#define    AXITP_ONE_ATM_CONFIG_RDLAT_MODE_SHIFT                          12
#define    AXITP_ONE_ATM_CONFIG_RDLAT_MODE_MASK                           0x00001000
#define    AXITP_ONE_ATM_CONFIG_LATENCY_FILTERED_SHIFT                    11
#define    AXITP_ONE_ATM_CONFIG_LATENCY_FILTERED_MASK                     0x00000800
#define    AXITP_ONE_ATM_CONFIG_LATENCY_EN_SHIFT                          10
#define    AXITP_ONE_ATM_CONFIG_LATENCY_EN_MASK                           0x00000400
#define    AXITP_ONE_ATM_CONFIG_BUSY_FILTERED_SHIFT                       9
#define    AXITP_ONE_ATM_CONFIG_BUSY_FILTERED_MASK                        0x00000200
#define    AXITP_ONE_ATM_CONFIG_BUSY_EN_SHIFT                             8
#define    AXITP_ONE_ATM_CONFIG_BUSY_EN_MASK                              0x00000100
#define    AXITP_ONE_ATM_CONFIG_BEATS_FILTERED_SHIFT                      7
#define    AXITP_ONE_ATM_CONFIG_BEATS_FILTERED_MASK                       0x00000080
#define    AXITP_ONE_ATM_CONFIG_BEATS_EN_SHIFT                            6
#define    AXITP_ONE_ATM_CONFIG_BEATS_EN_MASK                             0x00000040
#define    AXITP_ONE_ATM_CONFIG_CMDS_FILTERED_SHIFT                       5
#define    AXITP_ONE_ATM_CONFIG_CMDS_FILTERED_MASK                        0x00000020
#define    AXITP_ONE_ATM_CONFIG_CMDS_EN_SHIFT                             4
#define    AXITP_ONE_ATM_CONFIG_CMDS_EN_MASK                              0x00000010
#define    AXITP_ONE_ATM_CONFIG_TRACE_EN_SHIFT                            3
#define    AXITP_ONE_ATM_CONFIG_TRACE_EN_MASK                             0x00000008
#define    AXITP_ONE_ATM_CONFIG_FLUSH_SHIFT                               2
#define    AXITP_ONE_ATM_CONFIG_FLUSH_MASK                                0x00000004
#define    AXITP_ONE_ATM_CONFIG_SAT_EN_SHIFT                              1
#define    AXITP_ONE_ATM_CONFIG_SAT_EN_MASK                               0x00000002
#define    AXITP_ONE_ATM_CONFIG_CTRL_SRC_SHIFT                            0
#define    AXITP_ONE_ATM_CONFIG_CTRL_SRC_MASK                             0x00000001

#define AXITP_ONE_ATM_STATUS_OFFSET                                       0x00000004
#define AXITP_ONE_ATM_STATUS_TYPE                                         UInt32
#define AXITP_ONE_ATM_STATUS_RESERVED_MASK                                0xFFFFC0F0
#define    AXITP_ONE_ATM_STATUS_STATE_OUTS_EN_SHIFT                       13
#define    AXITP_ONE_ATM_STATUS_STATE_OUTS_EN_MASK                        0x00002000
#define    AXITP_ONE_ATM_STATUS_STATE_LATENCY_EN_SHIFT                    12
#define    AXITP_ONE_ATM_STATUS_STATE_LATENCY_EN_MASK                     0x00001000
#define    AXITP_ONE_ATM_STATUS_STATE_BEATS_EN_SHIFT                      11
#define    AXITP_ONE_ATM_STATUS_STATE_BEATS_EN_MASK                       0x00000800
#define    AXITP_ONE_ATM_STATUS_STATE_BUSY_EN_SHIFT                       10
#define    AXITP_ONE_ATM_STATUS_STATE_BUSY_EN_MASK                        0x00000400
#define    AXITP_ONE_ATM_STATUS_STATE_CMDS_EN_SHIFT                       9
#define    AXITP_ONE_ATM_STATUS_STATE_CMDS_EN_MASK                        0x00000200
#define    AXITP_ONE_ATM_STATUS_SAT_STOPPED_SHIFT                         8
#define    AXITP_ONE_ATM_STATUS_SAT_STOPPED_MASK                          0x00000100
#define    AXITP_ONE_ATM_STATUS_WFMT_EMPTY_SHIFT                          3
#define    AXITP_ONE_ATM_STATUS_WFMT_EMPTY_MASK                           0x00000008
#define    AXITP_ONE_ATM_STATUS_RFMT_EMPTY_SHIFT                          2
#define    AXITP_ONE_ATM_STATUS_RFMT_EMPTY_MASK                           0x00000004
#define    AXITP_ONE_ATM_STATUS_WFIFO_EMPTY_SHIFT                         1
#define    AXITP_ONE_ATM_STATUS_WFIFO_EMPTY_MASK                          0x00000002
#define    AXITP_ONE_ATM_STATUS_RFIFO_EMPTY_SHIFT                         0
#define    AXITP_ONE_ATM_STATUS_RFIFO_EMPTY_MASK                          0x00000001

#define AXITP_ONE_ATM_OUTIDS_OFFSET                                       0x00000008
#define AXITP_ONE_ATM_OUTIDS_TYPE                                         UInt32
#define AXITP_ONE_ATM_OUTIDS_RESERVED_MASK                                0xFFFF8080
#define    AXITP_ONE_ATM_OUTIDS_WATB_ID_SHIFT                             8
#define    AXITP_ONE_ATM_OUTIDS_WATB_ID_MASK                              0x00007F00
#define    AXITP_ONE_ATM_OUTIDS_RATB_ID_SHIFT                             0
#define    AXITP_ONE_ATM_OUTIDS_RATB_ID_MASK                              0x0000007F

#define AXITP_ONE_ATM_CMD_OFFSET                                          0x0000000C
#define AXITP_ONE_ATM_CMD_TYPE                                            UInt32
#define AXITP_ONE_ATM_CMD_RESERVED_MASK                                   0xFFFFFFFC
#define    AXITP_ONE_ATM_CMD_LOCAL_CMD_SHIFT                              0
#define    AXITP_ONE_ATM_CMD_LOCAL_CMD_MASK                               0x00000003

#define AXITP_ONE_ATM_WRCMDS_OFFSET                                       0x00000010
#define AXITP_ONE_ATM_WRCMDS_TYPE                                         UInt32
#define AXITP_ONE_ATM_WRCMDS_RESERVED_MASK                                0x00000000
#define    AXITP_ONE_ATM_WRCMDS_WRCMDS_SHIFT                              0
#define    AXITP_ONE_ATM_WRCMDS_WRCMDS_MASK                               0xFFFFFFFF

#define AXITP_ONE_ATM_RDCMDS_OFFSET                                       0x00000014
#define AXITP_ONE_ATM_RDCMDS_TYPE                                         UInt32
#define AXITP_ONE_ATM_RDCMDS_RESERVED_MASK                                0x00000000
#define    AXITP_ONE_ATM_RDCMDS_RDCMDS_SHIFT                              0
#define    AXITP_ONE_ATM_RDCMDS_RDCMDS_MASK                               0xFFFFFFFF

#define AXITP_ONE_ATM_AWCYCLES_OFFSET                                     0x00000018
#define AXITP_ONE_ATM_AWCYCLES_TYPE                                       UInt32
#define AXITP_ONE_ATM_AWCYCLES_RESERVED_MASK                              0x00000000
#define    AXITP_ONE_ATM_AWCYCLES_AWCYCLES_SHIFT                          0
#define    AXITP_ONE_ATM_AWCYCLES_AWCYCLES_MASK                           0xFFFFFFFF

#define AXITP_ONE_ATM_ARCYCLES_OFFSET                                     0x0000001C
#define AXITP_ONE_ATM_ARCYCLES_TYPE                                       UInt32
#define AXITP_ONE_ATM_ARCYCLES_RESERVED_MASK                              0x00000000
#define    AXITP_ONE_ATM_ARCYCLES_ARCYCLES_SHIFT                          0
#define    AXITP_ONE_ATM_ARCYCLES_ARCYCLES_MASK                           0xFFFFFFFF

#define AXITP_ONE_ATM_WCYCLES_OFFSET                                      0x00000020
#define AXITP_ONE_ATM_WCYCLES_TYPE                                        UInt32
#define AXITP_ONE_ATM_WCYCLES_RESERVED_MASK                               0x00000000
#define    AXITP_ONE_ATM_WCYCLES_WCYCLES_SHIFT                            0
#define    AXITP_ONE_ATM_WCYCLES_WCYCLES_MASK                             0xFFFFFFFF

#define AXITP_ONE_ATM_RCYCLES_OFFSET                                      0x00000024
#define AXITP_ONE_ATM_RCYCLES_TYPE                                        UInt32
#define AXITP_ONE_ATM_RCYCLES_RESERVED_MASK                               0x00000000
#define    AXITP_ONE_ATM_RCYCLES_RCYCLES_SHIFT                            0
#define    AXITP_ONE_ATM_RCYCLES_RCYCLES_MASK                             0xFFFFFFFF

#define AXITP_ONE_ATM_BCYCLES_OFFSET                                      0x00000028
#define AXITP_ONE_ATM_BCYCLES_TYPE                                        UInt32
#define AXITP_ONE_ATM_BCYCLES_RESERVED_MASK                               0x00000000
#define    AXITP_ONE_ATM_BCYCLES_BCYCLES_SHIFT                            0
#define    AXITP_ONE_ATM_BCYCLES_BCYCLES_MASK                             0xFFFFFFFF

#define AXITP_ONE_ATM_AWBUSY_OFFSET                                       0x0000002C
#define AXITP_ONE_ATM_AWBUSY_TYPE                                         UInt32
#define AXITP_ONE_ATM_AWBUSY_RESERVED_MASK                                0x00000000
#define    AXITP_ONE_ATM_AWBUSY_AWBUSY_SHIFT                              0
#define    AXITP_ONE_ATM_AWBUSY_AWBUSY_MASK                               0xFFFFFFFF

#define AXITP_ONE_ATM_ARBUSY_OFFSET                                       0x00000030
#define AXITP_ONE_ATM_ARBUSY_TYPE                                         UInt32
#define AXITP_ONE_ATM_ARBUSY_RESERVED_MASK                                0x00000000
#define    AXITP_ONE_ATM_ARBUSY_ARBUSY_SHIFT                              0
#define    AXITP_ONE_ATM_ARBUSY_ARBUSY_MASK                               0xFFFFFFFF

#define AXITP_ONE_ATM_WBUSY_OFFSET                                        0x00000034
#define AXITP_ONE_ATM_WBUSY_TYPE                                          UInt32
#define AXITP_ONE_ATM_WBUSY_RESERVED_MASK                                 0x00000000
#define    AXITP_ONE_ATM_WBUSY_WBUSY_SHIFT                                0
#define    AXITP_ONE_ATM_WBUSY_WBUSY_MASK                                 0xFFFFFFFF

#define AXITP_ONE_ATM_RBUSY_OFFSET                                        0x00000038
#define AXITP_ONE_ATM_RBUSY_TYPE                                          UInt32
#define AXITP_ONE_ATM_RBUSY_RESERVED_MASK                                 0x00000000
#define    AXITP_ONE_ATM_RBUSY_RBUSY_SHIFT                                0
#define    AXITP_ONE_ATM_RBUSY_RBUSY_MASK                                 0xFFFFFFFF

#define AXITP_ONE_ATM_BBUSY_OFFSET                                        0x0000003C
#define AXITP_ONE_ATM_BBUSY_TYPE                                          UInt32
#define AXITP_ONE_ATM_BBUSY_RESERVED_MASK                                 0x00000000
#define    AXITP_ONE_ATM_BBUSY_BBUSY_SHIFT                                0
#define    AXITP_ONE_ATM_BBUSY_BBUSY_MASK                                 0xFFFFFFFF

#define AXITP_ONE_ATM_WRSUM_OFFSET                                        0x00000040
#define AXITP_ONE_ATM_WRSUM_TYPE                                          UInt32
#define AXITP_ONE_ATM_WRSUM_RESERVED_MASK                                 0x00000000
#define    AXITP_ONE_ATM_WRSUM_WRSUM_SHIFT                                0
#define    AXITP_ONE_ATM_WRSUM_WRSUM_MASK                                 0xFFFFFFFF

#define AXITP_ONE_ATM_RDSUM_OFFSET                                        0x00000044
#define AXITP_ONE_ATM_RDSUM_TYPE                                          UInt32
#define AXITP_ONE_ATM_RDSUM_RESERVED_MASK                                 0x00000000
#define    AXITP_ONE_ATM_RDSUM_RDSUM_SHIFT                                0
#define    AXITP_ONE_ATM_RDSUM_RDSUM_MASK                                 0xFFFFFFFF

#define AXITP_ONE_ATM_WRMIN_OFFSET                                        0x00000048
#define AXITP_ONE_ATM_WRMIN_TYPE                                          UInt32
#define AXITP_ONE_ATM_WRMIN_RESERVED_MASK                                 0xFFFFFC00
#define    AXITP_ONE_ATM_WRMIN_WRMIN_SHIFT                                0
#define    AXITP_ONE_ATM_WRMIN_WRMIN_MASK                                 0x000003FF

#define AXITP_ONE_ATM_RDMIN_OFFSET                                        0x0000004C
#define AXITP_ONE_ATM_RDMIN_TYPE                                          UInt32
#define AXITP_ONE_ATM_RDMIN_RESERVED_MASK                                 0xFFFFFC00
#define    AXITP_ONE_ATM_RDMIN_RDMIN_SHIFT                                0
#define    AXITP_ONE_ATM_RDMIN_RDMIN_MASK                                 0x000003FF

#define AXITP_ONE_ATM_WRMAX_OFFSET                                        0x00000050
#define AXITP_ONE_ATM_WRMAX_TYPE                                          UInt32
#define AXITP_ONE_ATM_WRMAX_RESERVED_MASK                                 0xFFFFC000
#define    AXITP_ONE_ATM_WRMAX_WRMAX_SHIFT                                0
#define    AXITP_ONE_ATM_WRMAX_WRMAX_MASK                                 0x00003FFF

#define AXITP_ONE_ATM_RDMAX_OFFSET                                        0x00000054
#define AXITP_ONE_ATM_RDMAX_TYPE                                          UInt32
#define AXITP_ONE_ATM_RDMAX_RESERVED_MASK                                 0xFFFFC000
#define    AXITP_ONE_ATM_RDMAX_RDMAX_SHIFT                                0
#define    AXITP_ONE_ATM_RDMAX_RDMAX_MASK                                 0x00003FFF

#define AXITP_ONE_ATM_WRBEATS_OFFSET                                      0x00000058
#define AXITP_ONE_ATM_WRBEATS_TYPE                                        UInt32
#define AXITP_ONE_ATM_WRBEATS_RESERVED_MASK                               0x00000000
#define    AXITP_ONE_ATM_WRBEATS_WRBEATS_SHIFT                            0
#define    AXITP_ONE_ATM_WRBEATS_WRBEATS_MASK                             0xFFFFFFFF

#define AXITP_ONE_ATM_RDBEATS_OFFSET                                      0x0000005C
#define AXITP_ONE_ATM_RDBEATS_TYPE                                        UInt32
#define AXITP_ONE_ATM_RDBEATS_RESERVED_MASK                               0x00000000
#define    AXITP_ONE_ATM_RDBEATS_RDBEATS_SHIFT                            0
#define    AXITP_ONE_ATM_RDBEATS_RDBEATS_MASK                             0xFFFFFFFF

#define AXITP_ONE_ATM_WROUTS_OFFSET                                       0x00000060
#define AXITP_ONE_ATM_WROUTS_TYPE                                         UInt32
#define AXITP_ONE_ATM_WROUTS_RESERVED_MASK                                0x00000000
#define    AXITP_ONE_ATM_WROUTS_WROUTS_SHIFT                              0
#define    AXITP_ONE_ATM_WROUTS_WROUTS_MASK                               0xFFFFFFFF

#define AXITP_ONE_ATM_RDOUTS_OFFSET                                       0x00000064
#define AXITP_ONE_ATM_RDOUTS_TYPE                                         UInt32
#define AXITP_ONE_ATM_RDOUTS_RESERVED_MASK                                0x00000000
#define    AXITP_ONE_ATM_RDOUTS_RDOUTS_SHIFT                              0
#define    AXITP_ONE_ATM_RDOUTS_RDOUTS_MASK                               0xFFFFFFFF

#define AXITP_ONE_ATM_FILTER_0_OFFSET                                     0x00000080
#define AXITP_ONE_ATM_FILTER_0_TYPE                                       UInt32
#define AXITP_ONE_ATM_FILTER_0_RESERVED_MASK                              0x00000000
#define    AXITP_ONE_ATM_FILTER_0_TRIGGER_EN_0_SHIFT                      31
#define    AXITP_ONE_ATM_FILTER_0_TRIGGER_EN_0_MASK                       0x80000000
#define    AXITP_ONE_ATM_FILTER_0_FILTER_SEC_0_SHIFT                      30
#define    AXITP_ONE_ATM_FILTER_0_FILTER_SEC_0_MASK                       0x40000000
#define    AXITP_ONE_ATM_FILTER_0_FILTER_OPEN_0_SHIFT                     29
#define    AXITP_ONE_ATM_FILTER_0_FILTER_OPEN_0_MASK                      0x20000000
#define    AXITP_ONE_ATM_FILTER_0_FILTER_ID_0_SHIFT                       16
#define    AXITP_ONE_ATM_FILTER_0_FILTER_ID_0_MASK                        0x1FFF0000
#define    AXITP_ONE_ATM_FILTER_0_ID_MASK_0_SHIFT                         8
#define    AXITP_ONE_ATM_FILTER_0_ID_MASK_0_MASK                          0x0000FF00
#define    AXITP_ONE_ATM_FILTER_0_FILTER_LEN_0_SHIFT                      4
#define    AXITP_ONE_ATM_FILTER_0_FILTER_LEN_0_MASK                       0x000000F0
#define    AXITP_ONE_ATM_FILTER_0_LEN_MODE_0_SHIFT                        2
#define    AXITP_ONE_ATM_FILTER_0_LEN_MODE_0_MASK                         0x0000000C
#define    AXITP_ONE_ATM_FILTER_0_FILTER_WRITE_0_SHIFT                    1
#define    AXITP_ONE_ATM_FILTER_0_FILTER_WRITE_0_MASK                     0x00000002
#define    AXITP_ONE_ATM_FILTER_0_FILTER_READ_0_SHIFT                     0
#define    AXITP_ONE_ATM_FILTER_0_FILTER_READ_0_MASK                      0x00000001

#define AXITP_ONE_ATM_ADDRLOW_0_OFFSET                                    0x00000084
#define AXITP_ONE_ATM_ADDRLOW_0_TYPE                                      UInt32
#define AXITP_ONE_ATM_ADDRLOW_0_RESERVED_MASK                             0x000000FF
#define    AXITP_ONE_ATM_ADDRLOW_0_ADDR_LOW_0_SHIFT                       8
#define    AXITP_ONE_ATM_ADDRLOW_0_ADDR_LOW_0_MASK                        0xFFFFFF00

#define AXITP_ONE_ATM_ADDRHIGH_0_OFFSET                                   0x00000088
#define AXITP_ONE_ATM_ADDRHIGH_0_TYPE                                     UInt32
#define AXITP_ONE_ATM_ADDRHIGH_0_RESERVED_MASK                            0x000000FF
#define    AXITP_ONE_ATM_ADDRHIGH_0_ADDR_HIGH_0_SHIFT                     8
#define    AXITP_ONE_ATM_ADDRHIGH_0_ADDR_HIGH_0_MASK                      0xFFFFFF00

#endif /* __BRCM_RDB_AXITP_ONE_H__ */


