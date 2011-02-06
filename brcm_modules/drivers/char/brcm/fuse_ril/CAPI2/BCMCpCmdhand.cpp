/*******************************************************************************************
Copyright 2008 Broadcom Corporation.  All rights reserved.
This program is the proprietary software of Broadcom Corporation and/or its licensors, and 
may only be used, duplicated, modified or distributed pursuant to the terms and conditions 
of a separate, written license agreement executed between you and Broadcom 
(an "Authorized License").

Except as set forth in an Authorized License, Broadcom grants no license (express or implied),
right to use, or waiver of any kind with respect to the Software, and Broadcom expressly 
reserves all rights in and to the Software and all intellectual property rights therein.  
IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY 
WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
  
 Except as expressly set forth in the Authorized License,
1. This program, including its structure, sequence and organization, constitutes the 
valuable trade secrets of Broadcom, and you shall use all reasonable efforts to protect the 
confidentiality thereof, and to use this information only in connection with your use of 
Broadcom integrated circuit products.

2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH ALL 
FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, 
STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY 
AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET 
POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE 
OR PERFORMANCE OF THE SOFTWARE.

3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS BE 
LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES 
WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE 
SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY 
AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER 
IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE 
OF ANY LIMITED REMEDY.
*******************************************************************************************/
#include "BCMCpCmdhand.h"
#include "BCMSimLockFun.h"
#include <initguid.h>
#include <gpio.h>
#include <i2c.h>
#include <pmu_bcm59035.h>
#include "rpc_drv.h"

#ifdef __cplusplus
extern "C" {
#endif
extern DBGPARAM dpCurSettings;
#ifdef __cplusplus
};
#endif
bool g_bPmuSimInited = FALSE;

//Operate GPS module GPIO setting for Martini
#define GPS_GPIO_7_PIN          7
#define GPS_GPIO_15_PIN         15
#define GPS_GPIO_18_PIN         18
#define GPS_GPIO_23_PIN         23
#define GPS_GPIO_26_PIN         26
#define GPS_GPIO_56_PIN         56
#define GPS_GPIO_58_PIN         58

//Operate GPS module GPIO setting for Torpedoray
#define GPS_GPIO_25_PIN         25 //0x2000000
#define GPS_GPIO_30_PIN         30 //0x40000000
#define GPS_GPIO_31_PIN         31 //0x80000000
#define GPIO_INVALID            0x00FF

#if 1

#include "bcm2153_base_regs.h"
#include "bcm2153_timers.h"

//*****************************************************************************
//
//                          TIMER_GetValue
//
//  *  This function return the Ticks of Sleep Mode Timer which was used by BCM2153 CP
//  *  We can add the time stamp to the AP log for AP/CP interface debugging
//      Ouput:
//           The register value of   BCM2153_SMT_REGS->SMTCLK
//*****************************************************************************

UInt32 TIMER_GetValue(void)
{ 
    PHYSICAL_ADDRESS pa;
    BCM2153_SMT_REGS *pSmtReg = NULL;
    
    pa.QuadPart = BCM2153_APB_SLEEP_REGS_PA;
    pSmtReg = (BCM2153_SMT_REGS *)MmMapIoSpace(pa, sizeof(BCM2153_SMT_REGS), FALSE);                            
    
    while (TRUE)
    {
        register DWORD a, b;
        
        a =INREG32(&pSmtReg->SMTCLK); 
        b =INREG32(&pSmtReg->SMTCLK); 
        
        if (a == b)
        {  
            MmUnmapIoSpace((PVOID)pSmtReg, sizeof(BCM2153_SMT_REGS));
            return (UInt32)a;
        } 
    }
}   
#define MAX_BUF_SIZE 1024
static char buf[MAX_BUF_SIZE];
int RpcLog_DebugPrintf(char* fmt, ...)
{
	
if(!g_dwLogLEVEL)
	{
		va_list ap;
		va_start(ap, fmt);
		_vsnprintf(buf, MAX_BUF_SIZE, fmt, ap);
		va_end(ap);
		printf("TS[%d]%s",TIMER_GetValue(),buf);
	}


	return 1;
}

#endif

BOOL ReadPMUReg(UINT8 reg_id,UINT8* pWords)
{
    RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: ReadPMUReg++ TS[%d]\r\n",TIMER_GetValue()));
    BOOL rc = FALSE;
    HANDLE hI2C = NULL;

    hI2C = I2COpen(1);
    if(hI2C == NULL)
    {
        goto cleanUp;
    }

    if(!I2CSetSlaveAddress(hI2C, BCM59035_Device_ID))
    {
        RETAILMSG(ZONE_ERROR, (L"BCMCpCmdHand: Error - Initial i2c for Battery begin-allocate memory - I2CSetSlaveAddress == NULL\r\n"));
        goto cleanUp;
    }

    rc = I2CRead(hI2C, reg_id, I2CSUBADDR_8BIT, pWords, 1);
    RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: ReadPMUReg-- \r\n"));
cleanUp:
    I2CClose(hI2C);
    return rc;
}


BOOL WritePMUReg(UINT8 reg_id, UINT8 config)
{
    RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: WritePMUReg++ TS[%d]\r\n",TIMER_GetValue()));
    BOOL rc = FALSE;
    HANDLE hI2C = NULL;

    hI2C = I2COpen(1);
    if(hI2C == NULL)
    {
        goto cleanUp;
    }

    if(!I2CSetSlaveAddress(hI2C, BCM59035_Device_ID))
    {
        RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: Error - Initial i2c for Battery begin-allocate memory - I2CSetSlaveAddress == NULL\r\n"));
        goto cleanUp;
    }

    rc = I2CWrite(hI2C, reg_id, I2CSUBADDR_8BIT, &config, 1);
    RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: WritePMUReg--\r\n"));
cleanUp:
    I2CClose(hI2C);
    return rc;
}


void _DEF(CAPI2_PMU_IsSIMReady)(UInt32 tid, UInt8 clientID)
{
    RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_PMU_IsSIMReady -> return (%d)\r\n", g_bPmuSimInited));
    CAPI2_PMU_IsSIMReady_RSP(tid, clientID, g_bPmuSimInited);
}


