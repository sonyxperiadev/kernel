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
#include <mach/appf_types.h>
#include <mach/appf_internals.h>
#include <mach/appf_helpers.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <mach/sec_api.h>
#include <mach/capri_dormant.h>
#include <asm/proc-fns.h>
#include <asm/memory.h>
#include <linux/dma-mapping.h>
#include <linux/spinlock.h>

#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#include <mach/rdb/brcm_rdb_scu.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <mach/rdb/brcm_rdb_kproc_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_pl310.h>

#include <plat/pi_mgr.h>
#include <plat/pwr_mgr.h>
#include <mach/chipregHw_inline.h>

#include <asm/hardware/cache-l2x0.h>

/* Control variable to enable/disable dormant */
static u32 dormant_disable;
module_param_named(dormant_disable, dormant_disable,
		   int, S_IRUGO | S_IWUSR | S_IWGRP);

/* Variable disables dormant but goes through
 * dormant path.  i.e all code is executed as though
 * we are entering and exiting dormant but the
 * HW mask to be able to enter dormant is not set
 */
static u32 fake_dormant;
module_param_named(fake_dormant, fake_dormant,
		   int, S_IRUGO | S_IWUSR | S_IWGRP);

/* Control variable to turn off the power to L2 memory.  Default is set to
 * turn off the memory.  This involves cleaning/invalidating L2 memory
 * write a 0 to not turn off the l2 memory
 */
static int turn_off_l2_memory = 1;
module_param_named(turn_off_l2_memory, turn_off_l2_memory,
		   int, S_IRUGO | S_IWUSR | S_IWGRP);

/* Control variable to control periodic dump of counters */
static int dbg_msg_counters;
module_param_named(dbg_msg_counters, dbg_msg_counters,
		   int, S_IRUGO | S_IWUSR | S_IWGRP);

/* Control variable to control the peridicity of the dump of counters */
static int dbg_periodic_cnt = 500;
module_param_named(dbg_periodic_cnt, dbg_periodic_cnt,
		   int, S_IRUGO | S_IWUSR | S_IWGRP);

/* Variables to maintain counters */
static int cnt_attempts;
module_param_named(cnt_attempts, cnt_attempts,
		   int, S_IRUGO | S_IWUSR | S_IWGRP);

static int cnt_success;
module_param_named(cnt_success, cnt_success, int, S_IRUGO | S_IWUSR | S_IWGRP);

static int cnt_failure;
module_param_named(cnt_failure, cnt_failure, int, S_IRUGO | S_IWUSR | S_IWGRP);

/* Buffer to pass parameters to secure rom */
#define	SEC_BUFFER_ADDR				0x3404C400	/* SRAM */
#define	SEC_BUFFER_SIZE				0x00000400	/* 1kB */

#define MAX_SECURE_BUFFER_SIZE		0x280
#define NUM_API_PARAMETERS			0x3

#define SEC_EXIT_NORMAL 1
#define SSAPI_RET_FROM_INT_SERV	4

/* CORE-0 is the master core for boot-up */
#define MASTER_CORE					0

#define	SCU_DORMANT_L2_OFF_MODE		0x03030303
#define	SCU_DORMANT_MODE			0x03030202
#define	SCU_DORMANT_MODE_OFF		0x03030000

#define	SCU_DORMANT_L2_OFF_B		0x03
#define	SCU_DORMANT_MODE_B			0x02
#define	SCU_DORMANT_MODE_OFF_B		0x00

#define	USE_SCU_PWR_CTRL					0x100
#define	PWRCTRL_DORMANT_CORE_0				0x80
#define	PWRCTRL_DORMANT_L2_OFF_CORE_0		0xC0

#define	PWRCTRL_DORMANT_CORE_1				0x02000000
#define	PWRCTRL_DORMANT_L2_OFF_CORE_1		0x03000000

#define	UNLOCK_PROC_CLK				0xA5A501
#define FREQ_312_MHZ_ID				0x04040404

/* Define pointers per CPU to the data that is banked
 * for both the CPU's
 */
