/* arch/arm/mach-msm/power_debug.c
 *
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * Author: Valentin Shevchenko <valentin.shevchenko@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/io.h>
#include <linux/mfd/pm8xxx/pm8921.h>
#include <linux/mfd/pm8xxx/pm8821.h>
#include <linux/mfd/pm8xxx/gpio.h>
#include <linux/mfd/pm8xxx/mpp.h>
#include <linux/msm_ssbi.h>
#include <mach/power_debug.h>
#include <mach/msm_iomap.h>
#include <mach/gpiomux.h>
#include "devices.h"

enum {
	DUMP_APQ_GPIO,
	DUMP_PM8921_GPIO,
	DUMP_PM8921_MPP,
	DUMP_PM8821_MPP,
	DUMP_DEV_NUM
};

/* PMIC definitions */
#define SSBI_REG_GPIO_BASE	0x150
#define SSBI_REG_MPP_BASE	0x050
#define PM_GPIO_BANKS		6
#define PM_GPIO_BANK_SHIFT	4

/* APQ definitions */
#define APQ_NR_GPIOS		90
#define APQ_GPIO_CFG(n)		(MSM_TLMM_BASE + 0x1000 + (0x10 * n))
#define APQ_GPIO_IN_OUT(n)	(MSM_TLMM_BASE + 0x1004 + (0x10 * n))


#define APQ_GPIO_PULL(x)	((x.ctrl) & 0x3)
#define APQ_GPIO_FUNC(x)	(((x.ctrl) >> 2) & 0xF)
#define APQ_GPIO_DRV(x)		(((x.ctrl) >> 6) & 0x7)
#define APQ_GPIO_OUT_EN(x)	(((x.ctrl) >> 9) & 0x1)

#define APQ_GPIO_IN_VAL(x)	((x.inout) & 0x1)
#define APQ_GPIO_OUT_VAL(x)	(((x.inout) >> 1) & 0x1)


typedef void (*store_func) (struct device *dev, void *data, size_t num);
typedef int (*show_func) (struct seq_file *m, void *data, size_t num);

static void apq_gpio_store(struct device *dev, void *data, size_t num);
static void pm8x_gpio_store(struct device *dev, void *data, size_t num);
static void pm8x_mpp_store(struct device *dev, void *data, size_t num);

static int apq_gpio_show(struct seq_file *m, void *data, size_t num);
static int pm8x_gpio_show(struct seq_file *m, void *data, size_t num);
static int pm8x_mpp_show(struct seq_file *m, void *data, size_t num);


/* APQ GPIO */
struct apq_gpio {
	uint32_t ctrl;
	uint32_t inout;
};

/* PMIC GPIO */
struct pm8x_gpio {
	/* Bank 0 */
	u8	modesel_ena:1;
	u8	vin_sel:3;
	u8	spare0:4;
	/* Bank 1 */
	u8	output_val:1;
	u8	output_buf:1;
	u8	mode:2;
	u8	spare1:4;
	/* Bank 2 */
	u8	spare2_1:1;
	u8	pull:3;
	u8	spare2_2:4;
	/* Bank 3 */
	u8	disable:1;
	u8	dtest_ena:1;
	u8	out_strength:2;
	u8	spare3:4;
	/* Bank 4 */
	u8	spare_4_1:1;
	u8	function:3;
	u8	spare4_2:4;
	/* Bank 5 */
	u8	spare_5_1:3;
	u8	inv_int_pol:1;
	u8	spare5_2:4;
};

/* PMIC MPP */
struct pm8x_mpp {
	u8	cfg2:2;
	u8	cfg1:3;
	u8	type:3;
};

/* Abstract dump target declaration */
struct dump_desc {
	const char	*name;
	store_func	store;
	show_func	show;
	struct device	*dev;
	size_t		item_size;
	size_t		item_count;
	void		*sleep_data;
};


/* Registers fields decoding arrays */

static const char *apq_pull_map[4] = {
	[GPIOMUX_PULL_NONE]	= "none",
	[GPIOMUX_PULL_DOWN]	= "pd",
	[GPIOMUX_PULL_KEEPER]	= "keeper",
	[GPIOMUX_PULL_UP]	= "pu",
};

static const char *pm8x_gpio_mode_map[4] = {
	[0] = "in",
	[1] = "inout",
	[2] = "out",
	[3] = "N/A",
};

