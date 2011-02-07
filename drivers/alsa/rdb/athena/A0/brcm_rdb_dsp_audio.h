/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2009  Broadcom Corporation                                                        */
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
/************************************************************************************************/

#ifndef __BRCM_RDB_DSP_AUDIO_H__
#define __BRCM_RDB_DSP_AUDIO_H__

#define DSP_AUDIO_VINPATH_CTRL_R_OFFSET                                   0x00000A82
#define DSP_AUDIO_VINPATH_CTRL_R_TYPE                                     UInt16
#define DSP_AUDIO_VINPATH_CTRL_R_RESERVED_MASK                            0x00000000
#define    DSP_AUDIO_VINPATH_CTRL_R_VIN_IIR_OUTSEL_SHIFT                  13
#define    DSP_AUDIO_VINPATH_CTRL_R_VIN_IIR_OUTSEL_MASK                   0x0000E000
#define    DSP_AUDIO_VINPATH_CTRL_R_VIN_CIC_OUTSEL_SHIFT                  11
#define    DSP_AUDIO_VINPATH_CTRL_R_VIN_CIC_OUTSEL_MASK                   0x00001800
#define    DSP_AUDIO_VINPATH_CTRL_R_VIN_CIC_FINESCALE_SHIFT               3
#define    DSP_AUDIO_VINPATH_CTRL_R_VIN_CIC_FINESCALE_MASK                0x000007F8
#define    DSP_AUDIO_VINPATH_CTRL_R_VIN_INPUTSEL_SHIFT                    1
#define    DSP_AUDIO_VINPATH_CTRL_R_VIN_INPUTSEL_MASK                     0x00000006
#define    DSP_AUDIO_VINPATH_CTRL_R_VIN_ENABLE_SHIFT                      0
#define    DSP_AUDIO_VINPATH_CTRL_R_VIN_ENABLE_MASK                       0x00000001

#define DSP_AUDIO_AUXCOMP_R_OFFSET                                        0x00000A86
#define DSP_AUDIO_AUXCOMP_R_TYPE                                          UInt16
#define DSP_AUDIO_AUXCOMP_R_RESERVED_MASK                                 0x00003FFE
#define    DSP_AUDIO_AUXCOMP_R_AUXCOMP_SHIFT                              14
#define    DSP_AUDIO_AUXCOMP_R_AUXCOMP_MASK                               0x0000C000
#define    DSP_AUDIO_AUXCOMP_R_AUXCOMP_EN_SHIFT                           0
#define    DSP_AUDIO_AUXCOMP_R_AUXCOMP_EN_MASK                            0x00000001

#define DSP_AUDIO_AUDIOINPATH_CTRL_R_OFFSET                               0x00000A88
#define DSP_AUDIO_AUDIOINPATH_CTRL_R_TYPE                                 UInt16
#define DSP_AUDIO_AUDIOINPATH_CTRL_R_RESERVED_MASK                        0x00000000
#define    DSP_AUDIO_AUDIOINPATH_CTRL_R_AIN_IIR_OUTSEL_SHIFT              13
#define    DSP_AUDIO_AUDIOINPATH_CTRL_R_AIN_IIR_OUTSEL_MASK               0x0000E000
#define    DSP_AUDIO_AUDIOINPATH_CTRL_R_AIN_CIC_OUTSEL_SHIFT              11
#define    DSP_AUDIO_AUDIOINPATH_CTRL_R_AIN_CIC_OUTSEL_MASK               0x00001800
#define    DSP_AUDIO_AUDIOINPATH_CTRL_R_AIN_CIC_FINESCALE_SHIFT           3
#define    DSP_AUDIO_AUDIOINPATH_CTRL_R_AIN_CIC_FINESCALE_MASK            0x000007F8
#define    DSP_AUDIO_AUDIOINPATH_CTRL_R_AIN_INPUTSEL_SHIFT                1
#define    DSP_AUDIO_AUDIOINPATH_CTRL_R_AIN_INPUTSEL_MASK                 0x00000006
#define    DSP_AUDIO_AUDIOINPATH_CTRL_R_AIN_ENABLE_SHIFT                  0
#define    DSP_AUDIO_AUDIOINPATH_CTRL_R_AIN_ENABLE_MASK                   0x00000001

#define DSP_AUDIO_AUDIOLOOPBACK_CTRL_R_OFFSET                             0x00000A92
#define DSP_AUDIO_AUDIOLOOPBACK_CTRL_R_TYPE                               UInt16
#define DSP_AUDIO_AUDIOLOOPBACK_CTRL_R_RESERVED_MASK                      0x00001F00
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_R_RXANA1_EN_SHIFT                 15
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_R_RXANA1_EN_MASK                  0x00008000
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_R_AUD_IN_DMA_EN_SHIFT             14
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_R_AUD_IN_DMA_EN_MASK              0x00004000
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_R_AUDIOIN_FIFO_CLR_SHIFT          13
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_R_AUDIOIN_FIFO_CLR_MASK           0x00002000
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_R_AUDIOIN_BITMODE_DAC4_SHIFT      6
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_R_AUDIOIN_BITMODE_DAC4_MASK       0x000000C0
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_R_AUDIOIN_BITMODE_DAC3_SHIFT      4
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_R_AUDIOIN_BITMODE_DAC3_MASK       0x00000030
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_R_AUDIOIN_BITMODE_DAC2_SHIFT      2
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_R_AUDIOIN_BITMODE_DAC2_MASK       0x0000000C
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_R_AUDIOIN_BITMODE_DAC1_SHIFT      0
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_R_AUDIOIN_BITMODE_DAC1_MASK       0x00000003

#define DSP_AUDIO_AUDIOINCM_ADDR_R_OFFSET                                 0x00000A8E
#define DSP_AUDIO_AUDIOINCM_ADDR_R_TYPE                                   UInt16
#define DSP_AUDIO_AUDIOINCM_ADDR_R_RESERVED_MASK                          0x0000FF00
#define    DSP_AUDIO_AUDIOINCM_ADDR_R_AUDIOINCM_ADDR_SHIFT                0
#define    DSP_AUDIO_AUDIOINCM_ADDR_R_AUDIOINCM_ADDR_MASK                 0x000000FF

#define DSP_AUDIO_AUDIOINCM_DATA_R_OFFSET                                 0x00000A90
#define DSP_AUDIO_AUDIOINCM_DATA_R_TYPE                                   UInt16
#define DSP_AUDIO_AUDIOINCM_DATA_R_RESERVED_MASK                          0x00008000
#define    DSP_AUDIO_AUDIOINCM_DATA_R_AUDIOINCM_DATA_SHIFT                0
#define    DSP_AUDIO_AUDIOINCM_DATA_R_AUDIOINCM_DATA_MASK                 0x00007FFF

#define DSP_AUDIO_AUDIR2_R_OFFSET                                         0x00000F40
#define DSP_AUDIO_AUDIR2_R_TYPE                                           UInt16
#define DSP_AUDIO_AUDIR2_R_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_AUDIR2_R_AUDIR2_SHIFT                                0
#define    DSP_AUDIO_AUDIR2_R_AUDIR2_MASK                                 0x0000FFFF

#define DSP_AUDIO_AUDIOINFIFO_STATUS_R_OFFSET                             0x00000F42
#define DSP_AUDIO_AUDIOINFIFO_STATUS_R_TYPE                               UInt16
#define DSP_AUDIO_AUDIOINFIFO_STATUS_R_RESERVED_MASK                      0x00000000
#define    DSP_AUDIO_AUDIOINFIFO_STATUS_R_AUDIOINFIFO_STATUS_SHIFT        0
#define    DSP_AUDIO_AUDIOINFIFO_STATUS_R_AUDIOINFIFO_STATUS_MASK         0x0000FFFF

#define DSP_AUDIO_MIXER_INPUT_SEL_R_OFFSET                                0x00000FF0
#define DSP_AUDIO_MIXER_INPUT_SEL_R_TYPE                                  UInt16
#define DSP_AUDIO_MIXER_INPUT_SEL_R_RESERVED_MASK                         0x00003FE0
#define    DSP_AUDIO_MIXER_INPUT_SEL_R_PATHSEL_SHIFT                      14
#define    DSP_AUDIO_MIXER_INPUT_SEL_R_PATHSEL_MASK                       0x0000C000
#define    DSP_AUDIO_MIXER_INPUT_SEL_R_INPUT_TO_MIX_SHIFT                 0
#define    DSP_AUDIO_MIXER_INPUT_SEL_R_INPUT_TO_MIX_MASK                  0x0000001F

#define DSP_AUDIO_MIXER_GAIN_CHSEL_R_OFFSET                               0x00000FF2
#define DSP_AUDIO_MIXER_GAIN_CHSEL_R_TYPE                                 UInt16
#define DSP_AUDIO_MIXER_GAIN_CHSEL_R_RESERVED_MASK                        0x0000FFFC
#define    DSP_AUDIO_MIXER_GAIN_CHSEL_R_MIXER_GAIN_CHSEL_SHIFT            0
#define    DSP_AUDIO_MIXER_GAIN_CHSEL_R_MIXER_GAIN_CHSEL_MASK             0x00000003

#define DSP_AUDIO_MIXER_GAIN_ADJUST_R_OFFSET                              0x00000FF4
#define DSP_AUDIO_MIXER_GAIN_ADJUST_R_TYPE                                UInt16
#define DSP_AUDIO_MIXER_GAIN_ADJUST_R_RESERVED_MASK                       0x00000000
#define    DSP_AUDIO_MIXER_GAIN_ADJUST_R_MIXER_GAIN_ADJUST_SHIFT          0
#define    DSP_AUDIO_MIXER_GAIN_ADJUST_R_MIXER_GAIN_ADJUST_MASK           0x0000FFFF

#define DSP_AUDIO_BIQUAD_CFG_R_OFFSET                                     0x00000FF6
#define DSP_AUDIO_BIQUAD_CFG_R_TYPE                                       UInt16
#define DSP_AUDIO_BIQUAD_CFG_R_RESERVED_MASK                              0x0000200C
#define    DSP_AUDIO_BIQUAD_CFG_R_CHSEL_SHIFT                             14
#define    DSP_AUDIO_BIQUAD_CFG_R_CHSEL_MASK                              0x0000C000
#define    DSP_AUDIO_BIQUAD_CFG_R_MPMBIQUAD_OUTSEL_SHIFT                  8
#define    DSP_AUDIO_BIQUAD_CFG_R_MPMBIQUAD_OUTSEL_MASK                   0x00001F00
#define    DSP_AUDIO_BIQUAD_CFG_R_MPMBIQUAD_NUM_SHIFT                     4
#define    DSP_AUDIO_BIQUAD_CFG_R_MPMBIQUAD_NUM_MASK                      0x000000F0
#define    DSP_AUDIO_BIQUAD_CFG_R_BIQUAD_CLIP_CTRL_SHIFT                  0
#define    DSP_AUDIO_BIQUAD_CFG_R_BIQUAD_CLIP_CTRL_MASK                   0x00000003

#define DSP_AUDIO_MPM_FORCEOFF_R_OFFSET                                   0x00000FF8
#define DSP_AUDIO_MPM_FORCEOFF_R_TYPE                                     UInt16
#define DSP_AUDIO_MPM_FORCEOFF_R_RESERVED_MASK                            0x0000FFFE
#define    DSP_AUDIO_MPM_FORCEOFF_R_MPM_FORCEOFF_SHIFT                    0
#define    DSP_AUDIO_MPM_FORCEOFF_R_MPM_FORCEOFF_MASK                     0x00000001

#define DSP_AUDIO_MPMBIQUAD_COEF_ADDR_R_OFFSET                            0x00000F9A
#define DSP_AUDIO_MPMBIQUAD_COEF_ADDR_R_TYPE                              UInt16
#define DSP_AUDIO_MPMBIQUAD_COEF_ADDR_R_RESERVED_MASK                     0x0000FF00
#define    DSP_AUDIO_MPMBIQUAD_COEF_ADDR_R_BIQUAD_CO_ADDR_SHIFT           0
#define    DSP_AUDIO_MPMBIQUAD_COEF_ADDR_R_BIQUAD_CO_ADDR_MASK            0x000000FF

#define DSP_AUDIO_MPMBIQUAD_COEF_WDATAH_R_OFFSET                          0x00000F9C
#define DSP_AUDIO_MPMBIQUAD_COEF_WDATAH_R_TYPE                            UInt16
#define DSP_AUDIO_MPMBIQUAD_COEF_WDATAH_R_RESERVED_MASK                   0x00000000
#define    DSP_AUDIO_MPMBIQUAD_COEF_WDATAH_R_MPMBIQUAD_COEF_WDATA_BIT21_6_SHIFT 0
#define    DSP_AUDIO_MPMBIQUAD_COEF_WDATAH_R_MPMBIQUAD_COEF_WDATA_BIT21_6_MASK 0x0000FFFF

#define DSP_AUDIO_MPMBIQUAD_COEF_WDATAL_R_OFFSET                          0x00000F9E
#define DSP_AUDIO_MPMBIQUAD_COEF_WDATAL_R_TYPE                            UInt16
#define DSP_AUDIO_MPMBIQUAD_COEF_WDATAL_R_RESERVED_MASK                   0x0000FFC0
#define    DSP_AUDIO_MPMBIQUAD_COEF_WDATAL_R_MPMBIQUAD_COEF_WDATA_BIT5_0_SHIFT 0
#define    DSP_AUDIO_MPMBIQUAD_COEF_WDATAL_R_MPMBIQUAD_COEF_WDATA_BIT5_0_MASK 0x0000003F

#define DSP_AUDIO_POLYAUDMOD_R_OFFSET                                     0x00000F60
#define DSP_AUDIO_POLYAUDMOD_R_TYPE                                       UInt16
#define DSP_AUDIO_POLYAUDMOD_R_RESERVED_MASK                              0x0000001A
#define    DSP_AUDIO_POLYAUDMOD_R_POLY_18BIT_MODE_SHIFT                   15
#define    DSP_AUDIO_POLYAUDMOD_R_POLY_18BIT_MODE_MASK                    0x00008000
#define    DSP_AUDIO_POLYAUDMOD_R_POLY_OUT_DMA_EN_SHIFT                   14
#define    DSP_AUDIO_POLYAUDMOD_R_POLY_OUT_DMA_EN_MASK                    0x00004000
#define    DSP_AUDIO_POLYAUDMOD_R_PSAMPRATE_SHIFT                         10
#define    DSP_AUDIO_POLYAUDMOD_R_PSAMPRATE_MASK                          0x00003C00
#define    DSP_AUDIO_POLYAUDMOD_R_PRATEADP_SHIFT                          8
#define    DSP_AUDIO_POLYAUDMOD_R_PRATEADP_MASK                           0x00000300
#define    DSP_AUDIO_POLYAUDMOD_R_PHBIFMOD_SHIFT                          5
#define    DSP_AUDIO_POLYAUDMOD_R_PHBIFMOD_MASK                           0x000000E0
#define    DSP_AUDIO_POLYAUDMOD_R_PCOMPMOD_SHIFT                          2
#define    DSP_AUDIO_POLYAUDMOD_R_PCOMPMOD_MASK                           0x00000004
#define    DSP_AUDIO_POLYAUDMOD_R_PLYEN_SHIFT                             0
#define    DSP_AUDIO_POLYAUDMOD_R_PLYEN_MASK                              0x00000001

#define DSP_AUDIO_PFIFOCTRL_R_OFFSET                                      0x00000F62
#define DSP_AUDIO_PFIFOCTRL_R_TYPE                                        UInt16
#define DSP_AUDIO_PFIFOCTRL_R_RESERVED_MASK                               0x00000000
#define    DSP_AUDIO_PFIFOCTRL_R_POFIFOCRL0_SHIFT                         15
#define    DSP_AUDIO_PFIFOCTRL_R_POFIFOCRL0_MASK                          0x00008000
#define    DSP_AUDIO_PFIFOCTRL_R_POFIFOTHRES0_SHIFT                       8
#define    DSP_AUDIO_PFIFOCTRL_R_POFIFOTHRES0_MASK                        0x00007F00
#define    DSP_AUDIO_PFIFOCTRL_R_PIFIFOCRL1_SHIFT                         7
#define    DSP_AUDIO_PFIFOCTRL_R_PIFIFOCRL1_MASK                          0x00000080
#define    DSP_AUDIO_PFIFOCTRL_R_PIFIFOTHRES1_SHIFT                       0
#define    DSP_AUDIO_PFIFOCTRL_R_PIFIFOTHRES1_MASK                        0x0000007F

#define DSP_AUDIO_PIFIFOST_R_OFFSET                                       0x00000F64
#define DSP_AUDIO_PIFIFOST_R_TYPE                                         UInt16
#define DSP_AUDIO_PIFIFOST_R_RESERVED_MASK                                0x0000F800
#define    DSP_AUDIO_PIFIFOST_R_PIFIFOOVF_SHIFT                           10
#define    DSP_AUDIO_PIFIFOST_R_PIFIFOOVF_MASK                            0x00000400
#define    DSP_AUDIO_PIFIFOST_R_PIFIFOUDF_SHIFT                           9
#define    DSP_AUDIO_PIFIFOST_R_PIFIFOUDF_MASK                            0x00000200
#define    DSP_AUDIO_PIFIFOST_R_PIFIFOTHMET_SHIFT                         8
#define    DSP_AUDIO_PIFIFOST_R_PIFIFOTHMET_MASK                          0x00000100
#define    DSP_AUDIO_PIFIFOST_R_PIFIFOEMTRYCUNT_SHIFT                     0
#define    DSP_AUDIO_PIFIFOST_R_PIFIFOEMTRYCUNT_MASK                      0x000000FF

#define DSP_AUDIO_PLRCH_R_OFFSET                                          0x00000F66
#define DSP_AUDIO_PLRCH_R_TYPE                                            UInt16
#define DSP_AUDIO_PLRCH_R_RESERVED_MASK                                   0x0000FFF0
#define    DSP_AUDIO_PLRCH_R_PLCHMOD_SHIFT                                2
#define    DSP_AUDIO_PLRCH_R_PLCHMOD_MASK                                 0x0000000C
#define    DSP_AUDIO_PLRCH_R_PRCHMOD_SHIFT                                0
#define    DSP_AUDIO_PLRCH_R_PRCHMOD_MASK                                 0x00000003

#define DSP_AUDIO_PEQPATHOFST0_R_OFFSET                                   0x00000F68
#define DSP_AUDIO_PEQPATHOFST0_R_TYPE                                     UInt16
#define DSP_AUDIO_PEQPATHOFST0_R_RESERVED_MASK                            0x00000000
#define    DSP_AUDIO_PEQPATHOFST0_R_PPATH2DELAY_SHIFT                     14
#define    DSP_AUDIO_PEQPATHOFST0_R_PPATH2DELAY_MASK                      0x0000C000
#define    DSP_AUDIO_PEQPATHOFST0_R_PPATH1DELAY_SHIFT                     7
#define    DSP_AUDIO_PEQPATHOFST0_R_PPATH1DELAY_MASK                      0x00003F80
#define    DSP_AUDIO_PEQPATHOFST0_R_PPATH0DELAY_SHIFT                     0
#define    DSP_AUDIO_PEQPATHOFST0_R_PPATH0DELAY_MASK                      0x0000007F

#define DSP_AUDIO_PEQPATHOFST1_R_OFFSET                                   0x00000F6A
#define DSP_AUDIO_PEQPATHOFST1_R_TYPE                                     UInt16
#define DSP_AUDIO_PEQPATHOFST1_R_RESERVED_MASK                            0x00000000
#define    DSP_AUDIO_PEQPATHOFST1_R_PPATH4DELAY_SHIFT                     12
#define    DSP_AUDIO_PEQPATHOFST1_R_PPATH4DELAY_MASK                      0x0000F000
#define    DSP_AUDIO_PEQPATHOFST1_R_PPATH3DELAY_SHIFT                     5
#define    DSP_AUDIO_PEQPATHOFST1_R_PPATH3DELAY_MASK                      0x00000FE0
#define    DSP_AUDIO_PEQPATHOFST1_R_PPATH2DELAY_SHIFT                     0
#define    DSP_AUDIO_PEQPATHOFST1_R_PPATH2DELAY_MASK                      0x0000001F

#define DSP_AUDIO_PEQPATHOFST2_R_OFFSET                                   0x00000F6C
#define DSP_AUDIO_PEQPATHOFST2_R_TYPE                                     UInt16
#define DSP_AUDIO_PEQPATHOFST2_R_RESERVED_MASK                            0x0000FC00
#define    DSP_AUDIO_PEQPATHOFST2_R_PPATH5DELAY_SHIFT                     3
#define    DSP_AUDIO_PEQPATHOFST2_R_PPATH5DELAY_MASK                      0x000003F8
#define    DSP_AUDIO_PEQPATHOFST2_R_PPATH4DELAY_SHIFT                     0
#define    DSP_AUDIO_PEQPATHOFST2_R_PPATH4DELAY_MASK                      0x00000007

#define DSP_AUDIO_PLSLOPGAIN_R_OFFSET                                     0x00000F70
#define DSP_AUDIO_PLSLOPGAIN_R_TYPE                                       UInt16
#define DSP_AUDIO_PLSLOPGAIN_R_RESERVED_MASK                              0x00000600
#define    DSP_AUDIO_PLSLOPGAIN_R_PLSLOPGAINEN_SHIFT                      15
#define    DSP_AUDIO_PLSLOPGAIN_R_PLSLOPGAINEN_MASK                       0x00008000
#define    DSP_AUDIO_PLSLOPGAIN_R_PLSLOPMOD_SHIFT                         11
#define    DSP_AUDIO_PLSLOPGAIN_R_PLSLOPMOD_MASK                          0x00007800
#define    DSP_AUDIO_PLSLOPGAIN_R_PLTARGETGAIN_SHIFT                      0
#define    DSP_AUDIO_PLSLOPGAIN_R_PLTARGETGAIN_MASK                       0x000001FF

