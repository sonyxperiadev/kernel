/**
\file adux1050.h
ADUX1050 controller driver header.
This file is the header file for the ADUX1050 driver source.
It also has chip structure prototype and the platform data prototype.<br>
Copyright 2015 Analog Devices Inc. <br>
Licensed under the GPL version 2 or later.
*/
/*
 * Copyright (C) 2016 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#ifndef __LINUX_INPUT_ADUX1050_H__
#define __LINUX_INPUT_ADUX1050_H__

/*--------------------------------------------------
 REGISTER BASED DEFINES associated with the driver
---------------------------------------------------*/
/**
 Device ID Register Address
*/
#define DEV_ID_REG			(0x0)
/**
 Power and control Register
*/
#define CTRL_REG			(0x1)
/**
 Conversion time control Register
*/
#define CONV_TIME_CTRL_REG		(0x2)
/**
 Filter and interrupt control
*/
#define INT_CTRL_REG			(0x3)
/**
 Conversion and sync input config register
*/
#define CONV_SYNC_CFG_REG		(0x4)
/**
 Baseline / Ambient control Register Address
*/
#define BASELINE_CTRL_REG		(0x5)
/**
 Stage 0 configuration register
*/
#define CONFIG_STG0_REG			(0x6)
/**
 Stage 0 high threshold register
*/
#define HIGH_TH_STG0_REG		(0x7)
/**
 Stage 0 low threshold register
*/
#define LOW_TH_STG0_REG			(0x8)
/**
 DAC offset for stage 0
*/
#define OFFSET_DAC_STG0_REG		(0x9)
/**
 Hysteresis settings for stage 0
*/
#define HYS_STG0_REG			(0xA)
/**
 Interrupt status register
*/
#define INT_STATUS_REG			(0x70)
/**
 Stage 0 CDC result data register
*/
#define RESULT_STG0_REG			(0x71)
/**
 Peak to peak noise mesurement result register
*/
#define PK2P_STG0_REG			(0x75)
/**
 Register address of stage 0 baseline
*/
#define BASELINE_STG0_REG		(0x79)
/**
 Register address of stage 1 baseline
*/
#define BASELINE_STG1_REG		(0x7A)
/**
 Register address of stage 2 baseline
*/
#define BASELINE_STG2_REG		(0x7B)
/**
 Register address of stage 3 baseline
*/
#define BASELINE_STG3_REG		(0x7C)
/**
 Proximity status of all stages
*/
#define PROX_STATUS_REG			(0x7D)
/**
 Lowest Read/Write accessible register
*/
#define HIGHEST_WR_ACCESS		(0x19)
/**
 Highest accesible register
*/
#define HIGHEST_READ_REG		(0x7D)
/**
Total number of register count used in the initialization.
\note
This should be kept exactly as the register count in the
initialization of the platform device structure
adux1050_platform_data.
*/
#define REGCNT		(28)

