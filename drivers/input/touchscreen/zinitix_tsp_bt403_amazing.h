/* 
 *
 * Zinitix touch driver
 *
 * Copyright (C) 2009 Zinitix, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef ZINITIX_REG_HEADER
#define ZINITIX_REG_HEADER

#define	BT4x2_Series		0
#define	BT4x3_Above_Series	1

#if	BT4x2_Series
#undef	BT4x3_Above_Series
#define	BT4x3_Above_Series 0
#endif

#define	TS_DRVIER_VERSION	"3.0.15"

// select touch mode    // 0 is recommended
#define	TOUCH_MODE				0

// if you want to use firmware setting, set this value. 
// interrupt mask / button num / finger num
#define	USING_CHIP_SETTING			0

// max 10
#define	MAX_SUPPORTED_FINGER_NUM			2
#define	REAL_SUPPORTED_FINGER_NUM			2

// max 8
#define	MAX_SUPPORTED_BUTTON_NUM		8
#define	SUPPORTED_BUTTON_NUM			2

// Upgrade Method
#define	TOUCH_ONESHOT_UPGRADE	1
#define	TOUCH_FORCE_UPGRADE  1
#define	TOUCH_USING_ISP_METHOD	0	// if you use isp mode, you must add i2c device : name = "zinitix_isp" , addr 0x50

// Power Control
#define	RESET_CONTROL			0

#define	USE_HW_CALIBRATION		1

// resolution offset
#define	ABS_PT_OFFSET			1

// chip information
#define	CHIP_INFO	"ZINITIX"
#define	CHIP_NAME	"BT403"

#if BT4x2_Series

#define	CHIP_POWER_OFF_DELAY	500	//ms
#define	CHIP_POWER_OFF_AF_FZ_DELAY	500	//ms
#define	CHIP_ON_DELAY			500	//ms
#define	CHIP_ON_AF_FZ_DELAY			500	//ms
#define	DELAY_FOR_SIGNAL_DELAY	30	//us
#define	DELAY_FOR_TRANSCATION	50
#define	DELAY_FOR_POST_TRANSCATION	10

#if (RESET_CONTROL==0)
#undef	TOUCH_USING_ISP_METHOD
#define	TOUCH_USING_ISP_METHOD	0
#endif

#if !USE_HW_CALIBRATION
#define	CALIBRATION_AREA		0x3E00
#endif

#endif

#if BT4x3_Above_Series

#define	CHIP_POWER_OFF_DELAY	250	//ms
#define	CHIP_POWER_OFF_AF_FZ_DELAY	250	//ms
#define	CHIP_ON_DELAY			50	//ms
#define	CHIP_ON_AF_FZ_DELAY			250	//ms
#define	DELAY_FOR_SIGNAL_DELAY	30	//us
#define	DELAY_FOR_TRANSCATION	50
#define	DELAY_FOR_POST_TRANSCATION	10

#if TOUCH_ONESHOT_UPGRADE
#undef	TOUCH_USING_ISP_METHOD
#define	TOUCH_USING_ISP_METHOD	1
#endif

#if !USE_HW_CALIBRATION
#define	CALIBRATION_AREA		0x3800
#endif

#define	FIRMWARE_VERSION_POS	0x6410
#endif

typedef enum {
	POWER_OFF,
	POWER_ON,
	RESET_LOW,
	RESET_HIGH,
} _zinitix_power_control;

// Button Enum
typedef enum {
	ICON_BUTTON_UNCHANGE,
	ICON_BUTTON_DOWN,
	ICON_BUTTON_UP,
} _zinitix_button_event;

// ESD Protection
#define	ZINITIX_ESD_TIMER_INTERVAL	0	////3 //second : if 0, no use. if you have to use, 3 is recommended
#define	ZINITIX_SCAN_RATE_HZ	60
#define	ZINITIX_CHECK_ESD_TIMER		3

//Test Mode (Monitoring Raw Data)
#define	USE_TEST_RAW_TH_DATA_MODE	1
#if USE_TEST_RAW_TH_DATA_MODE
#define	X_RAW_DATA					16
#define	Y_RAW_DATA					10
#define	MAX_TEST_RAW_DATA			(X_RAW_DATA*Y_RAW_DATA)	// 16 x 10
#define	MAX_TEST_POINT_INFO			3	// status register + x + y
#define	MAX_RAW_DATA				(MAX_TEST_RAW_DATA + MAX_TEST_POINT_INFO*MAX_SUPPORTED_FINGER_NUM + 2)
#define	ZINITIX_RAW_DATA_ESD_TIMER_INTERVAL		1	// preriod raw data interval
#define	TOUCH_TEST_RAW_MODE			51
#define	TOUCH_NORMAL_MODE			48
#define	TOUCH_ZINITIX_BASELINED_RAW_MODE	3
#define	TOUCH_ZINITIX_PROCESSED_RAW_MODE	4
#define	TOUCH_ZINITIX_CAL_N_MODE		8
#define    ZINITIX_MENU_KEY        158
#define    ZINITIX_BACK_KEY        151

#define	CAL_MIN_NUM			1400
#define	CAL_MAX_NUM			1900
#define	INT_WAIT_TIME			50

#define	FULL_X_DATA		20
#define	FULL_Y_DATA		16
#define	SCANTIME_RAWDATA		(FULL_X_DATA*FULL_Y_DATA)

typedef struct {
	int sz;
	u8 *buf;
} _raw_ioctl;

typedef struct {
	int addr;
	int *val;
} _reg_ioctl;

#endif

// Other Things
#define	ZINITIX_INIT_RETRY_CNT	10
#define	I2C_SUCCESS				0
#define	INIT_RETRY_COUNT		2

#if	BT4x2_Series

// Register Map
#define ZINITIX_SWRESET_CMD			0x00
#define ZINITIX_WAKEUP_CMD			0x01

#define ZINITIX_IDLE_CMD			0x04
#define ZINITIX_SLEEP_CMD			0x05

#define	ZINITIX_CLEAR_INT_STATUS_CMD	0x03
#define	ZINITIX_CALIBRATE_CMD		0x06
#define	ZINITIX_SAVE_STATUS_CMD		0x07
#define	ZINITIX_RECALL_FACTORY_CMD		0x0f

// 0x10~12
#define ZINITIX_TOUCH_MODE			0x10
#define ZINITIX_CHIP_REVISION		0x13
#define ZINITIX_EEPROM_INFO		0x14

// 0x20~21
#define ZINITIX_TOTAL_NUMBER_OF_X		0x20
#define ZINITIX_TOTAL_NUMBER_OF_Y		0x21
#define ZINITIX_SUPPORTED_FINGER_NUM	0x22

#define ZINITIX_AFE_FREQUENCY		0x23

#define	ZINITIX_X_RESOLUTION		0x28
#define	ZINITIX_Y_RESOLUTION		0x29

// 0x30~33
#define ZINITIX_CALIBRATION_REF 		0x30
#define ZINITIX_CALIBRATION_DEFAULT_N 	0x31
#define ZINITIX_NUMBER_OF_CALIBRATION 	0x32
#define ZINITIX_CALIBRATION_ACCURACY	0x33

#define	ZINITIX_PERIODICAL_INTERRUPT_INTERVAL	0x35

#define	ZINITIX_BUTTON_SUPPORTED_NUM	0x70

#define	ZINITIX_POINT_STATUS_REG		0x80
#define	ZINITIX_ICON_STATUS_REG		0x9a	//icon event - four icon

#define	ZINITIX_RAWDATA_REG		0x9F	//raw data 512byte
#define	ZINITIX_EXTRA_RAWDATA_REG	0x9E

#define	ZINITIX_EEPROM_INFO_REG		0xaa
#define	ZINITIX_DATA_VERSION_REG		0xab

#define ZINITIX_FIRMWARE_VERSION		0xc9

#define	ZINITIX_ERASE_FLASH		0xc9
#define	ZINITIX_WRITE_FLASH		0xc8
#define	ZINITIX_READ_FLASH			0xca

//0xF0
#define	ZINITIX_INT_ENABLE_FLAG		0xf0
#endif
//---------------------------------------------------------------------
#if	BT4x3_Above_Series

// Register Map
#define ZINITIX_SWRESET_CMD			0x0000
#define ZINITIX_WAKEUP_CMD			0x0001

#define ZINITIX_IDLE_CMD			0x0004
#define ZINITIX_SLEEP_CMD			0x0005

#define	ZINITIX_CLEAR_INT_STATUS_CMD	0x0003
#define	ZINITIX_CALIBRATE_CMD		0x0006
#define	ZINITIX_SAVE_STATUS_CMD		0x0007
#define	ZINITIX_SAVE_CALIBRATION_CMD		0x08
#define	ZINITIX_RECALL_FACTORY_CMD		0x000f

#define ZINITIX_TOUCH_MODE			0x0010
#define ZINITIX_CHIP_REVISION		0x0011
#define ZINITIX_FIRMWARE_VERSION		0x0012
#define	ZINITIX_DATA_VERSION_REG		0x0013
#define ZINITIX_TSP_TYPE			0x0014
#define ZINITIX_SUPPORTED_FINGER_NUM	0x0015
#define	ZINITIX_MAX_Y_NUM		0x0016
#define ZINITIX_EEPROM_INFO		0x0018
#define ZINITIX_CAL_N_TOTAL_NUM		0x001B

#define ZINITIX_TOTAL_NUMBER_OF_X		0x0060
#define ZINITIX_TOTAL_NUMBER_OF_Y		0x0061

#define	ZINITIX_BUTTON_SUPPORTED_NUM	0xB0

#define	ZINITIX_X_RESOLUTION		0x00C0
#define	ZINITIX_Y_RESOLUTION		0x00C1

#define	ZINITIX_POINT_STATUS_REG		0x0080
#define	ZINITIX_ICON_STATUS_REG		0x00A0	//icon event - four icon

#define	ZINITIX_RAWDATA_REG		0x0200

#define	ZINITIX_EEPROM_INFO_REG		0x0018

//0xF0
#define	ZINITIX_INT_ENABLE_FLAG		0x00f0
#define	ZINITIX_PERIODICAL_INTERRUPT_INTERVAL	0x00f1

#endif
// Interrupt & status register flag bit
//-------------------------------------------------
#define	BIT_PT_CNT_CHANGE			0
#define	BIT_DOWN				1
#define	BIT_MOVE				2
#define	BIT_UP					3
#define	BIT_HOLD				4
#define	BIT_LONG_HOLD				5
#define	RESERVED_0				6
#define	RESERVED_1				7
#define	BIT_WEIGHT_CHANGE			8
#define	BIT_PT_NO_CHANGE			9
#define	BIT_REJECT				10
#define	BIT_PT_EXIST				11	// status register only
//-------------------------------------------------
#define	RESERVED_2				12
#define	RESERVED_3				13
#define	RESERVED_4				14
#define	BIT_ICON_EVENT				15

// 4 icon
#define	BIT_O_ICON0_DOWN			0
#define	BIT_O_ICON1_DOWN			1
#define	BIT_O_ICON2_DOWN			2
#define	BIT_O_ICON3_DOWN			3
#define	BIT_O_ICON4_DOWN			4
#define	BIT_O_ICON5_DOWN			5
#define	BIT_O_ICON6_DOWN			6
#define	BIT_O_ICON7_DOWN			7

#define	BIT_O_ICON0_UP				8
#define	BIT_O_ICON1_UP				9
#define	BIT_O_ICON2_UP				10
#define	BIT_O_ICON3_UP				11
#define	BIT_O_ICON4_UP				12
#define	BIT_O_ICON5_UP				13
#define	BIT_O_ICON6_UP				14
#define	BIT_O_ICON7_UP				15

#define	SUB_BIT_EXIST			0	// status register only
#define	SUB_BIT_DOWN			1
#define	SUB_BIT_MOVE			2
#define	SUB_BIT_UP				3
#define	SUB_BIT_UPDATE			4
#define	SUB_BIT_WAIT			5

#define	zinitix_bit_set(val,n)		((val) &=~(1<<(n)), (val) |=(1<<(n)))
#define	zinitix_bit_clr(val,n)		((val) &=~(1<<(n)))
#define	zinitix_bit_test(val,n)		((val) & (1<<(n)))
#define zinitix_swap_v(a, b, t)	((t) = (a), (a) = (b), (b) = (t))
#define zinitix_swap_16(s) (((((s) & 0xff) << 8) | (((s) >> 8) & 0xff)))

#endif //ZINITIX_REG_HEADER