DEFINE_PER_CPU(u8[PMU_DATA_SIZE], pmu_data);
DEFINE_PER_CPU(u8[TIMER_DATA_SIZE], timer_data);
DEFINE_PER_CPU(u8[GLOBAL_TIMER_DATA_SIZE], global_timer_data);
DEFINE_PER_CPU(u8[VFP_DATA_SIZE], vfp_data);
DEFINE_PER_CPU(u8[GIC_INTERFACE_DATA_SIZE], gic_interface_data);
DEFINE_PER_CPU(u8[GIC_DIST_PRIVATE_DATA_SIZE], gic_dist_private_data);
DEFINE_PER_CPU(u8[BANKED_REGISTERS_SIZE], banked_registers);
DEFINE_PER_CPU(u8[CP15_DATA_SIZE], cp15_data);
DEFINE_PER_CPU(u8[OTHER_DATA_SIZE], other_data);
DEFINE_PER_CPU(u8[DEBUG_DATA_SIZE], debug_data);
DEFINE_PER_CPU(u8[CONTROL_DATA_SIZE], control_data);
DEFINE_PER_CPU(u8[MMU_DATA_SIZE], mmu_data);

/* Data for the entire cluster */
static DEFINE_SPINLOCK(dormant_entry_lock);

/* Counts the numner of cores that are in the process of
 * entering dormant
 */
u32 num_cores_in_dormant;

u8 gic_dist_shared_data[GIC_DIST_SHARED_DATA_SIZE];

/* un-cached memory for dormant stack */
u32 un_cached_stack_ptr;

#define PROC_CLK_REG_ADDR(reg_name)		((u32)(KONA_PROC_CLK_VA + \
		(KPROC_CLK_MGR_REG_##reg_name##_OFFSET)))

#define PROC_CLK_ITEM_DEFINE(reg_name)	{PROC_CLK_REG_ADDR(reg_name), 0}

/* PROC ccu registers that are lost. Be careful
 * when changing the order etc.
 */
static u32 proc_clk_regs[][2] = {
	PROC_CLK_ITEM_DEFINE(PL310_DIV),
	PROC_CLK_ITEM_DEFINE(ARM_SWITCH_DIV),
	PROC_CLK_ITEM_DEFINE(APB_DIV),
	PROC_CLK_ITEM_DEFINE(ARM_DIV),
	PROC_CLK_ITEM_DEFINE(PL310_TRIGGER),
	PROC_CLK_ITEM_DEFINE(ARM_SWITCH_TRIGGER),
	PROC_CLK_ITEM_DEFINE(APB_DIV_TRIGGER),
	PROC_CLK_ITEM_DEFINE(ARM_SEG_TRG_OVERRIDE),
	PROC_CLK_ITEM_DEFINE(POLICY_FREQ),
	PROC_CLK_ITEM_DEFINE(POLICY_CTL),
	PROC_CLK_ITEM_DEFINE(POLICY0_MASK),
	PROC_CLK_ITEM_DEFINE(POLICY1_MASK),
	PROC_CLK_ITEM_DEFINE(POLICY2_MASK),
	PROC_CLK_ITEM_DEFINE(POLICY3_MASK),
	PROC_CLK_ITEM_DEFINE(INTEN),
	PROC_CLK_ITEM_DEFINE(INTSTAT),
	PROC_CLK_ITEM_DEFINE(LVM_EN),
	PROC_CLK_ITEM_DEFINE(LVM0_3),
	PROC_CLK_ITEM_DEFINE(LVM4_7),
	PROC_CLK_ITEM_DEFINE(VLT0_3),
	PROC_CLK_ITEM_DEFINE(VLT4_7),
	PROC_CLK_ITEM_DEFINE(BUS_QUIESC),
	PROC_CLK_ITEM_DEFINE(CORE0_CLKGATE),
	PROC_CLK_ITEM_DEFINE(CORE1_CLKGATE),
	PROC_CLK_ITEM_DEFINE(ARM_SWITCH_CLKGATE),
	PROC_CLK_ITEM_DEFINE(ARM_PERIPH_CLKGATE),
	PROC_CLK_ITEM_DEFINE(APB0_CLKGATE),
	PROC_CLK_ITEM_DEFINE(PLLARMA),
	PROC_CLK_ITEM_DEFINE(PLLARMB),
	PROC_CLK_ITEM_DEFINE(PLLARMC),
	PROC_CLK_ITEM_DEFINE(PLLARMCTRL0),
	PROC_CLK_ITEM_DEFINE(PLLARMCTRL1),
	PROC_CLK_ITEM_DEFINE(PLLARMCTRL2),
	PROC_CLK_ITEM_DEFINE(PLLARMCTRL3),
	PROC_CLK_ITEM_DEFINE(PLLARMCTRL4),
	PROC_CLK_ITEM_DEFINE(PLLARMCTRL5),
	PROC_CLK_ITEM_DEFINE(PLLARM_OFFSET),
	PROC_CLK_ITEM_DEFINE(PLL_DEBUG),
	PROC_CLK_ITEM_DEFINE(ACTIVITY_MON1),
	PROC_CLK_ITEM_DEFINE(ACTIVITY_MON2),
	PROC_CLK_ITEM_DEFINE(CLKGATE_DBG),
	PROC_CLK_ITEM_DEFINE(APB_CLKGATE_DBG1),
	PROC_CLK_ITEM_DEFINE(CLKMON),
	PROC_CLK_ITEM_DEFINE(POLICY_DBG),
	/*PROC_CLK_ITEM_DEFINE(ARM_SYS_IDLE_DLY),*/
	PROC_CLK_ITEM_DEFINE(TGTMASK_DBG1)
};

#define PROC_CLK_ITEM_ADDR(index)	(proc_clk_regs[(index)][0])
#define PROC_CLK_ITEM_VALUE(index)	(proc_clk_regs[(index)][1])

#define LOG_BUFFER_SIZE (SEC_BUFFER_SIZE -\
		(MAX_SECURE_BUFFER_SIZE +\
			sizeof(u32)*NUM_API_PARAMETERS +\
			sizeof(u32)*1))

