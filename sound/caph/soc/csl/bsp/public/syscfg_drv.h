/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

/**
*
*   @file   syscfg_drv.h
*   @brief  System Config driver 
*
****************************************************************************/
/**
*
* @defgroup CSLSystemConfigGroup System Configuration Group
* @brief This group defines the APIs for System Configuration
*
* @ingroup CSLGroup

Click here to navigate back to the Chip Support Library Overview page: \ref CSLOverview.
*****************************************************************************/

#ifndef __SYSCFG_DRV_H
#define __SYSCFG_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

#define SYSCFG_DRV_HAS_AUDIO_API

/******************************************************************************
Global types
*****************************************************************************/
//Interface mode
typedef enum {
    SYSCFG_PERIPH_AHB_CLK_VIDEO_CODEC=0,
    SYSCFG_PERIPH_AHB_CLK_CAMARA_IF,
    SYSCFG_PERIPH_AHB_CLK_USB,
    SYSCFG_PERIPH_AHB_CLK_GEA,
    SYSCFG_PERIPH_AHB_CLK_CRYPTO,
    SYSCFG_PERIPH_AHB_CLK_PKA,
    SYSCFG_PERIPH_AHB_CLK_UARTA,
    SYSCFG_PERIPH_AHB_CLK_UARTB,
    SYSCFG_PERIPH_AHB_CLK_DA,
    SYSCFG_PERIPH_AHB_CLK_MPCLK,
    SYSCFG_PERIPH_AHB_CLK_LCD,
    SYSCFG_PERIPH_AHB_CLK_DPE,
    SYSCFG_PERIPH_AHB_CLK_DSI,
    SYSCFG_PERIPH_AHB_CLK_DMAC,
    SYSCFG_PERIPH_AHB_CLK_SDIO1,
    SYSCFG_PERIPH_AHB_CLK_SDIO2,
    SYSCFG_PERIPH_AHB_CLK_DES,
    SYSCFG_PERIPH_AHB_CLK_UARTC,
    SYSCFG_PERIPH_AHB_CLK_RNG,
    SYSCFG_PERIPH_AHB_CLK_SDIO3,
    SYSCFG_PERIPH_AHB_CLK_FSUSBHOST,
    SYSCFG_PERIPH_AHB_CLK_MPHI,
    SYSCFG_PERIPH_AHB_CLK_DMAC_MODE,
    SYSCFG_PERIPH_AHB_CLK_HUCM_FW,
    SYSCFG_PERIPH_AHB_CLK_HTMH,
    SYSCFG_PERIPH_AHB_CLK_HTML,
    SYSCFG_PERIPH_AHB_CLK_HTM,
    SYSCFG_PERIPH_AHB_CLK_CIPHER_FW,
    SYSCFG_PERIPH_AHB_CLK_ITEM_NOT_VALID,
    SYSCFG_PERIPH_AHB_CLK_ITEM_COUNT,
}SYSCFG_PERIPH_AHB_CLK_ITEM_T;


typedef enum
{
    SYSCFG_PERIPH_AHB_CLK_OFF=0,
    SYSCFG_PERIPH_AHB_CLK_ON,
}SYSCFG_PERIPH_AHB_CLK_VALUE_T;
 
typedef struct _tagCSL_SysCfgAhbClkItem
{
    SYSCFG_PERIPH_AHB_CLK_ITEM_T item;
    SYSCFG_PERIPH_AHB_CLK_VALUE_T value;
}SYSCFG_PERIPH_AHB_CLK_DESCRIPTOR_T;


typedef enum
{
    SYSCFG_STATUS_OK=0,
    SYSCFG_STATUS_ERROR,
    SYSCFG_STATUS_INVALID_ARGUMENT,
    SYSCFG_STATUS_INVALID_AHB_CLK_ITEM,
    SYSCFG_STATUS_UNKNOWN_ERROR,
}SYSCFG_STATUS_T;