static const char *pm8x_gpio_buf_map[2] = {
	[PM_GPIO_OUT_BUF_CMOS]		= "cmos",
	[PM_GPIO_OUT_BUF_OPEN_DRAIN]	= "od",
};

static const char *pm8x_gpio_pull_map[8] = {
	[PM_GPIO_PULL_UP_30]		= "pu_30uA",
	[PM_GPIO_PULL_UP_1P5]		= "pu_1.5uA",
	[PM_GPIO_PULL_UP_31P5]		= "pu_31.5uA",
	[PM_GPIO_PULL_UP_1P5_30]	= "pu_1.5+30uA",
	[PM_GPIO_PULL_DN]		= "pd_10uA",
	[PM_GPIO_PULL_NO]		= "nopull",
	[6]				= "N/A",
	[7]				= "N/A",
};

static const char *pm8x_gpio_drv_map[4] = {
	[PM_GPIO_STRENGTH_NO]	= "none",
	[PM_GPIO_STRENGTH_HIGH]	= "high",
	[PM_GPIO_STRENGTH_MED]	= "med",
	[PM_GPIO_STRENGTH_LOW]	= "low",
};

static const char *pm8x_gpio_func_map[8] = {
	[PM_GPIO_FUNC_NORMAL]	= "gpio",
	[PM_GPIO_FUNC_PAIRED]	= "paired",
	[PM_GPIO_FUNC_1]	= "func1",
	[PM_GPIO_FUNC_2]	= "func2",
	[PM_GPIO_DTEST1]	= "dtest1",
	[PM_GPIO_DTEST2]	= "dtest2",
	[PM_GPIO_DTEST3]	= "dtest3",
	[PM_GPIO_DTEST4]	= "dtest4",
};

static const char *pm8x_mpp_type_map[8] = {
	[PM8XXX_MPP_TYPE_D_INPUT]	= "d_in",
	[PM8XXX_MPP_TYPE_D_OUTPUT]	= "d_out",
	[PM8XXX_MPP_TYPE_D_BI_DIR]	= "d_bidir",
	[PM8XXX_MPP_TYPE_A_INPUT]	= "a_in",
	[PM8XXX_MPP_TYPE_A_OUTPUT]	= "a_out",
	[PM8XXX_MPP_TYPE_SINK]		= "sink",
	[PM8XXX_MPP_TYPE_DTEST_SINK]	= "dt_sink",
	[PM8XXX_MPP_TYPE_DTEST_OUTPUT]	= "dt_out",
};

static const char *pm8x_mpp_amux_map[8] = {
	[PM8XXX_MPP_AIN_AMUX_CH5]	= "amux5",
	[PM8XXX_MPP_AIN_AMUX_CH6]	= "amux6",
	[PM8XXX_MPP_AIN_AMUX_CH7]	= "amux7",
	[PM8XXX_MPP_AIN_AMUX_CH8]	= "amux8",
	[PM8XXX_MPP_AIN_AMUX_CH9]	= "amux9",
	[PM8XXX_MPP_AIN_AMUX_ABUS1]	= "abus1",
	[PM8XXX_MPP_AIN_AMUX_ABUS2]	= "abus2",
	[PM8XXX_MPP_AIN_AMUX_ABUS3]	= "abus3",
};

static const char *pm8x_mpp_aout_lvl_map[8] = {
	[PM8XXX_MPP_AOUT_LVL_1V25]	= "1v25",
	[PM8XXX_MPP_AOUT_LVL_1V25_2]	= "1v25_2",
	[PM8XXX_MPP_AOUT_LVL_0V625]	= "0v625",
	[PM8XXX_MPP_AOUT_LVL_0V3125]	= "0v3125",
	[PM8XXX_MPP_AOUT_LVL_MPP]	= "mpp",
	[PM8XXX_MPP_AOUT_LVL_ABUS1]	= "abus1",
	[PM8XXX_MPP_AOUT_LVL_ABUS2]	= "abus2",
	[PM8XXX_MPP_AOUT_LVL_ABUS3]	= "abus3",
};

static const char *pm8x_mpp_route_map[] = {
	[PM8XXX_MPP_DIN_TO_INT]		= "int",
	[PM8XXX_MPP_DIN_TO_DBUS1]	= "dbus1",
	[PM8XXX_MPP_DIN_TO_DBUS2]	= "dbus2",
	[PM8XXX_MPP_DIN_TO_DBUS3]	= "dbus3",
};