#define DSP_AUDIO_PRSLOPGAIN_R_OFFSET                                     0x00000F72
#define DSP_AUDIO_PRSLOPGAIN_R_TYPE                                       UInt16
#define DSP_AUDIO_PRSLOPGAIN_R_RESERVED_MASK                              0x00000600
#define    DSP_AUDIO_PRSLOPGAIN_R_PRSLOPGAINEN_SHIFT                      15
#define    DSP_AUDIO_PRSLOPGAIN_R_PRSLOPGAINEN_MASK                       0x00008000
#define    DSP_AUDIO_PRSLOPGAIN_R_PRSLOPMOD_SHIFT                         11
#define    DSP_AUDIO_PRSLOPGAIN_R_PRSLOPMOD_MASK                          0x00007800
#define    DSP_AUDIO_PRSLOPGAIN_R_PRTARGETGAIN_SHIFT                      0
#define    DSP_AUDIO_PRSLOPGAIN_R_PRTARGETGAIN_MASK                       0x000001FF

#define DSP_AUDIO_PEQPATHGAIN1_R_OFFSET                                   0x00000F78
#define DSP_AUDIO_PEQPATHGAIN1_R_TYPE                                     UInt16
#define DSP_AUDIO_PEQPATHGAIN1_R_RESERVED_MASK                            0x00000600
#define    DSP_AUDIO_PEQPATHGAIN1_R_PEQPATHGAIN1EN_SHIFT                  15
#define    DSP_AUDIO_PEQPATHGAIN1_R_PEQPATHGAIN1EN_MASK                   0x00008000
#define    DSP_AUDIO_PEQPATHGAIN1_R_EQGAINSLOP_SHIFT                      11
#define    DSP_AUDIO_PEQPATHGAIN1_R_EQGAINSLOP_MASK                       0x00007800
#define    DSP_AUDIO_PEQPATHGAIN1_R_PEQPATHGAIN1_SHIFT                    0
#define    DSP_AUDIO_PEQPATHGAIN1_R_PEQPATHGAIN1_MASK                     0x000001FF

#define DSP_AUDIO_PEQPATHGAIN2_R_OFFSET                                   0x00000F7A
#define DSP_AUDIO_PEQPATHGAIN2_R_TYPE                                     UInt16
#define DSP_AUDIO_PEQPATHGAIN2_R_RESERVED_MASK                            0x0000FE00
#define    DSP_AUDIO_PEQPATHGAIN2_R_PEQPATHGAIN2_SHIFT                    0
#define    DSP_AUDIO_PEQPATHGAIN2_R_PEQPATHGAIN2_MASK                     0x000001FF

#define DSP_AUDIO_PEQPATHGAIN3_R_OFFSET                                   0x00000F7C
#define DSP_AUDIO_PEQPATHGAIN3_R_TYPE                                     UInt16
#define DSP_AUDIO_PEQPATHGAIN3_R_RESERVED_MASK                            0x0000FE00
#define    DSP_AUDIO_PEQPATHGAIN3_R_PEQPATHGAIN3_SHIFT                    0
#define    DSP_AUDIO_PEQPATHGAIN3_R_PEQPATHGAIN3_MASK                     0x000001FF

#define DSP_AUDIO_PEQPATHGAIN4_R_OFFSET                                   0x00000F7E
#define DSP_AUDIO_PEQPATHGAIN4_R_TYPE                                     UInt16
#define DSP_AUDIO_PEQPATHGAIN4_R_RESERVED_MASK                            0x0000FE00
#define    DSP_AUDIO_PEQPATHGAIN4_R_PEQPATHGAIN4_SHIFT                    0
#define    DSP_AUDIO_PEQPATHGAIN4_R_PEQPATHGAIN4_MASK                     0x000001FF

#define DSP_AUDIO_PEQPATHGAIN5_R_OFFSET                                   0x00000F6E
#define DSP_AUDIO_PEQPATHGAIN5_R_TYPE                                     UInt16
#define DSP_AUDIO_PEQPATHGAIN5_R_RESERVED_MASK                            0x0000FE00
#define    DSP_AUDIO_PEQPATHGAIN5_R_PEQPATHGAIN5_SHIFT                    0
#define    DSP_AUDIO_PEQPATHGAIN5_R_PEQPATHGAIN5_MASK                     0x000001FF

#define DSP_AUDIO_PIFIFODATA0_R_OFFSET                                    0x00000600
#define DSP_AUDIO_PIFIFODATA0_R_TYPE                                      UInt32
#define DSP_AUDIO_PIFIFODATA0_R_RESERVED_MASK                             0x00000000
#define    DSP_AUDIO_PIFIFODATA0_R_BTW_DATAR_SHIFT                        16
#define    DSP_AUDIO_PIFIFODATA0_R_BTW_DATAR_MASK                         0xFFFF0000
#define    DSP_AUDIO_PIFIFODATA0_R_BTW_DATAL_SHIFT                        4
#define    DSP_AUDIO_PIFIFODATA0_R_BTW_DATAL_MASK                         0x0000FFF0
#define    DSP_AUDIO_PIFIFODATA0_R_PIFIFODATAR_SHIFT                      2
#define    DSP_AUDIO_PIFIFODATA0_R_PIFIFODATAR_MASK                       0x0000000C
#define    DSP_AUDIO_PIFIFODATA0_R_PIFIFODATAL_SHIFT                      0
#define    DSP_AUDIO_PIFIFODATA0_R_PIFIFODATAL_MASK                       0x00000003

#define DSP_AUDIO_PIFIFODATA1_R_OFFSET                                    0x00000604
#define DSP_AUDIO_PIFIFODATA1_R_TYPE                                      UInt32
#define DSP_AUDIO_PIFIFODATA1_R_RESERVED_MASK                             0x00000000
#define    DSP_AUDIO_PIFIFODATA1_R_PIFIFORDATA_SHIFT                      16
#define    DSP_AUDIO_PIFIFODATA1_R_PIFIFORDATA_MASK                       0xFFFF0000
#define    DSP_AUDIO_PIFIFODATA1_R_PIFIFOLDATA_SHIFT                      0
#define    DSP_AUDIO_PIFIFODATA1_R_PIFIFOLDATA_MASK                       0x0000FFFF

#define DSP_AUDIO_PEQCOFADD_R_OFFSET                                      0x00000F88
#define DSP_AUDIO_PEQCOFADD_R_TYPE                                        UInt16
#define DSP_AUDIO_PEQCOFADD_R_RESERVED_MASK                               0x0000FF00
#define    DSP_AUDIO_PEQCOFADD_R_PEQCOFADD_SHIFT                          0
#define    DSP_AUDIO_PEQCOFADD_R_PEQCOFADD_MASK                           0x000000FF

#define DSP_AUDIO_PEQCOFDATA_R_OFFSET                                     0x00000F8A
#define DSP_AUDIO_PEQCOFDATA_R_TYPE                                       UInt16
#define DSP_AUDIO_PEQCOFDATA_R_RESERVED_MASK                              0x00008000
#define    DSP_AUDIO_PEQCOFDATA_R_PEQCOFDATA_SHIFT                        0
#define    DSP_AUDIO_PEQCOFDATA_R_PEQCOFDATA_MASK                         0x00007FFF

#define DSP_AUDIO_PCOMPIIRCOFADD_R_OFFSET                                 0x00000F90
#define DSP_AUDIO_PCOMPIIRCOFADD_R_TYPE                                   UInt16
#define DSP_AUDIO_PCOMPIIRCOFADD_R_RESERVED_MASK                          0x0000FF00
#define    DSP_AUDIO_PCOMPIIRCOFADD_R_PCOMPIIRCOFADD_SHIFT                0
#define    DSP_AUDIO_PCOMPIIRCOFADD_R_PCOMPIIRCOFADD_MASK                 0x000000FF

#define DSP_AUDIO_PCOMPIIRCOFDATA_R_OFFSET                                0x00000F92
#define DSP_AUDIO_PCOMPIIRCOFDATA_R_TYPE                                  UInt16
#define DSP_AUDIO_PCOMPIIRCOFDATA_R_RESERVED_MASK                         0x00008000
#define    DSP_AUDIO_PCOMPIIRCOFDATA_R_PCOMPIIRCOFDATA_SHIFT              0
#define    DSP_AUDIO_PCOMPIIRCOFDATA_R_PCOMPIIRCOFDATA_MASK               0x00007FFF

#define DSP_AUDIO_PEQEVT_R_OFFSET                                         0x00000F98
#define DSP_AUDIO_PEQEVT_R_TYPE                                           UInt16
#define DSP_AUDIO_PEQEVT_R_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_PEQEVT_R_EQEVTREG_SHIFT                              0
#define    DSP_AUDIO_PEQEVT_R_EQEVTREG_MASK                               0x0000FFFF

#define DSP_AUDIO_STEREOAUDMOD_R_OFFSET                                   0x00000FA0
#define DSP_AUDIO_STEREOAUDMOD_R_TYPE                                     UInt16
#define DSP_AUDIO_STEREOAUDMOD_R_RESERVED_MASK                            0x00000002
#define    DSP_AUDIO_STEREOAUDMOD_R_AUDIO_18BIT_MODE_SHIFT                15
#define    DSP_AUDIO_STEREOAUDMOD_R_AUDIO_18BIT_MODE_MASK                 0x00008000
#define    DSP_AUDIO_STEREOAUDMOD_R_AUD_OUT_DMA_EN_SHIFT                  14
#define    DSP_AUDIO_STEREOAUDMOD_R_AUD_OUT_DMA_EN_MASK                   0x00004000
#define    DSP_AUDIO_STEREOAUDMOD_R_ASAMPRATE_SHIFT                       10
#define    DSP_AUDIO_STEREOAUDMOD_R_ASAMPRATE_MASK                        0x00003C00
#define    DSP_AUDIO_STEREOAUDMOD_R_ARATEADP_SHIFT                        8
#define    DSP_AUDIO_STEREOAUDMOD_R_ARATEADP_MASK                         0x00000300
#define    DSP_AUDIO_STEREOAUDMOD_R_AHBIFMOD_SHIFT                        5
#define    DSP_AUDIO_STEREOAUDMOD_R_AHBIFMOD_MASK                         0x000000E0
#define    DSP_AUDIO_STEREOAUDMOD_R_AUDINTDIS_SHIFT                       4
#define    DSP_AUDIO_STEREOAUDMOD_R_AUDINTDIS_MASK                        0x00000010
#define    DSP_AUDIO_STEREOAUDMOD_R_I2SMODE_SHIFT                         3
#define    DSP_AUDIO_STEREOAUDMOD_R_I2SMODE_MASK                          0x00000008
#define    DSP_AUDIO_STEREOAUDMOD_R_ACOMPMOD_SHIFT                        2
#define    DSP_AUDIO_STEREOAUDMOD_R_ACOMPMOD_MASK                         0x00000004
#define    DSP_AUDIO_STEREOAUDMOD_R_AUDEN_SHIFT                           0
#define    DSP_AUDIO_STEREOAUDMOD_R_AUDEN_MASK                            0x00000001

#define DSP_AUDIO_AFIFOCTRL_R_OFFSET                                      0x00000FA2
#define DSP_AUDIO_AFIFOCTRL_R_TYPE                                        UInt16
#define DSP_AUDIO_AFIFOCTRL_R_RESERVED_MASK                               0x00000000
#define    DSP_AUDIO_AFIFOCTRL_R_AOFIFOCRL0_SHIFT                         15
#define    DSP_AUDIO_AFIFOCTRL_R_AOFIFOCRL0_MASK                          0x00008000
#define    DSP_AUDIO_AFIFOCTRL_R_AOFIFOTHRES0_SHIFT                       8
#define    DSP_AUDIO_AFIFOCTRL_R_AOFIFOTHRES0_MASK                        0x00007F00
#define    DSP_AUDIO_AFIFOCTRL_R_AIFIFOCRL1_SHIFT                         7
#define    DSP_AUDIO_AFIFOCTRL_R_AIFIFOCRL1_MASK                          0x00000080
#define    DSP_AUDIO_AFIFOCTRL_R_AIFIFOTHRES1_SHIFT                       0
#define    DSP_AUDIO_AFIFOCTRL_R_AIFIFOTHRES1_MASK                        0x0000007F

#define DSP_AUDIO_AIFIFOST_R_OFFSET                                       0x00000FA4
#define DSP_AUDIO_AIFIFOST_R_TYPE                                         UInt16
#define DSP_AUDIO_AIFIFOST_R_RESERVED_MASK                                0x0000F800
#define    DSP_AUDIO_AIFIFOST_R_AIFIFOOVF_SHIFT                           10
#define    DSP_AUDIO_AIFIFOST_R_AIFIFOOVF_MASK                            0x00000400
#define    DSP_AUDIO_AIFIFOST_R_AIFIFOUDF_SHIFT                           9
#define    DSP_AUDIO_AIFIFOST_R_AIFIFOUDF_MASK                            0x00000200
#define    DSP_AUDIO_AIFIFOST_R_AIFIFOTHMET_SHIFT                         8
#define    DSP_AUDIO_AIFIFOST_R_AIFIFOTHMET_MASK                          0x00000100
#define    DSP_AUDIO_AIFIFOST_R_AIFIFOEMTRYCOUNT_SHIFT                    0
#define    DSP_AUDIO_AIFIFOST_R_AIFIFOEMTRYCOUNT_MASK                     0x000000FF

#define DSP_AUDIO_ALRCH_R_OFFSET                                          0x00000FA6
#define DSP_AUDIO_ALRCH_R_TYPE                                            UInt16
#define DSP_AUDIO_ALRCH_R_RESERVED_MASK                                   0x0000FFF0
#define    DSP_AUDIO_ALRCH_R_ALCHMOD_SHIFT                                2
#define    DSP_AUDIO_ALRCH_R_ALCHMOD_MASK                                 0x0000000C
#define    DSP_AUDIO_ALRCH_R_ARCHMOD_SHIFT                                0
#define    DSP_AUDIO_ALRCH_R_ARCHMOD_MASK                                 0x00000003

#define DSP_AUDIO_AEQPATHOFST0_R_OFFSET                                   0x00000FA8
#define DSP_AUDIO_AEQPATHOFST0_R_TYPE                                     UInt16
#define DSP_AUDIO_AEQPATHOFST0_R_RESERVED_MASK                            0x00000000
#define    DSP_AUDIO_AEQPATHOFST0_R_APATH2DELAY_SHIFT                     14
#define    DSP_AUDIO_AEQPATHOFST0_R_APATH2DELAY_MASK                      0x0000C000
#define    DSP_AUDIO_AEQPATHOFST0_R_APATH1DELAY_SHIFT                     7
#define    DSP_AUDIO_AEQPATHOFST0_R_APATH1DELAY_MASK                      0x00003F80
#define    DSP_AUDIO_AEQPATHOFST0_R_APATH0DELAY_SHIFT                     0
#define    DSP_AUDIO_AEQPATHOFST0_R_APATH0DELAY_MASK                      0x0000007F

#define DSP_AUDIO_AEQPATHOFST1_R_OFFSET                                   0x00000FAA
#define DSP_AUDIO_AEQPATHOFST1_R_TYPE                                     UInt16
#define DSP_AUDIO_AEQPATHOFST1_R_RESERVED_MASK                            0x00000000
#define    DSP_AUDIO_AEQPATHOFST1_R_APATH4DELAY_SHIFT                     12
#define    DSP_AUDIO_AEQPATHOFST1_R_APATH4DELAY_MASK                      0x0000F000
#define    DSP_AUDIO_AEQPATHOFST1_R_APATH3DELAY_SHIFT                     5
#define    DSP_AUDIO_AEQPATHOFST1_R_APATH3DELAY_MASK                      0x00000FE0
#define    DSP_AUDIO_AEQPATHOFST1_R_APATH2DELAY_SHIFT                     0
#define    DSP_AUDIO_AEQPATHOFST1_R_APATH2DELAY_MASK                      0x0000001F

#define DSP_AUDIO_AEQPATHOFST2_R_OFFSET                                   0x00000FAC
#define DSP_AUDIO_AEQPATHOFST2_R_TYPE                                     UInt16
#define DSP_AUDIO_AEQPATHOFST2_R_RESERVED_MASK                            0x0000FC00
#define    DSP_AUDIO_AEQPATHOFST2_R_APATH5DELAY_SHIFT                     3
#define    DSP_AUDIO_AEQPATHOFST2_R_APATH5DELAY_MASK                      0x000003F8
#define    DSP_AUDIO_AEQPATHOFST2_R_APATH4DELAY_SHIFT                     0
#define    DSP_AUDIO_AEQPATHOFST2_R_APATH4DELAY_MASK                      0x00000007

#define DSP_AUDIO_AEQEVT_R_OFFSET                                         0x00000FAE
#define DSP_AUDIO_AEQEVT_R_TYPE                                           UInt16
#define DSP_AUDIO_AEQEVT_R_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_AEQEVT_R_EQEVTREG_SHIFT                              0
#define    DSP_AUDIO_AEQEVT_R_EQEVTREG_MASK                               0x0000FFFF

#define DSP_AUDIO_ALSLOPGAIN_R_OFFSET                                     0x00000FB0
#define DSP_AUDIO_ALSLOPGAIN_R_TYPE                                       UInt16
#define DSP_AUDIO_ALSLOPGAIN_R_RESERVED_MASK                              0x00000600
#define    DSP_AUDIO_ALSLOPGAIN_R_ALSLOPGAINEN_SHIFT                      15
#define    DSP_AUDIO_ALSLOPGAIN_R_ALSLOPGAINEN_MASK                       0x00008000
#define    DSP_AUDIO_ALSLOPGAIN_R_ALSLOPMOD_SHIFT                         11
#define    DSP_AUDIO_ALSLOPGAIN_R_ALSLOPMOD_MASK                          0x00007800
#define    DSP_AUDIO_ALSLOPGAIN_R_ALTARGETGAIN_SHIFT                      0
#define    DSP_AUDIO_ALSLOPGAIN_R_ALTARGETGAIN_MASK                       0x000001FF

#define DSP_AUDIO_ARSLOPGAIN_R_OFFSET                                     0x00000FB2
#define DSP_AUDIO_ARSLOPGAIN_R_TYPE                                       UInt16
#define DSP_AUDIO_ARSLOPGAIN_R_RESERVED_MASK                              0x00000600
#define    DSP_AUDIO_ARSLOPGAIN_R_ARSLOPGAINEN_SHIFT                      15
#define    DSP_AUDIO_ARSLOPGAIN_R_ARSLOPGAINEN_MASK                       0x00008000
#define    DSP_AUDIO_ARSLOPGAIN_R_ARSLOPMOD_SHIFT                         11
#define    DSP_AUDIO_ARSLOPGAIN_R_ARSLOPMOD_MASK                          0x00007800
#define    DSP_AUDIO_ARSLOPGAIN_R_ARTARGETGAIN_SHIFT                      0
#define    DSP_AUDIO_ARSLOPGAIN_R_ARTARGETGAIN_MASK                       0x000001FF

#define DSP_AUDIO_BTNBDINL_R_OFFSET                                       0x00000FB4
#define DSP_AUDIO_BTNBDINL_R_TYPE                                         UInt16
#define DSP_AUDIO_BTNBDINL_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_BTNBDINL_R_BTNBDATAIN_SHIFT                          0
#define    DSP_AUDIO_BTNBDINL_R_BTNBDATAIN_MASK                           0x0000FFFF

#define DSP_AUDIO_BTNBDINR_R_OFFSET                                       0x00000FB6
#define DSP_AUDIO_BTNBDINR_R_TYPE                                         UInt16
#define DSP_AUDIO_BTNBDINR_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_BTNBDINR_R_BTNBDATAIN_SHIFT                          0
#define    DSP_AUDIO_BTNBDINR_R_BTNBDATAIN_MASK                           0x0000FFFF

#define DSP_AUDIO_BTMIXER_CFG_R_OFFSET                                    0x00000FB8
#define DSP_AUDIO_BTMIXER_CFG_R_TYPE                                      UInt16
#define DSP_AUDIO_BTMIXER_CFG_R_RESERVED_MASK                             0x00000104
#define    DSP_AUDIO_BTMIXER_CFG_R_BTMIXER_WBFIFOTHRES_SHIFT              9
#define    DSP_AUDIO_BTMIXER_CFG_R_BTMIXER_WBFIFOTHRES_MASK               0x0000FE00
#define    DSP_AUDIO_BTMIXER_CFG_R_BTMIXER_DMA_EN_SHIFT                   7
#define    DSP_AUDIO_BTMIXER_CFG_R_BTMIXER_DMA_EN_MASK                    0x00000080
#define    DSP_AUDIO_BTMIXER_CFG_R_BTMIXER_WBINTDIS_SHIFT                 6
#define    DSP_AUDIO_BTMIXER_CFG_R_BTMIXER_WBINTDIS_MASK                  0x00000040
#define    DSP_AUDIO_BTMIXER_CFG_R_BTMIXER_WBINTFLAG_SHIFT                5
#define    DSP_AUDIO_BTMIXER_CFG_R_BTMIXER_WBINTFLAG_MASK                 0x00000020
#define    DSP_AUDIO_BTMIXER_CFG_R_WBLRSW_SHIFT                           4
#define    DSP_AUDIO_BTMIXER_CFG_R_WBLRSW_MASK                            0x00000010
#define    DSP_AUDIO_BTMIXER_CFG_R_WBRATE_SHIFT                           3
#define    DSP_AUDIO_BTMIXER_CFG_R_WBRATE_MASK                            0x00000008
#define    DSP_AUDIO_BTMIXER_CFG_R_WBENABLE_SHIFT                         0
#define    DSP_AUDIO_BTMIXER_CFG_R_WBENABLE_MASK                          0x00000003

