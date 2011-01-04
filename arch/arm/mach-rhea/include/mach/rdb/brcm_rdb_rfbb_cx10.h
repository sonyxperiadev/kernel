/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
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
/*     Date     : Generated on 11/9/2010 1:17:6                                             */
/*     RDB file : //R4/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_RFBB_CX10_H__
#define __BRCM_RDB_RFBB_CX10_H__

#define RFBB_CX10_TCCR_OFFSET                                             0x00000000
#define RFBB_CX10_TCCR_TYPE                                               UInt32
#define RFBB_CX10_TCCR_RESERVED_MASK                                      0x00000000
#define    RFBB_CX10_TCCR_RESERVED_31TO9_SHIFT                            9
#define    RFBB_CX10_TCCR_RESERVED_31TO9_MASK                             0xFFFFFE00
#define    RFBB_CX10_TCCR_CLK_REQ_SHIFT                                   8
#define    RFBB_CX10_TCCR_CLK_REQ_MASK                                    0x00000100
#define    RFBB_CX10_TCCR_RESERVED_7TO5_SHIFT                             5
#define    RFBB_CX10_TCCR_RESERVED_7TO5_MASK                              0x000000E0
#define    RFBB_CX10_TCCR_TMF_SHIFT                                       4
#define    RFBB_CX10_TCCR_TMF_MASK                                        0x00000010
#define    RFBB_CX10_TCCR_RESERVED_3TO1_SHIFT                             1
#define    RFBB_CX10_TCCR_RESERVED_3TO1_MASK                              0x0000000E
#define    RFBB_CX10_TCCR_TCEN_SHIFT                                      0
#define    RFBB_CX10_TCCR_TCEN_MASK                                       0x00000001

#define RFBB_CX10_TCPR_OFFSET                                             0x00000004
#define RFBB_CX10_TCPR_TYPE                                               UInt32
#define RFBB_CX10_TCPR_RESERVED_MASK                                      0x00000000
#define    RFBB_CX10_TCPR_TCL_SHIFT                                       0
#define    RFBB_CX10_TCPR_TCL_MASK                                        0xFFFFFFFF

#define RFBB_CX10_TCRR_OFFSET                                             0x00000008
#define RFBB_CX10_TCRR_TYPE                                               UInt32
#define RFBB_CX10_TCRR_RESERVED_MASK                                      0x00000000
#define    RFBB_CX10_TCRR_TCR_SHIFT                                       0
#define    RFBB_CX10_TCRR_TCR_MASK                                        0xFFFFFFFF

#define RFBB_CX10_TCOR_OFFSET                                             0x0000000C
#define RFBB_CX10_TCOR_TYPE                                               UInt32
#define RFBB_CX10_TCOR_RESERVED_MASK                                      0x00000000
#define    RFBB_CX10_TCOR_OFFSET_SHIFT                                    0
#define    RFBB_CX10_TCOR_OFFSET_MASK                                     0xFFFFFFFF

#define RFBB_CX10_PORTCR_OFFSET                                           0x00000010
#define RFBB_CX10_PORTCR_TYPE                                             UInt32
#define RFBB_CX10_PORTCR_RESERVED_MASK                                    0x00000000
#define    RFBB_CX10_PORTCR_RESERVED_31TO9_SHIFT                          9
#define    RFBB_CX10_PORTCR_RESERVED_31TO9_MASK                           0xFFFFFE00
#define    RFBB_CX10_PORTCR_SPI_FB_CLK_SEL_SHIFT                          8
#define    RFBB_CX10_PORTCR_SPI_FB_CLK_SEL_MASK                           0x00000100
#define    RFBB_CX10_PORTCR_GPEN_SPI_SEL2_SHIFT                           7
#define    RFBB_CX10_PORTCR_GPEN_SPI_SEL2_MASK                            0x00000080
#define    RFBB_CX10_PORTCR_GPEN_SPI_SEL1_SHIFT                           6
#define    RFBB_CX10_PORTCR_GPEN_SPI_SEL1_MASK                            0x00000040
#define    RFBB_CX10_PORTCR_GPEN_SPI_SEL0_SHIFT                           5
#define    RFBB_CX10_PORTCR_GPEN_SPI_SEL0_MASK                            0x00000020
#define    RFBB_CX10_PORTCR_COM_GPEN_SEL_SHIFT                            4
#define    RFBB_CX10_PORTCR_COM_GPEN_SEL_MASK                             0x00000010
#define    RFBB_CX10_PORTCR_RESERVED_3TO3_SHIFT                           3
#define    RFBB_CX10_PORTCR_RESERVED_3TO3_MASK                            0x00000008
#define    RFBB_CX10_PORTCR_COM_MODE_STATUS_SHIFT                         2
#define    RFBB_CX10_PORTCR_COM_MODE_STATUS_MASK                          0x00000004
#define    RFBB_CX10_PORTCR_COM_MODE_TIMED_SHIFT                          1
#define    RFBB_CX10_PORTCR_COM_MODE_TIMED_MASK                           0x00000002
#define    RFBB_CX10_PORTCR_COM_MODE_SHIFT                                0
#define    RFBB_CX10_PORTCR_COM_MODE_MASK                                 0x00000001

#define RFBB_CX10_PORTTR_OFFSET                                           0x00000014
#define RFBB_CX10_PORTTR_TYPE                                             UInt32
#define RFBB_CX10_PORTTR_RESERVED_MASK                                    0x00000000
#define    RFBB_CX10_PORTTR_PORTTIME_SHIFT                                0
#define    RFBB_CX10_PORTTR_PORTTIME_MASK                                 0xFFFFFFFF

#define RFBB_CX10_IMR_OFFSET                                              0x00000020
#define RFBB_CX10_IMR_TYPE                                                UInt32
#define RFBB_CX10_IMR_RESERVED_MASK                                       0x00000000
#define    RFBB_CX10_IMR_RESERVED_31TO16_SHIFT                            16
#define    RFBB_CX10_IMR_RESERVED_31TO16_MASK                             0xFFFF0000
#define    RFBB_CX10_IMR_MASK_SHIFT                                       0
#define    RFBB_CX10_IMR_MASK_MASK                                        0x0000FFFF

#define RFBB_CX10_ISR_OFFSET                                              0x00000024
#define RFBB_CX10_ISR_TYPE                                                UInt32
#define RFBB_CX10_ISR_RESERVED_MASK                                       0x00000000
#define    RFBB_CX10_ISR_RESERVED_31TO16_SHIFT                            16
#define    RFBB_CX10_ISR_RESERVED_31TO16_MASK                             0xFFFF0000
#define    RFBB_CX10_ISR_STATUS_SHIFT                                     0
#define    RFBB_CX10_ISR_STATUS_MASK                                      0x0000FFFF

#define RFBB_CX10_IPR_OFFSET                                              0x00000028
#define RFBB_CX10_IPR_TYPE                                                UInt32
#define RFBB_CX10_IPR_RESERVED_MASK                                       0x00000000
#define    RFBB_CX10_IPR_RESERVED_31TO16_SHIFT                            16
#define    RFBB_CX10_IPR_RESERVED_31TO16_MASK                             0xFFFF0000
#define    RFBB_CX10_IPR_PENDING_SHIFT                                    0
#define    RFBB_CX10_IPR_PENDING_MASK                                     0x0000FFFF

#define RFBB_CX10_ICR_OFFSET                                              0x0000002C
#define RFBB_CX10_ICR_TYPE                                                UInt32
#define RFBB_CX10_ICR_RESERVED_MASK                                       0x00000000
#define    RFBB_CX10_ICR_RESERVED_31TO16_SHIFT                            16
#define    RFBB_CX10_ICR_RESERVED_31TO16_MASK                             0xFFFF0000
#define    RFBB_CX10_ICR_CLEAR_SHIFT                                      0
#define    RFBB_CX10_ICR_CLEAR_MASK                                       0x0000FFFF

#define RFBB_CX10_INTTR_0_OFFSET                                          0x00000030
#define RFBB_CX10_INTTR_0_TYPE                                            UInt32
#define RFBB_CX10_INTTR_0_RESERVED_MASK                                   0x00000000
#define    RFBB_CX10_INTTR_0_INTTIME_SHIFT                                0
#define    RFBB_CX10_INTTR_0_INTTIME_MASK                                 0xFFFFFFFF

#define RFBB_CX10_INTTR_1_OFFSET                                          0x00000034
#define RFBB_CX10_INTTR_1_TYPE                                            UInt32
#define RFBB_CX10_INTTR_1_RESERVED_MASK                                   0x00000000
#define    RFBB_CX10_INTTR_1_INTTIME_SHIFT                                0
#define    RFBB_CX10_INTTR_1_INTTIME_MASK                                 0xFFFFFFFF

#define RFBB_CX10_INTTR_2_OFFSET                                          0x00000038
#define RFBB_CX10_INTTR_2_TYPE                                            UInt32
#define RFBB_CX10_INTTR_2_RESERVED_MASK                                   0x00000000
#define    RFBB_CX10_INTTR_2_INTTIME_SHIFT                                0
#define    RFBB_CX10_INTTR_2_INTTIME_MASK                                 0xFFFFFFFF

#define RFBB_CX10_INTTR_3_OFFSET                                          0x0000003C
#define RFBB_CX10_INTTR_3_TYPE                                            UInt32
#define RFBB_CX10_INTTR_3_RESERVED_MASK                                   0x00000000
#define    RFBB_CX10_INTTR_3_INTTIME_SHIFT                                0
#define    RFBB_CX10_INTTR_3_INTTIME_MASK                                 0xFFFFFFFF

#define RFBB_CX10_INTTR_4_OFFSET                                          0x00000040
#define RFBB_CX10_INTTR_4_TYPE                                            UInt32
#define RFBB_CX10_INTTR_4_RESERVED_MASK                                   0x00000000
#define    RFBB_CX10_INTTR_4_INTTIME_SHIFT                                0
#define    RFBB_CX10_INTTR_4_INTTIME_MASK                                 0xFFFFFFFF

#define RFBB_CX10_INTTR_5_OFFSET                                          0x00000044
#define RFBB_CX10_INTTR_5_TYPE                                            UInt32
#define RFBB_CX10_INTTR_5_RESERVED_MASK                                   0x00000000
#define    RFBB_CX10_INTTR_5_INTTIME_SHIFT                                0
#define    RFBB_CX10_INTTR_5_INTTIME_MASK                                 0xFFFFFFFF

#define RFBB_CX10_INTTR_6_OFFSET                                          0x00000048
#define RFBB_CX10_INTTR_6_TYPE                                            UInt32
#define RFBB_CX10_INTTR_6_RESERVED_MASK                                   0x00000000
#define    RFBB_CX10_INTTR_6_INTTIME_SHIFT                                0
#define    RFBB_CX10_INTTR_6_INTTIME_MASK                                 0xFFFFFFFF

#define RFBB_CX10_INTTR_7_OFFSET                                          0x0000004C
#define RFBB_CX10_INTTR_7_TYPE                                            UInt32
#define RFBB_CX10_INTTR_7_RESERVED_MASK                                   0x00000000
#define    RFBB_CX10_INTTR_7_INTTIME_SHIFT                                0
#define    RFBB_CX10_INTTR_7_INTTIME_MASK                                 0xFFFFFFFF

#define RFBB_CX10_INTTR_8_OFFSET                                          0x00000050
#define RFBB_CX10_INTTR_8_TYPE                                            UInt32
#define RFBB_CX10_INTTR_8_RESERVED_MASK                                   0x00000000
#define    RFBB_CX10_INTTR_8_INTTIME_SHIFT                                0
#define    RFBB_CX10_INTTR_8_INTTIME_MASK                                 0xFFFFFFFF

#define RFBB_CX10_INTTR_9_OFFSET                                          0x00000054
#define RFBB_CX10_INTTR_9_TYPE                                            UInt32
#define RFBB_CX10_INTTR_9_RESERVED_MASK                                   0x00000000
#define    RFBB_CX10_INTTR_9_INTTIME_SHIFT                                0
#define    RFBB_CX10_INTTR_9_INTTIME_MASK                                 0xFFFFFFFF

#define RFBB_CX10_INTTR_10_OFFSET                                         0x00000058
#define RFBB_CX10_INTTR_10_TYPE                                           UInt32
#define RFBB_CX10_INTTR_10_RESERVED_MASK                                  0x00000000
#define    RFBB_CX10_INTTR_10_INTTIME_SHIFT                               0
#define    RFBB_CX10_INTTR_10_INTTIME_MASK                                0xFFFFFFFF

#define RFBB_CX10_INTTR_11_OFFSET                                         0x0000005C
#define RFBB_CX10_INTTR_11_TYPE                                           UInt32
#define RFBB_CX10_INTTR_11_RESERVED_MASK                                  0x00000000
#define    RFBB_CX10_INTTR_11_INTTIME_SHIFT                               0
#define    RFBB_CX10_INTTR_11_INTTIME_MASK                                0xFFFFFFFF

#define RFBB_CX10_INTTR_12_OFFSET                                         0x00000060
#define RFBB_CX10_INTTR_12_TYPE                                           UInt32
#define RFBB_CX10_INTTR_12_RESERVED_MASK                                  0x00000000
#define    RFBB_CX10_INTTR_12_INTTIME_SHIFT                               0
#define    RFBB_CX10_INTTR_12_INTTIME_MASK                                0xFFFFFFFF

#define RFBB_CX10_INTTR_13_OFFSET                                         0x00000064
#define RFBB_CX10_INTTR_13_TYPE                                           UInt32
#define RFBB_CX10_INTTR_13_RESERVED_MASK                                  0x00000000
#define    RFBB_CX10_INTTR_13_INTTIME_SHIFT                               0
#define    RFBB_CX10_INTTR_13_INTTIME_MASK                                0xFFFFFFFF

#define RFBB_CX10_INTTR_14_OFFSET                                         0x00000068
#define RFBB_CX10_INTTR_14_TYPE                                           UInt32
#define RFBB_CX10_INTTR_14_RESERVED_MASK                                  0x00000000
#define    RFBB_CX10_INTTR_14_INTTIME_SHIFT                               0
#define    RFBB_CX10_INTTR_14_INTTIME_MASK                                0xFFFFFFFF

#define RFBB_CX10_INTTR_15_OFFSET                                         0x0000006C
#define RFBB_CX10_INTTR_15_TYPE                                           UInt32
#define RFBB_CX10_INTTR_15_RESERVED_MASK                                  0x00000000
#define    RFBB_CX10_INTTR_15_INTTIME_SHIFT                               0
#define    RFBB_CX10_INTTR_15_INTTIME_MASK                                0xFFFFFFFF

#define RFBB_CX10_DSTR_0_OFFSET                                           0x00000070
#define RFBB_CX10_DSTR_0_TYPE                                             UInt32
#define RFBB_CX10_DSTR_0_RESERVED_MASK                                    0x00000000
#define    RFBB_CX10_DSTR_0_DSTIME_SHIFT                                  0
#define    RFBB_CX10_DSTR_0_DSTIME_MASK                                   0xFFFFFFFF

#define RFBB_CX10_DSTR_1_OFFSET                                           0x00000074
#define RFBB_CX10_DSTR_1_TYPE                                             UInt32
#define RFBB_CX10_DSTR_1_RESERVED_MASK                                    0x00000000
#define    RFBB_CX10_DSTR_1_DSTIME_SHIFT                                  0
#define    RFBB_CX10_DSTR_1_DSTIME_MASK                                   0xFFFFFFFF

#define RFBB_CX10_DSTR_2_OFFSET                                           0x00000078
#define RFBB_CX10_DSTR_2_TYPE                                             UInt32
#define RFBB_CX10_DSTR_2_RESERVED_MASK                                    0x00000000
#define    RFBB_CX10_DSTR_2_DSTIME_SHIFT                                  0
#define    RFBB_CX10_DSTR_2_DSTIME_MASK                                   0xFFFFFFFF

#define RFBB_CX10_DSTR_3_OFFSET                                           0x0000007C
#define RFBB_CX10_DSTR_3_TYPE                                             UInt32
#define RFBB_CX10_DSTR_3_RESERVED_MASK                                    0x00000000
#define    RFBB_CX10_DSTR_3_DSTIME_SHIFT                                  0
#define    RFBB_CX10_DSTR_3_DSTIME_MASK                                   0xFFFFFFFF

#define RFBB_CX10_DSCR_0_OFFSET                                           0x00000080
#define RFBB_CX10_DSCR_0_TYPE                                             UInt32
#define RFBB_CX10_DSCR_0_RESERVED_MASK                                    0x00000000
#define    RFBB_CX10_DSCR_0_VALID_SHIFT                                   31
#define    RFBB_CX10_DSCR_0_VALID_MASK                                    0x80000000
#define    RFBB_CX10_DSCR_0_RESERVED_30TO24_SHIFT                         24
#define    RFBB_CX10_DSCR_0_RESERVED_30TO24_MASK                          0x7F000000
#define    RFBB_CX10_DSCR_0_CCU_PLL_PLAN_SHIFT                            16
#define    RFBB_CX10_DSCR_0_CCU_PLL_PLAN_MASK                             0x00FF0000
#define    RFBB_CX10_DSCR_0_RESERVED_15TO8_SHIFT                          8
#define    RFBB_CX10_DSCR_0_RESERVED_15TO8_MASK                           0x0000FF00
#define    RFBB_CX10_DSCR_0_PLL_PLAN_SHIFT                                0
#define    RFBB_CX10_DSCR_0_PLL_PLAN_MASK                                 0x000000FF

#define RFBB_CX10_DSCR_1_OFFSET                                           0x00000084
#define RFBB_CX10_DSCR_1_TYPE                                             UInt32
#define RFBB_CX10_DSCR_1_RESERVED_MASK                                    0x00000000
#define    RFBB_CX10_DSCR_1_VALID_SHIFT                                   31
#define    RFBB_CX10_DSCR_1_VALID_MASK                                    0x80000000
#define    RFBB_CX10_DSCR_1_RESERVED_30TO8_SHIFT                          8
#define    RFBB_CX10_DSCR_1_RESERVED_30TO8_MASK                           0x7FFFFF00
#define    RFBB_CX10_DSCR_1_PLL_PLAN_SHIFT                                0
#define    RFBB_CX10_DSCR_1_PLL_PLAN_MASK                                 0x000000FF

#define RFBB_CX10_DSCR_2_OFFSET                                           0x00000088
#define RFBB_CX10_DSCR_2_TYPE                                             UInt32
#define RFBB_CX10_DSCR_2_RESERVED_MASK                                    0x00000000
#define    RFBB_CX10_DSCR_2_VALID_SHIFT                                   31
#define    RFBB_CX10_DSCR_2_VALID_MASK                                    0x80000000
#define    RFBB_CX10_DSCR_2_RESERVED_30TO8_SHIFT                          8
#define    RFBB_CX10_DSCR_2_RESERVED_30TO8_MASK                           0x7FFFFF00
#define    RFBB_CX10_DSCR_2_PLL_PLAN_SHIFT                                0
#define    RFBB_CX10_DSCR_2_PLL_PLAN_MASK                                 0x000000FF

#define RFBB_CX10_DSCR_3_OFFSET                                           0x0000008C
#define RFBB_CX10_DSCR_3_TYPE                                             UInt32
#define RFBB_CX10_DSCR_3_RESERVED_MASK                                    0x00000000
#define    RFBB_CX10_DSCR_3_VALID_SHIFT                                   31
#define    RFBB_CX10_DSCR_3_VALID_MASK                                    0x80000000
#define    RFBB_CX10_DSCR_3_RESERVED_30TO8_SHIFT                          8
#define    RFBB_CX10_DSCR_3_RESERVED_30TO8_MASK                           0x7FFFFF00
#define    RFBB_CX10_DSCR_3_PLL_PLAN_SHIFT                                0
#define    RFBB_CX10_DSCR_3_PLL_PLAN_MASK                                 0x000000FF

#define RFBB_CX10_CNTR_OFFSET                                             0x00000090
#define RFBB_CX10_CNTR_TYPE                                               UInt32
#define RFBB_CX10_CNTR_RESERVED_MASK                                      0x00000000
#define    RFBB_CX10_CNTR_CNTR_SHIFT                                      0
#define    RFBB_CX10_CNTR_CNTR_MASK                                       0xFFFFFFFF

typedef volatile struct {
   UInt32 m_TCCR;                    // 0x0000
   UInt32 m_TCPR;                    // 0x0004
   UInt32 m_TCRR;                    // 0x0008
   UInt32 m_TCOR;                    // 0x000C
   UInt32 m_PORTCR;                  // 0x0010
   UInt32 m_PORTTR;                  // 0x0014
   UInt32 RESERVED_20_14[2];
   UInt32 m_IMR;                     // 0x0020
   UInt32 m_ISR;                     // 0x0024
   UInt32 m_IPR;                     // 0x0028
   UInt32 m_ICR;                     // 0x002C
   UInt32 m_INTTR_0;                 // 0x0030
   UInt32 m_INTTR_1;                 // 0x0034
   UInt32 m_INTTR_2;                 // 0x0038
   UInt32 m_INTTR_3;                 // 0x003C
   UInt32 m_INTTR_4;                 // 0x0040
   UInt32 m_INTTR_5;                 // 0x0044
   UInt32 m_INTTR_6;                 // 0x0048
   UInt32 m_INTTR_7;                 // 0x004C
   UInt32 m_INTTR_8;                 // 0x0050
   UInt32 m_INTTR_9;                 // 0x0054
   UInt32 m_INTTR_10;                // 0x0058
   UInt32 m_INTTR_11;                // 0x005C
   UInt32 m_INTTR_12;                // 0x0060
   UInt32 m_INTTR_13;                // 0x0064
   UInt32 m_INTTR_14;                // 0x0068
   UInt32 m_INTTR_15;                // 0x006C
   UInt32 m_DSTR_0;                  // 0x0070
   UInt32 m_DSTR_1;                  // 0x0074
   UInt32 m_DSTR_2;                  // 0x0078
   UInt32 m_DSTR_3;                  // 0x007C
   UInt32 m_DSCR_0;                  // 0x0080
   UInt32 m_DSCR_1;                  // 0x0084
   UInt32 m_DSCR_2;                  // 0x0088
   UInt32 m_DSCR_3;                  // 0x008C
   UInt32 m_CNTR;                    // 0x0090
} BRCM_RFBB_CX10_REGS;


#endif /* __BRCM_RDB_RFBB_CX10_H__ */


