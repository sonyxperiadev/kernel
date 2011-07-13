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
* @file   audio_ddriver.h
* @brief  Audio data driver interface
*
*****************************************************************************/
#ifndef __AUDIO_DDRIVER_H__
#define __AUDIO_DDRIVER_H__

/**
 * @addtogroup Audio_ddriver 
 * @{
 */

typedef enum AUDIO_DRIVER_TYPE_t
{
	AUDIO_DRIVER_PLAY_VOICE = 0,
    AUDIO_DRIVER_PLAY_AUDIO,
    AUDIO_DRIVER_PLAY_RINGER,
    AUDIO_DRIVER_PLAY_BT,
    AUDIO_DRIVER_CAPT_VOICE,
    AUDIO_DRIVER_CAPT_HQ,
    AUDIO_DRIVER_CAPT_FM,
    AUDIO_DRIVER_CAPT_BT
} AUDIO_DRIVER_TYPE_t;

typedef enum AUDIO_DRIVER_CTRL_t
{
	AUDIO_DRIVER_START = 0,   
    AUDIO_DRIVER_STOP,           
    AUDIO_DRIVER_PAUSE,        
    AUDIO_DRIVER_RESUME,    
    AUDIO_DRIVER_FLUSH,       
    AUDIO_DRIVER_CONFIG,    
    AUDIO_DRIVER_SET_CB,     
    AUDIO_DRIVER_SET_VOICE_CAPT_TYPE,  
    AUDIO_DRIVER_SET_INT_PERIOD,              
    AUDIO_DRIVER_SET_BUF_PARAMS,
    AUDIO_DRIVER_GET_DRV_TYPE,
    AUDIO_DRIVER_ALLOC_BUFFER,                 
    AUDIO_DRIVER_FREE_BUFFER                    
} AUDIO_DRIVER_CTRL_t;

typedef enum AUDIO_DRIVER_VOICE_CAPT_TYPE_t
{
	AUDIO_DRIVER_VOICE_CAPT_UL= 0,
    AUDIO_DRIVER_VOICE_CAPT_DL,
    AUDIO_DRIVER_VOICE_CAPT_BOTH,
}AUDIO_DRIVER_VOICE_CAPT_TYPE_t;

typedef struct AUDIO_DRIVER_CONFIG_t
{
	AUDIO_SAMPLING_RATE_t       sample_rate;
	AUDIO_CHANNEL_NUM_t         num_channel;
	AUDIO_BITS_PER_SAMPLE_t     bits_per_sample;
	UInt32 						instanceId; //ARM2SP1/ARM2SP2 mapped to PCMOUT1/PCMOUT2
} AUDIO_DRIVER_CONFIG_t;

typedef struct AUDIO_DRIVER_BUFFER_t
{
    UInt32    buf_size;
    UInt32    phy_addr;
    UInt8*    pBuf;
    
} AUDIO_DRIVER_BUFFER_t;

typedef void*  AUDIO_DRIVER_HANDLE_t;


typedef void (*AUDIO_DRIVER_InterruptPeriodCB_t)( void * p);

typedef struct AUDIO_DRIVER_CallBackParams_t
{
	AUDIO_DRIVER_InterruptPeriodCB_t	pfCallBack;
	void	*pPrivateData;

}AUDIO_DRIVER_CallBackParams_t;



/**
*  @brief  This function is used to open the audio data driver
*
*  @param drv_type   (in)    driver type
*
*  @return AUDIO_DRIVER_HANDLE_t
*
****************************************************************************/
AUDIO_DRIVER_HANDLE_t  AUDIO_DRIVER_Open(AUDIO_DRIVER_TYPE_t drv_type);

/**
*  @brief  This function is used to close the audio data driver
*
*  @param  drv_handle   (in)  handle returned while opening the driver
*
*  @return none
*
****************************************************************************/
void AUDIO_DRIVER_Close(AUDIO_DRIVER_HANDLE_t drv_handle);

/**
*  @brief  This function is used to read the data from the driver
*
*  @param  drv_handle   (in)  handle returned while opening the driver
*  @param  pBuf         (in)  Pointer to the buffer to be read
*  @param  nSize        (in)  size of the buffer
*
*  @return none
*
****************************************************************************/
void AUDIO_DRIVER_Read(AUDIO_DRIVER_HANDLE_t drv_handle,
                  UInt8* pBuf,
                  UInt32 nSize);

/**
*  @brief  This function is used to write the data to the driver
*
*  @param  drv_handle   (in)  handle returned while opening the driver
*  @param  pBuf         (in)  Pointer to the buffer to be read
*  @param  nSize        (in)  size of the buffer
*
*  @return none
*
****************************************************************************/
void AUDIO_DRIVER_Write(AUDIO_DRIVER_HANDLE_t drv_handle,
                   UInt8* pBuf,
                   UInt32 nBufSize);

/**
*  @brief  This function is used to send a control command to the driver
*
*  @param  drv_handle   (in)  handle returned while opening the driver
*  @param  ctrl_cmd     (in)  Command id
*  @param  pCtrlStruct  (in)  command data structure
*
*  @return none
*
****************************************************************************/
void AUDIO_DRIVER_Ctrl(AUDIO_DRIVER_HANDLE_t drv_handle,
                       AUDIO_DRIVER_CTRL_t ctrl_cmd,
                       void* pCtrlStruct);


/**
*  @brief  This function is used to update the buffer indexes
*
*  @param  drv_handle       (in)  handle returned while opening the driver
*  @param  pBuf             (in)  Pointer to the buffer to be updated
*  @param  nBufSize         (in)  size of the buffer
*  @param  nCurrentIndex    (in)  current index
*  @param  nSize            (in)  size from the current index
*
*  @return none
*
****************************************************************************/
void AUDIO_DRIVER_UpdateBuffer (AUDIO_DRIVER_HANDLE_t drv_handle,
                                UInt8* pBuf,
                                UInt32 nBufSize,
                                UInt32 nCurrentIndex,
                                UInt32 nSize);


UInt32 StreamIdOfDriver(AUDIO_DRIVER_HANDLE_t h);

#endif //#define __AUDIO_DDRIVER_H__
