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
#include <linux/delay.h>
#include <mach/sec_api.h>
#include <mach/dormant.h>
#include <asm/proc-fns.h>
#include <asm/memory.h>
#include <linux/dma-mapping.h>
#include <linux/spinlock.h>
#include <asm/suspend.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#include <mach/rdb/brcm_rdb_gic.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <mach/rdb/brcm_rdb_kproc_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_cdc.h>
#include <mach/rdb/brcm_rdb_cstf.h>
#include <mach/rdb/brcm_rdb_swstm.h>
#include <plat/pi_mgr.h>
#include <plat/pwr_mgr.h>
#include <mach/kona_timer.h>
#include <mach/memory.h>
#include <asm/hardware/cache-l2x0.h>
#include <asm/cacheflush.h>
#include <mach/pm.h>
#include <mach/sram_config.h>
#include <plat/clock.h>
#ifdef CONFIG_BRCM_CDC
#include <plat/cdc.h>
#else
#error "CDC not enabled !!"
#endif
#include <mach/rdb/brcm_rdb_csr.h>
#include <plat/kona_memc.h>

#include "pm_params.h"

/* DM log masks */
enum {

	LOG_TEST_INFO	= 1,
	LOG_PENDING_INTR = 1 << 1,
};

#define dm_dbg(id, format...) \
	do {		\
		if (dbg_log & (id)) \
			pr_info(format); \
	} while (0)



/* variable to store proc_ccu pointer. This is
 * used to enable/disable access to proc_ccu using
 * clock module API.
 */
static struct ccu_clk *proc_ccu;


/* Control variable to turn off the power to L2 memory.  Default is set to
 * turn off the memory.  This involves cleaning/invalidating L2 memory
 * write a 0 to not turn off the l2 memory
 */
/* Buffer to pass parameters to secure rom */
#define	SEC_BUFFER_ADDR			SRAM_SHARED_BUF_BASE	/* SRAM */
#define	SEC_BUFFER_SIZE			SRAM_SHARED_BUF_SIZE	/* 1kB */

#define MAX_SECURE_BUFFER_SIZE		0x280
#define NUM_API_PARAMETERS		0x3

#define SEC_EXIT_NORMAL			1
#define SSAPI_RET_FROM_INT_SERV		4

/* Define pointers per CPU to the data that is banked
 * for all the CPU's
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

u8 gic_dist_shared_data[GIC_DIST_SHARED_DATA_SIZE];

static DEFINE_PER_CPU(u32, cdm_success);
static DEFINE_PER_CPU(u32, cdm_failure_nrfd);
static DEFINE_PER_CPU(u32, cdm_failure_cene);
static DEFINE_PER_CPU(u32, cdm_failure_misc);
static DEFINE_PER_CPU(u32, cdm_attempts);

static u8 svc_req[DRMT_SVC_MAX];

static u32 fdm_success;
static u32 fdm_short_success;
static u32 fdm_attempt;
static u32 l2_off_en =  1;
static u32 l2_off_cnt;
static u32 fdm_en = 1; /* Enable full dormant */
static u32 dbg_log;
static int wr_enabled;
static int pllarma_inx = -1;

/*
 * If cpuidle framework has requested for 'full dormant', but the system
 * might have not entered full dormant. i.e. system might have attempted for
 * full dormant, but did not happen. So, we can't rely on the command
 * 'cat /sys/devices/system/cpu/cpuN/cpuidle/stateX/time' to get to know the
 * amount of time spend in 'full dormant'. Thereby calculating full dormant time
 * time and storing in fdm_time variable. This is mainly for debug purpose.
 */
static u32 fdm_time;
static u32 fdm_time_en;

/* Data for the entire cluster */
static DEFINE_SPINLOCK(drmt_lock);

#ifdef CONFIG_MEMC_FORCE_156M_IN_SUSPEND
static struct kona_memc_node memc_dfs_node;
#endif /*CONFIG_MEMC_FORCE_156M_IN_SUSPEND*/

/* un-cached memory for dormant stack */
u32 un_cached_stack_ptr;
dma_addr_t drmt_buf_phy;

#define PROC_CLK_REG_ADDR(reg_name)		((u32)(KONA_PROC_CLK_VA + \
		(KPROC_CLK_MGR_REG_##reg_name##_OFFSET)))

#define PROC_CLK_ITEM_DEFINE(reg_name)	{PROC_CLK_REG_ADDR(reg_name), 0}

#define PROC_CLK_ITEM_ADDR(index)	(proc_clk_regs[(index)][0])
#define PROC_CLK_ITEM_VALUE(index)	(proc_clk_regs[(index)][1])

#define LOG_BUFFER_SIZE (SEC_BUFFER_SIZE -\
		(MAX_SECURE_BUFFER_SIZE +\
			sizeof(u32)*NUM_API_PARAMETERS +\
			sizeof(u32)*1))

#define ADDNL_REG_DEFINE(base, offset)	{(base) + (offset), 0}
#define ADDNL_REG_ADDR(inx)	(addnl_regs[(inx)][0])
#define ADDNL_REG_VAL(inx)	(addnl_regs[(inx)][1])