static const char *pm8x_mpp_output_map[4] = {
	[PM8XXX_MPP_DOUT_CTRL_LOW]	= "low",
	[PM8XXX_MPP_DOUT_CTRL_HIGH]	= "high",
	[PM8XXX_MPP_DOUT_CTRL_MPP]	= "mpp",
	[PM8XXX_MPP_DOUT_CTRL_INV_MPP]	= "inv_mpp",
};

static const char *pm8x_mpp_pull_map[4] = {
	[PM8XXX_MPP_BI_PULLUP_1KOHM]	= "0.6kOhm",
	[PM8XXX_MPP_BI_PULLUP_OPEN]	= "open",
	[PM8XXX_MPP_BI_PULLUP_10KOHM]	= "10kOhm",
	[PM8XXX_MPP_BI_PULLUP_30KOHM]	= "30kOhm",
};

static const char *pm8x_mpp_sink_out_map[4] = {
	[PM8XXX_MPP_CS_CTRL_DISABLE]		= "disabled",
	[PM8XXX_MPP_CS_CTRL_ENABLE]		= "enabled",
	[PM8XXX_MPP_CS_CTRL_MPP_HIGH_EN]	= "mpp_high_en",
	[PM8XXX_MPP_CS_CTRL_MPP_LOW_EN]		= "mpp_low_en",
};

/* Global static variables */

/* List of dump targets */
static struct dump_desc dump_devices[] = {
	[DUMP_APQ_GPIO] = {
		.name = "apq_gpio",
		.store = apq_gpio_store,
		.show = apq_gpio_show,
		.dev = NULL,
		.item_size = sizeof(struct apq_gpio),
		.item_count = APQ_NR_GPIOS,
	},
	[DUMP_PM8921_GPIO] = {
		.name = "pm8921_gpio",
		.store = pm8x_gpio_store,
		.show = pm8x_gpio_show,
		.dev = &apq8064_device_ssbi_pmic1.dev,
		.item_size = sizeof(struct pm8x_gpio),
		.item_count = PM8921_NR_GPIOS,
	},
	[DUMP_PM8921_MPP] = {
		.name = "pm8921_mpp",
		.store = pm8x_mpp_store,
		.show = pm8x_mpp_show,
		.dev = &apq8064_device_ssbi_pmic1.dev,
		.item_size = sizeof(struct pm8x_mpp),
		.item_count = PM8921_NR_MPPS,
	},
	[DUMP_PM8821_MPP] = {
		.name = "pm8821_mpp",
		.store = pm8x_mpp_store,
		.show = pm8x_mpp_show,
		.dev = &apq8064_device_ssbi_pmic2.dev,
		.item_size = sizeof(struct pm8x_mpp),
		.item_count = PM8821_NR_MPPS,
	},
};

static u32 debug_mask;
static bool sleep_saved;
static struct dentry *debugfs_base;

/* APQ8064 GPIO specific functions */
static void apq_gpio_store(struct device *unused, void *data, size_t num)
{
	int i;
	struct apq_gpio *gpios = (struct apq_gpio *)data;
	for (i = 0; i < num; i++) {
		gpios[i].ctrl = __raw_readl(APQ_GPIO_CFG(i)) & 0x3FF;
		gpios[i].inout = __raw_readl(APQ_GPIO_IN_OUT(i)) & 0x3;
	}
}

static int apq_gpio_show(struct seq_file *m, void *data, size_t num)
{
	int i = 0;
	struct apq_gpio *gpios = (struct apq_gpio *)data;

	seq_printf(m, "+--+-----+-----+-----+------+------+\n");
	seq_printf(m, "|# | dir | val | drv | func | pull |\n");
	seq_printf(m, "+--+-----+-----+-----+------+------+\n");
	for (i = 0; i < num ; i++) {
		seq_printf(m, "|%02u|%-5s|%-5u|%-2umA |%-6u|%-6s|\n", i,
			APQ_GPIO_OUT_EN(gpios[i]) ? "out" : "in",
			APQ_GPIO_OUT_EN(gpios[i]) ?
				APQ_GPIO_OUT_VAL(gpios[i]) :
				APQ_GPIO_IN_VAL(gpios[i]),
			APQ_GPIO_DRV(gpios[i]) * 2 + 2,
			APQ_GPIO_FUNC(gpios[i]),
			apq_pull_map[APQ_GPIO_PULL(gpios[i])]
			);
	}
	seq_printf(m, "+--+-----+-----+-----+------+------+\n");
	return 0;
}

