/*****************************************************************************
*  Copyright 2001 - 2012 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#ifndef __LINUX_MFD_BCMPMU59xxx_H_
#define __LINUX_MFD_BCMPMU59xxx_H_

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/wakelock.h>
#include <plat/pi_mgr.h>
#include <linux/spinlock.h>
#include <linux/i2c.h>
#include <linux/power_supply.h>
#include <linux/platform_device.h>
#include <linux/regulator/machine.h>
#include <linux/i2c-kona.h>

#define BCMPMU_DUMMY_CLIENTS 1
#define REG_READ_COUNT_MAX	20
#define IRQ_REG_WIDTH 8
#define NUM_IRQ_REG	0xF

/* local debug level */

#define BCMPMU_PRINT_ERROR	(1U << 0)
#define BCMPMU_PRINT_INIT	(1U << 1)
#define BCMPMU_PRINT_FLOW	(1U << 2)
#define BCMPMU_PRINT_DATA	(1U << 3)
#define BCMPMU_PRINT_WARNING	(1U << 4)
#define BCMPMU_PRINT_VERBOSE	(1U << 5)

#define BB_BC_STS_SDP_MSK	(1<<0)
#define BB_BC_STS_DCP_MSK	(1<<1)
#define BB_BC_STS_CDP_MSK	(1<<2)
#define BB_BC_STS_TYPE1_MSK	(1<<3)
#define BB_BC_STS_TYPE2_MSK	(1<<4)
#define BB_BC_STS_ACA_MSK	(1<<5)
#define BB_BC_STS_PS2_MSK	(1<<6)
#define BB_BC_STS_BC_DONE_MSK	(1<<7)

#define PMU_BC_STS_SDP_MSK	(1<<2)
#define PMU_BC_STS_DCP_MSK	(1<<1)
#define PMU_BC_STS_CDP_MSK	(1<<3)
#define PMU_BC_STS_TYPE1_MSK	(1<<5)
#define PMU_BC_STS_TYPE2_MSK	(1<<6)
#define PMU_BC_STS_ACA_MSK	(1<<7)
#define PMU_BC_STS_PS2_MSK	(1<<4)
#define PMU_BC_STS_BC_DONE_MSK	(1<<8)

#define PMU_BC_NONE		0
#define PMU_BC_SDP		1
#define PMU_BC_CDP		2
#define PMU_BC_DCP		3
#define PMU_CHP_TYPE_MASK	0x3

#define JIG_BC_STS_SDP_MSK     (1<<2)
/* uart including Jig UART */
#define JIG_BC_STS_UART_MSK    ((1<<3) | (1<<11) | (1<<10))
#define JIG_BC_STS_DCP_MSK     (1<<6)
#define JIG_BC_STS_CDP_MSK     (1<<5)
#define CURR_LMT_MAX	0xFFFF

/* PMU ID and Revision */
#define BCMPMU_59054_ID		0x54
#define BCMPMU_59054A0_DIG_REV	1
#define BCMPMU_59054A1_ANA_REV	2

#define PMU_USB_FC_CC_OTP	400
#define PMU_USB_CC_ZERO_TRIM	0
#define PMU_DCP_DEF_CURR_LMT	700
#define PMU_MAX_CC_CURR		2200
#define PMU_OTP_CC_TRIM		0x1F
#define PMU_TYP_SAT_CURR	1600 /*mA*/

/*helper macros to manage regulator PC pin map*/
/*
SET0 holds ORed list of PC pin - regualtor will be enabled if
any of the PC pin in SET0 is HIGH

SET1 holds compined set. Regualtor will be enabled if all
the PC pins in SET1 are HIGH
*/
#define PCPIN_MAP_SET1_OFFSET   8
#define PCPIN_MAP_SET0_OFFSET   0
#define PCPIN_MAP_SET_MASK      7
#define PCPIN_MAP_ENC(set1, set0) \
	((((set1) & PCPIN_MAP_SET_MASK) << PCPIN_MAP_SET1_OFFSET) | \
		(((set0) & PCPIN_MAP_SET_MASK) << PCPIN_MAP_SET0_OFFSET))