/* PROC ccu registers that are lost. Be careful
 * when changing the order etc.
 */
static u32 proc_clk_regs[][2] = {
	PROC_CLK_ITEM_DEFINE(AXI_DIV_FID7),
	PROC_CLK_ITEM_DEFINE(AXI_TRIGGER_FID7),
	PROC_CLK_ITEM_DEFINE(CCI_DIV_FID7),
	PROC_CLK_ITEM_DEFINE(CCI_DIV_TRIGGER_FID7),
	PROC_CLK_ITEM_DEFINE(AXI_DIV_FID6),
	PROC_CLK_ITEM_DEFINE(AXI_TRIGGER_FID6),
	PROC_CLK_ITEM_DEFINE(CCI_DIV_FID6),
	PROC_CLK_ITEM_DEFINE(CCI_DIV_TRIGGER_FID6),
	PROC_CLK_ITEM_DEFINE(SS_STRETCH),
	PROC_CLK_ITEM_DEFINE(AXI_DIV_FID5),
	PROC_CLK_ITEM_DEFINE(AXI_TRIGGER_FID5),
	PROC_CLK_ITEM_DEFINE(CCI_DIV_FID5),
	PROC_CLK_ITEM_DEFINE(CCI_DIV_TRIGGER_FID5),
	PROC_CLK_ITEM_DEFINE(POLICY0_MASK),
	PROC_CLK_ITEM_DEFINE(POLICY1_MASK),
	PROC_CLK_ITEM_DEFINE(POLICY2_MASK),
	PROC_CLK_ITEM_DEFINE(POLICY3_MASK),
	PROC_CLK_ITEM_DEFINE(INTEN),
	PROC_CLK_ITEM_DEFINE(INTSTAT),
	PROC_CLK_ITEM_DEFINE(LVM0_3),
	PROC_CLK_ITEM_DEFINE(LVM4_7),
	PROC_CLK_ITEM_DEFINE(VLT0_3),
	PROC_CLK_ITEM_DEFINE(VLT4_7),
	PROC_CLK_ITEM_DEFINE(AXI_CLKGATE),
	PROC_CLK_ITEM_DEFINE(CCI_CLKGATE),
	PROC_CLK_ITEM_DEFINE(ALL_CLK_IDLE),
	PROC_CLK_ITEM_DEFINE(PLL_DEBUG),
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
	PROC_CLK_ITEM_DEFINE(ACTIVITY_MON1),
	PROC_CLK_ITEM_DEFINE(ACTIVITY_MON2),
	PROC_CLK_ITEM_DEFINE(CLKGATE_DBG),
	PROC_CLK_ITEM_DEFINE(CLKMON),
	PROC_CLK_ITEM_DEFINE(POLICY_DBG),
	PROC_CLK_ITEM_DEFINE(POLICY_FREQ),
	PROC_CLK_ITEM_DEFINE(TGTMASK_DBG1)
};

/*List of additional registers that needs to
be saved/restored during A9 dormant*/
static u32 addnl_regs[][2] = {
	ADDNL_REG_DEFINE(KONA_FUNNEL_VA, CSTF_FUNNEL_CONTROL_OFFSET),
	ADDNL_REG_DEFINE(KONA_CDC_VA, CDC_CONFIG_OFFSET),
	ADDNL_REG_DEFINE(KONA_CDC_VA, CDC_A7_DEBUG_BUS_SELECT_OFFSET)

};


/* Structure of the parameters passed in the buffer to secure side */
struct secure_params_t {
	u32 core0_reset_address;
	u32 core1_reset_address;
	/* Pointer to DRAM address for secure side logging */
	u32 dram_log_buffer;

	/* Remaining part of the buffer space */
	u8 buffer[MAX_SECURE_BUFFER_SIZE - 12];

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
static int dormant_enter_continue(unsigned long svc);


static void set_svc_req(u32 svc)
{
	unsigned long flgs;
	spin_lock_irqsave(&drmt_lock, flgs);
	BUG_ON(svc >= DRMT_SVC_MAX ||
	svc_req[svc] > CONFIG_NR_CPUS);
	svc_req[svc]++;
	spin_unlock_irqrestore(&drmt_lock, flgs);
}

static void clr_svc_req(u32 svc)
{
	unsigned long flgs;
	spin_lock_irqsave(&drmt_lock, flgs);
	BUG_ON(svc >= DRMT_SVC_MAX ||
		svc_req[svc] == 0);
	svc_req[svc]--;
	spin_unlock_irqrestore(&drmt_lock, flgs);
}

static u32 get_svc(void)
{
	unsigned long flgs;
	int i;
	spin_lock_irqsave(&drmt_lock, flgs);
	for (i = 0; i < DRMT_SVC_MAX; i++) {
		if (svc_req[i])
			break;
	}
	spin_unlock_irqrestore(&drmt_lock, flgs);
	return i;
}


/*  PWRCTL1_bypass & PWRCTL0_bypass in Periph Spare Control2
 * registers holds CPU power mode. Boot ROM reads this register
 * instead of SCU Power Status register to differentiate between POR and
 * dormant reset. Linux needs to set this register to DORMANT_MODE before
 * dormant entry.
 *
 * In the dormant entry path, if an event or interrupt becomes pending
 * soon after the power manager starts the dormant state machine, the SCU
 * power status bits gets changed from dormant to normal mode. This also
 * gets latched in the power manager. But the system anyway enters dormant
 * mode and on wakeup, boot ROM incorrectly senses POR instead of dormant
 * wakeup. The new bits listed above is meant to overcome this problem.
 */
static void set_spare_power_status(unsigned int mode)
{
	unsigned int val;
	mode = mode & 0x3;

	val = readl(KONA_CHIPREG_VA + CHIPREG_PERIPH_SPARE_CONTROL2_OFFSET);
	val &= ~(3 << CHIPREG_PERIPH_SPARE_CONTROL2_PWRCTL0_BYPASS_SHIFT);
	val |= mode << CHIPREG_PERIPH_SPARE_CONTROL2_PWRCTL0_BYPASS_SHIFT;
	writel_relaxed(val, KONA_CHIPREG_VA +
		       CHIPREG_PERIPH_SPARE_CONTROL2_OFFSET);
}
/*
 * Function to save additional registers that are
 * lost upon dormant entry */
static void save_addnl_regs(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(addnl_regs); i++)
		ADDNL_REG_VAL(i) = readl_relaxed(ADDNL_REG_ADDR(i));
}
/*
 * Function to restore additional registers that are
 * lost upon dormant entry */
