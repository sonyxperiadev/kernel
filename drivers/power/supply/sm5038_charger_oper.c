// SPDX-License-Identifier: GPL-2.0-only
/*
 * sm5038-charger.c - SM5038 Charger operation mode control module.
 *
 */

#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/power/sm5038_charger.h>


/**
 *
 *  (VBUS in/out) (WPC in/out) (USB-OTG in/out) (Power Sharing cable in/out)
 *  (5V TX-mode on/off) (9V TX-mode on/off) (flash on/off) (Torch on/off) (suspend in/out)
 **/
#define make_OP_STATUS(vbus, wpcin, tx_5v, tx_9v, otg, flash, torch, suspend) ( \
												((vbus & 0x1)      << SM5038_CHARGER_OP_EVENT_VBUSIN)      | \
												((wpcin & 0x1)     << SM5038_CHARGER_OP_EVENT_WPCIN)       | \
												((tx_5v & 0x1)     << SM5038_CHARGER_OP_EVENT_5V_TX)       | \
												((tx_9v & 0x1)     << SM5038_CHARGER_OP_EVENT_9V_TX)       | \
												((otg & 0x1)       << SM5038_CHARGER_OP_EVENT_USB_OTG)     | \
												((flash & 0x1)     << SM5038_CHARGER_OP_EVENT_FLASH)       | \
												((torch & 0x1)     << SM5038_CHARGER_OP_EVENT_TORCH)	   | \
												((suspend & 0x1)   << SM5038_CHARGER_OP_EVENT_SUSPEND))


struct sm5038_charger_oper_table_info {
	unsigned short status;
	unsigned char oper_mode;
	unsigned char BST_OUT;
	unsigned char OTG_CURRENT;
	unsigned char TX_CURRENT;
};

struct sm5038_charger_oper_info {
	struct i2c_client *i2c;
	struct mutex op_mutex;
	int max_table_num;
	int table_index;
	struct sm5038_charger_oper_table_info current_table;
};
static struct sm5038_charger_oper_info *oper_info;

/**
 *
 *  (VBUS in/out) (WPCIN in/out) (TX_5V in/out) (TX_9V in/out)
 *  (USB-OTG in/out) (Flash on/off) (Torch on/off) (Suspend mode on/off)
 **/
static struct sm5038_charger_oper_table_info sm5038_charger_op_mode_table[] = {

	/* ****************** Buck Mode ****************** */
	// Charger=ON Mode in a valid VBUS
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	{ make_OP_STATUS(0, 0, 0, 0, 0, 0, 0, 0), OP_MODE_CHG_ON_VBUS,     BSTOUT_4400mV, OTG_CURRENT_500mA, TX_CURRENT_200mA}, /* (0) DEFAULT */
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	{ make_OP_STATUS(0, 0, 0, 0, 0, 0, 0, 0), OP_MODE_SUSPEND,         BSTOUT_5100mV, OTG_CURRENT_500mA, TX_CURRENT_200mA}, /* (0) DEFAULT */
#endif

	{ make_OP_STATUS(1, 0, 0, 0, 0, 0, 0, 0), OP_MODE_CHG_ON_VBUS,     BSTOUT_4400mV, OTG_CURRENT_500mA, TX_CURRENT_200mA}, /* (1) VBUS */
	{ make_OP_STATUS(1, 0, 0, 0, 0, 0, 1, 0), OP_MODE_CHG_ON_VBUS,     BSTOUT_4400mV, OTG_CURRENT_500mA, TX_CURRENT_200mA}, /* (2) VBUS + TORCH */
//	{ make_OP_STATUS(1, 0, 0, 0, 0, 1, 0, 0), OP_MODE_CHG_ON_VBUS,     BSTOUT_4400mV, OTG_CURRENT_500mA, TX_CURRENT_200mA}, /* (3) VBUS + FLASH (not used : not chg on vbus but flash boost mode) */

