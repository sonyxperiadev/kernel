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
/************************************************************************************************/
#include <linux/version.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <linux/module.h>
#include <asm/mach/arch.h>
#include <mach/io_map.h>
#include <mach/memory.h>
#include <mach/rdb/brcm_rdb_root_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#include <mach/rdb/brcm_rdb_mm_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_csr.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <plat/clock.h>
#include <mach/clock.h>
#include <mach/pi_mgr.h>
#include <plat/pwr_mgr.h>
#include <plat/pi_mgr.h>

#include "pm_params.h"

#define RUN_POLICY PM_POLICY_5
#define RETN_POLICY PM_POLICY_1
#define SHTDWN_POLICY PM_POLICY_0


#define OPP_ECONOMY_STRING	"ECONOMY"
#define OPP_NORMAL_STRING	"NORMAL"
#define OPP_TURBO_STRING	"TURBO"


#define	ARM_PI_NUM_OPP			3
#define	MM_PI_NUM_OPP			3
#define	HUB_PI_NUM_OPP			2
#define	AON_PI_NUM_OPP			2
#define	SUB_SYS_PI_NUM_OPP		2


#define PI_STATE(state_id,policy,latency, flg) \
		{.id = state_id,.state_policy = policy,\
		.hw_wakeup_latency = latency,.flags = flg}

#ifdef CONFIG_RHEA_WA_HWJIRA_2221
extern char *noncache_buf_va;
#endif
#ifdef CONFIG_RHEA_WA_HWJIRA_2490
static struct clk *ref_8ph_en_pll1_clk;
#endif

char *armc_core_ccu[] = { KPROC_CCU_CLK_NAME_STR };

struct pi_opp arm_opp = {
	.opp = {
		[PI_OPP_ECONOMY] = PROC_CCU_FREQ_ID_ECO,
		[PI_OPP_NORMAL] = PROC_CCU_FREQ_ID_NRML,
		[PI_OPP_TURBO] = PROC_CCU_FREQ_ID_TURBO,
		},
};

static struct pi_state arm_core_states[] = {
	PI_STATE(ARM_CORE_STATE_ACTIVE, RUN_POLICY, 0, 0),
	PI_STATE(ARM_CORE_STATE_SUSPEND, RUN_POLICY, 0, 0),
#ifdef CONFIG_RHEA_A9_RETENTION_CSTATE
	PI_STATE(ARM_CORE_STATE_RETENTION, RETN_POLICY, 100, 0),
#endif
	PI_STATE(ARM_CORE_STATE_DORMANT, RETN_POLICY, 10000, 0),

};
#ifdef CONFIG_KONA_PI_DFS_STATS
static cputime64_t arm_core_time_in_state[ARM_PI_NUM_OPP];
static u32 arm_core_trans_table[ARM_PI_NUM_OPP * ARM_PI_NUM_OPP];
#endif

static struct pi arm_core_pi = {
	.name = "arm_core",
	.id = PI_MGR_PI_ID_ARM_CORE,
#ifdef CONFIG_RHEA_PI_MGR_DISABLE_POLICY_CHANGE
	.flags = PI_ENABLE_ON_INIT | PI_ARM_CORE | UPDATE_PM_QOS |
	    NO_POLICY_CHANGE | DFS_LIMIT_CHECK_EN,
#else
	.flags = PI_ENABLE_ON_INIT | PI_ARM_CORE | UPDATE_PM_QOS |
	    DFS_LIMIT_CHECK_EN,
#endif
	.ccu_id = armc_core_ccu,
	.num_ccu_id = ARRAY_SIZE(armc_core_ccu),
	.state_allowed = ARM_CORE_STATE_DORMANT,
	.pi_state = arm_core_states,
	.num_states = ARRAY_SIZE(arm_core_states),
	.opp_active = 2,
	.opp_lmt_max = 2,
	.opp_lmt_min = 0,
	.pi_opp = &arm_opp,
	.num_opp = ARM_PI_NUM_OPP,
	.qos_sw_event_id = SOFTWARE_0_EVENT,

