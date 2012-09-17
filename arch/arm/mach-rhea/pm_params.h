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

#include <plat/pwr_mgr.h>

/*Helper macros for HW JIRA workarounds*/
/*Macro to check if the workaround enable flag is enabled
 */

#define JIRA_WA_ENABLED(x) __jira_wa_enabled(x)

#define JIRA_WA_FLG_NAME(x) jira_ ## x ## _enable
/*Macro to define the JIRA workaround flag. R/W sysfs
   interface is also added to control the flag from console*/
#define DEFINE_JIRA_WA_FLG(x, def_val)	\
		int JIRA_WA_FLG_NAME(x) = def_val;\
		module_param_named(JIRA_WA_FLG_NAME(x),\
		JIRA_WA_FLG_NAME(x), int, S_IRUGO | \
				S_IWUSR | S_IWGRP)
/*Macro to define the JIRA workaround flag. Read only sysfs
   interface is also added to read the flag from console*/
#define DEFINE_JIRA_WA_RO_FLG(x, def_val)      \
		int JIRA_WA_FLG_NAME(x) = def_val;\
		module_param_named(JIRA_WA_FLG_NAME(x), \
			JIRA_WA_FLG_NAME(x), int, S_IRUGO)


#define ARRAY_LIST(...) {__VA_ARGS__}
#define SR_VLT_LUT_SIZE 16
#define SR_ECO_INX_START	8
#define SR_ECO_INX_END		0xA
#define SR_NRML_INX_START	0xB
#define SR_NRML_INX_END		0xD
#define SR_TURBO_INX_START	0xE
#define SR_TURBO_INX_END	0xF

#define VLT_ID_OFF		0
#define VLT_ID_RETN	1
#define VLT_ID_WAKEUP	2
#define VLT_ID_ECO		0xA
#define VLT_ID_NORMAL	0xD
#define VLT_ID_TURBO	0xF

#define PROC_CCU_FREQ_ID_ECO	3

#define PROC_CCU_FREQ_ID_NRML	6
#define PROC_CCU_FREQ_ID_TURBO	7

#define MM_CCU_FREQ_ID_ECO		1
#define MM_CCU_FREQ_ID_NRML		4
#define MM_CCU_FREQ_ID_TURBO	5

#define HUB_CCU_FREQ_ID_ECO		2
#define HUB_CCU_FREQ_ID_NRML	2

#define AON_CCU_FREQ_ID_ECO		2
#define AON_CCU_FREQ_ID_NRML	3

#define KPM_CCU_FREQ_ID_ECO		2
#define KPM_CCU_FREQ_ID_NRML	3

#define KPS_CCU_FREQ_ID_ECO		1
#define KPS_CCU_FREQ_ID_NRML	3

#define VLT_NORMAL_PERI		VLT_ID_ECO
#define VLT_HIGH_PERI		VLT_ID_NORMAL

#define PROC_CCU_FREQ_VOLT_TBL	\
		ARRAY_LIST(VLT_ID_ECO, VLT_ID_ECO, VLT_ID_ECO, VLT_ID_ECO,\
			VLT_ID_ECO, VLT_ID_ECO, VLT_ID_NORMAL, VLT_ID_TURBO)
#define PROC_CCU_FREQ_VOLT_TBL_SZ	8

#define MM_CCU_FREQ_VOLT_TBL	\
		ARRAY_LIST(VLT_ID_ECO, VLT_ID_ECO, VLT_ID_ECO, VLT_ID_ECO,\
			VLT_ID_NORMAL, VLT_ID_TURBO)
#define MM_CCU_FREQ_VOLT_TBL_SZ		6

#define HUB_CCU_FREQ_VOLT_TBL	\
		ARRAY_LIST(VLT_ID_ECO, VLT_ID_ECO, VLT_ID_ECO, VLT_ID_ECO,\
			VLT_ID_ECO, VLT_ID_ECO, VLT_ID_ECO)
#define HUB_CCU_FREQ_VOLT_TBL_SZ	7

/*AON is on fixed voltage domain. Voltage ids does not really matter*/
#define AON_CCU_FREQ_VOLT_TBL	\
		ARRAY_LIST(VLT_ID_ECO, VLT_ID_ECO, VLT_ID_ECO, VLT_ID_ECO,\
			VLT_ID_ECO)
#define AON_CCU_FREQ_VOLT_TBL_SZ	5

#define KPS_CCU_FREQ_VOLT_TBL	\
		ARRAY_LIST(VLT_ID_ECO, VLT_ID_ECO, VLT_ID_ECO, VLT_ID_NORMAL,\
			VLT_ID_ECO, VLT_ID_ECO)
#define KPS_CCU_FREQ_VOLT_TBL_SZ	6

#define KPM_CCU_FREQ_VOLT_TBL	\
		ARRAY_LIST(VLT_ID_ECO, VLT_ID_ECO, VLT_ID_ECO, VLT_ID_NORMAL,\
			VLT_ID_ECO, VLT_ID_ECO, VLT_ID_ECO, VLT_ID_ECO)
