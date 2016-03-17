/************************************************************************/
/*                                                                      */
/*  Copyright 2013  Broadcom Corporation                                */
/*                                                                      */
/* Unless you and Broadcom execute a separate written software license  */
/* agreement governing use of this software, this software is licensed  */
/* to you under the terms of the GNU General Public License version 2   */
/* (the GPL), available at						*/
/*                                                                      */
/*          http://www.broadcom.com/licenses/GPLv2.php                  */
/*                                                                      */
/*  with the following added to such license:                           */
/*                                                                      */
/*  As a special exception, the copyright holders of this software give */
/*  you permission to link this software with independent modules, and  */
/*  to copy and distribute the resulting executable under terms of your */
/*  choice, provided that you also meet, for each linked independent    */
/*  module, the terms and conditions of the license of that module. An  */
/*  independent module is a module which is not derived from this       */
/*  software.  The special   exception does not apply to any            */
/*  modifications of the software.					*/
/*									*/
/*  Notwithstanding the above, under no circumstances may you combine	*/
/*  this software in any way with any other Broadcom software provided	*/
/*  under a license other than the GPL, without Broadcom's express	*/
/*  prior written consent.						*/
/*									*/
/************************************************************************/
#define DEBUG
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/amba/bus.h>
#include <linux/io.h>
#include <mach/axitrace.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");

#define	FILL_ATTR(kobj_attr, xname, xmode, xshow, xstore)	\
		kobj_attr.attr.name	= xname;		\
		kobj_attr.attr.mode	= xmode;		\
		kobj_attr.show		= xshow;		\
		kobj_attr.store	= xstore;

/* Stores the capability name */
static char *capability_strings[] = {
	"Counter Saturation Enable",
	"Trace Enable",
	"Commands Counting Enable",
	"Commands Filters Enable",
	"Beats Counting Enable",
	"Beats Filters Enable",
	"Busy Counting Enable",
	"Busy Filters Enable",
	"Latency Measurement Enable",
	"Latency Filter",
	"Read Latency Mode",
	"Outstanding Count Enable",
	"Outstanding Filter Enable",
};

static const char filter_name_prefix[] = "filter";

struct trace_counter {
	u32	offset;
	char	*description;
	u32	mask;
	u32	depends;
};

#define TRACE_CNTR(xoffset, xdes, xmask, xdepend)\
	{ .offset = xoffset,			\
	  .description = xdes,			\
	  .mask	= xmask,			\
	  .depends = xdepend			\
	}

static struct trace_counter counters[] = {
TRACE_CNTR(ATM_WRCMDS, "Write commands", 0xffffffff, COMMANDS_COUNTING),
TRACE_CNTR(ATM_RDCMDS, "Read commands", 0xffffffff, COMMANDS_COUNTING),
TRACE_CNTR(ATM_AWCYCLES, "Write Address Cycles", 0xffffffff, BUSY_COUNTING),
TRACE_CNTR(ATM_ARCYCLES, "Read Address Cycles", 0xffffffff, BUSY_COUNTING),
TRACE_CNTR(ATM_WCYCLES, "Write Data Cycles", 0xffffffff, BUSY_COUNTING),
TRACE_CNTR(ATM_RCYCLES, "Read Data Cycles", 0xffffffff, BUSY_COUNTING),
TRACE_CNTR(ATM_BCYCLES, "Write Response Cycles", 0xffffffff, BUSY_COUNTING),
TRACE_CNTR(ATM_AWBUSY, "Write Address Busy Cycles", 0xffffffff, BUSY_COUNTING),
TRACE_CNTR(ATM_ARBUSY, "Read Address Busy Cycles", 0xffffffff, BUSY_COUNTING),
TRACE_CNTR(ATM_WBUSY, "Write Data Busy Cycles", 0xffffffff, BUSY_COUNTING),
TRACE_CNTR(ATM_RBUSY, "Read Data Busy Cycles", 0xffffffff, BUSY_COUNTING),
TRACE_CNTR(ATM_BBUSY, "Write Response Busy Cycles", 0xffffffff, BUSY_COUNTING),
TRACE_CNTR(ATM_WRSUM, "Write Latency Sum Cycles", 0xffffffff,
		LATENCY_MEASUREMENT),
TRACE_CNTR(ATM_RDSUM, "Read Latency Sum Cycles", 0xffffffff,
		LATENCY_MEASUREMENT),
TRACE_CNTR(ATM_WRMIN, "Write Latency Minimum Cycles", 0x3ff,
		LATENCY_MEASUREMENT),
TRACE_CNTR(ATM_RDMIN, "Read Latency Minimum Cycles", 0x3ff,
		LATENCY_MEASUREMENT),
TRACE_CNTR(ATM_WRMAX, "Write Latency Maximum Cycles", 0x3fff,
		LATENCY_MEASUREMENT),
TRACE_CNTR(ATM_RDMAX, "Read Latency Maximum Cycles", 0x3fff,
		LATENCY_MEASUREMENT),
TRACE_CNTR(ATM_WRBEATS, "Write Data Beats", 0xffffffff, BEATS_COUNTING),
TRACE_CNTR(ATM_RDBEATS, "Read Data Beats", 0xffffffff, BEATS_COUNTING),
TRACE_CNTR(ATM_WROUTS, "Write Outstanding Command Cycles", 0xffffffff,
		OUTS_COUNTING),
TRACE_CNTR(ATM_RDOUTS, "Read Outstanding Command Cycles", 0xffffffff,
		OUTS_COUNTING),
{}
};

