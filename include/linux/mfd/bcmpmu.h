/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
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

#ifndef __LINUX_MFD_BCMPMU_H_
#define __LINUX_MFD_BCMPMU_H_

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/power_supply.h>
#include <linux/platform_device.h>
#include <linux/regulator/machine.h>
#include <linux/i2c-kona.h>

#define	PMU_BITMASK_ALL		0xFFFFFFFF

struct bcmpmu;
struct regulator_init_data;

/* LDO or Switcher def */
#define BCMPMU_LDO    0x10
#define BCMPMU_SR     0x11
/* HOSTCTRL1 def*/
#define BCMPMU_SW_SHDWN 0x04

/* WRPREN def */
#define BCMPMU_DIS_WR_PRO       (1<<0)
#define BCMPMU_PMU_UNLOCK       (1<<1)
#define BCMPMU_WRLOCKKEY_VAL    0x38

/* Regulator OPMODE Donot Care*/
#define BCMPMU_REGU_OPMODE_DC 0xFF

int bcmpmu_register_regulator(struct bcmpmu *bcmpmu, int reg,
			      struct regulator_init_data *initdata);

struct bcmpmu_reg_info {
	/* address of regulator control register for mode control */
	u8 reg_addr;
	/* address of control register to change voltage */
	u8 reg_addr_volt;
	/* Mask of bits in register */
	u32 vout_mask;
	/* Bit shift in register */
	u32 vout_shift;
	/* Mask of bits in register */
	u32 vout_mask_l;
	/* Bit shift in register */
	u32 vout_shift_l;
	/* Mask of bits in register */
	u32 vout_mask_t;
	/* Bit shift in register */
	u32 vout_shift_t;
	/* Map for converting register voltage to register value */
	u32 *v_table;
	/* Size of register map */
	u32 num_voltages;
	u32 mode_mask;
	u8 ldo_or_sr;
};

/*State of enabled regualtor in deep sleep
Used to program PC2PC1 = 0b10 & 0b00 case
when the regulator is enabled*/
enum {
	BCMPMU_REGL_ON_IN_DSM = 1,
	BCMPMU_REGL_LPM_IN_DSM,
	BCMPMU_REGL_OFF_IN_DSM
};

struct bcmpmu_regulator_init_data {
	int regulator ; /* Regulator ID */
	struct regulator_init_data   *initdata;
	/* Default opmode value.Pass 0xFF to skip opmoe setting for a ldo/sr */
	u8 default_opmode;
	u8 dsm_mode;
};

enum bcmpmu_rgr_id {
	BCMPMU_REGULATOR_RFLDO,
	BCMPMU_REGULATOR_CAMLDO,
	BCMPMU_REGULATOR_HV1LDO,
	BCMPMU_REGULATOR_HV2LDO,
	BCMPMU_REGULATOR_HV3LDO,
	BCMPMU_REGULATOR_HV4LDO,
	BCMPMU_REGULATOR_HV5LDO,
	BCMPMU_REGULATOR_HV6LDO,
	BCMPMU_REGULATOR_HV7LDO,
	BCMPMU_REGULATOR_HV8LDO,
	BCMPMU_REGULATOR_HV9LDO,
	BCMPMU_REGULATOR_HV10LDO,
	BCMPMU_REGULATOR_SIMLDO,
	BCMPMU_REGULATOR_USBLDO,
	BCMPMU_REGULATOR_BCDLDO,
	BCMPMU_REGULATOR_DVS1LDO,
	BCMPMU_REGULATOR_DVS2LDO,
	BCMPMU_REGULATOR_SIM2LDO,
	BCMPMU_REGULATOR_CSR_NM,
	BCMPMU_REGULATOR_CSR_NM2,
	BCMPMU_REGULATOR_CSR_LPM,
	BCMPMU_REGULATOR_IOSR_NM,
	BCMPMU_REGULATOR_IOSR_NM2,
	BCMPMU_REGULATOR_IOSR_LPM,
	BCMPMU_REGULATOR_SDSR_NM,
	BCMPMU_REGULATOR_SDSR_NM2,
	BCMPMU_REGULATOR_SDSR_LPM,
	BCMPMU_REGULATOR_ASR_NM,
	BCMPMU_REGULATOR_ASR_NM2,
	BCMPMU_REGULATOR_ASR_LPM,

	BCMPMU_REGULATOR_MAX,
};

extern struct regulator_ops bcmpmuldo_ops;

/* struct bcmpmu; */
enum bcmpmu_reg {
	PMU_REG_SMPLCTRL,
	PMU_REG_WRLOCKKEY,
	PMU_REG_WRPROEN,
	PMU_REG_PMUGID,
	PMU_REG_PONKEYCTRL1,
	PMU_REG_PONKEYCTRL2,
	PMU_REG_PONKEYCTRL3,
	/* PMU Ponkey Shutdown/HardReset/Restart Reg */
	PMU_REG_KEY_PAD_LOCK,
	PMU_REG_POK_RSTPIN_ONLY,
	PMU_REG_PONKEYOFFHOLD_DEB,
	PMU_REG_PONKEY_SHUTDOWN_DLY,
	PMU_REG_PONKEY_RESTART_EN,
	PMU_REG_PONKEY_RESTART_DEB,
	PMU_REG_PONKEY_RESTART_DLY,

	PMU_REG_AUXCTRL,
	PMU_REG_PONKEY_ONHOLD_DEB,

	PMU_REG_NTCHT_RISE_LO,
	PMU_REG_NTCHT_RISE_HI,
	PMU_REG_NTCHT_FALL_LO,
	PMU_REG_NTCHT_FALL_HI,
	PMU_REG_NTCCT_RISE_LO,
	PMU_REG_NTCCT_RISE_HI,
	PMU_REG_NTCCT_FALL_LO,
	PMU_REG_NTCCT_FALL_HI,


