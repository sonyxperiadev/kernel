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

#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/init.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>

#include "bcm_cp_cmd_handler.h"
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
#include "bcm_kril_sysrpc_handler.h"
#endif
#include "bcm_kril_debug.h"
#include "bcm_kril_simlockfun.h"

bool g_bPmuSimInited = FALSE;
static struct regulator * sim_regulator = NULL;
extern struct work_struct ffs_control_wq;

extern UInt32 TIMER_GetValue(void);

extern void SetSIMData(SIMLOCK_SIM_DATA_t *sim_data);

#define MAX_BUF_SIZE 1024
static char buf[MAX_BUF_SIZE];
int RpcLog_DebugPrintf(char* fmt, ...)
{
#ifdef CONFIG_BRCM_UNIFIED_LOGGING
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, MAX_BUF_SIZE, fmt, ap);
    va_end(ap);
    KRIL_DEBUG(DBG_INFO, "TS[%ld]%s\n", TIMER_GetValue(), buf);
#else
    if(IsBasicCapi2LoggingEnable())
    {
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(buf, MAX_BUF_SIZE, fmt, ap);
        va_end(ap);
        pr_info("TS[%ld]%s",TIMER_GetValue(),buf);
    }
#endif
    return 1;
}



void bcm_cp_cmd_handler_init(void)
{
// pmu init done elsewhere for CIB build
#ifndef CONFIG_BRCM_FUSE_RIL_CIB
	pr_info(" inside %s\n",__func__);
	sim_regulator = regulator_get(NULL,"sim_vcc");
	if(!sim_regulator)
		pr_info(" regulator_get() for SIM failed !!!!\n");
#endif
}

void bcm_cp_cmd_handler_exit(void)
{
#ifndef CONFIG_BRCM_FUSE_RIL_CIB
	if(sim_regulator)
	{
		regulator_put(sim_regulator);
	}
#endif
}

#ifndef CONFIG_BRCM_FUSE_RIL_CIB
static bool pmu_is_sim_ready(void)
{
    KRIL_DEBUG(DBG_INFO,"g_bPmuSimInited:%d\n", g_bPmuSimInited);
    return g_bPmuSimInited;
}

static bool pmu_activate_sim(Int32 volt)
{
    bool bRet = FALSE;

    // we pre-define three slection ==>BCM_SIM_VOLT_0,     BCM_SIM_VOLT_1V8,   BCM_SIM_VOLT_3V
    switch((PMU_SIMVolt_t)volt)
    {
        case PMU_SIM3P0Volt:
        {
            //CLRREG8(&pWords, 0xc0);
			if(!regulator_is_enabled(sim_regulator))
				regulator_enable(sim_regulator);
			regulator_set_voltage(sim_regulator,3000000,3000000);
			pr_info("%s:PMU_SIM3P0Volt\n",__func__);
	    	bRet= TRUE;
            g_bPmuSimInited = TRUE;
        }
        break;

        case PMU_SIM2P5Volt:
        {
            //CLRREG8(&pWords, 0xc0);
            //bRet = WritePMUReg(PMU_REG_LCSIMDOCTRL, pWords);
            //bRet = WritePMUReg(PMU_REG_SOPMODCTRL, 0x00);
            KRIL_DEBUG(DBG_INFO,"---> BCMCpCmdHand: CAPI2_PMU_ActivateSIM -> set to 2.5V\n");
			if(!regulator_is_enabled(sim_regulator))
				regulator_enable(sim_regulator);
			regulator_set_voltage(sim_regulator,2500000,2500000);
			bRet= TRUE;
			g_bPmuSimInited = TRUE;
			pr_info("%s:PMU_SIM2P5Volt\n",__func__);
       }
        break;

        case PMU_SIM1P8Volt:
        {
            //SETREG8(&pWords, 0xc0);
			if(!regulator_is_enabled(sim_regulator))
				regulator_enable(sim_regulator);
			regulator_set_voltage(sim_regulator,1800000,1800000);
			bRet= TRUE;
			g_bPmuSimInited = TRUE;
			pr_info("%s:PMU_SIM1P8Volt\n",__func__);
			KRIL_DEBUG(DBG_INFO,"---> BCMCpCmdHand: CAPI2_PMU_ActivateSIM -> set to 1.8V\n");
            
        }
        break;

        case PMU_SIM0P0Volt:
        {
			if(regulator_is_enabled(sim_regulator))
				regulator_disable(sim_regulator);
			pr_info("%s:PMU_SIM0P0Volt\n",__func__);
            KRIL_DEBUG(DBG_INFO,"---> BCMCpCmdHand: CAPI2_PMU_ActivateSIM -> set to 0V\n");
			bRet= TRUE;
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"---> BCMCpCmdHand: CAPI2_PMU_ActivateSIM -> unknown Volt!\n");
            bRet = FALSE;
        }
    }

    if(bRet)
    {
        bRet = TRUE;
    }
    else
    {
        KRIL_DEBUG(DBG_ERROR," ---> BCMCpCmdHand: CAPI2_PMU_ActivateSIM fail!\n");
    }


    return bRet;
}
#endif // CONFIG_BRCM_FUSE_RIL_CIB

