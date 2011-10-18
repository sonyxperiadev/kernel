/*****************************************************************************
*
* Voltage tables for Rhea platform
*
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/
#ifndef __VLT_TBL_H__
#define __VLT_TBL_H__

#define VLT_ID_OFF		0
#define VLT_ID_RETN	1
#define VLT_ID_WAKEUP	2
#define VLT_ID_ECO		0xA
#define VLT_ID_NORMAL	0xD
#define VLT_ID_TURBO	0xF

#define PROC_CCU_FREQ_ID_ECO		6 /*3*/
#define PROC_CCU_FREQ_ID_NRML		6
#define PROC_CCU_FREQ_ID_TURBO		7

#define MM_CCU_FREQ_ID_ECO		1
#define MM_CCU_FREQ_ID_NRML		4
#define MM_CCU_FREQ_ID_TURBO		5

#define HUB_CCU_FREQ_ID_ECO		2
#define HUB_CCU_FREQ_ID_NRML		2

#define AON_CCU_FREQ_ID_ECO		2
#define AON_CCU_FREQ_ID_NRML		3

#define KPM_CCU_FREQ_ID_ECO		2
#define KPM_CCU_FREQ_ID_NRML		3

#define KPS_CCU_FREQ_ID_ECO		1
#define KPS_CCU_FREQ_ID_NRML		3

/*CSR voltage register values for BCM59055 PMU*/
#define CSR_REG_VAL_RETN_SS		0x4
#define CSR_REG_VAL_ECO_SS		0x7
#define CSR_REG_VAL_NRML_SS		0xF
#define CSR_REG_VAL_TURBO_SS		0x14

#define CSR_REG_VAL_RETN_TT		0x4
#define CSR_REG_VAL_ECO_TT		0x7
#define CSR_REG_VAL_NRML_TT		0xC
#define CSR_REG_VAL_TURBO_TT		0x11

#define CSR_REG_VAL_RETN_FF		0x4
#define CSR_REG_VAL_ECO_FF		0x7
#define CSR_REG_VAL_NRML_FF		0xA
#define CSR_REG_VAL_TURBO_FF		0xE


#define VLT_NORMAL_PERI	VLT_ID_ECO
#define VLT_HIGH_PERI	VLT_ID_NORMAL

#define PROC_CCU_FREQ_VOLT_TBL		VLT_ID_ECO,VLT_ID_ECO,VLT_ID_ECO,VLT_ID_ECO,\
						VLT_ID_ECO,VLT_ID_ECO,VLT_ID_NORMAL,VLT_ID_TURBO
#define PROC_CCU_FREQ_VOLT_TBL_SZ 	8

#define MM_CCU_FREQ_VOLT_TBL		VLT_ID_ECO,VLT_ID_ECO,VLT_ID_ECO,VLT_ID_ECO,\
						VLT_ID_NORMAL,VLT_ID_TURBO
#define MM_CCU_FREQ_VOLT_TBL_SZ 	6

#define HUB_CCU_FREQ_VOLT_TBL		VLT_ID_ECO,VLT_ID_ECO,VLT_ID_NORMAL,VLT_ID_ECO,\
						VLT_ID_ECO,VLT_ID_ECO,VLT_ID_ECO
#define HUB_CCU_FREQ_VOLT_TBL_SZ 	7

#define AON_CCU_FREQ_VOLT_TBL		VLT_ID_ECO,VLT_ID_ECO,VLT_ID_ECO,VLT_ID_NORMAL,VLT_ID_ECO
#define AON_CCU_FREQ_VOLT_TBL_SZ 	5

#define KPS_CCU_FREQ_VOLT_TBL		VLT_ID_ECO,VLT_ID_ECO,VLT_ID_ECO,VLT_ID_NORMAL,\
						VLT_ID_ECO,VLT_ID_ECO
#define KPS_CCU_FREQ_VOLT_TBL_SZ 	6

#define KPM_CCU_FREQ_VOLT_TBL		VLT_ID_ECO,VLT_ID_ECO,VLT_ID_ECO,VLT_ID_NORMAL,\
						VLT_ID_ECO,VLT_ID_ECO,VLT_ID_ECO,VLT_ID_ECO
#define KPM_CCU_FREQ_VOLT_TBL_SZ 	8