	// TX Mode in a valid VBUS
	{ make_OP_STATUS(1, 0, 1, 0, 0, 0, 0, 0), OP_MODE_TX_MODE_VBUS,     BSTOUT_5100mV, OTG_CURRENT_900mA, TX_CURRENT_1600mA}, /* (4) VBUS + 5V TX */
	{ make_OP_STATUS(1, 0, 1, 0, 0, 0, 1, 0), OP_MODE_TX_MODE_VBUS,     BSTOUT_5100mV, OTG_CURRENT_900mA, TX_CURRENT_1600mA}, /* (5) VBUS + 5V TX + TORCH */
	{ make_OP_STATUS(1, 0, 1, 0, 0, 1, 0, 0), OP_MODE_TX_MODE_VBUS,     BSTOUT_5100mV, OTG_CURRENT_900mA, TX_CURRENT_1600mA}, /* (6) VBUS + 5V TX + FLASH */
	{ make_OP_STATUS(1, 0, 0, 1, 0, 0, 0, 0), OP_MODE_TX_MODE_VBUS,     BSTOUT_9000mV, OTG_CURRENT_900mA, TX_CURRENT_1600mA}, /* (7) VBUS + 9V TX */
//	{ make_OP_STATUS(1, 0, 0, 1, 0, 0, 1, 0), OP_MODE_TX_MODE_VBUS,     BSTOUT_9000mV, OTG_CURRENT_900mA, TX_CURRENT_1600mA}, /* (8) VBUS + 9V TX + TORCH (not used : midvbus is lower than 6.8V) */
//	{ make_OP_STATUS(1, 0, 0, 1, 0, 1, 0, 0), OP_MODE_TX_MODE_VBUS,     BSTOUT_9000mV, OTG_CURRENT_900mA, TX_CURRENT_1600mA}, /* (9) VBUS + 9V TX + FLASH (not used : midvbus is lower than 6.8V) */

	// Charger=ON Mode over WPCIN
	{ make_OP_STATUS(0, 1, 0, 0, 0, 0, 0, 0), OP_MODE_CHG_ON_WPCIN,     BSTOUT_4400mV, OTG_CURRENT_500mA, TX_CURRENT_200mA}, /* (10) WPCIN 5V or 9V */
	{ make_OP_STATUS(0, 1, 0, 0, 0, 0, 1, 0), OP_MODE_CHG_ON_WPCIN,     BSTOUT_4400mV, OTG_CURRENT_500mA, TX_CURRENT_200mA}, /* (11) WPCIN 5V + TORCH */
	{ make_OP_STATUS(0, 1, 0, 0, 0, 1, 0, 0), OP_MODE_CHG_ON_WPCIN,     BSTOUT_4400mV, OTG_CURRENT_500mA, TX_CURRENT_200mA}, /* (12) WPCIN 5V + FLASH */

	// USB OTG Mode and in a vaild WPCIN
	{ make_OP_STATUS(0, 1, 0, 0, 1, 0, 0, 0), OP_MODE_WPC_OTG_CHG_ON,     BSTOUT_5100mV, OTG_CURRENT_900mA, TX_CURRENT_1600mA}, /* (13) WPCIN + OTG */
	{ make_OP_STATUS(0, 1, 0, 0, 1, 0, 1, 0), OP_MODE_WPC_OTG_CHG_ON,     BSTOUT_5100mV, OTG_CURRENT_900mA, TX_CURRENT_1600mA}, /* (14) WPCIN + OTG + TORCH */
	{ make_OP_STATUS(0, 1, 0, 0, 1, 1, 0, 0), OP_MODE_WPC_OTG_CHG_ON,     BSTOUT_5100mV, OTG_CURRENT_900mA, TX_CURRENT_1600mA}, /* (15) WPCIN + OTG + FLASH */