static struct complete_trace_src_info tracer;

static inline struct per_trace_info *
trace_attrs_to_per_trace_info(void *trace_attr)
{
	return container_of(trace_attr, struct per_trace_info, trace_attrs);
}

static inline struct per_trace_info *
fls_attrs_to_per_trace_info(void *trace_attr)
{
	return container_of(trace_attr, struct per_trace_info, filter_attrs);
}

static int get_cap_state(unsigned int cap, struct per_trace_info *info)
{
	if (cap == OUTS_THRES)
		return ((info->state.cap_state & OUTS_THRES_VAL(MAX_OUTS_THRES))
			>> OUTS_THRES_POS);
	else
		return info->state.cap_state & cap;
}

static void set_cap_state(u32 cap, u8 value, struct per_trace_info *info)
{
	if (cap == OUTS_THRES) {
		info->state.cap_state &= ~OUTS_THRES_VAL(MAX_OUTS_THRES);
		info->state.cap_state |= OUTS_THRES_VAL(value);
	} else {
		info->state.cap_state &= ~cap;
		info->state.cap_state |= (value << (ffs(cap) - 1));
	}
}

static u32 get_fls_state(unsigned int item, int index,
			struct per_trace_info *info)
{
	u32 retval;
	struct filter_state *state = &info->state.fls_state[index];

	switch (item) {
	case FLS_READ:
		retval = state->read;
		break;
	case FLS_WRITE:
		retval = state->write;
		break;
	case FLS_LEN_MODE:
		retval = state->len_mode;
		break;
	case FLS_LEN:
		retval = state->filter_len;
		break;
	case FLS_ID_MASK:
		retval = state->id_mask;
		break;
	case FLS_ID:
		retval = state->filter_id;
		break;
	case FLS_OPEN:
		retval = state->open;
		break;
	case FLS_SECURE:
		retval = state->secure;
		break;
	case FLS_ADDR_LOW:
		retval = state->addr_low;
		break;
	case FLS_ADDR_HIGH:
		retval = state->addr_high;
		break;
	default:
		pr_warn("invalid filter item");
		retval = -1;
		break;
	}

	dev_dbg(tracer.dev, "%s index = %d item = %d value = %d\n",
				__func__, index, item, retval);
	return retval;
}

static void set_fls_state(u32 item, s8 index, u32 value,
			struct per_trace_info *info)
{
	struct filter_state *state = &info->state.fls_state[index];

	dev_dbg(tracer.dev, "%s index = %d item = %d value = %d\n",
				__func__, index, item, value);
	switch (item) {
	case FLS_READ:
		state->read = value;
		break;
	case FLS_WRITE:
		state->write = value;
		break;
	case FLS_LEN_MODE:
		state->len_mode = value;
		break;
	case FLS_LEN:
		state->filter_len = value;
		break;
	case FLS_ID_MASK:
		state->id_mask = value & FLS_ID_MASK_WIDTH;
		break;
	case FLS_ID:
		state->filter_id = value & FLS_ID_WIDTH;
		break;
	case FLS_OPEN:
		state->open = value;
		break;
	case FLS_SECURE:
		state->secure = value;
		break;
	case FLS_ADDR_LOW:
		state->addr_low = value;
		break;
	case FLS_ADDR_HIGH:
		state->addr_high = value;
		break;
	default:
		pr_warn("invalid filter item");
		break;
	}
}

static bool get_trace_state(struct per_trace_info *info)
{
	return info->state.running;
}

static void set_trace_state(bool running, struct per_trace_info *info)
{
	info->state.running = running;
}
/*
 * Driver can have x capabilities, and a device can have y capabilites
 * where x >= y.
 * Eg: driver supports 1 to 10 caps. But device has only 2 , 3, and 4
 * caps listed in the driver caps.
 * When user cat the capability file, he see the caps starting from 1 to 3.
 * Below functions are helpers to converts device cap number to driver
 * cap number.
 */

/* Find bit postion of nth set bit.
 * value: any 32 bit number
 * n	: any number from 1 to 32
 * return : postion of nth setbit in value counting from 1.
 */
static int find_pos_nth_setbit(u32 value, int n)
{
	u32 set_bit = 0;
	while (n--) {
		set_bit = ffs(value);
		value &= ~(1 << (set_bit - 1));
	}

	return set_bit ? set_bit : 0;
}

/* Find number of set bits till bit postion n.
 * value: any 32 bit number
 * n    : any number from 0 to 31
 * return : number of setbits in value till nth bit.
 */
static int find_numof_setbit_till_n(u32 value, int n)
{
	u32 nsetbit = 0;
	u32 set_bit;

	do {
		set_bit =  ffs(value);
		value &= ~(1 << (set_bit - 1));
		nsetbit++;
	} while ((set_bit-1) < n);

	return nsetbit;
}

static int get_cap_info(struct per_trace_info *trace_info, char *buf)
{

	int index;
	int count = 1, retval = 0, enable;
	char *star = "*";
	char *blank = " ";
	u32 dev_cap = trace_info->p_source_info->cap;
	u32 drv_cap = trace_info->driver_cap;
	u32 set_bit;

	set_bit = ffs(dev_cap);
	if (0 == set_bit)
		return -EINVAL;

	do {
		dev_cap &= ~(1 << (set_bit - 1));
		index = find_numof_setbit_till_n(drv_cap, set_bit - 1);
		index -= 1;
		enable = get_cap_state(1 << (set_bit-1), trace_info);
		retval += sprintf(buf + retval,
				"%s %2d %s\n",
				enable ? star : blank,
				count++, capability_strings[index]);
		if (BIT(set_bit - 1) == OUTS_COUNTING)
			retval += sprintf(buf + retval,
					"\t\tThreshold = %2d\n",
					get_cap_state(OUTS_THRES, trace_info));
	} while ((set_bit = ffs(dev_cap)) != 0);

	return retval;

}

