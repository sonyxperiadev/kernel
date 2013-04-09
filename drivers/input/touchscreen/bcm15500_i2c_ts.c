/*****************************************************************************
* Copyright (c) 2011 Broadcom Corporation.  All rights reserved.
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.  IF YOU HAVE NO
* AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
* WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
* THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use
*    all reasonable efforts to protect the confidentiality thereof, and to
*    use this information only in connection with your use of Broadcom
*    integrated circuit products.
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*****************************************************************************/

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
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/kfifo.h>
#include <linux/version.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/firmware.h>
#include <linux/input/mt.h>
#include <linux/time.h>
#include <linux/timer.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#include <linux/i2c/bcm15500_i2c_ts.h>

/* -------------------------------------- */
/* - BCM Touch Controller Driver Macros - */
/* -------------------------------------- */

/* -- driver version -- */
#define	BCMTCH_DRIVER_VERSION	"1.3.0.01_RC1"

/* -- SPM addresses -- */
#define BCMTCH_SPM_REG_REVISIONID   0x40
#define BCMTCH_SPM_REG_CHIPID0      0x41
#define BCMTCH_SPM_REG_CHIPID1      0x42
#define BCMTCH_SPM_REG_CHIPID2      0x43

#define BCMTCH_SPM_REG_SPI_I2C_SEL  0x44
#define BCMTCH_SPM_REG_I2CS_CHIPID  0x45

#define BCMTCH_SPM_REG_PSR          0x48

#if (BCMTCH_HW_CHIP_VERSION == BCMTCH_HW_BCM15200A0||BCMTCH_HW_BCM15200A1)

#define BCMTCH_SPM_REG_MSG_FROM_HOST    0x49
#define BCMTCH_SPM_REG_MSG_FROM_HOST_1  0x4a
#define BCMTCH_SPM_REG_MSG_FROM_HOST_2  0x4b
#define BCMTCH_SPM_REG_MSG_FROM_HOST_3  0x4c
#define BCMTCH_SPM_REG_RQST_FROM_HOST   0x4d
#define BCMTCH_SPM_REG_MSG_TO_HOST      0x4e

#elif (BCMTCH_HW_CHIP_VERSION == BCMTCH_HW_BCM15500A0)

#define BCMTCH_SPM_REG_RQST_FROM_HOST   0x4c
#define BCMTCH_SPM_REG_MSG_TO_HOST      0x4d
#define BCMTCH_SPM_REG_MSG_FROM_HOST    0x4e

#endif

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
#if (BCMTCH_HW_CHIP_VERSION == BCMTCH_HW_BCM15200A0||BCMTCH_HW_BCM15200A1)
#define BCMTCH_ADDR_SPM_REMAP                (BCMTCH_ADDR_SPM_BASE + 0x100)
#endif
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
#define	BCMTCH_ROM_CHANNEL			1
#define	BCMTCH_POST_BOOT			1

/* -- constants -- */
#define BCMTCH_SUCCESS      0

#define BCMTCH_MAX_TOUCH    10

#define BCMTCH_MAX_X        4096
#define BCMTCH_MAX_Y        4096

/* hypot of (23*15) in x16 units == 439 */
#define BCMTCH_MAX_TOUCH_MAJOR	439
#define BCMTCH_MAX_TOUCH_MINOR	439
#define BCMTCH_MAX_WIDTH_MAJOR	439
#define BCMTCH_MAX_WIDTH_MINOR	439

#define BCMTCH_MAX_PRESSURE		500
#define BCMTCH_MIN_ORIENTATION	-512
#define BCMTCH_MAX_ORIENTATION	511

#define BCMTCH_DMA_MODE_READ    1
#define BCMTCH_DMA_MODE_WRITE   3

#define BCMTCH_IF_I2C_SEL       0
#define BCMTCH_IF_SPI_SEL       1

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

#define BCMTCH_RESET_MODE_SOFT_CLEAR    0x00
#define BCMTCH_RESET_MODE_SOFT_CHIP     0x01
#define BCMTCH_RESET_MODE_SOFT_ARM      0x02
#define BCMTCH_RESET_MODE_HARD          0x04

#if (BCMTCH_HW_CHIP_VERSION == BCMTCH_HW_BCM15200A0||BCMTCH_HW_BCM15200A1)
#define BCMTCH_MEM_REMAP_ADDR		BCMTCH_ADDR_SPM_REMAP
#elif (BCMTCH_HW_CHIP_VERSION == BCMTCH_HW_BCM15500A0)
#define BCMTCH_MEM_REMAP_ADDR		BCMTCH_ADDR_COMMON_ARM_REMAP
#endif

#define BCMTCH_MEM_ROM_BOOT 0x00
#define BCMTCH_MEM_RAM_BOOT 0x01
#define BCMTCH_MEM_MAP_1000 0x00
#define BCMTCH_MEM_MAP_3000 0x02

#define BCMTCH_SPM_STICKY_BITS_PIN_RESET    0x02

/* operations */
#define BCMTCH_USE_FAST_I2C     1
#define BCMTCH_USE_BUS_LOCK     0
#define BCMTCH_USE_WORK_LOCK    1
#define BCMTCH_USE_PROTOCOL_B   1
#define BCMTCH_USE_DMA_STATUS	0

/* development and test */
#define PROGRESS() (printk(KERN_INFO "%s : %d\n", __func__, __LINE__))


/* -------------------------------------- */
/* - Touch Firmware Environment (ToFE)  - */
/* -------------------------------------- */
#define TOFE_BUILD_ID_SIZE          8
#define TOFE_SIGNATURE_MAGIC_SIZE   4

#define TOFE_MESSAGE_SOC_REBOOT_PENDING				0x16
#define TOFE_MESSAGE_FW_READY						0x80
#define TOFE_MESSAGE_FW_READY_INTERRUPT				0x81
#define TOFE_MESSAGE_FW_READY_OVERRIDE				0x82
#define TOFE_MESSAGE_FW_READY_INTERRUPT_OVERRIDE	0x83

enum tofe_command_e_ {
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
#define tofe_command_e	enum tofe_command_e_

enum bcmtch_status_e_ {
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
#define	bcmtch_status_e	enum bcmtch_status_e_

/**
	@ struct tofe_command_response_t_
	@ brief Entry structure for command channel.
*/
struct tofe_command_response_t_ {
	uint8_t		flags;
	uint8_t		command;
	uint16_t	result;
	uint32_t	data;
};
#define tofe_command_response_t struct tofe_command_response_t_

#define	TOFE_COMMAND_FLAG_COMMAND_PROCESSED	(1 << 0)
#define	TOFE_COMMAND_FLAG_REQUEST_RESULT	(1 << 7)

/**
    @struct tofe_signature_t
    @brief Firmware ROM image signature structure.
*/
struct  tofe_signature_t_ {
	const char magic[TOFE_SIGNATURE_MAGIC_SIZE];
	const char build_release[4];
	const char build_version[TOFE_BUILD_ID_SIZE];
	const char build_date[TOFE_BUILD_ID_SIZE];
	const char build_time[TOFE_BUILD_ID_SIZE];
};
#define tofe_signature_t struct tofe_signature_t_

/* ToFE Signature */
#define TOFE_SIGNATURE_SIZE sizeof(tofe_signature_t)

#define iterator_t uint8_t

/* -------------------------- */
/* -	Dual Channel Mode	- */
/* -------------------------- */
#if BCMTCH_ROM_CHANNEL

#define iterator_rom_t uint16_t

enum bcmtch_rom_event_type_e_ {
	BCMTCH_EVENT_TYPE_INVALID,	/* Don't use zero. */

	/* Core events. */
	BCMTCH_EVENT_TYPE_FRAME,

	BCMTCH_EVENT_TYPE_DOWN,
	BCMTCH_EVENT_TYPE_MOVE,
	BCMTCH_EVENT_TYPE_UP,

	/* Auxillary events. */
	BCMTCH_EVENT_TYPE_TIMESTAMP,
};
#define bcmtch_rom_event_type_e enum bcmtch_rom_event_type_e_

struct tofe_rom_channel_header_t_ {
	uint32_t	write;

	 /* Number of entries.  Limited to 255 entries. */
	uint8_t		entry_num;
	/* Entry size in bytes.  Limited to 255 bytes. */
	uint8_t		entry_size;
	/* Number of entries in channel to trigger notification */
	uint8_t		trig_level;
	/* Bit definitions shared with configuration. */
	uint8_t		flags;

	uint32_t	read;
	int32_t		data_offset; /* Offset to data. May be negative. */
	iterator_rom_t	read_iterator;
	iterator_rom_t	write_iterator;
};
#define tofe_rom_channel_header_t struct tofe_rom_channel_header_t_

struct tofe_rom_channel_instance_cfg_t_ {
	uint8_t entry_num;	/* Must be > 0. */
	uint8_t entry_size;	/* Range [1..255]. */
	uint8_t trig_level;	/* 0 - entry_num */
	uint8_t flags;
	tofe_rom_channel_header_t *channel_header;
	void *channel_data;
};
#define tofe_rom_channel_instance_cfg_t struct tofe_rom_channel_instance_cfg_t_

struct bcmtch_rom_event_t_ {
	uint32_t	type:4;
	uint32_t	_pad:28; /* embedded pad in bitfield */

	uint32_t	_pad32;
};
#define bcmtch_rom_event_t struct bcmtch_rom_event_t_

struct  bcmtch_rom_event_frame_t_ {
	uint16_t	type:4;
	uint16_t	_pad:12; /* embedded pad in bitfield */
	uint16_t	frame_id;

	uint32_t	hash;
};
#define bcmtch_rom_event_frame_t struct  bcmtch_rom_event_frame_t_

struct bcmtch_rom_event_touch_t_ {
	uint16_t	type:4;
	uint16_t	track_tag:5;
	uint16_t	flags:4;
	uint16_t	tch_class:3;
	/*  Touch class. C++ does not like class.
			Use BCMTCH_EVENT_CLASS_TOUCH. */

	uint16_t	width:8;	/* x direction length of bounding box */
	uint16_t	height:8;	/* y direction length of bounding box */

	uint32_t	z:8;	/* pressure for contact, distance for hover */
	uint32_t	x:12;
	uint32_t	y:12;
};
#define bcmtch_rom_event_touch_t struct bcmtch_rom_event_touch_t_

#endif
/* ---------------------------------- */
/* -	End of Dual Channel Data	- */
/* ---------------------------------- */

/**
    @enum tofe_channel_flag_t
    @brief Channel flag field bit assignment.
*/
enum tofe_channel_flag_t_ {
	TOFE_CHANNEL_FLAG_STATUS_OVERFLOW      = 1 << 0,
	TOFE_CHANNEL_FLAG_STATUS_LEVEL_TRIGGER = 1 << 1,
	TOFE_CHANNEL_FLAG_FWDMA_BUFFER         = 1 << 3,
	TOFE_CHANNEL_FLAG_FWDMA_ENABLE         = 1 << 4,
	TOFE_CHANNEL_FLAG_INTERRUPT_ENABLE     = 1 << 5,
	TOFE_CHANNEL_FLAG_OVERFLOW_STALL       = 1 << 6,
	TOFE_CHANNEL_FLAG_INBOUND              = 1 << 7,
};
#define tofe_channel_flag_t enum tofe_channel_flag_t_

enum tofe_toc_index_ {
	TOFE_TOC_INDEX_CHANNEL = 2,
};
#define tofe_toc_index_e enum tofe_toc_index_

enum tofe_channel_id_t_ {
	TOFE_CHANNEL_ID_TOUCH,
	TOFE_CHANNEL_ID_COMMAND,
	TOFE_CHANNEL_ID_RESPONSE,
	TOFE_CHANNEL_ID_LOG,
};
#define tofe_channel_id_t enum tofe_channel_id_t_	/* Used as index. */

struct tofe_dmac_header_t_ {
	uint16_t			min_size;
	uint16_t			size;
};
#define tofe_dmac_header_t struct tofe_dmac_header_t_

struct tofe_channel_buffer_header_t_ {
	tofe_dmac_header_t	dmac;
	uint8_t				channel_id:4;
	uint8_t				flags:4;
	uint8_t				seq_number;
	uint8_t				entry_size;
	uint8_t				entry_count;
};
#define tofe_channel_buffer_header_t struct tofe_channel_buffer_header_t_

struct tofe_channel_buffer_t_ {
	tofe_channel_buffer_header_t	header;
	uint32_t						data[256];
};
#define tofe_channel_buffer_t struct tofe_channel_buffer_t_

struct tofe_channel_header_t_ {
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
	tofe_channel_buffer_t	*buffer[2];
};
#define tofe_channel_header_t struct tofe_channel_header_t_

struct tofe_channel_instance_cfg_t_ {
	uint8_t entry_num;	/* Must be > 0. */
	uint8_t entry_size;	/* Range [1..255]. */
	uint8_t trig_level;	/* 0 - entry_num */
	uint8_t flags;
	uint8_t buffer_num; /* Number of buffers for this channel */
	uint8_t _pad8;
	uint16_t offset;
	tofe_channel_header_t *channel_header;
	void *channel_data;
};
#define tofe_channel_instance_cfg_t struct tofe_channel_instance_cfg_t_

struct tofe_log_msg_t_ {
	uint16_t	log_code;
	uint16_t	param_0;
	uint32_t	params[2];
	uint32_t	timestamp;
};
#define tofe_log_msg_t struct tofe_log_msg_t_

struct combi_entry_t_ {
	uint32_t offset;
	uint32_t addr;
	uint32_t length;
	uint32_t flags;
};
#define bcmtch_combi_entry_t struct combi_entry_t_
/* ------------------------------------- */
/* - BCM Touch Controller Driver Enums - */
/* ------------------------------------- */

enum bcmtch_channel_e_ {
	/* NOTE : see above tofe_channel_id_t */
	BCMTCH_CHANNEL_TOUCH,
	BCMTCH_CHANNEL_COMMAND,
	BCMTCH_CHANNEL_RESPONSE,
	BCMTCH_CHANNEL_LOG,

	/* last */
	BCMTCH_CHANNEL_MAX
};
#define bcmtch_channel_e enum bcmtch_channel_e_

enum bcmtch_mutex_e_ {
	BCMTCH_MUTEX_BUS,
	BCMTCH_MUTEX_WORK,

	/* last */
	BCMTCH_MUTEX_MAX,
};
#define bcmtch_mutex_e enum bcmtch_mutex_e_

/* event kinds from BCM Touch Controller */
enum bcmtch_event_kind_e_ {
	BCMTCH_EVENT_KIND_RESERVED, /* Avoiding zero, but you may use it. */

	BCMTCH_EVENT_KIND_FRAME,
	BCMTCH_EVENT_KIND_TOUCH,
	BCMTCH_EVENT_KIND_TOUCH_END,
	BCMTCH_EVENT_KIND_BUTTON,
	BCMTCH_EVENT_KIND_GESTURE,

	BCMTCH_EVENT_KIND_EXTENSION = 7,
	BCMTCH_EVENT_KIND_MAX = BCMTCH_EVENT_KIND_EXTENSION,
};
#define bcmtch_event_kind_e enum bcmtch_event_kind_e_

enum _bcmtch_touch_status_ {
	BCMTCH_TOUCH_STATUS_INACTIVE,
	BCMTCH_TOUCH_STATUS_UP,
	BCMTCH_TOUCH_STATUS_MOVE,
	BCMTCH_TOUCH_STATUS_MOVING,
};
#define bcmtch_touch_status_e enum _bcmtch_touch_status_

/* -------------------------------------- */
/* - BCM Touch Controller Device Tables - */
/* -------------------------------------- */

static const uint32_t const BCMTCH_CHIP_IDS[] = {
	0x15200,
	0x15300,
	0x15500,

	0			/* last entry must be 0 */
};

/* ------------------------------------------ */
/* - BCM Touch Controller Driver Parameters - */
/* ------------------------------------------ */
#define BCMTCH_BOOT_FLAG_HARD_RESET_ON_LOAD 0x00000001
#define BCMTCH_BOOT_FLAG_SOFT_RESET_ON_LOAD 0x00000002
#define BCMTCH_BOOT_FLAG_RAM_BOOT           0x00000004
#define BCMTCH_BOOT_FLAG_DISABLE_POST_BOOT  0x00000008
#define BCMTCH_BOOT_FLAG_CHECK_INTERRUPT    0x00000010
#define BCMTCH_BOOT_FLAG_SUSPEND_COLD_BOOT  0x00000020

#define BCMTCH_BOOT_FLAG_RESET_ON_LOAD_MASK \
			(BCMTCH_BOOT_FLAG_HARD_RESET_ON_LOAD \
			| BCMTCH_BOOT_FLAG_SOFT_RESET_ON_LOAD)

static int bcmtch_boot_flag = (BCMTCH_BOOT_FLAG_SOFT_RESET_ON_LOAD);

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

#define BCMTCH_DEBUG_FLAG_FRAME             0x00000001
#define BCMTCH_DEBUG_FLAG_FRAMES            0x00000002
#define BCMTCH_DEBUG_FLAG_MOVE              0x00000004
#define BCMTCH_DEBUG_FLAG_UP                0x00000008
#define BCMTCH_DEBUG_FLAG_BUTTON            0x00000010
#define BCMTCH_DEBUG_FLAG_FRAME_EXT         0x00000020
#define BCMTCH_DEBUG_FLAG_TOUCH_EXT         0x00000040
#define BCMTCH_DEBUG_FLAG_PM                0x00000080
#define BCMTCH_DEBUG_FLAG_CHANNELS          0x00000100
#define BCMTCH_DEBUG_FLAG_POST_BOOT         0x00000200
#define BCMTCH_DEBUG_FLAG_WATCH_DOG         0x00000400
#define BCMTCH_DEBUG_FLAG_IRQ				0x00000800

static int bcmtch_debug_flag = BCMTCH_DEBUG_FLAG_FRAME;

module_param_named(debug_flag, bcmtch_debug_flag, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug_flag, "Debug bit-fields [UP|MV|DN|FR]");

/*-*/

#define BCMTCH_EVENT_FLAG_TOUCH_SIZE		0x00000001
#define BCMTCH_EVENT_FLAG_TOOL_SIZE			0x00000002
#define BCMTCH_EVENT_FLAG_PRESSURE			0x00000004
#define BCMTCH_EVENT_FLAG_ORIENTATION		0x00000008

static int bcmtch_event_flag = BCMTCH_EVENT_FLAG_TOUCH_SIZE
		| BCMTCH_EVENT_FLAG_ORIENTATION;

module_param_named(event_flag, bcmtch_event_flag, int, S_IRUGO);
MODULE_PARM_DESC(event_flag, "Extension events bit-fields [ORIEN|PRESSURE|TOOL_SIZE|TOUCH_SIZE]");

/*- firmware -*/

#define BCMTCH_FIRMWARE_FLAGS_CODE					0x0
#define BCMTCH_FIRMWARE_FLAGS_CONFIGS				0x01
#define BCMTCH_FIRMWARE_FLAGS_POST_BOOT				0x02
#define BCMTCH_FIRMWARE_FLAGS_POST_BOOT_CODE		0x02
#define BCMTCH_FIRMWARE_FLAGS_POST_BOOT_CONFIGS		0x03
#define BCMTCH_FIRMWARE_FLAGS_MASK					0x03

#define BCMTCH_FIRMWARE_FLAGS_COMBI     0x10

static int bcmtch_firmware_flag = BCMTCH_FIRMWARE_FLAGS_COMBI;

module_param_named(firmware_flag, bcmtch_firmware_flag, int, S_IRUGO);
MODULE_PARM_DESC(firmware_flag, "Firmware flag bit-fields (combi = 0x10  config = 0x01");

static char *bcmtch_firmware = NULL;

module_param_named(firmware, bcmtch_firmware, charp, S_IRUGO);
MODULE_PARM_DESC(firmware, "Filename of firmware to load");

static int bcmtch_firmware_addr = 0x0;

module_param_named(firmware_addr, bcmtch_firmware_addr, int, S_IRUGO);
MODULE_PARM_DESC(firmware_addr, "Address to load firmware");

/*- post boot -*/

#define	BCMTCH_POST_BOOT_RATE_HIGH	160
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
#define MS_TO_JIFFIES(x)			(((x)*HZ)/1000)
#define	BCMTCH_WATCHDOG_NORMAL		5000
#define	BCMTCH_WATCHDOG_POST_BOOT	500

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

/* ------------------------------------------ */
/* - BCM Touch Controller Driver Structures - */
/* ------------------------------------------ */

struct bcmtch_channel_t_ {
	tofe_channel_instance_cfg_t	cfg;
	tofe_channel_header_t		hdr;
	uint16_t					queued;
	uint8_t						active;
	/* intentional pad - may use for i2c tranactions */
	uint8_t						_pad8;
	uint32_t data;
};
#define bcmtch_channel_t struct bcmtch_channel_t_

#if BCMTCH_ROM_CHANNEL
struct bcmtch_rom_channel_t_ {
	tofe_rom_channel_instance_cfg_t	rom_cfg;
	tofe_rom_channel_header_t		rom_hdr;
	uint16_t				queued;
	uint16_t				_pad16;
	uint32_t data;
};
#define bcmtch_rom_channel_t struct bcmtch_rom_channel_t_
#endif

/**
	@ struct bcmtch_response_wait_t_
	@ brief storage of the results from response channel.
*/
struct bcmtch_response_wait_t_ {
	bool		wait;
	uint32_t	resp_data;
};
#define bcmtch_response_wait_t struct bcmtch_response_wait_t_

struct bcmtch_event_t_ {
	uint32_t           event_kind:3;
	uint32_t           _pad:29;
};
#define bcmtch_event_t struct bcmtch_event_t_

enum bcmtch_event_frame_extension_kind_t_ {
	BCMTCH_EVENT_FRAME_EXTENSION_KIND_TIMESTAMP,
	BCMTCH_EVENT_FRAME_EXTENSION_KIND_CHECKSUM,
	BCMTCH_EVENT_FRAME_EXTENSION_KIND_HEARTBEAT,

