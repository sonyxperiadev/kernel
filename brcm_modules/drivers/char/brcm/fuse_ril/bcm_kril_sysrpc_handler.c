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
#include "bcm_kril_sysrpc_handler.h"
#include "bcm_cp_cmd_handler.h"
#include "bcm_kril_ioctl.h"

#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>

#include "sys_common_rpc.h"
#include "sys_gen_rpc.h"
#include "sys_rpc.h"

static void KRIL_SysRpcHandleAckCbk(UInt32 tid, UInt8 clientid, RPC_ACK_Result_t ackResult, UInt32 ackUsrData);
static void KRIL_SysRpcHandleCapiReqMsg(RPC_Msg_t* pMsg, ResultDataBufHandle_t dataBufHandle, UInt32 userContextData);
static void KRIL_SysRpcHandleCapiEventRspCb(RPC_Msg_t* pMsg, 
						            ResultDataBufHandle_t dataBufHandle, 
						            UInt32 userContextData);
static void KRIL_SysRpcHandler(struct work_struct *data);
extern void KRIL_Capi2HandleFlowCtrl(RPC_FlowCtrlEvent_t event, UInt8 channel);


#define THREE_VOLTS_IN_MICRO_VOLTS 3000000
#define THREE_PT_THREE_VOLTS_IN_MICRO_VOLTS 3300000
#define THREE_PT_ONE_VOLTS_IN_MICRO_VOLTS 3100000
#define TWO_PT_FIVE_VOLTS_IN_MICRO_VOLTS 2500000
#define ONE_PT_EIGHT_VOLTS_IN_MICRO_VOLTS 1800000

static struct regulator* sim_regulator = NULL;

typedef struct
{
    struct list_head list;
    RPC_Msg_t* pMsg; 
    ResultDataBufHandle_t dataBufHandle;
} Kril_SysRpcInfo_t;

typedef struct 
{
    spinlock_t lock;
    struct work_struct sysRpcQ;
    struct workqueue_struct* sysRpc_wq;
    Kril_SysRpcInfo_t sysRpc_head;
} KRIL_SysRpcWq_t;

// work queue used to process CP->AP commands passed thru SysRpc channel
KRIL_SysRpcWq_t sKrilSysRpcWq;
// wake lock for SysRpc channel
#ifdef CONFIG_HAS_WAKELOCK
struct wake_lock sKrilSysRpcWakeLock;
#endif

// initialize sysrpc interface
void KRIL_SysRpc_Init( void )
{
    static RPC_Handle_t sRPCHandle = 0;
	RPC_InitParams_t params;

    KRIL_DEBUG(DBG_ERROR," enter SysRpc Init\n");
    if ( !sRPCHandle )
    {
        int ret;
        
        KRIL_DEBUG(DBG_ERROR,"initialising\n");
        
        // initialize work queue for handling CP->AP requests 
        INIT_LIST_HEAD(&sKrilSysRpcWq.sysRpc_head.list);
        INIT_WORK(&sKrilSysRpcWq.sysRpcQ, KRIL_SysRpcHandler);
        sKrilSysRpcWq.sysRpc_wq = create_workqueue("sysRpc_wq");
        spin_lock_init(&sKrilSysRpcWq.lock);

        // **FIXME** need to add method to shutdown sysRPC so we can destroy wakelocks, etc
#ifdef CONFIG_HAS_WAKELOCK
        wake_lock_init(&sKrilSysRpcWakeLock, WAKE_LOCK_SUSPEND, "kril_sysrpc_wake_lock");
#endif

    	params.ackCb = KRIL_SysRpcHandleAckCbk;
    	params.flowCb = KRIL_Capi2HandleFlowCtrl;
    	params.iType = INTERFACE_RPC_DEFAULT;
    	params.mainProc = xdr_SYS_ReqRep_t;
    	params.maxDataBufSize = sizeof(SYS_ReqRep_t);
    	params.reqCb = KRIL_SysRpcHandleCapiReqMsg;
    	params.respCb = KRIL_SysRpcHandleCapiEventRspCb;
		sysGetXdrStruct(&(params.xdrtbl), &(params.table_size));
    	params.userData = 0;

    	sRPCHandle = RPC_SYS_RegisterClient(&params);
        KRIL_DEBUG(DBG_ERROR,"RPC_SYS_RegisterClient returned 0x%x\n", (int)sRPCHandle);

        // **FIXME** add this to actual module init? Need a way to shut it down as well...
        KRIL_DEBUG(DBG_ERROR,"calling regulator_get...\n");
        sim_regulator = regulator_get(NULL,"sim_vcc");
        if ( sim_regulator > 0 )
        {
            KRIL_DEBUG(DBG_ERROR," regulator_get OK\n");
        }
        else
        {
            KRIL_DEBUG(DBG_ERROR," **regulator_get FAILED %d\n", sim_regulator);
        }
        
        // **FIXME** workaround for SIM LDO being enabled on startup by the driver; this 
        // just bumps up our ref count in the sim_regulator struct, so that the 
        // regulator_disable() call below doesn't fail
        /*
        KRIL_DEBUG(DBG_ERROR," calling regulator_enable...\n");
        ret = regulator_enable(sim_regulator);
        KRIL_DEBUG(DBG_ERROR," regulator_enable returned %d\n",ret);
        */
        // **FixMe** This causes a kernel panic on Big Island; disable for now
        KRIL_DEBUG(DBG_ERROR," don't call regulator_enable \n");
    }
    else
    {
        KRIL_DEBUG(DBG_ERROR," ** already started\n");
    }
}

