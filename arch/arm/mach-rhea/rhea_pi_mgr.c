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
#include <linux/sysdev.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <asm/mach/arch.h>
#include <mach/io_map.h>

#include<mach/clock.h>
#include<mach/pi_mgr.h>
#include<plat/pwr_mgr.h>
#include<plat/pi_mgr.h>

#define RUN_POLICY PM_POLICY_5
#define RETN_POLICY PM_POLICY_5
#define SHTDWN_POLICY PM_POLICY_0

#define PI_STATE(state_id,policy,latency) \
		{.id = state_id,.state_policy = policy,.hw_wakeup_latency = latency,}



/*ARM core PI CCU Ids  -- TBD*/
char* armc_core_ccu[] = {NULL};
/*ARM core PI states  -- TBD*/

static struct pi_state arm_core_states[] =
		{
			PI_STATE(ARM_CORE_STATE_ACTIVE,RUN_POLICY,0),
			PI_STATE(ARM_CORE_STATE_SUSPEND,RUN_POLICY,0),
			PI_STATE(ARM_CORE_STATE_RETENTION,RETN_POLICY,100),
			PI_STATE(ARM_CORE_STATE_DORMANT,RETN_POLICY,10000),
			PI_STATE(PI_MGR_STATE_UNSUPPORTED,0,0),

		};

static struct pi arm_core_pi =
	{
		.name = "arm_core",
		.id = PI_MGR_PI_ID_ARM_CORE,
		.flags = PI_ARM_CORE,
		.ccu_id = armc_core_ccu,
		.state_allowed = ARM_CORE_STATE_DORMANT,
		.pi_state = arm_core_states,
		.opp_active = 0,
		/*opp frequnecies ...need to revisit*/
		.opp =  {
					[PI_OPP_ECONOMY] = 3,
					[PI_OPP_NORMAL] = 6,
					[PI_OPP_TURBO] = 7,
				},
		.sw_event_id = SOFTWARE_0_EVENT,
		.pi_info =
				{
					.policy_reg_offset = PWRMGR_LCDTE_VI_ARM_CORE_POLICY_OFFSET,
					.ac_shift = PWRMGR_LCDTE_VI_ARM_CORE_POLICY_LCDTE_PI_ARM_CORE_PM_AC_SHIFT,
					.atl_shift = PWRMGR_LCDTE_VI_ARM_CORE_POLICY_LCDTE_PI_ARM_CORE_PM_ATL_SHIFT,
					.pm_policy_shift = PWRMGR_LCDTE_VI_ARM_CORE_POLICY_LCDTE_PI_ARM_CORE_PM_POLICY_SHIFT,

					.fixed_vol_map_mask = PWRMGR_FIXED_VOLTAGE_MAP_VI_ARM_CORE_FIXED_VOLTAGE_MAP_MASK,
					.vi_to_vOx_map_mask = PWRMGR_VI_TO_VO0_MAP_VI_ARM_CORE_TO_VO0_MAP_MASK,
					.wakeup_overide_mask = PWRMGR_PI_DEFAULT_POWER_STATE_PI_ARM_CORE_WAKEUP_OVERRIDE_MASK,
					.counter_reg_offset = PWRMGR_PI_ARM_CORE_ON_COUNTER_OFFSET,
					.rtn_clmp_dis_mask = PWRMGR_PI_DEFAULT_POWER_STATE_PI_ARM_CORE_RETENTION_CLAMP_DISABLE_MASK,
				},
		.ops = &gen_pi_ops,
	};

/*MM PI CCU Id*/
static char* mm_ccu[] = {MM_CCU_CLK_NAME_STR,NULL};

static struct pi_state mm_states[] =
		{
			PI_STATE(PI_STATE_ACTIVE,RUN_POLICY,0),
			PI_STATE(PI_STATE_RETENTION,RETN_POLICY,100),
			//PI_STATE(PI_STATE_SHUTDOWN,SHTDWN_POLICY,100),
			PI_STATE(PI_MGR_STATE_UNSUPPORTED,0,0),

		};