static void restore_addnl_regs(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(addnl_regs); i++)
		writel_relaxed(ADDNL_REG_VAL(i), ADDNL_REG_ADDR(i));
}


/*
 * Function to save proc_clk registers that are
 * lost upon dormant entry */
static void save_proc_clk_regs(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(proc_clk_regs); i++)
		PROC_CLK_ITEM_VALUE(i) = readl_relaxed(PROC_CLK_ITEM_ADDR(i));

	wr_enabled = readl_relaxed(PROC_CLK_REG_ADDR(WR_ACCESS)) &
				KPROC_CLK_MGR_REG_WR_ACCESS_CLKMGR_ACC_MASK;
}

static void clear_wakeup_interrupts(void)
{
	/* clear interrupts for COMMON_INT_TO_AC_EVENT */
	writel_relaxed(0xFFFFFFFF,
			KONA_CHIPREG_VA + CHIPREG_ENABLE_CLR0_OFFSET);
	writel_relaxed(0xFFFFFFFF,
			KONA_CHIPREG_VA + CHIPREG_ENABLE_CLR1_OFFSET);
	writel_relaxed(0xFFFFFFFF,
			KONA_CHIPREG_VA + CHIPREG_ENABLE_CLR2_OFFSET);
	writel_relaxed(0xFFFFFFFF,
			KONA_CHIPREG_VA + CHIPREG_ENABLE_CLR3_OFFSET);
	writel_relaxed(0xFFFFFFFF,
			KONA_CHIPREG_VA + CHIPREG_ENABLE_CLR4_OFFSET);
	writel_relaxed(0xFFFFFFFF,
			KONA_CHIPREG_VA + CHIPREG_ENABLE_CLR5_OFFSET);
	writel_relaxed(0xFFFFFFFF,
			KONA_CHIPREG_VA + CHIPREG_ENABLE_CLR6_OFFSET);

}

static void config_wakeup_interrupts(void)
{
	#define NUM_EN_REG 7
	int cpu;
	u32 enable_set[NUM_EN_REG];
	int i;
	u32 gic_mem_base;

	memset(enable_set, 0, sizeof(enable_set));
		/*Map all enabled interrupts to COMMON_INT_TO_AC_EVENT*/
	for_each_present_cpu(cpu) {
		gic_mem_base = KONA_GICDIST_MEM0_VA + cpu*0x10000;
		for (i = 0; i < NUM_EN_REG; i++)
			enable_set[i] |= readl(gic_mem_base +
				(GIC_GICD_ISENABLERN_1_OFFSET + i*4));
	}
	for (i = 0; i < NUM_EN_REG; i++)
		writel_relaxed(enable_set[i],
			KONA_CHIPREG_VA + (CHIPREG_ENABLE_SET0_OFFSET + i*4));
}


/*
 * Function to restored saved  proc_clk registers that are
 * lost upon dormant entry
 */
