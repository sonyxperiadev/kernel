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
#include <linux/dma-mapping.h>
#include <plat/kona_pm_dbg.h>
#include <plat/pwr_mgr.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_csr.h>
#include <mach/rdb/brcm_rdb_hsotg_ctrl.h>
#include <mach/rdb/brcm_rdb_gpio.h>
#include <mach/pm.h>
#include <mach/pwr_mgr.h>

/*****************************************************************************
 *                        SLEEP STATE DEBUG INTERFACE                        *
 *****************************************************************************/

u32 *dormant_trace_v;
u32 *dormant_trace_p;

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
	  "echo 'cmd string' > /sys/module/rhea_pm/parameters/debug\n"
	  "'cmd string' is constructed as follows:\n"
#ifdef CONFIG_RHEA_DORMANT_MODE
	  "set dormant gpio d g s <gpio number>\n"
	  "enable dormant gpio d g e\n"
	  "disable dormant gpio d g c\n"
	  "display dormant gpio data d g d\n"
#endif
	  "force sleep: f <state from 0 to 4>\n"
	  "display stats: s\n"
	  "\n";

	pr_info("%s", usage);
}

#ifdef CONFIG_RHEA_DORMANT_MODE
#define GPIO_GPORS_BASE_PHYS    (GPIO2_BASE_ADDR + GPIO_GPORS0_OFFSET)
#define GPIO_GPORS_BASE_VIRT    (KONA_GPIO2_VA + GPIO_GPORS0_OFFSET)
#define GPIO_GPORC_BASE_PHYS    (GPIO2_BASE_ADDR + GPIO_GPORC0_OFFSET)
#define GPIO_GPORC_BASE_VIRT    (KONA_GPIO2_VA + GPIO_GPORC0_OFFSET)

static void cmd_dormant_gpio(const char *p)
{
	u32 gpio;
	u32 reg;

	switch (*p) {
	case 'e':
		pr_info("%s: enabling dormant gpio\n", __func__);
		dormant_gpio_data.enable = 1;
		break;
	case 'c':
		pr_info("%s: disabling dormant gpio\n", __func__);
		dormant_gpio_data.enable = 0;
		break;
	case 's':
		p++;
		while (*p == ' ' || *p == '\t')
			p++;

		sscanf(p, "%d", &gpio);
		pr_info("%s: setting dormant gpio to %d\n", __func__, gpio);
		reg = gpio / 32;
		dormant_gpio_data.gpio_bit = 1 << (gpio % 32);
		dormant_gpio_data.gpio_set_p = GPIO_GPORS_BASE_PHYS + reg * 4;
		dormant_gpio_data.gpio_set_v = GPIO_GPORS_BASE_VIRT + reg * 4;
		dormant_gpio_data.gpio_clr_p = GPIO_GPORC_BASE_PHYS + reg * 4;
		dormant_gpio_data.gpio_clr_v = GPIO_GPORC_BASE_VIRT + reg * 4;
		break;
	case 'd':
		pr_info("enable       : %d\n", dormant_gpio_data.enable);
		pr_info("gpio set phys: %x\n", dormant_gpio_data.gpio_set_p);
		pr_info("gpio set virt: %x\n", dormant_gpio_data.gpio_set_v);
		pr_info("gpio clr phys: %x\n", dormant_gpio_data.gpio_clr_p);
		pr_info("gpio clr virt: %x\n", dormant_gpio_data.gpio_clr_v);
		pr_info("gpio bit     : %x\n", dormant_gpio_data.gpio_bit);
		break;
	}
}

static void cmd_dormant(const char *p)
{
	char cmd = *p;

	p++;
	while (*p == ' ' || *p == '\t')
		p++;

	switch (cmd) {
	case 'g': /* Handle dormant gpio related commands */
		cmd_dormant_gpio(p);
		break;
	}
}
#endif

static void cmd_display_stats(const char *p)
{
	pr_info("dormant trace v:%p, p:%p\n", dormant_trace_v, dormant_trace_p);
	pr_info("dormant trace %x\n", *dormant_trace_v);
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
	kona_pm_reg_pm_enter_handler(&rhea_force_sleep);
	request_suspend_state(PM_SUSPEND_MEM);
}

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
#ifdef CONFIG_RHEA_DORMANT_MODE
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

void instrument_dormant_entry(void)
{
	if (dormant_trace_v)
		*dormant_trace_v = DORMANT_ENTRY;
}

void instrument_dormant_exit(void)
{
	if (dormant_trace_v)
		*dormant_trace_v = DORMANT_EXIT;
}

/*****************************************************************************
 *               INTERFACE TO TAKE REGISTER SNAPSHOT BEFORE SLEEP            *
 *****************************************************************************/

#define MIN_PWR_STATE		\
	(KONA_MEMC0_NS_VA + CSR_APPS_MIN_PWR_STATE_OFFSET)
#define USB_OTG_P1CTL		\
	(KONA_USB_HSOTG_CTRL_VA + HSOTG_CTRL_PHY_P1CTL_OFFSET)

/* SNAPSHOT TABLE:
 * ---------------
 * Table of registers to be sampled before entering low power
 * state for debugging.
 */
static struct snapshot snapshot[] = {

	/*
	 * Simple register parms
	 */
	SIMPLE_PARM(MIN_PWR_STATE, 0, 3),

	/*
	 * List of clocks that prevent entry to low power state
	 */
	CLK_PARM("dig_ch0_clk"),

	/*
	 * AHB register parms (needs AHB clk enabled before register read)
	 */
	AHB_REG_PARM(USB_OTG_P1CTL, 0, (1 << 30), "usb_otg_clk"),
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
}

void instrument_idle_exit(void)
{
}

int __init rhea_pmdbg_init(void)
{
#ifdef CONFIG_RHEA_DORMANT_MODE
	void *v = NULL;
	dma_addr_t p;
#endif

	snapshot_table_register(snapshot, ARRAY_SIZE(snapshot));
#ifdef CONFIG_RHEA_DORMANT_MODE
	dormant_gpio_data.enable = 0;

	v = dma_alloc_coherent(NULL, SZ_1K, &p, GFP_ATOMIC);
	if (v == NULL) {
		pr_info("%s: tracer dma buffer alloc failed\n", __func__);
		return -ENOMEM;
	}

	dormant_trace_v = (u32 *) v;
	dormant_trace_p = (u32 *) p;

	pr_info("dormant trace v:0x%x, p:0x%x\n", (u32)v, (u32)p);
#endif

	return 0;
}

arch_initcall(rhea_pmdbg_init);