	.pi_info = {
		    .policy_reg_offset = PWRMGR_LCDTE_VI_ARM_CORE_POLICY_OFFSET,
		    .ac_shift =
		    PWRMGR_LCDTE_VI_ARM_CORE_POLICY_LCDTE_PI_ARM_CORE_PM_AC_SHIFT,
		    .atl_shift =
		    PWRMGR_LCDTE_VI_ARM_CORE_POLICY_LCDTE_PI_ARM_CORE_PM_ATL_SHIFT,
		    .pm_policy_shift =
		    PWRMGR_LCDTE_VI_ARM_CORE_POLICY_LCDTE_PI_ARM_CORE_PM_POLICY_SHIFT,

		    .fixed_vol_map_mask =
		    PWRMGR_FIXED_VOLTAGE_MAP_VI_ARM_CORE_FIXED_VOLTAGE_MAP_MASK,
		    .vi_to_vOx_map_mask =
		    PWRMGR_VI_TO_VO0_MAP_VI_ARM_CORE_TO_VO0_MAP_MASK,
		    .wakeup_overide_mask =
		    PWRMGR_PI_DEFAULT_POWER_STATE_PI_ARM_CORE_WAKEUP_OVERRIDE_MASK,
		    .counter_reg_offset = PWRMGR_PI_ARM_CORE_ON_COUNTER_OFFSET,
		    .rtn_clmp_dis_mask =
		    PWRMGR_PI_DEFAULT_POWER_STATE_PI_ARM_CORE_RETENTION_CLAMP_DISABLE_MASK,
		    },
#ifdef CONFIG_KONA_PI_DFS_STATS
	.pi_dfs_stats = {
		.qos_pi_id = PI_MGR_PI_ID_ARM_CORE,
		.time_in_state = arm_core_time_in_state,
		.trans_table = arm_core_trans_table,
	},
#endif
	.ops = &gen_pi_ops,
};

/*MM PI CCU Id*/
static char *mm_ccu[] = { MM_CCU_CLK_NAME_STR };

struct pi_opp mm_opp = {
	.opp = {
		[PI_OPP_ECONOMY] = MM_CCU_FREQ_ID_ECO,
		[PI_OPP_NORMAL] = MM_CCU_FREQ_ID_NRML,
		[PI_OPP_TURBO] = MM_CCU_FREQ_ID_TURBO,
		},
};

static struct pi_state mm_states[] = {
	PI_STATE(PI_STATE_ACTIVE, RUN_POLICY, 0, 0),
	PI_STATE(PI_STATE_RETENTION, RETN_POLICY, 10, 0),
	PI_STATE(PI_STATE_SHUTDOWN, SHTDWN_POLICY, 100, PI_STATE_SAVE_CONTEXT),
};

#ifdef CONFIG_RHEA_WA_HWJIRA_2490

static int mm_pi_enable(struct pi *pi, int enable)
{
	int ret;
	pi_dbg(pi->id, PI_LOG_EN_DIS, "%s\n", __func__);
	if (JIRA_WA_ENABLED(2490)) {
		if (enable && ref_8ph_en_pll1_clk)
			__clk_enable(ref_8ph_en_pll1_clk);
	}
	ret = gen_pi_ops.enable(pi, enable);
	if (JIRA_WA_ENABLED(2490)) {
		if (!enable && ref_8ph_en_pll1_clk)
			__clk_disable(ref_8ph_en_pll1_clk);
	}
	return ret;
}
static struct pi_ops mm_pi_ops;
#endif

#ifdef CONFIG_KONA_PI_DFS_STATS
static cputime64_t mm_time_in_state[MM_PI_NUM_OPP];
static u32 mm_trans_table[MM_PI_NUM_OPP * MM_PI_NUM_OPP];
#endif
static struct pi mm_pi = {
	.name = "mm",
	.id = PI_MGR_PI_ID_MM,
#ifdef CONFIG_RHEA_PI_MGR_DISABLE_POLICY_CHANGE
	.flags = NO_POLICY_CHANGE | DFS_LIMIT_CHECK_EN,
#else
	.flags = DFS_LIMIT_CHECK_EN,
#endif
	.ccu_id = mm_ccu,
	.num_ccu_id = ARRAY_SIZE(mm_ccu),
	.state_allowed = PI_STATE_RETENTION,
	.pi_state = mm_states,
	.num_states = ARRAY_SIZE(mm_states),
	.opp_active = 0,
	.pi_opp = &mm_opp,
	.opp_def_weightage = {
			      [PI_OPP_ECONOMY] = 35,
			      [PI_OPP_NORMAL] = 50,
			      },
	.num_opp = MM_PI_NUM_OPP,
	.opp_lmt_max = 2,
	.opp_lmt_min = 0,
	.qos_sw_event_id = SOFTWARE_0_EVENT,