#ifndef CONFIG_BRCM_FUSE_RIL_CIB
void CAPI2_CP2AP_PedestalMode_Control(UInt32 tid, UInt8 clientID, UInt32 enable)
{
    if (enable) { } //fixes compiler warning
    //TBD: Call pedestal mode function here
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback volt:%ld!\n", enable);
    CAPI2_CP2AP_PedestalMode_Control_RSP(tid, clientID, TRUE);
}

void CAPI2_resetDataSize(UInt32 tid, UInt8 clientID, UInt8 cid)
{
    if (tid || clientID || cid) { } //fixes warning
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback cid:%d!\n", cid);
    CAPI2_resetDataSize_RSP(tid, clientID, TRUE);
}
 
void CAPI2_addDataSentSizebyCid(UInt32 tid, UInt8 clientID, UInt8 cid, UInt32 size)
{
    if (tid || clientID || cid || size) { } //fixes warning
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback size:%lu!\n", size);
    CAPI2_addDataSentSizebyCid_RSP(tid, clientID, TRUE);
}
 
void CAPI2_addDataRcvSizebyCid(UInt32 tid, UInt8 clientID, UInt8 cid, UInt32 size)
{
   if (tid || clientID || cid || size) { } //fixes warning
   CAPI2_addDataRcvSizebyCid_RSP(tid, clientID, TRUE);
}

/* CAPI2 callback functions */
void CAPI2_PMU_IsSIMReady(UInt32 tid, UInt8 clientID)
{
    if (tid || clientID) { } //fixes warnings
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback\n");
   CAPI2_PMU_IsSIMReady_RSP(tid, clientID, pmu_is_sim_ready());
}

void CAPI2_PMU_ActivateSIM(UInt32 tid, UInt8 clientID,Int32 volt)
{
   if (tid || clientID) { } //fixes warnings
    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback volt:%lu!\n", volt);
   CAPI2_PMU_ActivateSIM_RSP(tid, clientID,  pmu_activate_sim(volt));
}

void CAPI2_PMU_DeactivateSIM(UInt32 tid, UInt8 clientID)
{
    if (tid || clientID) { } //fixes warnings
   
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback\n");
    CAPI2_PMU_DeactivateSIM_RSP(tid, clientID, pmu_activate_sim(PMU_SIM0P0Volt));
}

//CAPI2_LINUX_INTEGRATION_CHANGE 1.2.10.2 TEMPORARY
void CAPI2_GPIO_ConfigOutput_64Pin(UInt32 tid, UInt8 clientID, UInt32 mask)
{
    if (tid || clientID || mask) { } //fixes warnings

    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback\n");
    CAPI2_GPIO_ConfigOutput_64Pin_RSP(tid, clientID, FALSE);
}

void CAPI2_GPIO_Set_High_64Pin(UInt32 tid, UInt8 clientID,UInt32 gpio_pin)
{
    if (tid || clientID || gpio_pin) { } //fixes warnings

    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback\n");
    CAPI2_GPIO_Set_High_64Pin_RSP(tid, clientID, FALSE);
}

void CAPI2_GPIO_Set_Low_64Pin(UInt32 tid, UInt8 clientID, UInt32 gpio_pin)
{
    if (tid || clientID || gpio_pin) { } //fixes warnings

    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback\n");
    CAPI2_GPIO_Set_Low_64Pin_RSP(tid, clientID, FALSE);
}

void CAPI2_PMU_StartCharging(UInt32 tid, UInt8 clientID)
{
    if (tid || clientID) { }

    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback\n");
    CAPI2_PMU_StartCharging_RSP(tid, clientID, FALSE);
}

void CAPI2_PMU_StopCharging(UInt32 tid, UInt8 clientID)
{
    if (tid || clientID) { } //fixes warnings

    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback\n");
    CAPI2_PMU_StopCharging_RSP(tid, clientID, FALSE);
}
#endif // CONFIG_BRCM_FUSE_RIL_CIB


