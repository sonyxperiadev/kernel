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
#ifndef __AXITRACE_H__
#define __AXITRACE_H__
#include <linux/io.h>

struct funnel_map {
	void __iomem	*addr;
	unsigned char	bit;
	unsigned char	polarity;
};

#define AXI_FUNNEL(xaddr, xbit, xpol)		\
	{					\
		.addr		= (void *)xaddr,\
		.bit		= xbit,		\
		.polarity	= xpol,		\
	}

struct axi_master {
	char	*name;
	u8	enabled;
	u32	axi_id;
	u32	mask;
};

struct axi_slave {
	char	*name;
	u32	start_addr;
	u32	end_addr;
};

struct axitrace_source {
	char			*name;
	unsigned long		cap;
	unsigned long		filters;
	int			filter_count;
	struct axi_master	*masters;
	struct axi_slave	*slaves;
	struct funnel_map	*map;
	unsigned long		axi_id_mask;
	u8			all_masters_enabled;
	u8			filters_enabled;
	struct resource		*resource;
};

struct axitrace_platform_data {
	struct axitrace_source *sources;
};

enum {
FLS_READ = 0,
FLS_WRITE,
FLS_LEN_MODE,
FLS_LEN,
FLS_ID_MASK,
FLS_ID,
FLS_OPEN,
FLS_SECURE,
FLS_ADDR_LOW,
FLS_ADDR_HIGH,
FLS_END,
};

struct filter_state {
	u32 read:1;
	u32 write:1;
	u32 len_mode:2;
	u32 filter_len:4;
	u32 id_mask:8;
	u32 filter_id:13;
	u32 open:1;
	u32 secure:1;
	u32 addr_low;
	u32 addr_high;
};

/*
 * AXI Trace state
 */
struct trace_state {
	struct filter_state	*fls_state;
	u32			cap_state;
	bool			running;
};

enum {
CAP_ATTR = 0,
CUR_CONFIG_ATTR,
COUNTER_ATTR,
MASTER_ATTR,
SLAVE_ATTR,
CONTROL_ATTR,
TRACE_END_ATTR,
};

enum {
FIL_AXI_ID_ATTR = 0,
FIL_ID_MASK_ATTR,
FIL_MASTER_ATTR,
FIL_SLAVE_ATTR,
FIL_OPEN_ATTR,
FIL_SECURE_ATTR,
FIL_READ_ATTR,
FIL_WRITE_ATTR,
FIL_END_ATTR
};

struct per_trace_info {
	struct axitrace_source	*p_source_info;
	struct trace_state	state;
	void __iomem		*trace_regs;
	char			*name;
	struct kobject		*dir_kobj;
	struct kobject		**filterdir_kobj;
	struct kobj_attribute	trace_attrs[TRACE_END_ATTR];
	struct kobj_attribute	filter_attrs[FIL_END_ATTR];
	struct attribute_group	attr_group;
	struct attribute_group	*filter_group;
	u32	driver_cap;
};

struct complete_trace_src_info {
	struct per_trace_info	*per_trace;
	int			trace_src_count;
	struct device		*dev;
};

#define	CNTR_SAT_ENABLE		BIT(1)
#define	TRACE_ENABLE		BIT(3)
#define	COMMANDS_COUNTING	BIT(4)
#define	COMMANDS_FILTER		BIT(5)
#define	BEATS_COUNTING		BIT(6)
#define	BEATS_FILTER		BIT(7)
#define	BUSY_COUNTING		BIT(8)
#define	BUSY_FILTER		BIT(9)
#define	LATENCY_MEASUREMENT	BIT(10)
#define	LATENCY_FILTER		BIT(11)
#define	RD_LATENCY_MODE		BIT(12)
#define	OUTS_COUNTING		BIT(14)
#define	OUTS_FILTER		BIT(15)
#define	OUTS_THRES		BIT(16)
#define	OUTS_THRES_POS		16
#define	OUTS_THRES_VAL(x)	(x << OUTS_THRES_POS)
#define	MAX_OUTS_THRES		7

