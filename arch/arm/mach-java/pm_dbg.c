/******************************************************************************/
/* (c) 2011 Broadcom Corporation                                              */
/*                                                                            */
/* Unless you and Broadcom execute a separate written software license        */
/* agreement governing use of this software, this software is licensed to you */
/* under the terms of the GNU General Public License version 2, available at  */
/* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").                    */
/*                                                                            */
/******************************************************************************/

#include <linux/module.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <plat/kona_pm_dbg.h>
#include <plat/pwr_mgr.h>
#include <plat/pi_mgr.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_csr.h>
#include <mach/rdb/brcm_rdb_hsotg_ctrl.h>
#include <mach/rdb/brcm_rdb_gpio.h>
#include <mach/pm.h>
#include <mach/pwr_mgr.h>
#include <mach/pi_mgr.h>
#include <mach/memory.h>
#ifdef CONFIG_KONA_PROFILER
#include <plat/profiler.h>
#endif

/*****************************************************************************
 *                        SLEEP STATE DEBUG INTERFACE                        *
 *****************************************************************************/


#define GPIO_GPORS_BASE_PHYS    (GPIO2_BASE_ADDR + GPIO_GPORS0_OFFSET)
#define GPIO_GPORS_BASE_VIRT    (KONA_GPIO2_VA + GPIO_GPORS0_OFFSET)
#define GPIO_GPORC_BASE_PHYS    (GPIO2_BASE_ADDR + GPIO_GPORC0_OFFSET)
#define GPIO_GPORC_BASE_VIRT    (KONA_GPIO2_VA + GPIO_GPORC0_OFFSET)

#define CURR_DORM_TRACE_OFFSET		(SZ_4)
#define DORMANT_TRACE_OFFSET		(SZ_8)
#define	RETENTION_TRACE_OFFSET		(SZ_1K)
#define	WFI_TRACE_OFFSET		(SZ_2K)
#define	TRACE_PATTERN_OFFSET		0 /* 1st word from offset */
#define	COUNTER_OFFSET			1 /* 2nd word from offset */

/* No check for gpio number to speed up the API */
void dbg_gpio_set(u32 gpio)
{
	u32 reg, bit;

	reg = gpio / 32;
	reg = GPIO_GPORS_BASE_VIRT + reg * 4;
	bit = 1 << (gpio % 32);

	__raw_writel(bit, reg);
}
EXPORT_SYMBOL_GPL(dbg_gpio_set);

/* No check for gpio number to speed up the API */
void dbg_gpio_clr(u32 gpio)
{
	u32 reg, bit;

	reg = gpio / 32;
	reg = GPIO_GPORC_BASE_VIRT + reg * 4;
	bit = 1 << (gpio % 32);

	__raw_writel(bit, reg);
}
EXPORT_SYMBOL_GPL(dbg_gpio_clr);

/*
 * Dormant mode profiling
 */
#if defined(DORMANT_PROFILE) && defined(CONFIG_A9_DORMANT_MODE)

static u32 ns_gpio;
static u32 sec_gpio;
static u32 ref_gpio;

void clear_ns_gpio(void)
{
	if (dormant_profile_on)
		dbg_gpio_clr(ns_gpio);
}

static void dormant_profile_entry(void)
{
	if (dormant_profile_on) {
		dbg_gpio_set(ns_gpio);
		dbg_gpio_set(sec_gpio);
		dbg_gpio_set(ref_gpio);
	}
}

static void dormant_profile_exit(void)
{
	if (dormant_profile_on) {
		dbg_gpio_clr(ns_gpio);
		dbg_gpio_clr(sec_gpio);
		dbg_gpio_clr(ref_gpio);
	}
}

