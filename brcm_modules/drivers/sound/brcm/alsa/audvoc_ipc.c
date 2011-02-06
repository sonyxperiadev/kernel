/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/broadcom/IPCInterface.h>

#define __AUD "AUD:IPC: "

static IPC_BufferPool brcm_ApAudioPool = 0;

// extern
extern void brcm_dsp_interrupt_hisr(void *, UInt32);

// non-static
void brcm_ipc_aud_control_register(void);
int brcm_ipc_aud_control_send(const void * const, UInt32);

// static
static void AudioControlAp_DeliveryFunction(IPC_Buffer);

/* ---------------------------------------------------------------------------------------- */

static void AudioControlAp_DeliveryFunction (IPC_Buffer buf)
{
        brcm_dsp_interrupt_hisr(IPC_BufferDataPointer(buf), IPC_BufferDataSize(buf));
        IPC_FreeBuffer(buf);
}



void brcm_ipc_aud_control_register(void)
{
        IPC_EndpointRegister(
                IPC_EP_AudioControl_AP,
                NULL, //AudioControlAp_FlowControlFunction,
                AudioControlAp_DeliveryFunction,
                0
        );

        while (0 == IPC_SmIsEndpointRegistered(IPC_EP_AudioControl_AP)) {
                msleep(15);
        }

        brcm_ApAudioPool = IPC_CreateBufferPool(
                                   IPC_EP_AudioControl_AP,
                                   IPC_EP_AudioControl_CP,
                                   256,
                                   64,
                                   1,
                                   0
                           );
}

int brcm_ipc_aud_control_send(const void * const buff, UInt32 len)
{
        IPC_Buffer buffer;

        buffer = IPC_AllocateBuffer(brcm_ApAudioPool);

        if (buffer) {
                if (0 == (IPC_BufferSetDataSize(buffer, len))) {
                        return 1;
                }

                memcpy(IPC_BufferDataPointer(buffer), buff, len);
                IPC_SendBuffer(buffer, IPC_PRIORITY_DEFAULT);

        } else {
                pr_info(__AUD"IPC_AudioControlSend: Pool Empty\n");
                return 1;
        }
        return 0;
}

