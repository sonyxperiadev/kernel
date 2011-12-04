//--------------------------------------------------------
//
//
//	Melfas MCS8000 Series Download base v1.0 2010.04.05
//
//
//--------------------------------------------------------


#ifndef __MELFAS_FIRMWARE_DOWNLOAD_H__
#define __MELFAS_FIRMWARE_DOWNLOAD_H__



//=====================================================================
//
//   MELFAS Firmware download pharameters
//
//=====================================================================

#define MELFAS_TRANSFER_LENGTH					(32/8)		// Fixed value
#define MELFAS_FIRMWARE_MAX_SIZE				(32*1024)
#define MELFAS_2CHIP_DOWNLOAD_ENABLE            0       // 0 : 1Chip Download, 1: 2Chip Download
// For delay function test. ( Disable after Porting is finished )
#define MELFAS_ENABLE_DELAY_TEST										0
// ISC download mode
#define MELFAS_CORE_FIRWMARE_UPDATE_ENABLE			1	// 0 : disable, 1: enable
#define MELFAS_PRIVATE_CONFIGURATION_UPDATE_ENABLE	1	// 0 : disable, 1: enable
#define MELFAS_PUBLIC_CONFIGURATION_UPDATE_ENABLE	1	// 0 : disable, 1: enable

//----------------------------------------------------
//   ISC Mode
//----------------------------------------------------
#define MELFAS_CRC_CHECK_ENABLE				1

#define ISC_MODE_SLAVE_ADDRESS					0x48

#define ISC_READ_DOWNLOAD_POSITION			0			//0 : USE ISC_PRIVATE_CONFIG_FLASH_START 1: READ FROM RMI MAP(0x61,0x62)
#define ISC_PRIVATE_CONFIG_FLASH_START			25
#define ISC_PUBLIC_CONFIG_FLASH_START			28

//address for ISC MODE
#define ISC_DOWNLOAD_MODE_ENTER				0x5F
#define ISC_DOWNLOAD_MODE						0x60
#define ISC_PRIVATE_CONFIGURATION_START_ADDR	0x61
#define ISC_PUBLIC_CONFIGURATION_START_ADDR	0x62

#define ISC_READ_SLAVE_CRC_OK					0x63		// return value from slave
#define ISC_CORE_FIRMWARE_VERSION_ADDR		0x64

//mode
#define ISC_CORE_FIRMWARE_DL_MODE				0x01
#define ISC_PRIVATE_CONFIGURATION_DL_MODE		0x02
#define ISC_PUBLIC_CONFIGURATION_DL_MODE		0x03
#define ISC_SLAVE_DOWNLOAD_START				0x04
//----------------------------------------------------
//   ISP Mode
//----------------------------------------------------
#define ISP_MODE_ERASE_FLASH					0x01
#define ISP_MODE_SERIAL_WRITE					0x02
#define ISP_MODE_SERIAL_READ					0x03
#define ISP_MODE_NEXT_CHIP_BYPASS				0x04


//----------------------------------------------------
//   Return values of download function
//----------------------------------------------------
#define MCSDL_RET_SUCCESS						0x00
#define MCSDL_RET_ERASE_FLASH_VERIFY_FAILED		0x01
#define MCSDL_RET_PROGRAM_VERIFY_FAILED			0x02
#define MCSDL_FIRMWARE_UPDATE_MODE_ENTER_FAILED	0x03
#define MCSDL_FIRMWARE_UPDATE_FAILED				0x04
#define MCSDL_LEAVE_FIRMWARE_UPDATE_MODE_FAILED	0x05
#define MCSTS_FIRMWARE_VER_REG_MASTER				0x31	//F/W Version MASTER
#define MCSTS_FIRMWARE_VER_REG_SLAVE				0x32	//F/W Version SLAVE

#define MCSDL_RET_PROGRAM_SIZE_IS_WRONG			0x10
#define MCSDL_RET_VERIFY_SIZE_IS_WRONG			0x11
#define MCSDL_RET_WRONG_BINARY					0x12

#define MCSDL_RET_READING_HEXFILE_FAILED		0x21
#define MCSDL_RET_FILE_ACCESS_FAILED			0x22
#define MCSDL_RET_MELLOC_FAILED					0x23

#define MCSDL_RET_WRONG_MODULE_REVISION			0x30


//----------------------------------------------------
//	When you can't control VDD nor CE.
//	Set this value 1
//	Then Melfas Chip can prepare chip reset.
//----------------------------------------------------
#define MELFAS_USE_PROTOCOL_COMMAND_FOR_DOWNLOAD 	0		// If 'enable download command' is needed ( Pinmap dependent option ).


//============================================================
//
//	Delay parameter setting
//
//	These are used on 'mcsdl_delay()'
//
//============================================================
#if 1 //0905 SEC
void mcsdl_vdd_on(void);
void mcsdl_vdd_off(void);
#endif

#define MCSDL_DELAY_1US								    1
#define MCSDL_DELAY_2US								    2
#define MCSDL_DELAY_3US								    3
#define MCSDL_DELAY_5US								    5
#define MCSDL_DELAY_7US 								7
#define MCSDL_DELAY_10US 							   10
#define MCSDL_DELAY_15US							   15
#define MCSDL_DELAY_20US							   20
#define MCSDL_DELAY_40US                               40

#define MCSDL_DELAY_100US							  100
#define MCSDL_DELAY_150US							  150
#define MCSDL_DELAY_300US                             300
#define MCSDL_DELAY_500US             				  500
#define MCSDL_DELAY_800US							  800


#define MCSDL_DELAY_1MS								 1000
#define MCSDL_DELAY_5MS								 5000
#define MCSDL_DELAY_10MS							10000
#define MCSDL_DELAY_25MS							25000
#define MCSDL_DELAY_30MS							30000
#define MCSDL_DELAY_40MS							40000
#define MCSDL_DELAY_45MS							45000
#define MCSDL_DELAY_60MS                            60000


//============================================================
//
//	Porting factors for Baseband
//
//============================================================
#include "mcs8000_download_porting.h"


//----------------------------------------------------
//	Functions
//----------------------------------------------------
int mcsdl_download_binary_data(INT32 hw_ver); // with binary type .c   file.
int mcsdl_download_binary_file(void); // with binary type .bin file.
#if MELFAS_ENABLE_DELAY_TEST
void mcsdl_delay_test(INT32 nCount);
#endif
int mms100_ISC_download_binary_data(int hw_ver);


//---------------------------------
//	Delay functions
//---------------------------------
void mcsdl_delay(UINT32 nCount);


#endif		//#ifndef __MELFAS_FIRMWARE_DOWNLOAD_H__

