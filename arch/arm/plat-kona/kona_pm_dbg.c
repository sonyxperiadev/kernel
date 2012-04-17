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
#include <linux/list.h>
#include <linux/mutex.h>
#include <plat/clock.h>
#include <plat/kona_pm_dbg.h>

/* Snapshot handlers */
static u32 handle_simple_parm(struct snapshot *s);
static u32 handle_clk_parm(struct snapshot *s);
static u32 handle_ahb_reg_parm(struct snapshot *s);
static u32 handle_user_defined_parm(struct snapshot *s);
static void snapshot_add_reg(u32 reg, u32 mask, u32 good);
static void snapshot_del_reg(u32 reg);
static void snapshot_show_list(void);

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
	  "Take snapshot: s t\n"
	  "Show snapshot: s s\n"
	  "Add register to snapshot list: s a <reg> <mask> <good> (in hex)\n"
	  "Remove register from snapshot list: s r <reg>\n"
	  "Display snapshot list: s l\n"
	  "Display help: h\n"
	  "\n";

	pr_info("%s", usage);
}

static void handle_snapshot_list(const char *p)
{
	u32 reg = 0, mask = 0, good = 0;
	int cmd = *p;

	/* Skip past white spaces for reg address */
	p++;
	while (*p == ' ' || *p == '\t')
		p++;

	if (cmd == 'a') {
		sscanf(p, "%x %x %x", &reg, &mask, &good);
		snapshot_add_reg(reg, mask, good);
	} else if (cmd == 'r') {
		sscanf(p, "%x", &reg);
		snapshot_del_reg(reg);
	} else {
		snapshot_show_list();
	}
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
	case 'a': /* Fall through */
	case 'r':
	case 'l':
		handle_snapshot_list(p);
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
 *                            SNAPSHOT DATA                                  *
 *****************************************************************************/

/* Static snapshot table:
 * ---------------------
 * Table of registers to be sampled for debugging (for example before entering
 * low power state).
 */
static struct snapshot *snapshot;
static size_t snapshot_len;

/* Dynamic snapshot table:
 * ---------------------
 * Table of registers to be sampled that are added at runtime.
 */
static LIST_HEAD(snapshot_list);
static DEFINE_MUTEX(snapshot_list_lck);

/*****************************************************************************
 *                        DYNAMIC SNAPSHOT LIST HANDLING                     *
 *****************************************************************************/

static void snapshot_add_reg(u32 reg, u32 mask, u32 good)
{
	struct snapshot *s = NULL;

	pr_info("Adding: reg:0x%x mask:0x%x good:0x%x\n", reg, mask, good);

	mutex_lock(&snapshot_list_lck);
	list_for_each_entry(s, &snapshot_list, node) {
		if (s->reg == reg)
			break;
	}

	if (&s->node == &snapshot_list) { /* reg not already added */
		struct snapshot *new = kmalloc(sizeof(struct snapshot),
					       GFP_KERNEL);
		if (new) {
			new->data = NULL;
			new->type = SNAPSHOT_SIMPLE;
			new->reg = reg;
			new->mask = mask;
			new->good = good;
			new->name = NULL;
			new->handler = handle_simple_parm;
			list_add(&new->node, &snapshot_list);
		} else {
			pr_err("%s: out of memory\n", __func__);
		}
	}
	mutex_unlock(&snapshot_list_lck);
}

static void snapshot_del_reg(u32 reg)
{
	struct snapshot *s = NULL;
	pr_info("Removing: 0x%x\n", reg);

	mutex_lock(&snapshot_list_lck);
	list_for_each_entry(s, &snapshot_list, node) {
		if (s->reg == reg) {
			list_del(&s->node);
			kfree(s);
			break;
		}
	}
	mutex_unlock(&snapshot_list_lck);
}

static void snapshot_show_list(void)
{
	struct snapshot *s = NULL;
	int i;

	for (i = 0; i < snapshot_len; i++)
		pr_info("0x%08x\n", snapshot[i].reg);

	mutex_lock(&snapshot_list_lck);
	list_for_each_entry(s, &snapshot_list, node)
		pr_info("0x%08x\n", s->reg);
	mutex_unlock(&snapshot_list_lck);
}

/*****************************************************************************
 *                             SNAPSHOT HANDLERS                             *
 *****************************************************************************/

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

/* Call registered function to get data */
static u32 handle_user_defined_parm(struct snapshot *s)
{
	u32 ret = 0;

	if (s->func)
		ret = s->func(s->data);

	return ret;
}

/*****************************************************************************
 *                                   API                                     *
 *****************************************************************************/

/* Take the snapshot */
void snapshot_get(void)
{
	struct snapshot *s;
	int i;

	if (is_snapshot_enabled == 0)
		return;

	if (!snapshot)
		snapshot_len = 0;

	for (i = 0; i < snapshot_len; i++) {
		s = &snapshot[i];

		if (s->handler)
			s->curr = s->handler(s);
	}

	list_for_each_entry(s, &snapshot_list, node) {
		if (s->handler)
			s->curr = s->handler(s);
	}
}
EXPORT_SYMBOL(snapshot_get);

/*
 * Display only those snapshots which are responsible for the system
 * not entering into the low power state (non-zero 'actual' field)
 */
void snapshot_show(void)
{
	struct snapshot *s;
	int i;

	if (is_snapshot_enabled == 0)
		return;

	if (!snapshot)
		snapshot_len = 0;

	for (i = 0; i < snapshot_len; i++) {
		s = &snapshot[i];

		if (s->curr != s->good)
			pr_info("[%s]: expected:0x%08x mismatch:0x%08x\n",
				s->name, s->good, s->curr);
	}

	list_for_each_entry(s, &snapshot_list, node) {
		if (s->curr != s->good)
			pr_info("[%d]: expected:0x%08x mismatch:0x%08x\n",
				s->reg, s->good, s->curr);
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
		case SNAPSHOT_USER_DEFINED:
			snapshot[i].handler = handle_user_defined_parm;
			break;
		default:
			snapshot[i].handler = NULL;
			break;
		}
	}
}
EXPORT_SYMBOL(snapshot_table_register);