static struct pi mm_pi =
	{
		.name = "mm",
		.id = PI_MGR_PI_ID_MM,
		//.flags = PI_DISABLE_ON_INIT,
		.ccu_id = mm_ccu,
		.state_allowed = PI_STATE_RETENTION,
		.pi_state = mm_states,
		.opp_active = 0,
		/*opp frequnecies ...need to revisit*/
		.opp =  {
					[PI_OPP_ECONOMY] = 1,
					[PI_OPP_NORMAL] = 1,
					[PI_OPP_TURBO] = 1,
				},
		.sw_event_id = SOFTWARE_0_EVENT,
		.pi_info =
				{
					.policy_reg_offset = PWRMGR_LCDTE_VI_MM_POLICY_OFFSET,
					.ac_shift = PWRMGR_LCDTE_VI_MM_POLICY_LCDTE_PI_MM_PM_AC_SHIFT,
					.atl_shift = PWRMGR_LCDTE_VI_MM_POLICY_LCDTE_PI_MM_PM_ATL_SHIFT,
					.pm_policy_shift = PWRMGR_LCDTE_VI_MM_POLICY_LCDTE_PI_MM_PM_POLICY_SHIFT,

					.fixed_vol_map_mask = PWRMGR_FIXED_VOLTAGE_MAP_VI_MM_FIXED_VOLTAGE_MAP_MASK,
					.vi_to_vOx_map_mask = PWRMGR_VI_TO_VO0_MAP_VI_MM_TO_VO0_MAP_MASK,
					.wakeup_overide_mask = PWRMGR_PI_DEFAULT_POWER_STATE_PI_MM_WAKEUP_OVERRIDE_MASK,
					.counter_reg_offset = PWRMGR_PI_MM_ON_COUNTER_OFFSET,
					.rtn_clmp_dis_mask = PWRMGR_PI_DEFAULT_POWER_STATE_PI_MM_RETENTION_CLAMP_DISABLE_MASK,
				},
		.ops = &gen_pi_ops,
	};

/*HUB PI CCU Id*/
static char* hub_ccu[] = {KHUB_CCU_CLK_NAME_STR,NULL};

static struct pi_state hub_states[] =
		{
			PI_STATE(PI_STATE_ACTIVE,RUN_POLICY,0),
			PI_STATE(PI_STATE_RETENTION,RETN_POLICY,100),
			PI_STATE(PI_MGR_STATE_UNSUPPORTED,0,0),

		};


static struct pi hub_pi =
	{
		.name = "hub",
		.id = PI_MGR_PI_ID_HUB_SWITCHABLE,
		.ccu_id = hub_ccu,
		.state_allowed = PI_STATE_RETENTION,
		.pi_state = hub_states,
		.opp_active = 0,
		/*opp frequnecies ...need to revisit*/
		.opp =  {
					[PI_OPP_ECONOMY] = 2,
					[PI_OPP_NORMAL] = 2,
					[PI_OPP_TURBO] = 2,
				},
		.sw_event_id = SOFTWARE_0_EVENT,
		.pi_info =
				{
					.policy_reg_offset = PWRMGR_LCDTE_VI_HUB_POLICY_OFFSET,
					.ac_shift = PWRMGR_LCDTE_VI_HUB_POLICY_LCDTE_PI_HUB_SWITCHABLE_PM_AC_SHIFT,
					.atl_shift = PWRMGR_LCDTE_VI_HUB_POLICY_LCDTE_PI_HUB_SWITCHABLE_PM_ATL_SHIFT,
					.pm_policy_shift = PWRMGR_LCDTE_VI_HUB_POLICY_LCDTE_PI_HUB_SWITCHABLE_PM_POLICY_SHIFT,

					.fixed_vol_map_mask = PWRMGR_FIXED_VOLTAGE_MAP_VI_HUB_FIXED_VOLTAGE_MAP_MASK,
					.vi_to_vOx_map_mask = PWRMGR_VI_TO_VO0_MAP_VI_HUB_TO_VO0_MAP_MASK,
					.wakeup_overide_mask = PWRMGR_PI_DEFAULT_POWER_STATE_PI_HUB_SWITCHABLE_WAKEUP_OVERRIDE_MASK,
					.counter_reg_offset = PWRMGR_PI_HUB_SWITCHABLE_ON_COUNTER_OFFSET,
					.rtn_clmp_dis_mask = PWRMGR_PI_DEFAULT_POWER_STATE_PI_HUB_SWITCHABLE_RETENTION_CLAMP_DISABLE_MASK,
				},
		.ops = &gen_pi_ops,
	};


