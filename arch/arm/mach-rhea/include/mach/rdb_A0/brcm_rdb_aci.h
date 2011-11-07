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

#ifndef __BRCM_RDB_ACI_H__
#define __BRCM_RDB_ACI_H__

#define ACI_MICINTH_DEF_OFFSET                                            0x00000030
#define ACI_MICINTH_DEF_TYPE                                              UInt32
#define ACI_MICINTH_DEF_RESERVED_MASK                                     0xFFFFFF80
#define    ACI_MICINTH_DEF_MICINTH_DEF_SHIFT                              0
#define    ACI_MICINTH_DEF_MICINTH_DEF_MASK                               0x0000007F

#define ACI_MICINENTH_DEF_OFFSET                                          0x00000034
#define ACI_MICINENTH_DEF_TYPE                                            UInt32
#define ACI_MICINENTH_DEF_RESERVED_MASK                                   0xFFFFFF80
#define    ACI_MICINENTH_DEF_MICINENTH_DEF_SHIFT                          0
#define    ACI_MICINENTH_DEF_MICINENTH_DEF_MASK                           0x0000007F

#define ACI_MICONTH_DEF_OFFSET                                            0x00000038
#define ACI_MICONTH_DEF_TYPE                                              UInt32
#define ACI_MICONTH_DEF_RESERVED_MASK                                     0xFFFFFF80
#define    ACI_MICONTH_DEF_MICONTH_DEF_SHIFT                              0
#define    ACI_MICONTH_DEF_MICONTH_DEF_MASK                               0x0000007F

#define ACI_MICONENTH_DEF_OFFSET                                          0x0000003C
#define ACI_MICONENTH_DEF_TYPE                                            UInt32
#define ACI_MICONENTH_DEF_RESERVED_MASK                                   0xFFFFFF80
#define    ACI_MICONENTH_DEF_MICONENTH_DEF_SHIFT                          0
#define    ACI_MICONENTH_DEF_MICONENTH_DEF_MASK                           0x0000007F

#define ACI_MICINTH_OFFSET                                                0x00000040
#define ACI_MICINTH_TYPE                                                  UInt32
#define ACI_MICINTH_RESERVED_MASK                                         0xFFFFFF80
#define    ACI_MICINTH_MICINTH_SHIFT                                      0
#define    ACI_MICINTH_MICINTH_MASK                                       0x0000007F

#define ACI_MICONTH_OFFSET                                                0x00000044
#define ACI_MICONTH_TYPE                                                  UInt32
#define ACI_MICONTH_RESERVED_MASK                                         0xFFFFFF80
#define    ACI_MICONTH_MICONTH_SHIFT                                      0
#define    ACI_MICONTH_MICONTH_MASK                                       0x0000007F

#define ACI_ADCIN_SEL_OFFSET                                              0x00000048
#define ACI_ADCIN_SEL_TYPE                                                UInt32
#define ACI_ADCIN_SEL_RESERVED_MASK                                       0xFFFFFFFC
#define    ACI_ADCIN_SEL_ADCIN_SEL_SHIFT                                  0
#define    ACI_ADCIN_SEL_ADCIN_SEL_MASK                                   0x00000003
#define       ACI_ADCIN_SEL_ADCIN_SEL_CMD_SW4A_DOWN                       0x00000001
#define       ACI_ADCIN_SEL_ADCIN_SEL_CMD_SW4B_DOWN                       0x00000002

#define ACI_ADC_PWD_OFFSET                                                0x0000004C
#define ACI_ADC_PWD_TYPE                                                  UInt32
#define ACI_ADC_PWD_RESERVED_MASK                                         0xFFFFFFF0
#define    ACI_ADC_PWD_ADC2_DIS_CHOP_SHIFT                                3
#define    ACI_ADC_PWD_ADC2_DIS_CHOP_MASK                                 0x00000008
#define       ACI_ADC_PWD_ADC2_DIS_CHOP_CMD_NOT_LOADING                   0x00000000
#define       ACI_ADC_PWD_ADC2_DIS_CHOP_CMD_LOADING                       0x00000001
#define    ACI_ADC_PWD_ADC2_PWD_SHIFT                                     2
#define    ACI_ADC_PWD_ADC2_PWD_MASK                                      0x00000004
#define       ACI_ADC_PWD_ADC2_PWD_CMD_POWER_UP                           0x00000000
#define       ACI_ADC_PWD_ADC2_PWD_CMD_POWER_DOWN                         0x00000001
#define    ACI_ADC_PWD_ADC1_DIS_CHOP_SHIFT                                1
#define    ACI_ADC_PWD_ADC1_DIS_CHOP_MASK                                 0x00000002
#define       ACI_ADC_PWD_ADC1_DIS_CHOP_CMD_NOT_LOADING                   0x00000000
#define       ACI_ADC_PWD_ADC1_DIS_CHOP_CMD_LOADING                       0x00000001
#define    ACI_ADC_PWD_ADC1_PWD_SHIFT                                     0
#define    ACI_ADC_PWD_ADC1_PWD_MASK                                      0x00000001
#define       ACI_ADC_PWD_ADC1_PWD_CMD_POWER_UP                           0x00000000
#define       ACI_ADC_PWD_ADC1_PWD_CMD_POWER_DOWN                         0x00000001

#define ACI_COMP1TH_OFFSET                                                0x00000050
#define ACI_COMP1TH_TYPE                                                  UInt32
#define ACI_COMP1TH_RESERVED_MASK                                         0xFFFFFF00
#define    ACI_COMP1TH_COMP1TH_SHIFT                                      0
#define    ACI_COMP1TH_COMP1TH_MASK                                       0x000000FF

#define ACI_COMP2TH_OFFSET                                                0x00000054
#define ACI_COMP2TH_TYPE                                                  UInt32
#define ACI_COMP2TH_RESERVED_MASK                                         0xFFFFFF00
#define    ACI_COMP2TH_COMP2TH_SHIFT                                      0
#define    ACI_COMP2TH_COMP2TH_MASK                                       0x000000FF

#define ACI_ECIDATA_OFFSET                                                0x00000058
#define ACI_ECIDATA_TYPE                                                  UInt32
#define ACI_ECIDATA_RESERVED_MASK                                         0xFFFFFFFE
#define    ACI_ECIDATA_ECIDATA_SHIFT                                      0
#define    ACI_ECIDATA_ECIDATA_MASK                                       0x00000001
#define       ACI_ECIDATA_ECIDATA_CMD_OFF                                 0x00000000
#define       ACI_ECIDATA_ECIDATA_CMD_ON                                  0x00000001

#define ACI_COMP1MODE_OFFSET                                              0x0000005C
#define ACI_COMP1MODE_TYPE                                                UInt32
#define ACI_COMP1MODE_RESERVED_MASK                                       0xFFFFFFFE
#define    ACI_COMP1MODE_COMP1MODE_SHIFT                                  0
#define    ACI_COMP1MODE_COMP1MODE_MASK                                   0x00000001
#define       ACI_COMP1MODE_COMP1MODE_CMD_INTEGRATE_AND_DUMP              0x00000001
#define       ACI_COMP1MODE_COMP1MODE_CMD_DECIMATE_BY_M                   0x00000000

#define ACI_COMP1RESET_OFFSET                                             0x00000060
#define ACI_COMP1RESET_TYPE                                               UInt32
#define ACI_COMP1RESET_RESERVED_MASK                                      0xFFFFFFFC
#define    ACI_COMP1RESET_COMP1RESET_SHIFT                                1
#define    ACI_COMP1RESET_COMP1RESET_MASK                                 0x00000002
#define       ACI_COMP1RESET_COMP1RESET_CMD_RST_SIGNAL_BY_FW              0x00000001
#define    ACI_COMP1RESET_COMP1RESET_SW_SHIFT                             0
#define    ACI_COMP1RESET_COMP1RESET_SW_MASK                              0x00000001
#define       ACI_COMP1RESET_COMP1RESET_SW_CMD_FILTERS_CMP1OUT2_RST_BY_COMP1 0x00000001
#define       ACI_COMP1RESET_COMP1RESET_SW_CMD_FILTERS_CMP1OUT2_RST_BY_FW 0x00000000

#define ACI_COMP1RESETFLAG_OFFSET                                         0x00000064
#define ACI_COMP1RESETFLAG_TYPE                                           UInt32
#define ACI_COMP1RESETFLAG_RESERVED_MASK                                  0xFFFFFFFE
#define    ACI_COMP1RESETFLAG_COMP1RESETFLAG_SHIFT                        0
#define    ACI_COMP1RESETFLAG_COMP1RESETFLAG_MASK                         0x00000001

#define ACI_COMP2MODE_OFFSET                                              0x00000068
#define ACI_COMP2MODE_TYPE                                                UInt32
#define ACI_COMP2MODE_RESERVED_MASK                                       0xFFFFFFFE
#define    ACI_COMP2MODE_COMP2MODE_SHIFT                                  0
#define    ACI_COMP2MODE_COMP2MODE_MASK                                   0x00000001
#define       ACI_COMP2MODE_COMP2MODE_CMD_INTEGRATE_AND_DUMP              0x00000001
#define       ACI_COMP2MODE_COMP2MODE_CMD_DECIMATE_BY_M                   0x00000000