/*-------------------------------------------------------------
 Non-Register defines
---------------------------------------------------------------*/
/**
default proxy work frequncy
*/
#define DEFAULT_CNT		(15)
/**
maximum iterations for binary search
*/
#define MAX_SEARCH_DEPTH	(4)
/**
 Total stages count
*/
#define TOTAL_STG			(4)
/**
Total CIN Count
*/
#define TOTAL_CIN				(4)
/**
adux1050 vendor name.
*/
#define VENDOR_NAME			"adi"
/**
 Device name
*/
#define DEVICE_NAME			"adux1050"
/**
 Driver name of this adux1050 driver
*/
#define DRIVER_NAME			"adux1050_generic"
/**
 This hold the product ID of ADUX1050
*/
#define ADUX1050_GENERIC_ID		(0x5000)
/**
 Device/product ID mask
*/
#define ADUX1050_ID_MASK		(0xFF00)
/**
 ADUX1050 revision ID mask
*/
#define REV_ID_MASK			(0xF0)
/**
 ADUX1050 metal revision ID mask
*/
#define METAL_REV_ID_MASK		(0x0F)
/**
 Metal version 50A0/50A1
*/
#define MET_VER1		(1)
/**Extracting metal ID alone
*/
#define METAL_ID(x)		(x & METAL_REV_ID_MASK)
/**Extracting REV ID alone
*/
#define REV_ID(x)		(x & (REV_ID_MASK >> 4))
/**Extracting metal product ID alone
*/
#define PRODUCT_ID(x)		(x >> 8)
/**
Mask 8 LSB bits
*/
#define LOW_BYTE_MASK		(0x00FF)
/**
Mask 4 LSB bits
*/
#define LOW_NIBBLE_MASK		(0x000F)
/**
 Low data mask for the register
*/
#define ADDR_MASK			(0xFFFF0000)
/**
 High mask for the register address
*/
#define DATA_MASK			(0x0000FFFF)
/**
 ADUX1050 Power MODE mask
*/
#define PWR_MODE_MASK			(0x3)
/**
 ADUX1050 mask for getting the number of stages
*/
#define NUM_STAGE_MASK			(0xC)
/**
The Auto Threshold mask value
*/
#define AUTO_TH_MASK			(0x4)
/**
 Default ADUX1050 force calibration mask
*/
#define FORCE_CAL_MASK			(0x8000)
/**
 Inverted force calibration mask
*/
#define ANTI_FORCE_CAL_MASK		(0x7FFF)
/**
 Reset mask for ADUX1050
*/
#define RESET_MASK			(0x0800)
/**
 Hexa-decimal number base value
*/
#define HEX_BASE			(16)
/**
 Decimal base value
*/
#define DECIMAL_BASE			(10)
/**
 Minus char
*/
#define MINUS_CHAR			'-'
/**
 Space char
*/
#define SPACE_CHAR			" "
/**
 Minus value
*/
#define MINUS_VAL			(-1)
/**
 Parser address field value in the cmd parsing
*/
#define PARSE_ADDR			(0)
/**
 Parser value field value in the cmd parsing
*/
#define PARSE_CNT			(1)
/**
 Parser data field start value in the cmd parsing
*/
#define PARSE_DATA			(2)
/**
 I2C transfer length for a single read/write operation
*/
#define I2C_WRMSG_LEN			(2)
/**
 I2C read/write operation buffer size max
*/
#define MAX_ADUX1050_WR_LEN		(24)
/**
 Default I2C read/write length
*/
#define DEF_WR				(1)
/**
 Count of global control registers
*/
#define GLOBAL_REG_CNT			(6)
/**
 Count of stage configuration control registers
*/
#define STG_CNF_CNT			(20)
/**
 Count of status registers
*/
#define STATUS_REG_CNT			(14)
/**
 Maximum DAC offset value
*/
#define MAX_OFFSET			(510) /* INSTEAD of 512 */
/**
 Positive swap clear
*/
#define CLR_POS_SWAP			(0xBFFF)
/**
 Negative swap clear
*/
#define CLR_NEG_SWAP			(0xDFFF)
/**
 Positive swap set
*/
#define SET_POS_SWAP			(0x4000)
/**
 Negative swap set
*/
#define SET_NEG_SWAP			(0x2000)
/**
 Clear interrupt enable register
*/
#define DISABLE_DEV_INT			(0x1FF)
/**
 Interrupt polarity mask for ACTIVE_LOW
*/
#define ACTIVE_LOW			(0x0)
/**
 Interrupt polarity mask for ACTIVE_HIGH
*/
#define ACTIVE_HIGH			(0x200)
/**
 To get the hysteresis value from the register.
*/
#define HYS_BYTE_MASK			(0x00FF)
/**
 Initial DAC compensation codes in saturation.
*/
#define DAC_CODEOUT_SAT			(60000)
/**
 Configuration file parameters count
*/
#define FILP_PARAM_CNT			(5)
/**
 Zero's value
 */
