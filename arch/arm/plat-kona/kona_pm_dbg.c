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
#include <linux/slab.h>
#include <plat/kona_pm_dbg.h>
#include <linux/dma-mapping.h>

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#endif

#ifdef CONFIG_PM_LOG_TO_UNCACHED_MEM
/* PM log buf pointer */
static char *pm_log_buf;
static dma_addr_t pm_log_buf_p;
#define ENABLE		1
#define DISABLE		0
#define PM_LOG_BUF_MASK		(PM_LOG_BUF_SIZE - 1)
#define PM_LOG_BUF(idx)		(pm_log_buf[(idx) & PM_LOG_BUF_MASK])
#define CIRC_INC(idx)		(idx = ((idx+1) & PM_LOG_BUF_MASK))
#define CIRC_CHK(idx1, idx2)	(idx2 == ((idx1+1) & PM_LOG_BUF_MASK))
#define emit_pm_char(c)		(PM_LOG_BUF(log_end) = (c) ,\
				(CIRC_INC(log_end) == log_start) ? \
				CIRC_INC(log_start) : ++logged_chars)

/* Order is important, based on enum in pi_mgr.h */
static const char *pi_name_list[6] = {
	"mm",
	"hub",
	"aon",
	"arm_core",
	"sub_sys",
	"modem"
};

int dbg_en[DBG_MSG_MAX] = {
	ENABLE,		/* DBG_MSG_PI_ENABLE */
	ENABLE,		/* DBG_MSG_PI_SET_FREQ_POLICY */
	ENABLE,		/* DBG_MSG_PI_SET_FREQ_OPP */
	ENABLE,		/* DBG_MSG_PM_LPM_ENTER */
	ENABLE		/* DBG_MSG_PM_LPM_EXIT */
};

int timestamp_en = ENABLE;
static unsigned log_start;
static unsigned log_end;
static unsigned logged_chars;
static DEFINE_SPINLOCK(pm_logbuf_lock);
#endif

/* Snapshot handlers */
static u32 handle_simple_parm(struct snapshot *s);
static u32 handle_ahb_reg_parm(struct snapshot *s);
static u32 handle_user_defined_parm(struct snapshot *s);
static void snapshot_add_reg(void __iomem *reg, u32 mask, u32 good);
static void snapshot_del_reg(void __iomem *reg);
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
	CMD_PRINT_LOG = 'p',
	CMD_SHOW_HELP = 'h',
};

