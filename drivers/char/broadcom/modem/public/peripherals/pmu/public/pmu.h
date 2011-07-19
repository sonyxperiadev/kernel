/****************************************************************************
*
*     Copyright (c) 2007 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           5300 California Avenue
*           Irvine, California 92617
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
/**
*
*   @file   pmu.h
*
*   @brief  PMU header file, which includes common API defs for all PMU devices
*
****************************************************************************/
/**
*   @defgroup   PMU_DRV   PMU Driver
*   @brief      Common API definitions for all PMU devices
*   @ingroup    HAL_EM_PMU
*	@ingroup	DeviceDriverGroup
*
****************************************************************************/
/**
 * @addtogroup PMU_DRV
 * @{
 */
// $Log:  $
// 02/14/06	
//			- Initial version for platform software
//			- Modularized codes:
//				Codes for specific PMU devices are moved to their own modules
//				Philips PCF50603 is in pmu_pcf50603_1b.h
//				Philips PCF50611 is in pmu_pcf50611.h
//				Philips PCF50612 is in pmu_pcf50612.h
//			- There is a global switch PMU_MODULE that selects PMU device
// 02/13/07 
//			- Modified for HAL architecture
// 11/18/10   - Moved PMU_Power_Supply_t from pmu_max8966.h
//			- Added API's PMU_DRV_GetAPITable & PMU_DRV_GetPrivateAPITable
//			- Some cleanup of include files; now covers only 55,38, maxim for win32 and non-win32
//******************************************************************************
/* Requires the following header files before its inclusion in a c file
#include "mobcom_types.h"
#include "i2c_drv.h"
*/
#ifndef _PMU_MEMMAP_INC_
#define _PMU_MEMMAP_INC_

//******************************************************************************
//                          Definition block
//******************************************************************************

typedef enum 
{
    SIMLDO1,
    SIMLDO2
} PMU_SIMLDO_t;

typedef struct
{
	UInt8 power_rail;
	UInt8 power_vol;
}PMU_Power_Supply_t;

#if defined( PMU_PCF50603 ) || defined( PMU_PCF50611  ) || defined( PMU_PCF50612 )
	#define I2C_PMU            0x12     //!< Power Management Unit (write address)
#elif defined( PMU_BCM59001 ) || defined( PMU_BCM59035 ) || defined(PMU_BCM59036) || defined(PMU_BCM59038) || defined(PMU_BCM59055)
	#define I2C_PMU            0x10     //!< Power Management Unit (write address)
#else
	#define I2C_PMU			   0x10		//!< Dummy address for no_pmu build. This build will not access i2c (safe)
#endif

#if defined(PMU_BCM59055)
    #define I2C_PMU2           0x18
#endif

#if defined ( WIN32)         
	#if defined(PMU_BCM59038)
		#include "pmu_bcm59038.h"
	#elif defined(PMU_MAX8986)
		#include "pmu_max8986.h"
    #elif defined(PMU_BCM59055)
		#include "pmu_bcm59055.h"
    #else
         #include "pmu_pcf50612.h"
    #endif
#elif defined( PMU_PCF50612 )
     #include "pmu_pcf50612.h"
#elif defined(PMU_BCM59038)
	#include "pmu_bcm59038.h"
#elif defined(PMU_MAX8986)
	#include "pmu_max8986.h"
#elif defined(PMU_BCM59055)
	#include "pmu_bcm59055.h"
#elif defined UHT_HOST_BUILD
	#include "pmu_bcm59001.h"
#elif defined STACK_WIN32_BUILD
	#include "hw_sdtenv.h"
#else
    #include "pmu_stubs.h"
#endif
#include "i2c_drv.h"

// I2C addresses are the same for 50603, 50611 and 50612
#if !defined( PMU_BCM59001 ) && !defined ( PMU_BCM59035 ) && !defined( PMU_BCM59036 ) && !defined( PMU_BCM59038 ) && !defined( PMU_BCM59055 )
#define PMU_BASE_ADDR   0x12    // write address 00010010
#define PMU_BASE_W      0x12    // write address 00010010
#define PMU_BASE_R      0x13    // read  address 00010011
#else
#define PMU_BASE_ADDR   0x10    // write address 00010000
#define PMU_BASE_W      0x10    // write address 00010000
#define PMU_BASE_R      0x11    // read  address 00010001
#endif

// This is used in simio.c and 603. Maintain legacy codes.
#ifndef NO_PMU
#define SIMVCC_VIA_PMU          // PMU SIMVCC is used, instead of baseband
#endif

//******************************************************************************
// Typedefs
//******************************************************************************
#if defined(PMU_BCM59055)
typedef void (*pmuisr_t)(UInt8 irqId);
#else
typedef void (*isr_pmu_t)(void);
#endif


#if defined(PMU_MAX8986)
typedef enum {
      PMU_SIM3P0Volt = 0,
      PMU_SIM2P5Volt,
      PMU_SIM3P1Volt,
      PMU_SIM1P8Volt,
      PMU_SIM0P0Volt
} PMU_SIMVolt_t;

#elif (defined(PMU_BCM59035) && defined(BCM59035_REV_B0)) || defined(PMU_BCM59036)

typedef enum {
      PMU_SIM3P0Volt = 0,
      PMU_SIM2P5Volt,
      PMU_SIM0P0Volt,
      PMU_SIM1P8Volt
} PMU_SIMVolt_t;
#elif defined(PMU_BCM59038)
typedef enum {
      PMU_SIM3P0Volt = 0,
      PMU_SIM2P5Volt,
      PMU_SIM0P0Volt,
      PMU_SIM1P8Volt,
      PMU_SIM3P3Volt
} PMU_SIMVolt_t;

#else
typedef enum {
      PMU_SIM3P0Volt = 0,
      PMU_SIM1P8Volt,
      PMU_SIM0P0Volt
} PMU_SIMVolt_t;
#endif

// IMPORTANT: Please make sure to update sys_poweron.h when updating enum below as they are both associated
typedef enum {
    PMU_POWERUP_POWERKEY = 0,
    PMU_POWERUP_ALARM,
    PMU_POWERUP_CHARGER,
    PMU_POWERUP_ONREQ,
    PMU_POWERUP_CHARGER_AND_ONREQ,
    PMU_POWERUP_SMPL,
    PMU_POWERUP_BATTERY,
    PMU_POWERUP_AUX,
    PMU_SYSTEM_RESET
} PMU_PowerupId_t;

//! For PMU_IsChargerPresent params
typedef enum {
	PMU_I2C_INDIRECTCALL = 0,
	PMU_I2C_DIRECTCALL	 
} PMU_I2CDirectCall_t;

typedef enum {
	PMU_NO_CHARGER_IS_PRESENT = 0,		///< No charger is plugged
	PMU_USB_IS_PRESENT,					///< USB charger/cable is present/plugged
	PMU_WAC_IS_PRESENT,					///< WAC (wall adapter charger) is present
	PMU_USB_AND_WAC_ARE_PRESENT			///< Both USB and WAC are present - impossible for 
										///<    handset with one connector, but added for completeness
} PMU_ChargerPresence_t;

#if defined(PMU_BCM59038)
typedef enum
{
	PMU_ADC_CH_VMBAT,
	PMU_ADC_CH_VBBAT,
	PMU_ADC_CH_VWALL,
	PMU_ADC_CH_VBUS,
	PMU_ADC_CH_ID,
	PMU_ADC_CH_NTC,
	PMU_ADC_CH_BSI,
	PMU_ADC_CH_BOM,
	PMU_ADC_CH_32KTEMP,
	PMU_ADC_CH_PATEMP,
	PMU_ADC_CH_ALS,
	PMU_ADC_CH_FEM,
	PMU_ADC_CH_RES1,
	PMU_ADC_CH_RES2,
	PMU_ADC_CH_RES3,
	PMU_ADC_CH_RES4
} PMU_ADC_ChSel_t;
#elif defined(PMU_BCM59055)
typedef enum
{
	PMU_ADC_CH_VMBAT,
	PMU_ADC_CH_VBBAT,
	PMU_ADC_CH_VWALL,
	PMU_ADC_CH_VBUS,
	PMU_ADC_CH_ID,
	PMU_ADC_CH_NTC,
	PMU_ADC_CH_BSI,
	PMU_ADC_CH_BOM,
	PMU_ADC_CH_32KTEMP,
	PMU_ADC_CH_PATEMP,
	PMU_ADC_CH_ALS,
	PMU_ADC_CH_BSI_CAL_L,
	PMU_ADC_CH_NTC_CAL_L,
	PMU_ADC_CH_NTC_CAL_H,
    PMU_ADC_CH_RESERVE,
	PMU_ADC_CH_BSI_CAL_H,
    PMU_ADC_CH_MAX
} PMU_ADC_ChSel_t;
#else
// Remove it when a PMU is defined for Hera
typedef enum
{
	PMU_ADC_CH_VMBAT,
	PMU_ADC_CH_VBBAT,
	PMU_ADC_CH_VWALL,
	PMU_ADC_CH_VBUS,
	PMU_ADC_CH_ID,
	PMU_ADC_CH_NTC,
	PMU_ADC_CH_BSI,
	PMU_ADC_CH_BOM,
	PMU_ADC_CH_32KTEMP,
	PMU_ADC_CH_PATEMP,
	PMU_ADC_CH_ALS,
	PMU_ADC_CH_FEM,
	PMU_ADC_CH_RES1,
	PMU_ADC_CH_RES2,
	PMU_ADC_CH_RES3,
	PMU_ADC_CH_RES4
} PMU_ADC_ChSel_t;
#endif