	PMU_REG_CMPCTRL4,
	PMU_REG_CMPCTRL5,
	PMU_REG_CMPCTRL6,
	PMU_REG_CMPCTRL7,
	PMU_REG_CMPCTRL14,
	PMU_REG_RTCSC,
	PMU_REG_RTCMN,
	PMU_REG_RTCHR,
	PMU_REG_RTCDT,
	PMU_REG_RTCMT,
	PMU_REG_RTCYR,
	PMU_REG_RTCSC_ALM,
	PMU_REG_RTCMN_ALM,
	PMU_REG_RTCHR_ALM,
	PMU_REG_RTCWD_ALM,
	PMU_REG_RTCDT_ALM,
	PMU_REG_RTCMT_ALM,
	PMU_REG_RTCYR_ALM,
	PMU_REG_RTC_CORE,
	PMU_REG_RTC_C2C1_XOTRIM,
	PMU_REG_RFOPMODCTRL,
	PMU_REG_CAMOPMODCTRL,
	PMU_REG_HV1OPMODCTRL,
	PMU_REG_HV2OPMODCTRL,
	PMU_REG_HV3OPMODCTRL,
	PMU_REG_HV4OPMODCTRL,
	PMU_REG_HV5OPMODCTRL,
	PMU_REG_HV6OPMODCTRL,
	PMU_REG_HV7OPMODCTRL,
	PMU_REG_HV8OPMODCTRL,
	PMU_REG_HV9OPMODCTRL,
	PMU_REG_HV10OPMODCTRL,
	PMU_REG_SIMOPMODCTRL,
	PMU_REG_CSROPMODCTRL,
	PMU_REG_IOSROPMODCTRL,
	PMU_REG_SDSROPMODCTRL,
	PMU_REG_ASROPMODCTRL,
	PMU_REG_RFLDOCTRL,
	PMU_REG_CAMLDOCTRL,
	PMU_REG_HVLDO1CTRL,
	PMU_REG_HVLDO2CTRL,
	PMU_REG_HVLDO3CTRL,
	PMU_REG_HVLDO4CTRL,
	PMU_REG_HVLDO5CTRL,
	PMU_REG_HVLDO6CTRL,
	PMU_REG_HVLDO7CTRL,
	PMU_REG_HVLDO8CTRL,
	PMU_REG_HVLDO9CTRL,
	PMU_REG_HVLDO10CTRL,
	PMU_REG_SIMLDOCTRL,
	PMU_REG_SIMLDO2PMCTRL,
	PMU_REG_USBLDOPMCTRL,
	PMU_REG_DVSLDO1PMCTRL,
	PMU_REG_DVSLDO2PMCTRL,
	PMU_REG_DVSLDO1VSEL1,
	PMU_REG_DVSLDO1VSEL2,
	PMU_REG_DVSLDO1VSEL3,
	PMU_REG_DVSLDO2VSEL1,
	PMU_REG_DVSLDO2VSEL2,
	PMU_REG_DVSLDO2VSEL3,
	PMU_REG_SIM2OPMODCTRL,
	PMU_REG_SIMLDO2CTRL,
	PMU_REG_USBOPMODCTRL,
	PMU_REG_USBLDOCTRL,
	PMU_REG_BCDLDOCTRL,
	PMU_REG_DVS1OPMODCTRL,
	PMU_REG_DVS2OPMODCTRL,
	PMU_REG_SIMOFF_EN,
	PMU_REG_PWR_GRP_DLY,
	PMU_REG_CSRCTRL1,
	PMU_REG_CSRCTRL2,
	PMU_REG_CSRCTRL3,
	PMU_REG_CSRCTRL4,
	PMU_REG_CSRCTRL5,
	PMU_REG_CSRCTRL6,
	PMU_REG_CSRCTRL7,
	PMU_REG_CSRCTRL8,
	PMU_REG_CSRCTRL9,
	PMU_REG_CSRCTRL10,
	PMU_REG_IOSRCTRL1,
	PMU_REG_IOSRCTRL2,
	PMU_REG_IOSRCTRL3,
	PMU_REG_IOSRCTRL4,
	PMU_REG_IOSRCTRL5,
	PMU_REG_IOSRCTRL6,
	PMU_REG_IOSRCTRL7,
	PMU_REG_IOSRCTRL8,
	PMU_REG_SDSRCTRL1,
	PMU_REG_SDSRCTRL2,
	PMU_REG_SDSRCTRL3,
	PMU_REG_SDSRCTRL4,
	PMU_REG_SDSRCTRL5,
	PMU_REG_SDSRCTRL6,
	PMU_REG_SDSRCTRL7,
	PMU_REG_SDSRCTRL8,
	PMU_REG_ASRCTRL1,
	PMU_REG_ASRCTRL2,
	PMU_REG_ASRCTRL3,
	PMU_REG_ASRCTRL4,
	PMU_REG_ASRCTRL5,
	PMU_REG_ASRCTRL6,
	PMU_REG_ASRCTRL7,
	PMU_REG_ASRCTRL8,
	PMU_REG_ENV1,
	PMU_REG_ENV2,
	PMU_REG_ENV3,
	PMU_REG_ENV4,
	PMU_REG_ENV5,
	PMU_REG_ENV6,
	PMU_REG_ENV7,
	PMU_REG_ENV8,
	PMU_REG_ENV9,
	/* audio */
	PMU_REG_IHFTOP_IHF_IDDQ,
	PMU_REG_IHFTOP_BYPASS,
	PMU_REG_IHFLDO_PUP,
	PMU_REG_IHFPOP_PUP,
	PMU_REG_IHFAUTO_SEQ,
	PMU_REG_IHFPGA2_GAIN,
	PMU_REG_HIGH_GAIN_MODE,
	PMU_REG_IHF_SC_EDISABLE,
	PMU_REG_HS_SC_EDISABLE,
	PMU_REG_HSPUP_IDDQ_PWRDWN,
	PMU_REG_HSPUP_HS_PWRUP,
	PMU_REG_HSCP3_CP_CG_SEL,
	PMU_REG_HSIST_OC_DISOCMUX,
	PMU_REG_IHFPOP_POPTIME_CTL,
	PMU_REG_HSPGA1,
	PMU_REG_HSPGA2,
	PMU_REG_HSPGA1_LGAIN,
	PMU_REG_HSPGA2_RGAIN,
	PMU_REG_HSPGA3,
	PMU_REG_HSDRV_DISSC,
	PMU_REG_IHFALC_BYPASS,
	PMU_REG_IHFCLK_PUP,
	PMU_REG_IHFBIAS_EN,
	PMU_REG_IHFRCCAL_PUP,
	PMU_REG_IHFFF_PUP,
	PMU_REG_IHFRAMP_PUP,
	PMU_REG_IHFLF_PUP,
	PMU_REG_IHFCMPPD_PUP,
	PMU_REG_IHFFB_PUP,
	PMU_REG_IHFPWRDRV_PUP,
	PMU_REG_IHFNG_PUP,
	PMU_REG_IHF_NGTHRESH,
	PMU_REG_IHFPOP_EN,
	PMU_REG_IHFDRVCLAMP_DIS,
	PMU_REG_IHFCAL_SEL,
	PMU_REG_IHFPSRCAL_PUP,
	/* Charge */
	PMU_REG_CHRGR_USB_EN,
	PMU_REG_CHRGR_USB_MAINT,
	PMU_REG_CHRGR_WAC_EN,
	PMU_REG_CHRGR_ICC_FC,
	PMU_REG_CHRGR_ICC_QC,
	PMU_REG_CHRGR_VFLOAT,
	PMU_REG_CHRGR_EOC,
	PMU_REG_CHRGR_BCDLDO,
	PMU_REG_CHRGR_BCDLDO_AON,
	PMU_REG_CHP_TYP,
	PMU_REG_TCH_TIMER,
	/* fuel gauge */
	PMU_REG_FG_ACCM0,
	PMU_REG_FG_ACCM1,
	PMU_REG_FG_ACCM2,
	PMU_REG_FG_ACCM3,
	PMU_REG_FG_CNT0,
	PMU_REG_FG_CNT1,
	PMU_REG_FG_SLEEPCNT0,
	PMU_REG_FG_SLEEPCNT1,
	PMU_REG_FG_HOSTEN,
	PMU_REG_FG_RESET,
	PMU_REG_FG_CAL,
	PMU_REG_FG_FRZREAD,
	PMU_REG_FG_FRZSMPL,
	PMU_REG_FG_OFFSET0,
	PMU_REG_FG_OFFSET1,
	PMU_REG_FG_GAINTRIM,
	PMU_REG_FG_DELTA,
	PMU_REG_FG_CAP,
	PMU_REG_FG_CIC,
	/* usb control */
	PMU_REG_OTG_VBUS_PULSE,
	PMU_REG_OTG_VBUS_BOOST,
	PMU_REG_OTG_VBUS_DISCHRG,
	PMU_REG_OTG_ENABLE,
	PMU_REG_ADP_SENSE,
	PMU_REG_ADP_COMP_DB_TM,
	PMU_REG_ADP_SNS_TM,
	PMU_REG_ADP_PRB,
	PMU_REG_ADP_CAL_PRB,
	PMU_REG_ADP_PRB_MOD,
	PMU_REG_ADP_PRB_CYC_TIME,
	PMU_REG_ADP_COMP_METHOD,
	PMU_REG_ADP_ENABLE,
	PMU_REG_ADP_PRB_COMP,
	PMU_REG_ADP_PRB_REG_RST,
	PMU_REG_ADP_SNS_COMP,
	PMU_REG_ADP_SNS_AON,
	PMU_REG_OTGCTRL9,
	PMU_REG_OTGCTRL10,
	/* usb status */
	PMU_REG_USB_STATUS_ID_CODE,
	PMU_REG_OTG_STATUS_VBUS,
	PMU_REG_OTG_STATUS_SESS,
	PMU_REG_OTG_STATUS_SESS_END,
	PMU_REG_ADP_STATUS_ATTACH_DET,
	PMU_REG_ADP_STATUS_SNS_DET,
	PMU_REG_ADP_STATUS_RISE_TIMES_LSB,
	PMU_REG_ADP_STATUS_RISE_TIMES_MSB,
	/* BC ctrl n status */
	PMU_REG_BC_DET_EN,
	PMU_REG_BC_SW_RST,
	PMU_REG_BC_OVWR_KEY,	/* BC CTRL register Overwrite permission reg */