//
/// Pin mux interface
typedef enum 
{
    SYSCFG_CAMCK_GPIO_MUX_GPIO_SEL=0,
    SYSCFG_CAMCK_GPIO_MUX_CAMCK_SEL,

    // SYSCFG_FLASH_SD2_MUX
    SYSCFG_FLASH_SD2_MUX_FLASH_SEL,
    SYSCFG_FLASH_SD2_MUX_SD2_SEL,

    // SYSCFG_LCD_GPIO_MPHI_MUX
    SYSCFG_LCD_GPIO_MPHI_MUX_LCD_SEL,
    SYSCFG_LCD_GPIO_MPHI_MUX_GPIO_SEL,
    SYSCFG_LCD_GPIO_MPHI_MUX_MPHI_SEL,
    
    //SYSCFG_LCD_CAM_MPHI_MUX
    SYSCFG_LCD_CAM_MPHI_MUX_LCDD_SEL,
    SYSCFG_LCD_CAM_MPHI_MUX_GPIO_CAM_SEL,
    SYSCFG_LCD_CAM_MPHI_MUX_MPHI_SEL,

    //SYSCFG_GPEN9_GPIO54_MUX
    SYSCFG_GPEN9_GPIO54_MUX_GPEN9_SEL,
    SYSCFG_GPEN9_GPIO54_MUX_X3_SEL,
    SYSCFG_GPEN9_GPIO54_MUX_SPI_SEL,
    SYSCFG_GPEN9_GPIO54_MUX_GPIO54_SEL,

    //SYSCFG_DSP_LCD_GPIO_MUX
    SYSCFG_DSP_LCD_GPIO_MUX_NORMAL_SEL,
    SYSCFG_DSP_LCD_GPIO_MUX_DSP_SEL,
    SYSCFG_DSP_LCD_GPIO_MUX_WCDMA_SEL,

    //SYSCFG_DIGMIC_GPIO_MUX
    SYSCFG_DIGMIC_GPIO_MUX_DIGMIC_SEL,
    SYSCFG_DIGMIC_GPIO_MUX_GPIO_SEL,

    //SYSCFG_PCM_SPI2_GPIO_MUX
    SYSCFG_PCM_SPI2_GPIO_MUX_PCM_SEL,
    SYSCFG_PCM_SPI2_GPIO_MUX_SPI2_SEL,
    SYSCFG_PCM_SPI2_GPIO_MUX_GPIO_SEL,

    //SYSCFG_SPI_UARTC_GPIO_MUX
    SYSCFG_SPI_UARTC_GPIO_MUX_SPI_SEL,
    SYSCFG_SPI_UARTC_GPIO_MUX_UARTC_SEL,
    SYSCFG_SPI_UARTC_GPIO_MUX_GPIO_SEL,

    //SYSCFG_GPEN11_GPIO_MUX
    SYSCFG_GPEN11_GPIO_MUX_GPEN11_SEL,
    SYSCFG_GPEN11_GPIO_MUX_GSM_SEL,
    SYSCFG_GPEN11_GPIO_MUX_SPI_SEL,
    SYSCFG_GPEN11_GPIO_MUX_GPIO56_SEL,

    // SYSCFG_LCDD16_LCDD17_MUX
    SYSCFG_LCDD16_LCDD17_MUX_LCD_SEL,
    SYSCFG_LCDD16_LCDD17_MUX_GPIO63_LCD_SEL,
    SYSCFG_LCDD16_LCDD17_MUX_MPHI_SEL,

    // SYSCFG_AFCPDM_MUX
    SYSCFG_AFCPDM_MUX_AFCPDM_SEL,
    SYSCFG_AFCPDM_MUX_CLK_MONITOR_SEL,

    // SYSCFG_GPEN10_GPIO_MUX
    SYSCFG_GPEN10_GPIO_MUX_GPEN10_SEL,
    SYSCFG_GPEN10_GPIO_MUX_XCSSEL_SEL,
    SYSCFG_GPEN10_GPIO_MUX_SPI_SEL,
    SYSCFG_GPEN10_GPIO_MUX_GPIO55_SEL,

    //SYSCFG_GPEN8_GPIO53_MUX
    SYSCFG_GPEN8_GPIO53_MUX_GPEN8_SEL,
    SYSCFG_GPEN8_GPIO53_MUX_C3_SEL,
    SYSCFG_GPEN8_GPIO53_MUX_SPI_SEL,
    SYSCFG_GPEN8_GPIO53_MUX_GPIO53_SEL,

    //SYSCFG_GPEN7_GPIO52_MUX
    SYSCFG_GPEN7_GPIO52_MUX_GPEN7_SEL,
    SYSCFG_GPEN7_GPIO52_MUX_GPIO52_SEL,

    // SYSCFG_GPIO16_PWM0_MUX
    SYSCFG_GPIO16_PWM0_MUX_GPIO16_SEL,
    SYSCFG_GPIO16_PWM0_MUX_PWM0_SEL,
    
    //SYSCFG_GPIO17_PWM1_MUX
    SYSCFG_GPIO17_PWM1_MUX_GPIO16_SEL,
    SYSCFG_GPIO17_PWM1_MUX_PWM1_SEL,

    //SYSCFG_GPIO_MUX
    SYSCFG_GPIO_MUX_GPIO29_SEL,
    SYSCFG_GPIO_MUX_LEGACY_USB_OTG_SEL,
    SYSCFG_GPIO_MUX_GPEN13_SEL,

    // SYSCFG_I2S_GPIO_MUX
    SYSCFG_I2S_GPIO_MUX_I2S_SEL,
    SYSCFG_I2S_GPIO_MUX_I2S1_SEL,
    SYSCFG_I2S_GPIO_MUX_I2S2_SEL,
    SYSCFG_I2S_GPIO_MUX_GPIO60_SEL,

    //SYSCFG_SD1_MUX
    SYSCFG_SD1_MUX_SD1_SEL,
    SYSCFG_SD1_MUX_SPI2_SEL,
    SYSCFG_SD1_MUX_2G_TWIF_SEL,
    SYSCFG_SD1_MUX_GPIO52_SEL,

    //SYSCFG_M68_LCD_MUX
    SYSCFG_M68_LCD_MUX_M68_SEL,
    SYSCFG_M68_LCD_MUX_LCD_SEL,

    // SYSCFG_GPIO_MSPRO_SD3_MUX
    SYSCFG_GPIO_MSPRO_SD3_MUX_SD3_SEL,
    SYSCFG_GPIO_MSPRO_SD3_MUX_MSPRO_SEL,
    SYSCFG_GPIO_MSPRO_SD3_MUX_PM_MONITOR_SEL,
    SYSCFG_GPIO_MSPRO_SD3_MUX_GPIO_SEL,

    // Keypad and GPIO Mux
    SYSCFG_KEY_ROW_GPIO_MUX_SEL,
    SYSCFG_KEY_COL_GPIO_MUX_SEL,

    // ANA_SYSCLKEN_MUX
    SYSCFG_ANA_SYSCLKEN_GPIO42_MUX_ANA_SYSCLKEN_SEL,
    SYSCFG_ANA_SYSCLKEN_GPIO42_MUX_GPIO42_SEL,
    
    // SOFTRSTO_MUX
    SYSCFG_SOFTRSTO_MUX_SOFTRSTO_SEL,
    SYSCFG_SOFTRSTO_MUX_CE4N_SEL,
    SYSCFG_SOFTRSTO_MUX_GPIO40_SEL,

    // OTGCTRL1_MUX
    SYSCFG_OTGCTRL1_MUX_OTGCTL1_SEL,
    SYSCFG_OTGCTRL1_MUX_CE5N_SEL,
    SYSCFG_OTGCTRL1_MUX_GPEN15_SEL,
    SYSCFG_OTGCTRL1_MUX_GPIO41_SEL,

    // TRACE BUS Port
    SYSCFG_TRACEBUS_PORT_MUX_AP_ETM_SEL,
    SYSCFG_TRACEBUS_PORT_MUX_CP_ETM_SEL,
    SYSCFG_TRACEBUS_PORT_MUX_HTM_SEL,
    SYSCFG_TRACEBUS_PORT_MUX_DSP_SEL,
    SYSCFG_TRACEBUS_PORT_MUX_CAM_SEL,

    // SIM2_GPIO_MUX
    SYSCFG_SIM2_GPIO_MUX_SIM2_SEL,
    SYSCFG_SIM2_GPIO_MUX_GPIO_SEL,

    //
    // GPIO27_MUX
    SYSCFG_GPIO27_MUX_GPIO27_SEL,
    SYSCFG_GPIO27_MUX_PWM5_SEL,
    SYSCFG_GPIO27_MUX_GPCK_SEL,
    SYSCFG_GPIO27_MUX_PM_MONITOR_SEL,
    
    //
    // GPIO26_MUX
    SYSCFG_GPIO26_MUX_GPIO26_SEL,
    SYSCFG_GPIO26_MUX_PWM4_SEL,
    SYSCFG_GPIO26_MUX_PDPCK_SEL,
    SYSCFG_GPIO26_MUX_PM_MONITOR_SEL,

    // GPIO35 MUX
    SYSCFG_GPIO35_MUX_GPIO35_SEL,
    SYSCFG_GPIO35_MUX_ADCSYNC_SEL,
    SYSCFG_GPIO35_MUX_UARTA_OUT2N_SEL,

    // GPIO34 MUX
    SYSCFG_GPIO34_MUX_GPIO34_SEL,
    SYSCFG_GPIO34_MUX_MPHI_HAT1_SEL,
    SYSCFG_GPIO34_MUX_UARTA_OUT1N_SEL,
    SYSCFG_GPIO34_MUX_SRAM_PCRE_SEL,

    // GPIO33 MUX
    SYSCFG_GPIO33_MUX_GPIO33_SEL,
    SYSCFG_GPIO33_MUX_WCDMA_UART_TXD_SEL,
    SYSCFG_GPIO33_MUX_UARTA_DTR_SEL,
    SYSCFG_GPIO33_MUX_SRAM_PUBN_SEL,

    // GPIO32 MUX
    SYSCFG_GPIO32_MUX_GPIO32_SEL,
    SYSCFG_GPIO32_MUX_WCDMA_UART_RXD_SEL,
    SYSCFG_GPIO32_MUX_UARTA_DSR_SEL,
    SYSCFG_GPIO32_MUX_SRAM_PLBN_SEL,

    // GPIO31 MUX
    SYSCFG_GPIO31_MUX_GPIO31_SEL,
    SYSCFG_GPIO31_MUX_USB_OTGCTRL5_SEL,
    SYSCFG_GPIO31_MUX_GPEN15_SEL,
    SYSCFG_GPIO31_MUX_PEDESTAL_SEL,

    // GPIO30 MUX
    SYSCFG_GPIO30_MUX_GPIO30_SEL,
    SYSCFG_GPIO30_MUX_USB_VBUSSTAT2_SEL,
    SYSCFG_GPIO30_MUX_GPEN14_SEL,
    SYSCFG_GPIO30_MUX_SIM2_PRESENCE_DETECT_SEL,
   
    // GPIO25 MUX
    SYSCFG_GPIO25_MUX_GPIO25_SEL,
    SYSCFG_GPIO25_MUX_LCDTE_SEL,

    // GPIO48 MUX
    SYSCFG_GPIO48_MUX_GPIO48_SEL,
    SYSCFG_GPIO48_MUX_RFGPIO2_SEL,
 
     // GPIO49 MUX
    SYSCFG_GPIO49_MUX_GPIO49_SEL,
    SYSCFG_GPIO49_MUX_RFGPIO3_SEL,
     
      // GPIO50 MUX
    SYSCFG_GPIO50_MUX_GPIO50_SEL,
    SYSCFG_GPIO50_MUX_RFGPIO4_SEL,
    
      // GPIO51 MUX
    SYSCFG_GPIO51_MUX_GPIO51_SEL,
    SYSCFG_GPIO51_MUX_RFGPIO5_SEL,
    
    // GPIO38_TO_39 MUX
    SYSCFG_GPIO38_TO_39_MUX_GPIO38_TO_39_SEL,
    SYSCFG_GPIO38_TO_39_MUX_BSC3_SEL,
    
    // GPIO7_GPIO15 MUX
    SYSCFG_GPIO7_GPIO15_MUX_GPIO7_GPIO15_SEL,
    SYSCFG_GPIO7_GPIO15_MUX_BSC3_SEL,
    
}SYSCFG_PIN_MUX_SEL;


