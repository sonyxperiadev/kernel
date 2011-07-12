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
#include <linux/platform_device.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/userspace-consumer.h>

#define	PMU_BITMASK_ALL		0xFFFFFFFF

struct bcmpmu;
struct regulator_init_data;

/* Register field values for regulator. */
#define LDO_NORMAL              0   /* FOR LDO and Switchers it is NORMAL ( NM/NM1 for SRs).*/
#define LDO_STANDBY             1   /* FOR LDO and Swtichers it is STANDBY( LPM for SRs ). */
#define LDO_OFF                 2   /* OFF.*/
#define LDO_RESERVED_SR_IDLE    3   /* For LDO it is reserved. For CSR, IOSR, SDSR this is NM2 for SRs */
/* LDO or Switcher def */
#define BCMPMU_LDO    0x10
#define BCMPMU_SR     0x11

int bcmpmu_register_regulator(struct bcmpmu *bcmpmu, int reg,
			      struct regulator_init_data *initdata);

struct bcmpmu_reg_info 
{
	u8  reg_addr;		/* address of regulator control register for mode control */
	u8  reg_addr_volt;	/* address of control register to change voltage */
	u8  reg_addr_volt_l;
	u8  reg_addr_volt_t;
	u32 en_dis_mask;	/* Mask for enable/disable bits */
	u32 en_dis_shift;	/* Shift for enable/disalbe bits */
	u32 vout_mask;		/* Mask of bits in register */
	u32 vout_shift;		/* Bit shift in register */
	u32 vout_mask_l;	/* Mask of bits in register */
	u32 vout_shift_l;	/* Bit shift in register */
	u32 vout_mask_t;	/* Mask of bits in register */
	u32 vout_shift_t;	/* Bit shift in register */
	u32 *v_table;		/* Map for converting register voltage to register value */
	u32 num_voltages;	/* Size of register map */
	u32 mode;
	u32 mode_mask;
	u8 ldo_or_sr;
};

struct bcmpmu_regulator_init_data
{
	int regulator ; /* Regulator ID */
	struct regulator_init_data   *initdata;
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
	BCMPMU_REGULATOR_CSR,
	BCMPMU_REGULATOR_IOSR,
	BCMPMU_REGULATOR_SDSR,
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
	PMU_REG_AUXCTRL,
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
	PMU_REG_PWR_GRP_DLY,
	PMU_REG_CSRCTRL1,
	PMU_REG_CSRCTRL2,
	PMU_REG_CSRCTRL3,
	PMU_REG_CSRCTRL4,
	PMU_REG_CSRCTRL5,
	PMU_REG_CSRCTRL6,
	PMU_REG_CSRCTRL7,
	PMU_REG_CSRCTRL8,
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
	PMU_REG_IHFTOP_IHF_IDDQ,
	PMU_REG_IHFLDO_PUP,
	PMU_REG_IHFPOP_PUP,
	PMU_REG_IHFPGA2_GAIN,
	PMU_REG_HSPUP1_IDDQ_PWRDWN,
	PMU_REG_HSPUP2_HS_PWRUP,
	PMU_REG_HSPGA1_GAIN,
	PMU_REG_HSPGA2_GAIN,
	PMU_REG_PMUID,
	PMU_REG_PMUREV,
	PMU_REG_PLLCTRL,
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
	PMU_REG_SIMLDOEN,
	PMU_REG_SIMLDO2EN,
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
	PMU_IRQ_UART_INS,
	PMU_IRQ_ID_INS,
	PMU_IRQ_ID_RM,
	PMU_IRQ_MAX,
};

enum bcmpmu_adc_sig {
	PMU_ADC_VMBATT,
	PMU_ADC_VBBATT,
	PMU_ADC_VBUS,
	PMU_ADC_ID,
	PMU_ADC_NTC,
	PMU_ADC_BSI,
	PMU_ADC_32KTEMP,
	PMU_ADC_RTM,
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
	PMU_ADC_TM_HK,
	PMU_ADC_TM_MAX,
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

struct bcmpmu_adc_ctrl_map {
	unsigned int addr;
	unsigned int mask;
	unsigned int shift;
};

struct bcmpmu_adc_setting {
	unsigned int tx_rx_sel_addr;
	unsigned int tx_delay;
	unsigned int rx_delay;
};

struct bcmpmu_adc_req {
	enum bcmpmu_adc_sig sig;
	enum bcmpmu_adc_timing_t tm;
	unsigned int raw;
	unsigned int cal;
	unsigned int cnv;
	bool ready;
	struct list_head list;
};

struct bcmpmu_temp_map {
	int adc;
	int temp;
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

enum bcmpmu_batt_event {
	PMU_BATT_EVENT_WAKEUP,
	PMU_BATT_EVENT_CHRGR_INS,
	PMU_BATT_EVENT_CHRGR_RMV,
	PMU_BATT_EVENT_USB_INS,
	PMU_BATT_EVENT_USB_RMV,
	PMU_BATT_EVENT_USB_ENUM_500,
	PMU_BATT_EVENT_USB_ENUM_100,
	PMU_BATT_EVENT_USB_ENUM_0,
	PMU_BATT_EVENT_SUSPEND,
	PMU_BATT_EVENT_RESUME,
};

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
	PMU_ENV_P_CGPD_CHG,
	PMU_ENV_P_UBPD_CHR,
	PMU_ENV_PORT_DISABLE,
	PMU_ENV_MAX,
};

#define	PMU_ENV_BITMASK_MBWV_DELTA		0x00001
#define	PMU_ENV_BITMASK_CGPD_ENV		0x00002
#define	PMU_ENV_BITMASK_UBPD_ENV		0x00004
#define	PMU_ENV_BITMASK_UBPD_USBDET		0x00008
#define	PMU_ENV_BITMASK_CGPD_PRI		0x00010
#define	PMU_ENV_BITMASK_UBPD_PRI		0x00020
#define	PMU_ENV_BITMASK_WAC_VALID		0x00040
#define	PMU_ENV_BITMASK_USB_VALID		0x00080
#define	PMU_ENV_BITMASK_P_CGPD_CHG		0x00100
#define	PMU_ENV_BITMASK_P_UBPD_CHR		0x00200
#define	PMU_ENV_BITMASK_PORT_DISABLE		0x00400

struct bcmpmu_env_info {
 	struct bcmpmu_reg_map regmap;
	unsigned long bitmask;
};

struct bcmpmu_platform_data;
struct bcmpmu {
	struct device *dev;
	void *coreinfo;
	void *accinfo;
	void *irqinfo;
	void *adcinfo;
	void *battinfo;
	void *chrgrinfo;
	void *rtcinfo;

