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
#include "bcm_kril_simlockfun.h"


#define  APDUFILEID_EF_DIR            0x2F00
#define  APDUFILEID_DF_GRAPHICS       0x5F50
#define  APDUFILEID_EF_MML            0x4F47
#define  APDUFILEID_EF_MMDF           0x4F48
#define  APDUFILEID_DF_MULTIMEDIA     0X5F3B
#define  APDUFILEID_EF_PBK_ADN        0x4F3A
#define  APDUFILEID_EF_PBK_EMAIL      0x4F50
#define  APDUFILEID_EF_PBK_GRP1       0x4F25
#define  APDUFILEID_EF_PBK_EXT1       0x4F4A
#define  APDUFILEID_EF_PBK_AAS        0x4F4B
#define  APDUFILEID_EF_PBK_GAS        0x4F4C
#define  APDUFILEID_EF_PBK_PBC        0x4F09
#define  APDUFILEID_EF_PBK_ANRA       0x4F11
#define  APDUFILEID_EF_PBK_ANRB       0x4F13
#define  APDUFILEID_EF_PBK_SNE        0x4F19
#define  APDUFILEID_EF_PBK_UID        0x4F21
#define  APDUFILEID_EF_PBK_GRP        0x4F26
#define  APDUFILEID_EF_PBK_ANRC       0x4F15
#define  APDUFILEID_EF_PBK_ADN1       0x4F3B
#define  APDUFILEID_EF_PBK_PBC1       0x4F0A
#define  APDUFILEID_EF_PBK_ANRA1      0x4F12
#define  APDUFILEID_EF_PBK_ANRB1      0x4F14
#define  APDUFILEID_EF_PBK_EMAIL1     0x4F51
#define  APDUFILEID_EF_PBK_SNE1       0x4F1A
#define  APDUFILEID_EF_PBK_ANRC1      0x4F16
#define  APDUFILEID_DF_SOLSA          0X5F70

#define CONVERT_USIM_SELECT_RESPONSE_FORMAT_TO_SIM
#define ANDROID_SIMIO_PATH_AVAILABLE

const UInt16 Sim_Df_Telecom_Path[1] =        {APDUFILEID_MF};
const UInt16 Sim_Df_Gsm_Path[1] =            {APDUFILEID_MF};
const UInt16 Sim_Df_ADFusim_Path[1] =        {APDUFILEID_MF};
const UInt16 Sim_Df_Global_PhoneBK_Path[2] = {APDUFILEID_MF, APDUFILEID_DF_TELECOM};
const UInt16 Sim_Df_Local_PhoneBK_Path[2]  = {APDUFILEID_MF, APDUFILEID_USIM_ADF};
const UInt16 Sim_Df_Graphics_Path[2] =       {APDUFILEID_MF, APDUFILEID_DF_TELECOM};
const UInt16 Sim_Df_Acenet_Path[2] =         {APDUFILEID_MF, APDUFILEID_DF_CINGULAR};
const UInt16 Sim_Df_Cingular_Path[1] =       {APDUFILEID_MF};
const UInt16 Sim_Df_Multimedia_Path[1] =     {APDUFILEID_MF};
const UInt16 Sim_Df_SoLSA_Path[2] =          {APDUFILEID_MF, APDUFILEID_DF_GSM};
const UInt16 Sim_Df_SoLSA_Usim_Path[2] =     {APDUFILEID_MF, APDUFILEID_USIM_ADF};

Int16         gImageInstCount = 0;
APDUFileID_t  gImageInstance[30];
static SIM_PIN_Status_t sSimPinstatus = NO_SIM_PIN_STATUS;
static SIMLOCK_SIM_DATA_t *sSimdata = NULL;

void SetSimPinStatusHandle(SIM_PIN_Status_t SimPinState);
SIM_PIN_Status_t GetSimPinStatusHandle(void);

extern SS_CallBarType_t ssBarringTypes[];
extern SS_SvcCls_t GetServiceClass (int InfoClass);
extern int SvcClassToATClass (SS_SvcCls_t InfoClass);

Boolean SimIOGetDedicatedFileId(APDUFileID_t efile_id, 
                             SIM_APPL_TYPE_t simAppType, 
                             APDUFileID_t *dfile_id
                             )
{
    Boolean result = FALSE;
    UInt8 Identifier;
    
    switch (efile_id)
    {
        case APDUFILEID_EF_ADN    : 
        case APDUFILEID_EF_FDN    :
        case APDUFILEID_EF_SMS    :
        case APDUFILEID_EF_CCP    :
        case APDUFILEID_EF_MSISDN :
        case APDUFILEID_EF_SMSP   :
        case APDUFILEID_EF_SMSS   :
        case APDUFILEID_EF_LND    :
        case APDUFILEID_EF_SMSR   :
        case APDUFILEID_EF_SDN    :
        case APDUFILEID_EF_EXT1   :
        case APDUFILEID_EF_EXT2   :
        case APDUFILEID_EF_EXT3   :
        case APDUFILEID_EF_BDN    :
        case APDUFILEID_EF_EXT4   : 
        case APDUFILEID_EF_ECCP   :   
        case APDUFILEID_EF_CMI    :
        case APDUFILEID_DF_GRAPHICS :
        {
            *dfile_id = APDUFILEID_DF_TELECOM;
            break;
        }
        
        case APDUFILEID_DF_PHONEBK:
        case APDUFILEID_DF_MULTIMEDIA:
        {    
            if(SIM_APPL_2G == simAppType)
                goto Error;
            
            *dfile_id = APDUFILEID_DF_TELECOM; 
            break;
        }
        
        case APDUFILEID_EF_LP     :
        case APDUFILEID_EF_IMSI   :
        case APDUFILEID_EF_KC     :
        case APDUFILEID_EF_PLMNSEL  :
        case APDUFILEID_EF_HPLMN  : 
        case APDUFILEID_EF_ACMMAX :
        case APDUFILEID_EF_SST    : //2G:APDUFILEID_EF_SST 3G:APDUFILEID_EF_UST_3G
        case APDUFILEID_EF_ACM    :
        case APDUFILEID_EF_GID1   :
        case APDUFILEID_EF_GID2   :
        case APDUFILEID_EF_PUCT   :
        case APDUFILEID_EF_CBMI   :
        case APDUFILEID_EF_SPN    :
        case APDUFILEID_EF_CBMID  : 
        case APDUFILEID_EF_BCCH   :
        case APDUFILEID_EF_ACC    :
        case APDUFILEID_EF_FPLMN  : 
        case APDUFILEID_EF_LOCI   :
        case APDUFILEID_EF_AD     :
        case APDUFILEID_EF_PHASE  : 
        case APDUFILEID_EF_VGCS   :     
        case APDUFILEID_EF_VGCSS  : 
        case APDUFILEID_EF_VBS    :
        case APDUFILEID_EF_VBSS   :
        case APDUFILEID_EF_EMLPP  : 
        case APDUFILEID_EF_AAEM   :
        case APDUFILEID_EF_ECC    :
        case APDUFILEID_EF_CBMIR  :
        case APDUFILEID_EF_DCK    :   
        case APDUFILEID_EF_CNL    :
        case APDUFILEID_EF_NIA    :
        case APDUFILEID_EF_SUME   : 
        case APDUFILEID_EF_PLMN_WACT:   
        case APDUFILEID_EF_OPLMN_WACT:  
        case APDUFILEID_EF_HPLMN_WACT:  
        case APDUFILEID_EF_CPBCCH : 
        case APDUFILEID_EF_INV_SCAN : 
        case APDUFILEID_EF_RPLMN_ACT:   
        case APDUFILEID_EF_PNN    :
        case APDUFILEID_EF_OPL    : 
        case APDUFILEID_EF_MBDN   : 
        case APDUFILEID_EF_EXT6   : 
        case APDUFILEID_EF_MBI    :
        case APDUFILEID_EF_MWIS   : 
        case APDUFILEID_EF_CFIS   :
        case APDUFILEID_EF_EXT7   :
        case APDUFILEID_EF_SPDI   :
        case APDUFILEID_EF_MMSN   : 
        case APDUFILEID_EF_EXT8   :
        case APDUFILEID_EF_MMSICP : 
        case APDUFILEID_EF_MMSUP  :
        {
            if (SIM_APPL_3G == simAppType)
            {
                *dfile_id = APDUFILEID_USIM_ADF;
            }
            else if (SIM_APPL_2G == simAppType)
            {
                *dfile_id = APDUFILEID_DF_GSM;
            }
            else
            {
                goto Error;
            }
            break;
        }

        case APDUFILEID_EF_KC_GPRS  : 
        case APDUFILEID_EF_LOCI_GPRS:
	      // The following EF are defined in Common PCN Handset 
	      // Sepcification (CPHS): under DF GSM. They are not defined in GSM 11.11.
        case APDUFILEID_EF_MWI_CPHS :
        case APDUFILEID_EF_CFL_CPHS :   
        case APDUFILEID_EF_ONS_CPHS :   
        case APDUFILEID_EF_CSP_CPHS :   
        case APDUFILEID_EF_INFO_CPHS:   
        case APDUFILEID_EF_MBN_CPHS :   
        case APDUFILEID_EF_ONSS_CPHS:   
        case APDUFILEID_EF_IN_CPHS  :
        {
            *dfile_id = APDUFILEID_DF_GSM;    
            break;
        }
                  
        case APDUFILEID_EF_EST_3G: /* EF-EST, exists only in USIM */
        case APDUFILEID_EF_ACL_3G: /* EF-ACL, exists only in USIM */
        {
            if(SIM_APPL_2G == simAppType)
                goto Error;
            
            *dfile_id = APDUFILEID_USIM_ADF;
            break;
        }
        
        case APDUFILEID_EF_ACTHPLMN: 
        {
            *dfile_id = APDUFILEID_DF_ACTNET;
            break;
        }
        
        case APDUFILEID_EF_CINGULAR_TST:
        {
            *dfile_id = APDUFILEID_DF_CINGULAR;
            break;
        }
        
        case APDUFILEID_EF_IMG:
        {
            *dfile_id = (APDUFileID_t)APDUFILEID_DF_GRAPHICS;
            break;
        }
        
        case APDUFILEID_EF_PSC:
        case APDUFILEID_EF_CC:
        case APDUFILEID_EF_PUID:
        case APDUFILEID_EF_PBR:
        case APDUFILEID_EF_PBK_ADN:
        case APDUFILEID_EF_PBK_EMAIL:
        case APDUFILEID_EF_PBK_GRP1:
        case APDUFILEID_EF_PBK_EXT1:  
        case APDUFILEID_EF_PBK_AAS:   
        case APDUFILEID_EF_PBK_GAS:   
        case APDUFILEID_EF_PBK_PBC:   
        case APDUFILEID_EF_PBK_ANRA:  
        case APDUFILEID_EF_PBK_ANRB:  
        case APDUFILEID_EF_PBK_SNE:   
        case APDUFILEID_EF_PBK_UID:   
        case APDUFILEID_EF_PBK_GRP:   
        case APDUFILEID_EF_PBK_ANRC:  
        case APDUFILEID_EF_PBK_ADN1:  
        case APDUFILEID_EF_PBK_PBC1:  
        case APDUFILEID_EF_PBK_ANRA1: 
        case APDUFILEID_EF_PBK_ANRB1: 
        case APDUFILEID_EF_PBK_EMAIL1:
        case APDUFILEID_EF_PBK_SNE1:  
        case APDUFILEID_EF_PBK_ANRC1: 
        {
            if (SIM_APPL_2G == simAppType)
                goto Error;
            
            *dfile_id = APDUFILEID_DF_PHONEBK;
            break;
        }
        
        case APDUFILEID_EF_MML:
        case APDUFILEID_EF_MMDF:
        {
            if (SIM_APPL_2G == simAppType)
                goto Error;
            
            *dfile_id = (APDUFileID_t)APDUFILEID_DF_MULTIMEDIA;
            break;
        }
            
        case APDUFILEID_EF_ICCID:
        case APDUFILEID_EF_PL:
        case APDUFILEID_EF_DIR: //The presence of EFDIR on a SIM is optional
        case APDUFILEID_MF:
        case APDUFILEID_DF_TELECOM:
        case APDUFILEID_DF_GSM:
        case APDUFILEID_USIM_ADF:
        {
            *dfile_id = APDUFILEID_MF;
            break;
        }
        
        case APDUFILEID_EF_ARR_UNDER_MF:
        {
            if (SIM_APPL_2G == simAppType)
                goto Error;
            
            *dfile_id = APDUFILEID_MF;
            break;
        }
        
        default:          
            break;    
    }
    
    if (APDUFILEID_INVALID_FILE == *dfile_id)
    {
        //Some special EF ID need to be handled.
        Identifier = (UInt8)(((UInt16)efile_id & 0xFF00) >> 8);
        
        if (0x4F == Identifier)
        {
            UInt16 i;
            
            for (i = 0 ; i < gImageInstCount ; i++)
            {
                if (gImageInstance[i] == efile_id)
                {
                    KRIL_DEBUG(DBG_INFO,"The EF:0x%X belongs to Image Instance\n",efile_id);
                    *dfile_id = (APDUFileID_t)APDUFILEID_DF_GRAPHICS;
                    break;
                }
            }
        }
    }
    
    if (APDUFILEID_INVALID_FILE == *dfile_id)
        goto Error;
        
    result = TRUE;

Error:
    if (FALSE == result)
        KRIL_DEBUG(DBG_ERROR,"Unknow or not support EF ID:0x%X\n",efile_id);
    
    return result;
}