typedef enum 
{
    SYSCFG_KEY_GPIO_MUX_GPIO_SEL=0,
    SYSCFG_KEY_GPIO_MUX_PIN0_SEL=(1<<0),
    SYSCFG_KEY_GPIO_MUX_PIN1_SEL=(1<<1),
    SYSCFG_KEY_GPIO_MUX_PIN2_SEL=(1<<2),
    SYSCFG_KEY_GPIO_MUX_PIN3_SEL=(1<<3),
    SYSCFG_KEY_GPIO_MUX_PIN4_SEL=(1<<4),
    SYSCFG_KEY_GPIO_MUX_PIN5_SEL=(1<<5),
    SYSCFG_KEY_GPIO_MUX_PIN6_SEL=(1<<6),
    SYSCFG_KEY_GPIO_MUX_PIN7_SEL=(1<<7),
}SYSCFG_KEY_GPIO_MUX;

//
// IO configuration
typedef enum
{
    SYSCFG_IO_CR0,
    SYSCFG_IO_CR1,
    SYSCFG_IO_CR2,
    SYSCFG_IO_CR3,
    SYSCFG_IO_CR4,
    SYSCFG_IO_CR5,
    SYSCFG_IO_CR6,
    SYSCFG_IO_CR7,
    SYSCFG_IO_CR10,
} SYSCFG_IO_REG;

