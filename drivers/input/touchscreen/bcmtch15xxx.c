/*
 * Copyright 2013 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * A copy of the GPL is available at
 * http://www.broadcom.com/licenses/GPLv2.php, or by writing to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/freezer.h>
#include <linux/proc_fs.h>
#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/kfifo.h>
#include <linux/version.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/firmware.h>
#include <linux/input/mt.h>
#include <linux/time.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#ifdef CONFIG_REGULATOR
#include <linux/regulator/consumer.h>
#endif


#include <linux/timer.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#include <linux/i2c/bcmtch15xxx.h>
#include <linux/i2c/bcmtch15xxx_settings.h>
#undef CONFIG_OF

/* -------------------------------------- */
/* - BCM Touch Controller Driver Macros - */
/* -------------------------------------- */

/* -- driver version -- */
#define	BCMTCH_DRIVER_VERSION		"1.4.4.00"
#define	BCMTCH_DRIVER_BUILD_DATE	__DATE__
#define	BCMTCH_DRIVER_BUILD_TIME	__TIME__

/* -- SPM addresses -- */
#define BCMTCH_SPM_REG_REVISIONID   0x40
#define BCMTCH_SPM_REG_CHIPID0      0x41
#define BCMTCH_SPM_REG_CHIPID1      0x42
#define BCMTCH_SPM_REG_CHIPID2      0x43

#define BCMTCH_SPM_REG_SPI_I2C_SEL  0x44
#define BCMTCH_SPM_REG_I2CS_CHIPID  0x45

#define BCMTCH_SPM_REG_PSR          0x48

#define BCMTCH_SPM_REG_MSG_FROM_HOST    0x49
#define BCMTCH_SPM_REG_MSG_FROM_HOST_1  0x4a
#define BCMTCH_SPM_REG_MSG_FROM_HOST_2  0x4b
#define BCMTCH_SPM_REG_MSG_FROM_HOST_3  0x4c
#define BCMTCH_SPM_REG_RQST_FROM_HOST   0x4d
#define BCMTCH_SPM_REG_MSG_TO_HOST      0x4e

#define BCMTCH_SPM_REG_PMU_CONTROL2 0x52

#define BCMTCH_SPM_REG_SOFT_RESETS  0x59
#define BCMTCH_SPM_REG_FLL_STATUS   0x5c

#define BCMTCH_SPM_REG_ALFO_CTRL    0x60
#define BCMTCH_SPM_REG_LPLFO_CTRL   0x61

#define BCMTCH_SPM_REG_DMA_ADDR     0x80
#define BCMTCH_SPM_REG_DMA_STATUS   0x89
#define BCMTCH_SPM_REG_DMA_WFIFO    0x92
#define BCMTCH_SPM_REG_DMA_RFIFO    0xa2

/* -- SYS addresses -- */
#define BCMTCH_ADDR_BASE                    0x30000000

#define BCMTCH_ADDR_SPM_BASE                (BCMTCH_ADDR_BASE + 0x00100000)
#define BCMTCH_ADDR_SPM_PWR_CTRL            (BCMTCH_ADDR_SPM_BASE + 0x1c)
#define BCMTCH_ADDR_SPM_LPLFO_CTRL_RO       (BCMTCH_ADDR_SPM_BASE + 0xa0)
#define BCMTCH_ADDR_SPM_REMAP               (BCMTCH_ADDR_SPM_BASE + 0x100)
#define BCMTCH_ADDR_SPM_STICKY_BITS         (BCMTCH_ADDR_SPM_BASE + 0x144)

#define BCMTCH_ADDR_COMMON_BASE             (BCMTCH_ADDR_BASE + 0x00110000)
#define BCMTCH_ADDR_COMMON_ARM_REMAP        (BCMTCH_ADDR_COMMON_BASE + 0x00)
#define BCMTCH_ADDR_COMMON_SYS_HCLK_CTRL    (BCMTCH_ADDR_COMMON_BASE + 0x20)
#define BCMTCH_ADDR_COMMON_CLOCK_ENABLE     (BCMTCH_ADDR_COMMON_BASE + 0x48)
#define BCMTCH_ADDR_COMMON_FLL_CTRL0        (BCMTCH_ADDR_COMMON_BASE + 0x104)
#define BCMTCH_ADDR_COMMON_FLL_LPF_CTRL2    (BCMTCH_ADDR_COMMON_BASE + 0x114)
#define BCMTCH_ADDR_COMMON_FLL_TEST_CTRL1   (BCMTCH_ADDR_COMMON_BASE + 0x144)

#define BCMTCH_ADDR_TCH_BASE                (BCMTCH_ADDR_BASE + 0x00300000)
#define BCMTCH_ADDR_TCH_VER                 (BCMTCH_ADDR_TCH_BASE + 0x00)

/* -- SYS MEM addresses -- */
#define BCMTCH_ADDR_VECTORS     0x00000000
#define BCMTCH_ADDR_CODE        0x10000000
#define BCMTCH_ADDR_DATA        0x10009000
#define BCMTCH_ADDR_TOC_BASE    0x0020c000

/* -- guards -- */
#define BCMTCH_STATE_PROTOCOL	1

/* -- constants -- */
#define BCMTCH_MAX_TOUCH	10
#define BCMTCH_MAX_BUTTONS	16

#define BCMTCH_AXIS_MAX	4095

#define BCMTCH_AXIS_SHIFT_BITS	4

#define BCMTCH_MAX_PRESSURE		500
#define BCMTCH_MIN_ORIENTATION	-512
#define BCMTCH_MAX_ORIENTATION	511

#define BCMTCH_DMA_MODE_READ    1
#define BCMTCH_DMA_MODE_WRITE   3

#define BCMTCH_IF_I2C_SEL       0
#define BCMTCH_IF_SPI_SEL       1

#define BCMTCH_DEFAULT_I2C_ADDR_SYS	0x68

#define BCMTCH_IF_I2C_COMMON_CLOCK  0x387B
#define BCMTCH_IF_SPI_COMMON_CLOCK  0x387F

#define BCMTCH_COMMON_CLOCK_USE_FLL (0x1 << 18)

#define BCMTCH_POWER_STATE_SLEEP        0
#define BCMTCH_POWER_STATE_RETENTION    1
#define BCMTCH_POWER_STATE_IDLE         3
#define BCMTCH_POWER_STATE_ACTIVE       4

#define BCMTCH_POWER_MODE_SLEEP     0x01
#define BCMTCH_POWER_MODE_WAKE      0x02
#define BCMTCH_POWER_MODE_NOWAKE    0x00

#define BCMTCH_PMU_CNTL2_DLDO_1_1V  0x08

#define BCMTCH_RESET_MODE_SOFT_CLEAR    0x00
#define BCMTCH_RESET_MODE_SOFT_CHIP     0x01
#define BCMTCH_RESET_MODE_SOFT_ARM      0x02
#define BCMTCH_RESET_MODE_HARD          0x04

#define BCMTCH_MEM_REMAP_ADDR		BCMTCH_ADDR_SPM_REMAP

#define BCMTCH_MEM_ROM_BOOT 0x00
#define BCMTCH_MEM_RAM_BOOT 0x01
#define BCMTCH_MEM_MAP_1000 0x00
#define BCMTCH_MEM_MAP_3000 0x02

#define	BCMTCH_FW_READY_WAIT	1000

#define BCMTCH_SPM_STICKY_BITS_PIN_RESET    0x02

/* development and test */
#define	BCMPFX	"BCMTCH:"

#define	BCMTCH_USE_PRINTK	0

#if BCMTCH_USE_PRINTK
#define	BCMTCH_INFO(fmt, arg...)	printk(BCMPFX fmt, ##arg)

#define	BCMTCH_ERR(fmt, arg...)		printk(BCMPFX fmt, ##arg)