#define ZERO_VAL			(0)
/**
 ADUX1050 full scale value
*/
#define FULL_SCALE_VALUE		(0xFFFF)
/**
 ADUX1050 zero scale value
*/
#define ZERO_SCALE_VALUE		(0x0000)
/**
 Conversion complete bit location
*/
#define CONV_COMPLETE_BIT               (0x100)

/**
 ADUX1050 half scale value
*/
#define HALF_SCALE_VAL			(FULL_SCALE_VALUE / 2)
/**
 To get the low status
*/
#define GET_LOW_STATUS(status)		(((status) & 0xF0) >> 4)
/**
 To get the high status
*/
#define GET_HIGH_STATUS(status)		((status) & 0x0F)
/**
 To get the Conversion complete status
*/
#define GET_CONV_STATUS(status)		((status & 0x0100) >> 8)
/**
 Set power mode
*/
#define SET_PWR_MODE(pwr, mode)		(((pwr) & ~PWR_MODE_MASK) | (mode))
/**
 Get power mode
*/
#define GET_PWR_MODE(pwr)		((pwr) & PWR_MODE_MASK)
/**
 Get digital offset
*/
#define GET_DIGI_OFFSET(x, y)		(((x)-(y))/16)
/**
 Digital Offset's allowed size
*/
#define DIGI_OFFSET_SIZE		(2032)
/**
 Clamp for the digital offset
*/
#define CLAMP_DIGI_OFFSET(x)	(((x) > 127) ? 127 : ((-(x) > 127) ? -127 : x))
/**
 Get the hysteresis register number
*/
#define GET_HYS_REG(stg)		(((stg) * 5) + HYS_STG0_REG)
/**
 Get the CONFIG_STGx register
*/
#define GET_CONFIG_REG(stg)		(((stg) * 5) +  CONFIG_STG0_REG)
/**
 GET_AFE_REG to get the current stages AFE register address
*/
#define GET_OFFSET_REG(stg)		(((stg)*5) + OFFSET_DAC_STG0_REG)
/**
 To get the high threshold register value
*/
#define GET_HIGH_TH_REG(stg)		(((stg) * 5) + HIGH_TH_STG0_REG)
/**
 To get the low threshold register value
*/
#define GET_LOW_TH_REG(stg)		(((stg) * 5) + LOW_TH_STG0_REG)
/**
 To get the phase conversion time
*/
#define GET_CONV_TIME(reg)		(((((reg) & 0x3FC0) >> 6) > 20) ? \
					(((reg) & 0x3FC0) >> 6) : 20)
/**
Check for "Is the Measure noise bit set"
*/
#define IS_NOISE_MEASURE_EN(stg_cfg)	((stg_cfg) & 0x0400)
/**
 To get the Noise Sample count
*/
#define GET_NOISE_SAMPLE(cv_time_ctrl)	(1 << (((cv_time_ctrl) >> 14) + 1))
/**
 Get number of measurement stages
*/
#define GET_NUM_STG(pwr)		((((pwr) & NUM_STAGE_MASK) >> 2) + 1)
/**
 Get BASE_LINE register
*/
#define GET_BASE_LINE_REG(stg)		((stg) + BASELINE_STG0_REG)
/**
 Get noise measurement register
*/
#define GET_NOISE_MES_REG(stg)		((stg) + PK2P_STG0_REG)
/**
 Get result register for a stage
*/
#define GET_RESULT_REG(stg)		((stg) + RESULT_STG0_REG)
/**
 Get AUTO_WAKE_UP time
*/
#define GET_AUTO_WAKE_TIME(pwr)		(((pwr)  >> 12) * 20)
/**
 Get conversion time for ADUX1050 in TIMED CONV mode
*/
#define GET_TIMED_CONV_TIME(pwr)	(((((pwr) & 0x00F0) >> 4) + 1) * 100)
/**
 To check if the minimum time setting is done
*/
#define CHECK_MIN_TIME(cv_time_ctrl)	(((cv_time_ctrl) & ~(0xFFE0)) || \
					!((cv_time_ctrl) & 0x20))
