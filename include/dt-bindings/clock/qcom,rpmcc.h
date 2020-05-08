/*
 * Copyright 2015 Linaro Limited
 * Copyright (c) 2016-2017, The Linux Foundation. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _DT_BINDINGS_CLK_MSM_RPMCC_H
#define _DT_BINDINGS_CLK_MSM_RPMCC_H

/* apq8064 */
#define RPM_PXO_CLK				0
#define RPM_PXO_A_CLK				1
#define RPM_CXO_CLK				2
#define RPM_CXO_A_CLK				3
#define RPM_APPS_FABRIC_CLK			4
#define RPM_APPS_FABRIC_A_CLK			5
#define RPM_CFPB_CLK				6
#define RPM_CFPB_A_CLK				7
#define RPM_QDSS_CLK				8
#define RPM_QDSS_A_CLK				9
#define RPM_DAYTONA_FABRIC_CLK			10
#define RPM_DAYTONA_FABRIC_A_CLK		11
#define RPM_EBI1_CLK				12
#define RPM_EBI1_A_CLK				13
#define RPM_MM_FABRIC_CLK			14
#define RPM_MM_FABRIC_A_CLK			15
#define RPM_MMFPB_CLK				16
#define RPM_MMFPB_A_CLK				17
#define RPM_SYS_FABRIC_CLK			18
#define RPM_SYS_FABRIC_A_CLK			19
#define RPM_SFPB_CLK				20
#define RPM_SFPB_A_CLK				21

