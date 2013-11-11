/****************************************************************************
Copyright 2009 - 2011  Broadcom Corporation
 Unless you and Broadcom execute a separate written software license agreement
 governing use of this software, this software is licensed to you under the
 terms of the GNU General Public License version 2 (the GPL), available at
    http://www.broadcom.com/licenses/GPLv2.php

 with the following added to such license:
 As a special exception, the copyright holders of this software give you
 permission to link this software with independent modules, and to copy and
 distribute the resulting executable under terms of your choice, provided
 that you also meet, for each linked independent module, the terms and
 conditions of the license of that module.
 An independent module is a module which is not derived from this software.
 The special exception does not apply to any modifications of the software.
 Notwithstanding the above, under no circumstances may you combine this software
 in any way with any other Broadcom software provided under a license other than
 the GPL, without Broadcom's express prior written consent.
***************************************************************************/
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

#define VOIP_MAX_FRAME_LEN		642

enum __AUDIO_DRIVER_TYPE_t {
	AUDIO_DRIVER_PLAY_VOICE = 0,
	AUDIO_DRIVER_PLAY_AUDIO,
	AUDIO_DRIVER_PLAY_RINGER,
	AUDIO_DRIVER_PLAY_BT,
	AUDIO_DRIVER_CAPT_VOICE,
	AUDIO_DRIVER_CAPT_HQ,
	AUDIO_DRIVER_CAPT_FM,
	AUDIO_DRIVER_CAPT_BT,
	AUDIO_DRIVER_VOIP,
	AUDIO_DRIVER_VOIF,
	AUDIO_DRIVER_PLAY_EPT,
	AUDIO_DRIVER_CAPT_EPT,
	AUDIO_DRIVER_PTT
};
#define AUDIO_DRIVER_TYPE_t enum __AUDIO_DRIVER_TYPE_t

enum __AUDIO_DRIVER_CTRL_t {
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
	AUDIO_DRIVER_FREE_BUFFER,
	AUDIO_DRIVER_SET_VOIP_DL_CB,
	AUDIO_DRIVER_SET_VOIP_UL_CB,
	AUDIO_DRIVER_SET_VOIF_CB,
	AUDIO_DRIVER_BUFFER_READY,
	AUDIO_DRIVER_SET_PTT_CB,
	AUDIO_DRIVER_GET_PTT_BUFFER,
	AUDIO_DRIVER_SET_PERIOD_COUNT,
	AUDIO_DRIVER_SET_AMR,

	AUDIO_DRIVER_SET_DTX

};
#define AUDIO_DRIVER_CTRL_t enum __AUDIO_DRIVER_CTRL_t

struct _AUDIO_DRIVER_CONFIG_t {
	AUDIO_SAMPLING_RATE_t sample_rate;
	AUDIO_NUM_OF_CHANNEL_t num_channel;
	AUDIO_BITS_PER_SAMPLE_t bits_per_sample;
	UInt32 instanceId;	/* ARM2SP1/ARM2SP2 mapped to PCMOUT1/PCMOUT2 */
	UInt32 arm2sp_mixMode;
};
#define AUDIO_DRIVER_CONFIG_t struct _AUDIO_DRIVER_CONFIG_t

struct _AUDIO_DRIVER_BUFFER_t {
	UInt32 buf_size;
	UInt32 phy_addr;
	UInt8 *pBuf;
};
#define AUDIO_DRIVER_BUFFER_t struct _AUDIO_DRIVER_BUFFER_t

#define AUDIO_DRIVER_HANDLE_t void *

typedef void (*AUDIO_DRIVER_InterruptPeriodCB_t) (void *p);

typedef void (*AUDIO_DRIVER_VoipDLCB_t) (void *p,
			  u8 *pBuf,
			  u32 nsize,
			  u32 *timestamp);
typedef void (*AUDIO_DRIVER_VoipCB_t) (void *p, u8 * pBuf, u32 nsize);

typedef void (*AUDIO_DRIVER_PttCB_t) (void *p, Int16* pBuf, u32 nsize);


struct _AUDIO_DRIVER_CallBackParams_t {
	AUDIO_DRIVER_InterruptPeriodCB_t pfCallBack;
	void *pPrivateData;
	AUDIO_DRIVER_VoipCB_t voipULCallback;
	AUDIO_DRIVER_VoipDLCB_t voipDLCallback;
	AUDIO_DRIVER_PttCB_t  pttDLCallback;
};
#define AUDIO_DRIVER_CallBackParams_t struct _AUDIO_DRIVER_CallBackParams_t

struct _voip_data_t {
	UInt32 codec_type_ul;
	UInt32 codec_type_dl;
	UInt32 bitrate_index;
	UInt8 isVoLTE;
	UInt8 isDTXEnabled;
};
#define voip_data_t struct _voip_data_t

/* ARM2SP declarations */

enum __VORENDER_ARM2SP_INSTANCE_e {
	VORENDER_ARM2SP_INSTANCE1,
	VORENDER_ARM2SP_INSTANCE2,
	VORENDER_ARM2SP_INSTANCE_TOTAL
};
#define VORENDER_ARM2SP_INSTANCE_e enum __VORENDER_ARM2SP_INSTANCE_e

/* Voice capture declarations */

enum __VOCAPTURE_RECORD_MODE_t {
	VOCAPTURE_RECORD_NONE,
	VOCAPTURE_RECORD_UL,
	VOCAPTURE_RECORD_DL,
	VOCAPTURE_RECORD_BOTH
};
#define VOCAPTURE_RECORD_MODE_t enum __VOCAPTURE_RECORD_MODE_t