Boolean SimIOGetDedicatedFilePath(APDUFileID_t        dfile_id, 
                                  SIMServiceStatus_t  localPBKStatus, 
                                  KrilSimDedFilePath_t *path_info)
{
    Boolean result = FALSE;
    
    switch (dfile_id)
    {
        case APDUFILEID_DF_TELECOM:
            
            path_info->select_path = Sim_Df_Telecom_Path;
            path_info->path_len = sizeof(Sim_Df_Telecom_Path) / sizeof(UInt16);
            break;
        
        case APDUFILEID_DF_GSM:
            
            path_info->select_path = Sim_Df_Gsm_Path;
            path_info->path_len = sizeof(Sim_Df_Gsm_Path) / sizeof(UInt16);
            break;
        
        case APDUFILEID_USIM_ADF:
            
            path_info->select_path = Sim_Df_ADFusim_Path;
            path_info->path_len = sizeof(Sim_Df_ADFusim_Path) / sizeof(UInt16);
            break;
        
        case APDUFILEID_DF_PHONEBK:
            
            if (SIMSERVICESTATUS_ACTIVATED == localPBKStatus)
            {
                path_info->select_path = Sim_Df_Local_PhoneBK_Path;
                path_info->path_len = sizeof(Sim_Df_Local_PhoneBK_Path) / sizeof(UInt16);
            }
            else
            {
                path_info->select_path = Sim_Df_Global_PhoneBK_Path;
                path_info->path_len = sizeof(Sim_Df_Global_PhoneBK_Path) / sizeof(UInt16);
            }
            break;
        
        case APDUFILEID_DF_GRAPHICS:
            
            path_info->select_path = Sim_Df_Graphics_Path;
            path_info->path_len = sizeof(Sim_Df_Graphics_Path) / sizeof(UInt16);
            break;
        
        case APDUFILEID_DF_ACTNET:
            
            path_info->select_path = Sim_Df_Acenet_Path;
            path_info->path_len = sizeof(Sim_Df_Acenet_Path) / sizeof(UInt16);
            break;
        
        case APDUFILEID_DF_CINGULAR:
            
            path_info->select_path = Sim_Df_Cingular_Path;
            path_info->path_len = sizeof(Sim_Df_Cingular_Path) / sizeof(UInt16);
            break;
        
        case APDUFILEID_DF_MULTIMEDIA:
            
            path_info->select_path = Sim_Df_Multimedia_Path;
            path_info->path_len = sizeof(Sim_Df_Multimedia_Path) / sizeof(UInt16);
            break;
        
        case APDUFILEID_DF_SOLSA:
            
            path_info->select_path = Sim_Df_SoLSA_Path;
            path_info->path_len = sizeof(Sim_Df_SoLSA_Path) / sizeof(UInt16);
            break;
            
        case APDUFILEID_MF:
            
            path_info->select_path = NULL; 
            path_info->path_len = 0;
            break;
        
        default:
            KRIL_DEBUG(DBG_ERROR,"Unknow or not support DF ID:0x%X\n",dfile_id);
            goto Error;
    }
    result = TRUE;

Error:
    return result;  
}


void SimIOSubmitRestrictedAccess(KrilSimIOCmd_t     *cmd_data, 
                                 APDUFileID_t       efile_id, 
                                 APDUFileID_t       dfile_id, 
                                 KrilSimDedFilePath_t *path_info)
{
    Int16 i;
    
    if (path_info->path_len > 0)
    {
        for(i = 0 ; i < path_info->path_len ; i++ )
            KRIL_DEBUG(DBG_INFO,"select_path[%d]:0x%X\n", i, path_info->select_path[i]);
    }
    else
    {
        KRIL_DEBUG(DBG_INFO,"select_path is NULL\n");
    }
    
    if (cmd_data->data[0] != '\0')
    {
        CAPI2_SIM_SubmitRestrictedAccessReq(GetNewTID(), GetClientID(), USIM_BASIC_CHANNEL_SOCKET_ID,
            (APDUCmd_t)cmd_data->command, efile_id, dfile_id, (UInt8)cmd_data->p1, (UInt8)cmd_data->p2,
            (UInt8)cmd_data->p3, path_info->path_len, path_info->select_path, (UInt8*)cmd_data->data);
    }
    else
    {
        CAPI2_SIM_SubmitRestrictedAccessReq(GetNewTID(), GetClientID(), USIM_BASIC_CHANNEL_SOCKET_ID,
            (APDUCmd_t)cmd_data->command, efile_id, dfile_id, (UInt8)cmd_data->p1, (UInt8)cmd_data->p2,
            (UInt8)cmd_data->p3, path_info->path_len, path_info->select_path, NULL);
    }    
}


Boolean SimIOGetImageInstfileID(const UInt8 *sim_data)
{
    Boolean result = FALSE;
    Int16 CurInstance, CurDataOffset;
    UInt8* pbData;
    
    if (NULL == sim_data)
    {
        KRIL_DEBUG(DBG_ERROR,"sim_data is NULL Error!!\n");
        goto Error;
    }
        
    gImageInstCount = sim_data[0];
    
    if (1 > gImageInstCount || 30 < gImageInstCount)
    {
        KRIL_DEBUG(DBG_ERROR,"InstanceCount is invalid: %d\n",gImageInstCount);
        goto Error;
    }
    
    //retrieve the image instance file ID from sim data
    for (CurInstance = 0 ; CurInstance < gImageInstCount ; CurInstance++)
    {
        CurDataOffset = SIM_IMAGE_INSTANCE_COUNT_SIZE + (CurInstance * SIM_IMAGE_INSTANCE_SIZE);
        pbData = (UInt8*)(sim_data + CurDataOffset);
        
        gImageInstance[CurInstance] = (APDUFileID_t)((UInt16)pbData[3] << 8 | (UInt16)pbData[4]);
        KRIL_DEBUG(DBG_INFO,"SIM IMAGE INSTANCE: gImageInstance[%d]: 0x%X\n", CurInstance, gImageInstance[CurInstance]);
    }

    result = TRUE;
        
Error:
    return  result;  
}


