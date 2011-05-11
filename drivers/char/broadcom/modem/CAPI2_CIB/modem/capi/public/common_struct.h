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
*   @file   common_struct.h
*
*   @brief  This file defines structures from INC folder used by both CAPI and CAPI2
*
****************************************************************************/
#ifndef _COMMON_MSTRUCT_H_
#define _COMMON_MSTRUCT_H_


#ifdef __cplusplus
extern "C" {
#endif
   



//******************************************************************************
//
// MNATDS Message Types
//
//******************************************************************************

//******************************************************************************
//
// Message Parameters
//
//******************************************************************************

typedef struct
{
	T_SYSTEM_MODE		system_mode;
	T_DATA_RATE			data_rate;
	T_SYNCHRON_TYPE		synchron_type;
	T_CONN_ELEMENT		conn_element;
	T_SERVICE_MODE		service_mode;
    // 8/18/2004 From Comneon code (atc_setup.c), Instead of a list of possible RLP, it is now simplified:
    // if GEM, then version 2, if compression then version 1, otherwise, version 0.
	T_RLP_PAR			rlp_par;
	T_EST_CAUSE			est_cause;
	T_CALLING_SUBADDR	calling_subaddr;
	T_TEL_NUMBER		tel_number;
	T_CALLED_SUBADDR	called_subaddr;
	T_MN_CLIR			mn_clir;
	T_MN_CUG			mn_cug;
	SDL_Boolean			autocall;
	T_DC_PAR			ds_datacomp;
	T_DC_PAR			ds_hw_datacomp;
	T_DC_NEGOTIATION	ds_datacomp_neg;
    // 8/18/2004 yuanliu - this is for AT+ETBM. If not supported, then all values in the struct should be set to 0.
	T_ETBM				etbm;
} MNATDSParmSetupReq_t;


typedef struct
{
	T_AT_CAUSE			at_cause;
	T_TI_PD				ti_pd;
} MNATDSParmReleaseReq_t;

typedef struct
{
	T_SYSTEM_MODE		system_mode;
	T_RLP_PAR			rlp_par;
    T_TI_PD		  	   	ti_pd;
    T_CONN_SUBADDR  	conn_subaddr;
	T_DC_PAR			ds_datacomp;
	T_DC_PAR			ds_hw_datacomp;
	T_DC_NEGOTIATION	ds_datacomp_neg;
	T_ETBM				etbm;
} MNATDSParmSetupResp_t;


typedef struct
{
	T_DATA_RATE			data_rate;
	T_SERVICE_MODE		service_mode;
	T_CONN_ELEMENT		conn_element;
	T_TI_PD				ti_pd;
	T_TEL_NUMBER		tel_number;
	T_CALLING_SUBADDR	calling_subaddr;
	T_CALLED_SUBADDR	called_subaddr;
	Boolean				ds_datacomp_ind;
} MNATDSParmSetupInd_t;


typedef struct
{
	T_TI_PD				ti_pd;
	T_DC_PAR			ds_neg_datacomp;
} MNATDSParmConnectInd_t;

typedef struct
{
	T_TI_PD				ti_pd;
	T_SERVICE_MODE		service_mode;
} MNATDSParmServiceInd_t;

typedef struct
{ 
	T_TI_PD				ti_pd;
	T_TEL_NUMBER		tel_number;
	T_CONN_SUBADDR		conn_subaddr;
	T_DC_PAR			ds_neg_datacomp;
} MNATDSParmSetupCnf_t;

typedef struct
{
    T_MN_CAUSE  		mn_cause;
} MNATDSParmSetupRej_t;

typedef struct
{
    T_TI_PD  			ti_pd;
    T_MN_CAUSE  		mn_cause;
} MNATDSParmReleaseInd_t;

typedef struct
{
	T_TI_PD				ti_pd;
} MNATDSParmReleaseCnf_t;

typedef struct
{
	T_LINESTATE			linestate1;
	T_LINESTATE			linestate2;
	T_LINESTATE			linestate3;
} MNATDSParmStatusInd_t;

typedef struct 
{
	UInt8	ppm; 
} MNATDSParmFET_t;

typedef union
{
	MNATDSParmSetupReq_t		setup_req;
	MNATDSParmReleaseReq_t		release_req;
	MNATDSParmSetupResp_t		setup_resp;

	MNATDSParmSetupInd_t		setup_ind;
	MNATDSParmConnectInd_t		connect_ind;
	MNATDSParmServiceInd_t		service_ind;
	MNATDSParmSetupCnf_t		setup_cnf;
	MNATDSParmSetupRej_t		setup_rej;
	MNATDSParmReleaseInd_t		release_ind;
	MNATDSParmReleaseCnf_t		release_cnf;
	MNATDSParmStatusInd_t		status_ind;
	MNATDSParmFET_t				fet_ind;
} MNATDSMsgParm_t;						// MNATDS message parameters

typedef struct
{
	MNATDSMsgType_t 	type;			// MNATDS Message Type
	ClientInfo_t		clientInfo;		///<ClientInfo
	MNATDSMsgParm_t     parm;			// MNATDS Message Parameters
} MNATDSMsg_t;	

#ifdef __cplusplus
}
#endif


#endif  //_COMMON_MSTRUCT_H_