static ssize_t cap_show(struct kobject *kobj, struct kobj_attribute *attr,
		      char *buf)
{
	struct kobj_attribute *trace_attr = attr - CAP_ATTR;
	struct per_trace_info *trace_info =
				trace_attrs_to_per_trace_info(trace_attr);

	return get_cap_info(trace_info, buf);
}

static ssize_t cap_store(struct kobject *kobj, struct kobj_attribute *attr,
		       const char *buf, size_t count)
{
	struct kobj_attribute *trace_attr = attr - CAP_ATTR;
	struct per_trace_info *trace_info =
				trace_attrs_to_per_trace_info(trace_attr);
	int enable, retval;
	unsigned long item, outs_threshold;
	char *x = (char *)&buf[1];
	char *y;

	if (buf[0] == '+')
		enable = 1;
	else if (buf[0] == '-')
		enable = 0;
	else {
		pr_err("invlaid format\n");
		return count;
	}

	do {
		y = strsep(&x, "=");
		retval = kstrtoul(y, 0, &item);
		if (retval < 0) {
			pr_err("invalid argument\n");
			return -EINVAL;
		}

		if (x == NULL)
			break;

		y = strsep(&x, "=");
		retval = kstrtoul(y, 0, &outs_threshold);
		if (retval < 0) {
			pr_err("invalid argument\n");
			return -EINVAL;
		}

		if (outs_threshold > MAX_OUTS_THRES) {
			pr_err("Max available outstanding threshold is %d\n",
				MAX_OUTS_THRES);
			return -EINVAL;
		}
		set_cap_state(OUTS_THRES, outs_threshold,
				trace_info);

	} while (0);

	item = find_pos_nth_setbit(trace_info->p_source_info->cap, item);
	item -= 1;
	item = 1 << item;

	set_cap_state(item, enable, trace_info);
	return count;
}

static int get_master_info(struct per_trace_info *trace_info, char *buf)
{
	int retval = 0, i = 0;
	int count = 1;
	struct axi_master *masters = trace_info->p_source_info->masters;

	for (i = 0 ; masters[i].name != NULL ; i++) {
		retval += scnprintf(buf + retval, (PAGE_SIZE - retval),
				" %2d %s\n", count++, masters[i].name);
	}
	return retval;
}

static ssize_t master_show(struct kobject *kobj, struct kobj_attribute *attr,
		      char *buf)
{
	struct kobj_attribute *trace_attr = attr - MASTER_ATTR;
	struct per_trace_info *trace_info =
				trace_attrs_to_per_trace_info(trace_attr);

	return get_master_info(trace_info, buf);
}

static int get_slave_info(struct per_trace_info *trace_info, char *buf)
{
	struct axi_slave *slaves = trace_info->p_source_info->slaves;
	int retval = 0, i = 0;
	int count = 1;

	for (i = 0 ; slaves[i].name != NULL ; i++)
		retval += scnprintf(buf + retval, (PAGE_SIZE - retval),
				" %2d %s\n", count++, slaves[i].name);
	return retval;
}

static ssize_t slave_show(struct kobject *kobj, struct kobj_attribute *attr,
		      char *buf)
{
	struct kobj_attribute *trace_attr = attr - SLAVE_ATTR;
	struct per_trace_info *trace_info =
			trace_attrs_to_per_trace_info(trace_attr);

	return get_slave_info(trace_info, buf);
}

static ssize_t control_show(struct kobject *kobj, struct kobj_attribute *attr,
		      char *buf)
{
	struct kobj_attribute *trace_attr = attr - CONTROL_ATTR;
	struct per_trace_info *trace_info =
			trace_attrs_to_per_trace_info(trace_attr);

	return sprintf(buf, "%s\n",
			get_trace_state(trace_info) ? "running"  : "stopped");
}

static void send_local_cmd(struct per_trace_info *trace_info, int cmd)
{
	axi_writel(trace_info, cmd, ATM_CMD);
}

static void config_funnels(struct per_trace_info *trace_info)
{
	struct funnel_map *map = trace_info->p_source_info->map;

	for ( ; map->addr != NULL ; map++)
		writel((1 << map->bit), map->addr);
}