//
// workqueue to handle CP->AP requests via SysRpc channel
//
void KRIL_SysRpcHandler(struct work_struct *data)
{
    struct list_head *ptr, *pos;
    Kril_SysRpcInfo_t* entry = NULL;
    UInt32 irql;

    spin_lock_irqsave(&sKrilSysRpcWq.lock, irql);
    list_for_each_safe(ptr, pos, &sKrilSysRpcWq.sysRpc_head.list)
    {
        SYS_ReqRep_t* pReq = NULL;
        entry = list_entry(ptr, Kril_SysRpcInfo_t, list);
        spin_unlock_irqrestore(&sKrilSysRpcWq.lock, irql);
	
	    pReq = (SYS_ReqRep_t*)entry->pMsg->dataBuf;

        KRIL_DEBUG(DBG_ERROR," calling SYS_GenCommsMsgHnd\n");
		SYS_GenCommsMsgHnd( entry->pMsg, pReq );
        
        spin_lock_irqsave(&sKrilSysRpcWq.lock, irql);

        RPC_SYSFreeResultDataBuffer(entry->dataBufHandle);

        list_del(ptr); // delete response capi2
        kfree(entry);
        entry = NULL;
    }
    
    spin_unlock_irqrestore(&sKrilSysRpcWq.lock, irql);

#ifdef CONFIG_HAS_WAKELOCK
    wake_unlock(&sKrilSysRpcWakeLock);
#endif
    return;
}

/* Ack call back */
void KRIL_SysRpcHandleAckCbk(UInt32 tid, UInt8 clientid, RPC_ACK_Result_t ackResult, UInt32 ackUsrData)
{
    switch(ackResult)
    {
        case ACK_SUCCESS:
        {
            //capi2 request ack succeed
	        KRIL_DEBUG(DBG_ERROR, "KRIL_SysRpcHandleAckCbk::AckCbk ACK_SUCCESS tid=%lu\n", tid);
        }
        break;

        case ACK_FAILED:
        {
	        KRIL_DEBUG(DBG_ERROR, "KRIL_SysRpcHandleAckCbk::AckCbk ACK_FAILED\n");
            //capi2 ack fail for unknown reasons
        }
        break;

        case ACK_TRANSMIT_FAIL:
        {
	        KRIL_DEBUG(DBG_ERROR, "KRIL_SysRpcHandleAckCbk::AckCbk ACK_TRANSMIT_FAIL\n");
            //capi2 ack fail due to fifo full, fifo mem full etc.
        }
        break;

        case ACK_CRITICAL_ERROR:
        {
            KRIL_DEBUG(DBG_ERROR, "KRIL_SysRpcHandleAckCbk::AckCbk ACK_CRITICAL_ERROR\n");
            //capi2 ack fail due to comms processor reset ( The use case for this error is TBD )  
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "KRIL_SysRpcHandleAckCbk::AckCbk ackResult error!\n");
        }
        break;
    }
}