/**
 To check whether Conversion complete interrupt is enabled or not
*/
#define CHECK_CONV_EN(int_ctrl)	 (!(((int_ctrl) & CONV_COMPLETE_BIT) >> 8))
/**
 To check whether High threshold interrupt is enabled or not
*/
#define CHECK_THRESH_HIGH_EN(int_ctrl)	((~((int_ctrl) & (0xF))) & 0xF)
/**
 To check whether Low threshold interrupt is enabled or not
*/
#define CHECK_THRESH_LOW_EN(int_ctrl)	((~(((int_ctrl) & (0xF0)) >> 4)) & 0xF)

/**
 To check whether SW Rest bit is enabled or not
*/
#define CHK_SW_RESET_EN(ctrl_reg)		(ctrl_reg & RESET_MASK)
/**
 To Clear SW Reset enable
 */
 #define CLR_SW_RESET_EN(ctrl_reg)		(ctrl_reg & ~RESET_MASK)
 /**
 To Set SW Reset enable
 */
 #define SET_SW_RESET_EN(ctrl_reg)		(ctrl_reg | RESET_MASK)
 /**
 To Check for Force Calibration enable
 */
 #define CHK_FORCE_CALIB_EN(bs_reg)	(bs_reg & FORCE_CAL_MASK)
/**
 To Clear Force Calibration mask
*/
#define CLR_FORCE_CALIB_EN(bs_reg)	(bs_reg & ~FORCE_CAL_MASK)
/**
To Force calibrate the device
*/
#define SET_FORCE_CALIB_EN(bs_reg)	(bs_reg | FORCE_CAL_MASK)
/**
To set Minimal AVG and OSR use the following
*/
#define SET_MIN_TIME(cv_time_ctrl)	(((cv_time_ctrl) & 0xFFC0) | 0x20)
/**
 Get the DAC step size for the given cin range
*/
#define GET_ARB_DAC_STEP_SIZE(cin)	((1 << (cin)) * (650))
/**
To get the AVG parameter from conv_time register
*/
#define GET_AVG_CONV(conv_tmr)		(((conv_tmr) & 0x3) + 1)
/**
To icalculate the AVG multiplier from avg
*/
#define CALC_AVG_CONV(avg)		(((avg) > 1) ? (1 << (avg)) : (avg))
/**
To get the OSR parameter from the conv_time register
*/
#define GET_OSR_CONV(conv_tmr)		(((conv_tmr) & 0x38) >> 3)
/**
To get the OSR parameter from the conv_time register
*/
#define CALC_OSR_CONV(osr)		(((osr) < 4) ? 1 : (1 << ((osr) - 4)))
/**
 Get CIN_RANGE value
*/
#define GET_CIN_RANGE(ctrl_reg)		(((ctrl_reg) & 0x0300) >> 8)
/**
 Used to set the required CIN_range of the device
*/
#define SET_CIN_RANGE(pwr, cin)		(((pwr) & ~(0x0300)) | ((cin) << 8))
/**
 Macro to get the DAC step size to increment for the given CDC difference & cin
*/
#define GET_DAC_STEP_SIZE(x, cin)	((u32)(x) / ((1 << (cin)) * 650))
/**
 Macro to get the POS AFE value
*/
#define LD_POS_DAC_OFFSET(x, y)	((((x) & 0xFF00) >> 8) * \
				 (((y) & 0x4000) ? -1 : 1))