typedef enum
{
	PMU_ADC_RTMDELAY_NONE,
	PMU_ADC_RTMDELAY_31P25us,
	PMU_ADC_RTMDELAY_62P5us,
	PMU_ADC_RTMDELAY_93P75us,
	PMU_ADC_RTMDELAY_125us,
	PMU_ADC_RTMDELAY_156P25us,
	PMU_ADC_RTMDELAY_187P5us,
	PMU_ADC_RTMDELAY_218P75us,
	PMU_ADC_RTMDELAY_250us,
	PMU_ADC_RTMDELAY_281P25us,
	PMU_ADC_RTMDELAY_312P5us,
	PMU_ADC_RTMDELAY_343P75us,
	PMU_ADC_RTMDELAY_375us,
	PMU_ADC_RTMDELAY_406P25us,
	PMU_ADC_RTMDELAY_437P5us,
	PMU_ADC_RTMDELAY_468P75us,
	PMU_ADC_RTMDELAY_500us,
	PMU_ADC_RTMDELAY_531P25us,
	PMU_ADC_RTMDELAY_562P5us,
	PMU_ADC_RTMDELAY_593P75us,
	PMU_ADC_RTMDELAY_625us,
	PMU_ADC_RTMDELAY_656P25us,
	PMU_ADC_RTMDELAY_687P5us,
	PMU_ADC_RTMDELAY_718P75us,
	PMU_ADC_RTMDELAY_750us,
	PMU_ADC_RTMDELAY_781P25us,
	PMU_ADC_RTMDELAY_812P5us,
	PMU_ADC_RTMDELAY_843P75us,
	PMU_ADC_RTMDELAY_875us,
	PMU_ADC_RTMDELAY_906P25us,
	PMU_ADC_RTMDELAY_937P5us,
	PMU_ADC_RTMDELAY_968P75us
} PMU_RTM_Delay_t;

#ifdef PMU_BCM59055
typedef enum
{
	PMU_FG_TYPE_SMPL,
	PMU_FG_TYPE_SMPL_CAL,
	PMU_FG_TYPE_SMPLB,
    PMU_FG_TYPE_MAX
} PMU_FG_SMPLType_t;
#endif

typedef enum
{
	PMU_AUDIO_HS_LEFT,
	PMU_AUDIO_HS_RIGHT,
	PMU_AUDIO_HS_BOTH
}PMU_HS_path_t;

#if defined(PMU_MAX8986)
typedef enum
{
	PMU_HSGAIN_MUTE = -1,
	PMU_HSGAIN_64DB_N = 0x00,
	PMU_HSGAIN_60DB_N,
	PMU_HSGAIN_56DB_N,
	PMU_HSGAIN_52DB_N,
	PMU_HSGAIN_48DB_N,
	PMU_HSGAIN_44DB_N,
	PMU_HSGAIN_40DB_N,
	PMU_HSGAIN_37DB_N,
	PMU_HSGAIN_34DB_N,
	PMU_HSGAIN_31DB_N,
	PMU_HSGAIN_28DB_N,
	PMU_HSGAIN_25DB_N,
	PMU_HSGAIN_22DB_N,
	PMU_HSGAIN_19DB_N,
	PMU_HSGAIN_16DB_N,
	PMU_HSGAIN_14DB_N,
	PMU_HSGAIN_12DB_N,
	PMU_HSGAIN_10DB_N,
	PMU_HSGAIN_8DB_N,
	PMU_HSGAIN_6DB_N,
	PMU_HSGAIN_4DB_N,
	PMU_HSGAIN_2DB_N,
    PMU_HSGAIN_1DB_N,
    PMU_HSGAIN_0DB,
    PMU_HSGAIN_1DB_P,
    PMU_HSGAIN_2DB_P,
    PMU_HSGAIN_3DB_P,
    PMU_HSGAIN_4DB_P,
    PMU_HSGAIN_4P5DB_P,
    PMU_HSGAIN_5DB_P,
    PMU_HSGAIN_5P5DB_P,
    PMU_HSGAIN_6DB_P
}PMU_HS_Gain_t;

#else
typedef enum
{
	PMU_HSGAIN_MUTE,
	PMU_HSGAIN_66DB_N,
	PMU_HSGAIN_63DB_N,
	PMU_HSGAIN_60DB_N,
	PMU_HSGAIN_57DB_N,
	PMU_HSGAIN_54DB_N,
	PMU_HSGAIN_51DB_N,
	PMU_HSGAIN_48DB_N,
	PMU_HSGAIN_45DB_N,
	PMU_HSGAIN_42DB_N,
	PMU_HSGAIN_40P5DB_N,
	PMU_HSGAIN_39DB_N,
	PMU_HSGAIN_37P5DB_N,
	PMU_HSGAIN_36DB_N,
	PMU_HSGAIN_34P5DB_N,
	PMU_HSGAIN_33DB_N,
	PMU_HSGAIN_31P5DB_N,
	PMU_HSGAIN_30DB_N,
	PMU_HSGAIN_28P5DB_N,
	PMU_HSGAIN_27DB_N,
	PMU_HSGAIN_25P5DB_N,
	PMU_HSGAIN_24DB_N,
	PMU_HSGAIN_22P5DB_N,
	PMU_HSGAIN_22DB_N,
	PMU_HSGAIN_21P5DB_N,
	PMU_HSGAIN_21DB_N,
	PMU_HSGAIN_20P5DB_N,
	PMU_HSGAIN_20DB_N,
	PMU_HSGAIN_19P5DB_N,
	PMU_HSGAIN_19DB_N,
	PMU_HSGAIN_18P5DB_N,
	PMU_HSGAIN_18DB_N,
	PMU_HSGAIN_17P5DB_N,
	PMU_HSGAIN_17DB_N,
	PMU_HSGAIN_16P5DB_N,
	PMU_HSGAIN_16DB_N,
	PMU_HSGAIN_15P5DB_N,
	PMU_HSGAIN_15DB_N,
	PMU_HSGAIN_14P5DB_N,
	PMU_HSGAIN_14DB_N,
	PMU_HSGAIN_13P5DB_N,
	PMU_HSGAIN_13DB_N,
	PMU_HSGAIN_12P5DB_N,
	PMU_HSGAIN_12DB_N,
	PMU_HSGAIN_11P5DB_N,
	PMU_HSGAIN_11DB_N,
	PMU_HSGAIN_10P5DB_N,
	PMU_HSGAIN_10DB_N,
	PMU_HSGAIN_9P5DB_N,
	PMU_HSGAIN_9DB_N,
	PMU_HSGAIN_8P5DB_N,
	PMU_HSGAIN_8DB_N,
	PMU_HSGAIN_7P5DB_N,
	PMU_HSGAIN_7DB_N,
	PMU_HSGAIN_6P5DB_N,
	PMU_HSGAIN_6DB_N,
	PMU_HSGAIN_5P5DB_N,
	PMU_HSGAIN_5DB_N,
	PMU_HSGAIN_4P5DB_N,
	PMU_HSGAIN_4DB_N,
	PMU_HSGAIN_3P5DB_N,
	PMU_HSGAIN_3DB_N,
	PMU_HSGAIN_2P5DB_N,
	PMU_HSGAIN_2DB_N
}PMU_HS_Gain_t;
#endif // PMU_MAX8986
typedef enum
{
	PMU_HSGAIN_SE_MUTE,
	PMU_HSGAIN_SE_60DB_N,
	PMU_HSGAIN_SE_57DB_N,
	PMU_HSGAIN_SE_54DB_N,
	PMU_HSGAIN_SE_51DB_N,
	PMU_HSGAIN_SE_48DB_N,
	PMU_HSGAIN_SE_45DB_N,
	PMU_HSGAIN_SE_42DB_N,
	PMU_HSGAIN_SE_39DB_N,
	PMU_HSGAIN_SE_36DB_N,
	PMU_HSGAIN_SE_34P5DB_N,
	PMU_HSGAIN_SE_33DB_N,
	PMU_HSGAIN_SE_31P5DB_N,
	PMU_HSGAIN_SE_30DB_N,
	PMU_HSGAIN_SE_28P5DB_N,
	PMU_HSGAIN_SE_27DB_N,
	PMU_HSGAIN_SE_25P5DB_N,
	PMU_HSGAIN_SE_24DB_N,
	PMU_HSGAIN_SE_22P5DB_N,
	PMU_HSGAIN_SE_21DB_N,
	PMU_HSGAIN_SE_19P5DB_N,
	PMU_HSGAIN_SE_18DB_N,
	PMU_HSGAIN_SE_16P5DB_N,
	PMU_HSGAIN_SE_16DB_N,
	PMU_HSGAIN_SE_15P5DB_N,
	PMU_HSGAIN_SE_15DB_N,
	PMU_HSGAIN_SE_14P5DB_N,
	PMU_HSGAIN_SE_14DB_N,
	PMU_HSGAIN_SE_13P5DB_N,
	PMU_HSGAIN_SE_13DB_N,
	PMU_HSGAIN_SE_12P5DB_N,
	PMU_HSGAIN_SE_12DB_N,
	PMU_HSGAIN_SE_11P5DB_N,
	PMU_HSGAIN_SE_11DB_N,
	PMU_HSGAIN_SE_10P5DB_N,
	PMU_HSGAIN_SE_10DB_N,
	PMU_HSGAIN_SE_9P5DB_N,
	PMU_HSGAIN_SE_9DB_N,
	PMU_HSGAIN_SE_8P5DB_N,
	PMU_HSGAIN_SE_8DB_N,
	PMU_HSGAIN_SE_7P5DB_N,
	PMU_HSGAIN_SE_7DB_N,
	PMU_HSGAIN_SE_6P5DB_N,
	PMU_HSGAIN_SE_6DB_N,
	PMU_HSGAIN_SE_5P5DB_N,
	PMU_HSGAIN_SE_5DB_N,
	PMU_HSGAIN_SE_4P5DB_N,
	PMU_HSGAIN_SE_4DB_N,
	PMU_HSGAIN_SE_3P5DB_N,
	PMU_HSGAIN_SE_3DB_N,
	PMU_HSGAIN_SE_2P5DB_N,
	PMU_HSGAIN_SE_2DB_N,
	PMU_HSGAIN_SE_1P5DB_N,
	PMU_HSGAIN_SE_1DB_N,
	PMU_HSGAIN_SE_0P5DB_N,
	PMU_HSGAIN_SE_0DB_N,
	PMU_HSGAIN_SE_P5DB_P,
	PMU_HSGAIN_SE_1DB_P,
	PMU_HSGAIN_SE_1P5DB_P,
	PMU_HSGAIN_SE_2DB_P,
	PMU_HSGAIN_SE_2P5DB_P,
	PMU_HSGAIN_SE_3DB_P,
	PMU_HSGAIN_SE_3P5DB_P,
	PMU_HSGAIN_SE_4DB_P
}PMU_HS_SINGLE_END_Gain_t;

