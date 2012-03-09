/*****************************************************************************
*
* Power Manager config parameters for Rhea platform
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
#ifndef __PM_PARAMS_H__
#define __PM_PARAMS_H__

#include<plat/pwr_mgr.h>

#define VLT_ID_OFF		0
#define VLT_ID_RETN	1
#define VLT_ID_WAKEUP	2
#define VLT_ID_ECO		0xA
#define VLT_ID_NORMAL	0xD
#define VLT_ID_TURBO	0xF

#define PROC_CCU_FREQ_ID_ECO		3

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
#define PROC_CCU_FREQ_VOLT_TBL_SZ	8

#define MM_CCU_FREQ_VOLT_TBL		VLT_ID_ECO,VLT_ID_ECO,VLT_ID_ECO,VLT_ID_ECO,\
						VLT_ID_NORMAL,VLT_ID_TURBO
#define MM_CCU_FREQ_VOLT_TBL_SZ		6

#define HUB_CCU_FREQ_VOLT_TBL		VLT_ID_ECO,VLT_ID_ECO,VLT_ID_ECO,VLT_ID_ECO,\
						VLT_ID_ECO,VLT_ID_ECO,VLT_ID_ECO
#define HUB_CCU_FREQ_VOLT_TBL_SZ	7

/*AON is on fixed voltage domain. Voltage ids does not really matter*/
#define AON_CCU_FREQ_VOLT_TBL		VLT_ID_ECO,VLT_ID_ECO,VLT_ID_ECO,VLT_ID_ECO,VLT_ID_ECO
#define AON_CCU_FREQ_VOLT_TBL_SZ	5

#define KPS_CCU_FREQ_VOLT_TBL		VLT_ID_ECO,VLT_ID_ECO,VLT_ID_ECO,VLT_ID_NORMAL,\
						VLT_ID_ECO,VLT_ID_ECO
#define KPS_CCU_FREQ_VOLT_TBL_SZ	6

#define KPM_CCU_FREQ_VOLT_TBL		VLT_ID_ECO,VLT_ID_ECO,VLT_ID_ECO,VLT_ID_NORMAL,\
						VLT_ID_ECO,VLT_ID_ECO,VLT_ID_ECO,VLT_ID_ECO
#define KPM_CCU_FREQ_VOLT_TBL_SZ	8

#define BMDM_CCU_FREQ_VOLT_TBL		VLT_ID_ECO,VLT_ID_ECO,VLT_ID_NORMAL,VLT_ID_NORMAL,\
						VLT_ID_NORMAL,VLT_ID_TURBO,VLT_ID_TURBO,VLT_ID_TURBO
#define BMDM_CCU_FREQ_VOLT_TBL_SZ	8	/* As per RDB there are only 6 valid freq_ids for bmdm. this count
						   is used only to initialize voltage table from AP */
#define DSP_CCU_FREQ_VOLT_TBL		VLT_ID_ECO,VLT_ID_ECO,VLT_ID_NORMAL,VLT_ID_NORMAL,\
						VLT_ID_TURBO,VLT_ID_TURBO,VLT_ID_TURBO,VLT_ID_TURBO
#define DSP_CCU_FREQ_VOLT_TBL_SZ	8	/* As per RDB there are only 5 valid freq_ids for dsp. this count
						   is used only to initialize voltage table from AP */

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

#ifdef CONFIG_KONA_POWER_MGR
struct pwrmgr_init_param {
	struct i2c_cmd *cmd_buf;
	u32 cmb_buf_size;
	struct v0x_spec_i2c_cmd_ptr *v0ptr;
	u8 *def_vlt_tbl;
	u32 vlt_tbl_size;
#if defined(CONFIG_KONA_PWRMGR_REV2)
	u32 i2c_rd_off;
	int i2c_rd_slv_id_off1;	/*slave id offset -  write reg address */
	int i2c_rd_slv_id_off2;	/*slave id offset - read reg value */
	int i2c_rd_reg_addr_off;
	int i2c_rd_nack_off;
	int i2c_rd_nack_jump_off;
	int i2c_rd_fifo_off;
	u32 i2c_wr_off;
	int i2c_wr_slv_id_off;
	int i2c_wr_reg_addr_off;
	int i2c_wr_val_addr_off;
	u32 i2c_seq_timeout;	/*timeout in ms */
#endif
};

extern struct pwrmgr_init_param pwrmgr_init_param;

#endif /*CONFIG_KONA_POWER_MGR */

extern int __init rhea_pm_params_init(void);

/*Helper macros for HW JIRA workarounds*/
/*Macro to check if the workaround enable flag is enabled
*/
int __jira_wa_enabled(u32 jira);
#define JIRA_WA_ENABLED(x) __jira_wa_enabled(x)

#define JIRA_WA_FLG_NAME(x) jira_##x##_enable
/*Macro to define the JIRA workaround flag. R/W sysfs
interface is also added to control the flag from console*/
#define DEFINE_JIRA_WA_FLG(x, def_val)	\
				int JIRA_WA_FLG_NAME(x) = def_val;\
				module_param_named(JIRA_WA_FLG_NAME(x),\
				JIRA_WA_FLG_NAME(x), int, S_IRUGO | \
				S_IWUSR | S_IWGRP)
/*Macro to define the JIRA workaround flag. Read only sysfs
interface is also added to read the flag from console*/
#define DEFINE_JIRA_WA_RO_FLG(x, def_val)	\
				int JIRA_WA_FLG_NAME(x) = def_val;\
				module_param_named(JIRA_WA_FLG_NAME(x), \
				JIRA_WA_FLG_NAME(x), int, S_IRUGO)

#endif /*__PM_PARAMS_H__*/