#define AXITRACE_FULL_CAP	(CNTR_SAT_ENABLE | TRACE_ENABLE		|\
				COMMANDS_COUNTING | COMMANDS_FILTER	|\
				BEATS_COUNTING | BEATS_FILTER		|\
				BUSY_COUNTING | BUSY_FILTER		|\
				LATENCY_MEASUREMENT | LATENCY_FILTER	|\
				RD_LATENCY_MODE | OUTS_COUNTING		|\
				OUTS_FILTER)

#define	FILTER_READ		BIT(0)
#define	FILTER_WRITE		BIT(1)
#define	FILTER_LEN_MODE		BIT(2)
#define	FILTER_LEN		BIT(4)
#define	FILTER_ID_MASK		BIT(8)
#define	FILTER_ID		BIT(16)
#define	FILTER_SECURE		BIT(30)
#define	FILTER_OPEN		BIT(29)

#define	FLS_LEN_MODE_WIDTH	0x3
#define	FLS_LEN_WIDTH		0x7
#define	FLS_ID_MASK_WIDTH	0xFF
#define	FLS_ID_WIDTH		0x3FFF

#define AXITRACE_ALL_FILTERS	(FILTER_READ | FILTER_WRITE | \
				FILTER_OPEN | FILTER_SECURE)

#define	AXI_MASTER(xname, id)	\
	{	.name	= xname,	\
		.axi_id	= id,		\
	}

#define	AXI_SLAVE(xname, start, end)	\
	{	.name	= xname,	\
		.start_addr = start,	\
		.end_addr = end		\
	}

#define	ATM_CONFIG	0x0
#define	ATM_STATUS	0x4
#define	ATM_OUTIDS	0x8
#define	ATM_CMD		0xc
#define	ATM_WRCMDS	0x10
#define	ATM_RDCMDS	0x14
#define	ATM_AWCYCLES	0x18
#define	ATM_ARCYCLES	0x1c
#define	ATM_WCYCLES	0x20
#define	ATM_RCYCLES	0x24
#define	ATM_BCYCLES	0x28
#define	ATM_AWBUSY	0X2C
#define	ATM_ARBUSY	0X30
#define	ATM_WBUSY	0X34
#define	ATM_RBUSY	0X38
#define	ATM_BBUSY	0X3C
#define	ATM_WRSUM	0X40
#define	ATM_RDSUM	0X44
#define	ATM_WRMIN	0X48
#define	ATM_RDMIN	0X4C
#define	ATM_WRMAX	0X50
#define	ATM_RDMAX	0x54
#define	ATM_WRBEATS	0x58
#define	ATM_RDBEATS	0x5c
#define	ATM_WROUTS	0x60
#define	ATM_RDOUTS	0x64
#define	ATM_FILTER_BASE	0x80
#define	ATM_ADDRLOW_BASE 0x84
#define	ATM_ADDRLOW_0	0x84
#define	ATM_ADDRHIGH_BASE 0x88
#define	ATM_ADDRHIGH_0	0x88
#define	ATM_FILTER_1	0x90
#define	ATM_ADDRLOW_1	0x94
#define	ATM_ADDRHIGH_1	0x98

/* Trace control values */
#define	TRACE_CNTR_CLEAR	0
#define	TRACE_CNTR_STOP		1
#define	TRACE_CNTR_START	2

#define	axi_writel(t, v, x)	\
	do {\
		__raw_writel(v, (t)->trace_regs + (x));		  \
		dev_dbg(tracer.dev, "value = 0x%x addr = 0x%x\n", \
				v, (u32)((t)->trace_regs + (x))); \
	} while (0);

#define	axi_readl(t, x)	(__raw_readl(t)->trace_regs + (x))

#define SHIFT(x)	(ffs(x) - 1)

#endif /* __AXITRACE_H__ */