void CAPI2_FFS_Control(UInt32 tid, UInt8 clientID, UInt32 cmd, UInt32 address, UInt32 offset, UInt32 size)
{
    IPC_PersistentDataStore_t dataPtr;
    FFS_Control_Result_en_t ffsctrlResult = FFS_CONTROL_PASS;
    int retval;
    struct file *hFileTmp = NULL;
    struct inode *inode   = NULL;
    UInt32 fsize;
    void __iomem* pPersistDataAddr = NULL;
    mm_segment_t orgfs = get_fs();
    set_fs(KERNEL_DS);
            
    KRIL_DEBUG(DBG_INFO,"tid:%lu clientID:%u cmd:%lu address:0x%lX offset:%lu size:%lu\n",
        tid, clientID, cmd, address, offset, size);
        
    IPC_GetPersistentData(&dataPtr);
    
    if ((offset >= dataPtr.DataLength) || 
        (size > dataPtr.DataLength) || (!size) || 
        ((offset + size) > dataPtr.DataLength))
    {
        KRIL_DEBUG(DBG_ERROR,"FFS Parameter Error!! dataPtr.DataLength:%d offset:%lu size:%lu\n",
            dataPtr.DataLength, offset, size);
        ffsctrlResult = FFS_CONTROL_INVALID_PARAM;
        goto Exit;
    }

#ifdef CONFIG_BRCM_FUSE_RIL_CIB
    pPersistDataAddr = ioremap_nocache((UInt32)(dataPtr.DataPtr), dataPtr.DataLength);
    if ( !pPersistDataAddr )
    {
        KRIL_DEBUG(DBG_ERROR,"Failed to remap persist data ptr: dataPtr.DataPtr:0x%x dataPtr.DataLength:%d offset:%lu size:%lu\n",
            dataPtr.DataPtr, dataPtr.DataLength, offset, size);
        ffsctrlResult = FFS_CONTROL_INVALID_PARAM;
        goto Exit;
    }
#else
    pPersistDataAddr = dataPtr.DataPtr;
#endif
            
    switch (cmd)
    {
        case FFS_CONTROL_COPY_NVS_FILE_TO_SM: // copy the NVS file to shared memory
        {
            hFileTmp = filp_open("/data/cp_data.txt", O_RDONLY, 0);
            if (IS_ERR(hFileTmp))
            {
                KRIL_DEBUG(DBG_ERROR,"Open cp_data.txt failed!!\n");
                ffsctrlResult = FFS_CONTROL_FILE_OP_FAIL;
                goto Exit;
            }
            
            if (hFileTmp->f_path.dentry)
            {
                inode = hFileTmp->f_path.dentry->d_inode;
                fsize = (UInt32)inode->i_size;
                
                if(fsize != dataPtr.DataLength)
                {
                    KRIL_DEBUG(DBG_ERROR,"fsize:%lu does not match Persistent size:%d Error!!\n",
                        fsize,dataPtr.DataLength);
                    ffsctrlResult = FFS_CONTROL_FILE_OP_FAIL;
                    goto Exit;
                }
            }
            else
            {
                KRIL_DEBUG(DBG_ERROR,"hFileTmp->f_path.dentry is invalid\n");
                ffsctrlResult = FFS_CONTROL_FILE_OP_FAIL;
                goto Exit;
            }

            if (hFileTmp->f_op && hFileTmp->f_op->read)
            {
                retval = hFileTmp->f_op->read(hFileTmp, pPersistDataAddr, dataPtr.DataLength, &hFileTmp->f_pos);
                
                if (retval > 0)
                {
                    KRIL_DEBUG(DBG_TRACE,"COPY_NVS_FILE_TO_SM successfully\n");
                    ffsctrlResult = FFS_CONTROL_PASS;
                }
                else
                {
                    KRIL_DEBUG(DBG_ERROR,"COPY_NVS_FILE_TO_SM failed!! retval:%d dataPtr.DataLength:%d\n",
                        retval, dataPtr.DataLength);
                    ffsctrlResult = FFS_CONTROL_IPCAP_PDS_OP_FAIL;
                }
            }
            else
            {
                KRIL_DEBUG(DBG_ERROR,"hFileTmp does not have a read method\n");
                ffsctrlResult = FFS_CONTROL_FILE_OP_FAIL;
            }
            
            filp_close(hFileTmp, NULL);
            break;
        }

        case FFS_CONTROL_COPY_SM_TO_NVS_FILE: // copy shared memory to the NVS file
        {
            hFileTmp = filp_open("/data/cp_data.txt", O_CREAT|O_TRUNC|O_WRONLY, 0);
            if (IS_ERR(hFileTmp))
            {
                KRIL_DEBUG(DBG_ERROR,"Create cp_data.txt failed!!\n");
                ffsctrlResult = FFS_CONTROL_FILE_OP_FAIL;
                goto Exit;
            }
            
            if (hFileTmp->f_op && hFileTmp->f_op->write)
            {
                retval = hFileTmp->f_op->write(hFileTmp, pPersistDataAddr, dataPtr.DataLength, &hFileTmp->f_pos);

                if (retval != dataPtr.DataLength || retval <= 0)
                {
                    KRIL_DEBUG(DBG_ERROR,"COPY_SM_TO_NVS_FILE failed!! retval:%d dataPtr.DataLength:%d ptr:0x%lx\n",
                        retval, dataPtr.DataLength, (UInt32)(pPersistDataAddr));
                    ffsctrlResult = FFS_CONTROL_IPCAP_PDS_OP_FAIL;
                }
                else
                {
                    KRIL_DEBUG(DBG_TRACE,"COPY_SM_TO_NVS_FILE successfully!!\n");
                    ffsctrlResult = FFS_CONTROL_PASS;
                }
            }
            else
            {
                KRIL_DEBUG(DBG_ERROR,"hFileTmp does not have a write method\n");
                ffsctrlResult = FFS_CONTROL_FILE_OP_FAIL;
            }
            
            filp_close(hFileTmp, NULL);
            break;
        }
    }

Exit:
    set_fs(orgfs);
#ifndef CONFIG_BRCM_FUSE_RIL_CIB
    CAPI2_FFS_Control_RSP((UInt32)tid, (UInt8)clientID, ffsctrlResult);
#else
    if ( NULL != pPersistDataAddr )
    {
        iounmap( pPersistDataAddr );
    }
    KRIL_SysRpc_SendFFSControlRsp( (UInt32)tid, (UInt8)clientID, ffsctrlResult );
#endif
}