static void restore_proc_clk_regs(void)
{
	int i;
	u32 ins = 10000;

	/* Allow write access to the CCU registers */
	writel_relaxed(0xA5A501, PROC_CLK_REG_ADDR(WR_ACCESS));

	/*Stop policy engine*/
	writel_relaxed(readl_relaxed(PROC_CLK_REG_ADDR(LVM_EN)) |
		       KPROC_CLK_MGR_REG_LVM_EN_POLICY_CONFIG_EN_MASK,
		       PROC_CLK_REG_ADDR(LVM_EN));
	do {
		ins--;
		udelay(1);
	} while ((readl_relaxed(PROC_CLK_REG_ADDR(LVM_EN)) &
	       KPROC_CLK_MGR_REG_LVM_EN_POLICY_CONFIG_EN_MASK) && ins);
	BUG_ON(!ins);

	for (i = 0; i < ARRAY_SIZE(proc_clk_regs); i++)
		writel_relaxed(PROC_CLK_ITEM_VALUE(i), PROC_CLK_ITEM_ADDR(i));
	/* Write the GO bit */
	writel_relaxed(KPROC_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK |
		       KPROC_CLK_MGR_REG_POLICY_CTL_GO_MASK,
		       PROC_CLK_REG_ADDR(POLICY_CTL));
	ins = 10000;
	do {
		ins--;
		udelay(1);
	} while ((readl_relaxed(PROC_CLK_REG_ADDR(POLICY_CTL)) &
	       KPROC_CLK_MGR_REG_POLICY_CTL_GO_MASK) && ins);
	BUG_ON(!ins);
	/* Lock CCU registers */
	if (!wr_enabled)
		writel_relaxed(0xA5A500, PROC_CLK_REG_ADDR(WR_ACCESS));
}

/*
 * Function to call secure API locally.  When an API that
 * can be used without a warning when we know that the
 * function is being called from core-0, we can use that
 */

static void local_secure_api(unsigned service_id,
			     unsigned arg0, unsigned arg1, unsigned arg2)
{


#ifdef CONFIG_MOBICORE_DRIVER
	mobicore_smc(service_id, arg0, arg1, arg2);
#else
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
#ifdef SMC_INSTR
				".arch_extension sec\n"
#endif
				"smc	#0	@ switch to secure world\n"
				: "+r" (r12), "+r"(r4), "+r"(r5), "+r"(r6)
				:
				: "r0", "r1", "r2", "r3", "r7", "r8", "r14");
	} while (r12 != SEC_EXIT_NORMAL);
#endif

}

/* Save all the local data for this cpu */
static void save_arm_context(void)
{
	save_performance_monitors((void *)__get_cpu_var(pmu_data));

	save_generic_timer((void *)__get_cpu_var(timer_data));

	save_vfp((void *)__get_cpu_var(vfp_data));

	save_gic_interface((void *)__get_cpu_var(gic_interface_data),
			   (u32)KONA_GICCPU_VA, false);

	save_gic_distributor_private((void *)
				     __get_cpu_var(gic_dist_private_data),
				     (u32)KONA_GICDIST_VA, false);

	save_banked_registers((void *)__get_cpu_var(banked_registers));

	save_cp15((void *)__get_cpu_var(cp15_data));

	save_v7_debug((void *)__get_cpu_var(debug_data));

	save_control_registers((void *)__get_cpu_var(control_data), false);

	save_mmu((void *)__get_cpu_var(mmu_data));
}

/* Restore everything that is specific to this cpu */
static void restore_arm_context(bool restore_gic)
{
	restore_mmu((void *)__get_cpu_var(mmu_data));

	restore_control_registers((void *)__get_cpu_var(control_data),
					  false);
	restore_v7_debug((void *)__get_cpu_var(debug_data));

	/*For Java, GIC gets powered down only during
	cluster dormant.
	*/
	if (restore_gic) {
		/* continue restoring cpu specific content */
		restore_gic_distributor_private((void *)
						__get_cpu_var
						(gic_dist_private_data),
						(u32)KONA_GICDIST_VA, false);

		restore_gic_interface((void *)__get_cpu_var(gic_interface_data),
					  (u32)KONA_GICCPU_VA, false);
	}
	restore_cp15((void *)__get_cpu_var(cp15_data));

	restore_banked_registers((void *)
				 __get_cpu_var(banked_registers));

	restore_vfp((void *)__get_cpu_var(vfp_data));

	restore_generic_timer((void *)__get_cpu_var(timer_data));

	restore_performance_monitors((void *)__get_cpu_var(pmu_data));
}

/******************* Public Functions ***************/

/* Main dormant enter routine.  Must be called with
 * interrupts locked.  Will save/restore context of the CPU/CLUSTER
 * and returns back as a normal function call.
 */