	/* test */
	PMU_REG_HSIST_I_HS_ENST,
	PMU_REG_HSIST_I_HS_IST,
	PMU_REG_IHFSTIN_I_IHFSELFTEST_EN,
	PMU_REG_IHFSTIN_I_IHFSTI,
	PMU_REG_IHFSTIN_I_IHFSTO,
	PMU_REG_IHFSTO_O_IHFSTI,
	PMU_REG_HSOUT1_O_HS_IST,
	/* interrupt */
	PMU_REG_INT_START,
	PMU_REG_INT_MSK_START,
	/* bc */
	PMU_REG_BC_STATUS_CODE,
	PMU_REG_BC_STATUS_DONE,
	PMU_REG_BC_STATUS_TO,
	PMU_REG_BC_CTRL_DET_RST,
	PMU_REG_BC_CTRL_DET_EN,
	/* generic */
	PMU_REG_SWUP,
	PMU_REG_PMUID,
	PMU_REG_PMUREV,
	PMU_REG_PLLCTRL,
	PMU_REG_SW_SHDWN,
	PMU_REG_HOSTCTRL3,
	PMU_REG_MBCCTRL5_USB_DET_LDO_EN,
	PMU_REG_MBCCTRL5_CHARGE_DET,
	PMU_REG_STATMUX,

	/* PMU Bus */
	PMU_REG_BUS_STATUS_WRITE_FIFO,
	PMU_REG_BUS_STATUS_READ_FIFO,

	/* Watchdog */
	PMU_REG_SYS_WDT_CLR,
	PMU_REG_SYS_WDT_EN,
	PMU_REG_SYS_WDT_TIME,

	/* BM access */
	PMU_REG_MBCCTRL1,
	PMU_REG_MBCCTRL2,
	PMU_REG_MBCCTRL3,
	PMU_REG_MBCCTRL4,
	PMU_REG_MBCCTRL5,
	PMU_REG_MBCCTRL6,
	PMU_REG_MBCCTRL7,
	PMU_REG_MBCCTRL8,
	PMU_REG_MBCCTRL9,
	PMU_REG_MBCCTRL10,
	PMU_REG_BBCCTRL,

	/*UAS*/
	PMU_REG_UAS_DET_MODE,
	PMU_REG_UAS_SW_GRP,
	PMU_REG_UASCTRL3,
	PMU_REG_UASCTRL4,

	/* Common Control Registers */
	PMU_REG_COMM_CTRL,

	PMU_REG_MAX,
};
enum bcmpmu_irq_reg {
	PMU_REG_INT1,
	PMU_REG_INT2,
	PMU_REG_INT3,
	PMU_REG_INT4,
	PMU_REG_INT5,
	PMU_REG_INT6,
	PMU_REG_INT7,
	PMU_REG_INT8,
	PMU_REG_INT9,
	PMU_REG_INT10,
	PMU_REG_INT11,
	PMU_REG_INT12,
	PMU_REG_INT13,
	PMU_REG_INT14,
};

