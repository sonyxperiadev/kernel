/*****************************************************************************
  *
  * Voltage tables for Hawaii platform
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

 #include <linux/kernel.h>
#include <linux/module.h>
#include <mach/avs.h>
#include <linux/mfd/bcmpmu59xxx.h>
#include "volt_tbl.h"

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <asm/uaccess.h>
#include <linux/seq_file.h>
#endif
#include <plat/kona_memc.h>

#define REV_ID	"0.17"

#define ARRAY_LIST(...) {__VA_ARGS__}

#ifdef CONFIG_DEBUG_FS
static struct dentry *dent_vlt_root_dir;
#endif

u8 vlt_id_table[SR_VLT_LUT_SIZE];
static struct pmu_volt_dbg volt_dbg_log;


#define MSR_RETN_VAL			800
#define SDSR1_RETN_VAL			900

#define DEFAULT_AGING_MARGIN_1G	40
#define DEFAULT_AGING_MARGIN_1200M	50

#define CSR_XTAL_1G_SS			930
#define CSR_ECO_1G_SS			950
#define CSR_NM_1G_SS			980
#define CSR_TURBO_1G_SS			1050
#define CSR_SUPER_TURBO_1G_SS		1220

#define CSR_XTAL_1G_TS			920
#define CSR_ECO_1G_TS			940
#define CSR_NM_1G_TS			970
#define CSR_TURBO_1G_TS			1030
#define CSR_SUPER_TURBO_1G_TS		1190

#define CSR_XTAL_1G_TT			900
#define CSR_ECO_1G_TT			920
#define CSR_NM_1G_TT			940
#define CSR_TURBO_1G_TT			990
#define CSR_SUPER_TURBO_1G_TT		1150

#define CSR_XTAL_1G_TF			890
#define CSR_ECO_1G_TF			910
#define CSR_NM_1G_TF			930
#define CSR_TURBO_1G_TF			980
#define CSR_SUPER_TURBO_1G_TF		1130

#define CSR_XTAL_1G_FF			880
#define CSR_ECO_1G_FF			900
#define CSR_NM_1G_FF			920
#define CSR_TURBO_1G_FF			960
#define CSR_SUPER_TURBO_1G_FF		1100

#define CSR_XTAL_1200M_SS		930
#define CSR_ECO_1200M_SS		950
#define CSR_NM_1200M_SS			1030
#define CSR_TURBO_1200M_SS		1130
#define CSR_SUPER_TURBO_1200M_SS	1310

#define CSR_XTAL_1200M_TS		920
#define CSR_ECO_1200M_TS		940
#define CSR_NM_1200M_TS			1010
#define CSR_TURBO_1200M_TS		1100
#define CSR_SUPER_TURBO_1200M_TS	1280

#define CSR_XTAL_1200M_TT		900
#define CSR_ECO_1200M_TT		920
#define CSR_NM_1200M_TT			970
#define CSR_TURBO_1200M_TT		1060
#define CSR_SUPER_TURBO_1200M_TT	1240

#define CSR_XTAL_1200M_TF		890
#define CSR_ECO_1200M_TF		910
#define CSR_NM_1200M_TF			960
#define CSR_TURBO_1200M_TF		1040
#define CSR_SUPER_TURBO_1200M_TF	1220

#define CSR_XTAL_1200M_FF		880
#define CSR_ECO_1200M_FF		900
#define CSR_NM_1200M_FF			940
#define CSR_TURBO_1200M_FF		1010
#define CSR_SUPER_TURBO_1200M_FF	1190

/* MSR_SUPER_TURBO for 1 GHZ is defined only for testing purposes */

#define MSR_ECO_1G_SS			950
#define MSR_NM_1G_SS			1090
#define MSR_TURBO_1G_SS			1220
#define MSR_SUPER_TURBO_1G_SS		1310

#define MSR_ECO_1G_TS			940
#define MSR_NM_1G_TS			1070
#define MSR_TURBO_1G_TS			1190
#define MSR_SUPER_TURBO_1G_TS		1280

#define MSR_ECO_1G_TT			920
#define MSR_NM_1G_TT			1030
#define MSR_TURBO_1G_TT			1150
#define MSR_SUPER_TURBO_1G_TT		1240