void dormant_enter(u32 svc)
{
	u32 fd_cmd = CDC_CMD_CDCE;
	u32 pwr_ctrl;
	u32 cdc_states;
	int cdc_resp;
	u32 drmt_status = DORMANT_ENTRY_FAILURE;
	bool restore_gic = true;
	bool retry;
	u32 cpu;
	u32 svc_max = CORE_DORMANT;
	u32 insurance = 1000;
	u32 time1 = 0, time2 = 0;
	(*((u32 *)(&__get_cpu_var(cdm_attempts))))++;

	/*vote for dormant svc..*/
	set_svc_req(svc);
	instrument_lpm(LPM_TRACE_ENTER_DRMNT, svc);

	cdc_resp = cdc_send_cmd(CDC_CMD_RED);
	switch (cdc_resp) {

	case CDC_STATUS_NRFD:
		/*Some other core is entring dormant or an interrupt is pending.
		Retry later*/
		(*((u32 *)(&__get_cpu_var(cdm_failure_nrfd))))++;
		goto ret;
		break;

	case CDC_STATUS_RFD:
		fd_cmd = CDC_CMD_CDCE;
		break;

	case CDC_STATUS_RFDLC:
		svc_max = get_svc();

		if (fdm_en && (FULL_DORMANT_L2_ON == svc_max ||
			FULL_DORMANT_L2_OFF == svc_max)) {
			fd_cmd = CDC_CMD_FDCE;
		} else {
			fd_cmd = CDC_CMD_CDCE;
		}
		break;

	default:
		pr_err("%s cdc status: %u  cpu-%d\n", __func__,
				cdc_resp, smp_processor_id());
		BUG();
	}

	cdc_resp = cdc_send_cmd(fd_cmd);

	if (cdc_resp == CDC_STATUS_CENE) {
		(*((u32 *)(&__get_cpu_var(cdm_failure_cene))))++;
		cdc_resp = cdc_send_cmd(CDC_CMD_REDCAN);
		goto ret;
	}

	/* Save all the local data for this CPU for either service */
	save_arm_context();

	switch (cdc_resp) {

	case CDC_STATUS_FDCEOK:
		cdc_master_clk_gating_en(false);
		cdc_set_override(IS_IDLE_OVERRIDE, 0x1C0);
		save_proc_clk_regs();
		save_addnl_regs();
		save_gic_distributor_shared((void *)gic_dist_shared_data,
					    (u32)KONA_GICDIST_VA, false);
		if (l2_off_en && svc_max == FULL_DORMANT_L2_OFF)
			pwr_ctrl = CDC_PWR_DRMNT_L2_OFF;
		else
			pwr_ctrl = CDC_PWR_DRMNT_L2_ON;
		set_spare_power_status(pwr_ctrl);
		cdc_set_pwr_status(pwr_ctrl);
		/*Map enabled interrupts of all cores to
		COMMON_INT_TO_AC_EVENT event */
		clear_wakeup_interrupts();
		if (!pm_is_forced_sleep())
			config_wakeup_interrupts();
		/*Clear L2_IS_ON flags for FDCEOK irrespective
		of L2 ON status.Also clear other error status */
		cdc_set_fsm_ctrl(FSM_CLR_ALL_STATUS);
		cdc_set_override(WAIT_IDLE_TIMEOUT, 0xF);
		fdm_attempt++;
		if (unlikely(fdm_time_en))
			time1 = kona_hubtimer_get_counter();
		/*no break to continue to CEOK*/
	case CDC_STATUS_CEOK:
		/*dormant_enter_continue will turn OFF L2 mem only if
			 - svc_max == FULL_DORMANT_L2_OFF and
			 - CDC status == FDCEOK (last core entering dormant)*/
		drmt_status = cpu_suspend(svc_max, dormant_enter_continue);
		instrument_lpm(LPM_TRACE_DRMNT_WAKEUP, drmt_status);
		break;

	default:
		BUG();
	}

	/* We are here after either a failed dormant or a successful
	 * dormant
	 */

	/* if we failed to enter dormant, restore
	 * only what might have changed when dormant fails
	 */
	if (drmt_status == DORMANT_ENTRY_FAILURE) {
		/* restore only what we lost without entering
		 * dormant and return
		 */
		(*((u32 *)(&__get_cpu_var(cdm_failure_misc))))++;
		restore_control_registers((void *)__get_cpu_var(control_data),
					  false);
		restore_generic_timer((void *)__get_cpu_var(timer_data));
		restore_performance_monitors((void *)__get_cpu_var(pmu_data));
		goto ret;
	}


	/*get_cpu/put_cpu shouldn't be used
	before restoring context*/
	cpu = smp_processor_id();

	cdc_resp = cdc_get_status_for_core(cpu);
	instrument_lpm(LPM_TRACE_EXIT_DRMNT, cdc_resp);

	do {
		retry = false;
		insurance--;
		/*Read twice as master may return CDC_STATUS_RESFD_SHORT_WAIT
		for first read*/
		cdc_resp = cdc_get_status_for_core(cpu);

		switch (cdc_resp) {

		case CDC_STATUS_RESCD:
		case CDC_STATUS_RESFDS:
			if (CDC_STATUS_RESFDS == cdc_resp)
				restore_gic = true;
			/*No break continue...*/
		case CDC_STATUS_RESDFS_SHORT:
			(*((u32 *)(&__get_cpu_var(cdm_success))))++;
			cdc_resp = cdc_send_cmd_for_core(CDC_CMD_SDEC, cpu);
			break;

		case CDC_STATUS_RESFDM_SHORT:
			/*clear TIMEOUT_INT FDM_SHORT*/
			cdc_set_fsm_ctrl(FSM_CLR_TIMEOUT_INT);
			fdm_short_success++;
			/*No break continue...*/
		case CDC_STATUS_RESFDM:
			(*((u32 *)(&__get_cpu_var(cdm_success))))++;
#ifdef CONFIG_MEMC_FORCE_156M_IN_SUSPEND
			if (svc_max == FULL_DORMANT_L2_OFF)
				memc_update_dfs_req(&memc_dfs_node,
					MEMC_OPP_NORMAL);
#endif /*CONFIG_MEMC_FORCE_156M_IN_SUSPEND*/
			/* Calculate the time spent in full dormant */
			if (unlikely(fdm_time_en)) {
				time2 = kona_hubtimer_get_counter();
				if (time1 && (time2 > time1)) {
					fdm_time +=
					((time2 - time1) * 1000)
						/ CLOCK_TICK_RATE;
				}
			}
			if (CDC_STATUS_RESFDM == cdc_resp) {
				fdm_success++;
				restore_gic = true;
				restore_proc_clk_regs();
				restore_addnl_regs();
				/*restore GIC shared reg*/
				gic_distributor_set_enabled(false,
						    (u32)KONA_GICDIST_VA);
				restore_gic_distributor_shared((void *)
						       gic_dist_shared_data,
						       (u32)KONA_GICDIST_VA,
						       false);
				gic_distributor_set_enabled(true,
					(u32)KONA_GICDIST_VA);

				if (cdc_get_pwr_status() ==
					CDC_PWR_DRMNT_L2_OFF)
					l2_off_cnt++;
				instrument_lpm(LPM_TRACE_DRMNT_RS_ADDNL, 0);
			}
			clear_wakeup_interrupts();
			set_spare_power_status(CDC_PWR_NORMAL);
			cdc_set_pwr_status(CDC_PWR_NORMAL);
			/*Workaround for HWJAVA-215*/
			cdc_set_reset_counter(CD_RESET_TIMER, 0);
			cdc_states = CDC_STATUS_POR |
					CDC_STATUS_RESCDWAIT |
					CDC_STATUS_WAIT_CD_POK_STRONG |
					CDC_STATUS_WAIT_CD_POK_WEAK |
					CDC_STATUS_CLUSTER_DORMANT |
					CDC_STATUS_CORE_DORMANT;
			cdc_assert_reset_in_state(cdc_states);
			cdc_states = CDC_STATUS_POR |
				CDC_STATUS_RESCDWAIT |
				CDC_STATUS_WAIT_CD_POK_STRONG |
				CDC_STATUS_WAIT_CD_POK_WEAK |
				CDC_STATUS_RESVD |
				CDC_STATUS_CD_CLAMP_ASSERT |
				CDC_STATUS_CLUSTER_WAIT_IDLE |
				CDC_STATUS_CLUSTER_DORMANT |
				CDC_STATUS_CORE_DORMANT;
			cdc_enable_isolation_in_state(cdc_states);
			cdc_states = CDC_STATUS_RFD |
				CDC_STATUS_CENE |
				CDC_STATUS_RFDLC |
				CDC_STATUS_CEOK |
				CDC_STATUS_FDCEOK |
				CDC_STATUS_RESFDM_SHORT |
				CDC_STATUS_WAIT_CD_POK_STRONG |
				CDC_STATUS_WAIT_CD_POK_WEAK |
				CDC_STATUS_RESDFS_SHORT |
				CDC_STATUS_CD_CLAMP_ASSERT |
				CDC_STATUS_CLUSTER_WAIT_IDLE |
				CDC_STATUS_CLUSTER_DORMANT |
				CDC_STATUS_RESFD_SHORT_WAIT;
			cdc_assert_cdcbusy_in_state(cdc_states);

			cdc_set_switch_counter(WEAK_SWITCH_TIMER, 0x0C);
			cdc_set_switch_counter(STRONG_SWITCH_TIMER, 0x0C);

			cdc_resp = cdc_send_cmd_for_core(CDC_CMD_MDEC, cpu);
			cdc_set_override(IS_IDLE_OVERRIDE, 0x180);
			cdc_master_clk_gating_en(true);
			break;

		case CDC_STATUS_RESFD_SHORT_WAIT:
		case CDC_STATUS_RESFDWAIT:
		case CDC_STATUS_RESCDWAIT:
			retry = true;
			/*wfe();*/
			break;

		default:
			pr_err("%s: cdc_resp = %x\n", __func__,
				cdc_resp);
			BUG();
		}
	} while (retry && insurance);
	BUG_ON(!insurance && retry);

	instrument_lpm(LPM_TRACE_DRMNT_RS1, 0);

	/* restore everything that is specific to this core */
	restore_arm_context(restore_gic);

	instrument_lpm(LPM_TRACE_DRMNT_RS2, 0);
ret:
	/*Clr svc vote*/
	clr_svc_req(svc);

	instrument_lpm(LPM_TRACE_EXIT_DRMNT, 0);
}