void KRIL_SysRpcHandleCapiReqMsg(RPC_Msg_t* pMsg, ResultDataBufHandle_t dataBufHandle, UInt32 userContextData)
{
    KRIL_DEBUG(DBG_ERROR," enter KRIL_SysRpcHandleCapiReqMsg\n");
	RPC_SendAckForRequest(dataBufHandle, 0);

	if(pMsg->msgId == MSG_AT_COMMAND_REQ)
	{
        KRIL_DEBUG(DBG_ERROR," MSG_AT_COMMAND_REQ ignoring\n");
	}
	else
	{
	    // dispatch to work queue for actual processing
        Kril_SysRpcInfo_t *pSysRpc_Req = NULL;
        unsigned long irql;
        
        pSysRpc_Req = kmalloc(sizeof(Kril_SysRpcInfo_t), GFP_KERNEL);
        if(!pSysRpc_Req)
        {
            KRIL_DEBUG(DBG_ERROR, "Unable to allocate SysRPC Request memory\n");
            RPC_SYSFreeResultDataBuffer(dataBufHandle);
            return;
        }

        // note: msg pointer stays valid as long as dataBufHandle is not released
        pSysRpc_Req->pMsg = pMsg;
        pSysRpc_Req->dataBufHandle = dataBufHandle;

#ifdef CONFIG_HAS_WAKELOCK
        wake_lock(&sKrilSysRpcWakeLock);
#endif
	spin_lock_irqsave(&sKrilSysRpcWq.lock, irql);
        list_add_tail(&pSysRpc_Req->list, &sKrilSysRpcWq.sysRpc_head.list); 
        spin_unlock_irqrestore(&sKrilSysRpcWq.lock, irql);
        queue_work(sKrilSysRpcWq.sysRpc_wq, &sKrilSysRpcWq.sysRpcQ);
	}

    return;   
}

void KRIL_SysRpcHandleCapiEventRspCb(RPC_Msg_t* pMsg, 
						 ResultDataBufHandle_t dataBufHandle, 
						 UInt32 userContextData)
{

    KRIL_DEBUG(DBG_ERROR," enter KRIL_SysRpcHandleCapiEventRspCb\n");
	RPC_SYSFreeResultDataBuffer(dataBufHandle);
}

// ******************************************************************************
/**	
*   Utility function to dispatch result message for FFS control requests from CP
*   back to CP.
*
*	@param inTid        (in) transaction id of original request from CP
*	@param inClientId	(in) client id of original request from CP
*	@param inFFSCtrlResult	(in) result of FFS control operation
*
********************************************************************************/
void KRIL_SysRpc_SendFFSControlRsp( UInt32 inTid, UInt8 inClientId, UInt32 inFFSCtrlResult )
{
    SYS_ReqRep_t data;
    RPC_Msg_t rsp;

    printk("KRIL_SysRpc_SendFFSControlRsp: result 0x%x\r\n", inFFSCtrlResult );
    memset(&data, 0, sizeof(SYS_ReqRep_t));
    data.req_rep_u.CAPI2_CPPS_Control_Rsp.val = inFFSCtrlResult;
    data.result = RESULT_OK;

    rsp.msgId = MSG_CPPS_CONTROL_RSP;
    rsp.tid = inTid;
    rsp.clientID = inClientId;
    rsp.dataBuf = (void*)&data;
    rsp.dataLen = 0;

    RPC_SerializeRsp(&rsp);
}

// 
// Handlers for CP to AP requests
//
Result_t Handle_CAPI2_CPPS_Control(RPC_Msg_t* pReqMsg, UInt32 cmd, UInt32 address, UInt32 offset, UInt32 size)
{
    CAPI2_FFS_Control(pReqMsg->tid, pReqMsg->clientID, cmd, address, offset, size);
    
    return RESULT_OK;
}