#if defined(PMU_MAX8986)
typedef enum
{
    PMU_IHFGAIN_MUTE,
	PMU_IHFGAIN_30DB_N=0x18,
	PMU_IHFGAIN_26DB_N,
	PMU_IHFGAIN_22DB_N,
	PMU_IHFGAIN_18DB_N,
	PMU_IHFGAIN_14DB_N,
	PMU_IHFGAIN_12DB_N,
	PMU_IHFGAIN_10DB_N,
	PMU_IHFGAIN_8DB_N,
	PMU_IHFGAIN_6DB_N,
	PMU_IHFGAIN_4DB_N,
	PMU_IHFGAIN_2DB_N,
	PMU_IHFGAIN_0DB,
	PMU_IHFGAIN_1DB_P,
	PMU_IHFGAIN_2DB_P,
	PMU_IHFGAIN_3DB_P,
	PMU_IHFGAIN_4DB_P,
    PMU_IHFGAIN_5DB_P,
    PMU_IHFGAIN_6DB_P,
    PMU_IHFGAIN_7DB_P,
    PMU_IHFGAIN_8DB_P,
    PMU_IHFGAIN_9DB_P,
    PMU_IHFGAIN_10DB_P,
    PMU_IHFGAIN_11DB_P,
    PMU_IHFGAIN_12DB_P,
    PMU_IHFGAIN_12P5DB_P,
    PMU_IHFGAIN_13DB_P,
    PMU_IHFGAIN_13P5DB_P,
    PMU_IHFGAIN_14DB_P,
    PMU_IHFGAIN_14P5DB_P,
    PMU_IHFGAIN_15DB_P,
    PMU_IHFGAIN_15P5DB_P,
    PMU_IHFGAIN_16DB_P,
    PMU_IHFGAIN_16P5DB_P,
    PMU_IHFGAIN_17DB_P,
    PMU_IHFGAIN_17P5DB_P,
    PMU_IHFGAIN_18DB_P,
    PMU_IHFGAIN_18P5DB_P,
    PMU_IHFGAIN_19DB_P,
    PMU_IHFGAIN_19P5DB_P,
    PMU_IHFGAIN_20DB_P
}PMU_IHF_Gain_t;

#else
typedef enum
{
	PMU_IHFGAIN_MUTE,
	PMU_IHFGAIN_60DB_N,
	PMU_IHFGAIN_57DB_N,
	PMU_IHFGAIN_54DB_N,
	PMU_IHFGAIN_51DB_N,
	PMU_IHFGAIN_48DB_N,
	PMU_IHFGAIN_45DB_N,
	PMU_IHFGAIN_42DB_N,
	PMU_IHFGAIN_39DB_N,
	PMU_IHFGAIN_36DB_N,
	PMU_IHFGAIN_34P5DB_N,
	PMU_IHFGAIN_33DB_N,
	PMU_IHFGAIN_31P5DB_N,
	PMU_IHFGAIN_30DB_N,
	PMU_IHFGAIN_28P5DB_N,
	PMU_IHFGAIN_27DB_N,
	PMU_IHFGAIN_25P5DB_N,
	PMU_IHFGAIN_24DB_N,
	PMU_IHFGAIN_22P5DB_N,
	PMU_IHFGAIN_21DB_N,
	PMU_IHFGAIN_19P5DB_N,
	PMU_IHFGAIN_18DB_N,
	PMU_IHFGAIN_16P5DB_N,
	PMU_IHFGAIN_16DB_N,
	PMU_IHFGAIN_15P5DB_N,
	PMU_IHFGAIN_15DB_N,
	PMU_IHFGAIN_14P5DB_N,
	PMU_IHFGAIN_14DB_N,
	PMU_IHFGAIN_13P5DB_N,
	PMU_IHFGAIN_13DB_N,
	PMU_IHFGAIN_12P5DB_N,
	PMU_IHFGAIN_12DB_N,
	PMU_IHFGAIN_11P5DB_N,
	PMU_IHFGAIN_11DB_N,
	PMU_IHFGAIN_10P5DB_N,
	PMU_IHFGAIN_10DB_N,
	PMU_IHFGAIN_9P5DB_N,
	PMU_IHFGAIN_9DB_N,
	PMU_IHFGAIN_8P5DB_N,
	PMU_IHFGAIN_8DB_N,
	PMU_IHFGAIN_7P5DB_N,
	PMU_IHFGAIN_7DB_N,
	PMU_IHFGAIN_6P5DB_N,
	PMU_IHFGAIN_6DB_N,
	PMU_IHFGAIN_5P5DB_N,
	PMU_IHFGAIN_5DB_N,
	PMU_IHFGAIN_4P5DB_N,
	PMU_IHFGAIN_4DB_N,
	PMU_IHFGAIN_3P5DB_N,
	PMU_IHFGAIN_3DB_N,
	PMU_IHFGAIN_2P5DB_N,
	PMU_IHFGAIN_2DB_N,
	PMU_IHFGAIN_1P5DB_N,
	PMU_IHFGAIN_1DB_N,
	PMU_IHFGAIN_0DB,
	PMU_IHFGAIN_P5DB_P,
	PMU_IHFGAIN_1DB_P,
	PMU_IHFGAIN_1P5DB_P,
	PMU_IHFGAIN_2DB_P,
	PMU_IHFGAIN_2P5DB_P,
	PMU_IHFGAIN_3DB_P,
	PMU_IHFGAIN_3P5DB_P,
	PMU_IHFGAIN_4DB_P
}PMU_IHF_Gain_t;
#endif //defined PMU_MAX8986
// HS Short circuit current limit
typedef enum
{
	HS_SCKT_130MA,
	HS_SCKT_102MA,
	HS_SCKT_83MA,
	HS_SCKT_74MA
} PMU_HS_shortCircuit_curr_t;
#if defined(PMU_MAX8986)
typedef enum
{
    PMU_INA_SINGLE_ENDED_INB_DIFFERENTIAL_MODE0,
    PMU_INA_SINGLE_ENDED_INB_DIFFERENTIAL_MODE1,
    PMU_INA_SINGLE_ENDED_INB_DIFFERENTIAL_MODE2,
    PMU_INA_SINGLE_ENDED_INB_DIFFERENTIAL_MODE3,
    PMU_INA_SINGLE_ENDED_INB_DIFFERENTIAL_MODE4,
    PMU_INA_SINGLE_ENDED_INB_DIFFERENTIAL_MODE5,
    PMU_INA_SINGLE_ENDED_INB_DIFFERENTIAL_MODE6,
    PMU_INA_SINGLE_ENDED_INB_DIFFERENTIAL_MODE7,
    PMU_INA_SINGLE_ENDED_INB_DIFFERENTIAL_MODE8,
    PMU_INA_SINGLE_ENDED_INB_DIFFERENTIAL_MODE9,
    PMU_INA_SINGLE_ENDED_INB_DIFFERENTIAL_MODE10,
    PMU_INA_DIFFERENTIAL_INB_SINGLE_ENDED_MODE6
}PMU_HS_Inputmode_t;
typedef enum
{
    PMU_INPUT_PREAMPGAIN_6DB_N,
	PMU_INPUT_PREAMPGAIN_3DB_N,
	PMU_INPUT_PREAMPGAIN_0DB,
	PMU_INPUT_PREAMPGAIN_3DB_P,
	PMU_INPUT_PREAMPGAIN_6DB_P,
	PMU_INPUT_PREAMPGAIN_9DB_P,
	PMU_INPUT_PREAMPGAIN_18DB_P,
	PMU_INPUT_PREAMPGAIN_EXTERNAL   
}PMU_HS_Input_Preamp_Gain_t;

typedef struct
{
	PMU_HS_Input_Preamp_Gain_t      input_a_gain;
	PMU_HS_Input_Preamp_Gain_t		input_b_gain;
}PMU_HS_INPUT_PREAMP_st_t;

#elif defined(PMU_BCM59038)
typedef enum
{
	PMU_HS_DIFFERENTIAL,
	PMU_HS_SINGLE_ENDED
}PMU_HS_Inputmode_t;	
#elif defined(PMU_BCM59055)
typedef enum
{
	PMU_HS_DIFFERENTIAL_DC_COUPLED,
    PMU_HS_DIFFERENTIAL_AC_COUPLED,
	PMU_HS_SINGLE_ENDED_AC_COUPLED
}PMU_HS_Inputmode_t;

typedef enum
{
	PMU_HS_LOW_GAIN,
	PMU_HS_HIGH_GAIN
}PMU_HS_Gainmode_t;	
#else
//Remove it when PMU is defined for Hera
typedef enum
{
	PMU_HS_DIFFERENTIAL,
	PMU_HS_SINGLE_ENDED
}PMU_HS_Inputmode_t;
#endif


typedef struct
{
	PMU_HS_Inputmode_t hs_input_mode;
	PMU_HS_Gain_t	hs_gain;
	PMU_HS_path_t 	hs_path;
	PMU_HS_shortCircuit_curr_t hs_skt_thresh;
	Boolean			sktEnable;
	Boolean			hs_power;
    UInt8           temp1;
    UInt8           temp2;
}PMU_HS_st_t;

typedef struct
{
	PMU_IHF_Gain_t ihf_gain;
	Boolean		Bypass_En;
}PMU_IHF_st_t;

