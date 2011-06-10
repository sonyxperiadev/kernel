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
*   @file   i2c_drv.h
*
*   @brief  I2C device driver defines and prototypes.
*
****************************************************************************/
/**
*
* @defgroup BSCGroup Broadcom Serial Controller
* @ingroup CSLGroup
* @brief This group defines the APIs for BSC(Broadcom Serial Controller), which is I2C compatible.

Click here to navigate back to the Chip Support Library Overview page: \ref CSLOverview. \n
*****************************************************************************/
#ifndef _I2C_DRV_H_
#define _I2C_DRV_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(GPIO_MAX8986)

#define GPIO_I2C_SCL    GPIO7
#define GPIO_I2C_SDA    GPIO15
#endif

/** 
 * @addtogroup BSCGroup 
 * @{
 */


/**
*
*  I2C bus definitions
*
*****************************************************************************/
typedef enum {
    I2C_BUS1_ID = 0,		   ///< Select I2C bus 1 (Main I2C bus select)
    I2C_BUS2_ID = 1,		   ///< Select I2C bus 2 (Camera and TVOut I2C bus select)
#if defined (_HERA_) || defined(_RHEA_) || defined(_SAMOA_)
    I2C_BUS_PMU_ID = 2,
#endif    
#if defined(_ATHENA_) && (CHIP_REVISION >= 20)
    I2C_BUS3_ID = 2,
#endif
    NUM_I2C_BLOCK
} I2C_BUS_ID_t;


/**
*
*  Reserved device id that can be used for only changing the bus speed
*
*****************************************************************************/
typedef enum{
    I2C_BUS_SPEED_ID = 0xFFFF
} I2C_NO_DEVICE_REG_t;


/**
*
*  I2C read/write status
*
*****************************************************************************/
typedef enum {
    I2C_SUCCESS,                ///< Success return value
    I2C_ERROR_BUS_BUSY,         ///< I2C bus is busy, client need to retry later
    I2C_ERROR_IF,               ///< I2C interface error occurred, parameters should be checked
    I2C_ERROR_DEVICE_ACK,        ///< Addressed I2C device did not acknowledge
    I2C_ERROR_INVALID
} I2C_OPSTATUS_t;


/**
*
*  I2C_DATA_t struct for use with I2C read/write
*
*****************************************************************************/
typedef struct
{
    UInt32 len;                 ///< Number of bytes to be read or written
    UInt8  *dataPtr;            ///< Pointer to data to be written or buffer sufficient to hold 'len' bytes on read.
} I2C_DATA_t;


/**
*
*  I2C sub address ar 8 bit or 16 bit.
*
*****************************************************************************/
typedef enum {
    I2CSUBADDR_8BIT,            ///< use 8 bit sub-address
    I2CSUBADDR_16BIT,           ///< use 16 bit sub-address
    I2CSUBADDR_NONE             ///< sub-address not used
} I2CSUBADDR_t;


/**
*
*  I2C_ACCESS_t struct for I2C read/write command
*
*****************************************************************************/
typedef struct {
    I2C_BUS_ID_t    i2c_bus;    ///< I2C bus 1 or 2
    UInt16          deviceId;   ///< I2C device Id
    UInt16          subAddr;    ///< I2C device subaddress
    I2CSUBADDR_t    subAddrT;   ///< I2C subaddress size
    I2C_DATA_t      i2cData;    ///< On write the data and length to output.  On read completion contains
                                ///< the return data and length.
} I2C_ACCESS_t;


/**
*
*  I2C read/write callback function prototype
*
*****************************************************************************/
typedef void (*I2C_RW_CB_t)(
    I2C_OPSTATUS_t,             ///!< status
    I2C_DATA_t                  ///!< data
    );

/// I2C Write Callback
#define I2C_WRITE_CB_t I2C_RW_CB_t
/// I2C Read Callback
#define I2C_READ_CB_t  I2C_RW_CB_t


/**
*
*  I2C bus speeds
*
*****************************************************************************/
typedef enum{
    I2C_SPD_UNKNOWN = 0,       ///!< unspecified speed
    I2C_SPD_32K,               ///!< speed as 32K Hz
    I2C_SPD_50K,               ///!< speed as 50K Hz
    I2C_SPD_100K,              ///!< speed as 100K Hz
    I2C_SPD_230K,              ///!< speed as 230K Hz
    I2C_SPD_380K,              ///!< speed as 380K Hz
    I2C_SPD_400K,              ///!< speed as 400K Hz
    I2C_SPD_430K,              ///!< speed as 430K Hz
    I2C_SPD_HS,                ///!< high speed
    I2C_SPD_MAXIMUM            ///!< maximum speed 460K, assume auto sense turn off.
} I2C_SPEED_t;


