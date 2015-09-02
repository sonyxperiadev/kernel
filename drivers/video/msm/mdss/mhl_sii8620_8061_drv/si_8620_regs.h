/*

SiI8620 Linux Drive

Copyright (C) 2013 Silicon Image, Inc.

This program is free software; you can redistribute it and/o
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation version 2.
This program is distributed AS-IS WITHOUT ANY WARRANTY of any
kind, whether express or implied; INCLUDING without the implied warranty
of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE or NON-INFRINGEMENT.  See
the GNU General Public License for more details at http://www.gnu.org/licenses/gpl-2.0.html.

*/

/*----------------------------------------------------------------------------*/
/* File Header                                                                */
/*----------------------------------------------------------------------------*/

#define TX_PAGE_0		(0x72)
#define TX_PAGE_1		(0x7A)
#define TX_PAGE_2		(0x92)
#define TX_PAGE_3		(0x9A)
#define TX_PAGE_4		(0xBA)
#define TX_PAGE_5		(0xC8)
#define TX_PAGE_6		(0xB2)
#define TX_PAGE_7		(0xC2)
#define TX_PAGE_8		(0xD2)

/*----------------------------------------------------------------------------*/
/* Registers in TX_PAGE_0                                                     */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* 0x00 Vendor ID Low byte Register                        (Default: 0x01)    */
#define REG_PAGE_0_VND_IDL                                 TX_PAGE_0, 0x00

#define MSK_PAGE_0_VND_IDL_VHDL_IDL                        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x01 Vendor ID High byte Register                       (Default: 0x00)    */
#define REG_PAGE_0_VND_IDH                                 TX_PAGE_0, 0x01

#define MSK_PAGE_0_VND_IDH_VHDL_IDH                        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x02 Device ID Low byte Register                        (Default: 0x60)    */
#define REG_PAGE_0_DEV_IDL                                 TX_PAGE_0, 0x02

#define MSK_PAGE_0_DEV_IDL_DEV_IDL                         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x03 Device ID High byte Register                       (Default: 0x86)    */
#define REG_PAGE_0_DEV_IDH                                 TX_PAGE_0, 0x03

#define MSK_PAGE_0_DEV_IDH_DEV_IDH                         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x04 Device Revision Register                           (Default: 0x00)    */
#define REG_PAGE_0_DEV_REV                                 TX_PAGE_0, 0x04

#define MSK_PAGE_0_DEV_REV_DEV_REV_ID                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x05 Debug Mode Enable Register                         (Default: 0x60)    */
#define REG_PAGE_0_DEBUG_MODE_EN                           TX_PAGE_0, 0x05

#define MSK_PAGE_0_DEBUG_MODE_EN_SI2C_FILTER_SEL           (0x03 << 6)
#define BIT_PAGE_0_DEBUG_MODE_EN_SI2C_SDA_DLY_ON           (0x01 << 5)
#define BIT_PAGE_0_DEBUG_MODE_EN_SI2C_FILTER_TAP_SEL       (0x01 << 4)
#define BIT_PAGE_0_DEBUG_MODE_EN_DBGMODE3_EN               (0x01 << 3)
#define BIT_PAGE_0_DEBUG_MODE_EN_DBGMODE2_EN               (0x01 << 2)
#define BIT_PAGE_0_DEBUG_MODE_EN_SIMODE                    (0x01 << 1)
#define BIT_PAGE_0_DEBUG_MODE_EN_DBGMODE0_EN               (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x06 OTP                                                (Default: 0x08)    */
#define REG_PAGE_0_OTP_DBYTE510                            TX_PAGE_0, 0x06

/*----------------------------------------------------------------------------*/
/* 0x08 System Control #1 Register                         (Default: 0x10)    */
#define REG_PAGE_0_SYS_CTRL1                               TX_PAGE_0, 0x08

#define BIT_PAGE_0_SYS_CTRL1_CBUS125K_SWOVRIDE             (0x01 << 7)
#define BIT_PAGE_0_SYS_CTRL1_VSYNCPIN                      (0x01 << 6)
#define BIT_PAGE_0_SYS_CTRL1_BLOCK_DDC_VIA_UPSTREAM_HPD_LOW              (0x01 << 4)
#define BIT_PAGE_0_SYS_CTRL1_TX_CONTROL_HDMI               (0x01 << 1)
#define BIT_PAGE_0_SYS_CTRL1_CBUS125K_EN                   (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x09 System Status Register                             (Default: 0x00)    */
#define REG_PAGE_0_SYS_STAT                                TX_PAGE_0, 0x09

#define BIT_PAGE_0_SYS_STAT_OTP_ID8620                     (0x01 << 7)
#define MSK_PAGE_0_SYS_STAT_OTP_HDCPADRPMUT                (0x03 << 5)
#define BIT_PAGE_0_SYS_STAT_OTP_HDCPVMUTE                  (0x01 << 4)
#define BIT_PAGE_0_SYS_STAT_MHL3_DOC_MODE                  (0x01 << 3)
#define BIT_PAGE_0_SYS_STAT_RSEN                           (0x01 << 2)
#define BIT_PAGE_0_SYS_STAT_HPDPIN                         (0x01 << 1)
#define BIT_PAGE_0_SYS_STAT_P_STABLE                       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x0A System Control #3 Register                         (Default: 0x00)    */
#define REG_PAGE_0_SYS_CTRL3                               TX_PAGE_0, 0x0A

#define BIT_PAGE_0_SYS_CTRL3_SYS_CNTR                      (0x01 << 5)
#define MSK_PAGE_0_SYS_CTRL3_IO_SETUP_TEST                 (0x03 << 3)
#define MSK_PAGE_0_SYS_CTRL3_CTL                           (0x03 << 1)
#define BIT_PAGE_0_SYS_CTRL3_TDM_LSWRESET                  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x0B System Control DPD                                 (Default: 0x90)    */
#define REG_PAGE_0_DPD                                     TX_PAGE_0, 0x0B

#define BIT_PAGE_0_DPD_PWRON_PLL                           (0x01 << 7)
#define BIT_PAGE_0_DPD_PDNTX12                             (0x01 << 6)
#define BIT_PAGE_0_DPD_PDNRX12                             (0x01 << 5)
#define BIT_PAGE_0_DPD_OSC_EN                              (0x01 << 4)
#define BIT_PAGE_0_DPD_PWRON_HSIC                          (0x01 << 3)
#define BIT_PAGE_0_DPD_PDIDCK_N                            (0x01 << 2)
#define BIT_PAGE_0_DPD_PD_MHL_CLK_N                        (0x01 << 1)

/*----------------------------------------------------------------------------*/
/* 0x0C System Control #4 Register                         (Default: 0x07)    */
#define REG_PAGE_0_SYS_CTRL4                               TX_PAGE_0, 0x0C

#define BIT_PAGE_0_SYS_CTRL4_CTS_FIFO_OF_UF_FLAG           (0x01 << 6)
#define BIT_PAGE_0_SYS_CTRL4_TCF_OF_UF_FLAG                (0x01 << 5)
#define BIT_PAGE_0_SYS_CTRL4_MHL_FIFO_OF_UF_FLAG           (0x01 << 4)
#define BIT_PAGE_0_SYS_CTRL4_CTS_FIFO_AUTO_RST_EN          (0x01 << 2)
#define BIT_PAGE_0_SYS_CTRL4_TCF_AUTO_RST_EN               (0x01 << 1)
#define BIT_PAGE_0_SYS_CTRL4_MHL_FIFO_AUTO_RST_EN          (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x0D Dual link Control Register                         (Default: 0x00)    */
#define REG_PAGE_0_DCTL                                    TX_PAGE_0, 0x0D

#define BIT_PAGE_0_DCTL_TDM_LCLK_PHASE                     (0x01 << 7)
#define BIT_PAGE_0_DCTL_HSIC_CLK_PHASE                     (0x01 << 6)
#define BIT_PAGE_0_DCTL_CTS_TCK_PHASE                      (0x01 << 5)
#define BIT_PAGE_0_DCTL_EXT_DDC_SEL                        (0x01 << 4)
#define BIT_PAGE_0_DCTL_TRANSCODE                          (0x01 << 3)
#define BIT_PAGE_0_DCTL_HSIC_RX_STROBE_PHASE               (0x01 << 2)
#define BIT_PAGE_0_DCTL_TCLKNX_PHASE                       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x0E PWD Software Reset Register                        (Default: 0x00)    */
#define REG_PAGE_0_PWD_SRST                                TX_PAGE_0, 0x0E

#define BIT_PAGE_0_PWD_SRST_CBUS_RST_SW                    (0x01 << 6)
#define BIT_PAGE_0_PWD_SRST_CBUS_RST_SW_EN                 (0x01 << 5)
#define BIT_PAGE_0_PWD_SRST_MHLFIFO_RST                    (0x01 << 4)
#define BIT_PAGE_0_PWD_SRST_CBUS_RST                       (0x01 << 3)
#define BIT_PAGE_0_PWD_SRST_SW_RST_AUTO                    (0x01 << 2)
#define BIT_PAGE_0_PWD_SRST_HDCP2X_SW_RST                  (0x01 << 1)
#define BIT_PAGE_0_PWD_SRST_SW_RST                         (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x0F HDCP Control Register                              (Default: 0x00)    */
#define REG_PAGE_0_HDCP_CTRL                               TX_PAGE_0, 0x0F

#define BIT_PAGE_0_HDCP_CTRL_HDCP_ENC_ON                   (0x01 << 6)
#define BIT_PAGE_0_HDCP_CTRL_HDCP_BKSV_ERR                 (0x01 << 5)
#define BIT_PAGE_0_HDCP_CTRL_RX_RPTR                       (0x01 << 4)
#define BIT_PAGE_0_HDCP_CTRL_AN_STOP                       (0x01 << 3)
#define BIT_PAGE_0_HDCP_CTRL_CP_RESETN                     (0x01 << 2)
#define BIT_PAGE_0_HDCP_CTRL_HDCP_RI_RDY                   (0x01 << 1)
#define BIT_PAGE_0_HDCP_CTRL_ENC_EN                        (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x10 Write BKSV1 Register                               (Default: 0x00)    */
#define REG_PAGE_0_WR_BKSV_1                               TX_PAGE_0, 0x10

#define MSK_PAGE_0_WR_BKSV_1_BKSV0                         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x11 Write BKSV2 Register                               (Default: 0x00)    */
#define REG_PAGE_0_WR_BKSV_2                               TX_PAGE_0, 0x11

#define MSK_PAGE_0_WR_BKSV_2_BKSV1                         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x12 Write BKSV3 Register                               (Default: 0x00)    */
#define REG_PAGE_0_WR_BKSV_3                               TX_PAGE_0, 0x12

#define MSK_PAGE_0_WR_BKSV_3_BKSV2                         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x13 Write BKSV4 Register                               (Default: 0x00)    */
#define REG_PAGE_0_WR_BKSV_4                               TX_PAGE_0, 0x13

#define MSK_PAGE_0_WR_BKSV_4_BKSV3                         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x14 Write BKSV5 Register                               (Default: 0x00)    */
#define REG_PAGE_0_WR_BKSV_5                               TX_PAGE_0, 0x14

#define MSK_PAGE_0_WR_BKSV_5_BKSV4                         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x15 HDCP AN_1 Register                                 (Default: 0x00)    */
#define REG_PAGE_0_AN1                                     TX_PAGE_0, 0x15

#define MSK_PAGE_0_AN1_HDCP_AN0                            (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x16 HDCP AN_2 Register                                 (Default: 0x00)    */
#define REG_PAGE_0_AN2                                     TX_PAGE_0, 0x16

#define MSK_PAGE_0_AN2_HDCP_AN1                            (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x17 HDCP AN_3 Register                                 (Default: 0x00)    */
#define REG_PAGE_0_AN3                                     TX_PAGE_0, 0x17

#define MSK_PAGE_0_AN3_HDCP_AN2                            (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x18 HDCP AN_4 Register                                 (Default: 0x00)    */
#define REG_PAGE_0_AN4                                     TX_PAGE_0, 0x18

#define MSK_PAGE_0_AN4_HDCP_AN3                            (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x19 HDCP AN_5 Register                                 (Default: 0x00)    */
#define REG_PAGE_0_AN5                                     TX_PAGE_0, 0x19

#define MSK_PAGE_0_AN5_HDCP_AN4                            (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x1A HDCP AN_6 Register                                 (Default: 0x00)    */
#define REG_PAGE_0_AN6                                     TX_PAGE_0, 0x1A

#define MSK_PAGE_0_AN6_HDCP_AN5                            (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x1B HDCP AN_7 Register                                 (Default: 0x00)    */
#define REG_PAGE_0_AN7                                     TX_PAGE_0, 0x1B

#define MSK_PAGE_0_AN7_HDCP_AN6                            (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x1C HDCP AN_8 Register                                 (Default: 0x00)    */
#define REG_PAGE_0_AN8                                     TX_PAGE_0, 0x1C

#define MSK_PAGE_0_AN8_HDCP_AN7                            (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x1D AKSV_1 Register                                    (Default: 0x00)    */
#define REG_PAGE_0_AKSV_1                                  TX_PAGE_0, 0x1D

#define MSK_PAGE_0_AKSV_1_AKSV0                            (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x1E AKSV_2 Register                                    (Default: 0x00)    */
#define REG_PAGE_0_AKSV_2                                  TX_PAGE_0, 0x1E

#define MSK_PAGE_0_AKSV_2_AKSV1                            (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x1F AKSV_3 Register                                    (Default: 0x00)    */
#define REG_PAGE_0_AKSV_3                                  TX_PAGE_0, 0x1F

#define MSK_PAGE_0_AKSV_3_AKSV2                            (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x20 AKSV_4 Register                                    (Default: 0x00)    */
#define REG_PAGE_0_AKSV_4                                  TX_PAGE_0, 0x20

#define MSK_PAGE_0_AKSV_4_AKSV3                            (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x21 AKSV_5 Register                                    (Default: 0x00)    */
#define REG_PAGE_0_AKSV_5                                  TX_PAGE_0, 0x21

#define MSK_PAGE_0_AKSV_5_AKSV4                            (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x22 Ri_1 Register                                      (Default: 0x00)    */
#define REG_PAGE_0_RI_1                                    TX_PAGE_0, 0x22

#define MSK_PAGE_0_RI_1_RI_PRIME0                          (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x23 Ri_2 Register                                      (Default: 0x00)    */
#define REG_PAGE_0_RI_2                                    TX_PAGE_0, 0x23

#define MSK_PAGE_0_RI_2_RI_PRIME1                          (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x24 HDCP Ri 128 Compare Value Register                 (Default: 0x7F)    */
#define REG_PAGE_0_RI_128_COMP                             TX_PAGE_0, 0x24

#define MSK_PAGE_0_RI_128_COMP_RI_128_COMP                 (0x7F << 0)

/*----------------------------------------------------------------------------*/
/* 0x25 HDCP I counter Register                            (Default: 0x00)    */
#define REG_PAGE_0_HDCP_I_CNT                              TX_PAGE_0, 0x25

#define MSK_PAGE_0_HDCP_I_CNT_HDCP_I_CNT                   (0x7F << 0)

/*----------------------------------------------------------------------------*/
/* 0x26 Ri Status Register                                 (Default: 0x00)    */
#define REG_PAGE_0_RI_STAT                                 TX_PAGE_0, 0x26

#define BIT_PAGE_0_RI_STAT_RI_ON                           (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x27 Ri Command Register                                (Default: 0x00)    */
#define REG_PAGE_0_RI_CMD                                  TX_PAGE_0, 0x27

#define BIT_PAGE_0_RI_CMD_BCAP_EN                          (0x01 << 1)
#define BIT_PAGE_0_RI_CMD_RI_EN                            (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x28 Ri Line Start Register                             (Default: 0x11)    */
#define REG_PAGE_0_RI_START                                TX_PAGE_0, 0x28

#define MSK_PAGE_0_RI_START_RI_LN_NUM                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x29 Ri from Rx #1 Register                             (Default: 0x00)    */
#define REG_PAGE_0_RI_RX_1                                 TX_PAGE_0, 0x29

#define MSK_PAGE_0_RI_RX_1_RI_RX1                          (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x2A Ri from Rx #2 Register                             (Default: 0x00)    */
#define REG_PAGE_0_RI_RX_2                                 TX_PAGE_0, 0x2A

#define MSK_PAGE_0_RI_RX_2_RI_RX2                          (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x2B HDCP Debug Register                                (Default: 0x00)    */
#define REG_PAGE_0_TXHDCP_DEBUG                            TX_PAGE_0, 0x2B

#define BIT_PAGE_0_TXHDCP_DEBUG_RI_TRUSH                   (0x01 << 7)
#define BIT_PAGE_0_TXHDCP_DEBUG_RI_HOLD                    (0x01 << 6)

/*----------------------------------------------------------------------------*/
/* 0x3A Video H Resolution #1 Register                     (Default: 0x00)    */
#define REG_PAGE_0_H_RESL                                  TX_PAGE_0, 0x3A

#define MSK_PAGE_0_H_RESL_HRESOUT_7_0                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x3B Video H Resolution #2 Register                     (Default: 0x00)    */
#define REG_PAGE_0_H_RESH                                  TX_PAGE_0, 0x3B

#define MSK_PAGE_0_H_RESH_HRESOUT_12_8                     (0x1F << 0)

/*----------------------------------------------------------------------------*/
/* 0x3C Video V Refresh Low Register                       (Default: 0x00)    */
#define REG_PAGE_0_V_RESL                                  TX_PAGE_0, 0x3C

#define MSK_PAGE_0_V_RESL_VRESOUT_7_0                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x3D Video V Refresh High Register                      (Default: 0x00)    */
#define REG_PAGE_0_V_RESH                                  TX_PAGE_0, 0x3D

#define MSK_PAGE_0_V_RESH_VRESOUT_11_8                     (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x3E Video Interlace Adjustment Register                (Default: 0x00)    */
#define REG_PAGE_0_IADJUST                                 TX_PAGE_0, 0x3E


/*----------------------------------------------------------------------------*/
/* 0x3F Video Sync Polarity Detection Register             (Default: 0x00)    */
#define REG_PAGE_0_POL_DETECT                              TX_PAGE_0, 0x3F

#define BIT_PAGE_0_POL_DETECT_INTERLACEDOUT                (0x01 << 2)
#define BIT_PAGE_0_POL_DETECT_VSYNCPOLOUT                  (0x01 << 1)
#define BIT_PAGE_0_POL_DETECT_HSYNCPOLOUT                  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x40 Video H De #1 Register                             (Default: 0x00)    */
#define REG_PAGE_0_H_DEL                                   TX_PAGE_0, 0x40

#define MSK_PAGE_0_H_DEL_HDEOUT_7_0                        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x41 Video H De #2 Register                             (Default: 0x00)    */
#define REG_PAGE_0_H_DEH                                   TX_PAGE_0, 0x41

#define MSK_PAGE_0_H_DEH_HDEOUT_12_8                       (0x1F << 0)

/*----------------------------------------------------------------------------*/
/* 0x42 Video V De #1 Register                             (Default: 0x00)    */
#define REG_PAGE_0_V_DEL                                   TX_PAGE_0, 0x42

#define MSK_PAGE_0_V_DEL_VDEOUT_7_0                        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x43 Video V De #2 Register                             (Default: 0x00)    */
#define REG_PAGE_0_V_DEH                                   TX_PAGE_0, 0x43

#define MSK_PAGE_0_V_DEH_VDEOUT_11_8                       (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x44 Video Refresh Rate #1 Register                     (Default: 0x00)    */
#define REG_PAGE_0_REFRESH_RATEL                           TX_PAGE_0, 0x44

#define MSK_PAGE_0_REFRESH_RATEL_REFRESH_RATE_7_0          (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x45 Video Refresh Rate #2 Register                     (Default: 0x00)    */
#define REG_PAGE_0_REFRESH_RATEH                           TX_PAGE_0, 0x45

#define BIT_PAGE_0_REFRESH_RATEH_REFRESH_RATE_7_0          (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x48 Video Control Register                             (Default: 0x00)    */
#define REG_PAGE_0_VID_CTRL                                TX_PAGE_0, 0x48

#define BIT_PAGE_0_VID_CTRL_EXTENDEDBITMODE                (0x01 << 5)
#define BIT_PAGE_0_VID_CTRL_CSCMODE709                     (0x01 << 4)
#define MSK_PAGE_0_VID_CTRL_ICLK                           (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x49 Video Action Enable Register                       (Default: 0x00)    */
#define REG_PAGE_0_VID_ACEN                                TX_PAGE_0, 0x49

#define BIT_PAGE_0_VID_ACEN_CLIPINPUTISYC                  (0x01 << 4)
#define BIT_PAGE_0_VID_ACEN_ENRANGECLIP                    (0x01 << 3)
#define BIT_PAGE_0_VID_ACEN_ENRGB2YCBCR                    (0x01 << 2)
#define BIT_PAGE_0_VID_ACEN_ENRANGECOMPRESS                (0x01 << 1)
#define BIT_PAGE_0_VID_ACEN_ENDOWNSAMPLE                   (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x4A Video Mode Register                                (Default: 0x00)    */
#define REG_PAGE_0_VID_MODE                                TX_PAGE_0, 0x4A

#define BIT_PAGE_0_VID_MODE_M1080P                         (0x01 << 6)
#define VAL_PAGE_0_VID_MODE_M1080P_DISABLE                 (0x00 << 6)
#define VAL_PAGE_0_VID_MODE_M1080P_ENABLE                  (0x01 << 6)

#define BIT_PAGE_0_VID_MODE_ENDITHER                       (0x01 << 5)
#define BIT_PAGE_0_VID_MODE_ENRANGEEXPAND                  (0x01 << 4)
#define BIT_PAGE_0_VID_MODE_ENCSC                          (0x01 << 3)
#define BIT_PAGE_0_VID_MODE_ENUPSAMPLE                     (0x01 << 2)
#define BIT_PAGE_0_VID_MODE_ENDEMUX                        (0x01 << 1)

/*----------------------------------------------------------------------------*/
/* 0x4B Video Blank Data Low Byte Register                 (Default: 0x00)    */
#define REG_PAGE_0_VID_BLANK0                              TX_PAGE_0, 0x4B

#define MSK_PAGE_0_VID_BLANK0_BLANK0                       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x4D Video Blank Data Mid Byte Register                 (Default: 0x00)    */
#define REG_PAGE_0_VID_BLANK1                              TX_PAGE_0, 0x4D

#define MSK_PAGE_0_VID_BLANK1_BLANK1                       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x4F Video Blank Data High Byte Register                (Default: 0x00)    */
#define REG_PAGE_0_VID_BLANK2                              TX_PAGE_0, 0x4F

#define MSK_PAGE_0_VID_BLANK2_BLANK2                       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x51 Video Input Mode Register                          (Default: 0xC0)    */
#define REG_PAGE_0_VID_OVRRD                               TX_PAGE_0, 0x51

#define	REG_PAGE_0_VID_OVRRD_DEFVAL							0x80
#define BIT_PAGE_0_VID_OVRRD_PP_AUTO_DISABLE               (0x01 << 7)
#define BIT_PAGE_0_VID_OVRRD_M1080P_OVRRD                  (0x01 << 6)
#define BIT_PAGE_0_VID_OVRRD_MINIVSYNC_ON                  (0x01 << 5)

#define BIT_PAGE_0_VID_OVRRD_3DCONV_EN                     (0x01 << 4)
#define VAL_PAGE_0_VID_OVRRD_3DCONV_EN_NORMAL              (0x00 << 4)
#define VAL_PAGE_0_VID_OVRRD_3DCONV_EN_FRAME_PACK          (0x01 << 4)

#define BIT_PAGE_0_VID_OVRRD_ENRGB2YCBCR_OVRRD             (0x01 << 2)
#define BIT_PAGE_0_VID_OVRRD_ENDOWNSAMPLE_OVRRD            (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x52 CEA-861 VSI InfoFrame MHL IEEE No #0 Register      (Default: 0x1D)    */
#define REG_PAGE_0_VSI_MHL_IEEE_NO_0                       TX_PAGE_0, 0x52

#define MSK_PAGE_0_VSI_MHL_IEEE_NO_0_MHL_IEEE_NO_7_0       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x53 CEA-861 VSI InfoFrame MHL IEEE No #1 Register      (Default: 0xA6)    */
#define REG_PAGE_0_VSI_MHL_IEEE_NO_1                       TX_PAGE_0, 0x53

#define MSK_PAGE_0_VSI_MHL_IEEE_NO_1_MHL_IEEE_NO_15_8      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x54 CEA-861 VSI InfoFrame MHL IEEE No #2 Register      (Default: 0x7C)    */
#define REG_PAGE_0_VSI_MHL_IEEE_NO_2                       TX_PAGE_0, 0x54

#define MSK_PAGE_0_VSI_MHL_IEEE_NO_2_MHL_IEEE_NO_23_16     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x57 I2C Address for Page MHLSpec                       (Default: 0x00)    */
#define REG_PAGE_0_PAGE_MHLSPEC_ADDR                       TX_PAGE_0, 0x57

#define MSK_PAGE_0_PAGE_MHLSPEC_ADDR_PAGE_MHLSPEC_ADDR_7_0 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x58 I2C Address for Page 7 Register                    (Default: 0x00)    */
#define REG_PAGE_0_PAGE_7_ADDR                             TX_PAGE_0, 0x58

#define MSK_PAGE_0_PAGE_7_ADDR_PAGE_7_ADDR_7_0             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x59 One Second Count #1 Register                       (Default: 0x0E)    */
#define REG_PAGE_0_ONE_SEC_CNTL                            TX_PAGE_0, 0x59

#define MSK_PAGE_0_ONE_SEC_CNTL_ONE_SEC_CNT_7_0            (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x5A One Second Count #2 Register                       (Default: 0x27)    */
#define REG_PAGE_0_ONE_SEC_CNTM                            TX_PAGE_0, 0x5A

#define MSK_PAGE_0_ONE_SEC_CNTM_ONE_SEC_CNT_15_8           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x5B One Second Count #3 Register                       (Default: 0x07)    */
#define REG_PAGE_0_ONE_SEC_CNTH                            TX_PAGE_0, 0x5B

#define MSK_PAGE_0_ONE_SEC_CNTH_ONE_SEC_CNT_19_16          (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x5D system counter_0                                   (Default: 0xD5)    */
#define REG_PAGE_0_SYS_CNTR_0                              TX_PAGE_0, 0x5D

#define MSK_PAGE_0_SYS_CNTR_0_CNTR_VALUE_7_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x5E system counter_1                                   (Default: 0x30)    */
#define REG_PAGE_0_SYS_CNTR_1                              TX_PAGE_0, 0x5E

#define MSK_PAGE_0_SYS_CNTR_1_CNTR_VALUE_15_8              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x5F Fast Interrupt Status Register                     (Default: 0x55)    */
#define REG_PAGE_0_FAST_INTR_STAT                          TX_PAGE_0, 0x5F

#define BIT_PAGE_0_FAST_INTR_STAT_FAST_INTR_STAT_4         (0x01 << 4)
#define BIT_PAGE_0_FAST_INTR_STAT_FAST_INTR_STAT_3         (0x01 << 3)
#define BIT_PAGE_0_FAST_INTR_STAT_FAST_INTR_STAT_2         (0x01 << 2)
#define BIT_PAGE_0_FAST_INTR_STAT_FAST_INTR_STAT_1         (0x01 << 1)
#define BIT_PAGE_0_FAST_INTR_STAT_FAST_INTR_STAT_0         (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x60 1st Layer Interrupt Status #1 Register             (Default: 0x55)    */
#define REG_PAGE_0_L1_INTR_STAT_0                          TX_PAGE_0, 0x60

#define MSK_PAGE_0_L1_INTR_STAT_0_                         (0x7F << 1)
#define BIT_PAGE_0_L1_INTR_STAT_0_L1_INTR_STAT_0           (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x61 1st Layer Interrupt Status #2 Register             (Default: 0x55)    */
#define REG_PAGE_0_L1_INTR_STAT_1                          TX_PAGE_0, 0x61

#define MSK_PAGE_0_L1_INTR_STAT_1_                         (0x7F << 1)
#define BIT_PAGE_0_L1_INTR_STAT_1_L1_INTR_STAT_8           (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x62 1st Layer Interrupt Status #3 Register             (Default: 0x55)    */
#define REG_PAGE_0_L1_INTR_STAT_2                          TX_PAGE_0, 0x62

#define MSK_PAGE_0_L1_INTR_STAT_2_                         (0x7F << 1)
#define BIT_PAGE_0_L1_INTR_STAT_2_L1_INTR_STAT_16          (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x63 1st Layer Interrupt Status #4 Register             (Default: 0x55)    */
#define REG_PAGE_0_L1_INTR_STAT_3                          TX_PAGE_0, 0x63

#define MSK_PAGE_0_L1_INTR_STAT_3_                         (0x7F << 1)
#define BIT_PAGE_0_L1_INTR_STAT_3_L1_INTR_STAT_24          (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x64 1st Layer Interrupt Status #5 Register             (Default: 0x55)    */
#define REG_PAGE_0_L1_INTR_STAT_4                          TX_PAGE_0, 0x64

#define MSK_PAGE_0_L1_INTR_STAT_4_                         (0x7F << 1)
#define BIT_PAGE_0_L1_INTR_STAT_4_L1_INTR_STAT_32          (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x65 1st Layer Interrupt Status #6 Register             (Default: 0x55)    */
#define REG_PAGE_0_L1_INTR_STAT_5                          TX_PAGE_0, 0x65

#define BIT_PAGE_0_L1_INTR_STAT_5_L1_INTR_STAT_41          (0x01 << 1)
#define BIT_PAGE_0_L1_INTR_STAT_5_L1_INTR_STAT_40          (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x6E GPIO Control Register 1                            (Default: 0x15)    */
#define REG_PAGE_0_GPIO_CTRL1                              TX_PAGE_0, 0x6E

#define BIT_PAGE_0_GPIO_CTRL1_GPIO_I_8                     (0x01 << 5)
#define BIT_PAGE_0_GPIO_CTRL1_GPIO_OEN_8                   (0x01 << 4)
#define BIT_PAGE_0_GPIO_CTRL1_GPIO_I_7                     (0x01 << 3)
#define BIT_PAGE_0_GPIO_CTRL1_GPIO_OEN_7                   (0x01 << 2)
#define BIT_PAGE_0_GPIO_CTRL1_GPIO_I_6                     (0x01 << 1)
#define BIT_PAGE_0_GPIO_CTRL1_GPIO_OEN_6                   (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x6F Interrupt Control Register                         (Default: 0x06)    */
#define REG_PAGE_0_INT_CTRL                                TX_PAGE_0, 0x6F

#define BIT_PAGE_0_INT_CTRL_SOFT_INTR_EN                   (0x01 << 7)
#define BIT_PAGE_0_INT_CTRL_INTR_OD                        (0x01 << 2)
#define BIT_PAGE_0_INT_CTRL_INTR_POLARITY                  (0x01 << 1)

/*----------------------------------------------------------------------------*/
/* 0x70 Interrupt State Register                           (Default: 0x00)    */
#define REG_PAGE_0_INTR_STATE                              TX_PAGE_0, 0x70

#define BIT_PAGE_0_INTR_STATE_INTR_STATE                   (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x71 Interrupt Source #1 Register                       (Default: 0x00)    */
#define REG_PAGE_0_INTR1                                   TX_PAGE_0, 0x71

#define BIT_PAGE_0_INTR1_INTR1_STAT7                       (0x01 << 7)
#define BIT_PAGE_0_INTR1_INTR1_STAT6                       (0x01 << 6)
#define BIT_PAGE_0_INTR1_INTR1_STAT5                       (0x01 << 5)
#define BIT_PAGE_0_INTR1_INTR1_STAT2                       (0x01 << 2)

/*----------------------------------------------------------------------------*/
/* 0x72 Interrupt Source #2 Register                       (Default: 0x00)    */
#define REG_PAGE_0_INTR2                                   TX_PAGE_0, 0x72

#define BIT_PAGE_0_INTR2_INTR2_STAT7                       (0x01 << 7)
#define BIT_PAGE_0_INTR2_INTR2_STAT5                       (0x01 << 5)
#define BIT_PAGE_0_INTR2_INTR2_STAT1                       (0x01 << 1)
#define BIT_PAGE_0_INTR2_INTR2_STAT0                       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x73 Interrupt Source #3 Register                       (Default: 0x01)    */
#define REG_PAGE_0_INTR3                                   TX_PAGE_0, 0x73

#define BIT_PAGE_0_INTR3_INTR3_STAT7                       (0x01 << 7)
#define BIT_PAGE_0_INTR3_INTR3_STAT6                       (0x01 << 6)
#define BIT_PAGE_0_INTR3_INTR3_STAT5                       (0x01 << 5)
#define BIT_PAGE_0_INTR3_INTR3_STAT4                       (0x01 << 4)
#define BIT_PAGE_0_INTR3_DDC_CMD_DONE                      (0x01 << 3)
#define BIT_PAGE_0_INTR3_INTR3_STAT2                       (0x01 << 2)
#define BIT_PAGE_0_INTR3_INTR3_STAT1                       (0x01 << 1)
#define BIT_PAGE_0_INTR3_INTR3_STAT0                       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x74 Interrupt Source #5 Register                       (Default: 0x00)    */
#define REG_PAGE_0_INTR5                                   TX_PAGE_0, 0x74

#define BIT_PAGE_0_INTR5_INTR5_STAT7                       (0x01 << 7)
#define BIT_PAGE_0_INTR5_INTR5_STAT6                       (0x01 << 6)
#define BIT_PAGE_0_INTR5_INTR5_STAT5                       (0x01 << 5)
#define BIT_PAGE_0_INTR5_INTR5_STAT4                       (0x01 << 4)
#define BIT_PAGE_0_INTR5_INTR5_STAT3                       (0x01 << 3)
#define BIT_PAGE_0_INTR5_INTR5_STAT2                       (0x01 << 2)
#define BIT_PAGE_0_INTR5_INTR5_STAT1                       (0x01 << 1)
#define BIT_PAGE_0_INTR5_INTR5_STAT0                       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x75 Interrupt #1 Mask Register                         (Default: 0x00)    */
#define REG_PAGE_0_INTR1_MASK                              TX_PAGE_0, 0x75

#define BIT_PAGE_0_INTR1_MASK_INTR1_MASK7                  (0x01 << 7)
#define BIT_PAGE_0_INTR1_MASK_INTR1_MASK6                  (0x01 << 6)
#define BIT_PAGE_0_INTR1_MASK_INTR1_MASK5                  (0x01 << 5)
#define BIT_PAGE_0_INTR1_MASK_INTR1_MASK2                  (0x01 << 2)

/*----------------------------------------------------------------------------*/
/* 0x76 Interrupt #2 Mask Register                         (Default: 0x00)    */
#define REG_PAGE_0_INTR2_MASK                              TX_PAGE_0, 0x76

#define BIT_PAGE_0_INTR2_MASK_INTR2_MASK7                  (0x01 << 7)
#define BIT_PAGE_0_INTR2_MASK_INTR2_MASK5                  (0x01 << 5)
#define BIT_PAGE_0_INTR2_MASK_INTR2_MASK1                  (0x01 << 1)
#define BIT_PAGE_0_INTR2_MASK_INTR2_MASK0                  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x77 Interrupt #3 Mask Register                         (Default: 0x00)    */
#define REG_PAGE_0_INTR3_MASK                              TX_PAGE_0, 0x77

#define BIT_PAGE_0_INTR3_MASK_INTR3_MASK7                  (0x01 << 7)
#define BIT_PAGE_0_INTR3_MASK_INTR3_MASK6                  (0x01 << 6)
#define BIT_PAGE_0_INTR3_MASK_INTR3_MASK5                  (0x01 << 5)
#define BIT_PAGE_0_INTR3_MASK_INTR3_MASK4                  (0x01 << 4)
#define BIT_PAGE_0_INTR3_MASK_INTR3_MASK3                  (0x01 << 3)
#define BIT_PAGE_0_INTR3_MASK_INTR3_MASK2                  (0x01 << 2)
#define BIT_PAGE_0_INTR3_MASK_INTR3_MASK1                  (0x01 << 1)
#define BIT_PAGE_0_INTR3_MASK_INTR3_MASK0                  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x78 Interrupt #5 Mask Register                         (Default: 0x00)    */
#define REG_PAGE_0_INTR5_MASK                              TX_PAGE_0, 0x78

#define BIT_PAGE_0_INTR5_MASK_INTR5_MASK7                  (0x01 << 7)
#define BIT_PAGE_0_INTR5_MASK_INTR5_MASK6                  (0x01 << 6)
#define BIT_PAGE_0_INTR5_MASK_INTR5_MASK5                  (0x01 << 5)
#define BIT_PAGE_0_INTR5_MASK_INTR5_MASK4                  (0x01 << 4)
#define BIT_PAGE_0_INTR5_MASK_INTR5_MASK3                  (0x01 << 3)
#define BIT_PAGE_0_INTR5_MASK_INTR5_MASK2                  (0x01 << 2)
#define BIT_PAGE_0_INTR5_MASK_INTR5_MASK1                  (0x01 << 1)
#define BIT_PAGE_0_INTR5_MASK_INTR5_MASK0                  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x79 Hot Plug Connection Control Register               (Default: 0x45)    */
#define REG_PAGE_0_HPD_CTRL                                TX_PAGE_0, 0x79

#define BIT_PAGE_0_HPD_CTRL_HPD_DS_SIGNAL                  (0x01 << 7)
#define BIT_PAGE_0_HPD_CTRL_HPD_OUT_OD_EN                  (0x01 << 6)

#define BIT_PAGE_0_HPD_CTRL_HPD_OUT_OVR_VAL                (0x01 << 5)
#define VAL_PAGE_0_HPD_CTRL_HPD_LOW				           (0x00 << 5)
#define VAL_PAGE_0_HPD_CTRL_HPD_HIGH			           (0x01 << 5)

#define BIT_PAGE_0_HPD_CTRL_HPD_OUT_OVR_EN                 (0x01 << 4)
#define VAL_PAGE_0_HPD_CTRL_HPD_OUT_OVR_EN_OFF             (0x00 << 4)
#define VAL_PAGE_0_HPD_CTRL_HPD_OUT_OVR_EN_ON              (0x01 << 4)

#define BIT_PAGE_0_HPD_CTRL_GPIO_I_1                       (0x01 << 3)
#define BIT_PAGE_0_HPD_CTRL_GPIO_OEN_1                     (0x01 << 2)
#define BIT_PAGE_0_HPD_CTRL_GPIO_I_0                       (0x01 << 1)
#define BIT_PAGE_0_HPD_CTRL_GPIO_OEN_0                     (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x7A GPIO Control Register                              (Default: 0x55)    */
#define REG_PAGE_0_GPIO_CTRL                               TX_PAGE_0, 0x7A

#define BIT_PAGE_0_GPIO_CTRL_GPIO_I_5                      (0x01 << 7)
#define BIT_PAGE_0_GPIO_CTRL_GPIO_OEN_5                    (0x01 << 6)
#define BIT_PAGE_0_GPIO_CTRL_GPIO_I_4                      (0x01 << 5)
#define BIT_PAGE_0_GPIO_CTRL_GPIO_OEN_4                    (0x01 << 4)
#define BIT_PAGE_0_GPIO_CTRL_GPIO_I_3                      (0x01 << 3)
#define BIT_PAGE_0_GPIO_CTRL_GPIO_OEN_3                    (0x01 << 2)
#define BIT_PAGE_0_GPIO_CTRL_GPIO_I_2                      (0x01 << 1)
#define BIT_PAGE_0_GPIO_CTRL_GPIO_OEN_2                    (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x7B Interrupt Source 7 Register                        (Default: 0x00)    */
#define REG_PAGE_0_INTR7                                   TX_PAGE_0, 0x7B

#define BIT_PAGE_0_INTR7_INTR7_STAT7                       (0x01 << 7)
#define BIT_PAGE_0_INTR7_INTR7_STAT6                       (0x01 << 6)
#define BIT_PAGE_0_INTR7_INTR7_STAT4                       (0x01 << 4)
#define BIT_PAGE_0_INTR7_INTR7_STAT3                       (0x01 << 3)
#define BIT_PAGE_0_INTR7_INTR7_STAT2                       (0x01 << 2)
#define BIT_PAGE_0_INTR7_INTR7_STAT1                       (0x01 << 1)
#define BIT_PAGE_0_INTR7_INTR7_STAT0                       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x7C Interrupt Source 8 Register                        (Default: 0x00)    */
#define REG_PAGE_0_INTR8                                   TX_PAGE_0, 0x7C

#define BIT_PAGE_0_INTR8_INTR8_STAT6                       (0x01 << 6)
#define BIT_PAGE_0_INTR8_INTR8_STAT5                       (0x01 << 5)
#define BIT_PAGE_0_INTR8_INTR8_STAT4                       (0x01 << 4)
#define BIT_PAGE_0_INTR8_INTR8_STAT3                       (0x01 << 3)
#define BIT_PAGE_0_INTR8_INTR8_STAT2                       (0x01 << 2)
#define BIT_PAGE_0_INTR8_INTR8_STAT1                       (0x01 << 1)
#define BIT_PAGE_0_INTR8_INTR8_STAT0                       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x7D Interrupt #7 Mask Register                         (Default: 0x00)    */
#define REG_PAGE_0_INTR7_MASK                              TX_PAGE_0, 0x7D

#define BIT_PAGE_0_INTR7_MASK_INTR7_MASK7                  (0x01 << 7)
#define BIT_PAGE_0_INTR7_MASK_INTR7_MASK6                  (0x01 << 6)
#define BIT_PAGE_0_INTR7_MASK_INTR7_MASK4                  (0x01 << 4)
#define BIT_PAGE_0_INTR7_MASK_INTR7_MASK3                  (0x01 << 3)
#define BIT_PAGE_0_INTR7_MASK_INTR7_MASK2                  (0x01 << 2)
#define BIT_PAGE_0_INTR7_MASK_INTR7_MASK1                  (0x01 << 1)
#define BIT_PAGE_0_INTR7_MASK_INTR7_MASK0                  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x7E Interrupt #8 Mask Register                         (Default: 0x00)    */
#define REG_PAGE_0_INTR8_MASK                              TX_PAGE_0, 0x7E

#define BIT_PAGE_0_INTR8_MASK_INTR8_MASK6                  (0x01 << 6)
#define BIT_PAGE_0_INTR8_MASK_INTR8_MASK5                  (0x01 << 5)
#define BIT_PAGE_0_INTR8_MASK_INTR8_MASK4                  (0x01 << 4)
#define BIT_PAGE_0_INTR8_MASK_INTR8_MASK3                  (0x01 << 3)
#define BIT_PAGE_0_INTR8_MASK_INTR8_MASK2                  (0x01 << 2)
#define BIT_PAGE_0_INTR8_MASK_INTR8_MASK1                  (0x01 << 1)
#define BIT_PAGE_0_INTR8_MASK_INTR8_MASK0                  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x80 IEEE                                               (Default: 0x10)    */
#define REG_PAGE_0_TMDS_CCTRL                              TX_PAGE_0, 0x80

#define BIT_PAGE_0_TMDS_CCTRL_TMDS_OE                      (0x01 << 4)

/*----------------------------------------------------------------------------*/
/* 0x81 TMDS Clock Status Register                         (Default: 0x04)    */
#define REG_PAGE_0_TMDS_CSTAT                              TX_PAGE_0, 0x81

#define BIT_PAGE_0_TMDS_CSTAT_RPWR5V                       (0x01 << 2)

/*----------------------------------------------------------------------------*/
/* 0x82 TMDS Control Register                              (Default: 0x20)    */
#define REG_PAGE_0_TMDS_CTRL                               TX_PAGE_0, 0x82

#define MSK_PAGE_0_TMDS_CTRL_TCLK_SEL                      (0x03 << 5)

/*----------------------------------------------------------------------------*/
/* 0x83 TMDS Control #2 Register                           (Default: 0x10)    */
#define REG_PAGE_0_TMDS_CTRL2                              TX_PAGE_0, 0x83

#define MSK_PAGE_0_TMDS_CTRL2_CLKMULT_CTL                  (0x03 << 4)

/*----------------------------------------------------------------------------*/
/* 0x85 TMDS Control #4 Register                           (Default: 0x02)    */
#define REG_PAGE_0_TMDS_CTRL4                              TX_PAGE_0, 0x85

#define BIT_PAGE_0_TMDS_CTRL4_SCDT_CKDT_SEL                (0x01 << 1)
#define BIT_PAGE_0_TMDS_CTRL4_TX_EN_BY_SCDT                (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x86 SCDT Holdoff MSB                                   (Default: 0x39)    */
#define REG_PAGE_0_SCDT_HOLDOFF_MSB                        TX_PAGE_0, 0x86

#define MSK_PAGE_0_SCDT_HOLDOFF_MSB_SCDT_HOLDOFF_MSB_7_0   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x88 TMDS Control #7 Register                           (Default: 0x00)    */
#define REG_PAGE_0_TMDS_CTRL7                              TX_PAGE_0, 0x88

#define BIT_PAGE_0_TMDS_CTRL7_TMDS_SWAP_BIT                (0x01 << 7)

/*----------------------------------------------------------------------------*/
/* 0xB0 MHL TX Multi Zone Control1 Register                (Default: 0x00)    */
#define REG_PAGE_0_TXMZ_CTRL1                              TX_PAGE_0, 0xB0


/*----------------------------------------------------------------------------*/
/* 0xB1 MHL TX Multi Zone Control2 Register                (Default: 0x00)    */
#define REG_PAGE_0_TXMZ_CTRL2                              TX_PAGE_0, 0xB1

#define BIT_PAGE_0_TXMZ_CTRL2_KEEPER_CLK_PHASE             (0x01 << 7)
#define BIT_PAGE_0_TXMZ_CTRL2_FC_CLK_PHASE                 (0x01 << 6)

/*----------------------------------------------------------------------------*/
/* 0xB2 MHL TX Multi Zone Control3 Register                (Default: 0x00)    */
#define REG_PAGE_0_TXMZ_CTRL3                              TX_PAGE_0, 0xB2


/*----------------------------------------------------------------------------*/
/* 0xB3 MHL TX Multi Zone Control4 Register                (Default: 0x00)    */
#define REG_PAGE_0_TXMZ_CTRL4                              TX_PAGE_0, 0xB3


/*----------------------------------------------------------------------------*/
/* 0xB4 MHL3 CTS Control Register                          (Default: 0x0A)    */
#define REG_PAGE_0_MHL3CTS_CTL                             TX_PAGE_0, 0xB4

#define BIT_PAGE_0_MHL3CTS_CTL_MHL3CTS_EN                  (0x01 << 7)
#define BIT_PAGE_0_MHL3CTS_CTL_MHL3CTS_1LANE               (0x01 << 4)
#define MSK_PAGE_0_MHL3CTS_CTL_MHL3CTS_CLKOUT_SEL          (0x03 << 2)
#define BIT_PAGE_0_MHL3CTS_CTL_MHL3CTS_3LANE               (0x01 << 1)
#define BIT_PAGE_0_MHL3CTS_CTL_MHL3CTS_RST                 (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xBB BIST CNTL Register                                 (Default: 0x00)    */
#define REG_PAGE_0_BIST_CTRL                               TX_PAGE_0, 0xBB

#define BIT_PAGE_0_BIST_CTRL_BIST_START_SEL                (0x01 << 5)
#define BIT_PAGE_0_BIST_CTRL_BIST_START_BIT                (0x01 << 4)
#define BIT_PAGE_0_BIST_CTRL_BIST_ALWAYS_ON                (0x01 << 3)
#define BIT_PAGE_0_BIST_CTRL_BIST_TRANS                    (0x01 << 2)
#define BIT_PAGE_0_BIST_CTRL_BIST_RESET                    (0x01 << 1)
#define BIT_PAGE_0_BIST_CTRL_BIST_EN                       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xBD BIST DURATION0 Register                            (Default: 0x00)    */
#define REG_PAGE_0_BIST_TEST_SEL                           TX_PAGE_0, 0xBD

#define MSK_PAGE_0_BIST_TEST_SEL_BIST_PATT_SEL             (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xBE BIST VIDEO_MODE Register                           (Default: 0x00)    */
#define REG_PAGE_0_BIST_VIDEO_MODE                         TX_PAGE_0, 0xBE

#define MSK_PAGE_0_BIST_VIDEO_MODE_BIST_VIDEO_MODE_3_0     (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xBF BIST DURATION0 Register                            (Default: 0x00)    */
#define REG_PAGE_0_BIST_DURATION_0                         TX_PAGE_0, 0xBF

#define MSK_PAGE_0_BIST_DURATION_0_BIST_DURATION_7_0       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC0 BIST DURATION1 Register                            (Default: 0x00)    */
#define REG_PAGE_0_BIST_DURATION_1                         TX_PAGE_0, 0xC0

#define MSK_PAGE_0_BIST_DURATION_1_BIST_DURATION_15_8      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC1 BIST DURATION2 Register                            (Default: 0x00)    */
#define REG_PAGE_0_BIST_DURATION_2                         TX_PAGE_0, 0xC1

#define MSK_PAGE_0_BIST_DURATION_2_BIST_DURATION_22_16     (0x7F << 0)

/*----------------------------------------------------------------------------*/
/* 0xC2 BIST 8BIT_PATTERN Register                         (Default: 0x00)    */
#define REG_PAGE_0_BIST_8BIT_PATTERN                       TX_PAGE_0, 0xC2

#define MSK_PAGE_0_BIST_8BIT_PATTERN_BIST_10BIT_PATT28LSB  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC3 BIST 10BIT_PATTERN_L Register                      (Default: 0x00)    */
#define REG_PAGE_0_BIST_10BIT_PATTERN_L                    TX_PAGE_0, 0xC3

#define MSK_PAGE_0_BIST_10BIT_PATTERN_L_BIST_10BIT_PATT18LSB (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC4 BIST 10BIT_PATTERN_H Register                      (Default: 0x00)    */
#define REG_PAGE_0_BIST_10BIT_PATTERN_H                    TX_PAGE_0, 0xC4

#define MSK_PAGE_0_BIST_10BIT_PATTERN_H_BIST_10BIT_PATT22MSB (0x03 << 2)
#define MSK_PAGE_0_BIST_10BIT_PATTERN_H_RG_BIST_10BIT_PATT12MSB (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0xC5 BIST 10BIT_PATTERN_H Register                      (Default: 0x00)    */
#define REG_PAGE_0_BIST_STATUS                             TX_PAGE_0, 0xC5

#define MSK_PAGE_0_BIST_STATUS_BIST_STATUS                 (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0xC7 LM DDC Register                                    (Default: 0x80)    */
#define REG_PAGE_0_LM_DDC                                  TX_PAGE_0, 0xC7

#define BIT_PAGE_0_LM_DDC_SW_TPI_EN                        (0x01 << 7)
#define VAL_PAGE_0_LM_DDC_SW_TPI_EN_ENABLED                (0x00 << 7)
#define VAL_PAGE_0_LM_DDC_SW_TPI_EN_DISABLED               (0x01 << 7)

#define BIT_PAGE_0_LM_DDC_VIDEO_MUTE_EN                    (0x01 << 5)
#define BIT_PAGE_0_LM_DDC_DDC_TPI_SW                       (0x01 << 2)
#define BIT_PAGE_0_LM_DDC_DDC_GRANT                        (0x01 << 1)
#define BIT_PAGE_0_LM_DDC_DDC_GPU_REQUEST                  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xCC TX SHA Control Register                            (Default: 0x01)    */
#define REG_PAGE_0_TXSHA_CTRL                              TX_PAGE_0, 0xCC

#define BIT_PAGE_0_TXSHA_CTRL_SHACTRL_STAT1                (0x01 << 1)
#define BIT_PAGE_0_TXSHA_CTRL_SHA_GO_STAT                  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xCD TX KSV FIFO Register                               (Default: 0x00)    */
#define REG_PAGE_0_TXKSV_FIFO                              TX_PAGE_0, 0xCD

#define MSK_PAGE_0_TXKSV_FIFO_KSV_FIFO_OUT                 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCE HDCP Repeater Down Stream BSTATUS #1 Register      (Default: 0x00)    */
#define REG_PAGE_0_TXDS_BSTATUS1                           TX_PAGE_0, 0xCE

#define BIT_PAGE_0_TXDS_BSTATUS1_DS_DEV_EXCEED             (0x01 << 7)
#define MSK_PAGE_0_TXDS_BSTATUS1_DS_DEV_CNT                (0x7F << 0)

/*----------------------------------------------------------------------------*/
/* 0xCF HDCP Repeater Down Stream BSTATUS #2 Register      (Default: 0x10)    */
#define REG_PAGE_0_TXDS_BSTATUS2                           TX_PAGE_0, 0xCF

#define MSK_PAGE_0_TXDS_BSTATUS2_DS_BSTATUS                (0x07 << 5)
#define BIT_PAGE_0_TXDS_BSTATUS2_DS_HDMI_MODE              (0x01 << 4)
#define BIT_PAGE_0_TXDS_BSTATUS2_DS_CASC_EXCEED            (0x01 << 3)
#define MSK_PAGE_0_TXDS_BSTATUS2_DS_DEPTH                  (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0xD8 HDCP Repeater V.H0 #0 Register                     (Default: 0x00)    */
#define REG_PAGE_0_TXVH0_0                                 TX_PAGE_0, 0xD8

#define MSK_PAGE_0_TXVH0_0_VP_H0_7_0                       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD9 HDCP Repeater V.H0 #1 Register                     (Default: 0x00)    */
#define REG_PAGE_0_TXVH0_1                                 TX_PAGE_0, 0xD9

#define MSK_PAGE_0_TXVH0_1_VP_H0_15_8                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xDA HDCP Repeater V.H0 #2 Register                     (Default: 0x00)    */
#define REG_PAGE_0_TXVH0_2                                 TX_PAGE_0, 0xDA

#define MSK_PAGE_0_TXVH0_2_VP_H0_23_16                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xDB HDCP Repeater V.H0 #3 Register                     (Default: 0x00)    */
#define REG_PAGE_0_TXVH0_3                                 TX_PAGE_0, 0xDB

#define MSK_PAGE_0_TXVH0_3_VP_H0_31_24                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xDC HDCP Repeater V.H1 #0 Register                     (Default: 0x00)    */
#define REG_PAGE_0_TXVH1_0                                 TX_PAGE_0, 0xDC

#define MSK_PAGE_0_TXVH1_0_VP_H1_7_0                       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xDD HDCP Repeater V.H1 #1 Register                     (Default: 0x00)    */
#define REG_PAGE_0_TXVH1_1                                 TX_PAGE_0, 0xDD

#define MSK_PAGE_0_TXVH1_1_VP_H1_15_8                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xDE HDCP Repeater V.H1 #2 Register                     (Default: 0x00)    */
#define REG_PAGE_0_TXVH1_2                                 TX_PAGE_0, 0xDE

#define MSK_PAGE_0_TXVH1_2_VP_H1_23_16                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xDF HDCP Repeater V.H1 #3 Register                     (Default: 0x00)    */
#define REG_PAGE_0_TXVH1_3                                 TX_PAGE_0, 0xDF

#define MSK_PAGE_0_TXVH1_3_VP_H1_31_24                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE0 HDCP Repeater V.H2 #0 Register                     (Default: 0x00)    */
#define REG_PAGE_0_TXVH2_0                                 TX_PAGE_0, 0xE0

#define MSK_PAGE_0_TXVH2_0_VP_H2_7_0                       (0xFF << 0)


/*----------------------------------------------------------------------------*/
/* 0xE1 HDCP Repeater V.H2 #1 Register                     (Default: 0x00)    */
#define REG_PAGE_0_TXVH2_1                                 TX_PAGE_0, 0xE1

#define MSK_PAGE_0_TXVH2_1_VP_H2_15_8                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE2 HDCP Repeater V.H2 #2 Register                     (Default: 0x00)    */
#define REG_PAGE_0_TXVH2_2                                 TX_PAGE_0, 0xE2

#define MSK_PAGE_0_TXVH2_2_VP_H2_23_16                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE3 HDCP Repeater V.H2 #3 Register                     (Default: 0x00)    */
#define REG_PAGE_0_TXVH2_3                                 TX_PAGE_0, 0xE3

#define MSK_PAGE_0_TXVH2_3_VP_H2_31_24                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE4 HDCP Repeater V.H3 #0 Register                     (Default: 0x00)    */
#define REG_PAGE_0_TXVH3_0                                 TX_PAGE_0, 0xE4

#define MSK_PAGE_0_TXVH3_0_VP_H3_7_0                       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE5 HDCP Repeater V.H3 #1 Register                     (Default: 0x00)    */
#define REG_PAGE_0_TXVH3_1                                 TX_PAGE_0, 0xE5

#define MSK_PAGE_0_TXVH3_1_VP_H3_15_8                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE6 HDCP Repeater V.H3 #2 Register                     (Default: 0x00)    */
#define REG_PAGE_0_TXVH3_2                                 TX_PAGE_0, 0xE6

#define MSK_PAGE_0_TXVH3_2_VP_H3_23_16                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE7 HDCP Repeater V.H3 #3 Register                     (Default: 0x00)    */
#define REG_PAGE_0_TXVH3_3                                 TX_PAGE_0, 0xE7

#define MSK_PAGE_0_TXVH3_3_VP_H3_31_24                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE8 HDCP Repeater V.H4 #0 Register                     (Default: 0x00)    */
#define REG_PAGE_0_TXVH4_0                                 TX_PAGE_0, 0xE8

#define MSK_PAGE_0_TXVH4_0_VP_H4_7_0                       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE9 HDCP Repeater V.H4 #1 Register                     (Default: 0x00)    */
#define REG_PAGE_0_TXVH4_1                                 TX_PAGE_0, 0xE9

#define MSK_PAGE_0_TXVH4_1_VP_H4_15_8                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xEA HDCP Repeater V.H4 #2 Register                     (Default: 0x00)    */
#define REG_PAGE_0_TXVH4_2                                 TX_PAGE_0, 0xEA

#define MSK_PAGE_0_TXVH4_2_VP_H4_23_16                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xEB HDCP Repeater V.H4 #3 Register                     (Default: 0x00)    */
#define REG_PAGE_0_TXVH4_3                                 TX_PAGE_0, 0xEB

#define MSK_PAGE_0_TXVH4_3_VP_H4_31_24                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xEC DDC I2C Manual Register                            (Default: 0x03)    */
#define REG_PAGE_0_DDC_MANUAL                              TX_PAGE_0, 0xEC

#define BIT_PAGE_0_DDC_MANUAL_MAN_DDC                      (0x01 << 7)
#define BIT_PAGE_0_DDC_MANUAL_VP_SEL                       (0x01 << 6)
#define BIT_PAGE_0_DDC_MANUAL_DSDA                         (0x01 << 5)
#define BIT_PAGE_0_DDC_MANUAL_DSCL                         (0x01 << 4)
#define BIT_PAGE_0_DDC_MANUAL_GCP_HW_CTL_EN                (0x01 << 3)
#define BIT_PAGE_0_DDC_MANUAL_DDCM_ABORT_WP                (0x01 << 2)
#define BIT_PAGE_0_DDC_MANUAL_IO_DSDA                      (0x01 << 1)
#define BIT_PAGE_0_DDC_MANUAL_IO_DSCL                      (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xED DDC I2C Target Slave Address Register              (Default: 0x00)    */
#define REG_PAGE_0_DDC_ADDR                                TX_PAGE_0, 0xED

#define MSK_PAGE_0_DDC_ADDR_DDC_ADDR                       (0x7F << 1)

/*----------------------------------------------------------------------------*/
/* 0xEE DDC I2C Target Segment Address Register            (Default: 0x00)    */
#define REG_PAGE_0_DDC_SEGM                                TX_PAGE_0, 0xEE

#define MSK_PAGE_0_DDC_SEGM_DDC_SEGMENT                    (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xEF DDC I2C Target Offset Adress Register              (Default: 0x00)    */
#define REG_PAGE_0_DDC_OFFSET                              TX_PAGE_0, 0xEF

#define MSK_PAGE_0_DDC_OFFSET_DDC_OFFSET                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xF0 DDC I2C Data In count #1 Register                  (Default: 0x00)    */
#define REG_PAGE_0_DDC_DIN_CNT1                            TX_PAGE_0, 0xF0

#define MSK_PAGE_0_DDC_DIN_CNT1_DDC_DIN_CNT_7_0            (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xF1 DDC I2C Data In count #2 Register                  (Default: 0x00)    */
#define REG_PAGE_0_DDC_DIN_CNT2                            TX_PAGE_0, 0xF1

#define MSK_PAGE_0_DDC_DIN_CNT2_DDC_DIN_CNT_9_8            (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF2 DDC I2C Status Register                            (Default: 0x04)    */
#define REG_PAGE_0_DDC_STATUS                              TX_PAGE_0, 0xF2

#define BIT_PAGE_0_DDC_STATUS_DDC_BUS_LOW                  (0x01 << 6)
#define BIT_PAGE_0_DDC_STATUS_DDC_NO_ACK                   (0x01 << 5)
#define BIT_PAGE_0_DDC_STATUS_DDC_I2C_IN_PROG              (0x01 << 4)
#define BIT_PAGE_0_DDC_STATUS_DDC_FIFO_FULL                (0x01 << 3)
#define BIT_PAGE_0_DDC_STATUS_DDC_FIFO_EMPTY               (0x01 << 2)
#define BIT_PAGE_0_DDC_STATUS_DDC_FIFO_READ_IN_SUE         (0x01 << 1)
#define BIT_PAGE_0_DDC_STATUS_DDC_FIFO_WRITE_IN_USE        (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF3 DDC I2C Command Register                           (Default: 0x30)    */
#define REG_PAGE_0_DDC_CMD                                 TX_PAGE_0, 0xF3

#define BIT_PAGE_0_DDC_CMD_SDA_DEL_EN                      (0x01 << 5)
#define BIT_PAGE_0_DDC_CMD_DDC_FLT_EN                      (0x01 << 4)

#define MSK_PAGE_0_DDC_CMD_DDC_CMD                         (0x0F << 0)
#define VAL_PAGE_0_DDC_CMD_ENH_DDC_READ_NO_ACK             0x04
#define VAL_PAGE_0_DDC_CMD_DDC_CMD_CLEAR_FIFO              (0x09 << 0)
#define VAL_PAGE_0_DDC_CMD_DDC_CMD_ABORT                   (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xF4 DDC I2C FIFO Data In/Out Register                  (Default: 0x00)    */
#define REG_PAGE_0_DDC_DATA                                TX_PAGE_0, 0xF4

#define MSK_PAGE_0_DDC_DATA_DDC_DATA_OUT                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xF5 DDC I2C Data Out Counter Register                  (Default: 0x00)    */
#define REG_PAGE_0_DDC_DOUT_CNT                            TX_PAGE_0, 0xF5

#define BIT_PAGE_0_DDC_DOUT_CNT_DDC_DELAY_CNT_8            (0x01 << 7)
#define MSK_PAGE_0_DDC_DOUT_CNT_DDC_DATA_OUT_CNT           (0x1F << 0)

/*----------------------------------------------------------------------------*/
/* 0xF6 DDC I2C Delay Count Register                       (Default: 0x14)    */
#define REG_PAGE_0_DDC_DELAY_CNT                           TX_PAGE_0, 0xF6

#define MSK_PAGE_0_DDC_DELAY_CNT_DDC_DELAY_CNT_7_0         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xF7 Test Control Register                              (Default: 0x80)    */
#define REG_PAGE_0_TEST_TXCTRL                             TX_PAGE_0, 0xF7

#define BIT_PAGE_0_TEST_TXCTRL_RCLK_REF_SEL                (0x01 << 7)
#define BIT_PAGE_0_TEST_TXCTRL_PCLK_REF_SEL                (0x01 << 6)
#define MSK_PAGE_0_TEST_TXCTRL_BYPASS_PLL_CLK              (0x0F << 2)
#define BIT_PAGE_0_TEST_TXCTRL_HDMI_MODE                   (0x01 << 1)
#define BIT_PAGE_0_TEST_TXCTRL_TST_PLLCK                   (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF8 CBUS Address Register                              (Default: 0x00)    */
#define REG_PAGE_0_PAGE_CBUS_ADDR                          TX_PAGE_0, 0xF8

#define MSK_PAGE_0_PAGE_CBUS_ADDR_PAGE_CBUS_ADDR_7_0       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xF9 I2C Status Register                                (Default: 0x01)    */
#define REG_PAGE_0_EPST                                    TX_PAGE_0, 0xF9

#define BIT_PAGE_0_EPST_OTP_UNLOCKED                       (0x01 << 7)
#define BIT_PAGE_0_EPST_BIST2_ERR_CLR                      (0x01 << 6)
#define BIT_PAGE_0_EPST_BIST1_ERR_CLR                      (0x01 << 5)
#define BIT_PAGE_0_EPST_BIST_ERR_CLR                       (0x01 << 1)
#define BIT_PAGE_0_EPST_CMD_DONE_CLR                       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xFA I2C Command Register                               (Default: 0x00)    */
#define REG_PAGE_0_EPCM                                    TX_PAGE_0, 0xFA

#define BIT_PAGE_0_EPCM_LD_KSV                             (0x01 << 5)
#define MSK_PAGE_0_EPCM_EPCM                               (0x1F << 0)

/*----------------------------------------------------------------------------*/
/* 0xFB I2C Command Register                               (Default: 0x20)    */
#define REG_PAGE_0_OTP_CLK                                 TX_PAGE_0, 0xFB

#define BIT_PAGE_0_OTP_CLK_OTP_PROTECT_EN                  (0x01 << 5)
#define BIT_PAGE_0_OTP_CLK_BYP_OCLKRING_OPTION             (0x01 << 3)
#define MSK_PAGE_0_OTP_CLK_OCLKDIV                         (0x03 << 1)
#define BIT_PAGE_0_OTP_CLK_BYPASS_HISC_CLK                 (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xFC I2C Address for Page 1                             (Default: 0x00)    */
#define REG_PAGE_0_PAGE_1_ADDR                             TX_PAGE_0, 0xFC

#define MSK_PAGE_0_PAGE_1_ADDR_PAGE_1_ADDR_7_0             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xFD I2C Address for Page 2                             (Default: 0x00)    */
#define REG_PAGE_0_PAGE_2_ADDR                             TX_PAGE_0, 0xFD

#define MSK_PAGE_0_PAGE_2_ADDR_PAGE_2_ADDR_7_0             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xFE I2C Address for Page 3                             (Default: 0x00)    */
#define REG_PAGE_0_PAGE_3_ADDR                             TX_PAGE_0, 0xFE

#define MSK_PAGE_0_PAGE_3_ADDR_PAGE_3_ADDR_7_0             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xFF I2C Address for HW TPI                             (Default: 0x00)    */
#define REG_PAGE_0_HW_TPI_ADDR                             TX_PAGE_0, 0xFF

#define MSK_PAGE_0_HW_TPI_ADDR_HW_TPI_ADDR_7_0             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* Registers in TX_PAGE_1                                                     */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* 0x00 USBT CTRL0 Register                                (Default: 0x00)    */
#define REG_PAGE_1_UTSRST                                  TX_PAGE_1, 0x00

#define BIT_PAGE_1_UTSRST_FC_SRST                          (0x01 << 5)
#define BIT_PAGE_1_UTSRST_KEEPER_SRST                      (0x01 << 4)
#define BIT_PAGE_1_UTSRST_HTX_SRST                         (0x01 << 3)
#define BIT_PAGE_1_UTSRST_TRX_SRST                         (0x01 << 2)
#define BIT_PAGE_1_UTSRST_TTX_SRST                         (0x01 << 1)
#define BIT_PAGE_1_UTSRST_HRX_SRST                         (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x01 HSIC RX Control0 Register                          (Default: 0x27)    */
#define REG_PAGE_1_HRXCTRL0                                TX_PAGE_1, 0x01

#define MSK_PAGE_1_HRXCTRL0_HRX_MARGIN_3_0                 (0x0F << 4)
#define BIT_PAGE_1_HRXCTRL0_HRX_ASSERT_RESET               (0x01 << 3)
#define BIT_PAGE_1_HRXCTRL0_HRX_EXTEND                     (0x01 << 2)
#define BIT_PAGE_1_HRXCTRL0_HRX_NULL_STUFFING              (0x01 << 1)
#define BIT_PAGE_1_HRXCTRL0_HRX_STOP_RESET                 (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x02 HSIC RX Control1 Register                          (Default: 0x42)    */
#define REG_PAGE_1_HRXCTRL1                                TX_PAGE_1, 0x02

#define MSK_PAGE_1_HRXCTRL1_HRX_RESET                      (0x0F << 4)
#define MSK_PAGE_1_HRXCTRL1_HRX_RESUME                     (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x03 HSIC RX Control2 Register                          (Default: 0x10)    */
#define REG_PAGE_1_HRXCTRL2                                TX_PAGE_1, 0x03

#define MSK_PAGE_1_HRXCTRL2_HRX_STAY_THRESH                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x04 HSIC RX Control3 Register                          (Default: 0x05)    */
#define REG_PAGE_1_HRXCTRL3                                TX_PAGE_1, 0x04

#define MSK_PAGE_1_HRXCTRL3_HRX_AFFCTRL                    (0x0F << 4)
#define BIT_PAGE_1_HRXCTRL3_HRX_OUT_EN                     (0x01 << 2)
#define BIT_PAGE_1_HRXCTRL3_STATUS_EN                      (0x01 << 1)
#define BIT_PAGE_1_HRXCTRL3_HRX_STAY_RESET                 (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x05 HSIC RX Control4 Register                          (Default: 0x03)    */
#define REG_PAGE_1_HRXCTRL4                                TX_PAGE_1, 0x05

#define MSK_PAGE_1_HRXCTRL4_HRX_BUFFER                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x06 HSIC RX Write Backoff Register                     (Default: 0x20)    */
#define REG_PAGE_1_HRXWRBKOF                               TX_PAGE_1, 0x06

#define MSK_PAGE_1_HRXWRBKOF_HRX_WRBKOF                    (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x07 HSIC RX Override Low Register                      (Default: 0x00)    */
#define REG_PAGE_1_HRXOVRL                                 TX_PAGE_1, 0x07

#define MSK_PAGE_1_HRXOVRL_HRX_OVERRIDE_7_0                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x08 HSIC RX Override High Register                     (Default: 0x00)    */
#define REG_PAGE_1_HRXOVRH                                 TX_PAGE_1, 0x08

#define MSK_PAGE_1_HRXOVRH_HRX_OVERRIDE_10_8               (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0x09 HSIC RX Status 1st Register                        (Default: 0x00)    */
#define REG_PAGE_1_HRXSTA1                                 TX_PAGE_1, 0x09

#define MSK_PAGE_1_HRXSTA1_HRX_STATUS_7_0                  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0A HSIC RX Status 2nd Register                        (Default: 0x00)    */
#define REG_PAGE_1_HRXSTA2                                 TX_PAGE_1, 0x0A

#define MSK_PAGE_1_HRXSTA2_HRX_STATUS_15_8                 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0B HSIC RX Status 3rd Register                        (Default: 0x00)    */
#define REG_PAGE_1_HRXSTA3                                 TX_PAGE_1, 0x0B

#define MSK_PAGE_1_HRXSTA3_HRX_STATUS_23_16                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0C HSIC RX Status 4th Register                        (Default: 0x00)    */
#define REG_PAGE_1_HRXSTA4                                 TX_PAGE_1, 0x0C

#define MSK_PAGE_1_HRXSTA4_HRX_STATUS_31_24                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0D HSIC RX Status 5th Register                        (Default: 0x00)    */
#define REG_PAGE_1_HRXSTA5                                 TX_PAGE_1, 0x0D

#define MSK_PAGE_1_HRXSTA5_HRX_STATUS_39_32                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0E HSIC RX Status 6th Register                        (Default: 0x00)    */
#define REG_PAGE_1_HRXSTA6                                 TX_PAGE_1, 0x0E

#define MSK_PAGE_1_HRXSTA6_HRX_STATUS_47_40                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0F HSIC RX Status 7th Register                        (Default: 0x00)    */
#define REG_PAGE_1_HRXSTA7                                 TX_PAGE_1, 0x0F

#define MSK_PAGE_1_HRXSTA7_HRX_STATUS_55_48                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x10 HSIC RX Status 8th Register                        (Default: 0x00)    */
#define REG_PAGE_1_HRXSTA8                                 TX_PAGE_1, 0x10

#define MSK_PAGE_1_HRXSTA8_HRX_STATUS_63_56                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x11 HSIC RX INT Low Register                           (Default: 0x00)    */
#define REG_PAGE_1_HRXINTL                                 TX_PAGE_1, 0x11

#define BIT_PAGE_1_HRXINTL_HRX_INTR7                       (0x01 << 7)
#define BIT_PAGE_1_HRXINTL_HRX_INTR6                       (0x01 << 6)
#define BIT_PAGE_1_HRXINTL_HRX_INTR5                       (0x01 << 5)
#define BIT_PAGE_1_HRXINTL_HRX_INTR4                       (0x01 << 4)
#define BIT_PAGE_1_HRXINTL_HRX_INTR3                       (0x01 << 3)
#define BIT_PAGE_1_HRXINTL_HRX_INTR2                       (0x01 << 2)
#define BIT_PAGE_1_HRXINTL_HRX_INTR1                       (0x01 << 1)
#define BIT_PAGE_1_HRXINTL_HRX_INTR0                       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x12 HSIC RX INT High Register                          (Default: 0x00)    */
#define REG_PAGE_1_HRXINTH                                 TX_PAGE_1, 0x12

#define BIT_PAGE_1_HRXINTH_HRX_INTR15                      (0x01 << 7)
#define BIT_PAGE_1_HRXINTH_HRX_INTR14                      (0x01 << 6)
#define BIT_PAGE_1_HRXINTH_HRX_INTR13                      (0x01 << 5)
#define BIT_PAGE_1_HRXINTH_HRX_INTR12                      (0x01 << 4)
#define BIT_PAGE_1_HRXINTH_HRX_INTR11                      (0x01 << 3)
#define BIT_PAGE_1_HRXINTH_HRX_INTR10                      (0x01 << 2)
#define BIT_PAGE_1_HRXINTH_HRX_INTR9                       (0x01 << 1)
#define BIT_PAGE_1_HRXINTH_HRX_INTR8                       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x13 HSIC RX INTMASK Low Register                       (Default: 0x00)    */
#define REG_PAGE_1_HRXINTML                                TX_PAGE_1, 0x13

#define BIT_PAGE_1_HRXINTML_HRX_INTRMASK7                  (0x01 << 7)
#define BIT_PAGE_1_HRXINTML_HRX_INTRMASK6                  (0x01 << 6)
#define BIT_PAGE_1_HRXINTML_HRX_INTRMASK5                  (0x01 << 5)
#define BIT_PAGE_1_HRXINTML_HRX_INTRMASK4                  (0x01 << 4)
#define BIT_PAGE_1_HRXINTML_HRX_INTRMASK3                  (0x01 << 3)
#define BIT_PAGE_1_HRXINTML_HRX_INTRMASK2                  (0x01 << 2)
#define BIT_PAGE_1_HRXINTML_HRX_INTRMASK1                  (0x01 << 1)
#define BIT_PAGE_1_HRXINTML_HRX_INTRMASK0                  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x14 HSIC RX INTMASK High Register                      (Default: 0x00)    */
#define REG_PAGE_1_HRXINTMH                                TX_PAGE_1, 0x14

#define BIT_PAGE_1_HRXINTMH_HRX_INTRMASK15                 (0x01 << 7)
#define BIT_PAGE_1_HRXINTMH_HRX_INTRMASK14                 (0x01 << 6)
#define BIT_PAGE_1_HRXINTMH_HRX_INTRMASK13                 (0x01 << 5)
#define BIT_PAGE_1_HRXINTMH_HRX_INTRMASK12                 (0x01 << 4)
#define BIT_PAGE_1_HRXINTMH_HRX_INTRMASK11                 (0x01 << 3)
#define BIT_PAGE_1_HRXINTMH_HRX_INTRMASK10                 (0x01 << 2)
#define BIT_PAGE_1_HRXINTMH_HRX_INTRMASK9                  (0x01 << 1)
#define BIT_PAGE_1_HRXINTMH_HRX_INTRMASK8                  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x15 TDM TX Buf Register                                (Default: 0x08)    */
#define REG_PAGE_1_TTXBUF                                  TX_PAGE_1, 0x15

#define MSK_PAGE_1_TTXBUF_TTX_BUFFER_7_0                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x16 TDM TX NUMBITS Register                            (Default: 0x04)    */
#define REG_PAGE_1_TTXNUMB                                 TX_PAGE_1, 0x16

#define MSK_PAGE_1_TTXNUMB_TTX_AFFCTRL_3_0                 (0x0F << 4)
#define MSK_PAGE_1_TTXNUMB_TTX_NUMBPS_2_0                  (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0x17 TDM TX NUMSPISYM Register                          (Default: 0x04)    */
#define REG_PAGE_1_TTXSPINUMS                              TX_PAGE_1, 0x17

#define MSK_PAGE_1_TTXSPINUMS_TTX_NUMSPISYM                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x18 TDM TX NUMHSICSYM Register                         (Default: 0x14)    */
#define REG_PAGE_1_TTXHSICNUMS                             TX_PAGE_1, 0x18

#define MSK_PAGE_1_TTXHSICNUMS_TTX_NUMHSICSYM              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x19 TDM TX NUMTOTSYM Register                          (Default: 0x18)    */
#define REG_PAGE_1_TTXTOTNUMS                              TX_PAGE_1, 0x19

#define MSK_PAGE_1_TTXTOTNUMS_TTX_NUMTOTSYM                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x1A TDM TX SYMSOP0 Low Register                        (Default: 0x55)    */
#define REG_PAGE_1_TTXSSOP0L                               TX_PAGE_1, 0x1A

#define MSK_PAGE_1_TTXSSOP0L_TTX_SYMSOP0_7_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x1B TDM TX SYMSOP0 High Register                       (Default: 0x00)    */
#define REG_PAGE_1_TTXSSOP0H                               TX_PAGE_1, 0x1B

#define MSK_PAGE_1_TTXSSOP0H_TTX_SYMSOP0_9_8               (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x1C TDM TX SYMSOP1 Low Register                        (Default: 0xAA)    */
#define REG_PAGE_1_TTXSSOP1L                               TX_PAGE_1, 0x1C

#define MSK_PAGE_1_TTXSSOP1L_TTX_SYMSOP1_7_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x1D TDM TX SYMSOP1 High Register                       (Default: 0x00)    */
#define REG_PAGE_1_TTXSSOP1H                               TX_PAGE_1, 0x1D

#define MSK_PAGE_1_TTXSSOP1H_TTX_SYMSOP1_9_8               (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x1E TDM TX SYMEOP0 Low Register                        (Default: 0x00)    */
#define REG_PAGE_1_TTXSEOP0L                               TX_PAGE_1, 0x1E

#define MSK_PAGE_1_TTXSEOP0L_TTX_SYMEOP0_7_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x1F TDM TX SYMEOP0 High Register                       (Default: 0x00)    */
#define REG_PAGE_1_TTXSEOP0H                               TX_PAGE_1, 0x1F

#define MSK_PAGE_1_TTXSEOP0H_TTX_SYMEOP0_9_8               (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x20 TDM TX SYMEOP1 Low Register                        (Default: 0xFF)    */
#define REG_PAGE_1_TTXSEOP1L                               TX_PAGE_1, 0x20

#define MSK_PAGE_1_TTXSEOP1L_TTX_SYMEOP1_7_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x21 TDM TX SYMEOP1 High Register                       (Default: 0x00)    */
#define REG_PAGE_1_TTXSEOP1H                               TX_PAGE_1, 0x21

#define MSK_PAGE_1_TTXSEOP1H_TTX_SYMEOP1_9_8               (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x22 TDM TX SYMCOM1 Low Register                        (Default: 0xA5)    */
#define REG_PAGE_1_TTXSCOM1L                               TX_PAGE_1, 0x22

#define MSK_PAGE_1_TTXSCOM1L_TTX_SYMCOM1_7_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x23 TDM TX SYMCOM1 High Register                       (Default: 0x00)    */
#define REG_PAGE_1_TTXSCOM1H                               TX_PAGE_1, 0x23

#define MSK_PAGE_1_TTXSCOM1H_TTX_SYMCOM1_9_8               (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x24 TDM TX SYMCOM2 Low Register                        (Default: 0x5A)    */
#define REG_PAGE_1_TTXSCOM2L                               TX_PAGE_1, 0x24

#define MSK_PAGE_1_TTXSCOM2L_TTX_SYMCOM2_7_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x25 TDM TX SYMCOM2 High Register                       (Default: 0x00)    */
#define REG_PAGE_1_TTXSCOM2H                               TX_PAGE_1, 0x25

#define MSK_PAGE_1_TTXSCOM2H_TTX_SYMCOM2_9_8               (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x26 TDM TX SYMIDLE Low Register                        (Default: 0x00)    */
#define REG_PAGE_1_TTXSIDLEL                               TX_PAGE_1, 0x26

#define MSK_PAGE_1_TTXSIDLEL_TTX_SYMIDLE_7_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x27 TDM TX SYMIDLE High Register                       (Default: 0x00)    */
#define REG_PAGE_1_TTXSIDLEH                               TX_PAGE_1, 0x27

#define MSK_PAGE_1_TTXSIDLEH_TTX_SYMIDLE_9_8               (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x28 TDM TX SYMRST Low Register                         (Default: 0x93)    */
#define REG_PAGE_1_TTXSRSTL                                TX_PAGE_1, 0x28

#define MSK_PAGE_1_TTXSRSTL_TTX_SYMRST_7_0                 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x29 TDM TX SYMRST High Register                        (Default: 0x00)    */
#define REG_PAGE_1_TTXSRSTH                                TX_PAGE_1, 0x29

#define MSK_PAGE_1_TTXSRSTH_TTX_SYMRST_9_8                 (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x2A TDM TX SYMCONN Low Register                        (Default: 0xC6)    */
#define REG_PAGE_1_TTXSCONNL                               TX_PAGE_1, 0x2A

#define MSK_PAGE_1_TTXSCONNL_TTX_SYMCONN_7_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x2B TDM TX SYMCONN High Register                       (Default: 0x00)    */
#define REG_PAGE_1_TTXSCONNH                               TX_PAGE_1, 0x2B

#define MSK_PAGE_1_TTXSCONNH_TTX_SYMCONN_9_8               (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x2C TDM TX SYMCBUS Low Register                        (Default: 0x39)    */
#define REG_PAGE_1_TTXSCBUSL                               TX_PAGE_1, 0x2C

#define MSK_PAGE_1_TTXSCBUSL_TTX_SYMCBUS_7_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x2D TDM TX SYMCBUS High Register                       (Default: 0x00)    */
#define REG_PAGE_1_TTXSCBUSH                               TX_PAGE_1, 0x2D

#define MSK_PAGE_1_TTXSCBUSH_TTX_SYMCBUS_9_8               (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x2E TDM TX SYMCBUSA Low Register                       (Default: 0x63)    */
#define REG_PAGE_1_TTXSCBUSAL                              TX_PAGE_1, 0x2E

#define MSK_PAGE_1_TTXSCBUSAL_TTX_SYMCBUSA_7_0             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x2F TDM TX SYMCBUSA High Register                      (Default: 0x00)    */
#define REG_PAGE_1_TTXSCBUSAH                              TX_PAGE_1, 0x2F

#define MSK_PAGE_1_TTXSCBUSAH_TTX_SYMCBUSA_9_8             (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x30 TDM TX SYMCBUSN Low Register                       (Default: 0x6C)    */
#define REG_PAGE_1_TTXSCBUSNL                              TX_PAGE_1, 0x30

#define MSK_PAGE_1_TTXSCBUSNL_TTX_SYMCBUSN_7_0             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x31 TDM TX SYMCBUSN High Register                      (Default: 0x00)    */
#define REG_PAGE_1_TTXSCBUSNH                              TX_PAGE_1, 0x31

#define MSK_PAGE_1_TTXSCBUSNH_TTX_SYMCBUSN_9_8             (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x32 TDM TX Status 1st Register                         (Default: 0x00)    */
#define REG_PAGE_1_TTXSTA1                                 TX_PAGE_1, 0x32

#define MSK_PAGE_1_TTXSTA1_TTX_STATUS_7_0                  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x33 TDM TX Status 2nd Register                         (Default: 0x00)    */
#define REG_PAGE_1_TTXSTA2                                 TX_PAGE_1, 0x33

#define MSK_PAGE_1_TTXSTA2_TTX_STATUS_15_8                 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x34 TDM TX Status 3rd Register                         (Default: 0x00)    */
#define REG_PAGE_1_TTXSTA3                                 TX_PAGE_1, 0x34

#define MSK_PAGE_1_TTXSTA3_TTX_STATUS_23_16                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x35 TDM TX Status 4th Register                         (Default: 0x00)    */
#define REG_PAGE_1_TTXSTA4                                 TX_PAGE_1, 0x35

#define MSK_PAGE_1_TTXSTA4_TTX_STATUS_31_24                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x36 TDM TX INT Low Register                            (Default: 0x00)    */
#define REG_PAGE_1_TTXINTL                                 TX_PAGE_1, 0x36

#define BIT_PAGE_1_TTXINTL_TTX_INTR7                       (0x01 << 7)
#define BIT_PAGE_1_TTXINTL_TTX_INTR6                       (0x01 << 6)
#define BIT_PAGE_1_TTXINTL_TTX_INTR5                       (0x01 << 5)
#define BIT_PAGE_1_TTXINTL_TTX_INTR4                       (0x01 << 4)
#define BIT_PAGE_1_TTXINTL_TTX_INTR3                       (0x01 << 3)
#define BIT_PAGE_1_TTXINTL_TTX_INTR2                       (0x01 << 2)
#define BIT_PAGE_1_TTXINTL_TTX_INTR1                       (0x01 << 1)
#define BIT_PAGE_1_TTXINTL_TTX_INTR0                       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x37 TDM TX INT High Register                           (Default: 0x00)    */
#define REG_PAGE_1_TTXINTH                                 TX_PAGE_1, 0x37

#define BIT_PAGE_1_TTXINTH_TTX_INTR15                      (0x01 << 7)
#define BIT_PAGE_1_TTXINTH_TTX_INTR14                      (0x01 << 6)
#define BIT_PAGE_1_TTXINTH_TTX_INTR13                      (0x01 << 5)
#define BIT_PAGE_1_TTXINTH_TTX_INTR12                      (0x01 << 4)
#define BIT_PAGE_1_TTXINTH_TTX_INTR11                      (0x01 << 3)
#define BIT_PAGE_1_TTXINTH_TTX_INTR10                      (0x01 << 2)
#define BIT_PAGE_1_TTXINTH_TTX_INTR9                       (0x01 << 1)
#define BIT_PAGE_1_TTXINTH_TTX_INTR8                       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x38 TDM TX INTMASK Low Register                        (Default: 0x00)    */
#define REG_PAGE_1_TTXINTML                                TX_PAGE_1, 0x38

#define BIT_PAGE_1_TTXINTML_TTX_INTRMASK7                  (0x01 << 7)
#define BIT_PAGE_1_TTXINTML_TTX_INTRMASK6                  (0x01 << 6)
#define BIT_PAGE_1_TTXINTML_TTX_INTRMASK5                  (0x01 << 5)
#define BIT_PAGE_1_TTXINTML_TTX_INTRMASK4                  (0x01 << 4)
#define BIT_PAGE_1_TTXINTML_TTX_INTRMASK3                  (0x01 << 3)
#define BIT_PAGE_1_TTXINTML_TTX_INTRMASK2                  (0x01 << 2)
#define BIT_PAGE_1_TTXINTML_TTX_INTRMASK1                  (0x01 << 1)
#define BIT_PAGE_1_TTXINTML_TTX_INTRMASK0                  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x39 TDM TX INTMASK High Register                       (Default: 0x00)    */
#define REG_PAGE_1_TTXINTMH                                TX_PAGE_1, 0x39

#define BIT_PAGE_1_TTXINTMH_TTX_INTRMASK15                 (0x01 << 7)
#define BIT_PAGE_1_TTXINTMH_TTX_INTRMASK14                 (0x01 << 6)
#define BIT_PAGE_1_TTXINTMH_TTX_INTRMASK13                 (0x01 << 5)
#define BIT_PAGE_1_TTXINTMH_TTX_INTRMASK12                 (0x01 << 4)
#define BIT_PAGE_1_TTXINTMH_TTX_INTRMASK11                 (0x01 << 3)
#define BIT_PAGE_1_TTXINTMH_TTX_INTRMASK10                 (0x01 << 2)
#define BIT_PAGE_1_TTXINTMH_TTX_INTRMASK9                  (0x01 << 1)
#define BIT_PAGE_1_TTXINTMH_TTX_INTRMASK8                  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x3A TDM RX Output Enable Register                      (Default: 0x3F)    */
#define REG_PAGE_1_TRXOUTEN                                TX_PAGE_1, 0x3A

#define MSK_PAGE_1_TRXOUTEN_TRX_OUT_EN                     (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0x3B TDM RX Control Register                            (Default: 0x1C)    */
#define REG_PAGE_1_TRXCTRL                                 TX_PAGE_1, 0x3B

#define BIT_PAGE_1_TRXCTRL_TRX_CLR_WVALLOW                 (0x01 << 4)
#define BIT_PAGE_1_TRXCTRL_TRX_FROM_SE_COC                 (0x01 << 3)
#define MSK_PAGE_1_TRXCTRL_TRX_NUMBPS_2_0                  (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0x3C TDM RX NUMSPISYM Register                          (Default: 0x04)    */
#define REG_PAGE_1_TRXSPINUMS                              TX_PAGE_1, 0x3C

#define MSK_PAGE_1_TRXSPINUMS_TRX_NUMSPISYM                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x3D TDM RX NUMHSICSYM Register                         (Default: 0x14)    */
#define REG_PAGE_1_TRXHSICNUMS                             TX_PAGE_1, 0x3D

#define MSK_PAGE_1_TRXHSICNUMS_TRX_NUMHSICSYM              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x3E TDM RX NUMTOTSYM Register                          (Default: 0x18)    */
#define REG_PAGE_1_TRXTOTNUMS                              TX_PAGE_1, 0x3E

#define MSK_PAGE_1_TRXTOTNUMS_TRX_NUMTOTSYM                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x3F TDM RX MIN12MCHPRD Register                        (Default: 0x08)    */
#define REG_PAGE_1_TRXMIN12                                TX_PAGE_1, 0x3F

#define MSK_PAGE_1_TRXMIN12_TRX_MIN12MP                    (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x40 TDM RX MIN2MCHPRD Register                         (Default: 0x08)    */
#define REG_PAGE_1_TRXMIN2                                 TX_PAGE_1, 0x40

#define MSK_PAGE_1_TRXMIN2_TRX_MIN2MP                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x41 TDM RX MAX12MCHPRD Register                        (Default: 0x0A)    */
#define REG_PAGE_1_TRXMAX12                                TX_PAGE_1, 0x41

#define MSK_PAGE_1_TRXMAX12_TRX_MAX12MP                    (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x42 TDM RX MAX2MCHPRD Register                         (Default: 0x0A)    */
#define REG_PAGE_1_TRXMAX2                                 TX_PAGE_1, 0x42

#define MSK_PAGE_1_TRXMAX2_TRX_MAX2MP                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x5B TDM RX Status 1st Register                         (Default: 0x00)    */
#define REG_PAGE_1_TRXSTA1                                 TX_PAGE_1, 0x5B

#define MSK_PAGE_1_TRXSTA1_TRX_STATUS_7_0                  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x5C TDM RX Status 2nd Register                         (Default: 0x00)    */
#define REG_PAGE_1_TRXSTA2                                 TX_PAGE_1, 0x5C

#define MSK_PAGE_1_TRXSTA2_TRX_STATUS_15_8                 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x5D TDM RX Status 3rd Register                         (Default: 0x00)    */
#define REG_PAGE_1_TRXSTA3                                 TX_PAGE_1, 0x5D

#define MSK_PAGE_1_TRXSTA3_TRX_STATUS_23_16                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x5E TDM RX Status 4th Register                         (Default: 0x00)    */
#define REG_PAGE_1_TRXSTA4                                 TX_PAGE_1, 0x5E

#define MSK_PAGE_1_TRXSTA4_TRX_STATUS_31_24                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x5F TDM RX Status 5th Register                         (Default: 0x00)    */
#define REG_PAGE_1_TRXSTA5                                 TX_PAGE_1, 0x5F

#define MSK_PAGE_1_TRXSTA5_TRX_STATUS_39_32                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x60 TDM RX Status 6th Register                         (Default: 0x00)    */
#define REG_PAGE_1_TRXSTA6                                 TX_PAGE_1, 0x60

#define MSK_PAGE_1_TRXSTA6_TRX_STATUS_47_40                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x61 TDM RX Status 7th Register                         (Default: 0x00)    */
#define REG_PAGE_1_TRXSTA7                                 TX_PAGE_1, 0x61

#define MSK_PAGE_1_TRXSTA7_TRX_STATUS_55_48                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x62 TDM RX Status 8th Register                         (Default: 0x00)    */
#define REG_PAGE_1_TRXSTA8                                 TX_PAGE_1, 0x62

#define MSK_PAGE_1_TRXSTA8_TRX_STATUS_63_56                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x63 TDM RX INT Low Register                            (Default: 0x00)    */
#define REG_PAGE_1_TRXINTL                                 TX_PAGE_1, 0x63

#define BIT_PAGE_1_TRXINTL_TRX_INTR7                       (0x01 << 7)
#define BIT_PAGE_1_TRXINTL_TRX_INTR6                       (0x01 << 6)
#define BIT_PAGE_1_TRXINTL_TRX_INTR5                       (0x01 << 5)
#define BIT_PAGE_1_TRXINTL_TRX_INTR4                       (0x01 << 4)
#define BIT_PAGE_1_TRXINTL_TRX_INTR3                       (0x01 << 3)
#define BIT_PAGE_1_TRXINTL_TRX_INTR2                       (0x01 << 2)
#define BIT_PAGE_1_TRXINTL_TRX_INTR1                       (0x01 << 1)
#define BIT_PAGE_1_TRXINTL_TRX_INTR0                       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x64 TDM RX INT High Register                           (Default: 0x00)    */
#define REG_PAGE_1_TRXINTH                                 TX_PAGE_1, 0x64

#define BIT_PAGE_1_TRXINTH_TRX_INTR15                      (0x01 << 7)
#define BIT_PAGE_1_TRXINTH_TRX_INTR14                      (0x01 << 6)
#define BIT_PAGE_1_TRXINTH_TRX_INTR13                      (0x01 << 5)
#define BIT_PAGE_1_TRXINTH_TRX_INTR12                      (0x01 << 4)
#define BIT_PAGE_1_TRXINTH_TRX_INTR11                      (0x01 << 3)
#define BIT_PAGE_1_TRXINTH_TRX_INTR10                      (0x01 << 2)
#define BIT_PAGE_1_TRXINTH_TRX_INTR9                       (0x01 << 1)
#define BIT_PAGE_1_TRXINTH_TRX_INTR8                       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x65 TDM RX INTMASK Low Register                        (Default: 0x00)    */
#define REG_PAGE_1_TRXINTML                                TX_PAGE_1, 0x65

#define BIT_PAGE_1_TRXINTML_TRX_INTRMASK7                  (0x01 << 7)
#define BIT_PAGE_1_TRXINTML_TRX_INTRMASK6                  (0x01 << 6)
#define BIT_PAGE_1_TRXINTML_TRX_INTRMASK5                  (0x01 << 5)
#define BIT_PAGE_1_TRXINTML_TRX_INTRMASK4                  (0x01 << 4)
#define BIT_PAGE_1_TRXINTML_TRX_INTRMASK3                  (0x01 << 3)
#define BIT_PAGE_1_TRXINTML_TRX_INTRMASK2                  (0x01 << 2)
#define BIT_PAGE_1_TRXINTML_TRX_INTRMASK1                  (0x01 << 1)
#define BIT_PAGE_1_TRXINTML_TRX_INTRMASK0                  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x66 TDM RX INTMASK High Register                       (Default: 0x00)    */
#define REG_PAGE_1_TRXINTMH                                TX_PAGE_1, 0x66

#define BIT_PAGE_1_TRXINTMH_TRX_INTRMASK15                 (0x01 << 7)
#define BIT_PAGE_1_TRXINTMH_TRX_INTRMASK14                 (0x01 << 6)
#define BIT_PAGE_1_TRXINTMH_TRX_INTRMASK13                 (0x01 << 5)
#define BIT_PAGE_1_TRXINTMH_TRX_INTRMASK12                 (0x01 << 4)
#define BIT_PAGE_1_TRXINTMH_TRX_INTRMASK11                 (0x01 << 3)
#define BIT_PAGE_1_TRXINTMH_TRX_INTRMASK10                 (0x01 << 2)
#define BIT_PAGE_1_TRXINTMH_TRX_INTRMASK9                  (0x01 << 1)
#define BIT_PAGE_1_TRXINTMH_TRX_INTRMASK8                  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x67 HSIC TX Strobet EOP Register                       (Default: 0x38)    */
#define REG_PAGE_1_HTXSTEOP                                TX_PAGE_1, 0x67

#define MSK_PAGE_1_HTXSTEOP_HTX_ST_AEOP_5_0                (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0x68 HSIC TX Datat EOP Register                         (Default: 0x38)    */
#define REG_PAGE_1_HTXDTEOP                                TX_PAGE_1, 0x68

#define MSK_PAGE_1_HTXDTEOP_HTX_DT_AEOP_5_0                (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0x69 HSIC TX CRTL Register                              (Default: 0x00)    */
#define REG_PAGE_1_HTXCTRL                                 TX_PAGE_1, 0x69

#define BIT_PAGE_1_HTXCTRL_HTX_ALLSBE_SOP                  (0x01 << 4)
#define BIT_PAGE_1_HTXCTRL_HTX_RGDINV_USB                  (0x01 << 3)
#define BIT_PAGE_1_HTXCTRL_HTX_RSPTDM_BUSY                 (0x01 << 2)
#define BIT_PAGE_1_HTXCTRL_HTX_DRVCONN1                    (0x01 << 1)
#define BIT_PAGE_1_HTXCTRL_HTX_DRVRST1                     (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x6A HSIC TX AFIFO Control Register                     (Default: 0x00)    */
#define REG_PAGE_1_HTXAFFCTRL                              TX_PAGE_1, 0x6A

#define MSK_PAGE_1_HTXAFFCTRL_HTX_AFFCTRL_3_0              (0x0F << 4)

/*----------------------------------------------------------------------------*/
/* 0x6B HSIC TX BUSIDLEP 1st Register                      (Default: 0x00)    */
#define REG_PAGE_1_HTXBIDLEP0                              TX_PAGE_1, 0x6B

#define MSK_PAGE_1_HTXBIDLEP0_HTX_BUSIDPRD_7_0             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x6C HSIC TX BUSIDLEP 2nd Register                      (Default: 0x00)    */
#define REG_PAGE_1_HTXBIDLEP1                              TX_PAGE_1, 0x6C

#define MSK_PAGE_1_HTXBIDLEP1_HTX_BUSIDPRD_15_8            (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x6D HSIC TX BUSIDLEP 3rd Register                      (Default: 0x08)    */
#define REG_PAGE_1_HTXBIDLEP2                              TX_PAGE_1, 0x6D

#define MSK_PAGE_1_HTXBIDLEP2_HTX_BUSIDPRD_23_16           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x6E HSIC TX BUSIDLEP 4th Register                      (Default: 0x00)    */
#define REG_PAGE_1_HTXBIDLEP3                              TX_PAGE_1, 0x6E

#define MSK_PAGE_1_HTXBIDLEP3_HTX_BUSIDPRD_31_24           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x6F HSIC TX BUSIDLEC 1st Register                      (Default: 0x10)    */
#define REG_PAGE_1_HTXBIDLEC0                              TX_PAGE_1, 0x6F

#define MSK_PAGE_1_HTXBIDLEC0_HTX_BUSIDCHK_7_0             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x70 HSIC TX BUSIDLEC 2nd Register                      (Default: 0x00)    */
#define REG_PAGE_1_HTXBIDLEC1                              TX_PAGE_1, 0x70

#define MSK_PAGE_1_HTXBIDLEC1_HTX_BUSIDCHK_15_8            (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x71 HSIC TX BUSIDLEC 3rd Register                      (Default: 0x00)    */
#define REG_PAGE_1_HTXBIDLEC2                              TX_PAGE_1, 0x71

#define MSK_PAGE_1_HTXBIDLEC2_HTX_BUSIDCHK_23_16           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x72 HSIC TX BUSIDLEC 4th Register                      (Default: 0x00)    */
#define REG_PAGE_1_HTXBIDLEC3                              TX_PAGE_1, 0x72

#define MSK_PAGE_1_HTXBIDLEC3_HTX_BUSIDCHK_31_24           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x73 HSIC TX PRETOUT Low Register                       (Default: 0x10)    */
#define REG_PAGE_1_HTXPRETOUTL                             TX_PAGE_1, 0x73

#define MSK_PAGE_1_HTXPRETOUTL_HTX_STPRETOUT_7_0           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x74 HSIC TX PRETOUT High Register                      (Default: 0x00)    */
#define REG_PAGE_1_HTXPRETOUTH                             TX_PAGE_1, 0x74

#define MSK_PAGE_1_HTXPRETOUTH_HTX_STPRETOUT_15_8          (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x75 HSIC TX BUF Register                               (Default: 0x08)    */
#define REG_PAGE_1_HTXBUF                                  TX_PAGE_1, 0x75

#define MSK_PAGE_1_HTXBUF_HTX_BUF_7_0                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x76 HSIC TX NNULLEOP Register                          (Default: 0x10)    */
#define REG_PAGE_1_HTXNNEOP                                TX_PAGE_1, 0x76

#define MSK_PAGE_1_HTXNNEOP_HTX_NNFEOP                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x77 HSIC TX OVRHOUT Low Register                       (Default: 0x33)    */
#define REG_PAGE_1_HTXOVRHOL                               TX_PAGE_1, 0x77

#define MSK_PAGE_1_HTXOVRHOL_HTX_OVRHOUT_7_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x78 HSIC TX OVRHOUT High Register                      (Default: 0x00)    */
#define REG_PAGE_1_HTXOVRHOH                               TX_PAGE_1, 0x78

#define MSK_PAGE_1_HTXOVRHOH_HTX_OVRHOUT_9_8               (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x79 HSIC TX Status 1st Register                        (Default: 0x00)    */
#define REG_PAGE_1_HTXSTA1                                 TX_PAGE_1, 0x79

#define MSK_PAGE_1_HTXSTA1_HTX_STATUS_7_0                  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x7A HSIC TX Status 2nd Register                        (Default: 0x00)    */
#define REG_PAGE_1_HTXSTA2                                 TX_PAGE_1, 0x7A

#define MSK_PAGE_1_HTXSTA2_HTX_STATUS_15_8                 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x7B HSIC TX Status 3rd Register                        (Default: 0x00)    */
#define REG_PAGE_1_HTXSTA3                                 TX_PAGE_1, 0x7B

#define MSK_PAGE_1_HTXSTA3_HTX_STATUS_23_16                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x7C HSIC TX Status 4th Register                        (Default: 0x00)    */
#define REG_PAGE_1_HTXSTA4                                 TX_PAGE_1, 0x7C

#define MSK_PAGE_1_HTXSTA4_HTX_STATUS_31_24                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x7D HSIC TX INT Low Register                           (Default: 0x00)    */
#define REG_PAGE_1_HTXINTL                                 TX_PAGE_1, 0x7D

#define BIT_PAGE_1_HTXINTL_HTX_INTR7                       (0x01 << 7)
#define BIT_PAGE_1_HTXINTL_HTX_INTR6                       (0x01 << 6)
#define BIT_PAGE_1_HTXINTL_HTX_INTR5                       (0x01 << 5)
#define BIT_PAGE_1_HTXINTL_HTX_INTR4                       (0x01 << 4)
#define BIT_PAGE_1_HTXINTL_HTX_INTR3                       (0x01 << 3)
#define BIT_PAGE_1_HTXINTL_HTX_INTR2                       (0x01 << 2)
#define BIT_PAGE_1_HTXINTL_HTX_INTR1                       (0x01 << 1)
#define BIT_PAGE_1_HTXINTL_HTX_INTR0                       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x7E HSIC TX INT High Register                          (Default: 0x00)    */
#define REG_PAGE_1_HTXINTH                                 TX_PAGE_1, 0x7E

#define BIT_PAGE_1_HTXINTH_HTX_INTR15                      (0x01 << 7)
#define BIT_PAGE_1_HTXINTH_HTX_INTR14                      (0x01 << 6)
#define BIT_PAGE_1_HTXINTH_HTX_INTR13                      (0x01 << 5)
#define BIT_PAGE_1_HTXINTH_HTX_INTR12                      (0x01 << 4)
#define BIT_PAGE_1_HTXINTH_HTX_INTR11                      (0x01 << 3)
#define BIT_PAGE_1_HTXINTH_HTX_INTR10                      (0x01 << 2)
#define BIT_PAGE_1_HTXINTH_HTX_INTR9                       (0x01 << 1)
#define BIT_PAGE_1_HTXINTH_HTX_INTR8                       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x7F HSIC TX INTMASK Low Register                       (Default: 0x00)    */
#define REG_PAGE_1_HTXINTML                                TX_PAGE_1, 0x7F

#define BIT_PAGE_1_HTXINTML_HTX_INTRMASK7                  (0x01 << 7)
#define BIT_PAGE_1_HTXINTML_HTX_INTRMASK6                  (0x01 << 6)
#define BIT_PAGE_1_HTXINTML_HTX_INTRMASK5                  (0x01 << 5)
#define BIT_PAGE_1_HTXINTML_HTX_INTRMASK4                  (0x01 << 4)
#define BIT_PAGE_1_HTXINTML_HTX_INTRMASK3                  (0x01 << 3)
#define BIT_PAGE_1_HTXINTML_HTX_INTRMASK2                  (0x01 << 2)
#define BIT_PAGE_1_HTXINTML_HTX_INTRMASK1                  (0x01 << 1)
#define BIT_PAGE_1_HTXINTML_HTX_INTRMASK0                  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x80 HSIC TX INTMASK High Register                      (Default: 0x00)    */
#define REG_PAGE_1_HTXINTMH                                TX_PAGE_1, 0x80

#define BIT_PAGE_1_HTXINTMH_HTX_INTRMASK15                 (0x01 << 7)
#define BIT_PAGE_1_HTXINTMH_HTX_INTRMASK14                 (0x01 << 6)
#define BIT_PAGE_1_HTXINTMH_HTX_INTRMASK13                 (0x01 << 5)
#define BIT_PAGE_1_HTXINTMH_HTX_INTRMASK12                 (0x01 << 4)
#define BIT_PAGE_1_HTXINTMH_HTX_INTRMASK11                 (0x01 << 3)
#define BIT_PAGE_1_HTXINTMH_HTX_INTRMASK10                 (0x01 << 2)
#define BIT_PAGE_1_HTXINTMH_HTX_INTRMASK9                  (0x01 << 1)
#define BIT_PAGE_1_HTXINTMH_HTX_INTRMASK8                  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x81 HSIC Keeper Register                               (Default: 0x00)    */
#define REG_PAGE_1_KEEPER                                  TX_PAGE_1, 0x81

#define MSK_PAGE_1_KEEPER_KEEPER_MODE_1_0                  (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x82 HSIC Keeper StartEnd Register                      (Default: 0x34)    */
#define REG_PAGE_1_KPSTEND                                 TX_PAGE_1, 0x82

#define MSK_PAGE_1_KPSTEND_KEEPER_END_3_0                  (0x0F << 4)
#define MSK_PAGE_1_KPSTEND_KEEPER_START_3_0                (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x83 HSIC Flow Control General Register                 (Default: 0x02)    */
#define REG_PAGE_1_FCGC                                    TX_PAGE_1, 0x83

#define BIT_PAGE_1_FCGC_HSIC_FC_HOSTMODE                   (0x01 << 1)
#define BIT_PAGE_1_FCGC_HSIC_FC_ENABLE                     (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x84 HSIC Flow Control CTR0 Register                    (Default: 0x00)    */
#define REG_PAGE_1_FCCTR0                                  TX_PAGE_1, 0x84

#define MSK_PAGE_1_FCCTR0_HFC_CONF0                        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x85 HSIC Flow Control CTR1 Register                    (Default: 0x00)    */
#define REG_PAGE_1_FCCTR1                                  TX_PAGE_1, 0x85

#define MSK_PAGE_1_FCCTR1_HFC_CONF1                        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x86 HSIC Flow Control CTR2 Register                    (Default: 0x00)    */
#define REG_PAGE_1_FCCTR2                                  TX_PAGE_1, 0x86

#define MSK_PAGE_1_FCCTR2_HFC_CONF2                        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x87 HSIC Flow Control CTR3 Register                    (Default: 0x00)    */
#define REG_PAGE_1_FCCTR3                                  TX_PAGE_1, 0x87

#define MSK_PAGE_1_FCCTR3_HFC_CONF3                        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x88 HSIC Flow Control CTR4 Register                    (Default: 0x8F)    */
#define REG_PAGE_1_FCCTR4                                  TX_PAGE_1, 0x88


#define MSK_PAGE_1_FCCTR4_HFC_CONF4                        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x89 HSIC Flow Control CTR5 Register                    (Default: 0x80)    */
#define REG_PAGE_1_FCCTR5                                  TX_PAGE_1, 0x89

#define MSK_PAGE_1_FCCTR5_HFC_CONF5                        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x8A HSIC Flow Control CTR6 Register                    (Default: 0x13)    */
#define REG_PAGE_1_FCCTR6                                  TX_PAGE_1, 0x8A

#define MSK_PAGE_1_FCCTR6_HFC_CONF6                        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x8B HSIC Flow Control CTR7 Register                    (Default: 0x00)    */
#define REG_PAGE_1_FCCTR7                                  TX_PAGE_1, 0x8B

#define MSK_PAGE_1_FCCTR7_HFC_CONF7                        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x8C HSIC Flow Control CTR8 Register                    (Default: 0x8F)    */
#define REG_PAGE_1_FCCTR8                                  TX_PAGE_1, 0x8C

#define MSK_PAGE_1_FCCTR8_HFC_CONF8                        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x8D HSIC Flow Control CTR9 Register                    (Default: 0x08)    */
#define REG_PAGE_1_FCCTR9                                  TX_PAGE_1, 0x8D

#define MSK_PAGE_1_FCCTR9_HFC_CONF9                        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x8E HSIC Flow Control CTR10 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCCTR10                                 TX_PAGE_1, 0x8E

#define MSK_PAGE_1_FCCTR10_HFC_CONF10                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x8F HSIC Flow Control CTR11 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCCTR11                                 TX_PAGE_1, 0x8F

#define MSK_PAGE_1_FCCTR11_HFC_CONF11                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x90 HSIC Flow Control CTR12 Register                   (Default: 0xF9)    */
#define REG_PAGE_1_FCCTR12                                 TX_PAGE_1, 0x90

#define MSK_PAGE_1_FCCTR12_HFC_CONF12                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x91 HSIC Flow Control CTR13 Register                   (Default: 0xFC)    */
#define REG_PAGE_1_FCCTR13                                 TX_PAGE_1, 0x91

#define MSK_PAGE_1_FCCTR13_HFC_CONF13                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x92 HSIC Flow Control CTR14 Register                   (Default: 0xFF)    */
#define REG_PAGE_1_FCCTR14                                 TX_PAGE_1, 0x92

#define MSK_PAGE_1_FCCTR14_HFC_CONF14                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x93 HSIC Flow Control CTR15 Register                   (Default: 0x0F)    */
#define REG_PAGE_1_FCCTR15                                 TX_PAGE_1, 0x93

#define MSK_PAGE_1_FCCTR15_HFC_CONF15                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x94 HSIC Flow Control CTR16 Register                   (Default: 0x73)    */
#define REG_PAGE_1_FCCTR16                                 TX_PAGE_1, 0x94

#define MSK_PAGE_1_FCCTR16_HFC_CONF16                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x95 HSIC Flow Control CTR17 Register                   (Default: 0x1E)    */
#define REG_PAGE_1_FCCTR17                                 TX_PAGE_1, 0x95

#define MSK_PAGE_1_FCCTR17_HFC_CONF17                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x96 HSIC Flow Control CTR18 Register                   (Default: 0x05)    */
#define REG_PAGE_1_FCCTR18                                 TX_PAGE_1, 0x96

#define MSK_PAGE_1_FCCTR18_HFC_CONF18                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x97 HSIC Flow Control CTR19 Register                   (Default: 0x09)    */
#define REG_PAGE_1_FCCTR19                                 TX_PAGE_1, 0x97

#define MSK_PAGE_1_FCCTR19_HFC_CONF19                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x98 HSIC Flow Control CTR20 Register                   (Default: 0x68)    */
#define REG_PAGE_1_FCCTR20                                 TX_PAGE_1, 0x98

#define MSK_PAGE_1_FCCTR20_HFC_CONF20                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x99 HSIC Flow Control CTR21 Register                   (Default: 0x26)    */
#define REG_PAGE_1_FCCTR21                                 TX_PAGE_1, 0x99

#define MSK_PAGE_1_FCCTR21_HFC_CONF21                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x9A HSIC Flow Control CTR22 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCCTR22                                 TX_PAGE_1, 0x9A

#define MSK_PAGE_1_FCCTR22_HFC_CONF22                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x9B HSIC Flow Control CTR23 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCCTR23                                 TX_PAGE_1, 0x9B

#define MSK_PAGE_1_FCCTR23_HFC_CONF23                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x9C HSIC Flow Control CTR24 Register                   (Default: 0xE2)    */
#define REG_PAGE_1_FCCTR24                                 TX_PAGE_1, 0x9C

#define MSK_PAGE_1_FCCTR24_HFC_CONF24                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x9D HSIC Flow Control CTR25 Register                   (Default: 0x04)    */
#define REG_PAGE_1_FCCTR25                                 TX_PAGE_1, 0x9D

#define MSK_PAGE_1_FCCTR25_HFC_CONF25                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x9E HSIC Flow Control CTR26 Register                   (Default: 0x4C)    */
#define REG_PAGE_1_FCCTR26                                 TX_PAGE_1, 0x9E

#define MSK_PAGE_1_FCCTR26_HFC_CONF26                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x9F HSIC Flow Control CTR27 Register                   (Default: 0x1D)    */
#define REG_PAGE_1_FCCTR27                                 TX_PAGE_1, 0x9F

#define MSK_PAGE_1_FCCTR27_HFC_CONF27                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA0 HSIC Flow Control CTR28 Register                   (Default: 0xE4)    */
#define REG_PAGE_1_FCCTR28                                 TX_PAGE_1, 0xA0

#define MSK_PAGE_1_FCCTR28_HFC_CONF28                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA1 HSIC Flow Control CTR29 Register                   (Default: 0x57)    */
#define REG_PAGE_1_FCCTR29                                 TX_PAGE_1, 0xA1

#define MSK_PAGE_1_FCCTR29_HFC_CONF29                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA2 HSIC Flow Control CTR30 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCCTR30                                 TX_PAGE_1, 0xA2

#define MSK_PAGE_1_FCCTR30_HFC_CONF30                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA3 HSIC Flow Control CTR31 Register                   (Default: 0x04)    */
#define REG_PAGE_1_FCCTR31                                 TX_PAGE_1, 0xA3

#define MSK_PAGE_1_FCCTR31_HFC_CONF31                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA4 HSIC Flow Control CTR32 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCCTR32                                 TX_PAGE_1, 0xA4

#define MSK_PAGE_1_FCCTR32_HFC_CONF32                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA5 HSIC Flow Control CTR33 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCCTR33                                 TX_PAGE_1, 0xA5

#define MSK_PAGE_1_FCCTR33_HFC_CONF33                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA6 HSIC Flow Control CTR34 Register                   (Default: 0x01)    */
#define REG_PAGE_1_FCCTR34                                 TX_PAGE_1, 0xA6

#define MSK_PAGE_1_FCCTR34_HFC_CONF34                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA7 HSIC Flow Control CTR35 Register                   (Default: 0x40)    */
#define REG_PAGE_1_FCCTR35                                 TX_PAGE_1, 0xA7

#define MSK_PAGE_1_FCCTR35_HFC_CONF35                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA8 HSIC Flow Control CTR36 Register                   (Default: 0x11)    */
#define REG_PAGE_1_FCCTR36                                 TX_PAGE_1, 0xA8

#define MSK_PAGE_1_FCCTR36_HFC_CONF36                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA9 HSIC Flow Control CTR37 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCCTR37                                 TX_PAGE_1, 0xA9

#define MSK_PAGE_1_FCCTR37_HFC_CONF37                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xAA HSIC Flow Control CTR38 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCCTR38                                 TX_PAGE_1, 0xAA

#define MSK_PAGE_1_FCCTR38_HFC_CONF38                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xAB HSIC Flow Control CTR39 Register                   (Default: 0x01)    */
#define REG_PAGE_1_FCCTR39                                 TX_PAGE_1, 0xAB

#define MSK_PAGE_1_FCCTR39_HFC_CONF39                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xAC HSIC Flow Control CTR40 Register                   (Default: 0xB0)    */
#define REG_PAGE_1_FCCTR40                                 TX_PAGE_1, 0xAC

#define MSK_PAGE_1_FCCTR40_HFC_CONF40                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xAD HSIC Flow Control CTR41 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCCTR41                                 TX_PAGE_1, 0xAD

#define MSK_PAGE_1_FCCTR41_HFC_CONF41                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xAE HSIC Flow Control CTR42 Register                   (Default: 0x83)    */
#define REG_PAGE_1_FCCTR42                                 TX_PAGE_1, 0xAE

#define MSK_PAGE_1_FCCTR42_HFC_CONF42                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xAF HSIC Flow Control CTR43 Register                   (Default: 0x04)    */
#define REG_PAGE_1_FCCTR43                                 TX_PAGE_1, 0xAF

#define MSK_PAGE_1_FCCTR43_HFC_CONF43                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB0 HSIC Flow Control CTR44 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCCTR44                                 TX_PAGE_1, 0xB0

#define MSK_PAGE_1_FCCTR44_HFC_CONF44                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB1 HSIC Flow Control CTR45 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCCTR45                                 TX_PAGE_1, 0xB1

#define MSK_PAGE_1_FCCTR45_HFC_CONF45                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB2 HSIC Flow Control CTR46 Register                   (Default: 0x50)    */
#define REG_PAGE_1_FCCTR46                                 TX_PAGE_1, 0xB2

#define MSK_PAGE_1_FCCTR46_HFC_CONF46                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB3 HSIC Flow Control CTR47 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCCTR47                                 TX_PAGE_1, 0xB3

#define MSK_PAGE_1_FCCTR47_HFC_CONF47                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB4 HSIC Flow Control CTR48 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCCTR48                                 TX_PAGE_1, 0xB4

#define MSK_PAGE_1_FCCTR48_HFC_CONF48                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB5 HSIC Flow Control CTR49 Register                   (Default: 0x01)    */
#define REG_PAGE_1_FCCTR49                                 TX_PAGE_1, 0xB5

#define MSK_PAGE_1_FCCTR49_HFC_CONF49                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB6 HSIC Flow Control CTR50 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCCTR50                                 TX_PAGE_1, 0xB6

#define MSK_PAGE_1_FCCTR50_HFC_CONF50                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB7 HSIC Flow Control CTR51 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCCTR51                                 TX_PAGE_1, 0xB7

#define MSK_PAGE_1_FCCTR51_HFC_CONF51                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB8 HSIC Flow Control CTR52 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCCTR52                                 TX_PAGE_1, 0xB8

#define MSK_PAGE_1_FCCTR52_HFC_CONF52                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB9 HSIC Flow Control CTR53 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCCTR53                                 TX_PAGE_1, 0xB9

#define MSK_PAGE_1_FCCTR53_HFC_CONF53                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBA HSIC Flow Control CTR54 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCCTR54                                 TX_PAGE_1, 0xBA

#define MSK_PAGE_1_FCCTR54_HFC_CONF54                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBB HSIC Flow Control CTR55 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCCTR55                                 TX_PAGE_1, 0xBB

#define MSK_PAGE_1_FCCTR55_HFC_CONF55                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBC HSIC Flow Control STAT0 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT0                                 TX_PAGE_1, 0xBC

#define MSK_PAGE_1_FCSTAT0_HFC_STAT0                       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBD HSIC Flow Control STAT1 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT1                                 TX_PAGE_1, 0xBD

#define MSK_PAGE_1_FCSTAT1_HFC_STAT1                       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBE HSIC Flow Control STAT2 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT2                                 TX_PAGE_1, 0xBE

#define MSK_PAGE_1_FCSTAT2_HFC_STAT2                       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBF HSIC Flow Control STAT3 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT3                                 TX_PAGE_1, 0xBF

#define MSK_PAGE_1_FCSTAT3_HFC_STAT3                       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC0 HSIC Flow Control STAT4 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT4                                 TX_PAGE_1, 0xC0

#define MSK_PAGE_1_FCSTAT4_HFC_STAT4                       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC1 HSIC Flow Control STAT5 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT5                                 TX_PAGE_1, 0xC1

#define MSK_PAGE_1_FCSTAT5_HFC_STAT5                       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC2 HSIC Flow Control STAT6 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT6                                 TX_PAGE_1, 0xC2

#define MSK_PAGE_1_FCSTAT6_HFC_STAT6                       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC3 HSIC Flow Control STAT7 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT7                                 TX_PAGE_1, 0xC3

#define MSK_PAGE_1_FCSTAT7_HFC_STAT7                       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC4 HSIC Flow Control STAT8 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT8                                 TX_PAGE_1, 0xC4

#define MSK_PAGE_1_FCSTAT8_HFC_STAT8                       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC5 HSIC Flow Control STAT9 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT9                                 TX_PAGE_1, 0xC5

#define MSK_PAGE_1_FCSTAT9_HFC_STAT9                       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC6 HSIC Flow Control STAT10 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT10                                TX_PAGE_1, 0xC6

#define MSK_PAGE_1_FCSTAT10_HFC_STAT10                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC7 HSIC Flow Control STAT11 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT11                                TX_PAGE_1, 0xC7

#define MSK_PAGE_1_FCSTAT11_HFC_STAT11                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC8 HSIC Flow Control STAT12 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT12                                TX_PAGE_1, 0xC8

#define MSK_PAGE_1_FCSTAT12_HFC_STAT12                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC9 HSIC Flow Control STAT13 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT13                                TX_PAGE_1, 0xC9

#define MSK_PAGE_1_FCSTAT13_HFC_STAT13                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCA HSIC Flow Control STAT14 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT14                                TX_PAGE_1, 0xCA

#define MSK_PAGE_1_FCSTAT14_HFC_STAT14                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCB HSIC Flow Control STAT15 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT15                                TX_PAGE_1, 0xCB

#define MSK_PAGE_1_FCSTAT15_HFC_STAT15                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCC HSIC Flow Control STAT16 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT16                                TX_PAGE_1, 0xCC

#define MSK_PAGE_1_FCSTAT16_HFC_STAT16                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCD HSIC Flow Control STAT17 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT17                                TX_PAGE_1, 0xCD

#define MSK_PAGE_1_FCSTAT17_HFC_STAT17                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCE HSIC Flow Control STAT18 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT18                                TX_PAGE_1, 0xCE

#define MSK_PAGE_1_FCSTAT18_HFC_STAT18                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCF HSIC Flow Control STAT19 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT19                                TX_PAGE_1, 0xCF

#define MSK_PAGE_1_FCSTAT19_HFC_STAT19                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD0 HSIC Flow Control STAT20 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT20                                TX_PAGE_1, 0xD0

#define MSK_PAGE_1_FCSTAT20_HFC_STAT20                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD1 HSIC Flow Control STAT21 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT21                                TX_PAGE_1, 0xD1

#define MSK_PAGE_1_FCSTAT21_HFC_STAT21                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD2 HSIC Flow Control STAT22 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT22                                TX_PAGE_1, 0xD2

#define MSK_PAGE_1_FCSTAT22_HFC_STAT22                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD3 HSIC Flow Control STAT23 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT23                                TX_PAGE_1, 0xD3

#define MSK_PAGE_1_FCSTAT23_HFC_STAT23                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD4 HSIC Flow Control STAT24 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT24                                TX_PAGE_1, 0xD4

#define MSK_PAGE_1_FCSTAT24_HFC_STAT24                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD5 HSIC Flow Control STAT25 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT25                                TX_PAGE_1, 0xD5

#define MSK_PAGE_1_FCSTAT25_HFC_STAT25                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD6 HSIC Flow Control STAT26 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT26                                TX_PAGE_1, 0xD6

#define MSK_PAGE_1_FCSTAT26_HFC_STAT26                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD7 HSIC Flow Control STAT27 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT27                                TX_PAGE_1, 0xD7

#define MSK_PAGE_1_FCSTAT27_HFC_STAT27                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD8 HSIC Flow Control STAT28 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT28                                TX_PAGE_1, 0xD8

#define MSK_PAGE_1_FCSTAT28_HFC_STAT28                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD9 HSIC Flow Control STAT29 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT29                                TX_PAGE_1, 0xD9

#define MSK_PAGE_1_FCSTAT29_HFC_STAT29                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xDA HSIC Flow Control STAT30 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT30                                TX_PAGE_1, 0xDA

#define MSK_PAGE_1_FCSTAT30_HFC_STAT30                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xDB HSIC Flow Control STAT31 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT31                                TX_PAGE_1, 0xDB

#define MSK_PAGE_1_FCSTAT31_HFC_STAT31                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xDC HSIC Flow Control STAT32 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT32                                TX_PAGE_1, 0xDC

#define MSK_PAGE_1_FCSTAT32_HFC_STAT32                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xDD HSIC Flow Control STAT33 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT33                                TX_PAGE_1, 0xDD

#define MSK_PAGE_1_FCSTAT33_HFC_STAT33                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xDE HSIC Flow Control STAT34 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT34                                TX_PAGE_1, 0xDE

#define MSK_PAGE_1_FCSTAT34_HFC_STAT34                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xDF HSIC Flow Control STAT35 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT35                                TX_PAGE_1, 0xDF

#define MSK_PAGE_1_FCSTAT35_HFC_STAT35                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE0 HSIC Flow Control STAT36 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT36                                TX_PAGE_1, 0xE0

#define MSK_PAGE_1_FCSTAT36_HFC_STAT36                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE1 HSIC Flow Control STAT37 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT37                                TX_PAGE_1, 0xE1

#define MSK_PAGE_1_FCSTAT37_HFC_STAT37                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE2 HSIC Flow Control STAT38 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT38                                TX_PAGE_1, 0xE2

#define MSK_PAGE_1_FCSTAT38_HFC_STAT38                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE3 HSIC Flow Control STAT39 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT39                                TX_PAGE_1, 0xE3

#define MSK_PAGE_1_FCSTAT39_HFC_STAT39                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE4 HSIC Flow Control STAT40 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT40                                TX_PAGE_1, 0xE4

#define MSK_PAGE_1_FCSTAT40_HFC_STAT40                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE5 HSIC Flow Control STAT41 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT41                                TX_PAGE_1, 0xE5

#define MSK_PAGE_1_FCSTAT41_HFC_STAT41                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE6 HSIC Flow Control STAT42 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT42                                TX_PAGE_1, 0xE6

#define MSK_PAGE_1_FCSTAT42_HFC_STAT42                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE7 HSIC Flow Control STAT43 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT43                                TX_PAGE_1, 0xE7

#define MSK_PAGE_1_FCSTAT43_HFC_STAT43                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE8 HSIC Flow Control STAT44 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT44                                TX_PAGE_1, 0xE8

#define MSK_PAGE_1_FCSTAT44_HFC_STAT44                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE9 HSIC Flow Control STAT45 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT45                                TX_PAGE_1, 0xE9

#define MSK_PAGE_1_FCSTAT45_HFC_STAT45                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xEA HSIC Flow Control STAT46 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT46                                TX_PAGE_1, 0xEA

#define MSK_PAGE_1_FCSTAT46_HFC_STAT46                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xEB HSIC Flow Control STAT47 Register                  (Default: 0x00)    */
#define REG_PAGE_1_FCSTAT47                                TX_PAGE_1, 0xEB

#define MSK_PAGE_1_FCSTAT47_HFC_STAT47                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xEC HSIC Flow Control INTR0 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCINTR0                                 TX_PAGE_1, 0xEC

#define BIT_PAGE_1_FCINTR0_HFC0_INTR7                      (0x01 << 7)
#define BIT_PAGE_1_FCINTR0_HFC0_INTR6                      (0x01 << 6)
#define BIT_PAGE_1_FCINTR0_HFC0_INTR5                      (0x01 << 5)
#define BIT_PAGE_1_FCINTR0_HFC0_INTR4                      (0x01 << 4)
#define BIT_PAGE_1_FCINTR0_HFC0_INTR3                      (0x01 << 3)
#define BIT_PAGE_1_FCINTR0_HFC0_INTR2                      (0x01 << 2)
#define BIT_PAGE_1_FCINTR0_HFC0_INTR1                      (0x01 << 1)
#define BIT_PAGE_1_FCINTR0_HFC0_INTR0                      (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xED HSIC Flow Control INTR1 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCINTR1                                 TX_PAGE_1, 0xED

#define BIT_PAGE_1_FCINTR1_HFC1_INTR7                      (0x01 << 7)
#define BIT_PAGE_1_FCINTR1_HFC1_INTR6                      (0x01 << 6)
#define BIT_PAGE_1_FCINTR1_HFC1_INTR5                      (0x01 << 5)
#define BIT_PAGE_1_FCINTR1_HFC1_INTR4                      (0x01 << 4)
#define BIT_PAGE_1_FCINTR1_HFC1_INTR3                      (0x01 << 3)
#define BIT_PAGE_1_FCINTR1_HFC1_INTR2                      (0x01 << 2)
#define BIT_PAGE_1_FCINTR1_HFC1_INTR1                      (0x01 << 1)
#define BIT_PAGE_1_FCINTR1_HFC1_INTR0                      (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xEE HSIC Flow Control INTR2 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCINTR2                                 TX_PAGE_1, 0xEE

#define BIT_PAGE_1_FCINTR2_HFC2_INTR7                      (0x01 << 7)
#define BIT_PAGE_1_FCINTR2_HFC2_INTR6                      (0x01 << 6)
#define BIT_PAGE_1_FCINTR2_HFC2_INTR5                      (0x01 << 5)
#define BIT_PAGE_1_FCINTR2_HFC2_INTR4                      (0x01 << 4)
#define BIT_PAGE_1_FCINTR2_HFC2_INTR3                      (0x01 << 3)
#define BIT_PAGE_1_FCINTR2_HFC2_INTR2                      (0x01 << 2)
#define BIT_PAGE_1_FCINTR2_HFC2_INTR1                      (0x01 << 1)
#define BIT_PAGE_1_FCINTR2_HFC2_INTR0                      (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xEF HSIC Flow Control INTR3 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCINTR3                                 TX_PAGE_1, 0xEF

#define BIT_PAGE_1_FCINTR3_HFC3_INTR7                      (0x01 << 7)
#define BIT_PAGE_1_FCINTR3_HFC3_INTR6                      (0x01 << 6)
#define BIT_PAGE_1_FCINTR3_HFC3_INTR5                      (0x01 << 5)
#define BIT_PAGE_1_FCINTR3_HFC3_INTR4                      (0x01 << 4)
#define BIT_PAGE_1_FCINTR3_HFC3_INTR3                      (0x01 << 3)
#define BIT_PAGE_1_FCINTR3_HFC3_INTR2                      (0x01 << 2)
#define BIT_PAGE_1_FCINTR3_HFC3_INTR1                      (0x01 << 1)
#define BIT_PAGE_1_FCINTR3_HFC3_INTR0                      (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF0 HSIC Flow Control INTR4 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCINTR4                                 TX_PAGE_1, 0xF0

#define BIT_PAGE_1_FCINTR4_HFC4_INTR7                      (0x01 << 7)
#define BIT_PAGE_1_FCINTR4_HFC4_INTR6                      (0x01 << 6)
#define BIT_PAGE_1_FCINTR4_HFC4_INTR5                      (0x01 << 5)
#define BIT_PAGE_1_FCINTR4_HFC4_INTR4                      (0x01 << 4)
#define BIT_PAGE_1_FCINTR4_HFC4_INTR3                      (0x01 << 3)
#define BIT_PAGE_1_FCINTR4_HFC4_INTR2                      (0x01 << 2)
#define BIT_PAGE_1_FCINTR4_HFC4_INTR1                      (0x01 << 1)
#define BIT_PAGE_1_FCINTR4_HFC4_INTR0                      (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF1 HSIC Flow Control INTR5 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCINTR5                                 TX_PAGE_1, 0xF1

#define BIT_PAGE_1_FCINTR5_HFC5_INTR7                      (0x01 << 7)
#define BIT_PAGE_1_FCINTR5_HFC5_INTR6                      (0x01 << 6)
#define BIT_PAGE_1_FCINTR5_HFC5_INTR5                      (0x01 << 5)
#define BIT_PAGE_1_FCINTR5_HFC5_INTR4                      (0x01 << 4)
#define BIT_PAGE_1_FCINTR5_HFC5_INTR3                      (0x01 << 3)
#define BIT_PAGE_1_FCINTR5_HFC5_INTR2                      (0x01 << 2)
#define BIT_PAGE_1_FCINTR5_HFC5_INTR1                      (0x01 << 1)
#define BIT_PAGE_1_FCINTR5_HFC5_INTR0                      (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF2 HSIC Flow Control INTR6 Register                   (Default: 0x20)    */
#define REG_PAGE_1_FCINTR6                                 TX_PAGE_1, 0xF2

#define BIT_PAGE_1_FCINTR6_HFC6_INTR7                      (0x01 << 7)
#define BIT_PAGE_1_FCINTR6_HFC6_INTR6                      (0x01 << 6)
#define BIT_PAGE_1_FCINTR6_HFC6_INTR5                      (0x01 << 5)
#define BIT_PAGE_1_FCINTR6_HFC6_INTR4                      (0x01 << 4)
#define BIT_PAGE_1_FCINTR6_HFC6_INTR3                      (0x01 << 3)
#define BIT_PAGE_1_FCINTR6_HFC6_INTR2                      (0x01 << 2)
#define BIT_PAGE_1_FCINTR6_HFC6_INTR1                      (0x01 << 1)
#define BIT_PAGE_1_FCINTR6_HFC6_INTR0                      (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF3 HSIC Flow Control INTR7 Register                   (Default: 0x00)    */
#define REG_PAGE_1_FCINTR7                                 TX_PAGE_1, 0xF3

#define BIT_PAGE_1_FCINTR7_HFC7_INTR7                      (0x01 << 7)
#define BIT_PAGE_1_FCINTR7_HFC7_INTR6                      (0x01 << 6)
#define BIT_PAGE_1_FCINTR7_HFC7_INTR5                      (0x01 << 5)
#define BIT_PAGE_1_FCINTR7_HFC7_INTR4                      (0x01 << 4)
#define BIT_PAGE_1_FCINTR7_HFC7_INTR3                      (0x01 << 3)
#define BIT_PAGE_1_FCINTR7_HFC7_INTR2                      (0x01 << 2)
#define BIT_PAGE_1_FCINTR7_HFC7_INTR1                      (0x01 << 1)
#define BIT_PAGE_1_FCINTR7_HFC7_INTR0                      (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF4 HSIC Flow Control INTRMASK0 Register               (Default: 0x00)    */
#define REG_PAGE_1_FCINTRMASK0                             TX_PAGE_1, 0xF4

#define BIT_PAGE_1_FCINTRMASK0_HFC0_INTRMASK7              (0x01 << 7)
#define BIT_PAGE_1_FCINTRMASK0_HFC0_INTRMASK6              (0x01 << 6)
#define BIT_PAGE_1_FCINTRMASK0_HFC0_INTRMASK5              (0x01 << 5)
#define BIT_PAGE_1_FCINTRMASK0_HFC0_INTRMASK4              (0x01 << 4)
#define BIT_PAGE_1_FCINTRMASK0_HFC0_INTRMASK3              (0x01 << 3)
#define BIT_PAGE_1_FCINTRMASK0_HFC0_INTRMASK2              (0x01 << 2)
#define BIT_PAGE_1_FCINTRMASK0_HFC0_INTRMASK1              (0x01 << 1)
#define BIT_PAGE_1_FCINTRMASK0_HFC0_INTRMASK0              (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF5 HSIC Flow Control INTRMASK1 Register               (Default: 0x00)    */
#define REG_PAGE_1_FCINTRMASK1                             TX_PAGE_1, 0xF5

#define BIT_PAGE_1_FCINTRMASK1_HFC1_INTRMASK7              (0x01 << 7)
#define BIT_PAGE_1_FCINTRMASK1_HFC1_INTRMASK6              (0x01 << 6)
#define BIT_PAGE_1_FCINTRMASK1_HFC1_INTRMASK5              (0x01 << 5)
#define BIT_PAGE_1_FCINTRMASK1_HFC1_INTRMASK4              (0x01 << 4)
#define BIT_PAGE_1_FCINTRMASK1_HFC1_INTRMASK3              (0x01 << 3)
#define BIT_PAGE_1_FCINTRMASK1_HFC1_INTRMASK2              (0x01 << 2)
#define BIT_PAGE_1_FCINTRMASK1_HFC1_INTRMASK1              (0x01 << 1)
#define BIT_PAGE_1_FCINTRMASK1_HFC1_INTRMASK0              (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF6 HSIC Flow Control INTRMASK2 Register               (Default: 0x00)    */
#define REG_PAGE_1_FCINTRMASK2                             TX_PAGE_1, 0xF6

#define BIT_PAGE_1_FCINTRMASK2_HFC2_INTRMASK7              (0x01 << 7)
#define BIT_PAGE_1_FCINTRMASK2_HFC2_INTRMASK6              (0x01 << 6)
#define BIT_PAGE_1_FCINTRMASK2_HFC2_INTRMASK5              (0x01 << 5)
#define BIT_PAGE_1_FCINTRMASK2_HFC2_INTRMASK4              (0x01 << 4)
#define BIT_PAGE_1_FCINTRMASK2_HFC2_INTRMASK3              (0x01 << 3)
#define BIT_PAGE_1_FCINTRMASK2_HFC2_INTRMASK2              (0x01 << 2)
#define BIT_PAGE_1_FCINTRMASK2_HFC2_INTRMASK1              (0x01 << 1)
#define BIT_PAGE_1_FCINTRMASK2_HFC2_INTRMASK0              (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF7 HSIC Flow Control INTRMASK3 Register               (Default: 0x00)    */
#define REG_PAGE_1_FCINTRMASK3                             TX_PAGE_1, 0xF7

#define BIT_PAGE_1_FCINTRMASK3_HFC3_INTRMASK7              (0x01 << 7)
#define BIT_PAGE_1_FCINTRMASK3_HFC3_INTRMASK6              (0x01 << 6)
#define BIT_PAGE_1_FCINTRMASK3_HFC3_INTRMASK5              (0x01 << 5)
#define BIT_PAGE_1_FCINTRMASK3_HFC3_INTRMASK4              (0x01 << 4)
#define BIT_PAGE_1_FCINTRMASK3_HFC3_INTRMASK3              (0x01 << 3)
#define BIT_PAGE_1_FCINTRMASK3_HFC3_INTRMASK2              (0x01 << 2)
#define BIT_PAGE_1_FCINTRMASK3_HFC3_INTRMASK1              (0x01 << 1)
#define BIT_PAGE_1_FCINTRMASK3_HFC3_INTRMASK0              (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF8 HSIC Flow Control INTRMASK4 Register               (Default: 0x00)    */
#define REG_PAGE_1_FCINTRMASK4                             TX_PAGE_1, 0xF8

#define BIT_PAGE_1_FCINTRMASK4_HFC4_INTRMASK7              (0x01 << 7)
#define BIT_PAGE_1_FCINTRMASK4_HFC4_INTRMASK6              (0x01 << 6)
#define BIT_PAGE_1_FCINTRMASK4_HFC4_INTRMASK5              (0x01 << 5)
#define BIT_PAGE_1_FCINTRMASK4_HFC4_INTRMASK4              (0x01 << 4)
#define BIT_PAGE_1_FCINTRMASK4_HFC4_INTRMASK3              (0x01 << 3)
#define BIT_PAGE_1_FCINTRMASK4_HFC4_INTRMASK2              (0x01 << 2)
#define BIT_PAGE_1_FCINTRMASK4_HFC4_INTRMASK1              (0x01 << 1)
#define BIT_PAGE_1_FCINTRMASK4_HFC4_INTRMASK0              (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF9 HSIC Flow Control INTRMASK5 Register               (Default: 0x00)    */
#define REG_PAGE_1_FCINTRMASK5                             TX_PAGE_1, 0xF9

#define BIT_PAGE_1_FCINTRMASK5_HFC5_INTRMASK7              (0x01 << 7)
#define BIT_PAGE_1_FCINTRMASK5_HFC5_INTRMASK6              (0x01 << 6)
#define BIT_PAGE_1_FCINTRMASK5_HFC5_INTRMASK5              (0x01 << 5)
#define BIT_PAGE_1_FCINTRMASK5_HFC5_INTRMASK4              (0x01 << 4)
#define BIT_PAGE_1_FCINTRMASK5_HFC5_INTRMASK3              (0x01 << 3)
#define BIT_PAGE_1_FCINTRMASK5_HFC5_INTRMASK2              (0x01 << 2)
#define BIT_PAGE_1_FCINTRMASK5_HFC5_INTRMASK1              (0x01 << 1)
#define BIT_PAGE_1_FCINTRMASK5_HFC5_INTRMASK0              (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xFA HSIC Flow Control INTRMASK6 Register               (Default: 0x00)    */
#define REG_PAGE_1_FCINTRMASK6                             TX_PAGE_1, 0xFA

#define BIT_PAGE_1_FCINTRMASK6_HFC6_INTRMASK7              (0x01 << 7)
#define BIT_PAGE_1_FCINTRMASK6_HFC6_INTRMASK6              (0x01 << 6)
#define BIT_PAGE_1_FCINTRMASK6_HFC6_INTRMASK5              (0x01 << 5)
#define BIT_PAGE_1_FCINTRMASK6_HFC6_INTRMASK4              (0x01 << 4)
#define BIT_PAGE_1_FCINTRMASK6_HFC6_INTRMASK3              (0x01 << 3)
#define BIT_PAGE_1_FCINTRMASK6_HFC6_INTRMASK2              (0x01 << 2)
#define BIT_PAGE_1_FCINTRMASK6_HFC6_INTRMASK1              (0x01 << 1)
#define BIT_PAGE_1_FCINTRMASK6_HFC6_INTRMASK0              (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xFB HSIC Flow Control INTRMASK7 Register               (Default: 0x00)    */
#define REG_PAGE_1_FCINTRMASK7                             TX_PAGE_1, 0xFB

#define BIT_PAGE_1_FCINTRMASK7_HFC7_INTRMASK7              (0x01 << 7)
#define BIT_PAGE_1_FCINTRMASK7_HFC7_INTRMASK6              (0x01 << 6)
#define BIT_PAGE_1_FCINTRMASK7_HFC7_INTRMASK5              (0x01 << 5)
#define BIT_PAGE_1_FCINTRMASK7_HFC7_INTRMASK4              (0x01 << 4)
#define BIT_PAGE_1_FCINTRMASK7_HFC7_INTRMASK3              (0x01 << 3)
#define BIT_PAGE_1_FCINTRMASK7_HFC7_INTRMASK2              (0x01 << 2)
#define BIT_PAGE_1_FCINTRMASK7_HFC7_INTRMASK1              (0x01 << 1)
#define BIT_PAGE_1_FCINTRMASK7_HFC7_INTRMASK0              (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xFC TDM Low Latency Register                           (Default: 0x20)    */
#define REG_PAGE_1_TDMLLCTL                                TX_PAGE_1, 0xFC

#define MSK_PAGE_1_TDMLLCTL_TRX_LL_SEL_MANUAL              (0x03 << 6)
#define MSK_PAGE_1_TDMLLCTL_TRX_LL_SEL_MODE                (0x03 << 4)
#define MSK_PAGE_1_TDMLLCTL_TTX_LL_SEL_MANUAL              (0x03 << 2)
#define BIT_PAGE_1_TDMLLCTL_TTX_LL_TIE_LOW                 (0x01 << 1)
#define BIT_PAGE_1_TDMLLCTL_TTX_LL_SEL_MODE                (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xFD TDM Byte Control Register                          (Default: 0x03)    */
#define REG_PAGE_1_TDMBYTE                                 TX_PAGE_1, 0xFD

#define BIT_PAGE_1_TDMBYTE_TTX_SLOT0_INTERLEAVING          (0x01 << 2)
#define BIT_PAGE_1_TDMBYTE_TRX_8BIT_CBUS_IF                (0x01 << 1)
#define BIT_PAGE_1_TDMBYTE_TTX_8BIT_CBUS_IF                (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xFE TDM Interleaving Target Register                   (Default: 0x0C)    */
#define REG_PAGE_1_S0TGT                                   TX_PAGE_1, 0xFE

#define MSK_PAGE_1_S0TGT_TTX_S0_TARGET                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xFF TDM Interleaving Allowance Register                (Default: 0x02)    */
#define REG_PAGE_1_S0ALLOW                                 TX_PAGE_1, 0xFF

#define MSK_PAGE_1_S0ALLOW_TTX_S0_ALLOWANCE                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* Registers in TX_PAGE_2                                                     */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* 0x01 TMDS Termination Control Register                  (Default: 0x00)    */
#define REG_PAGE_2_TMDS_TERMCTRL0                          TX_PAGE_2, 0x01

#define MSK_PAGE_2_TMDS_TERMCTRL0_TERM_SEL                 (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x02 DVI ctrl #0 Register                               (Default: 0x00)    */
#define REG_PAGE_2_DVI_CTRL0                               TX_PAGE_2, 0x02

#define MSK_PAGE_2_DVI_CTRL0_ZCTL_RST_DLY__7_0             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x03 DVI ctrl #1 Register                               (Default: 0x20)    */
#define REG_PAGE_2_DVI_CTRL1                               TX_PAGE_2, 0x03

#define BIT_PAGE_2_DVI_CTRL1_NO_ZONE2X                     (0x01 << 6)
#define BIT_PAGE_2_DVI_CTRL1_PSEL                          (0x01 << 5)
#define BIT_PAGE_2_DVI_CTRL1_USE_SCDT                      (0x01 << 4)
#define MSK_PAGE_2_DVI_CTRL1_ZCTL_RST_DLY__11_8            (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x04 Rx MISC Register                                   (Default: 0x30)    */
#define REG_PAGE_2_TMDS_MISC                               TX_PAGE_2, 0x04

#define BIT_PAGE_2_TMDS_MISC_MHL_TEST                      (0x01 << 7)
#define BIT_PAGE_2_TMDS_MISC_MHL_PP_EN                     (0x01 << 6)
#define BIT_PAGE_2_TMDS_MISC_EQ_BIAS_EN                    (0x01 << 5)
#define BIT_PAGE_2_TMDS_MISC_EN_GATE_CLK                   (0x01 << 4)
#define MSK_PAGE_2_TMDS_MISC_DPCOLOR_CTL                   (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0x10 TMDS 0 Clock Control Register 1                    (Default: 0x10)    */
#define REG_PAGE_2_TMDS0_CCTRL1                            TX_PAGE_2, 0x10

#define MSK_PAGE_2_TMDS0_CCTRL1_TEST_SEL                   (0x03 << 6)
#define MSK_PAGE_2_TMDS0_CCTRL1_CLK1X_CTL                  (0x03 << 4)

/*----------------------------------------------------------------------------*/
/* 0x11 TMDS Clock Enable Register                         (Default: 0x00)    */
#define REG_PAGE_2_TMDS_CLK_EN                             TX_PAGE_2, 0x11

#define BIT_PAGE_2_TMDS_CLK_EN_CLK_EN                      (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x12 TMDS Channel Enable Register                       (Default: 0x00)    */
#define REG_PAGE_2_TMDS_CH_EN                              TX_PAGE_2, 0x12

#define BIT_PAGE_2_TMDS_CH_EN_CH0_EN                       (0x01 << 4)
#define BIT_PAGE_2_TMDS_CH_EN_CH12_EN                      (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x13 TMDS Termination Control Register 1                (Default: 0x80)    */
#define REG_PAGE_2_TMDS_TERMCTRL1                          TX_PAGE_2, 0x13

#define MSK_PAGE_2_TMDS_TERMCTRL1_TERM_CTL                 (0x07 << 5)

/*----------------------------------------------------------------------------*/
/* 0x14 Clock Detect Control Register                      (Default: 0xA0)    */
#define REG_PAGE_2_CLKDETECT_CTL                           TX_PAGE_2, 0x14

#define MSK_PAGE_2_CLKDETECT_CTL_CLKDETECT_CTL             (0x0F << 4)
#define BIT_PAGE_2_CLKDETECT_CTL_LOBW                      (0x01 << 2)

/*----------------------------------------------------------------------------*/
/* 0x15 BGR_BIAS Register                                  (Default: 0x07)    */
#define REG_PAGE_2_BGR_BIAS                                TX_PAGE_2, 0x15

#define BIT_PAGE_2_BGR_BIAS_BGR_EN                         (0x01 << 7)
#define MSK_PAGE_2_BGR_BIAS_BIAS_BGR_D                     (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x16 PLL_CONFIG0 Register                               (Default: 0x4B)    */
#define REG_PAGE_2_PLL_CONFIG0                             TX_PAGE_2, 0x16

#define MSK_PAGE_2_PLL_CONFIG0_PLL_CONFIG                  (0x1F << 3)
#define MSK_PAGE_2_PLL_CONFIG0_PLL_VCO_IBIAS               (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0x17 PLL_CALREFSEL Register                             (Default: 0x09)    */
#define REG_PAGE_2_PLL_CALREFSEL                           TX_PAGE_2, 0x17

#define MSK_PAGE_2_PLL_CALREFSEL_PLL_MODE                  (0x03 << 6)
#define MSK_PAGE_2_PLL_CALREFSEL_PLL_ICPCOMP               (0x03 << 4)
#define MSK_PAGE_2_PLL_CALREFSEL_PLL_CALREFSEL             (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x18 PLL_ICPCNT Register                                (Default: 0x8A)    */
#define REG_PAGE_2_PLL_ICPCNT                              TX_PAGE_2, 0x18

#define MSK_PAGE_2_PLL_ICPCNT_PLL_SZONE                    (0x03 << 6)
#define BIT_PAGE_2_PLL_ICPCNT_PLL_SCPCAL                   (0x01 << 5)
#define MSK_PAGE_2_PLL_ICPCNT_PLL_ICPCNT                   (0x1F << 0)

/*----------------------------------------------------------------------------*/
/* 0x19 PLL_SPLLBIAS Register                              (Default: 0xCC)    */
#define REG_PAGE_2_PLL_SPLLBIAS                            TX_PAGE_2, 0x19

#define BIT_PAGE_2_PLL_SPLLBIAS_PLL_EN                     (0x01 << 7)
#define BIT_PAGE_2_PLL_SPLLBIAS_PLL_BIAS_EN                (0x01 << 6)
#define MSK_PAGE_2_PLL_SPLLBIAS_PLL_SPLLBIAS               (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x1A PLL_VCOCAL Register                                (Default: 0xA9)    */
#define REG_PAGE_2_PLL_VCOCAL                              TX_PAGE_2, 0x1A

#define MSK_PAGE_2_PLL_VCOCAL_PLL_VCOCAL                   (0x0F << 4)
#define MSK_PAGE_2_PLL_VCOCAL_PLL_VCOCAL_DEF               (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x20 TMDS 0 Digital EQ Value Register                   (Default: 0x3A)    */
#define REG_PAGE_2_ALICE0_EQ_VAL                           TX_PAGE_2, 0x20

#define MSK_PAGE_2_ALICE0_EQ_VAL_TMDS0_EQ_VAL_7_0          (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x21 TMDS 0 Digital I2C EQ Register                     (Default: 0xE0)    */
#define REG_PAGE_2_ALICE0_EQ_I2C                           TX_PAGE_2, 0x21

#define MSK_PAGE_2_ALICE0_EQ_I2C_TMDS0_EQ_I2C_7_0          (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x22 TMDS 0 Digital EQ_DATA0 Register                   (Default: 0xE0)    */
#define REG_PAGE_2_ALICE0_EQ_DATA0                         TX_PAGE_2, 0x22

#define MSK_PAGE_2_ALICE0_EQ_DATA0_TMDS0_EQ_DATA0_7_0      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x23 TMDS 0 Digital EQ_DATA1 Register                   (Default: 0xC0)    */
#define REG_PAGE_2_ALICE0_EQ_DATA1                         TX_PAGE_2, 0x23

#define MSK_PAGE_2_ALICE0_EQ_DATA1_TMDS0_EQ_DATA1_7_0      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x24 TMDS 0 Digital EQ_DATA2 Register                   (Default: 0xA0)    */
#define REG_PAGE_2_ALICE0_EQ_DATA2                         TX_PAGE_2, 0x24

#define MSK_PAGE_2_ALICE0_EQ_DATA2_TMDS0_EQ_DATA2_7_0      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x25 TMDS 0 Digital EQ_DATA3 Register                   (Default: 0x80)    */
#define REG_PAGE_2_ALICE0_EQ_DATA3                         TX_PAGE_2, 0x25

#define MSK_PAGE_2_ALICE0_EQ_DATA3_TMDS0_EQ_DATA3_7_0      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x26 TMDS 0 Digital EQ_DATA4 Register                   (Default: 0x60)    */
#define REG_PAGE_2_ALICE0_EQ_DATA4                         TX_PAGE_2, 0x26

#define MSK_PAGE_2_ALICE0_EQ_DATA4_TMDS0_EQ_DATA4_7_0      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x27 TMDS 0 Digital EQ_DATA5 Register                   (Default: 0x40)    */
#define REG_PAGE_2_ALICE0_EQ_DATA5                         TX_PAGE_2, 0x27

#define MSK_PAGE_2_ALICE0_EQ_DATA5_TMDS0_EQ_DATA5_7_0      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x28 TMDS 0 Digital EQ_DATA6 Register                   (Default: 0x20)    */
#define REG_PAGE_2_ALICE0_EQ_DATA6                         TX_PAGE_2, 0x28

#define MSK_PAGE_2_ALICE0_EQ_DATA6_TMDS0_EQ_DATA6_7_0      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x29 TMDS 0 Digital EQ_DATA7 Register                   (Default: 0x00)    */
#define REG_PAGE_2_ALICE0_EQ_DATA7                         TX_PAGE_2, 0x29

#define MSK_PAGE_2_ALICE0_EQ_DATA7_TMDS0_EQ_DATA7_7_0      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x30 TMDS 0 Digital BW Value Register                   (Default: 0x00)    */
#define REG_PAGE_2_ALICE0_BW_VAL                           TX_PAGE_2, 0x30

#define MSK_PAGE_2_ALICE0_BW_VAL_TMDS0_BW_VAL_7_0          (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x31 TMDS 0 Digital I2C BW Register                     (Default: 0x0A)    */
#define REG_PAGE_2_ALICE0_BW_I2C                           TX_PAGE_2, 0x31

#define MSK_PAGE_2_ALICE0_BW_I2C_TMDS0_BW_I2C_7_0          (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x32 TMDS 0 Digital BW_DATA0 Register                   (Default: 0x07)    */
#define REG_PAGE_2_ALICE0_BW_DATA0                         TX_PAGE_2, 0x32

#define MSK_PAGE_2_ALICE0_BW_DATA0_TMDS0_BW_DATA0_7_0      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x33 TMDS 0 Digital BW_DATA1 Register                   (Default: 0x0E)    */
#define REG_PAGE_2_ALICE0_BW_DATA1                         TX_PAGE_2, 0x33

#define MSK_PAGE_2_ALICE0_BW_DATA1_TMDS0_BW_DATA1_7_0      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x34 TMDS 0 Digital BW_DATA2 Register                   (Default: 0x07)    */
#define REG_PAGE_2_ALICE0_BW_DATA2                         TX_PAGE_2, 0x34

#define MSK_PAGE_2_ALICE0_BW_DATA2_TMDS0_BW_DATA2_7_0      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x35 TMDS 0 Digital BW_DATA3 Register                   (Default: 0x07)    */
#define REG_PAGE_2_ALICE0_BW_DATA3                         TX_PAGE_2, 0x35

#define MSK_PAGE_2_ALICE0_BW_DATA3_TMDS0_BW_DATA3_7_0      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x38 TMDS 0 Digital VT Value Register                   (Default: 0x00)    */
#define REG_PAGE_2_ALICE0_VT_VAL                           TX_PAGE_2, 0x38

#define MSK_PAGE_2_ALICE0_VT_VAL_TMDS0_VT_VAL_7_0          (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x39 TMDS 0 Digital I2C VT Register                     (Default: 0x00)    */
#define REG_PAGE_2_ALICE0_VT_I2C                           TX_PAGE_2, 0x39

#define MSK_PAGE_2_ALICE0_VT_I2C_TMDS0_VT_I2C_7_0          (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x3A TMDS 0 Digital VT_DATA0 Register                   (Default: 0x00)    */
#define REG_PAGE_2_ALICE0_VT_DATA0                         TX_PAGE_2, 0x3A

#define MSK_PAGE_2_ALICE0_VT_DATA0_TMDS0_VT_DATA0_7_0      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x3B TMDS 0 Digital VT_DATA1 Register                   (Default: 0x00)    */
#define REG_PAGE_2_ALICE0_VT_DATA1                         TX_PAGE_2, 0x3B

#define MSK_PAGE_2_ALICE0_VT_DATA1_TMDS0_VT_DATA1_7_0      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x3C TMDS 0 Digital VT_DATA2 Register                   (Default: 0x00)    */
#define REG_PAGE_2_ALICE0_VT_DATA2                         TX_PAGE_2, 0x3C

#define MSK_PAGE_2_ALICE0_VT_DATA2_TMDS0_VT_DATA2_7_0      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x3D TMDS 0 Digital VT_DATA3 Register                   (Default: 0x00)    */
#define REG_PAGE_2_ALICE0_VT_DATA3                         TX_PAGE_2, 0x3D

#define MSK_PAGE_2_ALICE0_VT_DATA3_TMDS0_VT_DATA3_7_0      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x44 TMDS0 Digital Control 0 Register                   (Default: 0x00)    */
#define REG_PAGE_2_ALICE0_CNTL0                            TX_PAGE_2, 0x44

#define MSK_PAGE_2_ALICE0_CNTL0_TMDS0_CHAN_SEL_1_0         (0x03 << 6)
#define BIT_PAGE_2_ALICE0_CNTL0_DVI0_RX_DIG_BYPASS         (0x01 << 5)
#define BIT_PAGE_2_ALICE0_CNTL0_TMDS0_MODE_INV             (0x01 << 4)
#define MSK_PAGE_2_ALICE0_CNTL0_TMDS0_BYP_MODE_CNTL_1_0    (0x03 << 1)
#define BIT_PAGE_2_ALICE0_CNTL0_TMDS0_LANE_ALG_OFF         (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x45 TMDS0 Digital Control 1 Register                   (Default: 0x06)    */
#define REG_PAGE_2_ALICE0_CNTL1                            TX_PAGE_2, 0x45

#define BIT_PAGE_2_ALICE0_CNTL1_TMDS0_DPLL_BW_SCAN_ON      (0x01 << 6)
#define BIT_PAGE_2_ALICE0_CNTL1_TMDS0_W_SEL                (0x01 << 4)
#define BIT_PAGE_2_ALICE0_CNTL1_TMDS0_EXT_BW               (0x01 << 3)
#define BIT_PAGE_2_ALICE0_CNTL1_TMDS0_BV_SEL               (0x01 << 2)
#define BIT_PAGE_2_ALICE0_CNTL1_TMDS0_EXT_EQ               (0x01 << 1)
#define BIT_PAGE_2_ALICE0_CNTL1_TMDS0_EV_SEL               (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x46 TMDS0 Digital Control 2 Register                   (Default: 0x02)    */
#define REG_PAGE_2_ALICE0_CNTL2                            TX_PAGE_2, 0x46

#define MSK_PAGE_2_ALICE0_CNTL2_TMDS0_EQ_MODE              (0x03 << 6)
#define MSK_PAGE_2_ALICE0_CNTL2_TMDS0_PH_TAPS_1_0          (0x03 << 4)
#define BIT_PAGE_2_ALICE0_CNTL2_TMDS0_EXT_SYNC             (0x01 << 3)
#define MSK_PAGE_2_ALICE0_CNTL2_TMDS0_PH_BS_2_0            (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0x47 TMDS0 Digital Control 3 Register                   (Default: 0xDC)    */
#define REG_PAGE_2_ALICE0_CNTL3                            TX_PAGE_2, 0x47

#define MSK_PAGE_2_ALICE0_CNTL3_TMDS0_STB_RSC_1_0          (0x03 << 6)
#define MSK_PAGE_2_ALICE0_CNTL3_TMDS0_ALICE_EDGE_CNTL_5_0  (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0x48 TMDS0 Digital Control 4 Register                   (Default: 0x80)    */
#define REG_PAGE_2_ALICE0_CNTL4                            TX_PAGE_2, 0x48

#define MSK_PAGE_2_ALICE0_CNTL4_TMDS0_RST_CON_1_0          (0x03 << 6)
#define BIT_PAGE_2_ALICE0_CNTL4_TMDS0_RELX_EYE_ON          (0x01 << 4)
#define BIT_PAGE_2_ALICE0_CNTL4_TMDS0_BP_FIX               (0x01 << 3)
#define BIT_PAGE_2_ALICE0_CNTL4_TMDS0_DPLL_STRNGTH         (0x01 << 2)
#define BIT_PAGE_2_ALICE0_CNTL4_TMDS0_LOW_MON              (0x01 << 1)

/*----------------------------------------------------------------------------*/
/* 0x49 TMDS0 Digital Control 5 Register                   (Default: 0x02)    */
#define REG_PAGE_2_ALICE0_CNTL5                            TX_PAGE_2, 0x49

#define BIT_PAGE_2_ALICE0_CNTL5_TMDS0_GLT_ON               (0x01 << 6)
#define BIT_PAGE_2_ALICE0_CNTL5_TMDS0_VLDCHK               (0x01 << 2)
#define BIT_PAGE_2_ALICE0_CNTL5_TMDS0_EDON                 (0x01 << 1)
#define BIT_PAGE_2_ALICE0_CNTL5_TMDS0_LOWLMT               (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x4B TMDS0 Digital Control 6 Register                   (Default: 0x06)    */
#define REG_PAGE_2_ALICE0_CNTL7                            TX_PAGE_2, 0x4B

#define BIT_PAGE_2_ALICE0_CNTL7_TMDS0_DC_CP_EN             (0x01 << 7)
#define MSK_PAGE_2_ALICE0_CNTL7_TMDS0_DEC_CON_2_0          (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0x4C TMDS 0 Digital Zone Control Register               (Default: 0xE0)    */
#define REG_PAGE_2_ALICE0_ZONE_CTRL                        TX_PAGE_2, 0x4C

#define BIT_PAGE_2_ALICE0_ZONE_CTRL_ICRST_N                (0x01 << 7)
#define BIT_PAGE_2_ALICE0_ZONE_CTRL_USE_INT_DIV20          (0x01 << 6)
#define MSK_PAGE_2_ALICE0_ZONE_CTRL_SZONE_I2C              (0x03 << 4)
#define MSK_PAGE_2_ALICE0_ZONE_CTRL_ZONE_CTRL              (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x4D TMDS 0 Digital PLL Mode Control Register           (Default: 0x00)    */
#define REG_PAGE_2_ALICE0_MODE_CTRL                        TX_PAGE_2, 0x4D

#define MSK_PAGE_2_ALICE0_MODE_CTRL_PLL_MODE_I2C           (0x03 << 2)
#define MSK_PAGE_2_ALICE0_MODE_CTRL_DIV20_CTRL             (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x50 TMDS 0 Status 0 Register                           (Default: 0x02)    */
#define REG_PAGE_2_ALICE0_STAT0                            TX_PAGE_2, 0x50

#define MSK_PAGE_2_ALICE0_STAT0_TMDS0_B_BP_OUT_4_0         (0x1F << 1)
#define BIT_PAGE_2_ALICE0_STAT0_TMDS0_EDOUT_B              (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x51 TMDS 0 Status 1 Register                           (Default: 0x00)    */
#define REG_PAGE_2_ALICE0_STAT1                            TX_PAGE_2, 0x51

#define MSK_PAGE_2_ALICE0_STAT1_TMDS0_R_BP_OUT_4_0         (0x1F << 1)
#define BIT_PAGE_2_ALICE0_STAT1_TMDS0_EDOUT_R              (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x52 TMDS 0 Status 2 Register                           (Default: 0x00)    */
#define REG_PAGE_2_ALICE0_STAT2                            TX_PAGE_2, 0x52

#define MSK_PAGE_2_ALICE0_STAT2_TMDS0_R_BP_OUT_4_0         (0x1F << 1)
#define BIT_PAGE_2_ALICE0_STAT2_TMDS0_EDOUT_R              (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x53 TMDS 0 Status 3 Register                           (Default: 0x00)    */
#define REG_PAGE_2_ALICE0_STAT3                            TX_PAGE_2, 0x53

#define MSK_PAGE_2_ALICE0_STAT3_TMDS0_BIAS_B_3_0           (0x0F << 4)
#define MSK_PAGE_2_ALICE0_STAT3_TMDS0_EOPEN_B_3_0          (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x54 TMDS 0 Status 4 Register                           (Default: 0x00)    */
#define REG_PAGE_2_ALICE0_STAT4                            TX_PAGE_2, 0x54

#define MSK_PAGE_2_ALICE0_STAT4_TMDS0_BIAS_G_3_0           (0x0F << 4)
#define MSK_PAGE_2_ALICE0_STAT4_TMDS0_EOPEN_G_3_0          (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x55 TMDS 0 Status 5 Register                           (Default: 0x00)    */
#define REG_PAGE_2_ALICE0_STAT5                            TX_PAGE_2, 0x55

#define MSK_PAGE_2_ALICE0_STAT5_TMDS0_BIAS_R_3_0           (0x0F << 4)
#define MSK_PAGE_2_ALICE0_STAT5_TMDS0_EOPEN_R_3_0          (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x56 TMDS 0 Status 6 Register                           (Default: 0x00)    */
#define REG_PAGE_2_ALICE0_STAT6                            TX_PAGE_2, 0x56

#define MSK_PAGE_2_ALICE0_STAT6_TMDS0_SCAN_VAL_7_0         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x60 TMDS 0 BIST CNTL Register                          (Default: 0x00)    */
#define REG_PAGE_2_TMDS0_BIST_CNTL                         TX_PAGE_2, 0x60

#define MSK_PAGE_2_TMDS0_BIST_CNTL_TMDS0_BIST_DURATION_2_0 (0x07 << 5)
#define BIT_PAGE_2_TMDS0_BIST_CNTL_TMDS0_BIST_START        (0x01 << 4)
#define BIT_PAGE_2_TMDS0_BIST_CNTL_TMDS0_BIST_CONT_PROG_DURAT (0x01 << 3)
#define BIT_PAGE_2_TMDS0_BIST_CNTL_TMDS0_BIST_VGB_MASK     (0x01 << 2)
#define BIT_PAGE_2_TMDS0_BIST_CNTL_TMDS0_BIST_RESET        (0x01 << 1)
#define BIT_PAGE_2_TMDS0_BIST_CNTL_TMDS0_BIST_ENABLE       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x61 TMDS 0 BIST DURATION0 Register                     (Default: 0x00)    */
#define REG_PAGE_2_TMDS0_BIST_DURATION0                    TX_PAGE_2, 0x61

#define MSK_PAGE_2_TMDS0_BIST_DURATION0_TMDS0_BIST_DURATION_10_3 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x62 TMDS 0 BIST DURATION1 Register                     (Default: 0x00)    */
#define REG_PAGE_2_TMDS0_BIST_DURATION1                    TX_PAGE_2, 0x62

#define MSK_PAGE_2_TMDS0_BIST_DURATION1_TMDS0_BIST_DURATION_18_11 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x63 TMDS 0 BIST DURATION2 Register                     (Default: 0x00)    */
#define REG_PAGE_2_TMDS0_BIST_DURATION2                    TX_PAGE_2, 0x63

#define MSK_PAGE_2_TMDS0_BIST_DURATION2_TMDS0_BIST_DURATION_22_19 (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x64 TMDS 0 BIST TEST SEL Register                      (Default: 0x02)    */
#define REG_PAGE_2_TMDS0_BIST_TEST_SEL                     TX_PAGE_2, 0x64

#define MSK_PAGE_2_TMDS0_BIST_TEST_SEL_TMDS0_BIST_TEST_SELECT (0x03 << 4)
#define MSK_PAGE_2_TMDS0_BIST_TEST_SEL_TMDS0_BIST_PATTERN_SELECT (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x65 TMDS 0 BIST VIDEO_MODE Register                    (Default: 0x02)    */
#define REG_PAGE_2_TMDS0_BIST_VIDEO_MODE                   TX_PAGE_2, 0x65

#define BIT_PAGE_2_TMDS0_BIST_VIDEO_MODE_TMDS0_BIST_FORCE_DE (0x01 << 3)
#define MSK_PAGE_2_TMDS0_BIST_VIDEO_MODE_TMDS0_BIST_VIDEO_MODE (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0x66 TMDS 0 BIST 8BIT_PATTERN Register                  (Default: 0x55)    */
#define REG_PAGE_2_TMDS0_BIST_8BIT_PATTERN                 TX_PAGE_2, 0x66

#define MSK_PAGE_2_TMDS0_BIST_8BIT_PATTERN_TMDS0_BIST_8BIT_PATTERN (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x67 TMDS 0 BIST 10BIT_PATTERN_L Register               (Default: 0x00)    */
#define REG_PAGE_2_TMDS0_BIST_10BIT_PATTERN_L              TX_PAGE_2, 0x67

#define MSK_PAGE_2_TMDS0_BIST_10BIT_PATTERN_L_TMDS0_BIST_10BIT_PATTERN_L (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x68 TMDS 0 BIST 10BIT_PATTERN_U Register               (Default: 0x00)    */
#define REG_PAGE_2_TMDS0_BIST_10BIT_PATTERN_U              TX_PAGE_2, 0x68

#define MSK_PAGE_2_TMDS0_BIST_10BIT_PATTERN_U_TMDS0_BIST_10BIT_PATTERN_U (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x69 TMDS 0 BIST STATUS Register                        (Default: 0x00)    */
#define REG_PAGE_2_TMDS0_BIST_STATUS                       TX_PAGE_2, 0x69

#define MSK_PAGE_2_TMDS0_BIST_STATUS_TMDS0_BIST_CONFIG_STATUS (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x6A TMDS 0 BIST RESULT Register                        (Default: 0x00)    */
#define REG_PAGE_2_TMDS0_BIST_RESULT                       TX_PAGE_2, 0x6A

#define BIT_PAGE_2_TMDS0_BIST_RESULT_TMDS0_BIST_CONFIG_CNTL_ERROR (0x01 << 5)
#define BIT_PAGE_2_TMDS0_BIST_RESULT_TMDS0_BIST_CONFIG_OVERFLOW (0x01 << 4)
#define BIT_PAGE_2_TMDS0_BIST_RESULT_TMDS0_BIST_CONFIG_DE_ERROR (0x01 << 3)
#define BIT_PAGE_2_TMDS0_BIST_RESULT_TMDS0_BIST_CONFIG_VSYNC_ERROR (0x01 << 2)
#define BIT_PAGE_2_TMDS0_BIST_RESULT_TMDS0_BIST_CONFIG_HSYNC_ERROR (0x01 << 1)
#define BIT_PAGE_2_TMDS0_BIST_RESULT_TMDS0_BIST_CONFIG_FAIL (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x6B TMDS 0 BIST P_ERROR_COUNT_0 Register               (Default: 0x00)    */
#define REG_PAGE_2_TMDS0_BIST_P_ERROR_COUNT_0              TX_PAGE_2, 0x6B

#define MSK_PAGE_2_TMDS0_BIST_P_ERROR_COUNT_0_TMDS0_BIST_P_ERROR_COUNT_0 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x6C TMDS 0 BIST P_ERROR_COUNT_1 Register               (Default: 0x00)    */
#define REG_PAGE_2_TMDS0_BIST_P_ERROR_COUNT_1              TX_PAGE_2, 0x6C

#define MSK_PAGE_2_TMDS0_BIST_P_ERROR_COUNT_1_TMDS0_BIST_P_ERROR_COUNT_1 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x6D TMDS 0 BIST R_ERROR_COUNT_0 Register               (Default: 0x00)    */
#define REG_PAGE_2_TMDS0_BIST_R_ERROR_COUNT_0              TX_PAGE_2, 0x6D

#define MSK_PAGE_2_TMDS0_BIST_R_ERROR_COUNT_0_TMDS0_BIST_R_ERROR_COUNT_0 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x6E TMDS 0 BIST R_ERROR_COUNT_1 Register               (Default: 0x00)    */
#define REG_PAGE_2_TMDS0_BIST_R_ERROR_COUNT_1              TX_PAGE_2, 0x6E

#define MSK_PAGE_2_TMDS0_BIST_R_ERROR_COUNT_1_TMDS0_BIST_R_ERROR_COUNT_1 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x6F TMDS 0 BIST G_ERROR_COUNT_0 Register               (Default: 0x00)    */
#define REG_PAGE_2_TMDS0_BIST_G_ERROR_COUNT_0              TX_PAGE_2, 0x6F

#define MSK_PAGE_2_TMDS0_BIST_G_ERROR_COUNT_0_TMDS0_BIST_G_ERROR_COUNT_0 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x70 TMDS 0 BIST G_ERROR_COUNT_1 Register               (Default: 0x00)    */
#define REG_PAGE_2_TMDS0_BIST_G_ERROR_COUNT_1              TX_PAGE_2, 0x70

#define MSK_PAGE_2_TMDS0_BIST_G_ERROR_COUNT_1_TMDS0_BIST_G_ERROR_COUNT_1 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x71 TMDS 0 BIST B_ERROR_COUNT_0 Register               (Default: 0x00)    */
#define REG_PAGE_2_TMDS0_BIST_B_ERROR_COUNT_0              TX_PAGE_2, 0x71

#define MSK_PAGE_2_TMDS0_BIST_B_ERROR_COUNT_0_TMDS0_BIST_B_ERROR_COUNT_0 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x72 TMDS 0 BIST B_ERROR_COUNT_1 Register               (Default: 0x00)    */
#define REG_PAGE_2_TMDS0_BIST_B_ERROR_COUNT_1              TX_PAGE_2, 0x72

#define MSK_PAGE_2_TMDS0_BIST_B_ERROR_COUNT_1_TMDS0_BIST_B_ERROR_COUNT_1 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x73 TMDS 0 BIST CNTL_ERROR_COUNT Register              (Default: 0x00)    */
#define REG_PAGE_2_TMDS0_BIST_CNTL_ERROR_COUNT             TX_PAGE_2, 0x73

#define MSK_PAGE_2_TMDS0_BIST_CNTL_ERROR_COUNT_TMDS0_BIST_CNTL_ERROR_COUNT (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x75 MBIST_CTL1 Register                                (Default: 0x00)    */
#define REG_PAGE_2_MBIST_CTL1                              TX_PAGE_2, 0x75

#define BIT_PAGE_2_MBIST_CTL1_MBIST_ERRMAP_EDID            (0x01 << 6)
#define BIT_PAGE_2_MBIST_CTL1_MBIST_FAIL_EDID              (0x01 << 5)
#define BIT_PAGE_2_MBIST_CTL1_MBIST_FINISH_EDID            (0x01 << 4)
#define MSK_PAGE_2_MBIST_CTL1_MBIST_EMA_EDID               (0x07 << 1)
#define BIT_PAGE_2_MBIST_CTL1_MBIST_MODE_EDID              (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x76 MBIST_CTL2 Register                                (Default: 0x00)    */
#define REG_PAGE_2_MBIST_CTL2                              TX_PAGE_2, 0x76

#define BIT_PAGE_2_MBIST_CTL2_MBIST_ERRMAP_DPRAM512X34     (0x01 << 6)
#define BIT_PAGE_2_MBIST_CTL2_MBIST_FAIL_DPRAM512X34       (0x01 << 5)
#define BIT_PAGE_2_MBIST_CTL2_MBIST_FINISH_DPRAM512X34     (0x01 << 4)
#define MSK_PAGE_2_MBIST_CTL2_MBIST_EMAA_DPRAM512X34       (0x07 << 1)
#define BIT_PAGE_2_MBIST_CTL2_MBIST_MODE_DPRAM512X34       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x77 MBIST_CTLHSIC2LINK Register                        (Default: 0x00)    */
#define REG_PAGE_2_MBIST_CTLH2L                            TX_PAGE_2, 0x77

#define MSK_PAGE_2_MBIST_CTLH2L_MBIST_EMABH2L_DPRAM4096X8  (0x07 << 5)
#define MSK_PAGE_2_MBIST_CTLH2L_MBIST_EMAAH2L_DPRAM4096X8  (0x07 << 2)
#define BIT_PAGE_2_MBIST_CTLH2L_MBIST_RETENH2L_DPRAM4096X8 (0x01 << 1)
#define BIT_PAGE_2_MBIST_CTLH2L_MBIST_MODEH2L_DPRAM4096X8  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x78 MBIST_CTLLINK2HSIC Register                        (Default: 0x00)    */
#define REG_PAGE_2_MBIST_CTLL2H                            TX_PAGE_2, 0x78

#define MSK_PAGE_2_MBIST_CTLL2H_MBIST_EMABL2H_DPRAM4096X8  (0x07 << 5)
#define MSK_PAGE_2_MBIST_CTLL2H_MBIST_EMAAL2H_DPRAM4096X8  (0x07 << 2)
#define BIT_PAGE_2_MBIST_CTLL2H_MBIST_RETENL2H_DPRAM4096X8 (0x01 << 1)
#define BIT_PAGE_2_MBIST_CTLL2H_MBIST_MODEL2H_DPRAM4096X8  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x79 MBIST_ROM_CTL1 Register                            (Default: 0x00)    */
#define REG_PAGE_2_MBIST_ROM_CTL1                          TX_PAGE_2, 0x79

#define MSK_PAGE_2_MBIST_ROM_CTL1_MBIST_EMAB_DPRAM512X34   (0x07 << 5)
#define MSK_PAGE_2_MBIST_ROM_CTL1_MBIST_EMA_ROM32768X8     (0x07 << 1)
#define BIT_PAGE_2_MBIST_ROM_CTL1_MBIST_MODE_ROM32768X8    (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x7A MBIST_ROM_STATUS1 Register                         (Default: 0x00)    */
#define REG_PAGE_2_MBIST_ROM_STATUS1                       TX_PAGE_2, 0x7A

#define MSK_PAGE_2_MBIST_ROM_STATUS1_MBIST_SOUT_ROM32768X8_7_0 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x7B MBIST_ROM_STATUS2 Register                         (Default: 0x00)    */
#define REG_PAGE_2_MBIST_ROM_STATUS2                       TX_PAGE_2, 0x7B

#define MSK_PAGE_2_MBIST_ROM_STATUS2_MBIST_SOUT_ROM32768X8_15_8 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x7C MBIST_ROM_STATUS3 Register                         (Default: 0x00)    */
#define REG_PAGE_2_MBIST_ROM_STATUS3                       TX_PAGE_2, 0x7C

#define MSK_PAGE_2_MBIST_ROM_STATUS3_MBIST_SOUT_ROM32768X8_23_16 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x7D MBIST_ROM_STATUS4 Register                         (Default: 0x40)    */
#define REG_PAGE_2_MBIST_ROM_STATUS4                       TX_PAGE_2, 0x7D

#define BIT_PAGE_2_MBIST_ROM_STATUS4_MBIST_SIGOUT_ROM32768X8 (0x01 << 7)
#define BIT_PAGE_2_MBIST_ROM_STATUS4_MBIST_SHIFTEND_ROM32768X8 (0x01 << 6)
#define BIT_PAGE_2_MBIST_ROM_STATUS4_MBIST_FAIL_ROM32768X8 (0x01 << 5)
#define BIT_PAGE_2_MBIST_ROM_STATUS4_MBIST_FINISH_ROM32768X8 (0x01 << 4)
#define MSK_PAGE_2_MBIST_ROM_STATUS4_MBIST_SOUT_ROM32768X8_27_24 (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x7E MBIST_CTL3 Register                                (Default: 0x00)    */
#define REG_PAGE_2_MBIST_CTL3                              TX_PAGE_2, 0x7E

#define BIT_PAGE_2_MBIST_CTL3_MBIST_ERRMAP_SPRAM256X8      (0x01 << 6)
#define BIT_PAGE_2_MBIST_CTL3_MBIST_FAIL_SPRAM256X8        (0x01 << 5)
#define BIT_PAGE_2_MBIST_CTL3_MBIST_FINISH_SPRAM256X8      (0x01 << 4)
#define MSK_PAGE_2_MBIST_CTL3_MBIST_EMA_SPRAM256X8         (0x07 << 1)
#define BIT_PAGE_2_MBIST_CTL3_MBIST_MODE_SPRAM256X8        (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x7F MBIST_CTL4 Register                                (Default: 0x00)    */
#define REG_PAGE_2_MBIST_CTL4                              TX_PAGE_2, 0x7F

#define BIT_PAGE_2_MBIST_CTL4_MBIST_ERRMAP_SPRAM1024X32    (0x01 << 6)
#define BIT_PAGE_2_MBIST_CTL4_MBIST_FAIL_SPRAM1024X32      (0x01 << 5)
#define BIT_PAGE_2_MBIST_CTL4_MBIST_FINISH_SPRAM1024X32    (0x01 << 4)
#define MSK_PAGE_2_MBIST_CTL4_MBIST_EMA_SPRAM1024X32       (0x07 << 1)
#define BIT_PAGE_2_MBIST_CTL4_MBIST_MODE_SPRAM1024X32      (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x80 MBIST_CTL5 Register                                (Default: 0x00)    */
#define REG_PAGE_2_MBIST_CTL5                              TX_PAGE_2, 0x80

#define BIT_PAGE_2_MBIST_CTL5_MBIST_ERRMAP_SPRAM16384X8    (0x01 << 6)
#define BIT_PAGE_2_MBIST_CTL5_MBIST_FAIL_SPRAM16384X8      (0x01 << 5)
#define BIT_PAGE_2_MBIST_CTL5_MBIST_FINISH_SPRAM16384X8    (0x01 << 4)
#define MSK_PAGE_2_MBIST_CTL5_MBIST_EMA_SPRAM16384X8       (0x07 << 1)
#define BIT_PAGE_2_MBIST_CTL5_MBIST_MODE_SPRAM16384X8      (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x81 MHL Tx Control 2nd Register                        (Default: 0x00)    */
#define REG_PAGE_2_MHLTX_CTL2                              TX_PAGE_2, 0x81

#define BIT_PAGE_2_MHLTX_CTL2_TX_DATA_INV                  (0x01 << 6)

/*----------------------------------------------------------------------------*/
/* 0x82 MBIST_dpram4096x8 Status Register                  (Default: 0x00)    */
#define REG_PAGE_2_MBIST_HSIC                              TX_PAGE_2, 0x82

#define BIT_PAGE_2_MBIST_HSIC_MBIST_ERRMAPL2H_DPRAM4096X8  (0x01 << 6)
#define BIT_PAGE_2_MBIST_HSIC_MBIST_FAILL2H_DPRAM4096X8    (0x01 << 5)
#define BIT_PAGE_2_MBIST_HSIC_MBIST_FINISHL2H_DPRAM4096X8  (0x01 << 4)
#define BIT_PAGE_2_MBIST_HSIC_MBIST_ERRMAPH2L_DPRAM4096X8  (0x01 << 2)
#define BIT_PAGE_2_MBIST_HSIC_MBIST_FAILH2L_DPRAM4096X8    (0x01 << 1)
#define BIT_PAGE_2_MBIST_HSIC_MBIST_FINISHH2L_DPRAM4096X8  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x84 MHL Tx Control 5th Register                        (Default: 0x1F)    */
#define REG_PAGE_2_MHLTX_CTL5                              TX_PAGE_2, 0x84

#define MSK_PAGE_2_MHLTX_CTL5_TX_CLK_SHAPE_7_0             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x85 MHL Tx Control 6th Register                        (Default: 0xA0)    */
#define REG_PAGE_2_MHLTX_CTL6                              TX_PAGE_2, 0x85

#define MSK_PAGE_2_MHLTX_CTL6_EMI_SEL                      (0x07 << 5)
#define MSK_PAGE_2_MHLTX_CTL6_TX_CLK_SHAPE_9_8             (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x86 MBIST_CTL6 Register                                (Default: 0x00)    */
#define REG_PAGE_2_MBIST_CTL6                              TX_PAGE_2, 0x86

#define MSK_PAGE_2_MBIST_CTL6_MBIST_EMAB_RDPRAM288X8       (0x07 << 5)
#define MSK_PAGE_2_MBIST_CTL6_MBIST_EMAA_RDPRAM288X8       (0x07 << 2)
#define BIT_PAGE_2_MBIST_CTL6_MBIST_RET_RDPRAM288X8        (0x01 << 1)
#define BIT_PAGE_2_MBIST_CTL6_MBIST_MODE_RDPRAM288X8       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x87 MBIST_CTL7 Register                                (Default: 0x00)    */
#define REG_PAGE_2_MBIST_CTL7                              TX_PAGE_2, 0x87

#define MSK_PAGE_2_MBIST_CTL7_MBIST_EMAB_XDPRAM288X8       (0x07 << 5)
#define MSK_PAGE_2_MBIST_CTL7_MBIST_EMAA_XDPRAM288X8       (0x07 << 2)
#define BIT_PAGE_2_MBIST_CTL7_MBIST_RET_XDPRAM288X8        (0x01 << 1)
#define BIT_PAGE_2_MBIST_CTL7_MBIST_MODE_XDPRAM288X8       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x88 MBIST_XDPRDP288x8 Status Register                  (Default: 0x00)    */
#define REG_PAGE_2_MBIST_DP288X8STA                        TX_PAGE_2, 0x88

#define BIT_PAGE_2_MBIST_DP288X8STA_MBIST_ERRMAP_XPRAM288X8 (0x01 << 5)
#define BIT_PAGE_2_MBIST_DP288X8STA_MBIST_FAIL_XDPRAM288X8 (0x01 << 4)
#define BIT_PAGE_2_MBIST_DP288X8STA_MBIST_FINISH_XDPRAM288X8 (0x01 << 3)
#define BIT_PAGE_2_MBIST_DP288X8STA_MBIST_ERRMAP_RDPRAM288X8 (0x01 << 2)
#define BIT_PAGE_2_MBIST_DP288X8STA_MBIST_FAIL_RDPRAM288X8 (0x01 << 1)
#define BIT_PAGE_2_MBIST_DP288X8STA_MBIST_FINISH_RDPRAM288X8 (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x90 Packet Filter0 Register                            (Default: 0x00)    */
#define REG_PAGE_2_PKT_FILTER_0                            TX_PAGE_2, 0x90

#define BIT_PAGE_2_PKT_FILTER_0_DROP_CEA_GAMUT_PKT         (0x01 << 7)
#define BIT_PAGE_2_PKT_FILTER_0_DROP_CEA_CP_PKT            (0x01 << 6)
#define BIT_PAGE_2_PKT_FILTER_0_DROP_MPEG_PKT              (0x01 << 5)
#define BIT_PAGE_2_PKT_FILTER_0_DROP_SPIF_PKT              (0x01 << 4)
#define BIT_PAGE_2_PKT_FILTER_0_DROP_AIF_PKT               (0x01 << 3)
#define BIT_PAGE_2_PKT_FILTER_0_DROP_AVI_PKT               (0x01 << 2)
#define BIT_PAGE_2_PKT_FILTER_0_DROP_CTS_PKT               (0x01 << 1)
#define BIT_PAGE_2_PKT_FILTER_0_DROP_GCP_PKT               (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x91 Packet Filter1 Register                            (Default: 0x00)    */
#define REG_PAGE_2_PKT_FILTER_1                            TX_PAGE_2, 0x91

#define BIT_PAGE_2_PKT_FILTER_1_VSI_OVERRIDE_DIS           (0x01 << 7)
#define BIT_PAGE_2_PKT_FILTER_1_AVI_OVERRIDE_DIS           (0x01 << 6)
#define BIT_PAGE_2_PKT_FILTER_1_DROP_AUDIO_PKT             (0x01 << 3)
#define BIT_PAGE_2_PKT_FILTER_1_DROP_GEN2_PKT              (0x01 << 2)
#define BIT_PAGE_2_PKT_FILTER_1_DROP_GEN_PKT               (0x01 << 1)
#define BIT_PAGE_2_PKT_FILTER_1_DROP_VSIF_PKT              (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x92 DROP GEN PACKET TYPE 0 Register                    (Default: 0x00)    */
#define REG_PAGE_2_DROP_GEN_TYPE_0                         TX_PAGE_2, 0x92

#define MSK_PAGE_2_DROP_GEN_TYPE_0_DROP_GEN_TYPE           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x93 DROP GEN PACKET TYPE 1 Register                    (Default: 0x00)    */
#define REG_PAGE_2_DROP_GEN_TYPE_1                         TX_PAGE_2, 0x93

#define MSK_PAGE_2_DROP_GEN_TYPE_1_DROP_GEN2_TYPE          (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA0 TMDS Clock Status Register Page 3                  (Default: 0x10)    */
#define REG_PAGE_2_TMDS_CSTAT_P3                           TX_PAGE_2, 0xA0

#define BIT_PAGE_2_TMDS_CSTAT_P3_RX_HDMI_CP_CLR_MUTE       (0x01 << 7)
#define BIT_PAGE_2_TMDS_CSTAT_P3_RX_HDMI_CP_SET_MUTE       (0x01 << 6)
#define BIT_PAGE_2_TMDS_CSTAT_P3_RX_HDMI_CP_NEW_CP         (0x01 << 5)
#define BIT_PAGE_2_TMDS_CSTAT_P3_RX_HDMI_MUTE_ON           (0x01 << 4)
#define BIT_PAGE_2_TMDS_CSTAT_P3_SCDT                      (0x01 << 1)

#define BIT_PAGE_2_TMDS_CSTAT_P3_CKDT                      (0x01 << 0)
#define VAL_PAGE_2_TMDS_CSTAT_P3_CKDT_STOPPED              (0x00 << 0)
#define VAL_PAGE_2_TMDS_CSTAT_P3_CKDT_DETECTED             (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xA1 RX_HDMI Control Register0                          (Default: 0x10)    */
#define REG_PAGE_2_RX_HDMI_CTRL0                           TX_PAGE_2, 0xA1

#define BIT_PAGE_2_RX_HDMI_CTRL0_BYP_DVIFILT_SYNC          (0x01 << 5)
#define BIT_PAGE_2_RX_HDMI_CTRL0_HDMI_MODE_EN_ITSELF_CLR   (0x01 << 4)
#define BIT_PAGE_2_RX_HDMI_CTRL0_HDMI_MODE_SW_VALUE        (0x01 << 3)
#define BIT_PAGE_2_RX_HDMI_CTRL0_HDMI_MODE_OVERWRITE       (0x01 << 2)
#define BIT_PAGE_2_RX_HDMI_CTRL0_RX_HDMI_HDMI_MODE_EN      (0x01 << 1)
#define BIT_PAGE_2_RX_HDMI_CTRL0_RX_HDMI_HDMI_MODE         (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xA2 RX_HDMI Control Register1                          (Default: 0x50)    */
#define REG_PAGE_2_RX_HDMI_CTRL1                           TX_PAGE_2, 0xA2

#define MSK_PAGE_2_RX_HDMI_CTRL1_AV_MUTE_GET_CON           (0x03 << 6)
#define MSK_PAGE_2_RX_HDMI_CTRL1_MUTE_CON                  (0x03 << 4)
#define BIT_PAGE_2_RX_HDMI_CTRL1_AUDIO_MUTE                (0x01 << 3)
#define BIT_PAGE_2_RX_HDMI_CTRL1_VIDEO_MUTE                (0x01 << 2)
#define BIT_PAGE_2_RX_HDMI_CTRL1_ECC_AUTO_AUDIO_MUTE       (0x01 << 1)
#define BIT_PAGE_2_RX_HDMI_CTRL1_ECC_AUTO_VIDEO_MUTE       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xA3 RX_HDMI Control Register2                          (Default: 0x38)    */
#define REG_PAGE_2_RX_HDMI_CTRL2                           TX_PAGE_2, 0xA3

#define BIT_PAGE_2_RX_HDMI_CTRL2_USE_AV_MUTE               (0x01 << 3)
#define VAL_PAGE_2_RX_HDMI_CTRL2_USE_AV_MUTE_DISABLE       (0x00 << 3)
#define VAL_PAGE_2_RX_HDMI_CTRL2_USE_AV_MUTE_ENABLE        (0x01 << 3)

#define BIT_PAGE_2_RX_HDMI_CTRL2_VSI_MON_SEL               (0x01 << 0)
#define VAL_PAGE_2_RX_HDMI_CTRL2_VSI_MON_SEL_AVI           (0x00 << 0)
#define VAL_PAGE_2_RX_HDMI_CTRL2_VSI_MON_SEL_VSI           (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xA4 RX_HDMI Control Register3                          (Default: 0x0F)    */
#define REG_PAGE_2_RX_HDMI_CTRL3                           TX_PAGE_2, 0xA4

#define MSK_PAGE_2_RX_HDMI_CTRL3_PP_MODE_CLK_EN            (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xA5 RX_HDMI Control Register4                          (Default: 0x30)    */
#define REG_PAGE_2_RX_HDMI_CTRL4                           TX_PAGE_2, 0xA5

#define MSK_PAGE_2_RX_HDMI_CTRL4_NO_PKT_CNT                (0x0F << 4)

/*----------------------------------------------------------------------------*/
/* 0xA6 RX_HDMI Status Register0                           (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_STAT0                           TX_PAGE_2, 0xA6

#define BIT_PAGE_2_RX_HDMI_STAT0_RX_HDMI_DOWMSMPL_ON       (0x01 << 2)
#define BIT_PAGE_2_RX_HDMI_STAT0_RX_HDMI_RGB2YCC_ON        (0x01 << 1)
#define BIT_PAGE_2_RX_HDMI_STAT0_RX_HDMI_PP_MODE_ON        (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xA9 rx_hdmi_criteria_hen Register                      (Default: 0x0C)    */
#define REG_PAGE_2_RX_HDMI_CRITERIA_HEN                    TX_PAGE_2, 0xA9

#define BIT_PAGE_2_RX_HDMI_CRITERIA_HEN_BYPASSSYNCFILTER   (0x01 << 6)
#define BIT_PAGE_2_RX_HDMI_CRITERIA_HEN_BYPASSDATAALIGN    (0x01 << 5)
#define MSK_PAGE_2_RX_HDMI_CRITERIA_HEN_CRITERIA_HEN       (0x1F << 0)

/*----------------------------------------------------------------------------*/
/* 0xAA rx_hdmi_criteria_prmbl Register                    (Default: 0x06)    */
#define REG_PAGE_2_RX_HDMI_CRITERIA_PRMBL                  TX_PAGE_2, 0xAA

#define MSK_PAGE_2_RX_HDMI_CRITERIA_PRMBL_CRITERIA_PRMBL   (0x1F << 0)

/*----------------------------------------------------------------------------*/
/* 0xAB rx_hdmi BCH corrected threshold Register           (Default: 0x01)    */
#define REG_PAGE_2_RX_HDMI_BCH_CORRECTED_THRESHOLD         TX_PAGE_2, 0xAB

#define MSK_PAGE_2_RX_HDMI_BCH_CORRECTED_THRESHOLD_BCH_CORRECTED_THRESHOLD (0x1F << 0)

/*----------------------------------------------------------------------------*/
/* 0xAC rx_hdmi Clear Buffer Register                      (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_CLR_BUFFER                      TX_PAGE_2, 0xAC

#define MSK_PAGE_2_RX_HDMI_CLR_BUFFER_AIF4VSI_CMP          (0x03 << 6)
#define BIT_PAGE_2_RX_HDMI_CLR_BUFFER_USE_AIF4VSI          (0x01 << 5)
#define BIT_PAGE_2_RX_HDMI_CLR_BUFFER_VSI_CLR_W_AVI        (0x01 << 4)
#define BIT_PAGE_2_RX_HDMI_CLR_BUFFER_VSI_IEEE_ID_CHK_EN   (0x01 << 3)
#define BIT_PAGE_2_RX_HDMI_CLR_BUFFER_SWAP_VSI_IEEE_ID     (0x01 << 2)
#define BIT_PAGE_2_RX_HDMI_CLR_BUFFER_AIF_CLR_EN           (0x01 << 1)

#define BIT_PAGE_2_RX_HDMI_CLR_BUFFER_VSI_CLR_EN           (0x01 << 0)
#define VAL_PAGE_2_RX_HDMI_CLR_BUFFER_VSI_CLR_EN_STALE     (0x00 << 0)
#define VAL_PAGE_2_RX_HDMI_CLR_BUFFER_VSI_CLR_EN_CLEAR     (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xAD RX_HDMI AIF Header Register                        (Default: 0x84)    */
#define REG_PAGE_2_RX_HDMI_AIF_HEADER                      TX_PAGE_2, 0xAD

#define MSK_PAGE_2_RX_HDMI_AIF_HEADER_RX_HDMI_AIF_HEADER   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xAE RX_HDMI VSI Header Register                        (Default: 0x81)    */
#define REG_PAGE_2_RX_HDMI_VSI_HEADER                      TX_PAGE_2, 0xAE

#define MSK_PAGE_2_RX_HDMI_VSI_HEADER_RX_HDMI_VSI_HEADER   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xAF RX_HDMI ACP Header Register                        (Default: 0x04)    */
#define REG_PAGE_2_RX_HDMI_ACP_DEC_SYNC                    TX_PAGE_2, 0xAF

#define MSK_PAGE_2_RX_HDMI_ACP_DEC_SYNC_RX_HDMI_ACP_DEC_SYNC (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB1 RX_HDMI ISRC1 Header Register                      (Default: 0x05)    */
#define REG_PAGE_2_RX_HDMI_ISRC1_HEADER                    TX_PAGE_2, 0xB1

#define MSK_PAGE_2_RX_HDMI_ISRC1_HEADER_RX_HDMI_ISRC1_HEADER (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB2 RX_HDMI ISRC2 Header Register                      (Default: 0x06)    */
#define REG_PAGE_2_RX_HDMI_ISRC2_HEADER                    TX_PAGE_2, 0xB2

#define MSK_PAGE_2_RX_HDMI_ISRC2_HEADER_RX_HDMI_ISRC2_HEADER (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB3 RX_HDMI SPD Header Register                        (Default: 0x83)    */
#define REG_PAGE_2_RX_HDMI_SPD_DEC_SYNC                    TX_PAGE_2, 0xB3

#define MSK_PAGE_2_RX_HDMI_SPD_DEC_SYNC_RX_HDMI_SPD_DEC_SYNC (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB4 RX_HDMI VSI ID in PB1 Register                     (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_ALT_VSI_PB1                     TX_PAGE_2, 0xB4

#define MSK_PAGE_2_RX_HDMI_ALT_VSI_PB1_RX_HDMI_ALT_VSI_PB1 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB5 RX_HDMI VSI ID in PB2 Register                     (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_ALT_VSI_PB2                     TX_PAGE_2, 0xB5

#define MSK_PAGE_2_RX_HDMI_ALT_VSI_PB2_RX_HDMI_ALT_VSI_PB2 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB6 RX_HDMI VSI ID in PB3 Register                     (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_ALT_VSI_PB3                     TX_PAGE_2, 0xB6

#define MSK_PAGE_2_RX_HDMI_ALT_VSI_PB3_RX_HDMI_ALT_VSI_PB3 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB7 RX_HDMI VSI ID in PB4 Register                     (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_ALT_VSI_PB4                     TX_PAGE_2, 0xB7

#define MSK_PAGE_2_RX_HDMI_ALT_VSI_PB4_RX_HDMI_ALT_VSI_PB4 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB8 RX_HDMI VSI Header1 Register                       (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_HEADER1                 TX_PAGE_2, 0xB8

#define MSK_PAGE_2_RX_HDMI_MON_PKT_HEADER1_RX_HDMI_MON_PKT_HEADER_7_0 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB9 RX_HDMI VSI Header2 Register                       (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_HEADER2                 TX_PAGE_2, 0xB9

#define MSK_PAGE_2_RX_HDMI_MON_PKT_HEADER2_RX_HDMI_MON_PKT_HEADER_15_8 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBA RX_HDMI VSI Length Register                        (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_LENGTH                  TX_PAGE_2, 0xBA

#define MSK_PAGE_2_RX_HDMI_MON_PKT_LENGTH_RX_HDMI_MON_PKT_LENGTH (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBB RX_HDMI VSI PB0 Register                           (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_0                    TX_PAGE_2, 0xBB

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_0_RX_HDMI_MON_PKT_PB_0 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBC RX_HDMI VSI PB1 Register                           (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_1                    TX_PAGE_2, 0xBC

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_1_RX_HDMI_MON_PKT_PB_1 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBD RX_HDMI VSI PB2 Register                           (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_2                    TX_PAGE_2, 0xBD

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_2_RX_HDMI_MON_PKT_PB_2 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBE RX_HDMI VSI PB3 Register                           (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_3                    TX_PAGE_2, 0xBE

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_3_RX_HDMI_MON_PKT_PB_3 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBF RX_HDMI VSI PB4 Register                           (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_4                    TX_PAGE_2, 0xBF

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_4_RX_HDMI_MON_PKT_PB_4 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC0 RX_HDMI VSI PB5 Register                           (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_5                    TX_PAGE_2, 0xC0

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_5_RX_HDMI_MON_PKT_PB_5 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC1 RX_HDMI VSI PB6 Register                           (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_6                    TX_PAGE_2, 0xC1

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_6_RX_HDMI_MON_PKT_PB_6 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC2 RX_HDMI VSI PB7 Register                           (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_7                    TX_PAGE_2, 0xC2

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_7_RX_HDMI_MON_PKT_PB_7 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC3 RX_HDMI VSI PB8 Register                           (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_8                    TX_PAGE_2, 0xC3

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_8_RX_HDMI_MON_PKT_PB_8 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC4 RX_HDMI VSI PB9 Register                           (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_9                    TX_PAGE_2, 0xC4

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_9_RX_HDMI_MON_PKT_PB_9 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC5 RX_HDMI VSI PB10 Register                          (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_10                   TX_PAGE_2, 0xC5

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_10_RX_HDMI_MON_PKT_PB_10 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC6 RX_HDMI VSI PB11 Register                          (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_11                   TX_PAGE_2, 0xC6

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_11_RX_HDMI_MON_PKT_PB_11 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC7 RX_HDMI VSI PB12 Register                          (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_12                   TX_PAGE_2, 0xC7

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_12_RX_HDMI_MON_PKT_PB_12 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC8 RX_HDMI VSI PB13 Register                          (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_13                   TX_PAGE_2, 0xC8

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_13_RX_HDMI_MON_PKT_PB_13 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC9 RX_HDMI VSI PB14 Register                          (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_14                   TX_PAGE_2, 0xC9

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_14_RX_HDMI_MON_PKT_PB_14 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCA RX_HDMI VSI PB15 Register                          (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_15                   TX_PAGE_2, 0xCA

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_15_RX_HDMI_MON_PKT_PB_15 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCB RX_HDMI VSI PB16 Register                          (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_16                   TX_PAGE_2, 0xCB

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_16_RX_HDMI_MON_PKT_PB_16 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCC RX_HDMI VSI PB17 Register                          (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_17                   TX_PAGE_2, 0xCC

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_17_RX_HDMI_MON_PKT_PB_17 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCD RX_HDMI VSI PB18 Register                          (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_18                   TX_PAGE_2, 0xCD

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_18_RX_HDMI_MON_PKT_PB_18 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCE RX_HDMI VSI PB19 Register                          (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_19                   TX_PAGE_2, 0xCE

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_19_RX_HDMI_MON_PKT_PB_19 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCF RX_HDMI VSI PB20 Register                          (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_20                   TX_PAGE_2, 0xCF

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_20_RX_HDMI_MON_PKT_PB_20 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD0 RX_HDMI VSI PB21 Register                          (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_21                   TX_PAGE_2, 0xD0

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_21_RX_HDMI_MON_PKT_PB_21 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD1 RX_HDMI VSI PB22 Register                          (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_22                   TX_PAGE_2, 0xD1

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_22_RX_HDMI_MON_PKT_PB_22 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD2 RX_HDMI VSI PB23 Register                          (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_23                   TX_PAGE_2, 0xD2

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_23_RX_HDMI_MON_PKT_PB_23 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD3 RX_HDMI VSI PB24 Register                          (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_24                   TX_PAGE_2, 0xD3

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_24_RX_HDMI_MON_PKT_PB_24 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD4 RX_HDMI VSI PB25 Register                          (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_25                   TX_PAGE_2, 0xD4

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_25_RX_HDMI_MON_PKT_PB_25 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD5 RX_HDMI VSI PB26 Register                          (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_26                   TX_PAGE_2, 0xD5

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_26_RX_HDMI_MON_PKT_PB_26 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD6 RX_HDMI VSI PB27 Register                          (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MON_PKT_PB_27                   TX_PAGE_2, 0xD6

#define MSK_PAGE_2_RX_HDMI_MON_PKT_PB_27_RX_HDMI_MON_PKT_PB_27 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD7 RX_HDMI VSI MHL Monitor Register                   (Default: 0x3C)    */
#define REG_PAGE_2_RX_HDMI_VSIF_MHL_MON                    TX_PAGE_2, 0xD7

#define MSK_PAGE_2_RX_HDMI_VSIF_MHL_MON_RX_HDMI_MHL_3D_FORMAT (0x0F << 2)
#define MSK_PAGE_2_RX_HDMI_VSIF_MHL_MON_RX_HDMI_MHL_VID_FORMAT (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0xD8 RX_HDMI VSI MHL Extended 3D Format Register        (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MHL_VFMT_EXTD                   TX_PAGE_2, 0xD8

#define MSK_PAGE_2_RX_HDMI_MHL_VFMT_EXTD_MHL_VFMT_EXTD_5_0 (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0xD9 RX_HDMI VSI MHL rsv3 3D Format Register            (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MHL_VFMT_RSV3                   TX_PAGE_2, 0xD9

#define MSK_PAGE_2_RX_HDMI_MHL_VFMT_RSV3_MHL_VFMT_RSV3_5_0 (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0xDA RX_HDMI VSI MHL rsv4 3D Format Register            (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MHL_VFMT_RSV4                   TX_PAGE_2, 0xDA

#define MSK_PAGE_2_RX_HDMI_MHL_VFMT_RSV4_MHL_VFMT_RSV4_5_0 (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0xDB RX_HDMI VSI MHL rsv5 3D Format Register            (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MHL_VFMT_RSV5                   TX_PAGE_2, 0xDB

#define MSK_PAGE_2_RX_HDMI_MHL_VFMT_RSV5_MHL_VFMT_RSV5_5_0 (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0xDC RX_HDMI VSI MHL rsv6 3D Format Register            (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MHL_VFMT_RSV6                   TX_PAGE_2, 0xDC

#define MSK_PAGE_2_RX_HDMI_MHL_VFMT_RSV6_MHL_VFMT_RSV6_5_0 (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0xDD RX_HDMI VSI MHL rsv7 3D Format Register            (Default: 0x00)    */
#define REG_PAGE_2_RX_HDMI_MHL_VFMT_RSV7                   TX_PAGE_2, 0xDD

#define MSK_PAGE_2_RX_HDMI_MHL_VFMT_RSV7_MHL_VFMT_RSV7_5_0 (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0xE0 Interrupt Source 9 Register                        (Default: 0x00)    */
#define REG_PAGE_2_INTR9                                   TX_PAGE_2, 0xE0

#define BIT_PAGE_2_INTR9_INTR9_STAT6                       (0x01 << 6)
#define BIT_PAGE_2_INTR9_INTR9_STAT5                       (0x01 << 5)
#define BIT_PAGE_2_INTR9_INTR9_STAT4                       (0x01 << 4)
#define BIT_PAGE_2_INTR9_INTR9_STAT1                       (0x01 << 1)
#define BIT_PAGE_2_INTR9_INTR9_STAT0                       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xE1 Interrupt 9 Mask Register                          (Default: 0x00)    */
#define REG_PAGE_2_INTR9_MASK                              TX_PAGE_2, 0xE1

#define BIT_PAGE_2_INTR9_MASK_INTR9_MASK6                  (0x01 << 6)
#define VAL_PAGE_2_INTR9_MASK_INTR9_MASK6_DISABLE          (0x00 << 6)
#define VAL_PAGE_2_INTR9_MASK_INTR9_MASK6_ENABLE           (0x01 << 6)

#define BIT_PAGE_2_INTR9_MASK_INTR9_MASK5                  (0x01 << 5)
#define VAL_PAGE_2_INTR9_MASK_INTR9_MASK5_DISABLE          (0x00 << 5)
#define VAL_PAGE_2_INTR9_MASK_INTR9_MASK5_ENABLE           (0x01 << 5)

#define BIT_PAGE_2_INTR9_MASK_INTR9_MASK4                  (0x01 << 4)
#define VAL_PAGE_2_INTR9_MASK_INTR9_MASK4_DISABLE          (0x00 << 4)
#define VAL_PAGE_2_INTR9_MASK_INTR9_MASK4_ENABLE           (0x01 << 4)

#define BIT_PAGE_2_INTR9_MASK_INTR9_MASK1                  (0x01 << 1)
#define BIT_PAGE_2_INTR9_MASK_INTR9_MASK0                  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xE2 TPI CBUS Start Register                            (Default: 0x00)    */
#define REG_PAGE_2_TPI_CBUS_START                          TX_PAGE_2, 0xE2

#define BIT_PAGE_2_TPI_CBUS_START_RCP_REQ_START            (0x01 << 7)
#define BIT_PAGE_2_TPI_CBUS_START_RCPK_REPLY_START         (0x01 << 6)
#define BIT_PAGE_2_TPI_CBUS_START_RCPE_REPLY_START         (0x01 << 5)
#define BIT_PAGE_2_TPI_CBUS_START_PUT_LINK_MODE_START      (0x01 << 4)
#define BIT_PAGE_2_TPI_CBUS_START_PUT_DCAPCHG_START        (0x01 << 3)
#define BIT_PAGE_2_TPI_CBUS_START_PUT_DCAPRDY_START        (0x01 << 2)
#define BIT_PAGE_2_TPI_CBUS_START_GET_EDID_START_0         (0x01 << 1)
#define BIT_PAGE_2_TPI_CBUS_START_GET_DEVCAP_START         (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xE3 EDID Control Register                              (Default: 0x10)    */
#define REG_PAGE_2_EDID_CTRL                               TX_PAGE_2, 0xE3

#define BIT_PAGE_2_EDID_CTRL_EDID_PRIME_VALID              (0x01 << 7)
#define VAL_PAGE_2_EDID_CTRL_EDID_PRIME_VALID_DISABLE      (0x00 << 7)
#define VAL_PAGE_2_EDID_CTRL_EDID_PRIME_VALID_ENABLE       (0x01 << 7)

#define BIT_PAGE_2_EDID_CTRL_XDEVCAP_EN                    (0x01 << 6)

#define BIT_PAGE_2_EDID_CTRL_DEVCAP_SEL                    (0x01 << 5)
#define VAL_PAGE_2_EDID_CTRL_DEVCAP_SELECT_EDID            (0x00 << 5)
#define VAL_PAGE_2_EDID_CTRL_DEVCAP_SELECT_DEVCAP          (0x01 << 5)

#define BIT_PAGE_2_EDID_CTRL_EDID_FIFO_ADDR_AUTO           (0x01 << 4)
#define VAL_PAGE_2_EDID_CTRL_EDID_FIFO_ADDR_AUTO_DISABLE   (0x00 << 4)
#define VAL_PAGE_2_EDID_CTRL_EDID_FIFO_ADDR_AUTO_ENABLE    (0x01 << 4)

#define BIT_PAGE_2_EDID_CTRL_INVALID_BKSV                  (0x01 << 1)

#define BIT_PAGE_2_EDID_CTRL_EDID_MODE_EN                  (0x01 << 0)
#define VAL_PAGE_2_EDID_CTRL_EDID_MODE_EN_DISABLE          (0x00 << 0)
#define VAL_PAGE_2_EDID_CTRL_EDID_MODE_EN_ENABLE           (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xE4 EDID Status Register                               (Default: 0x00)    */
#define REG_PAGE_2_EDID_STAT                               TX_PAGE_2, 0xE4

#define BIT_PAGE_2_EDID_STAT_MSC_STATUS_BUSY               (0x01 << 4)
#define BIT_PAGE_2_EDID_STAT_MSC_DEVCAP_VALID              (0x01 << 1)

/*----------------------------------------------------------------------------*/
/* 0xE5 CBUS LINK_MODE Register                            (Default: 0x0B)    */
#define REG_PAGE_2_CBUS_LINK_MODE                          TX_PAGE_2, 0xE5

#define MSK_PAGE_2_CBUS_LINK_MODE_CBUS_LINK_MODE           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE6 RCPE ERRCODE Register                              (Default: 0x00)    */
#define REG_PAGE_2_RCPE_ERRCODE                            TX_PAGE_2, 0xE6

#define MSK_PAGE_2_RCPE_ERRCODE_RCPE_ERRCODE               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE7 RCPK KEYCODE Register                              (Default: 0x00)    */
#define REG_PAGE_2_RCPK_KEYCODE                            TX_PAGE_2, 0xE7

#define MSK_PAGE_2_RCPK_KEYCODE_RCPK_KEYCODE               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE8 RCP KEYCODE Register                               (Default: 0x00)    */
#define REG_PAGE_2_RCP_KEYCODE                             TX_PAGE_2, 0xE8

#define MSK_PAGE_2_RCP_KEYCODE_RCP_KEYCODE                 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE9 EDID FIFO Addr Register                            (Default: 0x00)    */
#define REG_PAGE_2_EDID_FIFO_ADDR                          TX_PAGE_2, 0xE9

#define MSK_PAGE_2_EDID_FIFO_ADDR_EDID_FIFO_ADDR           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xEA EDID FIFO Write Data Register                      (Default: 0x00)    */
#define REG_PAGE_2_EDID_FIFO_WR_DATA                       TX_PAGE_2, 0xEA

#define MSK_PAGE_2_EDID_FIFO_WR_DATA_EDID_FIFO_WR_DATA     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xEB EDID/DEVCAP FIFO Internal Addr Register            (Default: 0x00)    */
#define REG_PAGE_2_EDID_FIFO_ADDR_MON                      TX_PAGE_2, 0xEB

#define MSK_PAGE_2_EDID_FIFO_ADDR_MON_EDID_FIFO_ADDR_MON   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xEC EDID FIFO Read Data Register                       (Default: 0x00)    */
#define REG_PAGE_2_EDID_FIFO_RD_DATA                       TX_PAGE_2, 0xEC

#define MSK_PAGE_2_EDID_FIFO_RD_DATA_EDID_FIFO_RD_DATA     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xED EDID DDC Segment Pointer Register                  (Default: 0x00)    */
#define REG_PAGE_2_EDID_START_EXT                          TX_PAGE_2, 0xED

#define BIT_PAGE_2_EDID_START_EXT_GET_EDID_START_8         (0x01 << 7)
#define BIT_PAGE_2_EDID_START_EXT_GET_EDID_START_7         (0x01 << 6)
#define BIT_PAGE_2_EDID_START_EXT_GET_EDID_START_6         (0x01 << 5)
#define BIT_PAGE_2_EDID_START_EXT_GET_EDID_START_5         (0x01 << 4)
#define BIT_PAGE_2_EDID_START_EXT_GET_EDID_START_4         (0x01 << 3)
#define BIT_PAGE_2_EDID_START_EXT_GET_EDID_START_3         (0x01 << 2)
#define BIT_PAGE_2_EDID_START_EXT_GET_EDID_START_2         (0x01 << 1)
#define BIT_PAGE_2_EDID_START_EXT_GET_EDID_START_1         (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xEE RXBIST CNTL Register                               (Default: 0x00)    */
#define REG_PAGE_2_RX_PHY_BIST_CNTL                        TX_PAGE_2, 0xEE

#define BIT_PAGE_2_RX_PHY_BIST_CNTL_RX_GATING_EN           (0x01 << 6)
#define MSK_PAGE_2_RX_PHY_BIST_CNTL_RXPAT98_1_0            (0x03 << 4)
#define BIT_PAGE_2_RX_PHY_BIST_CNTL_RXENABLE_CTR_ALIGH_OFF (0x01 << 3)
#define BIT_PAGE_2_RX_PHY_BIST_CNTL_RXBAC_FLAG_SEL         (0x01 << 2)
#define BIT_PAGE_2_RX_PHY_BIST_CNTL_RXBAC_EN               (0x01 << 1)
#define BIT_PAGE_2_RX_PHY_BIST_CNTL_RXBIST_EN              (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xEF RXBIST INST LOW Register                           (Default: 0x00)    */
#define REG_PAGE_2_RX_PHY_BIST_INST_LOW                    TX_PAGE_2, 0xEF

#define MSK_PAGE_2_RX_PHY_BIST_INST_LOW_RXINSTRUCTION_7_0  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xF0 RXBIST INST HIGH Register                          (Default: 0x00)    */
#define REG_PAGE_2_RX_PHY_BIST_INST_HIGH                   TX_PAGE_2, 0xF0

#define MSK_PAGE_2_RX_PHY_BIST_INST_HIGH_RXBIST_STATE_2    (0x03 << 6)
#define MSK_PAGE_2_RX_PHY_BIST_INST_HIGH_RXBIST_STATE_1    (0x03 << 4)
#define MSK_PAGE_2_RX_PHY_BIST_INST_HIGH_RXBIST_STATE_0    (0x03 << 2)
#define MSK_PAGE_2_RX_PHY_BIST_INST_HIGH_RXINSTRUCTION_9_8 (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF1 RXBIST STATUS Register                             (Default: 0x00)    */
#define REG_PAGE_2_RX_PHY_BIST_STATUS                      TX_PAGE_2, 0xF1

#define MSK_PAGE_2_RX_PHY_BIST_STATUS_RXBIST_ERROR         (0x07 << 4)
#define BIT_PAGE_2_RX_PHY_BIST_STATUS_RXBAC_ERROR          (0x01 << 3)
#define BIT_PAGE_2_RX_PHY_BIST_STATUS_RXBIST_DONE          (0x01 << 2)
#define BIT_PAGE_2_RX_PHY_BIST_STATUS_RXBIST_ON            (0x01 << 1)
#define BIT_PAGE_2_RX_PHY_BIST_STATUS_RXBIST_RUN           (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF2 TX IP BIST CNTL and Status Register                (Default: 0x00)    */
#define REG_PAGE_2_TX_IP_BIST_CNTLSTA                      TX_PAGE_2, 0xF2

#define BIT_PAGE_2_TX_IP_BIST_CNTLSTA_TXBIST_DONE          (0x01 << 5)
#define BIT_PAGE_2_TX_IP_BIST_CNTLSTA_TXBIST_ON            (0x01 << 4)
#define BIT_PAGE_2_TX_IP_BIST_CNTLSTA_TXBIST_RUN           (0x01 << 3)
#define BIT_PAGE_2_TX_IP_BIST_CNTLSTA_TXCLK_HALF_SEL       (0x01 << 2)
#define BIT_PAGE_2_TX_IP_BIST_CNTLSTA_TXBIST_EN            (0x01 << 1)
#define BIT_PAGE_2_TX_IP_BIST_CNTLSTA_TXBIST_SEL           (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF3 TX IP BIST INST LOW Register                       (Default: 0x00)    */
#define REG_PAGE_2_TX_IP_BIST_INST_LOW                     TX_PAGE_2, 0xF3

#define MSK_PAGE_2_TX_IP_BIST_INST_LOW_TXINSTRUCTION_7_0   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xF4 TX IP BIST INST HIGH Register                      (Default: 0x00)    */
#define REG_PAGE_2_TX_IP_BIST_INST_HIGH                    TX_PAGE_2, 0xF4

#define MSK_PAGE_2_TX_IP_BIST_INST_HIGH_TXINSTRUCTION_10_8 (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF5 TX IP BIST PATTERN LOW Register                    (Default: 0x00)    */
#define REG_PAGE_2_TX_IP_BIST_PAT_LOW                      TX_PAGE_2, 0xF5

#define MSK_PAGE_2_TX_IP_BIST_PAT_LOW_TXPATTERN_7_0        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xF6 TX IP BIST PATTERN HIGH Register                   (Default: 0x00)    */
#define REG_PAGE_2_TX_IP_BIST_PAT_HIGH                     TX_PAGE_2, 0xF6

#define MSK_PAGE_2_TX_IP_BIST_PAT_HIGH_TXPATTERN_9_8       (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF7 TX IP BIST CONFIGURE LOW Register                  (Default: 0x00)    */
#define REG_PAGE_2_TX_IP_BIST_CONF_LOW                     TX_PAGE_2, 0xF7

#define MSK_PAGE_2_TX_IP_BIST_CONF_LOW_TXCONFIGURE_7_0     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xF8 TX IP BIST CONFIGURE HIGH Register                 (Default: 0x00)    */
#define REG_PAGE_2_TX_IP_BIST_CONF_HIGH                    TX_PAGE_2, 0xF8

#define MSK_PAGE_2_TX_IP_BIST_CONF_HIGH_TXCONFIGURE_9_8    (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF9 HSIC RX BIST CNTL Register                         (Default: 0x00)    */
#define REG_PAGE_2_HSIC_RX_BIST_CNTL                       TX_PAGE_2, 0xF9

#define MSK_PAGE_2_HSIC_RX_BIST_CNTL_HRX_ODD               (0x03 << 4)
#define MSK_PAGE_2_HSIC_RX_BIST_CNTL_HRX_INVERT            (0x03 << 2)
#define BIT_PAGE_2_HSIC_RX_BIST_CNTL_HRX_RUN_CTRL          (0x01 << 1)
#define BIT_PAGE_2_HSIC_RX_BIST_CNTL_HRX_BIST_EN           (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xFA HSIC RX BIST PATTERN Register                      (Default: 0x00)    */
#define REG_PAGE_2_HSIC_RX_BIST_PAT                        TX_PAGE_2, 0xFA

#define MSK_PAGE_2_HSIC_RX_BIST_PAT_HRX_PATTERN            (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xFB HSIC RXTX BIST STATUS Register                     (Default: 0x10)    */
#define REG_PAGE_2_HSIC_RX_BIST_STATUS                     TX_PAGE_2, 0xFB

#define BIT_PAGE_2_HSIC_RX_BIST_STATUS_HTXBIST_DONE        (0x01 << 7)
#define BIT_PAGE_2_HSIC_RX_BIST_STATUS_HTXBIST_RUN         (0x01 << 6)
#define BIT_PAGE_2_HSIC_RX_BIST_STATUS_HRX_STROBE          (0x01 << 4)
#define BIT_PAGE_2_HSIC_RX_BIST_STATUS_HRX_DATA            (0x01 << 3)
#define BIT_PAGE_2_HSIC_RX_BIST_STATUS_HRXBIST_ERROR       (0x01 << 2)
#define BIT_PAGE_2_HSIC_RX_BIST_STATUS_HRXBIST_DONE        (0x01 << 1)
#define BIT_PAGE_2_HSIC_RX_BIST_STATUS_HRXBIST_RUN         (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xFC HSIC TX BIST CNTL1 Register                        (Default: 0x10)    */
#define REG_PAGE_2_HSIC_TX_BIST_CNTL1                      TX_PAGE_2, 0xFC

#define MSK_PAGE_2_HSIC_TX_BIST_CNTL1_HTX_NUM_KEEP_CYCLE   (0x03 << 4)
#define BIT_PAGE_2_HSIC_TX_BIST_CNTL1_HTX_RUN_CTRL         (0x01 << 2)
#define BIT_PAGE_2_HSIC_TX_BIST_CNTL1_HTX_BIST_EN          (0x01 << 1)
#define BIT_PAGE_2_HSIC_TX_BIST_CNTL1_HTX_BIST_SEL         (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xFD HSIC TX BIST CNTL2 Register                        (Default: 0x00)    */
#define REG_PAGE_2_HSIC_TX_BIST_CNTL2                      TX_PAGE_2, 0xFD

#define MSK_PAGE_2_HSIC_TX_BIST_CNTL2_HTX_STROBE_ODD       (0x03 << 6)
#define MSK_PAGE_2_HSIC_TX_BIST_CNTL2_HTX_STROBE_INVERT    (0x03 << 4)
#define MSK_PAGE_2_HSIC_TX_BIST_CNTL2_HTX_DATA_ODD         (0x03 << 2)
#define MSK_PAGE_2_HSIC_TX_BIST_CNTL2_HTX_DATA_INVERT      (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0xFE HSIC TX BIST DATA PATTERN Register                 (Default: 0x00)    */
#define REG_PAGE_2_HSIC_TX_BIST_DATA_PAT                   TX_PAGE_2, 0xFE

#define MSK_PAGE_2_HSIC_TX_BIST_DATA_PAT_HTX_DATA_PATTERN  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xFF HSIC TX BIST STROBE PATTERN Register               (Default: 0xF0)    */
#define REG_PAGE_2_HSIC_TX_BIST_STROBE_PAT                 TX_PAGE_2, 0xFF

#define MSK_PAGE_2_HSIC_TX_BIST_STROBE_PAT_HTX_STROBE_PATTERN (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* Registers in TX_PAGE_3                                                     */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* 0x00 E-MSC General Control Register                     (Default: 0x00)    */
#define REG_PAGE_3_GENCTL                                  TX_PAGE_3, 0x00

#define BIT_PAGE_3_GENCTL_SPEC_TRANS_DIS                   (0x01 << 7)
#define BIT_PAGE_3_GENCTL_DIS_XMIT_ERR_STATE               (0x01 << 6)
#define BIT_PAGE_3_GENCTL_SPI_MISO_EDGE                    (0x01 << 5)
#define BIT_PAGE_3_GENCTL_SPI_MOSI_EDGE                    (0x01 << 4)
#define BIT_PAGE_3_GENCTL_CLR_EMSC_RFIFO                   (0x01 << 3)
#define BIT_PAGE_3_GENCTL_CLR_EMSC_XFIFO                   (0x01 << 2)
#define BIT_PAGE_3_GENCTL_START_TRAIN_SEQ                  (0x01 << 1)
#define BIT_PAGE_3_GENCTL_EMSC_EN                          (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x01 E-MSC Comma Char1 Register                         (Default: 0x96)    */
#define REG_PAGE_3_COMMACH1                                TX_PAGE_3, 0x01

#define MSK_PAGE_3_COMMACH1_COMMA_CHAR1                    (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x02 E-MSC Comma Char2 Register                         (Default: 0x69)    */
#define REG_PAGE_3_COMMACH2                                TX_PAGE_3, 0x02

#define MSK_PAGE_3_COMMACH2_COMMA_CHAR2                    (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x03 E-MSC Null Char Register                           (Default: 0x00)    */
#define REG_PAGE_3_NULLCH                                  TX_PAGE_3, 0x03

#define MSK_PAGE_3_NULLCH_NULL_CHAR                        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x04 E-MSC Train Char Register                          (Default: 0xFF)    */
#define REG_PAGE_3_TRAINCH                                 TX_PAGE_3, 0x04

#define MSK_PAGE_3_TRAINCH_TRAIN_CHAR                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x05 E-MSC Comma ErrorCNT Register                      (Default: 0x03)    */
#define REG_PAGE_3_COMMECNT                                TX_PAGE_3, 0x05

#define BIT_PAGE_3_COMMECNT_I2C_TO_EMSC_EN                 (0x01 << 7)
#define MSK_PAGE_3_COMMECNT_COMMA_CHAR_ERR_CNT             (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x06 E-MSC Comma Window Register                        (Default: 0x18)    */
#define REG_PAGE_3_COMMAWIN                                TX_PAGE_3, 0x06

#define MSK_PAGE_3_COMMAWIN_COMMA_CHAR_WIN                 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x07 E-MSC TX Train ByteCnt Low Register                (Default: 0x04)    */
#define REG_PAGE_3_TXTRAINBCNTL                            TX_PAGE_3, 0x07

#define MSK_PAGE_3_TXTRAINBCNTL_TXTRAIN_CHAR_BYTE_CNT_7_0  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x08 E-MSC TX Train ByteCnt High Register               (Default: 0x01)    */
#define REG_PAGE_3_TXTRAINBCNTH                            TX_PAGE_3, 0x08

#define BIT_PAGE_3_TXTRAINBCNTH_TXTRAIN_CHAR_BYTE_CNT_8    (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x09 E-MSC Skip ByteCnt Low Register                    (Default: 0x04)    */
#define REG_PAGE_3_SKIPBCNTL                               TX_PAGE_3, 0x09

#define MSK_PAGE_3_SKIPBCNTL_SKIP_BYTE_CNT_7_0             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0A E-MSC Skip ByteCnt High Register                   (Default: 0x01)    */
#define REG_PAGE_3_SKIPBCNTH                               TX_PAGE_3, 0x0A

#define BIT_PAGE_3_SKIPBCNTH_SKIP_BYTE_CNT_8               (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x0B E-MSC Rcv Tout Register                            (Default: 0x09)    */
#define REG_PAGE_3_EMSCRCVTOUT                             TX_PAGE_3, 0x0B

#define MSK_PAGE_3_EMSCRCVTOUT_EMSC_RCV_TOUT_MSB           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0C E-MSC Xmit Tout Register                           (Default: 0x14)    */
#define REG_PAGE_3_EMSCXMITTOUT                            TX_PAGE_3, 0x0C

#define MSK_PAGE_3_EMSCXMITTOUT_EMSC_XMIT_TOUT_MSB         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0D E-MSC Tx Xmit Retry Max Low  Register              (Default: 0x20)    */
#define REG_PAGE_3_EMSCXMITRETRYMAXL                       TX_PAGE_3, 0x0D

#define MSK_PAGE_3_EMSCXMITRETRYMAXL_EMSC_XMIT_RETRY_MAX_MSB_7_0 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0E E-MSC Tx Xmit Retry Max High Register              (Default: 0x00)    */
#define REG_PAGE_3_EMSCXMITRETRYMAXH                       TX_PAGE_3, 0x0E

#define MSK_PAGE_3_EMSCXMITRETRYMAXH_EMSC_XMIT_RETRY_MAX_MSB_10_8 (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0x0F E-MSC Tx Xmit Retry Limit Register                 (Default: 0xFF)    */
#define REG_PAGE_3_EMSCXMITRETRYLMT                        TX_PAGE_3, 0x0F

#define MSK_PAGE_3_EMSCXMITRETRYLMT_EMSC_XMIT_RETRY_LIMIT  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x10 SPI CBUSWr SlowMode DummyCnt Register              (Default: 0x0C)    */
#define REG_PAGE_3_SPICWRDCNT                              TX_PAGE_3, 0x10

#define MSK_PAGE_3_SPICWRDCNT_SPI_CBUSWRDCNT               (0x1F << 0)

/*----------------------------------------------------------------------------*/
/* 0x11 SPI CBUSRd SlowMode DummyCnt Register              (Default: 0x13)    */
#define REG_PAGE_3_SPICRDDCNT                              TX_PAGE_3, 0x11

#define MSK_PAGE_3_SPICRDDCNT_SPI_CBUSRDDCNT               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x12 EMSC Space Check Register                          (Default: 0x04)    */
#define REG_PAGE_3_EMSCSPCHK                               TX_PAGE_3, 0x12

#define MSK_PAGE_3_EMSCSPCHK_SPACE_CHECK_NUM               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x13 EMSC Wait Check Register                           (Default: 0x10)    */
#define REG_PAGE_3_EMSCWTCHK                               TX_PAGE_3, 0x13

#define MSK_PAGE_3_EMSCWTCHK_WAIT_CHECK_SPACE_CNT          (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x14 E-MSC Xmit Status Register                         (Default: 0x00)    */
#define REG_PAGE_3_EMSCXMITSTAT                            TX_PAGE_3, 0x14

#define MSK_PAGE_3_EMSCXMITSTAT_EMSC_XMITSTAT              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x15 E-MSC Rcvd Status Register                         (Default: 0x00)    */
#define REG_PAGE_3_EMSCRCVDSTAT                            TX_PAGE_3, 0x15

#define MSK_PAGE_3_EMSCRCVDSTAT_EMSC_RCVDSTAT              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x16 MEM XFIFO ByteCnt Status Register                  (Default: 0x00)    */
#define REG_PAGE_3_MEMXFIFOBCNT                            TX_PAGE_3, 0x16

#define MSK_PAGE_3_MEMXFIFOBCNT_MEM_XFIFO_BYTE_CNT_4_0     (0x1F << 0)

/*----------------------------------------------------------------------------*/
/* 0x17 MEM RFIFO ByteCnt Status Register                  (Default: 0x00)    */
#define REG_PAGE_3_MEMRFIFOBCNT                            TX_PAGE_3, 0x17

#define MSK_PAGE_3_MEMRFIFOBCNT_MEM_RFIFO_BYTE_CNT_4_0     (0x1F << 0)

/*----------------------------------------------------------------------------*/
/* 0x18 E-MSC XFIFO ByteCnt Status Low Register            (Default: 0x00)    */
#define REG_PAGE_3_EMSCXFIFOBCNTL                          TX_PAGE_3, 0x18

#define MSK_PAGE_3_EMSCXFIFOBCNTL_EMSC_XFIFO_BYTE_CNT_7_0  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x19 E-MSC XFIFO ByteCnt Status High Register           (Default: 0x00)    */
#define REG_PAGE_3_EMSCXFIFOBCNTH                          TX_PAGE_3, 0x19

#define BIT_PAGE_3_EMSCXFIFOBCNTH_EMSC_XFIFO_BYTE_CNT_8    (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x1A E-MSC RFIFO ByteCnt Status Low Register            (Default: 0x00)    */
#define REG_PAGE_3_EMSCRFIFOBCNTL                          TX_PAGE_3, 0x1A

#define MSK_PAGE_3_EMSCRFIFOBCNTL_EMSC_RFIFO_BYTE_CNT_7_0  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x1B E-MSC RFIFO ByteCnt Status High Register           (Default: 0x00)    */
#define REG_PAGE_3_EMSCRFIFOBCNTH                          TX_PAGE_3, 0x1B

#define BIT_PAGE_3_EMSCRFIFOBCNTH_EMSC_RFIFO_BYTE_CNT_8    (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x1C E-MSC XFIFO Space Status Low Register              (Default: 0x20)    */
#define REG_PAGE_3_EMSCXFIFOSPACEL                         TX_PAGE_3, 0x1C

#define MSK_PAGE_3_EMSCXFIFOSPACEL_EMSC_XFIFO_SPACE_7_0    (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x1D E-MSC XFIFO Space Status High Register             (Default: 0x01)    */
#define REG_PAGE_3_EMSCXFIFOSPACEH                         TX_PAGE_3, 0x1D

#define BIT_PAGE_3_EMSCXFIFOSPACEH_EMSC_XFIFO_SPACE_8      (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x1E SPI Burst Cnt Status Register                      (Default: 0x00)    */
#define REG_PAGE_3_SPIBURSTCNT                             TX_PAGE_3, 0x1E

#define MSK_PAGE_3_SPIBURSTCNT_SPI_BURST_CNT               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x1F EMSC XMIT TOState Register                         (Default: 0x00)    */
#define REG_PAGE_3_EMSCTOSTT                               TX_PAGE_3, 0x1F

#define MSK_PAGE_3_EMSCTOSTT_XMIT_TIMEOUT_STATE            (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x20 EMSC PRAM XFIFO Byte Count  Register               (Default: 0x00)    */
#define REG_PAGE_3_PRAMXFFFBCNT                            TX_PAGE_3, 0x20

#define MSK_PAGE_3_PRAMXFFFBCNT_PRAM_XFIFO_BYTE_CNT        (0x1F << 0)

/*----------------------------------------------------------------------------*/
/* 0x21 EMSC PRAM RFIFO Byte Count  Register               (Default: 0x00)    */
#define REG_PAGE_3_PRAMRFFFBCNT                            TX_PAGE_3, 0x21

#define MSK_PAGE_3_PRAMRFFFBCNT_PRAM_RFIFO_BYTE_CNT        (0x1F << 0)

/*----------------------------------------------------------------------------*/
/* 0x22 SPI Burst Status and SWRST Register                (Default: 0x00)    */
#define REG_PAGE_3_SPIBURSTSTAT                            TX_PAGE_3, 0x22

#define BIT_PAGE_3_SPIBURSTSTAT_SPI_HDCPRST                (0x01 << 7)
#define BIT_PAGE_3_SPIBURSTSTAT_SPI_CBUSRST                (0x01 << 6)
#define BIT_PAGE_3_SPIBURSTSTAT_SPI_SRST                   (0x01 << 5)
#define BIT_PAGE_3_SPIBURSTSTAT_EMSC_NORMAL_MODE           (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x23 E-MSC Interrupt Register                           (Default: 0x00)    */
#define REG_PAGE_3_EMSCINTR                                TX_PAGE_3, 0x23

#define BIT_PAGE_3_EMSCINTR_EMSC_XFIFO_EMPTY               (0x01 << 7)
#define BIT_PAGE_3_EMSCINTR_EMSC_XMIT_ACK_TOUT             (0x01 << 6)
#define BIT_PAGE_3_EMSCINTR_SPI_EMSC_READ_ERR              (0x01 << 5)
#define BIT_PAGE_3_EMSCINTR_SPI_EMSC_WRITE_ERR             (0x01 << 4)
#define BIT_PAGE_3_EMSCINTR_SPI_COMMA_CHAR_ERR             (0x01 << 3)
#define BIT_PAGE_3_EMSCINTR_EMSC_XMITDONE                  (0x01 << 2)
#define BIT_PAGE_3_EMSCINTR_EMSC_XMIT_GNT_TOUT             (0x01 << 1)
#define BIT_PAGE_3_EMSCINTR_SPI_DVLD                       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x24 E-MSC Interrupt Mask Register                      (Default: 0x00)    */
#define REG_PAGE_3_EMSCINTRMASK                            TX_PAGE_3, 0x24

#define BIT_PAGE_3_EMSCINTRMASK_EMSC_INTRMASK7             (0x01 << 7)
#define BIT_PAGE_3_EMSCINTRMASK_EMSC_INTRMASK6             (0x01 << 6)
#define BIT_PAGE_3_EMSCINTRMASK_EMSC_INTRMASK5             (0x01 << 5)
#define BIT_PAGE_3_EMSCINTRMASK_EMSC_INTRMASK4             (0x01 << 4)
#define BIT_PAGE_3_EMSCINTRMASK_EMSC_INTRMASK3             (0x01 << 3)
#define BIT_PAGE_3_EMSCINTRMASK_EMSC_INTRMASK2             (0x01 << 2)
#define BIT_PAGE_3_EMSCINTRMASK_EMSC_INTRMASK1             (0x01 << 1)
#define BIT_PAGE_3_EMSCINTRMASK_EMSC_INTRMASK0             (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x25 E-MSC RX Train ByteCnt Low Register                (Default: 0x04)    */
#define REG_PAGE_3_RXTRAINBCNTL                            TX_PAGE_3, 0x25

#define MSK_PAGE_3_RXTRAINBCNTL_RXTRAIN_CHAR_BYTE_CNT_7_0  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x26 E-MSC RX Train ByteCnt High Register               (Default: 0x01)    */
#define REG_PAGE_3_RXTRAINBCNTH                            TX_PAGE_3, 0x26

#define BIT_PAGE_3_RXTRAINBCNTH_RXTRAIN_CHAR_BYTE_CNT_8    (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x27 E-MSC Grant Retry Cnt Status Register              (Default: 0x00)    */
#define REG_PAGE_3_EMSCGNTRTYCNT                           TX_PAGE_3, 0x27

#define MSK_PAGE_3_EMSCGNTRTYCNT_EMSC_GNT_RETRY_CNT        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x28 E-MSC ACK Retry Cnt Status Register                (Default: 0x00)    */
#define REG_PAGE_3_EMSCACKRTYCNT                           TX_PAGE_3, 0x28

#define MSK_PAGE_3_EMSCACKRTYCNT_EMSC_ACK_RETRY_CNT        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x29 SPI E-MSC FIFO Status Register                     (Default: 0x00)    */
#define REG_PAGE_3_SPIEMSCFIFOSTAT                         TX_PAGE_3, 0x29

#define MSK_PAGE_3_SPIEMSCFIFOSTAT_SPI_EMSC_FIFO_STATUS    (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x2A I2C E-MSC XMIT FIFO Write Port Register            (Default: 0x00)    */
#define REG_PAGE_3_EMSC_XMIT_WRITE_PORT                    TX_PAGE_3, 0x2A

#define MSK_PAGE_3_EMSC_XMIT_WRITE_PORT_EMSC_XMIT_WRITE_PORT (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x2B I2C E-MSC RCV FIFO Write Port Register             (Default: 0x00)    */
#define REG_PAGE_3_EMSC_RCV_READ_PORT                      TX_PAGE_3, 0x2B

#define MSK_PAGE_3_EMSC_RCV_READ_PORT_EMSC_RCV_READ_PORT   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x30 MHL Top Ctl Register                               (Default: 0x00)    */
#define REG_PAGE_3_MHL_TOP_CTL                             TX_PAGE_3, 0x30

#define BIT_PAGE_3_MHL_TOP_CTL_MHL3_DOC_SEL                (0x01 << 7)
#define BIT_PAGE_3_MHL_TOP_CTL_MHL_PP_SEL                  (0x01 << 6)
#define MSK_PAGE_3_MHL_TOP_CTL_IF_TIMING_CTL               (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x31 MHL DataPath 1st Ctl Register                      (Default: 0xBC)    */
#define REG_PAGE_3_MHL_DP_CTL0                             TX_PAGE_3, 0x31

#define BIT_PAGE_3_MHL_DP_CTL0_DP_OE                       (0x01 << 7)
#define BIT_PAGE_3_MHL_DP_CTL0_TX_OE_OVR                   (0x01 << 6)
#define MSK_PAGE_3_MHL_DP_CTL0_TX_OE                       (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0x32 MHL DataPath 2nd Ctl Register                      (Default: 0xBB)    */
#define REG_PAGE_3_MHL_DP_CTL1                             TX_PAGE_3, 0x32

#define MSK_PAGE_3_MHL_DP_CTL1_CK_SWING_CTL                (0x0F << 4)
#define MSK_PAGE_3_MHL_DP_CTL1_DT_SWING_CTL                (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x33 MHL DataPath 3rd Ctl Register                      (Default: 0x2F)    */
#define REG_PAGE_3_MHL_DP_CTL2                             TX_PAGE_3, 0x33

#define BIT_PAGE_3_MHL_DP_CTL2_CLK_BYPASS_EN               (0x01 << 7)
#define MSK_PAGE_3_MHL_DP_CTL2_DAMP_TERM_SEL               (0x03 << 4)
#define MSK_PAGE_3_MHL_DP_CTL2_CK_TERM_SEL                 (0x03 << 2)
#define MSK_PAGE_3_MHL_DP_CTL2_DT_TERM_SEL                 (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x34 MHL DataPath 4th Ctl Register                      (Default: 0x48)    */
#define REG_PAGE_3_MHL_DP_CTL3                             TX_PAGE_3, 0x34

#define MSK_PAGE_3_MHL_DP_CTL3_DT_DRV_VNBC_CTL             (0x0F << 4)
#define MSK_PAGE_3_MHL_DP_CTL3_DT_DRV_VNB_CTL              (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x35 MHL DataPath 5th Ctl Register                      (Default: 0x48)    */
#define REG_PAGE_3_MHL_DP_CTL4                             TX_PAGE_3, 0x35

#define MSK_PAGE_3_MHL_DP_CTL4_CK_DRV_VNBC_CTL             (0x0F << 4)
#define MSK_PAGE_3_MHL_DP_CTL4_CK_DRV_VNB_CTL              (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x36 MHL DataPath 6th Ctl Register                      (Default: 0x3F)    */
#define REG_PAGE_3_MHL_DP_CTL5                             TX_PAGE_3, 0x36

#define BIT_PAGE_3_MHL_DP_CTL5_RSEN_EN_OVR                 (0x01 << 7)
#define BIT_PAGE_3_MHL_DP_CTL5_RSEN_EN                     (0x01 << 6)
#define MSK_PAGE_3_MHL_DP_CTL5_DAMP_TERM_VGS_CTL           (0x03 << 4)
#define MSK_PAGE_3_MHL_DP_CTL5_CK_TERM_VGS_CTL             (0x03 << 2)
#define MSK_PAGE_3_MHL_DP_CTL5_DT_TERM_VGS_CTL             (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x37 MHL PLL 1st Ctl Register                           (Default: 0x05)    */
#define REG_PAGE_3_MHL_PLL_CTL0                            TX_PAGE_3, 0x37

#define BIT_PAGE_3_MHL_PLL_CTL0_AUD_CLK_EN                 (0x01 << 7)
#define MSK_PAGE_3_MHL_PLL_CTL0_AUD_CLK_RATIO              (0x07 << 4) // bits 6:4
#define BIT_PAGE_3_MHL_PLL_CTL0_AUD_CLK_RATIO_5_10         (0x07 << 4) // 3'b111 5/10 hdmi_clk_freq
#define BIT_PAGE_3_MHL_PLL_CTL0_AUD_CLK_RATIO_5_6          (0x06 << 4) // 3'b110 5/6 hdmi_clk_freq
#define BIT_PAGE_3_MHL_PLL_CTL0_AUD_CLK_RATIO_5_4          (0x05 << 4) // 3'b101 5/4 hdmi_clk_freq
//TODO: #define BIT_PAGE_3_MHL_PLL_CTL0_AUD_CLK_RATIO_5_2          (0x04 << 4) // 3'b100 5/2 hdmi_clk_freq
#define BIT_PAGE_3_MHL_PLL_CTL0_AUD_CLK_RATIO_5_5          (0x03 << 4) // 3'b011 5/5 hdmi_clk_freq
#define BIT_PAGE_3_MHL_PLL_CTL0_AUD_CLK_RATIO_5_3          (0x02 << 4) // 3'b010 5/3 hdmi_clk_freq
#define BIT_PAGE_3_MHL_PLL_CTL0_AUD_CLK_RATIO_5_2_PRIME    (0x01 << 4) // 3'b001 5/2 hdmi_clk_freq
#define BIT_PAGE_3_MHL_PLL_CTL0_AUD_CLK_RATIO_5_1          (0x00 << 4) // 3'b000 5/1 hdmi_clk_freq

#define MSK_PAGE_3_MHL_PLL_CTL0_HDMI_CLK_RATIO             (0x03 << 2)
#define BIT_PAGE_3_MHL_PLL_CTL0_HDMI_CLK_RATIO_4X          (0x03 << 2) // 2'b11 4x mode, hdmi_clk_freq = 4*pxl_clk_freq
#define BIT_PAGE_3_MHL_PLL_CTL0_HDMI_CLK_RATIO_2X          (0x02 << 2) // 2'b10 2x mode, hdmi_clk_freq = 2*pxl_clk_freq
#define BIT_PAGE_3_MHL_PLL_CTL0_HDMI_CLK_RATIO_1X          (0x01 << 2) // 2'b01 1x mode, hdmi_clk_freq = 1*pxl_clk_freq
#define BIT_PAGE_3_MHL_PLL_CTL0_HDMI_CLK_RATIO_HALF_X      (0x00 << 2) // 2'b00 0.5x mode, hdmi_clk_freq = 0.5*pxl_clk_freq

#define BIT_PAGE_3_MHL_PLL_CTL0_CRYSTAL_CLK_SEL            (0x01 << 1)
#define BIT_PAGE_3_MHL_PLL_CTL0_ZONE_MASK_OE               (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x38 MHL PLL 2nd Ctl Register                           (Default: 0xFC)    */
#define REG_PAGE_3_MHL_PLL_CTL1                            TX_PAGE_3, 0x38

#define MSK_PAGE_3_MHL_PLL_CTL1_FVCO_CTL                   (0x0F << 4)
#define MSK_PAGE_3_MHL_PLL_CTL1_PLL_BW_CTL                 (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x39 MHL PLL 3rd Ctl Register                           (Default: 0x80)    */
#define REG_PAGE_3_MHL_PLL_CTL2                            TX_PAGE_3, 0x39

#define BIT_PAGE_3_MHL_PLL_CTL2_CLKDETECT_EN               (0x01 << 7)
#define BIT_PAGE_3_MHL_PLL_CTL2_MEAS_FVCO                  (0x01 << 3)
#define BIT_PAGE_3_MHL_PLL_CTL2_PLL_FAST_LOCK              (0x01 << 2)
#define MSK_PAGE_3_MHL_PLL_CTL2_PLL_LF_SEL                 (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x3A MHL BIAS 1st Ctl Register                          (Default: 0xB8)    */
#define REG_PAGE_3_MHL_BIAS_CTL0                           TX_PAGE_3, 0x3A

#define BIT_PAGE_3_MHL_BIAS_CTL0_BIAS_SEL                  (0x01 << 7)
#define MSK_PAGE_3_MHL_BIAS_CTL0_BIAS_SW_CTL               (0x07 << 4)
#define MSK_PAGE_3_MHL_BIAS_CTL0_BGR_CTL                   (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x3B MHL BIAS 2nd Ctl Register                          (Default: 0x07)    */
#define REG_PAGE_3_MHL_BIAS_CTL1                           TX_PAGE_3, 0x3B

#define MSK_PAGE_3_MHL_BIAS_CTL1_RSWING_CTL                (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x3C MHL BIAS 3rd Ctl Register                          (Default: 0x07)    */
#define REG_PAGE_3_MHL_BIAS_CTL2                           TX_PAGE_3, 0x3C

#define MSK_PAGE_3_MHL_BIAS_CTL2_BIAS_TERM_SEL             (0x03 << 3)
#define MSK_PAGE_3_MHL_BIAS_CTL2_IBIAS_SEL                 (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0x3D MHL Misc 1st Ctl Register                          (Default: 0x00)    */
#define REG_PAGE_3_MHL_MISC_CTL0                           TX_PAGE_3, 0x3D

#define MSK_PAGE_3_MHL_MISC_CTL0_TEST_MODE                 (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0x3E MHL Misc 2nd Ctl Register                          (Default: 0x00)    */
#define REG_PAGE_3_MHL_MISC_CTL1                           TX_PAGE_3, 0x3E

#define MSK_PAGE_3_MHL_MISC_CTL1_RSV_7_0                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x3F MHL Misc 3rd Ctl Register                          (Default: 0x00)    */
#define REG_PAGE_3_MHL_MISC_CTL2                           TX_PAGE_3, 0x3F

#define MSK_PAGE_3_MHL_MISC_CTL2_RSV_15_8                  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x40 MHL CBUS 1st Ctl Register                          (Default: 0x12)    */
#define REG_PAGE_3_MHL_CBUS_CTL0                           TX_PAGE_3, 0x40

#define BIT_PAGE_3_MHL_CBUS_CTL0_CBUS_RGND_VBIAS_734       (0x00)

#define BIT_PAGE_3_MHL_CBUS_CTL0_CBUS_DRV_SEL_STRONG       (0x02) /* default */

#define BIT_PAGE_3_MHL_CBUS_CTL0_CBUS_RGND_TEST_MODE       (0x01 << 7)
#define MSK_PAGE_3_MHL_CBUS_CTL0_CBUS_RGND_VTH_CTL         (0x03 << 4)
#define MSK_PAGE_3_MHL_CBUS_CTL0_CBUS_RES_TEST_SEL         (0x03 << 2)
#define MSK_PAGE_3_MHL_CBUS_CTL0_CBUS_DRV_SEL              (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x41 MHL CBUS 2nd Ctl Register                          (Default: 0x03)    */
#define REG_PAGE_3_MHL_CBUS_CTL1                           TX_PAGE_3, 0x41

#define BIT_PAGE_3_MHL_CBUS_CTL1_1115_OHM                  (0x04)

#define MSK_PAGE_3_MHL_CBUS_CTL1_CBUS_RGND_RES_CTL         (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0x42 MHL CoC 1st Ctl Register                           (Default: 0xC3)    */
#define REG_PAGE_3_MHL_COC_CTL0                            TX_PAGE_3, 0x42

#define BIT_PAGE_3_MHL_COC_CTL0_COC_BIAS_EN                (0x01 << 7)
#define MSK_PAGE_3_MHL_COC_CTL0_COC_BIAS_CTL               (0x07 << 4)
#define MSK_PAGE_3_MHL_COC_CTL0_COC_TERM_CTL               (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0x43 MHL CoC 2nd Ctl Register                           (Default: 0x87)    */
#define REG_PAGE_3_MHL_COC_CTL1                            TX_PAGE_3, 0x43

#define BIT_PAGE_3_MHL_COC_CTL1_COC_EN                     (0x01 << 7)
#define MSK_PAGE_3_MHL_COC_CTL1_COC_DRV_CTL                (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0x46 MHL CoC 5th Ctl Register                           (Default: 0x28)    */
#define REG_PAGE_3_MHL_COC_CTL4                            TX_PAGE_3, 0x46

#define MSK_PAGE_3_MHL_COC_CTL4_COC_IF_CTL                 (0x0F << 4)
#define MSK_PAGE_3_MHL_COC_CTL4_COC_SLEW_CTL               (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x47 MHL CoC 6th Ctl Register                           (Default: 0x0D)    */
#define REG_PAGE_3_MHL_COC_CTL5                            TX_PAGE_3, 0x47

#define MSK_PAGE_3_MHL_COC_CTL5_COC_RSV_7_0                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x48 MHL CoC 7th Ctl Register                           (Default: 0x80)    */
#define REG_PAGE_3_MHL_COC_CTL6                            TX_PAGE_3, 0x48

#define MSK_PAGE_3_MHL_COC_CTL6_COC_RSV_15_8               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x49 MHL DoC 1st Ctl Register                           (Default: 0x00)    */
#define REG_PAGE_3_MHL_DOC_CTL0                            TX_PAGE_3, 0x49

#define BIT_PAGE_3_MHL_DOC_CTL0_DOC_RXDATA_EN              (0x01 << 7)
#define BIT_PAGE_3_MHL_DOC_CTL0_DOC_CK_EN                  (0x01 << 6)
#define MSK_PAGE_3_MHL_DOC_CTL0_DOC_OPMODE                 (0x03 << 1)
#define BIT_PAGE_3_MHL_DOC_CTL0_DOC_RXBIAS_EN              (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x4A MHL DoC 2nd Ctl Register                           (Default: 0x03)    */
#define REG_PAGE_3_MHL_DOC_CTL1                            TX_PAGE_3, 0x4A

#define MSK_PAGE_3_MHL_DOC_CTL1_DOC_BIAS                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x4D MHL DoC 5th Ctl Register                           (Default: 0x42)    */
#define REG_PAGE_3_MHL_DOC_CTL4                            TX_PAGE_3, 0x4D

#define MSK_PAGE_3_MHL_DOC_CTL4_DOC_RSV_7_0                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x4E MHL DoC 5th Ctl Register                           (Default: 0x00)    */
#define REG_PAGE_3_MHL_DOC_CTL5                            TX_PAGE_3, 0x4E

#define MSK_PAGE_3_MHL_DOC_CTL5_DOC_IF                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x4F MHL Oscillator 1st Ctl Register                    (Default: 0x00)    */
#define REG_PAGE_3_MHL_OSC_CTL0                            TX_PAGE_3, 0x4F

#define MSK_PAGE_3_MHL_OSC_CTL0_OSC_CTL                    (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x60 Tx Zone Ctl0 Register                              (Default: 0x08)    */
#define REG_PAGE_3_TX_ZONE_CTL0                            TX_PAGE_3, 0x60

#define MSK_PAGE_3_TX_ZONE_CTL0_MAX_DIFF_LIMIT             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x61 Tx Zone Ctl1 Register                              (Default: 0x00)    */
#define REG_PAGE_3_TX_ZONE_CTL1                            TX_PAGE_3, 0x61

#define MSK_PAGE_3_TX_ZONE_CTL1_TX_ZONE_CTRL_MODE          (0x08 << 0)
#define MSK_PAGE_3_TX_ZONE_CTL1_TX_ZONE_CTRL               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x62 Tx Zone Ctl2 Register                              (Default: 0x00)    */
#define REG_PAGE_3_TX_ZONE_CTL2                            TX_PAGE_3, 0x62

#define MSK_PAGE_3_TX_ZONE_CTL2_TX_ZONE                    (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x63 Tx Zone Ctl3 Register                              (Default: 0x00)    */
#define REG_PAGE_3_TX_ZONE_CTL3                            TX_PAGE_3, 0x63

#define MSK_PAGE_3_TX_ZONE_CTL3_TX_ZONE_CTRL3              (0x03 << 6)
#define MSK_PAGE_3_TX_ZONE_CTL3_HRV_ZONE_CTRL3             (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0x64 MHL3 Tx Zone Ctl Register                          (Default: 0x00)    */
#define REG_PAGE_3_MHL3_TX_ZONE_CTL                        TX_PAGE_3, 0x64

#define BIT_PAGE_3_MHL3_TX_ZONE_CTL_MHL2_INTPLT_ZONE_MANU_EN (0x01 << 7)
#define MSK_PAGE_3_MHL3_TX_ZONE_CTL_MHL3_TX_ZONE           (0x03 << 0)

#define VAL_PAGE_3_TX_ZONE_CTL3_TX_ZONE_6GBPS              (0x00 << 0)
#define VAL_PAGE_3_TX_ZONE_CTL3_TX_ZONE_3GBPS              (0x01 << 0)
#define VAL_PAGE_3_TX_ZONE_CTL3_TX_ZONE_1_5GBPS            (0x02 << 0)
/*----------------------------------------------------------------------------*/
/* 0x70 HDCP Temp Register                                 (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TEMP0                            TX_PAGE_3, 0x70

#define MSK_PAGE_3_HDCP2X_TEMP0_HDCP2X_TEMP0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x71 HDCP Temp Register                                 (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TEMP1                            TX_PAGE_3, 0x71

#define MSK_PAGE_3_HDCP2X_TEMP1_HDCP2X_TEMP1               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x72 HDCP Temp Register                                 (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TEMP2                            TX_PAGE_3, 0x72

#define MSK_PAGE_3_HDCP2X_TEMP2_HDCP2X_TEMP2               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x73 HDCP Temp Register                                 (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TEMP3                            TX_PAGE_3, 0x73

#define MSK_PAGE_3_HDCP2X_TEMP3_HDCP2X_TEMP3               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x74 HDCP Temp Register                                 (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TEMP4                            TX_PAGE_3, 0x74

#define MSK_PAGE_3_HDCP2X_TEMP4_HDCP2X_TEMP4               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x75 HDCP Temp Register                                 (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TEMP5                            TX_PAGE_3, 0x75

#define MSK_PAGE_3_HDCP2X_TEMP5_HDCP2X_TEMP5               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x76 HDCP Temp Register                                 (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TEMP6                            TX_PAGE_3, 0x76

#define MSK_PAGE_3_HDCP2X_TEMP6_HDCP2X_TEMP6               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x77 HDCP Temp Register                                 (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TEMP7                            TX_PAGE_3, 0x77

#define MSK_PAGE_3_HDCP2X_TEMP7_HDCP2X_TEMP7               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x78 HDCP Temp Register                                 (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TEMP8                            TX_PAGE_3, 0x78

#define MSK_PAGE_3_HDCP2X_TEMP8_HDCP2X_TEMP8               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x79 HDCP Temp Register                                 (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TEMP9                            TX_PAGE_3, 0x79

#define MSK_PAGE_3_HDCP2X_TEMP9_HDCP2X_TEMP9               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x7A HDCP Temp Register                                 (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TEMP10                           TX_PAGE_3, 0x7A

#define MSK_PAGE_3_HDCP2X_TEMP10_HDCP2X_TEMP10             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x7B HDCP Temp Register                                 (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TEMP11                           TX_PAGE_3, 0x7B

#define MSK_PAGE_3_HDCP2X_TEMP11_HDCP2X_TEMP11             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x7C HDCP Temp Register                                 (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TEMP12                           TX_PAGE_3, 0x7C

#define MSK_PAGE_3_HDCP2X_TEMP12_HDCP2X_TEMP12             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x7D HDCP Temp Register                                 (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TEMP13                           TX_PAGE_3, 0x7D

#define MSK_PAGE_3_HDCP2X_TEMP13_HDCP2X_TEMP13             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x7E HDCP Temp Register                                 (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TEMP14                           TX_PAGE_3, 0x7E

#define MSK_PAGE_3_HDCP2X_TEMP14_HDCP2X_TEMP14             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x7F HDCP Temp Register                                 (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TEMP15                           TX_PAGE_3, 0x7F

#define MSK_PAGE_3_HDCP2X_TEMP15_HDCP2X_TEMP15             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x80 HDCP Temp Register                                 (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TEMP16                           TX_PAGE_3, 0x80

#define MSK_PAGE_3_HDCP2X_TEMP16_HDCP2X_TEMP16             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x81 HDCP Temp Register                                 (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TEMP17                           TX_PAGE_3, 0x81

#define MSK_PAGE_3_HDCP2X_TEMP17_HDCP2X_TEMP17             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x82 HDCP Temp Register                                 (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TEMP18                           TX_PAGE_3, 0x82

#define MSK_PAGE_3_HDCP2X_TEMP18_HDCP2X_TEMP18             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x83 HDCP Temp Register                                 (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TEMP19                           TX_PAGE_3, 0x83

#define MSK_PAGE_3_HDCP2X_TEMP19_HDCP2X_TEMP19             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x90 HDCP Software Reset Register                       (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TX_SRST                          TX_PAGE_3, 0x90

#define BIT_PAGE_3_HDCP2X_TX_SRST_HDCP2X_DDCM_RST          (0x01 << 4)
#define BIT_PAGE_3_HDCP2X_TX_SRST_HDCP2X_ERST              (0x01 << 3)
#define BIT_PAGE_3_HDCP2X_TX_SRST_HDCP2X_PRST_AUTO         (0x01 << 2)
#define BIT_PAGE_3_HDCP2X_TX_SRST_HDCP2X_PRST              (0x01 << 1)
#define BIT_PAGE_3_HDCP2X_TX_SRST_HDCP2X_CRST              (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x91 HDCP Polling Control and Status Register           (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_POLL_CS                          TX_PAGE_3, 0x91

#define BIT_PAGE_3_HDCP2X_POLL_CS_HDCP2X_DIS_POLL_GNT      (0x01 << 1)
#define BIT_PAGE_3_HDCP2X_POLL_CS_HDCP2X_DIS_POLL_EN       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x92 HDCP SPI Starting Address Lo Register              (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_SPI_START_ADDR_LO                TX_PAGE_3, 0x92

#define MSK_PAGE_3_HDCP2X_SPI_START_ADDR_LO_HDCP2X_SPI_S_ADDR_7_0 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x93 HDCP SPI Starting Address Hi Register              (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_SPI_START_ADDR_HI                TX_PAGE_3, 0x93

#define MSK_PAGE_3_HDCP2X_SPI_START_ADDR_HI_HDCP2X_SPI_S_ADDR_15_8 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x94 HDCP SPI Signature Starting Address Lo Register    (Default: 0x80)    */
#define REG_PAGE_3_HDCP2X_SPI_SIGN_START_ADDR_LO           TX_PAGE_3, 0x94

#define MSK_PAGE_3_HDCP2X_SPI_SIGN_START_ADDR_LO_HDCP2X_SPI_SI_S_ADDR_7_0 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x95 HDCP SPI Signature Starting Address Hi Register    (Default: 0x3F)    */
#define REG_PAGE_3_HDCP2X_SPI_SIGN_START_ADDR_HI           TX_PAGE_3, 0x95

#define MSK_PAGE_3_HDCP2X_SPI_SIGN_START_ADDR_HI_HDCP2X_SPI_SI_S_ADDR_15_8 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x96 HDCP SPI Signature Ending Address Lo Register      (Default: 0xFF)    */
#define REG_PAGE_3_HDCP2X_SPI_SIGN_END_ADDR_LO             TX_PAGE_3, 0x96

#define MSK_PAGE_3_HDCP2X_SPI_SIGN_END_ADDR_LO_HDCP2X_SPI_SI_E_ADDR_7_0 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x97 HDCP SPI Signature Ending Address Hi Register      (Default: 0x3F)    */
#define REG_PAGE_3_HDCP2X_SPI_SIGN_END_ADDR_HI             TX_PAGE_3, 0x97

#define MSK_PAGE_3_HDCP2X_SPI_SIGN_END_ADDR_HI_HDCP2X_SPI_SI_E_ADDR_15_8 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x98 HDCP Interrupt 0 Register                          (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_INTR0                            TX_PAGE_3, 0x98

#define BIT_PAGE_3_HDCP2X_INTR0_HDCP2X_INTR0_STAT7         (0x01 << 7)
#define BIT_PAGE_3_HDCP2X_INTR0_HDCP2X_INTR0_STAT6         (0x01 << 6)
#define BIT_PAGE_3_HDCP2X_INTR0_HDCP2X_INTR0_STAT5         (0x01 << 5)
#define BIT_PAGE_3_HDCP2X_INTR0_HDCP2X_INTR0_STAT4         (0x01 << 4)
#define BIT_PAGE_3_HDCP2X_INTR0_HDCP2X_INTR0_STAT3         (0x01 << 3)
#define BIT_PAGE_3_HDCP2X_INTR0_HDCP2X_INTR0_STAT2         (0x01 << 2)
#define BIT_PAGE_3_HDCP2X_INTR0_HDCP2X_INTR0_STAT1         (0x01 << 1)
#define BIT_PAGE_3_HDCP2X_INTR0_HDCP2X_INTR0_STAT0         (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x99 HDCP Interrupt 0 Mask Register                     (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_INTR0_MASK                       TX_PAGE_3, 0x99

#define BIT_PAGE_3_HDCP2X_INTR0_MASK_HDCP2X_INTR0_MASK7    (0x01 << 7)
#define BIT_PAGE_3_HDCP2X_INTR0_MASK_HDCP2X_INTR0_MASK6    (0x01 << 6)
#define BIT_PAGE_3_HDCP2X_INTR0_MASK_HDCP2X_INTR0_MASK5    (0x01 << 5)
#define BIT_PAGE_3_HDCP2X_INTR0_MASK_HDCP2X_INTR0_MASK4    (0x01 << 4)
#define BIT_PAGE_3_HDCP2X_INTR0_MASK_HDCP2X_INTR0_MASK3    (0x01 << 3)
#define BIT_PAGE_3_HDCP2X_INTR0_MASK_HDCP2X_INTR0_MASK2    (0x01 << 2)
#define BIT_PAGE_3_HDCP2X_INTR0_MASK_HDCP2X_INTR0_MASK1    (0x01 << 1)
#define BIT_PAGE_3_HDCP2X_INTR0_MASK_HDCP2X_INTR0_MASK0    (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xA0 HDCP General Control 0 Register                    (Default: 0x02)    */
#define REG_PAGE_3_HDCP2X_CTRL_0                           TX_PAGE_3, 0xA0

#define BIT_PAGE_3_HDCP2X_CTRL_0_HDCP2X_ENCRYPT_EN         (0x01 << 7)
#define BIT_PAGE_3_HDCP2X_CTRL_0_HDCP2X_POLINT_SEL         (0x01 << 6)
#define BIT_PAGE_3_HDCP2X_CTRL_0_HDCP2X_POLINT_OVR         (0x01 << 5)
#define BIT_PAGE_3_HDCP2X_CTRL_0_HDCP2X_PRECOMPUTE         (0x01 << 4)
#define BIT_PAGE_3_HDCP2X_CTRL_0_HDCP2X_HDMIMODE           (0x01 << 3)
#define BIT_PAGE_3_HDCP2X_CTRL_0_HDCP2X_REPEATER           (0x01 << 2)
#define BIT_PAGE_3_HDCP2X_CTRL_0_HDCP2X_HDCPTX             (0x01 << 1)
#define BIT_PAGE_3_HDCP2X_CTRL_0_HDCP2X_EN                 (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xA1 HDCP General Control 1 Register                    (Default: 0x04)    */
#define REG_PAGE_3_HDCP2X_CTRL_1                           TX_PAGE_3, 0xA1

#define MSK_PAGE_3_HDCP2X_CTRL_1_HDCP2X_REAUTH_MSK_3_0     (0x0F << 4)
#define BIT_PAGE_3_HDCP2X_CTRL_1_HDCP2X_HPD_SW             (0x01 << 2)
#define BIT_PAGE_3_HDCP2X_CTRL_1_HDCP2X_HPD_OVR            (0x01 << 1)
#define BIT_PAGE_3_HDCP2X_CTRL_1_HDCP2X_REAUTH_SW          (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xA2 HDCP General Control 1 Register                    (Default: 0xA0)    */
#define REG_PAGE_3_HDCP2X_CTRL_2                           TX_PAGE_3, 0xA2

#define MSK_PAGE_3_HDCP2X_CTRL_2_HDCP2X_CPVER_3_0          (0x0F << 4)
#define BIT_PAGE_3_HDCP2X_CTRL_2_HDCP2X_SPI_ADDR_RESET     (0x01 << 1)
#define BIT_PAGE_3_HDCP2X_CTRL_2_HDCP2X_CUPD_START         (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xA3 HDCP CUPD Size Lo Register                         (Default: 0x80)    */
#define REG_PAGE_3_HDCP2X_CUPD_SIZE_LO                     TX_PAGE_3, 0xA3

#define MSK_PAGE_3_HDCP2X_CUPD_SIZE_LO_HDCP2X_CUPD_SIZE_7_0 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA4 HDCP CUPD Size Hi Register                         (Default: 0x3F)    */
#define REG_PAGE_3_HDCP2X_CUPD_SIZE_HI                     TX_PAGE_3, 0xA4

#define MSK_PAGE_3_HDCP2X_CUPD_SIZE_HI_HDCP2X_CUPD_SIZE_15_8 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA5 HDCP Misc Control Register                         (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_MISC_CTRL                        TX_PAGE_3, 0xA5

#define BIT_PAGE_3_HDCP2X_MISC_CTRL_HDCP2X_RPT_SMNG_XFER_START (0x01 << 4)
#define BIT_PAGE_3_HDCP2X_MISC_CTRL_HDCP2X_RPT_SMNG_WR_START (0x01 << 3)
#define BIT_PAGE_3_HDCP2X_MISC_CTRL_HDCP2X_RPT_SMNG_WR     (0x01 << 2)
#define BIT_PAGE_3_HDCP2X_MISC_CTRL_HDCP2X_RPT_RCVID_RD_START (0x01 << 1)
#define BIT_PAGE_3_HDCP2X_MISC_CTRL_HDCP2X_RPT_RCVID_RD    (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xA6 HDCP RPT SMNG K Register                           (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_RPT_SMNG_K                       TX_PAGE_3, 0xA6

#define MSK_PAGE_3_HDCP2X_RPT_SMNG_K_HDCP2X_RPT_SMNG_K_7_0 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA7 HDCP RPT SMNG In Register                          (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_RPT_SMNG_IN                      TX_PAGE_3, 0xA7

#define MSK_PAGE_3_HDCP2X_RPT_SMNG_IN_HDCP2X_RPT_SMNG_IN   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA8 HDCP General Staus Register                        (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_GEN_STATUS                       TX_PAGE_3, 0xA8

#define BIT_PAGE_3_HDCP2X_GEN_STATUS_HDCP2X_RPT_REPEATER   (0x01 << 2)
#define BIT_PAGE_3_HDCP2X_GEN_STATUS_HDCP2X_PRG_SEL        (0x01 << 1)
#define BIT_PAGE_3_HDCP2X_GEN_STATUS_HDCP2X_CUPD_DONE      (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xA9 HDCP Misc Staus Register                           (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_MISC_STATUS                      TX_PAGE_3, 0xA9

#define BIT_PAGE_3_HDCP2X_MISC_STATUS_HDCP2X_RPT_HDCP1DEV_DSTRM (0x01 << 5)
#define BIT_PAGE_3_HDCP2X_MISC_STATUS_HDCP2X_RPT_HDCP20RPT_DSTRM (0x01 << 4)
#define BIT_PAGE_3_HDCP2X_MISC_STATUS_HDCP2X_RPT_MX_CASC_EXC (0x01 << 3)
#define BIT_PAGE_3_HDCP2X_MISC_STATUS_HDCP2X_RPT_MX_DEVS_EXC (0x01 << 2)
#define BIT_PAGE_3_HDCP2X_MISC_STATUS_HDCP2X_RPT_SMNG_XFER_DONE (0x01 << 1)
#define BIT_PAGE_3_HDCP2X_MISC_STATUS_HDCP2X_RPT_RCVID_CHANGED (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xAA HDCP Auth Status Register                          (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_AUTH_STAT                        TX_PAGE_3, 0xAA

#define MSK_PAGE_3_HDCP2X_AUTH_STAT_HDCP2X_AUTH_STAT_7_0   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xAB HDCP State Status Register                         (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_STATE                            TX_PAGE_3, 0xAB

#define MSK_PAGE_3_HDCP2X_STATE_HDCP2X_STATE_7_0           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xAC HDCP RPT RCVID Out Register                        (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_RPT_RCVID_OUT                    TX_PAGE_3, 0xAC

#define MSK_PAGE_3_HDCP2X_RPT_RCVID_OUT_HDCP2X_RPT_RCVID_OUT_7_0 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xAD HDCP Depth Status Register                         (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_RPT_DEPTH                        TX_PAGE_3, 0xAD

#define MSK_PAGE_3_HDCP2X_RPT_DEPTH_HDCP2X_RPT_DEPTH_7_0   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xAE HDCP Devcnt Status Register                        (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_RPT_DEVCNT                       TX_PAGE_3, 0xAE

#define MSK_PAGE_3_HDCP2X_RPT_DEVCNT_HDCP2X_RPT_DEVCNT_7_0 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xAF HDCP Stream Counter Byte 0 Register                (Default: 0x02)    */
#define REG_PAGE_3_HDCP2X_STM_CTR0                         TX_PAGE_3, 0xAF

#define MSK_PAGE_3_HDCP2X_STM_CTR0_HDCP2X_STM_CTR_7_0      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB0 HDCP Stream Counter Byte 1 Register                (Default: 0x02)    */
#define REG_PAGE_3_HDCP2X_STM_CTR1                         TX_PAGE_3, 0xB0

#define MSK_PAGE_3_HDCP2X_STM_CTR1_HDCP2X_STM_CTR_15_8     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB1 HDCP Stream Counter Byte 2 Register                (Default: 0x02)    */
#define REG_PAGE_3_HDCP2X_STM_CTR2                         TX_PAGE_3, 0xB1

#define MSK_PAGE_3_HDCP2X_STM_CTR2_HDCP2X_STM_CTR_23_16    (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB2 HDCP Stream Counter Byte 3 Register                (Default: 0x02)    */
#define REG_PAGE_3_HDCP2X_STM_CTR3                         TX_PAGE_3, 0xB2

#define MSK_PAGE_3_HDCP2X_STM_CTR3_HDCP2X_STM_CTR_31_24    (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB3 HDCP TP0  Register                                 (Default: 0x02)    */
#define REG_PAGE_3_HDCP2X_TP0                              TX_PAGE_3, 0xB3

#define MSK_PAGE_3_HDCP2X_TP0_HDCP2X_TP0_7_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB4 HDCP TP1  Register                                 (Default: 0x75)    */
#define REG_PAGE_3_HDCP2X_TP1                              TX_PAGE_3, 0xB4

#define MSK_PAGE_3_HDCP2X_TP1_HDCP2X_TP1_7_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB5 HDCP TP2 Register                                  (Default: 0x01)    */
#define REG_PAGE_3_HDCP2X_TP2                              TX_PAGE_3, 0xB5

#define MSK_PAGE_3_HDCP2X_TP2_HDCP2X_TP2_7_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB6 HDCP TP3  Register                                 (Default: 0x32)    */
#define REG_PAGE_3_HDCP2X_TP3                              TX_PAGE_3, 0xB6

#define MSK_PAGE_3_HDCP2X_TP3_HDCP2X_TP3_7_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB7 HDCP TP4  Register                                 (Default: 0x14)    */
#define REG_PAGE_3_HDCP2X_TP4                              TX_PAGE_3, 0xB7

#define MSK_PAGE_3_HDCP2X_TP4_HDCP2X_TP4_7_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB8 HDCP TP5  Register                                 (Default: 0x64)    */
#define REG_PAGE_3_HDCP2X_TP5                              TX_PAGE_3, 0xB8

#define MSK_PAGE_3_HDCP2X_TP5_HDCP2X_TP5_7_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB9 HDCP TP6 Register                                  (Default: 0x02)    */
#define REG_PAGE_3_HDCP2X_TP6                              TX_PAGE_3, 0xB9

#define MSK_PAGE_3_HDCP2X_TP6_HDCP2X_TP6_7_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBA HDCP TP7  Register                                 (Default: 0x0A)    */
#define REG_PAGE_3_HDCP2X_TP7                              TX_PAGE_3, 0xBA

#define MSK_PAGE_3_HDCP2X_TP7_HDCP2X_TP7_7_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBB HDCP TP8  Register                                 (Default: 0x0A)    */
#define REG_PAGE_3_HDCP2X_TP8                              TX_PAGE_3, 0xBB

#define MSK_PAGE_3_HDCP2X_TP8_HDCP2X_TP8_7_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBC HDCP TP9  Register                                 (Default: 0x14)    */
#define REG_PAGE_3_HDCP2X_TP9                              TX_PAGE_3, 0xBC

#define MSK_PAGE_3_HDCP2X_TP9_HDCP2X_TP9_7_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBD HDCP TP10 Register                                 (Default: 0x14)    */
#define REG_PAGE_3_HDCP2X_TP10                             TX_PAGE_3, 0xBD

#define MSK_PAGE_3_HDCP2X_TP10_HDCP2X_TP10_7_0             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBE HDCP TP11  Register                                (Default: 0xC8)    */
#define REG_PAGE_3_HDCP2X_TP11                             TX_PAGE_3, 0xBE

#define MSK_PAGE_3_HDCP2X_TP11_HDCP2X_TP11_7_0             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBF HDCP TP12  Register                                (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TP12                             TX_PAGE_3, 0xBF

#define MSK_PAGE_3_HDCP2X_TP12_HDCP2X_TP12_7_0             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC0 HDCP TP13  Register                                (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TP13                             TX_PAGE_3, 0xC0

#define MSK_PAGE_3_HDCP2X_TP13_HDCP2X_TP13_7_0             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC1 HDCP TP14 Register                                 (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TP14                             TX_PAGE_3, 0xC1

#define MSK_PAGE_3_HDCP2X_TP14_HDCP2X_TP14_7_0             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC2 HDCP TP15  Register                                (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_TP15                             TX_PAGE_3, 0xC2

#define MSK_PAGE_3_HDCP2X_TP15_HDCP2X_TP15_7_0             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC3 HDCP GP In 0 Register                              (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_GP_IN0                           TX_PAGE_3, 0xC3

#define MSK_PAGE_3_HDCP2X_GP_IN0_HDCP2X_GP_IN0_7_0         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC4 HDCP GP In 1 Register                              (Default: 0x22)    */
#define REG_PAGE_3_HDCP2X_GP_IN1                           TX_PAGE_3, 0xC4

#define MSK_PAGE_3_HDCP2X_GP_IN1_HDCP2X_GP_IN1_7_0         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC5 HDCP GP In 2 Register                              (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_GP_IN2                           TX_PAGE_3, 0xC5

#define MSK_PAGE_3_HDCP2X_GP_IN2_HDCP2X_GP_IN2_7_0         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC6 HDCP GP In 3 Register                              (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_GP_IN3                           TX_PAGE_3, 0xC6

#define MSK_PAGE_3_HDCP2X_GP_IN3_HDCP2X_GP_IN3_7_0         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC7 HDCP GP Out 0 Register                             (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_GP_OUT0                          TX_PAGE_3, 0xC7

#define MSK_PAGE_3_HDCP2X_GP_OUT0_HDCP2X_GP_OUT0_7_0       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC8 HDCP GP Out 1 Register                             (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_GP_OUT1                          TX_PAGE_3, 0xC8

#define MSK_PAGE_3_HDCP2X_GP_OUT1_HDCP2X_GP_OUT1_7_0       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC9 HDCP GP Out 2 Register                             (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_GP_OUT2                          TX_PAGE_3, 0xC9

#define MSK_PAGE_3_HDCP2X_GP_OUT2_HDCP2X_GP_OUT2_7_0       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCA HDCP GP Out 3 Register                             (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_GP_OUT3                          TX_PAGE_3, 0xCA

#define MSK_PAGE_3_HDCP2X_GP_OUT3_HDCP2X_GP_OUT3_7_0       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCB HDCP Seq V Byte 0 Register                         (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_RPT_SEQ_NUM_V_0                  TX_PAGE_3, 0xCB

#define MSK_PAGE_3_HDCP2X_RPT_SEQ_NUM_V_0_HDCP2X_RPT_SEQ_NUM_V_7_0 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCC HDCP GP Out 1 Register                             (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_RPT_SEQ_NUM_V_1                  TX_PAGE_3, 0xCC

#define MSK_PAGE_3_HDCP2X_RPT_SEQ_NUM_V_1_HDCP2X_RPT_SEQ_NUM_V_15_8 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCD HDCP GP Out 2 Register                             (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_RPT_SEQ_NUM_V_2                  TX_PAGE_3, 0xCD

#define MSK_PAGE_3_HDCP2X_RPT_SEQ_NUM_V_2_HDCP2X_RPT_SEQ_NUM_V_23_16 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCE HDCP RPT Seq M Byte 0 Register                     (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_RPT_SEQ_NUM_M_0                  TX_PAGE_3, 0xCE

#define MSK_PAGE_3_HDCP2X_RPT_SEQ_NUM_M_0_HDCP2X_RPT_SEQ_NUM_M_7_0 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCF HDCP RPT Seq M Byte 1 Register                     (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_RPT_SEQ_NUM_M_1                  TX_PAGE_3, 0xCF

#define MSK_PAGE_3_HDCP2X_RPT_SEQ_NUM_M_1_HDCP2X_RPT_SEQ_NUM_M_15_8 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD0 HDCP RPT Seq M Byte 0 Register                     (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_RPT_SEQ_NUM_M_2                  TX_PAGE_3, 0xD0

#define MSK_PAGE_3_HDCP2X_RPT_SEQ_NUM_M_2_HDCP2X_RPT_SEQ_NUM_M_23_16 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD1 HDCP Repeater RCVR ID 0 Register                   (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_RPT_RCVR_ID0                     TX_PAGE_3, 0xD1

#define MSK_PAGE_3_HDCP2X_RPT_RCVR_ID0_HDCP2X_RCVR_ID_7_0  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD2 HDCP Repeater RCVR ID 1 Register                   (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_RPT_RCVR_ID1                     TX_PAGE_3, 0xD2

#define MSK_PAGE_3_HDCP2X_RPT_RCVR_ID1_HDCP2X_RCVR_ID_15_8 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD3 HDCP Repeater RCVR ID 2 Register                   (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_RPT_RCVR_ID2                     TX_PAGE_3, 0xD3

#define MSK_PAGE_3_HDCP2X_RPT_RCVR_ID2_HDCP2X_RCVR_ID_23_16 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD4 HDCP Repeater RCVR ID 3 Register                   (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_RPT_RCVR_ID3                     TX_PAGE_3, 0xD4

#define MSK_PAGE_3_HDCP2X_RPT_RCVR_ID3_HDCP2X_RCVR_ID_31_24 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD5 HDCP Repeater RCVR ID 4 Register                   (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_RPT_RCVR_ID4                     TX_PAGE_3, 0xD5

#define MSK_PAGE_3_HDCP2X_RPT_RCVR_ID4_HDCP2X_RCVR_ID_39_32 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD6 HDCP Poll Interval 0 Register                      (Default: 0x0A)    */
#define REG_PAGE_3_HDCP2X_POLL_VAL0                        TX_PAGE_3, 0xD6

#define MSK_PAGE_3_HDCP2X_POLL_VAL0_HDCP2X_POL_VAL0_7_0    (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD7 HDCP Poll Interval 1 Register                      (Default: 0x32)    */
#define REG_PAGE_3_HDCP2X_POLL_VAL1                        TX_PAGE_3, 0xD7

#define MSK_PAGE_3_HDCP2X_POLL_VAL1_HDCP2X_POL_VAL1_7_0    (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD8 HDCP DDCM Status Register                          (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_DDCM_STS                         TX_PAGE_3, 0xD8

#define MSK_PAGE_3_HDCP2X_DDCM_STS_HDCP2X_DDCM_ERR_STS_3_0 (0x0F << 4)
#define MSK_PAGE_3_HDCP2X_DDCM_STS_HDCP2X_DDCM_CTL_CS_3_0  (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xD9 HDCP Ring OSC Bist Register                        (Default: 0x00)    */
#define REG_PAGE_3_HDCP2X_ROSC_BIST                        TX_PAGE_3, 0xD9

#define BIT_PAGE_3_HDCP2X_ROSC_BIST_HDCP2X_RINGOSC_BIST_FAIL (0x01 << 2)
#define BIT_PAGE_3_HDCP2X_ROSC_BIST_HDCP2X_RINGOSC_BIST_DONE (0x01 << 1)
#define BIT_PAGE_3_HDCP2X_ROSC_BIST_HDCP2X_RINGOSC_BIST_START (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xE0 HDMI2MHL3 Control Register                         (Default: 0x0A)    */
#define REG_PAGE_3_M3_CTRL                                 TX_PAGE_3, 0xE0

#define BIT_PAGE_3_M3_CTRL_EXTMHL3_EN                      (0x01 << 5)
#define BIT_PAGE_3_M3_CTRL_H2M_SWRST                       (0x01 << 4)
#define BIT_PAGE_3_M3_CTRL_SW_MHL3_SEL                     (0x01 << 3)
#define BIT_PAGE_3_M3_CTRL_M3AV_EN                         (0x01 << 2)
#define BIT_PAGE_3_M3_CTRL_ENC_TMDS                        (0x01 << 1)
#define BIT_PAGE_3_M3_CTRL_MHL3_MASTER_EN                  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xE1 HDMI2MHL3 Port0 Control Register                   (Default: 0x04)    */
#define REG_PAGE_3_M3_P0CTRL                               TX_PAGE_3, 0xE1

#define BIT_PAGE_3_M3_P0CTRL_MHL3_P0_HDCP_ENC_EN           (0x01 << 4)
#define BIT_PAGE_3_M3_P0CTRL_MHL3_P0_UNLIMIT_EN            (0x01 << 3)
#define BIT_PAGE_3_M3_P0CTRL_MHL3_P0_HDCP_EN               (0x01 << 2)

#define BIT_PAGE_3_M3_P0CTRL_MHL3_P0_PIXEL_MODE            (0x01 << 1)
#define VAL_PAGE_3_M3_P0CTRL_MHL3_P0_PIXEL_MODE_NORMAL     (0x00)
#define VAL_PAGE_3_M3_P0CTRL_MHL3_P0_PIXEL_MODE_PACKED     (0x02)

#define BIT_PAGE_3_M3_P0CTRL_MHL3_P0_PORT_EN               (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xE3 HDMI2MHL3 Lane Register                            (Default: 0x00)    */
#define REG_PAGE_3_M3_NLN                                  TX_PAGE_3, 0xE3

#define MSK_PAGE_3_M3_NLN_MHL3_NUM_LANE                    (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0xE4 HDMI2MHL3 Payload Low Register                     (Default: 0xFC)    */
#define REG_PAGE_3_M3_PLDL                                 TX_PAGE_3, 0xE4

#define MSK_PAGE_3_M3_PLDL_MHL3_MAX_PAYLOAD_7_0            (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE5 HDMI2MHL3 Payload High Register                    (Default: 0x00)    */
#define REG_PAGE_3_M3_PLDH                                 TX_PAGE_3, 0xE5

#define MSK_PAGE_3_M3_PLDH_MHL3_MAX_PAYLOAD_9_8            (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0xE6 HDMI2MHL3 Scramble Control Register                (Default: 0x41)    */
#define REG_PAGE_3_M3_SCTRL                                TX_PAGE_3, 0xE6

#define MSK_PAGE_3_M3_SCTRL_MHL3_SR_LENGTH                 (0x0F << 4)
#define BIT_PAGE_3_M3_SCTRL_MHL3_SCRAMBLER_EN              (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xE7 HDMI2MHL3 SR Period Low Register                   (Default: 0x96)    */
#define REG_PAGE_3_M3_SRPL                                 TX_PAGE_3, 0xE7

#define MSK_PAGE_3_M3_SRPL_MHL3_SR_PERIOD_7_0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE8 HDMI2MHL3 SR Period High Register                  (Default: 0x98)    */
#define REG_PAGE_3_M3_SRPH                                 TX_PAGE_3, 0xE8

#define MSK_PAGE_3_M3_SRPH_MHL3_SR_PERIOD_15_8             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xE9 HDMI2MHL3 Type #1 Register                         (Default: 0x21)    */
#define REG_PAGE_3_M3_TYPE1                                TX_PAGE_3, 0xE9

#define MSK_PAGE_3_M3_TYPE1_MHL3_TYPE_HDCP_PKT             (0x0F << 4)
#define MSK_PAGE_3_M3_TYPE1_MHL3_TYPE_HDMI_PKT             (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xEA HDMI2MHL3 Type #2 Register                         (Default: 0x03)    */
#define REG_PAGE_3_M3_TYPE2                                TX_PAGE_3, 0xEA

#define MSK_PAGE_3_M3_TYPE2_MHL3_TYPE_HDMI2_PKT            (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xEB HDMI2MHL3 K_SR Register                            (Default: 0x00)    */
#define REG_PAGE_3_M3_KSR                                  TX_PAGE_3, 0xEB

#define MSK_PAGE_3_M3_KSR_MHL3_K_SR                        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xEC HDMI2MHL3 K_SOP Register                           (Default: 0x01)    */
#define REG_PAGE_3_M3_KSOP                                 TX_PAGE_3, 0xEC

#define MSK_PAGE_3_M3_KSOP_MHL3_K_SOP                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xED HDMI2MHL3 K_EOP Register                           (Default: 0x02)    */
#define REG_PAGE_3_M3_KEOP                                 TX_PAGE_3, 0xED

#define MSK_PAGE_3_M3_KEOP_MHL3_K_EOP                      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xF0 HISC PD Control Register                           (Default: 0x1B)    */
#define REG_PAGE_3_HSIC_PD_CTL                             TX_PAGE_3, 0xF0

#define BIT_PAGE_3_HSIC_PD_CTL_PDN_LDO                     (0x01 << 5)
#define BIT_PAGE_3_HSIC_PD_CTL_EN_RX                       (0x01 << 4)
#define BIT_PAGE_3_HSIC_PD_CTL_PDB_SKEW                    (0x01 << 3)
#define BIT_PAGE_3_HSIC_PD_CTL_SCPCAL_MAIN                 (0x01 << 2)
#define BIT_PAGE_3_HSIC_PD_CTL_ENBALE_IGEN_MAIN            (0x01 << 1)
#define BIT_PAGE_3_HSIC_PD_CTL_PDB_MAIN                    (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF1 HSIC VCOCAL Register                               (Default: 0x00)    */
#define REG_PAGE_3_HSIC_VCOCAL                             TX_PAGE_3, 0xF1

#define MSK_PAGE_3_HSIC_VCOCAL_VCO_BIAS_MAIN               (0x07 << 4)
#define MSK_PAGE_3_HSIC_VCOCAL_VOCAL_MAIN                  (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xF2 HSIC Div Ctl Register                              (Default: 0x05)    */
#define REG_PAGE_3_DIV_CTL_MAIN                            TX_PAGE_3, 0xF2

#define MSK_PAGE_3_DIV_CTL_MAIN_PRE_DIV_CTL_MAIN           (0x07 << 2)
#define MSK_PAGE_3_DIV_CTL_MAIN_FB_DIV_CTL_MAIN            (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF3 HSIC ICP Ctl Register                              (Default: 0x03)    */
#define REG_PAGE_3_ICP_CTL_MAIN                            TX_PAGE_3, 0xF3

#define MSK_PAGE_3_ICP_CTL_MAIN_ICP_CTL_MAIN               (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0xF4 HSIC Bias Bgr Register                             (Default: 0x37)    */
#define REG_PAGE_3_BIAS_BGR                                TX_PAGE_3, 0xF4

#define MSK_PAGE_3_BIAS_BGR_VCOCAL_DEF_MAIN                (0x07 << 4)
#define MSK_PAGE_3_BIAS_BGR_BIAS_BGR_D                     (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xF5 HSIC SPLL Bias Register                            (Default: 0x00)    */
#define REG_PAGE_3_SPLL_BIAS                               TX_PAGE_3, 0xF5

#define MSK_PAGE_3_SPLL_BIAS_LBW_MAIN                      (0x03 << 4)
#define MSK_PAGE_3_SPLL_BIAS_SPLLBIAS_MAIN                 (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xF6 HSIC VCOCAL Skew Register                          (Default: 0x00)    */
#define REG_PAGE_3_HSIC_VCOCAL_SKEW                        TX_PAGE_3, 0xF6

#define MSK_PAGE_3_HSIC_VCOCAL_SKEW_VCO_CTL_SKEW           (0x0F << 4)
#define MSK_PAGE_3_HSIC_VCOCAL_SKEW_VOCAL_SKEW             (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xF7 HSIC Div Ctl Skew Register                         (Default: 0x00)    */
#define REG_PAGE_3_DIV_CTL_SKEW                            TX_PAGE_3, 0xF7

#define MSK_PAGE_3_DIV_CTL_SKEW_PRE_DIV_CTL_SKEW           (0x03 << 2)
#define MSK_PAGE_3_DIV_CTL_SKEW_FB_DIV_CTL_SKEW            (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF8 HSIC ICP Ctl Skew Register                         (Default: 0x04)    */
#define REG_PAGE_3_ICP_CTL_SKEW                            TX_PAGE_3, 0xF8

#define MSK_PAGE_3_ICP_CTL_SKEW_ICP_CTL_SKEW               (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0xF9 HSIC Config Register                               (Default: 0x00)    */
#define REG_PAGE_3_HSIC_CONFIG                             TX_PAGE_3, 0xF9

#define MSK_PAGE_3_HSIC_CONFIG_CONFIG_HSIC                 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xFA HSIC LDO CTL Register                              (Default: 0x2B)    */
#define REG_PAGE_3_LDO_CTL                                 TX_PAGE_3, 0xFA

#define MSK_PAGE_3_LDO_CTL_SEL_VREF                        (0x03 << 4)
#define BIT_PAGE_3_LDO_CTL_SEL_OPAMP_BIASI                 (0x01 << 3)
#define BIT_PAGE_3_LDO_CTL_SEL_INT_COMP                    (0x01 << 2)
#define MSK_PAGE_3_LDO_CTL_BLEED_CURRENT                   (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0xFB HSIC Slew CTL Register                             (Default: 0x40)    */
#define REG_PAGE_3_SLEW_CTL                                TX_PAGE_3, 0xFB

#define MSK_PAGE_3_SLEW_CTL_SEL_SLICE                      (0x07 << 4)
#define MSK_PAGE_3_SLEW_CTL_SLEW_CTL                       (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xFC HSIC Clkdetect Register                            (Default: 0x00)    */
#define REG_PAGE_3_CLKDETECT                               TX_PAGE_3, 0xFC

#define BIT_PAGE_3_CLKDETECT_CLKDETECT_MAIN                (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* Registers in TX_PAGE_4                                                     */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* 0x00 MHL Capability 1st Byte Register                   (Default: 0x00)    */
#define REG_PAGE_4_MHL_DEVCAP_0                            TX_PAGE_4, 0x00

#define MSK_PAGE_4_MHL_DEVCAP_0_MHL_DEVCAP_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x01 MHL Capability 2nd Byte Register                   (Default: 0x00)    */
#define REG_PAGE_4_MHL_DEVCAP_1                            TX_PAGE_4, 0x01

#define MSK_PAGE_4_MHL_DEVCAP_1_MHL_DEVCAP_1               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x02 MHL Capability 3rd Byte Register                   (Default: 0x00)    */
#define REG_PAGE_4_MHL_DEVCAP_2                            TX_PAGE_4, 0x02

#define MSK_PAGE_4_MHL_DEVCAP_2_MHL_DEVCAP_2               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x03 MHL Capability 4th Byte Register                   (Default: 0x00)    */
#define REG_PAGE_4_MHL_DEVCAP_3                            TX_PAGE_4, 0x03

#define MSK_PAGE_4_MHL_DEVCAP_3_MHL_DEVCAP_3               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x04 MHL Capability 5th Byte Register                   (Default: 0x00)    */
#define REG_PAGE_4_MHL_DEVCAP_4                            TX_PAGE_4, 0x04

#define MSK_PAGE_4_MHL_DEVCAP_4_MHL_DEVCAP_4               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x05 MHL Capability 6th Byte Register                   (Default: 0x00)    */
#define REG_PAGE_4_MHL_DEVCAP_5                            TX_PAGE_4, 0x05

#define MSK_PAGE_4_MHL_DEVCAP_5_MHL_DEVCAP_5               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x06 MHL Capability 7th Byte Register                   (Default: 0x00)    */
#define REG_PAGE_4_MHL_DEVCAP_6                            TX_PAGE_4, 0x06

#define MSK_PAGE_4_MHL_DEVCAP_6_MHL_DEVCAP_6               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x07 MHL Capability 8th Byte Register                   (Default: 0x00)    */
#define REG_PAGE_4_MHL_DEVCAP_7                            TX_PAGE_4, 0x07

#define MSK_PAGE_4_MHL_DEVCAP_7_MHL_DEVCAP_7               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x08 MHL Capability 9th Byte Register                   (Default: 0x00)    */
#define REG_PAGE_4_MHL_DEVCAP_8                            TX_PAGE_4, 0x08

#define MSK_PAGE_4_MHL_DEVCAP_8_MHL_DEVCAP_8               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x09 MHL Capability 10th Byte Register                  (Default: 0x00)    */
#define REG_PAGE_4_MHL_DEVCAP_9                            TX_PAGE_4, 0x09

#define MSK_PAGE_4_MHL_DEVCAP_9_MHL_DEVCAP_9               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0A MHL Capability 11th Byte Register                  (Default: 0x00)    */
#define REG_PAGE_4_MHL_DEVCAP_A                            TX_PAGE_4, 0x0A

#define MSK_PAGE_4_MHL_DEVCAP_A_MHL_DEVCAP_A               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0B MHL Capability 12th Byte Register                  (Default: 0x00)    */
#define REG_PAGE_4_MHL_DEVCAP_B                            TX_PAGE_4, 0x0B

#define MSK_PAGE_4_MHL_DEVCAP_B_MHL_DEVCAP_B               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0C MHL Capability 13th Byte Register                  (Default: 0x00)    */
#define REG_PAGE_4_MHL_DEVCAP_C                            TX_PAGE_4, 0x0C

#define MSK_PAGE_4_MHL_DEVCAP_C_MHL_DEVCAP_C               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0D MHL Capability 14th Byte Register                  (Default: 0x00)    */
#define REG_PAGE_4_MHL_DEVCAP_D                            TX_PAGE_4, 0x0D

#define MSK_PAGE_4_MHL_DEVCAP_D_MHL_DEVCAP_D               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0E MHL Capability 15th Byte Register                  (Default: 0x00)    */
#define REG_PAGE_4_MHL_DEVCAP_E                            TX_PAGE_4, 0x0E

#define MSK_PAGE_4_MHL_DEVCAP_E_MHL_DEVCAP_E               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0F MHL Capability 16th Byte Register                  (Default: 0x00)    */
#define REG_PAGE_4_MHL_DEVCAP_F                            TX_PAGE_4, 0x0F

#define MSK_PAGE_4_MHL_DEVCAP_F_MHL_DEVCAP_F               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x20 MHL Interrupt 1st Byte Register                    (Default: 0x00)    */
#define REG_PAGE_4_MHL_INT_0                               TX_PAGE_4, 0x20

#define BIT_PAGE_4_MHL_INT_0_MHL_INT_0_STAT7               (0x01 << 7)
#define BIT_PAGE_4_MHL_INT_0_MHL_INT_0_STAT6               (0x01 << 6)
#define BIT_PAGE_4_MHL_INT_0_MHL_INT_0_STAT5               (0x01 << 5)
#define BIT_PAGE_4_MHL_INT_0_MHL_INT_0_STAT4               (0x01 << 4)
#define BIT_PAGE_4_MHL_INT_0_MHL_INT_0_STAT3               (0x01 << 3)
#define BIT_PAGE_4_MHL_INT_0_MHL_INT_0_STAT2               (0x01 << 2)
#define BIT_PAGE_4_MHL_INT_0_MHL_INT_0_STAT1               (0x01 << 1)
#define BIT_PAGE_4_MHL_INT_0_MHL_INT_0_STAT0               (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x21 MHL Interrupt 2nd Byte Register                    (Default: 0x00)    */
#define REG_PAGE_4_MHL_INT_1                               TX_PAGE_4, 0x21

#define BIT_PAGE_4_MHL_INT_1_MHL_INT_1_STAT7               (0x01 << 7)
#define BIT_PAGE_4_MHL_INT_1_MHL_INT_1_STAT6               (0x01 << 6)
#define BIT_PAGE_4_MHL_INT_1_MHL_INT_1_STAT5               (0x01 << 5)
#define BIT_PAGE_4_MHL_INT_1_MHL_INT_1_STAT4               (0x01 << 4)
#define BIT_PAGE_4_MHL_INT_1_MHL_INT_1_STAT3               (0x01 << 3)
#define BIT_PAGE_4_MHL_INT_1_MHL_INT_1_STAT2               (0x01 << 2)
#define BIT_PAGE_4_MHL_INT_1_MHL_INT_1_STAT1               (0x01 << 1)
#define BIT_PAGE_4_MHL_INT_1_MHL_INT_1_STAT0               (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x22 MHL Interrupt 3rd Byte Register                    (Default: 0x00)    */
#define REG_PAGE_4_MHL_INT_2                               TX_PAGE_4, 0x22

#define BIT_PAGE_4_MHL_INT_2_MHL_INT_2_STAT7               (0x01 << 7)
#define BIT_PAGE_4_MHL_INT_2_MHL_INT_2_STAT6               (0x01 << 6)
#define BIT_PAGE_4_MHL_INT_2_MHL_INT_2_STAT5               (0x01 << 5)
#define BIT_PAGE_4_MHL_INT_2_MHL_INT_2_STAT4               (0x01 << 4)
#define BIT_PAGE_4_MHL_INT_2_MHL_INT_2_STAT3               (0x01 << 3)
#define BIT_PAGE_4_MHL_INT_2_MHL_INT_2_STAT2               (0x01 << 2)
#define BIT_PAGE_4_MHL_INT_2_MHL_INT_2_STAT1               (0x01 << 1)
#define BIT_PAGE_4_MHL_INT_2_MHL_INT_2_STAT0               (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x23 MHL Interrupt 4th Byte Register                    (Default: 0x00)    */
#define REG_PAGE_4_MHL_INT_3                               TX_PAGE_4, 0x23

#define BIT_PAGE_4_MHL_INT_3_MHL_INT_3_STAT7               (0x01 << 7)
#define BIT_PAGE_4_MHL_INT_3_MHL_INT_3_STAT6               (0x01 << 6)
#define BIT_PAGE_4_MHL_INT_3_MHL_INT_3_STAT5               (0x01 << 5)
#define BIT_PAGE_4_MHL_INT_3_MHL_INT_3_STAT4               (0x01 << 4)
#define BIT_PAGE_4_MHL_INT_3_MHL_INT_3_STAT3               (0x01 << 3)
#define BIT_PAGE_4_MHL_INT_3_MHL_INT_3_STAT2               (0x01 << 2)
#define BIT_PAGE_4_MHL_INT_3_MHL_INT_3_STAT1               (0x01 << 1)
#define BIT_PAGE_4_MHL_INT_3_MHL_INT_3_STAT0               (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x30 Device Status 1st byte Register                    (Default: 0x00)    */
#define REG_PAGE_4_MHL_STAT_0                              TX_PAGE_4, 0x30

#define MSK_PAGE_4_MHL_STAT_0_MHL_STAT_0                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x31 Device Status 2nd byte Register                    (Default: 0x03)    */
#define REG_PAGE_4_MHL_STAT_1                              TX_PAGE_4, 0x31

#define MSK_PAGE_4_MHL_STAT_1_MHL_STAT_1                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x32 Device Status 3rd byte Register                    (Default: 0x00)    */
#define REG_PAGE_4_MHL_STAT_2                              TX_PAGE_4, 0x32

#define MSK_PAGE_4_MHL_STAT_2_MHL_STAT_2                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x33 Device Status 4th byte Register                    (Default: 0x00)    */
#define REG_PAGE_4_MHL_STAT_3                              TX_PAGE_4, 0x33

#define MSK_PAGE_4_MHL_STAT_3_MHL_STAT_3                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x40 CBUS Scratch Pad 1st Byte Register                 (Default: 0x00)    */
#define REG_PAGE_4_MHL_SCRPAD_0                            TX_PAGE_4, 0x40

#define MSK_PAGE_4_MHL_SCRPAD_0_MHL_SCRPAD_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x41 CBUS Scratch Pad 2nd Byte Register                 (Default: 0x00)    */
#define REG_PAGE_4_MHL_SCRPAD_1                            TX_PAGE_4, 0x41

#define MSK_PAGE_4_MHL_SCRPAD_1_MHL_SCRPAD_1               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x42 CBUS Scratch Pad 3rd Byte Register                 (Default: 0x00)    */
#define REG_PAGE_4_MHL_SCRPAD_2                            TX_PAGE_4, 0x42

#define MSK_PAGE_4_MHL_SCRPAD_2_MHL_SCRPAD_2               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x43 CBUS Scratch Pad 4th Byte Register                 (Default: 0x00)    */
#define REG_PAGE_4_MHL_SCRPAD_3                            TX_PAGE_4, 0x43

#define MSK_PAGE_4_MHL_SCRPAD_3_MHL_SCRPAD_3               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x44 CBUS Scratch Pad 5th Byte Register                 (Default: 0x00)    */
#define REG_PAGE_4_MHL_SCRPAD_4                            TX_PAGE_4, 0x44

#define MSK_PAGE_4_MHL_SCRPAD_4_MHL_SCRPAD_4               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x45 CBUS Scratch Pad 6th Byte Register                 (Default: 0x00)    */
#define REG_PAGE_4_MHL_SCRPAD_5                            TX_PAGE_4, 0x45

#define MSK_PAGE_4_MHL_SCRPAD_5_MHL_SCRPAD_5               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x46 CBUS Scratch Pad 7th Byte Register                 (Default: 0x00)    */
#define REG_PAGE_4_MHL_SCRPAD_6                            TX_PAGE_4, 0x46

#define MSK_PAGE_4_MHL_SCRPAD_6_MHL_SCRPAD_6               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x47 CBUS Scratch Pad 8th Byte Register                 (Default: 0x00)    */
#define REG_PAGE_4_MHL_SCRPAD_7                            TX_PAGE_4, 0x47

#define MSK_PAGE_4_MHL_SCRPAD_7_MHL_SCRPAD_7               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x48 CBUS Scratch Pad 9th Byte Register                 (Default: 0x00)    */
#define REG_PAGE_4_MHL_SCRPAD_8                            TX_PAGE_4, 0x48

#define MSK_PAGE_4_MHL_SCRPAD_8_MHL_SCRPAD_8               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x49 CBUS Scratch Pad 10th Byte Register                (Default: 0x00)    */
#define REG_PAGE_4_MHL_SCRPAD_9                            TX_PAGE_4, 0x49

#define MSK_PAGE_4_MHL_SCRPAD_9_MHL_SCRPAD_9               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x4A CBUS Scratch Pad 11th Byte Register                (Default: 0x00)    */
#define REG_PAGE_4_MHL_SCRPAD_A                            TX_PAGE_4, 0x4A

#define MSK_PAGE_4_MHL_SCRPAD_A_MHL_SCRPAD_A               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x4B CBUS Scratch Pad 12th Byte Register                (Default: 0x00)    */
#define REG_PAGE_4_MHL_SCRPAD_B                            TX_PAGE_4, 0x4B

#define MSK_PAGE_4_MHL_SCRPAD_B_MHL_SCRPAD_B               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x4C CBUS Scratch Pad 13th Byte Register                (Default: 0x00)    */
#define REG_PAGE_4_MHL_SCRPAD_C                            TX_PAGE_4, 0x4C

#define MSK_PAGE_4_MHL_SCRPAD_C_MHL_SCRPAD_C               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x4D CBUS Scratch Pad 14th Byte Register                (Default: 0x00)    */
#define REG_PAGE_4_MHL_SCRPAD_D                            TX_PAGE_4, 0x4D

#define MSK_PAGE_4_MHL_SCRPAD_D_MHL_SCRPAD_D               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x4E CBUS Scratch Pad 15th Byte Register                (Default: 0x00)    */
#define REG_PAGE_4_MHL_SCRPAD_E                            TX_PAGE_4, 0x4E

#define MSK_PAGE_4_MHL_SCRPAD_E_MHL_SCRPAD_E               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x4F CBUS Scratch Pad 16th Byte Register                (Default: 0x00)    */
#define REG_PAGE_4_MHL_SCRPAD_F                            TX_PAGE_4, 0x4F

#define MSK_PAGE_4_MHL_SCRPAD_F_MHL_SCRPAD_F               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x80 MHL Extended Capability 1st Byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTDEVCAP_0                         TX_PAGE_4, 0x80

#define MSK_PAGE_4_MHL_EXTDEVCAP_0_MHL_EXTDEVCAP_0         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x81 MHL Extended Capability 2nd Byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTDEVCAP_1                         TX_PAGE_4, 0x81

#define MSK_PAGE_4_MHL_EXTDEVCAP_1_MHL_EXTDEVCAP_1         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x82 MHL Extended Capability 3rd Byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTDEVCAP_2                         TX_PAGE_4, 0x82

#define MSK_PAGE_4_MHL_EXTDEVCAP_2_MHL_EXTDEVCAP_2         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x83 MHL Extended Capability 4th Byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTDEVCAP_3                         TX_PAGE_4, 0x83

#define MSK_PAGE_4_MHL_EXTDEVCAP_3_MHL_EXTDEVCAP_3         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x84 MHL Extended Capability 5th Byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTDEVCAP_4                         TX_PAGE_4, 0x84

#define MSK_PAGE_4_MHL_EXTDEVCAP_4_MHL_EXTDEVCAP_4         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x85 MHL Extended Capability 6th Byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTDEVCAP_5                         TX_PAGE_4, 0x85

#define MSK_PAGE_4_MHL_EXTDEVCAP_5_MHL_EXTDEVCAP_5         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x86 MHL Extended Capability 7th Byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTDEVCAP_6                         TX_PAGE_4, 0x86

#define MSK_PAGE_4_MHL_EXTDEVCAP_6_MHL_EXTDEVCAP_6         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x87 MHL Extended Capability 8th Byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTDEVCAP_7                         TX_PAGE_4, 0x87

#define MSK_PAGE_4_MHL_EXTDEVCAP_7_MHL_EXTDEVCAP_7         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x88 MHL Extended Capability 9th Byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTDEVCAP_8                         TX_PAGE_4, 0x88

#define MSK_PAGE_4_MHL_EXTDEVCAP_8_MHL_EXTDEVCAP_8         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x89 MHL Extended Capability 10th Byte Register         (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTDEVCAP_9                         TX_PAGE_4, 0x89

#define MSK_PAGE_4_MHL_EXTDEVCAP_9_MHL_EXTDEVCAP_9         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x8A MHL Extended Capability 11th Byte Register         (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTDEVCAP_A                         TX_PAGE_4, 0x8A

#define MSK_PAGE_4_MHL_EXTDEVCAP_A_MHL_EXTDEVCAP_A         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x8B MHL Extended Capability 12th Byte Register         (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTDEVCAP_B                         TX_PAGE_4, 0x8B

#define MSK_PAGE_4_MHL_EXTDEVCAP_B_MHL_EXTDEVCAP_B         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x8C MHL Extended Capability 13th Byte Register         (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTDEVCAP_C                         TX_PAGE_4, 0x8C

#define MSK_PAGE_4_MHL_EXTDEVCAP_C_MHL_EXTDEVCAP_C         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x8D MHL Extended Capability 14th Byte Register         (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTDEVCAP_D                         TX_PAGE_4, 0x8D

#define MSK_PAGE_4_MHL_EXTDEVCAP_D_MHL_EXTDEVCAP_D         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x8E MHL Extended Capability 15th Byte Register         (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTDEVCAP_E                         TX_PAGE_4, 0x8E

#define MSK_PAGE_4_MHL_EXTDEVCAP_E_MHL_EXTDEVCAP_E         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x8F MHL Extended Capability 16th Byte Register         (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTDEVCAP_F                         TX_PAGE_4, 0x8F

#define MSK_PAGE_4_MHL_EXTDEVCAP_F_MHL_EXTDEVCAP_F         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x90 Device Extended Status 1st byte Register           (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_0                           TX_PAGE_4, 0x90

#define MSK_PAGE_4_MHL_EXTSTAT_0_MHL_EXTSTAT_0             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x91 Device Extended Status 2nd byte Register           (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_1                           TX_PAGE_4, 0x91

#define MSK_PAGE_4_MHL_EXTSTAT_1_MHL_EXTSTAT_1             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x92 Device Extended Status 3rd byte Register           (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_2                           TX_PAGE_4, 0x92

#define MSK_PAGE_4_MHL_EXTSTAT_2_MHL_EXTSTAT_2             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x93 Device Extended Status 4th byte Register           (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_3                           TX_PAGE_4, 0x93

#define MSK_PAGE_4_MHL_EXTSTAT_3_MHL_EXTSTAT_3             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x94 Device Extended Status 5th byte Register           (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_4                           TX_PAGE_4, 0x94

#define MSK_PAGE_4_MHL_EXTSTAT_4_MHL_EXTSTAT_4             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x95 Device Extended Status 6th byte Register           (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_5                           TX_PAGE_4, 0x95

#define MSK_PAGE_4_MHL_EXTSTAT_5_MHL_EXTSTAT_5             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x96 Device Extended Status 7th byte Register           (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_6                           TX_PAGE_4, 0x96

#define MSK_PAGE_4_MHL_EXTSTAT_6_MHL_EXTSTAT_6             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x97 Device Extended Status 8th byte Register           (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_7                           TX_PAGE_4, 0x97

#define MSK_PAGE_4_MHL_EXTSTAT_7_MHL_EXTSTAT_7             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x98 Device Extended Status 9th byte Register           (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_8                           TX_PAGE_4, 0x98

#define MSK_PAGE_4_MHL_EXTSTAT_8_MHL_EXTSTAT_8             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x99 Device Extended Status 10th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_9                           TX_PAGE_4, 0x99

#define MSK_PAGE_4_MHL_EXTSTAT_9_MHL_EXTSTAT_9             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x9A Device Extended Status 11th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_A                           TX_PAGE_4, 0x9A

#define MSK_PAGE_4_MHL_EXTSTAT_A_MHL_EXTSTAT_A             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x9B Device Extended Status 12th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_B                           TX_PAGE_4, 0x9B

#define MSK_PAGE_4_MHL_EXTSTAT_B_MHL_EXTSTAT_B             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x9C Device Extended Status 13th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_C                           TX_PAGE_4, 0x9C

#define MSK_PAGE_4_MHL_EXTSTAT_C_MHL_EXTSTAT_C             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x9D Device Extended Status 14th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_D                           TX_PAGE_4, 0x9D

#define MSK_PAGE_4_MHL_EXTSTAT_D_MHL_EXTSTAT_D             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x9E Device Extended Status 15th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_E                           TX_PAGE_4, 0x9E

#define MSK_PAGE_4_MHL_EXTSTAT_E_MHL_EXTSTAT_E             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x9F Device Extended Status 16th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_F                           TX_PAGE_4, 0x9F

#define MSK_PAGE_4_MHL_EXTSTAT_F_MHL_EXTSTAT_F             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA0 Device Extended Status 17th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_10                          TX_PAGE_4, 0xA0

#define MSK_PAGE_4_MHL_EXTSTAT_10_MHL_EXTSTAT_10           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA1 Device Extended Status 18th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_11                          TX_PAGE_4, 0xA1

#define MSK_PAGE_4_MHL_EXTSTAT_11_MHL_EXTSTAT_11           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA2 Device Extended Status 19th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_12                          TX_PAGE_4, 0xA2

#define MSK_PAGE_4_MHL_EXTSTAT_12_MHL_EXTSTAT_12           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA3 Device Extended Status 20th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_13                          TX_PAGE_4, 0xA3

#define MSK_PAGE_4_MHL_EXTSTAT_13_MHL_EXTSTAT_13           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA4 Device Extended Status 21th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_14                          TX_PAGE_4, 0xA4

#define MSK_PAGE_4_MHL_EXTSTAT_14_MHL_EXTSTAT_14           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA5 Device Extended Status 22th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_15                          TX_PAGE_4, 0xA5

#define MSK_PAGE_4_MHL_EXTSTAT_15_MHL_EXTSTAT_15           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA6 Device Extended Status 23th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_16                          TX_PAGE_4, 0xA6

#define MSK_PAGE_4_MHL_EXTSTAT_16_MHL_EXTSTAT_16           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA7 Device Extended Status 24th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_17                          TX_PAGE_4, 0xA7

#define MSK_PAGE_4_MHL_EXTSTAT_17_MHL_EXTSTAT_17           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA8 Device Extended Status 25th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_18                          TX_PAGE_4, 0xA8

#define MSK_PAGE_4_MHL_EXTSTAT_18_MHL_EXTSTAT_18           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA9 Device Extended Status 26th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_19                          TX_PAGE_4, 0xA9

#define MSK_PAGE_4_MHL_EXTSTAT_19_MHL_EXTSTAT_19           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xAA Device Extended Status 27th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_1A                          TX_PAGE_4, 0xAA

#define MSK_PAGE_4_MHL_EXTSTAT_1A_MHL_EXTSTAT_1A           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xAB Device Extended Status 28th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_1B                          TX_PAGE_4, 0xAB

#define MSK_PAGE_4_MHL_EXTSTAT_1B_MHL_EXTSTAT_1B           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xAC Device Extended Status 29th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_1C                          TX_PAGE_4, 0xAC

#define MSK_PAGE_4_MHL_EXTSTAT_1C_MHL_EXTSTAT_1C           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xAD Device Extended Status 30th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_1D                          TX_PAGE_4, 0xAD

#define MSK_PAGE_4_MHL_EXTSTAT_1D_MHL_EXTSTAT_1D           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xAE Device Extended Status 31th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_1E                          TX_PAGE_4, 0xAE

#define MSK_PAGE_4_MHL_EXTSTAT_1E_MHL_EXTSTAT_1E           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xAF Device Extended Status 32th byte Register          (Default: 0x00)    */
#define REG_PAGE_4_MHL_EXTSTAT_1F                          TX_PAGE_4, 0xAF

#define MSK_PAGE_4_MHL_EXTSTAT_1F_MHL_EXTSTAT_1F           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* Registers in TX_PAGE_6                                                     */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* 0x02 TPI DTD Byte2 Register                             (Default: 0x00)    */
#define REG_PAGE_6_TPI_DTD_B2                              TX_PAGE_6, 0x02

#define MSK_PAGE_6_TPI_DTD_B2_DTDB2                        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x07 TPI DTD Byte7 Register                             (Default: 0x00)    */
#define REG_PAGE_6_TPI_MISC                                TX_PAGE_6, 0x07

#define BIT_PAGE_6_TPI_MISC_ONLY_BKSV_DONE_SEL             (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x08 TPI Pixel Repetition Data Register                 (Default: 0x40)    */
#define REG_PAGE_6_TPI_PRD                                 TX_PAGE_6, 0x08

#define MSK_PAGE_6_TPI_PRD_TCLK_SEL                        (0x03 << 6)
#define BIT_PAGE_6_TPI_PRD_EDGE                            (0x01 << 4)
#define MSK_PAGE_6_TPI_PRD_ICLK                            (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x09 Input Format Register                              (Default: 0x00)    */
#define REG_PAGE_6_TPI_INPUT                               TX_PAGE_6, 0x09

#define BIT_PAGE_6_TPI_INPUT_EXTENDEDBITMODE               (0x01 << 7)
#define BIT_PAGE_6_TPI_INPUT_ENDITHER                      (0x01 << 6)
#define MSK_PAGE_6_TPI_INPUT_INPUT_QUAN_RANGE              (0x03 << 2)
#define MSK_PAGE_6_TPI_INPUT_INPUT_FORMAT                  (0x03 << 0)
#define VAL_PAGE_6_INPUT_FORMAT_RGB                        (0x00 << 0)
#define VAL_PAGE_6_INPUT_FORMAT_YCBCR444                   (0x01 << 0)
#define VAL_PAGE_6_INPUT_FORMAT_YCBCR422                   (0x02 << 0)
#define VAL_PAGE_6_INPUT_FORMAT_INTERNAL_RGB               (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x0A Output Format Register                             (Default: 0x00)    */
#define REG_PAGE_6_TPI_OUTPUT                              TX_PAGE_6, 0x0A

#define BIT_PAGE_6_TPI_OUTPUT_CSCMODE709                   (0x01 << 4)
#define MSK_PAGE_6_TPI_OUTPUT_OUTPUT_QUAN_RANGE            (0x03 << 2)
#define MSK_PAGE_6_TPI_OUTPUT_OUTPUT_FORMAT                (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x0C TPI AVI Check Sum Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_AVI_CHSUM                           TX_PAGE_6, 0x0C

#define MSK_PAGE_6_TPI_AVI_CHSUM_AVI_CHECKSUM              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0D TPI AVI Data Byte 1 Register                       (Default: 0x00)    */
#define REG_PAGE_6_TPI_AVI_BYTE1                           TX_PAGE_6, 0x0D

#define MSK_PAGE_6_TPI_AVI_BYTE1_AVI_DBYTE1                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0E TPI AVI Data Byte 2 Register                       (Default: 0x00)    */
#define REG_PAGE_6_TPI_AVI_BYTE2                           TX_PAGE_6, 0x0E

#define MSK_PAGE_6_TPI_AVI_BYTE2_AVI_DBYTE2                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0F TPI AVI Data Byte 3 Register                       (Default: 0x00)    */
#define REG_PAGE_6_TPI_AVI_BYTE3                           TX_PAGE_6, 0x0F

#define MSK_PAGE_6_TPI_AVI_BYTE3_AVI_DBYTE3                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x10 TPI AVI Data Byte 4 Register                       (Default: 0x00)    */
#define REG_PAGE_6_TPI_AVI_BYTE4                           TX_PAGE_6, 0x10

#define MSK_PAGE_6_TPI_AVI_BYTE4_AVI_DBYTE4                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x11 TPI AVI Data Byte 5 Register                       (Default: 0x00)    */
#define REG_PAGE_6_TPI_AVI_BYTE5                           TX_PAGE_6, 0x11

#define MSK_PAGE_6_TPI_AVI_BYTE5_AVI_DBYTE5                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x12 TPI AVI Data Byte 6 Register                       (Default: 0x00)    */
#define REG_PAGE_6_TPI_AVI_BYTE6                           TX_PAGE_6, 0x12

#define MSK_PAGE_6_TPI_AVI_BYTE6_AVI_DBYTE6                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x13 TPI AVI Data Byte 7 Register                       (Default: 0x00)    */
#define REG_PAGE_6_TPI_AVI_BYTE7                           TX_PAGE_6, 0x13

#define MSK_PAGE_6_TPI_AVI_BYTE7_AVI_DBYTE7                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x14 TPI AVI Data Byte 8 Register                       (Default: 0x00)    */
#define REG_PAGE_6_TPI_AVI_BYTE8                           TX_PAGE_6, 0x14

#define MSK_PAGE_6_TPI_AVI_BYTE8_AVI_DBYTE8                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x15 TPI AVI Data Byte 9 Register                       (Default: 0x00)    */
#define REG_PAGE_6_TPI_AVI_BYTE9                           TX_PAGE_6, 0x15

#define MSK_PAGE_6_TPI_AVI_BYTE9_AVI_DBYTE9                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x16 TPI AVI Data Byte 10 Register                      (Default: 0x00)    */
#define REG_PAGE_6_TPI_AVI_BYTE10                          TX_PAGE_6, 0x16

#define MSK_PAGE_6_TPI_AVI_BYTE10_AVI_DBYTE10              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x17 TPI AVI Data Byte 11 Register                      (Default: 0x00)    */
#define REG_PAGE_6_TPI_AVI_BYTE11                          TX_PAGE_6, 0x17

#define MSK_PAGE_6_TPI_AVI_BYTE11_AVI_DBYTE11              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x18 TPI AVI Data Byte 12 Register                      (Default: 0x00)    */
#define REG_PAGE_6_TPI_AVI_BYTE12                          TX_PAGE_6, 0x18

#define MSK_PAGE_6_TPI_AVI_BYTE12_AVI_DBYTE12              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x19 TPI AVI Data Byte 13 Register                      (Default: 0x00)    */
#define REG_PAGE_6_TPI_AVI_BYTE13                          TX_PAGE_6, 0x19

#define MSK_PAGE_6_TPI_AVI_BYTE13_AVI_DBYTE13              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x1A TPI System Control Register                        (Default: 0x00)    */
#define REG_PAGE_6_TPI_SC                                  TX_PAGE_6, 0x1A

#define BIT_PAGE_6_TPI_SC_TPI_UPDATE_FLG                   (0x01 << 7)
#define BIT_PAGE_6_TPI_SC_TPI_REAUTH_CTL                   (0x01 << 6)
#define BIT_PAGE_6_TPI_SC_TPI_OUTPUT_MODE_1                (0x01 << 5)

#define BIT_PAGE_6_TPI_SC_REG_TMDS_OE                      (0x01 << 4)
#define VAL_PAGE_6_TPI_SC_REG_TMDS_OE_ACTIVE               (0x00 << 4)
#define VAL_PAGE_6_TPI_SC_REG_TMDS_OE_POWER_DOWN           (0x01 << 4)

#define BIT_PAGE_6_TPI_SC_TPI_AV_MUTE                      (0x01 << 3)
#define VAL_PAGE_6_TPI_SC_TPI_AV_MUTE_NORMAL               (0x00 << 3)
#define VAL_PAGE_6_TPI_SC_TPI_AV_MUTE_MUTED                (0x01 << 3)

#define BIT_PAGE_6_TPI_SC_DDC_GPU_REQUEST                  (0x01 << 2)
#define BIT_PAGE_6_TPI_SC_DDC_TPI_SW                       (0x01 << 1)

#define BIT_PAGE_6_TPI_SC_TPI_OUTPUT_MODE_0                (0x01 << 0)
#define VAL_PAGE_6_TPI_SC_TPI_OUTPUT_MODE_0_DVI            (0x00 << 0)
#define VAL_PAGE_6_TPI_SC_TPI_OUTPUT_MODE_0_HDMI           (0x01 << 0)


/*----------------------------------------------------------------------------*/
/* 0x1B TPI Device ID Register                             (Default: 0xB2)    */
#define REG_PAGE_6_TPI_DEV_ID                              TX_PAGE_6, 0x1B

#define MSK_PAGE_6_TPI_DEV_ID_TPI_DEV_ID                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x1C TPI Device Revision ID Register                    (Default: 0x00)    */
#define REG_PAGE_6_TPI_REV_ID                              TX_PAGE_6, 0x1C

#define MSK_PAGE_6_TPI_REV_ID_DEV_REV_ID                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x1D TPI Vendor Byte 3 Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_VENDOR_B3                           TX_PAGE_6, 0x1D

#define MSK_PAGE_6_TPI_VENDOR_B3_VENDOR_B3                 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x29 TPI COPP Query Data Register                       (Default: 0x00)    */
#define REG_PAGE_6_TPI_COPP_DATA1                          TX_PAGE_6, 0x29

#define BIT_PAGE_6_TPI_COPP_DATA1_COPP_GPROT               (0x01 << 7)
#define VAL_TPI_COPP_GPROT_NONE                            (0x00 << 7)
#define VAL_TPI_COPP_GPROT_SECURE                          (0x01 << 7)

#define BIT_PAGE_6_TPI_COPP_DATA1_COPP_LPROT               (0x01 << 6)
#define VAL_TPI_COPP_LPROT_NONE                     	   (0x00 << 6)
#define VAL_TPI_COPP_LPROT_SECURE                          (0x01 << 6)

#define MSK_PAGE_6_TPI_COPP_DATA1_COPP_LINK_STATUS         (0x03 << 4)
#define VAL_TPI_COPP_LINK_STATUS_NORMAL                    (0x00 << 4)
#define VAL_TPI_COPP_LINK_STATUS_LINK_LOST                 (0x01 << 4)
#define VAL_TPI_COPP_LINK_STATUS_RENEGOTIATION_REQ         (0x02 << 4)
#define VAL_TPI_COPP_LINK_STATUS_LINK_SUSPENDED            (0x03 << 4)

#define BIT_PAGE_6_TPI_COPP_DATA1_COPP_HDCP_REP            (0x01 << 3)
#define BIT_PAGE_6_TPI_COPP_DATA1_COPP_CONNTYPE_0          (0x01 << 2)
#define BIT_PAGE_6_TPI_COPP_DATA1_COPP_PROTYPE             (0x01 << 1)
#define BIT_PAGE_6_TPI_COPP_DATA1_COPP_CONNTYPE_1          (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x2A TPI COPP Control Data Register                     (Default: 0x00)    */
#define REG_PAGE_6_TPI_COPP_DATA2                          TX_PAGE_6, 0x2A

#define BIT_PAGE_6_TPI_COPP_DATA2_INTR_ENCRYPTION          (0x01 << 5)
#define BIT_PAGE_6_TPI_COPP_DATA2_KSV_FORWARD              (0x01 << 4)
#define BIT_PAGE_6_TPI_COPP_DATA2_INTERM_RI_CHECK_EN       (0x01 << 3)
#define BIT_PAGE_6_TPI_COPP_DATA2_DOUBLE_RI_CHECK          (0x01 << 2)
#define BIT_PAGE_6_TPI_COPP_DATA2_DDC_SHORT_RI_RD          (0x01 << 1)

#define BIT_PAGE_6_TPI_COPP_DATA2_COPP_PROTLEVEL           (0x01 << 0)
#define VAL_PAGE_6_TPI_COPP_PROTLEVEL_MIN                  (0x00 << 0)
#define VAL_PAGE_6_TPI_COPP_PROTLEVEL_MAX                  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x2B TPI Write BKSV1 Register                           (Default: 0x00)    */
#define REG_PAGE_6_TPI_WR_BKSV_1                           TX_PAGE_6, 0x2B

#define MSK_PAGE_6_TPI_WR_BKSV_1_BKSV0                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x2C TPI Write BKSV2 Register                           (Default: 0x00)    */
#define REG_PAGE_6_TPI_WR_BKSV_2                           TX_PAGE_6, 0x2C

#define MSK_PAGE_6_TPI_WR_BKSV_2_BKSV1                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x2D TPI_Write BKSV3 Register                           (Default: 0x00)    */
#define REG_PAGE_6_TPI_WR_BKSV_3                           TX_PAGE_6, 0x2D

#define MSK_PAGE_6_TPI_WR_BKSV_3_BKSV2                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x2E TPI Write BKSV4 Register                           (Default: 0x00)    */
#define REG_PAGE_6_TPI_WR_BKSV_4                           TX_PAGE_6, 0x2E

#define MSK_PAGE_6_TPI_WR_BKSV_4_BKSV3                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x2F TPI Write BKSV5 Register                           (Default: 0x00)    */
#define REG_PAGE_6_TPI_WR_BKSV_5                           TX_PAGE_6, 0x2F

#define MSK_PAGE_6_TPI_WR_BKSV_5_BKSV4                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x30 TPI HDCP Revision Register                         (Default: 0x12)    */
#define REG_PAGE_6_TPI_HDCP_REV                            TX_PAGE_6, 0x30

#define MSK_PAGE_6_TPI_HDCP_REV_TPI_HDCP_REV               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x31 TPI KSV and V' Value Data Register                 (Default: 0x00)    */
#define REG_PAGE_6_TPI_KSV_V                               TX_PAGE_6, 0x31

#define MSK_PAGE_6_TPI_KSV_V_AUTH_STATE                    (0x03 << 6)
#define BIT_PAGE_6_TPI_KSV_V_COPP_VSEL_RDY                 (0x01 << 3)
#define MSK_PAGE_6_TPI_KSV_V_TPI_V_SEL                     (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0x32 TPI V' Value Byte 0 Register                       (Default: 0x00)    */
#define REG_PAGE_6_TPI_VVALUE_B0                           TX_PAGE_6, 0x32

#define MSK_PAGE_6_TPI_VVALUE_B0_TPI_VP_HX_B0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x33 TPI V' Value Byte 1 Register                       (Default: 0x00)    */
#define REG_PAGE_6_TPI_VVALUE_B1                           TX_PAGE_6, 0x33

#define MSK_PAGE_6_TPI_VVALUE_B1_TPI_VP_HX_B1              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x34 TPI V' Value Byte 2 Register                       (Default: 0x00)    */
#define REG_PAGE_6_TPI_VVALUE_B2                           TX_PAGE_6, 0x34

#define MSK_PAGE_6_TPI_VVALUE_B2_TPI_VP_HX_B2              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x35 TPI V' Value Byte 3 Register                       (Default: 0x00)    */
#define REG_PAGE_6_TPI_VVALUE_B3                           TX_PAGE_6, 0x35

#define MSK_PAGE_6_TPI_VVALUE_B3_TPI_VP_HX_B3              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x36 TPI AKSV_1 Register                                (Default: 0x00)    */
#define REG_PAGE_6_TPI_AKSV_1                              TX_PAGE_6, 0x36

#define MSK_PAGE_6_TPI_AKSV_1_TPI_AKSV0                    (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x37 TPI AKSV_2 Register                                (Default: 0x00)    */
#define REG_PAGE_6_TPI_AKSV_2                              TX_PAGE_6, 0x37

#define MSK_PAGE_6_TPI_AKSV_2_TPI_AKSV1                    (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x38 TPI AKSV_3 Register                                (Default: 0x00)    */
#define REG_PAGE_6_TPI_AKSV_3                              TX_PAGE_6, 0x38

#define MSK_PAGE_6_TPI_AKSV_3_TPI_AKSV2                    (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x39 TPI AKSV_4 Register                                (Default: 0x00)    */
#define REG_PAGE_6_TPI_AKSV_4                              TX_PAGE_6, 0x39

#define MSK_PAGE_6_TPI_AKSV_4_TPI_AKSV3                    (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x3A TPI AKSV_5 Register                                (Default: 0x00)    */
#define REG_PAGE_6_TPI_AKSV_5                              TX_PAGE_6, 0x3A

#define MSK_PAGE_6_TPI_AKSV_5_TPI_AKSV4                    (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x3C TPI Interrupt Enable Register                      (Default: 0x00)    */
#define REG_PAGE_6_TPI_INTR_EN                             TX_PAGE_6, 0x3C

#define BIT_PAGE_6_TPI_INTR_EN_TPI_INTR_EN_7               (0x01 << 7)
#define BIT_PAGE_6_TPI_INTR_EN_TPI_INTR_EN_6               (0x01 << 6)
#define BIT_PAGE_6_TPI_INTR_EN_TPI_INTR_EN_5               (0x01 << 5)
#define BIT_PAGE_6_TPI_INTR_EN_TPI_INTR_EN_3               (0x01 << 3)
#define BIT_PAGE_6_TPI_INTR_EN_TPI_INTR_EN_2               (0x01 << 2)
#define BIT_PAGE_6_TPI_INTR_EN_TPI_INTR_EN_1               (0x01 << 1)
#define BIT_PAGE_6_TPI_INTR_EN_TPI_INTR_EN_0               (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x3D TPI Interrupt Status Low Byte Register             (Default: 0x00)    */
#define REG_PAGE_6_TPI_INTR_ST0                            TX_PAGE_6, 0x3D

#define BIT_PAGE_6_TPI_INTR_ST0_TPI_AUTH_CHNGE_STAT        (0x01 << 7)
#define BIT_PAGE_6_TPI_INTR_ST0_TPI_V_RDY_STAT             (0x01 << 6)
#define BIT_PAGE_6_TPI_INTR_ST0_TPI_COPP_CHNGE_STAT        (0x01 << 5)
#define BIT_PAGE_6_TPI_INTR_ST0_KSV_FIFO_FIRST_STAT        (0x01 << 3)
#define BIT_PAGE_6_TPI_INTR_ST0_READ_BKSV_BCAPS_DONE_STAT  (0x01 << 2)
#define BIT_PAGE_6_TPI_INTR_ST0_READ_BKSV_BCAPS_ERR_STAT   (0x01 << 1)
#define BIT_PAGE_6_TPI_INTR_ST0_READ_BKSV_ERR_STAT         (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x41 TPI KSV FIFO Fill Level Status Register            (Default: 0x00)    */
#define REG_PAGE_6_TPI_KSV_FIFO_STAT                       TX_PAGE_6, 0x41

#define BIT_PAGE_6_TPI_KSV_FIFO_STAT_KSV_FIFO_LAST         (0x01 << 7)
#define MSK_PAGE_6_TPI_KSV_FIFO_STAT_KSV_FIFO_BYTES        (0x1F << 0)

/*----------------------------------------------------------------------------*/
/* 0x42 TPI KSV FIFO Forward Port Register                 (Default: 0x00)    */
#define REG_PAGE_6_TPI_KSV_FIFO_FORW                       TX_PAGE_6, 0x42

#define MSK_PAGE_6_TPI_KSV_FIFO_FORW_KSV_FIFO_FORWARD      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x44 TPI DS BCAPS Status Register                       (Default: 0x00)    */
#define REG_PAGE_6_TPI_DS_BCAPS                            TX_PAGE_6, 0x44

#define MSK_PAGE_6_TPI_DS_BCAPS_DS_BCAPS                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x45 TPI BStatus1 Register                              (Default: 0x00)    */
#define REG_PAGE_6_TPI_BSTATUS1                            TX_PAGE_6, 0x45

#define BIT_PAGE_6_TPI_BSTATUS1_DS_DEV_EXCEED              (0x01 << 7)
#define MSK_PAGE_6_TPI_BSTATUS1_DS_DEV_CNT                 (0x7F << 0)

/*----------------------------------------------------------------------------*/
/* 0x46 TPI BStatus2 Register                              (Default: 0x10)    */
#define REG_PAGE_6_TPI_BSTATUS2                            TX_PAGE_6, 0x46

#define MSK_PAGE_6_TPI_BSTATUS2_DS_BSTATUS                 (0x07 << 5)
#define BIT_PAGE_6_TPI_BSTATUS2_DS_HDMI_MODE               (0x01 << 4)
#define BIT_PAGE_6_TPI_BSTATUS2_DS_CASC_EXCEED             (0x01 << 3)
#define MSK_PAGE_6_TPI_BSTATUS2_DS_DEPTH                   (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0x4B TPI Video Mute Low Byte Register                   (Default: 0x00)    */
#define REG_PAGE_6_VID_MUTE0                               TX_PAGE_6, 0x4B

#define MSK_PAGE_6_VID_MUTE0_VIDEO_MUTE_DATA_7_0           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x4D TPI Video Mute Mid Byte Register                   (Default: 0x00)    */
#define REG_PAGE_6_VID_MUTE1                               TX_PAGE_6, 0x4D

#define MSK_PAGE_6_VID_MUTE1_VIDEO_MUTE_DATA_15_8          (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x4F TPI Video Mute High Byte Register                  (Default: 0x00)    */
#define REG_PAGE_6_VID_MUTE2                               TX_PAGE_6, 0x4F

#define MSK_PAGE_6_VID_MUTE2_VIDEO_MUTE_DATA_23_16         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x60 TPI Sync Generation Control Register               (Default: 0x00)    */
#define REG_PAGE_6_TPI_SYNC_GEN                            TX_PAGE_6, 0x60

#define BIT_PAGE_6_TPI_SYNC_GEN_ENDEMUX                    (0x01 << 5)

/*----------------------------------------------------------------------------*/
/* 0x61 TPI Video Sync Polarity Detection Register         (Default: 0x00)    */
#define REG_PAGE_6_TPI_POL_DETECT                          TX_PAGE_6, 0x61

#define BIT_PAGE_6_TPI_POL_DETECT_INTERLACEDOUT            (0x01 << 2)
#define BIT_PAGE_6_TPI_POL_DETECT_VSYNCPOLOUT              (0x01 << 1)
#define BIT_PAGE_6_TPI_POL_DETECT_HSYNCPOLOUT              (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x6A TPI Video H Resolution #1 Register                 (Default: 0x00)    */
#define REG_PAGE_6_TPI_H_RESL                              TX_PAGE_6, 0x6A

#define MSK_PAGE_6_TPI_H_RESL_HRESOUT_7_0                  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x6B TPI Video H Resolution #2 Register                 (Default: 0x00)    */
#define REG_PAGE_6_TPI_H_RESH                              TX_PAGE_6, 0x6B

#define MSK_PAGE_6_TPI_H_RESH_HRESOUT_12_8                 (0x1F << 0)

/*----------------------------------------------------------------------------*/
/* 0x6C TPI Video V Refresh Low Register                   (Default: 0x00)    */
#define REG_PAGE_6_TPI_V_RESL                              TX_PAGE_6, 0x6C

#define MSK_PAGE_6_TPI_V_RESL_VRESOUT_7_0                  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x6D TPI Video V Refresh High Register                  (Default: 0x00)    */
#define REG_PAGE_6_TPI_V_RESH                              TX_PAGE_6, 0x6D

#define MSK_PAGE_6_TPI_V_RESH_VRESOUT_10_8                 (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0x79 TPI HW Debug #1 Register                           (Default: 0x00)    */
#define REG_PAGE_6_TPI_HW_DBG1                             TX_PAGE_6, 0x79

#define BIT_PAGE_6_TPI_HW_DBG1_READ_KSV_LIST_DONE          (0x01 << 7)
#define BIT_PAGE_6_TPI_HW_DBG1_READ_BSTATUS_DONE           (0x01 << 6)
#define BIT_PAGE_6_TPI_HW_DBG1_READ_KSV_FIFO_RDY_DONE      (0x01 << 5)
#define BIT_PAGE_6_TPI_HW_DBG1_READ_R0_PRIME_DONE          (0x01 << 4)
#define BIT_PAGE_6_TPI_HW_DBG1_WRITE_AKSV_DONE             (0x01 << 3)
#define BIT_PAGE_6_TPI_HW_DBG1_WRITE_AN_DONE               (0x01 << 2)
#define BIT_PAGE_6_TPI_HW_DBG1_READ_RX_REPEATER_DONE       (0x01 << 1)
#define BIT_PAGE_6_TPI_HW_DBG1_READ_BKSV_DONE              (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x7A TPI HW Debug #2 Register                           (Default: 0x00)    */
#define REG_PAGE_6_TPI_HW_DBG2                             TX_PAGE_6, 0x7A

#define BIT_PAGE_6_TPI_HW_DBG2_READ_RI_2ND_DONE            (0x01 << 2)
#define BIT_PAGE_6_TPI_HW_DBG2_READ_RI_PRIME_DONE          (0x01 << 1)
#define BIT_PAGE_6_TPI_HW_DBG2_READ_V_PRIME_DONE           (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x7B TPI HW Debug #3 Register                           (Default: 0x00)    */
#define REG_PAGE_6_TPI_HW_DBG3                             TX_PAGE_6, 0x7B

#define BIT_PAGE_6_TPI_HW_DBG3_READ_KSV_LIST_ERR           (0x01 << 7)
#define BIT_PAGE_6_TPI_HW_DBG3_READ_BSTATUS_ERR            (0x01 << 6)
#define BIT_PAGE_6_TPI_HW_DBG3_READ_KSV_FIFO_RDY_ERR       (0x01 << 5)
#define BIT_PAGE_6_TPI_HW_DBG3_READ_R0_PRIME_ERR           (0x01 << 4)
#define BIT_PAGE_6_TPI_HW_DBG3_WRITE_AKSV_ERR              (0x01 << 3)
#define BIT_PAGE_6_TPI_HW_DBG3_WRITE_AN_ERR                (0x01 << 2)
#define BIT_PAGE_6_TPI_HW_DBG3_READ_RX_REPEATER_ERR        (0x01 << 1)
#define BIT_PAGE_6_TPI_HW_DBG3_READ_BKSV_ERR               (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x7C TPI HW Debug #4 Register                           (Default: 0x00)    */
#define REG_PAGE_6_TPI_HW_DBG4                             TX_PAGE_6, 0x7C

#define BIT_PAGE_6_TPI_HW_DBG4_READ_RI_2ND_ERR             (0x01 << 2)
#define BIT_PAGE_6_TPI_HW_DBG4_READ_RI_PRIME_ERR           (0x01 << 1)
#define BIT_PAGE_6_TPI_HW_DBG4_READ_V_PRIME_ERR            (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x7D TPI HW Debug #5 Register                           (Default: 0x00)    */
#define REG_PAGE_6_TPI_HW_DBG5                             TX_PAGE_6, 0x7D

#define MSK_PAGE_6_TPI_HW_DBG5_TPI_DS_AUTH_CS              (0x0F << 4)
#define MSK_PAGE_6_TPI_HW_DBG5_TPI_HW_CS                   (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x7F TPI HW Debug #7 Register                           (Default: 0x00)    */
#define REG_PAGE_6_TPI_HW_DBG7                             TX_PAGE_6, 0x7F

#define MSK_PAGE_6_TPI_HW_DBG7_TPI_DDCM_CTL_CS_3_0         (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x80 TPI HW Debug #8 Register                           (Default: 0x00)    */
#define REG_PAGE_6_TPI_HW_DBG8                             TX_PAGE_6, 0x80

#define MSK_PAGE_6_TPI_HW_DBG8_DDC_HDCP_ACC_NMB_7_0        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x81 TPI HW Debug #9 Register                           (Default: 0x00)    */
#define REG_PAGE_6_TPI_HW_DBG9                             TX_PAGE_6, 0x81

#define MSK_PAGE_6_TPI_HW_DBG9_DDC_HDCP_ACC_NMB_9_8        (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0xB9 TPI HW Optimization Control #1 Register            (Default: 0x07)    */
#define REG_PAGE_6_TPI_HW_OPT1                             TX_PAGE_6, 0xB9

#define BIT_PAGE_6_TPI_HW_OPT1_DDC_DELAY_CNT_8             (0x01 << 7)
#define MSK_PAGE_6_TPI_HW_OPT1_TPI_AUTH_RETRY_CNT_2_0      (0x07 << 4)
#define MSK_PAGE_6_TPI_HW_OPT1_TPI_R0_CALC_TIME_3_0        (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xBA TPI HW Optimization Control #2 Register            (Default: 0x01)    */
#define REG_PAGE_6_TPI_HW_OPT2                             TX_PAGE_6, 0xBA

#define MSK_PAGE_6_TPI_HW_OPT2_DDC_DELAY_CNT_7_0           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBB TPI HW Optimization Control #3 Register            (Default: 0x00)    */
#define REG_PAGE_6_TPI_HW_OPT3                             TX_PAGE_6, 0xBB

#define BIT_PAGE_6_TPI_HW_OPT3_DDC_DEBUG                   (0x01 << 7)
#define BIT_PAGE_6_TPI_HW_OPT3_RI_CHECK_SKIP               (0x01 << 3)
#define BIT_PAGE_6_TPI_HW_OPT3_TPI_DDC_BURST_MODE          (0x01 << 2)
#define MSK_PAGE_6_TPI_HW_OPT3_TPI_DDC_REQ_LEVEL           (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0xBF TPI Info Frame Select Register                     (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_FSEL                           TX_PAGE_6, 0xBF

#define BIT_PAGE_6_TPI_INFO_FSEL_TPI_INFO_EN               (0x01 << 7)
#define BIT_PAGE_6_TPI_INFO_FSEL_TPI_INFO_RPT              (0x01 << 6)
#define BIT_PAGE_6_TPI_INFO_FSEL_TPI_INFO_READ_FLAG        (0x01 << 5)
#define MSK_PAGE_6_TPI_INFO_FSEL_TPI_INFO_SEL              (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0xC0 TPI Info Byte #0 Register                          (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B0                             TX_PAGE_6, 0xC0

#define MSK_PAGE_6_TPI_INFO_B0_MODE_BYTE0                  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC1 TPI Info Byte #1 Register                          (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B1                             TX_PAGE_6, 0xC1

#define MSK_PAGE_6_TPI_INFO_B1_MODE_BYTE1                  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC2 TPI Info Byte #2 Register                          (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B2                             TX_PAGE_6, 0xC2

#define MSK_PAGE_6_TPI_INFO_B2_MODE_BYTE2                  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC3 TPI Info Byte #3 Register                          (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B3                             TX_PAGE_6, 0xC3

#define MSK_PAGE_6_TPI_INFO_B3_MODE_BYTE3                  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC4 TPI Info Byte #4 Register                          (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B4                             TX_PAGE_6, 0xC4

#define MSK_PAGE_6_TPI_INFO_B4_MODE_BYTE4                  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC5 TPI Info Byte #5 Register                          (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B5                             TX_PAGE_6, 0xC5

#define MSK_PAGE_6_TPI_INFO_B5_MODE_BYTE5                  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC6 TPI Info Byte #6 Register                          (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B6                             TX_PAGE_6, 0xC6

#define MSK_PAGE_6_TPI_INFO_B6_MODE_BYTE6                  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC7 TPI Info Byte #7 Register                          (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B7                             TX_PAGE_6, 0xC7

#define MSK_PAGE_6_TPI_INFO_B7_MODE_BYTE7                  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC8 TPI Info Byte #8 Register                          (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B8                             TX_PAGE_6, 0xC8

#define MSK_PAGE_6_TPI_INFO_B8_MODE_BYTE8                  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC9 TPI Info Byte #9 Register                          (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B9                             TX_PAGE_6, 0xC9

#define MSK_PAGE_6_TPI_INFO_B9_MODE_BYTE9                  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCA TPI Info Byte #10 Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B10                            TX_PAGE_6, 0xCA

#define MSK_PAGE_6_TPI_INFO_B10_MODE_BYTE10                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCB TPI Info Byte #11 Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B11                            TX_PAGE_6, 0xCB

#define MSK_PAGE_6_TPI_INFO_B11_MODE_BYTE11                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCC TPI Info Byte #12 Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B12                            TX_PAGE_6, 0xCC

#define MSK_PAGE_6_TPI_INFO_B12_MODE_BYTE12                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCD TPI Info Byte #13 Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B13                            TX_PAGE_6, 0xCD

#define MSK_PAGE_6_TPI_INFO_B13_MODE_BYTE13                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCE TPI Info Byte #14 Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B14                            TX_PAGE_6, 0xCE

#define MSK_PAGE_6_TPI_INFO_B14_MODE_BYTE14                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCF TPI Info Byte #15 Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B15                            TX_PAGE_6, 0xCF

#define MSK_PAGE_6_TPI_INFO_B15_MODE_BYTE15                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD0 TPI Info Byte #16 Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B16                            TX_PAGE_6, 0xD0

#define MSK_PAGE_6_TPI_INFO_B16_MODE_BYTE16                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD1 TPI Info Byte #17 Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B17                            TX_PAGE_6, 0xD1

#define MSK_PAGE_6_TPI_INFO_B17_MODE_BYTE17                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD2 TPI Info Byte #18 Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B18                            TX_PAGE_6, 0xD2

#define MSK_PAGE_6_TPI_INFO_B18_MODE_BYTE18                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD3 TPI Info Byte #19 Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B19                            TX_PAGE_6, 0xD3

#define MSK_PAGE_6_TPI_INFO_B19_MODE_BYTE19                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD4 TPI Info Byte #20 Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B20                            TX_PAGE_6, 0xD4

#define MSK_PAGE_6_TPI_INFO_B20_MODE_BYTE20                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD5 TPI Info Byte #21 Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B21                            TX_PAGE_6, 0xD5

#define MSK_PAGE_6_TPI_INFO_B21_MODE_BYTE21                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD6 TPI Info Byte #22 Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B22                            TX_PAGE_6, 0xD6

#define MSK_PAGE_6_TPI_INFO_B22_MODE_BYTE22                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD7 TPI Info Byte #23 Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B23                            TX_PAGE_6, 0xD7

#define MSK_PAGE_6_TPI_INFO_B23_MODE_BYTE23                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD8 TPI Info Byte #24 Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B24                            TX_PAGE_6, 0xD8

#define MSK_PAGE_6_TPI_INFO_B24_MODE_BYTE24                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD9 TPI Info Byte #25 Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B25                            TX_PAGE_6, 0xD9

#define MSK_PAGE_6_TPI_INFO_B25_MODE_BYTE25                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xDA TPI Info Byte #26 Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B26                            TX_PAGE_6, 0xDA

#define MSK_PAGE_6_TPI_INFO_B26_MODE_BYTE26                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xDB TPI Info Byte #27 Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B27                            TX_PAGE_6, 0xDB

#define MSK_PAGE_6_TPI_INFO_B27_MODE_BYTE27                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xDC TPI Info Byte #28 Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B28                            TX_PAGE_6, 0xDC

#define MSK_PAGE_6_TPI_INFO_B28_MODE_BYTE28                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xDD TPI Info Byte #29 Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B29                            TX_PAGE_6, 0xDD

#define MSK_PAGE_6_TPI_INFO_B29_MODE_BYTE29                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xDE TPI Info Byte #30 Register                         (Default: 0x00)    */
#define REG_PAGE_6_TPI_INFO_B30                            TX_PAGE_6, 0xDE

#define MSK_PAGE_6_TPI_INFO_B30_MODE_BYTE30                (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* Registers in TX_PAGE_7                                                     */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* 0x00 CoC 1st Status Register                            (Default: 0x00)    */
#define REG_PAGE_7_COC_STAT_0                              TX_PAGE_7, 0x00

#define BIT_PAGE_7_COC_STAT_0_COC_STATUS0_7                (0x01 << 7)
#define BIT_PAGE_7_COC_STAT_0_COC_STATUS0_6                (0x01 << 6)
#define MSK_PAGE_7_COC_STAT_0_COC_STATUS0_5_0              (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0x01 CoC 2nd Status Register                            (Default: 0x00)    */
#define REG_PAGE_7_COC_STAT_1                              TX_PAGE_7, 0x01

#define MSK_PAGE_7_COC_STAT_1_COC_STATUS1_7_6              (0x03 << 6)
#define MSK_PAGE_7_COC_STAT_1_COC_STATUS1_5_0              (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0x02 CoC 3rd Status Register                            (Default: 0x00)    */
#define REG_PAGE_7_COC_STAT_2                              TX_PAGE_7, 0x02

#define MSK_PAGE_7_COC_STAT_2_COC_STATUS2_7_6              (0x03 << 6)
#define MSK_PAGE_7_COC_STAT_2_COC_STATUS2_5_0              (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0x03 CoC 4th Status Register                            (Default: 0x00)    */
#define REG_PAGE_7_COC_STAT_3                              TX_PAGE_7, 0x03

#define MSK_PAGE_7_COC_STAT_3_COC_STATUS3_7_0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x04 CoC 5th Status Register                            (Default: 0x00)    */
#define REG_PAGE_7_COC_STAT_4                              TX_PAGE_7, 0x04

#define MSK_PAGE_7_COC_STAT_4_COC_STATUS4_7_0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x05 CoC 6th Status Register                            (Default: 0x00)    */
#define REG_PAGE_7_COC_STAT_5                              TX_PAGE_7, 0x05

#define MSK_PAGE_7_COC_STAT_5_COC_STATUS5_7_0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x06 CoC 7th Status Register                            (Default: 0x00)    */
#define REG_PAGE_7_COC_STAT_6                              TX_PAGE_7, 0x06

#define BIT_PAGE_7_COC_STAT_6_COC_STATUS6_7                (0x01 << 7)
#define BIT_PAGE_7_COC_STAT_6_COC_STATUS6_6                (0x01 << 6)
#define BIT_PAGE_7_COC_STAT_6_COC_STATUS6_5                (0x01 << 5)
#define BIT_PAGE_7_COC_STAT_6_COC_STATUS6_4                (0x01 << 4)
#define BIT_PAGE_7_COC_STAT_6_COC_STATUS6_3                (0x01 << 3)
#define BIT_PAGE_7_COC_STAT_6_COC_STATUS6_2                (0x01 << 2)
#define BIT_PAGE_7_COC_STAT_6_COC_STATUS6_1                (0x01 << 1)
#define BIT_PAGE_7_COC_STAT_6_COC_STATUS6_0                (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x07 CoC 8th Status Register                            (Default: 0x00)    */
#define REG_PAGE_7_COC_STAT_7                              TX_PAGE_7, 0x07

#define MSK_PAGE_7_COC_STAT_7_COC_STATUS7_7_0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x08 CoC 9th Status Register                            (Default: 0x00)    */
#define REG_PAGE_7_COC_STAT_8                              TX_PAGE_7, 0x08

#define MSK_PAGE_7_COC_STAT_8_COC_STATUS8_7_0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x09 CoC 10th Status Register                           (Default: 0x00)    */
#define REG_PAGE_7_COC_STAT_9                              TX_PAGE_7, 0x09

#define BIT_PAGE_7_COC_STAT_9_COC_STATUS9_7                (0x01 << 7)
#define BIT_PAGE_7_COC_STAT_9_COC_STATUS9_6                (0x01 << 6)
#define BIT_PAGE_7_COC_STAT_9_COC_STATUS9_5                (0x01 << 5)
#define BIT_PAGE_7_COC_STAT_9_COC_STATUS9_4                (0x01 << 4)
#define MSK_PAGE_7_COC_STAT_9_COC_STATUS9_3_0              (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x0A CoC 11th Status Register                           (Default: 0x00)    */
#define REG_PAGE_7_COC_STAT_A                              TX_PAGE_7, 0x0A

#define MSK_PAGE_7_COC_STAT_A_COC_STATUSA_7_0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0B CoC 12th Status Register                           (Default: 0x00)    */
#define REG_PAGE_7_COC_STAT_B                              TX_PAGE_7, 0x0B

#define MSK_PAGE_7_COC_STAT_B_COC_STATUSB_7_0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0C CoC 13th Status Register                           (Default: 0x00)    */
#define REG_PAGE_7_COC_STAT_C                              TX_PAGE_7, 0x0C

#define MSK_PAGE_7_COC_STAT_C_COC_STATUSC_7_3              (0x1F << 3)
#define BIT_PAGE_7_COC_STAT_C_COC_STATUSC_2                (0x01 << 2)
#define MSK_PAGE_7_COC_STAT_C_COC_STATUSC_1_0              (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x0D CoC 14th Status Register                           (Default: 0x00)    */
#define REG_PAGE_7_COC_STAT_D                              TX_PAGE_7, 0x0D

#define MSK_PAGE_7_COC_STAT_D_COC_STATUSD_7_0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0E CoC 15th Status Register                           (Default: 0x00)    */
#define REG_PAGE_7_COC_STAT_E                              TX_PAGE_7, 0x0E

#define MSK_PAGE_7_COC_STAT_E_COC_STATUSE_7_0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x0F CoC 16th Status Register                           (Default: 0x00)    */
#define REG_PAGE_7_COC_STAT_F                              TX_PAGE_7, 0x0F

#define MSK_PAGE_7_COC_STAT_F_COC_STATUSF_7_4              (0x0F << 4)
#define MSK_PAGE_7_COC_STAT_F_COC_STATUSF_3_0              (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x10 CoC 1st Ctl Register                               (Default: 0x40)    */
#define REG_PAGE_7_COC_CTL0                                TX_PAGE_7, 0x10

#define BIT_PAGE_7_COC_CTL0_COC_CONTROL0_7                 (0x01 << 7)
#define BIT_PAGE_7_COC_CTL0_COC_CONTROL0_6                 (0x01 << 6)
#define BIT_PAGE_7_COC_CTL0_COC_CONTROL0_5                 (0x01 << 5)
#define MSK_PAGE_7_COC_CTL0_COC_CONTROL0_4_3               (0x03 << 3)
#define BIT_PAGE_7_COC_CTL0_COC_CONTROL0_2                 (0x01 << 2)
#define BIT_PAGE_7_COC_CTL0_COC_CONTROL0_1                 (0x01 << 1)
#define BIT_PAGE_7_COC_CTL0_COC_CONTROL0_0                 (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x11 CoC 2nd Ctl Register                               (Default: 0x0A)    */
#define REG_PAGE_7_COC_CTL1                                TX_PAGE_7, 0x11

#define MSK_PAGE_7_COC_CTL1_COC_CONTROL1_7_6               (0x03 << 6)
#define MSK_PAGE_7_COC_CTL1_COC_CONTROL1_5_0               (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0x12 CoC 3rd Ctl Register                               (Default: 0x14)    */
#define REG_PAGE_7_COC_CTL2                                TX_PAGE_7, 0x12

#define MSK_PAGE_7_COC_CTL2_COC_CONTROL2_7_6               (0x03 << 6)
#define MSK_PAGE_7_COC_CTL2_COC_CONTROL2_5_0               (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0x13 CoC 4th Ctl Register                               (Default: 0x40)    */
#define REG_PAGE_7_COC_CTL3                                TX_PAGE_7, 0x13

#define BIT_PAGE_7_COC_CTL3_COC_CONTROL3_7                 (0x01 << 7)
#define MSK_PAGE_7_COC_CTL3_COC_CONTROL3_6_0               (0x7F << 0)

/*----------------------------------------------------------------------------*/
/* 0x14 CoC 5th Ctl Register                               (Default: 0x00)    */
#define REG_PAGE_7_COC_CTL4                                TX_PAGE_7, 0x14

#define BIT_PAGE_7_COC_CTL4_COC_CONTROL4_7                 (0x01 << 7)
#define MSK_PAGE_7_COC_CTL4_COC_CONTROL4_6_0               (0x7F << 0)

/*----------------------------------------------------------------------------*/
/* 0x15 CoC 6th Ctl Register                               (Default: 0x00)    */
#define REG_PAGE_7_COC_CTL5                                TX_PAGE_7, 0x15

#define MSK_PAGE_7_COC_CTL5_COC_CONTROL5                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x16 CoC 7th Ctl Register                               (Default: 0x00)    */
#define REG_PAGE_7_COC_CTL6                                TX_PAGE_7, 0x16

#define BIT_PAGE_7_COC_CTL6_COC_CONTROL6_7                 (0x01 << 7)
#define BIT_PAGE_7_COC_CTL6_COC_CONTROL6_6                 (0x01 << 6)
#define MSK_PAGE_7_COC_CTL6_COC_CONTROL6_5_0               (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0x17 CoC 8th Ctl Register                               (Default: 0x06)    */
#define REG_PAGE_7_COC_CTL7                                TX_PAGE_7, 0x17

#define BIT_PAGE_7_COC_CTL7_COC_CONTROL7_7                 (0x01 << 7)
#define BIT_PAGE_7_COC_CTL7_COC_CONTROL7_6                 (0x01 << 6)
#define BIT_PAGE_7_COC_CTL7_COC_CONTROL7_5                 (0x01 << 5)
#define MSK_PAGE_7_COC_CTL7_COC_CONTROL7_4_3               (0x03 << 3)
#define MSK_PAGE_7_COC_CTL7_COC_CONTROL7_2_0               (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0x18 CoC 9th Ctl Register                               (Default: 0x00)    */
#define REG_PAGE_7_COC_CTL8                                TX_PAGE_7, 0x18

#define MSK_PAGE_7_COC_CTL8_COC_CONTROL8                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x19 CoC 10th Ctl Register                              (Default: 0x00)    */
#define REG_PAGE_7_COC_CTL9                                TX_PAGE_7, 0x19

#define MSK_PAGE_7_COC_CTL9_COC_CONTROL9                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x1A CoC 11th Ctl Register                              (Default: 0x00)    */
#define REG_PAGE_7_COC_CTLA                                TX_PAGE_7, 0x1A

#define MSK_PAGE_7_COC_CTLA_COC_CONTROLA                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x1B CoC 12th Ctl Register                              (Default: 0x00)    */
#define REG_PAGE_7_COC_CTLB                                TX_PAGE_7, 0x1B

#define MSK_PAGE_7_COC_CTLB_COC_CONTROLB                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x1C CoC 13th Ctl Register                              (Default: 0x0F)    */
#define REG_PAGE_7_COC_CTLC                                TX_PAGE_7, 0x1C

#define MSK_PAGE_7_COC_CTLC_COC_CONTROLC                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x1D CoC 14th Ctl Register                              (Default: 0x0A)    */
#define REG_PAGE_7_COC_CTLD                                TX_PAGE_7, 0x1D

#define BIT_PAGE_7_COC_CTLD_COC_CONTROLD_7                 (0x01 << 7)
#define MSK_PAGE_7_COC_CTLD_COC_CONTROLD_6_0               (0x7F << 0)

/*----------------------------------------------------------------------------*/
/* 0x1E CoC 15th Ctl Register                              (Default: 0x0A)    */
#define REG_PAGE_7_COC_CTLE                                TX_PAGE_7, 0x1E

#define BIT_PAGE_7_COC_CTLE_COC_CONTROLE_7                 (0x01 << 7)
#define MSK_PAGE_7_COC_CTLE_COC_CONTROLE_6_0               (0x7F << 0)

/*----------------------------------------------------------------------------*/
/* 0x1F CoC 16th Ctl Register                              (Default: 0x00)    */
#define REG_PAGE_7_COC_CTLF                                TX_PAGE_7, 0x1F

#define MSK_PAGE_7_COC_CTLF_COC_CONTROLF                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x20 CoC 17th Ctl Register                              (Default: 0x15)    */
#define REG_PAGE_7_COC_CTL10                               TX_PAGE_7, 0x20

#define MSK_PAGE_7_COC_CTL10_COC_CONTROL10_7_4             (0x0F << 4)
#define MSK_PAGE_7_COC_CTL10_COC_CONTROL10_3_0             (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x21 CoC 18th Ctl Register                              (Default: 0x32)    */
#define REG_PAGE_7_COC_CTL11                               TX_PAGE_7, 0x21

#define MSK_PAGE_7_COC_CTL11_COC_CONTROL11_7_4             (0x0F << 4)
#define MSK_PAGE_7_COC_CTL11_COC_CONTROL11_3_0             (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x22 CoC 19th Ctl Register                              (Default: 0x04)    */
#define REG_PAGE_7_COC_CTL12                               TX_PAGE_7, 0x22

#define BIT_PAGE_7_COC_CTL12_COC_CONTROL12_7               (0x01 << 7)
#define BIT_PAGE_7_COC_CTL12_COC_CONTROL12_6               (0x01 << 6)
#define MSK_PAGE_7_COC_CTL12_COC_CONTROL12_5_0             (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0x23 CoC 20th Ctl Register                              (Default: 0x76)    */
#define REG_PAGE_7_COC_CTL13                               TX_PAGE_7, 0x23

#define MSK_PAGE_7_COC_CTL13_COC_CONTROL13                 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x24 CoC 21st Ctl Register                              (Default: 0x00)    */
#define REG_PAGE_7_COC_CTL14                               TX_PAGE_7, 0x24

#define MSK_PAGE_7_COC_CTL14_COC_CONTROL14_7_4             (0x0F << 4)
#define MSK_PAGE_7_COC_CTL14_COC_CONTROL14_3_0             (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x25 CoC 22nd Ctl Register                              (Default: 0x00)    */
#define REG_PAGE_7_COC_CTL15                               TX_PAGE_7, 0x25

#define BIT_PAGE_7_COC_CTL15_COC_CONTROL15_7               (0x01 << 7)
#define MSK_PAGE_7_COC_CTL15_COC_CONTROL15_6_4             (0x07 << 4)
#define MSK_PAGE_7_COC_CTL15_COC_CONTROL15_3_0             (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x26 CoC Interrupt Register                             (Default: 0x00)    */
#define REG_PAGE_7_COC_INTR                                TX_PAGE_7, 0x26

#define BIT_PAGE_7_COC_INTR_COC_INTR_STAT4                 (0x01 << 4)
#define BIT_PAGE_7_COC_INTR_COC_INTR_STAT3                 (0x01 << 3)
#define BIT_PAGE_7_COC_INTR_COC_INTR_STAT2                 (0x01 << 2)
#define BIT_PAGE_7_COC_INTR_COC_INTR_STAT1                 (0x01 << 1)
#define BIT_PAGE_7_COC_INTR_COC_INTR_STAT0                 (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x27 CoC Interrupt Mask Register                        (Default: 0x00)    */
#define REG_PAGE_7_COC_INTR_MASK                           TX_PAGE_7, 0x27

#define BIT_PAGE_7_COC_INTR_MASK_COC_INTR_MASK4            (0x01 << 4)
#define BIT_PAGE_7_COC_INTR_MASK_COC_INTR_MASK3            (0x01 << 3)
#define BIT_PAGE_7_COC_INTR_MASK_COC_INTR_MASK2            (0x01 << 2)
#define BIT_PAGE_7_COC_INTR_MASK_COC_INTR_MASK1            (0x01 << 1)
#define BIT_PAGE_7_COC_INTR_MASK_COC_INTR_MASK0            (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x28 CoC Misc 1st Ctl Register                          (Default: 0x00)    */
#define REG_PAGE_7_COC_MISC_CTL0                           TX_PAGE_7, 0x28

#define MSK_PAGE_7_COC_MISC_CTL0_COC_DBG_SEL               (0x03 << 3)
#define BIT_PAGE_7_COC_MISC_CTL0_COC_DBG_EN                (0x01 << 2)
#define BIT_PAGE_7_COC_MISC_CTL0_COC_OE                    (0x01 << 1)
#define BIT_PAGE_7_COC_MISC_CTL0_COC_OE_OVR                (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x38 DoC 1st Status Register                            (Default: 0x00)    */
#define REG_PAGE_7_DOC_STAT_0                              TX_PAGE_7, 0x38

#define BIT_PAGE_7_DOC_STAT_0_DOC_STATUS0_7                (0x01 << 7)
#define BIT_PAGE_7_DOC_STAT_0_DOC_STATUS0_6                (0x01 << 6)
#define MSK_PAGE_7_DOC_STAT_0_DOC_STATUS0_7_0              (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0x39 DoC 2nd Status Register                            (Default: 0x00)    */
#define REG_PAGE_7_DOC_STAT_1                              TX_PAGE_7, 0x39

#define MSK_PAGE_7_DOC_STAT_1_DOC_STATUS1_7_0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x3A DoC 3rd Status Register                            (Default: 0x00)    */
#define REG_PAGE_7_DOC_STAT_2                              TX_PAGE_7, 0x3A

#define MSK_PAGE_7_DOC_STAT_2_DOC_STATUS2_7_0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x3B DoC 4th Status Register                            (Default: 0x00)    */
#define REG_PAGE_7_DOC_STAT_3                              TX_PAGE_7, 0x3B

#define MSK_PAGE_7_DOC_STAT_3_DOC_STATUS3_7_0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x3C DoC 5th Status Register                            (Default: 0x00)    */
#define REG_PAGE_7_DOC_STAT_4                              TX_PAGE_7, 0x3C

#define MSK_PAGE_7_DOC_STAT_4_DOC_STATUS4_7_0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x3D DoC 6th Status Register                            (Default: 0x00)    */
#define REG_PAGE_7_DOC_STAT_5                              TX_PAGE_7, 0x3D

#define MSK_PAGE_7_DOC_STAT_5_DOC_STATUS5_7_0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x3E DoC 7th Status Register                            (Default: 0x00)    */
#define REG_PAGE_7_DOC_STAT_6                              TX_PAGE_7, 0x3E

#define MSK_PAGE_7_DOC_STAT_6_DOC_STATUS6_7_4              (0x0F << 4)
#define BIT_PAGE_7_DOC_STAT_6_DOC_STATUS6_3                (0x01 << 3)
#define BIT_PAGE_7_DOC_STAT_6_DOC_STATUS6_2                (0x01 << 2)
#define BIT_PAGE_7_DOC_STAT_6_DOC_STATUS6_1                (0x01 << 1)
#define BIT_PAGE_7_DOC_STAT_6_DOC_STATUS6_0                (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x3F DoC 8th Status Register                            (Default: 0x00)    */
#define REG_PAGE_7_DOC_STAT_7                              TX_PAGE_7, 0x3F

#define MSK_PAGE_7_DOC_STAT_7_DOC_STATUS7_7_0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x40 DoC 9th Status Register                            (Default: 0x00)    */
#define REG_PAGE_7_DOC_STAT_8                              TX_PAGE_7, 0x40

#define MSK_PAGE_7_DOC_STAT_8_DOC_STATUS8_7_0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x41 DoC 10th Status Register                           (Default: 0x00)    */
#define REG_PAGE_7_DOC_STAT_9                              TX_PAGE_7, 0x41

#define BIT_PAGE_7_DOC_STAT_9_DOC_STATUS9_7                (0x01 << 7)
#define BIT_PAGE_7_DOC_STAT_9_DOC_STATUS9_6                (0x01 << 6)
#define BIT_PAGE_7_DOC_STAT_9_DOC_STATUS9_5                (0x01 << 5)
#define BIT_PAGE_7_DOC_STAT_9_DOC_STATUS9_4                (0x01 << 4)
#define MSK_PAGE_7_DOC_STAT_9_DOC_STATUS9_3_0              (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x42 DoC 11th Status Register                           (Default: 0x00)    */
#define REG_PAGE_7_DOC_STAT_A                              TX_PAGE_7, 0x42

#define MSK_PAGE_7_DOC_STAT_A_DOC_STATUSA_7_0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x43 DoC 12th Status Register                           (Default: 0x00)    */
#define REG_PAGE_7_DOC_STAT_B                              TX_PAGE_7, 0x43

#define MSK_PAGE_7_DOC_STAT_B_DOC_STATUSB_7_0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x44 DoC 13th Status Register                           (Default: 0x00)    */
#define REG_PAGE_7_DOC_STAT_C                              TX_PAGE_7, 0x44

#define MSK_PAGE_7_DOC_STAT_C_DOC_STATUSC_7_6              (0x03 << 6)
#define BIT_PAGE_7_DOC_STAT_C_DOC_STATUSC_5                (0x01 << 5)
#define BIT_PAGE_7_DOC_STAT_C_DOC_STATUSC_4                (0x01 << 4)
#define BIT_PAGE_7_DOC_STAT_C_DOC_STATUSC_3                (0x01 << 3)
#define BIT_PAGE_7_DOC_STAT_C_DOC_STATUSC_2                (0x01 << 2)
#define MSK_PAGE_7_DOC_STAT_C_DOC_STATUSC_1_0              (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x45 DoC 14th Status Register                           (Default: 0x00)    */
#define REG_PAGE_7_DOC_STAT_D                              TX_PAGE_7, 0x45

#define MSK_PAGE_7_DOC_STAT_D_DOC_STATUSD_7_0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x46 DoC 15th Status Register                           (Default: 0x00)    */
#define REG_PAGE_7_DOC_STAT_E                              TX_PAGE_7, 0x46

#define MSK_PAGE_7_DOC_STAT_E_DOC_STATUSE_7_0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x47 DoC 16th Status Register                           (Default: 0x00)    */
#define REG_PAGE_7_DOC_STAT_F                              TX_PAGE_7, 0x47

#define MSK_PAGE_7_DOC_STAT_F_DOC_STATUSF_7_0              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x48 DoC DRX Status Register                            (Default: 0x00)    */
#define REG_PAGE_7_DOC_DRX                                 TX_PAGE_7, 0x48

#define MSK_PAGE_7_DOC_DRX_DOC_DRX_7_0                     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x49 DoC FSM Status Register                            (Default: 0x00)    */
#define REG_PAGE_7_DOC_FSM                                 TX_PAGE_7, 0x49

#define MSK_PAGE_7_DOC_FSM_STATE_DOC_FSM_3_0               (0x0F << 4)
#define MSK_PAGE_7_DOC_FSM_DOC_DRX_9_8                     (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x4A DoC 1st CFG Register                               (Default: 0x15)    */
#define REG_PAGE_7_DOC_CFG0                                TX_PAGE_7, 0x4A

#define MSK_PAGE_7_DOC_CFG0_K_CHAR_1T0_7_4                 (0x0F << 4)
#define MSK_PAGE_7_DOC_CFG0_K_CHAR_1T0_3_0                 (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x4B DoC 2nd CFG Register                               (Default: 0x32)    */
#define REG_PAGE_7_DOC_CFG1                                TX_PAGE_7, 0x4B

#define MSK_PAGE_7_DOC_CFG1_K_CHAR_3T2_7_4                 (0x0F << 4)
#define MSK_PAGE_7_DOC_CFG1_K_CHAR_3T2_3_0                 (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x4C DoC 3rd CFG Register                               (Default: 0x04)    */
#define REG_PAGE_7_DOC_CFG2                                TX_PAGE_7, 0x4C

#define MSK_PAGE_7_DOC_CFG2_K_CHAR_5T4_7_4                 (0x0F << 4)
#define MSK_PAGE_7_DOC_CFG2_K_CHAR_5T4_3_0                 (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x4D DoC 4th CFG Register                               (Default: 0x76)    */
#define REG_PAGE_7_DOC_CFG3                                TX_PAGE_7, 0x4D

#define MSK_PAGE_7_DOC_CFG3_K_CHAR_7T6_7_4                 (0x0F << 4)
#define MSK_PAGE_7_DOC_CFG3_K_CHAR_7T6_3_0                 (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x4E DoC 5th CFG Register                               (Default: 0x00)    */
#define REG_PAGE_7_DOC_CFG4                                TX_PAGE_7, 0x4E

#define MSK_PAGE_7_DOC_CFG4_DBG_STATE_DOC_FSM              (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x4F DoC 6th CFG Register                               (Default: 0x00)    */
#define REG_PAGE_7_DOC_CFG5                                TX_PAGE_7, 0x4F

#define MSK_PAGE_7_DOC_CFG5_DOC_CTRL_ALPHA_RL              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x50 DoC 7th CFG Register                               (Default: 0x00)    */
#define REG_PAGE_7_DOC_CFG6                                TX_PAGE_7, 0x50

#define MSK_PAGE_7_DOC_CFG6_DOC_CTRL_ALPHA_RH              (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x51 DoC 1st Ctl Register                               (Default: 0x40)    */
#define REG_PAGE_7_DOC_CTL0                                TX_PAGE_7, 0x51

#define BIT_PAGE_7_DOC_CTL0_DOC_CONTROL0_7                 (0x01 << 7)
#define BIT_PAGE_7_DOC_CTL0_DOC_CONTROL0_6                 (0x01 << 6)
#define BIT_PAGE_7_DOC_CTL0_DOC_CONTROL0_5                 (0x01 << 5)
#define BIT_PAGE_7_DOC_CTL0_DOC_CONTROL0_4                 (0x01 << 4)
#define BIT_PAGE_7_DOC_CTL0_DOC_CONTROL0_3                 (0x01 << 3)
#define BIT_PAGE_7_DOC_CTL0_DOC_CONTROL0_2                 (0x01 << 2)
#define BIT_PAGE_7_DOC_CTL0_DOC_CONTROL0_1                 (0x01 << 1)
#define BIT_PAGE_7_DOC_CTL0_DOC_CONTROL0_0                 (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x52 DoC 2nd Ctl Register                               (Default: 0x00)    */
#define REG_PAGE_7_DOC_CTL1                                TX_PAGE_7, 0x52

#define MSK_PAGE_7_DOC_CTL1_DOC_CONTROL1_7_5               (0x07 << 5)
#define MSK_PAGE_7_DOC_CTL1_DOC_CONTROL1_4_2               (0x07 << 2)
#define BIT_PAGE_7_DOC_CTL1_DOC_CONTROL1_1                 (0x01 << 1)
#define BIT_PAGE_7_DOC_CTL1_DOC_CONTROL1_0                 (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x53 DoC 3rd Ctl Register                               (Default: 0x35)    */
#define REG_PAGE_7_DOC_CTL2                                TX_PAGE_7, 0x53

#define MSK_PAGE_7_DOC_CTL2_DOC_CONTROL2                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x54 DoC 4th Ctl Register                               (Default: 0x00)    */
#define REG_PAGE_7_DOC_CTL3                                TX_PAGE_7, 0x54

#define BIT_PAGE_7_DOC_CTL3_DOC_CONTROL3_7                 (0x01 << 7)
#define MSK_PAGE_7_DOC_CTL3_DOC_CONTROL3_6_0               (0x7F << 0)

/*----------------------------------------------------------------------------*/
/* 0x55 DoC 5th Ctl Register                               (Default: 0x00)    */
#define REG_PAGE_7_DOC_CTL4                                TX_PAGE_7, 0x55

#define MSK_PAGE_7_DOC_CTL4_DOC_CONTROL4_7_0               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x56 DoC 6th Ctl Register                               (Default: 0x00)    */
#define REG_PAGE_7_DOC_CTL5                                TX_PAGE_7, 0x56

#define MSK_PAGE_7_DOC_CTL5_DOC_CONTROL5_7_6               (0x03 << 6)
#define MSK_PAGE_7_DOC_CTL5_DOC_CONTROL5_5_2               (0x0F << 2)
#define MSK_PAGE_7_DOC_CTL5_DOC_CONTROL5_1_0               (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x57 DoC 7th Ctl Register                               (Default: 0x00)    */
#define REG_PAGE_7_DOC_CTL6                                TX_PAGE_7, 0x57

#define BIT_PAGE_7_DOC_CTL6_DOC_CONTROL6_7                 (0x01 << 7)
#define MSK_PAGE_7_DOC_CTL6_DOC_CONTROL6_6_4               (0x07 << 4)
#define MSK_PAGE_7_DOC_CTL6_DOC_CONTROL6_3_0               (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x58 DoC 8th Ctl Register                               (Default: 0x00)    */
#define REG_PAGE_7_DOC_CTL7                                TX_PAGE_7, 0x58

#define BIT_PAGE_7_DOC_CTL7_DOC_CONTROL7_7                 (0x01 << 7)
#define BIT_PAGE_7_DOC_CTL7_DOC_CONTROL7_6                 (0x01 << 6)
#define BIT_PAGE_7_DOC_CTL7_DOC_CONTROL7_5                 (0x01 << 5)
#define MSK_PAGE_7_DOC_CTL7_DOC_CONTROL7_4_3               (0x03 << 3)
#define MSK_PAGE_7_DOC_CTL7_DOC_CONTROL7_2_0               (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0x6C DoC 9th Ctl Register                               (Default: 0x00)    */
#define REG_PAGE_7_DOC_CTL8                                TX_PAGE_7, 0x6C

#define MSK_PAGE_7_DOC_CTL8_DOC_CONTROL8_7_4               (0x0F << 4)
#define MSK_PAGE_7_DOC_CTL8_DOC_CONTROL8_3_2               (0x03 << 2)
#define MSK_PAGE_7_DOC_CTL8_DOC_CONTROL8_1_0               (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0x6D DoC 10th Ctl Register                              (Default: 0x00)    */
#define REG_PAGE_7_DOC_CTL9                                TX_PAGE_7, 0x6D

#define MSK_PAGE_7_DOC_CTL9_DOC_CONTROL9                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x6E DoC 11th Ctl Register                              (Default: 0x00)    */
#define REG_PAGE_7_DOC_CTLA                                TX_PAGE_7, 0x6E

#define MSK_PAGE_7_DOC_CTLA_DOC_CONTROLA                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x6F DoC 12th Ctl Register                              (Default: 0x00)    */
#define REG_PAGE_7_DOC_CTLB                                TX_PAGE_7, 0x6F

#define MSK_PAGE_7_DOC_CTLB_DOC_CONTROLB                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x70 DoC 13th Ctl Register                              (Default: 0x00)    */
#define REG_PAGE_7_DOC_CTLC                                TX_PAGE_7, 0x70

#define MSK_PAGE_7_DOC_CTLC_DOC_CONTROLC                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x71 DoC 14th Ctl Register                              (Default: 0x09)    */
#define REG_PAGE_7_DOC_CTLD                                TX_PAGE_7, 0x71

#define BIT_PAGE_7_DOC_CTLD_DOC_CONTROLD_7                 (0x01 << 7)
#define MSK_PAGE_7_DOC_CTLD_DOC_CONTROLD_6_4               (0x07 << 4)
#define MSK_PAGE_7_DOC_CTLD_DOC_CONTROLD_3_0               (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x72 DoC 15th Ctl Register                              (Default: 0x00)    */
#define REG_PAGE_7_DOC_CTLE                                TX_PAGE_7, 0x72

#define MSK_PAGE_7_DOC_CTLE_DOC_CONTROLE                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x73 DoC 16th Ctl Register                              (Default: 0x00)    */
#define REG_PAGE_7_DOC_CTLF                                TX_PAGE_7, 0x73

#define MSK_PAGE_7_DOC_CTLF_DOC_CONTROLF                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x74 DoC Interrupt Register                             (Default: 0x00)    */
#define REG_PAGE_7_DOC_INTR                                TX_PAGE_7, 0x74

#define MSK_PAGE_7_DOC_INTR_                               (0x07 << 5)
#define BIT_PAGE_7_DOC_INTR_DOC_INTR_STAT4                 (0x01 << 4)
#define BIT_PAGE_7_DOC_INTR_DOC_INTR_STAT3                 (0x01 << 3)
#define BIT_PAGE_7_DOC_INTR_DOC_INTR_STAT2                 (0x01 << 2)
#define BIT_PAGE_7_DOC_INTR_DOC_INTR_STAT1                 (0x01 << 1)
#define BIT_PAGE_7_DOC_INTR_DOC_INTR_STAT0                 (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x75 DoC Interrupt Register                             (Default: 0x00)    */
#define REG_PAGE_7_DOC_INTR_MASK                           TX_PAGE_7, 0x75

#define BIT_PAGE_7_DOC_INTR_MASK_DOC_INTR_MASK4            (0x01 << 4)
#define BIT_PAGE_7_DOC_INTR_MASK_DOC_INTR_MASK3            (0x01 << 3)
#define BIT_PAGE_7_DOC_INTR_MASK_DOC_INTR_MASK2            (0x01 << 2)
#define BIT_PAGE_7_DOC_INTR_MASK_DOC_INTR_MASK1            (0x01 << 1)
#define BIT_PAGE_7_DOC_INTR_MASK_DOC_INTR_MASK0            (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x76 DoC Misc 1st Ctl Register                          (Default: 0x00)    */
#define REG_PAGE_7_DOC_MISC_CTL0                           TX_PAGE_7, 0x76

#define BIT_PAGE_7_DOC_MISC_CTL0_DOC_DBG_SEL               (0x01 << 3)
#define BIT_PAGE_7_DOC_MISC_CTL0_DOC_DBG_EN                (0x01 << 2)
#define BIT_PAGE_7_DOC_MISC_CTL0_DOC_OE                    (0x01 << 1)
#define BIT_PAGE_7_DOC_MISC_CTL0_DOC_OE_OVR                (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* Registers in TX_PAGE_5                                                     */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* 0x60 CBUS WRITE_BURST Transmit 1st Byte Register        (Default: 0x00)    */
#define REG_PAGE_5_WB_XMIT_DATA_0                          TX_PAGE_5, 0x60

#define MSK_PAGE_5_WB_XMIT_DATA_0_WB_XMIT_DATA_0           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x61 CBUS WRITE_BURST Transmit 2nd Byte Register        (Default: 0x00)    */
#define REG_PAGE_5_WB_XMIT_DATA_1                          TX_PAGE_5, 0x61

#define MSK_PAGE_5_WB_XMIT_DATA_1_WB_XMIT_DATA_1           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x62 CBUS WRITE_BURST Transmit 3rd Byte Register        (Default: 0x00)    */
#define REG_PAGE_5_WB_XMIT_DATA_2                          TX_PAGE_5, 0x62

#define MSK_PAGE_5_WB_XMIT_DATA_2_WB_XMIT_DATA_2           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x63 CBUS WRITE_BURST Transmit 4th Byte Register        (Default: 0x00)    */
#define REG_PAGE_5_WB_XMIT_DATA_3                          TX_PAGE_5, 0x63

#define MSK_PAGE_5_WB_XMIT_DATA_3_WB_XMIT_DATA_3           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x64 CBUS WRITE_BURST Transmit 5th Byte Register        (Default: 0x00)    */
#define REG_PAGE_5_WB_XMIT_DATA_4                          TX_PAGE_5, 0x64

#define MSK_PAGE_5_WB_XMIT_DATA_4_WB_XMIT_DATA_4           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x65 CBUS WRITE_BURST Transmit 6th Byte Register        (Default: 0x00)    */
#define REG_PAGE_5_WB_XMIT_DATA_5                          TX_PAGE_5, 0x65

#define MSK_PAGE_5_WB_XMIT_DATA_5_WB_XMIT_DATA_5           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x66 CBUS WRITE_BURST Transmit 7th Byte Register        (Default: 0x00)    */
#define REG_PAGE_5_WB_XMIT_DATA_6                          TX_PAGE_5, 0x66

#define MSK_PAGE_5_WB_XMIT_DATA_6_WB_XMIT_DATA_6           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x67 CBUS WRITE_BURST Transmit 8th Byte Register        (Default: 0x00)    */
#define REG_PAGE_5_WB_XMIT_DATA_7                          TX_PAGE_5, 0x67

#define MSK_PAGE_5_WB_XMIT_DATA_7_WB_XMIT_DATA_7           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x68 CBUS WRITE_BURST Transmit 9th Byte Register        (Default: 0x00)    */
#define REG_PAGE_5_WB_XMIT_DATA_8                          TX_PAGE_5, 0x68

#define MSK_PAGE_5_WB_XMIT_DATA_8_WB_XMIT_DATA_8           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x69 CBUS WRITE_BURST Transmit 10th Byte Register       (Default: 0x00)    */
#define REG_PAGE_5_WB_XMIT_DATA_9                          TX_PAGE_5, 0x69

#define MSK_PAGE_5_WB_XMIT_DATA_9_WB_XMIT_DATA_9           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x6A CBUS WRITE_BURST Transmit 11th Byte Register       (Default: 0x00)    */
#define REG_PAGE_5_WB_XMIT_DATA_A                          TX_PAGE_5, 0x6A

#define MSK_PAGE_5_WB_XMIT_DATA_A_WB_XMIT_DATA_A           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x6B CBUS WRITE_BURST Transmit 12th Byte Register       (Default: 0x00)    */
#define REG_PAGE_5_WB_XMIT_DATA_B                          TX_PAGE_5, 0x6B

#define MSK_PAGE_5_WB_XMIT_DATA_B_WB_XMIT_DATA_B           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x6C CBUS WRITE_BURST Transmit 13th Byte Register       (Default: 0x00)    */
#define REG_PAGE_5_WB_XMIT_DATA_C                          TX_PAGE_5, 0x6C

#define MSK_PAGE_5_WB_XMIT_DATA_C_WB_XMIT_DATA_C           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x6D CBUS WRITE_BURST Transmit 14th Byte Register       (Default: 0x00)    */
#define REG_PAGE_5_WB_XMIT_DATA_D                          TX_PAGE_5, 0x6D

#define MSK_PAGE_5_WB_XMIT_DATA_D_WB_XMIT_DATA_D           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x6E CBUS WRITE_BURST Transmit 15th Byte Register       (Default: 0x00)    */
#define REG_PAGE_5_WB_XMIT_DATA_E                          TX_PAGE_5, 0x6E

#define MSK_PAGE_5_WB_XMIT_DATA_E_WB_XMIT_DATA_E           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x6F CBUS WRITE_BURST Transmit 16th Byte Register       (Default: 0x00)    */
#define REG_PAGE_5_WB_XMIT_DATA_F                          TX_PAGE_5, 0x6F

#define MSK_PAGE_5_WB_XMIT_DATA_F_WB_XMIT_DATA_F           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x80 Interrupt Mask 1st Register                        (Default: 0x00)    */
#define REG_PAGE_5_MHL_INT_0_MASK                          TX_PAGE_5, 0x80

#define BIT_PAGE_5_MHL_INT_0_MASK_MHL_INT_0_MASK7          (0x01 << 7)
#define BIT_PAGE_5_MHL_INT_0_MASK_MHL_INT_0_MASK6          (0x01 << 6)
#define BIT_PAGE_5_MHL_INT_0_MASK_MHL_INT_0_MASK5          (0x01 << 5)
#define BIT_PAGE_5_MHL_INT_0_MASK_MHL_INT_0_MASK4          (0x01 << 4)
#define BIT_PAGE_5_MHL_INT_0_MASK_MHL_INT_0_MASK3          (0x01 << 3)
#define BIT_PAGE_5_MHL_INT_0_MASK_MHL_INT_0_MASK2          (0x01 << 2)
#define BIT_PAGE_5_MHL_INT_0_MASK_MHL_INT_0_MASK1          (0x01 << 1)
#define BIT_PAGE_5_MHL_INT_0_MASK_MHL_INT_0_MASK0          (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x81 Interrupt Mask 2nd Register                        (Default: 0x00)    */
#define REG_PAGE_5_MHL_INT_1_MASK                          TX_PAGE_5, 0x81

#define BIT_PAGE_5_MHL_INT_1_MASK_MHL_INT_1_MASK7          (0x01 << 7)
#define BIT_PAGE_5_MHL_INT_1_MASK_MHL_INT_1_MASK6          (0x01 << 6)
#define BIT_PAGE_5_MHL_INT_1_MASK_MHL_INT_1_MASK5          (0x01 << 5)
#define BIT_PAGE_5_MHL_INT_1_MASK_MHL_INT_1_MASK4          (0x01 << 4)
#define BIT_PAGE_5_MHL_INT_1_MASK_MHL_INT_1_MASK3          (0x01 << 3)
#define BIT_PAGE_5_MHL_INT_1_MASK_MHL_INT_1_MASK2          (0x01 << 2)
#define BIT_PAGE_5_MHL_INT_1_MASK_MHL_INT_1_MASK1          (0x01 << 1)
#define BIT_PAGE_5_MHL_INT_1_MASK_MHL_INT_1_MASK0          (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x82 Interrupt Mask 3rd Register                        (Default: 0x00)    */
#define REG_PAGE_5_MHL_INT_2_MASK                          TX_PAGE_5, 0x82

#define BIT_PAGE_5_MHL_INT_2_MASK_MHL_INT_2_MASK7          (0x01 << 7)
#define BIT_PAGE_5_MHL_INT_2_MASK_MHL_INT_2_MASK6          (0x01 << 6)
#define BIT_PAGE_5_MHL_INT_2_MASK_MHL_INT_2_MASK5          (0x01 << 5)
#define BIT_PAGE_5_MHL_INT_2_MASK_MHL_INT_2_MASK4          (0x01 << 4)
#define BIT_PAGE_5_MHL_INT_2_MASK_MHL_INT_2_MASK3          (0x01 << 3)
#define BIT_PAGE_5_MHL_INT_2_MASK_MHL_INT_2_MASK2          (0x01 << 2)
#define BIT_PAGE_5_MHL_INT_2_MASK_MHL_INT_2_MASK1          (0x01 << 1)
#define BIT_PAGE_5_MHL_INT_2_MASK_MHL_INT_2_MASK0          (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x83 Interrupt Mask 4th Register                        (Default: 0x00)    */
#define REG_PAGE_5_MHL_INT_3_MASK                          TX_PAGE_5, 0x83

#define BIT_PAGE_5_MHL_INT_3_MASK_MHL_INT_3_MASK7          (0x01 << 7)
#define BIT_PAGE_5_MHL_INT_3_MASK_MHL_INT_3_MASK6          (0x01 << 6)
#define BIT_PAGE_5_MHL_INT_3_MASK_MHL_INT_3_MASK5          (0x01 << 5)
#define BIT_PAGE_5_MHL_INT_3_MASK_MHL_INT_3_MASK4          (0x01 << 4)
#define BIT_PAGE_5_MHL_INT_3_MASK_MHL_INT_3_MASK3          (0x01 << 3)
#define BIT_PAGE_5_MHL_INT_3_MASK_MHL_INT_3_MASK2          (0x01 << 2)
#define BIT_PAGE_5_MHL_INT_3_MASK_MHL_INT_3_MASK1          (0x01 << 1)
#define BIT_PAGE_5_MHL_INT_3_MASK_MHL_INT_3_MASK0          (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x84 MDT Reveive Time Out Register                      (Default: 0x00)    */
#define REG_PAGE_5_MDT_RCV_TIMEOUT                         TX_PAGE_5, 0x84

#define MSK_PAGE_5_MDT_RCV_TIMEOUT_MDT_RCV_TIMEOUT_MAX_MSB (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x85 MDT Transmit Time Out Register                     (Default: 0x00)    */
#define REG_PAGE_5_MDT_XMIT_TIMEOUT                        TX_PAGE_5, 0x85

#define MSK_PAGE_5_MDT_XMIT_TIMEOUT_MDT_XMIT_TIMEOUT_MAX_MSB (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x86 MDT Receive Control Register                       (Default: 0x00)    */
#define REG_PAGE_5_MDT_RCV_CONTROL                         TX_PAGE_5, 0x86

#define BIT_PAGE_5_MDT_RCV_CONTROL_MDT_RCV_EN              (0x01 << 7)
#define BIT_PAGE_5_MDT_RCV_CONTROL_MDT_RFIFO_OVER_WR_EN    (0x01 << 4)
#define BIT_PAGE_5_MDT_RCV_CONTROL_MDT_XFIFO_OVER_WR_EN    (0x01 << 3)
#define BIT_PAGE_5_MDT_RCV_CONTROL_MDT_DISABLE             (0x01 << 2)
#define BIT_PAGE_5_MDT_RCV_CONTROL_MDT_RFIFO_CLR_ALL       (0x01 << 1)
#define BIT_PAGE_5_MDT_RCV_CONTROL_MDT_RFIFO_CLR_CUR       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x87 MDT Receive Read Port                              (Default: 0x00)    */
#define REG_PAGE_5_MDT_RCV_READ_PORT                       TX_PAGE_5, 0x87

#define MSK_PAGE_5_MDT_RCV_READ_PORT_MDT_RFIFO_DATA        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x88 MDT Transmit Control Register                      (Default: 0x70)    */
#define REG_PAGE_5_MDT_XMIT_CONTROL                        TX_PAGE_5, 0x88

#define BIT_PAGE_5_MDT_XMIT_CONTROL_MDT_XMIT_EN            (0x01 << 7)
#define BIT_PAGE_5_MDT_XMIT_CONTROL_MDT_XMIT_CMD_MERGE_EN  (0x01 << 6)
#define BIT_PAGE_5_MDT_XMIT_CONTROL_MDT_XMIT_FIXED_BURST_LEN (0x01 << 5)
#define BIT_PAGE_5_MDT_XMIT_CONTROL_MDT_XMIT_FIXED_AID     (0x01 << 4)
#define BIT_PAGE_5_MDT_XMIT_CONTROL_MDT_XMIT_SINGLE_RUN_EN (0x01 << 3)
#define BIT_PAGE_5_MDT_XMIT_CONTROL_MDT_CLR_ABORT_WAIT     (0x01 << 2)
#define BIT_PAGE_5_MDT_XMIT_CONTROL_MDT_XFIFO_CLR_ALL      (0x01 << 1)
#define BIT_PAGE_5_MDT_XMIT_CONTROL_MDT_XFIFO_CLR_CUR      (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x89 MDT Receive WRITE Port                             (Default: 0x00)    */
#define REG_PAGE_5_MDT_XMIT_WRITE_PORT                     TX_PAGE_5, 0x89

#define MSK_PAGE_5_MDT_XMIT_WRITE_PORT_MDT_XFIFO_WDATA     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x8A MDT RFIFO Status Register                          (Default: 0x00)    */
#define REG_PAGE_5_MDT_RFIFO_STAT                          TX_PAGE_5, 0x8A

#define MSK_PAGE_5_MDT_RFIFO_STAT_MDT_RFIFO_CNT            (0x07 << 5)
#define MSK_PAGE_5_MDT_RFIFO_STAT_MDT_RFIFO_CUR_BYTE_CNT   (0x1F << 0)

/*----------------------------------------------------------------------------*/
/* 0x8B MDT XFIFO Status Register                          (Default: 0x80)    */
#define REG_PAGE_5_MDT_XFIFO_STAT                          TX_PAGE_5, 0x8B

#define MSK_PAGE_5_MDT_XFIFO_STAT_MDT_XFIFO_LEVEL_AVAIL    (0x07 << 5)
#define BIT_PAGE_5_MDT_XFIFO_STAT_MDT_XMIT_PRE_HS_EN       (0x01 << 4)
#define MSK_PAGE_5_MDT_XFIFO_STAT_MDT_WRITE_BURST_LEN      (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0x8C MDT Interrupt 0 Register                           (Default: 0x0C)    */
#define REG_PAGE_5_MDT_INT_0                               TX_PAGE_5, 0x8C

#define BIT_PAGE_5_MDT_INT_0_MDT_INT_0_3                   (0x01 << 3)
#define BIT_PAGE_5_MDT_INT_0_MDT_INT_0_2                   (0x01 << 2)
#define BIT_PAGE_5_MDT_INT_0_MDT_INT_0_1                   (0x01 << 1)
#define BIT_PAGE_5_MDT_INT_0_MDT_INT_0_0                   (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x8D MDT Interrupt 0 Mask Register                      (Default: 0x00)    */
#define REG_PAGE_5_MDT_INT_0_MASK                          TX_PAGE_5, 0x8D

#define BIT_PAGE_5_MDT_INT_0_MASK_MDT_INT_0_MASK3          (0x01 << 3)
#define BIT_PAGE_5_MDT_INT_0_MASK_MDT_INT_0_MASK2          (0x01 << 2)
#define BIT_PAGE_5_MDT_INT_0_MASK_MDT_INT_0_MASK1          (0x01 << 1)
#define BIT_PAGE_5_MDT_INT_0_MASK_MDT_INT_0_MASK0          (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x8E MDT Interrupt 1 Register                           (Default: 0x00)    */
#define REG_PAGE_5_MDT_INT_1                               TX_PAGE_5, 0x8E

#define BIT_PAGE_5_MDT_INT_1_MDT_INT_1_7                   (0x01 << 7)
#define BIT_PAGE_5_MDT_INT_1_MDT_INT_1_6                   (0x01 << 6)
#define BIT_PAGE_5_MDT_INT_1_MDT_INT_1_5                   (0x01 << 5)
#define BIT_PAGE_5_MDT_INT_1_MDT_INT_1_2                   (0x01 << 2)
#define BIT_PAGE_5_MDT_INT_1_MDT_INT_1_1                   (0x01 << 1)
#define BIT_PAGE_5_MDT_INT_1_MDT_INT_1_0                   (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x8F MDT Interrupt 1 Mask Register                      (Default: 0x00)    */
#define REG_PAGE_5_MDT_INT_1_MASK                          TX_PAGE_5, 0x8F

#define BIT_PAGE_5_MDT_INT_1_MASK_MDT_INT_1_MASK7          (0x01 << 7)
#define BIT_PAGE_5_MDT_INT_1_MASK_MDT_INT_1_MASK6          (0x01 << 6)
#define BIT_PAGE_5_MDT_INT_1_MASK_MDT_INT_1_MASK5          (0x01 << 5)
#define BIT_PAGE_5_MDT_INT_1_MASK_MDT_INT_1_MASK2          (0x01 << 2)
#define BIT_PAGE_5_MDT_INT_1_MASK_MDT_INT_1_MASK1          (0x01 << 1)
#define BIT_PAGE_5_MDT_INT_1_MASK_MDT_INT_1_MASK0          (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x90 CBUS Vendor ID Register                            (Default: 0x01)    */
#define REG_PAGE_5_CBUS_VENDOR_ID                          TX_PAGE_5, 0x90

#define MSK_PAGE_5_CBUS_VENDOR_ID_CBUS_VENDOR_ID           (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0x91 CBUS Connection Status Register                    (Default: 0x00)    */
#define REG_PAGE_5_CBUS_STATUS                             TX_PAGE_5, 0x91

#define BIT_PAGE_5_CBUS_STATUS_MHL_CABLE_PRESENT           (0x01 << 4)
#define BIT_PAGE_5_CBUS_STATUS_MSC_HB_SUCCESS              (0x01 << 3)
#define BIT_PAGE_5_CBUS_STATUS_CBUS_HPD                    (0x01 << 2)
#define BIT_PAGE_5_CBUS_STATUS_MHL_MODE                    (0x01 << 1)
#define BIT_PAGE_5_CBUS_STATUS_CBUS_CONNECTED              (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x92 CBUS Interrupt 1st Register                        (Default: 0x00)    */
#define REG_PAGE_5_CBUS_INT_0                              TX_PAGE_5, 0x92

#define BIT_PAGE_5_CBUS_INT_0_CBUS_INT_0_STAT7             (0x01 << 7)
#define BIT_PAGE_5_CBUS_INT_0_CBUS_INT_0_STAT6             (0x01 << 6)
#define BIT_PAGE_5_CBUS_INT_0_CBUS_INT_0_STAT5             (0x01 << 5)
#define BIT_PAGE_5_CBUS_INT_0_CBUS_INT_0_STAT4             (0x01 << 4)
#define BIT_PAGE_5_CBUS_INT_0_CBUS_INT_0_STAT3             (0x01 << 3)
#define BIT_PAGE_5_CBUS_INT_0_CBUS_INT_0_STAT2             (0x01 << 2)
#define BIT_PAGE_5_CBUS_INT_0_CBUS_INT_0_STAT1             (0x01 << 1)
#define BIT_PAGE_5_CBUS_INT_0_CBUS_INT_0_STAT0             (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x93 CBUS Interrupt Mask 1st Register                   (Default: 0x00)    */
#define REG_PAGE_5_CBUS_INT_0_MASK                         TX_PAGE_5, 0x93

#define BIT_PAGE_5_CBUS_INT_0_MASK_CBUS_INT_0_MASK7        (0x01 << 7)
#define BIT_PAGE_5_CBUS_INT_0_MASK_CBUS_INT_0_MASK6        (0x01 << 6)
#define BIT_PAGE_5_CBUS_INT_0_MASK_CBUS_INT_0_MASK5        (0x01 << 5)
#define BIT_PAGE_5_CBUS_INT_0_MASK_CBUS_INT_0_MASK4        (0x01 << 4)
#define BIT_PAGE_5_CBUS_INT_0_MASK_CBUS_INT_0_MASK3        (0x01 << 3)
#define BIT_PAGE_5_CBUS_INT_0_MASK_CBUS_INT_0_MASK2        (0x01 << 2)
#define BIT_PAGE_5_CBUS_INT_0_MASK_CBUS_INT_0_MASK1        (0x01 << 1)
#define BIT_PAGE_5_CBUS_INT_0_MASK_CBUS_INT_0_MASK0        (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x94 CBUS Interrupt 2nd Register                        (Default: 0x00)    */
#define REG_PAGE_5_CBUS_INT_1                              TX_PAGE_5, 0x94

#define BIT_PAGE_5_CBUS_INT_1_CBUS_INT_1_STAT7             (0x01 << 7)
#define BIT_PAGE_5_CBUS_INT_1_CBUS_INT_1_STAT6             (0x01 << 6)
#define BIT_PAGE_5_CBUS_INT_1_CBUS_INT_1_STAT5             (0x01 << 5)
#define BIT_PAGE_5_CBUS_INT_1_CBUS_INT_1_STAT4             (0x01 << 4)
#define BIT_PAGE_5_CBUS_INT_1_CBUS_INT_1_STAT3             (0x01 << 3)
#define BIT_PAGE_5_CBUS_INT_1_CBUS_INT_1_STAT2             (0x01 << 2)
#define BIT_PAGE_5_CBUS_INT_1_CBUS_INT_1_STAT0             (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x95 CBUS Interrupt Mask 2nd Register                   (Default: 0x00)    */
#define REG_PAGE_5_CBUS_INT_1_MASK                         TX_PAGE_5, 0x95

#define BIT_PAGE_5_CBUS_INT_1_MASK_CBUS_INT_1_MASK7        (0x01 << 7)
#define BIT_PAGE_5_CBUS_INT_1_MASK_CBUS_INT_1_MASK6        (0x01 << 6)
#define BIT_PAGE_5_CBUS_INT_1_MASK_CBUS_INT_1_MASK5        (0x01 << 5)
#define BIT_PAGE_5_CBUS_INT_1_MASK_CBUS_INT_1_MASK4        (0x01 << 4)
#define BIT_PAGE_5_CBUS_INT_1_MASK_CBUS_INT_1_MASK3        (0x01 << 3)
#define BIT_PAGE_5_CBUS_INT_1_MASK_CBUS_INT_1_MASK2        (0x01 << 2)
#define BIT_PAGE_5_CBUS_INT_1_MASK_CBUS_INT_1_MASK0        (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x98 CBUS DDC Abort Interrupt Register                  (Default: 0x00)    */
#define REG_PAGE_5_DDC_ABORT_INT                           TX_PAGE_5, 0x98

#define BIT_PAGE_5_DDC_ABORT_INT_DDC_ABORT_INT_STAT7       (0x01 << 7)
#define BIT_PAGE_5_DDC_ABORT_INT_DDC_ABORT_INT_STAT2       (0x01 << 2)
#define BIT_PAGE_5_DDC_ABORT_INT_DDC_ABORT_INT_STAT1       (0x01 << 1)
#define BIT_PAGE_5_DDC_ABORT_INT_DDC_ABORT_INT_STAT0       (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x99 CBUS DDC Abort Interrupt Mask Register             (Default: 0x00)    */
#define REG_PAGE_5_DDC_ABORT_INT_MASK                      TX_PAGE_5, 0x99

#define BIT_PAGE_5_DDC_ABORT_INT_MASK_DDC_ABORT_INT_MASK7  (0x01 << 7)
#define BIT_PAGE_5_DDC_ABORT_INT_MASK_DDC_ABORT_INT_MASK2  (0x01 << 2)
#define BIT_PAGE_5_DDC_ABORT_INT_MASK_DDC_ABORT_INT_MASK1  (0x01 << 1)
#define BIT_PAGE_5_DDC_ABORT_INT_MASK_DDC_ABORT_INT_MASK0  (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x9A CBUS MSC Requester Abort Interrupt Register        (Default: 0x00)    */
#define REG_PAGE_5_MSC_MT_ABORT_INT                        TX_PAGE_5, 0x9A

#define BIT_PAGE_5_MSC_MT_ABORT_INT_MSC_MT_ABORT_INT_STAT7 (0x01 << 7)
#define BIT_PAGE_5_MSC_MT_ABORT_INT_MSC_MT_ABORT_INT_STAT5 (0x01 << 5)
#define BIT_PAGE_5_MSC_MT_ABORT_INT_MSC_MT_ABORT_INT_STAT3 (0x01 << 3)
#define BIT_PAGE_5_MSC_MT_ABORT_INT_MSC_MT_ABORT_INT_STAT2 (0x01 << 2)
#define BIT_PAGE_5_MSC_MT_ABORT_INT_MSC_MT_ABORT_INT_STAT1 (0x01 << 1)
#define BIT_PAGE_5_MSC_MT_ABORT_INT_MSC_MT_ABORT_INT_STAT0 (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x9B CBUS MSC Reqeuster Abort Interrupt Mask Register   (Default: 0x00)    */
#define REG_PAGE_5_MSC_MT_ABORT_INT_MASK                   TX_PAGE_5, 0x9B

#define BIT_PAGE_5_MSC_MT_ABORT_INT_MASK_MSC_MT_ABORT_INT_MASK7 (0x01 << 7)
#define BIT_PAGE_5_MSC_MT_ABORT_INT_MASK_MSC_MT_ABORT_INT_MASK5 (0x01 << 5)
#define BIT_PAGE_5_MSC_MT_ABORT_INT_MASK_MSC_MT_ABORT_INT_MASK3 (0x01 << 3)
#define BIT_PAGE_5_MSC_MT_ABORT_INT_MASK_MSC_MT_ABORT_INT_MASK2 (0x01 << 2)
#define BIT_PAGE_5_MSC_MT_ABORT_INT_MASK_MSC_MT_ABORT_INT_MASK1 (0x01 << 1)
#define BIT_PAGE_5_MSC_MT_ABORT_INT_MASK_MSC_MT_ABORT_INT_MASK0 (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x9C CBUS MSC Responder Abort Interrupt Register        (Default: 0x00)    */
#define REG_PAGE_5_MSC_MR_ABORT_INT                        TX_PAGE_5, 0x9C

#define BIT_PAGE_5_MSC_MR_ABORT_INT_MSC_MR_ABORT_INT_STAT7 (0x01 << 7)
#define BIT_PAGE_5_MSC_MR_ABORT_INT_MSC_MR_ABORT_INT_STAT5 (0x01 << 5)
#define BIT_PAGE_5_MSC_MR_ABORT_INT_MSC_MR_ABORT_INT_STAT4 (0x01 << 4)
#define BIT_PAGE_5_MSC_MR_ABORT_INT_MSC_MR_ABORT_INT_STAT3 (0x01 << 3)
#define BIT_PAGE_5_MSC_MR_ABORT_INT_MSC_MR_ABORT_INT_STAT2 (0x01 << 2)
#define BIT_PAGE_5_MSC_MR_ABORT_INT_MSC_MR_ABORT_INT_STAT1 (0x01 << 1)
#define BIT_PAGE_5_MSC_MR_ABORT_INT_MSC_MR_ABORT_INT_STAT0 (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x9D CBUS MSC Responder Abort Interrupt Mask Register   (Default: 0x00)    */
#define REG_PAGE_5_MSC_MR_ABORT_INT_MASK                   TX_PAGE_5, 0x9D

#define BIT_PAGE_5_MSC_MR_ABORT_INT_MASK_MSC_MR_ABORT_INT_MASK7 (0x01 << 7)
#define BIT_PAGE_5_MSC_MR_ABORT_INT_MASK_MSC_MR_ABORT_INT_MASK5 (0x01 << 5)
#define BIT_PAGE_5_MSC_MR_ABORT_INT_MASK_MSC_MR_ABORT_INT_MASK4 (0x01 << 4)
#define BIT_PAGE_5_MSC_MR_ABORT_INT_MASK_MSC_MR_ABORT_INT_MASK3 (0x01 << 3)
#define BIT_PAGE_5_MSC_MR_ABORT_INT_MASK_MSC_MR_ABORT_INT_MASK2 (0x01 << 2)
#define BIT_PAGE_5_MSC_MR_ABORT_INT_MASK_MSC_MR_ABORT_INT_MASK1 (0x01 << 1)
#define BIT_PAGE_5_MSC_MR_ABORT_INT_MASK_MSC_MR_ABORT_INT_MASK0 (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x9E CBUS RX DISCOVERY interrupt Register               (Default: 0x00)    */
#define REG_PAGE_5_CBUS_RX_DISC_INT0                       TX_PAGE_5, 0x9E

#define BIT_PAGE_5_CBUS_RX_DISC_INT0_CBUS_RXDISC_INTR0_STAT3 (0x01 << 3)
#define BIT_PAGE_5_CBUS_RX_DISC_INT0_CBUS_RXDISC_INTR0_STAT2 (0x01 << 2)
#define BIT_PAGE_5_CBUS_RX_DISC_INT0_CBUS_RXDISC_INTR0_STAT1 (0x01 << 1)
#define BIT_PAGE_5_CBUS_RX_DISC_INT0_CBUS_RXDISC_INTR0_STAT0 (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0x9F CBUS RX DISCOVERY Interrupt Mask Register          (Default: 0x00)    */
#define REG_PAGE_5_CBUS_RX_DISC_INT0_MASK                  TX_PAGE_5, 0x9F

#define BIT_PAGE_5_CBUS_RX_DISC_INT0_MASK_CBUS_RXDISC_INTR0_MASK3 (0x01 << 3)
#define BIT_PAGE_5_CBUS_RX_DISC_INT0_MASK_CBUS_RXDISC_INTR0_MASK2 (0x01 << 2)
#define BIT_PAGE_5_CBUS_RX_DISC_INT0_MASK_CBUS_RXDISC_INTR0_MASK1 (0x01 << 1)
#define BIT_PAGE_5_CBUS_RX_DISC_INT0_MASK_CBUS_RXDISC_INTR0_MASK0 (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xA0 CBUS_Link_Layer Control #1 Register                (Default: 0x18)    */
#define REG_PAGE_5_CBUS_LINK_CONTROL_1                     TX_PAGE_5, 0xA0

#define MSK_PAGE_5_CBUS_LINK_CONTROL_1_LNK_PACKET_LIMIT    (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0xA1 CBUS_Link_Layer Control #2 Register                (Default: 0x22)    */
#define REG_PAGE_5_CBUS_LINK_CONTROL_2                     TX_PAGE_5, 0xA1

#define MSK_PAGE_5_CBUS_LINK_CONTROL_2_LNK_GLITCH_FILTER_SEL (0x0F << 4)
#define MSK_PAGE_5_CBUS_LINK_CONTROL_2_LNK_INI_TIMEOUT_MAX (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xA2 CBUS_Link_Layer Control #3 Register                (Default: 0xC5)    */
#define REG_PAGE_5_CBUS_LINK_CONTROL_3                     TX_PAGE_5, 0xA2

#define BIT_PAGE_5_CBUS_LINK_CONTROL_3_LNK_SYNC_FILTER_EN  (0x01 << 7)
#define BIT_PAGE_5_CBUS_LINK_CONTROL_3_LNK_ARB_ID          (0x01 << 6)
#define MSK_PAGE_5_CBUS_LINK_CONTROL_3_LNK_FWR_ACK_OFFSET  (0x07 << 3)
#define MSK_PAGE_5_CBUS_LINK_CONTROL_3_LNK_RCV_BIT_CHECK_OPT (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0xA3 CBUS_Link_Layer Control #4 Register                (Default: 0xE2)    */
#define REG_PAGE_5_CBUS_LINK_CONTROL_4                     TX_PAGE_5, 0xA3

#define MSK_PAGE_5_CBUS_LINK_CONTROL_4_LNK_RCV_OPP_TIMEOUT_MAX (0x1F << 3)
#define MSK_PAGE_5_CBUS_LINK_CONTROL_4_LNK_XMIT_OPP_TIMEOUT_MAX (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0xA4 CBUS_Link_Layer Control #5 Register                (Default: 0x2C)    */
#define REG_PAGE_5_CBUS_LINK_CONTROL_5                     TX_PAGE_5, 0xA4

#define MSK_PAGE_5_CBUS_LINK_CONTROL_5_LNK_LOW_TIMEOUT_MAX_MSB (0x0F << 4)
#define MSK_PAGE_5_CBUS_LINK_CONTROL_5_LNK_HIGH_TIMEOUT_MAX_MSB (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xA5 CBUS_Link_Layer Control #6 Register                (Default: 0x12)    */
#define REG_PAGE_5_CBUS_LINK_CONTROL_6                     TX_PAGE_5, 0xA5

#define MSK_PAGE_5_CBUS_LINK_CONTROL_6_LNK_CHECK_HIGH_LIMIT (0x07 << 3)
#define MSK_PAGE_5_CBUS_LINK_CONTROL_6_LNK_DRV_HIGH_LIMIT  (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0xA6 CBUS_Link_Layer Control #7 Register                (Default: 0x30)    */
#define REG_PAGE_5_CBUS_LINK_CONTROL_7                     TX_PAGE_5, 0xA6

#define BIT_PAGE_5_CBUS_LINK_CONTROL_7_LNK_XMIT_BIT_TIME_SEL0 (0x01 << 5)
#define MSK_PAGE_5_CBUS_LINK_CONTROL_7_LNK_XMIT_IDLE_TIMEOUT (0x1F << 0)

/*----------------------------------------------------------------------------*/
/* 0xA7 CBUS_Link_Layer Control #8 Register                (Default: 0x00)    */
#define REG_PAGE_5_CBUS_LINK_CONTROL_8                     TX_PAGE_5, 0xA7

#define MSK_PAGE_5_CBUS_LINK_CONTROL_8_LNK_XMIT_BIT_TIME   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xA8 CBUS_Link_Layer Control #9 Register                (Default: 0x3F)    */
#define REG_PAGE_5_CBUS_LINK_CONTROL_9                     TX_PAGE_5, 0xA8

#define BIT_PAGE_5_CBUS_LINK_CONTROL_9_LNK_XMIT_BIT_TIME_SEL1 (0x01 << 7)
#define BIT_PAGE_5_CBUS_LINK_CONTROL_9_LNK_INV_POLARITY    (0x01 << 6)
#define MSK_PAGE_5_CBUS_LINK_CONTROL_9_LNK_CAL_FIFO_DEPTH  (0x07 << 3)
#define MSK_PAGE_5_CBUS_LINK_CONTROL_9_LNK_CAL_BYTE_CNT_MIN (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0xA9 CBUS_Link_Layer Control #10 Register               (Default: 0xAB)    */
#define REG_PAGE_5_CBUS_LINK_CONTROL_10                    TX_PAGE_5, 0xA9

#define BIT_PAGE_5_CBUS_LINK_CONTROL_10_LNK_CAL_ERROR_CHECK (0x01 << 7)
#define BIT_PAGE_5_CBUS_LINK_CONTROL_10_LNK_RCV_BIT_TIME_SEL (0x01 << 6)
#define MSK_PAGE_5_CBUS_LINK_CONTROL_10_LNK_CAL_BIT_OFFSET (0x07 << 3)
#define MSK_PAGE_5_CBUS_LINK_CONTROL_10_LNK_CAL_SYNC_OFFSET (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0xAA CBUS_Link_Layer Control #11 Register               (Default: 0x81)    */
#define REG_PAGE_5_CBUS_LINK_CONTROL_11                    TX_PAGE_5, 0xAA

#define MSK_PAGE_5_CBUS_LINK_CONTROL_11_LNK_XMIT_RETRY_LIMIT (0x3F << 2)
#define BIT_PAGE_5_CBUS_LINK_CONTROL_11_LNK_NORM_BDY_CTL   (0x01 << 1)
#define BIT_PAGE_5_CBUS_LINK_CONTROL_11_LNK_CAL_BDY_CTL    (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xAB CBUS Ack Control Register                          (Default: 0x03)    */
#define REG_PAGE_5_CBUS_ACK_CONTROL                        TX_PAGE_5, 0xAB

#define MSK_PAGE_5_CBUS_ACK_CONTROL_LNK_ACK_WIN_WIDTH      (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0xAC CBUS Calibration Control Register                  (Default: 0x50)    */
#define REG_PAGE_5_CBUS_CAL_CONTROL                        TX_PAGE_5, 0xAC

#define BIT_PAGE_5_CBUS_CAL_CONTROL_LNK_EN_CAL_BY_NACK     (0x01 << 6)
#define MSK_PAGE_5_CBUS_CAL_CONTROL_LNK_NACK_CNT_MAX       (0x3F << 0)

/*----------------------------------------------------------------------------*/
/* 0xAD CBUS Link Layer Status #1 Register                 (Default: 0x1F)    */
#define REG_PAGE_5_CBUS_LINK_STATUS_0                      TX_PAGE_5, 0xAD

#define MSK_PAGE_5_CBUS_LINK_STATUS_0_LNK_RCV_BIT_TIME     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xAE CBUS Xmit ERR CNT Register                         (Default: 0x00)    */
#define REG_PAGE_5_CBUS_XMIT_ERR_CNT                       TX_PAGE_5, 0xAE

#define MSK_PAGE_5_CBUS_XMIT_ERR_CNT_LNK_XMIT_ERR_COUNT    (0x7F << 0)

/*----------------------------------------------------------------------------*/
/* 0xAF CBUS Bi-Phase ERR CNT Register                     (Default: 0x00)    */
#define REG_PAGE_5_CBUS_BIPHZ_ERR_CNT                      TX_PAGE_5, 0xAF

#define MSK_PAGE_5_CBUS_BIPHZ_ERR_CNT_LNK_NORM_BIPHZ_ERR_CNT (0x0F << 4)
#define MSK_PAGE_5_CBUS_BIPHZ_ERR_CNT_LNK_CAL_BIPHZ_ERR_CNT (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xB0 CBUS Parity ERR CNT Register                       (Default: 0x00)    */
#define REG_PAGE_5_CBUS_PARITY_ERR_CNT                     TX_PAGE_5, 0xB0

#define MSK_PAGE_5_CBUS_PARITY_ERR_CNT_LNK_RCV_ERR_COUNT   (0x0F << 4)
#define MSK_PAGE_5_CBUS_PARITY_ERR_CNT_LNK_CAL_PARITY_ERR_CNT (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xB1 CBUS_Link_Layer Sync Filter Control #1 Register    (Default: 0x52)    */
#define REG_PAGE_5_CBUS_LINK_SYNC_CONTROL_1                TX_PAGE_5, 0xB1

#define MSK_PAGE_5_CBUS_LINK_SYNC_CONTROL_1_LNK_SYNC_LOW_MAX_OFFSET (0x0F << 4)
#define MSK_PAGE_5_CBUS_LINK_SYNC_CONTROL_1_LNK_SYNC_LOW_MIN_OFFSET (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xB2 CBUS_Link_Layer Sync Filter Control #2 Register    (Default: 0x71)    */
#define REG_PAGE_5_CBUS_LINK_SYNC_CONTROL_2                TX_PAGE_5, 0xB2

#define MSK_PAGE_5_CBUS_LINK_SYNC_CONTROL_2_LNK_SYNC_BIT_MAX_OFFSET (0x0F << 4)
#define MSK_PAGE_5_CBUS_LINK_SYNC_CONTROL_2_LNK_SYNC_BIT_MIN_OFFSET (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xB5 MDT State Machine Status Register                  (Default: 0x00)    */
#define REG_PAGE_5_MDT_SM_STAT                             TX_PAGE_5, 0xB5

#define MSK_PAGE_5_MDT_SM_STAT_MDT_RCV_STATE               (0x0F << 4)
#define MSK_PAGE_5_MDT_SM_STAT_MDT_XMIT_STATE              (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xB6 MDT Peer Adopter ID Low Byte Register              (Default: 0x00)    */
#define REG_PAGE_5_MDT_PEER_ADOPER_ID_LOW                  TX_PAGE_5, 0xB6

#define MSK_PAGE_5_MDT_PEER_ADOPER_ID_LOW_PEER_ADOPTER_ID_7_0 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB7 MDT Peer Adopter ID High Byte Register             (Default: 0x00)    */
#define REG_PAGE_5_MDT_PEER_ADOPER_ID_HIGH                 TX_PAGE_5, 0xB7

#define MSK_PAGE_5_MDT_PEER_ADOPER_ID_HIGH_PEER_ADOPTER_ID_15_8 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xB8 CBUS MSC command trigger Register                  (Default: 0x00)    */
#define REG_PAGE_5_MSC_COMMAND_START                       TX_PAGE_5, 0xB8

#define BIT_PAGE_5_MSC_COMMAND_START_MSC_DEBUG_CMD         (0x01 << 5)
#define BIT_PAGE_5_MSC_COMMAND_START_MSC_WRITE_BURST_CMD   (0x01 << 4)
#define BIT_PAGE_5_MSC_COMMAND_START_MSC_WRITE_STAT_CMD    (0x01 << 3)
#define BIT_PAGE_5_MSC_COMMAND_START_MSC_READ_DEVCAP_CMD   (0x01 << 2)
#define BIT_PAGE_5_MSC_COMMAND_START_MSC_MSC_MSG_CMD       (0x01 << 1)
#define BIT_PAGE_5_MSC_COMMAND_START_MSC_PEER_CMD          (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xB9 CBUS MSC Command/Offset Register                   (Default: 0x00)    */
#define REG_PAGE_5_MSC_CMD_OR_OFFSET                       TX_PAGE_5, 0xB9

#define MSK_PAGE_5_MSC_CMD_OR_OFFSET_MSC_MT_CMD_OR_OFFSET  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBA CBUS MSC 1st Transmit Data Register                (Default: 0x00)    */
#define REG_PAGE_5_MSC_1ST_TRANSMIT_DATA                   TX_PAGE_5, 0xBA

#define MSK_PAGE_5_MSC_1ST_TRANSMIT_DATA_MSC_MT_DATA0      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBB CBUS MSC 2nd Transmit Data Register                (Default: 0x00)    */
#define REG_PAGE_5_MSC_2ND_TRANSMIT_DATA                   TX_PAGE_5, 0xBB

#define MSK_PAGE_5_MSC_2ND_TRANSMIT_DATA_MSC_MT_DATA1      (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBC CBUS MSC Requester Received 1st Data Register      (Default: 0x00)    */
#define REG_PAGE_5_MSC_MT_RCVD_DATA0                       TX_PAGE_5, 0xBC

#define MSK_PAGE_5_MSC_MT_RCVD_DATA0_MSC_MT_RCVD_DATA0     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBD CBUS MSC Requester Received 2nd Data Register      (Default: 0x00)    */
#define REG_PAGE_5_MSC_MT_RCVD_DATA1                       TX_PAGE_5, 0xBD

#define MSK_PAGE_5_MSC_MT_RCVD_DATA1_MSC_MT_RCVD_DATA1     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBE CBUS MSC Requester Received State Register         (Default: 0x00)    */
#define REG_PAGE_5_MSC_MT_RCVD_STATE                       TX_PAGE_5, 0xBE

#define MSK_PAGE_5_MSC_MT_RCVD_STATE_MSC_MT_RCVD_STATE     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xBF CBUS MSC Responder MSC_MSG Received 1st Data Register (Default: 0x00)    */
#define REG_PAGE_5_MSC_MR_MSC_MSG_RCVD_1ST_DATA            TX_PAGE_5, 0xBF

#define MSK_PAGE_5_MSC_MR_MSC_MSG_RCVD_1ST_DATA_MSC_MR_MSC_MSG_RCVD_DATA0 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC0 CBUS MSC Responder MSC_MSG Received Data Register  (Default: 0x00)    */
#define REG_PAGE_5_MSC_MR_MSC_MSG_RCVD_2ND_DATA            TX_PAGE_5, 0xC0

#define MSK_PAGE_5_MSC_MR_MSC_MSG_RCVD_2ND_DATA_MSC_MR_MSC_MSG_RCVD_DATA1 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC1 CBUS MSC Responder Received Offset Register        (Default: 0x00)    */
#define REG_PAGE_5_MSC_MR_RCVD_OFFSET                      TX_PAGE_5, 0xC1

#define MSK_PAGE_5_MSC_MR_RCVD_OFFSET_MSC_MR_RCVD_OFFSET   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC2 CBUS MSC Responder Rcvd Data Register              (Default: 0x00)    */
#define REG_PAGE_5_MSC_MR_RCVD_DATA                        TX_PAGE_5, 0xC2

#define MSK_PAGE_5_MSC_MR_RCVD_DATA_MSC_MR_RCVD_DATA       (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC3 CBUS MSC Rcvd Data Cnt Register                    (Default: 0x00)    */
#define REG_PAGE_5_MSC_RCVD_DATA_CNT                       TX_PAGE_5, 0xC3

#define MSK_PAGE_5_MSC_RCVD_DATA_CNT_MSC_MR_RCVD_DATA_CNT  (0x1F << 0)

/*----------------------------------------------------------------------------*/
/* 0xC4 CBUS MSC Heartbeat Control Register                (Default: 0x78)    */
#define REG_PAGE_5_MSC_HEARTBEAT_CONTROL                   TX_PAGE_5, 0xC4

#define BIT_PAGE_5_MSC_HEARTBEAT_CONTROL_MSC_HB_EN         (0x01 << 7)
#define MSK_PAGE_5_MSC_HEARTBEAT_CONTROL_MSC_HB_FAIL_LIMIT (0x07 << 4)
#define MSK_PAGE_5_MSC_HEARTBEAT_CONTROL_MSC_HB_PERIOD_MSB (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xC5 CBUS MSC Heartbeat Fail Count Register             (Default: 0x00)    */
#define REG_PAGE_5_MSC_HEARTBEAT_FAIL_CNT                  TX_PAGE_5, 0xC5

#define MSK_PAGE_5_MSC_HEARTBEAT_FAIL_CNT_MSC_HB_FAIL_CNT  (0x1F << 0)

/*----------------------------------------------------------------------------*/
/* 0xC6 CBUS MSC Write_Burst length Register               (Default: 0x0F)    */
#define REG_PAGE_5_MSC_WRITE_BURST_DATA_LEN                TX_PAGE_5, 0xC6

#define MSK_PAGE_5_MSC_WRITE_BURST_DATA_LEN_MSC_WRITE_BURST_LEN (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xC7 CBUS MSC Compatibility Control Register            (Default: 0x12)    */
#define REG_PAGE_5_CBUS_MSC_COMPATIBILITY_CONTROL          TX_PAGE_5, 0xC7

#define BIT_PAGE_5_CBUS_MSC_COMPATIBILITY_CONTROL_DISABLE_MSC_ON_CBUS (0x01 << 6)
#define BIT_PAGE_5_CBUS_MSC_COMPATIBILITY_CONTROL_DISABLE_DDC_ON_CBUS (0x01 << 5)
#define BIT_PAGE_5_CBUS_MSC_COMPATIBILITY_CONTROL_DISABLE_GET_DDC_ERRORCODE (0x01 << 3)
#define BIT_PAGE_5_CBUS_MSC_COMPATIBILITY_CONTROL_DISABLE_GET_VS1_ERRORCODE (0x01 << 2)
#define BIT_PAGE_5_CBUS_MSC_COMPATIBILITY_CONTROL_DISABLE_CAP_ID_COMMANDS (0x01 << 1)

/*----------------------------------------------------------------------------*/
/* 0xC8 CBUS CEC Capture ID Low Byte Register              (Default: 0x00)    */
#define REG_PAGE_5_CEC_CAPTURE_ID_LOW                      TX_PAGE_5, 0xC8

#define MSK_PAGE_5_CEC_CAPTURE_ID_LOW_CEC_CAPTURE_ID_7_0   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xC9 CBUS CEC Capture ID High Byte Register             (Default: 0x00)    */
#define REG_PAGE_5_CEC_CAPTURE_ID_HIGH                     TX_PAGE_5, 0xC9

#define MSK_PAGE_5_CEC_CAPTURE_ID_HIGH_CEC_CAPTURE_ID_15_8 (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCA CBUS CEC Abort Reason Register                     (Default: 0x00)    */
#define REG_PAGE_5_CEC_ERRORCODE                           TX_PAGE_5, 0xCA

#define MSK_PAGE_5_CEC_ERRORCODE_CEC_ERRORCODE             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCB CBUS DDC Abort Reason Register                     (Default: 0x00)    */
#define REG_PAGE_5_DDC_ERRORCODE                           TX_PAGE_5, 0xCB

#define MSK_PAGE_5_DDC_ERRORCODE_DDC_ERRORCODE             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCC CBUS MSC Abort Reason Register                     (Default: 0x00)    */
#define REG_PAGE_5_MSC_ERRORCODE                           TX_PAGE_5, 0xCC

#define MSK_PAGE_5_MSC_ERRORCODE_MSC_ERRORCODE             (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xCE CBUS DDC Fail Limit Register                       (Default: 0x05)    */
#define REG_PAGE_5_CBUS_DDC_FAIL_LIMIT                     TX_PAGE_5, 0xCE

#define MSK_PAGE_5_CBUS_DDC_FAIL_LIMIT_DDC_RETRY_FAIL_LIMIT (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xCF CBUS MSC Fail Limit Register                       (Default: 0x05)    */
#define REG_PAGE_5_CBUS_MSC_FAIL_LIMIT                     TX_PAGE_5, 0xCF

#define MSK_PAGE_5_CBUS_MSC_FAIL_LIMIT_MSC_RETRY_FAIL_LIMIT (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xD1 CBUS DDC Time Out Register                         (Default: 0x0F)    */
#define REG_PAGE_5_DDC_TIMEOUT                             TX_PAGE_5, 0xD1

#define MSK_PAGE_5_DDC_TIMEOUT_DDC_TIMEOUT_MAX_MSB         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD2 CBUS MSC Time Out Register                         (Default: 0x07)    */
#define REG_PAGE_5_MSC_TIMEOUT                             TX_PAGE_5, 0xD2

#define MSK_PAGE_5_MSC_TIMEOUT_MSC_TIMEOUT_MAX_MSB         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD4 CBUS DDC Retry Interval Register                   (Default: 0x14)    */
#define REG_PAGE_5_DDC_RETRY_INTERVAL                      TX_PAGE_5, 0xD4

#define MSK_PAGE_5_DDC_RETRY_INTERVAL_DDC_RETRY_MAX_MSB    (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD5 CBUS MSC Retry Interval Register                   (Default: 0x14)    */
#define REG_PAGE_5_MSC_RETRY_INTERVAL                      TX_PAGE_5, 0xD5

#define MSK_PAGE_5_MSC_RETRY_INTERVAL_MSC_RETRY_MAX_MSB    (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD6 CBUS DDC Master Speed Control Register             (Default: 0x40)    */
#define REG_PAGE_5_CBUS_DDC_SPEED_CONTROL                  TX_PAGE_5, 0xD6

#define MSK_PAGE_5_CBUS_DDC_SPEED_CONTROL_DDC_SPEED_CTRL   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xD7 CBUS DDC START STOP Timing Control Register        (Default: 0x63)    */
#define REG_PAGE_5_CBUS_DDC_ST_SP_TIMING_CONTROL           TX_PAGE_5, 0xD7

#define MSK_PAGE_5_CBUS_DDC_ST_SP_TIMING_CONTROL_DDC_SP_TIMING_CTRL (0x0F << 4)
#define MSK_PAGE_5_CBUS_DDC_ST_SP_TIMING_CONTROL_DDC_ST_TIMING_CTRL (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xD8 CBUS MISC Control Register                         (Default: 0x0D)    */
#define REG_PAGE_5_CBUS_MISC_CONTROL                       TX_PAGE_5, 0xD8

#define BIT_PAGE_5_CBUS_MISC_CONTROL_SLOW_SL_ENH           (0x01 << 3)
#define BIT_PAGE_5_CBUS_MISC_CONTROL_DEV_CAT_OVR           (0x01 << 2)
#define BIT_PAGE_5_CBUS_MISC_CONTROL_RI_PREFETCH_EN        (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xD9 CBUS MISC Control Register                         (Default: 0x00)    */
#define REG_PAGE_5_CBUS_RCVD_PAYLOAD                       TX_PAGE_5, 0xD9

#define MSK_PAGE_5_CBUS_RCVD_PAYLOAD_CBUS_RCVD_PAYLOAD     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xDA CBUS MISC Control Register                         (Default: 0x00)    */
#define REG_PAGE_5_CBUS_RCVD_HC                            TX_PAGE_5, 0xDA

#define MSK_PAGE_5_CBUS_RCVD_HC_CBUS_RCVD_HC               (0x07 << 0)

/*----------------------------------------------------------------------------*/
/* 0xDB CBUS DDC EDID Slave Address Register               (Default: 0xA0)    */
#define REG_PAGE_5_CBUS_DDC_EDID                           TX_PAGE_5, 0xDB

#define MSK_PAGE_5_CBUS_DDC_EDID_CBUS_DDC_EDID_ADDR        (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xDC CBUS3 Converter Control Register                   (Default: 0x84)    */
#define REG_PAGE_5_CBUS3_CNVT                              TX_PAGE_5, 0xDC

#define MSK_PAGE_5_CBUS3_CNVT_CBUS3_RETRYLMT               (0x0F << 4)
#define MSK_PAGE_5_CBUS3_CNVT_CBUS3_PEERTOUT_SEL           (0x03 << 2)
#define BIT_PAGE_5_CBUS3_CNVT_TEARCBUS_EN                  (0x01 << 1)
#define BIT_PAGE_5_CBUS3_CNVT_CBUS3CNVT_EN                 (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xDD CBUS3 Converter INTR Register                      (Default: 0x00)    */
#define REG_PAGE_5_CBUS3_INTR                              TX_PAGE_5, 0xDD

#define BIT_PAGE_5_CBUS3_INTR_CBUS3_INTR_1                 (0x01 << 1)
#define BIT_PAGE_5_CBUS3_INTR_CBUS3_INTR_0                 (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xDE CBUS3 Converter INTRMASK Register                  (Default: 0x00)    */
#define REG_PAGE_5_CBUS3_INTRMASK                          TX_PAGE_5, 0xDE

#define BIT_PAGE_5_CBUS3_INTRMASK_CBUS3_INTRMASK_1         (0x01 << 1)
#define BIT_PAGE_5_CBUS3_INTRMASK_CBUS3_INTRMASK_0         (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xDF CBUS3 Converter Status Register                    (Default: 0x80)    */
#define REG_PAGE_5_CBUS3_STATUS                            TX_PAGE_5, 0xDF

#define BIT_PAGE_5_CBUS3_STATUS_TDM8BIF_EN                 (0x01 << 7)
#define MSK_PAGE_5_CBUS3_STATUS_CBUS3_STATE                (0x1F << 0)

/*----------------------------------------------------------------------------*/
/* 0xE0 Discovery Control1 Register                        (Default: 0x26)    */
#define REG_PAGE_5_DISC_CTRL1                              TX_PAGE_5, 0xE0

#define BIT_PAGE_5_DISC_CTRL1_CBUS_INTR_EN                 (0x01 << 7)
#define BIT_PAGE_5_DISC_CTRL1_HB_EN                        (0x01 << 6)
#define MSK_PAGE_5_DISC_CTRL1_DISC_ATT                     (0x03 << 4)
#define MSK_PAGE_5_DISC_CTRL1_DISC_CYC                     (0x03 << 2)
#define BIT_PAGE_5_DISC_CTRL1_PON_N                        (0x01 << 1)
#define BIT_PAGE_5_DISC_CTRL1_DISC_EN                      (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xE1 Discovery Control2 Register                        (Default: 0x88)    */
#define REG_PAGE_5_DISC_CTRL2                              TX_PAGE_5, 0xE1

#define BIT_PAGE_5_DISC_CTRL2_DLYTRG_EN                    (0x01 << 7)
#define BIT_PAGE_5_DISC_CTRL2_SKIP_RGND                    (0x01 << 6)
#define BIT_PAGE_5_DISC_CTRL2_HW_HB_EN                     (0x01 << 4)
#define BIT_PAGE_5_DISC_CTRL2_CBUS100K_EN                  (0x01 << 3)

/*----------------------------------------------------------------------------*/
/* 0xE2 Discovery Control3 Register                        (Default: 0x80)    */
#define REG_PAGE_5_DISC_CTRL3                              TX_PAGE_5, 0xE2

#define BIT_PAGE_5_DISC_CTRL3_COMM_IMME                    (0x01 << 7)
#define BIT_PAGE_5_DISC_CTRL3_FORCE_MHL                    (0x01 << 6)
#define BIT_PAGE_5_DISC_CTRL3_DISC_SIMODE                  (0x01 << 5)

/*----------------------------------------------------------------------------*/
/* 0xE3 Discovery Control4 Register                        (Default: 0x80)    */
#define REG_PAGE_5_DISC_CTRL4                              TX_PAGE_5, 0xE3

#define MSK_PAGE_5_DISC_CTRL4_CBUSDISC_PUP_SEL             (0x03 << 6)
#define MSK_PAGE_5_DISC_CTRL4_CBUSIDLE_PUP_SEL             (0x03 << 4)

/*----------------------------------------------------------------------------*/
/* 0xE4 Discovery Control5 Register                        (Default: 0x57)    */
#define REG_PAGE_5_DISC_CTRL5                              TX_PAGE_5, 0xE4

#define BIT_PAGE_5_DISC_CTRL5_DSM_OVRIDE                   (0x01 << 3)
#define MSK_PAGE_5_DISC_CTRL5_CBUSMHL_PUP_SEL              (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0xE5 Discovery Control6 Register                        (Default: 0x11)    */
#define REG_PAGE_5_DISC_CTRL6                              TX_PAGE_5, 0xE5

#define BIT_PAGE_5_DISC_CTRL6_BLOCK_RGNDINT                (0x01 << 4)
#define BIT_PAGE_5_DISC_CTRL6_CI2CA_POL                    (0x01 << 2)
#define BIT_PAGE_5_DISC_CTRL6_CI2CA_WKUP                   (0x01 << 1)
#define BIT_PAGE_5_DISC_CTRL6_SINGLE_ATT                   (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xE6 Discovery Control7 Register                        (Default: 0x00)    */
#define REG_PAGE_5_DISC_CTRL7                              TX_PAGE_5, 0xE6

#define BIT_PAGE_5_DISC_CTRL7_CBUSLVL_VAL                  (0x01 << 7)
#define BIT_PAGE_5_DISC_CTRL7_CBUSLVL_SW                   (0x01 << 6)
#define BIT_PAGE_5_DISC_CTRL7_CBUS_DBGMODE                 (0x01 << 3)
#define MSK_PAGE_5_DISC_CTRL7_RGND_INTP                    (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0xE7 Discovery Control8 Register                        (Default: 0x81)    */
#define REG_PAGE_5_DISC_CTRL8                              TX_PAGE_5, 0xE7

#define BIT_PAGE_5_DISC_CTRL8_NOMHLINT_CLR_BYPASS          (0x01 << 7)
#define BIT_PAGE_5_DISC_CTRL8_DELAY_CBUS_INTR_EN           (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xE8 Discovery Control9 Register                        (Default: 0x54)    */
#define REG_PAGE_5_DISC_CTRL9                              TX_PAGE_5, 0xE8

#define BIT_PAGE_5_DISC_CTRL9_MHL3_RSEN_BYP                (0x01 << 7)
#define BIT_PAGE_5_DISC_CTRL9_MHL3DISC_EN                  (0x01 << 6)
#define BIT_PAGE_5_DISC_CTRL9_WAKE_DRVFLT                  (0x01 << 4)
#define BIT_PAGE_5_DISC_CTRL9_NOMHL_EST                    (0x01 << 3)
#define BIT_PAGE_5_DISC_CTRL9_DISC_PULSE_PROCEED           (0x01 << 2)
#define BIT_PAGE_5_DISC_CTRL9_WAKE_PULSE_BYPASS            (0x01 << 1)
#define BIT_PAGE_5_DISC_CTRL9_VBUS_OUTPUT_CAPABILITY_SRC   (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xE9 Discovery Control10 Register                       (Default: 0x8C)    */
#define REG_PAGE_5_DISC_CTRL10                             TX_PAGE_5, 0xE9

#define MSK_PAGE_5_DISC_CTRL10_TSRC_WAKE_PULSE_WIDTH_1     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xEA Discovery Control11 Register                       (Default: 0x7A)    */
#define REG_PAGE_5_DISC_CTRL11                             TX_PAGE_5, 0xEA

#define MSK_PAGE_5_DISC_CTRL11_TSRC_WAKE_PULSE_WIDTH_2     (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xEB Discovery Status1 Register                         (Default: 0x00)    */
#define REG_PAGE_5_DISC_STAT1                              TX_PAGE_5, 0xEB

#define BIT_PAGE_5_DISC_STAT1_PSM_OVRIDE                   (0x01 << 5)
#define MSK_PAGE_5_DISC_STAT1_DISC_SM                      (0x0F << 0)

/*----------------------------------------------------------------------------*/
/* 0xEC Discovery Status2 Register                         (Default: 0x00)    */
#define REG_PAGE_5_DISC_STAT2                              TX_PAGE_5, 0xEC

#define BIT_PAGE_5_DISC_STAT2_CBUS_OE_POL                  (0x01 << 6)
#define BIT_PAGE_5_DISC_STAT2_RSEN                         (0x01 << 4)

#define MSK_PAGE_5_DISC_STAT2_MHL_VRSN                     (0x03 << 2)
#define VAL_PAGE_5_DISC_STAT2_DEFAULT                      (0x00 << 2)
#define VAL_PAGE_5_DISC_STAT2_MHL1_2                       (0x01 << 2)
#define VAL_PAGE_5_DISC_STAT2_MHL3                         (0x02 << 2)
#define VAL_PAGE_5_DISC_STAT2_RESERVED                     (0x03 << 2)

#define MSK_PAGE_5_DISC_STAT2_RGND                         (0x03 << 0)
#define VAL_PAGE_5_RGND_OPEN                               (0x00 << 0)
#define VAL_PAGE_5_RGND_2K                                 (0x01 << 0)
#define VAL_PAGE_5_RGND_1K                                 (0x02 << 0)
#define VAL_PAGE_5_RGND_SHORT                              (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0xED Interrupt CBUS_reg1 INTR0 Register                 (Default: 0x00)    */
#define REG_PAGE_5_CBUS_DISC_INTR0                         TX_PAGE_5, 0xED

#define BIT_PAGE_5_CBUS_DISC_INTR0_CBUS_DISC_INTR0_STAT7   (0x01 << 7)
#define BIT_PAGE_5_CBUS_DISC_INTR0_CBUS_DISC_INTR0_STAT6   (0x01 << 6)
#define BIT_PAGE_5_CBUS_DISC_INTR0_CBUS_DISC_INTR0_STAT5   (0x01 << 5)
#define BIT_PAGE_5_CBUS_DISC_INTR0_CBUS_DISC_INTR0_STAT4   (0x01 << 4)
#define BIT_PAGE_5_CBUS_DISC_INTR0_CBUS_DISC_INTR0_STAT3   (0x01 << 3)
#define BIT_PAGE_5_CBUS_DISC_INTR0_CBUS_DISC_INTR0_STAT2   (0x01 << 2)
#define BIT_PAGE_5_CBUS_DISC_INTR0_CBUS_DISC_INTR0_STAT1   (0x01 << 1)

/*----------------------------------------------------------------------------*/
/* 0xEE Interrupt CBUS_reg1 INTR0 Mask Register            (Default: 0x00)    */
#define REG_PAGE_5_CBUS_DISC_INTR0_MASK                    TX_PAGE_5, 0xEE

#define BIT_PAGE_5_CBUS_DISC_INTR0_MASK_CBUS_DISC_INTR0_MASK7 (0x01 << 7)
#define BIT_PAGE_5_CBUS_DISC_INTR0_MASK_CBUS_DISC_INTR0_MASK6 (0x01 << 6)
#define BIT_PAGE_5_CBUS_DISC_INTR0_MASK_CBUS_DISC_INTR0_MASK5 (0x01 << 5)
#define BIT_PAGE_5_CBUS_DISC_INTR0_MASK_CBUS_DISC_INTR0_MASK4 (0x01 << 4)
#define BIT_PAGE_5_CBUS_DISC_INTR0_MASK_CBUS_DISC_INTR0_MASK3 (0x01 << 3)
#define BIT_PAGE_5_CBUS_DISC_INTR0_MASK_CBUS_DISC_INTR0_MASK2 (0x01 << 2)
#define BIT_PAGE_5_CBUS_DISC_INTR0_MASK_CBUS_DISC_INTR0_MASK1 (0x01 << 1)

/*----------------------------------------------------------------------------*/
/* 0xEF MHL TX Test Control Register                       (Default: 0x00)    */
#define REG_PAGE_5_MHLTX_TESTCTRL                          TX_PAGE_5, 0xEF

#define BIT_PAGE_5_MHLTX_TESTCTRL_EVAL_MODE                (0x01 << 5)

/*----------------------------------------------------------------------------*/
/* 0xF0 CBUS HW HPD Retry Timer Register                   (Default: 0x49)    */
#define REG_PAGE_5_CBUS_HPD_RETRY_TIMER                    TX_PAGE_5, 0xF0

#define MSK_PAGE_5_CBUS_HPD_RETRY_TIMER_HW_HPD_RETRY_MAX_MSB (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xF1 CBUS HW HPD Retry Limit Register                   (Default: 0x10)    */
#define REG_PAGE_5_CBUS_HPD_RETRY_CNT                      TX_PAGE_5, 0xF1

#define MSK_PAGE_5_CBUS_HPD_RETRY_CNT_HW_HPD_RETRY_LIMIT   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xF2 CBUS HW HPD Enable Register                        (Default: 0x01)    */
#define REG_PAGE_5_CBUS_HW_HPD_EN                          TX_PAGE_5, 0xF2

#define BIT_PAGE_5_CBUS_HW_HPD_EN_HW_HPD_EN                (0x01 << 0)

/*----------------------------------------------------------------------------*/
/* 0xF8 Debug Control Low Register                         (Default: 0x00)    */
#define REG_PAGE_5_DBG_CTL_L                               TX_PAGE_5, 0xF8

#define MSK_PAGE_5_DBG_CTL_L_DBG_CTL_7_0                   (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xF9 Debug Control High Register                        (Default: 0x00)    */
#define REG_PAGE_5_DBG_CTL_H                               TX_PAGE_5, 0xF9

#define MSK_PAGE_5_DBG_CTL_H_DBG_CTL_15_8                  (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xFA Debug External Status Register                     (Default: 0x00)    */
#define REG_PAGE_5_DBG_EXT_STAT                            TX_PAGE_5, 0xFA

#define MSK_PAGE_5_DBG_EXT_STAT_DBG_EXT_STAT               (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xFB Debug Internal Status Select Register              (Default: 0x00)    */
#define REG_PAGE_5_DBG_INT_STAT_SEL                        TX_PAGE_5, 0xFB

#define MSK_PAGE_5_DBG_INT_STAT_SEL_DBG_INT_STAT_SEL       (0x03 << 0)

/*----------------------------------------------------------------------------*/
/* 0xFC Debug Internal Status Low Register                 (Default: 0x00)    */
#define REG_PAGE_5_DBG_INT_STAT_L                          TX_PAGE_5, 0xFC

#define MSK_PAGE_5_DBG_INT_STAT_L_DBG_INT_STAT_7_0         (0xFF << 0)

/*----------------------------------------------------------------------------*/
/* 0xFD Debug Internal Status High Register                (Default: 0x00)    */
#define REG_PAGE_5_DBG_INT_STAT_H                          TX_PAGE_5, 0xFD

#define MSK_PAGE_5_DBG_INT_STAT_H_DBG_INT_STAT_15_8        (0xFF << 0)

#define CachedRegRead(reg)  cache_##reg
#define CachedRegWrite(reg, value) mhl_tx_reg_write(reg, cache_##reg=value);
#define CachedRegModify(reg, mask, value) \
{ \
uint8_t holder = cache_##reg; \
	cache_##reg &= ~(mask); \
	cache_##reg |= ((mask) & value); \
	if (holder != cache_##reg) \
	{ \
		mhl_tx_reg_write(reg, cache_##reg); \
	} \
}