enum bcmpmu_irq {
	PMU_IRQ_RTC_ALARM,
	PMU_IRQ_RTC_SEC,
	PMU_IRQ_RTC_MIN,
	PMU_IRQ_RTCADJ,
	PMU_IRQ_BATINS,
	PMU_IRQ_BATRM,
	PMU_IRQ_GBAT_PLUG_IN,
	PMU_IRQ_SMPL_INT,
	PMU_IRQ_USBINS,
	PMU_IRQ_USBRM,
	PMU_IRQ_USBOV,
	PMU_IRQ_EOC,
	PMU_IRQ_RESUME_VBUS,
	PMU_IRQ_CHG_HW_TTR_EXP,
	PMU_IRQ_CHG_HW_TCH_EXP,
	PMU_IRQ_CHG_SW_TMR_EXP,
	PMU_IRQ_CHGDET_LATCH,
	PMU_IRQ_CHGDET_TO,
	PMU_IRQ_MBTEMPLOW,
	PMU_IRQ_MBTEMPHIGH,
	PMU_IRQ_MBOV,
	PMU_IRQ_MBOV_DIS,
	PMU_IRQ_USBOV_DIS,
	PMU_IRQ_CHGERRDIS,
	PMU_IRQ_VBUS_1V5_R,
	PMU_IRQ_VBUS_4V5_R,
	PMU_IRQ_VBUS_1V5_F,
	PMU_IRQ_VBUS_4V5_F,
	PMU_IRQ_MBWV_R_10S_WAIT,
	PMU_IRQ_BBLOW,
	PMU_IRQ_LOWBAT,
	PMU_IRQ_VERYLOWBAT,
	PMU_IRQ_RTM_DATA_RDY,
	PMU_IRQ_RTM_IN_CON_MEAS,
	PMU_IRQ_RTM_UPPER,
	PMU_IRQ_RTM_IGNORE,
	PMU_IRQ_RTM_OVERRIDDEN,
	PMU_IRQ_AUD_HSAB_SHCKT,
	PMU_IRQ_AUD_IHFD_SHCKT,
	PMU_IRQ_MBC_TF,
	PMU_IRQ_CSROVRI,
	PMU_IRQ_IOSROVRI,
	PMU_IRQ_SDSROVRI,
	PMU_IRQ_ASROVRI,
	PMU_IRQ_UBPD_CHG_F,
	PMU_IRQ_ACD_INS,
	PMU_IRQ_ACD_RM,
	PMU_IRQ_PONKEYB_HOLD,
	PMU_IRQ_PONKEYB_F,
	PMU_IRQ_PONKEYB_R,
	PMU_IRQ_PONKEYB_OFFHOLD,
	PMU_IRQ_PONKEYB_RESTART,
	PMU_IRQ_IDCHG,
	PMU_IRQ_JIG_USB_INS,
	PMU_IRQ_JIG_UART_INS,
	PMU_IRQ_ID_INS,
	PMU_IRQ_ID_RM,
	PMU_IRQ_ADP_CHANGE,
	PMU_IRQ_ADP_SNS_END,
	PMU_IRQ_SESSION_END_VLD,
	PMU_IRQ_SESSION_END_INVLD,
	PMU_IRQ_VBUS_OVERCURRENT,
	PMU_IRQ_FGC,
	PMU_IRQ_MAX,
};

enum bcmpmu_adc_sig {
	PMU_ADC_VMBATT,
	PMU_ADC_VBBATT,
	PMU_ADC_VWALL,
	PMU_ADC_VBUS,
	PMU_ADC_ID,
	PMU_ADC_NTC,
	PMU_ADC_BSI,
	PMU_ADC_BOM,
	PMU_ADC_32KTEMP,
	PMU_ADC_PATEMP,
	PMU_ADC_ALS,
	PMU_ADC_RTM,
	PMU_ADC_FG_CURRSMPL,
	PMU_ADC_FG_RAW,
	PMU_ADC_FG_VMBATT,
	PMU_ADC_BSI_CAL_LO,
	PMU_ADC_BSI_CAL_HI,
	PMU_ADC_NTC_CAL_LO,
	PMU_ADC_NTC_CAL_HI,
	PMU_ADC_TEMP_SNS,
	PMU_ADC_MAX,
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
	PMU_CHRGR_TYPE_ACA,
	PMU_CHRGR_TYPE_MAX,
};

enum bcmpmu_usb_type_t {
	PMU_USB_TYPE_NONE,
	PMU_USB_TYPE_SDP,
	PMU_USB_TYPE_CDP,
	PMU_USB_TYPE_ACA,
	PMU_USB_TYPE_MAX,
};

enum bcmpmu_usb_adp_mode_t {
	PMU_USB_ADP_MODE_REPEAT,
	PMU_USB_ADP_MODE_CALIBRATE,
	PMU_USB_ADP_MODE_ONESHOT,
};

enum bcmpmu_usb_id_lvl_t {
	PMU_USB_ID_NOT_SUPPORTED,
	PMU_USB_ID_GROUND,
	PMU_USB_ID_RID_A,
	PMU_USB_ID_RID_B,
	PMU_USB_ID_RID_C,
	PMU_USB_ID_FLOAT,
};

enum {
	FG_TMP_ZONE_MIN,
	FG_TMP_ZONE_n20 = FG_TMP_ZONE_MIN,
	FG_TMP_ZONE_n15,
	FG_TMP_ZONE_n10,
	FG_TMP_ZONE_n5,
	FG_TMP_ZONE_0,
	FG_TMP_ZONE_p5,
	FG_TMP_ZONE_p10,
	FG_TMP_ZONE_p15,
	FG_TMP_ZONE_p20,
	FG_TMP_ZONE_MAX = FG_TMP_ZONE_p20,
};

struct bcmpmu_rw_data {
	unsigned int map;
	unsigned int addr;
	unsigned int val;
	unsigned int mask;
};

struct bcmpmu_reg_map {
	unsigned int map;
	unsigned int addr;
	unsigned int mask;
	unsigned int ro;
	unsigned int shift;
};

struct bcmpmu_irq_map {
	unsigned int map;
	unsigned int int_addr;
	unsigned int mask_addr;
	unsigned int bit_mask;
};

struct bcmpmu_adc_map {
	unsigned int map;
	unsigned int addr0;
	unsigned int addr1;
	unsigned int dmask;
	unsigned int vmask;
	unsigned int rtmsel;
	unsigned int vrng;
};