	.pi_info = {
		    .policy_reg_offset = PWRMGR_LCDTE_VI_MM_POLICY_OFFSET,
		    .ac_shift =
		    PWRMGR_LCDTE_VI_MM_POLICY_LCDTE_PI_MM_PM_AC_SHIFT,
		    .atl_shift =
		    PWRMGR_LCDTE_VI_MM_POLICY_LCDTE_PI_MM_PM_ATL_SHIFT,
		    .pm_policy_shift =
		    PWRMGR_LCDTE_VI_MM_POLICY_LCDTE_PI_MM_PM_POLICY_SHIFT,

		    .fixed_vol_map_mask =
		    PWRMGR_FIXED_VOLTAGE_MAP_VI_MM_FIXED_VOLTAGE_MAP_MASK,
		    .vi_to_vOx_map_mask =
		    PWRMGR_VI_TO_VO0_MAP_VI_MM_TO_VO0_MAP_MASK,
		    .wakeup_overide_mask =
		    PWRMGR_PI_DEFAULT_POWER_STATE_PI_MM_WAKEUP_OVERRIDE_MASK,
		    .counter_reg_offset = PWRMGR_PI_MM_ON_COUNTER_OFFSET,
		    .rtn_clmp_dis_mask =
		    PWRMGR_PI_DEFAULT_POWER_STATE_PI_MM_RETENTION_CLAMP_DISABLE_MASK,
		    .reset_mgr_ccu_name = ROOT_CCU_CLK_NAME_STR,
		    .pd_soft_reset_offset =
		    ROOT_RST_MGR_REG_PD_SOFT_RSTN_OFFSET,
		    .pd_reset_mask0 =
		    ROOT_RST_MGR_REG_PD_SOFT_RSTN_MM_SOFT_RSTN_MASK,
		    .pd_reset_mask1 =
		    ROOT_RST_MGR_REG_PD_SOFT_RSTN_MM_SUB_SOFT_RSTN_MASK,
		    },
#ifdef CONFIG_KONA_PI_DFS_STATS
	.pi_dfs_stats = {
		.qos_pi_id = PI_MGR_PI_ID_MM,
		.time_in_state = mm_time_in_state,
		.trans_table = mm_trans_table,
	},
#endif
#ifdef CONFIG_RHEA_WA_HWJIRA_2490
	.ops = &mm_pi_ops,
#else
	.ops = &gen_pi_ops,
#endif
};

/*HUB PI CCU Id*/
static char *hub_ccu[] = { KHUB_CCU_CLK_NAME_STR };

struct pi_opp hub_opp = {
	.opp = {
		[PI_OPP_ECONOMY] = HUB_CCU_FREQ_ID_ECO,	/* 0 */
		[PI_OPP_NORMAL] = HUB_CCU_FREQ_ID_NRML,
		},
};

static struct pi_state hub_states[] = {
	PI_STATE(PI_STATE_ACTIVE, RUN_POLICY, 0, 0),
	PI_STATE(PI_STATE_RETENTION, RETN_POLICY, 100, 0),

};
#ifdef CONFIG_KONA_PI_DFS_STATS
static cputime64_t hub_time_in_state[HUB_PI_NUM_OPP];
static u32 hub_trans_table[HUB_PI_NUM_OPP * HUB_PI_NUM_OPP];
#endif
static struct pi hub_pi = {
	.name = "hub",
	.id = PI_MGR_PI_ID_HUB_SWITCHABLE,
#ifdef CONFIG_RHEA_PI_MGR_DISABLE_POLICY_CHANGE
	.flags = NO_POLICY_CHANGE | DFS_LIMIT_CHECK_EN,
#else
	.flags = DFS_LIMIT_CHECK_EN,
#endif
	.ccu_id = hub_ccu,
	.num_ccu_id = ARRAY_SIZE(hub_ccu),
	.state_allowed = PI_STATE_RETENTION,
	.pi_state = hub_states,
	.num_states = ARRAY_SIZE(hub_states),
	.opp_active = 0,
	.pi_opp = &hub_opp,
	.num_opp = HUB_PI_NUM_OPP,
	.opp_lmt_max = 1,
	.opp_lmt_min = 0,
	.opp_def_weightage = {
			      [PI_OPP_ECONOMY] = 25,
			      },

	.qos_sw_event_id = SOFTWARE_0_EVENT,

