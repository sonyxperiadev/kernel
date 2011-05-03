//*********************************************************************
//
//	Copyright © 2008 Broadcom Corporation
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
*   @file   ms_sysutil.h
*
*   @brief  This file defines the utility functions for MS.
*
****************************************************************************/

void MS_SetGsmRxLevThresold(UInt8 Thresold);
void MS_SetGsmRxQualThresold(UInt8 Thresold);
void MS_SetUmtsRscpThresold(UInt8 Thresold);
void MS_SetUmtsEcioThresold(UInt8 Thresold);
UInt8 MS_GetRxSignalLev( void );
UInt8 MS_GetRxSignalQual( void );

Boolean				MS_IsRegisteredHomePLMN(void);
void				MS_SetPlmnMCC(UInt16 mcc);
void				MS_SetPlmnMNC(UInt8 mnc);

void				MS_StatisticInfoReport(void);
Boolean				MS_GetInvalidSimMsMe(void);
void				SYS_SetRxSignalInfo(UInt8 RxLev, Boolean RxQualValid, UInt8 RxQual);
void				SYS_SetGSMRegistrationStatus(RegisterStatus_t status);
void				SYS_SetGPRSRegistrationStatus(RegisterStatus_t status);
void				SYS_SetGSMRegistrationCause( PCHRejectCause_t cause );
void				SYS_SetGPRSRegistrationCause( PCHRejectCause_t cause );
PCHRejectCause_t	SYS_GetGSMRegistrationCause(void);

//**************************************************************************************
/**
	This function is used to notify to all the registered clients of a database element
	content update..

	@param		inClientInfoPtr (in) The Client Information.
	@param		inElemType (in) The database element type.

	@return		Result_t
**/
void MS_SendLocalElemNotifyInd( ClientInfo_t*	inClientInfoPtr,
								MS_Element_t	inElemType);



/**
	The following APIs are obsolete. Please use MS_GetElement() and MS_SetElement() with element valuee
	set to MS_NETWORK_ELEM_GPRSINVALID_SIM_MS_ME_STATUS and MS_NETWORK_ELEM_CSINVALID_SIM_MS_ME_STATUS
**/
RegisterStatus_t	MS_GetCsInvalidSimMsMe(void);

RegisterStatus_t	MS_GetGprsInvalidSimMsMe(void);

Boolean  MS_IsClientGeneratedRingTone(void);

Boolean MS_IsPendingDetachCnf(SimNumber_t sim_id);

void MS_SetPendingDetachCnf(SimNumber_t sim_id, Boolean flag);

void MS_SetMsClass(SimNumber_t sim_id, MSClass_t ms_class);

MSClass_t MS_GetMsClass(SimNumber_t sim_id);

void MS_SetTestSIMInserted(Boolean is_test_sim);

Boolean MS_IsTestSIMInserted(void);