void _DEF(CAPI2_PMU_ActivateSIM)(UInt32 tid, UInt8 clientID, Int32 volt)
{
    RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_PMU_ActivateSIM ++, volt is (%d)\r\n", volt));
    BOOL bRet = FALSE;
    UINT8 pWords;

    if(FALSE == ReadPMUReg(PMU_REG_LCSIMDOCTRL, &pWords))
    {
        RETAILMSG(ZONE_ERROR, (L"ERROR:SimVoltCtrl:ReadPMUReg fail\r\n"));
        goto Error;
    }

    // we pre-define three slection ==>BCM_SIM_VOLT_0,     BCM_SIM_VOLT_1V8,   BCM_SIM_VOLT_3V
    switch((PMU_SIMVolt_t)volt)
    {
        case PMU_SIM3P0Volt:
        {
            CLRREG8(&pWords, 0xc0);
            bRet = WritePMUReg(PMU_REG_LCSIMDOCTRL, pWords);
            bRet = WritePMUReg(PMU_REG_SOPMODCTRL, 0x11);
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_PMU_ActivateSIM -> set to 3V\r\n"));
        }
        break;

        case PMU_SIM2P5Volt:
        {
            //CLRREG8(&pWords, 0xc0);
            //bRet = WritePMUReg(PMU_REG_LCSIMDOCTRL, pWords);
            //bRet = WritePMUReg(PMU_REG_SOPMODCTRL, 0x00);
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_PMU_ActivateSIM -> set to 2.5V\r\n"));
        }
        break;

        case PMU_SIM1P8Volt:
        {
            SETREG8(&pWords, 0xc0);
            bRet = WritePMUReg(PMU_REG_LCSIMDOCTRL, pWords);
            bRet = WritePMUReg(PMU_REG_SOPMODCTRL, 0x11);
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_PMU_ActivateSIM -> set to 1.8V\r\n"));
        }
        break;

        case PMU_SIM0P0Volt:
        {
            bRet = WritePMUReg(PMU_REG_SOPMODCTRL, 0xaa);
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_PMU_ActivateSIM -> set to 0V\r\n"));
        }
        break;

        default:
        {
            RETAILMSG(ZONE_ERROR, (L"BCMCpCmdHand: CAPI2_PMU_ActivateSIM -> unknown Volt.\r\n"));
            bRet = FALSE;
        }
    }

    if(bRet)
    {
        g_bPmuSimInited = TRUE;
    }
    else
    {
        RETAILMSG(ZONE_ERROR, (L"BCMCpCmdHand: CAPI2_PMU_ActivateSIM fail.\r\n"));
    }
Error:
    CAPI2_PMU_ActivateSIM_RSP(tid, clientID, bRet);
}

void _DEF(CAPI2_PMU_DeactivateSIM)(UInt32 tid, UInt8 clientID)
{
    RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_PMU_DeactivateSIM ++\r\n"));
    BOOL bRet = FALSE;
    bRet = WritePMUReg(PMU_REG_SOPMODCTRL, 0xaa);

    if(bRet)
    {
        g_bPmuSimInited = TRUE;
    }
    else
    {
        RETAILMSG(ZONE_ERROR, (L"BCMCpCmdHand: CAPI2_PMU_DeactivateSIM fail.\r\n"));
    }

    CAPI2_PMU_DeactivateSIM_RSP(tid, clientID, bRet);
}

void _DEF(CAPI2_GPIO_Set_High_64Pin)(UInt32 tid, UInt8 clientID, UInt32 gpio_pin)
{
    RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_Set_High ++ gpio_pin %d\r\n", gpio_pin));
    BOOL bRet = FALSE;
    DWORD dwGpioSetPin = 0;
    HANDLE hGpio = NULL;

    switch(gpio_pin)
    {
        case GPS_GPIO_7_PIN:
        {
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_Set_High GPS_GPIO_7\r\n"));
            dwGpioSetPin = GPIO_7;
        }
        break;

        case GPS_GPIO_15_PIN:
        {
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_Set_High GPS_GPIO_15\r\n"));
            dwGpioSetPin = GPIO_15;
        }
        break;

        case GPS_GPIO_18_PIN:
        {
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_Set_High GPS_GPIO_18\r\n"));
            dwGpioSetPin = GPIO_18;
        }
        break;

        case GPS_GPIO_23_PIN:
       	{
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_Set_High GPS_GPIO_23\r\n"));
            dwGpioSetPin = GPIO_23;
        }
        break;

        case GPS_GPIO_26_PIN:
       	{
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_Set_High GPS_GPIO_26\r\n"));
            dwGpioSetPin = GPIO_26;
        }
        break;

        case GPS_GPIO_56_PIN:
       	{
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_Set_High GPS_GPIO_56\r\n"));
            dwGpioSetPin = GPIO_56;
        }
        break;

        case GPS_GPIO_58_PIN:
        {
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_Set_High GPS_GPIO_58\r\n"));
            dwGpioSetPin = GPIO_58;
        }
        break;

        case GPS_GPIO_25_PIN:
        {
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_Set_High GPS_GPIO_25\r\n"));
            dwGpioSetPin = GPIO_INVALID;//GPIO_25;
        }
        break;

        case GPS_GPIO_30_PIN:
        {
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_Set_High GPS_GPIO_30\r\n"));
            dwGpioSetPin = GPIO_30;
        }
        break;

#if !defined(BSP_MARTINI_EP5)
        case GPS_GPIO_31_PIN:
        {
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_Set_High GPS_GPIO_31\r\n"));
            dwGpioSetPin = GPIO_31;
        }
        break;
#endif

        default:
            RETAILMSG(ZONE_ERROR, (L"BCMCpCmdHand: CAPI2_GPIO_Set_High unavailable GPIO pin. %d\r\n", gpio_pin));
            goto Error;
	}

    hGpio = GPIOOpen();
    if(NULL == hGpio)
    {
        RETAILMSG(ZONE_ERROR, (L"BCMCpCmdHand: Open GPIO failed.\r\n"));
        goto Error;
    }
    else
    {
        //GPIOSetBit(hGpio, dwGpioSetPin, GPIO_HIGH);
        // wait for somtime then check if the GPIO is set correctly
        Sleep(50);
        if(1)//(GPIO_HIGH == GPIOGetBit(hGpio, dwGpioSetPin))
        {
            bRet = TRUE;
        }
        else
        {
            RETAILMSG(ZONE_ERROR, (L"BCMCpCmdHand: CAPI2_GPIO_Set_HIGH -- fail\r\n"));
        }
    }

    GPIOClose(hGpio);
Error:
    CAPI2_GPIO_Set_High_64Pin_RSP(tid, clientID, bRet);
}