Boolean SimIOEFileUndefFirstUSIMADF(APDUFileID_t efile_id, SIM_APPL_TYPE_t SimAppType)
{
    Boolean result = FALSE;
    
    if (SIM_APPL_2G == SimAppType)
        return result;
    
    switch (efile_id)
    {
        case APDUFILEID_EF_LP     :     
        case APDUFILEID_EF_IMSI   :   
        case APDUFILEID_EF_KC     :     
        case APDUFILEID_EF_PLMNSEL: 
        case APDUFILEID_EF_HPLMN  :   
        case APDUFILEID_EF_ACMMAX :   
        case APDUFILEID_EF_SST    :   
        case APDUFILEID_EF_ACM    :   
        case APDUFILEID_EF_GID1   :   
        case APDUFILEID_EF_GID2   :   
        case APDUFILEID_EF_PUCT   :   
        case APDUFILEID_EF_CBMI   :   
        case APDUFILEID_EF_SPN    :   
        case APDUFILEID_EF_CBMID  :   
        case APDUFILEID_EF_BCCH   :   
        case APDUFILEID_EF_ACC    :   
        case APDUFILEID_EF_FPLMN  :   
        case APDUFILEID_EF_LOCI   :   
        case APDUFILEID_EF_AD     :     
        case APDUFILEID_EF_PHASE  :   
        case APDUFILEID_EF_VGCS   :   
        case APDUFILEID_EF_VGCSS  :   
        case APDUFILEID_EF_VBS    :   
        case APDUFILEID_EF_VBSS   :   
        case APDUFILEID_EF_EMLPP  :   
        case APDUFILEID_EF_AAEM   :   
        case APDUFILEID_EF_ECC    :   
        case APDUFILEID_EF_CBMIR  :   
        case APDUFILEID_EF_DCK    :   
        case APDUFILEID_EF_CNL    :   
        case APDUFILEID_EF_NIA    :   
        case APDUFILEID_EF_SUME   :   
        case APDUFILEID_EF_PLMN_WACT : 
        case APDUFILEID_EF_OPLMN_WACT:
        case APDUFILEID_EF_HPLMN_WACT:
        case APDUFILEID_EF_CPBCCH :   
        case APDUFILEID_EF_INV_SCAN  : 
        case APDUFILEID_EF_RPLMN_ACT : 
        case APDUFILEID_EF_PNN    :   
        case APDUFILEID_EF_OPL    :   
        case APDUFILEID_EF_MBDN   :   
        case APDUFILEID_EF_EXT6   :   
        case APDUFILEID_EF_MBI    :   
        case APDUFILEID_EF_MWIS   :   
        case APDUFILEID_EF_CFIS   :   
        case APDUFILEID_EF_EXT7   :   
        case APDUFILEID_EF_SPDI   :   
        case APDUFILEID_EF_MMSN   :   
        case APDUFILEID_EF_EXT8   :   
        case APDUFILEID_EF_MMSICP :   
        case APDUFILEID_EF_MMSUP  :   
            
            result = TRUE;
            break;
        
        default:
            result = FALSE;
            break;
    }   

    return  result;
}


Boolean SimIODetermineResponseError(UInt8 dwSW1, UInt8 dwSW2)
{
    Boolean hr = FALSE;
    
    switch (dwSW1)
    {
        case 0x90:
        case 0x91:
        case 0x9F:
            // This is good
            hr = TRUE;
            break;
        
        case 0x92:
            if ((dwSW2 & 0xf0) == 0x00)
            {
                // The command completed OK, but the SIM had to retry the command internally
                // a few times, but what do I care if the SIM is whining?
                hr = TRUE;
            }
            else
            {
                // Memory problem
                hr = FALSE;
            }
            break;
        
        case 0x94:
            // SW2 = 0x00, 0x02, or 0x08
            // These all means that there was a problem with the file (e.g. invalid address, or
            // they specified the type of file incorrectly)
            hr = FALSE;
            break;
        
        case 0x98:
            // Security and/or authentication problems
            hr = FALSE;
            break;
        
        case 0x6F:
            // Internal SIM error, no further information given
            hr = FALSE;
            break;
        
        case 0x67:
        case 0x6B:
        case 0x6D:
        case 0x6E:
            // I really shouldn't be getting these -- it means that one of the parameters I sent
            // down to the SIM was invalid, but I already error checked for those -- so I guess
            // it's best to assert and let a tester yell at me.  In other words, FALL THROUGH!
        default:
            // I don't recognize this command!
            hr = FALSE;
            break;
    }

    return hr;
}


void ParseSimBoolData(KRIL_CmdList_t *pdata, Kril_CAPI2Info_t *capi2_rsp)
{
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
    Boolean* rsp = (Boolean*)capi2_rsp->dataBuf;
#else
    SIM_BOOLEAN_DATA_t *rsp = (SIM_BOOLEAN_DATA_t*)capi2_rsp->dataBuf;
#endif
    KrilFacLock_t *lock_status;
    
    if (!rsp)
    {
        KRIL_DEBUG(DBG_ERROR,"capi2_rsp->dataBuf is NULL, Error!!\n");
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }
    
    pdata->bcm_ril_rsp = kmalloc(sizeof(KrilFacLock_t), GFP_KERNEL);
    if (!pdata->bcm_ril_rsp)
    {
        KRIL_DEBUG(DBG_ERROR,"Allocate bcm_ril_rsp memory failed!!\n");
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }

    lock_status = pdata->bcm_ril_rsp;
    memset(lock_status, 0, sizeof(KrilFacLock_t));
    pdata->rsp_len = sizeof(KrilFacLock_t);
    
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
    if (capi2_rsp->result != RESULT_OK)
    {
        KRIL_DEBUG(DBG_ERROR,"Get Lock status failed!! result:%d\n", capi2_rsp->result);
#else
    if (rsp->result != SIMACCESS_SUCCESS)
    {
        KRIL_DEBUG(DBG_ERROR,"Get Lock status failed!! result:%d\n", rsp->result);
#endif
        lock_status->result = RIL_E_GENERIC_FAILURE;
        lock_status->lock = 0;
    }
    else
    {
        lock_status->result = RIL_E_SUCCESS;
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
        lock_status->lock = (*rsp ? 1 : 0);
        KRIL_DEBUG(DBG_INFO,"Get Lock status:%d\n", *rsp);
#else
        lock_status->lock = (rsp->value ? 1 : 0);
        KRIL_DEBUG(DBG_INFO,"Get Lock status:%d\n", rsp->value);
#endif
    }     
    
    pdata->handler_state = BCM_FinishCAPI2Cmd;
}


void ParseGetCallBarringData(KRIL_CmdList_t *pdata, Kril_CAPI2Info_t *capi2_rsp)
{   
    if (NULL == capi2_rsp->dataBuf)
    {
        KRIL_DEBUG(DBG_ERROR,"capi2_rsp->dataBuf is NULL, Error!!\n");
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
    }
    else
    {
        if (MSG_SS_CALL_BARRING_STATUS_RSP == capi2_rsp->msgType)
        {
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilFacLock_t), GFP_KERNEL);
            if (NULL == pdata->bcm_ril_rsp)
            {
                KRIL_DEBUG(DBG_ERROR,"Allocate bcm_ril_rsp memory failed!!\n");
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                CallBarringStatus_t* rsp = (CallBarringStatus_t*) capi2_rsp->dataBuf;
                KrilFacLock_t *lock_status;
                lock_status = pdata->bcm_ril_rsp;
                memset(lock_status, 0, sizeof(KrilFacLock_t));
                pdata->rsp_len = sizeof(KrilFacLock_t);
                KRIL_DEBUG(DBG_INFO,"netCause:%d call_barring_type:%d class_size:%d\n", rsp->netCause, rsp->call_barring_type, rsp->class_size);
                if (GSMCAUSE_SUCCESS == rsp->netCause)
                {
                    int i;
                    lock_status->lock = 0;
                    for (i = 0 ; i < rsp->class_size ; i++)
                    {
                        KRIL_DEBUG(DBG_INFO,"activate[%d]:%d ss_class:%d lock:%d\n", i, rsp->ss_activation_class_info[i].activated, rsp->ss_activation_class_info[i].ss_class, lock_status->lock);
                        if (TRUE == rsp->ss_activation_class_info[i].activated)
                            lock_status->lock |= SvcClassToATClass(rsp->ss_activation_class_info[i].ss_class);
                    }
                    lock_status->result = RIL_E_SUCCESS;
                    pdata->handler_state = BCM_FinishCAPI2Cmd;
                }
                else
                {
                    KRIL_DEBUG(DBG_ERROR,"Get Lock status failed!! netCause:%d\n", rsp->netCause);
                    lock_status->result = RIL_E_GENERIC_FAILURE;
                    lock_status->lock = 0;
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                 }
            }
        }
        else
        {
            KRIL_DEBUG(DBG_ERROR,"Receive error MsgType:0x%x...!\n", capi2_rsp->msgType);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
        }
    }
}


int ParseSetCallBarringData(KRIL_CmdList_t *pdata, Kril_CAPI2Info_t *capi2_rsp)
{   
    if (capi2_rsp->msgType == MSG_SS_CALL_BARRING_RSP)
    {
        NetworkCause_t* presult = (NetworkCause_t*) capi2_rsp->dataBuf;
        KRIL_DEBUG(DBG_INFO,"netCause:%d\n", *presult);
        if(*presult != GSMCAUSE_SUCCESS)
        {
            return FALSE;
        }
    }
    else if(capi2_rsp->msgType == MSG_SS_CALL_BARRING_STATUS_RSP)
    {
        CallBarringStatus_t* presult = (CallBarringStatus_t*) capi2_rsp->dataBuf;
        KRIL_DEBUG(DBG_INFO,"netCause:%d call_barring_type:%d class_size:%d\n", presult->netCause, presult->call_barring_type, presult->class_size);

        if(GSMCAUSE_SUCCESS == presult->netCause)
        {
            int i;
            for(i = 0 ; i < presult->class_size ; i++)
            {
                KRIL_DEBUG(DBG_INFO,"activate[%d]:%d ss_class:%d\n", i, presult->ss_activation_class_info[i].activated, presult->ss_activation_class_info[i].ss_class);
            }
        }
        else
        {
            KRIL_DEBUG(DBG_ERROR,"Set Call Barring failed!! netCause:%d\n", presult->netCause);
            return FALSE;
         }
    }
    else
    {
        KRIL_DEBUG(DBG_ERROR,"Receive error MsgType:0x%x...!\n", capi2_rsp->msgType);
        return FALSE;
    }
    return TRUE;
}