typedef struct 
{
	PMU_ADC_ChSel_t	channel;
	UInt16	*data;
}PMU_ADC_data_st_t;

typedef struct 
{
	PMU_ADC_ChSel_t	startChannel;
    UInt8 numChannels;
	UInt16	*data;
}PMU_ADC_mult_data_st_t;

typedef struct 
{
	PMU_ADC_ChSel_t	rtmChannel;
	UInt16	*rtmData;
	PMU_RTM_Delay_t rtmDelay;
}PMU_RTM_data_st_t;

#ifdef PMU_BCM59055
typedef struct 
{
	PMU_FG_SMPLType_t	type;
	short	*smpl;
}PMU_FGSMPL_data_st_t;
#endif

typedef enum {
	PMU_BC_OTG_ID_FLOAT = 0,
	PMU_BC_OTG_ID_GND,
    PMU_BC_OTG_ID_RID_A,			 
	PMU_BC_OTG_ID_RID_B, 
	PMU_BC_OTG_ID_RID_C,
    PMU_BC_OTG_ID_UNKNOWN
} PMU_BC_OTG_Id_t;

typedef struct
{
	Boolean vbus_valid;
	Boolean a_session_valid;
	Boolean	b_session_end;
	Boolean id_b_device;
	Boolean otg_b_device;
}PMU_OTGStatus_st_t;

typedef struct
{
	UInt16 battmgr_BattPercentageLevel;
	UInt16 battmgr_VoltAvgBeforeChargerOn;
}PMU_BattmgrVars_st_t;


typedef enum
{
	BATTPERCERTAGELEVEL_VAR = 0,
	VOLTAVGBEFORECHARGERON_VAR
}PMU_BattmgrVars_en_t;



//! PMU register type
typedef UInt8 PMU_REG_ID_t;

//! PMU driver error codes definition
typedef enum
{
	PMU_DRV_SUCCESS = 0,							///< Success
	PMU_DRV_ERROR_API_NOT_SUPPORTED,				///< If API is not supported by PMU driver due to obselete PMU or so.
	PMU_DRV_ERROR_FEATURE_NOT_SUPPORTED_BY_PMU, 		///< if PMU IC doesn't support feature (ex. 59001 does not have GPOs)
	PMU_DRV_ERROR_INTERNAL_ERROR,
	PMU_DRV_ERROR_OTHERS
} PMU_DRV_ErrorCode_en_t;

//! Enum type for callback registration from HAL_EM_BATTMGR on hardware and batt/charging events
typedef enum 
{
	PMU_DRV_REGISTER_EXTREME_TEMP_CB = 0,  		///< Callback for extreme high or low batt temp - outside temp window 
	PMU_DRV_REGISTER_START_CHARGING_CB,			///< Callback for start charging event
	PMU_DRV_REGISTER_ENDOFCHARGE_CB,		 	///< Callback for batt has been charged to full event
	PMU_DRV_REGISTER_STOP_CHARGING_CB,			///< Callback for stop charging event
	PMU_DRV_REGISTER_BATTLOW_CB,				///< Callback for batt low event
	PMU_DRV_REGISTER_BATTEMPTY_CB,				///< Callback for batt empty event
#if defined(PMU_BCM59036) || defined(PMU_BCM59055)
	PMU_DRV_REGISTER_USB_CHRGINS_CB,			///< Callback for USB charger insertion 
	PMU_DRV_REGISTER_WALL_CHRGINS_CB,			///< Callback for Wall charger insertion
	PMU_DRV_REGISTER_USB_CHRGRM_CB,				///< Callback for USB charger removal
	PMU_DRV_REGISTER_WALL_CHRGRM_CB,			///< Callback for Wall charger removal
	PMU_DRV_REGISTER_BATREM_CB,					///< Callback for battery removal interrupt
#endif	
	PMU_DRV_REGISTER_BATTLEVEL_CB				///< Callback for batt level change event, as informed by batt monitor 
} PMU_DRV_RegisterCallbacks_en_t;

//! Enum type for Onkey events. 
typedef enum{
	PMU_DRV_ONKEY_LONG_KEY,		 
	PMU_DRV_ONKEY_PRESS,
	PMU_DRV_ONKEY_RELEASE
} PMU_DRV_OnkeyEvent_en_t;

//--------------------------------------------------------------
// Hardware EVENTS definitions
//--------------------------------------------------------------
/**  HAL_EM_PMU Device ACTION  definitions
*
*	If action is not supported by the pmu device on-board, function will return error.
*/

//! PMU driver hardware event type.  MULTIPLE CLIENTS ARE ALLOWED AND HANDLED IN HAL.
//! USB charger detection events are available at HAL ACCESSORIES
typedef enum
{	  	
	PMU_DRV_WAC_PLUG_IN_EVENT = 0,				///< Wallcharger adapter plug in event
	PMU_DRV_WAC_PLUG_OUT_EVENT,					///< Wallcharger adapter plug OUT event

    // the following are for B0 and C0
    PMU_DRV_VBUS_VALID_F,                        ///< Vbus valid falling edge interrupt
    PMU_DRV_VBUS_VALID_R,                        ///< Vbus valid rising edge interrupt
    PMU_DRV_A_SESS_VALID_F,                      ///< A session valid falling edge interrupt
    PMU_DRV_A_SESS_VALID_R,                      ///< A session valid rising edge interrupt
    PMU_DRV_B_SESS_END_F,                        ///< B session end falling edge interrupt
    PMU_DRV_B_SESS_END_R,                        ///< B session end falling edge interrupt
    PMU_DRV_ID_INSRT,                            ///< ID insertion interrupt
    PMU_DRV_RESUME_VBUS,                         ///< resume Vbus interrupt
    PMU_DRV_BATT_INSRT,                          ///< Battery insertion interrupt
    PMU_DRV_ID_REMOVE,                           ///< ID remove interrupt                                                
	PMU_DRV_ID_CHANGE,							///< ID change interrupt
    PMU_DRV_VBUS_OVRI_INITIAL,                   ///< Initial Vbus over current interrupt
    PMU_DRV_VBUS_OVRI_VBUS_RESTARTED,            ///< Vbus has completed restart after over current interrupt
    PMU_DRV_VBUS_OVRI,                           ///< Vbus over current interrupt
    PMU_DRV_VBUS_OV,                             ///< Vbus over voltage interrupt
    PMU_DRV_VBUS_PULSE_DONE,                     ///< Vbus pulsing done
    PMU_DRV_VBUS_PULSE_FAIL,                     ///< Vbus pulsing failure
    PMU_DRV_SRP_PULLDOWN_DONE,                   ///< SRP active pull down done  
    PMU_DRV_SRP_PULLDOWN_FAIL,                   ///< SRP active pull down failure
    PMU_DRV_SWITCH_TO_USB_CHRG,                  ///< ask user if wants to switch to USB charging from Wall
                                                
    // the following are for C0 
    PMU_DRV_CHG_ERR_DIS,                         ///< charger error goes away  
    PMU_DRV_LEAVE_PAUSE,                         ///< leave charger pause state
    PMU_DRV_LOWBAT,								///< Low battery indication                                          
	PMU_DRV_BATINS,								///< Battery insertion
	PMU_DRV_RESUME_VWALL,						///< Resume Wall charging
	PMU_DRV_CHGDET_SAMPLED,						///< Charger type detected
	PMU_DRV_CHDET_TIMEOUT,						///< Charger detection timed out
	PMU_DRV_RIDC_2_FLOAT,						///< ID changes from RID_C to float
	PMU_DRV_VBUS_LOWBOUND,						///< VBUS reached low threshold boundary						
	PMU_DRV_HEADSET_SHORT,						///< Audio headset short
	PMU_DRV_CLASSD_SHORT,						///< Audio - Class D short
    // added for 59055 below
    PMU_DRV_END_OF_CHARGE,
    PMU_DRV_WALL_CHARGER_OV,
    PMU_DRV_USB_CHARGER_OV,
    PMU_DRV_UBPD_CHP_DIS,
    PMU_DRV_WAC_CC_REDUCED,  // 59055 B0
    PMU_DRV_USB_CC_REDUCED,  // 59055 B0
    PMU_DRV_WAC_REV_POL,
    PMU_DRV_USB_REV_POL,
    PMU_DRV_CV_TMR_EXP,
    PMU_DRV_CGPD_CHG_F,
    PMU_DRV_UBPD_CHG_F,
    PMU_DRV_ACP7INS,
    PMU_DRV_MBC_OVER_VOLTAGE_DISAPPEAR,
    PMU_DRV_CHG_OVER_VOLTAGE_DISAPPEAR,
    PMU_DRV_USB_OVER_VOLTAGE_DISAPPEAR,
    PMU_DRV_USB_PORT_DISABLE,
    PMU_DRV_CHG_SW_TMR_EXP,
    PMU_DRV_CHG_HW_TMR_EXP,
    PMU_DRV_MBC_OVER_VOLTAGE,
    PMU_DRV_CHGDET_LATCH,
    PMU_DRV_CHGDET_TO,
    PMU_DRV_RID_C_TO_FLT,
    PMU_DRV_ADP_CHANGE,
    PMU_DRV_ADP_SNS_END,
    PMU_DRV_ADP_PROBE,
    PMU_DRV_ADP_REF_PROBE,				///< Reference probe done
    PMU_DRV_ADP_PROBE_ERR,				///< ADP Probing error event
    PMU_DRV_MBWV_R_10S,
    PMU_DRV_MBC_XTAL_FAILURE,
	PMU_DRV_FGSMPL_READ_FAIL,			///< FGSMPL read ready
	PMU_DRV_FGSMPL_READ_DONE,			///< FGSMPL read failed
	PMU_DRV_MAX_EVENTS
} 	PMU_DRV_Event_en_t;	   

//! Event callback routine definition
//! Client will only need to provide one function for every event 
//! register and use switch for event handling.
//!
//! Refer to glue for example.
typedef void  (*PMU_DRV_cb_t)(PMU_DRV_Event_en_t event);