#define ACI_COMP2RESET_OFFSET                                             0x0000006C
#define ACI_COMP2RESET_TYPE                                               UInt32
#define ACI_COMP2RESET_RESERVED_MASK                                      0xFFFFFFFC
#define    ACI_COMP2RESET_COMP2RESET_SHIFT                                1
#define    ACI_COMP2RESET_COMP2RESET_MASK                                 0x00000002
#define    ACI_COMP2RESET_COMP2RESET_SW_SHIFT                             0
#define    ACI_COMP2RESET_COMP2RESET_SW_MASK                              0x00000001

#define ACI_COMP2RESETFLAG_OFFSET                                         0x00000070
#define ACI_COMP2RESETFLAG_TYPE                                           UInt32
#define ACI_COMP2RESETFLAG_RESERVED_MASK                                  0xFFFFFFFE
#define    ACI_COMP2RESETFLAG_COMP2RESETFLAG_SHIFT                        0
#define    ACI_COMP2RESETFLAG_COMP2RESETFLAG_MASK                         0x00000001

#define ACI_N1_OFFSET                                                     0x00000074
#define ACI_N1_TYPE                                                       UInt32
#define ACI_N1_RESERVED_MASK                                              0xFFFFFFF0
#define    ACI_N1_N1_SHIFT                                                0
#define    ACI_N1_N1_MASK                                                 0x0000000F

#define ACI_N2_OFFSET                                                     0x00000078
#define ACI_N2_TYPE                                                       UInt32
#define ACI_N2_RESERVED_MASK                                              0xFFFFFFF0
#define    ACI_N2_N2_SHIFT                                                0
#define    ACI_N2_N2_MASK                                                 0x0000000F

#define ACI_T1_OFFSET                                                     0x0000007C
#define ACI_T1_TYPE                                                       UInt32
#define ACI_T1_RESERVED_MASK                                              0xFFFFFF00
#define    ACI_T1_T1_SHIFT                                                0
#define    ACI_T1_T1_MASK                                                 0x000000FF

#define ACI_T2_OFFSET                                                     0x00000080
#define ACI_T2_TYPE                                                       UInt32
#define ACI_T2_RESERVED_MASK                                              0xFFFFFF00
#define    ACI_T2_T2_SHIFT                                                0
#define    ACI_T2_T2_MASK                                                 0x000000FF

#define ACI_S1_OFFSET                                                     0x00000084
#define ACI_S1_TYPE                                                       UInt32
#define ACI_S1_RESERVED_MASK                                              0xFFFFFF00
#define    ACI_S1_S1_SHIFT                                                0
#define    ACI_S1_S1_MASK                                                 0x000000FF

#define ACI_S2_OFFSET                                                     0x00000088
#define ACI_S2_TYPE                                                       UInt32
#define ACI_S2_RESERVED_MASK                                              0xFFFFFF00
#define    ACI_S2_S2_SHIFT                                                0
#define    ACI_S2_S2_MASK                                                 0x000000FF

#define ACI_M1_OFFSET                                                     0x0000008C
#define ACI_M1_TYPE                                                       UInt32
#define ACI_M1_RESERVED_MASK                                              0xFFFF0000
#define    ACI_M1_M1_SHIFT                                                0
#define    ACI_M1_M1_MASK                                                 0x0000FFFF

#define ACI_MT1_OFFSET                                                    0x00000090
#define ACI_MT1_TYPE                                                      UInt32
#define ACI_MT1_RESERVED_MASK                                             0xFFFF0000
#define    ACI_MT1_MT1_SHIFT                                              0
#define    ACI_MT1_MT1_MASK                                               0x0000FFFF

#define ACI_M2_OFFSET                                                     0x00000094
#define ACI_M2_TYPE                                                       UInt32
#define ACI_M2_RESERVED_MASK                                              0xFFFF0000
#define    ACI_M2_M2_SHIFT                                                0
#define    ACI_M2_M2_MASK                                                 0x0000FFFF

#define ACI_MT2_OFFSET                                                    0x00000098
#define ACI_MT2_TYPE                                                      UInt32
#define ACI_MT2_RESERVED_MASK                                             0xFFFF0000
#define    ACI_MT2_MT2_SHIFT                                              0
#define    ACI_MT2_MT2_MASK                                               0x0000FFFF

#define ACI_COMP_DOUT_OFFSET                                              0x0000009C
#define ACI_COMP_DOUT_TYPE                                                UInt32
#define ACI_COMP_DOUT_RESERVED_MASK                                       0xFFFFFFC0
#define    ACI_COMP_DOUT_COMP2_DOUT2_SHIFT                                5
#define    ACI_COMP_DOUT_COMP2_DOUT2_MASK                                 0x00000020
#define       ACI_COMP_DOUT_COMP2_DOUT2_CMD_ZERO                          0x00000000
#define       ACI_COMP_DOUT_COMP2_DOUT2_CMD_ONE                           0x00000001
#define    ACI_COMP_DOUT_COMP2_DOUT1_SHIFT                                4
#define    ACI_COMP_DOUT_COMP2_DOUT1_MASK                                 0x00000010
#define       ACI_COMP_DOUT_COMP2_DOUT1_CMD_ZERO                          0x00000000
#define       ACI_COMP_DOUT_COMP2_DOUT1_CMD_ONE                           0x00000001
#define    ACI_COMP_DOUT_COMP2_DOUT_SHIFT                                 3
#define    ACI_COMP_DOUT_COMP2_DOUT_MASK                                  0x00000008
#define       ACI_COMP_DOUT_COMP2_DOUT_CMD_ZERO                           0x00000000
#define       ACI_COMP_DOUT_COMP2_DOUT_CMD_ONE                            0x00000001
#define    ACI_COMP_DOUT_COMP1_DOUT2_SHIFT                                2
#define    ACI_COMP_DOUT_COMP1_DOUT2_MASK                                 0x00000004
#define       ACI_COMP_DOUT_COMP1_DOUT2_CMD_ZERO                          0x00000000
#define       ACI_COMP_DOUT_COMP1_DOUT2_CMD_ONE                           0x00000001
#define    ACI_COMP_DOUT_COMP1_DOUT1_SHIFT                                1
#define    ACI_COMP_DOUT_COMP1_DOUT1_MASK                                 0x00000002
#define       ACI_COMP_DOUT_COMP1_DOUT1_CMD_ZERO                          0x00000000
#define       ACI_COMP_DOUT_COMP1_DOUT1_CMD_ONE                           0x00000001
#define    ACI_COMP_DOUT_COMP1_DOUT_SHIFT                                 0
#define    ACI_COMP_DOUT_COMP1_DOUT_MASK                                  0x00000001
#define       ACI_COMP_DOUT_COMP1_DOUT_CMD_ZERO                           0x00000000
#define       ACI_COMP_DOUT_COMP1_DOUT_CMD_ONE                            0x00000001

#define ACI_ADC1_DOUT_OFFSET                                              0x000000A0
#define ACI_ADC1_DOUT_TYPE                                                UInt32
#define ACI_ADC1_DOUT_RESERVED_MASK                                       0xFFFFFC00
#define    ACI_ADC1_DOUT_ADC1_DOUT_SHIFT                                  0
#define    ACI_ADC1_DOUT_ADC1_DOUT_MASK                                   0x000003FF

#define ACI_ADC1_DOUTF_OFFSET                                             0x000000A4
#define ACI_ADC1_DOUTF_TYPE                                               UInt32
#define ACI_ADC1_DOUTF_RESERVED_MASK                                      0xFFFFFC00
#define    ACI_ADC1_DOUTF_ADC1_DOUTF_SHIFT                                0
#define    ACI_ADC1_DOUTF_ADC1_DOUTF_MASK                                 0x000003FF

#define ACI_ADC2_DOUT_OFFSET                                              0x000000A8
#define ACI_ADC2_DOUT_TYPE                                                UInt32
#define ACI_ADC2_DOUT_RESERVED_MASK                                       0xFFFFFC00
#define    ACI_ADC2_DOUT_ADC2_DOUT_SHIFT                                  0
#define    ACI_ADC2_DOUT_ADC2_DOUT_MASK                                   0x000003FF
#define       ACI_ADC2_DOUT_ADC2_DOUT_CMD_MIN                             0x00000000
#define       ACI_ADC2_DOUT_ADC2_DOUT_CMD_MAX                             0x000003FF

#define ACI_ADC2_DOUTF_OFFSET                                             0x000000AC
#define ACI_ADC2_DOUTF_TYPE                                               UInt32
#define ACI_ADC2_DOUTF_RESERVED_MASK                                      0xFFFFFC00
#define    ACI_ADC2_DOUTF_ADC2_DOUTF_SHIFT                                0
#define    ACI_ADC2_DOUTF_ADC2_DOUTF_MASK                                 0x000003FF