/* Structure of the parameters passed in the buffer to secure side */
struct secure_params_t {
	u32 core0_reset_address;
	u32 core1_reset_address;
	/* Remaining part of the buffer space */
	u8 buffer[MAX_SECURE_BUFFER_SIZE - 8];

	/* We use this address to pass in the parameters */
	u32 api_params[NUM_API_PARAMETERS];

	/* Remaining space un-used
	 * SEC_BUFFER_SIZE -
	 * MAX_SECURE_BUFFER_SIZE + sizeof(u32)*NUM_API_PARAMETERS)
	 */
	u32 log_index;
	u8 log_buffer[LOG_BUFFER_SIZE];
};

enum DORMANT_LOG_TYPE {
	DORMNAT_ENTRY_LOG = 1,
	DORMANT_EXIT_LOG,
	DORMANT_BEFORE_L2_FLUSH_LOG,
	DORMANT_AFTER_L2_FLUSH_LOG,
	DORMANT_EXIT_SUCCESS_LOG = 0x75,
	DORMANT_EXIT_FAILURE_LOG = 0x7F
};

static struct secure_params_t *secure_params;

/*********** Local Functions *************/

static inline void print_dbg_counters(void)
{
	if (dbg_msg_counters && cnt_attempts &&
	    (cnt_attempts % dbg_periodic_cnt) == 0) {
		pr_info("Dormant Stats: Try:%d Win:%d Loss:%d\n", cnt_attempts,
		       cnt_success, cnt_failure);
	}
}

static inline void log_dormant_event(enum DORMANT_LOG_TYPE log)
{
	u32 processor_id = (read_mpidr() & 0xff);
	if (secure_params->log_index == LOG_BUFFER_SIZE)
		secure_params->log_index = 0;
	secure_params->log_buffer[secure_params->log_index++] =
	    (log | processor_id << 7);
}

/*
 * Function to save proc_clk registers that are
 * lost upon dormant entry */
static void save_proc_clk_regs(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(proc_clk_regs); i++)
		PROC_CLK_ITEM_VALUE(i) = readl_relaxed(PROC_CLK_ITEM_ADDR(i));
}

/*
 * Function to return the number of CPUs
 */
static u32 num_cpus(void)
{
	return (readl(KONA_SCU_VA + SCU_CONFIG_OFFSET) &
		 SCU_CONFIG_NUM_CPUS_MASK) + 1;
}