struct bcmpmu_adc_unit {
	unsigned int vstep;
	int voffset;
	unsigned int rpullup;
	void *lut_ptr;
	int lut_len;
	unsigned int fg_k;
	unsigned int vmax;
};

struct bcmpmu_adc_ctrl_map {
	unsigned int addr;
	unsigned int mask;
	unsigned int shift;
};

struct bcmpmu_adc_setting {
	unsigned int tx_rx_sel_addr;
	unsigned int tx_delay;
	unsigned int rx_delay;
	unsigned int sw_timeout;
	unsigned int txrx_timeout;
	unsigned int compensation_samples;
	unsigned int compensation_volt_lo;
	unsigned int compensation_volt_hi;
	unsigned int compensation_interval;
};

enum bcmpmu_adc_flags {
	PMU_ADC_RAW_AND_UNIT,
	PMU_ADC_RAW_ONLY,
	PMU_ADC_UNIT_ONLY,
};

struct bcmpmu_adc_req {
	enum bcmpmu_adc_sig sig;
	enum bcmpmu_adc_timing_t tm;
	enum bcmpmu_adc_flags flags;
	unsigned int raw;
	unsigned int cal;
	int cnv;
	bool ready;
};

struct bcmpmu_temp_map {
	int adc;
	int temp;
};

struct bcmpmu_bom_map {
	int bom;
	int low;
	int high;
};

struct bcmpmu_voltcap_map {
	int volt;
	int cap;
};

struct bcmpmu_charge_zone {
	int tl;    /* low boundary of this temperature range */
	int th;    /* High boundary of this temperature range */
	int v;     /* Charge voltage (Vfloat) in this temperature range */
	int fc;    /* percentage of 1C rate in this temperature range */
	int qc;    /* qc charging rate in this temperature range */
};

struct bcmpmu_fg_zone {
	int temp;
	int reset;
	int fct;
	int guardband;
	int esr_vl_lvl;
	int esr_vl;
	int esr_vl_slope;
	int esr_vl_offset;
	int esr_vm_lvl;
	int esr_vm;
	int esr_vm_slope;
	int esr_vm_offset;
	int esr_vh_lvl;
	int esr_vh;
	int esr_vh_slope;
	int esr_vh_offset;
	int esr_vf;
	int esr_vf_slope;
	int esr_vf_offset;
	struct bcmpmu_voltcap_map *vcmap;
	int maplen;
};

struct bcmpmu_adc_cal {
	unsigned int gain;
	unsigned int offset;
};

enum bcmpmu_ioctl {
	PMU_EM_IOCTL_ADC_REQ,
	PMU_EM_IOCTL_ADC_LOAD_CAL,
	PMU_EM_IOCTL_ENV_STATUS,
};

#define PMU_EM_ADC_REQ _IOWR(0, PMU_EM_IOCTL_ADC_REQ, struct bcmpmu_adc_req*)
#define PMU_EM_ADC_LOAD_CAL _IOW(0, PMU_EM_IOCTL_ADC_LOAD_CAL, u8*)
#define PMU_EM_ENV_STATUS _IOR(0, PMU_EM_IOCTL_ENV_STATUS, unsigned long*)

struct bcmpmu_rw_data_ltp {
	unsigned int map;
	unsigned int addr;
	unsigned int val[16];
	unsigned int mask;
	unsigned int len;
};

struct bcmpmu_ntc_data_ltp {
	unsigned int map;
	unsigned int addr;
	int val[3];
	unsigned int mask;
};

#define BCM_PMU_MAGIC   'P'
#define BCM_PMU_CMD_READ_REG            0x83
#define BCM_PMU_CMD_WRITE_REG           0x84
#define BCM_PMU_CMD_BULK_READ_REG       0x85
#define BCM_PMU_CMD_ADC_READ_REG        0x86
#define BCM_PMU_CMD_NTC_TEMP			0x87

#define BCM_PMU_IOCTL_READ_REG		\
_IOWR(BCM_PMU_MAGIC, BCM_PMU_CMD_READ_REG, struct bcmpmu_rw_data_ltp)
#define BCM_PMU_IOCTL_BULK_READ_REG		\
_IOWR(BCM_PMU_MAGIC, BCM_PMU_CMD_BULK_READ_REG, struct bcmpmu_rw_data_ltp)
#define BCM_PMU_IOCTL_ADC_READ_REG		\
_IOWR(BCM_PMU_MAGIC, BCM_PMU_CMD_ADC_READ_REG, struct bcmpmu_adc_req)
#define BCM_PMU_IOCTL_WRITE_REG		\
_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_WRITE_REG, struct bcmpmu_rw_data_ltp)
#define BCM_PMU_IOCTL_NTC_TEMP		\
_IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_NTC_TEMP, struct bcmpmu_ntc_data_ltp)

enum bcmpmu_usb_accy_t {
	USB,
	CHARGER,
	AUDIO_MONO,
	AUDIO_STEREO,
	AUDIO_HEADPHONE,
	UART,
	TTY,
	UART_JIG,
	USB_JIG,
	CARKIT,
};

enum bcmpmu_env_bit_t {
	PMU_ENV_MBWV_DELTA,
	PMU_ENV_CGPD_ENV,
	PMU_ENV_UBPD_ENV,
	PMU_ENV_UBPD_USBDET,
	PMU_ENV_CGPD_PRI,
	PMU_ENV_UBPD_PRI,
	PMU_ENV_WAC_VALID,
	PMU_ENV_USB_VALID,
	PMU_ENV_P_UBPD_INT,
	PMU_ENV_P_CGPD_CHG,
	PMU_ENV_P_UBPD_CHR,
	PMU_ENV_PORT_DISABLE,
	PMU_ENV_MBPD,
	PMU_ENV_MBOV,
	PMU_ENV_MBMC,
	PMU_ENV_MAX,
};