void _DEF(CAPI2_GPIO_Set_Low_64Pin)(UInt32 tid, UInt8 clientID, UInt32 gpio_pin)
{
    RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_Set_Low ++ gpio_pin %d\r\n", gpio_pin));
    BOOL bRet = FALSE;
    DWORD dwGpioSetPin = 0;
    HANDLE hGpio = NULL;

    switch(gpio_pin)
    {
        case GPS_GPIO_7_PIN:
        {
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_Set_Low GPS_GPIO_7\r\n"));
            dwGpioSetPin = GPIO_7;
        }
        break;

        case GPS_GPIO_15_PIN:
        {
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_Set_Low GPS_GPIO_15\r\n"));
            dwGpioSetPin = GPIO_15;
        }
        break;

        case GPS_GPIO_18_PIN:
        {
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_Set_Low GPS_GPIO_18\r\n"));
            dwGpioSetPin = GPIO_18;
        }
        break;

        case GPS_GPIO_23_PIN:
       	{
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_Set_Low GPS_GPIO_23\r\n"));
            dwGpioSetPin = GPIO_23;
        }
        break;

        case GPS_GPIO_26_PIN:
       	{
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_Set_Low GPS_GPIO_26\r\n"));
            dwGpioSetPin = GPIO_26;
        }
        break;

        case GPS_GPIO_56_PIN:
       	{
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_Set_Low GPS_GPIO_56\r\n"));
            dwGpioSetPin = GPIO_56;
        }
        break;

        case GPS_GPIO_58_PIN:
        {
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_Set_Low GPS_GPIO_58\r\n"));
            dwGpioSetPin = GPIO_58;
        }
        break;

        case GPS_GPIO_25_PIN:
        {
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_Set_Low GPS_GPIO_25\r\n"));
            dwGpioSetPin = GPIO_INVALID;//GPIO_25;
        }
        break;

        case GPS_GPIO_30_PIN:
        {
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_Set_Low GPS_GPIO_30\r\n"));
            dwGpioSetPin = GPIO_30;
        }
        break;

#if !defined(BSP_MARTINI_EP5)
        case GPS_GPIO_31_PIN:
        {
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_Set_Low GPS_GPIO_31\r\n"));
            dwGpioSetPin = GPIO_31;
        }
        break;
#endif

        default:
            RETAILMSG(ZONE_ERROR, (L"BCMCpCmdHand: CAPI2_GPIO_Set_Low unavailable GPIO pin. %d\r\n", gpio_pin));
            goto Error;
    }

    hGpio = GPIOOpen();
    if(NULL == hGpio)
    {
        RETAILMSG(ZONE_ERROR, (L"BCMCpCmdHand: Open GPIO failed.\r\n"));
        goto Error;
    }
    else
    {
        //GPIOSetBit(hGpio, dwGpioSetPin, GPIO_LOW);
        // wait for somtime then check if the GPIO is set correctly
        Sleep(50);
        if (1)//(GPIO_LOW == GPIOGetBit(hGpio, dwGpioSetPin))
        {
            bRet = TRUE;
        }
        else
        {
            RETAILMSG(ZONE_ERROR, (L"BCMCpCmdHand: CAPI2_GPIO_Set_Low -- fail\r\n"));
        }
    }

    GPIOClose(hGpio);
Error:
    CAPI2_GPIO_Set_Low_64Pin_RSP(tid, clientID, bRet);
}

void _DEF(CAPI2_PMU_StartCharging)(UInt32 tid, UInt8 clientID)
{
    CAPI2_PMU_StartCharging_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_PMU_StopCharging)(UInt32 tid, UInt8 clientID)
{
    CAPI2_PMU_StopCharging_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SMS_IsMeStorageEnabled)(UInt32 tid, UInt8 clientID)
{
    CAPI2_SMS_IsMeStorageEnabled_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SMS_GetMaxMeCapacity)(UInt32 tid, UInt8 clientID)
{
    CAPI2_SMS_GetMaxMeCapacity_RSP(tid, clientID, 10);
}

void _DEF(CAPI2_SMS_GetNextFreeSlot)(UInt32 tid, UInt8 clientID)
{
    CAPI2_SMS_GetNextFreeSlot_RSP(tid, clientID, 0);
}

void _DEF(CAPI2_SMS_SetMeSmsStatus)(UInt32 tid, UInt8 clientID, UInt16 slotNumber, SIMSMSMesgStatus_t status)
{
    CAPI2_SMS_SetMeSmsStatus_RSP(tid, clientID, FALSE);
}

void _DEF(CAPI2_SMS_GetMeSmsStatus)(UInt32 tid, UInt8 clientID, UInt16 slotNumber)
{
    CAPI2_SMS_GetMeSmsStatus_RSP(tid, clientID, SIMSMSMESGSTATUS_FREE);
}

void _DEF(CAPI2_SMS_StoreSmsToMe)(UInt32 tid, UInt8 clientID, UInt8 *inSms, UInt16 inLength, SIMSMSMesgStatus_t status, UInt16 slotNumber)
{
    CAPI2_SMS_StoreSmsToMe_RSP(tid, clientID, FALSE);
}

void _DEF(CAPI2_SMS_RetrieveSmsFromMe)(UInt32 tid, UInt8 clientID, UInt16 slotNumber)
{
    CAPI2_SMS_RetrieveSmsFromMe_RSP(tid, clientID, FALSE);
}

void _DEF(CAPI2_SMS_RemoveSmsFromMe)(UInt32 tid, UInt8 clientID, UInt16 slotNumber)
{
    CAPI2_SMS_RemoveSmsFromMe_RSP(tid, clientID, FALSE);
}

void _DEF(CAPI2_SMS_GetMeSmsBufferStatus)(UInt32 tid, UInt8 clientID, UInt16 cmd)
{
    CAPI2_SMS_GetMeSmsBufferStatus_RSP(tid, clientID, 0, 0);
}

void _DEF(CAPI2_PMU_GetPowerupCause)(UInt32 tid, UInt8 clientID)
{
    CAPI2_PMU_GetPowerupCause_RSP(tid, clientID, PMU_POWERUP_POWERKEY);
}

void _DEF(CAPI2_PMU_ClientPowerDown)(UInt32 tid, UInt8 clientID)
{
    CAPI2_PMU_ClientPowerDown_RSP(tid, clientID, FALSE);
}

void _DEF(CAPI2_SOCKET_Open)(UInt32 tid, UInt8 clientID, UInt8 domain, UInt8 type, UInt8 protocol)
{
    CAPI2_SOCKET_Open_RSP(tid, clientID, 10);
}

void _DEF(CAPI2_SOCKET_Bind)(UInt32 tid, UInt8 clientID, Int32 descriptor, sockaddr* addr)
{
    CAPI2_SOCKET_Bind_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SOCKET_Listen)(UInt32 tid, UInt8 clientID, Int32 descriptor, UInt32 backlog)
{
    CAPI2_SOCKET_Listen_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SOCKET_Accept)(UInt32 tid, UInt8 clientID, Int32 descriptor)
{
    //CAPI2_SOCKET_Accept_RSP(tid, clientID, NULL);
}

