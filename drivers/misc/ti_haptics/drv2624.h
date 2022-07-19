#ifndef __DRV2624_H__
#define __DRV2624_H__
/*
** =============================================================================
** Copyright (c)2021  Texas Instruments Inc.
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
** File:
**     drv2624.h
**
** Description:
**     Header file for drv2624-ram.c
**
** =============================================================================
*/
#define RAM_BIN_FILE		 						("drv2624.ram")

#define DRV26XX_I2C_RETRY_COUNT						(3)
#define	DRV26XX_GO_BIT_MAX_RETRY_CNT				(5)
#define	DRV26XX_GO_BIT_CHECK_INTERVAL           	(10)	/* 10 ms */
#define	DRV2624_SEQUENCER_SIZE						(8)

#define	DRV2624_R0X00								(0x00)
#define DRV2624_R0X00_CHIP_ID_REV					(0x03)

#define	DRV2624_R0X01_STATUS						(0x01)
#define	DRV2624_R0X01_STATUS_DIAG_RESULT_MSK		(0x80)
#define	DRV2624_R0X01_STATUS_DIAG_RESULT_OK			(0x00)
#define	DRV2624_R0X01_STATUS_DIAG_RESULT_NOK		(0x80)
#define	DRV2624_R0X01_STATUS_PRG_ERROR_MSK			(0x10)
#define	DRV2624_R0X01_STATUS_PROCESS_DONE_MSK		(0x08)
#define	DRV2624_R0X01_STATUS_UVLO_MSK				(0x04)
#define	DRV2624_R0X01_STATUS_OVER_TEMP_MSK			(0x02)
#define	DRV2624_R0X01_STATUS_OC_DETECT_MSK			(0x01)

#define	DRV2624_R0X02_INTZ							(0x02)
#define	DRV2624_R0X02_INTZ_MSK						(0x1F)
#define	DRV2624_R0X02_INTZ_DISABLE					(0x1F)
#define	DRV2624_R0X02_INTZ_ENABLE					(0x00)
#define	DRV2624_R0X02_INTZ_PROCESS_DONE_DISABLE		(0x08)

#define	DRV2624_RX03_DIAG_Z_RESULT					(0x03)

#define	DRV2624_R0X05_LRA_PERIOD_H					(0x05)
#define	DRV2624_R0X05_LRA_PERIOD_H_MSK				(0x03)
#define	DRV2624_R0X06_LRA_PERIOD_L					(0x06)

#define	DRV2624_R0X07								(0x07)
#define	DRV2624_R0X07_TRIG_PIN_FUNC_SFT				(2)
#define	DRV2624_R0X07_TRIG_PIN_FUNC_MSK				\
	(0x02 << DRV2624_R0X07_TRIG_PIN_FUNC_SFT)
#define	DRV2624_R0X07_TRIG_PIN_FUNC_INT				\
	(0x02 << DRV2624_R0X07_TRIG_PIN_FUNC_SFT)
#define DRV2624_R0X07_MODE_MSK						(0x3)
#define DRV2624_R0X07_MODE_RTP_MODE					(0x0)
#define DRV2624_R0X07_MODE_WVFRM_SEQ_MODE			(0x1)
#define DRV2624_R0X07_MODE_DIAG_RTN					(0x2)
#define DRV2624_R0X07_MODE_AUTO_LVL_CALIB_RTN		(0x3)

#define	DRV2624_R0X08								(0x08)
#define	DRV2624_R0X08_LRA_ERM_MSK					(0x80)
#define	DRV2624_R0X08_LRA_ERM_SFT					(7)
#define	DRV2624_R0X08_CTL_LOOP_MSK					(0x40)
#define	DRV2624_R0X08_CTL_LOOP_SFT					(6)
#define	DRV2624_R0X08_CTL_LOOP_CLOSED_LOOP			(0)
#define	DRV2624_R0X08_CTL_LOOP_OPEN_LOOP			(1<<DRV2624_R0X08_CTL_LOOP_SFT)
#define	DRV2624_R0X08_HYBRID_LOOP_MSK				(0x20)
#define	DRV2624_R0X08_HYBRID_LOOP_SFT				(5)
#define	DRV2624_R0X08_AUTO_BRK_OL_MSK				(0x10)
#define	DRV2624_R0X08_AUTO_BRK_OL_SFT				(4)
#define	DRV2624_R0X08_AUTO_BRK_OL_EN				(0x10)
#define	DRV2624_R0X08_AUTO_BRK_INTO_STBY_MSK		(0x08)
#define	DRV2624_R0X08_WITHOUT_AUTO_BRK_INTO_STBY	(0x00)
#define	DRV2624_R0X08_AUTO_BRK_INTO_STBY			(0x04)
#define	DRV2624_R0X08_AUTO_BRK_INTO_STBY_SFT		(3)