	BCMTCH_EVENT_FRAME_EXTENSION_KIND_MAX = 7  /* 3 bits */
};
#define bcmtch_event_frame_extension_kind_t \
		enum bcmtch_event_frame_extension_kind_t_

struct bcmtch_event_frame_t_ {
	uint32_t           event_kind:3;
	uint32_t           _pad:1;
	uint32_t           frame_id:12;
	uint32_t           timestamp:16;
};
#define bcmtch_event_frame_t struct bcmtch_event_frame_t_

struct bcmtch_event_frame_extension_t_ {
	uint32_t           event_kind:3;
	uint32_t           frame_kind:3;
	uint32_t           _pad:26;
};
#define bcmtch_event_frame_extension_t \
		struct bcmtch_event_frame_extension_t_

struct bcmtch_event_frame_extension_timestamp_t_ {
	uint32_t           event_kind:3;
	uint32_t           frame_kind:3;
	uint32_t           _pad:2;
	uint32_t           scan_end:8;
	uint32_t           mtc_start:8;
	uint32_t           mtc_end:8;
};
#define bcmtch_event_frame_extension_timestamp_t \
		struct bcmtch_event_frame_extension_timestamp_t_

struct bcmtch_event_frame_extension_checksum_t_ {
	uint32_t           event_kind:3;
	uint32_t           frame_kind:3;
	uint32_t           _pad:10;
	uint32_t           hash:16;
};
#define bcmtch_event_frame_extension_checksum_t \
		struct bcmtch_event_frame_extension_checksum_t_

struct bcmtch_event_frame_extension_heartbeat_t_ {
	uint32_t           event_kind:3;
	uint32_t           frame_kind:3;
	uint32_t           _pad:2;
	uint32_t           timestamp:24; /* 100 us units, free running */
};
#define bcmtch_event_frame_extension_heartbeat_t \
		struct bcmtch_event_frame_extension_heartbeat_t_

enum bcmtch_event_touch_extension_kind_t_ {
	BCMTCH_EVENT_TOUCH_EXTENSION_KIND_DETAIL,
	BCMTCH_EVENT_TOUCH_EXTENSION_KIND_BLOB,
	BCMTCH_EVENT_TOUCH_EXTENSION_KIND_SIZE,
	BCMTCH_EVENT_TOUCH_EXTENSION_KIND_HOVER,
	BCMTCH_EVENT_TOUCH_EXTENSION_KIND_TOOL,

	BCMTCH_EVENT_TOUCH_EXTENSION_KIND_MAX = 7  /* 3 bits */
};
#define bcmtch_event_touch_extension_kind_t \
		enum bcmtch_event_touch_extension_kind_t_

enum bcmtch_event_touch_tool_t_ {
	BCMTCH_EVENT_TOUCH_TOOL_FINGER,
	BCMTCH_EVENT_TOUCH_TOOL_STYLUS,
};
#define bcmtch_event_touch_tool_t enum bcmtch_event_touch_tool_t

struct bcmtch_event_touch_t_ {
	uint32_t           event_kind:3;
	uint32_t           track_tag:5;
	uint32_t           x:12;
	uint32_t           y:12;
};
#define bcmtch_event_touch_t struct bcmtch_event_touch_t_

struct bcmtch_event_touch_end_t_ {
	uint32_t           event_kind:3;
	uint32_t           track_tag:5;
	uint32_t           _pad:24;
};
#define bcmtch_event_touch_end_t struct bcmtch_event_touch_end_t_

struct bcmtch_event_touch_extension_t_ {
	uint32_t           event_kind:3;
	uint32_t           touch_kind:3;
	uint32_t           _pad:26;
};
#define bcmtch_event_touch_extension_t struct bcmtch_event_touch_extension_t_

struct bcmtch_event_touch_extension_detail_t_ {
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
#define bcmtch_event_touch_extension_detail_t \
		struct bcmtch_event_touch_extension_detail_t_

struct bcmtch_event_touch_extension_blob_t_ {
	uint32_t           event_kind:3;
	uint32_t           touch_kind:3;
	uint32_t           area:8;
	uint32_t           total_cap:18;
};
#define bcmtch_event_touch_extension_blob_t \
		struct bcmtch_event_touch_extension_blob_t_

#define BCMTCH_EVENT_TOUCH_EXTENSION_AREA_MAX 255

struct bcmtch_event_touch_extension_size_t_ {
	uint32_t           event_kind:3;
	uint32_t           touch_kind:3;
	uint32_t           _pad:6;
	uint32_t           major_axis:10;
	uint32_t           minor_axis:10;
};
#define bcmtch_event_touch_extension_size_t \
		struct bcmtch_event_touch_extension_size_t_

struct bcmtch_event_touch_extension_hover_t_ {
	uint32_t           event_kind:3;
	uint32_t           touch_kind:3;
	uint32_t           _pad:16;
	uint32_t           height:10;
};
#define bcmtch_event_touch_extension_hover_t \
		struct bcmtch_event_touch_extension_hover_t_

struct bcmtch_event_touch_extension_tool_t_ {
	uint32_t           event_kind:3;
	uint32_t           touch_kind:3;
	uint32_t           _pad:6;
	uint32_t           width_major:10;
	uint32_t           width_minor:10;
};
#define	bcmtch_event_touch_extension_tool_t \
		struct bcmtch_event_touch_extension_tool_t_

enum bcmtch_event_button_kind_t_ {
	BCMTCH_EVENT_BUTTON_KIND_CONTACT,
	BCMTCH_EVENT_BUTTON_KIND_HOVER,

	BCMTCH_EVENT_BUTTON_KIND_MAX = 1
};
#define bcmtch_event_button_kind_t enum bcmtch_event_button_kind_t_

struct bcmtch_event_button_t_ {
	uint32_t           event_kind:3;
	uint32_t           button_kind:1;
	uint32_t           _pad:12;
	uint32_t           status:16;
};
#define bcmtch_event_button_t struct bcmtch_event_button_t_


/* driver structure for a single touch point */
struct bcmtch_touch_t_ {
	uint16_t	x;		/* X Coordinate */
	uint16_t	y;		/* Y Coordinate */
	uint16_t	major_axis;
	uint16_t	minor_axis;
	uint16_t	width_major;
	uint16_t	width_minor;
	uint8_t		pressure;
	int16_t		orientation;

	/* Touch status: Down, Move, Up (Inactive) */
	bcmtch_touch_status_e	status;

	bcmtch_event_kind_e		event;	/* Touch Event Kind */
#if BCMTCH_ROM_CHANNEL
	bcmtch_rom_event_type_e	rom_event;
#endif
};
#define bcmtch_touch_t struct bcmtch_touch_t_

typedef uint16_t (*actual_x_coordinate_t)
	(uint16_t orig_x);
typedef uint16_t (*actual_y_coordinate_t)
	(uint16_t orig_y);
typedef void (*actual_x_y_axis_t)
	(uint16_t orig_x, uint16_t orig_y, bcmtch_touch_t *p_touch);

struct bcmtch_data_t_ {
	/* core 0S elements */
	/* Work queue structure for defining work queue handler */
	struct work_struct work;
	/* Work queue structure for transaction handling */
	struct workqueue_struct *p_workqueue;

	/* Critical Section : Mutexes : */
	struct mutex cs_mutex[BCMTCH_MUTEX_MAX];
	/*  (1) serial bus - I2C / SPI */
	/*  (2) deferred work */

	/* Pointer to allocated memory for input device */
	struct input_dev *p_inputDevice;

	/* I2C 0S elements */

	/* SPM I2C Client structure pointer */
	struct i2c_client *p_i2c_client_spm;

	/* SYS I2C Client structure pointer */
	struct i2c_client *p_i2c_client_sys;

	/* Local copy of platform data structure */
	bcmtch_platform_data_t platform_data;

	/* BCM Touch elements */
	bcmtch_channel_t *p_channels[BCMTCH_CHANNEL_MAX];
#if BCMTCH_ROM_CHANNEL
	bcmtch_rom_channel_t *p_rom_channels[BCMTCH_CHANNEL_MAX];
#endif
	bcmtch_touch_t touch[BCMTCH_MAX_TOUCH];	/* BCMTCH touch structure */
	uint32_t touch_count;

	/* Response storage */
	bcmtch_response_wait_t bcmtch_cmd_response[TOFE_COMMAND_LAST];

	/* BCM Button elements */
	uint16_t button_status;

	/* DMA transfer mode */
	bool has_dma_channel;
	bool host_override;
	uint32_t fwDMABufferSize;
	void *fwDMABuffer;

	/* Interrupts */
	bool irq_pending;
	bool irq_enabled;

	/* Watchdog Timer */
	uint32_t watchdog_expires;
	struct timer_list watchdog;

#if BCMTCH_ROM_CHANNEL
	/* Dual channel mode */
	bool rom_channel;
#endif

	/* Post Boot Downloads */
	uint8_t *post_boot_buffer;
	uint8_t *post_boot_data;
	uint32_t post_boot_addr;
	uint16_t post_boot_left;
	uint16_t post_boot_sections;
	uint16_t post_boot_section;
	uint32_t postboot_cfg_addr;
	uint32_t postboot_cfg_length;

	/* Axis orientation function pointers */
	actual_x_coordinate_t actual_x;
	actual_y_coordinate_t actual_y;
	actual_x_y_axis_t actual_x_y_axis;
};
#define bcmtch_data_t struct bcmtch_data_t_

/* Pointer to BCMTCH Data Structure */
static bcmtch_data_t *bcmtch_data_p = NULL;

/* -------------------------------------------- */
/* - BCM Touch Controller Function Prototypes - */
/* -------------------------------------------- */

/*  DEV Prototypes */
static void	   bcmtch_dev_process(void);
static int32_t bcmtch_dev_reset(uint8_t);
static int32_t bcmtch_dev_suspend(void);
static int32_t bcmtch_dev_resume(void);
static int32_t bcmtch_dev_request_power_mode(uint8_t, tofe_command_e);
static int32_t bcmtch_dev_send_command(tofe_command_e, uint32_t, uint8_t);
static inline bool bcmtch_dev_verify_buffer_header(
		tofe_channel_buffer_header_t*);
static int32_t bcmtch_dev_post_boot_download(int16_t data_rate);
static int32_t bcmtch_dev_post_boot_get_section(void);
static void    bcmtch_dev_watchdog_work(unsigned long int data);
static int32_t bcmtch_dev_watchdog_start(void);
static int32_t bcmtch_dev_watchdog_reset(void);
static int32_t bcmtch_dev_watchdog_stop(void);
static int32_t bcmtch_dev_watchdog_restart(uint32_t expires);

/*  COM Prototypes */
static int32_t  bcmtch_com_init(void);
static int32_t  bcmtch_com_read_spm(uint8_t, uint8_t*);
static int32_t  bcmtch_com_write_spm(uint8_t, uint8_t);
static int32_t  bcmtch_com_read_sys(uint32_t, uint16_t, uint8_t*);
static int32_t  bcmtch_com_write_sys(uint32_t, uint16_t, uint8_t*);
/* COM Helper */
static inline int32_t   bcmtch_com_fast_write_spm(uint8_t, uint8_t*, uint8_t*);
static inline int32_t   bcmtch_com_write_sys32(uint32_t, uint32_t);

/*  OS Prototypes */
static void		*bcmtch_os_mem_alloc(uint32_t);
static void     bcmtch_os_mem_free(void *);
static void     bcmtch_os_reset(void);
static void     bcmtch_os_lock_critical_section(uint8_t);
static void     bcmtch_os_release_critical_section(uint8_t);
static int32_t  bcmtch_os_interrupt_enable(void);
static void     bcmtch_os_interrupt_disable(void);
static void		bcmtch_os_deferred_worker(struct work_struct *work);
static void     bcmtch_os_clear_deferred_worker(void);

#ifdef CONFIG_PM
#ifndef CONFIG_HAS_EARLYSUSPEND
static int      bcmtch_os_suspend(
					struct i2c_client *p_client,
					pm_message_t mesg);
static int      bcmtch_os_resume(struct i2c_client *p_client);
#endif
#endif

/*  OS I2C Prototypes */
static int32_t  bcmtch_os_i2c_probe(
					struct i2c_client*,
					const struct i2c_device_id *);
static int32_t  bcmtch_os_i2c_remove(struct i2c_client *);
static int32_t  bcmtch_os_i2c_read_spm(struct i2c_client*, uint8_t, uint8_t*);
static int32_t  bcmtch_os_i2c_write_spm(struct i2c_client*, uint8_t, uint8_t);
static int32_t  bcmtch_os_i2c_fast_write_spm(
					struct i2c_client*,
					uint8_t,
					uint8_t*,
					uint8_t*);
static int32_t  bcmtch_os_i2c_read_sys(
					struct i2c_client*,
					uint32_t,
					uint16_t,
					uint8_t*);
static int32_t  bcmtch_os_i2c_write_sys(
					struct i2c_client*,
					uint32_t,
					uint16_t,
					uint8_t*);
static int32_t  bcmtch_os_i2c_init_clients(struct i2c_client *);
static void     bcmtch_os_i2c_free_clients(void);
static void     bcmtch_os_sleep_ms(uint32_t);

#if BCMTCH_ROM_CHANNEL
static inline void bcmtch_inline_rom_channel_write_begin(
		tofe_rom_channel_header_t *);
static inline void bcmtch_inline_rom_channel_write_end(
		tofe_rom_channel_header_t *);
static inline uint32_t bcmtch_inline_rom_channel_write(
		tofe_rom_channel_header_t *, void *);
static int32_t bcmtch_dev_read_rom_channel(bcmtch_rom_channel_t *);
static inline void *bcmtch_inline_rom_channel_read(tofe_rom_channel_header_t *);
static inline uint32_t bcmtch_inline_rom_channel_read_end(
		tofe_rom_channel_header_t *);
static int32_t bcmtch_dev_write_rom_command(
		tofe_command_e, uint32_t, uint16_t, uint8_t);
static int32_t bcmtch_dev_init_rom_channel(
		bcmtch_channel_e, tofe_rom_channel_instance_cfg_t *);
static int32_t bcmtch_dev_init_rom_channels(uint32_t, uint8_t *);
static int32_t bcmtch_dev_write_rom_channel(bcmtch_rom_channel_t *);
static int32_t bcmtch_dev_read_rom_channel(bcmtch_rom_channel_t *);
static int32_t bcmtch_dev_process_rom_event_frame(bcmtch_rom_event_frame_t *);
static int32_t bcmtch_dev_process_rom_event_touch(bcmtch_rom_event_touch_t *);
static int32_t bcmtch_dev_process_rom_channel_touch(bcmtch_rom_channel_t *);
#endif

static inline uint16_t bcmtch_get_coordinate(uint16_t value)
{
	return value;
}

static inline uint16_t bcmtch_reverse_x_coordinate(uint16_t x)
{
	return BCMTCH_MAX_X - x;
}

static inline uint16_t bcmtch_reverse_y_coordinate(uint16_t y)
{
	return BCMTCH_MAX_Y - y;
}

static inline void bcmtch_swap_x_y_axis(
	uint16_t orig_x,
	uint16_t orig_y,
	bcmtch_touch_t *p_touch)
{
	p_touch->x = orig_y;
	p_touch->y = orig_x;
}

static inline void bcmtch_set_x_y_axis(
	uint16_t orig_x,
	uint16_t orig_y,
	bcmtch_touch_t *p_touch)
{
	p_touch->x = orig_x;
	p_touch->y = orig_y;
}


/* ------------------------------------------- */
/* - BCM Touch Controller CLI Implementation - */
/* ------------------------------------------- */

static void bcmtch_os_cli_logmask_get(
					uint32_t in_logmask)
{
	bcmtch_response_wait_t *p_resp;
	uint32_t logmask;
	uint32_t ret_val;

	printk(KERN_INFO "BCMTCH: get logmask module_id=0x%04x\n",
			in_logmask);

	logmask = (uint32_t)(in_logmask << 16);
	p_resp = (bcmtch_response_wait_t *)
		&(bcmtch_data_p->bcmtch_cmd_response
				[TOFE_COMMAND_GET_LOG_MASK]);
	p_resp->wait = 1;

#if !BCMTCH_USE_BUS_LOCK
#if BCMTCH_USE_WORK_LOCK
	bcmtch_os_lock_critical_section(BCMTCH_MUTEX_WORK);
#else
#error "To use CLI, either BUS_LOCK or WORK_LOCK must be enabled"
#endif
#endif

	ret_val = bcmtch_dev_send_command(
			TOFE_COMMAND_GET_LOG_MASK,
			logmask,
			TOFE_COMMAND_FLAG_REQUEST_RESULT);
	if (ret_val != BCMTCH_SUCCESS) {
		printk(KERN_ERR "BCMTCH: send_command error [%d] cmd=%x\n",
				ret_val,
				TOFE_COMMAND_GET_LOG_MASK);
	}

#if !BCMTCH_USE_BUS_LOCK
#if BCMTCH_USE_WORK_LOCK
	bcmtch_os_release_critical_section(BCMTCH_MUTEX_WORK);
#endif
#endif
}

static void bcmtch_os_cli_logmask_set(
					uint32_t in_logmask)
{
	bcmtch_response_wait_t *p_resp;

	printk(KERN_INFO "BCMTCH: set logmask mask=0x%08x . . .\n",
			in_logmask);

	p_resp = (bcmtch_response_wait_t *)
		&(bcmtch_data_p->bcmtch_cmd_response
				[TOFE_COMMAND_SET_LOG_MASK]);
	p_resp->wait = 1;

#if !BCMTCH_USE_BUS_LOCK
#if BCMTCH_USE_WORK_LOCK
	bcmtch_os_lock_critical_section(BCMTCH_MUTEX_WORK);
#else
#error "To use CLI, either BUS_LOCK or WORK_LOCK must be enabled"
#endif
#endif

	bcmtch_dev_send_command(
			TOFE_COMMAND_SET_LOG_MASK,
			in_logmask,
			TOFE_COMMAND_FLAG_REQUEST_RESULT);

#if !BCMTCH_USE_BUS_LOCK
#if BCMTCH_USE_WORK_LOCK
	bcmtch_os_release_critical_section(BCMTCH_MUTEX_WORK);
#endif
#endif
}

static void bcmtch_os_cli_spm_poke(
					uint8_t in_addr,
					uint8_t in_data)
{
#if !BCMTCH_USE_BUS_LOCK
#if BCMTCH_USE_WORK_LOCK
	bcmtch_os_lock_critical_section(BCMTCH_MUTEX_WORK);
#else
#error "To use CLI, either BUS_LOCK or WORK_LOCK must be enabled"
#endif
#endif

	bcmtch_com_write_spm(in_addr, in_data);

#if !BCMTCH_USE_BUS_LOCK
#if BCMTCH_USE_WORK_LOCK
	bcmtch_os_release_critical_section(BCMTCH_MUTEX_WORK);
#endif
#endif

	printk(KERN_INFO "BCMTCH: poke spm Reg=%08x data=%08x\n",
		in_addr, in_data);
}

static void bcmtch_os_cli_spm_peek(
					uint8_t in_addr)
{
	uint8_t r8;

#if !BCMTCH_USE_BUS_LOCK
#if BCMTCH_USE_WORK_LOCK
	bcmtch_os_lock_critical_section(BCMTCH_MUTEX_WORK);
#else
#error "To use CLI, either BUS_LOCK or WORK_LOCK must be enabled"
#endif
#endif

	bcmtch_com_read_spm(in_addr, &r8);

#if !BCMTCH_USE_BUS_LOCK
#if BCMTCH_USE_WORK_LOCK
	bcmtch_os_release_critical_section(BCMTCH_MUTEX_WORK);
#endif
#endif

	printk(KERN_INFO "BCMTCH: peek spm reg=0x%02x data=0x%02x\n",
		in_addr, r8);
}

static void bcmtch_os_cli_sys_poke(
					uint32_t in_addr,
					uint32_t in_data)
{
#if !BCMTCH_USE_BUS_LOCK
#if BCMTCH_USE_WORK_LOCK
	bcmtch_os_lock_critical_section(BCMTCH_MUTEX_WORK);
#else
#error "To use CLI, either BUS_LOCK or WORK_LOCK must be enabled"
#endif
#endif

	bcmtch_com_write_sys32(in_addr, in_data);

#if !BCMTCH_USE_BUS_LOCK
#if BCMTCH_USE_WORK_LOCK
	bcmtch_os_release_critical_section(BCMTCH_MUTEX_WORK);
#endif
#endif

	printk(KERN_INFO "BCMTCH: poke sys addr=0x%08x data=0x%08x\n",
			in_addr, in_data);
}

static void bcmtch_os_cli_sys_peek(
					uint32_t in_addr,
					uint32_t in_count)
{
	uint32_t rBuf[8];
	uint32_t addr = in_addr;
	uint32_t count = in_count;

	memset(rBuf, 0, 8 * sizeof(uint32_t));

	printk(KERN_INFO "BCMTCH: peek sys addr=0x%08x len=0x%08x\n",
		in_addr, in_count);

#if !BCMTCH_USE_BUS_LOCK
#if BCMTCH_USE_WORK_LOCK
	bcmtch_os_lock_critical_section(BCMTCH_MUTEX_WORK);
#else
#error "To use CLI, either BUS_LOCK or WORK_LOCK must be enabled"
#endif
#endif

	while (count) {
		bcmtch_com_read_sys(addr, 8 * sizeof(uint32_t),
				    (uint8_t *)rBuf);

		printk(KERN_INFO
			"BCMTCH:0x%08x: 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
			addr,
			rBuf[0], rBuf[1], rBuf[2], rBuf[3],
			rBuf[4], rBuf[5], rBuf[6], rBuf[7]);

		count =
		    (count > (8 * sizeof(uint32_t))) ?
			(count - (8 * sizeof(uint32_t))) :
			0;
		addr += (8 * sizeof(uint32_t));
	}

#if !BCMTCH_USE_BUS_LOCK
#if BCMTCH_USE_WORK_LOCK
	bcmtch_os_release_critical_section(BCMTCH_MUTEX_WORK);
#endif
#endif

}

static ssize_t bcmtch_os_cli(
					struct device *dev,
					struct device_attribute *devattr,
					const char *buf,
					size_t count)
{
	uint32_t in_addr = 0;
	uint32_t in_value_count = 0;

	if (sscanf(buf, "poke sys %x %x", &in_addr, &in_value_count)) {
		bcmtch_os_cli_sys_poke(in_addr, in_value_count);
	} else if (sscanf(buf, "peek sys %x %x", &in_addr, &in_value_count)) {
		bcmtch_os_cli_sys_peek(in_addr, in_value_count);
	} else if (sscanf(buf, "poke spm %x %x", &in_addr, &in_value_count)) {
		bcmtch_os_cli_spm_poke(in_addr & 0xff, in_value_count & 0xff);
	} else if (sscanf(buf, "peek spm %x", &in_addr)) {
		bcmtch_os_cli_spm_peek(in_addr & 0xff);
	} else if (sscanf(buf, "debug_flag %x", &in_value_count)) {
		bcmtch_debug_flag = in_value_count;
		printk(KERN_INFO "BCMTCH: bcmtch_debug_flag=0x%08x\n",
			bcmtch_debug_flag);
	} else if (sscanf(buf, "set logmask %x", &in_value_count)) {
		bcmtch_os_cli_logmask_set(in_value_count);
	} else if (sscanf(buf, "get logmask %x", &in_value_count)) {
		bcmtch_os_cli_logmask_get(in_value_count);
	}
#if defined(CONFIG_PM)
	else if (sscanf(buf, "suspend %x", &in_value_count)) {
		printk(KERN_INFO
				"BCMTCH: cli --> suspend(%d)\n",
				in_value_count);
		bcmtch_dev_suspend();
	} else if (sscanf(buf, "resume %x", &in_value_count)) {
		printk(KERN_INFO
				"BCMTCH: cli --> resume(%d)\n",
				in_value_count);
		bcmtch_dev_resume();
	}
#endif
	else {
		printk(KERN_INFO "Usage:");
		printk(KERN_INFO "poke sys 0x<addr> 0x<data>");
		printk(KERN_INFO "peek sys 0x<addr> 0x<len>");
		printk(KERN_INFO "poke spm 0x<reg> 0x<data>");
		printk(KERN_INFO "peek spm 0x<reg>");
		printk(KERN_INFO "debug_flag 0x<flags - bitmap>");
		printk(KERN_INFO "set logmask 0x<module>");
		printk(KERN_INFO "get logmask 0x<mask - bitmap>");
		printk(KERN_INFO "record 0x<scale> 0x<count>");
		printk(KERN_INFO "suspend 0x1");
		printk(KERN_INFO "resume 0x1");
	}

	return count;
}

static struct device_attribute bcmtch_cli_attr =
		__ATTR(cli, 0664, NULL, bcmtch_os_cli);

/* ------------------------------------------- */
/* - BCM Touch Controller Internal Functions - */
/* ------------------------------------------- */

unsigned bcmtch_channel_num_queued(
		tofe_channel_header_t *channel)
{
	return (unsigned)channel->
		buffer[0]->header.entry_count;
}


/*
    Note: Internal use only function.
*/
static inline char *
_bcmtch_inline_channel_entry(
			tofe_channel_header_t *channel,
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
		tofe_channel_header_t *channel,
		iterator_t entry_index)
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
bcmtch_inline_channel_is_empty(tofe_channel_header_t *channel)
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
			tofe_channel_header_t *channel,
			uint16_t index)
{
	size_t byte_index;
	tofe_channel_buffer_t *buff = channel->buffer[0];

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
tofe_channel_write_begin(tofe_channel_header_t *channel)
{
	tofe_channel_buffer_header_t *buff =
		&channel->buffer[0]->header;
	if (channel->flags & TOFE_CHANNEL_FLAG_INBOUND)
		buff->entry_count = 0;
}

static inline uint32_t
tofe_channel_write(tofe_channel_header_t *channel, void *entry)
{
	tofe_channel_buffer_header_t *buff =
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

	return BCMTCH_SUCCESS;
}


/* ------------------------------------------- */
/* - BCM Touch Controller DEV Functions - */
/* ------------------------------------------- */

static int32_t bcmtch_dev_alloc(void)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	bcmtch_data_p = bcmtch_os_mem_alloc(sizeof(bcmtch_data_t));
	if (bcmtch_data_p == NULL) {
		printk(KERN_ERR "%s: failed to alloc mem.\n", __func__);
		ret_val = -ENOMEM;
	}
	return ret_val;
}

static void bcmtch_dev_free(void)
{
	bcmtch_os_mem_free(bcmtch_data_p);
}

static int32_t bcmtch_dev_init_clocks(void)
{
	int32_t ret_val = -EAGAIN;
	uint32_t val32;
	uint8_t locked;
	uint8_t waitFLL = 5;

	/* setup LPLFO - read OTP and set from value */
	bcmtch_com_read_sys(
			BCMTCH_ADDR_SPM_LPLFO_CTRL_RO,
			4,
			(uint8_t *)&val32);
	val32 &= 0xF0000000;
	val32 >>= 28;
	bcmtch_com_write_spm(BCMTCH_SPM_REG_LPLFO_CTRL, (uint8_t)val32 | 0x10);

    /* setup FLL */
	bcmtch_com_write_sys32(BCMTCH_ADDR_COMMON_FLL_CTRL0, 0xe0000002);
	bcmtch_com_write_sys32(BCMTCH_ADDR_COMMON_FLL_LPF_CTRL2, 0x01001007);
	bcmtch_com_write_sys32(BCMTCH_ADDR_COMMON_FLL_CTRL0, 0x00000001);
	bcmtch_os_sleep_ms(1);
	bcmtch_com_write_sys32(BCMTCH_ADDR_COMMON_FLL_CTRL0, 0x00000002);

    /* Set the clock dividers for SYS bus speeds*/
	bcmtch_com_write_sys32(BCMTCH_ADDR_COMMON_SYS_HCLK_CTRL, 0xF01);

    /* Enable clocks */
	bcmtch_com_write_sys32(
		BCMTCH_ADDR_COMMON_CLOCK_ENABLE,
		BCMTCH_IF_I2C_COMMON_CLOCK);

	/* wait for FLL to lock */
	do {
		bcmtch_com_read_spm(BCMTCH_SPM_REG_FLL_STATUS, &locked);
		if (locked) {
			/* switch to FLL */
			bcmtch_com_write_sys32(
				BCMTCH_ADDR_COMMON_CLOCK_ENABLE,
				(BCMTCH_IF_I2C_COMMON_CLOCK |
				 BCMTCH_COMMON_CLOCK_USE_FLL));

			ret_val = BCMTCH_SUCCESS;
			break;
		}
	} while (waitFLL--);

	return ret_val;
}

static int32_t bcmtch_dev_init_memory(void)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	uint32_t memMap;

