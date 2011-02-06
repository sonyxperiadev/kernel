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
#include "bcm_kril_cmd_handler.h"
#include "bcm_kril_ioctl.h"

#ifdef CONFIG_BRCM_FUSE_RIL_CIB
#include "capi2_reqrep.h"
#endif

// For Network
static MSRegState_t  sCreg_state = REG_STATE_NO_SERVICE;
static MSRegState_t  sCgreg_state = REG_STATE_NO_SERVICE;
MSRegInfo_t  gRegInfo;
MSUe3gStatusInd_t  gUE3GInfo;

// For STK
/* Macro to determine if the passed coding type is ALPHAID encoding */
#define IS_ALPHAID_CODING(code_type) ( ((code_type) == UNICODE_80) || ((code_type) == UNICODE_81) || ((code_type) == UNICODE_82))

#define MAX_SIMPLE_TLV_DATA_LENGTH  255
#define MAX_TLV_DATA_LENGTH         (MAX_SIMPLE_TLV_DATA_LENGTH + 3)
#define MAX_TLV_STRING_LENGTH       (MAX_TLV_DATA_LENGTH * 2 + 1)

extern CallIndex_t  gUssdID;
extern CallIndex_t  gPreviousUssdID;

extern KrilDataCallResponse_t pdp_resp[BCM_NET_MAX_PDP_CNTXS];
extern bcm_kril_dev_result_t bcm_dev_results[TOTAL_BCMDEVICE_NUM];

extern Boolean gIsFlightModeOnBoot;

//******************************************************************************
//
// Function Name: PoressDeviceNotifyCallbackFun
//
// Description:   Implement the Notify call back function of device client.
//
// Notes:
//
//******************************************************************************
void PoressDeviceNotifyCallbackFun(UInt32 msgType, void* dataBuf, UInt32 dataLength)
{
    int i,j;
    
    for(i = 0 ; i < TOTAL_BCMDEVICE_NUM ; i++)
    {
        if(bcm_dev_results[i].notifyid_list != NULL && bcm_dev_results[i].notifyid_list_len != 0)
        {
            for(j = 0 ; j < bcm_dev_results[i].notifyid_list_len ; j++)
            {
                if(bcm_dev_results[i].notifyid_list[j] == msgType)
                {
                    if(bcm_dev_results[i].notifyCb != NULL)
                    {
                        KRIL_DEBUG(DBG_INFO,"Call Notify Callback of ClientID:%d\n",i+1);
                        bcm_dev_results[i].notifyCb(msgType, 1, dataBuf, dataLength);
                    }
                    else
                    {
                        KRIL_DEBUG(DBG_ERROR,"Notify Callback of ClientID:%d is NULL !!!\n",i+1);
                    }
                    break;
                }
            }
        }
    }    
}


//******************************************************************************
//
// Function Name: DisplayStkTextString
//
// Description:   Display STK text string
//
// Notes:
//
//******************************************************************************
void DisplayStkTextString(SATKString_t pSTKstr)
{
    Unicode_t codeType;
    UInt16    len;
    char strout[255];
    
    codeType = pSTKstr.unicode_type;
    len = pSTKstr.len;
     
    if ((len > 0) && (codeType == UNICODE_GSM || codeType == UNICODE_UCS1))
    {
        memcpy(strout, pSTKstr.string, len);
        strout[len] = '\0';
        
        KRIL_DEBUG(DBG_INFO,"string: %s\n",strout);
    }
}


//******************************************************************************
//
// Function Name: ParseSetupMenuCommandQualifier
//
// Description:   Parse Setup Menu Command Qualifier data
//
// Notes:
//
//******************************************************************************
void ParseSetupMenuCommandQualifier(UInt8 *cmdQualifierData, SetupMenu_t *pSetupMenu)
{
    if (pSetupMenu->isHelpAvailable)
        *cmdQualifierData |= 0x80;
    else
        *cmdQualifierData &= ~0x80;    
    
    KRIL_DEBUG(DBG_INFO,"*cmdQualifierData:0x%02X\n",*cmdQualifierData);    
}


//******************************************************************************
//
// Function Name: ParseSelectItemCommandQualifier
//
// Description:   Parse Select Item Command Qualifier data
//
// Notes:
//
//******************************************************************************
void ParseSelectItemCommandQualifier(UInt8 *cmdQualifierData, SelectItem_t *pSelectItem)
{
    if (pSelectItem->isHelpAvailable)
        *cmdQualifierData |= 0x80;
    else
        *cmdQualifierData &= ~0x80;    
    
    KRIL_DEBUG(DBG_INFO,"*cmdQualifierData:0x%02X\n",*cmdQualifierData);
}


//******************************************************************************
//
// Function Name: ParseGetInputCommandQualifier
//
// Description:   Parse Get Input Command Qualifier data
//
// Notes:
//
//******************************************************************************
void ParseGetInputCommandQualifier(UInt8 *cmdQualifierData, GetInput_t *pGetInput)
{
    switch (pGetInput->inPutType)
    {
        case S_IKT_DIGIT:
            *cmdQualifierData &= ~0x03;
            break;
        
        case S_IKT_SMSDEFAULTSET:
            *cmdQualifierData |= 0x01;
            *cmdQualifierData &= ~0x02;
            break;
        
        case S_IKT_UCS2:
            *cmdQualifierData |= 0x01;
            *cmdQualifierData |= 0x02;
            break;
        
        default:
            KRIL_DEBUG(DBG_ERROR,"Not Support inPutType:%d\n",pGetInput->inPutType);
            break;
    }
    
    if (pGetInput->isEcho)
        *cmdQualifierData &= ~0x04;
    else
        *cmdQualifierData |= 0x04;
    
    if (pGetInput->isPacked)
        *cmdQualifierData |= 0x08;
    else
        *cmdQualifierData &= ~0x08;
        
    if (pGetInput->isHelpAvailable)
        *cmdQualifierData |= 0x80;
    else
        *cmdQualifierData &= ~0x80;
    
    KRIL_DEBUG(DBG_INFO,"*cmdQualifierData:0x%02X\n",*cmdQualifierData);
}


//******************************************************************************
//
// Function Name: ParseGetInkeyCommandQualifier
//
// Description:   Parse Get Inkey Command Qualifier data
//
// Notes:
//
//******************************************************************************
void ParseGetInkeyCommandQualifier(UInt8 *cmdQualifierData, GetInkey_t *pGetInkey)
{
    switch (pGetInkey->inKeyType)
    {
        case S_IKT_YESNO:
            *cmdQualifierData &= ~0x03;
            *cmdQualifierData |= 0x04;
            break;
        
        case S_IKT_DIGIT:
            *cmdQualifierData &= ~0x07;
            break;
        
        case S_IKT_SMSDEFAULTSET:
            *cmdQualifierData |= 0x01;
            *cmdQualifierData &= ~0x02;
            break;
        
        case S_IKT_UCS2:
            *cmdQualifierData |= 0x01;
            *cmdQualifierData |= 0x02;
            break;        
    }

    if (pGetInkey->isHelpAvailable)
        *cmdQualifierData |= 0x80;
    else
        *cmdQualifierData &= ~0x80;
    
    KRIL_DEBUG(DBG_INFO,"*cmdQualifierData:0x%02X\n",*cmdQualifierData);
}


//******************************************************************************
//
// Function Name: ParseDisplayTextCommandQualifier
//
// Description:   Parse Display Text Command Qualifier data
//
// Notes:
//
//******************************************************************************
void ParseDisplayTextCommandQualifier(UInt8 *cmdQualifierData, DisplayText_t *pDisplayText)
{
    if (pDisplayText->isHighPrio)
        *cmdQualifierData |= 0x01;
    else
        *cmdQualifierData &= ~0x01;
    
    if (pDisplayText->isDelay)
        *cmdQualifierData &= ~0x08;
    else
        *cmdQualifierData |= 0x08;
    
    KRIL_DEBUG(DBG_INFO,"*cmdQualifierData:0x%02X\n",*cmdQualifierData);
}


//******************************************************************************
//
// Function Name: ParseSendMOSMSCommandQualifier
//
// Description:   Parse Send MO SMS Command Qualifier data
//
// Notes:
//
//******************************************************************************
void ParseSendMOSMSCommandQualifier(UInt8 *cmdQualifierData, SendMOSMS_t *pSendMOSMS)
{
    *cmdQualifierData = 0x00;
    KRIL_DEBUG(DBG_INFO,"*cmdQualifierData:0x%02X\n",*cmdQualifierData);
}


//******************************************************************************
//
// Function Name: ParseSetupCallCommandQualifier
//
// Description:   Parse Setup Call Command Qualifier data
//
// Notes:
//
//******************************************************************************
void ParseSetupCallCommandQualifier(UInt8 *cmdQualifierData, SetupCall_t *pSetupCall)
{
    *cmdQualifierData = 0x00;
    
    switch (pSetupCall->callType)
    {
        case S_CT_ONIDLE:
            *cmdQualifierData = 0x00;
            break;
        
        case S_CT_ONIDLE_REDIALABLE:
            *cmdQualifierData = 0x01;
            break;
        
        case S_CT_HOLDABLE:
            *cmdQualifierData = 0x02;
            break;
        
        case S_CT_HOLDABLE_REDIALABLE:
            *cmdQualifierData = 0x03;
            break;
        
        case S_CT_DISCONNECTABLE:
            *cmdQualifierData = 0x04;
            break;
        
        case S_CT_DISCONNECTABLE_REDIALABLE:
            *cmdQualifierData = 0x05;
            break;
        
        default:
            KRIL_DEBUG(DBG_ERROR,"Unknow callType:0x%02X\n",pSetupCall->callType);
            break;
    }
    
    KRIL_DEBUG(DBG_INFO,"*cmdQualifierData:0x%02X\n",*cmdQualifierData);
}


//******************************************************************************
//
// Function Name: ParseRefreshCommandQualifier
//
// Description:   Parse Refresh Command Qualifier data
//
// Notes:
//
//******************************************************************************
void ParseRefreshCommandQualifier(UInt8 *cmdQualifierData, Refresh_t *pRefresh)
{
    *cmdQualifierData = 0x00;
    
    switch (pRefresh->refreshType)
    {
        case SMRT_INIT_FULLFILE_CHANGED:
            *cmdQualifierData = 0x00;
            break;
        
        case SMRT_FILE_CHANGED:
            *cmdQualifierData = 0x01;
            break;
        
        case SMRT_INIT_FILE_CHANGED:
            *cmdQualifierData = 0x02;
            break;
        
        case SMRT_INIT:
            *cmdQualifierData = 0x03;
            break;
        
        case SMRT_RESET:
            *cmdQualifierData = 0x04;
            break;
        
        default:
            KRIL_DEBUG(DBG_ERROR,"Unknow refreshType:%d\n",pRefresh->refreshType);
            break;
    }
    
    KRIL_DEBUG(DBG_INFO,"*cmdQualifierData:0x%02X\n",*cmdQualifierData);
}


//******************************************************************************
//
// Function Name: ParseLaunchBrowserCommandQualifier
//
// Description:   Parse Launch Browser Command Qualifier data
//
// Notes:
//
//******************************************************************************
void ParseLaunchBrowserCommandQualifier(UInt8 *cmdQualifierData, LaunchBrowserReq_t *pLaunchBrowserReq)
{
    *cmdQualifierData = 0x00;
    
    switch (pLaunchBrowserReq->browser_action)
    {
        case LAUNCH_BROWSER_NO_CONNECT:
            *cmdQualifierData = 0x00;
            break;
        
        case LAUNCH_BROWSER_CONNECT:
            *cmdQualifierData = 0x01;
            break;
        
        case LAUNCH_BROWSER_USE_EXIST:
            *cmdQualifierData = 0x02;
            break;
        
        case LAUNCH_BROWSER_CLOSE_EXIST_NORMAL:
            *cmdQualifierData = 0x03;
            break;
        
        case LAUNCH_BROWSER_CLOSE_EXIST_SECURE:
            *cmdQualifierData = 0x04;
            break;
        
        default:
            KRIL_DEBUG(DBG_ERROR,"Unknow browser_action:%d\n",pLaunchBrowserReq->browser_action);
            break;
    }
    
    KRIL_DEBUG(DBG_INFO,"*cmdQualifierData:0x%02X\n",*cmdQualifierData);    
}


//******************************************************************************
//
// Function Name: ParseCommandDetails
//
// Description:   Parse Command Details(refer to 11.14 section 12.6)
//
// Notes:
//
//******************************************************************************
UInt16 ParseCommandDetails(UInt8 *simple_tlv_ptr, void *pSATKdata, UInt8 cmdType)
{
    UInt8 command_detail_data[5] = {0x81, 0x03, 0x01, 0x00, 0x00};
    UInt16 command_detail_length = 0;
    
    // Fill command type
    command_detail_data[3] = cmdType;
    
    // Parse Command Qualifier
    switch (cmdType)
    {
        case STK_SETUPMENU:
        {
            SetupMenu_t *pSetupMenu = (SetupMenu_t*)pSATKdata;
            ParseSetupMenuCommandQualifier(&command_detail_data[4], pSetupMenu);
            break;
        }
        
        case STK_SELECTITEM:
        {
            SelectItem_t *pSelectItem = (SelectItem_t*)pSATKdata;
            ParseSelectItemCommandQualifier(&command_detail_data[4], pSelectItem);
            break;
        }
            
        case STK_GETINPUT:
        {
            GetInput_t *pGetInput = (GetInput_t*)pSATKdata;
            ParseGetInputCommandQualifier(&command_detail_data[4], pGetInput);
            break;
        }
        
        case STK_GETINKEY:
        {
            GetInkey_t *pGetInkey = (GetInkey_t*)pSATKdata;
            ParseGetInkeyCommandQualifier(&command_detail_data[4], pGetInkey);
            break;
        }
        
        case STK_DISPLAYTEXT:
        {
            DisplayText_t *pDisplayText = (DisplayText_t*)pSATKdata;
            ParseDisplayTextCommandQualifier(&command_detail_data[4], pDisplayText);
            break;
        }
        
        case STK_SENDSMS:
        {
            SendMOSMS_t *pSendMOSMS = (SendMOSMS_t*)pSATKdata;
            ParseSendMOSMSCommandQualifier(&command_detail_data[4], pSendMOSMS);
            break;
        }
        
        case STK_SETUPCALL:
        {
            SetupCall_t *pSetupCall = (SetupCall_t*)pSATKdata;
            ParseSetupCallCommandQualifier(&command_detail_data[4], pSetupCall);
            break;
        }
        
        case STK_REFRESH:
        {
            Refresh_t *pRefresh = (Refresh_t*)pSATKdata;
            ParseRefreshCommandQualifier(&command_detail_data[4], pRefresh);
            break;
        }
        
        case STK_LAUNCHBROWSER:
        {
            LaunchBrowserReq_t *pLaunchBrowserReq = (LaunchBrowserReq_t*)pSATKdata;
            ParseLaunchBrowserCommandQualifier(&command_detail_data[4], pLaunchBrowserReq);
            break;
        }
        
        case STK_SENDSS:
        case STK_SENDUSSD:
        case STK_EVENTLIST:
        case STK_PLAYTONE:
        case STK_SENDDTMF:
        case STK_SETUPIDLEMODETEXT:   
            // The command qualifier is RFU, there is nothing to do
            break;
        
        default:
            KRIL_DEBUG(DBG_ERROR,"Not Support Command type:0x%02X\n",cmdType);
            return 0;
    }

    command_detail_length = sizeof(command_detail_data)/sizeof(UInt8);
    memcpy(simple_tlv_ptr, command_detail_data, command_detail_length);    
    return command_detail_length;
}


//******************************************************************************
//
// Function Name: ParseDeviceIdentities
//
// Description:   Parse Device Identities(refer to 11.14 section 12.7)
//
// Notes:
//
//******************************************************************************
UInt16 ParseDeviceIdentities(UInt8 *simple_tlv_ptr, UInt8 cmdType)
{
    UInt8 device_identities_data[4] = {0x82, 0x02, 0x81, 0x82};
    UInt16 device_identities_length = 0;
    
    switch (cmdType)
    {
        case STK_SETUPCALL:
        case STK_SENDSS:
        case STK_SENDUSSD:
        case STK_SENDSMS:
        case STK_SENDDTMF:
            device_identities_data[3] = 0x83;
            break;
        
        default:
            device_identities_data[3] = 0x82;
            break;
    }

    device_identities_length = sizeof(device_identities_data)/sizeof(UInt8);
    memcpy(simple_tlv_ptr, device_identities_data, device_identities_length);  
    return device_identities_length;
}


//******************************************************************************
//
// Function Name: ParseAlphaIdentifier
//
// Description:   Parse Alpha Identifier(refer to 11.14 section 12.2)
//
// Notes:
//
//******************************************************************************
UInt16 ParseAlphaIdentifier(UInt8 *simple_tlv_ptr, SATKString_t title)
{
    UInt16 total_byte  = 0;
    UInt16 alphaid_byte = 0;
    UInt16 alpha_length = 0;
    UInt8  *tlv_ptr = simple_tlv_ptr;
    
    // Fill Alpha identifier tag
    tlv_ptr[0] = 0x85;
    tlv_ptr += 1;
    total_byte += 1;
    
    // Fill Length
    if (title.len == 0)
    {
        KRIL_DEBUG(DBG_INFO,"The text string length is 0\n");
        tlv_ptr[0] = 0x00;
        tlv_ptr += 1;
        total_byte += 1;
        return total_byte;
    }
    
    if (IS_ALPHAID_CODING(title.unicode_type))
        alphaid_byte = 1;

    alpha_length = alphaid_byte + title.len;
    
    if (alpha_length > 127)
    {
        tlv_ptr[0] = 0x81;
        tlv_ptr[1] = alpha_length;
        tlv_ptr += 2;
        total_byte += 2;
    }
    else
    {
        tlv_ptr[0] = alpha_length;
        tlv_ptr += 1;
        total_byte += 1;
    }
    
    // Fill Alpha identifier
    if (IS_ALPHAID_CODING(title.unicode_type))
    {
        tlv_ptr[0] = 0x80;
        tlv_ptr += 1;
        total_byte += 1;
    }

    memcpy(tlv_ptr, title.string, title.len);
    total_byte += title.len;
    //RawDataPrintfun(simple_tlv_ptr, total_byte, "AlphaId");
    return total_byte;
}