#define ACI_COMP_PWD_OFFSET                                               0x000000B0
#define ACI_COMP_PWD_TYPE                                                 UInt32
#define ACI_COMP_PWD_RESERVED_MASK                                        0xFFFFFFFC
#define    ACI_COMP_PWD_COMP2_PWD_SHIFT                                   1
#define    ACI_COMP_PWD_COMP2_PWD_MASK                                    0x00000002
#define       ACI_COMP_PWD_COMP2_PWD_CMD_POWER_DOWN                       0x00000001
#define       ACI_COMP_PWD_COMP2_PWD_CMD_POWER_UP                         0x00000000
#define    ACI_COMP_PWD_COMP1_PWD_SHIFT                                   0
#define    ACI_COMP_PWD_COMP1_PWD_MASK                                    0x00000001
#define       ACI_COMP_PWD_COMP1_PWD_CMD_POWER_DOWN                       0x00000001
#define       ACI_COMP_PWD_COMP1_PWD_CMD_POWER_UP                         0x00000000

#define ACI_COMPTH_SET_OFFSET                                             0x000000B4
#define ACI_COMPTH_SET_TYPE                                               UInt32
#define ACI_COMPTH_SET_RESERVED_MASK                                      0xFFFFFFFC
#define    ACI_COMPTH_SET_COMPTH2_SET_SHIFT                               1
#define    ACI_COMPTH_SET_COMPTH2_SET_MASK                                0x00000002
#define       ACI_COMPTH_SET_COMPTH2_SET_CMD_NOT_LOADING                  0x00000000
#define       ACI_COMPTH_SET_COMPTH2_SET_CMD_LOADING                      0x00000001
#define    ACI_COMPTH_SET_COMPTH1_SET_SHIFT                               0
#define    ACI_COMPTH_SET_COMPTH1_SET_MASK                                0x00000001
#define       ACI_COMPTH_SET_COMPTH1_SET_CMD_NOT_LOADING                  0x00000000
#define       ACI_COMPTH_SET_COMPTH1_SET_CMD_LOADING                      0x00000001

#define ACI_ADC1TH_OFFSET                                                 0x000000B8
#define ACI_ADC1TH_TYPE                                                   UInt32
#define ACI_ADC1TH_RESERVED_MASK                                          0xFFFFFF00
#define    ACI_ADC1TH_ADC1TH_SHIFT                                        0
#define    ACI_ADC1TH_ADC1TH_MASK                                         0x000000FF

#define ACI_ADC2TH_OFFSET                                                 0x000000BC
#define ACI_ADC2TH_TYPE                                                   UInt32
#define ACI_ADC2TH_RESERVED_MASK                                          0xFFFFFF00
#define    ACI_ADC2TH_ADC2TH_SHIFT                                        0
#define    ACI_ADC2TH_ADC2TH_MASK                                         0x000000FF

#define ACI_ADCTH_SET_OFFSET                                              0x000000C0
#define ACI_ADCTH_SET_TYPE                                                UInt32
#define ACI_ADCTH_SET_RESERVED_MASK                                       0xFFFFFFFC
#define    ACI_ADCTH_SET_ADC2TH_SET_SHIFT                                 1
#define    ACI_ADCTH_SET_ADC2TH_SET_MASK                                  0x00000002
#define    ACI_ADCTH_SET_ADC1TH_SET_SHIFT                                 0
#define    ACI_ADCTH_SET_ADC1TH_SET_MASK                                  0x00000001

#define ACI_MIC_BIAS_OFFSET                                               0x000000C4
#define ACI_MIC_BIAS_TYPE                                                 UInt32
#define ACI_MIC_BIAS_RESERVED_MASK                                        0xFFFFFFFC
#define    ACI_MIC_BIAS_MIC_BIAS_SHIFT                                    1
#define    ACI_MIC_BIAS_MIC_BIAS_MASK                                     0x00000002
#define       ACI_MIC_BIAS_MIC_BIAS_CMD_PERIODIC_MEASUREMENT_EN           0x00000001
#define       ACI_MIC_BIAS_MIC_BIAS_CMD_PERIODIC_MEASUREMENT_DIS          0x00000000
#define    ACI_MIC_BIAS_MIC_AUX_BIAS_GND_SHIFT                            0
#define    ACI_MIC_BIAS_MIC_AUX_BIAS_GND_MASK                             0x00000001
#define       ACI_MIC_BIAS_MIC_AUX_BIAS_GND_CMD_IMPEDANCE_CONTROL_HI_Z    0x00000000
#define       ACI_MIC_BIAS_MIC_AUX_BIAS_GND_CMD_IMPEDANCE_CONTROL_LO_Z    0x00000001

#define ACI_ACI_TX_INV_OFFSET                                             0x000000C8
#define ACI_ACI_TX_INV_TYPE                                               UInt32
#define ACI_ACI_TX_INV_RESERVED_MASK                                      0xFFFFFFFE
#define    ACI_ACI_TX_INV_ACI_TX_INV_SHIFT                                0
#define    ACI_ACI_TX_INV_ACI_TX_INV_MASK                                 0x00000001
#define       ACI_ACI_TX_INV_ACI_TX_INV_CMD_INVETED                       0x00000001

#define ACI_INT_OFFSET                                                    0x000000CC
#define ACI_INT_TYPE                                                      UInt32
#define ACI_INT_RESERVED_MASK                                             0xFFFFFF88
#define    ACI_INT_INV_COMP2INT_EN_SHIFT                                  6
#define    ACI_INT_INV_COMP2INT_EN_MASK                                   0x00000040
#define    ACI_INT_COMP2INT_EN_SHIFT                                      5
#define    ACI_INT_COMP2INT_EN_MASK                                       0x00000020
#define    ACI_INT_COMP1INT_EN_SHIFT                                      4
#define    ACI_INT_COMP1INT_EN_MASK                                       0x00000010
#define    ACI_INT_INV_COMP2INT_STS_SHIFT                                 2
#define    ACI_INT_INV_COMP2INT_STS_MASK                                  0x00000004
#define    ACI_INT_COMP2INT_STS_SHIFT                                     1
#define    ACI_INT_COMP2INT_STS_MASK                                      0x00000002
#define    ACI_INT_COMP1INT_STS_SHIFT                                     0
#define    ACI_INT_COMP1INT_STS_MASK                                      0x00000001

#define ACI_ACI_CTRL_OFFSET                                               0x000000D0
#define ACI_ACI_CTRL_TYPE                                                 UInt32
#define ACI_ACI_CTRL_RESERVED_MASK                                        0xFFFFFFCE
#define    ACI_ACI_CTRL_SW_MIC_DATAB_SHIFT                                5
#define    ACI_ACI_CTRL_SW_MIC_DATAB_MASK                                 0x00000020
#define    ACI_ACI_CTRL_SW_ACI_SHIFT                                      4
#define    ACI_ACI_CTRL_SW_ACI_MASK                                       0x00000010
#define    ACI_ACI_CTRL_ACI_BIAS_PWRDN_SHIFT                              0
#define    ACI_ACI_CTRL_ACI_BIAS_PWRDN_MASK                               0x00000001

#define ACI_ADC_CTRL_OFFSET                                               0x000000D4
#define ACI_ADC_CTRL_TYPE                                                 UInt32
#define ACI_ADC_CTRL_RESERVED_MASK                                        0xFFFFFF00
#define    ACI_ADC_CTRL_AUDIORX_VREF_PWRUP_SHIFT                          7
#define    ACI_ADC_CTRL_AUDIORX_VREF_PWRUP_MASK                           0x00000080
#define    ACI_ADC_CTRL_AUDIORX_BIAS_PWRUP_SHIFT                          6
#define    ACI_ADC_CTRL_AUDIORX_BIAS_PWRUP_MASK                           0x00000040
#define    ACI_ADC_CTRL_ADC_FS_BIG_SMALL_B_SHIFT                          5
#define    ACI_ADC_CTRL_ADC_FS_BIG_SMALL_B_MASK                           0x00000020
#define    ACI_ADC_CTRL_ADC_STEPSIZE_SET_SHIFT                            4
#define    ACI_ADC_CTRL_ADC_STEPSIZE_SET_MASK                             0x00000010
#define    ACI_ADC_CTRL_ADC_STEPSIZE_SHIFT                                1
#define    ACI_ADC_CTRL_ADC_STEPSIZE_MASK                                 0x0000000E
#define    ACI_ADC_CTRL_ADC_ADAPTIVE_EN_SHIFT                             0
#define    ACI_ADC_CTRL_ADC_ADAPTIVE_EN_MASK                              0x00000001