	.pi_info = {
		    .policy_reg_offset = PWRMGR_LCDTE_VI_HUB_POLICY_OFFSET,
		    .ac_shift =
		    PWRMGR_LCDTE_VI_HUB_POLICY_LCDTE_PI_HUB_SWITCHABLE_PM_AC_SHIFT,
		    .atl_shift =
		    PWRMGR_LCDTE_VI_HUB_POLICY_LCDTE_PI_HUB_SWITCHABLE_PM_ATL_SHIFT,
		    .pm_policy_shift =
		    PWRMGR_LCDTE_VI_HUB_POLICY_LCDTE_PI_HUB_SWITCHABLE_PM_POLICY_SHIFT,

		    .fixed_vol_map_mask =
		    PWRMGR_FIXED_VOLTAGE_MAP_VI_HUB_FIXED_VOLTAGE_MAP_MASK,
		    .vi_to_vOx_map_mask =
		    PWRMGR_VI_TO_VO0_MAP_VI_HUB_TO_VO0_MAP_MASK,
		    .wakeup_overide_mask =
		    PWRMGR_PI_DEFAULT_POWER_STATE_PI_HUB_SWITCHABLE_WAKEUP_OVERRIDE_MASK,
		    .counter_reg_offset =
		    PWRMGR_PI_HUB_SWITCHABLE_ON_COUNTER_OFFSET,
		    .rtn_clmp_dis_mask =
		    PWRMGR_PI_DEFAULT_POWER_STATE_PI_HUB_SWITCHABLE_RETENTION_CLAMP_DISABLE_MASK,
		    .reset_mgr_ccu_name = ROOT_CCU_CLK_NAME_STR,
		    .pd_soft_reset_offset =
		    ROOT_RST_MGR_REG_PD_SOFT_RSTN_OFFSET,
		    .pd_reset_mask0 =
		    ROOT_RST_MGR_REG_PD_SOFT_RSTN_HUB_SOFT_RSTN_MASK,

		    },
#ifdef CONFIG_KONA_PI_DFS_STATS
	.pi_dfs_stats = {
		.qos_pi_id = PI_MGR_PI_ID_ARM_CORE,
		.time_in_state = hub_time_in_state,
		.trans_table = hub_trans_table,
	},
#endif
	.ops = &gen_pi_ops,
};

/*AON PI CCU Id*/
static char *aon_ccu[] = { KHUBAON_CCU_CLK_NAME_STR };

struct pi_opp aon_opp = {
	.opp = {
		[PI_OPP_ECONOMY] = AON_CCU_FREQ_ID_ECO,	/* 0 */
		[PI_OPP_NORMAL] = AON_CCU_FREQ_ID_NRML,
		},
};

static struct pi_state aon_states[] = {
	PI_STATE(PI_STATE_ACTIVE, RUN_POLICY, 0, 0),
	PI_STATE(PI_STATE_RETENTION, RETN_POLICY, 100, 0),
};

#ifdef CONFIG_KONA_PI_DFS_STATS
static cputime64_t aon_time_in_state[AON_PI_NUM_OPP];
static u32 aon_trans_table[AON_PI_NUM_OPP * AON_PI_NUM_OPP];
#endif
static struct pi aon_pi = {
	.name = "aon",
	.id = PI_MGR_PI_ID_HUB_AON,
#ifdef CONFIG_RHEA_PI_MGR_DISABLE_POLICY_CHANGE
	.flags = NO_POLICY_CHANGE | DFS_LIMIT_CHECK_EN,
#else
	.flags = DFS_LIMIT_CHECK_EN,
#endif
	.ccu_id = aon_ccu,
	.num_ccu_id = ARRAY_SIZE(aon_ccu),
	.state_allowed = PI_STATE_RETENTION,
	.pi_state = aon_states,
	.num_states = ARRAY_SIZE(aon_states),
	.opp_active = 0,
	/*opp frequnecies ...need to revisit */
	.pi_opp = &aon_opp,
	.num_opp = AON_PI_NUM_OPP,
	.opp_lmt_max = 1,
	.opp_lmt_min = 0,
	.qos_sw_event_id = SOFTWARE_0_EVENT,