/* msm8916 and msm8996 */
#define RPM_SMD_XO_CLK_SRC			0
#define RPM_SMD_XO_A_CLK_SRC			1
#define RPM_SMD_PNOC_CLK			2
#define RPM_SMD_PNOC_A_CLK			3
#define RPM_SMD_SNOC_CLK			4
#define RPM_SMD_SNOC_A_CLK			5
#define RPM_SMD_BIMC_CLK			6
#define RPM_SMD_BIMC_A_CLK			7
#define RPM_SMD_QDSS_CLK			8
#define RPM_SMD_QDSS_A_CLK			9
#define RPM_SMD_BB_CLK1				11
#define RPM_SMD_BB_CLK1_A			12
#define RPM_SMD_BB_CLK1_PIN			13
#define RPM_SMD_BB_CLK1_A_PIN			14
#define RPM_SMD_BB_CLK2				15
#define RPM_SMD_BB_CLK2_A			16
#define RPM_SMD_BB_CLK2_PIN			17
#define RPM_SMD_BB_CLK2_A_PIN			18
#define RPM_SMD_RF_CLK1				19
#define RPM_SMD_RF_CLK1_A			20
#define RPM_SMD_RF_CLK1_PIN			21
#define RPM_SMD_RF_CLK1_A_PIN			22
#define RPM_SMD_RF_CLK2				23
#define RPM_SMD_RF_CLK2_A			24
#define RPM_SMD_RF_CLK2_PIN			25
#define RPM_SMD_RF_CLK2_A_PIN			26
#define RPM_SMD_RF_CLK3				27
#define RPM_SMD_RF_CLK3_A			28
#define RPM_SMD_RF_CLK3_PIN			29
#define RPM_SMD_RF_CLK3_A_PIN			30
#define RPM_SMD_AGGR1_NOC_CLK			31
#define RPM_SMD_AGGR1_NOC_A_CLK			32
#define RPM_SMD_AGGR2_NOC_CLK			33
#define RPM_SMD_AGGR2_NOC_A_CLK			34
#define RPM_SMD_CNOC_CLK			35
#define RPM_SMD_CNOC_A_CLK			36
#define RPM_SMD_MMAXI_CLK			37
#define RPM_SMD_MMAXI_A_CLK			38
#define RPM_SMD_IPA_CLK				39
#define RPM_SMD_IPA_A_CLK			40
#define RPM_SMD_QUP_CLK				41
#define RPM_SMD_QUP_A_CLK			42
#define RPM_SMD_MMRT_CLK			43
#define RPM_SMD_MMRT_A_CLK			44
#define RPM_SMD_MMNRT_CLK			45
#define RPM_SMD_MMNRT_A_CLK			46
#define RPM_SMD_SNOC_PERIPH_CLK			47
#define RPM_SMD_SNOC_PERIPH_A_CLK		48
#define RPM_SMD_SNOC_LPASS_CLK			49
#define RPM_SMD_SNOC_LPASS_A_CLK		50
#define RPM_SMD_CE1_CLK				51
#define RPM_SMD_CE1_A_CLK			52
#define RPM_SMD_DIV_CLK1			53
#define RPM_SMD_DIV_CLK1_AO			54
#define RPM_SMD_DIV_CLK2			55
#define RPM_SMD_DIV_CLK2_AO			56
#define RPM_SMD_DIV_CLK3			57
#define RPM_SMD_DIV_CLK3_AO			58
#define RPM_SMD_OCMEMGX_CLK			59
#define RPM_SMD_OCMEMGX_A_CLK			60
#define RPM_SMD_MMSSNOC_AHB_CLK			61
#define RPM_SMD_MMSSNOC_AHB_A_CLK		62
#define RPM_SMD_CXO_D0_PIN			63
#define RPM_SMD_CXO_D0_A_PIN			64
#define RPM_SMD_CXO_D1_PIN			65
#define RPM_SMD_CXO_D1_A_PIN			66
#define RPM_SMD_CXO_A0_PIN			67
#define RPM_SMD_CXO_A0_A_PIN			68
#define RPM_SMD_CXO_A1_PIN			69
#define RPM_SMD_CXO_A1_A_PIN			70
#define RPM_SMD_CXO_A2_PIN			71
#define RPM_SMD_CXO_A2_A_PIN			72
#define RPM_SMD_CXO_D0				73
#define RPM_SMD_CXO_D0_A			74
#define RPM_SMD_CXO_D1				75
#define RPM_SMD_CXO_D1_A			76
#define RPM_SMD_CXO_A0				77
#define RPM_SMD_CXO_A0_A			78
#define RPM_SMD_CXO_A1				79
#define RPM_SMD_CXO_A1_A			80
#define RPM_SMD_CXO_A2				81
#define RPM_SMD_CXO_A2_A			82
#define RPM_SMD_DIFF_CLK			83
#define RPM_SMD_DIFF_A_CLK			84
#define RPM_SMD_QPIC_CLK			85
#define RPM_SMD_QPIC_A_CLK			86
#define RPM_SMD_BIMC_GPU_CLK			87
#define RPM_SMD_BIMC_GPU_A_CLK			88
#define RPM_SMD_LN_BB_CLK			89
#define RPM_SMD_LN_BB_CLK_A			90
#define RPM_SMD_LN_BB_CLK1			91
#define RPM_SMD_LN_BB_CLK1_AO			92
#define RPM_SMD_LN_BB_CLK1_PIN			93
#define RPM_SMD_LN_BB_CLK1_PIN_AO		94
#define RPM_SMD_LN_BB_CLK2			95
#define RPM_SMD_LN_BB_CLK2_AO			96
#define RPM_SMD_LN_BB_CLK2_PIN			97
#define RPM_SMD_LN_BB_CLK2_PIN_AO		98
#define RPM_SMD_LN_BB_CLK3			99
#define RPM_SMD_LN_BB_CLK3_AO			100
#define RPM_SMD_LN_BB_CLK3_PIN			101
#define RPM_SMD_LN_BB_CLK3_PIN_AO		102
#define RPM_SMD_CNOC_PERIPH_CLK			103
#define RPM_SMD_CNOC_PERIPH_A_CLK		104