#define MSR_ECO_1G_TF			910
#define MSR_NM_1G_TF			1020
#define MSR_TURBO_1G_TF			1130
#define MSR_SUPER_TURBO_1G_TF		1220

#define MSR_ECO_1G_FF			900
#define MSR_NM_1G_FF			1000
#define MSR_TURBO_1G_FF			1100
#define MSR_SUPER_TURBO_1G_FF		1190

#define MSR_ECO_1200M_SS		950
#define MSR_NM_1200M_SS			1090
#define MSR_TURBO_1200M_SS		1220
#define MSR_SUPER_TURBO_1200M_SS	1310

#define MSR_ECO_1200M_TS		940
#define MSR_NM_1200M_TS			1070
#define MSR_TURBO_1200M_TS		1190
#define MSR_SUPER_TURBO_1200M_TS	1280

#define MSR_ECO_1200M_TT		920
#define MSR_NM_1200M_TT			1030
#define MSR_TURBO_1200M_TT		1150
#define MSR_SUPER_TURBO_1200M_TT	1240

#define MSR_ECO_1200M_TF		910
#define MSR_NM_1200M_TF			1020
#define MSR_TURBO_1200M_TF		1130
#define MSR_SUPER_TURBO_1200M_TF	1220

#define MSR_ECO_1200M_FF		900
#define MSR_NM_1200M_FF			1000
#define MSR_TURBO_1200M_FF		1100
#define MSR_SUPER_TURBO_1200M_FF	1190


#define PMU_VLT_TBL_1G_SS ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_XTAL_1G_SS,\
			CSR_ECO_1G_SS, CSR_NM_1G_SS,\
			CSR_TURBO_1G_SS, CSR_SUPER_TURBO_1G_SS),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_1G_SS, MSR_NM_1G_SS,\
			MSR_TURBO_1G_SS, MSR_SUPER_TURBO_1G_SS),\
		INIT_LPM_VLT_IDS(MSR_RETN_VAL, MSR_RETN_VAL, MSR_RETN_VAL),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_VAL))

#define PMU_VLT_TBL_1G_TS ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_XTAL_1G_TS,\
			CSR_ECO_1G_TS, CSR_NM_1G_TS,\
			CSR_TURBO_1G_TS, CSR_SUPER_TURBO_1G_TS),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_1G_TS, MSR_NM_1G_TS,\
			MSR_TURBO_1G_TS, MSR_SUPER_TURBO_1G_TS),\
		INIT_LPM_VLT_IDS(MSR_RETN_VAL, MSR_RETN_VAL, MSR_RETN_VAL),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_VAL))

#define PMU_VLT_TBL_1G_TT ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_XTAL_1G_TT,\
			CSR_ECO_1G_TT, CSR_NM_1G_TT,\
			CSR_TURBO_1G_TT, CSR_SUPER_TURBO_1G_TT),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_1G_TT, MSR_NM_1G_TT,\
			MSR_TURBO_1G_TT, MSR_SUPER_TURBO_1G_TT),\
		INIT_LPM_VLT_IDS(MSR_RETN_VAL, MSR_RETN_VAL, MSR_RETN_VAL),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_VAL))

#define PMU_VLT_TBL_1G_TF ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_XTAL_1G_TF,\
			CSR_ECO_1G_TF, CSR_NM_1G_TF,\
			CSR_TURBO_1G_TF, CSR_SUPER_TURBO_1G_TF),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_1G_TF, MSR_NM_1G_TF,\
			MSR_TURBO_1G_TF, MSR_SUPER_TURBO_1G_TF),\
		INIT_LPM_VLT_IDS(MSR_RETN_VAL, MSR_RETN_VAL, MSR_RETN_VAL),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_VAL))

#define PMU_VLT_TBL_1G_FF ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_XTAL_1G_FF,\
			CSR_ECO_1G_FF, CSR_NM_1G_FF,\
			CSR_TURBO_1G_FF, CSR_SUPER_TURBO_1G_FF),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_1G_FF, MSR_NM_1G_FF,\
			MSR_TURBO_1G_FF, MSR_SUPER_TURBO_1G_FF),\
		INIT_LPM_VLT_IDS(MSR_RETN_VAL, MSR_RETN_VAL, MSR_RETN_VAL),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_VAL))

