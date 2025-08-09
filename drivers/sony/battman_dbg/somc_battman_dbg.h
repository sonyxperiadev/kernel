// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2019-2020, The Linux Foundation. All rights reserved.
 */

#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include <linux/mutex.h>
#include <linux/spmi.h>
#include <linux/power_supply.h>
#include <linux/soc/qcom/pmic_glink.h>
#include <linux/soc/qcom/battery_charger.h>

#define MSG_OWNER_OEM			32782
#define MSG_OWNER_REG_DUMP		32783
#define MSG_TYPE_REQ_RESP		1
#define MSG_BATT_MNGR_GET_ULOG_REQ	0x0018
#define MSG_BATT_MNGR_GET_PMIC_REG_REQ	0x0037

#define MAX_ULOG_READ_BUFFER_SIZE	8192
#define MAX_REG_DUMP_SIZE		256

#define GLINK_WAIT_TIME_MS		1000
#define BMLOG_DELAY_TIME_MS		5000
#define BMREG_READ_DELAY_TIME_MS	1000

#define MSG_BATT_MNGR_SET_LOGGING_PROP_REQ	0x0019

#define BMLOG_DELAY_TIME_MS_DEFAULT		0

#define FIRST_LETTER				20
#define BUFFER_SIZE				256

enum battman_ulog_level_type {
	BATTMAN_LOG_LEVEL_NONE	= 0,
	BATTMAN_LOG_LEVEL_ERR	= 1,
	BATTMAN_LOG_LEVEL_WARN	= 2,
	BATTMAN_LOG_LEVEL_INFO	= 3,
	BATTMAN_LOG_LEVEL_DEBUG	= 4,
	BATTMAN_LOG_LEVEL_ALL_LOGS
};
#define BMLOG_LEVEL_DEFAULT	BATTMAN_LOG_LEVEL_INFO

enum _BATTMAN_LOG_CATEGORY_BITMAP {
	BattMngrWPP_SelfHost		= 1ULL << 0,
	BattMngrWPP_device		= 1ULL << 1,
	BattMngrWPP_ioimpl		= 1ULL << 2,
	PmicWPP_device			= 1ULL << 3,
	BattMngrWPP_None		= 1ULL << 4,
	BattMngrWPP_SMChgFGGge		= 1ULL << 5,
	BattMngrWPP_Platform		= 1ULL << 6,
	BattMngrWPP_IRQ			= 1ULL << 7,
	BattMngrWPP_notify		= 1ULL << 8,
	BattMngrWPP_statemachine	= 1ULL << 9,
	BattMngrWPP_statemachinelevel	= 1ULL << 10,
	BattMngrWPP_BattErrHandle	= 1ULL << 11,
	BattMngrWPP_FGGge		= 1ULL << 12,
	BattMngrWPP_ParallelCharging	= 1ULL << 13,
	BattMngrWPP_SMChg		= 1ULL << 14,
	BattMngrWPP_SWJEITA		= 1ULL << 15,
	BattMngrWPP_QBG			= 1ULL << 16,
	BattMngr_QBG			= 1ULL << 17,
	BattMngr_ADC			= 1ULL << 18,
	BattMngr_CAD			= 1ULL << 19,
	BattMngr_PmicPlatform		= 1ULL << 20,
	BattMngr_PmicHal		= 1ULL << 21,
	BattMngr_Aggregator		= 1ULL << 22,
	BattMngr_Summary		= 1ULL << 23,
	USBC_Common			= 1ULL << 24,
	USBC_DPM			= 1ULL << 25,
	USBC_LPM			= 1ULL << 26,
	USBC_PRL			= 1ULL << 27,
	USBC_PE				= 1ULL << 28,
	USBC_TCPC			= 1ULL << 29,
	USBC_TCPCPlatform		= 1ULL << 30,
	USBC_Summary			= 1ULL << 31,
	BattMngr_INIT			= 1ULL << 32,
	BattMngr_ULOG			= 1ULL << 33,
	BattMngr_ICMLOG			= 1ULL << 34,
	BattMngr_SSDEVLOG		= 1ULL << 35,
	USBC_ULOG			= 1ULL << 36,
	/*
	 * This block is used for important log for us
	 * (i.e. detection, charge curve...)
	 */
	BattMngr_SOMC			= 1ULL << 40,
	USBC_SOMC			= 1ULL << 41,
	/*
	 * This block is used for SOMC functions
	 * (i.e. stepcharge, softcharge...)
	 */
	BattMngr_SOMC_STEP		= 1ULL << 46,
	BattMngr_SOMC_ICM		= 1ULL << 47,
	Battman_LogCategoryMax		= 1ULL << 63,
};
#define BMLOG_CATEGORIES_DEFAULT	(BattMngr_SOMC | USBC_SOMC)

struct glink_ulog_req_msg {
	struct pmic_glink_hdr	hdr;
	u32			max_logsize;
};

struct glink_ulog_prop_req_msg {
	struct pmic_glink_hdr	hdr;
	u64			categories;
	u32			level;
};

struct glink_ulog_resp_msg {
	struct pmic_glink_hdr	hdr;
	char read_buf[MAX_ULOG_READ_BUFFER_SIZE];
};

struct reg_dump_read_req_msg {
	struct pmic_glink_hdr	hdr;
	u32			spmi_bus_id;
	u32			pmic_sid;
	u32			address;
	u32			byte_count;
};

struct reg_dump_read_resp_msg {
	struct pmic_glink_hdr	hdr;
	u32			spmi_bus_id;
	u32			pmic_sid;
	u32			address;
	u32			byte_count;
	u8			data[MAX_REG_DUMP_SIZE];
};

struct somc_bmdbg_dev {
	struct device			*dev;
	struct class			bmdbg_class;

	struct pmic_glink_client	*glink_client;
	atomic_t			glink_state;

	struct delayed_work		somc_bmdbg_log_request_work;

	int				bmlog_timer;

	u64				bmlog_categories;
	u32				bmlog_level;

	/* reg dump */
	struct completion		reg_read_ack;
	struct mutex			reg_read_lock;
	struct delayed_work		somc_bmdbg_reg_read_request_work;
	int				regupdate_timer;
	u8				chgr_reg[MAX_REG_DUMP_SIZE];
	u8				dcdc_reg[MAX_REG_DUMP_SIZE];
	u8				batif_reg[MAX_REG_DUMP_SIZE];
	u8				usb_reg[MAX_REG_DUMP_SIZE];
	u8				wls_reg[MAX_REG_DUMP_SIZE];
	u8				typec_reg[MAX_REG_DUMP_SIZE];
	u8				misc_reg[MAX_REG_DUMP_SIZE];
};