	memMap = (bcmtch_boot_flag & BCMTCH_BOOT_FLAG_RAM_BOOT) ?
	    BCMTCH_MEM_RAM_BOOT : BCMTCH_MEM_ROM_BOOT;

#if BCMTCH_ROM_CHANNEL
	bcmtch_data_p->rom_channel =
		(bcmtch_boot_flag & BCMTCH_BOOT_FLAG_RAM_BOOT) ? false : true;
#endif
	ret_val = bcmtch_com_write_sys32(BCMTCH_MEM_REMAP_ADDR, memMap);

	return ret_val;
}

static inline void
tofe_channel_header_init(
		bcmtch_channel_t *p_channel,
		tofe_channel_instance_cfg_t *p_chan_cfg)
{
	tofe_channel_header_t *hdr = &p_channel->hdr;
	tofe_channel_buffer_header_t *buff;
	hdr->entry_num = p_chan_cfg->entry_num;
	hdr->entry_size = p_chan_cfg->entry_size;
	hdr->trig_level = p_chan_cfg->trig_level;
	hdr->flags = p_chan_cfg->flags;
	hdr->buffer_num = p_chan_cfg->buffer_num;
	hdr->buffer_idx = 0;
	hdr->seq_count = 0;
	hdr->buffer[0] = (tofe_channel_buffer_t *)&p_channel->data;
	hdr->buffer[1] = (tofe_channel_buffer_t *)&p_channel->data;
	if (hdr->flags & TOFE_CHANNEL_FLAG_FWDMA_ENABLE) {
		bcmtch_data_p->has_dma_channel = true;
		buff = &hdr->buffer[1]->header;
		buff->channel_id = hdr->channel_id;
		buff->entry_count = 0;
		buff->entry_size = hdr->entry_size;
		bcmtch_data_p->fwDMABufferSize +=
			sizeof(tofe_channel_buffer_header_t)
			+ (hdr->entry_num * hdr->entry_size);
	}
}

static int32_t bcmtch_dev_init_channel(
		bcmtch_channel_e chan_id,
		tofe_channel_instance_cfg_t *p_chan_cfg,
		uint8_t active)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	uint32_t channel_size;
	bcmtch_channel_t *p_channel;

	if (active) {
		channel_size =
				/* channel data size   */
				sizeof(tofe_channel_buffer_header_t)
				+ (p_chan_cfg->flags &
					TOFE_CHANNEL_FLAG_FWDMA_ENABLE ? 0 :
					p_chan_cfg->entry_num
					* p_chan_cfg->entry_size)
				/* channel header size */
				+ sizeof(tofe_channel_header_t)
				/* channel config size */
				+ sizeof(tofe_channel_instance_cfg_t)
				/* sizes for added elements: queued, pad */
				+ (sizeof(uint16_t) * 2);
	} else {
		channel_size =
				/* channel header size */
				sizeof(tofe_channel_header_t)
				/* channel config size */
				+ sizeof(tofe_channel_instance_cfg_t)
				/* sizes for added elements: queued, pad */
				+ (sizeof(uint16_t) * 2);
	}

	p_channel =
		(bcmtch_channel_t *)bcmtch_os_mem_alloc(channel_size);

	if (p_channel) {
		p_channel->cfg = *p_chan_cfg;

		/* Initialize Header */
		p_channel->hdr.channel_id = (uint8_t)chan_id;
		p_channel->active = active;
		tofe_channel_header_init(p_channel, p_chan_cfg);

		bcmtch_data_p->p_channels[chan_id] = p_channel;
	} else
		ret_val = -ENOMEM;

	return ret_val;
}

static void bcmtch_dev_free_channels(void)
{
	uint32_t chan = 0;
	if (bcmtch_data_p->fwDMABuffer)
		bcmtch_os_mem_free(bcmtch_data_p->fwDMABuffer);
	while (chan < BCMTCH_CHANNEL_MAX)
		bcmtch_os_mem_free(bcmtch_data_p->p_channels[chan++]);
}

static int32_t bcmtch_dev_init_channels(uint32_t mem_addr, uint8_t *mem_data)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	void *p_buffer = NULL;
	uint32_t *p_cfg = NULL;
	tofe_channel_instance_cfg_t *p_chan_cfg = NULL;

	/* find channel configs */
	p_cfg = (uint32_t *)(mem_data + TOFE_SIGNATURE_SIZE);
	p_chan_cfg =
		(tofe_channel_instance_cfg_t *)
		((uint32_t)mem_data + p_cfg[TOFE_TOC_INDEX_CHANNEL] - mem_addr);

	/* check if processing channel(s) - add */
	ret_val = bcmtch_dev_init_channel(
			BCMTCH_CHANNEL_TOUCH,
			&p_chan_cfg[TOFE_CHANNEL_ID_TOUCH],
			bcmtch_channel_flag & BCMTCH_CHANNEL_FLAG_USE_TOUCH);
	if (ret_val) {
		printk(KERN_ERR
		       "%s: [%d] Touch Event Channel not initialized!\n",
		       __func__, ret_val);
	}

	/* Command & response channels */
	if (!ret_val) {
		ret_val = bcmtch_dev_init_channel(
				BCMTCH_CHANNEL_COMMAND,
				&p_chan_cfg[TOFE_CHANNEL_ID_COMMAND],
				bcmtch_channel_flag &
				BCMTCH_CHANNEL_FLAG_USE_CMD_RESP);
		ret_val |= bcmtch_dev_init_channel(
				BCMTCH_CHANNEL_RESPONSE,
				&p_chan_cfg[TOFE_CHANNEL_ID_RESPONSE],
				bcmtch_channel_flag &
				BCMTCH_CHANNEL_FLAG_USE_CMD_RESP);
	}

	/* Log channel */
	if (!ret_val) {
		ret_val = bcmtch_dev_init_channel(
				BCMTCH_CHANNEL_LOG,
				&p_chan_cfg[TOFE_CHANNEL_ID_LOG],
				bcmtch_channel_flag &
				BCMTCH_CHANNEL_FLAG_USE_LOG);
		if (ret_val)
			printk(KERN_ERR
					"%s: [%d] Log Channel initialization failed!\n",
					__func__, ret_val);
	}

	/* Initialize DMA buffer if there is any DMA mode channel */
	if (!ret_val &&
			bcmtch_data_p->has_dma_channel) {
		if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_CHANNELS)
			printk(KERN_INFO
				"BCMTOUCH: %s() dma buffer size=%d\n",
				__func__,
				bcmtch_data_p->fwDMABufferSize);
		p_buffer = bcmtch_os_mem_alloc(bcmtch_data_p->fwDMABufferSize);
		if (p_buffer) {
			bcmtch_data_p->fwDMABuffer = p_buffer;
		} else {
			printk(KERN_ERR
				"%s: [%d] DMA buffer allocation failed!\n",
				__func__, ret_val);
			bcmtch_data_p->fwDMABuffer = NULL;
			ret_val = -ENOMEM;
		}
	}

	return ret_val;
}

static int32_t bcmtch_dev_write_channel(bcmtch_channel_t *chan)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	int16_t writeSize;
	uint32_t sys_addr;

	/* read channel header and data all-at-once : need combined size */
	writeSize = sizeof(tofe_channel_buffer_header_t)
			+ (chan->cfg.entry_num * chan->cfg.entry_size);

	sys_addr = (uint32_t)chan->cfg.channel_data;

	/* write channel header & channel data buffer */
	ret_val = bcmtch_com_write_sys(
				sys_addr,
				writeSize,
				(uint8_t *)chan->hdr.buffer[0]);
	if (ret_val) {
		printk(KERN_ERR
				"BCMTOUCH: %s() write_sys err addr=0x%08x, rv=%d\n",
				__func__,
				sys_addr,
				ret_val);
	}
	return ret_val;
}

static int32_t bcmtch_dev_sync_channel(bcmtch_channel_t *chan)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	uint16_t readSize;
	uint32_t sys_addr;
	tofe_channel_header_t sync_hdr;

	/* Read channel header from firmware */
	readSize = sizeof(tofe_channel_header_t);
	sys_addr = (uint32_t)chan->cfg.channel_header;
	ret_val = bcmtch_com_read_sys(
				(uint32_t)chan->cfg.channel_header,
				readSize,
				(uint8_t *)&sync_hdr);
	if (ret_val) {
		printk(KERN_ERR
				"BCMTOUCH: %s() read hdr err addr=0x%08x, rv=%d\n",
				__func__,
				sys_addr,
				ret_val);
		return ret_val;
	}


	/* Read channel */
	readSize = sizeof(tofe_channel_buffer_header_t)
			+ (chan->cfg.entry_num * chan->cfg.entry_size);
	sys_addr = (uint32_t)(sync_hdr.buffer_idx > 0 ?
				(char *)chan->cfg.channel_data :
				(char *)chan->cfg.channel_data
					+ chan->cfg.offset);
	ret_val = bcmtch_com_read_sys(
				sys_addr,
				readSize,
				(uint8_t *)chan->hdr.buffer[0]);
	if (ret_val) {
		printk(KERN_ERR
				"BCMTOUCH: %s() read buffer err addr=0x%08x, rv=%d\n",
				__func__,
				sys_addr,
				ret_val);
		return ret_val;
	}
	chan->queued = bcmtch_channel_num_queued(&chan->hdr);

	/* Sync the channel header */
	chan->hdr.buffer_idx = sync_hdr.buffer_idx;
	chan->hdr.seq_count = sync_hdr.seq_count + 1;

	return ret_val;
}

static int32_t bcmtch_dev_read_channel(
					bcmtch_channel_t *chan)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	uint8_t buffer_idx = chan->hdr.buffer_idx;
	uint8_t seq_count = chan->hdr.seq_count;
	tofe_channel_buffer_header_t *buff =
				&chan->hdr.buffer[1]->header;
	uint16_t readSize;
	uint32_t sys_addr;

	/* channel buffer size: buffer header + entries */
	readSize = sizeof(tofe_channel_buffer_header_t)
			+ (chan->cfg.entry_num * chan->cfg.entry_size);

	sys_addr = (uint32_t)(buffer_idx == 0 ?
				(char *)chan->cfg.channel_data :
				(char *)chan->cfg.channel_data
					+ chan->cfg.offset);

	/* read channel header & channel data buffer */
	ret_val = bcmtch_com_read_sys(
				sys_addr,
				readSize,
				(uint8_t *)chan->hdr.buffer[1]);
	if (ret_val) {
		printk(KERN_ERR
				"BCMTOUCH: %s() read_sys err addr=0x%08x, rv=%d\n",
				__func__,
				sys_addr,
				ret_val);
		return ret_val;
	}

	/* check if data corrupted */
	if (!bcmtch_dev_verify_buffer_header(buff)) {
		if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_CHANNELS)
			printk(KERN_ERR
					"BCMTCH: %s() ch=%d buffer data corrupted!\n",
					__func__,
					chan->hdr.channel_id);
		return -EIO;
	}

	if (buff->flags & TOFE_CHANNEL_FLAG_STATUS_OVERFLOW) {
		if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_CHANNELS)
			printk(KERN_ERR
					"BCMTCH: %s() ch=%d channel overflow\n",
					__func__,
					chan->hdr.channel_id);
	}

	if (buff->seq_number != seq_count) {
		if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_CHANNELS)
			printk(KERN_ERR
				"BCMTOUCH: %s() host ch=%d seq=%d, fw seq=%d NOT Matched!!\n",
				__func__,
				buff->channel_id,
				seq_count,
				buff->seq_number);

		/* sync channel */
		ret_val = bcmtch_dev_sync_channel(chan);

		return ret_val;
	}

	/* Update channel header */
	chan->hdr.seq_count++;
	chan->hdr.buffer_idx = (buffer_idx > 0 ? 0 : 1);

	/* get count */
	chan->queued = bcmtch_channel_num_queued(&chan->hdr);

	return ret_val;
}

static uint32_t bcmtch_dev_read_dma_buffer(void)
{
	uint32_t read_size = 0;
	uint32_t dma_buff_size = bcmtch_data_p->fwDMABufferSize;
	uint8_t dmaReg = BCMTCH_SPM_REG_DMA_RFIFO;
	uint8_t *dma_buff = (uint8_t *)bcmtch_data_p->fwDMABuffer;
	struct i2c_client *p_i2c = bcmtch_data_p->p_i2c_client_sys;
	tofe_dmac_header_t *p_dmac;

	/* setup I2C messages for DMA read request transaction */
	struct i2c_msg dma_request[2] = {
		/* write the RFIFO address */
		{.addr = p_i2c->addr, .flags = 0, .len = 1, .buf = &dmaReg},
		/* read RFIFO data */
		{.addr = p_i2c->addr, .flags = I2C_M_RD,
				.len = (uint32_t)sizeof(tofe_dmac_header_t),
				.buf = dma_buff}
	};

	/* Set I2C master to read from RFIFO */
	if (dma_buff_size && dma_buff) {
		/* 1st I2C read dmac header */
		if (i2c_transfer(p_i2c->adapter, dma_request, 2) != 2) {
			printk(KERN_ERR
					"%s: I2C transfer error.\n",
					__func__);
			return 0;
		} else {
			p_dmac = (tofe_dmac_header_t *)dma_buff;
			read_size = (uint32_t)p_dmac->size;
			if (bcmtch_debug_flag &
					BCMTCH_DEBUG_FLAG_CHANNELS) {
				printk(KERN_INFO
						"%s: DMA buffer read size=%d min_size=%d.\n",
						__func__,
						read_size,
						p_dmac->min_size);
			}

			if (read_size > dma_buff_size) {
				printk(KERN_ERR
						"%s: DMA read overflow buffer [%d].\n",
						__func__,
						dma_buff_size);
				return 0;
			} else if (read_size <
					sizeof(tofe_dmac_header_t))
				return 0;

			/* 2nd I2C read entire DMA buffer */
			dma_request[1].len = read_size
						- sizeof(tofe_dmac_header_t);
			dma_request[1].buf = (uint8_t *)dma_buff
						+ sizeof(tofe_dmac_header_t);
			if (i2c_transfer(p_i2c->adapter, dma_request, 2) != 2) {
				printk(KERN_ERR
						"%s: I2C transfer error.\n",
						__func__);
				return 0;
			}
			bcmtch_os_sleep_ms(2);
		}
	} else {
		printk(KERN_ERR
				"%s: DMA buffer/size is NULL.\n",
				__func__);
	}
	return read_size;
}

