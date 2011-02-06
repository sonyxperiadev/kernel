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
#include "capi2_reqrep.h"
#endif

#define STK_COMMAND_DETAILS_LEN    5
#define STK_DEVICE_IDENTITIES_LEN  4
#define STK_RESULT_LEN             3

extern UInt8 terminal_profile_data[17];

static void SendCAPI2STKRsp( UInt32 tid, UInt8 clientID, SATK_EVENTS_t toEvent, UInt8 result1, UInt8 result2, SATKString_t *inText, UInt8 menuID);

//******************************************************************************
//
// Function Name: STK_ParseDeviceIdentities
//
// Description:   Parse Device Identities(refer to 11.14 section 12.7)
//
// Notes:
//
//******************************************************************************
int STK_ParseDeviceIdentities(UInt8 *byte)
{
    // Check tag
    if (!(byte[0] == 0x82 || byte[0] == 0x02))
    {
        KRIL_DEBUG(DBG_ERROR,"Error Device details tag:0x%X\n", byte[5]);
        return 0;
    }    
    
    // Check source device identity
    if (!(byte[2] == 0x82 || byte[2] == 0x01 || byte[2] == 0x02))
    {
        KRIL_DEBUG(DBG_ERROR,"Error Source Device identity:0x%X\n", byte[7]);
        return 0;
    }
    
    // Check destination device identity
    if (byte[3] != 0x81)
    {
        KRIL_DEBUG(DBG_ERROR,"Error Destination Device identity:0x%X\n", byte[8]);
        return 0;
    }
    
    return 1;    
}


//******************************************************************************
//
// Function Name: STK_ParseResult
//
// Description:   Parse Result(refer to 11.14 section 12.12)
//
// Notes:
//
//******************************************************************************
int STK_ParseResult(UInt8 *byte, SATK_ResultCode_t *resultcode, SATK_ResultCode2_t *resultcode2)
{
    UInt8 result_len;
    
    // Check result tag
    if (!(byte[0] == 0x83 || byte[0] == 0x03))
    {
        KRIL_DEBUG(DBG_ERROR,"Error Result tag:0x%X\n", byte[0]);
        return 0;
    }
    
    // Check length
    result_len = byte[1];    

    // Check General result
    KRIL_DEBUG(DBG_INFO,"General result:0x%02X\n", byte[2]);
    switch (byte[2])
    {
        case 0x00:
            *resultcode = SATK_Result_CmdSuccess;
            break;
        
        case 0x03:
            *resultcode = SATK_Result_RefreshPerformedWithAdditionalEFsRead;
            break;
        
        case 0x04:
            *resultcode = SATK_Result_CmdSuccessIconNotDisplayed;
            break;
        
        case 0x05:
            *resultcode = SATK_Result_CmdSuccessButModifiedBySim;
            break;
        
        case 0x10:
            *resultcode = SATK_Result_SIMSessionEndSuccess;
            break;
        
        case 0x11:
            *resultcode = SATK_Result_BackwardMove;
            break;
        
        case 0x12:
            *resultcode = SATK_Result_NoRspnFromUser;
            break;
        
        case 0x13:
            *resultcode = SATK_Result_RequestHelpByUser;
            break;
        
        case 0x14:
            *resultcode = SATK_Result_UserEndUSSD;
            break;
        
        case 0x20:
            *resultcode = SATK_Result_MeUnableToProcessCmd;
            break;
        
        case 0x21:
            *resultcode = SATK_Result_NetUnableToProcessCmd;
            break;
        
        case 0x22:
            *resultcode = SATK_Result_UserNotAcceptingCallSetup;
            break;
        
        case 0x23:
            *resultcode = SATK_Result_UserEndCallBeforeConnect;
            break;
        
        case 0x25:
            *resultcode = SATK_Result_InteractWithSimCCTempProblem;
            break;
        
        case 0x26:
            *resultcode = SATK_Result_LaunchBrowserGenericError;
            break;
        
        case 0x30:
            *resultcode = SATK_Result_BeyondMeCapability;
            break;
        
        case 0x31:
            *resultcode = SATK_Result_TypeUnknownToMe;
            break;
        
        case 0x32:
            *resultcode = SATK_Result_DataUnknownToMe;
            break;
            
        case 0x33:
            *resultcode = SATK_Result_NumberUnknownToMe;
            break;
        
        case 0x36:
            *resultcode = SATK_Result_ValueMissingError;
            break;
        
        case 0x37:
            *resultcode = SATK_Result_USSDError;
            break;
        
        case 0x39:
            *resultcode = SATK_Result_InteractWithSimCCPermProblem;
            break;
        
        default:
            KRIL_DEBUG(DBG_ERROR,"Not supported result:0x%02X Error!\n", byte[2]);
            return 0;
    }
    
    if (result_len == 2)
    {
        KRIL_DEBUG(DBG_ERROR,"General result2:0x%02X\n", byte[3]);
        
        switch (byte[3])
        {
            case 0x00:
                *resultcode2 = SATK_Result_NoCause;
                break;

            case 0x01:
                *resultcode2 = SATK_Result_ScreenBusy;
                break;

            case 0x02:
                *resultcode2 = SATK_Result_BusyOnCall;
                break;

            case 0x03:
                *resultcode2 = SATK_Result_BusyOnSS;
                break;

            case 0x04:
                *resultcode2 = SATK_Result_NoService;
                break;

            case 0x06:
                *resultcode2 = SATK_Result_RrNotGranted;
                break;

            case 0x07:
                *resultcode2 = SATK_Result_NotInSpeechCall;
                break;

            case 0x08:
                *resultcode2 = SATK_Result_BusyOnUSSD;
                break;

            case 0x09:
                *resultcode2 = SATK_Result_BusyOnDtmf;
                break;

            default:
                KRIL_DEBUG(DBG_ERROR,"Not supported result2:0x%02X Error!\n", byte[3]);
                return 0;
        }
    }
    
    return 1;
}