#define ACI_SLEEP_CTRL_OFFSET                                             0x000000D8
#define ACI_SLEEP_CTRL_TYPE                                               UInt32
#define ACI_SLEEP_CTRL_RESERVED_MASK                                      0xFFFFFCCE
#define    ACI_SLEEP_CTRL_CHARGEUP_TIME_SHIFT                             8
#define    ACI_SLEEP_CTRL_CHARGEUP_TIME_MASK                              0x00000300
#define    ACI_SLEEP_CTRL_RELEASE_TIME_SHIFT                              4
#define    ACI_SLEEP_CTRL_RELEASE_TIME_MASK                               0x00000030
#define    ACI_SLEEP_CTRL_WEAK_SLEEP_EN_SHIFT                             0
#define    ACI_SLEEP_CTRL_WEAK_SLEEP_EN_MASK                              0x00000001

#define ACI_ID_OFFSET                                                     0x00000400
#define ACI_ID_TYPE                                                       UInt32
#define ACI_ID_RESERVED_MASK                                              0xFFFF0000
#define    ACI_ID_ACIID_SHIFT                                             0
#define    ACI_ID_ACIID_MASK                                              0x0000FFFF

#define ACI_COMR_OFFSET                                                   0x00000404
#define ACI_COMR_TYPE                                                     UInt32
#define ACI_COMR_RESERVED_MASK                                            0xFFFFFE00
#define    ACI_COMR_BUSHT_SHIFT                                           8
#define    ACI_COMR_BUSHT_MASK                                            0x00000100
#define       ACI_COMR_BUSHT_CMD_BUSHOLDTIME_2MS                          0x00000000
#define       ACI_COMR_BUSHT_CMD_BUSHOLDTIME_10MS                         0x00000001
#define    ACI_COMR_DBIDOC_SHIFT                                          7
#define    ACI_COMR_DBIDOC_MASK                                           0x00000080
#define       ACI_COMR_DBIDOC_CMD_INUSE                                   0x00000000
#define       ACI_COMR_DBIDOC_CMD_DISABLED                                0x00000001
#define    ACI_COMR_DBIDOS_SHIFT                                          6
#define    ACI_COMR_DBIDOS_MASK                                           0x00000040
#define       ACI_COMR_DBIDOS_CMD_TXDOWN                                  0x00000000
#define       ACI_COMR_DBIDOS_CMD_TXUP                                    0x00000001
#define    ACI_COMR_DBIDIC_SHIFT                                          5
#define    ACI_COMR_DBIDIC_MASK                                           0x00000020
#define       ACI_COMR_DBIDIC_CMD_NORMAL                                  0x00000000
#define       ACI_COMR_DBIDIC_CMD_DATAREG                                 0x00000001
#define    ACI_COMR_DBIDIS_SHIFT                                          4
#define    ACI_COMR_DBIDIS_MASK                                           0x00000010
#define       ACI_COMR_DBIDIS_CMD_SPEEDREG                                0x00000000
#define       ACI_COMR_DBIDIS_CMD_RXMSB                                   0x00000001
#define    ACI_COMR_DBICON_SHIFT                                          3
#define    ACI_COMR_DBICON_MASK                                           0x00000008
#define       ACI_COMR_DBICON_CMD_DIRECT                                  0x00000000
#define       ACI_COMR_DBICON_CMD_VOTE                                    0x00000001
#define    ACI_COMR_DBICMD_SHIFT                                          0
#define    ACI_COMR_DBICMD_MASK                                           0x00000007
#define       ACI_COMR_DBICMD_CMD_NOP                                     0x00000000
#define       ACI_COMR_DBICMD_CMD_RESET                                   0x00000001
#define       ACI_COMR_DBICMD_CMD_SEND                                    0x00000002
#define       ACI_COMR_DBICMD_CMD_RECV                                    0x00000003
#define       ACI_COMR_DBICMD_CMD_RECV_LEARN                              0x00000004

#define ACI_TXDAR_OFFSET                                                  0x00000408
#define ACI_TXDAR_TYPE                                                    UInt32
#define ACI_TXDAR_RESERVED_MASK                                           0xFFFFFF00
#define    ACI_TXDAR_ACITXDAR_SHIFT                                       0
#define    ACI_TXDAR_ACITXDAR_MASK                                        0x000000FF

#define ACI_RXDAR_OFFSET                                                  0x0000040C
#define ACI_RXDAR_TYPE                                                    UInt32
#define ACI_RXDAR_RESERVED_MASK                                           0xFFFFFF00
#define    ACI_RXDAR_ACIRXDAR_SHIFT                                       0
#define    ACI_RXDAR_ACIRXDAR_MASK                                        0x000000FF

#define ACI_IMR_OFFSET                                                    0x00000410
#define ACI_IMR_TYPE                                                      UInt32
#define ACI_IMR_RESERVED_MASK                                             0xFFFFFE00
#define    ACI_IMR_ACIIMR_SHIFT                                           0
#define    ACI_IMR_ACIIMR_MASK                                            0x000001FF
#define       ACI_IMR_ACIIMR_CMD_ACI_INT                                  0x00000001
#define       ACI_IMR_ACIIMR_CMD_DATA_RECVED                              0x00000002
#define       ACI_IMR_ACIIMR_CMD_RDY_SEND_SENT                            0x00000004
#define       ACI_IMR_ACIIMR_CMD_SPD_SET                                  0x00000008
#define       ACI_IMR_ACIIMR_CMD_IF_ERR                                   0x00000010
#define       ACI_IMR_ACIIMR_CMD_FRAMING                                  0x00000020
#define       ACI_IMR_ACIIMR_CMD_RESET_LEARN                              0x00000040
#define       ACI_IMR_ACIIMR_CMD_BUS_COLLISION                            0x00000080
#define       ACI_IMR_ACIIMR_CMD_RET_IDLE                                 0x00000100

#define ACI_IIDR_OFFSET                                                   0x00000414
#define ACI_IIDR_TYPE                                                     UInt32
#define ACI_IIDR_RESERVED_MASK                                            0xFFFFFE00
#define    ACI_IIDR_ACIIIDR_SHIFT                                         0
#define    ACI_IIDR_ACIIIDR_MASK                                          0x000001FF

#define ACI_SPLR_OFFSET                                                   0x00000418
#define ACI_SPLR_TYPE                                                     UInt32
#define ACI_SPLR_RESERVED_MASK                                            0xFFFFFFC0
#define    ACI_SPLR_ACISPLR_SHIFT                                         0
#define    ACI_SPLR_ACISPLR_MASK                                          0x0000003F

#define ACI_ITR_OFFSET                                                    0x0000041C
#define ACI_ITR_TYPE                                                      UInt32
#define ACI_ITR_RESERVED_MASK                                             0xFFFFFFFC
#define    ACI_ITR_ACIITR_SHIFT                                           0
#define    ACI_ITR_ACIITR_MASK                                            0x00000003

#define ACI_DEBUG_OFFSET                                                  0x00000420
#define ACI_DEBUG_TYPE                                                    UInt32
#define ACI_DEBUG_RESERVED_MASK                                           0xFFFFFFF0
#define    ACI_DEBUG_ACIDEBUG_SHIFT                                       2
#define    ACI_DEBUG_ACIDEBUG_MASK                                        0x0000000C
#define    ACI_DEBUG_ACI_RTX_SHORT_SHIFT                                  1
#define    ACI_DEBUG_ACI_RTX_SHORT_MASK                                   0x00000002
#define    ACI_DEBUG_ACIDEBUG_CLR_SHIFT                                   0
#define    ACI_DEBUG_ACIDEBUG_CLR_MASK                                    0x00000001

#define ACI_DSP_MICINTH_DEF_OFFSET                                        0x00000018
#define ACI_DSP_MICINTH_DEF_TYPE                                          UInt32
#define ACI_DSP_MICINTH_DEF_RESERVED_MASK                                 0xFFFFFF80
#define    ACI_DSP_MICINTH_DEF_MICINTH_DEF_SHIFT                          0
#define    ACI_DSP_MICINTH_DEF_MICINTH_DEF_MASK                           0x0000007F

#define ACI_DSP_MICINENTH_DEF_OFFSET                                      0x0000001A
#define ACI_DSP_MICINENTH_DEF_TYPE                                        UInt32
#define ACI_DSP_MICINENTH_DEF_RESERVED_MASK                               0xFFFFFF80
#define    ACI_DSP_MICINENTH_DEF_MICINENTH_DEF_SHIFT                      0
#define    ACI_DSP_MICINENTH_DEF_MICINENTH_DEF_MASK                       0x0000007F

#define ACI_DSP_MICONTH_DEF_OFFSET                                        0x0000001C
#define ACI_DSP_MICONTH_DEF_TYPE                                          UInt32
#define ACI_DSP_MICONTH_DEF_RESERVED_MASK                                 0xFFFFFF80
#define    ACI_DSP_MICONTH_DEF_MICONTH_DEF_SHIFT                          0
#define    ACI_DSP_MICONTH_DEF_MICONTH_DEF_MASK                           0x0000007F

#define ACI_DSP_MICONENTH_DEF_OFFSET                                      0x0000001E
#define ACI_DSP_MICONENTH_DEF_TYPE                                        UInt32
#define ACI_DSP_MICONENTH_DEF_RESERVED_MASK                               0xFFFFFF80
#define    ACI_DSP_MICONENTH_DEF_MICONENTH_DEF_SHIFT                      0
#define    ACI_DSP_MICONENTH_DEF_MICONENTH_DEF_MASK                       0x0000007F