//******************************************************************************
//
// Function Name: ParseItemData
//
// Description:   Parse Item Data Object(refer to 11.14 section 12.9)
//
// Notes:
//
//******************************************************************************
UInt16 ParseItemData(UInt8 *simple_tlv_ptr, UInt8 numItems, UInt8 *pItemIdList, SATKString_t *pItemList)
{
    UInt8 i;
    UInt16 total_byte   = 0;
    UInt16 alphaid_byte = 0;
    UInt16 itemid_byte  = 1;
    UInt16 item_length  = 0;
    UInt16 total_sum    = 0;
    UInt8  *tlv_ptr = NULL;
    
    // If the "item data object for item 1" is a null data object(i.e. length = "00"
    // and no value part), this is an indication to the ME to remove the existing menu.
    if (0 == numItems)
    {
        tlv_ptr = simple_tlv_ptr;
        
        // Fill item tag
        tlv_ptr[0] = 0x8F;
        
        // Fill Length
        tlv_ptr[1] = 0;
        
        total_sum = 2;
    }
    else
    {
        for (i = 0 ; i < numItems ; i++)
        {
            tlv_ptr = simple_tlv_ptr;
            total_byte = 0;
            
            if (IS_ALPHAID_CODING(pItemList[i].unicode_type))
                alphaid_byte = 1;
            else
                alphaid_byte = 0;
            
            // Fill item tag
            tlv_ptr[0] = 0x8F;
            tlv_ptr += 1;
            total_byte += 1;
            
            // Fill Length
            item_length = itemid_byte + alphaid_byte + pItemList[i].len;
            if (item_length > 127)
            {
                tlv_ptr[0] = 0x81;
                tlv_ptr[1] = item_length;
                tlv_ptr += 2;
                total_byte += 2;
            }
            else
            {
                tlv_ptr[0] = item_length;
                tlv_ptr += 1;
                total_byte += 1;
            }
            
            // Fill Identifier od item
            tlv_ptr[0] = pItemIdList[i];
            tlv_ptr += 1;
            total_byte += 1;
            
            // Fill Text string of item
            if (IS_ALPHAID_CODING(pItemList[i].unicode_type))
            {
                tlv_ptr[0] = 0x80;
                tlv_ptr += 1;
                total_byte += 1;
            }        
            
            memcpy(tlv_ptr, pItemList[i].string, pItemList[i].len);
            total_byte += pItemList[i].len;
            KRIL_DEBUG(DBG_INFO,"i:%d itemID:%d total_byte:%d\n",i,pItemIdList[i],total_byte);
            DisplayStkTextString(pItemList[i]);
            
            //RawDataPrintfun(simple_tlv_ptr, total_byte, "item");
            simple_tlv_ptr += total_byte;
            total_sum += total_byte;
        }
    }
    
    KRIL_DEBUG(DBG_INFO,"total_sum:%d\n",total_sum);
    return total_sum;
}


//******************************************************************************
//
// Function Name: ParseTextString
//
// Description:   Parse TextString(refer to 11.14 section 12.15)
//
// Notes:
//
//******************************************************************************
UInt16 ParseTextString(UInt8 *simple_tlv_ptr, SATKString_t textStr)
{
    UInt16 total_byte  = 0;
    UInt16 codingscheme_byte = 1;
    UInt16 text_length = 0;
    UInt8  *tlv_ptr = simple_tlv_ptr;
    
    // Fill Text string tag
    tlv_ptr[0] = 0x8D;
    tlv_ptr += 1;
    total_byte += 1;
    
    // Fill Length
    if (textStr.len == 0)
    {
        KRIL_DEBUG(DBG_INFO,"The text string length is 0\n");
        tlv_ptr[0] = 0x00;
        tlv_ptr += 1;
        total_byte += 1;
        return total_byte;
    }

    text_length = codingscheme_byte + textStr.len;
    
    if (text_length > 127)
    {
        tlv_ptr[0] = 0x81;
        tlv_ptr[1] = text_length;
        tlv_ptr += 2;
        total_byte += 2;
    }
    else
    {
        tlv_ptr[0] = text_length;
        tlv_ptr += 1;
        total_byte += 1;
    }
    
    // Fill Data coding scheme
    switch (textStr.unicode_type)
    {
        case UNICODE_GSM:
        case UNICODE_UCS1:
            tlv_ptr[0] = 0x04;
            tlv_ptr += 1;
            total_byte += 1;
            break;
        
        case UNICODE_80:
            tlv_ptr[0] = 0x08;
            tlv_ptr += 1;
            total_byte += 1;
            break;
        
        default:
            KRIL_DEBUG(DBG_ERROR,"Not Supported code type:0x%02X\n",textStr.unicode_type);
            return 0;
    }
    
    memcpy(tlv_ptr, textStr.string, textStr.len);
    total_byte += textStr.len;
    //RawDataPrintfun(simple_tlv_ptr, total_byte, "TextStr");    
    
    return total_byte;
}


//******************************************************************************
//
// Function Name: ParseDefaultTextString
//
// Description:   Parse Default TextString(refer to 11.14 section 12.23)
//
// Notes:
//
//******************************************************************************
UInt16 ParseDefaultTextString(UInt8 *simple_tlv_ptr, SATKString_t textStr)
{
    UInt16 total_byte  = 0;
    UInt16 codingscheme_byte = 1;
    UInt16 text_length = 0;
    UInt8  *tlv_ptr = simple_tlv_ptr;
    
    // Fill Text string tag
    tlv_ptr[0] = 0x97;
    tlv_ptr += 1;
    total_byte += 1;
    
    // Fill Length
    if (textStr.len == 0)
    {
        KRIL_DEBUG(DBG_INFO,"The text string length is 0\n");
        tlv_ptr[0] = 0x00;
        tlv_ptr += 1;
        total_byte += 1;
        return total_byte;
    }

    text_length = codingscheme_byte + textStr.len;
    
    if (text_length > 127)
    {
        tlv_ptr[0] = 0x81;
        tlv_ptr[1] = text_length;
        tlv_ptr += 2;
        total_byte += 2;
    }
    else
    {
        tlv_ptr[0] = text_length;
        tlv_ptr += 1;
        total_byte += 1;
    }
    
    // Fill Data coding scheme
    switch (textStr.unicode_type)
    {
        case UNICODE_GSM:
        case UNICODE_UCS1:
            tlv_ptr[0] = 0x04;
            tlv_ptr += 1;
            total_byte += 1;
            break;
        
        case UNICODE_80:
            tlv_ptr[0] = 0x08;
            tlv_ptr += 1;
            total_byte += 1;
            break;
        
        default:
            KRIL_DEBUG(DBG_ERROR,"Not Supported code type:0x%02X\n",textStr.unicode_type);
            return 0;
    }
    
    memcpy(tlv_ptr, textStr.string, textStr.len);
    total_byte += textStr.len;
    //RawDataPrintfun(simple_tlv_ptr, total_byte, "TextStr");    
    
    return total_byte;
}


//******************************************************************************
//
// Function Name: ParseResponseLength
//
// Description:   Parse Response Length(refer to 11.14 section 12.11)
//
// Notes:
//
//******************************************************************************
UInt16 ParseResponseLength(UInt8 *simple_tlv_ptr, UInt8 minLen, UInt8 maxLen)
{
    UInt8 responseLength_data[4] = {0x91, 0x02, 0x00, 0x00};
    UInt16 total_byte  = 0;
    
    // Fill maximum and minimum length od response
    responseLength_data[2] = minLen;
    responseLength_data[3] = maxLen;
    
    total_byte = 4;
    memcpy(simple_tlv_ptr, responseLength_data, total_byte);
    return total_byte;
}


//******************************************************************************
//
// Function Name: ParseImmediateResponse
//
// Description:   Parse Immediate Response (refer to 11.14 section 12.43)
//
// Notes:
//
//******************************************************************************
void ParseImmediateResponse(UInt8 *simple_tlv_ptr)
{
    UInt8 immediateresponse_data[2] = {0xAB, 0x00};
    
    memcpy(simple_tlv_ptr, immediateresponse_data, 2);
}


//******************************************************************************
//
// Function Name: ParseTone
//
// Description:   Parse Tone (refer to 11.14 section 12.16)
//
// Notes:
//
//******************************************************************************
UInt16 ParseTone(UInt8 *simple_tlv_ptr, SATKToneType_t toneType)
{
    UInt8 tone_data[3] = {0x8E, 0x01, 0x00};
    UInt16 tone_length = 3;
    
    switch (toneType)
    {
        case S_TT_DEFAULTTONE:
            tone_data[2] = 0x10;
            break;
        
        case S_TT_DIALTONE:
            tone_data[2] = 0x01;
            break;
        
        case S_TT_CALLEDUSERBUSY:
            tone_data[2] = 0x02;
            break;
        
        case S_TT_CONGEST:
            tone_data[2] = 0x03;
            break;
        
        case S_TT_RADIOPATHACK:
            tone_data[2] = 0x04;
            break;
        
        case S_TT_RADIOPATHUNAVAILABLE:
            tone_data[2] = 0x05;
            break;
        
        case S_TT_ERROR:
            tone_data[2] = 0x06;
            break;
        
        case S_TT_CALLWAITING:
            tone_data[2] = 0x07;
            break;
        
        case S_TT_RINGING:
            tone_data[2] = 0x08;
            break;
          
        case S_TT_BEEP:
            tone_data[2] = 0x10;
            break;
        
        case S_TT_POSITIVEACK:
            tone_data[2] = 0x11;
            break;
        
        case S_TT_NEGTIVEACK:
            tone_data[2] = 0x12;
            break;
 
        default:
            KRIL_DEBUG(DBG_ERROR,"Unknow tone type:0x%02X\n",toneType);
            return 0;
    }
    
    memcpy(simple_tlv_ptr, tone_data, tone_length);
    return tone_length;
}


//******************************************************************************
//
// Function Name: ParseAddress
//
// Description:   Parse Address (refer to 11.14 section 12.1)
//
// Notes:
//
//******************************************************************************
UInt16 ParseAddress(UInt8 *simple_tlv_ptr, SATKNum_t num)
{
    UInt16 total_byte  = 0;
    UInt16 address_length = 0;
    UInt16 number_length = 0;
    UInt16 diallingNum_length = 0;
    UInt8  i;
    UInt8  *tlv_ptr = simple_tlv_ptr;
    
    // Fill Alpha identifier tag
    tlv_ptr[0] = 0x86;
    tlv_ptr += 1;
    total_byte += 1;
    
    // Fill the length
    number_length = strlen(num.Num);
    diallingNum_length = (number_length + 1)/2;
    address_length = 1 + diallingNum_length;
    
    if (address_length > 127)
    {
        tlv_ptr[0] = 0x81;
        tlv_ptr[1] = address_length;
        tlv_ptr += 2;
        total_byte += 2;
    }
    else
    {
        tlv_ptr[0] = address_length;
        tlv_ptr += 1;
        total_byte += 1;
    }
    
    // Fill TON and NPI
    tlv_ptr[0] = num.Npi | (num.Ton << 4) | 0x80;
    tlv_ptr += 1;
    total_byte += 1;
    
    // Fill Dialling number string
    for (i = 0 ; i < diallingNum_length ; i++)
    {
        // reset the value of tlv_ptr[i]
        tlv_ptr[i] = 0;
        
        if ((2*i) < number_length)
        {
            if (num.Num[2*i] == 0x50 || num.Num[2*i] == 0x70)
            {
                tlv_ptr[i] |= 0x0C;
            }
            else
            {
                tlv_ptr[i] |= ((num.Num[2*i] - 0x30)& 0x0F);
            }
        }
        
        if ((2*i+1) < number_length)
        {
            if (num.Num[2*i+1] == 0x50 || num.Num[2*i+1] == 0x70)
            {
                tlv_ptr[i] |= 0xC0;
            }
            else
            {
                tlv_ptr[i] |= (((num.Num[2*i+1] - 0x30)& 0x0F)<<4);
            }            
        }
    }
    
    if ((number_length % 2) != 0)
    {
        tlv_ptr[diallingNum_length - 1] |= 0xF0;
    }
    
    tlv_ptr += diallingNum_length;
    total_byte += diallingNum_length;
    
    //RawDataPrintfun(simple_tlv_ptr, total_byte, "Address");
    return total_byte;    
}


//******************************************************************************
//
// Function Name: ParseFileList
//
// Description:   Parse File List (refer to 11.14 section 12.18)
//
// Notes:
//
//******************************************************************************
UInt16 ParseFileList(UInt8 *simple_tlv_ptr, REFRESH_FILE_LIST_t	FileIdList)
{
    UInt16 total_byte  = 0;
    UInt16 files_length = 0;
    UInt8  i,j;
    UInt8  *tlv_ptr = simple_tlv_ptr;
    UInt8  *pfile_path = NULL;
    UInt16  file_path_len = 0;
    
    // Fill File List tag
    tlv_ptr[0] = 0x92;
    tlv_ptr += 1;
    total_byte += 1;
    
    // Fill the length
    files_length += 1;
    
    for (i = 0 ; i < FileIdList.number_of_file ; i++)
    {
        files_length += (FileIdList.changed_file[i].path_len*2);
    }
    
    if (files_length > 127)
    {
        tlv_ptr[0] = 0x81;
        tlv_ptr[1] = files_length;
        tlv_ptr += 2;
        total_byte += 2;
    }
    else
    {
        tlv_ptr[0] = files_length;
        tlv_ptr += 1;
        total_byte += 1;
    }
    
    // Fill number of files
    tlv_ptr[0] = FileIdList.number_of_file;
    tlv_ptr += 1;
    total_byte += 1;
    
    // Fill Files
    for (i = 0 ; i < FileIdList.number_of_file ; i++)
    {
        KRIL_DEBUG(DBG_INFO,"changed_file[%d]: path_len:%d\n",i,FileIdList.changed_file[i].path_len);
        pfile_path = (UInt8*)FileIdList.changed_file[i].file_path;
        file_path_len = FileIdList.changed_file[i].path_len*2;
        RawDataPrintfun(pfile_path, file_path_len, "file_path");
        
        for (j = 0 ; j < FileIdList.changed_file[i].path_len ; j++)
        {
            tlv_ptr[2*j] = pfile_path[2*j+1];
            tlv_ptr[2*j+1] = pfile_path[2*j];
        }
        tlv_ptr += file_path_len;
        total_byte += file_path_len;
    }    

    //RawDataPrintfun(simple_tlv_ptr, total_byte, "File List");
    return total_byte;    
}


//******************************************************************************
//
// Function Name: ParseItemsNextActionIndicator
//
// Description:   Parse Items Next Action Indicator (refer to 11.14 section 12.24)
//
// Notes:
//
//******************************************************************************
UInt16 ParseItemsNextActionIndicator(UInt8 *simple_tlv_ptr, UInt8 numItems, STKIconListId_t pNextActIndList)
{
    UInt16 total_byte  = 0;
    UInt8  *tlv_ptr = simple_tlv_ptr;
    
    // Fill Items Next Action Indicator tag
    tlv_ptr[0] = 0x18;
    tlv_ptr += 1;
    total_byte += 1;
    
    // Fill length
    tlv_ptr[0] = numItems;
    tlv_ptr += 1;
    total_byte += 1;
    
    // Fill Items Next Action Indicator list
    memcpy(tlv_ptr, pNextActIndList.Id, numItems);
    tlv_ptr += numItems;
    total_byte += numItems;
    
    //RawDataPrintfun(simple_tlv_ptr, total_byte, "Next Action Indicator");
    return total_byte;
}


//******************************************************************************
//
// Function Name: ParseItemIdentifier
//
// Description:   Parse Item Identifier (refer to 11.14 section 12.10)
//
// Notes:
//
//******************************************************************************
UInt16 ParseItemIdentifier(UInt8 *simple_tlv_ptr, UInt8 defaultItem)
{
    UInt16 total_byte  = 0;
    UInt8  *tlv_ptr = simple_tlv_ptr;    

    // Fill Item Identifier tag
    tlv_ptr[0] = 0x90;
    tlv_ptr += 1;
    total_byte += 1;    

    // Fill length
    tlv_ptr[0] = 0x01;
    tlv_ptr += 1;
    total_byte += 1;

    // Fill Identifier od item chosen
    tlv_ptr[0] = defaultItem;
    tlv_ptr += 1;
    total_byte += 1;

    //RawDataPrintfun(simple_tlv_ptr, total_byte, "Item Identifier");
    return total_byte;
}


//******************************************************************************
//
// Function Name: ParseIconIdentifier
//
// Description:   Parse Icon Identifier (refer to 11.14 section 12.31)
//
// Notes:
//
//******************************************************************************
UInt16 ParseIconIdentifier(UInt8 *simple_tlv_ptr, SATKIcon_t icon)
{
    UInt16 total_byte  = 0;
    UInt8  *tlv_ptr = simple_tlv_ptr;
    
    // Fill Icon Identifier tag
    tlv_ptr[0] = 0x9E;
    tlv_ptr += 1;
    total_byte += 1;
    
    // Fill length
    tlv_ptr[0] = 0x02;
    tlv_ptr += 1;
    total_byte += 1;    
        
    // Fill Icon qualifier
    if (icon.IsSelfExplanatory)
    {
        tlv_ptr[0] = 0x00;
    }
    else
    {
        tlv_ptr[0] = 0x01;
    }
    
    tlv_ptr += 1;
    total_byte += 1;
    
    // Fill Icon identifier
    tlv_ptr[0] = icon.Id;
    tlv_ptr += 1;
    total_byte += 1;
    
    //RawDataPrintfun(simple_tlv_ptr, total_byte, "Icon Identifier");
    return total_byte;    
}


//******************************************************************************
//
// Function Name: ParseIconIdentifierList
//
// Description:   Parse Icon Identifier List (refer to 11.14 section 12.32)
//
// Notes:
//
//******************************************************************************
UInt16 ParseIconIdentifierList(UInt8 *simple_tlv_ptr, UInt8 numItems, STKIconListId_t pIconList)
{
    UInt16 total_byte  = 0;
    UInt8  *tlv_ptr = simple_tlv_ptr;

    // Fill Icon Identifier tag
    tlv_ptr[0] = 0x9F;
    tlv_ptr += 1;
    total_byte += 1;
    
    // Fill length
    tlv_ptr[0] = numItems+1;
    tlv_ptr += 1;
    total_byte += 1;
    
    // Fill Icon list qualifier
    if (pIconList.IsSelfExplanatory)
    {
        tlv_ptr[0] = 0x00;
    }
    else
    {
        tlv_ptr[0] = 0x01;
    }
    
    tlv_ptr += 1;
    total_byte += 1;
    
    // Fill Icon identifier list
    memcpy(tlv_ptr, pIconList.Id, numItems);
    tlv_ptr += numItems;
    total_byte += numItems;

    //RawDataPrintfun(simple_tlv_ptr, total_byte, "Icon Identifier List");
    return total_byte;
}


//******************************************************************************
//
// Function Name: ParserBrowserIdentity
//
// Description:   Parse Browser Identity (refer to 11.14 section 12.47)
//
// Notes:
//
//******************************************************************************
UInt16 ParserBrowserIdentity(UInt8 *simple_tlv_ptr, UInt8 browser_id)
{
    UInt16 total_byte  = 0;
    UInt8  *tlv_ptr = simple_tlv_ptr;

    // Fill Browser Identity tag
    tlv_ptr[0] = 0x30;
    tlv_ptr += 1;
    total_byte += 1;
    
    // Fill length
    tlv_ptr[0] = 1;
    tlv_ptr += 1;
    total_byte += 1;
    
    // Fill Browser Identity
    tlv_ptr[0] = browser_id;
    tlv_ptr += 1;
    total_byte += 1;    

    //RawDataPrintfun(simple_tlv_ptr, total_byte, "Browser Identity");
    return total_byte;
}