#define	DRV2624_R0X09								(0x09)
#define	DRV2624_R0X09_UVLO_THRES_MSK				(0x07)
#define	DRV2624_R0X09_UVLO_THRES_3_2V				(0x07)

#define	DRV2624_R0X0C_GO							(0x0C)
#define	DRV2624_R0X0C_GO_MSK						(0x01)
#define	DRV2624_R0X0C_GO_BIT						(0x01)
#define	DRV2624_R0X0C_NGO_BIT						(0x00)

#define	DRV2624_R0X0D								(0x0D)
#define	DRV2624_R0X0D_PLAYBACK_INTERVAL_MSK			(0x20)
#define	DRV2624_R0X0D_PLAYBACK_INTERVAL_SFT			(5)
#define	DRV2624_R0X0D_PLAYBACK_INTERVAL_1MS			\
	(0x01 << DRV2624_R0X0D_PLAYBACK_INTERVAL_SFT)
#define	DRV2624_R0X0D_PLAYBACK_INTERVAL_5MS			\
		(0x00 << DRV2624_R0X0D_PLAYBACK_INTERVAL_SFT)
#define DRV2624_R0X0D_DIG_MEM_GAIN_MASK 			(0x03)
#define  DRV2624_R0X0D_DIG_MEM_GAIN_STRENGTH_100 (0x0)
#define  DRV2624_R0X0D_DIG_MEM_GAIN_STRENGTH_75	(0x1)
#define  DRV2624_R0X0D_DIG_MEM_GAIN_STRENGTH_50	(0x2)
#define  DRV2624_R0X0D_DIG_MEM_GAIN_STRENGTH_25	(0x3)

#define	DRV2624_R0X0E_RTP_INPUT						(0x0e)
#define	DRV2624_R0X0F_SEQ1							(0x0f)
#define	DRV2624_R0X10_SEQ2							(0x10)
#define	DRV2624_R0X17_WAV1_4_SEQ_LOOP				(0x17)

#define	DRV2624_R0X19_WAV_SEQ_MAIN_LOOP				(0x19)
#define	DRV2624_R0X19_WAV_SEQ_MAIN_LOOP_MSK			(0x07)
#define	DRV2624_R0X19_WAV_SEQ_MAIN_LOOP_ONCE		(0x00)
#define	DRV2624_R0X19_WAV_SEQ_MAIN_LOOP_INFINITE	(0x07)

#define	DRV2624_R0X1F_RATED_VOLTAGE					(0x1f)
#define	DRV2624_R0X20_OD_CLAMP						(0x20)

#define	DRV2624_R0X21_CAL_COMP						(0x21)
#define DRV2624_R0X22_CAL_BEMF						(0x22)
#define	DRV2624_R0X23								(0x23)
#define	DRV2624_R0X23_BEMF_GAIN_MSK					(0x03)
#define	DRV2624_R0X23_BEMF_GAIN_30X_LRA				(0x03)
#define	DRV2624_R0X23_FB_BRAKE_FACTOR_MSK			(0x70)
#define	DRV2624_R0X23_FB_BRAKE_FACTOR_SFT			(4)

#define	DRV2624_R0X27								(0x27)
#define	DRV2624_R0X27_DRIVE_TIME_MSK				(0x1f)
#define	DRV2624_R0X27_LRA_MIN_FREQ_SEL_SFT			(0x07)
#define	DRV2624_R0X27_LRA_MIN_FREQ_SEL_45HZ			\
	(0x01 << DRV2624_R0X27_LRA_MIN_FREQ_SEL_SFT)
#define	DRV2624_R0X27_LRA_MIN_FREQ_SEL_MSK			(0x80)

#define	DRV2624_R0X29								(0x29)
#define	DRV2624_R0X29_SAMPLE_TIME_MSK				(0x0C)
#define	DRV2624_R0X29_SAMPLE_TIME_250us				(0x08)

#define	DRV2624_R0X2A								(0x2A)
#define	DRV2624_R0X2A_AUTO_CAL_TIME_MSK				(0x03)
#define	DRV2624_R0X2A_AUTO_CAL_TIME_TRIGGER_CRTLD	(0x03)

#define	DRV2624_R0X2C								(0x2C)
#define	DRV2624_R0X2C_LRA_WAVE_SHAPE_MSK			(0x01)


#define	DRV2624_R0X2E_OL_LRA_PERIOD_H				(0x2e)
#define	DRV2624_R0X2E_OL_LRA_PERIOD_H_MSK			(0x03)
#define	DRV2624_R0X2F_OL_LRA_PERIOD_L				(0x2f)
#define	DRV2624_R0X2F_OL_LRA_PERIOD_L_MSK			(0xff)