//--------------------------------------------------------------
/// Hardware EVENTS definitions
//--------------------------------------------------------------

/**  HAL_EM_PMU Device EVENTS PRIVATE definitions
*/
//! PMU driver hardware event type. 
//! NOTE:  		ONLY ONE CLIENT IS ALLOWED. NO MULTIPLE CLIENT REGISTRATION OR WILL RETURN ERROR CODE: EM_PMU_ERROR_EVENT_HAS_A_CLIENT.
//! WARNING: 	Enum item numbering must be maintained from 0 to EM_PMU_MAX_PRIVATE_EVENT as it is used as offset in cb list array 
typedef enum
{
	PMU_DRV_REC1_ACD_RISING_EDGE_EVENT = 0, 		///< REC1 is accessory detection 1 for Philips and ACD is accessory detection for BRCM PMU 
												///< Both function the same (direct map). This is for Rising edge detection. (usually accessory "plug out" detection)
	PMU_DRV_REC1_ACD_FALLING_EDGE_EVENT,			///< This is for falling edge detection. Usually accessory "plug in" detection.
	PMU_DRV_REC2_PHFD_HIGH_RISING_EDGE_EVENT,	///< REC2 of Philips is the same as PHFD of BRCM PMU.  Typically, for "plug out" headset detection
	PMU_DRV_REC2_PHFD_HIGH_FALLING_EDGE_EVENT,	///< Typically, for "plug in" headset detection event.
	PMU_DRV_REC2_PHFD_LOW_RISING_EDGE_EVENT,		///< Typically, for "release" button (if headset has button) detection event.
	PMU_DRV_REC2_PHFD_LOW_FALLING_EDGE_EVENT,   	///< Typically, for "press" button detection event. 
	PMU_DRV_USB_PLUG_IN_EVENT,					///< USB  plug IN event
	PMU_DRV_USB_PLUG_OUT_EVENT,					///< USB plug OUT event
	PMU_DRV_PONKEY_LONG_KEY,						///< ONKEY long press (for power key). Debounced time is programmable in PMU or it will use reset value, typically 1s.
	PMU_DRV_PONKEY_RELEASE,						///< Rising edge is detected -- onkey is RELEASED
	PMU_DRV_PONKEY_PRESS,							///< Falling edge is detected -- onkey is PRESSED.
	PMU_DRV_EOC_EVENT,							///< End of Charge event
    PMU_DRV_ALDO1_OVER_CURRENT,                  ///< ALDO1 over current
    PMU_DRV_ALDO2_OVER_CURRENT,                  ///< ALDO2 over current
    PMU_DRV_RFLDO1_OVER_CURRENT,                 ///< RFLDO1 over current
    PMU_DRV_RFLDO2_OVER_CURRENT,                 ///< RFLDO2 over current
    PMU_DRV_HCLDO1_OVER_CURRENT,                 ///< HCLDO1 over current
    PMU_DRV_HCLDO2_OVER_CURRENT,                 ///< HCLDO2 over current
    PMU_DRV_MSLDO1_OVER_CURRENT,                 ///< MSLDO1 over current
    PMU_DRV_MSLDO2_OVER_CURRENT,                 ///< MSLDO2 over current
    PMU_DRV_LCLDO_OVER_CURRENT,                  ///< LCLDO over current
    PMU_DRV_LVLDO1_OVER_CURRENT,                 ///< LVLDO1 over current
    PMU_DRV_LVLDO2_OVER_CURRENT,                 ///< LVLDO2 over current
    PMU_DRV_IOLDO_OVER_CURRENT,                  ///< IOLDO over current
    PMU_DRV_SIMLDO_OVER_CURRENT,                 ///< SIMLDO over current
    PMU_DRV_AXLDO1_OVER_CURRENT,                 ///< AXLDO1 over current
    PMU_DRV_AXLDO2_OVER_CURRENT,                 ///< AXLDO2 over current

    PMU_DRV_SDSR_OVER_CURRENT,                   ///< SDSR over current (59055)
    PMU_DRV_CAMLDO_OVER_CURRENT,                 ///< CAMLDO over current (59055)
    PMU_DRV_RFLDO_OVER_CURRENT,                  ///< RFLDO over current (59055)
    PMU_DRV_HVLDO1_OVER_CURRENT,                 ///< HVLDO1 over current (59055)
    PMU_DRV_HVLDO2_OVER_CURRENT,                 ///< HVLDO2 over current (59055)
    PMU_DRV_HVLDO3_OVER_CURRENT,                 ///< HVLDO3 over current (59055)
    PMU_DRV_HVLDO4_OVER_CURRENT,                 ///< HVLDO4 over current (59055)
    PMU_DRV_HVLDO5_OVER_CURRENT,                 ///< HVLDO5 over current (59055)
    PMU_DRV_HVLDO6_OVER_CURRENT,                 ///< HVLDO6 over current (59055)
    PMU_DRV_HVLDO7_OVER_CURRENT,                 ///< HVLDO7 over current (59055)
    PMU_DRV_CAMLDO_OVER_CURRENT_SHD,             ///< CAMLDO over current shutdown (59055)
    PMU_DRV_RFLDO_OVER_CURRENT_SHD,              ///< RFLDO over current shutdown (59055)
    PMU_DRV_HVLDO1_OVER_CURRENT_SHD,             ///< HVLDO1 over current shutdown (59055)
    PMU_DRV_HVLDO2_OVER_CURRENT_SHD,             ///< HVLDO2 over current shutdown (59055)
    PMU_DRV_HVLDO3_OVER_CURRENT_SHD,             ///< HVLDO3 over current shutdown (59055)
    PMU_DRV_HVLDO4_OVER_CURRENT_SHD,             ///< HVLDO4 over current shutdown (59055)
    PMU_DRV_HVLDO5_OVER_CURRENT_SHD,             ///< HVLDO5 over current shutdown (59055)
    PMU_DRV_HVLDO6_OVER_CURRENT_SHD,             ///< HVLDO6 over current shutdown (59055)
    PMU_DRV_HVLDO7_OVER_CURRENT_SHD,             ///< HVLDO7 over current shutdown (59055)
    PMU_DRV_SIMLDO_OVER_CURRENT_SHD,             ///< SIMLDO over current shutdown (59055)

    PMU_DRV_MBC_THERMAL_FOLDBACK,

    PMU_DRV_IOSR_OVER_CURRENT,                   ///< io switching regulator over current
    PMU_DRV_CSR_OVER_CURRENT,                    ///< core switching regulator over current
    PMU_DRV_IOSR_OVER_VOLTAGE,                   ///< io switching regulator over voltage
    PMU_DRV_CSR_OVER_VOLTAGE,                    ///< core switching regulator over voltage
    PMU_DRV_MB_TEMP_FAULT_LIMIT,                 ///< Maximum temp fault limit reached for Main Battery
    PMU_DRV_MB_TEMP_LOW,                         ///< Temperature is too low for Main Battery
    PMU_DRV_MB_TEMP_HIGH,                        ///< Temperature is too high for Main Battery                                                
    PMU_DRV_MB_REMOVAL,                          ///< Main Battery is removed                                                
    PMU_DRV_MB_OVER_VOLTAGE,                     ///< Over voltage event for Main Battery
	PMU_DRV_RTM_DATA_RDY,						///< SAR ADC RTM data ready
	PMU_DRV_RTM_INTR_CONT,						///< RTM request arrives while continuous conversion
	PMU_DRV_RTM_MAX_REQ,							///< Max RTM request exceeded within a 20ms window
	PMU_DRV_RTM_IGNORE,							///< RTM request ignored
	PMU_DRV_RTM_OVERRIDE,						///< RTM over ridden
	PMU_DRV_CONT_ADC_DATA_READ_DONE,				///< Continuous ADC data ready
	PMU_DRV_CONT_ADC_DATA_READ_FAIL,				///< Continuous ADC data read failed
#if defined(PMU_MAX8986)
    PMU_DRV_USB_HEADSET_PLUG_IN_EVENT,             ///< ADC = 1000K or 1002K ohms
    PMU_DRV_USB_HEADSET_PLUG_OUT_EVENT,             ///< ADC = 1000K or 1002K ohms

    PMU_DRV_USB_CABLE_PLUG_IN_EVENT,                       ///< USB Cable attached
    PMU_DRV_USB_CABLE_PLUG_OUT_EVENT,                       ///< USB Cable attached
    PMU_DRV_USB_CDP_PLUG_IN_EVENT,                         ///< Charging Downstream port: current depends on USB operating speed
    PMU_DRV_USB_CDP_PLUG_OUT_EVENT,                         ///< Charging Downstream port: current depends on USB operating speed
    PMU_DRV_USB_DC1800MA_PLUG_IN_EVENT,                    ///< dedicated charger: current up to 1.8A
    PMU_DRV_USB_DC1800MA_PLUG_OUT_EVENT,                    ///< dedicated charger: current up to 1.8A
    PMU_DRV_USB_SC500MA_PLUG_IN_EVENT,                     ///< Special 500mA charger: Current 500mA max
    PMU_DRV_USB_SC500MA_PLUG_OUT_EVENT,                     ///< Special 500mA charger: Current 500mA max
    PMU_DRV_USB_SC1000MA_PLUG_IN_EVENT,                    ///< Special 1A charger: Current up to 1A
    PMU_DRV_USB_SC1000MA_PLUG_OUT_EVENT,                    ///< Special 1A charger: Current up to 1A
    PMU_DRV_USB_DBC100MA_PLUG_IN_EVENT,                    ///< Dead Battery Charging - 100mA max
    PMU_DRV_USB_DBC100MA_PLUG_OUT_EVENT,                    ///< Dead Battery Charging - 100mA max

#endif
	PMU_DRV_MAX_PRIVATE_EVENT
} PMU_DRV_Event_Private_en_t;

typedef struct 
{
	Boolean *ApiTable;	///< Pointer to an array; holds TRUE or FALSE for each API corresponding to HAL Command action
}PMU_DRV_API_Table_st_t;