#define ACI_DSP_MICINTH_OFFSET                                            0x00000020
#define ACI_DSP_MICINTH_TYPE                                              UInt32
#define ACI_DSP_MICINTH_RESERVED_MASK                                     0xFFFFFF80
#define    ACI_DSP_MICINTH_MICINTH_SHIFT                                  0
#define    ACI_DSP_MICINTH_MICINTH_MASK                                   0x0000007F

#define ACI_DSP_MICONTH_OFFSET                                            0x00000022
#define ACI_DSP_MICONTH_TYPE                                              UInt32
#define ACI_DSP_MICONTH_RESERVED_MASK                                     0xFFFFFF80
#define    ACI_DSP_MICONTH_MICONTH_SHIFT                                  0
#define    ACI_DSP_MICONTH_MICONTH_MASK                                   0x0000007F

#define ACI_DSP_ADCIN_SEL_OFFSET                                          0x00000024
#define ACI_DSP_ADCIN_SEL_TYPE                                            UInt32
#define ACI_DSP_ADCIN_SEL_RESERVED_MASK                                   0xFFFFFFFC
#define    ACI_DSP_ADCIN_SEL_ADCIN_SEL_SHIFT                              0
#define    ACI_DSP_ADCIN_SEL_ADCIN_SEL_MASK                               0x00000003
#define       ACI_DSP_ADCIN_SEL_ADCIN_SEL_CMD_SW4A_DOWN                   0x00000001
#define       ACI_DSP_ADCIN_SEL_ADCIN_SEL_CMD_SW4B_DOWN                   0x00000002

#define ACI_DSP_ADC_PWD_OFFSET                                            0x00000026
#define ACI_DSP_ADC_PWD_TYPE                                              UInt32
#define ACI_DSP_ADC_PWD_RESERVED_MASK                                     0xFFFFFFF0
#define    ACI_DSP_ADC_PWD_ADC2_DIS_CHOP_SHIFT                            3
#define    ACI_DSP_ADC_PWD_ADC2_DIS_CHOP_MASK                             0x00000008
#define       ACI_DSP_ADC_PWD_ADC2_DIS_CHOP_CMD_NOT_LOADING               0x00000000
#define       ACI_DSP_ADC_PWD_ADC2_DIS_CHOP_CMD_LOADING                   0x00000001
#define    ACI_DSP_ADC_PWD_ADC2_PWD_SHIFT                                 2
#define    ACI_DSP_ADC_PWD_ADC2_PWD_MASK                                  0x00000004
#define       ACI_DSP_ADC_PWD_ADC2_PWD_CMD_POWER_UP                       0x00000000
#define       ACI_DSP_ADC_PWD_ADC2_PWD_CMD_POWER_DOWN                     0x00000001
#define    ACI_DSP_ADC_PWD_ADC1_DIS_CHOP_SHIFT                            1
#define    ACI_DSP_ADC_PWD_ADC1_DIS_CHOP_MASK                             0x00000002
#define       ACI_DSP_ADC_PWD_ADC1_DIS_CHOP_CMD_NOT_LOADING               0x00000000
#define       ACI_DSP_ADC_PWD_ADC1_DIS_CHOP_CMD_LOADING                   0x00000001
#define    ACI_DSP_ADC_PWD_ADC1_PWD_SHIFT                                 0
#define    ACI_DSP_ADC_PWD_ADC1_PWD_MASK                                  0x00000001
#define       ACI_DSP_ADC_PWD_ADC1_PWD_CMD_POWER_UP                       0x00000000
#define       ACI_DSP_ADC_PWD_ADC1_PWD_CMD_POWER_DOWN                     0x00000001

#define ACI_DSP_COMP1TH_OFFSET                                            0x00000028
#define ACI_DSP_COMP1TH_TYPE                                              UInt32
#define ACI_DSP_COMP1TH_RESERVED_MASK                                     0xFFFFFF00
#define    ACI_DSP_COMP1TH_COMP1TH_SHIFT                                  0
#define    ACI_DSP_COMP1TH_COMP1TH_MASK                                   0x000000FF

#define ACI_DSP_COMP2TH_OFFSET                                            0x0000002A
#define ACI_DSP_COMP2TH_TYPE                                              UInt32
#define ACI_DSP_COMP2TH_RESERVED_MASK                                     0xFFFFFF00
#define    ACI_DSP_COMP2TH_COMP2TH_SHIFT                                  0
#define    ACI_DSP_COMP2TH_COMP2TH_MASK                                   0x000000FF

#define ACI_DSP_ECIDATA_OFFSET                                            0x0000002C
#define ACI_DSP_ECIDATA_TYPE                                              UInt32
#define ACI_DSP_ECIDATA_RESERVED_MASK                                     0xFFFFFFFE
#define    ACI_DSP_ECIDATA_ECIDATA_SHIFT                                  0
#define    ACI_DSP_ECIDATA_ECIDATA_MASK                                   0x00000001
#define       ACI_DSP_ECIDATA_ECIDATA_CMD_OFF                             0x00000000
#define       ACI_DSP_ECIDATA_ECIDATA_CMD_ON                              0x00000001

#define ACI_DSP_COMP1MODE_OFFSET                                          0x0000002E
#define ACI_DSP_COMP1MODE_TYPE                                            UInt32
#define ACI_DSP_COMP1MODE_RESERVED_MASK                                   0xFFFFFFFE
#define    ACI_DSP_COMP1MODE_COMP1MODE_SHIFT                              0
#define    ACI_DSP_COMP1MODE_COMP1MODE_MASK                               0x00000001
#define       ACI_DSP_COMP1MODE_COMP1MODE_CMD_INTEGRATE_AND_DUMP          0x00000001
#define       ACI_DSP_COMP1MODE_COMP1MODE_CMD_DECIMATE_BY_M               0x00000000

#define ACI_DSP_COMP1RESET_OFFSET                                         0x00000030
#define ACI_DSP_COMP1RESET_TYPE                                           UInt32
#define ACI_DSP_COMP1RESET_RESERVED_MASK                                  0xFFFFFFFC
#define    ACI_DSP_COMP1RESET_COMP1RESET_SHIFT                            1
#define    ACI_DSP_COMP1RESET_COMP1RESET_MASK                             0x00000002
#define       ACI_DSP_COMP1RESET_COMP1RESET_CMD_RST_SIGNAL_BY_FW          0x00000001
#define    ACI_DSP_COMP1RESET_COMP1RESET_SW_SHIFT                         0
#define    ACI_DSP_COMP1RESET_COMP1RESET_SW_MASK                          0x00000001
#define       ACI_DSP_COMP1RESET_COMP1RESET_SW_CMD_FILTERS_CMP1OUT2_RST_BY_COMP1 0x00000001
#define       ACI_DSP_COMP1RESET_COMP1RESET_SW_CMD_FILTERS_CMP1OUT2_RST_BY_FW 0x00000000

#define ACI_DSP_COMP1RESETFLAG_OFFSET                                     0x00000032
#define ACI_DSP_COMP1RESETFLAG_TYPE                                       UInt32
#define ACI_DSP_COMP1RESETFLAG_RESERVED_MASK                              0xFFFFFFFE
#define    ACI_DSP_COMP1RESETFLAG_COMP1RESETFLAG_SHIFT                    0
#define    ACI_DSP_COMP1RESETFLAG_COMP1RESETFLAG_MASK                     0x00000001

#define ACI_DSP_COMP2MODE_OFFSET                                          0x00000034
#define ACI_DSP_COMP2MODE_TYPE                                            UInt32
#define ACI_DSP_COMP2MODE_RESERVED_MASK                                   0xFFFFFFFE
#define    ACI_DSP_COMP2MODE_COMP2MODE_SHIFT                              0
#define    ACI_DSP_COMP2MODE_COMP2MODE_MASK                               0x00000001
#define       ACI_DSP_COMP2MODE_COMP2MODE_CMD_INTEGRATE_AND_DUMP          0x00000001
#define       ACI_DSP_COMP2MODE_COMP2MODE_CMD_DECIMATE_BY_M               0x00000000

#define ACI_DSP_COMP2RESET_OFFSET                                         0x00000036
#define ACI_DSP_COMP2RESET_TYPE                                           UInt32
#define ACI_DSP_COMP2RESET_RESERVED_MASK                                  0xFFFFFFFC
#define    ACI_DSP_COMP2RESET_COMP2RESET_SHIFT                            1
#define    ACI_DSP_COMP2RESET_COMP2RESET_MASK                             0x00000002
#define    ACI_DSP_COMP2RESET_COMP2RESET_SW_SHIFT                         0
#define    ACI_DSP_COMP2RESET_COMP2RESET_SW_MASK                          0x00000001