#define PMU_VLT_TBL_1200M_SS ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_XTAL_1200M_SS,\
			CSR_ECO_1200M_SS, CSR_NM_1200M_SS,\
			CSR_TURBO_1200M_SS, CSR_SUPER_TURBO_1200M_SS),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_1200M_SS, MSR_NM_1200M_SS,\
			MSR_TURBO_1200M_SS, MSR_SUPER_TURBO_1200M_SS),\
		INIT_LPM_VLT_IDS(MSR_RETN_VAL, MSR_RETN_VAL, MSR_RETN_VAL),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_VAL))

#define PMU_VLT_TBL_1200M_TS ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_XTAL_1200M_TS,\
			CSR_ECO_1200M_TS, CSR_NM_1200M_TS,\
			CSR_TURBO_1200M_TS, CSR_SUPER_TURBO_1200M_TS),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_1200M_TS, MSR_NM_1200M_TS,\
			MSR_TURBO_1200M_TS, MSR_SUPER_TURBO_1200M_TS),\
		INIT_LPM_VLT_IDS(MSR_RETN_VAL, MSR_RETN_VAL, MSR_RETN_VAL),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_VAL))

#define PMU_VLT_TBL_1200M_TT ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_XTAL_1200M_TT,\
			CSR_ECO_1200M_TT, CSR_NM_1200M_TT,\
			CSR_TURBO_1200M_TT, CSR_SUPER_TURBO_1200M_TT),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_1200M_TT, MSR_NM_1200M_TT,\
			MSR_TURBO_1200M_TT, MSR_SUPER_TURBO_1200M_TT),\
		INIT_LPM_VLT_IDS(MSR_RETN_VAL, MSR_RETN_VAL, MSR_RETN_VAL),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_VAL))

#define PMU_VLT_TBL_1200M_TF ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_XTAL_1200M_TF,\
			CSR_ECO_1200M_TF, CSR_NM_1200M_TF,\
			CSR_TURBO_1200M_TF, CSR_SUPER_TURBO_1200M_TF),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_1200M_TF, MSR_NM_1200M_TF,\
			MSR_TURBO_1200M_TF, MSR_SUPER_TURBO_1200M_TF),\
		INIT_LPM_VLT_IDS(MSR_RETN_VAL, MSR_RETN_VAL, MSR_RETN_VAL),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_VAL))

#define PMU_VLT_TBL_1200M_FF ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_XTAL_1200M_FF,\
			CSR_ECO_1200M_FF, CSR_NM_1200M_FF,\
			CSR_TURBO_1200M_FF, CSR_SUPER_TURBO_1200M_FF),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_1200M_FF, MSR_NM_1200M_FF,\
			MSR_TURBO_1200M_FF, MSR_SUPER_TURBO_1200M_FF),\
		INIT_LPM_VLT_IDS(MSR_RETN_VAL, MSR_RETN_VAL, MSR_RETN_VAL),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_VAL))


#define DEFAULT_VDDFIX_VOLTAGE	1240

#define VDDFIX_VOLT_SS_450M	1300
#define VDDFIX_VOLT_TS_450M	1300
#define VDDFIX_VOLT_TT_450M	1280
#define VDDFIX_VOLT_TF_450M	1280
#define VDDFIX_VOLT_FF_450M	1240


u32 pmu_vlt_table_1g[SILICON_TYPE_MAX][SR_VLT_LUT_SIZE] = {
	PMU_VLT_TBL_1G_SS, PMU_VLT_TBL_1G_TS, PMU_VLT_TBL_1G_TT,
	PMU_VLT_TBL_1G_TF, PMU_VLT_TBL_1G_FF,
};

u32 pmu_vlt_table_1200m[SILICON_TYPE_MAX][SR_VLT_LUT_SIZE] = {
	PMU_VLT_TBL_1200M_SS, PMU_VLT_TBL_1200M_TS, PMU_VLT_TBL_1200M_TT,
	PMU_VLT_TBL_1200M_TF, PMU_VLT_TBL_1200M_FF,
};