/*
 * Function to identify weather l2 controller is off
 */
static u32 is_l2_disabled(void)
{
	u32 aux;
	aux = readl_relaxed(KONA_L2C_VA + L2X0_CTRL);
	return !(aux & 1);
}

/*
 * Function to restored saved  proc_clk registers that are
 * lost upon dormant entry
 */
static void restore_proc_clk_regs(void)
{
	int i;
	u32 val1, val2;

	/* Allow write access to the CCU registers */
	writel_relaxed(UNLOCK_PROC_CLK, PROC_CLK_REG_ADDR(WR_ACCESS));

#ifdef CONFIG_BCM_HWCAPRI_1605
	/* If the issue is not fixed, first step to 312 MHZ before restoring
	 * all the registers
	 */
	writel_relaxed(FREQ_312_MHZ_ID, PROC_CLK_REG_ADDR(POLICY_FREQ));

	/* Write the go bit to trigger the frequency change
	 */
	writel_relaxed(KPROC_CLK_MGR_REG_POLICY_CTL_GO_AC_MASK |
		       KPROC_CLK_MGR_REG_POLICY_CTL_GO_MASK,
		       PROC_CLK_REG_ADDR(POLICY_CTL));

	/* Wait until the new frequency takes effect */
	do {
		val1 = readl_relaxed(PROC_CLK_REG_ADDR(POLICY_CTL)) &
		    KPROC_CLK_MGR_REG_POLICY_CTL_GO_MASK;

		val2 = readl_relaxed(PROC_CLK_REG_ADDR(POLICY_CTL)) &
		    KPROC_CLK_MGR_REG_POLICY_CTL_GO_MASK;
	} while (val1 | val2);
#endif

	for (i = 0; i < ARRAY_SIZE(proc_clk_regs); i++) {
		/* Restore the saved data */
		writel_relaxed(PROC_CLK_ITEM_VALUE(i), PROC_CLK_ITEM_ADDR(i));

		if ((PROC_CLK_ITEM_ADDR(i) ==
		     PROC_CLK_REG_ADDR(ARM_SEG_TRG_OVERRIDE)) &&
		    (PROC_CLK_ITEM_VALUE(i) & 1)) {

			/* We just restored arm_seg_trigger override
			 * and the override was set before.  trigger
			 * to take effect
			 */
			do {
				val1 =
				    readl_relaxed(PROC_CLK_REG_ADDR
							(ARM_SEG_TRG)) &
				KPROC_CLK_MGR_REG_ARM_SEG_TRG_ARM_TRIGGER_MASK;

				val2 =
				    readl_relaxed(PROC_CLK_REG_ADDR
							(ARM_SEG_TRG)) &
				KPROC_CLK_MGR_REG_ARM_SEG_TRG_ARM_TRIGGER_MASK;
			} while (val1 | val2);
		}
	}

	/* Finished restoring all the PROC_CLOCK registers
	 * lock the state machine and write the go bit
	 */
	writel_relaxed(readl_relaxed(PROC_CLK_REG_ADDR(LVM_EN)) |
		       KPROC_CLK_MGR_REG_LVM_EN_POLICY_CONFIG_EN_MASK,
		       PROC_CLK_REG_ADDR(LVM_EN));

	while (readl_relaxed(PROC_CLK_REG_ADDR(LVM_EN)) &
	       KPROC_CLK_MGR_REG_LVM_EN_POLICY_CONFIG_EN_MASK)
		;

	/* Write the GO bit */
	writel_relaxed(KPROC_CLK_MGR_REG_POLICY_CTL_GO_AC_MASK |
		       KPROC_CLK_MGR_REG_POLICY_CTL_GO_MASK,
		       PROC_CLK_REG_ADDR(POLICY_CTL));
}

/*
 * Function to call secure API locally.  When an API that
 * can be used without a warning when we know that the
 * function is being called from core-0, we can use that
 */