/**
 Macro to get the NEG AFE value
*/
#define LD_NEG_DAC_OFFSET(x, y)	(((x) & 0x00FF) * (((y) & 0x2000) ? 1 : -1))
/**
 Macro to set the POS AFE value
*/
#define ST_POS_DAC_OFFSET(x)	((x > 255) ? ((x - 255) << 8) | 0x00FF : x << 8)
/**
 Macro to set the NEG AFE value
*/
#define ST_NEG_DAC_OFFSET(x)	((-x > 255) ? (-x - 255) | 0xFF00 : (-x))
/**
To get the 60 percent of a value.
*/
#define GET_60_PERCENT(x)	(((x)*3)/5)
/**
To check the calibration bit statua
*/
#define CHECK_CAL_STATE(x, y)	((x) & (1 << (y)))
/**
To clear the calibration status bit
*/
#define CLR_CAL_STATUS(x, y)	((x) = (x) & ~(1 << (y)))
/**
 DAC calibration routine maximum loop count
*/
#define CALIB_LOOP_CNT			(10)
/**
I2c retry count for the i2c transfer
*/
#define I2C_RETRY_CNT			(3)
/**
Maximum DAC calibration target
*/
#define MAX_CALIB_TARGET		(63000)
/**
Minumum DAC calibration target
*/
#define MIN_CALIB_TARGET		(2000)
/**
Force calibration delay
*/
#define FORCE_CALIB_SLEEP_TIME	(300)


#ifdef CONFIG_ADUX1050_POLL

/**
Minimum msleep time
 */
#define MSLEEP_MIN_TIME (20)
/**
Min Polling time
*/
#define MIN_POLL_DELAY (10)
/*
Milli sec to micro second convertion factor
 */
#define MS_TO_US (1000)
#endif
#ifdef CONFIG_USE_FILP
/*
ASCII Values for 0 is 30
		 1 is 31
		 9 is 39
		 A is 41
		 F is 46
		\t is 09
*/
#define ASCII_0				(0x30)
#define ASCII_9				(0x39)
#define ASCII_A				(0x41)
#define ASCII_F				(0x46)
#define ASCII_TAB			(0x09)

#define LINE_BREAK			("\n")
#define TAB_BREAK			("\t")
#define PRE_LINE_BREAK			("\r")
/** Maximum Read Size of Filp
*/
#define MAX_FILP_READ_SIZE	(1024)
#endif
#define TRUE				(1)
#define FALSE				(0)
/**to convert proxy time units to seconds*/
#define PROXY_TIME		(1000)

#ifdef CONFIG_SOMC_EXTENSION
#define LONG_PRESS_TIME		(1000)
#endif

/**
Event packing for CDC Interrupt
*/
#define PACK_FOR_CDC_EVENT(cdc_value, stage_num) ((cdc_value << HEX_BASE) |\
						 (stage_num << 4) |\
						 (CONV_COMPLETE))
/**
Event packing for Active event
*/
#define PACK_FOR_ACTIVE_STATE(stg_num, th_type)	((TH_ACTIVE << 12) |\
						 (th_type << 8) |\
						 (stg_num << 4) |\
						 (THRESHOLD_CROSS))
/**
Event packing for Idle event
*/
#define PACK_FOR_IDLE_STATE(stg_num, th_type)	((TH_IDLE << 12) |\
						 (th_type << 8) |\
						 (stg_num << 4) |\
						 (THRESHOLD_CROSS))

/**
	Driver state
*/
enum device_state {
	ADUX1050_DISABLE = 0,
	ADUX1050_ENABLE,
	ADUX1050_SUSPEND
};

/**
	DAC Calibration return status
*/
enum calib_ret_status {
	CAL_RET_NONE = 0,
	CAL_RET_EXIST,
	CAL_RET_SUCCESS,
	CAL_RET_FAIL,
	CAL_RET_PENDING
};
/**
	Interrupt types of ADUX1050
*/
enum interrupt_types {
	CONV_COMPLETE = 0,
	THRESHOLD_CROSS
};
/**
	Threshold types
*/
enum threshold_types {
	TH_LOW = 0,
	TH_HIGH
};
/**
	Threshold states
*/
enum threshold_states {
	TH_IDLE = 0,
	TH_ACTIVE
};
/**
	Allowed power modes
*/
enum power_mode {
	PWR_STAND_BY = 0,
	PWR_FULL_POWER,
	PWR_TIMED_CONV,
	PWR_AUTO_WAKE
};
/**
	Allowed stage numbers
 */