//******************************************************************************
//
// Function Name: STK_ParseItemIdentifier
//
// Description:   Parse Item Identifier(refer to 11.14 section 12.10)
//
// Notes:
//
//******************************************************************************
int STK_ParseItemIdentifier(UInt8 *byte, UInt8 *itemId)
{
    // Check item identifier tag
    if (!(byte[0] == 0x90 || byte[0] == 0x10))
    {
        KRIL_DEBUG(DBG_ERROR,"Error Item identifier tag:0x%X\n", byte[0]);
        return 0;
    }
    
    // Check identifier of item chosen
    *itemId = byte[2];
    KRIL_DEBUG(DBG_INFO,"itemId:%d\n",*itemId);
    return 1;
}


//******************************************************************************
//
// Function Name: STK_ParseTextString
//
// Description:   Parse Text String(refer to 11.14 section 12.15)
//
// Notes:
//
//******************************************************************************
int STK_ParseTextString(UInt8 *byte, SATKString_t *intext)
{
    // Check Text string tag
    if (!(byte[0] == 0x8D || byte[0] == 0x0D))
    {
        KRIL_DEBUG(DBG_ERROR,"Error Text string tag:0x%X\n", byte[0]);
        return 0;
    }    
    
    // Check Length
    intext->len = byte[1] - 1;
    
    // Check Data coding scheme
    switch (byte[2])
    {
        case 0x00:
            intext->unicode_type = UNICODE_GSM;
            break;
            
        case 0x04:
            intext->unicode_type = UNICODE_UCS1;
            break;
        
        case 0x08:
            intext->unicode_type = UNICODE_80;
            break;
        
        default:
            KRIL_DEBUG(DBG_ERROR,"Not supported coding scheme:0x%02X Error!\n", byte[2]);
            return 0;
    }
    
    // Parse Text string
    memcpy(intext->string, &byte[3], intext->len);
    
    return 1;
}


//******************************************************************************
//
// Function Name: STK_SetupMenuRsp
//
// Description:   Handle Setup Menu response
//
// Notes:
//
//******************************************************************************
int STK_SetupMenuRsp(UInt8 *byte)
{
    SATK_ResultCode_t resultcode = SATK_Result_CmdSuccess;
    SATK_ResultCode2_t resultcode2 = SATK_Result_NoCause;
    // Parse device identities
    if (!STK_ParseDeviceIdentities(&byte[5]))
        return 0;
       
    // Parse Result
    if (!STK_ParseResult(&byte[9], &resultcode, &resultcode2))
        return 0;

    SendCAPI2STKRsp(GetNewTID(), GetClientID(), SATK_EVENT_SETUP_MENU, resultcode, resultcode2, NULL, 0);
        
    return 1;
}


