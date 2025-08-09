/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * lxs_ts.h
 *
 * LXS touch core layer
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __LXS_TS_H
#define __LXS_TS_H

#include <linux/version.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/async.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/slab.h>
#include <linux/pm.h>
#include <linux/gpio.h>
#include <linux/string.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/kthread.h>
#include <linux/notifier.h>
#include <linux/atomic.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/uaccess.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/consumer.h>
#include <linux/pinctrl/pinctrl-state.h>
#include <linux/pinctrl/consumer.h>
#include <linux/completion.h>
#include <linux/errno.h>
#include <linux/linkage.h>
#include <linux/syscalls.h>
#include <linux/namei.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/workqueue.h>
#include <linux/firmware.h>
#include <linux/time.h>
#include <linux/notifier.h>
#include <linux/export.h>
#include <linux/dma-mapping.h>
#include <asm/page.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/memory.h>

#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/spi/spi.h>

#include <linux/fb.h>

#include <linux/pm_wakeup.h>

#include <linux/input.h>
#include <linux/input/mt.h>

#include <linux/errno.h>

#include <linux/drm_notify.h>
#include <linux/incell.h>

#ifndef __weak
#define __weak __attribute__((weak))
#endif

enum _LXS_TS_DBG_FLAG {
	DBG_NONE		= 0,
	DBG_BASE		= BIT(0),
	DBG_TRACE		= BIT(1),
	DBG_TRC_ERR		= BIT(2),
	DBG_RSVD03		= BIT(3),
	//
	DBG_EVENT		= BIT(4),
	DBG_EVENT_M		= BIT(5),
	DBG_RSVD06		= BIT(6),
	DBG_RSVD07		= BIT(7),
	//
	DBG_IRQ			= BIT(8),
	DBG_IRQ_STS		= BIT(9),
	DBG_RSVD10		= BIT(10),
	DBG_RSVD11		= BIT(11),
	//
	DBG_ABS			= BIT(12),
	DBG_EVT			= BIT(13),
	DBG_RSVD14		= BIT(14),
	DBG_RSVD15		= BIT(15),
	//
	DBG_FWUP		= BIT(16),
	DBG_FWUP_WR		= BIT(17),
	DBG_FWUP_DBG	= BIT(18),
	DBG_RSVD19		= BIT(19),
	//
	DBG_RSVD20		= BIT(20),
	DBG_RSVD21		= BIT(21),
	DBG_RSVD22		= BIT(22),
	DBG_RSVD23		= BIT(23),
	//
	DBG_RSVD24		= BIT(24),
	DBG_RSVD25		= BIT(25),
	DBG_RSVD26		= BIT(26),
	DBG_RSVD27		= BIT(27),
	//
	DBG_RSVD28		= BIT(28),
	DBG_RSVD29		= BIT(29),
	DBG_RSVD30		= BIT(30),
	DBG_RSVD31		= BIT(31),
};

extern u32 t_pr_dbg_mask;
extern u32 t_dev_dbg_mask;
extern u32 t_bus_dbg_mask;

#define LOG_SIG_INFO		"(I) "
#define LOG_SIG_NOTI		"(N) "
#define LOG_SIG_WARN		"(W) "
#define LOG_SIG_ERR			"(E) "
#define LOG_SIG_TRACE		"(T) "
#define LOG_SIG_DBG			"(D) "

#define t_pr_info(fmt, args...)		pr_info(fmt, ##args)
#define t_pr_noti(fmt, args...)		pr_notice(fmt, ##args)
#define t_pr_warn(fmt, args...)		pr_warning(fmt, ##args)
#define t_pr_err(fmt, args...)		pr_err(fmt, ##args)