enum bcmpmu_event_t {
	/* events for usb driver */
	BCMPMU_USB_EVENT_USB_DETECTION,
	BCMPMU_USB_EVENT_IN,
	BCMPMU_USB_EVENT_RM,
	BCMPMU_USB_EVENT_ADP_CHANGE,
	BCMPMU_USB_EVENT_ADP_SENSE_END,
	BCMPMU_USB_EVENT_ADP_CALIBRATION_DONE,
	BCMPMU_USB_EVENT_ID_CHANGE,
	BCMPMU_USB_EVENT_VBUS_VALID,
	BCMPMU_USB_EVENT_VBUS_INVALID,
	BCMPMU_USB_EVENT_SESSION_VALID,
	BCMPMU_USB_EVENT_SESSION_INVALID,
	BCMPMU_USB_EVENT_SESSION_END_INVALID,
	BCMPMU_USB_EVENT_SESSION_END_VALID,
	BCMPMU_USB_EVENT_VBUS_OVERCURRENT,
	BCMPMU_USB_EVENT_RIC_C_TO_FLOAT,
	BCMPMU_USB_EVENT_CHGDET_LATCH,
	/* events for battery charging */
	BCMPMU_CHRGR_EVENT_CHGR_DETECTION,
	BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT,
	BCMPMU_CHRGR_EVENT_CHRG_RESUME_VBUS,
	BCMPMU_CHRGR_EVENT_MBOV,
	BCMPMU_CHRGR_EVENT_USBOV,
	BCMPMU_CHRGR_EVENT_MBTEMP,
	BCMPMU_CHRGR_EVENT_EOC,
	BCMPMU_CHRGR_EVENT_CHRG_STATUS,
	BCMPMU_CHRGR_EVENT_CAPACITY,
	/* events for fuel gauge */
	BCMPMU_FG_EVENT_FGC,
	/*Events for JIG*/
	BCMPMU_JIG_EVENT_USB,
	BCMPMU_JIG_EVENT_UART,

	BCMPMU_EVENT_MAX,
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
	BCMPMU_USB_CTRL_GET_SESSION_END_STATUS,
	BCMPMU_USB_CTRL_GET_ID_VALUE,
	BCMPMU_USB_CTRL_GET_CHRGR_TYPE,
	BCMPMU_USB_CTRL_GET_USB_TYPE,
	BCMPMU_USB_CTRL_SW_UP,
	BCMPMU_USB_CTRL_TPROBE_MAX,
};

enum bcmpmu_bc_t {
	BCMPMU_BC_BB_BC11,
	BCMPMU_BC_BB_BC12,
	BCMPMU_BC_PMU_BC12,
};

enum bcmpmu_uas_mode {
	BCMPMU_UAS_MODE_SW,
	BCMPMU_UAS_MODE_MANUAL,
	BCMPMU_UAS_MODE_HW,
	BCMPMU_UAS_MODE_MAX,
};

enum bcmpmu_uas_switch {
	UAS_SW1,
	UAS_SW2,
	UAS_SW3,
	UAS_SW4,
	UAS_SW5,
	UAS_SW6,
	UAS_SW7,
	UAS_SW8,
	UAS_SW_MAX
};

enum bcmpmu_uas_sw_grp {
	UAS_SW_GRP_USB_JIG,
	UAS_SW_GRP_AUDIO_MONO_TTY,
	UAS_SW_GRP_AUDIO_STEREO_HS,
	UAS_SW_GRP_UART_AUDIO_TYPE2,
	UAS_SW_GRP_UART_JIG,
	UAS_SW_GRP_CARKIT,
	UAS_SW_GRP_AUDIO_STEREO_HP,
	UAS_SW_GRP_OFF
};

enum bcmpmu_ponkeyonhold_deb {
	PONKEYONHOLD_0_DEB,
	PONKEYONHOLD_50MS_DEB,
	PONKEYONHOLD_100MS_DEB,
	PONKEYONHOLD_500MS_DEB,
	PONKEYONHOLD_1SEC_DEB,
	PONKEYONHOLD_2SEC_DEB,
	PONKEYONHOLD_3SEC_DEB
};

#define	PMU_ENV_BITMASK_MBWV_DELTA		(1<<0)
#define	PMU_ENV_BITMASK_CGPD_ENV		(1<<1)
#define	PMU_ENV_BITMASK_UBPD_ENV		(1<<2)
#define	PMU_ENV_BITMASK_UBPD_USBDET		(1<<3)
#define	PMU_ENV_BITMASK_CGPD_PRI		(1<<4)
#define	PMU_ENV_BITMASK_UBPD_PRI		(1<<5)
#define	PMU_ENV_BITMASK_WAC_VALID		(1<<6)
#define	PMU_ENV_BITMASK_USB_VALID		(1<<7)
#define	PMU_ENV_BITMASK_P_CGPD_CHG		(1<<8)
#define	PMU_ENV_BITMASK_P_UBPD_CHR		(1<<9)
#define	PMU_ENV_BITMASK_PORT_DISABLE		(1<<10)
#define	PMU_ENV_BITMASK_MBPD			(1<<11)
#define	PMU_ENV_BITMASK_MBOV			(1<<12)
#define PMU_ENV_BITMASK_MBMC			(1<<13)
#define	PMU_ENV_BITMASK_P_UBPD_INT		(1<<14)

struct bcmpmu_env_info {
	struct bcmpmu_reg_map regmap;
	unsigned long bitmask;
};

struct bcmpmu_batt_state {
	int capacity;
	int voltage;
	int temp;
	int present;
	int capacity_lvl;
	int status;
	int health;
};

struct bcmpmu_usb_accy_data {
	enum bcmpmu_chrgr_type_t chrgr_type;
	enum bcmpmu_usb_type_t usb_type;
	int max_curr_chrgr;
	int batt_present;
	int usb_dis;
};
/**
 * watchdog platform data
 */

#define WATCHDOG_OTP_ENABLED	(0x00000001)

struct bcmpmu_wd_setting {
	unsigned int flags;
	unsigned int watchdog_timeout;
};

/**
 * PMU revsion information
 */
struct bcmpmu_rev_info {
	u8 gen_id; /* Generation id */
	u8 prj_id; /* Project id */
	u8 dig_rev; /* Digital revision */
	u8 ana_rev; /* Analog revision */
};

struct event_notifier {
	u32 event_id;
	struct blocking_notifier_head notifiers;
};

/* referencing ACCY */
enum {
	SS_ACCY_GET_BC_STATUS = 1,
};

#ifdef CONFIG_CHARGER_BCMPMU_SPA
#define BCMPMU_SPA_EVENT_FIFO_LENGTH	16
#define SPA_FIFO_EMPTY(fifo)	((fifo.head == fifo.tail) && !fifo.fifo_full)
#define SPA_FIFO_HEAD(fifo)	(fifo.head = ((fifo.head+1) & (fifo.length-1)))
#define SPA_FIFO_TAIL(fifo)	(fifo.tail = ((fifo.tail+1) & (fifo.length-1)))
struct bcmpmu_spa_event_fifo {
	unsigned char		head;
	unsigned char		tail;
	unsigned char		length;
	bool			fifo_full;
	enum bcmpmu_event_t	event[BCMPMU_SPA_EVENT_FIFO_LENGTH];
	int			data[BCMPMU_SPA_EVENT_FIFO_LENGTH];
};
#endif