static void dormant_profile_config(u32 on, u32 ns, u32 sec, u32 ref)
{
	u32 reg;

	/* Setup configs for C routines */
	ns_gpio = ns;
	sec_gpio = sec;
	ref_gpio = ref;

	/* Setup configs for asm routines */
	reg = ns / 32;
	ns_gpio_set_v = GPIO_GPORS_BASE_VIRT + reg * 4;
	ns_gpio_clr_v = GPIO_GPORC_BASE_VIRT + reg * 4;
	ns_gpio_set_p = GPIO_GPORS_BASE_PHYS + reg * 4;
	ns_gpio_clr_p = GPIO_GPORC_BASE_PHYS + reg * 4;
	ns_gpio_bit = 1 << (ns % 32);

	/* Setup common configs */
	dormant_profile_on = on;
}
#else /* !DORMANT_PROFILE && !CONFIG_A9_DORMANT_MODE */
void clear_ns_gpio(void)
{
}

static void dormant_profile_entry(void)
{
}

static void dormant_profile_exit(void)
{
}

static void dormant_profile_config(u32 on, u32 ns, u32 sec, u32 ref)
{
}
#endif /* DORMANT_PROFILE && CONFIG_A9_DORMANT_MODE */
u32 dorm_profile_enable;

struct lowpower_trace {
	u32 *dormant_trace_v;
	u32 *dormant_trace_p;
	u32 *retention_trace;
	u32 *wfi_trace;
	u32 *dorm_count;
	u32 *curr_dorm_trace;
};

struct lowpower_trace lp_trace[CONFIG_NR_CPUS];

struct debug {
	int dummy;
};

#define __param_check_debug(name, p, type) \
	static inline struct type *__check_##name(void) { return (p); }

#define param_check_debug(name, p) \
	__param_check_debug(name, p, debug)

static int param_set_debug(const char *val, const struct kernel_param *kp);
static int param_get_debug(char *buffer, const struct kernel_param *kp);

static struct kernel_param_ops param_ops_debug = {
	.set = param_set_debug,
	.get = param_get_debug,
};

static struct debug debug;
module_param_named(debug, debug, debug, S_IRUGO | S_IWUSR | S_IWGRP);

/* List of supported commands */
enum {
	CMD_SHOW_HELP = 'h',
	CMD_DORMANT = 'd',
	CMD_DISPLAY_STATS = 's',
	CMD_FORCE_SLEEP = 'f',
};

static void cmd_show_usage(void)
{
	const char usage[] = "Usage:\n"
	  "echo 'cmd string' > /sys/module/pm_dbg/parameters/debug\n"
	  "'cmd string' is constructed as follows:\n"
#ifdef DORMANT_PROFILE
	  "start dormant profile: d p 1 <ns_gpio> <sec_gpio> <ref_gpio>\n"
	  "stop dormant profile : d p 0 0 0 0\n"
#endif
	  "force sleep: f <state from 0 to 4>\n"
	  "display stats: s\n"
	  "\n";

	pr_info("%s", usage);
}

static void cmd_display_stats(const char *p)
{
}

static int force_sleep_state = 2;
int get_force_sleep_state(void)
{
	return force_sleep_state;
}


static void cmd_force_sleep(const char *p)
{
	sscanf(p, "%d", &force_sleep_state);
	if (force_sleep_state < 0 || force_sleep_state > 4) {
		pr_err("Invalid state: %d\n", force_sleep_state);
		force_sleep_state = 2;
		return;
	}

	pr_info("Forcing system to state: %d\n", force_sleep_state);
	kona_pm_reg_pm_enter_handler(&hawaii_force_sleep);

	request_suspend_state(PM_SUSPEND_MEM);
}

#ifdef CONFIG_A9_DORMANT_MODE
static void cmd_dormant_profile(const char *p)
{
	u32 on, ns, sec, ref;

	sscanf(p, "%08x %08x %08x %08x", &on, &ns, &sec, &ref);
	dormant_profile_config(on, ns, sec, ref);
}

static void cmd_dormant(const char *p)
{
	char cmd = *p;

	p++;
	while (*p == ' ' || *p == '\t')
		p++;

	switch (cmd) {
	case 'p': /* Handle dormant profile commands */
		cmd_dormant_profile(p);
		break;
	}
}
#endif

