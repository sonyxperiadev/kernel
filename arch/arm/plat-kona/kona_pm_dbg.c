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
#include <linux/io.h>
#include <plat/clock.h>
#include <plat/kona_pm_dbg.h>

/* Snapshot handlers */
static u32 handle_simple_parm(struct snapshot *s);
static u32 handle_clk_parm(struct snapshot *s);
static u32 handle_ahb_reg_parm(struct snapshot *s);

/*****************************************************************************
 *                        SLEEP STATE DEBUG INTERFACE                        *
 *****************************************************************************/

struct debug {
	int snapshot_enable;	/* 1. enable snapshot, 0 disable snapshot */
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

#define is_snapshot_enabled	(debug.snapshot_enable == 0 ? 0 : 1)

/* List of supported commands */
enum {
	CMD_SNAPSHOT = 's',
	CMD_SHOW_HELP = 'h',
};

static void cmd_show_usage(void)
{
	const char usage[] = "Usage:\n"
	  "echo 'cmd string' > /sys/module/rhea_pm/parameters/debug\n"
	  "'cmd string' is constructed as follows:\n"
	  "Enable snapshot: s e\n"
	  "Disable snapshot: s d\n"
	  "Take shapshot: s t\n"
	  "Show shapshot: s s\n"
	  "Display help: h\n"
	  "\n";

	pr_info("%s", usage);
}

static void cmd_snapshot(const char *p)
{
	unsigned long flags;

	local_irq_save(flags);

	switch (*p) {
	case 's':
		snapshot_show();
		break;
	case 't':
		snapshot_get();
		break;
	case 'e':
		debug.snapshot_enable = 1;
		break;
	case 'd':
		debug.snapshot_enable = 0;
		break;
	default:
		pr_err("Unsupported option\n");
		break;
	}

	local_irq_restore(flags);
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
	case CMD_SNAPSHOT:
		cmd_snapshot(p);
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
 *               INTERFACE TO TAKE REGISTER SNAPSHOT BEFORE SLEEP            *
 *****************************************************************************/


/* Table of registers to be sampled before entering low power
 * state for debugging.
 */
static struct snapshot *snapshot;
static size_t snapshot_len;

/*
 * Snapshot handlers
 */


/* Returns the current masked value of the specified register */
static u32 handle_simple_parm(struct snapshot *s)
{
	u32 ret = 0;

	if (s)
		ret = readl(s->reg) & s->mask;

	return ret;
}

/* Returns the current use count of the clk */
static u32 handle_clk_parm(struct snapshot *s)
{
	struct clk *clk;
	u32 ret = 0;

	if (!s)
		goto err;

	clk = clk_get(NULL, (char *)s->data);
	if (IS_ERR_OR_NULL(clk))
		goto err;

	ret = clk_get_usage(clk);
	clk_put(clk);

err:
	return ret;
}

/* Returns the current masked value of the specified register */
static u32 handle_ahb_reg_parm(struct snapshot *s)
{
	struct clk *clk;
	u32 ret = 0;

	if (!s)
		goto err;

	clk = clk_get(NULL, (char *)s->data);
	if (IS_ERR_OR_NULL(clk))
		goto err;

	clk_enable(clk);
	ret = readl(s->reg) & s->mask;
	clk_disable(clk);

	clk_put(clk);

err:
	return ret;
}

/*****************************************************************************
 *                                   API                                     *
 *****************************************************************************/

/* Take the snapshot */
void snapshot_get(void)
{
	int i;

	if (is_snapshot_enabled == 0 || (!snapshot))
		return;

	for (i = 0; i < snapshot_len; i++) {
		struct snapshot *s = &snapshot[i];

		if (s->handler) {
			s->curr = s->handler(s);
			/* If the read value is different from the
			 * expected value, we get a mismatch (non-zero).
			 */
			s->curr ^= s->good;
		}
	}
}
EXPORT_SYMBOL(snapshot_get);

/*
 * Display only those snapshots which are responsible for the system
 * not entering into the low power state (non-zero 'actual' field)
 */
void snapshot_show(void)
{
	int i;

	if (is_snapshot_enabled == 0 || (!snapshot))
		return;

	for (i = 0; i < snapshot_len; i++) {
		struct snapshot *s = &snapshot[i];

		if (s->curr) {
			pr_info("[%s]: expected:0x%08x mismatch:0x%08x\n",
				s->name, s->good, s->curr);
		}
	}
}
EXPORT_SYMBOL(snapshot_show);

void snapshot_table_register(struct snapshot *table, size_t len)
{
	int i;

	snapshot = table;
	snapshot_len = len;

	for (i = 0; i < len ; i++) {
		switch (snapshot[i].type) {
		case SNAPSHOT_SIMPLE:
			snapshot[i].handler = handle_simple_parm;
			break;
		case SNAPSHOT_CLK:
			snapshot[i].handler = handle_clk_parm;
			break;
		case SNAPSHOT_AHB_REG:
			snapshot[i].handler = handle_ahb_reg_parm;
			break;
		default:
			snapshot[i].handler = NULL;
			break;
		}
	}
}
EXPORT_SYMBOL(snapshot_table_register);