static void trace_start(struct per_trace_info *trace_info)
{
	u32 value = 0;
	struct filter_state	*f;
	int i;
	int filter_count = trace_info->p_source_info->filter_count;

	config_funnels(trace_info);

	value = trace_info->state.cap_state;

	/* set local control by default */
	value |= 1;

	axi_writel(trace_info, value, ATM_CONFIG);

	for (i = 0 ; i < filter_count ; i++) {
		dev_dbg(tracer.dev, "filter %d\n", i);
		f = &trace_info->state.fls_state[i];
		value = (f->read ? 1 << SHIFT(FILTER_READ) : 0)		|
			(f->write ? 1 << SHIFT(FILTER_WRITE) : 0)	|
			(f->open ? 1 << SHIFT(FILTER_OPEN) : 0)		|
			(f->secure ? 1 << SHIFT(FILTER_SECURE) : 0) ;

		dev_dbg(tracer.dev, "0x%x open = %d\n", 1 << SHIFT(FILTER_OPEN),
								f->open);
		dev_dbg(tracer.dev, "0x%x secure = %d\n",
				1 << SHIFT(FILTER_SECURE), f->secure);

		value |= (f->filter_id << SHIFT(FILTER_ID));
		value |= (f->id_mask << SHIFT(FILTER_ID_MASK));

		/* TODO: Lets hard code len mode for now */
		value |= 0x3 << SHIFT(FILTER_LEN_MODE);
		axi_writel(trace_info, value, ATM_FILTER_BASE + i * 0x10);
		axi_writel(trace_info, f->addr_low,
				ATM_ADDRLOW_BASE + i * 0x10);
		axi_writel(trace_info, f->addr_high,
				ATM_ADDRHIGH_BASE + i * 0x10);
	}

	send_local_cmd(trace_info, TRACE_CNTR_START);
}

static ssize_t control_store(struct kobject *kobj, struct kobj_attribute *attr,
		       const char *buf, size_t count)
{
	struct kobj_attribute *trace_attr = attr - CONTROL_ATTR;
	struct per_trace_info *trace_info =
				trace_attrs_to_per_trace_info(trace_attr);
	int retval;
	unsigned long item;

	retval = kstrtoul(buf, 10, &item);
	if (retval < 0)
		return retval;
	/*
	 * 0 to reset the counters
	 * 1 to stop the trace
	 * 2 to start the trace
	 */
	switch (item) {
	case TRACE_CNTR_START:
		trace_start(trace_info);
		set_trace_state(1, trace_info);
		break;
	case TRACE_CNTR_STOP:
		send_local_cmd(trace_info, TRACE_CNTR_STOP);
		set_trace_state(0, trace_info);
		break;
	case TRACE_CNTR_CLEAR:
		if (get_trace_state(trace_info))
			pr_info("Cannot clear. Stop trace first\n");
		else
			send_local_cmd(trace_info, TRACE_CNTR_CLEAR);

		break;
	default:
		pr_err("invalid option\n");
		break;
	}

	return count;
}

static ssize_t counter_show(struct kobject *kobj, struct kobj_attribute *attr,
		      char *buf)
{
	struct kobj_attribute *trace_attr = attr - COUNTER_ATTR;
	struct per_trace_info *trace_info =
				trace_attrs_to_per_trace_info(trace_attr);
	struct trace_counter *temp;
	int retval = 0;

	for (temp = counters ; temp->offset != 0 ; temp++)
		if (trace_info->state.cap_state & temp->depends)
			retval += scnprintf(buf + retval, (PAGE_SIZE - retval),
				"%s: %x\n", temp->description,
				readl(trace_info->trace_regs + temp->offset));

	return retval;
}

static ssize_t cur_config_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	struct kobj_attribute *trace_attr = attr - CUR_CONFIG_ATTR;
	struct per_trace_info *trace_info =
		trace_attrs_to_per_trace_info(trace_attr);
	int filter_count = trace_info->p_source_info->filter_count;
	int retval = 0, i;

	retval = sprintf(buf, "%s", "Capabilities Config:\n");
	retval += get_cap_info(trace_info, buf + retval);

	for (i = 0 ; i < filter_count; i++) {
		retval += sprintf(buf + retval, "\nFilter%d Config:\n", i);
		retval += sprintf(buf + retval, "  Axi id    : 0x%x\n",
				get_fls_state(FLS_ID, i, trace_info));
		retval += sprintf(buf + retval, "  ID Mask   : 0x%x\n",
				get_fls_state(FLS_ID_MASK, i, trace_info));
		retval += sprintf(buf + retval, "  Read      : %d\n",
				get_fls_state(FLS_READ, i, trace_info));
		retval += sprintf(buf + retval, "  Write     : %d\n",
				get_fls_state(FLS_WRITE, i, trace_info));
		retval += sprintf(buf + retval, "  Open      : %d\n",
				get_fls_state(FLS_OPEN, i, trace_info));
		retval += sprintf(buf + retval, "  Secure    : %d\n",
				get_fls_state(FLS_SECURE, i, trace_info));
		retval += sprintf(buf + retval, "  Addr Low  : 0x%x\n",
				get_fls_state(FLS_ADDR_LOW, i, trace_info));
		retval += sprintf(buf + retval, "  Addr High : 0x%x\n",
				get_fls_state(FLS_ADDR_HIGH, i, trace_info));
	}
	return retval;
}

/* Since attr value will be same for all the filters, we have to
 * decipher the name(eg filter0, filter3) to find the filter index. */
static int find_filter_index(struct kobject *kobj, const char *name)
{
	unsigned long index;
	int retval;

	retval = kstrtoul(&kobj->name[strlen(name)], 0, &index);
	if (retval < 0)
		return retval;
	else
		return index;
}

static ssize_t filter_axi_id_store(struct kobject *kobj,
			struct kobj_attribute *attr,
			const char *buf, size_t count)
{
	struct kobj_attribute *fls_attr = attr - FIL_AXI_ID_ATTR;
	struct per_trace_info *trace_info =
				fls_attrs_to_per_trace_info(fls_attr);
	int filter_index, retval;
	unsigned long value;

	retval = kstrtoul(buf, 0, &value);
	if (retval < 0)
		return retval;

	filter_index = find_filter_index(kobj, filter_name_prefix);

	set_fls_state(FLS_ID, filter_index, value, trace_info);
	return count;
}