static void local_secure_api(unsigned service_id,
			     unsigned arg0, unsigned arg1, unsigned arg2)
{
	/* Set Up Registers to pass data to Secure Monitor */
	register u32 r4 asm("r4");
	register u32 r5 asm("r5");
	register u32 r6 asm("r6");
	register u32 r12 asm("r12");
	unsigned *buffer_ptr = (unsigned *)secure_params->api_params;

	buffer_ptr[0] = arg0;
	buffer_ptr[1] = arg1;
	buffer_ptr[2] = arg2;

	do {
		r4 = service_id;
		r5 = 0x3;	/* Keep IRQ and FIQ off in SM */
		r6 = SEC_BUFFER_ADDR + MAX_SECURE_BUFFER_SIZE;

		/* Secure Monitor Call */
		asm volatile (
				__asmeq("%0", "ip")
				__asmeq("%1", "r4")
				__asmeq("%2", "r5")
				__asmeq("%3", "r6")
#ifdef REQUIRES_SEC
				".arch_extension sec\n"
#endif
				"smc	#0	@ switch to secure world\n"
				: "+r" (r12), "+r"(r4), "+r"(r5), "+r"(r6)
				:
				: "r0", "r1", "r2", "r3", "r7", "r8", "r14");
	} while (r12 != SEC_EXIT_NORMAL);
}

/******************* Public Functions ***************/

/*
 * Function to externally query if dormant is currently
 * enabled or not
 */
u32 is_dormant_enabled(void)
{
	return !dormant_disable;
}

/* Main dormant enter routine.  Must be called with
 * interrupts locked.  Will save/restore context of the CPU/CLUSTER
 * and returns back as a normal function call.
 */
