/******************************************************************************
Copyright 2009 Broadcom Corporation.  All rights reserved.

This program is the proprietary software of Broadcom Corporation and/or its 
licensors, and may only be used, duplicated, modified or distributed pursuant 
to the terms and conditions of a separate, written license agreement executed 
between you and Broadcom (an "Authorized License").

Except as set forth in an Authorized License, Broadcom grants no license
(express or implied), right to use, or waiver of any kind with respect to the 
Software, and Broadcom expressly reserves all rights in and to the Software and 
all intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, 
THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY 
NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
  
 Except as expressly set forth in the Authorized License,
1. This program, including its structure, sequence and organization, 
constitutes the valuable trade secrets of Broadcom, and you shall use all 
reasonable efforts to protect the confidentiality thereof, and to use this 
information only in connection with your use of Broadcom integrated circuit 
products.

2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
USE OR PERFORMANCE OF THE SOFTWARE.

3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE 
OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
ANY LIMITED REMEDY.
******************************************************************************/
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
} AUDIO_DRIVER_CONFIG_t;

typedef struct AUDIO_DRIVER_BUFFER_t
{
    UInt32    buf_size;
    UInt32    phy_addr;
    UInt8*    pBuf;
    
} AUDIO_DRIVER_BUFFER_t;

typedef void*  AUDIO_DRIVER_HANDLE_t;


typedef void (*AUDIO_DRIVER_InterruptPeriodCB_t)( AUDIO_DRIVER_HANDLE_t drv_handle);

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


#endif //#define __AUDIO_DDRIVER_H__