#ifndef CONFIG_BRCM_FUSE_RIL_CIB

void CAPI2_SMS_IsMeStorageEnabled(UInt32 tid, UInt8 clientID)
{
    if (tid || clientID) { } //fixes warnings

    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback\n");
    CAPI2_SMS_IsMeStorageEnabled_RSP(tid, clientID, FALSE);
}

void CAPI2_SMS_GetMaxMeCapacity(UInt32 tid, UInt8 clientID)
{
    if (tid || clientID) { } //fixes warnings

    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback: return 10!\n");
    CAPI2_SMS_GetMaxMeCapacity_RSP(tid, clientID, 10);
}

void CAPI2_SMS_GetNextFreeSlot(UInt32 tid, UInt8 clientID)
{
    if (tid || clientID) { } //fixes warnings

    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback return 0!\n");
    CAPI2_SMS_GetNextFreeSlot_RSP(tid, clientID, 0);
}

void CAPI2_SMS_SetMeSmsStatus(UInt32 tid, UInt8 clientID, UInt16 slotNumber, SIMSMSMesgStatus_t status)
{
    if (tid || clientID || slotNumber || status) { } //fixes warnings

    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback!\n");
    CAPI2_SMS_SetMeSmsStatus_RSP(tid, clientID, FALSE);
}

void CAPI2_SMS_GetMeSmsStatus(UInt32 tid, UInt8 clientID, UInt16 slotNumber)
{
    if (tid || clientID || slotNumber) { } //fixes warnings
   
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback return SIMSMSMESGSTATUS_FREE!\n");
    CAPI2_SMS_GetMeSmsStatus_RSP(tid, clientID, SIMSMSMESGSTATUS_FREE);
}

void CAPI2_SMS_StoreSmsToMe(UInt32 tid, UInt8 clientID, UInt8 *inSms, UInt16 inLength, SIMSMSMesgStatus_t status, UInt16 slotNumber)
{
    if (tid || clientID || inSms || inLength || status || slotNumber) { } //fixes warnings

    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback!\n");
    CAPI2_SMS_StoreSmsToMe_RSP(tid, clientID, FALSE);
}

void CAPI2_SMS_RetrieveSmsFromMe(UInt32 tid, UInt8 clientID, UInt16 slotNumber)
{
    if (tid || clientID || slotNumber) { } //fixes warnings

    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback!\n");
    CAPI2_SMS_RetrieveSmsFromMe_RSP(tid, clientID, FALSE);
}