static ssize_t filter_axi_id_show(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf)
{
	struct kobj_attribute *fls_attr = attr - FIL_AXI_ID_ATTR;
	struct per_trace_info *trace_info =
				fls_attrs_to_per_trace_info(fls_attr);
	int filter_index;

	filter_index = find_filter_index(kobj, filter_name_prefix);
	return  scnprintf(buf, PAGE_SIZE, "%d\n",
			get_fls_state(FLS_ID, filter_index, trace_info));
}

static ssize_t filter_id_mask_store(struct kobject *kobj,
			struct kobj_attribute *attr,
			const char *buf, size_t count)
{
	struct kobj_attribute *fls_attr = attr - FIL_ID_MASK_ATTR;
	struct per_trace_info *trace_info =
				fls_attrs_to_per_trace_info(fls_attr);
	int filter_index, retval;
	unsigned long value;

	retval = kstrtoul(buf, 0, &value);
	if (retval < 0)
		return retval;

	filter_index = find_filter_index(kobj, filter_name_prefix);

	set_fls_state(FLS_ID_MASK, filter_index, value, trace_info);
	return count;
}

static ssize_t filter_id_mask_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	struct kobj_attribute *fls_attr = attr - FIL_ID_MASK_ATTR;
	struct per_trace_info *trace_info =
			fls_attrs_to_per_trace_info(fls_attr);
	int filter_index;

	filter_index = find_filter_index(kobj, filter_name_prefix);
	return  scnprintf(buf, PAGE_SIZE, "%d\n", get_fls_state(FLS_ID_MASK,
			filter_index, trace_info));
}

static ssize_t filter_master_store(struct kobject *kobj,
			struct kobj_attribute *attr,
			const char *buf, size_t count)
{
	struct kobj_attribute *fls_attr = attr - FIL_MASTER_ATTR;
	struct per_trace_info *trace_info =
				fls_attrs_to_per_trace_info(fls_attr);
	struct axitrace_source *sources = trace_info->p_source_info;
	int len = strlen(buf)-1;
	int master_count, i, filter_index;
	unsigned int axi_id_mask;

	for (master_count = 0 ; sources->masters[master_count].name != NULL ;
							master_count++)
		;

	for (i = 0 ; i < master_count ; i++)
		if (!strncmp(buf, sources->masters[i].name, len))
			break;

	if (i == master_count) {
		pr_warn("invalid argument\n");
		return -EINVAL;
	} else if (!strncmp("ALL", sources->masters[i].name, len)) {
		pr_warn("configure trace for all masters\n");
		axi_id_mask = 0;
	} else
		axi_id_mask = sources->axi_id_mask;

	filter_index = find_filter_index(kobj, filter_name_prefix);
	set_fls_state(FLS_ID, filter_index, sources->masters[i].axi_id,
								trace_info);
	set_fls_state(FLS_ID_MASK, filter_index, axi_id_mask, trace_info);

	return count;
}

static ssize_t filter_master_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	struct kobj_attribute *fls_attr = attr - FIL_MASTER_ATTR;
	struct per_trace_info *trace_info =
			fls_attrs_to_per_trace_info(fls_attr);
	struct axitrace_source *sources = trace_info->p_source_info;
	int master_count, i, filter_index, axi_id, retval;

	filter_index = find_filter_index(kobj, filter_name_prefix);
	axi_id = get_fls_state(FLS_ID, filter_index, trace_info);

	for (master_count = 0 ; sources->masters[master_count].name != NULL ;
							master_count++)
		;

	for (i = 0 ; i < master_count ; i++)
		if (sources->masters[i].axi_id == axi_id)
			break;

	if (i == master_count)
		retval = sprintf(buf, "%s\n", "unknown master");
	else
		retval = sprintf(buf, "%s\n",  sources->masters[i].name);

	return retval;
}

static ssize_t filter_slave_store(struct kobject *kobj,
			struct kobj_attribute *attr,
			const char *buf, size_t count)
{
	struct kobj_attribute *fls_attr = attr - FIL_SLAVE_ATTR;
	struct per_trace_info *trace_info =
				fls_attrs_to_per_trace_info(fls_attr);
	struct axitrace_source *sources = trace_info->p_source_info;
	int len = strlen(buf)-1;
	int slave_count, i, filter_index, retval;

	char *x = (char *)buf;
	char *y;
	unsigned long addr_low, addr_high;

	filter_index = find_filter_index(kobj, filter_name_prefix);

	/* get slave count */
	for (slave_count = 0 ; sources->slaves[slave_count].name != NULL ;
						slave_count++)
		;

	for (i = 0 ; i < slave_count ; i++)
		if (!strncmp(buf, sources->slaves[i].name, len))
			break;

	if (i == slave_count) {
		/* comparing slave names failed. Let's see if user
		 * had passed the range(eg: 0x80000000-0xa0000000) */
		y = strsep(&x, "-");
		if (x == NULL) {
			pr_warn("invalid argument\n");
			return -EINVAL;
		}
		retval = kstrtoul(y, 0, &addr_low);
		if (retval < 0)
			return -EINVAL;

		y = strsep(&x, "-");
		retval = kstrtoul(y, 0, &addr_high);
		if (retval < 0)
			return -EINVAL;

	} else {
		addr_low = sources->slaves[i].start_addr;
		addr_high = sources->slaves[i].end_addr;
	}

	set_fls_state(FLS_ADDR_LOW, filter_index, addr_low, trace_info);
	set_fls_state(FLS_ADDR_HIGH, filter_index, addr_high, trace_info);

	return count;
}