#define DSP_AUDIO_BTMIXER_CFG2_R_OFFSET                                   0x00000FBA
#define DSP_AUDIO_BTMIXER_CFG2_R_TYPE                                     UInt16
#define DSP_AUDIO_BTMIXER_CFG2_R_RESERVED_MASK                            0x00008000
#define    DSP_AUDIO_BTMIXER_CFG2_R_BTMIXER_WBENTRY_SHIFT                 8
#define    DSP_AUDIO_BTMIXER_CFG2_R_BTMIXER_WBENTRY_MASK                  0x00007F00
#define    DSP_AUDIO_BTMIXER_CFG2_R_PATH_SEL_SHIFT                        6
#define    DSP_AUDIO_BTMIXER_CFG2_R_PATH_SEL_MASK                         0x000000C0
#define    DSP_AUDIO_BTMIXER_CFG2_R_PATH_SOURCE_SHIFT                     4
#define    DSP_AUDIO_BTMIXER_CFG2_R_PATH_SOURCE_MASK                      0x00000030
#define    DSP_AUDIO_BTMIXER_CFG2_R_BTNB_ENABLE_SHIFT                     3
#define    DSP_AUDIO_BTMIXER_CFG2_R_BTNB_ENABLE_MASK                      0x00000008
#define    DSP_AUDIO_BTMIXER_CFG2_R_BTNB_RATE_SHIFT                       2
#define    DSP_AUDIO_BTMIXER_CFG2_R_BTNB_RATE_MASK                        0x00000004
#define    DSP_AUDIO_BTMIXER_CFG2_R_BTWBFIFO_UDF_SHIFT                    1
#define    DSP_AUDIO_BTMIXER_CFG2_R_BTWBFIFO_UDF_MASK                     0x00000002
#define    DSP_AUDIO_BTMIXER_CFG2_R_BTWBFIFO_OVF_SHIFT                    0
#define    DSP_AUDIO_BTMIXER_CFG2_R_BTWBFIFO_OVF_MASK                     0x00000001

#define DSP_AUDIO_BTMIXER_GAIN_L_R_OFFSET                                 0x00000FBC
#define DSP_AUDIO_BTMIXER_GAIN_L_R_TYPE                                   UInt16
#define DSP_AUDIO_BTMIXER_GAIN_L_R_RESERVED_MASK                          0x00000000
#define    DSP_AUDIO_BTMIXER_GAIN_L_R_BTMIXER_GAIN_L_SHIFT                0
#define    DSP_AUDIO_BTMIXER_GAIN_L_R_BTMIXER_GAIN_L_MASK                 0x0000FFFF

#define DSP_AUDIO_BTMIXER_GAIN_R_R_OFFSET                                 0x00000FBE
#define DSP_AUDIO_BTMIXER_GAIN_R_R_TYPE                                   UInt16
#define DSP_AUDIO_BTMIXER_GAIN_R_R_RESERVED_MASK                          0x00000000
#define    DSP_AUDIO_BTMIXER_GAIN_R_R_BTMIXER_GAIN_R_SHIFT                0
#define    DSP_AUDIO_BTMIXER_GAIN_R_R_BTMIXER_GAIN_R_MASK                 0x0000FFFF

#define DSP_AUDIO_AEQPATHGAIN1_R_OFFSET                                   0x00000FC2
#define DSP_AUDIO_AEQPATHGAIN1_R_TYPE                                     UInt16
#define DSP_AUDIO_AEQPATHGAIN1_R_RESERVED_MASK                            0x00000600
#define    DSP_AUDIO_AEQPATHGAIN1_R_AEQPATHGAIN1EN_SHIFT                  15
#define    DSP_AUDIO_AEQPATHGAIN1_R_AEQPATHGAIN1EN_MASK                   0x00008000
#define    DSP_AUDIO_AEQPATHGAIN1_R_EQGAINSLOP_SHIFT                      11
#define    DSP_AUDIO_AEQPATHGAIN1_R_EQGAINSLOP_MASK                       0x00007800
#define    DSP_AUDIO_AEQPATHGAIN1_R_AEQPATHGAIN1_SHIFT                    0
#define    DSP_AUDIO_AEQPATHGAIN1_R_AEQPATHGAIN1_MASK                     0x000001FF

#define DSP_AUDIO_AEQPATHGAIN2_R_OFFSET                                   0x00000FC4
#define DSP_AUDIO_AEQPATHGAIN2_R_TYPE                                     UInt16
#define DSP_AUDIO_AEQPATHGAIN2_R_RESERVED_MASK                            0x0000FE00
#define    DSP_AUDIO_AEQPATHGAIN2_R_AEQPATHGAIN2_SHIFT                    0
#define    DSP_AUDIO_AEQPATHGAIN2_R_AEQPATHGAIN2_MASK                     0x000001FF

#define DSP_AUDIO_AEQPATHGAIN3_R_OFFSET                                   0x00000FC6
#define DSP_AUDIO_AEQPATHGAIN3_R_TYPE                                     UInt16
#define DSP_AUDIO_AEQPATHGAIN3_R_RESERVED_MASK                            0x0000FE00
#define    DSP_AUDIO_AEQPATHGAIN3_R_AEQPATHGAIN3_SHIFT                    0
#define    DSP_AUDIO_AEQPATHGAIN3_R_AEQPATHGAIN3_MASK                     0x000001FF

#define DSP_AUDIO_AEQPATHGAIN4_R_OFFSET                                   0x00000FC8
#define DSP_AUDIO_AEQPATHGAIN4_R_TYPE                                     UInt16
#define DSP_AUDIO_AEQPATHGAIN4_R_RESERVED_MASK                            0x0000FE00
#define    DSP_AUDIO_AEQPATHGAIN4_R_AEQPATHGAIN4_SHIFT                    0
#define    DSP_AUDIO_AEQPATHGAIN4_R_AEQPATHGAIN4_MASK                     0x000001FF

#define DSP_AUDIO_AEQPATHGAIN5_R_OFFSET                                   0x00000FCA
#define DSP_AUDIO_AEQPATHGAIN5_R_TYPE                                     UInt16
#define DSP_AUDIO_AEQPATHGAIN5_R_RESERVED_MASK                            0x0000FE00
#define    DSP_AUDIO_AEQPATHGAIN5_R_AEQPATHGAIN5_SHIFT                    0
#define    DSP_AUDIO_AEQPATHGAIN5_R_AEQPATHGAIN5_MASK                     0x000001FF

#define DSP_AUDIO_AUDVOC_ISR_R_OFFSET                                     0x00000FE0
#define DSP_AUDIO_AUDVOC_ISR_R_TYPE                                       UInt16
#define DSP_AUDIO_AUDVOC_ISR_R_RESERVED_MASK                              0x0000FE20
#define    DSP_AUDIO_AUDVOC_ISR_R_VOICE_IN_ONLY_INT_SHIFT                 8
#define    DSP_AUDIO_AUDVOC_ISR_R_VOICE_IN_ONLY_INT_MASK                  0x00000100
#define    DSP_AUDIO_AUDVOC_ISR_R_BTTAP_NB_INT_SHIFT                      7
#define    DSP_AUDIO_AUDVOC_ISR_R_BTTAP_NB_INT_MASK                       0x00000080
#define    DSP_AUDIO_AUDVOC_ISR_R_BTTAP_WB_INT_SHIFT                      6
#define    DSP_AUDIO_AUDVOC_ISR_R_BTTAP_WB_INT_MASK                       0x00000040
#define    DSP_AUDIO_AUDVOC_ISR_R_POLYRINGER_INT_SHIFT                    4
#define    DSP_AUDIO_AUDVOC_ISR_R_POLYRINGER_INT_MASK                     0x00000010
#define    DSP_AUDIO_AUDVOC_ISR_R_ADC_AUDIO_INT_SHIFT                     3
#define    DSP_AUDIO_AUDVOC_ISR_R_ADC_AUDIO_INT_MASK                      0x00000008
#define    DSP_AUDIO_AUDVOC_ISR_R_DAC_AUDIO_INT_SHIFT                     2
#define    DSP_AUDIO_AUDVOC_ISR_R_DAC_AUDIO_INT_MASK                      0x00000004
#define    DSP_AUDIO_AUDVOC_ISR_R_VOICE_INT_SHIFT                         1
#define    DSP_AUDIO_AUDVOC_ISR_R_VOICE_INT_MASK                          0x00000002
#define    DSP_AUDIO_AUDVOC_ISR_R_PCM_INT_SHIFT                           0
#define    DSP_AUDIO_AUDVOC_ISR_R_PCM_INT_MASK                            0x00000001

#define DSP_AUDIO_AIFIFODATA0_R_OFFSET                                    0x00000400
#define DSP_AUDIO_AIFIFODATA0_R_TYPE                                      UInt32
#define DSP_AUDIO_AIFIFODATA0_R_RESERVED_MASK                             0x00000000
#define    DSP_AUDIO_AIFIFODATA0_R_AIFIFODATAR_SHIFT                      16
#define    DSP_AUDIO_AIFIFODATA0_R_AIFIFODATAR_MASK                       0xFFFF0000
#define    DSP_AUDIO_AIFIFODATA0_R_AIFIFODATAL_SHIFT                      0
#define    DSP_AUDIO_AIFIFODATA0_R_AIFIFODATAL_MASK                       0x0000FFFF

#define DSP_AUDIO_AIFIFODATA1_R_OFFSET                                    0x00000404
#define DSP_AUDIO_AIFIFODATA1_R_TYPE                                      UInt32
#define DSP_AUDIO_AIFIFODATA1_R_RESERVED_MASK                             0x00000000
#define    DSP_AUDIO_AIFIFODATA1_R_AIFIFORDATA_SHIFT                      16
#define    DSP_AUDIO_AIFIFODATA1_R_AIFIFORDATA_MASK                       0xFFFF0000
#define    DSP_AUDIO_AIFIFODATA1_R_AIFIFOLDATA_SHIFT                      0
#define    DSP_AUDIO_AIFIFODATA1_R_AIFIFOLDATA_MASK                       0x0000FFFF

#define DSP_AUDIO_AEQCOFADD_R_OFFSET                                      0x00000FE8
#define DSP_AUDIO_AEQCOFADD_R_TYPE                                        UInt16
#define DSP_AUDIO_AEQCOFADD_R_RESERVED_MASK                               0x0000FF00
#define    DSP_AUDIO_AEQCOFADD_R_AEQCOFADD_SHIFT                          0
#define    DSP_AUDIO_AEQCOFADD_R_AEQCOFADD_MASK                           0x000000FF

#define DSP_AUDIO_AEQCOFDATA_R_OFFSET                                     0x00000FEA
#define DSP_AUDIO_AEQCOFDATA_R_TYPE                                       UInt16
#define DSP_AUDIO_AEQCOFDATA_R_RESERVED_MASK                              0x00008000
#define    DSP_AUDIO_AEQCOFDATA_R_AEQCOFDATA_SHIFT                        0
#define    DSP_AUDIO_AEQCOFDATA_R_AEQCOFDATA_MASK                         0x00007FFF

#define DSP_AUDIO_ACOMPFIRCOFADD_R_OFFSET                                 0x00000FEC
#define DSP_AUDIO_ACOMPFIRCOFADD_R_TYPE                                   UInt16
#define DSP_AUDIO_ACOMPFIRCOFADD_R_RESERVED_MASK                          0x0000FF00
#define    DSP_AUDIO_ACOMPFIRCOFADD_R_ACOMPFIRCOFADD_SHIFT                0
#define    DSP_AUDIO_ACOMPFIRCOFADD_R_ACOMPFIRCOFADD_MASK                 0x000000FF

#define DSP_AUDIO_ACOMPFIRCOFDATA_R_OFFSET                                0x00000FEE
#define DSP_AUDIO_ACOMPFIRCOFDATA_R_TYPE                                  UInt16
#define DSP_AUDIO_ACOMPFIRCOFDATA_R_RESERVED_MASK                         0x00008000
#define    DSP_AUDIO_ACOMPFIRCOFDATA_R_ACOMPFIRCOFDATA_SHIFT              0
#define    DSP_AUDIO_ACOMPFIRCOFDATA_R_ACOMPFIRCOFDATA_MASK               0x00007FFF

#define DSP_AUDIO_MPM_ALGC_ADDRREG_R_OFFSET                               0x00000FFA
#define DSP_AUDIO_MPM_ALGC_ADDRREG_R_TYPE                                 UInt16
#define DSP_AUDIO_MPM_ALGC_ADDRREG_R_RESERVED_MASK                        0x00000000
#define    DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_SHIFT                        0
#define    DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_MASK                         0x0000FFFF
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_DGA_G0_ADDR       0x00000000
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_DGA_G1_ADDR       0x00000001
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_DGA_G2_ADDR       0x00000002
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_DGA_G3_ADDR       0x00000003
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_DGA_SLOPE0_ADDR   0x00000004
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_DGA_SLOPE1_ADDR   0x00000005
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_DGA_SLOPE2_ADDR   0x00000006
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_DGA_SLOPE3_ADDR   0x00000007
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_GAIN_ATTACK_STEP0_ADDR 0x00000008
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_GAIN_ATTACK_SLOPE0_ADDR 0x00000009
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_GAIN_DECAY_SLOPE0_ADDR 0x0000000A
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_GAIN_ATTACK_STEP1_ADDR 0x0000000B
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_GAIN_ATTACK_SLOPE1_ADDR 0x0000000C
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_GAIN_DECAY_SLOPE1_ADDR 0x0000000D
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_GAIN_ATTACK_STEP2_ADDR 0x0000000E
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_GAIN_ATTACK_SLOPE2_ADDR 0x0000000F
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_GAIN_DECAY_SLOPE2_ADDR 0x00000010
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_GAIN_ATTACK_STEP3_ADDR 0x00000011
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_GAIN_ATTACK_SLOPE3_ADDR 0x00000012
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_GAIN_DECAY_SLOPE3_ADDR 0x00000013
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_ALDC_NIIR0_ADDR   0x00000014
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_ALDC_NIIR1_ADDR   0x00000015
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_ALDC_NIIR2_ADDR   0x00000016
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_ALDC_NIIR3_ADDR   0x00000017
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_GAIN_ATTACK_THRESHOLD0_ADDR 0x00000018
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_GAIN_ATTACK_THRESHOLD1_ADDR 0x00000019
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_GAIN_ATTACK_THRESHOLD2_ADDR 0x0000001A
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_GAIN_ATTACK_THRESHOLD3_ADDR 0x0000001B
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_GAIN_CTRL_SEL0_ADDR 0x0000001C
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_GAIN_CTRL_SEL1_ADDR 0x0000001D
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_GAIN_CTRL_SEL2_ADDR 0x0000001E
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_GAIN_CTRL_SEL3_ADDR 0x0000001F
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_DGA_SLOPE_EN_ADDR 0x00000020
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_ALDC_EN0_ADDR     0x00000021
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_ALDC_EN1_ADDR     0x00000022
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_ALDC_EN2_ADDR     0x00000023
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_ALDC_EN3_ADDR     0x00000024
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_TEST_CTRL_ADDR    0x00000025
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_TEST_MIXER_INPUT_ADDR 0x00000026
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_TEST_BUS_OUT_ADDR 0x00000027

#define DSP_AUDIO_MPM_ALGC_DATAHREG_R_OFFSET                              0x00000FFC
#define DSP_AUDIO_MPM_ALGC_DATAHREG_R_TYPE                                UInt16
#define DSP_AUDIO_MPM_ALGC_DATAHREG_R_RESERVED_MASK                       0x00000000
#define    DSP_AUDIO_MPM_ALGC_DATAHREG_R_UNION_SHIFT                      0
#define    DSP_AUDIO_MPM_ALGC_DATAHREG_R_UNION_MASK                       0x0000FFFF

#define DSP_AUDIO_MPM_ALGC_DATALREG_R_OFFSET                              0x00000FFE
#define DSP_AUDIO_MPM_ALGC_DATALREG_R_TYPE                                UInt16
#define DSP_AUDIO_MPM_ALGC_DATALREG_R_RESERVED_MASK                       0x00000000
#define    DSP_AUDIO_MPM_ALGC_DATALREG_R_UNION_SHIFT                      0
#define    DSP_AUDIO_MPM_ALGC_DATALREG_R_UNION_MASK                       0x0000FFFF

#define DSP_AUDIO_AMCR_R_OFFSET                                           0x00000A80
#define DSP_AUDIO_AMCR_R_TYPE                                             UInt16
#define DSP_AUDIO_AMCR_R_RESERVED_MASK                                    0x00003F18
#define    DSP_AUDIO_AMCR_R_DAISTAT_SHIFT                                 15
#define    DSP_AUDIO_AMCR_R_DAISTAT_MASK                                  0x00008000
#define    DSP_AUDIO_AMCR_R_PCMEN_SHIFT                                   14
#define    DSP_AUDIO_AMCR_R_PCMEN_MASK                                    0x00004000
#define    DSP_AUDIO_AMCR_R_CRAMSEL_SHIFT                                 7
#define    DSP_AUDIO_AMCR_R_CRAMSEL_MASK                                  0x00000080
#define    DSP_AUDIO_AMCR_R_MODE_16K_SHIFT                                6
#define    DSP_AUDIO_AMCR_R_MODE_16K_MASK                                 0x00000040
#define    DSP_AUDIO_AMCR_R_AUDEN_SHIFT                                   5
#define    DSP_AUDIO_AMCR_R_AUDEN_MASK                                    0x00000020
#define    DSP_AUDIO_AMCR_R_DLOOPBACK_SHIFT                               2
#define    DSP_AUDIO_AMCR_R_DLOOPBACK_MASK                                0x00000004
#define    DSP_AUDIO_AMCR_R_AUDT_SHIFT                                    0
#define    DSP_AUDIO_AMCR_R_AUDT_MASK                                     0x00000003

#define DSP_AUDIO_VOICEFIFO_THRES_R_OFFSET                                0x00000A8A
#define DSP_AUDIO_VOICEFIFO_THRES_R_TYPE                                  UInt16
#define DSP_AUDIO_VOICEFIFO_THRES_R_RESERVED_MASK                         0x000088F8
#define    DSP_AUDIO_VOICEFIFO_THRES_R_BTNBFIFO_THRES_SHIFT               12
#define    DSP_AUDIO_VOICEFIFO_THRES_R_BTNBFIFO_THRES_MASK                0x00007000
#define    DSP_AUDIO_VOICEFIFO_THRES_R_VIFIFO_THRES_SHIFT                 8
#define    DSP_AUDIO_VOICEFIFO_THRES_R_VIFIFO_THRES_MASK                  0x00000700
#define    DSP_AUDIO_VOICEFIFO_THRES_R_VOINPUTFIFO_THRES_SHIFT            0
#define    DSP_AUDIO_VOICEFIFO_THRES_R_VOINPUTFIFO_THRES_MASK             0x00000007

#define DSP_AUDIO_VOICEFIFO_STATUS_R_OFFSET                               0x00000A98
#define DSP_AUDIO_VOICEFIFO_STATUS_R_TYPE                                 UInt16
#define DSP_AUDIO_VOICEFIFO_STATUS_R_RESERVED_MASK                        0x000000F0
#define    DSP_AUDIO_VOICEFIFO_STATUS_R_BTNBFIFO_ENTRY_SHIFT              12
#define    DSP_AUDIO_VOICEFIFO_STATUS_R_BTNBFIFO_ENTRY_MASK               0x0000F000
#define    DSP_AUDIO_VOICEFIFO_STATUS_R_VIFIFO_ENTRY_SHIFT                8
#define    DSP_AUDIO_VOICEFIFO_STATUS_R_VIFIFO_ENTRY_MASK                 0x00000F00
#define    DSP_AUDIO_VOICEFIFO_STATUS_R_VOINPUTFIFO_EMPTYCOUNT_SHIFT      0
#define    DSP_AUDIO_VOICEFIFO_STATUS_R_VOINPUTFIFO_EMPTYCOUNT_MASK       0x0000000F

#define DSP_AUDIO_VOICEFIFO_CLEAR_R_OFFSET                                0x00000A94
#define DSP_AUDIO_VOICEFIFO_CLEAR_R_TYPE                                  UInt16
#define DSP_AUDIO_VOICEFIFO_CLEAR_R_RESERVED_MASK                         0x0000FFF0
#define    DSP_AUDIO_VOICEFIFO_CLEAR_R_PCMFIFO_CLR_SHIFT                  3
#define    DSP_AUDIO_VOICEFIFO_CLEAR_R_PCMFIFO_CLR_MASK                   0x00000008
#define    DSP_AUDIO_VOICEFIFO_CLEAR_R_BTNBFIFO_CLR_SHIFT                 2
#define    DSP_AUDIO_VOICEFIFO_CLEAR_R_BTNBFIFO_CLR_MASK                  0x00000004
#define    DSP_AUDIO_VOICEFIFO_CLEAR_R_VIFIFO_CLR_SHIFT                   1
#define    DSP_AUDIO_VOICEFIFO_CLEAR_R_VIFIFO_CLR_MASK                    0x00000002
#define    DSP_AUDIO_VOICEFIFO_CLEAR_R_VOINPUTFIFO_CLR_SHIFT              0
#define    DSP_AUDIO_VOICEFIFO_CLEAR_R_VOINPUTFIFO_CLR_MASK               0x00000001

#define DSP_AUDIO_PCMFIFO_CONTROL_STATUS_R_OFFSET                         0x00000A96
#define DSP_AUDIO_PCMFIFO_CONTROL_STATUS_R_TYPE                           UInt16
#define DSP_AUDIO_PCMFIFO_CONTROL_STATUS_R_RESERVED_MASK                  0x00000088
#define    DSP_AUDIO_PCMFIFO_CONTROL_STATUS_R_PCMTXFIFO_EMPTYCOUNT_SHIFT  12
#define    DSP_AUDIO_PCMFIFO_CONTROL_STATUS_R_PCMTXFIFO_EMPTYCOUNT_MASK   0x0000F000
#define    DSP_AUDIO_PCMFIFO_CONTROL_STATUS_R_PCMRXFIFO_ENTRY_SHIFT       8
#define    DSP_AUDIO_PCMFIFO_CONTROL_STATUS_R_PCMRXFIFO_ENTRY_MASK        0x00000F00
#define    DSP_AUDIO_PCMFIFO_CONTROL_STATUS_R_PCMTXFIFO_THRES_SHIFT       4
#define    DSP_AUDIO_PCMFIFO_CONTROL_STATUS_R_PCMTXFIFO_THRES_MASK        0x00000070
#define    DSP_AUDIO_PCMFIFO_CONTROL_STATUS_R_PCMRXFIFO_THRES_SHIFT       0
#define    DSP_AUDIO_PCMFIFO_CONTROL_STATUS_R_PCMRXFIFO_THRES_MASK        0x00000007