#define	DRV2624_R0X30_CURRENT_K						(0x30)

#define	DRV2624_R0XFD_RAM_ADDR_UPPER				(0xfd)
#define	DRV2624_R0XFE_RAM_ADDR_LOWER				(0xfe)
#define	DRV2624_R0XFF_RAM_DATA						(0xff)

#define DRV26XX_RAM_SIZE        					(1024)

#define SMS_HTONS(a,b) (	(((a)&0x00FF)<<8) | \
						((b)&0x00FF) )

typedef enum tiDrv26xxRAM {
    DRV26XX_BF_FB_BRAKE_FACTOR						= 0x02,
    DRV26XX_BF_AUTO_BRAKE_STANDBY					= 0x30,
    DRV26XX_BF_AUTO_BRAKE							= 0x40,
    DRV26XX_BF_HYBRID_LOOP							= 0x50,
    DRV26XX_BF_WVFM_BRAKE							= 0x00,
    DRV26XX_BF_WVFM_LOOP_MOD						= 0x10,
    DRV26XX_BF_WVFM_WV_SHAPE						= 0x20
} tiDrv26xxBfEnumList_t;

enum actuator_type {
	ERM = 0,
	LRA
};

struct drv2624_waveform {
	unsigned char mnEffect;
	unsigned char mnLoop;
};

struct drv2624_waveform_sequencer {
	struct drv2624_waveform msWaveform[DRV2624_SEQUENCER_SIZE];
};

struct drv2624_waveform_sequencer msWaveformSequencer;

struct actuator_data {
	unsigned char mnActuatorType;
	unsigned char mnRatedVoltage;
	unsigned char mnOverDriveClampVoltage;
	unsigned char mnSampleTimeReg;
	unsigned int  mnSampleTime;
	unsigned int mnLRAFreq;
	int 		mnLRAPeriod;
	unsigned short openLoopPeriod;
	unsigned char mnCalRatedVoltage;
	unsigned char mnCalOverDriveClampVoltage;
};

struct drv2624_platform_data {
	int mnGpioNRST;
	int mnGpioINT;
	unsigned char mnLoop;
	struct actuator_data msActuator;
};

struct drv2624_lra_period_data {
	unsigned char msb;
	unsigned char lsb;
};

struct drv2624_autocal_data {
	unsigned char mnDoneFlag;
	unsigned char mnCalComp;
	unsigned char mnCalBemf;
	unsigned char mnCalGain;
	struct drv2624_lra_period_data mnLraPeriod;
	struct drv2624_lra_period_data mnOL_LraPeriod;
	unsigned char mnCnt;
};

struct drv26xx_RAMwaveform_info {
    unsigned short duration;
    unsigned char brake;
    unsigned char loop_mod;
    unsigned char wv_shape;
    unsigned int mnEffectTimems;
};

struct drv26xx_RAMwaveforms {
    unsigned char  fb_brake_factor;
    unsigned char  auto_brake_standby;
    unsigned char  auto_brake;
    unsigned char  hybrid_loop;
    unsigned int  rated_Voltage;
    unsigned int  overDrive_Voltage;
    unsigned char  lra_f0;
    unsigned char  nWaveforms;
	unsigned char  *rambin_name;
    struct drv26xx_RAMwaveform_info** ram_wvfm_info;
};

struct drv2624_diag_result {
	unsigned char mnResult;
	unsigned char mnDiagZ;
	unsigned char mnCurrentK;
	unsigned char mnCnt;
	unsigned int  mnRemohm;
};

struct drv2624_data
{
	struct i2c_client *client;
	struct regmap *mpRegmap;
	struct device *dev;
	struct led_classdev led_dev;
	struct drv2624_platform_data msPlatData;
	struct mutex reg_lock;
	struct mutex haptic_lock;
	struct drv2624_autocal_data mAutoCalData;
	struct drv2624_diag_result mDiagResult;
	struct drv2624_waveform_sequencer msWaveformSequencer;
	struct drv26xx_RAMwaveforms ramwvfm;
	struct work_struct vibrator_irq_work;
	struct work_struct vibrator_work;
	struct hrtimer haptics_timer;
	unsigned char mnDeviceID;
	unsigned int mnIRQ;
	int state;
	int duration;
	unsigned char waveform_id;
	unsigned int gain;
	bool mbIRQEnabled;
	bool mbWork;
	bool bRTPmode;
	bool bInternval_1ms;
	ktime_t current_ktime;
	ktime_t pre_enter_ktime;
	unsigned int interval_us;
};
#endif