static ssize_t filter_slave_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	struct kobj_attribute *fls_attr = attr - FIL_SLAVE_ATTR;
	struct per_trace_info *trace_info =
			fls_attrs_to_per_trace_info(fls_attr);
	struct axitrace_source *sources = trace_info->p_source_info;
	int slave_count, i, filter_index, retval;
	u32 addr_low, addr_high;

	filter_index = find_filter_index(kobj, filter_name_prefix);
	addr_low = get_fls_state(FLS_ADDR_LOW, filter_index, trace_info);
	addr_high = get_fls_state(FLS_ADDR_HIGH, filter_index, trace_info);

	for (slave_count = 0 ; sources->slaves[slave_count].name != NULL ;
				slave_count++)
		;

	for (i = 0 ; i < slave_count ; i++)
		if (sources->slaves[i].start_addr == addr_low &&
			sources->slaves[i].end_addr == addr_high)
			break;

	if (i == slave_count)
		retval = sprintf(buf, "%s\n", "unknown slave");
	else
		retval = sprintf(buf, "%s\n",  sources->slaves[i].name);

	return retval;
}

static ssize_t filter_open_store(struct kobject *kobj,
			struct kobj_attribute *attr,
			const char *buf, size_t count)
{
	struct kobj_attribute *fls_attr = attr - FIL_OPEN_ATTR;
	struct per_trace_info *trace_info =
			fls_attrs_to_per_trace_info(fls_attr);
	int filter_index, retval;
	unsigned long value;

	retval = kstrtoul(buf, 0, &value);
	if (retval < 0)
		return retval;

	filter_index = find_filter_index(kobj, filter_name_prefix);

	set_fls_state(FLS_OPEN, filter_index, value, trace_info);

	return count;
}

static ssize_t filter_open_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	struct kobj_attribute *fls_attr = attr - FIL_OPEN_ATTR;
	struct per_trace_info *trace_info =
			fls_attrs_to_per_trace_info(fls_attr);
	int filter_index;

	filter_index = find_filter_index(kobj, filter_name_prefix);
	return scnprintf(buf, PAGE_SIZE, "%d\n", get_fls_state(FLS_OPEN,
			filter_index, trace_info));
}

static ssize_t filter_secure_store(struct kobject *kobj,
			struct kobj_attribute *attr,
			const char *buf, size_t count)
{
	struct kobj_attribute *fls_attr = attr - FIL_SECURE_ATTR;
	struct per_trace_info *trace_info =
			fls_attrs_to_per_trace_info(fls_attr);
	int filter_index, retval;
	unsigned long value;

	retval = kstrtoul(buf, 0, &value);
	if (retval < 0)
		return retval;

	filter_index = find_filter_index(kobj, filter_name_prefix);

	set_fls_state(FLS_SECURE, filter_index, value, trace_info);

	return count;
}

static ssize_t filter_secure_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	struct kobj_attribute *fls_attr = attr - FIL_SECURE_ATTR;
	struct per_trace_info *trace_info =
			fls_attrs_to_per_trace_info(fls_attr);
	int filter_index;

	filter_index = find_filter_index(kobj, filter_name_prefix);
	return scnprintf(buf, PAGE_SIZE, "%d\n", get_fls_state(FLS_SECURE,
			filter_index, trace_info));
}

static ssize_t filter_read_store(struct kobject *kobj,
			struct kobj_attribute *attr,
		       const char *buf, size_t count)
{
	struct kobj_attribute *fls_attr = attr - FIL_READ_ATTR;
	struct per_trace_info *trace_info =
			fls_attrs_to_per_trace_info(fls_attr);
	int filter_index, retval;
	unsigned long value;

	retval = kstrtoul(buf, 0, &value);
	if (retval < 0)
		return retval;

	filter_index = find_filter_index(kobj, filter_name_prefix);

	set_fls_state(FLS_READ, filter_index, value, trace_info);

	return count;
}

static ssize_t filter_read_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	struct kobj_attribute *fls_attr = attr - FIL_READ_ATTR;
	struct per_trace_info *trace_info =
		fls_attrs_to_per_trace_info(fls_attr);
	int filter_index;

	filter_index = find_filter_index(kobj, filter_name_prefix);
	return scnprintf(buf, PAGE_SIZE, "%d\n", get_fls_state(FLS_READ,
			filter_index, trace_info));
}

static ssize_t filter_write_store(struct kobject *kobj,
			struct kobj_attribute *attr,
			const char *buf, size_t count)
{
	struct kobj_attribute *fls_attr = attr - FIL_WRITE_ATTR;
	struct per_trace_info *trace_info =
		fls_attrs_to_per_trace_info(fls_attr);
	int filter_index, retval;
	unsigned long value;

	retval = kstrtoul(buf, 0, &value);
	if (retval < 0)
		return retval;

	filter_index = find_filter_index(kobj, filter_name_prefix);

	set_fls_state(FLS_WRITE, filter_index, value, trace_info);

	return count;
}