#define DSP_AUDIO_VCOEFR0_R_OFFSET                                        0x00000BA0
#define DSP_AUDIO_VCOEFR0_R_TYPE                                          UInt16
#define DSP_AUDIO_VCOEFR0_R_RESERVED_MASK                                 0x00000000
#define    DSP_AUDIO_VCOEFR0_R_VOICEIIRCOEF_SHIFT                         0
#define    DSP_AUDIO_VCOEFR0_R_VOICEIIRCOEF_MASK                          0x0000FFFF

#define DSP_AUDIO_VCOEFR1_R_OFFSET                                        0x00000BA2
#define DSP_AUDIO_VCOEFR1_R_TYPE                                          UInt16
#define DSP_AUDIO_VCOEFR1_R_RESERVED_MASK                                 0x00000000
#define    DSP_AUDIO_VCOEFR1_R_VOICEIIRCOEF_SHIFT                         0
#define    DSP_AUDIO_VCOEFR1_R_VOICEIIRCOEF_MASK                          0x0000FFFF

#define DSP_AUDIO_VCOEFR2_R_OFFSET                                        0x00000BA4
#define DSP_AUDIO_VCOEFR2_R_TYPE                                          UInt16
#define DSP_AUDIO_VCOEFR2_R_RESERVED_MASK                                 0x00000000
#define    DSP_AUDIO_VCOEFR2_R_VOICEIIRCOEF_SHIFT                         0
#define    DSP_AUDIO_VCOEFR2_R_VOICEIIRCOEF_MASK                          0x0000FFFF

#define DSP_AUDIO_VCOEFR3_R_OFFSET                                        0x00000BA6
#define DSP_AUDIO_VCOEFR3_R_TYPE                                          UInt16
#define DSP_AUDIO_VCOEFR3_R_RESERVED_MASK                                 0x00000000
#define    DSP_AUDIO_VCOEFR3_R_VOICEIIRCOEF_SHIFT                         0
#define    DSP_AUDIO_VCOEFR3_R_VOICEIIRCOEF_MASK                          0x0000FFFF

#define DSP_AUDIO_VCOEFR4_R_OFFSET                                        0x00000BA8
#define DSP_AUDIO_VCOEFR4_R_TYPE                                          UInt16
#define DSP_AUDIO_VCOEFR4_R_RESERVED_MASK                                 0x00000000
#define    DSP_AUDIO_VCOEFR4_R_VOICEIIRCOEF_SHIFT                         0
#define    DSP_AUDIO_VCOEFR4_R_VOICEIIRCOEF_MASK                          0x0000FFFF

#define DSP_AUDIO_VCOEFR5_R_OFFSET                                        0x00000BAA
#define DSP_AUDIO_VCOEFR5_R_TYPE                                          UInt16
#define DSP_AUDIO_VCOEFR5_R_RESERVED_MASK                                 0x00000000
#define    DSP_AUDIO_VCOEFR5_R_VOICEIIRCOEF_SHIFT                         0
#define    DSP_AUDIO_VCOEFR5_R_VOICEIIRCOEF_MASK                          0x0000FFFF

#define DSP_AUDIO_VCOEFR6_R_OFFSET                                        0x00000BAC
#define DSP_AUDIO_VCOEFR6_R_TYPE                                          UInt16
#define DSP_AUDIO_VCOEFR6_R_RESERVED_MASK                                 0x00000000
#define    DSP_AUDIO_VCOEFR6_R_VOICEIIRCOEF_SHIFT                         0
#define    DSP_AUDIO_VCOEFR6_R_VOICEIIRCOEF_MASK                          0x0000FFFF

#define DSP_AUDIO_VCOEFR7_R_OFFSET                                        0x00000BAE
#define DSP_AUDIO_VCOEFR7_R_TYPE                                          UInt16
#define DSP_AUDIO_VCOEFR7_R_RESERVED_MASK                                 0x00000000
#define    DSP_AUDIO_VCOEFR7_R_VOICEIIRCOEF_SHIFT                         0
#define    DSP_AUDIO_VCOEFR7_R_VOICEIIRCOEF_MASK                          0x0000FFFF

#define DSP_AUDIO_VCOEFR8_R_OFFSET                                        0x00000BB0
#define DSP_AUDIO_VCOEFR8_R_TYPE                                          UInt16
#define DSP_AUDIO_VCOEFR8_R_RESERVED_MASK                                 0x00000000
#define    DSP_AUDIO_VCOEFR8_R_VOICEIIRCOEF_SHIFT                         0
#define    DSP_AUDIO_VCOEFR8_R_VOICEIIRCOEF_MASK                          0x0000FFFF

#define DSP_AUDIO_VCOEFR9_R_OFFSET                                        0x00000BB2
#define DSP_AUDIO_VCOEFR9_R_TYPE                                          UInt16
#define DSP_AUDIO_VCOEFR9_R_RESERVED_MASK                                 0x00000000
#define    DSP_AUDIO_VCOEFR9_R_VOICEIIRCOEF_SHIFT                         0
#define    DSP_AUDIO_VCOEFR9_R_VOICEIIRCOEF_MASK                          0x0000FFFF

#define DSP_AUDIO_VCOEFR10_R_OFFSET                                       0x00000BB4
#define DSP_AUDIO_VCOEFR10_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR10_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR10_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR10_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR11_R_OFFSET                                       0x00000BB6
#define DSP_AUDIO_VCOEFR11_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR11_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR11_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR11_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR12_R_OFFSET                                       0x00000BB8
#define DSP_AUDIO_VCOEFR12_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR12_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR12_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR12_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR13_R_OFFSET                                       0x00000BBA
#define DSP_AUDIO_VCOEFR13_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR13_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR13_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR13_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR14_R_OFFSET                                       0x00000BBC
#define DSP_AUDIO_VCOEFR14_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR14_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR14_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR14_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR15_R_OFFSET                                       0x00000BBE
#define DSP_AUDIO_VCOEFR15_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR15_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR15_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR15_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR16_R_OFFSET                                       0x00000BC0
#define DSP_AUDIO_VCOEFR16_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR16_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR16_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR16_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR17_R_OFFSET                                       0x00000BC2
#define DSP_AUDIO_VCOEFR17_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR17_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR17_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR17_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR18_R_OFFSET                                       0x00000BC4
#define DSP_AUDIO_VCOEFR18_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR18_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR18_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR18_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR19_R_OFFSET                                       0x00000BC6
#define DSP_AUDIO_VCOEFR19_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR19_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR19_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR19_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR20_R_OFFSET                                       0x00000BC8
#define DSP_AUDIO_VCOEFR20_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR20_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR20_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR20_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR21_R_OFFSET                                       0x00000BCA
#define DSP_AUDIO_VCOEFR21_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR21_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR21_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR21_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR22_R_OFFSET                                       0x00000BCC
#define DSP_AUDIO_VCOEFR22_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR22_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR22_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR22_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR23_R_OFFSET                                       0x00000BCE
#define DSP_AUDIO_VCOEFR23_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR23_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR23_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR23_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR24_R_OFFSET                                       0x00000BD0
#define DSP_AUDIO_VCOEFR24_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR24_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR24_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR24_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR25_R_OFFSET                                       0x00000BD2
#define DSP_AUDIO_VCOEFR25_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR25_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR25_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR25_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR26_R_OFFSET                                       0x00000BD4
#define DSP_AUDIO_VCOEFR26_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR26_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR26_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR26_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR27_R_OFFSET                                       0x00000BD6
#define DSP_AUDIO_VCOEFR27_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR27_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR27_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR27_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR28_R_OFFSET                                       0x00000BD8
#define DSP_AUDIO_VCOEFR28_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR28_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR28_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR28_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR29_R_OFFSET                                       0x00000BDA
#define DSP_AUDIO_VCOEFR29_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR29_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR29_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR29_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR30_R_OFFSET                                       0x00000BDC
#define DSP_AUDIO_VCOEFR30_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR30_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR30_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR30_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR31_R_OFFSET                                       0x00000BDE
#define DSP_AUDIO_VCOEFR31_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR31_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR31_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR31_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR32_R_OFFSET                                       0x00000BE0
#define DSP_AUDIO_VCOEFR32_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR32_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR32_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR32_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR33_R_OFFSET                                       0x00000BE2
#define DSP_AUDIO_VCOEFR33_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR33_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR33_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR33_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCOEFR34_R_OFFSET                                       0x00000BE4
#define DSP_AUDIO_VCOEFR34_R_TYPE                                         UInt16
#define DSP_AUDIO_VCOEFR34_R_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_VCOEFR34_R_VOICEIIRCOEF_SHIFT                        0
#define    DSP_AUDIO_VCOEFR34_R_VOICEIIRCOEF_MASK                         0x0000FFFF

#define DSP_AUDIO_VCFGR_R_OFFSET                                          0x00000F46
#define DSP_AUDIO_VCFGR_R_TYPE                                            UInt16
#define DSP_AUDIO_VCFGR_R_RESERVED_MASK                                   0x0000FF00
#define    DSP_AUDIO_VCFGR_R_VCFGR_SHIFT                                  0
#define    DSP_AUDIO_VCFGR_R_VCFGR_MASK                                   0x000000FF

#define DSP_AUDIO_AUDIR_R_OFFSET                                          0x00000F48
#define DSP_AUDIO_AUDIR_R_TYPE                                            UInt16
#define DSP_AUDIO_AUDIR_R_RESERVED_MASK                                   0x00000000
#define    DSP_AUDIO_AUDIR_R_AUDIR_SHIFT                                  0
#define    DSP_AUDIO_AUDIR_R_AUDIR_MASK                                   0x0000FFFF

#define DSP_AUDIO_AUDOR_R_OFFSET                                          0x00000F4A
#define DSP_AUDIO_AUDOR_R_TYPE                                            UInt16
#define DSP_AUDIO_AUDOR_R_RESERVED_MASK                                   0x00000000
#define    DSP_AUDIO_AUDOR_R_AUDOR_SHIFT                                  0
#define    DSP_AUDIO_AUDOR_R_AUDOR_MASK                                   0x0000FFFF

#define DSP_AUDIO_VMUT_R_OFFSET                                           0x00000F50
#define DSP_AUDIO_VMUT_R_TYPE                                             UInt16
#define DSP_AUDIO_VMUT_R_RESERVED_MASK                                    0x0000FFFF

#define DSP_AUDIO_VAFIFOCTRL_R_OFFSET                                     0x00000F52
#define DSP_AUDIO_VAFIFOCTRL_R_TYPE                                       UInt16
#define DSP_AUDIO_VAFIFOCTRL_R_RESERVED_MASK                              0x000000FF
#define    DSP_AUDIO_VAFIFOCTRL_R_VBOFIFOCRL_SHIFT                        15
#define    DSP_AUDIO_VAFIFOCTRL_R_VBOFIFOCRL_MASK                         0x00008000
#define    DSP_AUDIO_VAFIFOCTRL_R_VOFIFOTHRES_SHIFT                       8
#define    DSP_AUDIO_VAFIFOCTRL_R_VOFIFOTHRES_MASK                        0x00007F00

#define DSP_AUDIO_VSLOPGAIN_R_OFFSET                                      0x00000F56
#define DSP_AUDIO_VSLOPGAIN_R_TYPE                                        UInt16
#define DSP_AUDIO_VSLOPGAIN_R_RESERVED_MASK                               0x00000600
#define    DSP_AUDIO_VSLOPGAIN_R_VSLOPGAINEN_SHIFT                        15
#define    DSP_AUDIO_VSLOPGAIN_R_VSLOPGAINEN_MASK                         0x00008000
#define    DSP_AUDIO_VSLOPGAIN_R_VSLOPMOD_SHIFT                           11
#define    DSP_AUDIO_VSLOPGAIN_R_VSLOPMOD_MASK                            0x00007800
#define    DSP_AUDIO_VSLOPGAIN_R_VTARGETGAIN_SHIFT                        0
#define    DSP_AUDIO_VSLOPGAIN_R_VTARGETGAIN_MASK                         0x000001FF

#define DSP_AUDIO_APCTRK_R_OFFSET                                         0x00000F5A
#define DSP_AUDIO_APCTRK_R_TYPE                                           UInt16
#define DSP_AUDIO_APCTRK_R_RESERVED_MASK                                  0x000078FE
#define    DSP_AUDIO_APCTRK_R_SWGAIN_EN_SHIFT                             15
#define    DSP_AUDIO_APCTRK_R_SWGAIN_EN_MASK                              0x00008000
#define    DSP_AUDIO_APCTRK_R_SWGAIN_SHIFT                                8
#define    DSP_AUDIO_APCTRK_R_SWGAIN_MASK                                 0x00000700
#define    DSP_AUDIO_APCTRK_R_DACAP_SHIFT                                 0
#define    DSP_AUDIO_APCTRK_R_DACAP_MASK                                  0x00000001

#define DSP_AUDIO_ADCCONTROL_R_OFFSET                                     0x00000F58
#define DSP_AUDIO_ADCCONTROL_R_TYPE                                       UInt16
#define DSP_AUDIO_ADCCONTROL_R_RESERVED_MASK                              0x00000000
#define    DSP_AUDIO_ADCCONTROL_R_AUDIOINPATH_SEL_SHIFT                   15
#define    DSP_AUDIO_ADCCONTROL_R_AUDIOINPATH_SEL_MASK                    0x00008000
#define    DSP_AUDIO_ADCCONTROL_R_VINPATH_SEL_SHIFT                       14
#define    DSP_AUDIO_ADCCONTROL_R_VINPATH_SEL_MASK                        0x00004000
#define    DSP_AUDIO_ADCCONTROL_R_DIGMIC_CLK_SEL_SHIFT                    13
#define    DSP_AUDIO_ADCCONTROL_R_DIGMIC_CLK_SEL_MASK                     0x00002000
#define    DSP_AUDIO_ADCCONTROL_R_VINPATH2_16K_MODE_SEL_SHIFT             12
#define    DSP_AUDIO_ADCCONTROL_R_VINPATH2_16K_MODE_SEL_MASK              0x00001000
#define    DSP_AUDIO_ADCCONTROL_R_AUDIOIN_BITMODE_SHIFT                   11
#define    DSP_AUDIO_ADCCONTROL_R_AUDIOIN_BITMODE_MASK                    0x00000800
#define    DSP_AUDIO_ADCCONTROL_R_AUDIO_OVF_MODE_SHIFT                    10
#define    DSP_AUDIO_ADCCONTROL_R_AUDIO_OVF_MODE_MASK                     0x00000400
#define    DSP_AUDIO_ADCCONTROL_R_AUDIOIN_INTEN_SHIFT                     9
#define    DSP_AUDIO_ADCCONTROL_R_AUDIOIN_INTEN_MASK                      0x00000200
#define    DSP_AUDIO_ADCCONTROL_R_VINPATH_IIRCM_SEL_SHIFT                 8
#define    DSP_AUDIO_ADCCONTROL_R_VINPATH_IIRCM_SEL_MASK                  0x00000100
#define    DSP_AUDIO_ADCCONTROL_R_DIGIMIC_PH_SEL_SHIFT                    7
#define    DSP_AUDIO_ADCCONTROL_R_DIGIMIC_PH_SEL_MASK                     0x00000080
#define    DSP_AUDIO_ADCCONTROL_R_DIGIMIC_EN_SHIFT                        6
#define    DSP_AUDIO_ADCCONTROL_R_DIGIMIC_EN_MASK                         0x00000040
#define    DSP_AUDIO_ADCCONTROL_R_RXANA2_EN_SHIFT                         5
#define    DSP_AUDIO_ADCCONTROL_R_RXANA2_EN_MASK                          0x00000020
#define    DSP_AUDIO_ADCCONTROL_R_VOICEIN_INTOFFSET_SHIFT                 2
#define    DSP_AUDIO_ADCCONTROL_R_VOICEIN_INTOFFSET_MASK                  0x0000001C
#define    DSP_AUDIO_ADCCONTROL_R_LOOPEN_SHIFT                            1
#define    DSP_AUDIO_ADCCONTROL_R_LOOPEN_MASK                             0x00000002
#define    DSP_AUDIO_ADCCONTROL_R_SW_ADCAP_SHIFT                          0
#define    DSP_AUDIO_ADCCONTROL_R_SW_ADCAP_MASK                           0x00000001

#define DSP_AUDIO_AIR_R_OFFSET                                            0x00000F5C
#define DSP_AUDIO_AIR_R_TYPE                                              UInt16
#define DSP_AUDIO_AIR_R_RESERVED_MASK                                     0x00007FF8
#define    DSP_AUDIO_AIR_R_DAC_EDGE_SHIFT                                 15
#define    DSP_AUDIO_AIR_R_DAC_EDGE_MASK                                  0x00008000
#define    DSP_AUDIO_AIR_R_CCR_TONE_SHIFT                                 2
#define    DSP_AUDIO_AIR_R_CCR_TONE_MASK                                  0x00000004
#define    DSP_AUDIO_AIR_R_CCR_INIT_SHIFT                                 1
#define    DSP_AUDIO_AIR_R_CCR_INIT_MASK                                  0x00000002
#define    DSP_AUDIO_AIR_R_CCR_DIS_SHIFT                                  0
#define    DSP_AUDIO_AIR_R_CCR_DIS_MASK                                   0x00000001

#define DSP_AUDIO_APRR_R_OFFSET                                           0x00000F5E
#define DSP_AUDIO_APRR_R_TYPE                                             UInt16
#define DSP_AUDIO_APRR_R_RESERVED_MASK                                    0x00003F78
#define    DSP_AUDIO_APRR_R_CM_ORID_SHIFT                                 15
#define    DSP_AUDIO_APRR_R_CM_ORID_MASK                                  0x00008000
#define    DSP_AUDIO_APRR_R_RAMP_PD_SHIFT                                 14
#define    DSP_AUDIO_APRR_R_RAMP_PD_MASK                                  0x00004000
#define    DSP_AUDIO_APRR_R_UPDWON_SHIFT                                  7
#define    DSP_AUDIO_APRR_R_UPDWON_MASK                                   0x00000080
#define    DSP_AUDIO_APRR_R_RAMPTIME_SHIFT                                0
#define    DSP_AUDIO_APRR_R_RAMPTIME_MASK                                 0x00000007

#define DSP_AUDIO_LSDMSEEDL_R_OFFSET                                      0x00000FCE
#define DSP_AUDIO_LSDMSEEDL_R_TYPE                                        UInt16
#define DSP_AUDIO_LSDMSEEDL_R_RESERVED_MASK                               0x00000000
#define    DSP_AUDIO_LSDMSEEDL_R_RAMPTIME_SHIFT                           0
#define    DSP_AUDIO_LSDMSEEDL_R_RAMPTIME_MASK                            0x0000FFFF

#define DSP_AUDIO_LSDMSEEDH_R_OFFSET                                      0x00000FD0
#define DSP_AUDIO_LSDMSEEDH_R_TYPE                                        UInt16
#define DSP_AUDIO_LSDMSEEDH_R_RESERVED_MASK                               0x00008000
#define    DSP_AUDIO_LSDMSEEDH_R_RAMPTIME_SHIFT                           0
#define    DSP_AUDIO_LSDMSEEDH_R_RAMPTIME_MASK                            0x00007FFF

#define DSP_AUDIO_LSDMPOLYL_R_OFFSET                                      0x00000FD2
#define DSP_AUDIO_LSDMPOLYL_R_TYPE                                        UInt16
#define DSP_AUDIO_LSDMPOLYL_R_RESERVED_MASK                               0x00000000
#define    DSP_AUDIO_LSDMPOLYL_R_RAMPTIME_SHIFT                           0
#define    DSP_AUDIO_LSDMPOLYL_R_RAMPTIME_MASK                            0x0000FFFF

#define DSP_AUDIO_LSDMPOLYH_R_OFFSET                                      0x00000FD4
#define DSP_AUDIO_LSDMPOLYH_R_TYPE                                        UInt16
#define DSP_AUDIO_LSDMPOLYH_R_RESERVED_MASK                               0x00008000
#define    DSP_AUDIO_LSDMPOLYH_R_RAMPTIME_SHIFT                           0
#define    DSP_AUDIO_LSDMPOLYH_R_RAMPTIME_MASK                            0x00007FFF

#define DSP_AUDIO_RSDMSEEDL_R_OFFSET                                      0x00000FD6
#define DSP_AUDIO_RSDMSEEDL_R_TYPE                                        UInt16
#define DSP_AUDIO_RSDMSEEDL_R_RESERVED_MASK                               0x00000000
#define    DSP_AUDIO_RSDMSEEDL_R_RAMPTIME_SHIFT                           0
#define    DSP_AUDIO_RSDMSEEDL_R_RAMPTIME_MASK                            0x0000FFFF

#define DSP_AUDIO_RSDMSEEDH_R_OFFSET                                      0x00000FD8
#define DSP_AUDIO_RSDMSEEDH_R_TYPE                                        UInt16
#define DSP_AUDIO_RSDMSEEDH_R_RESERVED_MASK                               0x00008000
#define    DSP_AUDIO_RSDMSEEDH_R_RAMPTIME_SHIFT                           0
#define    DSP_AUDIO_RSDMSEEDH_R_RAMPTIME_MASK                            0x00007FFF

#define DSP_AUDIO_RSDMPOLYL_R_OFFSET                                      0x00000FDA
#define DSP_AUDIO_RSDMPOLYL_R_TYPE                                        UInt16
#define DSP_AUDIO_RSDMPOLYL_R_RESERVED_MASK                               0x00000000
#define    DSP_AUDIO_RSDMPOLYL_R_RAMPTIME_SHIFT                           0
#define    DSP_AUDIO_RSDMPOLYL_R_RAMPTIME_MASK                            0x0000FFFF

