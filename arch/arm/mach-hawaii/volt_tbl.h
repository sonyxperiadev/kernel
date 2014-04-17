#include <linux/init.h>
#include "pm_params.h"

#define PMU_SR_VOLTAGE_MASK     0x3F

struct pmu_volt_dbg {
	u32 sig_start;
	u8 si_type;
	u8 msr_retn;
	u8 sdsr1[2];
	u8 sdsr2[2];
	u8 pwr_mgr_volt_tbl[SR_VLT_LUT_SIZE];
	u32 sig_end;
};

u8 *get_sr_vlt_table(u32 silicon_type, int freq_id);
int get_vddvar_retn_vlt_id(void);
int get_vddfix_vlt_adj(u32 vddfix_vlt);
int get_vddfix_retn_vlt_id(u32 reg_val);
void populate_pmu_voltage_log(void);
extern void bcmpmu_populate_volt_dbg_log(struct pmu_volt_dbg
		*dbg_log);