void _DEF(CAPI2_SOCKET_Connect)(UInt32 tid, UInt8 clientID, Int32 descriptor, sockaddr* name)
{
    CAPI2_SOCKET_Connect_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SOCKET_GetPeerName)(UInt32 tid, UInt8 clientID, Int32 descriptor)
{
    //CAPI2_SOCKET_GetPeerName_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SOCKET_GetSockName)(UInt32 tid, UInt8 clientID, Int32 descriptor)
{
    //CAPI2_SOCKET_GetSockName_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SOCKET_SetSockOpt)(UInt32 tid, UInt8 clientID, Int32 descriptor, UInt16 optname, SockOptVal_t* optval)
{
    CAPI2_SOCKET_SetSockOpt_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SOCKET_GetSockOpt)(UInt32 tid, UInt8 clientID, Int32 descriptor, UInt16 optname)
{
    //SocketGetSockOptRsp_t val;
    CAPI2_SOCKET_GetSockOpt_RSP(tid, clientID, NULL, 0);
}

void _DEF(CAPI2_SOCKET_Send)(UInt32 tid, UInt8 clientID, SocketSendReq_t* sockSendReq)
{
    //CAPI2_SOCKET_Send_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SOCKET_SendTo)(UInt32 tid, UInt8 clientID, SocketSendReq_t* sockSendReq, sockaddr* to)
{
    //CAPI2_SOCKET_SendTo_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SOCKET_Recv)(UInt32 tid, UInt8 clientID, SocketRecvReq_t* sockRecvReq)
{
    //CAPI2_SOCKET_Recv_RSP(tid, clientID, -1);
}

void _DEF(CAPI2_SOCKET_RecvFrom)(UInt32 tid, UInt8 clientID, SocketRecvReq_t *sockRecvReq, sockaddr *from)
{
    //CAPI2_SOCKET_RecvFrom_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SOCKET_Close)(UInt32 tid, UInt8 clientID, Int32 descriptor)
{
    CAPI2_SOCKET_Close_RSP(tid, clientID, 0);
}

void _DEF(CAPI2_SOCKET_Shutdown)(UInt32 tid, UInt8 clientID, Int32 descriptor, UInt8 how)
{
    CAPI2_SOCKET_Shutdown_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SOCKET_Errno)(UInt32 tid, UInt8 clientID, Int32 descriptor)
{
    CAPI2_SOCKET_Errno_RSP(tid, clientID, 0);
}

void _DEF(CAPI2_SOCKET_SO2LONG)(UInt32 tid, UInt8 clientID, Int32 socket)
{
    CAPI2_SOCKET_SO2LONG_RSP(tid, clientID, 0);
}


void _DEF(CAPI2_SOCKET_GetSocketSendBufferSpace)(UInt32 tid, UInt8 clientID, Int32 bufferSpace)
{
    CAPI2_SOCKET_GetSocketSendBufferSpace_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DC_SetupDataConnection)(UInt32 tid, UInt8 clientID, UInt8 inClientID, UInt8 acctID, DC_ConnectionType_t linkType)
{
    CAPI2_DC_SetupDataConnection_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DC_SetupDataConnectionEx)(UInt32 tid, UInt8 clientID, UInt8 inClientID, UInt8 acctID, DC_ConnectionType_t linkType, uchar_ptr_t apnCheck, uchar_ptr_t actDCAcctId)
{
    CAPI2_DC_SetupDataConnectionEx_RSP(tid, clientID, 0, 0);
}

void _DEF(CAPI2_DC_ShutdownDataConnection)(UInt32 tid, UInt8 clientID, UInt8 inClientID, UInt8 acctID)
{
    CAPI2_DC_ShutdownDataConnection_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_IsAcctIDValid)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_IsAcctIDValid_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_CreateGPRSDataAcct)(UInt32 tid, UInt8 clientID, UInt8 acctID, GPRSContext_t *pGprsSetting)
{
    CAPI2_DATA_CreateGPRSDataAcct_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_CreateCSDDataAcct)(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDContext_t *pCsdSetting)
{
    CAPI2_DATA_CreateCSDDataAcct_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_DeleteDataAcct)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_DeleteDataAcct_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_SetUsername)(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t username)
{
    CAPI2_DATA_SetUsername_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetUsername)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetUsername_RSP(tid, clientID, NULL);
}

void _DEF(CAPI2_DATA_SetPassword)(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t password)
{
    CAPI2_DATA_SetPassword_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetPassword)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetPassword_RSP(tid, clientID, NULL);
}

void _DEF(CAPI2_DATA_SetStaticIPAddr)(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t staticIPAddr)
{
    CAPI2_DATA_SetStaticIPAddr_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetStaticIPAddr)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetStaticIPAddr_RSP(tid, clientID, NULL);
}

void _DEF(CAPI2_DATA_SetPrimaryDnsAddr)(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t priDnsAddr)
{
    CAPI2_DATA_SetPrimaryDnsAddr_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetPrimaryDnsAddr)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetPrimaryDnsAddr_RSP(tid, clientID, NULL);
}

void _DEF(CAPI2_DATA_SetSecondDnsAddr)(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t sndDnsAddr)
{
    CAPI2_DATA_SetSecondDnsAddr_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetSecondDnsAddr)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetSecondDnsAddr_RSP(tid, clientID, NULL);
}

void _DEF(CAPI2_DATA_SetDataCompression)(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean dataCompEnable)
{
    CAPI2_DATA_SetDataCompression_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetDataCompression)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetDataCompression_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetAcctType)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetAcctType_RSP(tid, clientID, DATA_ACCOUNT_NOT_USED);
}

void _DEF(CAPI2_DATA_GetEmptyAcctSlot)(UInt32 tid, UInt8 clientID)
{
    CAPI2_DATA_GetEmptyAcctSlot_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetCidFromDataAcctID)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetCidFromDataAcctID_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetDataAcctIDFromCid)(UInt32 tid, UInt8 clientID, UInt8 contextID)
{
    CAPI2_DATA_GetDataAcctIDFromCid_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetPrimaryCidFromDataAcctID)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetPrimaryCidFromDataAcctID_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_IsSecondaryDataAcct)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_IsSecondaryDataAcct_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetDataSentSize)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetDataSentSize_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetDataRcvSize)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetDataRcvSize_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_SetGPRSPdpType)(UInt32 tid, UInt8 clientID, UInt8 acctID, UInt8 *pdpType)
{
    CAPI2_DATA_SetGPRSPdpType_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetGPRSPdpType)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetGPRSPdpType_RSP(tid, clientID, NULL);
}

void _DEF(CAPI2_DATA_SetGPRSApn)(UInt32 tid, UInt8 clientID, UInt8 acctID, UInt8 *apn)
{
    CAPI2_DATA_SetGPRSApn_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetGPRSApn)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetGPRSApn_RSP(tid, clientID, NULL);
}

void _DEF(CAPI2_DATA_SetAuthenMethod)(UInt32 tid, UInt8 clientID, UInt8 acctID, DataAuthenMethod_t authenMethod)
{
    CAPI2_DATA_SetAuthenMethod_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetAuthenMethod)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetAuthenMethod_RSP(tid, clientID, DATA_DEFAULT_AUTHEN_TYPE);
}

