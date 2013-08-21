#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/clk.h>
#include <linux/debugfs.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/seq_file.h>
#include <plat/clock.h>
#include <plat/pi_mgr.h>

__weak int debug_bus_mux_sel(int mux_sel, int mux_param, u32 dbg_bit_sel)
{
	return 0;
}

__weak int set_clk_idle_debug_mon(int clk_idle, int db_sel, u32 dbg_bit_sel)
{
	return 0;
}

__weak int clk_mon_dbg(struct clk *clock, int path, int clk_sel, int clk_ctl,
		u32 dbg_bit_sel)
{
	return 0;
}

__weak int set_ccu_dbg_bus_mux(struct ccu_clk *ccu_clk, int mux_sel,
			int mux_param, u32 dbg_bit_sel)
{
	return 0;
}

__weak int set_misc_dbg_bus(int sel, u32 dbg_bit_sel)
{
	return 0;
}

__weak u32 get_misc_dbg_bus(void)
{
	return 0;
}

static int ccu_policy_dbg_get_act_freqid(struct ccu_clk *ccu_clk)
{
	u32 reg_val;

	reg_val = readl(ccu_clk->ccu_clk_mgr_base + ccu_clk->policy_dbg_offset);
	reg_val =
	    (reg_val >> ccu_clk->
	     policy_dbg_act_freq_shift) & CCU_POLICY_DBG_FREQ_MASK;

	return (int)reg_val;
}

static int clk_debugfs_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t set_clk_idle_debug(struct file *file, char const __user *buf,
				  size_t count, loff_t *offset)
{
	u32 len = 0;
	int db_sel = 0;
	int clk_idle = 0;
	char input_str[100];
	u32 dbg_bit_sel = 0;

	if (count > sizeof(input_str))
		len = sizeof(input_str);
	else
		len = count;

	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	/* coverity[secure_coding] */
	sscanf(input_str, "%x%d%x", &clk_idle, &db_sel, &dbg_bit_sel);
	set_clk_idle_debug_mon(clk_idle, db_sel, dbg_bit_sel);
	return count;
}

static const struct file_operations clock_idle_debug_fops = {
	.open = clk_debugfs_open,
	.write = set_clk_idle_debug,
};

static ssize_t set_clk_mon_dbg(struct file *file, char const __user *buf,
				 size_t count, loff_t *offset)
{
	struct clk *clock = file->private_data;
	u32 len = 0;
	int path = 0;
	int clk_sel = 0;
	int clk_ctl = 0;
	char input_str[100];
	u32 dbg_bit_sel = 0;
	BUG_ON(clock == NULL);
	if (count > 100)
		len = 100;
	else
		len = count;
	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	/* coverity[secure_coding] */
	sscanf(input_str, "%d%x%d%x", &path, &clk_sel, &clk_ctl, &dbg_bit_sel);
	clk_mon_dbg(clock, path, clk_sel, clk_ctl, dbg_bit_sel);
	return count;
}

static const struct file_operations clk_mon_fops = {
	.open = clk_debugfs_open,
	.write = set_clk_mon_dbg,
};

static int clk_debug_get_rate(void *data, u64 *val)
{
	struct clk *clock = data;
	*val = clk_get_rate(clock);
	return 0;
}

static int clk_debug_set_rate(void *data, u64 val)
{
	struct clk *clock = data;
	int ret;
	ret = clk_set_rate(clock, val);
	return ret;
}

DEFINE_SIMPLE_ATTRIBUTE(clock_rate_fops, clk_debug_get_rate,
			clk_debug_set_rate, "%llu\n");


static int pll_des_dbg_get_rate(void *data, u64 *val)
{
	struct clk *clk = data;
	*val = pll_get_desense_offset(clk);
	return 0;
}

static int pll_des_dbg_set_rate(void *data, u64 val)
{
	struct clk *clk = data;
	int ret;
	ret = pll_set_desense_offset(clk, (int)val);
	return ret;
}

DEFINE_SIMPLE_ATTRIBUTE(pll_desense_rate_fops, pll_des_dbg_get_rate,
			pll_des_dbg_set_rate, "%lli\n");