#define DSP_AUDIO_RSDMPOLYH_R_OFFSET                                      0x00000FDC
#define DSP_AUDIO_RSDMPOLYH_R_TYPE                                        UInt16
#define DSP_AUDIO_RSDMPOLYH_R_RESERVED_MASK                               0x00008000
#define    DSP_AUDIO_RSDMPOLYH_R_RAMPTIME_SHIFT                           0
#define    DSP_AUDIO_RSDMPOLYH_R_RAMPTIME_MASK                            0x00007FFF

#define DSP_AUDIO_SDMDTHER_R_OFFSET                                       0x00000FDE
#define DSP_AUDIO_SDMDTHER_R_TYPE                                         UInt16
#define DSP_AUDIO_SDMDTHER_R_RESERVED_MASK                                0x0000FFF0
#define    DSP_AUDIO_SDMDTHER_R_RDVAR_SHIFT                               3
#define    DSP_AUDIO_SDMDTHER_R_RDVAR_MASK                                0x00000008
#define    DSP_AUDIO_SDMDTHER_R_RDEN_SHIFT                                2
#define    DSP_AUDIO_SDMDTHER_R_RDEN_MASK                                 0x00000004
#define    DSP_AUDIO_SDMDTHER_R_LDVAR_SHIFT                               1
#define    DSP_AUDIO_SDMDTHER_R_LDVAR_MASK                                0x00000002
#define    DSP_AUDIO_SDMDTHER_R_LDEN_SHIFT                                0
#define    DSP_AUDIO_SDMDTHER_R_LDEN_MASK                                 0x00000001

#define DSP_AUDIO_VINPATH_CTRL_OFFSET                                     0x0000E541
#define DSP_AUDIO_VINPATH_CTRL_TYPE                                       UInt16
#define DSP_AUDIO_VINPATH_CTRL_RESERVED_MASK                              0x00000000
#define    DSP_AUDIO_VINPATH_CTRL_VIN_IIR_OUTSEL_SHIFT                    13
#define    DSP_AUDIO_VINPATH_CTRL_VIN_IIR_OUTSEL_MASK                     0x0000E000
#define    DSP_AUDIO_VINPATH_CTRL_VIN_CIC_OUTSEL_SHIFT                    11
#define    DSP_AUDIO_VINPATH_CTRL_VIN_CIC_OUTSEL_MASK                     0x00001800
#define    DSP_AUDIO_VINPATH_CTRL_VIN_CIC_FINESCALE_SHIFT                 3
#define    DSP_AUDIO_VINPATH_CTRL_VIN_CIC_FINESCALE_MASK                  0x000007F8
#define    DSP_AUDIO_VINPATH_CTRL_VIN_INPUTSEL_SHIFT                      1
#define    DSP_AUDIO_VINPATH_CTRL_VIN_INPUTSEL_MASK                       0x00000006
#define    DSP_AUDIO_VINPATH_CTRL_VIN_ENABLE_SHIFT                        0
#define    DSP_AUDIO_VINPATH_CTRL_VIN_ENABLE_MASK                         0x00000001

#define DSP_AUDIO_AUXCOMP_OFFSET                                          0x0000E543
#define DSP_AUDIO_AUXCOMP_TYPE                                            UInt16
#define DSP_AUDIO_AUXCOMP_RESERVED_MASK                                   0x00003FFE
#define    DSP_AUDIO_AUXCOMP_AUXCOMP_SHIFT                                14
#define    DSP_AUDIO_AUXCOMP_AUXCOMP_MASK                                 0x0000C000
#define    DSP_AUDIO_AUXCOMP_AUXCOMP_EN_SHIFT                             0
#define    DSP_AUDIO_AUXCOMP_AUXCOMP_EN_MASK                              0x00000001

#define DSP_AUDIO_AUDIOINPATH_CTRL_OFFSET                                 0x0000E544
#define DSP_AUDIO_AUDIOINPATH_CTRL_TYPE                                   UInt16
#define DSP_AUDIO_AUDIOINPATH_CTRL_RESERVED_MASK                          0x00000000
#define    DSP_AUDIO_AUDIOINPATH_CTRL_AIN_IIR_OUTSEL_SHIFT                13
#define    DSP_AUDIO_AUDIOINPATH_CTRL_AIN_IIR_OUTSEL_MASK                 0x0000E000
#define    DSP_AUDIO_AUDIOINPATH_CTRL_AIN_CIC_OUTSEL_SHIFT                11
#define    DSP_AUDIO_AUDIOINPATH_CTRL_AIN_CIC_OUTSEL_MASK                 0x00001800
#define    DSP_AUDIO_AUDIOINPATH_CTRL_AIN_CIC_FINESCALE_SHIFT             3
#define    DSP_AUDIO_AUDIOINPATH_CTRL_AIN_CIC_FINESCALE_MASK              0x000007F8
#define    DSP_AUDIO_AUDIOINPATH_CTRL_AIN_INPUTSEL_SHIFT                  1
#define    DSP_AUDIO_AUDIOINPATH_CTRL_AIN_INPUTSEL_MASK                   0x00000006
#define    DSP_AUDIO_AUDIOINPATH_CTRL_AIN_ENABLE_SHIFT                    0
#define    DSP_AUDIO_AUDIOINPATH_CTRL_AIN_ENABLE_MASK                     0x00000001

#define DSP_AUDIO_AUDIOLOOPBACK_CTRL_OFFSET                               0x0000E549
#define DSP_AUDIO_AUDIOLOOPBACK_CTRL_TYPE                                 UInt16
#define DSP_AUDIO_AUDIOLOOPBACK_CTRL_RESERVED_MASK                        0x00001F00
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_RXANA1_EN_SHIFT                   15
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_RXANA1_EN_MASK                    0x00008000
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_AUD_IN_DMA_EN_SHIFT               14
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_AUD_IN_DMA_EN_MASK                0x00004000
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_AUDIOIN_FIFO_CLR_SHIFT            13
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_AUDIOIN_FIFO_CLR_MASK             0x00002000
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_AUDIOIN_BITMODE_DAC4_SHIFT        6
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_AUDIOIN_BITMODE_DAC4_MASK         0x000000C0
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_AUDIOIN_BITMODE_DAC3_SHIFT        4
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_AUDIOIN_BITMODE_DAC3_MASK         0x00000030
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_AUDIOIN_BITMODE_DAC2_SHIFT        2
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_AUDIOIN_BITMODE_DAC2_MASK         0x0000000C
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_AUDIOIN_BITMODE_DAC1_SHIFT        0
#define    DSP_AUDIO_AUDIOLOOPBACK_CTRL_AUDIOIN_BITMODE_DAC1_MASK         0x00000003

#define DSP_AUDIO_AUDIOINCM_ADDR_OFFSET                                   0x0000E547
#define DSP_AUDIO_AUDIOINCM_ADDR_TYPE                                     UInt16
#define DSP_AUDIO_AUDIOINCM_ADDR_RESERVED_MASK                            0x0000FF00
#define    DSP_AUDIO_AUDIOINCM_ADDR_AUDIOINCM_ADDR_SHIFT                  0
#define    DSP_AUDIO_AUDIOINCM_ADDR_AUDIOINCM_ADDR_MASK                   0x000000FF

#define DSP_AUDIO_AUDIOINCM_DATA_OFFSET                                   0x0000E548
#define DSP_AUDIO_AUDIOINCM_DATA_TYPE                                     UInt16
#define DSP_AUDIO_AUDIOINCM_DATA_RESERVED_MASK                            0x00008000
#define    DSP_AUDIO_AUDIOINCM_DATA_AUDIOINCM_DATA_SHIFT                  0
#define    DSP_AUDIO_AUDIOINCM_DATA_AUDIOINCM_DATA_MASK                   0x00007FFF

#define DSP_AUDIO_AUDIR2_OFFSET                                           0x0000E7A0
#define DSP_AUDIO_AUDIR2_TYPE                                             UInt16
#define DSP_AUDIO_AUDIR2_RESERVED_MASK                                    0x00000000
#define    DSP_AUDIO_AUDIR2_AUDIR2_SHIFT                                  0
#define    DSP_AUDIO_AUDIR2_AUDIR2_MASK                                   0x0000FFFF

#define DSP_AUDIO_AUDIOINFIFO_STATUS_OFFSET                               0x0000E7A1
#define DSP_AUDIO_AUDIOINFIFO_STATUS_TYPE                                 UInt16
#define DSP_AUDIO_AUDIOINFIFO_STATUS_RESERVED_MASK                        0x00000000
#define    DSP_AUDIO_AUDIOINFIFO_STATUS_AUDIOINFIFO_STATUS_SHIFT          0
#define    DSP_AUDIO_AUDIOINFIFO_STATUS_AUDIOINFIFO_STATUS_MASK           0x0000FFFF

#define DSP_AUDIO_MIXER_INPUT_SEL_OFFSET                                  0x0000E7F8
#define DSP_AUDIO_MIXER_INPUT_SEL_TYPE                                    UInt16
#define DSP_AUDIO_MIXER_INPUT_SEL_RESERVED_MASK                           0x00003FE0
#define    DSP_AUDIO_MIXER_INPUT_SEL_PATHSEL_SHIFT                        14
#define    DSP_AUDIO_MIXER_INPUT_SEL_PATHSEL_MASK                         0x0000C000
#define    DSP_AUDIO_MIXER_INPUT_SEL_INPUT_TO_MIX_SHIFT                   0
#define    DSP_AUDIO_MIXER_INPUT_SEL_INPUT_TO_MIX_MASK                    0x0000001F

#define DSP_AUDIO_MIXER_GAIN_CHSEL_OFFSET                                 0x0000E7F9
#define DSP_AUDIO_MIXER_GAIN_CHSEL_TYPE                                   UInt16
#define DSP_AUDIO_MIXER_GAIN_CHSEL_RESERVED_MASK                          0x0000FFFC
#define    DSP_AUDIO_MIXER_GAIN_CHSEL_MIXER_GAIN_CHSEL_SHIFT              0
#define    DSP_AUDIO_MIXER_GAIN_CHSEL_MIXER_GAIN_CHSEL_MASK               0x00000003

#define DSP_AUDIO_MIXER_GAIN_ADJUST_OFFSET                                0x0000E7FA
#define DSP_AUDIO_MIXER_GAIN_ADJUST_TYPE                                  UInt16
#define DSP_AUDIO_MIXER_GAIN_ADJUST_RESERVED_MASK                         0x00000000
#define    DSP_AUDIO_MIXER_GAIN_ADJUST_MIXER_GAIN_ADJUST_SHIFT            0
#define    DSP_AUDIO_MIXER_GAIN_ADJUST_MIXER_GAIN_ADJUST_MASK             0x0000FFFF

#define DSP_AUDIO_BIQUAD_CFG_OFFSET                                       0x0000E7FB
#define DSP_AUDIO_BIQUAD_CFG_TYPE                                         UInt16
#define DSP_AUDIO_BIQUAD_CFG_RESERVED_MASK                                0x0000200C
#define    DSP_AUDIO_BIQUAD_CFG_CHSEL_SHIFT                               14
#define    DSP_AUDIO_BIQUAD_CFG_CHSEL_MASK                                0x0000C000
#define    DSP_AUDIO_BIQUAD_CFG_MPMBIQUAD_OUTSEL_SHIFT                    8
#define    DSP_AUDIO_BIQUAD_CFG_MPMBIQUAD_OUTSEL_MASK                     0x00001F00
#define    DSP_AUDIO_BIQUAD_CFG_MPMBIQUAD_NUM_SHIFT                       4
#define    DSP_AUDIO_BIQUAD_CFG_MPMBIQUAD_NUM_MASK                        0x000000F0
#define    DSP_AUDIO_BIQUAD_CFG_BIQUAD_CLIP_CTRL_SHIFT                    0
#define    DSP_AUDIO_BIQUAD_CFG_BIQUAD_CLIP_CTRL_MASK                     0x00000003

#define DSP_AUDIO_MPM_FORCEOFF_OFFSET                                     0x0000E7FC
#define DSP_AUDIO_MPM_FORCEOFF_TYPE                                       UInt16
#define DSP_AUDIO_MPM_FORCEOFF_RESERVED_MASK                              0x0000FFFE
#define    DSP_AUDIO_MPM_FORCEOFF_MPM_FORCEOFF_SHIFT                      0
#define    DSP_AUDIO_MPM_FORCEOFF_MPM_FORCEOFF_MASK                       0x00000001

#define DSP_AUDIO_MPMBIQUAD_COEF_ADDR_OFFSET                              0x0000E7CD
#define DSP_AUDIO_MPMBIQUAD_COEF_ADDR_TYPE                                UInt16
#define DSP_AUDIO_MPMBIQUAD_COEF_ADDR_RESERVED_MASK                       0x0000FF00
#define    DSP_AUDIO_MPMBIQUAD_COEF_ADDR_BIQUAD_CO_ADDR_SHIFT             0
#define    DSP_AUDIO_MPMBIQUAD_COEF_ADDR_BIQUAD_CO_ADDR_MASK              0x000000FF

#define DSP_AUDIO_MPMBIQUAD_COEF_WDATAH_OFFSET                            0x0000E7CE
#define DSP_AUDIO_MPMBIQUAD_COEF_WDATAH_TYPE                              UInt16
#define DSP_AUDIO_MPMBIQUAD_COEF_WDATAH_RESERVED_MASK                     0x00000000
#define    DSP_AUDIO_MPMBIQUAD_COEF_WDATAH_MPMBIQUAD_COEF_WDATA_BIT21_6_SHIFT 0
#define    DSP_AUDIO_MPMBIQUAD_COEF_WDATAH_MPMBIQUAD_COEF_WDATA_BIT21_6_MASK 0x0000FFFF

#define DSP_AUDIO_MPMBIQUAD_COEF_WDATAL_OFFSET                            0x0000E7CF
#define DSP_AUDIO_MPMBIQUAD_COEF_WDATAL_TYPE                              UInt16
#define DSP_AUDIO_MPMBIQUAD_COEF_WDATAL_RESERVED_MASK                     0x0000FFC0
#define    DSP_AUDIO_MPMBIQUAD_COEF_WDATAL_MPMBIQUAD_COEF_WDATA_BIT5_0_SHIFT 0
#define    DSP_AUDIO_MPMBIQUAD_COEF_WDATAL_MPMBIQUAD_COEF_WDATA_BIT5_0_MASK 0x0000003F

#define DSP_AUDIO_POLYAUDMOD_OFFSET                                       0x0000E7B0
#define DSP_AUDIO_POLYAUDMOD_TYPE                                         UInt16
#define DSP_AUDIO_POLYAUDMOD_RESERVED_MASK                                0x0000001A
#define    DSP_AUDIO_POLYAUDMOD_POLY_18BIT_MODE_SHIFT                     15
#define    DSP_AUDIO_POLYAUDMOD_POLY_18BIT_MODE_MASK                      0x00008000
#define    DSP_AUDIO_POLYAUDMOD_POLY_OUT_DMA_EN_SHIFT                     14
#define    DSP_AUDIO_POLYAUDMOD_POLY_OUT_DMA_EN_MASK                      0x00004000
#define    DSP_AUDIO_POLYAUDMOD_PSAMPRATE_SHIFT                           10
#define    DSP_AUDIO_POLYAUDMOD_PSAMPRATE_MASK                            0x00003C00
#define    DSP_AUDIO_POLYAUDMOD_PRATEADP_SHIFT                            8
#define    DSP_AUDIO_POLYAUDMOD_PRATEADP_MASK                             0x00000300
#define    DSP_AUDIO_POLYAUDMOD_PHBIFMOD_SHIFT                            5
#define    DSP_AUDIO_POLYAUDMOD_PHBIFMOD_MASK                             0x000000E0
#define    DSP_AUDIO_POLYAUDMOD_PCOMPMOD_SHIFT                            2
#define    DSP_AUDIO_POLYAUDMOD_PCOMPMOD_MASK                             0x00000004
#define    DSP_AUDIO_POLYAUDMOD_PLYEN_SHIFT                               0
#define    DSP_AUDIO_POLYAUDMOD_PLYEN_MASK                                0x00000001

#define DSP_AUDIO_PFIFOCTRL_OFFSET                                        0x0000E7B1
#define DSP_AUDIO_PFIFOCTRL_TYPE                                          UInt16
#define DSP_AUDIO_PFIFOCTRL_RESERVED_MASK                                 0x00000000
#define    DSP_AUDIO_PFIFOCTRL_POFIFOCRL0_SHIFT                           15
#define    DSP_AUDIO_PFIFOCTRL_POFIFOCRL0_MASK                            0x00008000
#define    DSP_AUDIO_PFIFOCTRL_POFIFOTHRES0_SHIFT                         8
#define    DSP_AUDIO_PFIFOCTRL_POFIFOTHRES0_MASK                          0x00007F00
#define    DSP_AUDIO_PFIFOCTRL_PIFIFOCRL1_SHIFT                           7
#define    DSP_AUDIO_PFIFOCTRL_PIFIFOCRL1_MASK                            0x00000080
#define    DSP_AUDIO_PFIFOCTRL_PIFIFOTHRES1_SHIFT                         0
#define    DSP_AUDIO_PFIFOCTRL_PIFIFOTHRES1_MASK                          0x0000007F

#define DSP_AUDIO_PIFIFOST_OFFSET                                         0x0000E7B2
#define DSP_AUDIO_PIFIFOST_TYPE                                           UInt16
#define DSP_AUDIO_PIFIFOST_RESERVED_MASK                                  0x0000F800
#define    DSP_AUDIO_PIFIFOST_PIFIFOOVF_SHIFT                             10
#define    DSP_AUDIO_PIFIFOST_PIFIFOOVF_MASK                              0x00000400
#define    DSP_AUDIO_PIFIFOST_PIFIFOUDF_SHIFT                             9
#define    DSP_AUDIO_PIFIFOST_PIFIFOUDF_MASK                              0x00000200
#define    DSP_AUDIO_PIFIFOST_PIFIFOTHMET_SHIFT                           8
#define    DSP_AUDIO_PIFIFOST_PIFIFOTHMET_MASK                            0x00000100
#define    DSP_AUDIO_PIFIFOST_PIFIFOEMTRYCUNT_SHIFT                       0
#define    DSP_AUDIO_PIFIFOST_PIFIFOEMTRYCUNT_MASK                        0x000000FF

#define DSP_AUDIO_PLRCH_OFFSET                                            0x0000E7B3
#define DSP_AUDIO_PLRCH_TYPE                                              UInt16
#define DSP_AUDIO_PLRCH_RESERVED_MASK                                     0x0000FFF0
#define    DSP_AUDIO_PLRCH_PLCHMOD_SHIFT                                  2
#define    DSP_AUDIO_PLRCH_PLCHMOD_MASK                                   0x0000000C
#define    DSP_AUDIO_PLRCH_PRCHMOD_SHIFT                                  0
#define    DSP_AUDIO_PLRCH_PRCHMOD_MASK                                   0x00000003

#define DSP_AUDIO_PEQPATHOFST0_OFFSET                                     0x0000E7B4
#define DSP_AUDIO_PEQPATHOFST0_TYPE                                       UInt16
#define DSP_AUDIO_PEQPATHOFST0_RESERVED_MASK                              0x00000000
#define    DSP_AUDIO_PEQPATHOFST0_PPATH2DELAY_SHIFT                       14
#define    DSP_AUDIO_PEQPATHOFST0_PPATH2DELAY_MASK                        0x0000C000
#define    DSP_AUDIO_PEQPATHOFST0_PPATH1DELAY_SHIFT                       7
#define    DSP_AUDIO_PEQPATHOFST0_PPATH1DELAY_MASK                        0x00003F80
#define    DSP_AUDIO_PEQPATHOFST0_PPATH0DELAY_SHIFT                       0
#define    DSP_AUDIO_PEQPATHOFST0_PPATH0DELAY_MASK                        0x0000007F

#define DSP_AUDIO_PEQPATHOFST1_OFFSET                                     0x0000E7B5
#define DSP_AUDIO_PEQPATHOFST1_TYPE                                       UInt16
#define DSP_AUDIO_PEQPATHOFST1_RESERVED_MASK                              0x00000000
#define    DSP_AUDIO_PEQPATHOFST1_PPATH4DELAY_SHIFT                       12
#define    DSP_AUDIO_PEQPATHOFST1_PPATH4DELAY_MASK                        0x0000F000
#define    DSP_AUDIO_PEQPATHOFST1_PPATH3DELAY_SHIFT                       5
#define    DSP_AUDIO_PEQPATHOFST1_PPATH3DELAY_MASK                        0x00000FE0
#define    DSP_AUDIO_PEQPATHOFST1_PPATH2DELAY_SHIFT                       0
#define    DSP_AUDIO_PEQPATHOFST1_PPATH2DELAY_MASK                        0x0000001F

#define DSP_AUDIO_PEQPATHOFST2_OFFSET                                     0x0000E7B6
#define DSP_AUDIO_PEQPATHOFST2_TYPE                                       UInt16
#define DSP_AUDIO_PEQPATHOFST2_RESERVED_MASK                              0x0000FC00
#define    DSP_AUDIO_PEQPATHOFST2_PPATH5DELAY_SHIFT                       3
#define    DSP_AUDIO_PEQPATHOFST2_PPATH5DELAY_MASK                        0x000003F8
#define    DSP_AUDIO_PEQPATHOFST2_PPATH4DELAY_SHIFT                       0
#define    DSP_AUDIO_PEQPATHOFST2_PPATH4DELAY_MASK                        0x00000007