static int param_set_debug(const char *val, const struct kernel_param *kp)
{
	const char *p;

	if (!val)
		return -EINVAL;

	p = &val[1];

	/* First character is the command. Skip past all whitespaces
	 * after the command to reach the arguments, if any.
	 */
	while (*p == ' ' || *p == '\t')
		p++;

	switch (val[0]) {
#ifdef CONFIG_A9_DORMANT_MODE
	case CMD_DORMANT:
		cmd_dormant(p);
		break;
#endif
	case CMD_DISPLAY_STATS:
		cmd_display_stats(p);
		break;
	case CMD_FORCE_SLEEP:
		cmd_force_sleep(p);
		break;
	case CMD_SHOW_HELP: /* Fall-through */
	default:
		cmd_show_usage();
		break;
	}

	return 0;
}

static int param_get_debug(char *buffer, const struct kernel_param *kp)
{
	cmd_show_usage();
	return 0;
}

/*****************************************************************************
 *                       DORMANT MODE INSTRUMENTATION                        *
 *****************************************************************************/
void instrument_dormant_trace(u32 trace, u32 service, u32 success)
{
	u32 cpu = smp_processor_id();
	if (lp_trace[cpu].dormant_trace_v) {
		lp_trace[cpu].curr_dorm_trace
			= lp_trace[cpu].dormant_trace_v
				+ *lp_trace[cpu].dorm_count;
		*(lp_trace[cpu].curr_dorm_trace)
			= trace + success + (service * 2);
		*lp_trace[cpu].dorm_count
			= (*lp_trace[cpu].dorm_count + 1)
				& DORMANT_MAX_TRACE_ENTRIES;
	}
}

/*****************************************************************************
 *               INTERFACE TO TAKE REGISTER SNAPSHOT BEFORE SLEEP            *
 *****************************************************************************/

static u32 mm_pi_id = PI_MGR_PI_ID_MM;
static u32 hub_pi_id = PI_MGR_PI_ID_HUB_SWITCHABLE;
static u32 hub_aon_pi_id = PI_MGR_PI_ID_HUB_AON;
static u32 arm_pi_id = PI_MGR_PI_ID_ARM_CORE;
static u32 arm_subsys_pi_id = PI_MGR_PI_ID_ARM_SUB_SYSTEM;
static u32 modem_pi_id = PI_MGR_PI_ID_MODEM;

static u32 get_pi_count(void *data)
{
	u32 ret = 0;
	int id = *(int *)data;

	ret = pi_get_use_count(id);

	return ret;
}

#define AP_MIN_PWR_STATE		\
	(KONA_MEMC0_NS_VA + CSR_APPS_MIN_PWR_STATE_OFFSET)
#define MODEM_MIN_PWR_STATE		\
	(KONA_MEMC0_NS_VA + CSR_MODEM_MIN_PWR_STATE_OFFSET)
#define DSP_MIN_PWR_STATE		\
	(KONA_MEMC0_NS_VA + CSR_DSP_MIN_PWR_STATE_OFFSET)
#define USB_OTG_P1CTL			\
	(KONA_USB_HSOTG_CTRL_VA + HSOTG_CTRL_PHY_P1CTL_OFFSET)
#define HW_FREQ_CHANGE_CNTRL		\
	(KONA_MEMC0_NS_VA+CSR_HW_FREQ_CHANGE_CNTRL_OFFSET)
#define DDR_PLL_PWRDN_BIT CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_PWRDN_ENABLE_MASK

/* SNAPSHOT TABLE:
 * ---------------
 * Table of registers to be sampled before entering low power
 * state for debugging.
 */
static struct snapshot snapshot[] = {

	/*
	 * Simple register parms
	 */
	SIMPLE_PARM(AP_MIN_PWR_STATE, 0, 3),
	SIMPLE_PARM(MODEM_MIN_PWR_STATE, 0, 3),
	SIMPLE_PARM(DSP_MIN_PWR_STATE, 0, 3),
	SIMPLE_PARM(HW_FREQ_CHANGE_CNTRL, DDR_PLL_PWRDN_BIT, DDR_PLL_PWRDN_BIT),

