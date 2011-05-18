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
*   @file   callctrlimpl.h
*
*   @brief  The file contains definitions of call control implementation 
*			hiden from the call control API.
*
****************************************************************************/

#ifndef _CALLCTRLIMPL
#define _CALLCTRLIMPL



#define INVALID_MARK	102

///	Fax Class
typedef enum
{
        FCLASSSERVICEMODE_DATA = 0,			///< Fax class 0
        FCLASSSSERVICEMODE_FAX_2 = 2,		///< Fax class 2
        FCLASSSSERVICEMODE_FAX_2dot0 = 20	///< Fax class 2.0

} FClassServiceMode_t;


//Call structure to keep the data about a call
typedef struct{

	UInt8					callNum[MAX_DIGITS+2]; ///< (Note: 1 byte for null termination and the other for int code '+')
	PresentationInd_t		present;				///< Calling party presentation indicator
	CallingInfo_t           callingInfo;
	UInt8					callIndex;
	CCallType_t				callType;
	CCallState_t 			callState;
	Cause_t					callExitCause;
	Boolean					callMpty;
	Boolean					callConnectedIDPresent;
	UInt8					callConnectedID[MAX_DIGITS+2];///< (Note: 1 byte for null termination and the other for int code '+')

	UInt32					callStateTimeStamp;
	UInt32					callStartTimeStamp;
	Boolean					callSetupPhase;
	Boolean					callSimOrigined;

	UInt8					clientID;
	ClientInfo_t			clientInfo;
} CCallInfo_t;


///	Call Session
typedef enum{

	VOICECALL,		///< Voice Call session
	DATACALL,		///< Data Call	session
	FAXDATACALL,	///< Fax/Data Call
	MULTIPARTY,		///< Multiparty Voice Call
	IDLE_SESSION,	///< Idle session
	VIDEOCALL		///< Video Call

} CCallSession_t;


typedef struct{

	CCallSession_t	curCallSession;
	UInt8			curCallNum[MAX_DIGITS+2];///< (Note: 1 byte for null termination and the other for int code '+')
	UInt8           curMOSetupNum[MAX_DIGITS+2];///< (Note: 1 byte for null termination and the other for int code '+')
	UInt8			curCallNumType;//145 or 129
	UInt8			curCallIndex;
	CCallType_t		curCallType;
	//the index to the callinfo table 
	UInt8			curCallInteralID;

	UInt8           curMOSetupInteralID;

	//Next held call index
	UInt8			nextHeldCallIndex;
	//Next wait call index
	UInt8			nextWaitCallIndex;

	Cause_t			lastCallExitCause;
	UInt32			lastCallCCMUnit;
	UInt32			lastCallDuration;
	UInt32			lastDataCallRxBytes;
	UInt32			lastDataCallTxBytes;

} CCallCtrlCxt_t;


typedef struct{

	//For data call request parameter
	MNATDSParmSetupReq_t	dataCallReqParam;
	

} DataCallReqParam_t;

typedef struct{

	//For data call response parameter
	MNATDSParmSetupResp_t	dataCallRespParam;

}DataCallRespParam_t;


CCallInfo_t*	_CallInfoTable(void);
CCallCtrlCxt_t*	_CallCtrlCxt(void);

void		 _InitCallCtrlImpl(void);

UInt8 _AddOneNewCall(ClientInfo_t*	inClientInfoPtr,
					 UInt8*			callNum,
					 CCallType_t	callTy);

Boolean		 _CheckCallIndex(UInt8 callIndex);
Boolean		 _UpdateCallState(UInt8 callIndex,CCallState_t state);
Boolean		 _UpdateAllCallStates(CCallState_t state);
Boolean		 _UpdateCallExitCause(UInt8 callIndex,Cause_t cause);
Boolean		 _UpdateAllActiveCallStates(CCallState_t state);
Boolean		 _UpdateCallConnectTime(UInt8 callIndex);
Boolean		 _NewCurCallContext(UInt8 callIndex);
Boolean		 _CleanupCurCallContext(void);
Boolean		 _SetNextHeldCallIndex(UInt8 callIndex);
UInt8		 _GetNextHeldCallIndex(void);
Boolean		 _SetNextWaitCallIndex(UInt8 callIndex);
UInt8		 _GetNextWaitCallIndex(void);
UInt8        _GetNextBeginningCallIndex(void);
UInt8		 _GetNextActiveCallIndex(void);
Boolean		 _GetCallIndexInThisState(	UInt8*			inCallIndexPtr,
										CCallState_t	inCcCallState);
Boolean		 _DelOneCallByIndex(UInt8 callIndex);
Boolean		 _DelOneCallByPhoneNum(UInt8* callNum);
Boolean		 _DelOneCallByInternalID(UInt8 id);
Boolean		 _FinishCallSetupPhase(UInt8 callIndex);
Boolean		 _ThereIsCallInSetupPhase(void);
Boolean		 _ClearCallTable(void);
Boolean		 _GetAllCallStates(CCallStateList_t* stateList,UInt8* listSz);
Boolean		 _GetAllCallIndex(CCallIndexList_t* indexList,UInt8* listSz);
Boolean		 _GetAllCallIndexByState(CCallIndexList_t* indexList,UInt8* listSz,CCallState_t state);
Boolean		 _GetAllCallIndexInMPTY(CCallIndexList_t* indexList,UInt8* listSz,Boolean inMPTY);

Cause_t		 _GetCallExitCause(UInt8 callIndex);
CCallState_t _GetCallState(UInt8 callIndex);
CCallType_t	 _GetCallType(UInt8 callIndex);
Boolean		 _GetCallNumber(UInt8 callIndex,UInt8* phNum);
Boolean		 _SetCallNumber(UInt8 callIndex,UInt8* phNum);


CCallInfo_t* _Get_CallTableCtx(UInt8  callIndex);
Boolean		 _GetCallingInfo(UInt8 callIndex, CallingInfo_t* callingInfoPtr);
Boolean		 _SetCallInfoPresent(UInt8 callIndex, UInt8 inPresent);
Boolean		 _SetCallName(	UInt8  callIndex,
							UInt8  dataCodeScheme,
							UInt8  lengthInChar,
							UInt8  nameSize,
							UInt8* namePtr);
Boolean		 _GetCallPresent(UInt8 inCallIndex, PresentationInd_t* inPresentPtr);


UInt8		 _GetTypeAdd(UInt8 callIndex);
//Boolean		 _SetCallClientID(UInt8 callIndex,UInt8 clientID);
UInt8		 _GetCallClientID(UInt8 callIndex);
void		 _GetCallClientInfo(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);
UInt8		 _CallTableIdx(UInt8 callIndex);

void		 _InitFaxCall(void);


void Post_SpeechCall(ClientInfo_t* inClientInfoPtr,
					 CCParmSend_t* ccParamSendPtr);

void _UpdateVoiceCallStatus(CallDesc_t*		inCcCtxPtr,
							CallStatus_t	status);

void _UpdateDataCallStatus(	CallDesc_t*		inCcCtxPtr,
							CallStatus_t	status);

void _UpdateDataCallMoCiAlloc(	CallIndex_t	index, 
								CallType_t	call_type);

void  _ComposeDataCallParam(CallConfig_t*			cfg,
							TelephoneNumber_t*		phoneNumberPtr,
							MNATDSParmSetupReq_t*	mnCallParam);

void  _ComposeDataCallRespParam(CallConfig_t*			cfg,
								MNATDSParmSetupResp_t* mnDataCallResp);

Boolean	_SetCallClientInfo(	ClientInfo_t*	inClientInfoPtr,
							UInt8			callIndex);

#endif