static ssize_t filter_write_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	struct kobj_attribute *fls_attr = attr - FIL_WRITE_ATTR;
	struct per_trace_info *trace_info =
			fls_attrs_to_per_trace_info(fls_attr);
	int filter_index;

	filter_index = find_filter_index(kobj, filter_name_prefix);
	return scnprintf(buf, PAGE_SIZE, "%d\n", get_fls_state(FLS_WRITE,
			filter_index, trace_info));
}
static char filter_name[20];

static int find_source_count(struct axitrace_source *sources)
{
	int count;

	for (count = 0 ; sources[count].name != NULL ; count++)
		;
	return count;
}


static int axitrace_probe(struct amba_device *dev,
				const struct amba_id *id)
{
	struct axitrace_platform_data *trace_data = dev->dev.platform_data;
	struct axitrace_source *sources;
	struct complete_trace_src_info *t = &tracer;
	int i = 0, retval, j, k;

	if (!trace_data)
		return -EINVAL;
	sources = trace_data->sources;
	t->dev = &dev->dev;
	amba_set_drvdata(dev, t);

	t->trace_src_count = find_source_count(sources);
	if (t->trace_src_count < 1) {
		dev_err(t->dev, "No sources found\n");
		retval = -EINVAL;
		goto out;
	}
	/* Allocate memory for per_trace array depending on the size of sources
	 * sent by platform */
	t->per_trace = kzalloc(sizeof(struct per_trace_info)*t->trace_src_count,
				GFP_KERNEL);
	if (t->per_trace == NULL) {
		dev_err(t->dev, "Failed to allocate memory\n");
		retval = -ENOMEM;
		goto out;
	}

	/* Iterate on all the trace sources. Sources are expected to be
	 * NULL terminated */
	while (sources[i].name != NULL) {
		t->per_trace[i].driver_cap = AXITRACE_FULL_CAP;
		dev_info(t->dev, "source name = %s\n", sources[i].name);
		t->per_trace[i].name = sources[i].name;

		/* Create per trace directory. eg:trace17, trace1 etc.. */
		t->per_trace[i].dir_kobj =
			kobject_create_and_add(sources[i].name, &dev->dev.kobj);
		if (t->per_trace[i].dir_kobj == NULL) {
			dev_err(t->dev, "Directory creation failed\n");
			retval =  -ENOMEM;
			goto out1;
		}

		/* Save source info to per trace info struct */
		t->per_trace[i].p_source_info = &sources[i];

		FILL_ATTR(t->per_trace[i].trace_attrs[CAP_ATTR], "capabilities",
				0644, cap_show, cap_store);
		FILL_ATTR(t->per_trace[i].trace_attrs[CUR_CONFIG_ATTR],
				"current_configuration",
				0644, cur_config_show, NULL);
		FILL_ATTR(t->per_trace[i].trace_attrs[COUNTER_ATTR], "counters",
				0644, counter_show, NULL);
		FILL_ATTR(t->per_trace[i].trace_attrs[MASTER_ATTR], "masters",
				0644, master_show, NULL);
		FILL_ATTR(t->per_trace[i].trace_attrs[SLAVE_ATTR], "slaves",
				0644, slave_show, NULL);
		FILL_ATTR(t->per_trace[i].trace_attrs[CONTROL_ATTR],
				"trace_control",
				0644, control_show, control_store);

		t->per_trace[i].attr_group.attrs =
			kmalloc(sizeof(struct attribute *) * TRACE_END_ATTR,
				GFP_KERNEL);
		if (NULL == t->per_trace[i].attr_group.attrs) {
			dev_err(t->dev,
				"Cannot allocate memory for sysfs groups\n");
			retval = -ENOMEM;
			goto out2;
		}

		/* Assign attributes to group */
		for (j = CAP_ATTR ; j < TRACE_END_ATTR ; j++) {
			t->per_trace[i].attr_group.attrs[j] =
				&t->per_trace[i].trace_attrs[j].attr;
			sysfs_attr_init(&t->per_trace[i].trace_attrs[j].attr);
		}

		t->per_trace[i].attr_group.attrs[TRACE_END_ATTR] = NULL;

		retval = sysfs_create_group(t->per_trace[i].dir_kobj,
					&t->per_trace[i].attr_group);
		if (retval) {
			dev_err(t->dev, "Cannot create group\n");
			goto out2;
		}

		t->per_trace[i].filterdir_kobj =
			kzalloc(sizeof(struct kobject *) *
			sources[i].filter_count, GFP_KERNEL);

		t->per_trace[i].filter_group =
			kzalloc(sizeof(struct attribute_group) *
			sources[i].filter_count, GFP_KERNEL);

		t->per_trace[i].state.fls_state =
			kzalloc(sizeof(struct filter_state) *
			sources[i].filter_count, GFP_KERNEL);

		if (t->per_trace[i].filterdir_kobj == NULL	||
		   t->per_trace[i].filter_group == NULL		||
		   t->per_trace[i].state.fls_state == NULL) {
			dev_err(t->dev, "Failed to alocate memory\n");
			retval = -ENOMEM;
			goto out2;
		}

		for (j = 0 ; j < sources[i].filter_count ; j++) {

			/* create filter directory with name filter0, fitler1.*/
			snprintf(filter_name, sizeof(filter_name), "%s%d",
				filter_name_prefix, j);

			t->per_trace[i].filterdir_kobj[j] =
				kobject_create_and_add(filter_name,
						t->per_trace[i].dir_kobj);
			if (NULL == t->per_trace[i].filterdir_kobj[j]) {
				dev_err(t->dev, "Filter dir creation failed\n");
				retval = -ENOMEM;
				goto out2;
			}

			/* Allocate memory for attribute array */
			t->per_trace[i].filter_group[j].attrs =
				kmalloc(sizeof(struct attribute *) *
				FIL_END_ATTR, GFP_KERNEL);
			if (NULL ==  t->per_trace[i].filter_group[j].attrs) {
				dev_err(t->dev, "Error allocating memory\n");
				retval = -ENOMEM;
				goto out2;
			}

			FILL_ATTR(t->per_trace[i].filter_attrs[FIL_AXI_ID_ATTR],
					"axi_id", 0644 , filter_axi_id_show,
					filter_axi_id_store);
			FILL_ATTR(t->per_trace[i].filter_attrs[FIL_ID_MASK_ATTR]
					, "id_mask", 0644, filter_id_mask_show,
					filter_id_mask_store);
			FILL_ATTR(t->per_trace[i].filter_attrs[FIL_MASTER_ATTR],
					"master", 0644, filter_master_show,
					filter_master_store);
			FILL_ATTR(t->per_trace[i].filter_attrs[FIL_SLAVE_ATTR],
					"slave", 0644, filter_slave_show,
					filter_slave_store);
			FILL_ATTR(t->per_trace[i].filter_attrs[FIL_OPEN_ATTR],
					"open", 0644, filter_open_show,
					filter_open_store);
			FILL_ATTR(t->per_trace[i].filter_attrs[FIL_SECURE_ATTR],
					"secure", 0644, filter_secure_show,
					filter_secure_store);
			FILL_ATTR(t->per_trace[i].filter_attrs[FIL_READ_ATTR],
					"read", 0644, filter_read_show,
					filter_read_store);
			FILL_ATTR(t->per_trace[i].filter_attrs[FIL_WRITE_ATTR],
					"write", 0644, filter_write_show,
					filter_write_store);

			/* Assign attributes to group */
			for (k = FIL_AXI_ID_ATTR ; k < FIL_END_ATTR ; k++) {
				t->per_trace[i].filter_group[j].attrs[k] =
					&t->per_trace[i].filter_attrs[k].attr;
				sysfs_attr_init
				(&t->per_trace[i].filter_attrs[k].attr);
			}

			/* NULL terminate the group */
			t->per_trace[i].filter_group[j].attrs[FIL_END_ATTR] =
									NULL;

			/* Create the filter group */
			retval = sysfs_create_group(
					t->per_trace[i].filterdir_kobj[j],
					&t->per_trace[i].filter_group[j]);
			if (retval) {
				dev_err(t->dev, "Fls group creation failed\n");
				goto out2;
			}
		}

		t->per_trace[i].trace_regs =
			ioremap_nocache(sources[i].resource->start,
					resource_size(sources[i].resource));
		if (!t->per_trace[i].trace_regs) {
			dev_err(t->dev, "ioremap failed for %s\n",
					sources[i].name);
			return -ENOMEM;
		}
		i++;
	}
	t->trace_src_count = i;

	return 0;
out2:
	for ( ; i >= 0 ; i--) {
		for (j = 0 ; j < sources[i].filter_count ; j++) {
			if (t->per_trace[i].filterdir_kobj[j])
				kobject_put(t->per_trace[i].filterdir_kobj[j]);
				kfree(t->per_trace[i].filter_group[j].attrs);
		}
		if (t->per_trace[i].dir_kobj)
			kobject_put(t->per_trace[i].dir_kobj);
		kfree(t->per_trace[i].attr_group.attrs);
		kfree(t->per_trace[i].filterdir_kobj);
		kfree(t->per_trace[i].filter_group);
		kfree(t->per_trace[i].state.fls_state);
	}
out1:
	kfree(t->per_trace);
out:
	amba_set_drvdata(dev, NULL);
	return retval;
}

