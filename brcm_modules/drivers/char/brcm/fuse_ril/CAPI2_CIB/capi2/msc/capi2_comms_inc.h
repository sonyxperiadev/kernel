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
#include "mobcom_types.h"
#include "resultcode.h"
#include "common_defs.h"
#include "uelbs_api.h"
#include "ms_database_def.h"
#include "common_sim.h"
#include "sim_def.h"
#include "capi2_taskmsgs.h"
#include "capi2_mstypes.h"
#include "capi_emulator_int.h"
#include <string.h>
#include "capi2_phonectrl_api.h"
#include "assert.h"
#include "sysparm.h"
#include "capi2_sim_api.h"

#include "capi2_sms_api.h"
#include "capi2_cc_api.h"
#include "capi2_lcs_cplane_api.h"
#include "capi2_ss_api.h"
#include "capi2_phonebk_api.h"
#include "capi2_gen_msg.h"
#include "capi2_cmd_resp.h"

#include "mobcom_types.h"
#include "resultcode.h"
#include "common_defs.h"
#include "capi2_reqrep.h"
#include "system_api.h"
#include "engmode_api.h"
#include "sysparm.h"
#include "cc_api.h"
#include "taskmsgs.h"
#include "ms_database_old.h"
#include "ms_database_api.h"
#include "netreg_util.h"
#include "netreg_api.h"
#include "ss_api.h"
#include "sms_api_atc.h"
#include "sms_api.h"
#include "cc_api_old.h"
#include "sim_api.h"
#include "capi2_comm.h"
#include "phonebk_api.h"
#include "phonectrl_api.h"
#include "system_api.h"

#include "isim_def.h"
#include "isim_api_old.h"
#include "isim_api.h"

#include "util_api.h"
#include "dialstr_api.h"
#include "stk_api.h"
#include "stk_api_old.h"

#include "pch_def.h"
#include "pch_api.h"
#include "pchex_def.h"
#include "pchex_api.h"

#include "ss_api_old.h"

#include "lcs_cplane_rrlp_api.h"


extern Boolean SIM_IsPbkAccessAllowed(		// returns if access to Phonebook is allowed
	SIMPBK_ID_t id					// Phonebook in question
	);

extern UInt16	SYSPARM_GetSysparmIndPartFileVersion(void);
extern void 			SYS_GetBootLoaderVersion(UInt8* BootLoaderVersion, UInt8 BootLoaderVersion_sz);
extern void			SYS_GetDSFVersion(UInt8* DSFVersion, UInt8 DSFVersion_sz);
Result_t SMS_ConfigureMEStorage(Boolean flag);
void SIMIO_DeactiveCard(void);
const UInt8* PATCH_GetRevision( void );
extern Result_t SIM_SubmitDFileInfoReqOld(UInt8 clientID, UInt8 socket_id, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* cb);
void CAPI2_DataStateCb(
	ClientInfo_t			*clientInfoPtr,
	PCHResponseType_t		response,
	PCHNsapi_t				nsapi,
	PCHPDPType_t			pdpType,
	PCHPDPAddress_t			pdpAddress,
	Result_t				cause
	);

typedef void (*ReturnValue_t)(			// Callback to return BADR value
	UInt16 badr_value,					// Returned BADR value
	UInt32 Context						// Context to track original caller task
	);

extern void cbk_ADCMGR_Start(UInt16 value, UInt32 Context );
UInt16 ADCMGR_Start(                      // Start ADC measurement
    UInt16 init_value,                  // BMR value to start measurement
    ReturnValue_t return_value_cb       // Callback function to return BADR value
    );                                   // Return: TRUE, if request submission succeeded

////////////////////////////////////////////////
Boolean SimApi_IsPbkAccessAllowed(ClientInfo_t* inClientInfoPtr, SIMPBK_ID_t id);
Boolean SimApi_IsPINRequired(ClientInfo_t* inClientInfoPtr);		
SIMAccess_t SimApi_GetAtrData(ClientInfo_t* inClientInfoPtr, APDU_t *atr_data);