void _DEF(CAPI2_DATA_SetGPRSHeaderCompression)(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean headerCompEnable)
{
    CAPI2_DATA_SetGPRSHeaderCompression_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetGPRSHeaderCompression)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetGPRSHeaderCompression_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_SetGPRSQos)(UInt32 tid, UInt8 clientID, UInt8 acctID, PCHQosProfile_t qos)
{
    CAPI2_DATA_SetGPRSQos_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetGPRSQos)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    //CAPI2_DATA_GetGPRSQos_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_SetAcctLock)(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean acctLock)
{
    CAPI2_DATA_SetAcctLock_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetAcctLock)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetAcctLock_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_SetGprsOnly)(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean gprsOnly)
{
    CAPI2_DATA_SetGprsOnly_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetGprsOnly)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetGprsOnly_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_SetGPRSTft)(UInt32 tid, UInt8 clientID, UInt8 acctID, PCHTrafficFlowTemplate_t *pTft)
{
    CAPI2_DATA_SetGPRSTft_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetGPRSTft)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    //CAPI2_DATA_GetGPRSTft_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_SetCSDDialNumber)(UInt32 tid, UInt8 clientID, UInt8 acctID, UInt8 *dialNumber)
{
    CAPI2_DATA_SetCSDDialNumber_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetCSDDialNumber)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetCSDDialNumber_RSP(tid, clientID, NULL);
}

void _DEF(CAPI2_DATA_SetCSDDialType)(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDDialType_t csdDialType)
{
    CAPI2_DATA_SetCSDDialType_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetCSDDialType)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetCSDDialType_RSP(tid, clientID, DATA_DEFAULT_DIAL_TYPE);
}

void _DEF(CAPI2_DATA_SetCSDBaudRate)(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDBaudRate_t csdBaudRate)
{
    CAPI2_DATA_SetCSDBaudRate_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetCSDBaudRate)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetCSDBaudRate_RSP(tid, clientID, DATA_DEFAULT_BAUDRATE);
}

void _DEF(CAPI2_DATA_SetCSDSyncType)(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDSyncType_t csdSyncType)
{
    CAPI2_DATA_SetCSDSyncType_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetCSDSyncType)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetCSDSyncType_RSP(tid, clientID, DATA_DEFAULT_SYNCTYPE);
}

void _DEF(CAPI2_DATA_SetCSDErrorCorrection)(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean enable)
{
    CAPI2_DATA_SetCSDErrorCorrection_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetCSDErrorCorrection)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetCSDErrorCorrection_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_SetCSDErrCorrectionType)(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDErrCorrectionType_t errCorrectionType)
{
    CAPI2_DATA_SetCSDErrCorrectionType_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetCSDErrCorrectionType)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetCSDErrCorrectionType_RSP(tid, clientID, DATA_DEFAULT_EC_TYPE);
}

void _DEF(CAPI2_DATA_SetCSDDataCompType)(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDDataCompType_t dataCompType)
{
    CAPI2_DATA_SetCSDDataCompType_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetCSDDataCompType)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetCSDDataCompType_RSP(tid, clientID, DATA_DEFAULT_DC_TYPE);
}

void _DEF(CAPI2_DATA_SetCSDConnElement)(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDConnElement_t connElement)
{
    CAPI2_DATA_SetCSDConnElement_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_DATA_GetCSDConnElement)(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
    CAPI2_DATA_GetCSDConnElement_RSP(tid, clientID, DATA_DEFAULT_CONN_ELEMENT);
}

void _DEF(CAPI2_DATA_UpdateAccountToFileSystem)(UInt32 tid, UInt8 clientID)
{
    CAPI2_DATA_UpdateAccountToFileSystem_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_resetDataSize)(UInt32 tid, UInt8 clientID, UInt8 cid)
{
    CAPI2_resetDataSize_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_addDataSentSizebyCid)(UInt32 tid, UInt8 clientID, UInt8 cid, UInt32 size)
{
    CAPI2_addDataSentSizebyCid_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_addDataRcvSizebyCid)(UInt32 tid, UInt8 clientID, UInt8 cid, UInt32 size)
{
    CAPI2_addDataRcvSizebyCid_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_GPIO_ConfigOutput_64Pin)(UInt32 tid, UInt8 clientID, UInt32 pin)
{
    RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_ConfigOutput ++\r\n"));
    BOOL bRet = FALSE;
    DWORD dwGpioConfigPin = 0;
    HANDLE hGpio = NULL;

    switch(pin)
    {
        case GPS_GPIO_7_PIN:
        {
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_ConfigOutput GPS_GPIO_7\r\n"));
            dwGpioConfigPin = GPIO_7;
        }
        break;

        case GPS_GPIO_15_PIN:
        {
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_ConfigOutput GPS_GPIO_15\r\n"));
            dwGpioConfigPin = GPIO_15;
        }
        break;

        case GPS_GPIO_18_PIN:
        {
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_ConfigOutput GPS_GPIO_18\r\n"));
            dwGpioConfigPin = GPIO_18;
        }
        break;

        case GPS_GPIO_23_PIN:
       	{
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_ConfigOutput GPS_GPIO_23\r\n"));
            dwGpioConfigPin = GPIO_23;
        }
        break;

        case GPS_GPIO_26_PIN:
       	{
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_ConfigOutput GPS_GPIO_26\r\n"));
            dwGpioConfigPin = GPIO_26;
        }
        break;

        case GPS_GPIO_56_PIN:
       	{
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_ConfigOutput GPS_GPIO_56\r\n"));
            dwGpioConfigPin = GPIO_56;
        }
        break;

        case GPS_GPIO_58_PIN:
        {
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_ConfigOutput GPS_GPIO_58\r\n"));
            dwGpioConfigPin = GPIO_58;
        }
        break;

        case GPS_GPIO_25_PIN:
        {
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_ConfigOutput GPS_GPIO_25\r\n"));
            dwGpioConfigPin = GPIO_INVALID;//GPIO_25;
        }
        break;

        case GPS_GPIO_30_PIN:
        {
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_ConfigOutput GPS_GPIO_30\r\n"));
            dwGpioConfigPin = GPIO_30;
        }
        break;

#if !defined(BSP_MARTINI_EP5)
        case GPS_GPIO_31_PIN:
        {
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPIO_ConfigOutput GPS_GPIO_31\r\n"));
            dwGpioConfigPin = GPIO_31;
        }
        break;
#endif

        default:
            RETAILMSG(ZONE_ERROR, (L"BCMCpCmdHand: CAPI2_GPIO_ConfigOutput unavailable GPIO pin. %d\r\n", pin));
            goto Error;
    }

    hGpio = GPIOOpen();
    if(NULL == hGpio)
    {
        RETAILMSG(ZONE_ERROR, (L"BCMCpCmdHand: Open GPIO failed.\r\n"));
        goto Error;
    }
    else
    {
        //GPIOSetMode(hGpio, dwGpioConfigPin, GPIO_DIR_OUTPUT);
        bRet = TRUE;
    }

    GPIOClose(hGpio);
Error:
    CAPI2_GPIO_ConfigOutput_64Pin_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_GPS_Control)(UInt32 tid, UInt8 clientID, UInt32 u32Cmnd, UInt32 u32Param0, UInt32 u32Param1, UInt32 u32Param2, UInt32 u32Param3, UInt32 u32Param4)
{
    // Call gps function here
    HAL_GPS_Result_en_t HAL_GPS_Result = HAL_GPS_PASS;
    PHYSICAL_ADDRESS pa;
    BCM2153_SYS_REGS *pSysReg = NULL;
    pa.QuadPart = BCM2153_APB_SYSCFG_REGS_PA;

    switch(u32Cmnd)
    {
        case CP2AP_GPS_CONTROL_SET_PORT_TO_NULL :
        {
            //We have set how UART-Port is used when eboot, so we don't need to access this case
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPS_Control: CP2AP_GPS_CONTROL_SET_PORT_TO_NULL <== WinMo no need process\r\n"));
            break;
        }
        case CP2AP_GPS_CONTROL_CONFIG_RFGPEN_AS_GPIO :
        {
            RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPS_Control: CP2AP_GPS_CONTROL_CONFIG_RFGPEN_AS_GPIO: rfgpen_as_gpio %d\r\n", u32Param0));
            pSysReg = (BCM2153_SYS_REGS *)MmMapIoSpace(pa, sizeof(BCM2153_SYS_REGS), FALSE);
            if (NULL == pSysReg)
            {
                RETAILMSG(ZONE_ERROR, (L"BCMCpCmdHand: CAPI2_GPS_Control: Fail to set IOREG\r\n"));
                HAL_GPS_Result = HAL_GPS_FAIL;
                break;
            }

            switch (u32Param0) // rfgpen_as_gpio
            {
                case 60 :
                    //*(volatile UInt32  *)IOCR0_REG &= ~(BIT26_MASK | BIT25_MASK);
                    CLRREG32(&pSysReg->IOCR0, (BIT26_MASK | BIT25_MASK));
                    break;
                case 58 :
                    //*(volatile UInt32  *)IOCR0_REG &= ~(BIT22_MASK | BIT19_MASK);
                    CLRREG32(&pSysReg->IOCR0, (BIT22_MASK | BIT19_MASK));
                    break;
                case 57 :
                    //*(volatile UInt32  *)IOCR0_REG &= ~(BIT21_MASK | BIT18_MASK);
                    CLRREG32(&pSysReg->IOCR0, (BIT21_MASK | BIT18_MASK));
                    break;
                case 56 :
                    //*(volatile UInt32  *)IOCR0_REG &= ~(BIT20_MASK | BIT17_MASK);
                    CLRREG32(&pSysReg->IOCR0, (BIT20_MASK | BIT17_MASK));
                    break;
                case 55 :
                    //*(volatile UInt32  *)IOCR0_REG |= (BIT15_MASK | BIT16_MASK);
                    CLRREG32(&pSysReg->IOCR0, (BIT15_MASK | BIT16_MASK));
                    break;
                case 54 :
                    //*(volatile UInt32  *)IOCR0_REG |= (BIT27_MASK | BIT14_MASK);
                    CLRREG32(&pSysReg->IOCR0, (BIT27_MASK | BIT14_MASK));
                    break;
                default:
                    HAL_GPS_Result = HAL_GPS_FAIL;
                    break;
            }
            MmUnmapIoSpace((PVOID)pSysReg, sizeof(BCM2153_SYS_REGS));
            break;
        }

        default:
            RETAILMSG(ZONE_ERROR, (L"BCMCpCmdHand: CAPI2_GPS_Control: default %d\r\n", u32Cmnd));
            HAL_GPS_Result = HAL_GPS_FAIL;
            break;

    } // switch(u32Cmnd)
    RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_GPS_Control GPS_RESULT:%d\r\n", HAL_GPS_Result));
    CAPI2_GPS_Control_RSP(tid, clientID, HAL_GPS_Result);
}