	/* ****************** Boost Mode ******************* */
	// Flash Boost Mode
	{ make_OP_STATUS(0, 0, 0, 0, 0, 0, 1, 0), OP_MODE_FLASH_BOOST,     BSTOUT_5100mV, OTG_CURRENT_900mA, TX_CURRENT_200mA},	/* (19) TORCH */
//	{ make_OP_STATUS(1, 0, 0, 0, 0, 0, 1, 0), OP_MODE_FLASH_BOOST,     BSTOUT_5100mV, OTG_CURRENT_900mA, TX_CURRENT_1600mA},	/* (20) TORCH + VBUS */
//	{ make_OP_STATUS(0, 1, 0, 0, 0, 0, 1, 0), OP_MODE_FLASH_BOOST,     BSTOUT_5100mV, OTG_CURRENT_900mA, TX_CURRENT_1600mA},	/* (21) TORCH + WPCIN */
	{ make_OP_STATUS(0, 0, 0, 0, 0, 1, 0, 0), OP_MODE_FLASH_BOOST,     BSTOUT_5100mV, OTG_CURRENT_900mA, TX_CURRENT_200mA},	/* (22) FLASH */
//	{ make_OP_STATUS(1, 0, 0, 0, 0, 1, 0, 0), OP_MODE_FLASH_BOOST,     BSTOUT_5100mV, OTG_CURRENT_900mA, TX_CURRENT_200mA},	/* (23) FLASH + VBUS */
	{ make_OP_STATUS(0, 1, 0, 0, 0, 1, 0, 0), OP_MODE_FLASH_BOOST,     BSTOUT_5100mV, OTG_CURRENT_900mA, TX_CURRENT_200mA},	/* (24) FLASH + WPCIN */
// 	{ make_OP_STATUS(0, 0, 0, 0, 0, 1, 1, 0), OP_MODE_FLASH_BOOST,     BSTOUT_5100mV, OTG_CURRENT_900mA, TX_CURRENT_1600mA},	/* (25) TORCH + FLASH */

	// TX Mode in NO valid VBUS
	{ make_OP_STATUS(0, 0, 1, 0, 0, 0, 0, 0), OP_MODE_TX_MODE_NOVBUS,     BSTOUT_5100mV, OTG_CURRENT_500mA, TX_CURRENT_1600mA}, /* (26) 5V TX */
	{ make_OP_STATUS(0, 0, 1, 0, 0, 0, 1, 0), OP_MODE_TX_MODE_NOVBUS,     BSTOUT_5100mV, OTG_CURRENT_500mA, TX_CURRENT_1600mA}, /* (27) 5V TX + TORCH */
	{ make_OP_STATUS(0, 0, 1, 0, 0, 1, 0, 0), OP_MODE_TX_MODE_NOVBUS,     BSTOUT_5100mV, OTG_CURRENT_500mA, TX_CURRENT_1600mA}, /* (28) 5V TX + FLASH */
	{ make_OP_STATUS(0, 0, 0, 1, 0, 0, 0, 0), OP_MODE_TX_MODE_NOVBUS,     BSTOUT_9000mV, OTG_CURRENT_500mA, TX_CURRENT_1000mA}, /* (29) 9V TX */

	// USB OTG Mode
	{ make_OP_STATUS(0, 0, 0, 0, 1, 0, 0, 0), OP_MODE_USB_OTG_MODE,     BSTOUT_5100mV, OTG_CURRENT_900mA, TX_CURRENT_400mA}, /* (30) OTG */
	{ make_OP_STATUS(0, 0, 0, 0, 1, 0, 1, 0), OP_MODE_USB_OTG_MODE,     BSTOUT_5100mV, OTG_CURRENT_900mA, TX_CURRENT_400mA}, /* (31) OTG + TORCH */
	{ make_OP_STATUS(0, 0, 0, 0, 1, 1, 0, 0), OP_MODE_USB_OTG_MODE,     BSTOUT_5100mV, OTG_CURRENT_900mA, TX_CURRENT_400mA}, /* (32) OTG + FLASH */

