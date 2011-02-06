/****************************************************************************
*
*     Copyright (c) 2008 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           5300 California Avenue
*           Irvine, California 92617
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/

//to remove:
#define COMMAND_IPC_VPU_CMD_OFFSET   (0x100) // (For forwarding VPU commands via IPC to FUSE CP) The starting index for cmd destined to VPU Command Queue.
#define COMMAND_IPC_FAST_CMD_OFFSET  (0x110) // (For forwarding fast commands via IPC to FUSE CP) The starting index for cmd destined to Fast Command Queue.
#define COMMAND_IPC_FUNC_CMD_OFFSET  (0x200) // (For forwarding audio commands via IPC to FUSE CP) The starting index for cmd destined to Fast Command Queue.

typedef enum {
	AUDIO_IPC_CMD_INVALID
} AudioIpcFuncCall_en_t;

int Audio_Apps_EP_Register (void);

void AudioControl_CpEndpointRegister(void);
//to remove:
int IPC_AudioControlApSend(char *buff, UInt32 len);