static inline bool bcmtch_dev_verify_buffer_header(
		tofe_channel_buffer_header_t *buff)
{
	uint8_t channel;
	bcmtch_channel_t *p_chan;
	bool ret_val = true;

	p_chan = NULL;
	channel = (uint8_t)buff->channel_id;
	if (channel >= BCMTCH_CHANNEL_MAX)
		ret_val = false;
	else {
		p_chan = bcmtch_data_p->p_channels[channel];
		if (!p_chan ||
			(buff->entry_size != p_chan->cfg.entry_size))
			ret_val = false;
	}

	if ((bcmtch_debug_flag &
		BCMTCH_DEBUG_FLAG_CHANNELS) &&
		(ret_val == false)) {
		printk(KERN_INFO
				"%s: ERROR : id=%d entry_size=%d [%d]\n",
				__func__,
				buff->channel_id,
				buff->entry_size,
				(p_chan) ? p_chan->cfg.entry_size : -1);
	}

	return true;
}

static int32_t bcmtch_dev_read_dma_channels(void)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	uint32_t read_size;
	uint32_t read_head;
	uint32_t offset;
	uint32_t channel;

	uint8_t *p_dma = (uint8_t *)bcmtch_data_p->fwDMABuffer;
	tofe_channel_buffer_header_t *buff;
	tofe_channel_header_t *hdr;

	/* Read DMA buffer via I2C */
	read_size = bcmtch_dev_read_dma_buffer();
	if (bcmtch_debug_flag &
		BCMTCH_DEBUG_FLAG_CHANNELS) {
		printk(KERN_INFO
				"%s: read DMA buffer %d bytes.\n",
				__func__,
				read_size);
	}

	if (read_size != bcmtch_data_p->fwDMABufferSize) {
		printk(KERN_ERR
				"%s: Invalid DMA data read size %d.\n",
				__func__,
				read_size);
		return -EIO;
	}

	/* Parse DMA buffer for channels */
	read_head = 0;
	while (read_head < read_size) {
		buff = (tofe_channel_buffer_header_t *)p_dma;
		if (!bcmtch_dev_verify_buffer_header(buff)) {
			printk(KERN_ERR
					"%s: corrupted buffer header in DMA channel!\n",
					__func__);
			return -EIO;
		}

		channel = buff->channel_id;
		if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_CHANNELS) {
			printk(KERN_INFO
					"%s: parsing channel [%d] min_size=%d\n",
					__func__,
					channel,
					buff->dmac.min_size);
		}

		hdr = &bcmtch_data_p->p_channels[channel]->hdr;
		if (hdr->flags & TOFE_CHANNEL_FLAG_FWDMA_ENABLE)
			hdr->buffer[0] = (tofe_channel_buffer_t *)p_dma;

		offset = (uint32_t)(buff->dmac.min_size ? buff->dmac.min_size :
				(buff->entry_size * buff->entry_count)
				+ sizeof(tofe_channel_buffer_header_t));
		read_head += offset;
		p_dma += offset;
	}

	return ret_val;
}

static int32_t bcmtch_dev_read_channels(void)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	uint32_t channel = 0;
	uint32_t channels_read = 0;

	while (channel < BCMTCH_CHANNEL_MAX) {
#if BCMTCH_ROM_CHANNEL
		if (bcmtch_data_p->rom_channel) {
			if (bcmtch_data_p->p_rom_channels[channel] &&
				!(bcmtch_data_p->
					p_rom_channels[channel]->rom_cfg.flags
					& TOFE_CHANNEL_FLAG_INBOUND)) {

				ret_val = bcmtch_dev_read_rom_channel(
						bcmtch_data_p->
						p_rom_channels[channel]);
				channels_read++;
			}
		} else {
#endif
			if (bcmtch_data_p->p_channels[channel]->active &&
				!(bcmtch_data_p->
					p_channels[channel]->cfg.flags &
					(TOFE_CHANNEL_FLAG_INBOUND
					 | TOFE_CHANNEL_FLAG_FWDMA_ENABLE))) {
				ret_val = bcmtch_dev_read_channel(
					bcmtch_data_p->p_channels[channel]);
				channels_read++;
			}
#if BCMTCH_ROM_CHANNEL
		}
#endif
		channel++;
	}

	return ret_val;
}

static int32_t bcmtch_dev_process_event_frame(
					bcmtch_event_frame_t *p_frame_event)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_FRAME)
		printk(KERN_INFO
			"BCMTCH: FR: T=%d ID=%d TS=%d\n",
			bcmtch_data_p->touch_count,
			p_frame_event->frame_id,
			p_frame_event->timestamp);

	return ret_val;
}

static int32_t bcmtch_dev_process_event_frame_extension(
			bcmtch_event_frame_extension_t *p_frame_event_extension)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	bcmtch_event_frame_extension_timestamp_t *timestamp;
	bcmtch_event_frame_extension_checksum_t  *checksum;

	switch (p_frame_event_extension->frame_kind) {
	case BCMTCH_EVENT_FRAME_EXTENSION_KIND_TIMESTAMP:
		{
			timestamp = (bcmtch_event_frame_extension_timestamp_t *)
				p_frame_event_extension;

			if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_FRAME_EXT)
				printk(KERN_INFO "Time offsets. %d %d %d",
					timestamp->scan_end,
					timestamp->mtc_start,
					timestamp->mtc_end);
		}
		break;
	case BCMTCH_EVENT_FRAME_EXTENSION_KIND_CHECKSUM:
		{
			checksum = (bcmtch_event_frame_extension_checksum_t *)
				p_frame_event_extension;

			if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_FRAME_EXT)
				printk(KERN_INFO "ERROR: Checksum not supported.  %#x",
					checksum->hash);
		}
		break;
	case BCMTCH_EVENT_FRAME_EXTENSION_KIND_HEARTBEAT:
		{
			if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_FRAME_EXT)
				printk(KERN_INFO "ERROR: Heartbeat not supported.");
		}
		break;
	default:
		if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_FRAME_EXT)
			printk(KERN_INFO "Invalid frame extension. %d",
				p_frame_event_extension->frame_kind);
		break;
	}

	return ret_val;
}


static int32_t bcmtch_dev_sync_event_frame(void)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	struct input_dev *pInputDevice = bcmtch_data_p->p_inputDevice;
	bcmtch_touch_t *pTouch;
	uint32_t numTouches = 0;
	uint32_t touchIndex = 0;

	for (touchIndex = 0; touchIndex < BCMTCH_MAX_TOUCH; touchIndex++) {
		pTouch = (bcmtch_touch_t *) &bcmtch_data_p->touch[touchIndex];

#if BCMTCH_USE_PROTOCOL_B
		input_mt_slot(pInputDevice, touchIndex);
		input_mt_report_slot_state(
			pInputDevice,
			MT_TOOL_FINGER,
			(pTouch->status > BCMTCH_TOUCH_STATUS_UP));
#endif
		if (pTouch->status > BCMTCH_TOUCH_STATUS_UP) {
			/* Count both of STATUS_MOVE and STATUS_MOVING */
			numTouches++;

			if (pTouch->status > BCMTCH_TOUCH_STATUS_MOVE) {

				input_report_abs(
						pInputDevice,
						ABS_MT_POSITION_X,
						pTouch->x);

				input_report_abs(
						pInputDevice,
						ABS_MT_POSITION_Y,
						pTouch->y);

				if (bcmtch_event_flag &
						BCMTCH_EVENT_FLAG_PRESSURE) {
					input_report_abs(
							pInputDevice,
							ABS_MT_PRESSURE,
							pTouch->pressure);
				}

				if (bcmtch_event_flag &
						BCMTCH_EVENT_FLAG_TOUCH_SIZE) {
					input_report_abs(
							pInputDevice,
							ABS_MT_TOUCH_MAJOR,
							pTouch->major_axis);

					input_report_abs(
							pInputDevice,
							ABS_MT_TOUCH_MINOR,
							pTouch->minor_axis);
				}

				if (bcmtch_event_flag &
						BCMTCH_EVENT_FLAG_ORIENTATION) {
					input_report_abs(
							pInputDevice,
							ABS_MT_ORIENTATION,
							pTouch->orientation);
				}

				if (bcmtch_event_flag &
						BCMTCH_EVENT_FLAG_TOOL_SIZE) {
					input_report_abs(
							pInputDevice,
							ABS_MT_WIDTH_MAJOR,
							pTouch->width_major);

					input_report_abs(
							pInputDevice,
							ABS_MT_WIDTH_MINOR,
							pTouch->width_minor);
				}
#if !BCMTCH_USE_PROTOCOL_B
				input_report_abs(
						pInputDevice,
						ABS_MT_TRACKING_ID,
						touchIndex);

				input_mt_sync(pInputDevice);
#endif
				/* reset the status from MOVING to MOVE. */
				pTouch->status = BCMTCH_TOUCH_STATUS_MOVE;
			}
		}
	}

	input_report_key(pInputDevice, BTN_TOUCH, (numTouches > 0));
	input_sync(pInputDevice);

	/* remember */
	bcmtch_data_p->touch_count = numTouches;

	return ret_val;
}

static void
bcmtch_dev_process_event_touch_extension(
		bcmtch_event_touch_extension_t *extension,
		uint8_t track_id)
{
	char *tool_str;
	bcmtch_event_touch_extension_detail_t *detail;
	bcmtch_event_touch_extension_blob_t *blob;
	bcmtch_event_touch_extension_size_t *size;
	bcmtch_event_touch_extension_tool_t *tool;
	bcmtch_touch_t *pTouch = (bcmtch_touch_t *)
						&bcmtch_data_p->touch[track_id];

	switch (extension->touch_kind) {
	case BCMTCH_EVENT_TOUCH_EXTENSION_KIND_DETAIL:
		detail =
			(bcmtch_event_touch_extension_detail_t *)
			extension;

		if (detail->tool == BCMTCH_EVENT_TOUCH_TOOL_FINGER)
			tool_str = "finger";
		else
			tool_str = "stylus";

		if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_TOUCH_EXT)
			printk(KERN_INFO "C%d:S%d:H%d %s Pres=%d Ornt=%#x",
				detail->confident,
				detail->suppressed,
				detail->hover,
				tool_str,
				detail->pressure,
				detail->orientation);
		/**
		 * ABS_MT_TOOL_TYPE
		 * - MT_TOOL_FINGER
		 * - MT_TOOL_PEN
		 **/
		pTouch->pressure = detail->pressure;

		/* get orientation
		 * - handle int12 to int16 conversion
		 */
		pTouch->orientation = detail->orientation;
		if (pTouch->orientation & (1<<11))
			pTouch->orientation -= 1<<12;
		break;

	case BCMTCH_EVENT_TOUCH_EXTENSION_KIND_BLOB:
		blob =
			(bcmtch_event_touch_extension_blob_t *)
			extension;
		if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_TOUCH_EXT)
			printk(KERN_INFO "\tArea=%d TCap=%d",
				blob->area, blob->total_cap);
		/**
		 * ABS_MT_BLOB_ID
		 */
		break;

	case BCMTCH_EVENT_TOUCH_EXTENSION_KIND_SIZE:
		size =
			(bcmtch_event_touch_extension_size_t *)
			extension;
		if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_TOUCH_EXT)
			printk(KERN_INFO "Track %d:\tMajor=%d Minor=%d",
					track_id,
					size->major_axis,
					size->minor_axis);
		/**
		 * ABS_MT_MAJOR/MINOR_AXIS
		 */
		pTouch->major_axis = size->major_axis;
		pTouch->minor_axis = size->minor_axis;
		break;

	case BCMTCH_EVENT_TOUCH_EXTENSION_KIND_HOVER:
		if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_TOUCH_EXT)
			printk(KERN_INFO "ERROR: Hover not supported.");
		break;

	case BCMTCH_EVENT_TOUCH_EXTENSION_KIND_TOOL:
		tool =
			(bcmtch_event_touch_extension_tool_t *)
			extension;
		if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_TOUCH_EXT)
			printk(KERN_INFO "Track %d:\tMajor=%d Minor=%d",
					track_id,
					tool->width_major,
					tool->width_minor);
		/**
		 * ABS_MT_MAJOR/MINOR_AXIS
		 */
		pTouch->width_major = tool->width_major;
		pTouch->width_minor = tool->width_minor;
		break;
	default:
		if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_TOUCH_EXT)
			printk(KERN_INFO "Invalid touch extension. %d",
					extension->touch_kind);
		break;
	}
}


static int32_t bcmtch_dev_process_event_touch(
					bcmtch_event_touch_t *p_touch_event)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	bcmtch_touch_t *p_touch;
	bcmtch_event_kind_e kind;

	if (p_touch_event->track_tag < BCMTCH_MAX_TOUCH) {
		p_touch =
		    (bcmtch_touch_t *)
			&bcmtch_data_p->touch[p_touch_event->track_tag];

		p_touch_event->x =
			bcmtch_data_p->actual_x(p_touch_event->x);

		p_touch_event->y =
			bcmtch_data_p->actual_y(p_touch_event->y);

		bcmtch_data_p->actual_x_y_axis(p_touch_event->x,
			p_touch_event->y, p_touch);

		kind = (bcmtch_event_kind_e)p_touch_event->event_kind;

		switch (kind) {
		case BCMTCH_EVENT_KIND_TOUCH:
			p_touch->event = kind;
			p_touch->status = BCMTCH_TOUCH_STATUS_MOVING;

			if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_MOVE)
				printk(KERN_INFO
					"BCMTCH: MV: T%d: (%04x , %04x)\n",
					p_touch_event->track_tag,
					p_touch->x,
					p_touch->y);
			break;
		case BCMTCH_EVENT_KIND_TOUCH_END:
			p_touch->event = kind;
			p_touch->status = BCMTCH_TOUCH_STATUS_UP;

			if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_UP)
				printk(KERN_INFO
					"BCMTCH: UP: T%d: (%04x , %04x)\n",
					p_touch_event->track_tag,
					p_touch->x,
					p_touch->y);
			break;
		default:
			printk(KERN_ERR "%s: Invalid touch event", __func__);
			break;
		}
	} else {

	}
	return ret_val;
}

static int32_t bcmtch_dev_process_event_button(
					bcmtch_event_button_t *p_button_event)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	uint16_t evt_status = p_button_event->status;
	uint16_t btn_status = bcmtch_data_p->button_status;
	struct input_dev *pInputDevice = bcmtch_data_p->p_inputDevice;
	bcmtch_event_kind_e kind = p_button_event->button_kind;
	uint32_t button_index = 0;
	uint16_t button_check;

	if (btn_status != evt_status) {
		switch (kind) {
		case BCMTCH_EVENT_BUTTON_KIND_CONTACT:
			while (button_index < bcmtch_data_p->
				  platform_data.ext_button_count) {
				button_check = (0x1 << button_index);
				if ((btn_status & button_check) !=
						(evt_status & button_check)) {
					input_report_key(
						pInputDevice,
						bcmtch_data_p->platform_data.
						  ext_button_map[button_index],
						(evt_status & button_check));
				}
				button_index++;
			}

			if (bcmtch_debug_flag &
					BCMTCH_DEBUG_FLAG_BUTTON)
				printk(KERN_INFO
					"BCMTCH: Button: %s %#04x\n",
					"press",
					evt_status);
			break;
		case BCMTCH_EVENT_BUTTON_KIND_HOVER:
			if (bcmtch_debug_flag &
					BCMTCH_DEBUG_FLAG_BUTTON)
				printk(KERN_INFO
					"BCMTCH: Button: %s %#04x\n",
					"hover",
					evt_status);
			break;
		default:
			printk(KERN_ERR "%s: Invalid button kind %d\n",
				__func__,
				kind);
			break;
		}

		/* Report SYNC */
		input_sync(pInputDevice);

		/* Update status */
		bcmtch_data_p->button_status = evt_status;
	} else {
		if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_BUTTON)
			printk(KERN_INFO
				"BCMTCH: button status unchanged. status=0x%04x\n",
				btn_status);
	}
	return ret_val;
}

/**
 * To process whole frames of data this variable should
 * be made global because one frame can be split across
 * two invocations of the function process_channel_touch().
 */
static	bcmtch_event_kind_e top_level_kind = BCMTCH_EVENT_KIND_EXTENSION;
static	uint8_t touch_event_track_id = 0;

static int32_t bcmtch_dev_process_channel_touch(bcmtch_channel_t *chan)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	bool syn_report_pending = false;
	tofe_channel_header_t *chan_hdr = (tofe_channel_header_t *)&chan->hdr;
	uint16_t read_idx;
	bcmtch_event_t *ptch_event;
	bcmtch_event_kind_e kind;

	uint32_t frames_in = 0;

	read_idx = 0;
	while ((ptch_event = (bcmtch_event_t *)
				bcmtch_inline_channel_read(chan_hdr,
				read_idx++))) {
		kind = (bcmtch_event_kind_e)ptch_event->event_kind;

		if (kind != BCMTCH_EVENT_KIND_EXTENSION) {
			top_level_kind = kind;

			if (syn_report_pending) {
				/**
				 * The end of frame extension events.
				 * Send the SYN_REPORT for the frame.
				 */
				bcmtch_dev_sync_event_frame();
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
					(bcmtch_event_frame_t *) ptch_event);
			break;
		case BCMTCH_EVENT_KIND_TOUCH:
		case BCMTCH_EVENT_KIND_TOUCH_END:
			touch_event_track_id =
				((bcmtch_event_touch_t *)ptch_event)->track_tag;
		    bcmtch_dev_process_event_touch(
					(bcmtch_event_touch_t *)ptch_event);
			break;
		case BCMTCH_EVENT_KIND_BUTTON:
			bcmtch_dev_process_event_button(
					(bcmtch_event_button_t *)ptch_event);
			break;
		case BCMTCH_EVENT_KIND_GESTURE:
		    printk(KERN_INFO "ERROR: Gesture: NOT SUPPORTED");
			break;
		case BCMTCH_EVENT_KIND_EXTENSION:
			switch (top_level_kind) {
			case BCMTCH_EVENT_KIND_FRAME:
				bcmtch_dev_process_event_frame_extension(
					(bcmtch_event_frame_extension_t *)
					ptch_event);
				break;
			case BCMTCH_EVENT_KIND_TOUCH:
				bcmtch_dev_process_event_touch_extension(
					(bcmtch_event_touch_extension_t *)
					ptch_event,
					touch_event_track_id);
				break;
			default:
				printk(KERN_INFO
						"ERROR: Improper event extension for: tlk=%d k=%d",
							top_level_kind, kind);
				break;
			}
			break;
		default:
			printk(KERN_INFO
				"ERROR: Invalid event kind: %d.", kind);
		}
	}

    /**
     * The last event in the channel is a frame (extension) event.
     * Send the SYN_REPORT for the frame.
     */
	if (syn_report_pending) {
		bcmtch_dev_sync_event_frame();
		syn_report_pending = false;
	}

	if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_FRAMES) {
		uint32_t stalled =
				(chan_hdr->flags &
					TOFE_CHANNEL_FLAG_STATUS_OVERFLOW);

		printk(KERN_INFO
				"frames: %d - %d",
				frames_in,
				stalled);
	}
	return ret_val;
}

static int32_t bcmtch_dev_process_channel_response(bcmtch_channel_t *chan)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	uint16_t read_idx;
	bcmtch_response_wait_t *p_resp;
	tofe_channel_header_t *chan_hdr = (tofe_channel_header_t *)&chan->hdr;
	tofe_command_response_t *resp_event;

	if (bcmtch_debug_flag &
			BCMTCH_DEBUG_FLAG_CHANNELS) {
		printk(KERN_INFO
			"BCMTCH: %s() - swap count=%d response evt count=%d.\n",
			__func__,
			chan_hdr->seq_count,
			chan->queued);
	}

	read_idx = 0;
	/* Process response events */
	while ((resp_event =
				(tofe_command_response_t *)
				bcmtch_inline_channel_read(chan_hdr,
				read_idx++))) {

		if (resp_event->flags &
				TOFE_COMMAND_FLAG_COMMAND_PROCESSED) {
			if (resp_event->command > TOFE_COMMAND_LAST)
				continue;

			/* Save the response result */
			p_resp = (bcmtch_response_wait_t *)
				&(bcmtch_data_p->bcmtch_cmd_response
						[resp_event->command]);
			p_resp->wait = 0;
			p_resp->resp_data = resp_event->data;
		}

		if (bcmtch_debug_flag &
				BCMTCH_DEBUG_FLAG_CHANNELS) {
			printk(KERN_INFO
				"BCMTCH: Response - command=0x%02x result=0x%04x data=0x%08x.\n",
				resp_event->command,
				resp_event->result,
				resp_event->data);
		}

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
		printk(KERN_INFO
			"BCMTCH: code:0x%04x para0:0x%04x para1:0x%08x para2:0x%08x ts:0x%08x\n",
			code,
			param0,
			param1,
			param2,
			timestamp);
}

