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
#include "capi2_reqrep.h"
#include "capi2_gen_cp_api.h"
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
#include <linux/broadcom/bcm_fuse_sysparm_CIB.h>
#include "bcm_kril_atrpc_handler.h"
#include "capi2_cc_api.h"
#else
#include <linux/broadcom/bcm_fuse_sysparm.h>
#endif // CONFIG_ARCH_BCM215XX
char* AtString = "at*mtest=1,1,\"coolcard\"";	// password to permit 'at*mtest' commands
// char* AtString1 = "at*mtest=4,0,511";		// would disable all CP logging
char *AtString1 = "at" ;						// a valid 'do-nothing' AT command

UInt8 g_RSSIThreshold = 7;

// Assume flight mode is on unless Android framework requests to set radio power
Boolean gIsFlightModeOnBoot = TRUE;

// For STK
UInt8 terminal_profile_data[17] = {0xFF,0xFF,0xFF,0xFF,0x1F,0x00,0x00,0x5F,0x57,0x00,0x00,
                                      0x00,0x00,0x10,0x20,0xA6,0x00};

static void ParseIMSIData(KRIL_CmdList_t *pdata, Kril_CAPI2Info_t *capi2_rsp);
static void ParseIMEIData(KRIL_CmdList_t *pdata, Kril_CAPI2Info_t *capi2_rsp);

// number of characters in IMEI string (15 digit IMEI plus check digit, plus 1 for null termination)
#define IMEI_STRING_LEN (IMEI_DIGITS+1)