#define PCPIN_MAP_GET_SET(map, n) \
		(((map) >> PCPIN_MAP_SET##n##_OFFSET) &  PCPIN_MAP_SET_MASK)

#define PCPIN_MAP_IS_SET0_MATCH(set, val) (!(!((set) & (val))))
#define PCPIN_MAP_IS_SET1_MATCH(set, val) ((set) && ((set) & (val)) == (set))

struct bcmpmu_rw_data_ltp {
unsigned int map;
unsigned int addr;
unsigned int val[16];
unsigned int mask;
unsigned int len;
};

#define BCM_PMU_MAGIC   'P'
#define BCM_PMU_CMD_READ_REG            0x83
#define BCM_PMU_CMD_WRITE_REG           0x84
#define BCM_PMU_CMD_BULK_READ_REG       0x85
#define BCM_PMU_CMD_ADC_READ_REG        0x86
#define BCM_PMU_CMD_NTC_TEMP            0x87
#define BCM_PMU_IOCTL_READ_REG          \
		_IOWR(BCM_PMU_MAGIC, BCM_PMU_CMD_READ_REG,\
				struct bcmpmu_rw_data_ltp)
#define BCM_PMU_IOCTL_BULK_READ_REG             \
			_IOWR(BCM_PMU_MAGIC, BCM_PMU_CMD_BULK_READ_REG,\
					struct bcmpmu_rw_data_ltp)
#define BCM_PMU_IOCTL_WRITE_REG         \
			_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_WRITE_REG,\
					struct bcmpmu_rw_data_ltp)

#define BCM59054_A1_ANA_REV 2
struct bcmpmu59xxx;
extern struct regulator_ops bcmpmu59xxx_ldo_ops;

enum bcmpmu59xxx_irq_reg {
	PMU_REG_IRQ1,
	PMU_REG_IRQ2,
	PMU_REG_IRQ3,
	PMU_REG_IRQ4,
	PMU_REG_IRQ5,
	PMU_REG_IRQ6,
	PMU_REG_IRQ7,
	PMU_REG_IRQ8,
	PMU_REG_IRQ9,
	PMU_REG_IRQ10,
	PMU_REG_IRQ11,
	PMU_REG_IRQ12,
	PMU_REG_IRQ13,
	PMU_REG_IRQ14,
};

enum bcmpmu59xxx_irq {
	PMU_IRQ_USBINS, /* 0 */
	PMU_IRQ_USBRM,
	PMU_IRQ_BATINS,
	PMU_IRQ_BATRM,
	PMU_IRQ_MBC_CV_LOOP,
	PMU_IRQ_MBC_CV_TMR_EXP,
	PMU_IRQ_EOC,
	PMU_IRQ_RESUME_VBUS, /* 7 */
	PMU_IRQ_MBTEMPLOW,
	PMU_IRQ_MBTEMPHIGH,
	PMU_IRQ_USBOV,
	PMU_IRQ_MBOV,
	PMU_IRQ_CHGERRDIS,
	PMU_IRQ_MBOV_DIS,
	PMU_IRQ_USBOV_DIS,
	PMU_IRQ_MBC_TF, /* 15 */
	PMU_IRQ_CHG_HW_TTR_EXP,
	PMU_IRQ_CHG_HW_TCH_EXP,
	PMU_IRQ_CHG_SW_TMR_EXP,
	PMU_IRQ_CHG_TCH_1MIN_BF_EXP,
	PMU_IRQ_USB_PORT_DIS,
	PMU_IRQ_USB_CC_REDUCE,
	PMU_IRQ_VBUSLOWBND,
	PMU_IRQ_UBPD_CHG_F, /* 23 */
	PMU_IRQ_VBUS_VALID_F,
	PMU_IRQ_OTG_SESS_VALID_F,
	PMU_IRQ_VB_SESS_END_F,
	PMU_IRQ_ID_RM,
	PMU_IRQ_VBUS_VALID_R,
	PMU_IRQ_VA_SESS_VALID_R,
	PMU_IRQ_VB_SESS_END_R,
	PMU_IRQ_ID_INS, /* 31 */
	PMU_IRQ_IDCHG,
	PMU_IRQ_RIC_C_TO_FLOAT,
	PMU_IRQ_CHGDET_LATCH,
	PMU_IRQ_CHGDET_TO,
	PMU_IRQ_ADP_CHANGE,
	PMU_IRQ_ADP_SNS_END,
	PMU_IRQ_ADP_PROB,
	PMU_IRQ_ADP_PRB_ERR, /* 39 */
	PMU_IRQ_POK_PRESSED,
	PMU_IRQ_POK_RELEASED,
	PMU_IRQ_POK_WAKEUP,
	PMU_IRQ_POK_BIT_VLD,
	PMU_IRQ_POK_RESTART,
	PMU_IRQ_POK_T1,
	PMU_IRQ_POK_T2,
	PMU_IRQ_POK_T3, /* 47 */
	PMU_IRQ_I2C_RESTART,
	PMU_IRQ_GBAT_PLUG_IN,
	PMU_IRQ_SMPL_INT,
	PMU_IRQ_AUX_INS,
	PMU_IRQ_AUX_RM,
	PMU_IRQ_XTAL_FAILURE,
	PMU_IRQ_MBWV_R_10S_WAIT,
	PMU_IRQ_MBWV_F, /* 55 */
	PMU_IRQ_RTC_ALARM,
	PMU_IRQ_RTC_SEC,
	PMU_IRQ_RTC_MIN,
	PMU_IRQ_RTCADJ,
	PMU_IRQ_FGC,
	PMU_IRQ_BBLOW,
	PMU_IRQ_DIE_OT_R,
	PMU_IRQ_DIE_OT_F, /* 63 */
	PMU_IRQ_RTM_DATA_RDY,
	PMU_IRQ_RTM_IN_CON_MEAS,
	PMU_IRQ_RTM_UPPER,
	PMU_IRQ_RTM_IGNORE,
	PMU_IRQ_RTM_OVERRIDDEN,
	PMU_IRQ_AUD_HSAB_SHCKT,
	PMU_IRQ_AUD_IHFD_SHCKT,
	PMU_IRQ_POK_NOP, /* 71 */
	PMU_IRQ_MIPI_LEN_ERR,
	PMU_IRQ_MIPI_RCV_ERR,
	PMU_IRQ_MIPI_BUSQ_RESP,
	PMU_IRQ_MIPI_BUSQ_POS,
	PMU_IRQ_MIPI_EOT,
	PMU_IRQ_MIPI_XMT_END,
	PMU_IRQ_MIPI_INT_POS,
	PMU_IRQ_LOWBAT, /* 79 */
	PMU_IRQ_CSROVRI,
	PMU_IRQ_VSROVRI,
	PMU_IRQ_MSROVRI,
	PMU_IRQ_SDSR1OVRI,
	PMU_IRQ_SDSR2OVRI,
	PMU_IRQ_IOSR1OVRI,
	PMU_IRQ_IOSR2OVRI,
	PMU_IRQ_RESERVED,
	PMU_IRQ_RFLDO_OVRI, /* 87 */
	PMU_IRQ_AUDLDO_OVRI,
	PMU_IRQ_USBLDO_OVR,
	PMU_IRQ_SDXLDO_OVRI,
	PMU_IRQ_MICLDO_OVRI,
	PMU_IRQ_SIMLDO1_OVRI,
	PMU_IRQ_SIMLDO2_OVRI,
	PMU_IRQ_MMCLDO1_OVRI, /* 96 */
	PMU_IRQ_CAMLDO1_OVRI,
	PMU_IRQ_CAMLDO2_OVRI,
	PMU_IRQ_VIBLDO_OVRI,
	PMU_IRQ_SDLDO_OVRI,
	PMU_IRQ_GPLDO1_OVRI,
	PMU_IRQ_GPLDO2_OVRI,
	PMU_IRQ_GPLDO3_OVRI,
	PMU_IRQ_RFLDO_SHD,
	PMU_IRQ_AUDLDO_SHD, /* 103 */
	PMU_IRQ_USBLDO_SHD,
	PMU_IRQ_SDXLDO_SHD,
	PMU_IRQ_MICLDO_SHD,
	PMU_IRQ_SIMLDO1_SHD,
	PMU_IRQ_SIMLDO2_SHD,
	PMU_IRQ_MMCLDO1_SHD,
	PMU_IRQ_MMCLDO2_SHD,
	PMU_IRQ_CAMLDO1_SHD, /* 111 */
	PMU_IRQ_CAMLDO2_SHD,
	PMU_IRQ_VIBLDO_SHD,
	PMU_IRQ_SDLDO_SHD,
	PMU_IRQ_GPLDO1_SHD,
	PMU_IRQ_GPLDO2_SHD,
	PMU_IRQ_GPLDO3_SHD,
	PMU_IRQ_TCXLDO_OVRI,
	PMU_IRQ_LVLDO1_OVRI, /* 119 */
	PMU_IRQ_LVLDO2_OVRI,
	PMU_IRQ_TCXLDO_SHD,
	PMU_IRQ_LVLDO1_SHD,
	PMU_IRQ_LVLDO2_SHD,
	PMU_IRQ_VBOVRV,
	PMU_IRQ_VBOVRI,
	PMU_IRQ_MAX,
};

enum bcmpmu_rgr_id {
	BCMPMU_REGULATOR_RFLDO,
	BCMPMU_REGULATOR_CAMLDO1,
	BCMPMU_REGULATOR_CAMLDO2,
	BCMPMU_REGULATOR_SIMLDO1,
	BCMPMU_REGULATOR_SIMLDO2,
	BCMPMU_REGULATOR_SDLDO,
	BCMPMU_REGULATOR_SDXLDO,
	BCMPMU_REGULATOR_MMCLDO1,
	BCMPMU_REGULATOR_MMCLDO2,
	BCMPMU_REGULATOR_AUDLDO,
	BCMPMU_REGULATOR_MICLDO,
	BCMPMU_REGULATOR_USBLDO,
	BCMPMU_REGULATOR_VIBLDO,
	BCMPMU_REGULATOR_GPLDO1,
	BCMPMU_REGULATOR_GPLDO2,
	BCMPMU_REGULATOR_GPLDO3,
	BCMPMU_REGULATOR_TCXLDO,
	BCMPMU_REGULATOR_LVLDO1,
	BCMPMU_REGULATOR_LVLDO2,
	BCMPMU_REGULATOR_VSR,
	BCMPMU_REGULATOR_CSR,
	BCMPMU_REGULATOR_MMSR,
	BCMPMU_REGULATOR_SDSR1,
	BCMPMU_REGULATOR_SDSR2,
	BCMPMU_REGULATOR_IOSR1,
	BCMPMU_REGULATOR_IOSR2,
	BCMPMU_REGULATOR_MAX,
};

/* Warning: Donot change order of channels,
 *	    Donot Insert/Remove channel from this enum
 **/
enum bcmpmu_adc_channel {
	PMU_ADC_CHANN_VMBATT,
	PMU_ADC_CHANN_VBBATT,
	PMU_ADC_CHANN_RESERVED,
	PMU_ADC_CHANN_VBUS,
	PMU_ADC_CHANN_IDIN,
	PMU_ADC_CHANN_NTC,
	PMU_ADC_CHANN_BSI,
	PMU_ADC_CHANN_BOM,
	PMU_ADC_CHANN_32KTEMP,
	PMU_ADC_CHANN_PATEMP,
	PMU_ADC_CHANN_ALS,
	PMU_ADC_CHANN_RID_CAL_L,
	PMU_ADC_CHANN_NTC_CAL_L,
	PMU_ADC_CHANN_NTC_CAL_H,
	PMU_ADC_CHANN_DIE_TEMP,
	PMU_ADC_CHANN_RID_CAL_H,
	PMU_ADC_CHANN_MAX,
};

enum bcmpmu_adc_req {
	PMU_ADC_REQ_SAR_MODE,
	PMU_ADC_REQ_RTM_MODE,
};

enum bcmpmu_adc_flag {
	PMU_ADC_FLAG_CONV_LUT = 0x1 << 0, /* USE Lookup table for conversion */
};
/**
 * returns the adc result
 * raw : adc sample raw value before any convertion
 * conv : converted value
 */
struct bcmpmu_adc_result {
	int raw;
	int conv;
};

struct bcmpmu_adc_lut {
	unsigned int raw;
	unsigned int map; /* temp, volt, etc map value in table */
};

/**
 * @chann: ADC channel
 * @flags: ADC flags
 * @resolution: resolution of ADC channel
 * (BCM PMU supports 10 bit ADC resolution = 1024)
 * @volt_range: voltage range of ADC channel in mv (designed voltage range)
 * @lut: Lookup table to be used for conversion if @flags=PMU_ADC_FLAG_CONV_LUT
 */
struct bcmpmu_adc_pdata {
	unsigned int flag;
	int volt_range;
	int adc_offset; /* +/- offset/gain */
	struct bcmpmu_adc_lut *lut;
	unsigned int lut_len;
	char *name;
	u32 reg;
};


enum bcmpmu_adc_ctrl {
	PMU_ADC_RST_CNT,
	PMU_ADC_RTM_START,
	PMU_ADC_RTM_MASK,
	PMU_ADC_RTM_SEL,
	PMU_ADC_RTM_DLY,
	PMU_ADC_GSM_DBNC,
	PMU_ADC_CTRL_MAX,
};

enum bcmpmu_adc_timing_t {
	PMU_ADC_TM_RTM_TX,
	PMU_ADC_TM_RTM_RX,
	PMU_ADC_TM_RTM_SW,
	PMU_ADC_TM_RTM_SW_TEST,
	PMU_ADC_TM_HK,
	PMU_ADC_TM_MAX,
};

struct bcmpmu_acld_pdata {
	int acld_vbus_margin;
	int acld_vbus_thrs;
	int acld_vbat_thrs;
	int i_sat;
	int i_def_dcp; /* Default DCP current */
	int i_max_cc;
	int acld_cc_lmt;
	int otp_cc_trim;
	int one_c_rate;
	bool qa_required; /* Set this to true if
			     Ibus is strictly limited to acld_cc_lmt */
};

enum bcmpmu_chrgr_fc_curr_t {
	PMU_CHRGR_CURR_50,
	PMU_CHRGR_CURR_100,
	PMU_CHRGR_CURR_150,
	PMU_CHRGR_CURR_200,
	PMU_CHRGR_CURR_250,
	PMU_CHRGR_CURR_300,
	PMU_CHRGR_CURR_350,
	PMU_CHRGR_CURR_400,
	PMU_CHRGR_CURR_450,
	PMU_CHRGR_CURR_500,
	PMU_CHRGR_CURR_550,
	PMU_CHRGR_CURR_600,
	PMU_CHRGR_CURR_650,
	PMU_CHRGR_CURR_700,
	PMU_CHRGR_CURR_750,
	PMU_CHRGR_CURR_800,
	PMU_CHRGR_CURR_850,
	PMU_CHRGR_CURR_900,
	PMU_CHRGR_CURR_950,
	PMU_CHRGR_CURR_1000,
	PMU_CHRGR_CURR_90,
	PMU_CHRGR_CURR_MAX,
};

enum bcmpmu_chrgr_qc_curr_t {
	PMU_CHRGR_QC_CURR_50,
	PMU_CHRGR_QC_CURR_60,
	PMU_CHRGR_QC_CURR_70,
	PMU_CHRGR_QC_CURR_80,
	PMU_CHRGR_QC_CURR_90,
	PMU_CHRGR_QC_CURR_100,
	PMU_CHRGR_QC_CURR_MAX,
};

enum bcmpmu_chrgr_eoc_curr_t {
	PMU_CHRGR_EOC_CURR_50,
	PMU_CHRGR_EOC_CURR_60,
	PMU_CHRGR_EOC_CURR_70,
	PMU_CHRGR_EOC_CURR_80,
	PMU_CHRGR_EOC_CURR_90,
	PMU_CHRGR_EOC_CURR_100,
	PMU_CHRGR_EOC_CURR_110,
	PMU_CHRGR_EOC_CURR_120,
	PMU_CHRGR_EOC_CURR_130,
	PMU_CHRGR_EOC_CURR_140,
	PMU_CHRGR_EOC_CURR_150,
	PMU_CHRGR_EOC_CURR_160,
	PMU_CHRGR_EOC_CURR_170,
	PMU_CHRGR_EOC_CURR_180,
	PMU_CHRGR_EOC_CURR_190,
	PMU_CHRGR_EOC_CURR_200,
	PMU_CHRGR_EOC_CURR_MAX,
};

enum bcmpmu_chrgr_volt_t {
	PMU_CHRGR_VOLT_3600,
	PMU_CHRGR_VOLT_3625,
	PMU_CHRGR_VOLT_3650,
	PMU_CHRGR_VOLT_3675,
	PMU_CHRGR_VOLT_3700,
	PMU_CHRGR_VOLT_3725,
	PMU_CHRGR_VOLT_3750,
	PMU_CHRGR_VOLT_3775,
	PMU_CHRGR_VOLT_3800,
	PMU_CHRGR_VOLT_3825,
	PMU_CHRGR_VOLT_3850,
	PMU_CHRGR_VOLT_3875,
	PMU_CHRGR_VOLT_3900,
	PMU_CHRGR_VOLT_3925,
	PMU_CHRGR_VOLT_3950,
	PMU_CHRGR_VOLT_3975,
	PMU_CHRGR_VOLT_4000,
	PMU_CHRGR_VOLT_4025,
	PMU_CHRGR_VOLT_4050,
	PMU_CHRGR_VOLT_4075,
	PMU_CHRGR_VOLT_4100,
	PMU_CHRGR_VOLT_4125,
	PMU_CHRGR_VOLT_4150,
	PMU_CHRGR_VOLT_4175,
	PMU_CHRGR_VOLT_4200,
	PMU_CHRGR_VOLT_4225,
	PMU_CHRGR_VOLT_4250,
	PMU_CHRGR_VOLT_4275,
	PMU_CHRGR_VOLT_4300,
	PMU_CHRGR_VOLT_4325,
	PMU_CHRGR_VOLT_4350,
	PMU_CHRGR_VOLT_4375,
	PMU_CHRGR_VOLT_MAX,
};

enum bcmpmu_chrgr_type_t {
	PMU_CHRGR_TYPE_NONE,
	PMU_CHRGR_TYPE_SDP,
	PMU_CHRGR_TYPE_CDP,
	PMU_CHRGR_TYPE_DCP,
	PMU_CHRGR_TYPE_TYPE1,
	PMU_CHRGR_TYPE_TYPE2,
	PMU_CHRGR_TYPE_PS2,
	PMU_CHRGR_TYPE_ACA_DOCK,
	PMU_CHRGR_TYPE_ACA,
	PMU_CHRGR_TYPE_MAX,
};

enum bcmpmu_usb_adp_mode_t {
	PMU_USB_ADP_MODE_REPEAT,
	PMU_USB_ADP_MODE_CALIBRATE,
	PMU_USB_ADP_MODE_ONESHOT,
};

enum bcmpmu_usb_id_lvl_t {
	PMU_USB_ID_GROUND,		/* 000 */
	PMU_USB_B_DEVICE,		/* 001 */
	PMU_USB_ID_RESERVED1,	/* 010 */
	PMU_USB_ID_RID_A,		/* 011 */
	PMU_USB_ID_RID_B,		/* 100 */
	PMU_USB_ID_RID_C,		/* 101 */
	PMU_USB_ID_RESERVED2,	/* 110 */
	PMU_USB_ID_FLOAT,		/* 111 */
	PMU_USB_ID_LVL_MAX,
};

enum bcmpmu_bc_t {
	BCMPMU_BC_BB_BC11,
	BCMPMU_BC_BB_BC12,
	BCMPMU_BC_PMU_BC12,
	BC_EXT_DETECT,
};

enum bcmpmu_event_t {
	PMU_ACCY_EVT_OUT_CHRGR_TYPE,
	PMU_ACCY_EVT_OUT_USB_IN,
	PMU_ACCY_EVT_OUT_USB_RM,
	PMU_ACCY_EVT_OUT_ADP_CHANGE,
	PMU_ACCY_EVT_OUT_ADP_SENSE_END,
	PMU_ACCY_EVT_OUT_ADP_CALIBRATION_DONE,
	PMU_ACCY_EVT_OUT_ID_CHANGE,
	PMU_ACCY_EVT_OUT_VBUS_VALID,
	PMU_ACCY_EVT_OUT_VBUS_INVALID,
	PMU_ACCY_EVT_OUT_SESSION_VALID,
	PMU_ACCY_EVT_OUT_SESSION_INVALID,
	PMU_ACCY_EVT_OUT_SESSION_END_INVALID,
	PMU_ACCY_EVT_OUT_SESSION_END_VALID,
	PMU_ACCY_EVT_OUT_CHGDET_LATCH,
	PMU_ACCY_EVT_OUT_CHGDET_LATCH_TO,
	PMU_ACCY_EVT_OUT_CHRG_RESUME_VBUS,
	PMU_ACCY_EVT_OUT_CHRG_CURR,
	PMU_ACCY_EVT_OUT_USBOV,
	PMU_ACCY_EVT_OUT_USBOV_DIS,
	PMU_ACCY_EVT_OUT_CHGERRDIS,

	PMU_CHRGR_DET_EVT_OUT_XCVR,

	PMU_CHRGR_EVT_MBTEMP,
	PMU_CHRGR_EVT_MBOV,
	PMU_CHRGR_EVT_EOC,
	PMU_CHRGR_EVT_CHRG_STATUS,
	PMU_ACLD_EVT_ACLD_STATUS,
	PMU_THEMAL_THROTTLE_STATUS,
	PMU_FG_EVT_CAPACITY,
	PMU_FG_EVT_FGC,
	PMU_JIG_EVT_USB,
	PMU_JIG_EVT_UART,
	PMU_EVENT_MAX,
};

enum bcmpmu_usb_ctrl_t {
	BCMPMU_USB_CTRL_CHRG_CURR_LMT,
	BCMPMU_USB_CTRL_VBUS_ON_OFF,
	BCMPMU_USB_CTRL_SET_VBUS_DEB_TIME,
	BCMPMU_USB_CTRL_SRP_VBUS_PULSE,
	BCMPMU_USB_CTRL_DISCHRG_VBUS,
	BCMPMU_USB_CTRL_START_STOP_ADP_SENS_PRB,
	BCMPMU_USB_CTRL_START_STOP_ADP_PRB,
	BCMPMU_USB_CTRL_START_ADP_CAL_PRB,
	BCMPMU_USB_CTRL_SET_ADP_PRB_MOD,
	BCMPMU_USB_CTRL_SET_ADP_PRB_CYC_TIME,
	BCMPMU_USB_CTRL_SET_ADP_COMP_METHOD,
	BCMPMU_USB_CTRL_GET_ADP_CHANGE_STATUS,
	BCMPMU_USB_CTRL_GET_ADP_SENSE_TIMER_VALUE,
	BCMPMU_USB_CTRL_GET_ADP_SENSE_STATUS,
	BCMPMU_USB_CTRL_GET_ADP_PRB_RISE_TIMES,
	BCMPMU_USB_CTRL_GET_VBUS_STATUS,
	BCMPMU_USB_CTRL_GET_SESSION_STATUS,
	BCMPMU_USB_CTRL_GET_USB_VALID,
	BCMPMU_USB_CTRL_GET_UBPD_INT,
	BCMPMU_USB_CTRL_GET_USB_PORT_DISABLED,
	BCMPMU_USB_CTRL_GET_SESSION_END_STATUS,
	BCMPMU_USB_CTRL_GET_ID_VALUE,
	BCMPMU_USB_CTRL_GET_CHRGR_TYPE,
	BCMPMU_USB_CTRL_SW_UP,
	BCMPMU_USB_CTRL_TPROBE_MAX,
	BCMPMU_USB_CTRL_ALLOW_BC_DETECT,
};

struct bcmpmu59xxx_rw_data {
	unsigned int addr;
	unsigned int val;
	unsigned int mask;
	unsigned int map;
};

/*PMU PC PINs*/
enum {
	PMU_PC1 = 1,
	PMU_PC2 = 1 << 1,
	PMU_PC3 = 1 << 2,
};

/*regualtor control flags*/
enum {
	RGLR_ON = (1 << 0),
	RGLR_SR = (1 << 1),
	RGLR_3BIT_PMCTRL = (1 << 2),
	RGLR_FIXED_VLT = (1 << 3),
};

struct bcmpmu59xxx_regulator_info {
	struct regulator_desc *rdesc;
	/*ctrl flags*/
	u32 flags;
	/* start addr of opmode ctrl register */
	u32 reg_pmctrl1;
	/* address of control register to change voltage */
	u32 reg_vout;
	u32 vout_mask;
	u32 vout_shift;
	u32 vout_trim;
	/* Map for converting register voltage to register value */
	u32 *v_table;
	/* Size of register map */
	u32 num_voltages;
};

struct event_notifier {
	u32 event_id;
	struct blocking_notifier_head notifiers;
};
struct event_list {
	struct list_head node;
	u32 event;
	void *para;
};

struct bcmpmu59xxx_rev_info {
	u8 gen_id;/* Generation id */
	u8 prj_id;/* Project id */
	u8 dig_rev;/* Digital revision */
	u8 ana_rev;/* Analog revision */
};

struct bcmpmu59xxx_bus {
	char *name;
	struct i2c_client *i2c;
	struct i2c_client *companinon[BCMPMU_DUMMY_CLIENTS];
#ifdef CONFIG_DEBUG_FS
	struct dentry *dentry;
#endif	/*CONFIG_DEBUG_FS*/
struct mutex i2c_mutex;
#ifdef CONFIG_HAS_WAKELOCK
	struct wake_lock i2c_lock;
	struct wake_lock usb_plug_out_wake_lock;
	u32 ref_count;
#endif
};

/*PONKEY press/release debounce*/
enum {
	PKEY_DEB_330US,
	PKEY_DEB_1P2MS,
	PKEY_DEB_10MS,
	PKEY_DEB_50MS,
	PKEY_DEB_100MS,
	PKEY_DEB_500MS,
	PKEY_DEB_1000MS,
	PKEY_DEB_2000MS,
	PKEY_DEB_MAX,
};

/*PONKEY wakeup debounce*/
enum {
	PKEY_WUP_DEB_NONE,
	PKEY_WUP_DEB_50MS,
	PKEY_WUP_DEB_100MS,
	PKEY_WUP_DEB_500MS,
	PKEY_WUP_DEB_1000MS,
	PKEY_WUP_DEB_2000MS,
	PKEY_WUP_DEB_3000MS,
	PKEY_WUP_DEB_MAX,
};


/*PONKEY long press function definition*/
enum {
	PKEY_ACTION_SHUTDOWN,
	PKEY_ACTION_RESTART,
	PKEY_ACTION_SMART_RESET,
	PKEY_ACTION_NOP,
};
/*PONKEY timer action delay*/
enum {
	PKEY_ACT_DELAY_50MS,
	PKEY_ACT_DELAY_500MS,
	PKEY_ACT_DELAY_1S,
	PKEY_ACT_DELAY_5S,
	PKEY_ACT_DELAY_7S,
	PKEY_ACT_DELAY_9S,
	PKEY_ACT_DELAY_11S,
	PKEY_ACT_DELAY_13S,
	PKEY_ACT_DELAY_MAX,
};

/*PONKEY timer action debounce*/
enum {
	PKEY_ACT_DEB_NONE,
	PKEY_ACT_DEB_1S,
	PKEY_ACT_DEB_2S,
	PKEY_ACT_DEB_3S,
	PKEY_ACT_DEB_4S,
	PKEY_ACT_DEB_5S,
	PKEY_ACT_DEB_6S,
	PKEY_ACT_DEB_7S,
	PKEY_ACT_DEB_8S,
	PKEY_ACT_DEB_MAX,
};

/*Tx action control flags*/
enum {
	PKEY_SMART_RST_PWR_EN = 1,
};

/*Ponkey smart reset pulse duration
POK_SMART_RST_DLY_1_0_*/
enum {
	PKEY_SR_DLY_30MS,
	PKEY_SR_DLY_60MS,
	PKEY_SR_DLY_90MS,
	PKEY_SR_DLY_120MS,
	PKEY_SR_DLY_MAX
};

/*Ponkey restart delay
POK_RESTART_DLY_1_0_*/
enum {
	PKEY_RESTART_DLY_500MS,
	PKEY_RESTART_DLY_1S,
	PKEY_RESTART_DLY_2S,
	PKEY_RESTART_DLY_4S,
	PKEY_RESTART_DLY_MAX
};

#define TIME_3HR	(3)

enum {
	TCH_HW_TIMER_3HR,
	TCH_HW_TIMER_4HR,
	TCH_HW_TIMER_5HR,
	TCH_HW_TIMER_6HR,
	TCH_HW_TIMER_7HR,
	TCH_HW_TIMER_8HR,
	TCH_HW_TIMER_9HR,
	TCH_HW_TIMER_MAX,
};

/*PONKEY T1/T2/T3 action config*/
struct pkey_timer_act {
	u32 flags;
	u32 action;
	u32 timer_dly;
	u32 timer_deb;
	u32 ctrl_params; /*action specific*/
};

struct bcmpmu59xxx_pkey_pdata {
	u32 press_deb;
	u32 release_deb;
	u32 wakeup_deb;
	struct pkey_timer_act *t1;
	struct pkey_timer_act *t2;
	struct pkey_timer_act *t3;
};

struct bcmpmu59xxx_audio_pdata {
	int ihf_autoseq_dis;
};

struct bcmpmu59xxx_regulator_init_data {
	int id; /* Regulator ID */
	struct regulator_init_data *initdata;
	u32 pc_pins_map;
	u32 mode;/* normal mode*/
	char *name;
	u32 req_volt; /*in uV*/

#if defined(CONFIG_MACH_HAWAII_SS_COMMON)
	u32 reg_value;
	u32 reg_value2;
	u32 off_value;
	u32 off_value2;
#endif

};

struct bcmpmu59xxx_regulator_pdata {
	struct bcmpmu59xxx_regulator_init_data *bcmpmu_rgltr;
	u8 num_rgltr;
};

struct bcmpmu59xxx_rpc_pdata {
	u32 delay;
	u32 fw_delay;
	u32 fw_cnt;
	u32 poll_time;
	u32 htem_poll_time;
	u32 mod_tem;
	u32 htem;
};

struct bcmpmu59xxx_spa_pb_pdata {
	char *chrgr_name;
};

struct bcmpmu59xxx_accy_pdata {
#define ACCY_USE_PM_QOS		0x1
	unsigned long flags;
	int qos_pi_id;
};

/* charger pdata data flags */
enum {
	/**
	 * support for extented TCH timer feature
	 */
	BCMPMU_CHRGR_TCH_EXT_TIMER = 0x1 << 0,
};

/**
 * @tch_base : TCH timer base to use (see enum TCH_HW_TIMER_XX)
 * @tch_multiplier : mutliplier to use with @tch_base
 */
struct bcmpmu_chrgr_pdata {
	int *chrgr_curr_lmt_tbl;
	unsigned int flags;
	unsigned int tch_base;
	unsigned int tch_multiplier;
};

#if defined(CONFIG_LEDS_BCM_PMU59xxx)
struct bcmpmu59xxx_led_pdata {
	char *led_name;
};
#endif

/*BCMPMU generic control flags*/
enum {
	BCMPMU_SPA_EN = 1 << 0,
	BCMPMU_ACLD_EN = 1 << 1,
};
/* Board id enum */
enum {
	EDN01x,
	EDN010,
};
struct bcmpmu59xxx_platform_data {
	u32 flags; /*ctrl flags*/
	struct i2c_slave_platform_data i2c_pdata;
	int (*init) (struct bcmpmu59xxx *bcmpmu);
	int (*exit) (struct bcmpmu59xxx *bcmpmu);
	u8 companion;
	struct i2c_board_info *i2c_companion_info;
	int i2c_adapter_id;
	int i2c_pagesize;
	int irq;
	struct bcmpmu59xxx_rw_data *init_data;
	struct bcmpmu59xxx_rw_data *exit_data;
	int init_max;
	int exit_max;
	/*FIXME need to check this variable */
	enum bcmpmu_bc_t bc;
	int piggyback_chrg;
	char *piggyback_chrg_name;
	int board_id;
};

struct bcmpmu59xxx {
	struct device *dev;
	struct bcmpmu59xxx_bus *pmu_bus;
	struct bcmpmu59xxx_platform_data *pdata;
	void *irqinfo;
	void *rtcinfo;
	void *accyinfo;
	void *accy_d;
	void *rgltr_data;
	void *ponkeyinfo;
	void *adc;
	void *rpcinfo;
	void *fg;
	void *acld;
	void *spa_pb_info;
	u32 flags; /*ctrl flags - copied from pdata*/
	/* event notifier */
	struct event_notifier event[PMU_EVENT_MAX];
#ifdef CONFIG_DEBUG_FS
	struct dentry *dent_bcmpmu;
#endif	/*CONFIG_DEBUG_FS*/
	struct bcmpmu59xxx_rev_info rev_info;
	int (*read_dev) (struct bcmpmu59xxx *bcmpmu, u32 reg, u8 *val);
	int (*write_dev) (struct bcmpmu59xxx *bcmpmu, u32 reg, u8 val);
	int (*read_dev_bulk) (struct bcmpmu59xxx *bcmpmu, u32 reg,
			u8 *val, int len);
	int (*write_dev_bulk) (struct bcmpmu59xxx *bcmpmu, u32 reg,
			u8 *val, int len);
	/* Set PMU Bus read/write mode - Interrupt or polled */
	int (*set_dev_mode) (struct bcmpmu59xxx *bcmpmu59, int poll);
	/* irq */
	int (*register_irq) (struct bcmpmu59xxx *bcmpmu, u32 irq,
			void (*callback) (u32 irq, void *),
			void *data);
	int (*unregister_irq) (struct bcmpmu59xxx *bcmpmu, u32 irq);
	int (*mask_irq) (struct bcmpmu59xxx *bcmpmu, u32 irq);
	int (*unmask_irq) (struct bcmpmu59xxx *bcmpmu, u32 irq);
};

int bcmpmu_get_pmu_mfd_cell(struct mfd_cell **);

struct bcmpmu59xxx_regulator_info *
bcmpmu59xxx_get_rgltr_info(struct bcmpmu59xxx *bcmpmu);

int bcmpmu_rgltr_get_volt_id(u32 voltage);
int bcmpmu_rgltr_get_volt_val(u32 id);

void bcmpmu_client_power_off(void);

int bcmpmu_client_hard_reset(u8 chreset_reason);

int bcmpmu_add_notifier(u32 event_id, struct notifier_block *notifier);

int bcmpmu_remove_notifier(u32 event_id, struct notifier_block *notifier);

void bcmpmu_call_notifier(struct bcmpmu59xxx *pmu,
					enum bcmpmu_event_t event, void *para);

int bcmpmu_usb_get(struct bcmpmu59xxx *bcmpmu,
			int ctrl, void *data);

int bcmpmu_usb_set(struct bcmpmu59xxx *bcmpmu,
			int ctrl, unsigned long data);
int bcmpmu_check_vbus(void);
int bcmpmu_accy_chrgr_type_notify(int chrgr_type);
u32 bcmpmu_get_chrgr_curr_lmt(u32 chrgr_type);
int bcmpmu_chrgr_usb_en(struct bcmpmu59xxx *bcmpmu, int enable);
bool bcmpmu_is_usb_host_enabled(struct bcmpmu59xxx *bcmpmu);
int bcmpmu_set_icc_fc(struct bcmpmu59xxx *bcmpmu, int curr);
int bcmpmu_icc_fc_step_down(struct bcmpmu59xxx *bcmpmu);
int bcmpmu_icc_fc_step_up(struct bcmpmu59xxx *bcmpmu);
int bcmpmu_get_icc_fc(struct bcmpmu59xxx *bcmpmu);
int bcmpmu_get_next_icc_fc(struct bcmpmu59xxx *bcmpmu);
int bcmpmu_set_cc_trim(struct bcmpmu59xxx *bcmpmu, int cc_trim);
int bcmpmu_cc_trim_up(struct bcmpmu59xxx *bcmpmu);
int bcmpmu_cc_trim_down(struct bcmpmu59xxx *bcmpmu);
bool bcmpmu_get_mbc_faults(struct bcmpmu59xxx *bcmpmu);
int  bcmpmu_get_trim_curr(struct bcmpmu59xxx *bcmpmu);

bool bcmpmu_is_acld_enabled(struct bcmpmu59xxx *bcmpmu);

/* ADC */
int bcmpmu_adc_read(struct bcmpmu59xxx *bcmpmu, enum bcmpmu_adc_channel channel,
		enum bcmpmu_adc_req req, struct bcmpmu_adc_result *result);
int *bcmpmu59xxx_get_trim_table(struct bcmpmu59xxx *bcmpmu);
int bcmpmu59xxx_rgltr_info_init(struct bcmpmu59xxx *bcmpmu);

#ifdef CONFIG_CHARGER_BCMPMU_SPA
int bcmpmu_post_spa_event_to_queue(struct bcmpmu59xxx *bcmpmu,
	u32 event, u32 param);
int bcmpmu_post_spa_event(struct bcmpmu59xxx *bcmpmu, u32 event,
		u32 param);
#else
static inline int bcmpmu_post_spa_event_to_queue(struct bcmpmu59xxx *bcmpmu,
	u32 event, u32 param)
{
	return 0;
}
static inline int bcmpmu_post_spa_event(struct bcmpmu59xxx *bcmpmu,
	u32 event, u32 param)
{
	return 0;
}
#endif /*CONFIG_CHARGER_BCMPMU_SPA*/
#ifdef CONFIG_DEBUG_FS
int bcmpmu_debugfs_open(struct inode *inode, struct file *file);
#endif

#endif