/**
*
*  This function initialize the I2C driver for both controllers
*  @return		void
*
*****************************************************************************/
void I2CDRV_Init( void );


/**
*
*  This function checks if the driver is initialized or not
*  @return		TRUE if the driver is initialized
*
*****************************************************************************/
Boolean I2CDRV_IsInitialized( void );

/**
*
*  This function configure the I2C speed
*  @param		i2c_bus (in) which I2C bus to be configured.
*  @param		i2c_speed (in) the new speed to be configured.
*  @param		device_id (in) the client device id whose speed to be changed
*
*  @return		I2C_SPEED_t
*
*****************************************************************************/
I2C_SPEED_t I2CDRV_Config_Speed(
    I2C_BUS_ID_t  i2c_bus,
    I2C_SPEED_t   i2c_speed,
    UInt16        device_id
    );

/**
*
*  This function return the I2C speed for the bus or for a device
*  @param		i2c_bus (in) which I2C bus.
*  @param		device_id (in) the client device id whose speed to be returned
*
*  @return		I2C_SPEED_t
*
*****************************************************************************/
I2C_SPEED_t I2CDRV_Get_Speed(
    I2C_BUS_ID_t  i2c_bus,
    UInt16        device_id
    );


/**
*
*  This function receive the i2c data from client device to baseband.
*  @param		i2c_read_cb (in) callback. If NULL, then i2c data will bypass 
*               the software OS queue and access the bus directly
*  @param		i2cAccess (in) the i2c data structure	
*
*  @return		I2C_OPSTATUS_t
*
*****************************************************************************/
I2C_OPSTATUS_t I2CDRV_Read(
    I2C_READ_CB_t   i2c_read_cb,
    I2C_ACCESS_t    i2cAccess
    );

/**
*
*  This function transmit the i2c data from baseband to client device
*  @param		i2c_write_cb (in) callback. If NULL, then i2c data will bypass 
*               the software OS queue and access the bus directly
*  @param		i2cAccess (in) the i2c data structure	
*
*  @return		I2C_OPSTATUS_t
*
*****************************************************************************/
I2C_OPSTATUS_t I2CDRV_Write(
    I2C_WRITE_CB_t  i2c_write_cb,
    I2C_ACCESS_t    i2cAccess
    );

/**
*
*  This function turns on/off autosense to allow/disallow clock stretching
*  @param		i2c_bus (in) which I2C bus.
*  @param		autosense (in) TRUE to turn on	
*  @return		void
*
*****************************************************************************/
void I2CDRV_Set_Autosense(I2C_BUS_ID_t i2c_bus, Boolean autosense);

/**
*
*  This function returns autosense status
*  @param		i2c_bus (in) which I2C bus.
*  @return		TRUE if autosense is on
*
*****************************************************************************/
Boolean I2CDRV_Get_Autosense(I2C_BUS_ID_t i2c_bus);

/**
*
*  This function Activate I2C driver message processing after I2C_Deactivate() called.
*  @return		void
*
*****************************************************************************/
void I2CDRV_Activate( void );


/**
*
*  This function Deactivate I2C driver message processing until I2C_Activate() called.
*  @return		void
*
*****************************************************************************/
void I2CDRV_Deactivate( void );

/**
*
*  This function Get I2C bus activate status
*  @return		void
*
*****************************************************************************/
Boolean I2CDRV_Get_Activate_Status( void );


/**
*
*  This function to get I2C driver to participate power managemnet
*  @return		TRUE if dormant IDs are obtained
*
*****************************************************************************/
Boolean I2CDRV_Enable_PowerManagement(void);

/**
*
*  @brief  Return I2C driver version information.
*
*  @return driver version 
******************************************************************************/

UInt32 I2CDRV_Get_Version(void);
//gpio i2c driver

#if defined(GPIO_MAX8986)

void I2CGPIODRV_Init( void );

I2C_OPSTATUS_t I2CGPIODRV_Write( I2C_WRITE_CB_t i2c_write_cb, I2C_ACCESS_t i2cAccess );

I2C_OPSTATUS_t I2CGPIODRV_Read( I2C_READ_CB_t i2c_read_cb, I2C_ACCESS_t i2cAccess );

Boolean I2CGPIODRV_Enable_PowerManagement(void);

void I2CGPIODRV_DeInit( void );
#endif

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* _I2C_DRV_H_ */