static int32_t bcmtch_dev_process_channel_log(bcmtch_channel_t *chan)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	uint16_t read_idx;
	tofe_channel_header_t *chan_hdr = (tofe_channel_header_t *)&chan->hdr;
	tofe_log_msg_t *log_msg;

	read_idx = 0;
	while ((log_msg =
				(tofe_log_msg_t *)
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

static int32_t bcmtch_dev_process_channels(void)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	uint32_t channel = 0;
	bcmtch_channel_t *p_chan = NULL;
#if BCMTCH_ROM_CHANNEL
	bcmtch_rom_channel_t *p_rom_chan = NULL;
#endif

	while (channel < BCMTCH_CHANNEL_MAX) {
#if BCMTCH_ROM_CHANNEL
		if (bcmtch_data_p->rom_channel) {
			p_rom_chan = bcmtch_data_p->p_rom_channels[channel];
			switch (channel) {
			case BCMTCH_CHANNEL_TOUCH:
				if (p_rom_chan)
					bcmtch_dev_process_rom_channel_touch(
							p_rom_chan);
				break;

			case BCMTCH_CHANNEL_COMMAND:
			case BCMTCH_CHANNEL_RESPONSE:
			case BCMTCH_CHANNEL_LOG:
			default:
				break;

			}
		} else {
#endif
			p_chan = bcmtch_data_p->p_channels[channel];
			if (!p_chan->active) {
				channel++;
				continue;
			}

			switch (channel) {
			case BCMTCH_CHANNEL_TOUCH:
				bcmtch_dev_process_channel_touch(p_chan);
				break;

			case BCMTCH_CHANNEL_COMMAND:
				break;
			case BCMTCH_CHANNEL_RESPONSE:
				bcmtch_dev_process_channel_response(p_chan);
				break;
			case BCMTCH_CHANNEL_LOG:
				bcmtch_dev_process_channel_log(p_chan);
				break;
			default:
				break;

			}

			if (p_chan->cfg.flags & TOFE_CHANNEL_FLAG_FWDMA_ENABLE)
				p_chan->hdr.buffer[0] = p_chan->hdr.buffer[1];
#if BCMTCH_ROM_CHANNEL
		}
#endif
		channel++;
	}

	return ret_val;
}


/* -------------------------- */
/* -	Dual Channel Mode	- */
/* -------------------------- */
#if BCMTCH_ROM_CHANNEL

unsigned bcmtch_rom_channel_num_queued(tofe_rom_channel_header_t *channel)
{
	if (channel->write >= channel->read)
		return channel->write - channel->read;
	else
		return channel->entry_num - (channel->read - channel->write);
}

static inline iterator_rom_t
_bcmtch_inline_rom_channel_next_index(
			tofe_rom_channel_header_t *channel,
			iterator_rom_t iterator)
{
	return (iterator == channel->entry_num - 1) ? 0 : iterator + 1;
}

static inline char *
_bcmtch_inline_rom_channel_entry(
			tofe_rom_channel_header_t *channel,
			uint32_t byte_index)
{
	char *data_bytes = (char *)channel + channel->data_offset;
	return &data_bytes[byte_index];
}

static inline size_t
_bcmtch_inline_rom_channel_byte_index(
		tofe_rom_channel_header_t *channel,
		iterator_rom_t entry_index)
{
	return entry_index * channel->entry_size;
}

static inline bool
bcmtch_inline_rom_channel_is_empty(tofe_rom_channel_header_t *channel)
{
	return (channel->read == channel->write);
}

static inline void
bcmtch_inline_rom_channel_read_begin(tofe_rom_channel_header_t *channel)
{
	channel->read_iterator = channel->read;
}

static inline void *bcmtch_inline_rom_channel_read(
		tofe_rom_channel_header_t *channel)
{
	char *entry;
	size_t byte_index;

	/* Validate that channel has entries. */
	if (bcmtch_inline_rom_channel_is_empty(channel))
		return NULL;

	/* Find entry in the channel. */
	byte_index =
	    _bcmtch_inline_rom_channel_byte_index(
				channel,
				channel->read_iterator);
	entry = (char *)_bcmtch_inline_rom_channel_entry(channel, byte_index);

	/* Update the read iterator. */
	channel->read_iterator =
	    _bcmtch_inline_rom_channel_next_index(
				channel,
				channel->read_iterator);

	return (void *)entry;
}

static inline uint32_t
bcmtch_inline_rom_channel_read_end(tofe_rom_channel_header_t *channel)
{
	uint32_t count = (channel->read_iterator >= channel->read) ?
	    (channel->read_iterator - channel->read) :
	    (channel->entry_num - (channel->read - channel->read_iterator));

	channel->read = channel->read_iterator;
	return count;
}

static int32_t bcmtch_dev_init_rom_channel(
		bcmtch_channel_e chan_id,
		tofe_rom_channel_instance_cfg_t *p_chan_cfg)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	uint32_t channel_size;
	tofe_rom_channel_header_t *hdr;

	channel_size =
			/* channel data size   */
			(p_chan_cfg->entry_num * p_chan_cfg->entry_size)
			/* channel header size */
			+ sizeof(tofe_rom_channel_header_t)
			/* channel config size */
			+ sizeof(tofe_rom_channel_instance_cfg_t)
			/* sizes for added elements: queued, pad */
			+ (sizeof(uint16_t) * 2);

	bcmtch_data_p->p_rom_channels[chan_id] =
		(bcmtch_rom_channel_t *)bcmtch_os_mem_alloc(channel_size);

	if (bcmtch_data_p->p_rom_channels[chan_id])
		bcmtch_data_p->p_rom_channels[chan_id]->rom_cfg = *p_chan_cfg;
	else
		ret_val = -ENOMEM;

	if (p_chan_cfg->flags & TOFE_CHANNEL_FLAG_INBOUND) {
		/* Initialize command channel header */
		hdr = &bcmtch_data_p->p_rom_channels[chan_id]->rom_hdr;
		hdr->data_offset =
			(char *)&bcmtch_data_p->p_rom_channels[chan_id]->data
			- (char *)hdr;
		hdr->entry_num = p_chan_cfg->entry_num;
		hdr->entry_size = p_chan_cfg->entry_size;
		hdr->trig_level = p_chan_cfg->trig_level;
		hdr->flags = p_chan_cfg->flags;
		hdr->read = 0;
		hdr->write = 0;
		hdr->read_iterator = 0;
		hdr->write_iterator = 0;
	}

	return ret_val;
}

static void bcmtch_dev_free_rom_channels(void)
{
	uint32_t chan = 0;
	while (chan < BCMTCH_CHANNEL_MAX)
		bcmtch_os_mem_free(bcmtch_data_p->p_rom_channels[chan++]);
}

static int32_t bcmtch_dev_init_rom_channels(
		uint32_t mem_addr,
		uint8_t *mem_data)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	uint32_t *p_cfg = NULL;
	tofe_rom_channel_instance_cfg_t *p_chan_cfg = NULL;

	/* find channel configs */
	p_cfg = (uint32_t *)(mem_data + TOFE_SIGNATURE_SIZE);
	p_chan_cfg =
		(tofe_rom_channel_instance_cfg_t *)
		((uint32_t)mem_data + p_cfg[TOFE_TOC_INDEX_CHANNEL] - mem_addr);

	/* check if processing channel(s) - add */
	if (bcmtch_channel_flag & BCMTCH_CHANNEL_FLAG_USE_TOUCH) {
		ret_val =
		    bcmtch_dev_init_rom_channel(
				BCMTCH_CHANNEL_TOUCH,
				&p_chan_cfg[TOFE_CHANNEL_ID_TOUCH]);
	}

	if (ret_val || !(bcmtch_channel_flag & BCMTCH_CHANNEL_FLAG_USE_TOUCH)) {
		printk(KERN_ERR
		       "%s: [%d] Touch Event Channel not initialized!\n",
		       __func__, ret_val);
	}

	if (!ret_val &&
		(bcmtch_channel_flag & BCMTCH_CHANNEL_FLAG_USE_CMD_RESP)) {
		ret_val =
		    bcmtch_dev_init_rom_channel(
						BCMTCH_CHANNEL_COMMAND,
					    &p_chan_cfg
					    [TOFE_CHANNEL_ID_COMMAND]);
		ret_val |=
		    bcmtch_dev_init_rom_channel(
						BCMTCH_CHANNEL_RESPONSE,
					    &p_chan_cfg
					    [TOFE_CHANNEL_ID_RESPONSE]);
	}

	if (!ret_val && (bcmtch_channel_flag & BCMTCH_CHANNEL_FLAG_USE_LOG)) {
		ret_val =
		    bcmtch_dev_init_rom_channel(
						BCMTCH_CHANNEL_LOG,
					    &p_chan_cfg[TOFE_CHANNEL_ID_LOG]);
	}

	return ret_val;
}

static inline void
bcmtch_inline_rom_channel_write_begin(tofe_rom_channel_header_t *channel)
{
	channel->write_iterator = (iterator_rom_t) channel->write;
}

static inline void
bcmtch_inline_rom_channel_write_end(tofe_rom_channel_header_t *channel)
{
	channel->write = (uint32_t) channel->write_iterator;
}

static inline iterator_t
bcmtch_inline_rom_channel_next_index(
		tofe_rom_channel_header_t *channel,
		iterator_t iterator)
{
	return (iterator == channel->entry_num-1) ? 0 : iterator+1;
}

static inline size_t
bcmtch_inline_rom_channel_byte_index(
		tofe_rom_channel_header_t *channel,
		iterator_t entry_index)
{
	return entry_index * channel->entry_size;
}

static inline char *
bcmtch_inline_rom_channel_entry(
		tofe_rom_channel_header_t *channel,
		uint32_t byte_index)
{
	char * data_bytes = (char *)channel + channel->data_offset;
	return &data_bytes[byte_index];
}

static inline uint32_t
bcmtch_inline_rom_channel_write(
		tofe_rom_channel_header_t *channel,
		void *entry)
{
	size_t byte_index;

	/* If channel is full. */
	if (channel->read ==
			 bcmtch_inline_rom_channel_next_index(
				 channel,
				 channel->write_iterator))
		return -ENOMEM;

	/* Copy entry to the channel. */
	byte_index = bcmtch_inline_rom_channel_byte_index(
			channel,
			channel->write_iterator);

	memcpy(bcmtch_inline_rom_channel_entry(
				channel,
				byte_index),
			entry, channel->entry_size);

	/* Update the write iterator. */
	channel->write_iterator =
		bcmtch_inline_rom_channel_next_index(
				channel,
				channel->write_iterator);

	return BCMTCH_SUCCESS;
}

static int32_t bcmtch_dev_write_rom_channel(bcmtch_rom_channel_t *chan)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	int16_t writeSize;
	uint32_t sys_addr;

	/* read channel header and data all-at-once : need combined size */
	writeSize = sizeof(chan->rom_hdr)
			+ (chan->rom_cfg.entry_num * chan->rom_cfg.entry_size);

	sys_addr = (uint32_t)chan->rom_cfg.channel_header;

	/* write channel header & channel data buffer */
	ret_val = bcmtch_com_write_sys(
				sys_addr,
				writeSize,
				(uint8_t *)&chan->rom_hdr);
	if (ret_val) {
		printk(KERN_ERR
				"BCMTOUCH: %s() write_sys err addr=0x%08x, rv=%d\n",
				__func__,
				sys_addr,
				ret_val);
	}
	return ret_val;
}

static int32_t bcmtch_dev_read_rom_channel(bcmtch_rom_channel_t *chan)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	int16_t readSize;
	uint32_t wbAddr;

	/* read channel header and data all-at-once : need combined size */
	readSize =
	    sizeof(chan->rom_hdr) +
		(chan->rom_cfg.entry_num * chan->rom_cfg.entry_size);

	/* read channel header & channel data buffer */
	ret_val = bcmtch_com_read_sys(
				(uint32_t)chan->rom_cfg.channel_header,
				readSize,
				(uint8_t *)&chan->rom_hdr);

	/* get count */
	chan->queued = bcmtch_rom_channel_num_queued(&chan->rom_hdr);

	/* write back to update channel */
	if (chan->queued) {
		wbAddr =
		    (uint32_t)chan->rom_cfg.channel_header +
		    offsetof(tofe_rom_channel_header_t, read);
		ret_val = bcmtch_com_write_sys32(wbAddr, chan->rom_hdr.write);
	}

	return ret_val;
}

static int32_t bcmtch_dev_process_rom_event_frame(
					bcmtch_rom_event_frame_t *p_frame_event)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	struct input_dev *pInputDevice = bcmtch_data_p->p_inputDevice;
	bcmtch_touch_t *pTouch;
	uint32_t numTouches = 0;
	uint32_t touchIndex = 0;

	for (touchIndex = 0; touchIndex < BCMTCH_MAX_TOUCH; touchIndex++) {
		pTouch = (bcmtch_touch_t *) &bcmtch_data_p->touch[touchIndex];

#if BCMTCH_USE_PROTOCOL_B
		input_mt_slot(pInputDevice, touchIndex);
		input_mt_report_slot_state(
			pInputDevice,
			MT_TOOL_FINGER,
			(pTouch->status > BCMTCH_TOUCH_STATUS_UP));
#endif
		if (pTouch->status > BCMTCH_TOUCH_STATUS_UP) {
			numTouches++;

			input_report_abs(
					pInputDevice,
					ABS_MT_POSITION_X,
					pTouch->x);

			input_report_abs(
					pInputDevice,
					ABS_MT_POSITION_Y,
					pTouch->y);

#if !BCMTCH_USE_PROTOCOL_B
			input_report_abs(
					pInputDevice,
					ABS_MT_TRACKING_ID,
					touchIndex);

			input_mt_sync(pInputDevice);
#endif
		}
	}

	input_report_key(pInputDevice, BTN_TOUCH, (numTouches > 0));
	input_sync(pInputDevice);

	/* remember */
	bcmtch_data_p->touch_count = numTouches;

	if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_FRAME)
		printk(KERN_INFO
			"BCMTCH: FR: T=%d ID=%d\n",
			numTouches,
			p_frame_event->frame_id);

	return ret_val;
}

static int32_t bcmtch_dev_process_rom_event_touch(
					bcmtch_rom_event_touch_t *p_touch_event)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	bcmtch_touch_t *p_touch;

	if (p_touch_event->track_tag < BCMTCH_MAX_TOUCH) {
		p_touch =
		    (bcmtch_touch_t *)
			&bcmtch_data_p->touch[p_touch_event->track_tag];

		p_touch_event->x =
			bcmtch_data_p->actual_x(p_touch_event->x);

		p_touch_event->y =
			bcmtch_data_p->actual_y(p_touch_event->y);

		bcmtch_data_p->actual_x_y_axis(p_touch_event->x,
			p_touch_event->y, p_touch);

		switch (p_touch_event->type) {
		case BCMTCH_EVENT_TYPE_DOWN:
			p_touch->rom_event = p_touch_event->type;
			p_touch->status = BCMTCH_TOUCH_STATUS_MOVE;

			if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_MOVE)
				printk(KERN_INFO
					"BCMTCH: DN: T%d: (%04x , %04x)\n",
					p_touch_event->track_tag,
					p_touch->x,
					p_touch->y);
			break;

		case BCMTCH_EVENT_TYPE_UP:
			p_touch->rom_event = p_touch_event->type;
			p_touch->status = BCMTCH_TOUCH_STATUS_UP;

			if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_UP)
				printk(KERN_INFO
					"BCMTCH: UP: T%d: (%04x , %04x)\n",
					p_touch_event->track_tag,
					p_touch->x,
					p_touch->y);
			break;

		case BCMTCH_EVENT_TYPE_MOVE:
			p_touch->rom_event = p_touch_event->type;
			p_touch->status = BCMTCH_TOUCH_STATUS_MOVING;

			if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_MOVE)
				printk(KERN_INFO
					"BCMTCH: MV: T%d: (%04x , %04x)\n",
					p_touch_event->track_tag,
					p_touch->x,
					p_touch->y);
			break;
		}
	} else {

	}
	return ret_val;
}

static int32_t bcmtch_dev_process_rom_channel_touch(bcmtch_rom_channel_t *chan)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	bcmtch_rom_event_t *ptch_event;
	tofe_rom_channel_header_t *chan_hdr =
		(tofe_rom_channel_header_t *)&chan->rom_hdr;

	bcmtch_inline_rom_channel_read_begin(chan_hdr);

	while ((ptch_event = (bcmtch_rom_event_t *)
				bcmtch_inline_rom_channel_read(chan_hdr))) {
		switch (ptch_event->type) {
		case BCMTCH_EVENT_TYPE_DOWN:
		case BCMTCH_EVENT_TYPE_UP:
		case BCMTCH_EVENT_TYPE_MOVE:
			bcmtch_dev_process_rom_event_touch(
				(bcmtch_rom_event_touch_t *)ptch_event);
			break;

		case BCMTCH_EVENT_TYPE_FRAME:
			bcmtch_dev_process_rom_event_frame(
				(bcmtch_rom_event_frame_t *)ptch_event);
			break;

		case BCMTCH_EVENT_TYPE_TIMESTAMP:
			break;

		default:
			break;
		}

		/* Finished processing event, so update read pointer. */
		bcmtch_inline_rom_channel_read_end(chan_hdr);
	}

	return ret_val;
}

static int32_t bcmtch_dev_write_rom_command(
	tofe_command_e command,
	uint32_t data,
	uint16_t data16,
	uint8_t flags)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	tofe_command_response_t cmd;
	bcmtch_rom_channel_t *chan;

	chan = bcmtch_data_p->p_rom_channels[TOFE_CHANNEL_ID_COMMAND];
	if (chan == NULL) {
		printk(KERN_ERR
				"%s: command channel has not initialized!\n",
				__func__);
		return -ENXIO;
	}

	/* Send host to firmware message. */
	ret_val = bcmtch_dev_request_power_mode(
					BCMTCH_POWER_MODE_WAKE,
					command);

	/* Setup the command entry */
	memset((void *)(&cmd), 0, sizeof(tofe_command_response_t));
	cmd.flags	= flags;
	cmd.command	= command;
	cmd.data	= data;
	cmd.result = data16;

	/* Write sys to command channel */
	bcmtch_inline_rom_channel_write_begin(&chan->rom_hdr);
	ret_val = bcmtch_inline_rom_channel_write(&chan->rom_hdr, &cmd);
	if (ret_val) {
		printk(KERN_ERR
				"%s: [%d] cmd channel write failed.\n",
				__func__,
				ret_val);
		return ret_val;
	}
	bcmtch_inline_rom_channel_write_end(&chan->rom_hdr);

	ret_val = bcmtch_dev_write_rom_channel(chan);
	if (ret_val) {
		printk(KERN_ERR
				"%s: [%d] cmd channel write back FW failed.\n",
				__func__,
				ret_val);
	}

    /* release channel */
	bcmtch_dev_request_power_mode(
		BCMTCH_POWER_MODE_NOWAKE,
		command);

	return ret_val;
}

#endif
/* -------------------------------------- */
/* -	End of Dual Channel Functions	- */
/* -------------------------------------- */


struct firmware_load_info_t_ {
	uint8_t *filename;
	uint32_t addr;
	uint32_t flags;
};
#define bcmtch_firmware_load_info_t struct firmware_load_info_t_

static const bcmtch_firmware_load_info_t bcmtch_binaries[] = {
	{"bcmtchfw_bin", 0, BCMTCH_FIRMWARE_FLAGS_COMBI},
	{0, 0, 0}
};

static int32_t bcmtch_dev_wait_for_firmware_ready(int32_t count)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	uint8_t ready;

	do {
		ret_val =
		    bcmtch_com_read_spm(BCMTCH_SPM_REG_MSG_TO_HOST, &ready);
	} while ((!ret_val) && (ready != TOFE_MESSAGE_FW_READY) && (count--));

	if (count <= 0) {
		printk(KERN_ERR
		       "ERROR: Failed to communicate with Napa FW. Error: 0x%x\n",
		       ready);
		ret_val = -1;
	}

	return ret_val;
}

static int32_t bcmtch_dev_run_firmware(void)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	ret_val = bcmtch_dev_reset(BCMTCH_RESET_MODE_SOFT_CLEAR);
	ret_val |=
		bcmtch_com_write_sys32(
			BCMTCH_ADDR_SPM_STICKY_BITS,
			BCMTCH_SPM_STICKY_BITS_PIN_RESET);

	if (bcmtch_dev_wait_for_firmware_ready(1000)) {
		uint8_t xaddr = 0x40;
		uint8_t xdata;
		while (xaddr <= 0x61) {
			bcmtch_com_read_spm(xaddr, &xdata);
			printk(KERN_ERR
				"%s: addr = 0x%02x  data = 0x%02x\n",
				__func__,
				xaddr++,
				xdata);
		}
	}

	return ret_val;
}