	// USB OTG and TX Mode
	{ make_OP_STATUS(0, 0, 1, 0, 1, 0, 0, 0), OP_MODE_USB_OTG_TX_MODE,     BSTOUT_5100mV, OTG_CURRENT_1500mA, TX_CURRENT_400mA}, /* (36) 5V TX + OTG */
	{ make_OP_STATUS(0, 0, 1, 0, 1, 0, 1, 0), OP_MODE_USB_OTG_TX_MODE,     BSTOUT_5100mV, OTG_CURRENT_1500mA, TX_CURRENT_400mA}, /* (37) 5V TX + OTG + TORCH */
	{ make_OP_STATUS(0, 0, 1, 0, 1, 1, 0, 0), OP_MODE_USB_OTG_TX_MODE,     BSTOUT_5100mV, OTG_CURRENT_1500mA, TX_CURRENT_400mA}, /* (38) 5V TX + OTG + FLASH */

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	/* ****************** Suspend Mode ****************** */
	{ make_OP_STATUS(0, 0, 0, 0, 0, 0, 0, 1), OP_MODE_SUSPEND,     BSTOUT_5100mV, OTG_CURRENT_500mA, TX_CURRENT_200mA},      /* (42) Reserved position of SUSPEND mode table */
#endif

};

static int set_OP_MODE(struct i2c_client *i2c, u8 mode)
{
	return sm5038_update_reg(i2c, SM5038_CHG_REG_CNTL2, (mode << 0), (0xF << 0));
}

static int set_BSTOUT(struct i2c_client *i2c, u8 bstout)
{
	return sm5038_update_reg(i2c, SM5038_CHG_REG_BSTCNTL1, (bstout << 0), (0x1F << 0));
}

static int set_OTG_CURRENT(struct i2c_client *i2c, u8 otg_curr)
{
	return sm5038_update_reg(i2c, SM5038_CHG_REG_BSTCNTL1, (otg_curr << 6), (0x3 << 6));
}

static int set_TX_CURRENT(struct i2c_client *i2c, u8 tx_curr)
{
	return sm5038_update_reg(i2c, SM5038_CHG_REG_BSTCNTL2, (tx_curr << 7), (0x7 << 0));
}