struct bcmpmu_platform_data;
struct bcmpmu {
	struct device *dev;
	void *accinfo;
	void *irqinfo;
	void *adcinfo;
	void *battinfo;
	void *chrgrinfo;
	void *rtcinfo;
	void *envinfo;
	void *fginfo;
	void *accyinfo;
	void *eminfo;
	void *ponkeyinfo;
	void *rpcinfo;
	struct bcmpmu_rev_info rev_info;

	/* event notifier */
	struct event_notifier event[BCMPMU_EVENT_MAX];

	/* reg access */
	int (*read_dev) (struct bcmpmu *bcmpmu, int reg, unsigned int *val,
			 unsigned int mask);
	int (*write_dev) (struct bcmpmu *bcmpmu, int reg, unsigned int val,
			  unsigned int mask);
	int (*read_dev_drct) (struct bcmpmu *bcmpmu, int map, int addr,
			      unsigned int *val, unsigned int mask);
	int (*write_dev_drct) (struct bcmpmu *bcmpmu, int map, int addr,
			       unsigned int val, unsigned int mask);
	int (*read_dev_bulk) (struct bcmpmu *bcmpmu, int map, int addr,
			      unsigned int *val, int len);
	int (*write_dev_bulk) (struct bcmpmu *bcmpmu, int map, int addr,
			       unsigned int *val, int len);
	/* Set PMU Bus read/write mode - Interrupt or polled */
	int (*set_dev_mode) (struct bcmpmu *bcmpmu, int poll);
	const struct bcmpmu_reg_map *regmap;
	/* irq */
	int (*register_irq) (struct bcmpmu *pmu, enum bcmpmu_irq irq,
			     void (*callback) (enum bcmpmu_irq irq, void *),
			     void *data);
	int (*unregister_irq) (struct bcmpmu *pmu, enum bcmpmu_irq irq);
	int (*mask_irq) (struct bcmpmu *pmu, enum bcmpmu_irq irq);
	int (*unmask_irq) (struct bcmpmu *pmu, enum bcmpmu_irq irq);

	/* adc */
	int (*adc_req)(struct bcmpmu *pmu, struct bcmpmu_adc_req *req);
	int (*unit_get)(struct bcmpmu *pmu, enum bcmpmu_adc_sig sig,
		struct bcmpmu_adc_unit *unit);
	int (*unit_set)(struct bcmpmu *pmu, enum bcmpmu_adc_sig sig,
		struct bcmpmu_adc_unit *unit);

	/* env */
	void (*update_env_status) (struct bcmpmu *pmu, unsigned long *env);
	 bool(*is_env_bit_set) (struct bcmpmu *pmu,
				enum bcmpmu_env_bit_t env_bit);
	 bool(*get_env_bit_status) (struct bcmpmu *pmu,
				    enum bcmpmu_env_bit_t env_bit);

	/* charge */
	int (*chrgr_usb_en) (struct bcmpmu *bcmpmu, int en);
	int (*chrgr_usb_maint) (struct bcmpmu *bcmpmu, int en);
	int (*chrgr_wac_en) (struct bcmpmu *bcmpmu, int en);
	int (*set_icc_fc) (struct bcmpmu *pmu, int curr);
	int (*set_icc_qc) (struct bcmpmu *pmu, int curr);
	int (*set_eoc) (struct bcmpmu *pmu, int curr);
	int (*set_vfloat) (struct bcmpmu *pmu, int volt);

	/* NTC  */
	int (*ntcht_rise_set) (struct bcmpmu *pmu, int val);
	int (*ntcht_fall_set) (struct bcmpmu *pmu, int val);
	int (*ntcct_rise_set) (struct bcmpmu *pmu, int val);
	int (*ntcct_fall_set) (struct bcmpmu *pmu, int val);

	/* referencing accy */
	int (*accy_info_get)(struct bcmpmu *bcmpmu, unsigned int req);

	/* fg */
	int (*fg_currsmpl) (struct bcmpmu *pmu, int *data);
	int (*fg_vmbatt) (struct bcmpmu *pmu, int *data);
	int (*fg_acc_mas) (struct bcmpmu *pmu, int *data);
	int (*fg_enable) (struct bcmpmu *pmu, int en);
	int (*fg_reset) (struct bcmpmu *pmu);
	int (*fg_offset_cal) (struct bcmpmu *pmu);
	int (*fg_offset_cal_read) (struct bcmpmu *pmu, int *data);
	int (*fg_trim_write) (struct bcmpmu *pmu, int data);
	void (*em_reset) (struct bcmpmu *pmu);
	int (*em_reset_status) (struct bcmpmu *pmu);
	int (*fg_get_capacity) (struct bcmpmu *bcmpmu);
	void (*fg_set_eoc) (struct bcmpmu *bcmpmu, int eoc);

	/* usb accy */
	struct bcmpmu_usb_accy_data usb_accy_data;
	int (*register_usb_callback) (struct bcmpmu *pmu,
				      void (*callback) (struct bcmpmu *pmu,
							unsigned char event,
							void *, void *),
				      void *data);
	int (*usb_set) (struct bcmpmu *pmu, enum bcmpmu_usb_ctrl_t ctrl,
			unsigned long val);
	int (*usb_get) (struct bcmpmu *pmu, enum bcmpmu_usb_ctrl_t ctrl,
			void *val);

	struct bcmpmu_platform_data *pdata;
	struct regulator_desc *rgltr_desc;
	struct bcmpmu_reg_info *rgltr_info;

	void *debugfs_root_dir;

	/* Client devices */
	struct platform_device *pdev[BCMPMU_REGULATOR_MAX];
};

/**
 * Data to be supplied by the platform to initialise the BCMPMU.
 *
 * @init: Function called during driver initialisation.  Should be
 *        used by the platform to configure GPIO functions and similar.
 */