typedef struct 
{
	Boolean *PrivateApiTable; ///<Pointer to an array; holds TRUE or FALSE for each API corresponding to HAL Private Command action
}PMU_DRV_Private_API_Table_st_t;


	
//! Callback routine definition for events
typedef void (*PMU_DRV_Private_cb_t)(PMU_DRV_Event_Private_en_t event);							   

/*
 * broadcast event handler definition
 */
//typedef void (*HAL_BroadcastEvent_handler_t)(HAL_EM_PMU_Event_en_t eventID, void *data1, void *data2);
typedef void (*HAL_BroadcastEvent_handler_t)(PMU_DRV_Event_en_t eventID);
typedef void (*HAL_BroadcastPrivateEvent_handler_t)(PMU_DRV_Event_Private_en_t eventID);
//typedef void (*HAL_BroadcastIdChangEvent_handler_t)(PMU_OTG_Id_t prevID, PMU_OTG_Id_t currID);


//! The events related to OnKey press
typedef enum
{
	HAL_KPD_ONKEY_LONG_KEY = 8,		///< ONKEY long press (for power key)
	HAL_KPD_ONKEY_RELEASE = 9,		///< Rising edge is detected -- onkey is RELEASED
	HAL_KPD_ONKEY_PRESS = 10		///< Falling edge is detected -- onkey is PRESSED.
} PMU_DRV_KPD_OnKey_en_t;


//! Keypad callback routine definition for onkey handler
typedef void	(*PMU_DRV_Keypad_OnKey_cb_t)(PMU_DRV_KPD_OnKey_en_t event);							   


//*****************************************************************************
/**
*----- Callback Definitions ------------------------------------------------
*
*
*******************************************************************************/
typedef void (*PMU_Powerup_Cb_t)( PMU_PowerupId_t );

//*****************************************************************************
/**
* for END key event call back function
*
*	@param Id (in) 			
*
*******************************************************************************/
typedef void ( *PMU_EndKeyHandler_t )( PMU_InterruptId_t Id );

//*****************************************************************************
/**
* 
*	@param Value(in) 			
*
*******************************************************************************/
typedef void ( *PMU_GetContent_Cb_t )( UInt8 Value );

//*****************************************************************************
/**
* 
*	@param ChargerPresence(in) 			
*
*******************************************************************************/
typedef void ( *PMU_IsChargerPresent_Cb_t )( PMU_ChargerPresence_t ChargerPresence );

//*****************************************************************************
/**
* 
*	Callback for HS Gain 			
*
*******************************************************************************/
typedef void ( *pmu_HeadsetGain_Cb_t )( void );

//*****************************************************************************
/**
* 
*	Callback for HS Gain 			
*
*******************************************************************************/
typedef void ( *pmu_IHFGain_Cb_t )( void );

//*****************************************************************************
/**
* 
*	@param On(in) 			
*
*******************************************************************************/
typedef void ( *PMU_HS_Power_Cb_t )( Boolean On );

//*****************************************************************************
/**
* 
*	@param On(in) 			
*
*******************************************************************************/
typedef void ( *PMU_IHF_Power_Cb_t )( Boolean On );

//*****************************************************************************
/**
* 
*	@param On(in) 			
*
*******************************************************************************/
typedef void ( *PMU_IHF_Bypass_Cb_t )( Boolean On );

//*****************************************************************************
/**
* 
*	@param HSmode(in) 			
*
*******************************************************************************/
typedef void ( *pmu_HeadsetInputmode_Cb_t )( PMU_HS_Inputmode_t HSmode );

//! Callback types for hardware events
typedef void (*PMU_DRV_BattEvent_cb_t)( void );	///< Battery manager callback function for other than level change


//! Callback type for battmgr variables
typedef void (*PMU_DRV_BattVars_cb_t)( PMU_BattmgrVars_en_t BattmgrVarsEnum, PMU_BattmgrVars_st_t BattmgrVarsSt );




///! Battery level status callback type
//*****************************************************************************
/**
* 
*	@param level(in)
*	@param adc_avg(in) 	
*	@param total_levels(in) 	
*
*******************************************************************************/
typedef void (*PMU_DRV_BattLevel_cb_t)(			
	UInt8 	level,								///< Current battery level
	UInt16	adc_avg,						   	///< Running average of raw ADC values
	UInt8	total_levels					   	///< Total battery levels
	);

#if defined (PMU_BCM59036) || defined (PMU_BCM59055)
//! Callback types for charger events
// S40 team needs to define the actual callback type for various charger events. This is just a prototype
typedef void (*PMU_DRV_USBChrgIns_cb_t)( void );	///< Callback function for USB charger insertion
typedef void (*PMU_DRV_WallChrgIns_cb_t)( void );	///< Callback function for Wall charger insertion
typedef void (*PMU_DRV_USBChrgRem_cb_t)( void );	///< Callback function for USB charger removal
typedef void (*PMU_DRV_WallChrgRem_cb_t)( void );	///< Callback function for Wall charger removal
typedef void (*PMU_DRV_batrem_cb_t)( Boolean batRemoved );		///< Callback function for battery removal
typedef void (*RTCAdjCBF_T)(void);	///< Callback function for RTC adjustment 
#endif

#if defined (PMU_BCM59036) || defined (PMU_BCM59055)
//! IBAT reading callback type
typedef void (*PMU_DRV_IBattReading_cb_t)(
	Int32	sampleCount							///< Sample Count
	);
#endif

#ifndef HAL_INCLUDED
///! Onkey event type, if HAL is not included 
//*****************************************************************************
/**
*   @param Id(in)
*
*******************************************************************************/
typedef void   (*pmu_drv_OnkeyEvent_Cb_t)( PMU_DRV_OnkeyEvent_en_t Id );
#endif	 //#ifndef HAL_INCLUDED
//******************************************************************************
//                          customer define block
//******************************************************************************

#define PMU_WRITE_CB_t I2C_WRITE_CB_t
#define PMU_READ_CB_t  I2C_READ_CB_t

//*****************************************************************************
/**
*
*******************************************************************************/
typedef void (*PMU_REC2Status_Cb_t)( Boolean );

//*****************************************************************************
/**
*
*******************************************************************************/
typedef void (*PMU_REC1Status_Cb_t)( Boolean );

//******************************************************************************
//                          API prototypes block
//******************************************************************************

//**************************************************************************************
/**
	PMU_DRV_Init
**/
void PMU_DRV_Init ( void );
//*******************************************************************************
/**
	PMU_DRV_ClientPowerDown
**/
void PMU_DRV_ClientPowerDown( void );
//*****************************************************************************
/**
*	@param PowerupCb (in) 	
*
*******************************************************************************/
void PMU_DRV_GetPowerupCause( PMU_Powerup_Cb_t PowerupCb );
//*****************************************************************************
/**
*	@param simldo (in) SIM LDO
*	@param volt (in) 	
*
*******************************************************************************/
void PMU_DRV_ActivateSIM( PMU_SIMLDO_t simldo, PMU_SIMVolt_t volt );

//*****************************************************************************
/**
*	PMU_DRV_IsSIMReady
*	@param simldo (in) SIM LDO
*	@return 
*******************************************************************************/
Boolean PMU_DRV_IsSIMReady( PMU_SIMLDO_t simldo );

//*****************************************************************************
/**
*	@param pmuRegId (in) 	
*	@param pmuReadCb (in) 	
*
*******************************************************************************/
void PMU_Read ( PMU_REG_ID_t pmuRegId, PMU_READ_CB_t  pmuReadCb );

//*****************************************************************************
/**
*
*	PMU_DRV_GetAPITable: Get the PMU API Status table
*
*	Called By: HAL PMU in its HAL_EM_PMU_Init
*
*	Calls: None
*
*   	@param void
*   	@return  PMU_DRV_API_Table_st_t * ; pointer to a structure 
*   	
*	@note    This function returns the pointer to a structure holding an array of TRUE/FALSE values for each
*	API (corresponding to HAL Command Action)
*	The API status table is filled during PMU_DRV_Init
*   
******************************************************************************/

PMU_DRV_API_Table_st_t *PMU_DRV_GetAPITable (void);


//*****************************************************************************
/**
*
*	PMU_DRV_GetPrivateAPITable: Get the PMU Private API Status table
*
*	Called By: HAL PMU Private in its HAL_EM_PMU_Init_Private
*
*	Calls: None
*
*   	@param void
*   	@return  PMU_DRV_Private_API_Table_st_t * ; pointer to a structure 
*   	
*	@note    This function returns the pointer to a structure holding an array of TRUE/FALSE values for each
*	API (corresponding to HAL Private Command Action)
*	The Private API status table is filled during PMU_DRV_Init
*   
******************************************************************************/

PMU_DRV_Private_API_Table_st_t *PMU_DRV_GetPrivateAPITable (void);


//*****************************************************************************
/**
*
*	PMU_DRV_Ctrl: 
*
*	Called By: HAL_EM_PMU_Ctrl
*
*	Calls: PMU specific API to provide the HAL action functionality
*
*	@param: Uint16 action; corresponds to HAL_EM_PMU_Action_en_t action code
*	@param: void *data - can not be NULL; assert in the implementation
*	@param:	void *callback - can be NULL
*   	@return  PMU_DRV_ErrorCode_en_t 
*			PMU_DRV_SUCCESS - Success
*			PMU_DRV_ERROR_API_NOT_SUPPORTED - API not implemented
*			PMU_DRV_ERROR_FEATURE_NOT_SUPPORTED_BY_PMU - Action not supported
*			PMU_DRV_ERROR_INTERNAL_ERROR - unrecoverable error
*			PMU_DRV_ERROR_OTHERS - Unknown Error
*   	
*	@note    This function is used by HAL layer to call device (PMU) specific API.
*   
******************************************************************************/
PMU_DRV_ErrorCode_en_t PMU_DRV_Ctrl(UInt16 action, void *data, void *callback);



