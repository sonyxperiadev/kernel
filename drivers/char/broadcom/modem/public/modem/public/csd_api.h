//***************************************************************************
//
//	Copyright © 2005-2008 Broadcom Corporation
//	
//	This program is the proprietary software of Broadcom Corporation 
//	and/or its licensors, and may only be used, duplicated, modified 
//	or distributed pursuant to the terms and conditions of a separate, 
//	written license agreement executed between you and Broadcom (an 
//	"Authorized License").  Except as set forth in an Authorized 
//	License, Broadcom grants no license (express or implied), right 
//	to use, or waiver of any kind with respect to the Software, and 
//	Broadcom expressly reserves all rights in and to the Software and 
//	all intellectual property rights therein.  IF YOU HAVE NO 
//	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE 
//	IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
//	ALL USE OF THE SOFTWARE.  
//	
//	Except as expressly set forth in the Authorized License,
//	
//	1.	This program, including its structure, sequence and 
//		organization, constitutes the valuable trade secrets 
//		of Broadcom, and you shall use all reasonable efforts 
//		to protect the confidentiality thereof, and to use 
//		this information only in connection with your use 
//		of Broadcom integrated circuit products.
//	
//	2.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE 
//		IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM 
//		MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, 
//		EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, 
//		WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY 
//		DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, 
//		MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
//		PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
//		COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR 
//		CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE 
//		RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.  
//
//	3.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT 
//		SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR 
//		(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
//		EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY 
//		WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE 
//		SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
//		POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
//		EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE 
//		ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
//		LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE 
//		OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
/**
*
*   @file   csd_api.h
*
*   @brief  This file defines APIs for CSD data access.
*
****************************************************************************/
/**

*   @defgroup   DATACSDGroup   CSD Data Access
*   @ingroup    DATAServiceGroup
*
*   @brief      This group defines APIs for CSD data access
*
*	The whole CSD call is divided into two setups --- 
*	CSD call setup and CSD data access

*	1. The CSD setup is accomplished by the API ::CC_MakeDataCall / ::CC_AcceptDataCall and 
*	message ::MSG_DATACALL_CONNECTED_IND. 
*
*	2. The CSD API will be a thin data access layer to shield the ecdc/v24 detail 
*	and is parallel with the current MPX/NON_MUX code. It could be used by 
*	WAP client and other clients which needs to access CSD data.
*
*	Stack <--> ECDC <-->V24<-->CSD API <---->WAP client\n
*	Stack <--> ECDC <-->V24<-->MUX/NON_MUX-->UART
*
*
****************************************************************************/
#ifndef __CSD_API_H__
#define __CSD_API_H__

typedef enum
{
	CSDLINESTATE_LINE_ON = 0,	// on-line
	CSDLINESTATE_LINE_OFF		// off-line
} CSDLineState_t;

/**
 * @addtogroup DATACSDGroup
 * @{
 */

/**
Data callback function prototype
@param clientID		(in)	Specify Client interesting in the CSD data
@param newDataSize	(out)	Received ata size
@return
**/
typedef void (*DataIndCallBackFunc_t)(UInt8 clientID, UInt16 newDataSize);

/**
//Send data pointed by the data pointer to the network(air interface).

//The caller should use the returned size as the indication of data buffer 
//availability.The caller can release its buffer after this API call 
//without affecting the data transfer.

@param clientID	(in) Specify Client to send the data to the network       
@param data		(in) date pointer 
@param dataSize	(in) size of the sent data
@return	Return the size of the data that is successfully sent. 
**/
UInt16 CSD_SendData(UInt8 clientID, UInt8 *data, UInt16 dataSize);

/**
//Check the data buffer space for sending     
//The client can call this API to check the free space 
//in the data buffer before sending the data using API CSD_SendData.

@param clientID (in) Specify Client to query the free space. 
@return	Return the size of the free buffer space for sending 
**/
UInt16 CSD_BufferFreeSpace(UInt8 clientID);

/**
//Register the callback to receive the indication of data available 
//from the network(air interface)

@param clientID (in) Specify Client interesting in the CSD data
@param dataIndFun (in) callback function to inform the availability of the data.
@return	Return RESULT_OK if the registration is successful. Otherwise RESULT_ERROR
**/

Result_t CSD_RegisterDataInd (UInt8 clientID, DataIndCallBackFunc_t dataIndFun);

/**
//The client of API should manage the receiver buffer after the data has been 
//processed in the buffer. Usually, After the client receives the data available 
//indication through the callback registered with CSD_RegisterDataInd API, 
//It calls this API to retrieve the available data based on it's capacity to 
//handle the data 

@param clientID		(in)	Specify Client to receive the data from the network(air interface).
@param recvDataBuf	(out)	The pointer to the receiver buffer.
@param readSize     (in)	number of bytes to retrieve from data buffer.
@return	Return the real available data size in the recvDataBuf.
**/

UInt16  CSD_GetData(UInt8 clientID, UInt8* recvDataBuf, UInt16 readSize);

typedef void (*DataIndCallBackFuncEx_t)(DLC_t dlc, UInt16 newDataSize);
typedef void (*FCIndCallBackFunc_t)(DLC_t dlc, CSDLineState_t fc);
typedef void (*BuffRdyCallBackFunc_t)(DLC_t dlc);


DLC_t CsdApi_RegisterDataInd (ClientInfo_t *clientInfoPtr, DataIndCallBackFuncEx_t dataIndFun, FCIndCallBackFunc_t FCIndFun);
Result_t CsdApi_RegisterBufRdy (ClientInfo_t *clientInfoPtr,  BuffRdyCallBackFunc_t buffRdyFun);
Result_t CsdApi_DeregisterDataInd (ClientInfo_t *clientInfoPtr);

Int16  CsdApi_GetData(DLC_t dlc, UInt8* recvDataBuf, UInt16 readSize);
Int16 CsdApi_GetBufferFreeSpace(DLC_t dlc);
Int16 CsdApi_SendData(DLC_t dlc, UInt8 *data, UInt16 dataSize);


typedef UInt16 (*VTIndCallBackFunc_t)(DLC_t dlc, UInt8 *data, UInt16 newDataSize);

DLC_t CsdApi_RegisterVTDataInd (ClientInfo_t *clientInfoPtr, VTIndCallBackFunc_t vtDataIndFun, VTIndCallBackFunc_t buffRdyFun);


/** @} */
#endif