enum stage_no {
	STG_ZERO = 0,
	STG_ONE,
	STG_TWO,
	STG_THREE
};
/**
	Allowed Cin Range
 */
enum cin_range {
	PICO_5 = 0,
	PICO_2P5,
	PICO_1P25,
	PICO_0P625
};
/**
	CIN Connection status
*/
enum cin_con {
	CIN_NOT_CONNECTED = 0,
	CIN_CONNECTED
};
/**
	CIN Connection setup
*/
enum cin_conn_setup {
	CIN_GND = 0,
	CIN_NEG_INPUT,
	CIN_POS_INPUT,
	CIN_SHIELD
};
/**
 enum for the conversion time calculation
*/
enum conv_calc_factor {
	TWICE_CONV_DELAY_TIME,
	CONV_TIME,
	CONV_DELAY_TIME
};
/**
 READ function prototype for ADUX1050
*/
typedef int32_t (*adux1050_read_t)(struct device *, uint8_t, uint16_t*,
				   uint16_t);
/**
 WRITE function prototype for ADUX1050
*/
typedef int32_t (*adux1050_write_t)(struct device *, uint8_t, uint16_t*,
				    uint16_t);

/**
Structure to hold register values from Device tree / Run time modifications
*/
struct adux1050_registers {
	bool wr_flag;
	u16	 value;
};

/**
Platform data structure of ADUX1050_SLD driver
*/
struct adux1050_platform_data {
	/**
	 This hold the initial register configurations of ADUX1050.
	*/
	unsigned int init_regs[REGCNT];
	/**
	 This holds the calbirated Factory baseline.
	*/
	unsigned short cal_fact_base[TOTAL_STG];
	/**
	 This holds the calibrated DAC offset.
	*/
	unsigned short cal_offset[TOTAL_STG];
	/**
	 This holds the digital offset for the calibrated stage.
	*/
	unsigned short digi_offset[TOTAL_STG];
	/**
	 This holds the required factory baseline.
	*/
	unsigned short stg_cfg[TOTAL_STG];
	/**
	 This holds the required stage 0 baseline.
	*/
	unsigned short req_stg0_base;
	/**
	 This holds the required stage 1 baseline.
	*/
	unsigned short req_stg1_base;
	/**
	 This holds the required stage 2 baseline.
	*/
	unsigned short req_stg2_base;
	/**
	 This holds the required stage 3 baseline.
	*/
	unsigned short req_stg3_base;
	/**
	 This holds the irq flags for the device.
	*/
	unsigned short irq_flags;
};