#define ACI_DSP_COMP2RESETFLAG_OFFSET                                     0x00000038
#define ACI_DSP_COMP2RESETFLAG_TYPE                                       UInt32
#define ACI_DSP_COMP2RESETFLAG_RESERVED_MASK                              0xFFFFFFFE
#define    ACI_DSP_COMP2RESETFLAG_COMP2RESETFLAG_SHIFT                    0
#define    ACI_DSP_COMP2RESETFLAG_COMP2RESETFLAG_MASK                     0x00000001

#define ACI_DSP_N1_OFFSET                                                 0x0000003A
#define ACI_DSP_N1_TYPE                                                   UInt32
#define ACI_DSP_N1_RESERVED_MASK                                          0xFFFFFFF0
#define    ACI_DSP_N1_N1_SHIFT                                            0
#define    ACI_DSP_N1_N1_MASK                                             0x0000000F

#define ACI_DSP_N2_OFFSET                                                 0x0000003C
#define ACI_DSP_N2_TYPE                                                   UInt32
#define ACI_DSP_N2_RESERVED_MASK                                          0xFFFFFFF0
#define    ACI_DSP_N2_N2_SHIFT                                            0
#define    ACI_DSP_N2_N2_MASK                                             0x0000000F

#define ACI_DSP_T1_OFFSET                                                 0x0000003E
#define ACI_DSP_T1_TYPE                                                   UInt32
#define ACI_DSP_T1_RESERVED_MASK                                          0xFFFFFF00
#define    ACI_DSP_T1_T1_SHIFT                                            0
#define    ACI_DSP_T1_T1_MASK                                             0x000000FF

#define ACI_DSP_T2_OFFSET                                                 0x00000040
#define ACI_DSP_T2_TYPE                                                   UInt32
#define ACI_DSP_T2_RESERVED_MASK                                          0xFFFFFF00
#define    ACI_DSP_T2_T2_SHIFT                                            0
#define    ACI_DSP_T2_T2_MASK                                             0x000000FF

#define ACI_DSP_S1_OFFSET                                                 0x00000042
#define ACI_DSP_S1_TYPE                                                   UInt32
#define ACI_DSP_S1_RESERVED_MASK                                          0xFFFFFF00
#define    ACI_DSP_S1_S1_SHIFT                                            0
#define    ACI_DSP_S1_S1_MASK                                             0x000000FF

#define ACI_DSP_S2_OFFSET                                                 0x00000044
#define ACI_DSP_S2_TYPE                                                   UInt32
#define ACI_DSP_S2_RESERVED_MASK                                          0xFFFFFF00
#define    ACI_DSP_S2_S2_SHIFT                                            0
#define    ACI_DSP_S2_S2_MASK                                             0x000000FF

#define ACI_DSP_M1_OFFSET                                                 0x00000046
#define ACI_DSP_M1_TYPE                                                   UInt32
#define ACI_DSP_M1_RESERVED_MASK                                          0xFFFF0000
#define    ACI_DSP_M1_M1_SHIFT                                            0
#define    ACI_DSP_M1_M1_MASK                                             0x0000FFFF

#define ACI_DSP_MT1_OFFSET                                                0x00000048
#define ACI_DSP_MT1_TYPE                                                  UInt32
#define ACI_DSP_MT1_RESERVED_MASK                                         0xFFFF0000
#define    ACI_DSP_MT1_MT1_SHIFT                                          0
#define    ACI_DSP_MT1_MT1_MASK                                           0x0000FFFF

#define ACI_DSP_M2_OFFSET                                                 0x0000004A
#define ACI_DSP_M2_TYPE                                                   UInt32
#define ACI_DSP_M2_RESERVED_MASK                                          0xFFFF0000
#define    ACI_DSP_M2_M2_SHIFT                                            0
#define    ACI_DSP_M2_M2_MASK                                             0x0000FFFF

#define ACI_DSP_MT2_OFFSET                                                0x0000004C
#define ACI_DSP_MT2_TYPE                                                  UInt32
#define ACI_DSP_MT2_RESERVED_MASK                                         0xFFFF0000
#define    ACI_DSP_MT2_MT2_SHIFT                                          0
#define    ACI_DSP_MT2_MT2_MASK                                           0x0000FFFF

#define ACI_DSP_COMP_DOUT_OFFSET                                          0x0000004E
#define ACI_DSP_COMP_DOUT_TYPE                                            UInt32
#define ACI_DSP_COMP_DOUT_RESERVED_MASK                                   0xFFFFFFC0
#define    ACI_DSP_COMP_DOUT_COMP2_DOUT2_SHIFT                            5
#define    ACI_DSP_COMP_DOUT_COMP2_DOUT2_MASK                             0x00000020
#define       ACI_DSP_COMP_DOUT_COMP2_DOUT2_CMD_ZERO                      0x00000000
#define       ACI_DSP_COMP_DOUT_COMP2_DOUT2_CMD_ONE                       0x00000001
#define    ACI_DSP_COMP_DOUT_COMP2_DOUT1_SHIFT                            4
#define    ACI_DSP_COMP_DOUT_COMP2_DOUT1_MASK                             0x00000010
#define       ACI_DSP_COMP_DOUT_COMP2_DOUT1_CMD_ZERO                      0x00000000
#define       ACI_DSP_COMP_DOUT_COMP2_DOUT1_CMD_ONE                       0x00000001
#define    ACI_DSP_COMP_DOUT_COMP2_DOUT_SHIFT                             3
#define    ACI_DSP_COMP_DOUT_COMP2_DOUT_MASK                              0x00000008
#define       ACI_DSP_COMP_DOUT_COMP2_DOUT_CMD_ZERO                       0x00000000
#define       ACI_DSP_COMP_DOUT_COMP2_DOUT_CMD_ONE                        0x00000001
#define    ACI_DSP_COMP_DOUT_COMP1_DOUT2_SHIFT                            2
#define    ACI_DSP_COMP_DOUT_COMP1_DOUT2_MASK                             0x00000004
#define       ACI_DSP_COMP_DOUT_COMP1_DOUT2_CMD_ZERO                      0x00000000
#define       ACI_DSP_COMP_DOUT_COMP1_DOUT2_CMD_ONE                       0x00000001
#define    ACI_DSP_COMP_DOUT_COMP1_DOUT1_SHIFT                            1
#define    ACI_DSP_COMP_DOUT_COMP1_DOUT1_MASK                             0x00000002
#define       ACI_DSP_COMP_DOUT_COMP1_DOUT1_CMD_ZERO                      0x00000000
#define       ACI_DSP_COMP_DOUT_COMP1_DOUT1_CMD_ONE                       0x00000001
#define    ACI_DSP_COMP_DOUT_COMP1_DOUT_SHIFT                             0
#define    ACI_DSP_COMP_DOUT_COMP1_DOUT_MASK                              0x00000001
#define       ACI_DSP_COMP_DOUT_COMP1_DOUT_CMD_ZERO                       0x00000000
#define       ACI_DSP_COMP_DOUT_COMP1_DOUT_CMD_ONE                        0x00000001

#define ACI_DSP_ADC1_DOUT_OFFSET                                          0x00000050
#define ACI_DSP_ADC1_DOUT_TYPE                                            UInt32
#define ACI_DSP_ADC1_DOUT_RESERVED_MASK                                   0xFFFFFC00
#define    ACI_DSP_ADC1_DOUT_ADC1_DOUT_SHIFT                              0
#define    ACI_DSP_ADC1_DOUT_ADC1_DOUT_MASK                               0x000003FF

#define ACI_DSP_ADC1_DOUTF_OFFSET                                         0x00000052
#define ACI_DSP_ADC1_DOUTF_TYPE                                           UInt32
#define ACI_DSP_ADC1_DOUTF_RESERVED_MASK                                  0xFFFFFC00
#define    ACI_DSP_ADC1_DOUTF_ADC1_DOUTF_SHIFT                            0
#define    ACI_DSP_ADC1_DOUTF_ADC1_DOUTF_MASK                             0x000003FF

#define ACI_DSP_ADC2_DOUT_OFFSET                                          0x00000054
#define ACI_DSP_ADC2_DOUT_TYPE                                            UInt32
#define ACI_DSP_ADC2_DOUT_RESERVED_MASK                                   0xFFFFFC00
#define    ACI_DSP_ADC2_DOUT_ADC2_DOUT_SHIFT                              0
#define    ACI_DSP_ADC2_DOUT_ADC2_DOUT_MASK                               0x000003FF
#define       ACI_DSP_ADC2_DOUT_ADC2_DOUT_CMD_MIN                         0x00000000
#define       ACI_DSP_ADC2_DOUT_ADC2_DOUT_CMD_MAX                         0x000003FF

#define ACI_DSP_ADC2_DOUTF_OFFSET                                         0x00000056
#define ACI_DSP_ADC2_DOUTF_TYPE                                           UInt32
#define ACI_DSP_ADC2_DOUTF_RESERVED_MASK                                  0xFFFFFC00
#define    ACI_DSP_ADC2_DOUTF_ADC2_DOUTF_SHIFT                            0
#define    ACI_DSP_ADC2_DOUTF_ADC2_DOUTF_MASK                             0x000003FF