static UInt32 PedestalModeAllowedByCP = 0;

Result_t Handle_CAPI2_CP2AP_PedestalMode_Control(RPC_Msg_t* pReqMsg, UInt32 enable)
{
    Result_t result = RESULT_OK;
	SYS_ReqRep_t data;
    KRIL_DEBUG(DBG_ERROR," enter Handle_CAPI2_CP2AP_PedestalMode_Control\n");
    
	memset(&data, 0, sizeof(SYS_ReqRep_t));
	PedestalModeAllowedByCP = enable;

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_CP2AP_PEDESTALMODE_CONTROL_RSP, &data);
    return result;
}

static Boolean gSimInited = FALSE;

Result_t Handle_CAPI2_PMU_IsSIMReady(RPC_Msg_t* pReqMsg, PMU_SIMLDO_t simldo)
{
    Result_t result = RESULT_OK;
	SYS_ReqRep_t data;
    int ret=0;
    
    KRIL_DEBUG(DBG_ERROR," enter Handle_CAPI2_PMU_IsSIMReady\n");
    ret = regulator_is_enabled(sim_regulator);
    KRIL_DEBUG(DBG_ERROR," regulator_is_enabled return %d\n", ret);
    
	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.req_rep_u.CAPI2_PMU_IsSIMReady_Rsp.val = gSimInited; //(Boolean)(regulator_is_enabled > 0);

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_PMU_IS_SIM_READY_RSP, &data);

    KRIL_DEBUG(DBG_ERROR," Handle_CAPI2_PMU_IsSIMReady DONE %d\n", (int)gSimInited);
    return result;
}


