/****************************************************************************
*
*     Copyright (c) 2007 Broadcom Corporation
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
#ifndef _CPPS_CONTROL_H_
#define _CPPS_CONTROL_H_


// The CPPS_Control() function call returns the following operation status
typedef enum
{
    CPPS_CONTROL_PASS = 0,               ///< Successful
    CPPS_CONTROL_INVALID_PARAM,          ///< invalid parameters
    CPPS_CONTROL_UNKNOWN_CMD,            ///< unknown command
    CPPS_CONTROL_NO_FFS_SUPPORT_ON_AP,   ///< no CPPS support on AP
    CPPS_CONTROL_FILE_OP_FAIL,           ///< one of the file operation failed
    CPPS_CONTROL_IPCAP_PDS_OP_FAIL,      ///< IPCAP_PersistentDataStore operation failed
    CPPS_CONTROL_FFS_NOT_INIT,           ///< CPPS is not initialized

} CPPS_Control_Result_en_t;

/// various commands for CPPS_Control()
typedef enum
{
    CPPS_CONTROL_COPY_NVS_FILE_TO_SM = 0,    ///< copy the NVS file to shared memory
    CPPS_CONTROL_COPY_SM_TO_NVS_FILE,        ///< copy shared memory to the NVS file

} CPPS_Control_en_t;



UInt32 CPPS_Control(UInt32 cmd, UInt32 address, UInt32 offset, UInt32 len);


#endif // _CPPS_CONTROL_H_

