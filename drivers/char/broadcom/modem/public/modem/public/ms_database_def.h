/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license 
*   agreement governing use of this software, this software is licensed to you 
*   under the terms of the GNU General Public License version 2, available 
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL"). 
*
*   Notwithstanding the above, under no circumstances may you combine this 
*   software in any way with any other Broadcom software provided under a license 
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
/**
*
*   @file   ms_database_def.h
*
*   @brief  This file contains the some common defines used by all layers
*
****************************************************************************/

#ifndef _MS_DATABASE_DEF_H_
#define _MS_DATABASE_DEF_H_

/**
Enum:	Database Element Type
		This enum is used for defining the type of elements in the database.

 * IMPORTANT NOTE: Please add any new element particular to a functional area in that section.
@internal
While adding a new element the checklist below should be followed:
1.	Update MS_Element_t enum in common_defs.h to add the new element name
    as per the naming conventions and in the correct section ( e.g ss element should be grouped together etc)
2.	Add code to the correct local function for setting and retrieving the element value ( MS_SetLocalSSElem, MS_GetLocalPhCtrlElem etc)
3.	Update the MS_GetElementFuncTable[] and MS_SetElementFuncTable[] to provide the appropriate function pointer corresponding to the elementtype.
    ** It is mandatory to provide the funcptr for both the Get and Set tables.
4.	Update the ms_database document (Sec 2.4) to add the element name and type and any comments and check it back in.
	( http://sp%2Dmpg%2Ebroadcom%2Ecom/mobcom/SW-Platform_Framework/Shared%20Documents/CAPI/Design/MS_DatabaseDesign.doc )
 

**/
typedef enum
{
	MS_ELEM_INVALID,								///< Invalid  

	//----SS Local Elements
	MS_LOCAL_SS_ELEM_CLIP, 							///<Element type {::UInt8}, CLIP  
	MS_LOCAL_SS_ELEM_CLIR,							///<Element type {::UInt8}, CLIR  
	MS_LOCAL_SS_ELEM_COLP,							///<Element type {::UInt8}, COLP  
	MS_LOCAL_SS_ELEM_CCWA,							///<Element type {::UInt8}, CCWA
	MS_LOCAL_SS_ELEM_LONG_FTN_SUPPORTED,			///<Element type {::UInt8}, Long Forwarded To Number Supported
	MS_LOCAL_SS_ELEM_FDN_CHECK,						///<Element type {::UInt8}, SS FDN check
	MS_LOCAL_SS_ELEM_NOTIFICATION_SWITCH,			///<Element type {::Uint8}, TRUE if SS Notification Switch is ON (e.g. multiple CFU notifications)
	MS_LOCAL_SS_ELEM_TX_USSD_DATA_ENCODE,			///<Element type {::UInt8}, Whether the Tx USSD phase 2 data is encoded by the client as per DCS (1/0 : encoded/not encoded)
	MS_LOCAL_SS_ELEM_RX_USSD_DATA_ENCODE,			///<Element type {::UInt8}, Whether the Rx USSD phase 2 data shall be encoded by the MNSS module as per DCS (1/0 : encoded/not encoded)
	MS_LOCAL_SS_ELEM_INCLUDE_FACILITY_IE,			///<Element type {::UInt8}, Include Facility IE in the res/ind/data for the client
	MS_LOCAL_SS_ELEM_INVOKE_ID,						///<Element type {::UInt8}, Invoke Identifier for each MO SS related session
	MS_LOCAL_SS_ELEM_ENABLE_OLD_SS_MSG,				///<Element type {::UInt8}, TRUE if old SS API MSG will be generated and send to client
	MS_LOCAL_SS_ELEM_CLIENT_HANDLE_USSD_NOTIFY,		///<Element type {::Uint8}, TRUE if client will handle USSD Notification response

	
	//----Test Local Elements
	MS_LOCAL_TEST_ELEM_CHAN,						///<Element type {::MS_TestChan_t}, the test frequencies for each band  
	MS_LOCAL_TEST_ELEM_PROD_MODE,					///<Element type {::UInt8}, if 0 - disable production test mode else  
													///<Element type {::UInt8}, enables production test mode.  
	//----CC Local Elements
	MS_LOCAL_CC_ELEM_AUDIO,							///<Element type {::Boolean}, AUDIO control in MNCC enable/disable 
	MS_LOCAL_CC_ELEM_VIDEO_CALL,					///<Element type {::UInt8}, Video call feature  
	MS_LOCAL_CC_ELEM_DEF_VOICE_BC,					///<Element type {::BearerCapability_t}, Default Bearer Capability used for voice call initiation  
	MS_LOCAL_CC_ELEM_CLIENT_GEN_TONE,				///<Element type {::Boolean}, whether client generated tone is enabled 
	MS_LOCAL_CC_ELEM_SCUDIF,						///<Element type {::Boolean}, whether the SCUDIF feature is enabled of disabled
	MS_LOCAL_CC_ELEM_SPEECH_CODEC_LIST,				///<Element type {::UInt8[MAX_SUPPORTED_CODEC_LIST_LEN]}, speech codec list to be used in call control messages, this should change if the channel mode supported changes

	MS_LOCAL_CC_ELEM_CURR_CALL_CFG,					///<Element type {::CallConfig_t}, Current call configuration (type CallCfg_t)
	MS_LOCAL_CC_ELEM_HSCSD_REPORT,					///<Element type {::HSCSDParamReport_t}, reported in ATDS_REPORT_IND message
	MS_LOCAL_CC_ELEM_IS_HSCSD_CALL,					///<Element type {::Boolean}, boolean true if CSD/HSCSD call
	MS_LOCAL_CC_ELEM_RX_SLOTS,						///<Element type {::UInt8}, current call receive slots	
	MS_LOCAL_CC_ELEM_TX_SLOTS,						///<Element type {::UInt8}, current call transmit slots
	MS_LOCAL_CC_ELEM_AIR_IF_USER_RATE,				///<Element type {::UInt8}, current call air interface user rate
	MS_LOCAL_CC_ELEM_CURRENT_CODING,				///<Element type {::UInt8}, current call codings (9600/14400 bps per slot)
	MS_LOCAL_CC_ELEM_CURRENT_CONNECT_ELEMENT,		///<Element type {::UInt8}, Current call connection element(Transparent vs. non-transparent)
	MS_LOCAL_CC_ELEM_TEL_NUMBER,					///<Element type {::PhoneNumber_t}, Telephone number
	MS_LOCAL_CC_ELEM_CALLING_SUBADDR,				///<Element type {::SubAddress_t}, Sub Address of the calling station
	MS_LOCAL_CC_ELEM_CALLED_SUBADDR,				///<Element type {::SubAddress_t}, Sub Address of the called station
	MS_LOCAL_CC_ELEM_CONNECT_SUBADDR,				///<Element type {::SubAddress_t}, Connection sub address
	MS_LOCAL_CC_ELEM_CRLP,							///<Element type {::RlpParam_t[2]}, Radio Link Protocol parameters	
	MS_LOCAL_CC_ELEM_CBST ,							///<Element type {::CBST_t}, Bearer Service parameters
	MS_LOCAL_CC_ELEM_DS_NEG_DATACOMP,				///<Element type {::DCParam_t}, Data compression parameters negotiated between peers
	MS_LOCAL_CC_ELEM_DS_REQ_DATACOMP,				///<Element type {::DCParam_t}, Data compression parameters requested by host
	MS_LOCAL_CC_ELEM_DS_REQ_SUCCESS_NEG,			///<Element type {::Boolean}, Boolean value to indicate if DC negotiation is necessary for call completion
	MS_LOCAL_CC_ELEM_EC_REQUESTED,					///<Element type {::ECOrigReq_t}, Error correction parameter requested
	MS_LOCAL_CC_ELEM_EC_FALLBACK_NOT_NEGOTIABLE,	///<Element type {::ECOrigFB_t}, Error correction parameter fall back value if negotiation is not possible
	MS_LOCAL_CC_ELEM_EC_FALLBACK_NO_ANSWER,			///<Element type {::ECAnsFB_t}, Error correction parameter answer fall back value
	MS_LOCAL_CC_ELEM_EC_PARAMS,						///<Element type {::ECMode_t}, Negotiated Error correction parameters
	MS_LOCAL_CC_ELEM_DC_PARAMS,						///<Element type {::DCMode_t}, Negotiated Data compression parameters
	MS_LOCAL_CC_ELEM_MCLASS,						///<Element type {::UInt8}, Multi Slot class of the mobile
	MS_LOCAL_CC_ELEM_MAX_RX_SLOTS,					///<Element type {::UInt8}, Maximum Receive slots
	MS_LOCAL_CC_ELEM_MAX_TX_SLOTS,					///<Element type {::UInt8}, Maximmum transmit slots
	MS_LOCAL_CC_ELEM_SUM_SLOTS,						///<Element type {::UInt8}, Sum of slots
	MS_LOCAL_CC_ELEM_CODINGS,						///<Element type {::UInt8}, Channel Codings ie 9600/14400 bps slots
	MS_LOCAL_CC_ELEM_WANTED_RX_SLOTS,				///<Element type {::UInt8}, Number of wanted receive time slots
	MS_LOCAL_CC_ELEM_WANTED_AIR_USER_RATE,			///<Element type {::UInt8}, Wanted Air Interface User Rate
	MS_LOCAL_CC_ELEM_MAX_TX_SLOTS_SUPPORTED,		///<Element type {::UInt8}, Maximum number of transmit slots that can be supported
	MS_LOCAL_CC_ELEM_CURR_TI_PD,					///<Element type {::UInt8}, Current transaction identifier and protocol discriminator
	MS_LOCAL_CC_ELEM_CURR_SERVICE_MODE,				///<Element type {::UInt8}, Current service mode
	MS_LOCAL_CC_ELEM_DS_DATACOMP_IND,				///<Element type {::Boolean}, Remote Compression indication 
	MS_LOCAL_CC_ELEM_CUGI,							///<Element type {::UInt8}, Closed user group index: 0 - 10
	MS_LOCAL_CC_ELEM_SPKR_LOUDNESS,					///<Element type {::UInt8}, for speaker loudness
	MS_LOCAL_CC_ELEM_SPKR_MONITOR,					///<Element type {::UInt8}, for speaker monitor
	MS_LOCAL_CC_ELEM_CALL_METER,					///<Element type {::UInt32}, Current Call Meter
	MS_LOCAL_CC_ELEM_ENABLE_AUTO_REJECT,			///<Element type {::Boolean}, auto reject MT voice call
	MS_LOCAL_CC_ELEM_ETBM,							///<Element type {::T_ETBM}, support of AT+ETBM command.
	MS_LOCAL_CC_ELEM_CLIR,							///<Element type {::CLIRMode_t}, Calling Line ID Restriction
	
	MS_LOCAL_CC_ELEM_FAX_PARAMS,					///<Element type {::Fax_Param_t}, Fax call related paramters (type FaxParam_t)
	MS_LOCAL_CC_ELEM_FAX_FCLASS,					///<Element type {::UInt8}, Fax class
	MS_LOCAL_CC_ELEM_CSNS,							///<Element type {::UInt8}, Single numbering scheme


	//----SMS Local Elements
	MS_LOCAL_SMS_ELEM_DEF_REC_NUMBER,				///<Element type {::UInt8}, SMS Default Recorde Number
	MS_LOCAL_SMS_ELEM_CLIENT_HANDLE_MT_SMS,			///<Element type {::UInt8}, Client Handles MT SMS
	MS_LOCAL_SMS_ELEM_CLIENT_HANDLE_MO_SMS_RETRY,	///<Element type {::UInt8}, Client Handles MO SMS RETRY
	MS_LOCAL_SMS_ELEM_MORE_MESSAGE_TO_SEND,			///<Element type {::UInt8}, More Message to Send
	
	//----SATK Local Elements
	MS_LOCAL_SATK_ELEM_SETUP_CALL_CTR,				///<Element type {::Boolean}, Whether BRCM STK module handles STK Setup Call  
	MS_LOCAL_SATK_ELEM_SEND_SS_CTR,					///<Element type {::Boolean}, Whether BRCM STK module handles STK Send SS  
	MS_LOCAL_SATK_ELEM_SEND_USSD_CTR,				///<Element type {::Boolean}, Whether BRCM STK module handles STK Send USSD  
	MS_LOCAL_SATK_ELEM_SEND_SMS_CTR,				///<Element type {::Boolean}, Whether BRCM STK module handles STK Send SMS   
	MS_LOCAL_SATK_ELEM_SEND_DTMF_CTR,				///<Element type {::Boolean}, Whether BRCM STK module handles STK Send DTMF  
	MS_LOCAL_SATK_ELEM_CALL_CONTROL_CTR,			///<Element type {::Boolean}, Whether BRCM STK module handles STK voice Call Control by SIM
	MS_LOCAL_SATK_ELEM_SS_CONTROL_CTR,				///<Element type {::Boolean}, Whether BRCM STK module handles STK SS Call Control by SIM
	MS_LOCAL_SATK_ELEM_USSD_CONTROL_CTR,			///<Element type {::Boolean}, Whether BRCM STK module handles STK USSD Call Control by SIM
	MS_LOCAL_SATK_ELEM_SMS_CONTROL_CTR,				///<Element type {::Boolean}, Whether BRCM STK module handles STK SMS Control by SIM
	MS_LOCAL_SATK_ELEM_GENERIC_INTERFACE_CTR,		///<Element type {::Boolean}, Whether BRCM STK module uses generic data interface for most STK commands
	MS_LOCAL_SATK_ELEM_DYN_TERM_PROFILE,		    ///<Element type {::Boolean}, Whether USIMAP to wait for terminal profile.
    MS_LOCAL_SATK_ELEM_CMD_FETCH_ENABLED_AT_STARTUP,///<Element type {::Boolean}, Whether proactive cmd fetching is enabled at startup 

	//Phonebook Elements
	MS_LOCAL_PBK_ELEM_BDN_CACHE_CTR,                ///<Element type {::Boolean}, Whether BRCM PBK module supports BDN caching

	//Pch Elements
	MS_LOCAL_PCH_ELEM_SECONDARY_PDP_CONTEXT, 		///<Element type {::UInt8}, TRUE if the secondary PDP context support is enabled. 

	MS_LOCAL_PHCTRL_ELEM_EBSE,						///<Element type {::UInt8}, EBSE the band selected through EBSE  
	MS_LOCAL_PHCTRL_ELEM_PLMN_SESSION,				///<Element type {::PLMNSession_t}, current plmn session - refer PLMNSession_t 
	MS_LOCAL_PHCTRL_ELEM_PLMN_MODE,					///<Element type {::PlmnSelectMode_t}, element should be accessed using PlmnSelectMode_t type  
	MS_LOCAL_PHCTRL_ELEM_PREV_PLMN_MODE,   			///<Element type {::PlmnSelectMode_t}, previous PLMN selection mode; accessed using PlmnSelectMode_t 
	MS_LOCAL_PHCTRL_ELEM_PLMN_SELECT_RAT,			///<Element type {::UInt8}, GSM,UMTS or both, accessed using UInt8  
	MS_LOCAL_PHCTRL_ELEM_PREV_PLMN_SELECT_RAT,		///<Element type {::UInt8}, rat for the selected plmn  
	MS_LOCAL_PHCTRL_ELEM_PLMN_FORMAT,				///<Element type {::PlmnSelectFormat_t}, refer PlmnSelectFormat_t  
	MS_LOCAL_PHCTRL_ELEM_ABORT_PLMN_SELECT,			///<Element type {::Boolean}, access using Boolean 
	MS_LOCAL_PHCTRL_ELEM_PCM_MODE,					///<Element type {::Boolean}, 1=Digital PCM interface enabled; 0=disabled,UInt8 
	MS_LOCAL_PHCTRL_ELEM_IS_PWRDWN_REFRESH_SIM_RESET, ///< Modem is being powered down for STK refresh of SIM reset type 
	MS_LOCAL_PHCTRL_ELEM_EMERGENCY_IDLE_MODE,		///<Element type {::Boolean},Force emergency idle mode so that we camp on a cell for limited service only
	MS_LOCAL_PHCTRL_ELEM_MASTER_SIM_MODE,           ///< Element type {::Boolean}, If SIM1 not inserted, tag SIM2 with SIM1 ID (required if MMI has a master SIM design)
	MS_LOCAL_PHCTRL_ELEM_DUALSIM_VM_PWR_OPTIMIZATION,   ///< Element type {::Boolean}, If we should enable dual-sim VM power optimization (automatically power-down one VM if two VM's are powered up 
														///< and at least one VM has abnormal SIM status, e.g. SIM PIN locked/blocked) 
	MS_LOCAL_PHCTRL_ELEM_CGEREP, 					///<Element type {::UInt8[2]}, group event reporting details  
	MS_LOCAL_PHCTRL_ELEM_SELECTED_PLMN,				///<Element type {::PLMN_t}, the current selected plmn  
	MS_LOCAL_PHCTRL_ELEM_PREV_SEL_PLMN, 			///<Element type {::PLMN_t}, previous selected plmn  
	MS_LOCAL_PHCTRL_ELEM_PREF_USIM_PBK,				///<Element type {::USIM_PBK_PREFERRED_t}, preferred USIM phonebook selection
	MS_LOCAL_PHCTRL_ELEM_CSCS,						///<Element type {::UInt8[10]}, the character set to be used for conversion of char between TE and MT
	MS_LOCAL_PHCTRL_ELEM_CPWC,						///<Element type {::UInt8[3]},the power class for all the gsm frequency bands, set by the user
	MS_LOCAL_PHCTRL_ELEM_PWRDWN_DETACH_TIMER,		///<Element type {::UInt8}, value for the power down detach timer
	MS_LOCAL_PHCTRL_ELEM_PWRDWN_STORAGE_TIMER,		///<Element type {::UInt8}, value for power down storage timer
	MS_LOCAL_PHCTRL_ELEM_ATTACH_MODE,				///<Element type {::MSAttachMode_t}, attach mode, can be GSM, GPRS, GSM_GPRS
	MS_LOCAL_PHCTRL_ELEM_USER_NW_RESELECT,			///<Element type {::Boolean}, whether user network reselection is triggered	
	MS_LOCAL_PHCTRL_ELEM_WAIT_BEFORE_DETACH_TIME,	///<Element type {::UInt16}, used to define the time MS should wait before detach after pdp deactivation or 
													///                          deactivation failure when the attach mode is GSM_ONLY
	MS_LOCAL_PHCTRL_ELEM_SYSTEM_STATE,				///<Element type {::SystemState_t }, System state
	MS_LOCAL_PHCTRL_ELEM_POWER_DOWN_STATE,			///<Element type {::PowerDownState_t}, Power Down state	
	MS_LOCAL_PHCTRL_ELEM_IMEI,		                ///< IMEI
	MS_LOCAL_PHCTRL_ELEM_SW_VERSION,				///< ( Software version )
	MS_LOCAL_PHCTRL_ELEM_FORCE_SIM_NOT_RDY,			///<Element type {::Boolean}, Force "SIM not ready" status so that we send "NO SIM" status in ATTACH_REQ to stack
	MS_LOCAL_PHCTRL_ELEM_FAST_BAND_SELECT_DISABLE,	///<Element type {::Boolean}, Disable fast band select during RAT/band switch
	MS_LOCAL_PHCTRL_ELEM_SMSME_STORAGE_ENABLE,		///<Element type {::Boolean}, SMS ME Storage Enable flag 
	MS_LOCAL_PHCTRL_ELEM_SMSME_MAX_CAPACITY,		///<Element type {::UInt16},  SMS ME Max capacity
	MS_LOCAL_PHCTRL_ELEM_GAN_MODE,					///<Element type {::GANSelect_t},  the preferrence for GERAN or GAN	
	MS_LOCAL_PHCTRL_ELEM_CELL_LOCK_PARAMS,			///< cell lock values to be set to the stack at powerup or afer internal deactivation.
	MS_LOCAL_PHCTRL_ELEM_PLMNSEL_REGISTERED_PLMN,	///< If this element is TRUE then enable the full manual selection even if the selected PLMN is
													///< the current registered PLMN. ( Disabled by default )
	MS_LOCAL_PHCTRL_ELEM_FREQ_SEARCH_LIST,			///< stores the freq search list set by MMI before power up.													
	MS_LOCAL_PHCTRL_ELEM_EM_ENABLED,				///< indicates if the engineering mode is enabled by MMI
	MS_LOCAL_PHCTRL_ELEM_EM_PERIODIC_TIME_INTERVL,	///< indicates the periodicity of the engineering mode measurement reporting by stack.
	MS_LOCAL_PHCTRL_ELEM_ON_DEMAND_ATTACH_ALWAYS,	///< Boolean. TRUE = attach mode will be GSM_ONLY from powerup. no configuration of attach mode allowed 
														///< till this flag is TRUE. FALSE = user configuration of attach mode allowed. msclass conflict check will 
														///< be enabled. 
	MS_LOCAL_PHCTRL_ELEM_CURR_ATTACH_MODE,			///< reflects the current status of attach required by user (changed by cgatt and ATTACH_MODE)


	//-----AT Elements	
	MS_LOCAL_AT_ELEM_AUDIO_CTRL,					///<Element type {::Boolean}, AT audio control setting  
	
	//-----General MS Elements
	MS_LOCAL_MS_ELEM_ASYNC_RETURN_RESULT,			///<Element type {::Boolean}, Whether the platform shall send syn/asyncronous response
	MS_LOCAL_MS_ELEM_DIALOG_ID,						///<Element type {::UInt32}, Dialog Identifier for each (i.e. SS, etc.) session

	//------Stack Elements
	
	MS_STACK_ELEM_NVRAM_CLASSMARK,					///<Element type {::NVRAMClassmark_t}, indicates the nvram classmark stored in stack	
	MS_STACK_ELEM_DTX_STATUS,						///<Element type {::Boolean}, status of dtx, dtx can be enabled/disabled by upper layers
	MS_STACK_ELEM_CELL_BARR_STATUS,					///<Element type {::CellBarrChangeStatus_t}, used by upper layer to notify stack if it can camp on barred cells or not
	MS_STACK_ELEM_IS_DEDICATED_MODE,				///<Element type {::Boolean}, used by upper layer to know if UE is in dedicated mode (either CS or PS )												 
	MS_STACK_ELEM_CURR_TIMING_ADVANCE,				///<Element type {::UInt8}, used by upper layer to know the current timing advance from stack
	MS_STACK_ELEM_SPEECHCODEC_BITMAP,				///<Element type {::MS_SpeechCodec_t}, used by upper layer to set the speech codecs supported as set by the user
	MS_STACK_ELEM_CHANNELMODE_SUPPORTED,			///<Element type {::MSChannelMode_t}, used for retreiveing the channel mode used by CAPI and stack.
    MS_STACK_ELEM_NW_MEAS_RESULT,                   ///<Element type {::None. MNR is included in MSG_NW_MEAS_RESULT_IND

	//----- DRIVER Elements

	MS_DRIVER_ELEM_SPINNER_SLEEP_MODE,				///<Element type {::null}, SPINNER sleep mode 

	//----- Network Registration related elements 
	
	MS_LOCAL_NETREG_ELEM_ISGPRS_ALLOWED,			///<Element type {::Boolean}, indicates whether GPRS is allowed in the current plmn
	MS_LOCAL_NETREG_ELEM_SYSTEM_RAT,				///<Element type {::RATSelect_t}, returns the system RAT (obtained from sysparm on initialization)
	MS_LOCAL_NETREG_ELEM_SUPPORTED_RAT,				///<Element type {::RATSelect_t}, returns the RAT supported by UE ( user configurable)
	MS_LOCAL_NETREG_ELEM_SYSTEM_BAND,				///<Element type {::BandSelect_t}, returns the system Band not configurable			
	MS_LOCAL_NETREG_ELEM_SUPPORTED_BAND,			///<Element type {::BandSelect_t}, returns the supported Band (user configurable)		
	MS_LOCAL_NETREG_ELEM_ISREGISTERED_GSM_OR_GPRS,  ///<Element type {::Boolean}, indicates whether we are registered to either GSM or GPRS
	MS_LOCAL_NETREG_ELEM_GET_MS_CLASS,				///<Element type {::MSClass_t}, returns msclass that is user configured

	MS_LOCAL_NETREG_ELEM_IS_REGISTER_IN_PROGRESS,	///<Element type {::Boolean}, returns  TRUE if we are in the middle of Attach for CS or PS
	MS_LOCAL_NETREG_ELEM_IS_DEREGISTER_IN_PROGRESS, ///<Element type {::Boolean}, returns  TRUE if we are in the middle of detach for either CS or PS
	MS_LOCAL_NETREG_ELEM_GPRSATTACH_STATUS,			///<Element type {::AttachState_t}, returns  ATTACHED if we are PS attached else returns DETACHED	
	MS_LOCAL_NETREG_ELEM_IS_GPRS_DETACHED,          ///<Element type {::Boolean}, returns TRUE if we have GPRS service detached,i.e. never send ATTACH_REQ for PS service or have detached from it. 
													///< Only if this element returns TRUE shall client activate PDP Ctx on the other VM because only one VM can be PS attached for dual-sim build. 

	MS_LOCAL_NETREG_ELEM_MANUAL_PLMN_SELECT_RAT,	///<Element type {::UInt8}, returns the RAT selected by the user for manual plmn selection.
	
	//-----Network Elements

	MS_NETWORK_ELEM_DTM,							///<Element type {::MSNetAccess_t}, DTM     
	MS_NETWORK_ELEM_HSDPA_SUPPORT,					///<Element type {::MSNetAccess_t}, HSDPA_SUPPORT  
	MS_NETWORK_ELEM_HSUPA_SUPPORT,				   	///<Element type {::MSNetAccess_t}, HSUPA_SUPPORT  
	MS_NETWORK_ELEM_GPRS_SUPPORT,					///<Element type {::MSNetAccess_t}, GPRS SUPPORT.
	MS_NETWORK_ELEM_EGPRS_SUPPORT,					///<Element type {::MSNetAccess_t}, EGPRS_SUPPORT.
	MS_NETWORK_ELEM_GAN_SUPPORT,					///<Element type {::MSNetAccess_t}, GAN_SUPPORT.
 	MS_NETWORK_ELEM_NOM,							///<Element type {::MSNwOperationMode_t}, Network operation mode of the network the UE is camped on  
	MS_NETWORK_ELEM_NW_TYPE,						///<Element type {::MSNwType_t},  TYPE of PLMN that the UE is camped on  
	MS_NETWORK_ELEM_RAC,							///<Element type {::UInt8},  RAC of the routing area that the UE is in.  
	MS_NETWORK_ELEM_RNC,			    		 	///<Element type {::UInt16},  RNC (Radio Network Controller) the UE is in.  
	MS_NETWORK_ELEM_GSMREG_STATE,					///<Element type {::MSRegState_t},  The current mobile gsm registration state    
	MS_NETWORK_ELEM_GPRSREG_STATE,  				///<Element type {::MSRegState_t},  The current mobile gprs registration state  
	MS_NETWORK_ELEM_GSM_CAUSE,						///<Element type {::NetworkCause_t},  The cause of gsm registration state  
	MS_NETWORK_ELEM_GPRS_CAUSE,	 					///<Element type {::NetworkCause_t},  GPRS registration state cause 
	MS_NETWORK_ELEM_NET_INFO,						///<Element type {::MSNetworkInfo_t},  Network information type refer MSNetworkInfo_t  
	MS_NETWORK_ELEM_GSM_NW_CAUSE,			 		///<Element type {::UInt8},  the GSM registration cause sent by network  
	MS_NETWORK_ELEM_GPRS_NW_CAUSE,					///<Element type {::UInt8},  the GPRS registratio cause sent by n/w 
	MS_NETWORK_ELEM_REG_INFO,						///<Element type {::MsState_t},  refer MSRegInfo_t used to send back to user  
	MS_NETWORK_ELEM_REGSTATE_INFO,					///<Element type {::MSRegStateInfo_t},  this provides the elements in MSRegStateInfo_t  
	MS_NETWORK_ELEM_ISGSM_REGISTERED,				///<Element type {::Boolean},  this indicates whether we GSM registered or not  
	MS_NETWORK_ELEM_ISGPRS_REGISTERED,				///<Element type {::Boolean},  this indicates whether we are registered for GPRS 
	MS_NETWORK_ELEM_CSINVALID_SIM_MS_ME_STATUS,		///<Element type {::RegisterStatus_t},  Invalid SIM/MS/ME CS status  
	MS_NETWORK_ELEM_GPRSINVALID_SIM_MS_ME_STATUS,	///<Element type {::RegisterStatus_t},  Invalid SIM/MS/ME status for GPRS
	MS_NETWORK_ELEM_IS_UMTS_DCH_STATE,				///<Element type {::Boolean},  TRUE if UMTS is in Cell-DCH state
	MS_NETWORK_RADIO_STATUS,                        ///<Element type {::MS_Radio_Status_t}, Radio status (i.e. whether UE in TBF state)
	MS_NETWORK_ELEM_CURRENT_CAMPED_PLMN,			///<Element type {::PLMNId_t}, the current camped plmn, this is updated from the cell_info_ind sent by stack
													//								isForbidden element in this context is not valid and is not filled in.
	MS_NETWORK_ELEM_UARFCN_DL,						///<Element type {::UInt16},  the current uarfcn_DL value
	MS_NETWORK_ELEM_SPECIAL_CGSEND_MODE,			///<Element type {::Boolean},  FALSE if special CGSEND mode is off (Fast mode)
	MS_NETWORK_ELEM_GPRS_CALL_ACTIVE_BY_CHAN,		///<Element type {::MS_ElemGprsParam_t}, in param: MS_ElemGprsParam_t.chan
	MS_NETWORK_ELEM_GPRS_CID_BY_CHAN,				///<Element type {::MS_ElemGprsParam_t}, in param: MS_ElemGprsParam_t.chan
	MS_NETWORK_ELEM_GPRS_CALL_ACTIVE_BY_CID,		///<Element type {::MS_ElemGprsParam_t}, ReadOnly, in param: MS_ElemGprsParam_t.cid.
	
	//-----SIM Elements

	MS_SIM_ELEM_PIN1_STATUS,                        ///<Element type {::CHVStatus_t},  SIM PIN1 Status
	MS_SIM_ELEM_PIN2_STATUS,                        ///<Element type {::CHVStatus_t},  SIM PIN2 Status
	MS_SIM_ELEM_ICC_ID,                             ///<Element type {::ICCID_ASCII_t},  SIM ICC ID
	MS_SIM_ELEM_RAW_ICC_ID,                         ///<Element type {::ICCID_BCD_t},  Raw SIM ICC ID
	MS_SIM_ELEM_SST,                                ///<Element type {::SIMDb_t},  SIM Service Table
	MS_SIM_ELEM_CPHS_ONS,                           ///<Element type {::UInt8[]},  CPHS Operator Name String
	MS_SIM_ELEM_CPHS_ONS_LEN,                       ///<Element type {::UInt8},  CPHS ONS Length
	MS_SIM_ELEM_CPHS_ONSS,                          ///<Element type {::UInt8[]},  CPHS Short Operator Name String
	MS_SIM_ELEM_CPHS_ONSS_LEN,                      ///<Element type {::UInt8},  CPHS ONSS Length
	MS_SIM_ELEM_CPHS_INFO,                          ///<Element type {::UInt8[CPHS_INFO_SIZE]},  CPHS Info
	MS_SIM_ELEM_ECC_REC_LIST,                       ///<Element type {::ECC_REC_LIST_t},  ECC List
	MS_SIM_ELEM_GID1_LEN,                           ///< GID1 Length
    MS_SIM_ELEM_GID2_LEN,                           ///< GID2 Length
	MS_SIM_ELEM_RUIM_SUPPORT,                       ///< MS_SIM_ELEM_RUIM_SUPPORT

	//------Configuration Elements
	MS_CFG_ELEM_SIM_LOCK_SUPPORTED,					///<Element type {::UInt8}, TRUE if Broadcom platform supports the SIMLOCK feature
	MS_CFG_ELEM_SIM_CINGULAR_ENS_ENABLED,			///<Element type {::UInt8}, TRUE if SIM supports the Cingular ENS feature in powerup sequence
	MS_CFG_ELEM_SKT_AUTO_SEND_CONFIRMATION,         ///<Element type {::UInt8}, TRUE if STK automatically sends Success Terminal Response for STK Refresh proactive command.
	MS_CFG_ELEM_PLMN_NAME_FLAGS,					///<Element type {::UInt8}, Configurate PLMN for MS_GetPLMNNameByCode(), see also ::PLMN_NAME_EONS_ENABLED_BIT_MASK
	MS_CFG_ELEM_DUAL_SIM_SUPPORTED, 				///<Element type {::UInt8}, TRUE if Broadcom platform supports the DUAL SIM feature
	MS_CFG_ELEM_HOMEZONE_SUPPORTED,					///<Element type {::UInt8}, TRUE if Broadcom platform supports the German O2 HomeZone feature; FALSE otherwise

	//------GPRS Data
    
	
    //-----NV Elements

    MS_NV_LOCAL_EQUI_PLMN_LIST,						///< Equi PLMN List
    MS_NV_LOCAL_NETPAR_INFO,						///< Net Par
    MS_NV_LOCAL_NITZ_NETWORK_NAME,					///< NW Name
    MS_NV_LOCAL_RPLMN_RAT,							///< LAST PLMN RAT
	MS_NV_LOCAL_SMS_DB,								///< SMS preference
	MS_NV_LOCAL_LAST_GOOD_BAND,						///< Last good band
	MS_NV_LOCAL_LOCK_ARFCN,							///lock arfcn
    MS_NV_LOCAL_STORED_PS_PARAMS,					///< Stored Stack Params
	MS_MAX_ELEMENT_TYPE								///< introduce new element type before value

} MS_Element_t;	///< MS Element Type 


typedef struct
{
	MS_Element_t elementType;		///< Database Element Type
} MS_LocalElemNotifyInd_t;			///< MS Database Element Notify Indicaiton


typedef struct
{
	UInt8		chan;				///< channel ID
	UInt8		cid;				///< cid hs been used if GPRS call is active
	Boolean		gprsCallActive;		///< Flag indicates GPRS call is active in ATC.
} MS_ElemGprsParam_t;


#define RX_SIGNAL_INFO_UNKNOWN 0xFF

typedef struct 
{
	Boolean				gprsCallActive;		//Flag indicates GPRS call is active in ATC.
	UInt8				cid;				//cid hs been used if GPRS call is active.
}MsGprsData_t;




#endif	//_MS_DATABASE_DEF_H_