#define PROC_CCU_FREQ_POLICY_TBL	PROC_CCU_FREQ_ID_ECO,PROC_CCU_FREQ_ID_ECO, \
						PROC_CCU_FREQ_ID_ECO,PROC_CCU_FREQ_ID_TURBO


#define MM_CCU_FREQ_POLICY_TBL		MM_CCU_FREQ_ID_ECO,MM_CCU_FREQ_ID_ECO, \
						MM_CCU_FREQ_ID_ECO, MM_CCU_FREQ_ID_TURBO

#define HUB_CCU_FREQ_POLICY_TBL		HUB_CCU_FREQ_ID_ECO,HUB_CCU_FREQ_ID_ECO, \
						HUB_CCU_FREQ_ID_ECO, HUB_CCU_FREQ_ID_NRML

#define AON_CCU_FREQ_POLICY_TBL		AON_CCU_FREQ_ID_ECO,AON_CCU_FREQ_ID_ECO, \
						AON_CCU_FREQ_ID_ECO, AON_CCU_FREQ_ID_NRML

#define KPM_CCU_FREQ_POLICY_TBL		KPM_CCU_FREQ_ID_ECO,KPM_CCU_FREQ_ID_ECO, \
						KPM_CCU_FREQ_ID_ECO, KPM_CCU_FREQ_ID_NRML

#define KPS_CCU_FREQ_POLICY_TBL		KPS_CCU_FREQ_ID_ECO,KPS_CCU_FREQ_ID_ECO, \
						KPS_CCU_FREQ_ID_ECO, KPS_CCU_FREQ_ID_NRML


#define PMU_SCR_VLT_TBL_SS		CSR_REG_VAL_RETN_SS, \
					CSR_REG_VAL_RETN_SS, \
					CSR_REG_VAL_RETN_SS, \
					CSR_REG_VAL_RETN_SS, \
					CSR_REG_VAL_RETN_SS, \
					CSR_REG_VAL_RETN_SS, \
					CSR_REG_VAL_RETN_SS, \
					CSR_REG_VAL_RETN_SS, \
					CSR_REG_VAL_ECO_SS,	 \
					CSR_REG_VAL_ECO_SS,	 \
					CSR_REG_VAL_ECO_SS,	 \
					CSR_REG_VAL_NRML_SS, \
					CSR_REG_VAL_NRML_SS, \
					CSR_REG_VAL_NRML_SS, \
					CSR_REG_VAL_TURBO_SS,\
					CSR_REG_VAL_TURBO_SS

#define PMU_SCR_VLT_TBL_TT		CSR_REG_VAL_RETN_TT, \
					CSR_REG_VAL_RETN_TT, \
					CSR_REG_VAL_RETN_TT, \
					CSR_REG_VAL_RETN_TT, \
					CSR_REG_VAL_RETN_TT, \
					CSR_REG_VAL_RETN_TT, \
					CSR_REG_VAL_RETN_TT, \
					CSR_REG_VAL_RETN_TT, \
					CSR_REG_VAL_ECO_TT,	 \
					CSR_REG_VAL_ECO_TT,	 \
					CSR_REG_VAL_ECO_TT,	 \
					CSR_REG_VAL_NRML_TT, \
					CSR_REG_VAL_NRML_TT, \
					CSR_REG_VAL_NRML_TT, \
					CSR_REG_VAL_TURBO_TT,\
					CSR_REG_VAL_TURBO_TT

#define PMU_SCR_VLT_TBL_FF		CSR_REG_VAL_RETN_FF, \
					CSR_REG_VAL_RETN_FF, \
					CSR_REG_VAL_RETN_FF, \
					CSR_REG_VAL_RETN_FF, \
					CSR_REG_VAL_RETN_FF, \
					CSR_REG_VAL_RETN_FF, \
					CSR_REG_VAL_RETN_FF, \
					CSR_REG_VAL_RETN_FF, \
					CSR_REG_VAL_ECO_FF,	 \
					CSR_REG_VAL_ECO_FF,	 \
					CSR_REG_VAL_ECO_FF,	 \
					CSR_REG_VAL_NRML_FF, \
					CSR_REG_VAL_NRML_FF, \
					CSR_REG_VAL_NRML_FF, \
					CSR_REG_VAL_TURBO_FF,\
					CSR_REG_VAL_TURBO_FF



#endif /*__VLT_TBL_H__*/