u32 sdsr1_active_voltage_450m[SILICON_TYPE_MAX] = {VDDFIX_VOLT_SS_450M,
	VDDFIX_VOLT_TS_450M, VDDFIX_VOLT_TT_450M, VDDFIX_VOLT_TF_450M,
	VDDFIX_VOLT_FF_450M};

#ifdef CONFIG_KONA_AVS
u8 *get_sr_vlt_table(u32 silicon_type, int freq_id)
{
	u32 *vlt_table;
	int i;
	int ret;
	u32 aging_margin;
	int vddvar_adj, vddvara9_adj;
	u32 vlt;
	u32 vddvar_a9_min = avs_get_vddvar_a9_vlt_min();
	u32 vddvar_min = avs_get_vddvar_vlt_min();
	u32 min_vlt_table[] = {
		INIT_A9_VLT_TABLE(vddvar_a9_min,
			vddvar_a9_min, vddvar_a9_min,
			vddvar_a9_min, vddvar_a9_min),
		INIT_OTHER_VLT_TABLE(vddvar_min, vddvar_min,
			vddvar_min, vddvar_min),
		INIT_LPM_VLT_IDS(MSR_RETN_VAL, MSR_RETN_VAL, MSR_RETN_VAL),
		INIT_UNUSED_VLT_IDS(MSR_RETN_VAL)
	};

	pr_info("%s silicon_type = %d, freq_id = %d\n", __func__,
		silicon_type, freq_id);
	if (silicon_type >= SILICON_TYPE_MAX || freq_id >= A9_FREQ_MAX)
		BUG();
	switch (freq_id) {
	case A9_FREQ_1000_MHZ:
			vlt_table = pmu_vlt_table_1g[silicon_type];
			aging_margin = DEFAULT_AGING_MARGIN_1G;
			break;
	case A9_FREQ_1200_MHZ:
			vlt_table = pmu_vlt_table_1200m[silicon_type];
			aging_margin = DEFAULT_AGING_MARGIN_1200M;
			break;
	case A9_FREQ_1400_MHZ:
	/* Right now 1.5Ghz table hasn't been defined */
	default:
			BUG();
	}

	ret = avs_get_vddvar_aging_margin(silicon_type, freq_id);
	if (ret >= 0)
		aging_margin = (u32)ret;
	for (i = 0; i < ACTIVE_VOLTAGE_OFFSET; i++) {
		vlt_id_table[i] = bcmpmu_rgltr_get_volt_id(vlt_table[i]);
		volt_dbg_log.pwr_mgr_volt_tbl[i] = vlt_id_table[i];
	}
	vddvara9_adj = avs_get_vddvar_a9_adj();
	vddvar_adj = avs_get_vddvar_adj();
	pr_info("MSR adjust: %d, CSR adjust: %d", vddvar_adj, vddvara9_adj);
	for (i = ACTIVE_VOLTAGE_OFFSET; i < SR_VLT_LUT_SIZE; i++) {
		vlt = vlt_table[i] + aging_margin;
		if ((i == ACTIVE_VOLTAGE_OFFSET) || (i % 2 == 0))
			vlt += vddvara9_adj;
		else
			vlt += vddvar_adj;
		vlt_id_table[i] = bcmpmu_rgltr_get_volt_id(max(vlt,
							min_vlt_table[i]));
		volt_dbg_log.pwr_mgr_volt_tbl[i] = vlt_id_table[i];
	}
	volt_dbg_log.si_type = silicon_type;
	return vlt_id_table;
}
#else
u8 *get_sr_vlt_table(u32 silicon_type, int freq_id)
{
	u32 *vlt_table;
	int i;
	u32 vlt_adj, temp;
	pr_info("%s silicon_type = %d, freq_id = %d\n", __func__,
		silicon_type, freq_id);
	if (silicon_type > SILICON_TYPE_MAX || freq_id > A9_FREQ_MAX)
		BUG();
	switch (freq_id) {
	case A9_FREQ_1000_MHZ:
			vlt_table = pmu_vlt_table_1g[silicon_type];
			vlt_adj = DEFAULT_AGING_MARGIN_1G;
			break;
	case A9_FREQ_1200_MHZ:
			vlt_table = pmu_vlt_table_1200m[silicon_type];
			vlt_adj = DEFAULT_AGING_MARGIN_1200M;
			break;
	case A9_FREQ_1400_MHZ:
	default:
			BUG();
	}
	for (i = 0; i < SR_VLT_LUT_SIZE; i++) {
		temp = vlt_table[i] + vlt_adj;
		vlt_id_table[i] = bcmpmu_rgltr_get_volt_id(temp);
		volt_dbg_log.pwr_mgr_volt_tbl[i] = vlt_id_table[i];
	}
	volt_dbg_log.si_type = silicon_type;
	return vlt_id_table;
}
#endif