void dormant_enter(enum CAPRI_DORMANT_SERVICE_TYPE service)
{
	unsigned long flgs;
	u32 boot_2nd_addr;
	u32 dormant_return;
	u32 reg_val;

	if (dormant_disable || fake_dormant) {
		/* Dis-allow entering dormant */
		reg_val = readl_relaxed(KONA_PWRMGR_VA +
					PWRMGR_PI_DEFAULT_POWER_STATE_OFFSET);

		reg_val |=
		    PWRMGR_PI_DEFAULT_POWER_STATE_ARM_CORE_DORMANT_DISABLE_MASK;

		writel_relaxed(reg_val, KONA_PWRMGR_VA +
			       PWRMGR_PI_DEFAULT_POWER_STATE_OFFSET);
	} else {
		/* Allow entering dormant */
		reg_val = readl_relaxed(KONA_PWRMGR_VA +
					PWRMGR_PI_DEFAULT_POWER_STATE_OFFSET);

		reg_val &=
		~PWRMGR_PI_DEFAULT_POWER_STATE_ARM_CORE_DORMANT_DISABLE_MASK;

		writel_relaxed(reg_val, KONA_PWRMGR_VA +
				PWRMGR_PI_DEFAULT_POWER_STATE_OFFSET);
	}

	if (dormant_disable) {

		/* This is retention case, so just execute WFI.
		 * and if this is for cluster dorn, write the rest
		 * of the needed register settings
		 */
		if (service == CAPRI_DORMANT_CLUSTER_DOWN) {

			writel_relaxed(SCU_DORMANT_MODE,
				       KONA_SCU_VA + SCU_POWER_STATUS_OFFSET);

		} else {
			/* This is the independent path for retention entry */
		}

		wfi();

		if (service == CAPRI_DORMANT_CLUSTER_DOWN) {

			writel_relaxed(SCU_DORMANT_MODE_OFF, KONA_SCU_VA
				       + SCU_POWER_STATUS_OFFSET);
		} else {
			/* This is the independent path for retention exit */
		}

		return;
	}

	/* Save CCU registers TBD */

	/* Save all the local data for this CPU for either service */

	save_performance_monitors((void *)__get_cpu_var(pmu_data));

	save_a9_timers((void *)__get_cpu_var(timer_data), (u32)KONA_SCU_VA);

	save_a9_global_timer((void *)__get_cpu_var(global_timer_data),
			     (u32)KONA_SCU_VA);

	save_vfp((void *)__get_cpu_var(vfp_data));

	save_gic_interface((void *)__get_cpu_var(gic_interface_data),
			   (u32)KONA_GICCPU_VA, false);

	save_gic_distributor_private((void *)
				     __get_cpu_var(gic_dist_private_data),
				     (u32)KONA_GICDIST_VA, false);

	save_banked_registers((void *)__get_cpu_var(banked_registers));

	save_cp15((void *)__get_cpu_var(cp15_data));

	save_a9_other((void *)__get_cpu_var(other_data), false);

	save_v7_debug((void *)__get_cpu_var(debug_data));

	save_control_registers((void *)__get_cpu_var(control_data), false);

	save_mmu((void *)__get_cpu_var(mmu_data));

	if ((service == CAPRI_DORMANT_CLUSTER_DOWN) && turn_off_l2_memory)
			local_secure_api(SSAPI_DISABLE_L2_CACHE, 0, 0, 0);

	spin_lock_irqsave(&dormant_entry_lock, flgs);
	num_cores_in_dormant++;
	log_dormant_event(DORMNAT_ENTRY_LOG);
	if (num_cores_in_dormant == num_cpus()) {
		/* This is the last core going down */
		cnt_attempts++;

		print_dbg_counters();

		save_gic_distributor_shared((void *)gic_dist_shared_data,
					    (u32)KONA_GICDIST_VA, false);

		/* save all proc registers except arm_sys_idle_dly */
		save_proc_clk_regs();

		if (fake_dormant)
			writel_relaxed(0, PROC_CLK_REG_ADDR(ARM_SYS_IDLE_DLY));
		else
			writel_relaxed(1, PROC_CLK_REG_ADDR(ARM_SYS_IDLE_DLY));

		/* Indicate 3 to the external power controller to turn off
		 * the L2 memory.  Fake dormant is like retention so just
		 * indicate a 2.
		 */
		if (is_l2_disabled() && !fake_dormant) {
			writel_relaxed(PWRCTRL_DORMANT_L2_OFF_CORE_0,
					   KONA_CHIPREG_VA +
					   CHIPREG_PERIPH_MISC_REG3_OFFSET);

		} else {
			writel_relaxed(PWRCTRL_DORMANT_CORE_0,
					   KONA_CHIPREG_VA +
					   CHIPREG_PERIPH_MISC_REG3_OFFSET);
		}

		/*
		 * Code to flush L2 and wait till done if needed
		 * log_dormant_event(DORMANT_BEFORE_L2_FLUSH_LOG);
		 * outer_flush_all();
		 * log_dormant_event(DORMANT_AFTER_L2_FLUSH_LOG);
		 * Wait till PL310 has no background stuff letf
		 * while (readl(KONA_L2C_VA + PL310_INV_PA_OFFSET ) &
		 * PL310_INV_PA_C_1_MASK);
		 * while (readl(KONA_L2C_VA + PL310_CLEAN_PA_OFFSET) &
		 * PL310_CLEAN_PA_C_2_MASK);
		 * while (readl(KONA_L2C_VA + PL310_CLEAN_INDEX_WAY_OFFSET) &
		 * PL310_CLEAN_INDEX_WAY_C_3_MASK);
		 * while (readl(KONA_L2C_VA + PL310_C_I_PA_OFFSET) &
		 * PL310_C_I_PA_C_4_MASK);
		 * while (readl(KONA_L2C_VA + PL310_C_I_INDEX_WAY_OFFSET) &
		 * PL310_C_I_INDEX_WAY_C_5_MASK);
		 */
	}
	spin_unlock_irqrestore(&dormant_entry_lock, flgs);

	dormant_return = dormant_enter_prepare(un_cached_stack_ptr + SZ_1K +
					       SZ_1K * smp_processor_id(),
					       PHYS_OFFSET - PAGE_OFFSET);

	/* We are here after either a failed dormant or a successful
	 * dormant
	 */

	/* Indicate that this core has the SCU in Normal mode */
	writeb_relaxed(SCU_DORMANT_MODE_OFF_B,
		       KONA_SCU_VA + SCU_POWER_STATUS_OFFSET +
		       smp_processor_id());

	if ((service == CAPRI_DORMANT_CLUSTER_DOWN) && turn_off_l2_memory)
			local_secure_api(SSAPI_ENABLE_L2_CACHE, 0, 0, 0);

	/* if we failed to enter dormant, restore
	 * only what might have changed when dormant fails
	 */
	if (!dormant_return) {
		/* restore only what we lost without entering
		 * dormant and return
		 */
		restore_control_registers((void *)__get_cpu_var(control_data),
					  false);

		restore_a9_timers((void *)__get_cpu_var(timer_data),
				  (u32)KONA_SCU_VA);

		restore_performance_monitors((void *)__get_cpu_var(pmu_data));
	}

	spin_lock_irqsave(&dormant_entry_lock, flgs);

	if (!dormant_return)
		log_dormant_event(DORMANT_EXIT_FAILURE_LOG);
	else
		log_dormant_event(DORMANT_EXIT_SUCCESS_LOG);

	if (num_cores_in_dormant == num_cpus()) {

		/* This is the first core trying to come out of dormant */

		/* Did we enter retention or dormant */
		if (!pwr_mgr_is_event_active(SOFTWARE_2_EVENT))
			cnt_success++;
		else
			cnt_failure++;

		writel_relaxed(UNLOCK_PROC_CLK, KONA_PROC_CLK_VA);

		/* Let ARM_SYS_IDLE_DLY be set to 0 for non dormant cases */
		writel_relaxed(0, PROC_CLK_REG_ADDR(ARM_SYS_IDLE_DLY));

		/* Use the SCU bits not the bypass bits for non dormant cases */
		writel_relaxed(USE_SCU_PWR_CTRL,
			       KONA_CHIPREG_VA +
			       CHIPREG_PERIPH_MISC_REG3_OFFSET);

	}

	if (num_cores_in_dormant)
		num_cores_in_dormant--;

	spin_unlock_irqrestore(&dormant_entry_lock, flgs);

	/* If dormant exit is successful, restore context */
	if (dormant_return) {
		if (smp_processor_id() == MASTER_CORE) {
			/*
			 * True dormant exit and we are core-0.  Let
			 * us first restore the proc registers
			 */
			restore_proc_clk_regs();
		}

		/* restore everything that is specific to this core */
		restore_mmu((void *)__get_cpu_var(mmu_data));

		restore_control_registers((void *)__get_cpu_var(control_data),
					  false);

		restore_v7_debug((void *)__get_cpu_var(debug_data));

		/*
		 * If we are the master core, and this is a true dormant exit,
		 * we should restore the common gic related registers
		 */
		if (smp_processor_id() == MASTER_CORE) {

			gic_distributor_set_enabled(false,
						    (u32)KONA_GICDIST_VA);

			restore_gic_distributor_shared((void *)
						       gic_dist_shared_data,
						       (u32)KONA_GICDIST_VA,
						       false);

			gic_distributor_set_enabled(true, (u32)KONA_GICDIST_VA);

		}

		/* continue restoring cpu specific content */
		restore_gic_distributor_private((void *)
						__get_cpu_var
						(gic_dist_private_data),
						(u32)KONA_GICDIST_VA, false);

		restore_gic_interface((void *)__get_cpu_var(gic_interface_data),
				      (u32)KONA_GICCPU_VA, false);

		restore_a9_other((void *)__get_cpu_var(other_data), false);

		restore_cp15((void *)__get_cpu_var(cp15_data));

		restore_banked_registers((void *)
					 __get_cpu_var(banked_registers));

		restore_vfp((void *)__get_cpu_var(vfp_data));

		restore_a9_timers((void *)__get_cpu_var(timer_data),
				  (u32)KONA_SCU_VA);

		restore_a9_global_timer((void *)
					__get_cpu_var(global_timer_data),
					(u32)KONA_SCU_VA);

		restore_performance_monitors((void *)__get_cpu_var(pmu_data));

		if (smp_processor_id() == MASTER_CORE) {
			/*
			 * Here we got out of idle do we let the core-0
			 * continue to run
			 */
			if (service == CAPRI_DORMANT_CORE_DOWN) {
				/* It was not a cluster down but still,
				 * we exited dormant. Let go of CORE-1
				 */
				boot_2nd_addr =
				    readl_relaxed(KONA_CHIPREG_VA +
						  CHIPREG_BOOT_2ND_ADDR_OFFSET);
				boot_2nd_addr |= 1;
				writel_relaxed(boot_2nd_addr,
					       KONA_CHIPREG_VA +
					       CHIPREG_BOOT_2ND_ADDR_OFFSET);
				dsb_sev();
				/* Wait for core-0 to acknowledge the wake-up */
				do {
					boot_2nd_addr =
					    readl_relaxed(KONA_CHIPREG_VA +
						CHIPREG_BOOT_2ND_ADDR_OFFSET);
				} while (boot_2nd_addr & 1);
			} /* core down */
		} /* Master core */
	} /* Success dormant return */
}