typedef enum 
{
    SYSCFG_SD_DRIVE_2MA = 1,
    SYSCFG_SD_DRIVE_4MA,
    SYSCFG_SD_DRIVE_6MA = 4,
    SYSCFG_SD_DRIVE_8MA,
    SYSCFG_SD_DRIVE_10MA,
	SYSCFG_SD_DRIVE_12MA
}SYSCFG_SD_DRIVE;

typedef enum 
{
    SYSCFG_LCD_DRIVE_2MA = 1,
    SYSCFG_LCD_DRIVE_4MA =2,
    SYSCFG_LCD_DRIVE_6MA = 3,
    SYSCFG_LCD_DRIVE_8MA =6,
    SYSCFG_LCD_DRIVE_10MA = 7,
}SYSCFG_LCD_DRIVE;

typedef enum 
{
    SYSCFG_SD_NO_PULL = 0,
    SYSCFG_SD_PULL_UP,
    SYSCFG_SD_PULL_DOWN,
    SYSCFG_SD_PULL_ERROR
}SYSCFG_SD_PULL_CTRL;

#if (defined(_ATHENA_) && (CHIP_REVISION >= 20))
typedef enum 
{
    SYSCFG_BSC_NO_PULL_UP = 0,
    SYSCFG_BSC_2360_PULL_UP,
    SYSCFG_BSC_1720_PULL_UP,
    SYSCFG_BSC_995_PULL_UP
}SYSCFG_BSC_PULL_CTRL;
#endif