/*
 * Routine called from assembly, after saving the CPU context
 * this is where WFI would be executed to take down the power
 */
static int dormant_enter_continue(unsigned long svc)
{
	u32 cpu;
	unsigned int arg2 = 2; /* L2 mem ON */
	cpu = smp_processor_id();

	if (svc == FULL_DORMANT_L2_OFF &&
		cdc_get_status_for_core(cpu) == CDC_STATUS_FDCEOK) {
#ifdef CONFIG_MEMC_FORCE_156M_IN_SUSPEND
		memc_update_dfs_req(&memc_dfs_node, MEMC_OPP_ECO);
#endif

		if (l2_off_en) {
			arg2 = 3;  /*L2 mem OFF*/
#ifndef CONFIG_MOBICORE_DRIVER
			disable_clean_inv_dcache_v7_all();
#endif
		} else {
			arg2 = 2;
			disable_clean_inv_dcache_v7_l1();
		}

	} else {
		arg2 = 2;  /*L2 mem ON*/
#ifndef CONFIG_MOBICORE_DRIVER
		disable_clean_inv_dcache_v7_l1();
#endif
	}

#ifndef CONFIG_MOBICORE_DRIVER
	write_actlr(read_actlr() & ~A15_SMP_BIT);
#endif

/* Inform Secure Core (core 0) that we are entering dormant.
 * so that Secure core( core 0) will save secure world context
 * and the return address.
 */
	if (cpu == 0) {
#ifdef CONFIG_MOBICORE_DRIVER
		u32 smc_cmd = SMC_CMD_SLEEP;
#else
		u32 smc_cmd = SSAPI_DORMANT_ENTRY_SERV;
#endif
		secure_params->core0_reset_address = virt_to_phys(cpu_resume);

		secure_params->core1_reset_address = virt_to_phys(cpu_resume);

		secure_params->dram_log_buffer = drmt_buf_phy;

		local_secure_api(smc_cmd,
				 (u32)SEC_BUFFER_ADDR,
				 (u32)SEC_BUFFER_ADDR +
				 MAX_SECURE_BUFFER_SIZE, arg2);
	} else {
		/* Write the address where we want other cores to boot */
		writel_relaxed(virt_to_phys(cpu_resume),
				KONA_CHIPREG_VA +
				CHIPREG_BOOT_2ND_ADDR_OFFSET);
		writel_relaxed(virt_to_phys(cpu_resume),
				KONA_CHIPREG_VA +
				CHIPREG_A9_DORMANT_BOOT_ADDR_REG0_OFFSET);
		writel_relaxed(virt_to_phys(cpu_resume),
				KONA_CHIPREG_VA +
				CHIPREG_A9_DORMANT_BOOT_ADDR_REG1_OFFSET);
		/* Directly execute WFI for non core-0 cores */
	}
#ifdef CONFIG_MOBICORE_DRIVER
	if (arg2 == 3)
		disable_clean_inv_dcache_v7_all();
	else
		disable_clean_inv_dcache_v7_l1();

	write_actlr(read_actlr() & ~A15_SMP_BIT);
#endif
	instrument_lpm(LPM_TRACE_DRMNT_CNTNUE, arg2);
	wfi();
	return 1;
}