/*
 * Routine called from assembly, after saving the CPU context
 * this is where WFI would be executed to take down the power
 */
void dormant_enter_continue(void)
{
	u32 processor_id;
	processor_id = (read_mpidr() & 0xff);

	disable_clean_inv_dcache_v7_l1();
	write_actlr(read_actlr() & ~(0x40));

	/* Let us always indicate the dormant mode to the SCU
	 * the external power-controller sees what is in the bypass
	 * register any way
	 */
	writeb_relaxed(SCU_DORMANT_MODE_B,
		       KONA_SCU_VA + SCU_POWER_STATUS_OFFSET + processor_id);


	if (processor_id == MASTER_CORE) {

		secure_params->core0_reset_address = virt_to_phys(cpu_resume);

		secure_params->core1_reset_address = virt_to_phys(cpu_resume);


		/* Check if L2 controller is off  or if this is a fake dormant
		 * if it is, do not bother saving/restoring L2 controlelr
		 * in the secure side.  For fake dormant, we do not want
		 * to save and restore the L2 controller since it would not
		 * be turned off.
		 */

		if (is_l2_disabled() || fake_dormant) {
			local_secure_api(SSAPI_DORMANT_ENTRY_SERV,
					 (u32)SEC_BUFFER_ADDR,
					 (u32)SEC_BUFFER_ADDR +
					 MAX_SECURE_BUFFER_SIZE, 3);
		} else {
			local_secure_api(SSAPI_DORMANT_ENTRY_SERV,
					 (u32)SEC_BUFFER_ADDR,
					 (u32)SEC_BUFFER_ADDR +
					 MAX_SECURE_BUFFER_SIZE, 2);
		}

	} else {
		/* Write the address where we want core-1 to boot */
		writel_relaxed(virt_to_phys(cpu_resume),
			       KONA_CHIPREG_VA + CHIPREG_BOOT_2ND_ADDR_OFFSET);

		/* Directly execute WFI for non core-0 cores */
		wfi();

	}
}