struct bcmpmu_platform_data {
	struct i2c_slave_platform_data i2c_pdata;
	int (*init) (struct bcmpmu *bcmpmu);
	int (*exit) (struct bcmpmu *bcmpmu);
	struct i2c_board_info *i2c_board_info_map1;
	int i2c_adapter_id;
	int i2c_pagesize;
	int irq;
	struct bcmpmu_rw_data *init_data;
	int init_max;
	int num_of_regl;
	struct bcmpmu_regulator_init_data *regulator_init_data;

	int batt_temp_in_celsius;
	struct bcmpmu_temp_map *batt_temp_map;
	int batt_temp_map_len;
	struct bcmpmu_temp_map *batt_temp_voltmap;
	int batt_temp_voltmap_len;
	struct bcmpmu_temp_map *pa_temp_voltmap;
	int pa_temp_voltmap_len;
	struct bcmpmu_temp_map *x32_temp_voltmap;
	int x32_temp_voltmap_len;
	struct bcmpmu_temp_map *pmu_temp_map;
	int pmu_temp_map_len;
	struct bcmpmu_bom_map *bom_map;
	int bom_map_len;

	struct bcmpmu_voltcap_map *batt_voltcap_map;
	int batt_voltcap_map_len;
	struct bcmpmu_adc_setting *adc_setting;
	int fg_smpl_rate;
	int fg_slp_rate;
	int fg_slp_curr_ua;
	int fg_factor;
	int fg_sns_res;
	int chrg_1c_rate;
	int chrg_eoc;
	int support_hw_eoc;
	int batt_impedence;
	int sys_impedence;
	struct bcmpmu_charge_zone *chrg_zone_map;
	int fg_capacity_full;
	int support_fg;
	int support_chrg_maint;
	int chrg_resume_lvl;
	int tch_timer_dis;
	int fg_support_tc;
	int fg_tc_dn_lvl;
	int fg_tc_up_lvl;
	int fg_zone_settle_tm;
	struct bcmpmu_fg_zone *fg_zone_info;
	int fg_poll_hbat;
	int fg_poll_lbat;
	int fg_lbat_lvl;
	int fg_fbat_lvl;
	int fg_low_cal_lvl;
	enum bcmpmu_bc_t bc;
	int rpc_rate;
	struct bcmpmu_wd_setting *wd_setting;
	char *batt_model;
	int cutoff_volt;
	int cutoff_count_max;
	/* Ponkey shutdown/hard reset/restart settings */
	int hard_reset_en;
	int restart_en;
	int pok_hold_deb;
	int pok_shtdwn_dly;
	int pok_restart_dly;
	int pok_restart_deb;
	int pok_lock;
	int pok_turn_on_deb;
	/* IHF power up/down auto seq */
	int ihf_autoseq_dis;
#ifdef CONFIG_CHARGER_BCMPMU_SPA
	int piggyback_chrg;
	char *piggyback_chrg_name;
	void (*piggyback_notify) (enum bcmpmu_event_t event, int data);
	struct delayed_work *piggyback_work;
	struct bcmpmu_spa_event_fifo *spafifo;
	struct mutex *spalock;
#endif
	int non_pse_charging;
	int max_vfloat;
};

struct bcmpmu_fg {
	struct bcmpmu *bcmpmu;
	int fg_acc;
	int fg_smpl_cnt;
	int fg_slp_cnt;
	int fg_smpl_cnt_tm;
	int fg_slp_cnt_tm;
	int fg_slp_curr_ua;
	int fg_sns_res;
	int fg_factor;
	int fg_columb_cnt;
	int fg_ibat_avg;
};

extern const unsigned int bcmpmu_chrgr_icc_fc_settings[PMU_CHRGR_CURR_MAX];
extern const unsigned int bcmpmu_chrgr_icc_qc_settings[PMU_CHRGR_QC_CURR_MAX];
extern const unsigned int bcmpmu_chrgr_eoc_settings[PMU_CHRGR_EOC_CURR_MAX];
extern const unsigned int bcmpmu_chrgr_vfloat_settings[PMU_CHRGR_VOLT_MAX];

int bcmpmu_sel_adcsync(enum bcmpmu_adc_timing_t timing);

int bcmpmu_init_pmurev_info(struct bcmpmu *bcmpmu);
const struct bcmpmu_reg_map *bcmpmu_get_pmurev_regmap(struct bcmpmu *bcmpmu);
const struct bcmpmu_reg_map *bcmpmu_get_regmap(struct bcmpmu *bcmpmu);
const struct bcmpmu_irq_map *bcmpmu_get_irqmap(struct bcmpmu *bcmpmu);
const struct bcmpmu_adc_map *bcmpmu_get_adcmap(struct bcmpmu *bcmpmu);
struct bcmpmu_adc_unit *bcmpmu_get_adcunit(struct bcmpmu *bcmpmu);
const struct bcmpmu_reg_map *bcmpmu_get_irqregmap(struct bcmpmu *bcmpmu,
								int *len);
const struct bcmpmu_reg_map *bcmpmu_get_adc_ctrl_map(struct bcmpmu *bcmpmu);
const struct bcmpmu_env_info *bcmpmu_get_envregmap(struct bcmpmu *bcmpmu,
								int *len);
const int *bcmpmu_get_usb_id_map(struct bcmpmu *bcmpmu, int *len);
const int bcmpmu_min_supported_curr(void);

struct regulator_desc *bcmpmu_rgltr_desc(struct bcmpmu *bcmpmu);
struct bcmpmu_reg_info *bcmpmu_rgltr_info(struct bcmpmu *bcmpmu);

void bcmpmu_reg_dev_init(struct bcmpmu *bcmpmu);
void bcmpmu_reg_dev_exit(struct bcmpmu *bcmpmu);
void bcmpmu_remove_JIG_force(void);
int bcmpmu_add_notifier(u32, struct notifier_block *);
int bcmpmu_remove_notifier(u32, struct notifier_block *);
int bcmpmu_usb_set(struct bcmpmu *bcmpmu, enum bcmpmu_usb_ctrl_t ctrl,
		   unsigned long data);
int bcmpmu_usb_get(struct bcmpmu *bcmpmu, enum bcmpmu_usb_ctrl_t ctrl,
		   void *data);
void bcmpmu_client_power_off(void);
int bcmpmu_client_hard_reset(unsigned char reset_reason);
void bcmpmu_update_pdata_dt_batt(struct bcmpmu_platform_data *pdata);
void bcmpmu_update_pdata_dt_pmu(struct bcmpmu_platform_data *pdata);
int bcmpmu_reg_write_unlock(struct bcmpmu *bcmpmu);
#endif