#define t_pr_dbg(condition, fmt, args...)			\
		do {							\
			if (unlikely(t_pr_dbg_mask & (condition)))	\
				pr_info(fmt, ##args);	\
		} while (0)


#define __t_dev_none(_dev, fmt, args...)	\
		do{	\
			if (0) dev_printk(KERN_DEBUG, _dev, fmt, ##args);	\
		} while (0)

#define __t_dev_info(_dev, fmt, args...)	dev_info(_dev, fmt, ##args)
#define __t_dev_warn(_dev, fmt, args...)	dev_warn(_dev, fmt, ##args)
#define __t_dev_err(_dev, fmt, args...)		dev_err(_dev, fmt, ##args)
#define __t_dev_trace(_dev, fmt, args...)	dev_info(_dev, fmt, ##args)
#define __t_dev_dbg(_dev, fmt, args...)		dev_info(_dev, fmt, ##args)

#define __t_dev_warn_tag(_dev, fmt, args...)	__t_dev_warn(_dev, LOG_SIG_WARN fmt, ##args)
#define __t_dev_err_tag(_dev, fmt, args...)		__t_dev_err(_dev, LOG_SIG_ERR fmt, ##args)
#define __t_dev_trace_tag(_dev, fmt, args...)	__t_dev_trace(_dev, LOG_SIG_TRACE fmt, ##args)
#define __t_dev_dbg_tag(_dev, fmt, args...)		__t_dev_dbg(_dev, LOG_SIG_DBG fmt, ##args)

#define t_dev_info_once(_dev, fmt, args...)	\
		({						\
			static bool __dev_info_once = false;	\
			if (!__dev_info_once) {			\
				__dev_info_once = true;		\
				__t_dev_info(_dev, fmt, ##args);	\
			}					\
		})
#define t_dev_warn_once(_dev, fmt, args...)	\
		({						\
			static bool __dev_warn_once = false;	\
			if (!__dev_warn_once) { 		\
				__dev_warn_once = true; 	\
				__t_dev_warn_tag(_dev, fmt, ##args); \
			}					\
		})

#define t_dev_trace_once(_dev, fmt, args...)	\
		({						\
			static bool __dev_trace_once = false;	\
			if (!__dev_trace_once) {		\
				__dev_trace_once = true;	\
				__t_dev_trace_tag(_dev, fmt, ##args); \
			}					\
		})

#define t_dev_err_once(_dev, fmt, args...)	\
		({						\
			static bool __dev_err_once = false; \
			if (!__dev_err_once) {		\
				__dev_err_once = true;	\
				__t_dev_err_tag(_dev, fmt, ##args); \
			}					\
		})

#define t_dev_info(_dev, fmt, args...)		__t_dev_info(_dev, fmt, ##args)
#define t_dev_warn(_dev, fmt, args...)		__t_dev_warn_tag(_dev, fmt, ##args)

#define t_dev_trace(_dev, fmt, args...)		__t_dev_trace_tag(_dev, fmt, ##args)
#define t_dev_err(_dev, fmt, args...)		__t_dev_err_tag(_dev, fmt, ##args)

#define t_dev_info_sel(_dev, _prt, fmt, args...)	\
		do {	\
			if (_prt)	\
				__t_dev_info(_dev, fmt, ##args);	\
		} while (0)

#define t_dev_dbg(condition, _dev, fmt, args...)			\
		do {							\
			if (unlikely(t_dev_dbg_mask & (condition)))	\
				__t_dev_dbg_tag(_dev, fmt, ##args);	\
		} while (0)

#define t_dev_dbg_base(_dev, fmt, args...)	\
		t_dev_dbg(DBG_BASE, _dev, fmt, ##args)

#define t_dev_dbg_trace(_dev, fmt, args...)	\
		t_dev_dbg(DBG_TRACE, _dev, fmt, ##args)

#define t_dev_dbg_event(_dev, fmt, args...)	\
		t_dev_dbg(DBG_EVENT, _dev, fmt, ##args)

#define t_dev_dbg_event_m(_dev, fmt, args...)	\
		t_dev_dbg(DBG_EVENT_M, _dev, fmt, ##args)

#define t_dev_dbg_irq(_dev, fmt, args...)	\
		t_dev_dbg(DBG_IRQ, _dev, fmt, ##args)

#define t_dev_dbg_irq_sts(_dev, fmt, args...)	\
		t_dev_dbg(DBG_IRQ_STS, _dev, fmt, ##args)

#define t_dev_dbg_abs(_dev, fmt, args...)	\
		t_dev_dbg(DBG_ABS, _dev, fmt, ##args)

#define t_dev_dbg_evt(_dev, fmt, args...)	\
		t_dev_dbg(DBG_EVT, _dev, fmt, ##args)

#define t_dev_dbg_fwup(_dev, fmt, args...)	\
		t_dev_dbg(DBG_FWUP, _dev, fmt, ##args)

#define t_dev_dbg_fwup_wr(_dev, fmt, args...)	\
		t_dev_dbg(DBG_FWUP_WR, _dev, fmt, ##args)

#define t_dev_dbg_fwup_dbg(_dev, fmt, args...)	\
		t_dev_dbg(DBG_FWUP_DBG, _dev, fmt, ##args)

#define ETDBOOTFAIL			((ENOMEDIUM<<3) + 0x00)
#define ETDSENTESD			((ENOMEDIUM<<3) + 0x01)
#define ETDSENTESDIRQ		((ENOMEDIUM<<3) + 0x0F)

/*-----------------------------------------------------------------------------
 * Configuration
 *---------------------------------------------------------------------------*/
#if IS_ENABLED(CONFIG_I2C)
#define __LXS_SUPPORT_I2C
#endif

#if IS_ENABLED(CONFIG_SPI) || IS_ENABLED(CONFIG_SPI_MASTER)
#define __LXS_SUPPORT_SPI
#endif

#if IS_ENABLED(CONFIG_PINCTRL)
#define __LXS_SUPPORT_PINCTRL
#endif

#if IS_ENABLED(CONFIG_NET)
//#define __LXS_SUPPORT_ABT
#endif

//#define __LXS_SUPPORT_DEBUG_OPTION

#if IS_ENABLED(CONFIG_OF)
#define __LXS_CONFIG_OF
#endif

#if IS_ENABLED(CONFIG_FB)
//#define __LXS_CONFIG_FB
#endif

#define __LXS_SUPPORT_OLED

#define LXS_TS_NAME		"lxs_ts"
#define LXS_TS_INPUT	"lxs_ts_input"

#define MAX_FINGER			10

enum _LXS_CHIP_TYPE {
	CHIP_NONE		= 0x0000,
	//
	CHIP_SW82907	= 0x0897,
	CHIP_SW82908	= 0x0898,
	CHIP_SW82910	= 0x089A,
};

#define __LXS_SUPPORT_LPWG
//#define __LXS_SUPPORT_SLEEP

#define __LXS_SUPPORT_AUTO_START

//#define __LXS_SUPPORT_SYS_RESET

#define __LXS_SUPPORT_WATCHDOG

#define __LXS_SUPPORT_FILTER_CON

#define CONFIG_TOUCHSCREEN_LXS_SW82907

/*---------------------------------------------------------------------------*/

//#define __LXS_CUSTOM_VERSION

#define __LXS_BASE_VERSION		"0.01b-f1"

#if defined(__LXS_CUSTOM_VERSION)
#define __LXS_CUSTOM_PRJ				"PRJ"
#define __LXS_CUSTOM_PATCH_NO		"P001"

#define LXS_DRV_VERSION	\
		__LXS_BASE_VERSION	\
		"-" \
		__LXS_CUSTOM_PRJ	\
		"-"	\
		__LXS_CUSTOM_PATCH_NO
#else	/* !__LXS_CUSTOM_VERSION */
#define LXS_DRV_VERSION		__LXS_BASE_VERSION
#endif	/* __LXS_CUSTOM_VERSION */

/*---------------------------------------------------------------------------*/

#define ADDR_SKIP_MASK				(0xFFFF)

/*---------------------------------------------------------------------------*/

#define INFO_PTR					(0x022)

#define SPR_CHIP_TEST				(0x024)

#define SPR_CHIP_ID					(0x000)
#define SPR_RST_CTL					(0x005)
#define SPR_BOOT_CTL				(0x00F)
#define SPR_SRAM_CTL				(0x010)
#define SPR_BOOT_STS				(0x011)

#define SPR_CODE_OFFSET				(0x03C)

#define TC_IC_STATUS				(0x600)
#define TC_FW_STATUS				(0x601)
#define TC_VERSION					(0x642)
#define TC_PRODUCT_ID1				(0x644)
#define TC_PRODUCT_ID2				(0x645)

#define INFO_FPC_TYPE				(0x278)
#define INFO_WFR_TYPE				(0x27B)
#define INFO_CHIP_VERSION			(0x001)
#define INFO_CG_TYPE				(0x27D)
#define INFO_LOT_NUM				(0x27E)
#define INFO_SERIAL_NUM				(0x27F)
#define INFO_DATE					(0x280)
#define INFO_TIME					(0x281)

#define CODE_ACCESS_ADDR			(0xFD8)
#define DATA_BASE_ADDR				(0xFD1)
#define PRD_TCM_BASE_ADDR			(0x303)

#define TC_DEVICE_CTL				(0xC00)
#define TC_DRIVE_CTL				(0xC03)

#define TCI_FAIL_DEBUG_R			(0x28D)
#define TCI_FAIL_BIT_R				(0x28E)
#define TCI_DEBUG_R					(0x2AE)

#define TCI_FAIL_DEBUG_W			(0xC2C)
#define TCI_FAIL_BIT_W				(0xC2D)

#define TCI_DEBUG_FAIL_CTRL			(0xF5D)
#define TCI_DEBUG_FAIL_STATUS		(0xF5E)
#define TCI_DEBUG_FAIL_BUFFER		(0xF5F)

/* debug data report mode setting */
#define CMD_RAW_DATA_REPORT_MODE_READ	(0x2A4)
#define CMD_RAW_DATA_COMPRESS_WRITE		(0xC47)
#define CMD_RAW_DATA_REPORT_MODE_WRITE	(0xC49)

#define RES_INFO					(0xD00)
#define CHANNEL_INFO				(0xD01)
#define T_FRAME_RATE				(0xD03)
#define CHARGER_INFO				(0xD05)
#define FRAME_RATE					(0xD06)
#define GAMING_MODE					(0xD07)
#define GLOVE_MODE					(0xD08)
#define IME_STATE					(0xD09)
#define CALL_STATE					(0xD0A)
#define GRAB_MODE					(0xD0B)

#define DIM_SYS						(0xD30)
#define DIM_ENG						(0xD31)

/* production test */
#define TC_TSP_TEST_CTL				(0xC04)
#define TC_TSP_TEST_STS				(0x690)
#define TC_TSP_TEST_PF_RESULT		(0x691)
#define TC_TSP_TEST_OFF_INFO		(0x2FB)

/* Firmware control */
#define TC_FLASH_DN_STS				(0x247)
#define TC_CONFDN_BASE_ADDR			(0x2F9)
#define TC_FLASH_DN_CTL				(0xC05)

/* */
#define RAW_DATA_CTL_READ			(0x2A4)
#define RAW_DATA_CTL_WRITE			(0xC49)
#define SERIAL_DATA_OFFSET			(0x047)

#define PRD_SERIAL_TCM_OFFSET		(0x07C)
#define PRD_TC_MEM_SEL				(0x457)
#define PRD_TC_TEST_MODE_CTL		(0xC6E)
#define PRD_M1_M2_RAW_OFFSET		(0x287)
#define PRD_TUNE_RESULT_OFFSET		(0x289)
#define PRD_OPEN3_SHORT_OFFSET		(0x288)

#define PRD_IC_AIT_START_REG		(0xC8C)
#define PRD_IC_AIT_DATA_READYSTATUS	(0xC84)

struct lxs_hal_reg {
	u32 info_ptr;
	u32 spr_chip_test;
	u32 spr_chip_id;
	u32 spr_rst_ctl;
	u32 spr_boot_ctl;
	u32 spr_sram_ctl;
	u32 spr_boot_status;
	u32 spr_code_offset;
	u32 tc_ic_status;
	u32 tc_status;
	u32 tc_version;
	u32 tc_product_id1;
	u32 tc_product_id2;
	u32 info_fpc_type;
	u32 info_wfr_type;
	u32 info_chip_version;
	u32 info_cg_type;
	u32 info_lot_num;
	u32 info_serial_num;
	u32 info_date;
	u32 info_time;
	u32 code_access_addr;
	u32 data_base_addr;
	u32 prd_tcm_base_addr;
	u32 tc_device_ctl;
	u32 tc_drive_ctl;
	u32 tci_debug_fail_ctrl;
	u32 tci_debug_fail_buffer;
	u32 tci_debug_fail_status;
	u32 cmd_raw_data_report_mode_read;
	u32 cmd_raw_data_compress_write;
	u32 cmd_raw_data_report_mode_write;
	u32 res_info;
	u32 channel_info;
	u32 t_frame_rate;
	u32 charger_info;
	u32 frame_rate;
	u32 gaming_mode;
	u32 glove_mode;
	u32 ime_state;
	u32 call_state;
	u32 grab_mode;
	u32 dim_sys;
	u32 dim_eng;
	u32 tc_tsp_test_ctl;
	u32 tc_tsp_test_status;
	u32 tc_tsp_test_pf_result;
	u32 tc_tsp_test_off_info;
	u32 tc_flash_dn_status;
	u32 tc_confdn_base_addr;
	u32 tc_flash_dn_ctl;
	u32 raw_data_ctl_read;
	u32 raw_data_ctl_write;
	u32 serial_data_offset;
	/* */
	u32 prd_serial_tcm_offset;
	u32 prd_tc_mem_sel;
	u32 prd_tc_test_mode_ctl;
	u32 prd_m1_m2_raw_offset;
	u32 prd_tune_result_offset;
	u32 prd_open3_short_offset;
	u32 prd_ic_ait_start_reg;
	u32 prd_ic_ait_data_readystatus;
};

/* Reg. exchange */
struct lxs_ts_reg_quirk {
	u32 old_addr;
	u32 new_addr;
};

/*---------------------------------------------------------------------------*/

struct lxs_ts;

enum {
	TS_BUF_MARGIN		= 32,
	TS_MAX_BUF_SIZE		= (32<<10),	//(64<<10),
};

enum {
	TS_POWER_OFF = 0,
	TS_POWER_ON,
	TS_POWER_HW_RESET,
};

enum {
	TS_DEV_PM_RESUME = 0,
	TS_DEV_PM_SUSPEND,
	TS_DEV_PM_SUSPEND_IRQ,
};

enum {
	TS_MODE_RESUME = 0,
	TS_MODE_SUSPEND,
};

/* Deep Sleep or not */
enum {
	TS_IC_NORMAL = 0,
	TS_IC_SLEEP,
};

enum {
	TS_IME_OFF = 0,
	TS_IME_ON,
	TS_IME_SWYPE,
};

enum {
	TS_INCOMING_CALL_IDLE,
	TS_INCOMING_CALL_RINGING,
	TS_INCOMING_CALL_OFFHOOK,
};

enum {
	/* NP */
	TS_TOUCH_FRAME_60HZ = 1,	// 60Hz/60Hz (Active/Doze)
	TS_TOUCH_FRAME_120HZ,		// 120Hz/120Hz
	TS_TOUCH_FRAME_240HZ_H,		// 240Hz/120Hz
	TS_TOUCH_FRAME_240HZ,		// 240Hz/240Hz
	/* LP */
	TS_TOUCH_FRAME_L60HZ = 5,	// 60Hz/30Hz
	TS_TOUCH_FRAME_L120HZ,		// 120Hz/30Hz
};

enum {
	TS_DISP_FRAME_ASYNC = 0,
	TS_DISP_FRAME_30HZ,
	TS_DISP_FRAME_60HZ,
	TS_DISP_FRAME_120HZ,
	/* */
	TS_FRAME_TBL_SZ	= 16,
};

enum {
	TS_CORE_NONE = 0,
	TS_CORE_PROBE,
	TS_CORE_UPGRADE,
	TS_CORE_NORMAL,
};

// QCT USB connection
enum {
	TS_CONNECT_INVALID = 0,
	TS_CONNECT_SDP,
	TS_CONNECT_DCP,
	TS_CONNECT_CDP,
	TS_CONNECT_PROPRIETARY,
	TS_CONNECT_FLOATED,
	TS_CONNECT_HUB, /* SHOULD NOT change the value */
};

enum {
	TS_CONNECT_NONE		= 0,
	TS_CONNECT_USB		= 1,
	TS_CONNECT_DC		= 2,
	TS_CONNECT_OTG		= 3,
	/* */
	TS_CONNECT_WIRELESS	= 0x10,
};

enum {
	TS_EARJACK_NONE = 0,
	TS_EARJACK_NORMAL,
	TS_EARJACK_DEBUG,
};

enum {
	TS_DEBUG_TOOL_DISABLE = 0,
	TS_DEBUG_TOOL_ENABLE,
	TS_DEBUG_TOOL_MAX,
};

struct touch_data {
	u16 id;
	u16 x;
	u16 y;
	u16 width_major;
	u16 width_minor;
	s16 orientation;
	u16 pressure;
	/* finger, palm, pen, glove, hover */
	u16 type;
	u16 event;

	u16 saved_data_x;
	u16 saved_data_y;
	bool saved;
	bool report_flag;
};

struct lxs_ts_fquirks {	//function quirks
	void (*charger_mode)(struct lxs_ts *ts);
	/* */
	int (*gpio_init_reset)(struct lxs_ts *ts);
	void (*gpio_free_reset)(struct lxs_ts *ts);
	void (*gpio_set_reset)(struct lxs_ts *ts, int val);
	/* */
	int (*gpio_init_irq)(struct lxs_ts *ts);
	void (*gpio_free_irq)(struct lxs_ts *ts);
	void (*irq_control)(struct lxs_ts *ts, bool on);
	/* */
	int (*init_pre)(struct lxs_ts *ts);
	int (*init_post)(struct lxs_ts *ts);
	int (*init_end)(struct lxs_ts *ts);
	/* */
	void (*cancel_event_finger)(struct lxs_ts *ts);
	/* */
	int (*fwup_check)(struct lxs_ts *ts, u8 *fw_buf);
	int (*fwup_upgrade)(struct lxs_ts *ts, u8 *fw_buf, int fw_size, int retry);
	/* */
	void *sysfs_group;
};

enum {
	TS_IRQ_NONE			= 0,
	TS_IRQ_FINGER		= BIT(0),
	TS_IRQ_KNOCK		= BIT(1),
	TS_IRQ_GESTURE		= BIT(2),
	//
	TS_IRQ_ERROR			= BIT(31),
};

enum {
	TS_ABS_MODE = 0,
	TS_KNOCK,
	TS_SWIPE,
};

struct lxs_ts_entry_data {
	int chip_type;
	const struct of_device_id *of_match_table;
	char *chip_id;			//chip id(fixed)
	char *chip_name;		//chip name
	int mode_allowed;
	int fw_size;		//Pure F/W size, not include config data(1K)

	int max_finger;

	int bus_type;
	int buf_len;	/* custom buffer size, 0 for system default */
	/* */
	int chip_select;
	int spi_mode;
	int bits_per_word;
	int max_freq;
	/* */
	int	bus_tx_hdr_size;
	int	bus_rx_hdr_size;
	int bus_tx_dummy_size;
	int bus_rx_dummy_size;
	int bus_custom;

	const struct lxs_ts_reg_quirk *reg_quirks;

	const struct lxs_ts_fquirks fquirks;
};

enum {
	DEFAULT_NAME_SZ = PATH_MAX,
};

struct touch_event_param {
	char *name;
	int cmd;
	int min;
	int max;
	int is_key;
};

#if !defined(__LXS_SUPPORT_OLED)
#define __LXS_SUPPORT_STATUS_ERROR_CFG
#endif

struct lxs_hal_rw_multi {
	int wr;
	int addr;
	void *data;
	int size;
	char *name;
};

enum {
	NON_FAULT_INT 	= -1,
	NON_FAULT_U32	= ~0,
};

#define TC_PALM_DETECTED	1

#define TC_REPORT_BASE_PKT		(1)
#define TC_REPORT_BASE_HDR		(3)

/* report packet - type 1 */
struct lxs_hal_touch_data_type_1 {
	u32 track_id:5;
	u32 tool_type:3;
	u32 angle:8;
	u32 event:2;
	u32 x:14;

	u32 y:14;
	u32 pressure:8;
	u32 reserve1:10;

	u32 reserve2:4;
	u32 width_major:14;
	u32 width_minor:14;
} __packed;

/* report packet */
struct lxs_hal_touch_data {
	u8 tool_type:4;
	u8 event:4;
	u8 track_id;
	u16 x;
	u16 y;
	u8 pressure;
	u8 angle;
	u16 width_major;
	u16 width_minor;
} __packed;

struct lxs_hal_touch_info {
	u32 ic_status;
	u32 device_status;
	//
	u32 wakeup_type:8;
	u32 touch_cnt:5;
	u32 button_cnt:3;
	u32 palm_bit:16;
	//
	struct lxs_hal_touch_data data[MAX_FINGER];
} __packed;

struct lxs_hal_touch_info_a {
	u32 ic_status;
	u32 device_status;
	//
	u32 wakeup_type:8;
	u32 touch_cnt:5;
	u32 button_cnt:3;
	u32 pen_cnt:1;
	u32 rsvd1:7;
	u32 curr_mode:3;
	u32 rsvd2:4;
	u32 palm_bit:1;
	//
	struct lxs_hal_touch_data data[MAX_FINGER];
} __packed;

static inline u32 lxs_tc_sts_irq_type(int status)
{
	return ((status >> 16) & 0x0F);
}

static inline u32 lxs_tc_sts_running_sts(int status)
{
	return (status & 0x1F);
}

enum {
	TC_STS_IRQ_TYPE_INIT_DONE	= 2,
	TC_STS_IRQ_TYPE_ABNORMAL	= 3,
	TC_STS_IRQ_TYPE_DEBUG		= 4,
	TC_STS_IRQ_TYPE_REPORT		= 5,
};

#define PALM_ID					15

enum {
	TC_DRIVE_CTL_START		= (0x1<<0),
	TC_DRIVE_CTL_STOP		= (0x1<<1),
	TC_DRIVE_CTL_DISP_U0	= (0x0<<7),
	TC_DRIVE_CTL_DISP_U3	= (0x3<<7),
	/* */
	TC_DRIVE_CTL_MODE_VB	= (0x0<<2),
	TC_DRIVE_CTL_MODE_6LHB	= (0x1<<2),
	TC_DRIVE_CTL_MODE_DOZE	= (0x1<<4),
};

/* Clock type 4 */
#define LDO15_CTL_T4		(0x44C)
#define SYS_LDO_CTL_T4		(0x006)
#define SYS_OSC_CTL_T4		(0xFE1)

enum {
	TS_GET_CHIP_NAME	= BIT(0),
	TS_GET_VERSION		= BIT(1),
	TS_GET_REVISION		= BIT(2),
	TS_GET_PRODUCT		= BIT(3),
	/* */
	TS_GET_OPT1			= BIT(8),
	/* */
	TS_GET_VER_SIMPLE	= BIT(16),
	/* */
	TS_GET_ALL			= 0xFFFF,
};

#define LCD_TYPE_A_U3_FINGER	BIT(0)
#define LCD_TYPE_A_STOP			BIT(2)

#define TC_SW_RST_TYPE_NONE		0x0F
#define TC_SW_RST_TYPE_MAX		5

enum {
	TC_SW_RESET = 0,
	TC_HW_RESET_ASYNC,
	TC_HW_RESET_SYNC,
	//
	TC_HW_RESET_COND = 0x5A,
	//
	TC_HW_RESET_SKIP_QUIRK = 0x10,
};

enum {
	TOUCHSTS_IDLE = 0,
	TOUCHSTS_DOWN,
	TOUCHSTS_MOVE,
	TOUCHSTS_UP,
};

enum {
	LCD_MODE_U0	= 0,
	LCD_MODE_U3,
	LCD_MODE_STOP,
	LCD_MODE_MAX,
};

enum {
	LCD_MODE_BIT_U0 	= BIT(LCD_MODE_U0),
	LCD_MODE_BIT_U3		= BIT(LCD_MODE_U3),
	LCD_MODE_BIT_STOP	= BIT(LCD_MODE_STOP),
};

static const char *__lxs_lcd_driving_mode_strs[] = {
	[LCD_MODE_U0]	= "LP",
	[LCD_MODE_U3]	= "NP",
	[LCD_MODE_STOP]	= "STOP",
};

static inline const char *lxs_lcd_driving_mode_str(int mode)
{
	return (mode < LCD_MODE_MAX) ?
		__lxs_lcd_driving_mode_strs[mode] : "(invalid)";
}

enum {
	TC_IC_INIT_NEED = 0,
	TC_IC_INIT_DONE,
};

enum {
	TC_IC_BOOT_DONE = 0,
	TC_IC_BOOT_FAIL,
};

enum {
	BOOT_CHK_SKIP = (1<<16),
};

enum {
	BOOT_CHK_MODE_RETRY = 2,
	BOOT_CHK_STS_RETRY	= 2,
	/* */
	BOOT_CHK_MODE_DELAY	= 10,
	BOOT_CHK_STS_DELAY	= 10,
};

enum {
	IC_TEST_ADDR_NOT_VALID = 0x8000,
};

enum {
	IC_CHK_LOG_MAX		= (1<<9),
	//
	INT_IC_ABNORMAL_STATUS	= (1<<0),
	//
	INT_IC_ERROR_STATUS = ((1<<5) | (1<<3)),
};

enum {
	WAFER_TYPE_MASK = (0x07),
};

#if defined(__LXS_SUPPORT_OLED)

#define OLED_CFG_MAGIC_CODE			(0xCACACACA)
#define OLED_CFG_C_SIZE				(OLED_NUM_C_CONF<<OLED_POW_C_CONF)
#define OLED_CFG_S_SIZE				(1<<OLED_POW_S_CONF)

enum {
	OLED_E_FW_CODE_SIZE_ERR		= 1,
	OLED_E_FW_CODE_ONLY_VALID,
	OLED_E_FW_CODE_AND_CFG_VALID,
	OLED_E_FW_CODE_CFG_ERR,
};

#define GDMA_CTRL_READONLY		BIT(17)
#define GDMA_CTRL_EN			BIT(26)

#define __FC_CTRL_PAGE_ERASE	BIT(0)
#define __FC_CTRL_MASS_ERASE	BIT(1)
#define __FC_CTRL_WR_EN			BIT(2)

#define FC_CTRL_PAGE_ERASE		(__FC_CTRL_PAGE_ERASE | __FC_CTRL_WR_EN)
#define FC_CTRL_MASS_ERASE		(__FC_CTRL_MASS_ERASE | __FC_CTRL_WR_EN)
#define FC_CTRL_WR_EN			(__FC_CTRL_WR_EN)

#define BDMA_CTRL_EN			BIT(16)
#define BDMA_CTRL_BST			(0x001E0000)
#define BDMA_STS_TR_BUSY		BIT(6)
#endif	/* __LXS_SUPPORT_OLED */

#define FW_DN_LOG_UNIT			(8<<10)	//8K

#define MAX_RW_SIZE				(1<<10)	//1K
#define FLASH_CONF_SIZE			(1<<10)	//1K

#define FLASH_KEY_CODE_CMD		0xDFC1
#define FLASH_KEY_CONF_CMD		0xE87B
#define FLASH_BOOTCHK_VALUE		0x0A0A0000
#define FLASH_CODE_DNCHK_VALUE	0x42
#define FLASH_CONF_DNCHK_VALUE	0x84

#define FW_BOOT_LOADER_INIT		(0x74696E69)	//"init"
#define FW_BOOT_LOADER_CODE		(0x544F4F42)	//"BOOT"

#if defined(__LXS_FW_TYPE_1)
/*
 * Common CONF + Specific CONF(s)
 */
enum {
	POW_C_CONF = 9,
	POW_S_CONF = 10,
};

enum {
	NUM_C_CONF = 1,
	MIN_S_CONF = 1,
	MAX_S_CONF = 31,
};

enum {
	MIN_S_CONF_IDX = 1,
	MAX_S_CONF_IDX = (MAX_S_CONF + 1),
};

#define FW_BOOT_CODE_ADDR		(0x044)
#define FW_S_CONF_IDX_ADDR		(0x260)
#define FW_S_CONF_DN_ADDR		(0x267)

#define FW_TYPE_STR		"FW_TYPE_1"

#define FLASH_CONF_DNCHK_VALUE_TYPE_X	(FLASH_CONF_DNCHK_VALUE | 0x0C)

#define FLASH_CONF_SIZE_TYPE_X			(1<<POW_C_CONF)

#define S_CFG_DBG_IDX			0

#define S_CFG_FIX_IDX			0x80
#else	/* !__LXS_FW_TYPE_1 */
#define FW_TYPE_STR		"FW_TYPE_0"

#define FLASH_CONF_DNCHK_VALUE_TYPE_X	(FLASH_CONF_DNCHK_VALUE)
#define FLASH_CONF_SIZE_TYPE_X			FLASH_CONF_SIZE
#endif	/* __LXS_FW_TYPE_1 */

#define FW_POST_QUIRK_DELAY		20
#define FW_POST_QUIRK_COUNT		200

#define	FW_POST_DELAY			20
#define FW_POST_COUNT			200

#define	CONF_POST_DELAY			20
#define CONF_POST_COUNT			200

#if defined(__LXS_SUPPORT_OLED)
#ifdef FW_TYPE_STR
#undef FW_TYPE_STR
#define FW_TYPE_STR		"FW_TYPE_OLED"
#endif
//
#ifdef FLASH_CONF_DNCHK_VALUE_TYPE_X
#undef FLASH_CONF_DNCHK_VALUE_TYPE_X
#define FLASH_CONF_DNCHK_VALUE_TYPE_X	0
#endif
//
#ifdef FLASH_CONF_SIZE_TYPE_X
#undef FLASH_CONF_SIZE_TYPE_X
#define FLASH_CONF_SIZE_TYPE_X			0
#endif
//
#endif	/*__LXS_SUPPORT_OLED */

enum {
	BIN_CFG_OFFSET_POS = 0xE0,
	BIN_VER_OFFSET_POS = 0xE8,
	BIN_PID_OFFSET_POS = 0xF0,
};

enum corners {		/* portrait,		landscape,	seascape     */
    CORNER_0 = 0,	/* Upper left,		Bottom left,	Upper right  */
    CORNER_1 = 1,	/* Bottom left,  	Bottom right,	Upper left   */
    CORNER_2 = 2,	/* Upper right,		Upper left,	Bottom right */
    CORNER_3 = 3,	/* Bottom right,	Upper right,	Bottom left  */
};

enum range_changer {
    TARGET_LANDSCAPE_BOTTOM_LEFT = CORNER_0,
    TARGET_LANDSCAPE_BOTTOM_RIGHT = CORNER_1,
    TARGET_LANDSCAPE_UPPER_LEFT = CORNER_2,
    TARGET_LANDSCAPE_UPPER_RIGHT = CORNER_3,
    TARGET_PORTRAIT_BOTTOM_LEFT = 4,	/* Bottom left  */
    TARGET_PORTRAIT_BOTTOM_RIGHT = 5,	/* Bottom right */
};

struct lxs_hal_tc_version_bin {
	u8 major:4;
	u8 build:4;
	u8 minor;
	u16 rsvd:12;
	u16 ext:4;
} __packed;

struct lxs_hal_tc_version {
	u8 minor;
	u8 major:4;
	u8 build:4;
	u8 chip;
	u8 protocol:4;
	u8 ext:4;
} __packed;

struct lxs_hal_fw_info {
	u32 chip_id_raw;
	u8 chip_id[8];
	u32 sys_id_addr;
	u32 sys_id_raw;
	u8 sys_id[8];
	/* */
	union {
		struct lxs_hal_tc_version version;
		u32 version_raw;
	} v;
	/* */
	int invalid_pid;
	u8 product_id[8+4];
	u8 revision;
	u32 fpc;
	u32 wfr;
	u32 cg;
	u32 lot;
	u32 sn;
	u32 date;
	u32 time;
	u32 boot_code_addr;
	/* __LXS_FW_TYPE_OLED_BASE */
	int sizeof_flash;
	u32 gdma_saddr;
	u32 gdma_ctrl;
	u32 gdma_ctrl_en;
	u32 gdma_ctrl_ro;
	u32 gdma_start;
	u32 fc_ctrl;
	u32 fc_prot;
	u32 fc_ctrl_mass_erase;
	u32 fc_ctrl_wr_en;
	u32 fc_start;
	u32 fc_addr;
	u32 flash_status;
	u32 fc_erase_wait_cnt;
	u32 fc_erase_wait_time;
	u32 bdma_saddr;
	u32 bdma_daddr;
	u32 bdma_cal_op;
	u32 bdma_cal_op_ctrl;
	u32 bdma_ctrl;
	u32 bdma_ctrl_en;
	u32 bdma_ctrl_bst;
	u32 bdma_start;
	u32 bdma_sts;
	u32 bdma_sts_tr_busy;
	u32 datasram_addr;
	u32 gdma_crc_result;
	u32 gdma_crc_pass;
	u32 crc_fixed_value;
	u32 cfg_chip_id;
	u32 info_ptr;
};

static inline void lxs_hal_fw_set_chip_id(struct lxs_hal_fw_info *fw, u32 chip_id)
{
	fw->chip_id_raw = chip_id;
	memset(fw->chip_id, 0, sizeof(fw->chip_id));
	fw->chip_id[0] = (chip_id>>24) & 0xFF;
	fw->chip_id[1] = (chip_id>>16) & 0xFF;
	fw->chip_id[2] = (chip_id>>8) & 0xFF;
	fw->chip_id[3] = chip_id & 0xFF;
}

static inline void lxs_hal_fw_set_version(struct lxs_hal_fw_info *fw,
						u32 version)
{
	fw->v.version_raw = version;
}

static inline void lxs_hal_fw_set_revision(struct lxs_hal_fw_info *fw, u32 revision)
{
	fw->revision = revision & 0xFF;
}

#define PID_LEN_MAX	8
#define PID_LEN_MIN	7

#define PID_IN_RANGE(_val, _min, _max)	(((_val) >= (_min)) && ((_val) <= (_max)))

static inline int lxs_hal_fw_check_pid(char *pid)
{
	int len = strlen(pid);
	int invalid = 0;
	int i;
	char c;

	if (len > PID_LEN_MAX) {
		invalid |= BIT(9);
	} else if (len < PID_LEN_MIN) {
		invalid |= BIT(8);
	}

	for (i = 0; i < len; i++) {
		c = pid[i];

	#if 0
		if (isdigit(c)) {
			continue;
		}

		if (isalpha(c) && isupper(c)) {
			continue;
		}
	#else
		if (PID_IN_RANGE(c, '0', '9')) {
			continue;
		}

		if (PID_IN_RANGE(c, 'A', 'Z')) {
			continue;
		}
	#endif

	#if defined(__LXS_HAL_TEST_ALLOW_PID_UNDERBAR)
		if (c == '_') {
			continue;
		}
	#endif

		invalid |= BIT(i);
	}

	return invalid;
}

static inline void lxs_hal_fw_set_prod_id(struct lxs_hal_fw_info *fw, u8 *prod, u32 size)
{
	int len = min((int)sizeof(fw->product_id), (int)size);
	memset(fw->product_id, 0, sizeof(fw->product_id));
	memcpy(fw->product_id, prod, len);

	fw->invalid_pid = lxs_hal_fw_check_pid(fw->product_id);
}

static inline int lxs_hal_fw_ver_cmp(u32 bin_major, u32 bin_minor, u32 bin_build,
					u32 dev_major, u32 dev_minor, u32 dev_build)
{
	int update = 0;

	/* specific case for MP */
	switch (dev_major) {
	case 7:
	case 8:
		if (bin_major < 2) {
			update |= BIT(4);
			goto out;
		}
		break;
	}

	/* check major no. */
	if (bin_major < dev_major) {
		goto out;
	}

	if (bin_major > dev_major) {
		update |= BIT(1);
		goto out;
	}

	/* check minor no. */
	if (bin_minor < dev_minor) {
		goto out;
	}

	if (bin_minor > dev_minor) {
		update |= BIT(2);
		goto out;
	}

	/* check build no. */
	if (bin_build < dev_build) {
		goto out;
	}

	if (bin_build > dev_build) {
		update |= BIT(3);
		goto out;
	}

out:
	return update;
}

enum {
	CHIP_REPORT_NONE = 0,
	CHIP_REPORT_TYPE_0,
	CHIP_REPORT_TYPE_1,
};

enum {
	CHIP_STATUS_NONE = 0,
	CHIP_STATUS_TYPE_0,
	CHIP_STATUS_TYPE_1,
	CHIP_STATUS_TYPE_2,
};

enum {
	STS_ID_NONE = 0,
	STS_ID_VALID_DEV_CTL,
	STS_ID_VALID_CODE_CRC,
	STS_ID_VALID_CFG_CRC,
	STS_ID_RSVD4,
	STS_ID_ERROR_ABNORMAL,
	STS_ID_ERROR_SYSTEM,
	STS_ID_ERROR_MISMTACH,
	STS_ID_VALID_IRQ_PIN,
	STS_ID_VALID_IRQ_EN,
	STS_ID_ERROR_MEM,
	STS_ID_VALID_TC_DRV,
	STS_ID_ERROR_DISP,
};

enum {
	STS_POS_VALID_DEV_CTL			= 5,
	STS_POS_VALID_CODE_CRC			= 6,
	STS_POS_VALID_CFG_CRC			= 7,
	STS_POS_ERROR_ABNORMAL			= 9,
	STS_POS_ERROR_SYSTEM			= 10,
	STS_POS_ERROR_MISMTACH			= 13,
	STS_POS_VALID_IRQ_PIN			= 15,
	STS_POS_VALID_IRQ_EN			= 20,
	STS_POS_ERROR_MEM				= 21,
	STS_POS_VALID_TC_DRV			= 22,
	STS_POS_ERROR_DISP				= 31,
	/* */
	STS_POS_VALID_CODE_CRC_TYPE_0	= 22,
};

struct lxs_hal_status_mask_bit {
	u32 valid_dev_ctl;
	u32 valid_code_crc;
	u32 valid_cfg_crc;
	u32 error_abnormal;
	u32 error_system;
	u32 error_mismtach;
	u32 valid_irq_pin;
	u32 valid_irq_en;
	u32 error_mem;
	u32 valid_tv_drv;
	u32 error_disp;
};

struct lxs_hal_status_filter {
	int id;
	u32 width;
	u32 pos;
	u32 flag;
	const char *str;
};

#define _STS_FILTER(_id, _width, _pos, _flag, _str)	\
		{ .id = _id, .width = _width, .pos = _pos, .flag = _flag, .str = _str, }

enum {
	STS_FILTER_FLAG_TYPE_ERROR		= BIT(0),
	STS_FILTER_FLAG_ESD_SEND		= BIT(16),
	STS_FILTER_FLAG_CHK_FAULT		= BIT(17),
};

#define REG_BURST_MAX			512
#define REG_BURST_COL_PWR		4

#define REG_LOG_MAX		8
#define REG_DIR_NONE	0
#define REG_DIR_RD		1
#define REG_DIR_WR		2
#define REG_DIR_ERR		(1<<8)
#define REG_BURST		(1<<9)
#define REG_DIR_MASK	(REG_DIR_ERR-1)

struct lxs_hal_reg_log {
	int dir;
	u32 addr;
	u32 data;
};

struct lxs_ts_chip_opt {
	u32 f_flex_report:1;
	u32 f_glove_en:1;
	u32 f_grab_en:1;
	u32 f_dbg_report:1;
	u32 f_rsvd00:4;
	/* */
	u32 f_rsvd01:8;
	u32 f_rsvd02:8;
	u32 f_rsvd03:8;
	/* */
	u32 t_bus_opt:4;
	u32 t_boot_mode:4;
	u32 t_sts_mask:4;
	u32 t_sw_rst:4;
	u32 t_clock:4;
	u32 t_bin:4;
	u32 t_oled:4;
	u32 t_tc_cmd:4;
	/* */
	u32 t_tc_quirk:4;
	u32 rsvd10:4;
	u32 rsvd11:8;
	u32 rsvd12:8;
	u32 rsvd13:8;
} __packed;

struct lxs_ic_info_chip_proto {
	int chip_type;
	int vchip;
	int vproto;
};

struct lxs_hal_ops_quirk {	//quirk operation
	int (*hw_reset)(struct lxs_ts *ts, int pwr_con);
	/* */
	int (*sw_reset_post)(struct lxs_ts *ts);
	/* */
	int (*boot_result)(struct lxs_ts *ts, u32 *boot_st);
	/* */
	int (*irq_handler)(struct lxs_ts *ts);
};

struct lxs_hal_fwup_ops {
	int (*fwup_check)(struct lxs_ts *ts, u8 *fw_buf);
	int (*fwup_upgrade)(struct lxs_ts *ts, u8 *fw_buf, int fw_size, int retry);
};

struct lxs_hal_report_ops {
	char *abs_name;
	int (*abs_pre)(struct lxs_ts *ts);
	int (*abs_data)(struct lxs_ts *ts);
	/* */
	char *lpwg_name;
	int (*lpwg_data)(struct lxs_ts *ts);
};

#define TS_MAX_LPWG_CODE	2
#define TS_MAX_SWIPE_CODE	2

enum {
	TCON_GLOVE = 1,
	TCON_GRAB,
};

struct lxs_hal_lpwg_point {
	int x;
	int y;
};

struct lxs_ts_chip {
#if defined(__LXS_SUPPORT_RINFO)
	u32 iinfo_addr;
	u32 rinfo_addr;
	u32 rinfo_data[10];
	u32 rinfo_ok;
#endif
	u32 res_x;
	u32 res_y;
	u32 tx_count;
	u32 rx_count;

	int tc_cmd_table[LCD_MODE_MAX];
	struct lxs_ts_chip_opt opt;
	struct lxs_hal_reg reg_cur;
	struct lxs_hal_reg *reg;
	struct kobject kobj;
	/* */
	union {
		struct lxs_hal_touch_info_a info_a;
		struct lxs_hal_touch_info info;
	} info_grp;
	void *report_info;
	void *report_data;
	int report_size;
	int report_type;
	struct lxs_hal_report_ops report_ops;
	/* */
	struct lxs_hal_fw_info fw;
	/* */
	int status_type;
	u32 status_mask;
	u32 status_mask_normal;
	u32 status_mask_logging;
	u32 status_mask_reset;
	u32 status_mask_ic_normal;
	u32 status_mask_ic_abnormal;
	u32 status_mask_ic_error;
	u32 status_mask_ic_valid;
	u32 status_mask_ic_disp_err;
	u32 status_mask_ic_debug;
	struct lxs_hal_status_mask_bit status_mask_bit;
	struct lxs_hal_status_filter *status_filter;
	/* */
	int enable;
	int enable_lpm;
	int enable_knock;
	int enable_prox;
	int enable_sleep;
	int prev_lpm;
	int prev_knock;
	int prev_prox;
	int prev_sleep;
	struct lxs_hal_lpwg_point knock[TS_MAX_LPWG_CODE];
	struct lxs_hal_lpwg_point swipe[TS_MAX_SWIPE_CODE];
	int swipe_time;
	/* */
	int drv_reset_low;
	int drv_delay;
	int drv_opt_delay;
	int driving_ctrl;
	int prev_lcd_mode;
	int lcd_mode;
	int driving_mode;
	u32 charger;
	u32 earjack;
	atomic_t recur;
	atomic_t init;
	atomic_t boot;
	int boot_fail_cnt;
	volatile bool reset_is_on_going;
	int glove;
	int grab;
	int debug_tool;
	struct lxs_hal_reg_log reg_log[REG_LOG_MAX];
	int fw_abs_path;
	int fwup_status;
	struct lxs_hal_fwup_ops fwup_ops;
	/* */
	int sysfs_done;
	/* */
	struct lxs_hal_ops_quirk ops_quirk;
};

enum {
	FWUP_STATUS_OK			= 0,
	FWUP_STATUS_BUSY		= 1,
	FWUP_STATUS_NG_OP	 	= 2,
	FWUP_STATUS_NG_F_OPEN	= 3,
	FWUP_STATUS_NG_F_CHK	= 4,
	FWUP_STATUS_NG_CODE 	= 5,
	FWUP_STATUS_NG_CFG		= 6,
	FWUP_STATUS_NG_IO		= 9,
	FWUP_STATUS_MAX,
};

#define LXS_FWUP_STATUS_STR(_sts)	\
		[FWUP_STATUS_##_sts] = #_sts

static const char *__lxs_fwup_status_strs[] = {
	LXS_FWUP_STATUS_STR(OK),
	LXS_FWUP_STATUS_STR(BUSY),
	LXS_FWUP_STATUS_STR(NG_OP),
	LXS_FWUP_STATUS_STR(NG_F_OPEN),
	LXS_FWUP_STATUS_STR(NG_F_CHK),
	LXS_FWUP_STATUS_STR(NG_CODE),
	LXS_FWUP_STATUS_STR(NG_CFG),
	LXS_FWUP_STATUS_STR(NG_IO),
};

static inline const char *lxs_fwup_status_str(int status)
{
	const char *str = (status < FWUP_STATUS_MAX) ?
						__lxs_fwup_status_strs[status] : NULL;

	return (str) ? str : "(invalid)";
}

enum {
	BOOT_FAIL_RECOVERY_MAX = 3,	/* to avoid infinite repetition */
};

static inline int lxs_addr_is_skip(u32 addr)
{
	return (addr >= ADDR_SKIP_MASK);
}

static inline int lxs_addr_is_invalid(u32 addr)
{
	return (!addr || lxs_addr_is_skip(addr));
}

enum {
	BOOT_STS_POS_MODE = 0,
	BOOT_STS_POS_BUSY,
	BOOT_STS_POS_DUMP_DONE,
	BOOT_STS_POS_DUMP_ERR,
	BOOT_STS_POS_MAGIC_ERR,
};

#define HAL_ACCESS_CHK_SKIP_SLEEP	BIT(3)
#define HAL_ACCESS_CHK_SKIP_FB		BIT(2)
#define HAL_ACCESS_CHK_SKIP_PM		BIT(1)
#define HAL_ACCESS_CHK_SKIP_INIT	BIT(0)

enum {
	EQ_COND = 0,
	NOT_COND,
};

#define LXS_TS_GPIO_RST		"lxs_gpio_reset"
#define LXS_TS_GPIO_IRQ		"lxs_gpio_irq"

#define LXS_BUS_TAG 		"bus: "
#define LXS_BUS_TAG_ERR 	"bus(E): "
#define LXS_BUS_TAG_WARN	"bus(W): "
#define LXS_BUS_TAG_DBG		"bus(D): "

#define t_bus_info(_dev, fmt, args...)	\
		__t_dev_info(_dev, LXS_BUS_TAG fmt, ##args)

#define t_bus_warn(_abt, fmt, args...)	\
		__t_dev_warn(_dev, LXS_BUS_TAG_WARN fmt, ##args)

#define t_bus_err(_dev, fmt, args...)	\
		__t_dev_err(_dev, LXS_BUS_TAG_ERR fmt, ##args)

#define t_bus_dbg(condition, _dev, fmt, args...)			\
		do {							\
			if (unlikely(t_bus_dbg_mask & (condition)))	\
				__t_dev_info(_dev, LXS_BUS_TAG_DBG fmt, ##args);	\
		} while (0)

#define t_bus_dbg_base(_dev, fmt, args...)	\
		t_bus_dbg(DBG_BASE, _dev, fmt, ##args)

#define t_bus_dbg_trace(_dev, fmt, args...)	\
		t_bus_dbg(DBG_TRACE, _dev, fmt, ##args)

struct lxs_ts_bus_msg {
	int tx_size;
	char *tx_buf;
	int rx_size;
	char *rx_buf;
};

#if defined(__LXS_SUPPORT_WATCHDOG)
struct lxs_ts_watchdog {
	bool support;
	int delay_ms;
};
#endif

struct lxs_ts_plat_data {
	int reset_pin;
	int irq_pin;

	u32 max_x;
	u32 max_y;
	int mt_tool_max;

	u32 area_indicator;
	u32 area_navigation;
	u32 area_edge;

	/* use option */
	bool use_pen;
	bool use_lpwg;
	bool use_firmware;
	bool use_fw_upgrade;
	bool use_fw_pwr_rst;
	bool use_fw_ver_diff;
	bool use_fw_skip_pid;
	bool use_irq_verify;
	bool use_skip_reset;
	bool use_input_parent;
	bool use_tc_vblank;
	bool use_tc_doze;
	u32 use_palm_opt;
	u32 use_charger_opt;

	u32 frame_tbl_type;

	const char *fw_name;
	const char *panel_spec_path;
	const char *self_test_path;

#if defined(__LXS_SUPPORT_WATCHDOG)
	struct lxs_ts_watchdog watchdog;
#endif

	/*
	 * PT cmp boundary
	 * [0]:min, [1]:max
	 */
	u32 pt_open_cmp[2];
	u32 pt_short_gnd_cmp[2];
	u32 pt_short_trx_cmp[2];
	u32 pt_short_vdd_cmp[2];
	u32 pt_sync_v_cmp[2];	/* v-sync */
};

struct after_probe_work_t {
	struct delayed_work start;
	bool err;
	int retry;
};

struct lxs_ts {
	int idx;

//	struct platform_device *pdev;

	struct lxs_ts_plat_data *plat_data;

	int chip_type;

	atomic_t lock_cnt;

	char chip_id[128];
	char chip_name[128];
	char input_name[128];
	char input_phys[128];
	char irq_name[128];
	int max_finger;
	u32 mode_allowed;

	int irq;
	unsigned long irqflags;

	void *bus_priv;				/* __LXS_SUPPORT_PM_QUIRK */

	struct device *dev;			/* client device : i2c->dev or spi->dev */
	struct lxs_ts_chip chip;
	u32 fw_size;

	struct input_dev *input;

	struct kobject kobj;

	int state_core;
	int state_pm;
	int state_mode;
	int state_sleep;
	int state_irq_enable;
	int state_connect; /* connection using USB port */
	int state_wireless; /* connection using wirelees_charger */
	int state_ime;
	int state_incoming_call;

	int d_frame_rate_np;
	int d_frame_rate_lp;
	int d_frame_rate_off;
	int d_frame_rate_aod;

	int d_frame_tbl_np[TS_FRAME_TBL_SZ];
	int d_frame_tbl_lp[TS_FRAME_TBL_SZ];

	int t_frame_rate_np;
	int t_frame_rate_lp;

	int t_frame_tbl_np[TS_FRAME_TBL_SZ];
	int t_frame_tbl_lp[TS_FRAME_TBL_SZ];

	int suspend_is_on_going;

	int hw_reset_delay;
	int sw_reset_delay;

	struct regulator *avdd;
	struct regulator *dvdd;
	int power_enabled;

	struct pinctrl *pinctrl;
	struct pinctrl_state *pinctrl_state_on;
	struct pinctrl_state *pinctrl_state_off;

	u32 intr_palm;
	u32 intr_status;
	u32 intr_gesture;
	u16 event_cnt_finger;
	/* */
	u32 new_mask;
	u32 old_mask;
	int tcount;
	struct touch_data tdata[MAX_FINGER];
	int is_cancel;

	int mc[MAX_FINGER];
	int pre_x[MAX_FINGER];
	int pre_y[MAX_FINGER];

	char test_fwpath[PATH_MAX];
	u32 force_fwup;

	struct mutex lock;
	struct mutex bus_lock;
	struct mutex reset_lock;
	struct workqueue_struct *wq;
	struct after_probe_work_t after_probe;
	struct delayed_work init_work;
	struct delayed_work upgrade_work;
	struct delayed_work sys_reset_work;
	struct delayed_work enable_work;

	int mode_async;

#if defined(__LXS_SUPPORT_WATCHDOG)
	struct mutex watchdog_lock;
	struct delayed_work watchdog_work;
	int watchdog_set;
	int watchdog_run;
	int watchdog_time;
#endif

	struct completion resume_done;

#if defined(__LXS_CONFIG_FB)
	struct notifier_block fb_notif;
#endif	/* __LXS_CONFIG_FB */

	volatile bool shutdown_called;

	/* */
#if defined(__LXS_SUPPORT_I2C)
	struct i2c_msg msgs_rx[2];
	struct i2c_msg msgs_tx[2];
#endif

#if defined(__LXS_SUPPORT_SPI)
	struct spi_transfer x_rx;
	struct spi_transfer x_tx;
	struct spi_message m_rx;
	struct spi_message m_tx;
#endif

	int buf_size;
	struct lxs_ts_bus_msg rd_msg;
	struct lxs_ts_bus_msg wr_msg;

	int bus_type;
	int buf_len;
	int chip_select;
	int spi_mode;
	int bits_per_word;
	int max_freq;
	int bus_tx_hdr_size;
	int bus_rx_hdr_size;
	int bus_tx_dummy_size;
	int bus_rx_dummy_size;
	int (*bus_read)(struct lxs_ts *ts);
	int (*bus_write)(struct lxs_ts *ts);

	struct lxs_ts_reg_quirk *reg_quirks;

	struct lxs_ts_fquirks *fquirks;

	/* */
	int is_charger;

	int probe_init;
	int probe_done;

	/* */
	void *prd;
	u32 prd_quirks;

	void *abt;
	u32 abt_quirks;

	struct notifier_block drm_notif;
	int after_probe_retry;

	/* For grip rejection */
	bool landscape;
	int rejection_mode;
	bool report_rejected_event_flag;

	u32 portrait_buffer[4];
	u32 landscape_buffer[4];
	u32 radius_portrait[2];
	u32 radius_landscape[4];

	u32 circle_range_p[2];
	u32 circle_range_l[4];

#if defined(__LXS_SUPPORT_FILTER_CON)
	int blend_filter;
	int stop_filter;
	int wet_mode;
#endif
};

enum {
	PRD_QUIRK_RAW_RETURN_MODE_VAL	= BIT(0),
	/* */
	ABT_QUIRK_RAW_RETURN_MODE_VAL	= BIT(0),
};

/* ts->force_fwup */
enum {
	TS_FORCE_FWUP_CLEAR		= 0,
	TS_FORCE_FWUP_ON		= BIT(0),
	TS_FORCE_FWUP_SYS_SHOW	= BIT(2),
	TS_FORCE_FWUP_SYS_STORE	= BIT(3),
	/* */
	TS_FORCE_FWUP_SKIP_PID	= BIT(7),
};

#ifndef __BIN_ATTR	/* for low version compatibility */
#define __BIN_ATTR(_name, _mode, _read, _write, _size) {		\
	.attr = { .name = __stringify(_name), .mode = _mode },		\
	.read   = _read,											\
	.write  = _write,											\
	.size   = _size,											\
}
#endif

struct lxs_ts_attribute {
	struct attribute attr;
	ssize_t (*show)(struct device *dev, char *buf);
	ssize_t (*store)(struct device *dev, const char *buf, size_t count);
};

#define TS_ATTR(_name, _show, _store)		\
		struct lxs_ts_attribute ts_attr_##_name	\
			= __ATTR(_name, 0664, _show, _store)

#define TS_BIN_ATTR(_name, _show, _store, _size)	\
		struct bin_attribute ts_bin_attr_##_name	\
			= __BIN_ATTR(_name, 0664, _show, _store, _size)

static inline int touch_lookup_frame_tbl(int *table, int value)
{
	int val = 0;
	int i;

	if (table == NULL)
		return -1;

	for (i = 0; i < TS_FRAME_TBL_SZ; i++) {
		val = *table++;
		if (value == val)
			return val;
		if (val < 0)
			break;
	}
	return -1;
}

static inline u32 touch_mode_allowed(struct lxs_ts *ts, u32 mode)
{
	return (ts->mode_allowed & BIT(mode));
}

static inline u32 touch_mode_not_allowed(struct lxs_ts *ts, u32 mode)
{
	int ret;

	ret = !touch_mode_allowed(ts, mode);
	if (ret) {
		t_dev_warn(ts->dev, "target mode(%d) not supported\n", mode);
	}

	return ret;
}

static inline void lxs_ts_delay(unsigned int msecs)
{
	if (!msecs)
		return;

	if (msecs >= 20)
		msleep(msecs);
	else
		usleep_range(msecs * 1000, msecs * 1000);
}

static inline u32 lxs_report_ic_status(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;

	return chip->info_grp.info.ic_status;
}

static inline u32 lxs_report_tc_status(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;

	return chip->info_grp.info.device_status;
}

static inline void *lxs_report_info(struct lxs_ts *ts)
{
	return ts->chip.report_info;
}

static inline int lxs_report_size(struct lxs_ts *ts)
{
	return ts->chip.report_size;
}

static inline void *lxs_report_data(struct lxs_ts *ts)
{
	return ts->chip.report_data;
}

static inline u32 lxs_report_info_wakeup_type(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;

	return chip->info_grp.info.wakeup_type;
}

static inline u32 lxs_report_info_touch_cnt(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;

	return chip->info_grp.info.touch_cnt;
}

static inline int lxs_hal_get_fwup_status(struct lxs_ts *ts)
{
	return ts->chip.fwup_status;
}

static inline void lxs_hal_set_fwup_status(struct lxs_ts *ts, int status)
{
	const char *str = lxs_fwup_status_str(status);

	t_dev_info(ts->dev, "FW upgrade: status %d(%s)\n", status, str);

	ts->chip.fwup_status = status;
}

static inline u32 lxs_hal_boot_sts_pos_busy(struct lxs_ts *ts)
{
	u32 pos = BOOT_STS_POS_BUSY;

	switch (ts->chip.opt.t_boot_mode) {
	case 2:
	case 1:
		pos = 0;
		break;
	}

	return pos;
}

static inline u32 lxs_hal_boot_sts_pos_dump_err(struct lxs_ts *ts)
{
	u32 pos = BOOT_STS_POS_DUMP_ERR;

	switch (ts->chip.opt.t_boot_mode) {
	case 2:
	case 1:
		pos = 2;
		break;
	}

	return pos;
}

static inline u32 lxs_hal_boot_sts_mask_empty(struct lxs_ts *ts)
{
	u32 mask = 0;

	switch (ts->chip.opt.t_boot_mode) {
	case 2:
		mask = BIT(6);
		break;
	}

	return mask;
}

#define __lxs_snprintf(_buf, _buf_max, _size, _fmt, _args...) \
		({	\
			int _n_size = 0;	\
			if (_size < _buf_max)	\
				_n_size = snprintf(_buf + _size, _buf_max - _size,\
								(const char *)_fmt, ##_args);	\
			_n_size;	\
		})


#define lxs_snprintf(_buf, _size, _fmt, _args...) \
		__lxs_snprintf(_buf, PAGE_SIZE, _size, _fmt, ##_args)

/*
 * Bus control
 */
enum {
	I2C_BUS_TX_HDR_SZ = 2,
	I2C_BUS_RX_HDR_SZ = 0,
	I2C_BUS_TX_DUMMY_SZ = 0,
	I2C_BUS_RX_DUMMY_SZ = 0,
};

enum {
	SPI_BUS_RX_HDR_SZ_16BIT = 4,
	SPI_BUS_RX_DUMMY_SZ_16BIT = 2,
	/* */
	SPI_BUS_RX_HDR_SZ_32BIT = (4+2),
	SPI_BUS_RX_DUMMY_SZ_32BIT = 4,
	/* */
	SPI_BUS_RX_HDR_SZ_128BIT = (16+2),
	SPI_BUS_RX_DUMMY_SZ_128BIT = 16,
	/* */
	SPI_BUS_TX_HDR_SZ = 2,
	SPI_BUS_RX_HDR_SZ = SPI_BUS_RX_HDR_SZ_32BIT,
	SPI_BUS_TX_DUMMY_SZ = 0,
	SPI_BUS_RX_DUMMY_SZ = SPI_BUS_RX_DUMMY_SZ_32BIT,
};

#define __CLOCK_KHZ(x)		((x) * 1000)
#define __CLOCK_MHZ(x)		((x) * 1000 * 1000)

static inline int spi_freq_out_of_range(u32 freq)
{
	return ((freq > __CLOCK_MHZ(10)) || (freq < __CLOCK_MHZ(1)));
}

static inline u32 freq_to_mhz_unit(u32 freq)
{
	return (freq / __CLOCK_MHZ(1));
}

static inline u32 freq_to_khz_unit(u32 freq)
{
	return (freq / __CLOCK_KHZ(1));
}

static inline u32 freq_to_khz_top(u32 freq)
{
	return ((freq % __CLOCK_MHZ(1)) / __CLOCK_KHZ(100));
}

struct lxs_ts_if_driver {
	union {
		struct i2c_driver i2c_drv;
		struct spi_driver spi_drv;
	} bus;
	const struct lxs_ts_entry_data *entry_data;
	int idx;
};

/*
 * Input control
 */
#define FINGER_MAX_PRESSURE		255
#define FINGER_MAX_WIDTH		15
#define FINGER_MAX_ORIENT		90

#if IS_ENABLED(CONFIG_ANDROID)
#define __LXS_CONFIG_INPUT_ANDROID
#endif	/* CONFIG_ANDROID */

/* HAL layer */
extern int lxs_hal_get_boot_result(struct lxs_ts *ts, u32 *boot_st);
extern int lxs_hal_read_value(struct lxs_ts *ts, u32 addr, u32 *value);
extern int lxs_hal_write_value(struct lxs_ts *ts, u32 addr, u32 value);
extern int lxs_hal_reg_read(struct lxs_ts *ts, u32 addr, void *data, int size);
extern int lxs_hal_reg_write(struct lxs_ts *ts, u32 addr, void *data, int size);
extern int lxs_hal_reg_read_single(struct lxs_ts *ts, u32 addr, void *data, int size);
extern int lxs_hal_reg_write_single(struct lxs_ts *ts, u32 addr, void *data, int size);
extern int lxs_hal_reg_rw_multi(struct lxs_ts *ts, struct lxs_hal_rw_multi *multi, char *title);
extern int lxs_hal_reg_bit_chg(struct lxs_ts *ts, u32 addr, u32 *value, u32 mask);
extern int lxs_hal_reg_bit_set(struct lxs_ts *ts, u32 addr, u32 *value, u32 mask);
extern int lxs_hal_reg_bit_clr(struct lxs_ts *ts, u32 addr, u32 *value, u32 mask);
extern int lxs_hal_access_not_allowed(struct lxs_ts *ts, char *title, int skip_flag);

extern int lxs_hal_ic_info(struct lxs_ts *ts);
extern int lxs_hal_init(struct lxs_ts *ts);
extern int lxs_hal_reset(struct lxs_ts *ts, int ctrl, int pwr_con);
extern void lxs_hal_deep_sleep(struct lxs_ts *ts);
extern int lxs_hal_usb_status(struct lxs_ts *ts);
extern int lxs_hal_wireless_status(struct lxs_ts *ts);
extern int lxs_hal_ime_state(struct lxs_ts *ts);
extern int lxs_hal_incoming_call(struct lxs_ts *ts);
extern int lxs_hal_tc_con(struct lxs_ts *ts, u32 code, void *param);
extern int lxs_hal_tc_driving(struct lxs_ts *ts, int mode);

extern int lxs_hal_enable_touch(struct lxs_ts *ts, int state_mode);
extern int lxs_hal_enable_device(struct lxs_ts *ts, int enable);

extern int lxs_hal_touch_mode(struct lxs_ts *ts, int state_mode);

extern int lxs_hal_check_status(struct lxs_ts *ts);
extern int lxs_hal_irq_handler(struct lxs_ts *ts);

extern int lxs_hal_irq_abs(struct lxs_ts *ts);
extern int lxs_hal_irq_lpwg(struct lxs_ts *ts);

extern void lxs_hal_activate(struct lxs_ts *ts);
extern void lxs_hal_deactivate(struct lxs_ts *ts);

extern int lxs_hal_probe(struct lxs_ts *ts);
extern void lxs_hal_remove(struct lxs_ts *ts);

extern void lxs_hal_upgrade_setup(struct lxs_ts *ts);
extern int lxs_hal_upgrade(struct lxs_ts *ts);
/* */

extern int lxs_ts_init_sysfs(struct lxs_ts *ts);
extern void lxs_ts_free_sysfs(struct lxs_ts *ts);

extern int lxs_ts_bus_read(struct lxs_ts *ts, u32 addr, void *data, int size);
extern int lxs_ts_bus_write(struct lxs_ts *ts, u32 addr, void *data, int size);

extern int lxs_ts_register_driver(struct lxs_ts_if_driver *if_drv);
extern void lxs_ts_unregister_driver(struct lxs_ts_if_driver *if_drv);

extern void lxs_ts_gpio_set_reset(struct lxs_ts *ts, bool val);
extern int lxs_ts_init_gpios(struct lxs_ts *ts);
extern void lxs_ts_free_gpios(struct lxs_ts *ts);
extern int lxs_ts_irq_level_check(struct lxs_ts *ts);

extern int lxs_ts_set_pinctrl(struct lxs_ts *ts, bool on);

extern int lxs_ts_power_ctrl(struct lxs_ts *ts, int ctrl);
extern int lxs_ts_power_reset(struct lxs_ts *ts, int ctrl);

extern void lxs_ts_irq_control(struct lxs_ts *ts, bool on);

extern void lxs_ts_release_all_event(struct lxs_ts *ts);

extern int lxs_ts_probe(struct lxs_ts *ts);
extern void lxs_ts_remove(struct lxs_ts *ts, int is_shutdown);

extern int lxs_ts_get_pw_status(void);
extern int lxs_ts_pw_lock(struct lxs_ts *ts, incell_pw_lock status);

#if defined(__LXS_SUPPORT_WATCHDOG)
extern const char *lxs_hal_watchdog_sts(struct lxs_ts *ts);
extern int lxs_hal_watchdog_work(struct lxs_ts *ts);
extern void lxs_ts_watchdog_run(struct lxs_ts *ts);
extern void lxs_ts_watchdog_stop(struct lxs_ts *ts);
extern void lxs_ts_watchdog_time(struct lxs_ts *ts, int time);
#else
static inline void lxs_ts_watchdog_run(struct lxs_ts *ts){	}
static inline void lxs_ts_watchdog_stop(struct lxs_ts *ts){	}
static inline void lxs_ts_watchdog_time(struct lxs_ts *ts, int time){	}
#endif

#if defined(__LXS_SUPPORT_FILTER_CON)
extern int lxs_hal_chk_filter(struct lxs_ts *ts, int idx, int wr);
#define lxs_hal_blend_filter(_ts)	lxs_hal_chk_filter(_ts, 0, 1)
#define lxs_hal_stop_filter(_ts)	lxs_hal_chk_filter(_ts, 1, 1)
#define lxs_hal_wet_mode(_ts)		lxs_hal_chk_filter(_ts, 0, 0)
#endif

#endif /* __LXS_TS_H */