#define DSP_AUDIO_PLSLOPGAIN_OFFSET                                       0x0000E7B8
#define DSP_AUDIO_PLSLOPGAIN_TYPE                                         UInt16
#define DSP_AUDIO_PLSLOPGAIN_RESERVED_MASK                                0x00000600
#define    DSP_AUDIO_PLSLOPGAIN_PLSLOPGAINEN_SHIFT                        15
#define    DSP_AUDIO_PLSLOPGAIN_PLSLOPGAINEN_MASK                         0x00008000
#define    DSP_AUDIO_PLSLOPGAIN_PLSLOPMOD_SHIFT                           11
#define    DSP_AUDIO_PLSLOPGAIN_PLSLOPMOD_MASK                            0x00007800
#define    DSP_AUDIO_PLSLOPGAIN_PLTARGETGAIN_SHIFT                        0
#define    DSP_AUDIO_PLSLOPGAIN_PLTARGETGAIN_MASK                         0x000001FF

#define DSP_AUDIO_PRSLOPGAIN_OFFSET                                       0x0000E7B9
#define DSP_AUDIO_PRSLOPGAIN_TYPE                                         UInt16
#define DSP_AUDIO_PRSLOPGAIN_RESERVED_MASK                                0x00000600
#define    DSP_AUDIO_PRSLOPGAIN_PRSLOPGAINEN_SHIFT                        15
#define    DSP_AUDIO_PRSLOPGAIN_PRSLOPGAINEN_MASK                         0x00008000
#define    DSP_AUDIO_PRSLOPGAIN_PRSLOPMOD_SHIFT                           11
#define    DSP_AUDIO_PRSLOPGAIN_PRSLOPMOD_MASK                            0x00007800
#define    DSP_AUDIO_PRSLOPGAIN_PRTARGETGAIN_SHIFT                        0
#define    DSP_AUDIO_PRSLOPGAIN_PRTARGETGAIN_MASK                         0x000001FF

#define DSP_AUDIO_PEQPATHGAIN1_OFFSET                                     0x0000E7BC
#define DSP_AUDIO_PEQPATHGAIN1_TYPE                                       UInt16
#define DSP_AUDIO_PEQPATHGAIN1_RESERVED_MASK                              0x00000600
#define    DSP_AUDIO_PEQPATHGAIN1_PEQPATHGAIN1EN_SHIFT                    15
#define    DSP_AUDIO_PEQPATHGAIN1_PEQPATHGAIN1EN_MASK                     0x00008000
#define    DSP_AUDIO_PEQPATHGAIN1_EQGAINSLOP_SHIFT                        11
#define    DSP_AUDIO_PEQPATHGAIN1_EQGAINSLOP_MASK                         0x00007800
#define    DSP_AUDIO_PEQPATHGAIN1_PEQPATHGAIN1_SHIFT                      0
#define    DSP_AUDIO_PEQPATHGAIN1_PEQPATHGAIN1_MASK                       0x000001FF

#define DSP_AUDIO_PEQPATHGAIN2_OFFSET                                     0x0000E7BD
#define DSP_AUDIO_PEQPATHGAIN2_TYPE                                       UInt16
#define DSP_AUDIO_PEQPATHGAIN2_RESERVED_MASK                              0x0000FE00
#define    DSP_AUDIO_PEQPATHGAIN2_PEQPATHGAIN2_SHIFT                      0
#define    DSP_AUDIO_PEQPATHGAIN2_PEQPATHGAIN2_MASK                       0x000001FF

#define DSP_AUDIO_PEQPATHGAIN3_OFFSET                                     0x0000E7BE
#define DSP_AUDIO_PEQPATHGAIN3_TYPE                                       UInt16
#define DSP_AUDIO_PEQPATHGAIN3_RESERVED_MASK                              0x0000FE00
#define    DSP_AUDIO_PEQPATHGAIN3_PEQPATHGAIN3_SHIFT                      0
#define    DSP_AUDIO_PEQPATHGAIN3_PEQPATHGAIN3_MASK                       0x000001FF

#define DSP_AUDIO_PEQPATHGAIN4_OFFSET                                     0x0000E7BF
#define DSP_AUDIO_PEQPATHGAIN4_TYPE                                       UInt16
#define DSP_AUDIO_PEQPATHGAIN4_RESERVED_MASK                              0x0000FE00
#define    DSP_AUDIO_PEQPATHGAIN4_PEQPATHGAIN4_SHIFT                      0
#define    DSP_AUDIO_PEQPATHGAIN4_PEQPATHGAIN4_MASK                       0x000001FF

#define DSP_AUDIO_PEQPATHGAIN5_OFFSET                                     0x0000E7B7
#define DSP_AUDIO_PEQPATHGAIN5_TYPE                                       UInt16
#define DSP_AUDIO_PEQPATHGAIN5_RESERVED_MASK                              0x0000FE00
#define    DSP_AUDIO_PEQPATHGAIN5_PEQPATHGAIN5_SHIFT                      0
#define    DSP_AUDIO_PEQPATHGAIN5_PEQPATHGAIN5_MASK                       0x000001FF

#define DSP_AUDIO_PIFIFODATA0_OFFSET                                      0x0000E300
#define DSP_AUDIO_PIFIFODATA0_TYPE                                        UInt32
#define DSP_AUDIO_PIFIFODATA0_RESERVED_MASK                               0x00000000
#define    DSP_AUDIO_PIFIFODATA0_BTW_DATAR_SHIFT                          16
#define    DSP_AUDIO_PIFIFODATA0_BTW_DATAR_MASK                           0xFFFF0000
#define    DSP_AUDIO_PIFIFODATA0_BTW_DATAL_SHIFT                          4
#define    DSP_AUDIO_PIFIFODATA0_BTW_DATAL_MASK                           0x0000FFF0
#define    DSP_AUDIO_PIFIFODATA0_PIFIFODATAR_SHIFT                        2
#define    DSP_AUDIO_PIFIFODATA0_PIFIFODATAR_MASK                         0x0000000C
#define    DSP_AUDIO_PIFIFODATA0_PIFIFODATAL_SHIFT                        0
#define    DSP_AUDIO_PIFIFODATA0_PIFIFODATAL_MASK                         0x00000003

#define DSP_AUDIO_PIFIFODATA1_OFFSET                                      0x0000E302
#define DSP_AUDIO_PIFIFODATA1_TYPE                                        UInt32
#define DSP_AUDIO_PIFIFODATA1_RESERVED_MASK                               0x00000000
#define    DSP_AUDIO_PIFIFODATA1_PIFIFORDATA_SHIFT                        16
#define    DSP_AUDIO_PIFIFODATA1_PIFIFORDATA_MASK                         0xFFFF0000
#define    DSP_AUDIO_PIFIFODATA1_PIFIFOLDATA_SHIFT                        0
#define    DSP_AUDIO_PIFIFODATA1_PIFIFOLDATA_MASK                         0x0000FFFF

#define DSP_AUDIO_PEQCOFADD_OFFSET                                        0x0000E7C4
#define DSP_AUDIO_PEQCOFADD_TYPE                                          UInt16
#define DSP_AUDIO_PEQCOFADD_RESERVED_MASK                                 0x0000FF00
#define    DSP_AUDIO_PEQCOFADD_PEQCOFADD_SHIFT                            0
#define    DSP_AUDIO_PEQCOFADD_PEQCOFADD_MASK                             0x000000FF

#define DSP_AUDIO_PEQCOFDATA_OFFSET                                       0x0000E7C5
#define DSP_AUDIO_PEQCOFDATA_TYPE                                         UInt16
#define DSP_AUDIO_PEQCOFDATA_RESERVED_MASK                                0x00008000
#define    DSP_AUDIO_PEQCOFDATA_PEQCOFDATA_SHIFT                          0
#define    DSP_AUDIO_PEQCOFDATA_PEQCOFDATA_MASK                           0x00007FFF

#define DSP_AUDIO_PCOMPIIRCOFADD_OFFSET                                   0x0000E7C8
#define DSP_AUDIO_PCOMPIIRCOFADD_TYPE                                     UInt16
#define DSP_AUDIO_PCOMPIIRCOFADD_RESERVED_MASK                            0x0000FF00
#define    DSP_AUDIO_PCOMPIIRCOFADD_PCOMPIIRCOFADD_SHIFT                  0
#define    DSP_AUDIO_PCOMPIIRCOFADD_PCOMPIIRCOFADD_MASK                   0x000000FF

#define DSP_AUDIO_PCOMPIIRCOFDATA_OFFSET                                  0x0000E7C9
#define DSP_AUDIO_PCOMPIIRCOFDATA_TYPE                                    UInt16
#define DSP_AUDIO_PCOMPIIRCOFDATA_RESERVED_MASK                           0x00008000
#define    DSP_AUDIO_PCOMPIIRCOFDATA_PCOMPIIRCOFDATA_SHIFT                0
#define    DSP_AUDIO_PCOMPIIRCOFDATA_PCOMPIIRCOFDATA_MASK                 0x00007FFF

#define DSP_AUDIO_PEQEVT_OFFSET                                           0x0000E7CC
#define DSP_AUDIO_PEQEVT_TYPE                                             UInt16
#define DSP_AUDIO_PEQEVT_RESERVED_MASK                                    0x00000000
#define    DSP_AUDIO_PEQEVT_EQEVTREG_SHIFT                                0
#define    DSP_AUDIO_PEQEVT_EQEVTREG_MASK                                 0x0000FFFF

#define DSP_AUDIO_STEREOAUDMOD_OFFSET                                     0x0000E7D0
#define DSP_AUDIO_STEREOAUDMOD_TYPE                                       UInt16
#define DSP_AUDIO_STEREOAUDMOD_RESERVED_MASK                              0x00000002
#define    DSP_AUDIO_STEREOAUDMOD_AUDIO_18BIT_MODE_SHIFT                  15
#define    DSP_AUDIO_STEREOAUDMOD_AUDIO_18BIT_MODE_MASK                   0x00008000
#define    DSP_AUDIO_STEREOAUDMOD_AUD_OUT_DMA_EN_SHIFT                    14
#define    DSP_AUDIO_STEREOAUDMOD_AUD_OUT_DMA_EN_MASK                     0x00004000
#define    DSP_AUDIO_STEREOAUDMOD_ASAMPRATE_SHIFT                         10
#define    DSP_AUDIO_STEREOAUDMOD_ASAMPRATE_MASK                          0x00003C00
#define    DSP_AUDIO_STEREOAUDMOD_ARATEADP_SHIFT                          8
#define    DSP_AUDIO_STEREOAUDMOD_ARATEADP_MASK                           0x00000300
#define    DSP_AUDIO_STEREOAUDMOD_AHBIFMOD_SHIFT                          5
#define    DSP_AUDIO_STEREOAUDMOD_AHBIFMOD_MASK                           0x000000E0
#define    DSP_AUDIO_STEREOAUDMOD_AUDINTDIS_SHIFT                         4
#define    DSP_AUDIO_STEREOAUDMOD_AUDINTDIS_MASK                          0x00000010
#define    DSP_AUDIO_STEREOAUDMOD_I2SMODE_SHIFT                           3
#define    DSP_AUDIO_STEREOAUDMOD_I2SMODE_MASK                            0x00000008
#define    DSP_AUDIO_STEREOAUDMOD_ACOMPMOD_SHIFT                          2
#define    DSP_AUDIO_STEREOAUDMOD_ACOMPMOD_MASK                           0x00000004
#define    DSP_AUDIO_STEREOAUDMOD_AUDEN_SHIFT                             0
#define    DSP_AUDIO_STEREOAUDMOD_AUDEN_MASK                              0x00000001

#define DSP_AUDIO_AFIFOCTRL_OFFSET                                        0x0000E7D1
#define DSP_AUDIO_AFIFOCTRL_TYPE                                          UInt16
#define DSP_AUDIO_AFIFOCTRL_RESERVED_MASK                                 0x00000000
#define    DSP_AUDIO_AFIFOCTRL_AOFIFOCRL0_SHIFT                           15
#define    DSP_AUDIO_AFIFOCTRL_AOFIFOCRL0_MASK                            0x00008000
#define    DSP_AUDIO_AFIFOCTRL_AOFIFOTHRES0_SHIFT                         8
#define    DSP_AUDIO_AFIFOCTRL_AOFIFOTHRES0_MASK                          0x00007F00
#define    DSP_AUDIO_AFIFOCTRL_AIFIFOCRL1_SHIFT                           7
#define    DSP_AUDIO_AFIFOCTRL_AIFIFOCRL1_MASK                            0x00000080
#define    DSP_AUDIO_AFIFOCTRL_AIFIFOTHRES1_SHIFT                         0
#define    DSP_AUDIO_AFIFOCTRL_AIFIFOTHRES1_MASK                          0x0000007F

#define DSP_AUDIO_AIFIFOST_OFFSET                                         0x0000E7D2
#define DSP_AUDIO_AIFIFOST_TYPE                                           UInt16
#define DSP_AUDIO_AIFIFOST_RESERVED_MASK                                  0x0000F800
#define    DSP_AUDIO_AIFIFOST_AIFIFOOVF_SHIFT                             10
#define    DSP_AUDIO_AIFIFOST_AIFIFOOVF_MASK                              0x00000400
#define    DSP_AUDIO_AIFIFOST_AIFIFOUDF_SHIFT                             9
#define    DSP_AUDIO_AIFIFOST_AIFIFOUDF_MASK                              0x00000200
#define    DSP_AUDIO_AIFIFOST_AIFIFOTHMET_SHIFT                           8
#define    DSP_AUDIO_AIFIFOST_AIFIFOTHMET_MASK                            0x00000100
#define    DSP_AUDIO_AIFIFOST_AIFIFOEMTRYCOUNT_SHIFT                      0
#define    DSP_AUDIO_AIFIFOST_AIFIFOEMTRYCOUNT_MASK                       0x000000FF

#define DSP_AUDIO_ALRCH_OFFSET                                            0x0000E7D3
#define DSP_AUDIO_ALRCH_TYPE                                              UInt16
#define DSP_AUDIO_ALRCH_RESERVED_MASK                                     0x0000FFF0
#define    DSP_AUDIO_ALRCH_ALCHMOD_SHIFT                                  2
#define    DSP_AUDIO_ALRCH_ALCHMOD_MASK                                   0x0000000C
#define    DSP_AUDIO_ALRCH_ARCHMOD_SHIFT                                  0
#define    DSP_AUDIO_ALRCH_ARCHMOD_MASK                                   0x00000003

#define DSP_AUDIO_AEQPATHOFST0_OFFSET                                     0x0000E7D4
#define DSP_AUDIO_AEQPATHOFST0_TYPE                                       UInt16
#define DSP_AUDIO_AEQPATHOFST0_RESERVED_MASK                              0x00000000
#define    DSP_AUDIO_AEQPATHOFST0_APATH2DELAY_SHIFT                       14
#define    DSP_AUDIO_AEQPATHOFST0_APATH2DELAY_MASK                        0x0000C000
#define    DSP_AUDIO_AEQPATHOFST0_APATH1DELAY_SHIFT                       7
#define    DSP_AUDIO_AEQPATHOFST0_APATH1DELAY_MASK                        0x00003F80
#define    DSP_AUDIO_AEQPATHOFST0_APATH0DELAY_SHIFT                       0
#define    DSP_AUDIO_AEQPATHOFST0_APATH0DELAY_MASK                        0x0000007F

#define DSP_AUDIO_AEQPATHOFST1_OFFSET                                     0x0000E7D5
#define DSP_AUDIO_AEQPATHOFST1_TYPE                                       UInt16
#define DSP_AUDIO_AEQPATHOFST1_RESERVED_MASK                              0x00000000
#define    DSP_AUDIO_AEQPATHOFST1_APATH4DELAY_SHIFT                       12
#define    DSP_AUDIO_AEQPATHOFST1_APATH4DELAY_MASK                        0x0000F000
#define    DSP_AUDIO_AEQPATHOFST1_APATH3DELAY_SHIFT                       5
#define    DSP_AUDIO_AEQPATHOFST1_APATH3DELAY_MASK                        0x00000FE0
#define    DSP_AUDIO_AEQPATHOFST1_APATH2DELAY_SHIFT                       0
#define    DSP_AUDIO_AEQPATHOFST1_APATH2DELAY_MASK                        0x0000001F

#define DSP_AUDIO_AEQPATHOFST2_OFFSET                                     0x0000E7D6
#define DSP_AUDIO_AEQPATHOFST2_TYPE                                       UInt16
#define DSP_AUDIO_AEQPATHOFST2_RESERVED_MASK                              0x0000FC00
#define    DSP_AUDIO_AEQPATHOFST2_APATH5DELAY_SHIFT                       3
#define    DSP_AUDIO_AEQPATHOFST2_APATH5DELAY_MASK                        0x000003F8
#define    DSP_AUDIO_AEQPATHOFST2_APATH4DELAY_SHIFT                       0
#define    DSP_AUDIO_AEQPATHOFST2_APATH4DELAY_MASK                        0x00000007

#define DSP_AUDIO_AEQEVT_OFFSET                                           0x0000E7D7
#define DSP_AUDIO_AEQEVT_TYPE                                             UInt16
#define DSP_AUDIO_AEQEVT_RESERVED_MASK                                    0x00000000
#define    DSP_AUDIO_AEQEVT_EQEVTREG_SHIFT                                0
#define    DSP_AUDIO_AEQEVT_EQEVTREG_MASK                                 0x0000FFFF

#define DSP_AUDIO_ALSLOPGAIN_OFFSET                                       0x0000E7D8
#define DSP_AUDIO_ALSLOPGAIN_TYPE                                         UInt16
#define DSP_AUDIO_ALSLOPGAIN_RESERVED_MASK                                0x00000600
#define    DSP_AUDIO_ALSLOPGAIN_ALSLOPGAINEN_SHIFT                        15
#define    DSP_AUDIO_ALSLOPGAIN_ALSLOPGAINEN_MASK                         0x00008000
#define    DSP_AUDIO_ALSLOPGAIN_ALSLOPMOD_SHIFT                           11
#define    DSP_AUDIO_ALSLOPGAIN_ALSLOPMOD_MASK                            0x00007800
#define    DSP_AUDIO_ALSLOPGAIN_ALTARGETGAIN_SHIFT                        0
#define    DSP_AUDIO_ALSLOPGAIN_ALTARGETGAIN_MASK                         0x000001FF

#define DSP_AUDIO_ARSLOPGAIN_OFFSET                                       0x0000E7D9
#define DSP_AUDIO_ARSLOPGAIN_TYPE                                         UInt16
#define DSP_AUDIO_ARSLOPGAIN_RESERVED_MASK                                0x00000600
#define    DSP_AUDIO_ARSLOPGAIN_ARSLOPGAINEN_SHIFT                        15
#define    DSP_AUDIO_ARSLOPGAIN_ARSLOPGAINEN_MASK                         0x00008000
#define    DSP_AUDIO_ARSLOPGAIN_ARSLOPMOD_SHIFT                           11
#define    DSP_AUDIO_ARSLOPGAIN_ARSLOPMOD_MASK                            0x00007800
#define    DSP_AUDIO_ARSLOPGAIN_ARTARGETGAIN_SHIFT                        0
#define    DSP_AUDIO_ARSLOPGAIN_ARTARGETGAIN_MASK                         0x000001FF

#define DSP_AUDIO_BTNBDINL_OFFSET                                         0x0000E7DA
#define DSP_AUDIO_BTNBDINL_TYPE                                           UInt16
#define DSP_AUDIO_BTNBDINL_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_BTNBDINL_BTNBDATAIN_SHIFT                            0
#define    DSP_AUDIO_BTNBDINL_BTNBDATAIN_MASK                             0x0000FFFF

#define DSP_AUDIO_BTNBDINR_OFFSET                                         0x0000E7DB
#define DSP_AUDIO_BTNBDINR_TYPE                                           UInt16
#define DSP_AUDIO_BTNBDINR_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_BTNBDINR_BTNBDATAIN_SHIFT                            0
#define    DSP_AUDIO_BTNBDINR_BTNBDATAIN_MASK                             0x0000FFFF

#define DSP_AUDIO_BTMIXER_CFG_OFFSET                                      0x0000E7DC
#define DSP_AUDIO_BTMIXER_CFG_TYPE                                        UInt16
#define DSP_AUDIO_BTMIXER_CFG_RESERVED_MASK                               0x00000104
#define    DSP_AUDIO_BTMIXER_CFG_BTMIXER_WBFIFOTHRES_SHIFT                9
#define    DSP_AUDIO_BTMIXER_CFG_BTMIXER_WBFIFOTHRES_MASK                 0x0000FE00
#define    DSP_AUDIO_BTMIXER_CFG_BTMIXER_DMA_EN_SHIFT                     7
#define    DSP_AUDIO_BTMIXER_CFG_BTMIXER_DMA_EN_MASK                      0x00000080
#define    DSP_AUDIO_BTMIXER_CFG_BTMIXER_WBINTDIS_SHIFT                   6
#define    DSP_AUDIO_BTMIXER_CFG_BTMIXER_WBINTDIS_MASK                    0x00000040
#define    DSP_AUDIO_BTMIXER_CFG_BTMIXER_WBINTFLAG_SHIFT                  5
#define    DSP_AUDIO_BTMIXER_CFG_BTMIXER_WBINTFLAG_MASK                   0x00000020
#define    DSP_AUDIO_BTMIXER_CFG_WBLRSW_SHIFT                             4
#define    DSP_AUDIO_BTMIXER_CFG_WBLRSW_MASK                              0x00000010
#define    DSP_AUDIO_BTMIXER_CFG_WBRATE_SHIFT                             3
#define    DSP_AUDIO_BTMIXER_CFG_WBRATE_MASK                              0x00000008
#define    DSP_AUDIO_BTMIXER_CFG_WBENABLE_SHIFT                           0
#define    DSP_AUDIO_BTMIXER_CFG_WBENABLE_MASK                            0x00000003