static int axitrace_remove(struct amba_device *dev)
{
	struct complete_trace_src_info *t = &tracer;
	struct axitrace_source *sources;
	int i, j;

	for (i = 0 ; i < t->trace_src_count ; i--) {
		sources = t->per_trace[i].p_source_info;

		for (j = 0 ; j < sources[i].filter_count ; j++) {
				kobject_put(t->per_trace[i].filterdir_kobj[j]);
				kfree(t->per_trace[i].filter_group[j].attrs);
		}

		kobject_put(t->per_trace[i].dir_kobj);
		kfree(t->per_trace[i].attr_group.attrs);
		kfree(t->per_trace[i].filterdir_kobj);
		kfree(t->per_trace[i].filter_group);
		kfree(t->per_trace[i].state.fls_state);
	}
	kfree(t->per_trace);

	amba_set_drvdata(dev, NULL);

	return 0;
}
static struct amba_id axitrace_ids[] = {
	{
		.id	= 0x0003c921,
		.mask	= 0x0007ffff,
	},
	{ 0, 0},
};

static struct amba_driver axitrace_driver = {
	.drv		= {
		.name	= "axitrace",
		.owner	= THIS_MODULE,
	},
	.probe		= axitrace_probe,
	.remove		= axitrace_remove,
	.id_table	= axitrace_ids,
};

static int __init axitrace_init(void)
{
	int retval;

	retval = amba_driver_register(&axitrace_driver);
	if (retval) {
		pr_err("Failed to register axitrace driver\n");
		return retval;
	}

	return 0;
}
device_initcall(axitrace_init);

static void __exit axitrace_exit(void)
{
	amba_driver_unregister(&axitrace_driver);
}
module_exit(axitrace_exit);