static inline int change_op_table(unsigned short new_status)
{
	int i = 0;

	pr_info("sm5038-charger: %s: Old table[%d] info (STATUS: 0x%x, MODE: 0x%x, BST_OUT: 0x%x, OTG_CURRENT: 0x%x, TX_CURRENT: 0x%x)\n",
			__func__, oper_info->table_index, oper_info->current_table.status, oper_info->current_table.oper_mode,
			oper_info->current_table.BST_OUT, oper_info->current_table.OTG_CURRENT, oper_info->current_table.TX_CURRENT);

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	/* Check actvated Suspend Mode */
	if (new_status & (0x1 << SM5038_CHARGER_OP_EVENT_SUSPEND)) {
		i = oper_info->max_table_num - 1;    /* Reserved SUSPEND Mode Table index */
	} else {
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	{
#endif
		/* Search matched Table */
		for (i = 0; i < oper_info->max_table_num; ++i) {
			if (new_status == sm5038_charger_op_mode_table[i].status) {
				break;
			}
		}
	}
	if (i == oper_info->max_table_num) {
		pr_err("sm5038-charger: %s: can't find matched charger op_mode table (status = 0x%x)\n", __func__, new_status);
		return -EINVAL;
	}

    /* Update current table info */
	if (sm5038_charger_op_mode_table[i].BST_OUT != oper_info->current_table.BST_OUT) {
		set_BSTOUT(oper_info->i2c, sm5038_charger_op_mode_table[i].BST_OUT);
		oper_info->current_table.BST_OUT = sm5038_charger_op_mode_table[i].BST_OUT;
	}

	if (sm5038_charger_op_mode_table[i].OTG_CURRENT != oper_info->current_table.OTG_CURRENT) {
		set_OTG_CURRENT(oper_info->i2c, sm5038_charger_op_mode_table[i].OTG_CURRENT);
		oper_info->current_table.OTG_CURRENT = sm5038_charger_op_mode_table[i].OTG_CURRENT;
	}

	if (sm5038_charger_op_mode_table[i].TX_CURRENT != oper_info->current_table.TX_CURRENT) {
		set_TX_CURRENT(oper_info->i2c, sm5038_charger_op_mode_table[i].TX_CURRENT);
		oper_info->current_table.TX_CURRENT = sm5038_charger_op_mode_table[i].TX_CURRENT;
	}

	if (sm5038_charger_op_mode_table[i].oper_mode != oper_info->current_table.oper_mode) {
		set_OP_MODE(oper_info->i2c, sm5038_charger_op_mode_table[i].oper_mode);
		oper_info->current_table.oper_mode = sm5038_charger_op_mode_table[i].oper_mode;
	}
	oper_info->current_table.status = new_status;
	oper_info->table_index = i;

	pr_info("sm5038-charger: %s: New table[%d] info (STATUS: 0x%x, MODE: 0x%x, BST_OUT: 0x%x, OTG_CURRENT: 0x%x, TX_CURRENT: 0x%x)\n",
			__func__, oper_info->table_index, oper_info->current_table.status, oper_info->current_table.oper_mode,
			oper_info->current_table.BST_OUT, oper_info->current_table.OTG_CURRENT, oper_info->current_table.TX_CURRENT);

	return 0;
}

static inline unsigned char update_status(int event_type, bool enable)
{
	if (event_type > SM5038_CHARGER_OP_EVENT_VBUSIN) {
		pr_err("sm5038-charger: %s: invalid event type (type=0x%x)\n", __func__, event_type);
		return oper_info->current_table.status;
	}

	if (enable) {
		return (oper_info->current_table.status | (1 << event_type));
	} else {
		return (oper_info->current_table.status & ~(1 << event_type));
	}
}

int sm5038_charger_oper_push_event(int event_type, bool enable)
{
	unsigned char new_status;
    int ret = 0;

	if (oper_info == NULL) {
		pr_err("sm5038-charger: %s: required init op_mode table\n", __func__);
		return -ENOENT;
	}
	pr_info("sm5038-charger: %s: event_type=%d, enable=%d\n", __func__, event_type, enable);

	mutex_lock(&oper_info->op_mutex);

	new_status = update_status(event_type, enable);
    if (new_status == oper_info->current_table.status) {
		goto out;
	}
    ret = change_op_table(new_status);

out:
	mutex_unlock(&oper_info->op_mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(sm5038_charger_oper_push_event);

static inline int detect_initial_table_index(struct i2c_client *i2c)
{
    return 0;
}
int sm5038_charger_oper_table_init(struct sm5038_dev *sm5038)
{
	struct i2c_client *i2c = sm5038->charger_i2c;

	int index = 0;

	if (oper_info) {
		pr_info("sm5038-charger: %s: already initialized\n", __func__);
		return 0;
	}

	if (i2c == NULL) {
		pr_err("sm5038-charger: %s: invalid i2c client handler=n", __func__);
		return -EINVAL;
	}

	oper_info = kmalloc(sizeof(struct sm5038_charger_oper_info), GFP_KERNEL);
	if (oper_info == NULL) {
		pr_err("sm5038-charger: %s: failed to alloctae memory\n", __func__);
		return -ENOMEM;
	}
	oper_info->i2c = i2c;

	mutex_init(&oper_info->op_mutex);

	/* set default operation mode condition */
	oper_info->max_table_num = ARRAY_SIZE(sm5038_charger_op_mode_table);
	index = detect_initial_table_index(oper_info->i2c);
	oper_info->table_index = index;
	oper_info->current_table.status = sm5038_charger_op_mode_table[index].status;
	oper_info->current_table.oper_mode = sm5038_charger_op_mode_table[index].oper_mode;
	oper_info->current_table.BST_OUT = sm5038_charger_op_mode_table[index].BST_OUT;
	oper_info->current_table.OTG_CURRENT = sm5038_charger_op_mode_table[index].OTG_CURRENT;
	oper_info->current_table.TX_CURRENT = sm5038_charger_op_mode_table[index].TX_CURRENT;

	set_OP_MODE(oper_info->i2c, oper_info->current_table.oper_mode);
	set_BSTOUT(oper_info->i2c, oper_info->current_table.BST_OUT);
	set_OTG_CURRENT(oper_info->i2c, oper_info->current_table.OTG_CURRENT);
	set_TX_CURRENT(oper_info->i2c, oper_info->current_table.TX_CURRENT);

	pr_info("sm5038-charger: %s: current table[%d] info (STATUS: 0x%x, MODE: 0x%x, BST_OUT: 0x%x, OTG_CURRENT: 0x%x, TX_CURRENT: 0x%x)\n", \
			__func__, oper_info->table_index, oper_info->current_table.status, oper_info->current_table.oper_mode, \
			oper_info->current_table.BST_OUT, oper_info->current_table.OTG_CURRENT, oper_info->current_table.TX_CURRENT);

	return 0;
}
EXPORT_SYMBOL_GPL(sm5038_charger_oper_table_init);

int sm5038_charger_oper_get_current_status(void)
{
	if (oper_info == NULL) {
		pr_err("sm5038-charger: %s: SM5038 charger is not initialized yet.\n", __func__);
		return -EINVAL;
	}
	return oper_info->current_table.status;
}
EXPORT_SYMBOL_GPL(sm5038_charger_oper_get_current_status);

int sm5038_charger_oper_get_current_op_mode(void)
{
	if (oper_info == NULL) {
		pr_err("sm5038-charger: %s: SM5038 charger is not initialized yet.\n", __func__);
		return -EINVAL;
	}
	return oper_info->current_table.oper_mode;
}
EXPORT_SYMBOL_GPL(sm5038_charger_oper_get_current_op_mode);

int sm5038_charger_oper_get_wpc_voltage(void)
{
	int mA;
	u8 reg;

	if (oper_info == NULL) {
		pr_err("sm5038-charger: %s: SM5038 charger is not initialized yet.\n", __func__);
		return -EINVAL;
	}

	sm5038_update_reg(oper_info->i2c, SM5038_CHG_REG_WPCINCNTL2, (0x1 << 7), (0x1 << 7)); 		/* WPC_READ = enable */

	sm5038_read_reg(oper_info->i2c, SM5038_CHG_REG_WPC_VOLTAGE, &reg);
	mA = (reg & 0xff) * 100;	/* 0.1V step */

	pr_info("sm5038-charger: %s read wpc voltage = %d mV \n", __func__, mA);

	return mA;
}
EXPORT_SYMBOL_GPL(sm5038_charger_oper_get_wpc_voltage);

int sm5038_charger_oper_get_vsys_voltage(void)
{
	u8 reg_h, reg_l;
	unsigned int vsys_mv;

	if (oper_info == NULL) {
		pr_err("sm5038-charger: %s: SM5038 charger is not initialized yet.\n", __func__);
		return -EINVAL;
	}

	sm5038_read_reg(oper_info->i2c, SM5038_CHG_REG_VOL_VSYS_H, &reg_h); /* vsys_h */
	sm5038_read_reg(oper_info->i2c, SM5038_CHG_REG_VOL_VSYS_L, &reg_l); /* vsys_l */

	vsys_mv = ((((reg_h & 0x3f) << 8) | reg_l) * 1000) / 2048;
	pr_info("sm5038-charger: %s read vsys = %d mV \n", __func__, vsys_mv);

	return vsys_mv;
}
EXPORT_SYMBOL_GPL(sm5038_charger_oper_get_vsys_voltage);

int sm5038_charger_oper_get_input_current_ua(void)
{
	u8 reg;
	int ma = 0;

	if (oper_info == NULL) {
		pr_err("sm5038-charger: %s: SM5038 charger is not initialized yet.\n", __func__);
		return -EINVAL;
	}

	sm5038_read_reg(oper_info->i2c, SM5038_CHG_REG_VBUSCNTL, &reg);
	ma = ((reg & 0x7F) * 25) + 100;

	return ma;
}
EXPORT_SYMBOL_GPL(sm5038_charger_oper_get_input_current_ua);

MODULE_LICENSE("GPL");