void CAPI2_SMS_RemoveSmsFromMe(UInt32 tid, UInt8 clientID, UInt16 slotNumber)
{
    if (tid || clientID || slotNumber) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback!\n");
    CAPI2_SMS_RemoveSmsFromMe_RSP(tid, clientID, FALSE);
}

void CAPI2_SMS_GetRecordNumberOfReplaceSMS(UInt32 tid, UInt8 clientID, SmsStorage_t storageType, UInt8 tp_pid, uchar_ptr_t oaddress)
{
    if (tid || clientID) { } //fixes warnings
   
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback return 0!\n");
    CAPI2_SMS_GetRecordNumberOfReplaceSMS_RSP(tid, clientID, 0);
}

void CAPI2_RTC_GetTime(UInt32 tid, UInt8 clientID, RTCTime_t *time)
{
    RTCTime_t  rtc_time;
    if (tid || clientID) { } //fixes warnings
    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback!\n");

    rtc_time.Sec   = (UInt8)30;
    rtc_time.Min   = (UInt8)1;
    rtc_time.Hour  = (UInt8)1;
    rtc_time.Day   = (UInt8)1;
    rtc_time.Week  = (UInt8)2;
    rtc_time.Month = (UInt8)1;
    rtc_time.Year  = (UInt16)99;
    
    CAPI2_RTC_GetTime_RSP(tid, clientID, rtc_time);
}

void _DEF(CAPI2_RTC_SetTime)(UInt32 tid, UInt8 clientID, RTCTime_t *inTime)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback!\n");
    CAPI2_RTC_SetTime_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_RTC_SetDST)(UInt32 tid, UInt8 clientID, UInt8 inDST)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback!\n");
    CAPI2_RTC_SetDST_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_RTC_GetDST)(UInt32 tid, UInt8 clientID)
{
    UInt8 dst = 0;
    KRIL_DEBUG(DBG_TRACE," --->CAPI2_RTC_GetDST::Callback!\n");

    CAPI2_RTC_GetDST_RSP(tid, clientID, dst);
}

void _DEF(CAPI2_RTC_SetTimeZone)(UInt32 tid, UInt8 clientID, Int8 inTimezone)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback!\n");
    CAPI2_RTC_SetTimeZone_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_RTC_GetTimeZone)(UInt32 tid, UInt8 clientID)
{
    Int8 timeZone = 0;
    if (tid || clientID) { } //fixes warnings

    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback!\n");
    CAPI2_RTC_GetTimeZone_RSP(tid, clientID, timeZone);
}

#endif

#ifndef CONFIG_BRCM_FUSE_RIL_CIB
void _DEF(CAPI2_SIMLOCK_GetStatus)(UInt32 tid, UInt8 clientID, SIMLOCK_SIM_DATA_t *sim_data)
#else
void CAPI2_SIMLOCK_GetStatus(UInt32 tid, UInt8 clientID, SIMLOCK_SIM_DATA_t *sim_data, Boolean is_testsim)
#endif
{
    SIMLOCK_STATE_t simlock_state;

    KRIL_DEBUG(DBG_INFO,"Get SIM lock status\n");
    SetSIMData(sim_data);
    
    simlock_state.network_lock = SIM_SECURITY_OPEN; 
    simlock_state.network_subset_lock = SIM_SECURITY_OPEN;
    simlock_state.service_provider_lock = SIM_SECURITY_OPEN;
    simlock_state.corporate_lock = SIM_SECURITY_OPEN;
    simlock_state.phone_lock = SIM_SECURITY_OPEN;
    
    SIMLockCheckAllLocks(sim_data->imsi_string, (0 != sim_data->gid1_len ? sim_data->gid1 : NULL), 
        (0 != sim_data->gid2_len ? sim_data->gid2 : NULL));
    
    SIMLockUpdateSIMLockState();
    
    SIMLockGetSIMLockState(&simlock_state);
    
    CAPI2_SIMLOCK_GetStatus_RSP(tid, clientID, simlock_state);
}

#ifndef CONFIG_BRCM_FUSE_RIL_CIB
void _DEF(CAPI2_FFS_Read)(UInt32 tid, UInt8 clientID, FFS_ReadReq_t *ffsReadReq)
{
    FFS_Data_t response = {NULL, 0};

    if (tid || clientID) { } //fixes warnings

    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback mFileId:%ld mMaxLength:%lu!\n", ffsReadReq->mFileId, ffsReadReq->mMaxLength);
    CAPI2_FFS_Read_RSP(tid, clientID, &response);
}