#define DSP_AUDIO_BTMIXER_CFG2_OFFSET                                     0x0000E7DD
#define DSP_AUDIO_BTMIXER_CFG2_TYPE                                       UInt16
#define DSP_AUDIO_BTMIXER_CFG2_RESERVED_MASK                              0x00008000
#define    DSP_AUDIO_BTMIXER_CFG2_BTMIXER_WBENTRY_SHIFT                   8
#define    DSP_AUDIO_BTMIXER_CFG2_BTMIXER_WBENTRY_MASK                    0x00007F00
#define    DSP_AUDIO_BTMIXER_CFG2_PATH_SEL_SHIFT                          6
#define    DSP_AUDIO_BTMIXER_CFG2_PATH_SEL_MASK                           0x000000C0
#define    DSP_AUDIO_BTMIXER_CFG2_PATH_SOURCE_SHIFT                       4
#define    DSP_AUDIO_BTMIXER_CFG2_PATH_SOURCE_MASK                        0x00000030
#define    DSP_AUDIO_BTMIXER_CFG2_BTNB_ENABLE_SHIFT                       3
#define    DSP_AUDIO_BTMIXER_CFG2_BTNB_ENABLE_MASK                        0x00000008
#define    DSP_AUDIO_BTMIXER_CFG2_BTNB_RATE_SHIFT                         2
#define    DSP_AUDIO_BTMIXER_CFG2_BTNB_RATE_MASK                          0x00000004
#define    DSP_AUDIO_BTMIXER_CFG2_BTWBFIFO_UDF_SHIFT                      1
#define    DSP_AUDIO_BTMIXER_CFG2_BTWBFIFO_UDF_MASK                       0x00000002
#define    DSP_AUDIO_BTMIXER_CFG2_BTWBFIFO_OVF_SHIFT                      0
#define    DSP_AUDIO_BTMIXER_CFG2_BTWBFIFO_OVF_MASK                       0x00000001

#define DSP_AUDIO_BTMIXER_GAIN_L_OFFSET                                   0x0000E7DE
#define DSP_AUDIO_BTMIXER_GAIN_L_TYPE                                     UInt16
#define DSP_AUDIO_BTMIXER_GAIN_L_RESERVED_MASK                            0x00000000
#define    DSP_AUDIO_BTMIXER_GAIN_L_BTMIXER_GAIN_L_SHIFT                  0
#define    DSP_AUDIO_BTMIXER_GAIN_L_BTMIXER_GAIN_L_MASK                   0x0000FFFF

#define DSP_AUDIO_BTMIXER_GAIN_R_OFFSET                                   0x0000E7DF
#define DSP_AUDIO_BTMIXER_GAIN_R_TYPE                                     UInt16
#define DSP_AUDIO_BTMIXER_GAIN_R_RESERVED_MASK                            0x00000000
#define    DSP_AUDIO_BTMIXER_GAIN_R_BTMIXER_GAIN_R_SHIFT                  0
#define    DSP_AUDIO_BTMIXER_GAIN_R_BTMIXER_GAIN_R_MASK                   0x0000FFFF

#define DSP_AUDIO_AEQPATHGAIN1_OFFSET                                     0x0000E7E1
#define DSP_AUDIO_AEQPATHGAIN1_TYPE                                       UInt16
#define DSP_AUDIO_AEQPATHGAIN1_RESERVED_MASK                              0x00000600
#define    DSP_AUDIO_AEQPATHGAIN1_AEQPATHGAIN1EN_SHIFT                    15
#define    DSP_AUDIO_AEQPATHGAIN1_AEQPATHGAIN1EN_MASK                     0x00008000
#define    DSP_AUDIO_AEQPATHGAIN1_EQGAINSLOP_SHIFT                        11
#define    DSP_AUDIO_AEQPATHGAIN1_EQGAINSLOP_MASK                         0x00007800
#define    DSP_AUDIO_AEQPATHGAIN1_AEQPATHGAIN1_SHIFT                      0
#define    DSP_AUDIO_AEQPATHGAIN1_AEQPATHGAIN1_MASK                       0x000001FF

#define DSP_AUDIO_AEQPATHGAIN2_OFFSET                                     0x0000E7E2
#define DSP_AUDIO_AEQPATHGAIN2_TYPE                                       UInt16
#define DSP_AUDIO_AEQPATHGAIN2_RESERVED_MASK                              0x0000FE00
#define    DSP_AUDIO_AEQPATHGAIN2_AEQPATHGAIN2_SHIFT                      0
#define    DSP_AUDIO_AEQPATHGAIN2_AEQPATHGAIN2_MASK                       0x000001FF

#define DSP_AUDIO_AEQPATHGAIN3_OFFSET                                     0x0000E7E3
#define DSP_AUDIO_AEQPATHGAIN3_TYPE                                       UInt16
#define DSP_AUDIO_AEQPATHGAIN3_RESERVED_MASK                              0x0000FE00
#define    DSP_AUDIO_AEQPATHGAIN3_AEQPATHGAIN3_SHIFT                      0
#define    DSP_AUDIO_AEQPATHGAIN3_AEQPATHGAIN3_MASK                       0x000001FF

#define DSP_AUDIO_AEQPATHGAIN4_OFFSET                                     0x0000E7E4
#define DSP_AUDIO_AEQPATHGAIN4_TYPE                                       UInt16
#define DSP_AUDIO_AEQPATHGAIN4_RESERVED_MASK                              0x0000FE00
#define    DSP_AUDIO_AEQPATHGAIN4_AEQPATHGAIN4_SHIFT                      0
#define    DSP_AUDIO_AEQPATHGAIN4_AEQPATHGAIN4_MASK                       0x000001FF

#define DSP_AUDIO_AEQPATHGAIN5_OFFSET                                     0x0000E7E5
#define DSP_AUDIO_AEQPATHGAIN5_TYPE                                       UInt16
#define DSP_AUDIO_AEQPATHGAIN5_RESERVED_MASK                              0x0000FE00
#define    DSP_AUDIO_AEQPATHGAIN5_AEQPATHGAIN5_SHIFT                      0
#define    DSP_AUDIO_AEQPATHGAIN5_AEQPATHGAIN5_MASK                       0x000001FF

#define DSP_AUDIO_AUDVOC_ISR_OFFSET                                       0x0000E7F0
#define DSP_AUDIO_AUDVOC_ISR_TYPE                                         UInt16
#define DSP_AUDIO_AUDVOC_ISR_RESERVED_MASK                                0x0000FE20
#define    DSP_AUDIO_AUDVOC_ISR_VOICE_IN_ONLY_INT_SHIFT                   8
#define    DSP_AUDIO_AUDVOC_ISR_VOICE_IN_ONLY_INT_MASK                    0x00000100
#define    DSP_AUDIO_AUDVOC_ISR_BTTAP_NB_INT_SHIFT                        7
#define    DSP_AUDIO_AUDVOC_ISR_BTTAP_NB_INT_MASK                         0x00000080
#define    DSP_AUDIO_AUDVOC_ISR_BTTAP_WB_INT_SHIFT                        6
#define    DSP_AUDIO_AUDVOC_ISR_BTTAP_WB_INT_MASK                         0x00000040
#define    DSP_AUDIO_AUDVOC_ISR_POLYRINGER_INT_SHIFT                      4
#define    DSP_AUDIO_AUDVOC_ISR_POLYRINGER_INT_MASK                       0x00000010
#define    DSP_AUDIO_AUDVOC_ISR_ADC_AUDIO_INT_SHIFT                       3
#define    DSP_AUDIO_AUDVOC_ISR_ADC_AUDIO_INT_MASK                        0x00000008
#define    DSP_AUDIO_AUDVOC_ISR_DAC_AUDIO_INT_SHIFT                       2
#define    DSP_AUDIO_AUDVOC_ISR_DAC_AUDIO_INT_MASK                        0x00000004
#define    DSP_AUDIO_AUDVOC_ISR_VOICE_INT_SHIFT                           1
#define    DSP_AUDIO_AUDVOC_ISR_VOICE_INT_MASK                            0x00000002
#define    DSP_AUDIO_AUDVOC_ISR_PCM_INT_SHIFT                             0
#define    DSP_AUDIO_AUDVOC_ISR_PCM_INT_MASK                              0x00000001

#define DSP_AUDIO_AIFIFODATA0_OFFSET                                      0x0000E200
#define DSP_AUDIO_AIFIFODATA0_TYPE                                        UInt32
#define DSP_AUDIO_AIFIFODATA0_RESERVED_MASK                               0x00000000
#define    DSP_AUDIO_AIFIFODATA0_AIFIFODATAR_SHIFT                        16
#define    DSP_AUDIO_AIFIFODATA0_AIFIFODATAR_MASK                         0xFFFF0000
#define    DSP_AUDIO_AIFIFODATA0_AIFIFODATAL_SHIFT                        0
#define    DSP_AUDIO_AIFIFODATA0_AIFIFODATAL_MASK                         0x0000FFFF

#define DSP_AUDIO_AIFIFODATA1_OFFSET                                      0x0000E202
#define DSP_AUDIO_AIFIFODATA1_TYPE                                        UInt32
#define DSP_AUDIO_AIFIFODATA1_RESERVED_MASK                               0x00000000
#define    DSP_AUDIO_AIFIFODATA1_AIFIFORDATA_SHIFT                        16
#define    DSP_AUDIO_AIFIFODATA1_AIFIFORDATA_MASK                         0xFFFF0000
#define    DSP_AUDIO_AIFIFODATA1_AIFIFOLDATA_SHIFT                        0
#define    DSP_AUDIO_AIFIFODATA1_AIFIFOLDATA_MASK                         0x0000FFFF

#define DSP_AUDIO_AEQCOFADD_OFFSET                                        0x0000E7F4
#define DSP_AUDIO_AEQCOFADD_TYPE                                          UInt16
#define DSP_AUDIO_AEQCOFADD_RESERVED_MASK                                 0x0000FF00
#define    DSP_AUDIO_AEQCOFADD_AEQCOFADD_SHIFT                            0
#define    DSP_AUDIO_AEQCOFADD_AEQCOFADD_MASK                             0x000000FF

#define DSP_AUDIO_AEQCOFDATA_OFFSET                                       0x0000E7F5
#define DSP_AUDIO_AEQCOFDATA_TYPE                                         UInt16
#define DSP_AUDIO_AEQCOFDATA_RESERVED_MASK                                0x00008000
#define    DSP_AUDIO_AEQCOFDATA_AEQCOFDATA_SHIFT                          0
#define    DSP_AUDIO_AEQCOFDATA_AEQCOFDATA_MASK                           0x00007FFF

#define DSP_AUDIO_ACOMPFIRCOFADD_OFFSET                                   0x0000E7F6
#define DSP_AUDIO_ACOMPFIRCOFADD_TYPE                                     UInt16
#define DSP_AUDIO_ACOMPFIRCOFADD_RESERVED_MASK                            0x0000FF00
#define    DSP_AUDIO_ACOMPFIRCOFADD_ACOMPFIRCOFADD_SHIFT                  0
#define    DSP_AUDIO_ACOMPFIRCOFADD_ACOMPFIRCOFADD_MASK                   0x000000FF

#define DSP_AUDIO_ACOMPFIRCOFDATA_OFFSET                                  0x0000E7F7
#define DSP_AUDIO_ACOMPFIRCOFDATA_TYPE                                    UInt16
#define DSP_AUDIO_ACOMPFIRCOFDATA_RESERVED_MASK                           0x00008000
#define    DSP_AUDIO_ACOMPFIRCOFDATA_ACOMPFIRCOFDATA_SHIFT                0
#define    DSP_AUDIO_ACOMPFIRCOFDATA_ACOMPFIRCOFDATA_MASK                 0x00007FFF

#define DSP_AUDIO_MPM_ALGC_ADDRREG_OFFSET                                 0x0000E7FD
#define DSP_AUDIO_MPM_ALGC_ADDRREG_TYPE                                   UInt16
#define DSP_AUDIO_MPM_ALGC_ADDRREG_RESERVED_MASK                          0x00000000
#define    DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_SHIFT                          0
#define    DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_MASK                           0x0000FFFF
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_DGA_G0_ADDR         0x00000000
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_DGA_G1_ADDR         0x00000001
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_DGA_G2_ADDR         0x00000002
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_DGA_G3_ADDR         0x00000003
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_DGA_SLOPE0_ADDR     0x00000004
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_DGA_SLOPE1_ADDR     0x00000005
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_DGA_SLOPE2_ADDR     0x00000006
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_DGA_SLOPE3_ADDR     0x00000007
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_GAIN_ATTACK_STEP0_ADDR 0x00000008
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_GAIN_ATTACK_SLOPE0_ADDR 0x00000009
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_GAIN_DECAY_SLOPE0_ADDR 0x0000000A
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_GAIN_ATTACK_STEP1_ADDR 0x0000000B
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_GAIN_ATTACK_SLOPE1_ADDR 0x0000000C
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_GAIN_DECAY_SLOPE1_ADDR 0x0000000D
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_GAIN_ATTACK_STEP2_ADDR 0x0000000E
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_GAIN_ATTACK_SLOPE2_ADDR 0x0000000F
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_GAIN_DECAY_SLOPE2_ADDR 0x00000010
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_GAIN_ATTACK_STEP3_ADDR 0x00000011
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_GAIN_ATTACK_SLOPE3_ADDR 0x00000012
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_GAIN_DECAY_SLOPE3_ADDR 0x00000013
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_ALDC_NIIR0_ADDR     0x00000014
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_ALDC_NIIR1_ADDR     0x00000015
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_ALDC_NIIR2_ADDR     0x00000016
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_ALDC_NIIR3_ADDR     0x00000017
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_GAIN_ATTACK_THRESHOLD0_ADDR 0x00000018
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_GAIN_ATTACK_THRESHOLD1_ADDR 0x00000019
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_GAIN_ATTACK_THRESHOLD2_ADDR 0x0000001A
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_GAIN_ATTACK_THRESHOLD3_ADDR 0x0000001B
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_GAIN_CTRL_SEL0_ADDR 0x0000001C
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_GAIN_CTRL_SEL1_ADDR 0x0000001D
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_GAIN_CTRL_SEL2_ADDR 0x0000001E
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_GAIN_CTRL_SEL3_ADDR 0x0000001F
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_DGA_SLOPE_EN_ADDR   0x00000020
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_ALDC_EN0_ADDR       0x00000021
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_ALDC_EN1_ADDR       0x00000022
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_ALDC_EN2_ADDR       0x00000023
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_ALDC_EN3_ADDR       0x00000024
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_TEST_CTRL_ADDR      0x00000025
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_TEST_MIXER_INPUT_ADDR 0x00000026
#define       DSP_AUDIO_MPM_ALGC_ADDRREG_ADDR_CMD_MPM_TEST_BUS_OUT_ADDR   0x00000027

#define DSP_AUDIO_MPM_ALGC_DATAHREG_OFFSET                                0x0000E7FE
#define DSP_AUDIO_MPM_ALGC_DATAHREG_TYPE                                  UInt16
#define DSP_AUDIO_MPM_ALGC_DATAHREG_RESERVED_MASK                         0x00000000
#define    DSP_AUDIO_MPM_ALGC_DATAHREG_UNION_SHIFT                        0
#define    DSP_AUDIO_MPM_ALGC_DATAHREG_UNION_MASK                         0x0000FFFF

#define DSP_AUDIO_MPM_ALGC_DATALREG_OFFSET                                0x0000E7FF
#define DSP_AUDIO_MPM_ALGC_DATALREG_TYPE                                  UInt16
#define DSP_AUDIO_MPM_ALGC_DATALREG_RESERVED_MASK                         0x00000000
#define    DSP_AUDIO_MPM_ALGC_DATALREG_UNION_SHIFT                        0
#define    DSP_AUDIO_MPM_ALGC_DATALREG_UNION_MASK                         0x0000FFFF

#define DSP_AUDIO_AMCR_OFFSET                                             0x0000E540
#define DSP_AUDIO_AMCR_TYPE                                               UInt16
#define DSP_AUDIO_AMCR_RESERVED_MASK                                      0x00003F18
#define    DSP_AUDIO_AMCR_DAISTAT_SHIFT                                   15
#define    DSP_AUDIO_AMCR_DAISTAT_MASK                                    0x00008000
#define    DSP_AUDIO_AMCR_PCMEN_SHIFT                                     14
#define    DSP_AUDIO_AMCR_PCMEN_MASK                                      0x00004000
#define    DSP_AUDIO_AMCR_CRAMSEL_SHIFT                                   7
#define    DSP_AUDIO_AMCR_CRAMSEL_MASK                                    0x00000080
#define    DSP_AUDIO_AMCR_MODE_16K_SHIFT                                  6
#define    DSP_AUDIO_AMCR_MODE_16K_MASK                                   0x00000040
#define    DSP_AUDIO_AMCR_AUDEN_SHIFT                                     5
#define    DSP_AUDIO_AMCR_AUDEN_MASK                                      0x00000020
#define    DSP_AUDIO_AMCR_DLOOPBACK_SHIFT                                 2
#define    DSP_AUDIO_AMCR_DLOOPBACK_MASK                                  0x00000004
#define    DSP_AUDIO_AMCR_AUDT_SHIFT                                      0
#define    DSP_AUDIO_AMCR_AUDT_MASK                                       0x00000003

#define DSP_AUDIO_VOICEFIFO_THRES_OFFSET                                  0x0000E545
#define DSP_AUDIO_VOICEFIFO_THRES_TYPE                                    UInt16
#define DSP_AUDIO_VOICEFIFO_THRES_RESERVED_MASK                           0x000088F8
#define    DSP_AUDIO_VOICEFIFO_THRES_BTNBFIFO_THRES_SHIFT                 12
#define    DSP_AUDIO_VOICEFIFO_THRES_BTNBFIFO_THRES_MASK                  0x00007000
#define    DSP_AUDIO_VOICEFIFO_THRES_VIFIFO_THRES_SHIFT                   8
#define    DSP_AUDIO_VOICEFIFO_THRES_VIFIFO_THRES_MASK                    0x00000700
#define    DSP_AUDIO_VOICEFIFO_THRES_VOINPUTFIFO_THRES_SHIFT              0
#define    DSP_AUDIO_VOICEFIFO_THRES_VOINPUTFIFO_THRES_MASK               0x00000007

#define DSP_AUDIO_VOICEFIFO_STATUS_OFFSET                                 0x0000E54C
#define DSP_AUDIO_VOICEFIFO_STATUS_TYPE                                   UInt16
#define DSP_AUDIO_VOICEFIFO_STATUS_RESERVED_MASK                          0x000000F0
#define    DSP_AUDIO_VOICEFIFO_STATUS_BTNBFIFO_ENTRY_SHIFT                12
#define    DSP_AUDIO_VOICEFIFO_STATUS_BTNBFIFO_ENTRY_MASK                 0x0000F000
#define    DSP_AUDIO_VOICEFIFO_STATUS_VIFIFO_ENTRY_SHIFT                  8
#define    DSP_AUDIO_VOICEFIFO_STATUS_VIFIFO_ENTRY_MASK                   0x00000F00
#define    DSP_AUDIO_VOICEFIFO_STATUS_VOINPUTFIFO_EMPTYCOUNT_SHIFT        0
#define    DSP_AUDIO_VOICEFIFO_STATUS_VOINPUTFIFO_EMPTYCOUNT_MASK         0x0000000F

#define DSP_AUDIO_VOICEFIFO_CLEAR_OFFSET                                  0x0000E54A
#define DSP_AUDIO_VOICEFIFO_CLEAR_TYPE                                    UInt16
#define DSP_AUDIO_VOICEFIFO_CLEAR_RESERVED_MASK                           0x0000FFF0
#define    DSP_AUDIO_VOICEFIFO_CLEAR_PCMFIFO_CLR_SHIFT                    3
#define    DSP_AUDIO_VOICEFIFO_CLEAR_PCMFIFO_CLR_MASK                     0x00000008
#define    DSP_AUDIO_VOICEFIFO_CLEAR_BTNBFIFO_CLR_SHIFT                   2
#define    DSP_AUDIO_VOICEFIFO_CLEAR_BTNBFIFO_CLR_MASK                    0x00000004
#define    DSP_AUDIO_VOICEFIFO_CLEAR_VIFIFO_CLR_SHIFT                     1
#define    DSP_AUDIO_VOICEFIFO_CLEAR_VIFIFO_CLR_MASK                      0x00000002
#define    DSP_AUDIO_VOICEFIFO_CLEAR_VOINPUTFIFO_CLR_SHIFT                0
#define    DSP_AUDIO_VOICEFIFO_CLEAR_VOINPUTFIFO_CLR_MASK                 0x00000001

#define DSP_AUDIO_PCMFIFO_CONTROL_STATUS_OFFSET                           0x0000E54B
#define DSP_AUDIO_PCMFIFO_CONTROL_STATUS_TYPE                             UInt16
#define DSP_AUDIO_PCMFIFO_CONTROL_STATUS_RESERVED_MASK                    0x00000088
#define    DSP_AUDIO_PCMFIFO_CONTROL_STATUS_PCMTXFIFO_EMPTYCOUNT_SHIFT    12
#define    DSP_AUDIO_PCMFIFO_CONTROL_STATUS_PCMTXFIFO_EMPTYCOUNT_MASK     0x0000F000
#define    DSP_AUDIO_PCMFIFO_CONTROL_STATUS_PCMRXFIFO_ENTRY_SHIFT         8
#define    DSP_AUDIO_PCMFIFO_CONTROL_STATUS_PCMRXFIFO_ENTRY_MASK          0x00000F00
#define    DSP_AUDIO_PCMFIFO_CONTROL_STATUS_PCMTXFIFO_THRES_SHIFT         4
#define    DSP_AUDIO_PCMFIFO_CONTROL_STATUS_PCMTXFIFO_THRES_MASK          0x00000070
#define    DSP_AUDIO_PCMFIFO_CONTROL_STATUS_PCMRXFIFO_THRES_SHIFT         0
#define    DSP_AUDIO_PCMFIFO_CONTROL_STATUS_PCMRXFIFO_THRES_MASK          0x00000007

#define DSP_AUDIO_VCOEFR0_OFFSET                                          0x0000E5D0
#define DSP_AUDIO_VCOEFR0_TYPE                                            UInt16
#define DSP_AUDIO_VCOEFR0_RESERVED_MASK                                   0x00000000
#define    DSP_AUDIO_VCOEFR0_VOICEIIRCOEF_SHIFT                           0
#define    DSP_AUDIO_VCOEFR0_VOICEIIRCOEF_MASK                            0x0000FFFF