static void cmd_show_usage(void)
{
	const char usage[] = "Usage:\n"
	  "echo 'cmd string' > /sys/module/kona_pm_dbg/parameters/debug\n"
	  "'cmd string' is constructed as follows:\n"
	  "Enable snapshot: s e\n"
	  "Disable snapshot: s d\n"
	  "Take snapshot: s t\n"
	  "Show snapshot: s s\n"
	  "Print log: p\n"
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
		/* coverity[secure_coding] */
		sscanf(p, "%x %x %x", &reg, &mask, &good);
		snapshot_add_reg((void __iomem *) reg, mask, good);
	} else if (cmd == 'r') {
		/* coverity[secure_coding] */
		sscanf(p, "%x", &reg);
		snapshot_del_reg((void __iomem *) reg);
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
	case CMD_PRINT_LOG:
		print_pm_log();
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

static void snapshot_add_reg(void __iomem *reg, u32 mask, u32 good)
{
	struct snapshot *s = NULL;

	pr_info("Adding: reg:0x%x mask:0x%x good:0x%x\n",
					(u32) reg, mask, good);

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

static void snapshot_del_reg(void __iomem *reg)
{
	struct snapshot *s = NULL;
	pr_info("Removing: 0x%x\n", (u32) reg);

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
		pr_info("0x%08x\n", (u32) snapshot[i].reg);

	mutex_lock(&snapshot_list_lck);
	list_for_each_entry(s, &snapshot_list, node)
		pr_info("0x%08x\n", (u32) s->reg);
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

/* Returns the current masked value of the specified register */
static u32 handle_ahb_reg_parm(struct snapshot *s)
{
	struct clk *clk;
	u32 ret = 0;

	if (!s)
		goto err;

	clk = clk_get(NULL, (char *)s->data);
	if (IS_ERR(clk))
		goto err;

	if (clk_enable(clk))
		goto err;
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
			pr_info("[%x]: expected:0x%08x mismatch:0x%08x\n",
				(u32) s->reg, s->good, s->curr);
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

#ifdef CONFIG_PM_LOG_TO_UNCACHED_MEM
/*****************************************************************************
 *                     PM logging to uncached memory                         *
 *****************************************************************************/


int print_pm_log(void)
{
	u32 idx, pi_id, val1, val2, dbg_flag, i;
	u32 sz = PM_LOG_BUF_SIZE;
	unsigned long flags;
	int ret = RET_SUCCESS;
	char timestamp[50];

	spin_lock_irqsave(&pm_logbuf_lock, flags);

	pr_info("************************** PM LOG BUFFER ************************** (size:%d start:%d end:%d)\n",
		sz, log_start, log_end);

	idx = log_start;

	/* Buffer is circular so if buffer is full, then first entry could be
	 * a partially overwritten one. To begin printing with a valid entry,
	 * initial few bytes until the first '\n' are ommited.
	 */
	if (CIRC_CHK(log_end, log_start)) {
		pr_info("Partial entry : Omitting bytes - ");
		while (pm_log_buf[idx] != '\n') {
			pr_info("%d ", pm_log_buf[idx]);
			CIRC_INC(idx);
		}
	pr_info("\n");
	}
	/* If buffer is not full, then first byte present at log_start is a
	 * valid dbg_flag. We can set the unused last byte of the buffer as
	 * new line and index it to indicate valid dbg_flag at start.
	 */
	else
		pm_log_buf[idx = (sz-1)] = '\n';

	while (!CIRC_CHK(idx, log_end)) {
		BUG_ON(pm_log_buf[idx] != '\n');
		if (timestamp_en) {
			i = -1;
			while (pm_log_buf[idx] != ']')
				timestamp[++i] = pm_log_buf[CIRC_INC(idx)];
			timestamp[++i] = '\0';
		}
		CIRC_INC(idx);
		dbg_flag = (int)pm_log_buf[idx];
		switch (dbg_flag) {

		case DBG_MSG_PI_ENABLE:
			pi_id = (int)pm_log_buf[CIRC_INC(idx)];
			val1 = (int)pm_log_buf[CIRC_INC(idx)];
			if (val1) {
				pr_info("%s Enable PI : pi_id=%d (%s)\n",
					timestamp, pi_id, pi_name_list[pi_id]);
			}

			else {
				pr_info("%s Disable PI : pi_id=%d (%s)\n",
					timestamp, pi_id, pi_name_list[pi_id]);
			}
			CIRC_INC(idx);
				break;

		case DBG_MSG_PI_SET_FREQ_POLICY:
			pi_id = (int)pm_log_buf[CIRC_INC(idx)];
			val1 = (int)pm_log_buf[CIRC_INC(idx)];
			val2 = (int)pm_log_buf[CIRC_INC(idx)];
			pr_info("%s Policy Change (%d -> %d) : pi_id=%d (%s)\n",
					timestamp, val1, val2, pi_id,
						pi_name_list[pi_id]);
			CIRC_INC(idx);
			break;

		case DBG_MSG_PI_SET_FREQ_OPP:
			pi_id = (int)pm_log_buf[CIRC_INC(idx)];
			val1 = (int)pm_log_buf[CIRC_INC(idx)];
			val2 = (int)pm_log_buf[CIRC_INC(idx)];
			pr_info("%s OPP Change, New_OPP=%d, freq_id=%d : pi_id=%d (%s)\n",
					timestamp, val1, val2, pi_id,
						pi_name_list[pi_id]);
			CIRC_INC(idx);
			break;

		case DBG_MSG_PM_LPM_ENTER:
			pi_id = (int)pm_log_buf[CIRC_INC(idx)];
			val1 = (int)pm_log_buf[CIRC_INC(idx)];
			pr_info("%s Enter Low-Power, state=%d : pi_id=%d (%s)\n",
					timestamp, val1, pi_id,
						pi_name_list[pi_id]);
			CIRC_INC(idx);
			break;

		case DBG_MSG_PM_LPM_EXIT:
			pi_id = (int)pm_log_buf[CIRC_INC(idx)];
			val1 = (int)pm_log_buf[CIRC_INC(idx)];
			pr_info("%s Exit Low-Power, state=%d : pi_id=%d (%s)\n",
					timestamp, val1, pi_id,
						pi_name_list[pi_id]);
			CIRC_INC(idx);
			break;

		default:
			pr_info("%s Unknown debug flag %d : Omitting bytes - ",
					timestamp, dbg_flag);
			while (!CIRC_CHK(idx, log_end)
				&& pm_log_buf[idx] != '\n') {
				pr_info("%d ", pm_log_buf[idx]);
				CIRC_INC(idx);
			}
			ret = RET_PARTIAL_SUCCESS;
			break;
		}
	}
	pr_info("************************ END OF LOG BUFFER ************************\n");
	spin_unlock_irqrestore(&pm_logbuf_lock, flags);
	return ret;
}
EXPORT_SYMBOL(print_pm_log);

int log_pm(int num, ...)
{
	va_list args;
	unsigned long long t;
	unsigned long nanosec_rem, flags;
	char tbuf[50], *tp;
	int byte, i, bytes = 0, this_cpu;
	unsigned tlen;

	if (num > MAX_PM_LOGGER_ARGS)
		return bytes;

	va_start(args, num);
	byte = va_arg(args, int);
	if (byte < 0 || byte >= DBG_MSG_MAX || !dbg_en[byte]) {
		va_end(args);
		return bytes;
	}
	this_cpu = smp_processor_id();
	spin_lock_irqsave(&pm_logbuf_lock, flags);
	if (timestamp_en) {
		/* Add the current time stamp */
		t = cpu_clock(this_cpu);
		nanosec_rem = do_div(t, 1000000000);
		tlen = snprintf(tbuf, 20, "[%5lu.%06lu]", (unsigned long) t,
			nanosec_rem / 1000);
		for (tp = tbuf; tp < tbuf + tlen; tp++)
			emit_pm_char(*tp);
		bytes += tlen;
	}

	/* Store the debug msg byte */
	emit_pm_char((char)byte);
	bytes++;
	/* Store rest of the parameter bytes from the arguments */
	for (i = 1; i < num; i++) {
		byte = va_arg(args, int);
		emit_pm_char((char)byte);
		bytes++;
	}
	va_end(args);

	emit_pm_char('\n');
	bytes++;
	spin_unlock_irqrestore(&pm_logbuf_lock, flags);
	return bytes;
}
EXPORT_SYMBOL(log_pm);

#ifdef CONFIG_DEBUG_FS
static struct dentry *dent_uncached_logging;
int __init uncached_logging_debug_init(void)
{
	/* create uncached logging dir /log */
	dent_uncached_logging = debugfs_create_dir("log", 0);
	if (!dent_uncached_logging)
		return -ENOMEM;
	printk(KERN_CRIT "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& Successful &&&&&&&&&");

	if (!debugfs_create_u32("enable", S_IRUGO | S_IWUSR,
				dent_uncached_logging, (int *)&dbg_en[0]))
		return -ENOMEM;

	if (!debugfs_create_u32("policy_switch", S_IRUGO | S_IWUSR,
				dent_uncached_logging, (int *)&dbg_en[1]))
		return -ENOMEM;

	if (!debugfs_create_u32("opp_switch", S_IRUGO | S_IWUSR,
				dent_uncached_logging, (int *)&dbg_en[2]))
		return -ENOMEM;

	if (!debugfs_create_u32("lpm_enter", S_IRUGO | S_IWUSR,
				dent_uncached_logging, (int *)&dbg_en[3]))
		return -ENOMEM;

	if (!debugfs_create_u32("lpm_exit", S_IRUGO | S_IWUSR,
				dent_uncached_logging, (int *)&dbg_en[4]))
		return -ENOMEM;

	if (!debugfs_create_u32("timestamp", S_IRUGO | S_IWUSR,
				dent_uncached_logging, (int *)&timestamp_en))
		return -ENOMEM;
	return 0;
}
#endif /* CONFIG_DEBUG_FS */

int __init kona_pmdbg_init(void)
{
	pm_log_buf = dma_alloc_coherent(NULL, PM_LOG_BUF_SIZE,
					&pm_log_buf_p, GFP_ATOMIC);
	if (pm_log_buf == NULL) {
		pr_info("%s: PM dbg buffer alloc failed\n", __func__);
		return -ENOMEM;
	}

	pr_info("pm_log_buf v:0x%x, p:0x%x\n", (u32)pm_log_buf,
			(u32)&pm_log_buf_p);
#ifdef CONFIG_DEBUG_FS
	uncached_logging_debug_init();
#endif
	return 0;
}

arch_initcall(kona_pmdbg_init);
#else
int log_pm(int num, ...)
{
return 0;
}

int print_pm_log(void)
{
return 0;
}
#endif /* CONFIG_PM_LOG_TO_UNCACHED_MEM */