static int ccu_debug_get_freqid(void *data, u64 *val)
{
	struct clk *clock = data;
	struct ccu_clk *ccu_clk;
	int freq_id;

	ccu_clk = to_ccu_clk(clock);
	freq_id = ccu_policy_dbg_get_act_freqid(ccu_clk);

	*val = freq_id;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(ccu_freqid_fops, ccu_debug_get_freqid, NULL, "%llu\n");

static int ccu_debug_wr_en_get(void *data, u64 *val)
{
	struct clk *clock = data;
	struct ccu_clk *ccu_clk;
	BUG_ON(!clock);
	ccu_clk = to_ccu_clk(clock);
	*val = ccu_clk->write_access_en_count;
	return 0;
}

static int ccu_debug_wr_en_set(void *data, u64 val)
{
	struct clk *clock = data;
	struct ccu_clk *ccu_clk;

	BUG_ON(!clock);
	ccu_clk = to_ccu_clk(clock);

	if (val)
		ccu_write_access_enable(ccu_clk, true);
	else
		ccu_write_access_enable(ccu_clk, false);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(ccu_wr_en_fops, ccu_debug_wr_en_get, ccu_debug_wr_en_set
		, "%llu\n");




static ssize_t ccu_debug_get_dbg_bus_status(struct file *file,
					char __user *user_buf,
				size_t count, loff_t *ppos)
{
	u32 len = 0;
	struct ccu_clk *ccu_clk;
	char out_str[20];
	int status;
	struct clk *clk = file->private_data;

	BUG_ON(clk == NULL);
	ccu_clk = to_ccu_clk(clk);
	memset(out_str, 0, sizeof(out_str));
	CCU_ACCESS_EN(ccu_clk, 1);
	status = ccu_get_dbg_bus_status(ccu_clk);
	CCU_ACCESS_EN(ccu_clk, 0);
	if (status == -EINVAL)
		len += snprintf(out_str+len, sizeof(out_str)-len,
			"error!!\n");
	else
		len += snprintf(out_str+len, sizeof(out_str)-len,
			"%x\n", (u32)status);

	return simple_read_from_buffer(user_buf, count, ppos,
			out_str, len);
}

static ssize_t ccu_debug_set_dbg_bus_sel(struct file *file,
				  char const __user *buf, size_t count,
				  loff_t *offset)
{
	struct clk *clk = file->private_data;
	struct ccu_clk *ccu_clk;
	u32 len = 0;
	char input_str[10];
	u32 sel = 0, mux = 0, mux_parm = 0;
	u32 dbg_bit_sel = 0;

	BUG_ON(clk == NULL);
	ccu_clk = to_ccu_clk(clk);
	BUG_ON(ccu_clk == NULL);
	memset(input_str, 0, ARRAY_SIZE(input_str));
	if (count > ARRAY_SIZE(input_str))
		len = ARRAY_SIZE(input_str);
	else
		len = count;

	if (copy_from_user(input_str, buf, len))
		return -EFAULT;

	/* coverity[secure_coding] */
	sscanf(&input_str[0], "%x%x%x%x", &sel, &mux, &mux_parm, &dbg_bit_sel);
	set_ccu_dbg_bus_mux(ccu_clk, mux, mux_parm, dbg_bit_sel);
	CCU_ACCESS_EN(ccu_clk, 1);
	ccu_set_dbg_bus_sel(ccu_clk, sel);
	CCU_ACCESS_EN(ccu_clk, 0);
	return count;
}

static const struct file_operations ccu_dbg_bus_fops = {
	.open = clk_debugfs_open,
	.write = ccu_debug_set_dbg_bus_sel,
	.read = ccu_debug_get_dbg_bus_status,
};
static int ccu_debug_get_policy(void *data, u64 *val)
{
	struct clk *clock = data;
	struct ccu_clk *ccu_clk;
	int policy;

	ccu_clk = to_ccu_clk(clock);
	policy = ccu_policy_dbg_get_act_policy(ccu_clk);

	*val = policy;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(ccu_policy_fops, ccu_debug_get_policy, NULL, "%llu\n");

static int clock_status_show(struct seq_file *seq, void *p)
{
	struct clk *c = seq->private;
	struct peri_clk *peri_clk;
	struct bus_clk *bus_clk;
	struct ref_clk *ref_clk;
	struct pll_clk *pll_clk;
	struct pll_chnl_clk *pll_chnl_clk;
	struct core_clk *core_clk;
	int enabled = 0;

	switch (c->clk_type) {
	case CLK_TYPE_PERI:
		peri_clk = to_peri_clk(c);
		enabled = peri_clk_get_gating_status(peri_clk);
		break;
	case CLK_TYPE_BUS:
		bus_clk = to_bus_clk(c);
		enabled = bus_clk_get_gating_status(bus_clk);
		break;
	case CLK_TYPE_REF:
		ref_clk = to_ref_clk(c);
		enabled = ref_clk_get_gating_status(ref_clk);
		break;
	case CLK_TYPE_PLL:
		pll_clk = to_pll_clk(c);
		enabled = pll_clk_get_lock_status(pll_clk);
		break;
	case CLK_TYPE_PLL_CHNL:
		pll_chnl_clk = to_pll_chnl_clk(c);
		/*0= divider outputs enabled, 1= divider outputs disabled
		 * So inverting to display status.
		 */
		enabled = !pll_chnl_clk_get_enb_clkout(pll_chnl_clk);
		break;
	case CLK_TYPE_CORE:
		core_clk = to_core_clk(c);
		enabled = core_clk_get_gating_status(core_clk);
		break;
	case CLK_TYPE_CCU:
		if (c->use_cnt > 0)
			enabled = 1;
		break;
	default:
		enabled = -1;
	}
	if (enabled < 0)
		seq_printf(seq, "-1\n");
	else
		seq_printf(seq, "%d\n", enabled);
	return 0;
}

static int fops_clock_status_show_open(struct inode *inode, struct file *file)
{
	return single_open(file, clock_status_show, inode->i_private);
}

static const struct file_operations clock_status_show_fops = {
	.open = fops_clock_status_show_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int clk_debug_reset(void *data, u64 val)
{
	struct clk *clock = data;
	if (val == 1)		/*reset and release the clock from reset */
		clk_reset(clock);
	else
		clk_dbg("Invalid value\n");

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(clock_reset_fops, NULL, clk_debug_reset, "%llu\n");

static int clk_debug_set_enable(void *data, u64 val)
{
	struct clk *clock = data;
	if (val == 1)
		clk_enable(clock);
	else if (val == 0)
		clk_disable(clock);
	else
		clk_dbg("Invalid value\n");

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(clock_enable_fops, NULL, clk_debug_set_enable,
			"%llu\n");

static int print_ref_clock_params(struct seq_file *seq, struct clk *temp_clk)
{
	int status, auto_gate, enable_bit;
	struct ref_clk *ref_clk;
	ref_clk = to_ref_clk(temp_clk);
	status = ref_clk_get_gating_status(ref_clk);
	if (status < 0)
		status = -1;
	auto_gate = ref_clk_get_gating_ctrl(ref_clk);
	if (auto_gate < 0)
		auto_gate = -1;
	enable_bit = ref_clk_get_enable_bit(ref_clk);
	if (enable_bit < 0)
		enable_bit = -1;
	seq_printf(seq, "Ref clock:%20s\t\tenable_bit:%d\t\tStatus:%d\t\t"\
		" use count:%d\t\tGating:%d\n", temp_clk->name, enable_bit,
		status, temp_clk->use_cnt, auto_gate);

	return 0;
}

static int print_peri_clock_params(struct seq_file *seq, struct clk *temp_clk)
{
	int status, auto_gate, sleep_prev, enable_bit;
	struct peri_clk *peri_clk;

	peri_clk = to_peri_clk(temp_clk);
	status = peri_clk_get_gating_status(peri_clk);
	if (status < 0)
		status = -1;
	auto_gate = peri_clk_get_gating_ctrl(peri_clk);
	if (auto_gate < 0)
		auto_gate = -1;
	enable_bit = peri_clk_get_enable_bit(peri_clk);
	if (enable_bit < 0)
		enable_bit = -1;
	sleep_prev = !CLK_FLG_ENABLED(temp_clk, DONOT_NOTIFY_STATUS_TO_CCU);
	seq_printf(seq, "Peri clock:%20s\t\tenable_bit:%d\t\tStatus:%d" \
		" use count:%d\t\tGating:%d\t\tcan_prevent_retn:%s\n",
		temp_clk->name, enable_bit, status, temp_clk->use_cnt,
		auto_gate, sleep_prev ? "YES" : "NO");

	return 0;
}

static int print_bus_clock_params(struct seq_file *seq, struct clk *temp_clk)
{
	int status, auto_gate, sleep_prev, enable_bit;
	struct bus_clk *bus_clk;

	bus_clk = to_bus_clk(temp_clk);
	status = bus_clk_get_gating_status(bus_clk);
	if (status < 0)
		status = -1;
	CCU_ACCESS_EN(bus_clk->ccu_clk, 1);
	auto_gate = bus_clk_get_gating_ctrl(bus_clk);
	if (auto_gate < 0)
		auto_gate = -1;
	enable_bit = bus_clk_get_enable_bit(bus_clk);
	if (enable_bit < 0)
		enable_bit = -1;
	CCU_ACCESS_EN(bus_clk->ccu_clk, 0);
	sleep_prev = CLK_FLG_ENABLED(temp_clk, NOTIFY_STATUS_TO_CCU)
	    && !CLK_FLG_ENABLED(temp_clk, AUTO_GATE);
	seq_printf(seq, "Bus clock:%20s\t\tenable_bit:%d\t\tStatus:%d "\
		"use count:%d\t\tGating:%d\t\tcan_prevent_retn:%s\n",
		temp_clk->name, enable_bit, status, temp_clk->use_cnt,
		auto_gate, sleep_prev ? "YES" : "NO");

	return 0;

}

static int print_pll_clock_params(struct seq_file *seq, struct clk *temp_clk)
{
	int lock, pdiv, ndiv_int, ndiv_frac, idle_pwrdwn_sw_ovrrid, pll_pwrdwn;
	struct pll_clk *pll_clk;

	pll_clk = to_pll_clk(temp_clk);
	lock = pll_clk_get_lock_status(pll_clk);
	if (lock < 0)
		lock = -1;
	pdiv = pll_clk_get_pdiv(pll_clk);
	if (pdiv < 0)
		pdiv = -1;
	ndiv_int = pll_clk_get_ndiv_int(pll_clk);
	if (ndiv_int < 0)
		ndiv_int = -1;
	ndiv_frac = pll_clk_get_ndiv_frac(pll_clk);
	if (ndiv_frac < 0)
		ndiv_frac = -1;
	idle_pwrdwn_sw_ovrrid = pll_clk_get_idle_pwrdwn_sw_ovrride(pll_clk);
	if (idle_pwrdwn_sw_ovrrid < 0)
		idle_pwrdwn_sw_ovrrid = -1;
	pll_pwrdwn = pll_clk_get_pwrdwn(pll_clk);
	if (pll_pwrdwn < 0)
		pll_pwrdwn = -1;

	seq_printf(seq, "PLL clock:%20s\t\tLock:%d\t\tpdiv:%x\t\t "\
		"ndiv_int:%x\t\tndiv_frac:%x\t\tidle_pwrdwn_sw_ovrrid:%d\t"\
		"pwr_dwn:%d\n", temp_clk->name, lock, pdiv, ndiv_int,
		ndiv_frac, idle_pwrdwn_sw_ovrrid, pll_pwrdwn);

	return 0;
}

static int print_pll_chnl_clock_params(struct seq_file *seq,
				       struct clk *temp_clk)
{
	int mdiv, out_enable;
	struct pll_chnl_clk *pll_chnl_clk;

	pll_chnl_clk = to_pll_chnl_clk(temp_clk);
	mdiv = pll_chnl_clk_get_mdiv(pll_chnl_clk);
	if (mdiv < 0)
		mdiv = -1;
	out_enable = pll_chnl_clk_get_enb_clkout(pll_chnl_clk);
	if (out_enable < 0)
		out_enable = -1;

	seq_printf(seq,
		   "PLL_chnl clock:%20s\t\tmdiv:%x\t\tclkout_enable:%d\t\t\n",
		   temp_clk->name, mdiv, out_enable);

	return 0;

}

static int print_core_clock_params(struct seq_file *seq, struct clk *temp_clk)
{
	int status, auto_gate, enable_bit;
	struct core_clk *core_clk;

	core_clk = to_core_clk(temp_clk);
	status = core_clk_get_gating_status(core_clk);
	if (status < 0)
		status = -1;
	auto_gate = core_clk_get_gating_ctrl(core_clk);
	if (auto_gate < 0)
		auto_gate = -1;
	enable_bit = core_clk_get_enable_bit(core_clk);
	if (enable_bit < 0)
		enable_bit = -1;

	seq_printf(seq, "core clock:%20s\t\tenable_bit:%d\t\t "\
		"Status:%d\t\tGating:%d\t\t\n", temp_clk->name,
		enable_bit, status, auto_gate);

	return 0;
}

static int ccu_clock_list_show(struct seq_file *seq, void *p)
{
	struct clk *clock = seq->private;
	struct ccu_clk *ccu_clk;
	struct clk *temp_clk;

	ccu_clk = to_ccu_clk(clock);
	list_for_each_entry(temp_clk, &ccu_clk->clk_list, list) {
		switch (temp_clk->clk_type) {
		case CLK_TYPE_REF:
			print_ref_clock_params(seq, temp_clk);
			break;

		case CLK_TYPE_PERI:
			print_peri_clock_params(seq, temp_clk);
			break;
		case CLK_TYPE_BUS:
			print_bus_clock_params(seq, temp_clk);
			break;
		case CLK_TYPE_PLL:
			print_pll_clock_params(seq, temp_clk);
			break;
		case CLK_TYPE_PLL_CHNL:
			print_pll_chnl_clock_params(seq, temp_clk);
			break;
		case CLK_TYPE_CORE:
			print_core_clock_params(seq, temp_clk);
			break;
		default:
			seq_printf(seq,
				   "clock:%20s\t\tuse count:%d\t\tGating:%d\n",
				   temp_clk->name, temp_clk->use_cnt,
				   CLK_FLG_ENABLED(temp_clk, AUTO_GATE));
			break;
		}
	}

	return 0;
}

static int fops_ccu_clock_list_open(struct inode *inode, struct file *file)
{
	return single_open(file, ccu_clock_list_show, inode->i_private);
}

static const struct file_operations ccu_clock_list_fops = {
	.open = fops_ccu_clock_list_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int clk_parent_show(struct seq_file *seq, void *p)
{
	struct clk *clock = seq->private;
	struct peri_clk *peri_clk;
	struct bus_clk *bus_clk;
	struct ref_clk *ref_clk;
	struct pll_chnl_clk *pll_chnl_clk;
	struct core_clk *core_clk;
	switch (clock->clk_type) {
	case CLK_TYPE_PERI:
		peri_clk = to_peri_clk(clock);
		seq_printf(seq, "name   -- %s\n", clock->name);
		if ((peri_clk->src.count > 0)
		    && (peri_clk->src.src_inx < peri_clk->src.count))
			seq_printf(seq, "parent -- %s\n",
				   peri_clk->src.list[peri_clk->src.
							 src_inx].clk->name);
		else
			seq_printf(seq, "parent -- NULL\n");
		break;
	case CLK_TYPE_BUS:
		bus_clk = to_bus_clk(clock);
		seq_printf(seq, "name   -- %s\n", clock->name);
		if (bus_clk->freq_tbl_index < 0 && bus_clk->src_clk)
			seq_printf(seq, "parent -- %s\n",
				   bus_clk->src_clk->name);
		else
			seq_printf(seq, "parent derived from internal bus\n");
		break;
	case CLK_TYPE_REF:
		ref_clk = to_ref_clk(clock);
		seq_printf(seq, "name   -- %s\n", clock->name);
		if ((ref_clk->src_clk.count > 0)
		    && (ref_clk->src_clk.src_inx < ref_clk->src_clk.count))
			seq_printf(seq, "parent -- %s\n",
				   ref_clk->src_clk.list[ref_clk->src_clk.
							src_inx].clk->name);
		else
			seq_printf(seq, "Derived from %s ccu\n",
				   ref_clk->ccu_clk->clk.name);
		break;
	case CLK_TYPE_PLL:
		seq_printf(seq, "PLL:  %s\n", clock->name);
		break;

	case CLK_TYPE_PLL_CHNL:
		pll_chnl_clk = to_pll_chnl_clk(clock);
		seq_printf(seq, "PLL:  %s; PLL channel:%s\n",
			   pll_chnl_clk->pll_clk->clk.name, clock->name);
		break;
	case CLK_TYPE_CORE:
		core_clk = to_core_clk(clock);
		seq_printf(seq, "PLL:  %s; core_clk:%s\n",
			   core_clk->pll_clk->clk.name, clock->name);
		break;

	default:
		return -EINVAL;
	}
	return 0;
}

static int fops_parent_open(struct inode *inode, struct file *file)
{
	return single_open(file, clk_parent_show, inode->i_private);
}

static const struct file_operations clock_parent_fops = {
	.open = fops_parent_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int clk_source_show(struct seq_file *seq, void *p)
{
	struct clk *clock = seq->private;
	struct peri_clk *peri_clk;
	struct bus_clk *bus_clk;
	struct ref_clk *ref_clk;
	struct pll_chnl_clk *pll_chnl_clk;
	struct core_clk *core_clk;
	switch (clock->clk_type) {
	case CLK_TYPE_PERI:
		peri_clk = to_peri_clk(clock);
		if (peri_clk->src.count > 0) {
			int i;
			seq_printf(seq, "clock source for %s\n", clock->name);
			for (i = 0; i < peri_clk->src.count; i++) {
				seq_printf(seq, "%d   %s\n", i,
					peri_clk->src.list[i].clk->name);
			}
		} else
			seq_printf(seq, "no source for %s\n", clock->name);
		break;
	case CLK_TYPE_BUS:
		bus_clk = to_bus_clk(clock);
		if (bus_clk->freq_tbl_index < 0 && bus_clk->src_clk)
			seq_printf(seq, "source for %s is %s\n", clock->name,
				   bus_clk->src_clk->name);
		else
			seq_printf(seq, "%s derived from %s CCU\n", clock->name,
				   bus_clk->ccu_clk->clk.name);
		break;
	case CLK_TYPE_REF:
		ref_clk = to_ref_clk(clock);
		if ((ref_clk->src_clk.count > 0)
		    && (ref_clk->src_clk.src_inx < ref_clk->src_clk.count))
			seq_printf(seq, "parent -- %s\n",
				   ref_clk->src_clk.list[ref_clk->src_clk.
							src_inx].clk->name);
		else
			seq_printf(seq, "%s derived from %s CCU\n", clock->name,
				   ref_clk->ccu_clk->clk.name);
		break;
	case CLK_TYPE_PLL:
		seq_printf(seq, "PLL: %s\n", clock->name);
		break;
	case CLK_TYPE_PLL_CHNL:
		pll_chnl_clk = to_pll_chnl_clk(clock);
		seq_printf(seq, "PLL: %s PLL Channel:%s\n",
			   pll_chnl_clk->pll_clk->clk.name, clock->name);
		break;
	case CLK_TYPE_CORE:
		core_clk = to_core_clk(clock);
		seq_printf(seq, "PLL: %s core_clk:%s\n",
			   core_clk->pll_clk->clk.name, clock->name);
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static int fops_source_open(struct inode *inode, struct file *file)
{
	return single_open(file, clk_source_show, inode->i_private);
}

static const struct file_operations clock_source_fops = {
	.open = fops_source_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};


static ssize_t ccu_volt_id_display(struct file *file, char __user *buf,
				   size_t len, loff_t *offset)
{
	u8 volt_tbl[8];
	int i, length = 0;
	ssize_t total_len = 0;
	char out_str[400];
	char *out_ptr;
	struct clk *clk = file->private_data;
	struct ccu_clk *ccu_clk;
	BUG_ON(clk == NULL);
	ccu_clk = to_ccu_clk(clk);

	memset(volt_tbl, 0, sizeof(volt_tbl));
	memset(out_str, 0, sizeof(out_str));
	out_ptr = &out_str[0];
	if (len < 400)
		return -EINVAL;

	if (ccu_volt_tbl_display(clk, volt_tbl))
		return -EINVAL;

	for (i = 0; i < ccu_clk->freq_count; i++) {
		length = snprintf(out_ptr, sizeof(out_str) - length,
				"volt_tbl[%d]: %x\n", i, volt_tbl[i]);
		out_ptr += length;
		total_len += length;
	}

	if (copy_to_user(buf, out_str, total_len))
		return -EFAULT;

	return simple_read_from_buffer(buf, len, offset, out_str,
				       total_len);
}

static ssize_t ccu_volt_id_update(struct file *file,
				  char const __user *buf, size_t count,
				  loff_t *offset)
{
	struct clk *clk = file->private_data;
	u32 len = 0;
	char *str_ptr;
	u32 freq_id = 0xFFFF, volt_id = 0xFFFF;
	char input_str[10];
	BUG_ON(clk == NULL);
	memset(input_str, 0, 10);
	if (count > 10)
		len = 10;
	else
		len = count;

	if (copy_from_user(input_str, buf, len))
		return -EFAULT;

	str_ptr = &input_str[0];
	/* coverity[secure_coding] */
	sscanf(str_ptr, "%x%x", &freq_id, &volt_id);
	if (freq_id == 0xFFFF || volt_id == 0xFFFF) {
		pr_err("invalid input\n");
		return count;
	}
	if (freq_id > 7 || volt_id > 0xF) {
		pr_err("Invalid param\n");
		return count;
	}
	ccu_volt_id_update_for_freqid(clk, (u8)freq_id, (u8)volt_id);
	return count;
}

static const struct file_operations ccu_volt_tbl_update_fops = {
	.open = clk_debugfs_open,
	.write = ccu_volt_id_update,
	.read = ccu_volt_id_display,
};

static ssize_t ccu_freq_list_show(struct file *file, char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct clk *clk = file->private_data;
	int i;
	u32 len = 0;
	char debug_fs_buf[200];
	struct pi *pi;
	struct pi_opp *pi_opp;
	struct ccu_clk *ccu_clk;

	BUG_ON(clk == NULL);
	ccu_clk = to_ccu_clk(clk);
	if (ccu_clk->pi_id == -1)
		return -1;

	pi = pi_mgr_get(ccu_clk->pi_id);
	BUG_ON(pi == NULL);
	pi_opp = pi->pi_opp;
	BUG_ON(pi_opp == NULL);
	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"Freq list for %s\n", ccu_clk->clk.name);

	for (i = 0; i < pi_opp->num_opp; i++)
		len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"OPP %d, Freq ID %d\n", (i + 1),
			ccu_clk_get_freq_id_from_opp(ccu_clk, i));

	return simple_read_from_buffer(user_buf, count, ppos, debug_fs_buf,
				       len);
}

static const struct file_operations ccu_freq_list_show_ops = {
	.open = clk_debugfs_open,
	.read = ccu_freq_list_show,
};

static ssize_t misc_dbg_bus_status(struct file *file,
		char __user *user_buf, size_t count, loff_t *ppos)
{
	u32 len = 0;
	char out_str[20];
	u32 bus_val;

	memset(out_str, 0, sizeof(out_str));
	bus_val = get_misc_dbg_bus();
	len += snprintf(out_str+len, sizeof(out_str)-len,
			"%x\n", bus_val);

	return simple_read_from_buffer(user_buf, count, ppos,
			out_str, len);
}

static ssize_t misc_dbg_bus_sel(struct file *file,
		char const __user *buf, size_t count, loff_t *offset)
{
	char input_str[10];
	u32 len = 0;
	u32 sel = 0;
	u32 dbg_bit_sel = 0;

	memset(input_str, 0, ARRAY_SIZE(input_str));
	if (count > ARRAY_SIZE(input_str))
		len = ARRAY_SIZE(input_str);
	else
		len = count;

	if (copy_from_user(input_str, buf, len))
		return -EFAULT;

	/* coverity[secure_coding] */
	sscanf(&input_str[0], "%x%x", &sel, &dbg_bit_sel);
	set_misc_dbg_bus(sel, dbg_bit_sel);

	return count;
}

static const struct file_operations misc_dbg_bus_fops = {
	.open = clk_debugfs_open,
	.write = misc_dbg_bus_sel,
	.read = misc_dbg_bus_status,
};

static struct dentry *dent_clk_root_dir;
int __init clock_debug_init(void)
{
	/* create root clock dir /clock */
	dent_clk_root_dir = debugfs_create_dir("clock", 0);
	if (!dent_clk_root_dir)
		return -ENOMEM;
	if (!debugfs_create_u32("debug", S_IRUGO | S_IWUSR,
				dent_clk_root_dir, (int *)&clk_debug))
		return -ENOMEM;
	if (!debugfs_create_file("misc_debug_bus", S_IRUSR | S_IWUSR,
				dent_clk_root_dir, NULL, &misc_dbg_bus_fops))
		return -ENOMEM;

	return 0;
}

int __init clock_debug_add_ccu(struct clk *c, bool is_root_ccu)
{
	#define DENT_COUNT 7
	struct ccu_clk *ccu_clk;
	int i = 0;
	struct dentry *dentry[DENT_COUNT] = {NULL};
	struct dentry *dent;
	BUG_ON(c == NULL);
	BUG_ON(!dent_clk_root_dir);
	ccu_clk = to_ccu_clk(c);
	BUG_ON(ccu_clk == NULL);
	ccu_clk->dent_ccu_dir = debugfs_create_dir(c->name, dent_clk_root_dir);
	if (!ccu_clk->dent_ccu_dir)
		goto err;

	dentry[i] = debugfs_create_file("clock_list",
					      S_IRUGO, ccu_clk->dent_ccu_dir, c,
					      &ccu_clock_list_fops);
	if (!dentry[i])
		goto err;

	dentry[++i] = debugfs_create_file("policy", S_IRUGO,
					  ccu_clk->dent_ccu_dir, c,
					  &ccu_policy_fops);
	if (!dentry[i])
		goto err;

	dentry[++i] = debugfs_create_u32("use_cnt", S_IRUGO,
					ccu_clk->dent_ccu_dir, &c->use_cnt);
	if (!dentry[i])
		goto err;

	dentry[++i] = debugfs_create_file("wr_en", S_IWUSR|S_IRUSR,
					  ccu_clk->dent_ccu_dir, c,
					  &ccu_wr_en_fops);
	if (!dentry[i])
		goto err;
	dentry[++i] = debugfs_create_file("clk_mon", S_IWUSR|S_IRUSR,
			ccu_clk->dent_ccu_dir, c,
			&clk_mon_fops);
	if (!dentry[i])
		goto err;

	if (ccu_clk->freq_count) {
		dentry[++i] = debugfs_create_file("freq_id", S_IRUGO,
					  ccu_clk->dent_ccu_dir, c,
					  &ccu_freqid_fops);
		if (!dentry[i])
			goto err;


		dentry[++i] = debugfs_create_file("freq_list", S_IWUSR|S_IRUSR,
			ccu_clk->dent_ccu_dir, c,
			&ccu_freq_list_show_ops);
		if (!dentry[i])
			goto err;

		dentry[++i] = debugfs_create_file("volt_id_update",
					    (S_IWUSR | S_IRUSR),
					    ccu_clk->dent_ccu_dir, c,
					    &ccu_volt_tbl_update_fops);
		if (!dentry[i])
			goto err;
	}

	if (CLK_FLG_ENABLED(c, CCU_DBG_BUS_EN)) {
		dent = debugfs_create_file("dbg_bus", S_IWUSR|S_IRUGO,
					  ccu_clk->dent_ccu_dir, c,
					  &ccu_dbg_bus_fops);
		if (!dent)
			goto err;
	}
	if (is_root_ccu) {
		if (!debugfs_create_file("clk_idle_debug", S_IRUSR | S_IWUSR,
					ccu_clk->dent_ccu_dir, NULL,
					&clock_idle_debug_fops))
			return -ENOMEM;
	}

	return 0;
err:
	for (i = 0; i < DENT_COUNT && dentry[i]; i++)
		debugfs_remove(dentry[i]);
	debugfs_remove(ccu_clk->dent_ccu_dir);

	return -ENOMEM;
}

int __init clock_debug_add_clock(struct clk *c)
{
	struct dentry *dent_clk_dir = 0, *dent_rate = 0, *dent_enable = 0,
	    *dent_status = 0, *dent_flags = 0, *dent_use_cnt = 0, *dent_id = 0,
	    *dent_parent = 0, *dent_source = 0, *dent_ccu_dir = 0,
	    *dent_reset = 0;
	struct peri_clk *peri_clk;
	struct pll_clk *pll_clk = NULL;
	struct core_clk *core_clk;
	struct pll_chnl_clk *pll_chnl_clk;
	struct bus_clk *bus_clk;
	struct ref_clk *ref_clk;
	switch (c->clk_type) {
	case CLK_TYPE_REF:
		ref_clk = to_ref_clk(c);
		dent_ccu_dir = ref_clk->ccu_clk->dent_ccu_dir;
		break;
	case CLK_TYPE_BUS:
		bus_clk = to_bus_clk(c);
		dent_ccu_dir = bus_clk->ccu_clk->dent_ccu_dir;
		break;
	case CLK_TYPE_PERI:
		peri_clk = to_peri_clk(c);
		dent_ccu_dir = peri_clk->ccu_clk->dent_ccu_dir;
		break;

	case CLK_TYPE_PLL:
		pll_clk = to_pll_clk(c);
		dent_ccu_dir = pll_clk->ccu_clk->dent_ccu_dir;
		break;

	case CLK_TYPE_PLL_CHNL:
		pll_chnl_clk = to_pll_chnl_clk(c);
		dent_ccu_dir = pll_chnl_clk->ccu_clk->dent_ccu_dir;
		break;

	case CLK_TYPE_CORE:
		core_clk = to_core_clk(c);
		dent_ccu_dir = core_clk->ccu_clk->dent_ccu_dir;
		break;
	default:
		return -EINVAL;
	}
	if (!dent_ccu_dir)
		return -ENOMEM;

	/* create root clock dir /clock/clk_a */
	dent_clk_dir = debugfs_create_dir(c->name, dent_ccu_dir);
	if (!dent_clk_dir)
		goto err;

	/* file /clock/clk_a/enable */
	dent_enable = debugfs_create_file("enable", S_IRUGO | S_IWUSR,
					  dent_clk_dir, c, &clock_enable_fops);
	if (!dent_enable)
		goto err;

	/* file /clock/clk_a/reset */
	dent_reset = debugfs_create_file("reset", S_IRUGO |
					 S_IWUSR, dent_clk_dir, c,
					 &clock_reset_fops);
	if (!dent_reset)
		goto err;

	/* file /clock/clk_a/status */
	dent_status = debugfs_create_file("status", S_IRUGO |
					  S_IWUSR, dent_clk_dir, c,
					  &clock_status_show_fops);
	if (!dent_status)
		goto err;

	/* file /clock/clk_a/rate */
	dent_rate = debugfs_create_file("rate", S_IRUGO | S_IWUSR,
					dent_clk_dir, c, &clock_rate_fops);
	if (!dent_rate)
		goto err;
	/* file /clock/clk_a/flags */
	dent_flags = debugfs_create_u32("flags", S_IRUGO |
			S_IWUSR, dent_clk_dir,
			(unsigned int *)&c->flags);
	if (!dent_flags)
		goto err;

	/* file /clock/clk_a/use_cnt */
	dent_use_cnt = debugfs_create_u32("use_cnt", S_IRUGO,
					  dent_clk_dir,
					  (unsigned int *)&c->use_cnt);
	if (!dent_use_cnt)
		goto err;

	/* file /clock/clk_a/id */
	dent_id = debugfs_create_u32("id", S_IRUGO, dent_clk_dir,
				     (unsigned int *)&c->id);
	if (!dent_id)
		goto err;

	/* file /clock/clk_a/parent */
	dent_parent = debugfs_create_file("parent", S_IRUGO,
		dent_clk_dir, c, &clock_parent_fops);
	if (!dent_parent)
		goto err;

	/* file /clock/clk_a/source */
	dent_source = debugfs_create_file("source", S_IRUGO,
		dent_clk_dir, c, &clock_source_fops);
	if (!dent_source)
		goto err;

	if (c->clk_type == CLK_TYPE_PLL) {
		BUG_ON(pll_clk == NULL);
		if (pll_clk->desense) {
			if (pll_clk->desense->flags & PLL_OFFSET_EN)
				debugfs_create_file("desense_rate",
					S_IRUGO | S_IWUSR, dent_clk_dir, c,
					&pll_desense_rate_fops);
		}
	}
	return 0;

err:
	debugfs_remove(dent_rate);
	debugfs_remove(dent_flags);
	debugfs_remove(dent_use_cnt);
	debugfs_remove(dent_id);
	debugfs_remove(dent_parent);
	debugfs_remove(dent_source);
	debugfs_remove(dent_clk_dir);
	return -ENOMEM;
}