#define KPM_CCU_FREQ_VOLT_TBL_SZ	8

#define BMDM_CCU_FREQ_VOLT_TBL	\
		ARRAY_LIST(VLT_ID_ECO, VLT_ID_ECO, VLT_ID_NORMAL,\
			VLT_ID_NORMAL, VLT_ID_NORMAL, VLT_ID_TURBO,\
			VLT_ID_TURBO, VLT_ID_TURBO)
/* As per RDB there are only 6 valid freq_ids for bmdm. this count
   is used only to initialize voltage table from AP */
#define BMDM_CCU_FREQ_VOLT_TBL_SZ	8

#define DSP_CCU_FREQ_VOLT_TBL	\
		ARRAY_LIST(VLT_ID_ECO, VLT_ID_ECO, VLT_ID_NORMAL,\
			VLT_ID_NORMAL, VLT_ID_TURBO, VLT_ID_TURBO,\
			VLT_ID_TURBO, VLT_ID_TURBO)
/* As per RDB there are only 5 valid freq_ids for dsp. this count
   is used only to initialize voltage table from AP */
#define DSP_CCU_FREQ_VOLT_TBL_SZ	8

#define PROC_CCU_FREQ_POLICY_TBL	\
		ARRAY_LIST(PROC_CCU_FREQ_ID_ECO, PROC_CCU_FREQ_ID_ECO,\
			PROC_CCU_FREQ_ID_ECO, PROC_CCU_FREQ_ID_TURBO)
#define MM_CCU_FREQ_POLICY_TBL	\
		ARRAY_LIST(MM_CCU_FREQ_ID_ECO, MM_CCU_FREQ_ID_ECO,\
			MM_CCU_FREQ_ID_ECO, MM_CCU_FREQ_ID_TURBO)
#define HUB_CCU_FREQ_POLICY_TBL	\
		ARRAY_LIST(HUB_CCU_FREQ_ID_ECO, HUB_CCU_FREQ_ID_ECO,\
			HUB_CCU_FREQ_ID_ECO, HUB_CCU_FREQ_ID_NRML)
#define AON_CCU_FREQ_POLICY_TBL	\
		ARRAY_LIST(AON_CCU_FREQ_ID_ECO, AON_CCU_FREQ_ID_ECO,\
			AON_CCU_FREQ_ID_ECO, AON_CCU_FREQ_ID_NRML)
#define KPM_CCU_FREQ_POLICY_TBL	\
		ARRAY_LIST(KPM_CCU_FREQ_ID_ECO, KPM_CCU_FREQ_ID_ECO,\
			KPM_CCU_FREQ_ID_ECO, KPM_CCU_FREQ_ID_NRML)
#define KPS_CCU_FREQ_POLICY_TBL	\
		ARRAY_LIST(KPS_CCU_FREQ_ID_ECO, KPS_CCU_FREQ_ID_ECO,\
			KPS_CCU_FREQ_ID_ECO, KPS_CCU_FREQ_ID_NRML)

/* PLL Offset config values for RF interference reduction
*  as suggested by modem team */
#define PLLARM_OFFEST_CONFIG    0x35589D9
#define PLLDSI_OFFEST_CONFIG    0x0
#define PLL1_OFFSET_CONFIG	0xBFFFF

/*supported A9 freqs*/
enum {
	A9_FREQ_700_MHZ,
	A9_FREQ_800_MHZ,
	A9_FREQ_850_MHZ,
	A9_FREQ_1_GHZ,
	A9_FREQ_MAX
};

#ifdef CONFIG_KONA_POWER_MGR
struct pwrmgr_init_param {
	struct i2c_cmd *cmd_buf;
	u32 cmb_buf_size;
	struct v0x_spec_i2c_cmd_ptr *v0ptr;
	u8 *def_vlt_tbl;
	u32 vlt_tbl_size;
	u32 i2c_rd_off;
	int i2c_rd_slv_id_off1;	/*slave id offset -  write reg address */
	int i2c_rd_slv_id_off2;	/*slave id offset - read reg value */
	int i2c_rd_reg_addr_off;
	int i2c_rd_fifo_off;
	u32 i2c_wr_off;
	int i2c_wr_slv_id_off;
	int i2c_wr_reg_addr_off;
	int i2c_wr_val_addr_off;
	u32 i2c_seq_timeout;	/*timeout in ms */
#ifdef CONFIG_KONA_PWRMGR_SWSEQ_FAKE_TRG_ERRATUM
	int pc_toggle_off;
#endif
};

extern struct pwrmgr_init_param pwrmgr_init_param;

#endif	/*CONFIG_KONA_POWER_MGR */

/*This API should be defined in appropriate PMU board file*/
extern const u8 *bcmpmu_get_sr_vlt_table(int sr, u32 freq_inx,
			u32 silicon_type);
extern int __jira_wa_enabled(u32 jira);
extern int __init rhea_pm_params_init(void);
extern int pm_init_pmu_sr_vlt_map_table(int silicon_type, int freq_id);

#endif	/*__PM_PARAMS_H__*/