void _DEF(CAPI2_GPS_Control)(UInt32 tid, UInt8 clientID, UInt32 u32Cmnd, UInt32 u32Param0, UInt32 u32Param1, UInt32 u32Param2, UInt32 u32Param3, UInt32 u32Param4)
{
    HAL_GPS_Result_en_t HAL_GPS_Result = HAL_GPS_PASS;
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE," --->Capi2 Callback!\n");
    CAPI2_GPS_Control_RSP(tid, clientID, HAL_GPS_Result);
}

void _DEF(CAPI2_SMS_GetMeSmsBufferStatus)(UInt32 tid, UInt8 clientID, UInt16 cmd)
{
    if (tid || clientID || cmd) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_SMS_GetMeSmsBufferStatus_RSP(tid, clientID, 0, 0);
}

void _DEF(CAPI2_PMU_GetPowerupCause)(UInt32 tid, UInt8 clientID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_PMU_GetPowerupCause_RSP(tid, clientID, PMU_POWERUP_POWERKEY);
}

void _DEF(CAPI2_PMU_ClientPowerDown)(UInt32 tid, UInt8 clientID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_PMU_ClientPowerDown_RSP(tid, clientID, FALSE);
}

void _DEF(CAPI2_SOCKET_Open)(UInt32 tid, UInt8 clientID, UInt8 domain, UInt8 type, UInt8 protocol)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_SOCKET_Open_RSP(tid, clientID, 10);
}

void _DEF(CAPI2_SOCKET_Bind)(UInt32 tid, UInt8 clientID, Int32 descriptor, sockaddr* addr)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_SOCKET_Bind_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SOCKET_Listen)(UInt32 tid, UInt8 clientID, Int32 descriptor, UInt32 backlog)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_SOCKET_Listen_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SOCKET_Accept)(UInt32 tid, UInt8 clientID, Int32 descriptor)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    //CAPI2_SOCKET_Accept_RSP(tid, clientID, NULL);
}

void _DEF(CAPI2_SOCKET_Connect)(UInt32 tid, UInt8 clientID, Int32 descriptor, sockaddr* name)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_SOCKET_Connect_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SOCKET_GetPeerName)(UInt32 tid, UInt8 clientID, Int32 descriptor)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    //CAPI2_SOCKET_GetPeerName_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SOCKET_GetSockName)(UInt32 tid, UInt8 clientID, Int32 descriptor)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    //CAPI2_SOCKET_GetSockName_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SOCKET_SetSockOpt)(UInt32 tid, UInt8 clientID, Int32 descriptor, UInt16 optname, SockOptVal_t* optval)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_SOCKET_SetSockOpt_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SOCKET_GetSockOpt)(UInt32 tid, UInt8 clientID, Int32 descriptor, UInt16 optname)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    //SocketGetSockOptRsp_t val;
    CAPI2_SOCKET_GetSockOpt_RSP(tid, clientID, NULL, 0);
}

void _DEF(CAPI2_SOCKET_Send)(UInt32 tid, UInt8 clientID, SocketSendReq_t* sockSendReq)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    //CAPI2_SOCKET_Send_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SOCKET_SendTo)(UInt32 tid, UInt8 clientID, SocketSendReq_t* sockSendReq, sockaddr* to)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    //CAPI2_SOCKET_SendTo_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SOCKET_Recv)(UInt32 tid, UInt8 clientID, SocketRecvReq_t* sockRecvReq)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    //CAPI2_SOCKET_Recv_RSP(tid, clientID, -1);
}

void _DEF(CAPI2_SOCKET_RecvFrom)(UInt32 tid, UInt8 clientID, SocketRecvReq_t *sockRecvReq, sockaddr *from)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    //CAPI2_SOCKET_RecvFrom_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SOCKET_Close)(UInt32 tid, UInt8 clientID, Int32 descriptor)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_SOCKET_Close_RSP(tid, clientID, 0);
}

void _DEF(CAPI2_SOCKET_Shutdown)(UInt32 tid, UInt8 clientID, Int32 descriptor, UInt8 how)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_SOCKET_Shutdown_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SOCKET_Errno)(UInt32 tid, UInt8 clientID, Int32 descriptor)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_SOCKET_Errno_RSP(tid, clientID, 0);
}

void _DEF(CAPI2_SOCKET_SO2LONG)(UInt32 tid, UInt8 clientID, Int32 socket)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_SOCKET_SO2LONG_RSP(tid, clientID, 0);
}