#define ACI_DSP_COMP_PWD_OFFSET                                           0x00000058
#define ACI_DSP_COMP_PWD_TYPE                                             UInt32
#define ACI_DSP_COMP_PWD_RESERVED_MASK                                    0xFFFFFFFC
#define    ACI_DSP_COMP_PWD_COMP2_PWD_SHIFT                               1
#define    ACI_DSP_COMP_PWD_COMP2_PWD_MASK                                0x00000002
#define       ACI_DSP_COMP_PWD_COMP2_PWD_CMD_POWER_DOWN                   0x00000001
#define       ACI_DSP_COMP_PWD_COMP2_PWD_CMD_POWER_UP                     0x00000000
#define    ACI_DSP_COMP_PWD_COMP1_PWD_SHIFT                               0
#define    ACI_DSP_COMP_PWD_COMP1_PWD_MASK                                0x00000001
#define       ACI_DSP_COMP_PWD_COMP1_PWD_CMD_POWER_DOWN                   0x00000001
#define       ACI_DSP_COMP_PWD_COMP1_PWD_CMD_POWER_UP                     0x00000000

#define ACI_DSP_COMPTH_SET_OFFSET                                         0x0000005A
#define ACI_DSP_COMPTH_SET_TYPE                                           UInt32
#define ACI_DSP_COMPTH_SET_RESERVED_MASK                                  0xFFFFFFFC
#define    ACI_DSP_COMPTH_SET_COMPTH2_SET_SHIFT                           1
#define    ACI_DSP_COMPTH_SET_COMPTH2_SET_MASK                            0x00000002
#define       ACI_DSP_COMPTH_SET_COMPTH2_SET_CMD_NOT_LOADING              0x00000000
#define       ACI_DSP_COMPTH_SET_COMPTH2_SET_CMD_LOADING                  0x00000001
#define    ACI_DSP_COMPTH_SET_COMPTH1_SET_SHIFT                           0
#define    ACI_DSP_COMPTH_SET_COMPTH1_SET_MASK                            0x00000001
#define       ACI_DSP_COMPTH_SET_COMPTH1_SET_CMD_NOT_LOADING              0x00000000
#define       ACI_DSP_COMPTH_SET_COMPTH1_SET_CMD_LOADING                  0x00000001

#define ACI_DSP_ADC1TH_OFFSET                                             0x0000005C
#define ACI_DSP_ADC1TH_TYPE                                               UInt32
#define ACI_DSP_ADC1TH_RESERVED_MASK                                      0xFFFFFF00
#define    ACI_DSP_ADC1TH_ADC1TH_SHIFT                                    0
#define    ACI_DSP_ADC1TH_ADC1TH_MASK                                     0x000000FF

#define ACI_DSP_ADC2TH_OFFSET                                             0x0000005E
#define ACI_DSP_ADC2TH_TYPE                                               UInt32
#define ACI_DSP_ADC2TH_RESERVED_MASK                                      0xFFFFFF00
#define    ACI_DSP_ADC2TH_ADC2TH_SHIFT                                    0
#define    ACI_DSP_ADC2TH_ADC2TH_MASK                                     0x000000FF

#define ACI_DSP_ADCTH_SET_OFFSET                                          0x00000060
#define ACI_DSP_ADCTH_SET_TYPE                                            UInt32
#define ACI_DSP_ADCTH_SET_RESERVED_MASK                                   0xFFFFFFFC
#define    ACI_DSP_ADCTH_SET_ADC2TH_SET_SHIFT                             1
#define    ACI_DSP_ADCTH_SET_ADC2TH_SET_MASK                              0x00000002
#define    ACI_DSP_ADCTH_SET_ADC1TH_SET_SHIFT                             0
#define    ACI_DSP_ADCTH_SET_ADC1TH_SET_MASK                              0x00000001

#define ACI_DSP_MIC_BIAS_OFFSET                                           0x00000062
#define ACI_DSP_MIC_BIAS_TYPE                                             UInt32
#define ACI_DSP_MIC_BIAS_RESERVED_MASK                                    0xFFFFFFFC
#define    ACI_DSP_MIC_BIAS_MIC_BIAS_SHIFT                                1
#define    ACI_DSP_MIC_BIAS_MIC_BIAS_MASK                                 0x00000002
#define       ACI_DSP_MIC_BIAS_MIC_BIAS_CMD_PERIODIC_MEASUREMENT_EN       0x00000001
#define       ACI_DSP_MIC_BIAS_MIC_BIAS_CMD_PERIODIC_MEASUREMENT_DIS      0x00000000
#define    ACI_DSP_MIC_BIAS_MIC_AUX_BIAS_GND_SHIFT                        0
#define    ACI_DSP_MIC_BIAS_MIC_AUX_BIAS_GND_MASK                         0x00000001
#define       ACI_DSP_MIC_BIAS_MIC_AUX_BIAS_GND_CMD_IMPEDANCE_CONTROL_HI_Z 0x00000000
#define       ACI_DSP_MIC_BIAS_MIC_AUX_BIAS_GND_CMD_IMPEDANCE_CONTROL_LO_Z 0x00000001

#define ACI_DSP_ACI_TX_INV_OFFSET                                         0x00000064
#define ACI_DSP_ACI_TX_INV_TYPE                                           UInt32
#define ACI_DSP_ACI_TX_INV_RESERVED_MASK                                  0xFFFFFFFE
#define    ACI_DSP_ACI_TX_INV_ACI_TX_INV_SHIFT                            0
#define    ACI_DSP_ACI_TX_INV_ACI_TX_INV_MASK                             0x00000001
#define       ACI_DSP_ACI_TX_INV_ACI_TX_INV_CMD_INVETED                   0x00000001

#define ACI_DSP_INT_OFFSET                                                0x00000066
#define ACI_DSP_INT_TYPE                                                  UInt32
#define ACI_DSP_INT_RESERVED_MASK                                         0xFFFFFF88
#define    ACI_DSP_INT_INV_COMP2INT_EN_SHIFT                              6
#define    ACI_DSP_INT_INV_COMP2INT_EN_MASK                               0x00000040
#define    ACI_DSP_INT_COMP2INT_EN_SHIFT                                  5
#define    ACI_DSP_INT_COMP2INT_EN_MASK                                   0x00000020
#define    ACI_DSP_INT_COMP1INT_EN_SHIFT                                  4
#define    ACI_DSP_INT_COMP1INT_EN_MASK                                   0x00000010
#define    ACI_DSP_INT_INV_COMP2INT_STS_SHIFT                             2
#define    ACI_DSP_INT_INV_COMP2INT_STS_MASK                              0x00000004
#define    ACI_DSP_INT_COMP2INT_STS_SHIFT                                 1
#define    ACI_DSP_INT_COMP2INT_STS_MASK                                  0x00000002
#define    ACI_DSP_INT_COMP1INT_STS_SHIFT                                 0
#define    ACI_DSP_INT_COMP1INT_STS_MASK                                  0x00000001

#define ACI_DSP_ACI_CTRL_OFFSET                                           0x00000068
#define ACI_DSP_ACI_CTRL_TYPE                                             UInt32
#define ACI_DSP_ACI_CTRL_RESERVED_MASK                                    0xFFFFFFCE
#define    ACI_DSP_ACI_CTRL_SW_MIC_DATAB_SHIFT                            5
#define    ACI_DSP_ACI_CTRL_SW_MIC_DATAB_MASK                             0x00000020
#define    ACI_DSP_ACI_CTRL_SW_ACI_SHIFT                                  4
#define    ACI_DSP_ACI_CTRL_SW_ACI_MASK                                   0x00000010
#define    ACI_DSP_ACI_CTRL_ACI_BIAS_PWRDN_SHIFT                          0
#define    ACI_DSP_ACI_CTRL_ACI_BIAS_PWRDN_MASK                           0x00000001

#define ACI_DSP_ADC_CTRL_OFFSET                                           0x0000006A
#define ACI_DSP_ADC_CTRL_TYPE                                             UInt32
#define ACI_DSP_ADC_CTRL_RESERVED_MASK                                    0xFFFFFF00
#define    ACI_DSP_ADC_CTRL_AUDIORX_VREF_PWRUP_SHIFT                      7
#define    ACI_DSP_ADC_CTRL_AUDIORX_VREF_PWRUP_MASK                       0x00000080
#define    ACI_DSP_ADC_CTRL_AUDIORX_BIAS_PWRUP_SHIFT                      6
#define    ACI_DSP_ADC_CTRL_AUDIORX_BIAS_PWRUP_MASK                       0x00000040
#define    ACI_DSP_ADC_CTRL_ADC_FS_BIG_SMALL_B_SHIFT                      5
#define    ACI_DSP_ADC_CTRL_ADC_FS_BIG_SMALL_B_MASK                       0x00000020
#define    ACI_DSP_ADC_CTRL_ADC_STEPSIZE_SET_SHIFT                        4
#define    ACI_DSP_ADC_CTRL_ADC_STEPSIZE_SET_MASK                         0x00000010
#define    ACI_DSP_ADC_CTRL_ADC_STEPSIZE_SHIFT                            1
#define    ACI_DSP_ADC_CTRL_ADC_STEPSIZE_MASK                             0x0000000E
#define    ACI_DSP_ADC_CTRL_ADC_ADAPTIVE_EN_SHIFT                         0
#define    ACI_DSP_ADC_CTRL_ADC_ADAPTIVE_EN_MASK                          0x00000001

