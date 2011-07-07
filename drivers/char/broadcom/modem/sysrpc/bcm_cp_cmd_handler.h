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

#ifndef _BCM_KRIL_CP_CMD_HANDLER_H
#define _BCM_KRIL_CP_CMD_HANDLER_H

#include "bcm_kril_common.h"
#include "bcm_kril_cmd_handler.h"

#include "mobcom_types.h"
#include "resultcode.h"
#include "common_defs.h"
#include "uelbs_api.h"


#define _DEF(a) a

#ifndef CONFIG_BRCM_FUSE_RIL_CIB
void _DEF(CAPI2_PMU_IsSIMReady)(UInt32 tid, UInt8 clientID);
void _DEF(CAPI2_PMU_ActivateSIM)(UInt32 tid, UInt8 clientID, Int32 volt);
#endif

void _DEF(CAPI2_PMU_DeactivateSIM)(UInt32 tid, UInt8 clientID);
void _DEF(CAPI2_GPIO_Set_High_64Pin)(UInt32 tid, UInt8 clientID, UInt32 gpio_pin);
void _DEF(CAPI2_GPIO_Set_Low_64Pin)(UInt32 tid, UInt8 clientID, UInt32 gpio_pin);
void _DEF(CAPI2_PMU_StartCharging)(UInt32 tid, UInt8 clientID);
void _DEF(CAPI2_PMU_StopCharging)(UInt32 tid, UInt8 clientID);
void _DEF(CAPI2_PMU_ClientPowerDown)(UInt32 tid, UInt8 clientID);
void _DEF(CAPI2_PMU_GetPowerupCause)(UInt32 tid, UInt8 clientID);
void _DEF(CAPI2_GPS_Control)(UInt32 tid, UInt8 clientID, UInt32 u32Cmnd, UInt32 u32Param0, UInt32 u32Param1, UInt32 u32Param2, UInt32 u32Param3, UInt32 u32Param4);
void _DEF(CAPI2_FFS_Control)(UInt32 tid, UInt8 clientID, UInt32 cmd, UInt32 address, UInt32 offset, UInt32 size);
void _DEF(CAPI2_CP2AP_PedestalMode_Control)(UInt32 tid, UInt8 clientID, UInt32 enable);
void _DEF(CAPI2_FLASH_SaveImage)(UInt32 tid, UInt8 clientID, UInt32 flash_addr, UInt32 length, UInt32 shared_mem_addr);
#ifndef CONFIG_BRCM_FUSE_RIL_CIB
void _DEF(CAPI2_FFS_Read)(UInt32 tid, UInt8 clientID, FFS_ReadReq_t *ffsReadReq);
#else
#endif
void _DEF(CAPI2_InterTaskMsgToAP)(UInt32 tid, UInt8 clientID, InterTaskMsg_t *inPtrMsg);
void CAPI2_FLASH_SaveImage_RSP(UInt32 tid, UInt8 clientID, Boolean status);
#ifndef CONFIG_BRCM_FUSE_RIL_CIB
void CAPI2_FFS_Read_RSP(UInt32 tid, UInt8 clientID, FFS_Data_t *ffsReadRsp);
#endif


extern void bcm_cp_cmd_handler_init(void);


/// The GPS hardware abstraction layer function call returns the following operation status
typedef enum
{
    HAL_GPS_PASS = 0, ///< Successful
    HAL_GPS_FAIL,    ///< Generic failure code
} HAL_GPS_Result_en_t;


// The FFS_Control() function call returns the following operation status
typedef enum
{
    FFS_CONTROL_PASS = 0,             ///< Successful
    FFS_CONTROL_INVALID_PARAM,        ///< invalid parameters
    FFS_CONTROL_UNKNOWN_CMD,            ///< unknown command
    FFS_CONTROL_NO_FFS_SUPPORT_ON_AP,   ///< no FFS support on AP
    FFS_CONTROL_FILE_OP_FAIL,         ///< one of the file operation failed
    FFS_CONTROL_IPCAP_PDS_OP_FAIL,      ///< IPCAP_PersistentDataStore operation failed

} FFS_Control_Result_en_t;


/// various commands for FFS_Control()
typedef enum
{
    FFS_CONTROL_COPY_NVS_FILE_TO_SM = 0,    ///< copy the NVS file to shared memory
    FFS_CONTROL_COPY_SM_TO_NVS_FILE,        ///< copy shared memory to the NVS file

} FFS_Control_en_t;


// for FFS Control
typedef struct{
    unsigned long   tid;
    unsigned short  clientID;
    unsigned long   cmd; 
    unsigned long   address;
    unsigned long   offset;
    unsigned long   size;
}KrilFFSControlCmd_t;

#ifndef CONFIG_BRCM_FUSE_RIL_CIB
typedef enum {
    PMU_SIM3P0Volt = 0,
    PMU_SIM2P5Volt,
    PMU_SIM0P0Volt,
    PMU_SIM1P8Volt
} PMU_SIMVolt_t;
#endif

#endif //_BCM_KRIL_CP_CMD_HANDLER_H