/*AON PI CCU Id*/
static char* aon_ccu[] = {KHUBAON_CCU_CLK_NAME_STR,NULL};

static struct pi_state aon_states[] =
		{
			PI_STATE(PI_STATE_ACTIVE,RUN_POLICY,0),
			PI_STATE(PI_STATE_RETENTION,RETN_POLICY,100),
			PI_STATE(PI_MGR_STATE_UNSUPPORTED,0,0),

		};


static struct pi aon_pi =
	{
		.name = "aon",
		.id = PI_MGR_PI_ID_HUB_AON,
		.ccu_id = aon_ccu,
		.state_allowed = PI_STATE_RETENTION,
		.pi_state = aon_states,
		.opp_active = 0,
		/*opp frequnecies ...need to revisit*/
		.opp =  {
					[PI_OPP_ECONOMY] = 4,
					[PI_OPP_NORMAL] = 4,
					[PI_OPP_TURBO] = 4,
				},
		.sw_event_id = SOFTWARE_0_EVENT,
		.pi_info =
				{
					.policy_reg_offset = PWRMGR_LCDTE_VI_HUB_POLICY_OFFSET,
					.ac_shift = PWRMGR_LCDTE_VI_HUB_POLICY_LCDTE_PI_HUB_AON_PM_AC_SHIFT,
					.atl_shift = PWRMGR_LCDTE_VI_HUB_POLICY_LCDTE_PI_HUB_AON_PM_ATL_SHIFT,
					.pm_policy_shift = PWRMGR_LCDTE_VI_HUB_POLICY_LCDTE_PI_HUB_AON_PM_POLICY_SHIFT,

					.fixed_vol_map_mask = PWRMGR_FIXED_VOLTAGE_MAP_VI_HUB_FIXED_VOLTAGE_MAP_MASK,
					.vi_to_vOx_map_mask = PWRMGR_VI_TO_VO0_MAP_VI_HUB_TO_VO0_MAP_MASK,
					.wakeup_overide_mask = PWRMGR_PI_DEFAULT_POWER_STATE_PI_HUB_AON_WAKEUP_OVERRIDE_MASK,
					.counter_reg_offset = PWRMGR_PI_HUB_AON_ON_COUNTER_OFFSET,
					.rtn_clmp_dis_mask = PWRMGR_PI_DEFAULT_POWER_STATE_PI_HUB_AON_RETENTION_CLAMP_DISABLE_MASK,
				},
		.ops = &gen_pi_ops,
	};

/*ARM subsystem PI Id*/
static char* sub_sys_ccu[] = {KPM_CCU_CLK_NAME_STR,KPS_CCU_CLK_NAME_STR,NULL};

static struct pi_state sub_sys_states[] =
		{
			PI_STATE(PI_STATE_ACTIVE,RUN_POLICY,0),
			PI_STATE(PI_STATE_RETENTION,RETN_POLICY,100),
			PI_STATE(PI_MGR_STATE_UNSUPPORTED,0,0),

		};