//******************************************************************************
//
// Function Name: ParseURL
//
// Description:   Parse URL (refer to 11.14 section 12.48)
//
// Notes:
//
//******************************************************************************
UInt16 ParseURL(UInt8 *simple_tlv_ptr, char *url)
{
    UInt16 total_byte  = 0;
    UInt8  *tlv_ptr = simple_tlv_ptr;
    UInt8  url_str_len = 0;
    //char*  defaultUrl = "http://www.android.com";

    // Fill URL tag
    tlv_ptr[0] = 0x31;
    tlv_ptr += 1;
    total_byte += 1;
    
    // Fill length and URL
    url_str_len = strlen(url);
    
    if (url_str_len > 0)
    {
        tlv_ptr[0] = url_str_len;
        tlv_ptr += 1;
        total_byte += 1;
        
        memcpy(tlv_ptr, url, url_str_len);
        tlv_ptr += url_str_len;
        total_byte += url_str_len;
    }
    else
    {
        //KRIL_DEBUG(DBG_INFO,"Use default URL\n");
        //url_str_len = strlen(defaultUrl);
        tlv_ptr[0] = 0;
        tlv_ptr += 1;
        total_byte += 1;
        
        //memcpy(tlv_ptr, defaultUrl, url_str_len);
        //tlv_ptr += url_str_len;
        //total_byte += url_str_len;        
    }

    //RawDataPrintfun(simple_tlv_ptr, total_byte, "URL");
    return total_byte;
}


//******************************************************************************
//
// Function Name: ParseBear
//
// Description:   Parse Bear (refer to 11.14 section 12.49)
//
// Notes:
//
//******************************************************************************
UInt16 ParseBear(UInt8 *simple_tlv_ptr, LaunchBrowserReq_t *pLaunchBrowserReq)
{
    UInt16 total_byte  = 0;
    UInt8  *tlv_ptr = simple_tlv_ptr;
    int i;

    // Fill Bear tag
    tlv_ptr[0] = 0x32;
    tlv_ptr += 1;
    total_byte += 1;

    // Fill Length
    tlv_ptr[0] = pLaunchBrowserReq->bearer_length;
    tlv_ptr += 1;
    total_byte += 1;

    // Fill List of bearers in order of priority requested
    for (i = 0 ; i < pLaunchBrowserReq->bearer_length ; i++)
    {
        tlv_ptr[0] = pLaunchBrowserReq->bearer[i];
        tlv_ptr += 1;
        total_byte += 1;
    }

    //RawDataPrintfun(simple_tlv_ptr, total_byte, "Bear");
    return total_byte;
}


//******************************************************************************
//
// Function Name: ParserProvisioning
//
// Description:   Parser Provisioning File Reference (refer to 11.14 section 12.50)
//
// Notes:
//
//******************************************************************************
UInt16 ParserProvisioning(UInt8 *simple_tlv_ptr, LaunchBrowserReq_t *pLaunchBrowserReq)
{
    UInt16 total_byte  = 0;
    UInt8  *tlv_ptr = simple_tlv_ptr;
    int i;

    for (i = 0 ; i < pLaunchBrowserReq->prov_length ; i++)
    {
        // Fill Provisioning File Reference tag
        tlv_ptr[0] = 0x33;
        tlv_ptr += 1;
        total_byte += 1;
        
        // Fill Length
        tlv_ptr[0] = pLaunchBrowserReq->prov_file[i].length;
        tlv_ptr += 1;
        total_byte += 1; 
        
        // Fill path to the provisioning file
        memcpy(tlv_ptr, pLaunchBrowserReq->prov_file[i].prov_file_data, 
        pLaunchBrowserReq->prov_file[i].length);
        tlv_ptr += pLaunchBrowserReq->prov_file[i].length;
        total_byte += pLaunchBrowserReq->prov_file[i].length;
    }
    
    //RawDataPrintfun(simple_tlv_ptr, total_byte, "Provisioning");
    return total_byte;
}


//******************************************************************************
//
// Function Name: ParseTlvData
//
// Description:   Parse TLV Data(refer to 11.14 section 13.2 and Annex D)
//
// Notes:
//
//******************************************************************************
UInt16 ParseTlvData(char *tlv_data_string, UInt8 *simple_tlv_objects, UInt16 simple_tlv_length)
{
    UInt8 tlv_object[MAX_TLV_DATA_LENGTH];
    UInt8* tlv_ptr = NULL;
    UInt16 tlv_length = 0;
    
    tlv_ptr = tlv_object;
    
    // Fill the Proactive command tag
    tlv_ptr[0] = 0xD0;
    tlv_ptr += 1;
    tlv_length += 1;
    
    // Fill the Length
    if (simple_tlv_length > MAX_SIMPLE_TLV_DATA_LENGTH)
    {
        KRIL_DEBUG(DBG_ERROR,"simple_tlv_length:%d is too large Error!!\n",simple_tlv_length);
        return 0;
    }
    
    if (simple_tlv_length > 127)
    {
        tlv_ptr[0] = 0x81;
        tlv_ptr[1] = simple_tlv_length;
        tlv_ptr += 2;
        tlv_length += 2;
    }
    else
    {
        tlv_ptr[0] = simple_tlv_length;
        tlv_ptr += 1;
        tlv_length += 1;
    }
    
    // Fill the TLV data objects
    memcpy(tlv_ptr, simple_tlv_objects, simple_tlv_length);
    tlv_length += simple_tlv_length;
    RawDataPrintfun(tlv_object, tlv_length, "TLV");
    
    // Convert the Hex data to Hex string
    HexDataToHexStr(tlv_data_string, tlv_object, tlv_length);
    
    return tlv_length;
}