/* PMIC GPIO specific functions */
static void pm8x_gpio_store(struct device *ssbi_dev, void *data, size_t num)
{
	int cur_gpio, cur_bank;
	struct pm8x_gpio *gpios = (struct pm8x_gpio *)data;

	for (cur_gpio = 0; cur_gpio < num ; cur_gpio++) {
		u8 *bank = (u8 *)&gpios[cur_gpio];
		for (cur_bank = 0; cur_bank < PM_GPIO_BANKS; cur_bank++) {
			bank[cur_bank] = cur_bank << PM_GPIO_BANK_SHIFT;
			msm_ssbi_write(ssbi_dev, SSBI_REG_GPIO_BASE + cur_gpio,
				&bank[cur_bank], 1);
			msm_ssbi_read(ssbi_dev, SSBI_REG_GPIO_BASE + cur_gpio,
				&bank[cur_bank], 1);
		}
	}
}

static int pm8x_gpio_show(struct seq_file *m, void *data, size_t num)
{
	int i = 0;
	struct pm8x_gpio *gpios = (struct pm8x_gpio *)data;

	seq_printf(m, "+--+-------+-----+----+---+-----------+----+---+"
		"------+\n");
	seq_printf(m, "|# | vin   |mode |buf |val|pull       |drv |ena|"
		"func  |\n");
	seq_printf(m, "+--+-------+-----+----+---+-----------+----+---+"
		"------+\n");

	for (i = 0; i < num ; i++) {
		seq_printf(m, "|%02u|vin_%u  |%-5s|%-4s|%-3s|%-11s|%-4s|"
			"%-3s|%-6s|\n", i+1,
			gpios[i].vin_sel,
			pm8x_gpio_mode_map[gpios[i].mode],
			pm8x_gpio_buf_map[gpios[i].output_buf],
			gpios[i].mode == 2 ?
				(gpios[i].output_val ? "1" : "0") : "-",
			pm8x_gpio_pull_map[gpios[i].pull],
			pm8x_gpio_drv_map[gpios[i].out_strength],
			gpios[i].disable ? "no" : "yes",
			pm8x_gpio_func_map[gpios[i].function]);
	}

	seq_printf(m, "+--+-------+-----+----+---+-----------+----+---+"
		"------+\n");
	return 0;
}

/* PMIC MPP specific functions */
static void pm8x_mpp_store(struct device *ssbi_dev, void *data, size_t num)
{
	int i;
	struct pm8x_mpp *mpps = (struct pm8x_mpp *)data;

	for (i = 0; i < num; i++) {
		msm_ssbi_read(ssbi_dev, SSBI_REG_MPP_BASE + i,
			(u8 *)&mpps[i], 1);
	}
}

static int pm8x_mpp_show(struct seq_file *m, void *data, size_t num)
{
	int i = 0;
	struct pm8x_mpp *mpps = (struct pm8x_mpp *)data;

	seq_printf(m, "+--+---------+--------------+--------------+\n");
	seq_printf(m, "|# | type    |     cfg1     |  cfg2        |\n");
	seq_printf(m, "+--+---------+--------------+--------------+\n");

	for (i = 0; i < num ; i++) {
		seq_printf(m, "|%02u|%-9s|", i+1,
			pm8x_mpp_type_map[mpps[i].type]);

		switch (mpps[i].type) {
		case PM8XXX_MPP_TYPE_D_INPUT:
			seq_printf(m, "vin_%-10u|%-14s|\n", mpps[i].cfg1,
				pm8x_mpp_route_map[mpps[i].cfg2]);
			break;
		case PM8XXX_MPP_TYPE_D_OUTPUT:
			seq_printf(m, "vin_%-10u|%-14s|\n", mpps[i].cfg1,
				pm8x_mpp_output_map[mpps[i].cfg2]);
			break;
		case PM8XXX_MPP_TYPE_D_BI_DIR:
			seq_printf(m, "vin_%-10u|%-14s|\n", mpps[i].cfg1,
				pm8x_mpp_pull_map[mpps[i].cfg2]);
			break;
		case PM8XXX_MPP_TYPE_A_INPUT:
			seq_printf(m, "%-14s|%-14s|\n",
				pm8x_mpp_amux_map[mpps[i].cfg1], "-");
			break;
		case PM8XXX_MPP_TYPE_A_OUTPUT:
			seq_printf(m, "vin_%-10u|%-14s|\n", mpps[i].cfg1,
				pm8x_mpp_aout_lvl_map[mpps[i].cfg2]);
			break;
		case PM8XXX_MPP_TYPE_SINK:
			seq_printf(m, "%-2umA          |%-14s|\n",
				5 + 5 * mpps[i].cfg1,
				pm8x_mpp_sink_out_map[mpps[i].cfg2]);
			break;
		default:
			seq_printf(m, "%-14s|%-14s|\n", "none", "none");
		}
	}

	seq_printf(m, "+--+---------+--------------+--------------+\n");

	return 0;
}