int get_vddvar_retn_vlt_id(void)
{
	u32 ret_vlt = MSR_RETN_VAL;
/*	Right now the retn value is same for all the silicon types
	In case tomorrow if it changes, we will modify the retn value
	returned accordingly
*/
#ifdef CONFIG_KONA_AVS
	u32 min = avs_get_vddvar_ret_vlt_min();
	u32 silicon_type = avs_get_silicon_type();
	switch (silicon_type) {
	case SILICON_TYPE_SLOW:
		ret_vlt = max(ret_vlt, min);
		break;
	case SILICON_TYPE_TYP_SLOW:
		ret_vlt = max(ret_vlt, min);
		break;
	case SILICON_TYPE_TYPICAL:
		ret_vlt = max(ret_vlt, min);
		break;
	case SILICON_TYPE_TYP_FAST:
		ret_vlt = max(ret_vlt, min);
		break;
	case SILICON_TYPE_FAST:
		ret_vlt = max(ret_vlt, min);
		break;
	default:
		BUG();
	}
#endif
	pr_info("MSR retention_voltage: %umV\n", ret_vlt);
	return bcmpmu_rgltr_get_volt_id(ret_vlt);
}

int get_vddfix_vlt_adj(u32 vddfix_vlt)
{
	int voltage = bcmpmu_rgltr_get_volt_val(vddfix_vlt);
	int adj_val, silicon_type = SILICON_TYPE_SLOW;
	u32 ddr_freq, ddr_freq_id;
	/* Convert uV to mV */
	voltage = voltage/1000;

	ddr_freq = kona_memc_get_ddr_clk_freq();
	if (ddr_freq < 400000000)
		ddr_freq_id = DDR_FREQ_400M;
	else
		ddr_freq_id = DDR_FREQ_450M;

#ifdef CONFIG_KONA_AVS
	adj_val = avs_get_vddfix_adj(ddr_freq_id);
	if (adj_val == 0) {
		if (ddr_freq_id == DDR_FREQ_450M) {
			silicon_type = avs_get_silicon_type();
			voltage = sdsr1_active_voltage_450m[silicon_type];
		} else
			voltage = DEFAULT_VDDFIX_VOLTAGE;
	} else
		voltage = DEFAULT_VDDFIX_VOLTAGE + adj_val;
#else
	if (ddr_freq_id == DDR_FREQ_450M)
		voltage = sdsr1_active_voltage_450m[silicon_type];
	else
		voltage = DEFAULT_VDDFIX_VOLTAGE;
#endif
	pr_info("SDSR1 active voltage: %dmV\n", voltage);
	return bcmpmu_rgltr_get_volt_id(voltage);
}

int get_vddfix_retn_vlt_id(u32 reg_val)
{
	int vddfix_ret = reg_val;
#ifdef CONFIG_KONA_AVS
	u32 vlt = SDSR1_RETN_VAL;
	u32 vddfix_min = avs_get_vddfix_ret_vlt_min();
	vlt = max(vlt, vddfix_min);
	vddfix_ret = bcmpmu_rgltr_get_volt_id(vlt);
	pr_info("SDSR1(AVS) min retn voltage: %dmV, curr val: %umV\n",
		vlt, bcmpmu_rgltr_get_volt_val(reg_val)/1000);
	BUG_ON(vddfix_ret < 0);
#endif
	return vddfix_ret;
}

void populate_pmu_voltage_log(void)
{
	bcmpmu_populate_volt_dbg_log(&volt_dbg_log);
}