void _DEF(CAPI2_FFS_Control)(UInt32 tid, UInt8 clientID, UInt32 cmd, UInt32 address, UInt32 offset, UInt32 size)
{
    IPC_PersistentDataStore_t *thePersistentData = NULL;
    HANDLE   hFileTmp = NULL;
    DWORD    dwBytes;
    DWORD    dwFileSize;
    DWORD    dwError;
    char*    chTempBuf = NULL;
    FFS_Control_Result_en_t FFS_Control_Result = FFS_CONTROL_PASS;

    switch (cmd)
    {
        case FFS_CONTROL_COPY_NVS_FILE_TO_SM: // copy the NVS file to shared memory
        {
            RETAILMSG(ZONE_FUNCTION ,(L"BCMCpCmdHand: CAPI2_FFS_Control: (CopyFileToPDS-Off=%d,Size=%d) TS[%d] \r\n", offset, size,TIMER_GetValue()));

            hFileTmp = CreateFile(TEXT("\\BRCM_PersistentData.txt"), GENERIC_READ, FILE_SHARE_READ, NULL,/* No security*/
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

            if (INVALID_HANDLE_VALUE != hFileTmp)
            {
                thePersistentData = (IPC_PersistentDataStore_t*)malloc(sizeof(IPC_PersistentDataStore_t));
                if(!thePersistentData)
                {
                    FFS_Control_Result = FFS_CONTROL_IPCAP_PDS_OP_FAIL;
                    goto cleanUp;
                }

                RPC_GetPersistentData(thePersistentData);

                if ((offset >= thePersistentData->DataLength) || 
                    (size > thePersistentData->DataLength) || (!size) || 
                    ((offset + size) > thePersistentData->DataLength))
                {
                    FFS_Control_Result = FFS_CONTROL_INVALID_PARAM;
                    RETAILMSG(ZONE_ERROR, (L"BCMCpCmdHand: CAPI2_FFS_Control: Parameter ERROR! Persist Data Len %d\r\n", thePersistentData->DataLength));
                    goto cleanUp;
                }

                dwFileSize = GetFileSize(hFileTmp, NULL);
                // Result on failure.
                if (0xFFFFFFFF == dwFileSize)
                {
                    dwError = GetLastError();
                    FFS_Control_Result = FFS_CONTROL_FILE_OP_FAIL;
                    RETAILMSG(ZONE_ERROR, (L"BCMCpCmdHand: CAPI2_FFS_Control: Read get persistent data file size fail 0x%x\r\n", dwError));
                    goto cleanUp;
                }

                if (dwFileSize != thePersistentData->DataLength)
                {
                    chTempBuf = (char*)malloc(thePersistentData->DataLength);
                    if (NULL == chTempBuf)
                    {
                        FFS_Control_Result = FFS_CONTROL_FILE_OP_FAIL;
                        RETAILMSG(ZONE_ERROR, (L"BCMCpCmdHand: CAPI2_FFS_Control: Allocate zero buffer fail\r\n"));
                        goto cleanUp;
                    }
                    memset(chTempBuf, 0, thePersistentData->DataLength);

                    CloseHandle(hFileTmp);
                    hFileTmp = CreateFile( TEXT("\\BRCM_PersistentData.txt"), GENERIC_WRITE,
                        FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

                    //Create the file in the Flash file system and init with zero
                    if(WriteFile(hFileTmp, chTempBuf, thePersistentData->DataLength, &dwBytes, NULL))
                    {
                        RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_FFS_Control: write zero data to FS ok\r\n"));
                        CloseHandle(hFileTmp);
                        hFileTmp = CreateFile(TEXT("\\BRCM_PersistentData.txt"), GENERIC_READ, FILE_SHARE_READ, NULL,/* No security*/
                                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                    }
                    else
                    {
                        FFS_Control_Result = FFS_CONTROL_FILE_OP_FAIL;;
                        RETAILMSG(ZONE_ERROR, (L"BCMCpCmdHand: CAPI2_FFS_Control: write zero data to FS fail\r\n"));
                        goto cleanUp;
                    }
                }

                if(ReadFile(hFileTmp, thePersistentData->DataPtr, thePersistentData->DataLength, &dwBytes, NULL))
                {
                    RETAILMSG(ZONE_FUNCTION, (L"BCMCpCmdHand: CAPI2_FFS_Control: read persistent data from file and write to shared memory ok\r\n"));
                }
                else
                {
                    FFS_Control_Result = FFS_CONTROL_IPCAP_PDS_OP_FAIL;
                    RETAILMSG(ZONE_ERROR, (L"BCMCpCmdHand: CAPI2_FFS_Control: read persistent data from file and write to shared memory fail\r\n"));
                    goto cleanUp;
                }

            }
            else
            {
                FFS_Control_Result = FFS_CONTROL_FILE_OP_FAIL;
                RETAILMSG(ZONE_ERROR,(L"BCMCpCmdHand: CAPI2_FFS_Control: open persistent data file fail \r\n"));
            }
        } // FFS_CONTROL_COPY_NVS_FILE_TO_SM
        break; // switch (cmd)

        case FFS_CONTROL_COPY_SM_TO_NVS_FILE: // copy shared memory to the NVS file
        {
            RETAILMSG(ZONE_FUNCTION,(L"BCMCpCmdHand: CAPI2_FFS_Control: (CopyPDS_ToFile-Off=%d,Size=%d) TS[%d]\r\n", offset, size,TIMER_GetValue()));

            hFileTmp = CreateFile( TEXT("\\BRCM_PersistentData.txt"), GENERIC_WRITE,
                        FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

            if (hFileTmp != INVALID_HANDLE_VALUE)
            {
                thePersistentData = (IPC_PersistentDataStore_t*)malloc(sizeof(IPC_PersistentDataStore_t));
                if (!thePersistentData)
                {
                    FFS_Control_Result = FFS_CONTROL_IPCAP_PDS_OP_FAIL;
                    goto cleanUp;
                }

                RPC_GetPersistentData(thePersistentData);

                if ((offset >= thePersistentData->DataLength) || 
                    (size > thePersistentData->DataLength) || (!size) || 
                    ((offset + size) > thePersistentData->DataLength))
                {
                    FFS_Control_Result = FFS_CONTROL_INVALID_PARAM;
                    RETAILMSG(ZONE_FUNCTION,(L"BCMCpCmdHand: CAPI2_FFS_Control: Parameter ERROR! Persist Data Len %d \r\n", thePersistentData->DataLength));
                    goto cleanUp;
                }

                if(WriteFile(hFileTmp, thePersistentData->DataPtr, thePersistentData->DataLength, &dwBytes, NULL))
                {
                    RETAILMSG(ZONE_FUNCTION,(L"BCMCpCmdHand: CAPI2_FFS_Control: write persistent data to FS ok \r\n"));
                }
                else
                {
                    FFS_Control_Result = FFS_CONTROL_IPCAP_PDS_OP_FAIL;
                    RETAILMSG(ZONE_ERROR,(L"BCMCpCmdHand: CAPI2_FFS_Control: write persistent data to FS fail \r\n"));
                }

            }
            else
            {
                FFS_Control_Result = FFS_CONTROL_FILE_OP_FAIL;
                RETAILMSG(ZONE_ERROR,(L"BCMCpCmdHand: CAPI2_FFS_Control: create persistent data file fail \r\n"));
            }
        } // FFS_CONTROL_COPY_SM_TO_NVS_FILE
        break; // switch (cmd)

        default:
        {
            FFS_Control_Result = FFS_CONTROL_UNKNOWN_CMD;
        }
        break; // switch (cmd)

    } // switch (cmd)

cleanUp:

    if (hFileTmp)
        CloseHandle(hFileTmp);
    if (thePersistentData)
        free(thePersistentData);
    if (chTempBuf)
        free(chTempBuf);

    CAPI2_FFS_Control_RSP(tid, clientID, FFS_Control_Result);
}

void _DEF(CAPI2_CP2AP_PedestalMode_Control)(UInt32 tid, UInt8 clientID, UInt32 enable)
{
    // Call pedestal mode function here
    CAPI2_CP2AP_PedestalMode_Control_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_SOCKET_ParseIPAddr)(UInt32 tid, UInt8 clientID, char_ptr_t ipString)
{

}

void _DEF(CAPI2_RTC_SetTime)(UInt32 tid, UInt8 clientID, RTCTime_t *inTime)
{
    CAPI2_RTC_SetTime_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_RTC_SetDST)(UInt32 tid, UInt8 clientID, UInt8 inDST)
{
    CAPI2_RTC_SetDST_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_RTC_SetTimeZone)(UInt32 tid, UInt8 clientID, Int8 inTimezone)
{
    CAPI2_RTC_SetTimeZone_RSP(tid, clientID, TRUE);
}

void _DEF(CAPI2_RTC_GetTime)(UInt32 tid, UInt8 clientID, RTCTime_t *time)
{
    SYSTEMTIME system_time;
    RTCTime_t  rtc_time;
    
    GetLocalTime(&system_time);
    RETAILMSG(ZONE_FUNCTION,(L"GetLocalTime: wMilliseconds:%d wSecond:%d wMinute:%d wHour:%d wDay:%d wDayOfWeek:%d wMonth:%d wYear:%d\r\n", 
        system_time.wMilliseconds, system_time.wSecond, system_time.wMinute, system_time.wHour, system_time.wDay, system_time.wDayOfWeek, system_time.wMonth, system_time.wYear));
    rtc_time.Sec   = (UInt8)system_time.wSecond;
    rtc_time.Min   = (UInt8)system_time.wMinute;
    rtc_time.Hour  = (UInt8)system_time.wHour;
    rtc_time.Day   = (UInt8)system_time.wDay;
    rtc_time.Week  = (UInt8)system_time.wDayOfWeek;
    rtc_time.Month = (UInt8)system_time.wMonth;
    rtc_time.Year  = (UInt16)system_time.wYear;
    
    CAPI2_RTC_GetTime_RSP(tid, clientID, rtc_time);
}

void _DEF(CAPI2_RTC_GetTimeZone)(UInt32 tid, UInt8 clientID)
{
    Int8 timeZone = 0;
    TIME_ZONE_INFORMATION time_zone;
    
    if(GetTimeZoneInformation(&time_zone) != TIME_ZONE_ID_UNKNOWN)
    {
        RETAILMSG(ZONE_FUNCTION,(L"GetTimeZoneInformation: Bias:%d\r\n", time_zone.Bias));
        timeZone = (Int8)time_zone.Bias;
        timeZone = (0 - timeZone)*4/60;
    }
    else
    {
        RETAILMSG(ZONE_ERROR,(L"GetTimeZoneInformation: TIME_ZONE_ID_UNKNOWN\r\n"));
    }
    
    CAPI2_RTC_GetTimeZone_RSP(tid, clientID, timeZone);
}

void _DEF(CAPI2_FLASH_SaveImage)(UInt32 tid, UInt8 clientID, UInt32 flash_addr, UInt32 length, UInt32 shared_mem_addr)
{
    CAPI2_FLASH_SaveImage_RSP(tid, clientID,  TRUE);
}

void _DEF(CAPI2_SIMLOCK_GetStatus)(UInt32 tid, UInt8 clientID, SIMLOCK_SIM_DATA_t *sim_data)
{

    SIMLOCK_STATE_t simlock_state;
    SIMLOCK_SIM_DATA_t* temp = sim_data;
    BOOL bEnableLock = FALSE;

    simlock_state.network_lock = SIM_SECURITY_OPEN; 
    simlock_state.network_subset_lock = SIM_SECURITY_OPEN;
    simlock_state.service_provider_lock = SIM_SECURITY_OPEN;
    simlock_state.corporate_lock = SIM_SECURITY_OPEN;
    simlock_state.phone_lock = SIM_SECURITY_OPEN;
    #if 0
    // Get current SIM lock stae for CP initial
    if(BRCM_SimLock_GetLockState(RIL_LOCKFACILITY_PH_SIM, bEnableLock))
    {
        simlock_state.phone_lock = (bEnableLock ? SIM_SECURITY_LOCKED : SIM_SECURITY_OPEN);
    }
    if(BRCM_SimLock_GetLockState(RIL_LOCKFACILITY_NETWORKPERS, bEnableLock))
    {
        simlock_state.network_lock = (bEnableLock ? SIM_SECURITY_LOCKED : SIM_SECURITY_OPEN);
    }
    if(BRCM_SimLock_GetLockState(RIL_LOCKFACILITY_NETWORKSUBPERS, bEnableLock))
    {
        simlock_state.network_subset_lock = (bEnableLock ? SIM_SECURITY_LOCKED : SIM_SECURITY_OPEN);
    }
    if(BRCM_SimLock_GetLockState(RIL_LOCKFACILITY_SERVICEPROVPERS, bEnableLock))
    {
        simlock_state.service_provider_lock = (bEnableLock ? SIM_SECURITY_LOCKED : SIM_SECURITY_OPEN);
    }
    if(BRCM_SimLock_GetLockState(RIL_LOCKFACILITY_CORPPERS, bEnableLock))
    {
        simlock_state.corporate_lock = (bEnableLock ? SIM_SECURITY_LOCKED : SIM_SECURITY_OPEN);
    }
    RETAILMSG(ZONE_FUNCTION,(L"CAPI2_SIMLOCK_GetStatus_RSP -> current simlock_state in registry, PN(%d), PU(%d), PP(%d), PC(%d), PS(%d).\r\n", simlock_state.network_lock, simlock_state.network_subset_lock, simlock_state.service_provider_lock, simlock_state.corporate_lock, simlock_state.phone_lock));
    // Check if this SIM should be lock
    BRCM_SimLock_CheckLockState(&simlock_state, temp);
    RETAILMSG(ZONE_FUNCTION,(L"CAPI2_SIMLOCK_GetStatus_RSP -> new simlock_state after check with SIM data, PN(%d), PU(%d), PP(%d), PC(%d), PS(%d).\r\n", simlock_state.network_lock, simlock_state.network_subset_lock, simlock_state.service_provider_lock, simlock_state.corporate_lock, simlock_state.phone_lock));
#endif
    CAPI2_SIMLOCK_GetStatus_RSP(tid, clientID, simlock_state);
}

void _DEF(CAPI2_FFS_Read)(UInt32 tid, UInt8 clientID, FFS_ReadReq_t *ffsReadReq)
{
    FFS_Data_t response ={NULL, 0};
    RETAILMSG(ZONE_FUNCTION,(L"CAPI2_FFS_Read : mFileId:%d mMaxLength:%d\r\n", ffsReadReq->mFileId, ffsReadReq->mMaxLength));

    CAPI2_FFS_Read_RSP(tid, clientID, &response);
}

void _DEF(CAPI2_SMS_GetRecordNumberOfReplaceSMS)(UInt32 tid, UInt8 clientID, SmsStorage_t storageType, UInt8 tp_pid, uchar_ptr_t oaddress)
{

    CAPI2_SMS_GetRecordNumberOfReplaceSMS_RSP(tid, clientID, 0);
}

#ifndef RPC_INCLUDED
//AT command for RPC reverse
#define ATC_DEVICE_NAME       L"ATT1:"
#define IOCTL_ATC_SEND_CMD    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0200, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define MAX_AT_STRING_LEN     256
#endif

BOOL SendATStringToATC(char *cmdStr, int len)
{
    BOOL bRet = FALSE;
    HANDLE hAtt = NULL;
    DWORD Ioctl = 0;
    char atString[MAX_AT_STRING_LEN]={0};

    if(len>= MAX_AT_STRING_LEN-1)
        RETAILMSG(1, (L"\r\n CpCmdHand: Error AT command string too long=%d\r\n",len ));

    hAtt = CreateFile(ATC_DEVICE_NAME, GENERIC_WRITE|GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

    if(INVALID_HANDLE_VALUE == hAtt)
    {
        RETAILMSG(1, (L" CPCmdHand: Open ATT driver failed!!\r\n"));
    }

    //memset(atString, 0x00, MAX_AT_STRING_LEN);
    memcpy(atString, cmdStr, len);
    cmdStr[len]=0;

    bRet = DeviceIoControl(hAtt, IOCTL_ATC_SEND_CMD, atString, len, 0, 0, NULL, NULL);
    RETAILMSG(1, (L"\r\n CpCmdHand: IOCTL_ATC_SENDCMD->Result(%d)\r\n", bRet));
    CloseHandle(hAtt);
    return bRet;
}


void _DEF(CAPI2_AT_ProcessCmdToAP)(UInt32 tid, UInt8 clientID, UInt8 channel, char* cmdStr)
{
    RETAILMSG(1,(L"\r\nCAPI2_AT_ProcessCmdToAP:len=%d [ %S ]\r\n", strlen(cmdStr), cmdStr));
    //IOCTL to ATC            
    SendATStringToATC(cmdStr, strlen(cmdStr));
    return;
}

void _DEF(CAPI2_InterTaskMsgToAP)(UInt32 tid, UInt8 clientID, InterTaskMsg_t *inPtrMsg)
{
	return;
}

void _DEF(CAPI2_RTC_GetDST)(UInt32 tid, UInt8 clientID)
{
    RETAILMSG(1,(L"CAPI2_RTC_GetDST::\r\n"));
    UInt8 dst = 0;

    CAPI2_RTC_GetDST_RSP(tid, clientID, dst);
}