Result_t Handle_CAPI2_PMU_ActivateSIM(RPC_Msg_t* pReqMsg, PMU_SIMLDO_t simldo, PMU_SIMVolt_t volt)
{
    Result_t result = RESULT_OK;
	SYS_ReqRep_t data;
    int simMicroVolts = 0;
    
    KRIL_DEBUG(DBG_ERROR," enter Handle_CAPI2_PMU_ActivateSIM\n");
    
    switch (volt)
    {
        case PMU_SIM3P0Volt:
        {
            KRIL_DEBUG(DBG_ERROR," PMU_SIM3P0Volt\n");
            // 3.0 V SIM
            simMicroVolts = THREE_VOLTS_IN_MICRO_VOLTS;
            break;
        }

#ifdef PMU_BCM59038
        case PMU_SIM3P3Volt:
        {
            KRIL_DEBUG(DBG_ERROR," PMU_SIM3P3Volt\n");
            // 3.3 V SIM
            simMicroVolts = THREE_PT_THREE_VOLTS_IN_MICRO_VOLTS;
            break;
        }
#endif // PMU_BCM59038

#ifdef PMU_MAX8986
		 case PMU_SIM3P1Volt:
		 {
			 KRIL_DEBUG(DBG_ERROR," PMU_SIM3P1Volt\n");
			 // 3.1 V SIM
			 simMicroVolts = THREE_PT_ONE_VOLTS_IN_MICRO_VOLTS;
			 break;
		 }
 #endif // PMU_MAX8986

        case PMU_SIM2P5Volt:
        {
            KRIL_DEBUG(DBG_ERROR," PMU_SIM2P5Volt\n");
            // 2.5 V SIM
            simMicroVolts = TWO_PT_FIVE_VOLTS_IN_MICRO_VOLTS;
            break;
        }
        
        case PMU_SIM1P8Volt:
        {
            KRIL_DEBUG(DBG_ERROR," PMU_SIM1P8Volt\n");
            // 1.8 V SIM
            simMicroVolts = ONE_PT_EIGHT_VOLTS_IN_MICRO_VOLTS;
            break;
        }
        
        case PMU_SIM0P0Volt:
        {
            int enabled = regulator_is_enabled(sim_regulator);
            KRIL_DEBUG(DBG_ERROR," ** PMU_SIM0P0Volt - regulator_is_enabled returned %d\n", enabled);
            
            simMicroVolts = 0;
            gSimInited = FALSE;
            if ( enabled > 0 )
            {
                int ret;
                KRIL_DEBUG(DBG_ERROR," ** PMU_SIM0P0Volt - turn off regulator (FORCE)\n");
                ret = regulator_disable(sim_regulator);
                KRIL_DEBUG(DBG_ERROR," regulator_disable returned 0x%x\n", ret);
 
            }
            else
            {
                //int ret;
                KRIL_DEBUG(DBG_ERROR," ** PMU_SIM0P0Volt - regulator not enabled, do nothing...\n");
                //ret = regulator_enable(sim_regulator);
                //KRIL_DEBUG(DBG_ERROR," regulator_enable returned 0x%x\n", ret);
                //ret = regulator_disable(sim_regulator);
                //KRIL_DEBUG(DBG_ERROR," regulator_disable returned 0x%x\n", ret);
            }
           break;
        }
        
        default:
        {
            KRIL_DEBUG(DBG_ERROR," unrecognized value for volt %d\n", (int)volt);
            break;
        }
            
    }
    
	memset(&data, 0, sizeof(SYS_ReqRep_t));
	
	if ( simMicroVolts > 0 )
	{
        int ret = regulator_set_voltage(sim_regulator,simMicroVolts,simMicroVolts);
        KRIL_DEBUG(DBG_ERROR," regulator_set_voltage returned %d\n", ret);
        if ( regulator_is_enabled(sim_regulator) > 0 )
        {
            KRIL_DEBUG(DBG_ERROR," regulator already enabled \n");
        }
        else
        {
            ret = regulator_enable(sim_regulator);
            KRIL_DEBUG(DBG_ERROR," regulator_enable returned %d\n", ret);
        }
        gSimInited = TRUE;
	}
	//PMU_ActivateSIM(volt);

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_PMU_ACTIVATE_SIM_RSP, &data);

    return result;
}
/*
// **FIXME** no longer used in CIB???
Result_t Handle_CAPI2_PMU_DeactivateSIM(RPC_Msg_t* pReqMsg)
{
    Result_t result = RESULT_OK;
	SYS_ReqRep_t data;
    KRIL_DEBUG(DBG_ERROR," enter Handle_CAPI2_PMU_DeactivateSIM\n");
    
	memset(&data, 0, sizeof(SYS_ReqRep_t));
    if ( regulator_is_enabled(sim_regulator) > 0 )
    {
        int ret = regulator_disable(sim_regulator);
        KRIL_DEBUG(DBG_ERROR," regulator_disable returned %d\n", ret);
    }
    else
    {
        KRIL_DEBUG(DBG_ERROR," ** regulator not enabled - ignoring\n");
    }

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_PMU_DEACTIVATE_SIM_RSP, &data);
    return result;
}
*/

Result_t Handle_CAPI2_PMU_ClientPowerDown(RPC_Msg_t* pReqMsg)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;
    KRIL_DEBUG(DBG_ERROR," enter Handle_CAPI2_PMU_ClientPowerDown\n");

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	// **FIXME** check with Shameer for equivalent in Android
	//PMU_ClientPowerDown();

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_PMU_ClientPowerDown_RSP, &data);
	return result;
}

// **FIXME** implement this
Result_t Handle_CAPI2_FLASH_SaveImage(RPC_Msg_t* pReqMsg, UInt32 flash_addr, UInt32 length, UInt32 shared_mem_addr)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;
    KRIL_DEBUG(DBG_ERROR," enter Handle_CAPI2_FLASH_SaveImage\n");

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.req_rep_u.CAPI2_FLASH_SaveImage_Rsp.val = TRUE; //(Boolean)FlashSaveData(flash_addr,length,(UInt8*)shared_mem_addr);

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_FLASH_SAVEIMAGE_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_USB_IpcMsg(RPC_Msg_t* pReqMsg, USBPayload_t *val)
{
    Result_t result = RESULT_OK;
    KRIL_DEBUG(DBG_ERROR," enter Handle_CAPI2_USB_IpcMsg\n");
   
    return result;
}