int ParseSimAccessResult(KRIL_CmdList_t *pdata, Kril_CAPI2Info_t *capi2_rsp)
{
    SIMAccess_t simAccessType;
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
    SIMAccess_t* rsp = (SIMAccess_t*)capi2_rsp->dataBuf;
#else
    SIM_ACCESS_RESULT_t *rsp = (SIM_ACCESS_RESULT_t*)capi2_rsp->dataBuf;
#endif

    KrilSimPinResult_t *SimPinResult;
    
    if (!rsp)
    {
        KRIL_DEBUG(DBG_ERROR,"capi2_rsp->dataBuf is NULL, Error!!\n");
        return 0;
    }

    pdata->bcm_ril_rsp = kmalloc(sizeof(KrilSimPinResult_t), GFP_KERNEL);
    if (!pdata->bcm_ril_rsp)
    {
        KRIL_DEBUG(DBG_ERROR,"Allocate bcm_ril_rsp memory failed!!\n");
        return 0;
    }

    SimPinResult = pdata->bcm_ril_rsp;
    memset(SimPinResult, 0, sizeof(KrilSimPinResult_t));
    pdata->rsp_len = sizeof(KrilSimPinResult_t);
        
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
    simAccessType = *rsp;
#else
    simAccessType = rsp->result;
#endif

    KRIL_DEBUG(DBG_INFO,"SIM_ACCESS_RESULT_t:%d\n", simAccessType);
    switch (simAccessType)
    {
        case SIMACCESS_SUCCESS:
            SimPinResult->result = RIL_E_SUCCESS;
           break;
        
        case SIMACCESS_INCORRECT_CHV:
        case SIMACCESS_BLOCKED_CHV:
            SimPinResult->result = RIL_E_PASSWORD_INCORRECT;
            break;
            
        default:
            SimPinResult->result = RIL_E_GENERIC_FAILURE;
            break;
    }
    return 1;
}


int ParseRemainingPinAttempt(KRIL_CmdList_t *pdata, Kril_CAPI2Info_t *capi2_rsp)
{
    PIN_ATTEMPT_RESULT_t *rsp = (PIN_ATTEMPT_RESULT_t*)capi2_rsp->dataBuf;
    KrilSimPinResult_t *SimPinResult = NULL;

    if (!rsp)
    {
        KRIL_DEBUG(DBG_ERROR,"capi2_rsp->dataBuf is NULL, Error!!\n");
        return 0;
    }

    if(rsp->result != SIMACCESS_SUCCESS)
    {
        KRIL_DEBUG(DBG_ERROR,"Get remaining PIN attempt failed!! result:%d\n", rsp->result);
        return 0;
    }

    if (!pdata->bcm_ril_rsp)
    {
        KRIL_DEBUG(DBG_ERROR,"pdata->bcm_ril_rsp is NULL, Error!!\n");
        return 0;
    }
    
    SimPinResult = (KrilSimPinResult_t*)pdata->bcm_ril_rsp;
        
    switch(pdata->ril_cmd->CmdID)
    {
        case RIL_REQUEST_ENTER_SIM_PIN:
            SimPinResult->remain_attempt = rsp->pin1_attempt_left;
            break;
        
        case RIL_REQUEST_ENTER_SIM_PIN2:
            SimPinResult->remain_attempt = rsp->pin2_attempt_left;
            break;
        
        case RIL_REQUEST_ENTER_SIM_PUK:
            SimPinResult->remain_attempt = rsp->puk1_attempt_left;
            break;
        
        case RIL_REQUEST_ENTER_SIM_PUK2:
            SimPinResult->remain_attempt = rsp->puk2_attempt_left;
            break;
        
        case RIL_REQUEST_CHANGE_SIM_PIN:
            SimPinResult->remain_attempt = rsp->pin1_attempt_left;
            break;
            
        case RIL_REQUEST_CHANGE_SIM_PIN2:
            SimPinResult->remain_attempt = rsp->pin2_attempt_left;
            break;
            
        case RIL_REQUEST_SET_FACILITY_LOCK:
        {
            KrilSetFacLock_t *cmd_data = (KrilSetFacLock_t*)pdata->ril_cmd->data;
            
            switch (cmd_data->fac_id)
            {
                case FAC_SC:
                    SimPinResult->remain_attempt = rsp->pin1_attempt_left;
                    break;
                
                case FAC_FD:
                    SimPinResult->remain_attempt = rsp->pin2_attempt_left;
                    break;
                
                default:
                    KRIL_DEBUG(DBG_ERROR,"Facility ID:%d Not Supported Error!!\n", cmd_data->fac_id);
                    return 0;
            }
            break;
        }
            
        default:
            KRIL_DEBUG(DBG_ERROR,"CmdID:%lu Not Supported Error!!\n", pdata->ril_cmd->CmdID);
            return 0;
    }
    KRIL_DEBUG(DBG_INFO,"CmdID:%lu remain_attempt:%d\n", pdata->ril_cmd->CmdID, SimPinResult->remain_attempt);
    
    return 1;
}


void ParseSimPinResult(KRIL_CmdList_t *pdata, Kril_CAPI2Info_t *capi2_rsp)
{
	SIMAccess_t result;					
	SIM_PIN_Status_t pin_status;		
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
    SIM_PIN_Status_t* rsp = (SIM_PIN_Status_t*)capi2_rsp->dataBuf;
#else
    SIM_PIN_STATUS_RESULT_t *rsp = (SIM_PIN_STATUS_RESULT_t*)capi2_rsp->dataBuf;
#endif // CONFIG_BRCM_FUSE_RIL_CIB

    KrilSimStatusResult_t *p_sim_status;
    
    if (!rsp)
    {
        KRIL_DEBUG(DBG_ERROR,"capi2_rsp->dataBuf is NULL, Error!!\n");
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }
 
    pdata->bcm_ril_rsp = kmalloc(sizeof(KrilSimStatusResult_t), GFP_KERNEL);
    if (!pdata->bcm_ril_rsp)
    {
        KRIL_DEBUG(DBG_ERROR,"Allocate bcm_ril_rsp memory failed!!\n");
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }

#ifdef CONFIG_BRCM_FUSE_RIL_CIB
	result = ((capi2_rsp->result == RESULT_OK) ? SIMACCESS_SUCCESS : SIMACCESS_INVALID);					
	pin_status = *rsp;		
#else
	result = rsp->result;					
	pin_status = rsp->pin_status;		
#endif // CONFIG_BRCM_FUSE_RIL_CIB

    p_sim_status = (KrilSimStatusResult_t *)pdata->bcm_ril_rsp;
    pdata->rsp_len = sizeof(KrilSimStatusResult_t);
     
    if ( result != SIMACCESS_SUCCESS )
    {
        KRIL_DEBUG(DBG_ERROR,"Get PIN status failed!! result:%d\n", result);
        p_sim_status->pin_status = SIM_NOT_READY;
    }
    else
    {
        SetSimPinStatusHandle(pin_status);
        
        switch (pin_status)
        {
            case PIN_READY_STATUS:
                p_sim_status->pin_status = SIM_READY;
                break;
            
            case SIM_PIN_STATUS:
                p_sim_status->pin_status = SIM_PIN;
                break;
                
            case SIM_PIN2_STATUS:
                p_sim_status->pin_status = SIM_PIN2;
                break;
                
            case PH_SIM_PIN_STATUS:
                p_sim_status->pin_status = SIM_SIM;
                break;
                
            case PH_NET_PIN_STATUS:
                p_sim_status->pin_status = SIM_NETWORK;
                break;
                
            case PH_NETSUB_PIN_STATUS:
                p_sim_status->pin_status = SIM_NETWORK_SUBSET;
                break;
                
            case PH_SP_PIN_STATUS:
                p_sim_status->pin_status = SIM_SERVICE_PROVIDER;
                break;
                
            case PH_CORP_PIN_STATUS:
                p_sim_status->pin_status = SIM_CORPORATE;
                break;
            
            case SIM_PUK_STATUS:
                p_sim_status->pin_status = SIM_PUK;
                break;
                
            case SIM_PUK2_STATUS:
                p_sim_status->pin_status = SIM_PUK2;
                break;
                
            case PH_SIM_PUK_STATUS:
                p_sim_status->pin_status = SIM_SIM_PUK;
                break;
            
            case PH_NET_PUK_STATUS:
                p_sim_status->pin_status = SIM_NETWORK_PUK;
                break;
                
            case PH_NETSUB_PUK_STATUS:
                p_sim_status->pin_status = SIM_NETWORK_SUBSET_PUK;
                break;
                
            case PH_SP_PUK_STATUS:
                p_sim_status->pin_status = SIM_SERVICE_PROVIDER_PUK;
                break;
                
            case PH_CORP_PUK_STATUS:
                p_sim_status->pin_status = SIM_CORPORATE_PUK;
                break;
                
            case SIM_PUK_BLOCK_STATUS:
                p_sim_status->pin_status = SIM_PUK_BLOCK;
                break;
                
            case SIM_PUK2_BLOCK_STATUS:
                p_sim_status->pin_status = SIM_PUK2_BLOCK;
                break;
            
            case SIM_ABSENT_STATUS:
                p_sim_status->pin_status = SIM_ABSENT;
                break;
                
            case NO_SIM_PIN_STATUS:
            case SIM_ERROR_STATUS:
                p_sim_status->pin_status = SIM_NOT_READY;
                break;
        }
    }

    switch (KRIL_GetSimAppType())
    {
        case SIM_APPL_2G:
           p_sim_status->app_type = RIL_APPTYPE_SIM;
           break;

        case SIM_APPL_3G:
           p_sim_status->app_type = RIL_APPTYPE_USIM;
           break;

        default:
           p_sim_status->app_type = RIL_APPTYPE_UNKNOWN;
           break;
    }

    pdata->handler_state = BCM_FinishCAPI2Cmd;    
}