#define DSP_AUDIO_VCOEFR1_OFFSET                                          0x0000E5D1
#define DSP_AUDIO_VCOEFR1_TYPE                                            UInt16
#define DSP_AUDIO_VCOEFR1_RESERVED_MASK                                   0x00000000
#define    DSP_AUDIO_VCOEFR1_VOICEIIRCOEF_SHIFT                           0
#define    DSP_AUDIO_VCOEFR1_VOICEIIRCOEF_MASK                            0x0000FFFF

#define DSP_AUDIO_VCOEFR2_OFFSET                                          0x0000E5D2
#define DSP_AUDIO_VCOEFR2_TYPE                                            UInt16
#define DSP_AUDIO_VCOEFR2_RESERVED_MASK                                   0x00000000
#define    DSP_AUDIO_VCOEFR2_VOICEIIRCOEF_SHIFT                           0
#define    DSP_AUDIO_VCOEFR2_VOICEIIRCOEF_MASK                            0x0000FFFF

#define DSP_AUDIO_VCOEFR3_OFFSET                                          0x0000E5D3
#define DSP_AUDIO_VCOEFR3_TYPE                                            UInt16
#define DSP_AUDIO_VCOEFR3_RESERVED_MASK                                   0x00000000
#define    DSP_AUDIO_VCOEFR3_VOICEIIRCOEF_SHIFT                           0
#define    DSP_AUDIO_VCOEFR3_VOICEIIRCOEF_MASK                            0x0000FFFF

#define DSP_AUDIO_VCOEFR4_OFFSET                                          0x0000E5D4
#define DSP_AUDIO_VCOEFR4_TYPE                                            UInt16
#define DSP_AUDIO_VCOEFR4_RESERVED_MASK                                   0x00000000
#define    DSP_AUDIO_VCOEFR4_VOICEIIRCOEF_SHIFT                           0
#define    DSP_AUDIO_VCOEFR4_VOICEIIRCOEF_MASK                            0x0000FFFF

#define DSP_AUDIO_VCOEFR5_OFFSET                                          0x0000E5D5
#define DSP_AUDIO_VCOEFR5_TYPE                                            UInt16
#define DSP_AUDIO_VCOEFR5_RESERVED_MASK                                   0x00000000
#define    DSP_AUDIO_VCOEFR5_VOICEIIRCOEF_SHIFT                           0
#define    DSP_AUDIO_VCOEFR5_VOICEIIRCOEF_MASK                            0x0000FFFF

#define DSP_AUDIO_VCOEFR6_OFFSET                                          0x0000E5D6
#define DSP_AUDIO_VCOEFR6_TYPE                                            UInt16
#define DSP_AUDIO_VCOEFR6_RESERVED_MASK                                   0x00000000
#define    DSP_AUDIO_VCOEFR6_VOICEIIRCOEF_SHIFT                           0
#define    DSP_AUDIO_VCOEFR6_VOICEIIRCOEF_MASK                            0x0000FFFF

#define DSP_AUDIO_VCOEFR7_OFFSET                                          0x0000E5D7
#define DSP_AUDIO_VCOEFR7_TYPE                                            UInt16
#define DSP_AUDIO_VCOEFR7_RESERVED_MASK                                   0x00000000
#define    DSP_AUDIO_VCOEFR7_VOICEIIRCOEF_SHIFT                           0
#define    DSP_AUDIO_VCOEFR7_VOICEIIRCOEF_MASK                            0x0000FFFF

#define DSP_AUDIO_VCOEFR8_OFFSET                                          0x0000E5D8
#define DSP_AUDIO_VCOEFR8_TYPE                                            UInt16
#define DSP_AUDIO_VCOEFR8_RESERVED_MASK                                   0x00000000
#define    DSP_AUDIO_VCOEFR8_VOICEIIRCOEF_SHIFT                           0
#define    DSP_AUDIO_VCOEFR8_VOICEIIRCOEF_MASK                            0x0000FFFF

#define DSP_AUDIO_VCOEFR9_OFFSET                                          0x0000E5D9
#define DSP_AUDIO_VCOEFR9_TYPE                                            UInt16
#define DSP_AUDIO_VCOEFR9_RESERVED_MASK                                   0x00000000
#define    DSP_AUDIO_VCOEFR9_VOICEIIRCOEF_SHIFT                           0
#define    DSP_AUDIO_VCOEFR9_VOICEIIRCOEF_MASK                            0x0000FFFF

#define DSP_AUDIO_VCOEFR10_OFFSET                                         0x0000E5DA
#define DSP_AUDIO_VCOEFR10_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR10_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR10_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR10_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR11_OFFSET                                         0x0000E5DB
#define DSP_AUDIO_VCOEFR11_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR11_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR11_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR11_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR12_OFFSET                                         0x0000E5DC
#define DSP_AUDIO_VCOEFR12_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR12_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR12_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR12_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR13_OFFSET                                         0x0000E5DD
#define DSP_AUDIO_VCOEFR13_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR13_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR13_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR13_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR14_OFFSET                                         0x0000E5DE
#define DSP_AUDIO_VCOEFR14_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR14_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR14_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR14_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR15_OFFSET                                         0x0000E5DF
#define DSP_AUDIO_VCOEFR15_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR15_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR15_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR15_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR16_OFFSET                                         0x0000E5E0
#define DSP_AUDIO_VCOEFR16_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR16_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR16_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR16_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR17_OFFSET                                         0x0000E5E1
#define DSP_AUDIO_VCOEFR17_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR17_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR17_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR17_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR18_OFFSET                                         0x0000E5E2
#define DSP_AUDIO_VCOEFR18_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR18_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR18_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR18_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR19_OFFSET                                         0x0000E5E3
#define DSP_AUDIO_VCOEFR19_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR19_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR19_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR19_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR20_OFFSET                                         0x0000E5E4
#define DSP_AUDIO_VCOEFR20_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR20_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR20_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR20_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR21_OFFSET                                         0x0000E5E5
#define DSP_AUDIO_VCOEFR21_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR21_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR21_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR21_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR22_OFFSET                                         0x0000E5E6
#define DSP_AUDIO_VCOEFR22_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR22_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR22_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR22_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR23_OFFSET                                         0x0000E5E7
#define DSP_AUDIO_VCOEFR23_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR23_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR23_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR23_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR24_OFFSET                                         0x0000E5E8
#define DSP_AUDIO_VCOEFR24_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR24_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR24_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR24_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR25_OFFSET                                         0x0000E5E9
#define DSP_AUDIO_VCOEFR25_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR25_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR25_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR25_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR26_OFFSET                                         0x0000E5EA
#define DSP_AUDIO_VCOEFR26_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR26_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR26_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR26_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR27_OFFSET                                         0x0000E5EB
#define DSP_AUDIO_VCOEFR27_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR27_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR27_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR27_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR28_OFFSET                                         0x0000E5EC
#define DSP_AUDIO_VCOEFR28_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR28_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR28_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR28_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR29_OFFSET                                         0x0000E5ED
#define DSP_AUDIO_VCOEFR29_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR29_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR29_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR29_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR30_OFFSET                                         0x0000E5EE
#define DSP_AUDIO_VCOEFR30_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR30_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR30_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR30_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR31_OFFSET                                         0x0000E5EF
#define DSP_AUDIO_VCOEFR31_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR31_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR31_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR31_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR32_OFFSET                                         0x0000E5F0
#define DSP_AUDIO_VCOEFR32_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR32_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR32_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR32_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR33_OFFSET                                         0x0000E5F1
#define DSP_AUDIO_VCOEFR33_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR33_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR33_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR33_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCOEFR34_OFFSET                                         0x0000E5F2
#define DSP_AUDIO_VCOEFR34_TYPE                                           UInt16
#define DSP_AUDIO_VCOEFR34_RESERVED_MASK                                  0x00000000
#define    DSP_AUDIO_VCOEFR34_VOICEIIRCOEF_SHIFT                          0
#define    DSP_AUDIO_VCOEFR34_VOICEIIRCOEF_MASK                           0x0000FFFF

#define DSP_AUDIO_VCFGR_OFFSET                                            0x0000E7A3
#define DSP_AUDIO_VCFGR_TYPE                                              UInt16
#define DSP_AUDIO_VCFGR_RESERVED_MASK                                     0x0000FF00
#define    DSP_AUDIO_VCFGR_VCFGR_SHIFT                                    0
#define    DSP_AUDIO_VCFGR_VCFGR_MASK                                     0x000000FF

#define DSP_AUDIO_AUDIR_OFFSET                                            0x0000E7A4
#define DSP_AUDIO_AUDIR_TYPE                                              UInt16
#define DSP_AUDIO_AUDIR_RESERVED_MASK                                     0x00000000
#define    DSP_AUDIO_AUDIR_AUDIR_SHIFT                                    0
#define    DSP_AUDIO_AUDIR_AUDIR_MASK                                     0x0000FFFF

#define DSP_AUDIO_AUDOR_OFFSET                                            0x0000E7A5
#define DSP_AUDIO_AUDOR_TYPE                                              UInt16
#define DSP_AUDIO_AUDOR_RESERVED_MASK                                     0x00000000
#define    DSP_AUDIO_AUDOR_AUDOR_SHIFT                                    0
#define    DSP_AUDIO_AUDOR_AUDOR_MASK                                     0x0000FFFF

#define DSP_AUDIO_VMUT_OFFSET                                             0x0000E7A8
#define DSP_AUDIO_VMUT_TYPE                                               UInt16
#define DSP_AUDIO_VMUT_RESERVED_MASK                                      0x0000FFFF

#define DSP_AUDIO_VAFIFOCTRL_OFFSET                                       0x0000E7A9
#define DSP_AUDIO_VAFIFOCTRL_TYPE                                         UInt16
#define DSP_AUDIO_VAFIFOCTRL_RESERVED_MASK                                0x000000FF
#define    DSP_AUDIO_VAFIFOCTRL_VBOFIFOCRL_SHIFT                          15
#define    DSP_AUDIO_VAFIFOCTRL_VBOFIFOCRL_MASK                           0x00008000
#define    DSP_AUDIO_VAFIFOCTRL_VOFIFOTHRES_SHIFT                         8
#define    DSP_AUDIO_VAFIFOCTRL_VOFIFOTHRES_MASK                          0x00007F00

#define DSP_AUDIO_VSLOPGAIN_OFFSET                                        0x0000E7AB
#define DSP_AUDIO_VSLOPGAIN_TYPE                                          UInt16
#define DSP_AUDIO_VSLOPGAIN_RESERVED_MASK                                 0x00000600
#define    DSP_AUDIO_VSLOPGAIN_VSLOPGAINEN_SHIFT                          15
#define    DSP_AUDIO_VSLOPGAIN_VSLOPGAINEN_MASK                           0x00008000
#define    DSP_AUDIO_VSLOPGAIN_VSLOPMOD_SHIFT                             11
#define    DSP_AUDIO_VSLOPGAIN_VSLOPMOD_MASK                              0x00007800
#define    DSP_AUDIO_VSLOPGAIN_VTARGETGAIN_SHIFT                          0
#define    DSP_AUDIO_VSLOPGAIN_VTARGETGAIN_MASK                           0x000001FF

#define DSP_AUDIO_APCTRK_OFFSET                                           0x0000E7AD
#define DSP_AUDIO_APCTRK_TYPE                                             UInt16
#define DSP_AUDIO_APCTRK_RESERVED_MASK                                    0x000078FE
#define    DSP_AUDIO_APCTRK_SWGAIN_EN_SHIFT                               15
#define    DSP_AUDIO_APCTRK_SWGAIN_EN_MASK                                0x00008000
#define    DSP_AUDIO_APCTRK_SWGAIN_SHIFT                                  8
#define    DSP_AUDIO_APCTRK_SWGAIN_MASK                                   0x00000700
#define    DSP_AUDIO_APCTRK_DACAP_SHIFT                                   0
#define    DSP_AUDIO_APCTRK_DACAP_MASK                                    0x00000001

#define DSP_AUDIO_ADCCONTROL_OFFSET                                       0x0000E7AC
#define DSP_AUDIO_ADCCONTROL_TYPE                                         UInt16
#define DSP_AUDIO_ADCCONTROL_RESERVED_MASK                                0x00000000
#define    DSP_AUDIO_ADCCONTROL_AUDIOINPATH_SEL_SHIFT                     15
#define    DSP_AUDIO_ADCCONTROL_AUDIOINPATH_SEL_MASK                      0x00008000
#define    DSP_AUDIO_ADCCONTROL_VINPATH_SEL_SHIFT                         14
#define    DSP_AUDIO_ADCCONTROL_VINPATH_SEL_MASK                          0x00004000
#define    DSP_AUDIO_ADCCONTROL_DIGMIC_CLK_SEL_SHIFT                      13
#define    DSP_AUDIO_ADCCONTROL_DIGMIC_CLK_SEL_MASK                       0x00002000
#define    DSP_AUDIO_ADCCONTROL_VINPATH2_16K_MODE_SEL_SHIFT               12
#define    DSP_AUDIO_ADCCONTROL_VINPATH2_16K_MODE_SEL_MASK                0x00001000
#define    DSP_AUDIO_ADCCONTROL_AUDIOIN_BITMODE_SHIFT                     11
#define    DSP_AUDIO_ADCCONTROL_AUDIOIN_BITMODE_MASK                      0x00000800
#define    DSP_AUDIO_ADCCONTROL_AUDIO_OVF_MODE_SHIFT                      10
#define    DSP_AUDIO_ADCCONTROL_AUDIO_OVF_MODE_MASK                       0x00000400
#define    DSP_AUDIO_ADCCONTROL_AUDIOIN_INTEN_SHIFT                       9
#define    DSP_AUDIO_ADCCONTROL_AUDIOIN_INTEN_MASK                        0x00000200
#define    DSP_AUDIO_ADCCONTROL_VINPATH_IIRCM_SEL_SHIFT                   8
#define    DSP_AUDIO_ADCCONTROL_VINPATH_IIRCM_SEL_MASK                    0x00000100
#define    DSP_AUDIO_ADCCONTROL_DIGIMIC_PH_SEL_SHIFT                      7
#define    DSP_AUDIO_ADCCONTROL_DIGIMIC_PH_SEL_MASK                       0x00000080
#define    DSP_AUDIO_ADCCONTROL_DIGIMIC_EN_SHIFT                          6
#define    DSP_AUDIO_ADCCONTROL_DIGIMIC_EN_MASK                           0x00000040
#define    DSP_AUDIO_ADCCONTROL_RXANA2_EN_SHIFT                           5
#define    DSP_AUDIO_ADCCONTROL_RXANA2_EN_MASK                            0x00000020
#define    DSP_AUDIO_ADCCONTROL_VOICEIN_INTOFFSET_SHIFT                   2
#define    DSP_AUDIO_ADCCONTROL_VOICEIN_INTOFFSET_MASK                    0x0000001C
#define    DSP_AUDIO_ADCCONTROL_LOOPEN_SHIFT                              1
#define    DSP_AUDIO_ADCCONTROL_LOOPEN_MASK                               0x00000002
#define    DSP_AUDIO_ADCCONTROL_SW_ADCAP_SHIFT                            0
#define    DSP_AUDIO_ADCCONTROL_SW_ADCAP_MASK                             0x00000001

#define DSP_AUDIO_AIR_OFFSET                                              0x0000E7AE
#define DSP_AUDIO_AIR_TYPE                                                UInt16
#define DSP_AUDIO_AIR_RESERVED_MASK                                       0x00007FF8
#define    DSP_AUDIO_AIR_DAC_EDGE_SHIFT                                   15
#define    DSP_AUDIO_AIR_DAC_EDGE_MASK                                    0x00008000
#define    DSP_AUDIO_AIR_CCR_TONE_SHIFT                                   2
#define    DSP_AUDIO_AIR_CCR_TONE_MASK                                    0x00000004
#define    DSP_AUDIO_AIR_CCR_INIT_SHIFT                                   1
#define    DSP_AUDIO_AIR_CCR_INIT_MASK                                    0x00000002
#define    DSP_AUDIO_AIR_CCR_DIS_SHIFT                                    0
#define    DSP_AUDIO_AIR_CCR_DIS_MASK                                     0x00000001

#define DSP_AUDIO_APRR_OFFSET                                             0x0000E7AF
#define DSP_AUDIO_APRR_TYPE                                               UInt16
#define DSP_AUDIO_APRR_RESERVED_MASK                                      0x00003F78
#define    DSP_AUDIO_APRR_CM_ORID_SHIFT                                   15
#define    DSP_AUDIO_APRR_CM_ORID_MASK                                    0x00008000
#define    DSP_AUDIO_APRR_RAMP_PD_SHIFT                                   14
#define    DSP_AUDIO_APRR_RAMP_PD_MASK                                    0x00004000
#define    DSP_AUDIO_APRR_UPDWON_SHIFT                                    7
#define    DSP_AUDIO_APRR_UPDWON_MASK                                     0x00000080
#define    DSP_AUDIO_APRR_RAMPTIME_SHIFT                                  0
#define    DSP_AUDIO_APRR_RAMPTIME_MASK                                   0x00000007

#define DSP_AUDIO_LSDMSEEDL_OFFSET                                        0x0000E7E7
#define DSP_AUDIO_LSDMSEEDL_TYPE                                          UInt16
#define DSP_AUDIO_LSDMSEEDL_RESERVED_MASK                                 0x00000000
#define    DSP_AUDIO_LSDMSEEDL_RAMPTIME_SHIFT                             0
#define    DSP_AUDIO_LSDMSEEDL_RAMPTIME_MASK                              0x0000FFFF

#define DSP_AUDIO_LSDMSEEDH_OFFSET                                        0x0000E7E8
#define DSP_AUDIO_LSDMSEEDH_TYPE                                          UInt16
#define DSP_AUDIO_LSDMSEEDH_RESERVED_MASK                                 0x00008000
#define    DSP_AUDIO_LSDMSEEDH_RAMPTIME_SHIFT                             0
#define    DSP_AUDIO_LSDMSEEDH_RAMPTIME_MASK                              0x00007FFF

#define DSP_AUDIO_LSDMPOLYL_OFFSET                                        0x0000E7E9
#define DSP_AUDIO_LSDMPOLYL_TYPE                                          UInt16
#define DSP_AUDIO_LSDMPOLYL_RESERVED_MASK                                 0x00000000
#define    DSP_AUDIO_LSDMPOLYL_RAMPTIME_SHIFT                             0
#define    DSP_AUDIO_LSDMPOLYL_RAMPTIME_MASK                              0x0000FFFF

#define DSP_AUDIO_LSDMPOLYH_OFFSET                                        0x0000E7EA
#define DSP_AUDIO_LSDMPOLYH_TYPE                                          UInt16
#define DSP_AUDIO_LSDMPOLYH_RESERVED_MASK                                 0x00008000
#define    DSP_AUDIO_LSDMPOLYH_RAMPTIME_SHIFT                             0
#define    DSP_AUDIO_LSDMPOLYH_RAMPTIME_MASK                              0x00007FFF

#define DSP_AUDIO_RSDMSEEDL_OFFSET                                        0x0000E7EB
#define DSP_AUDIO_RSDMSEEDL_TYPE                                          UInt16
#define DSP_AUDIO_RSDMSEEDL_RESERVED_MASK                                 0x00000000
#define    DSP_AUDIO_RSDMSEEDL_RAMPTIME_SHIFT                             0
#define    DSP_AUDIO_RSDMSEEDL_RAMPTIME_MASK                              0x0000FFFF

#define DSP_AUDIO_RSDMSEEDH_OFFSET                                        0x0000E7EC
#define DSP_AUDIO_RSDMSEEDH_TYPE                                          UInt16
#define DSP_AUDIO_RSDMSEEDH_RESERVED_MASK                                 0x00008000
#define    DSP_AUDIO_RSDMSEEDH_RAMPTIME_SHIFT                             0
#define    DSP_AUDIO_RSDMSEEDH_RAMPTIME_MASK                              0x00007FFF

#define DSP_AUDIO_RSDMPOLYL_OFFSET                                        0x0000E7ED
#define DSP_AUDIO_RSDMPOLYL_TYPE                                          UInt16
#define DSP_AUDIO_RSDMPOLYL_RESERVED_MASK                                 0x00000000
#define    DSP_AUDIO_RSDMPOLYL_RAMPTIME_SHIFT                             0
#define    DSP_AUDIO_RSDMPOLYL_RAMPTIME_MASK                              0x0000FFFF

#define DSP_AUDIO_RSDMPOLYH_OFFSET                                        0x0000E7EE
#define DSP_AUDIO_RSDMPOLYH_TYPE                                          UInt16
#define DSP_AUDIO_RSDMPOLYH_RESERVED_MASK                                 0x00008000
#define    DSP_AUDIO_RSDMPOLYH_RAMPTIME_SHIFT                             0
#define    DSP_AUDIO_RSDMPOLYH_RAMPTIME_MASK                              0x00007FFF

#define DSP_AUDIO_SDMDTHER_OFFSET                                         0x0000E7EF
#define DSP_AUDIO_SDMDTHER_TYPE                                           UInt16
#define DSP_AUDIO_SDMDTHER_RESERVED_MASK                                  0x0000FFF0
#define    DSP_AUDIO_SDMDTHER_RDVAR_SHIFT                                 3
#define    DSP_AUDIO_SDMDTHER_RDVAR_MASK                                  0x00000008
#define    DSP_AUDIO_SDMDTHER_RDEN_SHIFT                                  2
#define    DSP_AUDIO_SDMDTHER_RDEN_MASK                                   0x00000004
#define    DSP_AUDIO_SDMDTHER_LDVAR_SHIFT                                 1
#define    DSP_AUDIO_SDMDTHER_LDVAR_MASK                                  0x00000002
#define    DSP_AUDIO_SDMDTHER_LDEN_SHIFT                                  0
#define    DSP_AUDIO_SDMDTHER_LDEN_MASK                                   0x00000001

#endif /* __BRCM_RDB_DSP_AUDIO_H__ */