	/*
	 * List of clocks that prevent entry to low power state
	 */
	CLK_PARM("dig_ch0_clk"),
	CLK_PARM("tpiu_clk"),
	CLK_PARM("pti_clk"),

	/*
	 * AHB register parms (needs AHB clk enabled before register read)
	 */
	AHB_REG_PARM(USB_OTG_P1CTL, 0, (1 << 30), "usb_otg_clk"),

	/*
	 * PI usage counts
	 */
	USER_DEFINED_PARM(get_pi_count, &mm_pi_id, "mm"),
	USER_DEFINED_PARM(get_pi_count, &hub_pi_id, "hub"),
	USER_DEFINED_PARM(get_pi_count, &hub_aon_pi_id, "hub_aon"),
	USER_DEFINED_PARM(get_pi_count, &arm_pi_id, "arm"),
	USER_DEFINED_PARM(get_pi_count, &arm_subsys_pi_id, "arm_subsys"),
	USER_DEFINED_PARM(get_pi_count, &modem_pi_id, "modem"),
};

/*****************************************************************************
 *                        INSTRUMENT LOW POWER STATES                        *
 *****************************************************************************/

void instrument_idle_entry(void)
{
	/* Take snapshot of registers that can potentially prevent system from
	 * entering low power state.
	 */
	snapshot_get();

	/**
	 * Take profiling counter samples
	 * before entering idle state
	 */
#ifdef CONFIG_KONA_PROFILER
	profiler_idle_entry_cb();
#endif
}

void instrument_idle_exit(void)
{
}

/*****************************************************************************
 *                       RETENTION MODE INSTRUMENTATION                        *
 *****************************************************************************/

void instrument_retention(int trace_path)
{
}

/*****************************************************************************
 *                       IDLE INSTRUMENTATION                        *
 *****************************************************************************/

void instrument_wfi(int trace_path)
{
}


int __init __pmdbg_init(void)
{
	void *v[CONFIG_NR_CPUS];
	dma_addr_t p[CONFIG_NR_CPUS];
	int i;

	snapshot_table_register(snapshot, ARRAY_SIZE(snapshot));
	dormant_profile_config(0, 0, 0, 0);

	for (i = 0; i < CONFIG_NR_CPUS; i++) {
		v[i] = dma_alloc_coherent(NULL, SZ_4K, &p[i],
				GFP_ATOMIC | __GFP_ZERO);
		if (v[i] == NULL) {
			pr_info("%s: cpu%d tracer dma buffer alloc failed\n",
				__func__, i);
			return -ENOMEM;
		}
		lp_trace[i].dorm_count = (u32 *) v[i];
		lp_trace[i].curr_dorm_trace = (u32 *)((u32)v[i]
						+ CURR_DORM_TRACE_OFFSET);
		lp_trace[i].dormant_trace_v = (u32 *)((u32)v[i]
						+ DORMANT_TRACE_OFFSET);
		lp_trace[i].dormant_trace_p = (u32 *)((u32)p[i]
						+ DORMANT_TRACE_OFFSET);
		lp_trace[i].retention_trace = (u32 *)((u32)v[i]
						+ RETENTION_TRACE_OFFSET);
		lp_trace[i].wfi_trace = (u32 *)((u32)v[i] + WFI_TRACE_OFFSET);
		*lp_trace[i].dorm_count = 0;
		pr_info("%s: cpu%d dormant_trace_v:0x%x,_p:0x%x\n",
				__func__, i, (u32) v[i], (u32) p[i]);
		pr_info("%s: cpu%d ret_trace:0x%x; wfi_trace:0x%x\n",
			__func__, i, (u32) lp_trace[i].retention_trace,
				(u32) lp_trace[i].wfi_trace);
	}
	return 0;
}

arch_initcall(__pmdbg_init);