#ifdef CONFIG_BRCM_FUSE_RIL_CIB
Boolean ParseSimServiceStatusResult(Kril_CAPI2Info_t* capi2_rsp, 
                                    SIMServiceStatus_t* localPBKStatus)
{
    SIMServiceStatus_t* rsp = (SIMServiceStatus_t*)capi2_rsp->dataBuf;
    
    if (!rsp)
    {
        KRIL_DEBUG(DBG_ERROR,"capi2_rsp->dataBuf is NULL, Error!!\n");
        return FALSE;
    }
    
    if (capi2_rsp->result != RESULT_OK)
    {
        KRIL_DEBUG(DBG_ERROR,"CAPI2_SIM_GetServiceStatus result failed!! result:%d\n", capi2_rsp->result);
        return FALSE;
    }

    *localPBKStatus = *rsp;
    KRIL_DEBUG(DBG_INFO,"Local PHONE BOOK Status:%s\n",
        SIMSERVICESTATUS_ACTIVATED == *localPBKStatus ? "ACTIVATED": "not ACTIVATED or not ALLOCATED");
    return TRUE;
}
#else
Boolean ParseSimServiceStatusResult(Kril_CAPI2Info_t   *capi2_rsp, 
                                    SIMServiceStatus_t *localPBKStatus)
{
    SIM_SERVICE_STATUS_RESULT_t *rsp = (SIM_SERVICE_STATUS_RESULT_t*)capi2_rsp->dataBuf;
    
    if (!rsp)
    {
        KRIL_DEBUG(DBG_ERROR,"capi2_rsp->dataBuf is NULL, Error!!\n");
        return FALSE;
    }
    
    if (rsp->result != SIMACCESS_SUCCESS)
    {
        KRIL_DEBUG(DBG_ERROR,"CAPI2_SIM_GetServiceStatus result failed!! result:%d\n", rsp->result);
        return FALSE;
    }

    *localPBKStatus = rsp->status;
    KRIL_DEBUG(DBG_INFO,"Local PONE BOOK Status:%s\n",
    SIMSERVICESTATUS_ACTIVATED == *localPBKStatus ? "ACTIVATED": "not ACTIVATED or not ALLOCATED");
    return TRUE;
}
#endif // CONFIG_BRCM_FUSE_RIL_CIB


#ifdef CONVERT_USIM_SELECT_RESPONSE_FORMAT_TO_SIM
// Refer to TS 51.011 section 9.2.1
void ConvertUSIMSelectRspformatToSIM(SIM_RESTRICTED_ACCESS_DATA_t *rsp, 
                                     int fileid,
                                     UInt8 *data, 
                                     UInt16 *data_len
                                     )
{
    UInt8 *pdata = rsp->data;
    UInt16 pdata_len = rsp->data_len;
    UInt16 i;
    
    // Fill Byte 1-2: RFU
    data[0] = 0;
    data[1] = 0;
    
    // Fill Byte 3-4: File Size
    for (i = 0 ; i < (pdata_len-1) ; i++)
    {
        if (0x80 == pdata[i] && 0x02 == pdata[i+1])
        {
            data[2] = pdata[i+2];
            data[3] = pdata[i+3];
            break;
        }
    }
    
    if (i == (pdata_len-1))
    {
        KRIL_DEBUG(DBG_ERROR,"Get File Size failed!!\n");
        data[2] = 0x00;
        data[3] = 0x00;
    }
    
    // Fill Byte 5-6: File ID
#if 0
    for (i = 0 ; i < (pdata_len-1) ; i++)
    {
        if (0x83 == pdata[i] && 0x02 == pdata[i+1])
        {
            data[4] = pdata[i+2];
            data[5] = pdata[i+3];
            break;
        } 
    }
    
    if (i == (pdata_len-1))
    {
        KRIL_DEBUG(DBG_ERROR,"Get File ID failed!!\n");
        data[4] = 0x00;
        data[5] = 0x00;
    }
#endif

    data[4] = (fileid>>8)&0xFF;
    data[5] = fileid&0xFF;
    
    // Fill Byte 7: Type of file
    data[6] = 0x04;
    
    // Fill Byte 8:
    data[7] = 0x00;
    
    // Fill Byte 9-11: Access conditions
    if (APDUFILEID_EF_ADN == fileid || 
        APDUFILEID_EF_FDN == fileid ||
        APDUFILEID_EF_MSISDN == fileid)
    {
        data[8]  = 0x11;
        data[9]  = 0x00;
        data[10] = 0x22;
    }
    else if (APDUFILEID_EF_SMS == fileid)
    {
        data[8]  = 0x11;
        data[9]  = 0x00;
        data[10] = 0x55;
    }
    else if ((APDUFILEID_EF_IMG == fileid) || (APDUFILEID_EF_PBR == fileid))
    {
        data[8]  = 0x14;
        data[9]  = 0x00;
        data[10] = 0x44;
    }
    else if (APDUFILEID_EF_CFIS == fileid)
    {
        data[8]  = 0x11;
        data[9]  = 0x00;
        data[10] = 0x44;
    }
    else
    {
        /* EF_EMAIL, EF_ADN are of the same access conditions. However, their IDs are not fixed in USIM (actual ID is indicated in EF_PBR)
         * Temp workaround: Assume read/update are allowed in default case. If it is not so, we hope CP can catch the error if some wrong command is issued.
         * The current Android framework code does not access this field.
         */
        data[8]  = 0x11;
        data[9]  = 0x00;
        data[10] = 0x22;
    }
 
    // Fill Byte 12: File status
    data[11] = 0x01;
    
    // Fill Byte 13: Length of following data
    data[12] = 0x02;


    /* The file descriptor byte can be decoded from 'File Descriptor' (TLV = 0x82 0x02|0x05 ...) */
    for (i = 0 ; i < (pdata_len-1) ; i++)
    {
        if (0x82 == pdata[i] && (0x02 == pdata[i+1] || 0x05 == pdata[i+1]))
        {
            /*
              [ETSI 102 221 (V9.1.0) Table 11.5]
              -----001 Transparent structure
              -----010 Linear fixed structure
              -----110 Cyclic structure

              [3GPP 51.011 9.3 Definitions and Coding]
              Structure of file
              '00' : transparent;
              '01' : linear fixed;
              '03' : cyclic.
            */

            // Fill Byte 14: Structure of EF
            data[13] = (pdata[i+2] & 0x07) >> 1;

            /* '0x82' '0x05' ... are for linear fixed and cyclic files
             * ETSI 102 201 11.1.1.4.3 File Descriptor
             * "Record length", and "Number of records" are conditional fields.
             * C: These bytes are mandatory for linear fixed and cyclic files, otherwise they are not applicable. 
             */
            
            // Fill Byte 15: Length of a record
            data[14] = (0x05 == pdata[i+1])? pdata[i+5]: 0x00;

            break;
        }
    }

    if (i == (pdata_len-1))
    {
        KRIL_DEBUG(DBG_ERROR,"Get Length of a record failed!!\n");
        data[13] = 0x00;
        data[14] = 0x00;
    }
    
    // Fill the length once (constant 15)
    *data_len = 15;
}
#endif // CONVERT_USIM_SELECT_RESPONSE_FORMAT_TO_SIM


void ParseSimRestrictedAccessData(KRIL_CmdList_t *pdata, Kril_CAPI2Info_t *capi2_rsp)
{
    SIM_RESTRICTED_ACCESS_DATA_t *rsp = (SIM_RESTRICTED_ACCESS_DATA_t*)capi2_rsp->dataBuf;
    KrilSimIOCmd_t *cmd_data = (KrilSimIOCmd_t*)pdata->ril_cmd->data;
    KrilSimIOResponse_t *simioresult;
    
    simioresult = pdata->bcm_ril_rsp;
    
    if (!rsp)
    {
        KRIL_DEBUG(DBG_ERROR,"capi2_rsp->dataBuf is NULL, Error!!\n");
        simioresult->result = RIL_E_GENERIC_FAILURE;
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }
    
    if (rsp->result != SIMACCESS_SUCCESS)
    {
        KRIL_DEBUG(DBG_ERROR,"SIM IO RSP: SIM access failed!! result:%d\n", rsp->result);
        simioresult->result = RIL_E_GENERIC_FAILURE;
        pdata->handler_state = BCM_ErrorCAPI2Cmd;
        return;
    }
    
    KRIL_DEBUG(DBG_INFO,"SIM IO: command:%d fileid:0x%X SW1:0x%X SW2:0x%X DataLen:%d\n",
        cmd_data->command, cmd_data->fileid, rsp->sw1, rsp->sw2, rsp->data_len);
            
    // For some specificed EF, if response is error, try again.
#ifndef ANDROID_SIMIO_PATH_AVAILABLE
    if (SimIODetermineResponseError(rsp->sw1, rsp->sw2) == FALSE)
    {
        if (simioresult->searchcount < 1 && 
            SimIOEFileUndefFirstUSIMADF(cmd_data->fileid, KRIL_GetSimAppType()))
        {
            KrilSimDedFilePath_t path_info;
            
            simioresult->searchcount++;
            
            // Get Dedicated file path inform
            SimIOGetDedicatedFilePath(APDUFILEID_DF_GSM, SIMSERVICESTATUS_NOT_ALLOCATED1, &path_info);
            
            KRIL_DEBUG(DBG_INFO,"DF ID:0x%X path_len:%d searchcount:%d\n", APDUFILEID_DF_GSM,
                path_info.path_len, simioresult->searchcount);
            
            // Start running CAPI2_SIM_SendRestrictedAccessReq
            SimIOSubmitRestrictedAccess(cmd_data, cmd_data->fileid, APDUFILEID_DF_GSM, &path_info);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            return;               
        }
    }
#endif //ANDROID_SIMIO_PATH_AVAILABLE
    
    if (rsp->data_len > 0)
    {
        //print the SIM data
        //RawDataPrintfun(rsp->data, rsp->data_len, "SIM IO RSP");

        if (rsp->data_len > 256)
        {
            // If this issue happen, modify the buffer length of 
            // simResponse[] in KrilSimIOResponse_t
            KRIL_DEBUG(DBG_ERROR,"SIMIO RSP: SIM response length is too long:%d\n", rsp->data_len);
            simioresult->result = RIL_E_GENERIC_FAILURE;
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            return;
        }
        
        // Get image instance file identifier from the record data of EF IMG
        if ((APDUCmd_t)cmd_data->command == APDUCMD_READ_RECORD && 
            (APDUFileID_t)cmd_data->fileid == APDUFILEID_EF_IMG)
        {
            SimIOGetImageInstfileID(rsp->data);
        }
        
#ifdef CONVERT_USIM_SELECT_RESPONSE_FORMAT_TO_SIM
        // Convert USIM SELECT response format to SIM format.
       if (SIM_APPL_3G == KRIL_GetSimAppType() &&
           APDUCMD_GET_RESPONSE == cmd_data->command) 
        {
            UInt8 data[256];
            UInt16 data_len = 0;
            
            ConvertUSIMSelectRspformatToSIM(rsp, cmd_data->fileid, data, &data_len);
            //RawDataPrintfun(data, data_len, "CONVERT SIMIO");
            HexDataToHexStr(simioresult->simResponse, data, data_len);
        }
        else
#endif // CONVERT_USIM_SELECT_RESPONSE_FORMAT_TO_SIM
        {
            HexDataToHexStr(simioresult->simResponse, rsp->data, rsp->data_len);
        }

    }        
    
    simioresult->sw1 = rsp->sw1;
    simioresult->sw2 = rsp->sw2;    
    simioresult->result = RIL_E_SUCCESS;
    pdata->handler_state = BCM_FinishCAPI2Cmd;
}