/** 
 * @addtogroup CSLSystemConfigGroup 
 * @{
 */


/**
*
*  This function initialize the system configuration. All the peripheral
*  AHB clocks are disabled.
*
*******************************************************************************/
void SYSCFGDRV_Init(void);


/**
*
*  This function De-initialize the system configuration.
*
*******************************************************************************/
void SYSCFGDRV_Deinit(void);


/**
*
*  This function turn on/off the peripheral AHB clock
*
*  @param   item  Peripheral AHB Clock Item to set  
*  @param   v     Value that the item is set (either ON or OFF)
*
*  @return  SYSCFG_STATUS_OK if no error, or error status
*
*******************************************************************************/
SYSCFG_STATUS_T SYSCFGDRV_Set_Periph_AHB_Clk_Item
(
    const SYSCFG_PERIPH_AHB_CLK_ITEM_T item,
    const SYSCFG_PERIPH_AHB_CLK_VALUE_T v
);

/**
*
*  This function turn on/off the array of the peripherals AHB clock. Use This 
*  function to set the multiple peripherals 
*
*  @param   p  array of Peripherals AHB Clock Items to set  
*
*  @return  SYSCFG_STATUS_OK if no error, or error status
*
*******************************************************************************/
SYSCFG_STATUS_T SYSCFGDRV_Set_Periph_AHB_Clk_Items
(
    const SYSCFG_PERIPH_AHB_CLK_DESCRIPTOR_T* p
);