#ifdef CONFIG_DEBUG_FS
static struct dentry *dm_root_dir;

static int __init dm_debug_init(void)
{
	#define DIR_STR_LEN 10
	int cpu;
	char str[DIR_STR_LEN];
	static struct dentry *cpu_dir[CONFIG_NR_CPUS];

	/* create root clock dir /clock */
	dm_root_dir = debugfs_create_dir("dm", 0);
	if (!dm_root_dir)
		return -ENOMEM;
	for_each_present_cpu(cpu) {
		snprintf(str, DIR_STR_LEN - 1, "cpu%d", cpu);
		cpu_dir[cpu] = debugfs_create_dir(str, dm_root_dir);
		if (!cpu_dir[cpu])
			goto err;
		if (!debugfs_create_u32("cdm_success", S_IRUGO,
			cpu_dir[cpu], (u32 *)&per_cpu(cdm_success, cpu)))
			goto err;

		if (!debugfs_create_u32("cdm_failure_nrfd", S_IRUGO,
			cpu_dir[cpu], (u32 *)&per_cpu(cdm_failure_nrfd, cpu)))
			goto err;

		if (!debugfs_create_u32("cdm_failure_cene", S_IRUGO,
			cpu_dir[cpu], (u32 *)&per_cpu(cdm_failure_cene, cpu)))
			goto err;

		if (!debugfs_create_u32("cdm_failure_misc", S_IRUGO,
			cpu_dir[cpu], (u32 *)&per_cpu(cdm_failure_misc, cpu)))
			goto err;

		if (!debugfs_create_u32("cdm_attempts", S_IRUGO,
			cpu_dir[cpu], (u32 *)&per_cpu(cdm_attempts, cpu)))
			goto err;

	}
	if (!debugfs_create_u32("l2_off_cnt", S_IRUGO,
			dm_root_dir, &l2_off_cnt))
			goto err;

	if (!debugfs_create_u32("fdm_success", S_IRUGO,
			dm_root_dir, &fdm_success))
		goto err;

	if (!debugfs_create_u32("fdm_short_success", S_IRUGO,
			dm_root_dir, &fdm_short_success))
		goto err;

	if (!debugfs_create_u32("fdm_attempt", S_IRUGO,
			dm_root_dir, &fdm_attempt))
		goto err;

	if (!debugfs_create_u32("l2_off_en", S_IRUGO | S_IWUSR,
			dm_root_dir, &l2_off_en))
		goto err;

	if (!debugfs_create_u32("fdm_en", S_IRUGO | S_IWUSR,
			dm_root_dir, &fdm_en))
		goto err;

	if (!debugfs_create_u32("fdm_time", S_IRUGO | S_IWUSR,
			dm_root_dir, &fdm_time))
		goto err;

	if (!debugfs_create_u32("fdm_time_en", S_IRUGO | S_IWUSR,
			dm_root_dir, &fdm_time_en))
		goto err;

	if (!debugfs_create_u32("dbg_log", S_IRUGO | S_IWUSR,
			dm_root_dir, &dbg_log))
		goto err;

	return 0;
err:
	debugfs_remove_recursive(dm_root_dir);
	return -ENOMEM;
}