enum __VOCAPTURE_VOICE_MIX_MODE_t {
	VOCAPTURE_VOICE_MIX_NONE,
	VOCAPTURE_VOICE_MIX_DL,
	VOCAPTURE_VOICE_MIX_UL,
	VOCAPTURE_VOICE_MIX_BOTH
};
#define VOCAPTURE_VOICE_MIX_MODE_t enum __VOCAPTURE_VOICE_MIX_MODE_t

struct _AUDIO_DRIVER_Stats_t {
	int pipeID;
	int dmach;
	long intr_counter;
	int buffer_idx;
};

#define AUDIO_DRIVER_Stats_t struct _AUDIO_DRIVER_Stats_t

enum __VOICE_CALL_MODE_t {
	CALL_MODE_NONE,
	MODEM_CALL,
	PTT_CALL
};
#define VOICE_CALL_MODE_t enum __VOICE_CALL_MODE_t

struct _voice_rec_t {
	VOCAPTURE_RECORD_MODE_t recordMode;
	VOICE_CALL_MODE_t callMode;
};
#define voice_rec_t struct _voice_rec_t

extern UInt32 audio_control_dsp(UInt32 param1, UInt32 param2,
	UInt32 param3, UInt32 param4, UInt32 param5, UInt32 param6);

/**
*  @brief  This function is used to open the audio data driver
*
*  @param drv_type   (in)    driver type
*
*  @return AUDIO_DRIVER_HANDLE_t
*
 *************************************************************************/
AUDIO_DRIVER_HANDLE_t AUDIO_DRIVER_Open(AUDIO_DRIVER_TYPE_t drv_type);

/**
*  @brief  This function is used to check whether the voip driver is running
*
*  @param  none
*
*  @return True if voip driver is running, else false
*
 *************************************************************************/

Boolean AUDIO_DRIVER_VoipStatus(void);

/**
*  @brief  This function is used to close the audio data driver
*
*  @param  drv_handle   (in)  handle returned while opening the driver
*
*  @return none
*
 *************************************************************************/

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
 *************************************************************************/
void AUDIO_DRIVER_Read(AUDIO_DRIVER_HANDLE_t drv_handle,
		       UInt8 *pBuf, UInt32 nSize);

/**
*  @brief  This function is used to write the data to the driver
*
*  @param  drv_handle   (in)  handle returned while opening the driver
*  @param  pBuf         (in)  Pointer to the buffer to be read
*  @param  nSize        (in)  size of the buffer
*
*  @return none
*
 *************************************************************************/
void AUDIO_DRIVER_Write(AUDIO_DRIVER_HANDLE_t drv_handle,
			UInt8 *pBuf, UInt32 nBufSize);

/**
*  @brief  This function is used to send a control command to the driver
*
*  @param  drv_handle   (in)  handle returned while opening the driver
*  @param  ctrl_cmd     (in)  Command id
*  @param  pCtrlStruct  (in)  command data structure
*
*  @return none
*
 *************************************************************************/
void AUDIO_DRIVER_Ctrl(AUDIO_DRIVER_HANDLE_t drv_handle,
		       AUDIO_DRIVER_CTRL_t ctrl_cmd, void *pCtrlStruct);

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
 *************************************************************************/
void AUDIO_DRIVER_UpdateBuffer(AUDIO_DRIVER_HANDLE_t drv_handle,
			       UInt8 *pBuf,
			       UInt32 nBufSize,
			       UInt32 nCurrentIndex, UInt32 nSize);

UInt32 StreamIdOfDriver(AUDIO_DRIVER_HANDLE_t h);

/**
 *
 * Function Name: ARM2SP_Render_Request
 *
 * Description: Callback for ARM2SP1 playback
 *
 *************************************************************************/
void ARM2SP_Render_Request(UInt16 buf_index);

/**
 *
 * Function Name: ARM2SP2_Render_Request
 *
 * Description: Callback for ARM2SP2 playback
 *
 *************************************************************************/
void ARM2SP2_Render_Request(UInt16 buf_index);

/**
 *
 * Function Name: VPU_Capture_Request
 *
 * Description: Send a VPU capture request for voice capture driver to copy
 * data from DSP shared memory.
 *
 *************************************************************************/
void VPU_Capture_Request(UInt16 buf_index);

/**
* Prototype of voif callback function.
*
*   @param  ulData: The data pointer of the UL data. Input of VOIF processing
*   @param  dlData: The data pointer of the DL data. Input of VOIF processing
*	 and output VOIF processing.
*   @param  sampleCout: the number of samples, 16 bit per sample
*   @param  isCall16K: indicates whether voice call is WB or NB
*   @return void
*   @note   The user code use the callback function to get UL/DL data and
*	write back processed DL.
 *************************************************************************/
typedef void (*VOIF_CB) (Int16 *ulData, Int16 *dlData, UInt32 sampleCount,
			 UInt8 isCall16K);

/**
* VOIF processing interrupt handler.
*
*   @param  bufferIndex: The index of the ping-pong buffers to hold the DL/DL data
*   @return void
*   @note   Not for application programming.
 *************************************************************************/
void VOIF_Buffer_Request(UInt32 bufferIndex, UInt32 samplingRate);

void AP_ProcessStatusMainAMRDone(UInt16 codecType);
void VOIP_ProcessVOIPDLDone(void);


void AUDTST_VoIP(UInt32 Val2, UInt32 Val3, UInt32 Val4, UInt32 Val5,
		UInt32 Val6);
void AUDTST_VoIP_Stop(void);

void VOLTE_ProcessDLData(void);

void StartHRTimer(void);

#endif /* #define __AUDIO_DDRIVER_H__ */
