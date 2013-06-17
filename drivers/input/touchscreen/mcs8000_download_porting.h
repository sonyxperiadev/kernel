
//--------------------------------------------------------
//
//
//      Melfas MCS8000 Series Download base v1.0 2010.04.05
//
//
//--------------------------------------------------------

#ifndef __MELFAS_DOWNLOAD_PORTING_H_INCLUDED__
#define __MELFAS_DOWNLOAD_PORTING_H_INCLUDED__

//============================================================
//
//      Porting order
//
//============================================================
/*

1. melfas_download_porting.h
   - Check typedef	[melfas_download_porting.h]

   - Check download options	[melfas_download_porting.h]

   - Add Port control code  ( CE, RESETB, I2C,... )	[melfas_download_porting.h]

   - Apply your delay function ( inside mcsdl_delaly() )	[melfas_download.c]
      Modify delay parameter constant ( ex. MCSDL_DELAY_5MS ) to make it fit to your delay function.

   - Rename 'uart_printf()' to your console print function for debugging. [melfas_download_porting.h]
   	  or, define uart_printf() as different function properly.

   - Check Watchdog timer, Interrupt factor

   - Including Melfas binary .c file

   - Basenad dealy function
      fill up mcsdl_delay()

   - Implement processing external Melfas binary .bin file.

*/

//============================================================
//
//      Type define
//
//============================================================

typedef char INT8;
typedef unsigned char UINT8;
//typedef unsigned char         uint8_t;
typedef short INT16;
typedef unsigned short UINT16;
//typedef unsigned short        uint16_t;
typedef int INT32;
typedef unsigned int UINT32;
typedef unsigned char BOOLEAN;

#ifndef TRUE
#define TRUE 				(1==1)
#endif

#ifndef FALSE
#define FALSE 				(1==0)
#endif

#ifndef NULL
#define NULL 				0
#endif

#define PRESS_KEY           1
#define RELEASE_KEY         0

//============================================================
//
//      Porting Download Options
//
//============================================================

// For printing debug information. ( Please check 'printing function' )
#define MELFAS_ENABLE_DBG_PRINT											1
#define MELFAS_ENABLE_DBG_PROGRESS_PRINT								1

extern void touch_ctrl_regulator_mms128(int on_off);
#define GPIO_TSP_LDO		//GPIO NUM
#define GPIO_TOUCH_INT 121	//GPIO NUM
#define GPIO_TSP_SCL   87	//GPIO NUM
#define GPIO_TSP_SDA   86	//GPIO NUM
#define TOUCH_EN 22

//----------------
// VDD
//----------------
#define mcsdl_vdd_on()                  touch_ctrl_regulator_mms128(1)
#define mcsdl_vdd_off()                 touch_ctrl_regulator_mms128(0)

#define MCSDL_VDD_SET_HIGH()            mcsdl_vdd_on()
#define MCSDL_VDD_SET_LOW()             mcsdl_vdd_off()

//----------------
// RESETB ==> Interrupt Signal
//----------------
#define MCSDL_RESETB_SET_HIGH()             		gpio_direction_output(GPIO_TOUCH_INT, 1)
#define MCSDL_RESETB_SET_LOW()              		gpio_direction_output(GPIO_TOUCH_INT, 0)
#define MCSDL_RESETB_SET_OUTPUT(n)           		gpio_direction_output(GPIO_TOUCH_INT, n)
#define MCSDL_RESETB_SET_INPUT()            		gpio_direction_input(GPIO_TOUCH_INT)
#define MCSDL_RESETB_SET_ALT()	// Nothing

#define MCSDL_GPIO_RESETB_IS_HIGH()		((gpio_get_value(GPIO_TOUCH_INT) > 0) ? 1 : 0)

#define IRQ_TOUCH_INT		// Nothing  (IRQ_EINT_GROUP3_BASE+1)

//------------------
// I2C SCL & SDA
//------------------

#define MCSDL_GPIO_SCL_SET_HIGH()					gpio_direction_output(GPIO_TSP_SCL, 1)
#define MCSDL_GPIO_SCL_SET_LOW()					gpio_direction_output(GPIO_TSP_SCL, 0)

#define MCSDL_GPIO_SDA_SET_HIGH()					gpio_direction_output(GPIO_TSP_SDA, 1)
#define MCSDL_GPIO_SDA_SET_LOW()					gpio_direction_output(GPIO_TSP_SDA, 0)