//******************************************************************************
//
// Function Name: STK_SelectItemRsp
//
// Description:   Handle Select Item response
//
// Notes:
//
//******************************************************************************
int STK_SelectItemRsp(UInt8 *byte, UInt8 cmdlen)
{
    SATK_ResultCode_t resultcode = SATK_Result_CmdSuccess;
    SATK_ResultCode2_t resultcode2 = SATK_Result_NoCause;
    UInt8 itemId = 0;
    
    // Parse device identities
    if (!STK_ParseDeviceIdentities(&byte[5]))
        return 0;
       
    // Parse Result
    if (!STK_ParseResult(&byte[9], &resultcode, &resultcode2))
        return 0;
    
    cmdlen -= (STK_COMMAND_DETAILS_LEN + STK_DEVICE_IDENTITIES_LEN + STK_RESULT_LEN + (byte[10] - 1));
    byte += (STK_COMMAND_DETAILS_LEN + STK_DEVICE_IDENTITIES_LEN + STK_RESULT_LEN + (byte[10] - 1));
    KRIL_DEBUG(DBG_INFO,"After parsing result: byte[0]:0x%02X cmdlen:%d\n", byte[0], cmdlen);
    
    // Parse Item identifier
    if (cmdlen > 0)
    {
        if (!STK_ParseItemIdentifier(byte, &itemId))
            return 0;
    }
    
    SendCAPI2STKRsp(GetNewTID(), GetClientID(), SATK_EVENT_SELECT_ITEM, resultcode, resultcode2, NULL, itemId);
    
    return 1;
}


//******************************************************************************
//
// Function Name: STK_GetInputRsp
//
// Description:   Handle Get Input response
//
// Notes:
//
//******************************************************************************
int STK_GetInputRsp(UInt8 *byte, UInt8 cmdlen)
{
    SATK_ResultCode_t resultcode = SATK_Result_CmdSuccess;
    SATK_ResultCode2_t resultcode2 = SATK_Result_NoCause;
    SATKString_t intext;
    UInt8   string[255];
    
    memset(&intext, 0, sizeof(SATKString_t));
    
    // Parse device identities
    if (!STK_ParseDeviceIdentities(&byte[5]))
        return 0;
       
    // Parse Result
    if (!STK_ParseResult(&byte[9], &resultcode, &resultcode2))
        return 0;

    cmdlen -= (STK_COMMAND_DETAILS_LEN + STK_DEVICE_IDENTITIES_LEN + STK_RESULT_LEN + (byte[10] - 1));
    byte += (STK_COMMAND_DETAILS_LEN + STK_DEVICE_IDENTITIES_LEN + STK_RESULT_LEN + (byte[10] - 1));
    KRIL_DEBUG(DBG_INFO,"After parsing result: byte[0]:0x%02X cmdlen:%d\n", byte[0], cmdlen);
        
    // Parse Text string
    if (cmdlen > 0)
    {
        intext.string = string;
        if (!STK_ParseTextString(byte, &intext))
            return 0;
    }
    
    if (intext.len > 0)
        SendCAPI2STKRsp(GetNewTID(), GetClientID(), SATK_EVENT_GET_INPUT, resultcode, resultcode2, &intext, 0);
    else
        SendCAPI2STKRsp(GetNewTID(), GetClientID(), SATK_EVENT_GET_INPUT, resultcode, resultcode2, NULL, 0);
    
    return 1;
}