void _DEF(CAPI2_SOCKET_GetSocketSendBufferSpace)(UInt32 tid, UInt8 clientID, Int32 bufferSpace)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_SOCKET_GetSocketSendBufferSpace_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DC_SetupDataConnection)(UInt32 tid, UInt8 clientID, UInt8 inClientID, UInt8 acctID, DC_ConnectionType_t linkType)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DC_SetupDataConnection_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DC_SetupDataConnectionEx)(UInt32 tid, UInt8 clientID, UInt8 inClientID, UInt8 acctID, DC_ConnectionType_t linkType, uchar_ptr_t apnCheck, uchar_ptr_t actDCAcctId)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DC_SetupDataConnectionEx_RSP(tid, clientID, 0, 0);
}

void _DEF(CAPI2_DC_ShutdownDataConnection)(UInt32 tid, UInt8 clientID, UInt8 inClientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DC_ShutdownDataConnection_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_IsAcctIDValid)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_IsAcctIDValid_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_CreateGPRSDataAcct)(UInt32 tid, UInt8 clientID, UInt8 acctID, GPRSContext_t *pGprsSetting)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_CreateGPRSDataAcct_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_CreateCSDDataAcct)(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDContext_t *pCsdSetting)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_CreateCSDDataAcct_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_DeleteDataAcct)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_DeleteDataAcct_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_SetUsername)(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t username)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_SetUsername_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetUsername)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetUsername_RSP(tid, clientID, NULL);
}

void _DEF(CAPI2_DATA_SetPassword)(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t password)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_SetPassword_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetPassword)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetPassword_RSP(tid, clientID, NULL);
}

void _DEF(CAPI2_DATA_SetStaticIPAddr)(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t staticIPAddr)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_SetStaticIPAddr_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetStaticIPAddr)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetStaticIPAddr_RSP(tid, clientID, NULL);
}

void _DEF(CAPI2_DATA_SetPrimaryDnsAddr)(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t priDnsAddr)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_SetPrimaryDnsAddr_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetPrimaryDnsAddr)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetPrimaryDnsAddr_RSP(tid, clientID, NULL);
}

void _DEF(CAPI2_DATA_SetSecondDnsAddr)(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t sndDnsAddr)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_SetSecondDnsAddr_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetSecondDnsAddr)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetSecondDnsAddr_RSP(tid, clientID, NULL);
}

void _DEF(CAPI2_DATA_SetDataCompression)(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean dataCompEnable)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_SetDataCompression_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetDataCompression)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetDataCompression_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetAcctType)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetAcctType_RSP(tid, clientID, DATA_ACCOUNT_NOT_USED);
}

void _DEF(CAPI2_DATA_GetEmptyAcctSlot)(UInt32 tid, UInt8 clientID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetEmptyAcctSlot_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetCidFromDataAcctID)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetCidFromDataAcctID_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetDataAcctIDFromCid)(UInt32 tid, UInt8 clientID, UInt8 contextID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetDataAcctIDFromCid_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetPrimaryCidFromDataAcctID)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetPrimaryCidFromDataAcctID_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_IsSecondaryDataAcct)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_IsSecondaryDataAcct_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetDataSentSize)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetDataSentSize_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetDataRcvSize)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetDataRcvSize_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_SetGPRSPdpType)(UInt32 tid, UInt8 clientID, UInt8 acctID, UInt8 *pdpType)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_SetGPRSPdpType_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetGPRSPdpType)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetGPRSPdpType_RSP(tid, clientID, NULL);
}

void _DEF(CAPI2_DATA_SetGPRSApn)(UInt32 tid, UInt8 clientID, UInt8 acctID, UInt8 *apn)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_SetGPRSApn_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetGPRSApn)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetGPRSApn_RSP(tid, clientID, NULL);
}

void _DEF(CAPI2_DATA_SetAuthenMethod)(UInt32 tid, UInt8 clientID, UInt8 acctID, DataAuthenMethod_t authenMethod)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_SetAuthenMethod_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetAuthenMethod)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetAuthenMethod_RSP(tid, clientID, DATA_DEFAULT_AUTHEN_TYPE);
}

void _DEF(CAPI2_DATA_SetGPRSHeaderCompression)(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean headerCompEnable)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_SetGPRSHeaderCompression_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetGPRSHeaderCompression)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetGPRSHeaderCompression_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_SetGPRSQos)(UInt32 tid, UInt8 clientID, UInt8 acctID, PCHQosProfile_t qos)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_SetGPRSQos_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetGPRSQos)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID || acctID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    //CAPI2_DATA_GetGPRSQos_RSP(tid, clientID, NULL);
}