#define MCSDL_GPIO_SCL_SET_OUTPUT(n)				gpio_direction_output(GPIO_TSP_SCL, n)
#define MCSDL_GPIO_SCL_SET_INPUT()					gpio_direction_input(GPIO_TSP_SCL)
#define MCSDL_GPIO_SCL_SET_ALT()	// Nothing

#define MCSDL_GPIO_SDA_SET_OUTPUT(n)				gpio_direction_output(GPIO_TSP_SDA, n)
#define MCSDL_GPIO_SDA_SET_INPUT()					gpio_direction_input(GPIO_TSP_SDA)
#define MCSDL_GPIO_SDA_SET_ALT()	// Nothing

#define MCSDL_GPIO_SDA_IS_HIGH()					((gpio_get_value(GPIO_TSP_SDA) > 0) ? 1 : 0)

//============================================================
//
//      Defence External Effect
//
//============================================================
#define MELFAS_DISABLE_BASEBAND_ISR()	// Nothing
#define MELFAS_DISABLE_WATCHDOG_TIMER_RESET()	// Nothing

#define MELFAS_ROLLBACK_BASEBAND_ISR()	// Nothing
#define MELFAS_ROLLBACK_WATCHDOG_TIMER_RESET()	// Nothing

//============================================================
//
//      TSP Information
//
//============================================================
#define TS_MELFAS_VENDOR_NAME      "MELFAS"
#define TS_MELFAS_VENDOR_CHIP_NAME "MMS100"
#define TS_MELFAS_VENDOR_ID        0x00
#define TS_MELFAS_MODULE_REV       0x00
#define TS_MELFAS_FIRMWARE_VER     0x00

#define TS_MAX_X_COORD  320	//240
#define TS_MAX_Y_COORD  480	//320
#define TS_MAX_Z_TOUCH  255
#define TS_MAX_W_TOUCH  30
#define TS_MAX_TOUCH    2	//5

#define TS_MELFAS_SENSING_CHANNEL_NUM  10
#define TS_MELFAS_EXCITING_CHANNEL_NUM 15	//14

//============================================================
//
//      Delay parameter setting
//
//      These are used on 'mcsdl_delay()'
//
//============================================================

#define MCSDL_DELAY_1US			1
#define MCSDL_DELAY_2US			2
#define MCSDL_DELAY_3US			3
#define MCSDL_DELAY_5US			5
#define MCSDL_DELAY_7US			7
#define MCSDL_DELAY_10US		10
#define MCSDL_DELAY_15US		15
#define MCSDL_DELAY_20US		20
#define MCSDL_DELAY_40US		40
#define MCSDL_DELAY_70US		70

#define MCSDL_DELAY_100US		100
#define MCSDL_DELAY_150US		150
#define MCSDL_DELAY_300US		300
#define MCSDL_DELAY_500US		500
#define MCSDL_DELAY_800US		800

#define MCSDL_DELAY_1MS		1000
#define MCSDL_DELAY_5MS		5000
#define MCSDL_DELAY_10MS		10000
#define MCSDL_DELAY_25MS		25000
#define MCSDL_DELAY_30MS		30000
#define MCSDL_DELAY_40MS		40000
#define MCSDL_DELAY_45MS		45000
#define MCSDL_DELAY_60MS		60000
#define MCSDL_DELAY_80MS		80000
#define MCSDL_DELAY_100MS		100000

/* Touch Screen Interface Specification Multi Touch (V0.5) */
/* REGISTERS */
#define MCSTS_STATUS_REG        0x00	//Status
#define MCSTS_MODE_CONTROL_REG  0x01	//Mode Control
#define MCSTS_RESOL_HIGH_REG    0x02	//Resolution(High Byte)
#define MCSTS_RESOL_X_LOW_REG   0x08	//Resolution(X Low Byte)
#define MCSTS_RESOL_Y_LOW_REG	 0x0A	//Resolution(Y Low Byte)
#define MCSTS_INPUT_INFO_REG    0x10	//Input Information
#define MCSTS_POINT_HIGH_REG    0x11	//Point(High Byte)
#define MCSTS_POINT_X_LOW_REG   0x12	//Point(X Low Byte)
#define MCSTS_POINT_Y_LOW_REG   0x13	//Point(Y Low Byte)
#define MCSTS_WIDTH_REG      0x14	//Width
#define MCSTS_STRENGTH_REG      0x15	//Strength
#define MCSTS_VENDOR_ID_REG    0x1B	//Vendor ID
#define MCSTS_MODULE_VER_REG    0x1C	//H/W Module Revision
#define MCSTS_FIRMWARE_VER_REG  0x1D	//F/W Version

#endif