	.pi_info = {
		    .policy_reg_offset = PWRMGR_LCDTE_VI_HUB_POLICY_OFFSET,
		    .ac_shift =
		    PWRMGR_LCDTE_VI_HUB_POLICY_LCDTE_PI_HUB_AON_PM_AC_SHIFT,
		    .atl_shift =
		    PWRMGR_LCDTE_VI_HUB_POLICY_LCDTE_PI_HUB_AON_PM_ATL_SHIFT,
		    .pm_policy_shift =
		    PWRMGR_LCDTE_VI_HUB_POLICY_LCDTE_PI_HUB_AON_PM_POLICY_SHIFT,

		    .fixed_vol_map_mask =
		    PWRMGR_FIXED_VOLTAGE_MAP_VI_HUB_FIXED_VOLTAGE_MAP_MASK,
		    .vi_to_vOx_map_mask =
		    PWRMGR_VI_TO_VO0_MAP_VI_HUB_TO_VO0_MAP_MASK,
		    .wakeup_overide_mask =
		    PWRMGR_PI_DEFAULT_POWER_STATE_PI_HUB_AON_WAKEUP_OVERRIDE_MASK,
		    .counter_reg_offset = PWRMGR_PI_HUB_AON_ON_COUNTER_OFFSET,
		    .rtn_clmp_dis_mask =
		    PWRMGR_PI_DEFAULT_POWER_STATE_PI_HUB_AON_RETENTION_CLAMP_DISABLE_MASK,
		    },
#ifdef CONFIG_KONA_PI_DFS_STATS
	.pi_dfs_stats = {
		.qos_pi_id = PI_MGR_PI_ID_ARM_CORE,
		.time_in_state = aon_time_in_state,
		.trans_table = aon_trans_table,
	},
#endif
	.ops = &gen_pi_ops,
};

/*ARM subsystem PI Id*/
static char *sub_sys_ccu[] = { KPM_CCU_CLK_NAME_STR, KPS_CCU_CLK_NAME_STR };

struct pi_opp sub_sys_opp[2] = {
	[0] = { /*KPM*/.opp = {
			       [PI_OPP_ECONOMY] = KPM_CCU_FREQ_ID_ECO,
			       [PI_OPP_NORMAL] = KPM_CCU_FREQ_ID_NRML,
			       },
	       },
	[1] = { /*KPS*/.opp = {
			       [PI_OPP_ECONOMY] = KPS_CCU_FREQ_ID_ECO,
			       [PI_OPP_NORMAL] = KPS_CCU_FREQ_ID_NRML,
			       },
	       },

};

static struct pi_state sub_sys_states[] = {
	PI_STATE(PI_STATE_ACTIVE, RUN_POLICY, 0, 0),
	PI_STATE(PI_STATE_RETENTION, RETN_POLICY, 100, 0),

};
#ifdef CONFIG_KONA_PI_DFS_STATS
static cputime64_t subsys_time_in_state[SUB_SYS_PI_NUM_OPP];
static u32 subsys_trans_table[SUB_SYS_PI_NUM_OPP * SUB_SYS_PI_NUM_OPP];
#endif
/*

*/
#ifdef CONFIG_RHEA_WA_HWJIRA_2276
/*A9 sometimes freezes on exit from dormant. Recommended
workaround is to tie A9 & Fabric together to avoid A9 -> fabric
glitch when A9 is powered off.

With this workaround A9 enters dormant only when fabric is in
retention */

static u32 sub_sys_dep_pi[] = {
	PI_MGR_PI_ID_ARM_CORE,
};
#endif /*CONFIG_RHEA_WA_HWJIRA_2276 */
static struct pi sub_sys_pi = {
	.name = "sub_sys",
	.id = PI_MGR_PI_ID_ARM_SUB_SYSTEM,
	.ccu_id = sub_sys_ccu,
#ifdef CONFIG_RHEA_PI_MGR_DISABLE_POLICY_CHANGE
	.flags = NO_POLICY_CHANGE | DFS_LIMIT_CHECK_EN,
#else
	.flags = DFS_LIMIT_CHECK_EN,
#endif
	.num_ccu_id = ARRAY_SIZE(sub_sys_ccu),
	.state_allowed = PI_STATE_RETENTION,
#ifdef CONFIG_RHEA_WA_HWJIRA_2276
	.dep_pi = sub_sys_dep_pi,
	.num_dep_pi = ARRAY_SIZE(sub_sys_dep_pi),
#endif /*CONFIG_RHEA_WA_HWJIRA_2276 */
	.pi_state = sub_sys_states,
	.num_states = ARRAY_SIZE(sub_sys_states),
	.opp_active = 0,
	.pi_opp = sub_sys_opp,
	.num_opp = SUB_SYS_PI_NUM_OPP,
	.opp_lmt_max = 1,
	.opp_lmt_min = 0,
	.opp_def_weightage = {
			      [PI_OPP_ECONOMY] = 25,
			      },