	int (*read_dev)(struct bcmpmu *bcmpmu, int reg, unsigned int *val, unsigned int mask);
	int (*write_dev)(struct bcmpmu *bcmpmu, int reg, unsigned int val, unsigned int mask);
	int (*read_dev_drct)(struct bcmpmu *bcmpmu, int map, int addr, unsigned int *val, unsigned int mask);
	int (*write_dev_drct)(struct bcmpmu *bcmpmu, int map, int addr, unsigned int val, unsigned int mask);
	int (*read_dev_bulk)(struct bcmpmu *bcmpmu, int map, int addr, unsigned int *val, int len);
	int (*write_dev_bulk)(struct bcmpmu *bcmpmu, int map, int addr, unsigned int *val, int len);
	const struct bcmpmu_reg_map *regmap;
	
	int (*register_irq)(struct bcmpmu *pmu, enum bcmpmu_irq irq,
		void (*callback)(enum bcmpmu_irq irq, void *), void *data);
	int (*unregister_irq)(struct bcmpmu *pmu, enum bcmpmu_irq irq);
	int (*mask_irq)(struct bcmpmu *pmu, enum bcmpmu_irq irq);
	int (*unmask_irq)(struct bcmpmu *pmu, enum bcmpmu_irq irq);

	int (*adc_req)(struct bcmpmu *pmu, struct bcmpmu_adc_req *req);

	void (*update_env_status)(struct bcmpmu *pmu, unsigned long *env);
	bool (*is_env_bit_set)(struct bcmpmu *pmu, enum bcmpmu_env_bit_t env_bit);
	bool (*get_env_bit_status)(struct bcmpmu *pmu, enum bcmpmu_env_bit_t env_bit);

	int (*set_chrg_curr)(struct bcmpmu *pmu, int curr);
	int (*set_chrg_volt)(struct bcmpmu *pmu, int volt);
	int (*charging_mgr)(struct bcmpmu *pmu, enum bcmpmu_batt_event event);
	int (*metering_mgr)(struct bcmpmu *pmu, enum bcmpmu_batt_event event);

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
	int (*init)(struct bcmpmu *bcmpmu);
	int (*exit)(struct bcmpmu *bcmpmu);
	struct i2c_board_info *i2c_board_info_map1;
	int i2c_adapter_id;
	int i2c_pagesize;
	int irq;
	u32 baseaddr;
	struct bcmpmu_rw_data *init_data;
	int init_max;
	struct bcmpmu_regulator_init_data *regulator_init_data;
	const struct bcmpmu_temp_map *batt_temp_map;
	int batt_temp_map_len;
	const struct bcmpmu_adc_setting *adc_setting;
};

int bcmpmu_clear_irqs(struct bcmpmu *bcmpmu);
int bcmpmu_sel_adcsync(enum bcmpmu_adc_timing_t timing);

const struct bcmpmu_reg_map *bcmpmu_get_regmap(void);
const struct bcmpmu_irq_map *bcmpmu_get_irqmap(void);
const struct bcmpmu_adc_map *bcmpmu_get_adcmap(void);
const struct bcmpmu_reg_map *bcmpmu_get_irqregmap(int *len);
const struct bcmpmu_reg_map *bcmpmu_get_adc_ctrl_map(void);
const struct bcmpmu_env_info *bcmpmu_get_envregmap(int *len);

const struct regulator_desc *bcmpmu_rgltr_desc(void);
const struct bcmpmu_reg_info *bcmpmu_rgltr_info(void);

void bcmpmu_reg_dev_init(struct bcmpmu *bcmpmu);
void bcmpmu_reg_dev_exit(struct bcmpmu *bcmpmu);

#endif