void SetSimPinStatusHandle(SIM_PIN_Status_t SimPinState)
{
    KRIL_DEBUG(DBG_INFO,"SIM PIN status:%d\n", SimPinState);
    sSimPinstatus = SimPinState;
}


SIM_PIN_Status_t GetSimPinStatusHandle(void)
{
    KRIL_DEBUG(DBG_INFO,"SIM PIN status:%d\n", sSimPinstatus);
    return sSimPinstatus;
}


void SetSIMData(SIMLOCK_SIM_DATA_t *sim_data)
{
    if (NULL == sSimdata)
    {
        sSimdata = kmalloc(sizeof(SIMLOCK_SIM_DATA_t), GFP_KERNEL);
        if (!sSimdata)
        {
            KRIL_DEBUG(DBG_ERROR,"Allocate memory for sSimdata Failed!!\n");
            return;
        }
    }
    
    memcpy(sSimdata, sim_data, sizeof(SIMLOCK_SIM_DATA_t));  
}


SIMLOCK_SIM_DATA_t* GetSIMData(void)
{
    if (!sSimdata)
    {
        KRIL_DEBUG(DBG_INFO,"SIM data is NULL\n");
        return NULL;
    }
    return sSimdata;
}


int SetFacilityLockStateHandler(KRIL_CmdList_t *pdata, SIMLOCK_STATE_t* simlock_state)
{
    SIMLockType_t simlock_type;
    SIMLock_Status_t simlock_status;
    KrilSimPinResult_t *SimPinResult;
    KrilSetFacLock_t *cmd_data = (KrilSetFacLock_t*)pdata->ril_cmd->data;

    pdata->bcm_ril_rsp = kmalloc(sizeof(KrilSimPinResult_t), GFP_KERNEL);
    if (!pdata->bcm_ril_rsp)
    {
        KRIL_DEBUG(DBG_ERROR,"Allocate bcm_ril_rsp memory failed!!\n");
        return 0;
    }
    
    SimPinResult = pdata->bcm_ril_rsp;
    memset(SimPinResult, 0, sizeof(KrilSimPinResult_t));
    pdata->rsp_len = sizeof(KrilSimPinResult_t); 
        
    switch (cmd_data->fac_id)
    {
        case FAC_PS:
            simlock_type = SIMLOCK_PHONE_LOCK;
            break;
            
        case FAC_PN:
            simlock_type = SIMLOCK_NETWORK_LOCK;
            break;
            
        case FAC_PU:
            simlock_type = SIMLOCK_NET_SUBSET_LOCK;
            break;
            
        case FAC_PP:
            simlock_type = SIMLOCK_PROVIDER_LOCK;
            break;
            
        case FAC_PC:
            simlock_type = SIMLOCK_CORP_LOCK;
            break;

        default:
            KRIL_DEBUG(DBG_ERROR,"Facility ID:%d not support Error!!\n", cmd_data->fac_id);
            return 0;
    }
    
		if (SIMLOCK_PHONE_LOCK == simlock_type)
		{
			  SIMLOCK_SIM_DATA_t *sim_data;
        
			  // Check if SIM data is available
        sim_data = GetSIMData();
        if (!sim_data)
        {
            return 0;
        }
        
			  simlock_status = SIMLockSetLock(cmd_data->lock, FALSE, SIMLOCK_PHONE_LOCK, 
			  						(UInt8*) cmd_data->password, (UInt8*) sim_data->imsi_string, NULL, NULL); 
		}
		else
		{
			  simlock_status = SIMLockSetLock(cmd_data->lock, FALSE, simlock_type, 
			      (UInt8*) cmd_data->password, NULL, NULL, NULL);
		}
    
    switch (simlock_status)
    {
        case SIMLOCK_SUCCESS:
            SimPinResult->result = RIL_E_SUCCESS;
            break;
        
        case SIMLOCK_WRONG_KEY:
        case SIMLOCK_PERMANENTLY_LOCKED:
            SimPinResult->result = RIL_E_PASSWORD_INCORRECT;
            break;
        
        default:
            SimPinResult->result = RIL_E_GENERIC_FAILURE;
            break;
    }
    
    SIMLockGetSIMLockState(simlock_state);    
    SimPinResult->remain_attempt = (int)SIMLockGetRemainAttempt();
    return 1;
}


int QueryFacilityLockStateHandler(KRIL_CmdList_t *pdata)
{
    SIMLockType_t simlock_type;
    KrilFacLock_t *lock_status;
    KrilGetFacLock_t *cmd_data = (KrilGetFacLock_t*)pdata->ril_cmd->data;
    
    pdata->bcm_ril_rsp = kmalloc(sizeof(KrilFacLock_t), GFP_KERNEL);
    if (!pdata->bcm_ril_rsp)
    {
        KRIL_DEBUG(DBG_ERROR,"Allocate bcm_ril_rsp memory failed!!\n");
        return 0;
    }

    lock_status = pdata->bcm_ril_rsp;
    memset(lock_status, 0, sizeof(KrilFacLock_t));
    pdata->rsp_len = sizeof(KrilFacLock_t);

    switch (cmd_data->fac_id)
    {
        case FAC_PS:
            simlock_type = SIMLOCK_PHONE_LOCK;
            break;
            
        case FAC_PN:
            simlock_type = SIMLOCK_NETWORK_LOCK;
            break;
            
        case FAC_PU:
            simlock_type = SIMLOCK_NET_SUBSET_LOCK;
            break;
            
        case FAC_PP:
            simlock_type = SIMLOCK_PROVIDER_LOCK;
            break;
            
        case FAC_PC:
            simlock_type = SIMLOCK_CORP_LOCK;
            break;

        default:
            KRIL_DEBUG(DBG_ERROR,"Facility ID:%d not support Error!!\n", cmd_data->fac_id);
            return 0;
    }
        
    lock_status->lock = (SIMLockIsLockOn(simlock_type, NULL) == TRUE ? 1 : 0);
    lock_status->result = RIL_E_SUCCESS;
    return 1;    
}


int ChangeFacilityLockstateHandler(KRIL_CmdList_t *pdata, SIMLOCK_STATE_t* simlock_state)
{
    SIMLockType_t simlock_type;
    SIMLock_Status_t simlock_status;
    KrilSimPinResult_t *SimPinResult;
    KrilSimPinNum_t *pin_word = pdata->ril_cmd->data;
    
    pdata->bcm_ril_rsp = kmalloc(sizeof(KrilFacLock_t), GFP_KERNEL);
    if (!pdata->bcm_ril_rsp)
    {
        KRIL_DEBUG(DBG_ERROR,"Allocate bcm_ril_rsp memory failed!!\n");
        return 0;
    }

    SimPinResult = pdata->bcm_ril_rsp;
    memset(SimPinResult, 0, sizeof(KrilSimPinResult_t));
    pdata->rsp_len = sizeof(KrilSimPinResult_t);
            
    switch (GetSimPinStatusHandle())
    {
        case PH_SIM_PIN_STATUS:
            simlock_type = SIMLOCK_PHONE_LOCK;
            break;
            
        case PH_NET_PIN_STATUS:
            simlock_type = SIMLOCK_NETWORK_LOCK;
            break;
            
        case PH_NETSUB_PIN_STATUS:
            simlock_type = SIMLOCK_NET_SUBSET_LOCK;
            break;
            
        case PH_SP_PIN_STATUS:
            simlock_type = SIMLOCK_PROVIDER_LOCK;
            break;
            
        case PH_CORP_PIN_STATUS:
            simlock_type = SIMLOCK_CORP_LOCK;
            break;

        default:
            KRIL_DEBUG(DBG_ERROR,"Error SIM PIN status:%d\n", GetSimPinStatusHandle());
            return 0;
    }
    
		if (SIMLOCK_PHONE_LOCK == simlock_type)
		{
			  /* PH-SIM lock is a bit different from the other locks:
			   * after the password is verified to be correct, the PH-SIM lock
			   * is unlocked, but the PH-SIM lock setting is still on and now
			   * it is locked to the new SIM card. So we can not use SIMLockUnlockSIM
			   * function to unlock PH-SIM lock. We need to first obtain the IMSI
			   * of the new SIM card before trying to unlock the PH-SIM lock.
			   */
			  SIMLOCK_SIM_DATA_t *sim_data;
        
			  // Check if SIM data is available
        sim_data = GetSIMData();
        if (!sim_data)
        {
            return 0;
        }
        
			  simlock_status = SIMLockSetLock( 1, FALSE, SIMLOCK_PHONE_LOCK, 
			  						(UInt8*) pin_word->password, (UInt8*) sim_data->imsi_string, NULL, NULL); 
		}
		else
		{
			  simlock_status = SIMLockUnlockSIM(simlock_type, (UInt8*)pin_word->password);
		}

    
    switch (simlock_status)
    {
        case SIMLOCK_SUCCESS:
            SimPinResult->result = RIL_E_SUCCESS;
            break;
        
        case SIMLOCK_WRONG_KEY:
        case SIMLOCK_PERMANENTLY_LOCKED:
            SimPinResult->result = RIL_E_PASSWORD_INCORRECT;
            break;
        
        default:
            SimPinResult->result = RIL_E_GENERIC_FAILURE;
            break;
    }
    
    SIMLockGetSIMLockState(simlock_state);    
    SimPinResult->remain_attempt = (int)SIMLockGetRemainAttempt();
    return 1;
}