//******************************************************************************
//
// Function Name: ParseSATKSetupMenu
//
// Description:   Parse the SATK Setup Menu(refer to 11.14 section 6.6.7)
//
// Notes:
//
//******************************************************************************
UInt16 ParseSATKSetupMenu(char* tlv_data_string, SetupMenu_t *pSetupMenu)
{
    UInt8 simple_tlv_objects[MAX_SIMPLE_TLV_DATA_LENGTH];
    UInt16 simple_tlv_length = 0;
    UInt8 *simple_tlv_ptr = NULL;
    UInt16 tlv_length = 0;
    UInt16 command_detail_length = 0;
    UInt16 device_identities_length = 0;
    UInt16 alphaidentifier_length = 0;
    UInt16 itemdata_length = 0;
    UInt16 nextactionindicator_length = 0;
    UInt16 iconidentifier_length = 0;
    UInt16 iconidentifierlist_length = 0;
    
    KRIL_DEBUG(DBG_INFO,"listSize:%d isHelpAvailable:%d title.unicode_type:0x%02X len:%d pNextActIndList.IsExist:%d\n",
        pSetupMenu->listSize,
        pSetupMenu->isHelpAvailable,
        pSetupMenu->title.unicode_type,
        pSetupMenu->title.len,
        pSetupMenu->pNextActIndList.IsExist
        );
    
    KRIL_DEBUG(DBG_INFO,"titleIcon.IsExist:%d Id:%d IsSelfExplanatory:%d pIconList.IsExist:%d Id[0]:%d Id[1]:%d Id[2]:%d IsSelfExplanatory:%d\n",
        pSetupMenu->titleIcon.IsExist,
        pSetupMenu->titleIcon.Id,
        pSetupMenu->titleIcon.IsSelfExplanatory,
        pSetupMenu->pIconList.IsExist,
        pSetupMenu->pIconList.Id[0],
        pSetupMenu->pIconList.Id[1],
        pSetupMenu->pIconList.Id[2],
        pSetupMenu->pIconList.IsSelfExplanatory
        );

    simple_tlv_ptr = simple_tlv_objects;
    
    // Fill Command detail data
    command_detail_length = ParseCommandDetails(simple_tlv_ptr, (void*)pSetupMenu, STK_SETUPMENU);
    if (!command_detail_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseCommandDetails() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += command_detail_length;
    
    // Fill Device identities
    device_identities_length = ParseDeviceIdentities(simple_tlv_ptr, STK_SETUPMENU);
    if (!device_identities_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseDeviceIdentities() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += device_identities_length;
    
    // Parse Alpha identifier data    
    if (pSetupMenu->title.len > 0)
    {
        alphaidentifier_length = ParseAlphaIdentifier(simple_tlv_ptr, pSetupMenu->title);
        if (!alphaidentifier_length)
        {
            KRIL_DEBUG(DBG_ERROR,"ParseAlphaIdentifier() return 0 Error!!\n");
            return 0;
        }
        
        simple_tlv_ptr += alphaidentifier_length;
    }
    else
    {
        // Set default SIM Menu title as "SIM MENU".
        UInt8 default_title[] = {0x53, 0x49, 0x4D, 0x20, 0x4D, 0x45, 0x4E, 0x55};
        UInt16 default_title_len = sizeof(default_title)/sizeof(UInt8);
        
        simple_tlv_ptr[0] = 0x85;
        simple_tlv_ptr[1] = default_title_len;
        simple_tlv_ptr += 2;
        
        memcpy(simple_tlv_ptr, default_title, default_title_len);
        simple_tlv_ptr += default_title_len;
        alphaidentifier_length = default_title_len + 2;
    }
    
    // Parse item data object
    itemdata_length = ParseItemData(simple_tlv_ptr, pSetupMenu->listSize, pSetupMenu->pItemIdList,
        pSetupMenu->pItemList);
    if (!itemdata_length)
    {
       KRIL_DEBUG(DBG_ERROR,"ParseItemData() return 0 Error!!\n");
       return 0;
    }
    simple_tlv_ptr += itemdata_length;
    
    // Parse items Next Action Indicator
    if (pSetupMenu->pNextActIndList.IsExist)
    {
        nextactionindicator_length = ParseItemsNextActionIndicator(simple_tlv_ptr, pSetupMenu->listSize, 
            pSetupMenu->pNextActIndList);
        simple_tlv_ptr += nextactionindicator_length;
    }
    
    // Parse Icon Identifier
    if (pSetupMenu->titleIcon.IsExist)
    {
        iconidentifier_length = ParseIconIdentifier(simple_tlv_ptr, pSetupMenu->titleIcon);
        simple_tlv_ptr += iconidentifier_length;
    }
    
    // Parse Item Icon Identifier List
    if (pSetupMenu->pIconList.IsExist)
    {
        iconidentifierlist_length = ParseIconIdentifierList(simple_tlv_ptr, pSetupMenu->listSize, pSetupMenu->pIconList);
        simple_tlv_ptr += iconidentifierlist_length;
    }
    
    // Decide the Length
    simple_tlv_length = command_detail_length + device_identities_length + alphaidentifier_length 
        + itemdata_length + nextactionindicator_length + iconidentifier_length + iconidentifierlist_length;
                 
    // Fill the TLV data
    tlv_length = ParseTlvData(tlv_data_string, simple_tlv_objects, simple_tlv_length);

    return tlv_length;
}


//******************************************************************************
//
// Function Name: ParseSATKSelectItem
//
// Description:   Parse the SATK Select Item(refer to 11.14 section 6.6.8)
//
// Notes:
//
//******************************************************************************
UInt16 ParseSATKSelectItem(char* tlv_data_string, SelectItem_t *pSelectItem)
{
    UInt8 simple_tlv_objects[MAX_SIMPLE_TLV_DATA_LENGTH];
    UInt16 simple_tlv_length = 0;
    UInt8 *simple_tlv_ptr = NULL;
    UInt16 tlv_length = 0;
    UInt16 command_detail_length = 0;
    UInt16 device_identities_length = 0;
    UInt16 alphaidentifier_length = 0;
    UInt16 itemdata_length = 0;
    UInt16 nextactionindicator_length = 0;
    UInt16 itemidentifier_length = 0;
    UInt16 iconidentifier_length = 0;
    UInt16 iconidentifierlist_length = 0;

    KRIL_DEBUG(DBG_INFO,"listSize:%d isAlphaIdProvided:%d isHelpAvailable:%d defaultItem:%d\n",
        pSelectItem->listSize,
        pSelectItem->isAlphaIdProvided,
        pSelectItem->isHelpAvailable,
        pSelectItem->defaultItem
        );
    
    KRIL_DEBUG(DBG_INFO,"title.unicode_type:0x%02X len:%d titleIcon.IsExist:%d Id:%d IsSelfExplanatory:%d\n",
        pSelectItem->title.unicode_type,
        pSelectItem->title.len,
        pSelectItem->titleIcon.IsExist,
        pSelectItem->titleIcon.Id,
        pSelectItem->titleIcon.IsSelfExplanatory
        );

    KRIL_DEBUG(DBG_INFO,"pIconList.IsExist:%d Id[0]:%d Id[1]:%d Id[2]:%d IsSelfExplanatory:%d\n",
        pSelectItem->pIconList.IsExist,
        pSelectItem->pIconList.Id[0],
        pSelectItem->pIconList.Id[1],
        pSelectItem->pIconList.Id[2],
        pSelectItem->pIconList.IsSelfExplanatory
        );
        
    simple_tlv_ptr = simple_tlv_objects;
    
    // Fill Command detail data
    command_detail_length = ParseCommandDetails(simple_tlv_ptr, (void*)pSelectItem, STK_SELECTITEM);
    if (!command_detail_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseCommandDetails() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += command_detail_length;

    // Fill Device identities
    device_identities_length = ParseDeviceIdentities(simple_tlv_ptr, STK_SELECTITEM);
    if (!device_identities_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseDeviceIdentities() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += device_identities_length;

    // Parse Alpha identifier data    
    if (pSelectItem->title.len > 0)
    {
        alphaidentifier_length = ParseAlphaIdentifier(simple_tlv_ptr, pSelectItem->title);
        if (!alphaidentifier_length)
        {
            KRIL_DEBUG(DBG_ERROR,"ParseAlphaIdentifier() return 0 Error!!\n");
            return 0;
        }
        
        simple_tlv_ptr += alphaidentifier_length;
    }

    // Parse item data object
    itemdata_length = ParseItemData(simple_tlv_ptr, pSelectItem->listSize, pSelectItem->pItemIdList, pSelectItem->pItemList);
    if (!itemdata_length)
    {
       KRIL_DEBUG(DBG_ERROR,"ParseItemData() return 0 Error!!\n");
       return 0;
    }
    simple_tlv_ptr += itemdata_length;
                    
    // Parse items Next Action Indicator
    if (pSelectItem->pNextActIndList.IsExist)
    {
        nextactionindicator_length = ParseItemsNextActionIndicator(simple_tlv_ptr, pSelectItem->listSize, 
            pSelectItem->pNextActIndList);
        simple_tlv_ptr += nextactionindicator_length;
    }
    
    // Parse Item Identifier
    if (pSelectItem->defaultItem)
    {
        itemidentifier_length = ParseItemIdentifier(simple_tlv_ptr, pSelectItem->defaultItem);
        simple_tlv_ptr += itemidentifier_length;
    }
    
    // Parse Icon Identifier
    if (pSelectItem->titleIcon.IsExist)
    {
        iconidentifier_length = ParseIconIdentifier(simple_tlv_ptr, pSelectItem->titleIcon);
        simple_tlv_ptr += iconidentifier_length;
    }
    
    // Parse Item Icon Identifier List
    if (pSelectItem->pIconList.IsExist)
    {
        iconidentifierlist_length = ParseIconIdentifierList(simple_tlv_ptr, pSelectItem->listSize, pSelectItem->pIconList);
        simple_tlv_ptr += iconidentifierlist_length;
    }
                        
    // Decide the Length
    simple_tlv_length = command_detail_length + device_identities_length + alphaidentifier_length 
        + itemdata_length + nextactionindicator_length + itemidentifier_length + iconidentifier_length
        + iconidentifierlist_length;
                 
    // Fill the TLV data
    tlv_length = ParseTlvData(tlv_data_string, simple_tlv_objects, simple_tlv_length);
    
    return tlv_length;
}


//******************************************************************************
//
// Function Name: ParseSATKGetInput
//
// Description:   Parse the SATK Get Input(refer to 11.14 section 6.6.3)
//
// Notes:
//
//******************************************************************************
UInt16 ParseSATKGetInput(char* tlv_data_string, GetInput_t *pGetInput)
{
    UInt8 simple_tlv_objects[MAX_SIMPLE_TLV_DATA_LENGTH];
    UInt16 simple_tlv_length = 0;
    UInt8 *simple_tlv_ptr = NULL;
    UInt16 tlv_length = 0;
    UInt16 command_detail_length = 0;
    UInt16 device_identities_length = 0;
    UInt16 text_string_length = 0;
    UInt16 defaultext_string_length = 0;
    UInt16 responselen_length = 0;
    UInt16 iconidentifier_length = 0;

    KRIL_DEBUG(DBG_INFO,"minLen:%d maxLen:%d inPutType:%d isHelpAvailable:%d isEcho:%d isPacked:%d stkStr.unicode_type:0x%02X len:%d\n",
        pGetInput->minLen,
        pGetInput->maxLen,
        pGetInput->inPutType,
        pGetInput->isHelpAvailable,
        pGetInput->isEcho,
        pGetInput->isPacked,
        pGetInput->stkStr.unicode_type,
        pGetInput->stkStr.len
        );
    DisplayStkTextString(pGetInput->stkStr);
    
    KRIL_DEBUG(DBG_INFO,"defaultSATKStr.unicode_type:0x%02X len:%d icon.IsExist:%d Id:%d IsSelfExplanatory:%d\n",
        pGetInput->defaultSATKStr.unicode_type,
        pGetInput->defaultSATKStr.len,
        pGetInput->icon.IsExist,
        pGetInput->icon.Id,
        pGetInput->icon.IsSelfExplanatory
        );
    DisplayStkTextString(pGetInput->defaultSATKStr);
    
    simple_tlv_ptr = simple_tlv_objects;
    
    // Fill Command detail data
    command_detail_length = ParseCommandDetails(simple_tlv_ptr, (void*)pGetInput, STK_GETINPUT);
    if (!command_detail_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseCommandDetails() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += command_detail_length;

    // Fill Device identities
    device_identities_length = ParseDeviceIdentities(simple_tlv_ptr, STK_GETINPUT);
    if (!device_identities_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseDeviceIdentities() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += device_identities_length;

    // Fill Text string
    text_string_length = ParseTextString(simple_tlv_ptr, pGetInput->stkStr);
    if (!text_string_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseTextString() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += text_string_length;
    
    // Fill Response length
    responselen_length = ParseResponseLength(simple_tlv_ptr, pGetInput->minLen, pGetInput->maxLen);
    if (!responselen_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseTextString() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += responselen_length;
    
    // Fill default text
    if (pGetInput->defaultSATKStr.len > 0)
    {
        defaultext_string_length = ParseDefaultTextString(simple_tlv_ptr, pGetInput->defaultSATKStr);
        simple_tlv_ptr += defaultext_string_length;
    }

    // Parse Icon Identifier
    if (pGetInput->icon.IsExist)
    {
        iconidentifier_length = ParseIconIdentifier(simple_tlv_ptr, pGetInput->icon);
        simple_tlv_ptr += iconidentifier_length;
    }
        
    // Decide the total simple TLV Length
    simple_tlv_length = command_detail_length + device_identities_length + text_string_length
        + defaultext_string_length + responselen_length + iconidentifier_length; 
                 
    // Fill the TLV data
    tlv_length = ParseTlvData(tlv_data_string, simple_tlv_objects, simple_tlv_length);
    
    return tlv_length;
}


//******************************************************************************
//
// Function Name: ParseSATKGetInkey
//
// Description:   Parse the SATK Get Inkey(refer to 11.14 section 6.6.2)
//
// Notes:
//
//******************************************************************************
int ParseSATKGetInkey(char* tlv_data_string, GetInkey_t *pGetInkey)
{
    UInt8 simple_tlv_objects[MAX_SIMPLE_TLV_DATA_LENGTH];
    UInt16 simple_tlv_length = 0;
    UInt8 *simple_tlv_ptr = NULL;
    UInt16 tlv_length = 0;
    UInt16 command_detail_length = 0;
    UInt16 device_identities_length = 0;
    UInt16 text_string_length = 0;
    UInt16 iconidentifier_length = 0;
    
    KRIL_DEBUG(DBG_INFO,"unicode_type:0x%02X len:%d icon.IsExist:%d Id:%d IsSelfExplanatory:%d inKeyType:%d isHelpAvailable:%d\n",
        pGetInkey->stkStr.unicode_type, 
        pGetInkey->stkStr.len,
        pGetInkey->icon.IsExist,
        pGetInkey->icon.Id,
        pGetInkey->icon.IsSelfExplanatory,
        pGetInkey->inKeyType, 
        pGetInkey->isHelpAvailable);
            
    DisplayStkTextString(pGetInkey->stkStr);    

    simple_tlv_ptr = simple_tlv_objects;
    
    // Fill Command detail data
    command_detail_length = ParseCommandDetails(simple_tlv_ptr, (void*)pGetInkey, STK_GETINKEY);
    if (!command_detail_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseCommandDetails() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += command_detail_length;

    // Fill Device identities
    device_identities_length = ParseDeviceIdentities(simple_tlv_ptr, STK_GETINKEY);
    if (!device_identities_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseDeviceIdentities() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += device_identities_length;

    // Fill Text string
    text_string_length = ParseTextString(simple_tlv_ptr, pGetInkey->stkStr);
    if (!text_string_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseTextString() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += text_string_length;

    // Parse Icon Identifier
    if (pGetInkey->icon.IsExist)
    {
        iconidentifier_length = ParseIconIdentifier(simple_tlv_ptr, pGetInkey->icon);
        simple_tlv_ptr += iconidentifier_length;
    }
    
    // Decide the total simple TLV Length
    simple_tlv_length = command_detail_length + device_identities_length + text_string_length 
        + iconidentifier_length;
                 
    // Fill the TLV data
    tlv_length = ParseTlvData(tlv_data_string, simple_tlv_objects, simple_tlv_length);
            
    return tlv_length;
}


//******************************************************************************
//
// Function Name: ParseSATKDisplayText
//
// Description:   Parse the SATK Display Text(refer to 11.14 section 6.6.1)
//
// Notes:
//
//******************************************************************************
int ParseSATKDisplayText(char* tlv_data_string, DisplayText_t *pDisplayText)
{
    UInt8 simple_tlv_objects[MAX_SIMPLE_TLV_DATA_LENGTH];
    UInt16 simple_tlv_length = 0;
    UInt8 *simple_tlv_ptr = NULL;
    UInt16 tlv_length = 0;
    UInt16 command_detail_length = 0;
    UInt16 device_identities_length = 0;
    UInt16 text_string_length = 0;
    UInt16 iconidentifier_length = 0;
    UInt16 immediate_response_length = 0;

    KRIL_DEBUG(DBG_INFO,"isHighPrio:%d isDelay:%d isSustained:%d stkStr.unicode_type:0x%02X len:%d\n",
        pDisplayText->isHighPrio,
        pDisplayText->isDelay,
        pDisplayText->isSustained,
        pDisplayText->stkStr.unicode_type,
        pDisplayText->stkStr.len
        );

    DisplayStkTextString(pDisplayText->stkStr);
    
    KRIL_DEBUG(DBG_INFO,"icon.IsExist:%d Id:%d IsSelfExplanatory:%d\n",
        pDisplayText->icon.IsExist,
        pDisplayText->icon.Id,
        pDisplayText->icon.IsSelfExplanatory
        );
        
    simple_tlv_ptr = simple_tlv_objects;
    
    // Fill Command detail data
    command_detail_length = ParseCommandDetails(simple_tlv_ptr, (void*)pDisplayText, STK_DISPLAYTEXT);
    if (!command_detail_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseCommandDetails() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += command_detail_length;

    // Fill Device identities
    device_identities_length = ParseDeviceIdentities(simple_tlv_ptr, STK_DISPLAYTEXT);
    if (!device_identities_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseDeviceIdentities() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += device_identities_length;

    // Fill Text string
    text_string_length = ParseTextString(simple_tlv_ptr, pDisplayText->stkStr);
    if (!text_string_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseTextString() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += text_string_length;

    // Parse Icon Identifier
    if (pDisplayText->icon.IsExist)
    {
        iconidentifier_length = ParseIconIdentifier(simple_tlv_ptr, pDisplayText->icon);
        simple_tlv_ptr += iconidentifier_length;
    }
        
    // Fill Immediate response
    if (pDisplayText->isSustained)
    {
        ParseImmediateResponse(simple_tlv_ptr);
        immediate_response_length = 2;
        simple_tlv_ptr += immediate_response_length;
    }
    
    // Decide the total simple TLV Length
    simple_tlv_length = command_detail_length + device_identities_length + text_string_length
        + iconidentifier_length + immediate_response_length;
                 
    // Fill the TLV data
    tlv_length = ParseTlvData(tlv_data_string, simple_tlv_objects, simple_tlv_length);
    
    return tlv_length;
}


//******************************************************************************
//
// Function Name: ParseSATKSendMOSMS
//
// Description:   Parse the SATK Send MO SMS(refer to 11.14 section 6.6.9)
//
// Notes:
//
//******************************************************************************
int ParseSATKSendMOSMS(char* tlv_data_string, SendMOSMS_t *pSendMOSMS)
{
    UInt8 simple_tlv_objects[MAX_SIMPLE_TLV_DATA_LENGTH];
    UInt16 simple_tlv_length = 0;
    UInt8 *simple_tlv_ptr = NULL;
    UInt16 tlv_length = 0;
    UInt16 command_detail_length = 0;
    UInt16 device_identities_length = 0;
    UInt16 alphaidentifier_length = 0;
    UInt16 iconidentifier_length = 0;
    
    KRIL_DEBUG(DBG_INFO,"isAlphaIdProvided:%d text.unicode_type:0x%02X len:%d icon.IsExist:%d Id:%d IsSelfExplanatory:%d\n",
        pSendMOSMS->isAlphaIdProvided,
        pSendMOSMS->text.unicode_type,
        pSendMOSMS->text.len,
        pSendMOSMS->icon.IsExist,
        pSendMOSMS->icon.Id,
        pSendMOSMS->icon.IsSelfExplanatory
        );
    
    KRIL_DEBUG(DBG_INFO,"sms_data.sca_ton_npi:%d sca_len:%d pdu_len:%d\n",
        pSendMOSMS->sms_data.sca_ton_npi,
        pSendMOSMS->sms_data.sca_len,
        pSendMOSMS->sms_data.pdu_len
        );

    DisplayStkTextString(pSendMOSMS->text);

    simple_tlv_ptr = simple_tlv_objects;
    
    // Fill Command detail data
    command_detail_length = ParseCommandDetails(simple_tlv_ptr, (void*)pSendMOSMS, STK_SENDSMS);
    if (!command_detail_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseCommandDetails() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += command_detail_length;

    // Fill Device identities
    device_identities_length = ParseDeviceIdentities(simple_tlv_ptr, STK_SENDSMS);
    if (!device_identities_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseDeviceIdentities() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += device_identities_length;

    // Parse Alpha identifier data
    if (pSendMOSMS->text.len > 0)
    {
        alphaidentifier_length = ParseAlphaIdentifier(simple_tlv_ptr, pSendMOSMS->text);
        if(!alphaidentifier_length)
        {
            KRIL_DEBUG(DBG_ERROR,"ParseAlphaIdentifier() return 0 Error!!\n");
            return 0;
        }
        
        simple_tlv_ptr += alphaidentifier_length;
    }    

    // Parse Icon Identifier
    if (pSendMOSMS->icon.IsExist)
    {
        iconidentifier_length = ParseIconIdentifier(simple_tlv_ptr, pSendMOSMS->icon);
        simple_tlv_ptr += iconidentifier_length;
    }
        
    // Decide the total simple TLV Length
    simple_tlv_length = command_detail_length + device_identities_length + alphaidentifier_length
        + iconidentifier_length;
    
    // Fill the TLV data
    tlv_length = ParseTlvData(tlv_data_string, simple_tlv_objects, simple_tlv_length);
    
    return tlv_length;
}


//******************************************************************************
//
// Function Name: ParseSATKSendSs
//
// Description:   Parse the SATK Send SS(refer to 11.14 section 6.6.10)
//
// Notes:
//
//******************************************************************************
int ParseSATKSendSs(char* tlv_data_string, SendSs_t *pSendSs)
{
    UInt8 simple_tlv_objects[MAX_SIMPLE_TLV_DATA_LENGTH];
    UInt16 simple_tlv_length = 0;
    UInt8 *simple_tlv_ptr = NULL;
    UInt16 tlv_length = 0;
    UInt16 command_detail_length = 0;
    UInt16 device_identities_length = 0;
    UInt16 alphaidentifier_length = 0;
    UInt16 iconidentifier_length = 0;
    
    KRIL_DEBUG(DBG_INFO,"ssType:%d num.Num:%s Ton:%d Npi:%d dcs:0x%X len:%d\n",
        pSendSs->ssType,
        pSendSs->num.Num, 
        pSendSs->num.Ton, 
        pSendSs->num.Npi, 
        pSendSs->num.dcs,
        pSendSs->num.len
        );
    
    KRIL_DEBUG(DBG_INFO,"isAlphaIdProvided:%d icon.IsExist:%d Id:%d IsSelfExplanatory:%d text.unicode_type:0x%02X len:%d\n",
        pSendSs->isAlphaIdProvided, 
        pSendSs->icon.IsExist,
        pSendSs->icon.Id,
        pSendSs->icon.IsSelfExplanatory, 
        pSendSs->text.unicode_type,
        pSendSs->text.len
        );

    DisplayStkTextString(pSendSs->text);    

    simple_tlv_ptr = simple_tlv_objects;
    
    // Fill Command detail data
    command_detail_length = ParseCommandDetails(simple_tlv_ptr, (void*)pSendSs, STK_SENDSS);
    if (!command_detail_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseCommandDetails() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += command_detail_length;

    // Fill Device identities
    device_identities_length = ParseDeviceIdentities(simple_tlv_ptr, STK_SENDSS);
    if (!device_identities_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseDeviceIdentities() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += device_identities_length;

    // Parse Alpha identifier data
    if (pSendSs->text.len > 0)
    {
        alphaidentifier_length = ParseAlphaIdentifier(simple_tlv_ptr, pSendSs->text);
        if(!alphaidentifier_length)
        {
            KRIL_DEBUG(DBG_ERROR,"ParseAlphaIdentifier() return 0 Error!!\n");
            return 0;
        }
        
        simple_tlv_ptr += alphaidentifier_length;
    }    

    // Parse Icon Identifier
    if (pSendSs->icon.IsExist)
    {
        iconidentifier_length = ParseIconIdentifier(simple_tlv_ptr, pSendSs->icon);
        simple_tlv_ptr += iconidentifier_length;
    }
        
    // Decide the total simple TLV Length
    simple_tlv_length = command_detail_length + device_identities_length + alphaidentifier_length
        + iconidentifier_length;
    
    // Fill the TLV data
    tlv_length = ParseTlvData(tlv_data_string, simple_tlv_objects, simple_tlv_length);
    
    return tlv_length;
}


//******************************************************************************
//
// Function Name: ParseSATKSendUssd
//
// Description:   Parse the SATK Send USSd(refer to 11.14 section 6.6.11)
//
// Notes:
//
//******************************************************************************
int ParseSATKSendUssd(char* tlv_data_string, SendUssd_t *pSendUssd)
{
    UInt8 simple_tlv_objects[MAX_SIMPLE_TLV_DATA_LENGTH];
    UInt16 simple_tlv_length = 0;
    UInt8 *simple_tlv_ptr = NULL;
    UInt16 tlv_length = 0;
    UInt16 command_detail_length = 0;
    UInt16 device_identities_length = 0;
    UInt16 alphaidentifier_length = 0;
    UInt16 iconidentifier_length = 0;
    
    KRIL_DEBUG(DBG_INFO,"ssType:%d num.Num:%s Ton:%d Npi:%d dcs:0x%X len:%d\n",
        pSendUssd->ssType, 
        pSendUssd->num.Num, 
        pSendUssd->num.Ton, 
        pSendUssd->num.Npi, 
        pSendUssd->num.dcs, 
        pSendUssd->num.len);
    
    KRIL_DEBUG(DBG_INFO,"isAlphaIdProvided:%d icon.IsExist:%d Id:%d IsSelfExplanatory:%d text.unicode_type:0x%02X len:%d\n",
        pSendUssd->isAlphaIdProvided, 
        pSendUssd->icon.IsExist,
        pSendUssd->icon.Id, 
        pSendUssd->icon.IsSelfExplanatory, 
        pSendUssd->text.unicode_type,
        pSendUssd->text.len
        );

    DisplayStkTextString(pSendUssd->text);

    simple_tlv_ptr = simple_tlv_objects;
    
    // Fill Command detail data
    command_detail_length = ParseCommandDetails(simple_tlv_ptr, (void*)pSendUssd, STK_SENDUSSD);
    if (!command_detail_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseCommandDetails() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += command_detail_length;

    // Fill Device identities
    device_identities_length = ParseDeviceIdentities(simple_tlv_ptr, STK_SENDUSSD);
    if (!device_identities_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseDeviceIdentities() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += device_identities_length;

    // Parse Alpha identifier data
    if (pSendUssd->text.len > 0)
    {
        alphaidentifier_length = ParseAlphaIdentifier(simple_tlv_ptr, pSendUssd->text);
        if(!alphaidentifier_length)
        {
            KRIL_DEBUG(DBG_ERROR,"ParseAlphaIdentifier() return 0 Error!!\n");
            return 0;
        }
        
        simple_tlv_ptr += alphaidentifier_length;
    }    

    // Parse Icon Identifier
    if (pSendUssd->icon.IsExist)
    {
        iconidentifier_length = ParseIconIdentifier(simple_tlv_ptr, pSendUssd->icon);
        simple_tlv_ptr += iconidentifier_length;
    }
        
    // Decide the total simple TLV Length
    simple_tlv_length = command_detail_length + device_identities_length + alphaidentifier_length
        + iconidentifier_length;
    
    // Fill the TLV data
    tlv_length = ParseTlvData(tlv_data_string, simple_tlv_objects, simple_tlv_length);
    
    return tlv_length;
}


//******************************************************************************
//
// Function Name: ParseSATKPlayTone
//
// Description:   Parse the SATK Play Tone(refer to 11.14 section 6.6.5)
//
// Notes:
//
//******************************************************************************
int ParseSATKPlayTone(char* tlv_data_string, PlayTone_t *pPlayTone)
{
    UInt8 simple_tlv_objects[MAX_SIMPLE_TLV_DATA_LENGTH];
    UInt16 simple_tlv_length = 0;
    UInt8 *simple_tlv_ptr = NULL;
    UInt16 tlv_length = 0;
    UInt16 command_detail_length = 0;
    UInt16 device_identities_length = 0;
    UInt16 alphaidentifier_length = 0;
    UInt16 tone_length = 0;
    UInt16 iconidentifier_length = 0;

    KRIL_DEBUG(DBG_INFO,"stkStr.unicode_type:0x%02X len:%d toneType:0x%02X duration:%lu icon.IsExist:%d Id:%d IsSelfExplanatory:%d\n",
        pPlayTone->stkStr.unicode_type,
        pPlayTone->stkStr.len,
        pPlayTone->toneType,
        pPlayTone->duration,
        pPlayTone->icon.IsExist,
        pPlayTone->icon.Id,
        pPlayTone->icon.IsSelfExplanatory
        );

    DisplayStkTextString(pPlayTone->stkStr);

    simple_tlv_ptr = simple_tlv_objects;
    
    // Fill Command detail data
    command_detail_length = ParseCommandDetails(simple_tlv_ptr, (void*)pPlayTone, STK_PLAYTONE);
    if (!command_detail_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseCommandDetails() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += command_detail_length;

    // Fill Device identities
    device_identities_length = ParseDeviceIdentities(simple_tlv_ptr, STK_PLAYTONE);
    if (!device_identities_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseDeviceIdentities() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += device_identities_length;

    // Parse Alpha identifier data
    if (pPlayTone->stkStr.len > 0)
    {
        alphaidentifier_length = ParseAlphaIdentifier(simple_tlv_ptr, pPlayTone->stkStr);
        if(!alphaidentifier_length)
        {
            KRIL_DEBUG(DBG_ERROR,"ParseAlphaIdentifier() return 0 Error!!\n");
            return 0;
        }
        
        simple_tlv_ptr += alphaidentifier_length;
    }
    
    // Parse Tone
    tone_length = ParseTone(simple_tlv_ptr, pPlayTone->toneType);
    if (!tone_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseTone() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += tone_length;

    // Parse Icon Identifier
    if (pPlayTone->icon.IsExist)
    {
        iconidentifier_length = ParseIconIdentifier(simple_tlv_ptr, pPlayTone->icon);
        simple_tlv_ptr += iconidentifier_length;
    }
        
    // Decide the total simple TLV Length
    simple_tlv_length = command_detail_length + device_identities_length + alphaidentifier_length
        + tone_length + iconidentifier_length;
    
    // Fill the TLV data
    tlv_length = ParseTlvData(tlv_data_string, simple_tlv_objects, simple_tlv_length);
    
    return tlv_length;
}


//******************************************************************************
//
// Function Name: ParseSATKSendStkDtmf
//
// Description:   Parse the SATK Send STK DTMF(refer to 11.14 section 6.6.24)
//
// Notes:
//
//******************************************************************************
int ParseSATKSendStkDtmf(char* tlv_data_string, SendStkDtmf_t *pSendStkDtmf)
{
    UInt8 simple_tlv_objects[MAX_SIMPLE_TLV_DATA_LENGTH];
    UInt16 simple_tlv_length = 0;
    UInt8 *simple_tlv_ptr = NULL;
    UInt16 tlv_length = 0;
    UInt16 command_detail_length = 0;
    UInt16 device_identities_length = 0;
    UInt16 alphaidentifier_length = 0;
    UInt16 iconidentifier_length = 0;
    
    KRIL_DEBUG(DBG_INFO,"isAlphaIdProvided:%d alphaString.unicode_type:0x%02X len:%d dtmfIcon.IsExist:%d Id:%d IsSelfExplanatory:%d dtmf:%s\n",
        pSendStkDtmf->isAlphaIdProvided,
        pSendStkDtmf->alphaString.unicode_type,
        pSendStkDtmf->alphaString.len,
        pSendStkDtmf->dtmfIcon.IsExist,
        pSendStkDtmf->dtmfIcon.Id,
        pSendStkDtmf->dtmfIcon.IsSelfExplanatory,
        (char*)pSendStkDtmf->dtmf
        );

    DisplayStkTextString(pSendStkDtmf->alphaString);

    simple_tlv_ptr = simple_tlv_objects;
    
    // Fill Command detail data
    command_detail_length = ParseCommandDetails(simple_tlv_ptr, (void*)pSendStkDtmf, STK_SENDDTMF);
    if (!command_detail_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseCommandDetails() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += command_detail_length;

    // Fill Device identities
    device_identities_length = ParseDeviceIdentities(simple_tlv_ptr, STK_SENDDTMF);
    if (!device_identities_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseDeviceIdentities() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += device_identities_length;

    // Parse Alpha identifier data
    if (pSendStkDtmf->alphaString.len > 0)
    {
        alphaidentifier_length = ParseAlphaIdentifier(simple_tlv_ptr, pSendStkDtmf->alphaString);
        if(!alphaidentifier_length)
        {
            KRIL_DEBUG(DBG_ERROR,"ParseAlphaIdentifier() return 0 Error!!\n");
            return 0;
        }
        
        simple_tlv_ptr += alphaidentifier_length;
    }

    // Parse Icon Identifier
    if (pSendStkDtmf->dtmfIcon.IsExist)
    {
        iconidentifier_length = ParseIconIdentifier(simple_tlv_ptr, pSendStkDtmf->dtmfIcon);
        simple_tlv_ptr += iconidentifier_length;
    }
        
    // Decide the total simple TLV Length
    simple_tlv_length = command_detail_length + device_identities_length + alphaidentifier_length
        + iconidentifier_length;
    
    // Fill the TLV data
    tlv_length = ParseTlvData(tlv_data_string, simple_tlv_objects, simple_tlv_length);
    
    return tlv_length;
}


//******************************************************************************
//
// Function Name: ParseSATKSetupCall
//
// Description:   Parse the SATK Setup Call(refer to 11.14 section 6.6.12)
//
// Notes:
//
//******************************************************************************
int ParseSATKSetupCall(char* tlv_data_string, SetupCall_t *pSetupCall)
{
    UInt8 simple_tlv_objects[MAX_SIMPLE_TLV_DATA_LENGTH];
    UInt16 simple_tlv_length = 0;
    UInt8 *simple_tlv_ptr = NULL;
    UInt16 tlv_length = 0;
    UInt16 command_detail_length = 0;
    UInt16 device_identities_length = 0;
    UInt16 confirmPhaseStr_length = 0;
    UInt16 setupPhaseStr_length = 0;
    UInt16 address_length = 0;
    UInt16 confirmPhaseIcon_length = 0;
    UInt16 setupPhaseIcon_length = 0;

    KRIL_DEBUG(DBG_INFO,"isEmerCall:%d callType:%d num.Ton:%d Npi:%d Num:%s dcs:%d len:%d duration:%ld IsConfirm:%d IsSetup:%d\n",
        pSetupCall->isEmerCall,
        pSetupCall->callType,
        pSetupCall->num.Ton,
        pSetupCall->num.Npi,
        pSetupCall->num.Num,
        pSetupCall->num.dcs,
        pSetupCall->num.len,
        pSetupCall->duration,
        pSetupCall->IsConfirmAlphaIdProvided,
        pSetupCall->IsSetupAlphaIdProvided
        );
    
    if (pSetupCall->IsConfirmAlphaIdProvided)
    {
        KRIL_DEBUG(DBG_INFO,"ConfirmAlphaIdProvided: confirmPhaseStr.unicode_type:0x%02X len:%d confirmPhaseIcon.IsExist:%d Id:%d IsSelfExplanatory:%d\n",
            pSetupCall->confirmPhaseStr.unicode_type,
            pSetupCall->confirmPhaseStr.len,
            pSetupCall->confirmPhaseIcon.IsExist,
            pSetupCall->confirmPhaseIcon.Id,
            pSetupCall->confirmPhaseIcon.IsSelfExplanatory
            );
    
        DisplayStkTextString(pSetupCall->confirmPhaseStr);
    }
    
    if (pSetupCall->IsSetupAlphaIdProvided)
    {
        KRIL_DEBUG(DBG_INFO,"SetupAlphaIdProvided: setupPhaseStr.unicode_type:0x%02X len:%d setupPhaseIcon.IsExist:%d Id:%d IsSelfExplanatory:%d\n",
            pSetupCall->setupPhaseStr.unicode_type,
            pSetupCall->setupPhaseStr.len,
            pSetupCall->setupPhaseIcon.IsExist,
            pSetupCall->setupPhaseIcon.Id,
            pSetupCall->setupPhaseIcon.IsSelfExplanatory
            );
    
        DisplayStkTextString(pSetupCall->setupPhaseStr);
    }

    simple_tlv_ptr = simple_tlv_objects;
    
    // Fill Command detail data
    command_detail_length = ParseCommandDetails(simple_tlv_ptr, (void*)pSetupCall, STK_SETUPCALL);
    if (!command_detail_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseCommandDetails() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += command_detail_length;

    // Fill Device identities
    device_identities_length = ParseDeviceIdentities(simple_tlv_ptr, STK_SETUPCALL);
    if (!device_identities_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseDeviceIdentities() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += device_identities_length;

    // Confirm Phase Alpha identifier data
    if (pSetupCall->confirmPhaseStr.len > 0)
    {
        confirmPhaseStr_length = ParseAlphaIdentifier(simple_tlv_ptr, pSetupCall->confirmPhaseStr);
        if(!confirmPhaseStr_length)
        {
            KRIL_DEBUG(DBG_ERROR,"ParseAlphaIdentifier() return 0 Error!!\n");
            return 0;
        }
        
        simple_tlv_ptr += confirmPhaseStr_length;
    }
    
    // Parse Address
    address_length = ParseAddress(simple_tlv_ptr, pSetupCall->num);
    if (!address_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseAddress() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += address_length;

    // Parse confirm Phase Icon Identifier
    if (pSetupCall->confirmPhaseIcon.IsExist)
    {
        confirmPhaseIcon_length = ParseIconIdentifier(simple_tlv_ptr, pSetupCall->confirmPhaseIcon);
        simple_tlv_ptr += confirmPhaseIcon_length;
    }

    // Setup Phase Alpha identifier data
    if (pSetupCall->setupPhaseStr.len > 0)
    {
        setupPhaseStr_length = ParseAlphaIdentifier(simple_tlv_ptr, pSetupCall->setupPhaseStr);
        if(!setupPhaseStr_length)
        {
            KRIL_DEBUG(DBG_ERROR,"ParseAlphaIdentifier() return 0 Error!!\n");
            return 0;
        }
        
        simple_tlv_ptr += setupPhaseStr_length;
    }
    
    // Parse setup Phase Icon Identifier
    if (pSetupCall->setupPhaseIcon.IsExist)
    {
        setupPhaseIcon_length = ParseIconIdentifier(simple_tlv_ptr, pSetupCall->setupPhaseIcon);
        simple_tlv_ptr += setupPhaseIcon_length;
    }
            
    // Decide the total simple TLV Length
    simple_tlv_length = command_detail_length + device_identities_length + confirmPhaseStr_length
       + address_length + confirmPhaseIcon_length + setupPhaseStr_length + setupPhaseIcon_length;
    
    // Fill the TLV data
    tlv_length = ParseTlvData(tlv_data_string, simple_tlv_objects, simple_tlv_length);
    
    return tlv_length;
}


//******************************************************************************
//
// Function Name: ParseSATKIdleModeText
//
// Description:   Parse the SATK Idle Mode Text(refer to 11.14 section 6.6.22)
//
// Notes:
//
//******************************************************************************
int ParseSATKIdleModeText(char* tlv_data_string, IdleModeText_t *pIdleModeText)
{
    UInt8 simple_tlv_objects[MAX_SIMPLE_TLV_DATA_LENGTH];
    UInt16 simple_tlv_length = 0;
    UInt8 *simple_tlv_ptr = NULL;
    UInt16 tlv_length = 0;
    UInt16 command_detail_length = 0;
    UInt16 device_identities_length = 0;
    UInt16 text_string_length = 0;
    UInt16 iconidentifier_length = 0;

    KRIL_DEBUG(DBG_INFO,"stkStr.len:%d unicode_type:0x%X icon.IsExist:%d Id:%d IsSelfExplanatory:%d\n",
        pIdleModeText->stkStr.len, 
        pIdleModeText->stkStr.unicode_type, 
        pIdleModeText->icon.IsExist,
        pIdleModeText->icon.Id,
        pIdleModeText->icon.IsSelfExplanatory
        );

    DisplayStkTextString(pIdleModeText->stkStr);

    simple_tlv_ptr = simple_tlv_objects;
    
    // Fill Command detail data
    command_detail_length = ParseCommandDetails(simple_tlv_ptr, (void*)pIdleModeText, STK_SETUPIDLEMODETEXT);
    if (!command_detail_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseCommandDetails() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += command_detail_length;

    // Fill Device identities
    device_identities_length = ParseDeviceIdentities(simple_tlv_ptr, STK_SETUPIDLEMODETEXT);
    if (!device_identities_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseDeviceIdentities() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += device_identities_length;

    // Fill Text string
    text_string_length = ParseTextString(simple_tlv_ptr, pIdleModeText->stkStr);
    if (!text_string_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseTextString() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += text_string_length;

    // Parse Icon Identifier
    if (pIdleModeText->icon.IsExist)
    {
        iconidentifier_length = ParseIconIdentifier(simple_tlv_ptr, pIdleModeText->icon);
        simple_tlv_ptr += iconidentifier_length;
    }
    
    // Decide the total simple TLV Length
    simple_tlv_length = command_detail_length + device_identities_length + text_string_length
        + iconidentifier_length;
                 
    // Fill the TLV data
    tlv_length = ParseTlvData(tlv_data_string, simple_tlv_objects, simple_tlv_length);
    
    return tlv_length;
}


//******************************************************************************
//
// Function Name: ParseSATKRefresh
//
// Description:   Parse the SATK Refresh(refer to 11.14 section 6.6.13)
//
// Notes:
//
//******************************************************************************
int ParseSATKRefresh(char* tlv_data_string, Refresh_t *pRefresh)
{
    UInt8 simple_tlv_objects[MAX_SIMPLE_TLV_DATA_LENGTH];
    UInt16 simple_tlv_length = 0;
    UInt8 *simple_tlv_ptr = NULL;
    UInt16 tlv_length = 0;
    UInt16 command_detail_length = 0;
    UInt16 device_identities_length = 0;
    UInt16 filelist_length = 0;

    KRIL_DEBUG(DBG_INFO,"refreshType:%d appliType:%d FileIdList.number_of_file:%d\n",
        pRefresh->refreshType,
        pRefresh->appliType,
        pRefresh->FileIdList.number_of_file
        );

    simple_tlv_ptr = simple_tlv_objects;
    
    // Fill Command detail data
    command_detail_length = ParseCommandDetails(simple_tlv_ptr, (void*)pRefresh, STK_REFRESH);
    if (!command_detail_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseCommandDetails() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += command_detail_length;

    // Fill Device identities
    device_identities_length = ParseDeviceIdentities(simple_tlv_ptr, STK_REFRESH);
    if (!device_identities_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseDeviceIdentities() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += device_identities_length;
    
    // Fill File list
    if (pRefresh->FileIdList.number_of_file > 0)
    {
        filelist_length = ParseFileList(simple_tlv_ptr, pRefresh->FileIdList);
        if (!filelist_length)
        {
            KRIL_DEBUG(DBG_ERROR,"ParseFileList() return 0 Error!!\n");
            return 0;
        }
        simple_tlv_ptr += filelist_length;
    }

    // Decide the total simple TLV Length
    simple_tlv_length = command_detail_length + device_identities_length + filelist_length;
                 
    // Fill the TLV data
    tlv_length = ParseTlvData(tlv_data_string, simple_tlv_objects, simple_tlv_length);
    
    return tlv_length;
}


//******************************************************************************
//
// Function Name: ParseSATKLaunchBrowser
//
// Description:   Parse the SATK Launch Browser(refer to 11.14 section 6.6.26)
//
// Notes:
//
//******************************************************************************
int ParseSATKLaunchBrowser(char* tlv_data_string, LaunchBrowserReq_t *pLaunchBrowserReq)
{
    UInt8 simple_tlv_objects[MAX_SIMPLE_TLV_DATA_LENGTH];
    UInt16 simple_tlv_length = 0;
    UInt8 *simple_tlv_ptr = NULL;
    UInt16 tlv_length = 0;
    UInt16 command_detail_length = 0;
    UInt16 device_identities_length = 0;
    UInt16 browser_id_length = 0;
    UInt16 url_length = 0;
    UInt16 bear_length = 0;
    UInt16 provision_length = 0;
    UInt16 text_string_length = 0;
    UInt16 alphaidentifier_length = 0;
    UInt16 iconidentifier_length = 0;

    KRIL_DEBUG(DBG_INFO,"browser_action:%d browser_id_exist:%d browser_id:%d url:\"%s\"\r\n",
        pLaunchBrowserReq->browser_action,
        pLaunchBrowserReq->browser_id_exist,
        pLaunchBrowserReq->browser_id,
        pLaunchBrowserReq->url
        );

    KRIL_DEBUG(DBG_INFO,"bearer_length:%d bearer[0]:0x%02X prov_length:%d text.len:%d unicode_type:0x%X\r\n",
        pLaunchBrowserReq->bearer_length,
        pLaunchBrowserReq->bearer[0],
        pLaunchBrowserReq->prov_length,
        pLaunchBrowserReq->text.len,
        pLaunchBrowserReq->text.unicode_type
        );

    KRIL_DEBUG(DBG_INFO,"alpha_id.len:%d unicode_type:0x%X icon_id.IsExist:%d Id:%d IsSelfExplanatory:%d\r\n",
        pLaunchBrowserReq->alpha_id.len,
        pLaunchBrowserReq->alpha_id.unicode_type,
        pLaunchBrowserReq->icon_id.IsExist,
        pLaunchBrowserReq->icon_id.Id,
        pLaunchBrowserReq->icon_id.IsSelfExplanatory
        );
                
    simple_tlv_ptr = simple_tlv_objects;    
    
    // Fill Command detail data
    command_detail_length = ParseCommandDetails(simple_tlv_ptr, (void*)pLaunchBrowserReq, STK_LAUNCHBROWSER);
    if (!command_detail_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseCommandDetails() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += command_detail_length;

    // Fill Device identities
    device_identities_length = ParseDeviceIdentities(simple_tlv_ptr, STK_LAUNCHBROWSER);
    if (!device_identities_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseDeviceIdentities() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += device_identities_length;
    
    // Fill Browser Identity
    if (pLaunchBrowserReq->browser_id_exist)
    {
        browser_id_length = ParserBrowserIdentity(simple_tlv_ptr, pLaunchBrowserReq->browser_id);
        if (!browser_id_length)
        {
            KRIL_DEBUG(DBG_ERROR,"ParserBrowserIdentity() return 0 Error!!\n");
            return 0;
        }
        simple_tlv_ptr += browser_id_length;
    }
    
    // Fill URL
    url_length = ParseURL(simple_tlv_ptr, pLaunchBrowserReq->url);
    if (!url_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseURL() return 0 Error!!\n");
        return 0;
    }
    simple_tlv_ptr += url_length;
    
    // Fill Bear
    if (pLaunchBrowserReq->bearer_length > 0)
    {
        bear_length = ParseBear(simple_tlv_ptr, pLaunchBrowserReq);
        if (!bear_length)
        {
            KRIL_DEBUG(DBG_ERROR,"ParseBear() return 0 Error!!\n");
            return 0;
        }
        simple_tlv_ptr += bear_length;
    }

    // Fill Provisioning File Reference
    if (pLaunchBrowserReq->prov_length > 0)
    {
        provision_length = ParserProvisioning(simple_tlv_ptr, pLaunchBrowserReq);
        if (!provision_length)
        {
            KRIL_DEBUG(DBG_ERROR,"ParserProvisioning() return 0 Error!!\n");
            return 0;
        }
        simple_tlv_ptr += provision_length;
    }
    
    // Fill Text string
    if (pLaunchBrowserReq->text.len > 0)
    {
        text_string_length = ParseTextString(simple_tlv_ptr, pLaunchBrowserReq->text);
        if (!text_string_length)
        {
            KRIL_DEBUG(DBG_ERROR,"ParseTextString() return 0 Error!!\n");
            return 0;
        }
        simple_tlv_ptr += text_string_length;
    }
    
    // Fill Alpha identifier
    if (pLaunchBrowserReq->alpha_id.len > 0)
    {
        alphaidentifier_length = ParseAlphaIdentifier(simple_tlv_ptr, pLaunchBrowserReq->alpha_id);
        if (!alphaidentifier_length)
        {
            KRIL_DEBUG(DBG_ERROR,"ParseAlphaIdentifier() return 0 Error!!\n");
            return 0;
        }
        simple_tlv_ptr += alphaidentifier_length;        
    }
    
    // Fill Icon identifier
    if (pLaunchBrowserReq->icon_id.IsExist)
    {
        iconidentifier_length = ParseIconIdentifier(simple_tlv_ptr, pLaunchBrowserReq->icon_id);
        if (!iconidentifier_length)
        {
            KRIL_DEBUG(DBG_ERROR,"ParseIconIdentifier() return 0 Error!!\n");
            return 0;
        }        
        simple_tlv_ptr += iconidentifier_length;
    }

    // Decide the total simple TLV Length
    simple_tlv_length = command_detail_length + device_identities_length + browser_id_length +
        + url_length + bear_length + provision_length + text_string_length + alphaidentifier_length +
        + iconidentifier_length;
                 
    // Fill the TLV data
    tlv_length = ParseTlvData(tlv_data_string, simple_tlv_objects, simple_tlv_length);
    
    return tlv_length;
}


//******************************************************************************
//
// Function Name: ProcessSATKSetupMenu
//
// Description:   Process the SATK Setup Menu
//
// Notes:
//
//******************************************************************************
void ProcessSATKSetupMenu(void *dataBuf)
{
    SATK_EventData_t *pSATKEventData = (SATK_EventData_t*)dataBuf;
    SetupMenu_t *pSetupMenu = pSATKEventData->u.setup_menu;
    char tlv_data_string[MAX_TLV_STRING_LENGTH];
    UInt16 tlv_length = 0;
    
    tlv_length = ParseSATKSetupMenu(tlv_data_string, pSetupMenu);
    if (!tlv_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseSATKSetupMenu() return 0 Error!!\n");
        return;    
    }
    
    KRIL_SendNotify(RIL_UNSOL_STK_PROACTIVE_COMMAND, tlv_data_string, (tlv_length * 2 + 1));
}


//******************************************************************************
//
// Function Name: ProcessSATKSelectItem
//
// Description:   Process the SATK Select Item
//
// Notes:
//
//******************************************************************************
void ProcessSATKSelectItem(void *dataBuf)
{
    SATK_EventData_t *pSATKEventData = (SATK_EventData_t*)dataBuf;
    SelectItem_t *pSelectItem = pSATKEventData->u.select_item;
    char tlv_data_string[MAX_TLV_STRING_LENGTH];
    UInt16 tlv_length = 0;
    
    tlv_length = ParseSATKSelectItem(tlv_data_string, pSelectItem);
    if (!tlv_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseSATKSelectItem() return 0 Error!!\n");
        return;    
    }
    
    KRIL_SendNotify(RIL_UNSOL_STK_PROACTIVE_COMMAND, tlv_data_string, (tlv_length * 2 + 1));
}


//******************************************************************************
//
// Function Name: ProcessSATKGetInput
//
// Description:   Process the SATK Get Input
//
// Notes:
//
//******************************************************************************
void ProcessSATKGetInput(void *dataBuf)
{
    SATK_EventData_t *pSATKEventData = (SATK_EventData_t*)dataBuf;
    GetInput_t *pGetInput = pSATKEventData->u.get_input;
    char tlv_data_string[MAX_TLV_STRING_LENGTH];
    UInt16 tlv_length = 0;
    
    tlv_length = ParseSATKGetInput(tlv_data_string, pGetInput);
    if (!tlv_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseSATKGetInput() return 0 Error!!\n");
        return;    
    }
    
    KRIL_SendNotify(RIL_UNSOL_STK_PROACTIVE_COMMAND, tlv_data_string, (tlv_length * 2 + 1));
}


//******************************************************************************
//
// Function Name: ProcessSATKGetInkey
//
// Description:   Process the SATK Get Inkey
//
// Notes:
//
//******************************************************************************
void ProcessSATKGetInkey(void *dataBuf)
{
    SATK_EventData_t *pSATKEventData = (SATK_EventData_t*)dataBuf;
    GetInkey_t *pGetInkey = pSATKEventData->u.get_inkey;
    char tlv_data_string[MAX_TLV_STRING_LENGTH];
    UInt16 tlv_length = 0;
    
    tlv_length = ParseSATKGetInkey(tlv_data_string, pGetInkey);
    if (!tlv_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseSATKGetInkey() return 0 Error!!\n");
        return;    
    }
    
    KRIL_SendNotify(RIL_UNSOL_STK_PROACTIVE_COMMAND, tlv_data_string, (tlv_length * 2 + 1));    
}


//******************************************************************************
//
// Function Name: ProcessSATKDisplayText
//
// Description:   Process the SATK Display Text
//
// Notes:
//
//******************************************************************************
void ProcessSATKDisplayText(void *dataBuf)
{
    SATK_EventData_t *pSATKEventData = (SATK_EventData_t*)dataBuf;
    DisplayText_t *pDisplayText = pSATKEventData->u.display_text;
    char tlv_data_string[MAX_TLV_STRING_LENGTH];
    UInt16 tlv_length = 0;
    
    tlv_length = ParseSATKDisplayText(tlv_data_string, pDisplayText);
    if (!tlv_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseSATKDisplayText() return 0 Error!!\n");
        return;    
    }
    
    KRIL_SendNotify(RIL_UNSOL_STK_PROACTIVE_COMMAND, tlv_data_string, (tlv_length * 2 + 1));
}


//******************************************************************************
//
// Function Name: ProcessSATKSendMOSMS
//
// Description:   Process the SATK Send MO SMS
//
// Notes:
//
//******************************************************************************
void ProcessSATKSendMOSMS(void *dataBuf)
{
    SATK_EventData_t *pSATKEventData = (SATK_EventData_t*)dataBuf;
    SendMOSMS_t *pSendMOSMS = pSATKEventData->u.send_short_msg;
    char tlv_data_string[MAX_TLV_STRING_LENGTH];
    UInt16 tlv_length = 0;
    
    tlv_length = ParseSATKSendMOSMS(tlv_data_string, pSendMOSMS);
    if (!tlv_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseSATKSendMOSMS() return 0 Error!!\n");
        return;    
    }
    
    KRIL_SendNotify(RIL_UNSOL_STK_EVENT_NOTIFY, tlv_data_string, (tlv_length * 2 + 1));    
}


//******************************************************************************
//
// Function Name: ProcessSATKSendSs
//
// Description:   Process the SATK Send SS
//
// Notes:
//
//******************************************************************************
void ProcessSATKSendSs(void *dataBuf)
{
    SATK_EventData_t *pSATKEventData = (SATK_EventData_t*)dataBuf;
    SendSs_t *pSendSs = pSATKEventData->u.send_ss;
    char tlv_data_string[MAX_TLV_STRING_LENGTH];
    UInt16 tlv_length = 0;
    
    tlv_length = ParseSATKSendSs(tlv_data_string, pSendSs);
    if (!tlv_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseSATKSendSs() return 0 Error!!\n");
        return;    
    }
    
    KRIL_SendNotify(RIL_UNSOL_STK_EVENT_NOTIFY, tlv_data_string, (tlv_length * 2 + 1));
}


//******************************************************************************
//
// Function Name: ProcessSATKSendUssd
//
// Description:   Process the SATK Send USSD
//
// Notes:
//
//******************************************************************************
void ProcessSATKSendUssd(void *dataBuf)
{
    SATK_EventData_t *pSATKEventData = (SATK_EventData_t*)dataBuf;
    SendUssd_t *pSendUssd = pSATKEventData->u.send_ussd;
    char tlv_data_string[MAX_TLV_STRING_LENGTH];
    UInt16 tlv_length = 0;
    
    tlv_length = ParseSATKSendUssd(tlv_data_string, pSendUssd);
    if (!tlv_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseSATKSendUssd() return 0 Error!!\n");
        return;    
    }
    
    KRIL_SendNotify(RIL_UNSOL_STK_EVENT_NOTIFY, tlv_data_string, (tlv_length * 2 + 1));
}


//******************************************************************************
//
// Function Name: ProcessSATKPlayTone
//
// Description:   Process the SATK Play Tone
//
// Notes:
//
//******************************************************************************
void ProcessSATKPlayTone(void *dataBuf)
{
    SATK_EventData_t *pSATKEventData = (SATK_EventData_t*)dataBuf;
    PlayTone_t *pPlayTone = pSATKEventData->u.play_tone;
    char tlv_data_string[MAX_TLV_STRING_LENGTH];
    UInt16 tlv_length = 0;
    
    tlv_length = ParseSATKPlayTone(tlv_data_string, pPlayTone);
    if (!tlv_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseSATKPlayTone() return 0 Error!!\n");
        return;    
    }
    
    KRIL_SendNotify(RIL_UNSOL_STK_PROACTIVE_COMMAND, tlv_data_string, (tlv_length * 2 + 1));
}


//******************************************************************************
//
// Function Name: ProcessSATKSendStkDtmf
//
// Description:   Process the SATK Send STK DTMF
//
// Notes:
//
//******************************************************************************
void ProcessSATKSendStkDtmf(void *dataBuf)
{
    SATK_EventData_t *pSATKEventData = (SATK_EventData_t*)dataBuf;
    SendStkDtmf_t *pSendStkDtmf = pSATKEventData->u.send_dtmf;
    char tlv_data_string[MAX_TLV_STRING_LENGTH];
    UInt16 tlv_length = 0;
    
    tlv_length = ParseSATKSendStkDtmf(tlv_data_string, pSendStkDtmf);
    if (!tlv_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseSATKPlayTone() return 0 Error!!\n");
        return;    
    }
    
    KRIL_SendNotify(RIL_UNSOL_STK_EVENT_NOTIFY, tlv_data_string, (tlv_length * 2 + 1));
}


//******************************************************************************
//
// Function Name: ProcessSATKSetupCall
//
// Description:   Process the SATK Setup Call
//
// Notes:
//
//******************************************************************************
void ProcessSATKSetupCall(void *dataBuf)
{
    SATK_EventData_t *pSATKEventData = (SATK_EventData_t*)dataBuf;
    SetupCall_t *pSetupCall = pSATKEventData->u.setup_call;
    char tlv_data_string[MAX_TLV_STRING_LENGTH];
    UInt16 tlv_length = 0;
    int timeout[1] = {0};
    
    tlv_length = ParseSATKSetupCall(tlv_data_string, pSetupCall);
    if (!tlv_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseSATKSetupCall() return 0 Error!!\n");
        return;    
    }
    
    //KRIL_SendNotify(RIL_UNSOL_STK_CALL_SETUP, timeout, sizeof(int));
    KRIL_SendNotify(RIL_UNSOL_STK_EVENT_NOTIFY, tlv_data_string, (tlv_length * 2 + 1));
    
    KRIL_SendNotify(RIL_UNSOL_STK_CALL_SETUP, timeout, sizeof(int));
}


//******************************************************************************
//
// Function Name: ProcessSATKIdleModeText
//
// Description:   Process the SATK Idle Mode Text
//
// Notes:
//
//******************************************************************************
void ProcessSATKIdleModeText(void *dataBuf)
{
    SATK_EventData_t *pSATKEventData = (SATK_EventData_t*)dataBuf;
    IdleModeText_t *pIdleModeText = pSATKEventData->u.idlemode_text;
    char tlv_data_string[MAX_TLV_STRING_LENGTH];
    UInt16 tlv_length = 0;
    
    tlv_length = ParseSATKIdleModeText(tlv_data_string, pIdleModeText);
    if (!tlv_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseSATKIdleModeText() return 0 Error!!\n");
        return;    
    }
    
    KRIL_SendNotify(RIL_UNSOL_STK_PROACTIVE_COMMAND, tlv_data_string, (tlv_length * 2 + 1));
}


//******************************************************************************
//
// Function Name: ProcessSATKRefresh
//
// Description:   Process the SATK Refresh
//
// Notes:
//
//******************************************************************************
void ProcessSATKRefresh(void *dataBuf)
{
    SATK_EventData_t *pSATKEventData = (SATK_EventData_t*)dataBuf;
    Refresh_t *pRefresh = pSATKEventData->u.refresh;
    UInt8 path_len;
    int data[2];

    KRIL_DEBUG(DBG_INFO,"refreshType:%d appliType:%d FileIdList.number_of_file:%d\n",
        pRefresh->refreshType,
        pRefresh->appliType,
        pRefresh->FileIdList.number_of_file
        );
            
    switch (pRefresh->refreshType)
    {
        case SMRT_INIT_FULLFILE_CHANGED:
        case SMRT_INIT_FILE_CHANGED:
        case SMRT_INIT:
            data[0] = SIM_INIT;
            data[1] = 0;
            break;
        
        case SMRT_FILE_CHANGED:
            data[0] = SIM_FILE_UPDATE;
            path_len = pRefresh->FileIdList.changed_file[0].path_len;
            data[1] = (int)pRefresh->FileIdList.changed_file[0].file_path[path_len-1];
            KRIL_DEBUG(DBG_INFO,"SMRT_FILE_CHANGED: data[1]:0x%X\n",data[1]);
            break;
        
        case SMRT_RESET:
             data[0] = SIM_INIT;
             data[1] = 0;
             break;
        
        default:
            KRIL_DEBUG(DBG_ERROR,"Unknow refreshType:%d Error!!\n",pRefresh->refreshType);
            return;
    }
    
    KRIL_SendNotify(RIL_UNSOL_SIM_REFRESH, data, sizeof(int)*2);
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
    {
    	ClientInfo_t clientInfo;
	    CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
        CAPI2_SatkApi_CmdResp(&clientInfo, SATK_EVENT_REFRESH, SATK_Result_CmdSuccess, 0, NULL, 0);
    }
#else
    CAPI2_SATKCmdResp(GetNewTID(), GetClientID(), SATK_EVENT_REFRESH, SATK_Result_CmdSuccess, 0, NULL, 0);    
#endif
}


//******************************************************************************
//
// Function Name: ProcessSATKLaunchBrowser
//
// Description:   Process the SATK Launch Browser
//
// Notes:
//
//******************************************************************************
void ProcessSATKLaunchBrowser(void *dataBuf)
{
    SATK_EventData_t *pSATKEventData = (SATK_EventData_t*)dataBuf;
    LaunchBrowserReq_t *pLaunchBrowserReq = pSATKEventData->u.launch_browser;
    char tlv_data_string[MAX_TLV_STRING_LENGTH];
    UInt16 tlv_length = 0;
    
    tlv_length = ParseSATKLaunchBrowser(tlv_data_string, pLaunchBrowserReq);
    if (!tlv_length)
    {
        KRIL_DEBUG(DBG_ERROR,"ParseSATKIdleModeText() return 0 Error!!\n");
        return;    
    }
    
    KRIL_SendNotify(RIL_UNSOL_STK_PROACTIVE_COMMAND, tlv_data_string, (tlv_length * 2 + 1));
}


//******************************************************************************
//
// Function Name: ProcessUSSDDataInd
//
// Description:   Process the USSD Data
//
// Notes:
//
//******************************************************************************
Boolean ProcessUSSDDataInd(void* data)
{
    USSDataInfo_t *rsp = (USSDataInfo_t *) data;
    KrilReceiveUSSDInfo_t rdata;
    Boolean isSendData = TRUE;
    int i;
 
    KRIL_DEBUG(DBG_INFO, "call_index:%d service_type:%d oldindex:%d newindex:%d prob_tag:%d prob_code:%d err_code:%d code_type:0x%x used_size:%d\n", rsp->call_index,rsp->ussd_data.service_type,rsp->ussd_data.oldindex,rsp->ussd_data.newindex,rsp->ussd_data.prob_tag,
        rsp->ussd_data.prob_code,rsp->ussd_data.err_code,rsp->ussd_data.code_type,rsp->ussd_data.used_size);
    memset(rdata.USSDString, 0, PHASE1_MAX_USSD_STRING_SIZE+1);
 
    if (USSD_REQUEST == rsp->ussd_data.service_type || 
        USSD_REGISTRATION == rsp->ussd_data.service_type)
    {
        rdata.type = 1;
    }
    else if (USSD_NOTIFICATION == rsp->ussd_data.service_type)
    {
        rdata.type = 0;
    }
    else if (USSD_RELEASE_COMPLETE_RETURN_RESULT == rsp->ussd_data.service_type)
    {
        rdata.type = 2;
        if (rsp->ussd_data.used_size < 0)
        {
            isSendData = FALSE;
        }
    }
    else if (USSD_FACILITY_RETURN_RESULT == rsp->ussd_data.service_type)
    {
        if (rsp->ussd_data.used_size > 0)
        {
         rdata.type = 1;
        }
        if (rsp->ussd_data.used_size < 0)
        {
            rdata.type = 0;
            isSendData = FALSE;
        }
    }
    else if (USSD_FACILITY_REJECT == rsp->ussd_data.service_type ||
             USSD_RELEASE_COMPLETE_REJECT == rsp->ussd_data.service_type)
    {
        rdata.type = 4;
        isSendData = FALSE;
    }
    else if (USSD_FACILITY_RETURN_ERROR == rsp->ussd_data.service_type ||
                USSD_RELEASE_COMPLETE_RETURN_ERROR == rsp->ussd_data.service_type)
    {
        rdata.type = 5;
    }
    else if (USSD_RESEND == rsp->ussd_data.service_type)
    {
        KRIL_DEBUG(DBG_ERROR, "USSD_RESEND\n");
        return FALSE;
    }
    rdata.Length = rsp->ussd_data.used_size;
    rdata.codetype = rsp->ussd_data.code_type;
 
    for (i = 0 ; i < rdata.Length ; i++)
        KRIL_DEBUG(DBG_TRACE2, "string:0x%x\n", rsp->ussd_data.string[i]);
 
    if(TRUE == isSendData)
    {
        memcpy(rdata.USSDString, rsp->ussd_data.string, rdata.Length);
    }
    KRIL_SendNotify(RIL_UNSOL_ON_USSD, &rdata, sizeof(KrilReceiveUSSDInfo_t));

    return TRUE;
}


//******************************************************************************
//
// Function Name: ProcessUSSDCallIndexInd
//
// Description:   Process the USSD call index
//
// Notes:
//
//******************************************************************************
Boolean ProcessUSSDCallIndexInd(void* data)
{
    StkReportCallStatus_t *call_status = (StkReportCallStatus_t *) data;
    KRIL_DEBUG(DBG_INFO, "MSG_USSD_CALLINDEX_IND index:%d call_type:%d status:%d\n", call_status->index,call_status->call_type,call_status->status);
    if((call_status->call_type == CALLTYPE_MTUSSDSUPPSVC) || (call_status->call_type == CALLTYPE_MOUSSDSUPPSVC))
    {
        if((call_status->status == CALLSTATUS_MT_CI_ALLOC) && (gUssdID != CALLINDEX_INVALID))
        {
            // if there's on-going ussd session, still allow MT USSD event to come in
            gPreviousUssdID = gUssdID;
        }
    }
    gUssdID = call_status->index;
    return TRUE;
}


//******************************************************************************
//
// Function Name: ProcessUSSDSessionEndInd
//
// Description:   Process the USSD session end
//
// Notes:
//
//******************************************************************************
Boolean ProcessUSSDSessionEndInd(void* data)
{
    CallIndex_t *rsp = (CallIndex_t *) data;
    KRIL_DEBUG(DBG_INFO, "MSG_USSD_SESSION_END_IND index:%d\n", *rsp);

    if(gUssdID == *rsp)
    {
        gUssdID = CALLINDEX_INVALID;
    }
    else
    {
        KRIL_DEBUG(DBG_INFO, "MSG_USSD_SESSION_END_IND gUssdID:%d\n", gUssdID);
    }

    if(gPreviousUssdID != CALLINDEX_INVALID)
    {
        gUssdID = gPreviousUssdID;
        gPreviousUssdID = CALLINDEX_INVALID;
     }
    KRIL_DEBUG(DBG_INFO, "MSG_USSD_SESSION_END_IND gUssdID:%d gPreviousUssdID:%d\n", gUssdID, gPreviousUssdID);
    return TRUE;
}


//******************************************************************************
//
// Function Name: ProcessTimeZoneInd
//
// Description:   Process the Time Zone
//
// Notes:
//
//******************************************************************************
Boolean ProcessTimeZoneInd(void* data)
{
    TimeZoneDate_t *rsp = (TimeZoneDate_t*) data;
    KrilTimeZoneDate_t rdate;
    rdate.timeZone = rsp->timeZone;
    rdate.dstAdjust = rsp->dstAdjust;
    rdate.Sec = rsp->adjustedTime.Sec;
    rdate.Min = rsp->adjustedTime.Min;
    rdate.Hour = rsp->adjustedTime.Hour;
    rdate.Week = rsp->adjustedTime.Week;
    rdate.Day = rsp->adjustedTime.Day;
    rdate.Month = rsp->adjustedTime.Month;
    rdate.Year = rsp->adjustedTime.Year;
    
    KRIL_DEBUG(DBG_INFO, "MSG_DATE_TIMEZONE_IND::timeZone:%d dstAdjust:%d Sec:%d Min:%d Hour:%d Week:%d Day:%d Month:%d Year:%d\n", rdate.timeZone, rdate.dstAdjust, rdate.Sec, rdate.Min, rdate.Hour, rdate.Week, rdate.Day, rdate.Month, rdate.Year);
    KRIL_SendNotify(RIL_UNSOL_NITZ_TIME_RECEIVED, &rdate, sizeof(KrilTimeZoneDate_t));
    return TRUE;
}


//******************************************************************************
//
// Function Name: ProcessRestrictedState
//
// Description:   Process Restricted Status
//
// Notes:
//
//******************************************************************************
void ProcessRestrictedState(void)
{
    Boolean update = FALSE;

    if ((REG_STATE_NORMAL_SERVICE == sCgreg_state || REG_STATE_ROAMING_SERVICE == sCgreg_state) &&
        (REG_STATE_NORMAL_SERVICE == sCreg_state || REG_STATE_ROAMING_SERVICE == sCreg_state)) //CS and PS allow
    {
        if (RIL_RESTRICTED_STATE_NONE != KRIL_GetRestrictedState())
        {
            KRIL_SetRestrictedState(RIL_RESTRICTED_STATE_NONE);
            update = TRUE;
        }
    }
    else if ((REG_STATE_NORMAL_SERVICE != sCgreg_state || REG_STATE_ROAMING_SERVICE != sCgreg_state) &&
        (REG_STATE_NORMAL_SERVICE == sCreg_state || REG_STATE_ROAMING_SERVICE == sCreg_state))  //CS allow and PS restricted
    {
        if (RIL_RESTRICTED_STATE_PS_ALL != KRIL_GetRestrictedState())
        {
            KRIL_SetRestrictedState(RIL_RESTRICTED_STATE_PS_ALL);
            update = TRUE;
        }
    }
    else if ((REG_STATE_NORMAL_SERVICE == sCgreg_state || REG_STATE_ROAMING_SERVICE == sCgreg_state) &&
        (REG_STATE_NORMAL_SERVICE != sCreg_state || REG_STATE_ROAMING_SERVICE != sCreg_state))  //CS restricted and PS allow
    {
        if(REG_STATE_LIMITED_SERVICE == sCreg_state || REG_STATE_NO_SERVICE == sCreg_state)
        {
            if (RIL_RESTRICTED_STATE_CS_NORMAL != KRIL_GetRestrictedState())
            {
                KRIL_SetRestrictedState(RIL_RESTRICTED_STATE_CS_NORMAL);
                update = TRUE;
            }
        }
        else
        {
            if (RIL_RESTRICTED_STATE_CS_ALL != KRIL_GetRestrictedState())
            {
                KRIL_SetRestrictedState(RIL_RESTRICTED_STATE_CS_ALL);
                update = TRUE;
            }
        }
    }
    else // CS and PS no service
    {
        if (sCreg_state == REG_STATE_LIMITED_SERVICE)
        {
            if (RIL_RESTRICTED_STATE_CS_NORMAL != KRIL_GetRestrictedState()) //CS emergency and PS restricted
            {
                KRIL_SetRestrictedState(RIL_RESTRICTED_STATE_CS_NORMAL | RIL_RESTRICTED_STATE_PS_ALL);
                update = TRUE;
            }
        }
        else if (REG_STATE_SEARCHING == sCreg_state && REG_STATE_SEARCHING == sCgreg_state) // CS and PS restricted
        {
            if ((RIL_RESTRICTED_STATE_CS_ALL | RIL_RESTRICTED_STATE_PS_ALL) != KRIL_GetRestrictedState())
            {
                KRIL_SetRestrictedState(RIL_RESTRICTED_STATE_CS_ALL | RIL_RESTRICTED_STATE_PS_ALL);
                update = TRUE;
            }
        }
    }
    KRIL_DEBUG(DBG_INFO, "MSG_REG_GPRS_IND::update:%d RestrictedState:%d\n", update, KRIL_GetRestrictedState());
    if (TRUE == update)
    {
        int state = KRIL_GetRestrictedState();
        KRIL_SendNotify(RIL_UNSOL_RESTRICTED_STATE_CHANGED, &state, sizeof(int));
        update = FALSE;
    }
}

//******************************************************************************
//
// Function Name: ProcessGSMStatus
//
// Description:   Process GSM Status
//
// Notes:
//
//******************************************************************************
void ProcessGSMStatus(void* data)
{
    MSRegInfo_t *pMSRegInfo = (MSRegInfo_t*)data;
    KRIL_DEBUG(DBG_INFO, "MSG_REG_GSM_IND::GetGSMRegStatus:%d regState:%d rat:%d gprs_supported:%d egprs_supported:%d\n", sCreg_state, pMSRegInfo->regState, pMSRegInfo->netInfo.rat, pMSRegInfo->netInfo.gprs_supported, pMSRegInfo->netInfo.egprs_supported);
    KRIL_DEBUG(DBG_INFO, "MSG_REG_GSM_IND::hsdpa_supported:%d hsupa_supported:%d\n", pMSRegInfo->netInfo.hsdpa_supported, pMSRegInfo->netInfo.hsupa_supported);
    if (pMSRegInfo->regState != REG_STATE_NO_CHANGE)
    {
        if (sCreg_state != pMSRegInfo->regState)
        {
            sCreg_state = pMSRegInfo->regState;
            KRIL_SendNotify(RIL_UNSOL_RESPONSE_NETWORK_STATE_CHANGED, NULL, 0);
        }
        else
        {
            if (1 == KRIL_GetLocationUpdateStatus() &&
               (gRegInfo.netInfo.rat != pMSRegInfo->netInfo.rat || gRegInfo.lac !=  pMSRegInfo->lac || gRegInfo.cell_id != pMSRegInfo->cell_id)) //+Creg=2 ; <state>, <lac>, <ci>, <AcT>
            {
                KRIL_SendNotify(RIL_UNSOL_RESPONSE_NETWORK_STATE_CHANGED, NULL, 0);
            }
        }
        memcpy(&gRegInfo, pMSRegInfo, sizeof(MSRegInfo_t));
    }
}


//******************************************************************************
//
// Function Name: ProcessGPRSStatus
//
// Description:   Process GPRS Status
//
// Notes:
//
//******************************************************************************
void ProcessGPRSStatus(void* data)
{
    MSRegInfo_t *pMSRegInfo = (MSRegInfo_t*)data;
    KRIL_DEBUG(DBG_INFO, "MSG_REG_GPRS_IND::GetGPRSRegStatus:%d regState:%d rat:%d gprs_supported:%d egprs_supported:%d\n", sCgreg_state, pMSRegInfo->regState, pMSRegInfo->netInfo.rat, pMSRegInfo->netInfo.gprs_supported, pMSRegInfo->netInfo.egprs_supported);
    if (pMSRegInfo->regState != REG_STATE_NO_CHANGE)
    {
        if(sCgreg_state != pMSRegInfo->regState)
        {
            sCgreg_state = pMSRegInfo->regState;
            KRIL_SendNotify(RIL_UNSOL_RESPONSE_NETWORK_STATE_CHANGED, NULL, 0);
        }
#if 0 // remove the function to avoid the GPRS connection does not establish.
        ProcessRestrictedState();
#endif
    }
}


//******************************************************************************
//
// Function Name: ProcessUE3GStatus
//
// Description:   Process HSDPA Support Status
//
// Notes:
//
//******************************************************************************
void ProcessUE3GStatus(void* data)
{
    MSUe3gStatusInd_t *pUE3Ginfo = (MSUe3gStatusInd_t *) data;
    KRIL_DEBUG(DBG_INFO, "MSG_UE_3G_STATUS_IND::mask:%ld in_cell_dch_state:%d hsdpa_ch_allocated:%d\n", pUE3Ginfo->in_uas_conn_info.mask, pUE3Ginfo->in_uas_conn_info.in_cell_dch_state, pUE3Ginfo->in_uas_conn_info.hsdpa_ch_allocated);
    if (pUE3Ginfo->in_uas_conn_info.hsdpa_ch_allocated != gUE3GInfo.in_uas_conn_info.hsdpa_ch_allocated)
    {
        KRIL_SendNotify(RIL_UNSOL_RESPONSE_NETWORK_STATE_CHANGED, NULL, 0);
    }
    memcpy(&gUE3GInfo, pUE3Ginfo, sizeof(MSUe3gStatusInd_t));
}


//******************************************************************************
//
// Function Name: ProcessRSSIInd
//
// Description:   Process RSSI information
//
// Notes:
//
//******************************************************************************
void ProcessRSSIInfo(void* data)
{
    RxSignalInfo_t *pSignalInfo = (RxSignalInfo_t*) data;
    if (REG_STATE_NORMAL_SERVICE == sCreg_state || REG_STATE_ROAMING_SERVICE == sCreg_state)
    {
        KrilSignalStrength_t signal_strnegth;
        signal_strnegth.RAT = gRegInfo.netInfo.rat;
        signal_strnegth.RxLev = pSignalInfo->rssi;
        signal_strnegth.RxQual	= pSignalInfo->qual;
        KRIL_DEBUG(DBG_INFO, "MSG_RSSI_IND::RAT:%d RxLev:%d RxQual:%d\n", signal_strnegth.RAT, signal_strnegth.RxLev, signal_strnegth.RxQual);
        KRIL_SendNotify(RIL_UNSOL_SIGNAL_STRENGTH, &signal_strnegth, sizeof(KrilSignalStrength_t));
    }
}


//******************************************************************************
//
// Function Name: ProcessSMSIndexInSIM
//
// Description:   Process The Index of Write SMS In SIM
//
// Notes:
//
//******************************************************************************
void  ProcessSMSIndexInSIM(void* data)
{
    SmsIncMsgStoredResult_t *pSmsIndex = (SmsIncMsgStoredResult_t *) data;
    KRIL_DEBUG(DBG_INFO, "ProcessSMSIndexInSIM::result:%d rec_no:%d waitState:%d\n", pSmsIndex->result, pSmsIndex->rec_no, pSmsIndex->waitState);

    if(SIMACCESS_SUCCESS == pSmsIndex->result)
    {
        KrilMsgIndexInfo_t msg;
        msg.index = pSmsIndex->rec_no;
        msg.index++;
        KRIL_SendNotify(RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM, &msg, sizeof(KrilMsgIndexInfo_t));
    }
}


//******************************************************************************
//
// Function Name: ProcessNewSMSMessage
//
// Description:   Process The New SMS Message
//
// Notes:
//
//******************************************************************************
void  ProcessNewSMSMessage(void * data)
{
    SmsSimMsg_t * pmsg = (SmsSimMsg_t *) data;
    KRIL_SetSmsMti(pmsg->msgTypeInd); // store the MsgType for send SMS ACK message
    if (SMS_MSG_CLASS2 == pmsg->msg.msgRxData.codingScheme.MsgClass)
    {
        KrilWriteMsgInfo_t *tdata = kmalloc(sizeof(KrilWriteMsgInfo_t), GFP_KERNEL);
        tdata->Length = pmsg->pduSize;
        memcpy(tdata->Pdu, pmsg->PDU, tdata->Length);
        tdata->MsgState = 0; // Unread
        KRIL_DEBUG(DBG_INFO, "moreMsgFlag:%d\n", pmsg->msg.msgRxData.moreMsgFlag);
        tdata->MoreSMSToReceive = !(pmsg->msg.msgRxData.moreMsgFlag);
        SetIsRevClass2SMS(TRUE);
        if(!KRIL_DevSpecific_Cmd(BCM_KRIL_CLIENT, RIL_REQUEST_WRITE_SMS_TO_SIM, tdata, sizeof(KrilWriteMsgInfo_t)))
        {
            KRIL_DEBUG(DBG_ERROR,"Command KRIL_REQUEST_INIT_CMD failed\n");
        }
        kfree(tdata);
    }
    else
    {
        KrilMsgPDUInfo_t msg;
        msg.pduSize = pmsg->pduSize;
        memcpy(msg.PDU, pmsg->PDU, pmsg->pduSize);
        KRIL_SendNotify(RIL_UNSOL_RESPONSE_NEW_SMS, &msg, sizeof(KrilMsgPDUInfo_t));
    }
}


//******************************************************************************
//
// Function Name: ProcessNewSMSReport
//
// Description:   Process The New SMS REport
//
// Notes:
//
//******************************************************************************
void  ProcessNewSMSReport(void * data)
{
    SmsSimMsg_t * pmsg = (SmsSimMsg_t *) data;
    KrilMsgPDUInfo_t *msg = kmalloc(sizeof(KrilMsgPDUInfo_t), GFP_KERNEL);
    KRIL_SetSmsMti(pmsg->msgTypeInd); // store the MsgType for send SMS ACK message
    msg->pduSize = pmsg->pduSize;
    msg->pduSize++;
    msg->PDU[0] = 0x00;
    memcpy(&msg->PDU[1], pmsg->PDU, pmsg->pduSize);
    KRIL_DEBUG(DBG_INFO, "pduSize:%d pduSize:%d Number:%s\n", msg->pduSize, pmsg->pduSize, pmsg->daoaAddress.Number);
    KRIL_SendNotify(RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT, msg, sizeof(KrilMsgPDUInfo_t));
    kfree(msg);
}


//******************************************************************************
//
// Function Name: ProcessSuppSvcNotification
//
// Description:   Process The Index of Write SMS In SIM
//
// Notes:
//
//******************************************************************************
void  ProcessSuppSvcNotification(void * data)
{
    SS_CallNotification_t *theSsNotifyPtr = (SS_CallNotification_t *) data;
    SsNotifyParam_t*  theNotifyParamPtr = &theSsNotifyPtr->notify_param;
    CCallType_t    theCallType;
    KrilSuppSvcNotification_t *ndata = kmalloc(sizeof(KrilSuppSvcNotification_t), GFP_KERNEL);

    KRIL_DEBUG(DBG_INFO, "index:%d NotifySS_Oper:%d fac_ie:%s\n", theSsNotifyPtr->index,  theSsNotifyPtr->NotifySS_Oper,  theSsNotifyPtr->fac_ie);
    KRIL_DEBUG(DBG_INFO, "call_state:%d cug_index:%d callingName:%s\n", theNotifyParamPtr->ect_rdn_info.call_state,  theNotifyParamPtr->cug_index, (char *)theNotifyParamPtr->callingName);
    theCallType = KRIL_GetCallType(theSsNotifyPtr->index);
    if (MOVOICE_CALL == theCallType ||
        MODATA_CALL == theCallType ||
        MOFAX_CALL == theCallType)
    {
        ndata->notificationType = 0;
        switch (theSsNotifyPtr->NotifySS_Oper)
        {
            case CALLNOTIFYSS_CFU_ACTIVE:
            case CALLNOTIFYSS_INCALL_FORWARDED:
                ndata->code = 0;
                break;

            case CALLNOTIFYSS_CFC_ACTIVE:
                ndata->code = 1;
                break;

            case CALLNOTIFYSS_OUTCALL_FORWARDED:
                ndata->code = 2;
                break;

            case CALLNOTIFYSS_CALL_WAITING:
                ndata->code = 3;
                break;

            case CALLNOTIFYSS_CUGINDEX:
                ndata->code = 4;
                break;

            case CALLNOTIFYSS_OUTCALL_BARRED:
            case CALLNOTIFYSS_BAOC:
            case CALLNOTIFYSS_BOIC:
            case CALLNOTIFYSS_BOIC_EX_HC:
                ndata->code = 5;
                break;

            case CALLNOTIFYSS_INCALL_BARRED: //Incomming call barred
            case CALLNOTIFYSS_INCOMING_BARRED: //All incoming calls barred
            case CALLNOTIFYSS_BAIC_ROAM:
            case CALLNOTIFYSS_BAC:
                ndata->code = 6;
                break;

            case CALLNOTIFYSS_CLIRSUPREJ:
                ndata->code = 6;
                break;

            case CALLNOTIFYSS_CALL_DEFLECTED:
                ndata->code = 6;
                break;

            default:
                return;
        }
    }
    else if (MTVOICE_CALL == theCallType ||
                MTDATA_CALL == theCallType ||
                MTFAX_CALL == theCallType)
    {
        ndata->notificationType = 1;
        switch (theSsNotifyPtr->NotifySS_Oper)
        {
            case CALLNOTIFYSS_FORWARDED_CALL:
                ndata->code = 0;
                break;

            case CALLNOTIFYSS_CUGINDEX:
                ndata->code = 1;
                break;

            case CALLNOTIFYSS_CALLONHOLD:
                if (MOVOICE_CALL == theCallType)
                {
                    ndata->code = 2;
                }
                else
                {
                    return;
                }
                break;

            case CALLNOTIFYSS_CALLRETRIEVED:
                if (MOVOICE_CALL == theCallType)
                {
                    ndata->code = 3;
                }
                else
                {
                    return;
                }
                break;

            case CALLNOTIFYSS_MPTYIND:
                if (MOVOICE_CALL == theCallType)
                {
                    ndata->code = 4;
                }
                else
                {
                    return;
                }
                break;

            case CALLNOTIFYSS_CALL_ON_HOLD_RELEASED: //this is not a SS notification
                if (MOVOICE_CALL == theCallType)
                {
                    ndata->code = 5;
                }
                else
                {
                    return;
                }
                break;

            case CALLNOTIFYSS_FORWARD_CHECK_SS_MSG:
                ndata->code = 6;
                break;

            case CALLNOTIFYSS_ECT_INDICATION:
            {
                if (ECTSTATE_ALERTING == theSsNotifyPtr->notify_param.ect_rdn_info.call_state)
                    ndata->code = 7;
                else
                    ndata->code = 8;

                if (TRUE == theSsNotifyPtr->notify_param.ect_rdn_info.present_allowed_add ||
                    TRUE == theSsNotifyPtr->notify_param.ect_rdn_info.present_restricted_add)
                {
                    ndata->type = 0x80 | (theSsNotifyPtr->notify_param.ect_rdn_info.phone_number.ton << 4) | theSsNotifyPtr->notify_param.ect_rdn_info.phone_number.npi;
                    strcpy(ndata->number, (char*)theSsNotifyPtr->notify_param.ect_rdn_info.phone_number.number);
                }
                break;
            }

            case CALLNOTIFYSS_DEFLECTED_CALL:
                ndata->code = 9;
                break;

            default:
                return;
        }
    }
    else
    {
        kfree(ndata);
        return;
    }
    if (CALLNOTIFYSS_CUGINDEX == theSsNotifyPtr->NotifySS_Oper)
    {
        ndata->index = theSsNotifyPtr->notify_param.cug_index;
    }
    else
    {
        ndata->index = 0;
    }
    KRIL_SendNotify(RIL_UNSOL_SUPP_SVC_NOTIFICATION, ndata, sizeof(RIL_SuppSvcNotification));
    kfree(ndata);
}


//******************************************************************************
//
// Function Name: ProcessNotification
//
// Description:  Process the notify message from CP side.
//
// Notes:
//
//******************************************************************************
void ProcessNotification(Kril_CAPI2Info_t *notify)
{
    KRIL_DEBUG(DBG_TRACE, "msgType:0x%lX!\n", (UInt32)notify->msgType);

    switch((UInt32)notify->msgType)
        {
        case MSG_INCOMING_CALL_IND:
            {
            CallReceiveMsg_t * pIncomingCall = (CallReceiveMsg_t *) notify->dataBuf;
            KRIL_SetIncomingCallIndex(pIncomingCall->callIndex);
            KRIL_SetCallNumPresent(pIncomingCall->callIndex, pIncomingCall->callingInfo.present);
            KRIL_SendNotify(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
            break;
            }

        case MSG_VOICECALL_WAITING_IND:
        {
            VoiceCallWaitingMsg_t * pWaitingCall = (VoiceCallWaitingMsg_t *) notify->dataBuf;
            KRIL_SetWaitingCallIndex(pWaitingCall->callIndex);
            KRIL_SetCallNumPresent(pWaitingCall->callIndex, pWaitingCall->callingInfo.present);
            KRIL_SendNotify(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
            break;
        }

        case MSG_REG_GSM_IND:
            ProcessGSMStatus(notify->dataBuf);
            break;

        case MSG_REG_GPRS_IND:
            ProcessGPRSStatus(notify->dataBuf);
            break;

        case MSG_UE_3G_STATUS_IND:
             ProcessUE3GStatus(notify->dataBuf);
             break;

        case MSG_SMSPP_STORED_IND: //CMTI:
        case MSG_SMSCB_STORED_IND: //CBMI
        case MSG_SMSSR_STORED_IND: //CDSI
            ProcessSMSIndexInSIM(notify->dataBuf);
            break;

        case MSG_SMSPP_REGULAR_PUSH_IND: //MMS
        case MSG_SMSPP_SECURE_PUSH_IND:
        case MSG_SMSPP_DELIVER_IND: //+CMT:
        case MSG_SMSPP_OTA_SYNC_IND:
        case MSG_SMSPP_OTA_IND:
            ProcessNewSMSMessage(notify->dataBuf);
            break;

        case MSG_SMSSR_REPORT_IND: // CDS:
            ProcessNewSMSReport(notify->dataBuf);
            break;

        case MSG_GPRS_DEACTIVATE_IND:
        {
            KRIL_DEBUG(DBG_INFO, "MSG_GPRS_DEACTIVATE_IND::not process\n");
            break;
        }

        case MSG_PDP_DEACTIVATION_IND:
        {
            PDP_PDPDeactivate_Ind_t *pind = (PDP_PDPDeactivate_Ind_t *) notify->dataBuf;
            UInt8 i;
            
            KRIL_DEBUG(DBG_INFO, "MSG_PDP_DEACTIVATION_IND::cid %d,type[%s],add[%s]\n", pind->cid, pind->pdpType, pind->pdpAddress);
            for (i=0; i<BCM_NET_MAX_PDP_CNTXS; i++)
            {
                if (pind->cid == pdp_resp[i].cid)
                {
                    //pdp_resp[i].active = 0;
                    memset(&pdp_resp[i], 0, sizeof(KrilDataCallResponse_t));
                    KRIL_DEBUG(DBG_INFO, "MSG_PDP_DEACTIVATION_IND[%d]=%d \n", i, pdp_resp[i].cid);
                    KRIL_SendNotify(RIL_UNSOL_DATA_CALL_LIST_CHANGED, &pdp_resp[i], sizeof(KrilDataCallResponse_t));
                    //pdp_resp[i].cid = 0; //KRIL_SendNotify has backup pdp_resp by memcpy, so we can modify here
                    break;
                }
            }
            break;
        }

        case MSG_CELL_INFO_IND:
            KRIL_DEBUG(DBG_INFO, "MSG_CELL_INFO_IND::not process\n");
            break;

        case MSG_SIM_DETECTION_IND:
        {
            SIM_DETECTION_t *pind = (SIM_DETECTION_t *) notify->dataBuf;
            RIL_RadioState radiostate;
            
            KRIL_DEBUG(DBG_INFO, "MSG_SIM_DETECTION_IND::sim_appl_type:%d ruim_supported:%d\n", pind->sim_appl_type, pind->ruim_supported);
            KRIL_SetSimAppType(pind->sim_appl_type);
            
            if (SIM_APPL_INVALID == pind->sim_appl_type)
            {
                Kril_SIMEmergency data;
                data.simAppType = pind->sim_appl_type;
                KRIL_SendNotify(BRIL_UNSOL_EMERGENCY_NUMBER, &data, sizeof(Kril_SIMEmergency));
                radiostate = RADIO_STATE_SIM_LOCKED_OR_ABSENT;
            }
            else
            {
                radiostate = RADIO_STATE_SIM_READY;
            }

            // Do not send the radio state change notification to Android framework in flight mode to avoid to confusing its state machine.
            if (!gIsFlightModeOnBoot)
            {
                KRIL_SendNotify(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, &radiostate, sizeof(RIL_RadioState));
            }
            break;
        }

        case MSG_USSD_DATA_IND:
            ProcessUSSDDataInd(notify->dataBuf);
            break;

        case MSG_USSD_CALLINDEX_IND:
            ProcessUSSDCallIndexInd(notify->dataBuf);
            break;

        case MSG_USSD_SESSION_END_IND:
            ProcessUSSDSessionEndInd(notify->dataBuf);
            break;

        case MSG_SMS_READY_IND:
            KRIL_DEBUG(DBG_INFO, "MSG_SMS_READY_IND::not process\n");
            break;

        case MSG_SIM_CACHED_DATA_READY_IND:
            if(!KRIL_DevSpecific_Cmd(BCM_KRIL_CLIENT, KRIL_REQUEST_QUERY_SMS_IN_SIM, NULL, 0))
            {
                KRIL_DEBUG(DBG_ERROR,"Command KRIL_REQUEST_QUERY_SMS_IN_SIM failed\n");
            }
            break;

        case MSG_DATE_TIMEZONE_IND:
            ProcessTimeZoneInd(notify->dataBuf);
            break;

        case MSG_PBK_READY_IND:
            if(!KRIL_DevSpecific_Cmd(BCM_KRIL_CLIENT, KRIL_REQUEST_QUERY_SIM_EMERGENCY_NUMBER, NULL, 0))
            {
                KRIL_DEBUG(DBG_ERROR,"Command KRIL_REQUEST_QUERY_SIM_EMERGENCY_NUMBER failed\n");
            }
            break;

        case MSG_HOMEZONE_STATUS_IND:
            KRIL_DEBUG(DBG_INFO, "MSG_HOMEZONE_STATUS_IND::not process\n");
            break;

        case MSG_RSSI_IND:
            ProcessRSSIInfo(notify->dataBuf);
            break;

        case MSG_VOICECALL_ACTION_RSP:
        {
            VoiceCallActionMsg_t *ndata = (VoiceCallActionMsg_t*) notify->dataBuf;
            KRIL_DEBUG(DBG_INFO, "MSG_VOICECALL_ACTION_RSP::callResult:%d callIndex:%d errorCause:%lu\n", (UInt8)ndata->callResult, ndata->callIndex, (UInt32)ndata->errorCause);
            break;
        }

        case MSG_VOICECALL_RELEASE_IND:
        {
            VoiceCallReleaseMsg_t *ndata = (VoiceCallReleaseMsg_t*) notify->dataBuf;
            KRIL_DEBUG(DBG_INFO, "MSG_VOICECALL_RELEASE_IND::cause:%lu\n", (UInt32)ndata->exitCause);
            KRIL_SetLastCallFailCause( KRIL_MNCauseToRilError(ndata->exitCause) );
            break;
        }

        case MSG_VOICECALL_PRECONNECT_IND:
        {
            VoiceCallPreConnectMsg_t *ndata = (VoiceCallPreConnectMsg_t*) notify->dataBuf;
            KRIL_DEBUG(DBG_INFO, "MSG_VOICECALL_PRECONNECT_IND::callIndex:%d\n",ndata->callIndex);
            break;
        }

        case MSG_VOICECALL_CONNECTED_IND:
        {
            VoiceCallConnectMsg_t *ndata = (VoiceCallConnectMsg_t*) notify->dataBuf;
            KRIL_DEBUG(DBG_INFO, "MSG_VOICECALL_CONNECTED_IND::callIndex:%d\n",ndata->callIndex);
            KRIL_SendNotify(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
            break;
        }

        case MSG_CALL_STATUS_IND:
        {
            CallStatusMsg_t *pCallStatusMsg = (CallStatusMsg_t *) notify->dataBuf;
            KRIL_DEBUG(DBG_INFO, "MSG_CALL_STATUS_IND::callstatus:%d\n", pCallStatusMsg->callstatus);

#ifdef VIDEO_TELEPHONY_ENABLE
            if (MOVIDEO_CALL == pCallStatusMsg->callType && CC_CALL_ALERTING == pCallStatusMsg->callstatus)
            {
                int callIndex = pCallStatusMsg->callIndex;
                KRIL_SendNotify(RIL_UNSOL_RESPONSE_VT_CALL_EVENT_PROGRESS_INFO_IND, &callIndex, sizeof(int));
            }
            else 
#endif //VIDEO_TELEPHONY_ENABLE            
            if (//CC_CALL_BEGINNING == pCallStatusMsg->callstatus ||
                     CC_CALL_ACTIVE == pCallStatusMsg->callstatus      ||
                     CC_CALL_HOLD == pCallStatusMsg->callstatus        ||
                     CC_CALL_WAITING == pCallStatusMsg->callstatus     ||
                     CC_CALL_ALERTING == pCallStatusMsg->callstatus    ||
                     CC_CALL_CALLING == pCallStatusMsg->callstatus     ||
                     CC_CALL_DISCONNECT == pCallStatusMsg->callstatus
              )
            {
                KRIL_SendNotify(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
            }
            break;
        }

        case MSG_DTMF_STATUS_IND:
        {
            ApiDtmfStatus_t *pind = (ApiDtmfStatus_t *) notify->dataBuf;
            KRIL_DEBUG(DBG_INFO, "callIndex:%d dtmfIndex:%d dtmfTone:%d toneValume:%d isSilent:%d duration:%lu\n", pind->dtmfObj.callIndex, pind->dtmfObj.dtmfIndex, pind->dtmfObj.dtmfTone, pind->dtmfObj.toneValume, pind->dtmfObj.isSilent, pind->dtmfObj.duration);
            KRIL_DEBUG(DBG_INFO, "dtmfState:%d cause:%d\n", pind->dtmfState, pind->cause);

            if(DTMF_STATE_IDLE == pind->dtmfState && CC_OPERATION_SUCCESS == pind->cause)
            {


            }
            break;
        }
        
#ifdef VIDEO_TELEPHONY_ENABLE        
        case MSG_DATACALL_RELEASE_IND:
        {
            DataCallReleaseMsg_t *pCallReleaseMsg = (DataCallReleaseMsg_t*) notify->dataBuf;
            int callIndex = pCallReleaseMsg->callIndex;
            KRIL_DEBUG(DBG_INFO, "MSG_DATACALL_RELEASE_IND::callIndex:%d exitCause: 0x%02X\n", pCallReleaseMsg->callIndex, pCallReleaseMsg->exitCause);
            
            KRIL_SendNotify(RIL_UNSOL_RESPONSE_VT_CALL_EVENT_END, &callIndex, sizeof(int));
            break;
        }
#endif //VIDEO_TELEPHONY_ENABLE

        case MSG_SS_NOTIFY_EXTENDED_CALL_TRANSFER:
        case MSG_SS_NOTIFY_CALLING_NAME_PRESENT:
        case MSG_SS_NOTIFY_CLOSED_USER_GROUP:
        case MSG_SS_CALL_NOTIFICATION:
             ProcessSuppSvcNotification(notify->dataBuf);
             break;

        case MSG_SATK_EVENT_SETUP_MENU:
        {
            KRIL_DEBUG(DBG_INFO,"MSG_SATK_EVENT_SETUP_MENU\n");
            ProcessSATKSetupMenu(notify->dataBuf);
            break;
        }

        case MSG_SATK_EVENT_SELECT_ITEM:
        {
            KRIL_DEBUG(DBG_INFO,"MSG_SATK_EVENT_SELECT_ITEM\n");
            ProcessSATKSelectItem(notify->dataBuf);
            break;
        }

        case MSG_SATK_EVENT_GET_INPUT:
        {
            KRIL_DEBUG(DBG_INFO,"MSG_SATK_EVENT_GET_INPUT\n");
            ProcessSATKGetInput(notify->dataBuf);
            break;
        }
        
        case MSG_SATK_EVENT_GET_INKEY:
        {
            KRIL_DEBUG(DBG_INFO,"MSG_SATK_EVENT_GET_INKEY\n");
            ProcessSATKGetInkey(notify->dataBuf);
            break;
        }
        
        case MSG_SATK_EVENT_DISPLAY_TEXT:
        {
            KRIL_DEBUG(DBG_INFO,"MSG_SATK_EVENT_DISPLAY_TEXT\n");
            ProcessSATKDisplayText(notify->dataBuf);
            break;
        }
                
        case MSG_SATK_EVENT_SEND_SHORT_MSG:
        {
            KRIL_DEBUG(DBG_INFO,"MSG_SATK_EVENT_SEND_SHORT_MSG\n");
            ProcessSATKSendMOSMS(notify->dataBuf);
            break;
        }
        
        case MSG_SATK_EVENT_SEND_SS:
        {
            KRIL_DEBUG(DBG_INFO,"MSG_SATK_EVENT_SEND_SS\n");
            ProcessSATKSendSs(notify->dataBuf);
            break;
        }
        
        case MSG_SATK_EVENT_SEND_USSD:
        {
            KRIL_DEBUG(DBG_INFO,"MSG_SATK_EVENT_SEND_USSD\n");
            ProcessSATKSendUssd(notify->dataBuf);
            break;
        }
        
        case MSG_SATK_EVENT_PLAY_TONE:
        {
            KRIL_DEBUG(DBG_INFO,"MSG_SATK_EVENT_PLAY_TONE\n");
            ProcessSATKPlayTone(notify->dataBuf);
            break;
        }
        
        case MSG_SATK_EVENT_SEND_DTMF:
        {
            KRIL_DEBUG(DBG_INFO,"MSG_SATK_EVENT_SEND_DTMF\n");
            ProcessSATKSendStkDtmf(notify->dataBuf);
            break;
        }
        
        case MSG_SATK_EVENT_SETUP_CALL:
        {
            KRIL_DEBUG(DBG_INFO,"MSG_SATK_EVENT_SETUP_CALL\n");
            ProcessSATKSetupCall(notify->dataBuf);
            break;
        }
        
        case MSG_SATK_EVENT_IDLEMODE_TEXT:
        {
            KRIL_DEBUG(DBG_INFO,"MSG_SATK_EVENT_IDLEMODE_TEXT\n");
            ProcessSATKIdleModeText(notify->dataBuf);
            break;
        }
        
        case MSG_SATK_EVENT_REFRESH:
        {
            KRIL_DEBUG(DBG_INFO,"MSG_SATK_EVENT_REFRESH\n");
            ProcessSATKRefresh(notify->dataBuf);
            break;
        }
        
        case MSG_SATK_EVENT_LAUNCH_BROWSER:
        {
            KRIL_DEBUG(DBG_INFO,"MSG_SATK_EVENT_LAUNCH_BROWSER\n");
            ProcessSATKLaunchBrowser(notify->dataBuf);
            break;
        }
        
        case MSG_SATK_EVENT_STK_SESSION_END:
        {    
            KRIL_DEBUG(DBG_INFO,"MSG_SATK_EVENT_STK_SESSION_END\n");
            KRIL_SendNotify(RIL_UNSOL_STK_SESSION_END, NULL, 0);
            break;
        }

        case MSG_PMU_BATT_LEVEL_IND:
        {
            KRIL_DEBUG(DBG_INFO,"MSG_PMU_BATT_LEVEL_IND\n");
            PoressDeviceNotifyCallbackFun(RIL_NOTIFY_DEVSPECIFIC_BATT_LEVEL, notify->dataBuf, notify->dataLength);
            break;
        }

        case MSG_CAPI2_AT_RESPONSE_IND:
        {
            KRIL_DEBUG(DBG_INFO,"MSG_CAPI2_AT_RESPONSE_IND\n");
            PoressDeviceNotifyCallbackFun(RIL_NOTIFY_DEVSPECIFIC_ATCMD_RESP, notify->dataBuf, notify->dataLength);
            break;
        }

        case MSG_SMSCB_DATA_IND:
        {
            SmsStoredSmsCb_t *pind = (SmsStoredSmsCb_t*) notify->dataBuf;
            char data[88];

            KRIL_DEBUG(DBG_INFO,"MSG_SMSCB_DATA_IND\n");
            memset(data, 0x00, 88);
            data[0] = (pind->Serial & 0xFF00) >> 8;
            data[1] = pind->Serial & 0x00FF;
            data[2] = (pind->MsgId & 0xFF00) >> 8;
            data[3] = pind->MsgId & 0x00FF;
            data[4] = pind->Dcs;
            data[5] = ((pind->NoPage & 0x0F) << 4) | (pind->NbPages & 0x0F);

            if (CB_DATA_PER_PAGE_SZ < pind->NoOctets)
            {
                KRIL_DEBUG(DBG_ERROR,"Warning!! MSG_SMSCB_DATA_IND over 82 byte msg size: %d\n", pind->NoOctets);
                pind->NoOctets = CB_DATA_PER_PAGE_SZ;
            }

            memcpy(&data[6], pind->Msg, pind->NoOctets);
            KRIL_DEBUG(DBG_INFO,"Ser 0x%x, Msg 0x%x, Dcs %d, NoP %d, NbP %d\n", pind->Serial, pind->MsgId, pind->Dcs, pind->NoPage, pind->NbPages);
            KRIL_SendNotify(RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS, data, 88);
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_INFO, "The msgType:0x%lX is not process yet...!\n", (UInt32)notify->msgType);
            break;
        }
    }
}