/* Generic functions */
static int dump_current(struct seq_file *m, void *unused)
{
	struct dump_desc *dump_device = (struct dump_desc *)m->private;
	void *data;

	data = kcalloc(dump_device->item_count, dump_device->item_size,
		GFP_KERNEL);
	if (!data)
		return -ENOMEM;
	dump_device->store(dump_device->dev, data, dump_device->item_count);
	dump_device->show(m, data, dump_device->item_count);
	kfree(data);
	return 0;
}

static int dump_sleep(struct seq_file *m, void *unused)
{
	struct dump_desc *dump_device = (struct dump_desc *)m->private;
	void *data;

	if (sleep_saved && dump_device->sleep_data)
		data = dump_device->sleep_data;
	else{
		seq_printf(m, "not recorded\n");
		return 0;
	}

	dump_device->show(m, data, dump_device->item_count);
	return 0;
}

static int current_open(struct inode *inode, struct file *file)
{
	return single_open(file, dump_current, inode->i_private);
}

static const struct file_operations current_fops = {
	.open		= current_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};

static int sleep_open(struct inode *inode, struct file *file)
{
	return single_open(file, dump_sleep, inode->i_private);
}

static const struct file_operations sleep_fops = {
	.open		= sleep_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};


static int enable_set(void *data, u64 val)
{
	int i;

	debug_mask = (u32)val;
	sleep_saved = false;

	for (i = 0 ; i < DUMP_DEV_NUM ; i++) {
		struct dump_desc *dump_device = &dump_devices[i];
		if (val) {
			if (dump_device->sleep_data)
				continue;
			dump_device->sleep_data = kcalloc(
				dump_device->item_count, dump_device->item_size,
				GFP_KERNEL);
			if (!dump_device->sleep_data)
				return -ENOMEM;
		} else {
			kfree(dump_device->sleep_data);
			dump_device->sleep_data = NULL;
		}
	}
	return 0;
}

static int enable_get(void *data, u64 *val)
{
	*val = (u64)debug_mask;
	sleep_saved = false;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(enable_fops, enable_get,
			enable_set, "0x%08llx\n");

/* Init functions */
static int __init populate(struct dentry * base, const char* dir,
	struct dump_desc *dump_device)
{
	struct dentry *local_base;

	local_base = debugfs_create_dir(dir, base);
	if (!local_base)
		return -ENOMEM;

	if (!debugfs_create_file("current", S_IRUGO, local_base,
			(void *)dump_device, &current_fops))
		return -ENOMEM;

	if (!debugfs_create_file("sleep", S_IRUGO, local_base,
			(void *)dump_device, &sleep_fops))
		return -ENOMEM;

	return 0;
}

static int __init power_debug_init(void)
{
	int ret = 0;
	int i;

	debugfs_base = debugfs_create_dir("power_debug", NULL);
	if (!debugfs_base)
		return -ENOMEM;

	if (!debugfs_create_file("enable", S_IRUGO | S_IWUSR, debugfs_base,
				NULL, &enable_fops)){
		ret = -ENOMEM;
		goto fail;
	}

	for (i = 0 ; i < DUMP_DEV_NUM ; i++) {
		struct dump_desc *dump_device = &dump_devices[i];
		ret = populate(debugfs_base, dump_device->name,
			dump_device);
		if (ret)
			goto fail;
	}

	pr_info("power_debug init Ok\n");
	return 0;
fail:
	debugfs_remove_recursive(debugfs_base);
	pr_err("power_debug init failed\n");
	return ret;
}

/* Public functions */

void power_debug_collapse(void)
{
	int i;
	if (debug_mask) {
		pr_info("%s save sleep state\n", __func__);
		for (i = 0 ; i < DUMP_DEV_NUM ; i++) {
			struct dump_desc *dump_device = &dump_devices[i];
			if (dump_device->sleep_data)
				dump_device->store(dump_device->dev,
					dump_device->sleep_data,
					dump_device->item_count);
		}
		sleep_saved = true;
	}
}

late_initcall(power_debug_init);