//******************************************************************************
//
// Function Name: STK_GetInkeyRsp
//
// Description:   Handle Get Inkey response
//
// Notes:
//
//******************************************************************************
int STK_GetInkeyRsp(UInt8 *byte, UInt8 cmdlen)
{
    SATK_ResultCode_t resultcode = SATK_Result_CmdSuccess;
    SATK_ResultCode2_t resultcode2 = SATK_Result_NoCause;
    SATKString_t intext;
    UInt8   string[255];
    
    memset(&intext, 0, sizeof(SATKString_t));
    
    // Parse device identities
    if (!STK_ParseDeviceIdentities(&byte[5]))
        return 0;
       
    // Parse Result
    if (!STK_ParseResult(&byte[9], &resultcode, &resultcode2))
        return 0;
    
    cmdlen -= (STK_COMMAND_DETAILS_LEN + STK_DEVICE_IDENTITIES_LEN + STK_RESULT_LEN + (byte[10] - 1));
    byte += (STK_COMMAND_DETAILS_LEN + STK_DEVICE_IDENTITIES_LEN + STK_RESULT_LEN + (byte[10] - 1));
    KRIL_DEBUG(DBG_INFO,"After parsing result: byte[0]:0x%02X cmdlen:%d\n", byte[0], cmdlen);
    
    // Parse Text string
    if (cmdlen > 0)
    {
        intext.string = string;
        if (!STK_ParseTextString(byte, &intext))
            return 0;
    }
    
    if (intext.len > 0)
        SendCAPI2STKRsp(GetNewTID(), GetClientID(), SATK_EVENT_GET_INKEY, resultcode, resultcode2, &intext, 0);
    else
        SendCAPI2STKRsp(GetNewTID(), GetClientID(), SATK_EVENT_GET_INKEY, resultcode, resultcode2, NULL, 0);
    
    return 1;    
}


//******************************************************************************
//
// Function Name: STK_DisplayTextRsp
//
// Description:   Handle Display Text response
//
// Notes:
//
//******************************************************************************
int STK_DisplayTextRsp(UInt8 *byte, UInt8 cmdlen)
{
    SATK_ResultCode_t resultcode = SATK_Result_CmdSuccess;
    SATK_ResultCode2_t resultcode2 = SATK_Result_NoCause;

    // Parse device identities
    if (!STK_ParseDeviceIdentities(&byte[5]))
        return 0;
       
    // Parse Result
    if (!STK_ParseResult(&byte[9], &resultcode, &resultcode2))
        return 0;
    
    SendCAPI2STKRsp(GetNewTID(), GetClientID(), SATK_EVENT_DISPLAY_TEXT, resultcode, resultcode2, NULL, 0);        
    return 1;
}


//******************************************************************************
//
// Function Name: STK_SendMOSMSRsp
//
// Description:   Handle Send MO SMS response
//
// Notes:
//
//******************************************************************************
#if 0
int STK_SendMOSMSRsp(UInt8 *byte, UInt8 cmdlen)
{
    SATK_ResultCode_t resultcode = SATK_Result_CmdSuccess;
    SATK_ResultCode2_t resultcode2 = SATK_Result_NoCause;

    // Parse device identities
    if (!STK_ParseDeviceIdentities(&byte[5]))
        return 0;
       
    // Parse Result
    if (!STK_ParseResult(&byte[9], &resultcode, &resultcode2))
        return 0;
    
    SendCAPI2STKRsp(GetNewTID(), GetClientID(), SATK_EVENT_SEND_SHORT_MSG, resultcode, resultcode2, NULL, 0);
    return 1;    
}
#endif


//******************************************************************************
//
// Function Name: STK_PlayToneRsp
//
// Description:   Handle Play Tone response
//
// Notes:
//
//******************************************************************************
int STK_PlayToneRsp(UInt8 *byte, UInt8 cmdlen)
{
    SATK_ResultCode_t resultcode = SATK_Result_CmdSuccess;
    SATK_ResultCode2_t resultcode2 = SATK_Result_NoCause;

    // Parse device identities
    if (!STK_ParseDeviceIdentities(&byte[5]))
        return 0;
       
    // Parse Result
    if (!STK_ParseResult(&byte[9], &resultcode, &resultcode2))
        return 0;
    
    CAPI2_SATK_SendPlayToneRes(GetNewTID(), GetClientID(), resultcode);        
    return 1;
}


//******************************************************************************
//
// Function Name: STK_SetupIdleModeTextRsp
//
// Description:   Handle Setup Idle Mode Text response
//
// Notes:
//
//******************************************************************************
int STK_SetupIdleModeTextRsp(UInt8 *byte, UInt8 cmdlen)
{
    SATK_ResultCode_t resultcode = SATK_Result_CmdSuccess;
    SATK_ResultCode2_t resultcode2 = SATK_Result_NoCause;

    // Parse device identities
    if (!STK_ParseDeviceIdentities(&byte[5]))
        return 0;
       
    // Parse Result
    if (!STK_ParseResult(&byte[9], &resultcode, &resultcode2))
        return 0;
    
    SendCAPI2STKRsp(GetNewTID(), GetClientID(), SATK_EVENT_IDLEMODE_TEXT, resultcode, resultcode2, NULL, 0);
    return 1;    
}