void _DEF(CAPI2_DATA_SetAcctLock)(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean acctLock)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_SetAcctLock_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetAcctLock)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetAcctLock_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_SetGprsOnly)(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean gprsOnly)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_SetGprsOnly_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetGprsOnly)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetGprsOnly_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_SetGPRSTft)(UInt32 tid, UInt8 clientID, UInt8 acctID, PCHTrafficFlowTemplate_t *pTft)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_SetGPRSTft_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetGPRSTft)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    //CAPI2_DATA_GetGPRSTft_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_SetCSDDialNumber)(UInt32 tid, UInt8 clientID, UInt8 acctID, UInt8 *dialNumber)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_SetCSDDialNumber_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetCSDDialNumber)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetCSDDialNumber_RSP(tid, clientID, NULL);
}

void _DEF(CAPI2_DATA_SetCSDDialType)(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDDialType_t csdDialType)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_SetCSDDialType_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetCSDDialType)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetCSDDialType_RSP(tid, clientID, DATA_DEFAULT_DIAL_TYPE);
}

void _DEF(CAPI2_DATA_SetCSDBaudRate)(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDBaudRate_t csdBaudRate)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_SetCSDBaudRate_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetCSDBaudRate)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetCSDBaudRate_RSP(tid, clientID, DATA_DEFAULT_BAUDRATE);
}

void _DEF(CAPI2_DATA_SetCSDSyncType)(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDSyncType_t csdSyncType)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_SetCSDSyncType_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetCSDSyncType)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetCSDSyncType_RSP(tid, clientID, DATA_DEFAULT_SYNCTYPE);
}

void _DEF(CAPI2_DATA_SetCSDErrorCorrection)(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean enable)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_SetCSDErrorCorrection_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetCSDErrorCorrection)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetCSDErrorCorrection_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_SetCSDErrCorrectionType)(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDErrCorrectionType_t errCorrectionType)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_SetCSDErrCorrectionType_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetCSDErrCorrectionType)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetCSDErrCorrectionType_RSP(tid, clientID, DATA_DEFAULT_EC_TYPE);
}

void _DEF(CAPI2_DATA_SetCSDDataCompType)(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDDataCompType_t dataCompType)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_SetCSDDataCompType_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetCSDDataCompType)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetCSDDataCompType_RSP(tid, clientID, DATA_DEFAULT_DC_TYPE);
}

void _DEF(CAPI2_DATA_SetCSDConnElement)(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDConnElement_t connElement)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_SetCSDConnElement_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetCSDConnElement)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    if (tid || clientID || acctID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_GetCSDConnElement_RSP(tid, clientID, DATA_DEFAULT_CONN_ELEMENT);
}

void _DEF(CAPI2_DATA_UpdateAccountToFileSystem)(UInt32 tid, UInt8 clientID)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
    CAPI2_DATA_UpdateAccountToFileSystem_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_AT_ProcessCmdToAP)(UInt32 tid, UInt8 clientID, UInt8 channel, uchar_ptr_t cmdStr)
{
    if (tid || clientID) { } //fixes warnings
    
    return;
}

void _DEF(CAPI2_InterTaskMsgToAP)(UInt32 tid, UInt8 clientID, InterTaskMsg_t *inPtrMsg)
{
    return;
}

void _DEF(CAPI2_FLASH_SaveImage)(UInt32 tid, UInt8 clientID, UInt32 flash_addr, UInt32 length, UInt32 shared_mem_addr)
{
    CAPI2_FLASH_SaveImage_RSP(tid, clientID,  TRUE);
}

void _DEF(CAPI2_SOCKET_ParseIPAddr)(UInt32 tid, UInt8 clientID, char_ptr_t ipString)
{
    if (tid || clientID) { } //fixes warnings
      
    /* NOT implemented yet */
    KRIL_DEBUG(DBG_TRACE, " --->Capi2 Callback!\n");
}
#endif //  CONFIG_BRCM_FUSE_RIL_CIB


#ifdef CONFIG_BRCM_FUSE_RIL_CIB
void CAPI2_InterTaskMsgToAP(UInt32 tid, UInt8 clientID, InterTaskMsg_t *inPtrMsg)
{
    KRIL_DEBUG(DBG_ERROR,"*** unhandled call to CAPI2_InterTaskMsgToAP \n");
    CAPI2_InterTaskMsgToAP_RSP( tid, clientID );
}
#endif