//*****************************************************************************
/**
*
*	PMU_DRV_Ctrl_Private: 
*
*	Called By: HAL_EM_PMU_Ctrl_Private
*
*	Calls: PMU specific API to provide the HAL action functionality
*
*	@param: Uint16 action; corresponds to HAL_EM_PMU_Action_Private_en_t action code
*	@param: void *data - can not be NULL; assert in the implementation
*	@param:	void *callback - can be NULL
*   	@return  HAL_EM_PMU_Result_Private_en_t 
*			PMU_DRV_SUCCESS - Success
*			PMU_DRV_ERROR_API_NOT_SUPPORTED - API not implemented
*			PMU_DRV_ERROR_FEATURE_NOT_SUPPORTED_BY_PMU - Action not supported
*			PMU_DRV_ERROR_INTERNAL_ERROR - unrecoverable error
*			PMU_DRV_ERROR_OTHERS - Unknown Error
*   	
*	@note    This function is used by HAL layer to call device (PMU) specific API.
*   
******************************************************************************/

PMU_DRV_ErrorCode_en_t PMU_DRV_Ctrl_Private(UInt16 action, void *data, void *callback);



#if defined(PMU_BCM59055)
//*****************************************************************************
/**
*	PMU_ReadFromXscript
*   @param pmuRegId (in) 		
*   @param data (in)
*
*******************************************************************************/
void PMU_ReadFromXscript( PMU_REG_ID_t pmuRegId, UInt8* data );

//*****************************************************************************
/**
*	PMU_WriteFromXscript
*   @param pmuRegId (in) 		
*   @param data (in)
*
*******************************************************************************/
void PMU_WriteFromXscript( PMU_REG_ID_t pmuRegId, UInt8 data );

void PMU2_Read( PMU_REG_ID_t pmuRegId, PMU_READ_CB_t pmuReadCb );
void PMU2_Write( PMU_REG_ID_t pmuRegId, PMU_WRITE_CB_t pmuWriteCb, I2C_DATA_t i2cData );
#endif
//*****************************************************************************
/**
*	@param pmuRegId (in) 	
*	@param pmuWriteCb (in) 	
*	@param i2cData (in) 
*
*******************************************************************************/
void PMU_Write( PMU_REG_ID_t pmuRegId, PMU_WRITE_CB_t pmuWriteCb, I2C_DATA_t i2cData );

//*****************************************************************************
/**
*	@param Id (in) 	
*	@param isr (in) 	
*
*******************************************************************************/
#if defined(PMU_BCM59055)
void PMU_IRQRegister( PMU_InterruptId_t Id, pmuisr_t isr );
#else
void PMU_IRQRegister( PMU_InterruptId_t Id, isr_pmu_t isr );
#endif
//*******************************************************************************
/**
*	PMU_ISR
*******************************************************************************/
void PMU_ISR( void );

/**
*   PMU_I2C_Configure
*  
*   @param  pmuI2cAccess (in)
*  
*	@return Boolean
*******************************************************************************/
Boolean PMU_I2C_Configure(I2C_ACCESS_t * pmuI2cAccess);

#if defined (PMU_BCM59001) || defined (PMU_BCM59035) || defined(PMU_BCM59036)|| defined(PMU_BCM59038) || defined(PMU_BCM59055) || defined(PMU_MAX8986) 
#if defined(BCM59035_REV_B0) || defined(PMU_BCM59036)|| defined(PMU_BCM59038) || defined(PMU_BCM59055) || defined(PMU_MAX8986)
#if !defined(PMU_MAX8986)
//*****************************************************************************
/**
*	PMU_DRV_TurnOnMicBias
*	@return 
*******************************************************************************/
PMU_DRV_ErrorCode_en_t PMU_DRV_TurnOnMicBias( void );
//*****************************************************************************
/**
*	PMU_DRV_TurnOffMicBias
*	@return 
*******************************************************************************/
PMU_DRV_ErrorCode_en_t PMU_DRV_TurnOffMicBias( void );
//*****************************************************************************
/**
*	PMU_DirectTurnOffMicBias
*	@return 
*******************************************************************************/
PMU_DRV_ErrorCode_en_t PMU_DirectTurnOffMicBias( void );
PMU_DRV_ErrorCode_en_t PMU_DRV_EnablePmuInterrupt( void );
//*****************************************************************************
/**
*     @param on (in)
*	@return 
*
*******************************************************************************/
PMU_DRV_ErrorCode_en_t PMU_DRV_CtrlNTCBlock( Boolean on );
//*****************************************************************************
/**
*     @param enable (in)
*     @param voltage (in)
*	@return 
*
*******************************************************************************/
PMU_DRV_ErrorCode_en_t PMU_DRV_MaintchrgCtrl(Boolean enable, PMU_MBMCVS_t voltage);
#endif //#if !defined(PMU_MAX8986)

//*****************************************************************************
/**
*     @param current (in)
*	@return 
*
*******************************************************************************/
PMU_DRV_ErrorCode_en_t PMU_DRV_ChargerCtrlEOC( PMU_EOCS_t current );


PMU_DRV_ErrorCode_en_t PMU_DRV_EnablePmuInterrupt( void );
//*****************************************************************************
/**
*	PMU_DRV_DisablePmuInterrupt
*	@return 
*******************************************************************************/
PMU_DRV_ErrorCode_en_t PMU_DRV_DisablePmuInterrupt( void );

//*****************************************************************************
/**
*     @param pause (in)
*	@return 
*
*******************************************************************************/
PMU_DRV_ErrorCode_en_t PMU_DRV_PauseCharging( Boolean pause );

#else
//*******************************************************************************
/**
*PMU_DRV_TurnOnMicBias
*******************************************************************************/
void PMU_DRV_TurnOnMicBias( void );
//*******************************************************************************
/**
*PMU_DRV_TurnOffMicBias
*******************************************************************************/
void PMU_DRV_TurnOffMicBias( void );
//*******************************************************************************
/**
*PMU_DirectTurnOffMicBias
*******************************************************************************/
void PMU_DirectTurnOffMicBias( void );
#endif

//*****************************************************************************
/**
*	@param regId (in) 	
*	@param bits (in) 	
*	@param pmuSetCb (in) 	
*
*******************************************************************************/
void PMU_DRV_SetRegBits  ( PMU_REG_ID_t regId, UInt8 bits, PMU_WRITE_CB_t pmuSetCb );

//*****************************************************************************
/**
*	@param regId (in) 	
*	@param bits (in) 	
*	@param pmuClearCb (in) 	
*
*******************************************************************************/
void PMU_DRV_ClearRegBits( PMU_REG_ID_t regId, UInt8 bits, PMU_WRITE_CB_t pmuClearCb );
#else
//*******************************************************************************
/**
*PMU_TurnOnMicBias
*******************************************************************************/
void PMU_TurnOnMicBias( void );
//*******************************************************************************
/**
*PMU_TurnOffMicBias
*******************************************************************************/
void PMU_TurnOffMicBias( void );
//*******************************************************************************
/**
*PMU_DirectTurnOffMicBias
*******************************************************************************/
void PMU_DirectTurnOffMicBias( void );
void PMU_SetRegBits  ( PMU_REG_ID_t regId, UInt8 bits, PMU_WRITE_CB_t pmuSetCb );
//*****************************************************************************
/**
*	@param regId (in) 	
*	@param bits (in) 	
*	@param pmuClearCb (in) 	
*
*******************************************************************************/
void PMU_ClearRegBits( PMU_REG_ID_t regId, UInt8 bits, PMU_WRITE_CB_t pmuClearCb );
#endif
//*****************************************************************************
/**
*	@param Id (in) 	
*	@param pmuGetContentCb (in) 	
*
*******************************************************************************/
void PMU_GetContent( PMU_REG_ID_t Id, PMU_GetContent_Cb_t pmuGetContentCb );
//*******************************************************************************
/**
*PMU_DRV_SetVoltageInActiveMode
*******************************************************************************/
void PMU_DRV_SetVoltageInActiveMode( void );

//*****************************************************************************
/**
*	@param Id  (in) 	
*	@return 
*
*******************************************************************************/
UInt8 *PMU_GetMirroReg( PMU_REG_ID_t Id );

//*****************************************************************************
/**
*	@param Id  (in) 	
*	@return 
*
*******************************************************************************/
Boolean PMU_IsRegWritable( PMU_REG_ID_t Id );

//*****************************************************************************
/**
*	@param LedId (in) 	
*	@param Cycle (in) 	
*	@param Pattern (in) 	
*	@param Mode (in) 	
*
*******************************************************************************/
void PMU_SetLED( UInt8 LedId, UInt8 Cycle, UInt8 Pattern, UInt8 Mode );

//*****************************************************************************
/**
*	@param StatusOn (in) 	
*	@param level (in) 	
*	@param rampingtime (in) 	
*
*******************************************************************************/
void PMU_SetBacklight(
    Boolean     StatusOn,
    UInt8       level,
	UInt32		rampingtime
    );
//*******************************************************************************
/**
*PMU_Stop8SecondTimer
*******************************************************************************/
void PMU_Stop8SecondTimer( void );

//*******************************************************************************
/**
* PMU_DRV_GetOnKeyStatus
*@return
*******************************************************************************/
Boolean PMU_DRV_GetOnKeyStatus(void);
//*******************************************************************************
/**
*PMU_IsREC2LHigh
* @return
*******************************************************************************/
Boolean PMU_IsREC2LHigh( void );
//*******************************************************************************
/**
* PMU_IsREC2HHigh
*@return
*******************************************************************************/
Boolean PMU_IsREC2HHigh( void );

//*****************************************************************************
/**
*	@param rec2Status_Cb (in) 	
*
*******************************************************************************/
void PMU_RegisterREC2LStatusCB( PMU_REC2Status_Cb_t rec2Status_Cb );

//*****************************************************************************
/**
*	@param rec2Status_Cb (in) 	
*
*******************************************************************************/
void PMU_RegisterREC2HStatusCB( PMU_REC2Status_Cb_t rec2Status_Cb );

