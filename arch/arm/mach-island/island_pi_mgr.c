/****************************************************************************
*									      
* Copyright 2010 --2011 Broadcom Corporation.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*****************************************************************************/

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
#define RETN_POLICY PM_POLICY_1
#define SHTDWN_POLICY PM_POLICY_0

#define PI_STATE(state_id,policy,latency) \
		{.id = state_id,.state_policy = policy,.hw_wakeup_latency = latency,}


char* armc_core_ccu[] = {KPROC_CCU_CLK_NAME_STR};
struct pi_opp arm_opp = {
							.opp =  {
//										[PI_OPP_ECONOMY] = 3,
										[PI_OPP_NORMAL] = 6,
										[PI_OPP_TURBO] = 7,
									},
						};


static struct pi_state arm_core_states[] =
		{
			PI_STATE(ARM_CORE_STATE_ACTIVE,RUN_POLICY,0),
			PI_STATE(ARM_CORE_STATE_SUSPEND,RUN_POLICY,0),
			PI_STATE(ARM_CORE_STATE_RETENTION,RETN_POLICY,100),
			PI_STATE(ARM_CORE_STATE_DORMANT,RETN_POLICY,10000),

		};

static struct pi arm_core_pi =
	{
		.name = "arm_core",
		.id = PI_MGR_PI_ID_ARM_CORE,
		.flags = PI_ENABLE_ON_INIT|PI_ARM_CORE|UPDATE_PM_QOS,
		.ccu_id = armc_core_ccu,
		.num_ccu_id = ARRAY_SIZE(armc_core_ccu),
		.state_allowed = ARM_CORE_STATE_DORMANT,
		.pi_state = arm_core_states,
		.num_states = ARRAY_SIZE(arm_core_states),
		.opp_active = 1,
		.pi_opp =  &arm_opp,
		.num_opp = 3,
		.qos_sw_event_id = SOFTWARE_0_EVENT,
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
#if 0
/*MM PI CCU Id*/
static char* mm_ccu[] = {MM_CCU_CLK_NAME_STR};
struct pi_opp mm_opp = {
							.opp =  {
										[PI_OPP_ECONOMY] = 1,
										[PI_OPP_NORMAL] = 4,
										[PI_OPP_TURBO] = 5,
									},
						};

static struct pi_state mm_states[] =
		{
			PI_STATE(PI_STATE_ACTIVE,RUN_POLICY,0),
			PI_STATE(PI_STATE_RETENTION,RETN_POLICY,100)
			//PI_STATE(PI_STATE_SHUTDOWN,SHTDWN_POLICY,100)

		};


static struct pi mm_pi =
	{
		.name = "mm",
		.id = PI_MGR_PI_ID_MM,
		//.flags = PI_DISABLE_ON_INIT,
		.ccu_id = mm_ccu,
		.num_ccu_id = ARRAY_SIZE(mm_ccu),
		.state_allowed = PI_STATE_RETENTION,
		.pi_state = mm_states,
		.num_states = ARRAY_SIZE(mm_states),
		.opp_active = 0,
		.pi_opp =  &mm_opp,
		.num_opp = 3,
		.qos_sw_event_id = SOFTWARE_0_EVENT,
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
#endif
/*HUB PI CCU Id*/
static char* hub_ccu[] = {KHUB_CCU_CLK_NAME_STR};
struct pi_opp hub_opp = {
							.opp =  {
										[PI_OPP_ECONOMY] = 2, /* 0 */
										[PI_OPP_NORMAL] = 2,
									},
						};


static struct pi_state hub_states[] =
		{
			PI_STATE(PI_STATE_ACTIVE,RUN_POLICY,0),
			PI_STATE(PI_STATE_RETENTION,RETN_POLICY,100),

		};


static struct pi hub_pi =
	{
		.name = "hub",
		.id = PI_MGR_PI_ID_HUB_SWITCHABLE,
		.ccu_id = hub_ccu,
		.num_ccu_id = ARRAY_SIZE(hub_ccu),
		.state_allowed = PI_STATE_RETENTION,
		.pi_state = hub_states,
		.num_states = ARRAY_SIZE(hub_states),
		.opp_active = 0,
		/*opp frequnecies ...need to revisit*/
		.pi_opp =  &hub_opp,
		.num_opp = 2,
		.qos_sw_event_id = SOFTWARE_0_EVENT,
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
static char* aon_ccu[] = {KHUBAON_CCU_CLK_NAME_STR};
struct pi_opp aon_opp = {
							.opp =  {
										[PI_OPP_ECONOMY] = 2, 
										[PI_OPP_NORMAL] = 3,
									},
						};

static struct pi_state aon_states[] =
		{
			PI_STATE(PI_STATE_ACTIVE,RUN_POLICY,0),
			PI_STATE(PI_STATE_RETENTION,RETN_POLICY,100),
		};


static struct pi aon_pi =
	{
		.name = "aon",
		.id = PI_MGR_PI_ID_HUB_AON,
		//.flags = PI_ENABLE_ON_INIT,
		.ccu_id = aon_ccu,
		.num_ccu_id = ARRAY_SIZE(aon_ccu),
		.state_allowed = PI_STATE_RETENTION,
		.pi_state = aon_states,
		.num_states = ARRAY_SIZE(aon_states),
		.opp_active = 0,
		/*opp frequnecies ...need to revisit*/
		.pi_opp =  &aon_opp,
		.num_opp = 2,
		.qos_sw_event_id = SOFTWARE_0_EVENT,
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
static char* sub_sys_ccu[] = {KPM_CCU_CLK_NAME_STR,KPS_CCU_CLK_NAME_STR};
struct pi_opp sub_sys_opp[2] = 	{
									[0] = { /*KPM*/
									.opp =	{
												[PI_OPP_ECONOMY] = 2,
												[PI_OPP_NORMAL] = 3,
											},
										  },
									[1] = { /*KPS*/
									.opp =	{
												[PI_OPP_ECONOMY] = 1, /* 0 */
												[PI_OPP_NORMAL] = 3,
											},
										  },

								};

static struct pi_state sub_sys_states[] =
		{
			PI_STATE(PI_STATE_ACTIVE,RUN_POLICY,0),
			PI_STATE(PI_STATE_RETENTION,RETN_POLICY,100),

		};


static struct pi sub_sys_pi =
	{
		.name = "sub_sys",
		.id = PI_MGR_PI_ID_ARM_SUB_SYSTEM,
		.ccu_id = sub_sys_ccu,
		.num_ccu_id = ARRAY_SIZE(sub_sys_ccu),
		.state_allowed = PI_STATE_RETENTION,
		.pi_state = sub_sys_states,
		.num_states = ARRAY_SIZE(sub_sys_states),
		.opp_active = 0,
		/*opp frequnecies ...need to revisit*/
		.pi_opp =  sub_sys_opp,
		.num_opp = 2,
		.qos_sw_event_id = SOFTWARE_0_EVENT,
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
			PI_STATE(PI_STATE_ACTIVE,RUN_POLICY,0),

		};


static struct pi modem_pi =
	{
		.name = "modem",
		.id = PI_MGR_PI_ID_MODEM,
		.flags = PI_NO_DFS | PI_NO_QOS,
		.pi_state = modem_states,
		.num_states = ARRAY_SIZE(modem_states),
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

struct pi* pi_list[] = 	{
		&arm_core_pi,
		/*&mm_pi,*/
		&hub_pi,
		&aon_pi,
		&sub_sys_pi,
		&modem_pi

};

void __init island_pi_mgr_init()
{
	int i;
	pi_mgr_init();

	for(i = 0; i < ARRAY_SIZE(pi_list);i++)
	{
		pr_info("%s: PI:%s state[0] = %x %x\n",__func__,pi_list[i]->name,
			pi_list[i]->pi_state[1].id,pi_list[i]->pi_state[1].state_policy);
		pi_mgr_register(pi_list[i]);
	}

}
EXPORT_SYMBOL(island_pi_mgr_init);
#ifdef CONFIG_DEBUG_FS

int __init pi_mgr_late_init(void)
{
    int i;
    pi_debug_init();
    for(i=0;i<ARRAY_SIZE(pi_list);i++)
    {
		pi_debug_add_pi(pi_list[i]);
    }
    return 0;
}

late_initcall(pi_mgr_late_init);

#endif /* CONFIG_DEBUG_FS */