#define	BCMTCH_DBG(flag, fmt, arg...)	\
	{if (bcmtch_debug_flag & flag) printk(BCMPFX fmt, ##arg); }

#else
#define	BCMTCH_INFO(fmt, arg...)		pr_info(BCMPFX fmt, ##arg)

#define	BCMTCH_ERR(fmt, arg...)			pr_err(BCMPFX fmt, ##arg)

#define	BCMTCH_DBG(flag, fmt, arg...)	pr_debug(BCMPFX fmt, ##arg)
#endif

#define PROGRESS() (BCMTCH_INFO("%s : %d\n", __func__, __LINE__))

/* -------------------------------------- */
/* - Touch Firmware Environment (ToFE)  - */
/* -------------------------------------- */

#define TOFE_MESSAGE_SUCCESS						0x12
#define TOFE_MESSAGE_SOC_REBOOT_PENDING				0x16
#define TOFE_MESSAGE_COMMAND_ECHO					0x20
#define TOFE_MESSAGE_FW_READY						0x80
#define TOFE_MESSAGE_FW_READY_INTERRUPT				0x81
#define TOFE_MESSAGE_FW_READY_OVERRIDE				0x82
#define TOFE_MESSAGE_FW_READY_INTERRUPT_OVERRIDE	0x83

enum tofe_command {
	TOFE_COMMAND_NO_COMMAND = 0,
	TOFE_COMMAND_INTERRUPT_ACK,
	TOFE_COMMAND_SCAN_START,
	TOFE_COMMAND_SCAN_STOP,
	TOFE_COMMAND_SCAN_SET_RATE,
	TOFE_COMMAND_SET_MODE,
	TOFE_COMMAND_CALIBRATE,
	TOFE_COMMAND_AFEREGREAD,
	TOFE_COMMAND_AFEREGWRITE,
	TOFE_COMMAND_RUN_SEM,
	TOFE_COMMAND_SET_LOG_MASK,
	TOFE_COMMAND_SET_LOG_MASK_FWID_ALL,
	TOFE_COMMAND_GET_LOG_MASK,
	TOFE_COMMAND_INIT_POST_BOOT_PATCHES,
	TOFE_COMMAND_SET_POWER_MODE,
	TOFE_COMMAND_POWER_MODE_SUSPEND,
	TOFE_COMMAND_POWER_MODE_RESUME,
	TOFE_COMMAND_HOST_OVERRIDE_REQ,
	TOFE_COMMAND_HOST_OVERRIDE_REL,
	TOFE_COMMAND_REBOOT_APPROVED,

	TOFE_COMMAND_LAST,
	TOFE_COMMAND_MAX = 0xff
};

enum bcmtch_status {
	BCMTCH_STATUS_SUCCESS	= 0,	/* Success */
	BCMTCH_STATUS_ERR_FAIL,			/* Generic failure */
	BCMTCH_STATUS_ERR_NOMEM,		/* Memory error */
	BCMTCH_STATUS_ERR_NOARG,		/* No proper arguments */
	BCMTCH_STATUS_ERR_BADARG,		/* Bad argument */
	BCMTCH_STATUS_ERR_TOUT,			/* Timeout */
	BCMTCH_STATUS_ERR_IO,			/* I/O error */
	BCMTCH_STATUS_ERR_NOCFG,		/* No configuration */
	BCMTCH_STATUS_ERR_NOCHN,		/* No required channel */
};

/**
	@ struct tofe_command_response
	@ brief Entry structure for command channel.
*/
struct tofe_command_response {
	uint8_t		flags;
	uint8_t		command;
	uint16_t	result;
	uint32_t	data;
};

#define	TOFE_COMMAND_FLAG_COMMAND_PROCESSED	(1 << 0)
#define	TOFE_COMMAND_FLAG_REQUEST_RESULT	(1 << 7)

/* ToFE Signature */

#define	TOFE_SIGNATURE_MAGIC_SIZE	8
#define TOFE_MAGIC {'B', 'C', 'M', 'N', 'A', 'P', 'A', '\0'}

struct tofe_version {
	uint8_t generation;
	uint8_t spin;
	uint8_t major;
	uint8_t minor;
};

enum tofe_chip_variant {
	TOFE_CHIP_VARIANT_INVALID,
	TOFE_CHIP_VARIANT_PHONE,
	TOFE_CHIP_VARIANT_TABLET,
};

/**
    @struct tofe_signature
    @brief Firmware ROM image signature structure.
*/
#pragma pack(push, 1)
struct  tofe_signature {
	char                 magic[TOFE_SIGNATURE_MAGIC_SIZE];
	struct tofe_version  version;
	uint64_t             commit;
	uint32_t             build;
	uint16_t             compatibility;
	uint8_t              variant; /* tofe_chip_variant */
	uint8_t              release_type;
	uint8_t              release_number;
	uint8_t              _pad;
	uint16_t             cust_release_num;
};
#pragma pack(pop)

#define TOFE_SIGNATURE_SIZE sizeof(struct tofe_signature)

/**
    @enum tofe_channel_flag
    @brief Channel flag field bit assignment.
*/
enum tofe_channel_flag {
	TOFE_CHANNEL_FLAG_STATUS_OVERFLOW      = 1 << 0,
	TOFE_CHANNEL_FLAG_STATUS_LEVEL_TRIGGER = 1 << 1,
	TOFE_CHANNEL_FLAG_FWDMA_BUFFER         = 1 << 3,
	TOFE_CHANNEL_FLAG_FWDMA_ENABLE         = 1 << 4,
	TOFE_CHANNEL_FLAG_INTERRUPT_ENABLE     = 1 << 5,
	TOFE_CHANNEL_FLAG_OVERFLOW_STALL       = 1 << 6,
	TOFE_CHANNEL_FLAG_INBOUND              = 1 << 7,
};

enum tofe_toc_index {
	TOFE_TOC_INDEX_CHANNEL = 2,
	TOFE_TOC_INDEX_TCH = 3,
	TOFE_TOC_INDEX_DETECT = 6,
};

enum tofe_channel_id {
	TOFE_CHANNEL_ID_TOUCH,
	TOFE_CHANNEL_ID_COMMAND,
	TOFE_CHANNEL_ID_RESPONSE,
	TOFE_CHANNEL_ID_LOG,
};

struct tofe_dmac_header {
	uint16_t			min_size;
	uint16_t			size;
};

struct tofe_channel_buffer_header {
	struct tofe_dmac_header	dmac;
	uint8_t				channel_id:4;
	uint8_t				flags:4;
	uint8_t				seq_number;
	uint8_t				entry_size;
	uint8_t				entry_count;
};

struct tofe_channel_buffer {
	struct tofe_channel_buffer_header	header;
	uint32_t						data[256];
};

struct tofe_channel_header {
	/* Channel ID */
	uint8_t		channel_id;
	/* Number of entries.  Limited to 255 entries. */
	uint8_t		entry_num;
	/* Entry size in bytes.  Limited to 255 bytes. */
	uint8_t		entry_size;
	/* Number of entries in channel to trigger notification */
	uint8_t		trig_level;
	/* Bit definitions shared with configuration. */
	uint8_t		flags;
	/* Number of datat buffers for this channel */
	uint8_t		buffer_num;
	/* Select the buffer to write [0 .. buffer_num-1]. */
	uint8_t		buffer_idx;
	/* Count the number of buffer swapped for debug. */
	uint8_t		seq_count;
	struct tofe_channel_buffer	*buffer[2];
};

struct tofe_channel_instance_cfg {
	uint8_t entry_num;	/* Must be > 0. */
	uint8_t entry_size;	/* Range [1..255]. */
	uint8_t trig_level;	/* 0 - entry_num */
	uint8_t flags;
	uint8_t buffer_num; /* Number of buffers for this channel */
	uint8_t _pad8;
	uint16_t offset;
	struct tofe_channel_header *channel_header;
	void *channel_data;
};

#pragma pack(push, 1)
struct mtc_detect_cfg {
	/* compressed structure definition */
	uint16_t    _pad[17];
	int16_t     scaling_x_offset;
	uint16_t    scaling_x_gain;
	uint16_t    scaling_x_range;
	int16_t     scaling_y_offset;
	uint16_t    scaling_y_gain;
	uint16_t    scaling_y_range;
	uint16_t    class_finger_gate;
	uint16_t    _pad0;
	uint16_t    class_stylus_gate;
};
#pragma pack(pop)


struct tofe_log_msg {
	uint16_t	log_code;
	uint16_t	param_0;
	uint32_t	params[2];
	uint32_t	timestamp;
};

struct combi_entry {
	uint32_t offset;
	uint32_t addr;
	uint32_t length;
	uint32_t flags;
};

/* ------------------------------------- */
/* - BCM Touch Controller Driver Enums - */
/* ------------------------------------- */

enum bcmtch_channel_id {
	/* NOTE : see above tofe_channel_id */
	BCMTCH_CHANNEL_TOUCH,
	BCMTCH_CHANNEL_COMMAND,
	BCMTCH_CHANNEL_RESPONSE,
	BCMTCH_CHANNEL_LOG,

	/* last */
	BCMTCH_CHANNEL_MAX
};

enum bcmtch_channel_set {
	BCMTCH_RAM_CHANNELS = 0,
	BCMTCH_ROM_CHANNELS = 1,
	BCMTCH_MAX_CHANNEL_SET
};

/* event kinds from BCM Touch Controller */
enum bcmtch_event_kind {
	BCMTCH_EVENT_KIND_RESERVED, /* Avoiding zero, but you may use it. */

	BCMTCH_EVENT_KIND_FRAME,
	BCMTCH_EVENT_KIND_TOUCH,
	BCMTCH_EVENT_KIND_TOUCH_END,
	BCMTCH_EVENT_KIND_BUTTON,
	BCMTCH_EVENT_KIND_GESTURE,

	BCMTCH_EVENT_KIND_EXTENSION = 7,
	BCMTCH_EVENT_KIND_MAX = BCMTCH_EVENT_KIND_EXTENSION,
};

enum _bcmtch_touch_status {
	BCMTCH_TOUCH_STATUS_INACTIVE,
	BCMTCH_TOUCH_STATUS_UP,
	BCMTCH_TOUCH_STATUS_MOVE,
	BCMTCH_TOUCH_STATUS_MOVING,
};

/* -------------------------------------- */
/* - BCM Touch Controller Device Tables - */
/* -------------------------------------- */

static const uint32_t const BCMTCH_CHIP_IDS[] = {
	0x15200,
	0x15300,
	0x15500,
};

/* ------------------------------------------ */
/* - BCM Touch Controller Driver Parameters - */
/* ------------------------------------------ */
#define BCMTCH_BF_HARD_RESET_ON_LOAD 0x00000001
#define BCMTCH_BF_SOFT_RESET_ON_LOAD 0x00000002
#define BCMTCH_BF_VERIFY_CHIP        0x00000004
#define BCMTCH_BF_DISABLE_POST_BOOT  0x00000008
#define BCMTCH_BF_CHECK_INTERRUPT    0x00000010
#define BCMTCH_BF_SUSPEND_COLD_BOOT  0x00000020
#define BCMTCH_BF_FW_RESET_ON_WD     0x00000040
#define BCMTCH_BF_STATE_PROTOCOL     0x00000080
#if BCMTCH_STATE_PROTOCOL
#define BCMTCH_BF_STATE_SHORT_SLOT   0x00000100
#define BCMTCH_BF_STATE_SYNC_MODE    0x00000200
#endif /* BCMTCH_STATE_PROTOCOL */

#define BCMTCH_BF_RESET_ON_LOAD_MASK \
			(BCMTCH_BF_HARD_RESET_ON_LOAD \
			| BCMTCH_BF_SOFT_RESET_ON_LOAD)

static int bcmtch_boot_flag =
	(BCMTCH_BF_SOFT_RESET_ON_LOAD
	| BCMTCH_BF_SUSPEND_COLD_BOOT
	| BCMTCH_BF_FW_RESET_ON_WD
#if BCMTCH_STATE_PROTOCOL
	| BCMTCH_BF_STATE_SYNC_MODE
#endif
	| BCMTCH_BF_VERIFY_CHIP
	);

module_param_named(boot_flag, bcmtch_boot_flag, int, S_IRUGO);
MODULE_PARM_DESC(boot_flag, "Boot bit-fields [RAM|RESET]");

/*-*/

#define BCMTCH_CHANNEL_FLAG_USE_TOUCH       0x00000001
#define BCMTCH_CHANNEL_FLAG_USE_CMD_RESP    0x00000002
#define BCMTCH_CHANNEL_FLAG_USE_LOG         0x00000004

static int bcmtch_channel_flag = BCMTCH_CHANNEL_FLAG_USE_TOUCH
			| BCMTCH_CHANNEL_FLAG_USE_CMD_RESP;

module_param_named(channel_flag, bcmtch_channel_flag, int, S_IRUGO);
MODULE_PARM_DESC(channel_flag, "Channels allowed bit-fields [L|C/R|T]");

/*-*/

#define BCMTCH_EVENT_FLAG_TOUCH_SIZE		0x00000001
#define BCMTCH_EVENT_FLAG_TOOL_SIZE			0x00000002
#define BCMTCH_EVENT_FLAG_PRESSURE			0x00000004
#define BCMTCH_EVENT_FLAG_ORIENTATION		0x00000008

static int bcmtch_event_flag = BCMTCH_EVENT_FLAG_TOUCH_SIZE
		| BCMTCH_EVENT_FLAG_ORIENTATION
		| BCMTCH_EVENT_FLAG_PRESSURE;

module_param_named(event_flag, bcmtch_event_flag, int, S_IRUGO);
MODULE_PARM_DESC(event_flag, "Extension events bit-fields [ORIEN|PRESSURE|TOOL_SIZE|TOUCH_SIZE]");

/*- firmware -*/

#define BCMTCH_FIRMWARE_FLAGS_CODE					0x0
#define BCMTCH_FIRMWARE_FLAGS_CONFIGS				0x01
#define BCMTCH_FIRMWARE_FLAGS_POST_BOOT				0x02
#define BCMTCH_FIRMWARE_FLAGS_POST_BOOT_CODE		0x02
#define BCMTCH_FIRMWARE_FLAGS_POST_BOOT_CONFIGS		0x03
#define BCMTCH_FIRMWARE_FLAGS_MASK					0x03
#define BCMTCH_FIRMWARE_FLAGS_ROM_BOOT				0x04
#define BCMTCH_FIRMWARE_FLAGS_POST_BOOT_PATCH		0x08

#define BCMTCH_FIRMWARE_FLAGS_COMBI     0x10

static int bcmtch_firmware_flag = BCMTCH_FIRMWARE_FLAGS_COMBI;

module_param_named(firmware_flag, bcmtch_firmware_flag, int, S_IRUGO);
MODULE_PARM_DESC(firmware_flag, "Firmware flag bit-fields (combi = 0x10  config = 0x01");

static char *bcmtch_firmware;

module_param_named(firmware, bcmtch_firmware, charp, S_IRUGO);
MODULE_PARM_DESC(firmware, "Filename of firmware to load");

static int bcmtch_firmware_addr = 0x0;

module_param_named(firmware_addr, bcmtch_firmware_addr, int, S_IRUGO);
MODULE_PARM_DESC(firmware_addr, "Address to load firmware");

/*- post boot -*/

#define	BCMTCH_POST_BOOT_RATE_HIGH	(1<<10)
#define	BCMTCH_POST_BOOT_RATE_LOW	80

static int bcmtch_post_boot_rate_high = BCMTCH_POST_BOOT_RATE_HIGH;

module_param_named(
			pbr_high,
			bcmtch_post_boot_rate_high,
			int,
			S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(pbr_high, "Post Boot Download Rate - High");

static int bcmtch_post_boot_rate_low = BCMTCH_POST_BOOT_RATE_LOW;

module_param_named(pbr_low, bcmtch_post_boot_rate_low, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(pbr_low, "Post Boot Download Rate - Low");

/*- watch dog duration -*/

/* x milliseconds in jiffies */
#define	BCMTCH_WATCHDOG_NORMAL		5000
#define	BCMTCH_WATCHDOG_POST_BOOT	50

static uint32_t bcmtch_watchdog_normal = BCMTCH_WATCHDOG_NORMAL;

module_param_named(
		wdg_normal,
		bcmtch_watchdog_normal,
		uint,
		S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(wdg_normal, "Watch dog rate in normal operation (ms)");

static uint32_t bcmtch_watchdog_post_boot = BCMTCH_WATCHDOG_POST_BOOT;

module_param_named(
		wdg_post_boot,
		bcmtch_watchdog_post_boot,
		uint,
		S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(wdg_post_boot, "Watch dog rate at post boot download (ms)");


/*- debug flag -*/

#define BCMTCH_DF_MV	0x00000001		/* touch MOVE event */
#define BCMTCH_DF_UP	0x00000002		/* touch UP event */
#define BCMTCH_DF_TE	0x00000004		/* touch extension events */
#define BCMTCH_DF_BT	0x00000008		/* BUTTON event */
#define BCMTCH_DF_CH	0x00000010		/* channel protocol */
#define BCMTCH_DF_ST	0x00000020		/* state protocol */
#define BCMTCH_DF_RC	0x00000040		/* rm */
#define BCMTCH_DF_RF	0x00000080		/* rf */
#define BCMTCH_DF_FR	0x00000100		/* frame events */
#define BCMTCH_DF_FE	0x00000200		/* frame extension events */
#define BCMTCH_DF_PB	0x00000400		/* post-boot */
#define BCMTCH_DF_DT	0x00000800		/* device tree */
#define BCMTCH_DF_HO	0x00001000		/* host override */
#define BCMTCH_DF_PM	0x00002000		/* power management */
#define BCMTCH_DF_WD	0x00004000		/* watch dog */
#define BCMTCH_DF_IH	0x00008000		/* interrupt */
#define BCMTCH_DF_I2C	0x00010000		/* i2c */
#define BCMTCH_DF_INFO	0x00020000		/* info */

static int bcmtch_debug_flag; /* = 0; */

module_param_named(debug_flag, bcmtch_debug_flag, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug_flag, "Debug Bit-Flag");

/* ---------------------------------------- */
/* - BCM Touch Controller Firmware Tables - */
/* ---------------------------------------- */

#define	BCMTCHWC	0x9999

struct firmware_load_info_t {
	uint32_t chip_id;
	uint32_t chip_rev;
	uint8_t *filename;
	uint32_t addr;
	uint32_t flags;
};

static const struct firmware_load_info_t BCMTCH_BINARIES[] = {

	/* MUST ALWAYS START WITH WILDCARD */
	{BCMTCHWC, BCMTCHWC, "bcmtchfw_bin", 0, BCMTCH_FIRMWARE_FLAGS_COMBI},

	/*
	* ADD CHIP SPECIFIC BINARIES HERE
	*/

	/* CHIP ID specific */
	{0x15200,
		BCMTCHWC,
		"bcmtchfw15200_bin",
		0,
		BCMTCH_FIRMWARE_FLAGS_COMBI},

	{0x15300,
		BCMTCHWC,
		"bcmtchfw15300_bin",
		0,
		BCMTCH_FIRMWARE_FLAGS_COMBI},

	/*
	** CHIP ID & CHIP REV specific
	*
	*/

};


/* ------------------------------------------ */
/* - BCM Touch Controller Driver Structures - */
/* ------------------------------------------ */

/*- work queue enum -*/

enum bcmtch_work_process_function {
	BCMTCH_WP_CHANNEL,
	BCMTCH_WP_PATCH_INIT,
	BCMTCH_WP_STATE,
	BCMTCH_WP_NUMBER
};

/*- bcmtch channel structures -*/

struct bcmtch_channel {
	struct tofe_channel_instance_cfg	cfg;
	struct tofe_channel_header		hdr;
	uint16_t					queued;
	uint8_t						active;
	/* intentional pad - may use for i2c tranactions */
	uint8_t						_pad8;
	uint32_t data;
};

/**
	@ struct bcmtch_response_wait
	@ brief storage of the results from response channel.
*/
struct bcmtch_response_wait {
	bool		wait;
	uint32_t	resp_data;
};

struct bcmtch_event {
	uint32_t           event_kind:3;
	uint32_t           _pad:29;
};

enum bcmtch_event_frame_extension_kind {
	BCMTCH_EVENT_FRAME_EXTENSION_KIND_TIMESTAMP,
	BCMTCH_EVENT_FRAME_EXTENSION_KIND_CHECKSUM,
	BCMTCH_EVENT_FRAME_EXTENSION_KIND_HEARTBEAT,

	BCMTCH_EVENT_FRAME_EXTENSION_KIND_MAX = 7  /* 3 bits */
};

struct bcmtch_event_frame {
	uint32_t           event_kind:3;
	uint32_t           _pad:1;
	uint32_t           frame_id:12;
	uint32_t           timestamp:16;
};

struct bcmtch_event_frame_extension {
	uint32_t           event_kind:3;
	uint32_t           frame_kind:3;
	uint32_t           _pad:26;
};

struct bcmtch_event_frame_extension_timestamp {
	uint32_t           event_kind:3;
	uint32_t           frame_kind:3;
	uint32_t           _pad:2;
	uint32_t           scan_end:8;
	uint32_t           mtc_start:8;
	uint32_t           mtc_end:8;
};

struct bcmtch_event_frame_extension_checksum {
	uint32_t           event_kind:3;
	uint32_t           frame_kind:3;
	uint32_t           _pad:10;
	uint32_t           hash:16;
};

struct bcmtch_event_frame_extension_heartbeat {
	uint32_t           event_kind:3;
	uint32_t           frame_kind:3;
	uint32_t           _pad:2;
	uint32_t           timestamp:24; /* 100 us units, free running */
};

enum bcmtch_event_touch_extension_kind {
	BCMTCH_EVENT_TOUCH_EXTENSION_KIND_DETAIL,
	BCMTCH_EVENT_TOUCH_EXTENSION_KIND_BLOB,
	BCMTCH_EVENT_TOUCH_EXTENSION_KIND_SIZE,
	BCMTCH_EVENT_TOUCH_EXTENSION_KIND_HOVER,
	BCMTCH_EVENT_TOUCH_EXTENSION_KIND_TOOL,

	BCMTCH_EVENT_TOUCH_EXTENSION_KIND_MAX = 7  /* 3 bits */
};

enum bcmtch_event_touch_tool {
	BCMTCH_EVENT_TOUCH_TOOL_FINGER,
	BCMTCH_EVENT_TOUCH_TOOL_STYLUS,
};

struct bcmtch_event_touch {
	uint32_t           event_kind:3;
	uint32_t           track_tag:5;
	uint32_t           x:12;
	uint32_t           y:12;
};

struct bcmtch_event_touch_end {
	uint32_t           event_kind:3;
	uint32_t           track_tag:5;
	uint32_t           _pad:24;
};

struct bcmtch_event_touch_extension {
	uint32_t           event_kind:3;
	uint32_t           touch_kind:3;
	uint32_t           _pad:26;
};

struct bcmtch_event_touch_extension_detail {
	uint32_t           event_kind:3;
	uint32_t           touch_kind:3;
	uint32_t           confident:1;
	uint32_t           suppressed:1;
	uint32_t           hover:1;
	uint32_t           tool:1;
	uint32_t           large_touch:1;
	uint32_t           _pad:1;
	uint32_t           pressure:8;
	uint32_t           orientation:12;
};

struct bcmtch_event_touch_extension_blob {
	uint32_t           event_kind:3;
	uint32_t           touch_kind:3;
	uint32_t           area:8;
	uint32_t           total_cap:18;
};

#define BCMTCH_EVENT_TOUCH_EXTENSION_AREA_MAX 255

struct bcmtch_event_touch_extension_size {
	uint32_t           event_kind:3;
	uint32_t           touch_kind:3;
	uint32_t           _pad:6;
	uint32_t           major_axis:10;
	uint32_t           minor_axis:10;
};

struct bcmtch_event_touch_extension_hover {
	uint32_t           event_kind:3;
	uint32_t           touch_kind:3;
	uint32_t           _pad:16;
	uint32_t           height:10;
};

struct bcmtch_event_touch_extension_tool {
	uint32_t           event_kind:3;
	uint32_t           touch_kind:3;
	uint32_t           _pad:6;
	uint32_t           width_major:10;
	uint32_t           width_minor:10;
};

enum bcmtch_event_button_kind {
	BCMTCH_EVENT_BUTTON_KIND_CONTACT,
	BCMTCH_EVENT_BUTTON_KIND_HOVER,

	BCMTCH_EVENT_BUTTON_KIND_MAX = 1
};

struct bcmtch_event_button {
	uint32_t           event_kind:3;
	uint32_t           button_kind:1;
	uint32_t           _pad:12;
	uint32_t           status:16;
};

/* driver structure for a single touch point */
struct bcmtch_touch {
	uint16_t	x;		/* X Coordinate */
	uint16_t	y;		/* Y Coordinate */
	uint16_t	major_axis;
	uint16_t	minor_axis;
	uint16_t	width_major;
	uint16_t	width_minor;
	uint8_t		type;
	uint8_t		pressure;
	int16_t		orientation;

	/* Touch status: Down, Move, Up (Inactive) */
	enum _bcmtch_touch_status	status;

	enum bcmtch_event_kind		event;	/* Touch Event Kind */
};


#if BCMTCH_STATE_PROTOCOL
/* ---------------------------- */
/* - State Protocol Constants - */
/* ---------------------------- */

enum bcmtch_state_protocol_cmd {
	BCMTCH_CMD_LONG = 0x20,
	BCMTCH_CMD_START_SCAN = 0x21,
	BCMTCH_CMD_STOP_SCAN = 0x22,
	BCMTCH_CMD_SUSPEND = 0x23,
	BCMTCH_CMD_RESUME = 0x24,
	BCMTCH_CMD_SLEEP = 0x28,
	BCMTCH_CMD_SINGLE_SCAN = 0x30,
	BCMTCH_CMD_READ_RAM = 0x31,
	BCMTCH_CMD_WRITE_RAM = 0x32,
};

enum bcmtch_state_slot_type {
	STATE_SLOT_TYPE_EMPTY = 0,
	STATE_SLOT_TYPE_BUTTON = 1,
	STATE_SLOT_TYPE_STYLUS = 2,
	STATE_SLOT_TYPE_FINGER = 8,
	STATE_SLOT_TYPE_RESERVED = 96,
};

#define TOFE_HOST_CMD_BUF_SIZE			    512
#define TOFE_HOST_CMD_SHORT_SIZE			4
#define TOFE_HOST_MAX_NUM_SLOTS			    15
#define TOFE_HOST_TOUCH_SLOT_SMALL_SIZE     4
#define TOFE_HOST_TOUCH_SLOT_BIG_SIZE	    8
#define TOFE_HOST_LOG_SLOT_SIZE			    12
#define TOFE_HOST_SLOT_AREA_MIN             1

#define TOFE_HOST_EXCEPTION_REGS	24

#define TOFE_HOST_RSP_BUF_SIZE \
			(BCMTCH_MAX_TOUCH \
			* TOFE_HOST_TOUCH_SLOT_BIG_SIZE + 1)

/*- State protocol RAM access configuration */

struct bcmtch_state_cfg_rw {
	uint16_t cfg_size;
	uint16_t max_read_size;
	uint16_t max_write_size;
	uint16_t patch_cfg_max_size;
	uint32_t patch_cfg_addr;
};

#pragma pack(push, 1)
struct bcmtch_state_rw_params {
	uint32_t addr;
	uint16_t len;
};
#pragma pack(pop)

/*- Touch scan command/response format. -*/

union bcmtch_state_cmd_start_scan {
	struct {
		uint32_t touch_slot_format:3;
		uint32_t sync_mode:1;
		uint32_t max_slot_number:4;
	};
	uint8_t reg;
};

union bcmtch_state_ret_start_scan {
	struct {
		uint32_t slots:4;
		uint32_t frame:3;
		uint32_t done:1;
	};
	uint8_t reg;
};

struct bcmtch_state_start_scan {
	union bcmtch_state_cmd_start_scan	cmd;
	union bcmtch_state_ret_start_scan	status;
	uint8_t	touch_slot_size;
	uint8_t	touch_slot_num;
	uint8_t	log_slot_num;
};

union bcmtch_state_resp_slot {
	uint8_t	type:7;
	uint8_t	frame:1;

	struct {
		uint32_t type:7;
		uint32_t frame:1;
		uint32_t x:12;
		uint32_t y:12;
		uint32_t pressure:8;
		uint32_t orientation:8;
		uint32_t major:8;
		uint32_t minor:8;
	} long_slot;

	struct {
		uint32_t type:7;
		uint32_t frame:1;
		uint32_t x:12;
		uint32_t y:12;
	} short_slot;

	struct {
		uint32_t pad:8;
		uint32_t buttons:24;
	} button_slot;

	struct {
		uint32_t code_low:7;
		uint32_t frame:1;
		uint32_t code_hi:8;
		uint32_t param0:16;
		uint32_t param1;
		uint32_t param2;
	} log_slot;
};
#endif /* BCMTCH_STATE_PROTOCOL */

struct bcmtch_data {
	/* core 0S elements */
	/* Work queue structure for defining work queue handler */
	struct work_struct work;
	/* Work queue structure for transaction handling */
	struct workqueue_struct *p_workqueue;

	/* The table of worker process functions */
	void (*bcmtch_dev_process_table[BCMTCH_WP_NUMBER])
		(struct bcmtch_data *);
	/* The index of the process function table */
	uint8_t work_process_index;

	/* Critical Section : Mutex : */
	struct mutex mutex_work;

	/* Pointer to allocated memory for input device */
	struct input_dev *p_input_device;

	struct device *p_device;
	/* I2C 0S elements */

	/* SPM I2C Client structure pointer */
	struct i2c_client *p_i2c_client_spm;

	/* SYS I2C Client structure pointer */
	struct i2c_client *p_i2c_client_sys;

	/* Local copy of platform data structure */
	struct bcmtch_platform_data platform_data;

	/* BCM Touch elements */
	struct bcmtch_channel *p_channels
		[BCMTCH_MAX_CHANNEL_SET][BCMTCH_CHANNEL_MAX];

	/* BCMTCH touch structure */
	struct bcmtch_touch touch[BCMTCH_MAX_TOUCH];
	uint32_t touch_count;

	uint8_t touch_event_track_id;

	/* Response storage */
	struct bcmtch_response_wait bcmtch_cmd_response[TOFE_COMMAND_LAST];

	/* BCM Button elements */
	uint16_t button_status;
#ifdef CONFIG_OF
	const int32_t bcmtch_button_map[BCMTCH_MAX_BUTTONS];
#endif

	/* DMA transfer mode */
	bool has_dma_channel;
	bool host_override;
	uint32_t fw_dma_buffer_size;
	void *fw_dma_buffer;

	/* Interrupts */
	bool irq_pending;
	bool irq_enabled;

#ifdef CONFIG_REGULATOR
	struct regulator *regulator_avdd33;
	struct regulator *regulator_vddo;
	struct regulator *regulator_avdd_adldo;
#endif
	/* Power Management */
	uint32_t power_on_delay_us; /* us wait time after power on */

	/* Watchdog Timer */
	uint32_t watchdog_expires;
	struct timer_list watchdog;

	/* Post Boot Downloads */
	uint8_t channel_set;	/* ROM/RAM channels */
	uint8_t	post_boot_pending;
	uint8_t *post_boot_buffer;
	uint8_t *post_boot_data;
	uint32_t post_boot_addr;
	uint16_t post_boot_left;
	uint16_t post_boot_sections;
	uint16_t post_boot_section;
	uint16_t post_boot_patches;
	uint32_t postboot_cfg_addr;
	uint32_t postboot_cfg_length;

	/* Axis Limits */
	uint16_t axis_x_max;
	uint16_t axis_x_min;
	uint16_t axis_y_max;
	uint16_t axis_y_min;
	uint16_t axis_h_max;
	uint16_t axis_h_min;

	/* Thresholds */
	uint16_t threshold_gate_finger;
	uint16_t threshold_gate_stylus;

	/* FW Signature */
	struct tofe_signature fw_signature;

	/* BCMTCH chip ID */
	uint32_t chip_id;

	/* BCMTCH revision ID */
	uint8_t rev_id;

	/* ROM boot */
	bool boot_from_rom;

#ifdef CONFIG_HAS_EARLYSUSPEND
	/* Early suspend */
	struct early_suspend bcmtch_early_suspend_desc;
#endif

#if BCMTCH_STATE_PROTOCOL
	struct bcmtch_state_cfg_rw ram_rw_cfg;
	struct bcmtch_state_start_scan scan_data;
	uint32_t exception_buffer[TOFE_HOST_EXCEPTION_REGS];
	uint8_t bcmtch_state_cmd_buffer[TOFE_HOST_CMD_BUF_SIZE];
	uint8_t bcmtch_state_resp_buffer[TOFE_HOST_RSP_BUF_SIZE];
#endif /* BCMTCH_STATE_PROTOCOL */

	/* sysfs ABI data */
	struct mutex mutex_abi;
	uint8_t abi_spm_addr;
	uint32_t abi_sys_addr;
	uint16_t abi_sys_size;
	bool abi_suspend;
};

/* -------------------------------------------- */
/* - BCM Touch Controller Function Prototypes - */
/* -------------------------------------------- */

/*  DEV Prototypes */
static void	   bcmtch_dev_process(
	struct bcmtch_data *);
static void    bcmtch_dev_process_pb_patch_init(
	struct bcmtch_data *);
static int32_t bcmtch_dev_reset(struct bcmtch_data *,
	uint8_t);
static int32_t bcmtch_dev_suspend(
	struct bcmtch_data *);
static int32_t bcmtch_dev_resume(
	struct bcmtch_data *);
static int32_t bcmtch_dev_request_power_mode(
	struct bcmtch_data *,
	uint8_t, enum tofe_command);
static int32_t bcmtch_dev_send_command(
	struct bcmtch_data *,
	enum tofe_command, uint32_t, uint16_t, uint8_t);
static inline bool bcmtch_dev_verify_buffer_header(
	struct bcmtch_data *,
	struct tofe_channel_buffer_header*);
static int32_t bcmtch_dev_post_boot_download(
	struct bcmtch_data *bcmtch_data_ptr,
	int16_t data_rate);
static int32_t bcmtch_dev_post_boot_get_section(
	struct bcmtch_data *);
static void    bcmtch_dev_watchdog_work(unsigned long int data);
static void    bcmtch_dev_watchdog_start(
	struct bcmtch_data *);
static int32_t bcmtch_dev_watchdog_reset(
	struct bcmtch_data *);
static int32_t bcmtch_dev_watchdog_stop(
	struct bcmtch_data *);
static int32_t bcmtch_dev_watchdog_restart(
	struct bcmtch_data *, uint32_t);
static int32_t bcmtch_dev_watchdog_check(
	struct bcmtch_data *);
static int32_t bcmtch_dev_power_init(
	struct bcmtch_data *);
static int32_t bcmtch_dev_power_enable(
	struct bcmtch_data *,
	bool);
static int32_t bcmtch_dev_power_free(
	struct bcmtch_data *);
static void bcmtch_dev_reset_events(
	struct bcmtch_data *bcmtch_data_ptr);
static int32_t bcmtch_dev_sync_event_frame(
	struct bcmtch_data *bcmtch_data_ptr);

#if BCMTCH_STATE_PROTOCOL
static void bcmtch_dev_init_state(
	struct bcmtch_data *);
static void	bcmtch_dev_process_state_touches(
	struct bcmtch_data *);
static int32_t bcmtch_state_start_scan(
	struct bcmtch_data *);
static int32_t bcmtch_state_stop_scan(
	struct bcmtch_data *);
static int32_t bcmtch_state_sleep(
	struct bcmtch_data *);
static int32_t bcmtch_state_suspend(
	struct bcmtch_data *);
static int32_t bcmtch_state_resume(
	struct bcmtch_data *);
static int32_t bcmtch_state_write_ram(
	struct bcmtch_data *, uint32_t, uint16_t, uint8_t *);
static int32_t bcmtch_state_read_ram(
	struct bcmtch_data *, uint32_t, uint16_t, uint8_t *);
static int32_t bcmtch_state_check_command_status(
	struct bcmtch_data *, uint8_t);
static int32_t bcmtch_state_i2c_send_command(
	struct bcmtch_data *, uint8_t, uint8_t*, int);
#endif /* BCMTCH_STATE_PROTOCOL */


/*  COM Prototypes */
static int32_t  bcmtch_com_init(struct bcmtch_data *);
static int32_t  bcmtch_com_read_spm(
	struct bcmtch_data *, uint8_t, uint8_t*);
static int32_t  bcmtch_com_write_spm(
	struct bcmtch_data *, uint8_t, uint8_t);
static int32_t  bcmtch_com_read_sys(
	struct bcmtch_data *, uint32_t, uint16_t, uint8_t*);
static int32_t  bcmtch_com_write_sys(
	struct bcmtch_data *, uint32_t, uint16_t, uint8_t*);
static int32_t	bcmtch_com_read_dma(
	struct bcmtch_data *, uint16_t, uint8_t*);
/* COM Helper */
static inline int32_t bcmtch_com_fast_write_spm(
	struct bcmtch_data *, uint8_t, uint8_t*, uint8_t*);
static inline int32_t bcmtch_com_write_sys32(
	struct bcmtch_data *, uint32_t, uint32_t);

/*  OS Prototypes */
static void     bcmtch_reset(struct bcmtch_data *);
static int32_t  bcmtch_interrupt_enable(
	struct bcmtch_data *);
static void     bcmtch_interrupt_disable(
	struct bcmtch_data *);
static void		bcmtch_deferred_worker(
	struct work_struct *work);
static void     bcmtch_clear_deferred_worker(
	struct bcmtch_data *);

#ifdef CONFIG_PM
#ifndef CONFIG_HAS_EARLYSUSPEND
static int      bcmtch_suspend(
					struct i2c_client *p_client,
					pm_message_t mesg);
static int      bcmtch_resume(struct i2c_client *p_client);
#endif
#endif

/*  OS I2C Prototypes */
static int32_t  bcmtch_i2c_probe(
					struct i2c_client*,
					const struct i2c_device_id *);
static int32_t  bcmtch_i2c_remove(struct i2c_client *);
static int32_t  bcmtch_i2c_read_spm(struct i2c_client*, uint8_t, uint8_t*);
static int32_t  bcmtch_i2c_write_spm(struct i2c_client*, uint8_t, uint8_t);
static int32_t  bcmtch_i2c_fast_write_spm(
					struct i2c_client*,
					uint8_t,
					uint8_t*,
					uint8_t*);
static int32_t  bcmtch_i2c_read_sys(
					struct i2c_client*,
					uint32_t,
					uint16_t,
					uint8_t*);
static int32_t  bcmtch_i2c_write_sys(
					struct i2c_client*,
					uint32_t,
					uint16_t,
					uint8_t*);
static int32_t  bcmtch_i2c_init_clients(struct i2c_client *);
static void     bcmtch_i2c_free_clients(struct bcmtch_data *);
static int32_t  bcmtch_i2c_read_dma(
					struct i2c_client*,
					uint16_t,
					uint8_t*);

/* ------------------------------------------------- */
/* - BCM Touch Controller SysFs ABI Implementation - */
/* ------------------------------------------------- */

/* -- SYS peek/poke ABI -- */
static ssize_t bcmtch_os_abi_sys_addr_show(
					struct device *dev,
					struct device_attribute *devattr,
					char *buf)
{
	int count = 0;
	struct bcmtch_data *bcmtch_data_ptr =
		dev_get_drvdata(dev);

	count += snprintf(buf + count, PAGE_SIZE - count,
			"0x%04X %u\n",
			bcmtch_data_ptr->abi_sys_addr,
			bcmtch_data_ptr->abi_sys_size);

	return count;
}

static ssize_t bcmtch_os_abi_sys_addr_store(
					struct device *dev,
					struct device_attribute *devattr,
					const char *buf,
					size_t count)
{
	int ret_val = 0;
	struct bcmtch_data *bcmtch_data_ptr =
		dev_get_drvdata(dev);

	uint32_t addr;
	unsigned int len;

	ret_val = sscanf(buf, "0x%x %u", &addr, &len);
	if (ret_val != 2) {
		BCMTCH_ERR(
			"format: 0x<32bit address> <memory chunk size in bytes>\n");
		return -EINVAL;
	}

	mutex_lock(&bcmtch_data_ptr->mutex_abi);
	bcmtch_data_ptr->abi_sys_addr = addr;
	bcmtch_data_ptr->abi_sys_size = (uint16_t)len;
	mutex_unlock(&bcmtch_data_ptr->mutex_abi);

	return count;
}

static DEVICE_ATTR(sys_addr, 0664,
		bcmtch_os_abi_sys_addr_show,
		bcmtch_os_abi_sys_addr_store);

static ssize_t bcmtch_os_abi_sys_data_show(
					struct device *dev,
					struct device_attribute *devattr,
					char *buf)
{
	int count = 0;
	int32_t ret_val = 0;
	struct bcmtch_data *bcmtch_data_ptr =
		dev_get_drvdata(dev);

	int i;
	uint8_t r8[512];

	if (bcmtch_data_ptr->abi_sys_size > 512)
		return -EINVAL;

	mutex_lock(&bcmtch_data_ptr->mutex_work);
#if BCMTCH_STATE_PROTOCOL
	if (bcmtch_data_ptr->work_process_index >= BCMTCH_WP_STATE)
		ret_val = bcmtch_state_read_ram(
					bcmtch_data_ptr,
					bcmtch_data_ptr->abi_sys_addr,
					bcmtch_data_ptr->abi_sys_size,
					r8);
	else
#endif
		ret_val = bcmtch_com_read_sys(
					bcmtch_data_ptr,
					bcmtch_data_ptr->abi_sys_addr,
					bcmtch_data_ptr->abi_sys_size,
					r8);
	mutex_unlock(&bcmtch_data_ptr->mutex_work);

	if (!ret_val) {
		for (i = 0; i < bcmtch_data_ptr->abi_sys_size; i++) {
			count += snprintf(buf + count, PAGE_SIZE - count,
						"%02X ", r8[i]);
		}
		count += snprintf(buf + count,
				PAGE_SIZE - count, "\n");
	}

	return count;
}

static ssize_t bcmtch_os_abi_sys_data_store(
					struct device *dev,
					struct device_attribute *devattr,
					const char *buf,
					size_t count)
{
	int ret_val = 0;
	struct bcmtch_data *bcmtch_data_ptr =
		dev_get_drvdata(dev);
	char tok[5] = "0xff";

	unsigned int in_data;
	uint8_t r8[512];
	int i, j, k;

	for (i = 0, j = 0, k = 0; i < count; i++) {
		if (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n') {
			if (j) {
				tok[j] = '\0';
				ret_val = kstrtouint(tok, 16, &in_data);
				if (!ret_val)
					r8[k++] = (uint8_t)in_data;
			}
			j = 0;
			continue;
		}
		tok[j++] = buf[i];
		if (j > 4) {
			j = 0;
			continue;
		}
	}

	if (k < bcmtch_data_ptr->abi_sys_size) {
		BCMTCH_ERR("ERROR: only read %u out of %u data\n",
				k, bcmtch_data_ptr->abi_sys_size);
		return -EINVAL;
	}

	mutex_lock(&bcmtch_data_ptr->mutex_work);
#if BCMTCH_STATE_PROTOCOL
	if (bcmtch_data_ptr->work_process_index >= BCMTCH_WP_STATE)
		bcmtch_state_write_ram(
				bcmtch_data_ptr,
				bcmtch_data_ptr->abi_sys_addr,
				bcmtch_data_ptr->abi_sys_size,
				r8);
	else
#endif
		bcmtch_com_write_sys(
				bcmtch_data_ptr,
				bcmtch_data_ptr->abi_sys_addr,
				bcmtch_data_ptr->abi_sys_size,
				r8);
	mutex_unlock(&bcmtch_data_ptr->mutex_work);

	return count;
}

static DEVICE_ATTR(sys_data, 0664,
		bcmtch_os_abi_sys_data_show,
		bcmtch_os_abi_sys_data_store);


/* -- SPM peek/poke ABI -- */
static ssize_t bcmtch_os_abi_spm_addr_show(
					struct device *dev,
					struct device_attribute *devattr,
					char *buf)
{
	int count = 0;
	struct bcmtch_data *bcmtch_data_ptr =
		dev_get_drvdata(dev);

	count += snprintf(buf + count, PAGE_SIZE - count,
			"0x%02x\n", bcmtch_data_ptr->abi_spm_addr);

	return count;
}

static ssize_t bcmtch_os_abi_spm_addr_store(
					struct device *dev,
					struct device_attribute *devattr,
					const char *buf,
					size_t count)
{
	int ret_val = 0;
	struct bcmtch_data *bcmtch_data_ptr =
		dev_get_drvdata(dev);
	unsigned int addr;

	ret_val = kstrtouint(buf, 16, &addr);
	if (ret_val)
		return ret_val;

	mutex_lock(&bcmtch_data_ptr->mutex_abi);
	bcmtch_data_ptr->abi_spm_addr = (uint8_t)addr;
	mutex_unlock(&bcmtch_data_ptr->mutex_abi);

	return count;
}

static DEVICE_ATTR(spm_addr, 0664,
		bcmtch_os_abi_spm_addr_show,
		bcmtch_os_abi_spm_addr_store);

static ssize_t bcmtch_os_abi_spm_data_show(
					struct device *dev,
					struct device_attribute *devattr,
					char *buf)
{
	int count = 0;
	struct bcmtch_data *bcmtch_data_ptr =
		dev_get_drvdata(dev);
	uint8_t r8;

	mutex_lock(&bcmtch_data_ptr->mutex_work);
	bcmtch_com_read_spm(bcmtch_data_ptr,
			bcmtch_data_ptr->abi_spm_addr, &r8);
	mutex_unlock(&bcmtch_data_ptr->mutex_work);

	count += snprintf(buf + count, PAGE_SIZE - count,
				"0x%02x\n", r8);
	return count;
}

static ssize_t bcmtch_os_abi_spm_data_store(
					struct device *dev,
					struct device_attribute *devattr,
					const char *buf,
					size_t count)
{
	int ret_val = 0;
	struct bcmtch_data *bcmtch_data_ptr =
		dev_get_drvdata(dev);
	unsigned int in_data;

	ret_val = kstrtouint(buf, 16, &in_data);
	if (ret_val)
		return ret_val;

	mutex_lock(&bcmtch_data_ptr->mutex_work);
	bcmtch_com_write_spm(bcmtch_data_ptr,
			bcmtch_data_ptr->abi_spm_addr,
			(uint8_t)in_data);
	mutex_unlock(&bcmtch_data_ptr->mutex_work);

	return count;
}

static DEVICE_ATTR(spm_data, 0664,
		bcmtch_os_abi_spm_data_show,
		bcmtch_os_abi_spm_data_store);

/* -- Finger threshold ABI -- */
static ssize_t bcmtch_os_abi_finger_threshold(
					struct device *dev,
					struct device_attribute *devattr,
					char *buf)
{
	int count = 0;
	struct bcmtch_data *bcmtch_data_ptr =
		dev_get_drvdata(dev);

	count += snprintf(buf + count, PAGE_SIZE - count,
			"%u\n", bcmtch_data_ptr->threshold_gate_finger);

	return count;
}

static DEVICE_ATTR(
		finger_threshold,
		0444,
		bcmtch_os_abi_finger_threshold,
		NULL);

/* -- Stylus threshold ABI -- */
static ssize_t bcmtch_os_abi_stylus_threshold(
					struct device *dev,
					struct device_attribute *devattr,
					char *buf)
{
	int count = 0;
	struct bcmtch_data *bcmtch_data_ptr =
		dev_get_drvdata(dev);

	count += snprintf(buf + count, PAGE_SIZE - count,
			"%u\n", bcmtch_data_ptr->threshold_gate_stylus);

	return count;
}

static DEVICE_ATTR(
		stylus_threshold,
		0444,
		bcmtch_os_abi_stylus_threshold,
		NULL);

/* -- Suspend ABI -- */
static ssize_t bcmtch_os_abi_suspend_show(
					struct device *dev,
					struct device_attribute *devattr,
					char *buf)
{
	int count = 0;
	struct bcmtch_data *bcmtch_data_ptr =
		dev_get_drvdata(dev);

	count += snprintf(buf + count, PAGE_SIZE - count,
			"%u\n",
			bcmtch_data_ptr->abi_suspend ? 1 : 0);

	return count;
}

static ssize_t bcmtch_os_abi_suspend_store(
					struct device *dev,
					struct device_attribute *devattr,
					const char *buf,
					size_t count)
{
	int ret_val = 0;
	struct bcmtch_data *bcmtch_data_ptr =
		dev_get_drvdata(dev);
	unsigned int val;

	ret_val = kstrtouint(buf, 10, &val);
	if (ret_val)
		goto abi_suspend_error;

	switch (val) {
	case 0:
		if (bcmtch_data_ptr->abi_suspend)
			bcmtch_dev_resume(bcmtch_data_ptr);
		break;
	case 1:
		if (!bcmtch_data_ptr->abi_suspend)
			bcmtch_dev_suspend(bcmtch_data_ptr);
		break;
	default:
		goto abi_suspend_error;
	}

	return count;

abi_suspend_error:
	BCMTCH_ERR("ERROR: suspend ABI - set 0 or 1\n");
	return -EINVAL;
}

static DEVICE_ATTR(suspend, 0664,
		bcmtch_os_abi_suspend_show,
		bcmtch_os_abi_suspend_store);


static struct attribute *bcmtch_abi_attrs[] = {
	&dev_attr_spm_addr.attr,
	&dev_attr_spm_data.attr,
	&dev_attr_sys_addr.attr,
	&dev_attr_sys_data.attr,
	&dev_attr_finger_threshold.attr,
	&dev_attr_stylus_threshold.attr,
	&dev_attr_suspend.attr,
	NULL
};

static const struct attribute_group bcmtch_attr_group = {
	.attrs = bcmtch_abi_attrs,
};


static int32_t bcmtch_os_init_abi(struct device *p_device)
{
	int32_t ret_val = 0;
	struct bcmtch_data *bcmtch_data_ptr =
		dev_get_drvdata(p_device);

	ret_val = sysfs_create_group(&p_device->kobj, &bcmtch_attr_group);
	if (ret_val)
		BCMTCH_ERR(
			"ERROR: %s() - device_create_file() failed!\n",
			__func__);

	bcmtch_data_ptr->abi_suspend = false;

	return ret_val;
}

static inline void bcmtch_os_free_abi(struct device *p_device)
{
	sysfs_remove_group(&p_device->kobj, &bcmtch_attr_group);
}

/* ------------------------------------------- */
/* - BCM Touch Controller CLI Implementation - */
/* ------------------------------------------- */

static void bcmtch_os_cli_versions(
					struct bcmtch_data *bcmtch_data_ptr)
{
	/* Return driver and firmware version info */
	BCMTCH_INFO("Driver: %s : %s : %s\n",
		BCMTCH_DRIVER_VERSION,
		BCMTCH_DRIVER_BUILD_DATE,
		BCMTCH_DRIVER_BUILD_TIME);

	BCMTCH_INFO("F/W Version: %d.%d.%d.%d\n",
		bcmtch_data_ptr->fw_signature.version.generation,
		bcmtch_data_ptr->fw_signature.version.spin,
		bcmtch_data_ptr->fw_signature.version.major,
		bcmtch_data_ptr->fw_signature.version.minor);

	BCMTCH_INFO("F/W Commit: %016llx\n",
		bcmtch_data_ptr->fw_signature.commit);

	BCMTCH_INFO(
		"F/W Build:%d Compatibility:%d Variant:%d\n",
		bcmtch_data_ptr->fw_signature.build,
		bcmtch_data_ptr->fw_signature.compatibility,
		bcmtch_data_ptr->fw_signature.variant);

	BCMTCH_INFO(
		"F/W ReleaseType:%c ReleaseNum:%d CustReleaseNum:%d\n",
		bcmtch_data_ptr->fw_signature.release_type,
		bcmtch_data_ptr->fw_signature.release_number,
		bcmtch_data_ptr->fw_signature.cust_release_num);
}

static void bcmtch_os_cli_logmask_get(
					struct bcmtch_data *bcmtch_data_ptr,
					uint32_t in_logmask)
{
	struct bcmtch_response_wait *p_resp;
	uint32_t logmask;
	uint32_t ret_val;

	BCMTCH_INFO("get logmask module_id=0x%04x\n",
			in_logmask);

	logmask = (uint32_t)(in_logmask << 16);
	p_resp = (struct bcmtch_response_wait *)
		&(bcmtch_data_ptr->bcmtch_cmd_response
				[TOFE_COMMAND_GET_LOG_MASK]);
	p_resp->wait = 1;

	mutex_lock(&bcmtch_data_ptr->mutex_work);

	ret_val = bcmtch_dev_send_command(
			bcmtch_data_ptr,
			TOFE_COMMAND_GET_LOG_MASK,
			logmask,
			0,
			TOFE_COMMAND_FLAG_REQUEST_RESULT);
	if (ret_val != 0) {
		BCMTCH_ERR("send_command error [%d] cmd=%x\n",
				ret_val,
				TOFE_COMMAND_GET_LOG_MASK);
	}

	mutex_unlock(&bcmtch_data_ptr->mutex_work);
}

static void bcmtch_os_cli_logmask_set(
					struct bcmtch_data *bcmtch_data_ptr,
					uint32_t in_logmask)
{
	struct bcmtch_response_wait *p_resp;

	BCMTCH_INFO("set logmask mask=0x%08x . . .\n",
			in_logmask);

	p_resp = (struct bcmtch_response_wait *)
		&(bcmtch_data_ptr->bcmtch_cmd_response
				[TOFE_COMMAND_SET_LOG_MASK]);
	p_resp->wait = 1;

	mutex_lock(&bcmtch_data_ptr->mutex_work);

	bcmtch_dev_send_command(
			bcmtch_data_ptr,
			TOFE_COMMAND_SET_LOG_MASK,
			in_logmask,
			0,
			TOFE_COMMAND_FLAG_REQUEST_RESULT);

	mutex_unlock(&bcmtch_data_ptr->mutex_work);
}

static void bcmtch_os_cli_spm_poke(
					struct bcmtch_data *bcmtch_data_ptr,
					uint8_t in_addr,
					uint8_t in_data)
{
	mutex_lock(&bcmtch_data_ptr->mutex_work);

	bcmtch_com_write_spm(bcmtch_data_ptr,
		in_addr, in_data);

	mutex_unlock(&bcmtch_data_ptr->mutex_work);

	BCMTCH_INFO("poke spm Reg=%08x data=%08x\n",
		in_addr, in_data);
}

static void bcmtch_os_cli_spm_peek(
					struct bcmtch_data *bcmtch_data_ptr,
					uint8_t in_addr)
{
	uint8_t r8;

	mutex_lock(&bcmtch_data_ptr->mutex_work);

	bcmtch_com_read_spm(bcmtch_data_ptr, in_addr, &r8);

	mutex_unlock(&bcmtch_data_ptr->mutex_work);

	BCMTCH_INFO("peek spm reg=0x%02x data=0x%02x\n",
		in_addr, r8);
}

static void bcmtch_os_cli_sys_poke(
					struct bcmtch_data *bcmtch_data_ptr,
					uint32_t in_addr,
					uint32_t in_data)
{
	mutex_lock(&bcmtch_data_ptr->mutex_work);

#if BCMTCH_STATE_PROTOCOL
	if (bcmtch_data_ptr->work_process_index >= BCMTCH_WP_STATE)
		bcmtch_state_write_ram(
				bcmtch_data_ptr,
				in_addr,
				sizeof(uint32_t),
				(uint8_t *)&in_data);
	else
#endif
	bcmtch_com_write_sys32(bcmtch_data_ptr, in_addr, in_data);

	mutex_unlock(&bcmtch_data_ptr->mutex_work);

	BCMTCH_INFO("poke sys addr=0x%08x data=0x%08x\n",
			in_addr, in_data);
}

static void bcmtch_os_cli_sys_peek(
					struct bcmtch_data *bcmtch_data_ptr,
					uint32_t in_addr,
					uint32_t in_count)
{
	uint32_t r_buf[8];
	uint32_t addr = in_addr;
	uint32_t count = in_count;

	memset(r_buf, 0, 8 * sizeof(uint32_t));

	BCMTCH_INFO("peek sys addr=0x%08x len=0x%08x\n",
		in_addr, in_count);

	mutex_lock(&bcmtch_data_ptr->mutex_work);

	while (count) {
#if BCMTCH_STATE_PROTOCOL
		if (bcmtch_data_ptr->work_process_index >= BCMTCH_WP_STATE)
			bcmtch_state_read_ram(
					bcmtch_data_ptr,
					addr,
					8 * sizeof(uint32_t),
					(uint8_t *)r_buf);
		else
#endif
		bcmtch_com_read_sys(bcmtch_data_ptr,
			addr, 8 * sizeof(uint32_t),
			(uint8_t *)r_buf);

		BCMTCH_INFO(
			"0x%08x: 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
			addr,
			r_buf[0], r_buf[1], r_buf[2], r_buf[3],
			r_buf[4], r_buf[5], r_buf[6], r_buf[7]);

		count =
		    (count > (8 * sizeof(uint32_t))) ?
			(count - (8 * sizeof(uint32_t))) :
			0;
		addr += (8 * sizeof(uint32_t));
	}

	mutex_unlock(&bcmtch_data_ptr->mutex_work);
}


static ssize_t bcmtch_os_cli(
					struct device *dev,
					struct device_attribute *devattr,
					const char *buf,
					size_t count)
{
	uint32_t in_addr = 0;
	uint32_t in_value_count = 0;
	struct bcmtch_data *bcmtch_data_ptr =
		dev_get_drvdata(dev);


	/* We are now checking for exact number of matches
	 * to the format. Buf and format are const.
	 * sscanf will return 0 on first mismatch.
	 * We don't anticipate buf to be NULL here
	 * Annotate this if coverity flags it
	 */

	/* In future, we should also veriify that address
	 * is valid. We might change this interface in
	 * near future so leave that part as is
	 */
	if ((count > strlen("poke sys 0x 0x")) &&
			sscanf(buf, "poke sys %x %x", &in_addr,
				&in_value_count) == 2) {
		bcmtch_os_cli_sys_poke(bcmtch_data_ptr,
			in_addr, in_value_count);
	} else if ((count > strlen("peek sys 0x 0x")) &&
			sscanf(buf, "peek sys %x %x", &in_addr,
				&in_value_count) == 2) {
		bcmtch_os_cli_sys_peek(bcmtch_data_ptr,
			in_addr, in_value_count);
	} else if ((count > strlen("poke spm 0x 0x")) &&
			sscanf(buf, "poke spm %x %x", &in_addr,
				&in_value_count) == 2) {
		bcmtch_os_cli_spm_poke(bcmtch_data_ptr,
			in_addr & 0xff, in_value_count & 0xff);
	} else if ((count > strlen("peek spm 0x")) &&
			sscanf(buf, "peek spm %x",
				&in_addr) == 1) {
		bcmtch_os_cli_spm_peek(bcmtch_data_ptr,
			in_addr & 0xff);
	} else if ((count > strlen("set logmask 0x")) &&
			sscanf(buf, "set logmask %x",
				&in_value_count) == 1) {
		bcmtch_os_cli_logmask_set(bcmtch_data_ptr,
			in_value_count);
	} else if ((count > strlen("get logmask 0x")) &&
			sscanf(buf, "get logmask %x",
				&in_value_count) == 1) {
		bcmtch_os_cli_logmask_get(bcmtch_data_ptr,
			in_value_count);
	}
#if defined(CONFIG_PM)
	else if (strncmp(buf, "suspend", strlen("suspend")) == 0) {
		BCMTCH_INFO("cli --> suspend\n");
		bcmtch_dev_suspend(bcmtch_data_ptr);
	} else if (strncmp(buf, "resume", strlen("resume")) == 0) {
		BCMTCH_INFO("cli --> resume\n");
		bcmtch_dev_resume(bcmtch_data_ptr);
	}
#endif
	else if (strncmp(buf, "threshold finger",
					strlen("threshold finger")) == 0) {
		BCMTCH_INFO("finger threshold = %u\n",
			bcmtch_data_ptr->threshold_gate_finger);
	} else if (strncmp(buf, "threshold stylus",
					strlen("threshold stylus")) == 0) {
		BCMTCH_INFO("stylus threshold = %u\n",
			bcmtch_data_ptr->threshold_gate_stylus);
	} else if (strncmp(buf, "versions", strlen("versions")) == 0) {
		bcmtch_os_cli_versions(bcmtch_data_ptr);
	} else {
		BCMTCH_INFO("Usage:\n");
		BCMTCH_INFO("poke sys 0x<addr> 0x<data>\n");
		BCMTCH_INFO("peek sys 0x<addr> 0x<len>\n");
		BCMTCH_INFO("poke spm 0x<reg> 0x<data>\n");
		BCMTCH_INFO("peek spm 0x<reg>\n");
		BCMTCH_INFO("set logmask 0x<module>\n");
		BCMTCH_INFO("get logmask 0x<mask - bitmap>\n");
#if defined(CONFIG_PM)
		BCMTCH_INFO("suspend\n");
		BCMTCH_INFO("resume\n");
#endif
		BCMTCH_INFO("threshold finger\n");
		BCMTCH_INFO("threshold stylus\n");
		BCMTCH_INFO("versions\n");
	}

	return count;
}

static struct device_attribute bcmtch_cli_attr =
		__ATTR(cli, S_IWUSR|S_IWGRP, NULL, bcmtch_os_cli);


/* ------------------------------------------- */
/* - BCM Touch Controller Internal Functions - */
/* ------------------------------------------- */

static inline
unsigned bcmtch_channel_num_queued(
		struct tofe_channel_header *channel)
{
	return (unsigned)channel->
		buffer[0]->header.entry_count;
}

/*
    Note: Internal use only function.
*/
static inline char *
_bcmtch_inline_channel_entry(
			struct tofe_channel_header *channel,
			uint32_t byte_index)
{
	return (char *)channel->buffer[0]->data
			+ byte_index;
}

/*
    Note: Internal use only function.
*/
static inline size_t
_bcmtch_inline_channel_byte_index(
		struct tofe_channel_header *channel,
		uint8_t entry_index)
{
	return entry_index * channel->entry_size;
}

/**
    Check if a channel is empty.

    Events are not considered read or writen until the transaction is
    complete.  Therefore, a channel is empty even when in the middle of a
    set of writes.

    @param
	[in] channel Pointer to channel object.

    @retval
	bool True if channel is empty.

*/
static inline bool
bcmtch_inline_channel_is_empty(struct tofe_channel_header *channel)
{
	return (channel->buffer[0]->header.entry_count == 0);
}

/**
    Read a single entry from a channel.  This function must be called during
    a read transaction.

    The pointer returned by this function points into the channel object itself.
    Callers should not modify or reuse this memory.  Callers may not free the
    memory.


    @param
	[in] channel Pointer to channel object.

    @retval
	void * Pointer to returned entry.

*/
static inline void *bcmtch_inline_channel_read(
			struct tofe_channel_header *channel,
			uint16_t index)
{
	size_t byte_index;
	struct tofe_channel_buffer *buff = channel->buffer[0];

	/* Validate that channel has entries. */
	if (buff->header.entry_count == 0)
		return NULL;

	/* Check if buffer data corrupted */
	if (buff->header.entry_size != channel->entry_size)
		return NULL;

	/* Check if read end. */
	if (index >= buff->header.entry_count)
		return NULL;

	/* Find entry in the channel. */
	byte_index = _bcmtch_inline_channel_byte_index(channel, index);
	return (void *)((char *)channel->buffer[0]->data
			+ byte_index);
}

static inline void
tofe_channel_write_begin(struct tofe_channel_header *channel)
{
	struct tofe_channel_buffer_header *buff =
		&channel->buffer[0]->header;
	if (channel->flags & TOFE_CHANNEL_FLAG_INBOUND)
		buff->entry_count = 0;
}

static inline uint32_t
tofe_channel_write(struct tofe_channel_header *channel, void *entry)
{
	struct tofe_channel_buffer_header *buff =
		&channel->buffer[0]->header;
	size_t byte_index =
		channel->entry_size * buff->entry_count;
	char *p_data = _bcmtch_inline_channel_entry(
						channel,
						byte_index);

	if ((channel->flags & TOFE_CHANNEL_FLAG_INBOUND) == 0)
		return -EINVAL;

	if (buff->entry_count >= channel->entry_num)
		return -ENOMEM;

	memcpy(p_data, entry, channel->entry_size);
	buff->entry_count++;

	return 0;
}

static
uint16_t bcmtch_max_h_axis(uint16_t x, uint16_t y)
{
	uint16_t i;

	uint16_t res = 0;
	uint16_t add = 0x8000;

	uint32_t g = (x * x) + (y * y);

	for (i = 0; i < 16 ; i++) {
		uint16_t temp = res | add;
		uint32_t g2 = temp * temp;

		if (g >= g2)
			res = temp;

		add >>= 1;
	}
	return res;
}

/* ------------------------------------------- */
/* - BCM Touch Controller DEV Functions - */
/* ------------------------------------------- */

static int32_t bcmtch_dev_alloc(struct i2c_client *p_i2c_client)
{
	int32_t ret_val = 0;
	struct bcmtch_data *bcmtch_data_ptr;


	bcmtch_data_ptr =
		kzalloc(sizeof(struct bcmtch_data), GFP_KERNEL);

	if (bcmtch_data_ptr == NULL) {
		BCMTCH_ERR("%s: failed to alloc mem.\n", __func__);
		ret_val = -ENOMEM;
	}

	mutex_init(&bcmtch_data_ptr->mutex_work);
	mutex_init(&bcmtch_data_ptr->mutex_abi);

	i2c_set_clientdata(p_i2c_client, (void *)bcmtch_data_ptr);

	bcmtch_data_ptr->p_device = &p_i2c_client->dev;
	return ret_val;
}

static void bcmtch_dev_free(struct i2c_client *p_i2c_client)
{
	struct bcmtch_data *local_bcmtch_data_p =
		(struct bcmtch_data *)
			i2c_get_clientdata(p_i2c_client);

	kfree(local_bcmtch_data_p);
	i2c_set_clientdata(p_i2c_client, NULL);
}

static int32_t bcmtch_dev_init_clocks(
		struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;
	uint32_t val32;

	/* setup LPLFO - read OTP and set from value */
	bcmtch_com_read_sys(
			bcmtch_data_ptr,
			BCMTCH_ADDR_SPM_LPLFO_CTRL_RO,
			4,
			(uint8_t *)&val32);
	val32 &= 0xF0000000;
	val32 >>= 28;
	bcmtch_com_write_spm(
		bcmtch_data_ptr,
		BCMTCH_SPM_REG_LPLFO_CTRL,
		(uint8_t)val32 | 0x10);

	return ret_val;
}

static int32_t bcmtch_dev_init_memory(
		struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;
	uint32_t mem_map;

	mem_map = (bcmtch_data_ptr->boot_from_rom) ?
			BCMTCH_MEM_ROM_BOOT : BCMTCH_MEM_RAM_BOOT;

	bcmtch_data_ptr->channel_set =
		(bcmtch_data_ptr->boot_from_rom) ?
			BCMTCH_ROM_CHANNELS : BCMTCH_RAM_CHANNELS;

	if (mem_map)
		ret_val =
			bcmtch_com_write_sys32(
				bcmtch_data_ptr,
				BCMTCH_MEM_REMAP_ADDR, mem_map);

	return ret_val;
}

static inline void
tofe_channel_header_init(
		struct bcmtch_data *bcmtch_data_ptr,
		struct bcmtch_channel *p_channel,
		struct tofe_channel_instance_cfg *p_chan_cfg)
{
	struct tofe_channel_header *hdr = &p_channel->hdr;
	struct tofe_channel_buffer_header *buff;
	hdr->entry_num = p_chan_cfg->entry_num;
	hdr->entry_size = p_chan_cfg->entry_size;
	hdr->trig_level = p_chan_cfg->trig_level;
	hdr->flags = p_chan_cfg->flags;
	hdr->buffer_num = p_chan_cfg->buffer_num;
	hdr->buffer_idx = 0;
	hdr->seq_count = 0;
	hdr->buffer[0] = (struct tofe_channel_buffer *)&p_channel->data;
	hdr->buffer[1] = (struct tofe_channel_buffer *)&p_channel->data;
	if (hdr->flags & TOFE_CHANNEL_FLAG_FWDMA_ENABLE) {
		bcmtch_data_ptr->has_dma_channel = true;
		buff = &hdr->buffer[1]->header;
		buff->channel_id = hdr->channel_id;
		buff->entry_count = 0;
		buff->entry_size = hdr->entry_size;
		bcmtch_data_ptr->fw_dma_buffer_size +=
			sizeof(struct tofe_channel_buffer_header)
			+ (hdr->entry_num * hdr->entry_size);
	}
}

static int32_t bcmtch_dev_init_channel(
		struct bcmtch_data *bcmtch_data_ptr,
		uint8_t chan_set,
		enum bcmtch_channel_id chan_id,
		struct tofe_channel_instance_cfg *p_chan_cfg,
		uint8_t active)
{
	int32_t ret_val = 0;
	uint32_t channel_size;
	struct bcmtch_channel *p_channel;

	if (active) {
		channel_size =
				/* channel data size   */
				sizeof(struct tofe_channel_buffer_header)
				+ (p_chan_cfg->flags &
					TOFE_CHANNEL_FLAG_FWDMA_ENABLE ? 0 :
					p_chan_cfg->entry_num
					* p_chan_cfg->entry_size)
				/* channel header size */
				+ sizeof(struct tofe_channel_header)
				/* channel config size */
				+ sizeof(struct tofe_channel_instance_cfg)
				/* sizes for added elements: queued, pad */
				+ (sizeof(uint16_t) * 2);
	} else {
		channel_size =
				/* channel header size */
				sizeof(struct tofe_channel_header)
				/* channel config size */
				+ sizeof(struct tofe_channel_instance_cfg)
				/* sizes for added elements: queued, pad */
				+ (sizeof(uint16_t) * 2);
	}

	p_channel = kzalloc(channel_size, GFP_KERNEL);

	if (p_channel) {
		p_channel->cfg = *p_chan_cfg;

		/* Initialize Header */
		p_channel->hdr.channel_id = (uint8_t)chan_id;
		p_channel->active = active;
		tofe_channel_header_init(bcmtch_data_ptr,
			p_channel, p_chan_cfg);

		bcmtch_data_ptr->p_channels[chan_set][chan_id] = p_channel;
	} else if (active) {
		ret_val = -ENOMEM;
		bcmtch_data_ptr->p_channels[chan_set][chan_id] = NULL;
	}

	return ret_val;
}

static void bcmtch_dev_free_channels(
	struct bcmtch_data *bcmtch_data_ptr)
{
	uint32_t chan = 0;
	uint8_t chan_set = 0;

	kfree(bcmtch_data_ptr->fw_dma_buffer);
	bcmtch_data_ptr->fw_dma_buffer = NULL;

	while (chan_set < BCMTCH_MAX_CHANNEL_SET) {
		while (chan < BCMTCH_CHANNEL_MAX) {
			if (bcmtch_data_ptr->p_channels[chan_set][chan]) {
				kfree(bcmtch_data_ptr->
					p_channels[chan_set][chan]);
				bcmtch_data_ptr->
					p_channels[chan_set][chan] = NULL;
			}
			chan++;
		}
		chan_set++;
	}
}

static int32_t bcmtch_dev_init_channels(
		struct bcmtch_data *bcmtch_data_ptr,
		uint32_t mem_addr,
		uint8_t *mem_data,
		uint8_t chan_set)
{
	int32_t ret_val = 0;
	void *p_buffer = NULL;
	uint32_t *p_cfg = NULL;
	struct tofe_channel_instance_cfg *p_chan_cfg = NULL;

	/* find channel configs */
	p_cfg = (uint32_t *)(mem_data + TOFE_SIGNATURE_SIZE);
	p_chan_cfg =
		(struct tofe_channel_instance_cfg *)
		((uint32_t)mem_data + p_cfg[TOFE_TOC_INDEX_CHANNEL] - mem_addr);

	/* check if processing channel(s) - add */
	ret_val = bcmtch_dev_init_channel(
			bcmtch_data_ptr,
			chan_set,
			BCMTCH_CHANNEL_TOUCH,
			&p_chan_cfg[TOFE_CHANNEL_ID_TOUCH],
			bcmtch_channel_flag & BCMTCH_CHANNEL_FLAG_USE_TOUCH);
	if (ret_val) {
		BCMTCH_ERR("%s: [%d] Touch Event Channel not initialized!\n",
				__func__, ret_val);
	}

	/* Command & response channels */
	if (!ret_val) {
		ret_val = bcmtch_dev_init_channel(
				bcmtch_data_ptr,
				chan_set,
				BCMTCH_CHANNEL_COMMAND,
				&p_chan_cfg[TOFE_CHANNEL_ID_COMMAND],
				bcmtch_channel_flag &
				BCMTCH_CHANNEL_FLAG_USE_CMD_RESP);
		ret_val |= bcmtch_dev_init_channel(
				bcmtch_data_ptr,
				chan_set,
				BCMTCH_CHANNEL_RESPONSE,
				&p_chan_cfg[TOFE_CHANNEL_ID_RESPONSE],
				bcmtch_channel_flag &
				BCMTCH_CHANNEL_FLAG_USE_CMD_RESP);
		if (ret_val)
			BCMTCH_ERR(
				"%s: [%d] C/R Channel initialization failed!\n",
				__func__, ret_val);
	}

	/* Log channel */
	if (!ret_val) {
		ret_val = bcmtch_dev_init_channel(
				bcmtch_data_ptr,
				chan_set,
				BCMTCH_CHANNEL_LOG,
				&p_chan_cfg[TOFE_CHANNEL_ID_LOG],
				bcmtch_channel_flag &
				BCMTCH_CHANNEL_FLAG_USE_LOG);
		if (ret_val)
			BCMTCH_ERR(
				"%s: [%d] Log Channel initialization failed!\n",
				__func__, ret_val);
	}


	/* Initialize DMA buffer if there is any DMA mode channel */
	if (!ret_val &&
			bcmtch_data_ptr->has_dma_channel) {
		BCMTCH_DBG(
			BCMTCH_DF_CH,
			"CH:%s() dma buffer size=%d\n",
			__func__,
			bcmtch_data_ptr->fw_dma_buffer_size);
		p_buffer =
			kzalloc(
				bcmtch_data_ptr->fw_dma_buffer_size,
				GFP_KERNEL);
		if (p_buffer) {
			bcmtch_data_ptr->fw_dma_buffer = p_buffer;
		} else {
			BCMTCH_ERR("%s: [%d] DMA buffer allocation failed!\n",
				__func__, ret_val);
			bcmtch_data_ptr->fw_dma_buffer = NULL;
			ret_val = -ENOMEM;
		}
	}

	return ret_val;
}

static int32_t bcmtch_dev_write_channel(
		struct bcmtch_data *bcmtch_data_ptr,
		struct bcmtch_channel *chan)
{
	int32_t ret_val = 0;
	int16_t write_size;
	uint32_t sys_addr;

	/* read channel header and data all-at-once : need combined size */
	write_size = sizeof(struct tofe_channel_buffer_header)
			+ (chan->cfg.entry_num * chan->cfg.entry_size);

	sys_addr = (uint32_t)chan->cfg.channel_data;

	/* write channel header & channel data buffer */
	ret_val = bcmtch_com_write_sys(
				bcmtch_data_ptr,
				sys_addr,
				write_size,
				(uint8_t *)chan->hdr.buffer[0]);
	if (ret_val) {
		BCMTCH_ERR("BCMTOUCH: %s() write_sys err addr=0x%08x, rv=%d\n",
				__func__,
				sys_addr,
				ret_val);
	}
	return ret_val;
}

static int32_t bcmtch_dev_sync_channel(
		struct bcmtch_data *bcmtch_data_ptr,
		struct bcmtch_channel *chan)
{
	int32_t ret_val = 0;
	uint16_t read_size;
	uint32_t sys_addr;
	struct tofe_channel_header sync_hdr;

	/* Read channel header from firmware */
	read_size = sizeof(struct tofe_channel_header);
	sys_addr = (uint32_t)chan->cfg.channel_header;
	ret_val = bcmtch_com_read_sys(
				bcmtch_data_ptr,
				(uint32_t)chan->cfg.channel_header,
				read_size,
				(uint8_t *)&sync_hdr);
	if (ret_val) {
		BCMTCH_ERR("BCMTOUCH: %s() read hdr err addr=0x%08x, rv=%d\n",
				__func__,
				sys_addr,
				ret_val);
		return ret_val;
	}


	/* Read channel */
	read_size = sizeof(struct tofe_channel_buffer_header)
			+ (chan->cfg.entry_num * chan->cfg.entry_size);
	sys_addr = (uint32_t)(sync_hdr.buffer_idx > 0 ?
				(char *)chan->cfg.channel_data :
				(char *)chan->cfg.channel_data
					+ chan->cfg.offset);
	ret_val = bcmtch_com_read_sys(
				bcmtch_data_ptr,
				sys_addr,
				read_size,
				(uint8_t *)chan->hdr.buffer[0]);
	if (ret_val) {
		BCMTCH_ERR(
			"BCMTOUCH: %s() read buffer err addr=0x%08x, rv=%d\n",
			__func__,
			sys_addr,
			ret_val);
		return ret_val;
	}
	chan->queued = bcmtch_channel_num_queued(&chan->hdr);

	/* Sync the channel header */
	chan->hdr.buffer_idx = sync_hdr.buffer_idx;
	if (bcmtch_data_ptr->channel_set == BCMTCH_ROM_CHANNELS)
		chan->hdr.seq_count = sync_hdr.seq_count;
	else
		chan->hdr.seq_count = sync_hdr.seq_count + 1;

	return ret_val;
}

static int32_t bcmtch_dev_read_channel(
		struct bcmtch_data *bcmtch_data_ptr,
		struct bcmtch_channel *chan)
{
	int32_t ret_val = 0;
	uint8_t buffer_idx = chan->hdr.buffer_idx;
	uint8_t seq_count = chan->hdr.seq_count;
	struct tofe_channel_buffer_header *buff =
				&chan->hdr.buffer[1]->header;
	uint16_t read_size;
	uint32_t sys_addr;

	/* channel buffer size: buffer header + entries */
	read_size = sizeof(struct tofe_channel_buffer_header)
			+ (chan->cfg.entry_num * chan->cfg.entry_size);

	sys_addr = (uint32_t)(buffer_idx == 0 ?
				(char *)chan->cfg.channel_data :
				(char *)chan->cfg.channel_data
					+ chan->cfg.offset);

	/* read channel header & channel data buffer */
	ret_val = bcmtch_com_read_sys(
				bcmtch_data_ptr,
				sys_addr,
				read_size,
				(uint8_t *)chan->hdr.buffer[1]);
	if (ret_val) {
		BCMTCH_ERR("BCMTOUCH: %s() read_sys err addr=0x%08x, rv=%d\n",
				__func__,
				sys_addr,
				ret_val);
		return ret_val;
	}

	/* check if data corrupted */
	if (!bcmtch_dev_verify_buffer_header(bcmtch_data_ptr,
			buff)) {
		BCMTCH_DBG(
			BCMTCH_DF_CH,
			"CH:%s() ch=%d buffer data corrupted!\n",
			__func__,
			chan->hdr.channel_id);
		return -EIO;
	}

	if (buff->flags & TOFE_CHANNEL_FLAG_STATUS_OVERFLOW)
		BCMTCH_DBG(
			BCMTCH_DF_CH,
			"CH:%s() ch=%d channel overflow\n",
			__func__,
			chan->hdr.channel_id);

	if (buff->seq_number != seq_count) {
		BCMTCH_DBG(
			BCMTCH_DF_CH,
			"CH:%s() host ch=%d seq=%d, fw seq=%d NOT Matched!!\n",
			__func__,
			buff->channel_id,
			seq_count,
			buff->seq_number);

		/* sync channel */
		ret_val =
			bcmtch_dev_sync_channel(bcmtch_data_ptr, chan);

		return ret_val;
	}

	/* Update channel header */
	chan->hdr.seq_count++;
	chan->hdr.buffer_idx = (buffer_idx > 0 ? 0 : 1);

	/* get count */
	chan->queued = bcmtch_channel_num_queued(&chan->hdr);

	return ret_val;
}

static uint32_t bcmtch_dev_read_dma_buffer(
	struct bcmtch_data *bcmtch_data_ptr)
{
	uint32_t read_size = 0;
	uint32_t dma_buff_size = bcmtch_data_ptr->fw_dma_buffer_size;
	uint8_t dma_reg = BCMTCH_SPM_REG_DMA_RFIFO;
	uint8_t *dma_buff = (uint8_t *)bcmtch_data_ptr->fw_dma_buffer;
	struct i2c_client *p_i2c = bcmtch_data_ptr->p_i2c_client_sys;
	struct tofe_dmac_header *p_dmac;

	/* setup I2C messages for DMA read request transaction */
	struct i2c_msg dma_request[2] = {
		/* write the RFIFO address */
		{.addr = p_i2c->addr,
			.flags = 0,
			.len = 1,
			.buf = &dma_reg},
		/* read RFIFO data */
		{.addr = p_i2c->addr,
			.flags = I2C_M_RD,
			.len = (uint32_t)sizeof(struct tofe_dmac_header),
			.buf = dma_buff}
	};

	/* Set I2C master to read from RFIFO */
	if (dma_buff_size && dma_buff) {
		/* 1st I2C read dmac header */
		if (i2c_transfer(p_i2c->adapter, dma_request, 2) != 2) {
			BCMTCH_ERR("%s: I2C transfer error.\n",
					__func__);
			return 0;
		} else {
			p_dmac = (struct tofe_dmac_header *)dma_buff;
			read_size = (uint32_t)p_dmac->size;
			BCMTCH_DBG(
				BCMTCH_DF_CH,
				"CH:DMA buffer read size=%d min_size=%d.\n",
				read_size,
				p_dmac->min_size);

			if (read_size > dma_buff_size) {
				BCMTCH_ERR(
					"%s: DMA read overflow buffer [%d].\n",
					__func__,
					dma_buff_size);
				return 0;
			} else if (read_size <
					sizeof(struct tofe_dmac_header))
				return 0;

			/* 2nd I2C read entire DMA buffer */
			dma_request[1].len =
				read_size
				- sizeof(struct tofe_dmac_header);
			dma_request[1].buf =
				(uint8_t *)dma_buff
				+ sizeof(struct tofe_dmac_header);
			if (i2c_transfer(p_i2c->adapter, dma_request, 2) != 2) {
				BCMTCH_ERR("%s: I2C transfer error.\n",
						__func__);
				return 0;
			}
		}
	} else {
		BCMTCH_ERR("%s: DMA buffer/size is NULL.\n",
				__func__);
	}
	return read_size;
}

static inline bool bcmtch_dev_verify_buffer_header(
		struct bcmtch_data *bcmtch_data_ptr,
		struct tofe_channel_buffer_header *buff)
{
	uint8_t channel;
	uint8_t chan_set = bcmtch_data_ptr->channel_set;
	struct bcmtch_channel *p_chan;
	bool ret_val = true;

	p_chan = NULL;
	channel = (uint8_t)buff->channel_id;
	if (channel >= BCMTCH_CHANNEL_MAX)
		ret_val = false;
	else {
		p_chan = bcmtch_data_ptr->p_channels[chan_set][channel];
		if (!p_chan ||
			(buff->entry_size != p_chan->cfg.entry_size))
			ret_val = false;
	}

	if (ret_val == false)
		BCMTCH_DBG(
			BCMTCH_DF_CH,
			"CH:ERROR : id=%d entry_size=%d [%d]\n",
			buff->channel_id,
			buff->entry_size,
			(p_chan) ? p_chan->cfg.entry_size : -1);

	return ret_val;
}

static int32_t bcmtch_dev_read_dma_channels(
	struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;

	uint32_t read_size;
	uint32_t read_head;
	uint32_t offset;
	uint32_t channel;

	uint8_t *p_dma = (uint8_t *)bcmtch_data_ptr->fw_dma_buffer;
	uint8_t chan_set = bcmtch_data_ptr->channel_set;
	struct tofe_channel_buffer_header *buff;
	struct tofe_channel_header *hdr;

	/* Read DMA buffer via I2C */
	read_size = bcmtch_dev_read_dma_buffer(bcmtch_data_ptr);
	BCMTCH_DBG(BCMTCH_DF_CH, "CH:%s: read DMA buffer %d bytes.\n",
		__func__,
		read_size);

	if (read_size > bcmtch_data_ptr->fw_dma_buffer_size) {
		BCMTCH_ERR("%s: Invalid DMA data read size %d.\n",
				__func__,
				read_size);
		return -EIO;
	}

	/* Parse DMA buffer for channels */
	read_head = 0;
	while (read_head < read_size) {
		buff = (struct tofe_channel_buffer_header *)p_dma;
		if (!bcmtch_dev_verify_buffer_header(bcmtch_data_ptr,
				buff)) {
			BCMTCH_ERR(
				"%s: corrupted buffer header in DMA channel!\n",
				__func__);
			return -EIO;
		}

		channel = buff->channel_id;
		BCMTCH_DBG(
			BCMTCH_DF_CH,
			"CH:%s: parsing channel [%d] min_size=%d\n",
			__func__,
			channel,
			buff->dmac.min_size);

		hdr = &bcmtch_data_ptr->p_channels[chan_set][channel]->hdr;
		if (hdr->flags & TOFE_CHANNEL_FLAG_FWDMA_ENABLE)
			hdr->buffer[0] = (struct tofe_channel_buffer *)p_dma;

		offset = (uint32_t)(buff->dmac.min_size ? buff->dmac.min_size :
				(buff->entry_size * buff->entry_count)
				+ sizeof(struct tofe_channel_buffer_header));
		read_head += offset;
		p_dma += offset;
	}

	return ret_val;
}

#if BCMTCH_STATE_PROTOCOL

static int32_t bcmtch_dev_parse_response_data(
			struct bcmtch_data *bcmtch_data_ptr,
			uint8_t *data,
			uint16_t data_size)
{
	int32_t ret_val = 0;
	uint32_t slot_size =
		(uint32_t)
		bcmtch_data_ptr->scan_data.touch_slot_size;
	uint32_t slot_num =
		(uint32_t)
		bcmtch_data_ptr->scan_data.status.slots;

	/* Touch slot format - 0: long slot, 1: short slot */
	uint8_t	slot_format =
				bcmtch_data_ptr->scan_data.cmd
					.touch_slot_format;
	struct input_dev *input_dev_ptr =
				bcmtch_data_ptr->p_input_device;
	uint32_t button_index = 0;
	uint8_t *data_ptr = data;
	uint8_t slot_type;
	uint16_t tmp_axis;
	uint16_t button_check;
	uint16_t evt_status;
	uint16_t btn_status;
	uint32_t touch_index;
	union bcmtch_state_resp_slot *slot;
	struct bcmtch_touch	*touch_ptr;

	int32_t ao_flag =
			bcmtch_data_ptr->platform_data.axis_orientation_flag;

	for (touch_index = 0;
			touch_index < slot_num;
			data_ptr += slot_size,
			touch_index++) {
		slot = (union bcmtch_state_resp_slot *) data_ptr;
		touch_ptr = &bcmtch_data_ptr->touch[touch_index];

		/* Categorize slot type */
		if (slot->type > 7) {
			slot_type = STATE_SLOT_TYPE_FINGER;
			touch_ptr->type = MT_TOOL_FINGER;
		} else if (slot->type > 1) {
			slot_type = STATE_SLOT_TYPE_STYLUS;
			touch_ptr->type = MT_TOOL_PEN;
		} else if (slot->type == 1)
			slot_type = STATE_SLOT_TYPE_BUTTON;
		else
			slot_type = STATE_SLOT_TYPE_EMPTY;

		/* Process slots */
		switch (slot_type) {
		case STATE_SLOT_TYPE_EMPTY:
			touch_ptr->status = BCMTCH_TOUCH_STATUS_UP;
			break;
		case STATE_SLOT_TYPE_BUTTON:
			btn_status = bcmtch_data_ptr->button_status;
			evt_status = (uint16_t)(slot->button_slot.buttons);
			if (btn_status != evt_status) {
				BCMTCH_DBG(
					BCMTCH_DF_ST,
					"ST:BTN: 0x%0x\n",
					evt_status);

				while (button_index < bcmtch_data_ptr->
					  platform_data.ext_button_count) {
					button_check = (0x1 << button_index);
					if ((btn_status & button_check) !=
						(evt_status & button_check)) {
						input_report_key(
							input_dev_ptr,
							bcmtch_data_ptr->
							platform_data.
							ext_button_map
							[button_index],
							(evt_status &
							button_check));
					}
					button_index++;
				}

				/* Update status */
				bcmtch_data_ptr->button_status = evt_status;
			}
			break;
		case STATE_SLOT_TYPE_STYLUS:
			/* Not support yet */
		case STATE_SLOT_TYPE_FINGER:
			if (slot_format) {
				/* short slot */
				touch_ptr->x = slot->short_slot.x;
				touch_ptr->y = slot->short_slot.y;
				touch_ptr->pressure = 0;
				touch_ptr->orientation = 0;
				touch_ptr->major_axis = 0;
				touch_ptr->minor_axis = 0;
			} else {
				/* long slot */
				touch_ptr->x = slot->long_slot.x;
				touch_ptr->y = slot->long_slot.y;
				touch_ptr->pressure = slot->long_slot.pressure;
				touch_ptr->orientation =
					slot->long_slot.orientation;
				touch_ptr->major_axis =
					slot->long_slot.major
					<< BCMTCH_AXIS_SHIFT_BITS;
				touch_ptr->minor_axis =
					slot->long_slot.minor
					<< BCMTCH_AXIS_SHIFT_BITS;
			}


			/* axis reverse adjust */
			if (ao_flag & BCMTCH_AXIS_FLAG_X_REVERSED_MASK)
				touch_ptr->x =
					bcmtch_data_ptr->axis_x_max
					- touch_ptr->x;

			if (ao_flag & BCMTCH_AXIS_FLAG_Y_REVERSED_MASK)
				touch_ptr->y =
					bcmtch_data_ptr->axis_y_max
					- touch_ptr->y;

			if (ao_flag & BCMTCH_AXIS_FLAG_X_Y_SWAPPED_MASK) {
				tmp_axis = touch_ptr->x;
				touch_ptr->x = touch_ptr->y;
				touch_ptr->y = tmp_axis;
			}

			if (bcmtch_data_ptr->chip_id == 0x015200) {
				touch_ptr->y =
					bcmtch_data_ptr->axis_y_max
					- touch_ptr->y;
			}

			touch_ptr->status = BCMTCH_TOUCH_STATUS_MOVING;

			BCMTCH_DBG(
				BCMTCH_DF_ST,
				"ST: SLOT %d: X=%d Y=%d P=%d O=%d MAJX=%d MINX=%d T=%d\n",
				touch_index,
				touch_ptr->x,
				touch_ptr->y,
				touch_ptr->pressure,
				touch_ptr->orientation,
				touch_ptr->major_axis,
				touch_ptr->minor_axis,
				touch_ptr->type);
			break;
		default:
			BCMTCH_ERR("ST: SLOT %d: Unknown",
					touch_index);
		}
	}

	/* Report touch event to kernel */
	ret_val = bcmtch_dev_sync_event_frame(bcmtch_data_ptr);

	return ret_val;
}

static int32_t bcmtch_dev_read_scan_touches(
			struct bcmtch_data *bcmtch_data_ptr,
			uint8_t slots)
{
	int32_t ret_val = 0;
	uint16_t data_size;
	uint8_t *dma_buff = bcmtch_data_ptr->bcmtch_state_resp_buffer;

	if (!slots)
		return ret_val;

	data_size = slots * bcmtch_data_ptr->scan_data.touch_slot_size;
	if (data_size > TOFE_HOST_RSP_BUF_SIZE) {
		BCMTCH_ERR(
			"ST: data size to read exceed response buffer.");
		BCMTCH_ERR(
			"ST:	slots=%d data_size=%d resp_buf_size=%d",
			slots,
			data_size,
			TOFE_HOST_RSP_BUF_SIZE);
		return -EINVAL;
	}

	memset(dma_buff, 0, data_size);
	BCMTCH_DBG(
		BCMTCH_DF_ST,
		"ST: %s read %d touch slots size=%d",
		__func__,
		slots,
		data_size);

	ret_val =
		bcmtch_com_read_dma(
			bcmtch_data_ptr,
			data_size + 1,
			 dma_buff);
	if (ret_val) {
		BCMTCH_ERR("%s: DMA read error.\n", __func__);
		 return ret_val;
	}

    /* parse touch data: Slot 0 ...  Slot N-1 */
	bcmtch_dev_parse_response_data(
		bcmtch_data_ptr,
		&dma_buff[1],
		data_size);

	return ret_val;
}
#endif /* BCMTCH_STATE_PROTOCOL */

static int32_t bcmtch_dev_read_channels(
	struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;
	uint32_t channel = 0;
	uint8_t chan_set = bcmtch_data_ptr->channel_set;

	while (channel < BCMTCH_CHANNEL_MAX) {
		if (bcmtch_data_ptr->p_channels[chan_set][channel]->active &&
			!(bcmtch_data_ptr->
				p_channels[chan_set][channel]->cfg.flags &
				(TOFE_CHANNEL_FLAG_INBOUND
				 | TOFE_CHANNEL_FLAG_FWDMA_ENABLE))) {
			ret_val = bcmtch_dev_read_channel(
				bcmtch_data_ptr,
				bcmtch_data_ptr->p_channels[chan_set][channel]);
		}
		channel++;
	}

	return ret_val;
}

static int32_t bcmtch_dev_process_event_frame(
	struct bcmtch_data *bcmtch_data_ptr,
	struct bcmtch_event_frame *p_frame_event)
{
	int32_t ret_val = 0;

	BCMTCH_DBG(BCMTCH_DF_FR, "FR:T=%d ID=%d TS=%d\n",
		bcmtch_data_ptr->touch_count,
		p_frame_event->frame_id,
		p_frame_event->timestamp);

	return ret_val;
}

static int32_t bcmtch_dev_process_event_frame_extension(
		struct bcmtch_event_frame_extension
		*p_frame_event_extension)
{
	int32_t ret_val = 0;
	struct bcmtch_event_frame_extension_timestamp *timestamp;
	struct bcmtch_event_frame_extension_checksum  *checksum;

	switch (p_frame_event_extension->frame_kind) {
	case BCMTCH_EVENT_FRAME_EXTENSION_KIND_TIMESTAMP:
		timestamp = (struct bcmtch_event_frame_extension_timestamp *)
			p_frame_event_extension;

		BCMTCH_DBG(
			BCMTCH_DF_FE,
			"FE:Time offsets. %d %d %d",
			timestamp->scan_end,
			timestamp->mtc_start,
			timestamp->mtc_end);
		break;
	case BCMTCH_EVENT_FRAME_EXTENSION_KIND_CHECKSUM:
		checksum =
			(struct bcmtch_event_frame_extension_checksum *)
			p_frame_event_extension;

		BCMTCH_DBG(
			BCMTCH_DF_FE,
			"FE:ERROR: Checksum not supported.  %#x",
			checksum->hash);
		break;
	case BCMTCH_EVENT_FRAME_EXTENSION_KIND_HEARTBEAT:
		BCMTCH_DBG(
			BCMTCH_DF_FE,
			"FE:ERROR: Heartbeat not supported.");
		break;
	default:
		BCMTCH_DBG(
			BCMTCH_DF_FE,
			"FE:Invalid frame extension. %d",
			p_frame_event_extension->frame_kind);
		break;
	}

	return ret_val;
}


static int32_t bcmtch_dev_sync_event_frame(
		struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;

	struct input_dev *input_dev_ptr =
		bcmtch_data_ptr->p_input_device;
	struct bcmtch_touch *touch_ptr;
	uint32_t num_touches = 0;
	uint32_t touch_index = 0;

	for (touch_index = 0; touch_index < ARRAY_SIZE(bcmtch_data_ptr->touch);
			touch_index++) {
		touch_ptr =
			(struct bcmtch_touch *)
			&bcmtch_data_ptr->touch[touch_index];

		input_mt_slot(input_dev_ptr, touch_index);
		input_mt_report_slot_state(
			input_dev_ptr,
			touch_ptr->type,
			(touch_ptr->status > BCMTCH_TOUCH_STATUS_UP));

		if (touch_ptr->status > BCMTCH_TOUCH_STATUS_UP) {
			/* Count both of STATUS_MOVE and STATUS_MOVING */
			num_touches++;

			if (touch_ptr->status > BCMTCH_TOUCH_STATUS_MOVE) {

				input_report_abs(
						input_dev_ptr,
						ABS_MT_POSITION_X,
						touch_ptr->x);

				input_report_abs(
						input_dev_ptr,
						ABS_MT_POSITION_Y,
						touch_ptr->y);

				if (bcmtch_event_flag &
						BCMTCH_EVENT_FLAG_PRESSURE) {
					input_report_abs(
							input_dev_ptr,
							ABS_MT_PRESSURE,
							touch_ptr->pressure);
				}

				if (bcmtch_event_flag &
						BCMTCH_EVENT_FLAG_TOUCH_SIZE) {
					input_report_abs(
							input_dev_ptr,
							ABS_MT_TOUCH_MAJOR,
							touch_ptr->major_axis);

					input_report_abs(
							input_dev_ptr,
							ABS_MT_TOUCH_MINOR,
							touch_ptr->minor_axis);
				}

				if (bcmtch_event_flag &
						BCMTCH_EVENT_FLAG_ORIENTATION) {
					input_report_abs(
							input_dev_ptr,
							ABS_MT_ORIENTATION,
							touch_ptr->orientation);
				}

				if (bcmtch_event_flag &
						BCMTCH_EVENT_FLAG_TOOL_SIZE) {
					input_report_abs(
							input_dev_ptr,
							ABS_MT_WIDTH_MAJOR,
							touch_ptr->width_major);

					input_report_abs(
							input_dev_ptr,
							ABS_MT_WIDTH_MINOR,
							touch_ptr->width_minor);
				}

				/* reset the status from MOVING to MOVE. */
				touch_ptr->status = BCMTCH_TOUCH_STATUS_MOVE;
			}
		}
	}

	input_report_key(input_dev_ptr, BTN_TOUCH, (num_touches > 0));
	input_sync(input_dev_ptr);

	/* remember */
	bcmtch_data_ptr->touch_count = num_touches;

	return ret_val;
}

static void
bcmtch_dev_process_event_touch_extension(
		struct bcmtch_data *bcmtch_data_ptr,
		struct bcmtch_event_touch_extension *extension,
		uint8_t track_id)
{
	char *tool_str;
	struct bcmtch_event_touch_extension_detail *detail;
	struct bcmtch_event_touch_extension_blob *blob;
	struct bcmtch_event_touch_extension_size *size;
	struct bcmtch_event_touch_extension_tool *tool;
	struct bcmtch_touch *touch_ptr =
		(struct bcmtch_touch *)&bcmtch_data_ptr->touch[track_id];

	switch (extension->touch_kind) {
	case BCMTCH_EVENT_TOUCH_EXTENSION_KIND_DETAIL:
		detail =
			(struct bcmtch_event_touch_extension_detail *)
			extension;

		/* logic is inverted */
		if (detail->tool == BCMTCH_EVENT_TOUCH_TOOL_FINGER) {
			touch_ptr->type = MT_TOOL_PEN;
			tool_str = "stylus";
		} else {
			touch_ptr->type = MT_TOOL_FINGER;
			tool_str = "finger";
		}

		BCMTCH_DBG(
			BCMTCH_DF_TE,
			"TE:C%d:S%d:H%d %s(%d) Pres=%d Ornt=%#x",
			detail->confident,
			detail->suppressed,
			detail->hover,
			tool_str,
			detail->tool,
			detail->pressure,
			detail->orientation);
		/**
		 * ABS_MT_TOOL_TYPE
		 * - MT_TOOL_FINGER
		 * - MT_TOOL_PEN
		 **/
		touch_ptr->pressure = detail->pressure;

		/* get orientation
		 * - handle int12 to int16 conversion
		 */
		touch_ptr->orientation = detail->orientation;
		if (touch_ptr->orientation & (1<<11))
			touch_ptr->orientation -= 1<<12;
		break;

	case BCMTCH_EVENT_TOUCH_EXTENSION_KIND_BLOB:
		blob =
			(struct bcmtch_event_touch_extension_blob *)
			extension;
		BCMTCH_DBG(BCMTCH_DF_TE, "TE:Area=%d TCap=%d",
			blob->area, blob->total_cap);
		/**
		 * ABS_MT_BLOB_ID
		 */
		break;

	case BCMTCH_EVENT_TOUCH_EXTENSION_KIND_SIZE:
		size =
			(struct bcmtch_event_touch_extension_size *)
			extension;
		BCMTCH_DBG(
			BCMTCH_DF_TE,
			"TE:Track %d:\tMajor=%d Minor=%d",
			track_id,
			size->major_axis,
			size->minor_axis);
		/**
		 * ABS_MT_MAJOR/MINOR_AXIS
		 */
		touch_ptr->major_axis =
			size->major_axis << BCMTCH_AXIS_SHIFT_BITS;
		touch_ptr->minor_axis =
			size->minor_axis << BCMTCH_AXIS_SHIFT_BITS;
		break;

	case BCMTCH_EVENT_TOUCH_EXTENSION_KIND_HOVER:
		BCMTCH_DBG(
			BCMTCH_DF_TE,
			"TE:ERROR:Hover not supported.");
		break;

	case BCMTCH_EVENT_TOUCH_EXTENSION_KIND_TOOL:
		tool =
			(struct bcmtch_event_touch_extension_tool *)
			extension;
		BCMTCH_DBG(
			BCMTCH_DF_TE,
			"TE:Track %d:\tMajor=%d Minor=%d",
			track_id,
			tool->width_major,
			tool->width_minor);
		/**
		 * ABS_MT_MAJOR/MINOR_AXIS
		 */
		touch_ptr->width_major = tool->width_major;
		touch_ptr->width_minor = tool->width_minor;
		break;
	default:
		BCMTCH_DBG(
			BCMTCH_DF_TE,
			"TE:Invalid touch extension. %d",
			extension->touch_kind);
		break;
	}
}


static int32_t bcmtch_dev_process_event_touch(
		struct bcmtch_data *bcmtch_data_ptr,
		struct bcmtch_event_touch *p_touch_event)
{
	int axis_orientation_flag;
	int32_t ret_val = 0;
	struct bcmtch_touch *p_touch;
	enum bcmtch_event_kind kind;

	if (p_touch_event->track_tag < BCMTCH_MAX_TOUCH) {
		axis_orientation_flag =
			bcmtch_data_ptr->platform_data.axis_orientation_flag;
		p_touch =
			(struct bcmtch_touch *)
			&bcmtch_data_ptr->touch[p_touch_event->track_tag];

		if (axis_orientation_flag & BCMTCH_AXIS_FLAG_X_REVERSED_MASK)
			p_touch_event->x =
				bcmtch_data_ptr->axis_x_max - p_touch_event->x;

		if ((axis_orientation_flag & BCMTCH_AXIS_FLAG_Y_REVERSED_MASK)
			||
			(bcmtch_data_ptr->chip_id == 0x015200))
			p_touch_event->y =
				bcmtch_data_ptr->axis_y_max - p_touch_event->y;

		if (axis_orientation_flag & BCMTCH_AXIS_FLAG_X_Y_SWAPPED_MASK) {
			p_touch->y = p_touch_event->x;
			p_touch->x = p_touch_event->y;
		} else {
			p_touch->x  = p_touch_event->x;
			p_touch->y  = p_touch_event->y;
		}

		kind = (enum bcmtch_event_kind)p_touch_event->event_kind;

		switch (kind) {
		case BCMTCH_EVENT_KIND_TOUCH:
			p_touch->event = kind;
			p_touch->status = BCMTCH_TOUCH_STATUS_MOVING;

			BCMTCH_DBG(
				BCMTCH_DF_MV,
				"MV:T%d: (%04x , %04x)\n",
				p_touch_event->track_tag,
				p_touch->x,
				p_touch->y);
			break;
		case BCMTCH_EVENT_KIND_TOUCH_END:
			p_touch->event = kind;
			p_touch->status = BCMTCH_TOUCH_STATUS_UP;

			BCMTCH_DBG(
				BCMTCH_DF_UP,
				"UP:T%d: (%04x , %04x)\n",
				p_touch_event->track_tag,
				p_touch->x,
				p_touch->y);
			break;
		default:
			BCMTCH_ERR("%s: Invalid touch event", __func__);
			break;
		}
	}

	return ret_val;
}

static int32_t bcmtch_dev_process_event_button(
		struct bcmtch_data *bcmtch_data_ptr,
		struct bcmtch_event_button *p_button_event)
{
	int32_t ret_val = 0;
	uint16_t evt_status = p_button_event->status;
	uint16_t btn_status = bcmtch_data_ptr->button_status;
	struct input_dev *input_dev_ptr =
		bcmtch_data_ptr->p_input_device;
	enum bcmtch_event_kind kind = p_button_event->button_kind;
	uint32_t button_index = 0;
	uint16_t button_check;

	if (btn_status != evt_status) {
		switch (kind) {
		case BCMTCH_EVENT_BUTTON_KIND_CONTACT:
			while (button_index < bcmtch_data_ptr->
				  platform_data.ext_button_count) {
				button_check = (0x1 << button_index);
				if ((btn_status & button_check) !=
						(evt_status & button_check)) {
					input_report_key(
						input_dev_ptr,
						bcmtch_data_ptr->platform_data.
						  ext_button_map[button_index],
						(evt_status & button_check));
				}
				button_index++;
			}

			BCMTCH_DBG(BCMTCH_DF_BT, "BT:%s %#04x\n",
				"press",
				evt_status);
			break;
		case BCMTCH_EVENT_BUTTON_KIND_HOVER:
			BCMTCH_DBG(BCMTCH_DF_BT, "BT:%s %#04x\n",
				"hover",
				evt_status);
			break;
		default:
			BCMTCH_ERR("%s: Invalid button kind %d\n",
				__func__,
				kind);
			break;
		}

		/* Report SYNC */
		input_sync(input_dev_ptr);

		/* Update status */
		bcmtch_data_ptr->button_status = evt_status;
	} else {
		BCMTCH_DBG(
			BCMTCH_DF_BT,
			"BT:unchanged. status=0x%04x\n",
			btn_status);
	}
	return ret_val;
}

/**
 * To process whole frames of data this variable should
 * be made global because one frame can be split across
 * two invocations of the function process_channel_touch().
 */
static	enum bcmtch_event_kind top_level_kind = BCMTCH_EVENT_KIND_EXTENSION;

static int32_t bcmtch_dev_process_channel_touch(
		struct bcmtch_data *bcmtch_data_ptr,
		struct bcmtch_channel *chan)
{
	int32_t ret_val = 0;
	bool syn_report_pending = false;
	uint16_t read_idx;
	struct bcmtch_event *ptch_event;
	enum bcmtch_event_kind kind;
	struct bcmtch_event_touch *ptouch_event = NULL;
	struct tofe_channel_header *chan_hdr =
		(struct tofe_channel_header *)&chan->hdr;

	uint32_t frames_in = 0;

	read_idx = 0;
	while ((ptch_event = (struct bcmtch_event *)
				bcmtch_inline_channel_read(chan_hdr,
				read_idx++))) {
		kind = (enum bcmtch_event_kind)ptch_event->event_kind;

		if (kind != BCMTCH_EVENT_KIND_EXTENSION) {
			top_level_kind = kind;

			if (syn_report_pending) {
				/**
				 * The end of frame extension events.
				 * Send the SYN_REPORT for the frame.
				 */
				bcmtch_dev_sync_event_frame(bcmtch_data_ptr);
				syn_report_pending = false;

				if (frames_in)
					usleep_range(1000, 1500);
			}
		}

		switch (kind) {
		case BCMTCH_EVENT_KIND_FRAME:
			/**
			 * Only set the flag to wait for the following frame extension events
			 * rather than directly send SYN_REPORT message.
			 */
			frames_in++;
			syn_report_pending = true;
		    bcmtch_dev_process_event_frame(
					bcmtch_data_ptr,
					(struct bcmtch_event_frame *)
					ptch_event);
			break;
		case BCMTCH_EVENT_KIND_TOUCH:
		case BCMTCH_EVENT_KIND_TOUCH_END:
			ptouch_event =
				(struct bcmtch_event_touch *)ptch_event;
			bcmtch_data_ptr->touch_event_track_id =
				ptouch_event->track_tag;
		    bcmtch_dev_process_event_touch(
				bcmtch_data_ptr,
				ptouch_event);
			break;
		case BCMTCH_EVENT_KIND_BUTTON:
			bcmtch_dev_process_event_button(
				bcmtch_data_ptr,
				(struct bcmtch_event_button *)ptch_event);
			break;
		case BCMTCH_EVENT_KIND_GESTURE:
			BCMTCH_INFO("ERROR: Gesture: NOT SUPPORTED\n");
			break;
		case BCMTCH_EVENT_KIND_EXTENSION:
			switch (top_level_kind) {
			case BCMTCH_EVENT_KIND_FRAME:
				bcmtch_dev_process_event_frame_extension(
					(struct bcmtch_event_frame_extension *)
					ptch_event);
				break;
			case BCMTCH_EVENT_KIND_TOUCH:
				bcmtch_dev_process_event_touch_extension(
					bcmtch_data_ptr,
					(struct bcmtch_event_touch_extension *)
					ptch_event,
					bcmtch_data_ptr->touch_event_track_id);
				break;
			default:
				BCMTCH_INFO(
						"ERROR: Improper event extension for: tlk=%d k=%d\n",
							top_level_kind, kind);
				break;
			}
			break;
		default:
			BCMTCH_INFO("ERROR: Invalid event kind: %d\n", kind);
		}
	}

    /**
     * The last event in the channel is a frame (extension) event.
     * Send the SYN_REPORT for the frame.
     */
	if (syn_report_pending) {
		bcmtch_dev_sync_event_frame(bcmtch_data_ptr);
		syn_report_pending = false;
	}

	BCMTCH_DBG(BCMTCH_DF_FR, "FR:%d", frames_in);

	return ret_val;
}

static int32_t bcmtch_dev_process_channel_response(
		struct bcmtch_data *bcmtch_data_ptr,
		struct bcmtch_channel *chan)
{
	int32_t ret_val = 0;
	uint16_t read_idx;
	struct bcmtch_response_wait *p_resp;
	struct tofe_command_response *resp_event;
	struct tofe_channel_header *chan_hdr =
		(struct tofe_channel_header *)&chan->hdr;

	BCMTCH_DBG(
		BCMTCH_DF_CH,
		"CH:%s() - swap count=%d response evt count=%d.\n",
		__func__,
		chan_hdr->seq_count,
		chan->queued);

	read_idx = 0;
	/* Process response events */
	while ((resp_event =
				(struct tofe_command_response *)
				bcmtch_inline_channel_read(chan_hdr,
				read_idx++))) {

		if (resp_event->flags &
				TOFE_COMMAND_FLAG_COMMAND_PROCESSED) {
			if (resp_event->command > TOFE_COMMAND_LAST)
				continue;

			/* Save the response result */
			p_resp = (struct bcmtch_response_wait *)
				&(bcmtch_data_ptr->bcmtch_cmd_response
						[resp_event->command]);
			p_resp->wait = 0;
			p_resp->resp_data = resp_event->data;
		}

		BCMTCH_DBG(
			BCMTCH_DF_CH,
			"CH:Response - command=0x%02x result=0x%04x data=0x%08x.\n",
			resp_event->command,
			resp_event->result,
			resp_event->data);
	}
	return ret_val;
}

/**
    Log routine for printing log message in TOFE

    @param
		[in] code 16-bit log code
		[in] param_0 16-bit parameter
		[in] param_1 32-bit parameter
		[in] param_2 32-bit parameter

    @retval
		none
*/
static void bcmtch_log_str_print(uint16_t code, uint16_t param0,
					uint32_t param1,
					uint32_t param2,
					uint32_t timestamp)
{
		BCMTCH_INFO(
			" code:0x%04x para0:0x%04x para1:0x%08x para2:0x%08x ts:0x%08x\n",
			code,
			param0,
			param1,
			param2,
			timestamp);
}

static int32_t bcmtch_dev_process_channel_log(struct bcmtch_channel *chan)
{
	int32_t ret_val = 0;
	uint16_t read_idx;
	struct tofe_log_msg *log_msg;

	struct tofe_channel_header *chan_hdr =
		(struct tofe_channel_header *)&chan->hdr;

	read_idx = 0;
	while ((log_msg =
				(struct tofe_log_msg *)
				bcmtch_inline_channel_read(chan_hdr,
				read_idx++))) {
		bcmtch_log_str_print(log_msg->log_code,
			log_msg->param_0,
			log_msg->params[0],
			log_msg->params[1],
			log_msg->timestamp);
	}
	return ret_val;
}


static int32_t bcmtch_dev_process_channels(
		struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;
	uint32_t channel = 0;
	uint8_t chan_set = bcmtch_data_ptr->channel_set;
	struct bcmtch_channel *p_chan = NULL;

	while (channel < BCMTCH_CHANNEL_MAX) {
		p_chan = bcmtch_data_ptr->p_channels[chan_set][channel];
		if (!p_chan->active) {
			channel++;
			continue;
		}

		switch (channel) {
		case BCMTCH_CHANNEL_TOUCH:
			bcmtch_dev_process_channel_touch(
				bcmtch_data_ptr,
				p_chan);
			break;

		case BCMTCH_CHANNEL_COMMAND:
			break;

		case BCMTCH_CHANNEL_RESPONSE:
			bcmtch_dev_process_channel_response(
				bcmtch_data_ptr,
				p_chan);
			break;

		case BCMTCH_CHANNEL_LOG:
			bcmtch_dev_process_channel_log(p_chan);
			break;


		default:
			break;
		}

		if (p_chan->cfg.flags & TOFE_CHANNEL_FLAG_FWDMA_ENABLE)
			p_chan->hdr.buffer[0] = p_chan->hdr.buffer[1];

		channel++;
	}

	return ret_val;
}

static int32_t bcmtch_dev_wait_for_firmware_ready(
		struct bcmtch_data *bcmtch_data_ptr,
		int32_t count)
{
	int32_t ret_val = 0;
	uint8_t ready;

	do {
		ret_val =
			bcmtch_com_read_spm(
				bcmtch_data_ptr,
				BCMTCH_SPM_REG_MSG_TO_HOST, &ready);
	} while ((!ret_val) && !(ready & TOFE_MESSAGE_FW_READY) && (count--));

	if (count <= 0) {
		BCMTCH_ERR(
			"ERROR: Failed to communicate with Napa FW. Error: 0x%x\n",
			ready);
		ret_val = -1;
	}

	return ret_val;
}

static int32_t bcmtch_dev_run_firmware(
		struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;

	ret_val = bcmtch_dev_reset(bcmtch_data_ptr,
		BCMTCH_RESET_MODE_SOFT_CLEAR);

	/* set DLDO output */
	if (!ret_val)
		ret_val = bcmtch_com_write_spm(
					bcmtch_data_ptr,
					BCMTCH_SPM_REG_PMU_CONTROL2,
					BCMTCH_PMU_CNTL2_DLDO_1_1V);

	if (bcmtch_dev_wait_for_firmware_ready(
			bcmtch_data_ptr,
			BCMTCH_FW_READY_WAIT)) {
		uint8_t xaddr = 0x40;
		uint8_t xdata;
		while (xaddr <= 0x61) {
			bcmtch_com_read_spm(bcmtch_data_ptr,
				xaddr, &xdata);
			BCMTCH_ERR("%s: addr = 0x%02x  data = 0x%02x\n",
				__func__,
				xaddr++,
				xdata);
		}
	}

	return ret_val;
}

static int32_t bcmtch_dev_parse_firmware(
				struct bcmtch_data *bcmtch_data_ptr,
				const struct firmware *p_fw)
{
	int32_t ret_val = 0;
	uint32_t entry_id = 1;
	uint32_t *p_cfg = NULL;
	uint8_t *mem_data = NULL;

	struct tofe_signature *p_tofe_sig = NULL;
	struct mtc_detect_cfg *p_mtc_cfg = NULL;
	struct combi_entry *p_entry = (struct combi_entry *) p_fw->data;

	while (p_entry[entry_id].length) {

		if (p_entry[entry_id].flags & BCMTCH_FIRMWARE_FLAGS_ROM_BOOT)
			bcmtch_data_ptr->boot_from_rom = true;

		switch (p_entry[entry_id].flags & BCMTCH_FIRMWARE_FLAGS_MASK) {
		case BCMTCH_FIRMWARE_FLAGS_POST_BOOT_CONFIGS:
			mem_data = (uint8_t *)
						((uint32_t)p_fw->data
						+ p_entry[entry_id].offset);
			p_cfg = (uint32_t *)(mem_data + TOFE_SIGNATURE_SIZE);
			p_tofe_sig = (struct tofe_signature *)mem_data;

			/* Parse F/W Signature */
			bcmtch_data_ptr->fw_signature = *p_tofe_sig;

			/* Parse MTC parameters */
			p_mtc_cfg =
				(struct mtc_detect_cfg *)
				((uint32_t)mem_data
					+ p_cfg[TOFE_TOC_INDEX_DETECT]
					- BCMTCH_ADDR_TOC_BASE);

			if (p_mtc_cfg) {
				bcmtch_data_ptr->axis_x_max =
					(p_mtc_cfg->scaling_x_range
						> BCMTCH_AXIS_MAX) ?
					BCMTCH_AXIS_MAX :
					p_mtc_cfg->scaling_x_range - 1;

				bcmtch_data_ptr->axis_y_max =
					(p_mtc_cfg->scaling_y_range
						> BCMTCH_AXIS_MAX) ?
					BCMTCH_AXIS_MAX :
					p_mtc_cfg->scaling_y_range - 1;

				bcmtch_data_ptr->axis_h_max =
					bcmtch_max_h_axis(
						bcmtch_data_ptr->axis_x_max,
						bcmtch_data_ptr->axis_y_max);

				bcmtch_data_ptr->threshold_gate_finger =
					p_mtc_cfg->class_finger_gate;
				bcmtch_data_ptr->threshold_gate_stylus =
					p_mtc_cfg->class_stylus_gate;
			}

			/* Check channel/state protocol mode */
			if (!p_cfg[TOFE_TOC_INDEX_CHANNEL])
				bcmtch_boot_flag |= BCMTCH_BF_STATE_PROTOCOL;
			else
				bcmtch_boot_flag &= ~BCMTCH_BF_STATE_PROTOCOL;
			break;

		case BCMTCH_FIRMWARE_FLAGS_POST_BOOT_CODE:
			break;

		case BCMTCH_FIRMWARE_FLAGS_CONFIGS:
			mem_data = (uint8_t *)
						((uint32_t)p_fw->data
						+ p_entry[entry_id].offset);

			p_cfg = (uint32_t *)(mem_data + TOFE_SIGNATURE_SIZE);
			p_tofe_sig = (struct tofe_signature *)mem_data;

			/* Parse F/W Signature */
			bcmtch_data_ptr->fw_signature = *p_tofe_sig;

			/* Parse MTC parameters */
			p_mtc_cfg =
				(struct mtc_detect_cfg *)
				((uint32_t)mem_data
					+ p_cfg[TOFE_TOC_INDEX_DETECT]
					- p_entry[entry_id].addr);

			if (p_mtc_cfg) {
				bcmtch_data_ptr->axis_x_max =
					(p_mtc_cfg->scaling_x_range
						> BCMTCH_AXIS_MAX) ?
					BCMTCH_AXIS_MAX :
					p_mtc_cfg->scaling_x_range - 1;

				bcmtch_data_ptr->axis_y_max =
					(p_mtc_cfg->scaling_y_range
						> BCMTCH_AXIS_MAX) ?
					BCMTCH_AXIS_MAX :
					p_mtc_cfg->scaling_y_range - 1;

				bcmtch_data_ptr->axis_h_max =
					bcmtch_max_h_axis(
						bcmtch_data_ptr->axis_x_max,
						bcmtch_data_ptr->axis_y_max);

				bcmtch_data_ptr->threshold_gate_finger =
					p_mtc_cfg->class_finger_gate;
				bcmtch_data_ptr->threshold_gate_stylus =
					p_mtc_cfg->class_stylus_gate;
			}

			/* Check channel/state protocol mode */
			if (!p_cfg[TOFE_TOC_INDEX_CHANNEL])
				bcmtch_boot_flag |= BCMTCH_BF_STATE_PROTOCOL;
			else
				bcmtch_boot_flag &= ~BCMTCH_BF_STATE_PROTOCOL;
			break;

		case BCMTCH_FIRMWARE_FLAGS_CODE:
			break;

		default:
			BCMTCH_INFO("UNKNOWN BFF!!! : %d\n", entry_id);
			break;
		}

		/* next */
		entry_id++;
	}

	return ret_val;
}

static unsigned char bcmtchfw15200_bin[] = {
#include "bcmtchfw15200_bin.i"
};

static unsigned char bcmtchfw15300_bin[] = {
#include "bcmtchfw15300_bin.i"
};

struct firmware bcmtchfw15200_fw = {
	.data = bcmtchfw15200_bin,
	.size = sizeof(bcmtchfw15200_bin)
};

struct firmware bcmtchfw15300_fw = {
	.data = bcmtchfw15300_bin,
	.size = sizeof(bcmtchfw15300_bin)
};

static int32_t bcmtch_dev_download_firmware(
				struct bcmtch_data *bcmtch_data_ptr,
				uint8_t *fw_name,
				uint32_t fw_addr,
				uint32_t fw_flags)
{
	struct firmware *p_fw;
	int32_t ret_val = 0;

	uint32_t entry_id = 1;
	struct combi_entry *p_entry = NULL;
	struct combi_entry default_entry[] = {
		{.addr = fw_addr, .flags = fw_flags,},
		{0, 0, 0, 0},
	};

	if (strncmp("bcmtchfw15200_bin", fw_name, 13) == 0) {
		p_fw = &bcmtchfw15200_fw;
		BCMTCH_INFO("choosiing fw bcmtchfw15200_bin fw_size = %d\n",
								p_fw->size);
	} else if (strncmp("bcmtchfw15300_bin", fw_name, 13) == 0) {
		p_fw = &bcmtchfw15300_fw;
		BCMTCH_INFO("choosiing fw bcmtchfw15300_bin fw_size = %d\n",
								p_fw->size);
	} else {
		ret_val = -1;
		BCMTCH_ERR("cannot find a matching firmware\n");
	}

	if (ret_val) {
		BCMTCH_ERR("%s: Firmware request failed (%d) for %s\n",
			__func__,
			ret_val,
			fw_name);
	} else {
		BCMTCH_DBG(
			BCMTCH_DF_INFO,
			"INFO: FIRMWARE: %s\n",
			fw_name);

		BCMTCH_DBG(
			BCMTCH_DF_PB,
			"PB:f/w size= 0x%x\n",
			p_fw->size);

		/* pre-process binary according to flags */
		if (fw_flags & BCMTCH_FIRMWARE_FLAGS_COMBI) {
			p_entry = (struct combi_entry *) p_fw->data;
		} else {
			p_entry = default_entry;
			p_entry[entry_id].length = p_fw->size;
		}

		/* Parse firmware section flags and set mode */
		bcmtch_dev_parse_firmware(
			bcmtch_data_ptr,
			p_fw);

		/* init memory */
		ret_val = bcmtch_dev_init_memory(bcmtch_data_ptr);
		if (ret_val)
			return ret_val;

		while (p_entry[entry_id].length && !ret_val) {
			switch (p_entry[entry_id].flags &
					BCMTCH_FIRMWARE_FLAGS_MASK) {
			case BCMTCH_FIRMWARE_FLAGS_POST_BOOT_CONFIGS:
				BCMTCH_DBG(
					BCMTCH_DF_PB,
					"PB:entry_id=%d PB CONFIG\n",
					entry_id);
				BCMTCH_DBG(
					BCMTCH_DF_PB,
					"PB:pb chans init addr=0x%08x\n",
					p_entry[entry_id].addr);

				bcmtch_data_ptr->postboot_cfg_addr =
					p_entry[entry_id].addr;
				bcmtch_data_ptr->postboot_cfg_length =
					p_entry[entry_id].length;

#if BCMTCH_STATE_PROTOCOL
				if (!(bcmtch_boot_flag &
					BCMTCH_BF_STATE_PROTOCOL))
#endif
					ret_val =
						bcmtch_dev_init_channels(
							bcmtch_data_ptr,
							BCMTCH_ADDR_TOC_BASE,
							(uint8_t *)
							((uint32_t)p_fw->data +
							p_entry[entry_id].
								offset),
							BCMTCH_RAM_CHANNELS);

			case BCMTCH_FIRMWARE_FLAGS_POST_BOOT_CODE:
				if (!bcmtch_data_ptr->post_boot_sections)
					bcmtch_data_ptr->post_boot_buffer =
						(uint8_t *) p_fw->data;

				bcmtch_data_ptr->post_boot_sections++;

				if (p_entry[entry_id].flags &
					BCMTCH_FIRMWARE_FLAGS_POST_BOOT_PATCH)
					bcmtch_data_ptr->post_boot_patches++;

				BCMTCH_DBG(
					BCMTCH_DF_PB,
					"PB:entry_id=%d PB pb_sec=%d\n",
					entry_id,
					bcmtch_data_ptr->
					post_boot_sections);
				break;

			case BCMTCH_FIRMWARE_FLAGS_CONFIGS:
#if BCMTCH_STATE_PROTOCOL
				if ((bcmtch_data_ptr->boot_from_rom) ||
					!(bcmtch_boot_flag &
						BCMTCH_BF_STATE_PROTOCOL)) {
					/* Channel protocol */
					bcmtch_data_ptr->work_process_index =
						BCMTCH_WP_CHANNEL;
#endif /* BCMTCH_STATE_PROTOCOL */
					/* Initialize channels */
					BCMTCH_DBG(
						BCMTCH_DF_PB,
						"PB:entry_id=%d CONFIG\n",
						entry_id);

					BCMTCH_DBG(
						BCMTCH_DF_PB,
						"PB:%s chan init addr=0x%08x\n",
					(bcmtch_data_ptr->channel_set) ?
						"rom" : "ram",
						p_entry[entry_id].addr);

					ret_val =
						bcmtch_dev_init_channels(
							bcmtch_data_ptr,
							p_entry[entry_id].addr,
							(uint8_t *)
							((uint32_t)p_fw->data
							+ p_entry[entry_id].
							offset),
							(bcmtch_data_ptr->
								channel_set) ?
							BCMTCH_ROM_CHANNELS :
							BCMTCH_RAM_CHANNELS);
#if BCMTCH_STATE_PROTOCOL
				} else {
					/* State protocol */
					bcmtch_data_ptr->work_process_index =
						BCMTCH_WP_STATE;
				}
#endif

			default:
				BCMTCH_DBG(
					BCMTCH_DF_PB,
					"PB:entry_id=%d PREBOOT\n",
					entry_id);
				/** download to chip **/
				ret_val = bcmtch_com_write_sys(
					bcmtch_data_ptr,
					p_entry[entry_id].addr,
					p_entry[entry_id].length,
					(uint8_t *)((uint32_t)p_fw->data +
					p_entry[entry_id].offset));
			}

			/* next */
			entry_id++;
		}
	}

	if (bcmtch_boot_flag &
			BCMTCH_BF_DISABLE_POST_BOOT)
		bcmtch_data_ptr->post_boot_sections = 0;

	if (bcmtch_data_ptr->post_boot_sections) {
		/* setup first section for download */
		if (!bcmtch_dev_post_boot_get_section(bcmtch_data_ptr))
			bcmtch_data_ptr->post_boot_sections = 0;
	}

	BCMTCH_DBG(BCMTCH_DF_INFO, "INFO: FIRMWARE: loaded\n");

	return ret_val;
}

static int32_t bcmtch_dev_find_firmware(
	struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_file = -ENOENT;
	uint8_t file = 0;

	bool found_chip_id = false;

	uint32_t id = bcmtch_data_ptr->chip_id;
	uint32_t rev = bcmtch_data_ptr->rev_id;

	while (file < ARRAY_SIZE(BCMTCH_BINARIES)) {

		/* find matching chip id */
		if (BCMTCH_BINARIES[file].chip_id == id) {
			found_chip_id = true;

			/* if chip id found find matching chip rev */
			if (BCMTCH_BINARIES[file].chip_rev == rev) {
				ret_file = file;
				break;
			} else if (BCMTCH_BINARIES[file].chip_rev == BCMTCHWC) {
				ret_file = file;
			}
		} else if (BCMTCH_BINARIES[file].chip_id == BCMTCHWC) {
			if (!found_chip_id)
				ret_file = file;
		}

		file++;
	}

	if (ret_file < 0)
		BCMTCH_ERR(
			" firmware not configured:chip=0x%8x rev=0x%x\n",
			id,
			rev);

	return ret_file;
}

static int32_t bcmtch_dev_init_firmware(
	struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;
	uint8_t bin_file = 0;

	if (bcmtch_firmware) {
		ret_val =
			bcmtch_dev_download_firmware(
				bcmtch_data_ptr,
				bcmtch_firmware,
				bcmtch_firmware_addr,
				bcmtch_firmware_flag);
	} else {
		bin_file = bcmtch_dev_find_firmware(bcmtch_data_ptr);

		if (bin_file >= 0) {
			ret_val =
				bcmtch_dev_download_firmware(
					bcmtch_data_ptr,
					BCMTCH_BINARIES[bin_file].filename,
					BCMTCH_BINARIES[bin_file].addr,
					BCMTCH_BINARIES[bin_file].flags);
		} else
			ret_val = bin_file;
	}

#if BCMTCH_STATE_PROTOCOL
	/* init state protocol data structures. */
	if ((bcmtch_boot_flag & BCMTCH_BF_STATE_PROTOCOL)
			&& (!bcmtch_data_ptr->boot_from_rom))
		bcmtch_dev_init_state(bcmtch_data_ptr);
#endif

	if (!ret_val)
		ret_val = bcmtch_dev_run_firmware(bcmtch_data_ptr);

#if BCMTCH_STATE_PROTOCOL
		if (bcmtch_data_ptr->work_process_index >= BCMTCH_WP_STATE) {
			if (!ret_val)
				ret_val =
					bcmtch_state_resume(
						bcmtch_data_ptr);

			if (!ret_val) {
					ret_val =
						bcmtch_state_start_scan(
							bcmtch_data_ptr);
			}
		}
#endif /* BCMTCH_STATE_PROTOCOL */

	return ret_val;
}

static int32_t bcmtch_dev_init_platform(struct device *p_device)
{
	int32_t ret_val = 0;
	struct bcmtch_data *local_bcmtch_data_p = NULL;
#ifdef CONFIG_OF
	int32_t	idx;
	int32_t btn_count;
	int32_t of_ret_val;
	struct device_node *np;
	enum of_gpio_flags gpio_flags;
#else
	struct bcmtch_platform_data *p_platform_data = NULL;
#endif

	if (p_device) {
		local_bcmtch_data_p =
				dev_get_drvdata(p_device);

#ifdef CONFIG_OF
		np = p_device->of_node;
		if (!np) {
			BCMTCH_ERR(
				" Device tree (DT) error! of_node is NULL.\n");
			ret_val = -ENODEV;
			return ret_val;
		}

		/*
		 * Obtain the address of the SYS/AHB on I2C bus.
		 */
		of_ret_val =
				of_property_read_u32(np, "addr-sys",
					&local_bcmtch_data_p
						->platform_data.i2c_addr_sys);
		if (of_ret_val) {
			BCMTCH_ERR("DT property addr-sys not found!\n");
			goto of_read_error;
		}

		BCMTCH_DBG(BCMTCH_DF_DT, "DT:addr-sys = 0x%x\n",
			local_bcmtch_data_p->platform_data.i2c_addr_sys);

		/*
		 * Obtain the GPIO reset pin.
		 */
		if (!of_find_property(np, "reset-gpios", NULL)) {
			BCMTCH_ERR(
				" DT property reset-gpios not found!\n");
			ret_val = of_ret_val;
			local_bcmtch_data_p->
				platform_data.gpio_reset_pin = -1;
			local_bcmtch_data_p->
				platform_data.gpio_reset_polarity = -1;
		} else {

			local_bcmtch_data_p->
				platform_data.gpio_reset_pin =
					of_get_named_gpio_flags(
						np,
						"reset-gpios",
						0,
						&gpio_flags);
			BCMTCH_DBG(
				BCMTCH_DF_DT,
				"DT:gpio-reset-pin = 0x%x\n",
				local_bcmtch_data_p->platform_data
					.gpio_reset_pin);

			local_bcmtch_data_p->
				platform_data.gpio_reset_polarity =
					gpio_flags & OF_GPIO_ACTIVE_LOW;
			BCMTCH_DBG(
				BCMTCH_DF_DT,
				"DT:gpio-reset-polarity = 0x%x\n",
				local_bcmtch_data_p->platform_data
					.gpio_reset_polarity);

			/*
			 * Obtain the GPIO reset time in ms.
			 */
			of_ret_val = of_property_read_u32(np, "reset-time-ms",
					&local_bcmtch_data_p
						->platform_data
							.gpio_reset_time_ms);
			if (of_ret_val) {
				/* set default value */
				local_bcmtch_data_p->
					platform_data.
						gpio_reset_time_ms = 100;
			}
			BCMTCH_DBG(
				BCMTCH_DF_DT,
				"DT:gpio-reset-time = %u\n",
				local_bcmtch_data_p->platform_data
					.gpio_reset_time_ms);
		}

		/*
		 * Obtain the interrupt pin.
		 */
		local_bcmtch_data_p->platform_data.touch_irq =
				irq_of_parse_and_map(np, 0);
		if (local_bcmtch_data_p->platform_data.touch_irq) {
			BCMTCH_DBG(BCMTCH_DF_DT, "DT: irq = 0x%x",
				local_bcmtch_data_p->platform_data.touch_irq);

			local_bcmtch_data_p->
				platform_data.gpio_interrupt_pin = -1;
			local_bcmtch_data_p->
				platform_data.gpio_interrupt_trigger =
					IRQF_TRIGGER_NONE;
		} else {
			BCMTCH_ERR(
				"DT: interrupts (irq) request failed!\n");
			of_ret_val = -ENOENT;
			goto of_read_error;
		}

		/*
		 * Setup function pointers for axis coordinates.
		 */
		of_ret_val =
				of_property_read_u32(np,
					"axis-orientation-flag",
					&local_bcmtch_data_p->platform_data
						.axis_orientation_flag);
		if (of_ret_val) {
			pr_warn(" DT property axis-orientation-flag not found!\n");
			local_bcmtch_data_p->platform_data
				.axis_orientation_flag = 0;
		}

		BCMTCH_DBG(
			BCMTCH_DF_DT,
			"DT:axis-orientation-flag = 0x%02x\n",
			local_bcmtch_data_p->platform_data
			.axis_orientation_flag);

		/*
		 * Obtain the key map.
		 */
		of_ret_val =
				of_property_read_u32(np, "ext-button-count",
						&btn_count);
		if (of_ret_val) {
			BCMTCH_INFO(
				"DT property ext-button-count not found!\n");
			btn_count = 0;
		}

		if (btn_count) {
			BCMTCH_DBG(
				BCMTCH_DF_DT,
				"DT:ext-button-count = %d\n",
				btn_count);

			/* Allocate array */
			local_bcmtch_data_p
				->platform_data.ext_button_map =
					(const int *)local_bcmtch_data_p
					->bcmtch_button_map;

			/* Read array data from device tree */
			of_ret_val =
					of_property_read_u32_array(
						np, "ext-button-map",
						(u32 *)local_bcmtch_data_p
							->platform_data
							.ext_button_map,
						btn_count);
			if (of_ret_val) {
				BCMTCH_ERR(
					" DT property ext-button-map read failed!\n");
				local_bcmtch_data_p->platform_data
					.ext_button_count = 0;
			} else {
				local_bcmtch_data_p->platform_data
					.ext_button_count = btn_count;

				BCMTCH_DBG(
					BCMTCH_DF_DT,
					"DT:ext-button-map =");
				for (idx = 0; idx < btn_count; idx++)
					BCMTCH_DBG(
						BCMTCH_DF_DT,
						"DT: %d",
						local_bcmtch_data_p
						->platform_data
						.ext_button_map[idx]);
			}
		}
#else /* CONFIG_OF */

		p_platform_data =
			(struct bcmtch_platform_data *)p_device->platform_data;

		local_bcmtch_data_p->platform_data.i2c_addr_sys =
			p_platform_data->i2c_addr_sys;

		local_bcmtch_data_p->platform_data.i2c_addr_spm =
			p_platform_data->i2c_addr_spm;

		local_bcmtch_data_p->platform_data.gpio_reset_pin =
			p_platform_data->gpio_reset_pin;
		local_bcmtch_data_p->platform_data.gpio_reset_polarity =
			p_platform_data->gpio_reset_polarity;
		local_bcmtch_data_p->platform_data.gpio_reset_time_ms =
			p_platform_data->gpio_reset_time_ms;

		local_bcmtch_data_p->platform_data.gpio_interrupt_pin =
			p_platform_data->gpio_interrupt_pin;
		local_bcmtch_data_p->platform_data.gpio_interrupt_trigger =
			p_platform_data->gpio_interrupt_trigger;
		local_bcmtch_data_p->platform_data.touch_irq = gpio_to_irq(
			p_platform_data->gpio_interrupt_pin);

		local_bcmtch_data_p->platform_data.ext_button_count =
			p_platform_data->ext_button_count;
		local_bcmtch_data_p->platform_data.ext_button_map =
			p_platform_data->ext_button_map;

		local_bcmtch_data_p->platform_data.axis_orientation_flag =
			p_platform_data->axis_orientation_flag;

#endif /* CONFIG_OF */

		/*
		 * FIXME - these values would come from DT or FW or insmod
		 *
		 * NAPA POR is 4ms == 4000us
		 * - Value should NOT be less than 5000us
		 */
		local_bcmtch_data_p->power_on_delay_us = 60000; /* >= 5000 */

	} else {
		BCMTCH_ERR("%s() error, platform data == NULL\n",
			__func__);
		ret_val = -ENODATA;
	}

	return ret_val;

#ifdef CONFIG_OF
of_read_error:
	if (!ret_val)
		ret_val = -ENODEV;

	return ret_val;
#endif
}

static int32_t bcmtch_dev_request_power_mode(
		struct bcmtch_data *bcmtch_data_ptr,
		uint8_t mode, enum tofe_command command)
{
	int32_t ret_val = 0;

	uint8_t regs[5];
	uint8_t data[5];

	regs[0] = BCMTCH_SPM_REG_MSG_FROM_HOST;
	data[0] = command;
	regs[1] = BCMTCH_SPM_REG_RQST_FROM_HOST;
	data[1] = 0;

	switch (mode) {
	case BCMTCH_POWER_MODE_SLEEP:
		data[1] = BCMTCH_POWER_MODE_SLEEP;
		break;

	case BCMTCH_POWER_MODE_WAKE:
		data[1] = BCMTCH_POWER_MODE_WAKE;
		break;

	case BCMTCH_POWER_MODE_NOWAKE:
		data[1] = BCMTCH_POWER_MODE_NOWAKE;
		break;

	default:
		PROGRESS();
		break;
	}

	ret_val = bcmtch_com_fast_write_spm(bcmtch_data_ptr, 2, regs, data);

	return ret_val;
}

static int32_t bcmtch_dev_get_power_state(
		struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;
	uint8_t power_state;

	ret_val = bcmtch_com_read_spm(bcmtch_data_ptr,
		BCMTCH_SPM_REG_PSR, &power_state);

	return (ret_val) ? (ret_val) : ((uint32_t)power_state);
}

static int32_t bcmtch_dev_set_power_state(
		struct bcmtch_data *bcmtch_data_ptr,
		uint8_t power_state)
{
	int32_t ret_val = 0;

	switch (power_state) {
	case BCMTCH_POWER_STATE_SLEEP:
		ret_val =
			bcmtch_dev_request_power_mode(
				bcmtch_data_ptr,
				BCMTCH_POWER_MODE_SLEEP,
				TOFE_COMMAND_NO_COMMAND);
		break;

	case BCMTCH_POWER_STATE_RETENTION:
		PROGRESS();
		break;

	case BCMTCH_POWER_STATE_IDLE:
		PROGRESS();
		break;

	case BCMTCH_POWER_STATE_ACTIVE:
		PROGRESS();
		break;

	default:
		PROGRESS();
		break;
	}

	return ret_val;
}

static int32_t bcmtch_dev_check_power_state(
				struct bcmtch_data *bcmtch_data_ptr,
				uint8_t power_state,
				uint8_t wait_count)
{
	int32_t ret_val = -EAGAIN;
	int32_t read_state;

	do {
		read_state = bcmtch_dev_get_power_state(bcmtch_data_ptr);
		if (read_state == power_state) {
			ret_val = 0;
			break;
		}
	} while (wait_count--);

	return ret_val;
}

static enum bcmtch_status bcmtch_dev_request_host_override(
		struct bcmtch_data *bcmtch_data_ptr,
		enum tofe_command command)
{
	enum bcmtch_status ret_val = BCMTCH_STATUS_ERR_FAIL;
	int32_t count = 250;
	uint8_t m2h;

	/* Request channel & wakeup the firmware */
	ret_val = bcmtch_dev_request_power_mode(
					bcmtch_data_ptr,
					BCMTCH_POWER_MODE_WAKE,
					command);

	if (!ret_val)
		ret_val = bcmtch_dev_check_power_state(
					bcmtch_data_ptr,
					BCMTCH_POWER_STATE_ACTIVE,
					25);

	if (ret_val) {
		BCMTCH_ERR("%s: [%d] wake firmware failed.\n",
				__func__,
				ret_val);

	} else {

		/* Wait till FW OVERRIDE is ready */
		do {
			ret_val = bcmtch_com_read_spm(
					bcmtch_data_ptr,
					BCMTCH_SPM_REG_MSG_TO_HOST,
					&m2h);

			switch (m2h) {
			case TOFE_MESSAGE_FW_READY_OVERRIDE:
			case TOFE_MESSAGE_FW_READY_INTERRUPT_OVERRIDE:
				bcmtch_data_ptr->host_override = true;
				BCMTCH_DBG(
					BCMTCH_DF_HO,
					"HO: Request m=0x%0x  ho=%d",
					m2h,
					bcmtch_data_ptr->host_override);
				break;

			case TOFE_MESSAGE_FW_READY_INTERRUPT:
				BCMTCH_DBG(
					BCMTCH_DF_HO,
					"HO: Request m=0x%0x  ho=%d -> interrupt",
					m2h,
					bcmtch_data_ptr->host_override);
				bcmtch_dev_process(bcmtch_data_ptr);
			case TOFE_MESSAGE_FW_READY:
			default:
				BCMTCH_DBG(
					BCMTCH_DF_HO,
					"HO: Request m=0x%0x  ho=%d",
					m2h,
					bcmtch_data_ptr->host_override);
				break;
			}
		} while (!bcmtch_data_ptr->host_override && count--);

		if (bcmtch_data_ptr->host_override)
			ret_val = BCMTCH_STATUS_SUCCESS;
	}

	return ret_val;
}

static enum bcmtch_status bcmtch_dev_release_host_override(
		struct bcmtch_data *bcmtch_data_ptr,
		enum tofe_command command)
{
	enum bcmtch_status ret_val = BCMTCH_STATUS_ERR_FAIL;
	int32_t count = 250;
	uint8_t m2h;

    /* this should release hostOverride - do we need to check */
	/* Release channel */
	ret_val = bcmtch_dev_request_power_mode(
				bcmtch_data_ptr,
				BCMTCH_POWER_MODE_NOWAKE,
				command);

	/* Wait till FW is ready */
	do {
		ret_val = bcmtch_com_read_spm(bcmtch_data_ptr,
				BCMTCH_SPM_REG_MSG_TO_HOST, &m2h);
		switch (m2h) {
		case TOFE_MESSAGE_FW_READY_INTERRUPT_OVERRIDE:
			BCMTCH_DBG(
				BCMTCH_DF_HO,
				"HO: Release m=0x%0x  ho=%d -> interrupt",
				m2h,
				bcmtch_data_ptr->host_override);
			bcmtch_dev_process(bcmtch_data_ptr);
		case TOFE_MESSAGE_FW_READY_OVERRIDE:
		default:
			BCMTCH_DBG(
				BCMTCH_DF_HO,
				"HO: Release m=0x%0x  ho=%d",
				m2h,
				bcmtch_data_ptr->host_override);
			break;

		case TOFE_MESSAGE_FW_READY_INTERRUPT:
		case TOFE_MESSAGE_FW_READY:
			bcmtch_data_ptr->host_override = false;
			BCMTCH_DBG(
				BCMTCH_DF_HO,
				"HO: Release m=0x%0x  ho=%d",
				m2h,
				bcmtch_data_ptr->host_override);
			break;
		}
	} while (bcmtch_data_ptr->host_override && count--);

	if (!bcmtch_data_ptr->host_override)
		ret_val = BCMTCH_STATUS_SUCCESS;

	return ret_val;
}

static int32_t bcmtch_dev_send_command(
	struct bcmtch_data *bcmtch_data_ptr,
	enum tofe_command command,
	uint32_t data,
	uint16_t data16,
	uint8_t flags)
{
	int32_t ret_val = 0;
	struct tofe_command_response cmd;
	struct bcmtch_channel *chan;
	uint8_t chan_set = bcmtch_data_ptr->channel_set;

	if (command == TOFE_COMMAND_NO_COMMAND) {
		BCMTCH_ERR("%s: no_command.\n",
				__func__);
		return -EINVAL;
	}

	chan =
		bcmtch_data_ptr->p_channels[chan_set][TOFE_CHANNEL_ID_COMMAND];
	if (chan == NULL) {
		BCMTCH_ERR(
				"%s: command channel has not initialized!\n",
				__func__);
		return -ENXIO;
	}


	if (bcmtch_dev_request_host_override(
			bcmtch_data_ptr, command) ==
				BCMTCH_STATUS_SUCCESS) {

		/* Setup the command entry */
		memset(
			(void *)(&cmd),
			0,
			sizeof(struct tofe_command_response));
		cmd.flags	= flags;
		cmd.command	= command;
		cmd.data	= data;
		cmd.result	= data16;

		/* Write sys to command channel */
		tofe_channel_write_begin(&chan->hdr);
		ret_val = tofe_channel_write(&chan->hdr, &cmd);
		if (ret_val) {
			BCMTCH_ERR("%s: [%d] cmd channel write failed.\n",
					__func__,
					ret_val);
			goto send_command_exit;
		}

		ret_val = bcmtch_dev_write_channel(bcmtch_data_ptr,
			chan);
		if (ret_val) {
			BCMTCH_ERR(
				"%s: [%d] cmd channel write back FW failed.\n",
				__func__,
				ret_val);
			goto send_command_exit;
		}
	}

	bcmtch_dev_release_host_override(bcmtch_data_ptr,
		command);

send_command_exit:
	return ret_val;
}

static int32_t bcmtch_dev_reset(
		struct bcmtch_data *bcmtch_data_ptr,
		uint8_t mode)
{
	int32_t ret_val = 0;

	switch (mode) {
	case BCMTCH_RESET_MODE_HARD:
		bcmtch_reset(bcmtch_data_ptr);
		break;

	case BCMTCH_RESET_MODE_SOFT_CHIP:
		bcmtch_com_write_spm(
			bcmtch_data_ptr,
			BCMTCH_SPM_REG_SOFT_RESETS,
			BCMTCH_RESET_MODE_SOFT_CHIP);
		break;

	case BCMTCH_RESET_MODE_SOFT_ARM:
		bcmtch_com_write_spm(
			bcmtch_data_ptr,
			BCMTCH_SPM_REG_SOFT_RESETS,
			BCMTCH_RESET_MODE_SOFT_ARM);
		break;

	case (BCMTCH_RESET_MODE_SOFT_CHIP | BCMTCH_RESET_MODE_SOFT_ARM):
		bcmtch_com_write_spm(
			bcmtch_data_ptr,
			BCMTCH_SPM_REG_SOFT_RESETS,
			BCMTCH_RESET_MODE_SOFT_CHIP);
		bcmtch_com_write_spm(
			bcmtch_data_ptr,
			BCMTCH_SPM_REG_SOFT_RESETS,
			BCMTCH_RESET_MODE_SOFT_ARM);
		break;

	case BCMTCH_RESET_MODE_SOFT_CLEAR:
		ret_val = bcmtch_com_write_spm(
					bcmtch_data_ptr,
					BCMTCH_SPM_REG_SOFT_RESETS,
					BCMTCH_RESET_MODE_SOFT_CLEAR);
		break;

	default:
		break;
	}

	return ret_val;
}


static int32_t bcmtch_dev_get_rev_id(
	struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = -ENXIO;

	if (bcmtch_data_ptr) {
		ret_val =
			bcmtch_com_read_spm(bcmtch_data_ptr,
				BCMTCH_SPM_REG_REVISIONID,
				&bcmtch_data_ptr->rev_id);
	}
	return ret_val;
}

static int32_t bcmtch_dev_get_chip_id(
		struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = -ENXIO;

	if (bcmtch_data_ptr) {
		uint8_t id[3];

		ret_val =
			bcmtch_com_read_spm(bcmtch_data_ptr,
				BCMTCH_SPM_REG_CHIPID0, &id[0]);
		ret_val |=
			bcmtch_com_read_spm(bcmtch_data_ptr,
				BCMTCH_SPM_REG_CHIPID1, &id[1]);
		ret_val |=
			bcmtch_com_read_spm(bcmtch_data_ptr,
				BCMTCH_SPM_REG_CHIPID2, &id[2]);

		bcmtch_data_ptr->chip_id = ((((uint32_t)id[2]) << 16)
			| (((uint32_t)id[1]) << 8)
			| (uint32_t)id[0]);
	}

	return ret_val;

}

static int32_t bcmtch_dev_verify_chip_version(
		struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;
	uint32_t idx = 0;
	uint32_t *chips_ptr = (uint32_t *)BCMTCH_CHIP_IDS;

	/* Get Chip ID AFTER Power On due to OTP */
	ret_val = bcmtch_dev_get_chip_id(bcmtch_data_ptr);

	if (!ret_val)
		ret_val = bcmtch_dev_get_rev_id(bcmtch_data_ptr);

	if (!ret_val) {
		ret_val = -ENXIO;
		for (idx = 0;
				idx < ARRAY_SIZE(BCMTCH_CHIP_IDS);
				idx++) {
			if (chips_ptr[idx] ==
				bcmtch_data_ptr->chip_id) {
				/* Found a match in above search */
				ret_val = 0;

				BCMTCH_DBG(
					BCMTCH_DF_INFO,
					"INFO: chip_id = 0x%06X  rev = 0x%2X : %s\n",
					bcmtch_data_ptr->chip_id,
					bcmtch_data_ptr->rev_id,
					"Verified");
			}
		}
	}

	if (ret_val)
		BCMTCH_ERR(
			" chip_id = 0x%06X  rev = 0x%2X : %s : 0x%x\n",
			bcmtch_data_ptr->chip_id,
			bcmtch_data_ptr->rev_id,
			"Error - Unknown device",
			ret_val);

	return ret_val;
}

static void bcmtch_dev_init_worker_process(
		struct bcmtch_data *bcmtch_data_ptr)
{
	/* Channel protocol process function. */
	bcmtch_data_ptr->bcmtch_dev_process_table
			[BCMTCH_WP_CHANNEL] = bcmtch_dev_process;

	/* State protocol process function. */
#if BCMTCH_STATE_PROTOCOL
	bcmtch_data_ptr->bcmtch_dev_process_table
			[BCMTCH_WP_STATE] =
				bcmtch_dev_process_state_touches;

#endif /* BCMTCH_STATE_PROTOCOL */

	/* The process function during the post boot patch
	 * switch (boot from ROM to boot from RAM). */
	bcmtch_data_ptr->bcmtch_dev_process_table
			[BCMTCH_WP_PATCH_INIT] =
				bcmtch_dev_process_pb_patch_init;

	/* Set the default work process function. */
	bcmtch_data_ptr->work_process_index =
			BCMTCH_WP_CHANNEL;

}

#if BCMTCH_STATE_PROTOCOL
static void bcmtch_dev_init_state(
		struct bcmtch_data *bcmtch_data_ptr)
{
	if (bcmtch_boot_flag & BCMTCH_BF_STATE_PROTOCOL) {
		bcmtch_data_ptr->scan_data.cmd.touch_slot_format =
			(bcmtch_boot_flag & BCMTCH_BF_STATE_SHORT_SLOT) ?
			1 : 0;
		bcmtch_data_ptr->scan_data.cmd.sync_mode =
			(bcmtch_boot_flag & BCMTCH_BF_STATE_SYNC_MODE) ?
			0 : 1;
		bcmtch_data_ptr->scan_data.cmd.max_slot_number =
			BCMTCH_MAX_TOUCH;

		bcmtch_data_ptr->scan_data.touch_slot_size  =
			(bcmtch_boot_flag & BCMTCH_BF_STATE_SHORT_SLOT) ?
					TOFE_HOST_TOUCH_SLOT_SMALL_SIZE
					: TOFE_HOST_TOUCH_SLOT_BIG_SIZE;
		memset(&bcmtch_data_ptr->ram_rw_cfg, 0,
				sizeof(struct bcmtch_state_cfg_rw));
		bcmtch_data_ptr->work_process_index =
				BCMTCH_WP_STATE;
		BCMTCH_DBG(
			BCMTCH_DF_ST,
			"ST:%s() p0=0x%x slot_format %d sync_mode %d resp_buff_size %d max_slot_number %d\n",
			__func__,
			bcmtch_data_ptr->scan_data.cmd.reg,
			bcmtch_data_ptr->scan_data.cmd.touch_slot_format,
			bcmtch_data_ptr->scan_data.cmd.sync_mode,
			TOFE_HOST_RSP_BUF_SIZE,
			bcmtch_data_ptr->scan_data.cmd.max_slot_number);
	} else {
		bcmtch_data_ptr->work_process_index =
			BCMTCH_WP_CHANNEL;
	}
}
#endif /* BCMTCH_STATE_PROTOCOL */

static int32_t bcmtch_dev_init(
		struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;

	/* init com */
	if (!ret_val)
		ret_val = bcmtch_com_init(bcmtch_data_ptr);

	/* wakeup */
	if (!ret_val)
		ret_val = bcmtch_dev_request_power_mode(
					bcmtch_data_ptr,
					BCMTCH_POWER_MODE_WAKE,
					TOFE_COMMAND_NO_COMMAND);

	if (!ret_val)
		ret_val = bcmtch_dev_check_power_state(
					bcmtch_data_ptr,
					BCMTCH_POWER_STATE_ACTIVE,
					25);

	/* init clocks */
	if (!ret_val)
		ret_val = bcmtch_dev_init_clocks(bcmtch_data_ptr);

	/* read chip version and id with power on */
	if (!ret_val && (BCMTCH_BF_VERIFY_CHIP & bcmtch_boot_flag))
		ret_val = bcmtch_dev_verify_chip_version(bcmtch_data_ptr);

	/* download and run */
	if (!ret_val)
		ret_val = bcmtch_dev_init_firmware(bcmtch_data_ptr);

	return ret_val;
}

static void bcmtch_dev_process(
	struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;
	uint8_t m2h;

	if (bcmtch_boot_flag & BCMTCH_BF_CHECK_INTERRUPT) {
		/* Check msg2host */
		bcmtch_com_read_spm(bcmtch_data_ptr,
			BCMTCH_SPM_REG_MSG_TO_HOST, &m2h);
		if ((m2h & TOFE_MESSAGE_FW_READY_INTERRUPT) !=
				TOFE_MESSAGE_FW_READY_INTERRUPT) {
			BCMTCH_INFO("False interrupt\n");
			return;
		}
	}

	/* read DMA buffer */
	if (bcmtch_data_ptr->has_dma_channel &&
			!bcmtch_data_ptr->host_override) {
		ret_val =
			bcmtch_dev_read_dma_channels(bcmtch_data_ptr);
	}

	/* read channels */
	ret_val = bcmtch_dev_read_channels(bcmtch_data_ptr);

    /* release memory */
	bcmtch_dev_request_power_mode(
		bcmtch_data_ptr,
		BCMTCH_POWER_MODE_NOWAKE,
		TOFE_COMMAND_NO_COMMAND);

	/* process channels */
	ret_val = bcmtch_dev_process_channels(bcmtch_data_ptr);
}

static void bcmtch_dev_process_pb_patch_init(
	struct bcmtch_data *bcmtch_data_ptr)
{
	uint8_t m2h;

	/* Check post boot init req TOFE_MESSAGE_SOC_REBOOT_PENDING */
	if (bcmtch_data_ptr->post_boot_pending) {
		bcmtch_com_read_spm(bcmtch_data_ptr,
			BCMTCH_SPM_REG_MSG_TO_HOST, &m2h);
		if (m2h == TOFE_MESSAGE_SOC_REBOOT_PENDING) {
			/* Reset post boot pending */
			bcmtch_data_ptr->post_boot_pending = 0;

			/* Send command TOFE_COMMAND_REBOOT_APPROVED */
			bcmtch_dev_request_power_mode(
						bcmtch_data_ptr,
						BCMTCH_POWER_MODE_NOWAKE,
						TOFE_COMMAND_REBOOT_APPROVED);

			BCMTCH_DBG(
				BCMTCH_DF_PB,
				"PB:sent REBOOT_APPROVED\n");

#if BCMTCH_STATE_PROTOCOL
			if (bcmtch_boot_flag &
				BCMTCH_BF_STATE_PROTOCOL) {
				/* Switch to state protocol
					work process function.*/
				bcmtch_dev_init_state(bcmtch_data_ptr);
				bcmtch_state_resume(bcmtch_data_ptr);
					bcmtch_state_start_scan(
						bcmtch_data_ptr);
			} else {
#endif
				/* Switch channel mode. */
				bcmtch_data_ptr->work_process_index =
					BCMTCH_WP_CHANNEL;
				bcmtch_data_ptr->channel_set =
					BCMTCH_RAM_CHANNELS;
				bcmtch_dev_reset_events(bcmtch_data_ptr);
#if BCMTCH_STATE_PROTOCOL
			}
#endif
			return;
		}
	}

	/* Process the ROM channel */
	bcmtch_dev_process(bcmtch_data_ptr);
}

#if BCMTCH_STATE_PROTOCOL
static inline int32_t bcmtch_state_start_scan(
					struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;
	uint8_t parameters[TOFE_HOST_CMD_SHORT_SIZE];

	parameters[0] = bcmtch_data_ptr->scan_data.cmd.reg;

	ret_val = bcmtch_state_i2c_send_command(
					bcmtch_data_ptr,
					BCMTCH_CMD_START_SCAN,
					parameters,
					1);
	BCMTCH_DBG(BCMTCH_DF_ST, "ST: SCAN START: (p0=0x%x) = %d\n",
			bcmtch_data_ptr->scan_data.cmd.reg,
			ret_val);

	return ret_val;
}

static inline int32_t bcmtch_state_stop_scan(
					struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;

	ret_val = bcmtch_state_i2c_send_command(
					bcmtch_data_ptr,
					BCMTCH_CMD_STOP_SCAN,
					NULL,
					0);
	BCMTCH_DBG(BCMTCH_DF_ST, "ST: SCAN STOP: = %d\n", ret_val);

	return ret_val;
}


static inline int32_t bcmtch_state_suspend(
					struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;

	ret_val = bcmtch_state_stop_scan(bcmtch_data_ptr);
	if (ret_val)
		return ret_val;

	ret_val = bcmtch_state_i2c_send_command(
					bcmtch_data_ptr,
					BCMTCH_CMD_SUSPEND,
					NULL,
					0);
	if (!ret_val) {
		/* check command status */
		ret_val = bcmtch_state_check_command_status(
						bcmtch_data_ptr,
						100);
		BCMTCH_DBG(BCMTCH_DF_ST, "ST: CMD SUSPEND - 0x%x\n",
				bcmtch_data_ptr->bcmtch_state_resp_buffer[0]);
	}

	return ret_val;
}

static inline int32_t bcmtch_state_sleep(
					struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;

	ret_val = bcmtch_state_stop_scan(bcmtch_data_ptr);
	if (ret_val)
		return ret_val;

	ret_val = bcmtch_state_i2c_send_command(
					bcmtch_data_ptr,
					BCMTCH_CMD_SLEEP,
					NULL,
					0);
	if (!ret_val) {
		/* check command status */
		ret_val = bcmtch_state_check_command_status(
						bcmtch_data_ptr,
						100);
		BCMTCH_DBG(BCMTCH_DF_ST, "ST: CMD SLEEP - 0x%x\n",
				bcmtch_data_ptr->bcmtch_state_resp_buffer[0]);
	}

	return ret_val;
}

static inline int32_t bcmtch_state_resume(
					struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;

	ret_val = bcmtch_state_stop_scan(bcmtch_data_ptr);
	if (ret_val)
		return ret_val;

	ret_val = bcmtch_state_i2c_send_command(
					bcmtch_data_ptr,
					BCMTCH_CMD_RESUME,
					NULL,
					0);
	if (!ret_val) {
		/* check command status */
		ret_val = bcmtch_state_check_command_status(
						bcmtch_data_ptr,
						100);
		BCMTCH_DBG(BCMTCH_DF_ST, "ST: CMD RESUME - 0x%x\n",
				bcmtch_data_ptr->bcmtch_state_resp_buffer[0]);
	}

	return ret_val;
}

static int32_t bcmtch_state_rw_cfg_init(
				struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;
	uint8_t *parameters =
		bcmtch_data_ptr->bcmtch_state_resp_buffer;
	uint8_t *dma_buff =
		bcmtch_data_ptr->bcmtch_state_resp_buffer;
	struct bcmtch_state_rw_params *header =
		(struct bcmtch_state_rw_params *)
			bcmtch_data_ptr->bcmtch_state_resp_buffer;

	/* The Write RAM command returns configuration when
	 * the data size is set to zero.
	 */
	header->len = 0;
	ret_val = bcmtch_state_i2c_send_command(
					bcmtch_data_ptr,
					BCMTCH_CMD_WRITE_RAM,
					parameters,
					sizeof(struct bcmtch_state_rw_params));
	if (ret_val) {
		BCMTCH_ERR("ST: %s send cmd error! (%d)\n",
				__func__,
				ret_val);
		return ret_val;
	}

	/* check command status */
	ret_val = bcmtch_state_check_command_status(
					bcmtch_data_ptr,
					5);
	if (!ret_val)
		ret_val = bcmtch_com_read_dma(
					bcmtch_data_ptr,
					sizeof(struct bcmtch_state_cfg_rw) + 1,
					dma_buff);
	if (!ret_val)
		memcpy(&bcmtch_data_ptr->ram_rw_cfg,
				&dma_buff[1],
				sizeof(struct bcmtch_state_cfg_rw));

	BCMTCH_INFO("ST: RAM RW cfg init: max_r=%d max_w=%d\n",
				bcmtch_data_ptr->ram_rw_cfg.max_read_size,
				bcmtch_data_ptr->ram_rw_cfg.max_write_size);

	/* Resume the touch scan */
	bcmtch_state_start_scan(bcmtch_data_ptr);

	return ret_val;
}

static int32_t bcmtch_state_read_ram(
				struct bcmtch_data *bcmtch_data_ptr,
				uint32_t addr,
				uint16_t len,
				uint8_t *out)
{
	int32_t ret_val = 0;
	uint8_t *parameters =
		bcmtch_data_ptr->bcmtch_state_resp_buffer;
	uint8_t *dma_buff =
		bcmtch_data_ptr->bcmtch_state_resp_buffer;
	struct bcmtch_state_rw_params *header =
		(struct bcmtch_state_rw_params *)
			bcmtch_data_ptr->bcmtch_state_resp_buffer;

	/* Check the initialization of the read/write config. */
	if (bcmtch_data_ptr->ram_rw_cfg.cfg_size == 0) {
		ret_val =
			bcmtch_state_rw_cfg_init(bcmtch_data_ptr);
		if (ret_val)
			return ret_val;
	}

	if (len > bcmtch_data_ptr->ram_rw_cfg.max_read_size)
		return -EINVAL;

	ret_val = bcmtch_state_stop_scan(bcmtch_data_ptr);
	if (ret_val)
		return ret_val;

	header->addr = addr;
	header->len = len;
	ret_val = bcmtch_state_i2c_send_command(
					bcmtch_data_ptr,
					BCMTCH_CMD_READ_RAM,
					parameters,
					sizeof(struct bcmtch_state_rw_params));
	if (ret_val) {
		BCMTCH_ERR("ST: %s send cmd error! (%d)\n",
				__func__,
				ret_val);
		return ret_val;
	}

	/* check command status */
	ret_val = bcmtch_state_check_command_status(
					bcmtch_data_ptr,
					5);
	BCMTCH_DBG(
		BCMTCH_DF_ST,
		"ST: CMD R RAM - (%d) addr=0x%08x size=%d\n",
		bcmtch_data_ptr->bcmtch_state_resp_buffer[0],
		addr,
		len);

	/* read response data */
	if (!ret_val)
		ret_val = bcmtch_com_read_dma(
					bcmtch_data_ptr,
					len + 1,
					dma_buff);

	/* copy the RAM content */
	memcpy(out, &dma_buff[1], len);

	/* Resume the touch scan */
	bcmtch_state_start_scan(bcmtch_data_ptr);

	return ret_val;
}

static int32_t bcmtch_state_write_ram(
				struct bcmtch_data *bcmtch_data_ptr,
				uint32_t addr,
				uint16_t len,
				uint8_t *data)
{
	int32_t ret_val = 0;
	uint8_t *parameters =
		bcmtch_data_ptr->bcmtch_state_resp_buffer;
	struct bcmtch_state_rw_params *header =
		(struct bcmtch_state_rw_params *)
			bcmtch_data_ptr->bcmtch_state_resp_buffer;
	uint16_t data_size = len
				+ sizeof(struct bcmtch_state_rw_params);

	/* Check the initialization of the read/write config. */
	if (bcmtch_data_ptr->ram_rw_cfg.cfg_size == 0) {
		ret_val =
			bcmtch_state_rw_cfg_init(bcmtch_data_ptr);
		if (ret_val)
			return ret_val;
	}

	if (data_size > bcmtch_data_ptr->ram_rw_cfg.max_write_size)
		return -EINVAL;

	header->addr = addr;
	header->len = len;
	memcpy(parameters + sizeof(struct bcmtch_state_rw_params),
			data, len);

	ret_val = bcmtch_state_stop_scan(bcmtch_data_ptr);
	if (ret_val)
		return ret_val;

	ret_val = bcmtch_state_i2c_send_command(
					bcmtch_data_ptr,
					BCMTCH_CMD_WRITE_RAM,
					parameters,
					data_size);
	if (ret_val) {
		BCMTCH_ERR("ST: %s send cmd error! (%d)\n",
				__func__,
				ret_val);
		return ret_val;
	}

	/* check command status */
	ret_val = bcmtch_state_check_command_status(
					bcmtch_data_ptr,
					5);
	BCMTCH_DBG(
		BCMTCH_DF_ST,
		"ST: CMD W RAM - (%d) addr=0x%08x size=%d\n",
		bcmtch_data_ptr->bcmtch_state_resp_buffer[0],
		addr,
		len);

	/* Resume the touch scan */
	bcmtch_state_start_scan(bcmtch_data_ptr);

	return ret_val;
}

#if BCMTCH_STATE_PROTOCOL
static void bcmtch_state_check_fw_exception(
				struct bcmtch_data *bcmtch_data_ptr,
				uint8_t fw_exception)
{
	char *msg[] = {
		"Reset", "Reset", "NMI", "HardFault",
		"MemManage", "BusFault", "UsageFault",
	};

	uint32_t read_size =
			ARRAY_SIZE(bcmtch_data_ptr->exception_buffer)
			* sizeof(uint32_t);

	uint32_t *ex_buf = bcmtch_data_ptr->exception_buffer;

	if (fw_exception < ARRAY_SIZE(msg))
		BCMTCH_ERR("FWE:Critical error %d [%s]\n",
				fw_exception, msg[fw_exception]);
	else
		BCMTCH_ERR("FWE:Critical error %d\n", fw_exception);

	if (bcmtch_data_ptr->work_process_index >= BCMTCH_WP_STATE) {
		memset(ex_buf, 0, read_size);
		bcmtch_com_read_dma(
				bcmtch_data_ptr, read_size, (uint8_t *)ex_buf);

		if (ex_buf[15] == 0xDEADBEEF || ex_buf[15] == 0xDEADBEEB) {
			BCMTCH_ERR(
				"FWE:R0  = 0x%08X\n",
				ex_buf[0]);
			BCMTCH_ERR(
				"FWE:R1  = 0x%08X\n",
				ex_buf[1]);
			BCMTCH_ERR(
				"FWE:R2  = 0x%08X\n",
				ex_buf[2]);
			BCMTCH_ERR(
				"FWE:R3  = 0x%08X\n",
				ex_buf[3]);
			BCMTCH_ERR(
				"FWE:R12 = 0x%08X\n",
				ex_buf[4]);
			BCMTCH_ERR(
				"FWE:LR  = 0x%08X\n",
				ex_buf[5]);
			BCMTCH_ERR(
				"FWE:PC  = 0x%08X\n",
				ex_buf[6]);
			BCMTCH_ERR(
				"FWE:PSR = 0x%08X\n",
				ex_buf[7]);
			BCMTCH_ERR(
				"FWE:SP  = 0x%08X\n",
				ex_buf[8]);
			BCMTCH_ERR(
				"FWE:SCB_CCR   = 0x%08X\n",
				ex_buf[9]);
			BCMTCH_ERR(
				"FWE:SCB_SHCSR = 0x%08X\n",
				ex_buf[10]);
			BCMTCH_ERR(
				"FWE:SCB_CFS   = 0x%08X\n",
				ex_buf[11]);
			BCMTCH_ERR(
				"FWE:SCB_HFSR  = 0x%08X\n",
				ex_buf[12]);
			BCMTCH_ERR(
				"FWE:SCB_MMFAR = 0x%08X\n",
				ex_buf[13]);
			BCMTCH_ERR(
				"FWE:SCB_BFAR  = 0x%08X\n",
				ex_buf[14]);

			if (ex_buf[15] == 0xDEADBEEB) {
				BCMTCH_ERR(
					"FWE:Stack[0] = 0x%08X\n",
					ex_buf[16]);
				BCMTCH_ERR(
					"FWE:Stack[1] = 0x%08X\n",
					ex_buf[17]);
				BCMTCH_ERR(
					"FWE:Stack[2] = 0x%08X\n",
					ex_buf[18]);
				BCMTCH_ERR(
					"FWE:Stack[3] = 0x%08X\n",
					ex_buf[19]);
				BCMTCH_ERR(
					"FWE:Stack[4] = 0x%08X\n",
					ex_buf[20]);
				BCMTCH_ERR(
					"FWE:Stack[5] = 0x%08X\n",
					ex_buf[21]);
				BCMTCH_ERR(
					"FWE:Stack[6] = 0x%08X\n",
					ex_buf[22]);
				BCMTCH_ERR(
					"FWE:Stack[7] = 0x%08X\n",
					ex_buf[23]);
			}
		}
	}
}
#endif

static int32_t bcmtch_state_check_command_status(
				struct bcmtch_data *bcmtch_data_ptr,
				uint8_t timeout_ms)
{
	uint8_t timeout = timeout_ms;
	uint8_t m2h;
	int32_t ret_val = 0;

	do {
		ret_val =
			bcmtch_com_read_spm(
					bcmtch_data_ptr,
					BCMTCH_SPM_REG_MSG_TO_HOST,
					&m2h);
		if (ret_val)
			return ret_val;

		if ((m2h > 0x1) && (m2h < TOFE_MESSAGE_SUCCESS)) {
			ret_val = m2h;
#if BCMTCH_STATE_PROTOCOL
			bcmtch_state_check_fw_exception(
				bcmtch_data_ptr,
				m2h);
#else
			BCMTCH_ERR("exception = 0x%x\n", m2h);
#endif
			break;
		} else if (m2h > TOFE_MESSAGE_COMMAND_ECHO) {
			break;
		}

		usleep_range(800, 1200);
	} while (timeout--
			&& !(m2h & TOFE_MESSAGE_FW_READY));

	bcmtch_data_ptr->bcmtch_state_resp_buffer[0] =
			m2h & ~TOFE_MESSAGE_FW_READY;

	return ret_val;
}

static void bcmtch_dev_process_state_touches(
	struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;
	int slots;

	/* Check command status */
	ret_val = bcmtch_state_check_command_status(
					bcmtch_data_ptr,
					20);

	if (ret_val)
		return;

	/* Read command response status */
	bcmtch_data_ptr->scan_data.status.reg =
		bcmtch_data_ptr->bcmtch_state_resp_buffer[0];

	slots = bcmtch_data_ptr->scan_data.status.slots;
	/* FIXME: Need to take the log slot case into account. */

	/* Read variable-size response (long) */
	ret_val = bcmtch_dev_read_scan_touches(
					bcmtch_data_ptr,
					slots);

	/* Next touch scan */
	bcmtch_state_start_scan(bcmtch_data_ptr);
}
#endif /* BCMTCH_STATE_PROTOCOL */

static void bcmtch_dev_reset_events(
		struct bcmtch_data *bcmtch_data_ptr)
{
	uint32_t touch = 0;

	/* clear active touch structure */
	memset(
		&bcmtch_data_ptr->touch[0],
		0,
		sizeof(struct bcmtch_touch) * BCMTCH_MAX_TOUCH);

	/* clear system touches */
	if (bcmtch_data_ptr->touch_count) {
		for (touch = 0; touch < BCMTCH_MAX_TOUCH; touch++) {
			input_mt_slot(
				bcmtch_data_ptr->p_input_device,
				touch);
			input_mt_report_slot_state(
				bcmtch_data_ptr->p_input_device,
				MT_TOOL_FINGER,
				false);
		}

		input_report_key(
			bcmtch_data_ptr->p_input_device,
			BTN_TOUCH,
			false);

		input_sync(bcmtch_data_ptr->p_input_device);
	}
	bcmtch_data_ptr->touch_count = 0;
}

static void bcmtch_dev_post_boot_reset(
	struct bcmtch_data *bcmtch_data_ptr)
{
	bcmtch_data_ptr->post_boot_section = 0;
	bcmtch_data_ptr->post_boot_sections = 0;

	bcmtch_data_ptr->post_boot_patches = 0;

	bcmtch_data_ptr->post_boot_pending = 0;

	bcmtch_data_ptr->postboot_cfg_addr = 0;
	bcmtch_data_ptr->postboot_cfg_length = 0;

	/* free communication channels */
	bcmtch_dev_free_channels(bcmtch_data_ptr);

	bcmtch_data_ptr->post_boot_buffer = NULL;

}

static int32_t bcmtch_dev_suspend(
	struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;

	/* disable interrupts */
	bcmtch_interrupt_disable(bcmtch_data_ptr);

	/* free watchdog timer */
	bcmtch_dev_watchdog_stop(bcmtch_data_ptr);

	/* clear worker */
	bcmtch_clear_deferred_worker(bcmtch_data_ptr);

	/* lock */
	mutex_lock(&bcmtch_data_ptr->mutex_work);

	if (bcmtch_boot_flag & BCMTCH_BF_SUSPEND_COLD_BOOT) {
		/* post boot reset */
		bcmtch_dev_post_boot_reset(bcmtch_data_ptr);

#if BCMTCH_STATE_PROTOCOL
		if (bcmtch_data_ptr->work_process_index >= BCMTCH_WP_STATE)
			ret_val = bcmtch_state_sleep(bcmtch_data_ptr);
		else
#endif /* BCMTCH_STATE_PROTOCOL */
		ret_val = bcmtch_dev_set_power_state(
			bcmtch_data_ptr,
			BCMTCH_POWER_STATE_SLEEP);

		bcmtch_dev_power_enable(bcmtch_data_ptr, false);
	} else {
#if BCMTCH_STATE_PROTOCOL
		if (bcmtch_data_ptr->work_process_index >= BCMTCH_WP_STATE)
			ret_val = bcmtch_state_suspend(bcmtch_data_ptr);
		else
#endif /* BCMTCH_STATE_PROTOCOL */
		/* suspend */
		ret_val = bcmtch_dev_request_power_mode(
					bcmtch_data_ptr,
					BCMTCH_POWER_MODE_NOWAKE,
					TOFE_COMMAND_POWER_MODE_SUSPEND);
	}

	/* clear events */
	bcmtch_dev_reset_events(bcmtch_data_ptr);

	/* unlock */
	mutex_unlock(&bcmtch_data_ptr->mutex_work);

	/* update ABI status */
	mutex_lock(&bcmtch_data_ptr->mutex_abi);
	bcmtch_data_ptr->abi_suspend = true;
	mutex_unlock(&bcmtch_data_ptr->mutex_abi);

	BCMTCH_DBG(BCMTCH_DF_PM, "PM:%s() - 0x%x\n",
		__func__,
		ret_val);

	return ret_val;
}

static int32_t bcmtch_dev_resume(
	struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;

	/* lock */
	mutex_lock(&bcmtch_data_ptr->mutex_work);

	if (bcmtch_boot_flag & BCMTCH_BF_SUSPEND_COLD_BOOT) {

		ret_val = bcmtch_dev_power_enable(
					bcmtch_data_ptr,
					true);

		if (!ret_val)
			ret_val = bcmtch_dev_init(bcmtch_data_ptr);
	} else {
#if BCMTCH_STATE_PROTOCOL
		if (bcmtch_data_ptr->work_process_index >= BCMTCH_WP_STATE)
			ret_val = bcmtch_state_resume(
						bcmtch_data_ptr);
		else {
#endif
			ret_val = bcmtch_dev_request_power_mode(
						bcmtch_data_ptr,
						BCMTCH_POWER_MODE_WAKE,
						TOFE_COMMAND_NO_COMMAND);

			if (!ret_val)
				ret_val =
					bcmtch_dev_check_power_state(
						bcmtch_data_ptr,
						BCMTCH_POWER_STATE_ACTIVE,
						25);
			if (!ret_val)
				ret_val =
					bcmtch_dev_wait_for_firmware_ready(
						bcmtch_data_ptr,
						BCMTCH_FW_READY_WAIT);
#if BCMTCH_STATE_PROTOCOL
		}
#endif
	}

	if (!ret_val)
		bcmtch_dev_watchdog_start(bcmtch_data_ptr);

	if (!ret_val)
		ret_val = bcmtch_interrupt_enable(bcmtch_data_ptr);

#if BCMTCH_STATE_PROTOCOL
	if (!ret_val &&
		(bcmtch_data_ptr->work_process_index >= BCMTCH_WP_STATE)) {
			bcmtch_state_start_scan(bcmtch_data_ptr);
	}
#endif /* BCMTCH_STATE_PROTOCOL */

	if (ret_val)
		BCMTCH_ERR("BCMTOUCH: %s() error rv=%d bf=0x%x\n",
				__func__,
				ret_val,
				bcmtch_boot_flag);

	/* unlock */
	mutex_unlock(&bcmtch_data_ptr->mutex_work);

	/* update ABI status */
	mutex_lock(&bcmtch_data_ptr->mutex_abi);
	bcmtch_data_ptr->abi_suspend = false;
	mutex_unlock(&bcmtch_data_ptr->mutex_abi);

	BCMTCH_DBG(BCMTCH_DF_PM, "PM: %s() - 0x%x\n",
		__func__,
		ret_val);

	return ret_val;
}

void bcmtch_dev_watchdog_work(unsigned long int data)
{
	struct bcmtch_data *bcmtch_data_ptr =
		(struct bcmtch_data *)data;

	BCMTCH_DBG(BCMTCH_DF_WD, "WD:\n");

	/* queue the interrupt handler */
	queue_work(
		bcmtch_data_ptr->p_workqueue,
		(struct work_struct *)&bcmtch_data_ptr->work);

	bcmtch_dev_watchdog_reset(bcmtch_data_ptr);
}

static void bcmtch_dev_watchdog_start(
		struct bcmtch_data *bcmtch_data_ptr)
{
	if (timer_pending(&bcmtch_data_ptr->watchdog))
		del_timer_sync(&bcmtch_data_ptr->watchdog);

	init_timer(&bcmtch_data_ptr->watchdog);

	bcmtch_data_ptr->watchdog_expires =
			(bcmtch_data_ptr->post_boot_sections) ?
			msecs_to_jiffies(bcmtch_watchdog_post_boot) :
			msecs_to_jiffies(bcmtch_watchdog_normal);

	bcmtch_data_ptr->watchdog.function = bcmtch_dev_watchdog_work;
	bcmtch_data_ptr->watchdog.data = (unsigned long)bcmtch_data_ptr;
	bcmtch_data_ptr->watchdog.expires =
			jiffies + bcmtch_data_ptr->watchdog_expires;

	add_timer(&bcmtch_data_ptr->watchdog);
}

static int32_t bcmtch_dev_watchdog_restart(
		struct bcmtch_data *bcmtch_data_ptr,
		uint32_t expires)
{
	int32_t ret_val = 0;

	bcmtch_data_ptr->watchdog_expires = expires;

	ret_val = mod_timer(
			&bcmtch_data_ptr->watchdog,
			(jiffies + bcmtch_data_ptr->watchdog_expires));

	return ret_val;
}

static int32_t bcmtch_dev_watchdog_reset(
	struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;

	ret_val = mod_timer(
			&bcmtch_data_ptr->watchdog,
			(jiffies + bcmtch_data_ptr->watchdog_expires));

	return ret_val;
}

static int32_t bcmtch_dev_watchdog_stop(
	struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;

	if (bcmtch_data_ptr) {
		ret_val = del_timer_sync(&bcmtch_data_ptr->watchdog);
		bcmtch_data_ptr->watchdog_expires = 0;
	}

	return ret_val;
}

static int32_t bcmtch_dev_reset_fw(
	struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;

	/* disable interrupts */
	bcmtch_interrupt_disable(bcmtch_data_ptr);

	/* free watchdog timer */
	bcmtch_dev_watchdog_stop(bcmtch_data_ptr);

	/* post boot reset */
	bcmtch_dev_post_boot_reset(bcmtch_data_ptr);

	/* cycle power - off */
	bcmtch_dev_power_enable(
			bcmtch_data_ptr,
			false);

	/* pause */
	msleep(20);

	/* cycle power - on */
	ret_val = bcmtch_dev_power_enable(
			bcmtch_data_ptr,
			true);
	if (ret_val) {
		BCMTCH_ERR("WG: %s power enable error!\n",
				__func__);
		goto wdg_reset_error;
	}

	/* reset the chip on driver load ? */
	if (bcmtch_boot_flag &
			BCMTCH_BF_HARD_RESET_ON_LOAD) {
		bcmtch_dev_reset(
			bcmtch_data_ptr,
			BCMTCH_RESET_MODE_HARD);
	} else if (bcmtch_boot_flag &
			BCMTCH_BF_SOFT_RESET_ON_LOAD) {
		bcmtch_dev_reset(
			bcmtch_data_ptr,
			BCMTCH_RESET_MODE_SOFT_CHIP |
			BCMTCH_RESET_MODE_SOFT_ARM);
	}

	/* clear events */
	bcmtch_dev_reset_events(bcmtch_data_ptr);

	/* init touch controller */
	ret_val = bcmtch_dev_init(bcmtch_data_ptr);
	if (ret_val) {
		BCMTCH_ERR("WG: %s dev_init error!\n",
				__func__);
		goto wdg_reset_error;
	}

	/* restart watchdog */
	bcmtch_dev_watchdog_start(bcmtch_data_ptr);

	/* re-enable interrupts */
	ret_val =
		bcmtch_interrupt_enable(bcmtch_data_ptr);

wdg_reset_error:
	return ret_val;
}

static int32_t bcmtch_dev_watchdog_check(
	struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;
	bool bcmtch_error = false;

	BCMTCH_DBG(BCMTCH_DF_WD, "WD: Watch DOG\n");
#if BCMTCH_STATE_PROTOCOL
	/* Check status */
	ret_val =
		bcmtch_state_check_command_status(bcmtch_data_ptr, 1);

	if (ret_val)
		bcmtch_error = true;
	else if (bcmtch_data_ptr->work_process_index >= BCMTCH_WP_STATE) {

		/* Test stop command */
		bcmtch_state_stop_scan(bcmtch_data_ptr);
		ret_val =
			bcmtch_state_check_command_status(bcmtch_data_ptr, 25);

		if (ret_val)
			bcmtch_error = true;
		else {
			/* Test start command */
			bcmtch_state_start_scan(bcmtch_data_ptr);
			ret_val =
				bcmtch_state_check_command_status(
						bcmtch_data_ptr, 10);

			if (ret_val ||
				(bcmtch_data_ptr->bcmtch_state_resp_buffer[0]
					< TOFE_MESSAGE_SUCCESS)) {
				bcmtch_error = true;
			}
		}
	}
#endif

	/* Recover FW */
	if ((bcmtch_error) &&
		(bcmtch_boot_flag & BCMTCH_BF_FW_RESET_ON_WD)) {
		BCMTCH_ERR("WG: watch dog reset [%d]\n", ret_val);
		ret_val = bcmtch_dev_reset_fw(bcmtch_data_ptr);
		if (ret_val)
			return ret_val;
	}

	return ret_val;
}

static int32_t bcmtch_dev_power_init(
		struct bcmtch_data *p_bcmtch_data)
{
	int32_t ret_val = 0;

#ifdef CONFIG_REGULATOR
	p_bcmtch_data->regulator_avdd33 =
			regulator_get(
				p_bcmtch_data->p_device,
				"avdd33");
	if (IS_ERR(p_bcmtch_data->regulator_avdd33))
		p_bcmtch_data->regulator_avdd33 = NULL;

	p_bcmtch_data->regulator_vddo =
			regulator_get(
				p_bcmtch_data->p_device,
				"vddo");
	if (IS_ERR(p_bcmtch_data->regulator_vddo))
		p_bcmtch_data->regulator_vddo = NULL;

	p_bcmtch_data->regulator_avdd_adldo =
			regulator_get(
				p_bcmtch_data->p_device,
				"avdd_adldo");
	if (IS_ERR(p_bcmtch_data->regulator_avdd_adldo))
		p_bcmtch_data->regulator_avdd_adldo = NULL;

	BCMTCH_DBG(BCMTCH_DF_PM, "PM:%s() %x %x %x\n",
		__func__,
		(uint32_t)p_bcmtch_data->regulator_avdd33,
		(uint32_t)p_bcmtch_data->regulator_vddo,
		(uint32_t)p_bcmtch_data->regulator_avdd_adldo);
#endif

	return ret_val;
}

static int32_t bcmtch_dev_power_enable(
		struct bcmtch_data *p_bcmtch_data,
		bool enable)
{
	int32_t ret_val = 0;

#ifdef CONFIG_REGULATOR
	if (enable) {
		if (p_bcmtch_data->regulator_avdd33)
			ret_val |= regulator_enable(p_bcmtch_data
						->regulator_avdd33);

		if (p_bcmtch_data->regulator_vddo)
			ret_val |= regulator_enable(p_bcmtch_data
						->regulator_vddo);

		if (p_bcmtch_data->regulator_avdd_adldo)
			ret_val |= regulator_enable(p_bcmtch_data
						->regulator_avdd_adldo);

		if (p_bcmtch_data->power_on_delay_us)
			usleep_range(
				p_bcmtch_data->power_on_delay_us - 500,
				p_bcmtch_data->power_on_delay_us + 500);
	} else {
		if (p_bcmtch_data->regulator_avdd33 &&
			regulator_is_enabled(
					p_bcmtch_data->regulator_avdd33))
			regulator_disable(
				p_bcmtch_data->regulator_avdd33);

		if (p_bcmtch_data->regulator_vddo &&
			regulator_is_enabled(
					p_bcmtch_data->regulator_vddo))
			regulator_disable(
				p_bcmtch_data->regulator_vddo);

		if (p_bcmtch_data->regulator_avdd_adldo &&
			regulator_is_enabled(
					p_bcmtch_data->regulator_avdd_adldo))
			regulator_disable(
				p_bcmtch_data->regulator_avdd_adldo);
	}
#endif

	if (ret_val)
		BCMTCH_ERR("BCMTOUCH: %s() error rv=%d\n",
				__func__,
				ret_val);

	return ret_val;
}

static int32_t bcmtch_dev_power_free(
		struct bcmtch_data *p_bcmtch_data)
{
	int32_t ret_val = 0;

#ifdef CONFIG_REGULATOR
	if (p_bcmtch_data->regulator_avdd33)
		regulator_put(p_bcmtch_data->regulator_avdd33);

	if (p_bcmtch_data->regulator_vddo)
		regulator_put(p_bcmtch_data->regulator_vddo);

	if (p_bcmtch_data->regulator_avdd_adldo)
		regulator_put(p_bcmtch_data->regulator_avdd_adldo);

	p_bcmtch_data->regulator_avdd33 = NULL;
	p_bcmtch_data->regulator_vddo = NULL;
	p_bcmtch_data->regulator_avdd_adldo = NULL;
#endif

	return ret_val;
}

/* -------------------------------------------------- */
/* - BCM Touch Controller Com(munication) Functions - */
/* -------------------------------------------------- */

static int32_t bcmtch_com_init(
	struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;

	if (BCMTCH_DEFAULT_I2C_ADDR_SYS !=
		bcmtch_data_ptr->platform_data.i2c_addr_sys)
		ret_val |= bcmtch_com_write_spm(
				bcmtch_data_ptr,
				BCMTCH_SPM_REG_I2CS_CHIPID,
				bcmtch_data_ptr->platform_data.i2c_addr_sys);

	return ret_val;
}

static int32_t bcmtch_com_read_spm(
		struct bcmtch_data *bcmtch_data_ptr,
		uint8_t reg, uint8_t *data)
{
	int32_t ret_val = 0;

	if (bcmtch_data_ptr) {
		ret_val = bcmtch_i2c_read_spm(
					bcmtch_data_ptr->p_i2c_client_spm,
					reg,
					data);
	} else {
		BCMTCH_ERR("%s() error, bcmtch_data_ptr == NULL\n", __func__);

		ret_val = -ENODATA;
	}

	return ret_val;
}

static int32_t bcmtch_com_write_spm(
		struct bcmtch_data *bcmtch_data_ptr,
		uint8_t reg, uint8_t data)
{
	int32_t ret_val = 0;

	if (bcmtch_data_ptr) {
		ret_val = bcmtch_i2c_write_spm(
					bcmtch_data_ptr->p_i2c_client_spm,
					reg,
					data);
	} else {
		BCMTCH_ERR("%s() error, bcmtch_data_ptr == NULL\n", __func__);
		ret_val = -ENODATA;
	}

	return ret_val;
}

static inline int32_t bcmtch_com_fast_write_spm(
		struct bcmtch_data *bcmtch_data_ptr,
		uint8_t count, uint8_t *regs, uint8_t *data)
{
	int32_t ret_val = 0;

	ret_val = bcmtch_i2c_fast_write_spm(
				bcmtch_data_ptr->p_i2c_client_spm,
				count,
				regs,
				data);

	return ret_val;
}

static int32_t bcmtch_com_read_sys(
		struct bcmtch_data *bcmtch_data_ptr,
		uint32_t sys_addr,
		uint16_t read_len,
		uint8_t *read_data)
{
	int32_t ret_val = 0;

	if (bcmtch_data_ptr) {
		ret_val = bcmtch_i2c_read_sys(
					bcmtch_data_ptr->p_i2c_client_sys,
					sys_addr,
					read_len,
					read_data);
	} else {
		BCMTCH_ERR("%s() error, bcmtch_data_ptr == NULL\n", __func__);

		ret_val = -ENODATA;
	}

	return ret_val;
}

static inline int32_t bcmtch_com_write_sys32(
		struct bcmtch_data *bcmtch_data_ptr,
		uint32_t sys_addr,
		uint32_t write_data)
{
	return bcmtch_com_write_sys(bcmtch_data_ptr,
			sys_addr, 4, (uint8_t *)&write_data);
}

static int32_t bcmtch_com_write_sys(
		struct bcmtch_data *bcmtch_data_ptr,
		uint32_t sys_addr,
		uint16_t write_len,
		uint8_t *write_data)
{
	int32_t ret_val = 0;

	if (bcmtch_data_ptr) {
		ret_val = bcmtch_i2c_write_sys(
					bcmtch_data_ptr->p_i2c_client_sys,
					sys_addr,
					write_len,
					write_data);
	} else {
		BCMTCH_ERR("%s() error, bcmtch_data_ptr == NULL\n", __func__);

		ret_val = -ENODATA;
	}

	return ret_val;
}

static int32_t bcmtch_com_read_dma(
		struct bcmtch_data *bcmtch_data_ptr,
		uint16_t read_len,
		uint8_t *read_data)
{
	int32_t ret_val = 0;

	if (bcmtch_data_ptr) {
		ret_val = bcmtch_i2c_read_dma(
					bcmtch_data_ptr->p_i2c_client_sys,
					read_len,
					read_data);
	} else {
		BCMTCH_ERR("%s() error, bcmtch_data_ptr == NULL\n",
		       __func__);

		ret_val = -ENODATA;
	}

	return ret_val;
}

/* ------------------------------------- */
/* - BCM Touch Controller OS Functions - */
/* ------------------------------------- */
static irqreturn_t bcmtch_interrupt_handler(int32_t irq, void *dev_id)
{
	struct bcmtch_data *bcmtch_data_ptr =
		(struct bcmtch_data *)dev_id;

	/* track interrupts */
	bcmtch_data_ptr->irq_pending = true;

	BCMTCH_DBG(BCMTCH_DF_IH, "IH:p=%d\n",
		bcmtch_data_ptr->irq_pending);

	/* queue the interrupt handler */
	queue_work(
		bcmtch_data_ptr->p_workqueue,
		(struct work_struct *)&bcmtch_data_ptr->work);

	/* reset watchdog */
	bcmtch_dev_watchdog_reset(bcmtch_data_ptr);

	return IRQ_HANDLED;
}

static int32_t bcmtch_interrupt_enable(
		struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;
	struct bcmtch_platform_data *p_data;

	if (bcmtch_data_ptr->irq_enabled)
		return ret_val;

	p_data = &bcmtch_data_ptr->platform_data;

	ret_val = request_irq(
				p_data->touch_irq,
				bcmtch_interrupt_handler,
				p_data->gpio_interrupt_trigger,
				BCMTCH15XXX_NAME,
				bcmtch_data_ptr);

	if (ret_val) {
		BCMTCH_ERR("ERROR: %s() - Unable to request interrupt irq %d\n",
				__func__,
				p_data->touch_irq);

		/* note :
		* - polling is not enabled in this release
		* - it is an error if an irq is requested
		*	and not granted
		*/
	} else
		bcmtch_data_ptr->irq_enabled = true;

	return ret_val;
}

static void bcmtch_interrupt_disable(
		struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t irq;

	if (bcmtch_data_ptr && bcmtch_data_ptr->irq_enabled) {
		irq = bcmtch_data_ptr->platform_data.touch_irq;
		if (irq) {
			free_irq(irq, bcmtch_data_ptr);
			bcmtch_data_ptr->irq_enabled = false;
		}
	}
}

static int32_t bcmtch_init_input_device(
		struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;
	int32_t button_init = 0;

	if (bcmtch_data_ptr) {
		bcmtch_data_ptr->p_input_device = input_allocate_device();
		if (bcmtch_data_ptr->p_input_device) {

			bcmtch_data_ptr->p_input_device->name =
				"BCMTCH15xxx Touch Screen";

			bcmtch_data_ptr->p_input_device->phys =
				"I2C";
			bcmtch_data_ptr->p_input_device->id.bustype =
				BUS_I2C;
			bcmtch_data_ptr->p_input_device->id.vendor =
				BCMTCH_VENDOR_ID;
			bcmtch_data_ptr->p_input_device->id.product =
				((bcmtch_data_ptr->chip_id
					& BCMTCH_CHIPID_PID_MASK)
				>> BCMTCH_CHIPID_PID_SHIFT);
			bcmtch_data_ptr->p_input_device->id.version =
				bcmtch_data_ptr->rev_id;

			set_bit(EV_SYN, bcmtch_data_ptr->p_input_device->evbit);
			set_bit(EV_ABS, bcmtch_data_ptr->p_input_device->evbit);
			__set_bit(
				INPUT_PROP_DIRECT,
				bcmtch_data_ptr->p_input_device->propbit);

			set_bit(EV_KEY,
				bcmtch_data_ptr->p_input_device->evbit);
			set_bit(
				BTN_TOUCH,
				bcmtch_data_ptr->p_input_device->keybit);

			while (button_init < bcmtch_data_ptr->
					platform_data.ext_button_count) {
				set_bit(
					bcmtch_data_ptr->platform_data.
						ext_button_map[button_init],
					bcmtch_data_ptr->
						p_input_device->keybit);
				button_init++;
			}

			input_set_abs_params(
				bcmtch_data_ptr->p_input_device,
				ABS_MT_POSITION_X,
				bcmtch_data_ptr->axis_x_min,
				bcmtch_data_ptr->axis_x_max,
				0,
				0);

			input_set_abs_params(
				bcmtch_data_ptr->p_input_device,
				ABS_MT_POSITION_Y,
				bcmtch_data_ptr->axis_y_min,
				bcmtch_data_ptr->axis_y_max,
				0,
				0);

/*			input_set_abs_params(
				bcmtch_data_ptr->p_input_device,
				ABS_MT_TOOL_TYPE,
				0,
				MT_TOOL_MAX,
				0,
				0);
*/
			if (bcmtch_event_flag &
					BCMTCH_EVENT_FLAG_TOUCH_SIZE) {
				input_set_abs_params(
					bcmtch_data_ptr->p_input_device,
					ABS_MT_TOUCH_MAJOR,
					bcmtch_data_ptr->axis_h_min,
					bcmtch_data_ptr->axis_h_max,
					0,
					0);

				input_set_abs_params(
					bcmtch_data_ptr->p_input_device,
					ABS_MT_TOUCH_MINOR,
					bcmtch_data_ptr->axis_h_min,
					bcmtch_data_ptr->axis_h_max,
					0,
					0);
			}

			if (bcmtch_event_flag &
					BCMTCH_EVENT_FLAG_TOOL_SIZE) {
				input_set_abs_params(
					bcmtch_data_ptr->p_input_device,
					ABS_MT_WIDTH_MAJOR,
					bcmtch_data_ptr->axis_h_min,
					bcmtch_data_ptr->axis_h_max,
					0,
					0);

				input_set_abs_params(
					bcmtch_data_ptr->p_input_device,
					ABS_MT_WIDTH_MINOR,
					bcmtch_data_ptr->axis_h_min,
					bcmtch_data_ptr->axis_h_max,
					0,
					0);
			}

			if (bcmtch_event_flag &
					BCMTCH_EVENT_FLAG_PRESSURE) {
				input_set_abs_params(
					bcmtch_data_ptr->p_input_device,
					ABS_MT_PRESSURE,
					0,
					BCMTCH_MAX_PRESSURE,
					0,
					0);
			}

			if (bcmtch_event_flag &
					BCMTCH_EVENT_FLAG_ORIENTATION) {
				input_set_abs_params(
					bcmtch_data_ptr->p_input_device,
					ABS_MT_ORIENTATION,
					BCMTCH_MIN_ORIENTATION,
					BCMTCH_MAX_ORIENTATION,
					0,
					0);
			}

			set_bit(
				BTN_TOOL_FINGER,
				bcmtch_data_ptr->p_input_device->keybit);
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0)
			input_mt_init_slots(
				bcmtch_data_ptr->p_input_device,
				BCMTCH_MAX_TOUCH);
#else
			input_mt_init_slots(
				bcmtch_data_ptr->p_input_device,
				BCMTCH_MAX_TOUCH,
				0);
#endif
			/* request new os input queue size for this device */
			input_set_events_per_packet(
				bcmtch_data_ptr->p_input_device,
				6 * BCMTCH_MAX_TOUCH);

			/* register device */
			ret_val =
				input_register_device(
					bcmtch_data_ptr->p_input_device);

			if (ret_val) {
				BCMTCH_INFO(
					"%s() Unable to register input device\n",
					__func__);

				input_free_device(
					bcmtch_data_ptr->p_input_device);
				bcmtch_data_ptr->p_input_device = NULL;
			}
		} else {
			BCMTCH_ERR("%s() Unable to create device\n",
			       __func__);

			ret_val = -ENODEV;
		}
	} else {
		BCMTCH_ERR("%s() error, driver data structure == NULL\n",
				__func__);

		ret_val = -ENODATA;
	}

	return ret_val;
}

static inline void bcmtch_free_input_device(
		struct bcmtch_data *bcmtch_data_ptr)
{
	if (bcmtch_data_ptr && bcmtch_data_ptr->p_input_device) {
		input_unregister_device(bcmtch_data_ptr->p_input_device);
		bcmtch_data_ptr->p_input_device = NULL;
	}
}

static bool bcmtch_dev_process_post_boot(
		struct bcmtch_data *bcmtch_data_ptr,
		bool irq_serviced)
{
	int32_t still_downloading;
	int32_t ret_val;

	if (irq_serviced) {
		/* download shorter packet */
		still_downloading =
				bcmtch_dev_post_boot_download(
					bcmtch_data_ptr,
					bcmtch_post_boot_rate_low);
	} else {
		/* download larger packet */
		still_downloading =
				bcmtch_dev_post_boot_download(
					bcmtch_data_ptr,
					bcmtch_post_boot_rate_high);
	}

	if (!still_downloading) {
		BCMTCH_DBG(BCMTCH_DF_PB, "PB:DOWNLOAD COMPLETE\n");


		if (bcmtch_data_ptr->postboot_cfg_length) {
			/* don't want any stray interrupts */
			bcmtch_interrupt_disable(bcmtch_data_ptr);

			/* Send post boot init command */
			ret_val = bcmtch_dev_send_command(
					bcmtch_data_ptr,
					TOFE_COMMAND_INIT_POST_BOOT_PATCHES,
					bcmtch_data_ptr->postboot_cfg_addr,
					bcmtch_data_ptr->postboot_cfg_length,
					0);
			if (ret_val != 0) {
				BCMTCH_ERR(
					" send_command error [%d] cmd=%x\n",
					ret_val,
					TOFE_COMMAND_INIT_POST_BOOT_PATCHES);
			}

			/* Set to check post boot init request from FW */
			bcmtch_data_ptr->post_boot_pending = 1;

			/* Set the worker process function to the transient
			 * one which handles the ISR workqueue during
			 * the switch from ROM FW to post boot patched FW.
			 */
			bcmtch_data_ptr->work_process_index =
					BCMTCH_WP_PATCH_INIT;

			bcmtch_dev_watchdog_restart(
				bcmtch_data_ptr,
				msecs_to_jiffies(bcmtch_watchdog_normal));

			bcmtch_interrupt_enable(bcmtch_data_ptr);
		}
		return true;
	}

	return false;
}


static void bcmtch_deferred_worker(struct work_struct *work)
{
	struct bcmtch_data *bcmtch_data_ptr =
		container_of(work, struct bcmtch_data, work);
	bool work_done = false;

	mutex_lock(&bcmtch_data_ptr->mutex_work);

	if (bcmtch_data_ptr->irq_pending) {
		bcmtch_data_ptr->irq_pending = false;

		/* Process channels */
		bcmtch_data_ptr->bcmtch_dev_process_table
			[bcmtch_data_ptr->work_process_index]
				(bcmtch_data_ptr);

		work_done = true;
	}

	/* Amortized post boot download */
	if (bcmtch_data_ptr->post_boot_sections) {
		bcmtch_dev_process_post_boot(
			bcmtch_data_ptr,
			work_done);

		work_done = true;
	}

	if (!work_done)
		bcmtch_dev_watchdog_check(bcmtch_data_ptr);

	mutex_unlock(&bcmtch_data_ptr->mutex_work);
}

static int32_t bcmtch_dev_post_boot_get_section(
	struct bcmtch_data *bcmtch_data_ptr)
{
	struct combi_entry *pb_entry = NULL;

	if (bcmtch_data_ptr->post_boot_sections &&
			bcmtch_data_ptr->post_boot_buffer) {
		pb_entry = (struct combi_entry *)
			bcmtch_data_ptr->post_boot_buffer;

		while (pb_entry[bcmtch_data_ptr->post_boot_section].length) {
			if (pb_entry[bcmtch_data_ptr->post_boot_section].flags &
					BCMTCH_FIRMWARE_FLAGS_POST_BOOT) {
				bcmtch_data_ptr->post_boot_data =
					bcmtch_data_ptr->post_boot_buffer
					+ pb_entry[bcmtch_data_ptr->
					post_boot_section].offset;

				bcmtch_data_ptr->post_boot_addr =
					pb_entry[bcmtch_data_ptr->
					post_boot_section].addr;

				bcmtch_data_ptr->post_boot_left =
					pb_entry[bcmtch_data_ptr->
					post_boot_section].length;

				break;
			} else {
				bcmtch_data_ptr->post_boot_section++;
			}
		}
	}

	return pb_entry[bcmtch_data_ptr->post_boot_section].length;
}

static int32_t bcmtch_dev_post_boot_download(
		struct bcmtch_data *bcmtch_data_ptr,
		int16_t data_rate)
{
	int32_t ret_val;
	int16_t write_length;

	struct combi_entry *pb_entry = NULL;

	if (bcmtch_data_ptr->post_boot_left)	{
		write_length = data_rate;

		if (bcmtch_data_ptr->post_boot_left < write_length)
			write_length = bcmtch_data_ptr->post_boot_left;

		ret_val = bcmtch_com_write_sys(
					bcmtch_data_ptr,
					bcmtch_data_ptr->post_boot_addr,
					write_length,
					bcmtch_data_ptr->post_boot_data);

		if (ret_val) {
			BCMTCH_ERR("%s() Error - did not download\n",
					__func__);
		} else {
			bcmtch_data_ptr->post_boot_addr += write_length;
			bcmtch_data_ptr->post_boot_left -= write_length;
			bcmtch_data_ptr->post_boot_data += write_length;
		}
	} else {
		BCMTCH_ERR("%s() Error - no bytes to download\n",
				__func__);
	}

	if (!bcmtch_data_ptr->post_boot_left) {

		pb_entry = (struct combi_entry *)
			bcmtch_data_ptr->post_boot_buffer;

		BCMTCH_DBG(
			BCMTCH_DF_PB,
			"PB:Section %d  Addr 0x%08x  Len %d\n",
			bcmtch_data_ptr->post_boot_section,
			pb_entry[bcmtch_data_ptr->
			post_boot_section].addr,
			pb_entry[bcmtch_data_ptr->
			post_boot_section].length);

		if (pb_entry[bcmtch_data_ptr->post_boot_section].flags &
				BCMTCH_FIRMWARE_FLAGS_POST_BOOT_PATCH) {
			if (--bcmtch_data_ptr->post_boot_patches == 0) {
				/* send patch init */
				ret_val = bcmtch_dev_send_command(
					bcmtch_data_ptr,
					TOFE_COMMAND_INIT_POST_BOOT_PATCHES,
					0,
					0,
					0);
				if (ret_val != 0) {
					BCMTCH_ERR(
						" send_command error [%d] cmd=%x\n",
						ret_val,
					TOFE_COMMAND_INIT_POST_BOOT_PATCHES);
				}
			}
		}

		if (--bcmtch_data_ptr->post_boot_sections) {
			bcmtch_data_ptr->post_boot_section++;
			bcmtch_dev_post_boot_get_section(bcmtch_data_ptr);
		}
	}

	BCMTCH_DBG(BCMTCH_DF_PB, "PB:pb_sec=%d post_boot_left=%d.\n",
		bcmtch_data_ptr->post_boot_sections,
		bcmtch_data_ptr->post_boot_left);

	return bcmtch_data_ptr->post_boot_sections;
}

static int32_t bcmtch_init_deferred_worker(
		struct bcmtch_data *bcmtch_data_ptr)
{
	int32_t ret_val = 0;

	if (bcmtch_data_ptr) {
		bcmtch_data_ptr->p_workqueue =
			create_workqueue("bcmtch_wq");

		if (bcmtch_data_ptr->p_workqueue) {

			INIT_WORK(
				&bcmtch_data_ptr->work,
				bcmtch_deferred_worker);

		} else {
			BCMTCH_ERR("%s() Unable to create workqueue\n",
					__func__);

			ret_val = -ENOMEM;
		}
	} else {
		BCMTCH_ERR("%s() error, driver data structure == NULL\n",
		       __func__);
		ret_val = -ENODATA;
	}

	return ret_val;
}

static void bcmtch_clear_deferred_worker(
		struct bcmtch_data *bcmtch_data_ptr)
{
	if (bcmtch_data_ptr && bcmtch_data_ptr->p_workqueue)
		flush_workqueue(bcmtch_data_ptr->p_workqueue);
}

static void bcmtch_free_deferred_worker(
		struct bcmtch_data *bcmtch_data_ptr)
{
	if (bcmtch_data_ptr && bcmtch_data_ptr->p_workqueue) {
		cancel_work_sync(&bcmtch_data_ptr->work);
		destroy_workqueue(bcmtch_data_ptr->p_workqueue);

		bcmtch_data_ptr->p_workqueue = NULL;
	}
}

static void bcmtch_reset(
		struct bcmtch_data *bcmtch_data_ptr)
{
	if (bcmtch_data_ptr &&
		gpio_is_valid(bcmtch_data_ptr->
			platform_data.gpio_reset_pin)) {
		msleep(bcmtch_data_ptr->platform_data.gpio_reset_time_ms);

		gpio_set_value(
			bcmtch_data_ptr->platform_data.gpio_reset_pin,
			bcmtch_data_ptr->platform_data.gpio_reset_polarity);

		msleep(bcmtch_data_ptr->platform_data.gpio_reset_time_ms);

		gpio_set_value(
			bcmtch_data_ptr->platform_data.gpio_reset_pin,
			!bcmtch_data_ptr->platform_data.gpio_reset_polarity);

		msleep(bcmtch_data_ptr->platform_data.gpio_reset_time_ms);
	}
}

static int32_t bcmtch_init_gpio(
		struct bcmtch_data *bcmtch_data_ptr)
{
	struct bcmtch_platform_data *p_platform_data;
	int32_t ret_val = 0;

	if (!bcmtch_data_ptr) {
		BCMTCH_ERR("%s() error, driver data structure == NULL\n",
				__func__);

		ret_val = -ENODATA;
		return ret_val;
	}

	p_platform_data =
		(struct bcmtch_platform_data *)&bcmtch_data_ptr->platform_data;

    /*
     * setup a gpio pin for BCM Touch Controller reset function
     */
	if (gpio_is_valid(p_platform_data->gpio_reset_pin)) {
		ret_val = gpio_request(
					p_platform_data->gpio_reset_pin,
					"BCMTCH reset");

		if (ret_val < 0) {
			BCMTCH_ERR(
				"ERROR: %s() - Unable to request reset pin %d\n",
				__func__,
				p_platform_data->gpio_reset_pin);

			/* note :
			* it is an error if a reset pin is requested
			* and not granted --> return
			*/
			return ret_val;
		}

		/*
		* setup reset pin as output
		* - invert reset polarity --> don't want to hold in reset
		*/
		ret_val = gpio_direction_output(
					p_platform_data->gpio_reset_pin,
					!p_platform_data->gpio_reset_polarity);

		if (ret_val < 0) {
			BCMTCH_ERR("ERROR: %s() - Unable to set reset pin %d\n",
					__func__,
					p_platform_data->gpio_reset_pin);

			/* note :
			* it is an error if a reset pin is requested
			* and not set --> return
			*/
			return ret_val;
		}
	} else {
		BCMTCH_INFO("%s() : no reset pin configured\n",
				__func__);
	}

	/*
	* setup a gpio pin for BCM Touch Controller interrupt function
	*/
	if (gpio_is_valid(p_platform_data->gpio_interrupt_pin)) {
		ret_val = gpio_request(
					p_platform_data->gpio_interrupt_pin,
					"BCMTCH Interrupt");

		if (ret_val < 0) {
			BCMTCH_ERR(
				"ERROR: %s() - Unable to request interrupt pin %d\n",
				__func__,
				p_platform_data->gpio_interrupt_pin);

			/* note :
			* it is an error if an interrupt pin is requested
			* and not granted --> return
			*/
			return ret_val;
		}

		/* setup interrupt pin as input */
		ret_val = gpio_direction_input(
					p_platform_data->gpio_interrupt_pin);
		if (ret_val < 0) {
			BCMTCH_ERR(
				"ERROR: %s() - Unable to set interrupt pin %d\n",
				__func__,
				p_platform_data->gpio_interrupt_pin);

			/* note :
			 * it is an error if a interrupt pin is requested
			 * and not set --> return
			 */
			return ret_val;
		}
	} else if (!p_platform_data->touch_irq) {
		BCMTCH_INFO("%s() : no interrupt pin configured\n",
				__func__);
	}

	return ret_val;
}

static void bcmtch_free_gpio(
		struct bcmtch_data *bcmtch_data_ptr)
{
	struct bcmtch_platform_data *p_platform_data;

	if (bcmtch_data_ptr) {

		p_platform_data =
			(struct bcmtch_platform_data *)
				&bcmtch_data_ptr->platform_data;

		if (gpio_is_valid(p_platform_data->gpio_reset_pin))
			gpio_free(p_platform_data->gpio_reset_pin);


		if (gpio_is_valid(p_platform_data->gpio_interrupt_pin))
			gpio_free(p_platform_data->gpio_interrupt_pin);

	}
}

static int32_t bcmtch_os_init_cli(struct device *p_device)
{
	int32_t ret_val = 0;

	ret_val = device_create_file(p_device, &bcmtch_cli_attr);
	if (ret_val)
			BCMTCH_ERR(
				"ERROR: %s() - device_create_file() failed!\n",
				__func__);

	return ret_val;
}

static inline void bcmtch_os_free_cli(struct device *p_device)
{
	device_remove_file(p_device, &bcmtch_cli_attr);
}

/* ----------------------------------------- */
/* --- BCM Touch Controller PM Functions --- */
/* ----------------------------------------- */
#ifdef CONFIG_PM
#ifndef CONFIG_HAS_EARLYSUSPEND
static int bcmtch_suspend(struct i2c_client *p_client, pm_message_t mesg)
{
	if (mesg.event == PM_EVENT_SUSPEND)
		bcmtch_dev_suspend(i2c_get_clientdata(p_client));

	return 0;
}

static int bcmtch_resume(struct i2c_client *p_client)
{
	int rc = 0;

	bcmtch_dev_resume(i2c_get_clientdata(p_client));

	return rc;
}
#endif
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void bcmtch_early_suspend(struct early_suspend *h)
{
	struct bcmtch_data *local_bcmtch_data_p =
		container_of(
			h,
			struct bcmtch_data,
			bcmtch_early_suspend_desc);

	bcmtch_dev_suspend(local_bcmtch_data_p);
}

static void bcmtch_late_resume(struct early_suspend *h)
{
	struct bcmtch_data *bcmtch_data_ptr =
		container_of(h, struct bcmtch_data,
			bcmtch_early_suspend_desc);

	bcmtch_dev_resume(bcmtch_data_ptr);
}

static void bcmtch_register_early_suspend(
		struct bcmtch_data *bcmtch_data_ptr)
{
	/* Init early suspend parameters */
	bcmtch_data_ptr->bcmtch_early_suspend_desc.level =
		EARLY_SUSPEND_LEVEL_STOP_DRAWING;
	bcmtch_data_ptr->bcmtch_early_suspend_desc.suspend =
		bcmtch_early_suspend;
	bcmtch_data_ptr->bcmtch_early_suspend_desc.resume =
		bcmtch_late_resume;

	/* Register early suspend parameters */
	register_early_suspend(&bcmtch_data_ptr->
		bcmtch_early_suspend_desc);
}

static inline void bcmtch_unregister_early_suspend(
		struct bcmtch_data *bcmtch_data_ptr)
{

	/* Unregister early suspend parameters */
	unregister_early_suspend(&bcmtch_data_ptr->
		bcmtch_early_suspend_desc);
}
#endif

/* ----------------------------------------- */
/* -- BCM Touch Controller I2C Functions --- */
/* ----------------------------------------- */
static int32_t bcmtch_i2c_read_spm(
					struct i2c_client *p_i2c,
					uint8_t reg,
					uint8_t *data)
{
	int32_t ret_val = 0;

	/* setup I2C messages for single byte read transaction */
	struct i2c_msg msg[2] = {
		/* first write register to spm */
		{.addr = p_i2c->addr, .flags = 0, .len = 1, .buf = &reg},
		/* Second read data from spm reg */
		{.addr = p_i2c->addr, .flags = I2C_M_RD, .len = 1, .buf = data}
	};

	if (i2c_transfer(p_i2c->adapter, msg, 2) != 2)
		ret_val = -EIO;

	return ret_val;
}

static int32_t bcmtch_i2c_write_spm(
					struct i2c_client *p_i2c,
					uint8_t reg,
					uint8_t data)
{
	int32_t ret_val = 0;

	/* setup buffer with reg address and data */
	uint8_t buffer[2] = { reg, data };

	/* setup I2C message for single byte write transaction */
	struct i2c_msg msg[1] = {
		/* first write message to spm */
		{.addr = p_i2c->addr, .flags = 0, .len = 2, .buf = buffer}
	};

	if (i2c_transfer(p_i2c->adapter, msg, 1) != 1)
		ret_val = -EIO;

	return ret_val;
}

static int32_t bcmtch_i2c_fast_write_spm(
						struct i2c_client *p_i2c,
						uint8_t count,
						uint8_t *regs,
						uint8_t *data)
{
	int32_t ret_val = 0;

	/*
	 * support hard-coded for a max of 5 spm write messages
	 *
	 * - 1 i2c message uses 2 uint8_t buffers
	 *
	 */
	uint8_t buffer[10];	/* buffers for reg address and data */
	struct i2c_msg msg[5];
	uint32_t n_msg = 0;
	uint8_t *buf_ptr = buffer;

	/* setup I2C message for single byte write transaction */
	while (n_msg < count) {
		msg[n_msg].addr = p_i2c->addr;
		msg[n_msg].flags = 0;
		msg[n_msg].len = 2;
		msg[n_msg].buf = buf_ptr;

		*buf_ptr++ = regs[n_msg];
		*buf_ptr++ = data[n_msg];
		n_msg++;
	}

	if (i2c_transfer(p_i2c->adapter, msg, n_msg) != n_msg)
		ret_val = -EIO;

	return ret_val;
}

#if BCMTCH_STATE_PROTOCOL

static int32_t bcmtch_state_i2c_send_command(
			struct bcmtch_data *bcmtch_data_ptr,
			uint8_t cmd,
			uint8_t *params,
			int param_num)
{
	int ret_val = 0;

	struct i2c_client *i2c_spm_ptr =
			bcmtch_data_ptr->p_i2c_client_spm;
	struct i2c_client *i2c_sys_ptr =
			bcmtch_data_ptr->p_i2c_client_sys;
	uint8_t *i2c_buffer =
			bcmtch_data_ptr->bcmtch_state_cmd_buffer;

	uint8_t *cmd_buffer = &i2c_buffer[1];
	struct i2c_msg msg;
	int	i, dma_len;

	/* set target register */
	i2c_buffer[0] = BCMTCH_SPM_REG_MSG_FROM_HOST;

	/* distinguish short/long command */
	if (param_num < TOFE_HOST_CMD_SHORT_SIZE) {
		/* set command */
		cmd_buffer[0] = cmd;

		/* set parameters */
		for (i = 0; i < param_num; i++)
			cmd_buffer[i+1] = params[i];

		/* padding */
		for (i = param_num; i < TOFE_HOST_CMD_SHORT_SIZE; i++)
			cmd_buffer[i+1] = 0;

		if (cmd == BCMTCH_CMD_RESUME)
			cmd_buffer[TOFE_HOST_CMD_SHORT_SIZE] =
				BCMTCH_POWER_MODE_WAKE;

		/* i2c write transaction */
		msg.addr = i2c_spm_ptr->addr;
		msg.flags = 0;
		msg.len = TOFE_HOST_CMD_SHORT_SIZE+2;
		msg.buf = i2c_buffer;

		if (i2c_transfer(i2c_spm_ptr->adapter, &msg, 1) != 1)
			return -EIO;

		/* 500 us delay for FW to process the command */
		usleep_range(500, 1000);

	} else {
		BCMTCH_DBG(
			BCMTCH_DF_ST,
			"ST: %s long cmd=0x%0x param_num=%d\n",
			__func__,
			cmd,
			param_num);

		/* Before sending a long command, we need to make sure the chip
		 * is in Active mode (which enables the I2C DMA). The Retention
		 * mode is used only in two modes: StartScan command (for power
		 * saving) and Suspend. Here we terminate the StartScan command
		 * in case it's pending.
		 */
		if (cmd_buffer[0] == BCMTCH_CMD_START_SCAN ||
				cmd_buffer[0] == BCMTCH_CMD_SINGLE_SCAN) {
			cmd_buffer[0] = 0;
			ret_val = bcmtch_state_stop_scan(bcmtch_data_ptr);
			if (ret_val)
				return ret_val;
		}

		cmd_buffer[0] = BCMTCH_CMD_LONG;
		for (i = 0; i < TOFE_HOST_CMD_SHORT_SIZE; i++)
			cmd_buffer[i+1] = 0;

		/* i2c write transaction */
		msg.addr = i2c_spm_ptr->addr;
		msg.flags = 0;
		msg.len = TOFE_HOST_CMD_SHORT_SIZE+2;
		msg.buf = i2c_buffer;

		if (i2c_transfer(i2c_spm_ptr->adapter, &msg, 1) != 1)
			return -EIO;

		/* 500 us delay for FW to process the command */
		usleep_range(500, 1000);

		/* setup i2c messages for DMA write request transaction */
		i2c_buffer[0] = BCMTCH_SPM_REG_DMA_WFIFO;
		cmd_buffer[0] = cmd;
		memcpy(&cmd_buffer[1], params, param_num);
		dma_len = (((param_num+1) + 3) & ~0x3) + 1;

		/* padding */
		for (i = param_num + 2; i < dma_len; i++)
			i2c_buffer[i] = 0;

		/* debug */
		BCMTCH_DBG(
			BCMTCH_DF_ST,
			"ST: send long cmd params: dma_len=%d param_num=%d",
			dma_len,
			param_num);
		for (i = 0; i < dma_len; i++)
			BCMTCH_DBG(
				BCMTCH_DF_I2C,
				"I2C: 0x%02x",
				i2c_buffer[i]);

		BCMTCH_DBG(BCMTCH_DF_ST, "ST: END.");

		/* i2c write transaction */
		msg.addr = i2c_sys_ptr->addr;
		msg.flags = 0;
		msg.len = dma_len;
		msg.buf = i2c_buffer;

		if (i2c_transfer(i2c_spm_ptr->adapter, &msg, 1) != 1)
			ret_val = -EIO;
	}

	return ret_val;
}

#endif /* BCMTCH_STATE_PROTOCOL */

static int32_t bcmtch_i2c_read_dma(
						struct i2c_client *p_i2c,
						uint16_t read_len,
						uint8_t *read_data)
{
	int32_t ret_val = 0;
	uint8_t dma_reg = BCMTCH_SPM_REG_DMA_RFIFO;

	/* setup I2C messages for DMA read transaction */
	struct i2c_msg dma_read[2] = {
		/* next write messages to read the DMA request status */
		{.addr = p_i2c->addr, .flags = 0, .len = 1, .buf = &dma_reg},
		{.addr = p_i2c->addr, .flags = I2C_M_RD,
				.len = read_len, .buf = read_data}
	};

	/* read status */
	if (i2c_transfer(p_i2c->adapter, dma_read, 2) != 2)
			ret_val = -EIO;

	return ret_val;
}

static int32_t bcmtch_i2c_read_sys(
						struct i2c_client *p_i2c,
						uint32_t sys_addr,
						uint16_t read_len,
						uint8_t *read_data)
{
	int32_t ret_val = 0;
	uint8_t dma_reg = BCMTCH_SPM_REG_DMA_RFIFO;

	/* setup the DMA header for this read transaction */
	uint8_t dma_header[8] = {
		/* set dma controller addr */
		BCMTCH_SPM_REG_DMA_ADDR,
		/* setup dma address */
		(sys_addr & 0xFF),
		((sys_addr & 0xFF00) >> 8),
		((sys_addr & 0xFF0000) >> 16),
		((sys_addr & 0xFF000000) >> 24),
		/* setup dma length */
		(read_len & 0xFF),
		((read_len & 0xFF00) >> 8),
		/* setup dma mode */
		BCMTCH_DMA_MODE_READ
	};

	/* setup I2C messages for DMA read request transaction */
	struct i2c_msg dma_request[3] = {
		/* write DMA request header */
		{.addr = p_i2c->addr, .flags = 0, .len = 8, .buf = dma_header},

		/* next write messages to read the DMA request */
		{.addr = p_i2c->addr, .flags = 0, .len = 1, .buf = &dma_reg},
		{.addr = p_i2c->addr, .flags = I2C_M_RD,
				.len = read_len, .buf = read_data}
	};

	/* send complete DMA request */
	if (i2c_transfer(p_i2c->adapter, dma_request, 3) != 3)
		ret_val = -EIO;

	return ret_val;
}

static int32_t bcmtch_i2c_write_sys(
					struct i2c_client *p_i2c,
					uint32_t sys_addr,
					uint16_t write_len,
					uint8_t *write_data)
{

	int total_dma_len, len, dma_len, ret_val = 0;
	uint8_t *dma_data;
	struct i2c_msg request;

	uint8_t dma_header[8] = {
		/* dma controller addr */
		BCMTCH_SPM_REG_DMA_ADDR,
		/* dma address */
		(sys_addr & 0xFF),
		((sys_addr & 0xFF00) >> 8),
		((sys_addr & 0xFF0000) >> 16),
		((sys_addr & 0xFF000000) >> 24),
		/* dma length */
		(write_len & 0xFF),
		((write_len & 0xFF00) >> 8),
		/* dma mode */
		BCMTCH_DMA_MODE_WRITE
	};

	total_dma_len = write_len + 1;

	dma_data = kzalloc(SZ_4K, GFP_KERNEL);
	if (!dma_data)
		return -ENOMEM;

	request.addr = p_i2c->addr;
	request.flags = 0;
	request.len = 8;
	request.buf = dma_header;

	if (i2c_transfer(p_i2c->adapter, &request, 1) != 1) {
		ret_val = -EIO;
		goto error;
	}

	for (len = total_dma_len; len > 0; len -= (SZ_4K - 1)) {

		if (len < (SZ_4K - 1))
			dma_len = len;
		else
			dma_len = SZ_4K;

		request.addr = p_i2c->addr;
		request.flags = 0;
		request.len = dma_len;
		request.buf = dma_data;

		dma_data[0] = BCMTCH_SPM_REG_DMA_WFIFO;
		memcpy(&dma_data[1],
			(write_data + (total_dma_len - len)), dma_len - 1);

		if (i2c_transfer(p_i2c->adapter, &request, 1) != 1) {
			ret_val = -EIO;
			break;
		}
	}

error:
	kfree(dma_data);

	return ret_val;
}

static int32_t bcmtch_i2c_init_clients(struct i2c_client *p_i2c_client_spm)
{
	int32_t ret_val = 0;
	struct i2c_client *p_i2c_client_sys;
	struct bcmtch_data *bcmtch_data_ptr =
		i2c_get_clientdata(p_i2c_client_spm);

	if (p_i2c_client_spm->adapter) {
		/* Configure the second I2C slave address. */
		p_i2c_client_sys =
			i2c_new_dummy(
				p_i2c_client_spm->adapter,
				bcmtch_data_ptr->platform_data.i2c_addr_sys);

		if (p_i2c_client_sys) {
			/* assign */
			bcmtch_data_ptr->p_i2c_client_spm = p_i2c_client_spm;
			bcmtch_data_ptr->p_i2c_client_sys = p_i2c_client_sys;
		} else {
			BCMTCH_ERR(
				"%s() i2c_new_dummy == NULL, slave address: 0x%x\n",
				__func__,
				bcmtch_data_ptr->platform_data.i2c_addr_sys);

			ret_val = -ENODEV;
		}
	} else {
		BCMTCH_ERR("%s() p_i2c_adapter == NULL\n",
				__func__);

		ret_val = -ENODEV;
	}

	return ret_val;
}

static inline void bcmtch_i2c_free_clients(
		struct bcmtch_data *bcmtch_data_ptr)
{
	if (bcmtch_data_ptr && (bcmtch_data_ptr->p_i2c_client_sys)) {
		i2c_unregister_device(bcmtch_data_ptr->p_i2c_client_sys);
		bcmtch_data_ptr->p_i2c_client_sys = NULL;
	}
}

/* ------------------------------------------------- */
/* -- BCM Touch Controller I2C Driver Structures --- */
/* ------------------------------------------------- */
static const struct i2c_device_id bcmtch_i2c_id[] = {
	{BCMTCH15XXX_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, bcmtch_i2c_id);

static struct i2c_driver bcmtch_i2c_driver = {
	.driver = {
		   .name = BCMTCH15XXX_NAME,
		   .owner = THIS_MODULE,
		   },
	.probe = bcmtch_i2c_probe,
	.remove = bcmtch_i2c_remove,
	.id_table = bcmtch_i2c_id,

#ifdef CONFIG_PM
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = bcmtch_suspend,
	.resume = bcmtch_resume,
#endif
#endif
};

/* ------------------------------------------------ */
/* -- BCM Touch Controller I2C Driver Functions --- */
/* ------------------------------------------------ */
static int32_t bcmtch_i2c_probe(
					struct i2c_client *p_i2c_client,
					const struct i2c_device_id *id)
{
	struct bcmtch_data *bcmtch_data_ptr;
	int32_t ret_val = 0;

	BCMTCH_DBG(BCMTCH_DF_INFO, "INFO: Driver: %s : %s : %s\n",
			BCMTCH_DRIVER_VERSION,
			BCMTCH_DRIVER_BUILD_DATE,
			BCMTCH_DRIVER_BUILD_TIME);

	/* print driver probe header */
	if (p_i2c_client)
		BCMTCH_DBG(
			BCMTCH_DF_INFO,
			"INFO: dev=%s addr=0x%x irq=%d\n",
			p_i2c_client->name,
			p_i2c_client->addr,
			p_i2c_client->irq);

	if (id)
		BCMTCH_DBG(BCMTCH_DF_INFO, "INFO: match id=%s\n",
				id->name);

	/* allocate global BCM Touch Controller driver structure */
	ret_val = bcmtch_dev_alloc(p_i2c_client);
	if (ret_val)
		goto probe_error;

	/* setup local platform data from client device structure */
	ret_val = bcmtch_dev_init_platform(&p_i2c_client->dev);
	if (ret_val)
		goto init_platform_error;

	bcmtch_data_ptr = i2c_get_clientdata(p_i2c_client);

	mutex_lock(&bcmtch_data_ptr->mutex_work);

	/* initialize deferred worker (workqueue/tasklet/etc */
	ret_val = bcmtch_init_deferred_worker(bcmtch_data_ptr);
	if (ret_val)
		goto worker_error;

	/* initialize power supplies */
	ret_val = bcmtch_dev_power_init(bcmtch_data_ptr);
	if (ret_val)
		goto power_init_error;

	/* enable power supplies */
	ret_val = bcmtch_dev_power_enable(bcmtch_data_ptr, true);
	if (ret_val)
		goto power_enable_error;

	/* setup the gpio pins
	 * - 1 gpio used for reset control signal to BCM Touch Controller
	 * - 1 gpio used as interrupt signal from BCM Touch Controller
	 */
	ret_val = bcmtch_init_gpio(bcmtch_data_ptr);
	if (ret_val)
		goto gpio_error;

	/*
	* setup the i2c clients and bind (store pointers in global structure)
	* 1. SPM I2C client
	* 2. SYS I2C client
	*/
	ret_val = bcmtch_i2c_init_clients(p_i2c_client);
	if (ret_val)
		goto i2c_client_error;

    /* reset the chip on driver load ? */
	if (bcmtch_boot_flag &
			BCMTCH_BF_HARD_RESET_ON_LOAD) {
		bcmtch_dev_reset(
			bcmtch_data_ptr,
			BCMTCH_RESET_MODE_HARD);
	} else if (bcmtch_boot_flag &
			BCMTCH_BF_SOFT_RESET_ON_LOAD) {
		bcmtch_dev_reset(
			bcmtch_data_ptr,
			BCMTCH_RESET_MODE_SOFT_CHIP |
			BCMTCH_RESET_MODE_SOFT_ARM);
	}

	/* init the table of the worker process functions */
	bcmtch_dev_init_worker_process(bcmtch_data_ptr);

	/* perform BCM Touch Controller initialization */
	ret_val = bcmtch_dev_init(bcmtch_data_ptr);
	if (ret_val)
		goto init_dev_error;

	/* setup the os cli */
	ret_val = bcmtch_os_init_cli(&p_i2c_client->dev);
	if (ret_val)
		goto cli_error;

	/* setup the sysfs api */
	ret_val = bcmtch_os_init_abi(&p_i2c_client->dev);
	if (ret_val)
		goto sysfs_api_error;

	/* setup the os input device*/
	ret_val = bcmtch_init_input_device(bcmtch_data_ptr);
	if (ret_val)
		goto input_dev_error;

	bcmtch_dev_watchdog_start(bcmtch_data_ptr);

	ret_val = bcmtch_interrupt_enable(bcmtch_data_ptr);
	if (ret_val)
		goto interrupt_error;

	mutex_unlock(&bcmtch_data_ptr->mutex_work);

#ifdef CONFIG_HAS_EARLYSUSPEND
	bcmtch_register_early_suspend(bcmtch_data_ptr);
#endif /* CONFIG_HAS_EARLYSUSPEND */
	BCMTCH_INFO("PROBE: success\n");

	return 0;

interrupt_error:

input_dev_error:
	/* Undo input device init */
	bcmtch_free_input_device(bcmtch_data_ptr);

sysfs_api_error:
	/* Undo sysfs api init */
	bcmtch_os_free_abi(&p_i2c_client->dev);

cli_error:
	/* Undo os cli init */
	bcmtch_os_free_cli(&p_i2c_client->dev);

init_dev_error:
	/* Undo touch controller initialization */

i2c_client_error:
	/* Undo i2c clients init */
	bcmtch_i2c_free_clients(bcmtch_data_ptr);

gpio_error:
	/* Undo gpio init */
	bcmtch_free_gpio(bcmtch_data_ptr);

power_enable_error:
		/* undo power enable */
	bcmtch_dev_power_enable(bcmtch_data_ptr, false);

power_init_error:
	/* undo power init */
	bcmtch_dev_power_free(bcmtch_data_ptr);

	/* Undo worker init */
	bcmtch_free_deferred_worker(bcmtch_data_ptr);

worker_error:
	mutex_unlock(&bcmtch_data_ptr->mutex_work);

init_platform_error:
	/* Undo platform init */
	bcmtch_dev_free(p_i2c_client);

probe_error:
	BCMTCH_ERR("PROBE: failure\n");

	return ret_val;
}

static int32_t bcmtch_i2c_remove(struct i2c_client *p_i2c_client)
{
	struct bcmtch_data *bcmtch_data_ptr =
		i2c_get_clientdata(p_i2c_client);

    /* disable interrupts */
	bcmtch_interrupt_disable(bcmtch_data_ptr);

#if BCMTCH_STATE_PROTOCOL
	/* state protocol stop scan */
	if (bcmtch_data_ptr->work_process_index >= BCMTCH_WP_STATE)
		bcmtch_state_stop_scan(bcmtch_data_ptr);
#endif /* BCMTCH_STATE_PROTOCOL */

	/* free watchdog timer */
	bcmtch_dev_watchdog_stop(bcmtch_data_ptr);

	/* free deferred worker (queue) */
	bcmtch_free_deferred_worker(bcmtch_data_ptr);

	mutex_lock(&bcmtch_data_ptr->mutex_work);

#ifdef CONFIG_HAS_EARLYSUSPEND
	bcmtch_unregister_early_suspend(bcmtch_data_ptr);
#endif /* CONFIG_HAS_EARLYSUSPEND */

    /* force chip to sleep before exiting */
	if (BCMTCH_POWER_STATE_SLEEP !=
			bcmtch_dev_get_power_state(bcmtch_data_ptr)) {
		bcmtch_dev_set_power_state(bcmtch_data_ptr,
			BCMTCH_POWER_STATE_SLEEP);
	}

	/* disable power */
	bcmtch_dev_power_enable(bcmtch_data_ptr, false);

	/* release power */
	bcmtch_dev_power_free(bcmtch_data_ptr);

	/* reset + free communication channels */
	bcmtch_dev_post_boot_reset(bcmtch_data_ptr);

	/* free i2c device clients */
	bcmtch_i2c_free_clients(bcmtch_data_ptr);

	/* Undo sysfs api init */
	bcmtch_os_free_abi(&p_i2c_client->dev);

	/* remove the os cli */
	bcmtch_os_free_cli(&p_i2c_client->dev);

	/* free input device */
	bcmtch_free_input_device(bcmtch_data_ptr);

	/* free used gpio pins */
	bcmtch_free_gpio(bcmtch_data_ptr);

	/* free this mem last */
	bcmtch_dev_free(p_i2c_client);

	return 0;
}

static int32_t __init bcmtch_i2c_init(void)
{
	return i2c_add_driver(&bcmtch_i2c_driver);
}

late_initcall(bcmtch_i2c_init);

static void __exit bcmtch_i2c_exit(void)
{
	i2c_del_driver(&bcmtch_i2c_driver);
}

module_exit(bcmtch_i2c_exit);

MODULE_DESCRIPTION("I2C support for BCMTCH15XXX Touchscreen");
MODULE_LICENSE("GPL");
MODULE_VERSION(BCMTCH_DRIVER_VERSION);