//*****************************************************************************
/**
*	@param StatusOn (in) 	
*	@param level (in) 	
*
*******************************************************************************/
void PMU_SetKeyLight(
    Boolean     StatusOn,
    UInt8       level
    );

#if defined ( PMU_PCF50611 ) || defined( PMU_PCF50612 ) || defined( PMU_BCM59035 )
//*****************************************************************************
/**
*	@param PwmId (in) 	
*	@param DutyCycle (in) 	
*	@param StatusOn (in) 	
*
*******************************************************************************/
void PMU_SetPWM(
    PMUPwmID_t  PwmId,
    UInt8       DutyCycle,//DutyCycle%
    Boolean     StatusOn
    );

//*****************************************************************************
/**
*	@param GPOpin (in) 	
*	@param OutputVal (in) 	
*	@return	
*
*******************************************************************************/
PMU_DRV_ErrorCode_en_t PMU_DRV_SetGPOutput( PMU_GPOpin_t GPOpin, PMU_GPOutput_t OutputVal );

//*****************************************************************************
/**
*	@param Ldotype (in) 	
*
*******************************************************************************/
void PMU_SetLDOtoECO( PMU_LDO_t Ldotype );
//*****************************************************************************
/**
*	@param Ldotype (in) 	
*
*******************************************************************************/
void PMU_SetLDOtoDefaultMode ( PMU_LDO_t Ldotype );

#endif
/// Added API to register callback from battmgr, so PMU driver can get info on upper layer callback addresses.
/// Now, PMU related codes are moved to PMU drivers. That's why we need this 
//*****************************************************************************
/**
*	@param eventCB (in) 	
*	@param callback (in) 	
*
*******************************************************************************/
void PMU_RegisterBattmgrCallbacks( PMU_DRV_RegisterCallbacks_en_t eventCB, void* callback );

/// These APIs are used by HAL and returns error code if specific PMU driver does not support it
//*****************************************************************************
/**
*	@param pmuCb (in) 	
*	@param directI2Ccall (in) 	
*     @return
*
*******************************************************************************/
PMU_DRV_ErrorCode_en_t PMU_DRV_IsChargerPresent ( PMU_IsChargerPresent_Cb_t pmuCb, PMU_I2CDirectCall_t directI2Ccall  );
//*****************************************************************************
/**
*PMU_DRV_Init_I2C_Err_Recover	
* @return	
*******************************************************************************/
PMU_DRV_ErrorCode_en_t PMU_DRV_Init_I2C_Err_Recover( void );


#if defined (PMU_BCM59055)
PMU_DRV_ErrorCode_en_t PMU_DRV_StartIbatReading(PMU_DRV_IBattReading_cb_t ibatCB, UInt32 interval);
PMU_DRV_ErrorCode_en_t PMU_DRV_StopIbatReading( void );
#endif

// Battmgr PMU related functions. Migrated and adopted from battmgr.
void PMU_StartCharging( void );	
//*******************************************************************************
/**
*PMU_StopCharging
*******************************************************************************/
void PMU_StopCharging( void );

#ifndef HAL_INCLUDED
/// If HAL not included, use this to register onkey events
//*****************************************************************************
/**
*	@param eventCB (in) 	
*
*******************************************************************************/
void PMU_DRV_Register_OnkeyEventCB( pmu_drv_OnkeyEvent_Cb_t eventCB );
#endif	//#ifndef HAL_INCLUDED

// New API for 611/612
#if defined ( PMU_PCF50611 ) || defined( PMU_PCF50612 ) || defined( PMU_BCM59035 )|| defined(PMU_BCM59038) || defined(PMU_BCM59055) || defined(PMU_MAX8986) 

#if !defined(PMU_BCM59055)
//*******************************************************************************
/**
*PMU_StopCharging
*******************************************************************************/
void PMU_USB_StartFastCharge( void );
//*******************************************************************************
/**
*PMU_USB_StartSlowCharge
*******************************************************************************/
void PMU_USB_StartSlowCharge( void );

/// Temporary for HAL glue
//*****************************************************************************
/**
*	@param pmuCb (in) 	
*	@param directI2Ccall (in) 	
*
*******************************************************************************/
void PMU_IsChargerPresent ( PMU_IsChargerPresent_Cb_t pmuCb, PMU_I2CDirectCall_t directI2Ccall  );
#else

//*****************************************************************************
/**
*	PMU_DRV_GetCurrentID 
*   @param usbId (in)		
*
*******************************************************************************/
void PMU_DRV_GetCurrentID(PMU_BC_OTG_Id_t* usbId);

//*****************************************************************************
/**
*	@param startChannel (in)
*   @param numChannels (in) 
*	@param adcData (in)	
*
*******************************************************************************/
void PMU_ADC_GetMultipleADCData (PMU_ADC_ChSel_t startChannel, UInt8 numChannels, UInt16 *adcData);
#endif

//*****************************************************************************
/**
*	@param rtmChannel (in) 	
*	@param adcData (in) 	
*
*******************************************************************************/
//void PMU_ADC_RTMChannelSelect ( PMU_ADC_ChSel_t RTMchannel );
void PMU_ADC_RTMChannelSelect(PMU_ADC_ChSel_t rtmChannel,UInt16 * adcData);


//*****************************************************************************
/**
*	@param RTMDelay (in) 	
*******************************************************************************/
void PMU_ADC_RTMSetDelay (PMU_RTM_Delay_t RTMDelay);

//*****************************************************************************
/**
*	 @param enable (in)	
* 
*******************************************************************************/
void PMU_ADC_GSMDebounce (UInt8 enable);

//*****************************************************************************
/**
*	@param channel (in) 
*	@param adcData (in)	
*
*******************************************************************************/
void PMU_ADC_GetADCData (PMU_ADC_ChSel_t channel, UInt16 *adcData);

//*****************************************************************************
/**
*	@param rtmChannel (in) 	
*	@param adcData (in)	
*
*******************************************************************************/
void PMU_ADC_RTMStartConversion (PMU_ADC_ChSel_t rtmChannel, UInt16 *adcData);

//*****************************************************************************
/**
*	@param enable (in) 	
*	@param pmuIHFBypassCb (in)	
*
*******************************************************************************/
void PMU_DRV_AUDIO_IHF_BypassEnable (Boolean enable, PMU_IHF_Bypass_Cb_t pmuIHFBypassCb);

//*****************************************************************************
/**
*	@param pmuIHFPowerCb (in)	
*
*******************************************************************************/
void PMU_DRV_AUDIO_IHF_PowerDown (PMU_IHF_Power_Cb_t pmuIHFPowerCb);

//*****************************************************************************
/**
*	@param pmuIHFPowerCb (in)	
*
*******************************************************************************/
void PMU_DRV_AUDIO_IHF_PowerUp (PMU_IHF_Power_Cb_t pmuIHFPowerCb);

//*****************************************************************************
/**
*	@param current (in) 	
*
*******************************************************************************/
void PMU_DRV_AUDIO_HS_ShortCircuitThreshold (PMU_HS_shortCircuit_curr_t current);

//*****************************************************************************
/**
*	@param pmuHSpath (in) 	
*	@param pmuHSgain (in) 	
*	@param pmuHSGainCb (in)	
*
*******************************************************************************/
void PMU_DRV_AUDIO_HS_SetGain (PMU_HS_path_t pmuHSpath, PMU_HS_Gain_t pmuHSgain, pmu_HeadsetGain_Cb_t pmuHSGainCb);

//*****************************************************************************
/**
*	@param pmuHSgain (in) 	
*	@param pmuHSInputmode (in) 	
*	@param pmuHSInputmodeCb (in)	
*
*******************************************************************************/
void PMU_DRV_AUDIO_HS_SetInputmode ( PMU_HS_Gain_t pmuHSgain, PMU_HS_Inputmode_t pmuHSInputmode, pmu_HeadsetInputmode_Cb_t pmuHSInputmodeCb);

//*****************************************************************************
/**
*	@param enable (in) 	
*
*******************************************************************************/
void PMU_DRV_AUDIO_HS_ShortCircuitEnable (Boolean enable);

#if defined(PMU_BCM59055)
//*****************************************************************************
/**
*	@param i2cMethod (in) 	
*	@param classAB (in)	
*
*******************************************************************************/
void PMU_DRV_AUDIO_HS_ClassSelMethod (Boolean i2cMethod, Boolean classAB);
#endif

//*****************************************************************************
/**
*	@param on (in) 	
*	@param pmuHSPowerCb (in)	
*
*******************************************************************************/
void PMU_DRV_AUDIO_HS_PowerUp (Boolean on, PMU_HS_Power_Cb_t pmuHSPowerCb);

//*****************************************************************************
/**
*	@param pmuIHFgain (in) 	
*	@param pmuIHFGainCb (in)	
*
*******************************************************************************/
void PMU_DRV_AUDIO_IHF_SetGain (PMU_IHF_Gain_t pmuIHFgain, pmu_IHFGain_Cb_t pmuIHFGainCb);

#if !defined (PMU_BCM59055)
//*****************************************************************************
/**
*	@param On (in)	
*
*******************************************************************************/
void PMU_DRV_ExternalBoost( Boolean On );
#endif

#if defined (PMU_MAX8986)

void PMU_DRV_USBChargerDetectionEnable (Boolean enable);
void PMU_DRV_USBDCDEnable(Boolean enable);
PMU_DRV_MUICType_en_t  PMU_DRV_MUICDetectionStatus(void);
void PMU_DRV_USBHSEnable(Boolean enable);
void PMU_DRV_ActivatePowerSupply( UInt8 regNum, UInt8 volNum );

#endif

#endif	// #if defined ( PMU_PCF50611 ) || defined( PMU_PCF50612 ) || defined( PMU_BCM59035 )|| defined(PMU_BCM59038)

#endif // _PMU_MEMMAP_INC_

/** @} */