/**
*
*  This function config the pin mux of the peripheral
*
*  @param   pinMuxSel   identify the pin mux to config
*
*  @return  SYSCFG_STATUS_OK if no error, or error status
*
*******************************************************************************/
SYSCFG_STATUS_T SYSCFGDRV_Config_Pin_Mux
(
    const SYSCFG_PIN_MUX_SEL pinMuxSel
);


/**
*
*  This function config the keypad and GPIO pin mux
*
*  @param   pinMuxSel   identify the pin mux to config
*  @param   value       selected keypad pin to enable
*
*  @return  SYSCFG_STATUS_OK if no error, or error status
*
*******************************************************************************/
SYSCFG_STATUS_T SYSCFGDRV_Config_Keypad_Pin_Mux
(
    const SYSCFG_PIN_MUX_SEL pinMuxSel,
     const UInt32 value
 );

/**
*
*  This function read the value from system configuration register
*
*  @param   io                io configuration register identification
*  @param   pValueOut         pointer to data 
*
*  @return  SYSCFG_STATUS_OK if no error, or error status
*
*******************************************************************************/
SYSCFG_STATUS_T SYSCFGDRV_IoRead
(
    SYSCFG_IO_REG io,
    UInt32 *pValueOut
 );

/**
*
*  This function write the value to system configuration register
*
*  @param   id                io configuration register identification
*  @param   value             data to be written
*
*  @return  SYSCFG_STATUS_OK if no error, or error status
*
*******************************************************************************/
SYSCFG_STATUS_T SYSCFGDRV_IoWrite
(
    SYSCFG_IO_REG id,
   const UInt32 value
 );

#if !defined(_ATHENA_)

/**
*
*  This function get the identifier for the chip
*
*  @param   ProductFamilyID   pointer to address to store family ID
*  @param   ProductID            pointer to address to store product ID
*  @param   Revision              pointer to address to store revision ID
*  @param   value       selected keypad pin to enable
*
*  @return  void
*
*******************************************************************************/
void SYSCFGDRV_Get_Chipid(UInt32 * ProductFamilyID, UInt32 * ProductID, UInt32 * Revision);

/**
*
*  This function sets the reason for a software-based system reset
*
*  @param   reason    reason for reset
*
*  @return  void
*
*******************************************************************************/
void SYSCFGDRV_Set_Softrstreason(UInt32 reason);