//******************************************************************************
//
// Function Name: STK_RefreshRsp
//
// Description:   Handle Refresh response
//
// Notes:
//
//******************************************************************************
int STK_RefreshRsp(UInt8 *byte, UInt8 cmdlen)
{
    SATK_ResultCode_t resultcode = SATK_Result_CmdSuccess;
    SATK_ResultCode2_t resultcode2 = SATK_Result_NoCause;

    // Parse device identities
    if (!STK_ParseDeviceIdentities(&byte[5]))
        return 0;
       
    // Parse Result
    if (!STK_ParseResult(&byte[9], &resultcode, &resultcode2))
        return 0;
    
    SendCAPI2STKRsp(GetNewTID(), GetClientID(), SATK_EVENT_REFRESH, resultcode, resultcode2, NULL, 0);
    return 1;
}


//******************************************************************************
//
// Function Name: STK_LanuchBrowserRsp
//
// Description:   Handle Lanuch Browser response
//
// Notes:
//
//******************************************************************************
int STK_LanuchBrowserRsp(UInt8 *byte, UInt8 cmdlen)
{
    SATK_ResultCode_t resultcode = SATK_Result_CmdSuccess;
    SATK_ResultCode2_t resultcode2 = SATK_Result_NoCause;

    // Parse device identities
    if (!STK_ParseDeviceIdentities(&byte[5]))
        return 0;
       
    // Parse Result
    if (!STK_ParseResult(&byte[9], &resultcode, &resultcode2))
        return 0;
    
    SendCAPI2STKRsp(GetNewTID(), GetClientID(), SATK_EVENT_LAUNCH_BROWSER, resultcode, resultcode2, NULL, 0);
    return 1;    
}

//******************************************************************************
//
// Function Name: STK_SendTerminalRsp
//
// Description:   Handle Send terminal response
//
// Notes:
//
//******************************************************************************
int STK_SendTerminalRsp(KRIL_Command_t *ril_cmd)
{
    UInt8 *byte = (UInt8*)ril_cmd->data;
    UInt8 cmdlen = (UInt8)ril_cmd->datalen;
    
    RawDataPrintfun(byte, cmdlen, "TerminalRsp");
    
    // Parse command details
    if (!(byte[0] == 0x81 || byte[0] == 0x01))
    {
        KRIL_DEBUG(DBG_ERROR,"Error Command details tag:0x%X\n", byte[0]);
        return 0;
    }
        
    KRIL_DEBUG(DBG_INFO,"Command type:0x%X cmdlen:%d\n", byte[3], cmdlen);
    switch (byte[3])
    {
        case STK_SETUPMENU:
            if (!STK_SetupMenuRsp(byte))
                return 0;
            break;
        
        case STK_SELECTITEM:
            if (!STK_SelectItemRsp(byte, cmdlen))
                return 0;
            break;
        
        case STK_GETINPUT:
            if (!STK_GetInputRsp(byte, cmdlen))
                return 0;
            break;
        
        case STK_GETINKEY:
            if (!STK_GetInkeyRsp(byte, cmdlen))
                return 0;
            break;
            
        case STK_DISPLAYTEXT:
            if (!STK_DisplayTextRsp(byte, cmdlen))
                return 0;
            break;
        
        case STK_PLAYTONE:
            if (!STK_PlayToneRsp(byte, cmdlen))
                return 0;
            break;
        
        case STK_SETUPIDLEMODETEXT:
            if (!STK_SetupIdleModeTextRsp(byte, cmdlen))
                return 0;
            break;
        
        case STK_REFRESH:
            if (!STK_RefreshRsp(byte, cmdlen))
                return 0;
            break;
            
        case STK_LAUNCHBROWSER:
            if (!STK_LanuchBrowserRsp(byte, cmdlen))
                return 0;
            break;
            
        case STK_SENDSMS:
        case STK_SENDSS:
        case STK_SENDUSSD:
            // There is no need to send a response for these commands
            // when the radio is pre-parsing and sending text formatted
            // notifications.
            break;
        
        default:
            KRIL_DEBUG(DBG_ERROR,"Not suppported Command type:0x%X\n", byte[3]);
            return 0;
    }
    
    return 1;
}


