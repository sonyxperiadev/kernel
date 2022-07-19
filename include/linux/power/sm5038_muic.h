/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2021 SiliconMitus
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef __SM5038_MUIC_H__
#define __SM5038_MUIC_H__

#define MUIC_DEV_NAME	"sm5038-muic"

/* SM5038 MUIC attached device type */
typedef enum {
	ATTACHED_DEV_NONE = 0,
	ATTACHED_DEV_USB  = 1,
	ATTACHED_DEV_CDP  = 2,
	ATTACHED_DEV_DCP  = 3,
	ATTACHED_DEV_HVDCP = 4,
	ATTACHED_DEV_LOTA = 5,
	ATTACHED_DEV_JTAG = 6,
	ATTACHED_DEV_DCDTIMEOUT = 7,
	ATTACHED_DEV_NUM,
} sm5038_muic_attached_dev_t;

/* SM5038 MUIC Interrupt 2 register */
#define INT2_VBUS_UPDATE_SHIFT		2
#define INT2_VBUS_UPDATE_MASK		(0x1 << INT2_VBUS_UPDATE_SHIFT)

/* SM5038 MUIC Device Type 1 register */
#define DEV_TYPE1_LO_TA			(0x1 << 7)
#define DEV_TYPE1_DCP2			(0x1 << 4)
#define DEV_TYPE1_CDP			(0x1 << 3)
#define DEV_TYPE1_DCP			(0x1 << 2)
#define DEV_TYPE1_SDP			(0x1 << 1)
#define DEV_TYPE1_DCD_OUT_SDP	(0x1 << 0)

/* SM5038 MUIC Device Type 2 register */
#define DEV_TYPE2_DEBUG_JTAG		(0x1 << 7)
#define DEV_TYPE2_HVDCP				(0x1 << 6)

/* SM5038 CTRL2 register */
#define REG_CTRL2_DPDMCNTL		6
#define REG_CTRL2_VBUS_READ		3

enum {
	MUIC_INTR_DETACH	= 0,
	MUIC_INTR_ATTACH	= 1,
};

enum bc12_cntl{
	BC12_DISABLE	= 0,
	BC12_ENABLE		= 1,
};

enum mansw_cntl{
	MANSW_AUTOMATIC		= 0,
	MANSW_MANUAL		= 1,
};

/*
 * Manual Switch
 * D- [5:3] / D+ [2:0]
 * 000: Open all / 001: USB / 010: JTAG / 011: UART
 */

#define MANUAL_SW_DM_SHIFT	3
#define MANUAL_SW_DP_SHIFT	0

#define MANUAL_SW_OPEN		(0x0)
#define MANUAL_SW_USB		(0x1 << MANUAL_SW_DM_SHIFT \
							| 0x1 << MANUAL_SW_DP_SHIFT)
#define MANUAL_SW_JTAG		(0x2 << MANUAL_SW_DM_SHIFT \
							| 0x2 << MANUAL_SW_DP_SHIFT)
#define MANUAL_SW_UART		(0x3 << MANUAL_SW_DM_SHIFT \
							| 0x3 << MANUAL_SW_DP_SHIFT)

enum sm5038_reg_manual_sw_value {
	MANSW_OPEN = (MANUAL_SW_OPEN),
	MANSW_USB  = (MANUAL_SW_USB),
	MANSW_JTAG = (MANUAL_SW_JTAG),
	MANSW_OTG  = (MANUAL_SW_USB),
	MANSW_UART = (MANUAL_SW_UART),
};

#define SM5038_MUIC_REG_CFG1		0x31
#define SM5038_MUIC_REG_CFG2		0x32
#define SM5038_MUIC_REG_VBUS		0x3B
#define SM5038_MUIC_REG_BCD_STATE	0x42
#define SM5038_MUIC_REG_OTP_IF_STS	0x58

#define SM5038_HVDCP_5V		5
#define SM5038_HVDCP_9V		9

struct muic_irq_t {
	/* int1 */
	int irq_dpdm_ovp;
	int irq_vbus_detach;
	int irq_chgtype_attach;
	int irq_dcdtimeout;
	/* int2 */
	int irq_hvdcp;
};

struct sm5038_muic_data {

	struct device *dev;
	struct i2c_client *i2c; /* i2c addr: 0x4A; MUIC */
	struct mutex muic_mutex;

	struct power_supply	*usb_port_psy;

	void *muic_data;

	/* muic current attached device */
	sm5038_muic_attached_dev_t attached_dev;

	/*struct muic_intr_data intr;*/
	struct muic_irq_t irqs;

	/* muic Device ID */
	unsigned char muic_vendor;		/* Vendor ID */

	/* model dependant mfd platform data */
	struct sm5038_platform_data	*mfd_pdata;

	int dev1;
	int dev2;

	int fled_torch_enable;
	int fled_flash_enable;
	int power_supply_cable_type;
	struct wakeup_source *wake_lock;

	struct delayed_work	muic_debug_work;
	struct delayed_work	muic_init_detect_work;
	struct delayed_work	muic_noti_work;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	/* CC reconnection */
	bool			cc_reconnection_running;

	/* debug */
	int			debug_mask;
	u8			dfs_reg_addr;
	bool			regdump_en;
#endif
};

int sm5038_muic_i2c_read_byte(struct i2c_client *client, unsigned char command);
int sm5038_muic_i2c_write_byte(struct i2c_client *client, unsigned char command, unsigned char value);

#endif /* __SM5038_MUIC_H__ */