void KRIL_InitCmdHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
    {
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);
        
        if(capi2_rsp->result != RESULT_OK)
        {
           pdata->handler_state = BCM_ErrorCAPI2Cmd;
        }
    }

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            CAPI2_MS_Element_t data;   
            KrilInit_t *pInitData = (KrilInit_t *)(pdata->ril_cmd->data);

            // if there is a valid IMEI, make appropriate CAPI2 call to set
            // IMEI on CP, otherwise fall through to next init command
            if (pInitData->is_valid_imei)
            {
                KRIL_DEBUG(DBG_INFO, "OTP IMEI:%s\n", pInitData->imei);
                memset(&data, 0, sizeof(CAPI2_MS_Element_t));
                memcpy(data.data_u.imeidata, pInitData->imei, IMEI_DIGITS);
                data.inElemType = MS_LOCAL_PHCTRL_ELEM_IMEI;
                CAPI2_MS_SetElement(GetNewTID(), GetClientID(), &data);
                pdata->handler_state = BCM_SET_IMEI;
                break;
            }

            // if OTP IMEI passed from URIL is not valid, we skip the
            // CAPI2_MS_SetElement() call and fall through to execute the
            // next CAPI2 init call instead...
        }

        case BCM_SET_IMEI:
        {
        //  **FIXME** MAG - update to use new AT interface before checkin for Athena
#ifndef CONFIG_BRCM_FUSE_RIL_CIB
            CAPI2_AT_ProcessCmd(GetNewTID(), GetClientID(), 0, (UInt8*)AtString);
            pdata->handler_state = BCM_SET_RADIO_OFF;
            break;
        }

        case BCM_SET_RADIO_OFF:
        {
            // For flight mode power up battery ADC & deep sleep issue (MobC00131482), set the initial CP state to RADIO_OFF.
            // If MS is powered up in normal mode, Android framework will send RIL_REQUEST_RADIO_POWER to RIL.
            CAPI2_SYS_ProcessNoRfReq(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_AT_ProcessCmd;
            break;
        }

        case BCM_AT_ProcessCmd:
        {
            CAPI2_AT_ProcessCmd(GetNewTID(), GetClientID(), 0, (UInt8*)AtString1);
            pdata->handler_state = BCM_SMS_ELEM_CLIENT_HANDLE_MT_SMS;
            break;
        }

        case BCM_SMS_ELEM_CLIENT_HANDLE_MT_SMS:
        {
#endif
            CAPI2_MS_Element_t data;
            memset((UInt8*)&data, 0, sizeof(CAPI2_MS_Element_t));
            data.inElemType = MS_LOCAL_SMS_ELEM_CLIENT_HANDLE_MT_SMS;
            data.data_u.bData = TRUE;
            CAPI2_MS_SetElement(GetNewTID(), GetClientID(), &data);
            pdata->handler_state = BCM_SMS_SetSmsReadStatusChangeMode;
            break;
        }

        case BCM_SMS_SetSmsReadStatusChangeMode:
        {
            CAPI2_SMS_SetSmsReadStatusChangeMode(GetNewTID(), GetClientID(), FALSE);
            pdata->handler_state = BCM_SYS_SetFilteredEventMask;
            break;
        }

        case BCM_SYS_SetFilteredEventMask:
        {
            UInt16 filterList[]={MSG_CAPI2_AT_RESPONSE_IND, MSG_RSSI_IND};
            CAPI2_SYS_SetFilteredEventMask(GetNewTID(), GetClientID(), &filterList[0], sizeof(filterList)/sizeof(UInt16), SYS_AP_DEEP_SLEEP_MSG_FILTER);
            pdata->handler_state = BCM_SYS_SetRssiThreshold;
            break;
        }

        case BCM_SYS_SetRssiThreshold:
        {
            CAPI2_SYS_SetRssiThreshold(GetNewTID(), GetClientID(), g_RSSIThreshold, 20, g_RSSIThreshold, 20);
            pdata->handler_state = BCM_PMU_Low_Battery_Register;
            break;
        }

        case BCM_PMU_Low_Battery_Register:
        {
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
            CAPI2_PMU_Battery_Register(GetNewTID(), GetClientID(), BATTMGR_LOW_BATT_EVENT);
#else
            CAPI2_PMU_Battery_Register(GetNewTID(), GetClientID(), EM_BATTMGR_LOW_BATT_EVENT);
#endif
            pdata->handler_state = BCM_PMU_Empty_Battery_Register;
            break;
        }

        case BCM_PMU_Empty_Battery_Register:
        {
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
            CAPI2_PMU_Battery_Register(GetNewTID(), GetClientID(), BATTMGR_EMPTY_BATT_EVENT);
#else
            CAPI2_PMU_Battery_Register(GetNewTID(), GetClientID(), EM_BATTMGR_EMPTY_BATT_EVENT);
#endif
            pdata->handler_state = BCM_PMU_LevelChange_Battery_Register;
            break;
        }

        case BCM_PMU_LevelChange_Battery_Register:
        {
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
            CAPI2_PMU_Battery_Register(GetNewTID(), GetClientID(), BATTMGR_BATTLEVEL_CHANGE_EVENT);
#else
            CAPI2_PMU_Battery_Register(GetNewTID(), GetClientID(), EM_BATTMGR_BATTLEVEL_CHANGE_EVENT);
#endif
            pdata->handler_state = BCM_SATK_SetTermProfile;
            break;
        }

        case BCM_SATK_SetTermProfile:
        {
            CAPI2_SATK_SetTermProfile(GetNewTID(), GetClientID(), terminal_profile_data,
                sizeof(terminal_profile_data)/sizeof(UInt8));
            pdata->handler_state = BCM_SATK_SETUP_CALL_CTR;
            break;
        }

        case BCM_SATK_SETUP_CALL_CTR:
        {
            CAPI2_MS_Element_t data;
            memset((UInt8*)&data, 0x00, sizeof(CAPI2_MS_Element_t));
            data.inElemType = MS_LOCAL_SATK_ELEM_SETUP_CALL_CTR;
            data.data_u.bData = TRUE;
            CAPI2_MS_SetElement(GetNewTID(), GetClientID(), &data);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }
        
        case BCM_RESPCAPI2Cmd:
        {
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_RadioPowerHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    gIsFlightModeOnBoot = FALSE;

    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            int *OnOff = (int *)(pdata->ril_cmd->data);

            KRIL_DEBUG(DBG_INFO, "On-Off:%d\n", *OnOff);
            if (1 == *OnOff)
            {
               CAPI2_SYS_ProcessPowerUpReq(GetNewTID(), GetClientID());
            }
            else
            {
                CAPI2_SYS_ProcessNoRfReq(GetNewTID(), GetClientID());
            }
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            KRIL_DEBUG(DBG_TRACE, "handler state:%lu\n", pdata->handler_state);
            pdata->bcm_ril_rsp = NULL;
            pdata->rsp_len = 0;
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_SetTTYModeHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            int *mode = (int *)(pdata->ril_cmd->data);
            KRIL_DEBUG(DBG_INFO, "mode:%d\n", *mode);
            CAPI2_CC_SetTTYCall(GetNewTID(), GetClientID(), (Boolean) *mode);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            if(capi2_rsp->result != RESULT_OK)
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_QueryTTYModeHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            pdata->bcm_ril_rsp = kmalloc(sizeof(krilQueryTTYModeType_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(krilQueryTTYModeType_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
            CAPI2_CC_IsTTYEnable(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {

            if(capi2_rsp->result != RESULT_OK)
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                Boolean mode = *(Boolean *)capi2_rsp->dataBuf;
                krilQueryTTYModeType_t *rdata = (krilQueryTTYModeType_t *)pdata->bcm_ril_rsp;
                rdata->mode = (int)mode;
                KRIL_DEBUG(DBG_TRACE, "BCM_RESPCAPI2Cmd:: rdata->mode:%d mode:%d\n", rdata->mode, mode);
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_BasebandVersionHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
    {
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);
        if(capi2_rsp->result != RESULT_OK)
        {
            pdata->result = RILErrorResult(capi2_rsp->result);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            return;
        }
    }

    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            pdata->bcm_ril_rsp = kmalloc(sizeof(krilQueryBaseBandVersion_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(krilQueryBaseBandVersion_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
            CAPI2_SYSPARM_GetSWVersion(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            UInt8 *version = (UInt8 *)capi2_rsp->dataBuf;
            krilQueryBaseBandVersion_t *rdata = (krilQueryBaseBandVersion_t *)pdata->bcm_ril_rsp;
            strcpy(rdata->version, (char *)version);
            KRIL_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd:: rdata->version:[%s] version:[%s]\n", (char *)rdata->version, (char *)version);
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_PmuGetBattADCHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

    KRIL_DEBUG(DBG_INFO,"pdata->handler_state:0x%lX\n", pdata->handler_state);
    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            // **FIXME** MAG - need to determine where this is defined for CIB
            // sysrpc, maybe?
#ifndef CONFIG_BRCM_FUSE_RIL_CIB
            CAPI2_PMU_BattADCReq(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
#endif
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            if (capi2_rsp->result != RESULT_OK)
            {
                KRIL_DEBUG(DBG_INFO,"CAPI2 response failed:%d\n", capi2_rsp->result);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_GetActualLowVoltHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

    KRIL_DEBUG(DBG_INFO,"pdata->handler_state:0x%lX\n", pdata->handler_state);
    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            CAPI2_SYSPARM_GetActualLowVoltReading(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            if (capi2_rsp->result != RESULT_OK)
            {
                KRIL_DEBUG(DBG_INFO,"CAPI2 response failed:%d\n", capi2_rsp->result);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_GetBattLowThreshHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

    KRIL_DEBUG(DBG_INFO,"pdata->handler_state:0x%lX\n", pdata->handler_state);
    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
#ifndef CONFIG_BRCM_FUSE_RIL_CIB
            // **FIXME** MAG - need to add to sysparm driver for CIB
            CAPI2_SYSPARM_GetBattLowThresh(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
#endif
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            if (capi2_rsp->result != RESULT_OK)
            {
                KRIL_DEBUG(DBG_INFO,"CAPI2 response failed:%d\n", capi2_rsp->result);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}

void KRIL_GetIMSIHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

    KRIL_DEBUG(DBG_INFO,"pdata->handler_state:0x%lX\n", pdata->handler_state);
    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            CAPI2_SIM_GetIMSI(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            ParseIMSIData(pdata, capi2_rsp);
            break;
        }
        
        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }        
}


void KRIL_GetIMEIHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

    KRIL_DEBUG(DBG_INFO,"pdata->handler_state:0x%lX\n", pdata->handler_state);
    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            CAPI2_MS_GetElement(GetNewTID(), GetClientID(), MS_LOCAL_PHCTRL_ELEM_IMEI);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            ParseIMEIData(pdata, capi2_rsp);
            break;
        }
        
        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }        
}


void KRIL_GetIMEISVHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

    if (capi2_rsp != NULL)
    {
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);
        if(capi2_rsp->result != RESULT_OK)
        {
            pdata->result = RILErrorResult(capi2_rsp->result);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            return;
        }
    }

    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
            // **FIXME** MAG - MS_LOCAL_PHCTRL_ELEM_SW_VERSION not currently supported under CIB
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            KRIL_DEBUG(DBG_ERROR, "** needs fix for CIB\n");
#else
            CAPI2_MS_GetElement(GetNewTID(), GetClientID(), MS_LOCAL_PHCTRL_ELEM_SW_VERSION);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
#endif
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            CAPI2_MS_Element_t *rsp = (CAPI2_MS_Element_t *) capi2_rsp->dataBuf;
            KrilImeiData_t *imeisv_result;
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilImeiData_t), GFP_KERNEL);
            imeisv_result = (KrilImeiData_t *)pdata->bcm_ril_rsp;
            memset(imeisv_result, 0, sizeof(KrilImeiData_t));
            pdata->rsp_len = sizeof(KrilImeiData_t);
            strcpy(imeisv_result->imeisv, rsp->data_u.u3Bytes);
            KRIL_DEBUG(DBG_INFO, "u3Bytes:[%s] imeisv:[%s]\n", rsp->data_u.u3Bytes, imeisv_result->imeisv);
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_GetDeviceIdentityHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

    if (capi2_rsp != NULL)
    {
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);
        if(capi2_rsp->result != RESULT_OK)
        {
            pdata->result = RILErrorResult(capi2_rsp->result);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            return;
        }
    }

    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            CAPI2_MS_GetElement(GetNewTID(), GetClientID(), MS_LOCAL_PHCTRL_ELEM_IMEI);
            pdata->handler_state = BCM_GetIMEIInfo;
            break;
        }

        case BCM_GetIMEIInfo:
        {
            ParseIMEIData(pdata, capi2_rsp);
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
            // **FIXME** MAG - MS_LOCAL_PHCTRL_ELEM_SW_VERSION not currently supported under CIB
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            KRIL_DEBUG(DBG_ERROR, "** needs fix for CIB\n");
#else
            CAPI2_MS_GetElement(GetNewTID(), GetClientID(), MS_LOCAL_PHCTRL_ELEM_SW_VERSION);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
#endif
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            CAPI2_MS_Element_t *rsp = (CAPI2_MS_Element_t *) capi2_rsp->dataBuf;
            KrilImeiData_t *imeisv_result = (KrilImeiData_t *)pdata->bcm_ril_rsp;
            strcpy(imeisv_result->imeisv, rsp->data_u.u3Bytes);
            KRIL_DEBUG(DBG_INFO, "u3Bytes:[%s] imeisv:[%s]\n", rsp->data_u.u3Bytes, imeisv_result->imeisv);
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}

void KRIL_QuerySimEmergencyNumberHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

    if (capi2_rsp != NULL)
    {
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::msgType:%d result:%d\n", pdata->handler_state, capi2_rsp->msgType, capi2_rsp->result);
        if(capi2_rsp->result != RESULT_OK)
        {
            pdata->result = RILErrorResult(capi2_rsp->result);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            return;
        }
    }

    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            pdata->bcm_ril_rsp = kmalloc(sizeof(Kril_SIMEmergency), GFP_KERNEL);
            pdata->rsp_len = sizeof(Kril_SIMEmergency);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
            CAPI2_PBK_SendInfoReq(GetNewTID(), GetClientID(), PB_EN);
            pdata->handler_state = BCM_PBK_SendInfoReq;
            break;
        }

        case BCM_PBK_SendInfoReq:
        {
            PBK_INFO_RSP_t *rsp = (PBK_INFO_RSP_t *) capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_INFO,"total_entries:[%d] result:[%d]\n", rsp->total_entries, rsp->result);
            if (0 == rsp->total_entries ||FALSE == rsp->result)
            {
                Kril_SIMEmergency *rdata = (Kril_SIMEmergency *)pdata->bcm_ril_rsp;
                rdata->simAppType = KRIL_GetSimAppType();
                KRIL_SendNotify(BRIL_UNSOL_EMERGENCY_NUMBER, pdata->bcm_ril_rsp, pdata->rsp_len);
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            else
            {
                CAPI2_PBK_SendReadEntryReq(GetNewTID(), GetClientID(), PB_EN, 0, (rsp->total_entries-1));
                pdata->handler_state = BCM_PBK_ReadENEnteryReq;
            }
            break;
        }

        case BCM_PBK_ReadENEnteryReq:
        {
            PBK_ENTRY_DATA_RSP_t *rsp = (PBK_ENTRY_DATA_RSP_t *) capi2_rsp->dataBuf;
            Kril_SIMEmergency *rdata = (Kril_SIMEmergency *)pdata->bcm_ril_rsp;
            KRIL_DEBUG(DBG_INFO,"rsp->data_result:%d\n", rsp->data_result);

            if (rsp->data_result == PBK_ENTRY_VALID_IS_LAST || rsp->data_result == PBK_ENTRY_VALID_NOT_LAST)
            {
                KRIL_DEBUG(DBG_INFO,"simecclist:[%s] number:[%s] numlen:%d\n", rdata->simecclist, rsp->pbk_rec.number, strlen(rsp->pbk_rec.number));
                if (strlen(rsp->pbk_rec.number) != 0 && 
                    !(strcmp("112", rsp->pbk_rec.number) == 0 || strcmp("911", rsp->pbk_rec.number) == 0))
                {
                    if (strlen(rdata->simecclist) != 0)
                    {
                        sprintf(&rdata->simecclist[0], "%s%s%s", rdata->simecclist, ",", rsp->pbk_rec.number);
                    } 
                    else
                    {
                        strcpy(rdata->simecclist, rsp->pbk_rec.number);
                    }
                }
                if (rsp->data_result == PBK_ENTRY_VALID_IS_LAST)
                {
                    rdata->simAppType = KRIL_GetSimAppType();
                    KRIL_SendNotify(BRIL_UNSOL_EMERGENCY_NUMBER, pdata->bcm_ril_rsp, pdata->rsp_len);
                    pdata->handler_state = BCM_FinishCAPI2Cmd;
                }
            }
            else
            {
                rdata->simAppType = KRIL_GetSimAppType();
                KRIL_SendNotify(BRIL_UNSOL_EMERGENCY_NUMBER, pdata->bcm_ril_rsp, pdata->rsp_len);
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void ParseIMSIData(KRIL_CmdList_t *pdata, Kril_CAPI2Info_t *capi2_rsp)
{
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
    IMSI_t* rsp = (IMSI_t*)capi2_rsp->dataBuf;
#else
    SIM_IMSI_RESULT_t *rsp = (SIM_IMSI_RESULT_t*)capi2_rsp->dataBuf;
#endif

    KrilImsiData_t *imsi_result;

    if (!rsp)
    {
        KRIL_DEBUG(DBG_ERROR,"capi2_rsp->dataBuf is NULL, Error!!\n");
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }
    
    pdata->bcm_ril_rsp = kmalloc(sizeof(KrilImsiData_t), GFP_KERNEL);
    if (!pdata->bcm_ril_rsp)
    {
        KRIL_DEBUG(DBG_ERROR,"Allocate bcm_ril_rsp memory failed!!\n");
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }    

    imsi_result = pdata->bcm_ril_rsp;
    memset(imsi_result, 0, sizeof(KrilImsiData_t));
    pdata->rsp_len = sizeof(KrilImsiData_t);
    
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
    if ( capi2_rsp->result != RESULT_OK )
#else
    if (rsp->result != SIMACCESS_SUCCESS)
#endif
    {
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
        KRIL_DEBUG(DBG_ERROR,"IMSI: SIM access failed!! result:%d\n",capi2_rsp->result);
#else
        KRIL_DEBUG(DBG_ERROR,"IMSI: SIM access failed!! result:%d\n",rsp->result);
#endif
        imsi_result->result = RIL_E_GENERIC_FAILURE;
    }
    else
    {
        imsi_result->result = RIL_E_SUCCESS;
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
        KRIL_DEBUG(DBG_INFO,"IMSI:%s\n", (char*)rsp);
        strncpy(imsi_result->imsi, (char*)rsp, (IMSI_DIGITS+1));
#else
        KRIL_DEBUG(DBG_INFO,"IMSI:%s\n", (char*)rsp->imsi);
        strncpy(imsi_result->imsi, (char*)rsp->imsi, (IMSI_DIGITS+1));
#endif
    }
    
    pdata->handler_state = BCM_FinishCAPI2Cmd;
}


//******************************************************************************
// Function Name:      ParseIMEIData
//
// Description:        Internal helper function used to parse CAPI response
//                     to retrieval of IMEI from MS database. If retrieval
//                     from MS database fails, or IMEI stored there is all
//                     0's, we instead return the IMEI stored in sysparms
//                     (this mirrors behaviour of SYSPARM_GetImeiStr() on CP)
//
//******************************************************************************
static void ParseIMEIData(KRIL_CmdList_t* pdata, Kril_CAPI2Info_t* capi2_rsp)
{
    CAPI2_MS_Element_t* rsp = (CAPI2_MS_Element_t*)capi2_rsp->dataBuf;
    KrilImeiData_t* imei_result;

    if (!rsp)
    {
        KRIL_DEBUG(DBG_ERROR,"capi2_rsp->dataBuf is NULL, Error!!\n");
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }
    
    pdata->bcm_ril_rsp = kmalloc(sizeof(KrilImeiData_t), GFP_KERNEL);
    if (!pdata->bcm_ril_rsp)
    {
        KRIL_DEBUG(DBG_ERROR,"Allocate bcm_ril_rsp memory failed!!\n");
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }    

    imei_result = pdata->bcm_ril_rsp;
    memset(imei_result, 0, sizeof(KrilImeiData_t));
    pdata->rsp_len = sizeof(KrilImeiData_t);
    
    if (rsp->inElemType != MS_LOCAL_PHCTRL_ELEM_IMEI)
    {
        KRIL_DEBUG(DBG_ERROR,"IMEI: inElemType Error!! inElemType:%d\n",rsp->inElemType);
        imei_result->result = RIL_E_GENERIC_FAILURE;
    }
    else
    {
        // at this point, we have the MS database IMEI string; check if it is non-zero, and if not,
        // use IMEI from sysparms
        UInt8 i=0;
        Boolean bUseMSDBImei = FALSE;
        UInt8* pMSDBImeiStr = (UInt8*)rsp->data_u.imeidata;

        imei_result->result = RIL_E_SUCCESS;
        
        do
        {
            bUseMSDBImei = (pMSDBImeiStr[i] != '0');
        }
        while ( !bUseMSDBImei && (++i < IMEI_DIGITS) );
        
        if ( bUseMSDBImei )
        {
            // MS database IMEI is not all 0's, so we use it
            strncpy(imei_result->imei, pMSDBImeiStr, IMEI_DIGITS);
            imei_result->imei[IMEI_DIGITS] = '\0';
            KRIL_DEBUG(DBG_INFO,"Using MS DB IMEI:%s\n", pMSDBImeiStr );
        }
        else
        {
            // MS database IMEI is all 0's, so retrieve IMEI from sysparms
            UInt8 tmpImeiStr[IMEI_STRING_LEN];
            Boolean bFound;
            
            // retrieve null terminated IMEI string
            bFound = SYSPARM_GetImeiStr( tmpImeiStr );
            if ( bFound )
            {
                // got it from sysparms, so copy to the response struct
                KRIL_DEBUG(DBG_INFO,"Using sysparm IMEI:%s\n", tmpImeiStr );
                strncpy(imei_result->imei, tmpImeiStr, IMEI_STRING_LEN);
            }
            else
            {
                KRIL_DEBUG(DBG_INFO,"** SYSPARM_GetImeiStr() failed\n" );
                imei_result->result = RIL_E_GENERIC_FAILURE;
            }
        }
        
    }
    
    pdata->handler_state = BCM_FinishCAPI2Cmd;
}


void KRIL_ATProcessCmdHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

    KRIL_DEBUG(DBG_INFO,"KRIL_ATProcessCmdHandler() pdata->handler_state:0x%lX\n", pdata->handler_state);
    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            UInt32 tid = GetNewTID();
            bcm_kril_at_cmd_req_t *pATReq = (bcm_kril_at_cmd_req_t *)pdata->ril_cmd->data;
            KRIL_DEBUG(DBG_INFO,"BCM_SendCAPI2Cmd tid=%ld, cmd=%s\n", tid,(char*)(&(pATReq->ATCmd)));
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
            // in CIB, AT is routed directly thru RPC now, not CAPI2
            KRIL_SendATCmd(pATReq->chan,  (UInt8*)(&(pATReq->ATCmd)) );
            KRIL_DEBUG(DBG_INFO,"setting NULL response\n");	 
            pdata->bcm_ril_rsp = NULL;
            pdata->rsp_len = 0;
            pdata->handler_state = BCM_FinishCAPI2Cmd;
#else
            CAPI2_AT_ProcessCmd(GetNewTID(), GetClientID(), pATReq->chan, (UInt8*)(&(pATReq->ATCmd)));
            pdata->handler_state = BCM_RESPCAPI2Cmd;
#endif
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            if (capi2_rsp->result != RESULT_OK)
            {
                KRIL_DEBUG(DBG_INFO,"CAPI2 response failed:%d\n", capi2_rsp->result);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }    
}