//******************************************************************************
//
// Function Name: STK_MenuSelection
//
// Description:   Handle Menu Selection
// Notes:
//
//******************************************************************************
int STK_MenuSelection(UInt8 *envelopeCmd)
{
    UInt8 length;
    UInt8 itemId = 0;
    UInt8 helpRequest = 0;
    
    // Check length
    length = envelopeCmd[1];

    // Parse device identities
    if (!STK_ParseDeviceIdentities(&envelopeCmd[2]))
        return 0;    
    
    // Parse item identifier
    if (!STK_ParseItemIdentifier(&envelopeCmd[6], &itemId))
        return 0;
            
    // Check help request
    if (length > 7)
    {
        if (envelopeCmd[9] == 0x15 || envelopeCmd[9] == 0x95)
        {
            KRIL_DEBUG(DBG_ERROR,"Request help information\n");
            helpRequest = 1;
        }
        else
        {
            KRIL_DEBUG(DBG_ERROR,"Error help request tag:0x%X\n",envelopeCmd[9]);
            return 0;
        }
    }
    
    if (helpRequest)
        SendCAPI2STKRsp(GetNewTID(), GetClientID(), SATK_EVENT_MENU_SELECTION, 1, 0, NULL, itemId);
    else
        SendCAPI2STKRsp(GetNewTID(), GetClientID(), SATK_EVENT_MENU_SELECTION, SATK_Result_CmdSuccess, 0, NULL, itemId);
    
    return 1;
}


//******************************************************************************
//
// Function Name: STK_SendEnvelopeCmd
//
// Description:   Handle Send Envelope command
// Notes:
//
//******************************************************************************
int STK_SendEnvelopeCmd(KRIL_Command_t *ril_cmd)
{
    UInt8 *envelopeCmd = (UInt8*)ril_cmd->data;
    
    RawDataPrintfun(envelopeCmd, ril_cmd->datalen, "envelopeCmd");
    // Parse tag
    KRIL_DEBUG(DBG_INFO,"tag:0x%X\n", envelopeCmd[0]);
    switch (envelopeCmd[0])
    {
        case 0xD3:
            if (!STK_MenuSelection(envelopeCmd))
                return 0;
            break;
        
        default:
            KRIL_DEBUG(DBG_ERROR,"Not suppported tag:0x%X\n", envelopeCmd[0]);
            return 0;
    }

    return 1;
}


//******************************************************************************
//
// Function Name: KRIL_StkSendTerminalRspHandler
//
// Description:   
// Notes:
//
//******************************************************************************
void KRIL_StkSendTerminalRspHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

    KRIL_DEBUG(DBG_INFO,"pdata->handler_state:0x%lX\n", pdata->handler_state);
    
    if (capi2_rsp && capi2_rsp->result != RESULT_OK)
    {
        KRIL_DEBUG(DBG_ERROR,"CAPI2 response failed:%d\n", capi2_rsp->result);
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }
    
    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
            if (!STK_SendTerminalRsp(pdata->ril_cmd))
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
                
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;

        case BCM_RESPCAPI2Cmd:
            pdata->result = RIL_E_SUCCESS;
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
                    
        default:
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
    } 
}


//******************************************************************************
//
// Function Name: KRIL_StkSendEnvelopeCmdHandler
//
// Description:   
// Notes:
//
//******************************************************************************
void KRIL_StkSendEnvelopeCmdHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

    KRIL_DEBUG(DBG_INFO,"pdata->handler_state:0x%lX\n", pdata->handler_state);
    
    if (capi2_rsp && capi2_rsp->result != RESULT_OK)
    {
        KRIL_DEBUG(DBG_ERROR,"CAPI2 response failed:%d\n", capi2_rsp->result);
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }
    
    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
            if (!STK_SendEnvelopeCmd(pdata->ril_cmd))
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
                
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;

        case BCM_RESPCAPI2Cmd:
            pdata->result = RIL_E_SUCCESS;
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
                    
        default:
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
    } 
}