static int panic_event(struct notifier_block *this, unsigned long event,
		void *ptr)
{

	struct pmu_volt_dbg *volt_ptr = &volt_dbg_log;
	static int has_panicked;
	int i;
	if (has_panicked)
		return 0;

	pr_err("Reading the voltage table and sr voltages\n");
	pr_err("Silicon Type: %u\n", volt_ptr->si_type);
	for (i = 0; i < SR_VLT_LUT_SIZE; i++)
		pr_err("Tbl[%u] = 0x%x Val: %umV\n", i, volt_ptr->
			pwr_mgr_volt_tbl[i], bcmpmu_rgltr_get_volt_val(
			volt_ptr->pwr_mgr_volt_tbl[i])/1000);

	pr_err("MSR retn voltage ID: 0x%x Val: %umV\n",
			volt_ptr->msr_retn & PMU_SR_VOLTAGE_MASK,
			bcmpmu_rgltr_get_volt_val(volt_ptr->msr_retn
			& PMU_SR_VOLTAGE_MASK)/1000);
	pr_err("SDSR1 Active voltage ID: 0x%x Val: %umV\n",
			volt_ptr->sdsr1[0] & PMU_SR_VOLTAGE_MASK,
			bcmpmu_rgltr_get_volt_val(volt_ptr->sdsr1[0]
			& PMU_SR_VOLTAGE_MASK)/1000);
	pr_err("SDSR1 retn voltage ID: 0x%x Val: %umV\n",
			volt_ptr->sdsr1[1] & PMU_SR_VOLTAGE_MASK,
			bcmpmu_rgltr_get_volt_val(volt_ptr->sdsr1[1]
			& PMU_SR_VOLTAGE_MASK)/1000);
	pr_err("SDSR2 Active voltage ID: 0x%x Val: %umV\n",
			volt_ptr->sdsr2[0] & PMU_SR_VOLTAGE_MASK,
			bcmpmu_rgltr_get_volt_val(volt_ptr->sdsr2[0]
			& PMU_SR_VOLTAGE_MASK)/1000);
	pr_err("SDSR2 retn voltage ID: 0x%x Val: %umV\n",
			volt_ptr->sdsr2[1] & PMU_SR_VOLTAGE_MASK,
			bcmpmu_rgltr_get_volt_val(volt_ptr->sdsr2[1]
			& PMU_SR_VOLTAGE_MASK)/1000);
	has_panicked = 1;
	return 0;
}

static struct notifier_block panic_block = {
	.notifier_call	= panic_event,
	.next		= NULL,
	.priority	= 200	/* priority: INT_MAX >= x >= 0 */
};


static int voltage_table_probe(void)
{
	pr_info("Voltage Table Probe\n");
	volt_dbg_log.sig_start = 0x01234567;
	volt_dbg_log.sig_end = 0x89ABCDEF;
	atomic_notifier_chain_register(&panic_notifier_list, &panic_block);
	return 0;
}

module_init(voltage_table_probe);

#ifdef CONFIG_DEBUG_FS
static int volt_tbl_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t read_volt_tbl_rev_id(struct file *file, char __user *user_buf,
		size_t count, loff_t *ppos)
{
	u32 len = 0;
	char debug_fs_buf[50];
	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"VOLTAGE TABLE REVISION ID:: %s\n", REV_ID);

	return simple_read_from_buffer(user_buf, count, ppos, debug_fs_buf,
			len);

}

static const struct file_operations volt_tbl_rev_id_fops = {
	.open = volt_tbl_open,
	.read = read_volt_tbl_rev_id,
};

static ssize_t read_volt_tbl(struct file *file, const char __user *buf,
				  size_t count, loff_t *ppos)
{
	u32 len = 0;
	u32 freq_id = A9_FREQ_1000_MHZ;
	u32 silicon_type = SILICON_TYPE_SLOW;
	char input_str[10];
	u8 *volt_table;
	int i;
	if (count > sizeof(input_str))
		len = sizeof(input_str);
	else
		len = count;

	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	/* coverity[secure_coding] */
	sscanf(input_str, "%d%d", &silicon_type, &freq_id);
	if (silicon_type >= SILICON_TYPE_MAX) {
		pr_err("%s: Invalid silicon type\n", __func__);
		return count;
	}
	/* Right now, 1.5Ghz voltage table hasn't been defined */
	if (freq_id >= A9_FREQ_1400_MHZ) {
		pr_err("%s: Invalid freq id\n", __func__);
		return count;
	}
	volt_table = (u8 *) get_sr_vlt_table(silicon_type, freq_id);
	pr_info("Silicon Type: %d, Freq Id: %d", silicon_type, freq_id);
	for (i = 0; i < SR_VLT_LUT_SIZE; i++)
		pr_info("[%x] = %x ", i, volt_table[i]);

	return count;
}