struct adux1050_stage_info {
	enum cin_con status;
	unsigned char index;
};
struct dac_calib_structure {
	/** Interrupt enable setting */
	u16 enable_setting;
	/** Overall DAC calibration status*/
	u8 cal_flags;
	/** Used to denote to do or clear the DAC calibration
	for the next calibration call*/
	u8 action_flag;
	/** calibration status for each stage*/
	u8 stg_cal_stat;
	/** Saturation complete status */
	u8 sat_comp_stat;
};
/**
 This is the chip structure which contains software structures and
 other essential for the operation of the device.
*/
struct adux1050_chip {
	/** This holds the registers value */
	struct adux1050_registers reg[GLOBAL_REG_CNT + STG_CNF_CNT];
	/** This holds the baseline registers value */
	struct adux1050_registers bs_reg[TOTAL_STG];
	/** Platform data pointer*/
	struct adux1050_platform_data *pdata;
	/** Stages information*/
	struct adux1050_stage_info *stg_info;
	/** Registered input device pointer*/
	struct input_dev *input;
	/** Registered workstructure for the IRQ*/
	struct work_struct work;
	/** Registered workstructure for the DAC calibration work*/
	struct work_struct calib_work;
	/** Registered delayed work for force calibration on proximity*/
	struct delayed_work proxy_work;
	/** I2C client device pointer*/
	struct device *dev;
	/** Chip mutex*/
	struct mutex mutex; /*Chip Mutex*/
	/** Device calibration status structure*/
	struct dac_calib_structure dac_calib;
#ifdef CONFIG_OF
	/** DT node pointer*/
	struct device_node *dt_device_node;
#endif
#ifdef CONFIG_ADUX1050_POLL
    /** Kernel Polling Task struct for the adux1050.*/
	struct task_struct *polling_task;
#endif
	/** Read op pointer*/
	adux1050_read_t read;
	/** Write op pointer*/
	adux1050_write_t write;
	/** IRQ number */
	s32 irq;
	/**to set/reset or increament the count on constant proximity */
	u32 proxy_count;
	/** to store the proximity time limit to do force_calib*/
	u32 allowed_proxy_time;
	/**to store the max count value to do force calib*/
	u32 max_proxy_count;
#ifdef CONFIG_EVAL
	/** Reg buffer for read registers*/
	u16 stored_reg_data[MAX_ADUX1050_WR_LEN];
	/** Stored registers count in Reg buffer*/
	u16 stored_data_cnt;
#endif
 #ifdef CONFIG_ADUX1050_POLL
 /** Kernel Polling task delay time */
	u16 poll_delay;
#endif

	/**
	Device enable to enable the adux1050 chip
		0 - Disabled
		1 - Enabled
		2 - Suspended
	*/
	u16 dev_enable;
	/** sleep time required for the current conversion settings */
	u16 slp_time_conv_complete;
	/** Stage connected count */
	u16 conn_stg_cnt;
	/** Current power mode */
	u16 ctrl_reg;
	/** Interrupt Control */
	u16 int_ctrl;
	/** Prev current high status */
	u16 prev_high_status;
	/** Prev current low status */
	u16 prev_low_status;
	/** Current high status */
	u16 high_status;
	/** Current low status */
	u16 low_status;
	/** Conversion complete status */
	u16 conv_status;
	/** Current interrupt status */
	u16 int_status;
	/** Used to store the initial dac offset value */
	u16 cur_dac_offset[TOTAL_STG];
	/** Used to store the initial swap state */
	u16 cur_swap_state[TOTAL_STG];
	/** This hold the number of stage*/
	u16 tot_stg;
	/** Used to store the interrupt polarity*/
	u16 int_pol;
	/** Product ID of the device*/
	u16 product;
	/** Used to store version ID*/
	u16 version;
	/** Chip metal version*/
	u16 metal_id;
	/** Send event to enable/disable the input event*/
	s8 send_event;
	/** Error bits to do foce calib*/
	u8 user_intr_err;
	/**used to enable the Force_calib on proximity */
	u8 proxy_enable;
	/** Low threshold interupt control */
	u8	low_thresh_enable;
	/** High threshold interupt control */
	u8	high_thresh_enable;
		/** Number of measurement stages */
	u8	num_stages;
	/** Stage number to read raw CDC */
	u8  stg_raw_cdc;
	/** Stage number to show threshold details */
	u8  stg_threshold;
	/** Conversion enable */
	bool conv_enable;
	bool proxy_cancel;
	/** Power mode setting flag */
	bool power_mode_flag;
#ifdef CONFIG_SOMC_EXTENSION
	bool hdmi_detect;
	struct switch_dev hdmi;
	struct delayed_work hdmi_switch_work;
	u32 volume_up_stg;
	u32 volume_down_stg;
	u32 hdmi_switch_stg;
#endif
};

#endif /* __LINUX_INPUT_ADUX1050_H__*/