static int32_t bcmtch_dev_download_firmware(
					uint8_t *fw_name,
					uint32_t fw_addr,
					uint32_t fw_flags)
{
	const struct firmware *p_fw;
	int32_t ret_val = BCMTCH_SUCCESS;

	uint32_t entryId = 1;
	bcmtch_combi_entry_t *p_entry = NULL;
	bcmtch_combi_entry_t default_entry[] = {
		{.addr = fw_addr, .flags = fw_flags,},
		{0, 0, 0, 0},
	};

	/* request firmware binary from OS */
	ret_val = request_firmware(
				&p_fw, fw_name,
				&bcmtch_data_p->p_i2c_client_spm->dev);

	if (ret_val) {
		printk(KERN_ERR
			"%s: Firmware request failed (%d) for %s\n",
			__func__,
			ret_val,
			fw_name);
	} else {
		printk(KERN_INFO "BCMTCH: FIRMWARE: %s\n", fw_name);
#if BCMTCH_POST_BOOT
		if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_POST_BOOT) {
			printk(KERN_INFO "BCMTCH: firmware size= 0x%x\n",
					p_fw->size);
		}


		/* Allocate firmware buffer memory */
		bcmtch_data_p->post_boot_buffer =
			(uint8_t *)bcmtch_os_mem_alloc(p_fw->size);
		if (bcmtch_data_p->post_boot_buffer == NULL) {
			printk(KERN_ERR
					"%s: failed to alloc firmware buffer.\n",
					__func__);
			ret_val = -ENOMEM;
			goto download_error;
		}

		memcpy(bcmtch_data_p->post_boot_buffer,
				(void *) p_fw->data,
				p_fw->size);
#endif

		/* pre-process binary according to flags */
		if (fw_flags & BCMTCH_FIRMWARE_FLAGS_COMBI) {
			p_entry = (bcmtch_combi_entry_t *) p_fw->data;
		} else {
			p_entry = default_entry;
			p_entry[entryId].length = p_fw->size;
		}

		while (p_entry[entryId].length) {
			switch (p_entry[entryId].flags &
					BCMTCH_FIRMWARE_FLAGS_MASK) {
#if BCMTCH_POST_BOOT
			case BCMTCH_FIRMWARE_FLAGS_POST_BOOT_CONFIGS:
				if (bcmtch_debug_flag &
						BCMTCH_DEBUG_FLAG_POST_BOOT)
					printk(KERN_INFO "BCMTCH: entryId=%d PB CONFIG\n",
							entryId);
				if (bcmtch_debug_flag &
						BCMTCH_DEBUG_FLAG_POST_BOOT)
					printk(KERN_INFO "BCMTCH: pb chans init addr=0x%08x\n",
							p_entry[entryId].addr);

				bcmtch_data_p->postboot_cfg_addr =
					p_entry[entryId].addr;
				bcmtch_data_p->postboot_cfg_length =
					p_entry[entryId].length;

				bcmtch_dev_init_channels(
					BCMTCH_ADDR_TOC_BASE,
					(uint8_t *)((uint32_t)p_fw->data +
					p_entry[entryId].offset));

			case BCMTCH_FIRMWARE_FLAGS_POST_BOOT_CODE:
				bcmtch_data_p->post_boot_sections++;
				if (bcmtch_debug_flag &
						BCMTCH_DEBUG_FLAG_POST_BOOT)
					printk(KERN_INFO "BCMTCH: entryId=%d PB pb_sec=%d\n",
						entryId,
						bcmtch_data_p->
						post_boot_sections);
				break;
#endif /* POST_BOOT */
			case BCMTCH_FIRMWARE_FLAGS_CONFIGS:
				/* Initialize channels */
				if (bcmtch_debug_flag &
						BCMTCH_DEBUG_FLAG_POST_BOOT)
					printk(KERN_INFO "BCMTCH: entryId=%d CONFIG\n",
							entryId);
#if BCMTCH_ROM_CHANNEL
				if (bcmtch_data_p->rom_channel) {
					if (bcmtch_debug_flag &
						BCMTCH_DEBUG_FLAG_POST_BOOT)
						printk(KERN_INFO "BCMTCH: rom chan init addr=0x%08x\n",
							p_entry[entryId].addr);
					bcmtch_dev_init_rom_channels(
						p_entry[entryId].addr,
						(uint8_t *)((uint32_t)p_fw->data
						+ p_entry[entryId].offset));
				} else {
#endif
					if (bcmtch_debug_flag &
						BCMTCH_DEBUG_FLAG_POST_BOOT)
						printk(KERN_INFO "BCMTCH: ram chan init addr=0x%08x\n",
							p_entry[entryId].addr);
					bcmtch_dev_init_channels(
						p_entry[entryId].addr,
						(uint8_t *)((uint32_t)p_fw->data
						+ p_entry[entryId].offset));
#if BCMTCH_ROM_CHANNEL
				}
#endif
			default:
				if (bcmtch_debug_flag &
						BCMTCH_DEBUG_FLAG_POST_BOOT)
					printk(KERN_INFO "BCMTCH: entryId=%d PREBOOT\n",
							entryId);
				/** download to chip **/
				ret_val = bcmtch_com_write_sys(
					p_entry[entryId].addr,
					p_entry[entryId].length,
					(uint8_t *)((uint32_t)p_fw->data +
					p_entry[entryId].offset));
			}

			/* next */
			entryId++;
		}
	}

	if (bcmtch_boot_flag &
			BCMTCH_BOOT_FLAG_DISABLE_POST_BOOT)
		bcmtch_data_p->post_boot_sections = 0;

	if (bcmtch_data_p->post_boot_sections) {
		/* setup first section for download */
		if (!bcmtch_dev_post_boot_get_section())
			bcmtch_data_p->post_boot_sections = 0;
	}

download_error:
	/* free kernel structures */
	release_firmware(p_fw);
	return ret_val;
}

static int32_t bcmtch_dev_init_firmware(void)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	uint8_t binFile = 0;

	if (bcmtch_firmware) {
		ret_val =
			bcmtch_dev_download_firmware(
				bcmtch_firmware,
				bcmtch_firmware_addr,
				bcmtch_firmware_flag);
	} else {
		while (bcmtch_binaries[binFile].filename) {
			ret_val =
				bcmtch_dev_download_firmware(
					bcmtch_binaries[binFile].filename,
					bcmtch_binaries[binFile].addr,
					bcmtch_binaries[binFile].flags);
			binFile++;
		}
	}

	if (!ret_val)
		ret_val = bcmtch_dev_run_firmware();

	if (!ret_val)
		ret_val = bcmtch_dev_watchdog_start();

	if (!ret_val)
		ret_val = bcmtch_os_interrupt_enable();

	return ret_val;
}

static int32_t bcmtch_dev_init_platform(struct device *p_device)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	struct bcmtch_platform_data *p_platform_data;


	if (p_device && bcmtch_data_p) {
		p_platform_data =
			(struct bcmtch_platform_data *)p_device->platform_data;

	    bcmtch_data_p->platform_data.i2c_bus_id =
		    p_platform_data->i2c_bus_id;
		bcmtch_data_p->platform_data.i2c_addr_sys =
		    p_platform_data->i2c_addr_sys;

		bcmtch_data_p->platform_data.i2c_addr_spm =
		    p_platform_data->i2c_addr_spm;

		bcmtch_data_p->platform_data.gpio_reset_pin =
		    p_platform_data->gpio_reset_pin;
		bcmtch_data_p->platform_data.gpio_reset_polarity =
		    p_platform_data->gpio_reset_polarity;
		bcmtch_data_p->platform_data.gpio_reset_time_ms =
		    p_platform_data->gpio_reset_time_ms;

		bcmtch_data_p->platform_data.gpio_interrupt_pin =
		    p_platform_data->gpio_interrupt_pin;
		bcmtch_data_p->platform_data.gpio_interrupt_trigger =
		    p_platform_data->gpio_interrupt_trigger;

		bcmtch_data_p->platform_data.ext_button_count =
			p_platform_data->ext_button_count;

		bcmtch_data_p->platform_data.ext_button_map =
			p_platform_data->ext_button_map;
		bcmtch_data_p->platform_data.axis_orientation_flag =
			p_platform_data->axis_orientation_flag;

		/* setup function pointers for axis coordinates */
		bcmtch_data_p->actual_x =
			(bcmtch_data_p->platform_data.axis_orientation_flag &
				BCMTCH_AXIS_FLAG_X_REVERSED_MASK) ?
				bcmtch_reverse_x_coordinate :
				bcmtch_get_coordinate;

		bcmtch_data_p->actual_y =
			(bcmtch_data_p->platform_data.axis_orientation_flag &
				BCMTCH_AXIS_FLAG_Y_REVERSED_MASK) ?
				bcmtch_reverse_y_coordinate :
				bcmtch_get_coordinate;

		bcmtch_data_p->actual_x_y_axis =
			(bcmtch_data_p->platform_data.axis_orientation_flag &
				BCMTCH_AXIS_FLAG_X_Y_SWAPPED_MASK) ?
				bcmtch_swap_x_y_axis :
				bcmtch_set_x_y_axis;

		bcmtch_data_p->platform_data.bcmtch_on =
			p_platform_data->bcmtch_on;

		if (NULL == p_platform_data->bcmtch_on) {
			printk(KERN_ERR
				"%s. BCMTCH Power on function undefined\n",
				__func__);
		}
	} else {
		printk(KERN_ERR
			"%s() error, platform data == NULL\n",
			__func__);
		ret_val = -ENODATA;
	}

	return ret_val;
}

static int32_t bcmtch_dev_request_power_mode(
					uint8_t mode,
					tofe_command_e command)
{
	int32_t ret_val = BCMTCH_SUCCESS;

#if BCMTCH_USE_FAST_I2C
	uint8_t regs[5];
	uint8_t data[5];
#endif

	switch (mode) {
	case BCMTCH_POWER_MODE_SLEEP:
		ret_val =
		    bcmtch_com_write_spm(BCMTCH_SPM_REG_MSG_FROM_HOST, command);
		ret_val |=
		    bcmtch_com_write_spm(
				BCMTCH_SPM_REG_RQST_FROM_HOST,
				BCMTCH_POWER_MODE_SLEEP);
		break;

	case BCMTCH_POWER_MODE_WAKE:
		ret_val =
		    bcmtch_com_write_spm(BCMTCH_SPM_REG_MSG_FROM_HOST, command);
		ret_val |=
		    bcmtch_com_write_spm(
				BCMTCH_SPM_REG_RQST_FROM_HOST,
				BCMTCH_POWER_MODE_WAKE);
		break;

	case BCMTCH_POWER_MODE_NOWAKE:
#if BCMTCH_USE_FAST_I2C
		regs[0] = BCMTCH_SPM_REG_MSG_FROM_HOST;
		data[0] = command;
		regs[1] = BCMTCH_SPM_REG_RQST_FROM_HOST;
		data[1] = BCMTCH_POWER_MODE_NOWAKE;
		ret_val = bcmtch_com_fast_write_spm(2, regs, data);
#else
		ret_val =
		    bcmtch_com_write_spm(BCMTCH_SPM_REG_MSG_FROM_HOST, command);
		ret_val |=
		    bcmtch_com_write_spm(
				BCMTCH_SPM_REG_RQST_FROM_HOST,
				BCMTCH_POWER_MODE_NOWAKE);
#endif
		break;

	default:
		PROGRESS();
		break;
	}

	return ret_val;
}

static int32_t bcmtch_dev_get_power_state(void)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	uint8_t power_state;

	ret_val = bcmtch_com_read_spm(BCMTCH_SPM_REG_PSR, &power_state);

	return (ret_val) ? (ret_val) : ((uint32_t)power_state);
}

static int32_t bcmtch_dev_set_power_state(uint8_t power_state)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	int32_t state = power_state;

	switch (power_state) {
	case BCMTCH_POWER_STATE_SLEEP:
		ret_val =
		    bcmtch_dev_request_power_mode(
				BCMTCH_POWER_MODE_SLEEP,
				TOFE_COMMAND_NO_COMMAND);

		ret_val |=
		    bcmtch_com_write_sys(
				BCMTCH_ADDR_SPM_PWR_CTRL,
				sizeof(int32_t),
				(uint8_t *)&state);
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
					uint8_t power_state,
					uint8_t wait_count)
{
	int32_t ret_val = -EAGAIN;
	int32_t read_state;

	do {
		read_state = bcmtch_dev_get_power_state();
		if (read_state == power_state) {
			ret_val = BCMTCH_SUCCESS;
			break;
		}
		bcmtch_os_sleep_ms(1);
	} while (wait_count--);

	return ret_val;
}

static bcmtch_status_e bcmtch_dev_request_host_override(tofe_command_e command)
{
	bcmtch_status_e ret_val = BCMTCH_STATUS_ERR_FAIL;
	int32_t count = 250;
	uint8_t m2h;

	/* Request channel & wakeup the firmware */
	ret_val = bcmtch_dev_request_power_mode(
					BCMTCH_POWER_MODE_WAKE,
					command);

	if (!ret_val)
		ret_val = bcmtch_dev_check_power_state(
					BCMTCH_POWER_STATE_ACTIVE,
					25);

	if (ret_val) {
		printk(KERN_ERR
				"%s: [%d] wake firmware failed.\n",
				__func__,
				ret_val);

	} else {

		/* Wait till FW OVERRIDE is ready */
		do {
			ret_val = bcmtch_com_read_spm(
					BCMTCH_SPM_REG_MSG_TO_HOST,
					&m2h);

			switch (m2h) {
			case TOFE_MESSAGE_FW_READY_OVERRIDE:
			case TOFE_MESSAGE_FW_READY_INTERRUPT_OVERRIDE:
				bcmtch_data_p->host_override = true;
				printk(KERN_ERR
						"Request -host override- m=0x%0x  ho=%d",
						m2h,
						bcmtch_data_p->host_override);
				break;

			case TOFE_MESSAGE_FW_READY_INTERRUPT:
				printk(KERN_ERR
					"Request -host override- m=0x%0x  ho=%d -> interrupt",
					m2h,
					bcmtch_data_p->host_override);
				bcmtch_dev_process();
			case TOFE_MESSAGE_FW_READY:
			default:
				printk(KERN_ERR
						"Request -host override- m=0x%0x  ho=%d",
						m2h,
						bcmtch_data_p->host_override);
				break;
			}
		} while (!bcmtch_data_p->host_override && count--);

		if (bcmtch_data_p->host_override)
			ret_val = BCMTCH_STATUS_SUCCESS;
	}

	return ret_val;
}

static bcmtch_status_e bcmtch_dev_release_host_override(tofe_command_e command)
{
	bcmtch_status_e ret_val = BCMTCH_STATUS_ERR_FAIL;
	int32_t count = 250;
	uint8_t m2h;

    /* this should release hostOverride - do we need to check */
	/* Release channel */
	ret_val = bcmtch_dev_request_power_mode(
				BCMTCH_POWER_MODE_NOWAKE,
				command);

	/* Wait till FW is ready */
	do {
		ret_val = bcmtch_com_read_spm(BCMTCH_SPM_REG_MSG_TO_HOST, &m2h);
		switch (m2h) {
		case TOFE_MESSAGE_FW_READY_INTERRUPT_OVERRIDE:
			printk(KERN_ERR
					"Release -host override- m=0x%0x  ho=%d -> interrupt",
					m2h,
					bcmtch_data_p->host_override);
			bcmtch_dev_process();
		case TOFE_MESSAGE_FW_READY_OVERRIDE:
		default:
			printk(KERN_ERR
				"Release -host override- m=0x%0x  ho=%d",
				m2h,
				bcmtch_data_p->host_override);
			break;

		case TOFE_MESSAGE_FW_READY_INTERRUPT:
		case TOFE_MESSAGE_FW_READY:
			bcmtch_data_p->host_override = false;
			printk(KERN_ERR
					"Release -host override- m=0x%0x  ho=%d",
					m2h,
					bcmtch_data_p->host_override);
			break;
		}
	} while (bcmtch_data_p->host_override && count--);

	if (!bcmtch_data_p->host_override)
		ret_val = BCMTCH_STATUS_SUCCESS;

	return ret_val;
}

static int32_t bcmtch_dev_send_command(
	tofe_command_e command,
	uint32_t data,
	uint8_t flags)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	tofe_command_response_t cmd;
	bcmtch_channel_t *chan;

	if (command == TOFE_COMMAND_NO_COMMAND) {
		printk(KERN_ERR
				"%s: no_command.\n",
				__func__);
		return -EINVAL;
	}

	chan = bcmtch_data_p->p_channels[TOFE_CHANNEL_ID_COMMAND];
	if (chan == NULL) {
		printk(KERN_ERR
				"%s: command channel has not initialized!\n",
				__func__);
		return -ENXIO;
	}


	if (bcmtch_dev_request_host_override(command) ==
			BCMTCH_STATUS_SUCCESS) {

		/* Setup the command entry */
		memset((void *)(&cmd), 0, sizeof(tofe_command_response_t));
		cmd.flags	= flags;
		cmd.command	= command;
		cmd.data	= data;

		/* Write sys to command channel */
		tofe_channel_write_begin(&chan->hdr);
		ret_val = tofe_channel_write(&chan->hdr, &cmd);
		if (ret_val) {
			printk(KERN_ERR
					"%s: [%d] cmd channel write failed.\n",
					__func__,
					ret_val);
			goto send_command_exit;
		}

		ret_val = bcmtch_dev_write_channel(chan);
		if (ret_val) {
			printk(KERN_ERR
					"%s: [%d] cmd channel write back FW failed.\n",
					__func__,
					ret_val);
			goto send_command_exit;
		}
	}

	bcmtch_dev_release_host_override(command);

send_command_exit:
	return ret_val;
}

static int32_t bcmtch_dev_reset(uint8_t mode)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	switch (mode) {
	case BCMTCH_RESET_MODE_HARD:
		bcmtch_os_reset();
		break;

	case BCMTCH_RESET_MODE_SOFT_CHIP:
		bcmtch_com_write_spm(
			BCMTCH_SPM_REG_SOFT_RESETS,
			BCMTCH_RESET_MODE_SOFT_CHIP);
		break;

	case BCMTCH_RESET_MODE_SOFT_ARM:
		bcmtch_com_write_spm(
			BCMTCH_SPM_REG_SOFT_RESETS,
			BCMTCH_RESET_MODE_SOFT_ARM);
		break;

	case (BCMTCH_RESET_MODE_SOFT_CHIP | BCMTCH_RESET_MODE_SOFT_ARM):
		bcmtch_com_write_spm(
			BCMTCH_SPM_REG_SOFT_RESETS,
			BCMTCH_RESET_MODE_SOFT_CHIP);
		bcmtch_com_write_spm(
			BCMTCH_SPM_REG_SOFT_RESETS,
			BCMTCH_RESET_MODE_SOFT_ARM);
		break;

	case BCMTCH_RESET_MODE_SOFT_CLEAR:
		ret_val = bcmtch_com_write_spm(
					BCMTCH_SPM_REG_SOFT_RESETS,
					BCMTCH_RESET_MODE_SOFT_CLEAR);
		break;

	default:
		break;
	}

	return ret_val;
}

static int32_t bcmtch_dev_verify_chip_id(void)
{
	int32_t ret_val = -ENXIO;
	uint32_t chipID;
	uint8_t revID;
	uint8_t id[3];
	uint32_t *pChips = (uint32_t *)BCMTCH_CHIP_IDS;

	ret_val = bcmtch_com_read_spm(BCMTCH_SPM_REG_REVISIONID, &revID);
	ret_val |= bcmtch_com_read_spm(BCMTCH_SPM_REG_CHIPID0, &id[0]);
	ret_val |= bcmtch_com_read_spm(BCMTCH_SPM_REG_CHIPID1, &id[1]);
	ret_val |= bcmtch_com_read_spm(BCMTCH_SPM_REG_CHIPID2, &id[2]);

	chipID = ((((uint32_t)id[2]) << 16)
			| (((uint32_t)id[1]) << 8)
			| (uint32_t)id[0]);

	while (*pChips && (*pChips != chipID))
		pChips++;

	if (*pChips) {
		/* Found a match in above search */
		ret_val = BCMTCH_SUCCESS;
	} else
		ret_val = -ENXIO;

	printk(KERN_INFO
			"BCMTCH: ChipId = 0x%06X  Rev = 0x%2X : %s\n",
			chipID,
			revID,
			((ret_val) ? "Error - Unknown device" : "Verified"));

	return ret_val;
}

static int32_t bcmtch_dev_verify_chip_version(void)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	uint32_t version;

	ret_val = bcmtch_com_read_sys(
				BCMTCH_ADDR_TCH_VER,
				4,
				(uint8_t *)&version);

	printk(KERN_INFO "BCMTCH: Chip Version = 0x%08X\n", version);

	return ret_val;
}

static int32_t bcmtch_dev_init(void)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	/* verify chip id */
	ret_val = bcmtch_dev_verify_chip_id();

    /* verify initial / powerup reset */
	if (!ret_val &&
		(BCMTCH_POWER_STATE_SLEEP != bcmtch_dev_get_power_state())) {
		if (BCMTCH_POWER_STATE_SLEEP != bcmtch_dev_get_power_state())
			bcmtch_dev_reset(
				BCMTCH_RESET_MODE_SOFT_CHIP |
				BCMTCH_RESET_MODE_SOFT_ARM);

		ret_val = bcmtch_dev_check_power_state(
					BCMTCH_POWER_STATE_SLEEP,
					25);
	}

    /* init com */
	if (!ret_val)
		ret_val = bcmtch_com_init();

    /* wakeup */
	if (!ret_val)
		ret_val = bcmtch_dev_request_power_mode(
					BCMTCH_POWER_MODE_WAKE,
					TOFE_COMMAND_NO_COMMAND);

	if (!ret_val)
		ret_val = bcmtch_dev_check_power_state(
					BCMTCH_POWER_STATE_ACTIVE,
					25);

	/* init clocks */
	if (!ret_val)
		ret_val = bcmtch_dev_init_clocks();

	/* init memory */
	if (!ret_val)
		ret_val = bcmtch_dev_init_memory();

	if (!ret_val)
		ret_val = bcmtch_dev_verify_chip_version();

    /* download and run */
	if (!ret_val)
		ret_val = bcmtch_dev_init_firmware();

	return ret_val;
}

static void bcmtch_dev_process(void)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	uint8_t m2h;

	if (bcmtch_boot_flag & BCMTCH_BOOT_FLAG_CHECK_INTERRUPT) {
		/* Check msg2host */
		bcmtch_com_read_spm(BCMTCH_SPM_REG_MSG_TO_HOST, &m2h);
		if (m2h != 0x81) {
			printk(KERN_INFO
					"False interrupt.\n");
			return;
		}
	}

	/* read DMA buffer */
	if (bcmtch_data_p->has_dma_channel && !bcmtch_data_p->host_override)
		ret_val = bcmtch_dev_read_dma_channels();

	/* read channels */
	ret_val = bcmtch_dev_read_channels();

    /* release memory */
	bcmtch_dev_request_power_mode(
		BCMTCH_POWER_MODE_NOWAKE,
		TOFE_COMMAND_NO_COMMAND);

	/* process channels */
	ret_val = bcmtch_dev_process_channels();
}

static void bcmtch_dev_reset_events(void)
{
#if BCMTCH_USE_PROTOCOL_B
	uint32_t touch = 0;
#endif

	/* clear active touch structure */
	memset(
		&bcmtch_data_p->touch[0],
		0,
		sizeof(bcmtch_touch_t) * BCMTCH_MAX_TOUCH);

	/* clear system touches */
	if (bcmtch_data_p->touch_count) {
#if BCMTCH_USE_PROTOCOL_B
		for (touch = 0; touch < BCMTCH_MAX_TOUCH; touch++) {
			input_mt_slot(
				bcmtch_data_p->p_inputDevice,
				touch);
			input_mt_report_slot_state(
				bcmtch_data_p->p_inputDevice,
				MT_TOOL_FINGER,
				false);
		}
#endif

		input_report_key(
			bcmtch_data_p->p_inputDevice,
			BTN_TOUCH,
			false);

		input_sync(bcmtch_data_p->p_inputDevice);
	}
	bcmtch_data_p->touch_count = 0;
}