/* Initialization function for dormant module */
int __init capri_dormant_init(void)
{
	struct resource *res;

	u32 core1_pwrctrl_reg;
	void *vptr = NULL;

	dma_addr_t drmt_buf_phy;

	vptr = dma_alloc_coherent(NULL, SZ_1K * num_cpus(),
				  &drmt_buf_phy, GFP_ATOMIC);

	if (vptr == NULL) {
		pr_info("%s: dormant dma buffer alloc failed\n", __func__);
		return -ENOMEM;
	}

	un_cached_stack_ptr = (u32)vptr;

	res = request_mem_region(SEC_BUFFER_ADDR, SEC_BUFFER_SIZE,
				 "secure_params");
	BUG_ON(!res);

	secure_params =
	    (struct secure_params_t *)ioremap_nocache(SEC_BUFFER_ADDR,
						      SEC_BUFFER_SIZE);

	BUG_ON(!secure_params);

	secure_params->log_index = 0;

	writel_relaxed(UNLOCK_PROC_CLK, KONA_PROC_CLK_VA);

	if (chipregHw_getChipIdRev() > 0xA1) {
		/* Indicate that core-1 is always willing to enter dormnt
		 * so the final decision is actually done by core-0's
		 * pwrctrl value can change each core writing as needed
		 * in independent path.
		 */
		core1_pwrctrl_reg =
			readl_relaxed(KONA_CHIPREG_VA +
			CHIPREG_PERIPH_MISC_REG2_OFFSET);

		core1_pwrctrl_reg |= PWRCTRL_DORMANT_L2_OFF_CORE_1;

		writel_relaxed(core1_pwrctrl_reg, KONA_CHIPREG_VA +
			       CHIPREG_PERIPH_MISC_REG2_OFFSET);
	}

	return 0;
}

module_init(capri_dormant_init);