	.qos_sw_event_id = SOFTWARE_0_EVENT,

	.pi_info = {
		    .policy_reg_offset =
		    PWRMGR_LCDTE_VI_ARM_SUBSYSTEM_POLICY_OFFSET,
		    .ac_shift =
		    PWRMGR_LCDTE_VI_ARM_SUBSYSTEM_POLICY_LCDTE_PI_ARM_SUBSYSTEM_PM_AC_SHIFT,
		    .atl_shift =
		    PWRMGR_LCDTE_VI_ARM_SUBSYSTEM_POLICY_LCDTE_PI_ARM_SUBSYSTEM_PM_ATL_SHIFT,
		    .pm_policy_shift =
		    PWRMGR_LCDTE_VI_ARM_SUBSYSTEM_POLICY_LCDTE_PI_ARM_SUBSYSTEM_PM_POLICY_SHIFT,

		    .fixed_vol_map_mask =
		    PWRMGR_FIXED_VOLTAGE_MAP_VI_ARM_SUBSYSTEM_FIXED_VOLTAGE_MAP_SHIFT,
		    .vi_to_vOx_map_mask =
		    PWRMGR_VI_TO_VO0_MAP_VI_ARM_SUBSYSTEM_TO_VO0_MAP_MASK,
		    .wakeup_overide_mask =
		    PWRMGR_PI_DEFAULT_POWER_STATE_PI_ARM_SUBSYSTEM_WAKEUP_OVERRIDE_MASK,
		    .counter_reg_offset =
		    PWRMGR_PI_ARM_SUBSYSTEM_ON_COUNTER_OFFSET,
		    .rtn_clmp_dis_mask =
		    PWRMGR_PI_DEFAULT_POWER_STATE_PI_ARM_SUBSYSTEM_RETENTION_CLAMP_DISABLE_MASK,
		    .reset_mgr_ccu_name = ROOT_CCU_CLK_NAME_STR,
		    .pd_soft_reset_offset =
		    ROOT_RST_MGR_REG_PD_SOFT_RSTN_OFFSET,
		    .pd_reset_mask0 =
		    ROOT_RST_MGR_REG_PD_SOFT_RSTN_KSLV_SOFT_RSTN_MASK,
		    .pd_reset_mask1 =
		    ROOT_RST_MGR_REG_PD_SOFT_RSTN_KMST_SOFT_RSTN_MASK,

		    },
#ifdef CONFIG_KONA_PI_DFS_STATS
	.pi_dfs_stats = {
		.qos_pi_id = PI_MGR_PI_ID_ARM_CORE,
		.time_in_state = subsys_time_in_state,
		.trans_table = subsys_trans_table,
	},
#endif
	.ops = &gen_pi_ops,
};

/*MODEM CCU -  ADDED for initializing EVENT table only*/

static struct pi_state modem_states[] = {
	PI_STATE(PI_STATE_ACTIVE, RUN_POLICY, 0, 0),

};