/* Voter clocks */
#define MMSSNOC_AXI_CLK				105
#define MMSSNOC_AXI_A_CLK			106
#define MMSSNOC_GDS_CLK				107
#define BIMC_MSMBUS_CLK				108
#define BIMC_MSMBUS_A_CLK			109
#define CNOC_MSMBUS_CLK				110
#define CNOC_MSMBUS_A_CLK			111
#define PNOC_KEEPALIVE_A_CLK			112
#define PNOC_MSMBUS_CLK				113
#define PNOC_MSMBUS_A_CLK			114
#define PNOC_PM_CLK				115
#define PNOC_SPS_CLK				116
#define MCD_CE1_CLK				117
#define QCEDEV_CE1_CLK				118
#define QCRYPTO_CE1_CLK				119
#define QSEECOM_CE1_CLK				120
#define SCM_CE1_CLK				121
#define SNOC_MSMBUS_CLK				122
#define SNOC_MSMBUS_A_CLK			123
#define CXO_SMD_DWC3_CLK			124
#define CXO_SMD_LPM_CLK				125
#define CXO_SMD_OTG_CLK				126
#define CXO_SMD_PIL_LPASS_CLK			127
#define CXO_SMD_PIL_SSC_CLK			128
#define CXO_SMD_PIL_CDSP_CLK			129
#define CNOC_PERIPH_KEEPALIVE_A_CLK		130
#define AGGR2_NOC_MSMBUS_CLK			131
#define AGGR2_NOC_MSMBUS_A_CLK			132
#define AGGR2_NOC_SMMU_CLK			133
#define AGGR2_NOC_USB_CLK			134
#define SNOC_WCNSS_A_CLK			135
#define BIMC_WCNSS_A_CLK			136
#define CNOC_KEEPALIVE_A_CLK			137
#define SNOC_KEEPALIVE_A_CLK			138
#define CPP_MMNRT_MSMBUS_CLK			139
#define CPP_MMNRT_MSMBUS_A_CLK			140
#define JPEG_MMNRT_MSMBUS_CLK			141
#define JPEG_MMNRT_MSMBUS_A_CLK			142
#define VENUS_MMNRT_MSMBUS_CLK			143
#define VENUS_MMNRT_MSMBUS_A_CLK		144
#define ARM9_MMNRT_MSMBUS_CLK			145
#define ARM9_MMNRT_MSMBUS_A_CLK			146
#define MDP_MMRT_MSMBUS_CLK			147
#define MDP_MMRT_MSMBUS_A_CLK			148
#define VFE_MMRT_MSMBUS_CLK			149
#define VFE_MMRT_MSMBUS_A_CLK			150
#define QUP0_MSMBUS_SNOC_PERIPH_CLK		151
#define QUP0_MSMBUS_SNOC_PERIPH_A_CLK		152
#define QUP1_MSMBUS_SNOC_PERIPH_CLK		153
#define QUP1_MSMBUS_SNOC_PERIPH_A_CLK		154
#define QUP2_MSMBUS_SNOC_PERIPH_CLK		155
#define QUP2_MSMBUS_SNOC_PERIPH_A_CLK		156
#define DAP_MSMBUS_SNOC_PERIPH_CLK		157
#define DAP_MSMBUS_SNOC_PERIPH_A_CLK		158
#define SDC1_MSMBUS_SNOC_PERIPH_CLK		159
#define SDC1_MSMBUS_SNOC_PERIPH_A_CLK		160
#define SDC2_MSMBUS_SNOC_PERIPH_CLK		161
#define SDC2_MSMBUS_SNOC_PERIPH_A_CLK		162
#define CRYPTO_MSMBUS_SNOC_PERIPH_CLK		163
#define CRYPTO_MSMBUS_SNOC_PERIPH_A_CLK		164
#define SDC1_SLV_MSMBUS_SNOC_PERIPH_CLK		165
#define SDC1_SLV_MSMBUS_SNOC_PERIPH_A_CLK	166
#define SDC2_SLV_MSMBUS_SNOC_PERIPH_CLK		167
#define SDC2_SLV_MSMBUS_SNOC_PERIPH_A_CLK	168
#define BIMC_USB_A_CLK				169
#define BIMC_USB_CLK				170
#define PNOC_USB_A_CLK				171
#define PNOC_USB_CLK				172
#define SNOC_USB_A_CLK				173
#define SNOC_USB_CLK				174
#define CXO_SMD_PIL_MSS_CLK			175
#define CXO_SMD_PIL_PRONTO_CLK			176
#define CXO_SMD_WLAN_CLK			177
#define CXO_SMD_PIL_SPSS_CLK			178

#endif