void KRIL_GetSimStatusHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
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
            CAPI2_SIM_GetPinStatus(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        
        case BCM_RESPCAPI2Cmd:
            ParseSimPinResult(pdata, capi2_rsp);
            break;
        
        default:
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
    }
}


void KRIL_EnterSimPinHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
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
            KrilSimPinNum_t *pin_word = pdata->ril_cmd->data;
            
            KRIL_DEBUG(DBG_INFO,"PIN NUM:%s\n", pin_word->password);
            CAPI2_SIM_SendVerifyChvReq(GetNewTID(), GetClientID(), CHV1, (UInt8*)(pin_word->password));
            pdata->handler_state = BCM_SIM_GetRemainingPinAttempt;
            break;
        }
        
        case BCM_SIM_GetRemainingPinAttempt:

            if (!ParseSimAccessResult(pdata, capi2_rsp))
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            
            CAPI2_SIM_SendRemainingPinAttemptReq(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
            
        case BCM_RESPCAPI2Cmd:
            
            if (!ParseRemainingPinAttempt(pdata, capi2_rsp))
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;

        default:
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
    }
}


void KRIL_EnterSimPukHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
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
            KrilSimPinNum_t *pin_word = pdata->ril_cmd->data;
            
            KRIL_DEBUG(DBG_INFO,"PUK NUM:%s\n", pin_word->password);
            KRIL_DEBUG(DBG_INFO,"New PIN NUM:%s\n", pin_word->newpassword);
            CAPI2_SIM_SendUnblockChvReq(GetNewTID(), GetClientID(), CHV1, (UInt8*)(pin_word->password), (UInt8*)(pin_word->newpassword));
            pdata->handler_state = BCM_SIM_GetRemainingPinAttempt;
            break;
        }

        case BCM_SIM_GetRemainingPinAttempt:

            if (!ParseSimAccessResult(pdata, capi2_rsp))
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            
            CAPI2_SIM_SendRemainingPinAttemptReq(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
            
        case BCM_RESPCAPI2Cmd:
            
            if (!ParseRemainingPinAttempt(pdata, capi2_rsp))
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        
        default:
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
    }
}


void KRIL_EnterNetworkDepersonHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
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
            SIMLOCK_STATE_t simlock_state;
            
            if (!ChangeFacilityLockstateHandler(pdata, &simlock_state))
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }

            CAPI2_SIMLOCK_SetStatus(GetNewTID(), GetClientID(), &simlock_state);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }
        
        case BCM_RESPCAPI2Cmd:
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        
        default:
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;        
    }    
}


void KRIL_ChangeSimPinHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
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
            KrilSimPinNum_t *pin_word = pdata->ril_cmd->data;
            
            KRIL_DEBUG(DBG_INFO,"Old PIN NUM:%s\n", pin_word->password);
            KRIL_DEBUG(DBG_INFO,"New PIN NUM:%s\n", pin_word->newpassword);
            CAPI2_SIM_SendChangeChvReq(GetNewTID(), GetClientID(), (pdata->ril_cmd->CmdID == RIL_REQUEST_CHANGE_SIM_PIN ? CHV1 : CHV2), 
                (UInt8*)(pin_word->password), (UInt8*)(pin_word->newpassword));
            pdata->handler_state = BCM_SIM_GetRemainingPinAttempt;
            break;
        }

        case BCM_SIM_GetRemainingPinAttempt:

            if (!ParseSimAccessResult(pdata, capi2_rsp))
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            
            CAPI2_SIM_SendRemainingPinAttemptReq(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
            
        case BCM_RESPCAPI2Cmd:
            
            if (!ParseRemainingPinAttempt(pdata, capi2_rsp))
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;

        default:
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
    }
}


void KRIL_QueryFacilityLockHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
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
            KrilGetFacLock_t *cmd_data = (KrilGetFacLock_t*)pdata->ril_cmd->data;

            KRIL_DEBUG(DBG_INFO,"Facility ID:%d\n", cmd_data->fac_id);
            switch (cmd_data->fac_id)
            {
                case FAC_SC:
                    CAPI2_SIM_IsPINRequired(GetNewTID(), GetClientID());
                    pdata->handler_state = BCM_RESPCAPI2Cmd;
                    break;
                
                case FAC_FD:
                    CAPI2_SIM_IsOperationRestricted(GetNewTID(), GetClientID());
                    pdata->handler_state = BCM_RESPCAPI2Cmd;
                    break;

                case FAC_AO:
                case FAC_OI:
                case FAC_OX:
                case FAC_AI:
                case FAC_IR:
                case FAC_AB:
                case FAC_AG:
                case FAC_AC:
                    KRIL_DEBUG(DBG_INFO,"fac_id:%d password:%s service:%d\n", cmd_data->fac_id, cmd_data->password, cmd_data->service);
                    CAPI2_SS_QueryCallBarringStatus(GetNewTID(), GetClientID(), ssBarringTypes[cmd_data->fac_id], GetServiceClass(cmd_data->service));
                    pdata->handler_state = BCM_RESPCAPI2Cmd;
                    break;

                case FAC_PS:
                case FAC_PN:
                case FAC_PU:
                case FAC_PP:
                case FAC_PC:
                    if (QueryFacilityLockStateHandler(pdata))
                    {
                        pdata->handler_state = BCM_FinishCAPI2Cmd;
                    }
                    else
                    {
                        pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    }    
                    break;
                    
                default:
                    KRIL_DEBUG(DBG_ERROR,"Facility ID:%d Not Supported Error!!\n", cmd_data->fac_id);
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    break;
            }
            break;
        }
        
        case BCM_RESPCAPI2Cmd:
        {
            KrilGetFacLock_t *cmd_data = (KrilGetFacLock_t*)pdata->ril_cmd->data;

            switch (cmd_data->fac_id)
            {
                case FAC_SC:
                case FAC_FD:
                    ParseSimBoolData(pdata, capi2_rsp);
                    break;

                case FAC_AO:
                case FAC_OI:
                case FAC_OX:
                case FAC_AI:
                case FAC_IR:
                case FAC_AB:
                case FAC_AG:
                case FAC_AC:
                    ParseGetCallBarringData(pdata, capi2_rsp);
                    break;

                default:
                    KRIL_DEBUG(DBG_ERROR,"Facility ID:%d Not Supported Error!!\n", cmd_data->fac_id);
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    return;
            }
            
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }
        
        default:
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
    }
}


void KRIL_SetFacilityLockHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
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
            KrilSetFacLock_t *cmd_data = (KrilSetFacLock_t*)pdata->ril_cmd->data;
            
            KRIL_DEBUG(DBG_INFO,"Facility ID:%d password:%s service:%d\n", cmd_data->fac_id, cmd_data->password, cmd_data->service);
            switch (cmd_data->fac_id)
            {
                case FAC_SC:
                    CAPI2_SIM_SendSetChv1OnOffReq(GetNewTID(), GetClientID(), (UInt8*)cmd_data->password, (Boolean)cmd_data->lock);
                    pdata->handler_state = BCM_SIM_GetRemainingPinAttempt;
                    break;
                
                case FAC_FD:
                    CAPI2_SIM_SendSetOperStateReq(GetNewTID(), GetClientID(), 
                        (cmd_data->lock == 1) ? SIMOPERSTATE_RESTRICTED_OPERATION : SIMOPERSTATE_UNRESTRICTED_OPERATION, 
                        (UInt8*)cmd_data->password);
                    pdata->handler_state = BCM_SIM_GetRemainingPinAttempt;
                    break;

                case FAC_AO:
                case FAC_OI:
                case FAC_OX:
                case FAC_AI:
                case FAC_IR:
                case FAC_AB:
                case FAC_AG:
                case FAC_AC:
                    CAPI2_SS_SendCallBarringReq(GetNewTID(), GetClientID(), (cmd_data->lock == 1) ? SS_MODE_ENABLE : SS_MODE_DISABLE, 
                        ssBarringTypes[cmd_data->fac_id], GetServiceClass(cmd_data->service), (UInt8 *)cmd_data->password);
                    pdata->handler_state = BCM_SS_SendCallBarringReq;
                    break;

                case FAC_PS:
                case FAC_PN:
                case FAC_PU:
                case FAC_PP:
                case FAC_PC:
                {
                    SIMLOCK_STATE_t simlock_state;
                    
                    if (!SetFacilityLockStateHandler(pdata, &simlock_state))
                    {
                        pdata->handler_state = BCM_ErrorCAPI2Cmd;
                        return;
                    }
                    
                    CAPI2_SIMLOCK_SetStatus(GetNewTID(), GetClientID(), &simlock_state);
                    pdata->handler_state = BCM_RESPCAPI2Cmd;
                    break;
                }
                    
                default:
                    KRIL_DEBUG(DBG_ERROR,"Facility ID:%d Not Supported Error!!\n", cmd_data->fac_id);
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    return;
            }
            break;
        }

        case BCM_SIM_GetRemainingPinAttempt:
            if (!ParseSimAccessResult(pdata, capi2_rsp))
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            
            CAPI2_SIM_SendRemainingPinAttemptReq(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;

        case BCM_SS_SendCallBarringReq:
            if (!ParseSetCallBarringData(pdata, capi2_rsp))
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            break;

        case BCM_RESPCAPI2Cmd:
        {
            KrilSetFacLock_t *cmd_data = (KrilSetFacLock_t*)pdata->ril_cmd->data;
            
            if (FAC_SC == cmd_data->fac_id || FAC_FD == cmd_data->fac_id)
            {
                if (!ParseRemainingPinAttempt(pdata, capi2_rsp))
                {
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    return;
                }
            }
            
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }
        
        default:
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
    }
}