static struct pi sub_sys_pi =
	{
		.name = "sub_sys",
		.id = PI_MGR_PI_ID_ARM_SUB_SYSTEM,
		.ccu_id = sub_sys_ccu,
		.state_allowed = PI_STATE_RETENTION,
		.pi_state = sub_sys_states,
		.opp_active = 0,
		/*opp frequnecies ...need to revisit*/
		.opp =  {
					[PI_OPP_ECONOMY] = 2,
					[PI_OPP_NORMAL] = 2,
					[PI_OPP_TURBO] = 2,
				},
		.sw_event_id = SOFTWARE_0_EVENT,
		.pi_info =
				{
					.policy_reg_offset = PWRMGR_LCDTE_VI_ARM_SUBSYSTEM_POLICY_OFFSET,
					.ac_shift = PWRMGR_LCDTE_VI_ARM_SUBSYSTEM_POLICY_LCDTE_PI_ARM_SUBSYSTEM_PM_AC_SHIFT,
					.atl_shift = PWRMGR_LCDTE_VI_ARM_SUBSYSTEM_POLICY_LCDTE_PI_ARM_SUBSYSTEM_PM_ATL_SHIFT,
					.pm_policy_shift = PWRMGR_LCDTE_VI_ARM_SUBSYSTEM_POLICY_LCDTE_PI_ARM_SUBSYSTEM_PM_POLICY_SHIFT,

					.fixed_vol_map_mask = PWRMGR_FIXED_VOLTAGE_MAP_VI_ARM_SUBSYSTEM_FIXED_VOLTAGE_MAP_SHIFT,
					.vi_to_vOx_map_mask = PWRMGR_VI_TO_VO0_MAP_VI_ARM_SUBSYSTEM_TO_VO0_MAP_MASK,
					.wakeup_overide_mask = PWRMGR_PI_DEFAULT_POWER_STATE_PI_ARM_SUBSYSTEM_WAKEUP_OVERRIDE_MASK,
					.counter_reg_offset = PWRMGR_PI_ARM_SUBSYSTEM_ON_COUNTER_OFFSET,
					.rtn_clmp_dis_mask = PWRMGR_PI_DEFAULT_POWER_STATE_PI_ARM_SUBSYSTEM_RETENTION_CLAMP_DISABLE_MASK,
				},
		.ops = &gen_pi_ops,
	};

/*MODEM CCU -  ADDED for initializing EVENT table only*/
static struct pi_state modem_states[] =
		{
			PI_STATE(PI_MGR_STATE_UNSUPPORTED,0,0),

		};


static struct pi modem_pi =
	{
		.name = "modem",
		.id = PI_MGR_PI_ID_MODEM,
		.flags = PI_NO_DFS | PI_NO_QOS,
		.pi_state = modem_states,
		.pi_info =
				{
					.policy_reg_offset = PWRMGR_LCDTE_VI_ARM_CORE_POLICY_OFFSET,
					.ac_shift = PWRMGR_LCDTE_VI_ARM_CORE_POLICY_LCDTE_PI_MODEM_PM_AC_SHIFT,
					.atl_shift = PWRMGR_LCDTE_VI_ARM_CORE_POLICY_LCDTE_PI_MODEM_PM_ATL_SHIFT,
					.pm_policy_shift = PWRMGR_LCDTE_VI_ARM_CORE_POLICY_LCDTE_PI_MODEM_PM_POLICY_SHIFT,

					.fixed_vol_map_mask = PWRMGR_FIXED_VOLTAGE_MAP_VI_MODEM_FIXED_VOLTAGE_MAP_SHIFT,
					.vi_to_vOx_map_mask = PWRMGR_VI_TO_VO0_MAP_VI_MODEM_TO_VO0_MAP_MASK,
					.wakeup_overide_mask = PWRMGR_PI_DEFAULT_POWER_STATE_PI_MODEM_WAKEUP_OVERRIDE_MASK,
					.counter_reg_offset = PWRMGR_PI_MODEM_ON_COUNTER_OFFSET,
					.rtn_clmp_dis_mask = PWRMGR_PI_DEFAULT_POWER_STATE_PI_MODEM_RETENTION_CLAMP_DISABLE_MASK,
				},
		.ops = NULL,
	};



void __init rhea_pi_mgr_init()
{
	int i;
	struct pi* pi_list[] =
	{
		&arm_core_pi,
		&mm_pi,
		&hub_pi,
		&aon_pi,
		&sub_sys_pi,
		&modem_pi

	};
	pi_mgr_init();

	for(i = 0; i < ARRAY_SIZE(pi_list);i++)
	{
		pr_info("%s: PI:%s state[0] = %x %x\n",__func__,pi_list[i]->name,
			pi_list[i]->pi_state[1].id,pi_list[i]->pi_state[1].state_policy);
		pi_mgr_register(pi_list[i]);
	}

}
EXPORT_SYMBOL(rhea_pi_mgr_init);