//******************************************************************************
//
// Function Name: KRIL_StkHandleCallSetupRequestedHandler
//
// Description:   
// Notes:
//
//******************************************************************************
void KRIL_StkHandleCallSetupRequestedHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

    KRIL_DEBUG(DBG_INFO,"pdata->handler_state:0x%lX\n", pdata->handler_state);
    
    if (capi2_rsp && capi2_rsp->result != RESULT_OK)
    {
        KRIL_DEBUG(DBG_ERROR,"CAPI2 response failed:%d\n", capi2_rsp->result);
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }
    
    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            int *accept = (int*)pdata->ril_cmd->data;
            
            KRIL_DEBUG(DBG_INFO,"accept:%d\n", *accept);
            if (*accept)
                SendCAPI2STKRsp(GetNewTID(), GetClientID(), SATK_EVENT_SETUP_CALL, SATK_Result_CmdSuccess, 0, NULL, 0);
                //CAPI2_SATK_SendSetupCallRes(GetNewTID(), GetClientID(), SATK_Result_CmdSuccess);
            else
                SendCAPI2STKRsp(GetNewTID(), GetClientID(), SATK_EVENT_SETUP_CALL, SATK_Result_UserNotAcceptingCallSetup, 0, NULL, 0);
                //CAPI2_SATK_SendSetupCallRes(GetNewTID(), GetClientID(), SATK_Result_UserNotAcceptingCallSetup);
            
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
            pdata->result = RIL_E_SUCCESS;
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
                    
        default:
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
    } 
}


//******************************************************************************
//
// Function Name: KRIL_StkGetProfile
//
// Description:   
// Notes:
//
//******************************************************************************
void KRIL_StkGetProfile(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

    KRIL_DEBUG(DBG_INFO,"pdata->handler_state:0x%lX\n", pdata->handler_state);
    
    if (capi2_rsp && capi2_rsp->result != RESULT_OK)
    {
        KRIL_DEBUG(DBG_ERROR,"CAPI2 response failed:%d\n", capi2_rsp->result);
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }    

    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KrilStkprofile_t *stkprofile;
            
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilStkprofile_t), GFP_KERNEL);
            if (!pdata->bcm_ril_rsp)
            {
                KRIL_DEBUG(DBG_ERROR,"Allocate bcm_ril_rsp memory failed!!\n");
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            
            stkprofile = pdata->bcm_ril_rsp;
            memset(stkprofile, 0, sizeof(KrilStkprofile_t));
            pdata->rsp_len = sizeof(KrilStkprofile_t);
            
            HexDataToHexStr(stkprofile->stkprofile, terminal_profile_data, 
                sizeof(terminal_profile_data)/sizeof(UInt8));
            pdata->result = RIL_E_SUCCESS;
            
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }

        default:
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
    }     
}

//******************************************************************************
//
// Function Name: KRIL_StkSetProfile
//
// Description:   
// Notes:
//
//******************************************************************************
void KRIL_StkSetProfile(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t*)ril_cmd;

    KRIL_DEBUG(DBG_INFO,"pdata->handler_state:0x%lX\n", pdata->handler_state);
    
    if (capi2_rsp && capi2_rsp->result != RESULT_OK)
    {
        KRIL_DEBUG(DBG_ERROR,"CAPI2 response failed:%d\n", capi2_rsp->result);
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }
    
    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            UInt8 *stkprofile = (UInt8*)pdata->ril_cmd->data;
    
            RawDataPrintfun(stkprofile, pdata->ril_cmd->datalen, "stkprofile");
            
            // Update terminal_profile_data[]
            memcpy(terminal_profile_data, stkprofile, sizeof(terminal_profile_data)/sizeof(UInt8));
            
            CAPI2_SATK_SetTermProfile(GetNewTID(), GetClientID(), stkprofile,
                pdata->ril_cmd->datalen);
            
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }    
        
        case BCM_RESPCAPI2Cmd:
            pdata->result = RIL_E_SUCCESS;
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
                    
        default:
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;        
        
    }    
}

//******************************************************************************
//
// Function Name: SendCAPI2STKRsp
//
// Description:   
// Notes:
//
//******************************************************************************
void SendCAPI2STKRsp( UInt32 tid, UInt8 clientID, SATK_EVENTS_t toEvent, UInt8 result1, UInt8 result2, SATKString_t *inText, UInt8 menuID)
{
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
    ClientInfo_t clientInfo;
    CAPI2_InitClientInfo(&clientInfo, tid, clientID);
    CAPI2_SatkApi_CmdResp(&clientInfo, toEvent, result1, result2, inText, menuID);
#else
    CAPI2_SATKCmdResp(tid, clientID, toEvent, result1, result2, inText, menuID);
#endif
}

