/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
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
#include "bcm_kril_common.h"
#include "bcm_kril_capi2_handler.h"
#include "bcm_kril_cmd_handler.h"
#include "bcm_kril_ioctl.h"
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
#include "capi2_pch_api.h"
#include "capi2_reqrep.h"
#include "capi2_gen_api.h"
#endif // CONFIG_BRCM_FUSE_RIL_CIB

#define FILTER_GSM_SIGNAL_LEVEL 16

extern MSRegInfo_t  gRegInfo;
extern MSUe3gStatusInd_t  gUE3GInfo;
extern KrilDataCallResponse_t pdp_resp[BCM_NET_MAX_PDP_CNTXS];

// The cause of register denied
static const UInt8 g_DeniedCause[] =
{
    -1,
    23,
    2,
    3,
    6,
    11,
    12,
    13,
    15,
    17
};
#define NUM_DENIEDCAUSE (sizeof(g_DeniedCause) / sizeof(int))

// Support Band Mode for System
static const UInt32 g_bandMode[] =
{
    BAND_NULL, // "unspecified"
    BAND_NULL, // "EURO band" (GSM-900 / DCS-1800 / WCDMA-IMT-2000)
    BAND_GSM850_ONLY|BAND_PCS1900_ONLY|BAND_UMTS850_ONLY|BAND_UMTS1900_ONLY, //"US band"
    BAND_NULL, // "JPN band" (WCDMA-800 / WCDMA-IMT-2000)
    BAND_NULL,  // "AUS band" (GSM-900 / DCS-1800 / WCDMA-850 / WCDMA-IMT-2000)
    BAND_GSM850_ONLY|BAND_PCS1900_ONLY|BAND_UMTS850_ONLY, //"AUS band 2"
    BAND_NULL,  // "Cellular (800-MHz Band)"
    BAND_PCS1900_ONLY, // "PCS (1900-MHz Band)"
    BAND_NULL, // "Band Class 3 (JTACS Band)"
    BAND_NULL, // "Band Class 4 (Korean PCS Band)"(Tx = 1750-1780MHz)
    BAND_NULL, // "Band Class 5 (450-MHz Band)"
    BAND_NULL, // "Band Class 6 (2-GMHz IMT2000 Band)"
    BAND_NULL, // "Band Class 7 (Upper 700-MHz Band)"
    BAND_DCS1800_ONLY, // "Band Class 8 (1800-MHz Band)"
    BAND_GSM900_ONLY, // "Band Class 9 (900-MHz Band)"
    BAND_NULL, // "Band Class 10 (Secondary 800-MHz Band)"
    BAND_NULL, // "Band Class 11 (400-MHz European PAMR Band)"
    BAND_NULL, // "Band Class 15 (AWS Band)"
    BAND_NULL, // "Band Class 16 (US 2.5-GHz Band)"
};
#define NUM_BANDMODE (sizeof(g_bandMode) / sizeof(UInt32))

//******************************************************************************
// Function Name: MS_PlmnConvertRawMcc	
//
// Description: This function converts the MCC values: 
// For example: from 0x1300 to 0x310 for Pacific Bell.
//******************************************************************************
UInt16 MS_PlmnConvertRawMcc(UInt16 mcc)
{
    return ( (mcc & 0x0F00) | ((mcc & 0xF000) >> 8) | (mcc & 0x000F) );
}

//******************************************************************************
// Function Name: MS_PlmnConvertRawMnc	
//
// Description: This function converts the MNC values. 
// The third digit of MNC may be stored in the third nibble of the passed MCC.
// For example: the passed MCC is 0x1300 and passed MNC is 0x0071 for Pacific Bell.
// This function returns the converted MNC value as 0x170
//******************************************************************************
UInt16 MS_PlmnConvertRawMnc(UInt16 mcc, UInt16 mnc)
{
    UInt16 converted_mcc;

    if( (mcc & 0x00F0) == 0x00F0 )
    {
        converted_mcc = MS_PlmnConvertRawMcc(mcc);

        if( (converted_mcc == 0x302) ||  ((converted_mcc >= 0x0310) && (converted_mcc <= 0x0316)) )
        {
            /* This is a North America PCS network, the third MNC digit is 0: see Annex A of GSM 03.22.
             * Canadian PCS networks (whose MCC is 0x302) also use 3-digit MNC, even though GSM 03.22 does
             * not specify it.
             */
            return ( (mnc & 0x00F0) | ((mnc & 0x000F) << 8) );
        }
        else
        {
            /* Two digit MNC */
            return ( ((mnc & 0x00F0) >> 4) | ((mnc & 0x000F) << 4) );
        }
    }
    else
    {
        /* Three digit MNC */
        return ( (mnc & 0x00F0) | ((mnc & 0x000F) << 8) | ((mcc & 0x00F0) >> 4) );
    }
}

