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

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/wakelock.h>
#include <linux/i2c.h>
#include <linux/power_supply.h>
#include <linux/platform_device.h>
#include <linux/regulator/machine.h>
#include <linux/i2c-kona.h>

#define BCMPMU_DUMMY_CLIENTS 1
#define REG_READ_COUNT_MAX	20
#define IRQ_REG_WIDTH 8
#define NUM_IRQ_REG	8

/* local debug level */

#define BCMPMU_PRINT_ERROR	(1U << 0)
#define BCMPMU_PRINT_INIT	(1U << 1)
#define BCMPMU_PRINT_FLOW	(1U << 2)
#define BCMPMU_PRINT_DATA	(1U << 3)
#define BCMPMU_PRINT_WARNING	(1U << 4)

/* LDO or Switcher def */
#define BCMPMU_LDO    0x10
#define BCMPMU_SR     0x11

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
	PMU_IRQ_FGC,
	PMU_IRQ_MBWV_F,
	PMU_IRQ_RSRV0,
	PMU_IRQ_ACD_INS,
	PMU_IRQ_ACD_RM,
	PMU_IRQ_PONKEYB_HOLD,
	PMU_IRQ_PONKEYB_F,
	PMU_IRQ_PONKEYB_R,
	PMU_IRQ_PONKEYB_OFFHOLD,
	PMU_IRQ_PONKEYB_RESTART,
	PMU_IRQ_RSRV1,
	PMU_IRQ_IDCHG,
	PMU_IRQ_JIG_USB_INS,
	PMU_IRQ_JIG_UART_INS,
	PMU_IRQ_ID_INS,
	PMU_IRQ_ID_RM,
	PMU_IRQ_RSRV2,
	PMU_IRQ_RSRV3,
	PMU_IRQ_RSRV4,
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
	UAS_SW_MAX,
};

enum bcmpmu_bc_t {
	BCMPMU_BC_BB_BC11,
	BCMPMU_BC_BB_BC12,
	BCMPMU_BC_PMU_BC12,
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

enum bcmpmu_usb_det_state_t {
	USB_IDLE,
	USB_CONNECTED,
	USB_DETECT,
	USB_RETRY,
	USB_DISCONNECTED,
};

/*State of enabled regualtor in deep sleep
Used to program PC2PC1 = 0b10 & 0b00 case
when the regulator is enabled*/
enum {
	BCMPMU_REGL_ON_IN_DSM = 1,
	BCMPMU_REGL_LPM_IN_DSM,
	BCMPMU_REGL_OFF_IN_DSM
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

struct bcmpmu59xxx_rw_data {
	unsigned int addr;
	unsigned int val;
	unsigned int mask;
};

struct bcmpmu59xxx_regulator_info {
	struct regulator_desc *rdesc;
	/* address of regulator control register for mode control */
	u32 reg_addr;
	/* address of control register to change voltage */
	u32 reg_addr_volt;
	/* Map for converting register voltage to register value */
	u32 *v_table;
	/* Size of register map */
	u32 num_voltages;
	u8 ldo_or_sr;
	/*latch state FF uninit,0 OFF, 1 ON*/
	u8 onoff;
};

struct event_notifier {
	u32 event_id;
	struct blocking_notifier_head notifiers;
};

struct bcmpmu_usb_accy_data {
	enum bcmpmu_chrgr_type_t chrgr_type;
	enum bcmpmu_usb_type_t usb_type;
	int max_curr_chrgr;
	int batt_present;
	int usb_dis;
};

struct bcmpmu_accy {
	struct bcmpmu59xxx *bcmpmu;
	const int *usb_id_map;
	int usb_id_map_len;
	wait_queue_head_t wait;
	struct delayed_work adp_work;
	struct delayed_work det_work;
	struct mutex lock;
#ifdef CONFIG_HAS_WAKELOCK
	struct wake_lock wake_lock;
#endif
	int adp_cal_done;
	enum bcmpmu_usb_det_state_t det_state;
	int otg_block_enabled;
	int adp_block_enabled;
	int adp_prob_comp;
	int adp_sns_comp;
	int retry_cnt;
	int poll_cnt;
	bool clock_en;
	enum bcmpmu_bc_t bc;
	int piggyback_chrg;
	/* event notifier */
	struct event_notifier event[BCMPMU_EVENT_MAX];
	/* usb accy */
	struct bcmpmu_usb_accy_data usb_accy_data;
	int (*usb_set) (struct bcmpmu59xxx *pmu, enum bcmpmu_usb_ctrl_t ctrl,
			unsigned long val);
	int (*usb_get) (struct bcmpmu59xxx *pmu, enum bcmpmu_usb_ctrl_t ctrl,
			void *val);
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
	u32 ref_count;
#endif
};

struct bcmpmu59xxx_pok_pdata {
	int hard_reset_en;
	int restart_en;
	int pok_hold_deb;
	int pok_shtdwn_dly;
	int pok_restart_dly;
	int pok_restart_deb;
	int pok_lock;
	int pok_turn_on_deb;
};

struct bcmpmu59xxx_audio_pdata {
	int ihf_autoseq_dis;
};

struct bcmpmu59xxx_regulator_init_data {
	int regulator ; /* Regulator ID */
	struct regulator_init_data   *initdata;
	/* Default opmode value.Pass 0xFF to
	 * skip opmode setting for a ldo/sr */
	u8 default_opmode;
	u8 dsm_mode;
};

struct bcmpmu59xxx_regulator_pdata {
	struct bcmpmu59xxx_regulator_init_data  *bcmpmu_rgltr;
	u8 num_of_rgltr;
};

struct bcmpmu59xxx_platform_data {
	struct i2c_slave_platform_data i2c_pdata;
	int (*init) (struct bcmpmu59xxx *bcmpmu);
	int (*exit) (struct bcmpmu59xxx *bcmpmu);
	u8 companion;
	struct i2c_board_info *i2c_companion_info;
	int i2c_adapter_id;
	int i2c_pagesize;
	int irq;
	struct bcmpmu59xxx_rw_data *init_data;
	int init_max;
	/*FIXME need to check this variable */
	enum bcmpmu_bc_t bc;
	int piggyback_chrg;
	char *piggyback_chrg_name;
};

struct bcmpmu59xxx {
	struct device *dev;
	struct bcmpmu59xxx_bus *pmu_bus;
	struct bcmpmu59xxx_platform_data *pdata;
	void *irqinfo;
	void *rtcinfo;
	void *accyinfo;
	void *rgltr_data;
	void *ponkeyinfo;
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

void bcmpmu_client_power_off(void);

int bcmpmu_client_hard_reset(u8 chreset_reason);

int bcmpmu_add_notifier(u32 event_id, struct notifier_block *notifier);

int bcmpmu_remove_notifier(u32 event_id, struct notifier_block *notifier);

int bcmpmu_usb_get(struct bcmpmu59xxx *bcmpmu,
		enum bcmpmu_usb_ctrl_t ctrl, void *data);

int bcmpmu_usb_set(struct bcmpmu59xxx *bcmpmu,
		enum bcmpmu_usb_ctrl_t ctrl, unsigned long data);

#ifdef CONFIG_DEBUG_FS
int bcmpmu_debugfs_open(struct inode *inode, struct file *file);
#endif