/**
*
*  This function returns the reason for a software-based reset
*
*
*  @return  System reset reason
*
*******************************************************************************/
UInt32 SYSCFGDRV_Get_Softrstreason(void);

#endif

/**
*
*  This function config the SD drive strength.
*
*  @param   sdSel     identify the SD interface mux to config
*  @param   drive	  drive strength to be set.
*  @param	clock	  clock strength to be set.	
*
*  @return  SYSCFG_STATUS_OK if no error, or error status
*
*******************************************************************************/

SYSCFG_STATUS_T SYSCFGDRV_Set_SDIO_Drive_Strength
(   
    const SYSCFG_PIN_MUX_SEL  sdSel, 
	const SYSCFG_SD_DRIVE     drive, 
	const SYSCFG_SD_DRIVE     clock
);

/**
*
*  This function returns the SD drive strength.
*
*  @param   sdSel     identify the SD interface mux to config
*  @param   value     drive strength value.
*
*  @return  SYSCFG_STATUS_OK if no error, or error status
*
*******************************************************************************/

SYSCFG_STATUS_T SYSCFGDRV_Get_SDIO_Drive_Strength
(
    const SYSCFG_PIN_MUX_SEL sdSel, 
	UInt32 *value
);

/**
*
*  This function configs the SD pull on CMD/DAT/CLOCK lines.
*
*  @param   sdSel     identify the SD interface mux to config
*  @param   cmd       CMD line pull setting.
*  @param   data      DAT line pull setting.
*  @param   data2     CLK line pull setting.
*
*  @return  SYSCFG_STATUS_OK if no error, or error status
*
*******************************************************************************/

SYSCFG_STATUS_T SYSCFGDRV_Set_SDIO_Pull_Status
(
    const SYSCFG_PIN_MUX_SEL   sdSel, 
	const SYSCFG_SD_PULL_CTRL  cmd, 
	const SYSCFG_SD_PULL_CTRL  data, 
    const SYSCFG_SD_PULL_CTRL  data2
);

/**
*
*  This function returns the SD pull setting.
*
*  @param   sdSel     identify the SD interface mux to config
*  @param   value     the pull setting in following format.
*                     CMD setting bit[3:0]
*                     DAT setting bit[7:4]
*                     CLK setting bit[11:8]
*                     0 : no pull
*                     1 : pull up
*                     2 : pull down
*
*  @return  SYSCFG_STATUS_OK if no error, or error status
*
*******************************************************************************/

SYSCFG_STATUS_T SYSCFGDRV_Get_SDIO_Pull_Status
(
    const SYSCFG_PIN_MUX_SEL sdSel, 
	UInt32 *value
);

/**
*
*  This function config the LCD drive strength.
*
*  @param   drive	  drive strength to be set.
*
*  @return  SYSCFG_STATUS_OK if no error, or error status
*
*******************************************************************************/

SYSCFG_STATUS_T SYSCFGDRV_Set_LCD_Drive_Strength( 	const SYSCFG_LCD_DRIVE     drive );

#if (defined(_ATHENA_) && (CHIP_REVISION >= 20))
/**
*
*  This function config the BSC1&BSC2 intrnal pull_up value on athenaB0.
*
*  @param   bsc_bus_id:	  0-BSC1,  1-BSC2, no support for others.
*           pull_up_value: pull_up const value
*           ext_5v_pull_up: set TRUE if external pull-up to 5V, otherwise, set FALSE  
*
*  @return  SYSCFG_STATUS_OK if no error, or error status
*
*******************************************************************************/

SYSCFG_STATUS_T SYSCFGDRV_Set_BSC_PULL_UP( UInt8 bsc_bus_id, const SYSCFG_BSC_PULL_CTRL  pull_up_value, Boolean ext_5v_pull_up );
#endif
/** @} */

#endif // __SYSCFG_H