#define ACI_DSP_SLEEP_CTRL_OFFSET                                         0x0000006C
#define ACI_DSP_SLEEP_CTRL_TYPE                                           UInt32
#define ACI_DSP_SLEEP_CTRL_RESERVED_MASK                                  0xFFFFFCCE
#define    ACI_DSP_SLEEP_CTRL_CHARGEUP_TIME_SHIFT                         8
#define    ACI_DSP_SLEEP_CTRL_CHARGEUP_TIME_MASK                          0x00000300
#define    ACI_DSP_SLEEP_CTRL_RELEASE_TIME_SHIFT                          4
#define    ACI_DSP_SLEEP_CTRL_RELEASE_TIME_MASK                           0x00000030
#define    ACI_DSP_SLEEP_CTRL_WEAK_SLEEP_EN_SHIFT                         0
#define    ACI_DSP_SLEEP_CTRL_WEAK_SLEEP_EN_MASK                          0x00000001

#define ACI_DSP_ID_OFFSET                                                 0x00000200
#define ACI_DSP_ID_TYPE                                                   UInt32
#define ACI_DSP_ID_RESERVED_MASK                                          0xFFFF0000
#define    ACI_DSP_ID_ACIID_SHIFT                                         0
#define    ACI_DSP_ID_ACIID_MASK                                          0x0000FFFF

#define ACI_DSP_COMR_OFFSET                                               0x00000202
#define ACI_DSP_COMR_TYPE                                                 UInt32
#define ACI_DSP_COMR_RESERVED_MASK                                        0xFFFFFE00
#define    ACI_DSP_COMR_BUSHT_SHIFT                                       8
#define    ACI_DSP_COMR_BUSHT_MASK                                        0x00000100
#define       ACI_DSP_COMR_BUSHT_CMD_BUSHOLDTIME_2MS                      0x00000000
#define       ACI_DSP_COMR_BUSHT_CMD_BUSHOLDTIME_10MS                     0x00000001
#define    ACI_DSP_COMR_DBIDOC_SHIFT                                      7
#define    ACI_DSP_COMR_DBIDOC_MASK                                       0x00000080
#define       ACI_DSP_COMR_DBIDOC_CMD_INUSE                               0x00000000
#define       ACI_DSP_COMR_DBIDOC_CMD_DISABLED                            0x00000001
#define    ACI_DSP_COMR_DBIDOS_SHIFT                                      6
#define    ACI_DSP_COMR_DBIDOS_MASK                                       0x00000040
#define       ACI_DSP_COMR_DBIDOS_CMD_TXDOWN                              0x00000000
#define       ACI_DSP_COMR_DBIDOS_CMD_TXUP                                0x00000001
#define    ACI_DSP_COMR_DBIDIC_SHIFT                                      5
#define    ACI_DSP_COMR_DBIDIC_MASK                                       0x00000020
#define       ACI_DSP_COMR_DBIDIC_CMD_NORMAL                              0x00000000
#define       ACI_DSP_COMR_DBIDIC_CMD_DATAREG                             0x00000001
#define    ACI_DSP_COMR_DBIDIS_SHIFT                                      4
#define    ACI_DSP_COMR_DBIDIS_MASK                                       0x00000010
#define       ACI_DSP_COMR_DBIDIS_CMD_SPEEDREG                            0x00000000
#define       ACI_DSP_COMR_DBIDIS_CMD_RXMSB                               0x00000001
#define    ACI_DSP_COMR_DBICON_SHIFT                                      3
#define    ACI_DSP_COMR_DBICON_MASK                                       0x00000008
#define       ACI_DSP_COMR_DBICON_CMD_DIRECT                              0x00000000
#define       ACI_DSP_COMR_DBICON_CMD_VOTE                                0x00000001
#define    ACI_DSP_COMR_DBICMD_SHIFT                                      0
#define    ACI_DSP_COMR_DBICMD_MASK                                       0x00000007
#define       ACI_DSP_COMR_DBICMD_CMD_NOP                                 0x00000000
#define       ACI_DSP_COMR_DBICMD_CMD_RESET                               0x00000001
#define       ACI_DSP_COMR_DBICMD_CMD_SEND                                0x00000002
#define       ACI_DSP_COMR_DBICMD_CMD_RECV                                0x00000003
#define       ACI_DSP_COMR_DBICMD_CMD_RECV_LEARN                          0x00000004

#define ACI_DSP_TXDAR_OFFSET                                              0x00000204
#define ACI_DSP_TXDAR_TYPE                                                UInt32
#define ACI_DSP_TXDAR_RESERVED_MASK                                       0xFFFFFF00
#define    ACI_DSP_TXDAR_ACITXDAR_SHIFT                                   0
#define    ACI_DSP_TXDAR_ACITXDAR_MASK                                    0x000000FF

#define ACI_DSP_RXDAR_OFFSET                                              0x00000206
#define ACI_DSP_RXDAR_TYPE                                                UInt32
#define ACI_DSP_RXDAR_RESERVED_MASK                                       0xFFFFFF00
#define    ACI_DSP_RXDAR_ACIRXDAR_SHIFT                                   0
#define    ACI_DSP_RXDAR_ACIRXDAR_MASK                                    0x000000FF

#define ACI_DSP_IMR_OFFSET                                                0x00000208
#define ACI_DSP_IMR_TYPE                                                  UInt32
#define ACI_DSP_IMR_RESERVED_MASK                                         0xFFFFFE00
#define    ACI_DSP_IMR_ACIIMR_SHIFT                                       0
#define    ACI_DSP_IMR_ACIIMR_MASK                                        0x000001FF
#define       ACI_DSP_IMR_ACIIMR_CMD_ACI_INT                              0x00000001
#define       ACI_DSP_IMR_ACIIMR_CMD_DATA_RECVED                          0x00000002
#define       ACI_DSP_IMR_ACIIMR_CMD_RDY_SEND_SENT                        0x00000004
#define       ACI_DSP_IMR_ACIIMR_CMD_SPD_SET                              0x00000008
#define       ACI_DSP_IMR_ACIIMR_CMD_IF_ERR                               0x00000010
#define       ACI_DSP_IMR_ACIIMR_CMD_FRAMING                              0x00000020
#define       ACI_DSP_IMR_ACIIMR_CMD_RESET_LEARN                          0x00000040
#define       ACI_DSP_IMR_ACIIMR_CMD_BUS_COLLISION                        0x00000080
#define       ACI_DSP_IMR_ACIIMR_CMD_RET_IDLE                             0x00000100

#define ACI_DSP_IIDR_OFFSET                                               0x0000020A
#define ACI_DSP_IIDR_TYPE                                                 UInt32
#define ACI_DSP_IIDR_RESERVED_MASK                                        0xFFFFFE00
#define    ACI_DSP_IIDR_ACIIIDR_SHIFT                                     0
#define    ACI_DSP_IIDR_ACIIIDR_MASK                                      0x000001FF

#define ACI_DSP_SPLR_OFFSET                                               0x0000020C
#define ACI_DSP_SPLR_TYPE                                                 UInt32
#define ACI_DSP_SPLR_RESERVED_MASK                                        0xFFFFFFC0
#define    ACI_DSP_SPLR_ACISPLR_SHIFT                                     0
#define    ACI_DSP_SPLR_ACISPLR_MASK                                      0x0000003F

#define ACI_DSP_ITR_OFFSET                                                0x0000020E
#define ACI_DSP_ITR_TYPE                                                  UInt32
#define ACI_DSP_ITR_RESERVED_MASK                                         0xFFFFFFFC
#define    ACI_DSP_ITR_ACIITR_SHIFT                                       0
#define    ACI_DSP_ITR_ACIITR_MASK                                        0x00000003

#define ACI_DSP_DEBUG_OFFSET                                              0x00000210
#define ACI_DSP_DEBUG_TYPE                                                UInt32
#define ACI_DSP_DEBUG_RESERVED_MASK                                       0xFFFFFFF0
#define    ACI_DSP_DEBUG_ACIDEBUG_SHIFT                                   2
#define    ACI_DSP_DEBUG_ACIDEBUG_MASK                                    0x0000000C
#define    ACI_DSP_DEBUG_ACI_RTX_SHORT_SHIFT                              1
#define    ACI_DSP_DEBUG_ACI_RTX_SHORT_MASK                               0x00000002
#define    ACI_DSP_DEBUG_ACIDEBUG_CLR_SHIFT                               0
#define    ACI_DSP_DEBUG_ACIDEBUG_CLR_MASK                                0x00000001

#endif /* __BRCM_RDB_ACI_H__ */