static ssize_t read_pmu_voltage_log(struct file *file, char __user
		*user_buf, size_t count, loff_t *ppos)
{
	int i;
	char debug_fs_buf[1000];
	u32 len = 0;

	struct pmu_volt_dbg *ptr = &volt_dbg_log;

	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
		"Reading the voltage table and sr voltages\n");
	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"Silicon Type: %u\n", ptr->si_type);
	for (i = 0; i < SR_VLT_LUT_SIZE; i++)
		len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"Tbl[%u] = 0x%x Val: %umV\n", i,
			ptr->pwr_mgr_volt_tbl[i], bcmpmu_rgltr_get_volt_val(
				ptr->pwr_mgr_volt_tbl[i])/1000);

	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"MSR retn voltage ID: 0x%x Val: %umV\n",
			ptr->msr_retn & PMU_SR_VOLTAGE_MASK,
			bcmpmu_rgltr_get_volt_val(ptr->msr_retn &
				PMU_SR_VOLTAGE_MASK)/1000);
	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"SDSR1 Active voltage ID: 0x%x Val: %umV\n",
			ptr->sdsr1[0] & PMU_SR_VOLTAGE_MASK,
			bcmpmu_rgltr_get_volt_val(ptr->sdsr1[0] &
				PMU_SR_VOLTAGE_MASK)/1000);
	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"SDSR1 retn voltage ID: 0x%x Val: %umV\n",
			ptr->sdsr1[1] & PMU_SR_VOLTAGE_MASK,
			bcmpmu_rgltr_get_volt_val(ptr->sdsr1[1] &
				PMU_SR_VOLTAGE_MASK)/1000);
	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"SDSR2 Active voltage ID: 0x%x Val: %umV\n",
			ptr->sdsr2[0] & PMU_SR_VOLTAGE_MASK,
			bcmpmu_rgltr_get_volt_val(ptr->sdsr2[0] &
				PMU_SR_VOLTAGE_MASK)/1000);
	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"SDSR2 retn voltage ID: 0x%x Val: %umV\n",
			ptr->sdsr2[1] & PMU_SR_VOLTAGE_MASK,
			bcmpmu_rgltr_get_volt_val(ptr->sdsr2[1] &
				PMU_SR_VOLTAGE_MASK)/1000);

	return simple_read_from_buffer(user_buf, count, ppos, debug_fs_buf,
				       len);
}

static const struct file_operations pmu_volt_log_fops = {
	.open = volt_tbl_open,
	.read = read_pmu_voltage_log,
};

static const struct file_operations volt_tbl_fops = {
	.open = volt_tbl_open,
	.write = read_volt_tbl,
};

static int volt_tbl_init(void)
{
	int ret;
	dent_vlt_root_dir = debugfs_create_dir("voltage_table", 0);
	if (!dent_vlt_root_dir)
		return -ENOMEM;

	if (!debugfs_create_file
	    ("rev_id", S_IRUGO, dent_vlt_root_dir, NULL,
	     &volt_tbl_rev_id_fops)) {
		debugfs_remove(dent_vlt_root_dir);
		return -ENOMEM;
	}

	if (!debugfs_create_file
	    ("volt_tbl", S_IRUGO, dent_vlt_root_dir, NULL,
	     &volt_tbl_fops)) {
		ret = -ENOMEM;
		goto remove_root_dir;
	}

	if (!debugfs_create_file("pmu_voltage_log", S_IRUGO,
			dent_vlt_root_dir, NULL, &pmu_volt_log_fops)) {
		ret = -ENOMEM;
		goto remove_root_dir;
	}

	pr_info("Voltage Table Debug Init Successs\n");
	return 0;

remove_root_dir:

	debugfs_remove_recursive(dent_vlt_root_dir);
	return ret;
}

late_initcall(volt_tbl_init);
#endif