static struct pi modem_pi = {
	.name = "modem",
	.id = PI_MGR_PI_ID_MODEM,
	.flags = PI_NO_DFS | PI_NO_QOS,
	.pi_state = modem_states,
	.num_states = ARRAY_SIZE(modem_states),
	.pi_info = {
		    .policy_reg_offset = PWRMGR_LCDTE_VI_ARM_CORE_POLICY_OFFSET,
		    .ac_shift =
		    PWRMGR_LCDTE_VI_ARM_CORE_POLICY_LCDTE_PI_MODEM_PM_AC_SHIFT,
		    .atl_shift =
		    PWRMGR_LCDTE_VI_ARM_CORE_POLICY_LCDTE_PI_MODEM_PM_ATL_SHIFT,
		    .pm_policy_shift =
		    PWRMGR_LCDTE_VI_ARM_CORE_POLICY_LCDTE_PI_MODEM_PM_POLICY_SHIFT,

		    .fixed_vol_map_mask =
		    PWRMGR_FIXED_VOLTAGE_MAP_VI_MODEM_FIXED_VOLTAGE_MAP_SHIFT,
		    .vi_to_vOx_map_mask =
		    PWRMGR_VI_TO_VO0_MAP_VI_MODEM_TO_VO0_MAP_MASK,
		    .wakeup_overide_mask =
		    PWRMGR_PI_DEFAULT_POWER_STATE_PI_MODEM_WAKEUP_OVERRIDE_MASK,
		    .counter_reg_offset = PWRMGR_PI_MODEM_ON_COUNTER_OFFSET,
		    .rtn_clmp_dis_mask =
		    PWRMGR_PI_DEFAULT_POWER_STATE_PI_MODEM_RETENTION_CLAMP_DISABLE_MASK,
		    .reset_mgr_ccu_name = ROOT_CCU_CLK_NAME_STR,
		    .pd_soft_reset_offset =
		    ROOT_RST_MGR_REG_PD_SOFT_RSTN_OFFSET,
		    .pd_reset_mask0 =
		    ROOT_RST_MGR_REG_PD_SOFT_RSTN_MDM_SOFT_RSTN_MASK,
		    .pd_reset_mask1 =
		    ROOT_RST_MGR_REG_PD_SOFT_RSTN_MDM_SUB_SOFT_RSTN_MASK,

		    },
	.ops = NULL,
};

struct pi *pi_list[] = {
	&arm_core_pi,
	&mm_pi,
	&hub_pi,
	&aon_pi,
	&sub_sys_pi,
	&modem_pi
};

char *get_opp_name(int opp)
{
	char *name = NULL;
	switch (opp) {
	case PI_OPP_ECONOMY:
		name = OPP_ECONOMY_STRING;
		break;
	case PI_OPP_NORMAL:
		name = OPP_NORMAL_STRING;
		break;
	case PI_OPP_TURBO:
		name = OPP_TURBO_STRING;
		break;
	default:
		return NULL;
	}

	return name;
}

int rhea_pi_mgr_print_act_pis(void)
{
	pr_info("\n*** ACTIVE PIs DURING SUSPEND ***\n");
	pr_info("\tPI \t STATE \t USE_CNT\n");
	pi_mgr_print_active_pis();
	pr_info("**********************************\n");
	return 0;
}
EXPORT_SYMBOL(rhea_pi_mgr_print_act_pis);

void __init rhea_pi_mgr_init()
{
	int i;
#ifdef CONFIG_RHEA_WA_HWJIRA_2276
	if (!JIRA_WA_ENABLED(2276)) {

		sub_sys_pi.dep_pi = NULL;
		sub_sys_pi.num_dep_pi = 0;
	}
#endif /*CONFIG_RHEA_WA_HWJIRA_2276 */
#ifdef CONFIG_RHEA_WA_HWJIRA_2490
		mm_pi_ops = gen_pi_ops;
		mm_pi_ops.enable = mm_pi_enable;
#endif
	pi_mgr_init();

	for (i = 0; i < ARRAY_SIZE(pi_list); i++) {
		pr_info("%s: PI:%s state[0] = %x %x\n", __func__,
			pi_list[i]->name, pi_list[i]->pi_state[1].id,
			pi_list[i]->pi_state[1].state_policy);
		pi_mgr_register(pi_list[i]);
	}

}

EXPORT_SYMBOL(rhea_pi_mgr_init);

#ifdef CONFIG_RHEA_WA_HWJIRA_2490
static int __init rhea_mm_pre_init_state(void)
{
	if (JIRA_WA_ENABLED(2490)) {
		if (ref_8ph_en_pll1_clk == NULL) {
			ref_8ph_en_pll1_clk = clk_get(NULL,
				REF_8PHASE_EN_PLL1_CLK_NAME_STR);
		}
		BUG_ON(IS_ERR_OR_NULL(ref_8ph_en_pll1_clk));
	}

	return 0;
}
arch_initcall(rhea_mm_pre_init_state);
#endif

int __init pi_mgr_late_init(void)
{
#ifdef CONFIG_DEBUG_FS
	int i;
	pi_debug_init();
	for (i = 0; i < ARRAY_SIZE(pi_list); i++) {
		/*add debug interface for all domains except for modem */
		if (pi_list[i]->id != PI_MGR_PI_ID_MODEM)
			pi_debug_add_pi(pi_list[i]);
	}
#endif /* CONFIG_DEBUG_FS */

	return 0;
}

late_initcall_sync(pi_mgr_late_init);