#if BCMTCH_POST_BOOT
static void bcmtch_dev_post_boot_reset(void)
{
	bcmtch_data_p->post_boot_section = 0;
	bcmtch_data_p->post_boot_sections = 0;

	/* free communication channels */
	bcmtch_dev_free_channels();
	bcmtch_dev_free_rom_channels();
}
#endif

static int32_t bcmtch_dev_suspend(void)
{
	int32_t ret_val = BCMTCH_SUCCESS;

    /* disable interrupts */
	bcmtch_os_interrupt_disable();

	/* free watchdog timer */
	bcmtch_dev_watchdog_stop();

	/* clear worker */
	bcmtch_os_clear_deferred_worker();

	/* lock */
#if !BCMTCH_USE_BUS_LOCK
#if BCMTCH_USE_WORK_LOCK
	bcmtch_os_lock_critical_section(BCMTCH_MUTEX_WORK);
#endif
#endif

	if (bcmtch_boot_flag & BCMTCH_BOOT_FLAG_SUSPEND_COLD_BOOT) {
		/* post boot reset */
		bcmtch_dev_post_boot_reset();

		ret_val = bcmtch_dev_set_power_state(BCMTCH_POWER_STATE_SLEEP);

		if (bcmtch_data_p->platform_data.bcmtch_on)
			bcmtch_data_p->platform_data.bcmtch_on(false);
	} else {
		/* suspend */
		ret_val = bcmtch_dev_request_power_mode(
					BCMTCH_POWER_MODE_NOWAKE,
					TOFE_COMMAND_POWER_MODE_SUSPEND);
	}

	/* clear events */
	bcmtch_dev_reset_events();

    /* unlock */
#if !BCMTCH_USE_BUS_LOCK
#if BCMTCH_USE_WORK_LOCK
	bcmtch_os_release_critical_section(BCMTCH_MUTEX_WORK);
#endif
#endif

	if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_PM) {
		printk(KERN_INFO
			"BCMTOUCH: %s() - complete : result = 0x%x\n",
			__func__,
			ret_val);
	}

	return ret_val;
}

static int32_t bcmtch_dev_resume(void)
{
	int32_t ret_val = BCMTCH_SUCCESS;

    /* lock */
#if !BCMTCH_USE_BUS_LOCK
#if BCMTCH_USE_WORK_LOCK
	bcmtch_os_lock_critical_section(BCMTCH_MUTEX_WORK);
#endif
#endif

	if (bcmtch_boot_flag & BCMTCH_BOOT_FLAG_SUSPEND_COLD_BOOT) {
		if (bcmtch_data_p->platform_data.bcmtch_on)
			bcmtch_data_p->platform_data.bcmtch_on(true);

		ret_val = bcmtch_dev_init();
	} else {
		if (!ret_val)
			ret_val = bcmtch_dev_request_power_mode(
						BCMTCH_POWER_MODE_WAKE,
						TOFE_COMMAND_NO_COMMAND);

		if (!ret_val)
			ret_val = bcmtch_dev_check_power_state(
						BCMTCH_POWER_STATE_ACTIVE,
						25);

		if (!ret_val)
			ret_val = bcmtch_dev_wait_for_firmware_ready(1000);

		if (!ret_val)
			ret_val = bcmtch_dev_watchdog_start();

		if (!ret_val)
			ret_val = bcmtch_os_interrupt_enable();
	}

	if (ret_val)
		printk(KERN_ERR
				"BCMTOUCH: %s() error rv=%d\n",
				__func__,
				ret_val);

    /* unlock */
#if !BCMTCH_USE_BUS_LOCK
#if BCMTCH_USE_WORK_LOCK
	bcmtch_os_release_critical_section(BCMTCH_MUTEX_WORK);
#endif
#endif

	if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_PM) {
		printk(KERN_INFO
			"BCMTOUCH: %s() - complete : result = 0x%x\n",
			__func__,
			ret_val);
	}

	return ret_val;
}

void bcmtch_dev_watchdog_work(unsigned long int data)
{
	if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_WATCH_DOG)
		printk(KERN_INFO
			"BCMTOUCH: WDOG\n");

	/* queue the interrupt handler */
	queue_work(
		bcmtch_data_p->p_workqueue,
		(struct work_struct *)&bcmtch_data_p->work);

	bcmtch_dev_watchdog_reset();
}

static int32_t bcmtch_dev_watchdog_start(void)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	init_timer(&bcmtch_data_p->watchdog);

	bcmtch_data_p->watchdog_expires =
			(bcmtch_data_p->post_boot_sections) ?
			MS_TO_JIFFIES(bcmtch_watchdog_post_boot) :
			MS_TO_JIFFIES(bcmtch_watchdog_normal);

	bcmtch_data_p->watchdog.function = bcmtch_dev_watchdog_work;
	bcmtch_data_p->watchdog.expires =
			jiffies + bcmtch_data_p->watchdog_expires;

	add_timer(&bcmtch_data_p->watchdog);

	return ret_val;
}

static int32_t bcmtch_dev_watchdog_restart(uint32_t expires)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	bcmtch_data_p->watchdog_expires = expires;

	mod_timer(
			&bcmtch_data_p->watchdog,
			(jiffies + bcmtch_data_p->watchdog_expires));

	return ret_val;
}

static int32_t bcmtch_dev_watchdog_reset(void)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	mod_timer(
			&bcmtch_data_p->watchdog,
			(jiffies + bcmtch_data_p->watchdog_expires));

	return ret_val;
}

static int32_t bcmtch_dev_watchdog_stop(void)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	if (bcmtch_data_p) {
		del_timer_sync(&bcmtch_data_p->watchdog);
		bcmtch_data_p->watchdog_expires = 0;
	}

	return ret_val;
}

/* -------------------------------------------------- */
/* - BCM Touch Controller Com(munication) Functions - */
/* -------------------------------------------------- */

static int32_t bcmtch_com_init(void)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	ret_val = bcmtch_com_write_spm(
				BCMTCH_SPM_REG_SPI_I2C_SEL,
				BCMTCH_IF_I2C_SEL);

	ret_val |= bcmtch_com_write_spm(
				BCMTCH_SPM_REG_I2CS_CHIPID,
				bcmtch_data_p->platform_data.i2c_addr_sys);

	return ret_val;
}

static int32_t bcmtch_com_read_spm(uint8_t reg, uint8_t *data)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	if (bcmtch_data_p) {
#if BCMTCH_USE_BUS_LOCK
		bcmtch_os_lock_critical_section(BCMTCH_MUTEX_BUS);
#endif
		ret_val = bcmtch_os_i2c_read_spm(
					bcmtch_data_p->p_i2c_client_spm,
					reg,
					data);

#if BCMTCH_USE_BUS_LOCK
		bcmtch_os_release_critical_section(BCMTCH_MUTEX_BUS);
#endif
	} else {
		printk(KERN_ERR
				"%s() error, bcmtch_data_p == NULL\n",
		       __func__);

		ret_val = -ENODATA;
	}

	return ret_val;
}

static int32_t bcmtch_com_write_spm(uint8_t reg, uint8_t data)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	if (bcmtch_data_p) {
#if BCMTCH_USE_BUS_LOCK
		bcmtch_os_lock_critical_section(BCMTCH_MUTEX_BUS);
#endif
		ret_val = bcmtch_os_i2c_write_spm(
					bcmtch_data_p->p_i2c_client_spm,
					reg,
					data);

#if BCMTCH_USE_BUS_LOCK
		bcmtch_os_release_critical_section(BCMTCH_MUTEX_BUS);
#endif
	} else {
		printk(KERN_ERR
				"%s() error, bcmtch_data_p == NULL\n",
		       __func__);
		ret_val = -ENODATA;
	}

	return ret_val;
}

static inline int32_t bcmtch_com_fast_write_spm(
							uint8_t count,
							uint8_t *regs,
							uint8_t *data)
{
	int32_t ret_val = BCMTCH_SUCCESS;

#if BCMTCH_USE_BUS_LOCK
	bcmtch_os_lock_critical_section(BCMTCH_MUTEX_BUS);
#endif
	ret_val = bcmtch_os_i2c_fast_write_spm(
				bcmtch_data_p->p_i2c_client_spm,
				count,
				regs,
				data);

#if BCMTCH_USE_BUS_LOCK
	bcmtch_os_release_critical_section(BCMTCH_MUTEX_BUS);
#endif

	return ret_val;
}

static int32_t bcmtch_com_read_sys(
						uint32_t sys_addr,
						uint16_t read_len,
						uint8_t *read_data)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	if (bcmtch_data_p) {
#if BCMTCH_USE_BUS_LOCK
		bcmtch_os_lock_critical_section(BCMTCH_MUTEX_BUS);
#endif
		ret_val = bcmtch_os_i2c_read_sys(
					bcmtch_data_p->p_i2c_client_sys,
					sys_addr,
					read_len,
					read_data);

#if BCMTCH_USE_BUS_LOCK
		bcmtch_os_release_critical_section(BCMTCH_MUTEX_BUS);
#endif
	} else {
		printk(KERN_ERR
				"%s() error, bcmtch_data_p == NULL\n",
		       __func__);

		ret_val = -ENODATA;
	}

	return ret_val;
}

static inline int32_t bcmtch_com_write_sys32(
						uint32_t sys_addr,
						uint32_t write_data)
{
	return bcmtch_com_write_sys(sys_addr, 4, (uint8_t *)&write_data);
}

static int32_t bcmtch_com_write_sys(
				uint32_t sys_addr,
				uint16_t write_len,
				uint8_t *write_data)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	if (bcmtch_data_p) {
#if BCMTCH_USE_BUS_LOCK
		bcmtch_os_lock_critical_section(BCMTCH_MUTEX_BUS);
#endif
		ret_val = bcmtch_os_i2c_write_sys(
					bcmtch_data_p->p_i2c_client_sys,
					sys_addr,
					write_len,
					write_data);
#if BCMTCH_USE_BUS_LOCK
		bcmtch_os_release_critical_section(BCMTCH_MUTEX_BUS);
#endif
	} else {
		printk(KERN_ERR
				"%s() error, bcmtch_data_p == NULL\n",
		       __func__);

		ret_val = -ENODATA;
	}

	return ret_val;
}

/* ------------------------------------- */
/* - BCM Touch Controller OS Functions - */
/* ------------------------------------- */
static void bcmtch_os_sleep_ms(uint32_t ms)
{
	msleep(ms);
}

static irqreturn_t bcmtch_os_interrupt_handler(int32_t irq, void *dev_id)
{
	if (bcmtch_data_p->p_i2c_client_spm->irq == irq) {

		/* track interrupts */
		bcmtch_data_p->irq_pending = true;

		if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_IRQ)
			printk(KERN_INFO
				"%s p=%d\n",
				__func__,
				bcmtch_data_p->irq_pending);

		/* queue the interrupt handler */
		queue_work(
			bcmtch_data_p->p_workqueue,
			(struct work_struct *)&bcmtch_data_p->work);

		/* reset watchdog */
		bcmtch_dev_watchdog_reset();

	} else {
		printk(KERN_ERR
				"%s : Error - IRQ Mismatch ? int=%d client_int=%d\n",
				__func__,
				irq,
				(bcmtch_data_p) ?
				bcmtch_data_p->p_i2c_client_spm->irq :
				0);
	}

	return IRQ_HANDLED;
}

static int32_t bcmtch_os_interrupt_enable(void)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	uint32_t irq;
	bcmtch_platform_data_t *p_data;

	p_data = &bcmtch_data_p->platform_data;

	if (gpio_is_valid(p_data->gpio_interrupt_pin)) {
		/* Reserve the irq line. */
		irq = gpio_to_irq(p_data->gpio_interrupt_pin);
		ret_val = request_irq(
					irq,
					bcmtch_os_interrupt_handler,
					p_data->gpio_interrupt_trigger,
					BCM15500_TSC_NAME,
					bcmtch_data_p);

		if (ret_val) {
			printk(KERN_ERR
					"ERROR: %s() - Unable to request interrupt irq %d\n",
					__func__,
					irq);

			/* note :
			* - polling is not enabled in this release
			* - it is an error if an irq is requested
			*	and not granted
			*/
		} else
			bcmtch_data_p->irq_enabled = true;
	}
	return ret_val;
}

static void bcmtch_os_interrupt_disable(void)
{
	int32_t pin;
	if (bcmtch_data_p && bcmtch_data_p->irq_enabled) {
		pin = bcmtch_data_p->platform_data.gpio_interrupt_pin;

		if (gpio_is_valid(pin)) {
			free_irq(gpio_to_irq(pin), bcmtch_data_p);
			bcmtch_data_p->irq_enabled = false;
		}
	}
}

static void *bcmtch_os_mem_alloc(uint32_t mem_size_req)
{
	return kzalloc(mem_size_req, GFP_KERNEL);
}

static void bcmtch_os_mem_free(void *mem_p)
{
	kfree(mem_p);
	mem_p = NULL;
}

static int32_t bcmtch_os_init_input_device(void)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	int32_t button_init = 0;

	if (bcmtch_data_p) {
		bcmtch_data_p->p_inputDevice = input_allocate_device();
		if (bcmtch_data_p->p_inputDevice) {

			bcmtch_data_p->p_inputDevice->name =
				"BCM15500 Touch Screen";

			bcmtch_data_p->p_inputDevice->phys = "I2C";
			bcmtch_data_p->p_inputDevice->id.bustype = BUS_I2C;
			bcmtch_data_p->p_inputDevice->id.vendor = 0x0A5C;
			bcmtch_data_p->p_inputDevice->id.product = 0x0020;
			bcmtch_data_p->p_inputDevice->id.version = 0x0000;

			set_bit(EV_SYN, bcmtch_data_p->p_inputDevice->evbit);
			set_bit(EV_ABS, bcmtch_data_p->p_inputDevice->evbit);
			__set_bit(
				INPUT_PROP_DIRECT,
				bcmtch_data_p->p_inputDevice->propbit);

			set_bit(EV_KEY, bcmtch_data_p->p_inputDevice->evbit);
			set_bit(
				BTN_TOUCH,
				bcmtch_data_p->p_inputDevice->keybit);

			while (button_init < bcmtch_data_p->
				  platform_data.ext_button_count) {
				set_bit(
					bcmtch_data_p->platform_data.
					  ext_button_map[button_init],
					bcmtch_data_p->p_inputDevice->keybit);
				button_init++;
			}

			input_set_abs_params(
				bcmtch_data_p->p_inputDevice,
				ABS_MT_POSITION_X,
				0,
				BCMTCH_MAX_X,
				0,
				0);

			input_set_abs_params(
				bcmtch_data_p->p_inputDevice,
				ABS_MT_POSITION_Y,
				0,
				BCMTCH_MAX_Y,
				0,
				0);

			if (bcmtch_event_flag &
					BCMTCH_EVENT_FLAG_TOUCH_SIZE) {
				input_set_abs_params(
					bcmtch_data_p->p_inputDevice,
					ABS_MT_TOUCH_MAJOR,
					0,
					BCMTCH_MAX_TOUCH_MAJOR,
					0,
					0);

				input_set_abs_params(
					bcmtch_data_p->p_inputDevice,
					ABS_MT_TOUCH_MINOR,
					0,
					BCMTCH_MAX_TOUCH_MINOR,
					0,
					0);
			}

			if (bcmtch_event_flag &
					BCMTCH_EVENT_FLAG_TOOL_SIZE) {
				input_set_abs_params(
					bcmtch_data_p->p_inputDevice,
					ABS_MT_WIDTH_MAJOR,
					0,
					BCMTCH_MAX_WIDTH_MAJOR,
					0,
					0);

				input_set_abs_params(
					bcmtch_data_p->p_inputDevice,
					ABS_MT_WIDTH_MINOR,
					0,
					BCMTCH_MAX_WIDTH_MINOR,
					0,
					0);
			}

			if (bcmtch_event_flag &
					BCMTCH_EVENT_FLAG_PRESSURE) {
				input_set_abs_params(
					bcmtch_data_p->p_inputDevice,
					ABS_MT_PRESSURE,
					0,
					BCMTCH_MAX_PRESSURE,
					0,
					0);
			}

			if (bcmtch_event_flag &
					BCMTCH_EVENT_FLAG_ORIENTATION) {
				input_set_abs_params(
					bcmtch_data_p->p_inputDevice,
					ABS_MT_ORIENTATION,
					BCMTCH_MIN_ORIENTATION,
					BCMTCH_MAX_ORIENTATION,
					0,
					0);
			}

#if BCMTCH_USE_PROTOCOL_B
			set_bit(
				BTN_TOOL_FINGER,
				bcmtch_data_p->p_inputDevice->keybit);

			input_mt_init_slots(
				bcmtch_data_p->p_inputDevice,
				BCMTCH_MAX_TOUCH);
#else
			input_set_abs_params(
				bcmtch_data_p->p_inputDevice,
				ABS_MT_TRACKING_ID,
				0,
				BCMTCH_MAX_TOUCH,
				0,
				0);
#endif

			/* request new os input queue size for this device */
			input_set_events_per_packet(
				bcmtch_data_p->p_inputDevice,
				50 * BCMTCH_MAX_TOUCH);

			/* register device */
			ret_val = input_register_device(
						bcmtch_data_p->p_inputDevice);

			if (ret_val) {
				printk(KERN_INFO
					"%s() Unable to register input device\n",
					__func__);

				input_free_device(bcmtch_data_p->p_inputDevice);
				bcmtch_data_p->p_inputDevice = NULL;
			}
		} else {
			printk(KERN_ERR
					"%s() Unable to create device\n",
			       __func__);

			ret_val = -ENODEV;
		}
	} else {
		printk(KERN_ERR
				"%s() error, driver data structure == NULL\n",
				__func__);

		ret_val = -ENODATA;
	}

	return ret_val;
}

static void bcmtch_os_free_input_device(void)
{
	if (bcmtch_data_p && bcmtch_data_p->p_inputDevice) {
		input_unregister_device(bcmtch_data_p->p_inputDevice);
		bcmtch_data_p->p_inputDevice = NULL;
	}
}

static int32_t bcmtch_os_init_critical_sections(void)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	if (!bcmtch_data_p) {
		printk(KERN_ERR
				"%s() error, driver data structure == NULL\n",
				__func__);

		ret_val = -ENODATA;
		return ret_val;
	}

	mutex_init(&bcmtch_data_p->cs_mutex[BCMTCH_MUTEX_BUS]);
	mutex_init(&bcmtch_data_p->cs_mutex[BCMTCH_MUTEX_WORK]);

	return ret_val;
}

static void bcmtch_os_lock_critical_section(uint8_t lock_mutex)
{
	mutex_lock(&bcmtch_data_p->cs_mutex[lock_mutex]);
}

static void bcmtch_os_release_critical_section(uint8_t lock_mutex)
{
	mutex_unlock(&bcmtch_data_p->cs_mutex[lock_mutex]);
}

static bool bcmtch_os_post_boot_download(
		bool irq_serviced)
{
	int32_t still_downloading;

	if (irq_serviced) {
		/* download shorter packet */
		still_downloading =
				bcmtch_dev_post_boot_download(
					bcmtch_post_boot_rate_low);
	} else {
		/* download larger packet */
		still_downloading =
				bcmtch_dev_post_boot_download(
					bcmtch_post_boot_rate_high);
	}

	if (!still_downloading) {
			if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_POST_BOOT)
				printk(KERN_ERR
						"%s() DOWNLOAD COMPLETE\n",
						__func__);

			/* don't want any stray interrupts */
			bcmtch_os_interrupt_disable();

			/* Send post boot init command */
			bcmtch_dev_write_rom_command(
					TOFE_COMMAND_INIT_POST_BOOT_PATCHES,
					bcmtch_data_p->postboot_cfg_addr,
					(uint16_t)
					bcmtch_data_p->postboot_cfg_length,
					0x0);

			/* Switch channel mode. */
			bcmtch_data_p->rom_channel = false;
			bcmtch_dev_reset_events();

			/* Wait for firmware reboot ready. */
			if (bcmtch_dev_wait_for_firmware_ready(1000)) {
				uint8_t xaddr = 0x40;
				uint8_t xdata;
				while (xaddr <= 0x61) {
					bcmtch_com_read_spm(xaddr, &xdata);
					printk(KERN_ERR
						"%s: addr = 0x%02x  data = 0x%02x\n",
						__func__,
						xaddr++,
						xdata);
				}
			}

			bcmtch_os_interrupt_enable();

			bcmtch_dev_watchdog_restart(
					MS_TO_JIFFIES(bcmtch_watchdog_normal));

			return true;
	}

	return false;
}

static void bcmtch_os_deferred_worker(struct work_struct *work)
{
	bool irq_serviced = false;
#if BCMTCH_USE_WORK_LOCK
	bcmtch_os_lock_critical_section(BCMTCH_MUTEX_WORK);
#endif

	if (bcmtch_data_p->irq_pending) {
		bcmtch_data_p->irq_pending = false;

		/* Process channels */
		bcmtch_dev_process();

		irq_serviced = true;
	}

	/* Amortized post boot download */
	if (bcmtch_data_p->post_boot_sections)
		bcmtch_os_post_boot_download(irq_serviced);

#if BCMTCH_USE_WORK_LOCK
	bcmtch_os_release_critical_section(BCMTCH_MUTEX_WORK);
#endif
}

static int32_t bcmtch_dev_post_boot_get_section(void)
{
	bcmtch_combi_entry_t *pb_entry = NULL;

	if (bcmtch_data_p->post_boot_sections &&
			bcmtch_data_p->post_boot_buffer) {
		pb_entry = (bcmtch_combi_entry_t *)
			bcmtch_data_p->post_boot_buffer;

		while (pb_entry[bcmtch_data_p->post_boot_section].length) {
			if (pb_entry[bcmtch_data_p->post_boot_section].flags &
					BCMTCH_FIRMWARE_FLAGS_POST_BOOT) {
				bcmtch_data_p->post_boot_data =
					bcmtch_data_p->post_boot_buffer
					+ pb_entry[bcmtch_data_p->
					post_boot_section].offset;

				bcmtch_data_p->post_boot_addr =
					pb_entry[bcmtch_data_p->
					post_boot_section].addr;

				bcmtch_data_p->post_boot_left =
					pb_entry[bcmtch_data_p->
					post_boot_section].length;

				break;
			} else {
				bcmtch_data_p->post_boot_section++;
			}
		}
	}

	return pb_entry[bcmtch_data_p->post_boot_section].length;
}