//******************************************************************************
// Function Name: MS_ConvertRawPlmnToHer	
//
// Description: 
//******************************************************************************
void MS_ConvertRawPlmnToHer(UInt16 mcc, UInt16 mnc, UInt8 *plmn_str)
{
    unsigned short mcc_code = MS_PlmnConvertRawMcc( mcc );
    unsigned short mnc_code = MS_PlmnConvertRawMnc( mcc, mnc );

    if( (mcc & 0x00F0) != 0x00F0 ) {
        sprintf((char *) plmn_str, "%03X%03X", mcc_code, mnc_code);
    }
    else {
        sprintf((char *) plmn_str, "%03X%02X", mcc_code, mnc_code);
    }
}


void KRIL_SignalStrengthHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilSignalStrength_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KrilSignalStrength_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
            KRIL_DEBUG(DBG_TRACE, "handler state:%lu\n", pdata->handler_state);
            CAPI2_MS_GetCurrentRAT(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_MS_GetCurrentRAT;
        }
        break;

        case BCM_MS_GetCurrentRAT:
        {
            UInt8 presult = *((UInt8 *) capi2_rsp->dataBuf);
            KrilSignalStrength_t *rdata = (KrilSignalStrength_t *)pdata->bcm_ril_rsp;
            KRIL_DEBUG(DBG_INFO, "presult:%d\n", presult);
            rdata->RAT = presult;
            gRegInfo.netInfo.rat= rdata->RAT;
            CAPI2_SYS_GetRxSignalInfo(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            KRIL_DEBUG(DBG_INFO, "result:0x%x\n", capi2_rsp->result);
            if(RESULT_OK != capi2_rsp->result)
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }

            if(NULL != capi2_rsp->dataBuf)
            {
                MsRxLevelData_t* presult = (MsRxLevelData_t*) capi2_rsp->dataBuf;
                KrilSignalStrength_t *rdata = (KrilSignalStrength_t *)pdata->bcm_ril_rsp;

                rdata->RxLev = presult->RxLev;
                rdata->RxQual = presult->RxQual;
                KRIL_DEBUG(DBG_INFO, "rdata->RAT:%d RxLev:%d RxQual:%d\n", rdata->RAT, rdata->RxLev, rdata->RxQual);
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_RegistationStateHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilRegState_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KrilRegState_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);

            KRIL_DEBUG(DBG_TRACE, "handler state:%lu\n", pdata->handler_state);
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
            CAPI2_MS_GetElement(GetNewTID(), GetClientID(), MS_NETWORK_ELEM_REGSTATE_INFO);
#else
            CAPI2_MS_GetRegistrationInfo(GetNewTID(), GetClientID());
#endif
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            if(capi2_rsp->result != RESULT_OK)
            {
                KRIL_DEBUG(DBG_ERROR, "Error result:0x%x\n", capi2_rsp->result);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }

            if(NULL != capi2_rsp->dataBuf)
            {
                KrilRegState_t *rdata = (KrilRegState_t *)pdata->bcm_ril_rsp;
                MSRegStateInfo_t* presult = NULL; 
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
                CAPI2_MS_Element_t* rsp = (CAPI2_MS_Element_t*)capi2_rsp->dataBuf;
                presult = (MSRegStateInfo_t*)(&(rsp->data_u));
#else
                presult = (MSRegStateInfo_t*) capi2_rsp->dataBuf;
#endif

                rdata->gsm_reg_state = presult->gsm_reg_state;
                rdata->gprs_reg_state = presult->gprs_reg_state;
                rdata->mcc = presult->mcc;
                rdata->mnc = presult->mnc;
                rdata->band = presult->band;
                rdata->lac = presult->lac;
                rdata->cell_id = presult->cell_id;

                if (presult->gsm_reg_state != REG_STATE_NORMAL_SERVICE && presult->gsm_reg_state != REG_STATE_ROAMING_SERVICE &&
                    presult->gprs_reg_state != REG_STATE_NORMAL_SERVICE && presult->gprs_reg_state != REG_STATE_ROAMING_SERVICE)
                {
                    rdata->network_type = 0; //Unknown
                }
                else if (presult->rat == RAT_UMTS)
                {
                    if (SUPPORTED == gRegInfo.netInfo.hsdpa_supported ||
                        TRUE == gUE3GInfo.in_uas_conn_info.hsdpa_ch_allocated)
                    {
                        rdata->network_type = 9; //HSDPA
                    }
                    else if (SUPPORTED == gRegInfo.netInfo.hsupa_supported)
                    {
                        rdata->network_type = 10; //HSUPA
                    }
                    else
                    {
                        rdata->network_type = 3; //UMTS
                    }
                }
                else if (presult->rat == RAT_GSM)
                {
                    if (SUPPORTED == gRegInfo.netInfo.egprs_supported)
                    {
                        rdata->network_type = 2; //EDGE
                    }
                    else
                    {
                        rdata->network_type = 1; //GPRS
                    }
                }
                else
                {
                    rdata->network_type = 0; //Unknown
                }

                if(REG_STATE_LIMITED_SERVICE == presult->gsm_reg_state) // set denied cause
                {
                    UInt8 i;
                    rdata->cause = 0;
                    for(i = 0 ; i < NUM_DENIEDCAUSE ; i++)
                    {
                        if(g_DeniedCause[i] == gRegInfo.netCause)
                        {
                            rdata->cause = i;
                            break;
                        }
                    }
                }
                KRIL_DEBUG(DBG_INFO, "regstate:%d gprs_reg_state:%d mcc:ox%x mnc:0x%x rat:%d lac:%d cell_id:%d network_type:%d band:%d\n", rdata->gsm_reg_state, rdata->gprs_reg_state, rdata->mcc, rdata->mnc, presult->rat, rdata->lac, rdata->cell_id, rdata->network_type, rdata->band);
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_OperatorHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilOperatorInfo_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KrilOperatorInfo_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
            CAPI2_MS_GetElement(GetNewTID(), GetClientID(), MS_NETWORK_ELEM_REGSTATE_INFO);
#else
            CAPI2_MS_GetRegistrationInfo(GetNewTID(), GetClientID());
#endif
            pdata->handler_state = BCM_MS_GetRegistrationInfo;
        }
        break;

        case BCM_MS_GetRegistrationInfo:
        {
            if(RESULT_OK != capi2_rsp->result)
            {
                KRIL_DEBUG(DBG_INFO, "result:0x%x\n", capi2_rsp->result);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
			else
			{
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
                CAPI2_MS_Element_t* rsp = (CAPI2_MS_Element_t*)capi2_rsp->dataBuf;
                MSRegStateInfo_t* presult = NULL; 
                if ( rsp && (rsp->inElemType == MS_NETWORK_ELEM_REGSTATE_INFO) )
                {
                    presult = (MSRegStateInfo_t*)(&(rsp->data_u));
                }
                else
                {
                    KRIL_DEBUG(DBG_ERROR,"unexpected response retrieving CAPI2_MS_GetElement !!\n");
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    break;
                }
#else
                MSRegStateInfo_t* presult = (MSRegStateInfo_t*) capi2_rsp->dataBuf;
#endif
                if (presult->gsm_reg_state != REG_STATE_NORMAL_SERVICE && presult->gsm_reg_state != REG_STATE_ROAMING_SERVICE)
                {
                    pdata->result = RIL_E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW;
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    return;
                }
			    else
			    {
                    KrilOperatorInfo_t *rdata = (KrilOperatorInfo_t *)pdata->bcm_ril_rsp;
                    unsigned char oper_str[7];
                    KRIL_DEBUG(DBG_INFO, "mcc:%d mnc:%d lac:%d\n",presult->mcc, presult->mnc, presult->lac);
                    MS_ConvertRawPlmnToHer(presult->mcc, presult->mnc, oper_str);
                    strcpy(rdata->numeric, oper_str);
                    CAPI2_MS_GetPLMNNameByCode(GetNewTID(), GetClientID(), presult->mcc, presult->mnc, presult->lac, FALSE);
                    pdata->handler_state = BCM_RESPCAPI2Cmd;
			    }
            }
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            if ( MSG_MS_PLMN_NAME_RSP == capi2_rsp->msgType )
            {
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
                CAPI2_NetRegApi_GetPLMNNameByCode_Rsp_t* nameResult = (CAPI2_NetRegApi_GetPLMNNameByCode_Rsp_t*)capi2_rsp->dataBuf;
                if ( capi2_rsp->dataLength == sizeof(CAPI2_NetRegApi_GetPLMNNameByCode_Rsp_t) )
                {
                    if ( nameResult->val )
                    {
                        KrilOperatorInfo_t *rdata = (KrilOperatorInfo_t *)pdata->bcm_ril_rsp;
                        KRIL_DEBUG(DBG_INFO, "longName:%s size %d shortName:%s %d\n", nameResult->long_name.name,nameResult->long_name.name_size, nameResult->short_name.name,nameResult->short_name.name_size);
                        memcpy(rdata->longname, nameResult->long_name.name, (nameResult->long_name.name_size < PLMN_LONG_NAME)?nameResult->long_name.name_size:PLMN_LONG_NAME );
                        memcpy(rdata->shortname, nameResult->short_name.name, (nameResult->short_name.name_size < PLMN_SHORT_NAME)?nameResult->short_name.name_size:PLMN_SHORT_NAME);
                        pdata->handler_state = BCM_FinishCAPI2Cmd;
                    }
                    else
                    {
                        pdata->handler_state = BCM_ErrorCAPI2Cmd;
                        KRIL_DEBUG(DBG_ERROR, "CAPI2_MS_GetPLMNNameByCode result FALSE\n");
                    }
                }
#else
                MsPlmnName_t* presult = (MsPlmnName_t*) capi2_rsp->dataBuf;
                if ( capi2_rsp->dataLength == sizeof(MsPlmnName_t) )
                {                    
                    KrilOperatorInfo_t *rdata = (KrilOperatorInfo_t *)pdata->bcm_ril_rsp;
                    KRIL_DEBUG(DBG_INFO, "longName:%s shortName:%s\n", presult->longName.name, presult->shortName.name);
                    memcpy(rdata->longname, presult->longName.name, presult->longName.name_size);
                    memcpy(rdata->shortname, presult->shortName.name, presult->shortName.name_size);
                    pdata->handler_state = BCM_FinishCAPI2Cmd;
                }
#endif                
                else
                {
                    KRIL_DEBUG(DBG_ERROR, "** MSG_MS_PLMN_NAME_RSP size mismatch got %d expected %d\n", capi2_rsp->dataLength, sizeof(MsPlmnName_t) );
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                }
            }
            else
            {
                KRIL_DEBUG(DBG_ERROR, "Unexpected response message for CAPI2_MS_GetPLMNNameByCode 0x%x tid %d\n",capi2_rsp->msgType, capi2_rsp->tid);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}

void KRIL_QueryNetworkSelectionModeHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
             pdata->bcm_ril_rsp = kmalloc(sizeof(KrilSelectionMode_t), GFP_KERNEL);
             pdata->rsp_len = sizeof(KrilSelectionMode_t);
             memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
             CAPI2_MS_GetPlmnMode(GetNewTID(), GetClientID());
             pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            PlmnSelectMode_t presult = *(PlmnSelectMode_t *) capi2_rsp->dataBuf;
            KrilSelectionMode_t *rdata = (KrilSelectionMode_t *)pdata->bcm_ril_rsp;
            rdata->selection_mode = presult;
            KRIL_DEBUG(DBG_INFO, "selection_mode:%d PlmnSelectMode:%d\n", rdata->selection_mode, presult);
            pdata->handler_state = BCM_FinishCAPI2Cmd;
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_SetNetworkSelectionAutomaticHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            CAPI2_MS_PlmnSelect(GetNewTID(), GetClientID(), FALSE, PLMN_SELECT_AUTO, PLMN_FORMAT_LONG, "");
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            if (capi2_rsp->result != RESULT_OK)
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                if (capi2_rsp->dataBuf != NULL)
                {
                    UInt16 presult = *((UInt16*) capi2_rsp->dataBuf);
                    KRIL_DEBUG(DBG_INFO, "presult:%d\n", presult);
                }
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_SetNetworkSelectionManualHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KrilManualSelectInfo_t *tdata = (KrilManualSelectInfo_t *)pdata->ril_cmd->data;
            KRIL_DEBUG(DBG_INFO, "network_info:%s\n", tdata->networkInfo);
            CAPI2_MS_PlmnSelect(GetNewTID(), GetClientID(), FALSE, PLMN_SELECT_MANUAL, PLMN_FORMAT_NUMERIC, (char *)tdata->networkInfo);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            if (RESULT_PLMN_SELECT_OK == capi2_rsp->result) // for select the same network
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            else if (capi2_rsp->result != RESULT_OK)
            {
                pdata->result = RILErrorResult(capi2_rsp->result);
                CAPI2_MS_AbortPlmnSelect(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_MS_AbortPlmnSelect;
            }
            else
            {
                UInt16 presult = *((UInt16*) capi2_rsp->dataBuf);
                KRIL_DEBUG(DBG_INFO, "presult:%d\n", presult);
                if(OPERATION_SUCCEED == presult || NO_REJECTION == presult) //presult = OPERATION_SUCCEED ; PlmnSelect:Already in Auto Mode.Ignore.
                {
                    pdata->handler_state = BCM_FinishCAPI2Cmd;
                }
                else
                {
                    CAPI2_MS_AbortPlmnSelect(GetNewTID(), GetClientID());
                    pdata->handler_state = BCM_MS_AbortPlmnSelect;
                }
            }
        }
        break;

        case BCM_MS_AbortPlmnSelect:
        {
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_QueryAvailableNetworksHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilNetworkList_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KrilNetworkList_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
            CAPI2_MS_SearchAvailablePLMN(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            SEARCHED_PLMN_LIST_t *rsp = (SEARCHED_PLMN_LIST_t *)capi2_rsp->dataBuf;
            UInt8 i, j;
            Boolean match = FALSE;
            KrilNetworkList_t *rdata = (KrilNetworkList_t *)pdata->bcm_ril_rsp;
            rdata->num_of_plmn = 0;
            KRIL_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd :: num_of_plmn:%d\n", rsp->num_of_plmn);
            for(i = 0 ; i < rsp->num_of_plmn ; i++)
            {
                for (j = 0 ; j < i ; j++)
                {
                     KRIL_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd :: mcc[%d]:%d mcc[%d]:%d\n", i, rsp->searched_plmn[i].mcc, j, rsp->searched_plmn[j].mcc);
                     KRIL_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd :: mnc[%d]:%d mnc[%d]:%d\n", i, rsp->searched_plmn[i].mnc, j, rsp->searched_plmn[j].mnc);
                     if ((rsp->searched_plmn[i].mcc == rsp->searched_plmn[j].mcc) &&
                     	    (rsp->searched_plmn[i].mnc == rsp->searched_plmn[j].mnc))
                     {
                         match = TRUE;
                         break;
                     }
                     match = FALSE;
                }
                if (TRUE == match)
                {
                    match = FALSE;
                    continue;
                }
                rdata->available_plmn[rdata->num_of_plmn].mcc = rsp->searched_plmn[i].mcc;
                rdata->available_plmn[rdata->num_of_plmn].mnc = rsp->searched_plmn[i].mnc;
                rdata->available_plmn[rdata->num_of_plmn].network_type = rsp->searched_plmn[i].network_type;
                rdata->available_plmn[rdata->num_of_plmn].rat = rsp->searched_plmn[i].rat;
                strncpy(rdata->available_plmn[rdata->num_of_plmn].longname, rsp->searched_plmn[i].nonUcs2Name.longName.name, rsp->searched_plmn[i].nonUcs2Name.longName.name_size);
                strncpy(rdata->available_plmn[rdata->num_of_plmn].shortname, rsp->searched_plmn[i].nonUcs2Name.shortName.name, rsp->searched_plmn[i].nonUcs2Name.shortName.name_size);
                rdata->num_of_plmn++;
                KRIL_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd::num_of_plmn:%d network_type:%d longname:%s shortname:%s\n", rdata->num_of_plmn, rdata->available_plmn[rdata->num_of_plmn].network_type, rdata->available_plmn[rdata->num_of_plmn].longname, rdata->available_plmn[rdata->num_of_plmn].shortname);
            }
            pdata->handler_state = BCM_FinishCAPI2Cmd;
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_SetBandModeHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KrilBandModeInfo_t *tdata = (KrilBandModeInfo_t *)pdata->ril_cmd->data;
            CAPI2_SYS_SelectBand(GetNewTID(), GetClientID(),  tdata->bandmode);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            if (capi2_rsp->result != RESULT_OK)
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_QueryAvailableBandModeHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilAvailableBandMode_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KrilAvailableBandMode_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
            CAPI2_MS_GetSystemBand(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            if (capi2_rsp->result != RESULT_OK)
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                BandSelect_t SystemBand = *(BandSelect_t *) capi2_rsp->dataBuf;
                KrilAvailableBandMode_t *rdata = (KrilAvailableBandMode_t *)pdata->bcm_ril_rsp;
                int index, index1 = 0;
                int tempband[18];

                for (index = 0 ; index < NUM_BANDMODE ; index++)
                {
                    if(g_bandMode[index] & SystemBand)
                    {
                        tempband[index1] = index;
                        KRIL_DEBUG(DBG_INFO, "g_bandMode[%d]:%ld tempband[%d]:%d\n", index, g_bandMode[index], index1, tempband[index1]);
                        index1++;
                    }
                }
                rdata->band_mode[0] = index1;
                KRIL_DEBUG(DBG_INFO, "band_mode length:%d\n", rdata->band_mode[0]);
                for (index = 0 ; index < rdata->band_mode[0] ; index++)
                {
                    rdata->band_mode[index+1] = tempband[index];
                    KRIL_DEBUG(DBG_INFO, "rdata->band_mode[%d]:%d\n",index+1, rdata->band_mode[index+1]);
                }
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_SetPreferredNetworkTypeHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KrilSetPreferredNetworkType_t *tdata = (KrilSetPreferredNetworkType_t *)pdata->ril_cmd->data;
            KRIL_DEBUG(DBG_INFO, "BCM_SendCAPI2Cmd:: networktype:%d\n", tdata->networktype);
            if (tdata->networktype == KRIL_GetPreferredNetworkType())
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            else
            {
                if (KRIL_SetPreferredNetworkType(tdata->networktype) != TRUE)
                {
                     pdata->handler_state = BCM_ErrorCAPI2Cmd;
                     return;
                }
                pdata->handler_state = BCM_SYS_SelectBand;
                if (0 == tdata->networktype)
                    CAPI2_MS_SetSupportedRATandBand(GetNewTID(), GetClientID(), DUAL_MODE_UMTS_PREF, BAND_NULL);
                else if (1 == tdata->networktype)
                CAPI2_MS_SetSupportedRATandBand(GetNewTID(), GetClientID(), GSM_ONLY, BAND_NULL);
            else if (2 == tdata->networktype)
                CAPI2_MS_SetSupportedRATandBand(GetNewTID(), GetClientID(), UMTS_ONLY, BAND_NULL);
            else
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
        }
        break;

        case BCM_SYS_SelectBand:
        {
            UInt8 i;
            for (i=0; i<BCM_NET_MAX_PDP_CNTXS; i++)
            {
                KRIL_DEBUG(DBG_INFO, "msgType:0x%x pdp_resp[%d]:%d...!\n",capi2_rsp->msgType, i, pdp_resp[i].cid);
                if (pdp_resp[i].cid != 0)
                {
                    if (capi2_rsp->msgType == MSG_PDP_DEACTIVATION_RSP)
                    {
                        PDP_SendPDPDeactivateReq_Rsp_t *rsp = (PDP_SendPDPDeactivateReq_Rsp_t *)capi2_rsp->dataBuf;
                        if (rsp->response == PCH_REQ_ACCEPTED)
                        {
                            pdp_resp[i].cid = 0;
                            KRIL_SendNotify(RIL_UNSOL_DATA_CALL_LIST_CHANGED, &pdp_resp[i], sizeof(KrilDataCallResponse_t));
                        }
                        else // PDP deactive again if return fail
                        {
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
                            {
                                ClientInfo_t clientInfo;
                                CAPI2_InitClientInfo( &clientInfo, GetNewTID(), GetClientID());
                                CAPI2_PchExApi_SendPDPDeactivateReq( &clientInfo, pdp_resp[i].cid );
                            }
#else
                            CAPI2_PDP_SendPDPDeactivateReq(GetNewTID(), GetClientID(), pdp_resp[i].cid);
#endif
                            return;
                        }
                    }
                    else
                    {
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
                        {
                            ClientInfo_t clientInfo;
                            CAPI2_InitClientInfo( &clientInfo, GetNewTID(), GetClientID());
                            CAPI2_PchExApi_SendPDPDeactivateReq( &clientInfo, pdp_resp[i].cid );
                        }
#else
                    CAPI2_PDP_SendPDPDeactivateReq(GetNewTID(), GetClientID(), pdp_resp[i].cid);
#endif
                    return;
                    }
                }
            }
            CAPI2_SYS_SelectBand(GetNewTID(), GetClientID(), BAND_AUTO);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            if (capi2_rsp->result != RESULT_OK)
            {
                KRIL_DEBUG(DBG_ERROR, "handler result:%d error...!\n", capi2_rsp->result);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}



void KRIL_GetPreferredNetworkTypeHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            pdata->bcm_ril_rsp = kmalloc(sizeof(krilGetPreferredNetworkType_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(krilGetPreferredNetworkType_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
            CAPI2_MS_GetSupportedRAT(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            if (capi2_rsp->result != RESULT_OK)
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                RATSelect_t RAT = *(RATSelect_t *)capi2_rsp->dataBuf;
                krilGetPreferredNetworkType_t *rdata = (krilGetPreferredNetworkType_t *)pdata->bcm_ril_rsp;
                pdata->handler_state = BCM_FinishCAPI2Cmd;
                if(RAT == GSM_ONLY)
                {
                    rdata->network_type = 1;
                }
                else  if(RAT == UMTS_ONLY)
                {
                    rdata->network_type = 2;
                }
                else  if(RAT == DUAL_MODE_GSM_PREF || RAT == DUAL_MODE_UMTS_PREF)
                {
                    rdata->network_type = 0;
                }
                else
                {
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    return;
                }
                KRIL_SetPreferredNetworkType(rdata->network_type);
                KRIL_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd:: network_type:%d RAT:%d\n", rdata->network_type, RAT);
            }
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_GetNeighboringCellIDsHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
#ifndef CONFIG_BRCM_FUSE_RPC_2157SDB
		KRIL_DEBUG(DBG_INFO, "Wait for latest CPAI2 release...!\n");
		pdata->handler_state = BCM_FinishCAPI2Cmd;
#else
            CAPI2_SYS_EnableCellInfoMsg(GetNewTID(), GetClientID(), TRUE);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
#endif /* CONFIG_BRCM_FUSE_RPC_2157SDB */
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            if (capi2_rsp->result != RESULT_OK)
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
#ifndef CONFIG_BRCM_FUSE_RPC_2157SDB
		pdata->handler_state = BCM_FinishCAPI2Cmd;
#else
                if (MSG_SERVING_CELL_INFO_IND == capi2_rsp->msgType)
                {
                    ServingCellInfo_t *rsp = (ServingCellInfo_t *)capi2_rsp->dataBuf;
                    krilGetNeighborCell_t *rdata;
                    UInt8 loop = 0, count = 0;
                    void *temp_rsp;
                    KRIL_DEBUG(DBG_INFO, "mRAT:%d\n", rsp->mRAT);
                    if (RAT_UMTS == rsp->mRAT)
                    {
                        KRIL_DEBUG(DBG_INFO, "num_umts_freq:%d \n", rsp->mLbsParams.lbs_umts_params.num_umts_freq);
                        for (loop = 0 ; loop < rsp->mLbsParams.lbs_umts_params.num_umts_freq ; loop++)
                        {
                            count += rsp->mLbsParams.lbs_umts_params.umts_freqs[loop].num_cell;
                        }
                        KRIL_DEBUG(DBG_INFO, "count:%d \n", count);
                        temp_rsp = kmalloc(sizeof(krilGetNeighborCell_t)*count, GFP_KERNEL);
                        rdata = (krilGetNeighborCell_t *)temp_rsp;
                        memset(temp_rsp, 0,sizeof(krilGetNeighborCell_t)*count);
                        loop = 0;
                        count = 0;
                        for (loop = 0 ; loop < rsp->mLbsParams.lbs_umts_params.num_umts_freq ; loop++)
                        {
                            UInt8 loop1 = 0;
                            KRIL_DEBUG(DBG_INFO, "num_cell:%d\n", rsp->mLbsParams.lbs_umts_params.umts_freqs[loop].num_cell);
                            for (loop1 = 0 ; loop1 < rsp->mLbsParams.lbs_umts_params.umts_freqs[loop].num_cell ; loop1++)
                            {
                                if (rsp->mLbsParams.lbs_umts_params.umts_freqs[loop].cells[loop1].psc != 0 &&
                                    rsp->mLbsParams.lbs_umts_params.umts_freqs[loop].cells[loop1].rscp >= -121 &&
                                    rsp->mLbsParams.lbs_umts_params.umts_freqs[loop].cells[loop1].rscp <= -25) // 3G Received Signal Code Power Range: -121 to -25
                                {
                                    KRIL_DEBUG(DBG_INFO, "psc:0x%x rscp:%d\n", rsp->mLbsParams.lbs_umts_params.umts_freqs[loop].cells[loop1].psc, rsp->mLbsParams.lbs_umts_params.umts_freqs[loop].cells[loop1].rscp);
                                    rdata[count].cid = rsp->mLbsParams.lbs_umts_params.umts_freqs[loop].cells[loop1].psc;
                                    rdata[count].rssi = rsp->mLbsParams.lbs_umts_params.umts_freqs[loop].cells[loop1].rscp;
                                    KRIL_DEBUG(DBG_INFO, "loop:%d cid:0x%x rssi:%d\n", loop, rdata[count].cid, rdata[count].rssi);
                                    count++;
                                }
                            }
                        }
                        pdata->rsp_len = sizeof(krilGetNeighborCell_t)*count;
                        pdata->bcm_ril_rsp = kmalloc(pdata->rsp_len, GFP_KERNEL);
                        memcpy(pdata->bcm_ril_rsp, temp_rsp, pdata->rsp_len);
                        kfree(temp_rsp);
                    }
                    else if (RAT_GSM == rsp->mRAT)
                    {
                        temp_rsp = kmalloc(sizeof(krilGetNeighborCell_t)*rsp->mLbsParams.lbs_gsm_params.num_gsm_ncells, GFP_KERNEL);
                        KRIL_DEBUG(DBG_INFO, "num_gsm_ncells:%d \n", rsp->mLbsParams.lbs_gsm_params.num_gsm_ncells);
                        rdata = (krilGetNeighborCell_t *)temp_rsp;
                        memset(temp_rsp, 0,sizeof(krilGetNeighborCell_t)*rsp->mLbsParams.lbs_gsm_params.num_gsm_ncells);
                        count = 0;
                        for (loop = 0 ; loop < rsp->mLbsParams.lbs_gsm_params.num_gsm_ncells ; loop++)
                        {
                            if (rsp->mLbsParams.lbs_gsm_params.gsm_ncells[loop].lac != 0 &&
                                rsp->mLbsParams.lbs_gsm_params.gsm_ncells[loop].cell_id != 0 &&
                                rsp->mLbsParams.lbs_gsm_params.gsm_ncells[loop].rxlev >= FILTER_GSM_SIGNAL_LEVEL &&
                                rsp->mLbsParams.lbs_gsm_params.gsm_ncells[loop].rxlev <= 63) //2G receive power level Valid values: 0 to 63, and also filter lower signal level
                            {
                                KRIL_DEBUG(DBG_INFO, "lac:0x%x cell_id:0x%x rxlev:%d\n", rsp->mLbsParams.lbs_gsm_params.gsm_ncells[loop].lac, rsp->mLbsParams.lbs_gsm_params.gsm_ncells[loop].cell_id, rsp->mLbsParams.lbs_gsm_params.gsm_ncells[loop].rxlev);
                                rdata[count].cid = (rsp->mLbsParams.lbs_gsm_params.gsm_ncells[loop].lac << 16) | rsp->mLbsParams.lbs_gsm_params.gsm_ncells[loop].cell_id;
                                rdata[count].rssi = rsp->mLbsParams.lbs_gsm_params.gsm_ncells[loop].rxlev;
                                KRIL_DEBUG(DBG_INFO, "loop:%d count:%d cid:0x%x rssi:%d\n", loop, count, rdata[count].cid, rdata[count].rssi);
                                count++;
                            }
                        }
                        pdata->rsp_len = sizeof(krilGetNeighborCell_t)*count;
                        pdata->bcm_ril_rsp = kmalloc(pdata->rsp_len, GFP_KERNEL);
                        memcpy(pdata->bcm_ril_rsp, temp_rsp, pdata->rsp_len);
                        kfree(temp_rsp);
                    }
                    else
                    {
                        pdata->result = RIL_E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW;
                    }
                    CAPI2_SYS_EnableCellInfoMsg(GetNewTID(), GetClientID(), FALSE);
                    pdata->handler_state = BCM_SYS_EnableCellInfoMsg;
                }
                else
                {
                    KRIL_SetServingCellTID(capi2_rsp->tid);
                }
#endif /* CONFIG_BRCM_FUSE_RPC_2157SDB */
            }
        }
        break;

        case BCM_SYS_EnableCellInfoMsg:
        {
            if (RIL_E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW == pdata->result)
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            else
                pdata->handler_state = BCM_FinishCAPI2Cmd;
        }
        break;
 
        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_SetLocationUpdatesHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KrilLocationUpdates_t *tdate = (KrilLocationUpdates_t *)pdata->ril_cmd->data;
            KRIL_DEBUG(DBG_INFO, "location_updates:%d\n", tdate->location_updates);
            KRIL_SetLocationUpdateStatus(tdate->location_updates);
            pdata->handler_state = BCM_FinishCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            if (capi2_rsp->result != RESULT_OK)
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}