#endif


/* Initialization function for dormant module */
static int __init dm_init(void)
{
	struct resource *res;
	u32 cdc_states;

	void *vptr = NULL;
	struct clk *clk;
	int i;

	clk = clk_get(NULL, KPROC_CCU_CLK_NAME_STR);
	if (IS_ERR_OR_NULL(clk))
		return -EPERM;
	proc_ccu = to_ccu_clk(clk);
	if (proc_ccu == NULL)
		return -EPERM;

	vptr = dma_alloc_coherent(NULL, SZ_4K,
				  &drmt_buf_phy, GFP_ATOMIC);

	if (vptr == NULL) {
		pr_info("%s: dormant dma buffer alloc failed\n", __func__);
		return -ENOMEM;
	}
	un_cached_stack_ptr = (u32)vptr;
	pr_info("%s:secure side dram log buffer; drmt_buf_phy = 0x%x",
		__func__, drmt_buf_phy);

	res = request_mem_region(SEC_BUFFER_ADDR, SEC_BUFFER_SIZE,
				 "secure_params");
	BUG_ON(!res);

	secure_params =
	    (struct secure_params_t *)ioremap_nocache(SEC_BUFFER_ADDR,
						      SEC_BUFFER_SIZE);

	BUG_ON(!secure_params);

	secure_params->log_index = 0;
	secure_params->dram_log_buffer = drmt_buf_phy;
	secure_params->core0_reset_address = virt_to_phys(cpu_resume);
	pr_info("%s: secure_params->core0_reset_address = %x\n", __func__,
		secure_params->core0_reset_address);

	/*Workaround for HWJAVA-215*/
	cdc_set_reset_counter(CD_RESET_TIMER, 0);
	cdc_states = CDC_STATUS_POR |
			CDC_STATUS_RESCDWAIT |
			CDC_STATUS_WAIT_CD_POK_STRONG |
			CDC_STATUS_WAIT_CD_POK_WEAK |
			CDC_STATUS_CLUSTER_DORMANT |
			CDC_STATUS_CORE_DORMANT;
	cdc_assert_reset_in_state(cdc_states);
	cdc_states = CDC_STATUS_POR |
		CDC_STATUS_RESCDWAIT |
		CDC_STATUS_WAIT_CD_POK_STRONG |
		CDC_STATUS_WAIT_CD_POK_WEAK |
		CDC_STATUS_RESVD |
		CDC_STATUS_CD_CLAMP_ASSERT |
		CDC_STATUS_CLUSTER_WAIT_IDLE |
		CDC_STATUS_CLUSTER_DORMANT |
		CDC_STATUS_CORE_DORMANT;
	cdc_enable_isolation_in_state(cdc_states);
	cdc_states = CDC_STATUS_RFD |
		CDC_STATUS_CENE |
		CDC_STATUS_RFDLC |
		CDC_STATUS_CEOK |
		CDC_STATUS_FDCEOK |
		CDC_STATUS_RESFDM_SHORT |
		CDC_STATUS_WAIT_CD_POK_STRONG |
		CDC_STATUS_WAIT_CD_POK_WEAK |
		CDC_STATUS_RESDFS_SHORT |
		CDC_STATUS_CD_CLAMP_ASSERT |
		CDC_STATUS_CLUSTER_WAIT_IDLE |
		CDC_STATUS_CLUSTER_DORMANT |
		CDC_STATUS_RESFD_SHORT_WAIT;
	cdc_assert_cdcbusy_in_state(cdc_states);

	cdc_master_clk_gating_en(true);
	cdc_set_switch_counter(WEAK_SWITCH_TIMER, 0x0C);
	cdc_set_switch_counter(STRONG_SWITCH_TIMER, 0x0C);

	/*Find index of PLLARMA register in proc CCU reg save list*/
	for (i = 0; i < ARRAY_SIZE(proc_clk_regs); i++) {
		if (PROC_CLK_ITEM_ADDR(i) == PROC_CLK_REG_ADDR(PLLARMA)) {
			pllarma_inx = i;
			break;
		}
	}

#ifdef CONFIG_MEMC_FORCE_156M_IN_SUSPEND
	memc_add_dfs_req(&memc_dfs_node, "dm", MEMC_OPP_NORMAL);
#endif

#ifdef CONFIG_DEBUG_FS
	if (dm_debug_init() != 0) {
		dma_free_coherent(NULL, SZ_4K, vptr, drmt_buf_phy);
		return -ENOMEM;
	}
#endif
	return 0;
}
module_init(dm_init);

static void __exit dm_exit(void)
{
	dma_free_coherent(NULL, SZ_4K, (void *) un_cached_stack_ptr,
							drmt_buf_phy);
}
module_exit(dm_exit);