static int32_t bcmtch_dev_post_boot_download(int16_t data_rate)
{
	int32_t ret_val;
	int16_t write_length;

	bcmtch_combi_entry_t *pb_entry = NULL;

	if (bcmtch_data_p->post_boot_left)	{
		write_length = data_rate;

		if (bcmtch_data_p->post_boot_left < write_length)
			write_length = bcmtch_data_p->post_boot_left;

		ret_val = bcmtch_com_write_sys(
					bcmtch_data_p->post_boot_addr,
					write_length,
					bcmtch_data_p->post_boot_data);

		if (ret_val) {
			printk(KERN_ERR
					"%s() Error - did not download\n",
					__func__);
		} else {
			bcmtch_data_p->post_boot_addr += write_length;
			bcmtch_data_p->post_boot_left -= write_length;
			bcmtch_data_p->post_boot_data += write_length;
		}
	} else {
		printk(KERN_ERR
				"%s() Error - no bytes to download\n",
				__func__);
	}

	if (!bcmtch_data_p->post_boot_left) {

		pb_entry = (bcmtch_combi_entry_t *)
			bcmtch_data_p->post_boot_buffer;

		if (bcmtch_debug_flag & BCMTCH_DEBUG_FLAG_POST_BOOT)
			printk(KERN_INFO
					"%s() Section %d  Addr 0x%08x  Len %d\n",
					__func__,
					bcmtch_data_p->post_boot_section,
					pb_entry[bcmtch_data_p->
					post_boot_section].addr,
					pb_entry[bcmtch_data_p->
					post_boot_section].length);

		if (--bcmtch_data_p->post_boot_sections) {
			bcmtch_data_p->post_boot_section++;
			bcmtch_dev_post_boot_get_section();
		}
	}

	return bcmtch_data_p->post_boot_sections;
}

static int32_t bcmtch_os_init_deferred_worker(void)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	if (bcmtch_data_p) {
		bcmtch_data_p->p_workqueue = create_workqueue("bcmtch_wq");

		if (bcmtch_data_p->p_workqueue) {

			INIT_WORK(
				&bcmtch_data_p->work,
				bcmtch_os_deferred_worker);

		} else {
			printk(KERN_ERR
					"%s() Unable to create workqueue\n",
					__func__);

			ret_val = -ENOMEM;
		}
	} else {
		printk(KERN_ERR "%s() error, driver data structure == NULL\n",
		       __func__);
		ret_val = -ENODATA;
	}

	return ret_val;
}

static void bcmtch_os_clear_deferred_worker(void)
{
	if (bcmtch_data_p && bcmtch_data_p->p_workqueue)
		flush_workqueue(bcmtch_data_p->p_workqueue);
}

static void bcmtch_os_free_deferred_worker(void)
{
	if (bcmtch_data_p && bcmtch_data_p->p_workqueue) {
		cancel_work_sync(&bcmtch_data_p->work);
		destroy_workqueue(bcmtch_data_p->p_workqueue);

		bcmtch_data_p->p_workqueue = NULL;
	}
}

static void bcmtch_os_reset(void)
{
	if (bcmtch_data_p &&
		gpio_is_valid(bcmtch_data_p->platform_data.gpio_reset_pin)) {
		bcmtch_os_sleep_ms(
			bcmtch_data_p->platform_data.gpio_reset_time_ms);

		gpio_set_value(
			bcmtch_data_p->platform_data.gpio_reset_pin,
			bcmtch_data_p->platform_data.gpio_reset_polarity);

		bcmtch_os_sleep_ms(
			bcmtch_data_p->platform_data.gpio_reset_time_ms);

		gpio_set_value(
			bcmtch_data_p->platform_data.gpio_reset_pin,
			!bcmtch_data_p->platform_data.gpio_reset_polarity);

		bcmtch_os_sleep_ms(
			bcmtch_data_p->platform_data.gpio_reset_time_ms);
	}
}

static int32_t bcmtch_os_init_gpio(void)
{
	struct bcmtch_platform_data *p_platform_data;
	int32_t ret_val = BCMTCH_SUCCESS;

	if (!bcmtch_data_p) {
		printk(KERN_ERR
				"%s() error, driver data structure == NULL\n",
				__func__);

		ret_val = -ENODATA;
		return ret_val;
	}

	p_platform_data =
		(struct bcmtch_platform_data *)&bcmtch_data_p->platform_data;

    /*
     * setup a gpio pin for BCM Touch Controller reset function
     */
	if (gpio_is_valid(p_platform_data->gpio_reset_pin)) {
		ret_val = gpio_request(
					p_platform_data->gpio_reset_pin,
					"BCMTCH reset");

		if (ret_val < 0) {
			printk(KERN_ERR
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
			printk(KERN_ERR
					"ERROR: %s() - Unable to set reset pin %d\n",
					__func__,
					p_platform_data->gpio_reset_pin);

			/* note :
			* it is an error if a reset pin is requested
			* and not set --> return
			*/
			return ret_val;
		}
	} else {
		printk(KERN_INFO
				"%s() : no reset pin configured\n",
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
			printk(KERN_ERR
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
			printk(KERN_ERR
					"ERROR: %s() - Unable to set interrupt pin %d\n",
					__func__,
					p_platform_data->gpio_interrupt_pin);

			/* note :
			 * it is an error if a interrupt pin is requested
			 * and not set --> return
			 */
			return ret_val;
		}
	} else {
		printk(KERN_INFO
				"%s() : no interrupt pin configured\n",
				__func__);
	}

	return ret_val;
}

static void bcmtch_os_free_gpio(void)
{
	struct bcmtch_platform_data *p_platform_data;

	if (bcmtch_data_p) {

		p_platform_data =
			(struct bcmtch_platform_data *)
				&bcmtch_data_p->platform_data;

		if (gpio_is_valid(p_platform_data->gpio_reset_pin))
			gpio_free(p_platform_data->gpio_reset_pin);


		if (gpio_is_valid(p_platform_data->gpio_interrupt_pin))
			gpio_free(p_platform_data->gpio_interrupt_pin);

	}
}

static int32_t bcmtch_os_init_cli(struct device *p_device)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	ret_val = device_create_file(p_device, &bcmtch_cli_attr);

	return ret_val;
}

static void bcmtch_os_free_cli(struct device *p_device)
{
	device_remove_file(p_device, &bcmtch_cli_attr);
}

#ifdef CONFIG_PM
#ifndef CONFIG_HAS_EARLYSUSPEND
static int bcmtch_os_suspend(struct i2c_client *p_client, pm_message_t mesg)
{
	if (mesg.event == PM_EVENT_SUSPEND)
		bcmtch_dev_suspend();

	return 0;
}

static int bcmtch_os_resume(struct i2c_client *p_client)
{
	int rc = 0;

	bcmtch_dev_resume();

	return rc;
}
#endif
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void bcmtch_os_early_suspend(struct early_suspend *h)
{
	bcmtch_dev_suspend();
}

static void bcmtch_os_late_resume(struct early_suspend *h)
{
	bcmtch_dev_resume();
}

static struct early_suspend bcmtch_os_early_suspend_desc = {
	.level      = EARLY_SUSPEND_LEVEL_BLANK_SCREEN,
	.suspend    = bcmtch_os_early_suspend,
	.resume     = bcmtch_os_late_resume,
};
#endif

/* ----------------------------------------- */
/* - BCM Touch Controller OS I2C Functions - */
/* ----------------------------------------- */
static int32_t bcmtch_os_i2c_read_spm(
					struct i2c_client *p_i2c,
					uint8_t reg,
					uint8_t *data)
{
	int32_t ret_val = BCMTCH_SUCCESS;

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

static int32_t bcmtch_os_i2c_write_spm(
					struct i2c_client *p_i2c,
					uint8_t reg,
					uint8_t data)
{
	int32_t ret_val = BCMTCH_SUCCESS;

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

static int32_t bcmtch_os_i2c_fast_write_spm(
						struct i2c_client *p_i2c,
						uint8_t count,
						uint8_t *regs,
						uint8_t *data)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	/*
	 * support hard-coded for a max of 5 spm write messages
	 *
	 * - 1 i2c message uses 2 uint8_t buffers
	 *
	 */
	uint8_t buffer[10];	/* buffers for reg address and data */
	struct i2c_msg msg[5];
	uint32_t nMsg = 0;
	uint8_t *pBuf = buffer;

	/* setup I2C message for single byte write transaction */
	while (nMsg < count) {
		msg[nMsg].addr = p_i2c->addr;
		msg[nMsg].flags = 0;
		msg[nMsg].len = 2;
		msg[nMsg].buf = pBuf;

		*pBuf++ = regs[nMsg];
		*pBuf++ = data[nMsg];
		nMsg++;
	}

	if (i2c_transfer(p_i2c->adapter, msg, nMsg) != nMsg)
		ret_val = -EIO;

	return ret_val;
}

static int32_t bcmtch_os_i2c_read_sys(
						struct i2c_client *p_i2c,
						uint32_t sys_addr,
						uint16_t read_len,
						uint8_t *read_data)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	uint8_t dmaReg = BCMTCH_SPM_REG_DMA_RFIFO;
#if BCMTCH_USE_DMA_STATUS
	uint8_t statusReg = BCMTCH_SPM_REG_DMA_STATUS;
	uint8_t dmaStatus;
#endif

	/* setup the DMA header for this read transaction */
	uint8_t dmaHeader[8] = {
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


#if BCMTCH_USE_DMA_STATUS
	/* setup I2C messages for DMA read request transaction */
	struct i2c_msg dma_request[3] = {
		/* write DMA request header */
		{.addr = p_i2c->addr, .flags = 0, .len = 8, .buf = dmaHeader},

		/* write messages to read the DMA request status */
		{.addr = p_i2c->addr, .flags = 0, .len = 1, .buf = &statusReg},
		{.addr = p_i2c->addr, .flags = I2C_M_RD,
				.len = 1, .buf = &dmaStatus}
	};

	/* setup I2C messages for DMA read transaction */
	struct i2c_msg dma_read[2] = {
		/* next write messages to read the DMA request status */
		{.addr = p_i2c->addr, .flags = 0, .len = 1, .buf = &dmaReg},
		{.addr = p_i2c->addr, .flags = I2C_M_RD,
				.len = read_len, .buf = read_data}
	};

	/* send DMA request */
	if (i2c_transfer(p_i2c->adapter, dma_request, 3) != 3) {
		ret_val = -EIO;
	} else {
		while (dmaStatus != 1) {
			/* read status */
			if (i2c_transfer(p_i2c->adapter, &dma_request[1], 2)
					!= 2) {

				ret_val = -EIO;
				break;
			}
		}
	}

	if (dmaStatus) {
		/* read status */
		if (i2c_transfer(p_i2c->adapter, dma_read, 2) != 2)
			ret_val = -EIO;
	}
#else
	/* setup I2C messages for DMA read request transaction */
	struct i2c_msg dma_request[3] = {
		/* write DMA request header */
		{.addr = p_i2c->addr, .flags = 0, .len = 8, .buf = dmaHeader},

		/* next write messages to read the DMA request */
		{.addr = p_i2c->addr, .flags = 0, .len = 1, .buf = &dmaReg},
		{.addr = p_i2c->addr, .flags = I2C_M_RD,
				.len = read_len, .buf = read_data}
	};

	/* send complete DMA request */
	if (i2c_transfer(p_i2c->adapter, dma_request, 3) != 3)
		ret_val = -EIO;
#endif
	return ret_val;
}

static int32_t bcmtch_os_i2c_write_sys(
					struct i2c_client *p_i2c,
					uint32_t sys_addr,
					uint16_t write_len,
					uint8_t *write_data)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	uint16_t dmaLen = write_len + 1;
	uint8_t *dmaData = bcmtch_os_mem_alloc(dmaLen);

	/* setup the DMA header for this read transaction */
	uint8_t dmaHeader[8] = {
		/* set dma controller addr */
		BCMTCH_SPM_REG_DMA_ADDR,
		/* setup dma address */
		(sys_addr & 0xFF),
		((sys_addr & 0xFF00) >> 8),
		((sys_addr & 0xFF0000) >> 16),
		((sys_addr & 0xFF000000) >> 24),
		/* setup dma length */
		(write_len & 0xFF),
		((write_len & 0xFF00) >> 8),
		/* setup dma mode */
		BCMTCH_DMA_MODE_WRITE
	};

	/* setup I2C messages for DMA read request transaction */
	struct i2c_msg dma_request[2] = {
		/* write DMA request header */
		{.addr = p_i2c->addr, .flags = 0, .len = 8, .buf = dmaHeader},
		{.addr = p_i2c->addr, .flags = 0, .len = dmaLen, .buf = dmaData}
	};

	if (dmaData) {
		/* setup dma data buffer */
		dmaData[0] = BCMTCH_SPM_REG_DMA_WFIFO;
		memcpy(&dmaData[1], write_data, write_len);

		if (i2c_transfer(p_i2c->adapter, dma_request, 2) != 2)
			ret_val = -EIO;

		/* free dma buffer */
		bcmtch_os_mem_free(dmaData);
	} else {
		ret_val = -ENOMEM;
	}

	return ret_val;
}

static int32_t bcmtch_os_i2c_init_clients(struct i2c_client *p_i2c_client_spm)
{
	int32_t ret_val = BCMTCH_SUCCESS;
	struct i2c_client *p_i2c_client_sys;

	if (p_i2c_client_spm->adapter) {
		/* Configure the second I2C slave address. */
		p_i2c_client_sys =
			i2c_new_dummy(
				p_i2c_client_spm->adapter,
				bcmtch_data_p->platform_data.i2c_addr_sys);

		if (p_i2c_client_sys) {
			/* assign */
			bcmtch_data_p->p_i2c_client_spm = p_i2c_client_spm;
			bcmtch_data_p->p_i2c_client_sys = p_i2c_client_sys;
		} else {
			printk(KERN_ERR
				"%s() i2c_new_dummy == NULL, slave address: 0x%x\n",
				__func__,
				bcmtch_data_p->platform_data.i2c_addr_sys);

			ret_val = -ENODEV;
		}
	} else {
		printk(KERN_ERR
				"%s() p_i2c_adapter == NULL, adapter_id: 0x%x\n",
				__func__,
				bcmtch_data_p->platform_data.i2c_bus_id);

		ret_val = -ENODEV;
	}

	return ret_val;
}

static void bcmtch_os_i2c_free_clients(void)
{
	if (bcmtch_data_p && (bcmtch_data_p->p_i2c_client_sys)) {
		i2c_unregister_device(bcmtch_data_p->p_i2c_client_sys);
		bcmtch_data_p->p_i2c_client_sys = NULL;
	}
}

/* ------------------------------------------------- */
/* - BCM Touch Controller OS I2C Driver Structures - */
/* ------------------------------------------------- */
static const struct i2c_device_id bcmtch_i2c_id[] = {
	{BCM15500_TSC_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, bcmtch_i2c_id);

static struct i2c_driver bcmtch_i2c_driver = {
	.driver = {
		   .name = BCM15500_TSC_NAME,
		   .owner = THIS_MODULE,
		   },
	.probe = bcmtch_os_i2c_probe,
	.remove = bcmtch_os_i2c_remove,
	.id_table = bcmtch_i2c_id,

#ifdef CONFIG_PM
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = bcmtch_os_suspend,
	.resume = bcmtch_os_resume,
#endif
#endif
};

/* ------------------------------------------------ */
/* - BCM Touch Controller OS I2C Driver Functions - */
/* ------------------------------------------------ */
static int32_t bcmtch_os_i2c_probe(
					struct i2c_client *p_i2c_client,
					const struct i2c_device_id *id)
{
	int32_t ret_val = BCMTCH_SUCCESS;

	printk(KERN_INFO
		"BCMTCH: Driver: %s : %s : %s\n",
			BCMTCH_DRIVER_VERSION,
			__DATE__,
			__TIME__);

	/* print driver probe header */
	if (p_i2c_client)
		printk(KERN_INFO
				"BCMTCH: dev=%s addr=0x%x irq=%d\n",
				p_i2c_client->name,
				p_i2c_client->addr,
				p_i2c_client->irq);

	if (id)
		printk(KERN_INFO
				"BCMTCH: match id=%s\n",
				id->name);

	/* allocate global BCM Touch Controller driver structure */
	ret_val = bcmtch_dev_alloc();
	if (ret_val)
		goto probe_error;

	/* setup local platform data from client device structure */
	ret_val = bcmtch_dev_init_platform(&p_i2c_client->dev);
	if (ret_val)
		goto probe_error;

	/* setup the critical sections for concurrency */
	ret_val = bcmtch_os_init_critical_sections();
	if (ret_val)
		goto probe_error;

#if !BCMTCH_USE_BUS_LOCK
#if BCMTCH_USE_WORK_LOCK
	bcmtch_os_lock_critical_section(BCMTCH_MUTEX_WORK);
#endif
#endif

	/* initialize deferred worker (workqueue/tasklet/etc */
	ret_val = bcmtch_os_init_deferred_worker();
	if (ret_val)
		goto probe_error;


	if (bcmtch_data_p->platform_data.bcmtch_on)
		bcmtch_data_p->platform_data.bcmtch_on(true);

	/* setup the gpio pins
	 * - 1 gpio used for reset control signal to BCM Touch Controller
	 * - 1 gpio used as interrupt signal from BCM Touch Controller
	 */
	ret_val = bcmtch_os_init_gpio();
	if (ret_val)
		goto probe_error;


	/* setup the os input device*/
	ret_val = bcmtch_os_init_input_device();
	if (ret_val)
		goto probe_error;

	/* setup the os cli */
	ret_val = bcmtch_os_init_cli(&p_i2c_client->dev);
	if (ret_val)
		goto probe_error;

	/*
	* setup the i2c clients and bind (store pointers in global structure)
	* 1. SPM I2C client
	* 2. SYS I2C client
	*/
	ret_val = bcmtch_os_i2c_init_clients(p_i2c_client);
	if (ret_val)
		goto probe_error;

    /* reset the chip on driver load ? */
	if (bcmtch_boot_flag & BCMTCH_BOOT_FLAG_RESET_ON_LOAD_MASK) {
		if (bcmtch_boot_flag & BCMTCH_BOOT_FLAG_HARD_RESET_ON_LOAD)
			bcmtch_dev_reset(BCMTCH_RESET_MODE_HARD);
		else if (bcmtch_boot_flag & BCMTCH_BOOT_FLAG_SOFT_RESET_ON_LOAD)
			bcmtch_dev_reset(
				BCMTCH_RESET_MODE_SOFT_CHIP |
				BCMTCH_RESET_MODE_SOFT_ARM);
	}

	/* perform BCM Touch Controller initialization */
	ret_val = bcmtch_dev_init();
	if (ret_val)
		goto probe_error;

#ifdef CONFIG_HAS_EARLYSUSPEND
	register_early_suspend(&bcmtch_os_early_suspend_desc);
#endif /* CONFIG_HAS_EARLYSUSPEND */
	printk("BCMTCH: PROBE: success\n");

#if !BCMTCH_USE_BUS_LOCK
#if BCMTCH_USE_WORK_LOCK
	bcmtch_os_release_critical_section(BCMTCH_MUTEX_WORK);
#endif
#endif

	return BCMTCH_SUCCESS;

probe_error:
	printk(KERN_ERR "BCMTCH: PROBE: failure\n");

	bcmtch_os_i2c_remove(p_i2c_client);
	return ret_val;
}

static int32_t bcmtch_os_i2c_remove(struct i2c_client *p_i2c_client)
{
    /* disable interrupts */
	bcmtch_os_interrupt_disable();

	/* free watchdog timer */
	bcmtch_dev_watchdog_stop();

    /* free deferred worker (queue) */
	bcmtch_os_free_deferred_worker();

#if !BCMTCH_USE_BUS_LOCK
#if BCMTCH_USE_WORK_LOCK
	bcmtch_os_lock_critical_section(BCMTCH_MUTEX_WORK);
#endif
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&bcmtch_os_early_suspend_desc);
#endif /* CONFIG_HAS_EARLYSUSPEND */

    /* force chip to sleep before exiting */
	if (BCMTCH_POWER_STATE_SLEEP != bcmtch_dev_get_power_state())
		bcmtch_dev_set_power_state(BCMTCH_POWER_STATE_SLEEP);

	/* free communication channels */
	bcmtch_dev_free_channels();

	/* free i2c device clients */
	bcmtch_os_i2c_free_clients();

	/* remove the os cli */
	bcmtch_os_free_cli(&p_i2c_client->dev);

	/* free input device */
	bcmtch_os_free_input_device();

	/* free used gpio pins */
	bcmtch_os_free_gpio();

	/* free this mem last */
	bcmtch_dev_free();

	return BCMTCH_SUCCESS;
}

static int32_t __init bcmtch_os_i2c_init(void)
{
	return i2c_add_driver(&bcmtch_i2c_driver);
}
/* init early so consumer devices can complete system boot */
subsys_initcall(bcmtch_os_i2c_init);

static void __exit bcmtch_os_i2c_exit(void)
{
	i2c_del_driver(&bcmtch_i2c_driver);
}

module_exit(bcmtch_os_i2c_exit);

MODULE_DESCRIPTION("I2C support for BCM15500 Touchscreen");
MODULE_LICENSE("GPL");
MODULE_VERSION(BCMTCH_DRIVER_VERSION);