#ifdef ANDROID_SIMIO_PATH_AVAILABLE

void KRIL_SimIOHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
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
            KrilSimIOCmd_t *cmd_data = (KrilSimIOCmd_t*)pdata->ril_cmd->data;
            APDUFileID_t efile_id = APDUFILEID_INVALID_FILE;
            APDUFileID_t dfile_id = APDUFILEID_INVALID_FILE;
            KrilSimIOResponse_t *simioresult; 
            
            KRIL_DEBUG(DBG_INFO,"SIM IO: command:%d fileid:0x%X DFid:0x%X p1:%d p2:%d p3:%d pin2:%s\n",
                cmd_data->command, cmd_data->fileid, cmd_data->dfid, cmd_data->p1, cmd_data->p2, cmd_data->p3,
                cmd_data->pin2);            
            RawDataPrintfun((UInt8*)cmd_data->path, cmd_data->pathlen*2, "SIM IO: PATH");

            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilSimIOResponse_t), GFP_KERNEL);
            if (!pdata->bcm_ril_rsp)
            {
                KRIL_DEBUG(DBG_ERROR,"Allocate bcm_ril_rsp memory failed!!\n");
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }

            simioresult = pdata->bcm_ril_rsp;
            memset(simioresult, 0, sizeof(KrilSimIOResponse_t));
            pdata->rsp_len = sizeof(KrilSimIOResponse_t);
            simioresult->command = cmd_data->command;
            simioresult->fileid = cmd_data->fileid;
            simioresult->searchcount = 0;
            
            efile_id = (APDUFileID_t)cmd_data->fileid;     
            dfile_id = (APDUFileID_t)cmd_data->dfid; 
            
            if (cmd_data->data[0] != '\0')
            {
                CAPI2_SIM_SubmitRestrictedAccessReq(GetNewTID(), GetClientID(), USIM_BASIC_CHANNEL_SOCKET_ID,
                    (APDUCmd_t)cmd_data->command, efile_id, dfile_id, (UInt8)cmd_data->p1, (UInt8)cmd_data->p2,
                    (UInt8)cmd_data->p3, cmd_data->pathlen, cmd_data->path, (UInt8*)cmd_data->data);
            }
            else
            {
                CAPI2_SIM_SubmitRestrictedAccessReq(GetNewTID(), GetClientID(), USIM_BASIC_CHANNEL_SOCKET_ID,
                    (APDUCmd_t)cmd_data->command, efile_id, dfile_id, (UInt8)cmd_data->p1, (UInt8)cmd_data->p2,
                    (UInt8)cmd_data->p3, cmd_data->pathlen, cmd_data->path, NULL);
            }                
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            ParseSimRestrictedAccessData(pdata, capi2_rsp);
            break;
        }
        
        default:
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;        
    }
    
}

#else //ANDROID_SIMIO_PATH_AVAILABLE

void KRIL_SimIOHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
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
            KrilSimIOCmd_t *cmd_data = (KrilSimIOCmd_t*)pdata->ril_cmd->data;
            SIM_APPL_TYPE_t simAppType;
            KrilSimDedFilePath_t path_info;
            APDUFileID_t efile_id = APDUFILEID_INVALID_FILE;
            APDUFileID_t dfile_id = APDUFILEID_INVALID_FILE;
            KrilSimIOResponse_t *simioresult;        
            
            KRIL_DEBUG(DBG_INFO,"SIM IO: command:%d fileid:0x%X DFid:0x%X p1:%d p2:%d p3:%d pin2:%s\n",
                cmd_data->command, cmd_data->fileid, cmd_data->dfid, cmd_data->p1, cmd_data->p2, cmd_data->p3,
                cmd_data->pin2);            
            RawDataPrintfun((UInt8*)cmd_data->path, cmd_data->pathlen*2, "SIM IO: PATH");
                        
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilSimIOResponse_t), GFP_KERNEL);
            if (!pdata->bcm_ril_rsp)
            {
                KRIL_DEBUG(DBG_ERROR,"Allocate bcm_ril_rsp memory failed!!\n");
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }

            simioresult = pdata->bcm_ril_rsp;
            memset(simioresult, 0, sizeof(KrilSimIOResponse_t));
            pdata->rsp_len = sizeof(KrilSimIOResponse_t);
            simioresult->command = cmd_data->command;
            simioresult->fileid = cmd_data->fileid;
            simioresult->searchcount = 0;
                    
            simAppType = KRIL_GetSimAppType();
            
            if (SIM_APPL_INVALID == simAppType)
            {
                KRIL_DEBUG(DBG_ERROR,"SIM APP TYPE is Invalid Error!!\n");
                simioresult->result = RIL_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            
            efile_id = (APDUFileID_t)cmd_data->fileid;
            
            if (APDUFILEID_EF_ADN == efile_id && SIM_APPL_3G == simAppType)
            {
                efile_id = APDUFILEID_EF_PBK_ADN;
            }

            if (SIM_APPL_3G == simAppType && !strcmp(cmd_data->path, "3F007F105F3A"))
            {
                /* Special handling for 3G phonebook related EF whose ID is decided by EF_PBR (i.e. not fixed) 
                 * Trust Android and let CP catch error if any 
                 */
                dfile_id = APDUFILEID_DF_PHONEBK;
            }       
            else if (FALSE == SimIOGetDedicatedFileId(efile_id, simAppType, &dfile_id))
            {
                KRIL_DEBUG(DBG_ERROR,"SIM IO: EF ID:0x%X Get DF ID failed!!\n",efile_id);
                simioresult->result = RIL_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }

            if (APDUFILEID_DF_PHONEBK == dfile_id && SIM_APPL_3G == simAppType)
            {
                //Need to check if support the local phonebook
                CAPI2_SIM_GetServiceStatus(GetNewTID(), GetClientID(), SIMSERVICE_LOCAL_PHONEBK);
                pdata->handler_state = BCM_SIM_GetServiceStatus;
                break;
            }
            else
            {
                //Get Dedicated file path inform
                if (FALSE == SimIOGetDedicatedFilePath(dfile_id, SIMSERVICESTATUS_NOT_ALLOCATED1, &path_info))
                {
                    KRIL_DEBUG(DBG_ERROR,"DF ID:0x%X Get DF PATH failed!!\n",dfile_id);
                    simioresult->result = RIL_E_GENERIC_FAILURE;
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    return;
                }
                
                KRIL_DEBUG(DBG_INFO,"DF ID:0x%X path_len:%d\n", dfile_id, path_info.path_len);
                
                //Start running CAPI2_SIM_SendRestrictedAccessReq
                SimIOSubmitRestrictedAccess(cmd_data, efile_id, dfile_id, &path_info);
                pdata->handler_state = BCM_RESPCAPI2Cmd;
            }            
            break;
        }
        
        case BCM_SIM_GetServiceStatus:
        {
            SIMServiceStatus_t localPBKStatus = SIMSERVICESTATUS_NOT_ALLOCATED1;
            KrilSimIOCmd_t *cmd_data = (KrilSimIOCmd_t*)pdata->ril_cmd->data;
            KrilSimDedFilePath_t path_info;
            APDUFileID_t efile_id,dfile_id;
            KrilSimIOResponse_t *simioresult;
            SIM_APPL_TYPE_t simAppType;
            
            simioresult = pdata->bcm_ril_rsp;
            
            if (FALSE == ParseSimServiceStatusResult(capi2_rsp, &localPBKStatus))
            {
                simioresult->result = RIL_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            
            efile_id = (APDUFileID_t)cmd_data->fileid;
            simAppType = KRIL_GetSimAppType();
            
            if (APDUFILEID_EF_ADN == efile_id && SIM_APPL_3G == simAppType)
            {
                efile_id = APDUFILEID_EF_PBK_ADN;
            }
            
            dfile_id = APDUFILEID_DF_PHONEBK;
            
            //Get Dedicated file path inform
            if (FALSE == SimIOGetDedicatedFilePath(dfile_id, localPBKStatus, &path_info))
            {
                KRIL_DEBUG(DBG_ERROR,"DF ID:0x%X Get DF PATH failed!!\n",dfile_id);
                simioresult->result = RIL_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            
            KRIL_DEBUG(DBG_INFO,"DF ID:0x%X path_len:%d\n", dfile_id, path_info.path_len);

            //Start running CAPI2_SIM_SendRestrictedAccessReq
            SimIOSubmitRestrictedAccess(cmd_data, efile_id, dfile_id, &path_info);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }
        
        case BCM_RESPCAPI2Cmd:
        {
            ParseSimRestrictedAccessData(pdata, capi2_rsp);
            break;
        }
        
        default:
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
    }
}
#endif //ANDROID_SIMIO_PATH_AVAILABLE